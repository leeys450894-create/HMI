# SACC(프로브 자동교체) 구조 분석 및 P5 단일 프로브 · 카세트 7슬롯 대응 플랜

작성 2026-09-16. 대상: OCP 설비(프로브 P5 1개, Probe Cassette 5 → 7슬롯).
관련 문서: `CLAUDE.md`의 "SACC 화면 (dSACC DLL)" 절, `docs/HW_Verification_Checklist.md`.

---

## 1. 현재 구조 요약

### 1.1 구성 요소

| 계층 | 파일 | 역할 |
|---|---|---|
| UI (DLL) | `CoreFunction/dSACC/CDlgSACC.cpp/.h` (약 5,200줄) | SACC 유지보수 화면. 스프레드 4개, 프로브 선택 버튼, LOAD/UNLOAD/EXCHANGE·수동 단계 버튼. **구 시퀀스 구현(`DoWork_*`, `ProcessLoading`, `PLC_Do_*`, `MOTION_Do_*`, `UpdateBtn`)이 그대로 남아 있으나 핸들러에서 도달 불가(죽은 코드)** |
| UI (DLL) | `CoreFunction/dSACC/API_SACC.h` | `PLCRDATA`(`bMData[2000]`, `szData[100][32]`), `RITEMDATA`, `SACC_COMMAND_OPERATION_*`, `plc_PACC_PROBE_DATA_ARRAY` |
| UI (DLL) | `CoreFunction/dSACC/ShareManager.cpp` | 공유메모리 링(10) + 모니터 스레드. 명령을 `WM_CONTRIL_DOWN_EVENT`로 부모에 전달 |
| UI (공통) | `CoreFunction/common/MySpreadDll.cpp` | 스프레드 레이아웃(`InitMySpread_SACCIO/SACCBF/SACCID/InsideProbe`) |
| 앱 브리지 | `Source/ALPHA/Manager/SACCMgr.cpp` | DLL 로드, 50ms 데이터 펌프(`Updata_Data`), **`SACC_COMMAND_OPERATION_*` → `enumPACCSTATUS_*` 변환 + `SetProbe/SetLoadCassette/SetUnLoadCassette` + `Start()`** (`OnControlMsgReceive`). 프로브 장착 이력(`Probe_Mount/Unmount/ContactCount`) |
| 앱 호스트 | `Source/ALPHA/DlgSubManual.cpp` | DLL 다이얼로그 호스트. `ON_MESSAGE(WM_CONTRIL_DOWN_EVENT)` → `CSACCMgr::OnControlMsgReceive`. 기동 시 `stProbeInfo[0..4]` 5개 시드 |
| 시퀀스 | `Source/ALPHA/FunctionModule/FunctionModuleProbeACC.cpp/.h` | 25개 스텝 벡터(`enumPACCStatus`), `ThreadRun`, 데이터 펌프 `GetIOModuleProbeACC`(PLC M비트 2000개 + D영역 문자열 30개 → `PLCRDATA` + 공유메모리) |
| 시퀀스 | `Source/ALPHA/FunctionModule/FunctionModuleProbeACCStep.cpp` | 약 35개 스텝 클래스(아래 1.3) |
| PLC 계층 | `Source/ALPHA/FunctionModule/FunctionModuleEQState.cpp` | `SetEQLoad/UnLoad/Exchange`(M3950 명령워드), `SetEQCommandStart`, `SetEQCompleteOn/Off`, `SetEQGet/Set/ClearPrboeData`(D2100~D2150 문자열), 프로브 유닛 센서 판정 |
| 모션 | `Source/ALPHA/Manager/MaintControlMgr.cpp` | `Move_SelectProbe`, `GetCmdPos_ProbeX`, `MoveCmdPos_CompareMS/ProbeOffset/ProbeMove/ProbeContact`, `Do_PCB/CLAM/Zero_Lock` |
| 설정 | `Source/ALPHA/Manager/PositionConfigMgr.cpp` + `Bin/Config/ProbeChange.ini` | AC X/Z 티칭, 프로브 교체 위치, `PROBE1~5` Safety/오프셋 |
| 자동 트리거 | `Source/ALPHA/DoWork/DoWorkGlassRecipeStep.cpp:355~500` | 레시피 로드 시 레시피 프로브명 ↔ `m_szPROBEID[]` 비교 → 카세트에서 찾아 `LODING/EXCHANGE` 기동 |
| PLC 스캐너 | `CoreFunction/IOMapFunction/McProtocol/McCaddressScanner.cpp` | D2400(Load: EQ Load + CST 1~7), D2210(버퍼 2), D2100(Unload: EQ Unload + Probe Module 1~5) 워드 블록(10워드 = ID 1개) 읽기 |
| PLC 스캐너 | `McPaddressScanner.cpp` + `SACC.csv` | M3900~3990 SACC↔EQ 핸드셰이크 비트 |

### 1.2 데이터·명령 흐름

```
[명령] CDlgSACC 버튼 → RITEMDATA{nEventCmd, nWTSequence=프로브(1~5), nMeasMode=카세트 행(1~7)}
   → CShareManager 링 → ::SendMessage(WM_CONTRIL_DOWN_EVENT) → CDlgSubManual::OnContrilMsgEvent
   → CSACCMgr::OnControlMsgReceive → CFunctionModuleProbeACC::SetPACCStatusChangeStep / SetProbe / Set(Un)LoadCassette / Start

[상태] CDlgMainForm 타이머 → CSACCMgr::Updata_Data → GetIOModuleProbeACC
   bMData[i]  ← M비트 (enum 슬롯 순, DLL은 Source/Include/IOMapAddressDefine.h(0 시작)로 인덱싱)
   szData[]   ← McCaddressScanner::GetAddressExportDataValueArray
                [0..1] 버퍼 / [2] EQ Load / [3..9] CST Slot 1~7 / [12] EQ Unload / [13..17] Probe Module 1~5
   공유메모리  m_szBufferPROBEID[0..1], m_szLoadPROBEID, m_szCassettePROBEID[0..6], m_szPROBEID[4](P5; [0..3]은 OCP면 미기록)
   → API_SACC_Updata_Data → CDlgSACC::OnTimer → UpdateState / UpdateSpreadProbe

[PLC 쓰기] 스텝 → CFunctionModuleEQState → SetMemoryBitToDevice / SetMemoryNBitToDevice / SetMemoryNStrToDevice
```

### 1.3 교체 시퀀스(`enumPACCSTATUS_LODING` 24스텝, 핵심만)

`ProbeChangeMove` → AC/카드 센서 확인 → `ACReadyPosition` → **`SACCGetReq`(카세트 슬롯 번호를 M3950 명령워드 4비트에 실어 PLC에 요청)** → `SACCChangePosition`(AC Z·X4 SACC 위치, EQ BUSY) → `SACCToACReadyPosition` → `SetProbeloadData`(D2110~ 에 ID 기록) → `SACCCompleteReq` → `ProbeChangePosition`(프로브 Z/X/Y 교체 자세) → `ZEROUNLOCK` → `ACZxisUp` → `ZEROLOCK` → `CLAMPLOCK` → `PCBLOCK` → 카드 센서 확인 → `ACZxisDown` → **`Mount`** → `ACReadyPosition` → `Align`(Probe SW Align) → `Safety` → `Idle`.
UNLODING은 거울상(`SACCPutReq`가 언로드 슬롯 사용), EXCHANGE는 `SACCExChangeReq`(두 슬롯) 뒤 언로드→로드 34스텝.

**프로브 인덱스(`iProbe`, 1~5)에 의존하는 스텝**: `ProbeChangePosition`, `ACZxisUp/Down`, `Card*ExistSensor*`, `Align`, `ZERO/CLAMP/PCB LOCK·UNLOCK`, `SetProbeUnloadClear`, `SetProbeloadData`, `Mount/UnMount`, `SACCPutReq/ExChangeReq`, `ProbeChangeMove`.
**카세트 슬롯에 의존하는 스텝**: `SACCGetReq`(Load), `SACCPutReq`(Unload), `SACCExChangeReq`(둘 다) — 모두 `SetEQLoad/UnLoad/Exchange`의 4비트 필드(최대 15)로 전달되므로 **7슬롯은 수치상 문제 없음**.

---

## 2. 현재 반영 상태

### 2.1 P5 단일 프로브 — 이미 `IsOCP()`로 처리된 곳

- `FunctionModuleProbeACC.cpp:727~735` — `m_szPROBEID[0..3]` 기록 스킵, `[4]`만 기록.
- `FunctionModuleEQState.cpp` `IsProbeUnContactSensorOn`(P5 외 false), `IsSafetySensorOnProbe(-1)`(OCP 분기).
- `MaintControlMgr.cpp` — `MoveProbeAlignR`(즉시 성공), 7730/7770/7790 루프 시작을 P5로, `MoveCmdPos_ProbeContact/UnContact/ProbeMove`는 `AXIS_PROBE_Z5` 강제.
- `DoWorkGlassRecipeStep.cpp:360` 프로브 루프 시작 4, `SystemInitializeStep.cpp:2167` 프로브 얼라인 P5만, `DlgSubOperate.cpp:919` P1~P4 표시 공란.
- `dSACC.rc` — `Select Aging Probe 1~4`, `Probe Exist 1~4` 체크박스 숨김(`NOT WS_VISIBLE | WS_DISABLED`).

### 2.2 카세트 7슬롯 — 이미 반영된 곳 (2026-09-16 작업 포함)

- PLC 비트 `PLC_CASSETTE_PROBE_CHK_06/07_1·_2` enum·실사용 `PLCMAdress.csv`(M4475/4485, M4476/4486).
- D영역: `McCaddressScanner` Load 읽기 60→80워드, 파서 블록 6→8, `ProbeCard.csv`에 `D2460/D2470` 라벨. 09-16 11:52 빌드 로그에서 `ReadProbeLoadData(1) : …` 8개 값 정상 수신 확인.
- `FunctionModuleProbeACC.cpp:738~744` `m_szCassettePROBEID[0..6]`, `DoWorkGlassRecipeStep.cpp:441/466` 카세트 루프 7.
- UI: `InitMySpread_SACCID` 7행·행 높이 12.0, `ReadConfigRobotRFID` 뒤 셀 타입 재적용, 클릭 단일선택 루프 전 행, `SelectReadRFID` 루프 `GetMaxRows()`, `.rc` 스프레드 337/90 DLU.
- 배열 크기: `m_szProbeCardID[10]`, `m_bBFProbeSlotStatus[10]`, `m_szCassettePROBEID[10]`, `m_szLoadProbeCardID[10]`, `ProbeID.ini PROBEID1~10` — 모두 7 이상.

---

## 3. 잔여 문제 (file:line)

### 3.1 카세트 7슬롯

| # | 위치 | 내용 | 위험 |
|---|---|---|---|
| C1 | `Source/Bin/Config/.../PLCMAdress.csv:862, 873~876` | **(2026-09-16 처리)** 슬롯 6·7 존재 비트 `CHK_06/07_1·_2` = M4475/4485, M4476/4486 **유지**(한때 9999로 내렸다가 사용자 지시로 복구). `PLC_R_PACC_BUFFER_PROBE_CARD_CHK_02_2`는 09-14 일괄 비활성에서 **M4379로 복구**(스캔값 1 확인, 버퍼 2 프로브 체크에 사용). 배포 템플릿은 같은 날 실사용본으로 덮어써 동일(`cmp` 확인) | 완료 |
| C2 | `CoreFunction/dSACC/API_SACC.h:12~30` `plc_PACC_PROBE_DATA_ARRAY` | `CST_SLOT_5`까지만 있고 값(0~13)이 실제 `szData` 패킹([0..1] 버퍼, [2..9] Load, [12..17] EQ)과 **불일치**. 사용처 9곳(`CDlgSACC.cpp:1759,1824,2285,2345,2387,3517,4011,4020,4084`)은 전부 죽은 함수(`ManualGet/Put*`, `DoWork_*`, `ProcessLoading`, `SelectReadRFID`) | 저(죽은 코드) |
| C3 | `CDlgSACC.cpp:536` `_T("MAXPROBE ")` | 키 이름에 공백이 있어 ini에 `MAXPROBE=7`을 적어도 못 읽고 항상 기본값 7. `ProbeChange.ini` 3사본 어디에도 `[PROBESLOT]` 섹션 없음 | 저 |
| C4 | `SACC.csv:18~25`, `FunctionModuleEQState.cpp:1469/1480/1546/1557/1644/1655` | 슬롯 번호는 4비트(최대 15)라 7은 통과. **PLC가 6·7을 디코딩하는지는 미확인** | 중(PLC 측) |
| C5 | `McCaddressScanner.cpp` Unload 읽기 (`m_szUnloadProbeCardID`) | **(2026-09-16 처리)** D2100 6블록(`dfPROBE_UNLOAD_BLOCK_CNT`) 유지 — [0] EQ Unload Data, [1..5] Probe Module #1~#5 이고 현 설비는 [5](D2150, P5)만 유효. 같은 날 세 가지를 고쳤다: ① 생성자의 더미 `TEST1~TEST5` 제거(첫 응답 전 P1~P5 에 프로브가 있는 것처럼 보였고, 기동 로그 `ReadProbeUnloadData(1) : , TEST1, …` 로 확인됨). ② **빈 ID 정규화** — 이 PLC 는 D2100 영역의 빈 ID 를 공백 10자(0x2020×5워드 + 0x0000×5워드)로 두는데 뒤 5워드가 `'0'` 으로 바뀌어 `TrimRight` 가 공백을 못 지웠다 → `"          "` 이 `m_szPROBEID[4]` 까지 흘러 `IsEmpty()`/`_tcslen()` 판정이 항상 "프로브 있음", Probe Align 레시피명이 `PROBE_MEAS_  ` 가 됐다. 이제 10자 절단 후 `Trim()` 해 `""` 로 저장(Load 쪽 `0000000000` 은 그대로). ③ `GetIOModuleProbeACC` 가 OCP 면 `szProbeInfo[13..16]`(D2110~D2140, P1~P4)을 비워 DLL 스프레드에도 P5 행만 남는다 | 완료 |
| C6 | `CDlgSACC.cpp:4172` `PLC_CASSETTE_PROBE_CHK_01_1 + (2*i)` | i=0..6 인터리브 순서(01_1,01_2,02_1…)가 enum과 일치. 실사용 CSV 기준 정상 | 없음 |

### 3.2 P5 단일 프로브 — `IsOCP()` 없이 5프로브를 가정하는 곳

| # | 위치 | 내용 | 영향 |
|---|---|---|---|
| P1 | `CDlgSACC.cpp` `OnInitDialog` | **(2026-09-16 처리)** `ReadProbeIDtoFile()` 직후 `OnBnClickedBtnSelectProbe5()`를 호출해 기동 시 P5(Meas)가 선택된 상태로 시작. 구 상태는 `m_iSelectProbe = -1`이라 `Select Meas Probe`를 눌러야 LOAD/UNLOAD/EXCHANGE·잠금 버튼이 동작했다 | 완료 |
| P2 | `CDlgSACC.cpp:1243~1291` `OnBnClickedBtnSelectProbe1~5`, 메시지맵 `:143~146`, `UpdateState :4994~5057` case 1~4, `:5086~5089` EXIST2~4 | 숨겨진 버튼의 핸들러·분기가 남아 있음 | 없음(정리 대상) |
| P3 | `MySpreadDll.cpp:169` `InitMySpread_InsideProbe` 5행, `CDlgSACC.cpp:4207~4233` `szData[13+i]` / `PLC_R_PROBE_UNIT01_CARD_CHK_0x + 4*i` | "설비 내부 PROBE ID" 5행 중 4행이 공란. **(2026-09-16)** 앱 `GetIOModuleProbeACC` 가 OCP 면 `szProbeInfo[13..16]` 을 비우므로 1~4행은 PLC 잔존값과 무관하게 항상 빈칸, 5행이 P5(D2150). 행 수 자체를 1로 줄이는 것(A4)은 DLL 이 OCP 를 모르므로 미착수 | 표시만 |
| P4 | `FunctionModuleProbeACCStep.cpp:460~487, 609~636, 1044~1110` 5-way `switch(iProbe)` | P5 케이스가 정상 동작. 1~4는 도달 불가(버튼 숨김) | 없음 |
| P5 | `FunctionModuleProbeACC.h:126` `m_bProbeAligned[5]`, `SetProbeAlign(iProbe-1)` | 범위 검사 없음. P5 → [4] 정상 | 없음 |
| P6 | `MaintControlMgr.cpp:3203~3240`(`CompareMS`), `3271~3308`(`ProbeOffset`), `3330~3434`(`GetCmdPos_ProbeX`), `6946~7012`(`Move_SelectProbe`: `nProbeAxis[5]`, `stPositionProbeY[5]` `AXIS_PROBE_Y1..Y5` 멀티 5축, `stPositionProbeX[5]`) | **비활성 축(X1~X8, Y1~Y4)이 멀티 배열에 실림**. `CMaintControlMgr::MoveOrgPosMulti`의 `PackUsedAxes()`가 걸러내므로 실제 지령은 안 나가지만, `GetCmdPos_ProbeX`의 ±`dProbeGap` 사다리 계산은 P1~P4 값이 무의미 | 저(래퍼가 보호) |
| P7 | `MaintControlMgr.cpp:8199~8256, 8812~8830` `stPositionProbeZ[5]` `AXIS_PROBE_Z1..Z5` + `for i<5 … m_szPROBEID[i] … Probe_ContactCount(i)` | Z1~Z4 지령은 `PackUsedAxes`가 제거. 접촉 카운트는 P1~P4 빈 ID로 호출 | 저 |
| P8 | `FunctionModuleEQState.cpp:732~1121` 5-way `switch(iProbeUnit)` → `PLC_R_PROBE_UNIT01..05_*`, `:2396~2538` D2110~D2150 | P5(=UNIT05, D2150)만 호출됨. `IsSafetySensorOnProbe(-1)`은 OCP 분기 전에 P1~P4 루프를 먼저 `&=` 함(`:1160`) — P1~P4가 OCP에서 true를 돌려주는지 재확인 필요 | 중(확인) |
| P9 | `PositionConfigMgr.h:74~81` `dProbeSafety[5]` 등, `ProbeChange.ini [PROBEOFFSET] PROBE1~5` | P5 값만 쓰임 | 없음 |
| P10 | `SACCMgr.h:12~14`, `DlgSubManual.cpp:113`, `ShareMem.h:2330 m_szPROBEID[5]` | 5개 이력/ID 유지. 공유메모리 ABI라 크기 변경 금지 | 없음 |
| P11 | `SystemInitializeStep.cpp:2209~2258`, `DoWorkGlassAdsorptionStep.cpp:406~455` | `m_szPROBEID[0..4]` 5회 풀어 쓴 읽기 | 없음 |
| P12 | `API_SACC.h:86~90` `SACC_COMMAND_OPERATION_SELECT_PROBE1..5` | 선언만, 미사용 | 없음 |

### 3.3 그 외 발견

- `SEQ SACC_20260916.log`: `CLAMPLOCK Start` 직후 `Thread NG, Start=137, Step=1, Exit=139` — 수동 CLAMP LOCK이 첫 스텝에서 실패(프로브 미선택 또는 센서 조건). P1과 연관.
- CIM·FDC·MCC 어디에도 카세트 슬롯·프로브 ID를 실어 보내는 메시지는 없음(`MCC_A_CHNG_PROBE_SACC_*` 두 코드는 선언만). 외부 인터페이스 변경 불필요.
- `D:\HBS\Logs\...\*_20271001.log` 파일이 존재 — PC 시계가 한때 2027-10-01로 설정됐던 흔적. 날짜별 로그 분석 시 주의.

---

## 4. 수정 플랜

### 단계 A — 즉시(리스크 낮음, 빌드 1회)

| 항목 | 수정 | 파일 |
|---|---|---|
| A1 | **완료(2026-09-16)** — `OnInitDialog`에서 `OnBnClickedBtnSelectProbe5()` 호출로 항상 P5 선택 시작. 미선택 체크는 유지 | `CDlgSACC.cpp` |
| A2 | **완료(2026-09-16)** — 실사용 `PLCMAdress.csv`에서 `PLC_R_PACC_BUFFER_PROBE_CARD_CHK_02_2`를 M4379로 복구, `CHK_06/07_1·_2`는 M4475/4485/4476/4486 유지. 배포 템플릿은 실사용본을 그대로 복사해 `cmp` 동일(구 템플릿 `.bak_20260916_pre_sync`). 재기동 필요(CSV는 기동 시에만 읽음) | `Source/Bin/Config/.../PLCMAdress.csv`, `Source/ALPHA/ConfigurationFile/.../PLCMAdress.csv` |
| A3 | `_T("MAXPROBE ")` → `_T("MAXPROBE")`, `ProbeChange.ini`(실사용·템플릿·DLL Config)에 `[PROBESLOT] MAXPROBE=7` 추가 | `CDlgSACC.cpp:536`, ini 3사본 |
| A4 | `InitMySpread_InsideProbe` 5행 → 1행(OCP) 또는 `m_szPROBEID` 유효 개수만큼. `UpdateSpreadProbe`의 내부 프로브 루프도 같은 개수 | `MySpreadDll.cpp:165~`, `CDlgSACC.cpp:4207~4233` |
| A5 | `plc_PACC_PROBE_DATA_ARRAY`를 실제 패킹에 맞게 재정의(`BUFFER_1=0, BUFFER_2=1, EQ_LOAD=2, CST_SLOT_1..7=3..9, EQ_UNLOAD=12, EQ_PROBE_1..5=13..17`) — 죽은 코드만 참조하므로 동작 변화 없음, 문서화 목적 | `API_SACC.h` |

### 단계 B — PLC 협의 후

| 항목 | 내용 |
|---|---|
| B1 | PLC 담당자에게 확인: (1) M3950 명령워드의 Load/Unload 슬롯 4비트 필드가 6·7을 디코딩하는지. (2) `M4475/M4485`·`M4476/M4486`(슬롯 6·7 존재 센서) 배선 — CSV엔 살아 있음(2026-09-16 복구), 현재 스캔값은 전부 0이라 카세트에 프로브를 넣고 실제 출력 확인 필요. ~~(3) `D2460/D2470` ID 워드~~ → **2026-09-16 사용자 확인: `0000000000` = 빈 슬롯(정상 응답)** |
| B2 | 확인 후 카세트에 프로브를 6·7번에 넣고 SACC 화면 6·7행 노란색(존재)·ID 표시, `Logs\Control\IOControl\LogFile\ProbeCard_*.log`의 8번째 값으로 검증 |
| B3 | 6·7슬롯 대상 LOAD → UNLOAD 1회 실기 테스트. `SEQ SACC_*.log`의 스텝 전이와 `PLC_WRITE_*.log`의 M3950 워드 값(슬롯 비트 0110/0111) 확인 |

### 단계 D — 수동 장착 프로브 ID 등록 (2026-09-16 구현 완료, 빌드·재기동 대기)

구현 결과: D1~D3 아래 표대로 반영. 컨트롤 ID `IDC_EDIT_EQ_PROBEID`(1290) / `IDC_BTN_SET_PROBEID`(1291) / `IDC_BTN_CLEAR_PROBEID`(1292), 위치는 EQ Inside 스프레드 바로 아래(0,218). `RITEMDATA` 끝에 `szText[32]` 추가(양쪽 `API_SACC.h` 동일, `cmp` 확인). 앱 쪽 로그는 모두 `Logs\LogFile\SEQ SACC_*.log`: 등록 시 `SACC SET_PROBEID probe=5 D2150 id=[..] -> OK|NG`(`CSACCMgr` 가 ProbeACC accessor 로 기록 — `CFunctionModuleEQState` 자체 accessor 는 `Initialize` 되지 않아 쓰지 않음), 차단 시 `SACC unload/exchange blocked - EQ probe ID empty …` WARN. DLL 쪽은 SACC 화면 이벤트 로그에 `Manual probe ID SET|CLEAR : probe=5 id=[..]`. 백업 `.bak_20260916_probeid`.

배경: 수동으로 장착한 프로브는 D2150(P5 EQ Probe ID)이 비어 있고, 언로드 시 `SACCPutReq`/`SACCExChangeReq` 가 `m_szPROBEID[4]` 를 D2100 에 써 주면 **PLC 가 그 값을 로봇 동작에 쓴다(사용자 확인)**. 빈 값이 가면 안 되므로 등록 경로가 필요하다.

| 항목 | 내용 | 파일 |
|---|---|---|
| D1 | SACC 화면 EQ Inside 스프레드 옆에 ID 입력칸(10자) + `SET ID` / `CLEAR ID` 버튼. 클릭 시 `RITEMDATA.nEventCmd = SACC_COMMAND_OPERATION_SET_PROBEID`(신규, enum 2사본 동시 추가), `nWTSequence = 5`(P5), ID 는 `stDataP.szData[0]`(기존 구조체의 문자열 슬롯 재사용) 로 `UploadData` | `dSACC.rc`, `CDlgSACC.cpp/.h`, `API_SACC.h` ×2 |
| D2 | 앱 `CSACCMgr` 디스패치에 case 추가 → `CFunctionModuleEQState::SetEQGetPrboeData` 와 같은 경로로 `SetMemoryNStrToDevice(_T("D2150"), id, 10)`. CLEAR 는 공백 10자. 시퀀스 스레드가 아니라 직접 1회 쓰기라 `Start()` 불필요 | `SACCMgr.cpp`, `FunctionModuleEQState.cpp`(전용 함수 1개) |
| D3 | 보호: `SACCPutReq`/`SACCExChangeReq` 진입 시 `m_szPROBEID[4]` 가 비어 있으면 D2100 을 쓰지 않고 실패 + SYST 로그(`Unload probe ID empty - register ID first`). 언로드가 빈 ID 로 PLC 에 가는 것을 구조적으로 막는다 | `FunctionModuleProbeACCStep.cpp` |
| D4 | 확인: 스캐너 5초 주기 재읽기 후 `RECV ProbeUnloadData(...) : [..], …, [등록ID]` 로그와 화면 5행 표시 | 로그 |

운영 절차(구현 후): 수동 장착 → SACC 화면에서 ID 입력·SET → 5초 내 5행에 표시 확인 → 이후 언로드/교체는 평소대로. 수동 탈착 시 CLEAR. 레시피 `_PROBE_ID_` 5번째 값을 쓸 경우 등록 ID 와 같아야 자동 사이클의 프로브 검사가 통과한다.

### 단계 C — 정리(선택, 동작 변화 없음)

| 항목 | 내용 |
|---|---|
| C1 | `CDlgSACC.cpp`의 도달 불가 코드 제거 또는 `#if 0`: `DoWork_*`, `ProcessLoading`, `ManualGet/Put*`, `PLC_Do_*`, `MOTION_Do_*`, `UpdateBtn`, `SelectReadRFID`+`WM_MSG_READ_RFID`. 약 2,000줄 감소, `plc_PACC_PROBE_DATA_ARRAY` 사용처 소멸 |
| C2 | `OnBnClickedBtnSelectProbe1~4`, `UpdateState` case 1~4, `IDC_CHK_INSIDE_PROBE_EXIST2~4` 처리 제거(리소스는 유지해도 무방) |
| C3 | `Move_SelectProbe`/`GetCmdPos_ProbeX`/`MoveCmdPos_ProbeOffset`에 `IsOCP()` 분기 추가: P5 축(X9/Y5/X10)만 배열에 싣기. 현재는 `PackUsedAxes`가 보호하지만 의도를 코드에 드러냄 |
| C4 | `IsSafetySensorOnProbe(-1)`의 P1~P4 루프를 OCP면 건너뛰도록 정리(P8 확인 결과에 따라) |

### 하지 말아야 할 것

- `ShareMem.h`의 `m_szPROBEID[5]`, `m_szCassettePROBEID[10]`, `SACCMgr`의 5개 이력 배열 크기 변경 — 공유메모리·외부 프로세스 ABI.
- `dfMAX_PLC_IN_CNTD`의 다른 정의(`CommonFunc.h:151`, `DefinesSys.h:62`, `FDCSense/DlgPLCDRead.cpp:24`) — 별개 서브시스템.
- `PLCMAdress.csv` 중간 행 삽입 — positional 계약(`CLAUDE.md` PLC 절).

---

## 5. 검증 체크리스트

| 확인 | 방법 |
|---|---|
| 카세트 6·7행 UI | SACC 화면: ID 텍스트, LOAD/UNLOAD 체크박스, 존재 시 노란 배경. 클릭 시 다른 행 체크 해제 |
| D영역 8블록 수신 | `ProbeCard_yyyymmdd.log` `ReadProbeLoadData(1) : v1, …, v8`(값 8개). `Rcv TimeOut` 반복이면 PLC D2460~D2479 미할당 |
| 슬롯 6·7 존재 비트 | `PLC_READ_*.log` M영역 워드에서 M4475/4476/4485/4486 (워드 29·30, LSB=낮은 주소) |
| 명령워드 | `PLC_WRITE_*.log`의 M3950 쓰기: `000 001 0000 0110 01`(Load 슬롯 6) 형태 |
| 시퀀스 | `Logs\LogFile\SEQ SACC_*.log` 스텝 전이, `Thread NG` 없음 |
| 프로브 선택 기본값 | 기동 직후 `Select Meas Probe` 없이 CLAMP LOCK 버튼이 동작(P1 적용 후) |
