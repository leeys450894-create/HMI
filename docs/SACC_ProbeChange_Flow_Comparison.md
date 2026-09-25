# SACC Probe Change Flow — PPT(슬라이드 2) ↔ 코드 대조

작성 2026-09-11, **2026-09-25 재작성**(초판 파일이 트리 동기화 과정에서 사라져 복원; D8 정정 반영). 대조 대상:

- **PPT**: `Downloads\SDC_A3_HMI_OCP 설비 측정 시나리오.pptx` **슬라이드 2** — "SDC A3 HMI 개조 Main EQ ↔ SACC 운영 Flow Map (Online Mode 시) / Online 측정 시 (With Probe Change)"
  - (참고) 슬라이드 1은 동일 포맷의 "Without Probe Change" 버전
- **코드**: `D:\OCP\0.SW_Matching\HMI` (PC-B 실사용 트리; PC-A는 `D:\0.SW\HMI`)
- **런타임 근거**: `D:\HBS\Logs\LogFile\GLASS Sequence_Step_20260907.log` (2026-09-07 OOS 사이클, Long 레시피 5스틱 완주)

> PPT 슬라이드에는 커넥터 도형이 없고 **회전된 downArrow**로만 흐름을 표시한다. 화살표 회전각(rot 0/180/270)과 도형 좌표로 판독한 결과 **5열 뱀(boustrophedon) 배치**였다:
> 열A↓ → (하단)우 → 열B↑ → (상단)우 → 열C↓ → (하단)우 → 열D↑ → (상단)우 → 열E↓

---

## 1. PPT 플로우 (판독 결과, 20단계)

| # | 열 | PPT 박스 |
|---|---|---|
| 1 | A | Load State |
| 2 | A | CIM → Main Glass Info 전달 |
| 3 | A | Main Judge : Need Probe Change |
| 4 | A | AC Gantry Move (Safety Position) |
| 5 | B | SACC Probe Change Sequence 시작 |
| 6 | B | Buffer에서 SACC ARM이 Probe 취득 |
| 7 | B | Put Probe Change Buffer (elevator) |
| 8 | B | AC Gantry Move (Probe Change Position) |
| 9 | C | SACC Probe Change Buffer(elevator) Move(Down) |
| 10 | C | AC Gantry - Get Probe |
| 11 | C | AC Gantry Move (Probe Loading Position) |
| 12 | C | AC Z축 Up |
| 13 | D | Zero Lock & PCB Lock & Clamp On |
| 14 | D | AC Z축 Down |
| 15 | D | Measure State 변환시작 |
| 16 | D | AC Gantry Move (Probe Change Position) |
| 17 | E | Measure Sequence |
| 18 | E | Glass Unloading Position |
| 19 | E | AC Gantry Move (Safety Position) |
| 20 | E | Glass Unloading Sequence |

---

## 2. 코드 실제 플로우

### 2.1 Glass 9-item 시퀀스 내 위치 (2026-09-07 로그로 실증된 순서)

```
GLASS 1_ADSORPTION    ─ _Move → _ChangeMeasStatus(★ MEASURE 전환) → _CheckMeasStatus → _ProbeAlign
GLASS 2_RECIPE_CHANGE ─ _Change(레시피 배포) → _ProbeCheck(★ Probe 교체 판정 + PACC 기동 + AC→SACC 이동)
GLASS 3_GLASS_ALIGN
GLASS 3_PROBE_ALIGN   ─ (★ 교체 여부와 무관하게 매 글래스 수행)
GLASS 4_PRE_MEAS → 5_AGING → 6_MEASURE → 7_RESULT → 8_UNLOAD
```

- 교체 판정: `CDoWorkGlassRecipeStep_ProbeCheck::Run()` — [DoWorkGlassRecipeStep.cpp:342](../Source/ALPHA/DoWork/DoWorkGlassRecipeStep.cpp)
  레시피 `m_sProbeInfo.m_chProbeFirst/Second[iProbe]` ↔ 공유메모리 `m_szPROBEID[iProbe]` 비교.
  **OCP는 P5만** (`nProbeStart_RC = IsOCP() ? 4 : 0`, :360).
  Card 프로브 유무(`PLC_R_PROBE_UNIT05_CARD_CHK_01~04`)로 **Loding / Exchange** 분기 (:437).

### 2.2 PACC(Probe Auto Change) 서브 시퀀스 — Loding (Card에 프로브 없음)

[FunctionModuleProbeACC.cpp:111-145](../Source/ALPHA/FunctionModule/FunctionModuleProbeACC.cpp)

| # | 스텝 | 실제 동작 |
|---|---|---|
| 1 | `_ProbeChangeMove` | Probe Unit X/Y를 교체 위치로 선이동 |
| 2 | `_ACProbeExistSensorOff` | AC에 프로브 없음 확인 |
| 3 | `_CardProbeExistSensorOff` | Card에 프로브 없음 확인 |
| 4 | `_ACReadyPosition` | **AC Y(X4) → Safety(-1380), AC Z(Z9) → 0** |
| 5 | `_SACCGetReq` | `PLC_PACC_AUTO_MODE`·`PACC_ALL_SEQ_DONE` 대기 → `SetEQLoad(카세트)` |
| 6 | `_SACCChangePosition` | `ENTRY_REQUEST` 대기 → 셔터 확인 → **Buffer Load/Unload Pos 확인** → Probe Z → **AC Z→66, AC Y→SACC(-1581.69)** → `IsPositionOnAC(2)` |
| 7 | `_SACCToACReadyPosition` | `HANDFOLD_COMPLETE_ACK` 대기 → Buffer Pos Done 확인 → **AC Y→Safety(-1380)** → `IsPositionOnAC(1)` → AC Z→0 |
| 8 | `_SetProbeloadData` | Probe Load 데이터 기록 |
| 9 | `_SACCCompleteReq` | SACC에 완료 통보 |
| 10 | `_ACProbeExistSensorOn` | AC에 프로브 있음 확인 |
| 11 | `_CardProbeExistSensorOff` | Card 비어있음 재확인 |
| 12 | `_ProbeChangePosition` | Probe Z 교체위치 → **AC Y→Change(+263.5)** → `IsPositionOnAC(0)` → `Move_SelectProbe` |
| 13 | `_ZEROUNLOCK` | **Zero Unlock (Z Up 전 선행)** |
| 14 | `_ACZxisUp` | AC Z Up |
| 15 | `_ZEROLOCK` | Zero Lock |
| 16 | `_CLAMPLOCK` | Clamp Lock |
| 17 | `_PCBLOCK` | PCB Lock |
| 18 | `_CardProbeExistSensorOn` | Card에 프로브 장착 확인 |
| 19 | `_CardExistProbe_ZMOVE` | AC Z 이동 + 프로브 분리 확인 |
| 20 | `_ACZxisDown` | AC Z Down |
| 21 | `_Mount` | `CSACCMgr::Probe_Mount()` — ProbeID/장착시각 이력 기록 |
| 22 | `_ACReadyPosition` | **AC → Safety 복귀** |
| 23 | `_Align` | `SEQUENCE_ITEM_DWGLASS_SEQ_PROBE_SW_ALIGN` 호출 (Probe Align) |
| 24 | `_Safety` | EQ Safety 상태 확인/전환 |

**Exchange** (Card에 기존 프로브 있음)은 앞에 반납 반쪽이 추가된다:
`_SACCExChangeReq` → `_ProbeChangePosition` → `_ACZxisUp` → `_PCBUNLOCK` → `_CLAMPUNLOCK` → `_ZEROUNLOCK` → `_ACZxisDown` → `_UnMount` → `_ZEROLOCK` → `_ACReadyPosition` → SACC 반납(`_SACCChangePosition`/`_SACCToACReadyPosition`) → `_SetProbeUnloadClear` → `_WaitForMoveACZDown` → 이후 위 장착 반쪽.

### 2.3 AC Gantry 위치 정의 (실측값 — PC-B `Bin/Config/ProbeChange.ini`, 2026-09-11)

| 코드 | ini 키 | 값 (AXIS_X4) | AC Z (AXIS_Z9) | 위치 센서 |
|---|---|---|---|---|
| `dACPosition[0]` Change | `[ACXPOS] POS2` | **+263.500** | 96.0 | `IsPositionOnAC(0)` |
| `dACPosition[1]` Safety | `[ACXPOS] POS3` | **-1380.000** | 0.0 | `IsPositionOnAC(1)` |
| `dACPosition[2]` SACC | `[ACXPOS] POS4` | **-1581.6874** | 66.0 | `IsPositionOnAC(2)` |

즉 `Change(+263.5)`는 **EQ 내측(프로브 유닛 쪽)**, `SACC(-1581.7)`는 **가장 바깥(SACC 쪽)**, `Safety(-1380)`는 그 사이. (PC-A ini는 Safety −1402.6 등 값이 다르다 — 스킬 `hmi-head-gantry-interlock` 참조. 순서·의미는 동일.)

---

## 3. 단계별 대조

| # | PPT 단계 | 코드 대응 | 판정 |
|---|---|---|---|
| 1 | Load State | `enumEQStateLoad` | ✅ |
| 2 | CIM → Main Glass Info | `DoWorkSequenceCIMStep` | ✅ |
| 3 | Main Judge Need Probe Change | `_ProbeCheck` (레시피 ↔ 장착 ProbeID 비교) | ✅ |
| 4 | AC Gantry Move (Safety Position) | `_ACReadyPosition` → X4 = Safety | ✅ |
| 5 | SACC Probe Change Sequence 시작 | `_SACCGetReq` → `SetEQLoad()` | ✅ |
| 6 | Buffer에서 SACC ARM이 Probe 취득 | SACC 내부 (EQ 코드 없음) | ✅ 정보성 |
| 7 | Put Probe Change Buffer (elevator) | SACC 내부 | ✅ 정보성 |
| 8 | AC Gantry Move (**Probe Change** Position) | `_SACCChangePosition` → X4 = **SACC(POS4)** | ⚠️ **D6** 명칭 충돌 |
| 9 | Buffer(elevator) Move(Down) | `IsBufferLoadPos()/IsBufferUnloadPos()` — **AC 이동 전 선행조건** | ⚠️ **D9** 순서 상충 |
| 10 | AC Gantry - Get Probe | `_SACCToACReadyPosition`(Safety 복귀) + `_ACProbeExistSensorOn` | ⚠️ **D7** 중간단계 누락 |
| 11 | AC Gantry Move (**Probe Loading** Position) | `_ProbeChangePosition` → X4 = **Change(POS2)** | ⚠️ **D6** 명칭 충돌 |
| 12 | AC Z축 Up | `_ACZxisUp` — 단 직전에 `_ZEROUNLOCK` 있음 | ⚠️ **D5** |
| 13 | Zero Lock & PCB Lock & Clamp On | `_ZEROLOCK` → `_CLAMPLOCK` → `_PCBLOCK` | ⚠️ **D5** 순서 불일치 |
| 14 | AC Z축 Down | `_CardProbeExistSensorOn` → `_CardExistProbe_ZMOVE` → `_ACZxisDown` | ⚠️ **D7** |
| — | (없음) | `_Mount` / `_ACReadyPosition` / `_Align` / `_Safety` | ❌ **D2** PPT 누락 |
| 15 | Measure State 변환시작 | `_ChangeMeasStatus` — **GLASS 1_ADSORPTION(교체보다 먼저)** | ❌ **D1** 순서 역전 |
| 16 | AC Gantry Move (Probe Change Position) | `_ProbeCheck` 말미 → **SACC(POS4)** (2026-09-08 추가) | ❌ **D3** |
| 17 | Measure Sequence | GLASS 4~6 | ✅ |
| 18 | Glass Unloading Position | GLASS 8_UNLOAD | ✅ |
| 19 | AC Gantry Move (Safety Position) | **해당 이동 없음** | ❌ **D3** |
| 20 | Glass Unloading Sequence | `DoWorkGlassUnloadStep` (M2L/R2L) | ✅ |

---

## 4. 불일치 및 조치 필요 항목

### D1 — Measure State 전환 시점이 역전되어 있음 (최우선)

- **PPT**: 프로브 교체 완료(14) → **Measure State 변환(15)** → 측정
- **코드**: `GLASS 1_ADSORPTION`의 `_ChangeMeasStatus`([DoWorkGlassAdsorption.cpp:36](../Source/ALPHA/DoWork/DoWorkGlassAdsorption.cpp), [Step:197](../Source/ALPHA/DoWork/DoWorkGlassAdsorptionStep.cpp))가 READY/LOAD → **MEASURE 전환을 먼저** 수행하고, 그 다음 `GLASS 2_RECIPE_CHANGE`에서 프로브를 교체
- 2026-09-07 `GLASS Sequence_Step` 로그에 `1_ADSORPTION` → `2_RECIPE_CHANGE` 순서로 실증됨
- **결과: 실제로는 MEASURE 상태에서 프로브 교체가 일어난다.** 게다가 PACC 마지막 `_Safety` 스텝이 `enumEQSTATUS_SAFETY` 전환을 호출하므로([FunctionModuleProbeACCStep.cpp:1483](../Source/ALPHA/FunctionModule/FunctionModuleProbeACCStep.cpp)) MEASURE → SAFETY 상태 왕복이 추가로 발생

**조치안**
- (A) 코드 수정 — `_ProbeCheck`를 `_ChangeMeasStatus` 이전으로 이동. 글래스 시퀀스 아이템 순서를 바꾸는 작업이라 **영향 범위가 크다**
- (B) PPT 수정 — 현행대로 "Measure State 변환 → Probe Change"로 표기

→ **어느 쪽이 합의된 사양인지 고객 확정 필요.**

### D2 — Probe Align 단계가 PPT에 없음

- 코드는 Loding/Exchange 말미에 `_Align`([FunctionModuleProbeACCStep.cpp:1000](../Source/ALPHA/FunctionModule/FunctionModuleProbeACCStep.cpp))으로 `SEQUENCE_ITEM_DWGLASS_SEQ_PROBE_SW_ALIGN`을 호출한다. 프로브 교체는 Probe Align 오프셋·M/S 스큐를 지우므로 **교체 후 Align은 필수**
- 추가로 글래스 시퀀스 자체에 `GLASS 3_PROBE_ALIGN` 아이템이 별도로 있어, **교체가 발생한 글래스에서는 Probe Align이 2회 수행**된다
- **조치**: PPT에 Probe Align 단계 추가. 2회 중복 수행이 의도인지 코드 측 확인(택트 손해)

### D3 — 측정/언로딩 중 AC Gantry 파킹 위치가 PPT와 다름

- **PPT(슬라이드 1·2 공통)**: 측정 중 `Probe Change Position`, 언로딩 전 `Safety Position`
- **코드(2026-09-08 추가, [DoWorkGlassRecipeStep.cpp:538-580](../Source/ALPHA/DoWork/DoWorkGlassRecipeStep.cpp))**: `_ProbeCheck` 성공 시 **교체 발생 여부와 무관하게 매 글래스마다** AC를 **SACC 위치(POS4)**로 이동하고 `PLC_AUTO_CHANGE_X0_POSITION_04` 비트를 세트한다. 이후 측정·언로딩 구간에 AC 이동 없음
- 진입 조건으로 `PLC_AUTO_CHANGE_X0_POSITION_03`(Safety 위치) 비트가 ON이어야 하며, **아니면 `MOTION_INTERLOCK_9`로 글래스 시퀀스 전체가 실패**한다
- 좌표상 `Change(POS2)`는 EQ 내측이라 측정 중 파킹 위치로는 부적절해 보임 → **PPT의 "Probe Change Position" 표기가 오기일 가능성**이 높으나, 슬라이드 1·2에 동일하게 적혀 있어 단순 오타로 단정할 수 없음
- **조치**: 측정 중 AC 파킹 위치를 SACC(POS4)로 확정할지 결정. 확정 시 PPT 16/19번 박스를 수정하고, 19의 "언로딩 전 Safety 이동"은 코드에 없으므로 **삭제하거나 코드에 추가**
- 관련 주의: 이 코드가 의존하는 `PLC_AUTO_CHANGE_X0_POSITION_03/04`는 2026-09-03 PLCMAdress.csv 삽입 사고로 한 칸 밀려 오매핑된 전례가 있는 비트다(스킬 `hmi-plc-address`)

### D4 — 기존 프로브 반납(Exchange) 경로 전체 누락

- PPT는 **장착만** 묘사 → 코드 `m_vecSeqStep_Loding`(Card가 비어 있을 때)에만 대응
- 실제 온라인 교체는 대부분 `m_vecSeqStep_Exchange`(기존 프로브 반납 + 신규 장착). 분기는 `_ProbeCheck`가 Card 프로브 센서로 판정([DoWorkGlassRecipeStep.cpp:437](../Source/ALPHA/DoWork/DoWorkGlassRecipeStep.cpp))
- **조치**: PPT에 Exchange 분기(§2.2 하단 스텝 목록) 추가

### D5 — Lock 순서 불일치 / Zero Unlock 누락

| | 순서 |
|---|---|
| PPT | (Z Up) → Zero Lock & **PCB** Lock & **Clamp** On → (Z Down) |
| 코드 | **`_ZEROUNLOCK`** → Z Up → Zero Lock → **Clamp** Lock → **PCB** Lock → (센서/ZMOVE) → Z Down |

- Z Up **전의 Zero Unlock**이 PPT에 없고, Lock 3종 순서도 Clamp/PCB가 뒤바뀌어 있다
- **조치**: PPT 표기 수정. 순서가 기구상 의미를 갖는다면 코드 순서 재확인 필요

### D6 — 위치 명칭이 코드와 정반대 (용어 충돌 — 재티칭 사고 위험)

| PPT 용어 | 실제 위치 | 코드 용어 |
|---|---|---|
| "Probe Change Position" | POS4 (SACC 쪽) | `SACCChangePosition` / `dACPosition[2]` |
| "Probe Loading Position" | POS2 (EQ 프로브 유닛 쪽) | `ProbeChangePosition` / `GetProbeChangePos` / `dACPosition[0]` |

같은 단어("Probe Change")가 양쪽에서 **정반대 위치**를 가리킨다. 용어 통일 권고 (예: PPT를 `SACC Transfer Position` / `Probe Mount Position`으로 변경).

### D7 — 센서/확인 스텝 및 중간 이동 미표기

PPT에 없는 코드 스텝: `_ACProbeExistSensorOn/Off`, `_CardProbeExistSensorOn/Off`, `_CardExistProbe_ZMOVE`, `_SACCToACReadyPosition`(SACC↔Safety 왕복), `_SetProbeloadData`/`_SetProbeUnloadClear`, `_SACCCompleteReq`, `_Mount`/`_UnMount`(`CSACCMgr::Probe_Mount/Unmount` 이력 기록).

특히 **PPT 10번(AC Gantry - Get Probe) 직후 코드는 AC를 Safety로 되돌린 뒤** 프로브 유무를 확인한다. PPT에 이 복귀 단계가 없어 AC가 SACC 위치에서 바로 Loading 위치로 가는 것처럼 읽힌다.

### D8 — 대상 프로브가 카세트에 없으면 사이클 FAIL (PPT에 실패 분기 없음)

`_ProbeCheck`는 `m_szCassettePROBEID[0..6]`(7개)에서 레시피가 요구하는 ProbeID를 찾지 못하면 `bRunStart = false`로 PACC를 기동하지 않고, 직전에 넣어 둔 `nRet = VTM_RV_FAILED`([:493](../Source/ALPHA/DoWork/DoWorkGlassRecipeStep.cpp))가 그대로 남아 `break` → `SetFail(true, nRet)`([:529](../Source/ALPHA/DoWork/DoWorkGlassRecipeStep.cpp))로 **글래스 사이클이 실패로 끝난다**. (2026-09-25 정정 — 초판에 "알람 없이 skip"으로 적었던 것은 오독. 단 실패 코드가 범용 `VTM_RV_FAILED`라 어떤 알람으로 표시되는지는 `Alarm.xls` 확인 필요.) PPT에는 이 실패 분기가 없다.

**조치**: PPT에 "카세트 미보유 → 사이클 FAIL" 분기 표기. 전용 알람 ID 부여 여부는 별도 결정.

### D9 — Buffer(elevator) Down 시점 (확인 필요, 단정 불가)

- **PPT**: AC Gantry 이동(8) → Buffer Move Down(9)
- **코드**: `_SACCChangePosition` **진입 조건**으로 `IsBufferLoadPos()/IsBufferUnloadPos()`를 먼저 확인한 뒤 AC를 이동하고, `_SACCToACReadyPosition`에서 `...PosDone()`을 재확인
- SACC 측 Buffer 상태 비트(`PLC_PACC_LOAD/UNLOAD_BUFFER_POS_HIGH/LOW`)의 의미(High/Low = Up/Down?)를 SACC 사양서와 대조해야 순서 상충 여부를 확정할 수 있다

### D10 — 슬라이드 1의 "Safety Encoder Off/On" 미구현 (참고)

슬라이드 1에는 `[Processing] EQ State Busy On / Safety Encoder Off` … `[IDLE] EQ State Busy Off / Safety Encoder On` (EQ → PLC) 주석이 있다.
- `EQ State Busy On/Off` → 코드 `SetStatusbusyOn()/SetStatusbusyOff()` 존재 ✅ (2026-09-10부터 `PLC_BIT_W_EQ_BUSY_ON` M4982 주기 출력도 있음 — 스킬 `hmi-plc-address`)
- **`Safety Encoder Off/On` → 코드에 대응 구현 없음** (전체 소스 grep 무결과) ❌

신규 요구사항인지, 다른 이름의 기존 비트인지 확인 필요.

---

## 5. 요약

| 구분 | 건수 | 항목 |
|---|---|---|
| 사양 확정 필요 (코드 변경 가능성) | 3 | D1(Measure 전환 시점), D3(AC 파킹 위치), D10(Safety Encoder) |
| PPT 보완 필요 | 5 | D2(Probe Align), D4(Exchange 분기), D5(Lock 순서), D7(센서·중간이동), D8(FAIL 분기) |
| 용어 정리 | 1 | D6(Probe Change / Probe Loading 명칭 충돌) |
| SACC 사양 대조 필요 | 1 | D9(Buffer Down 시점) |

**코드는 수정하지 않았다.** D1·D3은 글래스 시퀀스 순서와 실기 파킹 위치를 바꾸는 작업이라 고객 사양 확정 후 착수해야 한다. PPT 보완안 파일: `Downloads\SDC_A3_HMI_OCP_ProbeChange_Flow_보완안_20260911.pptx`(D8 문구 2026-09-25 정정판).
