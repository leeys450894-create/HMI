# 09. Thread Analysis - ALPHA HMI

## 1. Overview

The ALPHA HMI application uses approximately **50+ threads** at runtime. All threads are created using `AfxBeginThread()` (MFC worker threads). No `CreateThread` or `_beginthreadex` calls are used in the application code.

Thread categories:
- **Sequence Threads** (CBaseSequenceImpl-derived): ~15+ threads for automated sequence steps
- **Alarm Monitor Threads**: ~24 threads monitoring safety sensors
- **Communication Threads**: ~13+ threads for TCP/IP and serial device communication
- **UI Worker Threads**: ~5 threads for background UI operations
- **Core Library Threads**: PLC scanner, timer, result monitor threads

---

## 2. Thread Architecture Patterns

### 2.1 Pattern A: Event-Driven Worker Thread (Primary Pattern)

Used by `CBaseSequenceImpl`, `CDoWorkGlassSequenceEngine`, `CMeasureSequenceItemEngine`, `CMeasureSequenceItemImpl`.

```
Constructor:
  m_pThread = AfxBeginThread(ThreadRun, this);
  m_hStartEvent = CreateEvent(NULL, FALSE, FALSE, NULL);  // auto-reset
  m_hCloseEvent = CreateEvent(NULL, FALSE, FALSE, NULL);  // auto-reset
  m_hMultiEvent[0] = m_hStartEvent;
  m_hMultiEvent[1] = m_hCloseEvent;

Static ThreadRun(LPVOID):
  while(1) {
    dw = WaitForMultipleObjects(2, m_hMultiEvent, FALSE, INFINITE);
    if (dw == WAIT_OBJECT_0)       // Start event -> run work
      pThis->ThreadRun();
    else if (dw == WAIT_OBJECT_0+1) // Close event -> exit
      break;
  }

Destructor:
  ResetEvent(m_hStartEvent);
  SetEvent(m_hCloseEvent);
  WaitForSingleObject(m_pThread->m_hThread, INFINITE);
```

- **File:** `Source/ALPHA/BaseSequenceImpl.cpp` (lines 11-89)
- **Key:** Thread is created at construction, waits for StartEvent to do work, CloseEvent to exit.

### 2.2 Pattern B: Polled Monitor Thread

Used by alarm monitors (`AlarmMonitorToEmergencyButton`, `AlarmMonitorToTeachKEY`, etc.).

```
Static ThreadRun(LPVOID):
  while(1) {
    if (WaitForSingleObject(m_hStartEvent, 10) == WAIT_OBJECT_0) {
      Sleep(200);
      pThis->ThreadRun();   // Do monitoring work
    }
    if (WaitForSingleObject(m_hCloseEvent, 10) == WAIT_OBJECT_0) {
      break;
    }
    Sleep(1);
  }
```

- **File:** `Source/ALPHA/Alarm/AlarmMonitorToEmergencyButton.cpp` (lines 65-95)
- **Key:** Manual-reset StartEvent stays set during monitoring, thread polls every ~10ms.

### 2.3 Pattern C: Connection Monitor Thread

Used by `CCommEthFunction`.

```
Static ThreadCheckConnect(LPVOID):
  while(1) {
    if (WaitForSingleObject(m_hCloseEvent, 1000) == WAIT_OBJECT_0) {
      pThis->NetDelete();
      break;
    } else {
      // Reconnect if disconnected
      if (pThis->IsConnected() == false)
        pThis->NetStart();
    }
  }
```

- **File:** `CoreFunction/CommEthFunction/CommEthFunction.cpp` (lines 490-534)
- **Key:** Checks connection every 1 second, auto-reconnects.

### 2.4 CCriticalSectionSync RAII Wrapper

Defined in `Source/ALPHA/pch.h` (lines 60-78):
```cpp
class CCriticalSectionSync {
  CCriticalSection* m_pSyncObject;
public:
  CCriticalSectionSync(CCriticalSection* p, BOOL bInitialLock = FALSE) {
    m_pSyncObject = p;
    if (bInitialLock) m_pSyncObject->Lock();
  }
  ~CCriticalSectionSync() { m_pSyncObject->Unlock(); }
};
```

Usage: `CCriticalSectionSync single(&m_cs, TRUE);` -- locks on construction, unlocks on scope exit.

---

## 3. Sequence Threads (CBaseSequenceImpl-Derived)

All inherit from `CBaseSequenceImpl` which creates a thread in its constructor (`BaseSequenceImpl.cpp:21`).

Each `CBaseSequenceImpl` subclass = 1 thread.

### 3.1 CSequenceManager Managed Threads

Created in `Source/ALPHA/Manager/SequenceManager.cpp` (lines 146-163):

| # | Class | Purpose | Thread Pattern |
|---|-------|---------|----------------|
| 1 | `CSystemInitialize` | System initialization (22 steps) | Pattern A |
| 2 | `CDoWorkGlassSequence` | Glass measurement main sequence | Pattern A |
| 3 | `CFunctionModuleEQStateChange` | Equipment state transition (READY/LOAD/MEASURE) | Pattern A |
| 4 | `CFunctionModuleEQState` | Equipment state monitoring | Pattern A |
| 5 | `CDoWorkSequenceCIM` | CIM communication sequence | Pattern A |
| 6 | `CFunctionModuleProbeACC` | SACC probe accumulation | Pattern A |

### 3.2 CDoWorkGlassSequenceEngine Thread

- **File:** `Source/ALPHA/DoWork/DoWorkGlassSequenceEngine.cpp` (lines 56-57)
- **Purpose:** Orchestrates the 9-step glass sequence pipeline
- **Thread function:** `ThreadRun()` -- iterates through sequence items (Adsorption, Recipe, GlassAlign, ProbeAlign, PreMeas, Aging, Measure, Result, Unload)
- **Events:** m_hStartEvent, m_hExitsEvent, m_hCloseEvent (3-event WaitForMultipleObjects)

### 3.3 DoWork Glass Sequence Item Threads (9 threads)

Each created as `CDoWorkGlassSequenceImpl` (extends `CBaseSequenceImpl`) inside `DoWorkGlassSequenceEngine` constructor (lines 43-51):

| # | Class | File | Purpose |
|---|-------|------|---------|
| 1 | `CDoWorkGlassAdsorption` | `DoWork/DoWorkGlassAdsorption.cpp` | Glass adsorption/loading |
| 2 | `CDoWorkGlassRecipe` | `DoWork/DoWorkGlassRecipe.cpp` | Recipe setup |
| 3 | `CDoWorkGlassSWAlign` | `DoWork/DoWorkGlassSWAlign.cpp` | Glass SW alignment |
| 4 | `CDoWorkProbeSWAlign` | `DoWork/DoWorkProbeSWAlign.cpp` | Probe SW alignment |
| 5 | `CDoWorkGlassPreMeas` | `DoWork/DoWorkGlassPreMeas.cpp` | Pre-measurement |
| 6 | `CDoWorkGlassAGING` | `DoWork/DoWorkGlassAGING.cpp` | Aging inspection |
| 7 | `CDoWorkGlassMeasure` | `DoWork/DoWorkGlassMeasure.cpp` | Main measurement |
| 8 | `CDoWorkGlassResult` | `DoWork/DoWorkGlassResult.cpp` | Result processing |
| 9 | `CDoWorkGlassUnload` | `DoWork/DoWorkGlassUnload.cpp` | Glass unloading |

### 3.4 CMeasureSequenceItemEngine Thread

- **File:** `Source/ALPHA/DoWork/MeasureSequenceItemEngine.cpp` (lines 40)
- **Purpose:** Orchestrates measurement sub-items (per-device measurement steps)
- **Thread function:** Same 3-event pattern as DoWorkGlassSequenceEngine
- **Note:** Created inside `CDoWorkGlassAGING` and `CDoWorkGlassMeasure`

### 3.5 CMeasureSequenceItemImpl Threads (Variable Count)

- **File:** `Source/ALPHA/DoWork/MeasureSequenceItemImpl.cpp` (line 25)
- **Base class:** `CBaseSequenceBody` (not `CBaseSequenceImpl`)
- **Purpose:** Individual measurement device threads (Vision, Color, Angle, etc.)
- **Pattern:** Same WaitForMultipleObjects(2) pattern as CBaseSequenceImpl
- **Count:** Depends on configured measurement devices at runtime

---

## 4. Alarm Monitor Threads

### 4.1 AlarmMonitorMgr Structure

`CAlarmMonitorMgr` (file: `Source/ALPHA/Alarm/AlarmMonitorMgr.cpp`) creates 5 top-level monitor groups:

| # | Class | Purpose |
|---|-------|---------|
| 1 | `CAlarmMonitorToEmergency` | Emergency stop buttons |
| 2 | `CAlarmMonitorToMcOFF` | MC contactor off detection |
| 3 | `CAlarmMonitorToSensor` | Environmental sensors |
| 4 | `CAlarmMonitorToTeachKEY` | Teach key position monitoring |
| 5 | `CAlarmMonitorToDoorOpen` | Door open detection |

### 4.2 Emergency Monitor Threads

- **File:** `Source/ALPHA/Alarm/AlarmMonitorToEmergency.cpp` (lines 14-53)
- **Without _USE_IOBOARD:** 16 x `CAlarmMonitorToEmergencyButton` threads (one per EMS switch)
- **With _USE_IOBOARD:** 4 x `CAlarmMonitorToEmergencyButton` threads

Each `CAlarmMonitorToEmergencyButton` creates 1 thread:
- **File:** `Source/ALPHA/Alarm/AlarmMonitorToEmergencyButton.cpp` (line 35)
- **Thread function:** Polls `m_hStartEvent` every 10ms, runs `ThreadRun()` which checks PLC bit, sets alarm if triggered
- **Sleep:** 200ms between checks (line 78)

### 4.3 MC OFF Monitor Threads

- **File:** `Source/ALPHA/Alarm/AlarmMonitorToMcOFF.cpp` (lines 13-47)
- **Without _USE_IOBOARD:** 10 x `CAlarmMonitorToMcOFFState` threads
- **With _USE_IOBOARD:** 6 x `CAlarmMonitorToMcOFFState` threads
- **Thread file:** `Source/ALPHA/Alarm/AlarmMonitorToMcOFFState.cpp` (line 32)

### 4.4 Sensor Monitor Threads

- **File:** `Source/ALPHA/Alarm/AlarmMonitorToSensor.cpp` (lines 23-159)
- Sub-monitors created:
  - 1 x `CAlarmMonitorToSensorCurtain` (line 26)
  - 1 x `CAlarmMonitorToSensorVacSwitch` (line 29)
  - 4 x `CAlarmMonitorToSensorIsolater` (lines 34-38)
  - 16 x `CAlarmMonitorToSensorSmoke` (lines 61-65)
  - 16 x `CAlarmMonitorToSensorTemperature` (lines 92-96)
  - 16 x `CAlarmMonitorToSensorTemperatureLow` (lines 98-103)
  - 8+6 x `CAlarmMonitorToSensorFAN` (lines 135-145, without _USE_IOBOARD; 2 with _USE_IOBOARD)
  - 4 x `CAlarmMonitorToLampTime` (lines 156-159)
  - 1 x `CAlarmMonitorToSafetyPLC` (referenced in header)

**Total sensor threads (without _USE_IOBOARD):** ~68 threads

### 4.5 Teach KEY Monitor Thread

- **File:** `Source/ALPHA/Alarm/AlarmMonitorToTeachKEY.cpp` (line 32)
- 1 thread monitoring teach key switch position

### 4.6 Door Open Monitor Threads

- **File:** `Source/ALPHA/Alarm/AlarmMonitorToDoorOpen.cpp` (lines 14-47)
- **Without _USE_IOBOARD:** 6 x `CAlarmMonitorToDoorOpenState` threads
- **With _USE_IOBOARD:** 2 x `CAlarmMonitorToDoorOpenState` threads
- Plus 1 x `CAlarmMonitorToDoorVoice` thread (line 46)

### 4.7 Alarm Thread Summary

| Category | Count (PLC) | Count (IOBoard) |
|----------|-------------|-----------------|
| Emergency buttons | 16 | 4 |
| MC OFF | 10 | 6 |
| Sensor: Curtain | 1 | 1 |
| Sensor: VacSwitch | 1 | 1 |
| Sensor: Isolater | 4 | 4 |
| Sensor: Smoke | 16 | 16 |
| Sensor: Temperature | 16 | 16 |
| Sensor: TemperatureLow | 16 | 16 |
| Sensor: FAN | 14 | 2 |
| Sensor: LampTime | 4 | 4 |
| Sensor: SafetyPLC | 1 | 1 |
| Teach KEY | 1 | 1 |
| Door Open | 6 | 2 |
| Door Voice | 1 | 1 |
| **Total Alarm Threads** | **~107** | **~75** |

**Note:** This is an extremely high thread count for alarm monitoring. Each individual sensor/switch has its own dedicated polling thread.

---

## 5. Communication Threads

### 5.1 CCommEthFunction (TCP/IP Connection Monitor)

- **File:** `CoreFunction/CommEthFunction/CommEthFunction.cpp` (line 576)
- **Thread function:** `ThreadCheckConnect` -- monitors connection state, auto-reconnects every 1 second
- **Events:** `m_hCloseEvent` (auto-reset), `m_hWaitsEvent`
- **Count:** 1 per `CCommEthFunction` instance. Each CommTcpIpObject creates one CCommEthFunction.

### 5.2 CClientSocket Thread

- **File:** `CoreFunction/CommEthFunction/ClientSocket.cpp` (line 13)
- **Thread function:** `ThreadRun` -- handles data send/receive on TCP client socket
- **Events:** `m_hStartEvent` (manual-set), `m_hCloseEvent` (auto-reset), WSA events for socket I/O
- **Pattern:** Polls m_hStartEvent (10ms) for data, m_hCloseEvent (10ms) for shutdown
- **Count:** 1 per CClientSocket (created inside CCommEthFunction/CEthernetManagerTCP for client mode)

### 5.3 CServerSocket Thread

- **File:** `CoreFunction/CommEthFunction/ServerSocket.cpp` (line 13)
- **Thread function:** `ThreadRun` -- handles accept, send/receive on TCP server socket
- **Events:** `m_hAcptDEvent` (manual), `m_hStartEvent` (manual), `m_hCloseEvent` (auto-reset)
- **Count:** 1 per CServerSocket (created inside CCommEthFunction/CEthernetManagerTCP for server mode)

### 5.4 CComm232Function Thread (Serial Port)

- **File:** `CoreFunction/Comm232Function/Comm232Function.cpp` (line 527)
- **Thread function:** `ThreadWorkingLoop` -- monitors serial port via `WaitCommEvent`, reads data
- **Events:** `m_hCloseEvent` with 2500ms timeout
- **Count:** 1 per serial port connection

### 5.5 CommTcpIpObject-Specific Threads

| Class | File | Thread | Purpose |
|-------|------|--------|---------|
| `CCommTcpIpObjectCIM` | `CommTcpIpObjectCIM.cpp:75` | `ThreadRun` | CIM protocol handler (polling m_hStartEvent/m_hCloseEvent) |
| `CCommTcpIpObjectVision` | `CommTcpIpObjectVision.cpp:83` | `QualityWaitThread` | Polls for measurement result files on network drive (per head) |
| `CCommTcpIpObjectVision` | `CommTcpIpObjectVision.cpp:1004` | `ThreadRun` | On-demand thread for filter motor moves (short-lived) |
| `CCommTcpIpObjectTSP` | `CommTcpIpObjectTSP.cpp:1235` | `ThreadRun` | On-demand thread for TSP inspection start (short-lived) |
| `CCommTcpIpObjectColor` | `CommTcpIpObjectColor.cpp:716` | `ThreadRun` | On-demand thread for PG turn-on commands (short-lived) |

### 5.6 CommTcpIpFactoryMgr Device Instances

From `Source/ALPHA/Manager/CommTcpIpFactoryMgr.cpp` (lines 216-288), the following TCP/IP objects are created at startup:

| Object Type | Array | Threads Created |
|-------------|-------|-----------------|
| `CCommTcpIpObjectVision` | `m_pCommTcpIpObj_Vision[]` | N instances (1 CCommEthFunction thread + 1 QualityWaitThread each) |
| `CCommTcpIpObjectColor` | `m_pCommTcpIpObj_Color[]` | N instances (1 CCommEthFunction thread each) |
| `CCommTcpIpObjectAngle` | `m_pCommTcpIpObj_Angle[]` | N instances (1 CCommEthFunction thread each) |
| `CCommTcpIpObjectWAD` | `m_pCommTcpIpObj_WAD` | 1 instance |
| `CCommTcpIpObjectPG` | `m_pCommTcpIpObj_PG` | 1 instance |
| `CCommTcpIpObjectTSP` | `m_pCommTcpIpObj_TSP[]` | N instances |
| `CCommTcpIpObjectLCR` | `m_pCommTcpIpObj_LCR` | 1 instance |
| `CCommTcpIpObjectAgingPG` | `m_pCommTcpIpObj_AgingPG` | 1 instance |
| `CCommTcpIpObjectCIM` | `m_pCommTcpIpObj_CIM` | 1 instance (+ own ThreadRun) |
| `CCommTcpIpObjectSysTime` | `m_pCommTcpIpObj_SysTime` | 1 instance |
| `CCommTcpIpObjectMotion` | `m_pCommTcpIpObj_VisMotion` | 1 instance |

Each CCommEthFunction instance internally creates 1 `ThreadCheckConnect` thread + 1 `CClientSocket` or `CServerSocket` thread.

---

## 6. UI Worker Threads

### 6.1 CDlgMainForm::WorkThreadProbeProc

- **File:** `Source/ALPHA/DlgMainForm.cpp` (lines 1397-1459)
- **Creation:** `AfxBeginThread(WorkThreadProbeProc, this)` in `BeginThread_WorkProc()` (line 1402)
- **Purpose:** Main form monitoring loop -- calls `GetMonitor()` every 50ms
- **Polling:** `WaitForSingleObject(m_hEvtKillThread, 50)` -- 50ms polling interval
- **Shutdown:** `SetEvent(m_hEvtKillThread)` in `TerminateWorkerThread()`, waits via `m_hEvtThreadKilled`

### 6.2 CDlgAlignCCD::WorkThreadProbeProc

- **File:** `Source/ALPHA/DlgAlignCCD.cpp` (line 237)
- **Purpose:** CCD alignment camera monitoring thread (per camera)
- **Count:** Up to `MAIN_CCD_MAX_NUM` (4) threads
- **Shutdown:** Same kill/killed event pattern as DlgMainForm

### 6.3 CDlgSubMaint::ThreadRun

- **File:** `Source/ALPHA/DlgSubMaint.cpp` (line 156)
- **Purpose:** Maintenance dialog background operations (motor jog, IO control, etc.)
- **Events:** 5-event WaitForMultipleObjects (4 start events + 1 close event)
- **Pattern:** 4 different start events trigger 4 different operations

### 6.4 CDlgSubMaint_MotionFile::ThreadRun

- **File:** `Source/ALPHA/DlgSubMaint_MotionFile.cpp` (line 82)
- **Purpose:** Motion file management operations (teach position save/load)
- **Events:** 5-event WaitForMultipleObjects (4 start events + 1 close event)

### 6.5 CD_PopUp_Motion_Move::ThreadRun

- **File:** `Source/ALPHA/CD_PopUp_Motion_Move.cpp` (line 62)
- **Purpose:** Modal popup for axis motion moves
- **Events:** 3-event WaitForMultipleObjects (2 start events + 1 close event)

### 6.6 CDlgSubManual_Meas::ThreadRun

- **File:** `Source/ALPHA/DlgSubManual_Meas.cpp` (line 46)
- **Purpose:** Manual measurement sequence operations
- **Events:** 2-event WaitForMultipleObjects (1 start + 1 close)

### 6.7 CDlgSubOperate Threads

- **File:** `Source/ALPHA/DlgSubOperate.cpp`
  - `ThreadInspectEnd` (line 1637): Short-lived thread for inspection stop/cleanup
  - `ThreadRun` (line 3053): Short-lived thread for CIM state update (`UpdateCIMState`)

---

## 7. Core Library Threads

### 7.1 CMcAddressScannerImpl (PLC Scanner)

- **File:** `CoreFunction/IOMapFunction/McProtocol/McAddressScannerImpl.cpp` (line 15)
- **Purpose:** Continuously reads PLC MC Protocol addresses over TCP/IP
- **Polling:** `WaitForSingleObject(m_hStartEvent, 10)` + `Sleep(200)` = ~210ms cycle
- **Thread function:** Reads PLC bit/word data, updates IO map
- **Count:** 1 (created by `CIOMapFunction` -> `CMcProtocolManager`)

### 7.2 CRecipeEditMgr::ThreadRun

- **File:** `Source/ALPHA/Recipe/RecipeEditMgr.cpp` (lines 42, 835-855)
- **Purpose:** Monitors shared memory for recipe change requests from external DLL
- **Polling:** `WaitForSingleObject(m_hCloseEvent, 10)` -- polls every 10ms
- **Thread function:** Checks `VTM_RECIPE` shared memory for `M_RUN_REQ` status

### 7.3 CMyLabelTimer::TimerCB

- **File:** `CoreFunction/common/MyLabelTimer.cpp` (lines 134, 145-169)
- **Purpose:** UI timer label countdown display
- **Creation:** `AfxBeginThread(TimerCB, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED)` -- created suspended, resumed when needed
- **Polling:** `WaitForSingleObject(m_hEventStop, m_iDelayTime)` -- configurable delay
- **Count:** Variable, 1 per active timer label

### 7.4 CDlgResult::CBResultMonitorProc

- **File:** `CoreFunction/dResult/CDlgResult.cpp` (line 218)
- **Purpose:** Monitors shared memory for measurement result data
- **Polling:** `WaitForSingleObject(m_hEventStop, 100)` -- 100ms cycle
- **Count:** 1 (when search mode enabled)

### 7.5 CDlgResult::WorkThreadExportDataProc

- **File:** `CoreFunction/dResult/CDlgResult.cpp` (line 1736)
- **Purpose:** Short-lived thread for exporting total result data
- **Count:** Created on-demand, terminates after export

### 7.6 CShareManager::CBResultMonitorProc (dSACC)

- **File:** `CoreFunction/dSACC/ShareManager.cpp` (line 209)
- **Purpose:** Monitors shared memory for SACC result data
- **Polling:** `WaitForSingleObject(m_hEventStop, 100)` -- 100ms cycle

### 7.7 CDlgnMCCReport::WorkThreadProbeProc

- **File:** `CoreFunction/dMCCSpec/DlgnMCCReport.cpp` (line 1072)
- **Purpose:** MCC log report worker thread (processes MCC send/receive jobs)
- **Pattern:** Work-dispatch via `WaitForSingleObject(m_hEvtWorkStart, INFINITE)`, kill via `m_hEvtKillThread`
- **Events:** m_hEvtWorkStart, m_hEvtKillThread, m_hEvtThreadKilled, m_hEvtWorkDone

---

## 8. Synchronization Mechanisms

### 8.1 CCriticalSection Usage

| Location | Variables | Protected Data |
|----------|-----------|----------------|
| `CommonFunc.cpp` (lines 16-24) | `g_csQuery`, `g_csTime`, `g_csTime2`, `g_csDir`, `g_csDay`, `g_csCTime`, `g_csSTime`, `g_csETime`, `g_chTime` | Global time/query/directory functions |
| `ShareMemFunction.h` (line 112) | `m_cs` | Shared memory read/write |
| `ShareMemoryMgr.h` (lines 17-18) | `m_cs_set`, `m_cs_get` | Shared memory get/set operations |
| `CommEthFunction.h` (line 8) | `m_cs` | Socket send/receive |
| `MotorControlMgr.h` (lines 22-26) | `m_cs`, `m_cs_Get`, `m_cs_Set`, `m_cs_done`, `m_cs_Stop` | Motor control state, position, done flags |
| `IOMapControlMgr.h` (lines 19-21) | `m_cs`, `m_cs_get[4]`, `m_cs_set[4]` | PLC IO map read/write (per channel) |
| `LightControlMgr.h` (lines 16-20) | `m_cs_on`, `m_cs_off`, `m_cs_set`, `m_cs_get`, `m_cs_res` | Light controller on/off/set/get/result |
| `AlignCameraMgr.h` (lines 18-19) | `m_cs_get`, `m_cs_res` | Camera parameter access |
| `AlarmController.h` (lines 9-10) | `m_cs_set`, `m_cs_get` | Alarm set/get operations |
| `AlarmManager.h` (lines 29-30) | `m_cs_set`, `m_cs_get` | Alarm manager data |
| `CommTcpIpObjectPG.h` (line 91) | `m_cs_Set` | PG command set operations |
| `BaseSequenceBody.h` (line 10) | `m_cs` | Sequence body state |
| `LogFileAccessor.h` (line 8) | `m_cs` | Log file write |
| `LogWrapper.h` (line 8) | `m_cs` | Log wrapper write |
| `DlgAlarm.cpp` (line 21) | `g_csAlarm` | Alarm dialog access |
| `DlgWarning.cpp` (line 18) | `g_csWarring` | Warning dialog access |
| `CDlgResult.cpp` (line 14) | `g_csUpdateSpead` | Result spread update |

### 8.2 Event Objects (HANDLE)

| Category | Events | Usage |
|----------|--------|-------|
| **Sequence Control** | `m_hStartEvent`, `m_hCloseEvent` | Start/stop sequence threads |
| **Sequence Completion** | `m_hCompleteEvent` | Signal sequence step completion |
| **Sequence Wait** | `m_hWaitTimeEvent` | Timed waits within sequences |
| **User Pause/Resume** | `m_hEvtUserPause` (manual-reset), `m_hEvtUserResume` (auto-reset) | User-initiated pause/resume |
| **Worker Thread Kill** | `m_hEvtKillThread`, `m_hEvtThreadKilled` | Worker thread shutdown handshake |
| **Worker Thread Work** | `m_hEvtWorkStart`, `m_hEvtWorkDone` | Work dispatch and completion (MCC Report) |
| **Comm Socket** | `m_hMesgSocketEvent`, `m_hAcptSocketEvent` | WSA events for socket I/O |
| **Comm Response** | `m_hRecvDEvent[]` | Response event arrays for device communication (Angle, CIM, Vision) |

### 8.3 Pause/Resume Mechanism

Defined in `CBaseSequenceBody` (`BaseSequenceBody.h` lines 52-53, 78-80):
- `m_hEvtUserPause`: Manual-reset event. `SetEvent` = pause requested.
- `m_hEvtUserResume`: Auto-reset event. `SetEvent` = resume.
- `IfPauseEvt_WaitResumeEvt()` (`BaseSequenceBody.cpp` lines 46-64): If pause set, blocks on resume event with INFINITE wait.

---

## 9. Thread Lifecycle Management

### 9.1 Startup Sequence

1. `CALPHAApp::InitInstance()` creates `CAuroraMgr` which creates all Manager objects
2. `CSequenceManager::LinkParam()` creates 6 CBaseSequenceImpl threads + their sub-engines
3. `CAlarmMonitorMgr` constructor creates all alarm monitor threads (dormant until `SetMonitoringStart()`)
4. `CommTcpIpFactoryMgr` creates device communication objects (each spawns CCommEthFunction threads)
5. `CDlgMainForm::BeginThread_WorkProc()` starts main UI monitor thread
6. `CDlgAlignCCD::BeginThread_WorkProc()` starts CCD monitor threads (up to 4)
7. `AlarmMonitorMgr::SetMonitoringStart()` activates all alarm monitor threads

### 9.2 Shutdown Sequence

1. `CDlgMainForm::TerminateWorkerThread()` signals all UI worker threads to stop
2. `CSequenceManager::~CSequenceManager()` deletes all sequence items (each destructor signals CloseEvent and waits)
3. `CAlarmMonitorMgr::SetMonitoringStop()` signals all alarm monitors to stop, then `Sleep(1000)`
4. `CCommEthFunction::Finalize()` signals close and waits 500ms for thread exit
5. Each CClientSocket/CServerSocket destructor signals close and waits INFINITE

---

## 10. Thread Safety Concerns

### 10.1 High Thread Count

The alarm monitoring subsystem alone creates **75-107 threads** (depending on `_USE_IOBOARD` define). This is an unusually high count for sensor monitoring. Most threads are polling at 10ms intervals with `WaitForSingleObject`.

### 10.2 SuspendThread/ResumeThread Usage

`CBaseSequenceImpl::Pause()` and `Resume()` use `SuspendThread()`/`ResumeThread()` (lines 146-158). This is dangerous:
- `SuspendThread` can suspend a thread while it holds a lock, causing deadlock
- The `CBaseSequenceBody::IfPauseEvt_WaitResumeEvt()` method provides a safer event-based pause mechanism
- Both mechanisms coexist, creating inconsistency

### 10.3 INFINITE Wait Risks

Several shutdown sequences use `WaitForSingleObject(..., INFINITE)`:
- `CBaseSequenceImpl::~CBaseSequenceImpl()` (line 46)
- `CClientSocket::~CClientSocket()` (line 40)
- `CServerSocket::~CServerSocket()` (line 56)
- If the target thread is blocked (e.g., waiting for network I/O), shutdown hangs indefinitely

### 10.4 Short-Lived Thread Proliferation

Several functions create threads that run once and exit:
- `CCommTcpIpObjectVision::SetRunToThread()` -- creates a new thread per filter move
- `CCommTcpIpObjectTSP::InspectStartReq()` -- creates a new thread per inspection start
- `CCommTcpIpObjectColor::SetRunToThread()` -- creates a new thread per PG command
- `CDlgSubOperate::UpdateCIMState()` -- creates a new thread per CIM state update
- `CDlgResult::ExportAsTotalData()` -- creates a new thread per data export

These are fire-and-forget with no lifecycle management.

### 10.5 Race Conditions

- `CMyLabelTimer::StopTimer()` calls `TerminateThread()` as fallback after 2-second timeout (line 114). This is unsafe and can corrupt process state.
- Several `CCommEthFunction` connect/disconnect paths may have races between the ThreadCheckConnect reconnect loop and explicit `CloseConnect()` calls.

### 10.6 CCriticalSectionSync Constructor Bug

The `CCriticalSectionSync` always calls `Unlock()` in the destructor (line 76), but only calls `Lock()` if `bInitialLock == TRUE` (line 69). If constructed with `bInitialLock = FALSE`, the destructor will unlock a lock that was never acquired, which is undefined behavior for `CCriticalSection`. In practice, all call sites use `TRUE`, so this is a latent bug.

---

## 11. Thread Count Summary

| Category | Minimum | Maximum |
|----------|---------|---------|
| Sequence Manager threads (CBaseSequenceImpl) | 6 | 6 |
| DoWorkGlassSequenceEngine | 1 | 1 |
| DoWork Glass Sequence Items | 9 | 9 |
| MeasureSequenceItemEngine | 1+ | 2+ |
| MeasureSequenceItemImpl | 2+ | 10+ |
| Alarm Monitor threads | 75 | 107 |
| CCommEthFunction (TCP/IP monitor) | 10 | 13 |
| CClientSocket / CServerSocket | 10 | 13 |
| CommTcpIpObject-specific threads | 2 | 5+ |
| UI Worker threads | 5 | 8 |
| PLC Scanner | 1 | 1 |
| RecipeEditMgr | 1 | 1 |
| dResult/dSACC monitors | 0 | 2 |
| MCC Report worker | 0 | 1 |
| Timer threads | 0 | 4+ |
| Short-lived threads | 0 | variable |
| **Total Estimate** | **~120** | **~180+** |

**Note:** The dominant contributor is the alarm monitor subsystem with its per-sensor thread model. The actual runtime count depends on build configuration (`_USE_IOBOARD`) and the number of configured communication devices.
