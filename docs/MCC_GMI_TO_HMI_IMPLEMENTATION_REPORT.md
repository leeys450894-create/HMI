# MCC 기능 GMI→HMI 구현 결과 보고서

작성: 2026-07-31 (LYS)
기준 프로젝트: GMI `D:\IT8.6G\GMI\GMI\IT_8.6G_HBS_20260331` (MCC Qual 완료, 2024-10-08)
대상 프로젝트: HMI `D:\OCP\0.SW_Matching\HMI` (ALPHA)
선행 문서: [MCC_GMI_HMI_Comparison.md](MCC_GMI_HMI_Comparison.md) (구조 비교), [MCC_Implementation_Plan.md](MCC_Implementation_Plan.md) (구현 계획)

---

## 1. GMI MCC 구조 분석 결과 (요약)

MCC는 외부 서버 통신이 아니라 **로컬 파일 수집 + 별도 프로세스 기록** 구조다.

```
App(CMCCLogMgr) → dMCCSpec.dll(API_MCC 10종) → 공유메모리 "VTM_MCC_Shared_Memory_KMAC"
                                                + WM_MCC_ADDQUEUE(WM_USER+5000)
                                              → MCCLog.exe(dMCCSpecRec.dll)가 CSV 기록 + FTP
```

- **T-Log** `T-<ModuleID>-YYYYMMDDhhmmss.csv`: 이벤트 행 `시각,모듈,A|S|E|W,Step,GlassID,LotID,Recipe=Recipe,이벤트=From=To=START|END`. 앱의 `SetMCC_Action/Signal/Alarm` → 공유메모리 링큐(10000) → MCCLog.exe가 MCCACTION/MCCESIGNAL/MCCERROR.csv로 번호→이름 변환 후 기록.
- **I-Log** `I-<ModuleID>-...csv`: 1초 주기. 컬럼 = `EventTime,LogType` + bUse=TRUE 축×5(SET_SPD/SET_POS/SPD/POS/TRQ) + 아날로그 `nCntILog`개. 헤더는 MCCNAME.csv에서 생성 (GMI: 38축×5+15=207컬럼, 실측 검증).
- **MccMasterList XML**: 삼성 MCC 평가 툴 측 채점 기준(Context별 Elapse/Range/누락검사). **설비 SW는 읽지도 쓰지도 않음** — Qual 시 협의 산출물.
- MCCLog.exe는 앱이 자동 기동(5초 후)·60초 생존 감시. 파일 롤오버는 UPLOAD_TIME(분) 주기.
- GMI도 `SetMccMoterInfo`(지령값) 호출부가 0건이라 I-Log SET_* 컬럼이 전부 0.000인 채로 **Qual 통과**했음 (실측 CSV로 확인).

## 2. GMI/HMI 기능 비교표

상세 14항목 비교표는 [MCC_GMI_HMI_Comparison.md](MCC_GMI_HMI_Comparison.md) §2 참조. 핵심 결론:

| 영역 | 결론 |
|---|---|
| 아키텍처/전송/UI/MCCLog 감시 | 양측 동일 — HMI 그대로 사용 |
| 이벤트 훅 | HMI 이미 408개소 완비 (시퀀스/상태전이/CIM/알람) |
| 설정 읽기 | HMI 결손(잘못된 ini 경로) → **이번에 GMI 방식으로 구현** |
| I-Log dData | HMI 가변 압축(결함) → **고정 슬롯 15개로 구현** |
| MCCNAME.csv | HMI 1축뿐 → **20축×5+15=115행 재생성** |
| MasterList XML | 양측 코드 무관 — 삼성 협의 산출물 (C3) |

## 3. HMI에 신규 추가한 기능

1. **MCC enable/disable** — `Config_MCC.ini [MCC] USE=0`이면 dMCCSpec 초기화·공유메모리 생성·MCCLog.exe 기동이 전부 생략되고, 모든 MCC 진입점이 NULL 가드로 무동작 (설비 운전 무영향).
2. **Config_MCC.ini 앱측 읽기** — `USE / OUT_OF_SERVICE / USABLE / WRITE_DATA / UPLOAD_TIME / SAMPLING_TIME`. `UPLOAD_TIME`·`USABLE`은 공유메모리(`uMCCUploadTime`/`bMCCTransfer`)로 반영되어 MCCLog.exe가 소비. (GMI의 후행 공백 키 버그 `"USABLE "`는 재현하지 않음)
3. **이벤트 직렬화 락** `m_cs_MCCEvent` — 단일 슬롯 `stMCCDataEQ` 기록~`API_MCC_AddQueue` 구간을 원자화. 다중 스레드(시퀀스/알람/CIM) 동시 호출 시 이벤트 유실·혼입 방지 (GMI에 없던 개선).
4. **I-Log dData 고정 슬롯제** — MCCNAME.csv 아날로그 15행과 1:1: `[0..2]` 온도(PC_RACK/EB_RACK_1/2 ← FDC Temp_PCBOX/EBOX_F/EBOX_B), `[3..8]`/`[9..14]` = N01/N02 ← ELP EB1/EB2 (T_PW←적산전력, I_PW←순시전력, R_VOL←전압, R_CUR←전류; T_VOL/T_CUR는 ELP에 T상 값이 없어 0 + `TODO: MCC_CONFIRM`). `nCntILog=15` 고정.
5. **MCCNAME.csv 실축 반영** — Config_Motion_ACS.dat의 USE=1 축 20개(축 인덱스 오름차순, MCCLog.exe 데이터 순서와 일치)×5행 + 아날로그 15행 = 115행.
6. **검증 도구** `docs/tools/mcc_sharemem_reader.py` — VTM_MCC 공유메모리를 열어(기존 매핑만) EQ정보/모터/dData/큐를 덤프.

## 4. 기존 기능 보완(버그 수정) 항목

| # | 수정 | 파일:위치 |
|---|---|---|
| B1 | `#include "...AxisType.h""` 따옴표 여분 제거 | `Manager\MCCLogMgr.cpp:10` |
| B2 | `SetMccMoterInfo/Cur, SetMCC_Action/Signal/Alarm` 전부에 `m_pSharedMCC` NULL 가드 (기존: 무가드 역참조 → USE=0 시 즉시 크래시) | `MCCLogMgr.cpp` |
| B3 | `UpdateSensor()` MCC 의존 블록(dData/축 발행) NULL 가드 — MCC OFF여도 FDC/ELP 발행은 유지 | `MCCLogMgr.cpp` |
| B4 | I-Log 표시 루프 `iAxis = iLoop;` 복원 (GMI에 있고 HMI에서 누락 → 축0 값만 90회 반복 표시) | `dMCCSpec\DlgnMCCReport.cpp:1665` |
| B5 | `DeleteQueue` memset `sizeof(PMCC_EQDATA_T)`(포인터 8B) → `sizeof(MCC_EQDATA_T)`(148B) — GMI에도 있는 결함 | `DlgnMCCReport.cpp:1820` |
| B6 | `CMCCManger` — 생성자 `m_dlgMCC=nullptr` 명시 + `KillThred/AddMccQueue/GetSharedMemory` NULL 가드 | `dMCCSpec\MCCManger.cpp` |
| B7 | **알람 짝 불일치**: CIM Alarm not-report 모드에서 SET은 미보고·RESET만 보고되어 T-Log에 짝 없는 RESET 발생 → SET 보고 복원 (MCC 채점은 START/END 쌍 검사) | `DlgSubAlarm.cpp:131` |
| B8 | Finalize에서 USE=0인데 DLL 종료 API 호출하던 경로 차단 | `MCCLogMgr.cpp` Finalize |

## 5. 수정·생성 파일 목록

| 파일 | 구분 | 백업 |
|---|---|---|
| `Source\ALPHA\Manager\MCCLogMgr.h` | 수정 (멤버 6개 추가) | `.bak_mcc` |
| `Source\ALPHA\Manager\MCCLogMgr.cpp` | 수정 (12개 편집) | `.bak_mcc` |
| `Source\ALPHA\DlgSubAlarm.cpp` | 수정 (B7) | `.bak_mcc` |
| `CoreFunction\dMCCSpec\DlgnMCCReport.cpp` | 수정 (B4, B5) | `.bak_mcc` |
| `CoreFunction\dMCCSpec\MCCManger.cpp` | 수정 (B6) | `.bak_mcc` |
| `Source\Bin\Config\Config_MCC.ini` | 수정 (키 4개 추가) | `.bak_mcc` |
| `Source\Bin\Config\MCC\MCCNAME.csv` | 재생성 (20→115행) | `.bak_mcc` |
| `docs\tools\mcc_sharemem_reader.py` | 신규 | — |
| `docs\MCC_GMI_HMI_Comparison.md` / `MCC_Implementation_Plan.md` / 본 보고서 | 신규 문서 | — |

`SheardDefineMCC.h`(공유 구조체)는 **무변경** — 배포 MCCLog.exe와의 레이아웃 호환 유지가 원칙 (비교 문서 §4-주의).

## 6. 주요 호출 흐름

```
[T-Log] 시퀀스/알람/CIM (408개소)
  → CMCCLogMgr::SetMCC_Action/Signal/Alarm   ← NULL가드 + m_cs_MCCEvent 락 (신규)
  → stMCCDataEQ 슬롯 기록 + Glass/Lot/Recipe/Step ID 복사
  → API_MCC_AddQueue → CDlgnMCCReport::AddMccQueue
  → SendMessageTimeout(MCCLog창, WM_MCC_ADDQUEUE, 200ms) + 링큐 push
  → MCCLog.exe: DeleteQueue → 이름 변환(CSV) → T-*.csv 기록

[I-Log] DlgMainForm 워커(50ms): 축 90개 중 USE축 → SetMccMoterInfoCur → stMoterInfo[]
        DlgSubOperate 타이머(1s): UpdateSensor → dData[0..14] 고정 슬롯 (신규) + nCntILog=15
  → MCCLog.exe: 1초 주기로 MCCNAME.csv 헤더 기준 I-*.csv 기록
```

## 7. HMI 시퀀스와 MCC 연동 위치 (기존 유지, 대표)

- 글라스 사이클: `DoWorkGlassSequenceEngine.cpp:627/857` (AUTO_CYCLE START/END)
- 반입/반출: `CommTcpIpObjectCIM.cpp:860-902` (COMPONENT_IN/OUT)
- 초기화 22단계: `SystemInitializeStep.cpp` 79개소
- 측정: `DoWorkGlassMeasureStep.cpp` 77개소, 상태전이: `FunctionModuleEQStateChangeStep.cpp` 66개소
- 알람 SET/RESET: `DlgSubAlarm.cpp` OnAlarmOccured/OnAlarmCleared (B7로 전 모드 쌍 보장)

## 8. 설정 파일 추가 항목

`Config\Config_MCC.ini [MCC]`: `USE=1`, `OUT_OF_SERVICE=0`, `WRITE_DATA=1`, `SAMPLING_TIME=1000` 추가 (기존 `EQTYPE/USABLE/UPLOAD_TIME/MODULEID/POSITION_*/[FTP]/[DIR]`는 MCCLog.exe가 종전대로 사용).

## 9. 통신 프로토콜 및 데이터 매핑

- 파일 계약(포맷)은 §1과 비교 문서 §1. GMI 실측 CSV·MasterList XML로 대조 완료.
- dData 매핑표는 §3-4 항. **프로토콜 문서 vs 코드 차이**: GMI `Doc\` 내 MCC Chart xlsx 2종은 미열람(바이너리) — 코드·실측 CSV 기준으로 검증했고, HMI용 MCC Chart는 C2 확인 필요.

## 10. 스레드 안전성·예외 처리

- 이벤트 경로: `m_cs_MCCEvent`(신규)로 직렬화. ELP 캐시는 기존 `m_cs_ELP`.
- MCC OFF/실패 경로: 모든 진입점 NULL 가드 → 크래시 불가. FDC/ELP 발행은 MCC와 독립 동작.
- MCCLog.exe 부재: 5초 후 자동 기동 시도, 60초 주기 재기동 (기존 로직, GMI 동일).
- 파일 기록 실패: MCCLog.exe 측 "File Save Failed" 표시 후 계속 (재시도 없음 — GMI 동일).
- 큐 오버플로: 링큐 10000, 초과 시 덮어씀 (GMI 동일 — 알림 없음, §14 개선 항목).

## 11. 테스트 결과

| 항목 | 결과 |
|---|---|
| ALPHA.sln **Debug x64** 전체 빌드 | ✅ 에러 0 (기존 경고만) |
| ALPHA.sln **Release x64** 전체 빌드 | ✅ 에러 0 |
| EUC-KR 손상 검사 (수정 5개 소스) | ✅ 전부 damaged=0, CRLF 유지 |
| MCCNAME.csv 정합 | ✅ 115행 = USE축 20×5 + 아날로그 15 (아날로그 15행 바이트 보존, ℃ 인코딩 유지) |
| USE=0 안전성 (정적) | ✅ 초기화 생략 + 전 진입점 가드 + CMCCManger 가드 + UI측 기존 null check 확인 |
| 시나리오 6·7·8 (불량 패킷/중복/타임아웃) | 해당 없음(파일 기반). SendMessageTimeout 200ms·큐 코드 검토로 대체 |

## 12. 런타임 검증 결과 (2026-07-31 실기동, ALPHA는 사용자 실행)

| # | 항목 | 결과 |
|---|---|---|
| 1 | `VTM_MCC_Shared_Memory_KMAC` 생성 + reader 검증 | ✅ nEQType=3, uMCCUploadTime=1440·bMCCTransfer=1(**ini 반영 확인**), bUse 축 정확히 20개, nCntILog=15, GlassID/LotID/Recipe 정상 |
| 2 | MCCLog.exe 연동 | ✅ 자동 기동·공유메모리 접속(ModuleID `A3HMI01H_HMIU` 기록), 큐 5건 발행→5건 소비(front=rear=5, 유실 0) |
| 3 | 강제 종료 후 자동 재기동 (시나리오 4·5) | ✅ taskkill 후 **13초 내 재기동**, 신규 T/I/index 파일 채번 |
| 4 | **I-Log 컬럼 정합** | ✅ 신규 파일 헤더 117 = 데이터 117 = 2+20축×5+15. 헤더 형식 `A3HMI01H_HMIU=SET_MOT_X0_SPD(mm/s)` — GMI와 동일 |
| 5 | T-Log 포맷/Unicode 호환 | ✅ `0731_1314_46.033,A3HMI01H_HMIU,A,...,INIT_PRE_CAM=...=START` — GMI 실측과 동일 포맷, 이벤트명 CSV 변환 정상, Unicode 문자열 정상 → **MCCLog.exe 바이너리 레이아웃 호환 확정** |
| 6 | 축 순서 실측 교정 | ✅ 공유메모리 실측으로 bUse 순서 확인 → MCCNAME.csv를 런타임 순서(X0,X2,X3,X4,Y1~Y4,R1,Z1~Z6,Z9,LZ,LR,PBY5,PBZ5)로 재생성 (dat 파일 순서와 LR 위치가 달랐음) |

### 미검증 잔여
1. USE=0 실기동 무크래시 (정적 검증만 완료 — 전 진입점 가드)
2. 프로그램 정상/비정상 종료 후 재기동 (시나리오 14·15)
3. 시뮬레이터(elp_elecbox_sim.py) 주입 → dData[3..8]/[9..14] ELP 값 반영
4. FDCSense 가동 시 dData[0..2] 온도 반영
5. UPLOAD_TIME(1440분) 자연 롤오버 및 FTP 전송 (팹 환경)

## 13. MCC Qual 전 추가 확인 필요 (TODO: MCC_CONFIRM)

| # | 항목 | 비고 |
|---|---|---|
| C1 | N0x_R/T상 컬럼 ↔ ELP 항목 매핑 | 기존 질의서 `ELP_Upload_Confirmation_Requests.md` B절. 현재 R←실측, T←0 |
| C2 | HMI MCC Chart(보고 축 목록·명명) | 현재 USE축 20개 전체·DIVISION명(X0, PBY5...) 기준 — 고객사 Chart와 대조 필요 |
| C3 | HMI판 MccMasterList XML 작성 | 삼성 측과 협의 (설비 SW 무관) |
| C4 | Config_MCC.ini MODULEID(`A3HMI01H_HMIU`)/POSITION/FTP 계정 현행성 | |
| C5 | 온도 명명 PC_RACK/EB_RACK ↔ FDC Temp_PCBOX/EBOX_F/B 등가 | 근거: FDC_LIST.xls |
| C6 | CIM MCC 원격 명령 확장 여부 (HMI 구식 `MCC_ON$` vs GMI `MCC_FTP_*` 6종) | CIM 스펙 확인 후 |
| C7 | `MCC_RESULT_UPLOAD` 등 GMI 전용 Action의 HMI 채택 여부 | MCCACTION.csv(251행)와 MasterList 협의 사항 |

## 14. 향후 개선 사항

- `MCC_A_CHK_MOTION_SAFETY` 훅은 주석 유지 — 인터록 폴링 경로라 활성화 시 큐 폭주 위험. 상태 전이 시점 훅으로 재설계 검토.
- 링큐 오버플로 감지/경고, MCCLog.exe 소스(`MCC_src.zip`, 2022) 현행화 및 ALPHA.sln 편입 (FDCSense 전례).
- `SetMccMoterInfo`(지령값) 채움 — GMI도 미구현으로 Qual 통과했으나, 지령/실측 비교가 필요해지면 MotorControlMgr 이동 명령부에 훅 추가.
- 헤더 2벌(`Source\Include` ↔ `CoreFunction\dMCCSpec`) `#include` 리다이렉트 단일화 (FDC 전례).
