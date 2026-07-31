# 01. ALPHA HMI Class Architecture

> Generated: 2026-06-26
> Source: `D:\OCP\0.SW_Matching\HMI\Source\ALPHA\`
> Project: Samsung Display Inspection Automation HMI (MFC C++, VS2019, Unicode, x64)

---

## Table of Contents

- [1. Application Layer](#1-application-layer)
  - [1.1 CALPHAApp](#11-calphaapp)
  - [1.2 CDlgALPHA](#12-cdlgalpha)
- [2. Central Orchestrator](#2-central-orchestrator)
  - [2.1 CAuroraMgr](#21-cauroramgr)
  - [2.2 stAuroraMgrPool](#22-stauroramgrpool)
  - [2.3 CAuroraSubMgr](#23-caurorasubmgr)
- [3. Singleton Infrastructure Classes](#3-singleton-infrastructure-classes)
  - [3.1 CSequenceManager](#31-csequencemanager)
  - [3.2 CWindowEventHandler](#32-cwindoweventhandler)
  - [3.3 CSystemParamShare](#33-csystemparamshare)
  - [3.4 CRecipeMgr](#34-crecipemgr)
  - [3.5 CResultMgr](#35-cresultmgr)
  - [3.6 CMCCLogMgr](#36-cmcclogmgr)
  - [3.7 CSACCMgr](#37-csaccmgr)
- [4. Manager Classes (CAuroraSubMgr Derived)](#4-manager-classes-caurorasubmgr-derived)
  - [4.1 CSystemConfigMgr](#41-csystemconfigmgr)
  - [4.2 CNetworkConfigMgr](#42-cnetworkconfigmgr)
  - [4.3 CPositionConfigMgr](#43-cpositionconfigmgr)
  - [4.4 CLogInControlMgr](#44-clogincontrolmgr)
  - [4.5 CIOMapControlMgr](#45-ciomapcontrolmgr)
  - [4.6 CMotorControlMgr](#46-cmotorcontrolmgr)
  - [4.7 CMaintControlMgr](#47-cmaintcontrolmgr)
  - [4.8 CLightControlMgr](#48-clightcontrolmgr)
  - [4.9 CShareMemoryMgr](#49-csharememorymgr)
  - [4.10 CCommTcpIpFactoryMgr](#410-ccommtcpipfactorymgr)
  - [4.11 CCommTcpIPMgr](#411-ccommtcpipmgr)
  - [4.12 CAlarmMonitorMgr](#412-calarmmonitormgr)
  - [4.13 CSeqTactTimeMgr](#413-cseqtacttimemgr)
  - [4.14 CAlignCameraMgr](#414-caligncameramgr)
- [5. Sequence Base Classes](#5-sequence-base-classes)
  - [5.1 CBaseSequenceBody](#51-cbasesequencebody)
  - [5.2 CBaseSequenceImpl](#52-cbasesequenceimpl)
  - [5.3 CDoWorkGlassSequenceImpl](#53-cdoworkglasssequenceimpl)
- [6. Sequence Engine Classes](#6-sequence-engine-classes)
  - [6.1 CDoWorkGlassSequenceEngine](#61-cdoworkglasssequenceengine)
  - [6.2 CMeasureSequenceItemEngine](#62-cmeasuresequenceitemengine)
  - [6.3 CMeasureSequenceItemFactory](#63-cmeasuresequenceitemfactory)
  - [6.4 CMeasureSequenceItemImpl](#64-cmeasuresequenceitemimpl)
- [7. DoWork Sequence Classes](#7-dowork-sequence-classes)
  - [7.1 CSystemInitialize](#71-csysteminitialize)
  - [7.2 CDoWorkGlassSequence](#72-cdoworkglasssequence)
  - [7.3 CDoWorkGlassReady](#73-cdoworkglassready)
  - [7.4 CDoWorkGlassRecipe](#74-cdoworkglassrecipe)
  - [7.5 CDoWorkGlassAdsorption](#75-cdoworkglassadsorption)
  - [7.6 CDoWorkGlassSWAlign](#76-cdoworkglassswalign)
  - [7.7 CDoWorkGlassPreMeas](#77-cdoworkglasspremeas)
  - [7.8 CDoWorkGlassMeasure](#78-cdoworkglassmeasure)
  - [7.9 CDoWorkGlassAGING](#79-cdoworkglassaging)
  - [7.10 CDoWorkGlassResult](#710-cdoworkglassresult)
  - [7.11 CDoWorkGlassUnload](#711-cdoworkglassunload)
  - [7.12 CDoWorkGlassUnloading](#712-cdoworkglassunloading)
  - [7.13 CDoWorkProbeSWAlign](#713-cdoworkprobeswalign)
  - [7.14 CDoWorkSequenceCIM](#714-cdoworksequencecim)
- [8. FunctionModule Classes](#8-functionmodule-classes)
  - [8.1 CFunctionModuleEQState](#81-cfunctionmoduleeqstate)
  - [8.2 CFunctionModuleEQStateChange](#82-cfunctionmoduleeqstatechange)
  - [8.3 CFunctionModuleEQInterlock](#83-cfunctionmoduleeqinterlock)
  - [8.4 CFunctionModuleProbeACC](#84-cfunctionmoduleprobeacc)
  - [8.5 CFunctionModuleAFControl](#85-cfunctionmoduleafcontrol)
  - [8.6 CFunctionModuleCentering](#86-cfunctionmodulecentering)
- [9. Communication Classes](#9-communication-classes)
  - [9.1 CCommEthFunction (Base)](#91-ccommethfunction-base)
  - [9.2 CCommTcpIpObjectUTG](#92-ccommtcpipobjectutg)
  - [9.3 CCommTcpIpObjectVision](#93-ccommtcpipobjectvision)
  - [9.4 CCommTcpIpObjectColor](#94-ccommtcpipobjectcolor)
  - [9.5 CCommTcpIpObjectWAD](#95-ccommtcpipobjectwad)
  - [9.6 CCommTcpIpObjectAngle](#96-ccommtcpipobjectangle)
  - [9.7 CCommTcpIpObjectPG](#97-ccommtcpipobjectpg)
  - [9.8 CCommTcpIpObjectAgingPG](#98-ccommtcpipobjectagingpg)
  - [9.9 CCommTcpIpObjectLCR](#99-ccommtcpipobjectlcr)
  - [9.10 CCommTcpIpObjectTSP](#910-ccommtcpipobjecttsp)
  - [9.11 CCommTcpIpObjectCIM](#911-ccommtcpipobjectcim)
  - [9.12 CCommTcpIpObjectSysTime](#912-ccommtcpipobjectsystime)
  - [9.13 CCommTcpIpObjectMotion](#913-ccommtcpipobjectmotion)
- [10. Alarm Monitor Classes](#10-alarm-monitor-classes)
  - [10.1 CAlarmMonitorImpl (Base)](#101-calarmmonitorimpl-base)
  - [10.2 Alarm Monitor Concrete Classes](#102-alarm-monitor-concrete-classes)
- [11. UI Dialog Classes](#11-ui-dialog-classes)
  - [11.1 CD_AuroraDlgWin](#111-cd_auroradlgwin)
  - [11.2 CMainSubDlgWin](#112-cmainsubdlgwin)
  - [11.3 CDlgMainForm](#113-cdlgmainform)
  - [11.4 CDlgNaviBar](#114-cdlgnavibar)
  - [11.5 CDlgSubOperate](#115-cdlgsuboperate)
  - [11.6 Other Sub Dialogs](#116-other-sub-dialogs)
- [12. Inheritance Hierarchy Summary](#12-inheritance-hierarchy-summary)
- [13. Class Relationship Diagram](#13-class-relationship-diagram)

---

## 1. Application Layer

### 1.1 CALPHAApp

| Item | Detail |
|------|--------|
| **Role** | MFC Application entry point. Creates CAuroraMgr and launches main dialog. Prevents duplicate process execution. |
| **Header** | `Source/ALPHA/ALPHA.h` |
| **Source** | `Source/ALPHA/ALPHA.cpp` |
| **Inheritance** | `CWinApp` -> `CALPHAApp` |
| **Singleton** | Yes (MFC global `theApp`) |
| **Manager** | No |
| **Creation** | Global instance `CALPHAApp theApp;` (`ALPHA.cpp:42`) |
| **Lifecycle** | Process start -> `InitInstance()` creates `CAuroraMgr` -> runs dialog -> process end |

**Key Members:**
- `CAuroraMgr* m_pAuroraMgr` -- central orchestrator, created in `InitInstance()`
- `InitInstance()` -- duplicate process check, creates CAuroraMgr, launches CDlgALPHA

**Key Interface:**
```cpp
virtual BOOL InitInstance();
```

---

### 1.2 CDlgALPHA

| Item | Detail |
|------|--------|
| **Role** | Main dialog window (IDD_ALPHA_DIALOG). Root UI container that receives `CAuroraMgr` pointer. |
| **Header** | `Source/ALPHA/DlgALPHA.h` |
| **Source** | `Source/ALPHA/DlgALPHA.cpp` |
| **Inheritance** | `CDialogEx` -> `CDlgALPHA` |
| **Singleton** | No (single instance via `DoModal()`) |
| **Manager** | No |
| **Creation** | `ALPHA.cpp`, `InitInstance()` -- `CDlgALPHA dlg(m_pAuroraMgr)` |
| **Lifecycle** | Created after CAuroraMgr, lives until user closes application |

**Key Members:**
- `CAuroraMgr* m_pAuroraMgr`

**Key Interface:**
```cpp
CDlgALPHA(CAuroraMgr* pAuroraMgr, CWnd* pParent = nullptr);
virtual BOOL OnInitDialog();
virtual BOOL PreTranslateMessage(MSG* pMsg);
afx_msg void OnClose();
```

---

## 2. Central Orchestrator

### 2.1 CAuroraMgr

| Item | Detail |
|------|--------|
| **Role** | Central orchestrator that owns all manager objects via `stAuroraMgrPool`. Provides accessor methods for all sub-managers. Manages system/login/frame info. |
| **Header** | `Source/ALPHA/AuroraMgr.h` |
| **Source** | `Source/ALPHA/AuroraMgr.cpp` |
| **Inheritance** | None (standalone class) |
| **Singleton** | No (single instance held by `CALPHAApp::m_pAuroraMgr`) |
| **Manager** | Yes -- master manager of all sub-managers |
| **Creation** | `ALPHA.cpp`, `InitInstance()` -- `new CAuroraMgr(strName, strAppPath, strConfigPath, strRecipePath, strLogPath)` |
| **Lifecycle** | Created once at startup in `InitInstance()`, destroyed at application shutdown via `Finalize()` |

**Key Members:**
- `std::vector<CAuroraSubMgr*> m_subMgr` -- all sub-managers as polymorphic list
- `std::map<UINT, CWnd*> m_subDlg` -- registered sub-dialogs
- `stSystemInfo m_systemInfo` -- app name/version
- `stLoginInfo m_loginInfo` -- current user/level
- `stMainFrameInfo m_mainFrameInfo` -- UI layout params
- `stAuroraMgrPool m_AuroraMgrPool` -- all manager pointers

**Manager Creation Order** (in constructor, `AuroraMgr.cpp:75-164`):
1. `CSystemConfigMgr` (line 75)
2. `CLogInControlMgr` (line 107)
3. `CAlignCameraMgr[MAIN_CCD_MAX_NUM]` (lines 113-115)
4. `CNetworkConfigMgr` (line 118)
5. `CPositionConfigMgr` (line 119)
6. `CRecipeEditMgr` (line 122)
7. `CShareMemoryMgr` (line 125)
8. `CLightControlMgr` (line 128)
9. `CIOMapControlMgr` (line 129)
10. `CMotorControlMgr` (line 130)
11. `CMaintControlMgr` (line 131)
12. `CSeqTactTimeMgr` (line 134)
13. `CAlarmMonitorMgr` (line 135)
14. `CCommTcpIpFactoryMgr` (line 138)
15. `CCommTcpIPMgr` (line 140)

**Key Interface:**
```cpp
CAuroraMgr(CString strName, CString strAppPath, CString strConfigPath, CString strRecipePath, CString strLogPath);
void Finalize();
void Notify(UINT, UINT, WPARAM, LPARAM);
void Notify_Direct(UINT, UINT, WPARAM, LPARAM);
void PushSubDlg(int nID, CWnd *pWnd);
void ChangeDlgView(int nID);
// 20+ GetXxxMgrPtr() accessor methods
```

---

### 2.2 stAuroraMgrPool

| Item | Detail |
|------|--------|
| **Role** | Plain data struct holding pointers to ALL manager instances. Passed by reference to sequence/engine classes. Acts as a dependency injection container. |
| **Header** | `Source/ALPHA/AuroraMgrPool.h` |
| **Singleton** | No (struct, value-copied) |

**Contains pointers to 28+ manager types:**
- `CLogInControlMgr`, `CSystemConfigMgr`, `CNetworkConfigMgr`, `CPositionConfigMgr`
- `CLockControlMgr`, `CRecipeEditMgr`, `CResultCollectMgr`, `CIOMapControlMgr`
- `CMotorControlMgr`, `CPatternMatchMgr`, `CLightControlMgr`, `CVCRControlMgr`
- `CCIMControlMgr`, `CAlignMatrixMgr`, `CAlignCameraMgr[MAIN_CCD_MAX_NUM]`
- `CReviewCameraHead1Mgr`, `CReviewCameraHead2Mgr`, `CShareMemoryMgr`, `CMeasureItemMgr`
- `CCommTcpIpFactoryMgr`, `CCommTcpIPMgr`, `CSensorMonitorMgr`, `CAlarmMonitorMgr`
- `CSeqTactTimeMgr`, `CVisionConfigMgr`, `CTriggerControlMgr`, `CSEMControlMgr`, `CMaintControlMgr`

Also holds path strings: `strAppPath`, `strConfigPath`, `strRecipePath`, `strLogPath`.

---

### 2.3 CAuroraSubMgr

| Item | Detail |
|------|--------|
| **Role** | Abstract base class for all pluggable manager components. Defines common lifecycle interface. |
| **Header** | `Source/ALPHA/AuroraSubMgr.h` |
| **Inheritance** | Base class (no parent) |
| **Singleton** | No |
| **Manager** | Yes (base for all managers) |

**Key Interface:**
```cpp
virtual bool Initialize();        // default: false
virtual bool LinkIniFile(bool bSave);  // default: false
virtual bool SetDefault();        // default: false
virtual void Finalize();          // default: empty
CString GetName() const;
void SetName(const CString &val);
```

---

## 3. Singleton Infrastructure Classes

### 3.1 CSequenceManager

| Item | Detail |
|------|--------|
| **Role** | Singleton that owns and orchestrates all sequence/function-module instances. Manages sequence lifecycle: init, start, stop, pause, resume. |
| **Header** | `Source/ALPHA/Manager/SequenceManager.h` |
| **Source** | `Source/ALPHA/Manager/SequenceManager.cpp` |
| **Inheritance** | None (standalone) |
| **Singleton** | **Yes** -- `static CSequenceManager* m_pInstance; static GetInstance(); static DelInstance();` |
| **Manager** | Yes |
| **Creation** | First call to `CSequenceManager::GetInstance()` |
| **Lifecycle** | Created lazily on first access, destroyed via `DelInstance()` |

**Key Members:**
- `CSystemInitialize* m_pSystemInitialize`
- `CDoWorkGlassSequence* m_pDoWorkGlassSequence`
- `CFunctionModuleProbeACC* m_pFunctionModuleProbeACC`
- `CFunctionModuleAutoFocus* m_pFunctionModuleAutoFocus`
- `CFunctionModuleEQStateChange* m_pFunctionModuleEQStateChange`
- `CFunctionModuleEQState* m_pFunctionModuleEQState`
- `CFunctionModuleAFControl* m_pFunctionModuleAFControl`
- `CFunctionModuleCentering* m_pFunctionModuleCentering`
- `CDoWorkSequenceCIM* m_pDoWorkSequenceCIM`
- `std::vector<CBaseSequenceImpl*> m_vecBaseSeqItem`
- `stAuroraMgrPool m_MgrPool_link`

**Key Interface:**
```cpp
bool EngineInitialize();
void EngineFinalize();
void EngineStart();
bool EngineStart(CString strType, bool bCompleteCheck);
void EngineResume();
void EnginePause();
void EngineStop();
void EngineStopAndSkip(CString strType);
void EngineStopAndExit(CString strType);
void EngineCycleInit();
bool IsEngineCompleteAll();
bool IsEngineComplete(int index);
bool LinkDataBase(CBinaryHelper *pBinaryHelper, bool bSave);
void LinkParam(stAuroraMgrPool &MgrPool);
```

---

### 3.2 CWindowEventHandler

| Item | Detail |
|------|--------|
| **Role** | Singleton message dispatcher. Routes WM_USER messages between sequence engine threads and UI windows. |
| **Header** | `Source/ALPHA/WindowEventHandler.h` |
| **Singleton** | **Yes** -- `static CWindowEventHandler* m_pInstance; static GetInstance(); static DelInstance();` |
| **Manager** | No |

**Key Interface:**
```cpp
void SetMessageHandle(enumWndID id, HWND hWnd);
void SetMessageHandle(enumSeqItemID id, HWND hWnd);
LRESULT SendMessageToUI(enumWndID id, UINT idMessage, WPARAM wParam, LPARAM lParam);
BOOL PostMessageToUI(enumWndID id, UINT idMessage, WPARAM wParam, LPARAM lParam);
BOOL PostMessageToUI(enumSeqItemID id, UINT idMessage, WPARAM wParam, LPARAM lParam);
```

**Message IDs defined:** `WM_MSG_UPDATE_LOGIN_STATE` (WM_USER+100) through `WM_MSG_CIM_ALARM`, `WM_MSG_SEQ_ITEM_COMPLTE` (WM_USER+300) series.

---

### 3.3 CSystemParamShare

| Item | Detail |
|------|--------|
| **Role** | Singleton global parameter store. Holds runtime state: folder paths, auto/cycle/test mode flags, glass/lot/recipe IDs. |
| **Header** | `Source/ALPHA/SystemParamShare.h` |
| **Singleton** | **Yes** -- `static CSystemParamShare* m_pInstance; static GetInstance(); static DelInstance();` |
| **Manager** | No |

**Key Interface:**
```cpp
// Path management
CString GetAppFolderPath() / SetAppFolderPath()
CString GetConfigFolderPath() / SetConfigFolderPath()
CString GetLoggerFolderPath() / SetLoggerFolderPath()
CString GetRecipeFolderPath() / SetRecipeFolderPath()
CString GetResultFolderPath() / SetResultFolderPath()
// State management
void SetInitialized(bool) / bool IsInitialized()
void SetAutoMode(bool) / bool IsAutoMode()
void SetCycleState(bool) / bool IsCycleState()
void SetForceStopByUser(bool) / bool IsForceStopByUser()
// Glass/Lot tracking
CString GetGlassID() / SetGlassID(), GetInnerID(), GetLotID(), GetRecipeID(), etc.
long GetCycleCount() / SetCycleCount()
```

---

### 3.4 CRecipeMgr

| Item | Detail |
|------|--------|
| **Role** | Singleton recipe manager. Wraps recipe DLL API (VTM_RECIPE shared memory). |
| **Header** | `Source/ALPHA/Manager/RecipeMgr.h` |
| **Singleton** | **Yes** -- `static CRecipeMgr* m_pInstance; static GetInstance(); static DelInstance();` |
| **Creation** | Lazy singleton; `Initialize()` called from `AuroraMgr.cpp` |

**Key Interface:**
```cpp
void Initialize(HWND hParentWnd, CRect rtROI, CString strAppName, CString strAppPath, CString strConfigPath, CString strLogPath);
CWnd* GetReportmWnd();
VTM_RECIPE* m_pSharedRecipe;
```

---

### 3.5 CResultMgr

| Item | Detail |
|------|--------|
| **Role** | Singleton result data manager. Wraps result DLL API. |
| **Header** | `Source/ALPHA/Manager/ResultMgr.h` |
| **Singleton** | **Yes** -- `static CResultMgr* m_pInstance; static GetInstance(); static DelInstance();` |

**Key Interface:**
```cpp
void Initialize(HWND hParentWnd, CRect rtROI);
void Updata_Data(PRSRITEMDATA result);
bool SaveResult(bool bSaveFile, bool bRemote, CString strStart);
bool SaveResultDone();
void ClearData();
```

---

### 3.6 CMCCLogMgr

| Item | Detail |
|------|--------|
| **Role** | Singleton MCC (Machine Control Communication) log manager. Manages VTM_MCC shared memory, FDC data, action/signal/alarm logging with 500-entry math arrays. |
| **Header** | `Source/ALPHA/Manager/MCCLogMgr.h` |
| **Singleton** | **Yes** -- `static CMCCLogMgr* m_pInstance; static GetInstance(); static DelInstance();` |

**Key Interface:**
```cpp
void Initialize(HWND hParentWnd, CRect rtROI, CString strAppName, ...);
void SetMCC_Action(int iSqNum, BOOL bStart, int nStep, int nFromTo);
void SetMCC_Signal(int iSqNum, BOOL bStart, int nStep, int nFromTo);
void SetMCC_Alarm(int iSqNum, BOOL bStart, int nStep, int nFromTo);
void SetMccMoterInfo(int iAxis, double dPos, double dSpeed);
void UpdateSensor();
bool GetFDCDValue(int iIndex, double *pdValue, double *pdMax, double *pdMin);
```

**Enums defined in this header:** `MCC_PROCESS_ACTION_NO` (256+ values), `MCC_PROCESS_SIGNAL_NO`.

---

### 3.7 CSACCMgr

| Item | Detail |
|------|--------|
| **Role** | Singleton SACC (Semi-Auto Contact Change) probe card management. Tracks probe card RFID, mount/unmount, contact count. |
| **Header** | `Source/ALPHA/Manager/SACCMgr.h` |
| **Singleton** | **Yes** -- `static CSACCMgr* m_pInstance; static GetInstance(); static DelInstance();` |

**Key Interface:**
```cpp
void Initialize(HWND hParentWnd, CRect rtROI, ...);
void Probe_Mount(int iProbe, CString strRFID, CString strMountTime);
void Probe_ContactCount(int iProbe, CString strRFID, CString strMountTime, int nContactCount);
void Probe_Unmount(int iProbe, CString strRFID, CString strMountTime, CString strUnmountTime);
void Probe_GetInfo(int iProbe, CString* strRFID, CString* strMountTime, int* nContactCount);
```

---

## 4. Manager Classes (CAuroraSubMgr Derived)

All classes in this section inherit from `CAuroraSubMgr` and are created/owned by `CAuroraMgr` via `stAuroraMgrPool`.

### 4.1 CSystemConfigMgr

| Item | Detail |
|------|--------|
| **Role** | System configuration manager. Reads/writes INI config for application parameters, result paths, communication type, alarm flags. |
| **Header** | `Source/ALPHA/Manager/SystemConfigMgr.h` |
| **Source** | `Source/ALPHA/Manager/SystemConfigMgr.cpp` |
| **Inheritance** | `CAuroraSubMgr` -> `CSystemConfigMgr` |
| **Singleton** | No (owned by `stAuroraMgrPool`) |
| **Manager** | Yes |
| **Creation** | `AuroraMgr.cpp:75` -- `new CSystemConfigMgr(strConfigPath)` |

**Key Interface:**
```cpp
CString GetConfigFolderPath() / GetRecipePath() / GetLogFilePath() / GetResultFilePath();
bool IsUseAlarmMonitor() / IsUseHardwareSensor() / IsUseHardwareMotor();
bool IsAutoChangeInlineMode();
bool IsCommunicationTypeString();
CString GetUploadResultDataAndImageSaveFolderPath();
virtual bool LinkIniFile(bool bSave) override;
bool GlassExistFile(bool bSave);
```

---

### 4.2 CNetworkConfigMgr

| Item | Detail |
|------|--------|
| **Role** | Network connection configuration. Manages TCP/IP parameters (IP, port, mode, device index) for all external devices. |
| **Header** | `Source/ALPHA/Manager/NetworkConfigMgr.h` |
| **Inheritance** | `CAuroraSubMgr` -> `CNetworkConfigMgr` |
| **Creation** | `AuroraMgr.cpp:118` -- `new CNetworkConfigMgr(strConfigPath)` |

**Key Members:**
- `std::vector<SNetInfo> m_vecParam` -- list of network endpoints
- `float m_fResponceTimeLimit_sec` -- response timeout (default 30s)

**SNetInfo struct:** `bUse`, `bOutOfService`, `strName`, `strIPNumber`, `strPortNumber`, `bConnetMode`, `iDeviceIndex`, `iMeasType`, `strNetDrive`

---

### 4.3 CPositionConfigMgr

| Item | Detail |
|------|--------|
| **Role** | Position configuration for all mechanical axes. Safety positions, loader positions, probe positions, head offsets. |
| **Header** | `Source/ALPHA/Manager/PositionConfigMgr.h` |
| **Inheritance** | `CAuroraSubMgr` -> `CPositionConfigMgr` |
| **Creation** | `AuroraMgr.cpp:119` -- `new CPositionConfigMgr(strConfigPath)` |

**Key stPositionParam fields:**
- `dSafetyStageX/HeadY1/HeadY2/HeadX1/HeadX2/Z/Z2` -- safety limits
- `dLoaderZUp/Middle/Pre/Down` -- lift pin positions
- `dLoadPosition/UnloadPosition/MeasPosition[3]` -- X/Y/R coordinates
- `dProbeSafety[5]` -- safety positions for 5 probe gantries
- `dACPosition[3]/dACZPosition[3]` -- auto changer positions
- `dHeadOffset[2][4]` -- per-head, per-mode offsets (WAD/Vision/Color/Angle)

---

### 4.4 CLogInControlMgr

| Item | Detail |
|------|--------|
| **Role** | Login/authentication manager. Manages user accounts, password validation, login state. |
| **Header** | `Source/ALPHA/Manager/LogInControlMgr.h` |
| **Inheritance** | `CAuroraSubMgr` -> `CLogInControlMgr` |
| **Creation** | `AuroraMgr.cpp:107` -- `new CLogInControlMgr(strConfigPath)` |

**Key Interface:**
```cpp
void SetLoginStatus(int nLevel, bool bLogin);
int SetLoninCheck(int nIdex, CString strId, CString strPassword, int nLevel);
bool GetLoginStatus() const;
int GetLoginLevel() const;
bool SetParam_Add(CString strId, CString strPassword, int nLevel);
```

---

### 4.5 CIOMapControlMgr

| Item | Detail |
|------|--------|
| **Role** | PLC I/O map control. Reads/writes PLC bit/word memory addresses. Drives signal tower, picker, stage vacuum, and interlock signals. |
| **Header** | `Source/ALPHA/Manager/IOMapControlMgr.h` |
| **Source** | `Source/ALPHA/Manager/IOMapControlMgr.cpp` |
| **Inheritance** | `CAuroraSubMgr` -> `CIOMapControlMgr` |
| **Creation** | `AuroraMgr.cpp:129` -- `new CIOMapControlMgr(strAppPath, strConfigPath, strLogPath)` |
| **Wraps** | `CIOFunction` (CoreFunction library) |

**Key Interface:**
```cpp
bool InitDevice(short index);
bool SetMemoryBitToDevice(int index, bool value);
bool SetMemoryWordToDevice(int index, float value);
bool SetMemoryNWordToDevice(CString address, float* value, long lSize);
bool CheckActionDone(int index, DWORD dwTimeout, bool value);
bool SetSinalTowerAlarm() / SetSinalTowerIdle() / SetSinalTowerRun();
bool SetPickertoStage() / SetStagetoPicker();
int GetMoveInterlock();
```

---

### 4.6 CMotorControlMgr

| Item | Detail |
|------|--------|
| **Role** | Motor/motion control via ACS SPiiPlus controller. Wraps `CMotorFunction` for all 90+ axes. Absolute/relative moves, homing, jogging, speed control. |
| **Header** | `Source/ALPHA/Manager/MotorControlMgr.h` |
| **Source** | `Source/ALPHA/Manager/MotorControlMgr.cpp` |
| **Inheritance** | `CAuroraSubMgr` -> `CMotorControlMgr` |
| **Creation** | `AuroraMgr.cpp:130` -- `new CMotorControlMgr(strAppPath, strConfigPath, strLogPath)` |
| **Wraps** | `CMotorFunction` (CoreFunction library) |

**Key Interface:**
```cpp
long MoveHome(int iAxis);
long MoveCmdPosA(int iAxis, double dPos, bool bBlockMode, int iSpeed, int iLens, int iTube);
long MoveCmdPosR(int iAxis, double dPos, bool bBlockMode, int iSpeed);
long MoveCmdPosMulti(stMotorPosition *pPos, int iAxis, int iSpeed, bool bDoneCheck);
bool CheckMoveDone(int iAxis, DWORD dwTimeout);
long SetSpeed(int iAxis, int iSpeedType, int iSpeedMode);
double GetCurPos(int iAxis, int iLens, int iTube);
long GetMoving(int iAxis);
bool GetHomeDone(int iAxis);
BOOL DoWork_MotionEnable(int iAxis);
```

---

### 4.7 CMaintControlMgr

| Item | Detail |
|------|--------|
| **Role** | Maintenance control coordinator. Aggregates motor, IO, position, light, and shared memory managers. Provides higher-level operations: probe handling, clamp control, stage vacuum, lift pin, align offset, SACC integration. |
| **Header** | `Source/ALPHA/Manager/MaintControlMgr.h` |
| **Source** | `Source/ALPHA/Manager/MaintControlMgr.cpp` |
| **Inheritance** | `CAuroraSubMgr` -> `CMaintControlMgr` |
| **Creation** | `AuroraMgr.cpp:131` -- `new CMaintControlMgr(pMotorControlMgr, pIOMapControlMgr, pPositionConfigMgr, pLightControlMgr, pShareMemoryMgr)` |

**Key Interface (150+ methods):**
```cpp
// Motion wrappers
long MoveCmdPosA(int iAxis, double dPos, bool bBlockMode, int iSpeed, ...);
long MoveCmd_SafetyAll(bool bInit);
long MoveCmdAlignPos_Glass(double *pPos);
long MoveCmdAlignPos_Probe(double *pPos);
// Probe operations (for 5 probe gantries)
int Do_PCB_UNIT_Lock/Unlock(int iProbe);
int Do_PROBE_CLAM_Lock/Unlock(int iProbe);
int Do_Zero_Position_Lock/Unlock(int Probe);
long Move_SelectProbe(int iProbe, double dPos, bool bDoneCheck);
long MoveCmdPos_ProbeContact(int nAxis, double fContatZ, double EContatZ, double dOD);
// Clamp/Vacuum/Air
int Do_Clamp_Up/Down/In/Out();
int Do_StageVac_On/Off();
int Do_StageAir_On/Off();
// Lift Pin
long DoWork_LPMove(bool bUp);
// Align
int SetGlassAlignXYR(double dX, double dY, double dR);
int SetProbeAlignXYR(double dX, double dY, double dR, int nProbe);
```

---

### 4.8 CLightControlMgr

| Item | Detail |
|------|--------|
| **Role** | LED light controller. Controls illumination channels via serial protocol through `CLightFunction`. |
| **Header** | `Source/ALPHA/Manager/LightControlMgr.h` |
| **Inheritance** | `CAuroraSubMgr` -> `CLightControlMgr` |
| **Creation** | `AuroraMgr.cpp:128` -- `new CLightControlMgr(strAppPath, strConfigPath, strLogPath)` |
| **Wraps** | `CLightFunction` (CoreFunction library) |

**Key Interface:**
```cpp
bool InitDevice(int port);
bool LedLight_Channel_TurnOn(int port, long nChannel, long nIntensity);
bool LedLight_Channel_TurnOff(int port, long nChannel, long nIntensity);
bool SetLedLampIntensity(int port, long nChannel, long nIntensity, CString &strCommand);
bool SetLedLampTurnOnOff(int port, long nChannel, long nIntensity, enumLED_LAMP_CONTROL nCONTROL, CString &strCommand);
```

---

### 4.9 CShareMemoryMgr

| Item | Detail |
|------|--------|
| **Role** | Shared memory manager wrapping VTM_SYSTEM structure. Provides get/set for system-wide state: alarm, process status, user info, machine mode, glass judge, lamp/temperature overrides. |
| **Header** | `Source/ALPHA/Manager/ShareMemoryMgr.h` |
| **Inheritance** | `CAuroraSubMgr` -> `CShareMemoryMgr` |
| **Creation** | `AuroraMgr.cpp:125` -- `new CShareMemoryMgr(strAppPath, strConfigPath)` |
| **Wraps** | `CShareMemFunction` (CoreFunction library) |

**Key Interface:**
```cpp
VTM_SYSTEM *GetSharedMemory();
void SetSys_ProcessStatus(int nState) / int GetSys_ProcessStatus();
void SetSys_AlarmStatus(int nState) / int GetSys_AlarmStatus();
void SetSys_MachineMode(int nMode) / int GetSys_MachineMode();
void SetSys_Validation(bool bOn) / bool GetSys_Validation();
void SetSys_GlassJudge(bool bOn) / bool GetSys_GlassJudge();
void SetSys_ProbeChange(int nType) / int GetSys_ProbeChange();
```

---

### 4.10 CCommTcpIpFactoryMgr

| Item | Detail |
|------|--------|
| **Role** | Factory-pattern TCP/IP communication manager. Creates/owns all device-specific CommTcpIpObject instances (13+ types). Provides high-level DoWork methods for inspection lifecycle. |
| **Header** | `Source/ALPHA/Manager/CommTcpIpFactoryMgr.h` |
| **Source** | `Source/ALPHA/Manager/CommTcpIpFactoryMgr.cpp` (~26,000 lines) |
| **Inheritance** | `CAuroraSubMgr` -> `CCommTcpIpFactoryMgr` |
| **Creation** | `AuroraMgr.cpp:138` -- `new CCommTcpIpFactoryMgr(m_AuroraMgrPool)` |

**Owned Communication Objects:**
- `CCommTcpIpObjectUTG *m_pCommTcpIpObjUTG1, *m_pCommTcpIpObjUTG2`
- `CCommTcpIpObjectVision* m_pCommTcpIpObj_Vision[2]`
- `CCommTcpIpObjectColor* m_pCommTcpIpObj_Color[2]`
- `CCommTcpIpObjectAngle* m_pCommTcpIpObj_Angle[2]`
- `CCommTcpIpObjectWAD* m_pCommTcpIpObj_WAD`
- `CCommTcpIpObjectPG* m_pCommTcpIpObj_PG`
- `CCommTcpIpObjectAgingPG* m_pCommTcpIpObj_AgingPG`
- `CCommTcpIpObjectLCR* m_pCommTcpIpObj_LCR`
- `CCommTcpIpObjectTSP* m_pCommTcpIpObj_TSP[6]`
- `CCommTcpIpObjectCIM* m_pCommTcpIpObj_CIM`
- `CCommTcpIpObjectSysTime* m_pCommTcpIpObj_SysTime`
- `CCommTcpIpObjectMotion* m_pCommTcpIpObj_VisMotion`

**Key Interface:**
```cpp
long DoWorkSetParameter(long lMeasType);
long DoWorkRecipeOpen(long lMeasType);
long DoWorkInspectStart(long lMeasType, int iMeasMode, int nCellNum, bool bSideOut, bool bWadRHeadUse, bool bWait);
long DoWorkInspectEnd(long lMeasType, int iMeasMode, DWORD dwTimeOut);
long DoWorkInspectComplate(long lMeasType, int iMeasMode, DWORD dwTimeOut, int nRetry);
long DoWorkVisionCmd(int nItem, BOOL bQualityAutoSet, int nHead);
void SetTcpConnection(long nID, bool bConnect);
void DisConnect();
```

**Device ID enum** (`CommTcpIpDefine.h`): `enumTCPIpFactorytID_VISION` through `enumTCPIpFactorytID_CIM` (20 IDs).

---

### 4.11 CCommTcpIPMgr

| Item | Detail |
|------|--------|
| **Role** | TCP/IP connection state manager. Manages `CCommEthFunction` instances for network monitoring. |
| **Header** | `Source/ALPHA/Manager/CommTcpIPMgr.h` |
| **Inheritance** | `CAuroraSubMgr` -> `CCommTcpIPMgr` |
| **Creation** | `AuroraMgr.cpp:140` -- `new CCommTcpIPMgr(m_AuroraMgrPool)` |

**Key Interface:**
```cpp
bool ReConnect() / void Disconnect();
bool ReConnect(int index) / void Disconnect(int index);
bool IsConnected(CString strName);
int GetCommTcpIPCount() const;
CCommEthFunction *GetCommTcpIP(int index);
```

---

### 4.12 CAlarmMonitorMgr

| Item | Detail |
|------|--------|
| **Role** | Alarm monitoring manager. Creates and manages 24+ individual alarm monitor threads, each watching specific I/O conditions. |
| **Header** | `Source/ALPHA/Alarm/AlarmMonitorMgr.h` |
| **Inheritance** | `CAuroraSubMgr` -> `CAlarmMonitorMgr` |
| **Creation** | `AuroraMgr.cpp:135` -- `new CAlarmMonitorMgr(m_AuroraMgrPool)` |

**Key Interface:**
```cpp
void SetMonitoringStart();
void SetMonitoringStop();
bool IsSafetyResetOn();
```

**Owns:** `std::vector<CAlarmMonitorImpl*> m_vecMonitor` (24+ monitors)

---

### 4.13 CSeqTactTimeMgr

| Item | Detail |
|------|--------|
| **Role** | Sequence tact time measurement and logging. Tracks cycle/step timing for performance analysis. |
| **Header** | `Source/ALPHA/Manager/SeqTactTimeMgr.h` |
| **Inheritance** | `CAuroraSubMgr` -> `CSeqTactTimeMgr` |
| **Creation** | `AuroraMgr.cpp:134` -- `new CSeqTactTimeMgr(strLogPath)` |

**Key Interface:**
```cpp
void SetTotalSeqTimeST() / SetTotalSeqTimeED();
void SetCycleTimeST(int cycle_index, CString strGlassID, CString strRecipeID);
void SetCycleTimeED(int cycle_index);
void SetSeqItemTimeST(int cycle_index, CString strSeqName, int sitem_index);
void SetSeqItemTimeED(int cycle_index, CString strSeqName, int sitem_index);
```

---

### 4.14 CAlignCameraMgr

| Item | Detail |
|------|--------|
| **Role** | Alignment camera control. Wraps NI frame grabber for image acquisition, pattern matching, and angle calculation. |
| **Header** | `Source/ALPHA/Manager/AlignCameraMgr.h` |
| **Inheritance** | `CAuroraSubMgr` -> `CAlignCameraMgr` |
| **Creation** | `AuroraMgr.cpp:113-115` -- `new CAlignCameraMgr(strConfigPath, strRecipePath, strResultPath, i)` (array of `MAIN_CCD_MAX_NUM`) |
| **Wraps** | `CFrameGrabberFunction` (CoreFunction library), `CIMAQImageCtrl` |

**Key Interface:**
```cpp
bool InitDevice();
bool SnapImage(short* pbyImage);
bool GetSnapImage(long nExpTime_ms, BYTE *pbyImage);
int PtrnMatch(CPtrnInfo* pPtrnInfo, float* pfScore, int nMeasLens, bool bGlass);
int SnapImage(Image* pImage, bool bDisplay);
int SaveImage(Image* pImage, int iType);
float CalcAngle(COORDINATE stBase1, COORDINATE stBase2, COORDINATE stDetect, COORDINATE stDetect2);
```

---

## 5. Sequence Base Classes

### 5.1 CBaseSequenceBody

| Item | Detail |
|------|--------|
| **Role** | Base class for all sequence bodies. Provides step index state machine, manager pool access, pause/resume events, fail/alarm/complete flags. |
| **Header** | `Source/ALPHA/BaseSequenceBody.h` |
| **Source** | `Source/ALPHA/BaseSequenceBody.cpp` |
| **Inheritance** | Base class (no parent) |

**Key Members:**
- `stAuroraMgrPool m_MgrPool_link` -- access to all managers
- `CLogFileAccessor *m_pLogFileAccessor` -- per-sequence log
- `CCriticalSection m_cs` -- thread safety
- Step control: `m_iStepIndex`, `m_iStepIndex_loop`, `m_iStepIndex_exit`, `m_iStepIndex_idle`, `m_iStepIndex_total`
- State flags: `m_bComplete`, `m_bFail`, `m_bAlarmOn`, `m_bStopUser`
- Events: `m_hEvtUserPause`, `m_hEvtUserResume`

**Key Interface:**
```cpp
void GoToInitStep() / GoToNextStep() / GoToBackStep() / GoToLoopStep() / GoToExitStep() / GoToIDLEStep();
long GetCurStep() const;
bool IfPauseEvt_WaitResumeEvt() const;
// 15+ GetXxxMgrPtr() accessors for manager pool
```

---

### 5.2 CBaseSequenceImpl

| Item | Detail |
|------|--------|
| **Role** | Abstract sequence implementation. Extends CBaseSequenceBody with thread management and pure virtual interface for all sequences. |
| **Header** | `Source/ALPHA/BaseSequenceImpl.h` |
| **Source** | `Source/ALPHA/BaseSequenceImpl.cpp` |
| **Inheritance** | `CBaseSequenceBody` -> `CBaseSequenceImpl` |

**Key Members:**
- `CWinThread *m_pThread` -- worker thread
- `bool m_bThreadLive`, `m_bIsThreadRun`
- Events: `m_hStartEvent`, `m_hCloseEvent`, `m_hMultiEvent[2]`, `m_hWaitTimeEvent`, `m_hCompleteEvent`

**Pure Virtual Interface:**
```cpp
virtual bool CreateDoWorkSeq() = 0;
virtual void DeleteDoWorkSeq() = 0;
virtual bool ChangeState() = 0;
virtual bool UpdateState() = 0;
virtual bool LinkDataBase(CBinaryHelper *pBinaryHelper, bool bSave) = 0;
```

**Concrete Methods:**
```cpp
virtual void ThreadRun();
virtual void Start() / Stop() / Skip() / Exit() / Pause() / Resume();
virtual bool Is_Complete() / Is_Fail() / Is_AlarmOn() / Is_ThreadRun();
static UINT ThreadRun(LPVOID p1);  // thread entry
```

---

### 5.3 CDoWorkGlassSequenceImpl

| Item | Detail |
|------|--------|
| **Role** | Intermediate abstract class for glass-processing sequence steps. Serves as common base for all DoWorkGlass* classes. |
| **Header** | `Source/ALPHA/DoWork/DoWorkGlassSequenceImpl.h` |
| **Inheritance** | `CBaseSequenceBody` -> `CBaseSequenceImpl` -> `CDoWorkGlassSequenceImpl` |

```cpp
CDoWorkGlassSequenceImpl(stAuroraMgrPool &MgrPool, int iSeqItemNumber);
```

---

## 6. Sequence Engine Classes

### 6.1 CDoWorkGlassSequenceEngine

| Item | Detail |
|------|--------|
| **Role** | Glass processing sequence engine. Manages ordered list of `CDoWorkGlassSequenceImpl` items and runs them sequentially in a dedicated thread. Owns the main glass-processing cycle. |
| **Header** | `Source/ALPHA/DoWork/DoWorkGlassSequenceEngine.h` |
| **Source** | `Source/ALPHA/DoWork/DoWorkGlassSequenceEngine.cpp` |
| **Inheritance** | None (standalone engine) |
| **Creation** | Created by `CDoWorkGlassSequence::CreateDoWorkSeq()` |

**Key Members:**
- `std::vector<CDoWorkGlassSequenceImpl*> m_vecItems` -- ordered sequence items
- `CWinThread *m_pThread`, events for thread lifecycle
- `stAuroraMgrPool m_MgrPool_link`

**Key Interface:**
```cpp
bool ItemInitialize() / void ItemFinalize();
bool ItemStart(CString strType, bool bCompleteCheck);
void ItemStopAndSkip(CString strType) / ItemStopAndExit(CString strType);
void EngineStart() / EngineResume() / EnginePause() / EngineStop();
bool Is_CompleteAll() / Is_Complete(int index);
void AddItem(CString strType, CString strName) / DelItem(int index);
bool ThreadRu_nSequence(int cycle_index, bool bFirst);
```

---

### 6.2 CMeasureSequenceItemEngine

| Item | Detail |
|------|--------|
| **Role** | Measurement sub-sequence engine. Runs within a glass processing step to execute ordered measurement items (Vision, Color, Angle, WAD, etc.) in a thread. |
| **Header** | `Source/ALPHA/DoWork/MeasureSequenceItemEngine.h` |
| **Source** | `Source/ALPHA/DoWork/MeasureSequenceItemEngine.cpp` |
| **Inheritance** | None (standalone engine) |
| **Creation** | Created by `CDoWorkGlassMeasure`, `CDoWorkGlassPreMeas`, `CDoWorkGlassAGING` |

**Key Members:**
- `CMeasureSequenceItemFactory *m_pItemFactory`
- `std::vector<CMeasureSequenceItemImpl*> m_vecItems`
- `int m_nMeasureArea`, `int m_nHead`

---

### 6.3 CMeasureSequenceItemFactory

| Item | Detail |
|------|--------|
| **Role** | Factory class for creating measurement sequence item instances based on type string. |
| **Header** | `Source/ALPHA/DoWork/MeasureSequenceItemFactory.h` |
| **Source** | `Source/ALPHA/DoWork/MeasureSequenceItemFactory.cpp` |

**Key Interface:**
```cpp
std::vector<CString> GetItemType();
std::vector<CString> GetItemDesc();
CMeasureSequenceItemImpl *GetItem(CString strType, CString strName);
```

---

### 6.4 CMeasureSequenceItemImpl

| Item | Detail |
|------|--------|
| **Role** | Abstract base for individual measurement items. Defines the measurement lifecycle: AutoFocus -> Centering -> GetImage -> Measure -> MeasureEnd -> Complete. |
| **Header** | `Source/ALPHA/DoWork/MeasureSequenceItemImpl.h` |
| **Source** | `Source/ALPHA/DoWork/MeasureSequenceItemImpl.cpp` |
| **Inheritance** | `CBaseSequenceBody` -> `CMeasureSequenceItemImpl` |

**Key Members:**
- `CRecipeData *m_pRecipeDataClone`
- `CCommEthFunction *m_pCommTcpIpObject`
- Own thread: `CWinThread *m_pThread`, events

**Pure Virtual Interface:**
```cpp
virtual bool CreateSeqItem() = 0;
virtual bool LinkDataBase(CBinaryHelper*, bool) = 0;
virtual bool UpdateState() = 0;
virtual void ThreadRun() = 0;
virtual bool Run_AutoFocus() = 0;
virtual bool Run_Centering() = 0;
virtual bool Run_GetImage() = 0;
virtual bool Run_Measure() = 0;
virtual bool Run_MeasureEnd() = 0;
virtual bool Run_Complete() = 0;
```

---

## 7. DoWork Sequence Classes

All inherit from `CDoWorkGlassSequenceImpl` (except CDoWorkSequenceCIM which inherits from `CBaseSequenceImpl` directly). Each has a corresponding `*Step` class for per-step logic.

### 7.1 CSystemInitialize

| Item | Detail |
|------|--------|
| **Role** | 22-step system initialization sequence. Hardware init, homing, safety checks. |
| **Header** | `Source/ALPHA/DoWork/SystemInitialize.h` |
| **Inheritance** | `CBaseSequenceImpl` -> `CSystemInitialize` |
| **Step class** | `CSystemInitializeStep` |
| **Creation** | `CSequenceManager` constructor |

---

### 7.2 CDoWorkGlassSequence

| Item | Detail |
|------|--------|
| **Role** | Top-level glass processing sequence. Contains `CDoWorkGlassSequenceEngine` that manages the 9-step glass cycle. |
| **Header** | `Source/ALPHA/DoWork/DoWorkGlassSequence.h` |
| **Inheritance** | `CBaseSequenceImpl` -> `CDoWorkGlassSequence` |
| **Creation** | `CSequenceManager` constructor |

**Key Members:**
- `CDoWorkGlassSequenceEngine *m_pSeqEngine` -- owns and runs sub-sequences

---

### 7.3 CDoWorkGlassReady

| Item | Detail |
|------|--------|
| **Role** | Glass ready (load preparation) sequence. |
| **Header** | `Source/ALPHA/DoWork/DoWorkGlassReady.h` |
| **Inheritance** | `CDoWorkGlassSequenceImpl` -> `CDoWorkGlassReady` |
| **Step class** | `CDoWorkGlassReadyStep` |

### 7.4 CDoWorkGlassRecipe

| Item | Detail |
|------|--------|
| **Role** | Recipe change sequence during glass processing. |
| **Header** | `Source/ALPHA/DoWork/DoWorkGlassRecipe.h` |
| **Inheritance** | `CDoWorkGlassSequenceImpl` -> `CDoWorkGlassRecipe` |
| **Step class** | `CDoWorkGlassRecipeStep` |

### 7.5 CDoWorkGlassAdsorption

| Item | Detail |
|------|--------|
| **Role** | Glass adsorption (vacuum chuck) sequence. |
| **Header** | `Source/ALPHA/DoWork/DoWorkGlassAdsorption.h` |
| **Inheritance** | `CDoWorkGlassSequenceImpl` -> `CDoWorkGlassAdsorption` |
| **Step class** | `CDoWorkGlassAdsorptionStep` |

### 7.6 CDoWorkGlassSWAlign

| Item | Detail |
|------|--------|
| **Role** | Software alignment (pattern matching) sequence for glass. |
| **Header** | `Source/ALPHA/DoWork/DoWorkGlassSWAlign.h` |
| **Inheritance** | `CDoWorkGlassSequenceImpl` -> `CDoWorkGlassSWAlign` |
| **Step class** | `CDoWorkGlassSWAlignStep` |

### 7.7 CDoWorkGlassPreMeas

| Item | Detail |
|------|--------|
| **Role** | Pre-measurement sequence (pre-contact, PTP test, OST test). Uses `CMeasureSequenceItemEngine`. |
| **Header** | `Source/ALPHA/DoWork/DoWorkGlassPreMeas.h` |
| **Inheritance** | `CDoWorkGlassSequenceImpl` -> `CDoWorkGlassPreMeas` |
| **Step class** | `CDoWorkGlassPreMeasStep` |
| **Contains** | `CMeasureSequenceItemEngine* m_pMeasSeqItemEngine` |

### 7.8 CDoWorkGlassMeasure

| Item | Detail |
|------|--------|
| **Role** | Main measurement sequence. Drives measurement instruments through `CMeasureSequenceItemEngine`. |
| **Header** | `Source/ALPHA/DoWork/DoWorkGlassMeasure.h` |
| **Inheritance** | `CDoWorkGlassSequenceImpl` -> `CDoWorkGlassMeasure` |
| **Step class** | `CDoWorkGlassMeasureStep` |
| **Contains** | `CMeasureSequenceItemEngine *m_pMeasSeqItemEngine` |

### 7.9 CDoWorkGlassAGING

| Item | Detail |
|------|--------|
| **Role** | AGING test sequence (burn-in with PG). Uses `CMeasureSequenceItemEngine`. P1-P4 probes for AGING. |
| **Header** | `Source/ALPHA/DoWork/DoWorkGlassAGING.h` |
| **Inheritance** | `CDoWorkGlassSequenceImpl` -> `CDoWorkGlassAGING` |
| **Step class** | `CDoWorkGlassAGINGStep` |
| **Contains** | `CMeasureSequenceItemEngine* m_pMeasSeqItemEngine` |

### 7.10 CDoWorkGlassResult

| Item | Detail |
|------|--------|
| **Role** | Result collection and saving sequence. |
| **Header** | `Source/ALPHA/DoWork/DoWorkGlassResult.h` |
| **Inheritance** | `CDoWorkGlassSequenceImpl` -> `CDoWorkGlassResult` |
| **Step class** | `CDoWorkGlassResultStep` |

### 7.11 CDoWorkGlassUnload

| Item | Detail |
|------|--------|
| **Role** | Glass unload preparation sequence. |
| **Header** | `Source/ALPHA/DoWork/DoWorkGlassUnload.h` |
| **Inheritance** | `CDoWorkGlassSequenceImpl` -> `CDoWorkGlassUnload` |
| **Step class** | `CDoWorkGlassUnloadStep` |

### 7.12 CDoWorkGlassUnloading

| Item | Detail |
|------|--------|
| **Role** | Physical glass unloading (lift pin, loader arm) sequence. |
| **Header** | `Source/ALPHA/DoWork/DoWorkGlassUnloading.h` |
| **Inheritance** | `CDoWorkGlassSequenceImpl` -> `CDoWorkGlassUnloading` |
| **Step class** | `CDoWorkGlassUnloadingStep` |

### 7.13 CDoWorkProbeSWAlign

| Item | Detail |
|------|--------|
| **Role** | Probe software alignment sequence. |
| **Header** | `Source/ALPHA/DoWork/DoWorkProbeSWAlign.h` |
| **Inheritance** | `CDoWorkGlassSequenceImpl` -> `CDoWorkProbeSWAlign` |
| **Step class** | `CDoWorkProbeSWAlignStep` |

### 7.14 CDoWorkSequenceCIM

| Item | Detail |
|------|--------|
| **Role** | CIM (Computer Integrated Manufacturing) communication sequence. Multi-phase state machine: INIT -> READY -> LOAD_READY -> LOAD -> MEAS -> MEAS_DONE -> DATA_UPLOAD -> UNLOAD_READY -> UNLOAD. |
| **Header** | `Source/ALPHA/DoWork/DoWorkSequenceCIM.h` |
| **Inheritance** | `CBaseSequenceImpl` -> `CDoWorkSequenceCIM` (NOT via CDoWorkGlassSequenceImpl) |
| **Step class** | `CDoWorkSequenceCIMStep` |

**Key Members:**
- 9 separate step vectors for each CIM state phase
- `long m_lCIMStatusStepStart/End[enumCIMStatus_CHANGE_COUNT]`
- `bool m_bThreadRun`, `long m_lCurrentStep`

**CIM Status enum:** `enumCIMStatus_INIT`, `READY`, `LOAD_READY`, `LOAD`, `MEAS`, `MEAS_DONE`, `DATA_UPLOAD`, `UNLOAD_READY`, `UNLOAD`

---

## 8. FunctionModule Classes

All inherit from `CBaseSequenceImpl` and run as independent threads managed by `CSequenceManager`.

### 8.1 CFunctionModuleEQState

| Item | Detail |
|------|--------|
| **Role** | Equipment state monitoring. Reads PLC sensor inputs: safety, glass exist, lift pin, vacuum, air, clamp, loader position, door, shutter, probe contact/exist/zero-lock, SACC buffer position. Also provides interlock queries and EQ command setters. |
| **Header** | `Source/ALPHA/FunctionModule/FunctionModuleEQState.h` |
| **Source** | `Source/ALPHA/FunctionModule/FunctionModuleEQState.cpp` |
| **Inheritance** | `CBaseSequenceImpl` -> `CFunctionModuleEQState` |
| **Creation** | `CSequenceManager` constructor |

**State enum:** `enumEQStateReady`, `enumEQStateLoad`, `enumEQStateMeas`, `enumEQStateAbnormal`

**Key Interface (80+ methods):**
```cpp
// State queries
int GetEQState();
bool IsEQStateReady() / IsEQStateMeas() / IsEQStateLoad();
// Sensor queries
bool IsSafetySensorOn() / IsGlassSensorOn() / IsLiftpinSensorUp() / IsVacuumSensorOn();
bool IsProbeExistSensorOn(int) / IsProbeZerolockSensorOn(int) / IsProbeClampFold(int);
// EQ commands
bool SetEQLoad(int) / SetEQUnLoad(int) / SetEQExchange(int, int);
// Interlock
int GetInterlockMoveX() / GetInterlockMoveY() / GetInterlockMoveZ();
int ChkMotionInterlock(int nType);
```

---

### 8.2 CFunctionModuleEQStateChange

| Item | Detail |
|------|--------|
| **Role** | Equipment state transition engine. Manages 7 transition types: READY-to-LOAD, LOAD-to-MEAS, MEAS-to-READY, etc. Each transition has its own step sequence vector. |
| **Header** | `Source/ALPHA/FunctionModule/FunctionModuleEQStateChange.h` |
| **Source** | `Source/ALPHA/FunctionModule/FunctionModuleEQStateChange.cpp` |
| **Inheritance** | `CBaseSequenceImpl` -> `CFunctionModuleEQStateChange` |
| **Step class** | `CFunctionModuleEQStateChangeStep` |

**Transition enum:** `enumEQSTATUS_R2L` (Ready->Load), `L2M` (Load->Meas), `M2R` (Meas->Ready), `R2M`, `M2L`, `L2R`, `SAFETY`

**Key Members:**
- 7 step vectors: `m_vecSeqStepR2L`, `L2R`, `R2M`, `M2L`, `L2M`, `M2R`, `Safety`
- `long m_lCurrentStep`, `m_lEQStatusStepStart/End[7]`

---

### 8.3 CFunctionModuleEQInterlock

| Item | Detail |
|------|--------|
| **Role** | Safety interlock module. Checks motion axis interlocks, I/O conditions (lift pin, clamp, vacuum, air), glass existence, stage/head safety. |
| **Header** | `Source/ALPHA/FunctionModule/FunctionModuleEQInterlock.h` |
| **Source** | `Source/ALPHA/FunctionModule/FunctionModuleEQInterlock.cpp` |
| **Inheritance** | `CBaseSequenceImpl` -> `CFunctionModuleEQInterlock` |

**Key Interface:**
```cpp
int GetInterlockMoveX() / GetInterlockMoveY() / GetInterlockMoveZ();
int GetInterlockMoveLDX() / LDY() / LDR() / LDZ();
int GetinterlockIOLP() / IOClamp() / IOVacuum() / IOAir();
int ChkMotionInterlock(int nType);
```

---

### 8.4 CFunctionModuleProbeACC

| Item | Detail |
|------|--------|
| **Role** | Probe Auto Contact Change module. Multi-phase state machine for probe loading/unloading/exchange via SACC system. Manages 5 probe gantries, cassette selection, alignment. 20+ step vector groups for different operations. |
| **Header** | `Source/ALPHA/FunctionModule/FunctionModuleProbeACC.h` |
| **Source** | `Source/ALPHA/FunctionModule/FunctionModuleProbeACC.cpp` |
| **Inheritance** | `CBaseSequenceImpl` -> `CFunctionModuleProbeACC` |
| **Step class** | `CFunctionModuleProbeACCStep` |

**PACC Status enum (30+ states):** `GETEQPROBE`, `PUTEQPROBE`, `LODING`, `UNLODING`, `EXCHANGE`, `ALIGNPROBE`, `READY_POS`, `MEASURE_POS`, `ZERO_LOCK_ON/OFF`, `PROBE_CLAMP_ON/OFF`, `PROBE_PCB_ON/OFF`, `AC_Y_CHANGEPOS/SAFETYPOS/SACCPOS`, `AC_Z_UP/DOWN`, `SLOT_GET/PUT`, `PACC_GET/PUT/DONE`

**Key Members:**
- `int m_iSelectProbe` (0-4), `m_iSelectLoadCassette`, `m_iSelectUnLoadCassette`
- `bool m_bProbeAligned[5]`
- `stAuroraMgrPool* m_pAuroraMgrPool`

---

### 8.5 CFunctionModuleAFControl

| Item | Detail |
|------|--------|
| **Role** | Auto-focus control module. Wraps KMAC AF controller for Z-axis focusing using image sharpness analysis. |
| **Header** | `Source/ALPHA/FunctionModule/FunctionModuleAFControl.h` |
| **Inheritance** | `CBaseSequenceImpl` -> `CFunctionModuleAFControl` |
| **Creation** | `CSequenceManager` -- `new CFunctionModuleAFControl(MgrPool, pAutoFocusController)` |

**AF Parameter struct (`stKMACAFModuleParameter`):**
- `dMulti`, `bFitPeak`, `nStep`, `dStepSize`, `dStroke`, `dSpeed`, `nChannel`

**Key Interface:**
```cpp
bool InitAFModule(int nIndex);
int HomingAFModule(int nIndex);
int StartAF(int nIndex) / StartAFKMACImage(int) / StartAFOther(int, bool);
int CheckAFDone(int nIndex);
void SetAFPosition(double) / double GetAFPosition();
int MovePosition(int nIndex, double dPostion);
```

---

### 8.6 CFunctionModuleCentering

| Item | Detail |
|------|--------|
| **Role** | Centering (position correction) module. |
| **Header** | `Source/ALPHA/FunctionModule/FunctionModuleCentering.h` |
| **Inheritance** | `CBaseSequenceImpl` -> `CFunctionModuleCentering` |
| **Step class** | `CFunctionModuleCenteringStep` |

**Key Members:**
- `int m_nAlignType` -- alignment algorithm type
- `int m_nHead` -- target head (0 or 1)

---

## 9. Communication Classes

### 9.1 CCommEthFunction (Base)

| Item | Detail |
|------|--------|
| **Role** | Base class for all TCP/IP device communication. Wraps `CEthernetManagerTCP` for connection management. Provides connect/disconnect, send/receive, message parsing. Thread-based auto-reconnect. |
| **Header** | `CoreFunction/CommEthFunction/CommEthFunction.h` |
| **Inheritance** | Base class for all CommTcpIpObject* classes |

**Key Members:**
- `CEthernetManagerTCP *m_pEthernet`
- `CWinThread *m_pThread` -- auto-reconnect thread
- `CallbackClassFunction2 m_CallbackFunc` -- receive callback

**Key Interface:**
```cpp
bool Initialize(bool bDefault = true);
void Finalize();
bool IsConnected() const;
void SetConnect(bool bStart);
bool SendCommand(CString strMsg, bool bUseTail, bool bUseLog);
void SetNetInfo(bool bRunMode, CString strIP, long nPort);
// Protected virtuals for subclass override:
virtual void xNetCallbackRecieveData(LPVOID);
virtual bool SendPacket(CString strMsg, ...);
virtual long ParseMsg(CString strMsg);
virtual std::vector<CString> GetCommand(CString &strData);
```

---

### 9.2 CCommTcpIpObjectUTG

| Item | Detail |
|------|--------|
| **Role** | UTG (Ultrasonic Thickness Gauge) communication object. |
| **Header** | `Source/ALPHA/Manager/CommTcpIpObjectUTG.h` |
| **Inheritance** | `CCommEthFunction` -> `CCommTcpIpObjectUTG` |
| **Creation** | `CCommTcpIpFactoryMgr::CreateComm()` |
| **Instances** | 2 (`m_pCommTcpIpObjUTG1`, `m_pCommTcpIpObjUTG2`) |

### 9.3 CCommTcpIpObjectVision

| Item | Detail |
|------|--------|
| **Role** | Vision inspection (screen quality) communication. Image grab, recipe change, result file management. Quality wait queue for async inspection. |
| **Header** | `Source/ALPHA/CommTcpIpObjectVision.h` |
| **Inheritance** | `CCommEthFunction` -> `CCommTcpIpObjectVision` |
| **Instances** | 2 (array `m_pCommTcpIpObj_Vision[2]`) |

**Key Interface:** `RecipeChange()`, `InspectStart()`, `InspectEnd()`, `WaitInspectionComplete()`, `QualityWaitSet()`, `QualityBufferEmpty()`, `TossResultFile()`

### 9.4 CCommTcpIpObjectColor

| Item | Detail |
|------|--------|
| **Role** | Colorimeter instrument communication. |
| **Header** | `Source/ALPHA/CommTcpIpObjectColor.h` |
| **Inheritance** | `CCommEthFunction` -> `CCommTcpIpObjectColor` |
| **Instances** | 2 (array `m_pCommTcpIpObj_Color[2]`) |

### 9.5 CCommTcpIpObjectWAD

| Item | Detail |
|------|--------|
| **Role** | WAD (Wide Angle Display) instrument communication. Uses `ELPPROTOCOL` struct. Supports right-head measurement mode. |
| **Header** | `Source/ALPHA/CommTcpIpObjectWAD.h` |
| **Inheritance** | `CCommEthFunction` -> `CCommTcpIpObjectWAD` |
| **Instances** | 1 |

### 9.6 CCommTcpIpObjectAngle

| Item | Detail |
|------|--------|
| **Role** | Viewing angle measurement instrument communication. |
| **Header** | `Source/ALPHA/CommTcpIpObjectAngle.h` |
| **Inheritance** | `CCommEthFunction` -> `CCommTcpIpObjectAngle` |
| **Instances** | 2 (array `m_pCommTcpIpObj_Angle[2]`) |

### 9.7 CCommTcpIpObjectPG

| Item | Detail |
|------|--------|
| **Role** | Pattern Generator communication. Controls display test patterns (PTP, OST, R-TEST, AGING), cell selection. Uses `PGPROTOCOL` struct for SECS-like stream/function commands. |
| **Header** | `Source/ALPHA/CommTcpIpObjectPG.h` |
| **Inheritance** | `CCommEthFunction` -> `CCommTcpIpObjectPG` |
| **Instances** | 1 |

### 9.8 CCommTcpIpObjectAgingPG

| Item | Detail |
|------|--------|
| **Role** | Aging-specific PG communication. Separate protocol (`AGINGPGPROTOCOL`) with start/status/judge/data/stop commands. Stores results per PG x Sequence matrix (`m_stAgingResult[10][30]`). |
| **Header** | `Source/ALPHA/CommTcpIpObjectAgingPG.h` |
| **Inheritance** | `CCommEthFunction` -> `CCommTcpIpObjectAgingPG` |
| **Instances** | 1 |

### 9.9 CCommTcpIpObjectLCR

| Item | Detail |
|------|--------|
| **Role** | LCR meter communication (electrical measurement). |
| **Header** | `Source/ALPHA/CommTcpIpObjectLCR.h` |
| **Inheritance** | `CCommEthFunction` -> `CCommTcpIpObjectLCR` |
| **Instances** | 1 |

### 9.10 CCommTcpIpObjectTSP

| Item | Detail |
|------|--------|
| **Role** | TSP (Touch Screen Panel) test communication. |
| **Header** | `Source/ALPHA/CommTcpIpObjectTSP.h` |
| **Inheritance** | `CCommEthFunction` -> `CCommTcpIpObjectTSP` |
| **Instances** | 6 (array `m_pCommTcpIpObj_TSP[6]`) |

### 9.11 CCommTcpIpObjectCIM

| Item | Detail |
|------|--------|
| **Role** | CIM (MES/Host) communication. Implements handshake protocol (HandOn/HandOff), PPID/LOT/GLS ID exchange, status reporting, alarm forwarding. FTP result upload thread. |
| **Header** | `Source/ALPHA/CommTcpIpObjectCIM.h` |
| **Inheritance** | `CCommEthFunction` -> `CCommTcpIpObjectCIM` |
| **Instances** | 1 |

**Key Interface (50+ methods):**
```cpp
int ProcessHandShake(CString) / ProcessHandOff() / ProcessHandOn();
int Process_LOTID() / Process_GLSID() / Process_PPID();
int ProcessTo_LDREQ() / LDCPL() / ULDREQ() / ULDCPL() / MeasStart() / MeasEnd();
int SendAlarmCode(int) / SendAlarmClear(int);
bool SendRecipeListToFTP() / SendAPDFileToFTP();
```

### 9.12 CCommTcpIpObjectSysTime

| Item | Detail |
|------|--------|
| **Role** | System time synchronization communication. |
| **Header** | `Source/ALPHA/Manager/CCommTcpIpObjectSysTime.h` |
| **Inheritance** | `CCommEthFunction` -> `CCommTcpIpObjectSysTime` |
| **Instances** | 1 |

### 9.13 CCommTcpIpObjectMotion

| Item | Detail |
|------|--------|
| **Role** | Vision motion controller communication (filter wheel, external motion device). Uses `VisMotionPROTOCOL` struct. |
| **Header** | `Source/ALPHA/Manager/CCommTcpIpObjectMotion.h` |
| **Inheritance** | `CCommEthFunction` -> `CCommTcpIpObjectMotion` |
| **Instances** | 1 |

---

## 10. Alarm Monitor Classes

### 10.1 CAlarmMonitorImpl (Base)

| Item | Detail |
|------|--------|
| **Role** | Abstract base for alarm monitors. Each concrete monitor runs its own thread polling specific I/O conditions. |
| **Header** | `Source/ALPHA/Alarm/AlarmMonitorImpl.h` |
| **Inheritance** | Base class |

**Pure Virtual Interface:**
```cpp
virtual void SetStart() = 0;
virtual void SetStop() = 0;
```

**Protected Members:** `CSystemConfigMgr*`, `CLogInControlMgr*`, `CIOMapControlMgr*`, `CMaintControlMgr*`, `CCommTcpIPMgr*`

---

### 10.2 Alarm Monitor Concrete Classes

All inherit from `CAlarmMonitorImpl`. Each runs an independent monitoring thread.

| Class | Header | Monitors |
|-------|--------|----------|
| `CAlarmMonitorToEmergency` | `Alarm/AlarmMonitorToEmergency.h` | Emergency stop buttons |
| `CAlarmMonitorToEmergencyButton` | `Alarm/AlarmMonitorToEmergencyButton.h` | Physical E-stop button state |
| `CAlarmMonitorToDoorOpen` | `Alarm/AlarmMonitorToDoorOpen.h` | Door open detection |
| `CAlarmMonitorToDoorOpenState` | `Alarm/AlarmMonitorToDoorOpenState.h` | Door state tracking |
| `CAlarmMonitorToDoorVoice` | `Alarm/AlarmMonitorToDoorVoice.h` | Door voice alarm |
| `CAlarmMonitorToSafetyPLC` | `Alarm/AlarmMonitorToSafetyPLC.h` | PLC safety circuit |
| `CAlarmMonitorToMcOFF` | `Alarm/AlarmMonitorToMcOFF.h` | Motor controller offline |
| `CAlarmMonitorToMcOFFState` | `Alarm/AlarmMonitorToMcOFFState.h` | MC offline state |
| `CAlarmMonitorToLampTime` | `Alarm/AlarmMonitorToLampTime.h` | Lamp usage time limit |
| `CAlarmMonitorToTeachKEY` | `Alarm/AlarmMonitorToTeachKEY.h` | Teach pendant key |
| `CAlarmMonitorToTeachKEYLock` | `Alarm/AlarmMonitorToTeachKEYLock.h` | Teach key lock state |
| `CAlarmMonitorToSensor` | `Alarm/AlarmMonitorToSensor.h` | General sensor monitor |
| `CAlarmMonitorToSensorCurtain` | `Alarm/AlarmMonitorToSensorCurtain.h` | Light curtain sensor |
| `CAlarmMonitorToSensorFAN` | `Alarm/AlarmMonitorToSensorFAN.h` | Cooling fan sensor |
| `CAlarmMonitorToSensorHandSwitch` | `Alarm/AlarmMonitorToSensorHandSwitch.h` | Hand switch sensor |
| `CAlarmMonitorToSensorIsolater` | `Alarm/AlarmMonitorToSensorIsolater.h` | Isolator sensor |
| `CAlarmMonitorToSensorPressure` | `Alarm/AlarmMonitorToSensorPressure.h` | Air pressure sensor |
| `CAlarmMonitorToSensorSmoke` | `Alarm/AlarmMonitorToSensorSmoke.h` | Smoke detector |
| `CAlarmMonitorToSensorTemperature` | `Alarm/AlarmMonitorToSensorTemperature.h` | Temperature sensor (high) |
| `CAlarmMonitorToSensorTemperatureLow` | `Alarm/AlarmMonitorToSensorTemperatureLow.h` | Temperature sensor (low) |
| `CAlarmMonitorToSensorVacSwitch` | `Alarm/AlarmMonitorToSensorVacSwitch.h` | Vacuum switch |

**Also referenced:** `CAlarmController` (`Alarm/AlarmController.h`) -- Unknown role, likely alarm state management.

---

## 11. UI Dialog Classes

### 11.1 CD_AuroraDlgWin

| Item | Detail |
|------|--------|
| **Role** | Base modeless dialog for popup/auxiliary windows. |
| **Header** | `Source/ALPHA/D_AuroraDlgWin.h` |
| **Inheritance** | `CDialogEx` -> `CD_AuroraDlgWin` |

**Key Interface:**
```cpp
BOOL Create(UINT id, BOOL bShow = TRUE);
virtual void ControlButtonEnable(BOOL bEnable);
```

**Used as base for:** `CDlgMainForm`

---

### 11.2 CMainSubDlgWin

| Item | Detail |
|------|--------|
| **Role** | Base dialog for all navigation-switchable sub-panels. Defines virtual update methods for status/recipe/inspection/alarm events. |
| **Header** | `Source/ALPHA/MainSubDlgWin.h` |
| **Inheritance** | `CDialogEx` -> `CMainSubDlgWin` |

**Virtual Interface:**
```cpp
virtual void UpdateOperateStatus(CString str);
virtual void ShowReportAlarm();
virtual void UpdateSeqItemElapsedTime();
virtual void UpdateMeasureElapsedTime(bool bStart);
virtual void UpdateCycleInspectionStart() / Result() / Complete();
virtual void UpdateChangeInitialize();
virtual void UpdateRecipeInformation();
virtual void UpdateInformation();
virtual void UpdateCIMState(int iState);
virtual void UpdateControlMode(int iState);
virtual void UpdateMeasColor();
```

**Used as base for:** `CDlgNaviBar`, `CDlgSubOperate`, and all `DlgSub*` dialogs.

---

### 11.3 CDlgMainForm

| Item | Detail |
|------|--------|
| **Role** | Main form (IDD_DLG_MAIN_FORM). Contains navigation bar, CCD alignment views, motor/IO info grids, log listbox. Runs a background thread for probe monitoring. |
| **Header** | `Source/ALPHA/DlgMainForm.h` |
| **Source** | `Source/ALPHA/DlgMainForm.cpp` |
| **Inheritance** | `CDialogEx` -> `CD_AuroraDlgWin` -> `CDlgMainForm` |
| **Creation** | Unknown (likely in `CDlgALPHA::OnInitDialog()` or `CAuroraMgr`) |

**Key Members:**
- `CAuroraMgr *m_pAuroraMgr`
- `std::vector<CMainSubDlgWin*> m_vecpMainSubDlg` -- switchable sub-panels
- `CDlgNaviBar *m_pMainNavigateDlg` -- navigation bar
- `CDlgAlignCCD *m_pMainAlignCCD[MAIN_CCD_MAX_NUM]` -- CCD preview windows
- `stMotorInfo m_stMoterInfo[90]` -- motor status for 90 axes
- Worker thread for probe monitoring

---

### 11.4 CDlgNaviBar

| Item | Detail |
|------|--------|
| **Role** | Navigation toolbar (IDD_DLG_MAIN_NAVI). Buttons for Login, Operate, Result, Maint, Manual, Recipe, Log, Config, Alarm, BuzzerOff, PM Mode. |
| **Header** | `Source/ALPHA/DlgNaviBar.h` |
| **Inheritance** | `CDialogEx` -> `CMainSubDlgWin` -> `CDlgNaviBar` |

**Navigation IDs:** `enumNavigateID_LOGIN` through `enumNavigateID_PM_MODE` (11 views).

---

### 11.5 CDlgSubOperate

| Item | Detail |
|------|--------|
| **Role** | Main operate panel (IDD_DLG_SUB_OPERATE). Controls: Initialize, Safety, Load/Meas state change, Cycle/Resume/Pause/Stop buttons. Displays tact time, probe info, cell info, fixture status, CIM state. |
| **Header** | `Source/ALPHA/DlgSubOperate.h` |
| **Inheritance** | `CDialogEx` -> `CMainSubDlgWin` -> `CDlgSubOperate` |

**Key Members:**
- `CD_MainOperateInitializing *m_pSystemInitializingDlg`
- `CDlgSubMeasure_Progress* m_pAgingProgressDlg`
- `CUIMeasPathDlg* m_pParentMeasPathDlg` / `m_pParentCellMeasDlg`
- Lamp timers, LED buttons for fixture status
- 5 grid controls for tact time, glass time, probe info, cell info, temperature

---

### 11.6 Other Sub Dialogs

| Class | Header | Role |
|-------|--------|------|
| `CDlgSubAlarm` | `DlgSubAlarm.h` | Alarm display panel |
| `CDlgSubConfig` | `DlgSubConfig.h` | System configuration panel |
| `CDlgSubConfigLoginFile` | `DlgSubConfigLoginFile.h` | Login configuration sub-panel |
| `CDlgSubConfigModuleFile` | `DlgSubConfigModuleFile.h` | Module configuration sub-panel |
| `CDlgSubConfigMotionFile` | `DlgSubConfigMotionFile.h` | Motion configuration sub-panel |
| `CDlgSubData` | `DlgSubData.h` | Data/result display panel |
| `CDlgSubLog` | `DlgSubLog.h` | Log viewer panel |
| `CDlgSubLogFile` | `DlgSubLogFile.h` | Log file browser |
| `CDlgSubMaint` | `DlgSubMaint.h` | Maintenance panel |
| `CDlgSubMaint_ETCFile` | `DlgSubMaint_ETCFile.h` | Misc maintenance sub-panel |
| `CDlgSubMaint_IOFile` | `DlgSubMaint_IOFile.h` | I/O maintenance sub-panel |
| `CDlgSubMaint_MotionFile` | `DlgSubMaint_MotionFile.h` | Motion maintenance sub-panel |
| `CDlgSubManual` | `DlgSubManual.h` | Manual operation panel |
| `CDlgSubManual_Meas` | `DlgSubManual_Meas.h` | Manual measurement sub-panel |
| `CDlgSubManual_SACC` | `DlgSubManual_SACC.h` | Manual SACC sub-panel |
| `CDlgSubManual_Ionizer` | `DlgSubManual_Ionizer.h` | Manual ionizer sub-panel |
| `CDlgSubManual_AutoContact` | `DlgSubManual_AutoContact.h` | Manual auto-contact sub-panel |
| `CDlgSubManual_AgingTest` | `DlgSubManual_AgingTest.h` | Manual aging test sub-panel |
| `CDlgSubManual_AgingSet` | `DlgSubManual_AgingSet.h` | Aging settings sub-panel |
| `CDlgSubManual_STEP` | `DlgSubManual_STEP.h` | Manual step-by-step sub-panel |
| `CDlgSubMeasure_Progress` | `DlgSubMeasure_Progress.h` | Measurement progress display |
| `CDlgSubRecipe` | `DlgSubRecipe.h` | Recipe edit panel |
| `CD_PopUp_Alarm_F` | `CD_PopUp_Alarm_F.h` | Popup alarm dialog |
| `CD_PopUp_Motion_Move` | `CD_PopUp_Motion_Move.h` | Popup motion move dialog |
| `CD_SelectColor` | `CD_SelectColor.h` | Color selection dialog |
| `CD_CCD_Review` | `D_CCD_Review.h` | CCD image review dialog |
| `CD_DisplayMessage` | `D_DisplayMessage.h` | Display message dialog |
| `CD_InputPassWord` | `D_InputPassWord.h` | Password input dialog |
| `CD_LogInMgr` | `D_LogInMgr.h` | Login dialog |
| `CD_MainOperateInitializing` | `D_MainOperateInitializing.h` | Initialization progress dialog |
| `CD_MeasureCycle` | `D_MeasureCycle.h` | Measurement cycle dialog |
| `CD_MoveHomePosition` | `D_MoveHomePosition.h` | Home position move dialog |
| `CD_PM_MODE` | `D_PM_MODE.h` | PM (Preventive Maintenance) mode dialog |
| `CD_RoiSet` | `D_RoiSet.h` | ROI (Region of Interest) setting dialog |
| `CD_SelectMeasOrUnload` | `D_SelectMeasOrUnload.h` | Measure/Unload selection dialog |
| `CD_SetLampTime` | `D_SetLampTime.h` | Lamp time setting dialog |
| `CD_SystemHostMode` | `D_SystemHostMode.h` | System host mode dialog |

---

## 12. Inheritance Hierarchy Summary

```
CWinApp
  +-- CALPHAApp

CDialogEx
  +-- CDlgALPHA
  +-- CD_AuroraDlgWin
  |     +-- CDlgMainForm
  +-- CMainSubDlgWin
        +-- CDlgNaviBar
        +-- CDlgSubOperate
        +-- CDlgSubAlarm, CDlgSubConfig, CDlgSubData, CDlgSubLog,
            CDlgSubMaint, CDlgSubManual, CDlgSubRecipe, ...

CAuroraSubMgr (base for all managers)
  +-- CSystemConfigMgr
  +-- CNetworkConfigMgr
  +-- CPositionConfigMgr
  +-- CLogInControlMgr
  +-- CIOMapControlMgr
  +-- CMotorControlMgr
  +-- CMaintControlMgr
  +-- CLightControlMgr
  +-- CShareMemoryMgr
  +-- CCommTcpIpFactoryMgr
  +-- CCommTcpIPMgr
  +-- CAlarmMonitorMgr
  +-- CSeqTactTimeMgr
  +-- CAlignCameraMgr

CBaseSequenceBody
  +-- CBaseSequenceImpl
  |     +-- CSystemInitialize
  |     +-- CDoWorkGlassSequence
  |     +-- CDoWorkSequenceCIM
  |     +-- CFunctionModuleEQState
  |     +-- CFunctionModuleEQStateChange
  |     +-- CFunctionModuleEQInterlock
  |     +-- CFunctionModuleProbeACC
  |     +-- CFunctionModuleAFControl
  |     +-- CFunctionModuleCentering
  |     +-- CDoWorkGlassSequenceImpl
  |           +-- CDoWorkGlassReady
  |           +-- CDoWorkGlassRecipe
  |           +-- CDoWorkGlassAdsorption
  |           +-- CDoWorkGlassSWAlign
  |           +-- CDoWorkGlassPreMeas
  |           +-- CDoWorkGlassMeasure
  |           +-- CDoWorkGlassAGING
  |           +-- CDoWorkGlassResult
  |           +-- CDoWorkGlassUnload
  |           +-- CDoWorkGlassUnloading
  |           +-- CDoWorkProbeSWAlign
  +-- CMeasureSequenceItemImpl (separate thread/step model)

CCommEthFunction (TCP/IP base)
  +-- CCommTcpIpObjectUTG
  +-- CCommTcpIpObjectVision
  +-- CCommTcpIpObjectColor
  +-- CCommTcpIpObjectWAD
  +-- CCommTcpIpObjectAngle
  +-- CCommTcpIpObjectPG
  +-- CCommTcpIpObjectAgingPG
  +-- CCommTcpIpObjectLCR
  +-- CCommTcpIpObjectTSP
  +-- CCommTcpIpObjectCIM
  +-- CCommTcpIpObjectSysTime
  +-- CCommTcpIpObjectMotion

CAlarmMonitorImpl (alarm thread base)
  +-- CAlarmMonitorToEmergency
  +-- CAlarmMonitorToEmergencyButton
  +-- CAlarmMonitorToDoorOpen
  +-- CAlarmMonitorToDoorOpenState
  +-- CAlarmMonitorToDoorVoice
  +-- CAlarmMonitorToSafetyPLC
  +-- CAlarmMonitorToMcOFF
  +-- CAlarmMonitorToMcOFFState
  +-- CAlarmMonitorToLampTime
  +-- CAlarmMonitorToTeachKEY
  +-- CAlarmMonitorToTeachKEYLock
  +-- CAlarmMonitorToSensor
  +-- CAlarmMonitorToSensorCurtain
  +-- CAlarmMonitorToSensorFAN
  +-- CAlarmMonitorToSensorHandSwitch
  +-- CAlarmMonitorToSensorIsolater
  +-- CAlarmMonitorToSensorPressure
  +-- CAlarmMonitorToSensorSmoke
  +-- CAlarmMonitorToSensorTemperature
  +-- CAlarmMonitorToSensorTemperatureLow
  +-- CAlarmMonitorToSensorVacSwitch
```

---

## 13. Class Relationship Diagram

```
CALPHAApp
  |
  +-- creates --> CAuroraMgr (owns stAuroraMgrPool with 28+ managers)
  |                 |
  |                 +-- owns --> CSystemConfigMgr, CNetworkConfigMgr, CPositionConfigMgr,
  |                 |            CLogInControlMgr, CIOMapControlMgr, CMotorControlMgr,
  |                 |            CMaintControlMgr, CLightControlMgr, CShareMemoryMgr,
  |                 |            CCommTcpIpFactoryMgr, CCommTcpIPMgr, CAlarmMonitorMgr,
  |                 |            CSeqTactTimeMgr, CAlignCameraMgr[N]
  |                 |
  |                 +-- CCommTcpIpFactoryMgr
  |                       +-- owns --> 13 CCommTcpIpObject* instances (CCommEthFunction derived)
  |                 |
  |                 +-- CAlarmMonitorMgr
  |                       +-- owns --> 24+ CAlarmMonitorImpl threads
  |
  +-- creates --> CDlgALPHA (main dialog)
                    |
                    +-- creates --> CDlgMainForm (main form)
                                     |
                                     +-- owns --> CDlgNaviBar (navigation)
                                     +-- owns --> CMainSubDlgWin* sub-panels
                                     |              (CDlgSubOperate, CDlgSubMaint, ...)
                                     +-- owns --> CDlgAlignCCD[N] (camera views)

CSequenceManager (Singleton)
  |
  +-- owns --> CSystemInitialize
  +-- owns --> CDoWorkGlassSequence
  |              +-- owns --> CDoWorkGlassSequenceEngine
  |                             +-- owns --> CDoWorkGlass* items (Ready, Recipe, Adsorption,
  |                                          SWAlign, PreMeas, Measure, AGING, Result,
  |                                          Unload, Unloading, ProbeSWAlign)
  |                                          each may own --> CMeasureSequenceItemEngine
  +-- owns --> CFunctionModuleEQState
  +-- owns --> CFunctionModuleEQStateChange
  +-- owns --> CFunctionModuleEQInterlock
  +-- owns --> CFunctionModuleProbeACC
  +-- owns --> CFunctionModuleAFControl
  +-- owns --> CFunctionModuleCentering
  +-- owns --> CDoWorkSequenceCIM

CSystemParamShare (Singleton) -- global runtime state
CWindowEventHandler (Singleton) -- UI message routing
CRecipeMgr (Singleton) -- recipe DLL wrapper
CResultMgr (Singleton) -- result DLL wrapper
CMCCLogMgr (Singleton) -- MCC/FDC logging
CSACCMgr (Singleton) -- probe card management
```

---

**Singleton Summary:**

| Class | Access Pattern |
|-------|---------------|
| `CSequenceManager` | `CSequenceManager::GetInstance()` |
| `CWindowEventHandler` | `CWindowEventHandler::GetInstance()` |
| `CSystemParamShare` | `CSystemParamShare::GetInstance()` |
| `CRecipeMgr` | `CRecipeMgr::GetInstance()` |
| `CResultMgr` | `CResultMgr::GetInstance()` |
| `CMCCLogMgr` | `CMCCLogMgr::GetInstance()` |
| `CSACCMgr` | `CSACCMgr::GetInstance()` |

**Thread Count Estimate:** ~50+ threads
- 1 main UI thread
- 24+ alarm monitor threads (CAlarmMonitorImpl)
- 10+ sequence threads (CBaseSequenceImpl + CDoWorkGlassSequenceEngine + CMeasureSequenceItemEngine)
- 13+ TCP/IP reconnect threads (CCommEthFunction)
- 1+ worker threads (CDlgMainForm probe monitor)
- 1 CIM FTP upload thread
