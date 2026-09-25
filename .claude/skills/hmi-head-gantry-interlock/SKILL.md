---
name: hmi-head-gantry-interlock
description: ALPHA HMI 헤드 갠트리(X2/X3)↔Z축(Z1~Z4) 인터락 체계, P5 프로브 유닛 겹침 대역, Sub Z 이동 규칙, Z2 부호 규약, Position.ini/ProbeChange.ini 죽은 섹션. MotorControlMgr/MaintControlMgr 이동·인터락 코드를 수정할 때 읽을 것
---

# hmi-head-gantry-interlock

출처: `CLAUDE.md` (2026-09-17 /doctor 정리로 이동, 백업 `CLAUDE.md.bak_20260917_doctor`). 전체 규칙(인코딩 편집 규칙, 빌드, 3벌 인터락 원칙 등)은 루트 `CLAUDE.md` 가 우선한다.

#### 헤드 갠트리 ↔ Z축 짝

| 갠트리 | Z축 | P5 프로브 유닛 간섭 |
|---|---|---|
| `AXIS_X2` BACK GANTRY | `AXIS_Z1` BACK Z(특성) | **있음** → `CMotorControlMgr::IsProbeUnitOverlapX()` 인터락 |
| `AXIS_X3` FRONT GANTRY | `AXIS_Z2` FRONT Z(화질+시야각) | 없음 (인터락 불필요) |

기존 인터락 코드에 이 짝이 **교차된 사례가 4곳 있었다**(2026-08-25 정리). 헤드 Z 관련 인터락을 손대기 전에 짝부터 확인할 것. Z2 분기에 남은 이동 범위 체크(`X2 < GetMinSWLimit(AXIS_X2) + 35`)도 X2를 보고 있어 교차 의심이 있으나 미확인.

**X3(FRONT GANTRY) ↔ Z2(FRONT Z) 하강 인터락 (2026-09-09 추가).** X3 분기의 기존 "Head Down 시 이동 금지"는 `GetOrgPos(AXIS_Z1) > 1`을 보는 교차 코드라 OCP(Z1 USE0)에선 dead. 그 바로 뒤에 **`Z2 > 1`(하강)이고 X3 목표 ≥ 100이면 거부**를 3벌(A_Sub: `dPos`, R_Sub: `현재+dPos`, Multi_Sub: 신규 `AXIS_X3` 분기) 모두 추가. **2026-09-17부터 세 경로 모두 `_Sub`은 코드만 반환하고 알람은 래퍼가 낸다** — 구 주석의 "멀티 래퍼는 `SetAlarm`을 안 하므로 직접 호출"은 더 이상 사실이 아니며, Multi_Sub 안의 직접 호출은 중복 팝업이 되어 주석 처리했다. 알람은 기존 `MOTION_INTERLOCK_5`(Head Gantry) 재사용(Alarm.xls 무변경), 로그 `MoveCmdPos*_Sub X3 NG - FRONT Z(Z2) DOWN …`. X3는 정상 동작에서 양수 영역으로 가지 않는다는 전제(사용자 확인).

**종속 갠트리 Safety 고정 (2026-09-09).** `MaintControlMgr`의 헤드 이동 5함수는 주동작 갠트리의 반대편을 "Gap 계산 위치"로 함께 보내던 것을 **Position.ini `[SAFETY]` 고정값**으로 교체했다: `MoveCmdPosAging/Review/WAD`(X2 주동작, `axis==AXIS_X2` 분기)의 종속 **X3 → `SafetyHeadX2`(−760)**, `MoveCmdPosVision/Angle`(X3 주동작, `axis==AXIS_X3` 분기)의 종속 **X2 → `SafetyHeadX1`(470)**. 매핑 근거는 `MoveCmd_SafetyAll`(7425행: X2←HeadX1, X3←HeadX2). 구 Gap 줄(`GetMinSWLimit(AXIS_X2) - (dGantryGap + 50)` / `dPos + dOffsetX[0] + dGantryGap`)은 **원복 대비 바로 위에 `//`로 남겨 둠** — 원복 시 주석 해제 후 `{ double dSafetyX* … }` 블록 제거. `MoveCmdPosColor`와 각 함수의 반대편 분기(HMI 모드용)는 손대지 않음.

**P5 프로브 유닛 인터락 체계 (2026-08-26 구축 / 2026-09-02 Z1→Z3/Z4 전환).** P5 유닛은 X9 위치에서 X방향 `INTERLOCKPROBESIZEX`(300mm) 폭을 차지하는 구조물이며, 보호는 두 겹이다. **AXIS_Z1은 OCP에서 미사용(USE0)이므로 2026-09-02에 두 보호를 모두 Sub Z RIGHT/LEFT(`AXIS_Z3`/`AXIS_Z4`)로 교체**했다 (Z3/Z4는 값이 클수록 하강, 0이 안전):
- **Z3/Z4 하강 거부** — `IsProbeUnitOverlapX(GetOrgPos(AXIS_X2))` 점 판정. 모션 계층 3벌(A/R/Multi_Sub)에서 SW리밋 체크 직후, **하강(=목표 > 현재)인데 겹치면** 거부. 세 경로 모두 `MOTION_INTERLOCK_CHECK_ALARM + 축`을 반환하고 래퍼가 `SetAlarm`을 낸다(Multi_Sub의 직접 호출은 2026-09-17에 중복이라 주석 처리). 기존 `else if(iAxis==AXIS_Z1)` 하강거부 3벌은 **dead code로 남겨둠**(Z1 USE0라 무해). 시퀀스 계층 `MoveCmdPosWAD`는 2026-09-07에 Z1 블록을 제거하고 XY 전 Sub Z 안전높이 후퇴 → 말미 Z3/Z4 하강(레시피 `m_fWadSubz*`, 하강 시 P5 겹침 인터락)으로 통일(호출부 Measure/Manual/TestMode/RecipeEditMgr 테스트모드/`MoveCmdPos_Head`의 Z1 항목 제거·교체), **`MoveCmdPosReview`는 2026-09-07에 Z1 블록을 제거하고 XY 전 Sub Z 안전높이 후퇴(`GetInterlockSubZSafety()`) → XY/Z2 → Z3/Z4 하강(레시피 `m_fVisionSubz*`, 하강 시 P5 겹침 인터락)으로 재구성** — Review 호출부 7곳(Measure/AGING×3/Manual/TestMode)의 `AXIS_Z1` 항목도 Z3(+Z4)로 교체(Manual/TestMode는 안전높이 0 유지). `MoveCmdPosAngle/Vision`의 Z1 지령 제거는 FRONT 갠트리 기반이라 그대로 둠.
- **겹침 대역은 [X9+1 .. X9+SIZE−1] = X9+1 ~ X9+299 이고, X2 가 X9 자리(±1)에 있으면 겹침이 아니다 (2026-09-16).** Probe Align 은 X2=0 / X9=0 에서 Sub Z 를 내리므로(카메라가 프로브 위) 구 대역(X9−1 부터)으로는 하강이 거부됐다 — 그래서 09-05 에 A/R_Sub 의 하강 인터락을 통째로 껐던 것. 09-16 에 `IsProbeUnitOverlapX` 대역을 좁히고 **A_Sub·R_Sub 하강 인터락을 재활성**해 3벌 모두 살아 있다. `RetractHeadZForProbeUnit` 의 경로 판정도 하한이 X9+1 로 같이 바뀌었다(상한은 X9+300 그대로). Probe Align 의 Z3/Z4 하강은 같은 날 **두 축 동시 지령 후 완료 대기**로 바꿨다(`DoWorkProbeSWAlignStep.cpp` `Align_MovePosition` 말미 — `MoveOrgPosMulti` 로 묶지 않은 이유는 Multi_Sub 에 살아 있던 하강 인터락에 걸리기 때문이었고, 대역을 좁힌 지금은 묶어도 되지만 그대로 둠). Glass Align 쪽(`DoWorkGlassSWAlignStep.cpp` `MoveAlignPosition` Head Z 블록)도 2026-09-16 에 같은 방식으로 동시 하강으로 바꿨다.
- **`MoveCmdPosWAD` 의 "XY 전 Sub Z 후퇴"는 X2 가 실제로 움직일 때만 한다 (2026-09-14 A안).** 09-07 구조는 함수 진입마다 Z3/Z4 가 안전높이(0)보다 내려가 있으면 무조건 0 으로 올렸는데, **하강(레시피 `m_fWadSubz*`) 슬롯은 특성 스텝의 첫 그룹(`bMoveFirstDone==false`, `DoWorkGlassMeasureStep.cpp` 2557행)에만 들어 있어** 2번째 그룹부터는 Y1/Y2 만 넘어와 후퇴만 하고 다시 내리지 않았다 → **첫 그룹만 레시피 높이(1/2)에서 재고 나머지 그룹은 전부 Sub Z 0 에서 측정**됐다(모션 로그: 매 글래스 `특성 Sub Z … POS:1.000/2.000` 1회 → 다음 그룹 `MoveCmdPosWAD SubZ retract before XY (Z3:1.000 Z4:2.000 -> 0.000)` → 이후 Sub Z 명령 없음). 수정: 넘겨받은 축에 `AXIS_X2` 가 있고 목표(`dPosition + dOffsetX[0] + dGlassOffsetX`)가 현재 `GetOrgPos(AXIS_X2)` 와 0.1 넘게 다를 때(또는 HMI 모드 `AXIS_X3` 주동작)만 후퇴(`bGantryXMove`). Y 만 움직이는 그룹은 P5 와 겹칠 일이 없고, X2 가 P5 대역을 지나는 경우는 모션 계층 `RetractHeadZForProbeUnit` 이 그대로 보호한다. 구 `if` 줄은 `//` 로 남김. **다점 측정(`stPosM`) 경로는 매 포인트 X2 목표가 달라 후퇴가 걸리는데 Z3/Z4 슬롯이 없어 같은 증상이 남는다** — 현재 레시피는 다점 미사용(09-14 로그 0건)이라 손대지 않았고, 쓰게 되면 그 경로에 Z3/Z4 레시피 슬롯을 넣을 것. `MoveCmdPosReview` 의 같은 후퇴 블록은 손대지 않았다(Review 는 호출마다 X2/Z3/Z4 를 다 넘긴다).
- **Z3/Z4 는 X2(BACK GANTRY)가 정지한 뒤에만 움직인다 — 모션 계층에서 강제 (2026-09-12).** 기존에는 시퀀스 계층의 호출 순서(`MoveCmdPosReview`/`WAD` 가 X 멀티를 `bDoneCheck=true` 로 보내고 그 뒤 Sub Z 멀티)에만 의존했는데, `MoveCmd_SafetyAll` · Glass/Probe SW Align 의 갠트리 멀티는 **`bDoneCheck` 를 생략(=false)** 하고 뒤쪽 `DoWork_MotionDone` 스윕으로 막는 구조라, 그 사이에 Sub Z 이동을 끼워 넣으면 바로 깨진다. 그래서 판정을 모션 계층으로 내렸다:
  - **`CMotorControlMgr::WaitBackGantryStopForSubZ(축, 위치문자열)`** — `m_pFunction->CheckMoveDone(AXIS_X2, 0)` 으로 **대기 없이** 현재 정지 여부를 먼저 보고, 움직이는 중이면 `CheckMoveDone(AXIS_X2, df_MOVINGDONE_TIME*3)`(=180초)로 **정지까지 기다린 뒤 통과**시킨다. **알람도 거부도 하지 않고 기다리기만 한다**(2026-09-12 요구사항 반영 — 인터락으로 사이클을 세우지 않는다). 타임아웃·disable 로 정지 확인이 안 되면 WARN 만 남기고 **그대로 진행**한다(SW 리밋·P5 겹침 인터락이 별도로 보호). X2 가 `bUse=false` 이거나 OUT_OF_SERVICE 면 즉시 반환.
  - 호출 지점은 **A_Sub / R_Sub / Multi_Sub 3벌 전부**이며 **방향 무관**(상승도 포함)이다. 반환값이 없고 호출부는 분기하지 않는다 — 기다린 뒤 그대로 지령이 나간다.
  - **`MoveCmdPosMulti_Sub` 는 한 배열에 X2 와 Z3/Z4 가 같이 실리면 WARN 로그만 남긴다**(동시 출발이라 "X2 먼저"가 성립 불가하지만 알람으로 막지는 않는다). 현재 그런 배열은 없지만 — X 배열은 X2/X3, Sub Z 배열은 Z3/Z4 로 분리되어 있고 모션 로그 6일치 `MoveCmdPosM Move(...)` 축 조합 전수 집계에서도 둘이 같은 줄에 나온 적이 없다 — 나중에 합쳐 넣는 실수를 구조적으로 막는다.
  - 로그(모션): `… Z<n> HOLD - BACK GANTRY(X2:…) moving, wait stop` → `… Z<n> GO - … stopped (waited N ms)`, 실패 시 `… Z<n> NG - … not stopped`. **HOLD 가 자주 찍히면 시퀀스가 X2 완료를 안 기다리고 있다는 뜻**이니 그 호출부를 고칠 것(모션 계층 대기는 최후의 안전망이지 정상 경로가 아니다).
  - 참고: `MoveCmdPosA_Sub`/`MoveCmdPosR_Sub` 의 **기존 Z3/Z4 하강 겹침 인터락은 주석 처리된 상태**(`// LYS 2026-09-05 인터락 임시 해제`)이고 `MoveCmdPosMulti_Sub` 에만 살아 있다. 이번 X2 정지 가드는 그와 별개로 3벌 모두에 들어갔다.
- **`MoveCmdPosAngle` 은 넘겨받은 Z3/Z4 를 조용히 버린다.** 내부 Sub Z 수집 루프가 `AXIS_Z5 || AXIS_Z6` 만 받는데 호출부 `DoWorkGlassMeasureStep.cpp`(ANGLE 측정)는 `stPosition[19]/[20]` 에 **Z3/Z4** 를 채워 보낸다 → ANGLE 에서 특성 Sub Z 는 한 번도 움직이지 않는다(에러·로그 없음). `MoveCmdPosAging` 도 Sub Z 블록 자체가 없다. 의도된 미사용인지 누락인지 확인 필요.
- **X2 이동 시 Z3/Z4 선상승** — `RetractHeadZForProbeUnit()`. 각 X2 분기의 말미(기존 인터락 전부 통과 후)에서 호출(A/R/Multi_Sub 3벌 모두 살아 있다 — R_Sub 의 "2026-09-10 임시 해제" 주석은 낡은 것). 실제 상승은 **`RetractSubZToSafety(from, why)`**(2026-09-21 분리, public)가 한다: Z3/Z4 중 하나라도 `INTERLOCKSUBZSAFETY`(0, Position.ini `[SUBZ_SAFETY]` 미러)보다 크면 `MoveCmdPosMulti` 로 둘 다 올리고 완료까지 기다리며(둘 다 이미 ≤safety면 skip), 실패하면 X2 이동을 포기한다.
  - **2026-09-21 부터 OCP 는 X2 가 실제로 움직이면(목표-현재 > 0.1) 항상 올린다.** 그 전에는 **[현재..목표] 경로가 P5 대역과 겹칠 때만** 올려서, 대역 밖 X2 이동은 Sub Z 가 내려간 채 움직였다 — 09-10~09-21 모션 로그에서 Sub Z 하강(295~370) 상태의 X2 이동 256회 중 197회가 그랬고 **전부 Maint 수동 조작**(R 189 / A 7 / M 1, 0.1·1·10mm 스텝)이었다. 자동 사이클은 `MoveCmdPosWAD`/`Review` 가 XY 전에 먼저 올리므로 영향이 없다. 같은 자리 재지령(TestMode 같은 스틱 Review 등)은 0.1 이내라 건드리지 않는다. **HMI 모드는 Z3/Z4 가 다른 축(Head1 Angle / Head2 Vision)이라 구 규칙(대역 통과 시에만)을 유지**한다.
  - **X2 원점복귀도 올린다** — `CMaintControlMgr::MoveHome(AXIS_X2)` 가 OCP 면 `RetractSubZToSafety(_T("MoveHome"), _T("X2 home"))` 를 먼저 부른다(원점복귀는 ACS 버퍼 직행이라 3벌을 안 탄다). 실패하면 원점복귀 안 하고 그 코드로 알람. 초기화 시퀀스는 `MotionHome` case 1 에서 Z3~Z6 을 X2 보다 먼저 호밍하므로 no-op.
  - 부작용: **Maint 화면에서 Sub Z 를 내린 채 X2 를 조금씩 움직이던 작업(초점 맞춘 채 X 위치 찾기)은 이제 매 스텝 Sub Z 가 0 으로 올라간다.** 이 작업을 살려야 하면 `RetractHeadZForProbeUnit` 의 `bNeed` 에 조건을 추가할 것.
  - 로그: `<from> X2 a -> b[ (crosses PROBE UNIT band)] - Sub Z down, retract Z3/Z4(z3,z4) -> 0.000 before BACK GANTRY moves`, 실패 시 `<from> Z3/Z4 retract FAIL(RET:n) - X2 move abandoned`.

#### Z2 부호 규약과 "Z2 → 0" 지령 (2026-09-12 정리)

**`AXIS_Z2`(FRONT Z, 화질+시야각)는 값이 작을수록 위, 클수록 아래다** — Sub Z(Z3/Z4)와 같은 방향 규약이되 안전 위치만 다르다.

| 값 | 의미 | 근거 |
|---|---|---|
| **−1000** | **상승(Safety)** | `Position.ini [SAFETY] SafetyZ2 = -1000`, `.dat HOME -1000.3` |
| −120 ~ −400 | 측정 높이 | 사이클 모션 로그의 실제 지령값 |
| **0 ~ 3.2** | **스트로크 최하단** | `.dat MN -1005.1 / MX 3.2` |

그래서 **`MoveCmdPosA(AXIS_Z2, 0, …)`은 "0으로 올린다"가 아니라 측정 위치보다 더 내려가는 하강**이다. 코드에 남아 있는 Z2→0 지령은 성격이 둘로 갈리니 구분할 것:
- **게이트 있는 복귀** — `MoveCmdPosAging`(`GetCurPos > 0`) · `MoveCmdPosReview`(`> 0.1`) · `MoveCmdPosVision`(`> 0.1`) · `MoveCmdPosColor`(`> 0.1`) · `DoWorkGlassMeasureStep`(`> 0`) · `DlgSubManual_Meas`(`> 0`). 0을 넘어간 만큼(최대 3.2mm)만 되돌리는 **상승**이라 유지한다.
- **무조건 하강** — `MoveCmdPosWAD`의 `if (bSetMoveAxisX) if (GetCurPos(AXIS_PROBE_X9) > dMoveAxisX - 20) MoveCmdPosA(AXIS_Z2, 0, …)`. Z2 현재 위치와 무관하게 0까지 내려가며, 2026-09-09 사이클 로그에 **1회 5.6초 스트로크로 66회** 찍혔다. **2026-09-12에 주석 처리로 제거**했다(`MoveCmdPosAngle`의 동일 블록은 그 전에 이미 주석 처리되어 있었으므로 둘을 같은 상태로 맞춘 것). 원복 시 주석 해제.
- Z2 이동 목표값 분포는 모션 로그로 바로 확인된다: `grep "AXIS:FRONT Z" | grep -oE "POS:[-0-9.]+" | sort | uniq -c`.

#### 위치 설정 ini는 2개이고, Position.ini의 일부 섹션은 죽어 있다 (2026-09-10 확인)

`CPositionConfigMgr`는 ini를 **두 개** 링크한다 — `m_pIniHelper` = `Position.ini`, `m_pIniHelper2` = **`ProbeChange.ini`**. 그런데 `LinkIniFile()`에서 **Position.ini 쪽 Link 호출 일부가 주석 처리**되어 있고 같은 값을 helper2가 다시 링크한다. 결과적으로 **Position.ini에 값이 버젓이 적혀 있는데도 읽지 않는 섹션**이 있다.

| ini | 실제로 읽는 섹션 |
|---|---|
| `Position.ini` | `SAFETY` · `SUBZ_SAFETY` · `ANGLE_SUB_SAFETY` · `LOADER_Z` · `LOADER_LOAD` · `LOADER_UNLOAD` · `LOADER_MEAS` · `ALIGN_PICKER` · `HEAD_OFFSET` |
| `ProbeChange.ini` | `ACXPOS` · `ACZPOS` · `PROBE_SAFETY` · `PROBECHANGE_Z` · `PROBEOFFSET` · `MEASPOS` · `ALIGNPOS` |
| **죽은 섹션(Position.ini에 있으나 무시)** | **`AUTOCHANGE_X` · `PROBE_SAFETY` · `PROBECHANGE_Z` · `PROBEOFFSET`** |

- **`GetACPosition(change, safety, sacc)` = `ProbeChange.ini [ACXPOS] POS2/POS3/POS4`.** AUTO CHANGE X(X4)의 Safety 위치는 Position.ini `[AUTOCHANGE_X] SAFETY=1340`이 **아니라** `[ACXPOS] POS3 = −1402.6`이다(`MoveCmd_SafetyAll`이 X4를 보내는 값과 일치). 프로브 Safety(`GetProbeSafetyPosition`)도 마찬가지로 ProbeChange.ini 쪽이다.
- 위치 관련 값을 확인·수정할 때 **Position.ini만 보고 판단하지 말 것.** 먼저 `PositionConfigMgr::LinkIniFile()`에서 그 섹션이 helper1인지 helper2인지, 주석 처리되어 있지 않은지 확인한다.

인터락 임계값은 `Bin/Config/CoreFunction/CoreFunction.ini`의 `[MOTOR_CONTROL_MGR]` 섹션에 있다 — `INTERLOCKPOS` / `GANGAP` / `HEADGAP` / `HEADGAPB` / `PROBEGAP` / `LIFTPIN` / `PROBESIZEX` / `PROBEZSAFETY` / `INTERLOCKSUBZSAFETY`. **키가 없으면 `CMotorControlMgr::LinkIniFile()`의 기본값이 조용히 적용**되므로(`LIFTPIN`=575, `PROBESIZEX`=300, `PROBEZSAFETY`=-600, `INTERLOCKSUBZSAFETY`=0 등 ini에 없는 키가 실제로 있다), 값을 확인할 때 ini만 보지 말 것.


#### Probe Z 컨택 ↔ STAGE / PROBE XY 양방향 인터락 (2026-09-21)

**규칙: Probe Z(OCP 는 `AXIS_PROBE_Z5`)가 내려가 있으면 STAGE X(`AXIS_X`)·STAGE ROTATE(`AXIS_LOADER_R`)·PROBE X/Y 는 절대 움직이면 안 된다.** 반대로 그 축들이 움직이는 중에는 Probe Z 를 내리지 않는다(같은 위험 상태를 다른 순서로 만드는 경로).

- **판정 한 곳: `CMotorControlMgr::CheckProbeContactInterlock(iAxis, dTarget, where)`** — 반환 `MOTION_INTERLOCK_12`(**3032**, 전용) 또는 성공. 보조 함수 `IsProbeContactGuardAxis` / `IsProbeZContact` / `IsProbeContactGuardAxisMoving`.
  - 하강 판정은 **사용 중(bUse) Probe Z 중 하나라도 `GetOrgPos > 0.1`**. Z5 는 `.dat` HOME −12 / MX 25 이고 컨택 지령값은 3~22(모션 로그)라 양수가 하강이다. **OCP 에서 미사용 Z1 은 Z5 와 ACS HW34 를 공유해 같은 값을 읽으므로 bUse 가드가 필수**다.
  - 이동 중 판정은 `GetMoving(ax) & 0x20`(`ACSC_MST_MOVE`, 이 계층은 ACS 헤더를 안 본다). `CheckMoveDone(ax, 0)` 은 디스에이블·리밋도 "미완료" 로 돌려 쓸 수 없다. `GetMoving` 이 −1(읽기 실패)이면 판정하지 않는다.
- **호출 지점 (전부 같은 함수):** `MoveCmdPosA_Sub` · `MoveCmdPosR_Sub`(목표 = 현재 + dPos) · `MoveCmdPosMulti_Sub`(배열 전 축 루프 + **같은 배열에 Probe Z 하강과 guard 축이 같이 실리면 그 자체로 거부** — 동시 출발이라 "움직이는 중" 판정이 성립하지 않는다) · A/Multi 의 **블로킹 재지령 직전**(목표 미도달 시 최대 3회 다시 보내는데 예전엔 인터락을 다시 안 봤다) · **`CMaintControlMgr::MoveHome`**.
- **`MoveHome` 이 빠져 있던 것이 실제 구멍이었다.** 원점복귀는 `m_pFunction->MoveHome` → ACS 버퍼로 바로 가서 A/R/Multi_Sub 3벌을 거치지 않는다. Maint 화면(`DlgSubMaint`·`DlgSubMaint_MotionFile`) Home 버튼으로 **컨택 중 STAGE X 가 원점(1064)까지 갈 수 있었고 알람도 없었다.** UI 쪽 `AXIS_UNIT_Z < 0.1` 검사는 프로브가 아니다. 두 화면은 이제 `MoveHome` 반환값이 1~999 가 아니면 `CheckHomeDone` 대기를 건너뛴다(예전엔 반환값을 버리고 최대 대기). 초기화 시퀀스는 원래 Probe Z 원점 → 완료 → 0 이동 뒤에 스테이지를 호밍하고 실패 시 즉시 빠지므로 안전했다.
- **구 3벌의 문제 두 가지 (구 코드는 `//` 로 남김).** ① 알람이 `MOTION_INTERLOCK_8`(3028) — `Alarm.xls` 문구가 **"Align 동작 전 Position NG Alarm"** 이라 작업자가 원인을 알 수 없었다(09-10 2건, 09-17 5건 실제 발생). 3028 은 다른 인터락 10곳과 공유라 문구만 바꿀 수 없어 전용 ID 를 신설했다. ② **`MoveCmdPosA_Sub` 만 거부 전에 Probe Z 5축을 0 으로 올리려 했다** — OCP 에서는 미사용 Z1~Z4 까지 보내 ACS 가 `MOTION_CMDPOS_ERROR`(3012, "모터 이동 제한(TEACHMODE/ARM/LIFE PIN/CLAMP)")로 거부 → 엉뚱한 Heavy 알람이 먼저 떠 호출 스레드가 리셋까지 멈췄고, 그 뒤 3028 이 또 떴다(09-17 17:07:49 모션 로그 `MoveCmdPosM … PROBE Z 1 … RET:3012` → `MoveCmdPosA_Sub PROBE_Z NG(1.000,-12,-12,-12,1.000)`). 성공했더라도 지령 없는 프로브 상승이다. R/Multi 의 같은 블록은 이미 주석이었다.
- **역방향 검사가 사이클을 안 세우는 근거:** 09-10~09-21 모션 로그 10일치의 Probe Z 하강 624회 중 guard 축이 이동 중이던 경우 0건(휴리스틱이 잡은 3건은 `RET:3017` 로 거부돼 실제로는 안 움직인 ROTATE 지령).
- **간접 보호만 있는 축:** STAGE LIFE PIN(`AXIS_LOADER_Z`)은 Z5 를 직접 보지 않지만 STAGE X 가 로딩 끝(≥ MX−100)이고 X9 ≤ `LIFTPIN` 일 때만 움직인다.
- 로그: `… PROBE_Z CONTACT NG - <축> move rejected, probe Z down (PROBE Z:…)` / `… PROBE_Z CONTACT NG - PROBE Z down to … rejected, <축> is moving (…)` / `MoveCmdPosMulti_Sub PROBE_Z CONTACT NG - probe Z down and stage/probe XY in the same command`. 알람 문구 `PROBE Z 컨택(하강) 상태 - STAGE / PROBE X,Y 이동 금지`.
- `AlarmIDList.h` 에 `static_assert(MOTION_INTERLOCK_12 == 3032 …)` — 앞 enum 이 바뀌어 번호가 밀리면 `Alarm.xls` 행과 어긋나므로 컴파일에서 막는다.
- **제자리 재지령은 통과 / 헤드 Cross 는 대상 아님 (2026-09-21 같은 날 보완).** 처음 버전은 guard 축이 **같은 위치로 다시 지령돼도** 거부해서, TestMode 가 같은 스틱에서 셀만 바꿀 때("stage not moving, probe UnContact skipped" 로 판정해 놓고) 스테이지 R/X 를 같은 위치로 재지령하는 것까지 3032 로 막았다 → Heavy 알람 2회, 리셋까지 스레드가 멈춰 헤드 Cross 이동이 밀렸다(16:25:36 ROTATE 89.984 / 16:29:10 STAGE X 263.091, 둘 다 현재 위치). 수정: `CheckProbeContactInterlock` 정방향은 **`|목표 − GetOrgPos| > 0.01`(A_Sub 도착 판정과 같은 값, `s_dProbeContactMoveTol`)일 때만** 판정하고, Multi 의 "같은 배열" 규칙도 실제로 움직이는 guard 축만 센다. 원점복귀는 목표를 모르므로 `bAlwaysMoves=true` 로 부른다. 그리고 TestMode(`RecipeEditMgr` MOVE)는 `bNeedUnContact==false` 면 스테이지 지령 자체를 보내지 않는다. **헤드 Cross(`AXIS_Y1~Y4`)·헤드 갠트리(X2/X3)·헤드 Z 는 guard 대상이 아니다** — 컨택 중 헤드만 이동하는 것이 측정 시퀀스·TestMode 의 정상 동작이다(헤드 Y 가 거부되면 원인은 헤드 간격 `MOTION_INTERLOCK_7`=3027 이나 SW 리밋 3901/3902 쪽이다).
