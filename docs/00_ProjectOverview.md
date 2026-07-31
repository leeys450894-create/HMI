# ALPHA HMI Project Overview

> ALPHA HMI - Samsung 디스플레이 Glass 검사 자동화 설비 제어 소프트웨어  
> MFC C++ Dialog-based Application (VS2019, Unicode, x64)

---

## Table of Contents

- [1. Solution 및 Project 구조](#1-solution-및-project-구조)
- [2. 폴더 구조](#2-폴더-구조)
- [3. 레이어 구조](#3-레이어-구조)
- [4. Project Dependencies](#4-project-dependencies)
- [5. DLL Dependencies](#5-dll-dependencies)
- [6. Major Namespaces / Enums](#6-major-namespaces--enums)
- [7. Entry Point](#7-entry-point)
- [8. Main Loop](#8-main-loop)

---

## 1. Solution 및 Project 구조

### Solution 파일

| 항목 | 값 |
|------|-----|
| Solution 파일 | `Source/ALPHA/ALPHA.sln` |
| VS Format | Visual Studio 2019 (Format Version 12.00, VS Version 16) |
| 소스 관리 | AnkhSVN2019 (Subversion) |
| Build Configurations | Debug\|x64, Release\|x64, Template\|x64 (+ x86 variants) |
| 주 빌드 타겟 | **x64** |

### Solution 내 Projects (7개)

| # | Project 이름 | GUID | 유형 | 위치 |
|---|-------------|------|------|------|
| 1 | **ALPHA** (Main EXE) | `{77FABB73-48FD-4EFA-815F-07161FB403F2}` | MFC EXE | `Source/ALPHA/ALPHA.vcxproj` |
| 2 | **Alarm** | `{A9C68540-A8FB-DE8E-E723-CD3EB6C7DE24}` | DLL | `CoreFunction/dll_Alarm/Alarm/Alarm.vcxproj` |
| 3 | **dSACC** | `{F1FCE3CA-A26F-4B25-A166-FBFB982D798F}` | DLL | `CoreFunction/dSACC/dSACC.vcxproj` |
| 4 | **dResult** | `{75B8C15E-3B18-4793-A7DB-D774B6965BE0}` | DLL | `CoreFunction/dResult/dResult.vcxproj` |
| 5 | **dfcRecipe** | `{D9CB0C22-4C81-49E7-9099-201F9046122A}` | DLL | `CoreFunction/dll_Recipe/dfcRecipe/dfcRecipe.vcxproj` |
| 6 | **dMCCSpec** | `{6CFC9A85-FB59-950E-C227-B04B43E5F6BD}` | DLL | `CoreFunction/dMCCSpec/dMCCSpec.vcxproj` |
| 7 | **ProbeHistory** | `{D20A313E-E99A-4247-BCC7-2314AF3680D3}` | DLL | `CoreFunction/dProbeHistory/ProbeHistory.vcxproj` |

### Solution Items

- `VS6.0 Temporarily File Cleaner.bat` - 임시 파일 정리 배치

---

## 2. 폴더 구조

```
HMI/
├── Source/
│   ├── ALPHA/                          # 메인 애플리케이션 소스
│   │   ├── ALPHA.cpp/.h                # CWinApp 파생 클래스 (Entry Point)
│   │   ├── DlgALPHA.cpp/.h             # 메인 대화상자 (CDlgALPHA : CDialogEx)
│   │   ├── DlgMainForm.cpp/.h          # 메인 폼 (CDlgMainForm : CD_AuroraDlgWin)
│   │   ├── DlgTitleBar.cpp/.h          # 타이틀바
│   │   ├── DlgNaviBar.cpp/.h           # 네비게이션 바
│   │   ├── AuroraMgr.cpp/.h            # 중앙 매니저 (CAuroraMgr)
│   │   ├── AuroraMgrPool.h             # 매니저 포인터 풀 구조체
│   │   ├── AuroraSubMgr.h              # SubMgr 기반 클래스
│   │   ├── AuroraFuncDefine.h          # 유틸리티 함수/구조체 정의
│   │   ├── SystemParamShare.cpp/.h     # 전역 파라미터 싱글톤
│   │   ├── WindowEventHandler.cpp/.h   # WM_USER 메시지 정의 및 핸들러
│   │   ├── BaseSequenceImpl.cpp/.h     # 시퀀스 기반 클래스 (Thread 포함)
│   │   ├── BaseSequenceBody.cpp/.h     # 시퀀스 상태/로그 관리
│   │   ├── pch.h                       # 미리 컴파일된 헤더 (MAIN_CCD_MAX_NUM=4 정의)
│   │   │
│   │   ├── Manager/                    # SubMgr 파생 매니저 클래스들
│   │   │   ├── SequenceManager.cpp/.h          # CSequenceManager (Singleton) - 시퀀스 총괄
│   │   │   ├── SystemConfigMgr.cpp/.h          # 시스템 설정
│   │   │   ├── NetworkConfigMgr.cpp/.h         # 네트워크 설정
│   │   │   ├── PositionConfigMgr.cpp/.h        # 위치 설정
│   │   │   ├── ShareMemoryMgr.cpp/.h           # 공유 메모리 관리
│   │   │   ├── IOMapControlMgr.cpp/.h          # I/O Map 제어
│   │   │   ├── MotorControlMgr.cpp/.h          # 모터 제어
│   │   │   ├── LightControlMgr.cpp/.h          # 조명 제어
│   │   │   ├── MaintControlMgr.cpp/.h          # 유지보수 제어
│   │   │   ├── CommTcpIpFactoryMgr.cpp/.h      # TCP/IP 장비 통신 Factory
│   │   │   ├── CommTcpIPMgr.cpp/.h             # TCP/IP 연결 관리
│   │   │   ├── CommTcpIpDefine.h               # TCP/IP 통신 상수/enum 정의
│   │   │   ├── AlignCameraMgr.cpp/.h           # Align 카메라 관리
│   │   │   ├── LogInControlMgr.cpp/.h          # 로그인 제어
│   │   │   ├── SeqTactTimeMgr.cpp/.h           # Tact Time 관리
│   │   │   ├── RecipeMgr.cpp/.h                # Recipe 관리
│   │   │   ├── ResultMgr.cpp/.h                # 결과 관리
│   │   │   ├── SACCMgr.cpp/.h                  # SACC 관리
│   │   │   ├── MCCLogMgr.cpp/.h                # MCC 로그 관리
│   │   │   ├── CCommTcpIpObjectMotion.cpp/.h   # Vision Motion 통신
│   │   │   └── CCommTcpIpObjectSysTime.cpp/.h  # 시간 동기화 통신
│   │   │
│   │   ├── DoWork/                     # 시퀀스 엔진/작업 단위
│   │   │   ├── SystemInitialize.cpp/.h             # 시스템 초기화 시퀀스
│   │   │   ├── SystemInitializeStep.cpp/.h         # 초기화 단계 정의
│   │   │   ├── DoWorkGlassSequenceEngine.cpp/.h    # Glass 시퀀스 엔진 (Thread)
│   │   │   ├── DoWorkGlassSequence.cpp/.h          # Glass 시퀀스 총괄
│   │   │   ├── DoWorkGlassSequenceImpl.cpp/.h      # Glass 시퀀스 구현
│   │   │   ├── DoWorkGlassReady.cpp/.h + Step      # Glass Ready 시퀀스
│   │   │   ├── DoWorkGlassRecipe.cpp/.h + Step     # Glass Recipe 변경
│   │   │   ├── DoWorkGlassSWAlign.cpp/.h + Step    # Glass SW Align
│   │   │   ├── DoWorkGlassPreMeas.cpp/.h + Step    # Glass 사전 측정
│   │   │   ├── DoWorkGlassAGING.cpp/.h + Step      # Glass Aging 테스트
│   │   │   ├── DoWorkGlassAdsorption.cpp/.h + Step # Glass 흡착
│   │   │   ├── DoWorkGlassMeasure.cpp/.h + Step    # Glass 측정
│   │   │   ├── DoWorkGlassResult.cpp/.h + Step     # Glass 결과 처리
│   │   │   ├── DoWorkGlassUnload.cpp/.h + Step     # Glass Unload
│   │   │   ├── DoWorkGlassUnloading.cpp/.h + Step  # Glass Unloading
│   │   │   ├── DoWorkProbeSWAlign.cpp/.h + Step    # Probe SW Align
│   │   │   ├── DoWorkSequenceCIM.cpp/.h + Step     # CIM 시퀀스
│   │   │   ├── MeasureSequenceItemEngine.cpp/.h    # 측정 시퀀스 아이템 엔진
│   │   │   ├── MeasureSequenceItemFactory.cpp/.h   # 측정 시퀀스 아이템 Factory
│   │   │   └── MeasureSequenceItemImpl.cpp/.h      # 측정 시퀀스 아이템 구현
│   │   │
│   │   ├── FunctionModule/             # 설비 상태/인터록 모듈
│   │   │   ├── FunctionModuleEQState.cpp/.h            # 설비 상태 머신
│   │   │   ├── FunctionModuleEQStateChange.cpp/.h      # 설비 상태 변경 처리
│   │   │   ├── FunctionModuleEQStateChangeStep.cpp/.h  # 상태 변경 단계
│   │   │   ├── FunctionModuleEQInterlock.cpp/.h        # 안전 인터록
│   │   │   ├── FunctionModuleProbeACC.cpp/.h + Step    # Probe Auto Contact Control
│   │   │   ├── FunctionModuleAFControl.h               # Auto Focus 제어 (헤더만)
│   │   │   └── FunctionModuleCentering.h               # Centering 제어 (헤더만)
│   │   │
│   │   ├── Alarm/                      # 알람 모니터링 (24+ 스레드)
│   │   │   ├── AlarmController.cpp/.h              # 알람 컨트롤러 (Singleton)
│   │   │   ├── AlarmMonitorMgr.cpp/.h              # 알람 모니터 매니저
│   │   │   ├── AlarmMonitorImpl.cpp/.h             # 알람 모니터 구현 기반
│   │   │   ├── AlarmMonitorToEmergency.cpp/.h      # 비상정지 모니터
│   │   │   ├── AlarmMonitorToEmergencyButton.cpp/.h # EMO 버튼 모니터
│   │   │   ├── AlarmMonitorToDoorOpen.cpp/.h       # 도어 열림 모니터
│   │   │   ├── AlarmMonitorToDoorOpenState.cpp/.h  # 도어 상태 모니터
│   │   │   ├── AlarmMonitorToDoorVoice.cpp/.h      # 도어 음성 경고
│   │   │   ├── AlarmMonitorToMcOFF.cpp/.h          # MC OFF 모니터
│   │   │   ├── AlarmMonitorToMcOFFState.cpp/.h     # MC OFF 상태
│   │   │   ├── AlarmMonitorToSafetyPLC.cpp/.h      # Safety PLC 모니터
│   │   │   ├── AlarmMonitorToTeachKEY.cpp/.h       # Teach KEY 모니터
│   │   │   ├── AlarmMonitorToTeachKEYLock.cpp/.h   # Teach KEY Lock
│   │   │   ├── AlarmMonitorToLampTime.cpp/.h       # 램프 시간 모니터
│   │   │   ├── AlarmMonitorToSensor.cpp/.h         # 센서 모니터
│   │   │   ├── AlarmMonitorToSensorCurtain.cpp/.h  # 커튼 센서
│   │   │   ├── AlarmMonitorToSensorFAN.cpp/.h      # FAN 센서
│   │   │   ├── AlarmMonitorToSensorHandSwitch.cpp/.h # 핸드 스위치
│   │   │   ├── AlarmMonitorToSensorIsolater.cpp/.h # Isolator 센서
│   │   │   ├── AlarmMonitorToSensorPressure.cpp/.h # 압력 센서
│   │   │   ├── AlarmMonitorToSensorSmoke.cpp/.h    # 연기 감지
│   │   │   ├── AlarmMonitorToSensorTemperature.cpp/.h     # 고온 센서
│   │   │   ├── AlarmMonitorToSensorTemperatureLow.cpp/.h  # 저온 센서
│   │   │   └── AlarmMonitorToSensorVacSwitch.cpp/.h       # 진공 스위치
│   │   │
│   │   ├── Recipe/                     # Recipe 편집 UI/데이터
│   │   │   ├── RecipeEditMgr.cpp/.h    # Recipe 편집 매니저
│   │   │   └── RecipeData.cpp/.h       # Recipe 데이터 구조
│   │   │
│   │   ├── CommTcpIpObject*.cpp/.h     # TCP/IP 장비별 통신 객체 (루트에 배치)
│   │   │   # CommTcpIpObjectVision     - Vision 장비
│   │   │   # CommTcpIpObjectColor      - 색차계
│   │   │   # CommTcpIpObjectAngle      - 시야각 측정기
│   │   │   # CommTcpIpObjectWAD        - WAD 장비
│   │   │   # CommTcpIpObjectPG         - Pattern Generator
│   │   │   # CommTcpIpObjectAgingPG    - Aging PG
│   │   │   # CommTcpIpObjectLCR        - LCR 측정기
│   │   │   # CommTcpIpObjectTSP        - TSP (6개 채널)
│   │   │   # CommTcpIpObjectCIM        - CIM/MES 통신
│   │   │
│   │   ├── Dlg*.cpp/.h                 # Sub Dialog 화면들
│   │   │   # DlgSubOperate     - 운영 화면
│   │   │   # DlgSubMaint       - 유지보수 화면 (IO/Motion/ETC 탭)
│   │   │   # DlgSubRecipe      - 레시피 화면
│   │   │   # DlgSubAlarm       - 알람 화면
│   │   │   # DlgSubConfig      - 설정 화면 (Login/Module/Motion 탭)
│   │   │   # DlgSubLog         - 로그 화면
│   │   │   # DlgSubData        - 데이터 화면
│   │   │   # DlgSubManual      - 수동 제어 화면 (Meas/SACC/Ionizer/AutoContact/Aging)
│   │   │   # DlgSubMeasure_Progress - 측정 진행 화면
│   │   │   # DlgAlignCCD       - Align CCD 화면
│   │   │   # DlgSMarkUI        - S-Mark UI
│   │   │
│   │   ├── D_*.cpp/.h                  # Popup/보조 Dialog
│   │   │   # D_AuroraDlgWin           - Dialog 기반 윈도우 베이스 클래스
│   │   │   # D_MainOperateInitializing - 초기화 진행 다이얼로그
│   │   │   # D_DisplayMessage         - 메시지 표시
│   │   │   # D_InputPassWord          - 비밀번호 입력
│   │   │   # D_LogInMgr               - 로그인 UI
│   │   │   # D_MeasureCycle           - 측정 사이클
│   │   │   # D_MoveHomePosition       - 홈 이동
│   │   │   # D_RoiSet                 - ROI 설정
│   │   │   # D_CCD_Review             - CCD 리뷰
│   │   │   # D_SelectMeasOrUnload     - 측정/언로드 선택
│   │   │   # D_SetLampTime            - 램프 시간 설정
│   │   │   # D_SystemHostMode         - 시스템 호스트 모드
│   │   │   # D_PM_MODE                - PM 모드
│   │   │   # CD_PopUp_Alarm_F         - 알람 팝업
│   │   │   # CD_PopUp_Motion_Move     - 모션 이동 팝업
│   │   │   # CD_SelectColor           - 색상 선택
│   │   │
│   │   ├── FTP_Link_CIM.cpp/.h        # FTP CIM 연동
│   │   ├── FTP_Link_Interface.cpp/.h   # FTP 인터페이스
│   │   ├── TransporterResult.cpp/.h    # Transporter 결과
│   │   ├── UIMeasPathDlg.cpp/.h        # 측정 경로 UI
│   │   ├── MainSubDlgWin.cpp/.h        # Sub Dialog 기반 클래스
│   │   ├── ConfigurationFile/          # 기본 설정 파일 (INI 등)
│   │   └── res/                        # 리소스 파일
│   │
│   ├── Include/                        # 공유 헤더 파일
│   │   ├── preDefine.h                 # 전역 정의, 구조체 (Recipe, Cell, Align 등)
│   │   ├── DefinesPLC.h                # PLC 비트 주소 맵 (1000+ 라인)
│   │   ├── DefinesSys.h                # EQUIP_STATE, PROCESS_STATUS enum
│   │   ├── DefinesCIM.h                # CIM/MES 프로토콜 명령
│   │   ├── ShareMem.h                  # 공유 메모리 구조체
│   │   ├── AlarmIDList.h               # 알람 ID 목록
│   │   ├── AlarmDefine.h               # 알람 정의
│   │   ├── IOMapAddressDefine.h        # I/O Map 주소 정의
│   │   ├── FDCSheardDefine.h           # FDC 공유 정의
│   │   ├── RecipeShareMem.h            # Recipe 공유 메모리
│   │   ├── SheardDefineMCC.h           # MCC 공유 정의
│   │   ├── OutRecipeFunc.h             # 외부 Recipe 함수
│   │   └── API_*.h                     # DLL API 헤더 (Alarm, MCC, ProbeHistory, RECIPE, Result, SACC)
│   │
│   ├── Lib/                            # 링크 라이브러리 (.lib)
│   │   ├── olch2du8.lib                # OLECTRA Chart (차트 컴포넌트)
│   │   ├── Alarm.lib
│   │   ├── dMCCSpec.lib
│   │   ├── dSACC.lib
│   │   ├── ProbeHistory.lib
│   │   ├── dResult.lib
│   │   ├── dfcRecipe64.lib / dfcRecipe64d.lib
│   │   └── dfcGrid64.lib / dfcGrid64d.lib / dfcGridd.lib
│   │
│   └── Bin/                            # 빌드 출력 및 실행 환경
│       ├── ALPHA.exe                   # 메인 실행 파일
│       ├── *.dll / *.pdb               # DLL 및 디버그 심볼
│       ├── Config/                     # 런타임 설정 파일
│       ├── Recipe/                     # 레시피 파일
│       └── DefaultRcp/                 # 기본 레시피 (Hidden)
│
├── CoreFunction/                       # 독립 라이브러리 (재사용 가능)
│   ├── CoreFunction.h                  # 전체 CoreFunction include 헤더
│   ├── common/                         # 공통 유틸리티
│   │   ├── CommonFunc.cpp/.h           # 전역 함수, CUR_EQU_TYPE 정의
│   │   ├── IniHelper.cpp/.h            # INI 파일 헬퍼
│   │   ├── BinaryHelper.cpp/.h         # 바이너리 데이터 헬퍼
│   │   ├── RawDataHelper.cpp/.h        # Raw 데이터 헬퍼
│   │   ├── TimeChecker.cpp/.h          # 시간 측정 유틸
│   │   ├── Label.cpp/.h                # MFC Label 컨트롤
│   │   ├── BtnST.cpp/.h               # 커스텀 버튼
│   │   ├── PingTest.cpp/.h             # Ping 테스트
│   │   ├── Registry.cpp/.h             # 레지스트리 접근
│   │   ├── crc16.cpp/.h                # CRC16 계산
│   │   ├── XStringHelper.cpp/.h        # 문자열 헬퍼
│   │   ├── MFCGridControl/             # MFC Grid 컨트롤
│   │   ├── FarPoint64/                 # FarPoint Spread 컨트롤
│   │   ├── Chart/                      # 차트 관련
│   │   ├── XButtonXP/                  # XP 스타일 버튼
│   │   ├── XLS/                        # Excel 관련
│   │   ├── NI/                         # National Instruments
│   │   └── IMAQImageCtrl/              # IMAQ 이미지 컨트롤
│   │
│   ├── MotorFunction/                  # 모터 제어 라이브러리
│   │   ├── MotorFunction.cpp/.h        # 모터 기능 인터페이스
│   │   ├── MotorImpl.cpp/.h            # 모터 구현
│   │   ├── AxisType.cpp/.h             # 축 타입 정의
│   │   ├── MotorCtrl_ACS/              # ACS SPiiPlus 모터 제어
│   │   ├── MotorCtrl_COMIZOA/          # COMIZOA 모터 제어
│   │   └── MotorCtrl_MEI/              # MEI 모터 제어
│   │
│   ├── IOMapFunction/                  # I/O 맵 (PLC 통신)
│   │   ├── IOMapFunction.cpp/.h        # I/O Map 인터페이스
│   │   ├── IOMapImpl.cpp/.h            # I/O Map 구현
│   │   ├── DataStructure.h             # 데이터 구조
│   │   └── McProtocol/                 # Mitsubishi MC Protocol
│   │       ├── McProtocolManager.cpp/.h    # MC Protocol 매니저
│   │       ├── McMaddressScanner.cpp/.h    # M address 스캐너
│   │       ├── McDaddressScanner.cpp/.h    # D address 스캐너
│   │       ├── McFaddressScanner.cpp/.h    # F address 스캐너
│   │       ├── McGaddressScanner.cpp/.h    # G address 스캐너
│   │       ├── McPaddressScanner.cpp/.h    # P address 스캐너
│   │       ├── McCaddressScanner.cpp/.h    # C address 스캐너
│   │       └── McAddressScannerImpl.cpp/.h # 스캐너 구현 기반
│   │
│   ├── CommEthFunction/                # Ethernet TCP/IP 통신
│   │   ├── CommEthFunction.cpp/.h      # Ethernet 기능 인터페이스
│   │   ├── EthernetManagerTCP.cpp/.h   # TCP 매니저
│   │   ├── ClientSocket.cpp/.h         # 클라이언트 소켓
│   │   ├── ServerSocket.cpp/.h         # 서버 소켓
│   │   └── EthernetSocket.cpp/.h       # 소켓 기반
│   │
│   ├── Comm232Function/                # RS-232 시리얼 통신
│   │   ├── Comm232Function.cpp/.h
│   │   └── Comm232Config.cpp/.h
│   │
│   ├── CommFtpFunction/                # FTP 통신
│   │   ├── CommFtpFunction.cpp/.h
│   │   └── EthernetManagerFTP.cpp/.h
│   │
│   ├── CommUdpFunction/                # UDP 통신 (헤더만)
│   │
│   ├── CIMFunction/                    # CIM/MES 통신 (헤더만)
│   │
│   ├── FrameGrabberFunction/           # 카메라 Frame Grabber
│   │   ├── FrameGrabberFunction.cpp/.h
│   │   ├── FrameGrabberImpl.cpp/.h
│   │   └── GigECamera_Basler/          # Basler GigE 카메라
│   │
│   ├── GlassAlignFunction/             # Glass Align 알고리즘 (헤더만)
│   ├── PatternMatchFunction/           # 패턴 매칭 (헤더만)
│   ├── LightFunction/                  # 조명 제어
│   │   ├── LightFunction.cpp/.h
│   │   ├── LightImpl.cpp/.h
│   │   ├── CAS/ / ECS/ / SPO/ / XCITE/ # 조명 제조사별 구현
│   │
│   ├── LogFunction/                    # 로깅
│   │   ├── LogWrapper/                 # 로그 래퍼 (spdlog 기반)
│   │   └── LogAccessor/               # 로그 파일 접근
│   │
│   ├── ShareMemFunction/              # 공유 메모리
│   │   ├── ShareMemFunction.cpp/.h
│   │   └── CPtrnInfo.cpp/.h
│   │
│   ├── SEMFunction/                    # SEM 제어
│   ├── VCRFunction/                    # VCR 제어 (헤더만)
│   │
│   ├── dSACC/                          # SACC DLL (Short-circuit Auto Contact Check)
│   ├── dResult/                        # Result DLL (결과 보고서)
│   ├── dMCCSpec/                       # MCC Spec DLL (MCC 사양 관리)
│   ├── dProbeHistory/                  # Probe History DLL (프로브 이력)
│   ├── dll_Alarm/                      # Alarm DLL
│   └── dll_Recipe/                     # Recipe DLL
│
└── Doc/                                # 문서
```

---

## 3. 레이어 구조

```
┌─────────────────────────────────────────────────────────────────────┐
│                        UI Layer (Dialog-based)                       │
│  CDlgALPHA → CDlgMainForm → CDlgNaviBar                            │
│  DlgSub* (Operate/Maint/Recipe/Alarm/Config/Log/Data/Manual)       │
│  D_* Popup Dialogs, DlgAlignCCD, CD_PopUp_*                        │
│  Base: CD_AuroraDlgWin (CDialogEx), CMainSubDlgWin (CDialogEx)    │
├─────────────────────────────────────────────────────────────────────┤
│                    Manager Layer (CAuroraMgr)                        │
│  stAuroraMgrPool: 27+ Manager 포인터 보유                            │
│  CAuroraSubMgr: Initialize/LinkIniFile/SetDefault/Finalize 인터페이스 │
│  ┌──────────────────┬─────────────────┬─────────────────────────┐   │
│  │ Config 계열       │ Control 계열     │ Communication 계열      │   │
│  │ SystemConfigMgr   │ IOMapControlMgr  │ CommTcpIpFactoryMgr    │   │
│  │ NetworkConfigMgr  │ MotorControlMgr  │ CommTcpIPMgr           │   │
│  │ PositionConfigMgr │ LightControlMgr  │ (13종 CommTcpIpObject) │   │
│  │ VisionConfigMgr   │ MaintControlMgr  │                        │   │
│  │ LogInControlMgr   │ AlignCameraMgr[4]│                        │   │
│  └──────────────────┴─────────────────┴─────────────────────────┘   │
│  Singleton: CSystemParamShare, CWindowEventHandler, CAlarmController│
├─────────────────────────────────────────────────────────────────────┤
│                   Sequence Layer (Thread-based)                       │
│  CSequenceManager (Singleton) ─ 시퀀스 총괄                          │
│  ├── CSystemInitialize         ─ 시스템 초기화 (22단계)               │
│  ├── CDoWorkGlassSequence      ─ Glass 시퀀스                       │
│  │   └── CDoWorkGlassSequenceEngine (9-step Glass 공정)             │
│  │       ├── DoWorkGlassReady       (대기)                           │
│  │       ├── DoWorkGlassRecipe      (Recipe 변경)                    │
│  │       ├── DoWorkGlassSWAlign     (Glass SW Align)                │
│  │       ├── DoWorkGlassPreMeas     (사전 측정)                      │
│  │       ├── DoWorkGlassAGING       (Aging 테스트)                   │
│  │       ├── DoWorkGlassAdsorption  (흡착)                           │
│  │       ├── DoWorkGlassMeasure     (본 측정)                        │
│  │       ├── DoWorkGlassResult      (결과 처리)                      │
│  │       └── DoWorkGlassUnload      (Unload)                        │
│  ├── CFunctionModuleProbeACC   ─ Probe Auto Contact                 │
│  ├── CFunctionModuleEQStateChange ─ 설비 상태 전이                    │
│  ├── CFunctionModuleEQState    ─ 설비 상태 머신                      │
│  ├── CFunctionModuleAFControl  ─ Auto Focus 제어                     │
│  ├── CFunctionModuleCentering  ─ Centering 제어                      │
│  ├── DoWorkProbeSWAlign        ─ Probe SW Align                     │
│  └── DoWorkSequenceCIM         ─ CIM 시퀀스                         │
│  Base: CBaseSequenceImpl → CBaseSequenceBody (Thread + State)       │
├─────────────────────────────────────────────────────────────────────┤
│                   Alarm Layer (개별 Thread 24+)                      │
│  CAlarmMonitorMgr ─ 모니터 매니저                                    │
│  CAlarmController ─ 알람 발생/해제 싱글톤                             │
│  AlarmMonitorImpl ─ 모니터 기반 클래스                                │
│  └── 24+ 개별 AlarmMonitorTo* 클래스 (각각 독립 Thread)              │
├─────────────────────────────────────────────────────────────────────┤
│                   CoreFunction Layer (재사용 라이브러리)               │
│  MotorFunction    ─ ACS SPiiPlus/COMIZOA/MEI 모터 제어               │
│  IOMapFunction    ─ Mitsubishi MC Protocol PLC 통신                  │
│  CommEthFunction  ─ TCP/IP 소켓 통신                                 │
│  Comm232Function  ─ RS-232 시리얼 통신                               │
│  CommFtpFunction  ─ FTP 파일 전송                                    │
│  FrameGrabber     ─ Basler GigE 카메라 Grab                         │
│  LightFunction    ─ CAS/ECS/SPO/XCITE 조명 제어                     │
│  ShareMemFunction ─ 프로세스 간 공유 메모리                           │
│  LogFunction      ─ spdlog 기반 로그 시스템                          │
├─────────────────────────────────────────────────────────────────────┤
│                   DLL Layer (독립 빌드 DLL)                           │
│  Alarm.dll        ─ 알람 관리 UI/기능                                │
│  dSACC.dll        ─ SACC 보고서/관리                                 │
│  dResult.dll      ─ 측정 결과 보고서                                 │
│  dMCCSpec.dll     ─ MCC 사양 보고서                                  │
│  ProbeHistory.dll ─ 프로브 이력 관리                                 │
│  dfcRecipe64.dll  ─ Recipe 데이터 관리                               │
│  dfcGrid64.dll    ─ Grid UI 컨트롤                                   │
│  olch2du8 (SPR64DU80.DLL) ─ OLECTRA Chart 컨트롤                   │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 4. Project Dependencies

### ALPHA.exe 빌드 의존성 (.sln ProjectDependencies)

```
ALPHA (Main EXE)
├── dSACC         (SACC DLL)
├── dfcRecipe     (Recipe DLL)
├── Alarm         (Alarm DLL)
├── dResult       (Result DLL)
├── dMCCSpec      (MCC Spec DLL)
└── ProbeHistory  (Probe History DLL)
```

ALPHA 프로젝트는 위 6개 DLL 프로젝트에 빌드 의존성을 가진다. DLL들이 먼저 빌드된 후 ALPHA.exe가 빌드된다.

### 클래스 의존 관계 (Manager 계층)

```
CALPHAApp (theApp)
└── CAuroraMgr
    ├── stAuroraMgrPool (모든 Manager 포인터 보유)
    │   ├── CSystemConfigMgr        (설정 읽기, 최초 생성)
    │   ├── CLogInControlMgr
    │   ├── CAlignCameraMgr[4]      (MAIN_CCD_MAX_NUM=4)
    │   ├── CNetworkConfigMgr
    │   ├── CPositionConfigMgr
    │   ├── CRecipeEditMgr          (← stAuroraMgrPool 참조)
    │   ├── CShareMemoryMgr
    │   ├── CLightControlMgr
    │   ├── CIOMapControlMgr
    │   ├── CMotorControlMgr
    │   ├── CMaintControlMgr        (← Motor, IOMap, Position, Light, ShareMem 참조)
    │   ├── CSeqTactTimeMgr
    │   ├── CAlarmMonitorMgr        (← stAuroraMgrPool 참조)
    │   ├── CCommTcpIpFactoryMgr    (← stAuroraMgrPool 참조)
    │   └── CCommTcpIPMgr           (← stAuroraMgrPool 참조)
    │
    └── m_subMgr (vector<CAuroraSubMgr*>) - 16개 SubMgr 등록:
        [0] CommTcpIPMgr
        [1] AlarmMonitorMgr
        [2] CommTcpIpFactoryMgr
        [3] SeqTactTimeMgr
        [4] ShareMemoryMgr
        [5-8] AlignCameraMgr[0..3]
        [9] LightControlMgr
        [10] MaintControlMgr
        [11] RecipeEditMgr
        [12] PositionConfigMgr
        [13] NetworkConfigMgr
        [14] SystemConfigMgr
        [15] LogInControlMgr
```

---

## 5. DLL Dependencies

### 링크 라이브러리 (ALPHA.vcxproj AdditionalDependencies)

| Library | 설명 | 유형 |
|---------|------|------|
| `version.lib` | Windows Version API | System |
| `olch2du8.lib` | OLECTRA Chart 2D Unicode 8 | 3rd Party |
| `Alarm.lib` | 알람 DLL import lib | In-Solution |
| `dMCCSpec.lib` | MCC Spec DLL import lib | In-Solution |
| `dSACC.lib` | SACC DLL import lib | In-Solution |
| `ProbeHistory.lib` | Probe History DLL import lib | In-Solution |
| `dResult.lib` | Result DLL import lib | In-Solution |
| `dfcRecipe64d.lib` (Debug) / `dfcRecipe64.lib` (Release) | Recipe DLL import lib | In-Solution |

### 런타임 DLL (Source/Bin/)

| DLL | 설명 |
|-----|------|
| `ALPHA.exe` | 메인 실행 파일 |
| `Alarm.dll` | 알람 관리 |
| `dSACC.dll` | SACC 보고서 |
| `dResult.dll` | 결과 보고서 |
| `dMCCSpec.dll` | MCC 사양 관리 |
| `dMCCSpecRec.dll` | MCC Spec Recipe (별도 빌드) |
| `ProbeHistory.dll` | 프로브 이력 |
| `dfcRecipe64.dll` / `dfcRecipe64d.dll` | Recipe 관리 |
| `dfcGrid64.dll` / `dfcGrid64d.dll` | Grid 컨트롤 |
| `SPR64DU80.DLL` | OLECTRA Chart 런타임 |
| `ACSCL_x64.dll` | ACS SPiiPlus 모터 컨트롤러 라이브러리 |
| `FTP_Dll.dll` | FTP 통신 |
| `MCCLog.exe` | MCC 로그 별도 프로세스 |
| `dXCrashRpt.dll` + `XCrashRpt.exe` | 크래시 리포트 |
| `dbghelp.dll` | 디버그 헬퍼 |

---

## 6. Major Namespaces / Enums

HMI 프로젝트는 C++ namespace를 사용하지 않는다. 대신 enum과 #define으로 식별자를 구분한다.

### 주요 Enum 정의

| Enum | 파일 | 용도 |
|------|------|------|
| `enumNavigateID` | `SystemParamShare.h` | 네비게이션 화면 ID (LOGIN, OPERATE, MAINT 등 11개) |
| `enumDoWorkStateID` | `SystemParamShare.h` | 작업 상태 ID (Idle, Loading, Measure, Unloading 등 30+) |
| `eMessage` (WM_MSG_*) | `WindowEventHandler.h` | 커스텀 윈도우 메시지 (WM_USER+100~) |
| `enumSeqItemMessage` | `WindowEventHandler.h` | 시퀀스 완료 메시지 (WM_USER+300~) |
| `enumWndID` | `WindowEventHandler.h` | 윈도우 ID (TITLE, FRAME) |
| `enumSeqItemID` | `WindowEventHandler.h` | 시퀀스 아이템 ID (READY, LOADING, MEASURE 등) |
| `enumLogItemID` | `WindowEventHandler.h` | 로그 항목 ID (DLOG, TLOG, OLOG 등 20개) |
| `enumManualTemID` | `WindowEventHandler.h` | 수동 탭 ID (MEAS, SACC, IONIZER 등) |
| `enumTCPIpFactorytID_HMI` | `CommTcpIpDefine.h` | TCP/IP 장비 ID (VISION~CIM, 19개) |
| `enumRecvEventID` | `CommTcpIpDefine.h` | 수신 이벤트 ID (88개) |
| `EQUIP_TYPE_VALUE` | `CommonFunc.h` | 설비 타입 (ICE, ICM, ICP, OCP) |
| `CAD_ROW` | `preDefine.h` | CAD 행 정의 (A~Y, X1~X7, P1~P5) |

### 주요 #define 상수

| 상수 | 값 | 파일 | 용도 |
|------|-----|------|------|
| `MAIN_CCD_MAX_NUM` | 4 | `pch.h` | Align CCD 카메라 최대 수 |
| `MAIN_STATE_TIMER_ID` | 1001 | `pch.h` | 메인 상태 타이머 ID |
| `CUR_EQU_TYPE` | `EQU_TYPE_OCP` | `CommonFunc.h` | 현재 장비 타입 |
| `M_RCP_PROBE_MAX` | 5 | `preDefine.h` | Recipe 프로브 최대 수 |
| `DF_MAX_HEAD` | 2 | `CommTcpIpDefine.h` | 측정 헤드 수 |
| `CH_LEN` | 1024 | `preDefine.h` | 문자열 버퍼 크기 |

### Singleton 클래스

| 클래스 | 파일 | 용도 |
|--------|------|------|
| `CSystemParamShare` | `SystemParamShare.h` | 전역 경로/상태 파라미터 |
| `CWindowEventHandler` | `WindowEventHandler.h` | 윈도우 메시지 라우팅 |
| `CSequenceManager` | `Manager/SequenceManager.h` | 시퀀스 총괄 관리 |
| `CAlarmController` | `Alarm/AlarmController.h` | 알람 발생/해제 |
| `CLogWrapper` | `CoreFunction/LogFunction/LogWrapper/LogWrapper.h` | 로그 기록 |

---

## 7. Entry Point

### 진입 흐름

```
WinMain() [MFC 내부, afxwin.h]
  └── CALPHAApp::InitInstance()                     [ALPHA.cpp:47]
        │
        ├── 1. 중복 실행 방지                         [ALPHA.cpp:52-76]
        │   CreateToolhelp32Snapshot() → Process32First/Next
        │   동일 프로세스 2개 이상이면 AfxMessageBox 후 return FALSE
        │
        ├── 2. Common Controls 초기화                [ALPHA.cpp:82-87]
        │   InitCommonControlsEx(ICC_WIN95_CLASSES)
        │
        ├── 3. MFC 초기화                            [ALPHA.cpp:89-104]
        │   CWinApp::InitInstance()
        │   AfxSocketInit()                          # 소켓 초기화
        │   AfxEnableControlContainer()
        │   CShellManager 생성
        │   CMFCVisualManager 설정 (Windows 스타일)
        │
        ├── 4. 경로 설정                             [ALPHA.cpp:122-159]
        │   GetModuleFileName → _tsplitpath_s
        │   CSystemParamShare::GetInstance()->SetAppFolderPath()
        │   Config, DefaultRcp, Alarm 디렉토리 생성
        │
        ├── 5. CAuroraMgr 생성 (핵심)                [ALPHA.cpp:171]
        │   new CAuroraMgr(strSysName, strAppDir, strConfigPath, strRecipePath, strLogPath)
        │   └── CAuroraMgr 생성자에서:
        │       ├── timeBeginPeriod(1)               # 타이머 정밀도 1ms
        │       ├── CSystemConfigMgr 생성 (최초)
        │       ├── CLogWrapper 초기화 (spdlog)
        │       ├── CLogInControlMgr 생성
        │       ├── CAlignCameraMgr[4] 생성
        │       ├── CNetworkConfigMgr 생성
        │       ├── CPositionConfigMgr 생성
        │       ├── CRecipeEditMgr 생성
        │       ├── CShareMemoryMgr 생성
        │       ├── CLightControlMgr 생성
        │       ├── CIOMapControlMgr 생성
        │       ├── CMotorControlMgr 생성
        │       ├── CMaintControlMgr 생성
        │       ├── CSeqTactTimeMgr 생성
        │       ├── CAlarmMonitorMgr 생성
        │       ├── CCommTcpIpFactoryMgr 생성
        │       ├── CCommTcpIPMgr 생성
        │       └── m_subMgr에 16개 SubMgr 등록
        │
        ├── 6. 공유 메모리 초기화                     [ALPHA.cpp:173-175]
        │   GetShareMemoryMgrPtr()->GetSharedMemory()
        │   SetSys_MachineMode(df_HOST_OFFLINE)
        │   SetSys_MachineType(CUR_EQU_TYPE)        # EQU_TYPE_OCP
        │
        ├── 7. 모듈 정보 로깅 (크래시 보고용)         [ALPHA.cpp:177-194]
        │   CreateToolhelp32Snapshot(TH32CS_SNAPMODULE)
        │   모든 모듈 base address 기록
        │
        ├── 8. 경로 재설정 (Config 기반)              [ALPHA.cpp:202-205]
        │   Recipe, Result 경로를 SystemConfigMgr에서 읽어 재설정
        │
        ├── 9. 언어 설정                             [ALPHA.cpp:210-232]
        │   GetLanguageIndex() → SetThreadLocale/SetThreadUILanguage
        │   0=Korean, 1=English, 2=Chinese
        │
        └── 10. 메인 대화상자 실행                    [ALPHA.cpp:237-239]
            CDlgALPHA dlg(m_pAuroraMgr)
            m_pMainWnd = &dlg
            dlg.DoModal()                           # 여기서 메인 루프 진입
            → return FALSE                          # DoModal 종료 후 앱 종료
```

### 주요 클래스

| 클래스 | 파일 | 역할 |
|--------|------|------|
| `CALPHAApp` | `ALPHA.cpp/.h` | CWinApp 파생, 글로벌 인스턴스 `theApp` |
| `CAuroraMgr` | `AuroraMgr.cpp/.h` | 모든 Manager의 생성/소유/접근 중개 |
| `stAuroraMgrPool` | `AuroraMgrPool.h` | 27+ Manager 포인터를 담는 구조체 |
| `CSystemParamShare` | `SystemParamShare.cpp/.h` | 전역 경로/상태 싱글톤 |

---

## 8. Main Loop

### MFC Dialog-based 앱의 메시지 루프

ALPHA HMI는 **Dialog-based** MFC 애플리케이션이다. `CDlgALPHA::DoModal()`이 호출되면 MFC 내부의 `RunModalLoop()`가 실행되어 Windows 메시지 펌프가 동작한다. **별도의 `Run()` 오버라이드나 커스텀 메시지 루프는 없다.**

```
CALPHAApp::InitInstance()
  └── CDlgALPHA::DoModal()                          [ALPHA.cpp:239]
        └── CDialogEx::DoModal() → RunModalLoop()   [MFC 내부]
              │
              │  ┌──── Windows Message Pump ────┐
              │  │  GetMessage()                │
              │  │  TranslateMessage()           │
              │  │  DispatchMessage()            │
              │  │  (반복)                        │
              │  └──────────────────────────────┘
              │
              ├── CDlgALPHA::OnInitDialog()          [DlgALPHA.cpp:51]
              │   ├── MoveWindow(0, 0, 1920, 1050)   # 1920x1050 전체화면
              │   ├── CDlgTitleBar 생성 → PushSubDlg(enumWndID_TITLE)
              │   └── CDlgMainForm 생성 → PushSubDlg(enumWndID_FRAME)
              │       └── CDlgMainForm::OnInitDialog()
              │           ├── Sub Dialog 생성 (NaviBar, SubOperate, SubMaint, ...)
              │           ├── CSequenceManager::GetInstance() 접근
              │           ├── WorkThread 시작 (WorkThreadProbeProc)
              │           └── SetTimer(MAIN_STATE_TIMER_ID) 시작
              │
              ├── OnTimer(MAIN_STATE_TIMER_ID)       # 주기적 상태 갱신
              │   └── UpdateInformation()
              │       ├── GetMonitorMotion()          # 모터 상태 갱신
              │       ├── GetMonitorIO()              # I/O 상태 갱신
              │       └── SetMCCSignal()              # MCC 신호 갱신
              │
              ├── WM_MSG_* 메시지 핸들러들            # 시퀀스/상태 변경 통보
              │   ├── OnUpdateChangeInitialize()
              │   ├── OnUpdateChangeViewState()
              │   ├── OnUpdateMotorPosition()
              │   ├── OnUpdateCIMState()
              │   └── ... (15+ 핸들러)
              │
              └── CDlgALPHA::OnClose()               [DlgALPHA.cpp:158]
                  ├── 사용자 확인 (MessageBox)
                  ├── AlarmMonitorMgr->SetMonitoringStop()
                  ├── Sleep(1500)                    # 알람 정지 대기
                  ├── SubDlg 순회 → WM_MSG_UPDATE_CLOSE_SYSTEM 전송
                  ├── CAlarmController::ClearAlarm()
                  ├── CAuroraMgr::Finalize()         # 모든 Manager 정리
                  ├── CAlarmController::DelInstance()
                  ├── delete m_pAuroraMgr
                  └── CDialogEx::OnClose()           # DoModal() 종료
```

### 주요 Thread 구조

| Thread | 클래스/함수 | 용도 |
|--------|------------|------|
| Main UI Thread | `CDlgALPHA::DoModal()` | MFC 메시지 펌프 |
| Work Thread | `CDlgMainForm::WorkThreadProbeProc()` | 모터/IO 모니터링 |
| System Init Thread | `CSystemInitialize::ThreadRun()` | 시스템 초기화 22단계 |
| Glass Sequence Thread | `CDoWorkGlassSequenceEngine::ThreadRun()` | Glass 공정 시퀀스 |
| DoWork Item Threads | `CBaseSequenceImpl::ThreadRun()` (각 DoWork*) | 개별 공정 단계 |
| Alarm Monitor Threads | `AlarmMonitorTo*::ThreadRun()` (24+) | 개별 알람 감시 |
| TCP/IP Recv Threads | `CCommEthFunction` 내부 | 장비 통신 수신 |
| PLC Read/Write | `McProtocolManager` 내부 | MC Protocol 통신 |

### PreTranslateMessage 처리

`CDlgALPHA::PreTranslateMessage()` (`DlgALPHA.cpp:139`)에서 VK_ESCAPE, VK_RETURN, VK_SPACE 키 입력을 차단하여 실수로 대화상자가 닫히는 것을 방지한다.

### 종료 시퀀스

1. `CDlgALPHA::OnClose()` - 사용자 종료 확인
2. `AlarmMonitorMgr->SetMonitoringStop()` - 알람 모니터 정지
3. `Sleep(1500)` - 알람 정지 대기
4. 모든 SubDlg에 `WM_MSG_UPDATE_CLOSE_SYSTEM` 전송
5. `CAlarmController::ClearAlarm()` - 알람 해제
6. `CAuroraMgr::Finalize()` - 모든 Manager 정리 (Thread 종료, 리소스 해제)
7. `CAlarmController::DelInstance()` - 알람 싱글톤 해제
8. `delete m_pAuroraMgr` - 매니저 메모리 해제
9. `CDialogEx::OnClose()` → `DoModal()` 반환 → `InitInstance()` return FALSE → 앱 종료

---

## 부록: TCP/IP 장비 통신 객체 (13종)

`CCommTcpIpFactoryMgr`가 생성/관리하는 TCP/IP 통신 객체:

| ID | 클래스 | 장비 | 인스턴스 수 |
|----|--------|------|------------|
| VISION | `CCommTcpIpObjectVision` | 화상 검사 장비 | 2 |
| COLOR | `CCommTcpIpObjectColor` | 색차계 | 2 |
| ANGLE | `CCommTcpIpObjectAngle` | 시야각 측정기 | 2 |
| WAD | `CCommTcpIpObjectWAD` | WAD 장비 | 1 |
| PG | `CCommTcpIpObjectPG` | Pattern Generator | 1 |
| AGING | `CCommTcpIpObjectAgingPG` | Aging PG | 1 |
| LCR | `CCommTcpIpObjectLCR` | LCR 측정기 | 1 |
| TSP | `CCommTcpIpObjectTSP` | TSP | 6 |
| CIM | `CCommTcpIpObjectCIM` | CIM/MES 통신 | 1 |
| TIME | `CCommTcpIpObjectSysTime` | 시간 동기화 | 1 |
| MOTION | `CCommTcpIpObjectMotion` | Vision Motion | 1 |
| UTG1 | `CCommTcpIpObjectUTG` | UTG 측정기 #1 | 1 |
| UTG2 | `CCommTcpIpObjectUTG` | UTG 측정기 #2 | 1 |
