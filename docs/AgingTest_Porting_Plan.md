# AgingTest 다이얼로그 이식 플랜 (OCP → HMI)

작성: 2026-08-05 (LYS)
대상: OCP `IDD_DLG_NAVI_MANUAL_AGING_TEST` (`CDlgNaviManualAgingTest`, 860줄) → HMI `IDD_DLG_SUB_MANUAL_AGING_TEST` (`CDlgSubManual_AgingTest`, 869줄)
연관: [AgingSet_Porting_Plan.md](AgingSet_Porting_Plan.md) — 동일 구조/규칙 적용 (그리드 좌표계, ctor 주입, Perl 편집, 경로 체계)

## 1. 목적 및 범위

OCP의 Aging Test 설정/기동 다이얼로그 기능을 HMI 기존 부분 이식 다이얼로그에 완성 이식.
**AgingSet과 동일 원칙**: 기존 HMI 구조(CMySpreadDll, 클래스 구성, UI 리소스) 유지, 주석 비활성 기능 복원 + 이식 버그 수정.
레시피 경로 체계도 동일: **`D:\HBS\PARAM_OCP\PARAM\AgingRcp\{RecipeID}.rcp`** (레지스트리 `DIR\AGING_RCP` 기본값, 사용자 지시 2026-08-05).

기능 요약 (OCP 원본):
- Aging 시퀀스 레시피 편집: 스텝 리스트(Aging사용/점등패턴/점등시간/WAD/VISION/**LCR**) — **현재 로드된 설비 레시피명** 기준 `AgingRcp\{RecipeID}.rcp` INI 저장/로드
- 셀별 측정 사용 맵: 하단 그리드 4시트(WAD/VISION/Color/LCR) 체크박스, `[MASECELL] STICK_n=비트마스크CSV`로 같은 .rcp에 저장, 헤더 더블클릭 일괄 토글
- START: Cycle Count 설정 후 Aging Cycle Test 기동 (OCP: `m_bAutoAgingMode`+`nJobCount`+`JOB_Proc::DoStartCycleTest()`)
- 진행 중 리스트 현재 스텝 노란색 하이라이트 (`m_nAgingListRow/Col`), STOP 버튼

## 2. 현황 비교 (2026-08-05 검증)

| 항목 | OCP | HMI 현재 | 판정 |
|---|---|---|---|
| 다이얼로그 리소스 | 전 컨트롤 | 동일 완비 (STOP, 콤보, STICK1~10, 스프레드 2개) | 완료 |
| 그리드 생성/폰트/헤더 | DDX OCX | CMySpreadDll 2개 + Init 함수 | 완료 (HMI 방식) |
| AgingItemAdd | 체크/콤보/에디트, **LCR 포함 4개 체크(col 2,5,6,7)** | 체크 3개(col 1,4,5)+콤보+에디트 동작 | **LCR 누락 + 콤보값 차이 (P2)** |
| Load/Save INI 키 | USE_AGING/PTN_RGB/PTN_SEC/USE_PROPERTY/USE_QUALITY/**USE_LCR** | USE_LCR 없음 | **LCR 누락 (P2)** |
| `MAX_AGING_ITEM` | **7** (20220418 kks LCR 추가) | 6 (구버전) | **P2** |
| `S_AgingTest` struct | `m_bUseLCR` 있음 (OCP CommonFunc.h) | 없음 (HMI CommonFunc.h:2093) | **P2** |
| 경로 초기화 | `DIR\AGING_RCP` | 활성이나 기본값 `D:\KMAC\PARAM_ICP\PARAM\AgingRcp\` | **경로 변경 (P1)** |
| Load/Save 경로 구성 | `dfcStr(경로+RecipeID)` | **주석 → 빈 경로 (무효)** | **P1** |
| **OnItemSave** | 현재 레시피명 검사 | **`strCurrentRcp=_T("")` 하드코딩 → 무조건 차단, 저장 100% 불가** | **P1 버그** |
| Load 그리드 기록 | SetRow/SetCol/SetValue | **대각선 SetData(i,i+2) + 콤보 미기록** (AgingSet B1과 동일) | **P1 버그** |
| Save 행 인덱스 | row i+2 (OCP 좌표) | `GetDataString(j, i+2)` — HMI 좌표(데이터 행 1..)에서 한 행 어긋남 | **P1 버그** |
| UpdateListNum | col 1(행헤더) | **col 1 = Aging 컬럼 침범** + 루프 범위 OCP 잔재 | **P1 버그** |
| IsOfflineStatus | HostMode 검사 | 항상 TRUE | **P1** |
| OnItemDel 가드 | `<= 1` (OCP 좌표) | `<= 0` — HMI 좌표로는 `< 1`과 동일하나 UpdateListNum 호출 주석 | P1 (정리) |
| OnRcpOpen | 파일선택 → **`LoadPPID2()` 설비 레시피 로드** → UpdateAgingCell+LoadAgingInfo | 전체 주석 | **P3** |
| UpdateAgingCell | 4시트(WAD/VISION/Color/LCR) 셀 체크맵 구성 | 전체 주석 | **P3** |
| MeasCellSave/Load | `[MASECELL] STICK_n` 저장 + 레시피 MEASGROUP 미러 | 전체 주석 | **P3** |
| DblClickFpspreadCellStatus | 헤더 더블클릭 일괄 토글 | **이식 완료** (CMySpreadDll GetDataString/SetData) | 완료 |
| SheetChangedFsMeasArea | SetSheet 전환 | 본문 주석 | P3 (한 줄) |
| OnAgingProcStrart | Cycle Test 기동 (`DoStartCycleTest`) | 전체 주석 | **P4 (시퀀스 연동)** |
| OnStop | `m_pJobProc->ActionStop()` | 본문 주석 | **P4** |
| UpdateStatus 진행 하이라이트 | `m_bAutoAgingMode` 시 현재 스텝 노란색 | 주석 | P4 (엔진 연동 후 의미) |

## 3. 핵심 설계 결정

### 3.1 그리드 좌표계 — AgingSet §3.1 규칙 그대로

리스트 그리드: HMI 헤더=SS_HEADER(row 0), 데이터 행 1..MaxRows, 열 1..6(LCR 추가 후).
OCP `row i+2` → HMI `i+1`, `count = MaxRows-1` → `MaxRows`, `col j(2..7)` → `j-1(1..6)`.

### 3.2 LCR 컬럼 추가 (OCP 2022-04-18 kks 변경분 반영)

- `DlgSubManual_AgingTest.h`: `MAX_AGING_ITEM` 6 → **7**
- `CommonFunc.h` `S_AgingTest`: `BOOL m_bUseLCR` **struct 끝에** 추가 (UI 전용 struct — 공유메모리 미사용 확인, ShareMem.h에 미포함)
- `MySpreadDll.cpp` `InitMySpread_AGINGTEST_LIST`: `SetMaxCols(5)`→`(6)` + `SetData(6, SS_HEADER, _T("LCR"))` (ASCII — 기존 한글 헤더 "특성"/"화질"은 HMI 명칭 유지, OCP의 WAD/VISION으로 개명하지 않음 — 기존 한글 문자열 불변 원칙)
- INI 키는 OCP와 동일 `USE_LCR` (파일 키 호환 유지)
- 셀상태 그리드 4번째 시트 "LCR" (§3.4)

### 3.3 콤보(점등패턴) 값 정책 — AgingSet과 동일 "텍스트=값"

OCP는 역순 콤보(10→0 삽입) + **CurSel 인덱스 저장**이라 PTN_RGB = 인덱스(값과 불일치, 10-i). HMI는 오름차순 `"0\t..\t10\t"` + 셀 텍스트 저장으로 통일 → INI에 패턴 번호 자체가 저장됨.
**호환성 주의**: OCP가 만든 기존 AgingRcp 파일의 PTN_RGB(인덱스 의미)와 해석이 다름 — AgingRcp는 설비 로컬 신규 생성 기준이므로 허용, 기존 OCP 파일 이관 시 변환 필요함을 명기. (기존 HMI 콤보는 "1~10"으로 0 누락 — 0~10으로 교정)

### 3.4 셀별 측정 맵 (UpdateAgingCell / MeasCellSave / MeasCellLoad)

- 시트 API `SetSheetCount/SetSheet/SetSheetName/SetColor` 모두 TSpread 래퍼(Ssdllmfc.h:818-831,395)에 존재 확인 → OCP 로직 직이식 가능
- 셀 수/회전: OCP `m_pDlgNaviRecipe->m_stRecipePath` → HMI `GetRecipeEditMgrPtr()->GetRecipeData()`의 `m_vecStick.size()`/`m_vecStick[i].m_vecCell.size()` + `GetRecipeInfo()->m_sOptionItem.m_nGlsRotate`
- 저장: OCP와 동일 `[MASECELL] STICK_n = "비트,비트,..."` (bitset<4>: WAD/VISION/Color/LCR)
- **레시피 MEASGROUP 미러는 하지 않음**: OCP MeasCellLoad는 `m_aryMeasGroup[].stProbe[0].m_iUsedMeas`에 써넣지만, HMI에서는 WadMultiMeasMgr 선례(“cell-use map kept here, recipe struct untouched”)대로 **다이얼로그(또는 추후 매니저) 보관 + INI 지속**으로 한정. 시퀀스가 이 맵을 소비하는 연동은 P4에서 설계 (**TODO: AGINGTEST_CONFIRM — 소비 주체/시점**)

### 3.5 환경 의존점 (AgingSet §3.3 재사용 + 추가)

| OCP | HMI 대체 |
|---|---|
| `m_pSystem->stLotInfo.m_szRecipeID` (현재 레시피명) | `m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeID()` (RecipeEditMgr.h:102) |
| `m_pDlgNaviRecipe->LoadPPID2(name)` (설비 레시피 로드) | `GetRecipeEditMgrPtr()->RecipeChange(name)` 또는 `OpenRecipe(name)` — 구현 시 동작 확인 (**TODO: AGINGTEST_CONFIRM — UI 갱신 부수효과**) |
| HostMode / ProcessStatus | AgingSet과 동일 (`GetShareMemoryMgrPtr()`) — **ctor 주입 `m_pAuroraMgr` 동일 적용** (DlgSubManual.cpp 생성부 수정) |
| `m_pSystem->stInterLock.m_bSafetyPos_X` | HMI 인터록 대응 확인 필요 (P4) |
| `m_bAutoAgingMode`/`m_nAgingListRow/Col`/`nJobCount` | HMI ShareMem.h:1918+에 **동일 필드 존재** — `GetShareMemoryMgrPtr()->GetSharedMemory()` 직접 접근 가능 |
| `m_pJobProc->DoStartCycleTest()` | **HMI 대응 엔진 부재** — P4 설계 항목 (CSequenceManager 경유 신규 시퀀스 or 기존 P5 Measure 시퀀스 반복 기동) |
| `m_pJobProc->ActionStop()` | HMI 시퀀스 정지 경로 (SequenceManager) — P4 |
| 진행 하이라이트 SetBackColor | `SetColor(col, row, bg, fg)` (래퍼 :395) |

## 4. 실행 Phase

### Phase 1 — 경로/복원 + 버그 수정 (AgingSet Phase 1과 동일 유형) ✅ 완료 (2026-08-06)

**실행 결과**: 아래 1~9 전부 적용 (`// LYS 2026-08-06` 주석). 추가로 ctor에서 spread/hfont 포인터 NULL 초기화, OnItemSave 시 EDIT_1 현재 레시피명 동기화, AgingItemAdd 끝 UpdateListNum 복원. `UpdateStatus`는 HMI에서 미호출임을 확인하여 EDIT_1 갱신은 OnInitDialog/OnItemSave/LoadAgingInfo 시점으로 배치. damaged=0 (3파일), Debug/Release x64 빌드 통과. 백업 `*.bak_agingtest`.

파일: `DlgSubManual_AgingTest.cpp`(+`.h` ctor 주입), `DlgSubManual.cpp` 생성부 — 전부 Perl raw byte
1. ctor 주입: `CAuroraMgr* m_pAuroraMgr` (AgingSet/MultiMeas 선례)
2. `AGING_RCP` 기본 경로 → `D:\\HBS\\PARAM_OCP\\PARAM\\AgingRcp\\`
3. **OnItemSave 차단 버그 해제**: `strCurrentRcp = GetRecipeEditMgrPtr()->GetRecipeID()` 복원
4. Load/Save/MeasCell 경로 구성 복원: `dfcStr(_T("%s%s.rcp"), m_strAgingRcpPath, GetRecipeID())`
5. LoadAgingInfo 재작성: 좌표계 정정(행 i+1, 열 1..5→P2에서 6), 콤보 셀 SetData 기록, `_ttoi` 유지(전 필드 정수 — AgingSet과 달리 실수 없음)
6. SaveAgingInfo: 행 i+1 통일, count=MaxRows, PTN_RGB는 셀 텍스트 저장
7. UpdateListNum: col 0(행헤더), 루프 1..MaxRows / OnItemDel에 UpdateListNum 복원
8. IsOfflineStatus 복원 (한글 메시지 바이트 스플라이스)
9. EDIT_1 표시: OnInitDialog/UpdateStatus에서 현재 레시피명 표시 (OCP는 RcpOpen 시 갱신 — HMI는 진입 시 GetRecipeID() 표시로 보강)

### Phase 2 — ~~LCR~~ **ANGLE** 컬럼 확장 ✅ 완료 (2026-08-06)

**구조 변경 (사용자 지시 2026-08-06)**: HMI에는 LCR 모드가 없어 OCP의 LCR 컬럼을 **ANGLE 컬럼으로 대체**. INI 키도 `USE_LCR` → `USE_ANGLE` (OCP 파일과 키 비호환 — HMI 신규 생성 기준).

**실행 결과**:
- `MAX_AGING_ITEM` 6→7, `S_AgingTest.m_bUseAngle` struct 끝 추가 (CommonFunc.h — 컴파일 전용, 공유메모리 무관)
- `InitMySpread_AGINGTEST_LIST`: `SetMaxCols(6)` + col 6 헤더 "ANGLE" (기존 한글 헤더 불변, AGINGSET 함수 미접촉 확인)
- `AgingItemAdd`: col 6 체크박스 추가 + 콤보 리스트 "1~10" → "0~10" 교정 (누락됐던 패턴 0 추가, §3.3 텍스트=값 정책)
- Load/Save: `USE_ANGLE` 키 + col 6 왕복
- damaged=0 (4파일), Debug/Release x64 빌드 통과. 백업 `*.bak_agingtest` (CommonFunc.h/MySpreadDll.cpp 포함)

### Phase 3 — 셀 측정 맵 + 레시피 열기 ✅ 완료 (2026-08-06)

1. UpdateAgingCell: 4시트 구성 (OCP 576-699 직이식, RecipeData 셀 수 기반, 래퍼 API로 치환)
2. MeasCellSave/Load: `[MASECELL]` INI 왕복 (MEASGROUP 미러 제외 — §3.4)
3. SheetChangedFsMeasArea 복원 (`SetSheet(NewSheet)`)
4. OnRcpOpen: 파일 선택 → `RecipeChange/OpenRecipe` → EDIT_1 갱신 → UpdateAgingCell + LoadAgingInfo

**실행 결과** (WadMultiMeas RebuildCellMap 패턴 채택, OCP 직이식 대신 HMI-native 재설계):
- `UpdateAgingCell`: 4시트(WAD/VISION/Color/**ANGLE**) × [행=Stick(S1..Sn), 열=Cell] 체크박스 맵. **회전 변환 없이** 현재 `RecipeData`의 `m_vecStick/m_vecCell` 구조 그대로 사용 (OCP는 CellCount.x/y+rotate로 유도 — HMI RecipeData는 이미 논리 구조). 값 기본 0, `SetSheetName`으로 시트명 부여
- `MeasCellSave/Load`: `[MASECELL] STICK_n = "마스크,마스크,..."` (bit0~3 = WAD/VISION/Color/ANGLE — OCP bit3=LCR 대체). 버퍼 MAX_PATH→1024 확대(셀 다수 시 잘림 방지), 행 토큰수 불일치 시 해당 행만 스킵(OCP는 전체 return). 시트 위치 보존(GetSheet/SetSheet 복원)
- **호출 순서 교정**: OCP는 MeasCellLoad(레시피 구조체 갱신)→UpdateAgingCell(구조체→그리드)이나, HMI는 구조체 미러를 하지 않으므로 **UpdateAgingCell(그리드 구축)→MeasCellLoad(INI→그리드)** 순서로 LoadAgingInfo 내 호출 교체
- 더블클릭 일괄 토글: OCX EVENTSINK는 DLL 스프레드에서 발화하지 않음 → `ON_MESSAGE(SSM_DBLCLK, OnSpreadDBClick)` 라우팅 신설(MultiMeas 선례, lParam=LPSS_CELLCOORD), 기존 `DblClickFpspreadCellStatus` 재사용. `SheetChangedFsMeasArea`는 사유 주석과 함께 no-op 유지
- `OnRcpOpen`: 파일 선택 → `RecipeChange()` (LoadRcpFile+Align+RecipeUpdateUI, `OpenRecipe`의 SetProbe 부수효과 회피) → EDIT_1 갱신 → LoadAgingInfo
- **추가 (사용자 요청 2026-08-06)**: RecipeChange 성공 후 `{AgingRcp경로}\{레시피명}.rcp` 부재 시 경고 팝업 — "{경로} 경로에 Recipe가 없습니다. 저장 후 재오픈 하십시오." (경로는 `m_strAgingRcpPath` 동적 표시, CP949 인코딩 삽입). 경고 후에도 LoadAgingInfo는 진행되어 리스트는 빈 상태로 초기화됨
- damaged=0, Debug/Release x64 빌드 통과. 백업 `.bak_agingtest_p3`
- **런타임 확인 항목**: DLL 스프레드 시트 탭 표시 여부(SSM_TABSCROLL 존재로 지원 추정), 탭 클릭 시 current sheet 동기(더블클릭 토글이 보이는 시트에 적용되는지)

### Phase 4 — Cycle Test 기동/정지 ✅ 구현 완료 (2026-08-06, 엔진 일부 TODO)

**분석 결과**: OCP `DoStartCycleTest` = 워커에 `WORK_START_CYCLE_TEST` 게시 → `nJobCount`회 [레시피 로드→측정 시퀀스→Safety 이동] 루프. 자동 에이징 시 측정 내부가 **`DoWorkStartMeasOCP_A`(6,743줄, JOB_Proc 12351-19094)** — 에이징 리스트 기반 패턴 점등/셀별 측정 전용 엔진으로 분기. HMI에는 이 엔진 부재이나, **HMI 엔진에 사이클 반복이 이미 존재**: `CSystemParamShare` `IsCycleState()/GetCycleCount()`를 `DoWorkGlassSequenceEngine`/`MeasureSequenceItemEngine`이 소비 (Operate Cycle 버튼 경로).

**구현 내역**:
- `OnAgingProcStrart`: 가드(Offline/SPS_PROCESS/레시피/리스트) → Cycle Count(콤보) → 그리드 6열 → `m_vecAginList` 수집 → 공유메모리 `m_bAutoAgingMode=TRUE`/`m_nAgingListRow·Col=0`/`nJobCount` 세팅(OCP 패리티) → **HMI 사이클 검사 경로로 기동**: `SetCycleCount`+`SetCycleState(true)` → FRAME/TITLE에 `WM_MSG_UPDATE_CYCLE_INSPECTION_START` 통지 → `EngineStart()` (DlgSubOperate Cycle 버튼과 동일 경로). STOP 버튼 표시
- `OnStop`: `SetCycleState(false)` + `m_bAutoAgingMode=FALSE` + `IsEngineCompleteAll()==false`면 `EngineStop()` (Operate STOP 버튼 패턴). STOP 버튼 숨김
- Safety 위치 가드(OCP `stInterLock.m_bSafetyPos_X`)는 명시 검사 생략 — HMI는 인터록 FunctionModule이 시퀀스 내에서 처리
- **신규 메시지 전부 영어** — 파일이 BOM 없는 UTF-8인데 컴파일은 CP949라 이 파일의 한글 리터럴은 런타임 표시가 깨짐(§리스크 참조)
- damaged=0, Debug/Release x64 빌드 통과. 백업 `.bak_agingtest_p4`

**남은 TODO (AGINGTEST_CONFIRM)**:
1. **에이징 리스트 구동 측정 엔진 미이식**: `m_bAutoAgingMode`의 HMI 엔진 소비처가 없어, 현재 START는 표준 측정 시퀀스를 N회 반복. 패턴별 점등(PTN/Sec)+셀맵 기반 WAD/VISION/Color/ANGLE 선별 측정은 `DoWorkStartMeasOCP_A`(6,743줄) 대응 설계 승인 후 별도 진행
2. UpdateStatus 진행 하이라이트(`m_nAgingListRow/Col` 노란색): HMI에서 UpdateStatus 미호출 + 엔진 미구현으로 보류
3. AgingSet P4의 P5 AGING 알람 훅과는 독립 동작 (사이클 중 AGING 스텝이 돌면 알람 훅도 그대로 적용됨)

**인코딩 리스크 (발견 2026-08-06)**: `DlgSubManual_AgingTest.cpp`(및 동류 파일)의 기존 한글 리터럴은 UTF-8 바이트인데 MSVC가 CP949로 컴파일 → **기존 한글 팝업도 런타임에 깨져 있음** (이식 이전부터의 상태). 근본 해결은 `/utf-8` 컴파일 옵션 추가 또는 파일 CP949 변환 — 전 프로젝트 영향이라 별도 결정 필요.

## 5. 수정 파일

| Phase | 파일 | 편집 방식 |
|---|---|---|
| 1 | `DlgSubManual_AgingTest.cpp/.h`, `DlgSubManual.cpp` | Perl raw byte (한글 포함) |
| 2 | 위 + `CoreFunction/common/CommonFunc.h`, `CoreFunction/common/MySpreadDll.cpp` | Perl raw byte (EUC-KR 한글 포함) |
| 3 | `DlgSubManual_AgingTest.cpp` | Perl raw byte |
| 4 | + 시퀀스/알람 파일 (설계 후 확정) | — |

리소스(.rc)/신규 파일 없음.

## 6. 검증 계획 (PG 불필요 — 전부 로컬)

1. 빌드 Debug/Release x64 + `damaged=0`
2. ITEM 추가: 체크 4개(Aging/특성/화질/LCR)/콤보 0~10/시간 에디트, No열 번호
3. 저장 → `D:\HBS\PARAM_OCP\PARAM\AgingRcp\{현재레시피}.rcp` 생성, USE_LCR 포함 키 확인 — **현재 레시피가 로드돼 있어야 함** (RecipeEditMgr GetRecipeID 비어있으면 차단 팝업이 정상 동작인지도 확인)
4. 이전값 불러오기 왕복 일치
5. 셀상태 그리드: 시트 4개 전환, 헤더 더블클릭 일괄 토글, [MASECELL] 저장/로드 왕복
6. START는 P4 전까지 무동작 유지 확인 (기존과 동일)

## 7. 리스크

| 리스크 | 완화 |
|---|---|
| `RecipeChange/OpenRecipe`의 UI 부수효과 (Operate 화면 갱신, CIM 통보 등) | P3 구현 전 호출 경로 추적, 필요 시 읽기 전용 로드 함수 선별 |
| S_AgingTest 확장이 dfcRecipe DLL 등 다른 사용처에 영향 | 사용처 grep 결과 다이얼로그 2개뿐임을 P2 시점 재확인 |
| OCP 기존 AgingRcp 파일의 PTN_RGB 인덱스 의미 차이 | §3.3 명기 — 신규 생성 기준, 이관 시 변환표 |
| MySpreadDll.cpp 공용 파일 수정(Init 함수) — 타 화면 영향 | AGINGTEST_LIST 전용 함수라 영향 없음, 백업 `.bak_agingtest` |
| Cycle Test 엔진 부재 | P4 분리, 설계 승인 후 진행 |
