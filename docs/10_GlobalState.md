# 10. Global State Analysis - ALPHA HMI

This document catalogs all global data in the ALPHA HMI project: singletons, extern globals, shared memory structures, static variables, function pointer typedefs, and configuration state.

---

## 1. Singleton Instances

All singletons follow the same pattern: `static T* m_pInstance` initialized to `nullptr`, lazy-created via `GetInstance()`, destroyed via `DelInstance()`.

| Class | Header | Impl | Purpose |
|---|---|---|---|
| `CSystemParamShare` | `Source/ALPHA/SystemParamShare.h:179` | `SystemParamShare.cpp:12` | System-wide runtime parameters (auto mode, glass ID, cycle state, folder paths) |
| `CSequenceManager` | `Source/ALPHA/Manager/SequenceManager.h:76` | `SequenceManager.cpp:25` | Sequence engine orchestrator (initialization, glass work, EQ state, interlock) |
| `CWindowEventHandler` | `Source/ALPHA/WindowEventHandler.h:158` | `WindowEventHandler.cpp:12` | UI window message routing (HWND registry for title bar, frame, sequence items) |
| `CAlarmController` | `Source/ALPHA/Alarm/AlarmController.h:32` | `AlarmController.cpp:13` | Alarm raise/clear/query controller |
| `CRecipeMgr` | `Source/ALPHA/Manager/RecipeMgr.h:18` | `RecipeMgr.cpp:7` | Recipe shared memory access (`VTM_RECIPE`) |
| `CResultMgr` | `Source/ALPHA/Manager/ResultMgr.h:21` | `ResultMgr.cpp:10` | Measurement result collection and save |
| `CSACCMgr` | `Source/ALPHA/Manager/SACCMgr.h:37` | `SACCMgr.cpp:17` | SACC (Probe card RFID management) |
| `CMCCLogMgr` | `Source/ALPHA/Manager/MCCLogMgr.h:479` | `MCCLogMgr.cpp:21` | MCC (Machine Control Computer) logging, FDC shared memory |

### Singleton Access Pattern (all identical)

```cpp
// In header:
private:
    static CSystemParamShare *m_pInstance;
public:
    static CSystemParamShare *GetInstance();
    static void               DelInstance();

// In cpp:
CSystemParamShare *CSystemParamShare::m_pInstance = nullptr;
CSystemParamShare *CSystemParamShare::GetInstance()
{
    return m_pInstance ? m_pInstance : (m_pInstance = new CSystemParamShare);
}
```

---

## 2. Extern Global Variables

Declared in `Source/Include/AlarmDefine.h:129-133`:

| Variable | Type | Purpose | Declared |
|---|---|---|---|
| `g_sAppPath` | `CString` | Application executable directory path | `AlarmDefine.h:129` |
| `g_csQuery` | `CCriticalSection` | Critical section for database/query synchronization | `AlarmDefine.h:130` |
| `gf_Initialize()` | `void()` | Global initialization function | `AlarmDefine.h:131` |
| `gf_MakeDir()` | `BOOL(CString)` | Directory creation utility | `AlarmDefine.h:132` |
| `gf_MakeCurTimeString()` | `CString()` | Current time string formatter | `AlarmDefine.h:133` |

Declared in `Source/ALPHA/ALPHA.h:34`:

| Variable | Type | Purpose | Declared |
|---|---|---|---|
| `theApp` | `CALPHAApp` | MFC application singleton instance | `ALPHA.h:34`, defined `ALPHA.cpp:42` |

---

## 3. File-Scope Global Variables

| Variable | Type | File | Line | Purpose |
|---|---|---|---|---|
| `g_bShowImage` | `bool` | `Source/ALPHA/DlgAlignCCD.cpp` | 18 | Controls CCD image display flag |

---

## 4. Shared Memory Structures

### 4.1 VTM_SYSTEM (Main Shared Memory)

- **Defined:** `Source/Include/ShareMem.h:1727-1926`
- **Shared Memory Name:** `_T("CCLINK")` (`WBS_SHARE_MEM_STRING` / `VTM_SHARE_MEM_STRING`)
- **Access:** `CShareMemFunction::GetSharedMemory()` -> `CShareMemoryMgr::GetSharedMemory()`
- **Memory Map Handle:** `CShareMemFunction::m_hVTMSysMap` (Win32 memory-mapped file)
- **Thread Safety:** `CCriticalSection m_cs_set` / `m_cs_get` in `CShareMemoryMgr`

Key members of `VTM_SYSTEM`:

| Member | Type | Purpose |
|---|---|---|
| `nMeasMode` | `int` | Current measurement mode |
| `m_bVirtualMode` | `BOOL` | Virtual/simulation mode flag |
| `hLog` | `HWND` | Log window handle |
| `hMetrosysView` | `HWND` | Main view window handle |
| `hCDlgNaviOperate` | `HWND` | Operate dialog handle |
| `stSystem[MAX_SUBMODULE]` | `SYSTEM[]` | SubModule status array |
| `stMonitor` | `MONITOR` | System monitor data |
| `stLotInfo` | `LOTINFO` | Current lot information |
| `stTitle` | `TITLEPANEL` | Title panel display data |
| `stInterLock` | `SYSINTERLOCK` | System interlock status |
| `stGlassInfo` | `ECS_INFO` | Current glass information (ID, recipe, state, size) |
| `stResult` | `RESULT_INFO` | Measurement result info |
| `stMeasGrp` | `MEASGROUP` | Current measurement group data |
| `stEquip` | `EQUIP` | Equipment status |
| `stMotionStatus[100]` | `MOTIONSTATUS[]` | Motion axis status for all 100 axes |
| `stSysPath` | `PATHINFO` | System file paths |
| `szLastGlassID[256]` | `char[]` | Last processed glass ID |
| `szLastRcpName[256]` | `char[]` | Last used recipe name |
| `iCurGroup` | `int` | Current measurement group index |
| `nJobCount` | `int` | Total cycle count |
| `nCurrentCycleCount` | `int` | Current cycle count |
| `m_bMeasMultiRcp` | `BOOL` | Multi-recipe measurement flag |
| `m_bDarkMeas` | `BOOL` | Dark measurement flag |
| `m_bReWork` | `BOOL` | Re-measurement flag |
| `m_bGlassJudge` | `BOOL` | Glass judgement flag |
| `m_bGlassLoadState` | `BOOL` | Glass load state flag |
| `iMachineStatus` | `int` | Machine status code |
| `dDisplacement_X/Y/Z/S` | `double` | Alignment displacement values |
| `dDisplacement_Angle` | `double` | Alignment rotation displacement |
| `stRecipeAlignMarkPos[][]` | `COORDINATE[][]` | Recipe align mark positions |
| `stDetectAlignMarkPos[][]` | `COORDINATE[][]` | Detected align mark positions |
| `m_bAlignMarkDetected` | `BOOL` | Align mark detection flag |
| `m_bValidation` | `BOOL` | Validation mode flag |
| `m_bCIMAlarmReported` | `BOOL` | CIM alarm reported flag |
| `m_bAutoAgingMode` | `BOOL` | Auto aging mode flag |
| `nCellStatus[100][50]` | `int[][]` | Cell inspection status matrix |
| `m_bMotionDisable` | `BOOL` | Motion disable flag |
| `m_nLanguage` | `int` | UI language (0=Korean, 1=English, 2=Chinese) |

### 4.2 VTM_RECIPE (Recipe Shared Memory)

- **Defined:** `Source/Include/RecipeShareMem.h:13-31`
- **Shared Memory Name:** `_T("HBS_RECIPE_SHARED_MEMORY")` (`df_RECIPE_SHARE_MEM_STRING`)
- **Access:** `CRecipeMgr::m_pSharedRecipe`
- **API:** `Recipe_GetSharedMemory()` from `API_RECIPE.h`

Members:

| Member | Type | Purpose |
|---|---|---|
| `m_sRunState` | `S_RecipeRunState` | Recipe run state (command, data, status) |
| `m_sAlign` | `S_Align` | Alignment data (glass/probe positions, CCD settings) |

### 4.3 VTM_FOR_FDC (FDC Shared Memory)

- **Defined:** `Source/Include/FDCSheardDefine.h:102-120`
- **Shared Memory Name:** `_T("KMAC_Shared_Memory_For_FDC_V2")` (`VTM_SHARE_MEM_STRING_FDC`)
- **Access:** `CMCCLogMgr::CreateMemoryMapFDC()`

Members:

| Member | Type | Purpose |
|---|---|---|
| `FDCValue.nMainDataCount` | `int` | Count of main SW data items |
| `FDCValue.stMainWData[1000]` | `RDATA[]` | Main SW data (name, data, max, min) |
| `FDCValue.lLiveCheck` | `DWORD` | FDC SW tick count (heartbeat) |
| `FDCValue.stFDCWData[1000]` | `RDATA[]` | FDC SW data (temperature, vacuum, EFU, etc.) |

### 4.4 VTM_MCC (MCC Shared Memory)

- **Defined:** `Source/Include/SheardDefineMCC.h:96-110`
- **Shared Memory Name:** `_T("VTM_MCC_Shared_Memory_KMAC")` (`VTM_SHARE_MEM_STRING_MCC`)

Members:

| Member | Type | Purpose |
|---|---|---|
| `nEQType` | `UINT` | Equipment type |
| `bMCCTransfer` | `BOOL` | MCC transfer enable flag |
| `stMCCInfoEQ` | `MCC_EQINFO` | Equipment ID, glass ID, recipe ID, lot ID |
| `stMCCDataEQ_ILog` | `MCC_EQDATA_I` | Motor info for 100 axes, data logging |
| `stMCCDataEQ` | `MCC_EQDATA_T` | Action log (event flag, time, step) |
| `m_queue_data[10000]` | `MCC_EQDATA_T[]` | Action log queue (10000 entries) |
| `stMCCOption` | `MCC_OPTION` | Simulation mode, log periods |

---

## 5. Central Orchestrator: CAuroraMgr

`CAuroraMgr` is not a singleton but acts as the central hub. A single instance is created in `CALPHAApp::InitInstance()` and stored in `CALPHAApp::m_pAuroraMgr`.

- **Defined:** `Source/ALPHA/AuroraMgr.h`
- **Created:** `Source/ALPHA/ALPHA.cpp` (InitInstance)

### 5.1 stAuroraMgrPool (Manager Pointer Collection)

Defined in `Source/ALPHA/AuroraMgrPool.h:101-173`. This struct holds pointers to all SubMgr instances:

| Pointer | Type | Purpose |
|---|---|---|
| `pLogInControlMgr` | `CLogInControlMgr*` | User login/permission control |
| `pSystemConfigMgr` | `CSystemConfigMgr*` | System configuration (INI parameters) |
| `pNetworkConfigMgr` | `CNetworkConfigMgr*` | Network/IP configuration |
| `pPositionConfigMgr` | `CPositionConfigMgr*` | Motion position configuration |
| `pLockCtrlMgr` | `CLockControlMgr*` | UI lock control |
| `pRecipeEditMgr` | `CRecipeEditMgr*` | Recipe editing |
| `pResultCollectMgr` | `CResultCollectMgr*` | Result data collection |
| `pIOMapControlMgr` | `CIOMapControlMgr*` | PLC I/O map control |
| `pMotorControlMgr` | `CMotorControlMgr*` | Motor/motion control |
| `pPatternMatchMgr` | `CPatternMatchMgr*` | Pattern matching for alignment |
| `pLightControlMgr` | `CLightControlMgr*` | Lamp/LED control |
| `pVCRControlMgr` | `CVCRControlMgr*` | VCR/barcode reader control |
| `pCIMControlMgr` | `CCIMControlMgr*` | CIM/MES protocol control |
| `pAlignMatrixMgr` | `CAlignMatrixMgr*` | Alignment matrix calculations |
| `pAlignCameraMgr[4]` | `CAlignCameraMgr*[]` | Align CCD camera control (4 cameras, `MAIN_CCD_MAX_NUM`) |
| `pReviewCameraHead1Mgr` | `CReviewCameraHead1Mgr*` | Review camera head 1 |
| `pReviewCameraHead2Mgr` | `CReviewCameraHead2Mgr*` | Review camera head 2 |
| `pShareMemoryMgr` | `CShareMemoryMgr*` | VTM_SYSTEM shared memory wrapper |
| `pMeasureItemMgr` | `CMeasureItemMgr*` | Measurement item management |
| `pCommTcpIpFactoryMgr` | `CCommTcpIpFactoryMgr*` | TCP/IP communication factory (13 device types) |
| `pCommTcpIPMgr` | `CCommTcpIPMgr*` | TCP/IP connection management |
| `pSensorMonitorMgr` | `CSensorMonitorMgr*` | Hardware sensor monitoring |
| `pAlarmMonitorMgr` | `CAlarmMonitorMgr*` | PLC alarm monitoring threads |
| `pSeqTactTimeMgr` | `CSeqTactTimeMgr*` | Sequence tact time tracking |
| `pVisionConfigMgr` | `CVisionConfigMgr*` | Vision/CCD configuration |
| `pTriggerControlMgr` | `CTriggerControlMgr*` | Trigger control |
| `pSEMControlMgr` | `CSEMControlMgr*` | SEM equipment control |
| `pMaintControlMgr` | `CMaintControlMgr*` | Maintenance control |

### 5.2 CAuroraMgr Internal State

| Member | Type | Purpose |
|---|---|---|
| `m_subMgr` | `vector<CAuroraSubMgr*>` | All registered SubMgr instances |
| `m_subDlg` | `map<UINT, CWnd*>` | Registered sub-dialog windows |
| `m_systemInfo` | `stSystemInfo` | App name and version |
| `m_loginInfo` | `stLoginInfo` | Current user name and level |
| `m_mainFrameInfo` | `stMainFrameInfo` | Main frame dimensions (1440x900 default) |
| `m_AuroraMgrPool` | `stAuroraMgrPool` | All manager pointers (see above) |

---

## 6. CSystemParamShare (Singleton Runtime State)

**File:** `Source/ALPHA/SystemParamShare.h`

This singleton holds runtime state accessible from anywhere:

### 6.1 Path Configuration

| Member | Type | Purpose |
|---|---|---|
| `m_strAppFolderPath` | `CString` | Application root folder |
| `m_strConfigFolderPath` | `CString` | Configuration folder |
| `m_strLoggerFolderPath` | `CString` | Log file folder |
| `m_strDefaultRecipeFolderPath` | `CString` | Default recipe folder |
| `m_strRecipeFolderPath` | `CString` | Active recipe folder |
| `m_strResultFolderPath` | `CString` | Result data folder |
| `m_strAlarmFolderPath` | `CString` | Alarm data folder |

### 6.2 System State Flags

| Member | Type | Default | Purpose |
|---|---|---|---|
| `m_bInitialized` | `bool` | `false` | System initialization complete flag |
| `m_bAutoMode` | `bool` | `false` | Auto/Manual mode (true = Auto Run) |
| `m_bForceStopByUser` | `bool` | `false` | User-initiated forced stop |
| `m_bCycleState` | `bool` | `false` | Cycle inspection mode active |
| `m_bTestMode` | `bool` | `false` | Test mode flag |

### 6.3 Glass/Lot Information

| Member | Type | Default | Purpose |
|---|---|---|---|
| `m_strInnerID` | `CString` | `"123456"` | Internal tracking ID |
| `m_strGlassID` | `CString` | `"ABCDEFG1234"` | Current glass ID |
| `m_strCellID` | `CString` | - | Current cell ID |
| `m_strLotID` | `CString` | `"ABCDEFG1234"` | Current lot ID |
| `m_strRecipeID` | `CString` | `"UNKNOWN"` | Current recipe ID |
| `m_strStepID` | `CString` | `"UNKNOWN"` | Current step ID |
| `m_strSlotID` | `CString` | `"UNKNOWN"` | Current slot ID |
| `m_strCassetteID` | `CString` | `"UNKNOWN"` | Current cassette ID |
| `m_strOperID` | `CString` | `"UNKNOWN"` | Current operator ID |
| `m_strGlassRot` | `CString` | `"0"` | Glass rotation state |
| `m_nCycleCount` | `long` | `1` | Cycle count |

---

## 7. CSystemConfigMgr (INI-based Config State)

**File:** `Source/ALPHA/Manager/SystemConfigMgr.h`  
**Base:** `CAuroraSubMgr` (SubMgr pattern, not singleton)  
**Access:** `CAuroraMgr::GetSystemConfigMgrPtr()`

| Member | Type | Purpose |
|---|---|---|
| `m_strPgmLicense` | `CString` | Program license key |
| `m_bDuplicateExecution` | `bool` | Allow duplicate process execution |
| `m_bAutoChangeInlineMode` | `bool` | Auto-switch to inline mode on start |
| `m_iLanguageType` | `int` | UI language index |
| `m_bUseAlarmMonitor` | `bool` | Enable alarm monitoring |
| `m_bUseHardwareSensor` | `bool` | Enable hardware sensor check |
| `m_bUseHardwareMotor` | `bool` | Enable motor control |
| `m_bUseTeachKeyMonitor` | `bool` | Enable teach key monitoring |
| `m_bUseUploadResultDataAndImageSave` | `bool` | Enable result/image upload |
| `m_strUploadResultDataAndImageSaveFolderPath` | `CString` | Upload destination path |
| `m_bUseBackupResultDataAndImageSave` | `bool` | Enable backup save |
| `m_strBackupResultDataAndImageSaveFolderPath` | `CString` | Backup destination path |
| `m_strUploadCIMFolderPath` | `CString` | CIM data upload path |
| `m_bCommunicationTypeString` | `bool` | String packet communication mode |
| `m_bUseVisionColorInsp` | `bool` | Enable vision color inspection |
| `m_strRecipePath` | `CString` | Recipe file path |
| `m_strLogPath` | `CString` | Log file path |
| `m_strResultPath` | `CString` | Result file path |
| `m_bModelChangeAlarm` | `bool` | Alarm on model change |
| `m_bCurGlsOneAlarm` | `bool` | Single glass alarm mode |
| `m_bCIMAlarmNotReport` | `bool` | Suppress CIM alarm reporting |
| `m_bGlassExist` | `bool` | Glass existence state |
| `m_strStopGlassID` | `CString` | Glass ID to stop at |

---

## 8. Function Pointer Typedefs

Defined in `Source/ALPHA/AuroraFuncDefine.h:6-8`:

```cpp
typedef void(*CallbackClassFunction1)(LPVOID);                    // Sequence callback
typedef void(*CallbackClassFunction2)(LPVOID, LPVOID);            // UI callback
typedef void(*CallbackClassFunction3)(LPVOID, LPVOID, LPVOID);   // UI callback (3-arg)
```

These typedefs are used for callback registration between sequence engine components and UI elements.

---

## 9. Static Variables (File-Scope)

Many dialog `.cpp` files declare static arrays for grid column names and sizes. These are file-local and not globally accessible.

### Pattern: UI Grid Column Definitions

| File | Variables | Purpose |
|---|---|---|
| `DlgMainForm.cpp:40-93` | `ColumnName[]`, `ColumnSize[]`, `ColumnName2[]`, `ColumnMotionName[]` | Main form grid columns |
| `DlgSubOperate.cpp:55-225` | `ColumnName[]` through `ColumnProbe[]` | Operate screen grid columns |
| `DlgSubMaint.cpp:28-109` | `ColumnName[]` through `ColumnIOOutput[]` | Maintenance screen grid columns |
| `DlgSubConfig.cpp:20-68` | `ColumnName[]`, `RowName[]`, `ColumnNameHeadOffset[]` | Config screen grid columns |
| `DlgTitleBar.cpp:21-43` | `ColumnName[]`, `ColumnSize[]` | Title bar grid columns |
| `DlgSubConfigMotionFile.cpp:12-36` | `ColumnName[]`, `ColumnSize[]` | Motion config grid columns |
| `DlgSubConfigModuleFile.cpp:18-64` | `ColumnName[]`, `ColumnName2[]` | Module config grid columns |
| `DlgSubMaint_IOFile.cpp:13-19` | `ColumnName[]`, `ColumnSize[]` | IO maintenance grid columns |
| `D_MeasureCycle.cpp:19-35` | `ColumnMeas[]`, `ColumnName[]` | Measure cycle grid columns |
| `D_SetLampTime.cpp:26-39` | `ColumnName[]`, `ColumnSize[]` | Lamp time setting grid columns |
| `D_LogInMgr.cpp:10-16` | `ColName[]`, `strLevel[]` | Login manager grid/level names |

---

## 10. Precompiled Header Constants

Defined in `Source/ALPHA/pch.h`:

| Constant | Value | Purpose |
|---|---|---|
| `MAIN_STATE_TIMER_ID` | `1001` | Main state timer ID |
| `MAIN_CCD_MAX_NUM` | `4` | Maximum number of CCD cameras |

Also defines `CCriticalSectionSync` RAII wrapper class for automatic lock/unlock.

---

## 11. Key Enumerations (Global State-Related)

### Equipment State (`ShareMem.h:86-94`)

```
EQUIP_STATE_VALUE: ESV_IDLE(0), ESV_READY, ESV_RUN, ESV_ERROR, ESV_SKIPMODE, ESV_OUTOFSERVICE, ESV_NOT_INITIALIZED
```

### Process Status (`ShareMem.h:57-67`)

```
PROCESS_STATUS: PROCESS_NONE(-1), PROCESS_SKIP, PROCESS_READY, PROCESS_RUNNING, PROCESS_COMPLETE, PROCESS_ALIGN_COMPLETE, PROCESS_ERROR
```

### System Process Status (`ShareMem.h:96-107`)

```
SPS_IDLE(0), SPS_PROCESS, SPS_INITIAL, SPS_MAINT, SPS_WAFERTRANSFER, SPS_CYCLETEST, SPS_PARTICLETEST, SPS_MAINTING, SPS_REMEASURE, SPS_EXITING
```

### Host Mode (`ShareMem.h:109-114`)

```
SHS_OFFLINE(0), SHS_ONLINE_REMOTE, SHS_ONLINE_LOCAL, SHS_MAINT
```

### Machine Mode (`DefinesSys.h:84-88`)

```
SYS_MachineMode: SYS_MachineMode_MEAS(0), SYS_MachineMode_SRM
```

### Navigation Pages (`SystemParamShare.h:4-18`)

```
enumNavigateID: LOGIN(0), OPERATE, RESULT_DATA, MAINT, MANUAL, RECIPE, SYSTEM_LOG, CONFIG, ALARM, BUZZER_OFF, PM_MODE
```

### DoWork State (`SystemParamShare.h:20-88`)

Defines the glass processing state machine steps from Idle through Loading, Measure, and Unloading with sub-states for moving, pattern search, align, AF, measure, safety, and probe operations.

---

## 12. Window Messages (Global Communication)

### UI Update Messages (`WindowEventHandler.h:6-33`)

Base: `WM_USER + 100`

| Message | Purpose |
|---|---|
| `WM_MSG_UPDATE_LOGIN_STATE` | Login state changed |
| `WM_MSG_UPDATE_CONNECT_STATE` | Connection state changed |
| `WM_MSG_UPDATE_CHANGE_INITIALIZE` | Initialization state changed |
| `WM_MSG_UPDATE_CHANGE_VIEW_STATE` | View state changed |
| `WM_MSG_UPDATE_RECIPE_INFORMATION` | Recipe info updated |
| `WM_MSG_UPDATE_REMOTE_INTERLOCK` | Remote interlock state changed |
| `WM_MSG_UPDATE_SEQ_ITEM_ELAPSED_TIME` | Sequence item elapsed time |
| `WM_MSG_UPDATE_CYCLE_INSPECTION_START` | Cycle inspection started |
| `WM_MSG_UPDATE_POSITION_STATE` | Position state updated |
| `WM_MSG_UPDATE_LAMP_TIME` | Lamp time updated |
| `WM_MSG_UPDATE_CLOSE_SYSTEM` | System close requested |
| `WM_MSG_UPDATE_CHANGE_CIM_STATE` | CIM state changed |
| `WM_MSG_CCD_LIVE` | CCD live display |
| `WM_MSG_CIM_ALARM` | CIM alarm event |

### Sequence Completion Messages (`WindowEventHandler.h:36-47`)

Base: `WM_USER + 300`

| Message | Purpose |
|---|---|
| `WM_MSG_SEQ_ITEM_COMPLTE` | Sequence item complete |
| `WM_MSG_SEQ_ITEM_INITIALIZE_COMPLTE` | Initialization complete |
| `WM_MSG_SEQ_ITEM_LOADING_COMPLTE` | Glass loading complete |
| `WM_MSG_SEQ_ITEM_UNLOADING_COMPLTE` | Glass unloading complete |
| `WM_MSG_SEQ_ITEM_PPA_COMPLTE` | PPA complete |
| `WM_MSG_SEQ_ITEM_SE_COMPLTE` | SE measurement complete |
| `WM_MSG_SEQ_ITEM_TR_COMPLTE` | TR measurement complete |
| `WM_MSG_SEQ_ITEM_PACC_COMPLTE` | PACC complete |

### Other Messages

| Message | Value | File |
|---|---|---|
| `WM_RECIPE_EVENT` | `WM_USER + 101` | `preDefine.h:5` |
| `WM_ALARM_ACCURED` | `WM_USER + 1300` | `AlarmDefine.h:5` |
| `WM_ALARM_CLEARED` | `WM_USER + 1301` | `AlarmDefine.h:6` |
| `WM_WARNING_ACCURED` | `WM_USER + 1302` | `AlarmDefine.h:7` |
| `WM_WARNING_CLEARED` | `WM_USER + 1303` | `AlarmDefine.h:8` |

---

## 13. Recipe Data Structures (preDefine.h)

Defined in `Source/Include/preDefine.h`. These are not global variables but define the recipe data model used by `VTM_RECIPE` and recipe management:

### S_RecipeMgr (Top-Level Recipe Container)

```
S_RecipeMgr
  +-- m_nVersion: int
  +-- m_nStickCnt: int
  +-- m_nCellCnt: int
  +-- m_sInfo: S_RecipeInfo
  |     +-- m_sInspectItem: S_InspectItem     // 10 inspection flags + timeouts
  |     +-- m_sOptionItem: S_OptionItem        // R-test, sampling, align options
  |     +-- m_sLampItem: S_LampItem            // Front/Back lamp intensity + SRM[4]
  |     +-- m_sContactPos: S_ContactPos        // Ready/Pre/End contact Z + OD offset
  |     +-- m_sMotionOffsetPos: S_MotionOffsetPos  // Stage/Head/Quality offsets
  |     +-- m_sProbeInfo: S_ProbeInfo          // Probe W/H, priority, IDs (5 probes)
  +-- m_sData: S_RecipeData
  |     +-- m_vecStick: vector<S_StickInfo>    // Stick list, each with cells
  |     +-- m_sHeadZ: S_HeadZ                  // Head Z positions for 5 modes
  +-- m_sMap: S_RecipeMap                      // Glass/Cell geometry, CAD data
  +-- m_sAlign: S_Align                        // Alignment patterns and positions
```

### S_RecipeRunState (Recipe Command Interface)

Used for inter-process command passing via `VTM_RECIPE`:

| Member | Type | Purpose |
|---|---|---|
| `m_pData` | `LPVOID` | Generic data pointer |
| `m_dwData1/2/3` | `DWORD` | Command codes (M_CMD_xxx constants) |
| `m_fData[100]` | `double[]` | Position/parameter data |
| `m_nData[10]` | `int[]` | Integer parameters |
| `m_nRunStatus` | `int` | Run status (M_RUN_NONE/REQ/PROC/END/NG) |
| `m_chData1[1024]` | `wchar_t[]` | String data (e.g., pattern path) |

---

## 14. Key System Constants

### From DefinesSys.h

| Constant | Value | Purpose |
|---|---|---|
| `df_USE_PROBE` | `3` | Probes per gantry |
| `df_USE_GANTRY` | `1` | Number of gantries |
| `df_MAX_LAMP` | `16` | Maximum lamp count |
| `df_MAX_MEAS_TYPE` | `20` | Maximum measurement types |
| `df_MAX_MEAS_STEP` | `5000` | Maximum measurement steps |
| `df_MAX_CELL_ID` | `64` | Maximum cell ID count |
| `MAX_STR_SIZE_NORMAL` | `1024` | Normal string buffer size |
| `MAX_STR_SIZE_MAX` | `4096` | Maximum string buffer size |

### From ShareMem.h

| Constant | Value | Purpose |
|---|---|---|
| `MAX_STAGE` | `8` | Maximum stage count |
| `MAX_SLOT` | `25` | Maximum slot count |
| `MAX_PROBE` | `1` | Maximum probes (in shared memory) |
| `MAX_ALIGN_MARK` | `2` | Align mark count |
| `MAX_GLASS_ID` | `64` | Glass ID string size |
| `MAX_RECIPE_ID` | `256` | Recipe ID string size |
| `df_MAX_PB_NUM` | `1000` | Max stick measurement count |

### From preDefine.h

| Constant | Value | Purpose |
|---|---|---|
| `M_RCP_PROBE_MAX` | `5` | Maximum probe count in recipe |
| `M_RCP_INS_ITEM` | `9` | Number of inspection items |
| `CH_LEN` | `1024` | Character buffer length |

---

## 15. Global Data Flow Summary

```
CALPHAApp::theApp (MFC app singleton)
  |
  +-- m_pAuroraMgr (CAuroraMgr - central orchestrator)
        |
        +-- stAuroraMgrPool (28 manager pointers)
        |     |
        |     +-- pShareMemoryMgr --> CShareMemFunction --> VTM_SYSTEM (shared memory)
        |     +-- pSystemConfigMgr (INI config state)
        |     +-- pCommTcpIpFactoryMgr (13 TCP/IP device connections)
        |     +-- ... (25 more managers)
        |
        +-- m_subMgr[] (all SubMgr instances registered here)
        +-- m_subDlg[] (all sub-dialogs registered here)

CSystemParamShare::GetInstance() (global runtime state - paths, mode, glass info)
CSequenceManager::GetInstance() (sequence engine - init, glass work, EQ state)
CWindowEventHandler::GetInstance() (UI message routing)
CAlarmController::GetInstance() (alarm management)
CRecipeMgr::GetInstance() --> VTM_RECIPE (recipe shared memory)
CResultMgr::GetInstance() (result collection)
CSACCMgr::GetInstance() (probe card RFID)
CMCCLogMgr::GetInstance() --> VTM_MCC, VTM_FOR_FDC (logging shared memory)

g_sAppPath (global app path string)
g_csQuery (global critical section)
```

---

## 16. Thread Safety Mechanisms

| Mechanism | Location | Purpose |
|---|---|---|
| `CCriticalSection m_cs_set/m_cs_get` | `CShareMemoryMgr` | Shared memory read/write protection |
| `CCriticalSection m_cs_set/m_cs_get` | `CAlarmController` | Alarm data protection |
| `CCriticalSectionSync` (RAII) | `pch.h:60-78` | Automatic lock/unlock wrapper |
| `g_csQuery` | `AlarmDefine.h:130` | Global query synchronization |
| `CCriticalSection m_cs` | `CShareMemFunction` | Low-level shared memory access protection |
