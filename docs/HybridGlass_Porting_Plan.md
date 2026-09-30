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

### Phase 4 — 결과 행·Cell ID 자동 이월
- `SetMeasUseInfo` `segBase`, `nAreaNum` 소비처 감사, Cell ID 자동 글자 base(§5.7 1단계).
- **결과 저장 게이트 (사용자 확정 2026-09-29)**: Result(`DoWorkGlassResultStep.cpp:70-101`, 광학 6항목 + AgingTest_2)와 Unload(`DoWorkGlassUnloadStep.cpp:195-240` CheckSaveReuslt, 광학 6항목만 → 아니면 `SetUserBypassGlass(true)`)가 **복귀한 세그먼트 0의 검사 항목만** 본다 → 세그먼트 0이 전기 항목만인 하이브리드 런은 결과 저장 생략 + CIM bypass(2026-09-29 17:39 OOS 런 실증, `RESULT\DATA` 무파일). 정책: **전기 검사(OST/R-TEST)만 켠 글라스는 bypass 유지, Aging(AgingTest_2) 검사는 결과 저장.** 수정: 두 단계를 같은 게이트 = "어느 세그먼트든 광학 6항목 또는 AgingTest_2가 켜져 있으면 저장"으로 통일(Unload에 AgingTest_2 추가 = 기존 불일치 해소).
- 검증: 결과 그리드 행이 세그먼트 0 끝 다음부터 연속, Cell ID 중복 0, APD 1개, N=1 ID·행 기존과 동일.

### Phase 4b — Cell ID A/B 수동 입력 + Manual 편집기 (D-5)
- 레시피 병렬 벡터(모드·수동 ID), V1011 꼬리 `_CELL_ID_PANEL_`/`_CELL_ID_MANUAL_`, 리졸버·검증(순수), `GenCellID` 셀 토큰 교체, DLL `CCellIdEditDlg` + RecipeMgrDlg EDIT 행, 저장 전 검증, MakeCell Apply 시 초기화.
- 검증: A Manual + B Auto 파생, 형식 오류·중복 시 APPLY/저장 거부, A/B 글라스·GlassID 없음 각각 전송 ID, 저장→재로드 왕복, 구 파일(Auto/Auto) 로드.

### Phase 5 — Resume (D-6)
- 진행 튜플(pass, seg, stick), 세그먼트별 검증·안내 문구, 재개 스틱 전체 재측정, 결과 저널(`ResumeJournal.csv`) 기록·재생, `nCellStatus` 스냅샷, 북마크 수명, P0-1 전제.
- 검증(OOS): ① 세그먼트 1 중간 STOP → 재개 → 세그먼트 0 skip, 세그먼트 1 재개 스틱부터, 재Align 수행, 결과 행 복원 ② AgingTest 반복 중 STOP → 반복 행 복원 ③ EFA T1 중 STOP → T0 재수행 없이 T1 이어짐, T0 상태 복원 ④ 프로그램 재시작 후 재개(P0-1) ⑤ "No" 선택 시 튜플 초기화.

### Phase 6 — 운전 화면 표시
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
| `Source/ALPHA/DlgSubManual_AgingTest.cpp/.h` | 3b | **UTF-8 BOM, CRLF**(2026-09-29 확인, 한글은 UTF-8 바이트) — 종전 "BOM 없는 UTF-8"은 stale |
| `Source/ALPHA/DlgSubManual_AgingNew.h/.cpp` (세그먼트 선택기) | 3b·3c | **CP949, LF-only, BOM 없음**(종전 "ASCII"는 틀림) — LF 정규화 후 `\n` 패치, 한글 팝업은 `Encode::encode("cp949")` |
| `Source/ALPHA/DoWork/DoWorkSequenceCIMStep.cpp` (`EFA^1$` 래치) | 3c | CP949 CRLF |
| `Source/ALPHA/Recipe/RecipeEditMgr.h/.cpp` (세그먼트 캐시·`GetSegLogTag`·EFA 래치 저장소) | 3c | CP949 CRLF — `.h` 변경은 넓게 재컴파일 |
| `Source/ALPHA/DoWork/DoWorkGlassMeasure.cpp`, `DoWorkGlassMeasureStep.cpp`, `MeasureSequenceItemEngine.h/.cpp` (태그·스틱 헤더·tact 이름·래치 게시) | 3c | CP949 CRLF |
| `Source/ALPHA/DoWork/DoWorkGlassRecipeStep.cpp` (`CheckHybridCapacity` EFA K, `CheckHybridAgingMask`) | 3c | CP949 CRLF |
| `Source/ALPHA/ALPHA.rc` (AgingNew 콤보·라벨) | 3c·3d | **UTF-16 LE BOM, CRLF** — 이 PC는 python이 깨져 Perl `Encode` UTF-16LE(`FB_CROAK`, BOM 유지)로 패치 |
| `Source/ALPHA/resource.h` (`IDC_COMBO_EFA_SEG` 21008, `IDC_STATIC_EFA_SEG` 21009) | 3c·3d | UTF-8 BOM, CRLF — ASCII raw 삽입 |
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
