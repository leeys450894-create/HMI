# ALPHA HMI - Project Index

> ALPHA HMI 프로젝트의 마스터 인덱스. Claude Code가 프로젝트를 빠르게 탐색하기 위한 진입점.
> 상세 분석은 `docs/` 폴더의 개별 문서 참조.

---

## 1. 프로젝트 전체 요약

ALPHA HMI는 Samsung 디스플레이 Glass 검사 자동화 설비를 제어하는 MFC C++ Dialog 기반 애플리케이션이다 (VS2019, Unicode, x64). CAuroraMgr를 중심으로 20+ SubMgr가 구성되며, CSequenceManager(Singleton)가 22단계 초기화 및 9단계 Glass 자동 검사 시퀀스를 관리한다. ACS SPiiPlus 모션 컨트롤러(TCP/IP), Mitsubishi MELSEC MC Protocol PLC(TCP/IP), Basler GigE 카메라(NI Vision), 13종 TCP/IP 장비 통신을 통합하며, 런타임 시 50+ 스레드가 동작한다. [See 00_ProjectOverview.md](docs/00_ProjectOverview.md)

---

## 2. 주요 클래스

| 클래스 | 파일 경로 | 역할 |
|--------|----------|------|
| `CALPHAApp` | `Source/ALPHA/ALPHA.cpp/.h` | MFC Application entry point, CAuroraMgr 생성 및 메인 다이얼로그 실행 |
| `CDlgALPHA` | `Source/ALPHA/DlgALPHA.cpp/.h` | 메인 대화상자 (IDD_ALPHA_DIALOG), 루트 UI 컨테이너 |
| `CAuroraMgr` | `Source/ALPHA/AuroraMgr.cpp/.h` | 중앙 오케스트레이터, 모든 SubMgr 소유 및 생명주기 관리 |
| `stAuroraMgrPool` | `Source/ALPHA/AuroraMgrPool.h` | 모든 매니저 포인터를 담는 풀 구조체 |
| `CAuroraSubMgr` | `Source/ALPHA/AuroraSubMgr.h` | SubMgr 기반 클래스 (모든 Manager가 상속) |
| `CSequenceManager` | `Source/ALPHA/Manager/SequenceManager.cpp/.h` | Singleton - 시퀀스 총괄 (초기화, Glass 시퀀스, 설비 상태) |
| `CSystemParamShare` | `Source/ALPHA/SystemParamShare.cpp/.h` | Singleton - 전역 런타임 파라미터 (auto mode, glass ID, cycle state) |
| `CWindowEventHandler` | `Source/ALPHA/WindowEventHandler.cpp/.h` | Singleton - UI 윈도우 메시지 라우팅 (HWND 레지스트리) |
| `CAlarmController` | `Source/ALPHA/Alarm/AlarmController.cpp/.h` | Singleton - Alarm DLL API 래핑, 알람 설정/해제/조회 |
| `CBaseSequenceImpl` | `Source/ALPHA/BaseSequenceImpl.cpp/.h` | 시퀀스 기반 클래스 (Thread + Start/Stop/Pause/Resume Event) |
| `CBaseSequenceBody` | `Source/ALPHA/BaseSequenceBody.cpp/.h` | 시퀀스 상태/로그/매니저 포인터 관리 |
| `CDoWorkGlassSequenceEngine` | `Source/ALPHA/DoWork/DoWorkGlassSequenceEngine.cpp/.h` | 9단계 Glass 시퀀스 엔진 (Thread) |
| `CDlgMainForm` | `Source/ALPHA/DlgMainForm.cpp/.h` | 메인 폼 - 모든 Sub Dialog 소유 |
| `CFunctionModuleEQState` | `Source/ALPHA/FunctionModule/FunctionModuleEQState.cpp/.h` | 설비 상태 머신 (READY/LOAD/MEASURE) |
| `CFunctionModuleEQInterlock` | `Source/ALPHA/FunctionModule/FunctionModuleEQInterlock.cpp/.h` | 안전 인터록 |

[See 01_ClassArchitecture.md](docs/01_ClassArchitecture.md) | [See 02_CallGraph.md](docs/02_CallGraph.md)

---

## 3. 주요 Manager

CAuroraMgr 생성자에서 순서대로 생성되는 SubMgr 목록:

| # | Manager | 파일 경로 | 역할 |
|---|---------|----------|------|
| 1 | `CSystemConfigMgr` | `Source/ALPHA/Manager/SystemConfigMgr.cpp/.h` | 시스템 설정 INI 로드/관리 |
| 2 | `CLogInControlMgr` | `Source/ALPHA/Manager/LogInControlMgr.cpp/.h` | 로그인/권한 제어 |
| 3 | `CAlignCameraMgr` (x4) | `Source/ALPHA/Manager/AlignCameraMgr.cpp/.h` | Align 카메라 관리 (Snap, PatternMatch, Angle 계산) |
| 4 | `CNetworkConfigMgr` | `Source/ALPHA/Manager/NetworkConfigMgr.cpp/.h` | 네트워크/TCP 설정 |
| 5 | `CPositionConfigMgr` | `Source/ALPHA/Manager/PositionConfigMgr.cpp/.h` | 모터 위치 프리셋 (Position.ini) |
| 6 | `CRecipeEditMgr` | `Source/ALPHA/Recipe/RecipeEditMgr.cpp/.h` | Recipe 편집/생명주기 (New/Open/Save/Delete) |
| 7 | `CShareMemoryMgr` | `Source/ALPHA/Manager/ShareMemoryMgr.cpp/.h` | 공유 메모리 관리 (VTM_SYSTEM) |
| 8 | `CLightControlMgr` | `Source/ALPHA/Manager/LightControlMgr.cpp/.h` | 조명 제어 |
| 9 | `CIOMapControlMgr` | `Source/ALPHA/Manager/IOMapControlMgr.cpp/.h` | PLC I/O Map 제어 (MC Protocol) |
| 10 | `CMotorControlMgr` | `Source/ALPHA/Manager/MotorControlMgr.cpp/.h` | 모터 제어 (ACS SPiiPlus, thread-safe wrapper) |
| 11 | `CMaintControlMgr` | `Source/ALPHA/Manager/MaintControlMgr.cpp/.h` | 유지보수 제어 (Motor+IO+Position 통합) |
| 12 | `CSeqTactTimeMgr` | `Source/ALPHA/Manager/SeqTactTimeMgr.cpp/.h` | Tact Time 로깅 |
| 13 | `CAlarmMonitorMgr` | `Source/ALPHA/Alarm/AlarmMonitorMgr.cpp/.h` | 알람 모니터 (24+ 개별 센서 스레드 관리) |
| 14 | `CCommTcpIpFactoryMgr` | `Source/ALPHA/Manager/CommTcpIpFactoryMgr.cpp/.h` | TCP/IP 장비 통신 Factory (13종 장비) |
| 15 | `CCommTcpIPMgr` | `Source/ALPHA/Manager/CommTcpIPMgr.cpp/.h` | TCP/IP 연결 관리 |

**Singleton Manager:**

| Manager | 파일 경로 | 역할 |
|---------|----------|------|
| `CRecipeMgr` | `Source/ALPHA/Manager/RecipeMgr.cpp/.h` | Recipe DLL 초기화 및 공유 메모리 접근 |
| `CResultMgr` | `Source/ALPHA/Manager/ResultMgr.cpp/.h` | 측정 결과 수집/저장 |
| `CSACCMgr` | `Source/ALPHA/Manager/SACCMgr.cpp/.h` | SACC (Probe card RFID 관리) |
| `CMCCLogMgr` | `Source/ALPHA/Manager/MCCLogMgr.cpp/.h` | MCC 로깅, FDC 공유 메모리 |

---

## 4. Motion 관련 파일

[See 04_Motion.md](docs/04_Motion.md)

### Core Library (CoreFunction)

| 파일 | 경로 | 역할 |
|------|------|------|
| `AxisType.h/.cpp` | `CoreFunction/MotorFunction/AxisType.h/.cpp` | 90축 axis enum, speed grades, axis_data_type 구조체 |
| `MotorFunction.h/.cpp` | `CoreFunction/MotorFunction/MotorFunction.h/.cpp` | Motor Facade (Strategy pattern host) |
| `MotorImpl.h/.cpp` | `CoreFunction/MotorFunction/MotorImpl.h/.cpp` | Motor 추상 기반 클래스 |
| `MotorControl_ACS.h/.cpp` | `CoreFunction/MotorFunction/MotorCtrl_ACS/MotorControl_ACS.h/.cpp` | ACS SPiiPlus TCP/IP 구현체 |
| `ACSC.h` | `CoreFunction/MotorFunction/MotorCtrl_ACS/include/ACSC.h` | ACS SPiiPlus C Library 헤더 v7.1.1.0 |
| `ACSCL_x64.dll/LIB` | `CoreFunction/MotorFunction/MotorCtrl_ACS/dll/` & `lib/` | ACS 런타임 라이브러리 (x64) |

### Application Layer

| 파일 | 경로 | 역할 |
|------|------|------|
| `MotorControlMgr.h/.cpp` | `Source/ALPHA/Manager/MotorControlMgr.h/.cpp` | Thread-safe 모터 매니저 (interlock 포함) |
| `MaintControlMgr.h/.cpp` | `Source/ALPHA/Manager/MaintControlMgr.h/.cpp` | 상위 오케스트레이터 (Motor+IO+Position) |
| `PositionConfigMgr.h/.cpp` | `Source/ALPHA/Manager/PositionConfigMgr.h/.cpp` | 위치 프리셋 관리 (Position.ini) |
| `SystemInitializeStep.cpp` | `Source/ALPHA/DoWork/SystemInitializeStep.cpp` | 호밍 시퀀스 구현 (Step 4~16) |

### Configuration

| 파일 | 경로 | 역할 |
|------|------|------|
| `Config_Motion_ACS.dat` | (런타임 Config 폴더) | 축별 파라미터 (속도/가속도/매핑) |
| `Position.ini` | (런타임 Config 폴더) | Safety/Loader/Align/Head/Probe 위치 프리셋 |
| `CoreFunction.ini` | `CoreFunction/CoreFunction.ini` | ACS 연결 설정 (`[MOTOR_CONTROL_MGR]` IP/Port) |

---

## 5. PLC 관련 파일

[See 05_PLC.md](docs/05_PLC.md)

### Core Library (CoreFunction)

| 파일 | 경로 | 역할 |
|------|------|------|
| `IOMapFunction.h/.cpp` | `CoreFunction/IOMapFunction/IOMapFunction.h/.cpp` | I/O Map Facade |
| `IOMapImpl.h/.cpp` | `CoreFunction/IOMapFunction/IOMapImpl.h/.cpp` | I/O Map 추상 기반 |
| `DataStructure.h` | `CoreFunction/IOMapFunction/DataStructure.h` | MC Protocol 상수, 디바이스 enum |
| `McProtocolManager.cpp/.h` | `CoreFunction/IOMapFunction/McProtocol/McProtocolManager.cpp/.h` | Multi-scanner 매니저 |
| `McMaddressScanner.cpp/.h` | `CoreFunction/IOMapFunction/McProtocol/McMaddressScanner.cpp/.h` | M-area 읽기/쓰기 |
| `McFaddressScanner.cpp/.h` | `CoreFunction/IOMapFunction/McProtocol/McFaddressScanner.cpp/.h` | F-alarm 읽기 |
| `McDaddressScanner.cpp/.h` | `CoreFunction/IOMapFunction/McProtocol/McDaddressScanner.cpp/.h` | D-area (FDC) |
| `McGaddressScanner.cpp/.h` | `CoreFunction/IOMapFunction/McProtocol/McGaddressScanner.cpp/.h` | Glass sensor D-area |
| `McPaddressScanner.cpp/.h` | `CoreFunction/IOMapFunction/McProtocol/McPaddressScanner.cpp/.h` | PACC M-area |
| `McCaddressScanner.cpp/.h` | `CoreFunction/IOMapFunction/McProtocol/McCaddressScanner.cpp/.h` | Probe card D-area |
| `McAddressScannerImpl.cpp/.h` | `CoreFunction/IOMapFunction/McProtocol/McAddressScannerImpl.cpp/.h` | 스캐너 추상 기반 |

### Application Layer

| 파일 | 경로 | 역할 |
|------|------|------|
| `IOMapControlMgr.cpp/.h` | `Source/ALPHA/Manager/IOMapControlMgr.cpp/.h` | PLC I/O 매니저 (SubMgr) |
| `IOMapAddressDefine.h` | `Source/ALPHA/Manager/IOMapAddressDefine.h` | plc_MBIT_t enum (~500 논리 상수) |
| `FunctionModuleEQState.cpp/.h` | `Source/ALPHA/FunctionModule/FunctionModuleEQState.cpp/.h` | PLC 비트 기반 설비 상태 쿼리 |
| `FunctionModuleEQInterlock.cpp/.h` | `Source/ALPHA/FunctionModule/FunctionModuleEQInterlock.cpp/.h` | PLC 비트 기반 모션 인터록 |

### Header/Config

| 파일 | 경로 | 역할 |
|------|------|------|
| `DefinesPLC.h` | `Source/Include/DefinesPLC.h` | PLC 비트 주소 맵 (1000+ 라인, 레거시) |
| `PLCMAdress.csv` | `Source/ALPHA/ConfigurationFile/CoreFunction/IOMapConfig/PLCMAdress.csv` | 논리-물리 M-address 매핑 테이블 |
| `M.csv` | `CoreFunction/IOMapConfig/M.csv` | M-address 설명 |
| `F.csv` | `CoreFunction/IOMapConfig/F.csv` | F-address (알람) 설명 |
| `CoreFunction.ini` | `CoreFunction/CoreFunction.ini` | PLC 연결 설정 (`[IO_MAP_CONTROL_MGR]` IP/Port) |

---

## 6. Vision 관련 파일

[See 06_Vision.md](docs/06_Vision.md)

### Core Library (CoreFunction)

| 파일 | 경로 | 역할 |
|------|------|------|
| `FrameGrabberFunction.h/.cpp` | `CoreFunction/FrameGrabberFunction/FrameGrabberFunction.h/.cpp` | Frame Grabber Facade, INI config |
| `FrameGrabberImpl.h/.cpp` | `CoreFunction/FrameGrabberFunction/FrameGrabberImpl.h/.cpp` | Frame Grabber 추상 기반 |
| `FrameGrabber_GigE_BS4M.h/.cpp` | `CoreFunction/FrameGrabberFunction/GigECamera_Basler/FrameGrabber_GigE_BS4M.h/.cpp` | GigE 어댑터 |
| `FrameGrabber_BS4M.h/.cpp` | `CoreFunction/FrameGrabberFunction/GigECamera_Basler/FrameGrabber_BS4M.h/.cpp` | NI-IMAQdx Basler 카메라 구현 |
| `IMAQImageCtrl` | `CoreFunction/common/IMAQImageCtrl/` | NI Vision wrapper (display, overlay, pattern search) |
| `PatternMatchFunction` | `CoreFunction/` | 패턴 매칭 (ProcessDirectMatch, ProcessCorrelation) |
| `GlassAlignFunction` | `CoreFunction/` | Align matrix 계산 |

### Application Layer

| 파일 | 경로 | 역할 |
|------|------|------|
| `AlignCameraMgr.cpp/.h` | `Source/ALPHA/Manager/AlignCameraMgr.cpp/.h` | 카메라 매니저 (4개, Snap/PatternMatch/Angle) |
| `DlgAlignCCD.cpp/.h` | `Source/ALPHA/DlgAlignCCD.cpp/.h` | CCD 라이브 뷰 다이얼로그 |
| `CD_CCD_Review.cpp/.h` | `Source/ALPHA/CD_CCD_Review.cpp/.h` | 전체화면 CCD 리뷰 |
| `CD_RoiSet.cpp/.h` | `Source/ALPHA/CD_RoiSet.cpp/.h` | ROI 설정 팝업 (패턴 Teach) |
| `DoWorkGlassSWAlign.cpp/.h` | `Source/ALPHA/DoWork/DoWorkGlassSWAlign.cpp/.h` | Glass Align 시퀀스 |
| `DoWorkProbeSWAlign.cpp/.h` | `Source/ALPHA/DoWork/DoWorkProbeSWAlign.cpp/.h` | Probe Align 시퀀스 |
| `CommTcpIpObjectVision.cpp/.h` | `Source/ALPHA/CommTcpIpObjectVision.cpp/.h` | 외부 Vision 장비 TCP/IP 통신 |

---

## 7. Auto Sequence 관련 파일

[See 03_AutoSequence.md](docs/03_AutoSequence.md)

### Sequence Infrastructure

| 파일 | 경로 | 역할 |
|------|------|------|
| `SequenceManager.cpp/.h` | `Source/ALPHA/Manager/SequenceManager.cpp/.h` | Singleton 시퀀스 총괄 |
| `BaseSequenceImpl.cpp/.h` | `Source/ALPHA/BaseSequenceImpl.cpp/.h` | Thread + Event 기반 시퀀스 기반 클래스 |
| `BaseSequenceBody.cpp/.h` | `Source/ALPHA/BaseSequenceBody.cpp/.h` | Step Index, Pause/Resume, 매니저 포인터 |

### System Initialize (22 Steps)

| 파일 | 경로 | 역할 |
|------|------|------|
| `SystemInitialize.cpp/.h` | `Source/ALPHA/DoWork/SystemInitialize.cpp/.h` | 시스템 초기화 시퀀스 |
| `SystemInitializeStep.cpp/.h` | `Source/ALPHA/DoWork/SystemInitializeStep.cpp/.h` | 초기화 22단계 구현 |

### Glass Auto-Run Sequence (9 Steps)

| 파일 | 경로 | 역할 |
|------|------|------|
| `DoWorkGlassSequence.cpp/.h` | `Source/ALPHA/DoWork/DoWorkGlassSequence.cpp/.h` | 시퀀스 엔진 래퍼 |
| `DoWorkGlassSequenceEngine.cpp/.h` | `Source/ALPHA/DoWork/DoWorkGlassSequenceEngine.cpp/.h` | 9개 시퀀스 아이템 엔진 (Thread) |
| `DoWorkGlassAdsorption.cpp/.h` + Step | `Source/ALPHA/DoWork/` | [0] Glass 흡착/측정 준비 |
| `DoWorkGlassRecipe.cpp/.h` + Step | `Source/ALPHA/DoWork/` | [1] Recipe 변경 및 장비 설정 |
| `DoWorkGlassSWAlign.cpp/.h` + Step | `Source/ALPHA/DoWork/` | [2] Glass Align |
| `DoWorkProbeSWAlign.cpp/.h` + Step | `Source/ALPHA/DoWork/` | [3] Probe Align |
| `DoWorkGlassPreMeas.cpp/.h` + Step | `Source/ALPHA/DoWork/` | [4] 사전 검사 (Contact, OST, R-Test) |
| `DoWorkGlassAGING.cpp/.h` + Step | `Source/ALPHA/DoWork/` | [5] Aging 검사 |
| `DoWorkGlassMeasure.cpp/.h` + Step | `Source/ALPHA/DoWork/` | [6] 본 검사 (WAD/VISION/COLOR/ANGLE/TSP) |
| `DoWorkGlassResult.cpp/.h` + Step | `Source/ALPHA/DoWork/` | [7] 결과 저장 및 판정 |
| `DoWorkGlassUnload.cpp/.h` + Step | `Source/ALPHA/DoWork/` | [8] Safety 이동 및 CIM 통보 |
| `DoWorkGlassUnloading.cpp/.h` + Step | `Source/ALPHA/DoWork/` | Unloading 상세 처리 |
| `DoWorkSequenceCIM.cpp/.h` + Step | `Source/ALPHA/DoWork/` | CIM/MES 시퀀스 |

### Measurement Engine

| 파일 | 경로 | 역할 |
|------|------|------|
| `MeasureSequenceItemEngine.cpp/.h` | `Source/ALPHA/DoWork/MeasureSequenceItemEngine.cpp/.h` | 측정 시퀀스 아이템 엔진 |
| `MeasureSequenceItemFactory.cpp/.h` | `Source/ALPHA/DoWork/MeasureSequenceItemFactory.cpp/.h` | 측정 아이템 Factory |
| `MeasureSequenceItemImpl.cpp/.h` | `Source/ALPHA/DoWork/MeasureSequenceItemImpl.cpp/.h` | 측정 아이템 구현 |

### Equipment State / Interlock

| 파일 | 경로 | 역할 |
|------|------|------|
| `FunctionModuleEQState.cpp/.h` | `Source/ALPHA/FunctionModule/FunctionModuleEQState.cpp/.h` | 설비 상태/센서 쿼리 |
| `FunctionModuleEQStateChange.cpp/.h` | `Source/ALPHA/FunctionModule/FunctionModuleEQStateChange.cpp/.h` | 설비 상태 전환 (READY<->LOAD<->MEASURE) |
| `FunctionModuleEQStateChangeStep.cpp/.h` | `Source/ALPHA/FunctionModule/FunctionModuleEQStateChangeStep.cpp/.h` | 상태 변경 단계 구현 |
| `FunctionModuleEQInterlock.cpp/.h` | `Source/ALPHA/FunctionModule/FunctionModuleEQInterlock.cpp/.h` | 안전 인터록 |
| `FunctionModuleProbeACC.cpp/.h` + Step | `Source/ALPHA/FunctionModule/FunctionModuleProbeACC.cpp/.h` | Probe Auto Contact Control |

---

## 8. Recipe 관련 파일

[See 07_Recipe.md](docs/07_Recipe.md)

### Application Layer

| 파일 | 경로 | 역할 |
|------|------|------|
| `RecipeData.cpp/.h` | `Source/ALPHA/Recipe/RecipeData.cpp/.h` | Recipe 데이터 모델 (INI/Binary 직렬화) |
| `RecipeEditMgr.cpp/.h` | `Source/ALPHA/Recipe/RecipeEditMgr.cpp/.h` | Recipe 생명주기 (New/Open/Save/Delete) |
| `RecipeMgr.cpp/.h` | `Source/ALPHA/Manager/RecipeMgr.cpp/.h` | Singleton - DLL 초기화, 공유 메모리 접근 |
| `DlgSubRecipe.cpp/.h` | `Source/ALPHA/DlgSubRecipe.cpp/.h` | Recipe UI 탭 |
| `DoWorkGlassRecipe.cpp/.h` | `Source/ALPHA/DoWork/DoWorkGlassRecipe.cpp/.h` | Auto-run 중 Recipe 변경 시퀀스 |
| `DoWorkGlassRecipeStep.cpp/.h` | `Source/ALPHA/DoWork/DoWorkGlassRecipeStep.cpp/.h` | Recipe 변경 Step 구현 |

### DLL Layer

| 파일 | 경로 | 역할 |
|------|------|------|
| `dfcRecipe.cpp/.h` | `CoreFunction/dll_Recipe/dfcRecipe/dfcRecipe.cpp/.h` | DLL 엔트리, 공유 메모리, config |
| `InRecipeFunc.cpp/.h` | `CoreFunction/dll_Recipe/dfcRecipe/InRecipeFunc.cpp/.h` | 내부 Recipe 로드/저장 |
| `OutRecipeFunc.cpp/.h` | `CoreFunction/dll_Recipe/dfcRecipe/OutRecipeFunc.cpp/.h` | 외부 Recipe 접근 API |
| `RecipeManager.cpp/.h` | `CoreFunction/dll_Recipe/dfcRecipe/RecipeManager.cpp/.h` | DLL UI CWnd 래퍼 |

### Header/Config

| 파일 | 경로 | 역할 |
|------|------|------|
| `preDefine.h` | `Source/Include/preDefine.h` | S_RecipeMgr, S_RecipeInfo 등 핵심 Recipe 구조체 |
| `RecipeShareMem.h` | `Source/Include/RecipeShareMem.h` | VTM_RECIPE 공유 메모리 구조체 |
| `API_RECIPE.h` | `Source/Include/API_RECIPE.h` | DLL API 함수 선언 |
| `OutRecipeFunc.h` | `Source/Include/OutRecipeFunc.h` | C_OutRecipeFunc DLL 접근 클래스 |
| `Recipe.ini` | `Source/ALPHA/ConfigurationFile/Recipe.ini` | DLL 설정 (폴더 경로, contact limit 등) |

---

## 9. Alarm 관련 파일

[See 08_Alarm.md](docs/08_Alarm.md)

### Application Layer

| 파일 | 경로 | 역할 |
|------|------|------|
| `AlarmController.cpp/.h` | `Source/ALPHA/Alarm/AlarmController.cpp/.h` | Singleton - Alarm DLL API facade |
| `AlarmMonitorMgr.cpp/.h` | `Source/ALPHA/Alarm/AlarmMonitorMgr.cpp/.h` | 알람 모니터 매니저 (전체 모니터 그룹 관리) |
| `AlarmMonitorImpl.cpp/.h` | `Source/ALPHA/Alarm/AlarmMonitorImpl.cpp/.h` | 알람 모니터 추상 기반 클래스 |
| `AlarmMonitorToEmergency.cpp/.h` | `Source/ALPHA/Alarm/AlarmMonitorToEmergency.cpp/.h` | EMS 버튼 모니터 그룹 |
| `AlarmMonitorToEmergencyButton.cpp/.h` | `Source/ALPHA/Alarm/AlarmMonitorToEmergencyButton.cpp/.h` | 개별 EMS 버튼 스레드 |
| `AlarmMonitorToMcOFF.cpp/.h` | `Source/ALPHA/Alarm/AlarmMonitorToMcOFF.cpp/.h` | MC OFF 모니터 그룹 |
| `AlarmMonitorToSensor.cpp/.h` | `Source/ALPHA/Alarm/AlarmMonitorToSensor.cpp/.h` | 센서 모니터 그룹 (80+ 스레드) |
| `AlarmMonitorToSensor*.cpp/.h` | `Source/ALPHA/Alarm/AlarmMonitorToSensor*.cpp/.h` | 개별 센서별 스레드 (Curtain/FAN/Smoke/Temp/Pressure/Vac/Isolater) |
| `AlarmMonitorToDoorOpen.cpp/.h` | `Source/ALPHA/Alarm/AlarmMonitorToDoorOpen.cpp/.h` | 도어 센서 모니터 그룹 |
| `AlarmMonitorToTeachKEY.cpp/.h` | `Source/ALPHA/Alarm/AlarmMonitorToTeachKEY.cpp/.h` | Teach/Auto 키 스위치 모니터 |
| `AlarmMonitorToSafetyPLC.cpp/.h` | `Source/ALPHA/Alarm/AlarmMonitorToSafetyPLC.cpp/.h` | Safety PLC 연결 모니터 |
| `AlarmMonitorToLampTime.cpp/.h` | `Source/ALPHA/Alarm/AlarmMonitorToLampTime.cpp/.h` | 램프 시간 모니터 |
| `DlgSubAlarm.cpp/.h` | `Source/ALPHA/DlgSubAlarm.cpp/.h` | 알람 UI 탭 |
| `CD_PopUp_Alarm_F.cpp/.h` | `Source/ALPHA/CD_PopUp_Alarm_F.cpp/.h` | 알람 팝업 |

### DLL Layer

| 파일 | 경로 | 역할 |
|------|------|------|
| `API_Alarm.cpp/.h` | `CoreFunction/dll_Alarm/Alarm/API_Alarm.cpp/.h` | DLL 내보내기 API |
| `AlarmManager.cpp/.h` | `CoreFunction/dll_Alarm/Alarm/AlarmManager.cpp/.h` | 핵심 알람 엔진 (lookup, set, clear) |
| `AlarmDefine.cpp/.h` | `CoreFunction/dll_Alarm/Alarm/AlarmDefine.cpp/.h` | ALARM_DATA, ALARM_EVENT_DATA 구조체 |
| `DlgAlarm.cpp/.h` | `CoreFunction/dll_Alarm/Alarm/DlgAlarm.cpp/.h` | 활성 알람 목록 다이얼로그 |
| `DlgAlarmModule.cpp/.h` | `CoreFunction/dll_Alarm/Alarm/DlgAlarmModule.cpp/.h` | 알람 상세/복구 패널 |

### Header/Config

| 파일 | 경로 | 역할 |
|------|------|------|
| `AlarmDefine.h` | `Source/Include/AlarmDefine.h` | 알람 구조체, extern 전역 변수 |
| `AlarmIDList.h` | `Source/Include/AlarmIDList.h` | enumAlarmID (1000~10000) |
| `API_Alarm.h` | `Source/Include/API_Alarm.h` | DLL API 함수 선언 |
| `Alarm.xls` | `Source/Bin/Config/CoreFunction/AlarmConfig/Alarm.xls` | 알람 정의 마스터 테이블 |
| `AlarmMessage.csv` | `Source/Bin/Config/CoreFunction/AlarmConfig/AlarmMessage.csv` | 알람 메시지 텍스트 |

---

## 10. 수정 시 가장 많이 참조해야 하는 파일

이 파일들은 프로젝트 수정 작업 시 가장 빈번하게 참조되는 핵심 파일이다.

### 최우선 참조 파일

| 파일 | 경로 | 이유 |
|------|------|------|
| `AuroraMgr.cpp/.h` | `Source/ALPHA/AuroraMgr.cpp/.h` | 전체 매니저 생성/접근 - 모든 기능의 진입점 |
| `AuroraMgrPool.h` | `Source/ALPHA/AuroraMgrPool.h` | 매니저 포인터 풀 - 새 매니저 추가 시 필수 |
| `SequenceManager.cpp/.h` | `Source/ALPHA/Manager/SequenceManager.cpp/.h` | 시퀀스 등록/실행 - 자동 실행 로직 변경 시 |
| `SystemParamShare.cpp/.h` | `Source/ALPHA/SystemParamShare.cpp/.h` | 전역 상태 - 거의 모든 클래스에서 참조 |
| `BaseSequenceImpl.cpp/.h` | `Source/ALPHA/BaseSequenceImpl.cpp/.h` | 스레드/이벤트 패턴 - 새 시퀀스 추가 시 |
| `ShareMem.h` | `Source/Include/ShareMem.h` | VTM_SYSTEM 공유 메모리 - 프로세스간 데이터 |

### 시퀀스 수정 시

| 파일 | 경로 | 이유 |
|------|------|------|
| `DoWorkGlassSequenceEngine.cpp/.h` | `Source/ALPHA/DoWork/DoWorkGlassSequenceEngine.cpp/.h` | 9단계 시퀀스 엔진 - 단계 추가/변경 시 |
| `SystemInitializeStep.cpp` | `Source/ALPHA/DoWork/SystemInitializeStep.cpp` | 초기화 단계 - 새 축/장비 추가 시 |
| `FunctionModuleEQStateChange.cpp/.h` | `Source/ALPHA/FunctionModule/FunctionModuleEQStateChange.cpp/.h` | 설비 상태 전환 - Safety/Load/Measure 로직 |

### 하드웨어 인터페이스 수정 시

| 파일 | 경로 | 이유 |
|------|------|------|
| `AxisType.h` | `CoreFunction/MotorFunction/AxisType.h` | 축 정의 - 모든 모션 코드의 기반 |
| `IOMapAddressDefine.h` | `Source/ALPHA/Manager/IOMapAddressDefine.h` | PLC 논리 주소 - 모든 I/O 코드의 기반 |
| `CommTcpIpFactoryMgr.cpp/.h` | `Source/ALPHA/Manager/CommTcpIpFactoryMgr.cpp/.h` | TCP/IP 장비 통신 - 새 장비 추가/프로토콜 변경 시 (~26,000줄 주의) |
| `CoreFunction.ini` | `CoreFunction/CoreFunction.ini` | 장비 연결 설정 (IP/Port/사용여부) |

### 공유 헤더 (여러 모듈에서 참조)

| 파일 | 경로 | 이유 |
|------|------|------|
| `preDefine.h` | `Source/Include/preDefine.h` | 전역 구조체 정의 (Recipe, Cell, Align 등) |
| `DefinesPLC.h` | `Source/Include/DefinesPLC.h` | PLC 비트 주소 맵 (1000+ 라인) |
| `DefinesSys.h` | `Source/Include/DefinesSys.h` | EQUIP_STATE, PROCESS_STATUS enum |
| `DefinesCIM.h` | `Source/Include/DefinesCIM.h` | CIM/MES 프로토콜 명령 |
| `AlarmIDList.h` | `Source/Include/AlarmIDList.h` | 알람 ID 상수 |
| `AuroraFuncDefine.h` | `Source/ALPHA/AuroraFuncDefine.h` | 유틸리티 함수/구조체 정의 |
| `pch.h` | `Source/ALPHA/pch.h` | 프리컴파일 헤더 (MAIN_CCD_MAX_NUM=4, CCriticalSectionSync 등) |

### 대용량 파일 주의 (반드시 특정 라인 범위만 읽을 것)

| 파일 | 경로 | 대략적 크기 |
|------|------|------------|
| `CommTcpIpFactoryMgr.cpp` | `Source/ALPHA/Manager/CommTcpIpFactoryMgr.cpp` | ~26,000줄 |
| `SystemInitializeStep.cpp` | `Source/ALPHA/DoWork/SystemInitializeStep.cpp` | ~2,000줄 |
| `DefinesPLC.h` | `Source/Include/DefinesPLC.h` | 1,000+줄 |
| `ShareMem.h` | `Source/Include/ShareMem.h` | ~1,700줄 |
| `MCCLogMgr.cpp/.h` | `Source/ALPHA/Manager/MCCLogMgr.cpp/.h` | 대형 |

---

## 상세 분석 문서 목록

| 문서 | 경로 | 내용 |
|------|------|------|
| Project Overview | [docs/00_ProjectOverview.md](docs/00_ProjectOverview.md) | Solution 구조, 폴더 구조, 레이어, Dependencies |
| Class Architecture | [docs/01_ClassArchitecture.md](docs/01_ClassArchitecture.md) | 전체 클래스 상세 (역할, 멤버, 인터페이스) |
| Call Graph | [docs/02_CallGraph.md](docs/02_CallGraph.md) | Entry point ~ UI ~ Manager ~ Device 호출 흐름 |
| Auto Sequence | [docs/03_AutoSequence.md](docs/03_AutoSequence.md) | 초기화 22단계, Glass 자동 검사 9단계 상세 |
| Motion | [docs/04_Motion.md](docs/04_Motion.md) | ACS SPiiPlus, 90축 정의, 속도/가속도, 호밍 |
| PLC | [docs/05_PLC.md](docs/05_PLC.md) | MC Protocol, M/F/D/P/C/G 스캐너, 주소 매핑 |
| Vision | [docs/06_Vision.md](docs/06_Vision.md) | Basler GigE, NI Vision, 패턴 매칭, Align |
| Recipe | [docs/07_Recipe.md](docs/07_Recipe.md) | Recipe 구조체, DLL, 공유 메모리, INI 포맷 |
| Alarm | [docs/08_Alarm.md](docs/08_Alarm.md) | 알람 DLL, 24+ 모니터 스레드, ALARM_DATA |
| Thread | [docs/09_Thread.md](docs/09_Thread.md) | 50+ 스레드 분류, 패턴 (Event-Driven/Polled/Connection) |
| Global State | [docs/10_GlobalState.md](docs/10_GlobalState.md) | Singleton, extern, 공유 메모리 (VTM_SYSTEM/VTM_RECIPE/VTM_FOR_FDC) |
