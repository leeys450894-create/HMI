# 11. Data Flow Analysis

End-to-end data flow from user Start button press through measurement to final result output.

---

## 1. Overall Data Flow Diagram

```
User Press Start
    |
    v
CDlgSubOperate (UI Button Handler)
    |
    v
CSequenceManager::EngineStart() [Singleton]
    |
    v
CDoWorkGlassSequence::EngineStart()
    |
    v
CDoWorkGlassSequenceEngine::EngineStart() --> SetEvent(m_hStartEvent)
    |
    v
ThreadRun() --> ThreadRu_nSequence() [Worker Thread]
    |
    v  (Sequential execution of 9 steps)
    +-- [0] CDoWorkGlassAdsorption  -- Stage vacuum, glass adsorption
    +-- [1] CDoWorkGlassRecipe      -- Recipe load, device recipe open
    +-- [2] CDoWorkGlassSWAlign     -- Glass alignment (camera + motion)
    +-- [3] CDoWorkProbeSWAlign     -- Probe alignment
    +-- [4] CDoWorkGlassPreMeas     -- Pre-measurement inspection
    +-- [5] CDoWorkGlassAGING       -- Aging test
    +-- [6] CDoWorkGlassMeasure     -- Main measurement (WAD/Vision/Color/Angle/TSP)
    +-- [7] CDoWorkGlassResult      -- Result save + CIM notification
    +-- [8] CDoWorkGlassUnload      -- Glass unload
    |
    v
Result Files + CIM/MES Report
```

---

## 2. Start Button --> Sequence Engine

### 2.1 User Start (Cycle Mode)

| Attribute | Value |
|-----------|-------|
| **Source** | `CDlgSubOperate::OnBnClickedButtonCycle()` (DlgSubOperate.cpp:1574) |
| **Data** | User confirmation (MB_YESNO dialog), cycle count, recipe ID |
| **Transport** | Function call chain |
| **Destination** | `CSequenceManager::EngineStart()` |
| **Transformation** | Sets cycle state via `CSystemParamShare::SetCycleState(true)`, updates UI buttons |

**Pre-conditions checked before start:**
1. No active alarms (`CAlarmController::GetAlarmCount() == 0`)
2. Process status is IDLE (`GetSys_ProcessStatus() == SPS_IDLE`)
3. EQ state change not running (`Is_ThreadRun() == false`)
4. Safety reset confirmed (`IsSafetyResetOn()`)

### 2.2 State Change Mode (Ready/Load/Meas transitions)

| Attribute | Value |
|-----------|-------|
| **Source** | `CDlgSubOperate::OnBnClickedButtonChangeReady/Load/Meas()` (DlgSubOperate.cpp:1350-1509) |
| **Data** | Target EQ status (`enumEQSTATUS_R2L`, `enumEQSTATUS_L2M`, etc.) |
| **Transport** | `CFunctionModuleEQStateChange::SetEQStatusChangeStep()` then `Start()` |
| **Destination** | `CFunctionModuleEQStateChange::ThreadRun()` |
| **Transformation** | Executes step sequence for physical state transition (motor moves, PLC signals) |

### 2.3 Sequence Engine Thread

| Attribute | Value |
|-----------|-------|
| **Source** | `CDoWorkGlassSequenceEngine::EngineStart()` (DoWorkGlassSequenceEngine.cpp) |
| **Data** | `m_hStartEvent` (Win32 Event) |
| **Transport** | `SetEvent()` wakes `ThreadRun()` |
| **Destination** | `ThreadRun()` --> `ThreadRu_nSequence()` |
| **Transformation** | Iterates `m_vecItems[0..8]`, calls `Start()` on each, waits for `GetCompleteEventHandle()` via `WaitForSingleObject(INFINITE)` |

**Key file:** `Source/ALPHA/DoWork/DoWorkGlassSequenceEngine.cpp`

**Cycle management** (DoWorkGlassSequenceEngine.cpp:637):
- Offline mode: cycle count from `CSystemParamShare::GetCycleCount()`
- Online remote: single cycle (count=1)
- Glass ID auto-generated for cycles > 1 using timestamp

---

## 3. Recipe Data Flow

### 3.1 Recipe Loading

```
Recipe INI File (.rcp)
    |  CRecipeEditMgr::OpenRecipe(strPPID)
    v
CRecipeData (in-memory)
    |  GetRecipeInfo() / GetRecipeData() / GetRecipeMap()
    v
S_RecipeInfo / S_RecipeData / S_RecipeMap structs
    |  Distributed to sequence steps via m_pShareItem->GetRecipeEditMgrPtr()
    v
Sequence Steps + Device Controllers
```

| Attribute | Value |
|-----------|-------|
| **Source** | Recipe file on disk (`.rcp` binary format) |
| **Data type** | `S_RecipeInfo` (inspect items, probe info, contact positions), `S_RecipeData` (sticks, cells, head Z), `S_RecipeMap` (cell distances) |
| **Transport** | `CRecipeEditMgr::OpenRecipe()` --> `CBinaryHelper` file I/O |
| **Destination** | `CRecipeData` in-memory object, accessible via `stAuroraMgrPool::pRecipeEditMgr` |
| **Transformation** | Binary file deserialized into structured data |

**Key files:**
- `Source/ALPHA/Recipe/RecipeEditMgr.h` -- recipe manager
- `Source/ALPHA/Recipe/RecipeData.h` -- data structures (`stCoordinate`, `stPtrnInfo`, `stRecipeAlign`, `stRecipeMeas`, `stRecipeOption`)

### 3.2 Recipe Distribution to Devices (DoWorkGlassRecipeStep_Change)

| Attribute | Value |
|-----------|-------|
| **Source** | `CDoWorkGlassRecipeStep_Change::Run()` (DoWorkGlassRecipeStep.cpp:67) |
| **Data** | Inspection mode flags from `S_RecipeInfo::m_sInspectItem` |
| **Transport** | TCP/IP via `CCommTcpIpFactoryMgr::DoWorkRecipeOpen(enumTCPIpFactorytID_*)` |
| **Destination** | External measurement devices (PG, AGING, WAD, LCR, TSP, VISION) |
| **Transformation** | Recipe inspect flags mapped to `enumMEASURE_MODE_*` booleans, then sent as device-specific commands |

**Inspection mode mapping** (DoWorkGlassRecipeStep.cpp:148-157):
```
m_bUseAgingTest_2  --> enumMEASURE_MODE_AGING
m_bUseSpec_3       --> enumMEASURE_MODE_WAD
m_bUseQuality_4    --> enumMEASURE_MODE_VISION
m_bUseColor_5      --> enumMEASURE_MODE_COLOR
m_bUseAngle_6      --> enumMEASURE_MODE_ANGLE
m_bUseTSP_7        --> enumMEASURE_MODE_TSP
m_bUsePre_0 | m_bUseElect_1 | m_bUseAgingTest_2 --> enumMEASURE_MODE_PG
```

### 3.3 Recipe to Shared Memory

| Attribute | Value |
|-----------|-------|
| **Source** | `CDoWorkGlassRecipeStep_Change::Run()` (DoWorkGlassRecipeStep.cpp:90-91) |
| **Data** | Cell count, stick count |
| **Transport** | Direct memory write via `GetShareMemoryMgrPtr()->GetSharedMemory()` |
| **Destination** | `VTM_SYSTEM::stGlassInfo.m_nCellCount`, `m_nStickCount` |
| **Transformation** | Copied from `CRecipeData::m_nCellCnt/m_nStickCnt` |

---

## 4. PLC Signal Flow

### 4.1 PLC Communication Architecture

```
Mitsubishi PLC (M-area bits/words)
    |  MC Protocol over TCP/IP
    v
CMcProtocolManager (CoreFunction/IOMapFunction/McProtocol/)
    |  Implements CIOMapImpl interface
    v
CIOFunction (CoreFunction/IOMapFunction/)
    |  Wraps CMcProtocolManager
    v
CIOMapControlMgr (Manager/IOMapControlMgr.h)
    |  CAuroraSubMgr, accessed via stAuroraMgrPool::pIOMapControlMgr
    v
CFunctionModuleEQState / CFunctionModuleEQStateChange / Sequence Steps
```

### 4.2 PLC Read (Import) Flow

| Attribute | Value |
|-----------|-------|
| **Source** | PLC M-area registers (e.g., `M4000+`) |
| **Data type** | Bit values (bool), Word values (long/float) |
| **Transport** | MC Protocol TCP/IP (Mitsubishi MELSEC), polled by `CMcAddressScannerImpl` threads |
| **Destination** | In-memory address map (`SADDRESSMAPDATA`) |
| **Transformation** | Raw PLC register data mapped to indexed arrays via CSV address files |

**Access pattern:**
```cpp
// Read sensor state
bool bSensor = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
```

**Key files:**
- `CoreFunction/IOMapFunction/McProtocol/McProtocolManager.h` -- PLC TCP/IP driver
- `CoreFunction/IOMapFunction/McProtocol/McCaddressScanner.cpp` -- address polling
- `Source/ALPHA/Manager/IOMapControlMgr.h` -- application-level PLC manager
- `Source/ALPHA/Manager/IOMapAddressDefine.h` -- PLC address constants

### 4.3 PLC Write (Export) Flow

| Attribute | Value |
|-----------|-------|
| **Source** | Sequence steps / EQ state machine |
| **Data type** | Bit/Word values |
| **Transport** | `CIOMapControlMgr::SetMemoryBitToDevice()` --> `CIOFunction` --> `CMcProtocolManager` --> MC Protocol TCP/IP |
| **Destination** | PLC M-area registers (e.g., `M4500+`) |
| **Transformation** | Boolean/integer values written to specific PLC addresses |

### 4.4 PLC --> Equipment State Decisions

| Attribute | Value |
|-----------|-------|
| **Source** | PLC sensor bits (via `CIOMapControlMgr`) |
| **Data type** | `bool` sensor states |
| **Transport** | `CFunctionModuleEQState::Is*SensorOn()` function calls |
| **Destination** | `enumEQState` (Ready/Load/Meas/Abnormal) |
| **Transformation** | Multiple PLC bit reads combined into equipment state enumeration |

**Sensor categories** (FunctionModuleEQState.h):
- Safety: `IsSafetySensorOn()`, `IsEQSensorClose()`, `IsDoorSensorClose()`
- Glass: `IsGlassSensorOn()`, `IsVacuumSensorOn()`, `IsLiftpinSensorUp/Down()`
- Loader: `IsLoaderXLoadSensorOn()`, `IsLoaderXMeasSensorOn()`
- Probe: `IsProbeContactSensorOn(iProbeUnit)`, `IsProbeExistSensorOn(iProbeUnit)`

[See 05_PLC.md](05_PLC.md) for full PLC address mapping.

---

## 5. Motion Data Flow

### 5.1 Motion Command Path

```
Recipe positions (stCoordinate)
    |  Sequence step calculates target position
    v
CMotorControlMgr::MoveCmdPosA(iAxis, dPos, bBlockMode, iSpeed)
    |  Wraps CMotorFunction
    v
CMotorFunction (CoreFunction/MotorFunction/)
    |  ACS SPiiPlus API calls (acsc_*)
    v
ACS Motion Controller (TCP/IP 10.0.0.100:701)
    |  Position command execution
    v
Physical axis movement + position feedback
```

### 5.2 Recipe Position --> Motor Command

| Attribute | Value |
|-----------|-------|
| **Source** | `stCoordinate` structs in recipe (dx, dy, ds, dz) |
| **Data type** | `double` position values (mm) |
| **Transport** | `CMotorControlMgr::MoveCmdPosA(iAxis, dPos, bBlockMode, iSpeed)` function call |
| **Destination** | ACS SPiiPlus controller via TCP/IP |
| **Transformation** | Recipe coordinates mapped to axis IDs, speed grade selected (HIGH/MID/LOW/HOME/LATCH) |

### 5.3 Motion Position Feedback

| Attribute | Value |
|-----------|-------|
| **Source** | ACS motion controller |
| **Data type** | `double` (current position), `long` (motion status), `bool` (move done) |
| **Transport** | `CMotorControlMgr::GetCurPos(iAxis)`, `CheckMoveDone()`, `WaitMotionEnd()` |
| **Destination** | Sequence steps, shared memory (`VTM_SYSTEM::stMotionStatus[100]`) |
| **Transformation** | Raw encoder values converted to mm via axis configuration |

### 5.4 Multi-axis Coordinated Moves

| Attribute | Value |
|-----------|-------|
| **Source** | Multiple target positions (`stMotorPosition`) |
| **Transport** | `CMotorControlMgr::MoveCmdPosMulti(pPos, iAxis, iSpeed, bDoneCheck)` |
| **Destination** | Multiple ACS axes simultaneously |
| **Transformation** | Position array sent to ACS buffer for synchronized motion |

**Key files:**
- `Source/ALPHA/Manager/MotorControlMgr.h` -- motion manager (90 axes, interlock positions)
- `CoreFunction/MotorFunction/MotorFunction.h` -- ACS SPiiPlus API wrapper

[See 04_Motion.md](04_Motion.md) for axis definitions and speed grades.

---

## 6. Vision / Alignment Data Flow

### 6.1 Glass Alignment Sequence

```
Recipe align params (S_Align)
    |  CDoWorkGlassSWAlignStep_AlignGlass::Run()
    v
AlignGlassSearch()  -- Move to align positions, grab camera images
    |  CAlignCameraMgr::SetParam_Camera*() + CLightControlMgr::SetLedLampIntensity()
    v
Pattern Detection (NI Vision: imaqCreateImage, pattern match)
    |  AlignGlassTZero() -- theta correction
    |  AlignGlassXYZero() -- XY offset correction
    v
tagAlignPos {dX, dY, dTheta}
    |  MaintControlMgr::SetGlassAlignXYR(dMovX, dMovY, dMovR)
    v
Motion correction applied + VTM_SYSTEM align offset stored
```

### 6.2 Camera Grab --> Pattern Match

| Attribute | Value |
|-----------|-------|
| **Source** | CCD camera via `CAlignCameraMgr` |
| **Data type** | Image (`imaqCreateImage(IMAQ_IMAGE_RGB, 7)`) |
| **Transport** | NI Vision API (IMAQ) function calls |
| **Destination** | Pattern matching algorithm |
| **Transformation** | Camera parameters applied first (exposure, gain, LED from `S_Align::m_sCcdSet`) |

### 6.3 Align Result --> Motion Correction

| Attribute | Value |
|-----------|-------|
| **Source** | Pattern detection result (2-point alignment) |
| **Data type** | `tagAlignPos` struct: dX, dY, dTheta (displacement values in mm/radians) |
| **Transport** | Direct function call to `CMotorControlMgr::MoveCmdPosA/R()` |
| **Destination** | Stage R-axis (theta correction), X/Y axes (position correction) |
| **Transformation** | Two detected mark positions used to calculate rotation angle (atan2) and XY offset |

### 6.4 Align Result --> Shared Memory

| Attribute | Value |
|-----------|-------|
| **Source** | Align calculation results |
| **Data type** | `double m_dAlignOffset[5]` (Glass), `double m_dAlignResult[3]` (Probe) |
| **Transport** | Write to `VTM_SYSTEM` shared memory |
| **Destination** | Available to all DLLs and subsequent sequence steps |

### 6.5 Glass Align Function (Matrix Calculation)

| Attribute | Value |
|-----------|-------|
| **Source** | Reference positions + detected positions |
| **Data type** | Position pairs (X, Y, Z) |
| **Transport** | `CGlassAlignFunction::SetParamRefPos()` / `SetParamDetPos()` --> `CalcAlignMatrix()` |
| **Destination** | Align transformation matrix |
| **Transformation** | `GetAlignedMotionPos(posX, posY, &NewX, &NewY)` -- applies matrix to any position |

**Key files:**
- `Source/ALPHA/DoWork/DoWorkGlassSWAlignStep.cpp` -- align step implementation
- `Source/ALPHA/DoWork/DoWorkGlassSWAlignStep.h` -- `AlignGlass()`, `AlignGlassSearch()`, `AlignGlassTZero()`, `AlignGlassXYZero()`
- `CoreFunction/GlassAlignFunction/GlassAlignFunction.h` -- matrix math
- `Source/ALPHA/Manager/AlignCameraMgr.h` -- camera control

[See 06_Vision.md](06_Vision.md) for camera and pattern match details.

---

## 7. Measurement Data Flow

### 7.1 Measurement Step Architecture

```
CDoWorkGlassMeasure
    |
    v
CDoWorkGlassMeasureStep (Step-based state machine)
    |
    +-- SetMeasureArea / MoveMeasureArea  -- position stage at cell
    +-- PtrnSearch                        -- pattern centering at cell
    +-- Autofocus                         -- Z-axis focus
    +-- PatternCentering                  -- fine position adjustment
    +-- Probe_Contact                     -- probe contact down
    +-- Measure (per type):
    |   +-- Measure_WAD                   -- spectral measurement
    |   +-- Measure_VISION                -- visual quality inspection
    |   +-- Measure_COLOR                 -- color measurement
    |   +-- Measure_ANGLE                 -- viewing angle
    |   +-- Measure_TSP                   -- touch screen panel test
    |   +-- Measure_VISION_COLOR          -- combined vision+color
    +-- Safety                            -- move to safe position
    +-- CheckMeasureComplete              -- loop control (next cell/stick)
```

### 7.2 Measurement Command to External Device

| Attribute | Value |
|-----------|-------|
| **Source** | `CDoWorkGlassMeasureStep_Measure_*::Run()` |
| **Data type** | Measurement command + cell info |
| **Transport** | `CCommTcpIpFactoryMgr::DoWorkInspectStart(enumTCPIpFactorytID_*, iMeasMode, nCellNum)` via TCP/IP |
| **Destination** | External instruments (PG, WAD, Vision, Color, Angle, TSP controllers) |
| **Transformation** | Measurement type + cell number encoded into device-specific TCP/IP command protocol |

### 7.3 Measurement Response from External Device

| Attribute | Value |
|-----------|-------|
| **Source** | External instrument TCP/IP response |
| **Data type** | Measurement completion event (`enumRecvEventID_INSP_*_OK/NG`) |
| **Transport** | `CCommTcpIpFactoryMgr::DoWorkInspectEnd()` / `DoWorkInspectComplate()` with timeout |
| **Destination** | Measurement step (blocking wait) |
| **Transformation** | TCP/IP response parsed, completion event signaled |

### 7.4 Measurement Result Collection

| Attribute | Value |
|-----------|-------|
| **Source** | Instrument response data |
| **Data type** | `RSRITEMDATA` struct -- contains LotID, GlassID, RecipeID, result path, panel data, measurement values (`RsDATA stData[100]`) |
| **Transport** | `CResultMgr::Updata_Data(PRSRITEMDATA result)` --> `API_Result_Upload()` |
| **Destination** | dResult DLL internal storage |
| **Transformation** | Per-cell measurement data accumulated in result collector |

**Instrument Communication Objects** (CommTcpIpFactoryMgr.h):
| Object | Count | Device |
|--------|-------|--------|
| `CCommTcpIpObjectUTG` | 2 | UTG thickness gauge |
| `CCommTcpIpObjectVision` | 2 | Visual quality inspection |
| `CCommTcpIpObjectColor` | 2 | Color measurement |
| `CCommTcpIpObjectAngle` | 2 | Viewing angle |
| `CCommTcpIpObjectWAD` | 1 | Spectral (WAD) |
| `CCommTcpIpObjectPG` | 1 | Pattern generator |
| `CCommTcpIpObjectAgingPG` | 1 | Aging pattern generator |
| `CCommTcpIpObjectLCR` | 1 | LCR meter |
| `CCommTcpIpObjectTSP` | 6 | Touch screen panel test |
| `CCommTcpIpObjectCIM` | 1 | CIM/MES host |
| `CCommTcpIpObjectSysTime` | 1 | System time sync |
| `CCommTcpIpObjectMotion` | 1 | Vision motion control |

### 7.5 Measurement Loop Control

| Attribute | Value |
|-----------|-------|
| **Source** | `CDoWorkGlassMeasureStep_CheckMeasureComplete::Run()` |
| **Data type** | Stick index, cell index, loop flag (`m_bMeasureLoop`) |
| **Transport** | Internal state in `CDoWorkGlassMeasureStep` |
| **Destination** | Loop back to `SetMeasureArea` or proceed to next step |
| **Transformation** | `m_nMeasureStick` incremented per stick, `m_nMeasureArea` per cell; `MakeCellID()` generates cell identification |

---

## 8. Result Data Flow

### 8.1 Result Save Path

```
Measurement data (per cell, accumulated)
    |  CDoWorkGlassResultStep_CompleteCheck::Run()
    v
CResultMgr::SaveResult(bSaveFile=true, bRemote=true, strStartTime)
    |  API_Result_SaveResult()  [dResult DLL]
    v
Result File (local disk)
    |  Path: VTM_SYSTEM::m_strResultPath
    v
FTP Upload (if CIM enabled)
    |  CTransporterResult::Run()
    v
MES/CIM Server
```

### 8.2 Result Save Trigger

| Attribute | Value |
|-----------|-------|
| **Source** | `CDoWorkGlassResultStep_CompleteCheck::Run()` (DoWorkGlassResultStep.cpp:59-103) |
| **Data type** | Accumulated measurement data + start time string |
| **Transport** | `CResultMgr::SaveResult()` --> `API_Result_SaveResult()` (dResult DLL export) |
| **Destination** | Local result file + optional remote upload |
| **Transformation** | Only saves if at least one measurement mode was used (WAD/VISION/COLOR/ANGLE/TSP/LCR) |

### 8.3 Result Data Structures

**RSRITEMDATA** (API_Result.h:88-118):
```cpp
struct RSRITEMDATA {
    TCHAR szLotID[32];
    TCHAR szGlassID[32];
    TCHAR szRecipeID[32];
    TCHAR szResultPath[260];
    TCHAR szResultFile[260];
    TCHAR szImageFile[260];
    int nMeasMode;          // Measurement mode
    int iCurArea;           // Current area number
    int iCurStep;           // Current step number
    int iPanelNo;           // Panel number
    int nCntData;           // Data count
    RsDATA stData[100];     // Measurement results
    TCHAR szCellData[2048]; // Panel data
};
```

**S_ResultData** (API_Result.h:27-79):
```cpp
struct S_ResultData {
    CString strBeginTime, strEndTime, strGlassID;
    double fRX_Avr, fGX_Avr, fBY_Avr;         // Color averages
    double fR_EFY_Avr, fG_EFY_Avr, fB_EFY_Avr; // Efficiency averages
    double fEFY_Out, fElectric_Out;              // Out-of-spec counts
    int nPercent[10];                            // Distribution percentages
    CString strAPD_Path;                         // APD data file path
};
```

**Per-instrument result structures:**
- `IPCDATA1` (API_Result.h:6-14): WAD spectral data -- InnerID, StartTime, EndTime, Judge, WRGB values
- `IPCDATA2` (API_Result.h:16-25): Vision quality data -- defect count, name, XY position, RGB, image path

### 8.4 Result File Output

| Attribute | Value |
|-----------|-------|
| **Source** | dResult DLL internal accumulator |
| **Data type** | Formatted result data |
| **Transport** | File I/O (`API_Result_SaveResult()`) |
| **Destination** | `VTM_SYSTEM::m_strResultPath` directory |
| **Transformation** | Measurement data formatted into output files; `API_Result_SaveResultDone()` for completion flag |

---

## 9. CIM/MES Data Flow

### 9.1 CIM Communication Architecture

```
CIM/MES Host Server
    ^  |
    |  v  TCP/IP (bidirectional)
CCommTcpIpObjectCIM (inherits CCommEthFunction)
    |  xNetCallbackRecieveData() -- async receive
    |  Cmd_Send() -- command send
    v
CDoWorkSequenceCIM --> CDoWorkSequenceCIMStep_* steps
```

### 9.2 CIM Sequence (Online Remote Mode)

The CIM sequence runs as a parallel track managed by `CDoWorkSequenceCIM` (via `CSequenceManager::GetDoWorkSequenceCIM()`).

**CIM Step Flow** (DoWorkSequenceCIMStep.h):
```
CIMStep_None          -- Initialize flags
CIMStep_Initial       -- AT1 handshake, EQPSTART, alarm clear, time sync
CIMStep_Initial2      -- Status report (STS0104, STS0204), homing, safety check
CIMStep_Loading       -- Move to load position (READY -> LOAD state change)
CIMStep_CheckLoadReq  -- Wait for host LDREQ
CIMStep_ChangeLoad    -- Physical load operation
CIMStep_LoadReq       -- Send LDCPL to host
CIMStep_PPIDREQ       -- Request recipe ID from host
CIMStep_LOTIDREQ      -- Request lot ID from host
CIMStep_GLSIDREQ      -- Request glass ID from host
CIMStep_PRCSReq       -- Process start request
CIMStep_ChangeReady   -- Move to READY state
CIMStep_ChangeMeas    -- Move to MEAS state
CIMStep_MeasStart     -- Notify host: measurement started
CIMStep_MeasDone      -- Notify host: measurement complete
CIMStep_GDTReq        -- Send glass data (results/judge)
CIMStep_CheckUnLoadReq -- Wait for unload request
CIMStep_UnLoadDone    -- Unload complete
CIMStep_CheckUnLoadCpl -- Verify unload completion
CIMStep_LoadReady     -- Ready for next glass
```

### 9.3 CIM Commands

| Direction | Command | Function | Description |
|-----------|---------|----------|-------------|
| EQ -> Host | `dfAT_1_OK` | `SendMainStatus()` | Equipment status report |
| EQ -> Host | `dfEQPSTART` | `Process_EQPStart()` | Equipment ready |
| EQ -> Host | `dfPPIDREQ` | `Cmd_Send_PPIDREQ()` | Request recipe ID |
| EQ -> Host | `dfSTS_DATAJUDGE_OK/NG` | `Cmd_Send_DataJudge()` | Glass judge result |
| EQ -> Host | `dfSTS0104/0204/0304` | Status commands | State transitions |
| Host -> EQ | `dfTIMESET` | `Process_SetMCCTime()` | Time synchronization |
| Host -> EQ | `dfPPID` | `Process_PPID()` | Recipe ID assignment |
| Host -> EQ | `dfLOTID` | `Process_LOTID()` | Lot ID assignment |
| Host -> EQ | `dfGLSID` | `Process_GLSID()` | Glass ID assignment |

### 9.4 CIM Result Upload (FTP)

| Attribute | Value |
|-----------|-------|
| **Source** | Result files in `m_strResultPath` |
| **Data type** | APD data files, result files |
| **Transport** | FTP upload via `CTransporterResult::Run()` (runs in dedicated thread) |
| **Destination** | CIM/MES FTP server |
| **Transformation** | Files copied from local result path to FTP remote path |

**Key file:** `Source/ALPHA/TransporterResult.h`, `Source/ALPHA/CommTcpIpObjectCIM.cpp` (SendAPDFileToFTP at line 1676)

[See 03_AutoSequence.md](03_AutoSequence.md) for CIM integration in auto sequence.

---

## 10. Shared Memory Data Flow

### 10.1 VTM_SYSTEM (Main Shared Memory)

| Attribute | Value |
|-----------|-------|
| **Source** | All managers and sequence steps |
| **Data type** | `_stVTM_SYSTEM` struct (~200+ fields) |
| **Transport** | Memory-mapped file (`CCLINK`), accessed via `CShareMemoryMgr::GetSharedMemory()` |
| **Destination** | All DLLs and threads (SMEM.dll provides external access) |
| **Transformation** | Direct read/write with critical section protection (`m_cs_set/m_cs_get`) |

**Key sections of VTM_SYSTEM** (CoreFunction/ShareMemFunction/ShareMem.h:2044):
| Field | Type | Purpose |
|-------|------|---------|
| `stProbeInfo[5]` | `PROBEINFO` | Probe unit status (5 probes) |
| `stSystem[MAX_SUBMODULE]` | `SYSTEM` | Sub-module states |
| `stGlassInfo` | `ECS_INFO` | Glass/cell information |
| `stResult` | `RESULT_INFO` | Result summary |
| `stMotionStatus[100]` | `MOTIONSTATUS` | Axis positions/status |
| `nCellStatus[100][50]` | `int` | Cell measurement status |
| `m_dAlignOffset[5]` | `double` | Glass align offsets |
| `m_dAlignResult[3]` | `double` | Probe align results |
| `dDisplacement_X/Y/Z/S` | `double` | Alignment displacement |
| `m_strResultPath` | `TCHAR[260]` | Result file output path |
| `sz_MeasStartTime` | `TCHAR[16]` | Measurement start timestamp |

### 10.2 VTM_RECIPE (Recipe Shared Memory)

| Attribute | Value |
|-----------|-------|
| **Source** | `CRecipeEditMgr` |
| **Data type** | `VTM_RECIPE` struct (RecipeShareMem.h) |
| **Transport** | Named shared memory (`HBS_RECIPE_SHARED_MEMORY`) |
| **Destination** | Recipe DLL (`dfcRecipe64`) and other modules |

**Contents:**
- `S_RecipeRunState m_sRunState` -- current recipe execution state
- `S_Align m_sAlign` -- alignment parameters

### 10.3 Shared Memory Write Patterns

| Writer | Field | When |
|--------|-------|------|
| Recipe step | `stGlassInfo.m_nCellCount/StickCount` | Recipe change |
| Align step | `m_dAlignOffset[]`, `m_dAlignResult[]` | After alignment |
| Sequence steps | `ProcessStatus` (SPS_IDLE/PROCESS) | Step start/end |
| CIM step | `MachineMode` (OFFLINE/ONLINE_REMOTE) | CIM handshake |
| Measure step | `nCellStatus[100][50]` | Per-cell result |
| Alarm monitor | `AlarmStatus` | Alarm detection |

---

## 11. stAuroraMgrPool -- Central Data Hub

All managers are connected through the `stAuroraMgrPool` struct, which is passed to every sequence step and manager via constructor injection.

```
stAuroraMgrPool
    +-- strAppPath, strConfigPath, strRecipePath, strLogPath
    +-- pRecipeEditMgr       -- Recipe data access
    +-- pIOMapControlMgr     -- PLC I/O
    +-- pMotorControlMgr     -- Motion control
    +-- pShareMemoryMgr      -- Shared memory
    +-- pCommTcpIpFactoryMgr -- TCP/IP instrument communication
    +-- pAlignCameraMgr[N]   -- Camera control
    +-- pLightControlMgr     -- LED lighting
    +-- pSeqTactTimeMgr      -- Tact time measurement
    +-- pMaintControlMgr     -- Maintenance/config
    +-- pAlarmMonitorMgr     -- Alarm monitoring
    +-- pSensorMonitorMgr    -- Sensor monitoring
    +-- pSystemConfigMgr     -- System configuration
    +-- pPositionConfigMgr   -- Position teaching data
    +-- pPatternMatchMgr     -- Pattern matching
    +-- ... (20+ managers total)
```

**Key file:** `Source/ALPHA/AuroraMgrPool.h`

Each `CDoWorkGlassSequenceImpl` (base class for all sequence steps) receives this pool at construction and provides accessor methods:
- `GetRecipeEditMgrPtr()`
- `GetIOMapControlMgrPtr()`
- `GetMotorControlMgrPtr()`
- `GetShareMemoryMgrPtr()`
- `GetCommTcpIpFactoryMgr()`
- `GetAlignCameraMgr(index)`
- etc.

---

## 12. Complete Data Flow Summary

### Phase 1: Initialization
```
User Button --> CSequenceManager::EngineStart()
    --> CDoWorkGlassSequenceEngine::ThreadRun()
    --> CMCCLogMgr::SetMCC_Action(MCC_A_AUTO_CYCLE)
    --> CSeqTactTimeMgr::SetTotalSeqTimeST()
```

### Phase 2: Adsorption + Recipe
```
CDoWorkGlassAdsorption: PLC write (vacuum ON) --> PLC read (vacuum sensor check)
CDoWorkGlassRecipe: Recipe file --> S_RecipeInfo --> TCP/IP DoWorkRecipeOpen() to each device
```

### Phase 3: Alignment
```
Recipe align coords --> Move stage to align pos (Motor)
Camera grab --> NI Vision pattern match --> tagAlignPos (dX, dY, dTheta)
Theta correction --> Motor R-axis move
XY correction --> Motor X/Y-axis move
Result --> VTM_SYSTEM::m_dAlignOffset[] + CGlassAlignFunction matrix
```

### Phase 4: Measurement
```
For each stick:
  For each cell:
    Recipe cell position --> Motor multi-axis move
    Camera pattern centering --> Motor fine adjust
    Autofocus --> Motor Z-axis
    Probe contact --> PLC write + sensor check
    DoWorkInspectStart(device) --> TCP/IP command to instrument
    Wait for completion --> TCP/IP response event
    Result data --> CResultMgr::Updata_Data(RSRITEMDATA)
```

### Phase 5: Result + Unload
```
CResultMgr::SaveResult() --> dResult DLL --> local file output
CIM::Cmd_Send_DataJudge() --> TCP/IP --> MES host judge report
CTransporterResult::Run() --> FTP upload to CIM server
CDoWorkGlassUnload: Motor move to unload pos --> PLC unload signals
```

---

## 13. Transport Mechanism Summary

| Mechanism | Usage | Protocol |
|-----------|-------|----------|
| Function call | Manager-to-manager, step-to-step | Direct C++ method call |
| Win32 Event | Thread synchronization (Start/Complete/Pause) | `SetEvent()`/`WaitForSingleObject()` |
| Windows Message | UI updates from worker threads | `SendMessageToUI()`/`PostMessageToUI()` via `CWindowEventHandler` |
| TCP/IP | PLC (MC Protocol), ACS motion, instruments, CIM | Custom protocols per device |
| Shared Memory | Cross-DLL data sharing | Named memory-mapped file (`CCLINK`, `HBS_RECIPE_SHARED_MEMORY`) |
| File I/O | Recipe load/save, result output | Binary (`.rcp`), INI, CSV, result files |
| FTP | CIM result upload | Standard FTP via `CCommFtpFunction` |
| Critical Section | Thread-safe memory access | `CCriticalSection` per manager |

---

## 14. Key Source Files Reference

| File | Role in Data Flow |
|------|-------------------|
| `Source/ALPHA/DlgSubOperate.cpp` | UI entry point (Start/Cycle/State change buttons) |
| `Source/ALPHA/Manager/SequenceManager.h` | Singleton orchestrator |
| `Source/ALPHA/DoWork/DoWorkGlassSequenceEngine.cpp` | Sequence thread + step execution |
| `Source/ALPHA/DoWork/DoWorkGlassRecipeStep.cpp` | Recipe distribution to devices |
| `Source/ALPHA/DoWork/DoWorkGlassSWAlignStep.cpp` | Glass alignment (camera + motion) |
| `Source/ALPHA/DoWork/DoWorkGlassMeasureStep.cpp` | Measurement execution per cell |
| `Source/ALPHA/DoWork/DoWorkGlassResultStep.cpp` | Result save trigger |
| `Source/ALPHA/DoWork/DoWorkSequenceCIMStep.cpp` | CIM/MES protocol steps |
| `Source/ALPHA/Manager/CommTcpIpFactoryMgr.h` | TCP/IP instrument factory (13 device types) |
| `Source/ALPHA/Manager/IOMapControlMgr.h` | PLC communication manager |
| `Source/ALPHA/Manager/MotorControlMgr.h` | Motion control manager |
| `Source/ALPHA/Manager/ResultMgr.cpp` | Result DLL wrapper (Singleton) |
| `Source/ALPHA/Manager/ShareMemoryMgr.h` | Shared memory access |
| `Source/ALPHA/CommTcpIpObjectCIM.h` | CIM/MES TCP/IP protocol |
| `Source/ALPHA/TransporterResult.h` | FTP result upload |
| `Source/ALPHA/Recipe/RecipeEditMgr.h` | Recipe manager |
| `Source/ALPHA/Recipe/RecipeData.h` | Recipe data structures |
| `Source/ALPHA/FunctionModule/FunctionModuleEQState.h` | Equipment state (PLC sensor checks) |
| `Source/ALPHA/FunctionModule/FunctionModuleEQStateChange.h` | State transitions (R2L, L2M, etc.) |
| `Source/ALPHA/AuroraMgrPool.h` | Central manager pool struct |
| `Source/Include/API_Result.h` | Result data structures (RSRITEMDATA, S_ResultData) |
| `Source/Include/ShareMem.h` | Shared memory enums and constants |
| `CoreFunction/ShareMemFunction/ShareMem.h` | VTM_SYSTEM struct definition |
| `Source/Include/RecipeShareMem.h` | VTM_RECIPE struct |
| `CoreFunction/IOMapFunction/McProtocol/McProtocolManager.h` | PLC MC Protocol driver |
| `CoreFunction/GlassAlignFunction/GlassAlignFunction.h` | Alignment matrix calculation |
