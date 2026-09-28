# EFA 검사(Manual Aging NEW) 이식 플랜 (OCP_EFA A3_ICP → HMI)

작성: 2026-09-28 (LYS)
원본: `D:\OCP\0.SW_Matching\OCP_EFA\A3_ICP_v3.1.0.2_20260915\` — `IDD_DLG_NAVI_MANUAL_AGING_NEW` (`CDlgNaviManualAgingNew`, 961줄, 2026-03-18 kks 신설, 버튼 라벨 **"EFA TEST"**)
대상: HMI `IDD_DLG_SUB_MANUAL_AGING_NEW` (`CDlgSubManual_AgingNew`, 신규) + 시퀀스 엔진 EFA 2-Pass 모드
선례: [AgingTest_Porting_Plan.md](AgingTest_Porting_Plan.md) (다이얼로그 삽입 구조), [AgingCycleEngine_Design_Plan.md](AgingCycleEngine_Design_Plan.md) (엔진 루프백), [AgingResult_RepeatSlot_Plan.md](AgingResult_RepeatSlot_Plan.md) (결과 행 산식)

---

## 0. 한 줄 요약

**다이얼로그는 AgingTest의 복제본**(diff 60줄)이고, **진짜 기능은 JOB_Proc의 `DoWorkStartMeasICP_EFA()`(3,600줄)와 CIM `EFA^0$/EFA^1$` 핸드셰이크**에 있다.
강화검사(`_A`)가 *스틱마다 [항목1→항목2…]* 를 도는 반면, EFA 검사는 *항목(=Pass)을 바깥 루프로 두고 글라스 전체를 두 번 측정*한다: **T0(에이징 전 검사) → 결과 저장·APD 전송·`EFA^0$` → T1(에이징 후 검사) → `EFA^1$`**.
HMI에는 AgingTest 이식 때 만든 엔진(`CAgingCycleInfo` + `AgingCycleBegin/NextItem/End` + `AGING_PTN` 스텝 + 진행표시)이 이미 있으므로, **UI 복제 + 공유상태 1개 + 엔진에 Pass 루프 + CIM 전문 4개**로 이식한다.

---

## 1. 원본 분석 — 기능 코드 단위

### 1.1 다이얼로그 `CDlgNaviManualAgingNew` (vs `CDlgNaviManualAgingTest`, 같은 EFA 트리)

`diff` 결과 실질 차이는 아래 9개뿐이다. 나머지 900줄은 AgingTest와 동일(HMI에 이미 이식됨).

| # | 항목 | AgingTest (강화검사) | AgingNew (EFA 검사) | HMI 반영 |
|---|---|---|---|---|
| D1 | 모드 플래그 | `m_pSystem->m_bAutoAgingMode` | **`m_pSystem->m_bNewAgingMode`** (ShareMem.h:1444, 별도 BOOL) | `VTM_SYSTEM` 끝에 `m_bNewAgingMode` 추가 |
| D2 | 레시피 폴더 | 레지스트리 `DIR\AGING_RCP`, 기본 `…\AgingRcp\` | **같은 키** `DIR\AGING_RCP`, 기본 `…\EFARcp\` | **별도 키 `DIR\EFA_RCP`** 사용 (§3.2 — 원본은 키 공유 버그) |
| D3 | 리스트 행 수 | 가변 (ITEM 추가/제거) | **2행 고정** (`AgingItemClearAll(TRUE)` "EFA는 리스트 2개 고정", 추가/제거 버튼 `NOT WS_VISIBLE`+핸들러 `return`) | 2행 고정, 버튼 숨김 |
| D4 | Cycle 콤보 | 1~10 | **1~25** | 1~25 |
| D5 | 점등패턴 콤보 | 0~25 (EFA 트리에서 10→25 확장, 20260403) | 0~25 | HMI AgingTest는 0~10 → **0~25** |
| D6 | START 사전검사 | Offline + Process + `m_bSafetyPos_X` + R1 각도 | Offline + Process만 (Safety 검사 제거) | HMI AgingTest P4 검사(OCP·레시피 플래그·rcp 존재) 재사용 |
| D7 | 초기화 확인문구 | "전체 리스트를 삭제합니다" | "전체 리스트를 초기화합니다" | 영어 메시지(파일 인코딩 규칙) |
| D8 | 셀맵 로드 | `MeasCellLoad`가 INI→MEASGROUP 미러 | **미러 코드 제거됨** — INI를 읽고 `GetAt/SetAt` 같은 값 → 사실상 no-op (죽은 코드) | HMI 방식(INI→그리드 직접)이 이미 정상 |
| D9 | 하이브리드 레시피 | `LoadAgingInfo(bHybrid)`, `m_RecipeAdd.m_bHybrid/m_vSpan/m_nDivisionCount`로 행열 재계산 | 동일 | **범위 외** — HMI에 하이브리드 레시피 개념 없음(`grep -i hybrid` 0건) |

공통(양쪽 동일, HMI 기이식): INI 포맷 `[COUNT] MaxCount`, `[ITEM_n] USE_AGING/PTN_RGB/PTN_SEC/USE_PROPERTY/USE_QUALITY`, `[MASECELL] STICK_n=비트마스크CSV`, `S_AgingTest` 5필드(JOB_Proc.h:225 — EFA 트리엔 LCR 없음), 진행 하이라이트 `m_nAgingListRow/Col`(Col 1=Aging 2=점등 3=유지 4=특성 5=화질).

### 1.2 시퀀스 — `m_bNewAgingMode` 소비처 전수 (Metrosys, 14곳)

| 위치 | 함수 | 역할 | HMI 대응 |
|---|---|---|---|
| `DlgNaviManualAgingNew.cpp:277` | `OnAgingProcStrart` | `m_bNewAgingMode=TRUE`, `nJobCount=Cycle`, Row/Col=0, `DoStartCycleTest()` | 다이얼로그 START (AgingTest P4 복제) |
| `DlgNaviMaintCIM.cpp:704-724` | PPID 수신 | 2번째 필드 4자리 중 **`Mid(1,1)=='B'`**(강화검사는 `'A'`) → `AgingNew.LoadAgingInfo()` 실패 시 `PPIDREQ_NG`+`JOB_RECIPE_NOT_EXIST`, 성공 시 `m_bNewAgingMode=TRUE`, `nJobCount=1` | `CCommTcpIpObjectCIM` PPID 핸들러(:1388) — **⚠ HMI는 현재 `'J'`로 강화검사 판정** (§5 결정 필요) |
| `DlgNaviMaintCIM.cpp:2140,2176` | `Write/ReadGalssExistFile` | `GlassExist.ini [GlassExist] EFAMode` 저장/복원(재기동 시 모드 유지) | HMI `m_bAutoAgingMode`도 미영속 → 패리티 유지, 필요 시 ResumeInfoMgr |
| `DlgNaviOperate.cpp:631` / `CIM:671` / `JOB_Proc:123` | 사이클·PPID·초기화 | 시작 시 `m_bAutoAgingMode`/`m_bNewAgingMode` 둘 다 FALSE로 리셋(**상호 배타**) | `SetSys_AutoAgingMode(false)` 옆에 `SetSys_NewAgingMode(false)` |
| `JOB_Proc.cpp:1769` | `DoWorkStartCycleTest` | 사이클마다 `LoadPPID2` 후 `AgingNew.LoadAgingInfo()` | 엔진 `AgingCycleBegin`이 파일 직접 로드(HMI 방식) |
| `JOB_Proc.cpp:14260` | `DoWorkStartGlassMeasReq2` | 리스트에 `m_bUseQuality` 항목 있으면 화질 레시피 Open(`ExeQuality(0)`) | HMI VISION 스텝 자체 게이트 |
| `JOB_Proc.cpp:14595` | `DoWorkStartGlassMeasReq2` | **`DoWorkStartMeasICP_EFA()`로 분기** (`_A`/표준과 3-way) | `CDoWorkGlassMeasure::ThreadRun` Pass 루프 |
| `JOB_Proc.cpp:9558-13162` | **`DoWorkStartMeasICP_EFA`** | §1.3 | §3.4 |
| `JOB_Proc.cpp:311` | `GoToWork` WORK_START_MEAS2 종료 | ONLINE_REMOTE: **`Cmd_Send_EFA1()` → 500ms → `Cmd_Send_WORKSTATE_CHANGE("_CAA")` → `STS1106`** | `DoWorkSequenceCIMStep.cpp:772` STS1106 직전 |
| `JOB_Proc.cpp:14985` | `DoWorkModeChangeSeq` (강제 Unload) | `WORKSTATE_CHANGE("_CAA")` | 보류 (§5) |
| `DlgNaviMaint.cpp:2922` | `RecipeOpen_Tact` | 리스트에 화질 항목 있으면 헤드 화질 레시피 Open | HMI 해당 없음 |
| `JOB_Proc.cpp:6219,9935` | `_A`/`_EFA` 내부 | T1 진입 시 EFA0 핸드셰이크(§1.3) | Pass 경계 훅 |

### 1.3 `DoWorkStartMeasICP_EFA()` 구조 (강화검사 `_A`와의 차이가 곧 이식 대상)

```
_A  (강화검사)                          _EFA (EFA 검사)
for iTop(스틱)                          nRepeat = max(#특성항목, #화질항목)   ← 동일
  for AgingCnt(항목)                    for AgingCnt(항목=Pass, 0=T0 1=T1)   ← 루프 순서 반전
    contact/사전검사(!bInspCheck)          bInspCheck = (AgingCnt==1)
    [AGING] [점등] [특성] [화질] [색]      ┌ AgingCnt==1 && ONLINE_REMOTE:           ← T0/T1 경계
    ...                                   │  ExportAsCellStatus → sz_MeasEndTime
  bInspCheck = TRUE (스틱 끝)             │  → DoWorkSendMargeFile → sz_ResultEndTime
                                          │  → DoWorkStartSave(결과파일)
                                          │  → SendThreadAPDFileToFTP(APD 업로드) → 500ms
                                          │  → Cmd_Send_EFA0 ("EFA^0$")
                                          └  → isEFA0RecvCheck() : EFA^0_OK$ 를 100ms×50=5s 폴링, 실패해도 로그만 남기고 진행
                                          for iTop(스틱)
                                            Z→ProbeSafetyPos, SetMeasInfo, contact (매 Pass — "한번만" 코드는 주석)
                                            T1이면 skip: ReviewSearch(:10215), 사전검사 ExeSubMeas 0/1/2/10(:10400~10774), bUse[17]/[18](:10447~10587)
                                            [AGING ExeSubMeas(3) + AgingAlarm]  if m_bUseAging          (Col=1)
                                            [점등 TURNON_ALL / NEXT_PTN×(PTN-1) / PtnSec 유지 / TurnOff]  (Col=2,3)
                                            [특성 if m_bUseProperty → 행 = m_iAreaNum+1+nMaxArea*nPropertyRepeat] (Col=4)
                                            [화질 if m_bUseQuality] (Col=5)   [색이상 bUse[4]]
                                            Pass 마지막(AgingCnt==N-1)일 때만 Z Uncontact + MCC step
```

**핵심 의미 2가지**
1. **T1 결과는 T0 행을 덮어쓴다.** `nPropertyRepeat`가 스틱 루프 안에서 선언·리셋되고 Pass가 바깥이라 Pass마다 0 → 같은 `nAreaNum`. 그래서 T1 시작 전에 T0를 **파일+APD로 먼저 밀어내고** `EFA^0$`로 알린다. 글라스 종료 시 통상 흐름으로 T1 결과가 나가고 `EFA^1$`가 붙는다. HMI 결과 구조도 last-write-wins(`iMeasArea` 단일 키)라 **같은 전략이 그대로 성립**한다.
2. **Pass 항목의 `m_bUseAging`은 "T0 검사 후 에이징 수행"의 스위치**다(ExeSubMeas(3) = PG AGING). 통상 EFA rcp는 ITEM_1(T0): Aging=1/특성·화질 체크, ITEM_2(T1): Aging=0/특성·화질 체크로 쓰인다. 리스트가 2행 고정인 이유.

### 1.4 CIM 전문 (EFA 트리 `Include/DefinesCIM.h:132-138`, `DlgNaviMaintCIM.cpp`)

| 전문 | 방향 | 시점 | 구현 |
|---|---|---|---|
| `EFA^0$` | EQ→HOST | T0 완료·APD 전송 후 | `Cmd_Send_EFA0()` (:3022) |
| `EFA^0_OK$` | HOST→EQ | 위 응답 | 수신 파서 `strRcv.Left(3)=="EFA"` && `Mid(3,1)=='0'` → `m_bRecvEFA=TRUE` (:1401-1421). `isEFA0RecvCheck()` 5초 대기 |
| `EFA^1$` | EQ→HOST | T1 완료(글라스 측정 종료) | `Cmd_Send_EFA1()` (:3027), 응답 `EFA^1_OK$`는 로그만 |
| `WORKSTATE_CHANGE^<s>$` | EQ→HOST | EFA1 후 / 강제 Unload | **원본 버그**: `strMsg.Format(_T("WORKSTATE_CHANGE^%s$"))` 인자 누락(:3035) → 가변인자 미정의 동작. 그대로 이식 금지 |
| PPID 2번째 필드 | HOST→EQ | 레시피 지정 | 4자리 WorkState(`m_chrWorkState`), `[1]=='A'` 강화검사 / `'B'` EFA (:647,675,704) |

---

## 2. HMI 현황 (2026-09-28 확인)

| 구성요소 | HMI 현재 | 판정 |
|---|---|---|
| `CDlgSubManual_AgingTest` (h 76줄/cpp 912줄) | AgingTest P1~P4 완료: ctor `CAuroraMgr*` 주입, `CMySpreadDll` 2개(`FPS_RESOURCE_ID+20/21`), `InitMySpread_AGINGTEST_LIST/MEAS`, `SSM_DBLCLK` 라우팅, START→`WM_MSG_UPDATE_CYCLE_INSPECTION_START`+`EngineStart`, OnTimer 하이라이트 | **복제 기반** |
| 호스트 `CDlgSubManual` | 서브 7개를 `new`→`Create`→`MoveWindow(rtROI)`→`SW_HIDE`, `RefreshFileView` switch, 버튼 `ON_BN_CLICKED` | 8번째 슬롯 추가 |
| `enumManualTemID_*` (`WindowEventHandler.h:129-135`) | MEAS/SACC/IONIZER/AUTOCONTACT/AGINGTEST/AGINGSET/WADMULTI | `_AGINGNEW` 추가 |
| 호스트 버튼 배치 (`ALPHA.rc` IDD_DLG_SUB_MANUAL) | y=10/49/88/127/168/207 사용, 246 비어 있음, 425 Multi Meas | **y=246** |
| `VTM_SYSTEM` (`Include/ShareMem.h`) | `m_bAutoAgingMode:1918`, `m_nAgingListRow/Col:1919-1920`, `nJobCount:1761`, 구조체 끝 `:1926` | `m_bNewAgingMode` **끝에** 추가 |
| `CShareMemoryMgr` | `Set/GetSys_AutoAgingMode` (:230-241) | `Set/GetSys_NewAgingMode` 추가 |
| 레시피 `S_InspectItem` (`preDefine.h:110-`) | `m_bUseAging_Test`(강화검사 사이클 게이트, 행 8) + **`m_bUseEFA`(2026-07-14 추가, InRecipe/OutRecipe/RecipeMgrDlg 행 6-2 저장·로드만 있고 소비처 0건)** | **`m_bUseEFA`를 EFA 게이트로 사용** — 레시피 포맷 변경 불필요 |
| `CAgingCycleInfo` (`DoWork/AgingCycleInfo.h`) | `Load(rcpName)` = `GetAgingRcpPath()`+rcp 고정, 항목/[MASECELL]/`GetKindUseCount` | 경로 인자 추가 |
| `CMeasureSequenceItemEngine` | `AgingCycleBegin`(IsOCP + `m_bUseAging_Test` + 파일) / `NextItem` / `End`, `m_nAgingRepeat[3]` | EFA 분기: 항목 = 현재 Pass 고정 |
| `CDoWorkGlassMeasure::ThreadRun` (:135-325) | `for 스틱 { AgingCycleBegin; while 스텝 { VISION 뒤 NextItem→GoToStep(Aging) }; AgingCycleEnd }` | **Pass 루프로 감싼다** |
| 측정 스텝 체인 | None→MoveMeasureArea→**PtrnSearch**→Probe_Contact→OST→RTEST→AGING→AGING_PTN→WAD→ANGLE→VISION→Idle | T1 skip 대상 = PtrnSearch, OST, RTEST (OCP의 ReviewSearch/사전검사에 대응) |
| 결과 저장 | `CDoWorkGlassResultStep_CompleteCheck` → `CResultMgr::SaveResult(bSaveFile, bRemote, start)` → dResult DLL `API_Result_SaveResult` | Pass 경계에서 동일 호출 (재호출 가능 여부 §6 검증) |
| CIM (`CommTcpIpObjectCIM`) | `Cmd_Send(str)` + STS/AT0/PPID 계열, 수신 `strRcv.Find(...)` 체인(:218-466), PPID 핸들러 `strAging=='J'`→`SetSys_AutoAgingMode(true)`(:1402) | EFA 4전문 추가 |
| `DefinesCIM.h` | `dfAT_0/1`, EFA 정의 없음 | `dfEFA_T0/T1/_OK` 추가 |
| 리소스 ID | IDD 12041~12050 사용(다음 **12051**), `_APS_NEXT_CONTROL_VALUE 1052`, 스프레드 `FPS_RESOURCE_ID+19~22,24~27` 사용 | IDD 12051, IDC 1052(→NEXT 1053), 스프레드 +28/+29 |

---

## 3. 설계 결정

### 3.1 별도 다이얼로그 + 별도 모드 플래그 (OCP 구조 그대로)
AgingTest 다이얼로그에 "EFA 모드" 체크를 넣는 대안도 있으나, OCP가 화면·플래그·rcp 폴더를 분리했고 두 검사는 **상호 배타**로 운용된다. 운영자 혼동과 CIM 분기(A/B)를 그대로 매핑하려면 분리가 맞다. 공통 코드는 복제하되, 향후 공통화는 `CDlgSubManual_AgingTest`를 베이스 클래스화하는 별건으로 남긴다(지금은 OCP 원본도 복제본).

### 3.2 rcp 폴더 = `D:\HBS\PARAM_OCP\PARAM\EFARcp\{RecipeID}.rcp`, 레지스트리 키 **`DIR\EFA_RCP`**
원본은 두 다이얼로그가 같은 키 `DIR\AGING_RCP`를 읽어 기본값만 다르다 → 키가 존재하는 실기에서는 **두 검사가 같은 파일을 덮어쓴다**. HMI는 별도 키로 분리한다. 파일 포맷은 AgingTest와 100% 동일하게 유지해 `CAgingCycleInfo` 파서를 그대로 쓴다(`USE_ANGLE` 키·3비트 `[MASECELL]` 포함 — OCP EFA 파일과의 상호 호환은 어차피 LCR/색이상 차이로 불가, AgingTest P2 결정과 동일).

### 3.3 그리드는 AgingTest와 동일 7열 / 3시트, 행만 2개 고정
OCP EFA는 6열(LCR 없음)·3시트(특성/화질/색이상)지만, HMI 측정 종류는 WAD/VISION/ANGLE 3종으로 이미 확정(2026-08-06)돼 있고 엔진 게이트가 그 비트를 읽는다. UI는 `InitMySpread_AGINGTEST_LIST/MEAS` 재사용, `AgingItemClearAll(TRUE)`에서 `SetMaxRows(2)`+행 타입 세팅, ITEM 추가/제거 버튼은 `.rc`에서 `NOT WS_VISIBLE | WS_DISABLED`.

### 3.4 시퀀스 = "Pass 루프 in `CDoWorkGlassMeasure::ThreadRun`" (OCP `_EFA` 루프 순서 재현)

```
// ThreadRun 개요 (EFA 활성 시)
nPass = EFA 활성 ? EFA rcp 항목 수(=2) : 1
for pass in 0..nPass-1
    if pass >= 1:  EfaPassBoundary()          // §3.5
    for stick in nStartStick..nStepIndex-1
        GoToInitStep()
        bAgingCycle = engine->AgingCycleBegin(pass)   // EFA: m_nAgingCurItem = pass 고정, NextItem은 항상 false(루프백 없음)
        while 스텝:
            EFA && pass>=1 && 스텝 ∈ {PtrnSearch, OST, RTEST} → 스텝 skip (GoToNextStep)   // OCP bInspCheck
            ... 기존 그대로 (AGING/AGING_PTN/WAD/ANGLE/VISION 게이트가 항목 플래그×셀맵을 이미 본다)
        engine->AgingCycleEnd()
    if bExis: break
```

- **왜 여기인가**: 글라스 시퀀스 아이템(Adsorption→Recipe→Align→PreMeas→AGING→**Measure**→Result→Unload)에서 T1이 다시 도는 범위는 OCP 기준 "contact + 측정"이고 사전검사(PreMeas 아이템)는 한 번뿐이다. Measure 아이템 내부 Pass 루프가 정확히 그 범위다. 결과(Result 아이템)는 T1 뒤에 통상대로 한 번 돈다.
- **엔진 변경 최소화**: `AgingCycleBegin(int nFixedItem = -1)` 오버로드 — `-1`이면 기존 강화검사 동작, `>=0`이면 게이트를 `IsOCP && (m_bNewAgingMode || recipe.m_bUseEFA)`로 바꾸고 `m_nAgingCurItem = nFixedItem`, `m_bAgingCycleActive = item.m_bUseAging || 측정항목 있음`. `AgingCycleNextItem()`은 EFA일 때 항상 false → `ThreadRun`의 GoToStep 루프백이 자연히 죽는다.
- 진행표시 `m_nAgingListRow = pass+1`은 기존 스텝 마커(`GetAgingCurItem()+1`)가 그대로 만족한다.
- 결과 행: EFA에서는 `GetAgingRepeat(kind)`가 스틱마다 0(Begin에서 리셋) → **T1이 T0 행을 덮어씀 = OCP와 동일 의미**(§1.3-1). `GetAgingKindMax`(=항목 수 2)만 MultiPoint 블록 오프셋에 영향 → §6에서 확인. `ClearInnerIDForAging`이 Pass마다 새 InnerID를 만들어 raw 파일은 보존된다.
- `m_bNewAgingMode`와 `m_bAutoAgingMode`는 동시에 TRUE가 될 수 없게 START/PPID/사이클 시작에서 서로 리셋.

### 3.5 Pass 경계 훅 `EfaPassBoundary()` (OCP `_EFA:9935-9962` 대응)

| OCP 호출 | HMI 대응 | 비고 |
|---|---|---|
| `ExportAsCellStatus()` | 셀 상태 UI는 스텝이 갱신 중 | 생략 |
| `sz_MeasEndTime`/`sz_ResultEndTime` 기록 | `SeqTactTimeMgr` 기존 값 | 생략 |
| `DoWorkSendMargeFile()`, `DoWorkStartSave()`, `SendThreadAPDFileToFTP()` | **`CResultMgr::GetInstance()->SaveResult(true, true, start)`** — `CDoWorkGlassResultStep_CompleteCheck::Run()`과 동일 호출(APD 생성+원격 전송을 dResult가 수행) | **재호출 안전성 검증 필요**(§6-3). 불가하면 `SaveResult(true,false)` + CIM FTP 별도 |
| 500ms, `Cmd_Send_EFA0()` | `GetCommTcpIpObjectCIM()->Cmd_Send_EFA0()` | ONLINE(REMOTE) 상태에서만 — Offline 수동 START 시 생략(OCP 동일) |
| `isEFA0RecvCheck()` (5초, 실패 무시) | `IsEFA0RecvCheck()` 동일 정책 + WARN 로그 | 응답 없을 때 정지 여부는 고객 사양 확인(§5) |
| — | `IfPauseEvt_WaitResumeEvt()`로 사용자 STOP 반영 | HMI 추가 |

### 3.6 CIM 이식 범위
- `DefinesCIM.h`: `dfEFA_T0 _T("EFA^0$")`, `dfEFA_T1`, `dfEFA_T0_OK`, `dfEFA_T1_OK` (HMI는 `_T()` 유니코드 리터럴).
- `CCommTcpIpObjectCIM`: `Cmd_Send_EFA0/EFA1()`, `m_bRecvEFA`, `IsEFA0RecvCheck()`, 수신 체인에 `strRcv.Left(3)==_T("EFA")` 분기(`Mid(3,1)`이 `'0'`이면 플래그, `'1'`이면 로그). `Find`가 아니라 `Left(3)`인 이유는 다른 전문(`EFA…`)과 접두 충돌 방지 — OCP와 동일.
- PPID 핸들러: 2번째 필드가 4자리 WorkState면 `[1]` 문자, 1자리면 그 문자로 판정. `'B'` → `CAgingCycleInfo::Load(EFA 경로)` 성공 시 `SetSys_NewAgingMode(true)`, 실패 시 `JOB_RECIPE_NOT_EXIST`. **HMI는 현재 강화검사를 `'J'`로 판정**하므로 `'A'/'B'` 도입 전에 고객 CIM 사양서와 대조(§5).
- 글라스 종료: `DoWorkSequenceCIMStep.cpp:772` `Cmd_Send_STS1106()` 직전에 `if NewAgingMode: Cmd_Send_EFA1(); Sleep(500);`. `WORKSTATE_CHANGE`는 보류.

---

## 4. 실행 Phase

### Phase 0 — 결정사항 확인 (코드 변경 0)
§5 D-1~D-4 답을 받는다. 미확정이어도 Phase 1~2는 진행 가능(UI/엔진은 CIM과 독립).

### Phase 1 — UI 셸 (다이얼로그 복제 + 호스트 삽입) ✅ 구현·빌드 완료, 화면 사용 확인 (2026-09-28)

**실제 적용 내용(플랜과 다른 점 굵게)**
- 신규 `DlgSubManual_AgingNew.h/.cpp`(ASCII, 영어 메시지). 리스트 행 헤더는 `T0`/`T1`로 표기. START는 `m_bUseEFA` + EFA rcp 존재 + `[COUNT]`≥1 검사, 항목 수≠2면 확인창. START/STOP이 `SetSys_NewAgingMode(true/false)`를 쓰고 `SetSys_AutoAgingMode(false)`로 상호 배타. **Phase 2 전까지는 플래그만 켜지고 시퀀스는 표준으로 돈다.**
- `resource.h`: `IDD_DLG_SUB_MANUAL_AGING_NEW 12051`, **`IDC_BUTTON_MANUAL_AGING_NEW 1078`**(1052는 다른 다이얼로그 3곳이 이미 사용 — 전 파일 미사용 번호 중 최소값 선택, `_APS_NEXT_CONTROL_VALUE` 미변경).
- `ALPHA.rc`: DIALOGEX·DESIGNINFO·AFX_DIALOG_LAYOUT 3블록 복제, 타이틀 "EFA Test View", `IDC_BUTTON_4/5` 숨김, 호스트 버튼 `"EFA 검사"` y=246.
- `DlgSubManual.h/.cpp`: 8번째 서브(생성·소멸·HIDE/SHOW·버튼 표시·핸들러), `WindowEventHandler.h` `enumManualTemID_AGINGNEW`, vcxproj/filters 2+2줄.
- **공유메모리 필드를 Phase 1에 앞당김**: 다이얼로그가 컴파일되려면 필요. **실제 컴파일되는 `VTM_SYSTEM`은 `CoreFunction/ShareMemFunction/ShareMem.h`(2,396줄)다** — `ShareMemoryMgr.h`→`ShareMemFunction\ShareMemFunction.h`→같은 폴더 `ShareMem.h`. `Source/Include/ShareMem.h`(2,000줄)는 어떤 TU에도 include되지 않는 **사장 사본**(내용도 이미 갈림)이라 건드리지 않았다. `m_bNewAgingMode`는 `m_nAgingStartStick[5]` 뒤 구조체 끝에 추가. `ShareMemFunction.cpp`는 ALPHA.exe에 직접 컴파일되므로 `sizeof(VTM_SYSTEM)` 불일치 없음(다른 DLL은 `VTM_MCC` 등 별도 매핑만 생성).
- 빌드: Debug x64 전체 성공, 신규 파일 경고 0. 백업 `*.bak_efa` 11개.
- **남은 확인**: ALPHA 기동 → MANUAL 탭 "EFA 검사" 버튼 → 화면 표시 → 리스트 초기화(2행) → 저장 → `D:\HBS\PARAM_OCP\PARAM\EFARcp\{Recipe}.rcp` 생성 → 이전값 불러오기.

원래 계획(참고):
1. **`resource.h`**(UTF-8 BOM, raw bytes): `IDD_DLG_SUB_MANUAL_AGING_NEW 12051`, `IDC_BUTTON_MANUAL_AGING_NEW 1052`, `_APS_NEXT_CONTROL_VALUE 1053`. 적용 전 `grep -n " 12051$\| 1052$"`로 충돌 재확인.
2. **`ALPHA.rc`**(UTF-16 LE, python `io.open(encoding='utf-16', newline='')`): `IDD_DLG_SUB_MANUAL_AGING_TEST` 블록 복제 → ID 교체, 타이틀 `"EFA Test View"`, `IDC_BUTTON_4/5`에 `NOT WS_VISIBLE | WS_DISABLED`, `IDC_STATIC` "Cycle Count" 유지. 호스트 `IDD_DLG_SUB_MANUAL`에 `PUSHBUTTON "EFA 검사"…IDC_BUTTON_MANUAL_AGING_NEW,654,246,50,32,BS_MULTILINE` (한글은 UTF-16이라 안전).
3. **`DlgSubManual_AgingNew.h/.cpp` 신규** = AgingTest 복제 후 치환: 클래스명, IDD, `FPS_RESOURCE_ID+28/+29`, `DIR\EFA_RCP` 기본 `D:\HBS\PARAM_OCP\PARAM\EFARcp\`, Cycle 콤보 1~25, 패턴 콤보 `0..25`, `AgingItemClearAll(BOOL bFixed2=FALSE)`(TRUE면 2행 생성), `OnItemAdd/Del` 즉시 return, 모드 플래그 전부 `m_bNewAgingMode`(START/STOP/OnTimer), START 사전검사는 `m_bUseAging_Test` 대신 **`m_bUseEFA`**, `CAgingCycleInfo::Load`에 EFA 경로 전달. 원본 파일이 BOM 없는 UTF-8이므로 **새 파일은 ASCII+영어 메시지로 작성**(CLAUDE.md 규칙 7).
4. **`DlgSubManual.h/.cpp`**: 전방선언·멤버 `m_pDlgSub_AgingNew`, `#include`, 소멸자 delete, `OnInitDialog` 8번 블록, `RefreshFileView` HIDE/SHOW case, `ON_BN_CLICKED` + 핸들러. `WindowEventHandler.h` `enumManualTemID_AGINGNEW`(WADMULTI 뒤).
5. **`ALPHA.vcxproj` / `.filters`**: AgingTest 항목 옆에 2줄씩.
6. **`MySpreadDll`**: 변경 없음(`InitMySpread_AGINGTEST_*` 재사용). 필요 시 `InitMySpread_EFA_LIST`는 헤더 문구만 다른 래퍼.
7. 빌드(루트 CLAUDE.md의 Community MSBuild 명령) → MANUAL 탭에서 버튼·화면·Load/Save(`EFARcp\{Recipe}.rcp` 생성, 2행) 확인.

### Phase 2 — 공유 상태 + 엔진 Pass 루프 ✅ 구현·빌드 완료, OOS 검증 완료 (2026-09-28)

**실제 적용 내용**
- `AgingCycleInfo.h/.cpp`(**LF-only 파일**): `GetEfaRcpPath()`(`DIR\EFA_RCP`), `Load(rcp, bEfa=false)`.
- `MeasureSequenceItemEngine`: `AgingCycleBegin(int nEfaPass = -1)` — `>=0`이면 EFA rcp를 로드하고 **항목 = nEfaPass 고정**, `m_bEfaCycle=true`; `AgingCycleNextItem()`은 EFA면 항상 false(루프백 없음); `GetEfaPassCount()` = 게이트(`IsOCP && (GetSys_NewAgingMode() || recipe.m_bUseEFA)`) 통과 시 EFA rcp 항목 수, 아니면 0; `IsEfaCycle()`; `AgingCycleEnd()`에서 리셋.
- `CDoWorkGlassMeasure::ThreadRun`: 스틱 루프를 **`for (nPass...)`로 감쌈**(기존 본문 1탭 재들여쓰기 — `git diff -w`로 볼 것). Pass≥1 진입 시 `EfaPassBoundary(nPass)`(Phase 2는 로그+사용자 STOP 확인만), 스틱 시작은 Pass 0만 Resume 인덱스 적용, T1+에서 `IsEfaSkipStep()` = `Pattern Search`/`OSTest`/`R-Test`를 `GoToNextStep()`으로 건너뜀, `bExis`면 남은 Pass 중단.
- `CDoWorkGlassMeasureStep_AGING::Run`: **EFA 사이클이고 현재 항목 `USE_AGING=0`이면 에이징 skip**(OCP `bUseAging = sAgingTest.m_bUseAging`). 강화검사 경로는 `IsEfaCycle()` 조건으로 완전히 분리.
- 상호 배타: CIM PPID 핸들러의 `SetSys_AutoAgingMode(false)` 옆에 `SetSys_NewAgingMode(false)`, AgingTest START에서 `SetSys_NewAgingMode(false)`.
- 빌드 Debug x64 오류 0. 백업 `*.bak_efa2` 9개. 패치 스크립트 `scratchpad/efa_p2_patch_v2.pl`(파일별 EOL 자동 판별 — **grep `\r$` 기반 EOL 판정은 MSYS에서 틀린다, Perl로 셀 것**).
- **결과 행 의미**: Pass마다 `AgingCycleBegin`이 `m_nAgingRepeat`를 리셋하므로 T1이 T0와 같은 행에 기록 = OCP와 동일. `GetAgingKindMax`는 `GetKindUseCount`(USE_AGING 항목만 셈)라 T1(Aging=0)은 제외 → 통상 1.
- **OOS 검증 결과(2026-09-28, 4회 런)**: ① 15:23 `MOTION_INTERLOCK_11`(ProbeCheck SACC 게이트, OOS 비트 0) → 모션 OOS 우회 6곳 ② 15:32 SW LIMIT(측정 스텝 AGING 리뷰 스냅이 Y3/Y4=FRONT를 720으로, 리밋 697) → OCP 분기 Y3/Y4 `df_HEAD_Y_IDLE_SAFETY`+카메라 0/1 ③ 15:58 `MOTION_INTERLOCK_12`(T0 마지막 스틱이 Idle 종료 스텝을 타 RESULT/UNLOAD가 T1과 동시 진행) → Last Stick 조건에 `nPass == nPassCount-1` ④ **16:12 런 알람 0, 순서 정상**: T0 5스틱(AGING Snap→AGING→PTN 5s→WAD/ANGLE/VISION) → `pass boundary` → T1 5스틱(PtrnSearch/OST/RTEST skip, AGING skip, PTN 0) → Idle → 7_RESULT → 8_UNLOAD End. `6_MEASURE - End`가 T1 뒤(16:13:40)에만 나옴. **단 WAD 실측 0건** — 16:11에 다이얼로그로 재저장된 EFA rcp의 `[MASECELL]`이 전부 0(21×5 픽스처가 레시피 5×20과 안 맞아 MeasCellLoad가 행을 건너뛰고 빈 그리드를 저장; STICK_6~21 잔존 키는 WritePrivateProfileString이 안 지움) → `IsStickUse` false. 픽스처를 5×20으로 재작성(`.bak_zeromap_1611`). **T0/T1 결과 행 덮어쓰기 확인은 다음 런.** 개선 후보: `MeasCellSave`가 스틱 수 밖 `STICK_n` 키를 지우도록(OCP도 미처리).
- **✅ Phase 2 OOS 검증 완료(2026-09-28 16:19 런, Operate 사이클 경로, flag 1/recipe 1)**: 알람 0, WARN 0. 5×20 픽스처(스틱 1~3 WAD)로 T0 WAD 30행 실측 → T1 WAD 30행 실측, **행 번호가 T0/T1 완전히 동일**(stick0 row 11~20/1~10, stick1 31~40/21~30, stick2 51~60/41~50) = T1이 T0를 덮어쓰는 OCP 동작 재현. WAD InnerID T0 61개 / T1 60개 전부 고유, **양 Pass 간 공유 0** → raw 파일 보존. 스틱 4~5는 셀맵 0이라 WAD skip. `6_MEASURE - End`(16:22:51)는 T1 뒤에만. 결과 그리드·APD 덮어쓰기 자체는 **OOS에서 검증 불가** — OOS는 WAD 응답이 없어 APD RAW_DATA가 원래 0행(09-21 이후 APD 전부 0행). 실기 또는 응답 시뮬레이터 필요.
- **OOS 검증 절차**: ① RECIPE 검사항목 6행 2열 "EFA" ON ② `EFARcp\{Recipe}.rcp` 2항목(T0 Aging=1/PTN/SEC/WAD, T1 Aging=0/WAD) ③ MANUAL "EFA 검사" START(또는 Operate 사이클) ④ 로그 `EFA: mode on … passes 2` → `EFA: pass 1/2 start (T0)` → Stick1..N `Aging`/`AgingPtn`/`WAD` → `EFA: pass boundary before pass 2` → `EFA: pass 2/2 start (T1)` → Stick1..N에 `EFA pass 2: skip step Pattern Search/OSTest/R-Test`, `AGING End. (EFA item USE_AGING=0 - skip)`, WAD 실측 → 결과 시트에 T1 값만 잔존 ⑤ 표준 레시피(EFA OFF, 플래그 OFF)에서 `EFA:` 로그 0줄(회귀 없음).

원래 계획(참고):
1. `Include/ShareMem.h` `VTM_SYSTEM` **끝**(`:1926` 직전)에 `BOOL m_bNewAgingMode;` — DLL/FDC/MCC 공유 구조체 규칙. `ShareMemoryMgr` `Set/GetSys_NewAgingMode`.
2. `AgingCycleInfo.h/.cpp`: `static CString GetEfaRcpPath()`(`DIR\EFA_RCP`), `bool Load(CString rcp, bool bEFA=false)`.
3. `MeasureSequenceItemEngine`: `AgingCycleBegin(int nFixedItem=-1)`, `IsEfaMode()`, `GetAgingItemCount()`; `AgingCycleNextItem`은 EFA면 false. 게이트: `IsOCP && (GetSys_NewAgingMode() || recipe.m_bUseEFA)` — 다이얼로그 START는 플래그를, CIM 'B'도 플래그를, 레시피만 켠 Auto 런은 `m_bUseEFA`를 본다(강화검사 `m_bUseAging_Test`와 동일 3경로).
4. `DoWorkGlassMeasure::ThreadRun`: §3.4 Pass 루프 + T1 skip 집합 `{"Pattern Search","OSTest","R-Test"}`(이름 비교는 기존 `GetName()=="Aging"` 관례). 스텝 skip 시 `SetSeqItemTimeST/ED` 호출 쌍 유지.
5. 사이클 시작(`DlgSubOperate` 사이클 버튼 경로·CIM PPID)에서 `SetSys_NewAgingMode(false)` 리셋 — `SetSys_AutoAgingMode(false)` 옆.
6. OOS 런: 2행 rcp(T0: Aging=1+WAD, T1: Aging=0+WAD) → 로그로 `Stick1..N Aging/AgingPtn/WAD` 후 다시 `Stick1..N Contact/WAD`(PtrnSearch 없음) 순서, `m_nAgingListRow` 1→2 하이라이트, 결과 행 T1이 T0 덮어씀 확인.

### Phase 3 — 결과 플러시 + CIM 핸드셰이크 ✅ 구현·빌드 완료, OOS 오프라인 검증 완료 (2026-09-28) / CIM·실기 검증은 추후 (§9)

**§6-1 검증 결과(dResult 소스)**: `API_Result_SaveResult` → `CDlgResult::SaveResult`는 `UploadDataDone`을 최대 5초 기다린 뒤 `AfxBeginThread(TotalResultData)`로 **비동기** 내보내기(XLS `RESULT\DATA\{day}\{glass}\{glass}_{HHMMSS}.xls` + `RESULT\APD\APD_{time}.txt` + `RESULT\APD.txt`)를 시작하고 즉시 반환한다. **그리드를 지우지 않는다**(`DataClear`는 별도 API). `bRemote`는 받기만 하고 안 쓴다(FTP는 CIM `SendAPDFileToFTP`). → 이중 호출 안전. 단 스레드가 그리드를 읽는 중 T1이 같은 행을 쓰면 섞이므로 **`SaveResultDone()`이 false→true 되는 것을 기다린 뒤** T1로 넘어간다.

**실제 적용 내용**
- `DefinesCIM.h`: `dfEFA_T0/T1/T0_OK/T1_OK` (`_T()`).
- `CCommTcpIpObjectCIM`: `m_bRecvEFA0`, `Cmd_Send_EFA0()`(플래그 리셋 후 송신), `Cmd_Send_EFA1()`, `WaitEFA0Ack(5000)`(100ms 폴링, OK/TIMEOUT 로그). 수신 체인 **맨 앞**에 `Left(3)=="EFA"` 분기 — `^` 다음 숫자로 0/1 판정(OCP는 `Mid(3,1)`이라 항상 `'^'`→0으로 읽는 버그). `Cmd_Send`는 CIM OOS면 성공 반환, OFFLINE이면 `CIM_COMMAND_ERROR`.
- `Process_PPID`: 2번째 필드 4자리면 `[1]`, 1자리면 `[0]`. **`'A'`→`SetSys_AutoAgingMode(true)`, `'B'`→EFA rcp 검사 후 `SetSys_NewAgingMode(true)`+`nJobCount=1`, rcp 없으면 `nRet=JOB_RECIPE_NOT_EXIST`(→ 호출부가 PPID NG 송신)**. 기존 `'J'` 분기 유지. 참고: 기존 코드는 `OpenRecipe` 실패·`'J'` 실패를 `nRtn`에만 넣고 `nRet`을 반환해 PPID NG가 안 나가는 기존 결함이 있음(미수정).
- `CDoWorkGlassMeasure::EfaPassBoundary`: ① `SaveResult(true,true,GetStartTime2())` + `SaveResultDone` 대기(시작 1s/완료 30s) ② `GetSys_MachineMode()==df_HOST_ONLINE_REMOTE`일 때만 `SendAPDFileToFTP()` + 전송 스레드 종료 대기(GDTReq와 동일, 최대 100s) → 500ms → `Cmd_Send_EFA0()` → `WaitEFA0Ack(5000)`, 타임아웃이어도 진행(D-2). 저장·CIM 실패는 WARN 후 진행, 사용자 STOP만 false.
- `CDoWorkSequenceCIMStep_MeasDone`: STS1106 직전에 EFA 글라스면 `Cmd_Send_EFA1()` + 500ms. 판정은 `GetEfaPassCount`와 같은 게이트(OCP + (플래그 or 레시피 m_bUseEFA) + EFA rcp 항목≥1)를 복제. `WORKSTATE_CHANGE`는 보류(D-3).
- 백업 `*.bak_efa3` 5개. 패치 `scratchpad/efa_p3_patch.pl`.
- **검증 포인트(OOS Offline)**: `EFA: T0 result flush OK` → `EFA: not ONLINE REMOTE - skip APD upload / EFA^0$` → T1. `RESULT\APD\`에 경계 시각 APD 1개 + 글라스 종료 APD 1개(T1). **Online**: CIM 로그 `SEND … EFA^0$` → `[CIM] EFA^0_OK received`/`TIMEOUT - continue`, 글라스 끝 `SEND … EFA^1$` → `STS1106`; PPID `^xBxx$` → `[CIM] PPID WorkState 'B' - EFA mode`.

원래 계획(참고):
1. `DefinesCIM.h` 4개 define, `CommTcpIpObjectCIM` 송신 2·수신 1·대기 1·플래그 1.
2. `EfaPassBoundary()`(Measure 아이템 또는 엔진): `SaveResult(true,true,start)` → 500ms → `Cmd_Send_EFA0` → `IsEFA0RecvCheck` (Offline이면 전부 생략, 로그 "EFA pass boundary (offline: skip CIM)").
3. `DoWorkSequenceCIMStep.cpp:772` 앞 `Cmd_Send_EFA1` + 500ms (NewAgingMode일 때만).
4. PPID `'B'` 진입(§3.6) — D-1 확정 후.
5. OOS + CIM 시뮬레이터(또는 로그 판독)로 순서 검증: `…WAD(T0 마지막 스틱) → SaveResult → SEND EFA^0$ → [RECV EFA^0_OK$] → Stick1 Contact(T1)… → STS1106 직전 SEND EFA^1$`.

### Phase 4 — 마무리
- `HMI/CLAUDE.md` OCP 핵심 차이 표에 "EFA 검사(2-Pass)" 1행, 스킬 `hmi-comm-pg`/`hmi-logs`에 EFA 로그 판독 포인트, 메모리 `project_aging_porting` 갱신.
- **커밋** — `HMI/`는 git이라 미커밋 파일은 다음 동기화에서 사라진다(이 문서 포함).

---

## 5. 결정 필요 (사용자/고객)

| # | 질문 | 기본 가정(미답 시) |
|---|---|---|
| D-1 | CIM PPID 2번째 필드: HMI 현행 `'J'`(강화검사)와 EFA 트리 `'A'/'B'`(4자리 WorkState의 `[1]`) 중 이 설비 사양은? | **✅ 확정(2026-09-28 사용자): EFA 트리 방식 `'A'`(강화검사)/`'B'`(EFA)** — Phase 3에서 PPID 판정을 `'A'/'B'`로 구현, 기존 `'J'` 분기는 호환용으로 유지 |
| D-2 | `EFA^0_OK$` 5초 무응답 시 진행(OCP) vs 알람 정지 | **✅ 확정: OCP 방식**(진행 + WARN 로그) |
| D-3 | `WORKSTATE_CHANGE^_CAA$`(EFA1 후·강제 Unload) 필요 여부 — 원본 구현이 인자 누락 버그라 실제로 무엇이 나갔는지 불명 | **보류**(미이식) |
| D-4 | T1에서 skip할 스텝 — PtrnSearch/OST/RTEST 3개로 충분한가(OCP는 ReviewSearch + 사전검사 ExeSubMeas 0/1/2/10 + bUse[17]/[18]) | **✅ 확정: 3개로 충분** |
| D-5 | 재기동 시 EFA 모드 복원(OCP `GlassExist.ini EFAMode`) 필요 여부 | 강화검사와 동일하게 미영속 |
| D-6 | 그리드 열: OCP EFA 6열(LCR 無)로 맞출지, HMI AgingTest 7열(ANGLE) 유지 | 7열 유지(§3.3) |

---

## 6. 검증 항목 (구현 전/중 반드시)

1. **`SaveResult` 이중 호출**: `API_Result_SaveResult(true,true,…)`를 Measure 아이템 중간과 Result 아이템에서 두 번 불렀을 때 dResult가 APD를 두 번 만들고 원격 전송을 두 번 하는지, 내부 `SaveResultDone`/`ClearData` 상태가 T1 결과 축적을 막지 않는지 — dResult 소스(`CoreFunction/dll_Result`) 확인. 막히면 T0는 `SaveResult(true,false)`(파일만)로 하고 FTP 전송을 CIM 쪽으로 분리.
2. **결과 행 덮어쓰기 의도 확인**: T1 후 `CDlgResult` 시트에 T0 값이 남지 않아야 정상(OCP 동일). MultiPoint 사용 시 `GetAgingKindMax`=2가 블록 오프셋을 바꾸므로 3점 픽스처로 행 충돌 0 확인(AgingResult_RepeatSlot §MultiPoint 절차 재사용).
3. **InnerID**: Pass마다 `ClearInnerIDForAging` 경유 → T0/T1 raw 파일명이 달라야 함(`MakeUniqueInnerID` WARN 0).
4. **Pass 경계 Z/Contact**: OCP는 매 Pass·매 스틱 `Z→ProbeSafetyPos`+contact. HMI `MoveMeasureArea`(UnContact)→`Probe_Contact`가 T1에서도 실행되는지(skip 집합에 넣지 않았는지) 로그 확인.
5. **상호 배타**: EFA START 후 AgingTest START(또는 반대) 시 이전 플래그가 FALSE인지.
6. **Offline 수동 START**: CIM 전문·SaveResult 원격 전송 없이 2-Pass만 도는지.
7. 빌드 후 `damaged=0` 검사(신규 파일은 ASCII라 해당 없음, 수정한 CP949 파일만).

---

## 7. 수정 파일 목록 (예정)

| 파일 | 인코딩/편집법 | 변경 |
|---|---|---|
| `Source/ALPHA/DlgSubManual_AgingNew.h/.cpp` | **신규, ASCII** (Write 가능) | 다이얼로그 |
| `Source/ALPHA/DlgSubManual.h/.cpp` | UTF-8 BOM(호스트, 한글 주석 있음) → Perl raw | 8번째 서브 |
| `Source/ALPHA/WindowEventHandler.h` | 확인 후 Perl | enum 1개 |
| `Source/ALPHA/resource.h` | UTF-8 BOM raw | ID 2개 + NEXT |
| `Source/ALPHA/ALPHA.rc` | UTF-16 LE, python | 다이얼로그 1 + 버튼 1 |
| `Source/ALPHA/ALPHA.vcxproj/.filters` | ASCII XML | 4줄 |
| `Source/Include/ShareMem.h` | CP949 Perl | `m_bNewAgingMode` (끝) |
| `Source/ALPHA/Manager/ShareMemoryMgr.h/.cpp` | CP949 Perl | 접근자 2 |
| `Source/ALPHA/DoWork/AgingCycleInfo.h/.cpp` | CP949 Perl | 경로 인자 |
| `Source/ALPHA/DoWork/MeasureSequenceItemEngine.h/.cpp` | CP949 Perl | `AgingCycleBegin(int)` 등 |
| `Source/ALPHA/DoWork/DoWorkGlassMeasure.cpp` | CP949 Perl | Pass 루프 + skip |
| `Source/Include/DefinesCIM.h` | 확인 후 Perl | EFA 4개 |
| `Source/ALPHA/CommTcpIpObjectCIM.h/.cpp` | CP949 Perl | 송수신/대기/PPID |
| `Source/ALPHA/DoWork/DoWorkSequenceCIMStep.cpp` | CP949 Perl | EFA1 송신 |
| `CoreFunction/common/MySpreadDll.*` | 변경 없음 | — |
| `preDefine.h`(2사본), dfcRecipe | **변경 없음** (`m_bUseEFA` 기존) | — |

백업 접미사: `.bak_efa` (편집 직전 `cp`), 주석 태그 `// LYS 2026-mm-dd EFA`.

---

## 8. 리스크

- **dResult 이중 저장**(§6-1)이 유일한 구조적 미지수. 여기서 막히면 Phase 3 설계를 "T0는 파일만 + CIM FTP 직접"으로 바꾼다.
- `VTM_SYSTEM` 필드 추가는 FDCSense/MCCLog와 공유 → 반드시 끝에, 헤더 사본(`Include/` 외 DLL 내부 사본) 동기화 확인.
- OCP `_EFA`는 3,600줄 복제본이라 `_A`와의 미묘한 차이(T1에서 contact를 "한 번만" 하려던 주석 처리 흔적 `//if(!bInspCheck)`)가 남아 있다 — 현재 동작(매 Pass contact)을 기준으로 이식하고 변경 이력을 문서에 남긴다.
- CIM 문자 판정(D-1)을 잘못 잡으면 Online에서 EFA가 아예 안 켜지거나 강화검사로 오진입한다 — Phase 3 PPID 분기는 사양 확인 전 배포 금지.

---

## 9. 검증 현황 (2026-09-28 기준)

### 9.1 OOS 오프라인에서 확인 완료

이 PC(PC-B, `USE_HARDWARE_MOTOR=0` / `USE_HARDWARE_SENSOR=0`, 호스트 OFFLINE)에서 사용자가 직접 돌린 런 기준. 픽스처 = `D:\HBS\PARAM_OCP\PARAM\EFARcp\1_OCP_TEST_Long_260907.rcp`(T0: Aging=1·PTN 1·5초·WAD / T1: Aging=0·WAD, 5스틱×20셀 중 스틱 1~3 WAD ON). 레시피 `1_OCP_TEST_Long_260907`(검사항목 EFA=1).

| 런 | 빌드 | 결과 | 조치 |
|---|---|---|---|
| 15:23 | Phase 2 | `MOTION_INTERLOCK_11` — ProbeCheck SACC 파킹 게이트가 OOS에서 PLC 비트 0으로 차단 | 모션 OOS면 비트 검사 생략(6곳, `.bak_20260928_oos11`) |
| 15:32 | 〃 | SW LIMIT — 측정 스텝 AGING 리뷰 스냅이 Y3/Y4(FRONT, 리밋 +697)를 720으로 이동 | OCP 분기에서 Y1/Y2=리뷰, Y3/Y4=[SAFETY], 스냅 카메라 0/1(`.bak_20260928_agingY`) |
| 15:58 | 〃 | `MOTION_INTERLOCK_12`(3032) — T0 마지막 스틱이 Idle 종료 스텝을 타 RESULT·UNLOAD가 T1과 동시 진행 | Last Stick 조건에 최종 Pass 조건(`.bak_20260928_efapass`) |
| 16:12 | 〃 | 알람 0, 순서 정상. WAD 실측 0건 — 다이얼로그 재저장으로 `[MASECELL]` 전부 0(21×5 픽스처가 5×20 레시피와 불일치) | 픽스처 5×20 재작성 |
| 16:19 | 〃 | **Phase 2 통과**: 알람·WARN 0, T0/T1 WAD 각 30행, 행 번호 동일(T1이 T0 덮어씀), InnerID 두 Pass 공유 0 | — |
| 16:36 | **Phase 3** (16:34:52 빌드) | **Phase 3 오프라인 통과** (아래) | — |

**16:36 런 상세 (Operate 사이클 1회, `flag 0, recipe 1` = 레시피 EFA 플래그만으로 게이트 통과)**

```
16:36:40.617  EFA: mode on (flag 0, recipe 1), passes 2
16:36:40.617  EFA: pass 1/2 start (T0)            → 스틱 1~5 (begin pass 1/2 ×5)
16:38:50.099  EFA: pass 2/2 start (T1)
16:38:50.102  EFA: pass boundary before pass 2 (T0 done) - result flush + CIM handshake
16:38:50.207  EFA: T0 result flush OK             ← SaveResult + SaveResultDone 대기 105 ms
16:38:50.207  EFA: not ONLINE REMOTE - skip APD upload / EFA^0$
              → 스틱 1~5 (begin pass 2/2 ×5, AGING skip ×5)
16:40:02.001  GLASS 6_MEASURE - End → 7_RESULT → 8_UNLOAD - End 16:40:09
```

- 알람 0, WARN 0. WAD 행 T0 30 / T1 30.
- 결과 파일: 경계 시각에 `RESULT\APD\APD_20260928-163850.txt` + `RESULT\DATA\28\_20260928-163850.xls`(T0), 글라스 종료에 `APD_20260928-164002.txt` + `_20260928-164002.xls`(T1) — **T0/T1 파일이 따로 남는 OCP 동작 재현**. `RESULT\APD.txt`는 마지막(T1)으로 덮임.
- APD 크기 597 B = RAW_DATA 0행. OOS는 WAD 장비 응답이 없어 원래 0행(09-21 이후 전 런 동일)이므로 **결과 값 내용은 이 PC에서 검증 불가**.
- XLS 파일명에 글라스 ID가 빠지는 것(`_2026….xls`)은 08-28 이후 모든 OOS 결과 파일과 같은 기존 동작(글라스 ID 미수신)으로 EFA와 무관.

### 9.2 추후 검증 (CIM 통신 · 실기) — 미실시

CIM 호스트(또는 시뮬레이터)와 실기 장비가 있어야 하는 항목. **사용자 결정(2026-09-28): 추후 검증.**

| # | 항목 | 확인 방법 / 기대 결과 |
|---|---|---|
| V1 | PPID `'B'` 진입 | 호스트가 `PPID^{레시피}^xBxx$` 송신 → CIM 로그 `[CIM] PPID WorkState 'B' - EFA mode`, `PPID OK` 응답. EFA rcp가 없는 레시피면 `… EFA rcp missing/empty` WARN + PPID NG |
| V2 | PPID `'A'` 진입 | `^xAxx$` → `[CIM] PPID WorkState 'A' - aging mode`(강화검사 플래그). 기존 `'J'` 호스트와의 호환도 확인 |
| V3 | T0 APD 업로드 | ONLINE REMOTE 런에서 경계 시 `SendAPDFileToFTP() Start` → `EFA: T0 APD FTP done`, FTP 서버 `{GlassID}\APD.txt`가 T0 내용 |
| V4 | `EFA^0$` 핸드셰이크 | CIM SEND 로그 `EFA^0$` → 호스트 응답 시 `[CIM] EFA^0_OK received` + `EFA^0_OK OK (n ms)`; 무응답 시 5초 후 `TIMEOUT - continue`로 T1 진행(D-2) |
| V5 | `EFA^1$` | 글라스 종료 CIM 시퀀스에서 `CDoWorkSequenceCIMStep_MeasDone : EFA glass - send EFA^1$` → 500 ms → `STS1106` → DataJudge 순서 |
| V6 | 최종 APD | GDTREQ 단계 APD 업로드가 T1 결과로 나가는지(호스트 측 T0/T1 두 건 수신) |
| V7 | 결과 덮어쓰기 실측 | 실기 WAD 응답이 있는 런에서 결과 화면·APD RAW_DATA가 T1 값만 남는지, T0 파일(경계 APD/XLS)은 T0 값인지 |
| V8 | 실기 모션 | AGING 리뷰 스냅 헤드 이동(Y1/Y2 리뷰, Y3/Y4 Safety)이 실기 간섭 없이 동작, 스냅 이미지(카메라 0/1)가 의도한 화면인지 |
| V9 | 모션 OOS 우회 영향 없음 | 실기(모션 사용)에서 `MOTION_INTERLOCK_11` 인터락이 종전대로 동작(우회는 `IsOutOfServiceMC()`일 때만) |
| V10 | 사이클 반복 | Cycle Count ≥ 2에서 글라스마다 T0/T1 반복, 플래그 상호배타(EFA ↔ 강화검사) 유지 |

**미결 사항**
- D-3 `WORKSTATE_CHANGE^_CAA$` 미이식(원본 Format 인자 누락 버그). 호스트 사양 확인 후 결정.
- 기존 결함(미수정): `Process_PPID`가 `OpenRecipe` 실패·`'J'` 실패를 반환값에 반영하지 않아 PPID NG가 안 나감.
- 다이얼로그 저장이 스틱 수 밖 `STICK_n` 키를 지우지 않음(무해, OCP 동일).
- 16:13:40 / 16:40 부근 Idle 직전 PG `08010200` 1회 추가 송신 — 15:58 런에도 있던 기존 패턴, 정체 미확인.
