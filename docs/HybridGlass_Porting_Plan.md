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
  - **파일**: V1011 꼬리 블록에 `_CELL_ID_PANEL_ modeA modeB`(레시피 전역, `_HYBRID_` 뒤) + 세그먼트별 `_CELL_ID_MANUAL_`(스틱마다 A줄·B줄, 셀 수만큼 토큰, 빈 토큰 보존 파서).
  - **편집기**: DLL 신규 `CCellIdEditDlg`(RecipeMgrDlg의 "CELL ID A/B → EDIT" 행에서 모달). 작업 사본에서 편집 → APPLY가 검증+커밋, CANCEL 폐기. 세그먼트/패널/모드 콤보, 셀 맵(1차는 **세그먼트별 그리드**, 물리 합성맵은 보류), 선택 셀 ID 편집, 일괄 입력(Deg0=행, Deg90=열, 시작 번호 2자리 패딩). 색: 빨강=무효/빈칸, 주황=중복, 연노랑=Auto와 다름.
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

**EFA×AgingTest 같은 글라스 동시**: 지금도 글라스당 배타(EFA 우선, `DoWorkGlassMeasure.cpp:210-212`) — 그대로 유지.

**단계**: 세그먼트 기본 루프(Phase 3)를 N=1 무회귀로 먼저 넣고, Phase 3b(AgingTest×세그먼트: A1~A4, A7, A8, A11, A13) → Phase 3c(EFA×세그먼트: A5, A6, A10, A12) 순서. 두 기능 모두 **N=1 동작은 한 줄도 바뀌지 않아야** 한다(행 산식·마스크 섹션·게이트 래치 모두 N=1에서 기존 값과 동일).

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

### Phase 3 — 런타임 세그먼트 루프 (ALPHA)
- `DoWorkGlassMeasure` pass→seg→stick 루프(EFA 아닐 때 pass=1)·Last-Stick 가드·`SegmentBoundary(from,to)`, 외부 레시피 재송신(이름 변경 시), 방향 전환(Safety→회전→재Align), 자동 시작 전 .alg 검사, 종료 시 세그먼트 0 복귀.
- 검증(OOS): N=1 레시피 회귀(기존과 로그 동일, AgingTest·EFA 포함), 합성 Long+Short 런 — 세그먼트 경계 로그, R 90→180 회전, Align 재실행, 알람 0, 완료 이벤트 1회.

### Phase 3b — AgingTest × 세그먼트
- 마스크 세그먼트 섹션(A1, A3), 게이트 래치(A4), 행 블록 `G`(A7), `nCellStatus` 오프셋(A8), AGING 스텝 확인(A11), 수동 다이얼로그 세그먼트 선택기(A13).
- 검증(OOS): Long+Short 픽스처 + AgingTest rcp(세그먼트별 마스크) — 항목 루프가 세그먼트마다 수행, 반복 행이 `G` 블록에 위치, 세그먼트 간 행 충돌 0, 마스크 0인 스틱 skip.

### Phase 3c — EFA × 세그먼트
- EFA 게이트·`EFA^1$` 래치(A5), pass 바깥 루프와 T1 시작 N-1→0 경계(A6), 가드(A10), PTN 검증(A12).
- 검증(OOS): Long+Short + EFA rcp — T0(seg0→seg1) → 결과 플러시 1회 → (회전·재Align) → T1(seg0→seg1), T1 행 = T0 행 덮어쓰기, `EFA^1$` 1회, 완료 이벤트 1회. CIM 온라인 검증은 EFA 문서 §9.2와 함께.

### Phase 4 — 결과 행·Cell ID 자동 이월
- `SetMeasUseInfo` `segBase`, `nAreaNum` 소비처 감사, Cell ID 자동 글자 base(§5.7 1단계).
- 검증: 결과 그리드 행이 세그먼트 0 끝 다음부터 연속, Cell ID 중복 0, APD 1개, N=1 ID·행 기존과 동일.

### Phase 4b — Cell ID A/B 수동 입력 + Manual 편집기 (D-5)
- 레시피 병렬 벡터(모드·수동 ID), V1011 꼬리 `_CELL_ID_PANEL_`/`_CELL_ID_MANUAL_`, 리졸버·검증(순수), `GenCellID` 셀 토큰 교체, DLL `CCellIdEditDlg` + RecipeMgrDlg EDIT 행, 저장 전 검증, MakeCell Apply 시 초기화.
- 검증: A Manual + B Auto 파생, 형식 오류·중복 시 APPLY/저장 거부, A/B 글라스·GlassID 없음 각각 전송 ID, 저장→재로드 왕복, 구 파일(Auto/Auto) 로드.

### Phase 5 — Resume (D-6)
- 진행 튜플(pass, seg, stick), 세그먼트별 검증·안내 문구, 재개 스틱 전체 재측정, 결과 저널(`ResumeJournal.csv`) 기록·재생, `nCellStatus` 스냅샷, 북마크 수명, P0-1 전제.
- 검증(OOS): ① 세그먼트 1 중간 STOP → 재개 → 세그먼트 0 skip, 세그먼트 1 재개 스틱부터, 재Align 수행, 결과 행 복원 ② AgingTest 반복 중 STOP → 반복 행 복원 ③ EFA T1 중 STOP → T0 재수행 없이 T1 이어짐, T0 상태 복원 ④ 프로그램 재시작 후 재개(P0-1) ⑤ "No" 선택 시 튜플 초기화.

### Phase 6 — 운전 화면 표시
- 세그먼트 표시기, ChartOrbit 활성 세그먼트 갱신.

### Phase 7 (후순위) — 수동 화면 세그먼트 선택기 (D-9)

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
| `Source/ALPHA/DlgSubManual_AgingTest.cpp` | 3b | **BOM 없는 UTF-8** — 새 문자열은 영어 |
| `Source/ALPHA/DlgSubManual_AgingNew.cpp` | 3b | ASCII |
| `Source/ALPHA/DoWork/DoWorkSequenceCIMStep.cpp` (`EFA^1$` 래치) | 3c | CP949 |
| DLL `CellIdEditDlg.h/.cpp` (신규) + RecipeMgrDlg EDIT 행 + `dfcRecipe.rc`/`resource.h` | 4b | 신규 ASCII / rc·resource.h **UTF-16** |
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
