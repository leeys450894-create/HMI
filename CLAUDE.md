# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ALPHA HMI — Samsung 디스플레이 Glass 검사 자동화 설비 제어 소프트웨어. MFC C++ Dialog-based 애플리케이션.
이 디렉토리에서 Claude Code를 실행하는 경우, HMI 코드 자체의 분석·수정·빌드·운용이 목적이다.

상세 분석: `docs/` 폴더 (핵심 아키텍처 분석 문서 00~12번 + 기능별 포팅/이슈 문서) 및 `PROJECT_INDEX.md` 참조.

## Build System

- **IDE:** Visual Studio 2022 Community (toolset v143), MFC Dynamic DLL, **Unicode** character set
- **Platform:** x64
- **Solution:** `Source/ALPHA/ALPHA.sln` (8 projects: ALPHA EXE + 6 DLLs + FDCSense EXE)
- **Output:** `Source/Bin/ALPHA.exe`
- **DLL Projects:** Alarm, dSACC, dResult, dfcRecipe, dMCCSpec, ProbeHistory
- **EXE Project (embedded):** FDCSense (`CoreFunction/EX_FDC/FDCSense/`) — separate FDC data-collection process, built alongside ALPHA but launched independently

```bash
# Debug build
"/c/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe" \
  "D:\\OCP\\0.SW_Matching\\HMI\\Source\\ALPHA\\ALPHA.sln" //p:Configuration=Debug //p:Platform=x64 //m //nologo //v:minimal

# Release build
"/c/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe" \
  "D:\\OCP\\0.SW_Matching\\HMI\\Source\\ALPHA\\ALPHA.sln" //p:Configuration=Release //p:Platform=x64 //m //nologo //v:minimal

# Clean
"/c/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe" \
  "D:\\OCP\\0.SW_Matching\\HMI\\Source\\ALPHA\\ALPHA.sln" //t:Clean //p:Configuration=Debug //p:Platform=x64 //nologo
```

## Encoding & Source File Editing Rules (Critical)

HMI는 Unicode 프로젝트이며 파일은 대부분 **BOM 없는 ANSI/UTF-8** (`ALPHA.cpp`는 UTF-8 BOM).

**한글 텍스트 손상 방지:** 소스 파일에 한글 주석/문자열 리터럴이 다수 포함. 수정 시 반드시 준수:

1. **한글 포함 소스 파일(.cpp/.h)은 Edit 도구 사용 금지.** Edit 도구는 파일 전체를 UTF-8로 재인코딩하여 기존 한글(EUC-KR/ANSI)을 `\xEF\xBF\xBD`(U+FFFD)로 손상시킨다. 반드시 Perl raw byte 편집을 사용할 것.
2. **Perl raw byte 편집 방법:**
   ```bash
   perl -e '
     open(my $in, "<:raw", "file.cpp") or die;
     my $data = do { local $/; <$in> }; close $in;
     my $old = "exact ASCII string to find";
     my $pos = index($data, $old);
     die "not found" if $pos < 0;
     my $new = "replacement string";
     substr($data, $pos, length($old), $new);
     open(my $out, ">:raw", "file.cpp") or die;
     print $out $data; close $out;
   '
   ```
3. **수정 후 검증 (필수):** `perl -e 'open(F,"<:raw",$ARGV[0]);$d=do{local $/;<F>};$c=()=$d=~/\xef\xbf\xbd/g;print"damaged=$c\n"' file.cpp` — `damaged=0`이 아니면 백업에서 복원 후 재작업.
4. **손상 복구:** 백업 `D:\OCP\0.SW_Matching\HMI_20260716\HMI\` 에서 원본 복사 후 Perl로 변경사항 재적용.
5. **새 코드 주석은 영문 권장.** 기존 한글 주석/문자열은 절대 삭제·변경하지 말 것.
6. **수정 주석 형식:** `// LYS yyyy-mm-dd brief description` — 모든 소스코드 수정 시 반드시 추가.

## OCP Option Branching (Runtime Equipment Type)

HMI는 `SystemConfig.ini`의 `[EQUIP_TYPE] MACHINE_TYPE` 값으로 런타임 설비 타입을 결정한다.
- `HMI` (기본값): 기존 5-Probe 동작 100% 유지
- `OCP`: 1-Probe(P5 MEAS only), Probe Rotate, Stage 이동 방식

### 접근 패턴
```cpp
// Manager 클래스에서 (m_pSystemConfigMgrPtr 보유)
bool bIsOCP = m_pSystemConfigMgrPtr->IsOCP();

// DoWork Step 클래스에서 (주의: GetSystemConfigMgrPtr() 직접 호출 불가)
bool bIsOCP = m_pShareItem->GetSystemConfigMgrPtr()->IsOCP();

// MotorControlMgr (자체 플래그 보유)
bool bIsOCP = m_pMotorControlMgr->IsOCP();
```

### OCP 핵심 차이
| 항목 | HMI | OCP |
|------|-----|-----|
| Probe Gantry | 5개 (P1~P4 AGING + P5 MEAS) | 1개 (P5 MEAS only) |
| Probe X축 | AXIS_PROBE_X9/X10 (Master/Slave) | 없음 → Probe Rotate(AXIS_R1) |
| Probe Y축 | AXIS_PROBE_Y5 | 사용 안 함 |
| 측정 이동 | Probe X 이동 → Contact → Head 측정 | Stage(AXIS_X) 이동 → Contact → Head 측정 |
| ANGLE Head | X2 주 제어 → X3 종속 | X3 주 제어 → X2 종속 |
| AGING | P1~P4 수행 | 강제 비활성화 |

상세: [docs/OCP_Option_ChangeHistory.md](docs/OCP_Option_ChangeHistory.md)

## Architecture

### Layer Structure

```
CALPHAApp::InitInstance()                          [ALPHA.cpp]
  → CAuroraMgr (Central Orchestrator)              [AuroraMgr.cpp]
    → stAuroraMgrPool (28+ manager pointers)       [AuroraMgrPool.h]
      → 15 CAuroraSubMgr-derived managers          [Manager/]
      → 4 Singleton managers                       [Manager/]
  → CDlgALPHA::DoModal() (MFC Message Loop)        [DlgALPHA.cpp]
    → CDlgMainForm                                 [DlgMainForm.cpp]
      → CDlgNaviBar → 8 DlgSub* pages             [DlgNaviBar.cpp, DlgSub*.cpp]
      → 4 CDlgAlignCCD instances                   [DlgAlignCCD.cpp]
```

### Core Components

| Component | Key Class | File | Role |
|-----------|-----------|------|------|
| **Orchestrator** | `CAuroraMgr` | `Source/ALPHA/AuroraMgr.cpp/.h` | 전체 매니저 생성/접근 |
| **Sequence Engine** | `CSequenceManager` (Singleton) | `Manager/SequenceManager.cpp/.h` | 초기화 22단계 + Glass 9단계 시퀀스 |
| **Glass Engine** | `CDoWorkGlassSequenceEngine` | `DoWork/DoWorkGlassSequenceEngine.cpp/.h` | 9개 시퀀스 아이템 실행 Thread |
| **Global State** | `CSystemParamShare` (Singleton) | `SystemParamShare.cpp/.h` | Auto mode, Glass ID, Cycle state |
| **Event Router** | `CWindowEventHandler` (Singleton) | `WindowEventHandler.cpp/.h` | Cross-layer WM_USER 메시지 라우팅 |
| **Alarm** | `CAlarmController` (Singleton) | `Alarm/AlarmController.cpp/.h` | Alarm DLL facade |
| **Equipment State** | `CFunctionModuleEQState` | `FunctionModule/FunctionModuleEQState.cpp/.h` | READY↔LOAD↔MEASURE 상태 머신 |
| **Interlock** | `CFunctionModuleEQInterlock` | `FunctionModule/FunctionModuleEQInterlock.cpp/.h` | 안전 인터록 |

### Auto Sequence (9-Step Glass Inspection)

각 단계: `DoWork/DoWorkGlass{Name}.cpp` (상태 머신) + `DoWorkGlass{Name}Step.cpp` (단계 구현)
기반 클래스: `BaseSequenceImpl` (Thread + Event) → `BaseSequenceBody` (Step/Pause/Resume)
상세: [docs/03_AutoSequence.md](docs/03_AutoSequence.md)

### System Initialize (22-Step)

구현: `DoWork/SystemInitialize.cpp`, `DoWork/SystemInitializeStep.cpp` (~2,000줄)

### Motion (ACS SPiiPlus)

- **Controller:** ACS SPiiPlus TCP/IP (10.0.0.100:701), ACSCL_x64.dll v7.1.1.0
- **Axes:** 90 logical axes (`axis_type_t` enum in `CoreFunction/MotorFunction/AxisType.h`)
- **Speed grades:** HIGH/MID/LOW/HOME/LATCH/MULTI (per-axis velocity, acceleration, S-curve)
- **Probe Gantry:** 5개 (P1~P4 AGING, P5 MEAS), master/slave X-axis pairs
- **Interlock:** `CMotorControlMgr::MoveCmdPosA_Sub()` — gantry/head/probe/loader collision 체크
- **Homing:** ACSPL+ buffer 프로그램 실행 (`acsc_RunBuffer`), HomeFlag 변수로 완료 확인
- **Thread safety:** 5 critical sections (move/get/set/done/stop 분리)
- **Simulation:** `OUT_OF_SERVICE` 플래그로 하드웨어 없이 소프트웨어 테스트 가능

- **Dual Controller (OCP mode):** `df_MAX_CTRL=2` (defined in `MotorControl_ACS.h`), each axis maps to controller via `m_cAxis[iAxis].m_iACSMap` (0 or 1). Dispatch pattern: `m_hAcs[m_cAxis[iAxis].m_iACSMap]`. HMI mode uses `df_MAX_CTRL=1`.

Layer: `CMaintControlMgr` → `CMotorControlMgr` → `CMotorFunction` → `CMotorControl_ACS`
상세: [docs/04_Motion.md](docs/04_Motion.md), [docs/ACS_Dual_Controller_Plan.md](docs/ACS_Dual_Controller_Plan.md)

### PLC (Mitsubishi MELSEC MC Protocol)

- **Protocol:** MC Protocol over TCP/IP
- **Memory:** M4000~4499 (PLC→PC read), M4500~4999 (PC→PLC write), F0~F1999 (alarms), D-area (FDC/sensor data)
- **Address mapping:** `PLCMAdress.csv` → ~500 logical enum (`plc_MBIT_t`) → 물리 M-address (9999 = unused)
- **Scanner threads:** M/F/D/PACC/ProbeCard/GlassSensor 영역별 독립 폴링 Thread
- **Thread safety:** 4 critical sections (bit/word/multiBit/actionDone)
- **Simulation:** `OUT_OF_SERVICE` 플래그로 가상 비트 동작

Layer: `CIOMapControlMgr` → `CIOFunction` → `CMcProtocolManager` → `CMcAddressScannerImpl` subclasses
상세: [docs/05_PLC.md](docs/05_PLC.md)

### Vision (NI Vision + Basler GigE)

- **Camera:** 4 Basler GigE cameras via NI-IMAQdx (`CAlignCameraMgr` x4)
- **Pattern matching:** `imaqMatchPattern3()` — MATCH_SCORE/DISTANCE/POSITION modes
- **Glass Align:** 3-phase (Search → TZero theta correction → XYZero position correction), 25-position x 6-theta grid, centering retry
- **Probe Align:** 5 probe gantry 지원, dynamic camera/axis selection
- **Image format:** U8 → RGB → BGRA 4-byte/pixel, PNG 저장

Layer: `CAlignCameraMgr` → `CFrameGrabberFunction` → `CFrameGrabber_BS4M` (IMAQdx)
상세: [docs/06_Vision.md](docs/06_Vision.md)

### Communication (TCP/IP Factory)

14종 장비 통신 — Factory 패턴 (`CCommTcpIpFactoryMgr`, `Source/ALPHA/Manager/CommTcpIpFactoryMgr.cpp`, ~2,400줄):
Vision, Color, WAD, Angle, PG, AgingPG, LCR, TSP, CIM, SysTime, Motion, UTG(x2), **PGMon**(ELP ElecBox 모니터링, 2026-07-29 추가)
모두 `CCommEthFunction` 상속, 개별 send/receive 프로토콜 구현.
상세: [docs/02_CallGraph.md](docs/02_CallGraph.md)

### Alarm System

- **3-layer:** Alarm DLL (`CAlarmManager`) → `CAlarmController` (Singleton facade) → `CAlarmMonitorMgr` (120+ 모니터 스레드)
- **Monitor groups:** Emergency(16), MC_OFF(10), Sensor(73+), TeachKEY(1), DoorOpen(7)
- **Alarm flow:** Monitor thread → `SetAlarm(id)` → Heavy: 블로킹 대화상자 / Light: 비블로킹 알림
- **Alarm codes:** `enumAlarmID` (1000~10000) in `Source/Include/AlarmIDList.h`

상세: [docs/08_Alarm.md](docs/08_Alarm.md)

## Thread Structure (~120-180 threads at runtime)

| Category | Count | Pattern | Key Files |
|----------|-------|---------|-----------|
| Alarm monitors | 75-107 | 개별 센서 폴링 (200ms~1s) | `Alarm/AlarmMonitorTo*.cpp` |
| Sequence | 6+9+N | Event-driven WaitForMultipleObjects | `DoWork/*.cpp` |
| PLC scanners | 6 | 주기적 폴링 | `CoreFunction/IOMapFunction/McProtocol/` |
| TCP/IP comm | 14+ | 수신 대기 Thread | `CommTcpIpObject*.cpp` |
| UI worker | 1+ | 50ms 주기 모니터링 | `DlgMainForm.cpp` |

동기화: `CCriticalSection` + `CCriticalSectionSync` (RAII), `CEvent`, `WaitForSingleObject/Multiple`
상세: [docs/09_Thread.md](docs/09_Thread.md)

## Global State (Singletons & Shared Memory)

**8 Singletons:** CSystemParamShare, CSequenceManager, CWindowEventHandler, CAlarmController, CRecipeMgr, CResultMgr, CSACCMgr, CMCCLogMgr — 모두 `GetInstance()`/`DelInstance()` 패턴

**4 Shared Memory:** VTM_SYSTEM (메인 시스템 상태, "CCLINK"), VTM_RECIPE (레시피, "HBS_RECIPE_SHARED_MEMORY"), VTM_FOR_FDC (FDC 센서), VTM_MCC (MCC 로깅)

**Extern globals:** `g_sAppPath` (CString), `g_csQuery` (CCriticalSection), `theApp` (CALPHAApp)

상세: [docs/10_GlobalState.md](docs/10_GlobalState.md)

## Recipe File Versioning (Critical)

Recipe Save/Load uses sequential `ReadString`/`ReadDouble`/`ReadInt` — **order must match exactly** between Save (`RecipeMgrDlg.cpp`) and both Load files (`OutRecipeFunc.cpp`, `InRecipeFunc.cpp`).

- **Version enum:** `RCP_VERSION` in `Source/Include/preDefine.h` (also mirrored in `CoreFunction/dll_Recipe/dfcRecipe/preDefine.h`)
- **Pattern:** New fields go at FILE END (before `File.Close()`), guarded by `if (nVersion > RECIPE_FILE_Vxxxx)`
- **RECIPE_FILE_END** doubles as the current version number written during Save
- **Backward compat:** Old files without new fields hit the `else` branch which calls `.Clear()` on the new struct
- **Three-file sync rule:** Any Save/Load field addition must be updated in all 3 files simultaneously with identical ordering

## Resume Inspection

Allows resuming an interrupted Glass inspection from where it stopped (e.g., after Alarm/Stop).

- **ResumeInfo.ini:** Persisted via `CResumeInfoMgr` (owned by `CAuroraMgr`'s manager pool, not a singleton — access via `m_pAuroraMgr->GetResumeInfoMgrPtr()` / `m_pShareItem->GetResumeInfoMgrPtr()`) — saves InnerID, last completed step, measurement index
- **Entry points:** Manual start (`DlgSubOperate.cpp`) and CIM Online (`TO_SERVER_MEAS_START`) both call `CheckAndApplyResumeMode()`
- **Recipe step:** In Resume mode, skips `ResultClear` and reloads previous raw data files into FPSpread via `Updata_Data()`
- 상세: [docs/Resume_Inspection_Plan.md](docs/Resume_Inspection_Plan.md)

## Key Rules for Modification

### 고위험 수정 영역

1. **`pch.h` 수정** — 전체 프로젝트 리빌드 유발 (`AlarmIDList.h`, `WindowEventHandler.h` 포함)
2. **`ShareMem.h` (VTM_SYSTEM)** — 멤버 추가 시 반드시 struct 끝에 추가 (공유 메모리 오프셋 보존)
3. **`AlarmIDList.h` 수정** — pch.h에 포함되어 있어 전체 리빌드
4. **`IOMapAddressDefine.h` (plc_MBIT_t enum)** — PLC 주소 상수는 물리 I/O에 직접 매핑, 변경 시 하드웨어 영향
5. **`AxisType.h` 수정** — 모든 모션 코드의 기반, 축 추가/변경 시 Position.ini와 Config_Motion_ACS.dat도 함께 수정
6. **`preDefine.h` (Recipe 구조체)** — App(`Source/Include/`)과 DLL(`CoreFunction/dll_Recipe/dfcRecipe/`) 양쪽에 동일 파일 존재, 구조체 변경 시 반드시 양쪽 동기화. `RCP_VERSION` enum과 `S_RecipeInfo` struct 포함
7. **`FDCSheardDefine.h` / `SheardDefineMCC.h` (FDC/MCC 공유메모리 계약)** — `Source/Include/`와 DLL 프로젝트 내부에 수기 사본 존재, 배포된 외부 프로세스(FDCSense.exe, MCCLog.exe)와 struct 레이아웃이 바이트 단위로 일치해야 함. 필드 추가는 반드시 struct 끝에만

### 수정 영향도 (Impact Matrix)

| 수정 대상 | 주요 영향 파일 |
|-----------|---------------|
| Motion | MotorControlMgr → MaintControlMgr → SystemInitializeStep → DoWork/*(전체) → UI(Manual/Maint) |
| PLC | IOMapControlMgr → FunctionModule/*(전체) → AlarmMonitor*(전체) → DoWork/*(전체) |
| Vision | AlignCameraMgr → DlgAlignCCD → DoWorkGlassSWAlign/ProbeSWAlign → FrameGrabber*(CoreFunction) |
| Recipe | RecipeData → RecipeEditMgr → CommTcpIpFactoryMgr(배포) → DoWorkGlassRecipe → 49개 소스 파일 |
| Alarm | AlarmController → AlarmMonitorMgr → 120+ 모니터 스레드 → DlgSubAlarm → Alarm.dll |
| Sequence | BaseSequenceImpl → DoWork/*(전체) → SequenceManager → FunctionModule/*(전체) |

상세: [docs/12_ImpactAnalysis.md](docs/12_ImpactAnalysis.md)

### 대용량 파일 (반드시 특정 라인 범위만 읽을 것)

| File | Path | Lines |
|------|------|-------|
| MaintControlMgr.cpp | `Source/ALPHA/Manager/` | ~8,400 |
| MotorControlMgr.cpp | `Source/ALPHA/Manager/` | ~2,900 |
| SystemInitializeStep.cpp | `Source/ALPHA/DoWork/` | ~2,000 |
| DlgMainForm.cpp | `Source/ALPHA/` | ~2,240 |
| ShareMem.h | `Source/Include/` | ~2,000 |
| DefinesPLC.h | `Source/Include/` | 1,000+ |
| preDefine.h | `Source/Include/` + `CoreFunction/dll_Recipe/dfcRecipe/` | ~770 (dual copy, kept in sync) |

`CommTcpIpFactoryMgr.cpp` (`Source/ALPHA/Manager/`) was previously ~26,000 lines but has since been refactored down to ~2,400 — no longer needs line-range-only reads.

## Knowledge Base (docs/)

| Document | Content |
|----------|---------|
| [00_ProjectOverview.md](docs/00_ProjectOverview.md) | Solution 구조, 폴더, 레이어, Dependencies, Entry Point |
| [01_ClassArchitecture.md](docs/01_ClassArchitecture.md) | 100+ 클래스 상세 (역할, 상속, 인터페이스, 생명주기) |
| [02_CallGraph.md](docs/02_CallGraph.md) | Entry Point → UI → Manager → Device 전체 호출 흐름 |
| [03_AutoSequence.md](docs/03_AutoSequence.md) | 초기화 22단계 + Glass 9단계 자동 검사 상세 |
| [04_Motion.md](docs/04_Motion.md) | ACS SPiiPlus, 90축, 속도 등급, 호밍, 인터록 |
| [05_PLC.md](docs/05_PLC.md) | MC Protocol, M/F/D/P/C/G 스캐너, 주소 매핑 |
| [06_Vision.md](docs/06_Vision.md) | Basler GigE, NI Vision, 패턴 매칭, Glass/Probe Align |
| [07_Recipe.md](docs/07_Recipe.md) | Recipe 구조체, DLL/App 이중 레이어, 공유 메모리 |
| [08_Alarm.md](docs/08_Alarm.md) | Alarm DLL, 120+ 모니터 스레드, 알람 코드 1000~10000 |
| [09_Thread.md](docs/09_Thread.md) | 120-180 스레드 분류, 동기화 패턴, Safety 주의사항 |
| [10_GlobalState.md](docs/10_GlobalState.md) | 8 Singleton, 4 Shared Memory, extern 전역 변수 |
| [11_DataFlow.md](docs/11_DataFlow.md) | Start → PLC → Motion → Vision → Result → CIM 데이터 흐름 |
| [12_ImpactAnalysis.md](docs/12_ImpactAnalysis.md) | 10개 서브시스템별 수정 영향도 매트릭스 |
| [OCP_Option_ChangeHistory.md](docs/OCP_Option_ChangeHistory.md) | OCP 옵션 분개 전체 작업 히스토리 (7 Step + 추가 작업) |
| [ACS_Dual_Controller_Plan.md](docs/ACS_Dual_Controller_Plan.md) | OCP 모드 Dual ACS Controller 설계 |
| [Resume_Inspection_Plan.md](docs/Resume_Inspection_Plan.md) | 검사 재개(Resume) 설계 |
| [PROJECT_INDEX.md](PROJECT_INDEX.md) | 마스터 인덱스 (주요 클래스, Manager, 파일 목록) |

### 기능별 포팅/이슈 문서 (docs/)

| Document | Content |
|----------|---------|
| [FDC_Structure_Comparison.md](docs/FDC_Structure_Comparison.md) | HMI vs GMI FDC(Factory Data Collection) 구조 비교 |
| [ELP_ElecBox_Porting_Plan.md](docs/ELP_ElecBox_Porting_Plan.md) | ELP ElecBox 모니터링(PGMon) 이식 계획 |
| [ELP_FDC_MCC_Upload_Plan.md](docs/ELP_FDC_MCC_Upload_Plan.md) | ELP 데이터의 FDC/MCC 업로드 경로 통합 계획 |
| [ELP_Upload_Confirmation_Requests.md](docs/ELP_Upload_Confirmation_Requests.md) | ELP 데이터 매핑 관련 미확정 항목(고객사/담당자 확인 필요) |
| [MCC_GMI_HMI_Comparison.md](docs/MCC_GMI_HMI_Comparison.md) | MCC(Machine Condition Check) GMI 기준 vs HMI 구조 비교 |
| [MCC_Implementation_Plan.md](docs/MCC_Implementation_Plan.md) | MCC 기능 이식 구현 계획 |
| [MCC_GMI_TO_HMI_IMPLEMENTATION_REPORT.md](docs/MCC_GMI_TO_HMI_IMPLEMENTATION_REPORT.md) | MCC 이식 결과 보고서 (파일 목록, 검증 결과, 미확정 항목) |
| [WAD_MultiMeas_Porting_Plan.md](docs/WAD_MultiMeas_Porting_Plan.md) | WAD 다점측정 이식 계획 |
