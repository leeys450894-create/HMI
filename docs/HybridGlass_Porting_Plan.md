# Hybrid Glass(멀티 세그먼트 레시피) 이식 플랜 (ICP → HMI)

작성: 2026-09-28 (LYS) — **플랜 단계, 코드 변경 0**
원본: `D:\OCP\0.SW_Matching\ICP\` (git, `feature/hybrid-glass` 첫 커밋 `4bf17e8` → master `1bd8b6e`, 125커밋, 2026-08-21~09-28)
대상: HMI `Source/ALPHA` + `CoreFunction/dll_Recipe/dfcRecipe`
참고 문서: ICP `HybridGlass_Design_Notes.md`(510줄), `docs/handoff/hybrid-panel.md`(386줄), `강제배출_EQPABORT.md`, `AGENTS.md`, `README.md` / HMI `docs/07_Recipe.md`, `03_AutoSequence.md`, `02_CallGraph.md`, `09_Thread.md`, `10_GlobalState.md`, `11_DataFlow.md`, `12_ImpactAnalysis.md`, `Resume_Inspection_Plan.md`, `Glass_Rotation_Change_Plan.md`, `OneCell_To_MultiPoint_Migration_Plan.md`, `Result_Identification_MES_Agreement_Plan.md`, `WAD_MultiMeas_Porting_Plan.md`, `AgingNew_EFA_Porting_Plan.md`, `AgingCycleEngine_Design_Plan.md`, `PLC_Mapping_and_Sequence.md`, skills `hmi-measure-grouping`/`hmi-logs`
선례: [AgingNew_EFA_Porting_Plan.md](AgingNew_EFA_Porting_Plan.md) — 스틱 루프를 `for(nPass)`로 감싸고 경계 훅(`EfaPassBoundary`)을 둔 구조가 이번 세그먼트 루프의 직접 선례

---

## 0. 한 줄 요약

ICP Hybrid Glass는 **"레시피 1개 = 세그먼트 N개, 세그먼트 1개 = 거의 완전한 레시피 1개"** 모델이다. 기존 단일 레시피 슬롯(`m_recipeMgr`)을 **활성 세그먼트의 스테이징 슬롯**으로 쓰고 옆에 `vector<S_RecipeMgr>`를 두어, 기존 접근자·섹션 로더·그리드 코드를 거의 건드리지 않은 것이 핵심이다. 측정은 `for(seg) { 세그먼트 전환 → 외부 레시피 재로드 → (방향 바뀌면) Detach/Attach/Align → 스틱 루프 → 스펙체크/재측정 }` 후 APD 1회.

HMI는 **이 "스테이징 슬롯 + 벡터" 메커니즘을 그대로 가져오되**, HMI 구조를 지키기 위해 세 가지를 ICP와 다르게 한다.
1. **파일 포맷은 꼬리(tail) 블록 방식** — 세그먼트 0은 지금 포맷 그대로 쓰고 세그먼트 1..N-1과 세그먼트 헤더를 파일 끝에 붙인다. 구 빌드도 세그먼트 0만은 정상 로드한다(ICP는 세그먼트 수를 버전 바로 뒤에 넣어 구 빌드가 읽지 못한다).
2. **`preDefine.h`(S_RecipeInfo 등) 구조체는 건드리지 않는다** — 세그먼트 메타(이름·외부 레시피명)는 `C_In/OutRecipeFunc` 클래스 끝의 별도 벡터에 둔다. DLL 원시 포인터 ABI·이중 사본 위험을 최소화한다.
3. **방향 전환은 HMI 방식(Stage R 회전 + 재Align)** — HMI는 글라스를 흡착한 채 `AXIS_LOADER_R`(=Stage 회전)로 스틱마다 이미 회전한다. ICP의 Detach→Loader 회전→Attach는 이식하지 않는다. **확정(D-1, 2026-09-28): 세그먼트 종료 → Stage Safety 위치 이동 → 회전 → 재Align → 다음 세그먼트.**

**AgingTest(강화검사)·EFA와 동시 사용을 지원한다(D-4 확정 변경, 2026-09-29)** — ICP도 Aging×Hybrid가 호환되는 상태이므로 HMI도 AgingTest·EFA 모두 세그먼트와 함께 동작해야 한다. AgingTest는 스틱 내부 루프라 세그먼트 루프 안에 그대로 들어가고, EFA는 `EFA^0$`가 글라스당 1회여야 하므로 루프 순서를 **pass → seg → stick**으로 둔다(§5.9).

**Cell ID는 자동 글자 이월을 먼저 넣고, 이어서 A/B 수동 입력·Manual 편집기까지 이식한다(D-5, 2026-09-29).** **Resume(중단 후 이어 검사)은 세그먼트·AgingTest·EFA 전부에서 동작해야 한다(D-6, 2026-09-29)** — 결과 재적재는 파일 정렬 추정 방식 대신 결과 저널 재생 방식으로 바꾼다(§5.8).

스펙 판정 후 재측정 다이얼로그(RemeasureDlg), 물리 셀맵, Inner ID 수명주기 개편은 **HMI에 대응 기능이 없거나 체계가 달라 범위에서 제외**한다(§7).

---

## 1. 교차검증 결과 — 문서 vs 코드 (요약)

문서만 믿고 설계하면 틀리는 지점. 전부 코드로 확인했다(✔ = 본 세션에서 직접 코드 확인).

### 1.1 ICP 문서 vs ICP 코드

| # | 문서 기술 | 실제 코드 | 이식 영향 |
|---|---|---|---|
| I1 | 설계노트 §3: "코드 변경 없음" | 125커밋 구현 완료 | 설계노트는 **의도**, 핸드오프가 **현황**. 둘 다 일부 stale |
| I2 | §1/§4C/§4D: 방향이 이전 세그먼트와 다르면 Detach/Attach | ✔ **Stage 진공압 센서만으로 판단** (`CommFunc_Proc.cpp:3288-3305`). 이전 방향 비교는 안전위치 이동(:3310)·Align(:3370) 조건에만 사용 | "방향 비교 vs 센서"를 분리해서 설계해야 함 |
| I3 | §3.5/§4.3: 별도 "Loader R 회전" 단계 + 신규 Loader-R 설정 위치 | 없음. 회전은 `CmProc_AttachGlass(EGlassRotation,bProcRcp)` 안의 `CFG_ATTACH_{S,L}_LOADER_R` 이동으로 암묵 수행, Deg0/Deg90만 지원 | HMI는 Loader가 없으므로 해당 없음(§3.4) |
| I4 | §4D: 재측정 루프는 그대로, RemeasureDlg 변경 불필요 | 재측정은 세그먼트 안 `do{}while(bRetrySegment)`로 이동(:3448-3925), CIM `PRCS`의 while 루프 삭제. RemeasureDlg는 레시피 인자·강제배출 감시·물리맵·`SetMeasured` 클리어로 크게 변경 | HMI엔 재측정 기능 자체가 없음 → 제외(Resume은 D-6으로 이식) |
| I5 | §4D: ABU 호출을 LoadRecipe 밖으로 이동, LCR은 자연히 반영 | ABU는 `CmProc_LoadRecipe` 안에 그대로(그 함수가 세그먼트마다 호출됨), LCR은 :3835에서 명시 해석 | HMI 외부 레시피는 PG/WAD 등(§5.6) |
| I6 | §4D: 진입 순서 Attach→LoadRecipe→Align | LoadRecipe(비동기 송신)→Detach/Attach→안전위치→`IsLoadRecipe` 대기→PTP→ABU→Align | **송신과 대기를 분리**하는 패턴은 HMI(RecipeStep 송신 / PreMeas 대기)와 같다 |
| I7 | §3.7/§4D: Area 오프셋 3곳 일치 필요 | 런타임은 `m_nArea`만 사용. 이월은 `UpdateAreaData`(저장 시)·`CmRcp_ApplyMeasFlags`에만. ✔ **TEG 경로는 세그먼트마다 `UpdateArea()`를 오프셋 0으로 호출**(:3404-3407) → 세그먼트 1부터 행 충돌 | ICP 결함, **이식 금지** |
| I8 | §4D: 세그먼트 블록 = index + name | `_SEGMENT_` + name만, 로드 시 태그 미검증 | HMI는 태그 검증 추가 |
| I9 | §4D: 세그먼트명 기본 "Segment#N", 유일성 보장 | MakeCell은 "Segment1"(# 없음, `MakeCellDlg.cpp:1008,1020`), 유일성은 이름 변경 시에만 검사 | 이름 규칙·검증 한 곳으로 통일 |
| I10 | 핸드오프 §4.15 "모든 세그먼트 단일 CAD 프레임" | `C_PhysicalCellLayout`은 무변환, `C_GlassMap`은 세그먼트 방향≠장착 방향이면 회전 변환 → **0°+90° 혼합 레시피에서 둘 중 하나는 틀림** | HMI는 프레임 규칙을 먼저 정한다(D-10) |
| I11 | 핸드오프: 7개 접근자 fallback 계약 통일 | 회전·셀크기 접근자는 범위 밖일 때 기본값 대신 live 값 반환(`RecipeFunc.cpp:151,158,165`) | 계약을 한 번에 정확히 구현 |
| I12 | 설계노트: 구 파일 호환 | `_CELL_SELECT_`(4→7줄)·`_INSPECTION_ITEM_`(6→7)이 **버전 게이트 없이** 변경 → 하이브리드 이전 파일은 정렬 어긋남 | HMI는 모든 추가를 게이트로 |
| I13 | 설계노트 §4D: 세그먼트 전환 시 팝업 재타깃 | SelectCellDlg만 갱신. 열린 ContactOffset/SPC/2DVS 등은 활성 세그먼트에 저장, AlignRcpDlg 미갱신 | HMI UI는 전환 시 팝업 닫기 or 재바인딩 |
| I14 | (문서 없음) | 세그먼트별 Align 레시피(`LoadSegmentAlignRcpFiles`, `S_Align::m_bLoaded`), 하나라도 없으면 자동 시작 거부(`CommFunc_Proc.cpp:3153-3158`) | **이식 대상**(HMI도 세그먼트별 .alg 필요) |
| I15 | `강제배출_EQPABORT.md` §2-7/2-8 | 인자·게이트·"항상 TRUE" 기술 stale, 중첩 팝업 레이스는 이미 처리됨 | HMI에 강제배출 체계 없음 → 참고만 |
| I16 | `README.md` / `AGENTS.md` | README는 GitLab 템플릿, AGENTS는 CLAUDE.md에서 Hybrid 절만 뺀 사본 | 정보 없음 |
| I17 | (실파일 확인, 2026-09-29) `D:\HBS\ICPControl\RECIPE\MAIN_RECIPE\hybrid_rcp.rcp` — 작성 순서: Seg 1/2에 N61_CE0_H2, Seg 2/2에 N71_CE0_H1 CAD Open+Apply → RECIPE APPLY | 파일은 V1007, 세그먼트 2, `_CELL_ID_PANEL_ 0 0`, 세그먼트마다 `_SEGMENT_`+이름+16섹션(§2.3 문법과 일치). 단 **`Segment1 = N71_CE0_H1`(L, rotate 1), `Segment2 = N61_CE0_H2`(S, rotate 0)으로 작성 순서와 반대**, Segment1의 `_CAD_INFO_`는 거의 0(Segment2만 값 보유) | ICP MakeCell 결함 확인 — CAD 데이터는 Apply 시점 활성 세그먼트만 저장(§2.5 UI 결함 #16), 세그먼트 전환 시 그리드↔세그먼트 대응 어긋남 의심. HMI MakeCell은 세그먼트마다 CAD 행 전체를 저장·복원하고, 세그먼트 순서를 입력 순서로 고정(Phase 2 검증 항목) |

### 1.2 HMI 문서 vs HMI 코드

| # | 문서 기술 | 실제 코드 | 이식 영향 |
|---|---|---|---|
| H1 | `07_Recipe` §5 버전표 V1004까지 | ✔ V1010까지, `RECIPE_FILE_END`=1011 | 신규는 **V1011 추가, END=1012** |
| H2 | `07_Recipe` §6/§4.1, `10_GlobalState` | 검사항목 10개(`M_RCP_INS_ITEM`=10), `_HEAD_SUBZ_`/`_ANGLE_AXIS_`/`_STICK_CONTACT_OFFSET_`/`_ANGLE_SUBZ_` 블록 누락 | 포맷 기준은 **코드**(§3.1) |
| H3 | `07_Recipe` §9.1: `GetRecipeMgr`=g_inRcpInfo 캐스트 | 별도 힙 객체 `g_outRcpInfo`(`API_RECIPE.cpp:27-29,78-82`) | 런타임 인스턴스는 `g_outRcpInfo` 하나 |
| H4 | `07_Recipe` §17.2: 인스턴스 2개 | 3개(g_inRcpInfo 편집기 / g_outRcpInfo 런타임 / `CRecipeData...stRecipeHMI` 사실상 사장) | 편집기↔런타임 동기화 없음(주석 처리됨) |
| H5 | 루트·HMI CLAUDE.md: 게이트는 항상 `>`, 모든 블록에 `else{Clear()}` | Aging_Test는 `>= V1006`(현장 1006 파일이 의존), V1001/1002/1009는 else 없음 | 기존 게이트는 **손대지 않는다** |
| H6 | (문서 없음) | ✔ 로더가 `m_nVersion`을 **대입하지 않음**(`GetRcpVer()` 항상 0), ✔ 로드 시 `m_sData`만 Clear — `m_sInfo/m_sMap`은 필드 단위 덮어쓰기(미저장 필드 SRM 램프·RTest·ProbeW/H 등은 직전 값 유지) | 세그먼트 k≥1 로드 기준값을 **세그먼트 0 복사본**으로(§5.3) |
| H7 | (문서 없음) | 타임아웃 파싱 `GetAt(i)` i<10 무게이트 — 9토큰(V1005) 파일에서 MFC 범위 예외 가능 | 선결 결함 후보(P0, 미확인) |
| H8 | `03_AutoSequence` §2.2: Adsorption 첫 사이클만 | `bFirst`가 사이클 루프 안 선언 → 매 사이클 수행 | Adsorption 재실행은 Align 리셋 — 세그먼트 경계에서 재사용 금지 |
| H9 | `03_AutoSequence` §10.1 측정 스텝 목록 | 실제 12스텝: None, MoveMeasureArea, PtrnSearch, Probe_Contact, OST, RTEST, AGING, AGING_PTN, WAD, ANGLE, VISION, Idle(`DoWorkGlassMeasure.cpp:42-59`) | 루프 설계 기준 |
| H10 | `09_Thread` §3.4: `CMeasureSequenceItemEngine`이 측정 오케스트레이션 | 스레드 미사용, 컨텍스트 객체(Aging/EFA 상태·InnerID) | 세그먼트 상태도 여기 두기 적합 |
| H11 | `Resume_Inspection_Plan` §9.4 재시작 reload 완료 | ✔ 엔진 시작부 `SaveResumeInfo(..., -1, ...)`가 resume 모드에서도 호출(`DoWorkGlassSequenceEngine.cpp:664-680`) → RecipeStep이 LastStick=-1을 읽어 reload 0건 (로그 확인 필요) | **선결 결함**(P0) — 세그먼트 Resume의 전제 |
| H12 | HMI CLAUDE.md Resume: "마지막 완료 스텝·측정 인덱스 저장" | `LastStick`만 저장, pass/segment 없음 | Resume에 LastSegment 추가(P5) |
| H13 | `AgingNew_EFA_Porting_Plan` | `UpdateStickProgress(i)`가 pass 비인식 — T1 중단 시 T0을 i+1부터 재개 | EFA 문서에 기록 필요(본 플랜 §10) |
| H14 | `02_CallGraph` §13.1, `PLC_Mapping_and_Sequence` §6.2 | Adsorption은 PLC R2M/L2M로 흡착, Unload는 글라스를 떼지 않음(CIM 시퀀스가 수행) | Detach/Attach를 시퀀스 중간에 넣는 설계는 비현실적(§3.4) |
| H15 | `10_GlobalState`: `m_bMeasMultiRcp` 멀티레시피 플래그 | 참조 0(주석만) | 재사용하지 않음(의미 혼동) |
| H16 | `Glass_Rotation_Change_Plan` §2.3 | 콤보 4옵션 편집 가능, MakeCell은 0°/90°만, rotate=0이면 `m_fStageRotate`=90 강제(`MakeCellDlg.cpp:463`) | 세그먼트별 회전 = 스틱 `m_fStageR` 체계 그대로 |
| H17 | `hmi-logs` 스킬: `measure_seq_item_engine_*.log` 항상 빔 | AgingCycle/EFA가 기록 중 | 세그먼트 로그도 이 파일로 |
| H18 | `WAD_MultiMeas_Porting_Plan` 경로 키 `[USER_OPTION]` | 실제 `[OPT_PARA] WAD_MULTI_RCP_PATH` | 사이드카 선례 참고 시 주의 |

---

## 2. ICP 원본 분석 — 기능 코드 단위

### 2.1 데이터 모델 (`preRecipeDefine.h`, `RecipeFunc.h`)

- `C_RecipeFunc`: `S_RecipeMgr m_recipeMgr`(활성=스테이징) + `vector<S_RecipeMgr> m_vecSegments` + `int m_nActiveSegment`(생성자 -1) + `ECellIdMode m_eCellIdMode[2]`(레시피 전역, 세그먼트 밖).
- `S_RecipeMgr`에 `TCHAR m_chSegmentName[CH_LEN]` 추가. 세그먼트 = `m_sInfo/m_sData/m_sMap/m_sAlign/m_sTegCellInfo` 전부 → **Align도 세그먼트별**.
- `S_RecipeMap::m_nGlsRotate(int)` → `EGlassRotation m_eGlsRotate`(Deg0/DegNeg90/Deg180/Deg90, 지원은 Deg0·Deg90만).
- `S_OptionItem`(세그먼트별): `m_chPGName/m_chSpecName/m_chQualityName/m_chLCRName[CH_LEN]`(V1003, 공란=Recipe ID fallback), 멀티패턴 Aging 배열(V1005).
- `S_CellInfo`: `m_strManualCellId[2]`(V1006), 런타임 `m_bIsMeasured[2][M_MODE_MAX]`, `m_strInnerID`, `m_nInnerIdAttempt`, `m_nSelectCell[INSP_TYPE_MAX][M_MODE_MAX]`(1차원→2차원).

### 2.2 세그먼트 API (`RecipeFunc.cpp`)

| API | 동작 |
|---|---|
| `SetActiveSegment(n)` (:132-146) | 범위 검사 → `m_vecSegments[old] = m_recipeMgr`(write-back) → `m_recipeMgr = m_vecSegments[n]` |
| `GetSegmentCount()` / `GetSegmentLoopCount()` | `size()` / `max(1,size())` — 0은 "한 번도 로드 안 됨" |
| `GetSegmentMgr(i)` (:225-231) | 쓰기 가능 포인터, 활성 슬롯이면 `&m_recipeMgr` |
| `GetSegmentGlassRotation/CellSizeX/Y/StickCount/CellCount/StickItem/CellItem(i,..)` (:148-223) | 읽기 전용. fallback: ①활성→live ②벡터 범위→그 항목 ③벡터 비었고 i=0→live ④그 외 기본값 |
| `LoadRcpFile` (:581-660) | 벡터 clear → 버전 → (≥V1002) 세그먼트 수 → (≥V1006) `_CELL_ID_PANEL_` → 세그먼트마다 `m_recipeMgr.Clear()` + 16섹션 로드 + push_back → `SetActiveSegment(0)` |
| `LoadSegmentAlignRcpFiles()` (:767-794) | 세그먼트마다 `m_chAlignName`의 .alg 파싱(공유 파일 캐시), 하나라도 실패하면 전부 `m_bLoaded=FALSE` |
| `UpdateArea(nBase)` (:1079-1123) | `Area = i*nCellCnt + j + offset` + TEG/VACS/Aging 추가 행, **다음 빈 Area 반환**(이월용) |
| `AllMeasCheckNonSamplingSegments()` (:483-500) | 새 잡 시작 시 1회, 활성 세그먼트를 바꾸지 않고 전 세그먼트 전체 선택 |
| Cell ID: `CellIdLetterPrefix`(bijective base-26), `BuildSegmentAutoCellIds`, `BuildResolvedCellIds`, `ValidateCellIdConfig`(순수), `UpdateAllCellID`(유일한 쓰기) | 글자 오프셋을 세그먼트 순서로 이월(Long=셀 수, Short=스틱 수) |

### 2.3 파일 포맷 (ICP)

```
_VERSION_ ver
[ver>=1002] segCount
[ver>=1006] _CELL_ID_PANEL_ modeA modeB
{ [ver>=1002] _SEGMENT_ name     ; 인덱스 없음, 로드 시 태그 미검증
  header option inspection lcr contact lamp coordinate cellSelect cad vacs qualCam varAngle padInfo specSpc specCoeff preTest } × segCount
```
N=1도 count=1로 저장. "하이브리드" 플래그는 없음(N>1이 곧 하이브리드). 저장은 `RecipeMgrDlg::SaveFile`(:399-496) — 파일 열기 **전에** Cell ID 검증.

### 2.4 시퀀스 (`CmProc_MeasProc`, `CommFunc_Proc.cpp:3141-3995`)

```
MeasProc(flow):                                   // Auto/CycleTest/StandaloneManual 정책
  세그먼트 중 Align 없는 것 → 알람, return F       // :3153
  Aging 스케줄 검증, Cell ID 준비, InnerID attempt  // :3161-3163
  !CIM resume → 결과 그리드 ClearData(1회)         // :3187
  !resuming → AllMeasCheckNonSamplingSegments      // :3225
  for seg in 0..N-1 while !eject:
    CIM resume && seg < LastSegment → continue      // :3233
    이전 세그먼트 비동기 Quality 결과 드레인        // :3237
    [CIM] LastSegment=seg, (resume 대상 아니면) LastStick=-1
    SetActiveSegment(seg); FDC 게시
    CmProc_LoadRecipe (PG/Spec/Quality/ABU 송신 — 매 세그먼트, 중복 제거 없음)
    tgt=세그먼트 회전; !진공(tgt) → [진공(opp)이면 Detach(opp)] → Attach(tgt), didAttach
    didAttach||첫 세그먼트||tgt≠prev → 안전위치 이동
    SetMaxStick, 측정 플래그, IsLoadRecipe 대기, PTP, ABU Contact
    useAlign && (didAttach||첫||tgt≠prev) → GlassAlign(세그먼트 .alg)
    [TEG] UpdateArea(0)  ← 결함(I7)
    결과 그리드 헤드 행 기록
    do:
      for stick: resume catch-up(대상 스틱 1회 m_bIsMeasured 클리어), 측정...
      세그먼트 SPC && 스펙아웃 → RemeasureDlg(작업 스레드 DoModal)
        Cancel→[CIM]북마크, return F | Remeasure→새 attempt, 재시도 | 진행
    while retry
  안전위치, Quality 드레인, SaveAPD 1회           // :3970
  [CIM] LastStick=LastSegment=-1
```

### 2.5 UI (총 ≈2,600줄, 신규 파일 ≈1,300줄)

| 다이얼로그 | 하이브리드 내용 |
|---|---|
| MakeCellDlg (~450줄) | 그리드형 HYBRID MODE 체크 + SEGMENT COUNT + APPLY(유일한 커밋 지점), SEGMENT NAVI `<` "Seg N/M" `>`, 자체 `m_sMakeRcpInfo`, 체크 해제 시 즉시 단일화(확인 없음), RECIPE APPLY 덮어쓰기 시 7단계 보존 재조정(`S_ProtectedSegmentFields`) |
| RecipeMgrDlg (~260줄) | 편집형 콤보(CBS_DROPDOWN, 정렬 없음)+Prev/Next, 전환 전 `GetRecipeView()` flush, 이름 변경(유일성), PG/Spec/Quality/LCR 레시피 콤보 행 4개(`GetReipceList` 1~4), Cell ID A/B EDIT 행, 저장 루프 |
| CopySegmentOptionsDlg (187) | 새 세그먼트가 기존 세그먼트 옵션을 12개 범주별 복사 |
| CellIdEditDlg (750) / PhysicalCellLayout+CellMapGrid (361) | A/B Auto/Manual ID 편집, 전 세그먼트 합성 물리 셀맵 |
| GlassMap (~95) | 전 세그먼트 합성 렌더, 장착 방향 기준 렌더 전용 회전 |
| Manual Meas/Teach (~350) | `theManualRecipe` 세그먼트 선택기 |

### 2.6 ICP 결함 — **이식하지 말 것**

1. TEG 경로 `UpdateArea()` 오프셋 0 (I7).
2. 덮어쓰기 Apply가 MakeCell의 `m_nActiveSegment`까지 복사 → 그리드가 보여주는 세그먼트 ≠ 활성 세그먼트 → 다음 `GetRecipeView()`가 옵션을 엉뚱한 세그먼트에 씀.
3. 세그먼트 전환 시 팝업 미재바인딩(I13).
4. Manual 세그먼트 전환 시 `SetMaxStick`/`SetIgnoreOSTRes` 미재호출.
5. 기본 이름 규칙 불일치, 태그 미검증, 이름 유일성 로드/저장 시 미검사.
6. 레시피 로드 실패 시 벡터가 반쯤 비워진 채 UI 미정리.
7. APD `CELL_X/Y_CNT`·`MEAS_MODE`가 마지막(또는 중단된) 세그먼트 값.
8. `WritePPIDFile`이 세그먼트 0 형상만 기록.
9. 외부 레시피를 이름이 같아도 매 세그먼트 재송신(중복 제거 없음).
10. `SaveProbeInfoSection`이 저장 시 `m_vecProbeX`를 pad 정보로 재계산 → 복사한 probe X 덮어씀.

---

## 3. HMI 현황 (2026-09-28 코드 기준)

### 3.1 레시피 구조

- **DLL `dfcRecipe`**: 편집기 `g_inRcpInfo`(C_InRecipeFunc, 정적) / 런타임 `g_outRcpInfo`(C_OutRecipeFunc, `Recipe_Initialize`에서 힙 생성, `GetRecipeMgr()` 반환). **`C_OutRecipeFunc`는 AFX_EXT_CLASS로 export, `m_recipeMgr`는 public, 인라인 접근자는 ALPHA 쪽에서 컴파일**.
- `S_RecipeMgr`(`preDefine.h:709-731`) = `m_nVersion, m_nStickCnt, m_nCellCnt, S_RecipeInfo m_sInfo, S_RecipeData m_sData, S_RecipeMap m_sMap, S_Align m_sAlign` + `Clear()`. **ICP와 같은 모양 → 세그먼트 1개 = `S_RecipeMgr` 1개로 그대로 쓸 수 있다.**
- 파일(`.rcp`, 텍스트 1줄 1값): `_VERSION_` → `_CELL_INFO_` → `_OPTION_INFO_` → `_INSPECTION_ITEM_`(10) → `_PROBE_ID_` → `_CONTACT_POS_` → `_LAMP_` → `_HEAD_OFFSET_` → `_STAGE_`(스틱별 번호/Y/R) → `_HEAD_Z_` → `_CELL_`(스틱별 ID/Y/X/Area) → `_CELL_SELECT_` → [>1001]`_CAD_INFO_` → [>1002]GlobalMark… → [>1005]`_HEAD_SUBZ_` → [>1007]`_ANGLE_AXIS_` → [>1009]`_STICK_CONTACT_OFFSET_` → [>1010]`_ANGLE_SUBZ_`.
  - 저장: `C_RecipeMgrDlg::SaveFile`(DLL `RecipeMgrDlg.cpp:416-682`, 항상 `RECIPE_FILE_END` 기록)
  - 로드: `C_OutRecipeFunc::LoadRcpFile`(:95-379) / `C_InRecipeFunc::LoadRcpFile` — **공백 외 동일한 이중 사본**
  - Align: `.alg`(무버전) — `LoadAlignRcpFile`
- **스틱당 셀 수 균일**이 포맷에 박혀 있음(저장 `GetCellCount(0)`, 로드는 `m_nCellCnt`를 모든 스틱에 적용). ICP도 "셀 수가 다르면 곧 세그먼트 경계"라는 불변식 → 충돌 없음.
- **외부 레시피명 없음**: PG(`CommTcpIpObjectPG` S2F1 `szModel`)·WAD(`stELP.szModel`)·Vision/Color/Angle 전부 `GetRecipeID()`(=PPID=.rcp 파일명)를 보냄. 레시피에 있는 외부 이름은 `m_chAlignName`뿐.
- **ABI**: `GetRecipeInfo/Data/Map/MgrPtr()`가 DLL 객체 내부 원시 포인터를 반환. 소비처 grep 수: `GetRecipeInfo()` ALPHA 55 + DoWork 54 + Recipe 12, `GetRecipeData()` 59, `GetRecipeMgrPtr()` 51. `preDefine.h`·`OutRecipeFunc.h`·`RecipeShareMem.h`·`API_RECIPE.h`는 ALPHA `Source/Include/`와 DLL에 **바이트 동일 사본**.

### 3.2 런타임 로드 경로

`OpenRecipe(id)` → `RecipeChange`(`Recipe/RecipeEditMgr.cpp:111-148`) → `LoadRcpFile` → `LoadAlignRcpFile(m_chAlignName)` → `SetRecipeID` → `WM_MSG_UPDATE_RECIPE_INFORMATION` 게시.
호출자: CIM `Process_PPID`(`CommTcpIpObjectCIM.cpp:1416`), 글라스 시작 Recipe 아이템(`DoWorkGlassRecipeStep.cpp:188-192`, 이어서 `stGlassInfo.m_nCellCount/m_nStickCount` 갱신 :201-202), **수동 화면 6곳이 같은 `g_outRcpInfo`로 로드**(Meas/MultiMeas/AgingTest/AgingNew/AutoContact/MeasureCycle). `.alg`만 재로드: Adsorption(:476, **PROBE_ALIGN 레시피로 `m_sAlign` 덮어씀**), SystemInitialize, ProbeACC.

### 3.3 셀/스틱/회전 모델

- 좌표 원점 = 글라스 중심. Stick Y(`m_fStageY`) — OCP는 절대값(`DoWorkGlassMeasureStep.cpp:709-720`). Cell X → 헤드 Y1..Y4 직접.
- **방향은 이미 스틱 단위**: MoveMeasureArea가 스틱마다 `dStageR = m_vecStick[i].m_fStageR + m_fStageRotate`로 ✔ `AXIS_LOADER_R` 회전(:723, :969-973). Long/Short는 지금 **별도 레시피**(예: `1_OCP_TEST_Long` rotate=1/R=90, `2_OCP_TEST_Short` rotate=2/R=180, 각자 .alg).
- `m_nGlsRotate` 소비처는 `==1` 분기만(0/2/3은 역순 경로). Cell ID는 런타임 `MakeCellID/GenCellID`(:116-223)가 GlassID 기반으로 생성(레시피 `m_chCellID`는 로그용), B 패널은 번호 오프셋.
- 결과 행: `nAreaNum = cellIdx + nArea*nMeasStick + 1 + nAgingRow`를 WAD/VISION/ANGLE/COLOR/VISION_COLOR 스텝이 **각자 계산**, `CCommTcpIpFactoryMgr::SetMeasUseInfo`(✔ :2313, `memcpy m_nAreaNum`)로 장치에 전달 → dResult 그리드 행.

### 3.4 시퀀스

- 글라스 엔진 9아이템(`DoWorkGlassSequenceEngine.cpp:46-54`): Adsorption → Recipe(ClearData 또는 resume reload, 장치 레시피 송신) → Glass SW Align → Probe SW Align → PreMeas(스틱0 이동, 패턴서치, `RcpChangeDone` 대기, P2P) → AGING → **Measure(스틱 루프)** → Result(`SaveResult`) → Unload(PG off·안전위치, **글라스 분리는 CIM 시퀀스**).
- 엔진은 아이템마다 ✔ `Start()` **다음에** `SetLoopStepIndex(nStick)`(:924→:932) — 경합 가능(첫 글라스는 0 → 측정 skip 위험).
- Measure(`DoWorkGlassMeasure.cpp:139-372`): `for(nPass){ for(i=스틱){ 스텝 루프 } }`. **완료 이벤트는 마지막 스틱·마지막 pass의 Idle만** 올린다(`i+1==GetLoopStepIndex() && nPass==nPassCount-1`, :242) — 이 가드가 없으면 RESULT/UNLOAD가 동시 진행(EFA 3032 사례).
- **HMI에는 Loader 분리 회전이 없다.** Attach/Detach = PLC R2M/L2M·M2R/M2L 상태 전환(`FunctionModuleEQStateChangeStep`), L2M/M2L/R2L의 `MoveLoaderXYR`은 `AXIS_X`+`AXIS_LOADER_R`을 움직이고 **`ResetAlign()`을 호출**(:32). 측정 중 재흡착은 기계적으로는 가능하나 Align 리셋·Stage X 안전위치 선행이 필요.
- Glass Align 재실행 선례: `GeDoWorkGlassSequence()->ItemStart("SEQUENCE_ITEM_DWGLASS_SEQ_..._SW_ALIGN", true)`(Adsorption :486-487이 Probe Align을 이렇게 호출). Align 결과는 XYR 오프셋 1개로 `AXIS_X/Y1-4/LOADER_R`에 가산 — **X/Y 오프셋이 Stage R과 함께 회전하지 않음** → 방향이 바뀌면 재Align 필수.

### 3.5 결과·CIM·Resume

- dResult `SaveResult` → `TotalResultData` 스레드(XLS/APD), 그리드는 지우지 않음. EFA 경계가 결과 플러시 선례.
- CIM: PPID 1개 → `OpenRecipe`. 글라스 끝 `MEAS_DONE` → (EFA1) → `STS1106`. **스펙 판정+재측정 기능 없음**(OCP 에이징 전류 스펙아웃 알람만 존재).
- Resume(`ResumeInfoMgr`, **LF-only**): GlassID/RecipeID/LotID/InnerID/LastStick/TotalStick/GlsRotate/상태/시각/경로. `ValidateResume`은 레시피·스틱 수·회전 검사.

### 3.6 선결 결함 후보 (하이브리드 이전에 존재)

| # | 결함 | 근거 | 하이브리드 영향 |
|---|---|---|---|
| P0-1 | Resume reload 무효(SaveResumeInfo(-1)가 resume 전에 호출) | H11 ✔코드 | 세그먼트 Resume 불가 |
| P0-2 | 엔진 `Start()`→`SetLoopStepIndex` 경합 | ✔코드 | 세그먼트별 스틱 수에서 악화 |
| P0-3 | 타임아웃 `GetAt` 9토큰 파일 예외 가능 | H7 (미확인) | 세그먼트 로더가 같은 코드 재사용 |
| P0-4 | `m_nVersion` 미대입 | H6 ✔코드 | 세그먼트 로더에서 버전 필요 → 대입 추가 |
| P0-5 | EFA `UpdateStickProgress` pass 비인식 | H13 | 세그먼트 진행 저장과 함께 정리 |

---

## 4. 갭 분석 — ICP 기능 → HMI 대응

| ICP 기능 | HMI 대응 | 판정 |
|---|---|---|
| 스테이징 슬롯 + 세그먼트 벡터 | `C_In/OutRecipeFunc`에 동일 도입 | **이식** |
| 16섹션 × N 파일 포맷 | 꼬리 블록 방식으로 변형(§5.3) | **변형 이식** |
| 세그먼트명 | 클래스 측 메타 벡터(§5.2) | **이식** |
| 세그먼트별 PG/Spec/Quality/LCR 이름 | HMI 장치 기준 PG/WAD(/Vision) 이름, 공란=RecipeID | **변형 이식**(D-3) |
| 세그먼트별 Align(.alg) + 누락 시 시작 거부 | 동일 | **이식** |
| 세그먼트 루프 + 외부 레시피 재로드 | Measure 아이템 안 `for(seg)`(EFA 선례) | **이식** |
| Detach→Loader 회전→Attach | Stage R 회전(기존 스틱 R 체계) + 재Align | **대체**(D-1) |
| 진공 센서 기반 재흡착 | HMI 흡착은 PLC 상태 전환 — 세그먼트 경계에서 흡착 상태 확인만 | **대체** |
| Area 이월 | `SetMeasUseInfo` 단일 지점 + 세그먼트 base | **이식**(결함 I7 제외) |
| Cell ID 글자 이월(Auto) | `MakeCellID/GenCellID`에 세그먼트 글자 base | **이식** |
| Cell ID A/B Auto/Manual 모델 + 편집기 | HMI `GenCellID`의 셀 토큰(글자+번호)을 리졸버가 공급 | **이식**(Phase 4b, D-5) |
| 물리 셀맵(`C_PhysicalCellLayout`/`C_CellMapGrid`) | 프레임 모순(I10) | **보류**(편집기는 세그먼트별 그리드로 시작) |
| LastSegment/LastStick Resume, 재개 스틱 전체 재측정 | ResumeInfoMgr 튜플 + 결과 저널 | **이식**(Phase 5, D-6) |
| 스펙 체크 + RemeasureDlg 세그먼트별 | HMI 기능 없음 | **제외** |
| LastSegment/LastStick Resume | ResumeInfoMgr 확장 | **이식**(P0-1 선결) |
| 결과 그리드 1회 초기화, APD 1회 | Result 아이템 1회 그대로 | **이식**(구조상 이미 충족) |
| MakeCell Hybrid 모드/개수/Apply/NAVI | DLL MakeCellDlg | **이식** |
| RecipeMgr 세그먼트 선택기·이름 변경 | DLL RecipeMgrDlg | **이식** |
| 덮어쓰기 옵션 보존 + CopySegmentOptions | 단순화 이식(존속 인덱스 보존 + 새 세그먼트는 원본 세그먼트 복사) | **단순화**(D-11) |
| 합성 GlassMap | HMI는 ChartOrbit — 1차는 활성 세그먼트만 표시 + 세그먼트 표시기 | **단순화**(D-10) |
| Manual/Teach 세그먼트 선택기 | 수동 화면 6곳 | **후순위**(D-9) |
| Aging(경시변화: 스틱마다 pass0 → 패턴 스케줄 → pass1)×Hybrid 호환 | HMI AgingTest(AgingCycle 항목 루프)·EFA(2-Pass)를 세그먼트와 동시 지원 | **대체 이식**(§5.9, D-4) |
| 멀티패턴 Aging 레시피 필드, Inner ID attempt, Cycle Test 정책 분리, 강제배출 | HMI에 별도 체계(AgingCycleInfo rcp, MakeUniqueInnerID) | **제외** |

---

## 5. 설계 — HMI 구조 보존 원칙

### 5.1 원칙

1. **N=1이 기본 경로**: 일반 레시피는 세그먼트 1개짜리 하이브리드다. 코드 분기를 "하이브리드냐"로 나누지 않고 `GetSegmentLoopCount()`로 돈다(ICP 원칙 유지).
2. **기존 소비처 무수정**: 원시 포인터 소비처 200여 곳은 계속 `m_recipeMgr`(=활성 세그먼트)를 읽는다. 세그먼트 전환은 오직 **세그먼트 경계(측정 아이템 내부, 장치 idle 시점)** 와 **편집기 UI** 에서만 일어난다.
3. **`preDefine.h` 무변경**: 세그먼트 메타는 클래스 쪽에 둔다. 불가피하게 구조체를 바꾸면 루트 CLAUDE.md 규칙(끝에만 추가, 두 사본 diff 0, 양 모듈 재빌드)을 따른다.
4. **구 파일·구 빌드 호환**: 기존 게이트는 손대지 않고, 새 데이터는 V1011 꼬리 블록에만.
5. **ICP 결함 비이식**(§2.6), **HMI 선결 결함 분리 커밋**(§3.6).

### 5.2 데이터 모델

`C_OutRecipeFunc`/`C_InRecipeFunc` 클래스 **끝에** 추가(두 헤더 사본 동일하게):

```cpp
struct S_SegmentHeader {            // 신규 헤더 SegmentDefine.h (DLL·ALPHA 공용 사본, ASCII)
    wchar_t m_chName[64];           // "Segment#1"...
    wchar_t m_chPGName[CH_LEN];     // 공란 = RecipeID
    wchar_t m_chWADName[CH_LEN];    // 공란 = RecipeID
    // (D-3 결과에 따라 Vision 등 추가)
};
std::vector<S_RecipeMgr>     m_vecSegments;
std::vector<S_SegmentHeader> m_vecSegHeader;
int                          m_nActiveSegment;   // 생성자 -1
```

API(ICP와 동일 계약, 접근자 fallback은 **7개 모두 ④기본값**으로 통일 — I11 교정):
`GetSegmentCount / GetSegmentLoopCount / GetActiveSegmentIndex / SetActiveSegment / GetSegmentMgr / GetSegmentHeader / GetSegmentRotate(i) / GetSegmentStickCount(i) / GetSegmentCellCount(i) / ...`

- `C_OutRecipeFunc`는 DLL 힙 할당이고 ALPHA는 포인터만 받으므로 **끝에 멤버를 붙여도 기존 오프셋은 불변**. 단, 인라인 접근자는 ALPHA에서 컴파일되므로 헤더 두 사본을 반드시 같이 바꾸고 전체 재빌드.
- `SetActiveSegment`가 `m_recipeMgr`를 **값 복사**로 교체하므로 `&m_recipeMgr.m_sInfo` 등 **원시 포인터 주소는 유지**된다(내용만 바뀜). 단, `m_vecStick`의 요소 포인터/반복자를 경계 너머 보관하는 소비처는 무효화 → Phase 3에서 grep 감사.

### 5.3 파일 포맷 — V1011 꼬리 블록

```
_VERSION_ 1012(=END)
<세그먼트 0 본문 : 지금 포맷 그대로, 기존 게이트 전부 통과>
[ver>V1011]
_HYBRID_         nSeg                         ; N=1이면 1
_SEGMENT_HEADER_ { name, pgName, wadName } × nSeg   ; 세그먼트 0 포함
{ _SEGMENT_ idx  <본문 전 섹션, 세그먼트 0과 동일 순서> } × (nSeg-1)
_HYBRID_END_
```

- **구 빌드**는 `_ANGLE_SUBZ_`까지 읽고 멈추므로 **세그먼트 0으로 정상 동작**(하이브리드 레시피를 구 빌드에서 열면 세그먼트 0만 측정 — 경고 문구는 D-12). 구 빌드로 **저장**하면 꼬리가 사라진다 → 두 PC 배포 순서 주의(§10).
- 로더는 기존 본문을 **멤버 함수 `LoadRcpSections(CStdioFile&, int nVersion)`로 추출**(코드 이동만, 논리 무변경)해서 세그먼트 0과 k≥1이 같은 코드를 쓴다. In/Out 두 사본 모두 동일 추출.
- 세그먼트 k≥1 로드 전 기준값 = **세그먼트 0 복사본**(H6: 미저장 필드가 지금처럼 "직전 값 유지" 의미를 갖도록). `Clear()`로 시작하면 SRM 램프·RTest 등 미저장 필드가 세그먼트마다 달라진다.
- 모든 태그를 **검증**(I8 교정). 불일치 시 로드 실패 → 벡터를 로드 전 상태로 되돌림(I-결함6 교정: 임시 객체에 로드 후 성공 시에만 swap).
- `m_nVersion = nVersion` 대입 추가(P0-4).
- `.alg`는 세그먼트마다 `LoadAlignRcpFile(seg.m_chAlignName)` → 각 `S_RecipeMgr::m_sAlign`에 저장. 파일명 공유 시 캐시. 하나라도 실패하면 **자동 시작 거부**(ICP I14와 동일).
- 저장은 세그먼트 0을 지금 코드로 쓰고 꼬리에서 k≥1을 같은 섹션 writer로 반복 — writer도 `SaveRcpSections(CStdioFile&)`로 추출.

### 5.4 런타임 — 세그먼트 루프 (Measure 아이템 내부)

EFA와 같은 위치·같은 패턴(`DoWorkGlassMeasure.cpp`):

```
nSeg       = recipe->GetSegmentLoopCount()
nPassCount = (EFA 글라스 래치) ? EFA 항목 수 : 1          // §5.9 — 글라스 시작 시 1회 래치
curSeg     = 0                                            // 현재 Stage가 맞춰진 세그먼트
for nPass in [resumePass .. nPassCount-1]:
   if nPass>=1 && !EfaPassBoundary(nPass) -> abort       // 모든 세그먼트 T0 뒤 1회 (APD·EFA^0$)
   for seg in [(nPass==resumePass ? resumeSeg : 0) .. nSeg-1]:
      if seg != curSeg && !SegmentBoundary(curSeg, seg) -> abort   // 임의 쌍(N-1→0 포함)
      curSeg = seg
      nStick = GetRecipeData()->m_vecStick.size()        // GetLoopStepIndex() 대신 세그먼트 값
      for i = start..nStick-1:
         AgingCycleBegin(EFA ? nPass : -1)                // AgingTest 항목 루프는 스틱 안(기존 그대로)
         ...
         if (i+1==nStick && nPass==nPassCount-1 && seg==nSeg-1)   // Last-Stick 가드 확장
             run through Idle (complete event)
         SaveProgress(nPass, seg, i)                      // Resume 튜플
```

`SegmentBoundary(from, to)` (신규, `EfaPassBoundary` 모델 — **임의 쌍**을 받는다. EFA T1 시작 시 N-1→0 복귀가 있기 때문):
1. 프로브 비접촉/안전 Z, PG OFF(패턴 전환 전) — MoveMeasureArea가 스틱 회전 전에 쓰는 안전 절차 재사용.
2. `recipe->SetActiveSegment(seg)` → `stGlassInfo.m_nCellCount/m_nStickCount` 갱신 → UI 갱신 메시지.
3. 외부 레시피: 세그먼트 PG/WAD 이름이 **이전 세그먼트와 다를 때만** `DoWorkRecipeOpen` + `DoWorkRecipeOpenComplate` 재호출(I-결함9 교정).
4. 방향(**D-1 확정 순서**): `GetSegmentRotate(to) != GetSegmentRotate(from)`이면
   1. **Stage Safety 위치 이동** — Stage X(`AXIS_X`)를 Safety로(Adsorption `DoWorkGlassAdsorptionStep.cpp:146-165`의 안전위치 이동 재사용), 헤드 Y는 `[SAFETY]`(`df_HEAD_Y_IDLE_SAFETY`) — 회전 인터락(X 이동은 R∈{0,90,180,270}±2) 충족 확인.
   2. **Stage 회전** — `AXIS_LOADER_R`을 새 세그먼트 회전값으로(`MoveCmdPosA(AXIS_LOADER_R, …)`, MoveMeasureArea와 같은 호출). 목표각 = 새 세그먼트 첫 스틱 `m_fStageR + m_fStageRotate`(§9-10 확인: .alg `StageRotate`와 일치해야 함).
   3. **재Align** — Glass SW Align 중첩 실행(`ItemStart(GLASS_SW_ALIGN, true)`, 세그먼트 `m_sAlign` 사용). 실패 시 알람·중단(ICP `EQP_GLASS_ALIGN_FAIL` 대응).
   4. 다음 세그먼트 스틱 루프 진입(첫 스틱 MoveMeasureArea가 측정 위치로 이동).
   같은 방향이면 Safety·회전·Align 모두 생략하고 세그먼트 전환만 한다(ICP와 동일 규칙).
5. 흡착 상태 확인(PLC `STATE_MEASURE` 유지) — 풀려 있으면 알람(자동 재흡착은 하지 않음: `ResetAlign`·안전위치 문제, D-1).
6. 결과 행 base 설정(§5.7), Cell ID 글자 base 설정.
7. 로그: `measure_seq_item_engine` + OPER에 `[HYBRID] Segment n/N name rot=.. PG=.. WAD=..`.

측정 아이템 외 아이템(Recipe/Align/PreMeas)은 **세그먼트 0 기준 1회** 그대로 — 세그먼트 k≥1의 준비는 경계 훅이 전담. 엔진 아이템 루프(Option B: 아이템 1~6을 세그먼트마다 반복)는 ClearData·Adsorption Align 리셋·Resume 저장 충돌로 **채택하지 않는다**.

글라스 종료 후 활성 세그먼트는 N-1로 남는다. 다음 글라스 Recipe 아이템이 `OpenRecipe`로 다시 로드하므로 문제없으나, 그 사이 UI/수동 화면이 보는 값은 N-1 — **종료 시 `SetActiveSegment(0)` 복귀**로 통일.

### 5.5 방향 전환 (D-1 — **A안 확정, 2026-09-28**)

| 안 | 내용 | 장점 | 단점 |
|---|---|---|---|
| **A(확정)** | 세그먼트 종료 → Stage Safety 이동 → Stage R 회전(글라스 흡착 유지) → 재Align → 다음 세그먼트 | 이미 스틱마다 하는 동작, 신규 모션 0, PLC 무변경 | 90° 회전 시 흡착 유지는 실기 확인 |
| B(미채택) | ICP식 M2L(분리)→Loader R→L2M(재흡착)→Align | ICP와 동일 | `ResetAlign`, PLC 상태 전환 왕복, 사이클 타임, 중단 복구 복잡 |

HMI 현행 Long/Short 레시피가 **같은 글라스 흡착 상태에서 R만 90/180**으로 다르다. 절차는 §5.4-4.

### 5.6 외부 레시피 (D-3)

- 세그먼트 헤더의 `m_chPGName/m_chWADName`, 공란 = `GetRecipeID()`(현행 동작) → **N=1 기존 레시피는 동작 불변**.
- 해석 함수 1개 `CRecipeEditMgr::ResolveExtRecipe(eDevice)` → `CommTcpIpObjectPG`의 `szModel`, WAD `stELP.szModel` 두 곳만 교체.
- CIM PPID/`SetRecipeID`/APD `RECIPE_ID`는 **하이브리드 파일명 1개 유지**(ICP와 동일: 세그먼트는 호스트 비가시).

### 5.7 결과 행·Cell ID

- **행 산식 일반화** (AgingTest 반복·MultiPoint 블록과 동시 지원):
  - `segBase_s = Σ(이전 세그먼트 nArea×nStick)`, **글라스 전체 블록 크기 `G = Σ_s nArea_s×nStick_s`**
  - `row = segBase_s + cell + nArea_s×nMeasStick + 1 + G×block`
  - `block` = 지금의 `rep`(AgingTest 종류별 dense 반복 카운터) 또는 MultiPoint 블록 번호 — 기존 의미 그대로
  - 지금 5곳(WAD `:2767/2773`, WAD MultiPoint `:2962-2968` 표준 경로 포함, VISION `:3805`, ANGLE `:5236`)이 `nArea*nMaxStick`(활성 세그먼트 크기)를 블록 크기로 쓰므로 전부 `G`로 교체. `segBase`는 `SetMeasUseInfo`(✔ 단일 깔때기)에서 가산하거나 같은 헬퍼 1개로 계산.
  - **N=1이면 `G = nArea×nMaxStick`, `segBase = 0` → 기존 행 번호와 완전히 동일**(회귀 없음). MES 합의(`Result_Identification_MES_Agreement_Plan`: 반복 블록 = 글라스 전체 셀 수)도 "글라스 전체"로 자연 확장.
  - EFA T0/T1은 같은 (seg, stick, cell)이 같은 행 → **T1이 T0 행을 덮어쓰는 기존 동작 유지**.
  - 상한: `G × 최대 block ≤ AREA_MAX_CNT(2500, CDlgResult.cpp:1944)` — 저장 시 검사.
  - Phase 4에서 `m_nAreaNum`/`nAreaNum`을 `SetMeasUseInfo` 밖에서 쓰는 곳 전수 감사(OST/RTEST/AgingPG/셀상태/Vision 비동기 큐).
- **Cell ID — 1단계 자동 글자 이월(Phase 4)**: `MakeCellID/GenCellID`에 세그먼트 글자 base(Long=셀 수, Short=스틱 수 이월, ICP `BuildSegmentAutoCellIds` 규칙) + bijective base-26(Z 다음 AA). B 패널 번호 오프셋 로직은 무변경. `m_szCellID[16][32]`는 스틱 단위라 영향 없음. N=1이면 base=0 → 기존 ID와 동일.
- **Cell ID — 2단계 A/B 수동 입력 + Manual 편집기(Phase 4b, D-5)**: ICP 모델을 그대로 가져오되 저장 위치만 HMI 원칙(§5.1-3)에 맞춘다.
  - **모델**: 레시피 전역 `ECellIdMode m_eCellIdMode[A/B]`(Auto/Manual) + 셀별 수동 ID `[A/B]`. 둘 다 `S_CellInfo`(preDefine)에 넣지 않고 `C_In/OutRecipeFunc`의 세그먼트 병렬 벡터(`vector<seg>` × 스틱 × 셀 × 패널)에 둔다.
  - **리졸버**(ICP `BuildResolvedCellIds` 대응, 순수 함수): 패널이 Manual이면 수동 ID, **A Manual + B Auto**면 B = A 접두 + (A 번호 + 세그먼트 셀 수(Short)/스틱 수(Long)), 그 외 Auto. `GenCellID`는 GlassID 접두 + **리졸버가 준 셀 토큰** + 체크섬(`gf_MakeMcrID`)으로 조립 — 셀 토큰 생성부만 교체.
  - **패널 판정**: 현행 규칙(GlassID 끝에서 두 번째 문자 `'B'`) 그대로. GlassID 없음/알 수 없음 → A(ICP §4.14와 동일, 측정을 막지 않음).
  - **검증**(ICP `ValidateCellIdConfig` 대응, 순수): 수동 ID 형식 `[A-Z]+[0-9]+`, B 파생 가능성, **전 세그먼트 A+B 합산 중복 0** — **저장 전**에 실행(파일을 열기 전 거부). 측정 시작 시 재검증 실패는 로그 + 자동 시작 거부(D-15 확정, 알람 없음).
  - **파일**: V1011 꼬리 블록에 `_CELL_ID_PANEL_ modeA modeB`(레시피 전역, `_HYBRID_` 뒤) + 세그먼트별 `_CELL_ID_MANUAL_`(스틱마다 A줄·B줄, 셀 수만큼 토큰, 빈 토큰 보존 파서). → **구현(2026-10-01, D4b-1)**: 꼬리 안이 아니라 `_HYBRID_END_` 뒤 트레일러 + `RECIPE_FILE_V1012`(저장 1013) — 구 빌드의 엄격한 꼬리 태그 검사 때문(Phase 4b 절).
  - **리졸버 B 오프셋 (구현 D4b-6)**: 위 "세그먼트 셀 수(Short)/스틱 수(Long)" 대신 HMI `nMaxTop`(rot1 = 스틱 수, 그 외(rot3 포함) = 스틱 0 셀 수) — 씨앗 A = Auto A일 때 파생 B = Auto B가 되도록.
  - **편집기**: DLL 신규 `CCellIdEditDlg`(RecipeMgrDlg의 "CELL ID A/B → EDIT" 행에서 모달). → **구현**: 클래스 `C_CellIdEditDlg`, 진입은 레시피 화면 "CELL ID" 버튼 `IDC_BUTTON_14`(D4b-12). 작업 사본에서 편집 → APPLY가 검증+커밋, CANCEL 폐기. 세그먼트/패널/모드 콤보, 셀 맵(1차는 **세그먼트별 그리드**, 물리 합성맵은 보류), 선택 셀 ID 편집, 일괄 입력(Deg0=행, Deg90=열, 시작 번호 2자리 패딩). 색: 빨강=무효/빈칸, 주황=중복, 연노랑=Auto와 다름.
  - **MakeCell Apply 시 A/B 모드·수동 ID 초기화**(ICP §4.13 — 재생성된 인덱스가 같은 물리 셀이라는 보장 없음).
  - ICP 결함 비이식: 편집기 진입 시 `GetRecipeView()` 전체 호출로 스틱 0 Head Move가 전 스틱에 복사되던 문제 — 편집기는 레시피 그리드를 읽지 않는다.
- **`nCellStatus[100][50]`([셀][스틱])은 표시용이 아니라 측정 제외 판정에 쓰인다**(PTP/OST short-open, AGING_NG, R-Test 초과 → WAD/VISION/ANGLE 제외, `DoWorkGlassMeasureStep.cpp:2469-2510` 등). EFA T1은 OST/RTEST를 건너뛰므로 **T0가 남긴 상태에 의존**한다. 따라서 경계에서 초기화하면 안 되고, **스틱 인덱스에 `segStickBase = Σ(이전 세그먼트 스틱 수)`를 더해** 세그먼트끼리 겹치지 않게 한다(총 스틱 ≤ 50, 저장 시 검사). 초기화는 지금처럼 글라스 시작(`DlgSubOperate::GenGlassPath`) 1회. PG 프로토콜로 보내는 Cell ID 스틱 번호(`CommTcpIpObjectPG.cpp:1702`)와 `m_nAgingStartStick[4]` 슬롯은 세그먼트 로컬 그대로 둔다 — 상태 배열 인덱스만 분리.

### 5.8 Resume (D-6 — **필수 대응, 2026-09-29**)

**현행 HMI Resume** (✔ 코드 확인):
- 저장: `CResumeInfoMgr`(LF-only) `ResumeInfo.ini` — GlassID/RecipeID/LotID/InnerID/**LastStick**/TotalStick/GlsRotate/Running/Completed/시각/ResultPath. 엔진 시작 시 `SaveResumeInfo(-1)`, 스틱마다 `UpdateStickProgress(i)`(`DoWorkGlassMeasure.cpp:361`), 정상 종료 시 Clear.
- 적용: 측정 시작 시 `CDlgSubOperate::CheckAndApplyResumeMode()`(`DlgSubOperate.cpp:3474-3525`) — `ValidateResume(레시피, 스틱 수, 회전)` 실패 시 새로 시작, 통과 시 Yes(재개)/No(새로)/Cancel(시작 취소).
- 결과 재적재: Recipe 아이템 `None`(`DoWorkGlassRecipeStep.cpp:46-150`)이 `RawData\{date}\{glass}\{WAD|ANGLE|VISION}\` 파일을 **파일명(InnerID 시각) 정렬 → i번째 파일 = i번째 셀**로 가정해 `iMeasArea = iCell + nCellsPerStick*nMeasStick + 1` 계산 후 `CResultMgr::Updata_Data`.
- 한계: ① P0-1로 재시작 후 reload 0건 ② 파일 순서 추정은 **AgingTest 반복(셀당 파일 여러 개)·EFA T0/T1(셀당 2개)·마스크/NG로 건너뛴 셀·세그먼트**가 하나라도 있으면 행이 어긋남 ③ pass/seg 소실.

**하이브리드 Resume 설계**:
1. **진행 튜플**: `ResumeInfo.ini`에 `LastPass`, `LastSegment`, `SegmentCount` 추가 → (pass, seg, stick). `UpdateStickProgress(pass, seg, stick)`. 키가 없는 구 파일은 (0, 0, LastStick)로 해석(하위 호환).
2. **검증**: `ValidateResume`이 레시피 ID + 세그먼트 수 + **세그먼트별** 스틱 수·회전을 비교(지금은 활성 세그먼트 스틱 수·회전만 봄). 재개 안내 문구에 `Segment n/N`, EFA면 `T0/T1` 표시.
3. **재개 위치**: (pass, seg) 이전은 건너뜀. 재개 대상 세그먼트는 **경계 훅 전체 수행** — Stage 상태를 알 수 없으므로 방향이 같아도 Safety→회전→재Align 강제(ICP도 재개 시 항상 재Align). 재개 스틱은 **그 스틱 전체 재측정**(ICP 411f449 정책: 이전 시도 결과를 섞지 않음, 대상 추가 없음). 시작 스틱은 재개 대상 (pass, seg)에만 적용 — 현행 EFA T1 중단 시 T0 재수행 문제(P0-5) 해소.
4. **결과 재적재 = 결과 저널 재생** (신규, 정렬 추정 대체):
   - 결과가 그리드에 들어가는 **단일 깔때기** ✔ `CResultMgr::Updata_Data(PRSRITEMDATA)`(`Manager/ResultMgr.cpp:51`, 호출처 WAD/Angle/Vision/Color 4곳)에서, 자동 측정 중이면 `RSRITEMDATA`(모드·`iMeasArea`·결과 파일 경로)를 글라스 결과 폴더의 `ResumeJournal.csv`에 1줄 append(+pass, seg, stick, cell).
   - 재개 시 저널을 순서대로 재생해 `Updata_Data` 재호출 → 세그먼트 `segBase`, AgingTest 반복 블록 `G`, EFA T0→T1 덮어쓰기(마지막 쓰기 우선), 건너뛴 셀이 **측정 당시 행 그대로** 복원된다. 재개 스틱에 해당하는 저널 줄은 재생하지 않는다(그 스틱은 재측정).
   - 저널이 없는 구 글라스 → 현행 정렬 방식 fallback(N=1 전용).
5. **`nCellStatus` 복원**: EFA T1 재개는 T0의 OST/AGING_NG 상태가 필요 → 경계 저장 시 `nCellStatus` 스냅샷을 결과 폴더에 저장하고 재개 시 복원(스냅샷 없으면 T1 재개를 T0부터로 강등 + 경고).
6. **북마크 수명**: 새 글라스(GlassID 변경) 시작 시 튜플 초기화, "No(새로)" 선택 시 **튜플 전부** 초기화(ICP 결함: "First" 선택이 LastSegment를 안 지움 — 비이식), 정상 완료 시 Clear.
7. **P0-1 선결 필수**: 엔진 시작부 `SaveResumeInfo(-1)`을 resume 모드에서 건너뛰도록 수정해야 1~5가 동작.

### 5.9 AgingTest / EFA 동시 지원 (D-4 — **지원으로 확정, 2026-09-29**)

**ICP 참고**: ICP Aging(경시변화)은 교차 코드 없이 동작한다 — 스케줄·셀 마스크(`m_nSelectCell[AGING]`)·완료 플래그·Area가 전부 세그먼트별 `S_RecipeMgr` 안에 있고 `SetActiveSegment`로 통째로 바뀐다. 세그먼트를 넘는 것은 Area 이월 하나뿐. 단 ICP에도 결함이 있다(비이식): ① 비샘플링 세그먼트는 측정 시작 시 Aging 마스크를 전부 켜는데 Area는 그 전에 저장된 마스크로 계산 → Aging 행 충돌 ② 한 세그먼트라도 스케줄이 비면 잡 전체 거부 ③ Aging on/off는 세그먼트별이 아니라 글라스 전역(Work State).

**HMI는 AgingTest·EFA 데이터가 레시피 밖(RecipeID 키 사이드카 `.rcp` INI)에 있어** ICP처럼 "세그먼트 구조체 교체"만으로는 안 된다. 접점별 변경:

| # | 접점 | 문제 (세그먼트 도입 시) | 변경 |
|---|---|---|---|
| A1 | `CAgingCycleInfo` `[MASECELL] STICK_n`(`AgingCycleInfo.cpp:86-103`, 행=스틱 인덱스, 열=셀, bit0 WAD/bit1 VISION/bit2 ANGLE) | 모든 세그먼트가 같은 `STICK_1..`을 읽음. 스틱이 더 많은 세그먼트는 초과 스틱이 **조용히 skip**(`IsStickUse` 범위 밖 false) | 세그먼트별 섹션: 세그먼트 0은 기존 `[MASECELL]`(하위 호환), k≥1은 `[MASECELL_SEG{k+1}]`. `IsCellUse/IsStickUse/GetStickCount/GetCellCount`에 `seg` 인자. 섹션 없음 = **전부 측정**(명시 정책) |
| A2 | `[COUNT]`/`[ITEM_n]` | — | **글라스 공통** 유지. EFA는 전 세그먼트 같은 pass 수 |
| A3 | 스텝 게이트 `IsStickUse(GetMeasureStick())`(WAD `:2104`, VISION `:3060`, ANGLE `:4587`), `IsAgingCellSkip`(`:453-466`) | 세그먼트 인자 없음 | 활성 세그먼트 인덱스를 함께 전달 |
| A4 | AgingTest 게이트 `m_bUseAging_Test`(`MeasureSequenceItemEngine.cpp:819-825`, 스틱마다 활성 레시피 읽고 파일 재로드) | 활성 세그먼트 값에 따라 세그먼트별로 암묵적 on/off | **글라스 시작 시 세그먼트 0 값으로 래치**(ICP와 같이 글라스 전역), 파일은 글라스당 1회 로드. 편집기는 이 플래그를 전 세그먼트 동일하게 강제 |
| A5 | EFA 게이트 `GetEfaPassCount`(`:842-866`)와 글라스 끝 `EFA^1$` 게이트(`DoWorkSequenceCIMStep.cpp:776-795`)가 **각자** 활성 레시피 `m_bUseEFA` 판정 | 시작은 세그먼트 0, 끝은 N-1(또는 복귀한 0)을 봐서 불일치 가능 | 글라스 시작 시 `bEfaGlass/nPassCount` 래치(공유 상태), `EFA^1$`는 래치만 읽음 |
| A6 | 루프 순서 | `EFA^0$`·T0 결과 플러시는 **글라스당 1회**(모든 세그먼트 T0 뒤) | **EFA: pass → seg → stick**. T1 시작 시 세그먼트 N-1 → 0 복귀 경계(방향이 다르면 Safety→회전→재Align 1회 추가, D-13). **AgingTest: 항목 루프가 스틱 안이므로 seg → stick → item 그대로** |
| A7 | 행 산식 블록 크기(§5.7) | 반복·MultiPoint 블록이 세그먼트 크기 기준 → 다음 세그먼트와 충돌 | 글라스 전체 `G`로 교체 |
| A8 | `nCellStatus`(§5.7) | EFA T1이 다른 세그먼트 T0의 OST/AGING_NG 상태를 읽어 잘못 skip | `segStickBase` 오프셋 |
| A9 | Resume(§5.8) | pass·seg 소실 | (pass, seg, stick) 튜플 |
| A10 | Last-Stick 가드(`DoWorkGlassMeasure.cpp:242`) | EFA 3032 재발 위험 | `i+1==nStick(seg) && nPass==nPassCount-1 && seg==nSeg-1` |
| A11 | AGING 스텝(`:6458-6870`) — `m_nAgingStartStick[4]`, 리뷰 스냅 헤드 Y, `m_nCellCnt/m_nStickCnt` | 활성 `S_RecipeMgr`를 읽으므로 **경계에서 전환된 뒤라면 정상** | `SetActiveSegment`가 `m_nStickCnt/m_nCellCnt`까지 교체하는지 확인. Aging 알람 대상 rcp(`LoadTargetRcp(GetRecipeID())`)는 글라스 공통 |
| A12 | AGING_PTN 스텝 | PG 패턴 번호는 **로드된 PG 모델 기준** — 세그먼트별 PG 모델(D-3)이면 같은 PTN 번호가 다른 패턴일 수 있음 | 검증 항목(§9-12). 필요 시 세그먼트별 PTN은 2차 |
| A13 | 수동 다이얼로그 AgingTest/AgingNew 그리드·`MeasCellSave/Load` | 활성 세그먼트 스틱만 표시·저장, 저장 시 다른 세그먼트의 `STICK_n` 덮어씀, 토큰 수 불일치 행은 0으로 저장(2026-09-28 16:12 사고 원인) | 세그먼트 선택기 + 세그먼트 섹션 저장 + 스틱 수 초과 stale 키 삭제. `OnRcpOpen`(`RecipeChange`)은 측정 중 차단 |
| A14 | InnerID 캐시(`SetMeasureArea`, 스틱 실행마다 재할당) | 이미 스틱 단위 | 변경 없음 |
| A15 | WAD Multi Point 파일 `[DATA]/[MASECELL]`(`Manager/WadMultiMeasMgr.cpp`, 레시피당 1개) + 수동 다이얼로그 `IDD_DLG_SUB_MANUAL_MULTI_MEAS` | 모든 세그먼트가 세그먼트 0의 점 리스트(장비 좌표 오프셋, 회전·셀 크기 항 없음)와 마스크 행을 쓴다 → 세그먼트 2는 엉뚱한 위치 측정·스틱 6~21 조용히 base 점만. 다이얼로그는 세그먼트 0만 편집, 측정 중 Open도 막히지 않음 | **Phase 3d**(B1~B23): 세그먼트별 `[DATA_SEG{k+1}]/[MASECELL_SEG{k+1}]`, 스텝이 활성 세그먼트 적재, 용량 P = 세그먼트 최대, 다이얼로그 세그먼트 선택기·가드 |

**EFA×AgingTest 같은 글라스 동시**: 지금도 글라스당 배타(EFA 우선, `DoWorkGlassMeasure.cpp:210-212`) — 그대로 유지.

**단계**: 세그먼트 기본 루프(Phase 3)를 N=1 무회귀로 먼저 넣고, Phase 3b(AgingTest×세그먼트: A1~A4, A7, A8, A11, A13) → Phase 3c(EFA×세그먼트: A5, A6, A10, A12) 순서. **상태(2026-09-30)**: A6·A10은 Phase 3에서 이미 들어가 13:03 OOS로 확인했다. A5(글라스 래치)는 3c에서 구현했다. A12는 3c에서 WARN만 넣었다(세그먼트별 PTN은 2차). A13의 AgingNew 선택기도 3c에서 구현했다. 셋 다 컴파일까지 확인했고 OOS는 대기다. 두 기능 모두 **N=1 동작은 한 줄도 바뀌지 않아야** 한다(행 산식·마스크 섹션·게이트 래치 모두 N=1에서 기존 값과 동일).

### 5.10 UI (DLL 다이얼로그, `dfcRecipe.rc`는 UTF-16 → python)

- **MakeCellDlg**: HYBRID MODE 체크 / SEGMENT COUNT / APPLY + `<` Seg N/M `>` (그리드형, ICP 구조). 체크 해제는 **확인 후** 단일화(I-결함 교정). CAD 행 전체를 세그먼트별로 저장·복원.
- **RecipeMgrDlg**: 세그먼트 콤보+Prev/Next, 이름 변경(유일성은 저장·로드 시에도 검사), 세그먼트 헤더 행(PG/WAD 레시피명), 전환 시 열린 팝업 닫기, 덮어쓰기 Apply 후 **활성 인덱스를 UI와 일치**(I-결함2 교정).
- **SelectCellDlg**: 활성 세그먼트 기준(편집기 선택기와 연동).
- 운전 화면(`DlgSubOperate`): 세그먼트 표시기 `Segment n/N (name)` 추가(ICP엔 없음), ChartOrbit는 활성 세그먼트.

---

## 6. 실행 Phase

### Phase 0 — 결정·선결 (코드 변경은 선결 결함만, 별도 커밋) — ✅ 완료, OOS 확인 (2026-09-29)
- **OOS 결과(2026-09-29 13:05~13:12)**: P0-1 ✅ 재개 시작(13:08:32)에 `Resume Save … Stick=0/5` 없음 → 저장된 LastStick 유지, `starting from stick 2 (total 5)`로 스틱 3부터 재개·완료·`Resume Reset`. 결과 재적재는 OOS에 RawData가 없어 `Resume reload SKIP: path not found` — **실기 확인 항목**. P0-2 ✅ 부팅 후 첫 글라스 스틱 1~5 측정. P0-3 ✅ `HMI.rcp`(V1005) 열기 정상(사용자 확인). 픽스처 ✅ 구 빌드에서 세그먼트 0(Long)만 로드. 알람·모션 WARN 0.
- §8 결정 확정. ✅ D-1~D-15 전부 확정.
- 선행 정리(사용자 지시): MakeCell 각도 콤보 ↔ `m_nGlsRotate` 매칭(4옵션, 스틱 R = rotate×90, StageRotate 오프셋 0), RecipeMgrDlg GLASS ROTATE 변경 시 스틱 R 동기화, GlassInfo `[OCP_TEST_SHORT_GLASS] ROTATE=2`. ✅ 빌드 OK.
- 선결 결함 수정:
  - **P0-1** `DoWorkGlassSequenceEngine::ThreadRun` 시작부 `SaveResumeInfo(-1)`을 `IsResumeMode() != true`일 때만 실행.
  - **P0-2** 아이템 `SetLoopStepIndex(nStick)`을 `Start()` **앞**으로 이동.
  - **P0-3** (확인됨) `D:\HBS\Recipe\Recipe\HMI.rcp`(V1005)는 타임아웃 9개인데 로더가 `GetAt(0..9)` — VS2022 MFC `CStringArray::GetAt`은 범위 초과 시 `AfxThrowInvalidArgException`(`afxcoll.inl:289`). 두 로더 모두 `i < GetSize()` 가드, 모자라는 칸 0.
  - **P0-4** 두 로더에 `m_recipeMgr.m_nVersion = nVersion` (소비처 `GetRcpVer()` 호출 0건 → 동작 영향 없음).
- 픽스처 ✅ `D:\HBS\Recipe\Recipe\9_OCP_TEST_Hybrid_LS_260929.rcp` (스크립트 `make_hybrid_fixture.pl`): `_VERSION_ 1012` + 세그먼트 0 본문(= `1_OCP_TEST_Long_260907` 본문과 **바이트 동일**) + `_HYBRID_ 2` + `_SEGMENT_HEADER_`(Segment#1/#2, PG·WAD 공란) + `_SEGMENT_ 1` + Short 본문(V1009 → `_STICK_CONTACT_OFFSET_` 0×21, `_ANGLE_SUBZ_` 0,0 보강) + `_HYBRID_END_`. 422줄.
  - 주의: Long/Short의 Probe ID가 다름(A3KR1B01P / A3NE1M01P) → **파일 형식·로더 검증 전용**, D-7 검사에서는 저장 거부 대상.
  - **지금 바로 가능한 검증**: 현재 빌드(꼬리 미지원)로 이 파일을 열면 세그먼트 0(Long)만 정상 로드되어야 한다 = §5.3 "구 빌드 호환" 확인.
- OOS 확인 항목: ① 재개 선택 후 "Resume reload: WAD n/n files loaded" 로그가 0이 아닌지(P0-1) ② 첫 글라스 측정 정상(P0-2) ③ `HMI.rcp` 열기 예외 없음(P0-3) ④ 픽스처 로드 = Long과 동일 화면.

### Phase 1 — 데이터 모델 + 파일 포맷 (DLL + 헤더 2사본) — ✅ 완료, 빌드·OOS 확인 (2026-09-29)
- **OOS 결과(14:03 빌드)**: ① ✅ `1_OCP_TEST_Long_260907` → 편집기 Save As `1_OCP_TEST_Long_P1RT` diff = 버전 줄(1011→1012) + 꼬리 7줄(`_HYBRID_ 1 … _HYBRID_END_`)뿐, 기존 142줄·CRLF 동일. ② ✅ 사이클 시작(14:14:58)·수동 레시피 선택(14:16:19) OLOG `[HYBRID] … 2 segments, align load fail 0` / `Segment 1/2 rot=1 stick=5 cell=20 R0=90.0 align=HMI_90_TEST2(OK)` / `Segment 2/2 rot=2 stick=21 cell=5 R0=180.0 align=HMI_180(OK)`, 런타임은 세그먼트 0(`Resume Save … Stick=0/5`, 수동 HeadPair K01~A01 20셀). ③ ✅ Phase 0 때 구 빌드로 확인. ④ ✅ BADTAG 열기 실패(사용자 확인 — 실패 경로는 로그를 남기지 않음). D-7 ✅ 하이브리드 픽스처 저장 거부(사용자 확인). 알람·크래시 0.
- `SegmentDefine.h`, `C_In/OutRecipeFunc` 멤버·API, `LoadRcpSections/SaveRcpSections` 추출, V1011 꼬리, 세그먼트 .alg 로드, `preDefine.h` enum에 `RECIPE_FILE_V1011`.
- 검증: ① 기존 레시피 로드→저장 diff = 버전 줄 + 꼬리 블록만 ② 합성 픽스처 로드 → 세그먼트 1의 스틱 R=180, .alg 분리 ③ 구 빌드로 합성 픽스처 로드 = 세그먼트 0 ④ 태그 손상 파일 → 로드 실패·이전 상태 유지.
- **구현 내역** (스크립트 `p1_patch.pl`):
  - `preDefine.h`(2사본): `RECIPE_FILE_V1011` → END=1012. 꼬리 게이트는 `nVersion > RECIPE_FILE_V1011`.
  - **`SegmentDefine.h`**(신규, 2사본, ASCII): `M_SEGMENT_MAX 8`, `M_SEGMENT_STICK_MAX 50`, `S_SegmentHeader{m_chName[64], m_chPGName[256], m_chWADName[256]}` — 이름 길이는 §5.2 초안의 `CH_LEN` 대신 64/256(복사는 `wcsncpy_s(_TRUNCATE)`).
  - `C_OutRecipeFunc`(2사본)·`C_InRecipeFunc` 클래스 **끝**에 `m_vecSegments / m_vecSegHeader / m_vecSegAlignOK / m_nActiveSegment(-1)` + API: `EnsureSegments, ResetSegments, GetSegmentCount, GetSegmentLoopCount, GetActiveSegmentIndex, SetActiveSegment(n, bWriteBack=TRUE), CommitActiveSegment, GetSegmentMgr(활성=&m_recipeMgr), GetSegmentHeader/Name/Rotate/StickCount/CellCount/StickBase/AlignName, LoadSegmentAlignRcpFiles(반환=실패 수), IsSegmentAlignLoaded`. 범위 밖 = 기본값(I11).
  - `LoadRcpFile`: 본문을 `LoadRcpSections(CStdioFile*, nVersion)`로 **코드 이동**(`&File`→`pFile`만 치환), 로드 후 1세그먼트로 초기화, V1012면 `LoadHybridTail`(태그 5종·세그먼트 인덱스·이름 중복 검증, k≥1 기준값 = 세그먼트 0 사본). **로드 전 상태 백업 + `try/catch(CException*)`** — 태그 불일치든 토큰 부족 예외든 실패하면 전부 되돌리고 FALSE(기존에는 예외가 앱을 죽였다).
  - 저장: `RecipeMgrDlg::SaveFile` 본문을 `C_InRecipeFunc::SaveRcpSections`로 코드 이동(`g_inRcpInfo.` 제거·`&File`→`pFile`), `SaveRcpSegments`가 세그먼트 0 본문 + 꼬리(N=1이어도 항상) 기록. 파일을 열기 전 `ValidateSegments()` — N≥2일 때 D-7(Probe ID·우선순위 세그먼트 0과 동일), 이름 중복, 빈 세그먼트, 스틱 합계 ≤50. 실패 시 영문 경고 후 저장·PPID 갱신 중단.
  - ALPHA `CRecipeEditMgr::RecipeChange`: 기존 `.alg` 로드 뒤 **N≥2일 때만** `LoadSegmentAlignRcpFiles()` + OPER 로그 `[HYBRID] Recipe … : n segments, align load fail k` / `[HYBRID]   Segment i/n [name] rot= stick= cell= R0= align=name(OK|FAIL) PG=[] WAD=[]`. 실패해도 로드는 막지 않음(자동 시작 거부는 Phase 3).
  - 런타임 측정은 여전히 활성 세그먼트(=0)만 — 세그먼트 루프는 Phase 3.
- **Phase 3로 넘긴 주의점**: `SetActiveSegment(bWriteBack)`은 작업 슬롯을 되쓰므로 Adsorption이 `m_sAlign`에 덮어쓴 PROBE_ALIGN `.alg` 값도 함께 되쓰인다(현행 단일 세그먼트 동작과 동일 결과) — 경계 훅에서 m_sAlign 처리 방침 확정 필요. 편집기에서 하이브리드 파일을 연 뒤 MakeCell로 재생성하면 활성 세그먼트(0)만 바뀌고 꼬리 세그먼트는 유지된다(단일화 UI는 Phase 2).
- **검증 픽스처**: `9_OCP_TEST_Hybrid_LS_260929.rcp`(정상, 세그먼트1 `.alg`=HMI_180, 세그먼트0=HMI_90_TEST2), `9_OCP_TEST_Hybrid_BADTAG_260929.rcp`(`_SEGMENT_ 1`→`2` 1바이트 손상 → ④ 로드 실패 기대). 정상 픽스처는 Probe ID가 달라 편집기 저장이 D-7로 **거부되는 것이 정상**.

### Phase 2 — 레시피 편집 UI (DLL) — ✅ 완료, OOS 확인 (2026-09-29)
- **OOS 결과**: MakeCell 하이브리드(Long `OCP_TEST_LONG_GLASS` + Short `OCP_TEST_SHORT_GLASS`, 옵션 "세그먼트 1 복사")로 `1_OCP_TEST_Long_P1RT` 생성·저장(V1012, 326줄, CRLF). 세그먼트 1 = rotate 1 / R 90 / 5×20 / `HMI_90_TEST2`, 세그먼트 2 = rotate 2 / R 180 / 9×12 / `HMI_180`(복사된 `.alg` 이름을 사용자가 변경), Stage Y·헤드 오프셋·CAD 세그먼트별 저장, P5 Probe ID 동일 → D-7 통과, Probe ID 불일치 시 거부 확인, 세그먼트 헤더 PG 입력(`test`) 저장·재로드 유지, 재로드 후 세그먼트 전환 확인(사용자). 남은 차이: Cell ID 글자 중복(세그먼트 1 A~T, 2 I) — Phase 4 이월 대상.
- **주의(Phase 3 전)**: "세그먼트 1 복사"는 `.alg` 이름까지 복사한다 → 새 세그먼트 생성 후 ALIGN RECIPE 확인 필요. PG/WAD 칸 값은 Phase 3에서 실제 장치 모델명으로 송신되므로 시험값(`test`)은 측정 전에 지울 것.
- RecipeMgrDlg 선택기·헤더 행·저장 루프, MakeCellDlg 하이브리드 UI·Apply·보존, SelectCellDlg.
- 검증: MakeCell N=2 생성 → RecipeMgr 옵션 세그먼트별 편집 → 저장 → 재로드 왕복.
- **구현 내역** (스크립트 `p2_patch.pl`, 리소스는 Perl UTF-16LE 디코드/인코드 — 이 PC python 불가):
  - 리소스: `resource.h` `IDC_GRID_SEG_HEAD 10432 / IDC_GRID_SEG_DATA 10433` 신규. 나머지는 기존 범용 ID 재사용(대화상자 내 미사용 확인) — RecipeMgr `IDC_BUTTON_12/13`(`<` `>`)·`IDC_COMBO_1`(세그먼트), MakeCell `IDC_CHECK_1`(HYBRID MODE)·`IDC_COMBO_3`(개수 1~8)·`IDC_BUTTON_6`(APPLY)·`IDC_BUTTON_7/8`(`<` `>`)·`IDC_STATIC_1`(Seg n/N).
  - **RecipeMgrDlg**: 상단 `< [n. 이름] >` 선택기(SAVE AS 오른쪽), 우하단 `SEGMENT INFO.` 그리드(SEGMENT n/N, SEGMENT NAME, PG RECIPE, WAD RECIPE — 공란=RecipeID). 전환 = `GetRecipeView()` flush(입력 검사 실패 시 전환 안 함) → MakeCell/SelectCell/Align 팝업 닫기 → `SetActiveSegment` → 세그먼트 `.alg` 로드 → `SetRecipeView()`. `GetRecipeView`가 세그먼트 헤더도 저장, `SetRecipeView`가 선택기도 갱신. `WritePPIDFile`은 세그먼트 0 기준(CIM에는 파일 1개).
  - **MakeCellDlg**: HYBRID 그룹. 세그먼트마다 입력 슬롯(셀 정보 이름, 좌표/CAD 그리드, OPEN 원본, 각도) 보관·`< >` 전환, 창 열 때 레시피 세그먼트 수로 동기화(N>1이면 체크). 개수 APPLY는 슬롯만 바꾸고(감소는 확인), **RECIPE APPLY가 유일한 커밋**: OPEN 한 슬롯만 재생성, 나머지 기존 세그먼트는 그대로, 새 세그먼트는 OPEN 필수 + 옵션 초기값 **세그먼트 1 복사/기본값** 2택(D-11), 이름 `Segment#n` 중복 회피, `ReplaceSegments`로 한 번에 교체(활성 0). 체크 해제는 **확인 후**, 해제 뒤 RECIPE APPLY는 보고 있던 세그먼트 기준 1세그먼트로 `CollapseToActiveSegment`. 1세그먼트 레시피의 RECIPE APPLY는 기존 동작 그대로(본문은 `BuildRecipeFromGrid`로 코드 이동). CAD APPLY를 OPEN 전에 누르면 `GetAt` 예외로 죽던 기존 결함 가드.
  - `C_InRecipeFunc`: `SetSegmentHeader / ReplaceSegments / CollapseToActiveSegment` 추가, `ValidateSegments`에 AGING TEST·EFA 사용 여부 세그먼트 간 동일 검사(D-14, A4 편집기 강제의 저장 시점 구현).
  - SelectCellDlg: 270°도 90° 배치 사용(표시 전용).
- **후속 수정(15:19 빌드)**: ① D-7 저장 검사가 Probe ID 5슬롯을 전부 비교해, P1~P4에 남은 예전 값(A3KR1B01P / A3NE1M01P) 때문에 P5가 같아도 거부됨 → 런타임 `ProbeCheck`와 같은 슬롯만 비교(`SystemConfig.ini [EQUIP_TYPE] MACHINE_TYPE=OCP`면 P5만, HMI면 P1~P5). ② 하이브리드 팝업 전부 + Resume 팝업 2개(`CheckAndApplyResumeMode`) 한국어화(사용자 요청 — 코드 리터럴도 한글). ③ SEGMENT INFO 그리드 위치는 사용자가 리소스 편집기에서 조정(430,166 / HEAD 그리드 242로 이동).
- **OOS 확인 절차**: ① 기존 1세그먼트 레시피 → MAKE CELL → RECIPE APPLY → 결과가 이전과 같은지(회귀) ② `1_OCP_TEST_Long_P1RT` 열기 → MAKE CELL → HYBRID MODE 체크, COUNT 2 APPLY → Seg 2/2에서 Short 셀 정보 OPEN → CAD APPLY → RECIPE APPLY(Yes=세그먼트 1 복사) → 선택기에 2개, 세그먼트 2 GLASS ROTATE 180·스틱 R 180 ③ 세그먼트 이름/PG/WAD 입력 → SAVE AS → 파일 꼬리 확인(제가 판독) → 다시 열기 왕복 ④ 세그먼트 전환 시 ALIGN 레시피가 세그먼트별로 바뀌는지.

### Phase 3 — 런타임 세그먼트 루프 (ALPHA) — ✅ OOS 1사이클 완주 (2026-09-29 17:26~17:39)
- `DoWorkGlassMeasure` pass→seg→stick 루프(EFA 아닐 때 pass=1)·Last-Stick 가드·`SegmentBoundary(from,to)`, 외부 레시피 재송신(이름 변경 시), 방향 전환(Safety→회전→재Align), 자동 시작 전 .alg 검사, 종료 시 세그먼트 0 복귀.
- 검증(OOS): N=1 레시피 회귀(기존과 로그 동일, AgingTest·EFA 포함), 합성 Long+Short 런 — 세그먼트 경계 로그, R 90→180 회전, Align 재실행, 알람 0, 완료 이벤트 1회.
- **구현 내역** (스크립트 `p3_patch.pl`, 백업 `p3_bak`):
  - `DoWorkGlassMeasure::ThreadRun`: 기존 스틱 루프를 `for (nSeg)`로 감쌈(본문 무변경 + 들여쓰기). 스틱 인덱스는 **세그먼트 로컬**(스텝은 활성 세그먼트 `m_vecStick[GetMeasureStick()]`을 읽으므로 스텝 코드 무수정), 스틱 수 = 활성 세그먼트 `m_vecStick.size()`. Last-Stick 가드 `i+1==nStickCnt && nPass==nPassCount-1 && nSeg==nSegCount-1`. Resume 시작 스틱은 (pass0, seg0)만, 진행 저장도 seg0만(튜플은 Phase 5). 경계 실패 시 SetFail(원인이 이미 알람을 냈으면 0) → 세그먼트 0 복귀 → Idle.
  - **세그먼트 0 복귀(`SegmentRestore0`, 데이터만)**: 마지막 Idle 직전·NG/정지 Exit 3곳·경계 실패·함수 끝 — Result 아이템이 Idle 완료 이벤트 직후 활성 세그먼트의 검사 항목을 읽기 때문(조사 결과, `DoWorkGlassResultStep.cpp:72-101`).
  - **`SegmentBoundary(from,to)`**: ① 정지 확인 ② 글라스 감지 중이면 EQ `MEASURE` 확인(아니면 `PLC_STATE_NOT_MEASURE`) ③ `SetActiveSegment` + 세그먼트 `.alg`를 파일에서 재로드(`OpenAlignRecipe` — 흡착 단계 Probe Align이 `m_sAlign`을 덮어쓴 이력 때문에 벡터 사본 불신, 실패 시 `JOB_RECIPE_NOT_EXIST`) ④ PG/WAD 레시피명이 **바뀔 때만** 재송신(PG = `DoWorkRecipeOpen` + `DoWorkInspectStart(PGEvent_1)` ACK 대기, 글라스 시작과 같은 2단계 / WAD = `DoWorkRecipeOpen`) ⑤ GLASS ROTATE가 다르면 `MoveCmd_SafetyAll()`(측정 Safety 스텝과 같은 경로 — 프로브 언컨택·헤드 Z/Y/X·Stage X 1064.7·R 0°) → Glass Align 오프셋만 0(`SetGlassAlignXYR(0,0,0)`, Probe Align 유지) → `MoveCmdPosA(AXIS_LOADER_R, 첫 스틱 R + 사용자 오프셋)` → `ItemStart("…GLASS_SW_ALIGN", true)`(흡착 단계 Probe Align 중첩 실행 선례, 복귀 후 `SPS_PROCESS` 재설정). ALIGN USE OFF면 재Align 생략. `.alg` StageRotate와 목표 R이 0.5° 넘게 다르면 WARN. 같은 방향이면 ⑤ 생략.
  - R 회전 인터락(2° 넘는 회전은 raw X ≥ `INTERLOCKPOS` 1000) — Safety X 1064.7로 충족. Glass SW Align은 `.alg` StageRotate로 R을 직접 돌리고 정렬한다(HMI_90_TEST2=90°, HMI_180=180°).
  - `CRecipeEditMgr::GetPGRecipeName/GetWADRecipeName`(활성 세그먼트 헤더, 공란 = RecipeID) → `CommTcpIpObjectPG` PGEvent_0 `szModel`, `CommTcpIpObjectWAD` Event_WAD_1 `szModel` 두 곳만 교체. 1세그먼트 레시피는 공란이라 송신값 불변. AgingPG(P1~P4, HMI 전용)는 미변경.
  - Recipe 아이템 `_Change`: 세그먼트 `.alg`가 하나라도 로드 실패면 `JOB_RECIPE_NOT_EXIST`로 자동 시작 거부(ICP I14). `stGlassInfo`(결과 헤더 CELL/STICK MAX)는 세그먼트 0 값 유지.
  - 측정 중 UI 갱신 메시지(`WM_MSG_UPDATE_RECIPE_INFORMATION`)는 **보내지 않는다** — `GenGlassPath`가 `nCellStatus`를 초기화한다(운전 화면 세그먼트 표시는 Phase 6).
- **OOS 1차 결과(2026-09-29 16:13~16:24, `1_OCP_TEST_Long_P1RT`)**: 세그먼트 1(5스틱) 측정 → 경계(16:22:27, Safety 7.5 s → R 180 → ALIGN USE OFF라 재Align 생략, 8.7 s) 정상 → 세그먼트 2 첫 스틱 `MoveMeasureArea`에서 `HEAD BACK LEFT target -1492.668 < MN -916`(3902)으로 정지 → 세그먼트 0 복귀 → Idle. **코드가 아니라 세그먼트 2 좌표 문제.**
  - 원인: GlassInfo `[OCP_TEST_SHORT_GLASS]`의 셀/스틱 수·블록·저장 간격·원점이 다른 제품(`[OCP_TEST_ROTATE_0]`) 값(스틱 3,3,3 / 셀 12 / 블록 167.7776)이고 CAD 값만 단변 제품 → 180°에서 셀 12개 × S 156.7452 = 1724 mm. MakeCell 수식 시뮬레이션(`scratchpad/mc_sim.pl`)으로 P1RT 세그먼트 2와 정답 `2_OCP_TEST_Short_260907`(= `[TEST]` 섹션: 스틱 7,7,7 / 셀 5 / 블록 76.3388)을 모두 **차이 0**으로 재현 → **MakeCell 구조는 정상, 셀 정보 데이터 오류**. 09-25 커밋에도 같은 값(오늘 ROTATE 0→2는 스왑 판정 무관).
  - 조치(2026-09-29): `Config`·`Config_` GlassInfo `[OCP_TEST_SHORT_GLASS]` 형상 12키를 `[TEST]` 값으로(ROTATE=2, PROBEID 유지). 재발 방지 ① DLL `C_InRecipeFunc::CheckRecipeLayout` — 셀 X ± 셀크기X/2, 스틱 글라스 좌표(StageY + CONTACT GAP) ± 셀크기Y/2가 글라스 반폭+1 mm 안(셀 방향 = rotate 90/270이면 1500, 0/180이면 925). MakeCell RECIPE APPLY(단일·하이브리드, 실패 시 되돌림 + 한글 팝업)와 하이브리드 저장 `ValidateSegments`에서 호출. ② ALPHA `DoWorkGlassRecipeStep_Change`의 `.alg` 검사 뒤 `CheckMeasureTargetSWLimit` — 모든 세그먼트·스틱의 Stage X / R / 헤드 Y1·Y2 리뷰 목표(`MoveMeasureArea`와 같은 식)를 `GetMax/MinSWLimit`과 비교, 위반 시 3901/3902로 자동 시작 거부(모션 전). 오프라인 재생(`guard_sim.pl`): Long·P1RT 세그먼트 1·0_OCP_TEST 통과, P1RT 세그먼트 2 두 검사 모두 거부.
  - **별도 발견**: 정답 단변 레시피도 스틱 20·21 Stage X 목표 -651.340 / -719.521이 **Stage X SW 리밋 MN -595.5** 밖 — 09-07 16:57:10 실제로 `STAGE X -651.340 RET 3902`로 정지한 기록 있음. 21스틱 단변은 현재 리밋으로 끝까지 못 잰다 → 사전 검사가 시작 시 거부함. Stage X 하한 확장(HW 이동 범위 확인) 또는 레시피 조정은 사용자 결정. 사전 검사는 리뷰 이동만 대상 — 측정 스텝(WAD/VISION/ANGLE)의 헤드 이동(예: 09-28 `HEAD FRONT RIGHT 720.158 > MX 697`)은 미포함.
- **스택 오버플로 수정(2026-09-29)**: 재발 방지 검사 추가 뒤 MakeCell 하이브리드 RECIPE APPLY 확인창에서 정지(콜스택 `ApplyHybridRecipe:1170 AfxMessageBox` → UI 타이머 `CDlgMainForm::OnTimer` → `CMCCLogMgr::UpdateSensor:209`). `S_RecipeMgr`는 고정 문자 배열만 약 88 KB라 `ApplyHybridRecipe`(작업 사본·삼항 임시 객체 4~5개)와 `OnRecipeUpdate`(되돌림 사본)가 약 0.5 MB, 확인창 중첩 루프에서 불린 `UpdateSensor`가 `VTM_FOR_FDC`(160,016 B)+`RDATA[1000]`을 스택에 올려 1 MB 메인 스택 초과. 수정(`p3d_stack.pl`): MakeCell 사본은 `unique_ptr`/벡터 원소에서 직접 생성, In/Out `LoadRcpFile` 백업·`LoadHybridTail` 세그먼트 0·`SaveRcpSegments` 작업 슬롯도 힙, `UpdateSensor`는 공유메모리 `stFDCWData`만 직접 복사(160 KB 절감). **규칙: `S_RecipeMgr`를 스택 변수·삼항 임시 객체로 만들지 말 것.**
- **OOS 3721 (2026-09-29 17:01)**: 세그먼트 2 스틱 1 WAD `MoveCmdPosWAD SubZ DOWN NG - PROBE UNIT(X9:0.000) vs BACK GANTRY(X2:25.555)` — MakeCell RECIPE APPLY가 REVIEW Y를 CONTACT OFFSET(27.5549)으로 다시 써서 WAD X2 목표(REVIEW Y + Stage Y 오프셋 -2)가 P5 겹침 구간 [1,299]에 들어감(09-25 이전부터의 동작, 16:13 `X2:84.515`도 같은 원인). 사용자가 REVIEW Y 0으로 수정. 재발 방지(기존 세그먼트 REVIEW Y 보존 / OCP 0 / 사전 검사)는 결정 대기.
- **OOS 1사이클 완주 (2026-09-29 17:26~17:39)**: 알람·모션 WARN 0, 사전 검사 통과, 경계 7.2 s, 세그먼트 2 21스틱×5셀 전부 측정(WAD 셀 ID 105개 중복 0), `restore segment 1` → Idle → Result → Unload. 세그먼트 1은 사용자 의도로 검사 항목이 Pre/Elect만이라 측정 생략. 두 가지 발견:
  - **OCP 미접촉 OST (기존 결함, 수정)**: MoveMeasureArea(스틱 이동)·Probe_Contact(접촉)가 광학 6항목만 보는데 OST는 Elect만 봄 → 스테이지가 스틱에 가지 않고 미접촉 OST. `NeedOcpElectContact`(OCP + Elect/RTest/AgingTest_2 또는 AgingCycle/EFA 활성) 추가, 두 스텝 모두 적용 + 스킵 로그(`p3e_ostcontact.pl`, 컴파일 OK, 링크·OOS 대기). HMI 모드 무변경.
  - **결과 저장 생략**: Result/Unload 게이트가 세그먼트 0 항목만 봄 → Phase 4 "결과 저장 게이트" 항목.
  - 참고: 스틱당 시간 33 → 41 s 점증(전 단계 균일) — 하이브리드 무관 추정, 긴 런 기준 기록 없음.
- **Phase 3b/3c로 남긴 것**: AgingTest 마스크 세그먼트 섹션(A1) — 지금은 세그먼트 k도 `[MASECELL] STICK_n`을 같은 인덱스로 읽는다. `nCellStatus` segStickBase 오프셋(A8) — 세그먼트 간 셀 상태 인덱스가 겹친다. 결과 행 base/G(Phase 4) — 세그먼트 1 이후 결과 행이 세그먼트 0 행을 덮어쓴다. EFA 래치(A5). 결과 Result 아이템은 세그먼트 0 검사 항목으로 저장 판단.

### Phase 3b — AgingTest × 세그먼트 — ✅ OOS 확인 (2026-09-30: 하이브리드 마스크·래치·용량, N=1 EFA 회귀, 차단 팝업) — 남은 확인: G 블록(활성 항목 2개+), 운전 화면, 비-EFA AgingTest N=1
- 마스크 세그먼트 섹션(A1, A3), 게이트 래치(A4), 행 블록 `G`(A7), `nCellStatus` 오프셋(A8), AGING 스텝 확인(A11), 수동 다이얼로그 세그먼트 선택기(A13).
- 설계: 워크플로(매퍼 5 → 설계 1 → 반박 검토 2)로 118개 지점 조사, 명세 `scratchpad/p3b_spec.md`. 적용 스크립트 `p3b_patch.pl`(21개 파일 원자 적용, 백업 `p3b_bak`).
- **구현 내역**:
  - **A1** `CAgingCycleInfo`: 마스크를 `[seg][stick][cell]`로 저장, `Load(rcp, bEfa, nSegCount = 1)`이 세그먼트 0 = `[MASECELL]`(무변경), k≥1 = `[MASECELL_SEG{k+1}]`을 한 번에 읽는다. 섹션 없음 = 행 0 = 전부 측정(기존 "마스크 없는 파일"과 같은 경로). 접근자 `GetStickCount/GetCellCount/IsCellUse/IsStickUse`에 `nSeg` 인자(기본값 없음 - 누락 호출은 컴파일 에러). `GetMaskSection(k)` 정적 함수가 엔진·다이얼로그 공통 이름 출처.
  - **A3** 스텝 게이트 3곳(WAD/VISION/ANGLE)과 `IsAgingCellSkip`이 `GetAgingMaskSeg()` = 활성 세그먼트를 넘긴다.
  - **A4** 엔진 `AgingGlassLatch()`: `ThreadRun`이 pass/seg 루프 직전 1회 호출 → N>1이면 세그먼트 0의 `m_bUseAging_Test`를 래치(OPER 로그 `[HYBRID] AgingTest flag latched from segment 1 for this glass: n`, 엔진 로그 파일). N=1은 -1(래치 없음)로 기존 식 그대로. aging rcp는 **기존처럼 스틱마다 재로드**(D1 기본안 - N=1의 "측정 중 편집이 다음 스틱부터" 의미 보존, 하이브리드 측정 중엔 다이얼로그 저장이 막혀 사실상 1회 로드). 저장 시 세그먼트 간 플래그 동일은 이미 `ValidateSegments`가 강제 - 래치는 미검증 구 파일 방어.
  - **A7** 행 블록 7곳(`nArea * nMaxStick`: WAD aging 행, WAD MultiPoint 4곳, VISION/ANGLE aging 행) → `CRecipeEditMgr::GetGlassRowBlock(nArea, nMaxStick)` = Σ 셀×스틱(N=1이면 `nArea*nMaxStick` 그대로). **segBase는 Phase 4** — 3b 뒤에도 같은 블록 안에서는 세그먼트 k 행이 세그먼트 0 행을 덮어쓴다(블록 간 충돌만 해소).
  - **A8** `nCellStatus` 열 = `ToCellStatusStick(로컬 스틱)` = 로컬 + 활성 세그먼트 스틱 base. 스텝 15곳 + PG 수신 6곳(슬롯 `m_nAgingStartStick[4]`). base는 **`CRecipeEditMgr`의 int 캐시**(`SyncCellStatusStickBase`: RecipeChange·`SegmentSwitchData`에서 갱신) — PG 수신 스레드·UI 타이머가 `SetActiveSegment`의 벡터 값 복사 중에 벡터를 읽지 않도록(검토 지적 반영). PG 프로토콜 Cell ID 스틱 번호(`+ 1` 줄)와 슬롯 값은 세그먼트 로컬 그대로. 운전 화면 `DlgSubOperate::UpdateCellStatus`는 **base를 더하지 않는다** — 셀 레이아웃 `m_aryCellGroup`은 글라스 시작 때 세그먼트 0으로 한 번 만들어지므로(GenGlassPath), 세그먼트 k 진행 중에는 세그먼트 0의 마지막 상태를 보여 준다(오래됐지만 틀리지 않음). 세그먼트별 표시는 Phase 6(레이아웃 재생성 + base). 한때 base를 더했다가 diff 검토에서 "세그먼트 k 상태가 세그먼트 0 레이아웃의 엉뚱한 셀에 그려지고 열 49를 넘을 수 있음"(major)으로 확인돼 되돌림.
  - **용량 검사**: DLL `ValidateSegments`에 G ≤ `M_RESULT_AREA_MAX`(2500, `SegmentDefine.h` 두 사본) 저장 거부(한글 팝업). ALPHA `DoWorkGlassRecipeStep_Change`의 `.alg` 검사 블록에 `CheckHybridCapacity` — 총 스틱 ≤ 50, `G × max(K_wad×P, K_vis, K_ang)` ≤ 2500(K = AgingTest 종류별 항목 수, P = WAD Multi Point 수), 초과 시 WARN + `JOB_RECIPE_NOT_EXIST`로 자동 시작 거부(D2 기본안), 통과 시 SYST `[HYBRID] capacity: ...` 1줄.
  - **A13** AgingTest 다이얼로그: `IDC_COMBO_AGING_SEG`(21006)/`IDC_STATIC_AGING_SEG`(21007) 숨김 컨트롤을 N>1일 때만 표시. 그리드는 `GetSegmentMgr(m_nViewSeg)` 크기(SetActiveSegment 호출 없음), 저장·불러오기는 해당 세그먼트 섹션, 세그먼트 스틱 수 초과 stale 키 삭제, 셀 수 불일치 행 경고(로드)·덮어쓰기 확인(저장), k≥1 섹션 없음은 전부 체크로 표시(D4 기본안). `MeasCellSave`가 BOOL을 반환 — 세그먼트 전환 중 저장 취소 시 전환도 취소, LOAD 뒤 레시피가 바뀌었으면 하이브리드 저장 거부(검토 지적 반영). 하이브리드 측정·사이클 중에는 AgingTest(열기/저장/불러오기/START/세그먼트 전환)·AgingNew·**AutoContact 레시피 열기**를 막는다(RecipeChange가 측정 스레드 밑에서 세그먼트를 리셋하므로). 전부 N>1 조건 — N=1 무변경.
  - **A11** 코드 변경 없음: `SetActiveSegment`가 `S_RecipeMgr` 전체(`m_nStickCnt/m_nCellCnt` 포함)를 값 복사하므로 AGING 스텝은 경계 전환 뒤 활성 세그먼트를 정상으로 읽는다.
- **적용 diff 검토(2026-09-29, 3관점 + 반박 검증, `p3b_fix1.pl`로 반영, 20:12 빌드 OK)**: ① 운전 화면 base 되돌림(위) ② AgingTest `LoadAgingInfo`가 그리드를 다시 만들기 전에 레시피 표식을 찍던 문제 → 그리드 재생성 직전에 찍고, AgingRcp가 없는 하이브리드 레시피도 자기 크기로 그리드를 만든다 ③ 레시피 변경 확인(`IsMaskViewStale`)을 `SaveAgingInfo`의 `[COUNT]/[ITEM_n]` 쓰기 **전**에 수행(부분 저장 방지) ④ 레시피가 다이얼로그 밖에서 1세그먼트로 바뀌면 500 ms 타이머가 세그먼트 콤보를 숨김. 1세그먼트 회귀 관점 지적은 ④(콤보가 남아 보이는 표시 문제)뿐이었고 나머지는 모두 하이브리드 한정.
- **OOS 1차 (2026-09-30 09:34~09:42, `9_OCP_TEST_Hybrid_LS_260929` + AgingRcp 항목 3개 중 활성 1개 = WAD)**: 1사이클 완주, 모션 WARN/ERR 0, 결과 파일 `RESULT\DATA\30\_20260930-094232.xls` 생성.
  - 용량 로그 `[HYBRID] capacity: sticks 26/50, result rows 205/2500 (G 205 x block 1; K wad/vis/ang 1/1/1, WAD points 1)`, 래치 로그 `[HYBRID] AgingTest flag latched from segment 1 for this glass: 1`, AgingCycle begin/end 26회(세그먼트 1 스틱 5 + 세그먼트 2 스틱 21).
  - **세그먼트별 마스크 적용 확인**: 세그먼트 1 `[MASECELL]`(스틱 1 = 셀 1·3·5, 스틱 4 = 셀 2·4·6·15·17·19, 스틱 5 = 셀 14·16·18·20)과 `AgingCycle row` 로그의 WAD 행이 전부 일치. 세그먼트 2 `[MASECELL_SEG2]`(다이얼로그 세그먼트 콤보로 09:29 저장, 21행 × 5셀 전부 7) → 스틱마다 5셀 전부 측정. 3b 이전이었다면 세그먼트 2 스틱 6~21은 세그먼트 1 행 수(5) 밖이라 조용히 skip됐을 부분. WAD 전송 124건 = 19 + 105.
  - VISION 26회는 활성 VISION 항목이 없어 즉시 종료(정상). 세그먼트 1 스틱마다 접촉 수행.
  - **Phase 4로 확인된 것**: 셀 ID 19개가 세그먼트 간 중복(세그먼트 1의 `A018 C010 … T051`이 세그먼트 2에서 다시 나옴 — Cell ID 자동 글자 이월 미구현), 결과 행도 세그먼트 1·2가 같은 1..105 영역을 공유(segBase 미구현). 둘 다 계획대로 Phase 4.
  - **이번 런으로 검증되지 않은 것**: 활성 항목이 1개라 repeat가 항상 0 → G 블록 간격(A7)은 미검증(항목 2개 이상 활성 필요). WAD Multi Point 없음(P=1). 운전 화면 표시는 로그로 확인 불가.
  - **측정 중 레시피 열기 차단 팝업: 사용자 확인 (2026-09-30).**
- **OOS N=1 회귀 (2026-09-30 10:08~10:13, `1_OCP_TEST_Long_260907` 5×20 + EFA 2패스)**: 완주, 모션 WARN/ERR 0, `[HYBRID]` 용량·래치 로그 없음(1세그먼트 경로). EFA rcp `[MASECELL]`(스틱 1~3 전부, 4~5 없음)대로 T0·T1 모두 스틱 1~3만 10쌍씩 측정, T1은 T0 행 덮어쓰기, T1 OST/R-TEST/패턴 서치 skip, T0 결과 플러시(`_20260930-101137.xls`) + 최종 결과(`_101311.xls`). **행 번호가 3b 이전 빌드(09-28 16:20/16:39 런)와 동일**(예: 스틱 1 `row(31,21)…`, T1 스틱 2 `row(59,49)`, `row(60,50)`). 비-EFA AgingTest 사이클의 N=1 경로(래치 -1 → 레시피 플래그)는 이 런에서 타지 않음.
- **OOS N=1 AgingTest (2026-09-30 10:29~10:34, `1_OCP_TEST_Long_260907` 5×20, 비-EFA)**: 완주. 사용자 지적 "K~T 화질 전부 측정" → **의도와 다름, 3b 회귀 아님(기존 결함)**. AgingRcp가 10:29:21에 처음 생성됐는데 `[MASECELL]`이 스틱 5행 × **10칸**(값 7/0만)이었다. 원인: AgingRcp가 없으면 `LoadAgingInfo`가 1세그먼트 레시피의 셀 맵을 만들지 않아(3b 보정은 하이브리드만) 다이얼로그 기본 그리드(`InitMySpread_AGINGTEST_MEAS` 10열 × 5행, 시트 1장 → 세 비트가 같은 칸에서 읽혀 7/0)가 그대로 저장됐다. 엔진은 행 길이 밖 셀 = 측정(`IsAgingCellSkip`)이라 A~J는 마스크대로, K~T(헤드 A 쪽 11~20)는 전부 측정. WAD도 같았다. **수정(`agmask_fix.pl`, 백업 `agmask_bak`, 컴파일 OK)**: ① AgingTest/AgingNew(EFA) 다이얼로그 — rcp가 없어도 모든 레시피가 자기 셀 맵(3시트)을 만든다, 저장 전 그리드 레시피 표식·크기 ≠ 레시피면 거부(기본 10×5 그리드 저장 차단, `IsMaskViewStale` 1세그먼트 확장), 칸 수 불일치 행 경고·덮어쓰기 확인(`ConfirmMaskOverwrite`)을 1세그먼트에도 ② 엔진 `CheckAgingMaskShape` — 마스크 행 수·칸 수가 레시피와 다르면 글라스·세그먼트당 1회 WARN `AgingCycle: mask [MASECELL] does not fit the recipe - rows r / sticks s, cell count differs on n row(s), stick(mask/recipe) ...`(측정 규칙은 그대로: 행 밖 셀 = 측정, 행 없는 스틱 = 생략). 기존 10칸 파일은 LOAD 시 경고 후 20칸으로 다시 체크·저장해야 한다.
- **수정 검증 OOS (2026-09-30 13:47~13:52, ALPHA.exe 13:29 빌드 — 오버레이·3027 사전 검사 포함, `1_OCP_TEST_Long_260907` 비-EFA AgingTest)**: 13:47:31 LOAD 시 경고 `AgingRcp MASECELL: 5 row(s) with cell count != 20 shown unchecked` → 13:48:45 5행 × 20칸으로 다시 저장(덮어쓰기 확인은 로그가 없어 저장 성공으로 추정) → 런 완주, 모션·PG WARN/ERR/인터락 0, `[PRECHECK] measure targets within SW limits`, 셀맵 오버레이 로그 없음(1세그먼트 게이트), `does not fit` WARN 없음(마스크가 맞으므로 정상 — WARN 경로 자체는 미검증). 활성 항목 ITEM_1(WAD, 3초)·ITEM_2(VISION, 5초), ITEM_3(ANGLE)은 USE_AGING 꺼짐. **측정 셀이 마스크와 5스틱 모두 정확히 일치**: 스틱 1 WAD A·C·E·G·I·K·M·O·Q·S(`row(11,1)…row(19,9)` 5쌍) / VISION K~T(행 11~20), 스틱 2 WAD B·D·…·T(`row(32,22)…row(40,30)`) / VISION A~J(행 21~30), 스틱 3(ANGLE 비트만)·스틱 4(전부 0)는 측정 0, 스틱 5 WAD 20셀(`row(91,81)…row(100,90)` 10쌍) / VISION 0. 합계 WAD 40 + VISION 20 = 60셀. OOS APD·xls는 이 런도 빈 템플릿(09-21 이후 모든 OOS 런이 동일 — 결과 파일로는 검증 불가).
  - **OCP와 다른 점(기존 AgingCycle 구조, 3b 무관)**: OCP `DoWorkStartMeasOCP_A`는 스틱마다 항목별 측정 그룹을 만들고 `nGroup < 1`이면 그 항목의 접촉 이동·패턴·측정을 건너뛴다(`JOB_Proc.cpp:12876-12877`). HMI는 측정 스텝만 `IsStickUse`로 건너뛰고 이동·접촉·OST·R-TEST·항목별 AGING 패턴·스냅은 그대로 수행 → 이번 런에서 스틱 3(25.7초)·스틱 4(26.1초)·스틱 5 ITEM_2(10.9초) 약 63초가 측정 없이 소요(MEASURE 211초의 약 30%).
  - **수정 — OCP식 셀 없는 스틱·항목 건너뛰기 (2026-09-30 사용자 지시, `scratchpad/skipcell_patch.pl`, 백업 `skipcell_bak`, ALPHA 컴파일 OK·빌드·OOS 대기)**:
    - 엔진 `MeasureSequenceItemEngine`
      - `AgingCycleBegin(nEfaPass, nStick)`
      - `AgingItemHasCell(i)`: 세 측정 스텝 게이트와 같은 식이다(항목 플래그 × 활성 세그먼트 `IsStickUse`, 마스크 섹션 없음 = 전부). 그래서 건너뛴 항목은 원래도 세 스텝이 모두 skip됐을 항목이고, 결과 행·repeat는 그대로다.
      - `AgingCycleNextItem`은 셀 없는 항목을 넘긴다. AGING 패턴·스냅도 돌지 않는다.
      - USE_AGING 항목은 있는데 이 스틱에 셀이 하나도 없으면 `IsAgingStickSkip()`이 켜지고 로그 `AgingCycle: stick n skipped - no cell for the active items in [MASECELL…]`를 남긴다.
      - EFA pass 항목도 같다: `EFA: pass p/P stick n skipped …`, OCP_EFA `nGroup < 1`.
      - USE_AGING 항목이 하나도 없으면 기존대로 표준 시퀀스다.
    - `DoWorkGlassMeasure::ThreadRun`
      - 건너뛴 스틱은 init 스텝(플래그·공정 상태)만 수행하고 다음 스틱으로 간다.
      - 글라스 마지막 스틱이면 Exit(Idle, 완료 이벤트 1회)로 간다.
      - Stop이 걸려 있으면 기존 경로를 탄다.
      - MCC `MCC_A_STEP_MEAS_N`은 내보내지 않는다(OCP도 없음).
      - Resume 진행 저장은 그대로다.
    - **OCP와 일부러 다르게 한 점**: OCP는 접촉 이동·사전 검사가 `AgingCnt == 0`(첫 목록 항목)에 묶여 있어, 첫 항목만 셀이 없으면 그 스틱은 접촉 없이 다음 항목을 측정한다. HMI는 스틱마다 셀 있는 첫 항목 앞에서 사전 검사를 한 번 수행한다.
    - **동작 변화**: USE_AGING=1인데 측정 플래그가 하나도 없는 "패턴만" 항목도 건너뛴다(OCP와 같음). 이전 HMI는 패턴만 점등했다.
    - 기대: 13:49 런을 같은 마스크로 다시 돌리면 스틱 3·4는 init만 수행하고, 스틱 5는 ITEM_1만 수행한다. 약 63초 단축.
- **기본안으로 진행한 결정(사용자 변경 가능)**: D1 래치만(파일은 스틱마다 재로드), D2 용량 초과 = 자동 시작 거부, D3 N=1 기존 다이얼로그 결함(측정 중 RecipeChange·불일치 행 0 저장·stale 키)은 3b OOS 뒤 별도 버그 수정(2026-09-30 일부 해소 — 위 N=1 AgingTest 항목: 기본 그리드 저장·불일치 행 경고·stale 저장 거부), D4 k≥1 섹션 없음 = 전부 체크 표시, D6 AgingNew(EFA) 세그먼트 선택기 = Phase 3c, D7 WAD Multi Point `[MASECELL]` 세그먼트 충돌 = A15 → **Phase 3d**(사용자 지시 2026-09-30, 아래 Phase 3d 절 — `IDD_DLG_SUB_MANUAL_MULTI_MEAS` 포함). 결과 행 겹침이 남으므로 하이브리드 + WAD Multi Point 세그먼트 k≥1 운용 금지는 Phase 4 OOS까지 유지(D3d-8).
- **검증(OOS)**: ① N=1 회귀 — 단일 레시피(AgingTest on/off) 로그·행 기존과 동일, `[HYBRID]` 래치/용량 로그 없음 ② 하이브리드 + AgingTest(두 세그먼트 `m_bUseAging_Test`=1, AgingTest 다이얼로그 세그먼트 콤보로 `[MASECELL]`/`[MASECELL_SEG2]` 작성) — 래치 로그, 용량 로그(P1RT: 스틱 26/50, G = 5×20 + 21×5 = 205), 항목 루프가 두 세그먼트에서 수행, 마스크 0 스틱 skip, 반복 행이 G 간격 ③ 운전 화면은 세그먼트 2 진행 중 세그먼트 1의 마지막 상태 유지(깨진 표시·예외 없음) ④ 하이브리드 측정 중 AgingTest/AgingNew/AutoContact 레시피 열기 차단 팝업. **"세그먼트 간 행 충돌 0"은 Phase 4 기준으로 이동**(segBase).

### Phase 3c — EFA × 세그먼트 + 세그먼트 구분 구조 — ✅ 구현·빌드, OOS 1차 OK (15:54~16:02 하이브리드 EFA), N=1 회귀 OK (16:13 EFA, 16:22 AgingTest) — 남은 확인: 화면 START 경로(마스크 보고·섹션 없음 확인·레시피 불일치 거부, 빌드 후), 실기 `EFA^1$`

**OOS 1차 (2026-09-30 15:54:57~16:02:56, `9_OCP_TEST_Hybrid_Half_260930`, 하이브리드 2세그먼트 + 레시피 EFA 2패스, Operate 시작)**

- **빌드:** ALPHA.exe 15:50:37. 링크 PDB에 `m_nSegLostTick`이 있어 S2 수정(F1)까지 들어간 exe임을 확인했다.
- **AgingNew 편집(15:53:21~15:54:27)**
  - seg1 → seg2 전환 시 `[MASECELL]`을 먼저 저장했다.
  - `[MASECELL_SEG2] not found - shown all checked`
  - `[MASECELL_SEG2] saved 5 stick(s) x 4 cell(s)`: 5회
  - 반대 방향 전환 시에도 먼저 저장했다.
  - 결과 파일: `[MASECELL]` 5/1/2 교대(3셀), `[MASECELL_SEG2]` 6·5 교대 + 가장자리 셀 ANGLE(4셀).
- **런 중 콤보:** 비활성화와 종료 후 재활성화는 화면에서 확인했다(사용자). 로그 쪽 증거: 런 중 `no longer hybrid` 숨김 줄 0건, 선택기 조작 로그 0건.
- **사전 검사**
  - `[HYBRID] capacity: ... (G 35 x block 1; K wad/vis/ang 1/1/1, WAD points 1) (EFA 2 passes: K from the EFA rcp, T1 overwrites T0)`
  - `EFA mask rows per segment: seg1 [MASECELL] 5/5 seg2 [MASECELL_SEG2] 5/5`
  - A12·AgingTest_2 WARN은 0건이다(두 세그먼트 PG 모델이 같다).
- **래치:** `[HYBRID] EFA latched for this glass: passes 2`. `MeasDone`은 오프라인이라 실행되지 않았다(실기 검증 대상).
- **태그**
  - 스틱 헤더 20줄(T1 skip 스틱 3개 포함).
  - 행 로그 51줄이 전부 태그됐다(seg1 `rowbase(0)` 21, seg2 `rowbase(15)` 30). 태그 없는 행 로그는 0이다.
  - Contact와 AGING_PTN(` pg(...)`)도 태그됐다.
  - 엔진 begin/end 17쌍과 skip 3줄이 전부 태그됐다. 마지막 `AgingCycle: end seg(2/2)`는 `restore segment 1` 뒤에 찍혔다(캐시 태그 정상).
- **마스크 적용**
  - T0: 스틱별 WAD/VISION 교대, 셀 수 3/4가 마스크와 일치한다.
  - T1 ANGLE: seg1은 스틱 1·2만 측정하고 3~5는 `EFA: pass 2/2 stick N skipped - no cell for the item in [MASECELL] seg(1/2)`로 skip했다(스틱당 약 30 ms). seg2는 셀 1·4만(스틱당 2행) 측정했다.
- **tact:** `T0_Seg1_Stick1..5`, `T0_Seg2_Stick1..5`, `T1_Seg1_Stick1..2`, `T1_Seg2_Stick1..5`. 접두 없는 `Stick%d_`는 0건이다.
- **결과·경고**
  - T0 플러시 `_20260930-160005`와 최종 `_160249`가 나왔다. APD가 비어 있는 것은 이전 OOS 런과 같다(597 B, 오프라인).
  - WARN은 세그먼트 로컬 행 경고 1줄과 `InnerID duplicate remapped` 2줄(무해)뿐이다. 알람·인터락은 0건이다.

**OOS 2차 — N=1 회귀 (2026-09-30, `1_OCP_TEST_Long_260907`, 둘 다 Operate CYCLE 시작)**

- **EFA 16:13:07~16:18:12** (레시피 EFA 2패스). EFA rcp는 16:13:00에 수정됐다: ITEM_1 WAD+VISION+ANGLE, ITEM_2 WAD. 마스크는 스틱 1~3 WAD, 4~5 VISION, 끝 셀 ANGLE.
  - `[HYBRID]`·` seg(`·`rowbase(` 0건. 래치 줄(`EFA latched`, `AgingTest flag latched`)도 없다(1세그먼트 조기 return).
  - tact는 `Stick%d_`만 있다(스틱 1~3 17개, 4~5 10개).
  - 10:08 기준 런과 줄 형태를 비교했다. 새로 나온 형태는 셀 없는 스틱 skip 줄(14:10 패치)과 ANGLE/VISION 행 줄뿐이다. 행 줄은 같은 `LogAgingRow` 형식이고, rcp 변경으로 새로 측정된 종류다. 빠진 형태는 `AGING End (EFA item USE_AGING=0 - skip)` 하나다(이제 두 항목 모두 USE_AGING=1).
  - 측정은 마스크와 일치한다. T0: WAD 20셀×3, VISION 20셀×2, ANGLE 셀 1·20×5. T1 WAD: 스틱 1~3 측정, 4~5 skip.
  - RECIPE_CHANGE·PRE_MEAS·RESULT·UNLOAD 로그는 기준 런과 diff 0이다.
- **AgingTest 16:22:59~16:25:48** (AgingRcp는 13:48:45 이후 무변경, 13:49 기준 런과 같은 조건)
  - 행 줄 40개가 기준 런과 같다.
  - 차이는 셀 없는 스틱 skip 패치의 예정된 동작 변화뿐이다.
    - 스틱 3(ITEM_3 ANGLE만 있고 USE_AGING 아님)과 스틱 4(마스크 전부 0)가 `AgingCycle: stick N skipped`로 넘어갔다. 기준 런은 컨택·OST·RTEST·AGING·PTN을 2회 돌렸다.
    - 스틱 5의 item 2(VISION 셀 없음) 패턴만 점등하던 동작도 빠졌다.
    - 런 시간은 3:46에서 2:49로 줄었다.
  - RECIPE_CHANGE·PRE_MEAS·RESULT·UNLOAD 로그는 diff 0이고 tact는 `Stick1/2/5_`다.
- **공통:** 16:03 이후 WARN·ERR와 알람은 0건이다.
  - 유일한 `[HYBRID]` 줄은 OLOG 16:11:56 `EFA view: the recipe is no longer hybrid - segment selector hidden`이다. 하이브리드 세션에서 떠 있던 선택기가 1세그먼트 레시피로 바뀌어 숨은 것으로, 설계대로다. 런 로그가 아니다.

**구현 결과 (2026-09-30)** — 아래 플랜 (0)~(14)는 구현 전 설계 원문이다. 줄 번호는 13:4x 기준이라 지금 코드와 다르다(앵커 문자열로 찾을 것).

- **진행 방식**
  - 스테이지별 워크플로: 구현 → 독립 리뷰 → 수정.
  - S1과 S2는 파일이 겹치지 않아 병렬로 돌렸다.
  - 스크립트·백업은 세션 scratchpad에 있다(`p3c_s0_patch.pl` / `p3c_s1_patch.pl` / `p3c_s1_lead_fix.pl` / `p3c_s2_patch.pl` / `p3c_s2_fix.pl`, 백업은 각 `*_bak`). 세션이 끝나면 사라지므로 복구는 `git checkout`으로 한다.
- **S0 구조 + 로그 (C1–C7, C9–C20)**: 리뷰 결함 0.
  - `CRecipeEditMgr`에 volatile int 캐시(세그먼트 수·활성 세그먼트·rowbase)와 EFA 래치 저장소를 두었다.
  - 캐시 갱신 지점은 기존 두 곳 그대로다(`RecipeChange`, `SegmentSwitchData`의 `SyncCellStatusStickBase`).
  - `GetSegLogTag(BOOL bRowBase)`는 캐시가 1 이하이면 `""`를 돌려준다. `m_pRecipeFunc->GetSegmentLoopCount() <= 1`이어도 `""`다. DlgTestMode처럼 RecipeChange 없이 재로드해 캐시가 낡는 경우를 막는다.
  - 태그 붙는 곳:
    - 행 로그 `LogAgingRow`(WAD / WAD-MP / VISION / ANGLE)
    - `Probe Contact`
    - WAD MultiPoint 점
    - AGING_PTN(+ ` pg(<PG 모델>)`)
    - `EFA pass %d: skip step`
    - 엔진 begin/end/skip. 사이클 시작 시 태그를 캐시하므로, 마지막 `end`가 SegmentRestore0 뒤에 찍혀도 `seg(N/N)`로 나온다.
  - 스틱 헤더(N>1만): `[HYBRID] seg k/N [name] stick i/n (glass stick g/G)[ pass p/P (Tp)]`. skip 스틱에도 찍힌다.
  - tact 이름(N>1만): `Seg%d_Stick%d_%s`, EFA는 `T%d_Seg%d_Stick%d_%s`. N=1은 `Stick%d_%s` 원문 그대로다.
  - WARN(N>1 측정 시작): 결과 행이 Phase 4 전까지 세그먼트 로컬이라는 경고.
  - SegmentRestore0 복귀 실패 WARN.
  - CheckAgingMaskShape: 섹션이 없으면 OPER 1회(글라스·세그먼트당).
- **S1 A5 래치 + 사전 검사 (C8, C21–C24)**
  - 래치 흐름: 측정 시작에서 `AgingGlassLatch(bEfaMode ? nPassCount : 0)`이 `SetEfaGlassLatch`를 부르고, `MeasDone`은 래치가 0 이상이면 그 값을 쓴다(플래그·EFA rcp 재읽기 없음).
  - 래치 -1(1세그먼트, 다음 글라스 RecipeChange 뒤)이면 원래 게이트 조건식을 그대로 쓴다.
  - 엔진 OPER `[HYBRID] EFA latched for this glass: passes %d (0 = not EFA) ...`: 하이브리드 글라스마다 1회.
  - `MeasDone` SYST 래치 줄: 래치 > 0일 때만.
  - D3c-2 정정: N>1의 `EFA^1$`는 측정 시작 판정을 따른다. N=1 게이트와 달라지는 경우는 두 가지다(스텝 인덱스 누락으로 EFA 미실행, 공유 플래그를 글라스 도중 해제).
  - `CheckHybridCapacity` EFA K:
    - 레시피 EFA 플래그 + 항목 있는 EFA rcp면 EFA 확정. K wad = max(1, EFA rcp WAD 종류 수), vis/ang = 1.
    - 공유 플래그만 켜진 경우: K wad = max(AgingRcp, EFA rcp). 둘 중 무엇이 돌아도 상한이 되게 한다.
  - **리드 수정**(리뷰 low, `p3c_s1_lead_fix.pl`): EFA WAD 블록 수를 K×P로 세던 것을 실제 레이아웃대로 고쳤다. P>1이면 `K + P - 1`, P=1이면 `1`이다(EFA는 스틱당 항목 1개라 base 점이 블록 0에 남는다). 공유 플래그만 켜진 경우는 max(AgingRcp K×P, EFA 블록).
  - 신규 `CheckHybridAgingMask`(static, WARN만):
    - 세그먼트별 마스크 행 요약 SYST
    - 섹션 없음 / 행 수 불일치 WARN
    - A12 PTN → PG 모델 불일치 WARN
    - 세그먼트별 `m_bUseAgingTest_2` 불일치 WARN
- **S2 AgingNew 세그먼트 선택기 (C25–C38)**
  - 컨트롤: `IDC_COMBO_EFA_SEG` 21008(166,53,160,120), `IDC_STATIC_EFA_SEG` 21009(134,56,30,8). `NOT WS_VISIBLE`이고 하이브리드일 때만 보인다.
  - 콤보 라벨: `Seg k/N name R90 (s x c)`.
  - 세그먼트별 섹션을 로드·저장한다(k≥1 `[MASECELL_SEGk+1]`). 스틱 수를 넘는 `STICK_n` 키는 삭제한다. k≥1 섹션이 없으면 전부 체크로 표시한다(D3c-13).
  - 전환 전 stale 검사 + 한글 예/아니요/취소.
  - START 시 세그먼트별 마스크 행 OPER. 섹션 없는 세그먼트가 있으면 한글 예/아니요(D3c-6).
  - 한글 팝업 5종(CP949).
  - 리뷰 F3: 하이브리드 START는 다이얼로그 레시피 ≠ 운전 레시피(`CSystemParamShare::GetRecipeID`)이면 거부한다(한글 경고 + OPER).
  - 리뷰 F6: 보기 리셋·콤보 숨김 때 `m_strViewRcp`를 지운다. 이후 SAVE는 LOAD 전까지 stale로 거부된다. 범위 밖 선택은 거부한다.
  - **리뷰 F1(medium) 수정**(`p3c_s2_fix.pl`):
    - 원인: 매 사이클 레시피 스텝의 `OpenRecipe`, CIM PPID 재로드 모두 `LoadRcpFile`이 `m_vecSegments.clear()` 뒤 `LoadHybridTail`까지 `GetSegmentLoopCount()==1`인 창을 만든다. 500 ms 타이머가 그 창에서 선택기를 숨길 수 있었다.
    - 수정 1: 타이머는 런 플래그(`IsEngineCompleteAll()==false || IsCycleState()`)를 먼저 본다. busy면 비활성화만 한다.
    - 수정 2: idle에서 2틱 연속 1세그먼트일 때만 숨긴다. 이때 `UpdateSegmentCombo()`를 부르지 않고 인라인으로 숨긴다(재로드 중 세그먼트 내용 읽기 금지).
- **F3 결정(2026-09-30 16:3x, 사용자): 선택지 ① — 1세그먼트·하이브리드 모두, AgingNew·AgingTest 두 화면 모두 거부**(`rcpmatch_patch.pl`, 백업 `rcpmatch_bak`, ClCompile 오류 0·경고 0).
  - 조건: 화면 레시피 ≠ 운전 레시피(`CSystemParamShare::GetRecipeID`, CompareNoCase)이면 START를 거부한다. 위치는 빈 레시피 검사 직후다.
  - OPER 로그: `EFA START refused - dialog recipe %s != run recipe %s (the cycle measures the run recipe)`(`[HYBRID]` 접두는 뺐다), `AgingTest START refused - ...`
  - 한글 팝업은 두 화면 공통이다. 레시피 여는 곳으로 운전 화면 CYCLE과 수동 측정 화면을 안내한다.
  - 운전 레시피는 `OpenRecipe`에서만 바뀐다(CYCLE 다이얼로그, 수동 측정·MultiMeas 화면, CIM PPID, 매 사이클 레시피 스텝). 프로그램 시작 직후에는 `UNKNOWN`이라, 첫 START 전에 운전 레시피를 열어야 한다.
  - 레시피 ID 형식: 두 화면은 `.rcp`를 뗀 이름으로 `RecipeChange`, CYCLE은 목록 이름으로 `OpenRecipe`를 부르고, 둘 다 인자를 그대로 저장한다. 따라서 형식 차이로 오판하지 않는다.
- **(해결됨) 종전 남은 결정**
  - F3 거부는 하이브리드 레시피일 때만 건다(1세그먼트 불변 원칙).
  - 그래서 다음 경우는 검사하지 않는다. 다이얼로그 레시피가 1세그먼트 X(`OnRcpOpen` = `RecipeChange`)이고 운전 레시피가 하이브리드 H인데 START하는 경우다. 이때 하이브리드 START 검사(마스크 보고·섹션 없음 확인)가 전부 빠진다.
  - AgingTest 다이얼로그도 같다.
  - 선택지:
    - ① 두 ID가 다르면 세그먼트 수와 무관하게 거부(또는 예/아니요). N=1에서도 두 ID가 다를 때만 동작이 바뀐다.
    - ② 두 다이얼로그의 `OnRcpOpen`을 `OpenRecipe`로 바꿔 ID가 갈라지지 않게 한다.
- **컴파일**
  - ALPHA `ClCompile` 전체(237 유닛, 기본 IntDir): 오류 0.
  - 수정 파일 경고는 기존 것뿐이다(`DoWorkGlassMeasure.cpp:321` C4267, `DoWorkGlassRecipeStep.cpp:984` C4805, 줄 이동만).
  - dfcRecipe는 이번 Phase에서 변경 없음.
  - **링크·실행 전**: ALPHA를 종료하고 전체 빌드해야 한다(DDX + rc + resource.h가 같은 빌드에 들어가야 한다).
- **N=1 불변 근거**
  - 모든 신규 경로는 `GetSegmentCount() > 1` / `GetSegmentLoopCount() > 1` / `IsSegmented()` 게이트 안이다.
  - 태그 `%s`에는 빈 문자열이 들어간다. 콤보는 숨김이라 타이머 블록이 단락된다. 래치 -1이면 원래 게이트를 쓴다.
  - tact 이름·팝업·저장 바이트는 else 원문 그대로다.
- **OOS 체크리스트**: 아래 (10) 검증 1~8. 우선순위:
  1. N=1 EFA·AgingTest 로그 diff 0
  2. 하이브리드 EFA 재실행 — 태그·헤더·tact·래치 줄·섹션 없음 WARN
  3. AgingNew 선택기
  4. START 거부/확인
  5. 런 중 콤보 비활성

---

상태(설계 시점): **플랜(2026-09-30), 구현 전.** 구현 결과는 위 블록을 볼 것.
- 출처: 워크플로 매퍼 3개(EFA 런타임 / AgingNew 다이얼로그 / 세그먼트 태깅) → 이 플래너.
- 앵커: 2026-09-30 13:4x에 raw 바이트와 CP949 디코드 사본에서 모두 1회만 나오는 것을 확인했다.
- 예외: `DoWork/DoWorkGlassRecipeStep.cpp`는 13:19 동시 편집(INTERLOCKHEADGAPB 사전 검사 +13줄)으로 줄 번호가 밀렸다. 패치 직전에 다시 읽을 것.
- 3b·3d·Phase 6의 미커밋 변경 위에 얹는다.
- 이 Phase는 두 가지를 한다.
  - (a) EFA × 세그먼트 잔여분: A5 래치, EFA 세그먼트 마스크 작성(AgingNew 선택기, D6/A13), EFA를 아는 사전 검사, A12.
  - (b) 사용자 요청 "세그먼트 구분 구조": 모든 스틱 단위 로그·tact 이름에 세그먼트를 붙인다. **1세그먼트 레시피는 동작·로그 모두 바이트 동일.**

**(0) 출발점 — 2026-09-30 13:03:08~13:10:26 OOS 런 판독**

| 항목 | 값 |
|---|---|
| 레시피 | `D:\HBS\Recipe\Recipe\9_OCP_TEST_Hybrid_Half_260930.rcp`. seg1 `LONG_H1` rot1(R90) 5스틱×3셀, seg2 `SHORT_H2` rot2(R180) 5스틱×4셀, G = 15+20 = 35 |
| 세그먼트 검사 항목 | 두 세그먼트 동일: Pre 1, Elect 1, AgingTest_2 1, Spec 1, Quality 1, Color 0, Angle 1, EFA 1, SpecMulti 0, Aging_Test 0 |
| PG/WAD 헤더 | 둘 다 공란 → RecipeID |
| EFA rcp | `D:\HBS\PARAM_OCP\PARAM\EFARcp\9_OCP_TEST_Hybrid_Half_260930.rcp`(13:02:12, 285 B) |
| EFA 항목 | `[COUNT] MaxCount=2`. ITEM_1 = `USE_AGING=1 PTN_RGB=1 PTN_SEC=3 USE_PROPERTY=1 USE_QUALITY=1`. ITEM_2 = `USE_ANGLE=1`(나머지 키는 빈 값 = 0) |
| EFA 마스크 | `[MASECELL]` STICK_1..5 = `5,5,5, / 6,6,6, / 1,1,1, / 2,2,2, / 1,1,1,` — 3칸이라 seg1 전용이다. **`[MASECELL_SEG2]`는 없다** |
| 시작 경로 | Operate 사이클 버튼(OLOG 13:02:57). AgingNew START가 아니다. 엔진 `EFA: mode on (flag 0, recipe 1), passes 2` |

| 시각 | 로그 | 판정 |
|---|---|---|
| 13:03:09.086 (DLOG) | `[HYBRID] capacity: sticks 10/50, result rows 35/2500 (G 35 x block 1; K wad/vis/ang 1/1/1, WAD points 1)` | 정상. EFA는 계산에 없다(C23) |
| 13:03:16.426 | `[HYBRID] measure start: 2 segments, 2 pass, active segment 1, resume start stick 0`, 엔진 `EFA: mode on (flag 0, recipe 1), passes 2` | A5 시작 쪽 판정 1회. 어디에도 게시되지 않는다(C21) |
| 13:03:16.429 (엔진) | `[HYBRID] AgingTest flag latched from segment 1 for this glass: 0` | EFA 글라스에서는 쓰이지 않는 래치다(C21) |
| 13:03:16~13:05:00 | T0 seg1 스틱 1~5 | WAD 1/3/5, VISION 2/4. `[MASECELL]`대로 측정됐다 |
| 13:05:00.225~09.035 | `[HYBRID] Segment boundary 1 -> 2/2 [SHORT_H2] rot 1 -> 2` | safety → R 180 → ALIGN USE OFF라 재Align 생략, 8.8 s |
| 13:05:09~13:07:53 | T0 seg2 | 스틱마다 4셀 WAD+VISION. 섹션이 없어 전부 측정했다(D4). **이 사실을 말하는 로그가 없다** |
| 13:07:53.832 → 54.032 | `EFA: pass 2/2 start (T1)` → `EFA: pass boundary before pass 2 (T0 done) - result flush + CIM handshake` → `EFA: T0 result flush OK` → `EFA: not ONLINE REMOTE - skip APD upload / EFA^0$` | A6 OK: 플러시와 EFA^0$ 게이트가 글라스당 1회, 모든 세그먼트 T0 뒤 |
| 13:07:54.033~08:02.203 | `Segment boundary 2 -> 1/2 [LONG_H1] rot 2 -> 1` | R 90, 8.2 s. T1 시작 N-1→0 경계(D-13 정순) OK |
| 13:08:02~08:44 | T1 seg1 | 스틱마다 `EFA pass 2: skip step Pattern Search / OSTest / R-Test`와 `CDoWorkGlassMeasureStep_AGING End. (EFA item USE_AGING=0 - skip)`. ANGLE은 스틱 1~2만(마스크 5/6의 bit2) |
| 13:08:44~08:53 | boundary 1 -> 2 | 8.9 s |
| 13:08:53~10:24 | T1 seg2 | ANGLE을 스틱마다 4셀. 섹션이 없어 전부 측정했다 |
| 13:10:26.064~.097 | `[HYBRID] restore segment 1 (data only, no motion)` → `CDoWorkGlassMeasureStep_Idle` 1회 → `CDoWorkGlassMeasure Thread End` | A10 OK(완료 이벤트 1회). 알람 0 |

- **확인된 것**
  - A6: pass → seg → stick 순서, 플러시 1회, N-1→0 경계.
  - A8: T1이 T0의 `nCellStatus`를 `ToCellStatusStick`로 읽는다. seg1은 열 0~4, seg2는 열 5~9다.
  - A10: 완료 이벤트 1회.
- **확인 안 된 것**
  - A5(`EFA^1$`): OFFLINE이라 `MeasDone`이 `GetUserMode()!=2`에서 먼저 돌아갔다. `SEQ CIM_20260930.log`는 0바이트다.
  - A12: 두 세그먼트 PG 이름이 같았다.
- **로그로는 세그먼트를 구분할 수 없다**(사용자 지적 "세그먼트 구분이 없네"):
  1. 행 로그에 세그먼트가 없다.
     - `AgingCycle row: WAD item(1) repeat(0) stick(0) row(0,1)`(13:03:31, seg1)와 `... stick(4) row(0,1)`(13:07:38, seg2)는 같은 행 1이다.
     - 판독: seg1은 WAD 1-3·7-9·13-15, VISION 4-6·10-12다.
     - seg2는 rot 2라 `MakeCellID`가 뒤집는다: 스틱1 = 17-20, 스틱2 = 13-16, 스틱3 = 9-12, 스틱4 = 5-8, 스틱5 = 1-4. `row(16,17)`·`row(19,16)`의 16은 쉬는 헤드의 가짜 행이다.
     - 그래서 **seg2(1..20)가 seg1의 15행을 전부 덮었다.** T1도 같다: seg1 ANGLE 1-6을 seg2 ANGLE 1..20이 덮었다.
  2. 엔진 로그: `EFA: begin pass 1/2`가 pass당 10번, `AgingCycle: end`가 20번 나오는데 세그먼트가 없다.
  3. 스텝 Start/End와 `Probe Contact stick=1 ...`가 13:03:18(seg1)과 13:05:13(seg2)에서 글자까지 같다.
  4. tact 이력 `D:\HBS\Logs\TACT TIME\history\30\tact_time_history_2026-09-30_131034.txt`에 `Stick1_WAD`가 4번 있다: #18 T0 seg1, #98 T0 seg2, #225 T1 seg1, #272 T1 seg2.
  5. seg2의 "섹션 없음 = 전부 측정"이 조용히 일어났다.
  6. 로그 밖의 문제:
     - WAD 로그에 seg1 Cell ID 9개 중 6개(A018, A030, B019, B031, C010, C032)가 seg2 셀로 다시 나갔다.
     - OOS 결과 파일 `APD_20260930-130753.txt`/`-131026.txt`는 AREA_CNT 0이다(OOS에서는 TossResultFile이 돌지 않는다). 그래서 행 충돌은 행 로그로만 보인다.

**(1) 목적·범위**

- **3c가 하는 일**
  - **세그먼트 구분 구조(N>1 전용)**
    - 출처는 하나다: `CRecipeEditMgr`의 int 캐시 3개(세그먼트 수, 활성 세그먼트, rowbase). 갱신 지점은 3b stick base와 같은 두 곳(`RecipeChange`, `SegmentSwitchData`)뿐이다.
    - 태그 함수 `GetSegLogTag()`는 1세그먼트면 빈 문자열을 돌려준다. 그래서 기존 줄은 전부 바이트 동일하다.
    - 스틱마다 헤더 1줄(세그먼트 이름, 로컬/글라스 스틱, pass).
    - 스틱 단위 로그 10곳 끝에 ` seg(k/N)`, 행 로그에는 ` rowbase(b)`까지 붙인다.
    - tact 이름: `Seg{k}_Stick{i}_{step}`, EFA면 앞에 `T{p}_`.
    - "섹션 없음 = 전부 측정"을 시작 시 WARN, 런타임 OPER로 드러낸다.
    - "행이 세그먼트 로컬" WARN.
  - **A5 EFA 글라스 래치**
    - 측정 시작 판정(pass 수)을 `CRecipeEditMgr`에 게시한다.
    - `MeasDone`은 래치가 있으면 그것만 읽는다. 플래그와 파일을 다시 읽지 않는다.
  - **사전 검사(WARN만)**
    - EFA 글라스면 용량 K = 1.
    - EFA/AgingRcp 마스크 섹션을 세그먼트별로 보고한다.
    - A12: PTN이 서로 다른 PG 모델로 간다.
    - 세그먼트별 `m_bUseAgingTest_2`가 다르다.
  - **AgingNew(EFA) 다이얼로그 세그먼트 선택기**(D6/A13, ID 21008/21009)
    - 3b AgingTest 선택기를 복제한다.
    - EFA `[MASECELL_SEG{k+1}]`을 UI에서 쓸 수 있게 한다. 13:03 런에서 seg2가 전부 측정된 직접 원인이 이것이 없어서다.
    - START 때 섹션 누락을 확인받는다.
- **로직을 바꾸지 않는 것**(검증 완료, 로그 문맥만 추가): A6(`EfaPassBoundary`, pass→seg→stick, 플러시 후 회전), A8(`ToCellStatusStick`), A10(Last-Stick 가드), `GetEfaPassCount` 본문, `SegmentBoundary`, EFA T1 skip 목록.
- **남기는 것**
  - Phase 4: segBase, Cell ID 글자 이월, APD 헤더, 결과 저장 게이트, 결과 업로드 로그의 세그먼트 스냅샷. 3c는 `rowbase(b)`를 **출력만** 한다(행에는 더하지 않는다).
  - Phase 5: Resume (pass, seg, stick) 튜플.
  - Phase 6: 운전 화면 `Segment n/N` 텍스트와 T0/T1 표시.
  - 틀린 채 남는 것은 (12)에 정리했다.
- **3d와의 관계:** EFA T0/T1은 같은 WAD 스텝을 타므로 3d의 WAD Multi 세그먼트 적재가 그대로 적용된다. D3d-8 운용 제한도 유지한다.

<details><summary>(2) 접점 표 C1–C38 (설계 시점 원문, 펼치기)</summary>

**(2) 접점 표** (줄 번호는 2026-09-30 13:4x 기준, 앵커는 전부 raw에서 유일)

*(2a) 세그먼트 구분 구조 — `Source/ALPHA/Recipe/RecipeEditMgr.h/.cpp` (CP949 CRLF)*

| # | 접점 (file:line, 앵커) | 문제 | 변경 |
|---|---|---|---|
| C1 | `RecipeEditMgr.h:58` `m_nCellStatusStickBase;\t// LYS 2026-09-29 Hybrid 3b A8 set on the measure thread, read by the PG receive / UI threads` 줄 뒤 | 세그먼트 식별값을 스레드 안전하게 읽을 곳이 없다. PG 수신·UI 스레드는 세그먼트 벡터를 읽으면 안 된다(`SetActiveSegment`가 값 복사 중일 수 있다) | 멤버 4개: `volatile int m_nSegCountCache;` (1 = 하이브리드 아님) / `m_nActiveSegCache;` / `m_nSegRowBaseCache;` (이전 세그먼트들의 셀×스틱 합 = Phase 4 segBase 미리보기) / `m_nEfaGlassLatch;` (-1 = 래치 없음, 0 = EFA 아님, n = pass 수) |
| C2 | `RecipeEditMgr.h:113` `GetGlassRowBlock(...)` 선언 줄 뒤 | 태그·래치 API가 없다 | (4) 참조: `GetSegLogTag(BOOL bRowBase = FALSE) const`, `GetSegRowBase()`, `SetEfaGlassLatch(int)`, `GetEfaGlassLatch()` |
| C3 | `RecipeEditMgr.cpp:40` `m_nCellStatusStickBase = 0;\t// LYS 2026-09-29 Hybrid 3b A8` 뒤 | 초기화 | `m_nSegCountCache = 1; m_nActiveSegCache = 0; m_nSegRowBaseCache = 0; m_nEfaGlassLatch = -1;` |
| C4 | `RecipeEditMgr.cpp:134` `SyncCellStatusStickBase();\t// LYS 2026-09-29 Hybrid 3b A8 LoadRcpFile activates segment 0 -> base 0` 뒤 | 래치 수명 | `\tm_nEfaGlassLatch = -1;\t// LYS 2026-09-30 Hybrid 3c A5 new recipe / new glass: the next measure start latches again`. 글라스마다 Recipe 아이템 `_Change`의 `OpenRecipe → RecipeChange`(`DoWorkGlassRecipeStep.cpp:423`, `RecipeEditMgr.cpp:255`)를 지나므로 글라스당 1회 리셋된다 |
| C5 | `RecipeEditMgr.cpp:327-334` `SyncCellStatusStickBase`, 앵커 `\tm_nCellStatusStickBase = nBase;\r\n` 뒤 | 캐시 갱신 | 아래 코드. 호출처는 그대로 두 곳이다(RecipeChange, `DoWorkGlassMeasure::SegmentSwitchData:505`) |
| C6 | `RecipeEditMgr.cpp` `bool CRecipeEditMgr::SaveRecipe(CString strRecipeID)` 앞(`GetGlassRowBlock` 본문 :342-350 다음) | — | `GetSegLogTag` 구현(아래) |

```cpp
// C5 - SyncCellStatusStickBase 끝에 추가
	// LYS 2026-09-30 Hybrid 3c segment log tag caches (same two refresh points: RecipeChange, SegmentSwitchData)
	int nSegCnt = (m_pRecipeFunc != NULL) ? m_pRecipeFunc->GetSegmentCount() : 1;
	int nActSeg = 0, nRowBase = 0;
	if (nSegCnt > 1)
	{
		nActSeg = m_pRecipeFunc->GetActiveSegmentIndex();
		for (int k = 0; k < nActSeg && k < nSegCnt; k++)
			nRowBase += m_pRecipeFunc->GetSegmentCellCount(k) * m_pRecipeFunc->GetSegmentStickCount(k);
	}
	else nSegCnt = 1;
	m_nSegCountCache = nSegCnt;
	m_nActiveSegCache = nActSeg;
	m_nSegRowBaseCache = nRowBase;

// C6
// LYS 2026-09-30 Hybrid Phase 3c segment tag appended to per-stick logs. Reads the int caches only (PG receive / UI threads may call it).
//   1-segment recipe -> empty string, so every existing log line stays byte-identical.
CString CRecipeEditMgr::GetSegLogTag(BOOL bRowBase) const
{
	CString strTag;
	if (m_nSegCountCache <= 1) return strTag;
	if (bRowBase) strTag.Format(_T(" seg(%d/%d) rowbase(%d)"), m_nActiveSegCache + 1, m_nSegCountCache, m_nSegRowBaseCache);
	else          strTag.Format(_T(" seg(%d/%d)"), m_nActiveSegCache + 1, m_nSegCountCache);
	return strTag;
}
```

*(2b) 측정 스레드 — `Source/ALPHA/DoWork/DoWorkGlassMeasure.cpp` (CP949 CRLF, 693줄)*

| # | 접점 | 문제 | 변경 |
|---|---|---|---|
| C7 | :198-203 `if (nSegCount > 1)` 블록, 앵커 `\t\tm_pLogFileAccessor->KMLOG(OPER, strSegLog);\r\n` 뒤(같은 블록 안) | segBase가 없어 행이 겹친다는 사실이 어디에도 없다 | WARN 1줄(아래). 값은 이 시점 측정 스레드에서 `pSegFunc->GetSegmentCellCount/StickCount(k)`로 만든다 |
| C8 | :206 `m_pMeasSeqItemEngine->AgingGlassLatch();` | A5 판정이 지역 변수 `nPassCount/bEfaMode`에만 있다 | `m_pMeasSeqItemEngine->AgingGlassLatch(bEfaMode ? nPassCount : 0);\t// LYS 2026-09-30 Hybrid 3c A5 also publishes the EFA glass latch`. **C21과 같은 빌드로 넣는다** |
| C9 | :213 `strPassLog.Format(_T("EFA: pass %d/%d start (T%d)"), nPass + 1, nPassCount, nPass);` 뒤 | pass 줄에 세그먼트 문맥이 없다 | `if (nSegCount > 1) { CString strPassSeg; strPassSeg.Format(_T(" - %d segments, segment 1 first"), nSegCount); strPassLog += strPassSeg; }` |
| C10 | :248 `GoToInitStep();  // Init` 뒤(AgingCycleBegin :251 앞) | 스틱 블록 20개가 시각·행 번호 말고는 똑같다 | 스틱 헤더(아래). 세그먼트 이름은 측정 스레드에서 헤더 벡터(`GetSegmentName`)로 읽는다 |
| C11 | :265 `strSkip.Format(_T("EFA pass %d: skip step %s"), nPass + 1, (LPCTSTR)pSeqItem->GetName());` | 세그먼트 없음 | 형식 `...skip step %s%s`, 인자 `(LPCTSTR)m_pShareItem->GetRecipeEditMgrPtr()->GetSegLogTag()` 추가 |
| C12 | :274 `strName.Format(_T("Stick%d_%s"), i + 1, pSeqItem->GetName());` | tact 이름이 세그먼트 로컬이고 pass를 모른다 | 아래 3분기. N=1 분기는 원문 그대로. `SeqTactTimeMgr::SetSeqItemTimeED`는 이름 동등 비교만 하고(:274), ST/ED가 같은 `strName`을 쓰므로 매칭 영향 없음 |
| C13 | `EfaPassBoundary` :438 `strLog.Format(_T("EFA: pass boundary before pass %d (T0 done) - result flush + CIM handshake"), nPass + 1);` 뒤, :456 `strLog.Format(_T("EFA: T0 result flush %s"), ...);` 뒤 | T0 플러시가 겹친 행을 담는다는 사실을 말하지 않는다 | `if (m_pShareItem->GetRecipeEditMgrPtr()->GetSegmentLoopCount() > 1)` 조건으로 각각 `(all %d segments)`, ` - hybrid: rows of the segments overlap until Phase 4 (segBase)` 덧붙임. 로직 무변경(플러시 후 회전 유지 → T1 경계 실패에도 T0 파일은 남는다) |
| C14 | :537 `SegmentRestore0` `if (SegmentSwitchData(0) == false) m_nCurSeg = 0;` | 복귀 실패 시 Result/EFA^1$가 세그먼트 N-1을 읽을 수 있는데 추적할 수 없다 | 하이브리드 전용 경로라 `{ WARN "[HYBRID] restore segment 1 failed - active segment stays %d (Result reads it, EFA^1$ uses the glass latch)", pFunc->GetActiveSegmentIndex() + 1; m_nCurSeg = 0; }` |

```cpp
// C7
		// LYS 2026-09-30 Hybrid 3c result rows are segment-local until Phase 4 (segBase) - say which rows overlap
		CString strRowLog = _T("[HYBRID] result rows are segment-local until Phase 4 (segBase):"), strRowOne;
		for (int k = 0; k < nSegCount; k++)
		{
			strRowOne.Format(_T(" seg%d rows 1..%d"), k + 1, pSegFunc->GetSegmentCellCount(k) * pSegFunc->GetSegmentStickCount(k));
			strRowLog += strRowOne;
		}
		strRowLog += _T(" - an overlapping row keeps the last segment's value");
		m_pLogFileAccessor->KMLOG(WARN, strRowLog);

// C10
				// LYS 2026-09-30 Hybrid 3c one header line per stick so the step Start/End lines can be told apart by segment (1-segment recipes: no line)
				if (nSegCount > 1)
				{
					CString strStickLog;
					strStickLog.Format(_T("[HYBRID] seg %d/%d [%s] stick %d/%d (glass stick %d/%d)"), nSeg + 1, nSegCount, pSegFunc->GetSegmentName(nSeg).GetString(),
						(int)i + 1, nStickCnt, m_pShareItem->GetRecipeEditMgrPtr()->ToCellStatusStick((int)i) + 1, pSegFunc->GetSegmentStickBase(nSegCount));
					if (bEfaMode)
					{
						CString strPass;
						strPass.Format(_T(" pass %d/%d (T%d)"), nPass + 1, nPassCount, nPass);
						strStickLog += strPass;
					}
					m_pLogFileAccessor->KMLOG(OPER, strStickLog);
				}

// C12 (:273-274 if-branch body)
						if (GetCurStep() > 0 && GetCurStep() < GetExitStepIndex() )
						{
							if (nSegCount > 1 && bEfaMode)	// LYS 2026-09-30 Hybrid 3c tact names carry pass and segment (1 segment: unchanged)
								strName.Format(_T("T%d_Seg%d_Stick%d_%s"), nPass, nSeg + 1, (int)i + 1, (LPCTSTR)pSeqItem->GetName());
							else if (nSegCount > 1)
								strName.Format(_T("Seg%d_Stick%d_%s"), nSeg + 1, (int)i + 1, (LPCTSTR)pSeqItem->GetName());
							else
								strName.Format(_T("Stick%d_%s"), i + 1, pSeqItem->GetName());
						}
```

*(2c) 스텝·엔진 로그 — `DoWork/DoWorkGlassMeasureStep.cpp`(CP949 CRLF 7023줄), `DoWork/MeasureSequenceItemEngine.h/.cpp`(CP949 CRLF)*

| # | 접점 | 문제 | 변경 |
|---|---|---|---|
| C15 | `DoWorkGlassMeasureStep.cpp:502-505` `LogAgingRow`, 형식 `_T("AgingCycle row: %s item(%d) repeat(%d) stick(%d) row(%d,%d)")`, 앵커 `\t\tpnAreaNum[0], pnAreaNum[1]);\r\n` | 행 로그에 세그먼트가 없다. 호출처 WAD :2846, WAD-MP :3030, VISION :3889, ANGLE :5301을 한 번에 덮는다 | 형식 끝에 `%s`, 인자 `(LPCTSTR)m_pShareItem->GetRecipeEditMgrPtr()->GetSegLogTag(TRUE)` 추가 |
| C16 | :2028 `Probe Contact stick=%d offset=...(End %.4f + OD %.4f)` | 13:03:18과 13:05:13이 같은 `stick=1` | 형식 끝 `%s` + `GetSegLogTag()` |
| C17 | :2867-2868 `WAD MultiPoint: probe(%d) point(base) stick(%d) cell(%d) row(%d)`, :3037-3038 `... point(%d/%d) idx(%d) stick(%d) cell(%d) row(%d)` | C15와 같다(행이 세그먼트 로컬) | 각각 끝 `%s` + `GetSegLogTag(TRUE)` |
| C18 | :6457-6458 `CDoWorkGlassMeasureStep_AGING_PTN Start. item(%d) ptn(%d) sec(%d)` | A12: 어떤 PG 모델이 PTN을 받았는지 보이지 않는다 | `CString strPtnTag = pRcp->GetSegLogTag(); if (!strPtnTag.IsEmpty()) strPtnTag += _T(" pg(") + pRcp->GetPGRecipeName() + _T(")");`(측정 스레드라 활성 헤더를 읽어도 된다), 형식 끝 `%s` |
| C19 | 엔진 `.h:126` 앵커 `bit k = segment k mask shape already checked on this glass\r\n` 뒤. `.cpp` AgingCycleBegin 2줄 앵커 `m_bEfaCycle = false;\t// LYS 2026-09-28 EFA\r\n\tZeroMemory(m_nAgingRepeat, sizeof(m_nAgingRepeat));\t// LYS 2026-08-12 AgingCycle R1\r\n` 뒤. `.cpp` 로그 :817, :843, :992 | 엔진 로그 `EFA: begin pass 1/2`, `AgingCycle: begin/end`에 세그먼트가 없다. 마지막 스틱의 `AgingCycle: end`는 `SegmentRestore0` **뒤**에 찍혀(13:10:26.098) 실시간 태그로는 seg(1/2)라고 거짓말한다 | `.h`: `CString m_strCycleSegTag;\t// LYS 2026-09-30 Hybrid 3c segment tag of the running cycle (AgingCycleEnd logs after the segment-0 restore)`. `.cpp` Begin: `m_strCycleSegTag = GetRecipeEditMgrPtr()->GetSegLogTag();`. :817 `EFA: begin pass %d/%d%s` + `m_strCycleSegTag.GetString()`. :843 `AgingCycle: begin, items(%d) active(%d)%s`. :992 `KMLOG(OPER, _T("AgingCycle: end") + m_strCycleSegTag);` 다음 줄에 `\tm_strCycleSegTag.Empty();`(중괄호 없는 if 밖이라 항상 실행). **Begin 앵커는 2줄로 쓸 것** — ZeroMemory 줄은 End(:1000)에도 같은 문구로 있다 |
| C20 | 엔진 :911 `\tif (nRows <= 0) return;\t// no mask section = every cell measured (intended)\r\n` | 13:03 seg2가 조용히 전부 측정됐다 | 아래 코드. 하이브리드만, 글라스·세그먼트당 1회(기존 `m_nAgingMaskChecked` 비트, AgingGlassLatch에서 리셋). T1 seg0 재진입 때는 다시 찍히지 않는다. 레벨은 OPER(시작 시 WARN은 C24가 담당) |

```cpp
// C20
	if (nRows <= 0)	// no mask section = every cell measured (intended)
	{
		if (GetRecipeEditMgrPtr()->GetSegmentLoopCount() > 1)	// LYS 2026-09-30 Hybrid 3c say it once per glass and segment (1 segment: silent as before)
		{
			CString strNoMask;
			strNoMask.Format(_T("%s: mask [%s] not found - segment %d/%d measures every cell of the item's kinds (no mask = measure all)"),
				m_bEfaCycle ? _T("EFA") : _T("AgingCycle"), CAgingCycleInfo::GetMaskSection(nSeg).GetString(), nSeg + 1, GetRecipeEditMgrPtr()->GetSegmentLoopCount());
			m_pLogFileAccessor->KMLOG(OPER, strNoMask);
		}
		return;
	}
```

*(2d) A5 EFA 글라스 래치*

| # | 접점 | 문제 | 변경 |
|---|---|---|---|
| C21 | 엔진 `.h:107` `AgingGlassLatch();` → `AgingGlassLatch(int nEfaPassCount = 0);`. `.cpp:882` `void CMeasureSequenceItemEngine::AgingGlassLatch()` → `(int nEfaPassCount)`. 본문 :884-897 | 시작 판정이 게시되지 않는다. 글라스 끝 게이트는 공유 `m_bNewAgingMode`를 다시 읽는데, 이 값은 글라스 도중 바뀔 수 있다(PPID마다 리셋 `CommTcpIpObjectCIM.cpp:1419`, AgingNew OnStop :247, AgingTest START :463). EFA rcp도 다시 읽는다 → T0 `EFA^0$`만 가고 `EFA^1$`는 안 가거나, 그 반대가 될 수 있다. EFA 글라스에서는 AgingTest 래치 줄이 잡음이다 | ① `m_nAgingMaskChecked = 0;` 줄 뒤: `\tGetRecipeEditMgrPtr()->SetEfaGlassLatch(-1);\t// LYS 2026-09-30 Hybrid 3c A5 1-segment recipe: no latch, MeasDone keeps its own gate` ② AgingTest 래치 형식 → `_T("[HYBRID] AgingTest flag latched from segment 1 for this glass: %d%s")`, 인자 `(nEfaPassCount > 0) ? _T(" (unused - EFA glass)") : _T("")` ③ 함수 끝 `}` 앞(N>1만 도달): `GetRecipeEditMgrPtr()->SetEfaGlassLatch(nEfaPassCount);` + OPER `[HYBRID] EFA latched for this glass: passes %d (0 = not EFA) - EFA^1$ at MeasDone follows this` |
| C22 | `DoWork/DoWorkSequenceCIMStep.cpp:776-786` `MeasDone`, 앵커 `\t\tbool bEfaGlass = false;\r\n\t\tif (m_pShareItem->GetSystemConfigMgrPtr() != nullptr && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP())\r\n` (`bool bEfaGlass = false;`는 파일에서 1회) | 위와 같다 | 아래 코드. 래치가 있으면 파일·플래그를 다시 읽지 않는다. 의미는 N=1과 같은 "EFA 글라스"다(D3c-2) |

```cpp
// C22
		bool bEfaGlass = false;
		int nEfaLatch = m_pShareItem->GetRecipeEditMgrPtr()->GetEfaGlassLatch();	// LYS 2026-09-30 Hybrid 3c A5 hybrid glass: the measure-start decision (-1 = 1-segment recipe -> gate below as before)
		if (nEfaLatch >= 0)
		{
			bEfaGlass = (nEfaLatch > 0);
			CString strLatch;
			strLatch.Format(_T("CDoWorkSequenceCIMStep_MeasDone : [HYBRID] EFA glass latch %d pass(es) - no flag / file re-read"), nEfaLatch);
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLatch);
		}
		else if (m_pShareItem->GetSystemConfigMgrPtr() != nullptr && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP())
```

- `GetEfaPassCount`(엔진 :851-875)는 **본문을 바꾸지 않는다.** 같은 글라스의 Recipe 아이템 `OpenRecipe`가 세그먼트 0을 활성화한 직후라서 `GetRecipeInfo()`가 곧 세그먼트 0이다(`RecipeEditMgr.cpp:130-134`).
- 스텝 인덱스(`nStepIdxAging/Vision`)가 없어 EFA가 실제로 안 돌면, N>1 래치는 0이다(= 실제로 돈 것). N=1은 기존 게이트 그대로.

*(2e) 사전 검사 — `DoWork/DoWorkGlassRecipeStep.cpp` (CP949 CRLF, 하이브리드 블록 :436-468 안에서만)*

| # | 접점 | 문제 | 변경 |
|---|---|---|---|
| C23 | `CheckHybridCapacity` :259-332. :281 `\tif (bOCP && pSeg0 != nullptr && pSeg0->m_sInfo.m_sInspectItem.m_bUseAging_Test != FALSE)\r\n`, :325 `strLog += _T(" (per segment ") + strPerSeg + _T(")");` 뒤 | 런타임은 EFA 글라스에서 AgingTest 대신 EFA를 돈다(ThreadRun `bEfaMode ? AgingCycleBegin(nPass) : AgingCycleBegin()`). 그런데 K를 AgingRcp에서 곱해 합법 글라스를 거부할 수 있다. EFA pass는 항상 블록 0이다(T1이 T0를 덮는다) | :281을 아래로 교체. 로그 끝에 `if (nEfaPass > 0) { CString s; s.Format(_T(" (EFA %d passes: K = 1, T1 overwrites T0)"), nEfaPass); strLog += s; }`. **완화는 레시피 플래그 기준만**(공유 플래그만 켜진 경우는 기존처럼 보수적으로 계산) — 레시피는 글라스 도중 바뀌지 않는다 |
| C24 | 신규 static `CheckHybridAgingMask`: `bool CDoWorkGlassRecipeStep_Change::Run()` (:409) 앞에 정의. 호출은 :463 `CheckHybridWadMulti(m_pShareItem, pSegFunc);` 뒤에 `\t\t\tCheckHybridAgingMask(m_pShareItem, pSegFunc);\t// LYS 2026-09-30 Hybrid 3c WARN only` | 세그먼트 k의 EFA/AgingRcp 섹션 유무, A12, 세그먼트별 AGING 플래그를 시작 전에 아무도 보지 않는다 | 아래 코드. **WARN만, 거부 없음**(D4/D3c-5). 게이트는 런타임과 같다(EFA = 공유 플래그 또는 레시피 플래그 + 항목 있는 EFA rcp, 아니면 세그먼트 0 `m_bUseAging_Test`) |

```cpp
// C23 (:281 교체)
	// LYS 2026-09-30 Hybrid 3c the runtime runs EFA instead of AgingTest and an EFA pass uses block 0 only (T1 overwrites T0)
	//   -> K = 1 when the recipe EFA flag + an EFA rcp with items decide EFA (the shared flag alone keeps the conservative AgingRcp K)
	int nEfaPass = 0;
	if (bOCP && pSeg0 != nullptr && pSeg0->m_sInfo.m_sInspectItem.m_bUseEFA != FALSE)
	{
		CAgingCycleInfo efa;
		if (efa.Load(pShare->GetRecipeEditMgrPtr()->GetRecipeID(), true) && efa.GetItemCount() > 0) nEfaPass = efa.GetItemCount();
	}
	if (bOCP && pSeg0 != nullptr && pSeg0->m_sInfo.m_sInspectItem.m_bUseAging_Test != FALSE && nEfaPass == 0)

// C24
// LYS 2026-09-30 Hybrid Phase 3c AgingRcp / EFA rcp masks per segment, before the glass moves (hybrid recipes only). WARN only, never refuses (D4):
//   a segment without its section measures every cell of each item's kinds - 2026-09-30 13:03 segment 2 did that without a log line.
//   Also A12 (the same PTN index goes to different PG models) and per-segment m_bUseAgingTest_2 (the AGING step runs only where it is on).
static void CheckHybridAgingMask(CBaseSequenceImpl* pShare, C_OutRecipeFunc* pFunc)
{
	if (pShare->GetSystemConfigMgrPtr() == nullptr || pShare->GetSystemConfigMgrPtr()->IsOCP() == false) return;
	S_RecipeMgr* pSeg0 = pFunc->GetSegmentMgr(0);
	if (pSeg0 == nullptr) return;
	CString strRcpID = pShare->GetRecipeEditMgrPtr()->GetRecipeID();
	int nSeg = pFunc->GetSegmentCount();

	CAgingCycleInfo info;	// same gates as the runtime (GetEfaPassCount / latched segment 0 AgingTest flag)
	bool bEfa = (pShare->GetShareMemoryMgrPtr()->GetSys_NewAgingMode() || pSeg0->m_sInfo.m_sInspectItem.m_bUseEFA != FALSE)
		&& info.Load(strRcpID, true, nSeg) && info.GetItemCount() > 0;
	if (bEfa == false)
	{
		if (pSeg0->m_sInfo.m_sInspectItem.m_bUseAging_Test == FALSE) return;
		if (info.Load(strRcpID, false, nSeg) == false || info.GetItemCount() < 1) return;	// the runtime logs "aging rcp not found"
	}
	LPCTSTR szKind = bEfa ? _T("EFA") : _T("AgingRcp");

	CString strLog, strOne, strWarn;
	strLog.Format(_T("[HYBRID] %s mask rows per segment:"), szKind);
	for (int k = 0; k < nSeg; k++)
	{
		int nRows = info.GetStickCount(k), nSticks = pFunc->GetSegmentStickCount(k);
		CString strSec = CAgingCycleInfo::GetMaskSection(k);
		if (nRows > 0) strOne.Format(_T(" seg%d [%s] %d/%d"), k + 1, strSec.GetString(), nRows, nSticks);
		else           strOne.Format(_T(" seg%d [%s] none"), k + 1, strSec.GetString());
		strLog += strOne;
		strWarn.Empty();
		if (nRows == 0)
			strWarn.Format(_T("[HYBRID] %s mask [%s] not found - segment %d/%d measures every cell of each item's kinds (no mask = measure all)"), szKind, strSec.GetString(), k + 1, nSeg);
		else if (nRows != nSticks)
			strWarn.Format(_T("[HYBRID] %s mask [%s] rows %d != segment %d/%d sticks %d - sticks without a row are skipped"), szKind, strSec.GetString(), nRows, k + 1, nSeg, nSticks);
		if (strWarn.IsEmpty() == FALSE) pShare->GetLogFileAccessorPtr()->KMLOG(WARN, strWarn);
	}
	strLog += _T(" (none = every cell measured)");
	pShare->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);

	bool bPtn = false, bAgingItem = false;
	for (int i = 0; i < info.GetItemCount(); i++)
	{
		const S_AgingTest* pItem = info.GetItem(i);
		if (pItem == nullptr || pItem->m_bUseAging == FALSE) continue;
		bAgingItem = true;
		if (pItem->m_nPtnRGB > 0) bPtn = true;
	}
	if (bPtn)	// A12 the PTN index is relative to the PG model of the segment (PGEvent_0 szModel = segment header, blank = RecipeID)
	{
		CString strPG0, strList;
		bool bDiff = false;
		for (int k = 0; k < nSeg; k++)
		{
			CString strPG = pFunc->GetSegmentHeader(k).m_chPGName;
			strPG.Trim();
			if (strPG.IsEmpty()) strPG = strRcpID;
			if (k == 0) strPG0 = strPG; else if (strPG != strPG0) bDiff = true;
			strOne.Format(_T("%sseg%d [%s]"), (k > 0) ? _T(" / ") : _T(""), k + 1, strPG.GetString());
			strList += strOne;
		}
		if (bDiff)
		{
			strLog.Format(_T("[HYBRID] %s PTN index goes to PG models %s - their pattern tables must match (A12)"), szKind, strList.GetString());
			pShare->GetLogFileAccessorPtr()->KMLOG(WARN, strLog);
		}
	}
	if (bAgingItem)	// the AGING step reads the ACTIVE segment's m_bUseAgingTest_2 (DoWorkGlassMeasureStep AGING bUseMeas); ValidateSegments does not compare it
	{
		BOOL b0 = (pSeg0->m_sInfo.m_sInspectItem.m_bUseAgingTest_2 != FALSE);
		CString strDiff;
		for (int k = 1; k < nSeg; k++)
		{
			S_RecipeMgr* pSeg = pFunc->GetSegmentMgr(k);
			if (pSeg == nullptr || (pSeg->m_sInfo.m_sInspectItem.m_bUseAgingTest_2 != FALSE) == b0) continue;
			strOne.Format(_T("%s%d"), strDiff.IsEmpty() ? _T("") : _T(","), k + 1);
			strDiff += strOne;
		}
		if (strDiff.IsEmpty() == FALSE)
		{
			strLog.Format(_T("[HYBRID] %s: AGING (m_bUseAgingTest_2) of segment(s) %s differs from segment 1 (%d) - aging runs only where it is on%s"),
				szKind, strDiff.GetString(), (int)b0, bEfa ? _T(", T1 still re-measures every segment") : _T(""));
			pShare->GetLogFileAccessorPtr()->KMLOG(WARN, strLog);
		}
	}
}
```

*(2f) AgingNew(EFA) 다이얼로그 선택기 — D6/A13*
- 대상: `Source/ALPHA/DlgSubManual_AgingNew.h/.cpp`, 인코딩은 **CP949, LF-only, BOM 없음**.
- 참조 구현: `DlgSubManual_AgingTest.cpp` :743-750, :811-868, :894-910, :960-1001, :1006-1052, :1056-1079, :203, :537.
- 참조 구현이 들어 있는 3d 스타일: `DlgSubManual_MultiMeas.cpp:1085-1103`.

| # | 접점 | 문제 | 변경 |
|---|---|---|---|
| C25 | `resource.h:578` `#define IDC_STATIC_AGING_SEG            21007\r\n` 뒤 (UTF-8 BOM CRLF) | ID 없음(21008/21009는 3d가 비워 둔 값. Source/ALPHA·Source/Include·CoreFunction/common에서 0건) | `#define IDC_COMBO_EFA_SEG               21008\r\n#define IDC_STATIC_EFA_SEG              21009\r\n`(값은 21007 줄과 같은 41열). `_APS_NEXT_CONTROL_VALUE`(1052)는 건드리지 않는다. ASCII라 raw 삽입 |
| C26 | `ALPHA.rc:720` `    CTEXT           "EFA Test View",IDC_STATIC_MANUAL_TITLE,0,0,650,10,SS_CENTERIMAGE \| WS_BORDER` 앞 (UTF-16LE BOM CRLF, 대화상자 :689-721) | 컨트롤 없음. 빈 사각형은 x 134..326 × y 53..67(EDIT_1 52,53,71,14 오른쪽, 셀 그리드 y 76 위), AgingTest와 같은 자리 | `    LTEXT           "Segment",IDC_STATIC_EFA_SEG,134,56,30,8,NOT WS_VISIBLE\r\n    COMBOBOX        IDC_COMBO_EFA_SEG,166,53,160,120,CBS_DROPDOWNLIST \| WS_VSCROLL \| NOT WS_VISIBLE \| WS_TABSTOP\r\n`. `IDC_FPS_AGING_LIST` 줄(:719)은 rc에 2번 있어 앵커로 못 쓴다. DESIGNINFO(:1341)·AFX_DIALOG_LAYOUT(:1616) 무변경 |
| C27 | `.h:46` `m_nMaskMismatch;` 줄 뒤 / `.h:60` `\tvoid MeasCellSave();` / `.h:63` `ConfirmMaskOverwrite();` 줄 뒤 / `.h:83` `\tafx_msg void OnAgingProcStrart();` 뒤 | 멤버·선언 없음 | `CComboBox m_cmbSeg;`, `int m_nViewSeg;` (주석 `// LYS 2026-09-30 Hybrid 3c A13 ...`), `BOOL MeasCellSave();` (FALSE = 저장 안 함), `BOOL IsSegmented(); BOOL IsHybridRunBusy(); void UpdateSegmentCombo();`, `afx_msg void OnSelchangeSeg();`. 기존 `m_strViewRcp`(:45)를 콤보·그리드 공통 레시피 표식으로 쓴다(AgingTest의 `m_strViewSegRcp` 역할) |
| C28 | `.cpp:18` `#include "Manager\SystemConfigMgr.h"` 뒤 | `CAgingCycleInfo::GetMaskSection`, `DEFINE_AGING_CYCLE_STICK_MAX`를 쓸 수 없다 | `#include "DoWork\AgingCycleInfo.h"\t\t// LYS 2026-09-30 Hybrid 3c A13 mask section names (engine loader)` |
| C29 | `.cpp:37-38` 2줄 앵커 `\tm_nHlCol = 0;\n\tm_nMaskMismatch = 0;\t// LYS 2026-09-30\n` 뒤 (2번째 줄은 :766에도 있다) / `:51` `\tDDX_Control(pDX, IDC_COMBO_1, m_cmbCycle);\n` 뒤 / `:65` `\tON_COMMAND(IDC_BUTTON_8, OnAgingProcStrart)\n` 뒤 | 초기화·바인딩·메시지 맵 | `\tm_nViewSeg = 0;` / `\tDDX_Control(pDX, IDC_COMBO_EFA_SEG, m_cmbSeg);` / `\tON_CBN_SELCHANGE(IDC_COMBO_EFA_SEG, OnSelchangeSeg)`. **DDX는 C25/C26과 같은 빌드**(컨트롤이 없으면 assert) |
| C30 | `.cpp:199-200` `\tif (nIDEvent == EFA_TIMER_HIGHLIGHT)\n\t{\n` 뒤 (500 ms 타이머) | 레시피가 밖에서 1세그먼트로 바뀌면 콤보가 남는다. 측정 중 콤보가 살아 있다 | 아래 코드(D3c-12: MultiMeas처럼 busy면 비활성). 기존 하이라이트 코드 무변경 |
| C31 | `.cpp:522` `\tif (strEfaRcp.IsEmpty()) return FALSE;\n` 뒤 (`LoadAgingInfo`) | 그리드 재생성 전에 보기 세그먼트를 리셋/클램프해야 한다 | `\tUpdateSegmentCombo();\t// LYS 2026-09-30 Hybrid 3c A13 reset/clamp the viewed segment before the grid is rebuilt (compares with the previous m_strViewRcp)`. 표식 :534/:577보다 앞(AgingTest :537과 같은 순서). 호출처(OnRcpOpen :294, OnLoadAgingInfo :343)는 이미 `IsOfflineStatus` 뒤다 |
| C32 | `.cpp:672-674` `UpdateAgingCell` 3줄 (`\tS_RecipeData* pRD = NULL;\n\tif (...)\n\t\tpRD = ...GetRecipeData();\n`) | 항상 활성(유휴 = 0) 세그먼트 크기라 seg2(5×4)를 만들 수 없다 | `{ C_OutRecipeFunc* pSegFunc = ...GetRecipeFuncPtr(); S_RecipeMgr* pSegMgr = (IsSegmented() && pSegFunc != NULL) ? pSegFunc->GetSegmentMgr(m_nViewSeg) : NULL; pRD = (pSegMgr != NULL) ? &pSegMgr->m_sData : ...GetRecipeData(); }`, `SetActiveSegment` 없음. :676 `if (pRD != NULL && pRD->m_vecStick.size() > 0)`는 :823에도 있어 앵커로 쓰지 않는다 |
| C33 | `.cpp:725-757` `MeasCellSave` 전체(순수 ASCII. `void CDlgSubManual_AgingNew::MeasCellSave()\n{\n`부터 `void CDlgSubManual_AgingNew::MeasCellLoad()\n{\n` 직전까지 통째로 교체. 조기 return 줄은 :669/:762에도 있다) | 항상 `[MASECELL]`에 쓴다(seg2 보기로 저장하면 seg0 파괴). 초과 STICK_n을 남긴다. 전환 저장용 자체 검사가 없다 | 아래 코드 |
| C34 | `MeasCellLoad`. 앵커 ① `:765-766` `\tif (strEfaRcp.IsEmpty()) return;\n\tm_nMaskMismatch = 0;\t// LYS 2026-09-30\n` 뒤 ② `:775-776` `\tTCHAR cCellData[EFA_CELLMASK_BUF];\n\tCArray<int, int> nCellChkArray;\n` 뒤 ③ `:780` `GetPrivateProfileString(_T("MASECELL"), ...` ④ `:806` 로그 ⑤ `:809` `\t\tCString strWarn;\n` 뒤(:810은 CP949 한글) | 섹션 고정. k≥1 섹션이 없으면 엔진은 전부 측정하는데 화면은 전부 미체크라, SAVE가 "전부 측정"을 "전부 제외"로 바꾼다 | ① `\tCString strSec = CAgingCycleInfo::GetMaskSection(IsSegmented() ? m_nViewSeg : 0);\t// segment 0 = MASECELL (unchanged)` ② k≥1 섹션 없음 → 3시트 전부 `"1"` + OPER `[HYBRID] EFA rcp [%s] not found - shown all checked (the engine measures every cell)` 후 return(아래) ③ `_T("MASECELL")` → `strSec` ④ `_T("%sEFA rcp [%s]: %d row(s) with cell count != %d shown unchecked (%s)")`, 인자 `IsSegmented() ? _T("[HYBRID] ") : _T("")`, `(LPCTSTR)strSec` 앞에 추가 ⑤ `\t\tif (IsSegmented())\n\t\t\tstrWarn.Format(_T("세그먼트 %d 셀 맵에 셀 수가 %d개가 아닌 행이 %d개 있습니다.\n해당 행은 미체크로 표시되며, 저장하면 화면 값으로 덮어씁니다."), m_nViewSeg + 1, nCell, m_nMaskMismatch);\n\t\telse\n\t` (끝 탭이 기존 :810을 else 본문으로 들여쓴다) |
| C35 | `.cpp:822` `\tS_RecipeData* pRD = (m_pAuroraMgr != NULL && ...GetRecipeData() : NULL;\n` 뒤 (`IsMaskViewStale`) | 크기 비교가 활성 세그먼트 기준이라 seg2 그리드는 늘 stale로 거부된다 | `\tif (IsSegmented())\t// LYS 2026-09-30 Hybrid 3c A13 compare with the viewed segment (no SetActiveSegment)\n\t{\n\t\tC_OutRecipeFunc* pSegFunc = ...GetRecipeFuncPtr();\n\t\tS_RecipeMgr* pSegMgr = (pSegFunc != NULL && m_nViewSeg < pSegFunc->GetSegmentCount()) ? pSegFunc->GetSegmentMgr(m_nViewSeg) : NULL;\n\t\tpRD = (pSegMgr != NULL) ? &pSegMgr->m_sData : NULL;\t// segment gone -> size 0 -> stale\n\t}\n`. 레시피 ID 검사(:829) 그대로 |
| C36 | `.cpp:846` `\tCString strAsk;\n` 뒤 (`ConfirmMaskOverwrite`, :847은 CP949 한글). **C37보다 먼저 적용**(C37이 같은 줄을 새로 만든다) | 세그먼트를 말하지 않는다 | `\tif (IsSegmented())\n\t\tstrAsk.Format(_T("세그먼트 %d 셀 맵에서 셀 수가 맞지 않던 행 %d개를 지금 화면 값으로 덮어씁니다.\n계속 하시겠습니까?"), m_nViewSeg + 1, m_nMaskMismatch);\n\telse\n\t` |
| C37 | `.cpp:853` `void CDlgSubManual_AgingNew::DblClickFpspreadCellStatus(long Col, long Row)\n` 앞 | 선택기 함수 4개 없음 | 아래 코드 |
| C38 | `.cpp:413` ASCII 접두 앵커 `\tint nMaxCount = m_spreadAgingList->GetMaxRows();\t// HMI` 앞(`OnAgingProcStrart`, 파일 검사 :388-411 뒤. :593에도 같은 식이 있지만 주석이 없다) | 수동 START가 세그먼트를 모른다. 섹션이 없는 세그먼트는 조용히 전부 측정된다 | 아래 코드(D3c-6). `IsOfflineStatus`(:351)를 이미 통과했으므로 세그먼트 벡터를 읽어도 된다. Operate/CIM 시작(13:03 런)은 이 경로를 타지 않으므로 C24가 맡는다 |
| — | `.cpp:497-515` `IsOfflineStatus`(3b 가드), `:257-303` OnRcpOpen, `:584-620` SaveAgingInfo, `:106-186` OnInitDialog | — | **무변경.** 가드가 Open/Save/Load/START/세그먼트 전환을 막는다. SaveAgingInfo는 `[COUNT]/[ITEM_n]`(글라스 공통, A2)을 쓰고, 보고 있는 세그먼트만 MeasCellSave한다 |

```cpp
// C30 (LF)
		// LYS 2026-09-30 Hybrid 3c A13 recipe changed outside this dialog to a 1-segment recipe -> hide; hybrid run -> selector disabled
		if (m_cmbSeg.GetSafeHwnd() != NULL && (m_cmbSeg.GetStyle() & WS_VISIBLE))
		{
			if (IsSegmented() == FALSE) UpdateSegmentCombo();
			else
			{
				BOOL bIdle = IsHybridRunBusy() ? FALSE : TRUE;	// engine / cycle flags + segment count only, never the segment contents
				if ((m_cmbSeg.IsWindowEnabled() ? TRUE : FALSE) != bIdle) m_cmbSeg.EnableWindow(bIdle);
			}
		}

// C33 (LF, ASCII)
BOOL CDlgSubManual_AgingNew::MeasCellSave()	// LYS 2026-09-30 Hybrid 3c A13 FALSE = not written (stale / overwrite declined)
{
	if (m_spreadAgingMeas == NULL || m_spreadAgingMeas->GetSafeHwnd() == NULL) return TRUE;

	CString strEfaRcp = GetEfaRcpFile();
	if (strEfaRcp.IsEmpty()) return TRUE;
	CString strSec = CAgingCycleInfo::GetMaskSection(IsSegmented() ? m_nViewSeg : 0);	// segment 0 = MASECELL (unchanged)
	if (IsSegmented() && (IsMaskViewStale() || ConfirmMaskOverwrite() == FALSE)) return FALSE;	// segment-switch save; SaveAgingInfo already checked (no second popup)

	short nKeepSheet = m_spreadAgingMeas->GetSheet();

	m_spreadAgingMeas->SetSheet(1);
	int nCell = m_spreadAgingMeas->GetMaxCols();
	int nStick = m_spreadAgingMeas->GetMaxRows();
	if (nStick < 1 || nCell < 1) { m_spreadAgingMeas->SetSheet(nKeepSheet); return TRUE; }

	std::bitset<3> bit;
	CString strCellData;
	for (int i = 0; i < nStick; i++) {
		strCellData.Empty();

		for (int j = 0; j < nCell; j++) {
			bit.reset();
			for (int k = 0; k < 3; k++) {	// WAD, VISION, ANGLE
				m_spreadAgingMeas->SetSheet(k + 1);
				bit[k] = (m_spreadAgingMeas->GetDataInt(j + 1, i + 1) != 0);
			}
			strCellData += dfcStr(_T("%d,"), (int)bit.to_ulong());
		}

		WritePrivateProfileString(strSec, dfcStr(_T("STICK_%d"), i + 1), strCellData, strEfaRcp);
	}

	// hybrid only: delete this section's rows beyond the segment's stick count (the loader reads until the first missing key)
	if (IsSegmented())
	{
		TCHAR cOld[16];
		for (int i = nStick; i < DEFINE_AGING_CYCLE_STICK_MAX; i++)
		{
			ZeroMemory(cOld, sizeof(cOld));
			GetPrivateProfileString(strSec, dfcStr(_T("STICK_%d"), i + 1), _T(""), cOld, _countof(cOld), strEfaRcp);
			if (cOld[0] == 0) break;
			WritePrivateProfileString(strSec, dfcStr(_T("STICK_%d"), i + 1), NULL, strEfaRcp);
		}
		CString strLog;
		strLog.Format(_T("[HYBRID] EFA rcp %s: [%s] saved %d stick(s) x %d cell(s)"), (LPCTSTR)GetRecipeID(), (LPCTSTR)strSec, nStick, nCell);
		CLogWrapper::GetInstance()->KMLOG(OPER, strLog);
		m_nMaskMismatch = 0;
	}

	m_spreadAgingMeas->SetSheet(nKeepSheet);
	return TRUE;
}

// C34 ② (LF, ASCII)
	// LYS 2026-09-30 Hybrid 3c A13 segment k>=1 without its section: the EFA engine measures every cell, so show every cell checked (D4)
	if (IsSegmented() && m_nViewSeg >= 1)
	{
		ZeroMemory(cCellData, sizeof(cCellData));
		GetPrivateProfileString(strSec, _T("STICK_1"), _T(""), cCellData, EFA_CELLMASK_BUF, strEfaRcp);
		if (cCellData[0] == 0)
		{
			for (int k = 0; k < 3; k++)
			{
				m_spreadAgingMeas->SetSheet(k + 1);
				for (int i = 0; i < nStick; i++)
					for (int j = 0; j < nCell; j++)
						m_spreadAgingMeas->SetData(j + 1, i + 1, _T("1"));
			}
			m_spreadAgingMeas->SetSheet(nKeepSheet);
			CString strNo;
			strNo.Format(_T("[HYBRID] EFA rcp [%s] not found - shown all checked (the engine measures every cell)"), (LPCTSTR)strSec);
			CLogWrapper::GetInstance()->KMLOG(OPER, strNo);
			return;
		}
	}

// C37 (LF, ASCII except the one popup = CP949 bytes)
// LYS 2026-09-30 Hybrid 3c A13 segment selector for the EFA rcp cell mask (D6, same structure as the AgingTest dialog).
//   Hybrid only: grid = GetSegmentMgr(m_nViewSeg), save/load = CAgingCycleInfo::GetMaskSection(m_nViewSeg).
//   [COUNT]/[ITEM_n] (T0/T1) stay glass-common (A2). SetActiveSegment is never called here.
BOOL CDlgSubManual_AgingNew::IsSegmented()
{
	if (m_pAuroraMgr == NULL || m_pAuroraMgr->GetRecipeEditMgrPtr() == NULL) return FALSE;
	return (m_pAuroraMgr->GetRecipeEditMgrPtr()->GetSegmentLoopCount() > 1) ? TRUE : FALSE;
}

BOOL CDlgSubManual_AgingNew::IsHybridRunBusy()
{
	if (IsSegmented() == FALSE) return FALSE;
	if (CSequenceManager::GetInstance()->IsEngineCompleteAll() == false) return TRUE;
	return CSystemParamShare::GetInstance()->IsCycleState() ? TRUE : FALSE;
}

void CDlgSubManual_AgingNew::UpdateSegmentCombo()
{
	if (m_cmbSeg.GetSafeHwnd() == NULL) return;
	CString strRcpID = GetRecipeID();
	C_OutRecipeFunc* pFunc = (m_pAuroraMgr != NULL && m_pAuroraMgr->GetRecipeEditMgrPtr() != NULL) ? m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeFuncPtr() : NULL;
	int nSeg = (IsSegmented() && pFunc != NULL) ? pFunc->GetSegmentCount() : 1;
	if (strRcpID != m_strViewRcp || m_nViewSeg < 0 || m_nViewSeg >= nSeg) m_nViewSeg = 0;	// m_strViewRcp is stamped by LoadAgingInfo right before the grid is rebuilt
	m_cmbSeg.ResetContent();
	if (nSeg > 1)
	{
		CString str;
		for (int k = 0; k < nSeg; k++)
		{
			str.Format(_T("Seg %d/%d %s R%d (%d x %d)"), k + 1, nSeg, pFunc->GetSegmentName(k).GetString(), pFunc->GetSegmentRotate(k) * 90, pFunc->GetSegmentStickCount(k), pFunc->GetSegmentCellCount(k));
			m_cmbSeg.AddString(str);
		}
		m_cmbSeg.SetCurSel(m_nViewSeg);
	}
	m_cmbSeg.EnableWindow(TRUE);
	m_cmbSeg.ShowWindow((nSeg > 1) ? SW_SHOW : SW_HIDE);
	if (GetDlgItem(IDC_STATIC_EFA_SEG) != NULL) GetDlgItem(IDC_STATIC_EFA_SEG)->ShowWindow((nSeg > 1) ? SW_SHOW : SW_HIDE);
}

void CDlgSubManual_AgingNew::OnSelchangeSeg()
{
	int nSel = m_cmbSeg.GetCurSel();
	if (nSel < 0 || nSel == m_nViewSeg) return;
	if (IsSegmented() == FALSE || !IsOfflineStatus() || IsMaskViewStale())	// IsOfflineStatus also blocks a busy hybrid run; stale = recipe changed since LOAD (MultiMeas does the same)
	{
		m_cmbSeg.SetCurSel(m_nViewSeg);
		return;
	}
	CString strAsk;
	strAsk.Format(_T("세그먼트 %d 셀 맵을 먼저 저장하시겠습니까?\n예: 저장 후 전환 / 아니요: 저장하지 않고 전환 / 취소: 전환 취소"), m_nViewSeg + 1);
	int nAns = AfxMessageBox(strAsk, MB_YESNOCANCEL | MB_ICONQUESTION);
	if (nAns == IDCANCEL || (nAns == IDYES && MeasCellSave() == FALSE))	// mask of the segment being left; the T0/T1 list is saved by SAVE
	{
		m_cmbSeg.SetCurSel(m_nViewSeg);
		return;
	}
	m_nViewSeg = nSel;
	UpdateAgingCell();
	MeasCellLoad();
}

// C38 (LF, ASCII except the popup = CP949 bytes)
	// LYS 2026-09-30 Hybrid 3c A13 hybrid: report each segment's EFA mask section (the engine reads the file, not this grid)
	if (IsSegmented())
	{
		C_OutRecipeFunc* pSegFunc = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeFuncPtr();
		int nSegCnt = (pSegFunc != NULL) ? pSegFunc->GetSegmentCount() : 1;
		CAgingCycleInfo efaInfo;
		efaInfo.Load(strRcp, true, nSegCnt);
		CString strSegLog = _T("[HYBRID] EFA START masks:"), strOne, strNoMask;
		for (int k = 0; k < nSegCnt && pSegFunc != NULL; k++)
		{
			int nRows = efaInfo.GetStickCount(k);
			strOne.Format(_T(" seg%d [%s] rows %d/%d"), k + 1, CAgingCycleInfo::GetMaskSection(k).GetString(), nRows, pSegFunc->GetSegmentStickCount(k));
			strSegLog += strOne;
			if (nRows == 0) { strOne.Format(_T("%s%d"), strNoMask.IsEmpty() ? _T("") : _T(", "), k + 1); strNoMask += strOne; }
		}
		CLogWrapper::GetInstance()->KMLOG(OPER, strSegLog);
		if (strNoMask.IsEmpty() == FALSE)
		{
			CString strAskSeg;
			strAskSeg.Format(_T("세그먼트 %s에는 EFA 셀 맵이 없어 모든 셀을 측정합니다.\n계속 하시겠습니까?"), (LPCTSTR)strNoMask);
			if (AfxMessageBox(strAskSeg, MB_YESNO | MB_ICONQUESTION) != IDYES) return;
		}
	}
```

*(2g) 무변경 확인* (코드 그대로, 이유만 기록)
- `DoWorkGlassMeasure.cpp:284` Last-Stick 가드(A10). 13:10:26 Idle 1회로 확인했다.
- `EfaPassBoundary` 로직(A6)과 `SegmentBoundary`(:548-693).
- `IsEfaSkipStep`(:425-428). T1이 PtrnSearch를 건너뛰어도 오프셋을 저장하지 않으므로(`DoWorkGlassMeasureStep.cpp:1368/1400`의 로컬 `dPixelx`) R180 → R90 복귀 뒤에도 무해하다.
- T1 seg0는 seg N-1의 front lamp/CCD 값으로 돈다. T1 스텝이 front CCD를 쓰지 않으므로 무해하다(메모).
- `ToCellStatusStick`와 PG 수신 6곳(A8).
- `AgingCycleInfo.*`(A1 완료).
- DLL `ValidateSegments`(D3c-9).
- Resume :406(D3c-11).
- `DlgSubOperate`(Phase 6).
- `ShareMem.h`: VTM_SYSTEM 변경 없음. 세그먼트/pass 필드는 프로세스 밖 소비자가 생길 때만 CoreFunction 사본 끝에 추가한다.

</details>

**(3) 파일 포맷 — EFA rcp (`{EFA_RCP}\{RecipeID}.rcp`, 기본 `D:\HBS\PARAM_OCP\PARAM\EFARcp\`)**

```ini
[COUNT]
MaxCount=2
[ITEM_1]
USE_AGING=1
PTN_RGB=1
PTN_SEC=3
USE_PROPERTY=1
USE_QUALITY=1
USE_ANGLE=
[ITEM_2]
USE_ANGLE=1
[MASECELL]
STICK_1=5,5,5,
STICK_5=1,1,1,
[MASECELL_SEG2]
STICK_1=7,7,7,7,
STICK_5=1,2,4,7,
```
(예시는 줄여 쓴 것이다: `STICK_2..4` 줄을 생략했고, 실제 `[ITEM_2]`에는 빈 키 5개가 더 있다.)

- **세그먼트 0:** `[MASECELL]`은 바이트 단위 무변경(구 빌드·N=1 호환).
- **세그먼트 k≥1:** `[MASECELL_SEG{k+1}]`에 `STICK_1..스틱 수(k)`를 쓴다.
  - 행마다 셀 수(k)개 토큰, `%d,` 형식.
  - 3비트: bit0 WAD, bit1 VISION, bit2 ANGLE.
  - 행 = 세그먼트 로컬 레시피 스틱 순서, 열 = 스틱 안 셀 인덱스(런타임 `GetMeasureStick()`/`m_nArea`와 같은 프레임, 회전 재배치 없음).
- **섹션 이름의 출처는 하나다:** `CAgingCycleInfo::GetMaskSection(k)`(엔진 로더·AgingTest·AgingNew 공통).
- **`[COUNT]/[ITEM_n]`은 글라스 공통**(A2). 전 세그먼트가 같은 pass 수·항목을 쓴다. 선택기 전환 저장은 마스크만 쓰고, 항목 리스트는 SAVE만 쓴다.
- **섹션이 없을 때:** 행 0 = 전부 측정(D4, AgingRcp와 같은 규칙). WAD Multi(D3d-1 "base 점만")와는 **일부러 다르다** — 두 경우 모두 "그 기능의 기본 측정"을 따른 결과다.
- **로더는 첫 빈 `STICK_n`에서 멈춘다.** 그래서 하이브리드 저장은 세그먼트 스틱 수를 넘는 키를 키 단위로 지운다. 섹션 통째 삭제는 금지(섹션이 파일 끝으로 옮겨진다).
- **고아 섹션**(k ≥ 세그먼트 수)은 읽지 않고 남긴다(3b 선례).
- **구 빌드·OCP**는 `[MASECELL]`만 읽는다. `WritePrivateProfileString`은 자기 키만 고치므로 구 빌드로 저장해도 접미사 섹션이 남는다.
- **AgingRcp 폴더**(`AgingCycleInfo.cpp:23-24`)와는 경로가 달라 섹션 이름이 같아도 충돌하지 않는다.

**(4) API 계약**

```cpp
// Recipe/RecipeEditMgr.h (CP949 CRLF) - LYS 2026-09-30 Hybrid Phase 3c
CString	GetSegLogTag(BOOL bRowBase = FALSE) const;	// "" when the recipe has <= 1 segment, else " seg(k/N)" (+ " rowbase(b)")
int		GetSegRowBase() const	{ return m_nSegRowBaseCache; }	// Phase 4 segBase preview - printed only, never added to a row in 3c
void	SetEfaGlassLatch(int nPass)	{ m_nEfaGlassLatch = nPass; }	// measure thread, once per glass (engine AgingGlassLatch)
int		GetEfaGlassLatch() const	{ return m_nEfaGlassLatch; }	// -1 = not latched (1-segment recipe / after RecipeChange), 0 = not EFA, n = EFA passes
// DoWork/MeasureSequenceItemEngine.h
void	AgingGlassLatch(int nEfaPassCount = 0);	// + publishes the EFA glass latch (N>1), sets -1 (N<=1)
```

- **캐시 3개**(`m_nSegCountCache/m_nActiveSegCache/m_nSegRowBaseCache`)는 `SyncCellStatusStickBase`에서만 쓴다.
  - 쓰는 곳: RecipeChange(UI·시퀀스 스레드, 하이브리드 측정 중에는 다이얼로그 가드로 막힘)와 `SegmentSwitchData`(측정 스레드).
  - 읽는 곳: 어느 스레드든 int 읽기만 한다. 세그먼트 벡터는 건드리지 않는다.
  - `SetActiveSegment`가 실패하면 캐시도 그대로라 실제 활성 세그먼트와 계속 같다.
- **래치 수명:** RecipeChange에서 -1 → 측정 시작 `AgingGlassLatch`가 설정(N=1이면 -1) → `MeasDone`이 읽는다.
  - 다음 글라스의 RecipeChange가 먼저 오면 -1이 되어 기존 게이트로 돌아간다(오늘 동작과 같다).
- **스틱 헤더의 세그먼트 이름**은 측정 스레드에서만 `GetSegmentName(k)`(헤더 벡터)로 읽는다. 태그에는 이름을 넣지 않는다(PG/UI 스레드 안전).
- **다이얼로그**는 `IsOfflineStatus`(하이브리드 busy 차단)를 통과한 뒤에만 `GetSegmentMgr(m_nViewSeg)`를 읽는다. 500 ms 타이머는 세그먼트 수(벡터 크기)와 엔진 플래그만 읽는다(3b와 같은 패턴).

**(5) 로그 사양** (N=1은 모두 기존 문자열 그대로. 아래는 13:03 조건을 다시 돌렸을 때 N>1에서 기대하는 출력)

| 파일 | 줄 (N>1) | 횟수 |
|---|---|---|
| GLASS 6_MEASURE | WARN `[HYBRID] result rows are segment-local until Phase 4 (segBase): seg1 rows 1..15 seg2 rows 1..20 - an overlapping row keeps the last segment's value` | 글라스 1 |
| 〃 | `EFA: pass 1/2 start (T0) - 2 segments, segment 1 first` / `EFA: pass 2/2 start (T1) - 2 segments, segment 1 first` | pass당 1 |
| 〃 | OPER `[HYBRID] seg 1/2 [LONG_H1] stick 1/5 (glass stick 1/10) pass 1/2 (T0)` … `[HYBRID] seg 2/2 [SHORT_H2] stick 5/5 (glass stick 10/10) pass 2/2 (T1)` | 스틱마다(이번 런 20) |
| 〃 | `Probe Contact stick=1 offset=0.0000 -> Z5 target=4.2000 (End 4.0000 + OD 0.2000) seg(2/2)` | 스틱마다 |
| 〃 | `AgingCycle row: WAD item(1) repeat(0) stick(0) row(0,1) seg(1/2) rowbase(0)` / `AgingCycle row: WAD item(1) repeat(0) stick(4) row(0,1) seg(2/2) rowbase(15)` | 행 로그 전부(WAD 29, VISION 26, ANGLE 26) |
| 〃 | `CDoWorkGlassMeasureStep_AGING_PTN Start. item(1) ptn(1) sec(3) seg(1/2) pg(9_OCP_TEST_Hybrid_Half_260930)` | T0 스틱마다 |
| 〃 | `EFA pass 2: skip step Pattern Search seg(1/2)` | T1 스틱마다 3 |
| 〃 | `EFA: pass boundary before pass 2 (T0 done) - result flush + CIM handshake (all 2 segments)` / `EFA: T0 result flush OK - hybrid: rows of the segments overlap until Phase 4 (segBase)` | 1 |
| 엔진 | `[HYBRID] AgingTest flag latched from segment 1 for this glass: 0 (unused - EFA glass)` / `[HYBRID] EFA latched for this glass: passes 2 (0 = not EFA) - EFA^1$ at MeasDone follows this` | 글라스 1 |
| 엔진 | `EFA: mask [MASECELL_SEG2] not found - segment 2/2 measures every cell of the item's kinds (no mask = measure all)` | seg2 첫 스틱 1(T1 재진입 없음) |
| 엔진 | `EFA: begin pass 1/2 seg(2/2)` / `AgingCycle: end seg(2/2)` (마지막 스틱 end도 seg(2/2) — 캐시) | 스틱마다 |
| DLOG(레시피 스텝) | SYST `[HYBRID] capacity: sticks 10/50, result rows 35/2500 (G 35 x block 1; K wad/vis/ang 1/1/1, WAD points 1) (EFA 2 passes: K = 1, T1 overwrites T0)` | 1 |
| 〃 | SYST `[HYBRID] EFA mask rows per segment: seg1 [MASECELL] 5/5 seg2 [MASECELL_SEG2] none (none = every cell measured)` + WARN `[HYBRID] EFA mask [MASECELL_SEG2] not found - segment 2/2 measures every cell of each item's kinds (no mask = measure all)` | 1 + 섹션 없는 세그먼트마다 |
| 〃 (조건부) | WARN `[HYBRID] EFA PTN index goes to PG models seg1 [A] / seg2 [B] - their pattern tables must match (A12)` / WARN `[HYBRID] EFA: AGING (m_bUseAgingTest_2) of segment(s) 2 differs from segment 1 (1) - aging runs only where it is on, T1 still re-measures every segment` | 조건 성립 시 |
| tact 이력 | `T0_Seg1_Stick1_WAD`, `T0_Seg2_Stick1_WAD`, `T1_Seg1_Stick1_WAD`, `T1_Seg2_Stick1_WAD` (비-EFA 하이브리드: `Seg2_Stick1_WAD`) | — |
| SEQ CIM (ONLINE만) | SYST `CDoWorkSequenceCIMStep_MeasDone : [HYBRID] EFA glass latch 2 pass(es) - no flag / file re-read` → 기존 `... : EFA glass - send EFA^1$` | 글라스 1 |
| OLOG/OCP (다이얼로그) | OPER `[HYBRID] EFA rcp 9_OCP_TEST_Hybrid_Half_260930: [MASECELL_SEG2] saved 5 stick(s) x 4 cell(s)`, OPER `[HYBRID] EFA START masks: seg1 [MASECELL] rows 5/5 seg2 [MASECELL_SEG2] rows 0/5`, OPER `[HYBRID] EFA rcp [MASECELL_SEG2] not found - shown all checked (the engine measures every cell)`, WARN `[HYBRID] EFA rcp [MASECELL_SEG2]: n row(s) with cell count != 4 shown unchecked (...)` | 조작 시 |

**(6) 다이얼로그 설계 (`IDD_DLG_SUB_MANUAL_AGING_NEW`, `CDlgSubManual_AgingNew`)**

- **컨트롤:** `IDC_STATIC_EFA_SEG`(21009) "Segment" 134,56,30,8, `IDC_COMBO_EFA_SEG`(21008) 166,53,160,120. 둘 다 `NOT WS_VISIBLE`이고 하이브리드 레시피를 LOAD/Open했을 때만 보인다.
- **콤보 항목:** `Seg %d/%d %s R%d (%d x %d)`. 예: `Seg 1/2 LONG_H1 R90 (5 x 3)` / `Seg 2/2 SHORT_H2 R180 (5 x 4)`. MultiMeas처럼 R을 넣어 장변/단변을 구분한다(D3c-12).
- **흐름**
  - **Open / LOAD:** `UpdateSegmentCombo`(레시피가 바뀌었으면 보기 0, 아니면 범위 클램프) → 표식 `m_strViewRcp` → `UpdateAgingCell`(보기 세그먼트 크기) → `MeasCellLoad`(그 세그먼트 섹션).
  - **SAVE:** `IsMaskViewStale`(보기 세그먼트 크기) → `ConfirmMaskOverwrite` → `[COUNT]/[ITEM_n]` → `MeasCellSave`(보기 세그먼트 섹션만, 초과 키 삭제).
  - **세그먼트 전환:** 가드 → stale 검사 → 한글 예/아니요/취소. 예는 떠나는 세그먼트의 마스크만 저장하고, 실패하면 전환을 취소한다 → 새 세그먼트 그리드·섹션.
  - `SetActiveSegment`는 어디서도 부르지 않는다.
- **새 팝업 4종, 한글, 파일이 CP949라 CP949 바이트로 넣는다**(`Encode::encode("cp949")`, C++ 리터럴 안 `\n`은 두 글자):
  1. 불일치 경고(C34⑤)
  2. 덮어쓰기 확인(C36)
  3. 전환 확인(C37)
  4. START 섹션 누락 확인(C38)
  - 문구는 AgingTest 3b와 같고, 4번만 새 문구다. 기존 영어 팝업(START 사전 검사 등)은 그대로 둔다.
- **가드 대응표**

| 3b 이름 | 3c AgingNew |
|---|---|
| 측정 중 차단 | 기존 `IsOfflineStatus` 3b 블록(:506-512)이 Open/Save/Load/START/전환을 막는다. 타이머가 busy 동안 콤보를 비활성화한다(C30) |
| 레시피 변경 | `IsMaskViewStale` 세그먼트 인식(C35). 전환 전에도 검사한다(C37) |
| 불일치 행 | 로드 경고(C34), 저장 확인(C36, `MeasCellSave`의 하이브리드 경로에서도) |
| stale 키 | `MeasCellSave` 하이브리드만 `DEFINE_AGING_CYCLE_STICK_MAX`까지 삭제(C33) |
| k≥1 섹션 없음 | 전부 체크로 표시(C34②, D3c-13). 세그먼트 0은 N=1과 같은 표시 |

- **범위 밖:** AgingNew T0/T1 하이라이트는 `GetSys_NewAgingMode()`일 때만 켜진다(기존 N=1 동작). 레시피 플래그로 시작한 EFA(13:03 런)에서는 켜지지 않는다. 세그먼트/pass 진행 표시는 Phase 6(D3c-14).

**(7) 용량 검사**

- **식은 그대로다:** `rows = G × max(K_wad × P, K_vis, K_ang) ≤ M_RESULT_AREA_MAX(2500)`, 총 스틱 ≤ 50.
- **3c가 바꾸는 것은 K 하나다.** 레시피 EFA 플래그(세그먼트 0)가 켜져 있고 EFA rcp에 항목이 있으면 K = {1,1,1}.
  - 근거: EFA는 pass마다 항목 1개(`AgingCycleBegin(nPass)`가 스틱마다 `m_nAgingRepeat`를 리셋, 엔진 :795)라 블록이 0이다. T1은 같은 행을 덮는다.
  - P(WAD Multi)는 3d 규칙 그대로다. EFA T0/T1의 WAD 스텝도 `K + rep×(P_k-1)`이 아니라 표준 경로 블록 `ip`를 쓰므로 `G × max P_k`가 상한이다.
- **완화만 한다:** 공유 플래그만 켜진 경우는 기존처럼 AgingRcp K로 보수적으로 계산한다. 거부 조건은 이것 말고 그대로다.
- **EFA 마스크·A12·AGING 플래그 검사(C24)는 WARN만** 하고 거부하지 않는다. N=1에는 없는 검사다.

**(8) N=1 불변 근거**

| 변경 | N=1 실행 경로 | 관측 결과 |
|---|---|---|
| C1–C6 구조 | `SyncCellStatusStickBase`가 count 1 / active 0 / rowbase 0을 쓴다. `GetSegLogTag`는 `m_nSegCountCache <= 1`이라 빈 문자열. 래치는 ctor·RecipeChange에서 -1 | 로그 없음, 동작 동일 |
| C7, C9, C10, C12, C13, C14 | 각각 `nSegCount > 1`, `GetSegmentLoopCount() > 1`, 하이브리드 전용 `SegmentRestore0` 조건. C12의 N=1 분기는 원문 `strName.Format(_T("Stick%d_%s"), i + 1, pSeqItem->GetName())` 그대로 | 줄 없음, tact 이름 동일 |
| C11, C15–C18 | 형식 끝 `%s`에 빈 문자열(C18은 태그가 빌 때 pg도 붙지 않는다) | 바이트 동일. 3b 회귀 행 `row(31,21)`, `row(59,49)` 포함 |
| C19 | `m_strCycleSegTag`가 빈 문자열. 대입·Empty는 로그가 아니다 | `EFA: begin pass 1/2`, `AgingCycle: begin, items(..) active(..)`, `AgingCycle: end` 동일 |
| C20 | `GetSegmentLoopCount() > 1` 조건. N=1 "섹션 없음 = 전부 측정"은 조용하다 | 동일 |
| C8, C21 | N<=1: `SetEfaGlassLatch(-1)` 후 기존 return. 로그 두 줄은 N>1에서만 도달 | 엔진 로그 동일 |
| C22 | 래치 -1 → `else if` = 원래 조건식 그대로 | CIM 동작·로그 동일 |
| C23, C24 | `GetSegmentCount() > 1` 블록(`DoWorkGlassRecipeStep.cpp:439`) 안에서만 호출 | 호출 안 됨 |
| C25, C26 | 새 ID(미사용 값), 새 컨트롤 `NOT WS_VISIBLE` | 화면 동일 |
| C27–C38 다이얼로그 | `IsSegmented()` FALSE: 섹션 `MASECELL`, 크기는 `GetRecipeData()`(유휴 = 세그먼트 0), `MeasCellSave`의 추가 검사·키 삭제·로그 없음(쓰는 바이트 동일), 로드 로그 접두 빈 문자열(`EFA rcp [MASECELL]: ...` 그대로), 팝업은 else = 원문, 타이머는 콤보 숨김이라 `WS_VISIBLE` 단락, START 블록 건너뜀, `UpdateSegmentCombo`는 숨은 컨트롤을 `SW_HIDE`만 | 화면·팝업·파일 동일 |

**(9) 결정** (권장안에 **굵게**. 사용자 변경 가능)

| # | 항목 | 선택지 | 권장 |
|---|---|---|---|
| D3c-1 | EFA 래치 위치 | `CRecipeEditMgr` volatile int / `CSystemParamShare` / 엔진 멤버 | **`CRecipeEditMgr`**. 측정 스레드와 `MeasDone`이 이미 이 포인터를 쓰고, 글라스마다 지나는 RecipeChange에서 리셋할 수 있으며, 3b stick base와 같은 패턴이다 |
| D3c-2 | `EFA^1$` 의미 | "EFA 글라스"(중단돼도 보낸다, N=1과 같음) / "T1 완료 시만" | **EFA 글라스.** N=1과 의미가 갈라지지 않는다 |
| D3c-3 | 태그 형식 | 줄 끝 ` seg(k/N)`, 행 로그만 ` rowbase(b)` / 줄 앞 `[S2]` / 세그먼트 이름 포함 | **줄 끝.** grep하기 쉽고 N=1이 구조적으로 바이트 동일. 이름은 스틱 헤더에만 |
| D3c-4 | tact 이름 | N>1 `Seg%d_Stick%d_%s`, EFA `T%d_` 접두 / 무변경 | **N>1만 변경.** 저장소 안 파서 없음, 매칭은 동등 비교. N=1 EFA의 T0/T1 구분은 N=1 변경이라 제외 |
| D3c-5 | EFA k≥1 섹션 없음 | 전부 측정 + 시작 WARN·런타임 OPER / 세그먼트 skip / 자동 시작 거부 | **전부 측정 + 로그**(D4 유지, AgingRcp와 같은 규칙) |
| D3c-6 | 수동 START 섹션 없음 | 로그 + 한글 예/아니요 / 로그만 | **로그 + 확인**(수동 START만. Operate/CIM 시작은 C24 WARN) |
| D3c-7 | Phase 4 전 하이브리드 결과 | WARN + 운용 제한(D3d-8과 같음) / 하이브리드 ONLINE REMOTE 자동 시작 거부 | **WARN + 운용 제한.** 행 충돌은 EFA와 무관하게 모든 하이브리드 결과에 있어 EFA만 막을 근거가 약하다. 막는다면 "하이브리드 ONLINE REMOTE 전체"로 |
| D3c-8 | segBase | Phase 4 유지 / 3c로 앞당김 | **Phase 4.** 행 깔때기가 하나가 아니다(Vision은 장치 인덱스·quality-wait 경로). Resume 재적재 식(`DoWorkGlassRecipeStep.cpp:128`)과 WAD-MP 식(:3021-3027)이 갈라진다. Cell ID 중복·APD 헤더가 남는다. OOS로 검증할 수 없다 |
| D3c-9 | 세그먼트별 `m_bUseAgingTest_2` | 자동 시작 WARN / DLL `ValidateSegments` 저장 거부 추가 | **WARN**(3c는 ALPHA만, DLL 재빌드 없음). Elect/RTest 불일치는 T0/T1이 일관돼 검사하지 않는다 |
| D3c-10 | A12 | WARN / 세그먼트별 PTN | **WARN.** 세그먼트별 PTN은 Long/Short PG 모델 패턴 표를 확인한 뒤 2차 |
| D3c-11 | 하이브리드 EFA Resume | Phase 5(3c 무변경) / N>1은 pass 0·seg 0에서만 진행 저장 | **Phase 5.** 부분 가드는 Resume 팝업의 스틱 번호가 실제 재개 위치와 달라지게 만든다. 운용 지침: 하이브리드 EFA 글라스가 중단되면 "No(새로)" |
| D3c-12 | 다이얼로그 세부 | 콤보에 R, busy 시 콤보 비활성, 전환 전 stale 검사(MultiMeas와 같음) / AgingTest와 같게(셋 다 없음) | **MultiMeas와 같게.** AgingTest와의 차이는 3개다 |
| D3c-13 | 섹션 없음 표시 | k≥1만 전부 체크(AgingTest와 같음) / 하이브리드면 세그먼트 0도 | **k≥1만.** 세그먼트 0 `[MASECELL]` 없음은 N=1과 같은 표시(저장하면 전부 제외가 되는 기존 함정은 D3 묶음) |
| D3c-14 | 결과 업로드 로그·진행 표시기 | 3c 포함 / 뒤로 | **업로드 로그(`SetMeasUseInfo` 세그먼트 스냅샷, 실기 전용)는 Phase 4, AgingNew/운전 화면 세그먼트·pass 표시기는 Phase 6** |

**(10) 검증 (OOS)**

1. **N=1 회귀(첫 번째, 필수)**
   - 대상: `1_OCP_TEST_Long_260907` + EFA 2패스(기준 = 09-30 10:08 런), 같은 레시피 비-EFA AgingTest(기준 = 10:29 런).
   - 로그: GLASS 6_MEASURE / 엔진 / DLOG에서 시각을 뺀 3c 전후 diff가 0이어야 한다(`row(31,21)`, `row(59,49)`, `row(60,50)` 포함).
   - `[HYBRID]`·` seg(`·`rowbase(` 0건, tact 이름은 `Stick%d_`만.
   - AgingNew: 콤보가 숨어 있고, LOAD → SAVE 전후 EFA rcp 해시가 같고, 새 팝업이 없다.
2. **하이브리드 EFA 재실행(13:03 조건 그대로, `[MASECELL_SEG2]` 없음)**
   - (5)의 N>1 줄이 전부 나온다: 스틱 헤더 20, 행 로그 81줄 전부 태그(seg1 `rowbase(0)`, seg2 `rowbase(15)`), 엔진 `begin/end` 20쌍 태그(마지막 end도 `seg(2/2)`).
   - 섹션 없음: 레시피 스텝 WARN 1 + 엔진 OPER 1(T1 재진입 때 반복 없음).
   - 래치 2줄, 용량 EFA 꼬리, 행 WARN 1.
   - tact `Stick1_WAD` 4개가 `T0_Seg1_/T0_Seg2_/T1_Seg1_/T1_Seg2_`로 구분된다.
   - 측정 셀·행·경계 시간은 13:03과 같다(동작 무변경).
3. **AgingNew 편집(하이브리드)**
   - LOAD → 콤보 `Seg 1/2 LONG_H1 R90 (5 x 3)` / `Seg 2/2 SHORT_H2 R180 (5 x 4)`.
   - seg2 보기: 5×4 그리드 전부 체크, OPER "shown all checked".
   - 편집 후 전환 "예" → `[MASECELL_SEG2]` STICK_1..5(4토큰)가 생기고 OPER saved 줄이 찍힌다. `[MASECELL]`·`[COUNT]`·`[ITEM_n]` 줄은 diff 0.
   - `[MASECELL_SEG2]`에 손으로 `STICK_6=` 추가 → 저장 뒤 삭제된다.
   - 3토큰 행 → 한글 경고 + WARN `[HYBRID] EFA rcp [MASECELL_SEG2]: 1 row(s) ...`, 저장 시 확인.
   - 다른 화면에서 레시피 변경 → 전환·저장이 stale 팝업으로 거부된다.
4. **seg2 마스크 적용 런**(예 STICK_1..5 = `1,2,4,7,` 류): seg2가 마스크 셀만 잰다. 섹션 없음 WARN 없음, 요약 `seg2 [MASECELL_SEG2] 5/5`.
5. **AgingNew START(섹션 없음):** 한글 확인이 뜬다. 아니요 → 시작하지 않고 `EFA cycle test start` 로그가 없다. 예 → `[HYBRID] EFA START masks: ...` 뒤에 시작한다.
6. **가드:** 하이브리드 런 중 500 ms 안에 콤보가 비활성화된다. 선택·Open·Save·LOAD·START가 막히고(3b 한글 팝업), RecipeChange 로그가 없다. 런이 끝나면 콤보가 다시 활성화된다.
7. **용량:** 하이브리드 + seg0 `m_bUseAging_Test`=1 + AgingRcp WAD 항목 2개 + 레시피 EFA → K = 1 + EFA 꼬리. EFA를 끄면 K = 2(기존).
8. **A12·AGING 플래그:** seg2 헤더 PG 이름을 다르게 → A12 WARN. seg2 AgingTest_2 = 0 → WARN, 런에서 seg2 T0 AGING skip·T1 재측정(기대 동작).
9. **실기 ONLINE REMOTE(OOS 불가, EFA 문서 §9.2와 함께):**
   - `EFA^0$` 1회(EfaPassBoundary), `MeasDone` 래치 줄 + `EFA^1$` 1회 → STS1106.
   - N=1 ONLINE은 래치 줄 없이 기존 줄.
10. **Phase 4 전 기대 동작으로 기록:** 행 충돌(seg2가 seg1 행 1..15를 덮는다)과 Cell ID 중복은 3c 뒤에도 그대로다. `rowbase(15)` = Phase 4에서 seg2가 갈 자리.

**(11) 수정 파일·인코딩** (2026-09-30 Perl로 EOL/BOM/damaged 확인, 전부 damaged=0)

| 파일 | 변경 | EOL/인코딩 |
|---|---|---|
| `Source/ALPHA/Recipe/RecipeEditMgr.h` / `.cpp` | C1–C6 | CP949 CRLF (150 / 2416줄). `.h`를 고치면 넓게 재컴파일된다 |
| `Source/ALPHA/DoWork/DoWorkGlassMeasure.cpp` | C7–C14 | CP949 CRLF (693줄) |
| `Source/ALPHA/DoWork/DoWorkGlassMeasureStep.cpp` | C15–C18 | CP949 CRLF (7023줄). 비슷한 블록이 반복되므로 앵커 2회 `index` 검사 |
| `Source/ALPHA/DoWork/MeasureSequenceItemEngine.h` / `.cpp` | C19–C21 | CP949 CRLF |
| `Source/ALPHA/DoWork/DoWorkSequenceCIMStep.cpp` | C22 | CP949 CRLF |
| `Source/ALPHA/DoWork/DoWorkGlassRecipeStep.cpp` | C23, C24 | CP949 CRLF (887줄). 13:19 동시 편집, 직전 재확인 |
| `Source/ALPHA/DlgSubManual_AgingNew.h` / `.cpp` | C27–C38 | **CP949, LF-only(88 / 889 LF, CR 0), BOM 없음**, 한글 쌍 128 / 807. 루트 CLAUDE.md LF-only 목록에 없고, §10의 "ASCII"는 틀렸다. LF 정규화 후 `\n`으로 패치, 새 한글 4종은 `Encode::encode("cp949")` 후 되디코딩 검증 |
| `Source/ALPHA/ALPHA.rc` | C26 | **UTF-16 LE BOM, CRLF.** python은 깨진 스텁이라 Perl `Encode` UTF-16LE를 쓴다: BOM 유지, `FB_CROAK \| LEAVE_SRC`, CRLF 수 = LF 수, 앵커 유일, 파일 증가 확인(이전 세션 `p3d_patch.pl` ALPHA.rc 블록 방식) |
| `Source/ALPHA/resource.h` | C25 | UTF-8 BOM, CRLF, ASCII 삽입(raw) |
| (무변경) `AgingCycleInfo.*`, `CommTcpIpObjectPG/WAD.cpp`, DLL 전부, `DlgSubManual_AgingTest.*`, `DlgSubManual_MultiMeas.*`, `DlgSubOperate.*`, `ShareMem.h` | — | — |
| `docs/HybridGlass_Porting_Plan.md` | §5.9 A5/A6/A10/A12 상태, §10에 3c 행 추가, §10 `DlgSubManual_AgingNew.cpp` → "CP949 LF-only", §10 `DlgSubManual_AgingTest.cpp` → "UTF-8 BOM, CRLF"(stale 정정) | UTF-8 |
| 루트 `CLAUDE.md` LF-only 목록에 `DlgSubManual_AgingNew.h/.cpp` 추가 | 제안만(`HMI/` git 밖이므로 사용자 결정) | UTF-8 |

- 전부 `HMI/` git 트리다. 구현 뒤 `git -C HMI status`로 확인하고, 커밋 여부는 사용자에게 묻는다.

**(12) Phase 4 전까지 틀린 채 남는 것** (3c로 고쳐지지 않음, 로그로만 드러남)

1. **결과 행**
   - 식은 `row = cell + nArea_k×nMeasStick + 1 + G×block`이고 segBase가 없다. 세그먼트 k의 행 1..(셀×스틱)_k가 앞 세그먼트 행을 덮는다.
   - 13:03 런: T0 플러시(`RESULT\DATA\30\_20260930-130753.xls` / `APD_20260930-130753.txt`)와 최종(`_131026`) 모두 행 1..15에는 seg2 값만, 16..20도 seg2, 21..35는 빈 행이다. seg1 값은 하나도 남지 않는다.
   - EFA는 같은 식이라 "T1이 T0를 덮는다"는 규칙은 맞다. 하지만 T1 seg0가 seg N-1의 T0 값이 있는 행을 덮고 seg N-1 T1이 그 종류를 다시 재지 않으면, 한 행에 다른 셀의 T0/T1 값이 표식 없이 섞인다.
   - 3c의 `rowbase(b)`는 Phase 4에서 그 세그먼트가 갈 자리를 보여 줄 뿐이다.
2. **Cell ID:** 세그먼트마다 A01부터 시작해 WAD/Vision/Angle PC에 같은 ID가 다시 간다(13:03: seg1 9개 중 6개).
3. **APD 헤더:** CELL_X_CNT/CELL_Y_CNT = 세그먼트 0(3×5, `stGlassInfo`, `DoWorkGlassRecipeStep.cpp:432-433`)인데 글라스는 35셀이다. AREA_MAX 2500.
4. **결과 저장 게이트:** Result/Unload가 복귀한 세그먼트 0의 검사 항목만 본다(세그먼트 0이 전기 항목만이면 저장 생략 + bypass).
5. **(Phase 5) Resume:** 세그먼트 0만 진행을 저장하고, T1 seg0가 LastStick을 덮는다. T0 seg k≥1 중단은 전체 재시작, T1 seg0 중단은 T0 seg0 스틱 j부터라 T0/T1이 섞인다.
6. **(Phase 6)** 운전 화면 `Segment n/N` 텍스트와 T0/T1 표시가 없다. 셀맵 오버레이의 흰 테두리만 있다.

**(13) 리스크**

| 리스크 | 대응 |
|---|---|
| 하이브리드 로그 형식 변경이 외부 파서에 영향 | 하이브리드 로그는 09-29부터 새로 생겨 소비자가 없다. N=1은 바이트 동일(검증 1) |
| 마지막 `AgingCycle: end`가 복귀 뒤 세그먼트로 찍힘 | 사이클 시작 시 태그를 캐시(C19) |
| 래치 수명: 다음 글라스 RecipeChange 뒤 `MeasDone` | -1 → 기존 게이트(오늘과 같다). 다음 글라스 ThreadRun이 이전 `MeasDone`보다 먼저 올 수는 없다(Load·흡착·Align 선행) |
| AgingNew CP949 LF 파일에 한글 삽입 | LF 정규화 패턴, cp949 인코드 후 되디코딩, 한글 쌍 수 증가분 = 새 문구 확인, damaged=0 |
| DDX만 들어가고 rc 컨트롤이 없으면 assert | C25·C26·C29를 같은 스테이지·같은 빌드로 |
| ALPHA.rc UTF-16 손상 | Perl Encode 경로(3d 선례), 적용 전후 디코드 줄 수 +2 확인 |
| 시작 WARN과 런타임 OPER 중복 | 레벨을 나눈다(WARN = 시작 전 조치, OPER = 세그먼트 문맥) |
| UI 타이머가 RecipeChange와 동시에 세그먼트 수를 읽음 | 벡터 크기 읽기만(3b와 같은 패턴). 내용은 읽지 않는다 |
| `DoWorkGlassRecipeStep.cpp` 동시 편집 | 패치 직전 재디코드, 앵커는 줄 번호가 아니라 문자열 |
| K 완화로 과소 계산 | 레시피 플래그 기준만 완화. 공유 플래그만 켜지면 보수적 AgingRcp K 유지 |

**(14) 구현 단계** (스크립트 `p3c_patch.pl --stage=N`, 백업 `p3c_bak`. 이전 세션 scratchpad의 `pfw.pl`(rd/rep1/ins_after/ins_before)을 이 세션 scratchpad로 복사해 쓴다)

- **3c-0 구조 + 로그**(C1–C7, C9–C20): 런타임만, UI 없음. 단독 빌드 가능. 검증 1·2.
- **3c-1 A5 래치**(C8, C21, C22): C1–C5에 의존. C8과 C21은 같은 빌드. 검증 2의 래치 줄, 9(실기).
- **3c-2 사전 검사**(C23, C24): 검증 2·7·8.
- **3c-3 AgingNew 선택기**(C25–C38)
  - rc·resource·.h·.cpp를 원자 적용한다. 적용 순서: C36을 C37보다 먼저.
  - 검증 3–6.
- **공통 절차**
  - 편집 전 `.bak_20260930_p3c`.
  - 앵커마다 `index` 2회로 유일성 확인, 치환 횟수 확인.
  - 편집 뒤 damaged=0, 엄밀 디코드(CP949 / UTF-8 / UTF-16LE, 사본으로 `FB_CROAK`), EOL 종류·수 확인.
  - 바뀐 블록을 `sed -n`으로 다시 읽는다.
- **빌드**
  - ALPHA가 떠 있으면 `ALPHA.vcxproj //t:ClCompile`로 문법만 먼저 확인하고, 종료를 요청한 뒤 전체 빌드한다(루트 CLAUDE.md HMI 명령, `2022/Community`).
  - DLL 변경은 없다.


### Phase 3d — WAD Multi Point × 세그먼트 (IDD_DLG_SUB_MANUAL_MULTI_MEAS 포함)

상태: ✅ **구현·컴파일 완료(2026-09-30)**, 링크(ALPHA.exe 종료 필요)·OOS 대기. 플랜(워크플로 매퍼 3 → 플래너 → 반박 검토 2, 검토 지적 11건 반영) 그대로 3d-0~3d-3을 한 번에 적용했다 — 스크립트 `scratchpad/p3d_patch.pl`(8개 파일 원자 적용, 백업 `p3d_bak`), 전체 재컴파일 오류 0. 결정 D3d-1~8은 **전부 권장안으로 확정**(사용자 지시 2026-09-30, 아래 (8)). Phase 3b D7("WAD Multi Point `[MASECELL]` 세그먼트 충돌 = A15로 Phase 4")을 이 Phase로 옮긴다. 결과 행 `segBase`는 그대로 Phase 4에 남긴다.

**구현 메모(플랜과 다른 점·추가한 점)**:
- B18: 하이브리드 뷰가 아닌데 로드된 레시피가 하이브리드면(다른 화면에서 연 경우) 1세그먼트 Save 경로도 한글 stale 팝업으로 막는다(1세그먼트 레시피가 로드돼 있으면 조건 불성립 → N=1 무변경).
- 정보 라벨의 found/none은 매니저 상태가 아니라 **세그먼트를 적재한 시점의 값**(`m_bViewDataFound/m_bViewMaskFound`)을 쓴다 — 런이 끝나면 공유 매니저에는 마지막으로 잰 세그먼트가 남기 때문. 저장은 그리드 → 매니저(`SaveToMgr`) → 파일이라 매니저 잔류와 무관.
- 500 ms 타이머의 라벨 갱신은 문자열이 바뀔 때만 `SetWindowText`(깜박임 방지).
- 새 로그: SYST `WAD Multi Point mode active (segment k/n, points p)`(N>1만, N=1은 기존 문구), SYST `[HYBRID] WAD Multi segment k/n: [DATA_SEGk] not found - base point only`(세그먼트 첫 적재 1회), 용량 로그 끝 `(per segment a/b)`, 자동 시작 WARN `[HYBRID] WAD Multi segment k/n: ...`(B11 4종), 다이얼로그 WARN `[HYBRID] WAD Multi [MASECELL_SEGk]: ...`, OPER `[HYBRID] WAD Multi saved ...`.

**(1) 목적·범위**

- **지금 무엇이 깨지는가.** WAD Multi 파일은 레시피마다 하나다(`{WAD_MULTI_RCP_PATH}{RecipeID}.rcp`, 기본 `D:\HBS\PARAM_OCP\PARAM\MultiMeasRcp\`, `SystemConfigMgr.cpp:66`). 섹션 이름은 고정이다(`Manager/WadMultiMeasMgr.cpp:123` `"DATA"`, `:149` `"MASECELL"`). WAD 스텝은 스틱마다 `LoadMeasData(GetRecipeID())`(`DoWork/DoWorkGlassMeasureStep.cpp:2445`)로 이 한 벌만 읽는다. 게이트 `m_bUseSpec_Multi`(:2442, 활성 `GetRecipeInfo()` :2137-2138)와 스틱·셀 인덱스(:2203, :2320-2328)는 이미 세그먼트 로컬이다. 데이터만 세그먼트 0이라서 세그먼트 k≥1에서 다음이 틀어진다.
  - **마스크.** `IsMultiCell(iStick, nUseCELLID[p])`(:2835/:2884)가 세그먼트 0 행을 읽는다. LS·P1RT 예:
    - 세그먼트 2(21×5)의 스틱 1~5는 세그먼트 1(5×20) 행의 앞 5칸을 읽는다.
    - 스틱 6~21은 범위 밖이라 false가 된다(`WadMultiMeasMgr.cpp:326-331`). 경고 없이 base 점만 잰다.
  - **점.** 이동식은 `Y1/Y2 = 셀 X + ptMulti.dPosX`, `X2 = dHeadBackG + dOffsetX - ptMulti.dPosY`(:2957-2965)다. 회전 항도 셀 크기 항도 없는 장비 좌표 오프셋이다.
    - 세그먼트 1용 점(셀 70.6760×159.4000, rot 1)이 세그먼트 2(셀 151.4972×66.6808, rot 2)에 그대로 쓰인다.
    - 그래서 엉뚱한 위치를 재고, 셀 밖이나 SW 리밋까지 갈 수 있다. 측정 스텝의 헤드 목표는 사전 검사 대상이 아니다.
    - 값은 `9_OCP_TEST_Hybrid_LS_260929.rcp`의 `_CELL_INFO_`·`_OPTION_INFO_`에서 확인했다.
  - **부수 효과.** ELP `nTotal`(`CommTcpIpObjectWAD.cpp:1202`), 결과 RX/GX/BY(:1317), 용량 검사 P(`DoWorkGlassRecipeStep.cpp:286-287`)도 모두 세그먼트 0 리스트를 쓴다.
  - **다이얼로그.**
    - 셀 맵 크기를 활성 슬롯에서 잡는다(`DlgSubManual_MultiMeas.cpp:448` `GetRecipeData()`, 유휴 시 세그먼트 0).
    - 저장은 `[DATA]/[MASECELL]`에만 한다(:639). 세그먼트 k는 볼 수도 저장할 수도 없다.
    - Recipe Open(:576 `OpenRecipe` → `RecipeChange`)은 하이브리드 측정 중에도 막히지 않는다. 3b는 AgingTest/AgingNew/AutoContact만 막았다. **지금도 열려 있는 구멍이다.**
  - **기존 파일.** 하이브리드 레시피(`1_OCP_TEST_Long_P1RT`, `9_OCP_TEST_Hybrid_LS_260929`)용 Multi 파일은 없다. 폴더의 파일 6개는 전부 단일 레시피용이다. 옮길 파일이 없다.
- **3d가 하는 일:** 세그먼트별 섹션, 스텝의 활성 세그먼트 적재, 용량 P = 세그먼트 최대값, 다이얼로그 세그먼트 선택기, 측정 중 가드.
- **남기는 것:**
  - Phase 4: 같은 블록 안의 세그먼트 간 행 충돌(`segBase`), Cell ID 글자 이월, 결과 저장 게이트.
  - Phase 3c: EFA T0/T1은 같은 WAD 스텝을 타므로 3d가 그대로 적용된다. AgingNew 선택기만 3c 몫이다.
  - Phase 5: Resume(셀당 결과 파일이 여러 개) — 저널로 해결.
  - Phase 6: 운전 화면.
- **D7 운용 제한은 유지한다.** 3d 뒤에도 세그먼트 k≥1의 Multi 행은 같은 블록에서 세그먼트 0 행을 덮어쓴다. 해제는 Phase 4 OOS 뒤다(D3d-8).
- **단계**(예정 스크립트 `p3d_patch.pl`, 백업 `p3d_bak`):
  - **3d-0** 다이얼로그 측정 중 가드(B16a). 단독 빌드·배포 가능, 지금 열린 구멍을 막는다.
  - **3d-1** 매니저(B1~B3).
  - **3d-2** 런타임·용량·WARN 검사(B5, B6, B10, B11).
  - **3d-3** 다이얼로그 선택기, rc, resource(B16b, B17~B21).

**(2) 접점 표**

| # | 접점 (file:line) | 문제 (세그먼트 도입 시) | 변경 |
|---|---|---|---|
| B1 | `Manager/WadMultiMeasMgr.cpp:123,130,133,137` `"DATA"`, `:149` `"MASECELL"`(읽기) / `:198,206,209,212`, `:228`(쓰기) | 섹션 이름 고정. 모든 세그먼트가 세그먼트 0의 점·CELL·마스크를 읽고 쓴다 | 정적 `GetDataSection(k)`/`GetMaskSection(k)`. k≤0이면 기존 리터럴 `DATA`/`MASECELL`, k≥1이면 `DATA_SEG{k+1}`/`MASECELL_SEG{k+1}`. 규칙은 `CAgingCycleInfo::GetMaskSection`(`DoWork/AgingCycleInfo.cpp:97-101`)과 같지만 별도 함수로 둔다 — `CWadMultiMeasMgr`를 `AgingCycleInfo`·`dfcStr` 의존이 없는 leaf 클래스로 유지하기 위해서다(`CommTcpIpObjectWAD`와 다이얼로그가 include) |
| B2 | `:97-173` `LoadMeasData(rcp)` | 세그먼트 인자 없음 | 본문을 `LoadMeasSegment(rcp, nSeg)`로 옮긴다(세그먼트 1개를 기존 평면 멤버에 적재). `LoadMeasData(rcp)` = `LoadMeasSegment(rcp, 0)`. `m_nLoadedSeg`, `m_bDataFound`(점 섹션 파싱 결과 점 ≥ 1 — 다이얼로그는 Del로도 1행을 남기므로 저장된 섹션은 항상 ≥ 1), `m_bMaskFound`(마스크 행 ≥ 1) 추가. 파싱 결과로 판정해 I/O 추가 없음. **두 플래그를 분리한다** — 로더가 `[MASECELL*]`을 `[DATA*]`와 무관하게 읽으므로(:130, :149) 하나로 합치면 `[DATA_SEG2]`만 없는 파일을 "있음"으로 본다(검토 major). ctor(:32-45)·`Clear()`에서 0/false 초기화. false 반환은 파일 없음일 때만(다이얼로그 생성 흐름 `DlgSubManual_MultiMeas.cpp:589-601` 유지) |
| B3 | `:178-238` `SaveMeasData(rcp)` | 세그먼트 k 저장 불가. `:217-229`는 `STICK_1..size`만 쓰고 더 큰 stale 키를 남긴다(로더 `:143-152`는 첫 빈 키까지 읽는다) | 본문을 `SaveMeasSegment(rcp, nSeg, bDeleteStaleRows)`로 옮긴다. `bDeleteStaleRows`면 쓴 행 수+1부터 첫 빈 키까지 `WritePrivateProfileString(sec, key, NULL)`로 **키 단위** 삭제. 섹션을 통째로 지우면 다시 쓸 때 섹션이 파일 끝으로 옮겨지므로 금지. `SaveMeasData(rcp)` = `SaveMeasSegment(rcp, 0, false)` |
| B4 | `:243-253` `IsLoaded/GetLoadedRecipeID`(소비처 없음), `:255-348` getter·`IsMultiCell`, `:353-420` 실행 상태, `:422-460` UI setter, `:462-485` `IsCellRangeValid` | 평면 멤버 1벌 | **변경 없음.** 평면 멤버 = "마지막으로 적재한 세그먼트". `IsCellRangeValid`는 적재된 세그먼트 자기 CELL_X/Y로 검사하므로 자동으로 세그먼트별이 된다. 시그니처가 그대로라 `CommTcpIpObjectWAD` 컴파일에 영향 없음 |
| B5 | `DoWork/DoWorkGlassMeasureStep.cpp:2441-2453` 게이트(`LoadMeasData` :2445, 로그 :2451) | 플래그는 세그먼트별인데 파일은 세그먼트 0만 적재 | `int nMultiSeg = GetRecipeEditMgrPtr()->GetActiveSegment();` → `LoadMeasSegment(GetRecipeID(), nMultiSeg)`. `GetSegmentLoopCount() > 1`일 때만 로그를 `WAD Multi Point mode active (segment %d/%d, points %d)`로 쓴다. `IsDataFound()==false`면 SYST `[HYBRID] WAD Multi segment %d/%d: [%s] not found - base point only`. **트리거는 "이 세그먼트의 첫 적재"**(적재 전 `GetLoadedSegment() != nMultiSeg`) — 스틱 0 기준이면 AgingCycle 마스크로 스틱 0의 WAD가 빠지거나 Resume으로 스틱 0을 건너뛸 때 안 찍히고, AgingCycle 항목 K개마다 반복된다(검토 지적) |
| B6 | `:3038` `ResetRunState()` — `:2880` if 안이라 base 측정 실패 시 건너뜀. `:2444` 리셋은 플래그 on일 때만 | 하이브리드는 한 글라스 안에서 Multi on → off 세그먼트로 넘어간다. 이때 `IsRunActive`가 남으면 다음 WAD 프레임이 len 132가 된다(`CommTcpIpObjectWAD.cpp:1200-1204`) | :2442 if에 `else if (pMultiMgr != nullptr && GetSegmentLoopCount() > 1) pMultiMgr->ResetRunState();` 추가(N>1만). 다이얼로그가 유휴 중 세그먼트를 적재할 때도 `ResetRunState()`(B20, `IsViewSegmented()`일 때만) — 공유 매니저에 다른 세그먼트가 올라간 채 실행 상태가 남으면 수동 측정(B23)의 WAD 프레임이 그 세그먼트 값을 쓴다. :3038은 무변경 |
| B7 | `:2835`, `:2884` `IsMultiCell(iStick, nUseCELLID[p])` | 3d 전에는 세그먼트 0 마스크 행을 읽는다(위 (1)) | **변경 없음.** `iStick` = `GetMeasureStick()`(:2203, 세그먼트 로컬), `nUseCELLID` = 로컬 `m_nArea`(:2320-2328, `CoreFunction/dll_Recipe/dfcRecipe/MakeCellDlg.cpp:947` `i*nCell+j`)라 세그먼트 섹션과 같은 인덱스 공간이다 |
| B8 | `:2901-2948` 점 순회·로그 4벌, `:2952-2969` 이동, `:3018` 트레이스 | 3d 전에는 세그먼트 0 오프셋으로 이동한다 | **변경 없음.** 기준값(`dMLeft/dMRight` = 활성 세그먼트 `m_fCellX`, `dHeadBackG` :2315, `dOffsetX` :2166)은 이미 세그먼트 기준이고, 적재된 리스트만 바뀌면 맞다. 런타임 회전 변환은 쓰지 않는다: 파일에 기준 프레임이 없고, 셀 크기 차이는 변환으로 못 고친다 |
| B9 | `:2998-3002` AgingCycle 블록 `K + rep×(nPtTotal-1) + (ip-1)`, `:3006-3007` 표준 블록 `ip` | 세그먼트마다 P가 달라서 블록 b가 뜻하는 (rep, point)가 세그먼트마다 다르다. 블록 간 충돌은 없다 | **변경 없음**(D3d-2). 블록 크기 `G`는 3b에서 이미 글라스 전체, `segBase`는 Phase 4 |
| B10 | `DoWork/DoWorkGlassRecipeStep.cpp:281-288` `CheckHybridCapacity` P | P를 세그먼트 0 리스트에서만 얻는다(:286). 점이 더 많은 세그먼트 k가 있으면 행 수를 적게 센다 | 로컬 매니저로, 플래그 on인 세그먼트 k마다 `LoadMeasSegment(rcp, k)` 후 `nP = max P_k`. 로그 끝에 `(per segment a/b/…)` 추가 |
| B11 | 같은 파일 `:350-358` 하이브리드 사전 검사 블록 | 세그먼트 k의 리스트·마스크가 그 세그먼트에 맞는지 아무도 확인하지 않는다 | 신규 static `CheckHybridWadMulti`, **WARN만**(D3d-3). 검사 항목: 섹션 없음 또는 INSP_MODE 0 / 마스크 행 수·폭 ≠ `GetSegmentStickCount/CellCount(k)` / CELL_X/Y와 `\|m_fCellSize.x/.y\|` 차이 0.5 mm 초과(뒤바뀌었으면 힌트) / 활성 점 ip≥1이 `±\|m_fCellSize\|/2` 밖. 축 대응(X ↔ `m_fCellSize.x` = 헤드 Y 방향)은 `InRecipeFunc.cpp:1207-1208` 기준 |
| B12 | 같은 파일 `:187-239` `CheckMeasureTargetSWLimit` | 리뷰 이동만 검사. Multi 점의 헤드 목표는 N=1에서도 검사하지 않는다 | **3d 범위 밖**(D3d-5). 알려진 갭으로 기록 |
| B13 | `CommTcpIpObjectWAD.cpp:1198-1206` ELP `nTotal`, `:1302-1324` `TossResultFile` 오프셋 | 3d 전에는 세그먼트 0의 점 개수와 RX/GX/BY | **변경 없음.** 둘 다 측정 스레드의 동기 호출이다: `_ExeMeasWAD`는 스텝의 `DoWorkInspectStart`에서, `TossResultFile`은 `Manager/CommTcpIpFactoryMgr.cpp:1536/1542`(단일 헤드)와 `:1549-1550`(양 헤드, 페어 그룹 기본 경로) → `WaitInspectionComplete:486` / `WaitInspectionEnd:390`에서 불린다. 따라서 B5 적재 뒤의 활성 세그먼트 값을 읽는다 |
| B14 | `CommTcpIpObjectWAD.cpp:1137` `GetWADRecipeName()` | Multi 파일 키 후보 | **변경 없음.** 파일을 WAD 모델명으로 나누지 않는다: 이름이 공란이면 RecipeID로 fallback되어(`Recipe/RecipeEditMgr.cpp:299-309`) 세그먼트 k가 세그먼트 0 파일을 가리키게 되고 D7이 재발한다 |
| B15 | `AuroraMgr.cpp:141`, `SystemConfigMgr.cpp:66`, `DoWork/DoWorkGlassMeasure.cpp:492-521` `SegmentSwitchData`, `CoreFunction/dResult/CDlgResult.cpp:497-502·1295`, `Source/Include/API_Result.h:120`과 사본 `CoreFunction/dResult/API_Result.h:120` | — | **변경 없음.** 인스턴스 1개·폴더 1개·RecipeID당 파일 1개다. 스텝이 스틱마다 다시 적재하므로 경계에서 할 일이 없다. 오프셋·점 번호는 행마다 `RSRITEMDATA`에 실려 간다(점 수 ≤ 99) |
| B16 | `DlgSubManual_MultiMeas.cpp:505-518` `IsOfflineStatus`, `:523-602` Open | 하이브리드 측정 중 Open → `RecipeChange`가 측정 스레드 밑에서 세그먼트 벡터·활성 인덱스를 리셋하고, 다음 글라스 RecipeID까지 바꾼다(`RecipeEditMgr.cpp:255-259`) | **B16a (3d-0, 단독 빌드)**: 멤버 `IsSegmented()`·`IsHybridRunBusy()`(AgingTest :959-970 복사), `#include "Manager\SequenceManager.h"`, `IsOfflineStatus` 끝에 3b 블록(`DlgSubManual_AgingTest.cpp:518-524`)을 추가해 Open·Save·세그먼트 전환을 차단한다. **B16b (3d-3)**: Open 성공 경로의 `LoadFromMgr()` 두 곳(:584, :599) 직전에 `UpdateSegmentCombo()`(콤보·ID가 필요해 B20/B21과 같이 간다) |
| B17 | `:434-491` `RebuildCellMap`(:448 `GetRecipeData()`, :487 `IsMultiCell`) | 활성 슬롯 크기라 세그먼트 2의 21×5 맵을 만들 수 없다 | 하이브리드 뷰면 `GetSegmentMgr(m_nViewSeg)->m_sData` 크기를 쓴다(읽기 전용, `SetActiveSegment` 호출 없음 — AgingTest :742-745). 채운 뒤 불일치 행·초과 행을 집계해 한글 경고 + WARN 로그, `m_nViewStick/m_nViewCell` 표식 |
| B18 | `:604-650` Save(`IsCellRangeValid` :631, `SaveMeasData` :639, 재적재 :646) | 세그먼트 0 섹션에만 쓰고, CELL 1벌로 검증한다 | 레시피 이름 검사(:615-622) 직후 `if (IsViewSegmented()) { SaveViewSegment(TRUE); return; }`. N=1 경로 :624-649는 그대로 |
| B19 | `:364-392` `LoadFromMgr` | 새 세그먼트는 CELL 0/0이라 base 점 (0,0)조차 `IsCellRangeValid`에서 걸린다(`>=` 비교, `WadMultiMeasMgr.cpp:476-477`) | 하이브리드 뷰이고 `IsDataFound()==false`면 CELL 그리드를 `fabs(GetSegmentMgr(k)->m_sMap.m_fCellSize.x/.y)`로 채운다(D3d-4) |
| B20 | `.h:28-71`, `.cpp:41-51`(ctor), `:61-64`(DDX), `:66-73`(메시지 맵), 파일 끝(:763 뒤), include :10-15 | 세그먼트 뷰 상태·핸들러가 없다 | (5)의 멤버·함수·메시지 맵, `#include "Manager\SequenceManager.h"` |
| B21 | `ALPHA.rc:994-1018`, `resource.h:577-578` | 컨트롤 없음 | 숨김 컨트롤 3개 + ID 21010~21012 |
| B22 | 그리드 전용(:121-362, :652-763), `SaveToMgr`(:394-432), `GetMgr`(:75-81), `OnInitDialog`(:83-116), `UpdateStatus`(:496-503, 호출처 없음 — `DlgSubManual.cpp`는 :178/206/240에서 ShowWindow만 부른다) | — | **변경 없음.** 보고 있는 세그먼트의 그리드만 다루고 파일·레시피는 건드리지 않는다. `SaveToMgr`는 그리드 자신의 행·열 수를 읽으므로 세그먼트 그리드에도 그대로 맞는다(`SaveViewSegment`가 호출) |
| B23 | `DlgSubManual_Meas.cpp:2665` 수동 WAD 측정(`DoWorkInspectStart(enumTCPIpFactorytID_WAD, ...)`) | 같은 `CCommTcpIpObjectWAD`를 거쳐 공유 매니저의 `IsRunActive/GetUseSize/GetRunPointIdx/GetPoint`를 **간접 소비**한다(`CommTcpIpObjectWAD.cpp:1198-1206, 1302-1324`) | 코드 무변경. B6의 다이얼로그 적재 시 리셋으로 잔류 실행 상태를 끊는다. N=1의 base 측정 실패 뒤 잔류(:3038이 SUCCESS 블록 안)는 기존 결함 — D3 묶음 |

**(3) 파일 포맷**

```ini
[DATA]
CLASS=C,C,...,
X=0.0000,2.0000,...,
INSP_MODE=1
CELL_X=70.6760
CELL_Y=159.4000
[MASECELL]
STICK_1=1,0,...,
[DATA_SEG2]
CLASS=C,C,
X=0.0000,3.0000,
Y=0.0000,3.0000,
INSP_MODE=1
CELL_X=151.4972
CELL_Y=66.6808
[MASECELL_SEG2]
STICK_1=1,0,0,0,0,
```

위 예에서 `[DATA]/[MASECELL]`은 세그먼트 1(5행 × 20칸)이다. `[DATA_SEG2]/[MASECELL_SEG2]`는 세그먼트 2이고, R180 장비 좌표 기준이며 마스크는 21행 × 5칸이다.

- **세그먼트 0.** `[DATA]`(점 10키 + `INSP_MODE`·`CELL_X`·`CELL_Y`)와 `[MASECELL] STICK_n`을 바이트 단위로 그대로 둔다(OCP 호환, `WadMultiMeasMgr.h:9-13`).
- **세그먼트 k≥1.** `[DATA_SEG{k+1}]`는 같은 13키, `[MASECELL_SEG{k+1}]`는 `STICK_1..스틱 수(k)`에 행마다 셀 수(k)개 토큰이다. 형식 문자열은 `%.4f,`/`%d,`/`%s,`로 같다(`:490-517`).
- **세그먼트 수 키는 두지 않는다.** N은 레시피의 `GetSegmentLoopCount()`에서 얻는다.
- **단위 구분:**

| 항목 | 단위 | 근거 |
|---|---|---|
| 파일(폴더·RecipeID 키) | 글라스 | 하이브리드 레시피 = `.rcp` 1개(D-2). 다이얼로그도 RecipeID로 연다 |
| 점 리스트(CLASS..WY) | 세그먼트 | 장비 좌표 오프셋(:2957-2965), 회전·셀 크기 항 없음. LS 세그먼트의 헤드 Y 방향 셀 반폭은 ±35 mm 대 ±76 mm |
| CELL_X/Y | 세그먼트 | `IsCellRangeValid` 기준값(:468-477). 기존 샘플이 원본 레시피 `m_fCellSize`와 같다(`1_OCP_TEST_Long_260720` 153.9054/67.9312, `2_OCP_TEST_Short_260720` 151.4972/66.6808 = LS 세그먼트 2) |
| INSP_MODE | 세그먼트 | 13키 파서·작성기 한 벌로 모든 세그먼트를 처리하고, "섹션 없음 = off"가 따로 코드 없이 성립한다 |
| 마스크 | 세그먼트 | 5×20 대 21×5. 행 = 세그먼트 로컬 레시피 스틱 순서, 열 = 스틱 안 셀 인덱스. 회전 재배치 없음(`RebuildCellMap` :477-489와 런타임이 같은 프레임) |
| 사용 스위치 `m_bUseSpec_Multi` | 세그먼트(레시피 안, 기존) | 각 `S_RecipeMgr`의 `S_RecipeInfo`. `ValidateSegments`(`InRecipeFunc.cpp:1129-1134`)는 AgingTest/EFA만 같게 강제한다 |

- **섹션이 없을 때:**
  - `[DATA_SEG{k+1}]` 없음 → `IsMultiMode()==false` → 그 세그먼트는 base 점만(N=1의 "파일 없음"과 같은 경로). `[DATA]`로 fallback하지 않는다(D3d-1).
  - `[MASECELL_SEG{k+1}]` 없음 → 모든 셀 `IsMultiCell` false → base 점만. N=1의 범위 밖 = false(`:325-331`)와 같다.
  - 이것은 AgingTest D4의 "섹션 없음 = 전부 측정"과 **일부러 다르다**. 두 경우 모두 "그 기능의 기본 측정"을 따른 결과다: AgingTest의 기본은 전 셀 측정, WAD Multi의 기본은 base 점만이다.
  - 고아 섹션(k ≥ 현재 세그먼트 수)은 읽지 않고 남겨 둔다(3b 선례).
- **구 빌드·OCP:** `[DATA]/[MASECELL]`만 읽는다(구 빌드는 하이브리드를 세그먼트 0만으로 돌린다, D-12). `WritePrivateProfileString`은 자기가 쓰는 키만 고치므로, 구 빌드로 저장해도 접미사 섹션은 남는다.
- **AgingRcp와의 관계:** `[MASECELL_SEG*]`는 이름이 같지만 폴더가 달라(`AgingCycleInfo.cpp:23-24`) 충돌하지 않는다.

**(4) API 계약**

```cpp
// Manager/WadMultiMeasMgr.h  (CP949, LF-only) - LYS 2026-09-30 Hybrid 3d
static CString GetDataSection(int nSeg);	// nSeg <= 0 -> "DATA",     else "DATA_SEG%d"     (nSeg + 1)
static CString GetMaskSection(int nSeg);	// nSeg <= 0 -> "MASECELL", else "MASECELL_SEG%d" (nSeg + 1)
bool	LoadMeasSegment(CString strRecipeID, int nSeg);	// ONE segment -> flat members; false only if the file is missing
bool	SaveMeasSegment(CString strRecipeID, int nSeg, bool bDeleteStaleRows);
bool	LoadMeasData(CString strRecipeID);		// = LoadMeasSegment(strRecipeID, 0)         (meaning unchanged)
bool	SaveMeasData(CString strRecipeID);		// = SaveMeasSegment(strRecipeID, 0, false)  (meaning unchanged)
int		GetLoadedSegment();
bool	IsDataFound();							// loaded segment's [DATA*] section parsed >= 1 point  (B5 log, label, B19)
bool	IsMaskFound();							// loaded segment's [MASECELL*] section has >= 1 row   (B11, label)
```

- **계약.** 평면 멤버(`m_vecPoint`, `m_bInspMode`, `m_dCellX/Y`, `m_vecCellUse`)는 **마지막으로 적재한 세그먼트** 한 벌이다. 기존 getter와 실행 상태 API는 시그니처·의미가 그대로다.
  - 런타임 소비자(스텝 :2835~:3018, WAD 객체 :1198/:1302)는 같은 측정 스레드에서 B5 적재 뒤에만 읽는다.
  - 다이얼로그는 하이브리드 측정 중이 아닐 때만 적재한다(B16).
  - ctor와 `Clear()`는 `m_nLoadedSeg = 0`, `m_bDataFound = m_bMaskFound = false`까지 초기화한다. 섹션 이름은 `CString::Format`으로 만든다(`dfcStr`에 의존하지 않는다).
- **기각한 안.** "모든 세그먼트를 메모리에 올리고 getter마다 `nSeg`를 받으며 `m_nRunSeg` 공유 상태를 둔다." 이 안은 `CommTcpIpObjectWAD` 2곳과 getter 전부를 바꿔야 한다. 스텝이 이미 WAD 스텝마다 다시 적재(:2445)하므로 "적재된 세그먼트 = 활성 세그먼트"가 그 안 없이도 성립한다.
- **`CRecipeEditMgr`: 신규 API 없음.** `GetActiveSegment`(`RecipeEditMgr.cpp:318-322`, 1세그먼트면 0), `GetSegmentLoopCount`(:313-316), `GetGlassRowBlock`(:342-350), `GetRecipeFuncPtr()->GetSegmentMgr/Name/Rotate/StickCount/CellCount`로 충분하다.
- **N=1 동작.** 기존 두 함수는 세그먼트 0 래퍼가 되어 같은 키를 같은 순서로 읽고 쓴다. 삭제는 없다.

**(5) 다이얼로그 설계 (`IDD_DLG_SUB_MANUAL_MULTI_MEAS`, `CDlgSubManual_MultiMeas`)**

- **컨트롤.** 셋 다 숨김이고 하이브리드 뷰에서만 표시한다. 빈 사각형은 x 454..641 × y 164..208이다: Cell Size 그룹이 x 450에서 끝나고(`ALPHA.rc:1009`), IMG 자리표시자가 y 222에서 시작한다(`:1013`). 아래 세 줄을 `ALPHA.rc:1017` 뒤, `END`(:1018) 앞에 넣는다:
  ```
      GROUPBOX        "Segment",IDC_STATIC_MM_SEG,454,164,187,44,NOT WS_VISIBLE
      COMBOBOX        IDC_COMBO_MM_SEG,461,174,173,120,CBS_DROPDOWNLIST | WS_VSCROLL | NOT WS_VISIBLE | WS_TABSTOP
      LTEXT           "",IDC_STATIC_MM_SEG_INFO,461,189,173,16,NOT WS_VISIBLE
  ```
  - `resource.h:578` 뒤에 `IDC_COMBO_MM_SEG 21010`, `IDC_STATIC_MM_SEG 21011`, `IDC_STATIC_MM_SEG_INFO 21012`를 추가한다. 21008~21013은 `Source/ALPHA`·`Source/Include`·`CoreFunction/common`에서 쓰이지 않는다.
  - 21008/21009는 Phase 3c AgingNew 선택기용으로 비워 둔다. `_APS_NEXT_CONTROL_VALUE`(:586)와 DESIGNINFO(:1414)·AFX_DIALOG_LAYOUT(:1673)은 건드리지 않는다.
  - 콤보 항목은 `Seg %d/%d %s R%d (%d x %d)`(이름, `GetSegmentRotate×90`, 스틱×셀)이다. 점 좌표의 기준 프레임을 보여 주기 위해 회전을 넣는다.
  - 정보 라벨은 2줄, 3b 콤보처럼 ASCII다: `Multi(recipe) ON|OFF  cell 151.50 x 66.68` / `[DATA_SEG2] found` 또는 `[DATA_SEG2] none -> base point only`(마스크만 있으면 `, [MASECELL_SEG2] present`를 덧붙임).
- **상태 멤버:** `CComboBox m_cmbSeg; int m_nViewSeg, m_nViewSegCount; CString m_strViewSegRcp; int m_nViewStick, m_nViewCell; int m_nMaskMismatch, m_nMaskExtra;`. ctor에서 0으로 초기화하고, DDX에 `DDX_Control(pDX, IDC_COMBO_MM_SEG, m_cmbSeg)`을 넣는다.
- **함수.** 3b 이름을 재사용하고, 주석은 `// LYS 2026-09-30 Hybrid 3d ...`로 단다.
  - `IsSegmented()`: AgingTest :959-963 복사. **로드된** 레시피가 N>1인가.
  - `IsViewSegmented()`: `m_nViewSegCount > 1`, 즉 **화면**이 하이브리드용으로 만들어졌는가. 그리드·저장 경로는 이것으로 나눈다. 3b `IsMaskViewStale`의 구멍(레시피가 다른 곳에서 1세그먼트로 바뀌면 FALSE가 되어 하이브리드 그리드가 N=1 경로로 저장됨)을 여기서는 막는다.
  - `IsHybridRunBusy()`: AgingTest :965-970 복사.
  - `UpdateSegmentCombo()`: AgingTest :972-1000 패턴. Open의 `LoadFromMgr()` 직전에만 부른다. `m_nViewSeg=0`, `m_nViewSegCount=GetSegmentLoopCount()`, `m_strViewSegRcp=GetRecipeID()`, 콤보를 채우거나 숨긴다. **콤보를 `EnableWindow(TRUE)`로 되살리고 `UpdateSegmentInfo()`를 부른다**(검토: 비활성화 뒤 되살리는 경로가 없었다). N>1이면 타이머를 켜고, N=1이면 끈다(아래). 생성 거절(No)로 return하면 뷰 표식이 이전 값으로 남아 stale 검사가 잡는다.
  - `UpdateSegmentInfo()`: 정보 라벨 갱신.
  - `IsMultiViewStale(BOOL bPopup)`: 3b `IsMaskViewStale` 확장. `IsViewSegmented()`일 때만 검사하고, 아래 순서로 단락 평가한다(수가 같을 때만 세그먼트 벡터를 읽는다):
    1. 편집창 파일 키 ≠ `GetRecipeID()`
    2. `m_strViewSegRcp` ≠ `GetRecipeID()`
    3. `GetSegmentLoopCount()` ≠ `m_nViewSegCount`
    4. `GetSegmentStickCount/CellCount(m_nViewSeg)` ≠ `m_nViewStick/m_nViewCell`
  - `SaveViewSegment(BOOL bAsk)`: 순서대로 수행한다.
    1. 한글 확인(bAsk일 때).
    2. stale 검사.
    3. `SaveToMgr()`.
    4. `IsCellRangeValid`(NG면 한글 팝업).
    5. CELL ↔ 레시피 `|m_fCellSize|` 0.5 mm 초과면 한글 Yes/No("포인트 좌표는 이 세그먼트(R%d) 기준").
    6. 불일치·초과 행이 있으면 덮어쓰기 확인.
    7. `G×P_k > M_RESULT_AREA_MAX`면 비차단 경고((6) 참조).
    8. `SaveMeasSegment(rcp, m_nViewSeg, true)`.
    9. `LoadMeasSegment` → `LoadFromMgr` → `UpdateSegmentInfo`, OPER 로그 `[HYBRID] WAD Multi saved %s [%s]/[%s] points %d rows %d`.
    - 실패하면 FALSE를 반환한다.
  - `OnSelchangeComboMmSeg()`: AgingTest :1012-1035 패턴.
    - `!IsViewSegmented()`, `!IsOfflineStatus()`, `IsMultiViewStale(TRUE)` 중 하나면 선택을 되돌린다.
    - 한글 예/아니요/취소를 묻는다. 예는 `SaveViewSegment(FALSE)`이고 실패하면 되돌린다.
    - `m_nViewSeg=nSel` 후 `LoadMeasSegment(rcp, nSel)`. false면 `Clear()`. 이어서 `LoadFromMgr()`, `UpdateSegmentInfo()`.
    - `SetActiveSegment`는 절대 부르지 않는다.
  - **500 ms 타이머 + `OnShowWindow`**(3b AgingTest :195-236 패턴): `OnShowWindow`만으로는 부족하다 — 이 다이얼로그는 `CDlgSubManual`의 WS_CHILD라 `WM_SHOWWINDOW`는 Manual 하위 탭 버튼(`DlgSubManual.cpp:318-324` → `RefreshFileView`)으로 들어올 때만 오고, 상단 내비게이션으로 Manual 화면에 돌아올 때는 부모만 숨김/표시된다(`DlgMainForm.cpp:486-503`, 검토 지적). 그래서 `UpdateSegmentCombo`가 N>1일 때 `SetTimer`, N=1일 때 `KillTimer` — **N=1에서는 타이머 자체가 없다.** 타이머 본문과 `OnShowWindow`는 같은 판정을 한다:
    - `!IsViewSegmented()`면 즉시 return.
    - busy면 콤보를 비활성화하고 라벨을 `Hybrid run in progress`로 한다. `GetSegmentMgr`는 읽지 않는다(`SetActiveSegment`가 벡터 원소를 값 복사하는 중일 수 있다).
    - stale면 콤보를 비활성화하고 라벨을 `Recipe changed - press Recipe Open`으로 한다.
    - 둘 다 아니면 콤보를 `EnableWindow(TRUE)`로 되살리고 `UpdateSegmentInfo()`.
    - 그리드는 다시 만들지 않고, 뷰 상태도 지우지 않는다(Save가 stale로 막히게 둔다).
- **Open·Save 흐름.**
  - Open(:523-602)은 `OpenRecipe`로 하이브리드 전체를 연다(활성 = 0).
  - `LoadMeasData(strFileName)`(:582)은 세그먼트 0을 그대로 적재한다.
  - `RebuildCellMap`은 세그먼트 0 크기로 그리드를 만든다.
  - 행 헤더는 N=1과 같은 `S1..Sn`(레시피 스틱 순서)이다. 180° 세그먼트에서는 Cell ID 글자 순서와 반대일 수 있어 라벨·검증 항목으로 안내한다.
  - 기존 영어 팝업은 그대로 두고, **새 팝업만 한글 CP949**로 쓴다. 로그는 영어다.
- **가드 대응표** (3b 이름 기준):

| 3b 이름 | 3d 대응 |
|---|---|
| 측정 중 차단 | `IsOfflineStatus`에 추가: 3b와 같은 문구 `하이브리드 레시피 측정 중에는 이 작업을 할 수 없습니다...` |
| 레시피 변경 | `IsMultiViewStale` |
| 불일치 행 | 로드 경고, 저장 시 확인. 표시는 런타임과 같은 셀 단위 `IsMultiCell` |
| stale 키 | `SaveMeasSegment(..., true)`. 하이브리드 뷰에서는 세그먼트 0도 삭제(3b 선례), N=1은 삭제 없음 |

- **MOVE·수동 측정.**
  - 이 다이얼로그에는 이동·측정·티칭 컨트롤이 없다(`ALPHA.rc:1014-1017` = List Init/Save/Add/Del). OCP 원본에도 없다(`docs/Dialog_Analysis_IDD_DLG_NAVI_MANUAL_MULTI_MEAS.md` §4). 그래서 "비활성 세그먼트 이동" 문제는 생기지 않는다.
  - 나중에 추가한다면(Phase 7, D-9): 유휴 상태에서 활성 세그먼트만 허용한다(유휴 시 활성 = 0). k≠활성은 Stage R이 다르므로 차단한다.
  - 직접 사용처는 스텝, 용량 검사, WAD 객체, 이 다이얼로그뿐이다(grep). 단 수동 측정 화면(`DlgSubManual_Meas.cpp:2665`)이 WAD 객체를 거쳐 **간접 소비**한다(B23).

**(6) 용량 검사**

- 식은 그대로다: `rows = G × max(K_wad × P, K_vis, K_ang) ≤ M_RESULT_AREA_MAX(2500)`.
- **P만 바뀐다.** 세그먼트 0 리스트 대신, `m_bUseSpec_Multi(k)`이고 `IsMultiMode && GetUseSize()>0 && GetPointCount()>1`(:286과 같은 조건)인 세그먼트의 `max_k GetPointCount(k)`다.
- **상한 근거.** 세그먼트 k의 최대 블록은 표준 경로에서 `P_k-1`(:3006), AgingCycle 경로에서 `K×P_k-1`(:2998-3000, ip ≤ P_k-1, rep ≤ K-1)이다. 따라서 D3d-2 기본안(세그먼트 자기 stride)에서도 `G×K×max P_k`가 상한이고, 용량 검사와 행 배치가 어긋나지 않는다.
- P는 비활성 점도 센다. 행 블록이 원시 인덱스 ip라서(:2906/:3006) `GetUseSize`가 아니라 `GetPointCount`를 쓴다(기존 규칙).
- **DLL 저장 시 검사는 없다.** `ValidateSegments`(`InRecipeFunc.cpp:1145-1155`)는 G만 본다. K×P는 사이드카 파일이라 ALPHA 자동 시작에서 본다는 주석(:1146-1147)을 유지한다.
- **다이얼로그 저장(하이브리드만)은 비차단 한글 경고만 한다.** 조건은 `GetGlassRowBlock(0,0)`(N>1이면 인자를 무시하고 Σ셀×스틱을 돌려준다) × P_k > 2500이다. K는 AgingRcp에서 따로 편집되므로 빼고, 하한 경고로 쓴다. 실제 거부는 자동 시작 검사가 맡는다.

**(7) N=1 불변 근거**

| 변경 | N=1 실행 경로 | 관측 결과 |
|---|---|---|
| B1~B3 매니저 | `LoadMeasData`/`SaveMeasData`가 `…Segment(rcp, 0[, false])`로 위임한다. k≤0 섹션 이름 = 기존 리터럴, 버퍼·스캔·포맷 그대로, `m_bDataFound/m_bMaskFound`는 파싱 결과(I/O 추가 없음), 삭제 없음 | 적재 상태·파일 바이트 동일 |
| B5 적재·로그 | `GetActiveSegment()` = 0(`RecipeEditMgr.cpp:320`)이라 `LoadMeasSegment(rcp,0)` ≡ 기존. 새 로그 두 종은 `GetSegmentLoopCount() > 1`일 때만 | INI 읽기·게이트·`WAD Multi Point mode active` 동일 |
| B6 else 리셋 | `GetSegmentLoopCount() > 1` 조건 | 실행 안 됨. N=1 글라스 간 잔류 결함은 D3 묶음으로 |
| B10/B11 | `GetSegmentCount() > 1` 블록 안(`DoWorkGlassRecipeStep.cpp:336`) | 호출 안 됨 |
| B16 가드 | `IsHybridRunBusy()` → `IsSegmented()` FALSE | 차단·팝업 없음 |
| B17~B20 다이얼로그 | Open이 `m_nViewSegCount = 1`로 두므로 `IsViewSegmented()` FALSE: `:448` 경로, Save `:624-649`, CELL 0/0, 집계·팝업 없음, `OnShowWindow` 즉시 return, 타이머 없음(`KillTimer`만), 적재 시 리셋 없음. `UpdateSegmentCombo`는 이미 숨은 컨트롤에 `SW_HIDE`만 한다 | 화면·팝업·파일 동일 |
| B21 rc/resource | 새 컨트롤은 전부 `NOT WS_VISIBLE`이고 ID는 쓰이지 않던 값 | 화면 동일 |
| B4, B7~B9, B12~B15, B22 | 코드 무변경 | — |

**(8) 결정 — 전부 권장안으로 확정(2026-09-30 사용자 지시)**

| # | 항목 | 선택지 | 권장(= 확정) |
|---|---|---|---|
| D3d-1 | 세그먼트 k≥1 섹션 없음(레시피 Multi on) | base 점만 + WARN / 세그먼트 0 `[DATA]` 상속 | **base 점만**. 세그먼트 0 점은 프레임·셀 크기가 다르다. N=1 "파일 없음 = off"와 같은 경로 |
| D3d-2 | AgingCycle 멀티포인트 블록 stride | 세그먼트 자기 P(코드 무변경) / 글라스 P_max | **세그먼트 자기 P**. 블록 간 충돌이 없고 용량은 max로 상한을 보장한다. 대안은 P_max를 글라스 래치로 스텝에 넘겨야 한다(공유 상태 추가). Phase 4 MES 합의 때 재검토 |
| D3d-3 | 자동 시작 WAD Multi 데이터 검사(B11) | WARN만 / 자동 시작 거부 | **WARN만**. 런타임은 범위 밖 = false로 안전하고, 1차 방어는 다이얼로그 저장 검사다. N=1에는 없는 검사 |
| D3d-4 | 새 세그먼트 CELL_X/Y 초기값 | 레시피 `\|m_fCellSize\|`로 채움 / 0,0 | **채움(하이브리드만)**. 0/0이면 base 점도 저장이 거부된다. 기존 샘플이 레시피 값과 같다 |
| D3d-5 | Multi 점 헤드 목표 SW 리밋 사전 검사 | 3d에 포함(N>1만) / 보류 | **보류**. N=1에도 있는 갭이라 3d 범위 밖. 알려진 갭으로 기록 |
| D3d-6 | 하이브리드 저장 범위 | 보는 세그먼트만 + 전환 시 저장 확인 / 전 세그먼트 일괄 | **보는 세그먼트만**. 3b AgingTest 선례이고, 전환 = 평면 매니저 재적재와 맞다 |
| D3d-7 | 단일 레시피 Multi 파일 가져오기(Import 버튼, `151,402,62,20`) | 3d-4로 추가 / 보류 | **보류**. 프레임·마스크는 스틱×셀×회전이 같을 때만 유효하다. 넣는다면 그 조건일 때만 허용하고 `OpenRecipe`는 부르지 않는다 |
| D3d-8 | D7 운용 제한 해제 시점 | 3d 완료 시 / Phase 4 segBase OOS 후 | **Phase 4 후**. 3d 뒤에도 세그먼트 k 행이 같은 블록에서 세그먼트 0 행을 덮어쓴다 |

**(9) 검증 (OOS)**

1. **N=1 회귀:** `1_OCP_TEST_Long_260720`(7점, 12×9)과 `2_OCP_TEST_Short_260720`(1점, 21×5).
   - 자동 1사이클의 `WAD Multi Point mode active`, `WAD MultiPoint: probe(..) point(..) … row(..)`(:2847/:3017), 모션 목표가 3d 이전 로그와 같다.
   - `[HYBRID]` 줄이 없다.
   - 다이얼로그 Open → 편집 없이 Save 전후 파일 해시가 같다. 콤보와 새 팝업이 없다.
2. **하이브리드 편집:** `9_OCP_TEST_Hybrid_LS_260929`, 두 세그먼트 `m_bUseSpec_Multi`=1.
   - 콤보 `Seg 1/2 … R90 (5 x 20)` / `Seg 2/2 … R180 (21 x 5)`, 그리드 5×20 / 21×5, 라벨 `[DATA_SEG2] none -> base point only`, CELL 자동 151.4972/66.6808.
   - 세그먼트 2 저장 후 `[DATA_SEG2]/[MASECELL_SEG2]`(STICK_1..21, 5칸)가 추가되고, `[DATA]/[MASECELL]` 줄은 diff 0이다.
3. **하이브리드 런:**
   - 용량 로그 `WAD points N (per segment a/b)`.
   - `WAD Multi Point mode active (segment 1/2, points …)` / `(segment 2/2, points …)`.
   - MultiPoint 트레이스가 각 세그먼트 마스크 셀에서만 나온다(세그먼트 2 스틱 6~21 포함).
   - 세그먼트 2 모션 목표 = 셀 중심 + 세그먼트 2 오프셋. 3901/3902 없음.
4. **섹션 없음:** `[DATA_SEG2]`만 삭제(`[MASECELL_SEG2]`는 남김) 후 `[HYBRID] WAD Multi segment 2/2: [DATA_SEG2] not found - base point only`가 세그먼트 2 첫 적재에서 1회, 세그먼트 2 MultiPoint 0, B11 WARN 1줄, 다이얼로그 라벨 `[DATA_SEG2] none -> base point only, [MASECELL_SEG2] present`, CELL 자동 채움, 세그먼트 1 정상.
5. **불일치:** `[MASECELL_SEG2]`를 20행이나 4칸 행으로 손으로 고친다.
   - 세그먼트 2 보기에서 한글 경고와 WARN 로그가 뜬다.
   - 저장 시 덮어쓰기 확인이 뜨고, 저장 뒤 초과 행이 삭제된다.
   - 자동 시작은 WARN만 남기고 진행한다.
6. **가드:**
   - 하이브리드 런 중 Open/Save/콤보 → 한글 팝업, `RecipeChange` 로그 없음.
   - 다른 화면에서 레시피를 하이브리드·1세그먼트로 각각 바꾼 뒤 Save/콤보 → stale 팝업(3b 구멍이 재현되지 않음을 확인).
   - 상단 내비게이션으로 다른 화면에 갔다가 Manual → WAD Multi로 돌아왔을 때 500 ms 안에 콤보·라벨이 busy/stale/정상으로 갱신되고, 레시피 Open 뒤 콤보가 다시 활성화된다.
7. **AgingTest 병행:** 활성 WAD 항목 2개 + Multi → `AgingCycle row` 멀티 블록 = `K + rep×(P_k-1) + ip-1`(세그먼트별), 용량 = `G×2×max P_k`.
8. **실 WAD 전용(OOS 불가):** OOS에서는 `WaitInspectionComplete` :461-462가 곧바로 반환해 `TossResultFile`까지 가지 않는다.
   - 세그먼트마다 다른 `nTotal`의 len 132 프레임을 WAD가 받는지.
   - RX/GX/BY가 세그먼트 2 리스트 값인지.
   - 활성 점 10개 이상에서 `% 10` 한 자리(:843/:845)가 되는 기존 제한 확인.
9. **Phase 4 전 기대 동작으로 기록:** 세그먼트 2 Multi 행이 같은 블록에서 세그먼트 1 행과 겹친다(segBase 미구현).

**(10) 수정 파일·인코딩** (2026-09-30 Perl로 EOL/BOM/엄밀 디코드 확인)

| 파일 | 변경 | EOL/인코딩 |
|---|---|---|
| `Source/ALPHA/Manager/WadMultiMeasMgr.h` | 정적 헬퍼 2개, `LoadMeasSegment/SaveMeasSegment/GetLoadedSegment/IsDataFound/IsMaskFound`, 멤버 3개, 헤더 주석(:9-13) | CP949, **LF-only**(CR 0), BOM 없음, 한글 주석 있음. LF 정규화 후 `\n`으로 패치, 한글은 `Encode::encode("cp949")` |
| `Source/ALPHA/Manager/WadMultiMeasMgr.cpp` | B1~B3 | 위와 동일 |
| `Source/ALPHA/DoWork/DoWorkGlassMeasureStep.cpp` | B5, B6(:2441-2453만) | CP949 CRLF. 비슷한 블록이 반복되므로 앵커 유일성 검사 |
| `Source/ALPHA/DoWork/DoWorkGlassRecipeStep.cpp` | B10, B11 | CP949 CRLF |
| `Source/ALPHA/DlgSubManual_MultiMeas.h/.cpp` | B16a(3d-0), B16b~B20(타이머 `OnTimer`/`ON_WM_TIMER`, `OnShowWindow`/`ON_WM_SHOWWINDOW` 포함) | CP949, **LF-only**, BOM 없음. 기존 팝업은 영어, **새 팝업은 한글 CP949 바이트** |
| `Source/ALPHA/ALPHA.rc` | 컨트롤 3개(:1017 뒤) | **UTF-16 LE BOM, CRLF**. python `io.open(encoding='utf-16', newline='')`. 3b 미커밋 변경 위에 얹힌다 |
| `Source/ALPHA/resource.h` | ID 3개(:578 뒤) | **UTF-8 BOM, CRLF**, ASCII. raw bytes. 3b 미커밋 변경 위에 얹힌다 |
| (무변경) `CommTcpIpObjectWAD.cpp`, `AuroraMgr.cpp`, `SystemConfigMgr.cpp`, `DoWork/DoWorkGlassMeasure.cpp`, `Recipe/RecipeEditMgr.*`, DLL 전부, `CoreFunction/dResult` | — | — |
| `docs/HybridGlass_Porting_Plan.md` | §5.9에 A15(→ 3d B표) 추가, Phase 3b D7 문구, §10에 3d 행 추가, §10 `DlgSubManual_AgingTest.cpp`를 **UTF-8 BOM, CRLF**로 정정("BOM 없는 UTF-8"은 stale) | UTF-8 |
| `docs/WAD_MultiMeas_Porting_Plan.md:139` | "회전(`m_nGlsRotate`) 반영" 정정. 코드는 회전 없는 레시피 스틱×셀 인덱스다(`RebuildCellMap` :477-489) | UTF-8 |

- 전부 `HMI/` git 트리다. 구현 뒤 `git -C HMI status`로 확인하고, 커밋 여부는 사용자에게 묻는다.

**(11) 리스크**

| 리스크 | 대응 |
|---|---|
| 세그먼트 간 점 복사(프레임·셀 크기 다름) | 콤보·라벨에 R과 셀 크기 표시, 저장 시 CELL ↔ 레시피 셀 크기 한글 확인, Import 보류(D3d-7) |
| MakeCell 재생성·세그먼트 수 변경 뒤 `[DATA_SEG*]/[MASECELL_SEG*]`가 옛 배치를 가리킴(고아 섹션 포함). 3b AgingRcp 마스크도 같은 문제 | 런타임은 범위 밖 = false로 안전. 다이얼로그 불일치 경고 + B11 WARN. 고아 섹션은 남겨 둔다 |
| UI와 측정 스레드가 한 매니저를 공유 | 하이브리드 측정 중 가드(B16), 스틱마다 재적재, 소비는 같은 스레드(B13). N=1의 기존 경합은 그대로(D3 묶음) |
| 세그먼트 간 행 충돌 | Phase 4 `segBase`까지 하이브리드 + Multi 결과는 신뢰하지 않는다(D3d-8) |
| `WAD_MULTI_RCP_PATH`를 AgingRcp 폴더로 설정하면 `[MASECELL*]` 의미 충돌(0/1 대 3비트) | 기본값이 아니고 3d가 만든 문제도 아니다. 설정 금지 메모 |

### 하이브리드 TEST 레시피 (ICP `hybrid_rcp.rcp` 대응, 2026-09-30)
- ICP `hybrid_rcp.rcp`(V1007, 2세그먼트) = Segment1 `N71_CE0_H1`(장변 절반, 5스틱 × 3셀) + Segment2 `N61_CE0_H2`(단변 절반, 5스틱 × 4셀). ICP 글라스 2290×1310을 HMI 1500×925로 **균등 배율 k = 1500/2290 = 0.6550**(사용자 지시 "균등 비율") — 925 쪽은 1310k = 858 mm라 양쪽 여유가 33.5 mm씩 늘어난다(셀 종횡비 유지).
- GlassInfo(`Source/Bin/Config`·`Config_` 두 사본 끝에 추가, ASCII CRLF, 다른 줄 무변경): `[260930_N71_CE0_H1]`(ROTATE=1, CELLX 225.8096 × CELLY 150.8011, 5 × 3) / `[260930_N61_CE0_H2]`(ROTATE=2, 197.5834 × 129.7621, 5 × 4). 좌표 키 = CAD APPLY 결과라 OPEN→RECIPE APPLY와 OPEN→CAD APPLY→RECIPE APPLY가 같은 형상. E/F = HMI 마크 19.5/16.04, 접촉·리뷰 오프셋 = ICP PAD Y/X × k, P5 ID(PROBEID9/10) 양쪽 동일. ICP rot 0(단변)은 HMI에서 **ROTATE 2**(rot 0이면 점대칭 글라스를 잰다).
- 레시피 `D:\HBS\Recipe\Recipe\9_OCP_TEST_Hybrid_Half_260930.rcp`(V1012, 세그먼트 `LONG_H1`/`SHORT_H2`): 형상 섹션은 MakeCell 수식으로 생성(09-29 LS 픽스처를 차이 0으로 재현하는 시뮬레이터 기준), 옵션·검사 항목은 `9_OCP_TEST_Hybrid_LS_260929` 세그먼트 0에서, `.alg`·헤드 Z 등 회전 의존 항목은 같은 회전의 픽스처 세그먼트에서 복사. 생성기 `scratchpad/hyb260930_gen.pl`.
- 형상: 세그먼트 1(R90) StageY 246.62…-404.15, 셀 X 611.53/375.00/138.47(A01..C05) / 세그먼트 2(R180) StageY 587.39…23.38, 셀 X ±308.92/±102.97(E01..A04). 글라스 좌표로 두 절반이 좌/우에 겹침 없이 배치(중앙 간격 약 54 mm). `CheckRecipeLayout`·SW 리밋(OOS Stage X MN -720.5, 실기 -595.5) 재생 검사 둘 다 통과.
- **OOS 3027 (2026-09-30 11:28)**: PreMeas_Move가 뒤쪽 헤드 Y1/Y2를 스틱 1 양 끝 셀(셀 X 611.5 / 138.5 + 리뷰 ±29.113)에 동시에 보내 간격 531 < `INTERLOCKHEADGAPB` 780 → `MOTION_INTERLOCK_7`. `MoveMeasureArea`도 같은 식이라 셀이 한쪽에 몰린 반쪽 세그먼트는 두 헤드 동시 배치가 불가능하다. 사용자가 세그먼트 1 리뷰를 ±154.1로 올려(간격 781) 이후 런은 통과. **재발 방지(사용자 지시 "3027 사전 검사만")**: `CheckMeasureTargetSWLimit`에 모든 세그먼트·스틱의 `|Y1 − Y2| < INTERLOCKHEADGAPB` 검사를 추가해 3027로 자동 시작 거부, WARN `[PRECHECK] segment k/n stick s HEAD Y1 … / Y2 … gap … < INTERLOCKHEADGAPB 780.000 - auto start refused`(`precheck_headgap.pl`, 컴파일 OK). 기존 레시피 13개 재생 전부 통과(최소 간격 781.25 = Half, 790.9 = 단변). 단일 헤드 폴백(한쪽 몰린 스틱에서 헤드 1개만 배치)은 보류.
- 확인 필요: 실기 R 90→180 회전 방향(OOS 무관), 접촉·리뷰 오프셋은 계산값이라 실기 티칭 필요. 운전 화면 셀 오버레이(Phase 6)는 별도 에이전트로 구현 중.

### Phase 4 — 결과 행 segBase · Cell ID 자동 이월 · 결과 저장 게이트 · 하이브리드 장치 레시피 — ✅ 구현·빌드(2026-10-01 13:10), OOS 1차 OK (N=1 AgingTest·EFA diff 0, 하이브리드 EFA 행·ID·게이트 기대값 일치) — 남은 OOS: H2~H8

**구현 결과 (2026-10-01)** — 아래 "설계 원문"은 구현 전 플랜이다. 줄 번호는 HEAD `8a454bf` 기준이라 지금 코드와 다르다. 위치는 앵커 문자열로 찾을 것.

- **진행 순서**
  1. 설계 워크플로: 매퍼 3 → 플래너 → 리뷰 2(계산·N=1 / 누락·위험) → 수정. 지적 8건 전부 반영.
  2. 사용자 결정: D4-17 (a), D4-18 (a). 나머지 D4-n은 권장안.
  3. 구현 워크플로: 4-0 → (4-1·4-2 / 4-3 / 4-4 / 4-5 병렬) → 4-7 → 전체 컴파일 → 리뷰 2 → 수정.
  - 스크립트·백업은 세션 scratchpad에 있다(`p4_4-0_patch.pl`, `p4_s41/s42/s43/s44/s45/s47_patch.pl`, `p4_fix_patch.pl`, 각 `*_bak`). 세션이 끝나면 사라지므로 복구는 git으로 한다.
- **4-0 API (`RecipeEditMgr.h/.cpp`)**
  - `GetResultRowBase()`: N≤1이면 0.
  - `GetCellIdLetterBase()`: N≤1이면 −1.
  - `Get/SetResultSaveLatch()`: N≤1이면 −1.
  - static `CellIdLetterPrefix(int)`: bijective base-26(0 A, 25 Z, 26 AA, 701 ZZ, 702 AAA). 음수는 `@`.
  - static `SegCellIdLetterExtent()`: rotate 1이면 스틱당 셀 수, 그 외 스틱 수.
  - 글자 base 캐시는 3c 캐시와 같은 곳(`SyncCellStatusStickBase`)에서 갱신한다. 저장 래치는 `RecipeChange`에서 −1로 리셋한다.
- **4-1 결과 행** (`DoWorkGlassMeasureStep.h/.cpp`, `DoWorkGlassMeasure.cpp`)
  - WAD / WAD Multi Point / VISION / ANGLE 행에 `GetResultRowBase()`를 더한다. 위치는 `SetMeasUseInfo` 전이고, VISION의 −1(행 없음)은 그대로 둔다.
  - 새 행 로그(N>1만): `[HYBRID] result row KIND ... cell(..) use(..) row(..) id(..)`.
  - 측정 시작 OPER `[HYBRID] rows per segment: seg1 rows 1..15 letters A..C seg2 rows 16..35 letters D..H (G 35)`. 3c의 "segment-local" WARN을 대체한다.
  - EFA T0 플러시 꼬리는 `- hybrid: T0 rows of all N segments (G n)`.
  - P38a/P38b: 스틱에 셀 1개뿐인 그룹(WAD bOneCell 헤드 2, VISION V2)을 N>1에서 바로잡는다. `nCellStatus[-1]` 공유 메모리 쓰기도 막는다.
  - **P38c (D4-17 (a), 모든 레시피):** `if (bOneCell && nUseCELLID[0] >= 0) memset(...)`. 1세그먼트 코드 경로의 유일한 변경이다. 정상 첨자 경로는 동작이 같고, 배열 앞 힙 헤더 쓰기만 없어진다.
- **4-2 Cell ID** (`MakeCellID`/`GenCellID`)
  - 글자 = `CellIdLetterPrefix(letterBase + …)`, N>1에서만 쓴다. N=1은 원래 `%c` 줄이 else로 그대로 돈다.
  - B 패널 다글자 ID 파서를 추가했다.
  - 31자를 넘으면 WARN을 남기고 31자로 자른다.
  - 오프라인 토큰 검사(`p4_s42_tokcheck.pl`) 결과가 플랜 (3) 표와 같다.
- **4-3 저장 게이트·APD** (`DoWorkGlassRecipeStep/ResultStep/UnloadStep.cpp`)
  - Recipe Change의 N>1 블록에서 `LatchHybridResultItems`를 부른다. 어느 세그먼트든 광학 6항목 또는 AgingTest_2가 켜져 있으면 save, 전기만이면 bypass다.
  - Result와 Unload가 같은 래치를 읽는다. 래치가 −1이면 기존 세그먼트 0 판정을 그대로 쓴다(N=1 무변경). Unload에서 AgingTest_2를 세는 것도 하이브리드만이다.
  - APD `CELL_X/Y_CNT` = G × 1 (N>1).
  - L12 dResult 병합 손실 추정 WARN. 리뷰 지적으로 마지막 광학 행(`nOptEnd`) 기준으로 고쳤다. 뒤 세그먼트가 전기만일 때 나던 오탐이 없어졌다.
- **4-4 업로드 로그** (`CommTcpIpObjectWAD/Angle/Vision`, 실기 전용): WAD/ANGLE은 Toss 시점, VISION은 grab 시점의 세그먼트 태그·행·ID를 남긴다.
- **4-5 운전 화면** (`DlgSubOperate.cpp`): 하이브리드 셀 맵 이름 = 실제 전송 ID.
- **4-7 하이브리드 장치 레시피** (D4-18 (a), N>1만; `RecipeEditMgr`, `DoWorkGlassRecipeStep`, `DoWorkGlassPreMeasStep`, `DoWorkGlassMeasureStep`)
  - `OrSegmentMeasMode`: Recipe Change와 PreMeas가 모든 세그먼트의 장치를 OR해서 연다. 로그는 L13 `[HYBRID] Recipe Change device recipe open (all segments): ...`, L14 `[HYBRID] PreMeas recipe done wait (all segments): ...`.
  - `TakeVisionRcpWait`: VISION 레시피 완료 대기를 글라스당 1회로 한다(N=1은 항상 true라 원문과 같다).
- **컴파일**
  - ALPHA 전체 ClCompile 244 유닛: 오류 0, 새 경고 0. 수정 파일의 경고 113개는 전부 기존 것이고 줄만 밀렸다.
  - 수정 파일 14개: damaged 0, 엄밀 디코드 OK, CRLF 일관.
  - 방법 메모: `SelectedFiles`에 `%3B`로 여러 파일을 넣으면 하나도 선택되지 않는다. 파일 하나씩 호출해야 한다.
- **리뷰:** N=1 불변 0건. 정확성 1건(low, L12 오탐)은 수정했다.
- **남은 결정·확인**
  - D4-14: dResult 병합 행 수 결함(기존 결함, N=1에도 영향). 하이브리드 ONLINE 전에 결정한다.
  - D4-4: APD `CELL_X/Y_CNT` = G × 1을 MES가 받아들이는지 확인한다.
  - D3c-7 하이브리드 운용 제한은 유지한다(실기 APD·MES·D4-14 뒤 해제). D3d-8은 H5 통과 시 해제한다.
- **OOS 체크리스트:** 아래 (8)의 1~8b, 실기 전용은 9번.
  1. N=1 R1(EFA)·R2(AgingTest) diff 0
  2. H1 하이브리드 EFA: seg2 행 16..35, ID D..H, T0 ID 35개
  3. H2 B 패널
  4. H3 P1RT: 게이트 save와 결과 파일 생성, 2글자 ID AO
  5. H4 전기만 bypass
  6. H5 WAD Multi Point
  7. H7 운전 화면 셀 이름
  8. H8 단일 셀 그룹(크래시 없음)
- **OOS 1차 결과 (2026-10-01, ALPHA.exe 13:10 빌드)** — 판독 스크립트 `scratchpad/n1ext2.pl`(날짜 인자), `wadnorm.pl`(WAD 프레임은 STX `0x02`로 시작한다), `p4oos_chk.pl`(화질·시야각), `p4oos_tl.pl`(스텝 시간).
  - **R2 AgingTest N=1** (13:22:31~13:25:14, 기준 09-30 16:22): GLASS 2/4/6/7/8·engine diff 0줄, WAD·PG·AGING·화질1/2 diff 0. DLOG 2줄은 스레드 간 줄 순서(`AGINGStep_None Start`)뿐. `[HYBRID]`·` seg(` 0건.
  - **R1 EFA N=1** (13:25:31~13:30:27, 기준 16:13): GLASS·engine·WAD·PG·AGING·DLOG·화질1/2·시야각1/2 전부 diff 0. 스틱별 시간은 기준과 1~4% 차이.
  - **H1 하이브리드 EFA** (13:30:35~13:37:10, 기준 15:54): 15:54 대비 차이는 의도한 것뿐이다.
    - 새 줄: L1 `-> save`, L2 `CELL_X_CNT 35 CELL_Y_CNT 1 (... 3 x 5)`, L3 `A..H ... 14/31`, L4 `rows per segment: seg1 rows 1..15 letters A..C seg2 rows 16..35 letters D..H (G 35)`, L13·L14 `PG 1 AGING 1 WAD 1 VISION 1`, L6 `T0 rows of all 2 segments (G 35)`, L7·L8 gate 1. `until Phase 4` 0건, L12 0건, WARN은 OOS의 `InnerID duplicate remapped` 1건뿐.
    - `AgingCycle row` 51줄: seg1은 15:54와 같고 seg2는 전부 +15(51/51 일치). L5 `result row` 51줄. use = 1 행: seg1 1..15, seg2 16..35, 교집합 0. seg2 첫 스틱 = 행 32..35 / `H015 H026 H037 H048`.
    - T0 서로 다른 ID 35개(A·B·C 각 5, D~H 각 4), 중복 0. 15:54에는 WAD SEND에 `B019`·`B031`이 seg1·seg2에서 두 번씩 나갔다.
    - 장치 SEND(WAD·화질1/2·시야각1/2·PG) 줄 수는 15:54와 같고 seg2 ID 글자만 +3(D→G, B→E 등, 체크섬 자리도 따라 바뀜). PG는 diff 0.
    - 결과 파일 2개(`RESULT\DATA\01\_20261001-133436.xls` T0, `-133703.xls` 최종).
    - 스틱 시간은 15:54보다 25~30% 짧다. 스텝 순서·횟수는 같고, Phase 4가 손대지 않은 스텝(Probe_Contact 1.59 → 0.48 s, AGING Snap 4.99 → 3.70 s, MoveMeasureArea 2.52 → 1.96 s)까지 고르게 줄었다 → 15:54 당시 PC 부하(빌드 동시 진행) 차이로 판단, 동작 변화 아님.
  - **OOS로 확인 못 하는 것:** APD는 N=1·하이브리드 모두 597 B 빈 틀(`AREA_CNT 0`, `CELL_X_CNT 0`)이다. 헤더 값이 WAD 결과 수신(`CommTcpIpObjectWAD` `stData[4]/[5]`)에서 채워지므로 실기 전용(9번)이다.
  - 남은 OOS: H2 B 패널, H3 P1RT, H4 전기만, H5 WAD Multi Point, H7 운전 화면 셀 이름(화면 확인), H8 단일 셀 그룹.

---

**설계 원문 (2026-10-01 플랜, 리뷰 반영본)**

상태(설계 시점): **플랜(2026-10-01), 리뷰 완료, 구현 전.** 사용자 결정 D4-17 (a)·D4-18 (a) 확정 — 구현 결과는 위 블록.
- 출처: 워크플로 매퍼 3개(`p4_map_rows.md` 행 / `p4_map_cellid.md` Cell ID / `p4_map_gate.md` 게이트) → 이 플래너. 매퍼 주장은 코드·로그로 다시 확인했다(확인 스크립트 `scratchpad/p4plan/chk.pl`, `anchors.txt`, `anchors2.txt`, 기대값 `expect.pl`).
- 코드 기준: `D:\OCP\0.SW_Matching\HMI`, 브랜치 `hmi-master`, HEAD `8a454bf`, **작업 트리 깨끗함**(`git status` 0건). 복구는 `git checkout -- <path>`.
- 앵커: 아래 표의 앵커는 전부 2026-10-01에 raw 바이트를 CRLF→LF로 정규화한 사본에서 `index()`로 **1회**임을 확인했다. pfw.pl(`rd`가 LF 정규화)에 그대로 넣을 수 있다. 표의 `\t`/`\n`은 실제 탭/개행이다.
- 줄 번호는 참고용이다. `DoWorkGlassMeasureStep.cpp`는 7029줄(3c 플랜 때 7023줄). 플랜 §5.7의 `:2767/2773`, `:2962-2968`, `:3805`, `:5236`은 낡았다.
- **HARD RULE:** 1세그먼트(N=1) 레시피는 동작·행 번호·Cell ID·결과 파일·CIM 데이터·로그 문자열이 바이트 동일해야 한다. 모든 변경은 "N>1에서만 다른 값" 구조로 넣는다((6)).

---

**(0) 출발점 — 2026-09-30 15:54:57~16:02:56 하이브리드 EFA OOS 런 판독**

| 항목 | 값 |
|---|---|
| 레시피 | `D:\HBS\Recipe\Recipe\9_OCP_TEST_Hybrid_Half_260930.rcp`(mtime 15:54). seg1 `LONG_H1` rot1 5스틱×3셀, seg2 `SHORT_H2` rot2 5스틱×4셀. G = 15+20 = 35 |
| 검사 항목 | 두 세그먼트 동일: Pre 1, Elect 1, AgingTest_2 1, Spec 1, Quality 1, Color 0, Angle 1, EFA 1 |
| GlassID | `ABCDEFG1234`(끝에서 두 번째 `3` → **A 패널**). Cell ID 접두 = `ABCDEFG123` |
| EFA | 2 pass. T0: seg1 WAD(스틱 1/3/5) + VISION(2/4), seg2 VISION(1/3/5) + WAD(2/4). T1: ANGLE(seg1 스틱 1~2, 3~5 stick skip / seg2 전 스틱) |
| 캐파 로그 | `15:54:57.744 [HYBRID] capacity: sticks 10/50, result rows 35/2500 (G 35 x block 1; ...)` |
| 3c 경고 | `15:55:05.437 WARN [HYBRID] result rows are segment-local until Phase 4 (segBase): seg1 rows 1..15 seg2 rows 1..20 ...` |

*행 — `GLASS 6_MEASURE_20260930.log` `AgingCycle row`(이 런 51줄)*

| 구간 | 로그 row(a,b) | 사용 행(지금) | Phase 4 행 |
|---|---|---|---|
| T0 seg1 s0 WAD | `row(0,1) row(0,2) row(0,3) seg(1/2) rowbase(0)` | 1,2,3 | 1,2,3 (무변경) |
| T0 seg1 s1 VISION | `row(-1,4)..row(-1,6)` | 4,5,6 | 4,5,6 |
| T0 seg1 s2/s4 WAD | `row(6,7..9)`, `row(12,13..15)` | 7-9, 13-15 | 같음 |
| T0 seg2 s0 VISION | `row(-1,17) row(-1,18) row(19,-1) row(20,-1) seg(2/2) rowbase(15)` | 17-20 | **32-35** |
| T0 seg2 s1 WAD | `row(12,13) row(12,14) row(15,12) row(16,12)` | 13-16 | **28-31** |
| T0 seg2 s2 VISION | `row(-1,9) row(-1,10) row(11,-1) row(12,-1)` | 9-12 | **24-27** |
| T0 seg2 s3 WAD | `row(4,5) row(4,6) row(7,4) row(8,4)` | 5-8 | **20-23** |
| T0 seg2 s4 VISION | `row(-1,1) row(-1,2) row(3,-1) row(4,-1)` | 1-4 | **16-19** |
| T1 seg2 s0..s4 ANGLE | `(16,17)(20,16) / (12,13)(16,12) / (8,9)(12,8) / (4,5)(8,4) / (0,1)(4,0)` | 17,20 / 13,16 / 9,12 / 5,8 / 1,4 | **32,35 / 28,31 / 24,27 / 20,23 / 16,19** |

- **`row(a,b)` 판독 규칙(코드로 확인):** a = 헤드/장치 0, b = 헤드/장치 1.
  - `-1` = VISION만 쓰는 "행 없음" 센티널(`QualityWaitSet`이 `nSheetRow < 0`이면 무시, `CommTcpIpObjectVision.cpp:1589`).
  - WAD/ANGLE의 쉬는 헤드(c = -1)는 `A·ms + G·rep`라는 **가짜 행**이다. 0은 "없음"이 아니라 ms=0, rep=0의 가짜 행이다. `bMeasProbe[h]=false` → `iMeasMode`가 한쪽만 고르므로 소비되지 않는다. **예외(리뷰 B1):** VISION V2와 WAD W3 bOneCell은 `nUseCELLID[0]` 기반 값을 실제로 기록한다 — (3)·D4-3·D4-16.
- **충돌(지금):** WAD 시트 seg1 9행 중 {7,8,13,14,15} 5행, VISION 6행 중 {4,10,11,12} 4행, ANGLE 6행 중 {1,4,5} 3행을 seg2가 덮는다. 병합(행 조인)에서는 한 줄에 다른 셀 값이 섞이고, 21~35행은 존재하지 않는다.

*Cell ID — `D:\HBS\Logs\Control\WAD\WAD_20260930.log`*
- 같은 문자열이 다른 셀로 두 번 나갔다:
  - `15:55:24:720 SEND [WAD](136) : 51302 2155524 ABCDEFG123B019 0` (seg1 스틱1 셀 B)
  - `15:59:33:488 SEND [WAD](136) : 51302 2155929 ABCDEFG123B019 0` (seg2 스틱4 셀 1)
- seg2 WAD 토큰: s1 `D011 D022 D033 D044`, s3 `B019 B020 B031 B042`. 화질1 seg2 s0 `E034 E045`, s2 `C032 C043`, s4 `A030 A041`.
- T0 글라스 35셀에 서로 다른 ID는 23개뿐이다(겹침 12개: A01-A04, B01-B04, C01-C04). InnerID는 달라 raw 파일은 충돌하지 않는다.
- PG(Ensis) 필드: seg1 `010..050`(rot 플래그 0), seg2 `051 041 031 021 011`(플래그 1) — PG 쪽도 seg2가 A..E를 다시 쓴다.

*결과 파일*
- `D:\HBS\RESULT\DATA\30\_20260930-160005.xls`(T0 플러시), `_20260930-160249.xls`(최종): 3584 B, 빈 시트.
- `D:\HBS\RESULT\APD\APD_20260930-160249.txt`: 597 B, `AREA_CNT 0`, `CELL_X_CNT 0`, `CELL_Y_CNT 0`, `GLASS_ID` 공란.
- 원인: OOS에서는 WAD `WaitInspectionComplete`가 `IsOutOfService()`에서 `TossResultFile` 전에 반환하고, VISION quality-wait도 수신이 없다. 그래서 dResult `UploadData`가 한 번도 안 불린다. **APD 내용(행·헤더·CELLID 열)은 OOS로 검증할 수 없다.** 행과 ID의 OOS 증거는 시퀀스 로그와 장치 SEND 로그뿐이다.

*게이트*
- 15:54 런: Result `16:02:48.465~.482`, Unload `CheckSaveReuslt Start/End` 16:02:56.504 같은 ms. 판정 로그 없음. seg0에 광학이 있어 파일은 생겼다.
- 2026-09-29 17:26~17:39 `1_OCP_TEST_Long_P1RT`(seg1 = Pre/Elect만, seg2 = Spec/Quality/Angle): seg2를 실측정했는데 `RESULT\DATA\29\`에 17:39 파일 없음, `APD_20260929-1739*` 없음. 게이트가 복귀한 세그먼트 0만 봤다.

*N=1 기준선(바이트 동일 대상, `1_OCP_TEST_Long_260907` rot1 5스틱×20셀, G 100)*
- 16:13 EFA: T0 WAD `row(11,1)…row(60,50)`, ANGLE `row(0,1) row(20,0)…row(80,81) row(100,80)`, VISION `row(71,61)…row(100,90)`, T1 WAD 1..60. AgingCycle row 90줄(16:13~16:19). T0 Cell ID 100개 A01..T05 전부 서로 다름(`A018 B019 C010 … T017`).
- 16:22 AgingTest: WAD `row(11,1) row(13,3)…`, VISION `row(-1,11) row(12,-1)…`, s4 WAD `row(91,81)…row(100,90)`. 40줄(VISION 최대 행 30, ANGLE 없음).
- 결과: 16:13 T0 `_161638` + 최종 `_161805`, 16:22 `_162541`. APD는 START/END만 다르다.

---

**(1) 목적·범위**

- **(a) 결과 행 segBase**
  - 실가동 행 사이트 7분기(WAD 기본점 일반·bOneCell, WAD-MP 에이징/비에이징, VISION nMeasArea==1·일반, ANGLE)에 `segBase_k = Σ_{j<k} A_j·S_j`를 더한다.
  - 위치는 스텝 안, `SetMeasUseInfo` **전**이다(D4-1). 그래서 로그 값 = 장치가 받는 값이고, VISION grab 시점 캡처(F6)도 맞는다.
  - `nAreaNum` 소비처 전수 감사 결과는 (2g). 깔때기(`CCommTcpIpFactoryMgr::SetMeasUseInfo`)·장치 3종·dResult는 받은 값을 그대로 쓰므로 바꾸지 않는다.
  - **단일 셀 그룹 마스터 정정(N>1만, 리뷰 B1, P38).** 쉬는 헤드의 가짜 행이 실제로 쓰이는 분기가 둘 있다: VISION V2(`nMeasArea == 1`, 셀이 헤드2에 배정되면 `nUseCELLID = {-1, c}`인데 장치0이 마스터로 grab·기록)와 WAD W3 bOneCell(헤드1 행을 `nUseCELLID[0]`으로 계산). segBase를 더하면 이 쓰기가 앞 세그먼트 마지막 셀 행(segBase_k)을 덮는다. N>1에서는 V2의 `nUseCELLID[0]`을 마스터 셀로 바꾸고(행·Cell ID·`nCellStatus` 첨자가 함께 맞는다), W3 헤드1 행을 자기 셀로 계산한다. N=1은 그대로(D4-16).
- **(b) Cell ID 자동 글자 이월**(ICP `BuildSegmentAutoCellIds` 규칙, §5.7 1단계)
  - 글자 base = 앞 세그먼트들의 글자 수 합. 축은 HMI 토큰 규칙을 따른다(rot1 → 셀 수, 그 외 → 스틱 수).
  - N>1은 bijective base-26(Z 다음 AA). **N=1은 기존 `%c` 경로 그대로**(base 0).
  - B 패널 파서를 여러 글자에 맞추고(N>1만), 31자 초과를 막는다(N>1만).
  - 운전 화면 하이브리드 셀맵 이름도 같은 규칙으로 맞춘다(D4-10, ALPHA N>1 경로만).
- **(c) APD 헤더 글라스 전체 값:** N>1이면 `stGlassInfo.m_nCellCount/m_nStickCount` = **G × 1**(D4-4). 운반 경로(WAD stData[4]/[5] → dResult `m_nCellCountX/Y` → `CELL_X_CNT/CELL_Y_CNT`)는 그대로다.
- **(d) 결과 저장 게이트 통일:** Result·Unload가 같은 값 = "어느 세그먼트든 광학(Spec_3/Quality_4/Color_5/Angle_6/TSP_7/LCR_8) 또는 AgingTest_2" (사용자 확정 2026-09-29). 글라스 시작(Recipe Change N>1 블록)에서 1회 래치한다(D4-6). **N=1은 원문 게이트 그대로**(D4-5).
- **(e) 세그먼트 스냅샷 로그(N>1만)**
  - 측정 스레드: `SetMeasUseInfo`마다 1줄(셀·사용 헤드·행·Cell ID·` seg(k/N) rowbase(b)`). OOS에서도 찍힌다.
  - 장치 업로드: WAD/ANGLE/VISION `TossResultFile` 업로드 줄에 행·Cell ID·스냅샷(실기 전용, D3c-14). VISION은 grab 시점 태그를 quality-wait 슬롯에 보관한다.
  - 측정 시작 1줄(세그먼트별 행 범위·글자 범위)이 3c WARN을 대체한다.
  - APD는 **글라스당 1개** 그대로다(세그먼트별 APD 없음).
  - Recipe Change에서 dResult 병합 잘림 추정 WARN(L12, D4-14 노출을 측정 전에 알림, 리뷰 A2).
- **(g) 하이브리드 장치 레시피(4-7, D4-18 — 사용자 선택):** 글라스 시작의 장치 레시피 열기(Recipe Change)·완료 대기(PreMeas)가 세그먼트 0 플래그만 보고, VISION 레시피 완료 대기는 세그먼트마다 스틱 0에서 다시 기다린다(자동 리셋 이벤트). seg0이 전기만인 하이브리드(P1RT)는 실기에서 WAD 레시피를 못 받고 seg2 VISION 대기에서 `HEAD_ACK_TIMEOUT`으로 멈춘다(리뷰 B2, Phase 3 결함). N>1에서만 모든 세그먼트 플래그를 OR하고 VISION 대기를 글라스당 1회로 한다. OOS는 이 경로가 항상 성공이라 보이지 않는다.
- **(f) AREA_MAX 2500 용량 검사:** 이미 있다(DLL 저장 `ValidateSegments` G ≤ 2500, 자동 시작 `CheckHybridCapacity` G × block ≤ 2500). segBase 뒤 최대 행 = segBase_{N-1} + A·S + G·(block−1) = G·block이라 **식이 그대로 유효**하다. 코드 변경 없음.
- **범위 밖**
  - Phase 4b(Cell ID A/B 수동·편집기, DLL 화면의 Cell ID 표시), Phase 5(Resume 저널), Phase 6 나머지 표시기.
  - PG(Ensis) 프로토콜 스틱 필드(D4-11, §5.7 로컬 유지).
  - dResult 병합 행 수 결함(D4-14), WAD bOneCell 헤드1 행·VISION V2 마스터 결함의 **N=1 쪽**(D4-16, N>1 쪽은 P38로 고침), dResult `CELL_X/Y` 글라스 간 잔존(D4-4 주), Resume 재적재 식 RS1. 모두 N=1 동작 변경이라 별도 승인 항목으로 둔다. 예외 하나: `:2597` 경계 밖 memset은 D4-17로 사용자에게 묻는다.

---

<details><summary>(2) 접점 표 P1–P42 (설계 시점 원문, 펼치기)</summary>

**(2) 접점 표** (줄 번호 = 2026-10-01 HEAD `8a454bf`, 앵커는 전부 LF 정규화 raw에서 n=1)

*(2a) 4-0 API — `Source/ALPHA/Recipe/RecipeEditMgr.h` / `.cpp` (CP949 CRLF, 159 / 2448줄)*

| # | file:line, 앵커 | 지금 | 변경 | N=1 |
|---|---|---|---|---|
| P1 | `.h:62` 줄 전체 `\tvolatile int\t\t\tm_nEfaGlassLatch;\t// LYS 2026-09-30 Hybrid 3c A5 EFA glass latch: -1 = not latched, 0 = not EFA, n = EFA passes (reset in RecipeChange)\n` 뒤 | 3c 캐시 4개까지 | 멤버 2개 추가: `volatile int m_nSegLetterBaseCache;` (앞 세그먼트 글자 수 합, Sync만 씀) / `volatile int m_nResultSaveLatch;` (-1 = 미래치, 0 = bypass, 1 = save) | ALPHA 내부 클래스 끝 추가. 공유메모리·DLL 아님 |
| P2 | `.h:61` `\tvolatile int\t\t\tm_nSegRowBaseCache;\t//   cell x stick sum of the segments before the active one (Phase 4 segBase preview, printed only)\n`, `.h:120` `\tint\t\tGetSegRowBase() const\t{ return m_nSegRowBaseCache; }\t// Phase 4 segBase preview - printed only, never added to a row in 3c` | 주석 "printed only" | 주석만 갱신: "segBase of the active segment - GetResultRowBase adds it to every live result row" | 주석 |
| P3 | `.h:122` 줄 전체 `\tint\t\tGetEfaGlassLatch() const\t{ return m_nEfaGlassLatch; }\t// A5 -1 = not latched (1-segment recipe / after RecipeChange), 0 = not EFA, n = EFA passes\n` 뒤 | — | 선언 6개((4)): `GetResultRowBase() const`, `GetCellIdLetterBase() const`, `static CellIdLetterPrefix(int)`, `static SegCellIdLetterExtent(C_OutRecipeFunc*, int)`, `SetResultSaveLatch(int)` 인라인, `GetResultSaveLatch() const` | 호출처 없으면 무영향 |
| P4 | `.cpp:44` 줄 전체 `\tm_nEfaGlassLatch = -1;\t// LYS 2026-09-30 Hybrid 3c A5 not latched\n` 뒤 | 생성자 | `\tm_nSegLetterBaseCache = 0;\t// LYS 2026-10-01 Hybrid Phase 4` + `\tm_nResultSaveLatch = -1;` | 대입만 |
| P5 | `.cpp:139` 줄 전체 `\tm_nEfaGlassLatch = -1;\t// LYS 2026-09-30 Hybrid 3c A5 new recipe / new glass: the next measure start latches again\n` 뒤 (`RecipeChange`, LoadRcpFile 성공 뒤) | — | `\tm_nResultSaveLatch = -1;\t// LYS 2026-10-01 Hybrid Phase 4 new recipe / new glass: Recipe Change latches again (hybrid only)` | 대입만, N=1은 이후 아무도 0/1로 안 바꿈 |
| P6 | `.cpp:352-353` `\tm_nSegRowBaseCache = nRowBase;\n}\n` (`SyncCellStatusStickBase` 끝) | rowbase 캐시까지 | 글자 base 캐시 계산 추가(아래 코드). 갱신 지점은 그대로 두 곳(`RecipeChange`, `SegmentSwitchData` `DoWorkGlassMeasure.cpp:569`) | `nActSeg = 0`이라 루프 0회 → 0 |
| P7 | `.cpp:384` `bool CRecipeEditMgr::SaveRecipe(CString strRecipeID)` 앞 | — | 본문 5개(아래 코드) | 새 함수 |

```cpp
// P6 - replace "\tm_nSegRowBaseCache = nRowBase;\n}\n"
	m_nSegRowBaseCache = nRowBase;

	// LYS 2026-10-01 Hybrid Phase 4 Cell ID letter carry (ICP BuildAutoCellIds: segment index order, not run order)
	int nLetterBase = 0;
	for (int k = 0; k < nActSeg && k < nSegCnt; k++)
		nLetterBase += SegCellIdLetterExtent(m_pRecipeFunc, k);
	m_nSegLetterBaseCache = nLetterBase;
}

// P7 - before "bool CRecipeEditMgr::SaveRecipe(CString strRecipeID)"
// LYS 2026-10-01 Hybrid Phase 4 (docs/HybridGlass_Porting_Plan.md 5.7). Same guard as GetSegLogTag: int caches + vector size only,
//   so any thread may call them and a stale cache (CDlgTestMode reload without RecipeChange) never leaks into a 1-segment recipe.
int CRecipeEditMgr::GetResultRowBase() const
{
	if (m_nSegCountCache <= 1 || m_pRecipeFunc == NULL || m_pRecipeFunc->GetSegmentLoopCount() <= 1) return 0;
	return m_nSegRowBaseCache;
}

int CRecipeEditMgr::GetCellIdLetterBase() const
{
	if (m_nSegCountCache <= 1 || m_pRecipeFunc == NULL || m_pRecipeFunc->GetSegmentLoopCount() <= 1) return -1;
	return m_nSegLetterBaseCache;
}

int CRecipeEditMgr::GetResultSaveLatch() const
{
	if (m_nSegCountCache <= 1 || m_pRecipeFunc == NULL || m_pRecipeFunc->GetSegmentLoopCount() <= 1) return -1;
	return m_nResultSaveLatch;
}

// bijective base-26 (ICP RecipeFunc.cpp:869 CellIdLetterPrefix): 0 A .. 25 Z, 26 AA, 40 AO, 51 AZ, 52 BA, 701 ZZ, 702 AAA
//   negative -> "@" (the legacy "%c" of 65 - 1): never a real cell's letter (review B1 - clamping to 'A' named a real cell)
CString CRecipeEditMgr::CellIdLetterPrefix(int nIndex)
{
	CString strPrefix;
	if (nIndex < 0) return CString(_T("@"));
	do
	{
		strPrefix.Insert(0, (TCHAR)(_T('A') + (nIndex % 26)));
		nIndex = nIndex / 26 - 1;
	} while (nIndex >= 0);
	return strPrefix;
}

// letters one segment uses = the MakeCellID letter axis: rotate 1 -> cells per stick (stick 0, uniform by file format), else sticks.
//   Reads the segment vectors - sequence thread (SyncCellStatusStickBase, Recipe Change) or UI GenGlassPath only.
int CRecipeEditMgr::SegCellIdLetterExtent(C_OutRecipeFunc* pFunc, int nSeg)
{
	if (pFunc == NULL) return 0;
	return (pFunc->GetSegmentRotate(nSeg) == 1) ? pFunc->GetSegmentCellCount(nSeg) : pFunc->GetSegmentStickCount(nSeg);
}
```

*(2b) 4-1 결과 행 segBase + 행 로그 — `DoWork/DoWorkGlassMeasureStep.h/.cpp` (CP949 CRLF, 296 / 7029줄), `DoWork/DoWorkGlassMeasure.cpp` (CP949 CRLF, 764줄)*

| # | file:line, 앵커 | 지금 | 변경 | N=1 |
|---|---|---|---|---|
| P8 | `.h:83` `\tint  GetAgingMaskSeg();\n` 뒤 | — | `\tvoid LogHybridRow(const TCHAR* szKind, int* pnCell, bool* pbUse, int* pnAreaNum);\t// LYS 2026-10-01 Hybrid Phase 4 one line per SetMeasUseInfo (hybrid only)` | 선언 |
| P9 | `.cpp:509` `bool CDoWorkGlassMeasureStep_None::Run()` 앞 | — | `LogHybridRow` 본문(아래) | 태그 빈 문자열이면 즉시 반환 |
| P10 (W1→W2,W3) | `.cpp:2827` 줄 전체 `\t\t\t\t\t\tint nAgingRow = m_pShareItem->GetRecipeEditMgrPtr()->GetGlassRowBlock(nArea, nMaxStick) * nAgingRep;\n` 뒤 | `nAgingRow = G·rep_W`. W2 `:2843-2844`, W3 `:2833-2834`(bOneCell, `+ (nMaxStick * k)`)가 둘 다 `+ nAgingRow` | `\t\t\t\t\t\tnAgingRow += m_pShareItem->GetRecipeEditMgrPtr()->GetResultRowBase();\t// LYS 2026-10-01 Hybrid Phase 4 segBase = rows of the segments before this one (1 segment: 0). Both branches below add nAgingRow` | `+= 0` |
| P11 (W5,W6) | `.cpp:3030` 줄 전체 `\t\t\t\t\t\t\t\tm_pShareItem->GetCommTcpIpFactoryMgr()->SetMeasUseInfo(bMeasProbe, nAreaNumM, nTimeOut);\n` 앞 | W5 `:3022-3023`(`* nBlock`), W6 `:3027-3028`(`* ip`) 둘 다 segBase 없음 | 8탭 3줄: 주석 `// LYS 2026-10-01 Hybrid Phase 4 segBase for both layouts above (1 segment: 0)` + `nAreaNumM[0] += m_pShareItem->GetRecipeEditMgrPtr()->GetResultRowBase();` + `nAreaNumM[1] += ...;` | `+= 0` |
| P12 (V1→V2,V3) | `.cpp:3865` 줄 전체 `\t\t\t\t\t\tnAgingRow = m_pShareItem->GetRecipeEditMgrPtr()->GetGlassRowBlock(nArea, nMaxStick) * m_pMeasSeqItemEngine->GetAgingRepeat(enumAgingMeasKind_VISION);\n` 뒤 | `int nAgingRow = 0; if (사이클) nAgingRow = G·rep_V;`. V2 `:3873`(head0 master, head1 = -1), V3 `:3882`(c ≥ 0일 때만 계산, 아니면 -1) | 5탭 `nAgingRow += m_pShareItem->GetRecipeEditMgrPtr()->GetResultRowBase();\t// LYS 2026-10-01 Hybrid Phase 4 segBase (1 segment: 0). Added only where a row is computed - the -1 "no row" stays -1` | `+= 0`. -1 센티널은 계산 분기 밖이라 그대로 |
| P13 (N1) | `.cpp:5296` 줄 전체 `\t\t\t\t\t\tnAgingRow = m_pShareItem->GetRecipeEditMgrPtr()->GetGlassRowBlock(nArea, nMaxStick) * m_pMeasSeqItemEngine->GetAgingRepeat(enumAgingMeasKind_ANGLE);\n` 뒤 | ANGLE `:5299-5300` `+ nAgingRow` | 5탭 같은 한 줄(주석 "segBase (1 segment: 0)") | `+= 0` |
| P14 | `.cpp:2847` 줄 전체 `\t\t\t\t\t\tLogAgingRow(enumAgingMeasKind_WAD, _T("WAD"), nAreaNum);\t// LYS 2026-08-12 AgingCycle R1\n` 뒤 | — | 6탭 `LogHybridRow(_T("WAD"), nUseCELLID, bMeasProbe, nAreaNum);\t// LYS 2026-10-01 Hybrid Phase 4 (1 segment: no line)` | 줄 없음 |
| P15 | `.cpp:3031` 줄 전체 `\t\t\t\t\t\t\t\tLogAgingRow(enumAgingMeasKind_WAD, _T("WAD-MP"), nAreaNumM);\t// LYS 2026-08-12 AgingCycle R1\n` 뒤 | — | 8탭 `LogHybridRow(_T("WAD-MP"), nUseCELLID, bMeasProbe, nAreaNumM);` | 줄 없음 |
| P16 | `.cpp:3890` 줄 전체 `\t\t\t\t\tLogAgingRow(enumAgingMeasKind_VISION, _T("VISION"), nAreaNum);\t// LYS 2026-08-12 AgingCycle R1\n` 뒤 | — | 5탭 블록(아래 코드, 리뷰 A3): VISION은 "use = 그 장치 결과가 행에 기록됨"(`bMeasProbe && row >= 0`)으로, V2(`nMeasArea == 1`)는 두 카메라가 같은 셀을 grab하므로 `cell(c,c)`로 찍는다. V3에서는 `bMeasProbe`와 같은 값이라 H1 기대값 무변경 | 줄 없음(계산만, `LogHybridRow`가 태그 ""면 반환) |
| P17 | `.cpp:5302` 줄 전체 `\t\t\t\t\tLogAgingRow(enumAgingMeasKind_ANGLE, _T("ANGLE"), nAreaNum);\t// LYS 2026-08-12 AgingCycle R1\n` 뒤 | — | 5탭 `LogHybridRow(_T("ANGLE"), nUseCELLID, bMeasProbe, nAreaNum);` | 줄 없음 |
| P18 | `DoWorkGlassMeasure.cpp:204-212`, 9줄 블록 전체(첫 줄 `\t\t// LYS 2026-09-30 Hybrid 3c result rows are segment-local until Phase 4 (segBase) - say which rows overlap\n` ~ 끝 줄 `\t\tm_pLogFileAccessor->KMLOG(WARN, strRowLog);\n`, 블록 전체가 n=1) | 3c WARN "rows are segment-local" | OPER 1줄로 교체: 세그먼트별 행 범위 + 글자 범위 + G(아래) | `if (nSegCount > 1)` 블록 안 |
| P19 | `DoWorkGlassMeasure.cpp:520` 줄 전체 `\tif (nBoundarySeg > 1) strLog += _T(" - hybrid: rows of the segments overlap until Phase 4 (segBase)");\t// LYS 2026-09-30 Hybrid 3c\n` | 3c 꼬리 | `if (nBoundarySeg > 1) { ... " - hybrid: T0 rows of all %d segments (G %d)" ... }` (G = `GetGlassRowBlock(0, 0)`, N>1이면 Σ) | N>1 조건 |
| P38a (W3) | `.cpp:2834` 줄 전체 `\t\t\t\t\t\t\tnAreaNum[1] = nUseCELLID[0] + (nArea * nMeasStick) + 1 + (nMaxStick * k) + nAgingRow;\n` 뒤 (n=1, 7탭, `if (bOneCell) {` 안) | 헤드1 행을 `nUseCELLID[0]`으로 계산 → 짝 `{c0,c1}`이면 c1 결과가 c0 행을, `{-1,c}`(헤드2 단독)면 가짜 행(segBase_k = 앞 세그먼트 마지막 셀)을 덮는다 | 7탭 2줄(아래): N>1이고 `nUseCELLID[1] >= 0`이면 `nAreaNum[1]`을 자기 셀로 다시 계산(일반 분기와 같은 식, `nAgingRow`에 segBase 포함) | `GetCellIdLetterBase() < 0` → 원문 줄 그대로 |
| P38b (V2) | `infunc('bool CDoWorkGlassMeasureStep_Measure_VISION::Run()')` 안에서 `\t\t\t\tif (nUseCELLID[1] < 0) bMeasProbe[1] = false;\n` 뒤 (:3420, 파일 전체 n=5 / 함수 안 n=1) | V2는 셀이 헤드2에 배정되면 `nUseCELLID = {-1, c}`인데 :3448/:3589 Cell ID(`nUseHead = {-1,-1}` → `@01`/`X00`), :3873 행(가짜 행, 장치0이 기록), :3910 `nCellStatus[-1][..]`(공유메모리 경계 밖 쓰기)가 모두 `nUseCELLID[0]`을 쓴다 | 4탭 2줄(아래): N>1이고 `nMeasArea == 1 && nUseCELLID[0] < 0`이면 `nUseCELLID[0] = nUseCELLID[1]`. `bMeasProbe[0]`은 false 그대로라 상태 검사·PG 설정·:3836 루프는 원래대로 헤드2만 본다 | N=1 → 원문 동작(결함 포함, D4-16) |
| P38c (D4-17) | `.cpp:2597` 줄 전체 `\t\t\t\t\t\tif (bOneCell) memset(m_pMeasSeqItemEngine->m_pStInnerID[nUseCELLID[0]], 0x00, sizeof(m_pMeasSeqItemEngine->m_pStInnerID[nUseCELLID[0]]));\n` (n=1) | `{-1,c}`면 `m_pStInnerID[-1]`(= `new TCHAR*[n]` 앞 힙 헤더)을 포인터로 읽어 8바이트를 0으로 쓴다 — 크래시/힙 손상, N=1에도 있음 | rep1. D4-17 (a) 권장: `if (bOneCell && nUseCELLID[0] >= 0) memset(...)` / (b) N>1만: `if (bOneCell && (nUseCELLID[0] >= 0 \|\| m_pShareItem->GetRecipeEditMgrPtr()->GetCellIdLetterBase() < 0)) memset(...)` | (a) 정의된 동작은 불변(UB 경로만 없어짐), (b) 바이트 동일 |

```cpp
// P9 - before "bool CDoWorkGlassMeasureStep_None::Run()"
// LYS 2026-10-01 Hybrid Phase 4 one line per live SetMeasUseInfo on a hybrid recipe: the rows the devices will write (segBase
//   included), the cells, which heads really measure and the cell IDs GenMeasID just put in stLotInfo. OOS prints it too
//   (dResult gets no rows offline). 1-segment recipe: GetSegLogTag() is empty -> no line (1-segment logs stay byte-identical).
void CDoWorkGlassMeasureStep::LogHybridRow(const TCHAR* szKind, int* pnCell, bool* pbUse, int* pnAreaNum)
{
	CString strTag = m_pShareItem->GetRecipeEditMgrPtr()->GetSegLogTag(TRUE);
	if (strTag.IsEmpty() || m_pMeasSeqItemEngine == NULL) return;

	CString strLog;
	strLog.Format(_T("[HYBRID] result row %s stick(%d) cell(%d,%d) use(%d,%d) row(%d,%d) id(%s,%s)%s"),
		szKind, GetMeasureStick(), pnCell[0], pnCell[1], pbUse[0] ? 1 : 0, pbUse[1] ? 1 : 0, pnAreaNum[0], pnAreaNum[1],
		m_pMeasSeqItemEngine->GetShareMemoryMgrPtr()->GetSharedMemory()->stLotInfo.m_szCellID[0],
		m_pMeasSeqItemEngine->GetShareMemoryMgrPtr()->GetSharedMemory()->stLotInfo.m_szCellID[1], (LPCTSTR)strTag);
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);
}

// P18 - replaces DoWorkGlassMeasure.cpp:204-212 (inside "if (nSegCount > 1)")
		// LYS 2026-10-01 Hybrid Phase 4 result rows / cell ID letters per segment (rows carry segBase from Phase 4 on)
		//   prefix is not "[HYBRID] result row" on purpose - that one is the per-SetMeasUseInfo line (review B4: grep counts)
		CString strRowLog = _T("[HYBRID] rows per segment:"), strRowOne;
		int nRowFrom = 1, nLetterFrom = 0;
		for (int k = 0; k < nSegCount; k++)
		{
			int nRows = pSegFunc->GetSegmentCellCount(k) * pSegFunc->GetSegmentStickCount(k);
			int nLetters = CRecipeEditMgr::SegCellIdLetterExtent(pSegFunc, k);
			strRowOne.Format(_T(" seg%d rows %d..%d letters %s..%s"), k + 1, nRowFrom, nRowFrom + nRows - 1,
				(LPCTSTR)CRecipeEditMgr::CellIdLetterPrefix(nLetterFrom), (LPCTSTR)CRecipeEditMgr::CellIdLetterPrefix(nLetterFrom + nLetters - 1));
			strRowLog += strRowOne;
			nRowFrom += nRows;
			nLetterFrom += nLetters;
		}
		strRowOne.Format(_T(" (G %d)"), nRowFrom - 1);
		strRowLog += strRowOne;
		m_pLogFileAccessor->KMLOG(OPER, strRowLog);

// P19 - replaces DoWorkGlassMeasure.cpp:520
	if (nBoundarySeg > 1)	// LYS 2026-10-01 Hybrid Phase 4 the T0 file holds every segment's T0 rows (segBase); 3c said "rows overlap"
	{
		CString strT0Rows;
		strT0Rows.Format(_T(" - hybrid: T0 rows of all %d segments (G %d)"), nBoundarySeg, m_pShareItem->GetRecipeEditMgrPtr()->GetGlassRowBlock(0, 0));
		strLog += strT0Rows;
	}

// P16 - after the VISION LogAgingRow line (5 tabs). Review A3: log what each camera stores
					{	// LYS 2026-10-01 Hybrid Phase 4 use = this device's result is written (VISION drops row -1); one group (nMeasArea == 1): both cameras grab cell 0's slot
						int nLogCell[2] = { nUseCELLID[0], (nMeasArea == 1) ? nUseCELLID[0] : nUseCELLID[1] };
						bool bLogUse[2] = { bMeasProbe[0] && nAreaNum[0] >= 0, bMeasProbe[1] && nAreaNum[1] >= 0 };
						LogHybridRow(_T("VISION"), nLogCell, bLogUse, nAreaNum);
					}

// P38a - after the bOneCell nAreaNum[1] line (7 tabs)
							if (m_pShareItem->GetRecipeEditMgrPtr()->GetCellIdLetterBase() >= 0 && nUseCELLID[1] >= 0)	// LYS 2026-10-01 Hybrid Phase 4 head 2 writes its own cell's row (1 segment: the line above, D4-16)
								nAreaNum[1] = nUseCELLID[1] + (nArea * nMeasStick) + 1 + (nMaxStick * k) + nAgingRow;

// P38b - inside CDoWorkGlassMeasureStep_Measure_VISION::Run, after "if (nUseCELLID[1] < 0) bMeasProbe[1] = false;" (4 tabs)
				if (nMeasArea == 1 && nUseCELLID[0] < 0 && m_pShareItem->GetRecipeEditMgrPtr()->GetCellIdLetterBase() >= 0)	// LYS 2026-10-01 Hybrid Phase 4 (review B1)
					nUseCELLID[0] = nUseCELLID[1];	// one VISION group: both cameras image this cell and device 0 is the master (row / cell ID / nCellStatus). Head-2 single cell left -1 here (1 segment: unchanged)
```

- P38b 뒤 V2 하이브리드 줄은 `cell(c,c) use(1,0) row(segBase+c+A·ms+1+G·rep, -1) id(X(c),X(c))`가 된다. `bMeasProbe[0]`이 :3875에서 true로 강제되므로 :3910은 `nCellStatus[c]`를 두 번 쓴다(같은 값).
- V2 이동은 셀 X와 무관하게 두 헤드를 ±HeadGap/2로 보낸다(:3622-3653, `MarkIdleHeadSafety`도 이 축은 건드리지 않음 — :446 주석). 그래서 장치0 마스터 설계는 셀이 어느 헤드에 배정됐든 유효하다. 1셀이 X≠0이면 카메라가 셀 중심에 있지 않은 것은 기존 동작(기록만).

*(2c) 4-2 Cell ID 이월 — `DoWork/DoWorkGlassMeasureStep.cpp` (MakeCellID :137-169, GenCellID :181-247)*

| # | file:line, 앵커 | 지금 | 변경 | N=1 |
|---|---|---|---|---|
| P20 | `:143-145` `\tZeroMemory(cCellID, sizeof(cCellID));\n\n\tfor (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)` (MakeCellID 안, 이 3줄 조합이 n=1. `ZeroMemory(cCellID` 단독은 n=11). **`rep1`로 넣는다**(리뷰 B5): 앵커가 `iProbe++)`에서 끝나 `ins_after`면 선언이 for 몸체가 되어 C2065. 치환 = `"\tZeroMemory(cCellID, sizeof(cCellID));\n" . P20줄 . "\n" . "\n\tfor (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)"` (대안: `infunc('int CDoWorkGlassMeasureStep::MakeCellID(')` + `ins_after("\tZeroMemory(cCellID, sizeof(cCellID));\n")`, 함수 안 n=1) | — | P20줄 = `\tint nLetterBase = m_pShareItem->GetRecipeEditMgrPtr()->GetCellIdLetterBase();\t// LYS 2026-10-01 Hybrid Phase 4 letter carry; -1 = 1-segment recipe -> the original "%c" lines below` | -1 |
| P21 | `:153` 줄 전체 `\t\t\t\tstrCellId.Format(_T("%c%02d"), 65 + nCellNum, iStick + 1);\n` **앞**(원문 줄은 그대로 남아 else 몸체가 된다) | rot1: 글자 = 셀, 번호 = 스틱+1 | `\t\t\t\tif (nLetterBase >= 0 && nCellNum >= 0) strCellId.Format(_T("%s%02d"), (LPCTSTR)CRecipeEditMgr::CellIdLetterPrefix(nLetterBase + nCellNum), iStick + 1);\t// LYS 2026-10-01 Hybrid Phase 4 (cell -1 -> the legacy "@" line)\n\t\t\t\telse\n`. `nCellNum >= 0` 조건(리뷰 B1): -1이 들어오면 base−1 = 앞 세그먼트 마지막 글자가 되던 것을 원문 `@`로 돌린다(P38b 뒤에는 V2에서도 -1이 오지 않는다 — 방어) | else = 원문 줄 |
| P22 | `:158` 줄 전체 `\t\t\t\tstrCellId.Format(_T("%c%02d"), 65 + nMaxStick - iStick - 1, nCellNum + 1);\n` 앞 | 그 외 rot: 글자 = 뒤집은 스틱, 번호 = 셀+1 | `\t\t\t\tif (nLetterBase >= 0) strCellId.Format(_T("%s%02d"), (LPCTSTR)CRecipeEditMgr::CellIdLetterPrefix(nLetterBase + nMaxStick - iStick - 1), nCellNum + 1);\n\t\t\t\telse\n` | else = 원문 줄. **반환 `nMeasStick`에는 base를 더하지 않는다**(nCellStatus 열은 3b A8 `ToCellStatusStick`이 따로 처리) |
| P23 | `:185` 줄 전체 `\tint iTop = 0, nMaxTop = 5;\n` 뒤 (GenCellID) | — | `\tbool bHybridCid = (m_pShareItem->GetRecipeEditMgrPtr()->GetCellIdLetterBase() >= 0);\t// LYS 2026-10-01 Hybrid Phase 4 (false = 1-segment recipe -> original B-panel parser, no length guard)` | false |
| P24 | `:231` 줄 전체 `\t\t\t\tstrBuf.Format(_T("%s%02d"), strTmp.Mid(0, 1), nMaxTop + _ttoi(strTmp.Mid(1, 2)));\n` 앞 | B 패널: 글자 1개 + 숫자 2자리를 가정. `AO01 `이면 `A05`처럼 틀린다 | 아래 블록 + `else`(원문 줄이 else 몸체) | else = 원문 줄(번호 ≥100 절단 동작까지 보존) |
| P25 | `:244` 줄 전체 `\t\tTRACE(_T("GenCellID:%s %d,%d:%s\n"), strMeasCell, pnCellNum[nProbe] + 1, nProbe + 1, strMcrID);\n` 앞 (주의: 이 줄의 `\n`은 C 문자열 안의 백슬래시-n 두 글자) | `_tcscpy_s(...m_szCellID[nProbe], 32, strMcrID)`(:245) — 32자 이상이면 abort | N>1에서 31자 초과 시 WARN + 31자로 자름(아래) | 조건 false |

```cpp
// P24 - inserted before the original B-panel line (4 tabs), which stays as the else body
				if (bHybridCid)	// LYS 2026-10-01 Hybrid Phase 4 the letter prefix may be 2 letters (AA01): split letters / digits instead of Mid(0,1) / Mid(1,2)
				{
					int nPfx = 0;
					while (nPfx < strTmp.GetLength() && strTmp[nPfx] >= _T('A') && strTmp[nPfx] <= _T('Z')) nPfx++;
					strBuf.Format(_T("%s%02d"), (LPCTSTR)strTmp.Left(nPfx), nMaxTop + _ttoi(strTmp.Mid(nPfx)));
				}
				else

// P25 - before the TRACE line (2 tabs, inside the probe loop)
		if (bHybridCid && strMcrID.GetLength() > 31)	// LYS 2026-10-01 Hybrid Phase 4 m_szCellID[..][32]: _tcscpy_s aborts on 32+ chars (2-letter prefix + long GlassID)
		{
			CString strCidLog;
			strCidLog.Format(_T("[HYBRID] cell ID %s longer than 31 - cut to 31"), (LPCTSTR)strMcrID);
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(WARN, strCidLog);
			strMcrID = strMcrID.Left(31);
		}
```

- `nMaxTop`(:195-202)은 활성 세그먼트 값 그대로다(rot1 → 스틱 수, 그 외 → 스틱0 셀 수). ICP `SegmentBNumberOffset`과 같은 의미라 바꾸지 않는다.
- 호출처 10곳(`nMeasStick = MakeCellID(`)·`GenMeasID(` 9곳은 무수정. 장치 객체는 `stLotInfo.m_szCellID[슬롯]`만 읽는다.

*(2d) 4-3 글라스 전체 결과 항목 — `DoWork/DoWorkGlassRecipeStep.cpp` (CP949 CRLF 1020줄), `DoWork/DoWorkGlassResultStep.cpp` (CP949 CRLF 138줄, 4칸 공백 들여쓰기 구간 있음), `DoWork/DoWorkGlassUnloadStep.cpp` (CP949 CRLF 298줄)*

| # | file:line, 앵커 | 지금 | 변경 | N=1 |
|---|---|---|---|---|
| P26 | `DoWorkGlassRecipeStep.cpp:540` `bool CDoWorkGlassRecipeStep_Change::Run()` 앞 | — | `static void LatchHybridResultItems(CBaseSequenceImpl*, C_OutRecipeFunc*)`(아래) | 호출 안 됨 |
| P27 | `:596` 줄 전체 `\t\t\tCheckHybridAgingMask(m_pShareItem, pSegFunc);\n` 뒤 (N>1 블록 `:570` 안, `if (bAlignOK != true)` 앞) | stGlassInfo `:563-564` = 세그먼트 0(`m_nCellCnt/m_nStickCnt`) | `\t\t\tLatchHybridResultItems(m_pShareItem, pSegFunc);\t// LYS 2026-10-01 Hybrid Phase 4 save gate latch + APD header G x 1 + cell ID letters (hybrid only)` | `GetSegmentCount() > 1` 블록 밖이라 미도달. `:563-564` 원문 대입은 그대로 |
| P28 | `DoWorkGlassResultStep.cpp:73-75` `    if (pRecipeInfo != nullptr)\n    {\n        if (pRecipeInfo->m_sInspectItem.m_bUseAgingTest_2 == true)` 앞 | 활성(= 복귀한 seg0) 7항목 OR | 래치 분기 + `else`(원문 `if (pRecipeInfo != nullptr) {...}` 블록이 else 몸체) | 래치 -1 → 원문 블록 |
| P29 | `DoWorkGlassUnloadStep.cpp:198-200` `\tif (pRecipeInfo != nullptr)\n\t{\n\t\tif (pRecipeInfo->m_sInspectItem.m_bUseSpec_3 == true)` 앞 | seg0 6항목 OR(AgingTest_2 없음) → false면 `SetUserBypassGlass(true)` :239 | 같은 래치 분기 + `else` | 래치 -1 → 원문 블록(N=1 Unload에 AgingTest_2 추가 안 함, D4-5) |

```cpp
// P26 - DoWorkGlassRecipeStep.cpp, before "bool CDoWorkGlassRecipeStep_Change::Run()"
// LYS 2026-10-01 Hybrid Phase 4 glass-wide result items, latched once per glass on this (sequence) thread - the segment vectors
//   are read only here (same place as CheckHybridCapacity / CheckHybridAgingMask). Hybrid recipes only.
//   (1) result save gate (user policy 2026-09-29): any segment with an optical item (Spec_3 / Quality_4 / Color_5 / Angle_6 /
//       TSP_7 / LCR_8) or AgingTest_2 -> save; electrical only -> no file + Unload CIM bypass. Result and Unload read the latch.
//   (2) APD header CELL_X_CNT / CELL_Y_CNT = G x 1 (stGlassInfo -> WAD result stData[4]/[5] -> dResult): X x Y = row block size G
//   (3) cell ID letter range and the longest ID vs m_szCellID[..][32] - log only (GenCellID cuts and warns at run time)
//   (4) D4-14 estimate (review A2): dResult ExportAsRawDateICP merges sheets 1..4 (WAD 1 / VISION 2 / ANGLE 3) with the MaxRows of the
//       LAST non-empty sheet, so rows above the top sheet's last row drop out of the merged sheet and the APD. Recipe flags only
//       (AgingRcp/EFA masks, sampling and AgingCycle blocks are not counted) - WARN, nothing is refused
static void LatchHybridResultItems(CBaseSequenceImpl* pShare, C_OutRecipeFunc* pFunc)
{
	CRecipeEditMgr* pRcp = pShare->GetRecipeEditMgrPtr();
	int nSeg = pFunc->GetSegmentCount();

	bool bSave = false, bSave0 = false;
	int nLetters = 0, nRowEnd = 0;
	int nSheetRow[4] = { 0, 0, 0, 0 };	// last reachable T0 row per dResult sheet: [1] WAD Spec_3, [2] VISION Quality_4, [3] ANGLE Angle_6
	CString strGate = _T("[HYBRID] result save gate:"), strOne;
	for (int k = 0; k < nSeg; k++)
	{
		nLetters += CRecipeEditMgr::SegCellIdLetterExtent(pFunc, k);
		nRowEnd += pFunc->GetSegmentCellCount(k) * pFunc->GetSegmentStickCount(k);
		S_RecipeMgr* pSeg = pFunc->GetSegmentMgr(k);
		if (pSeg == nullptr) continue;
		const S_InspectItem& sItem = pSeg->m_sInfo.m_sInspectItem;
		bool bOptical = (sItem.m_bUseSpec_3 == TRUE || sItem.m_bUseQuality_4 == TRUE || sItem.m_bUseColor_5 == TRUE
			|| sItem.m_bUseAngle_6 == TRUE || sItem.m_bUseTSP_7 == TRUE || sItem.m_bUseLCR_8 == TRUE);
		bool bAging2 = (sItem.m_bUseAgingTest_2 == TRUE);
		if (bOptical || bAging2) { bSave = true; if (k == 0) bSave0 = true; }
		if (sItem.m_bUseSpec_3 == TRUE)    nSheetRow[1] = nRowEnd;
		if (sItem.m_bUseQuality_4 == TRUE) nSheetRow[2] = nRowEnd;
		if (sItem.m_bUseAngle_6 == TRUE)   nSheetRow[3] = nRowEnd;
		strOne.Format(_T("%s seg%d [%s] optical %d aging2 %d"), (k > 0) ? _T(",") : _T(""), k + 1, pFunc->GetSegmentName(k).GetString(), bOptical ? 1 : 0, bAging2 ? 1 : 0);
		strGate += strOne;
	}
	strGate += bSave ? _T(" -> save") : _T(" -> bypass (electrical only)");
	if (bSave && bSave0 == false) strGate += _T(" (segment 1 alone: bypass)");
	pRcp->SetResultSaveLatch(bSave ? 1 : 0);
	pShare->GetLogFileAccessorPtr()->KMLOG(OPER, strGate);

	int nTopSheet = 3;
	while (nTopSheet > 0 && nSheetRow[nTopSheet] == 0) nTopSheet--;
	if (nTopSheet > 0 && nSheetRow[nTopSheet] < nRowEnd)
	{
		CString strMerge;
		strMerge.Format(_T("[HYBRID] APD merge keeps rows 1..%d of %d (top sheet %s ends at row %d, dResult D4-14) - later rows miss the merged sheet / APD"),
			nSheetRow[nTopSheet], nRowEnd, (nTopSheet == 3) ? _T("ANGLE") : (nTopSheet == 2) ? _T("VISION") : _T("WAD"), nSheetRow[nTopSheet]);
		pShare->GetLogFileAccessorPtr()->KMLOG(WARN, strMerge);
	}

	int nG = pRcp->GetGlassRowBlock(0, 0);
	CString strApd;
	strApd.Format(_T("[HYBRID] APD header CELL_X_CNT %d CELL_Y_CNT 1 (glass rows G x 1; segment 1 alone %d x %d)"), nG,
		pShare->GetShareMemoryMgrPtr()->GetSharedMemory()->stGlassInfo.m_nCellCount, pShare->GetShareMemoryMgrPtr()->GetSharedMemory()->stGlassInfo.m_nStickCount);
	pShare->GetShareMemoryMgrPtr()->GetSharedMemory()->stGlassInfo.m_nCellCount = nG;
	pShare->GetShareMemoryMgrPtr()->GetSharedMemory()->stGlassInfo.m_nStickCount = 1;
	pShare->GetLogFileAccessorPtr()->KMLOG(OPER, strApd);

	CString strGlassID = CSystemParamShare::GetInstance()->GetGlassID();
	CString strLast = CRecipeEditMgr::CellIdLetterPrefix(nLetters - 1);
	int nLongest = max(0, strGlassID.GetLength() - 1) + strLast.GetLength() + 2 + 1;	// GlassID - 1 + letters + 2 digits + checksum
	CString strCid;
	strCid.Format(_T("[HYBRID] cell ID letters A..%s over %d segments, longest ID %d/31 chars (GlassID length %d)"), strLast.GetString(), nSeg, nLongest, strGlassID.GetLength());
	pShare->GetLogFileAccessorPtr()->KMLOG(nLongest > 31 ? WARN : OPER, strCid);
}

// P28 - DoWorkGlassResultStep.cpp, before "    if (pRecipeInfo != nullptr)\n    {\n        if (...m_bUseAgingTest_2 == true)" (4 spaces)
    // LYS 2026-10-01 Hybrid Phase 4 hybrid recipe: the glass save gate latched at Recipe Change (any segment optical / AgingTest_2).
    //   -1 (1-segment recipe, or reloaded after the latch) -> the original segment-0 check below, unchanged
    int nSaveLatch = m_pShareItem->GetRecipeEditMgrPtr()->GetResultSaveLatch();
    if (nSaveLatch >= 0)
    {
        bUseMeas = (nSaveLatch > 0);
        m_pShareItem->GetLogFileAccessorPtr()->KMLOG(OPER, bUseMeas ? _T("[HYBRID] Result: glass save gate 1 - SaveResult") : _T("[HYBRID] Result: glass save gate 0 (electrical only) - no result file"));
    }
    else

// P29 - DoWorkGlassUnloadStep.cpp, before "\tif (pRecipeInfo != nullptr)\n\t{\n\t\tif (...m_bUseSpec_3 == true)" (tabs)
	// LYS 2026-10-01 Hybrid Phase 4 the same latch as Result (AgingTest_2 counts here too for hybrid recipes). -1 -> original check below
	int nSaveLatch = m_pShareItem->GetRecipeEditMgrPtr()->GetResultSaveLatch();
	if (nSaveLatch >= 0)
	{
		bUseMeas = (nSaveLatch > 0);
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(OPER, bUseMeas ? _T("[HYBRID] Unload: glass save gate 1 - wait SaveResultDone") : _T("[HYBRID] Unload: glass save gate 0 (electrical only) - CIM bypass (DATAJUDGE_NOTMEAS)"));
	}
	else
```

- 원문 줄 비교식은 `== true`(BOOL 대 bool 1)이고 새 식은 `== TRUE`(1)라 같은 결과다.
- bypass는 Unload false 분기(`:239`)가 유일한 측정 경로 설정점이고 CIM `Cmd_Send_DataJudge`(`CommTcpIpObjectCIM.cpp:2350`)가 그대로 따른다. CIM 코드는 바꾸지 않는다.

*(2e) 4-4 장치 업로드 스냅샷 로그(실기 전용) — `CommTcpIpObjectWAD.cpp`(CP949 CRLF 1377), `CommTcpIpObjectAngle.cpp`(CP949 CRLF 855), `CommTcpIpObjectVision.h/.cpp`(CP949 CRLF 111 / 1676)*

| # | file:line, 앵커 | 지금 | 변경 | N=1 |
|---|---|---|---|---|
| P30 | `CommTcpIpObjectWAD.cpp:1359` 줄 전체 `\t\tstrLog.Format(_T("[%s] : %s, %s"), GetName(), _T("TossResultFile Updata"), strInnerID);\n` 뒤 | `TossResultFile Updata, <InnerID>`만 | 2탭 블록: `CString strSegTag = m_pRecipeEditMgr->GetSegLogTag(TRUE);` → 비어 있지 않으면 `strLog += " row(%d) cell(%s)%s"`(nArea, `resultData.szCellData`, 태그). WAD Toss는 측정 스레드의 완료 대기 안이라 이 시점 태그 = 측정 시점 | 태그 "" → strLog 그대로 |
| P31 | `CommTcpIpObjectAngle.cpp:853` 줄 전체 `\tCResultMgr::GetInstance()->Updata_Data(&resultData);\n` 뒤 | 업로드 로그 없음 | 태그가 있으면 `[%s] : TossResultFile Updata, %s row(%d) cell(%s)%s`(GetName, InnerID, nArea, `stLotInfo.m_szCellID[m_iDeviceIndex]`, 태그)를 `m_pLogFileAccessor`(Control\시야각n)에 OPER. 수신 스레드지만 스텝이 `INSP_COMPLETE`를 기다리는 중이라 세그먼트는 측정 중인 것 | 줄 없음 |
| P32 | (a) `CommTcpIpObjectVision.h:14` 줄 전체 `\tCString m_strCellID;\n`(단독 n=1) 뒤에 `ins_after`(리뷰 B5: 3줄 앵커 `...\n\n\tint\t\tm_nSheetRow;`로 `ins_after`하면 새 멤버가 `m_nSheetRow;` 뒤·한글 주석 앞에 들어가 주석을 삼킨다) / (b) `:26` `\t\tm_strCellID = _T("");\n` 뒤 | `S_QualityWait`(ALPHA 내부, memset 없음 확인) | `\tCString m_strSegTag;\t// LYS 2026-10-01 Hybrid Phase 4 segment snapshot at grab ("" = 1 segment)` / `\t\tm_strSegTag = _T("");` | 빈 문자열 |
| P33 | `CommTcpIpObjectVision.cpp:1595` 줄 전체 `\t\t\tsQualityWait[i].m_nSheetRow\t  = nSheetRow;\n` 뒤 (`QualityWaitSet`, 측정 스레드 grab 시작) | — | `\t\t\tsQualityWait[i].m_strSegTag = (m_pRecipeEditMgr != nullptr) ? m_pRecipeEditMgr->GetSegLogTag(TRUE) : CString();\t// LYS 2026-10-01 Hybrid Phase 4` | "" |
| P34 | `CommTcpIpObjectVision.cpp:1544` 줄 전체 `\t\t\t\t\tpQuality->TossResultFile(pQuality->sQualityWait[i].m_strInnerID, pQuality->sQualityWait[i].m_nSheetRow);\n`(**5탭**) 뒤 | 업로드 로그 없음 | 슬롯 태그가 있으면 `[%s] : TossResultFile Updata, %s row(%d) cell(%s)%s`(GetName, 슬롯 InnerID, 슬롯 행, 슬롯 CellID, 슬롯 태그). 최대 150 s 뒤 다음 세그먼트에서 도착해도 grab 시점 값이 찍힌다 | 줄 없음 |

*(2f) 4-5 운전 화면 셀맵 이름 — `DlgSubOperate.cpp` (CP949 CRLF 4376줄, `BuildHybridCellMap` :3904, N>1에서만 진입)*

| # | file:line, 앵커 | 지금 | 변경 | N=1 |
|---|---|---|---|---|
| P35 | `:3933` 줄 전체 `\tint nBase = 0, nSkip = 0;\n` 뒤 | — | `\tint nLetterBase = 0;\t// LYS 2026-10-01 Hybrid Phase 4 cell ID letter carry (same rule as MakeCellID)` | 함수가 `:3908`에서 N≤1이면 반환 |
| P36 | `:3973-3978` 6줄 블록 전체(`\t\t\t\tif (sCell.m_chCellID[0] != 0)\n` ~ `...65 + nStick - i - 1, j + 1);\n`, 블록 n=1) | 레시피 `m_chCellID`(세그먼트마다 A부터) 우선, 없으면 `%c` fallback | `if (bLong) _stprintf_s(..., _T("%s%02d"), (LPCTSTR)CRecipeEditMgr::CellIdLetterPrefix(nLetterBase + j), i + 1); else ...((LPCTSTR)CRecipeEditMgr::CellIdLetterPrefix(nLetterBase + nStick - i - 1), j + 1);` + 주석 "the transmitted cell ID, not the recipe's per-segment m_chCellID" | 미도달 |
| P37 | `:4003` 줄 전체 `\t\tnBase += nStick;\n` 뒤 | — | `\t\tnLetterBase += CRecipeEditMgr::SegCellIdLetterExtent(pFunc, k);` | 미도달 |

- 이 함수는 GenGlassPath(레시피 열기·글라스 시작, 세그먼트 0 활성)에서만 세그먼트 벡터를 읽는 기존 규칙 안이다. A 패널 토큰을 표시한다(오늘과 같음, B 오프셋은 표시하지 않음).

*(2h) 4-7 하이브리드 장치 레시피(D4-18, 사용자 승인 시) — `RecipeEditMgr.h/.cpp`, `DoWorkGlassRecipeStep.cpp`, `DoWorkGlassPreMeasStep.cpp`(CP949, BOM 없음, CRLF 1203, damaged 0 확인), `DoWorkGlassMeasureStep.cpp`*

근거(리뷰 B2, 코드 확인): `CDoWorkGlassRecipeStep_Change::Run` `:674-683`이 활성 슬롯(= 세그먼트 0) 플래그로 `bMeasMode`를 정하고 `:698-731`에서 PG/AGING/WAD/VISION `DoWorkRecipeOpen`. `CDoWorkGlassPreMeasStep_RcpChangeDone::Run` `:704-712`도 같은 방식으로 PG Event_1·AGING 완료 대기(`:747/:752`). `SegmentBoundary`(`DoWorkGlassMeasure.cpp:669-686`)는 PG/WAD를 **이름이 바뀔 때만** 다시 보내고(빈 이름 = RecipeID라 보통 안 보냄) VISION은 보내지 않는다. VISION 레시피 완료 대기는 `DoWorkGlassMeasureStep.cpp:3845` `if (iStick == 0 && i == 0)`(스텝 로컬 스틱 → 세그먼트마다 다시 참)에서 `WaitRecipeChangeEnd`가 자동 리셋 이벤트(`CommTcpIpObjectVision.cpp:69`)를 기다린다 → 두 번째 대기, 또는 RecipeChange를 안 보낸 글라스의 대기는 `HEAD_ACK_TIMEOUT`(:539-541). OOS는 `IsOutOfService()`에서 모두 true.

| # | file:line, 앵커 | 변경 | N=1 |
|---|---|---|---|
| P39 | `RecipeEditMgr.h` P1 뒤 / P3 뒤, `.cpp` P4·P5 뒤 / P7 블록 | 멤버 `volatile int m_nVisionRcpWaitLatch;`(생성자·`RecipeChange`에서 0). 선언·본문 `BOOL OrSegmentMeasMode(bool* pbMeasMode, CString* pstrLog = NULL);`(모든 세그먼트 플래그 OR, `enumMEASURE_MODE_*`는 같은 헤더 `:12-23`에 있음) / `bool TakeVisionRcpWait();`(아래) | `OrSegmentMeasMode` → FALSE·무접촉, `TakeVisionRcpWait` → 항상 true |
| P40 | `DoWorkGlassRecipeStep.cpp:683` 줄 전체 `\t\tbMeasMode[enumMEASURE_MODE_PG] = true;\n` 뒤 (n=1) | 1탭 블록: `OrSegmentMeasMode(bMeasMode, &strDevLog)`가 TRUE면 OPER `[HYBRID] Recipe Change device recipe open (all segments): PG %d AGING %d WAD %d VISION %d` | 호출되지만 FALSE → 무변경, 로그 없음 |
| P41 | `DoWorkGlassPreMeasStep.cpp:712` 줄 전체 `\t\tbMeasMode[enumMEASURE_MODE_PG] = true;\n` 뒤 (n=1) | 같은 블록, 로그 `[HYBRID] PreMeas recipe done wait (all segments): ...` | 같음 |
| P42 | `infunc('bool CDoWorkGlassMeasureStep_Measure_VISION::Run()')` 안 `\t\t\t\t\tif (iStick == 0 && i == 0)\n`(파일 n=2 :3845/:6135, 함수 안 n=1) | rep1 → `\t\t\t\t\tif (iStick == 0 && i == 0 && m_pShareItem->GetRecipeEditMgrPtr()->TakeVisionRcpWait())\t// LYS 2026-10-01 Hybrid Phase 4 (4-7) hybrid: once per glass (MOT.RECIPE.CHANGE is sent once, RECIPE_CHANGE_OK auto-resets); 1 segment: every stick 0 as before\n` | 단락 평가 + 항상 true → 원문과 같음 |

```cpp
// P39 - RecipeEditMgr.cpp (P7 block)
// LYS 2026-10-01 Hybrid Phase 4 (4-7) devices any segment needs - same mapping as the Recipe Change / PreMeas flag lines. Sequence thread
//   only (reads the segment vectors). FALSE = 1-segment recipe -> pbMeasMode untouched
BOOL CRecipeEditMgr::OrSegmentMeasMode(bool* pbMeasMode, CString* pstrLog)
{
	if (pbMeasMode == NULL || m_pRecipeFunc == NULL || m_pRecipeFunc->GetSegmentCount() <= 1) return FALSE;
	for (int k = 0; k < m_pRecipeFunc->GetSegmentCount(); k++)
	{
		S_RecipeMgr* pSeg = m_pRecipeFunc->GetSegmentMgr(k);
		if (pSeg == NULL) continue;
		const S_InspectItem& s = pSeg->m_sInfo.m_sInspectItem;
		if (s.m_bUseAgingTest_2 == TRUE) pbMeasMode[enumMEASURE_MODE_AGING] = true;
		if (s.m_bUseSpec_3 == TRUE)      pbMeasMode[enumMEASURE_MODE_WAD] = true;
		if (s.m_bUseQuality_4 == TRUE)   pbMeasMode[enumMEASURE_MODE_VISION] = true;
		if (s.m_bUseColor_5 == TRUE)     pbMeasMode[enumMEASURE_MODE_COLOR] = true;
		if (s.m_bUseAngle_6 == TRUE)     pbMeasMode[enumMEASURE_MODE_ANGLE] = true;
		if (s.m_bUseTSP_7 == TRUE)       pbMeasMode[enumMEASURE_MODE_TSP] = true;
		if (s.m_bUsePre_0 == TRUE || s.m_bUseElect_1 == TRUE || s.m_bUseAgingTest_2 == TRUE) pbMeasMode[enumMEASURE_MODE_PG] = true;
	}
	if (pstrLog != NULL)
		pstrLog->Format(_T(" PG %d AGING %d WAD %d VISION %d"), pbMeasMode[enumMEASURE_MODE_PG] ? 1 : 0, pbMeasMode[enumMEASURE_MODE_AGING] ? 1 : 0,
			pbMeasMode[enumMEASURE_MODE_WAD] ? 1 : 0, pbMeasMode[enumMEASURE_MODE_VISION] ? 1 : 0);
	return TRUE;
}

// LYS 2026-10-01 Hybrid Phase 4 (4-7) VISION recipe-done wait once per glass on a hybrid recipe (measure thread). 1 segment: always true
bool CRecipeEditMgr::TakeVisionRcpWait()
{
	if (m_nSegCountCache <= 1 || m_pRecipeFunc == NULL || m_pRecipeFunc->GetSegmentLoopCount() <= 1) return true;
	if (m_nVisionRcpWaitLatch != 0) return false;
	m_nVisionRcpWaitLatch = 1;
	return true;
}

// P40 - DoWorkGlassRecipeStep.cpp, after "\t\tbMeasMode[enumMEASURE_MODE_PG] = true;\n" (P41 the same with "PreMeas recipe done wait")
	// LYS 2026-10-01 Hybrid Phase 4 (4-7) every device a later segment uses - segment 0 alone opened no WAD / VISION for 1_OCP_TEST_Long_P1RT (1 segment: no-op)
	{
		CString strDevLog;
		if (m_pShareItem->GetRecipeEditMgrPtr()->OrSegmentMeasMode(bMeasMode, &strDevLog))
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(OPER, CString(_T("[HYBRID] Recipe Change device recipe open (all segments):")) + strDevLog);
	}
```

- 경계의 PG/WAD 이름 변경 재전송(`SegmentBoundary`)은 그대로 둔다 — 글라스 시작에 seg0 이름으로 열고, 다른 이름의 세그먼트로 넘어갈 때만 다시 보낸다.
- VISION 레시피는 세그먼트별 이름이 없다(RecipeID). 대기가 어느 세그먼트의 스틱 0에서도 안 일어나는 EFA 교차 배치(seg1 VISION이 s1/s3)는 N=1과 같은 기존 동작이다.
- 범위 밖(기록): N=1 AgingCycle에서 VISION 항목이 스틱 0을 두 번 이상 지나면 같은 재대기가 실기에서 시간 초과할 수 있다(16:22 런 OOS, 실기 미확인). N=1이라 손대지 않는다.

*(2g) 무변경 확인 · `nAreaNum` 소비처 감사* (코드 그대로, 이유만)

| 대상 | 위치 | 판정 |
|---|---|---|
| 깔때기 `SetMeasUseInfo` | `Manager/CommTcpIpFactoryMgr.cpp:2313-2318` `memcpy(m_nAreaNum, nAreaNum, sizeof(m_nAreaNum));` | 복사만. segBase는 그 전(스텝)에 접혀 있다. 수동 측정 4곳(`DlgSubManual_Meas.cpp:3373/3508/3629/3750`)과 공유하므로 여기서 더하지 않는다(D4-1) |
| WAD Toss | `CommTcpIpObjectWAD.cpp:486`(헤드0) / `:390`(헤드1) `m_pCommTcpIpFactoryMgr->m_nAreaNum[0|1]` → `:1285` `resultData.iMeasArea = nArea;` | 받은 값 그대로 |
| ANGLE Toss | `CommTcpIpObjectAngle.cpp:710` `m_nAreaNum[m_iDeviceIndex]` → `:842` | 그대로 |
| VISION | `CommTcpIpFactoryMgr.cpp:2226` `InspectStartMulti(lDeviceIndex, m_nAreaNum[lDeviceIndex])` → `CommTcpIpObjectVision.cpp:413` `QualityWaitSet` 슬롯 캡처 → `:1544` Toss → `:1505` | grab 시점 캡처라 늦게 도착해도 행은 맞다. `:761` `nArea`(quality-wait 꺼짐 = 수동 전용, 행 = 장치 번호, 기존 결함)는 범위 밖 |
| COLOR / VISION_COLOR | `DoWorkGlassMeasureStep.cpp:4564-4566`, `:6148-6167` | 스텝 미등록(`DoWorkGlassMeasure.cpp:61-62` 주석), COLOR Toss도 주석(`CommTcpIpObjectColor.cpp:527`). 재활성화하면 G·block과 segBase가 둘 다 없다 — 기록만 |
| 그리드 깔때기 | `Manager/ResultMgr.cpp:51` `CResultMgr::Updata_Data` | 무변경(Phase 5 저널 자리) |
| dResult | `CoreFunction/dResult/CDlgResult.cpp`(UTF-8 BOM) 행 = `iMeasArea + 1`, 병합 `:1790-1868`, APD `:1941/1953/1956` | 무변경. 병합 행 수 결함은 D4-14. `m_nCellCountX/Y`는 생성자(`:108`)에서만 0이고 `nCntData > 0` 업로드(WAD뿐, `:535/:539`)에서만 갱신, `DataClear`(`:2396`)는 안 지운다 → 다음 글라스가 WAD 결과 없이 끝나면 이전 글라스 값이 APD에 남는다(리뷰 B3, D4-4 주) |
| Resume 재적재 | `DoWorkGlassRecipeStep.cpp:128` `rd.iMeasArea = iCell + (nCellsPerStick * nMeasStick) + 1;` | N=1 그대로(HARD RULE). 하이브리드는 D3c-11 "No(새로)" + Phase 5 저널 |
| `nCellStatus` 열·PG 슬롯 | `ToCellStatusStick`(15곳), `m_nAgingStartStick[4]`(OST :6293 / RTEST :6393 / AGING :6577), PG `CommTcpIpObjectPG.cpp:1702` | 행이 아니다. 3b A8로 이미 글라스 전역. PG 필드는 로컬(D4-11) |
| InnerID | `GenInnerID :249`, `m_pStInnerID[스틱 내 셀]` | Cell ID에서 파생되지 않음. 영향 없음 |
| 용량 | `DoWorkGlassRecipeStep.cpp:351` `int nRows = nG * nBlock;`, DLL `InRecipeFunc.cpp:1149` | 식 유효((1)-(f)) |
| `stGlassInfo` 소비처 | `CommTcpIpObjectWAD.cpp:1341/1346`뿐(HMI 트리 전체 grep) | APD 헤더 외 파급 없음 |
| EFA^1$ / MeasDone | `DoWorkSequenceCIMStep.cpp:781` 3c 래치 | 무관 |
| 수동 측정 Cell ID 사본 | `DlgSubManual_Meas.cpp:4663`(UTF-8 BOM) | 활성 = seg0이라 base 0과 같다. Phase 7 선택기 때 재검토 |
| DLL 화면 Cell ID | `GlassMap.cpp:58`, `RecipeMgrDlg.cpp:1653`, `SelectCellDlg.cpp:157`, DLL `DlgTestMode.cpp:168` | 레시피 `m_chCellID`(세그먼트 로컬) 표시가 남는다 → Phase 4b(리졸버와 함께, D4-10) |
| SYST 로그 `HeadPair firstNG` | `DoWorkGlassMeasureStep.cpp:433` `cellMinus.m_chCellID` | 레시피 ID가 찍힌다(로그만, 기록) |

---

</details>

**(3) 산식·ID 규칙**

*산식*
- `row = segBase_k + c + A_k·ms + 1 + G·block`
  - `segBase_k = Σ_{j<k} A_j·S_j`, `G = Σ_j A_j·S_j`. N=1: `segBase = 0`, `G = A·S` → 오늘 식과 같다.
  - `ms = (rot == 1) ? s : S_k − 1 − s` (rot 0/2/3은 역순). `s` = 세그먼트 로컬 측정 순서.
  - `A_k` = 스틱당 셀 수. 포맷상 세그먼트 안에서 균일하다(DLL `GetSegmentCellCount` 주석 "저장이 GetCellCount(0)을 쓴다").
  - `block`: WAD 기본점·VISION·ANGLE = `rep_K`(AgingCycle 종류별 dense 카운터, 비활성 0). WAD-MP 비활성 = `ip`, 활성 = `K_w + rep_W·(P−1) + ip−1`. W3 bOneCell은 `+ S·k`(k = 0 고정)가 더 붙는다.
  - 쉬는 헤드(WAD/ANGLE, c = −1) = `segBase + A·ms + G·block`(가짜 행, D4-3). WAD 일반·WAD-MP·ANGLE·VISION V3에서는 `iMeasMode`/`bMeasProbe`가 그 헤드를 고르지 않아 소비되지 않는다. VISION 쉬는 헤드 = `−1`.
  - **예외 두 곳(리뷰 B1):** VISION V2(`{-1,c}`, 장치0 마스터)와 WAD W3 bOneCell 헤드1은 `nUseCELLID[0]` 기반 값을 **실제로 쓴다**. N>1은 P38a/P38b로 자기 셀 행이 되고, N=1은 원문(V2는 ms ≥ 1이면 직전 측정 스틱 마지막 셀 행 `A·ms`, ms = 0이면 행 0 — D4-16).
  - 범위 검사용 성질: 세그먼트 k 줄의 −1이 아닌 값은 전부 `[segBase_k + G·b, segBase_k + A_k·S_k + G·b]` 안이다(하한은 ms = 0 가짜 행 = segBase_k + G·b라 **포함**). 사용 헤드(use = 1) 행만은 `[segBase_k + G·b + 1, segBase_k + A_k·S_k + G·b]`.
- Cell ID 토큰
  - 글자 인덱스 `L = letterBase_k + (rot == 1 ? c : S_k − 1 − s)`, 번호 `= (rot == 1) ? s + 1 : c + 1`.
  - `letterBase_k = Σ_{j<k} (rot_j == 1 ? A_j : S_j)`. ICP는 ±90(rot1/3)을 Long으로 보지만 HMI 토큰 규칙은 rot3을 "그 외"(글자 = 스틱)로 쓴다. 이월 축은 **HMI 토큰 규칙**을 따라야 글자가 겹치거나 비지 않는다.
  - 표기: N=1 = `%c`(65 + 인덱스, 26 이상은 기존처럼 비문자). N>1 = bijective base-26(0 A, 25 Z, 26 AA, 40 AO). 셀 −1(rot1)은 N>1에서도 원문 `@`(P21 `nCellNum >= 0`, `CellIdLetterPrefix(<0)` = `@`).
  - B 패널(GlassID 끝에서 두 번째 = `B`): 번호 `+= (rot == 1 ? S_k : A_k)`(자기 세그먼트, 무변경 의미).
  - 전체 ID = `GlassID[0..len−2] + 토큰 + (Σ문자코드 % 10)`. 31자 이하.

*`9_OCP_TEST_Hybrid_Half_260930` (G 35) — 행·토큰(A 패널 GlassID `ABCDEFG1234`, B 패널 예시 GlassID `ABCDEFG12B5`)*

| seg | s(측정 순서) | ms | Phase 4 행 | 지금 행 | Phase 4 토큰 | 지금 토큰 | A 패널 전체 ID | B 패널 전체 ID |
|---|---|---|---|---|---|---|---|---|
| 1 rot1, base 0 | s0 | 0 | 1,2,3 | 1,2,3 | A01 B01 C01 | 같음 | `ABCDEFG123A018 B019 C010` | `ABCDEFG12BA068 B069 C060` |
| 〃 | s1 | 1 | 4,5,6 | 같음 | A02 B02 C02 | 같음 | `…A029 B020 C021` | `…A079 B070 C071` |
| 〃 | s2 | 2 | 7,8,9 | 같음 | A03 B03 C03 | 같음 | `…A030 B031 C032` | `…A080 B081 C082` |
| 〃 | s3 | 3 | 10,11,12 | 같음 | A04 B04 C04 | 같음 | `…A041 B042 C043` | `…A091 B092 C093` |
| 〃 | s4 | 4 | 13,14,15 | 같음 | A05 B05 C05 | 같음 | `…A052 B053 C054` | `…A103 B104 C105` |
| 2 rot2, rowbase 15, letterbase 3 | s0 | 4 | **32,33,34,35** | 17-20 | **H01-H04** | E01-E04 | `…H015 H026 H037 H048` | `…H054 H065 H076 H087` |
| 〃 | s1 | 3 | **28-31** | 13-16 | **G01-G04** | D01-D04 | `…G014 G025 G036 G047` | `…G053 G064 G075 G086` |
| 〃 | s2 | 2 | **24-27** | 9-12 | **F01-F04** | C01-C04 | `…F013 F024 F035 F046` | `…F052 F063 F074 F085` |
| 〃 | s3 | 1 | **20-23** | 5-8 | **E01-E04** | B01-B04 | `…E012 E023 E034 E045` | `…E051 E062 E073 E084` |
| 〃 | s4 | 0 | **16-19** | 1-4 | **D01-D04** | A01-A04 | `…D011 D022 D033 D044` | `…D050 D061 D072 D083` |

- 글자 범위: seg1 A..C(extent 3 = 셀 수), seg2 D..H(extent 5 = 스틱 수). T0 서로 다른 ID 23 → **35**, 세그먼트 간 겹침 12 → **0**.
- 주의: Phase 4의 seg2 s4 `D011 D022 D033 D044`는 15:54 런에서 seg2 s1이 보낸 문자열과 같다(지금 규칙의 D = 뒤집은 인덱스 3). 셀이 바뀐 것이지 중복이 아니다 — 비교는 (seg, s, c) 단위로 할 것.
- 15:54 조건 재실행 시 장치별 기대 SEND(A 패널):
  - WAD T0 seg1(헤드1): `A018 B019 C010 / A030 B031 C032 / A052 B053 C054`(무변경). seg2 s1: `G014 G025`(헤드1, c0 c1) `G036 G047`(헤드0, c2 c3), s3: `E012 E023 E034 E045`.
  - 화질2(헤드1): seg1 `A029 B020 C021 / A041 B042 C043`(무변경), seg2 s0 `H015 H026`, s2 `F013 F024`, s4 `D011 D022`. 화질1(헤드0): seg2 s0 `H037 H048`, s2 `F035 F046`, s4 `D033 D044`.
  - 시야각2(헤드1, T1): seg1 `A018 B019 C010 A029 B020 C021`(무변경), seg2 `H015 G014 F013 E012 D011`. 시야각1(헤드0): seg2 `H048 G047 F046 E045 D044`.
  - PG Ensis 필드: seg1 `010..050`, seg2 `051 041 031 021 011` **무변경**(D4-11).
  - WAD 프레임 길이 136 무변경(MCRID 32자 공백 패딩, `CommTcpIpObjectWAD.cpp:808-818`).

*15:54 조건 재실행 — 기대 `AgingCycle row` 값(seg1은 15:54와 바이트 동일, seg2는 −1이 아닌 값 + 15)*

| pass / seg2 스틱 | 기대 row(a,b) |
|---|---|
| T0 s0 VISION | `(-1,32) (-1,33) (34,-1) (35,-1)` |
| T0 s1 WAD | `(27,28) (27,29) (30,27) (31,27)` (27 = 가짜 행 12+15) |
| T0 s2 VISION | `(-1,24) (-1,25) (26,-1) (27,-1)` |
| T0 s3 WAD | `(19,20) (19,21) (22,19) (23,19)` |
| T0 s4 VISION | `(-1,16) (-1,17) (18,-1) (19,-1)` |
| T1 s0..s4 ANGLE | `(31,32)(35,31) / (27,28)(31,27) / (23,24)(27,23) / (19,20)(23,19) / (15,16)(19,15)` |

- 꼬리 태그는 그대로 ` seg(2/2) rowbase(15)`. 이 런의 `AgingCycle row` 51줄 = L5 51줄(셀 때 정규식 `\[HYBRID\] result row (WAD|WAD-MP|VISION|ANGLE) `, L4는 `[HYBRID] rows per segment:`라 안 걸린다).
- 예: `[HYBRID] result row WAD stick(1) cell(-1,0) use(0,1) row(27,28) id(,ABCDEFG123G014) seg(2/2) rowbase(15)`, `[HYBRID] result row VISION stick(0) cell(2,-1) use(1,0) row(34,-1) id(ABCDEFG123H037,) seg(2/2) rowbase(15)`.

*EFA T0/T1*
- 같은 (seg, s, c, block)은 같은 행이라 **T1이 T0를 덮는 기존 규칙**이 세그먼트 단위로 정확해진다(3c (12)-1의 교차 혼입 소멸). T1 시작 N−1→0 경계에서 `SegmentSwitchData(0)` → Sync가 rowbase·letterbase를 0으로 되돌린다.
- T0 플러시 파일 = 전 세그먼트 T0 행 1..35. 최종 파일(실기 기대): WAD {1-3, 7-9, 13-15, 20-23, 28-31} = T0 값, VISION {4-6, 10-12, 16-19, 24-27, 32-35} = T0 값, ANGLE {1-6} ∪ {16,19,20,23,24,27,28,31,32,35} = T1 값. WAD+VISION이 35셀을 정확히 한 번씩 덮는다.

*AgingTest 반복(G × block)*
- Half + AgingRcp WAD 항목 2개(K_wad = 2): rep0 = 1..35, rep1 = 36..70(seg1 36..50, **seg2 51..70**). 지금은 seg2 rep1이 36..55라 seg1 rep1과 겹친다.
- N=1 Long rep1 = 101..200(무변경).

*WAD Multi Point 블록*
- 11:53 조건(`WAD points 3`, per segment 3/2, 사이클 비활성): seg1 idx1 = 36..50, idx2 = 71..85(로그 `row(36)`, `row(71)` 무변경), seg2 idx1 = **51..70**(지금 36..55, 로그 `11:57:07 ... stick(0) cell(0) row(52)` → Phase 4 `row(67)`). 캐파 G 35 × 3 = 105.
- 사이클 활성: `block = K_w + rep·(P−1) + ip−1`, 행 = segBase + c + A·ms + 1 + G·block.

*VISION 두 헤드*
- V2(`nMeasArea == 1`, 스틱당 2헤드 그룹 1개): `row(segBase + c_m + A·ms + 1 + G·rep, -1)`, 두 헤드 모두 ±HeadGap/2에서 grab, 헤드1은 행 없음. c_m = 마스터 셀: N>1은 P38b로 그룹의 실제 셀(`{-1,c}`이면 c), N=1은 원문 `nUseCELLID[0]`(−1일 수 있음). 로그 `cell(c_m,c_m) use(1,0)`.
- V3: c ≥ 0인 헤드마다 자기 행. N=1 16:13 s4 `row(91,81)…row(100,90)` 그대로. 하이브리드 Half는 단일 헤드 그룹뿐이라 `(-1,r)` / `(r,-1)`.

*`1_OCP_TEST_Long_P1RT` (게이트·2글자 검증용, G 205)*
- seg1 rot1 5×20(전기만) → 행 1..100, 글자 A..T. seg2 rot2 21×5 → 행 101..205, 글자 U..AO.
- seg2 s0(ms20): 행 201..205, `ABCDEFG123AO017 AO028 AO039 AO040 AO051`. s1: `AN016 …`. s20(ms0): 행 101..105, `U018 U029 U030 U041 U052`. B 패널 s0: `ABCDEFG12BAO067 … AO102`(번호 + 셀 수 5, 15자).

*`1_OCP_TEST_Long_260907` (N=1)* — segBase 0, letterBase −1(legacy). 행·ID 전부 (0)의 기준선과 같아야 한다.

---

**(4) API (CRecipeEditMgr, 4-0)**

| API | 의미 | 읽는 것 | 호출 스레드 |
|---|---|---|---|
| `int GetResultRowBase() const` | 활성 세그먼트 segBase. N≤1 = 0 | `m_nSegCountCache`, `m_nSegRowBaseCache`, 벡터 크기 | 측정 스레드(스텝) |
| `int GetCellIdLetterBase() const` | 앞 세그먼트 글자 수 합. N≤1 = **−1**(legacy 신호) | `m_nSegLetterBaseCache`, 같은 가드 | 측정 스레드(MakeCellID/GenCellID) |
| `static CString CellIdLetterPrefix(int)` | bijective base-26, 음수 = `@` | 없음(순수) | 아무 스레드 |
| `static int SegCellIdLetterExtent(C_OutRecipeFunc*, int)` | 세그먼트 글자 수(rot1 → 셀, 그 외 → 스틱) | **세그먼트 벡터** | Sync(Recipe Change·경계 = 시퀀스 스레드), Recipe 아이템, UI `GenGlassPath`, 측정 시작 로그(측정 스레드)만 |
| `void SetResultSaveLatch(int)` / `int GetResultSaveLatch() const` | 글라스 게이트 래치. 미래치·N≤1 = −1 | int + 가드 | Set = Recipe 아이템, Get = Result/Unload 아이템 |
| 기존 `GetSegLogTag(BOOL)` | ` seg(k/N)` (+ ` rowbase(b)`) | int 캐시 + 벡터 크기 | 장치 수신/대기 스레드에서도 가능(3c) |
| (4-7) `BOOL OrSegmentMeasMode(bool*, CString*)` | 모든 세그먼트 검사 플래그 OR → `bMeasMode`. N≤1 = FALSE·무접촉 | **세그먼트 벡터** | Recipe 아이템·PreMeas 아이템(시퀀스 스레드) |
| (4-7) `bool TakeVisionRcpWait()` | VISION 레시피 완료 대기 글라스당 1회. N≤1 = 항상 true | int 캐시 + 벡터 크기 + `m_nVisionRcpWaitLatch` | 측정 스레드 |

- **스레드 규칙:** 세그먼트 벡터(`GetSegmentMgr/Rotate/CellCount/StickCount`)는 시퀀스 스레드(Recipe 아이템, 경계 `SegmentSwitchData`, 측정 시작 블록) 또는 UI `GenGlassPath`(세그먼트 0 활성)에서만 읽는다. 장치 스레드·Result/Unload는 int 캐시와 벡터 크기만 읽는다.
- 캐시 갱신 지점은 3c와 같은 두 곳뿐이다(`RecipeChange` :138, `SegmentSwitchData` :569). 새 경로가 `SetActiveSegment`를 직접 부르면 캐시가 낡는다 — Phase 5에 넘길 주의점((12)).
- Phase 4b 대비: 토큰 생성은 `MakeCellID` 두 줄과 `BuildHybridCellMap` 한 곳에만 있다. 4b 리졸버는 DLL 편집기에서도 써야 하므로 그때 `CellIdLetterPrefix`를 공용 헤더(DLL·ALPHA 두 사본)로 옮긴다.

---

**(5) 로그 사양** (N=1은 모두 기존 문자열 그대로. 아래는 15:54 조건 재실행 시 N>1 기대 출력)

| # | 파일(로거) · 레벨 | 시점 | 형식 / 예 |
|---|---|---|---|
| L1 | GLASS 2_RECIPE_CHANGE · OPER | Recipe Change N>1 블록 | `[HYBRID] result save gate: seg1 [LONG_H1] optical 1 aging2 1, seg2 [SHORT_H2] optical 1 aging2 1 -> save`. P1RT: `... seg1 [Segment#1] optical 0 aging2 0, seg2 [Segment#2] optical 1 aging2 0 -> save (segment 1 alone: bypass)`. 전기만: `-> bypass (electrical only)` |
| L2 | 〃 · OPER | 〃 | `[HYBRID] APD header CELL_X_CNT 35 CELL_Y_CNT 1 (glass rows G x 1; segment 1 alone 3 x 5)` |
| L3 | 〃 · OPER(31 초과면 WARN) | 〃 | `[HYBRID] cell ID letters A..H over 2 segments, longest ID 14/31 chars (GlassID length 11)`. P1RT: `A..AO ... 15/31` |
| L4 | GLASS 6_MEASURE · OPER (3c WARN 대체) | 측정 시작 | `[HYBRID] rows per segment: seg1 rows 1..15 letters A..C seg2 rows 16..35 letters D..H (G 35)` (접두를 L5와 갈라 놓았다, 리뷰 B4) |
| L5 | GLASS 6_MEASURE · SYST | 실가동 `SetMeasUseInfo`마다(WAD/WAD-MP/VISION/ANGLE) | `[HYBRID] result row %s stick(%d) cell(%d,%d) use(%d,%d) row(%d,%d) id(%s,%s) seg(k/N) rowbase(b)`. use = 그 헤드 결과가 행에 기록됨(WAD/ANGLE `bMeasProbe`, VISION `bMeasProbe && row >= 0`). 셀 때 정규식 `\[HYBRID\] result row (WAD\|WAD-MP\|VISION\|ANGLE) ` |
| L6 | GLASS 6_MEASURE · OPER (3c 꼬리 대체) | T0 플러시 | `EFA: T0 result flush OK - hybrid: T0 rows of all 2 segments (G 35)` |
| L7 | GLASS 7_RESULT · OPER | CompleteCheck | `[HYBRID] Result: glass save gate 1 - SaveResult` / `[HYBRID] Result: glass save gate 0 (electrical only) - no result file` |
| L8 | GLASS 8_UNLOAD · OPER | CheckSaveReuslt | `[HYBRID] Unload: glass save gate 1 - wait SaveResultDone` / `... gate 0 (electrical only) - CIM bypass (DATAJUDGE_NOTMEAS)` |
| L9 | Control\WAD · OPER (실기 전용) | Toss | `[WAD] : TossResultFile Updata, 2155830 row(28) cell(ABCDEFG123G014) seg(2/2) rowbase(15)` (기존 줄에 꼬리) |
| L10 | Control\시야각n, 화질n · OPER (실기 전용) | Toss | `[<장치명>] : TossResultFile Updata, <InnerID> row(32) cell(ABCDEFG123H015) seg(2/2) rowbase(15)` (새 줄). VISION 태그 = grab 시점 |
| L11 | GLASS 6_MEASURE · WARN (드묾) | GenCellID | `[HYBRID] cell ID <id> longer than 31 - cut to 31` |
| L12 | GLASS 2_RECIPE_CHANGE · WARN (조건부, 리뷰 A2) | Recipe Change N>1 블록 | `[HYBRID] APD merge keeps rows 1..15 of 35 (top sheet ANGLE ends at row 15, dResult D4-14) - later rows miss the merged sheet / APD` (예: Half 변형 seg1만 Angle). 15:54 Half·P1RT는 상단 시트가 G까지 가므로 **줄 없음** |
| L13 | GLASS 2_RECIPE_CHANGE · OPER (4-7만) | Recipe Change | `[HYBRID] Recipe Change device recipe open (all segments): PG 1 AGING 0 WAD 1 VISION 1` (P1RT). Half: `PG 1 AGING 1 WAD 1 VISION 1`(seg0과 같음) |
| L14 | GLASS 4_PRE_MEAS · OPER (4-7만) | RcpChangeDone | `[HYBRID] PreMeas recipe done wait (all segments): PG 1 AGING 0 WAD 1 VISION 1` |

- 기존 N>1 줄의 **값**이 바뀌는 것: `AgingCycle row ... row(a,b)`, `WAD MultiPoint: ... row(%d)` (segBase 포함). 형식·꼬리 태그는 같다.
- 사라지는 N>1 줄: 3c `WARN [HYBRID] result rows are segment-local until Phase 4 ...`, 꼬리 ` - hybrid: rows of the segments overlap until Phase 4 (segBase)`.
- Phase 4 뒤 `[HYBRID] result row` 접두는 L5만 쓴다(L1 `result save gate`, L4 `rows per segment`, L7 `Result:`는 안 걸림).
- 태그 규칙은 3c D3c-3(줄 끝 ` seg(k/N)`, 행 줄은 ` rowbase(b)`)을 따른다.

---

**(6) N=1 불변 근거 (구조적)**

1. 캐시: Sync는 `nSegCnt > 1`일 때만 `nActSeg`를 읽는다. N≤1이면 `nActSeg = 0` → rowbase·letterbase 루프 0회 → 0. 게터 3개는 `m_nSegCountCache <= 1`이나 벡터 크기 ≤ 1이면 0 / −1 / −1을 돌려준다(낡은 캐시도 차단).
2. 행: 실가동 7분기 모두 `nAgingRow += 0` / `nAreaNumM[h] += 0`. 계산식·`SetMeasUseInfo` 인자·`LogAgingRow`·`WAD MultiPoint` 출력이 같다.
3. Cell ID: `nLetterBase = −1` → 원문 `%c` 줄이 else 몸체로 실행된다(원문 줄 바이트 무변경, `ins_before`). `bHybridCid = false` → 원문 B 파서, 길이 가드 미진입.
4. 게이트: 래치는 N>1 블록에서만 0/1이 되고 `RecipeChange`가 매 글라스 −1로 되돌린다. 게터 가드까지 있어 N=1은 항상 −1 → 원문 블록.
5. APD 헤더: `stGlassInfo` 덮어쓰기는 `GetSegmentCount() > 1` 블록 안이다. `:563-564` 원문 대입은 그대로.
6. 로그: 새 줄은 모두 `GetSegLogTag()` 비어 있지 않음, `nSegCount > 1`, `nBoundarySeg > 1`, N>1 블록, 래치 ≥ 0 중 하나 뒤다. 바뀌는 기존 줄(3c WARN·EFA 꼬리)은 원래 N>1 전용이다.
7. 장치 로그: N=1 태그 "" → WAD 줄 그대로, ANGLE/VISION 새 줄 없음. VISION 슬롯 `m_strSegTag`는 "" 대입뿐.
8. 운전 화면: `BuildHybridCellMap`은 `:3908`에서 N≤1이면 이름 블록 전에 반환한다.
9. 구조체: 공유메모리(`ShareMem.h`), DLL 헤더(`OutRecipeFunc.h`/`preDefine.h`/`SegmentDefine.h`), dResult 계약(`API_Result.h`) 무변경. 바뀌는 구조는 ALPHA 내부 클래스 `CRecipeEditMgr` 끝 멤버 2개와 `S_QualityWait` 멤버 1개뿐(전체 재빌드로 해결).
10. 동작 순서: 모션·대기·시퀀스 분기 변경 0(4-0~4-5). 추가 비용은 N>1의 문자열 포맷뿐이다. 4-7(승인 시)은 N>1에서만 장치 레시피 명령·VISION 대기 횟수를 바꾼다.
11. 리뷰 반영 코드(P38a/P38b)는 `GetCellIdLetterBase() >= 0`(N≤1이면 −1) 뒤에만 있다. P21의 `nCellNum >= 0`은 `nLetterBase >= 0`과 AND라 N=1은 여전히 원문 줄. `CellIdLetterPrefix`의 음수 처리 변경은 N>1 경로(P21/P22/P36/L3/L4)에서만 불린다. P16 블록은 N=1에서 배열 두 개를 채울 뿐 `LogHybridRow`가 즉시 반환한다.
12. L12(병합 추정)는 `LatchHybridResultItems` 안(N>1 블록)이다. P38c는 D4-17 (a)를 고르면 N=1의 **정의된** 동작은 같고 경계 밖 쓰기만 사라진다(바이트 동일 원칙의 유일한 예외 후보 — 사용자 결정), (b)면 바이트 동일.
13. 4-7: `OrSegmentMeasMode`는 N≤1이면 FALSE를 돌려 `bMeasMode`를 건드리지 않고 로그도 없다. `TakeVisionRcpWait`는 N≤1이면 항상 true라 `if (iStick == 0 && i == 0 && ...)`가 원문과 같다.

---

**(7) 결정** (권장안 **굵게**. 플랜 문서에서 이미 확정된 것 — 저장 게이트 정책(2026-09-29), D-5 자동 이월 먼저, base-26 표기, D3c-14 업로드 로그 Phase 4 — 은 다시 열지 않는다)

| # | 항목 | 선택지 | 권장 |
|---|---|---|---|
| D4-1 | segBase 가산 위치 | 스텝(`SetMeasUseInfo` 전, `nAgingRow`/`nAreaNumM`에 더함) / Factory `SetMeasUseInfo` 안 | **스텝.** 로그 = 장치 값, VISION grab 캡처 전, 수동 측정(공유 깔때기) 무영향. Factory는 0과 가짜 행을 구분 못 한다 |
| D4-2 | segBase 출처 | int 캐시 `m_nSegRowBaseCache`(+가드) / 측정 스레드에서 매번 벡터 합 | **int 캐시.** 3c가 이미 같은 값을 `rowbase(b)`로 찍어 15:54 런으로 검증됐다(seg2 = 15) |
| D4-3 | 쉬는 헤드 가짜 행(WAD/ANGLE c = −1) | 가산 안 함(`A·ms + G·b`) / **같은 식으로 가산** / N>1에서 −1 | **가산 + 소비되는 두 분기는 마스터 셀로 정정(P38a/P38b).** 식 하나로 끝나고(`nAgingRow` 경유) 세그먼트 k 줄의 값이 전부 그 세그먼트 범위에 든다(하한 포함). 리뷰 B1: "소비되지 않는다"는 WAD 일반·WAD-MP·ANGLE·VISION V3에서만 참이고, VISION V2 `{-1,c}`(장치0 마스터 grab)와 W3 bOneCell 헤드1은 이 값을 실제로 기록한다 — segBase를 더하면 앞 세그먼트 마지막 셀(segBase_k)을 덮으므로 N>1에서 그 두 분기를 자기 셀 행으로 바꾼다. −1 도입은 WAD/ANGLE 경로 검증 부담이라 여전히 안 한다 |
| D4-4 | APD `CELL_X_CNT/CELL_Y_CNT`(N>1) | seg0 값(3×5, ICP와 비슷) / **G × 1** / 최대 셀 × 총 스틱(4×10) / 세그먼트별 헤더 블록(dResult 수정) | **G × 1.** N=1에서 X·Y = A·S = 반복 블록 크기이고 MES 문서 §3이 블록 = "글라스 전체 셀 수"라 정의한다. G × 1만 이 성질(X·Y = G)을 지킨다. 4×10은 40 ≠ 35. **ONLINE 운용 전 MES 확인 필수**(바꿀 곳은 `LatchHybridResultItems` 2줄). **주(리뷰 B3):** dResult `m_nCellCountX/Y`는 글라스마다 지워지지 않고 WAD 업로드에서만 갱신된다 → 하이브리드 글라스 다음에 WAD 결과 없는 글라스(N=1 VISION/ANGLE만, 또는 WAD 끈 하이브리드)는 APD에 `35 / 1`이 남는다(Phase 4 전에는 seg0 `3 / 5`가 남았다 — 다른 레시피 간 잔존은 오늘도 있다). 고치려면 `CDlgResult::DataClear`에서 0으로 — dResult(UTF-8 BOM) 수정이고 N=1 비-WAD 글라스의 헤더도 바뀌므로 **별도 승인 항목**. Phase 4는 기록만 |
| D4-5 | Unload AgingTest_2 추가의 N=1 적용 | **N>1만**(래치) / N=1도 | **N>1만.** N=1에 넣으면 AgingTest_2-only 레시피의 bypass → 저장 대기, NOTMEAS → OK로 바뀐다(HARD RULE 위반). 이 PC 레시피 13개 중 해당 0개. N=1 정리는 사용자 승인 + 실기 레시피 조사 후 별건 |
| D4-6 | 게이트 판정 방식 | Result/Unload가 각자 계산 / **글라스 시작 래치** | **래치**(3c A5 패턴). Result·Unload가 같은 값, 벡터는 Recipe 아이템만 읽음, 측정 전에 판정 로그(09-29 17:26에 이 줄이 있었다면 13분 측정 전에 알았다). CIM PPID 재로드가 끼면 −1 → 오늘 동작으로 안전하게 후퇴 |
| D4-7 | AgingCycle/EFA rcp 항목 광학을 게이트에 반영(gate Q3) | **레시피 플래그만(N=1과 같음)** / 하이브리드만 사이클 항목 반영 | **레시피 플래그만.** 확정 정책 문장과 같고 N=1과 의미가 갈라지지 않는다. EFA T0 플러시는 게이트 없이 저장(기존) |
| D4-8 | B 패널 새 파서 적용 범위 | **N>1이면 항상** / 접두 길이 > 1일 때만 | **N>1이면 항상.** 판정이 `bHybridCid` 하나로 단순. N=1은 원문(번호 ≥100 절단까지) 보존 |
| D4-9 | 31자 초과 | **N>1 런타임 31자 절단 + WARN, 시작 시 최장 길이 로그** / 시작 거부 | **절단 + WARN.** `_tcscpy_s` abort(프로그램 종료)만 막으면 된다. 실 GlassID(11자)면 최장 15자로 여유 큼 |
| D4-10 | 화면 Cell ID 정합 | 런타임만 / **런타임 + 운전 화면 셀맵(ALPHA N>1 경로)** / + DLL 레시피 `m_chCellID` 재기록(.rcp 바이트 변경, DLL 재빌드) | **런타임 + 운전 화면.** DLL 화면(MakeCell 오버레이·레시피 그리드·SelectCell·TestMode)은 Phase 4b 리졸버와 함께. 레시피 파일은 안 바꾼다 |
| D4-11 | PG(Ensis) 프로토콜 스틱 필드 | **로컬 유지(§5.7 그대로)** / rot≠1 세그먼트만 필드에 base 가산 | **로컬 유지.** rot1 세그먼트 k ≥ 1은 프로토콜로 표현할 수 없고, 필드 > 26일 때 PG 표기도 모른다. PG가 만든 ID의 용도(로그/결과/MES)를 벤더에 질의(기록). B 패널 rot≠1 불일치는 N=1에도 이미 있다(`:1732` 주석) |
| D4-12 | 측정 스레드 행 로그 | **스텝 헬퍼 `LogHybridRow`, N>1에서 `SetMeasUseInfo`마다 항상** / AgingCycle 비활성일 때만 / Factory 안 | **항상.** 셀·사용 헤드·ID를 가진 유일한 줄이고, 비-Aging 하이브리드 런(P1RT 등)에는 다른 행 로그가 없다. AgingCycle 런은 `AgingCycle row`와 함께 2줄 |
| D4-13 | 업로드 로그 스냅샷 | **WAD/ANGLE = Toss 시점 `GetSegLogTag`, VISION = grab 시점 슬롯 캡처** / Factory에 스냅샷 int 저장 | **앞쪽.** WAD/ANGLE은 측정 스레드가 완료를 기다리는 동안 업로드되므로 세그먼트가 바뀔 수 없다. VISION만 비동기(최대 150 s)라 슬롯에 보관. Factory·헤더 변경 불필요 |
| D4-14 | dResult 병합 행 수 결함(`CDlgResult.cpp:1824` `if (GetMaxRows() != 0) nMaxRows = ...` → 시트 1..4(WAD 1 / VISION 2 / ANGLE 3) 중 **번호가 가장 큰 비어 있지 않은 시트**의 행 수, 최댓값 아님) | **Phase 4 밖 별도 승인 항목 + 하이브리드 WARN(L12)** / Phase 4에 포함 | **별도 + L12.** 수정은 N=1 출력이 바뀐다. 16:22 AgingTest(로그 재확인): WAD 행 s0 홀수 1..19, s1 짝수 22..40, s4 81..100 / VISION 최대 30 / ANGLE 없음 → 병합 = VISION 행 수 → WAD **32·34·36·38·40과 81..100**이 병합·APD에서 빠질 것(리뷰 A2로 정정, 실기 미확인). 하이브리드는 segBase 뒤 상단 시트가 끝나는 행 위가 **세그먼트째** 빠진다(Half 변형 seg1만 Angle → seg2 16..35 전부; Phase 4 전에는 1..15에 섞여 일부만 보였다). OOS는 APD를 못 보므로 Recipe Change에서 L12 WARN으로 측정 전에 알린다(레시피 플래그 기준 추정, N=1 무로그). **하이브리드 ONLINE 전에 결정**(DLL UTF-8 BOM, `dResult` 재빌드) |
| D4-15 | 운용 제한 해제 | D3d-8(하이브리드 + WAD Multi k ≥ 1 금지) / D3c-7(하이브리드 결과 WARN + 운용 제한) | **D3d-8은 Phase 4 OOS H5 통과 시 해제. D3c-7은 유지** — 실기 APD(AREA_CNT 35, 혼입 0) + MES의 CELL_X/Y 수용 + D4-14 결정 + **D4-18(4-7) 적용 또는 seg0이 모든 장치를 쓰는 레시피만 운용** 뒤 해제 |
| D4-16 | WAD bOneCell 헤드1 행 결함(W3 `nAreaNum[1] = nUseCELLID[0]…`) + VISION V2 `{-1,c}` 마스터 결함(행·ID·`nCellStatus[-1]`) | 그대로(별건) / **N>1만 고침, N=1 그대로** / 전부 고침 | **N>1만(P38a/P38b).** 리뷰 B1로 segBase가 이 결함을 세그먼트 간 덮어쓰기로 키운다는 것이 확인돼 하이브리드 경로에서는 고친다. N=1 결함(V2: 장치0 결과가 직전 측정 스틱 마지막 셀 행 또는 행 0에, ID `@01`/`X00`, `:3910` 공유메모리 `nCellStatus[-1]` 쓰기 / W3: 짝이면 c1이 c0 행을 덮음)은 N=1 동작 변경이라 별도 승인 항목으로 기록. 1셀/스틱 N=1 레시피에서 VISION 셀이 헤드2로 배정되면(`PickSingleCellHead` 동률 → 헤드2) 실제로 걸린다 |
| D4-17 | `:2597` `if (bOneCell) memset(m_pStInnerID[nUseCELLID[0]], ...)` — `{-1,c}`면 `new TCHAR*[n]` 앞 힙 헤더를 포인터로 읽어 8바이트 0 쓰기(UB, N=1에도 있음) | **(a) 모든 N에 `nUseCELLID[0] >= 0` 가드** / (b) N>1만 가드 / (c) 그대로 | **(a), 사용자 승인 필요.** 정의된 동작(유효 첨자)은 바이트까지 같고 UB 경로만 없어진다. 단 N=1 소스 경로를 건드리므로 HARD RULE상 사용자가 정한다. 승인 없으면 (b)로 넣는다(N>1 H8에서 크래시를 막는 최소안). 헤드2 셀의 InnerID를 지우지 않아도 무해 — 3점 측정용 초기화였고 `nOneCellCnt = 1`로 폐기됨, 에이징 반복은 `ClearInnerIDForAging`이 따로 처리 |
| D4-18 | 4-7 하이브리드 장치 레시피(리뷰 B2: Recipe Change/PreMeas가 seg0 플래그만, VISION 대기 세그먼트마다 재대기) | **(a) Phase 4에 4-7로 포함(N>1만, 독립 빌드, 마지막)** / (b) 별도 항목(Phase 3e)으로 실기 전 / (c) 운용 지침만(seg0에 모든 장치 항목) | **(a), 사용자 선택 필요.** Phase 4 게이트 통일의 대상(seg0 전기만, P1RT)이 실기에서는 WAD 레시피 미수신 + seg2 VISION `HEAD_ACK_TIMEOUT`으로 끝나지 못한다. N=1 무변경이고 약 40줄이지만, Phase 4 원래 범위(행·ID·게이트) 밖의 시퀀스/장치 명령 변경이고 (6)-10 "시퀀스 변경 0"을 깨므로 사용자가 정한다. OOS로는 검증 불가(로그 L13/L14만) |

---

**(8) 검증 (OOS)** — 공통: GlassID는 사이클 화면에 직접 입력한다(빈 칸이면 시각 기반 B 패널 ID가 생겨 비교가 안 된다). 비교 스크립트는 `scratchpad/n1ext.pl` 방식(시각 제거 + 휘발 값 정규화)을 확장한다.

0. **오프라인(빌드 전)**
   - `scratchpad/p4plan/expect.pl`로 (3) 표 재생성. `CellIdLetterPrefix` 경계값 0 A / 25 Z / 26 AA / 40 AO / 51 AZ / 52 BA / 701 ZZ / 702 AAA.
   - 앵커 재검사(`chk.pl anchors.txt`, `anchors2.txt`)가 전부 n=1.
1. **N=1 회귀(필수, 첫 번째)**
   - R1: `1_OCP_TEST_Long_260907` + EFA 2패스, GlassID `ABCDEFG1234`(기준 16:13 런). R2: 같은 레시피 비-EFA AgingTest(기준 16:22 런).
   - 비교 대상: `GLASS 2_RECIPE_CHANGE / 4_PRE_MEAS / 6_MEASURE / 7_RESULT / 8_UNLOAD`, `measure_seq_item_engine`, `Control\WAD`, `화질1/2`, `시야각1/2`, `PG`(각 런 시간 구간만).
   - 정규화: 줄 머리 `[2026-..-.. hh:mm:ss:mmm] - ` 제거, `InnerID duplicate remapped: N -> N`, `\d{8}-\d{6}`, `(x.x s)`, 장치 로그의 InnerID 7자리(`\b[12]\d{6}\b`, WAD `5130[12] ?` 뒤 / `GRAB.START.` 뒤).
   - 기대: **diff 0줄.** `[HYBRID]` 0건, ` seg(` 0건, `result row` 0건. WAD 프레임 `(136)`, 토큰·체크섬 `A018 B019 C010 … T017` 동일. R1 `AgingCycle row` 90줄, R2 40줄.
   - (4-7 적용 시) `device recipe open` / `recipe done wait` 0건, 화질 `WaitRecipeChangeEnd` 줄 수 기준선과 동일.
   - APD/XLS: 새 파일이 생기고(`_HHMMSS.xls` + `APD_*.txt`), APD는 `START_TIME/END_TIME`만 다르다(597 B).
2. **H1 하이브리드 EFA 재실행(15:54 조건 그대로)**
   - L1 `... -> save`, L2 `CELL_X_CNT 35 CELL_Y_CNT 1 (... 3 x 5)`, L3 `A..H ... 14/31`, 캐파 줄 15:54와 동일.
   - L4 `[HYBRID] rows per segment: seg1 rows 1..15 letters A..C seg2 rows 16..35 letters D..H (G 35)`. `until Phase 4` 문자열 0건. L12 `APD merge` 0건(모든 세그먼트 Angle).
   - `AgingCycle row` 51줄: seg1 15:54와 동일, seg2 = (3)의 표. L5(정규식 `\[HYBRID\] result row (WAD|WAD-MP|VISION|ANGLE) `) 51줄(값 동일 + 셀·use·id). 부분 문자열 `result row`로 세지 말 것.
   - 범위 검사: seg2 줄의 −1이 아닌 값 ⊂ [15, 35], seg1 줄 ⊂ [0, 15]. 사용 헤드(use = 1) 행 집합: seg1 = 1..15, seg2 = 16..35, 교집합 0.
   - (4-7 적용 시) L13 `PG 1 AGING 1 WAD 1 VISION 1`, L14 같은 값. 장치 SEND 줄 수는 15:54와 같다(seg0이 이미 모든 장치를 씀).
   - 장치 SEND: (3)의 기대 ID. T0 서로 다른 ID 35개. PG 필드 무변경.
   - L6 `EFA: T0 result flush OK - hybrid: T0 rows of all 2 segments (G 35)`, L7 gate 1, L8 gate 1. 결과 파일 2개(T0 + 최종). 경계·측정 시간·측정 셀 수는 15:54와 같다(동작 무변경).
3. **H2 B 패널:** H1을 GlassID `ABCDEFG12B5`로 → (3) 표 B열(`ABCDEFG12BA068 …`, seg2 s0 `…H054 H065 H076 H087`). `cell ID ... cut` 0건.
4. **H3 게이트 + 2글자: `1_OCP_TEST_Long_P1RT` 재실행**(09-29 17:26 조건)
   - L1 `-> save (segment 1 alone: bypass)`. L2 `CELL_X_CNT 205 ... (segment 1 alone 20 x 5)`. L3 `A..AO ... 15/31`. L4 `seg1 rows 1..100 letters A..T seg2 rows 101..205 letters U..AO (G 205)`.
   - seg1은 광학 스텝이 건너뛰어져 L5 0줄. seg2 줄: **−1이 아닌 값 ⊂ [100, 205], use = 1 행 ⊂ [101, 205]**(리뷰 A1: ms = 0 스틱 s20의 쉬는 헤드 가짜 행 = segBase 100 — WAD 단독 c2(X = 0.0, plus 목록)와 모든 ANGLE 단독 그룹, `row(100,103)` 꼴). seg2 첫 스틱 ID `ABCDEFG123AO017 …`(WAD/화질/시야각 SEND), 마지막 스틱 `U018 …`.
   - L7 gate 1, L8 gate 1, **`RESULT\DATA\<dd>\`에 파일 생성**(09-29는 없었다). L12 0건(seg2 Angle이 205까지).
   - (4-7 적용 시) L13 `PG 1 AGING 0 WAD 1 VISION 1`. OOS라 장치 대기는 통과 — 실기 판정은 9번.
5. **H4 전기만:** Half 사본(예 `9_OCP_TEST_Hybrid_Half_ELEC`)에서 두 세그먼트의 AgingTest_2/Spec/Quality/Color/Angle/EFA를 0으로.
   - L1 `-> bypass (electrical only)`, L7 `gate 0 ... no result file`, L8 `gate 0 ... CIM bypass`. 새 결과 파일 0개. L5 0줄(L4 `rows per segment`는 1줄 — N>1 측정 시작마다 찍힌다).
6. **H5 WAD Multi Point(11:53 조건, D3d-8 해제 판정)**
   - seg1 `WAD MultiPoint ... idx(1)` 행 36..50, `idx(2)` 71..85(무변경), seg2 `idx(1)` **51..70**(예 `stick(0) cell(0) row(67)`). 캐파 G 35 × block 3.
7. **H6(선택) AgingTest 하이브리드 K_wad = 2:** use = 1 행만 rep1 seg1 36..50, seg2 51..70(리뷰 A1: 쉬는 헤드 가짜 행은 35 + segBase_k + A·ms라 seg1은 35부터, seg2는 50부터 — 범위에서 제외).
8. **H7 운전 화면(4-5):** Half 열기 → 셀맵 seg2 이름 H01..D04(세그먼트 1은 A01..C05). 측정 중 이름 불변.
8b. **H8 단일 셀 그룹(리뷰 B1, P38):** Half + AgingTest(또는 EFA) rcp의 `[MASECELL_SEG2]`로 seg2 한 스틱에 셀 1개만 남긴다 — 헤드2로 가는 셀(X ≥ 0 쪽; `PickHead` SYST 줄로 확인)을 고른다. WAD 그 스틱과 VISION 그 스틱을 각각 한 번씩.
   - WAD(W3 bOneCell): `cell(-1,c) use(0,1) row(f,r)`, r = 15 + c + 4·ms + 1(seg2 범위, seg1 행과 겹치지 않음), f = 가짜 행(소비 안 됨). 크래시 없음(P38c).
   - VISION(V2): `cell(c,c) use(1,0) row(r,-1)`, 화질1/화질2 SEND ID 둘 다 c의 토큰(`@` 아님, 앞 세그먼트 글자 아님).
   - 비교 기준: P38 없이 돌리면 r 자리에 segBase 15(= seg1 마지막 셀)가 찍힌다.
9. **실기 전용(OOS 불가)**
   - APD `AREA_CNT 35`, `CELL_X_CNT 35`, `CELL_Y_CNT 1`, `[RAW_DATA]` 35줄에 셀 혼입 0, WAD 시트 CELLID 열 35개 서로 다름.
   - L9/L10 업로드 줄의 row/cell/seg가 L5와 일치. VISION 늦은 도착분도 grab 시점 seg.
   - ONLINE REMOTE: gate 0 글라스 `DATAJUDGE_NOTMEAS`, gate 1 `DATAJUDGE_OK`, GDTREQ → `RESULT\APD.txt`.
   - D4-14 확인: WAD·VISION·ANGLE 최대 행이 다른 글라스에서 병합 행 수(16:22 조건이면 WAD 32..40·81..100 누락 예상). L12가 뜬 하이브리드 글라스의 APD 행 수 = L12 X.
   - D4-18: 4-7 적용 상태에서 P1RT — 글라스 시작에 WAD `RecipeOpen` SEND, 화질 `WaitRecipeChangeEnd` 1회, `HEAD_ACK_TIMEOUT` 없음. 미적용이면 같은 런이 seg2 첫 VISION에서 멈추는지(결함 재현).
   - D4-4 주: 하이브리드 글라스 다음 비-WAD 글라스의 APD `CELL_X_CNT/CELL_Y_CNT`(35 / 1 잔존 예상).

---

**(9) 수정 파일·인코딩** (2026-10-01 Perl로 BOM/엄밀 디코드/EOL 수/damaged 확인, 전부 damaged=0, 스크립트 `scratchpad/p4plan/enc.pl`)

| 파일 | 단계 | 인코딩 / EOL (crlf=lf) | 비고 |
|---|---|---|---|
| `Source/ALPHA/Recipe/RecipeEditMgr.h` | 4-0 | CP949, BOM 없음, CRLF 159 | `.h` 변경 → 넓게 재컴파일 |
| `Source/ALPHA/Recipe/RecipeEditMgr.cpp` | 4-0 | CP949 CRLF 2448 | |
| `Source/ALPHA/DoWork/DoWorkGlassMeasureStep.h` | 4-1 | CP949 CRLF 296 | |
| `Source/ALPHA/DoWork/DoWorkGlassMeasureStep.cpp` | 4-1, 4-2 | CP949 CRLF 7029 | 비슷한 블록 반복 → 앵커는 줄 전체(탭 수 포함)로 |
| `Source/ALPHA/DoWork/DoWorkGlassMeasure.cpp` | 4-1 | CP949 CRLF 764 | |
| `Source/ALPHA/DoWork/DoWorkGlassRecipeStep.cpp` | 4-3 | CP949 CRLF 1020 | |
| `Source/ALPHA/DoWork/DoWorkGlassResultStep.cpp` | 4-3 | CP949 CRLF 138 | CompleteCheck 구간은 **4칸 공백** 들여쓰기 |
| `Source/ALPHA/DoWork/DoWorkGlassUnloadStep.cpp` | 4-3 | CP949 CRLF 298 | 탭 |
| `Source/ALPHA/CommTcpIpObjectWAD.cpp` | 4-4 | CP949 CRLF 1377 | |
| `Source/ALPHA/CommTcpIpObjectAngle.cpp` | 4-4 | CP949 CRLF 855 | |
| `Source/ALPHA/CommTcpIpObjectVision.h` / `.cpp` | 4-4 | CP949 CRLF 111 / 1676 | `.h` 변경 → Vision 포함 파일 재컴파일 |
| `Source/ALPHA/DlgSubOperate.cpp` | 4-5 | CP949 CRLF 4376 | Phase 6 미빌드 변경 위에 얹는다(HEAD에 포함됨) |
| `Source/ALPHA/DoWork/DoWorkGlassPreMeasStep.cpp` | 4-7(D4-18) | CP949, BOM 없음, CRLF 1203, damaged 0 (리뷰 반영 때 확인) | P41만 |
| (4-7은 위 `RecipeEditMgr.h/.cpp`, `DoWorkGlassRecipeStep.cpp`, `DoWorkGlassMeasureStep.cpp`도 만진다) | 4-7 | — | P39/P40/P42 |
| `docs/HybridGlass_Porting_Plan.md` 등 | 4-6 | UTF-8 | Edit 도구 가능 |

- **무변경(확인):** DLL 전부(`OutRecipeFunc.h` 두 사본, `preDefine.h` 두 사본, `SegmentDefine.h` 두 사본, `dfcRecipe.rc`/`resource.h` = UTF-16LE), `CoreFunction/dResult/CDlgResult.cpp`(UTF-8 BOM), `Manager/CommTcpIpFactoryMgr.*`, `Manager/ResultMgr.cpp`(ASCII CRLF), `CommTcpIpObjectPG.cpp`, `CommTcpIpObjectCIM.cpp`, `DoWorkSequenceCIMStep.cpp`, `Source/ALPHA/ALPHA.rc`(UTF-16LE BOM), `Source/ALPHA/resource.h`(UTF-8 BOM), `ShareMem.h`.
- **패치 방식:** pfw.pl. `rd($f, 'cp949')` → LF 정규화 텍스트에 `rep1/ins_after/ins_before` → `stage` → `commit("p4_bak")`. 새 코드는 ASCII만이라 `E()` 인코딩이 바이트를 바꾸지 않는다. 한글 주석은 넣지 않는다(기존 한글 줄은 앵커로 쓰지 않음).
  - "줄 전체 + `\n`" 앵커는 `ins_after`(P1, P3–P5, P8, P10, P12–P17, P30, P31, P32(a)(b), P33–P35, P37, P38a, P40, P41), 원문 줄을 else 몸체로 남기는 곳(P21, P22, P24, P28, P29)과 함수 앞(P7, P9, P26)·줄 앞(P11, P25)은 `ins_before`, 블록 교체(P2, P6, P18, P19, P20, P36, P38c, P42(`infunc` 안))는 `rep1`. P39는 P1/P3/P4/P5/P7과 같은 앵커에 이어 붙인다(4-0 뒤에 적용하면 앵커가 4-0의 새 줄이 되므로 4-7 스크립트는 4-0 적용 후 앵커를 다시 잡는다).
  - **줄 끝에서 끝나지 않는 앵커는 `ins_after` 금지**(pfw `ins_after` = 앵커 바로 뒤에 붙인다). P20은 `rep1`로 치환 문자열을 끝까지 적는다(리뷰 B5). 함수 안에서만 유일한 앵커는 `infunc(서명, sub { ... })`로 감싼다: P38b·P42 = `bool CDoWorkGlassMeasureStep_Measure_VISION::Run()`(첫 `\n}\n` = :3978, 889줄), P20 대안 = `int CDoWorkGlassMeasureStep::MakeCellID(`.
  - 각 스테이지 뒤 `post`로 새 문자열 개수 확인: 예 `GetResultRowBase()` 호출 = 5(P10, P11×2, P12, P13), `LogHybridRow(` = 5(정의 1 + 호출 4, `.h` 별도), `CellIdLetterPrefix(` in MeasureStep = 2, `GetCellIdLetterBase()` in MeasureStep = 4(P20, P23, P38a, P38b; D4-17 (b)면 P38c 포함 5), 4-7: `TakeVisionRcpWait()` = 1, `OrSegmentMeasMode(` RecipeStep 1 / PreMeasStep 1.
- **EOL 주의(머지 뒤 바뀐 것, 이번 대상 아님):** `DoWork/AgingCycleInfo.h/.cpp`, `Manager/WadMultiMeasMgr.h/.cpp`, `DlgSubManual_MultiMeas.h/.cpp`, `DlgSubManual_AgingNew.h/.cpp`가 이제 **CRLF**다(루트 CLAUDE.md의 LF-only 목록·3c 표가 낡음). `Manager/ResumeInfoMgr.h/.cpp`(Phase 5 대상)와 `DlgTestMode.h/.cpp` 두 사본은 여전히 **LF-only CP949**. CLAUDE.md 갱신은 사용자 결정(루트는 git 밖).
- 전부 `HMI/` git 트리다. 구현 뒤 `git -C HMI status`로 확인하고 커밋 여부는 사용자에게 묻는다.

---

**(10) 리스크**

| 리스크 | 대응 |
|---|---|
| VISION 결과가 다음 세그먼트·T0 플러시 뒤에 도착 | 행은 grab 시점에 캡처(`m_nSheetRow`)되어 segBase가 이미 들어 있다. 로그 태그도 슬롯에 보관(P33). 플러시가 늦은 VISION을 빠뜨리는 문제는 기존(N=1 동일) |
| dResult 병합 행 수 결함(D4-14) — segBase로 행이 퍼지면 상단 시트 끝 행 위가 세그먼트째 APD에서 빠짐 | 하이브리드 ONLINE 전 결정. 그때까지 D3c-7 운용 제한 유지(D4-15). Recipe Change L12 WARN이 레시피 플래그로 미리 알린다(마스크·샘플링·AgingCycle 블록은 못 셈) |
| MES가 `CELL_X_CNT 35 / CELL_Y_CNT 1`을 거부·오해 | ONLINE 전 MES 확인. 대안(a)/(c)는 `LatchHybridResultItems` 2줄 교체로 끝 |
| Resume 재적재(RS1)가 하이브리드 seg ≥ 1 파일을 seg0 로컬 행에 넣음(segBase 뒤 불일치가 더 뚜렷) | D3c-11 운용 지침 "No(새로)" 유지, Phase 5 저널. N=1은 무변경 |
| Recipe 아이템과 Result 사이 CIM PPID `OpenRecipe` | 래치 −1 → 원문 seg0 게이트(오늘 동작). 드묾. Result 뒤·Unload 전에 끼면 G1/G2가 갈릴 수 있음 — 기록 |
| `_USE_TEST` 빌드: 래치가 `:619` 테스트 덮어쓰기(seg0 활성 슬롯만) 전에 계산됨 | 생산 빌드 무관. 테스트 빌드 하이브리드는 래치가 원래 플래그를 따른다 — 기록 |
| 2글자 접두 + 긴 GlassID | 31자 절단 + WARN(D4-9), 시작 시 최장 길이 로그(L3) |
| PG(Ensis)가 만드는 ID와 HMI ID가 seg ≥ 1에서 다름 | D4-11 벤더 질의. 결과·APD의 CELLID는 HMI ID(WAD) |
| 운전 화면·DLL 화면 Cell ID 불일치(4b 전까지 DLL 화면은 세그먼트 로컬 ID) | D4-10, Phase 4b에서 해소. 운전 화면은 4-5로 일치 |
| 단일 그룹 스틱(V2/W3)의 가짜 행 기록이 segBase로 앞 세그먼트 마지막 셀을 덮음(리뷰 B1) | N>1은 P38a/P38b로 정정, H8로 OOS 확인. N=1 결함은 D4-16 별건. H1~H5 레시피는 해당 분기 미진입(그룹 3·4개) |
| `:2597` `m_pStInnerID[-1]` memset(UB)·V2 `:3910` `nCellStatus[-1]`(공유메모리 경계 밖) — N=1에도 있음 | N>1: P38b가 V2 첨자를 고치고 P38c가 memset을 막는다. N=1: D4-17(a) 승인 시 memset만 가드, `:3910`은 D4-16 별건 |
| seg0에 없는 장치를 뒤 세그먼트가 쓰는 하이브리드가 실기에서 레시피 미수신·VISION `HEAD_ACK_TIMEOUT`(리뷰 B2, Phase 3 결함) | D4-18: 4-7(N>1만). 미승인이면 운용 지침 "seg0이 모든 장치 항목을 켠다" + D3c-7 유지. OOS H3는 이 결함과 무관하게 통과한다(대기 경로가 OOS에서 true) |
| VISION 레시피 완료 대기의 재대기(같은 글라스에서 스틱 0을 두 번) — N=1 AgingCycle VISION 반복에도 해당 가능 | N>1은 4-7 `TakeVisionRcpWait`. N=1은 손대지 않는다(실기 확인 후 별건) |
| CIM PPID 재로드가 글라스 중간에 끼면 `RecipeChange`가 VISION 대기 래치도 0으로 → 다음 스틱 0에서 다시 대기 | 드묾. 4-7 전에도 N=1은 매 스틱 0에서 대기하므로 새 위험 아님 — 기록 |
| dResult `CELL_X/Y`가 다음 비-WAD 글라스로 넘어감(리뷰 B3) | D4-4 주, 별도 승인(`DataClear` 리셋). N=1 회귀 R1/R2는 하이브리드 글라스 직후가 아닌 시점에 돌린다(OOS는 업로드가 없어 0 그대로) |
| 헤더 2개 변경으로 넓은 재컴파일 | 4-0을 먼저 단독 ClCompile, ALPHA 실행 중이면 `ALPHA.vcxproj //t:ClCompile` 후 종료 요청 → 전체 빌드 |
| N=1 26글자 초과 레시피의 비문자 ID(`[` 등) | N=1 legacy 유지(HARD RULE). 현재 N=1 최대 21글자 |
| 3c 로그 파서 의존 | 하이브리드 로그는 09-29 신설이라 소비자 없음. N=1 바이트 동일(검증 1) |

---

**(11) 구현 단계** (스크립트 `p4_patch.pl --stage=N`, 백업 `p4_bak`, 스테이지마다 단독 컴파일 가능)

| 단계 | 내용 | 파일 | 의존 | 검증 |
|---|---|---|---|---|
| **4-0 API** | P1–P7: 멤버 2, 게터 3, 정적 2(`CellIdLetterPrefix` 음수 = `@`), 래치 Set/Get, Sync 글자 base, 생성자·RecipeChange 리셋 | `RecipeEditMgr.h/.cpp` | 없음 | ClCompile. 호출처 없어 동작 0 |
| **4-1 행 segBase + 행 로그 + 단일 그룹 정정** | P8–P19, P38a, P38b, P38c(D4-17 답에 따라 (a)/(b)) | `DoWorkGlassMeasureStep.h/.cpp`, `DoWorkGlassMeasure.cpp` | 4-0 | 검증 1, H1(행), H3(행), H5, H6, H8 |
| **4-2 Cell ID 이월** | P20(`rep1`)–P25 | `DoWorkGlassMeasureStep.cpp` | 4-0 | 검증 1(ID), H1(장치 ID), H2, H3(2글자), H8(ID) |
| **4-3 게이트 + APD 헤더 + 병합 추정** | P26(L12 포함)–P29 | `DoWorkGlassRecipeStep.cpp`, `DoWorkGlassResultStep.cpp`, `DoWorkGlassUnloadStep.cpp` | 4-0 | 검증 1(7/8 로그), H1 L1–L3/L7/L8·L12 0건, H3 파일 생성, H4 |
| **4-4 업로드 스냅샷 로그** | P30–P34 (P32(a)는 단독 줄 앵커) | `CommTcpIpObjectWAD.cpp`, `CommTcpIpObjectAngle.cpp`, `CommTcpIpObjectVision.h/.cpp` | 없음(3c `GetSegLogTag`만) | 검증 1(Control 로그 diff 0), 실기 9 |
| **4-5 운전 화면 셀 이름** | P35–P37 | `DlgSubOperate.cpp` | 4-0 | H7 |
| **4-7 하이브리드 장치 레시피 (D4-18 승인 시)** | P39–P42 | `RecipeEditMgr.h/.cpp`, `DoWorkGlassRecipeStep.cpp`, `DoWorkGlassPreMeasStep.cpp`, `DoWorkGlassMeasureStep.cpp` | 4-0(같은 파일 앵커), 4-3 뒤 권장 | 검증 1(로그 0건), H1/H3 L13/L14, 실기 9(D4-18) |
| **4-6 문서** | 아래 | `docs/*.md` | 4-0~4-5(+4-7) | — |

- 4-1과 4-2는 같은 파일이지만 다른 함수라 따로 적용·빌드해 이분 탐색이 되게 한다. 4-4는 다른 단계와 독립이라 언제든 넣을 수 있다. 4-7은 미승인이면 빼고, 승인돼도 마지막에 따로 빌드한다(시퀀스 변경을 행·ID 변경과 분리).
- P38a/P38b는 4-1에 넣는다: P10/P12가 segBase를 더하는 순간 단일 그룹 분기가 앞 세그먼트 행을 덮으므로 같은 스테이지에서 막아야 한다.
- **공통 절차**(3c와 같다)
  - 편집 전 `p4_bak`에 원본 백업(pfw `commit`).
  - 앵커마다 `index` 2회로 유일성 확인, 치환 개수 `post` 확인.
  - 편집 뒤 damaged = 0, `FB_CROAK` 엄밀 디코드(CP949), CRLF 수 = LF 수.
  - 바뀐 블록을 `sed -n`으로 다시 읽는다.
- **빌드:** 루트 CLAUDE.md HMI 명령(`2022/Community`, `ALPHA.sln //p:Configuration=Debug //p:Platform=x64`). DLL 변경 없음 → `dfcRecipe:Rebuild` 불필요. ALPHA가 떠 있으면 `//t:ClCompile`로 문법만 먼저.
- **4-6 문서 갱신 목록**
  - `HybridGlass_Porting_Plan.md` §5.7: 행 사이트 줄 번호 갱신, "segBase는 SetMeasUseInfo 또는 헬퍼" → "스텝(SetMeasUseInfo 전)", "B 패널 로직 무변경" → "의미 무변경, 파서는 여러 글자 대응(N>1)", "`m_szCellID[16][32]`는 스틱 단위" → "헤드 슬롯 단위, 31자 제한".
  - §6 Phase 4 상태·결정 D4-1~18, 3c (12)의 1~4번 해소 표시, §9-3/4/6/7/14 답, §10 수정 파일 표에 Phase 4 행. Phase 3 알려진 결함에 "seg0 전용 장치 레시피 열기·VISION 재대기"(4-7 또는 운용 지침) 추가.
  - `docs/` 또는 HMI CLAUDE.md 측정 그룹핑 절(스킬 `hmi-measure-grouping`)에 V2 장치0 마스터·±HeadGap/2 이동, `{-1,c}` 결함(N=1 별건) 기록 제안.
  - `Result_Identification_MES_Agreement_Plan.md` §3에 segBase 정의, CELL_X/Y = G×1(하이브리드) 질의, §5의 "1000 하드코딩"을 2500으로 정정.
  - 루트 CLAUDE.md LF-only 목록 정정 제안(사용자 결정).

---

**(12) Phase 5(Resume)에 넘길 것**

1. **행은 측정 시점에 최종값이다.** `RSRITEMDATA.iMeasArea`에 segBase·G·block이 다 들어 있다. 저널(`CResultMgr::Updata_Data`, `Manager/ResultMgr.cpp:51`, ASCII CRLF 73줄)은 받은 값을 그대로 기록·재생하면 된다. 재생 때 행을 다시 계산하지 말 것.
2. **세그먼트 식별:** WAD/ANGLE은 업로드 시점 `GetSegLogTag(TRUE)` = 측정 세그먼트, VISION은 슬롯 `m_strSegTag`(grab 시점). 저널에 seg를 적으려면 이 값을 쓴다(또는 L4 표로 행 → seg 역산).
3. **캐시 갱신은 `SyncCellStatusStickBase`뿐이다**(`RecipeChange`, `SegmentSwitchData`). 재개가 (pass, seg)로 바로 들어갈 때 반드시 `SegmentBoundary`/`SegmentSwitchData`를 거쳐야 rowbase·letterbase·stick base가 맞는다. `SetActiveSegment` 직접 호출 금지.
4. **Recipe Change N>1 블록을 재개 때도 돌려야 한다.** 저장 게이트 래치와 APD 헤더(G × 1)가 여기서 매 글라스 정해진다. 재개 글라스에서 이 블록을 건너뛰면 래치 −1(seg0 게이트), 헤더 seg0 값이 된다.
5. **RS1 재적재(`DoWorkGlassRecipeStep.cpp:128`)는 segBase·G·block이 없다.** N=1 fallback 전용으로 두고 하이브리드는 저널만 쓴다. 저널 없는 하이브리드 글라스는 재개 거부 또는 "새로" 강제.
6. **Cell ID는 상태가 없다.** 같은 (seg, s, c, 패널)이면 재측정해도 같은 ID가 나온다(InnerID만 새로). 재개 스틱 전체 재측정 정책과 충돌 없음.
7. **EFA T1 재개:** T1이 T0 행을 덮는 규칙은 (seg, s, c) 단위로 정확하다. 저널 재생은 "마지막 쓰기 우선"이면 된다(§9-18).
8. **파일 인코딩:** `Manager/ResumeInfoMgr.h/.cpp`는 **CP949 LF-only**(87 / 387 LF, CR 0). `ResultMgr.cpp` ASCII CRLF, `DlgSubOperate.cpp` CP949 CRLF, `DoWorkGlassRecipeStep.cpp` CP949 CRLF.
9. 검증 기준선: Phase 4 OOS 런(H1, H3, H8)의 L5 줄(정규식 `\[HYBRID\] result row (WAD|WAD-MP|VISION|ANGLE) `)이 재개 뒤 재생 결과와 행 단위로 같아야 한다. L5의 use는 "그 행이 실제로 기록됨"(VISION V2 = `use(1,0)`)이라 재생 대상 행 집합과 일치한다.
10. (4-7 적용 시) 재개 글라스는 Recipe Change가 다시 돌므로 `m_nVisionRcpWaitLatch`가 0이 되고 장치 레시피를 모든 세그먼트 기준으로 다시 연다 — 재개 스틱이 세그먼트 k ≥ 1의 스틱 0이 아니어도 VISION 대기가 한 번은 일어나도록(또는 재개 시 대기 생략) Phase 5에서 정한다.

---

**(13) 리뷰 반영** (2026-10-01, 리뷰어 2명: math-n1 / complete. 판정 표·근거 줄은 `p4_plan_reviews.md`)

| # | 지적 | 판정 | 플랜에 바뀐 것 |
|---|---|---|---|
| A1 | H3/H6 범위 하한이 ms = 0 가짜 행(segBase)을 실패로 본다 | 채택 | (3) 성질 문구(하한 포함, use = 1 행은 +1), H3 `[100,205]` / use = 1 `[101,205]`, H6 use = 1만 |
| A2 | D4-14 예시 과소, Phase 4 뒤 세그먼트째 병합 누락에 경고 없음 | 채택 | D4-14 예시 정정(WAD 32..40 짝수 + 81..100), P26에 병합 추정 L12 WARN(N>1만), (10)·(8)-9 |
| A3 | VISION V2 로그 `cell(c0,c1) use(1,1)` 오해 | 채택 | P16 → 블록(V2 `cell(c,c)`, use = `bMeasProbe && row >= 0`) |
| B1 | D4-3 틀림 — V2 `{-1,c}`·W3 bOneCell은 가짜 행을 실제 기록, segBase 뒤 앞 세그먼트 마지막 셀을 덮음 | 채택(+확장) | P38a(W3 헤드1 행 = 자기 셀, N>1), P38b(V2 `nUseCELLID[0]` = 마스터 셀, N>1 — 행·ID·`:3910 nCellStatus[-1]`까지 한 줄로), P38c(`:2597` memset, D4-17), P21 `nCellNum >= 0`, `CellIdLetterPrefix(<0)` = `@`, D4-3/D4-16 재작성, H8 신설. 리뷰 제안 중 "V2 행을 마스터 셀로 장치0에"는 그대로 따랐다 — V2 이동이 두 헤드를 ±HeadGap/2로 보내고 `MarkIdleHeadSafety`가 그 축을 건드리지 않으므로 장치0도 그 셀을 찍는다 |
| B2 | 장치 레시피 열기·VISION 대기가 seg0 전용/세그먼트마다 재대기 → seg0 전기만 하이브리드는 실기에서 완주 불가 | 채택(사용자 선택) | (1)(g), (2h) 4-7 P39–P42(N>1만), D4-18, D4-15 조건, (10), (8)-9. Phase 3 결함이라 포함 여부를 사용자가 정한다 |
| B3 | dResult `CELL_X/Y`가 다음 비-WAD 글라스로 넘어감 | 채택(기록) | D4-4 주, (2g) dResult 행, (10), (8)-9. 수정(`DataClear`)은 N=1 헤더도 바꾸므로 별도 승인 |
| B4 | L4·L5 접두 충돌로 H1/H4 개수 틀림 | 채택 | L4 = `[HYBRID] rows per segment:`(P18), 정규식 명시(H1/H3/H4, (3), (5)) |
| B5 | P20 `ins_after`면 for 몸체에 선언 → C2065, P32a 한글 주석 삼킴 | 채택 | P20 `rep1`(치환 문자열 명시, 대안 `infunc`), P32(a) 단독 줄 앵커(n=1), (9) 방식 목록에 전 패치 배정 + "줄 끝 아닌 앵커 ins_after 금지" |

- 새 앵커 확인(2026-10-01, LF 정규화 raw): P38a `:2834` n=1, P38c `:2597` n=1, P38b `if (nUseCELLID[1] < 0) bMeasProbe[1] = false;\n`(4탭) 파일 n=5 / VISION::Run 안 n=1, P42 `if (iStick == 0 && i == 0)\n`(5탭) 파일 n=2 / VISION::Run 안 n=1, P40 `RecipeStep:683` n=1, P41 `PreMeasStep:712` n=1, P32(a) `\tCString m_strCellID;\n` n=1. VISION::Run 범위 = :3088 서명 ~ :3978 첫 `\n}\n`.
- 사용자 결정 필요: **D4-17**(`:2597` 가드를 N=1에도 — 권장 (a)), **D4-18**(4-7 포함 — 권장 (a)). 나머지 리뷰 반영은 모두 N>1 전용이거나 문서·검증 정정이라 HARD RULE과 충돌하지 않는다.

### Phase 4b — Cell ID A/B 수동 입력 + Manual 편집기 (D-5) — ✅ 구현·컴파일 오류 0 (2026-10-01, dfcRecipe 18/18 · ALPHA 244/244 ClCompile), 링크·OOS 대기

**구현 결과 (2026-10-01)** — 아래 "설계 원문"은 구현 전 플랜(리뷰 반영본)이다. 줄 번호는 설계 시점 기준이라 지금 코드와 다르다. 위치는 앵커 문자열로 찾을 것.

- **진행 순서**
  1. 설계 워크플로: 매퍼 3(ICP 원본 / HMI 레시피 DLL·UI / HMI 런타임) → 플래너 → 리뷰 2(호환성 / 완전성) → 수정. 지적 18건 전부 반영(기각 0).
  2. 사용자 결정: D4b-7 (a) 기본 모드 화면 그대로, D4b-15 (b) `.rcp`를 못 읽었을 때만 거부. 나머지 D4b-1~6, 8~14는 권장안 (a).
  3. 구현 워크플로: 4b-0 공용 헤더·규칙 코어 → (4b-1 DLL 로드·저장 / 4b-2 편집기·rc / 4b-4 ALPHA 런타임·D-15) 병렬 → 4b-3 DLL 화면 연결 → 전체 컴파일 → 리뷰 2(N=1 불변 / 정확성) → 수정.
  - 스크립트·백업은 세션 scratchpad(`p4b_4b0~4b4_patch.pl`, `p4b_fix_patch.pl`, `p4b_bak\<stage>\`). 세션이 끝나면 사라지므로 복구는 git으로 한다.
- **파일 포맷 (D4b-1, D4b-2)**
  - `RECIPE_FILE_V1012` 신설(저장 버전 1013). `_HYBRID_END_` 뒤에 `_CELL_ID_PANEL_` / 모드 A / 모드 B / nBlock(0 또는 세그먼트 수) / 세그먼트마다 `_CELL_ID_MANUAL_ k` + 스틱 수·셀 수 + 스틱마다 A줄·B줄(토큰마다 쉼표) / `_CELL_ID_END_`. 두 로더 모두 `> RECIPE_FILE_V1012` 게이트.
  - §5.7의 "V1011 꼬리 안"에서 바꾼 이유: 배포된 빌드의 `LoadHybridTail`이 태그를 엄격히 검사해 꼬리 안에 넣으면 구 빌드가 새 파일을 못 연다. 지금 위치면 구 빌드는 트레일러만 무시한다.
  - 구 파일(1012 이하) → Auto/Auto. Auto/Auto 저장은 5줄(N=1 포함). 트레일러가 깨지면 로드 실패 + 전체 롤백.
- **모델·규칙 코드 (D4b-11)**
  - `S_CellIdConfig`(모드[2] + [패널][세그먼트][스틱][셀]) / `S_CellIdError` / `inline SegCellIdLetterPrefix`는 `SegmentDefine.h` 두 사본(자동 복사 없음 — 손으로 맞춘다). 멤버는 `C_In/OutRecipeFunc` 클래스 끝. `preDefine.h` 구조체·`S_CellInfo`는 무변경(enum 1줄만).
  - 규칙은 DLL `CellIdRule.h/.cpp` 한 벌: Auto 토큰(ALPHA `MakeCellID`/`GenCellID` 바이트 재현), 리졸버, 파생 B(D4b-5 `%0*d`, D4b-6 HMI `nMaxTop`), 검증(형식·파생·길이·전 세그먼트 A+B 중복), 트레일러 Load/Save. ALPHA `CellIdLetterPrefix`는 `SegCellIdLetterPrefix`로 forward.
  - 오프라인 기대값(`p4b_expect.pl`, `p4b_expect_rot.pl`)과 C++ 출력이 줄 단위로 일치(4b-0 Perl 전사 + 정확성 리뷰의 MSVC 하니스 둘 다).
- **편집기 (D4b-12)**: 레시피 화면 "CELL ID" 버튼(`IDC_BUTTON_14`, 641,144) → `C_CellIdEditDlg`(IDD 2008, `MFCGridCtrl` 9개, `CellIdEditDlg.cpp` CP949).
  - 작업 사본 편집, APPLY = 검증 + 커밋, CANCEL = 폐기. `GetRecipeView()`는 부르지 않는다.
  - MakeCell 창이 보이면 열기 거부. 열린 뒤 설정 세대(`GetCellIdCfgSeq`)나 형상이 바뀌었으면 APPLY 거부(리뷰 C1/K1). `AFX_MANAGE_STATE` 첫 줄, `DoModal() == -1` 팝업.
  - 저장 전 검증: `SaveFile`이 파일을 열기 전에 실행(Auto/Auto는 검사 없음). MakeCell APPLY 두 곳에서 Cell ID 설정 초기화, 비기본이었을 때만 팝업(D4b-13).
- **화면 (D4b-7 (a))**: HEAD 그리드 CELL ID 열·SELECT CELL·DLL 검증 모드는 `GetDisplayCellId` — Auto/Auto(세그먼트 수 무관)는 기존 `m_chCellID` 그대로, 수동 설정일 때만 해석된 패널 A. 하이브리드 Auto/Auto의 이월 글자 표시, MakeCell 오버레이·운전 화면의 수동 ID는 Phase 6.
- **런타임**
  - `GenCellID`만 수정(`MakeCellID` 무변경): `GetManualCellToken(bPanelB, stick, cell, GlassID, tok)`이 토큰을 주면 셀 토큰만 교체하고 B 오프셋 분기를 건너뛴다. 31자 가드는 수동 토큰까지 넓히고 태그는 하이브리드 `[HYBRID]`, N=1 수동 `[CELLID]`.
  - 토큰은 래치 1 + Out 로드 일련번호(`GetRcpLoadSeq`) 일치 + GlassID 일치일 때만. 아니면 수동 설정이면 `[CELLID] ... not validated ... - Auto cell IDs from here` WARN 1회 후 래치 2(나머지 셀 Auto). Auto/Auto는 래치 0, 로그 없음.
  - 리뷰 수정(C2): `C_OutRecipeFunc`에 로드 진행 카운트(`IsRcpLoadBusy`)·시도 카운트(`GetRcpLoadTry`). 측정 스레드는 로드 중이면 읽지 않고, 읽은 뒤 다시 확인해 바뀌었으면 토큰을 버리고 `[CELLID] recipe reload during the cell ID read ...` WARN 1회.
- **시작 재검증 D-15**: Recipe Change 스텝, 하이브리드 블록 뒤·`[PRECHECK]` 앞(세그먼트 수 무관).
  - Auto/Auto: 로그 0. 수동 설정: OPER `[CELLID] manual cell ID: A ..., B ..., glass panel ..., longest ID n/31 chars`. 실패: `[CELLID]` WARN + `cell ID check failed (n errors, GlassID ...) - auto start refused` + `SetFail(true, 0)`(알람 없음).
  - 리뷰 수정(C1): 로드 일련번호를 검사 전에 읽어 래치에 묶는다. 검사 중 재로드가 끼면 시작 거부(`recipe reloaded during the cell ID check ... auto start refused`, 알람 없음, 수동 설정에서만).
- **D4b-15 (b) 레시피 읽기 실패**: Out 로드 일련번호가 `OpenRecipe` 전후 같을 때만(= `.rcp` 자체를 못 읽음).
  - Recipe 스텝(기존 `:626`은 반환값 무시): `[RECIPE] recipe <id> could not be read (file missing or corrupt) - auto start refused` WARN + `SetFail(true, JOB_RECIPE_NOT_EXIST)`(알람 1013).
  - 사이클 화면 `D_MeasureCycle.cpp`(기존 `0 < OpenRecipe()`가 1013도 성공으로 받음): 한국어 팝업 후 대화상자 유지.
  - 읽을 수 있는 레시피(기본 `.alg` 누락 반환 포함)는 경로 무변경. 같은 결함인 수동 측정 `DlgSubManual_Meas.cpp`의 `0 < OpenRecipe()`는 Phase 7.
- **컴파일** (Debug x64, 링크 없이 ClCompile, scratchpad 전용 IntDir — VS·ALPHA 실행 중)
  - dfcRecipe 18/18 유닛 0 오류, 새 경고 0, ResourceCompile OK(다이얼로그 2008 + "CELL ID" 버튼 확인). ALPHA 244/244 0 오류, 경고 755곳 = Phase 4와 같은 위치(줄만 밀림), 4b 줄 새 경고 0.
  - 4b 수정 파일 27개: damaged 0, 엄밀 디코드 OK, EOL 일관(`dfcRecipe/DlgTestMode.cpp`는 LF 유지), BOM 유지(rc·resource.h UTF-16LE, vcxproj·filters UTF-8). 이중 사본 3쌍 cmp 일치.
- **리뷰**: N=1 불변 — 지적 0(유일한 화면 텍스트 차이: 셀 ID에 `%`가 든 손편집 `.rcp`만, MakeCell은 만들 수 없음 — 기존 코드는 ID를 서식 문자열로 넘겨 오히려 깨졌다, 수용). 정확성 — low 3건(C1 래치 일련번호 읽는 순서, C2 로드 중 읽기, C3 일괄 입력 무선택 문구) 전부 수정.
- **남은 것**
  - 전체 빌드: ALPHA 종료 → `dfcRecipe:Rebuild` → 일반 빌드(ALPHA 재링크). ALPHA가 새 export(`IsRcpLoadBusy`, `GetRcpLoadTry`, `GetRcpLoadSeq` 등)를 쓰므로 DLL과 exe를 같이 배포해야 한다(구 DLL이면 진입점 없음으로 실행 실패). Release|x64 빌드 + 스모크(O20)도.
  - 새 파일 4개(`CellIdRule.h/.cpp`, `CellIdEditDlg.h/.cpp`)는 git 미추적 — 두 PC 동기화 전에 커밋해야 유실되지 않는다.
  - OOS: 아래 설계 원문 (8)의 O1~O21. 첫 번째는 N=1·하이브리드 Auto/Auto 회귀 diff 0(Phase 4 OOS 13:22/13:25/13:30 기준).

---

**설계 원문 (2026-10-01 플랜, 리뷰 반영본)**

상태: **플랜(2026-10-01), 리뷰 2건(compat / complete) 반영 완료(2026-10-01, (13) 표), 구현 전.** 확정 사항(D-5 편집기까지, D-15 로그 + 자동 시작 거부·알람 없음, §5.7 모델·리졸버·검증·MakeCell 초기화·GetRecipeView 비호출)은 다시 열지 않는다. 사용자 선택이 필요한 것은 (7)의 D4b-n이다(리뷰로 D4b-7 재작성, D4b-15 신설).
**사용자 결정(2026-10-01): D4b-7 (a) 기본 모드 화면 그대로, D4b-15 (b) `.rcp`를 못 읽었을 때만 거부(P21 `D_MeasureCycle.cpp` 포함). 나머지 D4b-1~6, 8~14는 권장안 (a)로 확정.**
- 리뷰 원문: `scratchpad/p4b_plan_reviews.md`. 리뷰 반영 검증 스크립트: `scratchpad/p4b_fix/`(`anchors_new.txt` 새 앵커 n, `enc1.pl` 인코딩, `p4b_expect_rot.pl` rot0/rot3·GlassID 기대값).
- 출처: 매퍼 3개(`p4b_map_icp.md` ICP 참조 구현 / `p4b_map_dll.md` DLL·파일 / `p4b_map_rt.md` ALPHA 런타임) → 이 플래너. 매퍼 주장 중 설계에 걸리는 것은 코드로 다시 확인했다(스크립트 `scratchpad/p4b_plan/`: `show.pl` 보기, `cnt.pl`+`anchors.txt` 앵커 개수, `enc.pl` 인코딩/EOL, `p4b_expect.pl` 규칙 시뮬레이터 = (3)·(8)의 기대값 출처).
- 코드 기준: `D:\OCP\0.SW_Matching\HMI`, 브랜치 `hmi-master`, HEAD `8a454bf` + **Phase 4 미커밋 14파일**(작업 트리). DLL(`CoreFunction/dll_Recipe/dfcRecipe`)은 HEAD와 같다. 줄 번호는 작업 트리 값(참고용), 앵커는 raw를 CRLF→LF 정규화한 사본에서 `index()`로 n=1 확인(예외는 표에 n 표기).
- **HARD RULE:** 1세그먼트(N=1) 레시피와 기본 Auto/Auto 모드는 동작·로그 문자열·장치 SEND·결과가 바이트 동일해야 한다. 구 파일(≤1011, 1012 하이브리드 꼬리 파일)은 그대로 로드돼야 한다. 버전 게이트는 `>`. 두 `preDefine.h`는 ABI 계약. 새 팝업은 한국어, 로그는 영어. 주석 `// LYS 2026-10-01 ...`.

---

**(0) 요약 / 범위**

*한 줄*: ICP Cell ID A/B Auto/Manual 모델을 HMI에 옮긴다. 저장은 `S_CellInfo`가 아니라 `C_In/OutRecipeFunc` 끝의 `S_CellIdConfig`(레시피 전역 모드[2] + [패널][세그먼트][스틱][셀] 수동 ID), 파일은 `_HYBRID_END_` **뒤** 트레일러(`RECIPE_FILE_V1012` 신설, 게이트 `> V1012`), 규칙은 DLL 내부 순수 함수(`CellIdRule.*`)를 In(편집기·저장)과 Out(ALPHA 런타임)이 공유, 런타임은 `GenCellID`의 셀 토큰만 교체(Manual/파생 B일 때만, B 오프셋 분기 건너뜀), D-15는 Recipe Change 스텝에서 `SetFail(true, 0)`. D-15 래치는 **검사한 설정(Out 로드 일련번호 `GetRcpLoadSeq()`)과 GlassID에 묶인다** — 글라스 중간 재로드(CIM PPID, DLL 검증 모드)·다른 GlassID면 WARN 1회 + 나머지 셀 Auto(Auto/Auto는 로그 0). 편집기는 MakeCell 창이 열려 있으면 열리지 않고, APPLY는 In 설정 세대(`GetCellIdCfgSeq()`)가 바뀌었으면 거부한다.

*4b에 들어가는 것*
1. 데이터 모델: `S_CellIdConfig`(SegmentDefine.h 두 사본) + In/Out 멤버·접근자, 로드 백업/복원, MakeCell APPLY 초기화.
2. 파일: `RECIPE_FILE_V1012`(END 1013) + `_CELL_ID_PANEL_ … _CELL_ID_END_` 트레일러(N=1 포함 항상 기록, Auto/Auto는 5줄).
3. 리졸버·검증(순수): HMI Auto 토큰 바이트 재현(N=1 `%c` / N>1 base-26 이월 + B 파서), A Manual + B Auto 파생(B 오프셋 = HMI `nMaxTop`), 형식 `[A-Z]+[0-9]+`, 길이(저장 20자 / 시작 시 실제 GlassID), 전 세그먼트 A+B 합산 중복.
4. 저장 전 검증(`SaveFile`, 파일 열기 전, 한국어 팝업 최대 5건).
5. 편집기 `C_CellIdEditDlg`(DLL, 모달, 세그먼트별 그리드, 콤보 3개, 선택 편집, 일괄 입력, 색, APPLY/CLOSE) + RecipeMgrDlg 진입 버튼 "CELL ID".
6. 런타임: `GenCellID` 토큰 교체(래치 1 + 로드 일련번호·GlassID 일치일 때만), 31자 가드를 override까지 확장(WARN 태그는 N=1이면 `[CELLID]`), 래치 무효화 시 WARN 1회.
7. D-15: Recipe Change 스텝 하이브리드 블록 뒤, `[PRECHECK]` 앞. Auto/Auto는 로그 0줄.
8. DLL 레시피 화면 3곳(HEAD 그리드 CELL ID 열, SELECT CELL, 검증 모드 그리드)의 Cell ID = 리졸버 패널 A 값 — **설정이 Manual(비기본)일 때만**. Auto/Auto(N 무관)는 기존 `m_chCellID` 그대로(D4b-7 (a), HARD RULE 문자 그대로).
9. (D4b-15 (b) 선택 시) `.rcp` 자체를 못 읽었을 때만 자동 시작 거부(Recipe 스텝 :626) + 사이클 화면 선택 거부(`D_MeasureCycle.cpp:223`). 읽을 수 있는 레시피의 경로(기본 `.alg` 누락 반환 포함)는 무변경.

*이월(기록)*
- 물리 합성 셀맵(`C_PhysicalCellLayout`/`C_CellMapGrid`) — §7 보류 그대로. 편집기는 세그먼트별 그리드.
- MakeCell 오버레이(`GlassMap`)·운전 화면 셀 이름(`DlgSubOperate`)의 수동 ID 반영 → Phase 6(D4b-7). **N>1 Auto/Auto의 DLL 화면 이월 글자 표시(D4-10이 4b로 넘긴 항목)도 Phase 6**(D4b-7 (a)).
- 수동 측정(`DlgSubManual_Meas`, UTF-8 BOM)의 리졸버 적용 → Phase 7(D4b-9). 그때까지 수동 측정은 Manual 레시피에서도 Auto ID를 보낸다. (같은 파일 `:2092` `0 < OpenRecipe(...)`도 실패 코드 1013을 성공으로 받는다 — D4b-15와 같은 결함, Phase 7에서.)
- PG(Ensis)는 Cell ID 문자열 없이 스틱 2자리 + 방향 1자리만 보낸다(`CommTcpIpObjectPG.cpp:1702-1759`) → 수동 ID 표현 불가, 기록만.
- **HMI P1~P4 AgingPG**(`CommTcpIpObjectAgingPG.cpp:1603-1640`, `:2207-2261` 독자 `%c%02d` + B 파서 `:2253` + `gf_MakeMcrID`, UTF-8 BOM CRLF 2317) — 무변경. Manual 레시피라도 HMI P1~P4 에이징 결과는 Auto ID를 싣는다. OCP는 P5 = `CommTcpIpObjectPG`라 해당 없음.
- WAD Multi Point 로그(`DoWorkGlassMeasureStep.cpp:2992-3007`)는 MakeCellID 토큰(Auto)을 계속 찍는다. 장치 SEND·L5 `[HYBRID] result row … id(..)`는 최종 ID라 자동 반영.
- `DoWork/DoWorkGlassMeasure.cpp:206-220` `[HYBRID] rows per segment: … letters %s..%s` — Auto 글자 기준(Manual 모드에선 실제 ID와 다름), 무변경·기록.
- `InRecipeFunc.cpp:1225` `CheckRecipeLayout` 오류 문구의 셀 이름 = `m_chCellID`(세그먼트 로컬 레시피 값), 무변경·기록.
- ALPHA `DlgTestMode.cpp:216`(LF-only) 독자 `%c%02d` 표시, 사이클 GlassID 규칙 — 무변경.

---

**(1) 데이터 모델**

*1-1 왜 `S_CellInfo`가 아닌가 (확정 §5.7, 근거 재확인)*
- `S_CellInfo`(`preDefine.h:364`)는 `wchar_t m_chCellID[1024]` 등 POD + `Clear()`=ZeroMemory이고, ALPHA는 `GetRecipeInfo/GetRecipeData`로 DLL 구조체에 원시 포인터를 잡는다. CString 추가 = ABI 파괴(ICP X3).
- `m_chCellID`는 MakeCell이 만든 세그먼트 로컬 값이며 HeadPair 로그(`DoWorkGlassMeasureStep.cpp:453`)와 DLL 화면이 읽는다 → 4b는 **재기록하지 않는다**(D4b-8).

*1-2 새 타입 — `SegmentDefine.h`(DLL·`Source/Include` 두 사본, ASCII CRLF, 자동 복사 대상 아님 → 손으로 둘 다)*
```cpp
#include <vector>	// LYS 2026-10-01 Hybrid Phase 4b S_CellIdConfig   (#pragma once 바로 뒤)

// LYS 2026-10-01 Hybrid Phase 4b Cell ID A/B Auto/Manual (docs/HybridGlass_Porting_Plan.md 5.7, D-5)
//   Recipe-global modes + per-cell manual IDs, kept out of S_CellInfo (preDefine.h raw-pointer ABI to ALPHA).
//   Invariant: an Auto panel has no manual IDs (empty vector); a Manual panel holds [seg][stick][cell] of the recipe geometry.
#define M_CELLID_PANEL_MAX		2		// 0 = A, 1 = B
#define M_CELLID_MODE_AUTO		0
#define M_CELLID_MODE_MANUAL	1
#define M_CELLID_TOKEN_MAX		20		// manual / derived token chars at save: GlassID 11 - 1 + token 20 + checksum 1 = 31
#define M_CELLID_ID_MAX			31		// final ID chars: stLotInfo.m_szCellID[..][32], WAD MCRID[2][32] (_tcscpy_s aborts on 32+)
#define M_CELLID_ERR_EMPTY		1
#define M_CELLID_ERR_LENGTH		2
#define M_CELLID_ERR_FORMAT		3
#define M_CELLID_ERR_DERIVE		4
#define M_CELLID_ERR_DUP		5

struct S_CellIdConfig {
	int	m_nMode[M_CELLID_PANEL_MAX];												// M_CELLID_MODE_*
	std::vector< std::vector< std::vector<CString> > > m_vecManual[M_CELLID_PANEL_MAX];	// [panel][seg][stick][cell]
	S_CellIdConfig () { Clear (); }
	void Clear () { for (int p = 0; p < M_CELLID_PANEL_MAX; p++) { m_nMode[p] = M_CELLID_MODE_AUTO; m_vecManual[p].clear (); } }
	BOOL IsDefault () const { return m_nMode[0] == M_CELLID_MODE_AUTO && m_nMode[1] == M_CELLID_MODE_AUTO; }
};

struct S_CellIdError {
	int		m_nCode;									// M_CELLID_ERR_*
	int		m_nSeg, m_nStick, m_nCell, m_nPanel;		// 0-based location
	int		m_nSeg2, m_nStick2, m_nCell2, m_nPanel2;	// M_CELLID_ERR_DUP: first location
	int		m_nLimit;									// M_CELLID_ERR_LENGTH: allowed chars
	CString	m_strToken;									// offending text (DERIVE: the A source)
	S_CellIdError () : m_nCode (0), m_nSeg (-1), m_nStick (-1), m_nCell (-1), m_nPanel (0),
					   m_nSeg2 (-1), m_nStick2 (-1), m_nCell2 (-1), m_nPanel2 (0), m_nLimit (0) {}
};

// LYS 2026-10-01 Hybrid Phase 4b bijective base-26 (0 A .. 25 Z, 26 AA, 702 AAA; negative -> "@") - moved here from
//   CRecipeEditMgr::CellIdLetterPrefix (Phase 4) so the DLL editor and ALPHA use one copy (D4b-11)
inline CString SegCellIdLetterPrefix (int nIndex)
{
	CString strPrefix;
	if (nIndex < 0) return CString (_T("@"));
	do { strPrefix.Insert (0, (TCHAR)(_T('A') + (nIndex % 26))); nIndex = nIndex / 26 - 1; } while (nIndex >= 0);
	return strPrefix;
}
```
- 앵커: `#pragma once\n`(n=1, :1) 뒤 include, `\t\tZeroMemory (m_chWADName, sizeof (m_chWADName));\n\t}\n};\n`(n=1, :26) 뒤 나머지. 두 사본 `cmp` 0.
- 불변식: `m_nMode[p] == AUTO ⇒ m_vecManual[p].empty()`; `MANUAL ⇒ m_vecManual[p]`가 현재 형상(세그먼트 수 × `GetSegmentStickCount(k)` × `GetSegmentCellCount(k)`). 지키는 곳 = 로더(Auto 패널 토큰 버림), 편집기 APPLY(`CellId_EnsureShape`), MakeCell APPLY(초기화). 형상 접근은 항상 범위 검사(밖 = 빈 문자열).

*1-3 클래스 멤버 — 끝에만 추가(비가상)*
- `C_OutRecipeFunc`(`OutRecipeFunc.h`, DLL 원본 + `Source/Include` 사본. x64 DLL 빌드의 PreBuildEvent가 DLL 사본을 덮어쓰지만(`dfcRecipe.vcxproj:177-202`) ALPHA 단독 ClCompile 전에 두 사본을 같이 고친다): 앵커 `\n};`(n=1, :79-80) 앞.
```cpp
	// LYS 2026-10-01 Hybrid Phase 4b Cell ID A/B (docs/HybridGlass_Porting_Plan.md 5.7). Non-inline: ALPHA calls the DLL code.
	//   Members stay at the class end: the object lives on the DLL heap only and ALPHA holds a pointer, so no offset moves.
	public:
		BOOL	IsCellIdDefault ();				// both panels Auto (every recipe saved before 4b) -> legacy IDs everywhere
		int		GetCellIdMode (int nPanel);		// M_CELLID_MODE_*
		int		ResolveCellToken (int nSeg, int nStick, int nCell, int nPanel, CString &strToken, BOOL bAutoFallback = FALSE);
												// 1 = manual / derived from A (final token, B offset inside), 0 = Auto (strToken = Auto token only with bAutoFallback)
		BOOL	ValidateCellIdConfig (int nGlassIdLen, std::vector<S_CellIdError> &vecErr, int *pnLongest = NULL);
												// nGlassIdLen < 0 -> token cap M_CELLID_TOKEN_MAX; else the 31-char budget of that Glass ID. TRUE = OK (Auto/Auto always)
		int		GetRcpLoadSeq ();				// successful LoadRcpFile count: binds the D-15 latch to the config it checked (review C2/K3), D4b-15 (b)
	protected:
		BOOL	LoadCellIdTail (CStdioFile *pFile);
	public:
		S_CellIdConfig	m_sCellIdCfg;
		int				m_nRcpLoadSeq;			// ++ at the end of a successful LoadRcpFile only (a rolled-back load leaves it)
```
- `C_InRecipeFunc`(`InRecipeFunc.h`, DLL 전용·export 없음): 앵커 `\t\tCString CheckRecipeLayout (const S_RecipeMgr *pMgr, CString strWhat);\n`(n=1, :111) 뒤. 위 6개(`GetRcpLoadSeq`·`m_nRcpLoadSeq` 제외) + 편집기용:
```cpp
		BOOL			ResetCellIdConfig ();							// MakeCell APPLY: TRUE = a non-default config was cleared (info popup). Every call bumps m_nCellIdCfgSeq (cells rebuilt)
		S_CellIdConfig	GetCellIdConfig ();								// editor working copy
		void			SetCellIdConfig (const S_CellIdConfig &cfg);		// editor APPLY (Auto panels emptied, Manual panels shaped), bumps m_nCellIdCfgSeq
		CString			GetDisplayCellId (int nSeg, int nStick, int nCell);	// DLL screens: Auto/Auto (any N) -> m_chCellID as before, else resolved panel A (D4b-7 (a))
		void			SaveCellIdTail (CStdioFile *pFile);
		int				GetCellIdCfgSeq ();								// editor: refuse APPLY when the config / cells changed under it (review C1/K1)
		int				m_nCellIdCfgSeq;								// ++ on successful LoadRcpFile, every ResetCellIdConfig, SetCellIdConfig
```
- ABI: `C_OutRecipeFunc`는 `API_RECIPE.cpp:27-28`에서만 `new`, ALPHA는 포인터만(by-value 0건, `m_vecSegments` 직접 접근 0건 확인). `std::vector<S_CellIdError>`를 export 함수 인자로 넘긴다 — ALPHA Debug ↔ `dfcRecipe64d`, Release ↔ `dfcRecipe64`(`ALPHA.vcxproj:137/:223`), 둘 다 /MD·같은 툴셋이라 힙·이터레이터 레벨 일치(Release는 4b-5에서 실제 빌드·스모크로 확인, O20). C4251 경고가 `m_sCellIdCfg`에 1개 늘 수 있다(기존 `m_vecSegments`와 같은 성격). `m_nRcpLoadSeq`(int)도 클래스 끝 → 기존 오프셋 불변.
- 생성자: `S_CellIdConfig` 기본 생성자가 Auto/Auto라 설정은 손대지 않지만 **일련번호 초기화가 필요**하다 — In/Out 생성자 앵커 `\tm_nActiveSegment = -1;\t// LYS 2026-09-29 Hybrid Phase 1\n`(In :14 / Out :14, 각 n=1) 뒤 `\tm_nCellIdCfgSeq = 0;\t// LYS 2026-10-01 Hybrid Phase 4b\n`(In) / `\tm_nRcpLoadSeq = 0;\t// LYS 2026-10-01 Hybrid Phase 4b\n`(Out).
- 일련번호 증가(로드 성공): In/Out `LoadRcpFile` 롤백 블록 앵커 `\t\tm_nActiveSegment = nBakActive;\n\t\treturn FALSE;\n\t}\n`(In :163 / Out :146, 각 n=1 — (2-5)의 복원 줄을 그 앞에 넣은 뒤에도 유일) 뒤 `\tm_nCellIdCfgSeq++;\t// LYS 2026-10-01 Hybrid Phase 4b config generation (editor APPLY check)\n`(In) / `\tm_nRcpLoadSeq++;\t// LYS 2026-10-01 Hybrid Phase 4b successful load serial (D-15 latch binding, D4b-15)\n`(Out). 실패(롤백)는 증가하지 않는다.

*1-4 수명·동기*
| 시점 | In(`g_inRcpInfo`, 편집기) | Out(`g_outRcpInfo`, ALPHA) |
|---|---|---|
| 로드 | `C_RecipeMgrDlg::OnRecipeLoad` → `LoadRcpFile` | `CRecipeEditMgr::RecipeChange`(매 글라스 Recipe Change `:626`, CIM PPID, 화면들) / DLL `DlgTestMode.cpp:99-104` |
| 백업/복원 | `LoadRcpFile` 실패 시 Phase 1 롤백(`pBakMgr`, `vecBak*`, `nBakActive`)에 `sBakCid` 추가 | 같음 |
| 구 파일 | 본문 뒤 "1세그먼트 초기화" 블록에서 `m_sCellIdCfg.Clear ()` → Auto/Auto | 같음 |
| 편집 | 편집기 작업 사본 → APPLY → `SetCellIdConfig` | 없음 |
| 저장 | `SaveFile` 검증 → `SaveRcpSegments` 끝 트레일러 | 없음 |
| 형상 변경 | MakeCell APPLY 두 지점 → `ResetCellIdConfig`(세대 ++) | 다음 로드 |
| 일련번호 | `m_nCellIdCfgSeq`: 로드 성공·Reset(매번)·Set → 편집기가 OnInitDialog 값과 APPLY 때 비교 | `m_nRcpLoadSeq`: 로드 성공(`RecipeChange`, DLL `DlgTestMode` 재로드 포함) → D-15 래치와 함께 저장, `GetManualCellToken`이 비교 |
- In↔Out 메모리 동기는 없다(기존과 같음). 편집기 저장 → 파일 → 다음 Recipe Change에서 Out 재로드.
- **편집기와 MakeCell의 동시성(리뷰 C1/K1)**: 레시피 페이지는 자식 창(`dfcRecipe.rc:109` `WS_CHILD`)이고 MakeCell·SelectCell·Align·TestMode는 페이지가 `Create (IDD_…, this)`로 만든 모델리스 팝업(`RecipeMgrDlg.cpp:17-32`, 소유자 = 최상위 ALPHA 창)이다. 편집기 `DoModal`은 최상위 창만 비활성화하므로 이미 보이는 MakeCell은 클릭할 수 있다 → MakeCell RECIPE APPLY(`MakeCellDlg.cpp:438`/`:1251-1257`)가 `ResetCellIdConfig`를 부른 뒤 편집기 APPLY가 옛 `m_work`를 `CellId_EnsureShape`(인덱스 유지)로 새 셀에 덮을 수 있다(같은 크기 재생성이면 형상 비교로도 못 잡는다). 대응 2단: ① `OnCellIdEdit`이 `m_pMakeCellDlg->IsWindowVisible()`이면 열기 거부(모달 중엔 페이지의 MAKE CELL 버튼이 비활성이라 새로 열 수 없음 → 이것으로 충분), ② 방어: 편집기 APPLY가 `GetCellIdCfgSeq()` 변화 시 거부 + `EndDialog(IDCANCEL)`. In `LoadRcpFile` 호출처는 `RecipeMgrDlg.cpp:317`(페이지 LOAD, 모달 중 비활성) 하나뿐, SelectCell/Align/TestMode는 셀 구성을 바꾸지 않음(TestMode는 Out만 재로드) — 확인.
- `SetActiveSegment`는 `m_recipeMgr`만 값 교체한다(`InRecipeFunc.cpp:616-626`) → 세그먼트 인덱스로 직접 여는 `S_CellIdConfig`는 교체 로직 불필요. 단 **`SaveRcpSegments` 도중에는 `m_recipeMgr`가 세그먼트 N-1 사본인데 `m_nActiveSegment`는 그대로**라 `GetSegmentMgr(active)`가 틀린 형상을 준다 → 트레일러 쓰기는 `\tm_recipeMgr = *pStage;\n`(복원, :1062) **뒤**에서 한다.
- MakeCell `BuildGridPreview`(오버레이 미리보기)는 `m_recipeMgr`를 임시로 바꿨다 되돌릴 뿐 → 설정 무관.
- RecipeMgrDlg 그리드는 스틱/셀 수를 바꾸지 않는다(DLL 매퍼 grep). GLASS ROTATE 변경은 Auto 토큰만 바꾼다(수동 ID는 인덱스 고정) → 중복이 생기면 저장 검증·D-15가 잡는다.

---

**(2) 파일 포맷**

*2-1 결정(D4b-1): `RECIPE_FILE_V1012` 신설, 트레일러는 `_HYBRID_END_` 뒤, 새 빌드는 항상 기록*

근거(코드·파일 확인):
| 사실 | 위치 |
|---|---|
| 저장 버전 = `RECIPE_FILE_END` | `RecipeMgrDlg.cpp:451` `WriteInt (&File, RECIPE_FILE_END);`, enum `preDefine.h:104-105`(V1011 = 1011, END = 1012) |
| 꼬리 게이트 `> V1011` | `InRecipeFunc.cpp:149`, `OutRecipeFunc.cpp:132` |
| 실파일 | `D:\HBS\RECIPE\Recipe\` 1012 파일 6개(`1_OCP_TEST_Long_260907`, `1_OCP_TEST_Long_P1RT`, `9_…BADTAG`, `9_…Half_260930`, `…_2`, `9_…LS`) 전부 마지막 줄 `_HYBRID_END_`, 나머지 1005~1009 |
| 꼬리는 위치 고정 태그 엄격 비교 | `LoadHybridTail` In `:734 _HYBRID_`, `:738 _SEGMENT_HEADER_`, `:760 _SEGMENT_`, `:761` 인덱스, `:770 _HYBRID_END_` — 어긋나면 FALSE → 전체 롤백(메모리 = 직전 로드 상태) → `RecipeChange`가 `JOB_RECIPE_NOT_EXIST`(1013) 반환(`RecipeEditMgr.cpp:137-139`, 로그 없음) |
| 그 반환값을 누가 보나 (리뷰 C3/K2) | CIM PPID `CommTcpIpObjectCIM.cpp:1416` = `nRtn` 확인(NG 응답). 사이클 화면 `D_MeasureCycle.cpp:223` `if (0 < OpenRecipe(...))` = **1013도 통과** → 대화상자 닫힘, `SetRecipeID`는 성공 때만(`RecipeEditMgr.cpp:267-270`)이라 **직전 레시피 ID로 사이클**. Recipe 스텝 `DoWorkGlassRecipeStep.cpp:626` = **반환값 무시** → 마지막으로 성공한 메모리 사본으로 진행, D-15도 그 사본을 검사. 기존 결함(Phase 1 BADTAG 파일도 같음)이지만 4b가 모든 저장 파일에 엄격 트레일러를 붙여 실패 면이 늘어난다 → **D4b-15** |
| `_HYBRID_END_` 뒤는 아무도 안 읽음, EOF 읽기 무해 | `LoadRcpFile`이 꼬리 뒤 바로 `File.Close()`; `ReadString`(`Global.cpp:155-160`)은 반환값 무시 → "" |
| ALPHA는 `RECIPE_FILE_*` 미사용 | grep `Source/ALPHA` 0건 |
- §5.7 문구("V1011 꼬리 블록 `_HYBRID_` 뒤 + 세그먼트별 `_CELL_ID_MANUAL_`")를 **꼬리 안**으로 읽으면 현재 배포 빌드가 새 파일을 못 연다(엄격 태그) → 위치만 `_HYBRID_END_` 뒤로 바꾼다(내용·문법 의미는 §5.7 그대로: 레시피 전역 모드 + 세그먼트별 블록 + 스틱마다 A줄·B줄, 빈 토큰 보존). 이것이 §5.1-4 "구 파일·구 빌드 호환" 원칙과 맞는 유일한 배치다.

호환 행렬:
| 읽는 빌드 \ 파일 | ≤1011(꼬리 없음) | 1012(Phase 1~4 꼬리) | 1013(4b, 트레일러) |
|---|---|---|---|
| 구 빌드(Phase 1~4) | OK | OK | **OK** — `> V1011` 꼬리까지 읽고 멈춤, 트레일러 무시 = Auto/Auto. 구 빌드로 재저장하면 트레일러 소실(D-12와 같은 주의: 두 PC 동시 배포) |
| 4b 빌드 | OK, Auto/Auto | OK, `> V1012` 거짓 → Auto/Auto | OK, 트레일러 엄격 파싱(어긋나면 로드 실패·롤백 — DLL LOAD는 실패 팝업, ALPHA는 위 표 "그 반환값을 누가 보나"대로, D4b-15) |
- 비용: 4b 빌드로 저장하면 모든 레시피가 `1012`→`1013` + 트레일러 5줄(Auto/Auto)로 바뀐다. 동작·로그 불변(Phase 1이 N=1에도 꼬리를 항상 쓰게 했을 때와 같은 성격, `InRecipeFunc.cpp:1032` 주석). 바이트 동일 파일이 필요하면 D4b-1 (b).

*2-2 문법 (ASCII, 줄 = `WriteString`/`WriteInt`, 텍스트 모드라 디스크는 CRLF)*
```
_HYBRID_END_                      ; 기존 꼬리 끝 (InRecipeFunc.cpp:1060)
_CELL_ID_PANEL_                   ; [ver > RECIPE_FILE_V1012]
<modeA>                           ; "0" Auto / "1" Manual (문자열로 읽어 정확히 0/1만 허용)
<modeB>
<nBlock>                          ; 0 = 둘 다 Auto (수동 데이터 없음) / nSeg = 세그먼트마다 블록 1개. 그 외 값 = 로드 실패
{ _CELL_ID_MANUAL_
  <k>                             ; 0-기준 세그먼트 인덱스, 블록 순서와 같아야 함
  <nStick>                        ; = GetSegmentStickCount(k) 이어야 함 (형상 확인)
  <nCell>                         ; = GetSegmentCellCount(k) 이어야 함
  { <A 토큰,>×nCell               ; 스틱 i 패널 A. 모든 토큰 뒤에 쉼표(= _CELL_ 줄과 같은 규칙)
    <B 토큰,>×nCell } × nStick    ; Auto 패널 줄은 빈 토큰만 (",,,")
} × nBlock
_CELL_ID_END_
```
- 파서: 기존 `dfcArryParsing`(`Global.cpp:120-131`)은 쉼표를 만날 때만 토큰을 내고 각 토큰을 `Trim()` → **쉼표 종결 토큰이면 빈 토큰이 보존된다**(`",,,"` → 빈 3개, `"X01,,C01,"` → 3개). ICP의 끝 쉼표 없는 줄 + `SplitAgingList`는 이 파서로 마지막 토큰을 잃으므로 옮기지 않는다. 토큰 수 ≠ nCell → 로드 실패.
- 규칙: `(modeA || modeB) != (nBlock > 0)` → 실패. Auto 패널 줄의 토큰은 개수만 검사하고 버린다(불변식, 손편집 파일의 Auto 패널 초안은 남기지 않는다 — ICP X12의 HMI 선택). Manual 패널 토큰은 **내용 검증 없이** 저장(ICP와 같음: 소문자·형식 오류는 편집기 빨강/저장 거부/D-15가 잡는다).
- 모든 태그 검증(I8 원칙). 실패 = `LoadRcpFile` FALSE → 기존 롤백(본문·세그먼트·Cell ID 설정 모두 로드 전 상태).
- **예외 안전(리뷰 K8)**: `LoadRcpFile`은 `catch (CException *e)`만 있다(In `:153`, Out 동일) → `std::bad_alloc`/`std::out_of_range`가 새면 `RecipeChange` 밖으로 튀어 ALPHA가 죽는다. 그래서 `CellId_LoadTail`은 (1) 파일의 `nStick`/`nCell`을 형상(`geo`)과 **먼저** 비교하고 일치할 때만 할당(크기 상한 = 레시피 형상), (2) `std::vector::at` 미사용·모든 인덱스 범위 검사, (3) 본문 전체를 `try { … } catch (std::exception &) { return FALSE; }`로 감싼다(`CFileException` 등 `CException*`은 그대로 바깥 핸들러로 — 객체 `Delete` 책임 유지). `CellId_EnsureShape`/`CellId_Validate`도 `.at` 미사용, 크기는 형상 이하.
- 쓰기: `nBlock = cfg.IsDefault() ? 0 : nSeg`. Manual 패널은 형상 범위 밖이면 "" 기록(형상은 항상 현재 레시피 기준이라 줄 수·토큰 수가 본문과 일치).
- 세그먼트 인덱스·형상 에코를 넣는 이유: ICP는 세그먼트 블록 위치로만 대응해 검증이 없다. HMI는 형상 불일치를 조용히 넘기면 다른 셀에 수동 ID가 붙는다.

*2-3 리터럴 — `9_OCP_TEST_Hybrid_Half_260930`(seg1 `LONG_H1` rot1 5스틱×3셀, seg2 `SHORT_H2` rot2 5스틱×4셀)*

(a) Auto/Auto로 다시 저장했을 때 (파일 1~2행과 끝만 바뀜):
```
_VERSION_
1013
...                                (본문·꼬리 무변경)
_ANGLE_SUBZ_
0.0000
0.0000
_HYBRID_END_
_CELL_ID_PANEL_
0
0
0
_CELL_ID_END_
```
(b) A Manual(Auto 씨앗 + seg1 셀 1 줄에 PREFIX `X`) + B Auto(파생) — (3) 예 S2:
```
_HYBRID_END_
_CELL_ID_PANEL_
1
0
2
_CELL_ID_MANUAL_
0
5
3
X01,B01,C01,
,,,
X02,B02,C02,
,,,
X03,B03,C03,
,,,
X04,B04,C04,
,,,
X05,B05,C05,
,,,
_CELL_ID_MANUAL_
1
5
4
H01,H02,H03,H04,
,,,,
G01,G02,G03,G04,
,,,,
F01,F02,F03,F04,
,,,,
E01,E02,E03,E04,
,,,,
D01,D02,D03,D04,
,,,,
_CELL_ID_END_
```
- 본문 `_CELL_` 줄(seg2는 `E01..` 세그먼트 로컬)은 그대로다(D4b-8). seg2 A 수동 씨앗이 `H01..`인 것은 런타임 Auto(Phase 4 이월)를 씨앗으로 쓰기 때문.

*2-4 N=1 파일*
- **쓴다/읽는다.** 근거: 모드는 레시피 전역(§5.7)이라 N=1 레시피도 Manual을 쓸 수 있어야 하고(ICP도 동일), Phase 1이 이미 N=1에도 꼬리를 항상 쓰는 단일 경로를 택했다. 분기 없는 한 경로가 검증이 쉽다. N=1 Auto/Auto는 트레일러 5줄, 동작·로그 0 변화.

*2-5 로더·라이터 코드 위치(In/Out 동일 앵커, 각 n=1)*
| 목적 | 앵커 | 삽입 |
|---|---|---|
| 백업 | `\tint nBakActive = m_nActiveSegment;\n` (In :127 / Out :110) 뒤 | `\tS_CellIdConfig sBakCid = m_sCellIdCfg;\t// LYS 2026-10-01 Hybrid Phase 4b\n` |
| 구 파일 = Auto/Auto | `\t\tEnsureSegments ();\n` (In :147 / Out :130, 탭 2개는 LoadRcpFile에만) 뒤 | `\t\tm_sCellIdCfg.Clear ();\t// LYS 2026-10-01 Hybrid Phase 4b file <= V1012 -> Auto/Auto\n` |
| 트레일러 읽기 | `\t\tif (bOK && nVersion > RECIPE_FILE_V1011) bOK = LoadHybridTail (&File, nVersion);\n` (In :149 / Out :132) 뒤 | `\t\tif (bOK && nVersion > RECIPE_FILE_V1012) bOK = LoadCellIdTail (&File);\t// LYS 2026-10-01 Hybrid Phase 4b trailer after _HYBRID_END_\n` |
| 복원 | `\t\tm_nActiveSegment = nBakActive;\n` (In :163 / Out :146) 앞 | `\t\tm_sCellIdCfg = sBakCid;\n` |
| 로드 성공 일련번호 | `\t\tm_nActiveSegment = nBakActive;\n\t\treturn FALSE;\n\t}\n` (In :163 / Out :146) 뒤 | In `\tm_nCellIdCfgSeq++;…` / Out `\tm_nRcpLoadSeq++;…` (1-3) |
| 생성자 | `\tm_nActiveSegment = -1;\t// LYS 2026-09-29 Hybrid Phase 1\n` (In :14 / Out :14) 뒤 | In `\tm_nCellIdCfgSeq = 0;…` / Out `\tm_nRcpLoadSeq = 0;…` |
| 트레일러 쓰기(In) | `\tm_recipeMgr = *pStage;\n` (:1062) 뒤 | `\tSaveCellIdTail (pFile);\t// LYS 2026-10-01 Hybrid Phase 4b right after _HYBRID_END_; needs the restored working slot (GetSegmentMgr(active))\n` |
- `LoadCellIdTail` 진입 시: 꼬리가 방금 `m_vecSegments`·`m_recipeMgr = seg0`·`m_nActiveSegment = 0`을 세웠으므로 `GetSegment*` 형상이 정확하다. 1013 파일은 항상 꼬리가 있다(> 1011).

---

**(3) 리졸버·검증**

*3-1 위치 (D4b-11)*
- DLL 내부 신규 `CellIdRule.h/.cpp`(ASCII CRLF, export 없음): 순수 함수. In/Out 메서드는 얇은 래퍼. ALPHA는 Out의 export 메서드만 부른다 → 규칙 코드 사본 1개.
- `CellIdRule.cpp`는 **첫 줄 `#include "pch.h"`**, 다음 `#include "CellIdRule.h"`(리뷰 C8a). x64 구성은 PCH 설정이 없어(`dfcRecipe.vcxproj:146-215`, `Use`는 Win32 `:93/:123`뿐) `pch.h`는 일반 include이고, `CellIdRule.h`는 `SegmentDefine.h`만 끌어오지만 CString·CStdioFile·std::vector를 쓴다 → MFC 헤더를 `pch.h`가 공급해야 한다.
- 글자 접두는 `SegmentDefine.h`의 `inline SegCellIdLetterPrefix`(두 사본)로 옮기고, ALPHA `CRecipeEditMgr::CellIdLetterPrefix`(:417-427) 본문은 `return SegCellIdLetterPrefix (nIndex);`로 바꾼다(같은 알고리즘, 출력 동일 — Phase 4 계획 :2439의 약속 이행). MakeCellID/GenCellID 자체는 손대지 않는다.
```cpp
// CellIdRule.h (DLL internal)
#pragma once
#include "SegmentDefine.h"
// LYS 2026-10-01 Hybrid Phase 4b Cell ID rules shared by C_InRecipeFunc (editor / save) and C_OutRecipeFunc (ALPHA runtime). Pure functions.
struct S_CellIdSegGeo { int m_nRotate; int m_nStick; int m_nCell; };	// GLASS ROTATE, sticks, cells of stick 0 (uniform by file format)
typedef std::vector<S_CellIdSegGeo> CCellIdGeo;
typedef std::vector< std::vector< std::vector<CString> > > CCellIdGrid;	// [seg][stick][cell]

template <class T> void CellId_BuildGeo (T &rcp, CCellIdGeo &geo)		// C_InRecipeFunc / C_OutRecipeFunc (same segment API)
{
	geo.clear ();
	for (int k = 0; k < rcp.GetSegmentLoopCount (); k++) {
		S_CellIdSegGeo g = { rcp.GetSegmentRotate (k), rcp.GetSegmentStickCount (k), rcp.GetSegmentCellCount (k) };
		geo.push_back (g);
	}
}
int		CellId_BOffset (const S_CellIdSegGeo &g);		// GenCellID nMaxTop: rotate 1 -> sticks, else cells of stick 0
CString	CellId_AutoToken (const CCellIdGeo &geo, int nSeg, int nStick, int nCell, int nPanel);	// byte copy of MakeCellID + GenCellID
BOOL	CellId_DeriveB (const CString &strA, int nOffset, CString &strB);
int		CellId_Resolve (const S_CellIdConfig &cfg, const CCellIdGeo &geo, int nSeg, int nStick, int nCell, int nPanel, CString &strTok, BOOL bAutoFallback);
void	CellId_BuildResolved (const S_CellIdConfig &cfg, const CCellIdGeo &geo, int nPanel, CCellIdGrid &out);	// every cell, Auto included
void	CellId_BuildAuto (const CCellIdGeo &geo, int nPanel, CCellIdGrid &out);
int		CellId_TokenMax (int nGlassIdLen);				// < 0 -> M_CELLID_TOKEN_MAX, else max(0, min(M_CELLID_TOKEN_MAX, 31 - 1 - max(len - 1, 0))) (GlassID >= 31 chars -> 0, every token refused)
BOOL	CellId_Validate (const S_CellIdConfig &cfg, const CCellIdGeo &geo, int nTokenMax, std::vector<S_CellIdError> &vecErr, int *pnLongest);
void	CellId_EnsureShape (S_CellIdConfig &cfg, const CCellIdGeo &geo);	// Manual -> [seg][stick][cell] (keeps values), Auto -> empty
BOOL	CellId_LoadTail (CStdioFile *pFile, const CCellIdGeo &geo, S_CellIdConfig &cfg);
void	CellId_SaveTail (CStdioFile *pFile, const CCellIdGeo &geo, const S_CellIdConfig &cfg);
```
- 세그먼트 수 판정 `bHybrid = geo.size() > 1` = ALPHA `GetCellIdLetterBase() >= 0`(`m_nSegCountCache > 1 && GetSegmentLoopCount() > 1`)과 같은 값(캐시가 낡지 않은 한).

*3-2 Auto 토큰 — ALPHA를 바이트 단위로 재현(B3)*
```cpp
CString CellId_AutoToken (const CCellIdGeo &geo, int k, int i, int j, int nPanel)
{
	if (k < 0 || k >= (int)geo.size ()) return CString ();
	const S_CellIdSegGeo &g = geo[k];
	BOOL bHybrid = (geo.size () > 1);
	int nBase = 0;																	// RecipeEditMgr.cpp:359-363 (SegCellIdLetterExtent sum)
	for (int q = 0; q < k; q++) nBase += (geo[q].m_nRotate == 1) ? geo[q].m_nCell : geo[q].m_nStick;
	int nLetter = (g.m_nRotate == 1) ? j : g.m_nStick - i - 1;					// MakeCellID :152-165
	int nNum	= (g.m_nRotate == 1) ? i + 1 : j + 1;
	CString strTok;
	if (bHybrid) strTok.Format (_T("%s%02d"), (LPCTSTR)SegCellIdLetterPrefix (nBase + nLetter), nNum);	// :154 / :161
	else		 strTok.Format (_T("%c%02d"), 65 + nLetter, nNum);											// :156 / :163 (>= 26 -> '[' etc., as today)
	if (nPanel != 1) return strTok;
	CString strTmp = strTok + _T(" "), strBuf;										// GenCellID :224 "%s " then :234-245
	int nMaxTop = CellId_BOffset (g);												// :201-208
	if (bHybrid) {
		int nPfx = 0;
		while (nPfx < strTmp.GetLength () && strTmp[nPfx] >= _T('A') && strTmp[nPfx] <= _T('Z')) nPfx++;
		strBuf.Format (_T("%s%02d"), (LPCTSTR)strTmp.Left (nPfx), nMaxTop + _ttoi (strTmp.Mid (nPfx)));		// :237-242
	}
	else strBuf.Format (_T("%s%02d"), (LPCTSTR)strTmp.Mid (0, 1), nMaxTop + _ttoi (strTmp.Mid (1, 2)));		// :244 (number >= 100 cut to 2 digits, as today)
	return strBuf;
}
```
- 키 `(k, i, j)` = 런타임 `(활성 세그먼트, GetMeasureStick(), pnCellNum[nProbe])`. `iStick`은 모든 MakeCellID 블록에서 `iStick = GetMeasureStick();`(:2240/:3216/:4096/:4750/:5556 → 호출 :2555, :3499/:3503, :4347, :5008, :5807/:5811 — 리뷰 C8c 정정), `pnCellNum`은 `m_nArea -= nArea*iStick`로 만든 스틱 내 셀 인덱스, 회전은 `pReciepInfo->m_sOptionItem.m_nGlsRotate == 1`(:2548-2552) = `GetSegmentRotate(k)`와 같은 필드. GenMeasID는 MakeCellID와 같은 배열을 받는다(:3641 `nUseHead` / :3646 `nUseCELLID`).
- 이 함수는 **검증·편집기 표시·씨앗**에만 쓴다. 런타임 Auto 패널은 지금 코드가 그대로 만든다(대체 금지 — 바이트 동일의 핵심).

*3-3 B 오프셋·A→B 파생*
- `CellId_BOffset(g) = (g.m_nRotate == 1) ? g.m_nStick : g.m_nCell` — HMI `nMaxTop` 규칙(rot3 = 셀 수). ICP Long/Short(rot3 = 스틱 수)는 쓰지 않는다(D4b-6).
- `CellId_DeriveB(A, off)`: A가 정규형(`[A-Z]+[0-9]+`, 트림·대문자 후 원문과 같음) 아니면 실패. 숫자 9자 초과(앞자리 0 포함) 실패. `n = digits + off`, `n > INT_MAX` 실패. 출력 `Format("%s%0*d", letters, len(digits), n)` — **A 자릿수를 최소 폭으로 유지(ICP 그대로, D4b-5)**.
  | A | off | B |
  |---|---|---|
  | `A05` | 5 | `A10` |
  | `A5` | 5 | `A10` (폭 1은 최소 폭) |
  | `A005` | 4 | `A009` |
  | `AB99` | 5 | `AB104` |
  | `A999999999` | 4 | `A1000000003` (11자, ≤ 20 통과) |
  | `x01` / `A-01` / `` / `A0000000001` | 4 | 실패 |

*3-4 해석 — `CellId_Resolve`*
| modeA | modeB | 패널 A | 패널 B |
|---|---|---|---|
| Auto | Auto | 0 (Auto) | 0 (Auto) |
| Manual | Auto | 1, 수동 A | **1, DeriveB(수동 A, BOffset(seg))**, 실패 = "" |
| Auto | Manual | 0 (Auto) | 1, 수동 B |
| Manual | Manual | 1, 수동 A | 1, 수동 B |
- 반환 1 = override(최종 토큰, B 오프셋 포함). 0 = Auto(`bAutoFallback`이면 `strTok = CellId_AutoToken`, 아니면 ""). 범위 밖 인덱스 = 0.
- ALPHA는 반환 1이고 토큰이 비지 않았을 때만 쓴다(빈 토큰은 D-15 검증 없이만 가능 → 원문 경로로 후퇴).

*3-5 검증 — `CellId_Validate`(순수, 오류 순서 고정)*
0. `cfg.IsDefault()` → 즉시 TRUE(오류 0). **Auto/Auto는 검사하지 않는다** → 기존 레시피는 저장 팝업·D-15 로그가 생길 수 없다.
1. Manual 패널 형식(패널 A → B, 세그먼트 → 스틱 → 셀): 트림·대문자 → 빈 값 `EMPTY` / 길이 > nTokenMax `LENGTH` / `[A-Z]+[0-9]+` 아님 또는 정규형 ≠ 원문 `FORMAT`.
2. B 파생(modeA Manual && modeB Auto): 1에서 A가 실패한 셀은 **건너뛴다**(ICP X7 이중 보고 교정 — 건수만 다름). 파생 실패 `DERIVE`(토큰 = A 원문), 파생 길이 > nTokenMax `LENGTH`(패널 B).
3. 합산 중복: 패널 A 전 세그먼트 → 패널 B 전 세그먼트 순으로 해석값(Auto 포함)을 맵에 등록, 빈 값 건너뜀, 이미 있으면 `DUP`(첫 위치 기록).
- `nTokenMax`: DLL 저장·편집기 = `CellId_TokenMax(-1)` = 20. ALPHA D-15 = `CellId_TokenMax(len(GlassID))` = `max(0, min(20, 30 − max(len−1, 0)))`:
  | GlassID | 길이 | 상한 | 패널 |
  |---|---|---|---|
  | 실 GlassID(예 `ABCDEFG1234`) | 11 | 20 | 끝에서 둘째 문자 |
  | 사이클 1번째 글라스, 칸 비움: `gf_MakeMcrID("GLASSID<HHMMSS>B")`(`D_MeasureCycle.cpp:205-211`) | 15 | 16 | **B** |
  | **사이클 2번째부터**: 엔진이 `GLASSID<HHMMSS>`로 덮음(`DoWorkGlassSequenceEngine.cpp:712-723`, 체크섬·B 없음) | **13** | **18** | **A**(끝에서 둘째 = 초 십의 자리 숫자) |
  | GlassID 없음(ALPHA 기동 직후 AgingTest/AgingNew START — 두 화면은 `SetGlassID`를 부르지 않는다) | 0 | 20 | A, 최종 ID = 토큰 + 체크섬 |
  | 31자 이상 | ≥31 | 0(클램프, 리뷰 C7b) | — 모든 Manual 토큰 LENGTH 거부 |
  → 다회 사이클은 글라스 1(15자, B)과 2번째부터(13자, A)의 상한·패널이 다르다: 17~18자 토큰은 글라스 1에서 거부, 19~20자는 실 GlassID 글라스 1 통과 후 2번째 글라스에서 거부될 수 있다(리뷰 C7c).
- `*pnLongest` = override 토큰(수동·파생) 최대 길이(로그용).
- 오류 문구(DLL 한국어, `CellIdEditDlg.cpp`의 static `C_CellIdEditDlg::FormatError/FormatErrors`, CP949):
  - 감싸기 `Segment %d Stick %d Cell %d (%c): %s` (1-기준, Stick = 레시피 스틱 인덱스+1)
  - EMPTY `Cell ID가 비어 있습니다.`
  - LENGTH `Cell ID '%s' 길이가 최대값(%d)을 초과했습니다.`
  - FORMAT `Cell ID 형식이 올바르지 않습니다. (영문 대문자 + 숫자: '%s')`
  - DERIVE `A Manual ID '%s'로부터 B Cell ID를 만들 수 없습니다.`
  - DUP `Cell ID '%s' 중복 (Segment %d Stick %d Cell %d (%c))`
  - 묶음(`FormatErrors(vec, 5)`): 최대 5줄 + `... 외 %d건`
- ALPHA 로그 문구(영어, `DoWorkGlassRecipeStep.cpp` static `CellIdErrText`): `Segment %d Stick %d Cell %d (%c): ` + `empty` / `'%s' longer than %d chars (cell ID max 31, GlassID length %d)` / `'%s' is not A-Z letters + digits` / `B cell ID cannot be derived from A manual ID '%s'` / `duplicate '%s' (Segment %d Stick %d Cell %d (%c))`.

*3-6 작업 예 (`p4b_expect.pl` 출력, GlassID A = `ABCDEFG1234` → 접두 `ABCDEFG123`, B = `ABCDEFG12B5` → 접두 `ABCDEFG12B`)*

S1 Half Auto/Auto (Phase 4 런타임과 같아야 함, H1 13:30 런 `H015 H026 H037 H048`과 일치):
| seg/stick | A 토큰 | B 토큰 |
|---|---|---|
| seg1 s0 / s4 | A01 B01 C01 / A05 B05 C05 | A06 B06 C06 / A10 B10 C10 |
| seg2 s0 / s4 (base 3) | H01..H04 / D01..D04 | H05..H08 / D05..D08 |
- 전송 seg1 s0: A `ABCDEFG123A018 …B019 …C010`, B `ABCDEFG12BA068 …BB069 …BC060`. seg2 s0: A `…H015 H026 H037 H048`, B `ABCDEFG12BH054 H065 H076 H087`. A+B 70개 서로 다름.

S2 A Manual(씨앗 = Auto A) + seg1 셀 1 줄 PREFIX `X`, B Auto(파생):
| seg/stick | A (수동) | B (파생, off seg1 = 5 / seg2 = 4) |
|---|---|---|
| seg1 s0..s4 | X01 B01 C01 … X05 B05 C05 | X06 B06 C06 … X10 B10 C10 |
| seg2 s0..s4 | H01..H04 … D01..D04 | H05..H08 … D05..D08 |
- 전송 seg1 s0 셀 0: A 글라스 `ABCDEFG123X011`(Auto면 `A018`), B 글라스 `ABCDEFG12BX061`. 나머지 셀·seg2는 S1과 같은 문자열(씨앗 = Auto, 파생 B = Auto B). 검증 OK, longest 3.

S3 중복(S2 + seg2 s0 셀 0 A = `X06`) → **2건**:
1. `Segment 1 Stick 1 Cell 1 (B): Cell ID 'X06' 중복 (Segment 2 Stick 1 Cell 1 (A))`
2. `Segment 2 Stick 1 Cell 1 (B): Cell ID 'X10' 중복 (Segment 1 Stick 5 Cell 1 (B))` (seg2 파생 B = X06 + 4)
- 교훈(ICP 예4와 같음): 파생 B는 A 번호 + 스틱/셀 하나 크기라 세그먼트를 넘는 연속 번호는 거의 확실히 겹친다.

S3b 형식·파생·길이(S2 씨앗에서 seg1 s0c0 `x01`, seg1 s1c1 `B-02`, seg2 s4c3 `D0000000004`, seg2 s3c3 `ABCDEFGHIJKLMNOPQ04`(19자)):
- 저장(상한 20): `Segment 1 Stick 1 Cell 1 (A): …형식…('x01')`, `Segment 1 Stick 2 Cell 2 (A): …('B-02')`, `Segment 2 Stick 5 Cell 4 (B): A Manual ID 'D0000000004'로부터…` = 3건(19자 토큰 통과). seg1 s0c0·s1c1의 B 파생은 건너뜀.
- 시작(사이클 기본 GlassID 15자 → 상한 16): 위 + `Segment 2 Stick 4 Cell 4 (A): 'ABCDEFGHIJKLMNOPQ04' longer than 16 chars …` = 4건 → 거부. `ABCDEFG1234`(상한 20)면 3건(형식 오류는 여전히 거부).

S4 B Manual만(씨앗 = Auto B, seg1 셀 1 줄 PREFIX `Y`):
- A 글라스 전송 = S1과 **완전히 같음**(`…A018 B019 C010`, A 패널 Auto = 원문 경로). B 글라스 seg1 s0c0 `ABCDEFG12BY062`.

N=1 `1_OCP_TEST_Long_260907`(rot1 5스틱×20셀, 글자 A..T):
- Auto: A `…A018 B019 … T017`, B `…A068 … T067`(무변경 기준).
- A Manual + 셀 1 줄 PREFIX `Z`: A 글라스 `ABCDEFG123Z013`, B 글라스 `ABCDEFG12BZ063`(파생 Z06). 검증 OK.
- 같은 조작을 PREFIX `P`로 하면 셀 16(`P01..P05`, Auto 씨앗)과 겹쳐 **10건** 중복(A 5 + 파생 B 5) → APPLY 거부. 편집기 주황.

S5 rot0 / rot3 (리뷰 K4 — 실레시피 없음, `p4b_fix/p4b_expect_rot.pl` 합성 형상):
| 형상 | A (Auto) | B (Auto) |
|---|---|---|
| N=1 rot0 4스틱×3셀 | s0 `D01 D02 D03` … s3 `A01 A02 A03` (글자 = 스틱 역순, 번호 = 셀+1) | s0 `D04 D05 D06` … s3 `A04 A05 A06` (off = 셀 수 3) |
| N=1 rot3 4×3 | rot0과 같음 | rot0과 같음 — **ICP Long 규칙(off = 스틱 4)이면 파생 B `D05` ≠ Auto B `D04`** → D4b-6 근거 |
| N=1 rot3, A Manual 씨앗 | A = Auto A | 파생 B = Auto B(`D04..`, 연노랑 없음) |
| N=2 seg1 rot3 3×4 + seg2 rot0 2×5 | seg1 s0 `C01..C04` … s2 `A01..A04` / seg2(base 3) s0 `E01..E05`, s1 `D01..D05` | seg1 `C05..C08` … `A05..A08` / seg2 `E06..E10`, `D06..D10`; A 씨앗 검증 OK(중복 0) |

S6 GlassID 경우(Half, `p4b_expect_rot.pl`):
- GlassID 없음 → 패널 A, 최종 = 토큰 + 체크섬: S1 seg1 s0 `A012 B013 C014`, seg2 s0 `H019 H020 H031 H042`; S2 seg1 s0 `X015 B013 C014`.
- 사이클 2번째 글라스 `GLASSID101530`(13자, A, 상한 18): GenCellID가 마지막 글자를 체크섬으로 보고 버림 → S2 seg1 s0 `GLASSID10153X014 GLASSID10153B012 GLASSID10153C013`.
- 사이클 1번째 `GLASSID101530B3`(15자, B, 상한 16): S1 seg1 s0 `GLASSID101530BA060 …BB061 …BC062`.

---

**(4) 런타임 연결**

*4-1 GenCellID 셀 토큰 교체 (`DoWorkGlassMeasureStep.cpp`, CP949 CRLF 7083)*
- 키는 GenCellID 안에서 모두 얻는다: 세그먼트 = `GetRecipeEditMgrPtr()->GetActiveSegment()`(N≤1 → 0, :328-332), 스틱 = `GetMeasureStick()`(:82), 셀 = `pnCellNum[nProbe]`. 패널 = 기존 :232-234 판정(`strGlassID.Mid(len−2, 1).CompareNoCase("B") == 0`, 빈 GlassID → A) 그대로.
- 변경 4곳(앵커 n=1, 줄 5개 — 4번이 :257·:260 두 줄):
  1. `\t\tstrMcrID.Empty();\n\n\t\tif (pbMeasProbe[nProbe])\n`(:219) — `strMcrID.Empty();` 줄 뒤에 `\t\tbool bCidOverride = false;\t// LYS 2026-10-01 Hybrid Phase 4b manual / derived token this slot\n`(루프 스코프: :257 가드가 `if (pbMeasProbe)` 밖이라).
  2. `\t\t\tstrPart.Format(_T("%s"), strGlassID.Mid(strGlassID.GetLength() - 2, 1));\n`(:232) 뒤:
```cpp
			// LYS 2026-10-01 Hybrid Phase 4b manual / derived cell token (docs/HybridGlass_Porting_Plan.md 5.7): final value, B offset already
			//   inside -> skip the B-panel branch below. Only after the Recipe Change step validated the config (D-15 latch 1); else the original path
			CString strCidTok;
			bCidOverride = m_pShareItem->GetRecipeEditMgrPtr()->GetManualCellToken(strPart.CompareNoCase(_T("B")) == 0, GetMeasureStick(), pnCellNum[nProbe], strGlassID, strCidTok);
			if (bCidOverride) strTmp = strCidTok;
```
     (`strGlassID`는 바로 위 :228에서 `CSystemParamShare::GetGlassID()`로 채워짐 — D-15가 래치에 저장한 값과 같은 출처.)
  3. `\t\t\tif (strPart.CompareNoCase(_T("B")) == 0)\t// AB\n`(:234) → `\t\t\tif (bCidOverride == false && strPart.CompareNoCase(_T("B")) == 0)\t// AB (LYS 2026-10-01 Hybrid Phase 4b: an override token is final)\n`
  4. 두 줄(각 n=1): `\t\tif (bHybridCid && strMcrID.GetLength() > 31)`(:257) → `\t\tif ((bHybridCid || bCidOverride) && strMcrID.GetLength() > 31)\t// LYS 2026-10-01 Hybrid Phase 4b manual / derived token too`, 그리고 `\t\t\tstrCidLog.Format(_T("[HYBRID] cell ID %s longer than 31 - cut to 31"), (LPCTSTR)strMcrID);\n`(:260) → `\t\t\tstrCidLog.Format(bHybridCid ? _T("[HYBRID] cell ID %s longer than 31 - cut to 31") : _T("[CELLID] cell ID %s longer than 31 - cut to 31"), (LPCTSTR)strMcrID);\t// LYS 2026-10-01 Hybrid Phase 4b N=1 manual override -> [CELLID] tag (hybrid text unchanged)\n`(리뷰 C7a). D-15가 31자 초과를 미리 거부하므로 방어용 — `_tcscpy_s` abort 방지, B1.
- `strTmp`는 원래 `"A01 "`(뒤 공백)이고 :253에서 GlassID 접두와 붙은 뒤 `gf_MakeMcrID`가 트림한다(`CommonFunc.cpp:268-282`) → override 토큰(공백 없음)도 같은 최종 형식.
- **MakeCellID(:137-174)와 그 반환 `nMeasStick`은 무변경.** 수동 토큰을 `cCellID[2][32]`에 넣지 않으므로 그 `_tcscpy_s`(:167) 위험도 없다.
- `CRecipeEditMgr`(ALPHA, CP949 CRLF) 추가:
```cpp
// .h  - after "\tvolatile int\t\t\tm_nVisionRcpWaitLatch;" line (:65)
	volatile int			m_nCellIdLatch;	// LYS 2026-10-01 Hybrid Phase 4b D-15: -1 = not checked, 0 = Auto/Auto, 1 = manual config validated, 2 = invalidated mid-glass (warned once)
	int						m_nCellIdLatchSeq;	// LYS 2026-10-01 Hybrid Phase 4b C_OutRecipeFunc::GetRcpLoadSeq() the latch-1 check saw
	CString					m_strCellIdLatchGlass;	// LYS 2026-10-01 Hybrid Phase 4b Glass ID the latch-1 check used
// .h  - after "\tbool\tTakeVisionRcpWait();" line (:135)
	// LYS 2026-10-01 Hybrid Phase 4b Cell ID A/B (docs/HybridGlass_Porting_Plan.md 5.7) - any segment count (the manual mode is recipe-global)
	void	SetCellIdLatch(int nLatch, int nSeq = 0, const CString& strGlassID = CString())	{ m_nCellIdLatchSeq = nSeq; m_strCellIdLatchGlass = strGlassID; m_nCellIdLatch = nLatch; }	// Recipe Change step (D-15)
	int		GetCellIdLatch() const	{ return m_nCellIdLatch; }
	bool	GetManualCellToken(bool bPanelB, int nStick, int nCell, const CString& strGlassID, CString& strToken);	// measure thread: true = final token (manual, or B derived from A)
// .cpp - ctor, after "\tm_nVisionRcpWaitLatch = 0;\t// LYS 2026-10-01 Hybrid Phase 4 (4-7) VISION recipe-done wait not taken\n" (:47)
	m_nCellIdLatch = -1;	// LYS 2026-10-01 Hybrid Phase 4b not checked
	m_nCellIdLatchSeq = 0;
// .cpp - RecipeChange, BEFORE "\tif (m_pRecipeFunc->LoadRcpFile(strRecipeID) != TRUE)\n" (:137, n=1) - review C2: no window where latch 1 sits
//        next to a config that is being replaced, and a failed / rolled-back load also clears it (the Recipe step re-checks)
	m_nCellIdLatch = -1;	// LYS 2026-10-01 Hybrid Phase 4b new recipe / new glass: the Recipe Change step checks again
// .cpp - before "bool CRecipeEditMgr::SaveRecipe(CString strRecipeID)\n" (:472)
// LYS 2026-10-01 Hybrid Phase 4b (5.7) GenCellID hook. Manual / derived token only while the Recipe Change check (D-15) still holds:
//   latch 1 + the same Out load serial (no CIM PPID / DLL TestMode reload since) + the same Glass ID. Latch 0 (Auto/Auto) -> false at once
//   (HARD RULE path). A manual config that is not (or no longer) validated -> one WARN, then Auto IDs for the rest of the glass.
//   Cell -1 (resting head) -> false (Phase 4 "@" behaviour kept)
bool CRecipeEditMgr::GetManualCellToken(bool bPanelB, int nStick, int nCell, const CString& strGlassID, CString& strToken)
{
	strToken.Empty();
	if (m_nCellIdLatch == 0 || m_nCellIdLatch == 2 || m_pRecipeFunc == NULL) return false;
	if (m_nCellIdLatch == 1 && m_pRecipeFunc->GetRcpLoadSeq() == m_nCellIdLatchSeq && strGlassID == m_strCellIdLatchGlass)
	{
		if (nStick < 0 || nCell < 0) return false;
		if (m_pRecipeFunc->ResolveCellToken(GetActiveSegment(), nStick, nCell, bPanelB ? 1 : 0, strToken) != 1) return false;
		return (strToken.IsEmpty() != true);
	}
	if (m_pRecipeFunc->IsCellIdDefault()) return false;	// Auto/Auto in memory: nothing to warn about, no log
	CString strLog;
	strLog.Format(_T("[CELLID] manual cell ID config not validated for this glass (latch %d, recipe load %d/%d, GlassID %s/%s) - Auto cell IDs from here"),
		(int)m_nCellIdLatch, m_nCellIdLatchSeq, m_pRecipeFunc->GetRcpLoadSeq(), m_strCellIdLatchGlass.GetString(), strGlassID.GetString());
	CLogWrapper::GetInstance()->KMLOG(WARN, strLog);
	m_nCellIdLatch = 2;	// once per glass (the next RecipeChange / Recipe Change step resets it)
	return false;
}
```
- 래치 상태와 결과(리뷰 C2/K3/K9):
  | 상황 | 래치 | GenCellID | 로그 |
  |---|---|---|---|
  | Auto/Auto 레시피(N 무관), 정상 글라스 | 0 | 즉시 false → 원문 경로 | 없음 |
  | Manual 검증 통과, 같은 로드·같은 GlassID | 1 | 수동/파생 토큰 | D-15 OPER 1줄(4-3) |
  | 글라스 중간 CIM PPID 재로드(`RecipeChange` → −1) | −1 | Auto | `[CELLID] … not validated …` WARN 1회(설정이 Manual일 때만) |
  | 글라스 중간 DLL 검증 모드 열기(`DlgTestMode.cpp:99-104` `pOut->LoadRcpFile`, `RecipeChange` 안 거침) | 1, 일련번호 불일치 | Auto | WARN 1회 |
  | 다른 GlassID(Recipe 아이템을 건너뛴 경로 — Phase 5 재개 등) | 1, GlassID 불일치 | Auto | WARN 1회 |
  | 같은 글라스 재개(같은 GlassID, 재로드 없음) | 1 유지 | 수동 토큰(올바름) | 없음 |
  | 래치 0인데 글라스 중간 재로드로 Manual 설정이 들어옴 | 0 | Auto(검사한 설정 = Auto/Auto와 일치) | 없음 — 기록 |
  검사 통과 뒤 같은 파일을 다시 읽어도(내용 동일) 일련번호가 바뀌므로 보수적으로 Auto + WARN이 된다(운용 주의: 자동 운전 중 Manual 레시피로 DLL 검증 모드를 열지 말 것).
- 스레드: 측정 스텝 `Run()`은 측정 아이템 스레드, `SegmentSwitchData`(`SetActiveSegment`)도 같은 스레드(`DoWorkGlassMeasure.cpp:329-337`) → GenCellID 시점 활성 세그먼트 안정. `m_nCellIdLatchSeq`/`m_strCellIdLatchGlass`는 Recipe 아이템 스레드가 쓰고 측정 아이템 스레드가 나중에 읽는다(아이템 순차). Out 설정 벡터를 다시 쓰는 곳은 `LoadRcpFile`뿐 — 측정 중 재로드(CIM·TestMode)의 데이터 경합은 `m_vecSegments`/`m_recipeMgr`에 이미 있는 것과 같은 수준이고, 새 판독 경로(`ResolveCellToken`)는 일련번호가 같을 때만 돈다.
- 런타임 연결 방식 = GenCellID가 Out을 직접 조회(D4b-4). 글라스 단위 해석 캐시는 만들지 않는다.

*4-2 N=1 / Auto/Auto 바이트 동일 논증*
| 경로 | Auto/Auto(N=1·N>1) | 근거 |
|---|---|---|
| GenCellID | 래치 0 → `GetManualCellToken` 즉시 false → `bCidOverride == false` → :234 조건값·:257 조건값·:260 문구(`bHybridCid` 선택 = 원문 `[HYBRID]`)가 원문과 같음. 로그 0. (래치 −1 + Auto/Auto(CIM 중간 재로드)도 `IsCellIdDefault()` → false, 로그 0) | 변경 4곳 모두 `bCidOverride`가 false면 원문 식과 동치 |
| Recipe Change | `IsCellIdDefault()` → 래치 0, return true, 로그 0줄 | (4-3) |
| RecipeChange | 래치 −1 대입만(위치만 `LoadRcpFile` 앞) | |
| DLL 로드 | ≤1012: `Clear()` → 기본. 1013: 트레일러 nBlock 0. 성공 시 int 일련번호 ++ | |
| DLL 저장 | 검증 즉시 OK(팝업 없음), 버전 1013 + 트레일러 5줄 | 파일 바이트만(D4b-1) |
| DLL 화면 | Auto/Auto(N 무관) → `GetCellID(i, j)` 그대로 | `GetDisplayCellId`, D4b-7 (a) |
| MakeCell | `ResetCellIdConfig` 기본이면 no-op(세대 ++만), 팝업 없음 | |
| Recipe 스텝 :626 (D4b-15 (b)일 때) | 읽을 수 있는 레시피는 일련번호가 늘어 거부 블록 미진입 → 로그·경로 동일 | (4-3) |
- B Manual만 + A 글라스: A 패널 Resolve 0 → 원문 경로 → 전송 바이트 동일(S4). 추가 로그는 D-15 OPER 1줄뿐.
- Phase 4 불변 규칙 유지: `GetCellIdLetterBase() >= 0`(N>1 신호)을 모드 신호로 쓰지 않는다. 레시피 `m_chCellID`를 런타임에서 다시 쓰지 않는다.

*4-3 D-15 시작 재검증 (`DoWorkGlassRecipeStep.cpp`, CP949 CRLF 1099)*
- 위치: `CDoWorkGlassRecipeStep_Change::Run`(:612) — 하이브리드 블록 닫힘 뒤, `[PRECHECK]` 앞. 앵커 `\t\t\tif (bAlignOK != true)\n\t\t\t{\n\t\t\t\tm_pShareItem->SetFail(true, JOB_RECIPE_NOT_EXIST);\n\t\t\t\treturn false;\n\t\t\t}\n\t\t}\n\t}\n`(n=1, :670-676) 뒤에 삽입(한글 주석 줄 :678은 앵커로 쓰지 않음).
- 하이브리드 블록(`GetSegmentCount() > 1`) **밖**: 모드는 레시피 전역 → N=1 Manual도 검사.
- 이 시점: 레시피를 방금 다시 읽음(:626 `OpenRecipe` → `RecipeChange`, 스킵 코드 주석 처리 :129-133 확인 → 항상 재로드 시도. **실패해도 :626은 반환값을 버리므로 여기서 검사하는 것은 마지막으로 성공한 메모리 사본** — D4b-15), GlassID 확정(CIM은 PPID → LOTID → GLSID 뒤 MEAS_START, 사이클 index>0은 엔진이 아이템 전에 `GLASSID<HHMMSS>` 설정 `DoWorkGlassSequenceEngine.cpp:722`).
- 4개 START 경로(운전 CYCLE `DlgSubOperate.cpp:1790`, CIM MEAS_START :3449, AgingTest :495, AgingNew/EFA :550)가 모두 `EngineStart` → Adsorption → **Recipe** 순서로 여기를 지난다.
- 코드(정적 헬퍼는 `bool CDoWorkGlassRecipeStep_Change::Run()\n`(:612, n=1) 앞):
```cpp
// LYS 2026-10-01 Hybrid Phase 4b D-15 (docs/HybridGlass_Porting_Plan.md 5.7) English text of one Cell ID error for the GLASS log
static CString CellIdErrText(const S_CellIdError& e, int nGlassIdLen)
{
	CString strWhere, strWhy;
	strWhere.Format(_T("Segment %d Stick %d Cell %d (%c)"), e.m_nSeg + 1, e.m_nStick + 1, e.m_nCell + 1, e.m_nPanel ? _T('B') : _T('A'));
	switch (e.m_nCode)
	{
	case M_CELLID_ERR_EMPTY:  strWhy = _T("empty"); break;
	case M_CELLID_ERR_LENGTH: strWhy.Format(_T("'%s' longer than %d chars (cell ID max 31, GlassID length %d)"), e.m_strToken.GetString(), e.m_nLimit, nGlassIdLen); break;
	case M_CELLID_ERR_FORMAT: strWhy.Format(_T("'%s' is not A-Z letters + digits"), e.m_strToken.GetString()); break;
	case M_CELLID_ERR_DERIVE: strWhy.Format(_T("B cell ID cannot be derived from A manual ID '%s'"), e.m_strToken.GetString()); break;
	case M_CELLID_ERR_DUP:    strWhy.Format(_T("duplicate '%s' (Segment %d Stick %d Cell %d (%c))"), e.m_strToken.GetString(),
								  e.m_nSeg2 + 1, e.m_nStick2 + 1, e.m_nCell2 + 1, e.m_nPanel2 ? _T('B') : _T('A')); break;
	default:                  strWhy.Format(_T("error %d"), e.m_nCode); break;
	}
	return strWhere + _T(": ") + strWhy;
}

// LYS 2026-10-01 Hybrid Phase 4b D-15 Cell ID A/B re-check at every auto start, right after the recipe reload (Glass ID final).
//   Auto/Auto (every recipe saved before 4b): latch 0, no log. false = refuse without an alarm (caller SetFail(true, 0))
static bool CheckCellIdConfig(CBaseSequenceImpl* pShare)
{
	CRecipeEditMgr* pRcp = pShare->GetRecipeEditMgrPtr();
	C_OutRecipeFunc* pFunc = pRcp->GetRecipeFuncPtr();
	if (pFunc == NULL || pFunc->IsCellIdDefault())
	{
		pRcp->SetCellIdLatch(0);
		return true;
	}
	CString strGlassID = CSystemParamShare::GetInstance()->GetGlassID();
	CString strGidLog = strGlassID.IsEmpty() ? CString(_T("(none)")) : strGlassID;	// AgingTest / AgingNew START right after boot: no Glass ID (panel A)
	int nGid = strGlassID.GetLength(), nLongest = 0;
	std::vector<S_CellIdError> vecErr;
	CString strLog;
	if (pFunc->ValidateCellIdConfig(nGid, vecErr, &nLongest) != TRUE)
	{
		for (size_t i = 0; i < vecErr.size() && i < 10; i++)
			pShare->GetLogFileAccessorPtr()->KMLOG(WARN, _T("[CELLID] ") + CellIdErrText(vecErr[i], nGid));
		if (vecErr.size() > 10)
		{
			strLog.Format(_T("[CELLID] ... %d more"), (int)vecErr.size() - 10);
			pShare->GetLogFileAccessorPtr()->KMLOG(WARN, strLog);
		}
		strLog.Format(_T("[CELLID] cell ID check failed (%d errors, GlassID %s) - auto start refused"), (int)vecErr.size(), strGidLog.GetString());
		pShare->GetLogFileAccessorPtr()->KMLOG(WARN, strLog);
		pRcp->SetCellIdLatch(-1);
		return false;
	}
	pRcp->SetCellIdLatch(1, pFunc->GetRcpLoadSeq(), strGlassID);	// bound to this load + this Glass ID (GetManualCellToken compares both)
	bool bManA = (pFunc->GetCellIdMode(0) == M_CELLID_MODE_MANUAL), bManB = (pFunc->GetCellIdMode(1) == M_CELLID_MODE_MANUAL);
	bool bPanelB = (strGlassID.Mid(nGid - 2, 1).CompareNoCase(_T("B")) == 0);	// same rule as GenCellID
	strLog.Format(_T("[CELLID] manual cell ID: A %s, B %s, glass panel %c (GlassID %s), longest ID %d/31 chars"),
		bManA ? _T("MANUAL") : _T("AUTO"), bManB ? _T("MANUAL") : (bManA ? _T("AUTO (from A)") : _T("AUTO")),
		bPanelB ? _T('B') : _T('A'), strGidLog.GetString(), ((nGid > 0) ? nGid - 1 : 0) + nLongest + 1);
	pShare->GetLogFileAccessorPtr()->KMLOG(OPER, strLog);
	return true;
}

// Run() - after the hybrid block (anchor above)
	// LYS 2026-10-01 Hybrid Phase 4b D-15 Cell ID A/B re-check (any segment count - the manual mode is recipe-global). Auto/Auto -> no log
	if (CheckCellIdConfig(m_pShareItem) != true)
	{
		m_pShareItem->SetFail(true, 0);	// D-15: no alarm (the Alarm DLL ignores id < 1000) - WARN lines + "Fail Exit" + "The Inspection is Fail.!!"
		return false;
	}
```
- 거부 메커니즘 확인: `return false`만 하면 Idle(:161-178)이 `GetFail() != true`면 Complete를 세워 **거부가 안 된다**. `SetFail(true, 0)` → 엔진(`DoWorkGlassSequenceEngine.cpp:963-972`) OPER `Cycle Seq Item : %s - Fail Exit` + `CAlarmController::SetAlarm(0)`(`AlarmController.cpp:53` → `Alarm_SetAlarm`) → `AlarmManager.cpp:476-480` `if( id < 1000) return FALSE;` → **알람 없음**. 선례 `DoWorkGlassMeasure.cpp:254/:763`. `SetStopUser`는 `ForceStopByUser` 잔존으로 비권장. 글라스는 흡착 상태(기존 Recipe 거부와 같은 부작용).
- 로그 예(S3b 손편집 파일, GlassID `ABCDEFG1234`) — 리뷰 K4로 전체 순서 보강:
```
(GLASS 2_RECIPE_CHANGE.log = Recipe 아이템 로그, m_strSeqName "GLASS 2_RECIPE_CHANGE")
SYST CDoWorkGlassRecipeStep_Change Start.
... ([HYBRID] Segment/capacity/WAD/aging/L3 줄 - N>1 블록, 기존과 같음)
WARN [CELLID] Segment 1 Stick 1 Cell 1 (A): 'x01' is not A-Z letters + digits
WARN [CELLID] Segment 1 Stick 2 Cell 2 (A): 'B-02' is not A-Z letters + digits
WARN [CELLID] Segment 2 Stick 5 Cell 4 (B): B cell ID cannot be derived from A manual ID 'D0000000004'
WARN [CELLID] cell ID check failed (3 errors, GlassID ABCDEFG1234) - auto start refused
WARN CDoWorkGlassRecipe Thread NG, Step = 1, Exit = 3          (DoWorkGlassRecipe.cpp:138; Change = step 1, Idle = exit step 3)
SYST CDoWorkGlassRecipeStep_Idle                               (DoWorkGlassRecipeStep.cpp:173; GetFail() == true -> SetComplete 안 함)
(엔진 로그)
OPER Cycle Seq Item : <Recipe 아이템명> - Fail Exit            (DoWorkGlassSequenceEngine.cpp:965; SetAlarm(0) -> AlarmManager.cpp:476 id < 1000 무시)
```
  `[PRECHECK] measure targets within SW limits` 줄은 **없다**(D-15가 그 앞에서 return). 통과 예(S2, B 글라스): `OPER [CELLID] manual cell ID: A MANUAL, B AUTO (from A), glass panel B (GlassID ABCDEFG12B5), longest ID 14/31 chars`. GlassID 없음: `… glass panel A (GlassID (none)), longest ID 4/31 chars`.

*4-3b D4b-15 (b) 선택 시 — `.rcp`를 못 읽었을 때만 거부* (리뷰 C3/K2)
- 판정 = Out 로드 일련번호가 `OpenRecipe` 전후로 같음(롤백 또는 `File.Open` 실패). 읽을 수 있는 레시피는 `LoadRcpFile` 성공 시 ++되므로 **기본 `.alg` 누락**(`RecipeChange :148-151`이 `.rcp` 성공 뒤 `JOB_RECIPE_NOT_EXIST` 반환, 지금은 진행)을 포함한 기존 경로는 그대로다. 빈 RecipeID(`RecipeChange :123-127` −1, 로드 시도 없음)도 제외.
- Recipe 스텝: `\tm_pShareItem->GetRecipeEditMgrPtr()->OpenRecipe(strPPID); \n`(:626, 끝 공백 포함 n=1) 치환:
```cpp
	// LYS 2026-10-01 Hybrid Phase 4b D4b-15 (b) the .rcp itself could not be read (missing / corrupt, e.g. a broken Cell ID trailer):
	//   OpenRecipe left the previous in-memory recipe -> refuse instead of running it. A readable recipe (load serial advanced) keeps
	//   the original path, including the "base .alg missing" return (JOB_RECIPE_NOT_EXIST after a good .rcp load)
	C_OutRecipeFunc* pLoadFunc = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeFuncPtr();
	int nLoadSeq = (pLoadFunc != NULL) ? pLoadFunc->GetRcpLoadSeq() : 0;
	m_pShareItem->GetRecipeEditMgrPtr()->OpenRecipe(strPPID); 
	if (pLoadFunc != NULL && strPPID.IsEmpty() != true && pLoadFunc->GetRcpLoadSeq() == nLoadSeq)
	{
		CString strRcpLog;
		strRcpLog.Format(_T("[RECIPE] recipe %s could not be read (file missing or corrupt) - auto start refused, the previous in-memory recipe is not used"), strPPID.GetString());
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(WARN, strRcpLog);
		m_pShareItem->SetFail(true, JOB_RECIPE_NOT_EXIST);	// alarm 1013, same as the hybrid .alg refusal (:672)
		return false;
	}
```
- 사이클 화면 `D_MeasureCycle.cpp`(CP949 CRLF 281): `\tif (0 < m_pRecipeEditMgr->OpenRecipe(strSelectRecipeID))\n`(:223, n=1) 치환:
```cpp
	// LYS 2026-10-01 Hybrid Phase 4b D4b-15 (b) 0 < OpenRecipe() also accepted JOB_RECIPE_NOT_EXIST (1013): an unreadable .rcp closed the
	//   dialog and the cycle ran the previous recipe. Refuse only when the .rcp load itself failed (load serial unchanged)
	C_OutRecipeFunc* pLoadFunc = m_pRecipeEditMgr->GetRecipeFuncPtr();
	int nLoadSeq = (pLoadFunc != NULL) ? pLoadFunc->GetRcpLoadSeq() : 0;
	int nOpenRtn = m_pRecipeEditMgr->OpenRecipe(strSelectRecipeID);
	if (pLoadFunc != NULL && strSelectRecipeID.IsEmpty() != true && pLoadFunc->GetRcpLoadSeq() == nLoadSeq)
	{
		AfxMessageBox(_T("레시피 [") + strSelectRecipeID + _T("] 파일을 읽을 수 없습니다.\n파일이 없거나 손상되었습니다. (Cell ID 트레일러 등)\n\n이전 레시피로 측정하지 않도록 시작하지 않습니다."), MB_ICONWARNING);
		return;
	}
	if (0 < nOpenRtn)
```
  (Korean 리터럴은 `use utf8` + `Encode::encode("cp949", …, FB_CROAK)`. 기존 팝업은 영어지만 메모리 규칙 "신규 팝업 한국어"를 따른다.)
- CIM 경로는 이미 `nRtn`을 본다 → 무변경. 수동 측정 `DlgSubManual_Meas.cpp:2092`의 같은 패턴은 Phase 7.
- Phase 4 L3(`[HYBRID] cell ID letters A..%s … longest ID %d/31`, :603-609)는 Auto 글자 기준 그대로 둔다(N>1 블록 내부, 무변경). 수동 모드의 실제 최장은 위 `[CELLID]` 줄이 알려 준다.
- UI 조기 경고(운전 CYCLE/AgingTest/AgingNew 화면 팝업)는 넣지 않는다(D4b-14): 사이클 index>0은 GlassID가 Recipe 스텝 시점에 바뀌고, CIM은 팝업 금지 → 권위 있는 검사는 Recipe 스텝 하나.

*4-4 표시 지점 정리*
| 지점 | 4b | 근거 |
|---|---|---|
| 장치 SEND / dResult / APD / Vision·Angle grab | GenCellID 슬롯 → **자동 반영** | `stLotInfo.m_szCellID`만 읽음(rt 매퍼 §1.6-1.7) |
| L5 `[HYBRID] result row … id(..)` | 자동 반영 | `stLotInfo` 최종 ID |
| WAD Multi Point 로그 :2992-3007 | 무변경(Auto 토큰) | 수동 모드에서만 어긋남, 기록 |
| HeadPair `firstNG` :453 | 무변경(`m_chCellID`) | N=1 로그 보존 |
| DLL HEAD 그리드 열 3(`RecipeMgrDlg.cpp:1653`), SELECT CELL(`SelectCellDlg.cpp:157`), DLL 검증 모드(`DlgTestMode.cpp:168`) | **`GetDisplayCellId(active, i, j)`** — Auto/Auto는 `m_chCellID` 그대로, Manual이면 해석 A | D4b-7 (a) |
| `[HYBRID] rows per segment: … letters %s..%s`(`DoWorkGlassMeasure.cpp:206-220`) | 무변경(Auto 글자) | Manual 모드에선 실제 ID와 다름, 기록 |
| HMI P1~P4 AgingPG(`CommTcpIpObjectAgingPG.cpp:1603-1640`, `:2207-2261`, UTF-8 BOM) | 무변경(독자 `%c%02d` + B 파서 `:2253`) | Manual 레시피도 HMI P1~P4 에이징 결과는 Auto ID. OCP(P5 PG) 무관 |
| `CheckRecipeLayout` 오류 문구(`InRecipeFunc.cpp:1225`) | 무변경(`m_chCellID`) | 레시피 로컬 셀 이름, 기록 |
| DLL MakeCell 오버레이(`GlassMap.cpp:58`) | Phase 6 | 슬롯 ≠ 세그먼트(미리보기 형상), APPLY가 수동 설정을 지움 → 이월 Auto 계산을 슬롯 순서로 새로 해야 함 |
| 운전 화면 N>1 셀맵(`DlgSubOperate.cpp:3976-3979`) / N=1 이름(:3711) | Phase 6 | Phase 6 = 운전 화면 표시. N=1 경로는 Auto/Auto 원문 유지 필수 |
| 수동 측정 `DlgSubManual_Meas.cpp:4663/:4707/:2994-3053` | Phase 7 | D4b-9 |

---

**(5) 편집기 UI — `C_CellIdEditDlg`** (계획 표기 `CCellIdEditDlg` = DLL 명명 규칙 `C_…Dlg`, ICP와 같은 이름)

*5-1 리소스 (`dfcRecipe.rc` UTF-16LE BOM CRLF 370, `resource.h` UTF-16LE BOM CRLF 151)*
- `resource.h`: 앵커 `#define IDD_DLG_TEST_MODE               2006\n`(n=1, :12) 뒤 `#define IDD_CELL_ID_EDIT_DLG            2008\n`(값 41열 정렬). 2008은 모든 이름공간에서 미사용 확인. 컨트롤은 DLL 공용 ID 재사용(`IDC_GRID_1..9` 10401~, `IDC_BUTTON_1` 10201, `IDCANCEL`) — 다른 DLL 팝업과 같은 방식, 새 정의 없음. `_APS_NEXT_*`는 손대지 않는다(VS 편집기 전용).
- 진입 버튼: `IDD_PAGE_RECIPE`에 `IDC_BUTTON_14`(10214, .rc·코드 사용 0건) 재사용. **`IDC_BUTTON_11`은 금지**(`IDC_BUTTON_TEST_MODE`와 같은 10211). 앵커 `IDC_BUTTON_TEST_MODE,641,115,62,26,BS_VCENTER\n`(n=1, :154) 뒤:
  `    PUSHBUTTON      "CELL ID",IDC_BUTTON_14,641,144,62,26,BS_VCENTER\n` (오른쪽 버튼 열 641,28/57/86/115 다음 칸, 641,144 아래 빈 공간 확인)
- 템플릿: 앵커 `    CONTROL         "Custom1",IDC_GRID_14,"MFCGridCtrl",WS_TABSTOP,484,18,139,27\nEND\n`(n=1, Align 다이얼로그 끝 :267-268) 뒤에 빈 줄 + 아래. ICP 689×426 배치, HMI 팝업 스타일(SelectCell과 같은 `DS_FIXEDSYS`, FONT 8):
```
IDD_CELL_ID_EDIT_DLG DIALOGEX 0, 0, 689, 426
STYLE DS_SETFONT | DS_MODALFRAME | DS_FIXEDSYS | WS_POPUP | WS_CAPTION
CAPTION "Cell ID"
FONT 8, "MS Shell Dlg", 400, 0, 0x1
BEGIN
    CONTROL         "Custom1",IDC_GRID_1,"MFCGridCtrl",WS_TABSTOP,7,7,300,14
    CONTROL         "Custom1",IDC_GRID_2,"MFCGridCtrl",WS_TABSTOP,7,21,300,16
    CONTROL         "Custom1",IDC_GRID_3,"MFCGridCtrl",WS_TABSTOP,7,43,675,14
    CONTROL         "Custom1",IDC_GRID_4,"MFCGridCtrl",WS_TABSTOP,7,57,675,16
    CONTROL         "Custom1",IDC_GRID_5,"MFCGridCtrl",WS_TABSTOP,7,80,675,262
    CONTROL         "Custom1",IDC_GRID_6,"MFCGridCtrl",WS_TABSTOP,7,350,300,14
    CONTROL         "Custom1",IDC_GRID_7,"MFCGridCtrl",WS_TABSTOP,7,364,300,16
    CONTROL         "Custom1",IDC_GRID_8,"MFCGridCtrl",WS_TABSTOP,314,350,368,14
    CONTROL         "Custom1",IDC_GRID_9,"MFCGridCtrl",WS_TABSTOP,314,364,368,16
    LTEXT           "빨강 = 빈 값/형식 오류   주황 = 중복   연노랑 = Manual(Auto와 다름)   흰색 = Manual(Auto와 같음)   하늘 = Auto",IDC_STATIC,7,400,535,10
    PUSHBUTTON      "APPLY",IDC_BUTTON_1,552,392,63,27,BS_VCENTER
    PUSHBUTTON      "CLOSE",IDCANCEL,619,392,63,27,BS_VCENTER
END
```
- (선택, VS 편집기 일관성) DESIGNINFO 앵커 `    IDD_ALIGN_RECIPE_DLG, DIALOG\n    BEGIN\n    END\nEND\n`(n=1)의 `END\n` 앞에 `\n    IDD_CELL_ID_EDIT_DLG, DIALOG\n    BEGIN\n    END\n`, AFX_DIALOG_LAYOUT 앵커 `IDD_ALIGN_RECIPE_DLG AFX_DIALOG_LAYOUT\nBEGIN\n    0\nEND\n`(n=1) 뒤 `\nIDD_CELL_ID_EDIT_DLG AFX_DIALOG_LAYOUT\nBEGIN\n    0\nEND\n`. 컴파일에는 불필요.
- 그리드는 전부 `MFCGridCtrl`(`C_GridCtrl` = `dfcGrid/GridCtrlEx.h:5`, AFX_EXT_CLASS) → **DLGINIT 같은 수기 바이너리 불필요**(dfcRecipe에 FPSpread/CListCtrl 없음). 셀 타입 `CGridCellCombo`/`CGridCellButton`은 export되어 이미 사용 중(`RecipeMgrDlg.cpp:579-604, 775-784`).
- 패치 방법(python 고장 → Perl): `p2_patch.pl` 방식 — raw 읽기, `FF FE` 확인, `Encode::decode('UTF-16LE', …, FB_CROAK)`, CRLF 수 = LF 수 확인, LF 정규화, `rep1`(index 2회로 유일성), LF→CRLF, `encode('UTF-16LE')` + BOM, 백업 후 raw 쓰기. 한국어 LTEXT는 스크립트 `use utf8;`. 사후 UTF-16 EOL 계수(`p3d_rc.pl`) + `damaged`(U+FFFD) 0.

*5-2 클래스 (`CellIdEditDlg.h` ASCII CRLF / `CellIdEditDlg.cpp` **CP949** CRLF — 한국어 팝업, D4b-10)*
```cpp
#pragma once
#include "CellIdRule.h"
// LYS 2026-10-01 Hybrid Phase 4b Cell ID A/B Auto/Manual editor (docs/HybridGlass_Porting_Plan.md 5.7, ICP C_CellIdEditDlg).
//   Edits a copy of g_inRcpInfo's S_CellIdConfig only; APPLY validates and commits it, CLOSE drops it. Never touches the recipe grids.
class C_CellIdEditDlg : public C_BaseDialog
{
	DECLARE_DYNAMIC (C_CellIdEditDlg)
	public:
		enum { IDD = IDD_CELL_ID_EDIT_DLG };
		C_CellIdEditDlg (CWnd* pParent = NULL);
		virtual ~C_CellIdEditDlg ();
		static CString	FormatError (const S_CellIdError &e);						// Korean, used by RecipeMgrDlg::SaveFile too
		static CString	FormatErrors (const std::vector<S_CellIdError> &vec, int nMax);	// first nMax + "... 외 %d건"
	protected:
		C_GridCtrl	m_gridCtrlHead, m_gridCtrlData, m_gridStatHead, m_gridStatData, m_gridMap,
					m_gridSelHead, m_gridSelData, m_gridBulkHead, m_gridBulkData;	// IDC_GRID_1 .. 9
		S_CellIdConfig	m_work;
		CCellIdGeo		m_geo;
		CCellIdGrid		m_auto[M_CELLID_PANEL_MAX];	// pure Auto tokens of m_geo, computed in OnInitDialog. Valid while g_inRcpInfo.GetCellIdCfgSeq() == m_nCfgSeq
		int				m_nCfgSeq;					// g_inRcpInfo.GetCellIdCfgSeq() at open: MakeCell APPLY (a modeless popup stays clickable) bumps it -> APPLY refused
		int		m_nSeg, m_nPanel, m_nSelStick, m_nSelCell;
		void	InitControlGrid ();	void InitStatusGrid ();	void InitSelectGrid ();	void InitBulkGrid ();
		void	BuildMap ();		void RefreshView ();	void RefreshSelectGrid ();	void RefreshBulkGrid ();
		void	SetPanelMode (int nMode);
		void	SelectError (const S_CellIdError &e);	// switch segment / panel combos + selection to the error (ICP X6 fix)
		BOOL	GetBulkLineCells (std::vector< std::pair<int, int> > &vecCells, CString &strLine, CString &strError);
		void	ApplyBulkPrefix ();	void ApplyBulkNumbers ();
		CString*	ManualId (int nSeg, int nStick, int nCell);	// NULL when out of shape (ICP X14 guard)
		BOOL	IsManual () { return m_work.m_nMode[m_nPanel] == M_CELLID_MODE_MANUAL; }
		virtual BOOL	OnInitDialog ();
		virtual void	DoDataExchange (CDataExchange* pDX);
		virtual void	OnOK ();	// empty: Enter must not close / apply (APPLY = IDC_BUTTON_1)
		afx_msg void	OnCtrlEndEdit (NMHDR* pNMHDR, LRESULT* pResult);	// GVN_ENDLABELEDIT IDC_GRID_2
		afx_msg void	OnMapClick (NMHDR* pNMHDR, LRESULT* pResult);		// NM_CLICK IDC_GRID_5
		afx_msg void	OnSelEndEdit (NMHDR* pNMHDR, LRESULT* pResult);		// GVN_ENDLABELEDIT IDC_GRID_7
		afx_msg void	OnBulkClick (NMHDR* pNMHDR, LRESULT* pResult);		// NM_CLICK IDC_GRID_9
		afx_msg void	OnBulkEndEdit (NMHDR* pNMHDR, LRESULT* pResult);	// GVN_ENDLABELEDIT IDC_GRID_9
		afx_msg void	OnBtnApply ();										// ON_COMMAND IDC_BUTTON_1
	DECLARE_MESSAGE_MAP ()
};
```
- 생성자 `C_BaseDialog (IDD_CELL_ID_EDIT_DLG, pParent)`. `C_BaseDialog::DoModal`이 `m_bModal`을 세워 `OnOK/OnCancel`이 다이얼로그를 끝낸다(`BaseDialog.cpp:54-58, :95-103`). `.cpp`는 `#include "pch.h"` + `#include "dfcRecipe.h"` + `#include "CellIdEditDlg.h"`(x64는 PCH 미사용이지만 `pch.h`가 일반 include로 MFC·Global·In/Out 헤더를 준다).
- `CellIdEditDlg.h`는 `RecipeMgrDlg.cpp`에서만 include한다(`RecipeManager.cpp`가 `pch.h` 없이 `RecipeMgrDlg.h`를 include — `RecipeMgrDlg.h`에 새 타입 금지).

*5-3 동작*
- `OnInitDialog`: `m_nCfgSeq = g_inRcpInfo.GetCellIdCfgSeq ()`(맨 먼저), `m_work = g_inRcpInfo.GetCellIdConfig ()`, `CellId_BuildGeo (g_inRcpInfo, m_geo)`, `CellId_EnsureShape (m_work, m_geo)`, `m_auto[p] = CellId_BuildAuto (m_geo, p)`, `m_nSeg = GetActiveSegmentIndex ()`(범위 밖 0), 패널 A, 선택 없음 → Init 4개 + `BuildMap` + `RefreshView`.
- 컨트롤 그리드(`IDC_GRID_1/2`, 1×3, 열 폭 `(W−4)/3`, 헤더 `EnableWindow(FALSE)`, 데이터 `SetFixedRowCol(0, 0)`, 콤보 `CBS_DROPDOWNLIST`):
  | 열 | 헤더 | 옵션 | 변경 시 |
  |---|---|---|---|
  | 0 | SEGMENT | `GetSegmentName(k)`(빈 값 → `Segment#%d`) | 같으면 무시 / `m_nSeg` 변경, 선택 해제, `BuildMap`(형상이 달라짐), Refresh |
  | 1 | PANEL | `A`, `B` | `m_nPanel` 변경, 선택 유지, Refresh |
  | 2 | MODE | `AUTO`, `MANUAL` | 같으면 무시. MANUAL→AUTO는 `AfxMessageBox (dfcStr (_T("Panel %c의 Manual Cell ID가 모든 Segment에서 삭제됩니다.\n계속 하시겠습니까?"), …), MB_YESNO \| MB_ICONQUESTION)`, 아니오 → `*pResult = -1`(dfcGrid `OnEndEditCell`이 이전 텍스트로 되돌림, `GridCtrl.cpp:7239-7256` 확인) / `SetPanelMode` |
- `SetPanelMode(mode)`(ICP :533 그대로): 전환 **전** `CellId_BuildResolved (m_work, m_geo, p, seed)` → 모드 대입 → Manual이면 전 세그먼트·스틱·셀 = seed(빈 값이면 `m_auto[p]`; B를 Manual로 바꿀 때 A가 Manual이면 파생 B가 씨앗), Auto면 `m_vecManual[p].clear ()` → Refresh.
- 상태 그리드(`IDC_GRID_3/4`, 열 110/110/110/나머지 px, 편집 불가, 타이틀팁): `PANEL MODE`(`A / MANUAL`), `VALID (%c)`(현재 패널 문자, 전 세그먼트, `%d/%d` — 빈 값 아님·Manual이면 정규형·합산 중복 아님), **`ERROR (A+B)`**(`CellId_Validate (m_work, m_geo, 20, …)` 건수, 양 패널 — ICP X15 혼동을 라벨로 해소, 리뷰 K10), `FIRST ERROR`(`FormatError(vec[0])` 또는 `-`, 좌측 정렬). 오류 있으면 2·3열 배경 빨강.
- 셀 맵(`IDC_GRID_5`, **현재 세그먼트**, 행 = 스틱(레시피 인덱스 순), 열 = 셀 — `SelectCellDlg::SetViewSelectCell :153-157`과 같은 방향): `InitGridUI (m_gridMap, IDC_GRID_5, nStick + 1, nCell + 1, TRUE, TRUE)`(고정 1행 1열), 고정 행 `C1..Cn`, 고정 열 `S%d`(스틱 인덱스+1), 열 0 폭 60 px·나머지 `max(60, (W−64)/nCell)`, 행 높이 `max(22, (H−4)/(nStick+1))`, `SetEditable(FALSE)`, `EnableSelection(FALSE)`, `EnableTitleTips(TRUE)`(작으면 그리드 자체 스크롤). 텍스트 = 현재 패널 해석값(`CellId_BuildResolved`, 파생 B 반영).
- 색(위에서 첫 일치, 중복 맵 = A+B 양 패널 전 세그먼트 해석값):
  | 조건 | 색 |
  |---|---|
  | 빈 값, 또는 (Manual && 비정규형/길이 > 20) | 빨강 `COLOR_RED` RGB(255,0,0) |
  | 합산 중복 | 주황 RGB(255,165,0) |
  | Manual && 값 ≠ `m_auto[p]`(B는 순수 Auto B와 비교) | 연노랑 RGB(255,255,190) |
  | Manual && 같음 | 흰색 `COLOR_WHITE` |
  | Auto(파생 B 포함) | 하늘 RGB(204,229,255) |
  | 선택 셀 | 글자 `COLOR_BLUE`, 그 외 `COLOR_BLACK` |
  (COLOR_* = `dfcRecipe/Global.h:23-35`, 나머지 3색은 `.cpp` 로컬 `const COLORREF`.)
- 선택(`IDC_GRID_6/7`, 1×4: `STICK`(`%d (#%d)` = 인덱스+1, `m_nStickNum`) / `CELL`(j+1) / `AUTO`(그 패널 순수 Auto, ICP에 없는 보조 열) / `CELL ID`): CELL ID 편집 가능 = 선택 있음 && Manual, 아니면 읽기 전용 + `COLOR_GRAY_BRIGHT`. `OnSelEndEdit`: `Trim()`+`MakeUpper()` 후 **잘못돼도 저장**(빨강 표시용, ICP) → Refresh.
- 일괄 입력(`IDC_GRID_8/9`, 1×5: `LINE`(나머지 ≥80) / `PREFIX`(80) / [`APPLY PREFIX`](120, `CGridCellButton`+`GVIS_READONLY`) / `START NO.`(80) / [`APPLY NUMBERS`](120)):
  - 대상 줄 = 선택 셀과 **같은 Auto 글자**를 쓰는 같은 세그먼트 셀, **Auto 번호 오름차순**(ICP Deg0=행/Deg90=열의 HMI 번역, 전 rot 지원 — ICP의 Deg180/−90 거부 문구는 옮기지 않음, X5):
    - rot == 1(글자 = 셀): 같은 셀 인덱스의 스틱 0..N−1 = 그리드 **열** 위→아래. `LINE` = `CELL %d : STICK 1->%d (%d)`.
    - rot ≠ 1(글자 = 스틱): 선택 스틱의 셀 0..M−1 = 그리드 **행** 왼→오. `LINE` = `STICK %d : CELL 1->%d (%d)`.
    - 이렇게 정하면 START `1` = Auto 번호와 같아진다(Half seg1 셀 1 줄 → `X01..X05`).
  - PREFIX/START 편집 가능 = 줄 있음 && Manual. 버튼은 Manual 아니면 `일괄 입력은 Manual 모드에서만 사용할 수 있습니다.`
  - APPLY PREFIX: `Trim`+`MakeUpper`, 비었거나 `[A-Z]`만이 아니면 `Prefix는 영문 대문자만 입력할 수 있습니다.` 셀마다 숫자 = 지금 수동 ID의 끝 숫자열, 없으면 그 패널 순수 Auto 토큰의 끝 숫자열(B는 B Auto 번호 — ICP handoff §2.7).
  - APPLY NUMBERS: `NormalizeBulkStart`(트림, 정확히 1자리 `1`~`9`면 앞에 `0`), 비었거나 9자 초과·숫자 아님·≤0이면 `Start Number는 9자리 이하의 양의 정수만 입력할 수 있습니다.`, `start + n − 1 > 999999999`면 줄 전체 사전 거부 `마지막 번호가 9자리를 초과합니다.` 폭 = `max(2, len(start))`, 글자 = 지금 수동 ID 앞 글자열, 없으면 순수 Auto 글자. `OnBulkEndEdit`: START 칸을 정규화 결과로 다시 씀.
  - 선택 없음: `대상 Cell을 먼저 선택하십시오.`
- APPLY(`OnBtnApply`): **먼저** `if (g_inRcpInfo.GetCellIdCfgSeq () != m_nCfgSeq)` → `AfxMessageBox (_T("편집 중에 레시피 셀 구성 또는 Cell ID 설정이 바뀌었습니다. (MAKE CELL 적용 등)\n편집 내용을 적용하지 않고 닫습니다. CELL ID 편집기를 다시 여십시오."), MB_ICONWARNING)` + `EndDialog (IDCANCEL)` + return(리뷰 C1/K1 방어선 ②; 형상 비교만으로는 같은 크기 재생성을 못 잡는다). 다음 `CellId_Validate (m_work, m_geo, CellId_TokenMax (-1), vecErr, NULL)` 실패 → `SelectError (vecErr[0])`(세그먼트·패널 콤보와 선택을 첫 오류로 이동 + `EnsureVisible`) + `AfxMessageBox (dfcStr (_T("Cell ID 설정이 올바르지 않습니다. (%d건)\n%s"), n, FormatError (vecErr[0])))` → 머무름. 통과 → `g_inRcpInfo.SetCellIdConfig (m_work)` → `C_BaseDialog::OnOK ()`. **파일은 저장하지 않는다**(SAVE가 재검증).
- CLOSE/Esc = `OnCancel` → 작업 사본 폐기.

*5-4 RecipeMgrDlg 진입 (`RecipeMgrDlg.cpp/.h`, CP949 CRLF)*
- `#include "RecipeMgrDlg.h"\n`(n=1, :3) 뒤 `#include "CellIdEditDlg.h"\t// LYS 2026-10-01 Hybrid Phase 4b\n`.
- 메시지 맵 `\tON_COMMAND(IDC_BUTTON_TEST_MODE, OnBtnTestMode)\t// LYS 2026-07-02\n`(n=1, :123) 뒤 `\tON_COMMAND (IDC_BUTTON_14, OnCellIdEdit)\t// LYS 2026-10-01 Hybrid Phase 4b\n`.
- `.h` `\tafx_msg void OnBtnTestMode();\t// LYS 2026-07-02 TestMode dialog\n`(n=1, :199) 뒤 `\tafx_msg void OnCellIdEdit ();\t// LYS 2026-10-01 Hybrid Phase 4b Cell ID A/B editor (modal)\n`.
- 핸들러(`OnBtnTestMode` 본문 블록(n=1, :2325-2330) 뒤):
```cpp
// LYS 2026-10-01 Hybrid Phase 4b Cell ID A/B editor (docs/HybridGlass_Porting_Plan.md 5.7). No GetRecipeView(): GetHeadMoveData would copy
//   stick 0's HEAD Y/X into every stick (ICP 8d162da defect). Only the option grid (GLASS ROTATE) is flushed so the Auto preview matches the screen.
void	C_RecipeMgrDlg::OnCellIdEdit ()
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState ());	// first DLL dialog opened with DoModal (all others are modeless Create) - make the template lookup explicit
	if (GetRecipeName ().IsEmpty ()) { AfxMessageBox (_T("로드된 레시피가 없습니다.")); return; }
	if (g_inRcpInfo.GetSegmentStickCount (g_inRcpInfo.GetActiveSegmentIndex ()) <= 0) { AfxMessageBox (_T("Cell 정보가 없습니다.")); return; }
	// review C1/K1: MakeCell is a modeless popup the modal editor does not disable - its RECIPE APPLY would reset the Cell ID config under the editor
	if (m_pMakeCellDlg && m_pMakeCellDlg->IsWindowVisible ()) { AfxMessageBox (_T("MAKE CELL 창이 열려 있습니다.\nMAKE CELL 창을 닫은 뒤 CELL ID 편집기를 여십시오."), MB_ICONWARNING); return; }
	GetInspectOptionData ();	// active segment option item only (RecipeMgrDlg.cpp:620-632)

	C_CellIdEditDlg dlg (this);
	INT_PTR nRet = dlg.DoModal ();
	if (nRet == -1) {	// template / window creation failed (review C5/K7) - not the same as CLOSE
		TRACE (_T("C_CellIdEditDlg::DoModal failed (IDD_CELL_ID_EDIT_DLG)\n"));
		AfxMessageBox (_T("CELL ID 편집기를 열 수 없습니다. (IDD_CELL_ID_EDIT_DLG)"), MB_ICONWARNING);
		return;
	}
	if (nRet != IDOK) return;

	// HEAD grid CELL ID column only - SetHeadMoveData() would discard unsaved HEAD Y/X edits
	int nSeg = g_inRcpInfo.GetActiveSegmentIndex ();
	for (int i = 0; i < m_gridHeadMoveData.GetRowCount () && i < g_inRcpInfo.GetCellCount (0); i++)
		m_gridHeadMoveData.SetItemText (i, 3, g_inRcpInfo.GetDisplayCellId (nSeg, 0, i));
	m_gridHeadMoveData.Invalidate ();
}
```
- 버튼은 `UpdateUI`(300 ms) `EnableControls(FALSE)`로 ALPHA 레시피 명령 중 자동 비활성(`BaseDialog.cpp:60-93`). 스타일은 `검증 모드` 버튼처럼 기본(`C_ButtonEx` DDX 없음).
- 모듈 상태(리뷰 C5/K7 정정): dfcRecipe x64 구성은 `_AFXEXT;_USRDLL;_WINDLL`을 함께 정의한다(`dfcRecipe.vcxproj:155` Debug|x64, `:213` Release|x64 — 순수 `_USRDLL` 아님)지만 `CdfcRecipeApp theApp`(`dfcRecipe.cpp:66`)을 가진 regular DLL 구조이고, 레시피 페이지는 `Recipe_Initialize`의 `AFX_MANAGE_STATE (AfxGetStaticModuleState ())`(`API_RECIPE.cpp:25`) 아래에서 만들어진다. `AfxGetStaticModuleState` 패턴은 이 DLL에서 이미 컴파일된다(`API_RECIPE.cpp:19-80`). DLL의 기존 `DoModal`은 전부 `CFileDialog`(시스템 템플릿)라 **DLL 템플릿 모달은 첫 사례** → 핸들러 첫 줄에 `AFX_MANAGE_STATE`를 무조건 둔다. 2008은 ALPHA·모든 CoreFunction DLL의 `resource.h`에서 미사용이라 모듈 상태가 틀려도 남의 템플릿을 열 수는 없고 `DoModal`이 −1을 돌려준다 → 위 −1 팝업으로 드러난다. OOS O2 유지.
- 저장 검증(`SaveFile`, 앵커 `\tCString strPath;\n\tstrPath = theApp.m_strRcpFolder + strRcpName + _T(".rcp");\n\n\tdfcCreateDirectory (strPath);\n`(n=1, :440) 앞):
```cpp
	// LYS 2026-10-01 Hybrid Phase 4b Cell ID A/B check before the file is opened (format, B derivation, A+B duplicates over all segments).
	//   Auto/Auto (every recipe saved before 4b) is not checked
	{
		std::vector<S_CellIdError> vecCid;
		if (g_inRcpInfo.ValidateCellIdConfig (-1, vecCid) != TRUE) {
			AfxMessageBox (_T("레시피 저장 불가 (Cell ID 검사)\n\n") + C_CellIdEditDlg::FormatErrors (vecCid, 5), MB_ICONWARNING);
			return FALSE;
		}
	}
```
  `OnRecipeSave/SaveAs`는 `SaveFile` 전에 `GetRecipeView()`를 부르므로(:357/:401) 검증은 그리드 flush 뒤 회전값을 본다. 실패 시 `WritePPIDFile`도 안 돈다(:370 기존 구조).
- HEAD 그리드 표시(`\t\tm_gridHeadMoveData.SetItemText (i, 3, dfcStr (sCell.m_chCellID));\n`, n=1, :1653) → `\t\tm_gridHeadMoveData.SetItemText (i, 3, g_inRcpInfo.GetDisplayCellId (g_inRcpInfo.GetActiveSegmentIndex (), nStick, i));\t// LYS 2026-10-01 Hybrid Phase 4b\n`. 열 3은 읽어 들이지 않는다(:1669 주석) → 저장 무영향.

*5-5 `GetDisplayCellId` (In)*
```cpp
// LYS 2026-10-01 Hybrid Phase 4b DLL screens: Auto/Auto (any segment count, every recipe saved before 4b) -> the recipe's m_chCellID exactly
//   as before (HARD RULE, D4b-7 (a)); a manual config -> the panel-A token the runtime sends (manual A, else the MakeCellID token with the
//   Phase 4 letter carry). nSeg = the active segment (GetCellID reads m_recipeMgr = the active segment)
CString	C_InRecipeFunc::GetDisplayCellId (int nSeg, int nStick, int nCell)
{
	if (m_sCellIdCfg.IsDefault ()) return GetCellID (nStick, nCell);
	CString strTok;
	ResolveCellToken (nSeg, nStick, nCell, 0, strTok, TRUE);
	return strTok;
}
```
  (D4b-7 (b)를 고르면 첫 줄만 `if (GetSegmentLoopCount () <= 1 && m_sCellIdCfg.IsDefault ())`로 — N>1 Auto/Auto도 이월 글자 표시.)
- `SelectCellDlg.cpp:157` `\t\t\tm_gridSelSpec.SetItemText (i, j, g_inRcpInfo.GetCellID (i, j));\n` → `\t\t\tm_gridSelSpec.SetItemText (i, j, g_inRcpInfo.GetDisplayCellId (g_inRcpInfo.GetActiveSegmentIndex (), i, j));\t// LYS 2026-10-01 Hybrid Phase 4b\n`. DLL `DlgTestMode.cpp:168`(**LF-only**, 공백 없는 스타일) `\t\t\tm_gridGlass.SetItemText(i, j, g_inRcpInfo.GetCellID(i, j));\n` → `\t\t\tm_gridGlass.SetItemText(i, j, g_inRcpInfo.GetDisplayCellId(g_inRcpInfo.GetActiveSegmentIndex(), i, j));\t// LYS 2026-10-01 Hybrid Phase 4b\n`(파일 스타일 유지 — 사후 개수는 정규식 `GetDisplayCellId\s*\(`로, 리뷰 C8b).

*5-6 MakeCell APPLY 초기화 (`MakeCellDlg.cpp`, CP949 CRLF)*
- 비하이브리드(N=1 포함): `\tif (bCollapse) g_inRcpInfo.CollapseToActiveSegment ();\n`(n=1, :438, 레이아웃 실패 return 뒤) 뒤.
- 하이브리드: `\tfor (int k = 0; k < nNew; k++) m_vecSegInput[k].bOpened = FALSE;`(ASCII 부분 n=1, :1257 — `ReplaceSegments` 실패 return 뒤) **앞**(줄 시작 앵커).
```cpp
	// LYS 2026-10-01 Hybrid Phase 4b rebuilt cells are not proven to be the same physical cells (ICP 4.13, plan 5.7) -> Cell ID back to Auto/Auto
	if (g_inRcpInfo.ResetCellIdConfig ()) AfxMessageBox (_T("셀 구성이 바뀌어 Cell ID Manual 설정(A/B 모드·수동 ID)을 초기화했습니다.\nCELL ID 편집기에서 다시 입력하십시오."), MB_ICONINFORMATION);
```
- `\tLoadSegInput (m_nSegView);\n\tUpdateHybridUI ();\n`은 n=3이라 쓰지 않는다. 기본 설정이면 `ResetCellIdConfig` FALSE → 팝업 없음(D4b-13). 반환값과 무관하게 `m_nCellIdCfgSeq`는 매 호출 ++(셀이 재생성됐다는 신호 — 편집기 APPLY 방어선 ②, 리뷰 C1).
- `ResetCellIdConfig` 본문: `BOOL bWas = !m_sCellIdCfg.IsDefault (); m_sCellIdCfg.Clear (); m_nCellIdCfgSeq++; return bWas;`. `SetCellIdConfig`: `m_sCellIdCfg = cfg; CellId_EnsureShape (m_sCellIdCfg, geo현재); m_nCellIdCfgSeq++;`.

*5-7 vcxproj / filters (UTF-8 BOM CRLF)*
- `dfcRecipe.vcxproj`: `    <ClCompile Include="GlassMap.cpp" />\r\n`(n=1, :234) 뒤 `    <ClCompile Include="CellIdRule.cpp" />\r\n`(4b-0)·`    <ClCompile Include="CellIdEditDlg.cpp" />\r\n`(4b-2), `    <ClInclude Include="GlassMap.h" />\r\n`(n=1, :253) 뒤 `.h` 두 개. 등록과 파일 생성은 같은 스테이지(없는 파일 등록 = 빌드 실패).
- `.filters`(리뷰 C8d — 필터 위치 정정): `CellIdEditDlg.cpp/.h`는 `    <ClCompile Include="GlassMap.cpp">`(n=1, :78) / `    <ClInclude Include="GlassMap.h">`(:134) 블록 3줄 복제(필터 `소스 파일\Dlg`), `CellIdRule.cpp/.h`는 `    <ClCompile Include="InRecipeFunc.cpp">`(n=1, :84) / `    <ClInclude Include="InRecipeFunc.h">`(n=1, :140) 블록 복제(필터 `소스 파일\Func`). 한글 필터명 바이트는 원본 블록에서 raw 복제(`mcoverlay_patch.pl` 방식). 필터는 표시용 — 빌드 무영향.

---

<details><summary>(6) 접점 표 P1–P21 (설계 시점 원문, 펼치기)</summary>

**(6) 접점 표** (DLL 줄 = HEAD, ALPHA 줄 = 작업 트리. 앵커는 `p4b_plan/anchors.txt`로 n 확인)

| # | file | function / anchor | 변경 | N=1 / Auto-Auto 영향 |
|---|---|---|---|---|
| P1 | `dfcRecipe/preDefine.h` + `Source/Include/preDefine.h` | `\tRECIPE_FILE_END,\n` (:105) 앞 | `\tRECIPE_FILE_V1012,\t\t// LYS 2026-10-01 Hybrid Phase 4b Cell ID A/B trailer (_CELL_ID_PANEL_ ... _CELL_ID_END_ after _HYBRID_END_)\n` (ASCII 주석). 두 사본 `cmp` 0 | 저장 버전 1013. 구조체 무변경, ALPHA 미사용 |
| P2 | `dfcRecipe/SegmentDefine.h` + `Source/Include/SegmentDefine.h` | `#pragma once\n` 뒤 / struct 끝 `\t\tZeroMemory (m_chWADName, …);\n\t}\n};\n` 뒤 | `<vector>`, `M_CELLID_*`, `S_CellIdConfig`, `S_CellIdError`, `inline SegCellIdLetterPrefix` (1-2) | 없음 |
| P3 | `dfcRecipe/OutRecipeFunc.h` + `Source/Include/OutRecipeFunc.h` | `\n};` (:79→80) | 끝 블록(1-3) | ALPHA 재컴파일, 오프셋 불변 |
| P4 | `dfcRecipe/InRecipeFunc.h` | `\t\tCString CheckRecipeLayout (…);\n` (:111) 뒤 | In 블록(1-3) | 없음 |
| P5 | 신규 `dfcRecipe/CellIdRule.h/.cpp` | — | 순수 함수(3-1~3-5, 2-2 Load/Save) | 호출 시에만 |
| P6 | `dfcRecipe/OutRecipeFunc.cpp` | `#include "OutRecipeFunc.h"\n` 뒤 include / 2-5 표 6곳(생성자 :14, 로드 성공 일련번호 포함) / EOF에 메서드 본문(+ `GetRcpLoadSeq`) | 로드·접근자·검증·일련번호 | 구 파일 Auto/Auto |
| P7 | `dfcRecipe/InRecipeFunc.cpp` | include / 2-5 표 7곳(생성자·일련번호 포함) / EOF 본문(+ Reset/Get/Set/Display/SaveTail/`GetCellIdCfgSeq`) | 로드·저장·편집 API | 저장 시 트레일러 5줄 |
| P8 | `dfcRecipe/RecipeMgrDlg.cpp` | :3 include / :123 메시지 맵 / :440 앞 저장 검증 / :2325-2330 뒤 핸들러(`AFX_MANAGE_STATE`, MakeCell 열림 거부, `DoModal` −1 팝업) / :1653 HEAD 열 | 5-4 | Auto/Auto 검증 스킵, Auto/Auto 표시 동일(N 무관) |
| P9 | `dfcRecipe/RecipeMgrDlg.h` | :199 뒤 | `afx_msg void OnCellIdEdit ();` | 없음 |
| P10 | `dfcRecipe/MakeCellDlg.cpp` | :438 뒤 / :1257 앞 | `ResetCellIdConfig` + 조건부 팝업 | 기본이면 no-op |
| P11 | `dfcRecipe/SelectCellDlg.cpp` | :157 | `GetDisplayCellId` | Auto/Auto 동일(N 무관) |
| P12 | `dfcRecipe/DlgTestMode.cpp` (**LF-only**) | :168 | `GetDisplayCellId(` (공백 없는 파일 스타일) | 동일 |
| P13 | 신규 `dfcRecipe/CellIdEditDlg.h/.cpp` | — | 편집기(5-2, 5-3; `m_nCfgSeq` APPLY 검사, `ERROR (A+B)`) | 열 때만 |
| P14 | `dfcRecipe/dfcRecipe.rc` (UTF-16LE) | :154 뒤 버튼 / :267-268 뒤 템플릿 / (선택) DESIGNINFO·LAYOUT | 5-1 | 버튼 1개 추가(화면) |
| P15 | `dfcRecipe/resource.h` (UTF-16LE) | :12 뒤 | `IDD_CELL_ID_EDIT_DLG 2008` | 없음 |
| P16 | `dfcRecipe/dfcRecipe.vcxproj` / `.filters` | :234/:253 / :78/:134 | 4개 파일 등록 | 없음 |
| P17 | `Source/ALPHA/Recipe/RecipeEditMgr.h` | :65 뒤 / :135 뒤 | `m_nCellIdLatch`·`m_nCellIdLatchSeq`·`m_strCellIdLatchGlass`, Set(latch, seq, GlassID)/Get, `GetManualCellToken(…, strGlassID, …)` | `.h` → 넓은 재컴파일 |
| P18 | `Source/ALPHA/Recipe/RecipeEditMgr.cpp` | :47 뒤 / **:137 앞**(`\tif (m_pRecipeFunc->LoadRcpFile(strRecipeID) != TRUE)\n`, n=1) / :472 앞 / :417-427 본문 | 래치 리셋 2곳(생성자, `LoadRcpFile` 앞), 함수(일련번호·GlassID 비교 + WARN 1회), `CellIdLetterPrefix` → `SegCellIdLetterPrefix` forward(D4b-11) | 출력 동일 |
| P19 | `Source/ALPHA/DoWork/DoWorkGlassMeasureStep.cpp` | :219 / :232 / :234 / :257 / :260 | 4-1 | 조건식·문구 동치(래치 0) |
| P20 | `Source/ALPHA/DoWork/DoWorkGlassRecipeStep.cpp` | :612 앞 헬퍼 2개 / :670-676 뒤 호출 / (D4b-15 (b)) :626 치환 | 4-3, 4-3b | Auto/Auto 로그 0, 읽을 수 있는 레시피 경로 동일 |
| P21 | (D4b-15 (b)일 때만) `Source/ALPHA/D_MeasureCycle.cpp` (CP949 CRLF 281) | :223 치환 | 4-3b | 읽을 수 있는 레시피는 기존 `0 < nOpenRtn` 판정 그대로 |
- 무변경 확인: `S_CellInfo`/`S_RecipeMgr`(preDefine 구조체), MakeCellID, `CommTcpIpObject*`(AgingPG 포함), dResult, `DlgSubOperate.cpp`, `DlgSubManual_Meas.cpp`, `GlassMap.*`, ALPHA `DlgTestMode.cpp`, `DoWorkGlassMeasure.cpp`, `ShareMem.h`, `ALPHA.rc`/`resource.h`, (D4b-15 (a)이면) `D_MeasureCycle.cpp`.
- 사후 `post` 개수(정규식, 공백 허용 `\s*\(`): `GetManualCellToken\s*\(` MeasureStep 1 / RecipeEditMgr.cpp 1(정의) / `.h` 1; `CheckCellIdConfig\s*\(` RecipeStep 2(정의+호출); `bCidOverride` MeasureStep 5; `ResetCellIdConfig\s*\(` MakeCellDlg 2; `GetDisplayCellId\s*\(` RecipeMgrDlg 2(:1653 + 핸들러), SelectCell 1, DLL TestMode 1; `RECIPE_FILE_V1012` In 1 / Out 1 / preDefine 각 1; `LoadCellIdTail` In 2 / Out 2(+ `.h` 각 1); `GetRcpLoadSeq\s*\(` Out `.h` 각 1 / Out `.cpp` 1 / RecipeEditMgr.cpp 2 / RecipeStep 1(+2 D4b-15 (b)) / D_MeasureCycle 0(+2 (b)); `m_nRcpLoadSeq` Out `.cpp` 3(생성자·++·getter); `m_nCellIdCfgSeq` In `.cpp` 5(생성자·로드 ++·Reset·Set·getter); `GetCellIdCfgSeq\s*\(` CellIdEditDlg.cpp 2(OnInitDialog·APPLY); `m_nCellIdLatch = -1;` RecipeEditMgr.cpp 2; `IsWindowVisible` RecipeMgrDlg.cpp +1.

</details>

---

**(7) 결정** (권장안 **굵게**. 확정 사항 — D-5, D-15, §5.7의 모델·리졸버 의미·검증 3규칙·MakeCell 초기화·편집기 GetRecipeView 금지·세그먼트별 그리드 1차, HMI 패널 판정 규칙 — 은 다시 열지 않는다)

| # | 항목 | 선택지 | 권장 |
|---|---|---|---|
| D4b-1 | 파일 버전·배치 | **(a) `RECIPE_FILE_V1012` 신설(END 1013) + `_HYBRID_END_` 뒤 트레일러, 4b 빌드는 항상 기록(N=1 포함), 게이트 `> V1012`** / (b) 버전 유지, 비기본일 때만 트레일러, EOF 탐지 로드 / (c) §5.7 문구대로 꼬리 안(`_HYBRID_` 뒤) | **(a).** CLAUDE.md 규칙("새 필드 = 새 V 상수, `>` 게이트")과 Phase 1 선례(N=1도 꼬리 항상)를 따르고 구 빌드가 새 파일을 연다(트레일러 무시). (c)는 구 빌드의 엄격 태그(`LoadHybridTail :738/:760/:770`)에서 로드 실패 + 1012 파일 판별에 어차피 버전 신설 필요. (b)는 Auto/Auto 파일 바이트까지 같지만 버전 규칙에서 벗어나고 다음 필드가 이 트레일러 뒤 EOF 탐지에 묶인다. 비용 = 재저장 시 버전 줄 + 5줄 |
| D4b-2 | 트레일러 형태 | **(a) 모드 2줄 + nBlock(0 \| nSeg) + 세그먼트 블록(인덱스·스틱 수·셀 수 에코, 스틱마다 A줄·B줄, 쉼표 종결 토큰)** / (b) ICP처럼 Auto/Auto도 블록 전부(쉼표만 있는 줄) / (c) Manual 패널만 블록 | **(a).** Auto/Auto 5줄, 형상 불일치를 로드에서 잡는다, 기존 `dfcArryParsing` 그대로(빈 토큰 보존), §5.7 "스틱마다 A줄·B줄" 유지 |
| D4b-3 | 토큰 길이 상한 | **(a) 저장·편집기 20자(실 GlassID 11자 기준), D-15는 실제 GlassID로 `min(20, 30−(len−1))`** / (b) 저장 16자(사이클 기본 GlassID 15자 기준) / (c) 저장 상한 없음, D-15만 | **(a).** 31자 최종 ID 제약(`m_szCellID[..][32]`, WAD `MCRID[2][32]`)을 실 GlassID에서 정확히 채우고, GlassID별 차이는 시작 검사가 잡는다(OOS 기본 GlassID 15자에서 17~20자는 시작 거부 — O7b; 사이클 2번째 글라스부터는 13자 → 상한 18, 패널 A — (3-5) 표). MES 규약(§9-16)이 정해지면 상수만 바꾼다 |
| D4b-4 | 런타임 연결 | **(a) GenCellID가 래치 1일 때 `C_OutRecipeFunc::ResolveCellToken` 직접 조회** / (b) Recipe Change에서 글라스 단위 해석 캐시(2 × 셀 수 CString)를 ALPHA에 만들고 조회 | **(a).** 검증한 데이터 그대로, ALPHA 사본 없음, 측정 스레드의 다른 레시피 읽기와 같은 규칙. 래치가 검증되지 않은 설정의 사용을 막는다. (b)는 동기 코드·메모리만 늘고 CIM PPID 중간 재로드 문제는 같다 |
| D4b-5 | 파생 B 자릿수 | **(a) ICP `%0*d`(A 자릿수 유지)** / (b) HMI Auto B와 같은 `%02d` / (c) `max(2, len)` | **(a).** Auto 씨앗·일괄 입력은 항상 2자리 이상이라 (a)(b)(c)가 실사용에서 같다. 1자리 수동 A(`A5`)만 다름 → "B는 A 형식을 따른다"가 ICP·문서와 일치 |
| D4b-6 | 파생 B 오프셋 축 | **(a) HMI `nMaxTop`(rot1 → 스틱 수, 그 외(rot3 포함) → 스틱0 셀 수)** / (b) ICP Long/Short(rot1·rot3 → 스틱 수) | **(a).** 씨앗 A = Auto A일 때 파생 B = Auto B(GenCellID :201-208과 같은 값)가 되어야 한다. (b)면 rot3 세그먼트에서 파생 B ≠ Auto B |
| D4b-7 | DLL·ALPHA 화면 Cell ID (**리뷰 C4/K5로 재작성 — 명시적 사용자 확인 필요**) | **(a) DLL 레시피 화면 3곳(HEAD 열·SELECT CELL·DLL 검증 모드) = `GetDisplayCellId`, 단 Auto/Auto는 N과 무관하게 `m_chCellID` 그대로(Manual 설정일 때만 해석 A). N>1 Auto/Auto 이월 글자 표시·MakeCell 오버레이·운전 화면은 Phase 6** / (b) 직전 안: N=1 Auto/Auto만 그대로, **N>1 Auto/Auto도 이월 글자로 표시 변경**(HEAD 그리드 seg2 `E01`→`H01`, SelectCell, DLL 검증 모드 — D4-10이 4b로 넘긴 항목) / (c) 표시 무변경(Manual도 `m_chCellID`) | **(a).** HARD RULE("기본 Auto/Auto 모드는 바이트 동일")을 문자 그대로 지킨다 — 기본 모드에서는 화면 텍스트까지 0 변화. 편집기 옆 화면은 Manual에서 실제 ID를 보여야 하므로 (c)는 부적합. 비용: N>1 Auto/Auto에서 편집기(`H01`)와 HEAD 그리드(`E01`)가 다르게 보이는 상태가 Phase 6까지 남는다(Phase 4 리스크 표 :2600에 이미 기록된 상태 그대로). (b)는 D4-10 의도와 맞지만 기본 모드 화면을 바꾸므로 사용자가 명시적으로 승인할 때만 — 승인 시 `GetDisplayCellId` 첫 줄만 바꾼다(5-5) |
| D4b-8 | 레시피 `_CELL_`(`m_chCellID`) 재기록 | **(a) 안 함** / (b) ICP처럼 저장·APPLY 때 해석 A로 재기록 | **(a).** `m_chCellID`는 HeadPair 로그(:453)와 구 MakeCell 규칙 레시피에서 N=1 바이트를 지키는 값. 화면은 리졸버를 직접 쓴다(D4b-7) |
| D4b-9 | 수동 측정(`DlgSubManual_Meas`, UTF-8 BOM) | **(a) Phase 7(D-9 수동 화면 세그먼트화)로 이월, 기록** / (b) 4b에서 세그먼트 0 기준 리졸버 + 측정 전 검증 | **(a).** 수동 화면은 D-9로 세그먼트 0 전용이고 GenCellID 사본·TSP 16슬롯 경로가 따로 있다. 그때까지 Manual 레시피의 수동 측정은 Auto ID(엔지니어링 용도) — 운용 공지 |
| D4b-10 | 신규 파일 인코딩 | **(a) `CellIdEditDlg.cpp` = CP949 CRLF(Perl로 UTF-8 원본 → `encode('cp949', FB_CROAK)`), `CellIdEditDlg.h`·`CellIdRule.h/.cpp` = ASCII CRLF** / (b) 전부 ASCII + 한국어는 `\uXXXX` | **(a).** dfcRecipe 다이얼로그 `.cpp`가 전부 CP949 한글 리터럴이고 메모리 규칙(팝업 한국어, CP949 파일)과 같다. 계획 §10 "신규 ASCII"는 이 행으로 정정. 이 파일도 Edit 도구 금지 |
| D4b-11 | 규칙 코드 위치 | **(a) DLL 내부 `CellIdRule.*` 1벌 + Out export 메서드, 글자 접두는 `SegmentDefine.h` inline(두 사본), ALPHA `CellIdLetterPrefix`는 forward** / (b) In·Out에 각각 복제 / (c) ALPHA 접두 사본 유지 | **(a).** 검증·리졸버가 편집기·저장·런타임에서 같은 코드. Phase 4 계획 :2439 약속(접두를 공용 헤더로) 이행. MakeCellID 본체는 그대로 |
| D4b-12 | 진입점 | **(a) RecipeMgrDlg 버튼 "CELL ID" `IDC_BUTTON_14` 641,144** / (b) SEGMENT INFO 그리드 5행 EDIT 버튼(행 높이 축소 필요, 세그먼트 그리드에 전역 설정) / (c) 옵션 그리드 행(가득 참, 재배치) | **(a).** resource.h 무변경(버튼), 빈 자리 확인, 기존 버튼 열과 같은 형태 |
| D4b-13 | MakeCell APPLY 초기화 알림 | **(a) 비기본 설정이었을 때만 정보 팝업** / (b) ICP처럼 무알림 | **(a).** 수동 ID 소실을 알린다. 기본 설정(모든 기존 레시피)은 팝업 없음 |
| D4b-14 | START 화면 조기 경고 | **(a) 안 함 — Recipe 스텝 1곳이 권위** / (b) 운전 CYCLE·AgingTest·AgingNew에서 같은 검증 + 한국어 팝업 | **(a).** 사이클 index>0의 GlassID가 Recipe 스텝 시점에 바뀌고 CIM은 팝업 금지. UI 3파일(하나는 UTF-8 BOM) 추가 수정 회피 |
| D4b-15 | 레시피 파일을 못 읽었을 때의 자동 시작 (**리뷰 C3/K2로 신설 — 기존 결함, 4b가 실패 면을 넓힘**) | (a) 기록만: 지금처럼 사이클 화면은 1013도 통과(`D_MeasureCycle.cpp:223` `0 <`) → 직전 레시피 ID로 사이클, Recipe 스텝 :626은 반환값 무시 → 마지막 성공 사본으로 측정. 4b 코드 없음 / **(b) 좁은 거부: Out 로드 일련번호가 `OpenRecipe` 전후 같을 때만(= `.rcp` 자체를 못 읽음) Recipe 스텝 WARN + `SetFail(true, JOB_RECIPE_NOT_EXIST)`(알람 1013) + 사이클 화면 한국어 팝업·머무름(4-3b)** / (c) 넓은 거부: :626 `!= VTM_RV_SUCCESSED` + :223 `== VTM_RV_SUCCESSED` | **(b).** 4b는 모든 저장 파일(N=1 포함)에 엄격 트레일러를 붙이므로 "트레일러 손상 → 조용히 다른 레시피로 측정"의 면이 커진다. (b)는 읽을 수 있는 레시피의 경로를 하나도 바꾸지 않는다(로드 성공 시 일련번호 ++ → 거부 블록 미진입; 기본 `.alg` 누락 반환·빈 RecipeID 제외) → HARD RULE과 충돌 없음, 비용 = 2파일 삽입 2개(일련번호는 C2 수정과 공용). (c)는 기본 `.alg`가 없는 N=1 레시피(지금은 진행)까지 거부해 N=1 동작을 바꾼다 → 비권장. 4b 범위를 최소로 두려면 (a) — 그 경우 (8) O18이 현행 동작을 기록 |

- 기록만(결정 아님): PG(Ensis) 수동 ID 미표현(D4-11과 같은 성격), CIM ONLINE 거부 시 호스트 ALM 없음(D-15 확정), 손편집 파일의 Auto 패널 토큰은 로드 시 버림(불변식), **HMI P1~P4 AgingPG 독자 Auto ID**(OCP 무관), **D-15 래치의 로드·GlassID 결합**(검사 뒤 재로드·다른 GlassID → WARN 1회 + Auto, 리뷰 C2/K3/K9 — 설계 수정이지 선택지 아님), **편집기 열기 중 MakeCell 거부 + APPLY 세대 검사**(리뷰 C1/K1).

---

**(8) 검증 (OOS)** — 기대값 출처 `scratchpad/p4b_plan/p4b_expect.pl` + `scratchpad/p4b_fix/p4b_expect_rot.pl`(rot0/rot3·GlassID). GlassID는 사이클 화면에 직접 입력(빈 칸이면 `GLASSID<HHMMSS>B<chk>` 15자 B 패널). **사이클 횟수는 1로** — 2번째 글라스부터 엔진이 GlassID를 `GLASSID<HHMMSS>`(13자, 패널 A, 상한 18)로 덮는다(`DoWorkGlassSequenceEngine.cpp:722`, (3-5) 표). 비교는 Phase 4 방식(`n1ext2.pl`, `wadnorm.pl`, `p4oos_chk.pl` — 시각 제거 + 휘발 값 정규화). 레시피 파일은 테스트 전 백업(`D:\HBS\RECIPE\Recipe\BACKUP` 또는 scratchpad). **테스트용 손상·손편집 파일은 메모장이 아니라 scratchpad Perl 스크립트(`p4b_oos_edit.pl`, raw 읽기·CRLF 유지·편집 전후 바이트 diff 출력)로 만든다**(리뷰 K4 — 바이트 통제).

0. **오프라인(빌드 전·후)**: `p4b_expect.pl`이 (3-6) S1~S4·N=1 표를, `p4b_expect_rot.pl`이 S5(rot0/rot3)·S6(GlassID) 표를 재현. 접두 경계 0 A / 25 Z / 26 AA / 40 AO / 51 AZ / 52 BA / 701 ZZ / 702 AAA. `CellId_TokenMax` 0/11/13/15/30/31/35 → 20/20/18/16/1/0/0. 앵커 재검사(`cnt.pl anchors.txt` + `p4b_fix/anchors_new.txt`) 전부 기대 n. 두 사본 `cmp`(preDefine·SegmentDefine·OutRecipeFunc) 0.
1. **N=1 Auto/Auto 회귀(필수, 첫 번째)** — `1_OCP_TEST_Long_260907`(1012, **재저장 전**), GlassID `ABCDEFG1234`: R1 EFA·R2 AgingTest를 Phase 4 OOS(2026-10-01 13:25:31 / 13:22:31)와 비교 → GLASS 2/4/6/7/8·engine·WAD·PG·AGING·화질1/2·시야각1/2 diff 0, `[CELLID]` 0건, `[HYBRID]` 0건, `[RECIPE]` 0건.
   - 1b: 편집기 LOAD → SAVE(무변경) → 파일 diff = 2행 `1012`→`1013` + 끝 5줄(`_CELL_ID_PANEL_`/0/0/0/`_CELL_ID_END_`)뿐. HEAD 그리드 CELL ID 열 = 이전과 같음(`A01..T01`). **SELECT CELL 그리드·DLL 검증 모드 그리드(P11/P12)도 이전과 같음**(리뷰 K4). R2 재실행 diff 0.
2. **하이브리드 Auto/Auto 회귀** — `9_OCP_TEST_Hybrid_Half_260930`, H1(13:30:35) 조건: diff 0(시각 제외), `[CELLID]` 0건. 편집기 열림(DLL 템플릿 DoModal 첫 사례 — −1 팝업 없음): seg1 A `A01..C05`, seg2 A `H01..D04`(하늘색), B `A06..C10` / `H05..D08`. 상태 그리드 헤더 `VALID (A)` / `ERROR (A+B)`. HEAD 그리드·SELECT CELL·DLL 검증 모드의 seg2 셀 이름 = **`E01..E04` 그대로**(D4b-7 (a); (b)를 승인했으면 `H01..H04`).
3. **구 파일 로드** — `2_OCP_TEST_Short_260907`(1009), `9_OCP_TEST_Hybrid_LS_260929`(1012): 편집기 모드 AUTO/AUTO, ALPHA Recipe Change OK, `9_…BADTAG_260929`는 이전처럼 로드 실패.
4. **A Manual + B Auto (S2)** — MODE A→MANUAL(확인 없음, 씨앗) → seg1 S1 C1 선택 → LINE `CELL 1 : STICK 1->5 (5)` → PREFIX `X` → APPLY PREFIX → A `X01..X05`(연노랑), PANEL B 보기 `X06..X10`(하늘, 파생). APPLY → SAVE → 트레일러 = (2-3)(b) 리터럴.
   - DLL 화면(Manual이라 해석 A, seg1 활성): HEAD 그리드 열 3 = `X01 B01 C01`(스틱 0), SELECT CELL·DLL 검증 모드 행 S1 = `X01 B01 C01`, 행 S5 = `X05 B05 C05`.
   - A 글라스(`ABCDEFG1234`): WAD SEND seg1 스틱0 셀0 `ABCDEFG123X011`(이전 `A018`), 같은 스틱 `B019 C010` 무변경, seg2 `H015…` 무변경. OPER `[CELLID] manual cell ID: A MANUAL, B AUTO (from A), glass panel A (GlassID ABCDEFG1234), longest ID 14/31 chars` 1줄. L5 `id(...)`에 `X011` 반영.
   - B 글라스(`ABCDEFG12B5`): `ABCDEFG12BX061`, `ABCDEFG12BB069`, `ABCDEFG12BC060`; seg2 s0 `ABCDEFG12BH054…`.
5. **중복 거부(APPLY)** — S2 + seg2 S1 C1 A = `X06` → APPLY 팝업 `Cell ID 설정이 올바르지 않습니다. (2건)` + 첫 줄 `Segment 1 Stick 1 Cell 1 (B): Cell ID 'X06' 중복 (Segment 2 Stick 1 Cell 1 (A))`, 편집기가 seg1/PANEL B/S1 C1로 이동. 주황 4칸(seg1 s0c0 B, seg2 s0c0 A·B, seg1 s4c0 B). N=1 변형: Long + 셀 1 줄 PREFIX `P` → 10건 거부.
6. **형식 오류·저장 거부** — S2 저장 파일을 `p4b_oos_edit.pl`로 손편집(CRLF 유지): seg1 첫 A줄 `X01`→`x01`, 둘째 A줄 `B02`→`B-02`, seg2 마지막 A줄 `D04`→`D0000000004`. 편집기 LOAD 성공(내용은 로드 때 검증 안 함) → 빨강 셀 → SAVE → `레시피 저장 불가 (Cell ID 검사)` + 3줄 → 파일 mtime·바이트 무변경.
7. **시작 재검증 거부(D-15)** — 위 손편집 파일로 사이클 START(GlassID `ABCDEFG1234`) → `GLASS 2_RECIPE_CHANGE.log`에 (4-3) 로그 예의 순서 그대로: `[CELLID]` WARN 3줄 + `cell ID check failed (3 errors, GlassID ABCDEFG1234) - auto start refused` + `CDoWorkGlassRecipe Thread NG, Step = 1, Exit = 3` + SYST `CDoWorkGlassRecipeStep_Idle`, `[PRECHECK]` 줄 없음; 엔진 로그 `Cycle Seq Item : … - Fail Exit`; UI `The Inspection is Fail.!!`; **알람 목록·알람 로그 무변화**; Recipe 다음 아이템(Glass SW Align) 미시작.
   - 7b 길이: 유효 파일에서 seg2 S4 C4 A = `ABCDEFGHIJKLMNOPQ04`(19자) → SAVE 통과 → GlassID 빈 칸(15자) START → `… longer than 16 chars (cell ID max 31, GlassID length 15)` 거부. `ABCDEFG1234` → 통과, OPER `longest ID 30/31 chars`(10 + 19 + 체크섬 1). 이 셀은 A가 실패하면 B 파생 검사를 건너뛰므로 시작 거부는 1건. (다회 사이클이면 2번째 글라스가 13자 → 상한 18 → 19자 토큰은 2번째 글라스에서 거부 — 사이클 1회로 테스트.)
8. **왕복** — SAVE → LOAD → 편집기 동일, SAVE 두 번 → `fc /b` 동일. ALPHA Recipe Change 후 같은 SEND.
9. **B Manual만(S4)** — A 글라스: 장치 SEND·GLASS 로그가 Auto/Auto(2번) 기준과 같고 추가는 `[CELLID]` OPER 1줄뿐. B 글라스 seg1 s0c0 `ABCDEFG12BY062`. (DLL 화면은 Manual 설정이라 해석 A = Auto 토큰 → seg2 `H01..`로 보임 — D4b-7 (a)의 의도된 차이, 기본 모드 아님.)
10. **모드 전환·CLOSE** — A MANUAL→AUTO → 확인 팝업(아니오 = 콤보 원복) → 예 → A 수동 삭제, B 보기 순수 Auto 복귀. CLOSE → 다시 열면 이전 설정.
11. **MakeCell APPLY 초기화** — S2 저장 상태에서 MakeCell RECIPE APPLY → 정보 팝업 → 편집기 AUTO/AUTO → SAVE 트레일러 nBlock 0. Auto/Auto 레시피 APPLY는 팝업 없음.
12. **N=1 Manual** — Long + A Manual + 셀 1 줄 PREFIX `Z`: A 글라스 `ABCDEFG123Z013`, B 글라스 `ABCDEFG12BZ063`, abort 없음, `[HYBRID]` 0건.
13. **일괄 입력 경계** — START `1`→`01`, `99`→`99,100,…`, `0` 거부, `999999998` + 5셀 사전 거부(줄 무변경), Auto 모드 버튼 거부, 선택 없음 거부.
14. (선택) **구 빌드 호환** — Phase 4 exe가 남아 있으면 1013 파일을 열어 정상 동작(Cell ID 무시) 확인.
15. **GlassID 없음(리뷰 K4, Phase 4b 체크리스트 `HybridGlass_Porting_Plan.md:2676`)** — ALPHA 재기동 직후(GlassID 빈 값) S2 레시피로 AgingTest START → 패널 A, 최종 ID = 토큰 + 체크섬: seg1 s0 `X015 B013 C014`, seg2 s0 `H019 H020 H031 H042`(S6). OPER `[CELLID] manual cell ID: A MANUAL, B AUTO (from A), glass panel A (GlassID (none)), longest ID 4/31 chars`. 같은 조건 Auto/Auto(Half)는 `A012 B013 C014`, `[CELLID]` 0건.
16. **AgingTest / AgingNew(EFA) START + Manual(리뷰 K4)** — S2 레시피, GlassID `ABCDEFG1234`(사이클로 한 번 설정 후): (a) AgingTest → 반복 행의 WAD/L5 `id(...)`가 `X011` 등 수동 ID, (b) AgingNew(EFA) → T0·T1 두 패스 모두 같은 수동 ID(같은 (seg, stick, cell) = 같은 토큰), (c) S3b 손상 파일로 두 화면 START → D-15 거부(로그는 7번과 같음, 알람 없음), 화면 상태(START 버튼 복귀·진행 표시) 확인·기록.
17. **편집기 ↔ MakeCell 동시성(리뷰 C1/K1)** — MakeCell 창을 연 채 CELL ID 버튼 → `MAKE CELL 창이 열려 있습니다.…` 팝업, 편집기 안 열림. MakeCell 닫고 다시 → 열림. (APPLY 세대 검사(방어선 ②)는 가드가 있으면 UI로 도달 불가 — 코드 리뷰로 확인, 필요 시 디버거로 `m_nCellIdCfgSeq`를 바꿔 팝업·IDCANCEL 확인.)
18. **트레일러 손상(리뷰 C3/K2)** — `p4b_oos_edit.pl`로 S2 파일 사본의 `_CELL_ID_END_` 줄 삭제(또는 seg2 블록 `<nCell>` 4→5). DLL LOAD → 로드 실패 팝업, 이전 레시피 유지. ALPHA:
    - D4b-15 (b): 사이클 화면에서 그 레시피 선택 → 한국어 팝업 `레시피 [...] 파일을 읽을 수 없습니다.…`, 대화상자 머무름. 정상 레시피를 연 뒤 그 파일을 손상시키고 START → `[RECIPE] recipe … could not be read … auto start refused` WARN + 알람 1013 + Fail Exit. 기본 `.alg`만 없는 N=1 레시피(사본으로 재현)는 **지금처럼 진행**(경로 불변 확인).
    - D4b-15 (a): 같은 조작에서 사이클 화면이 닫히고 직전 레시피로 측정됨(현행) — 기록만.
19. **rot0 / rot3 편집기 미리보기(리뷰 K4, D4b-6)** — 실레시피가 없으므로 Half seg1(또는 Long)을 연 뒤 화면 GLASS ROTATE를 0, 3으로 바꾸고(**저장하지 않음**) 편집기 열기 → Auto A/B가 S5와 같은 규칙(글자 = 스틱 역순, 번호 = 셀+1, B 오프셋 = 셀 수). A→MANUAL → PANEL B 보기 = Auto B와 같음(파생 B가 연노랑/주황 아님). 끝나면 LOAD로 버림.
20. **Release 빌드 + 스모크(리뷰 K4)** — `std::vector<S_CellIdError>`가 export 경계를 넘으므로 Release|x64 sln 빌드(`dfcRecipe64.lib`/`.dll` ↔ ALPHA Release, `ALPHA.vcxproj:223`) 성공 확인 후 배포 구성 exe로: 편집기 열기·S3 APPLY 거부 팝업(오류 벡터 경유)·SAVE 거부 팝업·S2 사이클 1회(D-15 통과 OPER 줄). 크래시·힙 경고 없음.
21. **글라스 중간 재로드(리뷰 C2/K3/K9)** — S2 레시피 사이클 중(seg1 측정 중) DLL 검증 모드 창 열기(Out 재로드) → `[CELLID] manual cell ID config not validated for this glass (latch 1, recipe load n/n+1, …) - Auto cell IDs from here` WARN 1회, 이후 셀은 Auto ID(S1 값, 예 seg1 s0 셀0이면 `ABCDEFG123A018`). 다음 글라스는 Recipe 스텝이 다시 검사 → 수동 ID 복귀. 같은 조작을 Auto/Auto(Half)로 → `[CELLID]` 0건.
- 실기 전용: 없음(Cell ID는 SEND 로그로 OOS 검증 가능). APD CELLID 열은 Phase 4와 같이 실기 업로드에서만 확인.

---

**(9) 수정 파일·인코딩** (2026-10-01 `p4b_plan/enc.pl`, Perl raw: BOM / 엄밀 디코드 / CRLF·LF 수 / damaged)

| 파일 | 단계 | 인코딩 / EOL | 비고 |
|---|---|---|---|
| `CoreFunction/dll_Recipe/dfcRecipe/preDefine.h` | 4b-0 | CP949, BOM 없음, CRLF 836 | `Source/Include/preDefine.h`와 동일(cmp 0) — 둘 다 수정 |
| `Source/Include/preDefine.h` | 4b-0 | CP949 CRLF 836 | DLL 빌드가 덮어씀(PreBuildEvent) |
| `dfcRecipe/SegmentDefine.h` / `Source/Include/SegmentDefine.h` | 4b-0 | ASCII CRLF 28 | **자동 복사 없음** → 손으로 둘 다 |
| `dfcRecipe/OutRecipeFunc.h` / `Source/Include/OutRecipeFunc.h` | 4b-0 | CP949 CRLF 80 | 동일(cmp 0) |
| `dfcRecipe/InRecipeFunc.h` | 4b-0 | CP949 CRLF 112 | |
| 신규 `dfcRecipe/CellIdRule.h` / `.cpp` | 4b-0 | ASCII CRLF | Perl로 CRLF 작성 |
| `dfcRecipe/dfcRecipe.vcxproj` / `.vcxproj.filters` | 4b-0, 4b-2 | UTF-8 BOM, CRLF 273 / 163 | filters는 한글 필터명 — 블록 raw 복제 |
| `dfcRecipe/OutRecipeFunc.cpp` | 4b-1 | CP949 CRLF 756 | |
| `dfcRecipe/InRecipeFunc.cpp` | 4b-1 | CP949 CRLF 1231 | |
| 신규 `dfcRecipe/CellIdEditDlg.h` | 4b-2 | ASCII CRLF | |
| 신규 `dfcRecipe/CellIdEditDlg.cpp` | 4b-2 | **CP949** CRLF(한글 팝업) | UTF-8 원본 → `encode('cp949', FB_CROAK)`; CP949 밖 문자(→, — 등) 금지 |
| `dfcRecipe/dfcRecipe.rc` | 4b-2 | **UTF-16LE BOM**, CRLF 370 | Perl `Encode`(python 고장), `use utf8` |
| `dfcRecipe/resource.h` | 4b-2 | **UTF-16LE BOM**, CRLF 151 | ALPHA `resource.h`(UTF-8 BOM)와 다름 |
| `dfcRecipe/RecipeMgrDlg.cpp` / `.h` | 4b-3 | CP949 CRLF 2438 / 218 | 한글 팝업 CP949 |
| `dfcRecipe/MakeCellDlg.cpp` | 4b-3 | CP949 CRLF 1361 | |
| `dfcRecipe/SelectCellDlg.cpp` | 4b-3 | CP949 CRLF 233 | |
| `dfcRecipe/DlgTestMode.cpp` | 4b-3 | CP949, **LF-only** 0/449 | LF 정규화 → `\n` 패치 → LF 그대로 쓰기 |
| `Source/ALPHA/Recipe/RecipeEditMgr.h` / `.cpp` | 4b-4 | CP949 CRLF 172 / 2536 | `.h` → 넓게 재컴파일 |
| `Source/ALPHA/DoWork/DoWorkGlassMeasureStep.cpp` | 4b-4 | CP949 CRLF 7083 | 반복 블록 많음 — 줄 전체 앵커 |
| `Source/ALPHA/DoWork/DoWorkGlassRecipeStep.cpp` | 4b-4 | CP949 CRLF 1099 | (D4b-15 (b)) :626 치환 포함 |
| `Source/ALPHA/D_MeasureCycle.cpp` | 4b-4 (**D4b-15 (b)일 때만**) | CP949, BOM 없음, CRLF 281/281, damaged 0(2026-10-01 `p4b_fix/enc1.pl`) | 한글 팝업 CP949 |
| `docs/HybridGlass_Porting_Plan.md` 등 | 4b-6 | UTF-8 | Edit 도구 가능 |
- 무변경(확인): `GlassMap.cpp`(ASCII), `DlgSubOperate.cpp`(CP949), `DlgSubManual_Meas.cpp`(UTF-8 BOM), ALPHA `DlgTestMode.cpp`(LF-only), `CommTcpIpObject*`(AgingPG = UTF-8 BOM CRLF 2317), `DoWork/DoWorkGlassMeasure.cpp`(CP949 CRLF 777), `CDlgResult.cpp`, `ShareMem.h`. 전 파일 damaged 0.
- 패치 방식: `pfw.pl`(`rd` LF 정규화, `rep1`/`ins_after`/`ins_before`/`post`/`stage`/`commit`), 새 코드 ASCII(한글은 CP949 대상 파일의 팝업 리터럴만, `use utf8` + `Encode::encode("cp949")`). 스크립트는 파일로 작성 후 `perl` 실행(Bash 인라인 금지 — 백슬래시 반감). 백업 `p4b_bak`.
- 전부 `HMI/` git 트리. 구현 뒤 `git -C HMI status`로 확인하고 커밋 여부는 사용자에게 묻는다.

---

**(10) 구현 단계** (스크립트 `p4b_patch.pl --stage=N`, 스테이지마다 단독 컴파일)

| 단계 | 내용 | 파일 | 의존 | 병렬 | 컴파일 확인 |
|---|---|---|---|---|---|
| **4b-0 공용 헤더·규칙 코어** | P1–P5, P16(CellIdRule 등록) | preDefine.h×2, SegmentDefine.h×2, OutRecipeFunc.h×2, InRecipeFunc.h, 신규 CellIdRule.h/.cpp, vcxproj/filters | 없음 | 단독(맨 먼저) | dfcRecipe `CellIdRule.cpp`; ALPHA `Recipe\RecipeEditMgr.cpp`(헤더가 ALPHA에서 컴파일되는지); `cmp` 3쌍 |
| **4b-1 DLL 로드·저장·접근자** | P6, P7 | OutRecipeFunc.cpp, InRecipeFunc.cpp | 4b-0 | 4b-2·4b-4와 병렬 | dfcRecipe `OutRecipeFunc.cpp`, `InRecipeFunc.cpp` |
| **4b-2 DLL 편집기** | P13, P14, P15, P16(CellIdEditDlg 등록) | 신규 CellIdEditDlg.h/.cpp, dfcRecipe.rc, resource.h, vcxproj/filters | 4b-0(In 선언·CellIdRule.h) | 4b-1·4b-4와 병렬(vcxproj는 4b-0 뒤 순차) | dfcRecipe `CellIdEditDlg.cpp`; rc는 4b-5 전체 빌드에서 |
| **4b-3 DLL 연결** | P8–P12 | RecipeMgrDlg.cpp/.h, MakeCellDlg.cpp, SelectCellDlg.cpp, DlgTestMode.cpp | 4b-1, 4b-2 | 4b-4와 병렬 | 각 파일 ClCompile |
| **4b-4 ALPHA 런타임 + D-15** | P17–P20 (+ P21, D4b-15 (b)) | RecipeEditMgr.h/.cpp, DoWorkGlassMeasureStep.cpp, DoWorkGlassRecipeStep.cpp (+ D_MeasureCycle.cpp) | 4b-0(헤더 선언 — `GetRcpLoadSeq` 포함) — 링크는 4b-1 DLL 필요 | 4b-1·4b-2·4b-3과 병렬 | ALPHA `Recipe\RecipeEditMgr.cpp`, `DoWork\DoWorkGlassMeasureStep.cpp`, `DoWork\DoWorkGlassRecipeStep.cpp` (+ `D_MeasureCycle.cpp`) |
| **4b-5 전체 빌드** | — | — | 4b-1~4 | — | `taskkill //IM ALPHA.exe //F`(실행 중이면 사용자 확인) → sln `//t:dfcRecipe:Rebuild` → sln 일반 빌드(ALPHA 재링크), 로그 `vcxproj ->` 확인. **이어서 Release|x64 같은 순서**(`//p:Configuration=Release`, `dfcRecipe64` ↔ ALPHA Release — export 인자 `std::vector<S_CellIdError>` 확인, 리뷰 K4) → O20 스모크 |
| **4b-6 문서** | 아래 | `docs/*.md` | 4b-5 | — | — |
- 같은 파일을 두 병렬 스테이지가 만지지 않는다: vcxproj/filters는 4b-0과 4b-2만(순차), `RecipeEditMgr.*`는 4b-4만, In/Out `.cpp`는 4b-1만, `RecipeMgrDlg.*`는 4b-3만.
- ClCompile 명령(파일 하나씩 — `SelectedFiles`에 `%3B` 여러 개를 넣으면 하나도 선택되지 않는다, Phase 4 메모):
```bash
"/c/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe" \
  "D:\\OCP\\0.SW_Matching\\HMI\\CoreFunction\\dll_Recipe\\dfcRecipe\\dfcRecipe.vcxproj" \
  //t:ClCompile //p:Configuration=Debug //p:Platform=x64 //p:SelectedFiles=CellIdRule.cpp //nologo //v:minimal
"/c/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe" \
  "D:\\OCP\\0.SW_Matching\\HMI\\Source\\ALPHA\\ALPHA.vcxproj" \
  //t:ClCompile //p:Configuration=Debug //p:Platform=x64 //p:SelectedFiles=DoWork\\DoWorkGlassMeasureStep.cpp //nologo //v:minimal
```
  (명령은 스크립트 파일로 실행하거나 위처럼 `//` 형식. DLL만 고친 뒤 증분 빌드가 최신이라 착각할 수 있어 `dfcRecipe:Rebuild` → 일반 빌드 순서 필수 — 루트 CLAUDE.md.)
- **공통 절차**: 편집 전 백업, 앵커 `index` 2회 유일성, 치환 개수 `post`, 편집 뒤 damaged 0·`FB_CROAK` 엄밀 디코드·EOL 수 일치(LF-only 파일은 CR 0), 바뀐 블록 `show.pl`로 재확인, 두 사본 `cmp`.
- **4b-6 문서 갱신 목록**
  - `HybridGlass_Porting_Plan.md` §5.7 "파일" 줄(:375): `_HYBRID_` 뒤 → `_HYBRID_END_` 뒤 트레일러 + `RECIPE_FILE_V1012`(D4b-1 근거), §5.3에 1013 행, §6 Phase 4b 상태·결정 D4b-1~15, §7(물리 셀맵 보류 유지 + MakeCell 오버레이/운전 화면/수동 측정 이월), §9-16 길이 상한 20 기록, §10 표(:2801)의 "CellIdEditDlg 신규 ASCII" → CP949(D4b-10) + CellIdRule.
  - (리뷰 K11) §5.7 리졸버 문구(:372) "세그먼트 셀 수(Short)/스틱 수(Long)" → HMI `nMaxTop`(rot1 = 스틱 수, 그 외 rot3 포함 = 스틱0 셀 수, D4b-6).
  - (리뷰 K11) §5.7 편집기 진입점(:376) "RecipeMgrDlg의 CELL ID A/B → EDIT 행" → 버튼 "CELL ID" `IDC_BUTTON_14` 641,144(D4b-12); §2.5 표(:148) "Cell ID A/B EDIT 행"은 ICP 설명이라 그대로 두고 HMI 차이 주석.
  - (리뷰 K11) D4-10(:2500)과 리스크 표(:2600)가 "DLL 화면(MakeCell 오버레이 포함)은 Phase 4b" → MakeCell 오버레이는 **Phase 6**(D4b-7), N>1 Auto/Auto DLL 화면 이월 글자도 D4b-7 (a)면 Phase 6.
  - (리뷰 K11) §6 Phase 4b 글머리(:2675) "V1011 꼬리 `_CELL_ID_PANEL_`/`_CELL_ID_MANUAL_`" → "`_HYBRID_END_` 뒤 트레일러, `RECIPE_FILE_V1012`", "RecipeMgrDlg EDIT 행" → "CELL ID 버튼"; 검증 글머리(:2676)는 O15(GlassID 없음) 반영.
  - D4b-15 결과(선택한 안)와 `D_MeasureCycle.cpp:223`·Recipe 스텝 :626의 기존 결함 기록, D-15 래치의 로드·GlassID 결합(WARN 1회) 기록.
  - 루트 CLAUDE.md "Recipe file version gate" 예시에 V1012(Cell ID 트레일러) 추가 제안, 인코딩 예외 목록에 새 파일(CellIdEditDlg.cpp = CP949) 기록 제안(루트는 git 밖 — 사용자 결정).
  - HMI 스킬/문서: Cell ID 조립식(GlassID−1 + 토큰 + 체크섬, 31자), D-15 로그 `[CELLID]`.

---

**(11) 리스크**

| 리스크 | 대응 |
|---|---|
| DLL Auto 토큰 재현이 ALPHA MakeCellID/GenCellID와 어긋남(중복 판정·씨앗·표시 오류) | `CellId_AutoToken`에 원문 줄 번호 주석, O2(편집기 Auto = H1 SEND 토큰) + O1 기준선, `p4b_expect.pl`. ALPHA 쪽 규칙을 바꾸는 미래 변경은 두 곳을 같이 고쳐야 한다(문서화) |
| 글라스 중간 재로드(CIM PPID → `RecipeChange`, DLL 검증 모드 → `pOut->LoadRcpFile`)·다른 GlassID로 검증되지 않은 설정이 쓰임(리뷰 C2/K3/K9) | **4b에서 해소**: 래치를 Out 로드 일련번호 + GlassID에 묶고 `RecipeChange`의 리셋을 `LoadRcpFile` 앞으로. 불일치 → `[CELLID] … not validated …` WARN 1회 + 나머지 셀 Auto(한 글라스 안 혼합은 남지만 로그로 드러남). Auto/Auto는 로그 0. O21 |
| 편집기가 열린 동안 MakeCell APPLY가 초기화를 되돌림(리뷰 C1/K1) | **4b에서 해소**: MakeCell 열림 시 편집기 열기 거부 + APPLY 세대(`GetCellIdCfgSeq`) 검사. O17 |
| 수동 측정은 Manual 레시피에서도 Auto ID | D4b-9, 운용 공지, Phase 7 |
| HMI P1~P4 AgingPG는 Manual 레시피에서도 Auto ID | 기록(OCP 무관), 필요 시 별도 |
| 구 빌드가 1013 파일을 재저장하면 트레일러 소실(Auto로 돌아감) | 두 PC 동시 배포(D-12와 같음), 4b 배포 공지 |
| DLL 템플릿 `DoModal` 첫 사례 — 리소스 핸들 | 핸들러 첫 줄 `AFX_MANAGE_STATE` 무조건 + `DoModal() == -1` 팝업(리뷰 C5/K7), O2 |
| `std::vector<S_CellIdError>` export 인자 | Debug/Release 짝(`dfcRecipe64d`/`dfcRecipe64`), 같은 툴셋 /MD, **Release 실제 빌드·스모크 O20** |
| 손편집·손상 트레일러 → `LoadRcpFile` 실패·롤백 | DLL LOAD는 실패 팝업으로 바로 드러남. **ALPHA는 CIM PPID만 NG; 사이클 화면은 1013을 통과시켜 직전 레시피로 사이클, Recipe 스텝 :626은 마지막 성공 메모리 사본으로 진행**(기존 결함, 리뷰 C3/K2) → D4b-15 (b) 권장(파일을 못 읽을 때만 거부), (a)면 기록·운용 공지. BACKUP 폴더 복구. O18 |
| 새 로더 코드의 C++ 예외가 `catch (CException*)`를 빠져나감(리뷰 K8) | `CellId_LoadTail` 형상 선검사·`.at` 미사용·`catch (std::exception &)` → FALSE((2-2)) |
| 자동 운전 중 Manual 레시피로 DLL 검증 모드를 열면(같은 파일이라도) 나머지 셀 Auto | 보수적 설계(일련번호 비교). WARN으로 드러남, 운용 공지 |
| 운전 화면·MakeCell 오버레이가 수동 ID를 안 보임 | Phase 6(D4b-7). 장치 SEND·결과는 정확 |
| N>1 Auto/Auto에서 DLL 화면(`E01`)과 편집기·런타임(`H01`)이 다름 | D4b-7 (a)의 대가(기본 모드 0 변화), Phase 6에서 정합. (b) 승인 시 해소 |
| PG(Ensis) ID ≠ HMI 수동 ID | 기록(D4-11과 같음) |
| N=1 26글자 초과 레시피: Auto 씨앗이 비문자(`[`) → 빨강 | 기존 N=1 한계(Phase 4 기록). 사용자가 수동으로 고친다 |
| `.h` 3개(OutRecipeFunc·SegmentDefine·RecipeEditMgr) 변경으로 넓은 재컴파일 | 4b-0 단독 ClCompile 먼저, 4b-5에서 전체 |

---

**(12) Phase 5/6/7에 넘길 것**
1. Phase 5(Resume): Cell ID는 상태가 없다 — 같은 (seg, stick, cell, 패널)은 같은 토큰. 재개 글라스도 Recipe Change가 돌면 D-15 래치가 다시 선다. **정정(리뷰 C2)**: 재개 경로가 Recipe 아이템을 건너뛰면 래치는 −1이 아니라 **직전 글라스의 값이 그대로 남는다**(글라스 끝에서 아무도 리셋하지 않음). 4b의 로드·GlassID 결합 때문에 결과는: 같은 GlassID·재로드 없음 → 직전 검사 결과로 수동 ID(같은 글라스 재개라면 올바름), 다른 GlassID 또는 그 사이 재로드 → WARN 1회 + Auto ID, 직전이 래치 0(Auto/Auto)이면 Auto. 재개 설계에서 Recipe 아이템 또는 `CheckCellIdConfig` 호출을 포함할 것(설정이 바뀌었을 수 있으므로).
2. Phase 6(운전 화면): `BuildHybridCellMap`(N>1 :3976-3979)과 GlassMap에 `C_OutRecipeFunc::ResolveCellToken(k, i, j, 0, tok, TRUE)` 또는 In `GetDisplayCellId` 사용. `szName[32]`는 `_stprintf_s`(초과 abort) → `_TRUNCATE` 복사로. N=1 :3711 경로는 Auto/Auto 원문 유지. D4b-7 (a)를 택했으면 DLL 화면 3곳의 N>1 Auto/Auto 이월 글자 표시(`GetDisplayCellId` 첫 줄 조건)도 여기서 사용자 승인과 함께.
3. Phase 7(수동 화면): `DlgSubManual_Meas` GenCellID 사본(:4707, UTF-8 BOM)에 같은 훅 + 측정 전 `ValidateCellIdConfig(len(GlassID))`, TSP 16슬롯(:2994-3053)도. 같은 파일 `:2092` `if (0 < …OpenRecipe(strRcpFile))`도 1013을 성공으로 받는다 — D4b-15에서 고른 판정(로드 일련번호)을 그대로 적용.

---

**(13) 리뷰 반영** — 요청 표기 "(11) 리뷰 반영"(이 문서의 (11)은 리스크라 13으로 번호). 리뷰 원문 `scratchpad/p4b_plan_reviews.md`. 모든 항목은 코드로 다시 확인한 뒤 반영했다(확인 위치는 각 행). C = compat 리뷰, K = complete 리뷰.

| # | 지적(요지) | 코드 확인 | 판정 | 플랜에 바뀐 것 |
|---|---|---|---|---|
| C1 / K1 (med) | 편집기가 열린 동안 모델리스 MakeCell의 RECIPE APPLY가 `ResetCellIdConfig`를 부르고, 편집기 APPLY가 옛 수동 ID를 인덱스대로 새 셀에 되살림. "(geometry cannot change while modal)" 주석 틀림 | `RecipeMgrDlg.cpp:17-32`(모델리스 `Create`), `:461-466`(ShowWindow), `dfcRecipe.rc:109`(페이지 `WS_CHILD`), `BaseDialog.cpp:54-58`(`CDialogEx::DoModal`), `MakeCellDlg.cpp:438/:1251-1257`; In `LoadRcpFile` 호출처는 `RecipeMgrDlg.cpp:317` 하나 | **반영** | (1-3) In `m_nCellIdCfgSeq`/`GetCellIdCfgSeq()`(로드 성공·**Reset 매 호출**·Set에서 ++ — 리뷰 C1의 "Reset이 TRUE일 때만"은 기본 설정 → 편집기에서 Manual로 바꾼 경우를 놓쳐 "매 호출"로 강화), (1-4) 동시성 문단, (5-2) `m_nCfgSeq`·주석 정정, (5-3) OnInitDialog 저장 + APPLY 세대 검사 → 한국어 팝업 + `EndDialog(IDCANCEL)`, (5-4) MakeCell 보이면 열기 거부, (8) O17, (11) 행 |
| C2 / K3 (low / low-med) | D-15 래치가 검사한 설정·GlassID와 묶이지 않음(DLL TestMode 재로드, CIM 스레드 재로드, `:137`~`:144` 사이 창), §(12)-1 문구 틀림 | `DlgTestMode.cpp:99-104`(`pOut->LoadRcpFile`), `RecipeMgrDlg.cpp:2079-2102`(UpdateUI는 레시피 명령만), `CommTcpIpObjectCIM.cpp:1416`, `RecipeEditMgr.cpp:137-144`, GenCellID `:228` `strGlassID` | **반영** | (1-3) Out `m_nRcpLoadSeq`/`GetRcpLoadSeq()`(클래스 끝, export, 로드 성공 시 ++) + 생성자 초기화, (2-5) 표 2행, (4-1) `GetManualCellToken(…, strGlassID, …)` 일련번호·GlassID 비교, `SetCellIdLatch(latch, seq, GlassID)`, `RecipeChange`의 래치 리셋을 `LoadRcpFile` **앞**(:137)으로, 래치 상태 표, (4-3) `SetCellIdLatch(1, GetRcpLoadSeq(), strGlassID)`, (12)-1 정정, (8) O21. 이름은 리뷰안 `m_nCellIdCfgSeq` 대신 Out은 `m_nRcpLoadSeq`(D4b-15 (b)와 공용이라 의미대로) |
| C3 / K2 (low / med) | 손상 트레일러가 START를 거부하지 않음 — :626 반환값 무시, `D_MeasureCycle.cpp:223` `0 <`가 1013 통과 → 직전 레시피로 사이클 | `DoWorkGlassRecipeStep.cpp:626`, `D_MeasureCycle.cpp:223`, `AlarmIDList.h:617/:631`(`VTM_RV_SUCCESSED` 1, `JOB_RECIPE_NOT_EXIST` 1013), `RecipeEditMgr.cpp:137-151/:259-273` | **반영** | (2-1) 표 문구 정정 + "그 반환값을 누가 보나" 행, 호환 행렬, (4-3) 이 시점 문구, (4-3b) 신설, **D4b-15 신설**((a) 기록 / **(b) 로드 일련번호로 `.rcp` 읽기 실패만 거부(권장)** / (c) 넓은 거부 — 기본 `.alg` 누락 N=1까지 바꿔 비권장), (6) P20·P21, (9) `D_MeasureCycle.cpp` CP949 CRLF 281, (11) 행, (8) O18. C3의 "4b 밖 별도 결정"과 K2의 "거부 추가" 사이에서: 별도 결정(D4b-15)으로 두되 HARD RULE과 충돌하지 않는 좁은 안을 권장 |
| C4 / K5 (low) | N>1 Auto/Auto DLL 표시 변경이 HARD RULE 문자와 충돌 | `RecipeMgrDlg.cpp:1653`, `SelectCellDlg.cpp:157`, `DlgTestMode.cpp:168`, 플랜 (5-5) | **반영** | D4b-7 재작성: **(a) Auto/Auto는 N 무관 `m_chCellID` 그대로(권장)** / (b) 직전 안(명시 승인 필요) / (c) 무변경. (0)-8, (4-2), (4-4), (5-5) 코드(`if (m_sCellIdCfg.IsDefault ()) return GetCellID (…)`), (8) O2 기대값 `E01..E04`, (11)·(12)-2 |
| C5 / K7 (low) | DLL 모듈 형식 서술 틀림(`_USRDLL`만이 아님), `DoModal` −1이 CLOSE와 구분 안 됨 | `dfcRecipe.vcxproj:155/:213`(`_AFXEXT;_USRDLL;_WINDLL`), `dfcRecipe.cpp:66`(`theApp`), `API_RECIPE.cpp:19-80`, DLL 기존 `DoModal` = `CFileDialog`뿐 | **반영** | (5-4) 핸들러 첫 줄 `AFX_MANAGE_STATE (AfxGetStaticModuleState ());` 무조건, `INT_PTR nRet` −1 → TRACE + 한국어 팝업, 모듈 상태 문단 정정, (11) 행. O2 유지 |
| C6 / K6 (low) | Cell ID 출처 누락: AgingPG(P1~P4), `DoWorkGlassMeasure.cpp:206-220` rows 로그, `InRecipeFunc.cpp:1225` | `CommTcpIpObjectAgingPG.cpp:1603-1640/:2207-2261`(UTF-8 BOM CRLF 2317), `DoWorkGlassMeasure.cpp:206-220`, `InRecipeFunc.cpp:1225` | **반영** | (0) 이월 3행, (4-4) 표 3행 "무변경·기록", (6) 무변경 목록, (7) 기록 줄, (9) 무변경 목록, (11) AgingPG 행 |
| C7a (low) | 넓힌 31자 가드가 N=1 override에 `[HYBRID]` 태그를 찍음 | `DoWorkGlassMeasureStep.cpp:257/:260` | **반영** | (4-1) 변경 4: `bHybridCid ? "[HYBRID] …" : "[CELLID] …"`(하이브리드 문구 불변), (4-2), (6) P19 :260 |
| C7b (low) | `CellId_TokenMax`가 31자 이상 GlassID에서 음수 | 산식 확인(len 35 → −4) | **반영** | (3-1) 선언 주석 `max(0, …)`, (3-5) 표 행, (8) O0 경계값 |
| C7c (low) | 사이클 2번째 글라스부터 GlassID 13자(상한 18, 패널 A) | `DoWorkGlassSequenceEngine.cpp:712-723`, `D_MeasureCycle.cpp:205-211` | **반영** | (3-5) GlassID 표, (3-6) S6, D4b-3 문구, (8) 머리말(사이클 1회)·O7b |
| C8a (low) | `CellIdRule.cpp`는 `pch.h`를 먼저 | `dfcRecipe.vcxproj:93/:123`(Win32만 PCH), x64 :146-215 없음, `pch.h` | **반영** | (3-1) include 규칙 |
| C8b (low) | 사후 개수 `GetDisplayCellId (`가 LF 파일 스타일(공백 없음)과 안 맞음 | `DlgTestMode.cpp:168` `GetCellID(i, j)` | **반영** | (5-5) 파일별 치환 문자열 명시, (6) 정규식 `\s*\(` |
| C8c (low) | MakeCellID 블록 줄 번호 | `iStick = GetMeasureStick();` :2240/:3216/:4096/:4750/:5556, 호출 :2555/:3499/:3503/:4347/:5008/:5807/:5811 | **반영** | (3-2) 줄 번호 정정(주장 자체는 유지) |
| C8d (low) | `.filters` 복제 시 CellIdRule이 `Dlg` 필터로 | `.filters:78/:84/:134/:140` | **반영** | (5-7) CellIdRule = `InRecipeFunc` 블록(`소스 파일\Func`) 복제 |
| K4 (low-med) | OOS 누락: GlassID 없음, AgingTest/EFA + Manual, DLL 화면 P11/P12, Release, rot0/rot3, O7 로그, 동시성·손상 트레일러, 메모장 대신 Perl | `HybridGlass_Porting_Plan.md:2676`, `DlgSubManual_AgingTest.cpp:495`/`AgingNew.cpp:550`(GlassID 미설정), `DoWorkGlassRecipe.cpp:22/:34-40/:138`, `DoWorkGlassRecipeStep.cpp:173`, `ALPHA.vcxproj:223` | **반영** | (3-6) S5(rot0/rot3)·S6(GlassID) — `p4b_fix/p4b_expect_rot.pl`, (4-3) 로그 예 전체 순서, (8) 머리말(Perl 편집)·O1b/O2/O4 DLL 화면·O6 Perl·O7 로그·O15~O21 신설, (10) 4b-5 Release |
| K8 (low) | 로더가 MFC 예외만 잡음 | `InRecipeFunc.cpp:153` `catch (CException *e)` | **반영** | (2-2) 예외 안전 문단(형상 선검사, `.at` 미사용, `catch (std::exception &)` → FALSE), (11) 행 |
| K9 (low) | CIM 중간 재로드 시 수동/Auto 혼합이 조용함 → 4b에서 WARN 1회 | C2와 같은 확인 | **반영** | `GetManualCellToken`의 WARN 1회(래치 −1 또는 불일치 && `!IsCellIdDefault()`, 래치 2로 재경고 방지) — Auto/Auto 로그 0 유지, (11) 행(후속 → 4b) |
| K10 (low) | ICP X15: VALID는 현재 패널, ERROR는 양 패널 | `p4b_map_icp.md:579` | **반영** | (5-3) 헤더 `VALID (%c)` / `ERROR (A+B)`, O2 |
| K11 (low) | 4b-6 문서 목록 누락 | `HybridGlass_Porting_Plan.md:148/:372/:375/:376/:2500/:2600/:2675-2676/:2801` | **반영** | (10) 4b-6 목록 4행 추가 |

- 거부한 지적: **없음**. 부분 수정 2건 — C1의 "Reset이 TRUE일 때만 ++"는 "매 호출 ++"로 강화(이유: 기본 설정 레시피에서 편집기로 Manual 전환 중 MakeCell이 재생성하면 TRUE가 아니어서 못 잡음), C2의 Out 멤버 이름은 `m_nRcpLoadSeq`(D4b-15 (b)와 공용).
- 리뷰가 "문제 없음"으로 확인한 영역(버전 게이트, 구 파일, preDefine ABI, 리졸버 산식, 중복 범위, N=1 바이트 동일, 스레드, 31자 산식, D-15 무알람, 앵커 50개, 인코딩 표)은 위 수정으로 바뀌지 않는다 — 새 앵커 15개는 `p4b_fix/anchors_new.txt`로 n=1 확인(`OnBtnTestMode` 앵커는 탭이 아닌 공백 `void C_RecipeMgrDlg::OnBtnTestMode()\n` n=1), 새로 만지는 `D_MeasureCycle.cpp`는 CP949 CRLF 281 damaged 0.

### Phase 5 — Resume (D-6) — ✅ 구현·컴파일 오류 0 (2026-10-08, dResult·ALPHA ClCompile), 링크·OOS 대기

**구현 완료 (2026-10-08)** — 아래 2026-10-06 진행 상태는 중단 시점 기록이다.
- 10-08 재개 워크플로: 5-1 마무리(결함 3건 수정 — 레시피 지문 경로 `{root}Recipe\`, 셀 상태 복원의 역방향 스틱 열, 64 MB 초과 저널이 모든 START를 막던 분류) → 5-2 북마크(Format 2, `MoveFileEx` 원자 교체) → 5-3 진입점 ∥ 5-4a 시퀀스(Measure/Recipe/Result/Unload) ∥ 5-4b MeasureStep·VISION → 전체 컴파일(오류 0, 새 경고 0) → 리뷰 2 → 수정 4건(1세그먼트 글라스의 헛 `DeleteFile` 12회 제거 → 파일 접근 0, 재개 시 직전 실패 플래그 잔존, 측정 중 재개 팝업 거부, 배포 묶음 명시).
- 변경 기록: work 폴더 `p5_50/51/52/53/54a/54b/compile/fix_changes.md`. 5-7 문서 정리(§5.8 줄 번호·저널 위치·RS1 실상 등)는 OOS 뒤.
- **배포:** `dResult.dll`·`dResult.lib`·`API_Result.h`(2사본)·`dfcRecipe64d.dll`·`ALPHA.exe`를 같이 빌드·배포·커밋. 새 exe + 구 dResult.dll이면 기동 실패. 순서: sln `//t:dResult:Rebuild` → `//t:dfcRecipe:Rebuild` → 일반 빌드.
- OOS: (8) O0~O25(1세그먼트 일반 측정 diff 0이 첫 번째). `RESUME_TUPLE_ALL`은 기본 0 — 통과 후 사용자가 1로 전환(5-6).
- **빌드 (2026-10-08 11:44, 사용자):** `ALPHA.exe`·`dResult.dll` 새로 빌드(Phase 5 + Sampling 포함, `dfcRecipe64d.dll`은 10-06 그대로 — 4b 이후 dfcRecipe 변경 없음). 11:49 ALPHA 기동, 12:08~12:09 `Bin\Config\ResumeJournal.csv`·`ResumeCellStatus0~2.dat`·`ResumeInfo.ini` 생성 확인(런타임 파일 — 커밋 대상 아님). OOS 판독은 아직 안 함.
- **OOS 1차 판독 (2026-10-12, 10-08 11:52~12:09 런, 시뮬 키 0):**
  - N=1 회귀(`1_OCP_TEST_Long_260907` CYCLE 11:54:18 ↔ 10-06 09:39:32, 4b 빌드): GLASS 1/2/4/5/6/7/8·Sequence_Step·엔진·WAD·PG·AGING·화질1/2·시야각1/2·OCP 로그 17개 **diff 0**. 예외는 WAD·화질2·시야각2의 셀 ID 5개(`…Z013` → `…A018`)뿐이고, 원인은 사용자가 11:54:07에 수동 Cell ID 표를 고쳐 저장한 것(레시피 파일 시각). 레거시 `Resume Save/Update/Reset` 줄 동일, `[RESUME]` 0줄. 스크립트 scratchpad `p6oos/n1cmp1008.pl`.
  - 하이브리드 EFA 재개(`9_OCP_TEST_Hybrid_Half_260930`, 레시피 EFA 2패스): 12:04:42 `[RESUME] new glass bookmark` + `Resume Begin` + 스틱마다 `Resume Update: T0 seg k/2 stick n/5`. T0 seg 2 stick 1 측정 중 STOP(12:06:59, 기존 `Thread NG, Step = 10` 경로) → 12:08 START·재개: `Resume mode (journal): result grid cleared`, `[RESUME] cell status restored (snapshot after T0 seg 1 stick 5)`, `[RESUME] resume point T0 seg 2/2 stick 1/5 (glass stick 6/10) ... attempt 1`, 곧바로 `Segment boundary 1 -> 2/2`(seg1 스틱 줄 없음), `Resume Attempt: 1`. 두 번째 STOP(12:09:44, T0 seg2 stick 4) 뒤 INI `LastSegment=1 LastStick=2 Attempt=1` — 일치. 알람 0.
  - `journal replay: 0 rows`는 시뮬 키 0이라 결과 행이 없어서다(설계대로). 재생 행 비교(O2·O3·O5)는 시뮬 키 1로 다시 돌려야 한다.
  - **OOS 2차 (2026-10-12 09:09, 10-08 북마크 재개, 재기동 후):** SYST `Resume tuple: Format 2, T0 seg 2/2 stick 3 ...` → START·재개 `resume point T0 seg 2/2 stick 4/5 ... attempt 2`, `cell status restored (snapshot after T0 seg 2 stick 3)` → T0 seg2 stick 4~5 → `EFA: T0 result flush OK` + `Resume Boundary: pass 1 boundary done` → T1 seg1(stick 3~5 마스크 skip)·seg2 → `Resume ResultSaved: 1/2` → `Resume Reset`. 재시작 후 재개(O6)·EFA T0→T1 경계 통과 확인.
  - 이 런에서 사용자가 본 두 현상과 수정(2026-10-12, 사용자 승인, 컴파일 OK·빌드 대기, 상세 work 폴더 `fix1012_changes.md`):
    - **세그먼트 경계 직후 Probe Z 3 mm 하강**(글라스 진입 전, 09:09:44.312 `PROBE Z 3.000 SPPED 2` → 0): 재개 전용 아님, 경계마다. `MoveMeasureArea`가 Z 위치와 무관하게 UnContact(PreContact 3.0)를 부르고, OCP가 `AXIS_ALL_Z`를 Z5 단일축으로 돌리면서 2023-04-07 위치 가드가 빠져 0 → 3.0 → 0. `MaintControlMgr::MoveCmdPos_ProbeUnContact` Z5 case에 가드 복원(이중 위치 읽기, 생략 시 SYST `... slow lift skipped`).
    - **EFA T1 AGING Start 때 셀맵 `AGING S` 미표시**: T1은 OST·R-TEST skip이라 T0 측정 끝 상태가 남아 PG AGING START 대상 목록에 없음(1세그먼트 EFA T1도 동일). 패스 ≥ 1에서 측정 끝 상태도 `AGING_START`로(`CommTcpIpObjectPG.cpp` 7곳 + `m_nEfaCurPass`), T1 에이징 NG여도 T0 측정 셀은 T1에서 계속 측정(사용자 결정, OCP_EFA와 같음 — 측정 제외 5곳에 `IsEfaReAged` 예외).
  - **OOS 3차 (2026-10-12 09:54~10:04, ALPHA.exe 09:51 빌드 = 위 두 수정 포함, 시뮬 키 0) — OK:**
    - 런 A 새 글라스: `[RESUME] new glass bookmark` + `Resume Begin` → T0 seg1 1~5 → 경계 → seg2 stick 3 측정 중 **ALPHA 종료**(STOP 줄 없음) → 09:57 재기동 `Resume tuple: Format 2, T0 seg 2/2 stick 2 ... attempt 0`.
    - 런 B 재개: `Resume Attempt: 1`, `cell status restored (snapshot after T0 seg 2 stick 2)`, `resume point T0 seg 2/2 stick 3/5` → seg2 3~5 → `EFA: T0 result flush OK`(APD·xls 10:00:04) + `Resume Boundary: pass 1 boundary done` → T1 seg1 stick 1 → stick 2 측정 중 **ALPHA 종료** → 10:01 재기동 `Resume tuple: ... T1 seg 1/2 stick 1, ... boundary 1, attempt 1`.
    - 런 C 재개: `Resume Attempt: 2`, `snapshot after T1 seg 1 stick 1`, `resume point T1 seg 1/2 stick 2/5, pass boundary done 1`, **`EFA: pass boundary skipped on resume (T0 already flushed, EFA^0$ already sent)`**(T0 파일은 10:00:04 1개뿐) → T1 seg1 stick 2 전체 재측정 → stick 3~5 마스크 skip → 경계 → seg2 1~5 → `Result: glass save gate 1 - SaveResult`(APD·xls 10:04:01) → `Resume ResultSaved: 1/2` → `Resume Reset`, Resume 파일 삭제 확인.
    - Probe Z: 경계·재개 시작 5곳(09:56:27, 09:58:53, 10:00:13, 10:02:07 = seg1 stick 2부터 재개, 10:02:36) 모두 `MoveCmdPos_ProbeUnContact PROBE Z at 0.000 is not below the slow-lift target 3.000 - slow lift skipped` + `PROBE Z 0.000`만, 3.0 하강 없음. 스틱 사이 해제는 4.2 → 3.0(SPEED 2) → 0 그대로.
    - T1 셀맵 `AGING S` 표시: 사용자 화면 확인 OK. PG `AGING START`/`AGING END` 18/18(런 A 7, B 5, C 6 = 측정 스틱 수).
    - WARN: `InnerID duplicate remapped` 2건(무해), 기동 시 카메라 없음(OOS). 알람 0.
    - 저널 재생 0행: OOS는 장비가 결과 파일을 통보하지 않아 `TossResultFile` → `Updata_Data`가 없다(측정 로그의 `[HYBRID] result row`는 행 배정 로그). 행 재생 검증은 `RESUME_JOURNAL_OOS_SIM=1`로.
    - 이 런으로 확인된 것: O6(재시작 후 재개 — 정상 종료가 아닌 측정 중 종료로 2회), O5(T1 재개·경계 skip), 경계 완료 플래그(5d 상당), 시도 횟수 증가.
  - 남은 OOS: 시뮬 키 1 하이브리드 정상(O2)·세그먼트 2 중간 STOP(O3)·AgingTest 반복 중 STOP(O4)·EFA T1 중 STOP(O5, 변형 5b~5f)·재시작 후 재개(O6)·"아니요"(O7) 등 (8) 나머지, N=1 AgingTest·EFA 회귀(O1 R1/R2).
- 같은 날 별건으로 **Sampling 스틱 skip**(OCP, `MeasureSequenceItemEngine.h/.cpp`, `DoWorkGlassMeasure.cpp`)이 같은 작업 트리에 들어갔다 — `docs/Sampling_StickSkip_Plan.md`.

**진행 상태 (2026-10-06)**
- 설계 워크플로: 매퍼 3(현행 Resume / 결과 깔때기·dResult / 시퀀스·래치·ICP) → 플래너 → 리뷰 2(정확성·N=1 / 완전성·위험) → 수정. 지적 32건 전부 반영(5건은 리뷰어 제안과 다른 방식).
- 사용자 결정: 패키지 승인 — D5-1 (b) `RESUME_TUPLE_ALL`(기본 0, OOS 통과 후 사용자가 1로 전환), D5-2 (b), D5-8 (a), D5-9 (b), D5-10 (a), D5-11 (a), D5-15 (a). D5-6 (b) 재Align은 필요할 때만, D5-19 (a) 재개 시 PreMeas 컨택 유지. 나머지 권장안.
- 구현 (모두 미커밋):
  - 5-0 dResult 빠른 재생 export(`API_Result_SetFastUpload`/`API_Result_GetUploadState`): 완료, 컴파일 OK.
  - 5-1 저널 코어(`Manager/ResumeJournal.*` 신규, `ResultMgr` 훅, `ResumePrompt.*` 뼈대, vcxproj/filters): 파일 작성·인코딩 OK, **컴파일 미확인**.
  - 5-2 북마크, 5-3 진입점, 5-4 시퀀스, 전체 컴파일, 리뷰·수정: 미착수.
- 설계·변경 기록·패치 스크립트·백업 사본: `D:\OCP\0.SW_Matching\work\hybrid_session_20261006\`(git 밖, 재개 순서 `README_재개.md`).
- 배포 주의: dResult.dll·dfcRecipe64d.dll·ALPHA.exe는 같이 빌드·배포(새 export).

---

**설계 원문 (2026-10-06 플랜, 리뷰 반영본)**

**사용자 결정(2026-10-06): 패키지 승인 — D5-1 (b)(`RESUME_TUPLE_ALL`, 기본 0, OOS 통과 후 사용자가 1로 전환), D5-2 (b), D5-8 (a), D5-9 (b), D5-10 (a), D5-11 (a), D5-15 (a). D5-6 (b) 필요할 때만 강제 재Align. D5-19 (a) 재개 시 PreMeas 컨택 유지. 나머지 D5-3~5, 7, 12~14, 16~18은 권장안.**

상태: **플랜(2026-10-06), 리뷰 2건(correctness·N=1 / completeness·risk) 반영 완료(2026-10-06, (13) 표), 구현 전.** 확정 사항(D-6 "Resume은 세그먼트·AgingTest·EFA 전부에서 동작", §5.8의 진행 튜플·세그먼트별 검증·재개 스틱 전체 재측정·저널 재생·`nCellStatus` 복원·북마크 수명·P0-1 전제, D-13 EFA 정순, D-14 글라스 전역 플래그, D-15 Cell ID 재검증)은 다시 열지 않는다. **예외 2건(리뷰 C11 반영)**: D5-6은 §5.8-3의 "방향이 같아도 강제 경계" 문구를 바꾸는 결정이라 사용자 승인 대상(§), §5.8-5의 "스냅샷 없으면 T0부터로 강등"은 팝업 단계 거부로 구현한다(결과 = 새 글라스로 같음, 기록). 사용자 선택이 필요한 것은 (7)의 D5-n이다. **HARD RULE과 부딪히는 것(N=1 재개 동작 변경)은 D5-1, D5-2, D5-8, D5-9, D5-10, D5-11, D5-15(★)에 모았고, §5.8 문구 변경 D5-6(§)과 공정 판단 D5-19(공정)도 명시 승인 대상이다.**
- 리뷰 원문: `scratchpad/p5_plan_reviews.md`(C = correctness·N=1 리뷰 1~15, K = completeness·risk 리뷰 1~17). 반영 검증: `scratchpad/p5fix/`(`anc_fix.pl` 새 앵커 n·인코딩, `dec/` 2026-10-06 디코드 사본 — 리뷰어 사본 `rv5/`와 `cmp` 동일).
- 출처: 매퍼 3개(`p5_map_resume.md` 현행 Resume 경로 / `p5_map_result.md` 결과 깔때기·dResult / `p5_map_seq.md` 시퀀스·래치·ICP) → 이 플래너. 설계에 걸리는 주장은 코드로 다시 확인했다(스크립트 `scratchpad/p5plan/`: `enc.pl` 인코딩·EOL, `anchors.pl`·`anchors2.pl` 앵커 개수, 디코드 사본 `scratchpad/p5dec/`).
  - 직접 확인: dResult 1초 대기와 플래그 미해제(`CDlgResult.cpp:597-618`, `:96`, 메시지 맵 `:123-124` 비어 있음), 20칸 링 최고참 버림(`:405`), `SaveResult` 5초 포기(`:1731-1752`), Measure 루프·진행 저장·경계 함수 전체(`DoWorkGlassMeasure.cpp:143-777`), `GetEfaPassCount`·`AgingGlassLatch`(`MeasureSequenceItemEngine.cpp:896-952`), `TakeVisionRcpWait`(`RecipeEditMgr.cpp:462-468`, 호출 1곳 `DoWorkGlassMeasureStep.cpp:3897`), Idle 스텝(`:1958-1985`, `GetFail()`이 아니면 완료), `CheckAndApplyResumeMode` 전문(`DlgSubOperate.cpp:3479-3532`)과 두 호출부(`:1786-1790`, `:3425-3449`), `GenGlassPath` 단일 호출처(`:3595`), START 메시지가 모든 서브 화면의 `UpdateRecipeInformation`을 부른다는 것(`DlgMainForm.cpp:643-653`), `EngineStart()` 호출처 4곳(Operate CYCLE, CIM, AgingTest, AgingNew), VISION `QualityWaitSet`이 OOS 반환 **전에** 불린다는 것(`CommTcpIpObjectVision.cpp:412-414`).
- 코드 기준: `D:\OCP\0.SW_Matching\HMI`, 브랜치 `hmi-master`, Phase 4·4b 미커밋 작업 트리. 줄 번호는 2026-10-06 작업 트리(참고용), 패치는 앵커로 한다(앵커 n은 (6)·(9)에 표기).
- **HARD RULE:** N=1 레시피의 정상(비재개) 런은 동작·로그·장치 SEND·결과가 바이트 동일. N=1 **재개** 동작을 바꾸는 것은 사용자 명시 결정. 새 팝업은 한국어(CP949 파일은 CP949 바이트), 로그는 영어. 주석 `// LYS 2026-10-06 ...`.

---

**(0) 요약 / 범위**

*한 줄*: 재개 북마크를 `(pass, seg, stick)` 튜플(`ResumeInfo.ini` `Format=2`)로 바꾸고, 그리드에 들어간 모든 결과 행을 측정 시점 그대로 `Config\ResumeJournal.csv`에 1줄씩 적어 두었다가 재개 때 **측정 아이템 진입부에서 재생**한다. 재생 속도는 dResult에 "재생 중에만 1초 표시 대기 생략" 스위치와 처리 카운터를 더해 해결한다(N=1 정상 런 무영향). 재개 위치는 기존 루프 시작식만 바꿔 (pass, seg) 이전을 건너뛰고, 재개 스틱은 처음부터 다시 잰다. EFA는 `EfaPassBoundary` **안에서** 세우는 "경계 완료" 플래그로 T1 재개 때 T0 플러시·APD·`EFA^0$` 재송신을 막고, 셀 상태는 스틱마다 저장한 `nCellStatus` 스냅샷(3칸 링, 튜플 태그)에서 **재개점 직전 튜플과 정확히 같은 것**을 UI 리셋 **뒤**에 복원한다.

*안전 원칙(리뷰 반영)*: ① 측정 진입은 파일을 지우지 않는다 — 진입 검사·재생 실패는 `SetFail(true, 0)`로 글라스만 멈추고 북마크·저널·스냅샷을 그대로 둔다(다음 START에 같은 제안), 사용자 STOP은 기존 정지 경로로 나간다. 지우는 것은 팝업에서 운전자가 고른 경우(아니요/거부 OK)와 정상 완료뿐. ② 북마크를 되돌리는 쓰기(VISION 당김)는 **INI를 먼저**, 저널 교체를 나중에 — 어느 틈에서 멈춰도 다음 재개가 같은 결과를 낸다. ③ 저널 쓰기 실패는 끈적한 플래그(`JournalOK`)와 줄 수(`JournalLines`)로 북마크에 남겨 재개 때 거부한다. ④ 결과 저장 완료는 2단(`ResultSaved` 1 = Result 아이템 완료, 2 = Unload의 `SaveResultDone` 확인)으로 적는다.

*튜플 글라스*: `IsTupleGlass(nSegCount)` = `nSegCount > 1` (D5-1 (b)를 승인하고 `RESUME_TUPLE_ALL=1`이면 모든 글라스). 튜플 글라스만 새 경로(북마크 Format=2·저널·스냅샷·튜플 팝업)를 쓰고, 그 밖(N=1, 기본)은 **지금 코드 경로를 한 글자도 바꾸지 않는다**(레거시 Format=1, RS1, 기존 팝업 문구).

*Phase 5에 들어가는 것*
1. 북마크 Format=2: `LastPass/LastSegment/SegmentCount/SegShape/GlassRows/PassCount/EfaMode/EfaFlag/PassBoundaryDone/ResultSaved/Attempt/JournalOK/JournalLines/Fingerprint`, 쓰는 지점(측정 시작·스틱 완료(마지막 스틱은 Idle **전**)·EFA 경계 안·압축/당김·Result 완료·Unload 저장 확인), 구 파일 호환, 원자 교체(`MoveFileEx`).
2. 검증·안내: 튜플 검증(이미 저장된 글라스 조용히 정리를 맨 앞으로, 레시피·튜플 대상 여부·세그먼트 형상·G·파일 지문·저널 헤더/쓰기 상태/줄 수·스냅샷(EFA 글라스는 재개점 직전 튜플 필수)·GlassID·EFA 모드 일치), 새 한국어 팝업(세그먼트 n/N, T0/T1, 중단 시각, 현재/이전 Glass ID, VISION 당김 예고, 누락 raw 수), 되돌릴 수 있는 거부(`MB_OKCANCEL`), 진입점 4곳 공용 헬퍼 `ResumePrompt.cpp`(신규 CP949) + 팝업 중 다른 START 차단 가드.
3. 재개 위치: 루프 시작식 `(nPass == rPass && nSeg == rSeg) ? rStick : 0`, 경계는 기존 `SegmentBoundary(0, k)` + 필요할 때만 강제 재Align(D5-6 (b)), EFA 경계 생략, "끝까지 측정됨"(D5-13) 처리, 0회 반복 무한 대기 방지, 진입 중 STOP/일시정지 처리.
4. 결과 저널: 형식(R/P/X/S 줄, 퍼센트 이스케이프, `$` 종결), 깔때기 `CResultMgr::Updata_Data` 훅 + 측정 스레드가 게시하는 무장(armed) 컨텍스트 + VISION grab 시점 컨텍스트(슬롯 + 대기 전 지역 사본 + 스레드 로컬), 파일 핸들 수명·공유 규칙, 압축(재개 튜플 이후 줄 삭제), 재생(dResult 빠른 업로드 + 큐 깊이·처리 수 페이싱 + STOP/일시정지 폴링), VISION 미도착 감지·**범위 제한** 재개 위치 당김(D5-12: InnerID 짝, 타임아웃 X 줄은 당기지 않음, 헤드당 최근 2 grab만, 플러시된 pass 제외, EFA는 스냅샷 있을 때만).
5. 셀 상태·래치: 스틱 완료마다 `nCellStatus` 스냅샷(3칸 링 × 20 KB, 튜플 태그), `GenGlassPath` 리셋 카운터로 복원 시점 보장, EFA 모드·플래그·pass 수 보존·비교(플래그를 **쓰지 않는다**), 재개 첫 VISION 레시피 대기(D5-15, 글라스 세대에 묶음). 나머지 래치는 Recipe/Measure 아이템이 다시 세운다(복원 안 함).
6. dResult 추가 export 2개(`API_Result_SetFastUpload`, `API_Result_GetUploadState`) — `RSRITEMDATA`·링 레이아웃 무변경.
7. OOS 검증용 디버그 키 `RESUME_JOURNAL_OOS_SIM`(기본 0) — OOS에서는 `Updata_Data`가 한 번도 안 불리므로(매퍼 B5) 저널 줄을 `LogHybridRow` 지점에서 합성.
8. 레거시 경로 결함 중 D5-10(취소 시 사이클 상태 고착, stale 재개 플래그)은 승인 시 같이 고친다(N=1 정상 런 무영향, 재개 경로만). CIM 경로는 검사를 앞으로 옮기는 대신 생기는 "팝업 중 두 번째 시작" 틈을 `ResumePromptBusy()` 가드로 막는다(리뷰 C9/K10).
9. Unload `CheckSaveReuslt`에 `ResultSaved=2` 표시 1곳(튜플 북마크만, 기존 30초 대기 재사용 — 타이밍 무변경).

*이월(기록)*
- RS1(`DoWorkGlassRecipeStep.cpp:48-151`)은 레거시 Format=1 북마크 전용으로 **무변경**(경로가 `RawData\{YYYYMMDD}`라 실기에서도 늘 SKIP — 매퍼 B4/D1). 고치지 않는다(D5-1 (c) 비권장). 문서 오기만 정정(5-7).
- CIM ONLINE REMOTE 재개 팝업이 워커 스레드 모달·타임아웃 없음(D6) — 기존 동작 유지, Phase 6/운용.
- APD `START_TIME`이 재개 시도의 시작 시각(`GetStartTime2`) — 기록, 필요 시 별건.
- PreMeas·Align이 재개 글라스에서도 세그먼트 0 기준으로 다 도는 비효율(D19) — 무해, 유지.
- 같은 초에 겹친 InnerID 재배정·자정 넘김 폴더 분할(D14) — 저널은 절대 경로를 적으므로 재개에 무관, 기록만.
- VISION 슬롯 2개가 차면 grab을 조용히 버리는 기존 동작(`QualityWaitSet`) — 라이브와 같은 손실, 무변경.
- 수동 측정(`DlgSubManual_Meas`)은 저널 대상이 아니다(무장 안 됨). 자동 런 일시정지 중 수동 측정이 가능하다면 그 행이 저널에 섞일 수 있음 → (11) 리스크, Phase 7.
- ICP "First"가 `LastSegment`를 안 지우는 결함과 "세그먼트 진입 직후 `LastStick=-1`로 이어서 불가" 결함은 **이식하지 않는다**: "아니요"는 `ClearResumeInfo()`(INI·저널·스냅샷 전부 삭제), 튜플은 "마지막 완료 스틱 + 1" 정규화라 −1 상태가 필요 없다.

---

**(1) 진행 튜플 — `ResumeInfo.ini`**

*1-1 키* (`[RESUME]` 섹션, 기존 13키 뒤에 덧붙임. `m_nFormat >= 2`일 때만 쓴다 → 레거시 글라스는 파일 바이트 동일)

| 키 | 의미 | 기본(키 없음) |
|---|---|---|
| `Format` | 2 = 튜플 북마크 | 1 (구 파일·레거시) |
| `LastPass` | 마지막 완료 스틱의 pass(0 = T0) | 0 |
| `LastSegment` | 마지막 완료 스틱의 세그먼트(0 기준) | 0 |
| `LastStick` (기존) | 그 세그먼트 안 로컬 스틱(0 기준, −1 = 없음) — 의미 그대로 | −1 |
| `SegmentCount` | N | 1 |
| `SegShape` | 세그먼트별 `{스틱 수}x{셀 수}r{회전}`를 `\|`로 연결(예 `5x3r1\|5x4r2`) | — |
| `GlassRows` | G = Σ(셀 × 스틱) | 0 |
| `PassCount` | 측정 시작 때의 pass 수(`nPassCount`, EFA rcp 항목 수 — **1이어도 EFA일 수 있다**, 리뷰 K13) | 1 |
| `EfaMode` | 측정 시작 때 `bEfaMode`(`DoWorkGlassMeasure.cpp:185-189`, `nEfaPass > 0`) | 0 |
| `EfaFlag` | 측정 시작 때 공유 `NewAgingMode`(0/1) — 비교용, 복원하지 않음(D5-9 (b)) | 0 |
| `PassBoundaryDone` | 진입 경계를 마친 가장 큰 pass(0 = 없음, 1 = T0→T1 경계 완료: T0 플러시 끝 + ONLINE REMOTE면 `EFA^0$` 송신 끝). `EfaPassBoundary` 안에서 세운다((3-4)) | 0 |
| `ResultSaved` | 0 = 아직, 1 = Result 아이템 완료(`SaveResult` 내보내기 시작), 2 = 결과 처리 끝(Unload `CheckSaveReuslt`가 `SaveResultDone` 확인, 또는 저장 게이트 0 글라스의 Result 완료) (D5-13, 리뷰 K3/C15) | 0 |
| `Attempt` | 재개 횟수(저널 압축마다 +1) | 0 |
| `JournalOK` | 이 글라스의 저널 쓰기가 한 번도 실패하지 않았음(끈적, 0이 되면 글라스 끝까지 0) (리뷰 K7) | 1 |
| `JournalLines` | 마지막 `Commit()` 때까지 성공적으로 쓴 저널 줄 수(헤더 제외). 재개 때 파일의 온전한 줄 수 ≥ 이 값이어야 한다 | 0 |
| `Fingerprint` | `{rcp}:{aging}:{efa}:{wadmp}` FNV-1a 32bit hex, 파일 없음 `-` (D5-14) | — |
| `TotalStick`·`GlsRotate` (기존) | 세그먼트 0 값 그대로(레거시 로그 `Stick=%d/%d`용) | |

예시(Half, 세그먼트 2 스틱 2까지 완료):
```
[RESUME]
GlassID=ABCDEFG1234
RecipeID=9_OCP_TEST_Hybrid_Half_260930
LotID=
InnerID=131500
LastStick=1
TotalStick=5
GlsRotate=1
Running=1
Completed=0
SaveTime=20261006-132210
StartTime=20261006-131502
InspDate=20261006
ResultPath=D:\HBS\RESULT\
Format=2
LastPass=0
LastSegment=1
SegmentCount=2
SegShape=5x3r1|5x4r2
GlassRows=35
PassCount=1
EfaMode=0
EfaFlag=0
PassBoundaryDone=0
ResultSaved=0
Attempt=0
JournalOK=1
JournalLines=27
Fingerprint=5d1c0a77:-:-:-
```

*1-2 재개점 정규화* `CResumeInfoMgr::GetResumePoint(int& rPass, int& rSeg, int& rStick)` (순수 함수, `SegShape`의 스틱 수 사용)
```
next = (LastPass, LastSegment, LastStick + 1)
if next.stick >= sticks[next.seg]  -> (pass, seg + 1, 0)
if next.seg   >= SegmentCount      -> (pass + 1, 0, 0)
if next.pass  >= PassCount         -> (rPass, rSeg, rStick) = (PassCount, 0, 0) 끝 표지, return 1 (ALL DONE, D5-13)
LastStick < 0 or 범위 밖           -> return -1 (무효)
return 0
```
- ALL DONE도 끝 표지 `(PassCount, 0, 0)`를 내보낸다(리뷰 C14) → 압축은 모든 줄을 남기고(모든 줄 < 끝 표지), VISION 당김이 T를 끝 표지보다 앞으로 낮출 수 있다.
- 이전 튜플 `pred(T)`: `(p, s, k>0) → (p, s, k−1)`, `(p, s>0, 0) → (p, s−1, sticks[s−1]−1)`, `(p>0, 0, 0) → (p−1, N−1, sticks[N−1]−1)`, `(0, 0, 0) → 없음`. 끝 표지의 pred는 `(PassCount−1, N−1, 마지막)`. 당김(4-5)·스냅샷 선택(5-1)·되감기(C1)가 같은 함수를 쓴다(`CResumeJournal::PredTuple`, 순수 함수).
- 세그먼트 경계 직후 정지(`(p, k−1, 마지막)`)는 정규화로 `(p, k, 0)`이 된다 → ICP처럼 경계에서 (pass, seg, −1)을 따로 저장할 필요가 없다(매퍼 seq S9 불필요).
- EFA T0 마지막 스틱 뒤 정지는 `(1, 0, 0)` + `PassBoundaryDone`으로 경계 재실행 여부를 가른다((3-4)).
- **0회 반복 무한 대기(매퍼 seq (0)-4) 방지**: 1(ALL DONE)이면 Measure가 루프를 돌지 않고 Init → Exit(Idle) 스텝을 직접 실행한다((3-5)). −1이면 재개 거부(팝업 단계) 또는 `SetFail(true, 0)` + 파일 유지(측정 단계, 리뷰 C2).

*1-3 쓰는 지점*

| 시점 | 호출 | 내용 | 로그(SYST) |
|---|---|---|---|
| 엔진 시작(비재개, 기존) | `SaveResumeInfo(-1)` `DoWorkGlassSequenceEngine.cpp:674` **무변경 호출** | 함수 안에서 `m_nFormat=1`과 튜플 멤버 리셋 + `CResumeJournal::DeleteFiles()`(이전 미완 글라스의 저널·스냅샷 삭제, 파일 없으면 무로그) | 기존 `Resume Save: ...` 그대로 |
| 측정 시작, 튜플 글라스 새 글라스(사이클 index>0 포함) | `BeginTupleGlass(...)` (Measure, (3-2)) | Format=2, (0,0,−1), 형상·지문·PassCount·EfaMode·EfaFlag, `JournalOK=1`, `JournalLines=0`, `ResultSaved=0`, `StartTime/InspDate` **새로**(D12 해소), 현재 GlassID(D11 해소) | N>1만 `Resume Begin: Glass=%s, Recipe=%s, segments %d, pass %d, G %d, OK=%d` (N=1 튜플은 무로그) |
| 스틱 완료(`!bExis`). **글라스의 마지막 스틱은 Exit(Idle) 스텝 실행 전**(리뷰 C10) | 저널 `Commit()`(bool) → `SaveCellStatus`(링 1칸) → `UpdateTupleProgress(nPass, nSeg, i, bJournalOK, nJournalLines)` | Last* 3개 + `JournalOK`(끈적) + `JournalLines` + SaveTime | N>1 `Resume Update: T%d seg %d/%d stick %d/%d, OK=%d`(+ 저널 실패 뒤 첫 줄에만 WARN `Resume Update: journal write failed earlier - this glass will not be resumable`); **N=1 튜플은 기존 줄 `Resume Update: Stick=%d/%d, OK=%d` 그대로** |
| EFA 경계 부수효과 완료(pass ≥ 1): 비-REMOTE·`pCIM == NULL`이면 T0 플러시 뒤, REMOTE면 `Cmd_Send_EFA0()` 직후 | `SetPassBoundaryDone(nPass)` (`EfaPassBoundary` 안, 튜플만 — 리뷰 C3/K8) | | `Resume Boundary: pass %d boundary done, OK=%d` |
| 재개 적용(측정 진입) — 압축·당김 | `ApplyCompaction(bRewind, rewindTuple, nKeptLines)` → **그다음** 저널 교체 | Attempt+1, `JournalLines` = 남긴 줄 수, 당김이면 Last* = pred(Tp)·`ResultSaved=0`(리뷰 C1/K1) | `Resume Attempt: %d, journal lines %d` / 당김이면 `Resume Rewind: T%d seg %d/%d stick %d/%d (VISION pending), OK=%d` |
| Result `CompleteCheck` 끝(`bRet == true`, 튜플만) | `MarkResultSaved(bUseMeas ? 1 : 2)` | 저장 게이트 0(전기 검사만)은 파일이 없으므로 바로 2(리뷰 K3) | N>1 `Resume ResultSaved: %d, OK=%d` |
| Unload `CheckSaveReuslt` 성공(튜플만) | `MarkResultSaved(2)` | 기존 30초 `SaveResultDone` 대기 결과 재사용(리뷰 C15) | 같음 |
| 정상 완료(기존) | `ClearResumeInfo()` `Engine.cpp:884` | INI·tmp + **저널·스냅샷 링 삭제** | 기존 `Resume Reset: info cleared` 그대로 |
| "아니요" / 검증 실패 OK(기존·신규) | `ClearResumeInfo()` | 튜플 전부 초기화(ICP "First" 결함 비이식) | 같음 |
| 알람·정지·크래시·**측정 진입 실패·재생 중 STOP** | 쓰지 않음(압축이 이미 끝났으면 그 결과만 남음 — 멱등) | Running=1 유지 → 다음 START에 제안 | — |

- 레거시 경로(Format=1): `UpdateStickProgress`·`SaveResumeInfo`·`ValidateResume`·`GetResumeStartStick`·로그 전부 무변경.
- 로드(`LoadResumeInfo`): 새 키는 `GetPrivateProfileInt` 기본값으로 읽는다(구 파일 = Format 1). Format=2면 생성자 끝에 한 줄 더: `Resume tuple: Format 2, T%d seg %d/%d stick %d, pass %d (EFA %d), boundary %d, result saved %d, attempt %d, journal ok %d lines %d` (레거시 파일이면 줄 없음 → N=1 부팅 로그 동일).
  - **파일 없음 조기 반환(`ResumeInfoMgr.cpp:41-57`)도 새 멤버를 전부 기본값으로**(리뷰 C14): `m_nFormat=1`, `m_nLastPass=0`, `m_nLastSegment=0`, `m_nSegmentCount=1`, `m_strSegShape` 빈 값, `m_nGlassRows=0`, `m_nPassCount=1`, `m_nEfaMode=0`, `m_nEfaFlag=0`, `m_nPassBoundaryDone=0`, `m_nResultSaved=0`, `m_nAttempt=0`, `m_nJournalOK=1`, `m_nJournalLines=0`, `m_strFingerprint` 빈 값. 같은 리셋 블록을 `ClearResumeInfo`와 `SaveResumeInfo`(Format 1로 되돌릴 때)가 공유한다(`ResetTupleMembers()` private).
- 원자성(D13, D5-18): `WriteToFile(tmp)` 뒤 `DeleteFile(ini)+MoveFile` 2곳(`:137-142`, `:168-173`, `DeleteFile(m_strFilePath);` n=3이므로 함수 범위로 치환)을 `MoveFileEx(tmp, ini, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)`로, 실패 시 기존 Copy 폴백. 파일 내용·로그 동일.
- 잠금: `CRITICAL_SECTION m_cs`로 Save/Update/Begin/Boundary/Mark/Rewind/Compaction/Clear/Load 직렬화(팝업 = UI 또는 CIM 워커, Measure 스레드, Result·Unload 아이템 스레드, 엔진 스레드). **잠금 순서: `m_cs` → 저널 잠금**(`ClearResumeInfo`/`SaveResumeInfo(-1)`이 `m_cs` 안에서 `CResumeJournal::DeleteFiles()`를 부른다). 저널 잠금을 쥔 채 `CResumeInfoMgr`나 dResult를 부르지 않는다(리뷰 C6/K14) — 그래서 압축은 "저널 tmp 작성(저널 잠금) → INI `ApplyCompaction`(`m_cs`) → 저널 교체(저널 잠금)" 세 단계로 나뉜다.

*1-4 북마크 수명*

| 사건 | 동작 |
|---|---|
| 새 글라스(비재개 START) | 엔진 `SaveResumeInfo(-1)`이 Format 1로 리셋 + 저널·스냅샷 삭제 → 튜플 글라스면 Measure `BeginTupleGlass`가 Format 2로 다시 시작 |
| 사이클 index > 0 | 그 글라스의 Measure 시작에서 `BeginTupleGlass`(현재 `GLASSID%s`) — 튜플 글라스는 D11 해소. 레거시 N=1은 기존대로(엔진 실행당 1회) |
| "아니요" | `ClearResumeInfo()` → 3파일 삭제 |
| 정상 완료 | `ClearResumeInfo()` |
| 알람·정지·Result/Unload 실패 | 유지. `ResultSaved=2`(저장 확인 끝 또는 게이트 0) → 다음 START는 제안 없이 Clear(중복 저장 방지). `ResultSaved=1`(내보내기 시작 뒤 Unload 확인 전 정지 — 사이에는 Unload `Move_Safety`뿐) → "저장 중 멈춤" 팝업으로 다시 저장할지 묻는다((2-3)) |
| 사이클 index > 0 글라스가 Measure 전(Align 등)에 정지 | 북마크는 아직 직전 글라스(ALL DONE). 직전 글라스가 Unload까지 끝났으면 `ResultSaved=2` → 다음 START는 조용히 Clear(리뷰 K3: 게이트 0 글라스도 2) |
| 만료(`RESUME_EXPIRE_HOURS`) | 기존과 같음(제안 안 함, 파일 유지 → 다음 새 글라스가 덮음) |

---

**(2) 검증·안내**

*2-1 진입점*

| 진입점 | 스레드 | 지금 | Phase 5 |
|---|---|---|---|
| 운전 화면 CYCLE `OnBnClickedButtonCycle` | UI | `SetCycleState(true)`·START 메시지·버튼 비활성 **뒤**에 `CheckAndApplyResumeMode()` (`:1786-1788`) → 취소 시 상태 고착(D5) | **D5-10 (a)**: 검사를 `CD_MeasureCycle ... DoModal() == IDOK` 직후(사이클 상태 변경 전)로 옮긴다. 취소 = 아무것도 안 바뀐 채 return. 검사 동안 `CResumePromptBusy` 가드(아래). 그리고 기존 가드 3개(`IsCycleState()`·`SPS_IDLE`·`EQStateChange` 스레드, `:1736-1760`) 다음, `AfxMessageBox("Do you want cycle Start sequence???")` 앞에 `ResumePromptBusy()`면 같은 알람 35 팝업 후 return(CIM 팝업이 떠 있는 동안 두 번째 시작 차단, 리뷰 C9/K10) |
| CIM `TO_SERVER_MEAS_START` (`RunStart`) | `AfxBeginThread` 워커 | 같은 순서(`:3428→:3446`) | 같은 이동(`case TO_SERVER_MEAS_START:` 블록 첫 문장)이되 **`CResumePromptBusy` 가드 안에서** 검사 — 팝업이 소유 창 없이 워커에서 뜨는 동안 CYCLE 버튼(:1737 가드 뒤의 새 가드)과 다이얼로그 START(`ResumeDialogEntry` 첫 줄)가 거부된다. 취소 = `break` 전에 바뀐 것 없음. (CIM 경로 자체는 가드를 검사하지 않는다 — 수동 팝업 중 CIM 시작이 겹치는 틈은 지금도 있고 무변경, (11)) |
| AgingTest START (`DlgSubManual_AgingTest.cpp:474-495`, UTF-8 BOM) | UI | 재개 확인·플래그 처리 없이 `EngineStart()` | **D5-11 (a)**: 공유 상태 블록(`:474` 주석 + `:475` `if (...GetSharedMemory() != NULL)`) 앞에 `ResumeDialogEntry(m_pAuroraMgr, RESUME_ENTRY_AGINGTEST)`; CANCEL이면 return |
| AgingNew(EFA) START (`DlgSubManual_AgingNew.cpp:527-550`, CP949 CRLF) | UI | 같음 | 공유 상태 블록(`:528`) 앞에 `ResumeDialogEntry(m_pAuroraMgr, RESUME_ENTRY_AGINGNEW)` |
| MultiMeas | — | 시작 경로 없음 | 해당 없음 |

- `CheckAndApplyResumeMode()` 첫머리에 튜플·레거시 하이브리드 분기만 끼운다(기존 본문 무변경):
```cpp
	// LYS 2026-10-06 Hybrid Phase 5 journal (Format=2) bookmark -> ResumePrompt.cpp; a format-1 bookmark of this same journal recipe is refused there.
	//   1-segment format-1 bookmark (or a format-1 bookmark of another recipe): the code below runs unchanged
	if (pResumeInfo && m_pAuroraMgr->GetSystemConfigMgrPtr()->IsUseResumeInspection())
	{
		int nEntry = (GetCurrentThreadId() != AfxGetApp()->m_nThreadID) ? RESUME_ENTRY_CIM : RESUME_ENTRY_OPERATE;	// only RunStart calls this off the UI thread
		if (pResumeInfo->IsTupleBookmark())
			return (ResumeTuplePrompt(m_pAuroraMgr, nEntry) != RESUME_PROMPT_CANCEL);
		if (CResumeJournal::GetInstance()->IsTupleGlass(m_pAuroraMgr->GetRecipeEditMgrPtr()->GetSegmentLoopCount())
			&& pResumeInfo->GetRecipeID().CompareNoCase(m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeID()) == 0
			&& ResumeRefuseLegacyHybrid(m_pAuroraMgr))
			return true;
	}
```
  - `IsTupleBookmark()`는 로그가 없다. `IsResumeAvailable()`(만료 시 WARN)은 각 경로가 한 번만 부르게 해서 N=1 만료 로그 개수 불변.
  - 레거시 하이브리드(Format=1 + **같은 RecipeID** + 현재 레시피가 튜플 대상)는 `IsResumeAvailable()`이 true일 때만 거부 팝업 + Clear(D5-16). RecipeID가 다르면 아래 기존 코드가 기존 "레시피/스틱 수/회전 불일치" 팝업으로 처리한다(리뷰 K12). 술어는 `N > 1`이 아니라 `IsTupleGlass(N)` — D5-1 (b) 활성(`RESUME_TUPLE_ALL=1`) 직후 남은 N=1 Format=1 북마크도 저널이 없으므로 같은 거부(리뷰 C13). D5-1 (a) 동안 N=1은 이 분기에 들어가지 않는다.
  - 호출부 두 곳은 `{ CResumePromptBusy busy; bOk = CheckAndApplyResumeMode(); }` 형태로 감싼다(가드 = `ResumePrompt.cpp`의 `static volatile LONG` + RAII, 로그 없음).
- `ResumeDialogEntry(pMgr, nEntry)`: 첫 줄 `if (ResumePromptBusy())` → 한국어 `AfxMessageBox("다른 검사 시작 확인 창이 열려 있습니다. 그 창을 먼저 닫으세요.")` + OPER `Resume: another start prompt is open - dialog START ignored` + CANCEL. 그다음 튜플 북마크면 가드 안에서 `ResumeTuplePrompt`, 아니면 `CSystemParamShare::SetResumeMode(false)`만(stale 플래그 정리, 팝업 없음 — 레거시 N=1 북마크는 지금처럼 다이얼로그 START가 무시·덮어쓴다).
- 결과: 4개 START 경로가 **모두** 재개 플래그를 세우거나 지운다 → stale 플래그 결함(D7)이 사라진다(엔진 수정 불필요).

*2-2 튜플 검증* (`ResumeTuplePrompt` 안, 순서대로. 실패 = 한국어 거부 팝업 + WARN `Resume Refuse: <영문 사유> - fresh start` / `... - start cancelled, bookmark kept`. 거부 팝업 종류(리뷰 K11):
- **되돌릴 수 있는 거부**(#2 RecipeID, #4 파일 변경, #8 GlassID, #9 EFA 모드 — 운전자의 선택 실수일 수 있는 것): `MB_OKCANCEL`. 확인 = `ClearResumeInfo()` + `SetResumeMode(false)` + 새로 시작(FRESH), 취소 = 아무것도 지우지 않고 START 취소(CANCEL — 사이클 화면에서 레시피를 다시 고르거나 AgingNew 화면으로 가서 다시 시작).
- **구조적 거부**(#3, #3b, #5, #6, #7): `MB_OK` + Clear + 새로 시작(지금 설계 그대로). #1b는 팝업 없는 정리, #7b는 안내만(거부 아님).
- **읽기 실패**(#5의 파일 열기 실패가 "없음"이 아니라 공유 위반·I/O 오류): `MB_OK` + **지우지 않고** CANCEL(`journal busy/unreadable (%u)`, "결과 기록 파일을 지금 읽을 수 없습니다. 잠시 후 다시 시작하세요.") — 리뷰 C6의 "공유 위반이 재개를 파괴" 방지.)

| # | 검사 | 현재 값 출처 | 영문 사유(로그) | 한국어 사유(팝업) |
|---|---|---|---|---|
| 1 | `IsResumeAvailable()` | INI | (false면 조용히 `SetResumeMode(false)`, 지금과 같음) | — |
| 1b | **ALL DONE + `ResultSaved=2`** (리뷰 C12/K3 — 다른 검사보다 먼저) | `GetResumePoint` = 1 | (팝업 없이) OPER `Resume: previous glass already saved - bookmark cleared` + Clear + 새로 시작 | — |
| 2 | RecipeID | `GetRecipeID()` (사이클 화면/CIM PPID가 연 레시피) | `RecipeID mismatch` | 레시피가 다릅니다 |
| 3 | `SegmentCount`·`SegShape`·`GlassRows` | `CResumeJournal::BuildShape(pRcp, ...)` | `segment shape mismatch (saved %s, now %s)` | 세그먼트 구성(개수/스틱 수/셀 수/회전)이 다릅니다 |
| 3b | 현재 레시피가 튜플 대상(`IsTupleGlass(N)`) | | `journal bookmark but this recipe keeps none` | 이전 검사 이력의 기록 방식이 현재 설정과 다릅니다 |
| 4 | `Fingerprint` (D5-14) | 같은 함수가 4파일 FNV-1a | `files changed (%s)` (rcp/aging/efa/wadmp) | 레시피 또는 에이징/EFA/WAD 다점 설정 파일이 바뀌었습니다 |
| 5 | 저널: 파일 있음, 헤더 파싱, 헤더 GlassID·RecipeID·N·G = INI, **`JournalOK=1`**, **온전한 줄 수 ≥ `JournalLines`** (리뷰 K7) | `CResumeJournal::Analyze()` | `journal missing or broken` / `journal write failed during the run` / `journal shorter than recorded (%d < %d)` | 결과 기록(저널) 파일이 없거나 손상되었습니다 / 결과 기록(저널) 쓰기에 실패한 적이 있어 이어 검사할 수 없습니다 |
| 6 | 재개점 `GetResumePoint` = −1 | | `bookmark out of range` | 이전 진행 위치가 올바르지 않습니다 |
| 7 | **EFA 글라스(`EfaMode=1`)는 pass와 무관하게** 재개점 T(당김 반영) 직전 튜플 `pred(T)`와 정확히 같은 스냅샷이 링에 있음(GlassID·체크섬 일치). T = (0,0,0)이면 해당 없음 (리뷰 K2) | `CResumeJournal::Analyze()`(링 헤더만 읽음) | `cell status snapshot for T%d seg %d stick %d missing (EFA glass)` | 셀 상태 기록이 없어 EFA 검사를 이어갈 수 없습니다 |
| 7b | (비-EFA 글라스) 정확한 스냅샷이 없으면 거부하지 않고 팝업 끝에 한 줄 "셀 상태 표시는 일부만 복원됩니다" + WARN | | `cell status snapshot for T%d seg %d stick %d missing - cell map partly restored` | (안내만) |
| 8 | CIM: INI GlassID = 현재 GlassID (D5-8) | `CSystemParamShare::GetGlassID()` | `GlassID mismatch (host %s, saved %s)` | Glass ID가 다릅니다 (호스트 %s, 이전 %s) |
| 9 | EFA 모드(D5-9 (b), 리뷰 C8/K13 — **공유 플래그를 쓰지 않고 입구 조건으로만 맞춘다**): OPERATE·CIM은 INI `EfaFlag` = 현재 `GetSys_NewAgingMode()`, AgingTest 입구는 `EfaFlag == 0`(다이얼로그가 플래그를 끈다 `:478`), AgingNew 입구는 `EfaMode == 1`(다이얼로그가 플래그를 켠다 `:531`) | `GetSys_NewAgingMode()` | `EFA mode mismatch (saved flag %d mode %d, now flag %d, entry %s)` | 검사 모드(EFA/일반)가 이전과 다릅니다. EFA 시작으로 검사한 Glass는 EFA 화면의 START로 이어서 검사할 수 있습니다 |

- 7번은 §5.8-5의 "스냅샷 없으면 T0부터로 강등 + 경고"를 **팝업 단계의 거부**로 구현한다(강등 = 새 글라스와 같은 결과이므로 운전자에게 사유를 보여 주고 새로 시작). T0에서 멈춘 EFA 글라스도 대상이다 — T1이 OSTest/Pattern Search/R-Test를 건너뛰고(`DoWorkGlassMeasure.cpp:306`) 측정 스텝이 `nCellStatus`로만 NG 셀을 거른다(`DoWorkGlassMeasureStep.cpp:2571-2612`).
- 정확한 튜플 비교라 "쓰기 실패로 남은 옛 스냅샷이 GlassID만 맞아 통과"하는 경우도 막힌다(리뷰 K2).
- 측정 진입에서 다시 확인하는 것(경합·재로드 대비): pass 수·EFA 모드(EFA 판정은 측정 시작에만 가능), 세그먼트 수·스틱 수, 저널 헤더·쓰기 상태, 스냅샷, 재생. 실패하면 **새 글라스로 강등하지 않고** WARN + `SetFail(true, 0)`로 글라스만 멈추고 파일은 그대로 둔다((3-2), 리뷰 C2/K4). 다음 START 팝업이 원인을 다시 판정한다(운전자는 아니요로 새로 시작할 수 있다).

*2-3 팝업 문구* (`ResumePrompt.cpp`, CP949. `→`·`—` 같은 CP949 의심 문자 금지)
- 재개 가능:
```
이전 검사 이력이 있습니다.

Glass ID : %s
레시피 : %s
중단 시각 : %s
진행 : [T1 ][세그먼트 %d/%d, ]스틱 %d/%d까지 완료 (전체 %d/%d 스틱)
다음 : [T1 ][세그먼트 %d ]스틱 %d부터 이어서 검사합니다. 이 스틱은 처음부터 다시 측정합니다.
[VISION 결과가 오지 않은 스틱이 있어 [T1 ][세그먼트 %d ]스틱 %d부터 다시 측정합니다.]
[현재 Glass ID : %s  (예를 누르면 이전 Glass ID로 검사합니다)]
[결과 파일 %d개를 찾을 수 없습니다. 해당 행은 비어 있게 됩니다.]
[셀 상태 표시는 일부만 복원됩니다.]

예 = 이어서 검사
아니요 = 처음부터 검사
취소 = 검사 시작 취소
```
  - `[T1 ]`은 `PassCount >= 2`일 때 `T%d `(재개 pass), `[세그먼트 ...]`는 N > 1일 때, Glass ID 줄은 수동·다이얼로그 입구에서 현재 값이 다를 때, 누락 줄은 `Analyze`가 센 누락 raw 파일 수 > 0일 때만, VISION 줄은 `Analyze`가 당김을 예상할 때만(이때 "다음" 줄 대신 이 줄), 셀 상태 줄은 (2-2) 7b일 때만.
  - "중단 시각"은 INI `SaveTime`(`20261006-132210` → `2026-10-06 13:22:10`) — 다이얼로그 입구에서는 GlassID가 부팅 기본값 `ABCDEFG1234`(`SystemParamShare.cpp:36`)로 같아 글라스를 구분할 수 없으므로 시각으로 구분한다(리뷰 K15).
  - "전체 %d/%d 스틱"은 글라스 스틱(`stickBase + local + 1` / Σ스틱) × pass로 센 완료 수.
  - 당김 예고는 팝업 시점의 저널로 계산한 **예상**이다. 팝업과 측정 진입 사이에 늦은 VISION이 도착하면 측정 진입의 실제 재개점이 더 뒤일 수 있고, 실제 값은 `[RESUME] resume point ...` 로그가 기준이다(리뷰 K1).
- 끝까지 측정됨(D5-13, `ResultSaved=0`, 당김 없음):
```
이전 검사 이력이 있습니다.

Glass ID : %s
레시피 : %s
중단 시각 : %s
모든 스틱의 측정을 마쳤지만 결과 저장 전에 멈췄습니다.

예 = 기록된 결과로 저장부터 진행 (다시 측정하지 않음)
아니요 = 처음부터 검사
취소 = 검사 시작 취소
```
- 저장 중 멈춤(D5-13, `ResultSaved=1`, 당김 없음 — 리뷰 C15): 둘째 줄 문구만 `결과 저장을 시작한 뒤 저장 확인 전에 멈췄습니다.`, 예 문구 `예 = 기록된 결과로 다시 저장 (결과 파일이 한 번 더 만들어질 수 있음)`.
- ALL DONE 북마크인데 `Analyze`가 VISION 당김을 예상하면 ALL DONE 팝업이 아니라 위의 "재개 가능" 팝업(VISION 줄 포함)을 띄운다(리뷰 K1 — "다시 측정하지 않음" 약속 위반 방지).
- 구조적 거부(`MB_OK`): `이전 검사 정보로 이어 검사할 수 없습니다.\n사유 : %s\n\n처음부터 검사합니다.`
- 되돌릴 수 있는 거부(`MB_OKCANCEL`): `이전 검사 정보로 이어 검사할 수 없습니다.\n사유 : %s\n\n확인 = 이전 기록을 지우고 처음부터 검사\n취소 = 검사 시작 취소 (이전 기록 유지)`
- 레거시 하이브리드 거부 사유: `하이브리드 레시피의 이전 검사 이력에 결과 기록(저널)이 없습니다`
- 예(YES) 적용(`ResumeTuplePrompt`):
  - 수동(OPERATE)·다이얼로그 입구: `SetGlassID(INI GlassID)`, INI LotID가 비어 있지 않으면 `SetLotID(...)` → OPER `[RESUME] GlassID restored from bookmark: %s (was %s)` (같으면 무로그). START 메시지가 검사 **뒤**에 오므로 화면도 복원된 ID를 보인다.
  - EFA 공유 플래그는 **쓰지 않는다**(리뷰 C8: 복원한 플래그가 이후 비재개 CYCLE 글라스까지 EFA로 돌리는 문제). 모드는 (2-2) 9의 입구 조건으로만 맞춘다 — 다이얼로그는 자기 플래그를 그다음에 세운다, OPERATE·CIM은 현재 플래그가 이미 북마크와 같다.
  - `SetResumeMode(true, rStick)` + 기존 로그 `Resume Start: user selected resume inspection` 재사용.
  - 아니요/취소: 기존 로그 `Resume Cancel: user selected fresh start` / `Resume Abort: user cancelled inspection start` 재사용.
- CIM ONLINE REMOTE에서도 모달(기존 D6 그대로).

---

**(3) 재개 위치**

*3-1 엔진 9아이템 — 재개 글라스에서도 전부 돈다(무변경, Phase 4 (12)-4·4b (12)-1 충족)*
- 흡착 `ResetAlign()` → Recipe(None → **Change**: `OpenRecipe` → 래치 리셋·`SyncCellStatusStickBase`(세그먼트 0) → N>1 블록(용량·마스크·`LatchHybridResultItems`·APD 헤더 G×1) → `CheckCellIdConfig`(D-15, **복원된 GlassID**로 결합) → SW 리밋 → `OrSegmentMeasMode`(D4-18) → `EXE_QUALITY_RESET/WAIT_USE` → 장치 레시피) → Glass/Probe SW Align(세그먼트 0 `.alg`) → PreMeas(세그먼트 0 스틱 0) → AGING → **Measure** → Result → Unload.
- **ICP처럼 "재개 시 레시피 재로드 생략"을 넣지 않는다.** HMI는 행·Cell ID·마스크가 레시피/사이드카에서 결정적으로 나오고 래치를 Recipe Change가 세운다. 재로드가 정지 뒤 편집을 끌어들이는 문제는 지문(D5-14)으로 막는다.
- Recipe None만 바뀐다(P16): 튜플 북마크 재개면 `ClearData()` + `MarkCellStatusReset()` + OPER `Resume mode (journal): result grid cleared - journal replay at measure start`, RS1 안 탐. 레거시는 기존 코드 그대로.

*3-2 Measure `ThreadRun` 변경 (P15, `DoWork/DoWorkGlassMeasure.cpp`)*
```cpp
	// (A) :159-167 legacy block split - format-2 bookmark is resolved after the EFA / segment decision
	size_t nStartStick = 0;
	bool bTupleResume = false;		// LYS 2026-10-06 Hybrid Phase 5 journal (Format=2) bookmark
	bool bLegacyResumeFlag = false;	// format-1 resume flag consumed here (Recipe None already ran RS1) - review C13
	if (CSystemParamShare::GetInstance()->IsResumeMode())
	{
		CResumeInfoMgr* pResumeFmt = m_pShareItem->GetResumeInfoMgrPtr();
		if (pResumeFmt != NULL && pResumeFmt->IsTupleBookmark())
			bTupleResume = true;
		else
		{
			bLegacyResumeFlag = true;
			nStartStick = ...;  if (nStartStick >= (size_t)nStepIndex) nStartStick = 0;   // existing lines, unchanged
			strLog.Format(_T("Resume: starting from stick %d (total %d)"), ...);          // existing log, unchanged
		}
	}
	CSystemParamShare::GetInstance()->SetResumeMode(false);   // existing, unchanged

	// (B) right after "m_nCurSeg = (pSegFunc != NULL) ? pSegFunc->GetActiveSegmentIndex() : 0;" (:197), before the [HYBRID] measure start log
	CResumeJournal::GetInstance()->Disarm();	// every glass, no log, no file I/O when nothing is open: drops a stale arm and the D5-15 flag (review C7)
	CResumeInfoMgr* pResumeT = m_pShareItem->GetResumeInfoMgrPtr();
	bool bTupleGlass = (pResumeT != NULL && m_pShareItem->GetSystemConfigMgrPtr()->IsUseResumeInspection()
						&& CResumeJournal::GetInstance()->IsTupleGlass(nSegCount));
	int  nResumePass = 0, nResumeSeg = 0;
	bool bSkipPassBoundary = false, bAllDone = false, bForceFirstBoundary = false, bTupleLastSaved = false;
	if (bTupleGlass)
	{
		int nEnter = RESUME_ENTER_NEW;
		if (bTupleResume)
			nEnter = ResumeTupleEnter(nPassCount, bEfaMode, nSegCount, nResumePass, nResumeSeg, nStartStick, bSkipPassBoundary, bAllDone, bForceFirstBoundary);
		if (nEnter == RESUME_ENTER_NEW)	// new glass, or a pull-back to (0,0,0) (nothing kept - the only demotion left)
		{
			nResumePass = nResumeSeg = 0; nStartStick = 0; bSkipPassBoundary = bAllDone = bForceFirstBoundary = false;
			ResumeTupleBegin(nPassCount, bEfaMode, nSegCount, bTupleResume || bLegacyResumeFlag);	// bDemoted: clear RS1 / replayed rows
		}
		else if (nEnter != RESUME_ENTER_OK)	// RESUME_ENTER_STOP / RESUME_ENTER_FAIL: files untouched (or compacted idempotently), glass stops
		{
			if (nEnter == RESUME_ENTER_FAIL && GetFail() != true && GetStopUser() != true) SetFail(true, 0);	// no alarm id (<1000 ignored), engine logs "Fail Exit"
			SegmentRestore0();
			GoToExitStep();
			CDoWorkGlassMeasureStep* pExitItem = GetSeqItemStep();
			if (pExitItem) pExitItem->Run();	// Idle: complete event (stop) / no complete (fail) - same as the existing stop / NG paths
			bExis = true;
		}
	}
	else if (bTupleResume)
	{	// journal bookmark but this glass keeps none (popup check 3b should have refused): keep the files, stop the glass (review C2/C13)
		m_pLogFileAccessor->KMLOG(WARN, _T("[RESUME] journal bookmark but this recipe keeps no journal - glass stopped, bookmark kept"));
		... same SetFail(true, 0) + exit-step block as above; bExis = true;
	}

	// (C) loops
	for (int nPass = nResumePass; nPass < nPassCount && bAllDone == false && bExis == false; nPass++)   // was: nPass = 0; nPass < nPassCount
	   ... if (nPass >= 1 && !(bSkipPassBoundary && nPass == nResumePass) && EfaPassBoundary(nPass, bTupleGlass) == false) { existing abort }   // flag set inside (review C3/K8)
	       if (nPass >= 1 && bSkipPassBoundary && nPass == nResumePass) OPER "EFA: pass boundary skipped on resume (T0 already flushed, EFA^0$ already sent)"
	   for (int nSeg = (nPass == nResumePass) ? nResumeSeg : 0; nSeg < nSegCount; nSeg++)        // was: nSeg = 0
	      boundary (D5-6 (b); bForceFirstBoundary is false except on the first resumed segment):
	          if ((nSeg != m_nCurSeg || bForceFirstBoundary) && SegmentBoundary(m_nCurSeg, nSeg, bForceFirstBoundary) == false) { existing abort }
	          bForceFirstBoundary = false;
	      for (size_t i = ((nPass == nResumePass && nSeg == nResumeSeg) ? nStartStick : 0); ...)  // was: (nPass == 0 && nSeg == 0)
	          if (bTupleGlass) CResumeJournal::GetInstance()->SetStick(nPass, nSeg, (int)i, ((nSegCount > 1) ? pSegFunc->GetSegmentStickBase(nSeg) : 0) + (int)i);
	          GoToInitStep();  // Init   (existing)
	          ...
	          top of the step loop (anchor "if (GetCurStep() == GetExitStepIndex()) SegmentRestore0();", n=1) - reached only on the glass's last stick,
	          after the Exit-1 step returned true and no stop was pending, before Idle releases the engine (review C10):
	              if (GetCurStep() == GetExitStepIndex())
	              {
	                  if (bTupleGlass && bTupleLastSaved == false) { TupleStickDone(nPass, nSeg, i); bTupleLastSaved = true; }
	                  SegmentRestore0();     // existing statement
	              }
	          progress (anchor "if (nSeg == 0) pResume->UpdateStickProgress((int)i);"):
	              if (bTupleGlass) { if (bTupleLastSaved == false) TupleStickDone(nPass, nSeg, i); }
	              else if (nSeg == 0) pResume->UpdateStickProgress((int)i);   // existing statement, unchanged

	// (D) after the pass loop (anchor "if (bExis == true) break;\t// LYS 2026-09-28 EFA:" + "\t}\r\n"), before the final SegmentRestore0()
	if (bAllDone && bExis == false) { ... (3-5) ... }
	CResumeJournal::GetInstance()->Disarm();	// every glass (no-op when not armed)
```
- `TupleStickDone(nPass, nSeg, i)`(신규 private): 글라스 스틱 `gs = ((nSegCount > 1) ? GetSegmentStickBase(nSeg) : 0) + i` → `bool bOK = CResumeJournal::Commit()` → `SaveCellStatus(nCellStatus, nPass, nSeg, i)` → `pResumeT->UpdateTupleProgress(nPass, nSeg, i, bOK, CResumeJournal::GetLineCount())`. 쓰기 순서는 저널 → 스냅샷 → INI(5-1의 근거).
- 레거시(N=1 기본): `nResumePass = nResumeSeg = 0`, `bAllDone = bSkipPassBoundary = bForceFirstBoundary = false`, `bTupleGlass = false`, `bExis = false` → 루프 시작식·조건은 지금과 같은 값, `EfaPassBoundary(nPass, false)`는 플래그를 안 쓰고, 마지막 스틱 훅은 `SegmentRestore0()`만, 진행 저장은 기존 문장, `Disarm()` 2회는 무장 안 된 상태라 아무 일도 없다(로그·파일 0). 새 로그 0줄.
- `[HYBRID] measure start: ... resume start stick %d`(N>1)는 (B) 뒤에 찍히므로 해석된 로컬 스틱을 보인다(형식 무변경). 진입 실패·STOP이면 이 줄 뒤에 루프가 돌지 않는다.
- 글라스 스틱은 `GetSegmentStickBase(nSeg) + i`로 계산한다. `ToCellStatusStick(i)`는 활성 세그먼트 base를 쓰므로 `SegmentRestore0()` 뒤에는 틀린다(마지막 스틱 훅은 `SegmentRestore0()` **앞**에서 부르지만 base 직접 계산으로 통일).
- `ResumeTupleEnter`(신규 private, `.h` P14) → `RESUME_ENTER_OK / NEW / STOP / FAIL`. **어느 단계도 북마크를 새 글라스로 되돌리거나 저널을 자르지 않는다**(리뷰 C2/K4). 순서:
  1. `pResumeT->GetPassCount() == nPassCount`, `GetEfaMode() == bEfaMode`(리뷰 K13), `GetSegmentCount() == nSegCount`, `SegShape`의 스틱 수 = 라이브 `GetSegmentStickCount(k)`(N=1은 `m_vecStick.size()`) — 다르면 FAIL(`why`).
  2. `GetResumePoint(rPass, rSeg, rStick)` → 1이면 `bAllDone`(끝 표지 T), −1이면 FAIL.
  3. `CResumeJournal::Analyze(bookmark, plan)`(읽기 전용, 팝업과 같은 함수): 헤더(GlassID = 현재 GlassID(팝업이 복원), RecipeID, N, PassCount, G), `JournalOK`, 온전한 줄 수 ≥ `JournalLines`, 찢긴 끝 줄, InnerID 짝, **VISION 당김 후보**(4-5)와 스냅샷 링 → 결과 `plan.T`(당김 반영), `plan.bRewind`, `plan.nKeep`. 실패·읽기 실패(공유 위반) → FAIL.
  4. `plan.T == (0,0,0)`(당김이 글라스 시작까지) → OPER `[RESUME] VISION pending on T0 seg 1 stick 1 - glass restarts as a new glass` → NEW(남길 줄이 없으므로 새 글라스가 곧 정확한 결과, 리뷰 C1).
  5. EFA 글라스(`EfaMode=1`)인데 `pred(T)` 스냅샷이 링에 없음 → FAIL(`cell status snapshot for T%d seg %d stick %d missing`). 비-EFA는 WARN만.
  6. 압축 3단계((1-3) 잠금 규칙): `CResumeJournal::WriteCompacted(plan)`(tmp, seq 이어감, 헤더 Attempt+1, 짝 없는 P 중 당김 자격이 없는 것은 버림) → `pResumeT->ApplyCompaction(plan.bRewind, pred(plan.T), plan.nKeep)`(**INI 먼저**: Attempt+1, `JournalLines`, 당김이면 Last* = pred(T)·`ResultSaved=0` — 리뷰 C1/K1) → `CResumeJournal::SwapCompacted()`(저널 잠금 안에서 핸들 닫고 `MoveFileEx`, `nGen++`). 어느 틈에서 멈춰도: INI가 먼저 바뀌었으면 다음 압축이 옛 저널에서 같은 T로 같은 결과를 낸다(멱등), INI 쓰기 실패면 FAIL(저널 무변경).
  7. `Replay(...)`((4-4)) — 사용자 STOP이면 STOP(WARN `[RESUME] journal replay stopped by user at %d/%d - bookmark and journal kept`), 무진행·예외면 FAIL(WARN `[RESUME] journal replay failed (%s) - glass stopped, bookmark and journal kept`).
  8. 스냅샷 복원(5-1): `WaitCellStatusReset(5000)`(STOP/일시정지 폴링, 실패 시 WARN `[RESUME] cell status reset not seen within 5 s - restored anyway`) → `LoadCellStatus(pred(T))`(정확한 튜플, 없으면 비-EFA만 가장 최근 < T + 열 ≥ gs(T) IDLE) → OPER `[RESUME] cell status restored (snapshot after T%d seg %d stick %d)`.
  9. `bSkipPassBoundary = (rPass >= 1 && GetPassBoundaryDone() >= rPass)`; `bForceFirstBoundary = ResumeForceBoundary(rSeg)`((3-3), D5-6 (b)); `bAllDone`이 아니면 `ArmResumeVisionWait()`(D5-15, 현재 세대에 묶임); `Arm(Attempt)`.
  10. OPER `[RESUME] resume point T%d seg %d/%d stick %d/%d (glass stick %d/%d), pass boundary done %d, attempt %d` → OK.
- `ResumeTupleBegin(nPassCount, bEfaMode, nSegCount, bDemoted)`: `BuildShape` → `BeginTupleGlass(GlassID, RecipeID, LotID, InnerID, ResultFolder, stick0, rot0, shape, nPassCount, bEfaMode, GetSys_NewAgingMode(), bLog = nSegCount > 1)` → `CResumeJournal::Begin(...)`(잘라 쓰기, 헤더, 스냅샷 링 삭제) → `bDemoted`(튜플 재개가 새 글라스가 된 경우, 또는 **레거시 재개 플래그로 RS1이 돈 글라스** — 리뷰 C13)면 `ClearData()` → `Arm(0)` → N>1만 OPER `[RESUME] new glass bookmark: GlassID %s, %d segments, %d pass, G %d`.
- (삭제) `ResumeTupleDowngrade` — 튜플 북마크 + 튜플 아닌 글라스는 팝업 3b가 거부하고, 측정 진입에서 만나면(경합) 파일을 지우지 않고 FAIL. 강등을 다시 넣는다면 엔진 시작과 같은 `SaveResumeInfo(GlassID, RecipeID, -1, true, false, nStickTotal, LotID, InnerID, ResultFolder, nGlsRotate)`로 모든 식별 필드를 다시 쓰고 `DeleteFiles`까지 해야 한다(리뷰 C13, `DoWorkGlassSequenceEngine.cpp:666-683`).

*3-3 세그먼트 경계·캐시*
- 측정 진입 때 `m_nCurSeg = 0`(RecipeChange가 세그먼트 0 활성). (seg k ≥ 1)로 재개하면 기존 줄 `if (nSeg != m_nCurSeg && SegmentBoundary(m_nCurSeg, nSeg) == false)`가 **자동으로** `SegmentBoundary(0, k)`를 부른다 → `SegmentSwitchData(k)`가 `SetActiveSegment` + `SyncCellStatusStickBase()`(rowbase·letterbase·stick base·seg tag) + 세그먼트 `.alg` 재로드 → PG/WAD 레시피명이 바뀌면 재송신(비교 기준 = 세그먼트 0 = Recipe Change가 방금 보낸 이름, 맞음) → 회전이 다르면 Safety → R → Glass SW Align. 정상 런의 0 → k 전환과 같다. `SetActiveSegment` 직접 호출은 넣지 않는다(Phase 4 (12)-3).
- EFA T1 재개 (1, k, j): 정상 T1은 N−1 → 0 복귀 경계로 시작하지만, 재개는 엔진이 세그먼트 0으로 맞춘 상태에서 시작하므로 `SegmentBoundary(0, k)`(k ≥ 1) 또는 경계 없음(k = 0)이 맞다.
- **강제 경계는 "중단 없는 런과 같아지는 경우에만" 넣는 것을 권장(D5-6 (b), §5.8-3 문구 변경이라 사용자 승인 필요 — 리뷰 C11/K5)**. 근거와 한계:
  - §5.8-3의 전제 "Stage 상태를 알 수 없음"은 HMI에서 성립하지 않는다 — 측정 아이템 전에 흡착 `ResetAlign()`, 세그먼트 0 `.alg` Glass/Probe SW Align(Align 아이템이 `.alg` StageRotate로 R을 맞춤), PreMeas 세그먼트 0 스틱 0 이동이 매번 돈다. ICP가 늘 재Align하는 것은 ICP 측정 프로세스가 Align을 다시 하지 않기 때문이다(`!bHasPrevGlassDir`).
  - 정상 런의 `SegmentBoundary`는 **회전이 바뀔 때만** 재Align하고(`DoWorkGlassMeasure.cpp:641` `bool  bTurn\t\t= (nRotFrom != nRotTo);`), 그때 대상 세그먼트의 `.alg`로 한다(`SegmentSwitchData(nTo)` `:583-590` → GLASS_SW_ALIGN `:754-766`). 같은 방향 경계는 직전 Glass Align 오프셋을 유지한다. 따라서 정상 런의 세그먼트 k는 "0..k 중 마지막 회전 경계 m의 `.alg`로 잰 Align"(회전 경계가 없으면 엔진의 세그먼트 0 Align)을 쓴다.
  - 재개 `SegmentBoundary(0, k)`는 rot(k) ≠ rot(0)이면 세그먼트 k `.alg`로 재Align, 같으면 엔진의 세그먼트 0 Align을 쓴다. 이전 플랜의 "강제 안 함 = 중단 없는 런과 같음"은 **N=2에서만** 맞다. 반례(N ≥ 3): 회전 (1, 2, 1)에서 세그먼트 3 재개 — 정상 런은 2→3 경계에서 세그먼트 3 `.alg`로 재Align했지만, 재개는 방향이 세그먼트 0과 같아 엔진 Align을 쓴다.
  - (b) 규칙 `ResumeForceBoundary(k)` = `k >= 1 && rot(k) == rot(0) && 1..k 안에 rot(j) != rot(j−1)인 j가 있음`(순수 함수, `p5_journal_sim.pl` 표 시험). 이때만 첫 재개 세그먼트에서 `bForce`로 Safety → R → 세그먼트 k `.alg` 재Align. rot(k) ≠ rot(0)이면 `bTurn`이 이미 참이라 강제할 필요가 없고, 회전 경계가 없으면 엔진 Align이 정상 런과 같다.
  - 남는 차이(기록): 마지막 회전 경계 m의 `.alg` 이름 ≠ 세그먼트 k의 `.alg` 이름이면(m..k가 같은 방향인데 세그먼트마다 다른 Align 레시피) 정상 런은 `.alg_m`의 피두셜로 잰 오프셋, 재개는 `.alg_k`로 잰 오프셋을 쓴다. 닫으려면 `.alg_m`으로 Align한 뒤 `.alg_k`를 다시 여는 경로가 필요 — Phase 5 범위 밖, (12).
  - 구현: `bool SegmentBoundary(int nFrom, int nTo, bool bForce = false);`(`.h`, 앵커 `SegmentBoundary(int nFrom, int nTo);` n=1), 정의 `bool CDoWorkGlassMeasure::SegmentBoundary(int nFrom, int nTo)\r\n` n=1 → `(int nFrom, int nTo, bool bForce)`, `bool  bTurn\t\t= (nRotFrom != nRotTo) || bForce;`(앵커 n=1), 경계 로그에 bForce일 때만 ` (resume: forced re-align)`, 호출부 `if ((nSeg != m_nCurSeg || bForceFirstBoundary) && SegmentBoundary(m_nCurSeg, nSeg, bForceFirstBoundary) == false)`(앵커 `if (nSeg != m_nCurSeg && SegmentBoundary(m_nCurSeg, nSeg) == false)` n=1). 정상 런은 `bForceFirstBoundary == false`라 조건·인자·로그 동일. 비용 약 7~9 s + Align(해당 패턴에서만).
  - (a) 늘 강제(§5.8-3 문구 그대로): `bForceFirstBoundary = (rSeg >= 1)`. (c) 강제 안 함(이전 권장): N ≥ 3 위 반례에서 다른 결과.

*3-4 재개 스틱 전체 재측정 + P0-5*
- HMI는 셀 완료 플래그가 없고 스틱을 늘 Init부터 잰다. 시작 = "마지막 완료 + 1" = 중단 스틱 → ICP 411f449 정책(그 스틱 전체, 새 InnerID 시도, 대상 추가 없음)이 자동 성립. AgingTest 반복·EFA 항목·WAD MP 블록은 `AgingCycleBegin`이 스틱마다 0부터 → 같은 행에 다시 쓴다.
- "섞지 않음": 저널 재생이 중단 스틱(≥ 재개 튜플)의 줄을 버리고 그리드를 `ClearData()`부터 다시 채우므로, 첫 시도의 반쯤 잰 행(재측정 때 NG로 빠지는 셀 포함)이 남지 않는다(매퍼 seq D5-j, 같은 세션·재시작 모두).
- **P0-5 해소(튜플 글라스)**: 진행이 pass를 갖고 시작식이 (rPass, rSeg)에 적용되므로 T1 중단은 T1에서 이어진다. 레거시 N=1 EFA는 D5-1 (b) 승인·활성화 뒤에만 해소(그전까지 운용 지침 "N=1 EFA T1 중단 → 아니요" 유지).
- EFA 경계: `PassBoundaryDone >= rPass`면 생략(T0 파일·APD FTP·`EFA^0$` 재송신 없음). 플래그는 `EfaPassBoundary(nPass, bTupleGlass)` **안에서** 부수효과가 끝난 지점에 세운다(리뷰 C3/K8 — 함수 반환값은 끝의 `IfPauseEvt_WaitResumeEvt()`(`:563`)가 정지면 false라 부수효과와 무관):
  - 비-REMOTE: `EFA: not ONLINE REMOTE - skip APD upload / EFA^0$` 로그 뒤 `return true;` 직전(앵커 `\t\tm_pLogFileAccessor->KMLOG(OPER, _T("EFA: not ONLINE REMOTE - skip APD upload / EFA^0$"));\r\n\t\treturn true;\r\n` n=1) — T0 플러시가 끝난 뒤.
  - `pCIM == NULL` 조기 반환 직전(앵커 `\tif (pCIM == NULL) return true;\r\n` n=1 → `{ 플래그; return true; }`).
  - REMOTE: `pCIM->Cmd_Send_EFA0();` **직후**(앵커 `\tpCIM->Cmd_Send_EFA0();\r\n` n=1) — ACK 대기(5 s) 중 크래시여도 `EFA^0$` 재송신이 없다. ACK는 D-2대로 결과와 무관하게 T1이 진행되므로 "완료"의 조건이 아니다.
  - `:513`의 진입 정지 검사(플러시 전)에서 나가면 플래그 0 → 재개 때 경계를 다시 돈다(아무것도 안 보냈으므로 맞다). 플러시 뒤 APD FTP 대기 중 정지(REMOTE)면 플래그 0 → T0 파일 재저장·APD 재업로드 1회, `EFA^0$`는 1회뿐 — 기록.
  - 재생이 완전히 끝난 뒤라 경계의 `SaveResult`(5초 `UploadDataDone` 대기, B3)는 정상 동작.

*3-5 ALL DONE (D5-13 (a))*
```cpp
	// LYS 2026-10-06 Hybrid Phase 5 (D5-13) every stick of this glass was measured before the stop: the replayed grid goes to Result / Unload
	if (bAllDone)
	{
		m_pLogFileAccessor->KMLOG(OPER, _T("[RESUME] all sticks already measured - results replayed, measure item completes for Result / Unload"));
		GoToInitStep();
		CDoWorkGlassMeasureStep* pInit = GetSeqItemStep();
		if (pInit) { pInit->SetMeasureStick(0); pInit->Run(); }	// resets Fail/Stop/Complete; stick 0 = always valid (a stale index could exceed segment 0)
		GoToExitStep();
		CDoWorkGlassMeasureStep* pExit = GetSeqItemStep();
		if (pExit) pExit->Run();	// Idle: SetComplete + complete event -> the engine does not wait forever (seq (0)-4)
	}
```
- `ResultSaved=2`면 팝업 단계에서 제안 없이 Clear((2-2) 1b), `ResultSaved=1`이면 "저장 중 멈춤" 팝업((2-3)). VISION 당김이 ALL DONE을 해제하면 일반 재개가 된다((4-5)).
- Init 스텝이 `SetStopUser(false)`를 하므로(`DoWorkGlassMeasureStep.cpp:557`) 재생 끝 ~ Init 사이의 짧은 틈에 누른 STOP은 사라질 수 있다(Result가 그대로 진행) — 재생 중 STOP은 (4-4)가 처리하므로 남는 틈은 수 ms, 기록.

*3-6 Recipe Change N>1 블록·D-15·장치 레시피·VISION 대기*
- 블록·D-15·장치 레시피 송신은 재개 때도 Recipe Change가 그대로 돈다(무변경). D-15 래치는 YES 적용으로 복원된 GlassID에 묶인다 → 재측정 스틱의 Cell ID가 첫 시도와 같다(Cell ID 무상태, Phase 4 (12)-6).
- VISION 레시피 완료 대기(D5-15 (a), P17): `DoWorkGlassMeasureStep.cpp:3897`
```cpp
					bool bResumeVisWait = (i == 0) && CResumeJournal::GetInstance()->ConsumeResumeVisionWait();	// LYS 2026-10-06 Hybrid Phase 5 (D5-15) first VISION after a journal resume waits once, whatever the stick (normal runs: never armed)
					if ((iStick == 0 || bResumeVisWait) && i == 0 && m_pShareItem->GetRecipeEditMgrPtr()->TakeVisionRcpWait())
```
  - 하이브리드: `TakeVisionRcpWait()` 래치 0 → 첫 VISION에서 1회 대기, 이후 false(auto-reset 이벤트 두 번째 대기 → `HEAD_ACK_TIMEOUT` 방지 유지).
  - 재개 스틱이 0이면 `iStick == 0`으로 대기하고 플래그는 같은 줄에서 소비 → 이중 대기 없음.
  - N=1 튜플(D5-1 (b)): `TakeVisionRcpWait()`는 N=1이면 늘 true이므로, 플래그를 첫 VISION에서 반드시 소비하게 한 위 식이 다음 스틱의 두 번째 대기를 막는다. 레거시 N=1 재개(j > 0)는 지금처럼 대기 0회.
  - **플래그 수명(리뷰 C7)**: 재개 글라스가 첫 VISION 전에 멈추면 플래그가 다음 글라스로 넘어가 N=1 정상 런의 스틱 ≥1 첫 VISION에 없던 대기를 만든다. 그래서 ① `ArmResumeVisionWait()`는 현재 무장 세대(`nGen`)를 함께 기억하고, ② `ConsumeResumeVisionWait()`는 "플래그가 서 있고 **지금 무장 중이며 세대가 같을 때만**" true(그 외에는 플래그만 지우고 false), ③ `Disarm()`(측정 시작·끝에서 모든 글라스가 부름)이 플래그를 지운다. 비튜플 글라스는 무장되지 않으므로 늘 false.

---

**(4) 결과 저널 — `Config\ResumeJournal.csv`**

*4-1 위치·수명 (D5-4 (a))*
- `{AppDir}\Config\ResumeJournal.csv`, 스냅샷 링 `ResumeCellStatus0.dat`~`ResumeCellStatus2.dat`, 임시 `*.tmp` — `ResumeInfo.ini`와 같은 폴더(생성자 인자 `strConfigPath`). 단일 슬롯(글라스 1장), 북마크와 같이 삭제된다(`ClearResumeInfo`, 엔진 시작 `SaveResumeInfo(-1)`).
- 결과 트리(`RawData\{DD}\...`)를 쓰지 않는 이유: DD(일)만인 날짜 폴더·자정 분할·GlassID 없는 AgingTest(빈 GlassID → `RawData\{dd}\\`) — 매퍼 result 3.1.

*4-2 형식* (ASCII/UTF-8, CRLF, 필드 구분 `,`, 필드 안의 `%` `,` CR LF TAB은 `%25 %2C %0D %0A %09`, 줄 끝 토큰 `$` — 찢긴 끝 줄 판별)
```
#RJ1,<GlassID>,<RecipeID>,<SegCount>,<PassCount>,<G>,<BeginTime>,<Attempt>
R,<seq>,<att>,<pass>,<seg>,<stick>,<gstick>,<kind>,<nMeasMode>,<iMeasArea>,<m_iProbe>,<iCurArea>,<iCurStep>,<iPanelNo>,<nUseResultFile>,
  <m_iTValue>,<m_nMeasType>,<m_nMeasAreaCnt>,<m_nMaxMeasureRcpFlag>,<m_iRetryCnt>,<m_iRetryReason>,<nMultiPoint>,<dOffsetRX>,<dOffsetGX>,<dOffsetBY>,
  <nCntData>,{<szName>,<szData>,<nReason>}*nCntData,<szCellData>,<szLotID>,<szGlassID>,<szRecipeID>,<szResultPath>,<szResultFile>,<szImageFile>,$
P,<seq>,<att>,<pass>,<seg>,<stick>,<gstick>,VISION,<head>,<iMeasArea>,<InnerID>,$
X,<seq>,<att>,<pass>,<seg>,<stick>,<gstick>,VISION,<head>,<iMeasArea>,<InnerID>,TIMEOUT,$
S,<seq>,<att>,<pass>,<seg>,<stick>,<gstick>,<kind>,<nMeasMode>,<iMeasArea>,<CellID>,$
```
- P의 `<head>` = 그 VISION 객체의 `m_iDeviceIndex`(`CommTcpIpObjectVision.h:80`, 생성자 `:38`) — 슬롯 `QUALITY_MAX` 2개가 객체마다 있으므로 당김 자격(4-5)을 헤드별로 센다(리뷰 C4).
- X = 그 grab의 결과 파일이 150 s 안에 오지 않아 슬롯이 비워짐(`QualityWaitThread` 타임아웃 분기, 리뷰 C4/K6). 라이브 런에서도 그 행은 비므로 P를 닫고 당기지 않는다. 저널 전용 — 라이브 동작 무변경.
- R = 그리드에 올린 행(무장 상태의 `Updata_Data` 1회 = 1줄). `RSRITEMDATA`의 큰 배열을 빼고 **dResult가 읽는 필드 전부**(매퍼 result 1.2 표)를 무손실로 적는다: double은 `%.17g`, `stData`는 `nCntData`개만, kind는 모드에서(`PGM_MODE_LUMIN` → WAD, VISION, ANGLE, COLOR). 재생은 `ZeroMemory` 뒤 채운다(라이브도 안 쓰는 필드는 0).
- P = VISION grab 대기(측정 스레드, `QualityWaitSet`에서 슬롯을 실제로 잡았을 때만). R VISION 줄(`szResultFile` = `{InnerID}.txt`, 그리고 재생 시점에 raw 파일이 있을 것) 또는 X 줄과 **InnerID로** 짝짓는다(같은 InnerID가 여럿이면 P 뒤의 첫 R/X). raw 파일이 없는 R VISION 줄은 P를 닫지 않는다 — 슬롯 재사용으로 엉뚱한 InnerID가 파일 없이 토스된 경우(아래 4-3)를 잡는다(리뷰 K6). S(시뮬)는 (pass, seg, stick, iMeasArea)로 짝짓는다.
- S = OOS 합성 행(`RESUME_JOURNAL_OOS_SIM`, D5-17) — 실기에서는 생기지 않는다.
- **`RSRITEMDATA`를 바이너리로 쓰지 않고 레이아웃도 안 바꾼다**(64,736 B, 두 모듈이 링 크기를 이 sizeof로 잡음).
- 예시(행 값은 설명용):
```
#RJ1,ABCDEFG1234,9_OCP_TEST_Hybrid_Half_260930,2,1,35,20261006-131502,0
R,1,0,0,0,0,0,WAD,<LUMIN>,1,0,0,0,0,0,0,0,0,0,1,0,1,0,0,0,6,Y,123.45,0,X,78.9,0,,,0,,,0,CELL MAX,35,0,STICK MAX,1,0,ABCDEFG123A018,,ABCDEFG1234,9_OCP_TEST_Hybrid_Half_260930,D:\HBS\RESULT\RawData\06\ABCDEFG1234\WAD\,1131512_1.dat,,$
P,2,0,0,0,0,0,VISION,0,1,1131520,$
R,3,0,0,0,0,0,VISION,<VISION>,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,,,ABCDEFG1234,9_OCP_TEST_Hybrid_Half_260930,D:\HBS\RESULT\RawData\06\ABCDEFG1234\VISION\,1131520.txt,,$
```
- 크기: 줄당 약 250~400 B. 하이브리드 EFA G=35 × 3종 × 2 pass ≈ 210줄 ≈ 60~85 KB, AgingTest 최악 2268줄 ≈ 0.9 MB, 상한 15,000줄 ≈ 6 MB.

*4-3 기록 지점 (D5-5 (a)) — 스레드 근거*

| 호출자 | 스레드 | 컨텍스트 |
|---|---|---|
| WAD `TossResultFile` → `Updata_Data` (`CommTcpIpObjectWAD.cpp:1357`) | 측정 스레드(`DoWorkInspectEnd` 안, 동기) | 현재 무장 컨텍스트 = 측정 중인 (pass, seg, stick) |
| ANGLE (`CommTcpIpObjectAngle.cpp:853`) | Angle 수신 스레드, 측정 스레드는 `INSP_COMPLETE_OK`까지 대기 | 현재 무장 컨텍스트 유효(무장 해제 뒤 늦게 오면 버림 — (11)) |
| VISION (`CommTcpIpObjectVision.cpp:1516` ← QualityWaitThread `:1544`) | `QualityWaitThread`, grab 뒤 최대 150 s(정지 뒤·Result 중에도 도착) | **grab 시점** 슬롯 컨텍스트(대기 **전** 지역 사본 → 스레드 로컬로 전달) |
| VISION quality-wait 꺼짐 경로(`:761`, 수신 스레드) | 측정 스레드 대기 중 | 현재 무장 컨텍스트(자동 런에서는 `EXE_QUALITY_WAIT_USE TRUE`라 안 쓰임, 리뷰 확인) |
| RS1 / 재생 | Recipe / Measure 스레드 | 무장 안 됨 → 기록 안 함(재생이 저널을 다시 쓰지 않는다) |
| 수동 측정(`DlgSubManual_Meas`) | 수동 워커 | 무장 안 됨 → 기록 안 함. 수동 측정 중 `QualityWaitSet`(`Vision.cpp:413`)이 부르는 `AppendPending`도 `nGen == 0`이라 버림 |

- 훅: `CResultMgr::Updata_Data`(`Manager/ResultMgr.cpp:51`, 앵커 `::API_Result_Upload(result);` n=1) 뒤에 `CResumeJournal::GetInstance()->OnUpload(result);` 1줄. 1인자 시그니처 그대로.
- `OnUpload`(리뷰 C6/K14):
  - 스레드 로컬 오버라이드가 **설정돼 있으면 그것만** 쓴다 — 오버라이드의 `nGen == 0`(grab 때 무장 안 됨, 또는 슬롯이 대기 중 재사용됨)이면 버림, 현재 무장 컨텍스트로 넘어가지 않는다.
  - 오버라이드가 없으면 현재 무장 컨텍스트. **무장 안 됨이면 잠금을 잡지 않고 즉시 반환**(`volatile LONG m_nArmedGen` 한 번 읽기) — 일시정지(`SuspendThread`, `BaseSequenceImpl.cpp:144-149`)로 멈춘 스레드가 잠금을 쥔 채 다른 업로드 스레드(수동 측정, Angle)를 세우는 경우를 줄인다.
  - `nGen != m_nGlassGen`(다른 글라스·이전 시도)이면 버림(SYST `ResumeJournal: late VISION row dropped (generation %d/%d) row(%d)`).
  - 저널 잠금 안에서는 포맷·`WriteFile` 1회만 하고, dResult·`CResumeInfoMgr`를 부르지 않는다.
- 무장 수명: `Arm(attempt)` = 측정 진입((3-2) B 끝), `SetStick(...)` = 스틱 시작, `Disarm()` = 측정 시작과 `ThreadRun` 끝(모든 글라스). `nGen`(= `m_nGlassGen`)은 `Begin`·압축 교체·`DeleteFiles`마다 증가 → 정상 완료로 저널이 지워진 뒤, 또는 다음 재개의 압축 뒤에 늦게 온 VISION은 버려진다. 정지 뒤 ~ 다음 START 압축 전에 온 VISION(세대 같음)은 **기록한다**(마지막 스틱들의 VISION이 Result 중·정지 뒤에 오므로 필요).
- VISION 슬롯(P19/P20): `S_QualityWait`에 `S_ResumeCtx m_sJournal;` 추가(`Clear()`에서 0). `QualityWaitSet`이 `m_bWaitStatus = TRUE` 직전에 `m_sJournal = Capture()` + `(IsOutOfService() == false || GetSimMode() > 0)`이면 `AppendPending(m_sJournal, m_iDeviceIndex, nSheetRow, strInnerID)`(빈 슬롯이 없어 grab을 버리는 기존 경로에서는 P 줄도 없다). 예외: OOS + 시뮬 모드에서만 슬롯 루프 뒤(빈 슬롯 없음)에도 `AppendPending` — OOS는 `m_bRunStatus`가 서지 않아 슬롯이 처음 2개 grab 뒤 영영 차 있으므로, 이 줄이 없으면 당김 시험이 글라스 첫 2 grab만 본다(디버그 전용, 실기 경로 무영향).
- `QualityWaitThread`(리뷰 K6 — 라이브 동작은 한 글자도 안 바꾸고 저널 컨텍스트만 고정):
  - `m_bRunStatus`를 본 순간, `CmFileFindTimeOut` **전에** `CString strInner0 = slot.m_strInnerID; int nRow0 = slot.m_nSheetRow; S_ResumeCtx ctx0 = slot.m_sJournal;` 지역 사본.
  - 파일 찾음: 토스는 기존 식 그대로(`TossResultFile(sQualityWait[i].m_strInnerID, sQualityWait[i].m_nSheetRow)`). 그 앞뒤로 `SetThreadCtx(slot.m_strInnerID == strInner0 ? &ctx0 : &s_ctxDropped)` / `SetThreadCtx(NULL)` — 대기 중 Recipe Change `EXE_QUALITY_RESET`(`DoWorkGlassRecipeStep.cpp:862`)이 슬롯을 지우고 다음 grab이 슬롯을 재사용했으면 그 토스는 저널에 남기지 않는다(`s_ctxDropped.nGen = 0`).
  - 타임아웃: `if (!CmFileFindTimeOut(...)) {...}`에 `else { CResumeJournal::GetInstance()->AppendClose(ctx0, m_iDeviceIndex, nRow0, strInner0); }` 추가(X 줄, `ctx0.nGen == 0`이면 아무것도 안 함). 그 뒤 기존 `sQualityWait[i].Clear();` 그대로.
  - 앵커: `if (!pQuality->m_pCommTcpIpFactoryMgr->CmFileFindTimeOut(strPath, 150000/*pQuality->m_nQualityDelay*/)) {` n=1, `\t\t\t\tpQuality->sQualityWait[i].Clear();\r\n` n=1(그 앞에 `else` 블록), 지역 사본은 `strPath.Format(...)` 줄 앞(같은 `if (pQuality->sQualityWait[i].m_bRunStatus) {` 블록 첫 줄).
- 파일 I/O·핸들(리뷰 C6/K14):
  - 쓰기 핸들: `Arm` 때 `CreateFile(FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_DELETE, OPEN_ALWAYS)`로 열고 무장 동안 유지, 줄마다 `WriteFile` 1회(로컬 NTFS 원자 append). `Disarm()`이 저널 잠금 안에서 닫는다.
  - 무장 해제 뒤 세대가 같은 늦은 줄(VISION 오버라이드, X 줄): 잠금 안에서 **열기 → 쓰기 → 닫기**(선택: 버리지 않고 기록 — 정지 뒤 도착한 VISION이 P를 닫아 불필요한 당김을 막는다).
  - 읽기(`Analyze`, 압축 tmp 작성, 링 헤더): `FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE`. 열기 실패가 `ERROR_FILE_NOT_FOUND`/`ERROR_PATH_NOT_FOUND`면 "없음", 그 밖(공유 위반 등)은 "읽기 실패" — 팝업은 지우지 않고 취소((2-2)).
  - `DeleteFiles`·`SwapCompacted`·`Begin`: 저널 잠금을 잡고 쓰기 핸들을 먼저 닫은 뒤 `DeleteFile`/`MoveFileEx`/잘라 쓰기(열린 핸들 위의 삭제 보류로 같은 이름 재생성이 실패하는 것 방지).
  - `AppendPending`·`AppendClose`도 `nGen == 0` 또는 `nGen != m_nGlassGen`이면 버린다(`OnUpload`와 같은 규칙).
  - **쓰기 실패**(리뷰 K7): `WriteFile`/열기 실패 시 끈적 플래그 `m_bWriteOK = false` + WARN 1회 `ResumeJournal: write failed (%u) - this glass will not be resumable`. `Commit()` = `FlushFileBuffers` 후 `m_bWriteOK && 성공`을 반환, `GetLineCount()` = 이 글라스(이 시도 포함)에서 성공적으로 쓴 줄 수(압축 뒤에는 남긴 줄 수부터). `TupleStickDone`이 둘을 `UpdateTupleProgress`로 북마크에 적는다.
  - `Commit()`은 **스틱 완료 때 진행 저장 직전**(저널이 디스크에 있은 뒤에만 북마크 전진). 프로세스 크래시는 OS 버퍼로 보존, 전원 차단 대비가 Commit.
- 잠금 순서: `CResumeInfoMgr::m_cs` → 저널 잠금(1-3). 저널 잠금을 쥔 채 다른 잠금을 잡지 않는다.
- 비용: WAD 행마다 측정 스레드에서 수 ms I/O. 튜플 글라스만.

*4-4 재생 (D5-3 (b))*
- 위치: Measure `ResumeTupleEnter` 7단계(측정 스레드, 루프 전). Recipe None이 이미 `ClearData()`(VIS/MICRO 결함 벡터 포함)를 했다.
- 알고리즘:
```
SetFastUpload(TRUE) (RAII - 실패·정지 경로에서도 FALSE 복귀)
// (a) Recipe None 뒤 ~ 지금 사이에 늦게 들어온 업로드(ANGLE, 대기 중이던 VISION 슬롯) 정리 - 리뷰 K9
wait until GetUploadState().q == 0 (STOP/일시정지 폴링, 1000회 x 10 ms 무진행 -> FAIL)
CResultMgr::ClearData()  (튜플 경로에서만, 두 번째)
GetUploadState(&q, &done0); sent = 0; idle = 0
for line in keep (파일 순서 = 라이브 순서, 마지막 쓰기 우선 — 행 재계산 없음, Phase 4 (12)-1/-7):
    if (IfPauseEvt_WaitResumeEvt() == false) -> STOP     // 일시정지면 여기서 재개를 기다리고, 정지면 false (BaseSequenceBody.cpp:46-64)
    P/X 줄: 건너뜀
    S 줄: 시뮬 모드면 가짜 경로로 업로드(dResult FILE_NG → 행 없음, 파이프라인·카운터 검증), 아니면 건너뜀
    R 줄: raw 파일(path+file) 없으면 누락 +1, 건너뜀(1초 대기·빈 행 방지); 있으면
          while (q >= 8) { if (!IfPauseEvt_WaitResumeEvt()) -> STOP; Sleep(10); GetUploadState; 진행 없으면 idle++, 있으면 idle = 0; idle >= 1000 -> FAIL }
          CResultMgr::Updata_Data(&rd)  (무장 전이라 다시 기록 안 됨); sent++
wait until done - done0 >= sent (같은 폴링·무진행 1000회 -> FAIL)
SetFastUpload(FALSE)
```
- **타이머는 벽시계가 아니라 폴링 횟수**(1000회 × 10 ms ≈ 10 s 무진행)다. 일시정지는 측정 스레드를 `SuspendThread`로 세우므로(`DoWorkGlassSequenceEngine.cpp:538`, `BaseSequenceImpl.cpp:144-149`) 벽시계 기준이면 재개 직후 "10 s 무진행"으로 오판해 재개 데이터를 버리게 된다(리뷰 K4). 일시정지 이벤트 대기 중인 시간도 횟수에 들어가지 않는다.
- STOP은 Init 스텝의 `SetStopUser(false)`(`DoWorkGlassMeasureStep.cpp:557`) 전에 잡아야 한다 — 재생은 루프 전이라, 폴링하지 않으면 재생 중 STOP이 첫 Init에서 지워지고 글라스가 끝까지 측정된다(리뷰 K4).
- 빠른 모드는 dResult `UploadData`의 1초 `m_bDisplayResulted` 대기만 건너뛴다(PostMessage는 남김). 정상 런은 스위치가 꺼져 있어 타이밍 동일.
- 예상 시간: 행당 `ReadResultData`(파일 읽기 + FPSpread 쓰기) 수십 ms → 210줄 수 초~20초, 2268줄 1~4분(지금 경로면 40~60분). 실측은 실기 항목.
- 로그: OPER `[RESUME] journal replay: %d rows (WAD %d, VISION %d, ANGLE %d, sim %d), dropped %d, torn %d, missing raw %d, %.1f s`; 누락이 있으면 SYST에 처음 5개 파일명. 시뮬 모드에서만 줄마다 SYST `[RESUME] replay %c %s row(%d) T%d seg %d stick %d`(검증용).
- 남는 경합(기록, (11)): 재생 시작 뒤에 도착한 이전 시도의 늦은 VISION은 라이브 경로로 그리드에 들어가지만(세대가 바뀌어) 저널에는 남지 않는다.

*4-5 압축·VISION 미도착 당김 (D5-12 (b))* — `CResumeJournal::Analyze`(팝업·측정 진입 공용, 읽기 전용) + `WriteCompacted`/`SwapCompacted`
- 재개 튜플 T = (rPass, rSeg, rStick)(ALL DONE이면 끝 표지 `(PassCount, 0, 0)`). 사전식 (pass, seg, stick) < T인 줄만 남긴다 → 중단 스틱과 그 뒤 줄 삭제, T1 재개 때 같은 스틱의 T0 줄은 (0, s, k) < (1, s, k)라 남는다(라이브 그리드와 같음, T1이 덮음).
- 짝 없는 P 줄 = 같은 InnerID의 X 줄도, raw 파일이 있는 R VISION 줄도 P 뒤에 없음. **당김 자격**(전부 만족, 리뷰 C4/K6):
  1. 그 P가 **자기 헤드의 마지막 `QUALITY_MAX`(2)개 P 줄** 안에 있음(파일 전체 기준, T 이후 줄 포함). 슬롯이 헤드당 2개이고 `CmQualityEmptyTimeOut(NOT_LAST_ITEM, ...)`(`DoWorkGlassMeasureStep.cpp:3844`)이 빈 슬롯이 날 때까지 grab을 막으므로, 정지 순간 "아직 오는 중"일 수 있는 grab은 헤드당 최근 2개뿐이다. 그보다 오래된 짝 없는 P = 결과를 영영 못 받은 grab(장치가 `INSP` 완료를 안 보내 슬롯이 묶인 경우 등) — 라이브 런에서도 그 행은 비므로 당기지 않는다.
  2. `Tp.pass >= PassBoundaryDone`(이미 플러시한 pass로는 당기지 않음 — T1이 같은 행을 덮는다).
  3. EFA 글라스면 `pred(Tp)` 스냅샷이 링에 있음(또는 Tp = (0,0,0)) — 없으면 당기지 않음(5-1).
- 자격 있는 P 중 가장 이른 튜플 Tp < T이면 T := Tp → OPER `[RESUME] VISION pending on T%d seg %d stick %d - resume moved back to that stick (was T%d seg %d stick %d)`. ALL DONE도 해제된다. Tp = (0,0,0)이면 새 글라스((3-2) 4).
- 자격 없는 짝 없는 P는 WARN 한 줄로 개수·첫 InnerID를 남기고(`[RESUME] VISION never arrived for %d grab(s) (first %s T%d seg %d stick %d) - not pulled back (%s), rows stay empty`, 사유 = `older than the last 2 grabs of head %d` / `pass already flushed` / `no cell status snapshot`) **압축 파일에서 버린다** — 다음 재개 때 "헤드의 마지막 2개"가 바뀌어 같은 P가 뒤늦게 당김을 일으키는 일을 막는다. 불변식: 저널 = 그리드를 이루는 줄 + 자격을 아직 판정받지 않은 P(= 마지막 압축 뒤의 P) + X.
- 압축 순서는 (3-2) 6: tmp 작성 → INI `ApplyCompaction`(당김이면 Last* = pred(T), `ResultSaved=0`, `JournalLines` = 남긴 줄 수, Attempt+1) → 저널 교체(`MoveFileEx`, 헤더 `Attempt`+1, seq는 최대값+1부터). INI가 먼저라 "교체 뒤 Tp 완료 전 정지"에서도 다음 재개가 Tp에서 시작한다(리뷰 C1/K1).
- 비용: VISION grab당 1줄. 당김은 헤드당 최근 2 grab 범위(보통 직전 1~2 스틱)로 제한되고, 타임아웃(X)·오래 묶인 슬롯·플러시된 pass는 당기지 않는다.
- 팝업은 같은 `Analyze`로 당김을 **예상**해 보여 준다((2-3)). 팝업 뒤 늦게 온 VISION이 P를 닫으면 측정 진입의 실제 T가 더 뒤일 수 있다.

*4-6 저널 없는 글라스*
- N=1 레거시(Format=1) 북마크: 기존 RS1(무변경, 늘 SKIP) — D5-1 (a).
- 하이브리드(튜플 대상) 레시피 + **같은 RecipeID**의 Format=1 북마크(Phase 4 빌드가 남긴 것 등): 거부 + Clear(D5-16 (a), Phase 4 (12)-5). RecipeID가 다르면 기존 불일치 팝업(리뷰 K12).
- Format=2인데 저널 없음·손상·쓰기 실패 기록(`JournalOK=0`)·줄 수 부족: 거부 + Clear((2-2) 5). 찢긴 끝 줄만이면 그 줄만 버리고 재개. 공유 위반 등 읽기 실패: 지우지 않고 START 취소.

---

**(5) 셀 상태·래치 복원**

*5-1 `nCellStatus[100][50]` 스냅샷* (`CoreFunction/ShareMemFunction/ShareMem.h:2064`, ALPHA 프로세스 전용 → 재시작 시 0)
- 형식 `ResumeCellStatus{0,1,2}.dat`(3칸 링, 리뷰 C5): magic `RCS1`, version 1, GlassID(`WCHAR[32]`), attempt, 쓰기 일련번호(글라스 안 단조 증가), pass/seg/stick(int×3), `int[100][50]`, FNV-1a 체크섬. 칸당 ≈ 20 KB. 칸 = 일련번호 % 3, tmp + `MoveFileEx`. 쓰기 실패는 WARN 1회(EFA 글라스는 그 튜플로 재개할 수 없게 될 뿐, 측정 계속).
- 시점: 튜플 글라스 스틱 완료마다(저널 Commit 뒤, 진행 저장 전 — 글라스 마지막 스틱은 Idle 전). 열은 글라스 절대 스틱이라 base 계산 없이 배열 전체를 그대로 저장·복원.
- **선택 규칙**(리뷰 C5/K2): 재개점 T(당김 반영)의 `pred(T)`와 **튜플이 정확히 같은** 칸 중 GlassID·체크섬이 맞고 일련번호가 가장 큰 것. 그 칸은 "스틱 pred(T)를 마친 직후"의 상태라, T 이후 열은 그 시도에서 아직 손대지 않은 값이다 — T0면 IDLE, T1이면 T0 최종값(라이브 T1 스틱이 보던 값). T0 복원 뒤에는 방어적으로 글라스 스틱 ≥ gs(T) 열을 IDLE로 다시 지운다(T1은 지우지 않는다).
  - 정확한 칸이 없을 때: **EFA 글라스(`EfaMode=1`)는 재개 불가**(팝업 거부 (2-2) 7 / 측정 진입 FAIL, 당김 후보면 당김 포기 (4-5)). 비-EFA 글라스는 일련번호가 가장 큰 "튜플 < T" 칸을 복원하고 열 ≥ gs(T)를 IDLE로 지운 뒤 WARN(표시용 손실만), 칸이 하나도 없으면 IDLE 그대로 + WARN.
  - 쓰기 순서 = 저널 Commit → 스냅샷 → INI(리뷰 C5의 "INI → 스냅샷" 제안은 채택하지 않음): 스냅샷과 INI 사이에서 크래시하면 INI는 아직 L−1, 재개점 T = L, `pred(T)` = L−1 칸이 링에 남아 있어 정확히 복원된다. 반대 순서면 INI L, 링에 L 칸이 없어 EFA 글라스가 거부된다. 앞선 스냅샷(L 칸)은 튜플이 달라 선택되지 않으므로 무해하다.
  - 링 3칸이면 `pred(T)`는 L, L−1, L−2까지 덮는다 — 당김은 헤드당 최근 2 grab 범위(4-5)라 보통 Tp ≥ L−1, `pred(Tp)` ≥ L−2. 더 깊으면(중간 스틱에 VISION이 없던 경우) EFA 글라스는 당기지 않고 WARN.
- 복원: Measure 진입(`ResumeTupleEnter` 8단계). Recipe Change의 `RecipeUpdateUI()` → PostMessage → UI 스레드 `GenGlassPath()`가 **비동기로** 전체를 IDLE로 지우므로, 그 뒤여야 한다.
  - 보장 장치: `GenGlassPath`의 리셋 루프 직후(`nBFCellStatus[iX][iY] = -1;\r\n\t\t}\r\n\t}\r\n` n=1 뒤) `CResumeJournal::GetInstance()->NoteCellStatusReset();`(InterlockedIncrement). Recipe None(튜플 재개 분기)이 `MarkCellStatusReset()`으로 그 값을 기억 → Measure가 값이 증가할 때까지 최대 5초 대기 후 복원. None은 Change보다 먼저 돌고 START 메시지의 `GenGlassPath`(동기 SendMessage)는 None 전에 끝나므로, 증가 = Change가 게시한 리셋이 처리됨.
  - 잔여 위험: None 전에 게시된 다른 리셋 메시지가 큐에 남아 있다가 None 뒤에 처리되면 조기 통과(UI가 수 초 막혀 있어야 가능) → (11).
- 필요성: **EFA 글라스의 모든 재개에서 기능상 필수**(리뷰 K2로 정정 — 이전 문구 "T0 재개는 표시용"은 틀림). T1은 Pattern Search/OSTest/R-Test를 건너뛰므로(`DoWorkGlassMeasure.cpp:306`) T0의 PTP/OST short·open, AGING_NG, R-Test OVER/UNDER 제외 판정(`DoWorkGlassMeasureStep.cpp:2571-2612`)이 스냅샷에서 와야 한다. T1 재개는 바로, T0 재개(스틱 j)는 나중의 T1에서 스틱 < j의 상태가 필요하다. 비-EFA 재개는 재측정하지 않는 스틱의 상태를 아무 측정도 읽지 않으므로 표시용(운전 화면 셀맵 색), 재측정 스틱은 열 ≥ gs(T) IDLE 지움으로 옛 NG가 남지 않는다(AgingPG는 AGING_NG를 OK로 되돌리지 않음 — `CommTcpIpObjectAgingPG.cpp:803-948`, 리뷰 C5).
- 스냅샷이 없거나 깨진 EFA 글라스 재개 → 팝업 거부((2-2) 7) / 측정 진입이면 파일을 그대로 두고 FAIL + WARN.

*5-2 래치·상태 — 다시 세움 vs 복원*

| 상태 | 처리 | 근거 |
|---|---|---|
| `m_nEfaGlassLatch`(EFA^1$) | 다시 세움(측정 시작 `AgingGlassLatch(bEfaMode ? nPassCount : 0)`) | pass 수·EFA 모드는 북마크와 비교((2-2) 9, (3-2) 1) |
| EFA 공유 플래그 `NewAgingMode` | **쓰지 않는다**. 북마크 `EfaFlag`·`EfaMode`는 입구 조건 비교에만(OPERATE·CIM = 현재 플래그와 같아야, AgingTest = 0, AgingNew = EFA 글라스) (D5-9 (b)) | 공유메모리라 재시작 시 소실, CIM PPID마다 리셋(`CommTcpIpObjectCIM.cpp:1419`), AgingNew OnStop/START(`:287`/`:531`), AgingTest START(`:478`)만 쓴다. 복원하면 이후 비재개 글라스까지 EFA로 돈다(리뷰 C8) |
| `m_nAgingGlassLatch`(AgingTest) | 다시 세움 | 레시피 기반, 지문으로 변경 차단 |
| AgingCycle 스틱 상태(item/repeat/skip) | 복원 안 함 | 스틱마다 `AgingCycleBegin`이 0부터, 재개 스틱은 처음부터 |
| `m_nResultSaveLatch`, APD 헤더 G×1 | 다시 세움(Recipe Change) | Phase 4 (12)-4 |
| `m_nVisionRcpWaitLatch` | 다시 세움(0) + 재개 첫 VISION 대기 플래그(D5-15) | Phase 4 (12)-10 |
| Cell ID 래치(D-15) | 다시 세움(복원된 GlassID로) | 4b (12)-1 |
| 세그먼트 캐시 5종 | `SegmentBoundary`/`SegmentSwitchData`로 다시 계산 | Phase 4 (12)-3 |
| `m_nAgingStartStick[]` | 다시 세움(PreMeas, 스텝) | |
| 셀 InnerID | 새로 발급(재측정 = 새 raw 파일) | 저널이 경로를 적으므로 무관 |
| GlassID/LotID | 북마크에서 복원(수동·다이얼로그) / 비교(CIM) (D5-8) | 수동 CYCLE은 START마다 새 ID |
| dResult 그리드 + VIS/MICRO 결함 벡터 | 저널 재생(순서대로 → 결함 벡터도 라이브와 같은 순서·개수) | 매퍼 result 1.3 |
| `nCellStatus` | 스냅샷 링에서 `pred(T)` 칸 복원(5-1) | |
| VISION 대기 슬롯 | 복원 안 함(Recipe Change가 지움) → P 줄로 감지·범위 제한 당김(4-5) | |
| 재개 첫 VISION 대기 플래그(D5-15) | 무장 세대에 묶임, `Disarm`이 지움(3-6) | 리뷰 C7 |
| APD `START_TIME` | 재개 시도 시각(기존) | 이월 |

*5-3 CIM 상태*
- T1 재개 + `PassBoundaryDone >= 1`: 경계를 건너뛰므로 T0 APD 재업로드·`EFA^0$` 재송신 없음. 글라스 끝 MEAS_DONE·`EFA^1$`는 정상(`EFA^1$`는 N>1이면 측정 시작에 다시 선 래치, N=1이면 MeasDone이 공유 플래그를 읽는 판정 — D5-9 (b)의 입구 조건으로 현재 플래그가 원래 글라스와 같으므로 같은 결과).
- 경계 도중 정지: `Cmd_Send_EFA0()` 뒤면 플래그 1 → 다시 보내지 않는다. 그 전(APD FTP 대기 중)이면 경계를 다시 돈다 — T0 파일·APD 1회 더, `EFA^0$`는 1회뿐(3-4). REMOTE 경로는 OOS에 호스트가 없어 코드 리뷰 항목.
- CIM 입구 GlassID 불일치 = 다른 글라스 → 재개 거부(ICP "새 글라스는 절대 재개하지 않는다"와 같은 방향).
- T0와 T1 사이 시간이 재개 대기만큼 늘어난다(공정 판단, (11)).

---

**(6) 접점 표** (ALPHA 경로 = `HMI/Source/ALPHA/` 기준, dResult = `HMI/CoreFunction/dResult/`. 앵커 n은 `p5plan/anchors.pl`·`anchors2.pl` 결과. "N=1 정상" = 1세그먼트 비재개 런, "N=1 재개" = 1세그먼트 재개 런. 기본은 D5-1 (a) 기준, (b) 활성 시 차이는 괄호)

| # | file | function / anchor (n) | 변경 | N=1 정상 | N=1 재개 |
|---|---|---|---|---|---|
| P1 | dResult `CDlgResult.h` (UTF-8 BOM) | `\tbool m_bDisplayResulted;` (1) 뒤 | `volatile bool m_bFastUpload;`, `volatile LONG m_nUploadDoneCount;` | 없음 | 없음 |
| P2 | dResult `CDlgResult.cpp` (UTF-8 BOM) | 생성자 `\tm_bDisplayResulted = false;\r\n` (1) 뒤 초기화 / 모니터 `UploadData(&m_RData); // ` (1) 뒤 `InterlockedIncrement(&m_nUploadDoneCount);` / `UploadData`의 `if (nRet != MEAS_RESULT_NONE)` 블록(1) 안 do-while을 `if (m_bFastUpload == false)`로 감쌈 | 빠른 재생 + 처리 카운터 | 없음(스위치 false = 1초 대기 그대로) | 없음 |
| P3 | dResult `ResultManager.h` (ASCII) / `.cpp` (CP949) | `\tvoid DataClear();\r\n` (1) 뒤 / EOF | `SetFastUpload(bool)`, `GetUploadState(int*, long*)` | 없음 | 없음 |
| P4 | dResult `API_Result.cpp` (ASCII) + `API_Result.h` 두 사본(CP949, dResult 사본은 post-build로 `Source/Include`에 복사) + `dResult.def` (UTF-8 BOM) | `void __stdcall API_Result_DataClear()` 뒤 / `bool WINAPI API_Result_SaveResultDone();` (1) 뒤 / `API_Result_SaveResultDone` (1) 뒤 | export 2개 `API_Result_SetFastUpload(bool)`, `API_Result_GetUploadState(int*, long*)`. 두 `.h` `cmp` 0 | 없음 | 없음 |
| P5 | 신규 `Manager/ResumeJournal.h/.cpp` (ASCII CRLF) | — | `CResumeJournal` 싱글턴(아래 6-1) | 없음(무장 안 됨) ((b): 저널·스냅샷 파일 기록, 무로그) | 없음 |
| P6 | `Manager/ResultMgr.cpp` (ASCII) / `.h` (CP949) | `::API_Result_Upload(result);` (1) 뒤 / `ClearData();` 선언 뒤 | `OnUpload` 훅, `SetFastUpload`/`GetUploadState` 래퍼 | 행마다 CS 1회 + 무장 검사(파일 없음) | 없음 |
| P7 | `ALPHA.vcxproj` (UTF-8, BOM 없음) / `.filters` (UTF-8 BOM) | `Manager\ResumeInfoMgr.h` (:581) / `.cpp` (:837) 근처 | 신규 4파일 등록(filters는 ResumeInfoMgr 블록의 `<Filter>` 값 raw 복제) | 없음 | 없음 |
| P8 | `Manager/ResumeInfoMgr.h` (CP949 **LF**) | `\tbool UpdateStickProgress(int nLastStick);\n` (1) 뒤 / `\tCString\t\tm_strResultPath;\n};` (1) | 튜플 API·멤버·CS(6-2) | 없음 | 없음 |
| P9 | `Manager/ResumeInfoMgr.cpp` (CP949 **LF**) | 생성자 / `bool CResumeInfoMgr::LoadResumeInfo()\n` / `SaveResumeInfo(` / `UpdateStickProgress` / `WriteToFile(CString strPath)\n` / `ClearResumeInfo()\n` (각 1) / `DeleteFile(m_strFilePath);` (3) + `if (!MoveFile(m_strTmpPath, m_strFilePath))` (2) 함수 범위 치환 | `SetFolder`, Format 2 키 읽기/쓰기(≥2만), Save/Clear에서 튜플 리셋 + `DeleteFiles`, `MoveFileEx`, 새 메서드 | INI 바이트·로그 동일 ((b): Format 2 키가 붙음, 로그 동일) | 없음 ((b): 튜플 경로) |
| P10 | 신규 `ResumePrompt.h` (ASCII) / `.cpp` (**CP949** CRLF, 한글 팝업) | — | `ResumeTuplePrompt`, `ResumeRefuseLegacyHybrid`, `ResumeDialogEntry`, `ResumePromptBusy()` + `CResumePromptBusy`(RAII 가드), 엔트리·결과 enum | 없음 | 없음 ((b): N=1 튜플 팝업) |
| P11 | `DlgSubOperate.cpp` (CP949 CRLF) | `#include "Manager/ResumeInfoMgr.h"` (1) 뒤 include / `CheckAndApplyResumeMode()\r\n{\r\n\tCResumeInfoMgr* pResumeInfo = ...;\r\n` (1) 뒤 분기 / CYCLE: `//\tm_pAuroraMgr->GetRecipeEditMgrPtr()->OpenRecipe(_T("KR1_DM0"));\r\n` (1) 앞에 `ResumePromptBusy()` 가드(알람 35 팝업, `IsCycleState()` 블록과 같은 모양 — `\t// jsyoon 2022.07.19 add\r\n\tif (CSystemParamShare::GetInstance()->IsCycleState() == true)\r\n`은 n=5라 쓰지 않는다), `CD_MeasureCycle dlg(...);\r\n\t\tif (dlg.DoModal() == IDOK)\r\n\t\t{\r\n` (1) 뒤로 검사 이동(`CResumePromptBusy` 블록) + `if (!CheckAndApplyResumeMode())\r\n\t\t\t\t\treturn;\r\n` (1) 삭제 / CIM: `\tcase TO_SERVER_MEAS_START:\r\n\t{\r\n` (1) 뒤로 이동(가드 블록) + `\t\t// LYS 2026-07-07 Resume check for CIM Online path\r\n\t\tif (!CheckAndApplyResumeMode())\r\n\t\t\tbreak;\r\n` (1) 삭제 / `GenGlassPath` 리셋 루프 뒤 (1) | 튜플·레거시 하이브리드 분기(RecipeID 먼저), D5-10 순서 이동, 팝업 중 두 번째 시작 차단, 리셋 카운터 | 없음(북마크 없으면 `SetResumeMode(false)`만, 순서 무관; 가드는 팝업 동안만 서므로 정상 런에서 CYCLE 가드는 늘 통과) | **취소 시 상태 고착 해소, 팝업이 START 메시지보다 먼저, 팝업 중 CYCLE·다이얼로그 START 거부**(D5-10 승인 필요) |
| P12 | `DlgSubManual_AgingTest.cpp` (**UTF-8 BOM** CRLF) | `#include "DoWork\AgingCycleInfo.h"\t\t// LYS 2026-08-18 ...` 뒤 include / `\t// shared state (OCP parity: ...)\r\n` (1) 앞 | `ResumeDialogEntry(..., AGINGTEST)` | 없음(북마크 없음 = 플래그 false만) | stale 재개 플래그 정리(D5-10) ((b): 튜플 북마크면 팝업, D5-11) |
| P13 | `DlgSubManual_AgingNew.cpp` (CP949 CRLF) | `#include "DoWork\AgingCycleInfo.h"\t\t// LYS 2026-09-30 ...` 뒤 / `\tif (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory() != NULL)\r\n\t{\r\n\t\t...SetSys_AutoAgingMode(false);\r\n` (1) 앞(한글 주석 줄은 앵커에서 제외) | `ResumeDialogEntry(..., AGINGNEW)` | 없음 | 같음 |
| P14 | `DoWork/DoWorkGlassMeasure.h` (CP949 CRLF 33) | `\tvoid SegmentRestore0();` (1) 줄 뒤 / `EfaPassBoundary(int nPass);` (1) / `SegmentBoundary(int nFrom, int nTo);` (1) | private `ResumeTupleEnter`, `ResumeTupleBegin`, `TupleStickDone`, `ResumeForceBoundary` 선언 + `RESUME_ENTER_*` enum; `bool EfaPassBoundary(int nPass, bool bTupleGlass = false);`, `bool SegmentBoundary(int nFrom, int nTo, bool bForce = false);` | 없음(기본 인자) | 없음 |
| P15 | `DoWork/DoWorkGlassMeasure.cpp` (CP949 CRLF 777) | (A) `if (CSystemParamShare::GetInstance()->IsResumeMode())` (1, :160) / (B) `m_nCurSeg = (pSegFunc != NULL) ? ...;` (1, :197) 뒤 / (C) `for (int nPass = 0; nPass < nPassCount; nPass++)` (1), `if (nPass >= 1 && EfaPassBoundary(nPass) == false)` (1), `for (int nSeg = 0; nSeg < nSegCount; nSeg++)` (1), `if (nSeg != m_nCurSeg && SegmentBoundary(m_nCurSeg, nSeg) == false)` (1), `for (size_t i = ((nPass == 0 && nSeg == 0) ? nStartStick : 0); ...` (1), `GoToInitStep();  // Init` (1) 앞, `if (GetCurStep() == GetExitStepIndex()) SegmentRestore0();` (1), `if (nSeg == 0) pResume->UpdateStickProgress((int)i);` (1) / (D) `if (bExis == true) break;\t// LYS 2026-09-28 EFA:` (1) 블록 뒤 / `EfaPassBoundary`: 정의 `bool CDoWorkGlassMeasure::EfaPassBoundary(int nPass)\r\n` (1), `\t\tm_pLogFileAccessor->KMLOG(OPER, _T("EFA: not ONLINE REMOTE - skip APD upload / EFA^0$"));\r\n\t\treturn true;\r\n` (1), `\tif (pCIM == NULL) return true;\r\n` (1), `\tpCIM->Cmd_Send_EFA0();\r\n` (1) / `SegmentBoundary`: 정의 `bool CDoWorkGlassMeasure::SegmentBoundary(int nFrom, int nTo)\r\n` (1), `bool  bTurn\t\t= (nRotFrom != nRotTo);` (1), 경계 로그 / EOF 헬퍼 본문 + `#include "..\Manager\ResumeJournal.h"` | (3-2)·(3-3)·(3-4)·(3-5) | 루프 시작값·조건·진행 저장 문장 동일, 경계 플래그·강제 경계·마지막 스틱 훅은 튜플만, `Disarm()` 2회 무동작, 새 로그 0 ((b): 무로그 파일 기록) | 레거시 경로 무변경 ((b): 튜플 재개 — P0-5 해소, 재생, 스냅샷) |
| P16 | `DoWork/DoWorkGlassRecipeStep.cpp` (CP949 CRLF) | `\t// LYS 2026-07-03 Resume: skip ResultClear ...\r\n\tif (CSystemParamShare::GetInstance()->IsResumeMode())\r\n` (1) 앞 | 튜플 재개 분기(`ClearData` + `MarkCellStatusReset`), 나머지 `else`로 기존 | 없음 | 없음 ((b): RS1 대신 ClearData) |
| P17 | `DoWork/DoWorkGlassMeasureStep.cpp` (CP949 CRLF) | `if (iStick == 0 && i == 0 && ...TakeVisionRcpWait())` (1, :3897) / `void CDoWorkGlassMeasureStep::LogHybridRow(...)\r\n{\r\n` (1, :538) 뒤 / include | 재개 첫 VISION 대기(D5-15), OOS 합성 훅(D5-17) | 없음(플래그 미무장, 시뮬 키 0) | 없음 ((b): j>0 재개 첫 VISION 대기) |
| P18 | `DoWork/DoWorkGlassResultStep.cpp` (CP949 CRLF 147, 공백 들여쓰기) | `    return bRet;\r\n}\r\n\r\nbool CDoWorkGlassResultStep_Exit::Run()` (1) 의 `return` 앞 / include | `CompleteCheck` 끝 `if (bRet && 튜플 북마크) MarkResultSaved(bUseMeas ? 1 : 2)`(리뷰 K3: 게이트 0도) | 없음(Format 1이면 무동작) | 없음 |
| P18b | `DoWork/DoWorkGlassUnloadStep.cpp` (CP949 CRLF 306) | `\t\tm_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassUnloadStep_CheckSaveReuslt End."));\r\n` (1) 뒤 / include | `CheckSaveReuslt` 성공 시 `if (튜플 북마크) MarkResultSaved(2)`(리뷰 C15: 기존 30초 `SaveResultDone` 대기 재사용, 타이밍 무변경) | 없음 | 없음 |
| P19 | `CommTcpIpObjectVision.h` (CP949 CRLF) | `\tCString m_strSegTag;` (1) 뒤 / `\t\tm_nSheetRow = -1;\r\n` (1) 뒤 / include | `S_ResumeCtx m_sJournal;` + Clear | 없음 | 없음 |
| P20 | `CommTcpIpObjectVision.cpp` (CP949 CRLF 1686) | `\t\t\tsQualityWait[i].m_bWaitStatus = TRUE;\r\n` (1) 앞 / `QualityWaitThread`: `if (pQuality->sQualityWait[i].m_bRunStatus) {` 블록 첫 줄(지역 사본), `if (!pQuality->m_pCommTcpIpFactoryMgr->CmFileFindTimeOut(strPath, 150000/*pQuality->m_nQualityDelay*/)) {` (1), `pQuality->TossResultFile(pQuality->sQualityWait[i].m_strInnerID, ...m_nSheetRow);` (1) 감쌈, `\t\t\t\tpQuality->sQualityWait[i].Clear();\r\n` (1) 앞에 `else` 블록 / `#include "Manager/ResultMgr.h"\r\n` (1) 뒤 | grab 컨텍스트·P 줄(헤드), 대기 전 사본·슬롯 재사용 감지, 타임아웃 X 줄, 토스 스레드 로컬 | 없음(무장 안 됨 → 모든 저널 호출이 무동작, 토스·Clear 순서 그대로) | 없음 |
| P21 | (선택) `Source/Bin/Config/SystemConfig.ini`, `Config_/SystemConfig.ini` `[OPT_PARA]` | — | `RESUME_TUPLE_ALL`(D5-1 (b) 활성, 5-6), `RESUME_JOURNAL_OOS_SIM`(OOS PC만) — 키 없음 = 0 | 키 없으면 없음 | 키 없으면 없음 |

- 무변경 확인: `DoWorkGlassSequenceEngine.cpp`(엔진 시작 저장 호출·종료 Clear 그대로 — 동작 변화는 `ResumeInfoMgr` 안), `DoWorkGlassUnload.cpp`(스텝 순서 None → Move_Safety → CheckSaveReuslt → SendToCIM → Idle, `:34-38`), `SystemParamShare.h/.cpp`, `RecipeEditMgr.h/.cpp`, `MeasureSequenceItemEngine.*`, `CommTcpIpObjectWAD/Angle/Color.cpp`, `DlgSubManual_Meas.cpp`, `ShareMem.h`(VTM_SYSTEM), `RSRITEMDATA`(`API_Result.h` 구조체), `D_MeasureCycle.cpp`, `DoWorkSequenceCIMStep.cpp`, `ALPHA.rc`/`resource.h`, dfcRecipe 전부, RS1 코드.
- 사후 `post` 개수(정규식): `OnUpload\s*\(` ResultMgr.cpp 1 / ResumeJournal.cpp 1(정의); `IsTupleBookmark\s*\(` ResumeInfoMgr.h 1, DlgSubOperate.cpp 1, Measure.cpp 1, RecipeStep 1, ResultStep 1, UnloadStep 1, ResumePrompt.cpp ≥1; `UpdateStickProgress\s*\(` Measure.cpp 1(기존 문장 유지); `UpdateTupleProgress\s*\(` Measure.cpp 1(`TupleStickDone` 안); `TupleStickDone\s*\(` Measure.cpp 3(정의 + 마지막 스틱 훅 + 진행 블록); `SetPassBoundaryDone\s*\(` Measure.cpp 3(비-REMOTE, `pCIM == NULL`, `Cmd_Send_EFA0` 뒤); `EfaPassBoundary\s*\(` Measure.cpp 2(정의 + 호출, 호출은 `(nPass, bTupleGlass)`); `SegmentBoundary\s*\(` Measure.cpp 2(정의 + 호출 1, 인자 3개); `ApplyCompaction\s*\(` Measure.cpp 1; `MarkResultSaved\s*\(` ResultStep 1, UnloadStep 1; `Disarm\s*\(` Measure.cpp 2; `AppendClose\s*\(` Vision.cpp 1; `AppendPending\s*\(` Vision.cpp 1; `SetThreadCtx\s*\(` Vision.cpp 2; `CheckAndApplyResumeMode\s*\(` DlgSubOperate.cpp 3(정의 + 이동된 호출 2); `CResumePromptBusy` DlgSubOperate.cpp 2; `ResumePromptBusy\s*\(\)` DlgSubOperate.cpp 1(CYCLE 가드); `ResumeDialogEntry\s*\(` AgingTest 1, AgingNew 1; `NoteCellStatusReset\s*\(` DlgSubOperate.cpp 1; `ConsumeResumeVisionWait\s*\(` MeasureStep 1; `SetSys_NewAgingMode\s*\(` **신규 파일 0**(D5-9 (b)); `m_bFastUpload` CDlgResult.cpp 2(초기화 + 조건) / `.h` 1; `API_Result_SetFastUpload` def 1, API_Result.cpp 1, `.h` 각 1; `Format` 키 `WritePrivateProfileString` ResumeInfoMgr.cpp 1.
- 새 앵커 개수(2026-10-06 `p5fix/anc_fix.pl`, raw 바이트): `\tpCIM->Cmd_Send_EFA0();\r\n` 1, 비-REMOTE 로그 + `return true;` 1, `\tif (pCIM == NULL) return true;\r\n` 1, `if (GetCurStep() == GetExitStepIndex()) SegmentRestore0();` 1, `bool CDoWorkGlassMeasure::EfaPassBoundary(int nPass)\r\n` 1, `bool CDoWorkGlassMeasure::SegmentBoundary(int nFrom, int nTo)\r\n` 1, `bool  bTurn\t\t= (nRotFrom != nRotTo);` 1, `if (nSeg != m_nCurSeg && SegmentBoundary(m_nCurSeg, nSeg) == false)` 1, `.h`의 `EfaPassBoundary(int nPass);` 1·`SegmentBoundary(int nFrom, int nTo);` 1, ResultStep `    return bRet;\r\n}\r\n\r\nbool CDoWorkGlassResultStep_Exit::Run()` 1, UnloadStep `CheckSaveReuslt End.` 로그 줄 1, Vision `CmFileFindTimeOut(strPath, 150000...` 1·`\t\t\t\tpQuality->sQualityWait[i].Clear();\r\n` 1, DlgSubOperate `//\tm_pAuroraMgr->GetRecipeEditMgrPtr()->OpenRecipe(_T("KR1_DM0"));\r\n` 1·CIM 검사 3줄 1. 주의: `\tSegmentRestore0();\t// LYS 2026-09-29 Hybrid Phase 3 ` 4, `IsCycleState() == true` 블록 7(jsyoon 주석 포함 5) — 앵커로 쓰지 않는다.

*6-1 `CResumeJournal` (P5) 공개 API*
```cpp
#pragma once
#include <vector>
#include "..\Include\API_Result.h"		// RSRITEMDATA (same include as ResultMgr.h)
class CRecipeEditMgr;

struct S_ResumeCtx { int nGen; int nAttempt; int nPass; int nSeg; int nStick; int nGlassStick; };	// POD, nGen 0 = not armed / dropped
struct S_ResumeTuple { int nPass; int nSeg; int nStick; };
struct S_ResumeShape { int nSegCount; CString strSegShape; int nGlassRows; CString strFingerprint; };
struct S_ResumeBookmark { CString strGlassID, strRecipeID, strSegShape; int nSegCount, nPassCount, nEfaMode, nGlassRows, nPassBoundaryDone, nJournalOK, nJournalLines, nAttempt;
						  S_ResumeTuple last; int nPoint; /* GetResumePoint: 0, 1 all done, -1 */ S_ResumeTuple T; };
struct S_ResumePlan { S_ResumeTuple T; bool bAllDone; bool bRewind; int nKeep; int nLines; int nTorn; int nMissing; int nPendingNoPull;
					  bool bSnapExact; int nWhy; /* 0 ok, else RESUME_WHY_* (missing, unreadable, header, write failed, short, snapshot) */ CString strWhyEn; };
struct S_ResumeReplayStat { int nRows, nWad, nVision, nAngle, nSim, nDropped, nTorn, nMissing; DWORD dwMs; };

class CResumeJournal
{
public:
	static CResumeJournal* GetInstance();
	void SetFolder(const CString& strConfigPath);			// CResumeInfoMgr ctor; reads [OPT_PARA] RESUME_TUPLE_ALL / RESUME_JOURNAL_OOS_SIM of SystemConfig.ini (log only when non-zero)
	bool IsTupleGlass(int nSegCount) const;					// nSegCount > 1 || RESUME_TUPLE_ALL
	int  GetSimMode() const;								// 0 off, 1 sim rows, 2 sim rows without VISION, 3 sim rows with VISION closed alternately by S / X, 4 sim + fail the 5th journal write
	static bool BuildShape(CRecipeEditMgr* pRcp, const CString& strWadMpFile, S_ResumeShape& out);	// 1 segment: active recipe; N>1: segment getters; FNV-1a of 4 files
	static bool PredTuple(const S_ResumeTuple& t, int nSegCount, const int* pnSticks, int nPassCount, S_ResumeTuple& out);	// false = (0,0,0) has none
	// lifecycle (journal lock; closes the writer handle first)
	bool Begin(const CString& strGlassID, const CString& strRecipeID, int nSegCount, int nPassCount, int nGlassRows);	// truncate + header + delete ring, nGen++
	void DeleteFiles();										// journal + ring + tmp, nGen++ (log only if something existed)
	// context (measure thread)
	void Arm(int nAttempt);									// opens the writer handle (FILE_SHARE_READ | FILE_SHARE_DELETE)
	void SetStick(int nPass, int nSeg, int nStick, int nGlassStick);
	void Disarm();											// closes the handle under the lock, clears the D5-15 flag; no-op when not armed
	S_ResumeCtx Capture() const;							// nGen 0 when not armed
	static void SetThreadCtx(const S_ResumeCtx* pCtx);		// thread_local override (QualityWaitThread); an override with nGen 0 drops the row
	// append (drop when nGen == 0 or nGen != m_nGlassGen; after Disarm with a current nGen: open-append-close)
	void OnUpload(const RSRITEMDATA* p);					// CResultMgr::Updata_Data hook; lock-free return when not armed and no override
	void AppendPending(const S_ResumeCtx& c, int nHead, int nArea, const CString& strInnerID);
	void AppendClose(const S_ResumeCtx& c, int nHead, int nArea, const CString& strInnerID);	// X line (VISION file timeout)
	void AppendSim(const TCHAR* szKind, const int* pnArea, const bool* pbUse, const TCHAR* szCell0, const TCHAR* szCell1);
	bool Commit();											// FlushFileBuffers; false once any write of this glass failed (sticky)
	int  GetLineCount() const;								// lines written OK since Begin / since the last compaction
	// resume (Analyze: read-only, popup and measure entry; never holds the lock while calling other managers)
	bool Analyze(const S_ResumeBookmark& bm, S_ResumePlan& plan) const;	// header, JournalOK, line count, torn line, InnerID pairing, pull-back (4-5), ring lookup
	bool WriteCompacted(const S_ResumeBookmark& bm, const S_ResumePlan& plan, std::vector<CStringA>& vecKeep);	// tmp file
	bool SwapCompacted();									// MoveFileEx tmp -> journal, nGen++ (after the INI ApplyCompaction)
	bool Replay(const std::vector<CStringA>& vecKeep, S_ResumeReplayStat& st, int& nResult);	// nResult: 0 ok, 1 stopped by user, 2 failed (4-4)
	// cell status ring (3 slots)
	bool SaveCellStatus(int (*pnStatus)[50], int nPass, int nSeg, int nStick);
	int  LoadCellStatus(int (*pnStatus)[50], const S_ResumeTuple& pred, bool bEfa, int nGlassStickT, int nPassT, S_ResumeTuple& got) const;	// 1 exact, 0 older (non-EFA), -1 none
	void NoteCellStatusReset();  void MarkCellStatusReset();  int WaitCellStatusReset(DWORD dwMs, CBaseSequenceBody* pStopSrc);	// 1 seen, 0 timeout, -1 stopped
	// VISION recipe wait on resume (bound to the armed generation)
	void ArmResumeVisionWait();  bool ConsumeResumeVisionWait();
};
```
- `pch.h` 첫 include(ALPHA PCH). 잠금은 `CRITICAL_SECTION`(afxmt 불필요). 로그는 `CLogWrapper`(SYST/WARN), 요약 로그는 Measure가 자기 로거로. `m_nArmedGen`·`m_nGlassGen`은 `volatile LONG`(Interlocked 읽기) — `OnUpload`의 무장 안 됨 빠른 반환용.
- `SystemConfig.ini` 키를 `SystemConfigMgr` 대신 이 모듈이 직접 읽는다(`{Config}\SystemConfig.ini` — `ResumeInfo.ini`와 같은 폴더). CP949 `SystemConfigMgr.*`를 건드리지 않기 위한 선택, 문서화.
- `WaitCellStatusReset`의 정지 원천: Measure 아이템 자신(`IfPauseEvt_WaitResumeEvt()`), 헤더 의존을 피하려면 `std::function<bool()>` 콜백으로 받아도 된다(구현 선택).
- `ResumePrompt.h`(P10): `int ResumeTuplePrompt(CAuroraMgr*, int nEntry)`, `bool ResumeRefuseLegacyHybrid(CAuroraMgr*)`, `int ResumeDialogEntry(CAuroraMgr*, int nEntry)`, `bool ResumePromptBusy()`, `class CResumePromptBusy { CResumePromptBusy(); ~CResumePromptBusy(); }`(static `volatile LONG` InterlockedIncrement/Decrement), `RESUME_ENTRY_OPERATE/CIM/AGINGTEST/AGINGNEW`, `RESUME_PROMPT_RESUME/FRESH/CANCEL`.

*6-2 `CResumeInfoMgr` 추가 (P8/P9)*
```cpp
	// LYS 2026-10-06 Hybrid Phase 5 tuple bookmark (Format=2): hybrid / journal glass
	bool IsTupleBookmark() const { return m_nFormat >= 2; }
	bool BeginTupleGlass(CString strGlassID, CString strRecipeID, CString strLotID, CString strInnerID, CString strResultPath,
						 int nTotalStick0, int nGlsRotate0, const S_ResumeShape& shape, int nPassCount, int nEfaMode, int nEfaFlag, bool bLog);
	bool UpdateTupleProgress(int nPass, int nSeg, int nStick, bool bJournalOK, int nJournalLines);	// JournalOK is sticky (once 0 stays 0)
	bool SetPassBoundaryDone(int nPass);
	bool MarkResultSaved(int nLevel);								// 1 Result item done, 2 result handled (never lowers)
	bool ApplyCompaction(bool bRewind, const S_ResumeTuple& last, int nKeptLines);	// Attempt+1, JournalLines; bRewind: Last* = last, ResultSaved = 0 (written BEFORE the journal swap)
	int  GetResumePoint(int& rPass, int& rSeg, int& rStick) const;	// 0 point, 1 all done (outputs PassCount,0,0), -1 invalid
	bool ValidateTupleShape(const CString& strRecipeID, const S_ResumeShape& cur, CString& strWhyEn) const;
	void GetBookmark(S_ResumeBookmark& out) const;					// one consistent copy under m_cs for Analyze / popup
	int  GetLastPass() const; int GetLastSegment() const; int GetSegmentCount() const; int GetPassCount() const; int GetEfaMode() const;
	int  GetEfaFlag() const; int GetPassBoundaryDone() const; int GetResultSaved() const; int GetAttempt() const;
	int  GetJournalOK() const; int GetJournalLines() const;
	CString GetSegShape() const; int GetGlassRows() const; CString GetFingerprint() const;
private:
	void ResetTupleMembers();										// shared by the missing-file early return of LoadResumeInfo, ClearResumeInfo, SaveResumeInfo (review C14)
	CRITICAL_SECTION m_cs;
```
- `BumpAttempt()`는 없앴다 — Attempt는 `ApplyCompaction`이 압축과 같은 INI 쓰기에서 올린다.
- LF-only 파일: CRLF→LF 정규화 상태로 `\n` 패치, LF로 저장(CR 0 유지 확인). 한글 주석은 넣지 않는다(ASCII).

---

**(7) 결정** (권장안 **굵게**. 확정 사항 — D-6, §5.8 7항목의 취지, D-13, D-14, D-15 — 은 다시 열지 않는다. ★ = HARD RULE상 N=1 재개 동작을 바꾸므로 명시 승인 필요, § = 확정된 §5.8 문구를 바꾸므로 명시 승인 필요, 공정 = 공정 판단이 필요해 명시 승인 필요)

| # | 항목 | 선택지 | 권장 |
|---|---|---|---|
| D5-1 ★ | N=1 재개 경로 | (a) N=1은 지금 그대로(레거시 Format=1, RS1 — 실기에서도 경로 불일치로 늘 SKIP, 재시작 뒤 재개 글라스는 재개 이전 스틱 행이 빈 채 저장, P0-5·D4·D10·D11·D12 남음) / **(b) 모든 글라스를 튜플·저널 경로로(N=1 포함). 코드는 술어 하나(`IsTupleGlass`)로 갈리고, `RESUME_TUPLE_ALL=1`로 켠다 — N>1 OOS 통과(5-5) 뒤 5-6에서 활성** / (c) RS1 경로만 `{DD}`로 고침 | **(b)**, 단 승인 전까지와 5-5까지는 (a)로 동작. 근거: D-6은 AgingTest·EFA 재개를 요구하는데 현장 AgingTest/EFA는 대부분 N=1이고 RS1은 한 번도 동작한 적이 없다. (b)는 N=1 재개를 고친다(행 복원, T1 이어 재기, GlassID 복원, 끝까지 측정된 글라스 저장). N=1 정상 런 로그는 바이트 동일하게 설계했다(D5-2). (c)는 2헤드 접두·반복·EFA·마스크·NG·스틱 0 셀 수 가정 때문에 행을 틀리게 채운다(빈 행보다 나쁨) — 비권장 |
| D5-2 ★ | N=1 정상 런이 저널 파일을 쓰는가 | (a) 쓰지 않음(= D5-1 (a)) / **(b) D5-1 (b)와 함께: 저널(행마다 1줄)·스틱마다 스냅샷 20 KB·INI Format 2 키를 `Config`에 기록, 정상 완료 시 삭제. 새 로그 줄 0(N=1 튜플은 `Resume Begin` 무로그, `Resume Update`는 기존 형식)** | **(b)** (D5-1 (b) 승인 시). 장치 SEND·결과·로그는 동일하고 디스크 부수효과만 생긴다 — HARD RULE의 "동작"을 문자 그대로 읽으면 변화이므로 승인 대상. 미승인이면 (a) |
| D5-3 | 재생 메커니즘 | (a) 기존 링 그대로, 행당 1~1.6 s 페이싱(DLL 무변경; 210줄 4~6분, 2268줄 40~60분) / **(b) dResult 추가 export 2개: 재생 중에만 1초 표시 대기 생략 + 큐 깊이·처리 수로 페이싱(드롭 없음)** / (c) 1초 대기 자체 제거 | **(b).** 정상 런은 스위치가 꺼져 타이밍 동일. (c)는 N=1 라이브 타이밍(`UploadDataDone` 시점)을 바꾼다. (a)는 AgingTest에서 비현실적 |
| D5-4 | 저널 위치 | **(a) `Config\ResumeJournal.csv`(북마크와 같은 폴더, 단일 슬롯, 같이 삭제)** / (b) 결과 트리 + 북마크에 경로 | **(a).** raw 폴더가 일(DD)만이라 날짜가 갈리고, AgingTest는 GlassID가 비어 있을 수 있다. §5.8-4의 "글라스 결과 폴더" 문구는 이 행으로 정정 |
| D5-5 | 기록 지점 | **(a) `CResultMgr::Updata_Data` 깔때기 1곳 + 측정 스레드 무장 컨텍스트 + VISION grab 시점 컨텍스트(슬롯 + 스레드 로컬)** / (b) WAD/ANGLE/VISION 호출부 3곳 | **(a).** §5.8-4 그대로이고 새 호출자도 자동 포함. 수동 측정·RS1·재생은 무장이 아니라 자동 제외, 늦은 VISION은 세대 번호로 글라스 경계 보호 |
| D5-6 § | 재개 첫 세그먼트(k ≥ 1)의 강제 경계 (**§5.8-3 문구 변경 — 사용자 승인 필요**, 리뷰 C11/K5) | (a) 늘 강제: Safety → R → 세그먼트 k `.alg` 재Align(§5.8-3 문구, ICP) / **(b) 필요할 때만 강제: `rot(k) == rot(0)`이고 1..k 안에 회전 경계가 있을 때(예: 회전 1→2→1의 세그먼트 3). 나머지는 기존 `SegmentBoundary(0, k)`가 자연히 수행(방향이 다르면 회전·재Align)** / (c) 강제 안 함(이전 권장) | **(b).** §5.8-3의 전제("Stage 상태를 알 수 없음")는 HMI에서 성립하지 않지만(엔진이 흡착 ResetAlign → 세그먼트 0 SW Align → PreMeas를 매번 돌림), 정상 런은 "마지막 회전 경계의 `.alg`로 잰 Align"을 쓰므로 (c)는 N ≥ 3 회전 패턴에서 중단 없는 런과 다르다(이전 플랜의 "같은 결과" 주장은 N=2에서만 참). (b)는 그 차이만 메운다. 남는 차이: 같은 방향 구간 안에서 세그먼트마다 `.alg` 이름이 다르면 Align 피두셜이 다름 — (12). OOS ①은 Half·LS 픽스처가 방향이 달라 (a)(b)(c) 모두 재Align. 3세그먼트는 오프라인 표 + 코드 리뷰(픽스처 있으면 O26) |
| D5-7 | T1 재개 시 EFA 경계 | **(a) `PassBoundaryDone`으로 생략(T0 파일·APD FTP·`EFA^0$` 재송신 없음). 플래그는 `EfaPassBoundary` 안에서 부수효과 직후(비-REMOTE = 플러시 뒤, REMOTE = `Cmd_Send_EFA0()` 직후) 세운다 — 플러시 전 정지면 재실행** / (b) 항상 재실행 | **(a).** §6 OOS ③ "T0 재수행 없이 T1 이어짐". 반환값 기준(이전 안)은 끝의 일시정지→정지 검사가 false를 내면 이미 보낸 `EFA^0$`를 다시 보낸다(리뷰 C3/K8) |
| D5-8 ★ | 재개 시 GlassID | **(a) 수동·다이얼로그 입구: YES에서 북마크 GlassID(+LotID) 복원, 팝업에 두 ID 표시. CIM 입구: 다르면 거부** / (b) 모든 입구 비교·거부(수동 재개는 같은 ID를 다시 입력해야 함) / (c) 지금처럼 비교·복원 없음 | **(a).** 수동 CYCLE은 START마다 새 ID(`D_MeasureCycle.cpp:201-214`)라 (b)는 수동 재개를 사실상 막는다. 복원해야 raw 폴더·Cell ID 접두·D-15 래치·dResult GlassID·저널 헤더가 맞는다. 튜플 글라스에만 적용(N=1은 D5-1 (b)일 때) |
| D5-9 ★ | 재시작 뒤 EFA 플래그 | (a) 북마크 `EfaFlag`를 OPERATE 입구 YES에서 공유 플래그에 복원(이전 권장) — 복원값이 이후 비재개 CYCLE 글라스까지 EFA로 돌린다(리뷰 C8, 플래그를 지우는 곳은 CIM PPID·AgingNew OnStop/START·AgingTest START뿐). 고치려면 "글라스 끝 비교 후 원복"이 필요하고 N=1 MeasDone(`EFA^1$`)이 Unload에서 플래그를 읽어 시점이 복잡 / **(b) 플래그를 쓰지 않는다. 북마크에 `PassCount`·`EfaMode`·`EfaFlag` 저장, 입구 조건으로만 맞춘다: OPERATE·CIM = 현재 플래그가 북마크와 같아야, AgingTest = `EfaFlag == 0`, AgingNew = `EfaMode == 1`. 다르면 되돌릴 수 있는 거부(취소 = 북마크 유지 → "EFA 화면의 START로 이어서 검사" 안내). 측정 진입에서 pass 수·EFA 모드 불일치면 FAIL(파일 유지)** / (c) 지금처럼(재시작 뒤 조용히 비-EFA) | **(b).** AgingNew START로 시작한 EFA 글라스는 재시작 뒤 AgingNew START로 이어 간다(다이얼로그가 지금처럼 플래그를 세운다). 숨은 공유 상태 변경이 없어 다음 글라스에 새지 않는다. 비용 = 재시작 뒤 CYCLE로 재개하려던 운전자는 안내 팝업을 한 번 본다. 튜플 글라스에만(N=1은 D5-1 (b)일 때) |
| D5-10 ★ | 레거시 재개 경로 결함(팝업 취소 시 사이클 상태·버튼 고착 → 알람 35, stale 재개 플래그) | **(a) 고친다: CYCLE·CIM 경로에서 검사를 사이클 상태 변경 전으로 옮기고, 4개 START 경로가 모두 재개 플래그를 세우거나 지운다. 팝업이 떠 있는 동안 `ResumePromptBusy()` 가드로 CYCLE 버튼·다이얼로그 START를 거부(CIM 팝업은 소유 창 없는 워커 모달이라 CYCLE이 눌릴 수 있음, 리뷰 C9/K10)** / (a') CIM 경로는 옮기지 않고 취소 때 사이클 상태·버튼을 되돌림(되돌릴 UI 갱신이 `WM_MSG_UPDATE_CYCLE_INSPECTION_COMPLETE` 팝업·PLC 비트와 얽혀 있어 비권장) / (b) 튜플 북마크만 고침 / (c) 그대로 | **(a).** 북마크가 없으면 검사는 `SetResumeMode(false)`만 하므로 N=1 정상 런 무영향(가드는 팝업 동안만 선다). 바뀌는 것은 "재개 팝업이 START 화면 갱신보다 먼저 뜬다", "취소가 진짜 취소", "팝업 중 다른 START 거부" 세 가지. 수동 팝업 중 CIM 시작이 겹치는 틈은 지금도 있고 무변경((11)) |
| D5-11 ★ | AgingTest / AgingNew START에서 재개 | **(a) 튜플 북마크면 같은 팝업으로 제안(모드 충돌 검사), 레거시 북마크면 stale 플래그만 지우고 팝업 없음** / (b) 제안 없이 덮어쓰기 전에 경고만 / (c) 그대로(북마크를 조용히 덮음) | **(a).** 오프라인 EFA는 AgingNew START로만 플래그가 켜진다 — 다이얼로그가 재개를 안 물으면 EFA 재개가 사실상 안 된다 |
| D5-12 | 정지 때 아직 안 온 VISION 결과 | (a) 손실 수용(ICP) + 개수 WARN / **(b) grab 때 P 줄(헤드 포함), 결과·타임아웃(X) 줄과 InnerID로 짝, 재개 때 짝 없는 P 중 자격(헤드의 최근 2 grab, 플러시 안 된 pass, EFA는 `pred(Tp)` 스냅샷 있음)이 있는 것만 그 스틱으로 재개점을 당김. INI를 먼저 되감고(Last* = pred(Tp), `ResultSaved=0`) 저널을 교체. 자격 없는 P는 WARN 후 버림. 팝업이 같은 계산으로 당김을 예고** / (c) 스틱 끝마다 VISION 도착 대기(정상 런 타이밍 변경) | **(b).** 완료로 기록된 스틱의 VISION 행이 비는 결함을 재측정으로 메우되, 라이브 런에서도 비었을 행(150 s 타임아웃, 결과를 영영 못 받은 슬롯)은 당기지 않아 "몇 스틱" 비용이 헤드당 최근 2 grab(보통 직전 1~2 스틱)으로 묶인다(리뷰 C4/K6). 되감기를 INI에 먼저 적어 두 번째 정지에도 데이터를 잃지 않는다(리뷰 C1/K1) |
| D5-13 | 모든 스틱 측정 후 Result/Unload 전 정지 | **(a) "저장부터" 재개: 재생 → 측정 아이템 즉시 완료 → Result/Unload. `ResultSaved` 2단(1 = Result 완료·내보내기 시작, 2 = Unload `SaveResultDone` 확인 또는 저장 게이트 0) — 2면 다음 START가 조용히 정리(다른 검사보다 먼저), 1이면 "저장 중 멈춤" 팝업으로 다시 저장할지 묻는다** / (b) 정확한 문구로 거부(새로) | **(a).** 지금은 틀린 "불일치" 문구로 다 잰 데이터를 버린다(D10, `Config_/ResumeInfo.ini` 실물). 0회 반복 무한 대기도 같이 막는다. 2단 표시는 사이클 2번째 글라스가 Measure 전에 멈출 때 직전 글라스(게이트 0 포함)를 조용히 정리하고(리뷰 K3), 내보내기 도중 크래시를 "저장됨"으로 오인하지 않는다(리뷰 C15). 튜플 글라스만 |
| D5-14 | 정지 ~ 재개 사이 파일 변경 | **(a) 레시피 `.rcp`·AgingRcp·EFA rcp·WAD MultiMeas rcp 내용 FNV-1a 지문, 다르면 거부** / (b) 형상·G만 비교 | **(a).** HMI는 재개 때 레시피·사이드카를 다시 읽으므로(ICP와 반대) 편집이 끼면 저널 행(이전 형상)과 새 행이 섞인다. 내용 해시라 같은 내용 재저장은 통과 |
| D5-15 ★ | 재개 첫 VISION 레시피 완료 대기 | **(a) 튜플 재개: 첫 VISION에서 1회 대기(스틱 무관)** / (b) 그대로(Phase 4 (12)-10 공백 — 마지막 세그먼트 j>0 재개면 대기 0회) | **(a).** 장치 레시피 전환 전에 grab할 위험 제거. 플래그는 무장 세대에 묶고 `Disarm`이 지운다 — 재개 글라스가 첫 VISION 전에 멈춰도 다음 글라스(N=1 정상 런 포함)에 넘어가지 않는다(리뷰 C7). N=1은 D5-1 (b)일 때만(지금 N=1 j>0 재개도 대기 0회) |
| D5-16 | 저널 없는 하이브리드 북마크(Format=1 + 같은 RecipeID + 튜플 대상 레시피) | **(a) 거부 + Clear(팝업). RecipeID가 다르면 기존 불일치 팝업(리뷰 K12)** / (b) 지금처럼 세그먼트 0 시작 스틱 재개(RS1은 행이 틀림) | **(a).** Phase 4 (12)-5. 술어는 `IsTupleGlass(N)` — D5-1 (a) 동안 N>1 전용, (b) 활성 직후 남은 N=1 Format=1 북마크도 같은 거부(리뷰 C13) |
| D5-17 | 저널 OOS 검증 | **(a) 디버그 키 `RESUME_JOURNAL_OOS_SIM`(1 = `LogHybridRow` 지점 합성 S 줄, 2 = VISION 합성 생략 → 당김 시험, 3 = 1이되 VISION grab을 번갈아 S/X(타임아웃)로 닫음 → X 파싱·짝·비당김 시험, 4 = 1 + 5번째 저널 쓰기를 실패로 → `JournalOK` 시험), 기본 0, OOS PC 전용** / (b) 손으로 만든 저널 픽스처만 | **(a).** OOS에서는 `Updata_Data`가 한 번도 불리지 않는다(매퍼 B5). (a)로 무장·튜플 태그·압축·당김·되감기·재생 페이싱·쓰기 실패까지 시험, 실제 raw 파싱만 실기. 키가 0이면 코드 경로 무영향 |
| D5-18 | INI 원자 교체 | **(a) `MoveFileEx(REPLACE_EXISTING \| WRITE_THROUGH)` (두 저장 함수, N=1 포함 — 같은 바이트·같은 로그)** / (b) Format 2만 / (c) 그대로(Delete → Move 틈) | **(a).** 저널 글라스는 북마크 소실 비용이 커졌다. 내용·로그 동일이라 N=1 무영향 |
| D5-19 공정 | 재개 시 PreMeas 재접촉(리뷰 K15, D19) | **(a) 그대로: 재개 글라스도 엔진의 PreMeas가 세그먼트 0 스틱 0을 다시 접촉한다(레거시 N=1 재개도 같음). EFA T1 재개·ALL DONE 재개에서도 1회 더 접촉** / (b) 재개 글라스는 PreMeas 생략(엔진 변경, D5-6과 함께 Phase 6에서 판단 — 생략하면 Align 상태 가정이 바뀌어 (a) 강제 경계가 필요해진다) | **(a).** Phase 5는 엔진 아이템을 바꾸지 않는다. 다만 EFA T1 재개에서 T0 뒤의 셀을 한 번 더 접촉하는 것이 공정상 허용되는지 **공정 확인 필요**(승인 항목) |

- 묶음 승인 안: "N=1 재개 패키지" = D5-1 (b) + D5-2 (b) + D5-8 (a) + D5-9 (b) + D5-10 (a) + D5-11 (a) + D5-15 (a). 하나라도 거부하면 해당 항목만 튜플 글라스(N>1) 한정으로 남긴다(D5-10은 레거시 경로라 별도).
- 별도 승인: D5-6 (b)(§5.8-3 문구 변경), D5-19 (a)(공정 확인). 나머지(D5-3/4/5/7/12/13/14/16/17/18)는 권장안으로 진행 가능한 설계 선택.
- 기록만(결정 아님): CIM 팝업 타임아웃 없음(D6), APD START_TIME, 늦은 ANGLE `CLASS.END`가 다음 스틱 컨텍스트로 기록될 수 있음(측정 스레드가 타임아웃 뒤 넘어간 경우, 행 값은 맞고 튜플 태그만 다름 — 압축 판정에 영향 가능, (11)), 무장 해제 뒤 도착한 ANGLE 행은 버림, 재생 시작 뒤 도착한 이전 시도의 VISION 행은 그리드에만 들어감((4-4)).
- 설계로 정한 것(리뷰 반영, 결정 항목 아님): 측정 진입 실패·재생 실패 = `SetFail(true, 0)` + 파일 유지(강등 없음, 리뷰 C2/K4), 되돌릴 수 있는 거부 `MB_OKCANCEL`(리뷰 K11), 저널 쓰기 실패 = `JournalOK`/`JournalLines`로 재개 거부(리뷰 K7), 스냅샷 3칸 링 + 정확한 튜플 선택(리뷰 C5/K2), 팝업 중 START 가드(리뷰 C9/K10).

---

**(8) 검증 (OOS)** — 공통: `USE_RESUME_INSPECTION=1`, 비교는 Phase 4 방식(`n1ext2.pl` 시각 제거·휘발 값 정규화, `wadnorm.pl`, `p4oos_chk.pl`). 행 집합 비교용 새 스크립트 `scratchpad/p5oos_rows.pl`(기준 런의 `[HYBRID] result row KIND ... row(a,b)`·`AgingCycle row: ... row(a,b)` ↔ 재개 런의 `[RESUME] replay S KIND row(n)` + 재개 뒤 라이브 L5 줄). INI·저널·스냅샷 확인용 `scratchpad/p5oos_dump.pl`(raw 읽기, 키·줄 수·튜플별 줄 수·스냅샷 헤더/비IDLE 셀 수 출력). 테스트용 파일 손편집은 메모장이 아니라 `scratchpad/p5_oos_edit.pl`(raw, CRLF 유지, 전후 바이트 diff). 사이클 횟수 1(2번째 글라스부터 GlassID가 바뀜, 21번만 2). 레시피·AgingRcp 백업 후 시작. **OOS로 못 보는 것은 코드 리뷰 항목으로 명시**(리뷰 K16): CIM 입구(GlassID·EFA 플래그 비교, 팝업 가드), REMOTE `EFA^0$` 경계 플래그(`Cmd_Send_EFA0` 직후), VISION 150 s 타임아웃·슬롯 재사용(OOS는 `m_bRunStatus`가 안 섬), 재개 첫 VISION 대기 플래그의 세대 묶음(C7).

0. **오프라인(빌드 전·후)**: `p5_journal_sim.pl`(Perl로 압축·당김·정규화 재구현) — 표 입력(튜플 경계, T0/T1 같은 스틱, 짝 없는 P가 플러시 전/후 pass, 찢긴 끝 줄, ALL DONE 끝 표지, `LastStick=-1`, 세그먼트 경계 직후, **X 줄로 닫힌 P, 헤드의 마지막 2개 밖의 오래된 P, raw 없는 R VISION, 같은 InnerID 2회, 당김이 (0,0,0)까지, EFA 링에 `pred(Tp)` 없음, `JournalLines` > 실제 줄 수, `JournalOK=0`**)의 기대 재개점·남는 줄 수·되감기 튜플, `PredTuple` 표, `ResumeForceBoundary` 표(회전 (1,2), (1,1), (1,2,1), (1,2,2), (2,1,1,2) × k). 앵커 재검사(`p5plan/anchors*.pl` + 스테이지 스크립트) 전부 기대 n. `API_Result.h` 두 사본 `cmp` 0. 인코딩 표(9) 재측정, damaged 0, LF-only 파일 CR 0.
1. **N=1 정상 런 회귀(필수, 첫 번째)** — `1_OCP_TEST_Long_260907`, GlassID `ABCDEFG1234`, 시뮬 키 0: R1 EFA(레시피 EFA 플래그 + CYCLE)·R2 AgingTest를 **가장 최근 승인 기준선**(Phase 4b O1을 이미 돌렸으면 그 런, 아니면 Phase 4 2026-10-01 13:25:31 / 13:22:31)과 비교 → GLASS 2/4/6/7/8·엔진·WAD·PG·AGING·화질1/2·시야각1/2 diff 0, SYST `Resume Save/Update/Reset` 줄 diff 0, `[RESUME]` 0줄. 런 중 `p5oos_dump.pl`: `ResumeInfo.ini`에 `Format` 키 없음, `ResumeJournal.csv`·`ResumeCellStatus0~2.dat` 없음.
2. **하이브리드 정상 런** — `9_OCP_TEST_Hybrid_Half_260930`, H1(2026-10-01 13:30:35) 조건: diff는 예상 줄뿐(`[RESUME] new glass bookmark ...` 1, `Resume Begin ...` 1, 세그먼트 0 `Resume Update: Stick=...` → `Resume Update: T0 seg 1/2 stick n/5 ...`, 세그먼트 2 `Resume Update: T0 seg 2/2 ...` 추가). 런 중 INI Format 2·저널 헤더·스냅샷 링 3칸 갱신(`p5oos_dump.pl`: 칸별 튜플·일련번호), 끝 `Resume Reset` 뒤 파일 없음. **마지막 스틱의 `Resume Update: T0 seg 2/2 stick 5/5`가 `CDoWorkGlassMeasureStep_Idle`보다 먼저**(리뷰 C10). 시뮬 키 1로 한 번 더: 저널 S 줄 수 = H1 L5 used 행 수.
3. **§6 ① 세그먼트 2 중간 STOP → 재개(재시작 없음)** — Half, 시뮬 1. seg2 stick 3 측정 중 STOP → dump: `LastSegment=1`, `LastStick=1`, S 줄 = seg1 전부 + seg2 stick 1~2 + stick 3 부분. START → 팝업 "진행 : 세그먼트 2/2, 스틱 2/5까지 완료 (전체 7/10 스틱) / 다음 : 세그먼트 2 스틱 3부터" → 예.
   - GLASS 2: `Resume mode (journal): result grid cleared - journal replay at measure start`, `Resume reload` 줄 없음.
   - GLASS 6: `[RESUME] journal replay: N rows ... dropped M ...`(M = stick 3 부분 줄), `[RESUME] cell status restored (snapshot after T0 seg 2 stick 2)`, `[RESUME] resume point T0 seg 2/2 stick 3/5 (glass stick 8/10) ...`, `[HYBRID] measure start: 2 segments, 1 pass, active segment 1, resume start stick 2`, `[HYBRID] Segment boundary 1 -> 2/2 [...] rot 1 -> 2 : safety + rotate + re-align` + `3/3 glass SW align`, seg1 스틱 줄 없음, `[HYBRID] seg 2/2 ... stick 3/5`~`5/5`.
   - `p5oos_rows.pl`: 재생 S 행 ∪ 재개 뒤 L5 행 = H1 L5 행(다중집합). 끝 `Resume Reset`, 파일 삭제.
4. **§6 ② AgingTest 반복 중 STOP** — Half + 세그먼트 0 `m_bUseAging_Test=1` + AgingRcp 항목 2개(Phase 3b OOS 구성), 시뮬 1: seg1 stick 3의 항목 2 도중 STOP → 재개 → `[RESUME] journal replay: N rows (... sim N) ...`(N = stick 1~2의 모든 블록 행 수, `p5oos_dump.pl` 튜플별 줄 수와 같음), `[RESUME] resume point T0 seg 1/2 stick 3/5 (glass stick 3/10) ...`, stick 3이 항목 1 반복 0부터(`AgingCycle row: ... item(1) repeat(0) stick(2) ...`), 그 행들이 첫 시도의 같은 항목 행과 같음. (D5-1 (b) 활성 뒤 N=1 Long R2로 반복: 5-6.)
5. **§6 ③ EFA T1 중 STOP** — Half + 레시피 `m_bUseEFA` + CYCLE(오프라인 EFA 재개 경로), 시뮬 1: T1 seg2 stick 2 도중 STOP → 재개:
   - `EFA: pass 2/2 start (T1) - 2 segments, segment 1 first`, `EFA: pass boundary skipped on resume (T0 already flushed, EFA^0$ already sent)`, `EFA: pass boundary before pass 2` 줄 **없음**, `EFA: T0 result flush` 줄 없음.
   - `[HYBRID] Segment boundary 1 -> 2/2`, T1 seg2 stick 2~5만, `EFA pass 2: skip step OSTest ...` 있음, `[RESUME] cell status restored (snapshot after T1 seg 2 stick 1)`.
   - 재생 행 = T0 전부 + T1 seg1 전부 + T1 seg2 stick 1. dump: 스냅샷 비IDLE 셀 수 = 정지 직전 값.
   - 변형 5b: `EFA: pass boundary before pass 2` 줄이 찍힌 순간 PAUSE → STOP(플러시 전 `:513` 검사에서 나감) → 재개 → `PassBoundaryDone=0`이라 경계 1회 다시(`EFA: T0 result flush OK ...`), T1 seg1 stick 1부터.
   - 변형 5c: 스냅샷 링 3파일 삭제 후 START → 거부 팝업 "셀 상태 기록이 없어 EFA 검사를 이어갈 수 없습니다" + `Resume Refuse: cell status snapshot for T1 seg 2 stick 1 missing (EFA glass) - fresh start`.
   - 변형 5d(리뷰 K8): `EFA: T0 result flush OK` 직후 STOP(OOS = 비-REMOTE라 플래그는 `not ONLINE REMOTE` 반환 직전에 섬) → dump `PassBoundaryDone=1` → 재개 → `EFA: pass boundary skipped on resume ...`. REMOTE의 `Cmd_Send_EFA0` 직후 플래그는 코드 리뷰.
   - 변형 5e(리뷰 K2): T0 seg2 stick 2 도중 STOP → ALPHA 재기동 → 재개 → `[RESUME] cell status restored (snapshot after T0 seg 2 stick 1)`, dump 비IDLE 셀 수 = 정지 전 값 → 이어서 T1 전체가 돈다. 같은 정지에서 링 3파일 삭제 → T0 재개인데도 거부(EFA 글라스).
   - 변형 5f(리뷰 K2): 5e 정지 뒤 `p5_oos_edit.pl --snapshot`으로 `pred(T)` 칸의 seg1 stick1 셀 하나를 `df_CELLSTATE_OST_OPEN`으로 바꾸고 체크섬 재계산 → 재개 → T1 seg1 stick1에서 그 셀이 측정 대상에서 빠짐(`[HYBRID] result row` 해당 셀 줄 없음).
6. **§6 ④ 프로그램 재시작 후 재개** — 3번처럼 STOP 뒤 ALPHA 종료·재기동 → SYST `ResumeInfoMgr initialized, ... Running=1 ...` + `Resume tuple: Format 2, T0 seg 2/2 stick 1, ...` → START → 예 → 재생 줄 수가 3번과 같고(그리드가 비어 있던 상태에서 복원) 행 비교 통과. 같은 절차를 5번(EFA)에도 1회: 재기동 뒤 공유 플래그가 꺼져 있어도 레시피 EFA라 pass 2 — 플래그 경로 시험은 13번.
7. **§6 ⑤ "아니요" 선택** — STOP 뒤 START → 아니요 → `Resume Cancel: user selected fresh start` + `Resume Reset: info cleared`, 3파일 없음 → 새 글라스 `Resume Begin`, `[RESUME] new glass bookmark`. 레거시 N=1 "아니요"도 1번 조건에서 1회(기존 줄 그대로).
8. **취소(D5-10)** — 팝업 취소 → `Resume Abort: user cancelled inspection start`, 사이클 상태·버튼 변화 없음(START 화면 갱신 없음) → 바로 CYCLE 다시 → 알람 35 없이 팝업 재표시. CIM 경로와 `ResumePromptBusy()` 가드(CIM 팝업 중 CYCLE·다이얼로그 START 거부)는 코드 리뷰(OOS에 호스트 없음, UI 팝업은 모달이라 다른 버튼을 누를 수 없음).
9. **저널 없는 하이브리드(D5-16)** — `p5_oos_edit.pl`로 Format 키 없는 `ResumeInfo.ini`(Half RecipeID, `LastStick=2`, `Running=1`) 작성 → START → 거부 팝업("하이브리드 레시피의 이전 검사 이력에 결과 기록(저널)이 없습니다") + `Resume Refuse: hybrid recipe with a format-1 bookmark (no journal) - fresh start` + Reset → 새 글라스.
10. **저널 손상** — STOP 뒤 저널 삭제 → START → 거부(`journal missing or broken`). 끝 줄을 반쯤 자름 → 재개 성공, `torn 1`. 가운데 줄 3개 삭제(`JournalLines`보다 짧게) → 거부(`journal shorter than recorded`). 저널을 다른 프로세스가 독점으로 연 상태(`p5_oos_edit.pl --lock`)에서 START → "지금 읽을 수 없습니다" + 북마크·저널 그대로(리뷰 C6).
11. **지문(D5-14)·되돌릴 수 있는 거부(리뷰 K11)** — STOP 뒤 AgingRcp `PTN_SEC` 1곳 변경(`p5_oos_edit.pl`) → START → 거부 팝업에서 **취소** → `Resume Refuse: files changed (aging) - start cancelled, bookmark kept`, dump 3파일 바이트 동일 → 원복 → START → 재개 성공. 다시 변경 → **확인** → `... - fresh start` + Reset.
12. **GlassID(D5-8)** — 3번 조건에서 재개 START의 사이클 화면 GlassID를 비워 새 자동 ID → 팝업에 "현재 Glass ID : GLASSID...B.." 줄 → 예 → `[RESUME] GlassID restored from bookmark: ABCDEFG1234 (was GLASSID...)`, 재개 뒤 WAD SEND 셀 ID 접두가 첫 시도와 같음(3번 정지 전 SEND와 비교).
13. **다이얼로그 입구·EFA 모드(D5-11, D5-9 (b))** — AgingNew START(EFA 플래그)로 Half 시작 → T1 중 STOP → ALPHA 재기동(공유 플래그 0) → **OPERATE CYCLE** → 되돌릴 수 있는 거부 "... EFA 화면의 START로 이어서 검사할 수 있습니다" → 취소(북마크 유지) → AgingNew START → 튜플 팝업 → 예 → T1 이어짐(`EFA: mode on (flag 1, recipe 0), passes 2`). 같은 북마크로 AgingTest START → 거부(`EFA mode mismatch`) → 취소. 레거시 N=1 북마크 + 다이얼로그 START → 팝업 없음. 리뷰 C8 회귀: 레시피 EFA(`m_bUseEFA`, 플래그 0) Half 글라스를 CYCLE로 재개해 끝낸 뒤, EFA 없는 레시피로 새 CYCLE → `EFA: mode on` 없음(신규 코드가 `SetSys_NewAgingMode`를 부르지 않음 — post 개수 0).
14. **stale 플래그(D5-10)** — 재개 START 후 Recipe Change를 실패시킴(4b O7 손상 Cell ID 파일, D-15 거부) → 그 상태에서 AgingTest START → `Resume: starting from stick`·`Resume mode` 줄 없음(새 글라스).
15. **ALL DONE(D5-13)** — 3번 정지 북마크를 `p5_oos_edit.pl`로 `LastSegment=1`, `LastStick=4`, `ResultSaved=0`으로(저널은 시뮬로 끝까지 채운 런의 것) → START → "모든 스틱의 측정을 마쳤지만 ..." 팝업 → 예 → `[RESUME] all sticks already measured ...`, `CDoWorkGlassMeasureStep_Idle`, Result `SaveResult` + `Resume ResultSaved: 1`, Unload `CheckSaveReuslt End.` + `Resume ResultSaved: 2`, `Resume Reset`. `ResultSaved=2`로 바꾸면(레시피도 바꿔서) 팝업 없이 `Resume: previous glass already saved - bookmark cleared`(검사 1b가 RecipeID 검사보다 먼저, 리뷰 C12). `ResultSaved=1`이면 "결과 저장을 시작한 뒤 ..." 팝업.
16. **VISION 당김(D5-12)** — VISION 항목이 있는 Half, 시뮬 2(VISION 합성 생략; OOS는 슬롯이 안 비워지므로 시뮬 모드에서만 빈 슬롯이 없어도 P 줄을 쓴다 — (4-3)) → seg2 stick 3 STOP → dump로 헤드별 마지막 2개 P의 튜플 확인 → 재개 → 팝업 VISION 줄 → `[RESUME] VISION pending on T0 seg 2 stick k - resume moved back to that stick (was T0 seg 2 stick 3)`(k = dump 기대값), `Resume Rewind: ...`, 오래된 P는 `[RESUME] VISION never arrived for n grab(s) ... older than the last 2 grabs of head h ...` 후 압축 파일에서 사라짐.
   - 16b(리뷰 C1/K1): 16의 재개 글라스가 stick k를 끝내기 전에 STOP → dump: INI `LastSegment/LastStick` = pred(k)(되감김 유지), 저널 = < k 줄 → START → "다음 : 세그먼트 2 스틱 k" → 재개 → 행 비교(`p5oos_rows.pl`): < k 행 그대로, k 이후 재측정.
   - 16c: 시뮬 3(VISION을 번갈아 S/X로 닫음) → STOP → 재개 → 당김 없음, `[RESUME] journal replay ...`에서 X 줄은 건너뜀.
   - 16d: 당김이 글라스 시작까지 가는 경우(시뮬 2, seg1 stick 2 도중 STOP — 헤드별 마지막 2 grab이 stick 1~2라 Tp = (0,0,0)) → `... glass restarts as a new glass` + `[RESUME] new glass bookmark`.
17. **재생 페이싱** — 시뮬 1에서 재생 시간(`%.1f s`)이 행당 0.1 s 미만(빠른 모드), `UploadDataDone` 뒤 EFA 경계 `SaveResult` 성공(5b).
18. **N=1 재개(D5-1 (a) 동안)** — Long STOP stick 3 → START → **기존 팝업 문구 그대로** → `Resume: starting from stick 3 (total 5)`, `Resume reload SKIP: path not found [...]`(기존과 같음).
19. **Release 빌드 + 스모크** — dResult export 추가로 Release|x64 sln 빌드(`dResult` ↔ ALPHA Release) → Half 3번 시나리오 1회.
20. **5-6 활성 뒤(D5-1 (b) 승인 시)** — `RESUME_TUPLE_ALL=1`: 1번 다시(로그 diff 0, 런 중 3파일 존재·끝에 삭제), Long EFA T1 stick 3 STOP → T1 stick 3~5만(P0-5 해소), Long R2 AgingTest 반복 STOP → 4번과 같은 판정, 18번 → 튜플 팝업(세그먼트 줄 없음).
21. **사이클 2 + 이미 저장된 글라스(리뷰 K3)** — 게이트 0 하이브리드 변형 레시피(Half 복사본에서 광학 항목·AgingTest_2 끔, 레시피 편집기로 만들고 이름 기록) 사이클 2 → 글라스 2를 Align 중 STOP → START → 팝업 없이 `Resume: previous glass already saved - bookmark cleared`. 게이트 1(Half 원본)로 같은 절차 → 같은 결과(글라스 1 Unload에서 `ResultSaved=2`).
22. **재생 중 STOP / 일시정지(리뷰 C2/K4)** — 3번 정지 뒤 `p5_oos_edit.pl --pad-journal 3000`(< T S 줄 복제)로 재생을 길게 → START → 예 → `[RESUME] replay ...` 진행 중 STOP → `[RESUME] journal replay stopped by user at a/b - bookmark and journal kept` + 정지 경로 로그, Attempt+1 외 INI·저널 줄 수 그대로 → START → 같은 재개점 제안. 같은 조건에서 재생 중 PAUSE 15 s → RESUME → 재생이 이어져 끝남(실패 없음, 폴링 횟수 타이머).
23. **저널 쓰기 실패(리뷰 K7)** — 시뮬 4 → SYST/WARN `ResumeJournal: write failed ...` 1회, `Resume Update: journal write failed earlier ...` 1회, 측정은 끝까지 → 도중 STOP한 경우 START → 거부 "결과 기록(저널) 쓰기에 실패한 적이 있어 ...".
24. **3세그먼트 방향(D5-6, 리뷰 C11/K5)** — 0번 표 + 코드 리뷰가 기본. 회전 (1,2,1) 3세그먼트 픽스처를 편집기로 만들 수 있으면: seg3 stick 2 STOP → 재개 → `[HYBRID] Segment boundary 1 -> 3/3 ... rot 1 -> 1 : safety + rotate + re-align (resume: forced re-align)`.
25. **팝업 가드·VISION 대기 플래그(리뷰 C9/K10/C7)** — 코드 리뷰: CYCLE 가드 위치(기존 가드 3개 다음, 확인 메시지 박스 앞), `ResumeDialogEntry` 첫 줄, `ConsumeResumeVisionWait`의 무장·세대 조건, 측정 시작·끝 `Disarm()`.
- **실기 전용**: 실제 raw 파일 재생 후 그리드·xls·APD가 중단 없는 글라스와 같은지(행·값·VIS_DEFECT 순서, 두 번째 `ClearData` 뒤 결함 벡터 중복 없음), 누락 raw 수 표시, VISION 늦은 도착(150 s)과 당김, **VISION 타임아웃(X 줄) 뒤 재개 시 당김 없음**, 정지 뒤 도착한 VISION이 P를 닫음, CIM T1 재개 시 호스트 로그에 `EFA^0$` 1회, ANGLE 재생, 재생 속도 실측, CIM 입구 GlassID·EFA 플래그 거부와 취소.

---

**(9) 수정 파일·인코딩** (2026-10-06 `p5plan/enc.pl`, Perl raw: BOM / 엄밀 디코드 / CRLF·LF 수 / damaged)

| 파일 | 단계 | 인코딩 / EOL | 비고 |
|---|---|---|---|
| `CoreFunction/dResult/CDlgResult.cpp` / `.h` | 5-0 | **UTF-8 BOM**, CRLF 2406 / 143 | ASCII만 삽입 |
| `CoreFunction/dResult/ResultManager.cpp` / `.h` | 5-0 | CP949 CRLF 114 / ASCII CRLF 33 | |
| `CoreFunction/dResult/API_Result.cpp` | 5-0 | ASCII CRLF 77 | |
| `CoreFunction/dResult/API_Result.h` + `Source/Include/API_Result.h` | 5-0 | CP949 CRLF 141 (두 사본 동일) | 둘 다 패치(ALPHA ClCompile이 DLL 빌드 전에 돌기 때문), `cmp` 0. `RSRITEMDATA` 무변경 |
| `CoreFunction/dResult/dResult.def` | 5-0 | **UTF-8 BOM** CRLF 16 | |
| 신규 `Source/ALPHA/Manager/ResumeJournal.h` / `.cpp` | 5-1 | ASCII CRLF | Perl로 CRLF 작성 |
| `Source/ALPHA/Manager/ResultMgr.cpp` / `.h` | 5-1 | ASCII CRLF 73 / CP949 CRLF 34 | |
| 신규 `Source/ALPHA/ResumePrompt.h` / `.cpp` | 5-1(뼈대) → 5-3(본문) | `.h` ASCII CRLF / `.cpp` **CP949** CRLF | UTF-8 원본 → `encode('cp949', FB_CROAK)`. Edit 도구 금지 |
| `Source/ALPHA/ALPHA.vcxproj` / `.filters` | 5-1 | UTF-8(BOM 없음) CRLF 947 / **UTF-8 BOM** CRLF 2350 | filters는 한글 필터명 raw 복제 |
| `Source/ALPHA/Manager/ResumeInfoMgr.h` / `.cpp` | 5-2 | CP949, **LF only** 87 / 387 (CR 0) | LF 정규화 → `\n` 패치 → LF 저장 |
| `Source/ALPHA/DlgSubOperate.cpp` | 5-3 | CP949 CRLF 4378 | `.h` 무변경 |
| `Source/ALPHA/DlgSubManual_AgingTest.cpp` | 5-3 | **UTF-8 BOM** CRLF 1143 | ASCII만 삽입(첫 3바이트 `EF BB BF` 재확인) |
| `Source/ALPHA/DlgSubManual_AgingNew.cpp` | 5-3 | CP949 CRLF 1140 | 플랜 §10의 "LF-only"는 낡음 |
| `Source/ALPHA/DoWork/DoWorkGlassMeasure.cpp` / `.h` | 5-4 | CP949 CRLF 777 / 33 | |
| `Source/ALPHA/DoWork/DoWorkGlassRecipeStep.cpp` | 5-4 | CP949 CRLF 1195 | |
| `Source/ALPHA/DoWork/DoWorkGlassResultStep.cpp` | 5-4 | CP949 CRLF 147 | 공백 들여쓰기 |
| `Source/ALPHA/DoWork/DoWorkGlassUnloadStep.cpp` | 5-4 | CP949 CRLF 306, damaged 0 (2026-10-06 `p5fix/anc_fix.pl`) | 리뷰 C15로 추가(P18b) |
| `Source/ALPHA/DoWork/DoWorkGlassMeasureStep.cpp` | 5-4 | CP949 CRLF 7089 | 반복 블록 많음 — 줄 전체 앵커 |
| `Source/ALPHA/CommTcpIpObjectVision.cpp` / `.h` | 5-4 | CP949 CRLF 1686 / 113 | `.h` 변경 → 넓게 재컴파일 |
| (선택) `Source/Bin/Config*/SystemConfig.ini` | 5-6 / OOS | (패치 전 측정) | 사용자 결정 후, 키 추가만 |
| `docs/*.md` | 5-7 | UTF-8 | Edit 가능 |
- 무변경 확인: `DoWorkGlassSequenceEngine.cpp`(CP949 CRLF 1050), `SystemParamShare.h`(CP949 CRLF 194), `RecipeEditMgr.h`(CP949 CRLF 179), `D_MeasureCycle.cpp`(CP949 CRLF 291), `AgingCycleInfo.h`(CP949 **CRLF** 61 — 루트 CLAUDE.md LF-only 목록이 낡음). 전 파일 damaged 0.
- 패치 방식: `pfw.pl` 스타일(`rd` LF 정규화, `rep1`/`ins_after`/`ins_before`, 앵커 `index` 2회 유일성, `post` 개수, EOL 원복), 새 코드 ASCII(한글은 `ResumePrompt.cpp` 팝업 리터럴만, `use utf8` + `Encode::encode("cp949", ..., FB_CROAK)`). 스크립트는 scratchpad 파일로 작성 후 `perl` 실행(Bash 인라인 금지). 백업 `scratchpad/p5_bak`.
- 전부 `HMI/` git 트리. 구현 뒤 `git -C HMI status` 확인, 커밋 여부는 사용자에게 묻는다(새 파일 4개 미추적 주의 — 두 PC 동기화 유실 사례).

---

**(10) 구현 단계** (스크립트 `p5_patch.pl --stage=N`, 스테이지마다 단독 컴파일)

| 단계 | 내용 | 파일 | 의존 | 병렬 | 컴파일 확인 |
|---|---|---|---|---|---|
| **5-0 dResult** | P1–P4 | CDlgResult.h/.cpp, ResultManager.h/.cpp, API_Result.cpp, API_Result.h×2, dResult.def | 없음 | 단독(맨 먼저) | dResult `CDlgResult.cpp`, `ResultManager.cpp`, `API_Result.cpp`; `cmp` API_Result.h |
| **5-1 저널 코어** | P5, P6, P7, `ResumePrompt.h/.cpp` 뼈대(FRESH 반환) | 신규 ResumeJournal.h/.cpp, ResultMgr.h/.cpp, 신규 ResumePrompt.h/.cpp, ALPHA.vcxproj/.filters | 5-0(API 선언) | 단독 | ALPHA `Manager\ResumeJournal.cpp`, `Manager\ResultMgr.cpp`, `ResumePrompt.cpp` |
| **5-2 북마크** | P8, P9 | ResumeInfoMgr.h/.cpp (LF) | 5-1(`S_ResumeShape`, `DeleteFiles`) | 단독 | ALPHA `Manager\ResumeInfoMgr.cpp`, `AuroraMgr.cpp`(헤더 포함 확인) |
| **5-3 진입점** | P10 본문, P11, P12, P13 | ResumePrompt.cpp, DlgSubOperate.cpp, DlgSubManual_AgingTest.cpp, DlgSubManual_AgingNew.cpp | 5-1, 5-2 | **5-4와 병렬** | `ResumePrompt.cpp`, `DlgSubOperate.cpp`, `DlgSubManual_AgingTest.cpp`, `DlgSubManual_AgingNew.cpp` |
| **5-4 시퀀스** | P14–P20, P18b | DoWorkGlassMeasure.h/.cpp, DoWorkGlassRecipeStep.cpp, DoWorkGlassResultStep.cpp, DoWorkGlassUnloadStep.cpp, DoWorkGlassMeasureStep.cpp, CommTcpIpObjectVision.h/.cpp | 5-1, 5-2 | **5-3과 병렬**(내부 4a = Measure/Recipe/Result/Unload, 4b = MeasureStep/Vision으로 다시 나눠도 파일 겹침 없음) | `DoWork\DoWorkGlassMeasure.cpp`, `DoWork\DoWorkGlassRecipeStep.cpp`, `DoWork\DoWorkGlassResultStep.cpp`, `DoWork\DoWorkGlassUnloadStep.cpp`, `DoWork\DoWorkGlassMeasureStep.cpp`, `CommTcpIpObjectVision.cpp`, `Manager\CommTcpIpFactoryMgr.cpp`(Vision.h 포함 확인), `DoWork\DoWorkGlassSequenceEngine.cpp`(Measure.h 기본 인자 변경 확인) |
| **5-5 전체 빌드 + OOS (a)** | — | — | 5-0~5-4 | — | ALPHA 실행 중이면 사용자 확인 후 `taskkill //IM ALPHA.exe //F` → sln `//t:dResult:Rebuild` → sln 일반 빌드(ALPHA 재링크), 로그 `vcxproj ->` 확인 → Release|x64 같은 순서 → (8) O0~O19, O21~O25 |
| **5-6 N=1 활성** | `RESUME_TUPLE_ALL=1` (D5-1 (b) 승인 시만) | SystemConfig.ini(사용자 결정) | 5-5 통과 | — | 재빌드 없음 → (8) O20 |
| **5-7 문서** | 아래 | `docs/*.md`, HMI/CLAUDE.md | 5-5(5-6) | — | — |
- 같은 파일을 두 병렬 스테이지가 만지지 않는다: vcxproj/filters와 `ResumePrompt.cpp` 생성은 5-1만(5-3은 본문 채우기 — `ResumePrompt.h`는 5-1에서 `ResumePromptBusy`/`CResumePromptBusy`까지 완성해 둔다, 5-3·5-4가 함께 쓰므로), `ResultMgr.*`는 5-1만, `ResumeInfoMgr.*`는 5-2만, UI 3파일 + ResumePrompt 본문은 5-3만, DoWork·Vision은 5-4만.
- ClCompile(파일 하나씩 — `SelectedFiles`에 여러 개를 넣으면 하나도 선택되지 않는다. 하위 폴더 경로는 PowerShell로. VS와 ALPHA가 실행 중이므로 IntDir은 scratchpad):
```powershell
# scratchpad\p5_cl.ps1  - ClCompile of ONE file (Debug|x64) into a stage IntDir. No link, no sln.
param([Parameter(Mandatory=$true)][ValidateSet('dres','alpha')][string]$Proj,
      [Parameter(Mandatory=$true)][string]$File, [Parameter(Mandatory=$true)][string]$Tag)
$msb = 'C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe'
$sp  = 'C:\Users\C0000183\AppData\Local\Temp\claude\D--OCP-0-SW-Matching\2b7657ff-4f5d-4f68-ac4c-b3dc943d5551\scratchpad'
if ($Proj -eq 'dres') { $vcx = 'D:\OCP\0.SW_Matching\HMI\CoreFunction\dResult\dResult.vcxproj'; $int = "$sp\p5_int_dres\" }
else                  { $vcx = 'D:\OCP\0.SW_Matching\HMI\Source\ALPHA\ALPHA.vcxproj';           $int = "$sp\p5_int_alpha\" }
$log = "$sp\build_p5_$Tag.u8"
& $msb $vcx '/t:ClCompile' '/p:Configuration=Debug' '/p:Platform=x64' "/p:SelectedFiles=$File" "/p:IntDir=$int" '/nologo' '/v:minimal' 2>&1 | Out-File -FilePath $log -Encoding utf8
"exit=$LASTEXITCODE log=$log"
```
  - `p5_int_alpha`는 첫 호출 전에 `p4b_int_alphaF`(ALPHA.pch·pch.obj 포함)를 복사해 PCH를 재사용(우리 헤더는 `pch.h`에 없음). `p5_int_dres`는 비어 있으면 `pch.cpp`부터(`/p:SelectedFiles=pch.cpp`). 예: `powershell -File p5_cl.ps1 -Proj alpha -File DoWork\DoWorkGlassMeasure.cpp -Tag 54_meas`.
  - 경고 비교: Phase 4b `p4b_cc_warn.pl` 방식으로 새 경고 0 확인(C4996 등 기존 경고는 줄 번호 이동만).
- **공통 절차**: 편집 전 백업, 앵커 `index` 2회 유일성(예외: `DeleteFile(m_strFilePath);` n=3 / `MoveFile` n=2는 함수 범위 안에서 1회), 치환 개수 `post`, 편집 뒤 damaged 0·`FB_CROAK` 엄밀 디코드·EOL 수 일치(LF-only 파일은 CR 0, UTF-8 BOM 파일은 BOM 유지), 바뀐 블록 디코드 재확인, `API_Result.h` 두 사본 `cmp`.
- **5-7 문서 갱신 목록**
  - `HybridGlass_Porting_Plan.md` §5.8: 줄 번호 정정(`DoWorkGlassMeasure.cpp:361` → `:470`, `DlgSubOperate.cpp:3474-3525` → `:3479-3532`), 저널 위치 = `Config`(D5-4), 강제 경계 결정(D5-6), RS1 경로 실상(`{DD}`) 기록, "스냅샷 없으면 강등" → 팝업 거부. §6 Phase 5 상태·D5-1~18·OOS 결과, Phase 0 P0-5 해소 표시(튜플 글라스/(b)), D3c-11 운용 지침 해제 범위, §10 표(ResultMgr.cpp는 ASCII, AgingNew는 CRLF, 새 파일 4개).
  - `docs/Resume_Inspection_Plan.md:47, :97, :273-276`(`{YYYYMMDD}` → `{DD}`), `:90`(InnerID 식별 오기), `:256`(플래그 리셋 서술), `:301`(iMeasArea 식) — 정정 + Phase 5 링크.
  - `HMI/CLAUDE.md` "Resume Inspection" 절(`:289` 서술 정정, Format 2·저널·스냅샷 파일, 로그 태그 `[RESUME]`).
  - 루트 CLAUDE.md LF-only 목록에서 `DoWork/AgingCycleInfo.h/.cpp`·`DlgSubManual_AgingNew.*` 제거 제안, 새 파일 인코딩(`ResumePrompt.cpp` = CP949) 기록 제안(루트는 git 밖 — 사용자 결정).
  - log-analyze 스킬에 `[RESUME]`·`Resume Begin/Update/Boundary/Refuse` 줄 해석 추가 제안.

---

**(11) 리스크**

| 리스크 | 대응 |
|---|---|
| dResult 빠른 모드에서 FPSpread 쓰기 폭주(모니터 스레드가 UI 소유 OCX를 연속 호출) | 큐 깊이 8 상한·10 ms 폴링으로 페이싱, 지금도 모니터 스레드가 쓰는 경로 그대로. 실기 측정(O17/실기) |
| 재생 중 다른 업로드(이전 시도의 늦은 VISION·ANGLE) | 재생 첫 줄 전에 dResult 큐가 빌 때까지 기다린 뒤 `ClearData()`를 한 번 더(리뷰 K9). 그 뒤 도착분은 라이브 경로로 그리드에만 들어가고 저널에는 없다(세대 변경). 처리 수 비교는 `>=` → 기록 |
| 재생 중 STOP·일시정지 | 줄마다·대기 루프마다 `IfPauseEvt_WaitResumeEvt()` 폴링, 정지면 파일 유지하고 정지 경로, 타이머는 폴링 횟수라 일시정지 시간이 들어가지 않음(리뷰 C2/K4) |
| VISION 당김이 너무 멀리 감 / 되감기 전 두 번째 정지로 행 소실 | 당김 자격 = 헤드의 최근 2 grab + 플러시 안 된 pass + EFA는 스냅샷 있음, 타임아웃은 X로 닫음, INI 되감기를 저널 교체보다 먼저(리뷰 C1/C4/K1/K6) |
| 팝업 예고와 실제 재개점이 다름 | 팝업 뒤 늦은 VISION 도착으로 당김이 사라질 수 있다 — 실제 값은 `[RESUME] resume point` 로그, 운전자 혼동 가능성은 기록 |
| 수동 CYCLE 팝업 중 CIM MEAS_START가 겹침 | 지금도 있는 틈(CIM 경로는 사이클 상태를 검사하지 않음), 무변경. 가드는 CIM 팝업 중 수동·다이얼로그 START만 막는다 |
| `nCellStatus` 복원이 UI 리셋보다 앞섬 | 리셋 카운터 대기(5 s, STOP 폴링). None 전에 게시된 리셋이 큐에 남는 경우만 조기 통과 — UI가 수 초 막혀야 가능. 첫 재개 스틱 전 WARN으로 드러남 |
| 스냅샷이 재개점보다 앞섬·옛 NG가 재측정 열에 남음 | 3칸 링 + `pred(T)` 정확 선택, T0는 열 ≥ gs(T) IDLE로 지움, EFA는 정확한 칸 없으면 재개 불가(리뷰 C5/K2) |
| 늦은 ANGLE `CLASS.END`가 다음 스틱 컨텍스트로 기록 | 행 값은 맞음, 튜플 태그만 뒤 스틱 → 압축에서 그 행이 지워지면 재측정으로 다시 생김(데이터 손실 없음). 기록 |
| 자동 런 일시정지 중 수동 측정이 저널에 섞임 | 사이클 중 수동 화면 진입 가능 여부 확인(Phase 7), 필요 시 `OnUpload`에 `IsCycleState()` 추가 게이트 |
| 저널 쓰기 실패(디스크) | 끈적 플래그 → `Commit()` false → 북마크 `JournalOK=0`, `JournalLines`로 줄 수 기록 → 재개 때 거부(리뷰 K7). WARN 1회, 측정은 계속 |
| `ResultSaved`가 실제 저장보다 앞섬 | 2단 표시: Result 완료 = 1, Unload `SaveResultDone` 확인 = 2. 1이면 다시 저장할지 묻는다(리뷰 C15) |
| 파일 지문이 너무 엄격(무관한 재저장에도 바뀌는 필드) | 내용 해시라 같은 내용이면 통과. 레시피 저장이 시각 등을 기록하면 거부가 잦아짐 → OOS 11에서 "같은 내용 재저장" 1회 확인 |
| EFA T0 → T1 간격이 재개 대기만큼 늘어남 | 공정 판단(필요하면 `RESUME_EXPIRE_HOURS`로 상한) |
| CIM 팝업이 ONLINE REMOTE에서 사람을 기다림(D6) | 기존. Phase 6/운용 |
| `S_QualityWait`·`API_Result.h`·`ResumeInfoMgr.h` 변경으로 넓은 재컴파일 | 5-1/5-2/5-4 단독 ClCompile 먼저, 5-5 전체. `RSRITEMDATA`는 무변경이라 링 ABI 영향 없음 |
| dResult DLL만 바뀐 증분 빌드가 최신으로 착각 | `//t:dResult:Rebuild` → 일반 빌드 순서 고정(루트 CLAUDE.md), `vcxproj ->` 확인 |
| 두 PC 배포 불일치(구 ALPHA가 Format 2 INI를 읽음) | 구 빌드는 새 키를 무시하고 레거시로 해석 → 하이브리드 글라스는 구 빌드에서 `LastStick exceeds`/세그먼트 0 재개(지금과 같은 상태). 두 PC 동시 배포 공지 |
| 같은 글라스 반복 재개로 Attempt 증가·저널 비대 | 압축이 재개마다 불필요 줄 제거, 상한 6 MB |
| OOS 시뮬 키가 실기에 남음 | 키 기본 0, 실기 `SystemConfig.ini`에 넣지 않는다는 배포 체크리스트. 키가 켜지면 SYST 부팅 로그 1줄로 드러남 |
| 0회 반복 경로(잘못된 튜플)로 엔진 무한 대기 | `GetResumePoint` −1 거부, ALL DONE은 Init → Exit 직접 실행, 측정 진입 재검사 실패는 `SetFail(true, 0)` + Exit 스텝(파일 유지) |
| 측정 진입 실패가 반복됨(팝업은 통과, 측정 진입은 실패) | 경합 때만 생김(예: 팝업 뒤 CIM PPID가 플래그를 바꿈). 운전자는 다음 팝업에서 아니요로 새로 시작할 수 있다 — 기록 |
| 일시정지(`SuspendThread`)가 저널 잠금 보유 스레드를 세움 | 무장 안 됨은 잠금 없이 반환, 잠금 안에서는 포맷·`WriteFile`만. 무장 중 수동 측정은 잠깐 기다릴 수 있다 — 기록 |

---

**(12) Phase 6/7에 넘길 것**
1. Phase 6(운전 화면): 재개 진행 표시(세그먼트 n/N, T0/T1, 재생 진행률) — 지금은 로그·팝업뿐. 셀맵은 `nCellStatus` 복원으로 자동 반영된다(`UpdateHybridCellStatus`). CIM 팝업 타임아웃/자동 응답 정책(D6).
2. Phase 6: 재개 시 PreMeas·Align 생략으로 시간 단축(D19, D5-19 (b)) — 강제 경계(D5-6 (a))와 함께 판단해야 한다(생략하면 (a)가 필요해진다). D5-6 (b)가 남긴 차이(같은 방향 구간 안에서 세그먼트별 `.alg`가 다를 때 Align 피두셜 차이)도 이때 같이 닫는다.
3. Phase 7(수동 화면): 수동 측정은 저널 대상이 아니다 — 자동 런 일시정지 중 수동 측정 허용 여부와 `OnUpload` 게이트. `DlgSubManual_Meas`(UTF-8 BOM)는 무변경.
4. 별건: APD `START_TIME`을 북마크 `StartTime`으로 복원할지, RS1 레거시 코드 제거 시점(D5-1 (b) 활성 후 구 북마크가 사라지면), VISION 슬롯 2개 초과 grab 손실(기존), `MakeUniqueInnerID` 재시작 후 목록 소실(시계 역행 때만 충돌).
5. Phase 4b OOS가 아직이면 (8) O1의 기준선은 Phase 4 런 — 4b O1을 먼저 돌려 기준선을 갱신해 두면 5-5 비교가 단순해진다.
6. 기존 라이브 결함(기록, Phase 5는 저널만 보호): VISION `QualityWaitThread`가 대기 뒤 슬롯을 다시 읽어 Recipe Change로 지워지고 재사용된 슬롯의 InnerID를 파일 없이 토스한다(`CommTcpIpObjectVision.cpp:1541-1544`), `TossResultFile`은 `CopyFile` 실패에도 `Updata_Data`를 부른다(`:1494-1516`), 늦은 VISION 행이 다음 글라스의 GlassID·날짜 폴더로 들어간다(토스 시점의 `GetGlassID()`) — 별건 후보.

---

**(13) 리뷰 반영** — 리뷰 원문 `scratchpad/p5_plan_reviews.md`. 모든 항목은 코드로 다시 확인한 뒤 반영했다(확인 위치는 각 행, 디코드 사본 `scratchpad/p5fix/dec/` = 리뷰어 사본 `rv5/`와 `cmp` 동일, 새 앵커·인코딩 `p5fix/anc_fix.pl`). C = correctness·N=1 리뷰, K = completeness·risk 리뷰. 같은 지적은 한 행에 묶었다.

| # | 지적(요지) | 코드 확인 | 판정 | 플랜에 바뀐 것 |
|---|---|---|---|---|
| C1 / K1 (high) | VISION 당김이 저널 줄을 지우고 북마크는 되돌리지 않음 → 재개 글라스가 Tp 완료 전에 다시 멈추면 [Tp, T) 행이 영구 소실. 팝업도 당김을 모름 | `DoWorkGlassMeasure.cpp:465-472`(진행 저장은 스틱 완료 때만), `ResumeInfoMgr.cpp:267-273`(`IsResumeAvailable`은 `LastStick >= 0` 필요) | **반영** | (3-2) 6단계 압축 3단계: tmp 작성 → INI `ApplyCompaction`(Last* = pred(Tp), `ResultSaved=0`, Attempt, `JournalLines`) **먼저** → 저널 교체. Tp = (0,0,0)이면 새 글라스(남길 줄 없음). (1-2) `PredTuple`, (1-3) 쓰는 지점 행, (6-2) `ApplyCompaction`, `Analyze`를 팝업·측정 공용으로 → (2-3) 팝업 VISION 당김 줄, ALL DONE 팝업 대신 재개 팝업. (8) O16b |
| C2 / K4 (high / med) | 재생 중 STOP·dResult 정체가 강등 → 저널 잘림·북마크 리셋. 재생에 정지 폴링 없음, STOP은 첫 Init의 `SetStopUser(false)`가 지움, 일시정지는 10 s 타이머를 오판 | `DoWorkGlassMeasureStep.cpp:557`, `BaseSequenceBody.cpp:46-64`(`IfPauseEvt_WaitResumeEvt`), `BaseSequenceImpl.cpp:144-149`·`DoWorkGlassSequenceEngine.cpp:538`(`SuspendThread`), 엔진 실패 처리 `:960-975`·`:991`, `AlarmManager.cpp:476`(id < 1000 무시) | **반영** | "측정 진입은 파일을 지우지 않는다" 원칙((0) 안전 원칙): `ResumeTupleEnter` → OK/NEW/STOP/FAIL, STOP = 기존 정지 경로(Exit 스텝), FAIL = `SetFail(true, 0)` + Exit + 파일 유지, NEW는 Tp = (0,0,0)뿐. (4-4) 줄마다·대기 루프마다 `IfPauseEvt_WaitResumeEvt()` 폴링, 타이머 = 폴링 횟수(일시정지 제외), 셀 상태 대기도 폴링. (2-2) 측정 진입 문단, (11) 행 2개, (8) O22 |
| C3 / K8 (med / low-med) | `PassBoundaryDone`을 `EfaPassBoundary` 반환값으로 세우면 끝의 일시정지→정지 false 때 `EFA^0$`·APD·T0 파일 재송신 | `DoWorkGlassMeasure.cpp:513`(플러시 전 정지 검사), `:537-541`(비-REMOTE 반환), `:544`(`pCIM == NULL`), `:559-563`(`Cmd_Send_EFA0` → `WaitEFA0Ack(5000)` → 정지 검사) | **반영(강화)** | `EfaPassBoundary(int nPass, bool bTupleGlass = false)` 안 3곳에서 플래그: 비-REMOTE 반환 직전, `pCIM == NULL` 반환 직전, **`Cmd_Send_EFA0()` 직후**(리뷰안 "`WaitEFA0Ack` 뒤"보다 앞 — ACK 대기 중 크래시도 재송신 없음, ACK는 D-2상 완료 조건 아님). (3-2)(C) 호출부, (3-4), (1-1)·(1-3), (6) P14/P15·post, D5-7, (8) O5d + REMOTE는 코드 리뷰 |
| C4 / K6 (med) | 당김 거리 제한 없음: 150 s 타임아웃·결과를 영영 못 받은 슬롯이 그 스틱부터 전부 재측정. 대기 뒤 슬롯을 다시 읽어 재사용 슬롯이 P를 거짓으로 닫음 | `CommTcpIpObjectVision.cpp:1522-1567`(타임아웃이면 토스 없이 `Clear`, 대기 뒤 `sQualityWait[i].m_strInnerID` 재사용), `:1494-1516`(`CopyFile` 실패에도 `Updata_Data`), `.h:8` `QUALITY_MAX 2`, `DoWorkGlassMeasureStep.cpp:3844`(`CmQualityEmptyTimeOut(NOT_LAST_ITEM, ...)`), `:1596-1610`(`QualityWaitSet`), `DoWorkGlassRecipeStep.cpp:862`(`EXE_QUALITY_RESET`) | **반영** | (4-2) P에 `<head>`(`m_iDeviceIndex`), X 줄(타임아웃) 신설, InnerID 짝, raw 없는 R VISION은 P를 닫지 않음. (4-3) 대기 **전** 지역 사본, 재사용 감지 시 저널 제외, 타임아웃 `else`에 `AppendClose`(라이브 무변경). (4-5) 당김 자격 = 헤드의 마지막 2 P + 플러시 안 된 pass + EFA는 스냅샷 있음, 자격 없는 P는 WARN 후 압축에서 버림. D5-12 비용 문구 정정. (8) O16/O16c, 실기 항목 |
| C5 (med) | 스냅샷이 재개점보다 새것일 수 있고 통째 복원하면 재측정 열에 옛 NG가 남음 | `DoWorkGlassMeasureStep.cpp:2571-2612`(NG 셀 제외), `CommTcpIpObjectAgingPG.cpp:803-948`(AGING_NG를 OK로 못 되돌림), `DlgSubOperate.cpp:2789-2796`(`GenGlassPath` 유일한 일괄 리셋) | **반영(쓰기 순서 제안만 거부)** | (5-1) 3칸 링(튜플·일련번호 태그), `pred(T)` 정확 선택, T0는 열 ≥ gs(T) IDLE 지움. **거부**: "INI → 스냅샷" 순서 — 링 + 정확 선택이면 "저널 → 스냅샷 → INI"가 더 안전하다(스냅샷과 INI 사이 크래시 때 `pred(T)` = L−1 칸이 남아 정확히 복원, 반대 순서면 L 칸이 없어 EFA 글라스 거부). 근거를 (5-1)에 적음 |
| K2 (high) | T0에서 멈춘 EFA 글라스도 스냅샷이 기능상 필수, 스냅샷 튜플을 북마크와 대조하지 않음 | `DoWorkGlassMeasure.cpp:306`(T1 사전검사 생략), `DoWorkGlassMeasureStep.cpp:2571-2586` | **반영** | (2-2) 7: EFA 글라스는 pass 무관 `pred(T)` 정확 스냅샷 필수, 7b 비-EFA 안내. (5-1) 필요성 문구 정정("T0 재개는 표시용" 삭제). (3-2) 5단계. (8) O5c/O5e/O5f |
| C6 / K14 (med / low) | 저널 핸들 수명·공유 미정 → 공유 위반이 "손상"으로 처리돼 재개 파괴, 무장 해제 뒤 늦은 VISION, 잠금 순서, 일시정지 중 잠금 | `CommTcpIpObjectVision.cpp:1541-1544`(정지 뒤에도 토스), `:413`(수동 측정에서도 `QualityWaitSet`), `BaseSequenceImpl.cpp:144-149` | **반영** | (4-3) 쓰기 핸들 `FILE_SHARE_READ \| FILE_SHARE_DELETE`, 읽기 `READ\|WRITE\|DELETE`, `Disarm`이 잠금 안에서 닫음, 늦은 줄 = 열기-쓰기-닫기(선택·기록), `DeleteFiles`/교체는 핸들 먼저 닫음, `AppendPending`/`AppendClose`도 세대 규칙, 오버라이드 `nGen 0` = 버림, 무장 안 됨 = 잠금 없이 반환, 잠금 순서 `m_cs` → 저널, 저널 잠금 중 다른 관리자 호출 금지. (2-2) 읽기 실패 = 지우지 않고 취소. (8) O10 잠금 변형 |
| C7 (med) | D5-15 재개 VISION 대기 플래그가 다음 글라스(N=1 정상 런 포함)로 넘어감 | `DoWorkGlassMeasureStep.cpp:3885-3907`(소비는 VISION 첫 측정점에서만), `RecipeEditMgr.cpp:462-468`(`TakeVisionRcpWait`, N=1 늘 true) | **반영** | (3-6) 플래그를 무장 세대에 묶고 `Consume`은 무장·세대 일치일 때만 true, `Disarm`이 지움, (3-2)(B) 측정 시작·끝에서 모든 글라스가 `Disarm()`(무동작·무로그). D5-15 문구, (5-2) 행, (8) O25 코드 리뷰 |
| C8 (med) / K13 기록 | OPERATE YES의 EFA 플래그 복원이 이후 비재개 CYCLE 글라스까지 EFA로 돌림 | `CommTcpIpObjectCIM.cpp:1419`/`:1463`, `DlgSubManual_AgingNew.cpp:287`/`:531`, `DlgSubManual_AgingTest.cpp:478`(유일한 쓰기), `MeasureSequenceItemEngine.cpp:896-920`(측정 시작에 플래그 읽음) | **반영(리뷰의 대안)** | 리뷰 1안(원복 로직)은 N=1 MeasDone이 Unload에서 플래그를 읽어 원복 시점이 복잡하므로 대신 2안 계열로: **플래그를 쓰지 않는다**. D5-9 재작성 — (b) 입구 조건(OPERATE·CIM = 현재 플래그 일치, AgingTest = `EfaFlag 0`, AgingNew = `EfaMode 1`), 다르면 되돌릴 수 있는 거부 + "EFA 화면의 START" 안내. (2-2) 9, (2-3) YES 적용, (5-2) 행, (5-3), post `SetSys_NewAgingMode` 0, (8) O13 |
| C9 / K10 (low-med) | CIM 검사를 앞으로 옮기면 소유 창 없는 워커 팝업 동안 CYCLE로 두 번째 엔진 시작 가능 | `DlgSubOperate.cpp:3425-3449`, CYCLE 가드 `:1736-1751`, 비-REMOTE 버튼 `:3438-3443`·`:3559`, 다이얼로그 START는 `IsCycleState`를 안 봄(`DlgSubManual_AgingTest.cpp:474-495`, `AgingNew.cpp:527-550`) | **반영(가드 안)** | 리뷰 1안(옮기지 않고 취소 때 되돌림)은 되돌릴 UI 갱신이 `WM_MSG_UPDATE_CYCLE_INSPECTION_COMPLETE`(팝업·PLC 비트, `DlgMainForm.cpp:677-725`)와 얽혀 비권장 → D5-10 (a')로 기록. 2안 채택: `ResumePromptBusy()`/`CResumePromptBusy`, CYCLE 가드(기존 가드 3개 다음, 알람 35), `ResumeDialogEntry` 첫 줄 거부. (2-1) 표·코드, (6) P10/P11, (11) 행, (8) O8·O25 코드 리뷰 |
| C10 (low) | 마지막 스틱의 Commit·스냅샷·진행 저장이 Idle(완료 이벤트) 뒤에 돌아 Result/Unload/`ClearResumeInfo`와 경합 | `DoWorkGlassMeasure.cpp:304`(Exit 스텝 직전 `SegmentRestore0`), `:335-365`, `:465-472`, `DoWorkGlassMeasureStep.cpp:1958-1985`(Idle `SetEvent`), `DoWorkGlassSequenceEngine.cpp:880-884` | **반영** | (3-2) 마지막 스틱 훅(앵커 `if (GetCurStep() == GetExitStepIndex()) SegmentRestore0();` n=1, 튜플만) — Idle 전에 `TupleStickDone`, 진행 블록은 중복 방지. (1-3), (5-1) 시점, (8) O2 로그 순서 |
| C11 / K5 (low / med) | D5-6 (b)가 N ≥ 3에서 중단 없는 런과 다르고, §5.8-3 확정 문구를 다시 염 | `DoWorkGlassMeasure.cpp:641`(`bTurn`), `:583-590`(세그먼트 `.alg` 로드), `:754-766`(재Align) | **반영** | (3-3) 정상 런의 Align 출처 분석·반례 (1,2,1), D5-6 재작성: (a) 늘 강제 / **(b) 필요할 때만(`rot(k)==rot(0)` + 1..k 회전 경계)** / (c) 강제 안 함, **§ 승인 표시**, 남는 차이(같은 방향 구간 `.alg` 이름 차이) (12)-2. 머리말 예외 문단, (6) P14/P15 `bForce`, (8) O0 표·O24 |
| C12 / K3 일부 (low) | ALL DONE + 저장됨 정리가 다른 검사 뒤라 레시피·파일이 바뀐 저장 완료 글라스가 거부 팝업 | 플랜 (2-2) 순서 | **반영** | (2-2) 1b로 이동(1 바로 뒤). (8) O15 |
| C13 (low) | 강등 경로 미정(식별 필드·`DeleteFiles`), 레거시 플래그 + 튜플 글라스면 RS1 행이 남음 | `DoWorkGlassSequenceEngine.cpp:663-683`(재개 모드면 시작 저장 생략), `DoWorkGlassRecipeStep.cpp:48-151`(RS1) | **반영(1번은 다른 방식)** | `ResumeTupleDowngrade` 삭제 → 팝업 3b 거부 + 측정 진입 FAIL(파일 유지). 강등을 되살릴 경우의 조건(엔진과 같은 `SaveResumeInfo(-1)`)은 (3-2)에 기록. 2번: `bLegacyResumeFlag` → `ResumeTupleBegin(..., bDemoted = true)`, 레거시 하이브리드 거부 술어를 `IsTupleGlass(N)`로 |
| C14 (low) | `LoadResumeInfo` 파일 없음 조기 반환이 새 멤버를 안 지움, ALL DONE이 끝 표지를 안 냄 | `ResumeInfoMgr.cpp:41-57` | **반영** | (1-3) `ResetTupleMembers()`(조기 반환·Clear·Save(-1) 공유), (6-2) 선언. (1-2) ALL DONE 끝 표지 `(PassCount, 0, 0)` |
| C15 (low) / K3 (high-med) | `SaveResult` true는 내보내기 스레드 시작일 뿐 / `SaveResult`는 `bUseMeas`일 때만 → 게이트 0 글라스가 사이클 2번째 글라스 정지 때 거짓 ALL DONE | `CDlgResult.cpp:1731-1752`, `DoWorkGlassResultStep.cpp:67-114`(게이트 0이면 `SaveResult` 없음), `DoWorkGlassUnloadStep.cpp:190-260`(`CheckSaveReuslt`가 이미 `SaveResultDone` 30 s 대기), `DoWorkGlassUnload.cpp:34-38`(스텝 순서), `DoWorkGlassSequenceEngine.cpp:696-726`·`:880-884` | **반영** | `ResultSaved` 2단: Result `CompleteCheck` 끝(`bRet`) = 1(게이트 0이면 바로 2), Unload `CheckSaveReuslt` 성공 = 2(기존 대기 재사용, 타이밍 무변경). 2 = 조용히 정리(1b), 1 = "저장 중 멈춤" 팝업. 새 접점 P18b(`DoWorkGlassUnloadStep.cpp` CP949 CRLF 306), (1-1)·(1-3)·(1-4), (2-3), D5-13, (9)·(10), (8) O15·O21 |
| K7 (med) | 저널 쓰기 실패를 나중에 알 방법이 없음(플랜 (11)의 "줄 수 불일치로 거부"는 근거 없음) | 플랜 (1-1)·(4-2) | **반영** | 끈적 `m_bWriteOK`, `Commit()` bool, `GetLineCount()`, 북마크 `JournalOK`·`JournalLines`(스틱마다·압축 때), (2-2) 5에서 비교·한국어 거부, (4-3), (11) 행, 시뮬 4, (8) O10·O23 |
| K9 (low-med) | Recipe None의 `ClearData()`와 재생 사이에 늦게 온 ANGLE/VISION 행이 그리드·결함 벡터에 남음 | `CommTcpIpObjectAngle.cpp:853`, `CommTcpIpObjectVision.cpp:1541`, `CDlgResult.cpp:871`(`m_vecVisDefecs.push_back`) | **반영** | (4-4) (a) 큐가 빌 때까지 대기 → 두 번째 `ClearData()`(튜플 경로만), 재생 뒤 도착분은 (11)·(7) 기록 |
| K11 (low) | 튜플 거부가 늘 저널을 지움(되돌릴 수 없음) | 플랜 (2-2)·(2-3) | **반영(확장)** | #2·#4·#8에 #9까지 `MB_OKCANCEL`(확인 = 지우고 새로, 취소 = 북마크 유지·START 취소), 구조적 거부는 `MB_OK`. (2-2) 머리말, (2-3) 문구, (8) O11 |
| K12 (low) | 레거시 하이브리드 거부가 다른 N=1 레시피의 북마크에도 "저널 없음" 사유를 보임 | `DlgSubOperate.cpp:3481-3532`(기존 불일치 팝업) | **반영** | (2-1) 코드에 RecipeID 일치 조건, 다르면 기존 코드의 불일치 팝업. (4-6), D5-16 |
| K13 (low) | `PassCount=1`이어도 EFA일 수 있음(항목 1개) → AgingNew 입구 조건 오류 | `MeasureSequenceItemEngine.cpp:896-920`, `DoWorkGlassMeasure.cpp:185-189` | **반영** | 북마크 `EfaMode` 신설, AgingNew 입구 = `EfaMode == 1`, 측정 진입에서 `EfaMode`·`PassCount` 모두 비교, `AgingGlassLatch(bEfaMode ? nPassCount : 0)` 표기. 기록 항목(플래그 복원의 지속)은 C8 반영으로 해당 없음 |
| K15 (low) | 다이얼로그 입구에서 GlassID가 부팅 기본값이라 글라스를 구분 못 함, PreMeas 재접촉은 공정 판단 | `SystemParamShare.cpp:36`, `D_MeasureCycle.cpp:214`, CIM `:1263`, 엔진 `:723` | **반영** | (2-3) "중단 시각" 줄(INI `SaveTime`). D5-19(공정) 신설 — (a) 그대로(권장) / (b) Phase 6 생략. (12)-2 |
| K16 (med) | OOS 보강: O4 기대 로그, O16b, EFA T0 정지 + 재기동, 사이클 2 게이트 0, 재생 중 STOP, 3세그먼트, VISION 타임아웃, 쓰기 실패, `EFA^0$` ACK 대기 중 STOP, 취소 거부, CIM은 코드 리뷰 명시 | 픽스처 `9_OCP_TEST_Hybrid_Half_260930`(seg1 5×3) | **반영** | (8) 머리말 코드 리뷰 목록, O0 표 확장, O2·O4·O5b~f·O8·O10·O11·O13·O15·O16(b~d) 수정, O21~O25 신설, 실기 항목 보강. VISION 타임아웃은 OOS에서 `m_bRunStatus`가 안 서므로 시뮬 3(S/X 번갈아) + 실기, ACK 대기 STOP은 REMOTE 전용이라 코드 리뷰 |
| K17 / C "No finding" | 진입점 4곳, P0-1, 앵커·인코딩, 구 파일 호환, 루프 시작값, EFA T1 경로, 세그먼트 캐시, 저널 완전성, 재생 규칙, 래치, Recipe Change 블록, 자정, ICP 교훈 | — | 확인(변경 없음) | 새 앵커 15개 n 재확인(`p5fix/anc_fix.pl`, 위 (6) 목록), 새 접점 `DoWorkGlassUnloadStep.cpp` CP949 CRLF 306 damaged 0. 리뷰가 "N=1 정상 런 바이트 동일, 7·8번 제외"라고 한 두 곳은 C7·C8 반영으로 해소 |

- **거부한 지적: 없음.** 부분 수정·대안 채택 5건 — C5 쓰기 순서 제안 거부(근거 (5-1)), C3 플래그 위치를 `WaitEFA0Ack` 뒤 대신 `Cmd_Send_EFA0` 직후로 강화, C8은 원복 로직 대신 "플래그를 쓰지 않음"(D5-9 (b)), C9/K10은 되돌림 대신 가드, C13 1번은 강등 경로를 없애고 FAIL로.
- N=1 정상 런 바이트 동일(HARD RULE) 재확인: 새로 들어간 코드는 모두 `bTupleGlass`/`IsTupleBookmark()`/무장 여부로 막히거나 무동작이다 — `Disarm()` 2회(무장 안 됨), 루프 조건 `bExis == false`(초기 false), `EfaPassBoundary(nPass, false)`, 마지막 스틱 훅(튜플만), `SegmentBoundary` 기본 인자(N=1은 호출 없음), Result/Unload `MarkResultSaved`(Format 1이면 무동작), Vision 지역 사본·`else` 블록(세대 0이면 무동작, 토스·`Clear` 순서 그대로), CYCLE 가드(팝업 동안만 섬). 로그·장치 SEND·결과 무변경.

### Phase 6 — 운전 화면 표시 — ✅ 구현·빌드(2026-10-12 17:00, dfcRecipe 포함), N=1 회귀 OOS OK — 남은 OOS: 하이브리드 Half CYCLE·EFA 표시, 재개 진행, MakeCell/레시피 화면 이월 글자, N=1 AGING 켬 T1 `AGING S`
- **OOS 1차 — N=1 회귀(2026-10-12 17:12~17:16, 17:00 빌드 `ALPHA.exe`·`dfcRecipe64d.dll`):** `1_OCP_TEST_Long_260907`(AGING 0·EFA 1·Sampling 0, 수동 Cell ID A/B). 레시피 파일이 11:03 기준 런(09:51 빌드, Phase 6 이전) 때 내용과 바이트 동일(`RECIPEBAK\...\20261012-160049.bak` = 현재 `.rcp`).
  - 정규화 비교(`p6oos/n1cmp_args.pl`·`p6x/p6x_n1cmp.pl`·`p6oos/n1cmp_motion1012.pl`): GLASS 1~8·Sequence_Step·engine·WAD·PG·AGING·화질1/2·시야각1/2·OCP·DLOG·Motion(674줄)·IOControl·Light·Probe_Move_Z **전부 posdiff 0**. 택트 267행 항목명 동일, 전체 288.3 → 257.5 s(전 항목 고르게 5~10% 단축 = 11:03 때 PC 부하, 코드 차이 아님).
  - 차이는 OLOG `[CELLID] Cell map manual names 100 of 100 (panel A, 1 segment, rot=1)` START 때 3줄뿐 — 수동 Cell ID 레시피의 설계 동작(`p6x_fix_changes.md` OOS 5번 P6Z "START 때 3줄"). `Path view`·`[HYBRID]` 0줄. 알람 없음(ALARM 폴더 10:04 이후 무변경).
  - 측정 중 레시피 열기 가드: 런 중 17:12:40 수동 → AgingNew 진입(OLOG), 16:03:28 때와 달리 재로드 흔적(`[CELLID] Cell map` 재출력·`AGING Process Close!!`) 없음. 팝업 자체는 로그를 남기지 않는다 — **사용자가 화면에서 팝업 "측정 중에는 이 작업을 할 수 없습니다." 확인(2026-10-12), 가드 OK.**
- **설계**(2026-10-12, run `wf_f994b2e4-d52`): 매퍼 3 → 플래너 → 리뷰 2(지적 23건, 기각 0) → 수정. 설계서·리뷰는 work 폴더 `p6x_plan.md`·`p6x_plan_reviews.md`.
- **사용자 결정(2026-10-12):** D6-9 승인(D4b-7 (b) — N>1 Auto/Auto DLL 레시피 화면 3곳에 이월 글자), D6-4 (b) 하이브리드 레시피 플래그 EFA까지 AgingNew 하이라이트, D6-11 (a) 셀 이름은 GenGlassPath 때만 갱신, D6-3 (a) N=1 표시기 숨김. 나머지 D6-1/2/5/6/7/8/10/12/13/14 권장안(셀 맵 위 런타임 라벨, 한국어, 경계 완료 시 경로 외곽선 전환·루프 밖 세그먼트 0, 셀 차트 용량 512 + 1세그먼트 그리기·지우기 상한 150, 수동 ID 패널 A, MakeCell APPLY 뒤 ID 등).
- **구현**(run `wf_d01bb09c-960`, 컴파일만): 6-0 `RecipeEditMgr` 화면용 int 4개(`SetUiProgress`/`GetUi*`) · 6-1 `DoWorkGlassMeasure` 게시 4곳 · 6-2 `ResumeJournal` 재생 진행 카운터 · 6-3 `ChartOrbit` 그리기·지우기 상한 · 6-4 `DlgSubOperate` 표시기 라벨·경로 외곽선·수동 Cell ID 이름·용량 512 · 6-5 `DlgSubManual_AgingNew` 제목 접미사·하이라이트 게이트 · 6-6 `GlassMap`/`MakeCellDlg` 오버레이 ID · 6-7 `InRecipeFunc` 이월 글자. 리뷰 2 → 수정 5건(N=1 A Auto+B Manual 무로그, `SetUiProgress` 쓰기 순서, 재생 카운터 읽기 순서, 낡은 주석 2). 전체 ClCompile ALPHA·dfcRecipe 오류 0, 새 경고 0. 파일 16개(ALPHA 9, ChartOrbit 2, dfcRecipe 5). 변경 기록 work 폴더 `p6x/p6x_6?_changes.md`, `p6x_fix_changes.md`(최종 OOS 체크리스트 포함).
- **빌드 주의:** DLL 변경 → sln `//t:dfcRecipe:Rebuild` → 일반 빌드. 15:59 사용자 빌드는 리뷰 수정·측정 중 레시피 열기 가드(아래) 이전 상태라 다시 빌드해야 한다.
- **같은 날 별건 — 측정 중 레시피 열기 가드 1세그먼트 확장(사용자 요청):** AgingNew·AgingTest(열기·저장·다시 읽기·START), AutoContact(열기), MultiMeas(열기·저장)가 1세그먼트 레시피도 측정 중이면 팝업 "측정 중에는 이 작업을 할 수 없습니다."로 거부(work 폴더 `fix1012_changes.md`). 계기: 16:03:28 1세그먼트 EFA 런 중 AgingNew RCP OPEN으로 레시피 재로드(Sampling 적용, 래치·셀 상태 초기화, `[CELLID] ... not validated` WARN).
- 이전 상태 기록: 2026-10-08 12:10 설계 워크플로 중단(매퍼 3개 완료) → 10-12 재개.
- 세그먼트 표시기, ChartOrbit 활성 세그먼트 갱신.
- **운전 화면 셀맵(`IDC_STATIC_CELL_MEASURE`) 전 세그먼트 오버레이 — ✅ 구현·컴파일 완료(2026-09-30, 별도 에이전트), 빌드·OOS 대기.** `DlgSubOperate.cpp/.h`만 수정(스크립트 `scratchpad/p6_patch.pl`, 백업 `p6_bak`, 오프라인 재생 `p6_overlay_sim.pl`).
  - 차트는 회전하지 않는 한 글라스 프레임(x = 1500, y = 925 아래로, 원점 = 글라스 중심). 스틱 글라스 좌표 s = StageY + `m_sMap.m_fContactOffset`(REVIEW Y가 아님), c = CellX. rotate 1: 중심 (−c, −s), 크기 (CellSize.x, .y) / 그 외: (s, −c), (CellSize.y, .x).
  - `GenGlassPath`(레시피 열기·글라스 시작, 세그먼트 0 활성)가 nCellStatus 초기화 뒤 `BuildHybridCellMap()`으로 전 세그먼트 셀을 캐시 — 세그먼트 벡터는 여기서만 읽는다. 1초 타이머 `UpdateCellStatus` → N>1이면 `UpdateHybridCellStatus()`(캐시 + nCellStatus + `GetActiveSegment()` int만). 상태 슬롯 = `nCellStatus[j][stickBase_k + (rot==1 ? i : N−1−i)]` = `ToCellStatusStick` 열. 끝난 세그먼트 색 유지, 사이클 중 활성 세그먼트에 흰 테두리(8 mm 바깥).
  - 1세그먼트: `GetSegmentLoopCount() <= 1`이면 새 경로 진입 없음(기존 GenGlassPath·UpdateCellStatus 그대로, 3b A8 주석만 갱신). 오프라인 재생 결과 rotate 2는 1세그먼트 화면과 0.000 mm 일치. rotate 1은 1세그먼트 화면이 스틱 행을 세그먼트 자기 중심 기준으로 뒤집어 그려(비중심 배치에서 어긋남) 오버레이는 강체 위치를 쓴다 — LS 세그먼트 0은 1세그먼트 화면보다 28 mm 위에 그려짐. 1세그먼트 레시피 자체엔 영향 없음.
  - 남은 것: 차트 CELL_MAX 기본 150(`Bin\Config\DEFAULT\Config.ini` 없음) → LS 픽스처(205셀)는 잘림 + WARN, 상향은 1세그먼트 차트에도 영향이라 보류. `IDC_STATIC_PATH` 경로 차트는 여전히 세그먼트 0 회전. 새 로그 `[HYBRID] Cell map segment k/n ...`, `[HYBRID] Cell map n segments, N cells ...`.

- **MakeCell 셀 오버레이 `IDC_STATIC_CELL_OVERLAY` (2026-09-30 사용자 지시, ICP MakeCellDlg `IDC_STATIC_2` = `C_GlassMap` 이식) — ✅ 구현·dfcRecipe/ALPHA 컴파일 OK, 빌드·화면 확인 대기.**
  - 컨트롤: 사용자가 `IDD_MAKE_CELL_DLG`에 추가한 `SS_BLACKFRAME` static(2009). 새 파일 `dfcRecipe/GlassMap.h/.cpp`(ASCII CRLF, vcxproj·filters 등록). 스크립트 `scratchpad/mcoverlay_patch.pl`·`mcoverlay_fix.pl`, 백업 `mcoverlay_bak`·`mcoverlay_fix_bak`.
  - **그리는 것**: 모든 세그먼트 셀을 한 글라스에 그리고 셀 ID를 표시한다. 보는 세그먼트는 ICP `BLOCK_PROC`, 나머지는 `BLOCK_UNPROC`이다. 글꼴 7 px 미만이면 ID를 생략한다.
  - **좌표·edge cut**: 운전 화면 `IDC_STATIC_CELL_MEASURE`와 같다.
    - 프레임: x = 1500 오른쪽, y = 925 아래쪽.
    - edge cut: `RotateGlassPath`가 고정으로 쓰는 ChartOrbit 코드 4다. 글라스 점 (+750, −462.5), 다리는 각 변의 5%, 화면 우상단.
    - 셀 배치: rotate 1/3(장변 풋프린트)은 중심 (−c, −s), 0/2는 (s, −c).
    - `DlgSubOperate::BuildHybridCellMap`도 같은 풋프린트 규칙으로 맞췄다(리뷰 지적). 상태 열·셀 이름은 MakeCellID의 rotate==1 규칙을 그대로 쓴다.
    - rotate 3의 부호는 270 실기 런으로 확인해야 한다.
  - **캔버스**: ICP 규칙을 따른다. 보는 세그먼트가 장변이면 가로, 단변이면 세로이고, 세로는 (x, y) → (−y, x)로 돌린다. 세로 회전 방향은 화면 확인이 필요하다.
  - **데이터 출처** = 다음 RECIPE APPLY가 쓰는 것.
    - 단일 모드: 그리드. 그리드로 만들 수 없으면 레시피.
    - 하이브리드 OPEN 슬롯: 그리드. 보고 있을 때 만들고, 세그먼트를 넘길 때 떠나는 슬롯도 다시 만든다.
    - 나머지 슬롯: 현재 레시피(`g_inRcpInfo`).
    - 그리드 미리보기는 `BuildRecipeFromGrid`를 `g_inRcpInfo.m_recipeMgr`에 돌리고 힙 사본으로 복원한다(ApplyHybridRecipe와 같은 방식). 스틱·셀 개수는 미리 검사한다. `new double[스틱 수]`에 검사가 없기 때문이다.
  - **갱신 시점**: OPEN, CAD CALC, 세그먼트 이동, 하이브리드 체크, 개수 적용, 창 표시. ICP는 OPEN과 CALC 때만 갱신한다.
  - **리뷰**(워크플로 wf_5f5a82a2-f1d, 리뷰어 4 + 반박 검증): 확정 3건(rotate 3 풋프린트, 떠나는 슬롯 미리보기, 단일 모드 출처)을 모두 수정했다. y 방향 반전은 리뷰 중에 이미 고친 상태였다. 스킵 패치는 제기된 5건이 전부 반박됐다(OCP 동작과 같거나 기존 결함).

### Phase 7 — 수동 화면 세그먼트 선택기 (D-9) — 7a(안전 게이트) 구현·전체 컴파일 오류 0(2026-10-13), 빌드·OOS 대기 / 7b(선택기) 구현 전
- **7a 구현**(run `wf_92f7c141-b34` 7-0 → 끊김 뒤 `wf_df2d36fa-b8a` wave A·컴파일·리뷰·수정, 컴파일만): 파일 11개(+907 −7) — `Manager/MaintControlMgr.h/.cpp`(Glass Align 기준 R 표식 API `SetGlassAlignRefR`/`GetGlassAlignRefR`/`IsGlassAlignOrientationOk`, `SetGlassAlignXYR`·`Do_StageVac_On/Off`에서 해제), `DoWork/DoWorkGlassSWAlignStep.cpp`(Align 성공 시 표식, 하이브리드면 OPER `[HYBRID] glass align mark R …`), `DlgSubManual_Meas.h/.cpp`(3상태 워커 잠금, 워커 진입 디스패처 `CheckHybridGate` = 판정 → 분류 → 로드 중 → R-T → G0-b → G0 → PG 모델 → G → 자세 → R-A → S → R-B → R-C, `:2092` 로드 일련번호·하이브리드 1013 거부, `m_strPgModelSent`, 수동 Cell ID 이월·토큰·31자·검증 래치, 수동 `SetMeasUseInfo` segBase, IsCycleState 거부 — D7-11/12/14 하이브리드만), `DlgSubManual_AutoContact.h/.cpp`(START·워커 R-A/R-C, 하이브리드만 SPS_CYCLETEST + 모든 종료에서 SPS_IDLE 복구 가드 객체, 스레드 시작 실패 복구, 세그먼트 라벨), `DlgSubManual_AgingTest.cpp`/`DlgSubManual_AgingNew.cpp`(START: 기존 `== SPS_PROCESS` + 하이브리드면 `!= SPS_IDLE`, D7-15 b), `DlgSubManual_MultiMeas.cpp`(`:656` 로드 일련번호, 하이브리드만), `Recipe/RecipeEditMgr.cpp`(티칭 MOVE·STAGE_MOVE·STAGE_PROBE_MOVE R-A, 하이브리드 런타임 TestMode PRE_CONTACT/CONTACT 거부). 하이브리드 판정은 6곳 공통 "로드 중이면 캐시, 아니면 DLL 실제 세그먼트 수"(리뷰 CS-1).
  - 전체 ALPHA ClCompile 247개 오류 0, 새 경고 0, 인코딩 VERIFY OK. 리뷰 2(정확성 8건 중 6건 수정·2건 무변경, 완전성은 도구·픽스처·체크리스트로 처리). 변경 기록 `work/hybrid_session_20261006/p7x/p7x_70/71/72/73/75/compile/fix_changes.md`, 리뷰 `p7x_impl_reviews.md`.
  - OOS 픽스처(설치 전): `p7x/oos/HBS/` — `9_OCP_TEST_Hybrid_Half_260930_ALN.rcp`(두 세그먼트 ALIGN USE 켬) + PARAM 사본 3, `_NOALG.rcp`(기본 `.alg` 없음). `perl p7x/oos/install_aln.pl copy`(덮어쓰기 안 함).
  - 1세그먼트: 모든 새 분기가 하이브리드 판정에서 로그 없이 빠짐 → 수동 측정·AutoContact·AGING START·TestMode 동작·로그 바이트 동일이 OOS 판정 기준. 예외 하나 — 레시피 로드가 2초 넘게 계속되는 동안의 수동 단계는 거부(안전 측, 리뷰 CS-5).
- **같은 날 별건 — 수동 특성·화질·시야각 이동 위치 ≠ CycleTest(사용자 보고 2026-10-13, 기존 결함, 7a 무관):** 11:37 수동 `화질 이동`이 FRONT GANTRY 158.852(CycleTest −104.841) → Cross Interlock 3027, 11:38:05 `시야각 이동` FRONT GANTRY −600 → SW Limit 3902. 원인: 수동 화면의 복제 코드(`ManualMoveCell`/`MakeMeasGroup`)가 자동 측정 스텝의 06-26~09-21 변경을 따라가지 않음 — ① 갠트리 기준에 스틱 0 StageY(263.693)를 이중 가산(`DlgSubManual_Meas.cpp` `m_vecStick[0].m_fStageY + m_fHeadReviewY`, 자동은 `m_fHeadReviewY`만) ② 시야각이 HMI 시절 `AXIS_X2` 전달(OCP 자동은 X3) ③ 짝 간격 780 하나(자동 WAD 444/VISION 500/ANGLE 1400) ④ Z·광학축 슬롯 HMI 배치. 수동 버튼은 이날 처음 사용(OLOG 전 기간 검색). RCA·검증 `work/hybrid_session_20261006/manualpos/rca.md`·`verify_numbers.md`·`verify_fix.md`(run `wf_0891ce7b-c96`, 로그 숫자 0.001 재현). **사용자 결정:** 공용 함수 `MeasBuildGroups`/`MeasBuildMovePos`로 자동·수동이 같은 코드 호출(자동 원본 `#if 0` 보존, CycleTest 로그 diff 0 확인), CYCLE과 같은 2헤드 짝, 다음 CELL은 매번 전체 이동, **1세그먼트에도** 스틱 자세 검사 + 스틱 이동·회전 전 Sub Z·FRONT Z 후퇴(1세그먼트 수동 동작 변경 승인). 구현 run `wf_5960d91d-705` 완료(컴파일만): `DoWork/DoWorkGlassMeasureStep.cpp/.h`(CP949) 공용 함수 + 자동 WAD/VISION/ANGLE 6곳 호출(원본 `#if 0`, 삽입분 제거 시 백업과 바이트 동일), `DlgSubManual_Meas.cpp/.h`(UTF-8 BOM) `ManualMoveCellShared`(OCP, 화질은 `USE_VISION_COLOR` 꺼짐일 때만), 1세그먼트 자세 거부 WARN `[MANUAL] Manual Meas … refused - stage not at stick n pose`, 스틱 이동 전 FRONT Z→SafetyZ2·Sub Z→INTERLOCKSUBZSAFETY(내려가 있을 때만, SYST 1줄), InnerID는 스틱·레시피 바뀔 때만 리셋(리뷰 #7), 레세피 선택 취소 시 그룹 유지(#9). 전체 ClCompile 246개 오류 0, 새 경고 0(이동된 C4267 6개 = 원본 줄 그대로). 변경 기록·OOS 체크리스트 `manualpos/mp_auto/manual/fix_changes.md`, 리뷰 `mp_reviews.md`. 사용자 결정: #8 하이브리드 스틱 이동 전 후퇴 그대로 둠, #6 ALIGN·GLASS GET/PUT(로더 이동)·PROBE ALIGN·PROBE MOVE 시작 전 같은 후퇴 자동 실행(`RetractHeadZBeforeStep`, OCP만, 내려가 있는 축만, `mp_patch_r6.pl`, ClCompile 오류 0·새 경고 0). 자동 무변경 OOS 기준 = 2026-10-13 11:43:24~11:50:36 CycleTest(11:32 exe).
- **2026-10-14 수정(컴파일만, 빌드·OOS 대기):** ① 수동 화질 다음 CELL에서 BACK GANTRY 표시가 470↔570으로 오가던 것 — `MoveCmdPosVision(..., bHoldSafetyX2)` 새 인자(수동만 true): 쉬는 X2에 헤드 오프셋 0, 이미 Safety면 재지령 생략(자동·TestMode 무변경, `manualpos/mp_patch_1014.pl`) ② 위치 거부 팝업 좌표를 "현재 위치 / 목표 위치" 두 줄로 ③ **Short 레시피 3721(특성 Sub Z DOWN NG) = OOS 전용 좌표 오판**: `CMaintControlMgr::GetOrgPos`가 OOS에서 헤드 오프셋(−63)을 빼 X2 −37.445를 25.555로 판정 → PROBE UNIT 판정 4곳(WAD·Review Sub Z, Aging Z1, WAD 후퇴 판정)과 WARN 로그를 raw X2·X9로(사용자 결정 A안, `shortilk/sk_patch_a.pl`, 실기 동작 무변경). 09-29 P1RT seg2도 같은 원인이었음. 실기 전 HW 확인은 §9-19. 시험 레시피 `_ALN`·`_NOALG` 설치(10-14 10:26).
- **2026-10-14 수정 2(컴파일만, 빌드·OOS 대기):** ① **하이브리드 START 시 ALPHA 종료 = 레시피 이름 길이**: 33자 `9_OCP_TEST_Hybrid_Half_260930_ALN`이 MCC 공유메모리 `TCHAR szRecipeID[32]`로 `_tcscpy_s` → Debug CRT "Buffer is too small" assert(시험 레시피 이름을 `_ALN`/`_NOALG`로 줄임). 수정(사용자 결정 "잘라 쓰기 + 시작 거부"): MCC 복사 12곳·결과 경로 Recipe/Glass/Lot ID 복사·Resume 재적재를 `_tcsncpy_s(.., _TRUNCATE)`, CYCLE START에서 31자 초과 레시피·28자 초과 Glass ID 거부 팝업(입력 칸 28자 제한), `RecipeChange` 맨 앞 31자 초과 거부(-1), 세그먼트 WAD 모델명 32자+·PG 모델명 100자+는 송신 직전 거부(잘라 쓰지 않음 → `JOB_RECIPE_OPEN_FAIL_ERROR`), **CIM: PPID 31자 초과 → PPID NG(알람 7003), GLSID 28자 초과 → GLSID NG**(호스트가 보는 응답 변경, 종전에는 이후 크래시하던 경우만 — **사용자 결정 2026-10-14: 유지**). 31자 이하 레시피·28자 이하 Glass ID는 동작·로그 무변경. 기록 `work/hybrid_session_20261006/hybcrash/rl_impl_changes.md`, `rl_review.md`, `rl_fix_changes.md`. 남은 것: dfcRecipe 저장 단계 길이 검사 없음, 2번째 세그먼트의 긴 WAD/PG 이름은 경계에서야 거부, 수동 화면 거부 문구는 "Load 실패"(원인은 WARN 로그). ② **운전 화면 하이브리드 라벨이 셀 차트 "Glass Size" 글자를 가림**: 차트가 `TextOut(5, 20)` 글자를 y 20 근처 기준선으로 그려(실측 y 8..23) Phase 6 라벨(y 1..19, 전체 폭)과 겹침 → 라벨을 Glass Size 글자 오른쪽(라벨 글꼴로 잰 최악 샘플 폭 + 여백, 차트 x ≈ 246)·y 1..27로 옮기고, 한 줄에 안 들어가면 세그먼트 뒤에서 두 줄로(`DlgSubOperate.cpp`, `aln1014/ui_patch_1014.pl`, 기록 `aln1014/ui_changes.md`). 1세그먼트는 라벨이 없어 무변경. ③ **NOALG CYCLE(16:15) = 하이브리드 `.alg` 실패가 CYCLE START를 통과**: `.rcp`는 읽히고 기본 `.alg`(`HMI_NOALG_X`)만 없어 `RecipeChange`가 1013 → `OpenRecipe`가 전역 RecipeID를 이전 값으로 둔 채 CYCLE 창이 `0 < 1013`으로 시작 → Recipe Change 단계가 전역 RecipeID(재시작 직후 `UNKNOWN` → 1013 거부, **이전 레시피가 남아 있으면 그 레시피로 측정**)를 다시 연다. 수정(사용자 결정 "하이브리드만 거부"): `D_MeasureCycle.cpp`에서 `nOpenRtn == JOB_RECIPE_NOT_EXIST && GetSegmentLoopCount() > 1`이면 수동 측정 (14)-K와 같은 팝업 + WARN `[HYBRID] CYCLE START refused`로 거부. 1세그먼트는 원래 동작(이전 레시피로 측정할 수 있는 원본 결함은 그대로 — 위험으로 기록). 기록 `aln1014/noalg_changes.md`. **ALN 하이브리드 CYCLE OOS 1차 OK**(15:22·15:35, exe 15:17): 알람·인터락·Motion WARN 0, EFA seg1p1→seg2p1→seg1p2→seg2p2·skip 마스크 일치, 경계 재Align 3회 + `glass align mark R` 90/180, Phase 6 Path view 8줄, Short 세그먼트 Sub Z 하강 24건 OK, 결과 T0 35행·T1 16행 중복 0, MCC RecipeID 26자 그대로. 16:32 빌드에서 NOALG START 거부·32자 레시피 START/수동 열기 거부 확인(16:34~16:35). 보고서 `aln1014/report.md`. ④ **새 팝업 글자 잘림**: 수동 측정·하이브리드 게이트·TestMode 거부 팝업이 쓰는 `CD_DisplayMessage`(CLabel `SS_CENTERIMAGE`)는 CRLF 없는 문구를 `DT_SINGLELINE`으로 그려 "\n" 줄이 한 줄로 이어져 잘림 → 줄바꿈 있는 문구만 CRLF·자동 줄바꿈·높이 맞춤(작업 영역 85% 넘으면 글자 축소), 한 줄 문구는 무변경(`D_DisplayMessage.cpp`, 기록 `aln1014/popup_changes.md`). `AfxMessageBox` 팝업은 Windows가 감싸 주므로 해당 없음. 1세그먼트 회귀: Long 16:51 diff 0(10-13 16:58 대비), Short 17:04는 레시피 `_HEAD_OFFSET_` 수정분(갠트리 X −35.11)만 다름, 17:03 Short START는 그 전 판(헤드 Y ±29.7)이 PRECHECK(Y 간격 686 < 780)에 정상 거부. ⑤ **16:32 빌드 판독(16:34~16:50, `aln1014/b_report.md`)**: NOALG START 거부·길이 가드·수동 화질 X2 재지령 0·헤드 Z 후퇴·Short raw X2 확인, 회귀 없음. 새로 찾은 것: (중) 하이브리드 글라스 끝 GLASS 7_RESULT 약 5 s UI 정지(ALN 3회 중 2회, 글라스 종료 Path view 복원과 결과 내보내기가 겹칠 때) → 사용자 결정 "원인 확인·수정"(`c3freeze/`). **보류(사용자 미선택, 10-14)**: ① 수동 화면 32자 레시피 거부 팝업이 "파일을 읽을 수 없습니다"로 나옴(길이 사유 미표시, WARN에는 있음) ② 수동 PIN TO PIN / OPEN-SHORT / R-TEST가 OCP에서도 EVEN 송신(`DlgSubManual_Meas.cpp` :2676/:3204/:3243/:3281/:3321, 09-07 ODD 전용 게이트 미적용, PTP는 5 s 추가 대기) ③ 수동 시야각·Color 이동은 아직 X2에 헤드 오프셋(`MaintControlMgr.cpp` Angle :3252, Color :2976 — 모션은 470 그대로, 표시값만 오갈 수 있음). ⑥ **C3 5 s 정지 원인 = CLogWrapper 잠금 교착**(2026-10-15, `c3freeze/design.md`·`c3_review.md`): `CLogWrapper::KMLOG`는 전역 잠금을 쥔 채 UI 창에 `SendMessageTimeout(SMTO_NOTIMEOUTIFNOTHUNG)`로 로그를 보낸다(LogWrapper.cpp:71, :181-199). 결과 단계 스레드가 `MarkResultSaved`(하이브리드만 로그) 로그로 잠금을 쥐고 UI를 기다리는 동안, UI 스레드는 글라스 종료 Path view 복원(약 0.3 s, 메시지 처리 없음) 끝의 `KMLOG(OPER)`에서 같은 잠금을 기다림 → Windows가 UI를 응답 없음으로 판정하는 5 s 뒤에야 풀림. 수정(`DlgSubOperate.cpp`, 하이브리드 전용): 글라스 종료 세그먼트 0 복원을 엔진 idle(`IsEngineCompleteAll`, Unload 완료) 뒤로 미룸, Path view 로그는 1회용 워커 스레드가 200 ms 뒤 기록(UI 스레드는 이 잠금을 기다리지 않음), 로그 끝에 `draw=N ms`. 남은 같은 종류 위험: UI 스레드의 다른 KMLOG(`OnCcdLiveMode`, `BuildHybridCellMap` 등) — 근본 해결은 LogWrapper가 잠금 밖에서 UI로 보내는 것(전 레시피 영향, 별도 결정). ⑦ **C3 수정 OOS 확인(2026-10-15 10:24~11:09, exe 10:22, ALN 5회 연속)**: GLASS 7_RESULT 0.08~0.52 s(수정 전 5.2/6.3 s), 글라스 종료 Path view 복원은 Unload 끝난 뒤 +0.6~1.7 s(결과 단계 중 0건), CCD LIVE START는 Unload 뒤 3~62 ms(역할 바뀐 교착 재발 없음), 시퀀스 중 4 s 이상 로그 공백 0, draw 47~437 ms. EFA 25/25/10/25 + skip 15 정확, 알람·인터락·DOWN NG 0. 글라스마다 어제 런과 비교 시 차이는 자동 Glass ID(2장째부터 `GLASSID<시각>`, 13자 → Cell ID 16/31자)와 START 전용 줄뿐. 측정 시간이 438→634 s로 늘었으나 5장째는 같은 PC의 다른 작업(PowerPoint 내보내기) 시간과 겹쳐 MCCLog(별도 프로세스)도 2~7.6 s 공백 16회 → 환경 요인.
- 설계·리뷰·결정 완료(2026-10-13)
- **설계**(run `wf_2c2cfd3b-599`): 매퍼 4(ICP 원본·수동 측정·나머지 수동/티칭 화면·런타임) → 플래너 → 리뷰 2(correctness/safety, completeness — 지적 36건, 기각 6건) → 수정. 설계서 `work/hybrid_session_20261006/p7x/p7x_plan.md`(본문 + (14) 리뷰 반영 + (15) 사용자 결정), 리뷰 원문 `p7x_plan_reviews.md`, 앵커 게이트 `p7x/plan/plan_anchors_g0.pl`.
- **모델:** 수동 측정 화면(`DlgSubManual_Meas`, UTF-8 BOM)에 하이브리드일 때만 런타임 콤보(활성 세그먼트 전환, UI 스레드, 유휴 게이트 뒤)와 [세그먼트 준비] 워커 스텝(PG Turn Off → Z5 상승 → PG 모델 송신 → Safety → Align 해제 → 세그먼트 `.alg` 재읽기 → Stage R → Glass Align). 화면을 떠나면 데이터만 세그먼트 0 복귀 + 엔진 시작 백스톱. ICP `theManualRecipe`(제3 인스턴스)는 HMI 구조상 이식하지 않는다.
- **모션 안전:** `CMaintControlMgr`에 "Glass Align 기준 R" 표식(Align 성공 순간 기록, `SetGlassAlignXYR`·수동 진공 On/Off에서 해제). 하이브리드 수동 측정·AutoContact·ALPHA 티칭 이동에 R-A(목표 스틱 R vs 표식 2°)·R-B(컨택·측정 전 스테이지 자세 X 0.1 mm / R 0.02°)·R-C(ALIGN USE면 같은 방향 Align 필수)·R-T(TSP/LCR/Probe 이동 거부)·G0(재로드 재동기, 우회 재로드는 거부)·PG 모델 게이트. 지금도 있는 위험(세그먼트 k STOP 뒤 STICK MOVE가 세그먼트 0 각도로 돌아 틀린 Align으로 컨택)을 닫는다.
- **사용자 결정(2026-10-13):** D7-1 (a) 선택기 + 게이트, 7a(게이트) 빌드·OOS 뒤 7b(선택기). 안전 게이트 D7-3 (a)·D7-5 (a)·D7-20 (a)·D7-21 (a) 전부 적용(하이브리드 전용). **1세그먼트 동작이 바뀌는 D7-11/12/14/15는 전부 하이브리드만((b)) — Phase 7은 1세그먼트에서 바이트 동일.** 나머지 권장안.
- **1세그먼트 적용 보류 항목(사용자 요청으로 따로 기록, 나중에 결정):**
  1. D7-11 수동 측정 `DlgSubManual_Meas.cpp:2092` `if (0 < OpenRecipe())`·MultiMeas `:656` `<= 0`이 `.rcp`를 못 읽은 1013을 성공으로 받음(D4b-15와 같은 결함) → 로드 일련번호 판정.
  2. D7-12 수동 측정의 수동 Cell ID 해석·측정 전 검증(D4b-9). 지금은 "수동 측정 = Auto ID" 운용 공지. 1세그먼트 Manual 설정 레시피(`1_OCP_TEST_Long_260907` A/B Manual)의 수동 측정 ID가 바뀌는 항목.
  3. D7-14 다중 사이클 사이 약 2초 공백(`IsCycleState`)에 수동 측정 워커 진입 거부.
  4. D7-15 수동 워커·DLL 명령(SPS_CYCLETEST) 중 AgingTest/AgingNew START 거부(지금은 `== SPS_PROCESS`만 거부, CYCLE은 이미 `!= SPS_IDLE`).
- **같은 날 별건 — EFA 패스 경계 STOP 엔진 고착 수정(사용자 결정, 2026-10-13):** `DoWorkGlassMeasure.cpp` EFA 경계 실패 블록이 `bExis = true; break;`만 해 측정 아이템 Exit(Idle)이 돌지 않음 → 완료 이벤트 없음 → 엔진 `WaitForSingleObject(..., INFINITE)` 고착, `IsEngineCompleteAll()` false 고착(1세그먼트 EFA 포함, 2026-09-28 EFA 이식 때부터). `EfaPassBoundary` false = T0 저장·FTP·`EFA^0$` 대기 중 STOP. 세그먼트 경계 실패 경로와 같은 `SetFail` 가드 → `SegmentRestore0` → `GoToExitStep` → Exit `Run` 10줄 추가(ASCII, CRLF 1183 → 1193), ClCompile 오류 0·새 경고 0. 백업·스크립트 `work/hybrid_session_20261006/efastop1013/`. **빌드·OOS 대기**: EFA 런에서 `EFA: pass boundary before pass 2` 직후 STOP → 엔진 복귀, 수동 화면 명령 정상.
- **그 밖의 별건(설계 중 발견, 기록만):** DLL TestMode `ShowWindow`가 가드 없이 `LoadRcpFile`(운전 중 열면 측정 스레드 밑에서 활성 세그먼트 0으로), 흡착 유지 재시작 시 Glass Align이 R부터 돌려 실기에서 R 인터락 거부(OOS 재현 불가), `MotorControlMgr.cpp:599` `nRet == df_MOTION_RV_SUCCESSED;` 비교문(OOS에서 X < INTERLOCKPOS R 회전 거부가 꺼짐 — 의도 확인 필요). 상세 `p7x_plan.md` (13).
- **구현 단계:** 7-0(표식) → wave A(7-1 수동 측정, 7-2 AutoContact, 7-3 START 조건·MultiMeas, 7-5 티칭 — 파일 겹침 없음) → 7-6a 빌드·OOS → 7-4(선택기) → 7-6b 빌드·OOS → 7-7 문서.

### Phase 8 — 문서·스킬
- `07_Recipe.md`/`03_AutoSequence.md`/`Resume_Inspection_Plan.md`/HMI CLAUDE.md 갱신(§1.2 불일치 포함), 메모리.

---

## 7. 이식 제외 / 보류 (근거)

| 항목 | 근거 |
|---|---|
| 스펙 체크 + RemeasureDlg 세그먼트별 재측정 | **보류**(D-6, 2026-09-29) — HMI에 기능 자체가 없음. 필요 시 별도 기능 이식(Resume은 D-6으로 이식) |
| 물리 셀맵(`C_PhysicalCellLayout`/`C_CellMapGrid`) | 프레임 모순(I10) 미해결 — Cell ID 편집기는 세그먼트별 그리드로 시작 |
| CopySegmentOptionsDlg 12범주 | 1차는 "원본 세그먼트 전체 복사/기본값" 2택으로 단순화(D-11) |
| Detach/Attach 세그먼트 전환 | D-1 안 A 채택 시 불필요 |
| 멀티패턴 Aging(V1005), Inner ID attempt, Cycle Test 정책 분리, 강제배출 | HMI는 AgingCycle/EFA·MakeUniqueInnerID 등 별도 체계 |
| 합성 GlassMap(방향 혼합 렌더) | ChartOrbit 체계 다름, 프레임 규칙(D-10) 확정 후 |

---

## 8. 결정 필요

| # | 항목 | 선택지 | 권장 |
|---|---|---|---|
| D-1 | 방향 전환 메커니즘 | A: Stage R 회전+재Align / B: 분리→회전→재흡착 | ✅ **확정 A** — 세그먼트 종료 → Stage Safety 이동 → 회전 → 재Align → 다음 세그먼트 (2026-09-28) |
| D-2 | 컨테이너 | 단일 파일(꼬리 블록) / 사이드카 INI(세그먼트=기존 .rcp 참조) | ✅ **확정: 단일 파일**(꼬리 블록) (2026-09-28) |
| D-3 | 세그먼트별 외부 레시피 장치 | PG / WAD / Vision / Angle / Color | ✅ **확정: PG + WAD** 먼저 (2026-09-28) |
| D-4 | EFA·AgingTest 조합 | 1차 금지 / 동시 지원 | ✅ **확정 변경: 동시 지원** (2026-09-29, ICP Aging×Hybrid 호환과 동일 수준) — 설계 §5.9 |
| D-13 | EFA T1의 세그먼트 순서 | 정순(T0와 같은 seg0→N-1, T1 시작 시 복귀 회전·재Align 1회 추가) / 역순(seg N-1→0, 복귀 회전 없음) | ✅ **확정: 정순** (2026-09-29) — 셀마다 T0→T1 경과 시간이 고르게 유지됨 |
| D-14 | AgingTest·EFA 사용 플래그 단위 | 글라스 전역(세그먼트 0 값 래치, 편집기가 전 세그먼트 동일 강제) / 세그먼트별 | ✅ **확정: 글라스 전역** (2026-09-29) — 세그먼트별 제외는 마스크(세그먼트 섹션)로 |
| D-5 | Cell ID | 자동 글자 이월만 / A/B Manual 편집기까지 | ✅ **확정: A/B 수동 입력·Manual 편집기까지** (2026-09-29) — 자동 글자 이월(Phase 4) 먼저, 편집기(Phase 4b) 다음 |
| D-6 | 스펙 체크·재측정 / Resume | — | ✅ **확정: Resume 필수 대응** (2026-09-29) — 세그먼트·AgingTest·EFA 전부, §5.8. ICP RemeasureDlg(스펙 판정 후 재측정)는 **보류** (2026-09-29 확정) |
| D-15 | Cell ID 측정 시작 재검증 실패 처리 | 로그 + 자동 시작 거부 / 알람 번호 신설 | ✅ **확정: 로그 + 자동 시작 거부** (2026-09-29, ICP와 동일하게 알람 없음) |
| D-7 | 세그먼트 간 Probe 카드 | 동일 카드 전제(다르면 저장 거부) / 중간 교체 | **동일 전제** — `_PROBE_ID_` 불일치 시 저장 거부 — ✅ 확정(권장안, 2026-09-29) |
| D-8 | 선결 결함(P0-1/2/4) | 포함 / 별건 | **포함, 별도 커밋** — ✅ 확정(권장안, 2026-09-29) |
| D-9 | 수동 화면 세그먼트화 | Phase 7 / 제외 | **Phase 7** (그전엔 세그먼트 0만) — ✅ 확정(권장안, 2026-09-29) |
| D-10 | 좌표 프레임·합성 표시 | 세그먼트별 장착 프레임 저장(MakeCell 생성 그대로), 표시는 활성 세그먼트만 | **권장안대로** — ✅ 확정(권장안, 2026-09-29) |
| D-11 | 새 세그먼트 옵션 초기값 | 원본 세그먼트 전체 복사 / 기본값 / 12범주 선택 | **2택(복사/기본값)** — ✅ 확정(권장안, 2026-09-29) |
| D-12 | 구 빌드가 하이브리드 파일을 여는 경우 | 무경고(세그먼트 0만) / 세그먼트 0 모델명에 표식 | 두 PC 동시 배포로 회피 + 로그 — ✅ 확정(권장안, 2026-09-29) |

---

## 9. 검증 항목 (구현 전·중)

1. D-1 실기: Stage Safety 위치에서 글라스 흡착 상태 R 90→180 회전 시 흡착 유지, 회전 중 헤드·프로브 Z 안전.
2. PG에 글라스 측정 도중 S2F1(모델 변경) 허용 여부, WAD 레시피 중간 변경 허용 여부.
3. WAD가 `stGlassInfo` 셀/스틱 수를 글라스 시작에만 쓰는지(세그먼트마다 바꿔도 되는지).
4. APD(dResult) 헤더의 셀/스틱 수·`AREA_MAX` 류 필드 유무와 호스트 파서 제약(ICP `AREA_MAX_CNT=2000` 대응).
5. `m_vecStick` 요소 포인터·반복자를 세그먼트 경계 너머 보관하는 소비처 grep.
6. `nAreaNum`/`m_nAreaNum`을 `SetMeasUseInfo` 외에서 쓰는 곳 전수.
7. Vision 비동기 결과가 세그먼트 경계 뒤 도착 시 행 번호(요청 시점 스냅샷인지).
8. 수동 화면 6곳이 `g_outRcpInfo`를 덮어쓴 뒤 자동 운전 시작 시 Recipe 아이템 재로드로 복구되는지(현행과 동일해야 함).
9. P0-3(타임아웃 9토큰) 실제 예외 여부 — 현장 V1005 파일로 확인.
10. 회전 목표각: 세그먼트 첫 스틱 `m_fStageR + m_fStageRotate`와 그 세그먼트 .alg의 Glass Align `StageRotate`(`DoWorkGlassSWAlignStep.cpp:88`)가 같은 방향인지 — 다르면 Align이 다시 돌려 측정 R과 어긋남. Long/Short 현장 레시피 두 쌍으로 비교.
11. Stage Safety 이동에 쓸 기존 함수 확정(Adsorption의 Stage X Safety 이동 vs `MoveCmd_SafetyAll`) — 헤드 Y `[SAFETY]` 치환(`ApplyIdleHeadSafetyY`) 포함 여부.
12. 세그먼트별 PG 모델이 다를 때 AgingTest/EFA `PTN_RGB` 번호가 같은 패턴을 가리키는지(PG 모델 기준 상대 번호).
13. `SetActiveSegment`가 `S_RecipeMgr::m_nStickCnt/m_nCellCnt`까지 교체하는지 — OST/RTEST/AGING 스텝의 `m_nAgingStartStick` 슬롯 계산이 이 값을 씀.
14. `G × 최대 block`이 `AREA_MAX_CNT`(2500)를 넘는 조합(AgingTest 반복 × MultiPoint × 세그먼트 합산 셀) — 저장 시 거부 기준.
15. EFA T1 시작 복귀 경계(N-1→0)에서 PtrnSearch skip 상태로도 재Align 후 측정 위치가 맞는지.
16. 수동 Cell ID 형식이 MES/호스트 규약(`Result_Identification_MES_Agreement_Plan`, `gf_MakeMcrID` 체크섬, `m_szCellID[..][32]` 길이)과 맞는지 — 멀티 글자 접두(AA01) 포함.
17. 결과 저널 기록 지점: `Updata_Data`가 Vision 비동기 결과도 같은 경로로 받는지(✔ `CommTcpIpObjectVision.cpp:1516`), 수동 측정(`DlgSubManual_Meas`, 주석 처리)은 저널 대상 제외.
18. 저널 재생 시 dResult가 같은 행에 두 번 쓰기(EFA T0→T1)를 마지막 값으로 처리하는지.
19. **(2026-10-14 추가, 실기 전 필수)** Sub Z(Z3/Z4) 몸체의 X 위치(카메라 기준 0 / WAD 센서 쪽 +63), P5 유닛 X 범위·높이, X2↔X9 원점 관계 — PROBE UNIT 겹침 판정(`IsProbeUnitOverlapX`, 대역 [X9+1, X9+299])이 raw X2 기준으로 맞는지. +63이면 실기가 WAD 자세에서 63 mm 덜 보호하므로 Short 계열(HeadReviewY 3~66) 실기 가동 전에 확인하고 고정 상수 보정(E안) 검토. 근거 `work/hybrid_session_20261006/shortilk/rca.md`.
20. **(2026-10-14 추가, 실기 전 필수)** seg2 `HMI_180.alg`(09-07, `HMI_90.alg` 템플릿에서 R만 바꾼 미티칭 값)는 Glass Align에서 BACK 헤드를 Y1(RIGHT) −400 / Y2(LEFT) +400으로 지령한다. 평소 Y1 > Y2인 두 헤드가 서로 지나쳐야 하는 위치(실기 충돌 경로)인데, Y1/Y2 인터락(`MotorControlMgr.cpp:790-836`, `:2492-2545`)과 PRECHECK(`DoWorkGlassRecipeStep.cpp:253-259`)는 `fabs(Y1 − Y2) ≥ HEADGAPB`(800 ≥ 780)만 봐서 통과한다. 실기 ALIGN 하이브리드 전에 재티칭(헤드 Y 순서·Sub Z·패턴), 부호 있는 간격 검사·PRECHECK의 `.alg` 위치 포함은 검토 과제. 근거 `work/hybrid_session_20261006/aln1014/report.md` C2.
21. **(2026-10-14 추가)** 자동 PreMeas 첫 이동은 Sub Z를 Align 높이(285/345)에 둔 채 Stage X·헤드 Y를 움직인다(기존 동작, 09-21 1세그먼트도 같음). 실기 간섭 확인, 수동 화면처럼 먼저 올릴지 결정 필요(report.md C4).

---

## 10. 수정 파일 (예정) · 인코딩 주의

| 파일 | Phase | 인코딩/EOL |
|---|---|---|
| `CoreFunction/dll_Recipe/dfcRecipe/OutRecipeFunc.h/.cpp`, `InRecipeFunc.h/.cpp` | 1 | CP949 CRLF(패치 전 확인) — Perl |
| `Source/Include/OutRecipeFunc.h` (사본) | 1 | 동일 — **DLL 사본과 diff 0** |
| `SegmentDefine.h` (신규, DLL·Include 2사본) | 1 | ASCII 신규 |
| `preDefine.h` 2사본 (`RECIPE_FILE_V1011`만) | 1 | CP949 — diff 0 확인 |
| DLL `RecipeMgrDlg.cpp/.h`, `MakeCellDlg.cpp/.h`, `SelectCellDlg.cpp` | 2 | CP949 — Perl |
| DLL `dfcRecipe.rc`, `resource.h` | 2 | **UTF-16 LE** — python |
| `Source/ALPHA/Recipe/RecipeEditMgr.cpp/.h` | 1·3 | CP949 |
| `Source/ALPHA/DoWork/DoWorkGlassMeasure.cpp/.h`, `MeasureSequenceItemEngine.cpp/.h` | 3 | CP949 |
| `Source/ALPHA/DoWork/DoWorkGlassSequenceEngine.cpp`, `DoWorkGlassRecipeStep.cpp`, `DoWorkGlassPreMeasStep.cpp` | 0·3·5 | CP949 |
| `Source/ALPHA/DoWork/DoWorkGlassMeasureStep.cpp` (MakeCellID/GenCellID) | 4 | CP949 |
| `Source/ALPHA/Manager/CommTcpIpFactoryMgr.cpp` | 4 | CP949 |
| `Source/ALPHA/CommTcpIpObjectPG.cpp`, `CommTcpIpObjectWAD.cpp` | 3 | CP949 |
| `Source/ALPHA/Manager/ResumeInfoMgr.h/.cpp` | 5 | **LF-only** |
| `Source/ALPHA/DoWork/AgingCycleInfo.h/.cpp` (마스크 세그먼트 섹션) | 3b | **LF-only** |
| `Source/ALPHA/DoWork/DoWorkGlassMeasureStep.cpp` (행 산식 5곳, 마스크 게이트, `nCellStatus` 인덱스) | 3b | CP949 |
| `Source/ALPHA/CommTcpIpObjectPG.cpp` (`nCellStatus` 인덱스 6곳) | 3b | CP949 |
| `Source/ALPHA/DlgSubManual_AgingTest.cpp/.h` | 3b | **UTF-8 BOM, CRLF**(2026-09-29 확인, 한글은 UTF-8 바이트) — 종전 "BOM 없는 UTF-8"은 stale |
| `Source/ALPHA/DlgSubManual_AgingNew.h/.cpp` (세그먼트 선택기) | 3b·3c | **CP949, LF-only, BOM 없음**(종전 "ASCII"는 틀림) — LF 정규화 후 `\n` 패치, 한글 팝업은 `Encode::encode("cp949")` |
| `Source/ALPHA/DoWork/DoWorkSequenceCIMStep.cpp` (`EFA^1$` 래치) | 3c | CP949 CRLF |
| `Source/ALPHA/Recipe/RecipeEditMgr.h/.cpp` (세그먼트 캐시·`GetSegLogTag`·EFA 래치 저장소) | 3c | CP949 CRLF — `.h` 변경은 넓게 재컴파일 |
| `Source/ALPHA/DoWork/DoWorkGlassMeasure.cpp`, `DoWorkGlassMeasureStep.cpp`, `MeasureSequenceItemEngine.h/.cpp` (태그·스틱 헤더·tact 이름·래치 게시) | 3c | CP949 CRLF |
| `Source/ALPHA/DoWork/DoWorkGlassRecipeStep.cpp` (`CheckHybridCapacity` EFA K, `CheckHybridAgingMask`) | 3c | CP949 CRLF |
| `Source/ALPHA/ALPHA.rc` (AgingNew 콤보·라벨) | 3c·3d | **UTF-16 LE BOM, CRLF** — 이 PC는 python이 깨져 Perl `Encode` UTF-16LE(`FB_CROAK`, BOM 유지)로 패치 |
| `Source/ALPHA/resource.h` (`IDC_COMBO_EFA_SEG` 21008, `IDC_STATIC_EFA_SEG` 21009) | 3c·3d | UTF-8 BOM, CRLF — ASCII raw 삽입 |
| DLL `CellIdEditDlg.h/.cpp`·`CellIdRule.h/.cpp` (신규) + RecipeMgrDlg "CELL ID" 버튼 + `dfcRecipe.rc`/`resource.h` | 4b | `CellIdEditDlg.cpp` **CP949**(한글 팝업, D4b-10), 나머지 신규 ASCII CRLF / rc·resource.h **UTF-16LE** |
| `Source/ALPHA/Manager/ResultMgr.cpp` (결과 저널) | 5 | CP949 |
| `Source/ALPHA/DlgSubOperate.cpp` (`CheckAndApplyResumeMode` 튜플·문구) | 5 | CP949 |
| `Source/ALPHA/DoWork/DoWorkGlassRecipeStep.cpp` (저널 재생·fallback) | 5 | CP949 |
| `Source/ALPHA/DlgSubOperate.cpp` | 6 | CP949 |
| `docs/*.md`, CLAUDE.md | 8 | UTF-8 |

---

## 11. 리스크

| 리스크 | 대응 |
|---|---|
| DLL 원시 포인터 ABI — 헤더 사본 불일치 시 컴파일 에러 없이 오동작 | 구조체 무변경 원칙, 클래스 끝 추가, 두 사본 diff 0 확인, 전체 재빌드(`dfcRecipe:Rebuild` 후 ALPHA) |
| 세그먼트 전환 중 UI 스레드가 `m_vecStick` 읽기(경합) | 전환은 경계 훅에서만, 전환 후 UI 갱신 메시지 — 현행 `RecipeChange`와 같은 수준 |
| Last-Stick 가드 누락 시 RESULT/UNLOAD 동시 진행(3032) | 가드를 `seg==nSeg-1`까지 확장, OOS에서 완료 이벤트 1회 로그 확인 |
| 구 빌드 저장 시 꼬리 블록 소실(두 PC) | 두 PC 동시 배포, 구 빌드 저장 금지 공지 |
| Adsorption이 `m_sAlign`을 PROBE_ALIGN으로 덮어씀 | 세그먼트 .alg는 각 `S_RecipeMgr`에 보관, 경계 훅이 `SetActiveSegment`로 복원 — Recipe 아이템 재로드 순서 유지 |
| 대용량 구조체 복사(`S_Align` 등 세그먼트당 ≈100KB) | 전환은 세그먼트당 2회 — 무시 가능 |
| AgingTest 반복·MultiPoint·EFA 행 산식 충돌 | 블록 크기를 글라스 전체 `G`로 일반화 — N=1에서 기존과 동일(§5.7) |
| `nCellStatus` 세그먼트 간 덮어쓰기 → EFA T1 잘못된 skip | `segStickBase` 오프셋, 총 스틱 ≤ 50 저장 검사(§5.7) |
| 수동 Aging 다이얼로그 저장이 다른 세그먼트 마스크 파괴 | 세그먼트 섹션 저장 + 선택기(A13) |
| EFA T1 복귀 회전 추가로 사이클 타임 증가 | 방향이 같으면 생략, 다를 때만 1회(D-13) |
| Resume 결과 재적재가 파일 순서 추정에 의존 → 반복·EFA·세그먼트에서 행 어긋남 | 결과 저널 재생으로 교체, 구 글라스만 fallback(§5.8) |
| 수동 Cell ID가 호스트 규약 위반(길이·체크섬) | 저장 전 검증 + MES 규약 확인(§9-16) |
| 문서 stale(§1) | Phase 8에서 07/03/Resume/CLAUDE.md 동시 갱신 |
