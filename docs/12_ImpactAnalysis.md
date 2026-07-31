# 12. Impact Analysis (Subsystem Modification Dependency Map)

> ALPHA HMI 프로젝트의 주요 서브시스템별 수정 시 영향받는 파일 목록.
> 실제 `#include` 참조, 함수 호출 관계, 변수 사용을 기반으로 작성됨.
> 모든 경로는 `HMI/Source/ALPHA/` 기준 (별도 표기 없는 한).

---

## 1. Motion/Servo 수정

모터/서보 제어 관련 변경 시 영향 범위. [See 04_Motion.md](04_Motion.md)

### Core Files (직접 수정 대상)
| File | Class/Role |
|------|-----------|
| `Manager/MotorControlMgr.h/.cpp` | `CMotorControlMgr` - 모터 제어 매니저 |
| `Manager/MaintControlMgr.h/.cpp` | `CMaintControlMgr` - 유지보수 제어 (MotorControlMgr.h include) |
| `CoreFunction/MotorFunction/` | `CMotorFunction` - ACS SPiiPlus 모터 API 래퍼 |
| `MotorFunction/AxisType.h` | 축 enum 정의, `axis_data_type` 구조체 |

### Affected: Sequence Engine (DoWork/)
| File | Dependency |
|------|-----------|
| `DoWork/SystemInitialize.cpp` | `#include MotorControlMgr.h` - 초기화 시 모터 연결 |
| `DoWork/DoWorkGlassAdsorptionStep.cpp` | `#include MaintControlMgr.h` - 흡착 동작 |
| `DoWork/DoWorkGlassAGINGStep.cpp` | `#include MaintControlMgr.h` - AGING 모터 이동 |
| `DoWork/DoWorkGlassMeasureStep.cpp` | `#include MaintControlMgr.h` - 측정 위치 이동 |
| `DoWork/DoWorkGlassPreMeasStep.cpp` | `#include MaintControlMgr.h` - 사전 측정 위치 |
| `DoWork/DoWorkGlassSWAlignStep.cpp` | (모션 의존 - align 시 축 이동) |
| `DoWork/DoWorkProbeSWAlignStep.cpp` | (모션 의존 - probe align 시 축 이동) |

### Affected: FunctionModule
| File | Dependency |
|------|-----------|
| `FunctionModule/FunctionModuleEQInterlock.cpp` | `#include MotorControlMgr.h` - 인터록 시 모터 상태 확인 |
| `FunctionModule/FunctionModuleEQStateChangeStep.cpp` | (commented out MotorControlMgr include - 간접 의존) |

### Affected: UI Dialogs
| File | Dependency |
|------|-----------|
| `DlgSubConfigMotionFile.cpp` | `#include MotorControlMgr.h` - 모터 설정 UI |
| `DlgSubMaint_MotionFile.cpp` | `#include MaintControlMgr.h` - 유지보수 모션 탭 |
| `DlgSubManual_STEP.h` | `#include AxisType.h` - 수동 스텝 이동 UI |
| `DlgSubManual_Meas.cpp` | `#include MaintControlMgr.h` - 수동 측정 UI |
| `CD_PopUp_Motion_Move.cpp` | `#include MaintControlMgr.h, ShareMemoryMgr.h` - 모션 이동 팝업 |

### Affected: Alarm Monitors
| File | Dependency |
|------|-----------|
| `Alarm/AlarmMonitorToSensorFAN.cpp` | (commented out MotorControlMgr) |
| `Alarm/AlarmMonitorToLampTime.cpp` | `#include MaintControlMgr.h` |
| `Alarm/AlarmMonitorToTeachKEY.cpp` | `#include MaintControlMgr.h` |
| `Alarm/AlarmMonitorToSensorTemperature.cpp` | `#include MaintControlMgr.h` |
| `Alarm/AlarmMonitorToSensorSmoke.cpp` | `#include MaintControlMgr.h` |
| `Alarm/AlarmMonitorToSensorIsolater.cpp` | `#include MaintControlMgr.h` |
| `Alarm/AlarmMonitorToSensorTemperatureLow.cpp` | `#include MaintControlMgr.h` |

### Affected: Communication
| File | Dependency |
|------|-----------|
| `Manager/CCommTcpIpObjectMotion.h/.cpp` | ACS 모터 컨트롤러 TCP/IP 통신 |
| `CommTcpIpObjectVision.cpp` | `#include MaintControlMgr.h` - Vision과 모터 연동 |
| `CommTcpIpObjectCIM.cpp` | `#include MaintControlMgr.h, MCCLogMgr.h` |

### Affected: Other Managers
| File | Dependency |
|------|-----------|
| `Manager/MCCLogMgr.cpp` | `#include AxisType.h` - MCC 로그에 축 정보 기록 |
| `Manager/SequenceManager.cpp` | (모터 제어 간접 참조) |

### Affected: Config
| File | Dependency |
|------|-----------|
| `Manager/SystemConfigMgr.h/.cpp` | 모터 관련 INI 설정 로드 |

---

## 2. PLC I/O 수정

PLC 비트 주소 맵, I/O 매핑 변경 시 영향 범위. [See 05_PLC.md](05_PLC.md)

### Core Files (직접 수정 대상)
| File | Class/Role |
|------|-----------|
| `Include/DefinesPLC.h` | PLC 비트 주소 상수 정의 (1000+ lines) |
| `Manager/IOMapAddressDefine.h` | I/O 맵 주소 상수, 센서/밸브/실린더 주소 |
| `Manager/IOMapControlMgr.h/.cpp` | `CIOMapControlMgr` - PLC I/O 읽기/쓰기 매니저 |
| `CoreFunction/IOMapFunction/` | I/O 맵 기능 라이브러리 |
| `CoreFunction/IOMapFunction/McProtocol/McProtocolManager.h/.cpp` | Mitsubishi MC Protocol 통신 |
| `IOMapFunction/DataStructure.h` | I/O 데이터 구조체 |

### Affected: Alarm Monitors (최대 영향 - 거의 모든 알람 모니터가 IOMap 참조)
| File | IOMapControlMgr | IOMapAddressDefine |
|------|:-:|:-:|
| `Alarm/AlarmMonitorMgr.cpp` | O | O |
| `Alarm/AlarmMonitorToEmergencyButton.cpp` | O | O |
| `Alarm/AlarmMonitorToSafetyPLC.cpp` | O | O |
| `Alarm/AlarmMonitorToSensorCurtain.cpp` | O | O |
| `Alarm/AlarmMonitorToMcOFFState.cpp` | O | O |
| `Alarm/AlarmMonitorToDoorOpenState.cpp` | O | O |
| `Alarm/AlarmMonitorToDoorVoice.cpp` | O | O |
| `Alarm/AlarmMonitorToSensorHandSwitch.cpp` | O | O |
| `Alarm/AlarmMonitorToSensorPressure.cpp` | O | O |
| `Alarm/AlarmMonitorToSensorVacSwitch.cpp` | O | O |
| `Alarm/AlarmMonitorToTeachKEYLock.cpp` | O | O |
| `Alarm/AlarmMonitorToDoorOpen.cpp` | - | O |
| `Alarm/AlarmMonitorToEmergency.cpp` | - | O |
| `Alarm/AlarmMonitorToMcOFF.cpp` | - | O |
| `Alarm/AlarmMonitorToSensorFAN.cpp` | - | O |
| `Alarm/AlarmMonitorToSensorIsolater.cpp` | - | O |
| `Alarm/AlarmMonitorToSensorSmoke.cpp` | - | O |
| `Alarm/AlarmMonitorToSensorTemperature.cpp` | - | O |
| `Alarm/AlarmMonitorToSensorTemperatureLow.cpp` | - | O |
| `Alarm/AlarmMonitorToLampTime.cpp` | - | O |
| `Alarm/AlarmMonitorToSensor.cpp` | - | O |
| `Alarm/AlarmMonitorToTeachKEY.cpp` | - | O |

### Affected: UI Dialogs
| File | Dependency |
|------|-----------|
| `DlgMainForm.cpp` | `#include IOMapControlMgr.h` |
| `DlgNaviBar.cpp` | `#include IOMapControlMgr.h` |
| `DlgTitleBar.cpp` | `#include IOMapControlMgr.h` |
| `DlgSubAlarm.cpp` | `#include IOMapControlMgr.h` |
| `DlgSubMaint_IOFile.cpp` | `#include IOMapControlMgr.h, DataStructure.h` |
| `DlgSubOperate.cpp` | `#include IOMapAddressDefine.h` |
| `CD_PopUp_Alarm_F.cpp` | `#include DataStructure.h` |

### Affected: Sequences
| File | Dependency |
|------|-----------|
| `DoWork/DoWorkGlassSWAlignStep.cpp` | `#include IOMapControlMgr.h` |
| `D_MeasureCycle.cpp` | `#include IOMapControlMgr.h` |
| `D_MoveHomePosition.cpp` | `#include IOMapControlMgr.h` |
| `D_PM_MODE.cpp` | `#include IOMapAddressDefine.h` |

### Affected: FunctionModule
| File | Dependency |
|------|-----------|
| `FunctionModule/FunctionModuleEQInterlock.cpp` | `#include IOMapControlMgr.h` - 인터록 I/O 확인 |

---

## 3. Vision/Camera 수정

비전 카메라, 패턴 매칭, 얼라인 관련 변경 시 영향 범위. [See 06_Vision.md](06_Vision.md)

### Core Files (직접 수정 대상)
| File | Class/Role |
|------|-----------|
| `Manager/AlignCameraMgr.h/.cpp` | `CAlignCameraMgr` - 카메라 매니저 (FrameGrabberFunction, ShareMem include) |
| `CoreFunction/FrameGrabberFunction/` | 프레임 그래버 하드웨어 인터페이스 |
| `CoreFunction/GlassAlignFunction/` | Glass 얼라인 알고리즘 |
| `CoreFunction/PatternMatchFunction/` | 패턴 매칭 알고리즘 |
| `CoreFunction/CoreFunction.h` | `#include GlassAlignFunction.h, PatternMatchFunction.h` |

### Affected: UI Dialogs
| File | Dependency |
|------|-----------|
| `DlgAlignCCD.h/.cpp` | `#include AlignCameraMgr.h, AuroraMgr.h` - CCD 카메라 뷰 UI |
| `DlgMainForm.cpp` | `#include DlgAlignCCD.h` - AlignCCD를 메인폼에 포함 |

### Affected: Sequences (DoWork/)
| File | Dependency |
|------|-----------|
| `DoWork/DoWorkGlassSWAlignStep.cpp` | `#include AlignCameraMgr.h` - Glass SW 얼라인 시 카메라 |
| `DoWork/DoWorkProbeSWAlignStep.cpp` | `#include AlignCameraMgr.h` - Probe SW 얼라인 시 카메라 |
| `DoWork/DoWorkGlassPreMeasStep.cpp` | `#include AlignCameraMgr.h` - 사전 측정 시 카메라 |
| `DoWork/DoWorkGlassMeasureStep.cpp` | `#include AlignCameraMgr.h` - 측정 시 카메라 |
| `DoWork/DoWorkGlassAGINGStep.cpp` | `#include AlignCameraMgr.h` - AGING 시 카메라 |
| `DoWork/DoWorkGlassSequenceEngine.cpp` | `#include DoWorkGlassSWAlign.h, DoWorkProbeSWAlign.h` |

### Affected: Communication
| File | Dependency |
|------|-----------|
| `CommTcpIpObjectVision.h/.cpp` | `#include CommEthFunction.h, AlarmController.h` - Vision 장비 TCP/IP 통신 |
| `Manager/CommTcpIpFactoryMgr.cpp` | `#include CommTcpIpObjectVision.h` - Vision 객체 생성 |
| `Manager/CommTcpIPMgr.cpp` | `#include CommTcpIpObjectVision.h` |

### Affected: Recipe
| File | Dependency |
|------|-----------|
| `Recipe/RecipeEditMgr.cpp` | `#include AlignCameraMgr.h` - 레시피에 비전 파라미터 포함 |

### Affected: Orchestrator
| File | Dependency |
|------|-----------|
| `AuroraMgr.cpp` | `#include AlignCameraMgr.h` - 카메라 매니저 생성/관리 |

---

## 4. Recipe 수정

레시피 데이터 구조, 편집, 배포 관련 변경 시 영향 범위. [See 07_Recipe.md](07_Recipe.md)

### Core Files (직접 수정 대상)
| File | Class/Role |
|------|-----------|
| `Recipe/RecipeData.h/.cpp` | `CRecipeData` - 레시피 데이터 구조체 (ShareMemoryMgr, WindowEventHandler include) |
| `Recipe/RecipeEditMgr.h/.cpp` | `CRecipeEditMgr` - 레시피 편집 매니저 (RecipeData include) |
| `Manager/RecipeMgr.h/.cpp` | `CRecipeMgr` - 레시피 관리 매니저 |
| `CoreFunction/dll_Recipe/` | 레시피 DLL 라이브러리 |
| `Include/API_RECIPE.h` | `#include RecipeShareMem.h, OutRecipeFunc.h` |

### Affected: TCP/IP Communication (레시피 배포 - 9개 장비)
| File | RecipeEditMgr Include |
|------|:-:|
| `CommTcpIpObjectPG.cpp` | O |
| `CommTcpIpObjectAgingPG.cpp` | O |
| `CommTcpIpObjectCIM.cpp` | O |
| `CommTcpIpObjectColor.cpp` | O |
| `CommTcpIpObjectAngle.cpp` | O |
| `CommTcpIpObjectLCR.cpp` | O |
| `CommTcpIpObjectTSP.cpp` | O |
| `CommTcpIpObjectVision.cpp` | O |
| `CommTcpIpObjectWAD.cpp` | O |

### Affected: Sequences (DoWork/)
| File | Dependency |
|------|-----------|
| `DoWork/DoWorkGlassSequenceEngine.cpp` | `#include RecipeEditMgr.h, DoWorkGlassRecipe.h` |
| `DoWork/DoWorkGlassRecipe.cpp/.h` | 레시피 적용 시퀀스 |
| `DoWork/DoWorkGlassRecipeStep.cpp` | `#include RecipeEditMgr.h` |
| `DoWork/DoWorkGlassAGINGStep.cpp` | `#include RecipeData.h, RecipeEditMgr.h` |
| `DoWork/DoWorkGlassMeasureStep.h/.cpp` | `#include RecipeData.h, RecipeEditMgr.h` |
| `DoWork/DoWorkGlassPreMeasStep.cpp` | `#include RecipeEditMgr.h` |
| `DoWork/DoWorkGlassReadyStep.cpp` | `#include RecipeEditMgr.h` |
| `DoWork/DoWorkGlassResultStep.cpp` | `#include RecipeEditMgr.h` |
| `DoWork/DoWorkGlassAdsorptionStep.cpp` | `#include RecipeEditMgr.h` |
| `DoWork/DoWorkGlassSWAlignStep.cpp` | `#include RecipeEditMgr.h` |
| `DoWork/DoWorkProbeSWAlignStep.cpp` | `#include RecipeEditMgr.h` |
| `DoWork/DoWorkGlassUnloadStep.cpp` | `#include RecipeEditMgr.h` |
| `DoWork/MeasureSequenceItemEngine.cpp` | `#include RecipeEditMgr.h` |
| `DoWork/SystemInitializeStep.cpp` | `#include RecipeEditMgr.h` |

### Affected: UI Dialogs
| File | Dependency |
|------|-----------|
| `DlgSubRecipe.cpp` | `#include RecipeEditMgr.h, RecipeMgr.h` - 레시피 편집 UI |
| `DlgSubOperate.cpp` | `#include RecipeEditMgr.h` |
| `DlgSubManual_Meas.h/.cpp` | `#include RecipeData.h, RecipeEditMgr.h` |
| `DlgTitleBar.cpp` | `#include RecipeEditMgr.h` - 타이틀바에 레시피명 표시 |
| `DlgMainForm.cpp` | `#include RecipeMgr.h, DlgSubRecipe.h` |
| `D_MeasureCycle.cpp` | `#include RecipeEditMgr.h` |
| `D_RoiSet.cpp` | (RecipeEditMgr 간접 참조) |

### Affected: FunctionModule
| File | Dependency |
|------|-----------|
| `FunctionModule/FunctionModuleProbeACCStep.cpp` | `#include RecipeEditMgr.h` |

### Affected: Other Managers
| File | Dependency |
|------|-----------|
| `Manager/MaintControlMgr.cpp` | `#include RecipeEditMgr.h` |
| `Recipe/RecipeEditMgr.cpp` | `#include AlignCameraMgr.h, SequenceManager.h, D_RoiSet.h, FunctionModuleProbeACC.h` |

### Affected: Orchestrator
| File | Dependency |
|------|-----------|
| `AuroraMgr.cpp` | `#include RecipeEditMgr.h, RecipeMgr.h` |

---

## 5. Alarm 수정

알람 정의, 알람 모니터링, 알람 컨트롤러 변경 시 영향 범위. [See 08_Alarm.md](08_Alarm.md)

### Core Files (직접 수정 대상)
| File | Class/Role |
|------|-----------|
| `Include/AlarmDefine.h` | 알람 코드 정의 |
| `Include/AlarmIDList.h` | 알람 ID 목록 (pch.h에서 전역 include) |
| `Include/API_Alarm.h` | 알람 DLL API 인터페이스 |
| `Alarm/AlarmController.h/.cpp` | `CAlarmController` - 알람 발생/해제 컨트롤러 (AlarmDefine.h include) |
| `Alarm/AlarmMonitorMgr.h/.cpp` | `CAlarmMonitorMgr` - 모니터 스레드 총괄 |
| `Alarm/AlarmMonitorImpl.h/.cpp` | `CAlarmMonitorImpl` - 모니터 기본 구현체 (AuroraMgrPool include) |
| `CoreFunction/dll_Alarm/` | 알람 DLL 라이브러리 |

### Affected: Alarm Monitor Threads (24개 개별 모니터)
모든 모니터가 `AlarmMonitorImpl` 상속, `AlarmController` 호출:

| Monitor File | IOMap | SystemConfig | MaintControl | ShareMem |
|-------------|:-----:|:------------:|:------------:|:--------:|
| `AlarmMonitorToEmergency.cpp` | O | - | - | - |
| `AlarmMonitorToEmergencyButton.cpp` | O | O | - | - |
| `AlarmMonitorToMcOFF.cpp` | O | - | - | - |
| `AlarmMonitorToMcOFFState.cpp` | O | O | - | - |
| `AlarmMonitorToSafetyPLC.cpp` | O | O | - | - |
| `AlarmMonitorToDoorOpen.cpp` | O | - | - | - |
| `AlarmMonitorToDoorOpenState.cpp` | O | O | - | - |
| `AlarmMonitorToDoorVoice.cpp` | O | O | - | - |
| `AlarmMonitorToSensor.cpp` | O | - | - | - |
| `AlarmMonitorToSensorCurtain.cpp` | O | O | - | - |
| `AlarmMonitorToSensorFAN.cpp` | O | O | O | - |
| `AlarmMonitorToSensorHandSwitch.cpp` | O | O | - | - |
| `AlarmMonitorToSensorIsolater.cpp` | O | O | O | - |
| `AlarmMonitorToSensorPressure.cpp` | O | O | - | - |
| `AlarmMonitorToSensorSmoke.cpp` | O | O | O | - |
| `AlarmMonitorToSensorTemperature.cpp` | O | O | O | - |
| `AlarmMonitorToSensorTemperatureLow.cpp` | O | O | O | O |
| `AlarmMonitorToSensorVacSwitch.cpp` | O | O | - | - |
| `AlarmMonitorToTeachKEY.cpp` | O | O | O | - |
| `AlarmMonitorToTeachKEYLock.cpp` | O | O | - | - |
| `AlarmMonitorToLampTime.cpp` | O | O | O | O |

### Affected: TCP/IP Communication (알람 발생 시 CIM 보고)
| File | Dependency |
|------|-----------|
| `CommTcpIpObjectCIM.cpp` | `#include AlarmController.h` |
| `CommTcpIpObjectPG.cpp` | `#include AlarmController.h` |
| `CommTcpIpObjectAgingPG.cpp` | `#include AlarmController.h` |
| `CommTcpIpObjectColor.cpp` | `#include AlarmController.h` |
| `CommTcpIpObjectAngle.cpp` | `#include AlarmController.h` |
| `CommTcpIpObjectLCR.cpp` | `#include AlarmController.h` |
| `CommTcpIpObjectTSP.cpp` | `#include AlarmController.h` |
| `CommTcpIpObjectVision.cpp` | `#include AlarmController.h` |
| `CommTcpIpObjectWAD.cpp` | `#include AlarmController.h` |

### Affected: UI Dialogs
| File | Dependency |
|------|-----------|
| `DlgALPHA.cpp` | `#include AlarmMonitorMgr.h, AlarmController.h` - 알람 모니터 시작/중지 |
| `DlgMainForm.cpp` | `#include AlarmMonitorMgr.h, DlgSubAlarm.h, CD_PopUp_Alarm_F.h` |
| `DlgSubAlarm.cpp` | `#include AlarmController.h, CommTcpIpObjectCIM.h` - 알람 UI |
| `CD_PopUp_Alarm_F.cpp` | 알람 팝업 다이얼로그 |

### Affected: Global (pch.h)
| File | Dependency |
|------|-----------|
| `pch.h` | `#include AlarmIDList.h` - **모든 소스 파일에 전파** |

> **주의:** `AlarmIDList.h`는 `pch.h`에 포함되어 있어 이 파일 수정 시 **전체 프로젝트 리빌드** 발생.

### Affected: Orchestrator
| File | Dependency |
|------|-----------|
| `AuroraMgr.cpp` | `#include AlarmMonitorMgr.h` |

---

## 6. Auto Sequence 수정

자동 시퀀스 (DoWork), 시퀀스 매니저, FunctionModule 변경 시 영향 범위. [See 03_AutoSequence.md](03_AutoSequence.md)

### Core Files (직접 수정 대상)
| File | Class/Role |
|------|-----------|
| `Manager/SequenceManager.h/.cpp` | `CSequenceManager` (Singleton) - 시퀀스 총괄 |
| `BaseSequenceBody.h/.cpp` | `CBaseSequenceBody` - 시퀀스 기본 클래스 (AuroraMgrPool include) |
| `BaseSequenceImpl.h/.cpp` | `CBaseSequenceImpl` - 시퀀스 구현 기본 클래스 |

### DoWork/ Sequence Files (9-step Glass Sequence)
| File | Class | Step |
|------|-------|------|
| `DoWork/DoWorkGlassSequenceEngine.h/.cpp` | `CDoWorkGlassSequenceEngine` | 시퀀스 엔진 총괄 |
| `DoWork/DoWorkGlassSequence.h/.cpp` | `CDoWorkGlassSequence` | Glass 시퀀스 컨테이너 |
| `DoWork/DoWorkGlassSequenceImpl.h/.cpp` | `CDoWorkGlassSequenceImpl` | Glass 시퀀스 구현체 |
| `DoWork/DoWorkGlassReady.h/.cpp` + Step | `CDoWorkGlassReady` | Step 1: Ready |
| `DoWork/DoWorkGlassAdsorption.h/.cpp` + Step | `CDoWorkGlassAdsorption` | Step 2: Adsorption |
| `DoWork/DoWorkGlassSWAlign.h/.cpp` + Step | `CDoWorkGlassSWAlign` | Step 3: Glass SW Align |
| `DoWork/DoWorkProbeSWAlign.h/.cpp` + Step | `CDoWorkProbeSWAlign` | Step 4: Probe SW Align |
| `DoWork/DoWorkGlassPreMeas.h/.cpp` + Step | `CDoWorkGlassPreMeas` | Step 5: Pre-Measure |
| `DoWork/DoWorkGlassRecipe.h/.cpp` + Step | `CDoWorkGlassRecipe` | Step 6: Recipe Apply |
| `DoWork/DoWorkGlassMeasure.h/.cpp` + Step | `CDoWorkGlassMeasure` | Step 7: Measure |
| `DoWork/DoWorkGlassAGING.h/.cpp` + Step | `CDoWorkGlassAGING` | Step 8: AGING |
| `DoWork/DoWorkGlassResult.h/.cpp` + Step | `CDoWorkGlassResult` | Step 9: Result |
| `DoWork/DoWorkGlassUnload.h/.cpp` + Step | `CDoWorkGlassUnload` | Unload |
| `DoWork/DoWorkGlassUnloading.h/.cpp` + Step | `CDoWorkGlassUnloading` | Unloading |
| `DoWork/SystemInitialize.h/.cpp` + Step | `CSystemInitialize` | 22단계 초기화 |
| `DoWork/DoWorkSequenceCIM.h/.cpp` + Step | `CDoWorkSequenceCIM` | CIM 시퀀스 |
| `DoWork/MeasureSequenceItemEngine.h/.cpp` | `CMeasureSequenceItemEngine` | 측정 아이템 엔진 |
| `DoWork/MeasureSequenceItemFactory.h/.cpp` | `CMeasureSequenceItemFactory` | 측정 아이템 팩토리 |
| `DoWork/MeasureSequenceItemImpl.h/.cpp` | `CMeasureSequenceItemImpl` | 측정 아이템 구현 |

### FunctionModule Files (상태 머신)
| File | Class |
|------|-------|
| `FunctionModule/FunctionModuleEQState.h/.cpp` | `CFunctionModuleEQState` - 설비 상태 관리 |
| `FunctionModule/FunctionModuleEQStateChange.h/.cpp` | `CFunctionModuleEQStateChange` - 상태 전환 |
| `FunctionModule/FunctionModuleEQStateChangeStep.h/.cpp` | 상태 전환 단계 구현 |
| `FunctionModule/FunctionModuleEQInterlock.h/.cpp` | `CFunctionModuleEQInterlock` - 안전 인터록 |
| `FunctionModule/FunctionModuleProbeACC.h/.cpp` | `CFunctionModuleProbeACC` - 프로브 ACC |
| `FunctionModule/FunctionModuleProbeACCStep.h/.cpp` | 프로브 ACC 단계 구현 |
| `FunctionModule/FunctionModuleAFControl.h` | AF 제어 (header only) |
| `FunctionModule/FunctionModuleCentering.h` | 센터링 (header only) |

### Affected: SequenceManager Consumers (42개 파일 참조)
SequenceManager는 시스템에서 가장 널리 참조되는 매니저 중 하나:

**TCP/IP Objects (9개):** CommTcpIpObject{CIM, AgingPG, PG, Color, Angle, LCR, TSP, Vision, WAD}.cpp
**UI Dialogs (8개):** DlgMainForm, DlgSubConfigModuleFile, DlgSubLog, DlgSubMaint, DlgSubMaint_MotionFile, DlgSubManual, DlgSubManual_Meas, DlgSubMeasure_Progress, DlgSubOperate
**DoWork Steps (12개):** 모든 DoWork*Step.cpp
**FunctionModule (3개):** FunctionModuleEQState, FunctionModuleEQStateChangeStep, FunctionModuleProbeACCStep
**Other Managers (3개):** MaintControlMgr, RecipeEditMgr, SACCMgr
**App Init (2개):** D_MainOperateInitializing, AuroraMgr

---

## 7. Communication (TCP/IP) 수정

TCP/IP 통신 객체, 팩토리 매니저, 이더넷 기능 변경 시 영향 범위.

### Core Files (직접 수정 대상)
| File | Class/Role |
|------|-----------|
| `Manager/CommTcpIpFactoryMgr.h/.cpp` | `CCommTcpIpFactoryMgr` - TCP/IP 객체 팩토리 (13종 장비) |
| `Manager/CommTcpIPMgr.h/.cpp` | `CCommTcpIPMgr` - TCP/IP 매니저 |
| `Manager/CommTcpIpDefine.h` | TCP/IP 통신 상수 정의 |
| `CoreFunction/CommEthFunction/` | `CCommEthFunction` - 이더넷 통신 기능 |
| `CoreFunction/CommEthFunction/EthernetManagerTCP.h` | TCP 연결 관리 |

### Device-Specific TCP/IP Objects (9개 + 2 내부)
| File | Class | Device |
|------|-------|--------|
| `CommTcpIpObjectCIM.h/.cpp` | `CCommTcpIpObjectCIM` | CIM/MES 서버 |
| `CommTcpIpObjectPG.h/.cpp` | `CCommTcpIpObjectPG` | Pattern Generator |
| `CommTcpIpObjectAgingPG.h/.cpp` | `CCommTcpIpObjectAgingPG` | Aging PG |
| `CommTcpIpObjectColor.h/.cpp` | `CCommTcpIpObjectColor` | Color 측정기 |
| `CommTcpIpObjectAngle.h/.cpp` | `CCommTcpIpObjectAngle` | Angle 측정기 |
| `CommTcpIpObjectLCR.h/.cpp` | `CCommTcpIpObjectLCR` | LCR 측정기 |
| `CommTcpIpObjectTSP.h/.cpp` | `CCommTcpIpObjectTSP` | TSP 측정기 |
| `CommTcpIpObjectVision.h/.cpp` | `CCommTcpIpObjectVision` | Vision 시스템 |
| `CommTcpIpObjectWAD.h/.cpp` | `CCommTcpIpObjectWAD` | WAD 측정기 |
| `Manager/CCommTcpIpObjectMotion.h/.cpp` | `CCommTcpIpObjectMotion` | ACS 모터 컨트롤러 |
| `Manager/CCommTcpIpObjectSysTime.h/.cpp` | `CCommTcpIpObjectSysTime` | 시스템 시간 동기화 |
| `Manager/CommTcpIpObjectUTG.h` | Unknown | UTG (header only) |

**공통 의존성:** 모든 CommTcpIpObject*는 다음을 include:
- `CommEthFunction/CommEthFunction.h`
- `Manager/CommTcpIpDefine.h`
- `AuroraMgrPool.h`
- `Manager/SystemConfigMgr.h`
- `Manager/SequenceManager.h`
- `Alarm/AlarmController.h`
- `Recipe/RecipeEditMgr.h`

### Affected: UI Dialogs
| File | Dependency |
|------|-----------|
| `DlgALPHA.cpp` | `#include CommTcpIpFactoryMgr.h` |
| `DlgSubAlarm.cpp` | `#include CommTcpIpObjectCIM.h, CommTcpIpFactoryMgr.h` |
| `DlgSubData.cpp` | `#include CommTcpIpObjectCIM.h, CommTcpIpFactoryMgr.h` |
| `DlgSubLog.cpp` | `#include CommTcpIpFactoryMgr.h` |
| `DlgSubOperate.cpp` | `#include CommTcpIpFactoryMgr.h, CommTcpIpObjectCIM.h` |
| `DlgSubManual_Meas.cpp` | `#include CommTcpIpFactoryMgr.h, CommTcpIpObjectPG.h` |
| `DlgSubConfigModuleFile.cpp` | `#include CommTcpIpFactoryMgr.h, CommEthFunction.h` |
| `DlgTitleBar.cpp` | `#include CommTcpIpFactoryMgr.h` |

### Affected: Sequences
| File | Dependency |
|------|-----------|
| `DoWork/DoWorkGlassAGINGStep.cpp` | `#include CommTcpIpFactoryMgr.h, CommTcpIpObjectPG.h` |
| `DoWork/DoWorkGlassMeasureStep.cpp` | `#include CommTcpIpFactoryMgr.h` |

### Affected: Orchestrator
| File | Dependency |
|------|-----------|
| `AuroraMgr.cpp` | `#include CommTcpIpFactoryMgr.h` |

---

## 8. UI 수정

다이얼로그, 이벤트 핸들러, 화면 구성 변경 시 영향 범위.

### Core Files (직접 수정 대상)
| File | Class/Role |
|------|-----------|
| `DlgALPHA.h/.cpp` | `CDlgALPHA` - 메인 다이얼로그 (Dialog-based app) |
| `DlgMainForm.h/.cpp` | `CDlgMainForm` - 메인 폼 (탭 전환, 서브다이얼로그 관리) |
| `WindowEventHandler.h/.cpp` | `CWindowEventHandler` - 윈도우 이벤트 핸들러 |
| `DlgTitleBar.h/.cpp` | `CDlgTitleBar` - 타이틀바 |
| `DlgNaviBar.h/.cpp` | `CDlgNaviBar` - 네비게이션바 |

### Sub Dialogs (탭별 화면)
| File | Class | Tab/Function |
|------|-------|-------------|
| `DlgSubOperate.h/.cpp` | `CDlgSubOperate` | 운영 탭 (WindowEventHandler include) |
| `DlgSubMaint.h/.cpp` | `CDlgSubMaint` | 유지보수 탭 (WindowEventHandler include) |
| `DlgSubRecipe.h/.cpp` | `CDlgSubRecipe` | 레시피 탭 (WindowEventHandler include) |
| `DlgSubAlarm.h/.cpp` | `CDlgSubAlarm` | 알람 탭 |
| `DlgSubConfig.h/.cpp` | `CDlgSubConfig` | 설정 탭 (WindowEventHandler include) |
| `DlgSubLog.h/.cpp` | `CDlgSubLog` | 로그 탭 (WindowEventHandler include) |
| `DlgSubData.h/.cpp` | `CDlgSubData` | 데이터 탭 |
| `DlgSubManual.h/.cpp` | `CDlgSubManual` | 수동 조작 탭 |
| `DlgAlignCCD.h/.cpp` | `CDlgAlignCCD` | CCD 카메라 뷰 |

### Sub-Sub Dialogs (탭 내부 페이지)
| File | Parent |
|------|--------|
| `DlgSubMaint_MotionFile.h/.cpp` | DlgSubMaint |
| `DlgSubMaint_IOFile.h/.cpp` | DlgSubMaint |
| `DlgSubMaint_ETCFile.h/.cpp` | DlgSubMaint |
| `DlgSubMeasure_Progress.h/.cpp` | DlgSubMaint |
| `DlgSubConfigMotionFile.h/.cpp` | DlgSubConfig |
| `DlgSubConfigModuleFile.h/.cpp` | DlgSubConfig |
| `DlgSubConfigLoginFile.h/.cpp` | DlgSubConfig |
| `DlgSubLogFile.h/.cpp` | DlgSubLog |
| `DlgSubManual_STEP.h/.cpp` | DlgSubManual |
| `DlgSubManual_Meas.h/.cpp` | DlgSubManual |
| `DlgSubManual_Ionizer.h/.cpp` | DlgSubManual |
| `DlgSubManual_AutoContact.h/.cpp` | DlgSubManual |
| `DlgSubManual_AgingTest.h/.cpp` | DlgSubManual |
| `DlgSubManual_AgingSet.h/.cpp` | DlgSubManual |

### Popup Dialogs
| File | Class |
|------|-------|
| `CD_PopUp_Alarm_F.h/.cpp` | 알람 팝업 |
| `CD_PopUp_Motion_Move.h/.cpp` | 모션 이동 팝업 |
| `D_MeasureCycle.h/.cpp` | 측정 사이클 다이얼로그 |
| `D_MoveHomePosition.h/.cpp` | 홈 위치 이동 다이얼로그 |
| `D_MainOperateInitializing.h/.cpp` | 초기화 진행 다이얼로그 |
| `D_PM_MODE.h/.cpp` | PM 모드 다이얼로그 |
| `D_SystemHostMode.h/.cpp` | 시스템 호스트 모드 |
| `D_RoiSet.h/.cpp` | ROI 설정 |
| `D_DisplayMessage.h` | 메시지 표시 |
| `UIMeasPathDlg.h` | 측정 경로 UI |

### WindowEventHandler Consumers
`WindowEventHandler.h`는 pch.h에도 include되어 있으며, 다음 파일에서 직접 include:
- `DlgSubConfig.h`, `DlgSubLog.h`, `DlgSubMaint.h`, `DlgSubOperate.h`, `DlgSubRecipe.h`
- `Recipe/RecipeData.h`
- `DoWork/DoWorkGlassUnloading.cpp`, `DoWorkGlassUnloadingStep.cpp`, `DoWorkGlassUnloadStep.cpp`

### AuroraMgrPool.h (UI → Manager 브릿지)
매니저 포인터 풀 - 다음에서 직접 include:
- 모든 `CommTcpIpObject*.h` (9개)
- `Alarm/AlarmMonitorImpl.h`, `AlarmMonitorMgr.h`
- `BaseSequenceBody.h`
- `AuroraMgr.h`
- `CD_PopUp_Alarm_F.cpp`

---

## 9. Shared Memory 수정

공유 메모리 구조체, 공유 메모리 매니저 변경 시 영향 범위. [See 10_GlobalState.md](10_GlobalState.md)

### Core Files (직접 수정 대상)
| File | Class/Role |
|------|-----------|
| `Include/ShareMem.h` | VTM_SYSTEM 공유 메모리 구조체 (DefinesSys.h include) |
| `Include/DefinesSys.h` | EQUIP_STATE, PROCESS_STATUS 등 enum 정의 (ShareMem.h include) |
| `CoreFunction/ShareMemFunction/` | 공유 메모리 기능 라이브러리 |
| `Manager/ShareMemoryMgr.h/.cpp` | `CShareMemoryMgr` - 공유 메모리 매니저 (ShareMemFunction.h include) |

### Affected: Application Init
| File | Dependency |
|------|-----------|
| `ALPHA.cpp` | `#include ShareMemoryMgr.h` - 앱 초기화 시 공유 메모리 생성 |

### Affected: Sequences (DoWork/)
| File | Dependency |
|------|-----------|
| `DoWork/SystemInitialize.cpp` | `#include ShareMemoryMgr.h` |
| `DoWork/SystemInitializeStep.cpp` | `#include ShareMemoryMgr.h` |
| `DoWork/DoWorkGlassUnloadStep.cpp` | `#include ShareMemoryMgr.h` |
| `DoWork/DoWorkSequenceCIMStep.cpp` | `#include ShareMemoryMgr.h` |

### Affected: FunctionModule
| File | Dependency |
|------|-----------|
| `FunctionModule/FunctionModuleEQInterlock.cpp` | `#include ShareMemoryMgr.h` |
| `FunctionModule/FunctionModuleEQState.cpp` | `#include ShareMemoryMgr.h` |
| `FunctionModule/FunctionModuleEQStateChangeStep.cpp` | `#include ShareMemoryMgr.h` |
| `FunctionModule/FunctionModuleProbeACC.cpp` | `#include ShareMemoryMgr.h` |
| `FunctionModule/FunctionModuleProbeACCStep.cpp` | `#include ShareMemoryMgr.h` |

### Affected: TCP/IP Communication
| File | Dependency |
|------|-----------|
| `CommTcpIpObjectCIM.cpp` | `#include ShareMemoryMgr.h` |
| `CommTcpIpObjectPG.cpp` | `#include ShareMemoryMgr.h` |
| `CommTcpIpObjectAgingPG.cpp` | `#include ShareMemoryMgr.h` |
| `CommTcpIpObjectWAD.cpp` | `#include ShareMemoryMgr.h` |

### Affected: UI Dialogs
| File | Dependency |
|------|-----------|
| `DlgMainForm.cpp` | `#include ShareMemoryMgr.h` |
| `DlgSubAlarm.cpp` | `#include ShareMemoryMgr.h` |
| `DlgSubMaint_MotionFile.cpp` | `#include ShareMemoryMgr.h` |
| `D_MainOperateInitializing.cpp` | `#include ShareMemoryMgr.h` |
| `D_SystemHostMode.cpp` | `#include ShareMemoryMgr.h` |
| `CD_PopUp_Motion_Move.cpp` | `#include ShareMemoryMgr.h` |

### Affected: Alarm Monitors
| File | Dependency |
|------|-----------|
| `Alarm/AlarmMonitorToLampTime.cpp` | `#include ShareMemoryMgr.h` |
| `Alarm/AlarmMonitorToSensorTemperatureLow.cpp` | `#include ShareMemoryMgr.h` |

### Affected: Other (간접 참조)
| File | Dependency |
|------|-----------|
| `Manager/AlignCameraMgr.h` | `#include ShareMem.h` - 카메라 매니저에서 공유 메모리 직접 참조 |
| `Recipe/RecipeData.h` | `#include ShareMemoryMgr.h` - 레시피 데이터에서 참조 |
| `D_RoiSet.h` | `#include ShareMem.h` |
| `UIMeasPathDlg.h` | `#include DefinesSys.h` |
| `Include/API_RECIPE.h` | `#include RecipeShareMem.h` |

### Affected: Orchestrator
| File | Dependency |
|------|-----------|
| `AuroraMgr.cpp` | `#include ShareMemoryMgr.h` |

> **주의:** `ShareMem.h` 또는 `DefinesSys.h` 구조체 수정 시 DLL 경계 호환성 확인 필요. 구조체 끝에만 멤버 추가.

---

## 10. Config/INI 수정

시스템 설정, INI 파일 관리, 설정 매니저 변경 시 영향 범위.

### Core Files (직접 수정 대상)
| File | Class/Role |
|------|-----------|
| `Manager/SystemConfigMgr.h/.cpp` | `CSystemConfigMgr` - 시스템 설정 매니저 (INI 파일 로드/저장) |
| `Manager/NetworkConfigMgr.h/.cpp` | `CNetworkConfigMgr` - 네트워크 설정 |
| `Manager/PositionConfigMgr.h/.cpp` | `CPositionConfigMgr` - 위치 설정 |
| `CoreFunction/common/IniHelper.h` | INI 파일 유틸리티 |

### Affected: TCP/IP Communication (모든 장비가 설정 참조 - 10개)
| File | Dependency |
|------|-----------|
| `CommTcpIpObjectCIM.cpp` | `#include SystemConfigMgr.h` |
| `CommTcpIpObjectPG.cpp` | `#include SystemConfigMgr.h` |
| `CommTcpIpObjectAgingPG.cpp` | `#include SystemConfigMgr.h` |
| `CommTcpIpObjectColor.cpp` | `#include SystemConfigMgr.h` |
| `CommTcpIpObjectAngle.cpp` | `#include SystemConfigMgr.h` |
| `CommTcpIpObjectLCR.cpp` | `#include SystemConfigMgr.h` |
| `CommTcpIpObjectTSP.cpp` | `#include SystemConfigMgr.h` |
| `CommTcpIpObjectVision.cpp` | `#include SystemConfigMgr.h` |
| `CommTcpIpObjectWAD.cpp` | `#include SystemConfigMgr.h` |

### Affected: Alarm Monitors (거의 전부 - 17개)
| File |
|------|
| `Alarm/AlarmMonitorToDoorVoice.cpp` |
| `Alarm/AlarmMonitorToDoorOpenState.cpp` |
| `Alarm/AlarmMonitorToEmergencyButton.cpp` |
| `Alarm/AlarmMonitorToLampTime.cpp` |
| `Alarm/AlarmMonitorToMcOFFState.cpp` |
| `Alarm/AlarmMonitorToSafetyPLC.cpp` |
| `Alarm/AlarmMonitorToSensorCurtain.cpp` |
| `Alarm/AlarmMonitorToSensorFAN.cpp` |
| `Alarm/AlarmMonitorToSensorHandSwitch.cpp` |
| `Alarm/AlarmMonitorToSensorIsolater.cpp` |
| `Alarm/AlarmMonitorToSensorPressure.cpp` |
| `Alarm/AlarmMonitorToSensorSmoke.cpp` |
| `Alarm/AlarmMonitorToSensorTemperature.cpp` |
| `Alarm/AlarmMonitorToSensorTemperatureLow.cpp` |
| `Alarm/AlarmMonitorToSensorVacSwitch.cpp` |
| `Alarm/AlarmMonitorToTeachKEY.cpp` |
| `Alarm/AlarmMonitorToTeachKEYLock.cpp` |

### Affected: Sequences (DoWork/)
| File | Dependency |
|------|-----------|
| `DoWork/DoWorkGlassAdsorptionStep.cpp` | `#include SystemConfigMgr.h` |
| `DoWork/DoWorkGlassAGINGStep.cpp` | `#include SystemConfigMgr.h` |
| `DoWork/DoWorkGlassMeasureStep.cpp` | `#include SystemConfigMgr.h` |
| `DoWork/DoWorkGlassPreMeasStep.cpp` | `#include SystemConfigMgr.h` |
| `DoWork/DoWorkGlassReadyStep.cpp` | `#include SystemConfigMgr.h` |
| `DoWork/DoWorkGlassResultStep.cpp` | `#include SystemConfigMgr.h` |
| `DoWork/DoWorkGlassSWAlignStep.cpp` | `#include SystemConfigMgr.h` |
| `DoWork/DoWorkGlassUnloadStep.cpp` | `#include SystemConfigMgr.h` |
| `DoWork/DoWorkGlassUnloadingStep.cpp` | `#include SystemConfigMgr.h` |
| `DoWork/DoWorkProbeSWAlignStep.cpp` | `#include SystemConfigMgr.h` |

### Affected: FunctionModule
| File | Dependency |
|------|-----------|
| `FunctionModule/FunctionModuleEQStateChange.cpp` | `#include SystemConfigMgr.h` |
| `FunctionModule/FunctionModuleEQStateChangeStep.cpp` | `#include SystemConfigMgr.h` |
| `FunctionModule/FunctionModuleProbeACC.cpp` | `#include SystemConfigMgr.h` |
| `FunctionModule/FunctionModuleProbeACCStep.cpp` | `#include SystemConfigMgr.h` |

### Affected: UI Dialogs
| File | Dependency |
|------|-----------|
| `ALPHA.cpp` | `#include SystemConfigMgr.h` - 앱 시작 시 설정 로드 |
| `DlgSubConfig.cpp` | `#include SystemConfigMgr.h` |
| `DlgSubConfigMotionFile.cpp` | `#include SystemConfigMgr.h` |
| `DlgSubAlarm.cpp` | `#include SystemConfigMgr.h` |
| `DlgSubOperate.cpp` | `#include SystemConfigMgr.h, IniHelper.h` |
| `DlgSubManual_Meas.cpp` | `#include SystemConfigMgr.h` |
| `D_MoveHomePosition.cpp` | `#include SystemConfigMgr.h` |

### Affected: Recipe
| File | Dependency |
|------|-----------|
| `Recipe/RecipeData.cpp` | `#include SystemConfigMgr.h` |

### Affected: Orchestrator
| File | Dependency |
|------|-----------|
| `AuroraMgr.cpp` | `#include SystemConfigMgr.h` |

---

## Cross-Subsystem Impact Matrix

어떤 서브시스템 수정이 다른 서브시스템에 영향을 미치는지 요약:

| Modified \ Affected | Motion | PLC | Vision | Recipe | Alarm | Sequence | CommTCP | UI | SharedMem | Config |
|---------------------|:------:|:---:|:------:|:------:|:-----:|:--------:|:-------:|:--:|:---------:|:------:|
| **1. Motion** | **X** | - | - | - | O | O | O | O | - | - |
| **2. PLC I/O** | - | **X** | - | - | O | O | - | O | - | - |
| **3. Vision** | - | - | **X** | O | - | O | O | O | - | - |
| **4. Recipe** | - | - | O | **X** | - | O | O | O | O | - |
| **5. Alarm** | - | O | - | - | **X** | - | O | O | - | - |
| **6. Sequence** | O | O | O | O | O | **X** | O | O | O | O |
| **7. CommTCP** | O | - | O | O | O | O | **X** | O | O | O |
| **8. UI** | - | - | - | - | - | - | - | **X** | - | - |
| **9. SharedMem** | - | - | O | O | O | O | O | O | **X** | - |
| **10. Config** | - | - | - | O | O | O | O | O | - | **X** |

**X** = 자기 자신, **O** = 영향 있음

### 가장 영향 범위가 큰 서브시스템 (수정 위험도 순)

1. **Config/INI 수정** (SystemConfigMgr) - 50+ 파일 참조 (전 서브시스템)
2. **Auto Sequence 수정** (SequenceManager) - 42+ 파일 참조
3. **Alarm 수정** (AlarmIDList.h in pch.h) - 전체 리빌드 유발
4. **Recipe 수정** (RecipeEditMgr) - 34+ 파일 참조 (모든 CommTcpIpObject + 대부분 DoWork)
5. **PLC I/O 수정** (IOMapAddressDefine) - 22+ 알람 모니터 + UI + 시퀀스
6. **Shared Memory 수정** - 30+ 파일 (DLL 경계 주의)
7. **Communication 수정** - 각 장비 객체는 독립적이나 팩토리 수정은 광범위
8. **Motion 수정** - MaintControlMgr 경유로 폭넓은 영향
9. **Vision 수정** - AlignCameraMgr 중심으로 비교적 제한적
10. **UI 수정** - 대부분 독립적 (WindowEventHandler 제외)

---

## pch.h Global Includes (전체 프로젝트 영향)

`pch.h`에서 전역 include하는 파일들 (수정 시 전체 리빌드):

| Include | Impact |
|---------|--------|
| `AlarmIDList.h` | 알람 ID 변경 시 전체 리빌드 |
| `WindowEventHandler.h` | 이벤트 핸들러 인터페이스 변경 시 전체 리빌드 |
| `LogWrapper.h` | 로그 인터페이스 변경 시 전체 리빌드 |

---

## CoreFunction/ Library Dependencies

외부 라이브러리(CoreFunction/) 수정 시 영향 범위:

| CoreFunction Module | Consuming Manager | Affected Subsystems |
|---------------------|-------------------|---------------------|
| `MotorFunction/` | MotorControlMgr | Motion, Sequence, UI |
| `IOMapFunction/` | IOMapControlMgr | PLC, Alarm, Sequence, UI |
| `FrameGrabberFunction/` | AlignCameraMgr | Vision, Sequence |
| `GlassAlignFunction/` | (via CoreFunction.h) | Vision, Sequence |
| `PatternMatchFunction/` | (via CoreFunction.h) | Vision, Sequence |
| `CommEthFunction/` | CommTcpIpFactoryMgr | Communication, all CommTcpIpObjects |
| `ShareMemFunction/` | ShareMemoryMgr | SharedMemory, all consumers |
| `LogFunction/` | (via pch.h) | **ALL** (전체 리빌드) |
| `dll_Alarm/` | AlarmController | Alarm |
| `dll_Recipe/` | RecipeEditMgr | Recipe |
| `common/IniHelper.h` | DlgSubOperate, FTP_Link | Config, UI |
| `common/MySpreadDll.h` | DlgSubManual, DlgSubMeasure | UI |
| `CIMFunction/` | CommTcpIpObjectCIM | Communication |
| `dMCCSpec/` | MCCLogMgr | Logging |
| `dSACC/` | SACCMgr | Unknown |
| `dResult/` | ResultMgr | Result data |
| `dProbeHistory/` | Unknown | Unknown |
