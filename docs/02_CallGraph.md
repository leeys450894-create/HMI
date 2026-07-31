# 02. ALPHA HMI - Full Call Graph Analysis

## 1. Entry Point to Main Dialog

```
theApp (CALPHAApp global instance)
  |
  +-- CALPHAApp::InitInstance()                          [ALPHA.cpp:47]
       |
       +-- Duplicate process check (CreateToolhelp32Snapshot)
       +-- InitCommonControlsEx(), AfxSocketInit()
       +-- CSystemParamShare::GetInstance()->SetAppFolderPath()
       +-- CreateDirectory() for Config/, DefaultRcp/, Config/CoreFunction/AlarmConfig/
       |
       +-- new CAuroraMgr(...)                           [ALPHA.cpp:171]
       |    |
       |    +-- (See Section 2: Manager Layer Creation)
       |
       +-- GetShareMemoryMgrPtr()->GetSharedMemory()
       +-- GetShareMemoryMgrPtr()->SetSys_MachineMode(df_HOST_OFFLINE)
       +-- GetShareMemoryMgrPtr()->SetSys_MachineType(CUR_EQU_TYPE)
       +-- GetSystemConfigMgrPtr()->GetLanguageIndex()
       |
       +-- CDlgALPHA dlg(m_pAuroraMgr)                  [ALPHA.cpp:237]
       +-- m_pMainWnd = &dlg
       +-- dlg.DoModal()   ------>  CDlgALPHA::OnInitDialog()
                                     |
                                     (See Section 3)
```

## 2. Manager Layer Creation (CAuroraMgr Constructor)

```
CAuroraMgr::CAuroraMgr()                                [AuroraMgr.cpp:55]
  |
  +-- CSystemConfigMgr(strConfigPath)                    -- System configuration INI
  +-- CLogInControlMgr(strConfigPath)                    -- Login/auth
  +-- CAlignCameraMgr(strConfigPath,...) x MAIN_CCD_MAX_NUM  -- Camera managers (4)
  +-- CNetworkConfigMgr(strConfigPath)                   -- Network/TCP config
  +-- CPositionConfigMgr(strConfigPath)                  -- Motor position config
  +-- CRecipeEditMgr(m_AuroraMgrPool)                    -- Recipe editor
  +-- CShareMemoryMgr(strAppPath, strConfigPath)         -- Shared memory (inter-process)
  +-- CLightControlMgr(strAppPath, strConfigPath, strLogPath)  -- Light control
  +-- CIOMapControlMgr(strAppPath, strConfigPath, strLogPath)  -- PLC I/O map
  +-- CMotorControlMgr(strAppPath, strConfigPath, strLogPath)  -- ACS motion control
  +-- CMaintControlMgr(pMotorCtrl, pIOMap, pPosConfig, pLight, pShareMem)
  +-- CSeqTactTimeMgr(strLogPath)                        -- Tact time logging
  +-- CAlarmMonitorMgr(m_AuroraMgrPool)                  -- Alarm monitor threads
  +-- CCommTcpIpFactoryMgr(m_AuroraMgrPool)              -- TCP/IP device factory
  +-- CCommTcpIPMgr(m_AuroraMgrPool)                     -- TCP/IP connection manager
  |
  +-- m_subMgr.push_back(...)  -- Registers 16 sub-managers for lifecycle
  |
  +-- CSequenceManager::GetInstance()->LinkParam(m_AuroraMgrPool)
       |
       +-- new CSystemInitialize(MgrPool)                -- Init sequence
       +-- new CDoWorkGlassSequence(MgrPool)             -- Glass auto-run sequence
       +-- new CFunctionModuleEQStateChange(MgrPool)     -- EQ state transitions
       +-- new CFunctionModuleEQState(MgrPool)           -- EQ state queries
       +-- new CDoWorkSequenceCIM(MgrPool)               -- CIM/MES sequence
       +-- new CFunctionModuleProbeACC(MgrPool)          -- SACC probe auto-change
       |
       +-- m_vecBaseSeqItem stores all above (polymorphic vector)
```

## 3. UI Hierarchy and Dialog Creation

```
CDlgALPHA::OnInitDialog()                               [DlgALPHA.cpp:51]
  |
  +-- MoveWindow(0, 0, 1920, 1050)                      -- Full-screen 1920x1050
  |
  +-- new CDlgTitleBar(m_pAuroraMgr, this)               [DlgALPHA.cpp:79]
  |    +-- Create(IDD_DLG_MAIN_TITLE)
  |    +-- PushSubDlg(enumWndID_TITLE, ...)              -- Registered as subDlg[0]
  |
  +-- new CDlgMainForm(m_pAuroraMgr, this)               [DlgALPHA.cpp:86]
       +-- Create(IDD_DLG_MAIN_FORM)
       +-- PushSubDlg(enumWndID_FRAME, ...)              -- Registered as subDlg[1]
       |
       +-- CDlgMainForm constructor creates:             [DlgMainForm.cpp:112]
       |    +-- CDlgSubOperate     -> m_vecpMainSubDlg[enumNavigateID_OPERATE]
       |    +-- CDlgSubData        -> m_vecpMainSubDlg[enumNavigateID_RESULT_DATA]
       |    +-- CDlgSubMaint       -> m_vecpMainSubDlg[enumNavigateID_MAINT]
       |    +-- CDlgSubManual      -> m_vecpMainSubDlg[enumNavigateID_MANUAL]
       |    +-- CDlgSubRecipe      -> m_vecpMainSubDlg[enumNavigateID_RECIPE]
       |    +-- CDlgSubLog         -> m_vecpMainSubDlg[enumNavigateID_SYSTEM_LOG]
       |    +-- CDlgSubConfig      -> m_vecpMainSubDlg[enumNavigateID_CONFIG]
       |    +-- CDlgSubAlarm       -> m_vecpMainSubDlg[enumNavigateID_ALARM]
       |    +-- CDlgNaviBar        -> m_pMainNavigateDlg
       |    +-- CDlgAlignCCD[4]    -> m_pMainAlignCCD[0..3]
       |    +-- CD_PopUp_Alarm_F   -> m_pPopUp_AlarmDlg
       |
       +-- CDlgMainForm::OnInitDialog()                  [DlgMainForm.cpp:265]
            +-- Creates all sub-dialogs (8 pages + navi + 4 CCD + alarm popup)
            +-- CWindowEventHandler::GetInstance()->SetMessageHandle(enumWndID_FRAME, hwnd)
            +-- BeginThread_WorkProc()                   -- Worker thread
            +-- SetTimer(MAIN_STATE_TIMER_ID, 600ms)     -- UI refresh
            +-- SetTimer(MAIN_STATE_TIMER_ID+1, 5000ms)  -- Alarm monitor start (one-shot)
            +-- SetTimer(MAIN_STATE_TIMER_ID+2, 10ms)    -- IO info refresh
            +-- SetTimer(MAIN_STATE_TIMER_ID+3, 1000ms)  -- PLC alarm popup check
```

## 4. Full Layer Diagram

```
+============================================================================+
|                            APPLICATION LAYER                                |
|  CALPHAApp::InitInstance() -> CDlgALPHA::DoModal()                         |
+============================================================================+
       |
       v
+============================================================================+
|                              UI LAYER                                       |
|  CDlgALPHA (main dialog)                                                   |
|    +-- CDlgTitleBar          (status bar: time, recipe, login, connection)  |
|    +-- CDlgMainForm          (main frame, owns all sub-pages)              |
|         +-- CDlgNaviBar      (navigation buttons)                          |
|         +-- CDlgSubOperate   (operate: init, cycle, start/stop/pause)      |
|         +-- CDlgSubMaint     (maintenance: motor, IO, device test)         |
|         +-- CDlgSubManual    (manual measurement)                          |
|         +-- CDlgSubRecipe    (recipe editor)                               |
|         +-- CDlgSubConfig    (system configuration)                        |
|         +-- CDlgSubAlarm     (alarm display)                               |
|         +-- CDlgSubLog       (system log viewer)                           |
|         +-- CDlgSubData      (result data viewer)                          |
|         +-- CDlgAlignCCD[4]  (CCD camera live views)                       |
|         +-- CD_PopUp_Alarm_F (alarm popup overlay)                         |
+============================================================================+
       |                            |
       v (direct call)              v (Windows message)
+============================================================================+
|                           MANAGER LAYER                                     |
|  CAuroraMgr (orchestrator, holds stAuroraMgrPool)                          |
|    +-- CSystemConfigMgr        CNetworkConfigMgr    CPositionConfigMgr    |
|    +-- CLogInControlMgr        CRecipeEditMgr       CShareMemoryMgr       |
|    +-- CIOMapControlMgr        CMotorControlMgr     CLightControlMgr       |
|    +-- CMaintControlMgr        CSeqTactTimeMgr      CAlarmMonitorMgr       |
|    +-- CCommTcpIpFactoryMgr    CCommTcpIPMgr        CAlignCameraMgr[4]    |
|                                                                             |
|  Singletons:                                                                |
|    CSequenceManager    CWindowEventHandler    CSystemParamShare            |
|    CAlarmController    CSACCMgr    CResultMgr    CMCCLogMgr    CRecipeMgr  |
+============================================================================+
       |
       v
+============================================================================+
|                          SEQUENCE LAYER                                      |
|  CSequenceManager (Singleton)                                               |
|    +-- CSystemInitialize              (22-step system init)                |
|    +-- CDoWorkGlassSequence           (glass auto-run)                     |
|    |    +-- CDoWorkGlassSequenceEngine (9-step engine)                     |
|    |         +-- CDoWorkGlassAdsorption                                    |
|    |         +-- CDoWorkGlassRecipe                                        |
|    |         +-- CDoWorkGlassSWAlign                                       |
|    |         +-- CDoWorkProbeSWAlign                                       |
|    |         +-- CDoWorkGlassPreMeas                                       |
|    |         +-- CDoWorkGlassAGING                                         |
|    |         +-- CDoWorkGlassMeasure                                       |
|    |         +-- CDoWorkGlassResult                                        |
|    |         +-- CDoWorkGlassUnload                                        |
|    +-- CFunctionModuleEQStateChange   (READY<->LOAD<->MEASURE)             |
|    +-- CFunctionModuleEQState         (sensor/interlock queries)           |
|    +-- CFunctionModuleProbeACC        (probe auto-change)                  |
|    +-- CDoWorkSequenceCIM             (CIM/MES protocol sequence)          |
+============================================================================+
       |
       v
+============================================================================+
|                          DEVICE LAYER                                        |
|  Motion:    CMotorControlMgr -> CMotorFunction (ACS SPiiPlus TCP/IP)       |
|  PLC:       CIOMapControlMgr -> CIOFunction (Mitsubishi MC Protocol TCP)   |
|  Light:     CLightControlMgr                                                |
|  CommTcp:   CCommTcpIpFactoryMgr (factory for 13+ device types)           |
|               +-- CCommTcpIpObjectVision[2]   (optical measurement)        |
|               +-- CCommTcpIpObjectColor[2]    (colorimeter)                |
|               +-- CCommTcpIpObjectAngle[2]    (viewing angle)              |
|               +-- CCommTcpIpObjectWAD         (WAD measurement)            |
|               +-- CCommTcpIpObjectPG          (pattern generator)          |
|               +-- CCommTcpIpObjectAgingPG     (aging PG)                   |
|               +-- CCommTcpIpObjectLCR         (LCR meter)                  |
|               +-- CCommTcpIpObjectTSP[6]      (TSP sensors)               |
|               +-- CCommTcpIpObjectCIM         (CIM/MES host)              |
|               +-- CCommTcpIpObjectSysTime     (system time sync)           |
|               +-- CCommTcpIpObjectMotion      (vision motion)              |
|               +-- CCommTcpIpObjectUTG[2]      (UTG measurement)            |
|  Camera:    CAlignCameraMgr[4]                                              |
|  Memory:    CShareMemoryMgr (inter-process shared memory)                  |
+============================================================================+
```

## 5. Navigation Flow (UI Switching)

```
User clicks NaviBar button (e.g., "Operate", "Maint", "Recipe"...)
  |
  CDlgNaviBar::OnBnClickedButtonXxx()                    [DlgNaviBar.cpp]
  |   mechanism: BN_CLICKED (MFC message map)
  |
  +-- m_iCurrentView = enumNavigateID_XXX
  +-- GetParent()->SendMessage(WM_MSG_UPDATE_CHANGE_VIEW_STATE, m_iCurrentView)
       |   mechanism: SendMessage (synchronous Windows message)
       |
       CDlgMainForm::OnUpdateChangeViewState()           [DlgMainForm.cpp:534]
       |   mechanism: ON_MESSAGE handler
       |
       +-- Hide all m_vecpMainSubDlg[]
       +-- Show m_vecpMainSubDlg[index]
       +-- m_pMainNavigateDlg->ChangeCheck(index)        -- Update button states
```

### Navigation Page IDs (from SystemParamShare.h or MainSubDlgWin.h):
```
enumNavigateID_LOGIN        = 0
enumNavigateID_OPERATE      = 1
enumNavigateID_RESULT_DATA  = 2
enumNavigateID_MAINT        = 3
enumNavigateID_MANUAL       = 4
enumNavigateID_RECIPE       = 5
enumNavigateID_SYSTEM_LOG   = 6
enumNavigateID_CONFIG       = 7
enumNavigateID_ALARM        = 8
enumNavigateID_BUZZER_OFF   = 9
enumNavigateID_PM_MODE      = 10
```

## 6. Cycle Start Flow (Start Button -> Sequence Engine)

```
User clicks [Cycle Start] button on CDlgSubOperate
  |
  CDlgSubOperate::OnBnClickedButtonCycle()               [DlgSubOperate.cpp:~1574]
  |   mechanism: BN_CLICKED
  |
  +-- Safety checks: alarm count, cycle state, process status, thread run
  +-- CD_MeasureCycle dlg.DoModal()                      -- Popup: cycle configuration
  |
  +-- CSystemParamShare::GetInstance()->SetCycleState(true)
  +-- CWindowEventHandler::GetInstance()->SendMessageToUI(
  |       enumWndID_FRAME, WM_MSG_UPDATE_CYCLE_INSPECTION_START)
  |   |   mechanism: SendMessageTimeout -> CDlgMainForm handler
  |   |
  |   CDlgMainForm::OnUpdateCycleInspectionStart()       [DlgMainForm.cpp:624]
  |     +-- Iterates all sub-dialogs -> UpdateCycleInspectionStart()
  |     +-- m_pMainNavigateDlg->UpdateCycleInspectionStart()
  |
  +-- CSequenceManager::GetInstance()->EngineStart()
       |   mechanism: direct call (Singleton)
       |
       CSequenceManager::EngineStart()                   [SequenceManager.cpp:202]
         +-- m_pDoWorkGlassSequence->EngineStart()
              |
              CDoWorkGlassSequence::EngineStart()
                +-- m_pSeqEngine->EngineStart()
                     |
                     CDoWorkGlassSequenceEngine::EngineStart()
                       +-- SetEvent(m_hStartEvent)       -- Signals engine thread
                            |
                            Engine thread wakes up (ThreadRun)
                            +-- Runs 9-step glass sequence in loop
```

## 7. System Initialize Flow

```
User clicks [Initialize] button on CDlgSubOperate
  |
  CDlgSubOperate::OnBnClickedButtonInitial()             [DlgSubOperate.cpp:~1246]
  |   mechanism: BN_CLICKED
  |
  +-- Safety checks (MC-ON, etc.)
  +-- CSequenceManager::GetInstance()->EngineCycleInit()
  +-- CWindowEventHandler::SendMessageToUI(enumWndID_FRAME,
  |       WM_MSG_UPDATE_CHANGE_NOT_INITIALIZE)
  |   |
  |   CDlgMainForm::OnUpdateChangeNotInitialize()        [DlgMainForm.cpp:490]
  |     +-- Disables non-essential sub-pages
  |
  +-- m_pSystemInitializingDlg->InitStart()              -- Shows progress dialog
  |     +-- SetTimer(MAIN_OPERATE_SUB_SYSTEM_INITIALIZE_TIMER_ID, 500ms)
  |         -- Timer polls CSystemInitialize completion
  |
  +-- MoveCommandProcess("SEQUENCE_ITEM_INITIALIZE", false)
  +-- CSequenceManager::GetInstance()->GetSystemInitialize()->Start()
       |
       CBaseSequenceImpl::Start()                        [BaseSequenceImpl.cpp:110]
         +-- GoToInitStep()                              -- Reset step index to 0
         +-- ResetEvent(m_hCompleteEvent)
         +-- SetEvent(m_hStartEvent)                     -- Wake up thread
              |
              CBaseSequenceImpl thread loop wakes
              +-- Calls CSystemInitialize::ThreadRun()   -- 22-step init
                   (motor home, IO init, device connect, etc.)
```

## 8. Async Call Patterns

### 8.1 Thread-based Async (AfxBeginThread)

All sequence items inherit from `CBaseSequenceImpl`, which spawns a thread on
construction. The thread sleeps on `WaitForMultipleObjects` until signaled:

```
CBaseSequenceImpl constructor                            [BaseSequenceImpl.cpp:11]
  +-- m_pThread = AfxBeginThread(ThreadRun, this)
  +-- Thread loop: WaitForMultipleObjects(hStartEvent, hCloseEvent)
       |
       WAIT_OBJECT_0 (Start):  pThis->ThreadRun()        -- Polymorphic dispatch
       WAIT_OBJECT_0+1 (Close): break                    -- Thread exit
```

**Thread inventory (from AfxBeginThread grep):**

| Source File | Thread Purpose |
|-------------|---------------|
| BaseSequenceImpl.cpp | Sequence item threads (6 instances via SequenceManager) |
| DoWorkGlassSequenceEngine.cpp | Glass sequence engine (9-step cycle) |
| MeasureSequenceItemEngine.cpp | Individual measurement step engine |
| MeasureSequenceItemImpl.cpp | Measurement step implementation |
| DlgMainForm.cpp | Worker thread (WorkThreadProbeProc) -- monitor/probe |
| DlgSubMaint.cpp | Maintenance worker thread |
| DlgSubMaint_MotionFile.cpp | Motion file operation thread |
| DlgSubManual_Meas.cpp | Manual measurement thread |
| DlgSubOperate.cpp | ThreadInspectEnd -- stop inspection cleanup |
| DlgSubOperate.cpp | ThreadRun -- operate page async work |
| DlgAlignCCD.cpp | CCD grab worker thread |
| CD_PopUp_Motion_Move.cpp | Motion move popup thread |
| CommTcpIpObjectCIM.cpp | CIM communication thread |
| CommTcpIpObjectVision.cpp | Vision measurement thread + QualityWaitThread |
| CommTcpIpObjectColor.cpp | Color measurement thread |
| CommTcpIpObjectTSP.cpp | TSP sensor thread |
| RecipeEditMgr.cpp | Recipe editing thread |
| AlarmMonitorToEmergencyButton.cpp | Emergency button monitor |
| AlarmMonitorToDoorOpenState.cpp | Door open state monitor |
| AlarmMonitorToDoorVoice.cpp | Door voice alarm monitor |
| AlarmMonitorToLampTime.cpp | Lamp time monitor |
| AlarmMonitorToMcOFFState.cpp | MC-OFF state monitor |
| AlarmMonitorToSafetyPLC.cpp | Safety PLC monitor |
| AlarmMonitorToSensorCurtain.cpp | Light curtain sensor monitor |
| AlarmMonitorToSensorFAN.cpp | Fan sensor monitor |
| AlarmMonitorToSensorHandSwitch.cpp | Hand switch monitor |
| AlarmMonitorToSensorIsolater.cpp | Isolator sensor monitor |
| AlarmMonitorToSensorPressure.cpp | Pressure sensor monitor |
| AlarmMonitorToSensorSmoke.cpp | Smoke sensor monitor |
| AlarmMonitorToSensorTemperature.cpp | Temperature sensor monitor (high) |
| AlarmMonitorToSensorTemperatureLow.cpp | Temperature sensor monitor (low) |
| AlarmMonitorToSensorVacSwitch.cpp | Vacuum switch monitor |
| AlarmMonitorToTeachKEY.cpp | Teach key monitor |
| AlarmMonitorToTeachKEYLock.cpp | Teach key lock monitor |

**Total: ~35+ threads**

### 8.2 Windows Message-based Communication

#### CAuroraMgr::Notify / Notify_Direct Pattern
```
CAuroraMgr::Notify(nID, msgID, wp, lp)                  [AuroraMgr.cpp:321]
  +-- m_subDlg[nID]->PostMessage(msgID, wp, lp)          -- Async (PostMessage)

CAuroraMgr::Notify_Direct(nID, msgID, wp, lp)            [AuroraMgr.cpp:329]
  +-- m_subDlg[nID]->SendMessage(msgID, wp, lp)          -- Sync (SendMessage)
```

Registered dialogs:
- `m_subDlg[enumWndID_TITLE]` = CDlgTitleBar
- `m_subDlg[enumWndID_FRAME]` = CDlgMainForm

Usage: `CDlgALPHA::OnClose()` broadcasts WM_MSG_UPDATE_CLOSE_SYSTEM to all subDlgs.

#### CWindowEventHandler Singleton Pattern
```
CWindowEventHandler::GetInstance()                        [WindowEventHandler.cpp]
  |
  +-- SendMessageToUI(enumWndID, msg, wp, lp)
  |     Uses SendMessageTimeout(1000ms, SMTO_BLOCK|SMTO_NOTIMEOUTIFNOTHUNG)
  |
  +-- PostMessageToUI(enumWndID, msg, wp, lp)
       Uses ::PostMessage() (async, non-blocking)
```

Registered HWNDs:
- `enumWndID_TITLE` -> CDlgTitleBar (set by CDlgTitleBar::OnInitDialog)
- `enumWndID_FRAME` -> CDlgMainForm (set by CDlgMainForm::OnInitDialog)

**Key message flows via CWindowEventHandler:**

| Sender | Message | Receiver | Purpose |
|--------|---------|----------|---------|
| DlgSubOperate | WM_MSG_UPDATE_CHANGE_NOT_INITIALIZE | CDlgMainForm | Pre-init UI lock |
| DlgSubOperate | WM_MSG_UPDATE_CYCLE_INSPECTION_START | CDlgMainForm, CDlgTitleBar | Cycle start UI update |
| CommTcpIpObjectCIM | WM_MSG_UPDATE_CHANGE_CONTROLMODE | CDlgMainForm | CIM control mode change |
| CommTcpIpObjectCIM | WM_MSG_UPDATE_CHANGE_CIM_STATE | CDlgMainForm | CIM state transitions |
| AlarmMonitorToEmergencyButton | WM_MSG_UPDATE_POSITION_STATE | CDlgMainForm | EMO position update |
| DlgSubConfig | WM_MSG_WRITE_LAMP_TIME | CDlgMainForm | Lamp time config |
| DlgSubConfig | WM_MSG_UPDATE_COLOR | CDlgMainForm | Measurement color config |
| DlgMainForm | WM_MSG_UPDATE_RECIPE_INFORMATION | CDlgTitleBar | Recipe info refresh |
| DlgSubManual_Meas | WM_MSG_UPDATE_POSITION_STATE | CDlgMainForm | Aging start/end |

#### Child-to-Parent SendMessage Pattern (CDlgNaviBar)
```
CDlgNaviBar::OnBnClickedButtonXxx()
  +-- GetParent()->SendMessage(WM_MSG_UPDATE_CHANGE_VIEW_STATE, viewID)
       |
       CDlgMainForm::OnUpdateChangeViewState()  (parent handles it)
```

Same pattern used by:
- CDlgSubAlarm -> parent (CDlgMainForm) for view switching
- CDlgSubRecipe -> parent (CDlgMainForm) for recipe info update

### 8.3 Custom Message Definitions

```
WindowEventHandler.h -- eMessage enum (WM_USER + 100 range):
  WM_MSG_UPDATE_LOGIN_STATE            WM_USER+100
  WM_MSG_UPDATE_LOGIN_INFO             WM_USER+101
  WM_MSG_UPDATE_CONNECT_STATE          WM_USER+102
  WM_MSG_UPDATE_AUTO_CHANGE_INLINE_MODE WM_USER+103
  WM_MSG_UPDATE_CHANGE_NOT_INITIALIZE  WM_USER+104
  WM_MSG_UPDATE_CHANGE_INITIALIZE      WM_USER+105
  WM_MSG_UPDATE_CHANGE_VIEW_STATE      WM_USER+106
  WM_MSG_UPDATE_RECIPE_INFORMATION     WM_USER+107
  WM_MSG_UPDATE_REMOTE_INTERLOCK       WM_USER+108
  WM_MSG_UPDATE_SEQ_ITEM_ELAPSED_TIME  WM_USER+109
  WM_MSG_UPDATE_CYCLE_INSPECTION_START WM_USER+110
  WM_MSG_UPDATE_CYCLE_INSPECTION_RESULT WM_USER+111
  WM_MSG_UPDATE_CYCLE_INSPECTION_COMPLETE WM_USER+112
  WM_MSG_UPDATE_POSITION_STATE         WM_USER+113
  WM_MSG_UPDATE_STATE_INITIALIZE       WM_USER+114
  WM_MSG_UPDATE_LAMP_TIME              WM_USER+115
  WM_MSG_WRITE_LAMP_TIME               WM_USER+116
  WM_MSG_UPDATE_COLOR                  WM_USER+117
  WM_MSG_UPDATE_CLOSE_SYSTEM           WM_USER+118
  WM_MSG_UPDATE_MEASURS_ELAPSED_TIME   WM_USER+119
  WM_MSG_UPDATE_CHANGE_CIM_STATE       WM_USER+120
  WM_MSG_UPDATE_CHANGE_CONTROLMODE     WM_USER+121
  WM_MSG_CCD_LIVE                      WM_USER+122
  WM_MSG_CIM_ALARM                     WM_USER+123

WindowEventHandler.h -- enumSeqItemMessage (WM_USER + 300 range):
  WM_MSG_SEQ_ITEM_COMPLTE              WM_USER+300
  WM_MSG_SEQ_ITEM_INITIALIZE_COMPLTE   WM_USER+301
  WM_MSG_SEQ_ITEM_LOADING_COMPLTE      WM_USER+302
  ... (through WM_MSG_SEQ_ITEM_PACC_COMPLTE)
```

### 8.4 Timer Callbacks

| Dialog | Timer ID | Interval | Purpose |
|--------|----------|----------|---------|
| CDlgMainForm | MAIN_STATE_TIMER_ID (1001) | 600ms | UpdateInformation() -- refresh motion/IO grid |
| CDlgMainForm | 1002 | 5000ms (one-shot) | AlarmMonitorMgr->SetMonitoringStart() |
| CDlgMainForm | 1003 | 10ms | __RefreshListData2() -- IO info refresh |
| CDlgMainForm | 1004 | 1000ms | PLC alarm popup check |
| CDlgSubOperate | MAIN_STATE_TIMER_ID (1001) | 1000ms | UpdateCellStatus() |
| CDlgTitleBar | TIMERID_PROGRAM_TIME (100) | 500ms | UpdateTime() |
| CDlgTitleBar | TIMERID_IO_STATE_TIME (200) | 1000ms | SetIOStatus() |
| CDlgTitleBar | TIMERID_CONNECT_STATE (300) | 2500ms | SetConnectStatus() |
| D_MainOperateInitializing | 5001 | 500ms | Poll system initialize completion |
| D_MainOperateInitializing | 5011 | 500ms | Poll SACC completion |
| D_MainOperateInitializing | 5012 | 1000ms | Aging timer |
| D_MoveHomePosition | -- | 1000ms/15000ms | Home position status/timeout |
| CD_PopUp_Alarm_F | 1000 | 1000ms | Alarm popup refresh |
| CD_PopUp_Motion_Move | 1000 | 1000ms | Motion move status |
| DlgSubMaint_IOFile | 1 | 1000ms | IO file monitoring |
| D_PM_MODE | -- | 1000ms/500ms | PM mode monitoring |

### 8.5 Singleton Access Pattern

The following singletons provide global access points:

```
CSequenceManager::GetInstance()       -- Sequence dispatch (created on first call)
CWindowEventHandler::GetInstance()    -- UI message routing
CSystemParamShare::GetInstance()      -- System-wide parameters
CAlarmController::GetInstance()       -- Alarm set/clear
CSACCMgr::GetInstance()               -- SACC control
CResultMgr::GetInstance()             -- Result data management
CMCCLogMgr::GetInstance()             -- MCC log actions
CRecipeMgr::GetInstance()             -- Recipe management
CLogWrapper::GetInstance()            -- System logging
```

All are destroyed in `CAuroraMgr::~CAuroraMgr()` via `DelInstance()`.

### 8.6 Event-based Thread Synchronization

```
CBaseSequenceBody (base class):
  +-- m_hEvtUserPause     -- Set by UI to pause sequence
  +-- m_hEvtUserResume    -- Set by UI to resume sequence
  +-- IfPauseEvt_WaitResumeEvt()  -- Blocking wait if paused

CBaseSequenceImpl (derived):
  +-- m_hStartEvent       -- Signal to start ThreadRun()
  +-- m_hCloseEvent       -- Signal to exit thread
  +-- m_hWaitTimeEvent    -- Wait with timeout
  +-- m_hCompleteEvent    -- Sequence completed

CDoWorkGlassSequenceEngine (engine):
  +-- m_hStartEvent       -- Start engine cycle
  +-- m_hExitsEvent       -- Exit after current step
  +-- m_hCloseEvent       -- Kill engine thread
  +-- m_hWaitsEvent       -- Wait between cycles
  +-- m_hBreakEvent_link  -- Link to item break event
```

## 9. CDlgMainForm Message Map (Central Router)

CDlgMainForm serves as the central message router, handling all WM_MSG_* messages
and dispatching to the appropriate sub-dialogs:

```
CDlgMainForm::BEGIN_MESSAGE_MAP                          [DlgMainForm.cpp:227]

  WM_MSG_UPDATE_CHANGE_NOT_INITIALIZE   -> OnUpdateChangeNotInitialize
    -> Disables sub-pages (except Operate, Alarm)

  WM_MSG_UPDATE_CHANGE_INITIALIZE       -> OnUpdateChangeInitialize
    -> Re-enables all sub-pages, shows Operate page

  WM_MSG_UPDATE_CHANGE_VIEW_STATE       -> OnUpdateChangeViewState
    -> Hides all, shows selected sub-page, updates NaviBar

  WM_MSG_UPDATE_RECIPE_INFORMATION      -> OnUpdateDataBaseInformation
    -> CSequenceManager::UpdateDataBaseInformation()
    -> Iterates sub-dialogs -> UpdateRecipeInformation()
    -> Forwards to TitleBar via CWindowEventHandler

  WM_MSG_UPDATE_SEQ_ITEM_ELAPSED_TIME   -> OnUpdateSeqItemElapsedTime
    -> Iterates sub-dialogs -> UpdateSeqItemElapsedTime()

  WM_MSG_UPDATE_CYCLE_INSPECTION_START  -> OnUpdateCycleInspectionStart
    -> Iterates sub-dialogs -> UpdateCycleInspectionStart()
    -> m_pMainNavigateDlg->UpdateCycleInspectionStart()

  WM_MSG_UPDATE_CYCLE_INSPECTION_COMPLETE -> OnUpdateCycleInspectionComplete
    -> Iterates sub-dialogs -> UpdateCycleInspectionComplete()

  WM_MSG_UPDATE_POSITION_STATE          -> OnUpdateMotorPosition
    -> Dispatches to sub-dialogs for aging/SACC status

  WM_MSG_UPDATE_CHANGE_CIM_STATE       -> OnUpdateCIMState
    -> Iterates sub-dialogs -> UpdateCIMState()

  WM_MSG_UPDATE_CHANGE_CONTROLMODE     -> OnUpdateControlMode
    -> Iterates sub-dialogs -> UpdateControlMode()

  WM_MSG_UPDATE_CLOSE_SYSTEM           -> OnCloseSysrem
    -> Finalize() cleanup

  WM_MSG_CCD_LIVE                       -> OnCcdLiveMode
    -> CCD live mode toggle
```

## 10. CIM/MES Communication Flow

```
CCommTcpIpObjectCIM (TCP/IP thread)                      [CommTcpIpObjectCIM.cpp]
  |
  +-- m_pThread = AfxBeginThread(ThreadRun, this)         -- Dedicated comm thread
  |
  +-- Receives commands from MES host
  |     (LDREQ, LDCPL, ULDREQ, ULDCPL, PRCS, GDTREQ, PPID, etc.)
  |
  +-- Notifies UI via CWindowEventHandler:
  |     SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CHANGE_CIM_STATE, cmd)
  |     SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CHANGE_CONTROLMODE, mode)
  |
  +-- CDlgMainForm receives message -> dispatches to sub-dialogs
       -> CDlgSubOperate::UpdateCIMState() handles CIM state changes
```

## 11. Alarm Monitor Architecture

```
CAlarmMonitorMgr (manager, CAuroraSubMgr)                [AlarmMonitorMgr.h]
  |
  +-- m_vecMonitor: vector<CAlarmMonitorImpl*>
  |    |
  |    +-- AlarmMonitorToEmergencyButton     -- E-stop button
  |    +-- AlarmMonitorToDoorOpenState        -- Safety door
  |    +-- AlarmMonitorToDoorVoice            -- Door voice alarm
  |    +-- AlarmMonitorToLampTime             -- Lamp timeout
  |    +-- AlarmMonitorToMcOFFState           -- Motor controller off
  |    +-- AlarmMonitorToSafetyPLC            -- Safety PLC
  |    +-- AlarmMonitorToSensorCurtain        -- Light curtain
  |    +-- AlarmMonitorToSensorFAN            -- Cooling fan
  |    +-- AlarmMonitorToSensorHandSwitch     -- Hand switch
  |    +-- AlarmMonitorToSensorIsolater       -- Isolator
  |    +-- AlarmMonitorToSensorPressure       -- Pressure sensor
  |    +-- AlarmMonitorToSensorSmoke          -- Smoke detector
  |    +-- AlarmMonitorToSensorTemperature    -- Over-temperature
  |    +-- AlarmMonitorToSensorTemperatureLow -- Under-temperature
  |    +-- AlarmMonitorToSensorVacSwitch      -- Vacuum switch
  |    +-- AlarmMonitorToTeachKEY             -- Teach pendant key
  |    +-- AlarmMonitorToTeachKEYLock         -- Teach key lock
  |
  +-- SetMonitoringStart()
  |     Calls each monitor's SetStart()
  |     -> Each spawns AfxBeginThread(ThreadRun, this)
  |     -> Thread polls IOMapControlMgr for PLC bit state
  |     -> On alarm: CAlarmController::GetInstance()->SetAlarm(code)
  |
  +-- SetMonitoringStop()
       Calls each monitor's SetStop()
```

## 12. Sequence Inheritance Hierarchy

```
CBaseSequenceBody                                         [BaseSequenceBody.h]
  |  - Step machine (GoToInitStep/NextStep/ExitStep)
  |  - MgrPool access (all manager pointers)
  |  - Pause/Resume events
  |
  +-- CBaseSequenceImpl                                   [BaseSequenceImpl.h]
       |  - Thread lifecycle (AfxBeginThread, event wait)
       |  - Start/Stop/Pause/Resume/Skip/Exit interface
       |
       +-- CSystemInitialize                              [DoWork/SystemInitialize.h]
       +-- CDoWorkGlassSequence                           [DoWork/DoWorkGlassSequence.h]
       +-- CFunctionModuleEQStateChange                   [FunctionModule/]
       +-- CFunctionModuleEQState                         [FunctionModule/]
       +-- CFunctionModuleProbeACC                        [FunctionModule/]
       +-- CDoWorkSequenceCIM                             [DoWork/DoWorkSequenceCIM.h]

CDoWorkGlassSequenceImpl (separate hierarchy for engine items):
  +-- CDoWorkGlassAdsorption
  +-- CDoWorkGlassRecipe
  +-- CDoWorkGlassSWAlign
  +-- CDoWorkProbeSWAlign
  +-- CDoWorkGlassPreMeas
  +-- CDoWorkGlassAGING
  +-- CDoWorkGlassMeasure
  +-- CDoWorkGlassResult
  +-- CDoWorkGlassUnload
```

## 13. Key Cross-Layer Call Chains

### 13.1 Auto-Run: Start to First Measurement
```
[UI] CDlgSubOperate::OnBnClickedButtonCycle()
  -> [Singleton] CSequenceManager::GetInstance()->EngineStart()
  -> [Sequence] CDoWorkGlassSequence::EngineStart()
  -> [Engine] CDoWorkGlassSequenceEngine::EngineStart()
  -> [Thread] SetEvent(m_hStartEvent) -- wakes engine thread
  -> [Engine Thread] ThreadRun() iterates 9 steps:
       Step 0: CDoWorkGlassAdsorption
         -> [Device] CMaintControlMgr::DoWork_StageVac_OnNOff()
         -> [PLC] CIOMapControlMgr::SetMemoryBitToDevice()
       Step 1: CDoWorkGlassRecipe
         -> [Device] CCommTcpIpFactoryMgr::DoWorkRecipeOpen()
       Step 2: CDoWorkGlassSWAlign
         -> [Device] CMaintControlMgr::MoveCmdAlignPos_Glass()
         -> [Motion] CMotorControlMgr::MoveCmdPosA()
       ...
       Step 6: CDoWorkGlassMeasure
         -> [Device] CCommTcpIpFactoryMgr::DoWorkInspectStart()
         -> [TCP/IP] CCommTcpIpObjectVision::InspectStart()
       Step 7: CDoWorkGlassResult
         -> [Singleton] CResultMgr::GetInstance()->...
       Step 8: CDoWorkGlassUnload
         -> [PLC] CIOMapControlMgr::SetMemoryBitToDevice()
```

### 13.2 CIM Host Command to Equipment Action
```
[MES Host] TCP packet received
  -> [Thread] CCommTcpIpObjectCIM::ThreadRun() parses command
  -> [Message] CWindowEventHandler::SendMessageToUI(
       enumWndID_FRAME, WM_MSG_UPDATE_CHANGE_CIM_STATE, cmd)
  -> [UI] CDlgMainForm::OnUpdateCIMState()
  -> [UI] CDlgSubOperate::UpdateCIMState(cmd)
  -> [Sequence] CDoWorkSequenceCIM handles CIM protocol steps
  -> [Device] Various manager calls (recipe open, cycle start, etc.)
```

### 13.3 Alarm Detection to UI Popup
```
[Thread] AlarmMonitorToXxx::ThreadRun() polling loop
  -> [PLC] CIOMapControlMgr::GetParam_DIOMapMemoryBitImport(address)
  -> [Alarm] CAlarmController::GetInstance()->SetAlarm(code)
  -> [Timer] CDlgMainForm::OnTimer(1004) every 1000ms
       -> CIOMapControlMgr::GetParam_DIOMapMemoryBitImport(PLC_BIT_STATE_ALARM)
       -> m_pPopUp_AlarmDlg->ShowWindow(SW_SHOW)
```

## 14. Shutdown Flow

```
User clicks close button (X) or menu close
  |
  CDlgALPHA::OnClose()                                   [DlgALPHA.cpp:158]
  |
  +-- AfxMessageBox("Would you like to exit?")
  +-- m_pAuroraMgr->GetAlarmMonitorMgrPtr()->SetMonitoringStop()
  +-- Sleep(1500)  -- Wait for alarm threads to stop
  |
  +-- for each subDlg:
  |     Notify_Direct(i, WM_MSG_UPDATE_CLOSE_SYSTEM)     -- Sync cleanup
  |     ShowWindow(SW_HIDE)
  |
  +-- CAlarmController::GetInstance()->ClearAlarm()
  |
  +-- m_pAuroraMgr->Finalize()
  |     +-- GetMaintControlMgr()->GetIOMapControlMgrPtr()->DisConnect()
  |     +-- GetCommTcpIpFactoryMgrPtr()->DisConnect()
  |
  +-- CAlarmController::GetInstance()->DelInstance()
  |
  +-- delete m_pAuroraMgr
       |
       CAuroraMgr::~CAuroraMgr()
         +-- Delete all subDlg windows
         +-- For each subMgr: Finalize() + delete
         +-- DelInstance() for all singletons:
              CWindowEventHandler, CSystemParamShare,
              CSequenceManager, CLogWrapper,
              CSACCMgr, CResultMgr, CMCCLogMgr, CRecipeMgr
```
