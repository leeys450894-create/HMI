# OCP 옵션 분개 작업 히스토리

> **작업일:** 2026-06-26
> **목적:** HMI 설비 Type을 런타임 Config 기반으로 분개하여, `MACHINE_TYPE=OCP` 시 1-Probe(P5 MEAS) 전용 동작 구현
> **원칙:** `MACHINE_TYPE=HMI`(기본값)이면 기존 동작 100% 유지, `OCP`일 때만 분기

---

## 목차

1. [Step 1: 설비 Type 옵션 인프라](#step-1-설비-type-옵션-인프라)
2. [Step 2: P1~P4 축 비활성화](#step-2-p1p4-축-비활성화)
3. [Step 3: AGING 비활성화 + Probe 반복 분개](#step-3-aging-비활성화--probe-반복-분개)
4. [Step 4: Probe X 인터록 분개](#step-4-probe-x-인터록-분개)
5. [Step 5: 측정 동작 분개](#step-5-측정-동작-분개)
6. [Step 6: ANGLE Head 분개](#step-6-angle-head-분개)
7. [Step 7: UI / 호밍 / AgingPG 마무리](#step-7-ui--호밍--agingpg-마무리)
8. [추가: m_szPROBEID 분개](#추가-m_szprobeid-분개)
9. [추가: MoveCmdPos 전체 감사 + Log 매칭](#추가-movecmdpos-전체-감사--log-매칭)
10. [수정 파일 전체 목록](#수정-파일-전체-목록)
11. [OCP 모드 동작 요약](#ocp-모드-동작-요약)
12. [결정 사항](#결정-사항)

---

## Step 1: 설비 Type 옵션 인프라

### 배경
기존 HMI는 `#define CUR_EQU_TYPE EQU_TYPE_OCP`로 설비 타입이 컴파일타임 상수. 런타임 분기 코드는 모두 주석 처리되어 있었음.

### 수정 내용

| 파일 | 변경 |
|------|------|
| `Manager/SystemConfigMgr.h` | `m_strMachineType` (CString), `m_bIsOCP` (bool) 멤버 추가. `IsOCP()`, `GetMachineType()` 접근자 추가 |
| `Manager/SystemConfigMgr.cpp` | `LinkIniFile()`에 `[EQUIP_TYPE] MACHINE_TYPE` 로딩 추가. 기본값 `"HMI"`. `m_bIsOCP = (m_strMachineType == "OCP")` |
| `ConfigurationFile/SystemConfig.ini` | `[EQUIP_TYPE]` 섹션 추가 |
| `Source/Bin/Config/SystemConfig.ini` | 동일 (런타임 복사본, `MACHINE_TYPE=OCP`로 설정됨) |

### 접근 패턴
```cpp
// 시퀀스/매니저에서
m_pShareItem->GetSystemConfigMgrPtr()->IsOCP()
// 또는
m_pSystemConfigMgrPtr->IsOCP()
```

---

## Step 2: P1~P4 축 비활성화

### 배경
OCP는 Probe Gantry 1개(P5 MEAS)만 사용. P1~P4(AGING)와 P5의 X/Y축은 물리적으로 없음.

### 수정 내용

| 파일 | 변경 |
|------|------|
| `Manager/MotorControlMgr.h` | `m_bIsOCP` 멤버, `DisableAxesForOCP()` 선언 추가 |
| `Manager/MotorControlMgr.cpp` | `DisableAxesForOCP()` 구현 — 19개 축 `bUse=FALSE` 강제: P1~P4 X1~X8/Y1~Y4/Z1~Z4 + P5 X9/X10/Y5 |
| `AuroraMgr.cpp` | 생성자에서 `IsOCP()` 체크 후 `DisableAxesForOCP()` 호출 |
| `FunctionModule/FunctionModuleEQState.cpp` | `IsProbeExistSensorOn()` — OCP 시 P1~P4는 항상 `false` 리턴. `SystemConfigMgr.h` include 추가 |

### 비활성화 축 목록
| 축 | Enum | 용도 |
|----|------|------|
| AXIS_PROBE_X1~X8 | P1~P4 Master/Slave X | AGING Probe X |
| AXIS_PROBE_Y1~Y4 | P1~P4 Y | AGING Probe Y |
| AXIS_PROBE_Z1~Z4 | P1~P4 Z | AGING Probe Z |
| AXIS_PROBE_X9, X10 | P5 Master/Slave X | OCP에 X축 없음 |
| AXIS_PROBE_Y5 | P5 Y | OCP에서 Y 사용 안 함 |

---

## Step 3: AGING 비활성화 + Probe 반복 분개

### 배경
OCP에는 AGING 프로브(P1~P4)가 없으므로 AGING 시퀀스 전체 스킵. 기존 `m_bUseAgingTest_2` 플래그를 강제 FALSE로 설정하여 이미 구현된 AGING 스킵 로직 활용.

### 수정 내용

| 파일 | 변경 |
|------|------|
| `DoWork/DoWorkGlassRecipeStep.cpp` | Recipe open 후 OCP이면 `m_bUseAgingTest_2 = FALSE` 강제 → AGING 시퀀스 전체 스킵. `SystemConfigMgr.h` include 추가 |
| `DoWork/DoWorkGlassPreMeasStep.cpp` | Probe X 위치 계산: OCP 시 P5만 계산, P1~P4 `m_nAgingStartStick = -1`. Probe Y 이동: OCP 시 스킵. Probe Contact: OCP 시 `AXIS_PROBE_Z5`만 |
| `DoWork/DoWorkGlassMeasureStep.cpp` | `_MoveMeasureArea`: P1~P4 Z축 체크 루프 2곳 OCP 시 스킵. `SystemConfigMgr.h` include 추가 |

---

## Step 4: Probe X 인터록 분개

### 배경
다른 축(Head Z, Loader Z 등) 이동 시 `AXIS_PROBE_X9` 위치를 참조하는 충돌 인터록이 있음. OCP에서 X9가 비활성화되어 있으므로 해당 인터록 스킵 필요.

### 수정 내용

| 파일 | 변경 |
|------|------|
| `Manager/MotorControlMgr.h` | `m_bIsOCP` 멤버 추가, `IsOCP()` getter 추가 |
| `Manager/MotorControlMgr.cpp` | `m_bIsOCP` 초기화 + `DisableAxesForOCP()`에서 `true` 설정 |
| `Manager/MotorControlMgr.cpp` | Loader Z → Probe X9 인터록 2곳 `if (!m_bIsOCP)` 가드 |
| `Manager/MotorControlMgr.cpp` | Head Gantry → Probe X9 충돌 인터록 3곳 `if (!m_bIsOCP)` 가드 |

---

## Step 5: 측정 동작 분개

### 배경
측정 시퀀스(DoWorkGlassMeasureStep)에서 Stick 전환 시 Probe X/Y 이동 및 P1~P4 Z 체크가 있음.

### 수정 내용

| 파일 | 변경 |
|------|------|
| `DoWork/DoWorkGlassMeasureStep.cpp` | Probe X 이동 블록: OCP 시 스킵 (`GetCurPos(AXIS_PROBE_X9)` 체크 + `GetCmdPos_ProbeX` + `MoveCmdPosMulti`) |
| | Probe Y 이동 블록: OCP 시 스킵 (`MoveCmdPosMulti(stPositionProbeY)`) |
| | P1~P4 Z축 체크 루프: OCP 시 스킵 (if + else 블록 모두) |
| | Probe UnContact 센서 루프: OCP 시 `df_PROBE_MEAS`부터 시작 |

### OCP 측정 Stick 전환 흐름
```
Probe Z5 언컨택 → Head 이동 → Stage Rotate → Stage X 이동 → Probe Z5 컨택
(Probe X/Y 이동 없음)
```

---

## Step 6: ANGLE Head 분개

### 배경
HMI: ANGLE 측정 시 X2(Bottom Gantry)가 주 제어축, X3(Back Gantry)가 종속.
OCP: X3가 주 제어축, X2가 종속으로 역전.

### 수정 내용

| 파일 | 변경 |
|------|------|
| `DoWork/DoWorkGlassMeasureStep.cpp` | ANGLE `stPosition[4].axis` — OCP이면 `AXIS_X3`, HMI이면 `AXIS_X2` (3곳) |
| `Manager/MaintControlMgr.cpp` | `MoveCmdPosAngle()` 내 Probe X9 충돌 체크: OCP 시 스킵 |

### OCP ANGLE 동작
- 호출부가 `AXIS_X3` 전달 → `MoveCmdPosAngle()` 내부 기존 `AXIS_X3` 분기 실행
- X3: 주 제어축 (목표 위치 직접 이동)
- X2: 종속축 (X3 + GantryGap)
- Y3, Y4: 기존과 동일

---

## Step 7: UI / 호밍 / AgingPG 마무리

### 수정 내용

| 파일 | 변경 |
|------|------|
| `DoWork/SystemInitializeStep.cpp` | Step 4(Probe Z): P1~P4 호밍 스킵, P5 Z만 호밍 |
| | Step 5(Probe Y): OCP 시 전체 스킵 |
| | Step 6(Probe X): OCP 시 전체 스킵, **Probe Rotate(AXIS_R1) 호밍 추가** |
| `Manager/CommTcpIpFactoryMgr.cpp` | OCP 시 `enumTCPIpFactorytID_AGING` AgingPG 객체 생성 스킵. `SystemConfigMgr.h` include 추가 |
| `DlgMainForm.cpp` | 7개 Probe 센서 루프: OCP 시 P5(index 4)만 체크. `SystemConfigMgr.h` include 추가 |
| `DlgTitleBar.cpp` | Probe 카드 체크 루프: OCP 시 P5만 체크. `SystemConfigMgr.h` include 추가 |

---

## 추가: m_szPROBEID 분개

### 배경
`VTM_SYSTEM::m_szPROBEID[5][32]` — 5개 Probe의 ID 문자열. OCP에서는 index 4(P5)만 유효.

### 수정 내용

| 파일 | 변경 |
|------|------|
| `DoWork/DoWorkGlassRecipeStep.cpp` | Probe 교환 체크 루프: OCP 시 P5(index 4)부터 시작 |
| `DoWork/DoWorkGlassAdsorptionStep.cpp` | Probe Align 루프: OCP 시 `df_PROBE_MEAS`부터 시작 |
| `DlgSubOperate.cpp` | Probe ID UI 표시: OCP 시 P1~P4 빈 문자열 처리 |
| `FunctionModule/FunctionModuleProbeACC.cpp` | SACC→공유메모리 PROBEID 쓰기: OCP 시 P1~P4 쓰기 스킵, P5만 |

### 이미 안전한 곳 (수정 불필요)
- `FunctionModuleEQState::SetEQSetPrboeData()` — 파라미터 기반, P5만 호출됨
- `FunctionModuleProbeACCStep` Mount/UnMount — `GetProbe()`가 P5만 선택
- `MaintControlMgr` ProbeContact 내 PROBEID — OCP에서 Z5만 진입하므로 index 4만 사용
- `SystemInitializeStep` — 상위 루프 `IsProbeExistSensorOn()` OCP 가드로 보호

---

## 추가: MoveCmdPos 전체 감사 + Log 매칭

### 배경
`MaintControlMgr.cpp`의 모든 MoveCmdPos 계열 함수에서 P1~P4 Probe 동작/로그가 OCP에서 발생하지 않도록 감사.

### 게이트웨이 함수 수정 (하위 호출 자동 보호)

| 함수 | 수정 |
|------|------|
| `MoveCmdPos_ProbeMove()` | OCP: `AXIS_ALL_Z` → Z5만 이동 후 즉시 리턴 |
| `MoveCmdPos_ProbeContact()` | OCP: 모든 nAxis를 `AXIS_PROBE_Z5`로 강제 리디렉트 → P5만 접촉, P1-P4 PROBEID 로그 제거 |
| `MoveCmdPos_ProbeUnContact()` | OCP: `AXIS_ALL_Z`/`AXIS_NONE` → Z5로 리디렉트 |
| `MoveCmdPos_ProbeContactAging()` | OCP: 함수 진입 즉시 `df_MOTION_RV_SUCCESSED` 리턴 (전체 스킵) |
| `MoveProbeAlignR()` | OCP: 함수 진입 즉시 리턴 (P1-P4 slave X축 이동 스킵) |

### 개별 함수 수정

| 함수 | 수정 |
|------|------|
| `MoveCmd_SafetyAll()` | 3개 센서 루프 OCP 시 `df_PROBE_MEAS`부터 시작, 1개 Z 루프 `AXIS_PROBE_Z5`부터 시작 |
| `MoveCmdPosAngle()` | Probe X9 충돌 체크 미가드 1곳 추가 (`!m_pMotorControlMgr->IsOCP()`) |
| `MoveCmdPosWAD()` | Probe X9 충돌 체크 — 이전 Step에서 이미 가드됨 |

### 게이트웨이 보호로 자동 안전해진 상위 호출자

| 함수 | 호출하는 게이트웨이 | 보호 원리 |
|------|---------------------|-----------|
| `MoveCmdAlignPos_Glass()` | `MoveCmdPos_ProbeMove(AXIS_ALL_Z)` | ProbeMove에서 Z5만 처리 |
| `MoveCmdAlignPos_Probe()` | `MoveCmdPos_ProbeMove(AXIS_ALL_Z)` | ProbeMove에서 Z5만 처리 |
| `MoveCmdPos_Stage()` | `MoveCmdPos_ProbeMove(AXIS_ALL_Z)` | ProbeMove에서 Z5만 처리 |
| `Move_SelectProbe()` | `GetCmdPos_ProbeX` + `MoveCmdPosMulti` | P1~P4 축 `bUse=FALSE` → 모션 무시 |
| `MoveCmdPosA/R/Multi()` | 축 레벨 `bUse` 체크 | P1~P4 축 즉시 리턴 |

---

## 수정 파일 전체 목록

| # | 파일 경로 | 수정 Step |
|---|----------|-----------|
| 1 | `Source/ALPHA/Manager/SystemConfigMgr.h` | Step 1 |
| 2 | `Source/ALPHA/Manager/SystemConfigMgr.cpp` | Step 1 |
| 3 | `Source/ALPHA/ConfigurationFile/SystemConfig.ini` | Step 1 |
| 4 | `Source/Bin/Config/SystemConfig.ini` | Step 1 |
| 5 | `Source/ALPHA/Manager/MotorControlMgr.h` | Step 2, 4, 6 |
| 6 | `Source/ALPHA/Manager/MotorControlMgr.cpp` | Step 2, 4 |
| 7 | `Source/ALPHA/AuroraMgr.cpp` | Step 2 |
| 8 | `Source/ALPHA/FunctionModule/FunctionModuleEQState.cpp` | Step 2 |
| 9 | `Source/ALPHA/DoWork/DoWorkGlassRecipeStep.cpp` | Step 3, PROBEID |
| 10 | `Source/ALPHA/DoWork/DoWorkGlassPreMeasStep.cpp` | Step 3 |
| 11 | `Source/ALPHA/DoWork/DoWorkGlassMeasureStep.cpp` | Step 3, 5, 6 |
| 12 | `Source/ALPHA/Manager/MaintControlMgr.cpp` | Step 6, 6+, MoveCmdPos감사 |
| 13 | `Source/ALPHA/DoWork/SystemInitializeStep.cpp` | Step 7 |
| 14 | `Source/ALPHA/Manager/CommTcpIpFactoryMgr.cpp` | Step 7 |
| 15 | `Source/ALPHA/DlgMainForm.cpp` | Step 7 |
| 16 | `Source/ALPHA/DlgTitleBar.cpp` | Step 7 |
| 17 | `Source/ALPHA/DoWork/DoWorkGlassAdsorptionStep.cpp` | PROBEID |
| 18 | `Source/ALPHA/DlgSubOperate.cpp` | PROBEID |
| 19 | `Source/ALPHA/FunctionModule/FunctionModuleProbeACC.cpp` | PROBEID |

---

## OCP 모드 동작 요약

### Probe 구성
- **사용:** P5 (MEAS) Z축만 동작
- **비활성화:** P1~P4 전축, P5 X/Y축
- **추가:** Probe Rotate (AXIS_R1) — 호밍 포함

### Auto Sequence
```
Adsorption → Recipe → GlassAlign → ProbeAlign → PreMeas → [AGING 스킵] → Measure → Result → Unload
```

### 측정 Stick 전환
```
Probe Z5 언컨택 → Head 이동 → Stage Rotate → Stage X 이동 → Probe Z5 컨택
```

### ANGLE 측정
```
X3 (Back Gantry, 주 제어) + X2 (Bottom Gantry, 종속) + Y3 + Y4
```

### 호밍
```
Step 4: P5 Z만 호밍 (P1~P4 Z 스킵)
Step 5: Probe Y 전체 스킵
Step 6: Probe X 스킵 + Probe Rotate(AXIS_R1) 호밍
```

---

## 결정 사항

| # | 항목 | 결정 |
|---|------|------|
| 1 | Probe Y5 사용 여부 | **사용 안 함** — Y5 비활성화, OCP Probe는 Z축만 동작 |
| 2 | ANGLE 측정 시 X2 역할 | **X2+X3 둘 다 사용** — X3가 주 제어축, X2는 종속 이동 |
| 3 | Probe Rotate 호밍 | **추가 필요** — SystemInitialize Step 6에 AXIS_R1 호밍 삽입 |
| 4 | Config_Motion_ACS.dat 관리 | **코드에서 강제** — 단일 dat 파일, OCP 시 `bUse` 오버라이드 |
| 5 | 주석 규칙 | `// LYS 2026-06-26` 형식으로 모든 OCP 수정에 주석 추가 |
