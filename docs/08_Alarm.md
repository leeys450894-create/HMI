# 08. Alarm System Analysis

## 1. Architecture Overview

The ALPHA HMI alarm system is a multi-layered architecture consisting of:

1. **Alarm DLL** (`CoreFunction/dll_Alarm/Alarm.dll`) -- Core alarm management, UI dialogs, data persistence
2. **AlarmController** (Singleton) -- Application-level facade wrapping DLL API calls
3. **AlarmMonitorMgr** -- Manager that owns and coordinates multiple monitor groups
4. **AlarmMonitorImpl** (abstract) -- Base class for monitor groups
5. **Individual Monitor Threads** -- Each physical I/O point has its own dedicated thread

```
CAlarmMonitorMgr (CAuroraSubMgr)
  |-- vector<CAlarmMonitorImpl*> m_vecMonitor
       |-- CAlarmMonitorToEmergency    --> 4~16 CAlarmMonitorToEmergencyButton threads
       |-- CAlarmMonitorToMcOFF        --> 6~10 CAlarmMonitorToMcOFFState threads
       |-- CAlarmMonitorToSensor       --> 80+ individual sensor threads
       |    |-- CAlarmMonitorToSensorCurtain     (1 thread)
       |    |-- CAlarmMonitorToSensorVacSwitch   (1 thread)
       |    |-- CAlarmMonitorToSensorIsolater    (4 threads)
       |    |-- CAlarmMonitorToSensorFAN         (8+6=14 threads, PLC mode)
       |    |-- CAlarmMonitorToSensorSmoke       (16 threads)
       |    |-- CAlarmMonitorToSensorTemperature (16 threads)
       |    |-- CAlarmMonitorToSensorTemperatureLow (16 threads)
       |    |-- CAlarmMonitorToSensorPressure    (16 threads)
       |    |-- CAlarmMonitorToLampTime          (4 threads)
       |    |-- CAlarmMonitorToSafetyPLC         (1 thread)
       |-- CAlarmMonitorToTeachKEY     --> 1 thread
       |-- CAlarmMonitorToDoorOpen     --> 6 CAlarmMonitorToDoorOpenState threads + 1 CAlarmMonitorToDoorVoice thread
       |-- (CAlarmMonitorToTeachKEYLock) --> commented out

CAlarmController (Singleton)
  |-- Wraps Alarm_* DLL API functions
  |-- CCriticalSection m_cs_set, m_cs_get (thread-safe)

Alarm DLL (CAlarmManager)
  |-- CDlgAlarm (alarm list dialog)
  |    |-- CDlgAlarmModule (detail/recovery dialog)
  |    |-- CMySpreadDll (Excel data reader for Alarm.xls)
  |-- CDlgWarning (light alarm popup)
  |-- CDlgReportAlarm (alarm history/report)
  |-- CMap<UINT, ALARM_DATA> m_alarmTable (alarm definition lookup)
```

## 2. Key Source Files

### Application Layer (Source/ALPHA/)
| File | Class | Purpose |
|------|-------|---------|
| `Alarm/AlarmMonitorMgr.cpp/.h` | `CAlarmMonitorMgr` | Top-level monitor manager, owns all monitor groups |
| `Alarm/AlarmMonitorImpl.cpp/.h` | `CAlarmMonitorImpl` | Abstract base for monitor groups |
| `Alarm/AlarmController.cpp/.h` | `CAlarmController` | Singleton facade for Alarm DLL API |
| `Alarm/AlarmMonitorToEmergency.cpp/.h` | `CAlarmMonitorToEmergency` | EMS button monitor group |
| `Alarm/AlarmMonitorToEmergencyButton.cpp/.h` | `CAlarmMonitorToEmergencyButton` | Individual EMS button thread |
| `Alarm/AlarmMonitorToMcOFF.cpp/.h` | `CAlarmMonitorToMcOFF` | MC contactor OFF monitor group |
| `Alarm/AlarmMonitorToMcOFFState.cpp/.h` | `CAlarmMonitorToMcOFFState` | Individual MC OFF thread |
| `Alarm/AlarmMonitorToSensor.cpp/.h` | `CAlarmMonitorToSensor` | Sensor monitor group (smoke, temp, fan, pressure, etc.) |
| `Alarm/AlarmMonitorToSensorCurtain.cpp/.h` | `CAlarmMonitorToSensorCurtain` | Light curtain sensor thread |
| `Alarm/AlarmMonitorToSensorFAN.cpp/.h` | `CAlarmMonitorToSensorFAN` | Fan failure sensor thread |
| `Alarm/AlarmMonitorToSensorSmoke.cpp/.h` | `CAlarmMonitorToSensorSmoke` | Smoke detector thread |
| `Alarm/AlarmMonitorToSensorTemperature.cpp/.h` | `CAlarmMonitorToSensorTemperature` | Over-temperature sensor thread |
| `Alarm/AlarmMonitorToSensorTemperatureLow.cpp/.h` | `CAlarmMonitorToSensorTemperatureLow` | Temperature warning thread |
| `Alarm/AlarmMonitorToSensorPressure.cpp/.h` | `CAlarmMonitorToSensorPressure` | Pressure switch sensor thread |
| `Alarm/AlarmMonitorToSensorVacSwitch.cpp/.h` | `CAlarmMonitorToSensorVacSwitch` | Vacuum switch sensor thread |
| `Alarm/AlarmMonitorToSensorIsolater.cpp/.h` | `CAlarmMonitorToSensorIsolater` | Isolator sensor thread |
| `Alarm/AlarmMonitorToSensorHandSwitch.cpp/.h` | `CAlarmMonitorToSensorHandSwitch` | Hand switch sensor (included but usage unclear) |
| `Alarm/AlarmMonitorToLampTime.cpp/.h` | `CAlarmMonitorToLampTime` | Lamp timeout monitor thread |
| `Alarm/AlarmMonitorToSafetyPLC.cpp/.h` | `CAlarmMonitorToSafetyPLC` | Safety PLC connection monitor thread |
| `Alarm/AlarmMonitorToTeachKEY.cpp/.h` | `CAlarmMonitorToTeachKEY` | Teach/Auto key switch monitor thread |
| `Alarm/AlarmMonitorToDoorOpen.cpp/.h` | `CAlarmMonitorToDoorOpen` | Door open monitor group |
| `Alarm/AlarmMonitorToDoorOpenState.cpp/.h` | `CAlarmMonitorToDoorOpenState` | Individual door sensor thread |
| `Alarm/AlarmMonitorToDoorVoice.cpp/.h` | `CAlarmMonitorToDoorVoice` | Door open voice announcement thread |
| `DlgSubAlarm.cpp/.h` | `CDlgSubAlarm` | Main alarm tab UI in the application |
| `CD_PopUp_Alarm_F.cpp/.h` | `CD_PopUp_Alarm_F` | Popup showing active fault I/O bits |

### Alarm DLL (CoreFunction/dll_Alarm/)
| File | Class | Purpose |
|------|-------|---------|
| `Alarm/API_Alarm.cpp/.h` | (free functions) | DLL exported API (`Alarm_*` functions) |
| `Alarm/AlarmManager.cpp/.h` | `CAlarmManager` | Core alarm engine: lookup, set, clear, dialog management |
| `Alarm/AlarmDefine.cpp/.h` | -- | `ALARM_DATA`, `ALARM_EVENT_DATA`, `ALARM_MESSAGE` structs, WM messages |
| `Alarm/DlgAlarm.cpp/.h` | `CDlgAlarm` | Active alarm list dialog with CListCtrl |
| `Alarm/DlgAlarmModule.cpp/.h` | `CDlgAlarmModule` | Alarm detail/recovery button panel |
| `Alarm/DlgReportAlarm.cpp/.h` | `CDlgReportAlarm` | Alarm history list + Access DB persistence |
| `Alarm/DlgReportAlarmSearch.cpp/.h` | `CDlgReportAlarmSearch` | Alarm history search dialog |
| `Alarm/AlarmFiles/AlarmCSVFile.cpp/.h` | `CAlarmCSVFile` | AlarmMessage.csv parser |
| `Alarm/AlarmFiles/AlarmXLSFile.cpp/.h` | `CAlarmXLSFile` | Alarm.xls/.dat parser (legacy) |
| `Alarm/AlarmFiles/AlarmFileTransfer.cpp/.h` | `CAlarmFileTransfer` | Alarm file transfer utility |

### Header/Definition Files
| File | Purpose |
|------|---------|
| `Source/Include/AlarmDefine.h` | `ALARM_DATA`, `ALARM_EVENT_DATA`, `ALARM_MESSAGE` structs, WM_ALARM_* messages |
| `Source/Include/AlarmIDList.h` | `enumAlarmID` enum -- all alarm ID constants (1000~10000) |
| `Source/Include/API_Alarm.h` | DLL API function declarations |

### Configuration Files
| File | Purpose |
|------|---------|
| `Source/Bin/Config/CoreFunction/AlarmConfig/Alarm.xls` | Alarm definition master table (Excel) |
| `Source/Bin/Config/CoreFunction/AlarmConfig/AlarmMessage.csv` | Alarm message text table (CSV) |
| `Source/Bin/Config/CoreFunction/AlarmConfig/AlarmMessage.dat` | Compiled alarm message data |
| `Source/Bin/Config/CoreFunction/AlarmConfig/Alarm.dat` | Compiled alarm data (legacy) |

## 3. Alarm Data Structures

Defined in `Source/Include/AlarmDefine.h` and `CoreFunction/dll_Alarm/Alarm/AlarmDefine.h`:

### ALARM_DATA
```cpp
struct ALARM_DATA {
    UINT id;            // Alarm ID key
    UINT code;          // Alarm code
    UINT nRet;          // Return value (recovery action taken)
    UINT nRetry;        // Retry enabled (1=yes, 0=no)
    UINT nType;         // Type (1=Danger for human, 2=Machine HW Error, 3, 4...)
    UINT nInsertLine;   // List control insert position
    UINT level;         // ALARM_LEVLE_HIGH / LOW / MSG
    TCHAR module[256];  // Module name
    TCHAR cmd[1024];    // Alarm command
    TCHAR text[1024];   // Alarm text description
    TCHAR Description[1024]; // Detailed description
    TCHAR Num[256];
    TCHAR happendTime[32];
    TCHAR clearedTime[32];
};
```

### ALARM_EVENT_DATA (extends ALARM_DATA)
```cpp
struct ALARM_EVENT_DATA : public ALARM_DATA {
    HANDLE hEvent;  // Manual-reset event for blocking until recovery
};
```

### Alarm Levels
```cpp
enum ALARM_LEVEL_VALUE {
    ALARM_LEVLE_HIGH = 0,  // Heavy alarm -- blocks thread until user responds
    ALARM_LEVLE_LOW,       // Light alarm (Warning) -- non-blocking popup
    ALARM_LEVLE_MSG,       // Message -- non-blocking, no WM_WARNING message
};
```

### Recovery Return Values
```cpp
enum ALARM_RETURN_VALUE {
    ALARM_RV_SUCCESSED = 0,     // Already exists (duplicate)
    ALARM_RV_IGNORE,            // User clicked Ignore
    ALARM_RV_RETRY,             // User clicked Retry
    ALARM_RV_INITRETRY,         // User clicked Init+Retry
    ALARM_RV_ABORT,             // User clicked Stop/Abort
    ALARM_RV_PROGRAMEXIT,       // Program is shutting down
    ALARM_RV_ALARM_NOT_EXIST = 101, // Alarm ID not found in table
};
```

### Custom Windows Messages
```cpp
#define WM_ALARM_ACCURED    (WM_USER + 1300)  // Heavy alarm occurred
#define WM_ALARM_CLEARED    (WM_USER + 1301)  // Heavy alarm cleared
#define WM_WARNING_ACCURED  (WM_USER + 1302)  // Warning occurred
#define WM_WARNING_CLEARED  (WM_USER + 1303)  // Warning cleared
```

## 4. Alarm Code Definitions (enumAlarmID)

All alarm codes are defined in `Source/Include/AlarmIDList.h` as `enumAlarmID`. Range: 1000--10000.

### Job/Process Alarms (1000--1199)
| ID | Name | Level |
|----|------|-------|
| 1000 | `JOB_PROCESS_ALARM` | -- |
| 1001 | `JOB_PROCESS_UNKONWN_ALARM` | -- |
| 1002 | `JOB_PROCESS_OUTOFSERVICE` | -- |
| 1003 | `JOB_PROCESS_VIRTUAL_MODE` | -- |
| 1004 | `JOB_PROCESS_RUN_ERROR` | -- |
| 1011 | `JOB_LOTID_NOT_EXIST` | H |
| 1012 | `JOB_GLSID_NOT_EXIST` | H |
| 1013 | `JOB_RECIPE_NOT_EXIST` | H |
| 1014 | `JOB_RECIPE_NOT_SELECTED` | H |
| 1015 | `JOB_LOAD_SEQ_GLASS_DETECTED` | H |
| 1016 | `JOB_MOVE_TO_UNLOAD_FAIL` | H |
| 1017 | `JOB_INIT_AF_MODULE_ERROR` | H |
| 1018 | `JOB_GLASS_NOT_EXIST` | H |
| 1019 | `JOB_GLASS_AND_VACUUM_SENSOR_MISMATCH_ERROR` | H |
| 1020 | `VTM_RV_STOP_BY_USER` | L |
| 1021 | `JOB_FILE_OPEN_ERROR` | L |
| 1022 | `JOB_FILE_WRITE_ERROR` | L |
| 1023 | `JOB_SEND_APD_FAIL` | L |
| 1024 | `JOB_LAMP_TIMEOUT` | L |
| 1025 | `JOB_MEAS_DATA_OUT_OF_SPEC` | L |
| 1100 | `JOB_PROC_NO_ERROR` | H |
| 1101 | `JOB_PROC_MEAS_STATUS_ERROR` | -- |
| 1102 | `JOB_PROC_ALIGN_ERROR` | -- |
| 1103 | `JOB_PROC_SHUTTER_ERROR` | -- |
| 1104 | `JOB_PROC_SAFETY_POS_ERROR` | H |
| 1105 | `JOB_PROC_ALIGN_POS_ERROR` | H |
| 1106 | `JOB_PROC_MEAS_POS_ERROR` | H |
| 1107--1110 | `JOB_PROC_LOADER_*_ERROR` | -- |
| 1111 | `JOB_PROC_PRE_ALIGN_POS_ERROR` | H |
| 1112 | `UNIT_INTERLOCK_MOVE_FAIL` | -- |
| 1113 | `UNIT_READY_MOVE_FAIL` | -- |
| 1114 | `UNIT_MEAS_MOVE_FAIL` | -- |
| 1115 | `JOB_PROC_SACC_SEQ_ERROR` | -- |

### Measurement Alarms (1201--1209)
| ID | Name | Level |
|----|------|-------|
| 1201 | `MEAS_FAIL_REF` | H |
| 1202 | `MEAS_FAIL_DARK` | H |
| 1203 | `MEAS_FAIL_SAMAPLE` | H |
| 1204--1209 | `TR_MEAS_FAIL_*`, `SR_MEAS_FAIL_*` | H |

### HEAD Communication Alarms (2001--2081)
| ID | Name | Level |
|----|------|-------|
| 2001 | `HEAD_NOT_CONNECTED` | -- |
| 2021 | `HEAD_ACK_TIMEOUT` | -- |
| 2041 | `HEAD_ACK_FAIL` | -- |
| 2061 | `HEAD_COMMAND_ERROR` | -- |
| 2081 | `HEAD_COMPLETE_NG` | -- |

### Motion Alarms (3001--3801)
| ID | Name | Level |
|----|------|-------|
| 3001 | `MEI_NOT_CREATED` | H |
| 3002 | `MEI_INIT_MC_FAIL` | H |
| 3012 | `MOTION_CMDPOS_ERROR` | -- |
| 3021--3029 | `MOTION_INTERLOCK_1`--`9` | -- |
| 3101 | `MTOR_DRIVER_ALARM` | -- |
| 3201 | `MTOR_HOME_FAIL_ALARM` | -- |
| 3301 | `MTOR_MOVE_DONE_TIME_OUT_ALARM` | -- |
| 3401 | `MTOR_DRIVER_ENABLE_ALARM` | -- |
| 3501 | `MOTION_OVER_LIMIT_POS` | -- |
| 3601 | `MOTION_OVER_LIMIT_NEG` | -- |
| 3701 | `MOTION_INTERLOCK_CHECK_ALARM` | -- |
| 3801 | `MOTION_MOVE_PROBE_CROSS_N_ROTATE_ALARM` | -- |

### PLC Alarms (4001--4099+)
| ID | Name | Level |
|----|------|-------|
| 4001 | `PLC_SOCKET_CREATE_FAIL` | H |
| 4002 | `PLC_INITIAL_FAIL` | H |
| 4003 | `PLC_COMMAND_TIMEOUT` | H |
| 4004--4009 | `PLC_READY/LOAD/MEAS_FAIL_TIMEOUT`, `PLC_STATE_NOT_*` | H |
| 4010--4011 | `PLC_CANNOT_DETECT_SAFETY_SENSOR_X/Y` | H |
| 4012--4016 | `PLC_GLASS_EXIST_*`, `PLC_GLASS_OBLIVIOUS_*` | H |
| 4017--4058 | Various PLC I/O errors (AIR, VACUUM, DOOR, EMO, MC OFF, TEMP, CYLINDER, LIFT PIN, SHUTTER, etc.) | H/L |

### Real-time I/O Monitor Alarm IDs (4481--4700+)

These are the alarm IDs used by the monitoring threads:

| ID Range | Name Pattern | Monitor | Count |
|----------|-------------|---------|-------|
| 4481--4495 | `PLC_MC_OFF_ALARM_101`..`PLC_SMPS1_1_CP_OFF_ALARM` | CAlarmMonitorToMcOFF | 10+ |
| 4496--4500 | `PLC_GPS/UPS_BREAKER_*_ALARM`, `PLC_DELAY_TIMER_OFF_ALARM` | CAlarmMonitorToMcOFF | 5 |
| 4501--4520 | `PLC_TEMPERATURE_SENSOR_OVER_01`..`_END` | CAlarmMonitorToSensorTemperature | 16 |
| 4521--4540 | `PLC_TEMPERATURE_SENSOR_WARRING_01`..`_END` | CAlarmMonitorToSensorTemperatureLow | 16 |
| 4551--4570 | `PLC_EMO_SWITCH_PUSHED_01`..`_END` | CAlarmMonitorToEmergencyButton | 16 |
| 4581--4586 | `PLC_DOOR_OPENED_01`..`06` | CAlarmMonitorToDoorOpenState | 6 |
| 4601--4608 | `PLC_ELECTRIC_BOX_FAN_ERROR_01`..`08` | CAlarmMonitorToSensorFAN | 8 |
| 4611--4618 | `PLC_PC_BOX_FAN_ERROR_01`..`08` | CAlarmMonitorToSensorFAN | 6+ |
| 4651--4658 | `PLC_ISOLATOR_SENSOR_01`..`08` | CAlarmMonitorToSensorIsolater | 4 |
| 4661--4685 | `PLC_PRESSURE_*` | CAlarmMonitorToSensorPressure | 16 |
| 4691--4710 | `PLC_SMOKE_DETECT_ALARM_01`..`_END` | CAlarmMonitorToSensorSmoke | 16 |

### Vision/Optics Alarms (6001--6620)
| ID Range | Category |
|----------|----------|
| 6001--6014 | Vision (align, AF, pattern, snap) |
| 6101--6102 | Line scan |
| 6201--6203 | LED light controller |
| 6301--6302 | Illumination measurement |
| 6401--6405 | VCR (barcode reader) |
| 6501--6516 | PCAN module |

### External System Alarms (7001--7920)
| ID Range | Category |
|----------|----------|
| 7001--7013 | CIM/MES communication |
| 7100--7192 | PG (Pattern Generator) module |
| 7201--7243 | Quality measurement module |
| 7301--7344 | Viewing angle module |
| 7400--7406 | Property measurement module |
| 7501--7506 | PPA module |
| 7601--7608 | Aligner module |
| 7701--7718 | SE (Spectral Ellipsometer) |
| 7801--7808 | AFS (Auto Focus) |
| 7851--7862 | RS8 serial device |
| 7901--7920 | Zoom scope |

### Probe/SACC Alarms (8001--8050)
| ID Range | Category |
|----------|----------|
| 8001--8006 | Probe get/put from unit |
| 8007--8014 | Probe get/put from SACC |
| 8015--8023 | Probe existence, PCB lock, clamp, zero position |
| 8024--8026 | PACC (AC unloading, Z down) |
| 8027--8028 | SACC gate |
| 8031--8044 | SACC interface (mode, state, O2, commands, slot errors) |

### System/FDC/Other (8501--9998)
| ID | Name |
|----|------|
| 8501 | `FDC_LIVE_CHECK_ERROR` |
| 8502 | `SDC_SAFETYPLC_MODULE_CONNECT_ERROR` |
| 9001 | `SDC_PPA_MODULE_FILE_EXIST_ERROR` |
| 9002 | `SDC_SE_MODULE_FILE_EXIST_ERROR` |
| 9003 | `SDC_DEF_MODULE_FILE_EXIST_ERROR` |
| 9998 | `SDC_SEM_CONTROLLER_HW_ERROR` |

## 5. Alarm Monitoring Threads

### Thread Architecture

Each individual monitor item creates its own `CWinThread` via `AfxBeginThread()` in its constructor. The thread pattern is consistent across all monitor types:

```
Constructor:
  m_pThread = AfxBeginThread(ThreadRun, this);
  m_hStartEvent = CreateEvent(NULL, TRUE, FALSE, NULL);   // manual-reset
  m_hCloseEvent = CreateEvent(NULL, FALSE, FALSE, NULL);  // auto-reset

Static ThreadRun(LPVOID):
  while (1) {
    if (WaitForSingleObject(m_hStartEvent, 10) == WAIT_OBJECT_0) {
      Sleep(<poll_interval>);     // varies per monitor type
      ThreadRun();                // instance method -- does the actual check
    }
    if (WaitForSingleObject(m_hCloseEvent, 10) == WAIT_OBJECT_0) {
      break;                      // exit thread
    }
    Sleep(1);
  }
```

### Thread Count Summary (PLC mode, non-`_USE_IOBOARD`)

| Monitor Group | Thread Count | Poll Interval |
|---------------|-------------|---------------|
| Emergency Buttons (EMS) | 16 | 200ms |
| MC OFF States | 10 | 200ms |
| Sensor Curtain | 1 | 200ms |
| Vacuum Switch | 1 | Unknown |
| Isolator Sensors | 4 | Unknown |
| FAN Sensors (EBox + PCBox) | 8 + 6 = 14 | Unknown |
| Smoke Detectors | 16 | 400ms |
| Temperature Over | 16 | 1000ms |
| Temperature Warning | 16 | Unknown |
| Pressure Switches | 16 | Unknown |
| Lamp Timeout | 4 | Unknown |
| Safety PLC | 1 | Unknown |
| Teach KEY | 1 | 1000ms |
| Door Open States | 6 | 600ms |
| Door Voice | 1 | Unknown |
| **Total** | **~124** | |

In `_USE_IOBOARD` mode (IO board hardware), the thread counts are reduced (e.g., 4 EMS buttons, 6 MC OFFs, 2 fans, 3 pressures).

### Start/Stop Control

- `CAlarmMonitorMgr::SetMonitoringStart()` -- Iterates all monitor groups, calls `SetStart()` on each
- Each group's `SetStart()` calls `SetEvent(m_hStartEvent)` on all child items
- `SetMonitoringStop()` calls `ResetEvent(m_hStartEvent)` on all children, then `Sleep(1000)` to allow threads to stop polling

## 6. Alarm Occurrence Flow

### Detection Phase (Monitor Thread)

Each monitor thread follows this pattern in its `ThreadRun()` instance method:

1. **Guard checks** (all monitors perform these):
   - `m_pSystemConfigMgr->IsUseAlarmMonitor()` -- global alarm monitoring enable
   - `m_pSystemConfigMgr->IsUseHardwareSensor()` -- hardware sensor use flag (most sensors)
   - `m_pIOMapControlMgr->IsConnected()` -- PLC/IO board connection check

2. **I/O read**: `m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(m_iMonitorIONumber)`
   - Returns -1 (error), 0 (inactive), or 1 (active)
   - Some monitors check for `nRet == 0` (normally-closed signals like MC contact, smoke, temperature)
   - Some check for `nRet == 1` (normally-open signals like EMS button push, door open)

3. **Debounce** (Emergency buttons only): Counts consecutive NG readings (`m_nNGCount > 3` triggers alarm)

4. **Alarm trigger**:
   ```cpp
   m_pLogFileAccessor->KMLOG(WARN, m_strLogMsg);       // Log to file
   CAlarmController::GetInstance()->SetAlarm(m_iAlarmID); // Raise alarm
   ```

### Processing Phase (Alarm DLL)

`CAlarmController::SetAlarm(id)` calls `Alarm_SetAlarm(id)` which reaches `CAlarmManager::SetAlarm(id)`:

1. **Validation**: `id < 1000` is rejected; `m_bInitialized` must be TRUE

2. **Table lookup**: `FindAlarmTable(id, alarm)` searches `m_alarmTable` (CMap loaded from Alarm.xls)
   - If not found, sets `alarm.id = id, alarm.code = id` with defaults

3. **Level dispatch**:
   - `ALARM_LEVLE_HIGH` --> `SetHeavyAlarm()`
   - `ALARM_LEVLE_LOW` --> `SetLightAlarm(pCS, TRUE)`
   - `ALARM_LEVLE_MSG` --> `SetLightAlarm(pCS, FALSE)`

### Heavy Alarm Flow (`SetHeavyAlarm`)

1. **Duplicate check**: `m_pDlgAlarm->FindList(alarm.id)` -- if already displayed, return `ALARM_RV_SUCCESSED`
2. **Timestamp**: Sets `alarm.happendTime`
3. **Report history**: `m_pDlgReportAlarm->AddAlarmHistory(alarm)` -- adds to report list (occurred)
4. **Display**: `m_pDlgAlarm->InsertList(&alarm)` -- adds to active alarm list control
5. **Notify UI**: `PostMessage(m_hAlarmWnd, WM_ALARM_ACCURED, alarm.id, NULL)`
6. **Unlock critical section** (allows other alarms to be raised)
7. **Block**: `WaitForSingleObject(alarm.hEvent, INFINITE)` -- **blocks the calling monitor thread** until user resolves
8. **On resolution**: Records `clearedTime`, removes from list, adds cleared history, posts `WM_ALARM_CLEARED`
9. **Returns**: `alarm.nRet` (the recovery action chosen by user)

### Light Alarm Flow (`SetLightAlarm`)

1. **Duplicate check**: `m_pDlgWarning->FindList(alarm.id)` -- shows window if exists
2. **Timestamps**: Sets both `happendTime` and `clearedTime` immediately
3. **Report**: Adds to report history with `bClear=true` (single-entry, not split)
4. **Display**: `m_pDlgWarning->InsertList(&alarm)` and shows warning dialog
5. **Notify**: Posts `WM_WARNING_ACCURED` if `bLow == TRUE`
6. **Does NOT block** -- returns immediately

## 7. Recovery Procedures

### CDlgAlarmModule -- Recovery Button Panel

The alarm detail dialog (`CDlgAlarmModule`) displays alarm information and provides recovery buttons:

| Button | Return Value | Effect |
|--------|-------------|--------|
| **Init+Retry** | `ALARM_RV_INITRETRY` (3) | Sets event, unblocks monitor thread |
| **Retry** | `ALARM_RV_RETRY` (2) | Sets event, only shown if `alarm.nRetry == 1` |
| **Ignore** | `ALARM_RV_IGNORE` (1) | Sets event, unblocks monitor thread |
| **Stop** | `ALARM_RV_ABORT` (4) | Sets event, unblocks monitor thread |

The recovery mechanism works by:
1. User clicks a recovery button in `CDlgAlarmModule`
2. `m_pAlarm->nRet` is set to the chosen value
3. `SetEvent(m_pAlarm->hEvent)` is called
4. This unblocks the `WaitForSingleObject()` in `CAlarmManager::SetHeavyAlarm()`
5. The alarm is removed from the active list and recorded in history
6. The calling monitor thread receives the return value

### ClearAlarm (Alarm Reset)

`CAlarmController::ClearAlarm()` --> `CAlarmManager::ClearAlarm()`:
- Iterates all active alarms in `m_pDlgAlarm->m_listAlarm`
- Sets `pAlarm->nRet = ALARM_RV_ABORT` for each
- This unblocks all waiting threads
- Destroys the warning dialog if visible

## 8. Emergency Stop Handling

### EMS Button Monitoring (`CAlarmMonitorToEmergencyButton`)

**PLC Mode** (16 buttons):
- 16 EMS switches: OP, LEFT FRONT, RIGHT FRONT, RIGHT BACK, LEFT BACK, INNER (4 positions), SACC (2), E-BOX (2), ENABLE SWITCH, S-BOX (2)
- PLC bit addresses: `PLC_BIT_EMERGENCY_SWITCH01` through `+15`
- Alarm IDs: `PLC_EMO_SWITCH_PUSHED_01` (4551) through `+15`

**IO Board Mode** (4 buttons):
- OP, FRONT, RIGHT, EBOX
- IO addresses: `IO_BOARD_READ_INPUT_012`, `023`, `024`, `025`

**Detection logic**:
- Polls every 200ms
- Debounce: requires 4 consecutive readings (`m_nNGCount > 3`) before triggering
- On trigger: sends UI message to update EMO position state display, logs warning, calls `SetAlarm()`
- When button released (nRet == 0 after alarm was on): sends UI position reset message

### Safety Reset Check

`CAlarmMonitorMgr::IsSafetyResetOn()`:
- Checks MC contact bits (either IO Board or PLC depending on build config)
- PLC mode: checks `PLC_BIT_MC_CONTACT_01` AND `PLC_BIT_MC_CONTACT_02`
- IO Board mode: checks `IO_BOARD_READ_IO_BOARD_READ_INPUT_007` AND `IO_BOARD_READ_INPUT_008`
- If `IsOutOfService()` returns true, always returns true (bypass)

## 9. Alarm UI Display

### CDlgSubAlarm -- Main Alarm Tab

Located in `Source/ALPHA/DlgSubAlarm.cpp`. This is the application-level alarm view that:

1. **Initializes alarm system** in `OnInitDialog()`:
   ```cpp
   CAlarmController::GetInstance()->InitializeAlarm(GetSafeHwnd(), rtROI, alarmFolderPath);
   ```

2. **Handles WM_ALARM_ACCURED** (`OnAlarmOccured`):
   - Logs the alarm text
   - If CIM alarm report is configured and online-remote: skips buzzer
   - If offline/online-local: activates buzzer via `PLC_BIT_EQ_BUZZER`
   - If online-remote: sends alarm code to CIM via `SendAlarmCode(id)`
   - Switches view to alarm page: `WM_MSG_UPDATE_CHANGE_VIEW_STATE` with `enumNavigateID_ALARM`
   - Logs MCC alarm: `CMCCLogMgr::GetInstance()->SetMCC_Alarm(id, TRUE)`

3. **Handles WM_ALARM_CLEARED** (`OnAlarmCleared`):
   - Logs cleared message
   - Deactivates buzzer
   - Switches view back to alarm page
   - Logs MCC clear: `CMCCLogMgr::GetInstance()->SetMCC_Alarm(id, FALSE)`

4. **ShowReportAlarm()**: Toggles between active alarm view and report (history) view based on alarm count

5. **Reload button**: Reloads alarm definitions from `Alarm.xls`

### CDlgAlarm -- Active Alarm List (in DLL)

- `CListCtrl m_listAlarm` with columns: Time, Alarm Id, Module, Alarm Text
- When an alarm is selected (clicked), shows `CDlgAlarmModule` detail panel
- `CDlgAlarmModule` shows: Alarm ID, Type, Module, CMD, Text, Description, and recovery buttons

### CDlgReportAlarm -- Alarm History (in DLL)

- `CListCtrl m_listReportAlarm` with columns: Happened Time, Cleared Time, Module, Alarm ID, Alarm Code, Host Mode, Command, String
- Maximum 500 entries in list (oldest deleted when exceeded)
- Includes search functionality via `CDlgReportAlarmSearch`

### CD_PopUp_Alarm_F -- Fault I/O Popup

- Grid-based popup dialog showing active fault I/O bits
- Uses `CGridCtrl` with columns: No, Description
- Timer-based refresh (1 second interval)
- Shows only active (true) I/O bits with yellow background; hides inactive ones (row height = 0)

## 10. Alarm Logging and Persistence

### File Logging

Each monitor thread has its own log file via `CLogFileAccessor`:

| Monitor Type | Log File Pattern |
|-------------|-----------------|
| Emergency Button | `{LogPath}/Event/emergency_button_{NN}.log` |
| MC OFF State | `{LogPath}/Event/mc_off_state_{NN}.log` |
| Teach Key | `{LogPath}/Event/teach_key.log` |
| Door State | `{LogPath}/Event/Door_state_{NN}.log` |
| Sensor Curtain | `{LogPath}/Event/sensor_cutain.log` |
| Smoke Sensor | `{LogPath}/Event/sensor_Smoke.log` |
| Temperature Sensor | `{LogPath}/Event/sensor_temperature.log` |

Log levels used: `WARN` (most alarms), `ERRO` (temperature over), `SYST` (teach key mode changes), `OPER` (UI operations).

### Database Persistence

`CDlgReportAlarm::AddAlarmHistory()` persists alarm events to Microsoft Access database:

- **File pattern**: `{SavePath}/ALARM/Report_YYYYMM.accdb` (or `.mdb` for older format)
- **Source template**: `Config/DB/ReportSource.accdb` -- copied when monthly file does not exist
- **Connection**: ODBC `Microsoft Access Driver (*.mdb, *.accdb)`
- **Table**: `AlarmHistory`
- **Columns**: `sHappenedTime, sClearedTime, sModule, nAlarmCode, sHostMode, sCommand, sString`
- **Thread safety**: Protected by `g_csQuery` global critical section

### Alarm Definition Loading

Alarm definitions are loaded from `Config/CoreFunction/AlarmConfig/Alarm.xls` at initialization:

1. `CAlarmManager::LoadAlarmFile()` reads the Excel file via `CMySpreadDll`
2. Each row is parsed into an `ALARM_DATA` struct
3. Stored in `CMap<UINT, ALARM_DATA> m_alarmTable` keyed by alarm ID
4. Excel columns: ID (1), Code (2), Level (3: H/L/O), Module (4), Command (5), Text-KR (6), Description-KR (7), Retry (8), Type (9), Text-Use (10: K/E), Text-EN (11), Description-EN (12), Use (13: 1=active)
5. Only rows with `Use == 1` are loaded
6. Row processing stops at `[FILE END]` marker or row 9999

Additional message data loaded from `AlarmMessage.csv`/`AlarmMessage.dat` for `MessageCode2Str()` lookups.

## 11. Interlock System (`CFunctionModuleEQInterlock`)

Located in `Source/ALPHA/FunctionModule/FunctionModuleEQInterlock.cpp/.h`.

The interlock system is **separate from the alarm monitoring system** -- it provides motion safety checks rather than alarm raising. Key characteristics:

- Extends `CBaseSequenceImpl` (sequence engine item)
- Provides interlock check functions for each axis group:
  - `GetInterlockMoveX/Y/Z()` -- Stage XYZ motion interlocks
  - `GetInterlockMoveLDX/Y/R/Z()` -- Loader motion interlocks
  - `GetinterlockIOLP/Clamp/Vacuum/Air()` -- I/O operation interlocks
  - `ChkMotionInterlock(nType)` -- ACS buffer-based interlock check

- Each function checks a combination of safety conditions:
  - `IsStageSafety()` -- Stage gantry safety position
  - `IsHeadSafety()` -- Head safety position
  - `IsLDXLoading/Contact()` -- Loader X position state
  - `IsLDZLoading/Contact()` -- Loader Z position state
  - `IsLPUp/Down()` -- Lift pin state
  - `IsClampFold/Unfold()` -- Clamp state
  - `IsGlassExist()` -- Glass presence

- **Current state**: All safety check functions return `VTM_RV_SUCCESSED` unconditionally (stub implementations). The interlock logic framework is in place but the actual position/sensor checks are not yet implemented.

## 12. CIM Alarm Reporting

When alarms occur while the system is in CIM online-remote mode:

1. `CDlgSubAlarm::OnAlarmOccured()` detects `df_HOST_ONLINE_REMOTE` mode
2. Sends alarm code to CIM host: `m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->GetCommTcpIpObjectCIM()->SendAlarmCode(id)`
3. Buzzer is NOT activated in online-remote mode (host manages equipment state)
4. `CIMAlarmReportInfo()` provides additional alarm context including:
   - `IsUseCIMAlarmNotReport()` -- manual alarm not-report flag
   - `IsUseModelChangeAlarm()` -- model change alarm flag
   - `IsUseCurGlsOneAlarm()` -- current glass one alarm flag
   - Sends `AT0` command to CIM host

## 13. Thread Count Verification

Counting all threads created by the alarm monitoring system (PLC mode):

| Component | Threads |
|-----------|---------|
| CAlarmMonitorToEmergencyButton | 16 |
| CAlarmMonitorToMcOFFState | 10 |
| CAlarmMonitorToSensorCurtain | 1 |
| CAlarmMonitorToSensorVacSwitch | 1 |
| CAlarmMonitorToSensorIsolater | 4 |
| CAlarmMonitorToSensorFAN (EBox) | 8 |
| CAlarmMonitorToSensorFAN (PCBox) | 6 |
| CAlarmMonitorToSensorSmoke | 16 |
| CAlarmMonitorToSensorTemperature | 16 |
| CAlarmMonitorToSensorTemperatureLow | 16 |
| CAlarmMonitorToSensorPressure (Vacuum+CDA+Fixture+Isolator+Shutter+SACC+Probe) | 16 |
| CAlarmMonitorToLampTime | 4 |
| CAlarmMonitorToSafetyPLC | 1 |
| CAlarmMonitorToTeachKEY | 1 |
| CAlarmMonitorToDoorOpenState | 6 |
| CAlarmMonitorToDoorVoice | 1 |
| **Total** | **~123** |

In IO Board mode (`_USE_IOBOARD`), the count drops to approximately 20-25 threads.

## 14. Key Observations for OCP Porting

1. **Thread-per-sensor design**: HMI creates 100+ threads for I/O monitoring. OCP uses a simpler PLC polling model with fewer threads.

2. **Alarm DLL**: The `Alarm.dll` is a reusable CoreFunction library. OCP has its own alarm infrastructure in `CommonFunc.h` (alarm codes 982-4561).

3. **Blocking Heavy Alarms**: `SetHeavyAlarm()` blocks the calling monitor thread using `WaitForSingleObject(hEvent, INFINITE)`. This means the specific I/O point stops being monitored until the user responds. This is by design since the alarm condition persists until resolved.

4. **Alarm.xls**: The alarm definition file is an Excel spreadsheet loaded via FPSpread OCX (`CMySpreadDll`). OCP would need to map its alarm codes to equivalent definitions.

5. **Interlock stubs**: `CFunctionModuleEQInterlock` has the framework but all safety checks return success. This is likely intentionally incomplete or handled elsewhere in the sequence engine.

6. **Conditional compilation**: `_USE_IOBOARD` switches between direct IO board access and PLC-based I/O. The PLC path creates significantly more monitoring threads.

7. **No alarm priority queue**: Alarms are processed in arrival order. Multiple simultaneous alarms each block their own thread independently.
