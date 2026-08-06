# Aging Cycle 측정 엔진 설계 플랜 (OCP DoWorkStartMeasOCP_A 대응)

작성: 2026-08-06 (LYS)
선행: [AgingTest_Porting_Plan.md](AgingTest_Porting_Plan.md) Phase 4 (START/STOP은 HMI 사이클 검사 경로로 연결 완료, 본 문서는 남겨진 "에이징 리스트 구동 측정 엔진"의 설계)

## 1. 배경

OCP는 AgingTest START 시 `m_bAutoAgingMode=TRUE`로 측정 진입점이 `DoWorkStartMeasOCP()` 대신 **`DoWorkStartMeasOCP_A()`** (JOB_Proc.cpp 12351~19094, **6,743줄**)로 분기한다. HMI에는 이 엔진이 없어, 현재 AgingTest START는 표준 측정 시퀀스를 사이클 수만큼 반복할 뿐 에이징 리스트(점등 패턴/유지시간/검사 선택)를 소비하지 않는다.

## 2. OCP DoWorkStartMeasOCP_A 구조 분석 (골격)

행 번호는 함수 시작(12351) 기준 상대값.

```
준비 (:198~233)
  nAgingListCount = m_vecAginList.size()
  nRepeatCount = max(리스트 중 특성 사용 수, 화질 사용 수, LCR 사용 수)
  nTGroup = MEASGROUP 수 × nRepeatCount        ← 결과 그룹 버퍼 확장
  AutoContact 상태 준비 (:254~275)

for iTop < nMaxTop:                              ← 스틱(Top) 이동 루프
  for AgingCnt < nAgingListCount:                ← ★ 에이징 리스트 루프
    진행표시: m_nAgingListRow = AgingCnt+1, m_nAgingListCol = -1  (:349)
    sAgingTest = 리스트[AgingCnt] → bUseProperty/bUseQuality/bUseLCR (:352~356)
    Contact/전기검사는 AgingCnt==0에서만 수행 (:379 주석 "2번 이상이면 한번만")
    Bottom(2Head) 루프: 셀맵 GetMeasUse(k, stProbe[0].m_iUsedMeas)로
        k=0(WAD)/1(VISION)/2(Color)/3(LCR) 셀별 측정 대상 마스킹 (:406~512)
    Contact 좌표 이동/AutoContact (:530~919, Col=1)
    ★ 점등 (:1315~1357, Col=2→3):
        if (m_nPtnRGB > 0):
            ExePGCtrl(PG_CMD_TURNON_ALL)          ← 전체 점등
            (m_nPtnRGB - 1)회 ExePGCtrl(PG_CMD_NEXT_PTN_ALL)  ← 패턴 이동
            m_nPtnSec 초 대기 (tick 루프)
            ExePGCtrl(7)                          ← Turn Off
    ★ 측정 dispatch (리스트 플래그 × 셀맵):
        특성(WAD):  Col=4, ExePGCtrl(11) + per-probe ExePGCtrl(5+nProbe, CELLID) (:1394~, 2059)
        화질(VISION): Col=5, ExePGCtrl(12) (:4627~4631)
        색이상(Color): ExePGCtrl(14) (:5630)
        LCR: 전용 분기 (Col=7, :2680)
        ExeSubMeas(iTop, 0)=특성검사, (iTop, 10)=Contact (:831, 919)
    결과 저장 슬롯 = MEASGROUP × nRepeatCount 오프셋 (리스트 회차별 분리)
```

핵심 요약: **[스틱 → 리스트 항목 → (점등→대기→소등) → 항목 플래그×셀맵으로 선별 측정]**의 2중 루프이며, 측정 자체는 표준 엔진과 같은 하위 함수를 쓰되 진행 마커(m_nAgingListRow/Col)와 결과 슬롯 오프셋만 다르다.

## 3. HMI 대응 설계

### 3.1 설계 원칙

1. **기존 스텝 재사용 극대화** — HMI `CDoWorkGlassMeasure` 스텝 체인(MoveMeasureArea→PtrnSearch→Probe_Contact→OST→RTEST→AGING→**Measure_WAD→Measure_ANGLE→Measure_VISION**→Idle)이 이미 검증됨. 6,743줄 직이식 금지.
2. **다이얼로그 비의존** — OCP는 시퀀스에서 `m_dlgNaviManualAgingTest.m_vecAginList`를 직접 읽음. HMI는 비-UI 헬퍼가 AgingRcp INI를 재로드 (AgingAlarmCheck 선례).
3. **가드 (사용자 확정 2026-08-06)**: 엔진 활성 옵션 = **Recipe의 `m_sInspectItem.m_bUseAgingTest_2` 플래그** (`preDefine.h:106`, 기존 P5 AGING 스텝 게이트와 동일 필드) + `bIsOCP`. 즉 레시피 단위로 에이징 사이클 동작이 결정되며, `m_bAutoAgingMode`는 수동(AgingTest START) 기동 표식/진행표시용으로 병용. 표준/CIM 시퀀스 무영향 원칙 유지.
4. LCR → **ANGLE** 대체 (AgingTest P2 결정과 일관).
5. **P5 1스틱 고정 (사용자 확정 2026-08-06)**: PG 명령은 OCP의 프로브별(5+nProbe) 축 없이 P5/단일 스틱 기준 — HMI PG는 단일 장비라 S6(AGING 시작)은 이미 `m_nAgingStartStick[4]`(P5 슬롯)를 사용하고, 셀 지정은 S8 Cell Command의 CellNum만 사용하면 됨 (자연 충족 확인).

### 3.2 구성 요소

| # | 컴포넌트 | 신규/수정 | 내용 |
|---|---|---|---|
| C1 | `CAgingCycleInfo` (DoWork/AgingCycleInfo.h/.cpp) | 신규 | AgingRcp INI 로더: 리스트 6필드(`USE_AGING/PTN_RGB/PTN_SEC/USE_PROPERTY/USE_QUALITY/USE_ANGLE`) + `[MASECELL]` 셀맵(bit0~3=WAD/VISION/Color/ANGLE). 시퀀스 스레드 안전, `RecipeEditMgr::GetRecipeID()` 기준 |
| C2 | PG 패턴 명령 3종 | **기존 재사용 (E1 완료 — 코드 변경 없음)** | §E1 결과 참조 — `enumTCPIpFactoryt_PGEvent_5/6/8`이 OCP와 동일 프로토콜 코드로 이미 구현·실사용 중 |
| C3 | 리스트 루프 분기 | `MeasureSequenceItemEngine` 수정 | §3.3 옵션 B — 스텝 벡터 반복 제어 |
| C4 | 진행 표시 | 엔진 + `DlgSubManual_AgingTest` | 엔진이 `m_nAgingListRow/Col` 갱신, 다이얼로그는 `SetTimer` 1초 폴링으로 현재 항목 하이라이트(`SetColor`) — UpdateStatus 미호출 구조 대응 |
| C5 | 결과 슬롯 | 확인 후 결정 | OCP nRepeatCount×MEASGROUP 확장 대응 — HMI 결과 구조(dResult)에서 리스트 회차별 분리 저장 방식 확인 필요 |

### 3.3 엔진 통합 지점 — 옵션 비교

**옵션 A — 단일 신규 스텝 `CDoWorkGlassMeasureStep_AGINGCYCLE`**
리스트 루프+점등+측정 dispatch를 한 스텝이 전부 수행. 측정은 `CommTcpIpFactoryMgr::DoWorkInspectStart/Complate`를 직접 호출(기존 Measure_WAD/ANGLE/VISION 스텝의 통신 핵심을 미러).
- 장점: 기존 엔진/스텝 무수정, 영향 범위 최소
- 단점: WAD/ANGLE/VISION 스텝의 이동·조명·스냅 로직 중복(각 수백 줄), 이중 유지보수

**옵션 B — 엔진 레벨 리스트 루프 (권장)**
`MeasureSequenceItemEngine`(스틱 루프 보유, `IsCycleState/GetCycleCount` 소비 지점)에 aging 모드 분기:
```
if (m_bAutoAgingMode) {
    CAgingCycleInfo 로드 (스틱 진입 시 1회)
    for each 리스트 항목 (USE_AGING=1):
        m_nAgingListRow/Col 갱신
        신규 스텝 CDoWorkGlassMeasureStep_AGING_PTN 실행:  ← 점등→PtnSec 대기→소등 (C2 사용)
        기존 Measure_WAD/ANGLE/VISION 스텝 재실행:
            스텝 진입 가드에서 항목 플래그(bUseProperty/Quality/Angle) 및
            셀맵(스틱×셀 bit)으로 skip/제한   ← 스텝에 aging 컨텍스트 전달
    Contact/PtrnSearch류 스텝은 첫 항목에서만 (OCP :379 정책)
}
```
- 장점: 이동/조명/스냅/결과 로직 전부 재사용, OCP 구조(항목별 선별 측정)와 1:1 대응
- 단점: 엔진 스텝 진행 로직(step index 제어) 수정 — 회귀 리스크 → `m_bAutoAgingMode` 가드로 격리 + 표준 경로 무수정 원칙

**결정: 옵션 B.** 스텝별 skip 가드는 각 Measure_* 스텝 `Run()` 선두의 `bUseMeas` 판정 지점에 aging 분기 추가(레시피 플래그 대신 리스트 항목 플래그 사용) — 스텝당 수 줄.

### 3.4 셀맵 적용 방식 (확인 필요)

OCP는 `ExePGCtrl(5+nProbe, CELLID)`로 PG에 셀 단위 측정 지시. HMI PG 프로토콜에 셀 지정 명령이 있는지 확인 필요 — 없으면 1차 구현은 **스틱 단위 필터**(해당 스틱의 셀맵 비트가 하나라도 1이면 해당 검사 수행)로 낮추고, 셀 단위는 PG 협의 후. (**TODO: AGINGCYCLE_CONFIRM-1**)

## 4. 실행 Phase (승인 후 각각 진행)

| Phase | 내용 | 규모 추정 |
|---|---|---|
| E1 | PG 프로토콜 확인(문서/OCP PGPROTOCOL 코드 대조) + C2 패턴 명령 3종 + OOS 가드 | ✅ 완료 — **코드 변경 0건** (아래 결과) |
| E2 | C1 `CAgingCycleInfo` + 단위 확인(INI 왕복) | ✅ 완료 (2026-08-06, 아래 결과) |
| E3 | C3 엔진 분기 + AGING_PTN 스텝 + Measure_* 스텝 aging 가드 | ✅ 완료 (2026-08-06, 아래 결과) |
| E4 | C4 진행 표시(공유메모리→다이얼로그 타이머) + C5 결과 슬롯 + OOS 통합 검증 | ✅ 구현 완료 (2026-08-06, 아래 결과 — OOS 통합 검증 실행 대기) |

### E1 결과 (2026-08-06) — 패턴 명령은 HMI에 기구현, 프로토콜 완전 일치

| 기능 | OCP (DlgNaviMaint.cpp ExePGCtrl) | HMI (CommTcpIpObjectPG.cpp `_ExePGCtrl`) | 코드 일치 |
|---|---|---|---|
| 전체 점등 | `PG_CMD_TURNON_ALL(8)`: S8/F1/C1/Cell0 (:3085) | `enumTCPIpFactoryt_PGEvent_5` "Turn On": S8/F1/C1/Cell0 (:1992) | ✅ |
| 다음 패턴 | `PG_CMD_NEXT_PTN_ALL(9)`: S8/F1/C3/Cell0 (:3095) | `enumTCPIpFactoryt_PGEvent_6` "Pattern Next": S8/F1/C3/Cell0 (:2006) | ✅ |
| 소등 | iEvent 7: S10/F3 (:3077) | `enumTCPIpFactoryt_PGEvent_8` "Turn Off": S10/F3 (:2032) | ✅ |
| 셀 점등 L/R | iEvent 5/6: S1/F5/C0·1+CellNum (:3056) | `SetCommand` iFunc==5 분기에서 인코딩 지원 (:1772) — 전용 이벤트는 미정의 (E3 필요 시 추가) | 인코딩 지원 |

- **호출 경로 검증 완료**: `DoWorkInspectStart(enumTCPIpFactorytID_PG, PGEvent_5/6/8)` — `DlgSubManual_Meas`(수동 점등 버튼 :2110~2143)과 측정 시퀀스(:3976 Turn Off)에서 이미 실사용 중인 검증된 경로
- **OOS 가드 내장**: `SetCommand`가 패킷 조립·KMLOG(SEND) 기록 후 송신 직전 OOS면 성공 반환(:1811) → OOS 검증 시 송신 문자열이 로그로 남아 E3 검증에 활용 가능
- **OCP와의 차이 1건**: HMI PGEvent_5/6은 ACK 후 `WaitInspectionComplete(2초)`를 추가 대기 (OCP는 ACK만) — HMI 신판 PG 응답 규약에 맞춘 기존 동작으로 그대로 수용. E3에서 (패턴번호-1)회 반복 호출 시 tact 영향 실측 확인
- 결론: **E2로 직행 가능**. C2 신규 구현 불필요

### E2 결과 (2026-08-06) — CAgingCycleInfo 구현 완료

- 신규 `DoWork/AgingCycleInfo.h/.cpp` (~200줄, ASCII): `Load(rcpName)` → 리스트 6필드 + `[MASECELL]` STICK_1..N 스캔(빈 키에서 중단, 상한 100). API: `GetItemCount/GetItem`, `GetStickCount/GetCellCount`, `IsCellUse(iStick, iCell, kind)`, `IsStickUse(iStick, kind)`. 시퀀스 엔진 단일 스레드 소유 전제. vcxproj/.filters 등록, Debug/Release x64 빌드 통과
- **측정 종류 3종 확정 (사용자 지시 2026-08-06)**: HMI에는 COLOR 모드가 없음 → kind = `enumAgingMeasKind_WAD/VISION/ANGLE`(**bit0~2**), 셀상태 그리드도 3시트로 축소(다이얼로그 UpdateAgingCell/MeasCellSave/Load — 시트 배열·bitset<3>·k루프는 사용자 직접 수정분 확인, 주석 정리는 LYS). §5-3(Color 처리) 확인 항목 해소. **주의**: 3시트 변경 전 저장된 [MASECELL]의 값 4(구 Color비트)는 신체계에서 ANGLE로 해석됨 — 기존 테스트 파일 재저장 권장
- **실파일 왕복 검증**: 다이얼로그(AgingTest P1~3)로 저장된 실파일 `AgingRcp\0_OCP_TEST_260720.rcp`(12스틱×9셀, 3항목) 포맷이 파서와 완전 호환 — 트레일링 콤마는 Tokenize가 자연 처리. 기대값: Item3건(PTN1/특성, PTN4/화질, PTN9/ANGLE), STICK_1="1,0,2,0,4,..." → IsCellUse(0,0,WAD)/(0,2,VISION)/(0,4,COLOR)=true
- **마이너 발견**: 다이얼로그 Save 시 미체크 체크박스가 빈 값(`USE_QUALITY=`)으로 저장됨 — `_ttoi("")=0`이라 판정은 정상(FALSE), 기능 무해로 현행 유지 (수정 최소화 원칙). 필요 시 Save에서 %d 정규화 가능
- 검증용 추가 픽스처: `AgingRcp\TEST.rcp` (2항목, 3비트 체계 STICK_1=7,1,0 / STICK_2=2,4,0 — 마스크 조합 케이스)

### E3 결과 (2026-08-06) — 엔진 분기 구현 완료

**설계 정정 (OCP 재확인)**: `_A`도 아이템 루프 **안에서** `ExeSubMeas(iTop,3)`(S6 AGING 측정 + 에이징 알람 훅 7495~7497)을 수행하고 **그 뒤에** 점등 블록이 옴 → 루프백 타깃은 AGING_PTN이 아니라 **기존 AGING 스텝**. 따라서 AgingSet P4의 알람 훅도 아이템마다 자연 반복 (AGING 스텝 수정 불필요).

**스텝 체인 (aging 모드)**: `None→MoveMeasureArea→PtrnSearch→Probe_Contact→OST→RTEST→[**AGING→AGING_PTN(신규 점등)→WAD→ANGLE→VISION**]×아이템→Idle` — Contact/전기검사는 루프 밖(첫 회만, OCP :379 정책 자연 충족).

**구현 파일** (damaged=0, Debug/Release x64 빌드 통과, 백업 `.bak_e3`):
| 파일 | 변경 |
|---|---|
| `BaseSequenceBody.h` | `GoToStep(long)` 추가 (additive, 임의 스텝 점프) |
| `MeasureSequenceItemEngine.h/.cpp` | aging 컨텍스트: `AgingCycleBegin/End/NextItem`, `IsAgingCycleActive`, `GetAgingCurItem`, `GetAgingCycleInfo` + `CAgingCycleInfo` 멤버. Begin 게이트 = `IsOCP() && m_bUseAgingTest_2 && Load(GetRecipeID())` — 실패 시 표준 시퀀스 폴백(로그) |
| `DoWorkGlassMeasureStep.h/.cpp` | 신규 `CDoWorkGlassMeasureStep_AGING_PTN`("AgingPtn"): PGEvent_5 → (PTN-1)×PGEvent_6 → PtnSec 대기(100ms 폴링, `IfPauseEvt_WaitResumeEvt`로 Stop 즉응) → PGEvent_8 소등(항상 시도). 진행표시 Row=아이템, Col=2(점등)/3(유지). 비활성 시 pass-through |
| `DoWorkGlassMeasureStep.cpp` (게이트) | WAD/VISION/ANGLE 스텝 `bUseMeas`에 aging 분기: 아이템 플래그(Property/Quality/Angle) × 스틱 셀맵(`IsStickUse`, 맵 없으면 무필터) — **스틱 단위 필터(§3.4 1차)**. ANGLE에서는 `bUseMeasLCR=false`. VISION 게이트는 함수 영역 스코프(비활성 Measure_VISION_COLOR의 동일 라인 미접촉) |
| `DoWorkGlassMeasure.cpp` | AGING_PTN 등록(AGING 뒤) + ThreadRun: 스텝 인덱스 이름 스캔("Aging"/"VISION" — 하드코딩 회피) → 스틱 진입 시 `AgingCycleBegin` → VISION 성공 시 다음 아이템 있으면 `GoToStep(AGING)`+continue (마지막/비마지막 스틱 양 분기) → 스틱 종료 시 `AgingCycleEnd`(진행표시 리셋) |

**표준 시퀀스 영향**: `bAgingCycle=false`면 삽입 코드 전부 미동작 — 기존 흐름 보존 (Begin 게이트 3중: OCP옵션+레시피 플래그+파일 존재).

**E4 이관**: 진행표시 다이얼로그 타이머(C4), 결과 슬롯 회차 분리(C5, §5-4), OOS 통합 검증(점등 SEND 로그 순서/대기시간/스텝 skip 확인), 셀 단위 필터 확장(§3.4 — 현재 스틱 단위)

### E3 검증 중 발견/수정 (2026-08-06) — Cross Interlock 3027

OOS 검증 run에서 aging item2 VISION 진입 시 **알람 3027(Cross Interlock, `MOTION_INTERLOCK_7` — X2↔X3 갠트리 갭 385mm)** 발생. 원인: per-item 선별로 ANGLE이 skip되면 X2(BACK GANTRY)가 WAD 종료 위치(+79.8)에 잔류한 채 VISION이 X3 단독 이동(-121.25, 갭 201mm). 표준 흐름은 사이에 ANGLE이 X2를 633으로 옮겨 우연히 통과 — per-item 선별이 처음 노출한 잠복 버그.

**근본 원인**: `MaintControlMgr::MoveCmdPosVision`의 X2 선대피 판정이 `GetCurPos(AXIS_X2)`(직전 `SetHeadOffset(X2,-204.09)` 반영, 오프셋 차감값 283.9) 기준 → "이미 대피"로 오판. 정작 인터록 검사는 raw(`GetOrgPos`=79.8) 기준 — 판정/검사 기준 불일치.

**수정**: `MaintControlMgr.cpp:2299` 판정을 `m_pMotorControlMgr->GetOrgPos(AXIS_X2)`(raw)로 변경 (백업 `.bak_e3fix`). 표준 케이스(X2=633)는 분기 결과 동일 — 무영향. 동일 패턴 `MoveCmdPosColor(:2543)`는 COLOR 스텝 비활성+오판 방향이 안전(불필요 대피)이라 미수정. **주의**: `CMaintControlMgr::GetOrgPos`는 내부적으로 오프셋 차감 GetCurPos 호출 — raw는 `m_pMotorControlMgr->GetOrgPos` 직접 호출 필수.

### E4 결과 (2026-08-06) — 진행표시 + 결과슬롯 결정 (백업 `.bak_e4`)

**C4 진행 표시 (구현 완료)**:
- 엔진→UI 채널은 기존 공유메모리 필드 재사용: `m_nAgingListRow`(1-base 항목 행) / `m_nAgingListCol`(그리드 컬럼과 1:1 — 1=Aging, 2=점등패턴, 3=유지시간, 4=특성, 5=화질, 6=시야각). E3에서 AGING_PTN이 Col=2/3 기록 완료 → E4에서 나머지 마커 추가:
  - `DoWorkGlassMeasureStep.cpp`: AGING 스텝(사이클 활성 시 Col=1), WAD/VISION/ANGLE 게이트의 aging 분기 내 `bUseMeas==true`일 때 Col=4/5/6 (표준 시퀀스 무영향 — aging 분기 내부에만 삽입)
  - `DlgSubManual_AgingTest.h/.cpp`: `SetTimer(1, 500ms)` + `OnTimer` — `m_bAutoAgingMode`일 때 (Row,Col) 변화 감지 시 이전 셀 복원(RGB_WHITE)→현재 셀 하이라이트(`SetColor(Col,Row,RGB_YELLOW,RGB_BLACK)`; FarPoint `SetColor`는 Col,Row 순서). 정지/종료 시 Row=Col=0 → 자동 해제 (AgingCycleEnd/OnStop 기존 리셋 재사용). OCP UpdateStatus 방식(전체 클리어+짝수회 갱신) 대신 변화 감지 단일 셀 갱신으로 부하 축소
- OCP와의 차이: OCP Col=1은 Contact였으나 HMI는 Contact가 루프 밖(첫 회만)이라 Col=1을 AGING(S6 측정)에 배정 — 그리드 헤더와 의미 일치

**C5 결과 슬롯 (확인 완료 — 1차 last-write-wins 확정)**:
- HMI 결과 경로: 측정 수신(CommTcpIpObjectWAD/Vision/Angle) → `CResultMgr::Updata_Data(RSRITEMDATA)` → dResult DLL `API_Result_Upload`. `RSRITEMDATA`는 위치 키(iCurArea/iCurStep/iPanelNo/셀데이터) 기준이며 **회차(repeat) 차원이 없음** → 같은 검사 종류를 복수 항목이 같은 스틱에서 중복 측정하면 뒤 회차가 덮어씀
- OCP식 회차 분리(nTGroup×nRepeat 버퍼 확장)는 dResult DLL 구조체/리포트 그리드/저장 포맷 동시 변경 필요(공유 구조체 계약, 고위험) → **1차 결정: last-write-wins 수용**. 실사용 레시피는 항목별 검사 종류가 겹치지 않는 구성(특성/화질/시야각 각 1회)이 기본이라 실충돌 없음. 회차 분리 요구 발생 시 `RSRITEMDATA` 끝에 회차 필드 추가(WAD MultiPoint `nMultiPoint` 선례) + dResult 확장으로 별도 진행 (§5-4 해소)

**셀 단위 필터 (§3.4 — 보류 유지)**: PG 셀 지정 점등 협의(AGINGCYCLE_CONFIRM-1) 전에는 SW측 셀 필터만으로는 소등 셀 측정이 되어 불완전 + 회전(iRotate)에 따른 셀 인덱스↔[MASECELL] CSV 정렬 검증 필요 → 스틱 단위 유지, PG 협의 후 확장

**OOS 통합 검증 (2026-08-06 완료)**: 16:05~16:15 풀런(스틱 1~10, 인터록 수정 exe) + 16:49 Resume 런(스틱 12, E4 exe) 로그 판독 결과:
- ① **3항목 루프백 × 전 스틱 완주, NG 0건** — "Thread NG"/"Fail Exit"는 전부 수정 전(≤15:06) 기록, 16시 이후 없음. 스틱당 item(1)→(2)→(3) AGING_PTN 반복 확인
- ② **3027 수정 검증** — 스틱 1/2/3 VISION마다 `BACK GANTRY→263.750`(X3목표+갭385) 선대피 후 `FRONT GANTRY→-121.250` RET:1 (Motion 로그 16:06:24/16:07:55/16:09:27), 이후 화질 헤드 스윕 실측 진행
- ④ **PG SEND 순서** (Control/PG/PG_20260806.log): 항목마다 [S6 AGING 측정(050100/060100...) → `08010100`(TurnOn) → `08010300`×(PTN-1) (item1 ptn1=0회/item2 ptn4=3회/item3 ptn9=8회) → 5초 유지 → `1003`(TurnOff)] — OCP ExePGCtrl 순서와 완전 일치
- **스틱 필터 동작 검증**: GetMeasureStick() 0-base = [MASECELL] STICK_(n+1) **1:1 매핑(반전 없음)** — 16:49 런(스틱 인덱스 11=STICK_12)에서 item1 WAD skip(비트 무)/item3 ANGLE 실측 12.3s(비트 유, TP삽입 010104+셀점등 0105001/0105106+ANGLE 측정) — 셀맵 편집 내용 그대로 반영
- ③ 하이라이트는 로그로 판독 불가(UI) — AgingTest START(m_bAutoAgingMode=TRUE) 기동 시 육안 확인 필요. Operate/Resume 기동은 m_bAutoAgingMode=FALSE라 표시 안 됨(설계 의도)
- 잔여 원복: 레지스트리 `CONFIG\AGING_ALARM_USE=1`, PTN_SEC 실사용값(100) — 실기 전환 시

검증(각 Phase): OOS 시뮬레이션으로 [리스트 3항목 × 패턴/시간 상이] 기동 → KMLOG로 점등 명령 순서/대기 시간/스텝 skip 여부 확인 → PG 시뮬레이터(기존 elp_elecbox_sim.py 방식) 확장 여부는 E1에서 판단.

## 5. 확인 필요 항목 (AGINGCYCLE_CONFIRM)

1. ~~PG 셀 지정 측정 명령~~ → **E1에서 해소**: S8 Cell Command에 CellNum 인코딩 존재, P5 단일 장비라 프로브 축 불필요 (§3.1-5)
2. ~~PG 패턴 명령 코드~~ → **E1에서 해소**: OCP와 완전 일치 확인 (§E1 결과)
3. ~~Color 측정~~ → **해소 (사용자 확정 2026-08-06)**: HMI에 COLOR 모드 없음 — 측정 종류는 WAD/VISION/ANGLE 3종(bit0~2), Color 비트 제거 (E2 결과 참조)
4. **결과 저장**: 리스트 회차별 결과 분리(OCP nTGroup×nRepeat) vs 마지막 회차 덮어쓰기 — dResult 구조 확인 후 결정
5. **Contact 1회 정책**: OCP는 리스트 첫 항목에서만 Contact — HMI Probe_Contact 스텝 skip 조건으로 동일 적용 여부
6. **전기검사(OST/RTEST)**: 에이징 사이클 중 수행 여부 (OCP는 첫 항목만)

## 6. 리스크

| 리스크 | 완화 |
|---|---|
| MeasureSequenceItemEngine 수정으로 표준 시퀀스 회귀 | `m_bAutoAgingMode` 가드 외부 코드 무수정 원칙, OOS로 표준/에이징 양 경로 회귀 테스트 |
| PG 패턴 명령 코드 오매핑 (실기 점등 오동작) | E1에서 프로토콜 문서 + OCP 코드 이중 대조, 실기 검증 전 OOS 로그 검증 |
| PtnSec 대기 중 Pause/Stop 미반응 | 대기 루프에 `IfPauseEvt_WaitResumeEvt()` 삽입 (AGING 스텝 :5939 선례) |
| 리스트 항목 다수 × 사이클 다수 시 tact 폭증 | 진행 표시(C4)로 가시화, Stop 즉시 반응 보장 |
