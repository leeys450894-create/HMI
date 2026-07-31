# 03. ALPHA HMI Auto-Run Sequence Flow Analysis

## 1. Overview (개요)

ALPHA HMI의 자동 검사 시퀀스는 Glass 디스플레이 패널의 자동화 검사를 수행하는 핵심 플로우이다.
전체 구조는 **CSequenceManager(Singleton)** → **CDoWorkGlassSequence** → **CDoWorkGlassSequenceEngine** → 9개 **CDoWorkGlassSequenceImpl** 파생 클래스로 구성된다.

### Architecture Diagram

```
CSequenceManager (Singleton)
 ├─ CSystemInitialize         → 22-step 초기화 (별도 시퀀스)
 ├─ CDoWorkGlassSequence      → Auto-Run 시퀀스 래퍼
 │   └─ CDoWorkGlassSequenceEngine (Thread)
 │       ├─ [0] CDoWorkGlassAdsorption   → Stage/Measure 준비
 │       ├─ [1] CDoWorkGlassRecipe       → Recipe 변경 및 장비 설정
 │       ├─ [2] CDoWorkGlassSWAlign      → Glass Align
 │       ├─ [3] CDoWorkProbeSWAlign      → Probe Align
 │       ├─ [4] CDoWorkGlassPreMeas      → 사전 검사 (Contact, OST, R-Test)
 │       ├─ [5] CDoWorkGlassAGING        → Aging 검사
 │       ├─ [6] CDoWorkGlassMeasure      → 본 검사 (WAD/VISION/COLOR/ANGLE/TSP)
 │       ├─ [7] CDoWorkGlassResult       → 결과 저장 및 판정
 │       └─ [8] CDoWorkGlassUnload       → Safety 이동 및 CIM 통보
 ├─ CFunctionModuleEQStateChange → 설비 상태 전환 (R2L, L2M, M2R, etc.)
 ├─ CFunctionModuleEQState       → 설비 상태/센서 체크
 ├─ CDoWorkSequenceCIM           → CIM 통신 시퀀스
 └─ CFunctionModuleProbeACC      → SACC Probe Auto Change
```

### Base Class Hierarchy

```
CBaseSequenceBody
 └─ CBaseSequenceImpl (Thread + Start/Stop/Pause/Resume/Complete events)
     ├─ CDoWorkGlassSequenceImpl (DoWork 서브클래스 base)
     │   ├─ CDoWorkGlassAdsorption, CDoWorkGlassRecipe, ...
     │   └─ CDoWorkGlassMeasure, CDoWorkGlassResult, CDoWorkGlassUnload
     ├─ CSystemInitialize
     ├─ CDoWorkGlassSequence
     ├─ CFunctionModuleEQStateChange
     └─ CFunctionModuleEQState
```

### Key Files

| File | Location | Purpose |
|------|----------|---------|
| SequenceManager.cpp/.h | `Source/ALPHA/Manager/` | Singleton 시퀀스 총괄 |
| DoWorkGlassSequenceEngine.cpp/.h | `Source/ALPHA/DoWork/` | 9개 시퀀스 아이템 엔진 (Thread) |
| DoWorkGlassSequence.cpp/.h | `Source/ALPHA/DoWork/` | 시퀀스 엔진 래퍼 |
| BaseSequenceImpl.cpp/.h | `Source/ALPHA/` | Thread + Event 기반 시퀀스 base |
| BaseSequenceBody.h | `Source/ALPHA/` | Step Index, Pause/Resume, Manager 포인터 |
| SystemInitialize.cpp/.h | `Source/ALPHA/DoWork/` | 22-step 시스템 초기화 |
| FunctionModuleEQState.cpp/.h | `Source/ALPHA/FunctionModule/` | 설비 상태/센서 쿼리 |
| FunctionModuleEQStateChange.cpp/.h | `Source/ALPHA/FunctionModule/` | 설비 상태 전환 실행 |

---

## 2. Sequence Engine Execution Model (시퀀스 엔진 실행 모델)

### 2.1 Engine Thread (`CDoWorkGlassSequenceEngine::ThreadRun()`)

**파일:** `DoWorkGlassSequenceEngine.cpp` (line 576~828)

1. `CMCCLogMgr::SetMCC_Action(MCC_A_AUTO_CYCLE, true)` 시작
2. Glass ID / Recipe ID 취득
3. Cycle Count 결정: OFFLINE이면 `CSystemParamShare::GetCycleCount()`, ONLINE이면 1
4. Cycle 루프 (`for index = 0..nCycleCount-1`):
   - TactTime 초기화 (`SetDataClear`, `SetTotalSeqTimeST`)
   - `ThreadRu_nSequence(cycle_index, bFirst)` 호출
   - CCD LIVE 시작 메시지 전송
   - TactTime 기록 (`SetCycleTimeED`, `SetTotalSeqTimeED`)
   - 실패 시 break
5. 완료 시 `WM_MSG_UPDATE_CYCLE_INSPECTION_COMPLETE` 메시지 전송

### 2.2 Sequence Item Execution (`ThreadRu_nSequence()`)

**파일:** `DoWorkGlassSequenceEngine.cpp` (line 830~928)

9개 아이템을 순차 실행 (첫 사이클이 아닌 경우 index 1부터 시작 = Adsorption 스킵):

```
for (i = nStart; i < nEnd; ++i) {
    m_vecItems[i]->Start();           // GoToInitStep + SetEvent(StartEvent)
    m_vecItems[i]->SetLoopStepIndex(nStick);  // Stick 수 설정
    
    WaitForSingleObject(CompleteEvent, INFINITE);  // 완료 대기
    
    if (GetFail()) → AlarmController::SetAlarm → return false
    if (GetStopUser()) → return false
    if (!GetComplete()) → return false
}
```

### 2.3 Common Step Pattern (공통 Step 실행 패턴)

모든 DoWork 클래스는 동일한 패턴으로 ThreadRun을 구현:

```cpp
GoToInitStep();
for (i = 0; i < m_vecSeqStep.size(); ++i) {
    SetBackupStepIndex(GetCurStep());
    SeqTactTimeMgr->SetSeqItemTimeST(...);     // 시작 시간 기록
    
    if (pSeqItem->Run() == true)               // 성공
        GoToNextStep();
    else if (GetCurStep() == GetExitStepIndex()) // Exit Step
        return;
    else {                                      // NG
        GoToExitStep();
        GetSeqItemStep()->Run();               // Idle Step 실행
        return;
    }
    
    if (GetStopUser() == true)                 // 사용자 정지
        GoToExitStep() → Idle → return;
}
```

### 2.4 Stop/Pause/Resume Flow

- **EngineStop()** (line 517): `m_bEngineBreak = true`, `UserStopState(true)`, `SetEvent(hBreakEvent)`
- **EnginePause()** (line 498): `SuspendThread()`, `SetEvtPause()` for all items
- **EngineResume()** (line 479): `ResumeThread()`, `SetEvtResume()` → Resume all recipe items
- **IsThreadStopState()** (line 954): `IsCycleState() == false || m_bEngineBreak == true`

---

## 3. System Initialize (시스템 초기화)

**클래스:** `CSystemInitialize`
**파일:** `SystemInitialize.cpp` (line 19~201), `SystemInitializeStep.cpp` (line 1~1985)
**SeqItemID:** `enumSeqItemID_INITIALIZE`

### 3.1 Step 구성 (22 Steps)

| Step | Class | Description |
|------|-------|-------------|
| 0 | `CSystemInitializeStep_None` | 상태 초기화 (Complete=false, Fail=false, ProcessStatus=SPS_INITIAL) |
| 1 | `CSystemInitializeStep_InitModule` | Motion/IO/Light 장치 초기화 |
| 2 | `CSystemInitializeStep_CheckInterlock` | Loader Arm/Shutter/X-Ray/Door 센서 체크 |
| 3 | `CSystemInitializeStep_InitIO` | PLC 연결 확인, Probe UnContact 확인, Fixture/PLC 초기화 |
| 4 | `CSystemInitializeStep_MotionHoming1` | Lift Pin 홈 (AXIS_LOADER_Z) |
| 5 | `CSystemInitializeStep_MotionHoming2` | Head Z축 홈 (Z1,Z2,ZOOM,THETA,FILTER,VLENS,ROTATE,FOCUS 등) |
| 6 | `CSystemInitializeStep_MotionHoming3` | (Reserved - returns SUCCESS) |
| 7 | `CSystemInitializeStep_MotionHoming4` | Head XY축 홈 (X2,X3,Y1,Y2,Y3,Y4) |
| 8 | `CSystemInitializeStep_MotionHoming5` | Probe Z축 홈 (PROBE_Z1~Z5) |
| 9 | `CSystemInitializeStep_MotionHoming6` | Probe Y축 홈 (PROBE_Y1~Y5) + ProbeAlign Reset |
| 10 | `CSystemInitializeStep_MotionHoming7` | Probe X축 홈 (PROBE_X1,X3,X5,X7,X9) |
| 11 | `CSystemInitializeStep_MotionHoming8` | Stage X축 홈 (AXIS_X) |
| 12 | `CSystemInitializeStep_MotionHoming9` | Stage R축 홈 (AXIS_LOADER_R) |
| 13 | `CSystemInitializeStep_MotionHoming10` | Auto Change 위치 확인 → 이동 |
| 14 | `CSystemInitializeStep_MotionHoming11` | AC Z축 홈 (AXIS_Z9) |
| 15 | `CSystemInitializeStep_MotionHoming12` | AC X축 홈 (AXIS_X4) |
| 16 | `CSystemInitializeStep_MotionComplete` | Motion 초기화 완료 플래그 설정 |
| 17 | `CSystemInitializeStep_InitIO` | PLC 재초기화 (반복) |
| 18 | `CSystemInitializeStep_Safety` | Safety 위치 이동 (EQStateChange → enumEQSTATUS_SAFETY) |
| 19 | `CSystemInitializeStep_InitComplete` | IO/Motion 초기화 확인, EQ 상태 체크, bSystemInitOk = TRUE |
| 20 | `CSystemInitializeStep_ProbeAlign` | 5개 Probe 정렬 (Glass 존재 시) |
| 21 | `CSystemInitializeStep_Safety` | Safety 위치 재이동 |
| 22 | `CSystemInitializeStep_Idle` | 완료 처리, ProcessStatus=SPS_IDLE |

### 3.2 주요 조건 및 예외

- **Interlock 체크 (Step 2):**
  - `IsLoaderArmSensorOn()` → true이면 Fail (`PLC_ARM_SENSOR_DETECT`)
  - `IsShutterSensorClose()` → false이면 Fail (`PLC_LOADER_SHUTTER_UP_ERROR`)
  - `IsX_RaySensorRunOn()` → true이면 Fail
  - `IsDoorSensorClose()` → false이면 Fail (`PLC_DOOR_SENSOR_DETECT`)

- **Motion Homing (Step 4~15):**
  - 각 축별 `MoveHome()` → `CheckHomeDone(df_HOMINGDONE_TIME)` 시퀀스
  - `GetStopUser()` 체크로 중간 정지 가능
  - AC(Auto Change) 위치 판단 후 이동 방향 결정
  - Vision Motion Homing: `CommTcpIpFactoryMgr::DoWorkSetParameter(enumTCPIpFactorytID_VISION)`

- **Probe Align (Step 20):**
  - Glass 센서 ON 시에만 실행
  - 5개 Probe (df_PROBE_AGING1~df_PROBE_MEAS) 순회
  - Probe Exist 센서 확인 → Align Recipe 오픈 → Probe SW Align 실행
  - PROBE ID 기반 Recipe 이름 결정 (예: `PROBE_ALIGN1_XX`)

### 3.3 사용 변수

- `bSystemInitOk` (SharedMemory)
- `ProcessStatus` (SPS_INITIAL → SPS_IDLE)
- `df_HOMINGDONE_TIME` - 호밍 완료 대기 시간
- `CSystemParamShare::SetInitialized()`

---

## 4. Stage 1: Glass Adsorption (Glass 흡착/측정준비)

**클래스:** `CDoWorkGlassAdsorption`
**파일:** `DoWorkGlassAdsorption.cpp` (line 15~183), `DoWorkGlassAdsorptionStep.cpp` (line 1~503)
**SeqItemID:** `enumSeqItemID_GLASS_ADSORPTION`
**시퀀스 이름:** `GLASS 1_ADSORPTION`

### 4.1 Step 구성

| Step | Class | Name | Description |
|------|-------|------|-------------|
| 0 | `_None` | - | 상태 초기화, SPS_PROCESS, MCC_A_MEAS_READY 시작 |
| 1 | `_Move` | Move Measure Pos | Safety 위치 이동, 상태 전환 준비 |
| 2 | `_ChangeMeasStatus` | Change Measure Status | EQ 상태를 Measure로 변경 |
| 3 | `_CheckMeasStatus` | Check Measure Status | Measure 상태 확인 |
| 4 | `_ProbeAlign` | Probe Align Check | Probe 정렬 확인/실행 |
| 5 | `_Idle` | - | 완료 처리 |

### 4.2 상세 동작

**Step 1 - Move:**
- `MaintControlMgr::ResetAlign()` - Align 초기화
- Loader Arm 센서 체크 → `PLC_ARM_SENSOR_DETECT`
- Shutter 센서 체크 → `PLC_LOADER_SHUTTER_UP_ERROR`
- EQ 상태가 Meas가 아니면:
  - Safety 센서 확인
  - Lift Pin Down 확인 (아니면 `DoWork_LPMove(false)`)
  - Safety 위치(dSafetyStageX)로 Stage X 이동
  - Safety 센서 재확인

**Step 2 - ChangeMeasStatus:**
- Glass 미존재 + Ready 상태 → Skip
- Ready → `enumEQSTATUS_R2M` 상태 전환
- Load → `enumEQSTATUS_L2M` 상태 전환
- Abnormal → Fail (`PLC_STATE_UNKNOWN_ERROR`)
- 완료 대기: 100ms 간격, 1000회 시도 (약 100초)

**Step 3 - CheckMeasStatus:**
- Glass 미존재 → Skip
- EQ 상태가 Meas가 아닌 경우 폴링 대기 (100ms * 100회)
- 실패 시 `PLC_STATE_NOT_MEASURE`

**Step 4 - ProbeAlign:**
- 5개 Probe 순회 (df_PROBE_AGING1~df_PROBE_MEAS)
- Pause/Resume 이벤트 대기
- Probe Exist 센서 확인, 미정렬 Probe만 처리
- PROBE ID 기반 Align Recipe 오픈 → ProbeSWAlign 실행

### 4.3 사용 Recipe

- `PositionConfigMgr::GetSafetyPosition()` - Safety 위치 좌표

---

## 5. Stage 2: Recipe Change (레시피 변경)

**클래스:** `CDoWorkGlassRecipe`
**파일:** `DoWorkGlassRecipe.cpp` (line 15~170), `DoWorkGlassRecipeStep.cpp` (line 1~430)
**SeqItemID:** `enumSeqItemID_GLASS_RECIPE_CHANGE`
**시퀀스 이름:** `GLASS 2_RECIPE_CHANGE`

### 5.1 Step 구성

| Step | Class | Name | Description |
|------|-------|------|-------------|
| 0 | `_None` | - | 상태 초기화, ResultMgr::ClearData() |
| 1 | `_Change` | - | Recipe 오픈 및 검사 장비 Recipe 설정 |
| 2 | `_ProbeCheck` | - | Probe 교체 확인/실행 |
| 3 | `_Idle` | - | 완료 처리 |

### 5.2 상세 동작

**Step 1 - Change:**
- `RecipeEditMgr::OpenRecipe(PPID)` - Recipe 파일 오픈
- Recipe 정보에서 검사 항목 플래그 결정:
  - `m_bUseAgingTest_2` → `enumMEASURE_MODE_AGING`
  - `m_bUseSpec_3` → `enumMEASURE_MODE_WAD`
  - `m_bUseQuality_4` → `enumMEASURE_MODE_VISION`
  - `m_bUseColor_5` → `enumMEASURE_MODE_COLOR`
  - `m_bUseAngle_6` → `enumMEASURE_MODE_ANGLE`
  - `m_bUseTSP_7` → `enumMEASURE_MODE_TSP`
  - `m_bUsePre_0 | m_bUseElect_1 | m_bUseAgingTest_2` → `enumMEASURE_MODE_PG`
- Vision Quality 초기화: `DoWorkVisionCmd(EXE_QUALITY_RESET)`, `EXE_QUALITY_WAIT_USE`
- 각 장비별 Recipe Open (TCP/IP 통신):
  - PG → AGING → WAD → LCR → TSP → VISION

**Step 2 - ProbeCheck:**
- Recipe의 Probe Priority (0=First, 1=Second) 에 따라 5개 Probe 확인
- 현재 장착 Probe ID vs Recipe 요구 Probe ID 비교
- 불일치 시:
  - Probe 미장착 → `PACC Loading` (Cassette에서 로딩)
  - Probe 장착 → `PACC Exchange` (교환)
  - `FunctionModuleProbeACC::Start()` 실행, 최대 30분 대기

### 5.3 사용 Recipe 파라미터

- `S_RecipeInfo::m_sInspectItem` - 검사 항목 플래그 (Pre, Elect, Aging, Spec, Quality, Color, Angle, TSP)
- `S_RecipeInfo::m_sProbeInfo` - Probe Priority, ProbeFirst[5], ProbeSecond[5]
- `S_RecipeData::m_vecStick` - Stick 정보 (Cell 수 포함)
- `GlassInfo::m_nCellCount`, `m_nStickCount`

---

## 6. Stage 3: Glass SW Align (Glass 정렬)

**클래스:** `CDoWorkGlassSWAlign`
**파일:** `DoWorkGlassSWAlign.cpp` (line 14~191), `DoWorkGlassSWAlignStep.cpp` (line 1~1595)
**SeqItemID:** `enumSeqItemID_GLASS_SW_ALIGN`
**시퀀스 이름:** `GLASS 3_GLASS_ALIGN`

### 6.1 Step 구성

| Step | Class | Name | Description |
|------|-------|------|-------------|
| 0 | `_None` | - | 상태 초기화, MCC_A_PROC_ALIGN 시작 |
| 1 | `_MoveAlignPosistion` | - | Align 위치로 이동 |
| 2 | `_AlignGlass` | - | Glass Align 실행 |
| 3 | `_Idle` | - | 완료 처리 |

### 6.2 상세 동작

**Step 1 - MoveAlignPosition:**
- Glass Exist 센서 확인 → 미존재 시 Fail (`PLC_GLASS_OBLIVIOUS_WORKPLATE_SENSOR_ALARM`)
- `MoveAlignPosition(0, 0)` 호출:
  - Recipe Align 파라미터에서 CCD/Light 설정 (Exposure, Gain, LED Intensity)
  - Light 설정: SPO LED Lamp Intensity ch[0~3]
  - Camera Exposure/Gain 설정 (Front Left/Right)
  - Interlock 체크: Loader Arm, Shutter, Probe UnContact
  - 이동 순서: **Stage R → Head Y → Probe X → Head X → Stage X → Head Z**
  - 각 축 MotionDone 대기

**Step 2 - AlignGlass:**
1. **AlignGlassSearch()** - 패턴 서치
   - 2개 Head (Front Left/Right) 에서 최대 5개 패턴 매칭 시도
   - `AlignCameraMgr::PtrnMatch()` 사용
   - 실패 시 Shift 이동 후 재시도 (3 X/Y 루프 * 6 Theta)
   - 최종 실패 시 `VISION_ALIGN_DETECTED_FAIL`

2. **AlignGlassTZero()** - Theta(R축) 보정
   - 2개 Head 패턴 결과로 각도 계산 (`CalcAngle()`)
   - Margin 이내 확인 (`m_sMargin.m_sGlassMargin.m_fR`)
   - 초과 시 `AXIS_LOADER_R` 보정 이동, 최대 nRetrylimit(=Centering) 회 재시도

3. **AlignGlassXYZero()** - XY 보정
   - Front Left 카메라 기준 XY 편차 계산
   - Margin 확인 (`m_fX`, `m_fY`)
   - 초과 시 `AXIS_X`, `AXIS_Y1`, `AXIS_Y2` 보정 이동
   - 최대 nRetrylimit 회 재시도

4. Align 결과 저장: `MaintControlMgr::SetGlassAlignXYR(dMovX, dMovY, dMovR)`

### 6.3 사용 Recipe 파라미터

- `S_Align::m_sCcdSet` - Front Left/Right CCD 설정 (LED, ExposureTime, Gain, MatchingScore)
- `S_Align::m_sGlsAlign` - Align 위치 (StageY, StageRotate, FrontY, FrontLeft, FrontRight, FrontZ)
- `S_Align::m_chPtnFrontLeft[5]`, `m_chPtnFrontRight[5]` - 패턴 이미지 파일명
- `S_Align::m_sMargin.m_sGlassMargin` - 정렬 마진 (X, Y, R, nCentering)
- `S_Align::m_sGlsAlign.m_fBase_FrontRight` - 기준 위치

---

## 7. Stage 4: Probe SW Align (프로브 정렬)

**클래스:** `CDoWorkProbeSWAlign`
**파일:** `DoWorkProbeSWAlign.cpp` (line 14~), `DoWorkProbeSWAlignStep.cpp`
**SeqItemID:** `enumSeqItemID_PROBE_SW_ALIGN`
**시퀀스 이름:** `GLASS 3_PROBE_ALIGN`

### 7.1 Step 구성

| Step | Class | Name | Description |
|------|-------|------|-------------|
| 0 | `_None` | - | 상태 초기화 |
| 1 | `_Move` | - | Probe Align 위치 이동 |
| 2 | `_Align` | - | Probe Align 실행 |
| 3 | `_Idle` | - | 완료 처리 |

### 7.2 호출 경로

Probe Align은 두 곳에서 호출됨:
1. **SystemInitialize Step 20** (`CSystemInitializeStep_ProbeAlign`) - 초기화 시
2. **DoWorkGlassAdsorption Step 4** (`CDoWorkGlassAdsorptionStep_ProbeAlign`) - 매 측정 사이클

모두 `CDoWorkGlassSequence::ItemStart("SEQUENCE_ITEM_DWGLASS_SEQ_PROBE_SW_ALIGN", true)` 로 호출.

---

## 8. Stage 5: Pre-Measurement (사전 검사)

**클래스:** `CDoWorkGlassPreMeas`
**파일:** `DoWorkGlassPreMeas.cpp` (line 17~193), `DoWorkGlassPreMeasStep.cpp`
**SeqItemID:** `enumSeqItemID_GLASS_PRE_MEAS`
**시퀀스 이름:** `GLASS 4_PRE_MEAS`

### 8.1 Step 구성

| Step | Class | Name | Description |
|------|-------|------|-------------|
| 0 | `_None` | - | 상태 초기화, Glass Sensor Power ON |
| 1 | `_Move` | - | PAD Pattern 위치 이동 (첫 Stick) |
| 2 | `_PtrnSearch` | - | PAD Pattern Search |
| 3 | `_RcpChangeDone` | - | 장비 Recipe 변경 완료 확인 |
| 4 | `_P2P` | - | Probe Pin to Pin |
| 5 | `_Contact` | - | Probe Contact |
| 6 | `_OST` | - | Open Short Test |
| 7 | `_RTEST` | - | R-Test (저항 검사) |
| 8 | `_Idle` | - | 완료 처리, Glass Sensor Power OFF |

### 8.2 주요 동작

- **Move:** Stage/Gantry를 첫 Stick 위치로 이동, 조명 설정
- **PtrnSearch:** PAD 패턴 매칭으로 정밀 위치 보정
- **RcpChangeDone:** 외부 장비(PG, WAD 등) Recipe 변경 완료 대기
- **P2P:** Probe Pin to Pin 위치 정밀 조정
- **Contact:** Probe를 Glass PAD에 접촉
- **OST:** Open/Short Test 실행 (PG TCP/IP 통신)
- **RTEST:** 저항 측정 Test

### 8.3 사용 Recipe 파라미터

- `S_RecipeData::m_vecStick[0]` - 첫 Stick 위치 (StageR, StageY)
- `S_RecipeInfo::m_sMotionOffsetPos` - Stage Y/R 오프셋
- `S_RecipeInfo::m_sLampItem` - Front Left/Right Lamp Intensity
- `S_RecipeInfo::m_sContactPos` - PreContactPos, EndContactPos, AddOD

---

## 9. Stage 6: AGING (에이징 검사)

**클래스:** `CDoWorkGlassAGING`
**파일:** `DoWorkGlassAGING.cpp` (line 18~183), `DoWorkGlassAGINGStep.cpp`
**SeqItemID:** `enumSeqItemID_GLASS_AGING`
**시퀀스 이름:** `GLASS 5_AGING`

### 9.1 Step 구성

| Step | Class | Name | Description |
|------|-------|------|-------------|
| 0 | `_None` | - | 상태 초기화 |
| 1 | `_Move_First` | - | 첫 번째 Aging Probe 이동 |
| 2 | `_Aging` | - | 첫 번째 Aging Test 실행 |
| 3 | `_Move_Second` | - | 두 번째 Stick 이동 |
| 4 | `_Aging_Second` | - | 두 번째 Aging Test |
| 5 | `_Move_3` | - | 세 번째 Stick 이동 |
| 6 | `_Aging_3` | - | 세 번째 Aging Test |
| 7 | `_Idle` | - | 완료 처리 |

### 9.2 동작 개요

- HMI는 **4개 Aging Probe** (P1~P4) 사용 (OCP에는 해당 없음)
- 최대 3회 Stick 이동 + Aging 반복
- `CMeasureSequenceItemEngine` 사용
- Recipe의 `m_bUseAgingTest_2` 플래그로 활성화

---

## 10. Stage 7: Measure (본 검사)

**클래스:** `CDoWorkGlassMeasure`
**파일:** `DoWorkGlassMeasure.cpp` (line 16~263), `DoWorkGlassMeasureStep.cpp`
**SeqItemID:** `enumSeqItemID_MEASURE`
**시퀀스 이름:** `GLASS 6_MEASURE`

### 10.1 Step 구성

| Step | Class | Name | Description |
|------|-------|------|-------------|
| 0 | `_None` | - | 상태 초기화 |
| 1 | `_MoveMeasureArea` | - | Stick 위치 이동 (Probe UnContact) |
| 2 | `_PtrnSearch` | - | Pattern Search |
| 3 | `_Probe_Contact` | - | Probe Contact |
| 4 | `_Measure_WAD` | - | 특성 검사 이동 & 실행 |
| 5 | `_Measure_ANGLE` | - | 시야각 검사 이동 & 실행 |
| 6 | `_Measure_TSP` | - | TSP 검사 |
| 7 | `_Measure_VISION` | - | 화면 검사 이동 & 실행 |
| 8 | `_Measure_COLOR` | - | 색좌표 검사 이동 & 실행 |
| 9 | `_Measure_VISION_COLOR` | - | 화면+색좌표 동시 검사 |
| 10 | `_Idle` | - | 완료 처리 |

### 10.2 Stick 루프 실행

**ThreadRun() 특수 구조** (line 125~263):

```
nStepIndex = GetLoopStepIndex()   // = Stick 수
if (nStepIndex == 0) return       // 0이면 즉시 완료

for (i = 0; i < nStepIndex; i++)  // N Stick 반복
{
    GoToInitStep();               // 매 Stick마다 Step 0부터
    
    while (true) {
        pSeqItem->SetMeasureStick(i);  // 현재 Stick 설정
        
        if (마지막 Stick)
            → Step 0 ~ Idle(Exit) 전체 실행
        else
            → Step 0 ~ (Idle-1) 실행 후 break → 다음 Stick
    }
}
```

### 10.3 Cell ID 생성

`MakeCellID()` (line 112~143):
- Glass Rotate = 1 (YY, BT): `strCellId = "{A+cellNum}{stickNum+1}"`
- Glass Rotate = 0 (DG): `strCellId = "{A+maxStick-stick-1}{cellNum+1}"`

### 10.4 사용 검사 장비 (TCP/IP 통신)

| Mode | Factory ID | Description |
|------|-----------|-------------|
| WAD | `enumTCPIpFactorytID_WAD` | 특성 검사 (Spectrophotometer) |
| VISION | `enumTCPIpFactorytID_VISION` | 화면 자동 검사 |
| COLOR | `enumTCPIpFactorytID_COLOR` | 색좌표 검사 |
| ANGLE | `enumTCPIpFactorytID_ANGLE` | 시야각 검사 |
| TSP | `enumTCPIpFactorytID_TSP` | TSP 검사 |
| PG | `enumTCPIpFactorytID_PG` | Pattern Generator |
| AGING | `enumTCPIpFactorytID_AGING` | Aging 검사 |
| LCR | `enumTCPIpFactorytID_LCR` | LCR 검사 |

### 10.5 사용 Recipe 파라미터

- `S_RecipeData::m_vecStick[]` - Stick별 Cell 정보
- `S_RecipeInfo::m_sInspectItem` - 검사 항목 활성화 플래그
- `S_RecipeInfo::m_sMotionOffsetPos` - Motion Offset
- `S_RecipeInfo::m_sContactPos` - Probe Contact 위치
- `S_RecipeInfo::m_sOptionItem.m_nGlsRotate` - Glass Rotate 모드
- `S_RecipeData::m_sHeadZ.m_fSpecZ` - Spec Z 위치

---

## 11. Stage 8: Result (결과 처리)

**클래스:** `CDoWorkGlassResult`
**파일:** `DoWorkGlassResult.cpp` (line 15~194), `DoWorkGlassResultStep.cpp`
**SeqItemID:** `enumSeqItemID_GLASS_RESULT`
**시퀀스 이름:** `GLASS 7_RESULT`

### 11.1 Step 구성

| Step | Class | Name | Description |
|------|-------|------|-------------|
| 0 | `_None` | - | 상태 초기화 |
| 1 | `_CompleteCheck` | - | 검사 결과 저장 (ResultMgr::SaveResult) |
| 2 | `_Idle` | - | 완료 처리 |

### 11.2 상세 동작

**Step 1 - CompleteCheck:**
- Recipe의 검사 항목(Spec, Quality, Color, Angle, TSP, LCR) 중 하나라도 사용 시:
  - `CResultMgr::GetInstance()->SaveResult(true, true, strStartTime)` 호출
  - 시작 시간은 `SeqTactTimeMgr::GetStartTime2()`에서 가져옴
- 검사 항목 미사용 시: Skip

### 11.3 사용 변수

- `CResultMgr` (Singleton) - 결과 데이터 수집/저장
- `CSeqTactTimeMgr` - Tact Time 관리

---

## 12. Stage 9: Unload (언로드)

**클래스:** `CDoWorkGlassUnload`
**파일:** `DoWorkGlassUnload.cpp` (line 15~183), `DoWorkGlassUnloadStep.cpp` (line 1~298)
**SeqItemID:** `enumSeqItemID_GLASS_UNLOAD`
**시퀀스 이름:** `GLASS 8_UNLOAD`

### 12.1 Step 구성

| Step | Class | Name | Description |
|------|-------|------|-------------|
| 0 | `_None` | - | 상태 초기화 |
| 1 | `_Move_Safety` | - | Safety 위치로 이동 |
| 2 | `_CheckSaveResult` | - | 결과 저장 완료 확인 |
| 3 | `_SendToCIM` | - | CIM 측정 완료 통보 |
| 4 | `_Idle` | - | 완료 처리 |

### 12.2 상세 동작

**Step 1 - Move_Safety:**
- PG Turn Off: `DoWorkInspectStart(PG, PGEvent_8)`
- `MaintControlMgr::MoveCmd_SafetyAll()` - 전체 축 Safety 위치로 이동
- 실패 시 Fail 설정

**Step 2 - CheckSaveResult:**
- 검사 항목 사용 시: `CResultMgr::SaveResultDone()` 대기 (100ms * 300회 = 30초)
- 검사 미사용 시: `CommTcpIpObjectCIM::SetUserBypassGlass(true)` 설정

**Step 3 - SendToCIM:**
- ONLINE_REMOTE 모드 시에만 실행
- `WM_MSG_UPDATE_CHANGE_CIM_STATE` + `TO_SERVER_MEAS_END` 메시지 전송

### 12.3 사용 변수

- `df_HOST_ONLINE_REMOTE` - CIM Online Remote 모드
- `TO_SERVER_MEAS_END` - CIM 측정 완료 커맨드

---

## 13. Equipment State Machine (설비 상태 머신)

### 13.1 EQ States (`enumEQState`)

```
enumEQStateReady    = 0  → 대기
enumEQStateLoad     = 1  → Glass 로딩
enumEQStateMeas     = 2  → 측정 중
enumEQStateAbnormal = 3  → 비정상
```

### 13.2 State Transitions (`enumEQStatus`)

```
enumEQSTATUS_R2L    = 1  → Ready → Load
enumEQSTATUS_L2M    = 2  → Load → Measure
enumEQSTATUS_M2R    = 3  → Measure → Ready
enumEQSTATUS_R2M    = 4  → Ready → Measure
enumEQSTATUS_M2L    = 5  → Measure → Load
enumEQSTATUS_L2R    = 6  → Load → Ready
enumEQSTATUS_SAFETY = 7  → Safety 위치 이동
```

### 13.3 실행 방법

```cpp
CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()
    ->SetEQStatusChangeStep(enumEQSTATUS_R2M);
CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()
    ->Start();
// 완료 대기 폴링
```

---

## 14. CIM Sequence (CIM 통신)

**클래스:** `CDoWorkSequenceCIM`
**파일:** `DoWorkSequenceCIM.cpp`

### 14.1 CIM Status Steps

| Status | Steps | Description |
|--------|-------|-------------|
| INIT | None → Initial(TIMEREQ) → Initial2(STS0304) → Loading → LoadReady(STS0004/STS0506) → MeasDone(STS1106) → Idle | 초기 온라인 전환 |
| READY | None → Loading → LoadReady → Idle | Load 대기 |
| LOAD_READY | None → CheckLoadReq → ChangeLoad → LoadReq → Idle | Load 요청 대기 |
| LOAD | None → PPIDREQ → LOTIDREQ → GLSIDREQ → ... | 자재 정보 요청 |

---

## 15. Process Status Values

```
SPS_INITIAL  → 초기화 중
SPS_PROCESS  → 공정 진행 중
SPS_IDLE     → 대기 (완료)
```

---

## 16. Complete Auto-Run Flow Summary

```
[EngineStart]
  │
  ├── MCC_A_AUTO_CYCLE = true
  ├── Get GlassID, RecipeID, CycleCount
  │
  └── for (cycle = 0..N) {
        │
        ├── TactTime Start
        │
        ├── ThreadRu_nSequence(cycle)
        │   │
        │   ├── [0] Adsorption (첫 사이클만)
        │   │     ├─ Move → Safety/LiftDown 위치 확인
        │   │     ├─ ChangeMeasStatus → R2M or L2M
        │   │     ├─ CheckMeasStatus → enumEQStateMeas 확인
        │   │     └─ ProbeAlign → 미정렬 Probe 정렬
        │   │
        │   ├── [1] Recipe
        │   │     ├─ Change → Recipe Open, 검사장비 Recipe 설정
        │   │     └─ ProbeCheck → Probe 교체 (SACC)
        │   │
        │   ├── [2] Glass Align
        │   │     ├─ MoveAlignPosition → CCD/Light 설정, 축 이동
        │   │     └─ AlignGlass → Search → TZero → XYZero
        │   │
        │   ├── [3] Probe Align
        │   │     ├─ Move → Probe Align 위치
        │   │     └─ Align → Probe 패턴 매칭/보정
        │   │
        │   ├── [4] PreMeas
        │   │     ├─ Move → 첫 Stick PAD 위치
        │   │     ├─ PtrnSearch → PAD 패턴 찾기
        │   │     ├─ RcpChangeDone → 장비 Ready 확인
        │   │     ├─ P2P → Pin to Pin
        │   │     ├─ Contact → Probe 접촉
        │   │     ├─ OST → Open Short Test
        │   │     └─ RTEST → 저항 Test
        │   │
        │   ├── [5] AGING
        │   │     ├─ Move/Aging × 3회 (Stick별)
        │   │     └─ (P1~P4 Aging Probe 사용)
        │   │
        │   ├── [6] Measure (× N Sticks)
        │   │     ├─ MoveMeasureArea → Stick 위치 이동
        │   │     ├─ PtrnSearch → 패턴 매칭
        │   │     ├─ Probe_Contact → 프로브 접촉
        │   │     ├─ Measure_WAD → 특성 검사
        │   │     ├─ Measure_ANGLE → 시야각 검사
        │   │     ├─ Measure_TSP → TSP 검사
        │   │     ├─ Measure_VISION → 화면 검사
        │   │     ├─ Measure_COLOR → 색좌표 검사
        │   │     └─ Measure_VISION_COLOR → 동시 검사
        │   │
        │   ├── [7] Result
        │   │     └─ CompleteCheck → ResultMgr::SaveResult()
        │   │
        │   └── [8] Unload
        │         ├─ Move_Safety → 전축 Safety 이동
        │         ├─ CheckSaveResult → 저장 완료 대기
        │         └─ SendToCIM → 측정 완료 통보
        │
        ├── CCD LIVE Start
        ├── TactTime End
        │
        └── if (StopState) break
      }
  │
  └── WM_MSG_UPDATE_CYCLE_INSPECTION_COMPLETE
```

---

## 17. Error Codes Used

| Code | Constant | Description |
|------|----------|-------------|
| - | `PLC_ARM_SENSOR_DETECT` | Loader Arm 감지 |
| - | `PLC_LOADER_SHUTTER_UP_ERROR` | Shutter 미닫힘 |
| - | `PLC_DOOR_SENSOR_DETECT` | Door 미닫힘 |
| - | `PLC_STATE_UNKNOWN_ERROR` | EQ 상태 비정상 |
| - | `PLC_STATE_NOT_MEASURE` | Measure 상태 미전환 |
| - | `PLC_LIFTPIN_DOWN_POS_ERROR` | Lift Pin Down 실패 |
| - | `PLC_CANNOT_DETECT_SAFETY_SENSOR_X` | Safety 센서 미감지 |
| - | `PLC_GLASS_OBLIVIOUS_WORKPLATE_SENSOR_ALARM` | Glass 미존재 |
| - | `PLC_FIXTURE_CYLINDER_FWD_ERROR` | Fixture Forward 실패 |
| - | `PLC_FIXTURE_CYLINDER_DOWN_ERROR` | Fixture Down 실패 |
| - | `PLC_INITIAL_FAIL` | PLC 초기화 실패 |
| - | `PLC_VACUUM_SENSOR_ERROR` | Vacuum 센서 에러 |
| - | `PLC_SOCKET_CREATE_FAIL` | PLC 소켓 연결 실패 |
| - | `MEI_INIT_MC_FAIL` | Motion 초기화 실패 |
| - | `MTOR_HOME_FAIL_ALARM` | 홈 실패 |
| - | `MOTION_INTERLOCK_6` | Probe UnContact 미확인 |
| - | `VISION_ALIGN_DETECTED_FAIL` | Align 패턴 미검출 |
| - | `VISION_ALIGN_CENTERING_FAIL` | Align Centering 실패 |
| - | `JOB_PROC_SAFETY_POS_ERROR` | Safety 위치 이동 실패 |
| - | `JOB_PROC_ALIGN_POS_ERROR` | Align 위치 이동 실패 |

---

## 18. Thread Model Summary

| Component | Thread Type | Description |
|-----------|-------------|-------------|
| `CDoWorkGlassSequenceEngine` | `AfxBeginThread` + WaitForMultipleObjects | 9개 시퀀스 순차 실행 |
| `CBaseSequenceImpl` (각 DoWork) | `AfxBeginThread` + Start/Close Event | 개별 시퀀스 Step 실행 |
| `CSystemInitialize` | 상동 | 22 Step 초기화 |
| `CFunctionModuleEQStateChange` | 상동 | EQ 상태 전환 |
| `CMeasureSequenceItemEngine` | 상동 | 측정 아이템 서브엔진 |

모든 Thread는 Event 기반 동기화 사용:
- `m_hStartEvent` - 시작 신호
- `m_hCloseEvent` - 종료 신호  
- `m_hCompleteEvent` - 완료 신호
- `m_hEvtUserPause` / `m_hEvtUserResume` - 일시정지/재개
