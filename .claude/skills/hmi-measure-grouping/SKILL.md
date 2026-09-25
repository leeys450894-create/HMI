---
name: hmi-measure-grouping
description: ALPHA HMI 측정 셀 그룹핑(헤드 페어링 순번 방식), 유휴 헤드 Safety, HEAD_OFFSET 센서 배치와 오프셋 축 규약. DoWorkGlassMeasureStep 측정 스텝·MoveCmdPos* 헤드 이동을 수정할 때 읽을 것
---

# hmi-measure-grouping

출처: `CLAUDE.md` (2026-09-17 /doctor 정리로 이동, 백업 `CLAUDE.md.bak_20260917_doctor`). 전체 규칙(인코딩 편집 규칙, 빌드, 3벌 인터락 원칙 등)은 루트 `CLAUDE.md` 가 우선한다.

## 측정 셀 그룹핑 & 유휴 헤드 (WAD/VISION/ANGLE 공통 — 2026-09-09 정리)

측정 스텝(`DoWorkGlassMeasureStep.cpp`의 `_Measure_WAD` / `_Measure_VISION` / `_Measure_ANGLE`)은 **셀을 2개씩 묶어 두 헤드에 하나씩 배정**하고, 그 그룹(`vecMeasGroup`, `S_MeasPos`) 하나가 이동 1회 + 측정 1회다. 세 스텝이 **거의 동일한 코드로 3벌 복제**되어 있으므로 그룹핑을 고칠 땐 항상 3벌을 함께 볼 것.

- **페어링 규칙 — 2026-09-14부터 순번 방식** (`MakeHeadPairByOrder`, `DoWorkGlassMeasureStep.cpp`, 선언은 `.h`). −측(`X<0`)/+측(`X>=0`) 셀을 각각 레시피 순서로 늘어놓고 **k번째끼리** 묶는다(20셀: K01↔A01, L01↔B01 … T01↔J01 = 10그룹). 헤드 간격(`dHeadGap` = `INTERLOCKHEADGAP`과 레시피 `m_fBackHeadGap` 중 큰 값)은 **짝 선택에 쓰지 않고** 묶인 쌍의 거리 검사에만 쓴다 — 거리 ≤ 간격이면 OCP 로그 `[WAD] HeadPair NG …` WARN 후 두 셀 모두 잔여 패스에서 단독 측정. 매 측정 `[<태그>] HeadPair by order: minus=… plus=… pair=… gapNG=… single=… HeadGap=… firstNG …` 한 줄(SYST)만 남긴다 — 쌍마다 WARN을 찍으면 단변(스틱 폭 626.98 < 간격 720, 전 쌍 미달이 정상)에서 글래스당 백여 줄이 되어 요약으로 줄였다. 에이징 셀 스킵·샘플링 필터는 호출부가 `pbUse`로 미리 넘긴다. 적용 6곳: Measure 스텝 `WAD`/`VISION`/`COLOR`/`ANGLE`/`VISION_COLOR` + 수동 `CDlgSubManual_Meas::MakeMeasGroup`(필터 없음). **잔여 단독 패스의 헤드 판정은 `> 0`** (2026-09-14 저녁 확정 — 50 → 0 → 50 → 0 으로 오갔다). VISION 오프셋이 0이 된 뒤로는 X=0 셀을 어느 헤드가 맡아도 되고, ANGLE(Y3 = X−272)은 J01(41.8) 같은 +측 소형 X 셀을 Y3로 보내야 쉬는 Y4 Safety(−860)와 근접센서 간격(580)이 나온다. X=0 셀은 `> 0`이 거짓이라 LEFT(Y2/Y4)가 맡는다.
  - **구 방식(원복 대비 `//` 주석으로 남김)의 사고 2가지**: 구 코드는 `X < 50`인 셀마다 A01부터 "`dHeadGap`보다 먼 첫 셀"을 탐욕적으로 골랐다. ① **50 기준이 +측 셀을 −측으로 분류** — 2026-09-12 레시피 J01 X=+41.80이 기준 셀이 되어 **J01↔T01이 1번 그룹**(18:37/18:41 회차). ② **헤드 간격 값에 따라 짝이 밀림** — 간격이 −측 첫 셀↔A01 거리(729.77)보다 크면 L01↔A01(간격 730~801, 17:55)이나 O01↔A01(946~1018, 18:18)부터 시작하고 뒤쪽 셀이 짝 없이 남았다. 같은 레시피로 회차마다 순서가 달랐던 이유다.
  - 순번 방식에서도 **쌍 거리(현재 레시피 729.77)보다 헤드 간격을 크게 잡으면 전 쌍이 단독 측정**으로 떨어진다(순서는 유지). 간격은 쌍 거리 미만으로 둘 것.
- **짝 없는 셀의 쉬는 헤드는 `[SAFETY]`로 보낸다 (2026-09-14).** 구 방식은 짝 없는 셀의 반대 헤드를 `셀 X ± dHeadGap`(`m_nArea = -1` 더미 셀) 좌표로 **실제 지령**했고, 헤드 오프셋까지 더해져 사고가 났다 — 2026-09-09 ANGLE S01에서 유휴 Y3 −65.9 HW 리밋 정지, 가운데 셀(X=0)에서 WAD 유휴 Y1 = 720+175 = 895(Y1 HW 리밋 ~790 초과), 화질/시야각은 오프셋 ∓50 때문에 **모든 단독 셀의 Y3–Y4 간격이 620 < `INTERLOCKHEADGAP` 720**이라 모션 인터락에 거부된다. 현재 구조:
  - **표시**: 측정 스텝이 이동 직전에 `MarkIdleHeadSafety(stPosition, nAxis, vecMeasGroup[i], dShiftX)`로 더미 슬롯 X(+ `dShiftX`, WAD Multi Point는 `ptMulti.dPosX`)와 **정확히 같은 값이 들어간 Y1~Y4 축**을 `df_HEAD_Y_IDLE_SAFETY`(−99999)로 바꾼다. `nMeasArea==1` 분기처럼 ±gap/2로 덮어쓴 축은 값이 달라 건드리지 않는다. 호출 11곳: Measure 스텝 WAD(일반+Multi)·VISION·COLOR·ANGLE·VISION_COLOR, 수동 `ManualMoveCell` 4종. 더미 셀 자체(`m_fCellX`)는 그대로라 dLeft/dRight 판정·`SetMeasPos` 값은 바뀌지 않는다.
  - **치환**: `MoveCmdPosWAD/Vision/Color/Angle`이 Y 멀티 직전 `CMaintControlMgr::ApplyIdleHeadSafetyY()`를 불러 표시된 축 목표를 **헤드 오프셋 없이** `SafetyHeadY1`(Y1) · `SafetyHeadY2`(Y2) · `SafetyHeadY3`(Y3) · `SafetyHeadY4`(Y4)로 바꾸고, 뒤따르는 `MoveCmdPosMulti`가 더할 `m_dAlignOffset`을 미리 뺀다 → `MoveCmd_SafetyAll`과 같은 원시좌표에 선다. 모션 로그 `MoveCmdPos<종류> idle head Y<n> -> SAFETY <값>`.
  - **FRONT 헤드 Safety 키 분리 (2026-09-14)**: Position.ini `[SAFETY]`에 **`SafetyHeadY3`(FRONT RIGHT) / `SafetyHeadY4`(FRONT LEFT)** 추가. 이전엔 Y3/Y4도 `SafetyHeadY1/Y2`를 같이 썼다. 읽기는 `CPositionConfigMgr::GetFrontHeadSafetyPosition()`, 키가 없으면 `LinkIniFile`이 **바로 앞에서 읽은 Y1/Y2 값을 기본값**으로 받는다(`CIniHelper::Read`는 없는 키를 파일에 쓰지 않음). 적용처: `ApplyIdleHeadSafetyY`, `MoveCmd_SafetyAll`의 Y3/Y4 이동 2곳(구 `dSafetyHeadY1/Y2` 줄은 `//`로 남김), COLOR `bOneCell`·수동 COLOR `nMeasArea==1`의 `dLeft2/dRight2`(FRONT) 대입. `RecipeEditMgr`의 헤드 ID 연결 계산은 Y1/Y2 그대로. 설정 화면(`DlgSubConfig`)은 Y1/Y2만 표시하며 Y3/Y4는 ini로만 바꾼다.
  - **Safety 값 제약**: 쉬는 헤드 Safety와 측정 헤드 목표(셀 X + 헤드 오프셋) 간격이 그 갠트리의 인터락 간격(FRONT `INTERLOCKHEADGAP`, BACK `INTERLOCKHEADGAPB`) 이상이어야 한다. 미달이면 모션 계층이 `MOTION_INTERLOCK_7`로 거부하므로 그 전에 `… idle head SAFETY gap NG - Y3 … / Y4 … gap … < INTERLOCKHEADGAP[B] …, check Position.ini [SAFETY] SafetyHeadY1~Y4` WARN을 남긴다.
  - **FRONT 근접 센서 실측 569.1 → `INTERLOCKHEADGAP` = 580, `SafetyHeadY3/Y4` = 860/−860 (2026-09-14 저녁 확정).** 사용자가 엔코더로 앞 헤드를 붙여 센서가 안 걸리는 Y3−Y4 간격 569.1을 실측했고, 여유 11을 둬 580으로 `CoreFunction.ini`에 넣었다(BACK `INTERLOCKHEADGAPB`는 720 유지). 이때 `MoveCmdPosMulti_Sub`의 Y1/Y2 검사가 2026-07-16부터 `m_dInterlockHeadGap`을 쓰고 있어 **`m_dInterlockHeadGapB`로 되돌렸다**(A/R_Sub은 원래 B). 오프셋 168/0/272 기준 ANGLE 단독 측정 여유(전 셀 통과):

    | 셀 | 경로 | 간격 | 여유(580 기준) |
    |---|---|---|---|
    | X=0 (E01, U03) | Y4 = 272, Y3 idle 860 | 588 | **8** — 가장 빠듯. 더 벌리려면 Safety를 ±870~880으로 |
    | K01 (−39.3) | Y4 = 232.7, Y3 860 | 627 | 47 |
    | J01 (41.8) | **Y3 = −230.2**(`> 0` 판정), Y4 −860 | 630 | 50 |
    | I01 (113.9) / D01·U02 (157) | Y3 = X−272, Y4 −860 | ≥ 702 | 충분 |
    | WAD X=0 | Y2 = −168, Y1 750 | 918 | 충분 |

    Y4 +272 / Y3 −230까지 실제 주행 실적은 없다(SW 한계 Y4 MX 360, Y3 MN −320 안) — 첫 가동 전 조그로 확인할 것. 화질은 간격 580이 되면서 단변 U01↔U05(627)도 짝이 된다. 배포 템플릿 `ConfigurationFile/Position.ini`(560/−560)·`CoreFunction.ini`는 손대지 않았다.
  - **⚠ 위 표는 580/±860/272 기준이며, 실사용 ini는 같은 날 17시경 사용자가 다시 바꿨다 (2026-09-15 확인).** `CoreFunction.ini` 17:29 `INTERLOCKHEADGAP = 400`, `Position.ini` 17:05 `SafetyHeadY3 = 690` / `SafetyHeadY4 = −760` / `HEAD1_ANGLE_Y = −359`(HEAD2는 +272 그대로, 비대칭). **400은 근접 센서 실측 569.1보다 작아** 모션 인터락이 통과시킨 간격에서 센서가 먼저 걸려 `DoWork_MotionDone NG(AXIS:9/10)`로 정지할 수 있다. 여유·짝짓기 수치(`dPairGap`)는 현재 값으로 다시 계산해야 하며, ini 값을 판단 근거로 쓸 때는 항상 파일을 직접 읽을 것.
- **`nMeasArea == 1`(그룹 1개)이면 두 헤드를 레시피 `m_fFrontHeadGap`/`m_fBackHeadGap`의 ±절반 위치로 강제 배치**한다(셀 좌표 무시). 그룹 수에 따라 이동 좌표계가 통째로 바뀌므로 로그 해석 시 주의.
- **(2026-09-14 해결) OCP에서 반대편 갠트리 헤드가 첫 이동에 셀 좌표로 따라 움직이던 문제.** ANGLE/VISION 첫 이동에 Y1/Y2가 함께 나가 Y1 +리밋의 원인이 된 적이 있다(2026-09-09 18:26 Y1 790.4). 지금은 아래 "코드 검토 결과" 2번대로 첫 이동의 반대편 헤드 슬롯을 `df_HEAD_Y_IDLE_SAFETY`로 보내 `[SAFETY]`에 세운다.
### 헤드 센서 배치와 HEAD_OFFSET (2026-09-14 사용자 도면 기준)

축 좌표 = 헤드 **기준 센서**의 위치. BACK 갠트리(Y1/Y2) 기준 = **Review**, FRONT 갠트리(Y3/Y4) 기준 = **VISION**. 다른 센서는 기준에서 Y로 떨어져 있고 **안쪽(글래스 중심 쪽)에 있으면 축 목표에 그만큼 더 가야** 하므로 부호는 다음과 같다(+측 헤드 = Y1/Y3, −측 = Y2/Y4). `GetHeadOffset(iHead, iMode)`의 `iMode`는 **0=WAD, 1=ANGLE, 2=VISION, 3=COLOR**(struct 주석 `1:Vision 2:Color 3:Angle`은 틀림 — `LinkIniFile` 기준).

| 센서 | 위치 | `Position.ini [HEAD_OFFSET]` (Y) | 2026-09-14 적용값 |
|---|---|---|---|
| Review (Y1/Y2 기준) | — | (REVIEW 키 없음, `MoveCmdPosReview`는 오프셋 0) | — |
| WAD | Review에서 **안쪽** 168 | `HEAD1_WAD_Y = +168`, `HEAD2_WAD_Y = −168` | 175/−178.35 → 168/−168 |
| VISION (Y3/Y4 기준) | — | `HEAD1/2_VISION_Y = 0` | −50/+50 → 0/0 |
| ANGLE | VISION에서 **바깥쪽** 272 | `HEAD1_ANGLE_Y = −272`, `HEAD2_ANGLE_Y = +272` | −50/+50 → −272/+272 (17:05에 사용자가 HEAD1만 −359로 변경) |

X 오프셋(`*_X`)은 아직 미정(−44/−50 그대로). 축 목표 = 셀 X + 오프셋 (+ 얼라인 오프셋). 두 헤드 목표 간격 = 셀 거리 + (o₊ − o₋) → **WAD는 셀 거리 + 336, VISION은 그대로, ANGLE은 셀 거리 − 544**. `INTERLOCKHEADGAP` 720 기준 짝 측정에 필요한 셀 거리: WAD ≥ 384, VISION ≥ 720, **ANGLE ≥ 1264**(1500 장변에서 A01↔T01 1378 한 쌍뿐).

**이 배치로 코드 검토 결과 (2026-09-14). 1~4 모두 같은 날 수정:**
1. **`MoveCmdPosAngle`이 ANGLE 오프셋을 Y1/Y2에, VISION 오프셋을 Y3/Y4에 건다** (HMI 시절 ANGLE=BACK 갠트리 기준). OCP는 ANGLE 측정 축이 Y3/Y4(`_Measure_ANGLE`의 dLeft2/dRight2)라 **ANGLE 센서가 셀에서 272 어긋난 채(VISION 자리) 측정**된다. 오프셋이 ∓50으로 같았을 땐 드러나지 않았다. → **수정**: `MoveCmdPosAngle` 오프셋 로드 직후 `IsOCP()`면 `dOffsetY[0..1]↔[2..3]` 교차(Y만, `dOffsetX[0]`는 갠트리용이라 유지).
2. **FRONT 측정(VISION/ANGLE/COLOR/VISION_COLOR) 첫 이동의 Y1/Y2가 셀 좌표 + ANGLE 오프셋으로 나간다**(`MoveCmdPosVision`은 SW 리밋 클램프 후, `MoveCmdPosAngle`은 그대로). ∓272면 Y1−Y2 간격 = 셀 거리 − 544 = 186 → `MOTION_INTERLOCK_7` 거부로 **VISION 첫 이동 자체가 실패**(∓50이던 오늘 ini도 630 < 720이라 이미 거부 대상). BACK 갠트리는 이때 `SafetyHeadX1`에 있으므로 Y1/Y2는 셀을 따라갈 이유가 없다 → **수정**: OCP면 첫 이동(`bMoveFirstDone==false`)의 반대편 헤드 슬롯 `stPosition[2]/[3]`을 `df_HEAD_Y_IDLE_SAFETY`로 — VISION/COLOR/VISION_COLOR의 Y1/Y2, WAD의 Y3/Y4(ANGLE 첫 이동은 원래 Y1/Y2가 주석 처리돼 있어 대상 아님). 수동 `ManualMoveCell`의 SPEC(Y3/Y4)·QUALITY/COLOR/ANGLE(Y1/Y2)도 동일(REVIEW는 `MoveCmdPosReview`에 치환 로직이 없어 제외).
3. **짝짓기(`MakeHeadPairByOrder`) 거리 조건이 오프셋을 모른다.** ANGLE은 729.77 쌍을 만들어 놓고 모션 계층에서 전부 거부된다 → **수정**: 각 스텝 짝짓기 직전 `dPairGap = dHeadGap − (HEAD1_Y − HEAD2_Y)`(`GetHeadOffset(0/1, mode)`, mode WAD 0·ANGLE 1·VISION 2·COLOR 3)를 `MakeHeadPairByOrder`에 넘긴다. 잔여 단독 패스의 더미 좌표는 원래 `dHeadGap` 그대로. 결과: WAD 384(단변 U01↔U05 627 짝 가능, U02↔U04 313.5는 단독), VISION 720, ANGLE 1264(전부 단독). 수동 `MakeMeasGroup`은 측정 종류를 모르는 공용 그룹이라 720 유지. 로그 `HeadPair by order … HeadGap=` 값이 이 dPairGap이다.
4. (→ 위 "FRONT 근접 센서 실측 569.1" 항목으로 해결) **ANGLE은 간격 720이면 중앙 셀을 못 잰다** — 단독 측정으로 돌려도 Y3=X−272 / Y4=X+272 이고 쉬는 헤드가 Safety(800/−800)에 있어도 `|X| < 192`인 셀(장변 260907 H01~M01 6개, 260720 D01/E01/F01, 단변 U02~U04)은 간격 미달. Safety를 SW 리밋(920/−884)까지 밀어도 J01/K01·U03은 불가. **FRONT 헤드 몸체 실제 최소 간격을 실측해 `INTERLOCKHEADGAP`(Y3/Y4용)을 낮춰야** 한다(중앙 셀 단독 측정엔 ≤ 528, 729.77 쌍 측정엔 ≤ 185). 낮출 땐 `MoveCmdPosMulti_Sub`가 Y1/Y2 검사에도 `m_dInterlockHeadGap`을 쓰므로(2026-07-16 변경) BACK은 `INTERLOCKHEADGAPB`로 되돌려야 한다.
5. WAD/VISION은 문제 없음: 장변 260907 WAD Y1 최대 858.4(865.4 도달 실적), Y2 최소 −856(−866 실적), VISION 쌍 간격 729.77 ≥ 720(여유 9.8); 260720 · 단변 단독 셀도 Safety 800/−800·750/−750으로 전부 통과.

- 셀 X 부호 규약: 짝짓기는 `X < 0` = −측 / `X >= 0` = +측(`MakeHeadPairByOrder`). **잔여 단독 셀의 담당 헤드는 2026-09-18 부터 부호가 아니라 '간격'로 고른다 — `PickSingleCellHead()`** (선언 `DoWorkGlassMeasureStep.h`, 정의는 `MakeHeadPairByOrder` 바로 위). 두 후보(+측 Y1/Y3 가 맡는 경우 / −측 Y2/Y4 가 맡는 경우)에 대해 **목표 = 셀 X + 그 헤드의 HEAD_OFFSET Y** 를 계산해 ① 그 축 SW 리밋 안인지 ② 반대 헤드 `[SAFETY]` 와의 간격이 얼마인지 보고 **갈 수 있고 간격이 큰 쪽**을 고른다. 둘 다 범위 밖이면 구 부호 규칙(`> 0`)으로 되돌아가며 WARN 을 남긴다. 적용 6곳: Measure 스텝 WAD(mode 0, BACK Y1/Y2)·VISION(2)·COLOR(3)·ANGLE(1)·VISION_COLOR(2, 이상 FRONT Y3/Y4) + TestMode(`RecipeEditMgr`, Review 는 HEAD_OFFSET 키가 없어 레시피 `m_fHeadReviewLeft/Right` 를 넘긴다). 구 `> 0` 줄은 각 자리에 `//` 로 남겼다. **구 규칙과 달라질 때만** 로그 `[<태그>] PickHead cellX=… : Head1 -> Head2 (gap Head2 … / Head1 …)`. 구 기준 `50`은 2026-09-14에 폐기.
  - **현재 설정에서 실제로 담당이 바뀌는 셀은 VISION/COLOR 의 중앙 근처 1개뿐이다** (2026-09-18 두 레시피 전수 계산): 90도 `K01`(X=−39.3) 과 180도 `U03`(X=0) 이 Head1 → Head2. VISION/COLOR 는 오프셋이 0/0 인데 `SafetyHeadY3`(695)·`SafetyHeadY4`(−790) 가 비대칭이라 Y3 쪽 간격이 더 크기 때문이다(둘 다 구 규칙에서도 통과하던 셀이라 **측정 가부가 아니라 어느 카메라가 재느냐만 바뀐다**). WAD/ANGLE 은 변경 0건. `dGlassConactPos`(= 레시피 `m_fHeadReviewY`)가 갠트리 목표에 더해지고, 반대 헤드 갠트리는 `± dGantryGap`로 산출된다.


- **`MoveCmdPosAging` 도 쉬는 헤드 마커를 처리한다 (2026-09-21).** 헤드 이동 6함수 중 이것만 `ApplyIdleHeadSafetyY` 가 없어서, **PRE_MEAS `_Move`**(BACK 갠트리 X2 기준 리뷰 이동)가 FRONT 헤드 Y3/Y4 에 BACK 과 같은 셀 좌표(`dLeft`/`dRight`)를 넣어 보냈고 셀 716.397 에서 **Y3 SW 리밋(MX 697) 3901** 이 났다(09-19 19:06·09-21 17:39, PRE_MEAS 이동은 기록된 3회 모두 실패). 이제 `MoveCmdPosAging` 이 Y 멀티 직전에 `ApplyIdleHeadSafetyY(…, _T("MoveCmdPosAging"))` 를 부르고, PRE_MEAS 는 OCP 면 Y3/Y4 를 `df_HEAD_Y_IDLE_SAFETY` 로 넘긴다 → Y3 695 / Y4 −795, X3 는 `MoveCmdPosAging` 의 X2 분기가 `SafetyHeadX2` 로 보낸다. AGING 호출(X3 기준, Y3/Y4 가 실제 측정 헤드)은 마커를 안 쓰므로 영향 없음. 로그 `MoveCmdPosAging idle head Y3 -> SAFETY 695.000`.
- **WAD 짝 측정이 한 번도 안 되던 원인 = 그룹 생성 재검사가 오프셋을 몰랐다 (2026-09-21 WAD 만 수정).** 짝은 두 번 검사된다: ① `MakeHeadPairByOrder` 는 09-14 부터 `dPairGap = dHeadGap − (HEAD1_Y − HEAD2_Y)`(WAD 765.6 − 336 = 429.6)로 판정해 A01↔K01 … J01↔T01 10쌍을 만든다. ② 그 뒤 그룹 생성 루프가 `if (fabs(셀X1 − 셀X2) > dHeadGap)` 로 **오프셋 없는 셀 거리**를 다시 비교해, 셀 거리 729.77 < 765.6 이라 10쌍을 전부 깨고 잔여 단독 패스로 보냈다 → 20셀 단독 측정(2026-09-21 18:01: 로그 `HeadPair by order … pair=10 single=0` 인데 모션은 `idle head Y2 -> SAFETY` 반복, WAD 송신 `51302`(A~J)/`51301`(K~T)). 실제 헤드 간격은 858.215 − (−207.558) = 1065.77 이라 모션 인터락(HEADGAPB)은 통과한다. 수정: WAD `Run()` 함수 범위에 `dPairGapWad` 를 두고 ② 도 이 값으로 비교(구 줄 `//`). 결과 순서 K01+A01 → L01+B01 → … (10그룹). **VISION/COLOR/ANGLE/VISION_COLOR 에도 같은 ② 가 한 벌씩 남아 있다**(오프셋 0 이거나 ① 이 더 엄격해 현재 동작 차이는 없음) — 오프셋을 바꾸면 같은 문제가 생기니 그때 함께 고칠 것.
