# HMI MCC 기능 구현 계획서 (GMI Qual 기준)

작성: 2026-07-31 (LYS) / 선행 문서: `MCC_GMI_HMI_Comparison.md`

## 설계 원칙 (요청 원칙 반영)

1. GMI Qual 완료 구조 기준 — 단 GMI 결함(D1~D5)은 따라하지 않고 개선.
2. **기존 필드 순서/크기 불변** — `SheardDefineMCC.h`의 `VTM_MCC` 레이아웃은 배포 MCCLog.exe와 공유하므로 절대 변경 금지.
3. `Config_MCC.ini [MCC] USE=0`이면 MCC 전체 비활성 + **어떤 경로도 크래시 없이 무동작** (NULL 가드 전면화).
4. EUC-KR 파일은 Perl raw-byte 편집, 신규 코드는 ASCII+CRLF, 주석 `// LYS 2026-07-31 ...`.
5. 불명확한 매핑은 값 0 + `TODO: MCC_CONFIRM` 주석 (임의 결정 금지).

## Phase 목록

### M1. 안전화 + 버그 수정 (회귀 위험 최소 항목부터)

| 파일 | 수정 | 목적 |
|---|---|---|
| `Source\ALPHA\Manager\MCCLogMgr.cpp:10` | `.h""` → `.h"` | H4 문법 오염 |
| `MCCLogMgr.cpp` `SetMccMoterInfo/SetMccMoterInfoCur/SetMCC_Action/Signal/Alarm/UpdateSensor/GetFDCDValue/SetDataToELP...` | 진입부 `if (m_pSharedMCC == nullptr) return;` (FDC측은 `m_pSharedFDC`/`m_pDFDCData`) | H2 — USE=0 안전화 |
| `MCCLogMgr.h/.cpp` | `CCriticalSection m_cs_MCCEvent` 신설, `SetMCC_Action/Signal/Alarm`에서 슬롯 기록~`API_MCC_AddQueue` 전 구간 락 | D4 이벤트 유실 방지 (GMI 개선) |
| `CoreFunction\dMCCSpec\DlgnMCCReport.cpp` UpdateMCCILOG | `iAxis = iLoop;` 복원 (GMI와 동일화) | H1 |
| `DlgnMCCReport.cpp` DeleteQueue | `sizeof(PMCC_EQDATA_T)` → `sizeof(MCC_EQDATA_T)` | H5/D3 |

주의: `Source\Include\SheardDefineMCC.h` ↔ `CoreFunction\dMCCSpec\SheardDefineMCC.h` 2벌 수기 사본 — 이번에 FDC 헤더처럼 `#include` 리다이렉트로 단일화(가능 시) 또는 동시 수정.

### M2. 설정 체계 — Config_MCC.ini 채택

- `CMCCLogMgr::LinkIniFile`을 GMI 방식으로 재작성: 경로 `<AppPath>\Config\Config_MCC.ini` (실존: `Source\Bin\Config\Config_MCC.ini`).
- 읽는 키: `[MCC] USE`(기본 1), `OUT_OF_SERVICE`(기본 0), `USABLE`(→`bMCCTransfer`), `WRITE_DATA`(앱 보관용), `UPLOAD_TIME`(→`uMCCUploadTime`), `SAMPLING_TIME`(앱 보관용) — **후행 공백 없는 키** (D1 회피).
- `Initialize()`: `m_bUse==false`면 API 초기화 전부 skip (m_pSharedMCC는 nullptr 유지 → M1 가드가 안전 보장). MCCLog.exe 기동도 일어나지 않음(초기화 안 된 DLL 다이얼로그가 없으므로).
- 공유메모리 반영: 기존 필드 `uMCCUploadTime`, `bMCCTransfer`만 (신규 필드 추가 금지 — 계획 원칙 2).
- `Config_MCC.ini`에 `USE=1`, `OUT_OF_SERVICE=0`, `WRITE_DATA=1`, `SAMPLING_TIME=1000` 키 추가 (파일은 CP949 유지).
- CIM 원격 명령: 기존 `MCC_ON$/MCC_OFF$/MCCTIMESET` 유지 (GMI식 `MCC_FTP_*` 명칭 확장은 CIM 스펙 확인 후 — TODO: MCC_CONFIRM, 이번 범위 제외).

### M3. I-Log dData 고정 슬롯제 (U2 해소)

`UpdateSensor()`의 dData 기록부를 GMI 패턴(고정 슬롯, MCCNAME.csv와 1:1)으로 교체:

| dData | MCCNAME 컬럼 | 소스 | 비고 |
|---|---|---|---|
| [0] | TEMP_PC_RACK | FDC dTemperature[?] | 인덱스 확정 필요 → 확정 전 0 + TODO |
| [1] | TEMP_EB_RACK_1 | FDC dTemperature[?] | 〃 |
| [2] | TEMP_EB_RACK_2 | FDC dTemperature[?] | 〃 |
| [3] | N01_T_PW(kWh) | ELP EB1 ACCUM_PW | 의미 명확 — 매핑 |
| [4] | N01_I_PW(kW) | ELP EB1 INST_PW | 〃 |
| [5] | N01_R_VOL | ELP EB1 VOLT | 잠정 (R상 단상 가정) TODO: MCC_CONFIRM |
| [6] | N01_T_VOL | 0 | ELP에 T상 없음 → TODO: MCC_CONFIRM |
| [7] | N01_R_CUR | ELP EB1 CURR | 잠정 TODO |
| [8] | N01_T_CUR | 0 | TODO |
| [9]~[14] | N02_* | ELP EB2 동일 패턴 | 〃 |

- `nCntILog = 15` 고정.
- 기존 온도 압축 루프는 제거(주석 보존)하되, FDC `stFDCWData` 온도들은 종전대로 FDC 경로에 남음 — 회귀 없음.
- ELP 값 접근은 기존 `m_dDataToELP[2][9]` + `m_cs_ELP` 재사용.

### M4. MCCNAME.csv 축 확장

- 데이터 행(MCCLog.exe 기록)은 `stMoterInfo[i].bUse==TRUE`인 축마다 5컬럼 — 헤더(MCCNAME.csv)와 개수가 일치해야 함.
- HMI에서 bUse=TRUE가 되는 축 = `DlgMainForm.cpp:1672` 루프에서 `SetMccMoterInfoCur` 호출되는 축 집합. 이 집합을 코드/Config_Motion에서 추출해 `<축명>_SPD/POS/TRQ` 5행×N 생성.
- **MCC Chart(보고 대상 축 목록)는 고객사 확인 항목(C2)** — 확인 전에는 "현재 유효 축 전체"로 생성하고 TODO 표기.
- 생성 후 런타임 검증: I-Log 헤더 컬럼 수 == 데이터 컬럼 수.

### M5. 훅 보완 (감사 후 최소 수정)

- `FunctionModuleEQState.cpp:143,150` `MCC_A_CHK_MOTION_SAFETY` 주석 훅 — 주석 사유 조사 후 복원 여부 결정.
- `DlgSubAlarm.cpp:131` 주석 알람 훅 — 동일.
- `MCC_RESULT_UPLOAD` 대응 훅 존재 여부 감사.
- GMI의 Align 반복스텝 특례(iSpqNum+nStep)는 HMI Align 시퀀스의 반복 구조 확인 후 필요 시.

### M6. 검증 (17개 시나리오 매핑)

| 시나리오 | 방법 |
|---|---|
| 1. MCC 비활성(USE=0) 정상 운전 | USE=0 설정 후 기동, SetMCC_* 경로 크래시 없음 (코드 검증 + 기동 확인) |
| 2~5. MCCLog.exe 연결/부재/끊김/복구 | MCCLog.exe 없이 기동(5초 후 자동기동 시도 로그), 강제 종료 후 60초 내 재기동 확인 |
| 6~8. 불량 패킷/중복/타임아웃 | 해당 없음(파일 기반) — SendMessageTimeout 200ms 및 큐 오버플로 코드 검토로 대체 |
| 9~13. Recipe/Glass/검사/알람 | 기존 훅 경유 T-Log 이벤트를 공유메모리 큐 리더 스크립트로 확인 (ALPHA 실행은 사용자) |
| 14~15. 정상/비정상 종료 | Finalize 경로 + 재기동 시 공유메모리 재생성 확인 |
| 16. 연속 데이터 누락/중복 | 락 적용 후 다중 이벤트 연타 시나리오 코드 검토 |
| 17. GMI 대조 | I-Log/T-Log 컬럼 구조를 GMI 샘플과 비교 |

- 빌드: ALPHA.sln Debug (dMCCSpec 포함), EUC-KR 손상 검사 `damaged=0`.
- 공유메모리 검사 도구: 기존 `docs\tools\fdc_sharemem_reader.py` 패턴으로 `mcc_sharemem_reader.py` 신규 (VTM_MCC_Shared_Memory_KMAC 열어 ILog/큐 확인).

## 위험 요소

| 위험 | 완화 |
|---|---|
| MCCLog.exe(바이너리, 소스 세대 미상)와 구조체 불일치 | 레이아웃 불변 원칙 + 실기동 CSV 검증. 불일치 발견 시 MCC_src.zip 재빌드 검토(별도 승인) |
| dMCCSpec.dll 수정이 GMI 등 타 기종과 소스 분기 | HMI 로컬 사본만 수정, 변경점 주석으로 식별 |
| UpdateSensor 수정 시 FDC 경로(stMainWData) 회귀 | FDC 발행 블록 무변경, dData 블록만 교체. ELP 시뮬레이터 회귀 테스트 재실행 |
| MCCNAME.csv 확장 시 기존 수집 데이터와 컬럼 불일치 | 어차피 현재 파일 미생성 상태 → 신규 기준으로 시작. 보고서에 명기 |

## 확인 필요 항목 (TODO: MCC_CONFIRM 목록)

1. N0x R상/T상 ↔ ELP 매핑 (기존 질의서 B절)
2. TEMP_PC_RACK/EB_RACK_1/2의 FDC 온도 인덱스
3. HMI MCC Chart (보고 축 목록)
4. Config_MCC.ini MODULEID/POSITION/FTP 현행 값
5. CIM MCC 원격 명령 스펙 (구식 MCC_ON$ vs GMI식 MCC_FTP_*)
6. HMI MCCLog.exe 바이너리 세대/소스 확보
