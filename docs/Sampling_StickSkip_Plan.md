# Sampling 스틱 skip (OCP) — Sampling 선택 셀이 없는 스틱은 이동·컨택·OST·R-TEST·AGING 생략

작성: 2026-10-08 (LYS). 상태: **구현·빌드(사용자 10-08 11:44) 완료, OOS 1차 OK, 미커밋.**

## OOS 1차 (2026-10-12 판독, 10-08 런)
- 11:52:11 `1_OCP_TEST_Long_260907` CYCLE(Sampling ON, WAD·VISION·ANGLE): 엔진 로그 `Sampling: stick 2 skipped ...`, `Sampling: stick 3 skipped ...`(번호는 1부터). 측정 로그에서 이 두 스틱은 init 스텝(`CDoWorkGlassMeasureStep_None Start`)만 있고 MoveMeasureArea·Probe_Contact·PtrnSearch·OST·RTEST·AGING·측정 스텝이 없다. 나머지 3스틱은 정상 측정, WARN 0. `Resume Update: Stick=2/5`·`3/5`가 skip 직후 수 ms 안에 찍힌다.
- 선택 셀 배치 대조는 못 함 — 레시피가 11:54:07에 다시 저장돼 그 런의 Sampling 선택이 남아 있지 않다.
- 11:54:18 같은 레시피 5스틱 모두 측정 런은 10-06 09:39 런(이 변경 전 빌드)과 로그 17개 diff 0(사용자의 수동 Cell ID 표 수정분 5개 제외) — 스틱이 skip되지 않을 때 동작 불변 확인.
- 남은 OOS: (8) O3(특정 스틱만 해제 후 선택 셀 대조)·O6(전부 해제 → WARN)·O8a(EFA 글라스), 실기 R1·R1-EFA(VISION 대기).

## 구현 결과 (2026-10-08)

- **요청:** Sampling 옵션 사용 시 WAD·VISION·ANGLE 모드에서 해당 스틱이 전부 미측정이면 OST·R-TEST·AGING(컨택 포함)을 하지 않는다.
- **사용자 결정:** D-SS-1 스틱 전체(이동·패턴 서치 포함, OCP `nGroup < 1 → continue`와 동일), D-SS-2 OCP만, D-SS-3 (b1s) AgingTest·EFA 글라스에도 스틱 단위 적용(셀마다 "마스크 안 ∧ Sampling 선택"), D-SS-4 **1셀 스틱은 지금처럼 측정**(설계 권장안과 다름), D-SS-5 모든 스틱 skip 시 WARN 1줄, D-SS-6 마지막 스틱 skip 시 VISION 결과 대기(시간 초과 = WARN 후 진행), D-SS-7 전기 검사만 하는 레시피는 skip 안 함, D-SS-8 하이브리드 세그먼트 경계 유지.
- **수정 파일** (전부 CP949 CRLF, 삽입만):
  - `Source/ALPHA/DoWork/MeasureSequenceItemEngine.h/.cpp`: `IsSamplingStickSkip(int)`(일반 글라스), `SamplingCycleSkip()` + `IsSamplingCycleSkip()`(AgingCycleBegin 안, 3c 판정 다음).
  - `Source/ALPHA/DoWork/DoWorkGlassMeasure.cpp`: 스틱 루프의 `bAgingStickSkip` 줄 다음 `bSamplingSkip` 훅(3c skip 경로 재사용), 카운터와 전부 skip WARN, VISION 대기 람다 `SsVisionWait`(글라스 끝 + EFA 비최종 패스 끝).
- **로그** (엔진 로그 `measure_seq_item_engine_*.log` OPER):
  - `Sampling: stick %d skipped - no selected cell for the enabled items (%s) - no move/contact/OST/R-TEST/AGING%s`
  - `Sampling: stick %d skipped - no selected cell for the cycle items in [%s] (%s) - no move/contact/OST/R-TEST/AGING%s`
  - 측정 로그 WARN: `Sampling: %s ends on a skipped stick - VISION result wait timed out ...`, `Sampling: no stick of this glass has a selected cell ...`, `Sampling: every stick of this glass was skipped (Sampling %d, cycle mask %d) ...`
- **불변:** Sampling OFF, 비OCP, 전기 전용, 모든 스틱에 선택 셀이 있음, 1셀 스틱, 3c 마스크 skip — 상태 변경·로그 없음(리뷰 확인).
- **리뷰:** 불변성 지적 0. 수정 2건(EFA T0 flush 전 VISION 대기 누락 → 추가, 3c+Sampling 혼합 시 WARN 누락 → 카운터), 주석 2건, 기각 1건. 참고: 사이클 글라스에서 레시피 광학 플래그가 모두 꺼져 있어도 사이클 항목이 광학 측정이면 Sampling skip이 적용된다(스텝이 항목 플래그로 측정하므로 같은 기준).
- **알려진 틈(기존, 범위 밖):** 3c 마스크 skip으로 끝나는 글라스·EFA T0 flush에는 VISION 대기가 없다(F1). 셀 1개 스틱의 남은 셀 측정은 Sampling을 보지 않는다(F2).
- **OOS:** 아래 (8) — O1 회귀(Sampling 0) 먼저, O3 스틱 3 해제, O6 전부 해제(WARN), O8a EFA 복사본(EFA rcp도 복사), 실기 R1·R1-EFA(VISION 대기는 OOS로 확인 불가).
- 변경 기록·패치 스크립트·백업: `D:\OCP\0.SW_Matching\work\hybrid_session_20261006\`(`ss_impl_changes.md`, `ss_fix_changes.md`, `ss_impl\`, `ss_bak\`).

---

## 설계 원문 (2026-10-08, 리뷰 반영본)

**사용자 결정(2026-10-08): D-SS-1 스틱 전체(이동·패턴 서치 포함), D-SS-2 OCP만, D-SS-3 (b1s) AgingTest/EFA 글라스에도 스틱 단위 적용, D-SS-4 1셀 스틱은 지금처럼 측정(skip 제외 — 권장안과 다름), D-SS-5~8 권장안.**

# Sampling 스틱 skip 구현 계획 (설계 전용, 2026-10-08)

요청(사용자 원문): "Sampling 옵션 사용 시 WAD,Vision,Angle Mode에서 해당 Stick 전부 미측정 시 OST,RTEST,AGING 동작(Contact 포함) 하지 않도록 수정해줘"

입력: `ss_map_flow.md`, `ss_map_rule.md`, `ss_decision.md`(스틱 전체 skip, Phase 5 워크플로 이후 적용 - 스크래치패드 기록, 사용자 재확인 대상).
이 계획을 쓰는 동안 소스/.rc/.ini/레시피는 하나도 건드리지 않았다. 아래 앵커와 줄 번호는 2026-10-08 트리에서 Perl `index()`로 raw 바이트를 다시 세어 확인한 값이다.
~~`DoWorkGlassMeasure.cpp`의 mtime은 아직 10-01이다.~~ **(10-08 10:2x 재확인, (10) 참조)** Phase 5 P15가 이미 들어갔다(mtime 10-08 10:06:20, CRLF 1124/1124, damaged 0, BOM 없음). `DoWorkGlassMeasureStep.cpp`도 09:59:40에 다시 바뀌었다(CRLF 7105/7105). 아래 본문의 줄 번호는 (10)에서 갱신한 값을 우선한다. 엔진 두 파일은 09-30 그대로다.
리뷰(8건) 반영: 본문 해당 절을 고치고, 근거와 처리 내역은 **(10) 리뷰 반영**에 모았다.

---

## (0) 요약

- **동작.** OCP 장비, 표준 시퀀스(AgingTest/EFA 사이클 아님), 활성 세그먼트의 Sampling이 ON인 조건에서, 켜진 광학 모드(WAD=Spec_3, VISION=Quality_4, ANGLE=Angle_6) 어느 것으로도 선택된 셀(`m_nSelectCell[mode]==1`)이 없는 스틱은 **init 스텝만 돌고 건너뛴다**. 이동, 패턴 서치, Contact, OST, R-TEST, AGING, 측정 모두 하지 않는다. OCP `DoWorkStartMeasOCP`의 `nGroup = aryMeasGroup.GetSize(); if(nGroup < 1) continue;`(JOB_Proc.cpp:6210-6212, 접촉 위치 이동 전)와 같다.
- **구현 방식.** 이미 OOS 검증을 마친 Hybrid 3c skip 경로(`bAgingStickSkip`, ThreadRun)를 **그대로 재사용**한다. 새 코드는 셋이다.
  - 판정 함수 `CMeasureSequenceItemEngine::IsSamplingStickSkip(int nStick)`: 엔진 .h/.cpp에 추가. Phase 5는 이 두 파일을 건드리지 않는다.
  - ThreadRun 훅: `bAgingStickSkip` 선언 바로 뒤에 별도 지역 플래그 `bSamplingSkip`을 두고, 참이면 `bAgingStickSkip`도 세운다(리뷰 #1). Phase 5 앵커 목록에 없는 줄이다.
  - **글라스 끝 VISION 결과 대기(리뷰 #1, high).** 마지막 스틱 skip 분기(`GoToExitStep();` 직전)에서 `bSamplingSkip`일 때만 `CmQualityEmptyTimeOut(LAST_ITEM, ...)`을 부른다. HMI의 "모든 결과 대기"는 VISION 스텝 안(마지막 스틱의 마지막 그룹)에만 있어서, 꼬리 스틱이 skip되면 직전 측정 스틱의 VISION 결과가 저장 전에 다 들어오지 않을 수 있다. OCP는 스틱 루프 뒤 글라스 레벨에서 기다린다(2-3b). 같은 블록에서 D-SS-5 권장안(전 스틱 skip WARN)도 낸다.
- **바뀌지 않는 것.** 스텝 파일(`DoWorkGlassMeasureStep.cpp`), 레시피 DLL/`preDefine.h`, .rc, .ini, ResumeInfoMgr, 결과/CIM 경로.
- **불변 보장.**
  - Sampling OFF, 비OCP, 광학 항목이 없는 전기 전용 레시피, 사이클/EFA 글라스: 판정이 첫 검사에서 false(로그 없음)를 내거나 아예 호출되지 않는다. 동작과 로그가 바이트 단위로 같다.
  - Sampling ON이고 모든 스틱에 선택 셀이 하나 이상 있으면 역시 false라서 동일하다.
- **새 로그.** skip된 스틱마다 엔진 로그에 OPER 1줄이 남는다. `Sampling: stick %d skipped - no selected cell for the enabled items (%s) - no move/contact/OST/R-TEST/AGING%s`. 글라스가 skip 스틱으로 끝날 때만 측정 로그에 WARN이 최대 2줄(VISION 대기 타임아웃, 전 스틱 skip) 남을 수 있다((4)).
- **AgingTest/EFA 글라스(리뷰 #2).** 권장안 (a)(미적용)는 Sampling ON 사이클 글라스에서 요청을 **충족하지 못한다**. D-SS-3을 다시 썼다((7)).
- **적용 시점.** Phase 5 5-4(시퀀스) 패치가 들어가고 5-5 빌드/OOS를 통과한 다음, 앵커를 다시 세고 적용한다.

---

## (1) 규칙

### 1-1 판정식 (스틱 루프의 스틱 `i`마다, `AgingCycleBegin(...)` 직후, 활성 세그먼트 전환 이후에 평가)

```
Skip(i) =  IsOCP()                                                           // D-SS-2
        && bAgingStickSkip == false && bAgingCycle == false                  // 3c skip / 사이클(AgingTest 항목, EFA pass)이 아닐 때만 (D-SS-3)
        && Info(active).m_sOptionItem.m_bUseSampling != FALSE                // Sampling OFF -> 절대 skip 안 함
        && E != {}                                                           // 광학 항목 없음(전기 전용) -> 절대 skip 안 함
        && 0 <= i < Data(active).m_vecStick.size()
        && for all c in Data(active).m_vecStick[i].m_vecCell :
               for all k in E : c.m_nSelectCell[M_k] != 1                    // 스텝과 같은 "미선택" 판정(!= 1)
E = { WAD    if m_sInspectItem.m_bUseSpec_3    != FALSE  (M_SPEC    = 0)
      VISION if m_sInspectItem.m_bUseQuality_4 != FALSE  (M_QUALITY = 1)
      ANGLE  if m_sInspectItem.m_bUseAngle_6   != FALSE  (M_ANGLE   = 3) }
```

- 모드는 OR로 묶는다. 예를 들어 WAD 선택 셀이 하나라도 있으면, VISION/ANGLE 선택이 없어도 skip하지 않는다. 그 스틱은 WAD 측정 때문에 접촉이 필요하다.
- `i`는 활성 세그먼트의 로컬 스틱 인덱스다. 스텝이 `pRecipeData->m_vecStick[GetMeasureStick()]`을 읽는 것과 같은 도메인이며, 회전 반전도 `ToCellStatusStick`도 없다. SelectCellDlg도 같은 raw 인덱스에 쓴다.
- 검증한 근거(코드):
  - WAD는 `bUseMeas = ...m_bUseSpec_3;`, `bUseSampling = ...m_sOptionItem.m_bUseSampling;`, `m_nSelectCell[M_SPEC] != 1`(DoWorkGlassMeasureStep.cpp, 10-08 09:59 기준 :2199/2220/2291/2327/2351).
  - VISION은 `m_bUseQuality_4`(:3183), `M_QUALITY`(:3267/3303/3327).
  - ANGLE은 `m_bUseAngle_6`(:4718), `M_ANGLE`(:4806/4842/4866).
  - 타입: `BOOL m_bUseSampling`(preDefine.h:161), `BOOL m_bUseSpec_3/Quality_4/Angle_6`(:119/120/122), `int m_nSelectCell[M_MODE_MAX]`(:371). 생성자에서 0이 들어가고, `_CELL_SELECT_`는 버전 가드 없이 스틱당 4행(SPEC/QUALITY/COLOR/ANGLE)으로 읽힌다(OutRecipeFunc.cpp:342-364).
- E에 넣지 않는 항목:
  - COLOR_5 / TSP_7 / VISION_COLOR: 스텝 목록에서 주석 처리되어 실행되지 않는다.
  - LCR_8: ANGLE 그룹 루프 안에서만 실행되므로(:5384 부근 `i == 0 && bUseMeasLCR`) ANGLE 선택이 이미 포함한다.
  - Elect_1 / RTest / AgingTest_2: 전기 항목이고 요청상 skip 대상이다.
- **VISION과 `USE_VISION_COLOR`.** VISION은 레시피 플래그만 본다. `IsUseInspVisionColor()` 항은 넣지 않는다.
  - 근거 1: 이 키는 PARAM에 없어서 기본값 false이고 실영향이 없다.
  - 근거 2: 키가 1이면 지금도 품질을 재는 스텝이 없는 비정상 구성이다. 그때 VISION 선택만 있는 스틱은 오늘처럼 접촉한다. 즉 요청 문구보다 더 많이 skip하는 일이 없다.
  - 근거 3: 나중에 VISION_COLOR 스텝이 복원되면 그 스텝도 같은 `M_QUALITY` 선택을 쓴다. 이 항을 넣으면 그때 잘못 skip하게 된다.
  - 이 점에서 `ss_map_rule.md` E-D5(스텝 미러)와 다르게 정했다.

### 1-2 엣지 케이스

| # | 경우 | 동작 | 비고 |
|---|---|---|---|
| E1 | Sampling OFF | skip 없음, 새 로그 없음 | 판정이 Sampling 검사에서 false를 내고 로그 전에 반환 |
| E2 | Sampling ON, 모든 스틱에 선택 셀 1개 이상 | 동일 | 셀 루프 중간에 false |
| E3 | Spec_3 / Quality_4 / Angle_6이 모두 OFF (전기 전용, COLOR/TSP/LCR만 켜진 경우 포함) | skip 안 함 | **설계 해석(사용자 확인 D-SS-7)**. 요청 문구 "WAD,Vision,Angle Mode에서"는 광학 모드가 하나도 켜지지 않은 레시피에는 해당하지 않는다고 본다. 이 경우 OCP는 Sampling과 상관없이 전 스틱을 skip하지만(광학/LCR 그룹이 없으면 `nGroup`이 항상 0) HMI는 다르게 간다. `NeedOcpElectContact`가 접촉을 유지한다. (종전 "하드 룰" 표기는 워크플로 스크립트의 규칙이었지 사용자 지시가 아니었다 - 리뷰 #6) |
| E4 | Sampling ON + 선택 데이터 전부 0 | 전 스틱 init만, 마지막 스틱에서 Idle(완료 이벤트) 1회, Result/Unload 진행, 알람 없음. **권장안(D-SS-5): 측정 로그 WARN 1줄** | OCP와 같다(전부 `continue`). `_CELL_SELECT_`는 MakeCell이 0으로 만들고 버전 가드 없이 읽으므로, Sampling만 켜고 셀 선택을 안 한 레시피가 이 경우에 빠진다. 지금은 그래도 전 스틱 OST/AGING이 돌지만, 변경 뒤에는 전기·광학 데이터 0으로 정상 완료된다(저장 게이트는 플래그 기준이라 `SaveResult`도 돈다). 품질 유출 위험 - 리뷰 #4 |
| E5 | 마지막 스틱 skip | 3c 경로: `GoToExitStep(); continue;` → `SegmentRestore0()` → Idle → `CDoWorkGlassMeasure Thread End`. **Sampling skip이면 `GoToExitStep()` 직전에 VISION 결과 대기(2-3b)** | Phase 5 마지막 스틱 훅(`TupleStickDone`)도 이 경로에 있다. 대기가 `TupleStickDone`보다 앞이므로 저널 커밋에 대기 중 들어온 VISION 줄이 포함된다 |
| E6 | 셀 1개 스틱, 미선택 | skip | 지금은 잔여 셀 패스(`// 선택하고 남은것`, WAD :2389 / VISION :3364 / ANGLE :4899)가 `if (nArea > 1)` 밖에 있어서 Sampling 검사 없이 **측정한다**(코드로 확인함. 08-18 메모리의 "미측정"은 틀렸다). 1셀 스틱만 동작이 바뀐다(D-SS-4). 현 TEST 레시피는 스틱당 20/3/4셀이라 영향이 없다 |
| E7 | AgingTest 사이클 / EFA pass가 활성인 스틱 | D-SS-3에 따라 다르다. (a)면 미적용(3c 마스크 규칙 `AgingItemHasCell`만), (b1s)/(b1)이면 사이클 항목 기준 Sampling 판정 | 사이클 중에도 스텝은 마스크(`IsAgingCellSkip`)와 Sampling(`m_nSelectCell != 1`)을 **함께** 적용한다(VISION :3265-3268 등). 그래서 (a)면 Sampling ON 사이클 글라스의 0셀 스틱이 USE_AGING 항목마다 이동/접촉/OST/AGING을 하고 아무것도 측정하지 않는다. 이것이 사용자가 말한 증상 그 자체다(리뷰 #2) |
| E8 | `AgingCycleBegin`이 false를 반환하고 skip 표시도 없음 (AgingTest OFF, 래치 0, rcp 없음, USE_AGING 항목 없음) | 규칙 적용 | 스텝이 레시피 플래그를 읽는 바로 그 글라스들이다 |
| E9 | 하이브리드 | 세그먼트별 자체 Sampling, 검사 플래그, 선택을 본다(`ValidateSegments`는 이 값들을 같게 강제하지 않는다) | 훅은 `SegmentBoundary` 뒤에 있다. `GetRecipeInfo()/GetRecipeData()`는 활성 세그먼트를 돌려준다 |
| E10 | 한 세그먼트의 스틱이 전부 skip | 세그먼트 경계(회전 + `MoveCmd_SafetyAll`, `.alg`/재Align)는 그대로 수행한 뒤 스틱만 skip | 경계는 글라스 레벨 상태 전환이다. `bTurn`이 레시피 회전값에서 나오므로(DoWorkGlassMeasure.cpp :734) 데이터만 바꾸는 전환으로 대신하면 이후 경계가 어긋난다. **수용 비용(D-SS-8, 리뷰 #5):** 그 경계의 Glass SW Align이 NG면(:852-854) 그 세그먼트가 아무것도 재지 않는데도 글라스가 중단된다. 문서에도 적는다 |
| E11 | Resume (레거시 N=1, Phase 5 튜플 모두) | skip된 스틱도 3c처럼 "완료"로 진행 저장. 재개하면 같은 레시피로 다시 판정하므로 결과가 같다 | 튜플 북마크(Format 2)는 rcp 지문이 바뀌면(선택을 바꾸면) 재개를 거부한다(Phase 5 #4). **레거시(Format 1, N=1) 북마크에는 지문이 없다**(`m_strFingerprint`는 Format 2 전용, ResumeInfoMgr.h:135/149). STOP과 재개 사이에 선택을 바꾸면, skip되어 완료로 기록된 스틱은 다시 돌지 않는다. 기존 레거시 동작이고 범위 밖(리뷰 #7) |
| E12 | skip된 스틱의 init 시점에 STOP이 대기 중 | 정상 STOP 경로 (`GetStopUser() != true` 가드) | 기존 3c와 같다. None이 `SetStopUser(false)`를 하는(DoWorkGlassMeasureStep.cpp :572) 기존 경합은 범위 밖. **skip만 이어지는 런에서는 거의 전 구간이 스틱 사이 틈이라 STOP 유실 가능성이 더 크다**(런 길이 약 0.1~0.3 s, 리뷰 #8). 변경 없음, O10에 명시 |
| E13 | 직전 측정 스틱에서 Z5가 접촉 상태 | skip 스틱은 무동작. 다음 측정 스틱의 MEAS MOVE에서 `bMoveStage`가 true가 되어 `MoveCmdPos_ProbeUnContact(AXIS_PROBE_Z5, ...)` 후 이동. 마지막 skip이면 Unload `MoveCmd_SafetyAll`까지 지금과 같다 | OCP는 매 스틱 `DoWork_Move(AXIS_UNIT_Z, dCentactZ[0])`로 Z를 올린다. HMI는 그럴 필요가 없다 |
| E14 | 비OCP HMI 장비 | skip 안 함 | D-SS-2 |
| E15 | PreMeas P2P (`PG_TEST_1`, `m_bUsePre_0`) | 변화 없음 | HMI P2P는 글라스 레벨 스텝이다(PreMeas, 접촉 없음). OCP는 스틱별 PTP(`ExeSubMeas(iTop,0)`)도 함께 건너뛰지만 HMI에는 해당하는 스틱별 동작이 없다 |

참고(범위 밖, 기존): `m_bUseRTest`는 어느 로더나 편집기도 쓰지 않아 항상 FALSE다(preDefine.h:117/142만 있음). 그래서 측정 스틱에서도 R-TEST 스텝은 `CDoWorkGlassMeasureStep_RTEST Start.`만 남기고 `PG_TEST_3`를 보내지 않는다. OOS 기대값에 반영했다.

---

## (2) 삽입 위치와 코드 텍스트

세 파일 모두 BOM 없는 **CP949, CRLF**이다. 2026-10-08 Perl 확인 결과는 다음과 같다.
- `DoWorkGlassMeasure.cpp`: crlf 1124 = lf 1124, damaged 0 (Phase 5 P15 적용 뒤, 10-08 10:06:20. 적용 전 값은 777)
- `MeasureSequenceItemEngine.cpp`: crlf 1090 = lf 1090, damaged 0
- `MeasureSequenceItemEngine.h`: crlf 136 = lf 136, damaged 0

payload는 ASCII만 쓰고 CRLF로 넣는다. 패치는 Perl raw 바이트로 하며, 스크립트 파일로 실행하고 앵커마다 `index()`를 두 번 불러 유일한지 확인한다.

### 2-1 `HMI/Source/ALPHA/DoWork/MeasureSequenceItemEngine.h` (public 블록, 선언 1줄)

앵커(n=1, :103, 줄 끝까지 ASCII):
`IsAgingStickSkip() const\t{ return m_bAgingStickSkip; }\t// LYS 2026-09-30 AgingCycleBegin(nStick >= 0) found no USE_AGING item with a cell on the stick\r\n`
이 앵커의 **바로 뒤**, 즉 `\tvoid\t\t\t\tAgingCycleEnd();\r\n`(n=1, :104) 앞에 넣는다.
```cpp
	bool				IsSamplingStickSkip(int nStick);	// LYS 2026-10-08 standard sequence + recipe Sampling on + no cell of the stick selected for the enabled WAD/VISION/ANGLE items (active segment) -> ThreadRun skips the stick like the 3c skip (OCP nGroup < 1). Logs the skip
```
- :102의 `AgingCycleBegin` 선언 줄은 CP949 주석을 담고 있으니 건드리지 않는다.
- 멤버 변수는 추가하지 않는다. 클래스 레이아웃이 그대로이고, 이 클래스는 ALPHA.exe 내부 전용이라 ABI 문제도 없다.

### 2-2 `HMI/Source/ALPHA/DoWork/MeasureSequenceItemEngine.cpp` (정의)

앵커(n=1, :1072): `void CMeasureSequenceItemEngine::AgingCycleEnd()\r\n`, 그 **앞**에 넣는다. 바로 위 :1070-1071은 `}\r\n\r\n`이므로 payload는 빈 줄 하나로 끝낸다.
- 필요한 include는 이미 있다: `..\Recipe\RecipeEditMgr.h`(:8), `..\Manager\SystemConfigMgr.h`(:22).
- `S_RecipeData`는 `CheckAgingMaskShape`에서 이미 쓰고 있어서, `preDefine.h`의 `M_SPEC/M_QUALITY/M_ANGLE`과 `S_CellInfo`도 보인다.
- `IsOCP()`(SystemConfigMgr.h:93)와 `GetSegLogTag()`(RecipeEditMgr.h:125, const, 1세그먼트면 "")가 있음을 확인했다.
```cpp
// LYS 2026-10-08 Sampling stick skip (OCP DoWorkStartMeasOCP: nGroup < 1 -> continue, before the contact move).
//   Standard sequence only: an AgingTest item / EFA pass (or the 3c skip) already decided the stick - ThreadRun also checks bAgingCycle.
//   Recipe Sampling on + at least one of WAD (m_bUseSpec_3 / M_SPEC), VISION (m_bUseQuality_4 / M_QUALITY), ANGLE (m_bUseAngle_6 / M_ANGLE)
//   + no cell of the stick selected for any of them (m_nSelectCell == 1, the steps' own test) -> true: ThreadRun runs only the init step
//   (no move, pattern search, contact, OST, R-TEST, AGING, measure). Active segment, segment-local stick, no rotation reversal.
//   Not OCP / Sampling off / no optical item (electrical-only recipe) / stick out of range -> false, no log (sequence unchanged).
bool CMeasureSequenceItemEngine::IsSamplingStickSkip(int nStick)
{
	if (m_MgrPool_link.pSystemConfigMgr == nullptr || m_MgrPool_link.pSystemConfigMgr->IsOCP() == false) return false;
	if (m_bAgingCycleActive || m_bAgingStickSkip) return false;
	if (GetRecipeEditMgrPtr() == nullptr) return false;

	S_RecipeInfo* pInfo = GetRecipeEditMgrPtr()->GetRecipeInfo();
	S_RecipeData* pData = GetRecipeEditMgrPtr()->GetRecipeData();
	if (pInfo == nullptr || pData == nullptr) return false;
	if (pInfo->m_sOptionItem.m_bUseSampling == FALSE) return false;

	bool bWad = (pInfo->m_sInspectItem.m_bUseSpec_3 != FALSE);
	bool bVis = (pInfo->m_sInspectItem.m_bUseQuality_4 != FALSE);
	bool bAng = (pInfo->m_sInspectItem.m_bUseAngle_6 != FALSE);
	if (bWad == false && bVis == false && bAng == false) return false;	// electrical-only recipe: the stick is always contacted
	if (nStick < 0 || nStick >= (int)pData->m_vecStick.size()) return false;

	const std::vector<S_CellInfo>& vecCell = pData->m_vecStick[nStick].m_vecCell;
	for (size_t c = 0; c < vecCell.size(); c++)
	{
		if (bWad && vecCell[c].m_nSelectCell[M_SPEC] == 1)		return false;
		if (bVis && vecCell[c].m_nSelectCell[M_QUALITY] == 1)	return false;
		if (bAng && vecCell[c].m_nSelectCell[M_ANGLE] == 1)		return false;
	}

	CString strItems;
	if (bWad) strItems += _T("WAD,");
	if (bVis) strItems += _T("VISION,");
	if (bAng) strItems += _T("ANGLE,");
	strItems.TrimRight(_T(','));

	CString strSkip;
	strSkip.Format(_T("Sampling: stick %d skipped - no selected cell for the enabled items (%s) - no move/contact/OST/R-TEST/AGING%s"),
		nStick + 1, strItems.GetString(), GetRecipeEditMgrPtr()->GetSegLogTag().GetString());	// 1 segment: empty tag
	m_pLogFileAccessor->KMLOG(OPER, strSkip);	// measure_seq_item_engine_YYYYMMDD.log, next to the 3c "AgingCycle: stick n skipped" lines
	return true;
}

```
- 판정은 상태를 바꾸지 않는다(로그 한 줄만 남긴다). 엔진의 `m_bAgingStickSkip`도 세우지 않는다. 그래서 `IsAgingStickSkip()`의 의미(3c 전용)는 그대로이고, `AgingCycleEnd()`의 정리 동작도 바뀌지 않는다.
- `m_bAgingCycleActive || m_bAgingStickSkip` 검사는 방어용이다. 훅의 `bAgingCycle == false` 검사와 겹친다. 다른 호출처가 생겨도 사이클 스틱을 잘못 skip하지 않게 해 둔다.

### 2-3 `HMI/Source/ALPHA/DoWork/DoWorkGlassMeasure.cpp` ThreadRun 훅 (Phase 5 적용 뒤)

앵커(2줄 연속, n=1, Phase 5 P15 적용 뒤 현재 :360-361, 들여쓰기는 탭 4개):
```
\t\t\t\tbool bAgingStickSkip = (m_pMeasSeqItemEngine != NULL && m_pMeasSeqItemEngine->IsAgingStickSkip());\r\n
\t\t\t\tif (bAgingStickSkip == false) CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_STEP_MEAS_N, true, 0);\r\n
```
두 줄 **사이**에 넣는다. 반드시 MCC 줄보다 앞이어야 skip된 스틱이 MCC `MCC_A_STEP_MEAS_N`을 내지 않는다(OCP도 `continue`가 `SetMccEvt`보다 앞이다).
```cpp
				// LYS 2026-10-08 Sampling: standard sequence and no selected WAD/VISION/ANGLE cell on this stick -> the same init-only skip as above (OCP nGroup < 1)
				bool bSamplingSkip = (bAgingStickSkip == false && bAgingCycle == false && m_pMeasSeqItemEngine != NULL && m_pMeasSeqItemEngine->IsSamplingStickSkip((int)i));
				nSsStickRun++;
				if (bSamplingSkip) { bAgingStickSkip = true; nSsSkipRun++; }
```
- (리뷰 #1) `bSamplingSkip`을 **별도 지역 플래그**로 남기는 이유: 2-3b의 글라스 끝 VISION 대기를 Sampling skip에만 걸어 3c 경로를 바이트 단위로 그대로 두기 위해서다. 스틱 반복마다 새로 선언되므로 다음 스틱으로 넘어가지 않는다.
- `nSsStickRun / nSsSkipRun`은 D-SS-5 권장안(전 스틱 skip WARN)용 카운터다. 선언은 2-3c. D-SS-5에서 "경고 없음"을 고르면 이 두 줄과 2-3c를 빼고 `if (bSamplingSkip) bAgingStickSkip = true;`만 둔다.
- 이후 경로는 수정 없이 재사용한다(줄 번호는 10-08 10:06 기준).
  - 마지막 스틱: `if (bAgingStickSkip && GetCurStep() == nInitStep && GetStopUser() != true)` → `GoToExitStep(); continue;`(:407-411). 여기에만 2-3b를 넣는다.
  - 그 밖의 스틱: 같은 조건에서 `break;	// Next Stick`(:458-461)
  - 루프 머리: Phase 5 `if (bTupleGlass && bTupleLastSaved == false && GetCurStep() == GetExitStepIndex()) { TupleStickDone(...) }`(:368), `SegmentRestore0()`(:369)
  - 공통 꼬리: MCC false 생략(:524), `AgingCycleEnd()`(:527), 진행 저장(:535-536, 튜플이면 `TupleStickDone`, 아니면 `nSeg == 0`일 때 `UpdateStickProgress`)
- 지역 변수 이름 `bAgingStickSkip`은 바꾸지 않는다. Phase 5 앵커를 안정되게 두기 위해서다. 의미가 넓어진 것은 새 주석 한 줄로 밝힌다.
- 3c 블록 주석(:358-359, ASCII)은 그대로 둔다. 그 주석 자체는 틀리지 않았다.

### 2-3b `DoWorkGlassMeasure.cpp` 마지막 스틱 skip 분기: 글라스 끝 VISION 결과 대기 (리뷰 #1)

앵커(3줄 연속, n=1, :407-409. 앞의 `if (...)`는 2곳(:407, :458)에 있으므로 반드시 3줄 전체로 찾는다):
```
if (bAgingStickSkip && GetCurStep() == nInitStep && GetStopUser() != true)\r\n
\t\t\t\t\t\t\t\t{\r\n
\t\t\t\t\t\t\t\t\tGoToExitStep();\r\n
```
3번째 줄(`\t\t\t\t\t\t\t\t\tGoToExitStep();\r\n`) **앞**, 즉 `pos + length(1~2번째 줄)`에 넣는다. 들여쓰기는 탭 9개.
```cpp
									// LYS 2026-10-08 Sampling: the glass ends on skipped stick(s). The VISION step waits for all results only at the last stick's
									//   last group; OCP waits once per glass after the stick loop (JOB_Proc CmQualityEmptyTimeOut(LAST_ITEM)). Empty buffers return at once.
									if (bSamplingSkip)
									{
										S_RecipeInfo* pVisInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
										int nVisTimeOut = (pVisInfo != nullptr) ? pVisInfo->m_sInspectItem.m_nTimeOut[4] : 30;	// seconds, the VISION step's value (DEFINE_INSPECITON_WAIT_TIME is ms)
										if (m_pShareItem->GetCommTcpIpFactoryMgr()->CmQualityEmptyTimeOut(LAST_ITEM, nVisTimeOut))
											m_pLogFileAccessor->KMLOG(WARN, _T("Sampling: glass ends on a skipped stick - VISION result wait timed out, the last VISION results may be missing"));
										if (nSsSkipRun == nSsStickRun)	// D-SS-5: every stick run by this measure was skipped
										{
											CString strAllSkip;
											strAllSkip.Format(_T("Sampling: no stick of this glass has a selected cell for the enabled items - the glass is not contacted or measured (sticks run %d)"), nSsStickRun);
											m_pLogFileAccessor->KMLOG(WARN, strAllSkip);
										}
									}
```
- **왜 여기인가.**
  - `CDoWorkGlassMeasureStep_None::Run()`이 `SetFail(false)`를 하므로(DoWorkGlassMeasureStep.cpp :571) 그 뒤여야 한다. 이 분기는 None이 끝난 직후다.
  - 루프 머리의 Phase 5 `TupleStickDone`(:368)보다 앞이라, 저널 커밋에 대기 중 들어온 VISION 줄이 포함된다.
  - 이 시점의 활성 세그먼트는 마지막 세그먼트다(`SegmentRestore0()`은 다음 반복의 루프 머리 :369). 그래서 `m_nTimeOut[4]`는 활성(마지막) 세그먼트 값이다.
- **include.** `DoWorkGlassMeasure.cpp`는 이미 `..\Manager\CommTcpIpFactoryMgr.h`(:17)를 포함하고, 그 헤더가 `CommTcpIpDefine.h`(:5, `LAST_ITEM` = TRUE)를 포함한다. `CmQualityEmptyTimeOut`은 public(CommTcpIpFactoryMgr.h :99). `S_RecipeInfo`는 이미 이 파일에서 쓴다(:763/920).
- **타임아웃 단위.** `CmQualityEmptyTimeOut`은 초 단위(`nTimeOut * 1000`, CommTcpIpFactoryMgr.cpp :2154)다. `DEFINE_INSPECITON_WAIT_TIME`은 30000(ms 의미, CommTcpIpDefine.h :6)이라 대체값으로 쓰면 30000초가 된다. 그래서 대체값은 리터럴 30(초)로 둔다. VISION 스텝은 레시피 포인터가 null일 때 이 define을 그대로 넘기는 기존 결함이 있다(범위 밖).
- **Sampling OFF / 꼬리 skip 아님.** `bSamplingSkip`이 거짓이면 블록 전체를 건너뛴다. 버퍼가 비어 있으면 `DoWorkVisionCmd(EXE_QUALITY_EMPTY_BUFFER_ALL)`이 바로 성공해 즉시, 로그 없이 돌아온다. VISION이 꺼진 레시피도 같다.
- **OOS로는 보이지 않는다.** `QualityBufferEmptyAll()`이 `m_bOutOfService`면 TRUE를 돌려준다(CommTcpIpObjectVision.cpp :1601). 코드 리뷰와 실장비 확인(R1, (8))으로 검증한다.
- **STOP.** `CmQualityEmptyTimeOut`에는 STOP 검사가 없다(:2184-2189 주석). 대기 중 STOP을 눌러도 최대 타임아웃까지 기다린 뒤 Idle이 완료로 처리한다. VISION 스텝의 같은 대기와 같은 동작이다.
- **타임아웃 처리**는 D-SS-6(사용자 확인). 권장안은 위 코드처럼 WARN만 남기고 계속 진행하는 것이다. VISION 스텝의 `LAST_ITEM` 경로도 `CmQualityAlarm()` 반환값을 버리고 `bRet = false`만 하며 `SetFail`을 부르지 않는다(Step.cpp :3999-4002). 그래서 실제로는 "Thread NG" WARN 뒤 Idle이 완료로 처리하고 글라스가 Result로 간다.
- **3c 마지막 스틱 skip과 EFA T0 flush에도 같은 틈이 있다.** 3c를 바이트 단위로 유지하려고 여기서는 고치지 않고 별건으로 남긴다((10) 후속).

### 2-3c `DoWorkGlassMeasure.cpp` 카운터 선언 (D-SS-5 권장안을 고를 때만)

앵커(n=1, :269, ASCII): `\tif (m_pMeasSeqItemEngine != NULL) m_pMeasSeqItemEngine->AgingGlassLatch(bEfaMode ? nPassCount : 0);\r\n`, 그 **뒤**에 넣는다(탭 1개).
```cpp
	int nSsStickRun = 0, nSsSkipRun = 0;	// LYS 2026-10-08 Sampling: sticks entered / Sampling-skipped by this measure (all skipped -> one WARN at the glass end)
```
- Phase 5가 고친 `bool bSkipPassBoundary = ...`(:215) 줄은 앵커로 쓰지 않는다.
- 재개 런은 이번 런에서 들어간 스틱만 센다. 그래서 문구 끝에 `(sticks run %d)`를 붙였다. 재개 런에서 남은 스틱이 모두 skip이면 이 WARN도 나간다(남은 구간 기준).

### 2-4 AgingTest / EFA / 하이브리드 경로와의 관계 (중복과 충돌 없음)

| 글라스 종류 | `AgingCycleBegin` 결과 | 3c skip | Sampling 판정 |
|---|---|---|---|
| 표준(AgingTest OFF 또는 래치 0, EFA 아님) | false, 로그 없음 | 없음 | **호출됨** → 규칙 적용 |
| AgingTest ON, 셀 있는 USE_AGING 항목 존재 | true(사이클) | 없음 | `bAgingCycle == false`가 거짓이라 **호출 안 함** |
| AgingTest ON, USE_AGING 항목은 있으나 이 스틱에 셀 없음 | false + `m_bAgingStickSkip` | `AgingCycle: stick n skipped ...` | `bAgingStickSkip == false`가 거짓이라 **호출 안 함**(로그 중복 없음) |
| AgingTest ON, USE_AGING 항목 0개 또는 aging rcp 없음 | false (`AgingCycle: begin ... active(0)` / WARN) | 없음 | 호출됨 → 표준 스텝이 레시피 플래그를 읽으므로 규칙 적용이 맞다 |
| EFA pass, 항목에 셀 있음 | true | 없음 | 호출 안 함 |
| EFA pass, 항목에 셀 없음 | false + skip | `EFA: pass p/P stick n skipped ...` | 호출 안 함 |
| 하이브리드 N>1 | 위와 같음(AgingTest는 세그먼트 0 래치) | 위와 같음 | 호출 시점에 `SegmentBoundary`가 이미 끝났다 → 활성 세그먼트 기준. 로그 끝에 ` seg(k/N)` |

결과적으로 스틱 하나에 skip 로그는 많아야 1줄(3c 또는 Sampling)이다. 로컬 플래그는 스틱 반복마다 새로 선언되므로 다음 스틱으로 넘어가지 않는다.

위 표는 D-SS-3 (a)(사이클 미적용) 기준이다. (a)에서는 "AgingTest ON, 셀 있는 USE_AGING 항목 존재"와 "EFA pass, 항목에 셀 있음" 행의 스틱이 Sampling 선택 셀 0개여도 접촉한다. 이것이 요청을 충족하지 못하는 부분이다(리뷰 #2). (b1s)/(b1)을 고르면 이 두 행이 바뀐다((7) D-SS-3).

**복사본 레시피와 사이클의 연결(리뷰 #3).** `CAgingCycleInfo::Load`는 `{AGING_RCP 또는 EFA_RCP}\{레시피 이름}.rcp`를 이름으로 찾는다(AgingCycleInfo.cpp :49, 기본 경로 `D:\HBS\PARAM_OCP\PARAM\AgingRcp\` / `...\EFARcp\`, 레지스트리 `DIR\AGING_RCP` / `DIR\EFA_RCP`). 그래서 레시피를 다른 이름으로 복사하면 사이클 연결이 끊긴다.
- EFA=1 레시피의 복사본: `GetEfaPassCount()`가 `EFA: mode requested but efa rcp missing/empty - run standard sequence` WARN을 내고 0을 돌려준다(MeasureSequenceItemEngine.cpp :911). 글라스는 **표준 글라스**가 되어 Sampling skip이 적용된다.
- Aging_Test=1 레시피의 복사본: `AgingCycle: aging rcp not found - run standard sequence`(:855). 역시 표준이다.
- 이것은 (2-4) 표 4번째 행("aging rcp 없음 → 규칙 적용")과 같은 경우다. 동작은 맞지만 OOS O8의 기대값을 바꾼다((8)).

### 2-5 대안 (권장하지 않음, 기록만)

- **엔진 전용 래퍼.** 기존 `AgingCycleBegin` 본문을 `AgingCycleBeginCore`로 이름을 바꾸고, 래퍼 안에서 `m_bAgingStickSkip`을 세우는 방식이다(`ss_map_rule.md` 6.1). ThreadRun을 고칠 필요가 없어 Phase 5와 무관하게 적용할 수 있다.
  - 단점: 이름 변경 앵커(:792) 줄에 CP949 주석이 있다. Sampling 규칙이 "aging cycle" API 안에 숨는다. 3c 플래그의 의미가 섞인다.
  - 사용자 결정이 "Phase 5 이후 적용"이므로 2-3 훅이 더 낫다. Phase 5가 길어지는데 먼저 넣고 싶을 때만 고려한다.
- **Contact부터 skip(Option B), 스텝별 게이트(Option C).**
  - 이동과 패턴 서치에 시간이 든다. 측정하지 않을 스틱에서 패턴 NG 알람이 날 수 있다.
  - 수정 지점이 Phase 5가 고치는 `DoWorkGlassMeasureStep.cpp`의 4~6개 게이트에 흩어진다.
  - OCP 동작과 다르다.
  - 그래서 채택하지 않는다(D-SS-1).

---

## (3) skip된 스틱의 부수효과 처리 (3c 경로와 같다. 새로 고칠 곳은 글라스 끝 VISION 대기 하나 - 리뷰 #1)

**수행하는 것**
- `GoToInitStep()` + `CDoWorkGlassMeasureStep_None::Run()`:
  - Complete/AlarmOn/Fail/StopUser를 false로 초기화
  - `SPS_PROCESS`
  - `SetMeasureArea(nArea)`
  - PLC `PLC_BIT_W_GLASS_SENSOR_POWER_ON`
  - SYST `CDoWorkGlassMeasureStep_None Start.`

  3c 실측 기준으로 스틱당 약 25 ms다(10-01 13:24:32 로그).
- N>1이면 `[HYBRID] seg k/N [name] stick i/n (glass stick g/G)` 헤더 줄(3c와 같음).
- **진행 저장:**
  - 레거시는 `if (nSeg == 0) pResume->UpdateStickProgress((int)i);`이고, skip 스틱도 완료로 센다. OCP도 `m_pSystem->nLastStepT = iTop`을 `continue`보다 먼저 한다.
  - Phase 5 튜플은 Phase 5 (3-2)에 따라 다음 순서로 처리된다.
    - 루프 머리의 `SetStick(nPass, nSeg, i, gs)`
    - 비마지막 스틱은 `break` 뒤 진행 블록의 `TupleStickDone(nPass, nSeg, i)`
    - 마지막 스틱은 `GoToExitStep(); continue;` → 루프 머리 `if (GetCurStep() == GetExitStepIndex())`의 `TupleStickDone` + `bTupleLastSaved`

    저널 커밋은 0줄이고 `nCellStatus` 스냅샷은 바뀌지 않으므로 정합이 유지된다.
- **마지막 스틱:** (Sampling skip이면 먼저 2-3b의 VISION 결과 대기) → `SegmentRestore0()` → Idle 1회. Idle은 SetComplete, 센서 OFF, MCC `MCC_A_PROC_MEAS`, 완료 이벤트, SPS_IDLE, OPER `CDoWorkGlassMeasureStep_Idle`을 처리한다. 이어서 INSP `CDoWorkGlassMeasure Thread End` → Result → Unload.

**VISION 결과 비동기 수신과 글라스 끝 대기 (리뷰 #1, 새로 고칠 곳)**
- VISION 결과는 비동기로 온다. 그랩 때 `QualityWaitSet`(CommTcpIpObjectVision.cpp :1611)이 슬롯을 잡고, `QualityWaitThread`가 나중에 `CmFileFindTimeOut(..., 150000)`(:1546) 뒤 `TossResultFile`(:1553)로 결과를 올린다.
- "모든 결과가 들어올 때까지" 기다리는 호출은 VISION 스텝 안의 하나뿐이다: 마지막 스틱의 마지막 그룹에서 `if (iStick + 1 == ...GetStickCount()) { if (...CmQualityEmptyTimeOut(LAST_ITEM, nTimeOut)) {`(Step.cpp :3998-4003). 그 밖의 그룹은 `NOT_LAST_ITEM`(두 버퍼 중 하나만 비면 통과, :3859)이다.
- 글라스 끝 대기는 바로 뒤에 주석 처리되어 있다(`////모든 검사 완료 후 대기, 전수 검사 아닐시 처리 방안 확인 필요`, :4031-4037). 원작자가 이미 Sampling(전수 검사 아님)을 처리하지 못한다고 적어 두었다.
- `CDlgResult::SaveResult`는 `UploadDataDone`(공유 메모리 버퍼가 잠깐 빈 상태)만 최대 5 s 기다린다(CDlgResult.cpp :1747-1765, 플래그는 :337/349). 대기 중인 VISION 처리는 기다리지 않는다.
- 지금은 VISION 그룹이 없는 마지막 스틱도 이동/접촉/OST/AGING에 몇 초를 쓰므로 그 시간이 틈을 메운다. skip하면 직전 측정 스틱의 VISION 그랩이 끝난 뒤 skip 스틱당 약 25 ms만에 Idle이 돌고, 직전 스틱의 VISION 결과(최대 2건)가 저장된 결과에서 빠질 수 있다.
- OCP는 스틱 루프 뒤와 STOP 경로에서 글라스 레벨로 기다린다: `if (bUse[3] || nMaxeQuality > 0) { CmQualityEmptyTimeOut(LAST_ITEM); ... ExportAsRawDateTmp(); }`(DoWorkStartMeasOCP(:5690~) 안, 스크래치패드 `jobproc_utf8.cpp` :12302-12303 정상 끝 / :12325-12326 `GOTO_USER_STOP`). 그래서 OCP의 `continue` skip은 안전하다. HMI의 스텝 내부 대기는 이식 과정의 산물이고, 이번 변경이 그것을 드러낸다.
- 처리: 2-3b. Sampling skip으로 글라스가 끝날 때만 대기한다.

**수행하지 않는 것**
- **모션 일체:** 스테이지 R/Y, Z5 접촉/해제, 리뷰 헤드, AGING 리뷰 이동. P5 UnContact 센서 인터락 검사도 하지 않는다.
- **PG / 장비 명령:** `PG_TEST_2`(OST), `PG_TEST_4` + AgingPG 시작/완료, AgingAlarmCheck, WAD `PG_TP_1`, 광학 장비 검사 명령, AGING_PTN PGEvent.
- **MCC:** `MCC_A_STEP_MEAS_N` true/false, 스텝 MCC(CONTACT_*, PROC_MEAS_OST/R_TEST, MEAS_N_PROC_*).
- **tact:** init/Idle 스텝은 이름이 비어 있어 `SetSeqItemTimeST/ED`가 즉시 반환한다. 그래서 `TACT TIME\tact_time_*.log`에 `StickN_*` / `SegK_StickN_*` 항목이 없다. 3c 실측에서도 `Stick2_VISION - Finish` 바로 다음이 `Stick5_MEAS MOVE - Start`였다.
- **`m_nAgingStartStick[4]`:** 쓰지 않는다. 이전 값이 남지만, PG 파싱은 PG 테스트 명령 뒤에만 하고 다음 측정 스틱이 먼저 다시 쓴다.
- **`nCellStatus`:** 쓰지 않는다. 글라스 시작값(IDLE) 또는 Phase 5 스냅샷 값이 유지된다.
- **이미지:** `SaveCurImage(2/3/4+/8)`을 남기지 않는다.

**화면:**
- 운전 화면 셀 맵에서 skip 스틱의 셀은 **IDLE 색**으로 남는다. 지금은 미선택 셀도 OST/AGING 색으로 칠해진다. 이것이 눈에 보이는 유일한 차이다.
- `WM_MSG_UPDATE_POSITION_STATE`가 나가지 않는다. 상태 표시는 다음 측정 스틱의 MOVE까지 직전 상태로 남는다.
- 결과 행과 CIM: 결과 경로는 `nCellStatus`를 읽지 않는다(grep 결과 ResultMgr/dResult 사용처 없음). 광학 결과가 없는 것은 지금(미선택 셀)과 같다. `RESULT\AGING\{recipe}`의 에이징 결과 파일에는 skip 스틱 항목이 없다(OCP와 같음).

**VISION 장비 레시피 대기(기존 문제, 이번 변경과 무관):**
- 대기는 `if ((iStick == 0 || bResumeVisWait) && i == 0 && ...TakeVisionRcpWait())`(Step.cpp :3913, Phase 5 P17 적용 뒤)에서만 걸린다.
- 스틱 0에 VISION 그룹이 없으면 지금도 대기하지 않는다. skip 스틱 0은 정의상 VISION 그룹이 없으므로 동작이 같다.
- "글라스/세그먼트의 첫 VISION 그룹에서 대기"로 고치는 일은 별건으로 남긴다.

---

## (4) 로그 문구

| 로그 | 파일 | 레벨 | 조건 |
|---|---|---|---|
| `Sampling: stick %d skipped - no selected cell for the enabled items (%s) - no move/contact/OST/R-TEST/AGING%s` | `D:\HBS\Logs\LogFile\measure_seq_item_engine_YYYYMMDD.log` | OPER | skip할 때만. `%d` = 세그먼트 로컬 스틱 + 1, `(%s)` = 켜진 모드의 `WAD,VISION,ANGLE` 부분집합, 끝 `%s` = `GetSegLogTag()`(1세그먼트면 "", 하이브리드면 ` seg(k/N)`) |

- 예: `Sampling: stick 3 skipped - no selected cell for the enabled items (WAD,VISION,ANGLE) - no move/contact/OST/R-TEST/AGING`
- 예(하이브리드): `... stick 2 skipped - ... (WAD,VISION,ANGLE) - no move/contact/OST/R-TEST/AGING seg(2/2)`
- 3c 줄(`AgingCycle: stick %d skipped - no cell for the active items in [%s]%s`, `EFA: pass %d/%d stick %d skipped ...`)과 같은 파일, 같은 레벨, 같은 번호 체계다. grep 키는 `Sampling: stick`.
- `GLASS 6_MEASURE` 로그에는 스틱마다 새 줄이 없다. skip 스틱은 `CDoWorkGlassMeasureStep_None Start.`만 남는다.
- 글라스가 Sampling skip 스틱으로 끝날 때만, `CDoWorkGlassMeasure`의 로거(`[HYBRID]` 줄과 같은 측정 로그)에 아래 WARN이 남을 수 있다(2-3b).

| 로그 | 레벨 | 조건 |
|---|---|---|
| `Sampling: glass ends on a skipped stick - VISION result wait timed out, the last VISION results may be missing` | WARN | 꼬리 skip + VISION 버퍼가 `m_nTimeOut[4]`초 안에 비지 않음 (D-SS-6) |
| `Sampling: no stick of this glass has a selected cell for the enabled items - the glass is not contacted or measured (sticks run %d)` | WARN | 이번 측정에서 들어간 스틱이 전부 Sampling skip. **D-SS-5 권장안**(리뷰 #4). 종전 계획의 문구를 그대로 쓰고 재개 런 구분용으로 `(sticks run %d)`만 붙였다. 종전 계획은 엔진 로그 + 글라스 시작 사전 스캔이었으나, 사이클/EFA/재개 판정을 사전 스캔으로 정확히 재현하기 어려워 ThreadRun 카운터 + 측정 로그로 바꿨다 |

---

## (5) N=1 / Sampling OFF 불변 근거

1. **Sampling OFF.** 훅이 판정을 부르지만 판정은 `IsOCP()`, 사이클/3c, Recipe 포인터, `m_bUseSampling == FALSE` 순으로 검사해 false를 낸다. 상태를 쓰지도 로그를 남기지도 않는다. 훅은 false를 OR할 뿐이라 `bAgingStickSkip`이 그대로다. 이후 MCC, tact, 스텝, 진행 저장, Phase 5 훅이 바뀌지 않고 로그도 바이트 단위로 같다. 추가 비용은 스틱당 함수 호출 1회와 포인터 읽기 몇 번이다.
2. **Sampling ON + 모든 스틱에 선택 셀.** 셀 루프에서 첫 선택 셀을 만나 false를 내고(로그 없음) 1번과 같아진다. 스텝 안의 Sampling 셀 필터는 원래 코드 그대로다.
3. **전기 전용 레시피.** E가 비어 false를 낸다. 접촉은 기존 `NeedOcpElectContact`가 맡는다.
4. **AgingTest/EFA 글라스.** `bAgingCycle`이 true이거나 3c skip이면 `&&` 단락 평가로 판정이 **호출되지도 않는다**. 3c 로그와 동작이 같다.
5. **N=1(비하이브리드).** `GetRecipeInfo()/GetRecipeData()`가 단일 레시피를 돌려주고 `GetSegLogTag()`는 ""다. 루프 구조와 `[HYBRID]` 줄(N>1 전용)에는 변화가 없다.
6. **비OCP.** 첫 줄에서 false.
7. **글라스 끝 VISION 대기와 카운터(리뷰 #1/#4).** 대기와 두 WARN은 `if (bSamplingSkip)` 안에만 있다. Sampling OFF, 모든 스틱에 선택 셀, 전기 전용, 사이클/EFA, 3c 마지막 스틱 skip에서는 `bSamplingSkip`이 거짓이라 실행되지 않는다. 카운터 증가(`nSsStickRun++`)는 매 스틱 돌지만 상태·로그·타이밍에 영향이 없다.
8. **빌드 측면.**
   - 바뀌는 것: 헤더 public 메서드 1개 추가, cpp 함수 1개 추가, ThreadRun 카운터 선언 1줄 + 훅 3문장 + 마지막 스틱 skip 분기 블록 1개.
   - 바뀌지 않는 것: 멤버, 구조체, `preDefine.h`, 레시피 파일 형식, 공유 메모리. DLL ABI나 레시피 버전 게이트와 관계가 없다.

---

## (6) 접점 표

| 파일 | 인코딩 / EOL | 앵커 (exact ASCII, n) | 현재 줄 | 동작 | Phase 5 |
|---|---|---|---|---|---|
| `HMI/Source/ALPHA/DoWork/MeasureSequenceItemEngine.h` | CP949(한글 주석 있음), CRLF 136/136, BOM 없음 | `IsAgingStickSkip() const\t{ return m_bAgingStickSkip; }\t// LYS 2026-09-30 AgingCycleBegin(nStick >= 0) found no USE_AGING item with a cell on the stick\r\n` (1) | 103 | 뒤에 선언 1줄 | 건드리지 않음 |
| `HMI/Source/ALPHA/DoWork/MeasureSequenceItemEngine.cpp` | CP949, CRLF 1090/1090, BOM 없음 | `void CMeasureSequenceItemEngine::AgingCycleEnd()\r\n` (1) | 1072 | 앞에 `IsSamplingStickSkip` 정의(주석 6줄 + 본문, 빈 줄로 끝) | 건드리지 않음 |
| `HMI/Source/ALPHA/DoWork/DoWorkGlassMeasure.cpp` | CP949, CRLF 1124/1124, BOM 없음, damaged 0 (10-08 10:06:20) | (a) `\tif (m_pMeasSeqItemEngine != NULL) m_pMeasSeqItemEngine->AgingGlassLatch(bEfaMode ? nPassCount : 0);\r\n` (1) | 269 | 뒤에 카운터 선언 1줄 (2-3c, D-SS-5 권장안일 때만) | P15 적용 완료. 이 줄은 P15 앵커 아님 |
| 〃 | 〃 | (b) `\t\t\t\tbool bAgingStickSkip = (m_pMeasSeqItemEngine != NULL && m_pMeasSeqItemEngine->IsAgingStickSkip());\r\n` + `\t\t\t\tif (bAgingStickSkip == false) CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_STEP_MEAS_N, true, 0);\r\n` (연속 2줄 1) | 360-361 | 사이에 주석 1줄 + 문장 3줄 (2-3) | P15 적용 뒤 재확인 n=1 |
| 〃 | 〃 | (c) `if (bAgingStickSkip && GetCurStep() == nInitStep && GetStopUser() != true)\r\n\t\t\t\t\t\t\t\t{\r\n\t\t\t\t\t\t\t\t\tGoToExitStep();\r\n` (연속 3줄 1. 첫 줄만으로는 2: :407/:458) | 407-409 | 3번째 줄 앞에 블록 (2-3b) | P15 적용 뒤 재확인 n=1 |
| (변경 없음) `DoWorkGlassMeasureStep.cpp/.h`, `DoWorkGlassMeasure.h`, `RecipeEditMgr.*`, `ResumeInfoMgr.*`(LF), dfcRecipe/`preDefine.h` 2벌, `.rc`, `SystemConfig.ini`, 레시피 | - | - | - | - | - |

- 패치 규칙(CLAUDE.md):
  - Perl 스크립트 파일로 작성하고 인라인 Bash로 돌리지 않는다.
  - `<:raw`로 읽고, 앵커 `index()` 두 번으로 유일성을 확인한다.
  - payload는 ASCII만 쓰고, 줄바꿈을 `\r\n`으로 바꿔 넣는다.
  - 멱등 가드: `IsSamplingStickSkip` 문자열이 이미 있으면 die.
  - 사후 확인:
    - `damaged=0` (`\xef\xbf\xbd`가 0개)
    - 파일별 crlf == lf
    - 새 문자열 개수: `IsSamplingStickSkip` .h 1, .cpp 정의 1, Measure.cpp 호출 1. Measure.cpp의 `bSamplingSkip` 3개(훅 선언 1, 훅 if 1, 2-3b if 1. D-SS-3 (a) 기준), `LAST_ITEM` 2개(2-3b 주석 1 + 호출 1), `nSsStickRun` 4개 / `nSsSkipRun` 3개(D-SS-5 권장안 기준), `CmQualityEmptyTimeOut` 2개(주석 1 + 호출 1)
    - 앵커 n=1 유지 (Measure.cpp는 (a)(b)(c) 세 개)
  - 백업은 스크래치패드 `ss_bak/`에 둔다.
  - Edit/Write 도구는 쓰지 않는다.

---

## (7) 결정 사항 (사용자 확인 필요분만, 권장안 굵게)

- **D-SS-1 skip 범위.** **스틱 전체(이동, 패턴 서치, Contact, OST, R-TEST, AGING, 측정 모두 생략. OCP `nGroup < 1 → continue`와 같음)** / Contact부터만 생략(이동과 패턴 서치는 수행).
  - `ss_decision.md`에 "스틱 전체"로 기록되어 있으니 재확인만 하면 된다.
  - Contact부터만 생략하면 측정하지 않을 스틱에서도 패턴 NG 알람과 이동 시간이 남는다.
- **D-SS-2 적용 장비.** **OCP만(`IsOCP()`)** / 모든 HMI 장비. `NeedOcpElectContact`, `AgingCycleBegin`과 같은 게이트다. 비OCP는 P1~P4 구조라 따로 검토해야 한다.
- **D-SS-3 AgingTest 사이클 / EFA 글라스 (리뷰 #2로 다시 씀).** 아래 넷 중 하나. **권장: (b1s)**.
  - **(a) 미적용.** 3c 마스크 규칙만 유지. **이 안은 AgingTest/EFA 글라스에서 요청을 충족하지 못한다.** 사이클 중에도 스텝은 마스크와 Sampling을 함께 적용하므로(WAD :2291, VISION :3265-3268, ANGLE :4806 부근의 `pbUse` 루프), Sampling ON 사이클 글라스에서 선택 셀이 없는 스틱이 USE_AGING 항목마다 이동/접촉/OST/AGING을 하고 아무것도 재지 않는다(`NeedOcpElectContact`가 사이클/EFA면 true, Step.cpp :53). 종전 계획의 "OCP_A와 같다"는 근거도 맞지 않는다. OCP `DoWorkStartMeasOCP_A`는 Sampling을 주석 처리하고(`// 20240502 kks //if( bUseSampling )`, jobproc_utf8.cpp :12768 등) 항상 마스크(`GetMeasUse`)만 보므로, OCP에서는 마스크에 든 스틱은 **항상 측정된다**. 0셀 접촉은 HMI 스텝이 사이클 중에도 Sampling을 적용해서 생기는 HMI 고유 문제다.
  - **(b1s) 스틱 단위만 (권장).** 사이클/EFA 스틱에서, 활성 항목(EFA는 그 pass 항목, AgingTest는 USE_AGING 항목 전체) 중 어느 것도 "마스크 사용 ∧ Sampling 선택"인 셀을 갖지 않으면 그 스틱만 3c처럼 skip한다. 항목 순서·반복 인덱스는 건드리지 않는다(통째로 skip된 스틱은 결과 행을 쓰지 않고, 반복 인덱스는 스틱마다 리셋된다). 요청("해당 Stick 전부 미측정 시")과 범위가 정확히 같다.
    - 위치: `AgingCycleBegin` 안, 시작 로그 바로 앞 두 곳. EFA 앵커 `\t\tCString strEfaLog;\r\n\t\tstrEfaLog.Format(_T("EFA: begin pass %d/%d%s")`(n=1, :836), AgingTest 앵커 `\tCString strLog;\r\n\tstrLog.Format(_T("AgingCycle: begin, items(%d) active(%d)%s")`(n=1, :887, `m_bAgingCycleActive`일 때만). 사이클을 끄고(`m_bAgingCycleActive = m_bEfaCycle = false`, `m_nAgingCurItem = -1`) `m_bAgingStickSkip = true` + 새 멤버 `m_bSamplingCycleSkip = true`를 세운 뒤 `Sampling: stick %d skipped - no selected cell for the cycle items in [%s] - no move/contact/OST/R-TEST/AGING%s` OPER 1줄을 남기고 false를 돌려준다. begin/end 줄은 남지 않는다(3c와 같은 모양).
    - 셀 판정은 스텝의 `pbUse`와 같아야 한다: 셀 c, 항목 종류 k(Property→M_SPEC, Quality→M_QUALITY, Angle→M_ANGLE)에 대해 `마스크 사용(c,k) && m_nSelectCell[M_k] == 1`. 마스크 사용은 `IsAgingCellSkip`(Step.cpp :501-515) 규칙 그대로(세그먼트 마스크 행 없음 → 사용, `iCell >= GetCellCount` → 사용, 아니면 `IsCellUse`). **스틱 단위 `IsStickUse`와 Sampling을 따로 보면 안 된다**: 마스크 셀 {1,2}와 선택 셀 {3,4}처럼 겹치지 않으면 둘 다 "있음"인데 실제로는 아무것도 재지 않는다.
    - 훅: `bool bSamplingSkip = (bAgingStickSkip && engine->IsSamplingCycleSkip()) || (기존 표준 조건)`. 이렇게 해야 2-3b 꼬리 대기가 사이클 Sampling skip에도 걸린다. `m_bSamplingCycleSkip`은 `AgingCycleBegin` 첫머리에서 false로 리셋한다(.h에 private 멤버 1개 + 인라인 getter 1개, ALPHA 내부 클래스라 ABI 영향 없음).
    - EFA T0 pass의 마지막 스틱이 이 경로로 skip되면 `EfaPassBoundary`의 T0 flush(`SaveResult`)가 VISION을 기다리지 않는 틈이 생긴다. 3c와 같은 기존 틈이다((10) 후속 F1). (b1s)를 고르면 이 틈이 Sampling으로도 열린다는 점을 함께 확인받는다.
    - Sampling OFF면 판정을 부르기 전에 false라 3c/EFA 로그와 동작이 그대로다. 그래도 3c/EFA OOS(Sampling OFF 회귀 + Sampling ON 새 케이스)를 다시 한다.
  - **(b1) 항목 + 스틱.** `AgingItemHasCell`(MeasureSequenceItemEngine.cpp :1025)에 종류별 Sampling 항을 넣어 항목 skip과 스틱 skip이 함께 따라가게 한다. Sampling OFF는 그대로.
    - 판정은 (b1s)와 같은 셀 단위 교집합이어야 한다(리뷰 예시의 "스틱에 `m_nSelectCell == 1` 셀이 있는가"를 마스크와 따로 보면 위의 겹치지 않는 경우를 놓친다).
    - 주의: WAD/VISION/ANGLE 세 스텝 모두 `bUseMeas`(항목 플래그 ∧ 마스크)가 참이면 그룹이 0개여도 `AgingRepeatNext`를 부른다(Step.cpp :3141-3142, :4040-4041, :5440-5441). 항목을 Sampling으로 건너뛰면 그 증가가 사라져 뒤 항목의 결과 행 블록이 당겨진다. 결과 행을 확인하고 3c/EFA OOS를 다시 해야 한다. (리뷰는 VISION만 지적했으나 세 종류 모두 같다.)
    - 3c 로그 문구 `no cell for the active items in [MASECELL]`도 "/Sampling"을 포함하도록 바꿔야 한다. 3c 바이트 동일성이 깨진다.
    - 한 스틱에서 측정할 항목이 일부라도 있으면 나머지 항목의 AGING도 생략된다. 요청 범위(스틱 전체 미측정)를 넘는 동작 변화다.
  - **(b2) OCP_A 패리티.** 사이클 중에는 스텝이 Sampling을 무시한다(OCP처럼 마스크만). 무엇을 재는지가 바뀌므로(Sampling ON 사이클 글라스에서 미선택 셀도 측정) 사용자 승인이 필요하다. 이 경우 Sampling 스틱 skip은 사이클 글라스에 해당이 없어진다(마스크 스틱은 측정되므로).
- **D-SS-4 셀 1개 스틱.** **선택 기준대로 skip** / 지금처럼 측정(1셀 스틱만 규칙에서 제외).
  - 지금은 스텝 결함 때문에 미선택 1셀이 측정된다(잔여 셀 패스가 `if (nArea > 1)` 밖에 있고 Sampling 검사가 없음 - 리뷰에서 재확인).
  - 현 레시피는 영향이 없다.
  - 스텝 결함 자체(잔여 셀 패스에 Sampling 검사 없음)를 고치는 일은 별건이다.
- **D-SS-5 글라스 전 스틱 skip (리뷰 #4로 권장안 변경).** 경고 없음(OCP와 같음) / **측정 로그 WARN 1줄(권장, 2-3b/2-3c)** / START 거부(알람).
  - 경고 없음은 권장하지 않는다. `_CELL_SELECT_`는 MakeCell이 0으로 만들고(preDefine.h :383 `ZeroMemory`) 버전 가드 없이 읽는다(InRecipeFunc.cpp :359-362 / OutRecipeFunc.cpp :358-361). Sampling만 켜고 셀을 고르지 않은 레시피는 지금은 그래도 전 스틱 OST/AGING을 하지만, 변경 뒤에는 전기·광학 데이터 없이 Result/Unload로 정상 완료된다. 알람이 없고, 저장 게이트가 레시피 플래그라(DoWorkGlassResultStep.cpp :83-112) `SaveResult`도 돈다. 품질 유출 위험이다.
  - WARN 문구는 종전 계획과 같다(영어). 재개 런 구분용으로 `(sticks run %d)`만 붙였다.
  - START 거부를 고르면 START 경로(레시피 스텝 또는 시작 팝업)에 세그먼트별 사전 스캔(`GetRecipeFuncPtr()->GetSegmentMgr(k)`, 1세그먼트면 활성 레시피)과 알람 ID가 필요하다. D-SS-3 결과(사이클 글라스 포함 여부)에 따라 판정이 달라지므로 별도 설계로 넘긴다.
- **D-SS-6 글라스 끝 VISION 대기 타임아웃 (리뷰 #1, 새 결정).** **WARN만 남기고 계속(권장)** / 글라스 실패 + 알람.
  - 권장 근거: VISION 스텝의 같은 `LAST_ITEM` 대기도 `CmQualityAlarm()` 반환값을 버리고 `bRet = false`만 하며 `SetFail`을 부르지 않는다(Step.cpp :3999-4002). 결과적으로 "Thread NG" WARN 뒤 Idle이 완료로 처리해(`GetFail() != true` → `SetComplete(true)`, Step.cpp :1980-1982) 글라스가 Result로 간다. 같은 수준으로 맞춘다.
  - 실패 + 알람을 고르면 `SetFail(true, <알람 ID>)` 후 기존 NG 경로처럼 `GoToExitStep()` + Idle, `bExis = true`(진행 저장 생략)로 바꿔야 하고 알람 ID 신설이 필요하다.
- **D-SS-7 전기 전용 레시피 (리뷰 #6, 새 결정).** **skip 안 함(권장, E3)** / OCP처럼 skip.
  - 종전 계획은 이것을 "하드 룰"로 적었지만 워크플로 스크립트의 규칙이었지 사용자 지시가 아니다. OCP는 광학/LCR 그룹이 없으면 Sampling과 상관없이 전 스틱을 `nGroup < 1 → continue`로 건너뛴다(jobproc_utf8.cpp :6210-6212).
  - 권장 근거: 요청 문구 "WAD,Vision,Angle Mode에서"는 광학 모드가 켜진 레시피를 전제로 한다. 전기 전용 레시피에서 skip하면 OST/AGING이 전혀 돌지 않게 되어 2026-09-29 `NeedOcpElectContact` 수정(전기 항목만 있어도 이동+접촉)과 정면으로 부딪친다.
- **D-SS-8 하이브리드: 전 스틱 skip 세그먼트의 경계 (리뷰 #5, 수용 비용 확인).** **경계 유지(권장)** / 경계 생략.
  - 경계(`MoveCmd_SafetyAll` → Stage R 회전 → Glass SW Align)는 그대로 돈다. 그 세그먼트가 아무것도 재지 않아도 그 경계의 Align NG면 글라스가 중단된다(DoWorkGlassMeasure.cpp :852-854 → :313-323).
  - 생략은 권장하지 않는다. `bTurn`이 레시피 회전값에서 나오므로(:734) 데이터만 바꾸면 이후 경계의 회전/Align 판단이 어긋난다. HMI/docs 문서에도 이 비용을 적는다.

(사용자 확인이 필요 없는, 설계 판단으로 정한 것)
- VISION은 레시피 `m_bUseQuality_4`만 본다(`USE_VISION_COLOR` 항 없음, 1-1).
- 훅 위치는 ThreadRun 2-3(Phase 5 이후)이다. 글라스 끝 VISION 대기는 마지막 스틱 skip 분기(2-3b)다.
- 3c 마지막 스틱 skip의 같은 VISION 틈은 이번 변경에서 고치지 않는다(3c 바이트 동일성 유지, (10) 후속 F1).

---

## (8) OOS 검증 절차와 기대 로그

**공통 준비**
- Phase 5와 이번 패치를 Debug x64로 전체 빌드한다. OOS(OutOfService)로 기동한다. EFA START 화면 플래그(공유 `m_bNewAgingMode`)는 OFF로 둔다.
- 기존 TEST 레시피는 Phase 5 OOS에도 쓰이므로 **복사본**을 만든다. 예: `1_OCP_TEST_Long_SS_261008`, `9_OCP_TEST_Hybrid_Half_SS_261008`. 복사본 이름은 AgingRcp/EFARcp/MultiMeasRcp와 이름이 연결되지 않아서 사이클 영향도 끊긴다. **이 때문에 사이클 글라스를 시험하는 O8은 연결 파일도 같이 복사해야 한다(리뷰 #3, 아래 O8a).** O7처럼 표준 글라스로 돌릴 복사본은 연결이 끊겨도 된다(Aging_Test=1이면 `AgingCycle: aging rcp not found - run standard sequence` WARN이 한 번씩 나오는 것은 정상).
- 레시피를 바꾸는 방법:
  - UI: 레시피 편집기 Inspect Option 그리드의 Sampling(행 1, 열 1). `IDD_SELECT_CELL_DLG`에서 모드 라디오를 고르고 셀을 클릭하거나 `Select All` / `Clear All`을 누른 뒤, **`Cell Update`**와 레시피 Save.
  - 텍스트 편집(복사본만): `_OPTION_INFO_` 2번째 값이 Sampling이다. `_CELL_SELECT_`는 스틱 k(1-base)마다 4k-3 ~ 4k행이고, 순서는 SPEC, QUALITY, COLOR, ANGLE이다.
- 기준 레시피 사실(2026-10-08 확인):
  - `1_OCP_TEST_Long_260907`: 5스틱 × 20셀, Sampling 0, 선택 전부 0, Pre/Elect/AgingTest_2/Spec/Quality/Angle = 1, EFA 0, Aging_Test 0 → 표준 시퀀스.
  - `9_OCP_TEST_Hybrid_Half_260930`: 2세그먼트(LONG_H1 5×3, rot 90 / SHORT_H2 5×4, rot 180). 두 세그먼트 모두 Sampling 0, **EFA 1**(EFARcp 있음) → EFA 글라스.
- 확인할 로그 파일:
  - `D:\HBS\Logs\LogFile\measure_seq_item_engine_YYYYMMDD.log`(`Sampling: stick`)
  - `D:\HBS\Logs\LogFile\GLASS 6_MEASURE_YYYYMMDD.log`(스텝 Start 줄)
  - `D:\HBS\Logs\TACT TIME\tact_time_YYYYMMDD.log`(`Cycle Seq Item : StickN_...`)
  - `GLASS Sequence_Step_*.log`(7_RESULT / 8_UNLOAD)
  - `ResumeInfo.ini`(Config 폴더)

| # | 설정 | 기대 결과 |
|---|---|---|
| O1 | 원본 Long, Sampling 0 (회귀) | 엔진 로그 `Sampling: stick` 0줄. 스틱 5개 모두 `MoveMeasureArea/PtrnSearch/Probe_Contact/OST/RTEST/AGING/Measure_WAD/ANGLE/VISION Start.`. tact `Stick1_`~`Stick5_` 모두 있음. 적용 전 런과 줄 모양 diff 0(`n1ext2.pl` 방식 정규화) |
| O2 | Long 복사본, Sampling 1, WAD `Select All`(VISION/ANGLE은 Clear) | `Sampling:` 0줄. 모든 스틱이 이동/접촉/OST/AGING. WAD는 선택 셀만, VISION/ANGLE은 그룹이 0이라 PG/장비 명령 없음(지금 동작) |
| O3 | O2 + 스틱 3의 WAD 20셀 해제 | 엔진: `Sampling: stick 3 skipped - no selected cell for the enabled items (WAD,VISION,ANGLE) - no move/contact/OST/R-TEST/AGING` 1줄. MEASURE 로그: 스틱 2 `..._Measure_VISION End.` 뒤에 `CDoWorkGlassMeasureStep_None Start.`(스틱 3)과 `CDoWorkGlassMeasureStep_None Start.`(스틱 4)가 이어지고 바로 `..._MoveMeasureArea Start.`. 스틱 3의 Contact/OST/RTEST/AGING Start는 없음. tact에 `Stick3_` 없음(`Stick2_VISION - Finish` 다음이 `Stick4_MEAS MOVE - Start`). 스틱 4 MEAS MOVE에서 Z5 UnContact 후 이동(기존 스틱 전환과 같음). MCC `MCC_A_STEP_MEAS_N`은 4회 |
| O3a | O2 + 스틱 1만 해제 | 첫 스틱 skip. 스틱 2부터 정상. VISION 레시피 대기(스틱 0 조건)는 발생하지 않음(기존 동작, 이번 변경과 무관) |
| O3b | O2 + 스틱 5(마지막)만 해제 | 스틱 4 VISION End 뒤 `None Start.` → (2-3b VISION 대기: OOS에서는 `QualityBufferEmptyAll()`이 TRUE라 즉시 통과, 새 WARN 없음) → OPER `CDoWorkGlassMeasureStep_Idle` → INSP `CDoWorkGlassMeasure Thread End` → `GLASS 7_RESULT` Start → `8_UNLOAD`(Z5는 Unload `MoveCmd_SafetyAll`에서 올라감). Idle은 정확히 1회. 대기 자체의 효과는 OOS로 볼 수 없다(R1) |
| O4 | O3 + 스틱 3에 ANGLE 선택 1셀(ANGLE 켜짐) | skip 없음(모드 OR). 스틱 3 이동/접촉/OST/AGING, ANGLE 1셀 측정 |
| O5 | Long 복사본, Sampling 1, Spec/Quality/Angle 0(전기 전용), 선택 0 | `Sampling:` 0줄. 모든 스틱 이동, 접촉, OST, AGING(`NeedOcpElectContact`). 회귀가 없어야 함 |
| O6 | Long 복사본, Sampling 1, 광학 켜짐, 선택 전부 0 | `Sampling: stick 1..5 skipped` 5줄. MEASURE는 `None Start.` 5회 + Idle 1회 + Thread End. tact `Stick` 항목 0개. Result/Unload 정상, 알람 없음. D-SS-5 권장안이면 측정 로그에 `Sampling: no stick of this glass has a selected cell ... (sticks run 5)` WARN 1줄(Idle 바로 앞). O3/O3b에서는 이 WARN이 없어야 함 |
| O7 | Hybrid 복사본: **두 세그먼트 EFA 0**(`ValidateSegments`가 같게 강제), seg 1 Sampling 0, seg 2 Sampling 1 + WAD `Select All` 후 seg 2 스틱 2 해제 | MEASURE: `[HYBRID] seg 2/2 [SHORT_H2] stick 2/5 (glass stick 7/10)` 뒤에 `None Start.`만. 엔진: `Sampling: stick 2 skipped ... seg(2/2)` 1줄. seg 1에는 줄 없음. 세그먼트 경계(회전 180)는 그대로. tact에 `Seg2_Stick2_` 없음. 측정 스틱의 결과 행(segBase)은 적용 전과 같음 |
| O8a | **사이클 연결 복사본**(리뷰 #3): 원본 Hybrid(EFA 1)를 새 이름으로 복사하고 `{EFA_RCP}\<원본>.rcp`도 `{EFA_RCP}\<새 이름>.rcp`로 복사(기본 `D:\HBS\PARAM_OCP\PARAM\EFARcp\`, 레지스트리 `DIR\EFA_RCP`가 있으면 그 경로). AgingTest 변형(Aging_Test 1)이면 `{AGING_RCP}\<원본>.rcp`도 복사(기본 `...\PARAM\AgingRcp\`). WAD 다점측정을 쓰면 MultiMeasRcp도. 실장비(OOS 아님)라면 장비 측 AGING 대상 레시피 연결도 확인. seg 2 Sampling 1 + 선택 0 | 시작 시 `EFA: mode on (flag 0, recipe 1), passes N`(원본과 같은 N, missing WARN이 없어야 함). **D-SS-3 (a):** `Sampling:` 0줄(EFA pass = 사이클, 판정 미호출), `EFA: pass ...`/3c 줄은 적용 전과 같음. **(b1s):** seg 2 스틱마다 pass별 `Sampling: stick n skipped - no selected cell for the cycle items in [...] ... seg(2/2)` 1줄, 그 스틱의 `EFA: begin pass` 줄 없음, seg 1은 적용 전과 같음. **(b1):** 3c 줄 문구가 바뀌고 항목 skip이 생김(결과 행 대조 필요). AgingTest 변형도 같은 방식 |
| O8b | (선택) **연결 없는 복사본**: 위 복사본에서 rcp 파일은 복사하지 않음 | 시작 시 `EFA: mode requested but efa rcp missing/empty - run standard sequence` WARN(AgingTest 변형은 스틱마다 `AgingCycle: aging rcp not found - run standard sequence`). 표준 글라스로 돌고 Sampling skip이 **적용됨**: seg 2 스틱마다 `Sampling: stick n skipped ... seg(2/2)`. "rcp 없음 → 표준 대체" 경로를 일부러 확인하는 케이스다 |
| O9 | O3 설정에서 스틱 4 진행 중 STOP → 재개 | 레거시: 재개 시작이 index 3(스틱 4). `Resume: starting from stick 3 (total 5)`. 스틱 3은 다시 돌지 않음. 튜플(Phase 5 `RESUME_TUPLE_ALL=1` 또는 하이브리드): 저널에 스틱 3 줄 0개, 스냅샷 링 진행, `LastStick`에 스틱 3 포함. 재개 전에 선택을 바꾸면 지문 불일치로 거부되는지 확인(튜플만). 레거시 북마크는 지문이 없어 거부되지 않고 스틱 3을 다시 판정하지 않는다(E11, 기존 동작 - 확인만) |
| O10 | O6 실행 중(skip 연속) STOP | 정상 STOP 경로(`CDoWorkGlassMeasure Thread Stop` 또는 다음 init 전 종료). 축 이동 없음. **주의(리뷰 #8):** skip만 이어지는 런은 약 0.1~0.3 s이고 거의 전 구간이 스틱 사이 틈이라, None의 `SetStopUser(false)`(Step.cpp :572)에 STOP이 지워져 글라스가 정상 완료될 수 있다. 기존 경합(E12)이며 이것도 합격으로 본다. STOP 확인은 O3처럼 측정 스틱이 섞인 런에서 한다 |
| R1 | **실장비(OOS 불가, 리뷰 #1):** Long 복사본, Sampling 1, VISION 켜짐, 스틱 1~4는 VISION 선택, 스틱 5(마지막)는 선택 0 | 측정 로그: 스틱 4 `Measure_VISION End` 뒤 스틱 5 `None Start.` → (VISION 대기, 결과 파일 수신 시간만큼) → Idle. 결과 파일(APD/CSV)에 스틱 4의 VISION 값이 모두 있음(적용 전 바이너리로 같은 설정을 돌리면 빠지는지 대조). 타임아웃 WARN 없음. 코드 리뷰: 대기 블록이 None 뒤, 루프 머리 `TupleStickDone` 앞인지 |

---

## (9) Phase 5 워크플로 완료 후 적용 순서

1. **대기 조건.** Phase 5 5-4(P14~P20 시퀀스, `DoWorkGlassMeasure.cpp` 포함) 적용, 5-5 전체 빌드, OOS (a)가 끝날 때까지 기다린다(`ss_decision.md`: "AFTER the Phase 5 implementation workflow finishes"). 10-08 10:2x 현재 P15는 이미 들어갔다((10) 참조). 엔진 두 파일은 Phase 5와 겹치지 않으므로 기술적으로는 먼저 넣어도 된다. 다만 결정에 따라 함께 뒤로 미룬다. D-SS-3/5/6/7/8 답을 먼저 받는다(특히 D-SS-3이 (b1s)/(b1)이면 엔진 패치 범위가 커진다).
2. **재확인(읽기 전용).**
   - `DoWorkGlassMeasure.cpp` ThreadRun을 다시 읽는다.
     - (a) 2-3 앵커 2줄이 연속, n=1로 남아 있는지 (10-08 10:2x: :360-361)
     - (b) 3c 가드 두 곳(`if (bAgingStickSkip && GetCurStep() == nInitStep && GetStopUser() != true)`, 마지막 스틱 `GoToExitStep(); continue;` / 비마지막 `break;`)이 Phase 5 뒤에도 같은지. 2-3b의 3줄 앵커가 n=1인지 (:407-409)
     - (b2) 2-3c 앵커(`AgingGlassLatch(bEfaMode ? nPassCount : 0);` 줄) n=1인지 (:269). `bSamplingSkip` / `nSsStickRun` 문자열이 0개인지(멱등)
     - (c) Phase 5의 `SetStick` / 루프 머리 `TupleStickDone` / 진행 블록 `TupleStickDone`이 skip 경로에서 도달 가능한지
   - 엔진 .h :103 / .cpp :1072 앵커 n=1인지. `IsSamplingStickSkip` 문자열이 0개인지(멱등).
   - 세 파일 모두 CP949, CRLF(crlf == lf), BOM 없음, damaged 0인지.
3. **백업.** 세 파일을 스크래치패드 `ss_bak/`에 복사한다. `git -C D:/OCP/0.SW_Matching/HMI status --short`와 `diff --stat` 스냅샷도 남긴다(Phase 4/4b/5 미커밋 변경과 섞이므로 파일 목록을 기록해 둔다).
4. **스테이지 A (엔진).** 2-1과 2-2를 Perl로 패치한다. ClCompile `DoWork\MeasureSequenceItemEngine.cpp` 하나만(Phase 5의 `p5_cl.ps1` 방식, IntDir은 스크래치패드).
5. **스테이지 B (훅).** 2-3b → 2-3 → 2-3c 순서(파일 뒤쪽부터)로 Perl 패치한다. 각 패치마다 앵커 위치를 새로 계산한다. ClCompile `DoWork\DoWorkGlassMeasure.cpp`.
6. **사후 확인.** (6)의 확인 목록: damaged 0, crlf == lf, 새 문자열 개수, payload ASCII.
7. **전체 빌드.** HMI Debug x64. ALPHA가 실행 중이면 **사용자 확인 뒤** `taskkill //IM ALPHA.exe //F`. 이 워크스페이스의 `2022/Community` MSBuild로 sln을 빌드하고 로그의 `vcxproj ->`를 확인한다. 필요하면 Release x64도 같은 순서로 빌드한다.
8. **OOS.** O1(회귀) → O2/O3/O3a/O3b → O4/O5/O6 → O7/O8a(/O8b) → O9/O10. O1과 O8a는 적용 전 로그와 줄 모양을 비교한다. D-SS-3이 (b1s)/(b1)이면 3c/EFA 기존 OOS도 다시 한다. 실장비 기회가 생기면 R1.
9. **문서와 기록.**
   - `HMI/docs/`(예: HybridGlass 계획서의 3c 절 옆 또는 새 짧은 문서)에 규칙, 로그, 결정을 적는다. D-SS-8 수용 비용(전 스틱 skip 세그먼트의 경계 Align NG가 글라스를 멈춤)과 (10) 후속 F1~F4도 적는다.
   - 메모리 노트를 갱신한다.
   - HMI는 git 저장소이고 두 PC가 동기화한다. 커밋하지 않은 변경은 유실될 수 있으므로 **커밋 여부를 사용자에게 묻는다**. 임의로 커밋하지 않는다.

### 부록: 패치 스크립트 골격 (나중에 `ss_patch.pl`로 작성할 때 사용. ASCII 전용, 스크립트 파일로 실행)

```perl
use strict; use warnings;
my $A  = 'D:/OCP/0.SW_Matching/HMI/Source/ALPHA/DoWork';
my $BK = 'C:/Users/C0000183/AppData/Local/Temp/claude/D--OCP-0-SW-Matching/2b7657ff-4f5d-4f68-ac4c-b3dc943d5551/scratchpad/ss_bak';
sub rd   { my $p = shift; open(my $f, '<:raw', $p) or die "$p: $!"; local $/; my $d = <$f>; close $f; $d }
sub wr   { my ($p, $d) = @_; open(my $f, '>:raw', $p) or die "$p: $!"; print $f $d; close $f }
sub upos { my ($d, $s, $t) = @_; my $p = index($d, $s); die "$t: anchor not found\n" if $p < 0;
           die "$t: anchor not unique\n" if index($d, $s, $p + 1) >= 0; $p }
sub crlf { my $t = shift; $t =~ s/\r?\n/\r\n/g; die "non-ASCII payload\n" if $t =~ /[^\x00-\x7F]/; $t }
# per file: $d = rd(); die if index($d,'IsSamplingStickSkip') >= 0; copy to $BK;
#  .h  : $p = upos($d, $ancH, 'H');  substr($d, $p + length($ancH), 0, crlf($declH));
#  .cpp: $p = upos($d, $ancC, 'C');  substr($d, $p, 0, crlf($defC));
#  .Measure.cpp (back to front, index recomputed each time; die if 'bSamplingSkip' or 'nSsStickRun' already present):
#    2-3b: $p = upos($d, $ancT1.$ancT2.$ancT3, 'T'); substr($d, $p + length($ancT1.$ancT2), 0, crlf($tailM));   # T1 = the if line, T2 = "\t"x8."{\r\n", T3 = "\t"x9."GoToExitStep();\r\n"
#    2-3 : $p = upos($d, $ancM1.$ancM2, 'M'); substr($d, $p + length($ancM1), 0, crlf($hookM));
#    2-3c: $p = upos($d, $ancL, 'L');         substr($d, $p + length($ancL), 0, crlf($cntM));                # only with D-SS-5 = WARN
# post: damaged = () = $d =~ /\xef\xbf\xbd/g == 0; (() = $d =~ /\r\n/g) == (() = $d =~ /\n/g); new-string counts ((6)); wr()
# payloads ($declH/$defC/$hookM/$tailM/$cntM) = the code blocks of 2-1/2-2/2-3/2-3b/2-3c, written with literal tabs in a <<'EOT' heredoc
```

---

## (10) 리뷰 반영 (2026-10-08 10:20~10:30 코드 재확인, 읽기 전용)

적대적 리뷰 8건을 하나씩 코드로 다시 확인했다. 소스, .rc, .ini, 레시피는 건드리지 않았다. 확인용 Perl 스크립트(`ss_rv_show.pl`, `ss_rv_cnt*.pl`)는 스크래치패드에서 돌린 뒤 지웠다.

### 10-1 트리 상태 재확인 (Perl raw 바이트)

| 파일 | mtime | CRLF / LF | damaged | BOM | 앵커 (n, 줄) |
|---|---|---|---|---|---|
| `DoWork/DoWorkGlassMeasure.cpp` | 10-08 10:06:20 (Phase 5 P15 적용 뒤) | 1124 / 1124 | 0 | 없음 | 2-3 훅 2줄 (1, :360-361) / 2-3b 3줄 (1, :407-409; 첫 줄만은 2: :407, :458) / 2-3c `AgingGlassLatch(...)` 줄 (1, :269) / `IsSamplingStickSkip`·`bSamplingSkip`·`nSs` 0개 |
| `DoWork/DoWorkGlassMeasureStep.cpp` | 10-08 09:59:40 | 7105 / 7105 | 0 | 없음 | (변경 없음) 참고 줄: WAD :2199/:2220/:2291, VISION :3183/:3203/:3265-3268, ANGLE :4718/:4742/:4806, LCR :5384, VISION `LAST_ITEM` 대기 :3998-4003, 주석 처리된 글라스 끝 대기 :4031-4037, `TakeVisionRcpWait` :3913, `AgingRepeatNext` :3142/:4041/:5441, None `SetFail(false)`/`SetStopUser(false)` :571/:572, Idle 완료 판정 :1980-1982 |
| `DoWork/MeasureSequenceItemEngine.h` | 09-30 17:25:49 | 136 / 136 | 0 | 없음 | 2-1 (1, :103) |
| `DoWork/MeasureSequenceItemEngine.cpp` | 09-30 17:25:49 | 1090 / 1090 | 0 | 없음 | 2-2 `AgingCycleEnd()` (1, :1072); (b1s)용 EFA begin 로그 2줄 (1, :836), AgingCycle begin 로그 2줄 (1, :887); `AgingItemHasCell` (1, :1025) |

ThreadRun 현재 구조(10:06 기준): `SetStick`(:334) → `GoToInitStep`/`nInitStep`(:335-336) → `[HYBRID]` 헤더(:339-351) → `AgingCycleBegin`(:354-356) → 3c 플래그(:360) / MCC true(:361) → while: 루프 머리 Phase 5 `TupleStickDone`(:368), `SegmentRestore0`(:369) → 마지막 스틱 분기(:400-450, 3c 가드 :407) / 그 밖(:451-499, 3c 가드 :458) / STOP 블록(:501-520) → MCC false(:524) → `AgingCycleEnd`(:527) → 진행 저장(:530-538).

### 10-2 지적별 검증과 반영

| # | 심각도 | 지적 | 코드 확인 | 판정 | 반영한 곳 |
|---|---|---|---|---|---|
| 1 | high | 꼬리 스틱이 skip되면 글라스 끝 VISION 결과 대기가 없어 마지막 측정 스틱의 VISION 데이터가 저장에서 빠질 수 있음 | 비동기 수신: `QualityWaitSet`(CommTcpIpObjectVision.cpp :1611), 스레드의 `CmFileFindTimeOut(..., 150000)`(:1546) → `TossResultFile`(:1553). 전체 대기는 VISION 스텝의 `iStick + 1 == GetStickCount()` 조건 안 `CmQualityEmptyTimeOut(LAST_ITEM, ...)`뿐(Step.cpp :3998-4003). 글라스 레벨 대기는 주석 처리(:4031-4037, 원작자 메모 "전수 검사 아닐시 처리 방안 확인 필요"). `SaveResult`는 `UploadDataDone`만 5 s 기다림(CDlgResult.cpp :1747-1765). OOS면 `QualityBufferEmptyAll()`이 TRUE(:1601). OCP는 스틱 루프 뒤/STOP 경로에서 글라스 레벨 대기(jobproc_utf8.cpp :12302-12303, :12325-12326). 버퍼가 비면 `CmQualityEmptyTimeOut`은 즉시 반환(CommTcpIpFactoryMgr.cpp :2167) | **확인** | (0), E5, 2-3(별도 플래그 `bSamplingSkip`), **2-3b 신설**, (3) 새 절, (4) WARN 표, (5)-7, (6) 앵커 (c), D-SS-6 신설, O3b, R1 신설, (9) |
| 2 | med | D-SS-3 권장 (a)는 AgingTest/EFA 글라스에서 요청 미충족, OCP 패리티 근거도 틀림 | OCP_A의 Sampling 주석 처리(`// 20240502 kks //if( bUseSampling )`, jobproc_utf8.cpp :12768/:12796/:12824/:12853) 확인. HMI 스텝은 사이클 중에도 `IsAgingCellSkip`과 `bUseSampling && m_nSelectCell != 1`을 함께 적용(VISION :3265-3268, WAD :2291, ANGLE :4806). `NeedOcpElectContact`는 사이클/EFA면 true(Step.cpp :53) | **확인** | (0), E7, 2-4 뒤 주석, D-SS-3 재작성((a)/(b1s)/(b1)/(b2)), O8a |
| 3 | med | O8은 기대값을 낼 수 없음(복사본은 EFA/Aging rcp 연결이 끊겨 표준 글라스가 됨) | `Load`의 경로 `{EFA_RCP 또는 AGING_RCP}\{레시피 이름}.rcp`(AgingCycleInfo.cpp :49, 경로 함수 :21-34). EFA 미발견 WARN `EFA: mode requested but efa rcp missing/empty - run standard sequence`(MeasureSequenceItemEngine.cpp :911), AgingTest 미발견 `AgingCycle: aging rcp not found - run standard sequence`(:855) | **확인** | 2-4 뒤 "복사본 레시피와 사이클의 연결", (8) 공통 준비, **O8 → O8a(연결 파일 복사) + O8b(대체 경로 일부러 확인)** |
| 4 | med | D-SS-5: 전 스틱 skip 글라스가 검사 0으로 정상 완료 - 경고 없음 대신 WARN 권장 | `S_CellInfo` 생성자 `ZeroMemory(&m_nSelectCell, ...)`(dfcRecipe preDefine.h :383), 로더는 버전 가드 없이 읽음(InRecipeFunc.cpp :359-362, OutRecipeFunc.cpp :358-361). Result 저장 게이트는 레시피 플래그(DoWorkGlassResultStep.cpp :83-112) | **확인** | E4, D-SS-5 권장안 변경(WARN), 2-3b/2-3c 구현(ThreadRun 카운터), (4), O6 |
| 5 | low | E10: 전 스틱 skip 세그먼트도 `SegmentBoundary`(Safety/회전/SW Align)를 돌고, Align NG면 글라스 중단 | `bTurn = (nRotFrom != nRotTo) or bForce`(DoWorkGlassMeasure.cpp :734), `MoveCmd_SafetyAll`(:817), SW Align 실패(:852-854) → 경계 실패 경로(:311-323) | **확인** | E10, **D-SS-8 신설(수용 비용)**, (9) 문서 항목 |
| 6 | low | E3(전기 전용은 절대 skip 안 함)는 사용자가 아니라 워크플로 스크립트의 규칙 | OCP `DoWorkStartMeasOCP`는 광학/LCR 그룹만 `aryMeasGroup`에 넣고 `nGroup < 1 → continue`(jobproc_utf8.cpp :6087-6212) → 전기 전용이면 Sampling과 상관없이 전 스틱 skip | **확인** | E3 표기 변경, **D-SS-7 신설**(권장: skip 안 함) |
| 7 | low | 레거시(Format 1, N=1) 재개에는 rcp 지문이 없음 | `m_nFormat` 1 = 레거시, `m_strFingerprint`는 Format 2 튜플 전용(ResumeInfoMgr.h :135/:149, :17 `RESUME_SHAPE_FILES`) | **확인** (기존 동작) | E11 한 줄, O9 |
| 8 | low | skip만 이어지는 런에서 STOP 유실 가능성이 더 큼 | `CDoWorkGlassMeasureStep_None::Run()`의 `SetStopUser(false)`(Step.cpp :572) | **확인** (기존 동작, 변경 없음) | E12, O10 |

### 10-3 리뷰 지적을 다듬은 부분 (검증 중 새로 확인)

- **반복 인덱스 주의는 세 종류 모두.** 리뷰는 VISION의 `AgingRepeatNext`만 들었지만 WAD(:3141-3142)와 ANGLE(:5440-5441)도 `bUseMeas`(항목 플래그 ∧ 마스크)만 보고 그룹 수와 상관없이 증가한다. (b1)의 결과 행 블록 이동 위험은 세 종류 모두에 해당한다.
- **(b1)의 Sampling 항은 셀 단위 교집합이어야 한다.** 리뷰 예시(`stick has a cell with m_nSelectCell[...]==1`)를 마스크(`IsStickUse`)와 따로 평가하면, 마스크 셀과 선택 셀이 겹치지 않는 스틱(예: 마스크 {1,2}, 선택 {3,4})을 "측정 있음"으로 잘못 본다. 스텝의 `pbUse`(마스크 ∧ 선택, 셀마다)와 같은 식으로 써야 한다.
- **(b1s) 추가.** 요청은 스틱 단위다. 항목 순서·반복 인덱스·3c 문구를 건드리지 않고 사이클 스틱만 skip하는 안을 권장안으로 넣었다(D-SS-3). 이 경우 2-3b 꼬리 대기가 사이클 Sampling skip에도 걸리도록 엔진 getter(`IsSamplingCycleSkip()`)를 훅 조건에 더한다.
- **타임아웃 단위.** `CmQualityEmptyTimeOut`은 초(`nTimeOut * 1000`, :2154)인데 `DEFINE_INSPECITON_WAIT_TIME`은 30000(CommTcpIpDefine.h :6, ms 의미)이다. 2-3b 대체값은 리터럴 30(초)으로 정했다. VISION 스텝은 레시피 포인터가 null일 때 이 define을 그대로 넘긴다(기존 결함, 후속 F4).
- **대기에 STOP 검사가 없다.** `CmQualityEmptyTimeOut`의 STOP 검사는 주석(:2184-2189)이다. 꼬리 대기 중 STOP은 최대 타임아웃까지 기다린다. VISION 스텝과 같은 동작이라 그대로 둔다(2-3b).
- **D-SS-5 WARN 구현 방식 변경.** 종전 안(엔진, 글라스 시작 사전 스캔 약 30줄)은 사이클/EFA 여부(D-SS-3에 좌우), aging rcp 존재, 재개 시작점을 사전에 재현해야 한다. ThreadRun 카운터(실제로 skip된 수)가 정확하고 짧다. 문구는 그대로 두고 `(sticks run %d)`만 붙였다. 로그 파일이 엔진 로그에서 측정 로그로 바뀐다.
- **리뷰의 "마지막 스틱 skip 분기 앵커"**는 3줄 연속으로만 n=1이다(첫 줄만이면 n=2). (6)과 부록에 3줄 앵커로 적었다.

### 10-4 문제 없음으로 확인된 리뷰 항목 (재확인 결과 동일)

- 삽입 위치/Contact/UnContact/Stage: skip은 모션 전이고, None은 플래그·SPS·측정 영역·센서 전원만 만진다. 다음 측정 스틱 MEAS MOVE가 Z5 UnContact 후 이동한다. 마지막 skip 꼬리는 정상 측정된 마지막 스틱과 같은 Z5 상태로 Unload에 간다.
- 하이브리드 스틱 인덱스: 훅은 `SegmentBoundary`(:311) 뒤의 세그먼트 로컬 `i`를 받는다. `GetRecipeInfo()/GetRecipeData()`는 활성 세그먼트다(`GetSegmentMgr`도 활성이면 `&m_recipeMgr`, OutRecipeFunc.cpp :639-644).
- 3c/EFA skip 경로: `&&` 단락으로 이중 로그 없음, `AgingCycleEnd`는 활성일 때만 "end"(:1074-1075).
- `m_nAgingStartStick[4]`, Idle/Exit("Thread End"), Phase 5 `SetStick`/`TupleStickDone` 도달성, Sampling OFF/전기 전용 불변, 인코딩/패치 위생, 로그 문구, 1셀 스틱(D-SS-4): 계획 그대로 유효.

### 10-5 후속 (이번 변경 범위 밖, 문서에 기록)

- **F1** 3c 마지막 스틱 skip과 EFA T0 flush(`EfaPassBoundary` → `SaveResult`)에도 같은 VISION 대기 틈이 있다. 3c 바이트 동일성을 지키려고 여기서는 고치지 않는다. 고친다면 `GoToExitStep()` 직전 대기를 `bAgingStickSkip` 전체로 넓히고, `EfaPassBoundary` 첫머리에 같은 `LAST_ITEM` 대기를 둔다. D-SS-3 (b1s)/(b1)을 고르면 이 틈이 Sampling으로도 열리므로 함께 검토한다.
- **F2** 스텝의 잔여 셀 패스(`// 선택하고 남은것`)가 `if (nArea > 1)` 밖에서 Sampling 검사 없이 1셀 스틱을 측정하는 결함(D-SS-4).
- **F3** VISION 장비 레시피 대기를 "글라스/세그먼트의 첫 VISION 그룹"으로 옮기는 일((3) 끝).
- **F4** VISION 스텝 `nTimeOut` 기본값(`DEFINE_INSPECITON_WAIT_TIME` = 30000)이 초 단위 API에 그대로 들어가는 기존 결함(레시피 포인터 null일 때만).

### 10-6 사용자 확인 목록 (갱신)

| 결정 | 권장안 | 비고 |
|---|---|---|
| D-SS-1 skip 범위 | 스틱 전체(이동부터) | `ss_decision.md`에 기록됨, 재확인만 |
| D-SS-2 적용 장비 | OCP만 | |
| D-SS-3 AgingTest/EFA 글라스 | **(b1s) 스틱 단위만** | (a)는 요청 미충족. (b1)은 항목 skip·반복 인덱스·3c 문구까지 바뀜. (b2)는 측정 대상 변경 |
| D-SS-4 1셀 스틱 | 선택 기준대로 skip | |
| D-SS-5 전 스틱 skip 글라스 | **측정 로그 WARN 1줄** | 종전 권장(경고 없음)에서 변경 |
| D-SS-6 꼬리 VISION 대기 타임아웃 | WARN만, 글라스 계속 | VISION 스텝의 실제 동작과 같은 수준 |
| D-SS-7 전기 전용 레시피 | skip 안 함 | OCP와 다름. 종전 "하드 룰" 표기 철회 |
| D-SS-8 전 스틱 skip 세그먼트의 경계 | 경계 유지(수용 비용) | Align NG면 글라스 중단 가능 |
