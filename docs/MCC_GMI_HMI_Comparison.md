# MCC 구조 비교 — GMI(기준) vs HMI(대상)

작성: 2026-07-31 (LYS) / 근거: GMI `D:\IT8.6G\GMI\GMI\IT_8.6G_HBS_20260331`, HMI `D:\OCP\0.SW_Matching\HMI`

## 0. 아키텍처 결론

**양쪽 모두 동일한 3계층 구조. 외부 MCC 서버와의 TCP/시리얼 링크는 양쪽 다 없음.**

```
App(EXE/DLL) ── CMCCLogMgr ──> dMCCSpec.dll (API_MCC 10종, CDlgnMCCReport)
                                  │ 공유메모리 "VTM_MCC_Shared_Memory_KMAC"
                                  │ + WM_MCC_ADDQUEUE(WM_USER+5000) SendMessageTimeout
                                  ▼
                             MCCLog.exe (별도 프로세스, dMCCSpecRec.dll)
                                  ├─ T-Log/I-Log/index CSV 실제 기록
                                  └─ (옵션) FTP 업로드 — Config_MCC.ini [FTP]
```

- 인앱 dMCCSpec의 파일 기록/FTP 코드는 **양쪽 모두 주석 처리** (GMI `DlgnMCCReport.cpp:344-366, 1101-1215` ≡ HMI 동일) — 화면 표시용만 남음.
- CSV 파일 기록의 실체는 **MCCLog.exe**. 앱은 공유메모리를 채우고 큐에 push할 뿐.
- MccMasterList XML은 **양쪽 소스 어디에서도 읽고 쓰지 않음** — 삼성 MCC 평가 툴 측 채점 기준 파일 (Qual 시 함께 제출/보관).
- dMCCSpec DLL 소스는 9개 핵심 파일 중 7개가 GMI↔HMI **바이트 동일**. 차이 2개 파일 (아래 §2).

## 1. Qual 산출물 파일 계약 (GMI 실물 기준)

| 파일 | 형식 |
|---|---|
| **I-Log** `I-<ModuleID>-YYYYMMDDhhmmss.csv` | 1초 주기. 헤더 `EventTime,LogType` + `모듈=항목명(단위)`×N. GMI: 207컬럼 = 2 + 38축×5(SET_SPD/SET_POS/SPD/POS/TRQ) + 아날로그 15(TEMP 3 + 전력계 N01/N02 각 6) |
| **T-Log** `T-<ModuleID>-YYYYMMDDhhmmss.csv` | 이벤트. `시각(MMDD_HHMM_SS.ms),모듈,A|S|E|W,StepID,GlassID,LotID,Recipe=Recipe,이벤트명=From=To=START|END` |
| **index** `index-...csv` | 빈 파일 (존재 표식) |
| **MccMasterList XML** | 모듈별×LogType(A/S/A3I)별 Context 목록 + Elapse_Min/Maxtime, Range_Min/Max, 누락검사 플래그. 삼성 측 관리 |

파일명의 `<EQPID>` 자리에는 실제로 **ModuleID**(Config_MCC.ini `[MCC] MODULEID`)가 들어가며 채번은 MCCLog.exe 몫.

## 2. 14개 항목 비교표

| 구분 | GMI (기준) | HMI (대상) | 차이점 | 적용 방안 |
|---|---|---|---|---|
| 초기화 | `DlgSubLog::OnInitDialog`→`CMCCLogMgr::Initialize` (`dMainForm\DlgSubLog.cpp:566`). `Config_MCC.ini` 읽고 `m_bUse` 게이트 후 API 10종 초기화 | 동일 지점 (`DlgSubLog.cpp:324`). 단 **ini는 `CoreFunction.ini [MCC]`를 잘못된 경로로 읽어 항상 기본값**, `Config_MCC.ini` 읽는 코드 없음 | 설정 소스·게이트 부재 | GMI 방식 채택: `Config_MCC.ini` 읽기 + `m_bUse` 게이트 (M2) |
| 연결 관리 | MCCLog.exe를 `ShellExecute`로 자동 기동(T6, 5초) + 60초 생존감시(T5). `FindTargetWindow("MCCLog")` | **동일 코드 존재·동작** (`DlgnMCCReport.cpp:189-191, 527-550`) | 없음 | 그대로 사용 |
| 송수신 구조 | 공유메모리 + 링큐 10000 + `WM_MCC_ADDQUEUE` | 동일 | 없음 (양쪽 다 큐/슬롯 무락) | 그대로 사용 + 이벤트 슬롯 락 추가 (M1) |
| 설비 상태 보고 | Auto/Manual 등 상태 전용 훅 없음 — 개별 Action/Signal로 간접 표현. 상태전이 훅 673개소(`FunctionModuleEQStateChangeStep.cpp`) | 훅 408개소, 동일 패턴. `MCC_A_CHK_MOTION_SAFETY`(`FunctionModuleEQState.cpp:143,150`)·`DlgSubAlarm.cpp:131` 일부 주석 | 훅 밀도·일부 주석 | 주석 훅 2건 검토 복원 (M5) |
| Recipe/Model 연동 | CIM 수신 필드 17/18/19→`SetMCCGlassID/LotID/RecipeID`, 이벤트마다 공유메모리 복사. GMI는 dual-glass `szGlassID2` | 동일 구조 (단일 글라스). `stMCCInfoEQ` 복사 동작 | GMI dual-glass는 GMI 전용 | 그대로 사용 (szGlassID2 이식 불필요) |
| 검사 시작/완료 | `MCC_AUTO_CYCLE`(CIM), `MCC_COMPONENT_IN/OUT`, `MCC_RESULT_UPLOAD` | `DoWorkGlassSequenceEngine.cpp:627/857` AUTO_CYCLE, CIM `:860-902` COMPONENT_IN/OUT — **이미 존재** | 없음 | 그대로 사용 |
| 결과 데이터 | `MCC_RESULT_UPLOAD` Action (T-Log) | 대응 Action 존재 여부 감사 필요 | 소차이 | 감사 후 필요 시 훅 추가 (M5) |
| Alarm/Error | `DlgSubAlarm`→`SetMCC_Alarm`. **GMI는 Qual 후 20250112 조기 return으로 비활성화** (Qual 시점엔 활성 — 샘플 T-Log에 E행 존재) | 활성 (`DlgSubAlarm.cpp:155/184`). 일부 경로(`:131`) 주석 | GMI의 조기 return은 Qual 후 조치 → 따라하지 않음 | HMI는 활성 유지 (Qual 시점 동작 기준) |
| UI 표시 | Log 탭 MCC 버튼 → `CDlgnMCCReport`(실시간 뷰어, 버튼/차트 없음) | 동일 (`DlgSubLog.cpp:560`, `ALPHA.rc:256`) | 없음 | 그대로 사용 |
| 설정 파일 | `Config_MCC.ini`(앱: USE/USABLE/WRITE_DATA/UPLOAD_TIME/SAMPLING_TIME ← 단 후행공백 키버그 / MCCLog.exe: MODULEID·FTP·DIR) + `CONFIG\MCC\` CSV 4종 (MCCNAME 205행=38축×5+15) | `Config_MCC.ini` 존재하나 **앱이 안 읽음**. `MCCNAME.csv` **20행 = 1축(WT_X)×5 + 15** — 모터 헤더 사실상 미구성 | **최대 격차**: 설정 읽기 부재 + MCCNAME 축 목록 부재 | M2(ini) + M4(MCCNAME 축 확장) |
| 로그 | MCCLog.exe가 `<LOG>\MCC\{T,I,INDEX}\` 기록, UPLOAD_TIME(분) 롤오버, 보존정책 없음 | 동일 구조 (MCCLog.exe 존재 `Source\Bin\MCCLog.exe`). 산출 여부 미검증 | 바이너리 세대 차이(HMI판 34KB vs GMI판 2024-10 37KB) | 실기동 검증 (M6). 필요 시 MCC_src.zip 재빌드 검토 |
| 재접속/복구 | MCCLog.exe 죽으면 60초 내 재기동. 파일 기록 실패 시 재시도 없이 "File Save Failed" 표시. 앱 재시작 시 공유메모리 재생성(큐 유실 허용) | 동일 | 없음 | 그대로 사용 |
| 스레드 동기화 | 앱 수집: 워커 200ms(모터)+타이머 200ms(센서). DLL: g_csTLog 등 4개 CS. **이벤트 슬롯 `stMCCDataEQ`·링큐 무보호 (양쪽 공통 결함)** | 워커 50ms(모터)+UI타이머 1000ms(센서). 동일 결함 + `m_pDFDCData` 무락 | 주기 차이는 무해 | 이벤트 슬롯 락 신설 (M1) — GMI보다 개선 |
| Qual 대응 로직 | 엣지 검출(`unloadFlag`), 센서 2초 디바운스, Align 반복스텝 특례(`iSpqNum+nStep`), nModuleID 모듈 구분, 2024-09~10 주석 다수 | Align류 특례 없음(필요 여부 감사), 모듈 단일이라 nModuleID 불요 | GMI 특례는 GMI 시퀀스 종속 | HMI 시퀀스에 맞는 것만 선별 (M5) |

## 3. 6분류

### (1) HMI에 이미 존재, 그대로 사용
- 3계층 아키텍처 전체, API 10종 연동, 공유메모리/링큐/메시지, MCCLog.exe 자동기동·감시
- 이벤트 훅 408개소 (시퀀스/상태전이/CIM/알람), Glass·Lot·Recipe ID 연동
- 모터 현재값(POS/SPD/RMS) 50ms 수집, MCC 로그 뷰어 UI

### (2) 존재하나 GMI 대비 부족 → 보완
- **I-Log dData**: HMI는 온도 압축 루프(가변 슬롯)만 — GMI는 MCCNAME.csv와 1:1 고정 슬롯 15개(TEMP 3+전력 12). → 고정 슬롯제 전환 (M3)
- **설정**: `Config_MCC.ini` 앱측 읽기 + USE 게이트 + CIM 원격제어 명령(HMI는 구식 `MCC_ON$/MCC_OFF$`만, GMI는 `MCC_FTP_ON/OFF/TERM`, `MCC_LOG_WRITE_*`, `MCC_ILOG_TERM`) (M2)
- **MCCNAME.csv**: 1축→실보고 축 전체로 확장 (M4)
- NULL 가드/락/버그 수정 (M1)

### (3) HMI에 없어 신규 추가
- MCC enable/disable 스위치 (Config_MCC.ini `[MCC] USE`) + 비활성 시 안전 동작
- (선택) `uMCCSamplingTime`/`bMCCWriteFile` 구조체 필드 — **MCCLog.exe 바이너리 호환성 리스크로 보류** (아래 §4-주의)

### (4) GMI 전용 → 이식 금지
- `szGlassID2`(dual-glass), `nModuleID`+MODULEID2~5(ST01/ST02/TR01/BF01 다중 모듈), `stMoterInfo[300]` 확장(HMI 90축엔 100이면 충분)
- GMI 액션 코드셋(`MCC_LD_STG1_*`, `MCC_WAD_MEAS_*`...), UPS/GPS 장치 연동, P2K/Align PC 데이터, PG1~4 상태

### (5) HMI 시퀀스에 맞게 변환
- dData 전력 12슬롯의 소스: GMI=UPS/GPS 계측기 ↔ HMI=ELP ElecBox 2대 (의미 매핑 확인 필요)
- 온도 3슬롯 소스: GMI=`dTemperature[6],[8],[9]` ↔ HMI 대응 FDC 온도 인덱스 확정 필요

### (6) 프로토콜/고객사 확인 필요
- **C1**: MCCNAME.csv의 `N0x_R_VOL/T_VOL/R_CUR/T_CUR`(R상/T상) ↔ ELP 항목(전압/고조파/누설) 매핑 — 기존 질의서 `ELP_Upload_Confirmation_Requests.md` B절과 동일 사안
- **C2**: HMI가 I-Log에 보고할 축 목록(=MCC Chart) — GMI는 38축이었음. HMI MCC Chart 문서 확인 필요
- **C3**: MccMasterList XML의 HMI판 작성 주체/시점 (삼성 측과 협의 산출물)
- **C4**: HMI `Config_MCC.ini`의 MODULEID(`A3HMI01H_HMIU`)/POSITION_FROM/TO/FTP 계정 현행 유효성

## 4. GMI에서 발견된 결함 (따라하지 말 것 / HMI에서 개선)

| # | GMI 결함 | HMI 대응 |
|---|---|---|
| D1 | ini 키 후행 공백 `"USABLE "`, `"WRITE_DATA "` → 값 미반영 의심 (`GMI MCCLogMgr.cpp:142-143`) | 공백 없는 키 사용 |
| D2 | `dfMCC_LOG_WRITE_OFF == "MCC_LOG_WRITE_ON"` 오타 → OFF 명령 도달 불가 (`GMI DefinesCIM.h:190`) | 올바른 문자열로 정의 |
| D3 | `DeleteQueue` memset `sizeof(PMCC_EQDATA_T)`(포인터 8B) (`DlgnMCCReport.cpp:1822`) — **HMI에도 동일 존재** | `sizeof(MCC_EQDATA_T)`로 수정 |
| D4 | 이벤트 슬롯/링큐 무락 — 다중 스레드 이벤트 유실 가능 | 앱측 CS 신설 |
| D5 | `SetMCC_Signal/Alarm` NULL 가드 없음 (Action만 있음) | 전 함수 가드 |
| D6 | `SetMccMoterInfo`(지령값) 호출 0건 → SET_* 컬럼 상시 0. **GMI도 이 상태로 Qual 통과** | 동등 수준 유지(구현 불요). 보고서에 명기 |
| D7 | `uMCCSamplingTime` 어디서도 타이머로 미소비 (하드코딩 200ms/1000ms) | 동일 유지 (MCCLog.exe 소비용 파라미터) |
| D8 | `MCC_src.zip`(2022) ↔ 배포 MCCLog.exe(2024-10) 소스 불일치 | HMI MCCLog.exe(34KB)도 소스 세대 확인 필요 — 검증 항목 |

**주의(구조체 호환성)**: GMI `VTM_MCC`는 `uMCCUploadTime` 뒤에 `uMCCSamplingTime`/`bMCCWriteFile`을 **중간 삽입**했다. HMI의 배포 MCCLog.exe는 HMI 레이아웃(해당 필드 없음, `stMoterInfo[100]`) 기준으로 빌드된 것으로 추정되므로, **HMI `SheardDefineMCC.h`의 기존 필드 순서/크기는 절대 변경 금지**. 필드 추가가 필요해지면 구조체 끝에 append + MCCLog.exe 재빌드(MCC_src.zip)와 세트로만 진행.

## 5. HMI측 기지 결함 목록 (이번 구현에서 수정)

| # | 내용 | 위치 |
|---|---|---|
| H1 | `UpdateMCCILOG` 모터 루프에서 GMI에 있는 `iAxis = iLoop;` 누락 → 축0 값만 반복 표시 (화면 표시 한정, 파일은 MCCLog.exe가 별도 생성이라 무관) | `CoreFunction\dMCCSpec\DlgnMCCReport.cpp:1663` 부근 |
| H2 | `m_pSharedMCC` NULL 가드 부재 — USE=0 설정 시 즉시 크래시 | `Manager\MCCLogMgr.cpp:162,170,486,514,534` |
| H3 | ini 경로 불일치(`Bin\CoreFunction\CoreFunction.ini` — 실존 안 함) → 설정 무효 | `MCCLogMgr.cpp:62` |
| H4 | `#include "MotorFunction/AxisType.h""` 따옴표 여분 | `MCCLogMgr.cpp:10` |
| H5 | D3과 동일 memset 버그 | `dMCCSpec\DlgnMCCReport.cpp` DeleteQueue |
| H6 | dData 가변 슬롯 vs MCCNAME 고정 컬럼 불일치 (U2) | `MCCLogMgr.cpp:249-255` |
| H7 | 종료 시 워커 스레드 미기동인데 5초 대기 타임아웃 | `DlgnMCCReport.cpp:1294` (GMI 동일 — 수정은 선택) |
