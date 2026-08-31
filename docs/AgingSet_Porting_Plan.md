# AgingSet 다이얼로그 이식 플랜 (OCP → HMI)

작성: 2026-08-05 (LYS)
대상: OCP `IDD_DLG_NAVI_MANUAL_AGING_SET` (`C_DlgNaviManualAgingSet`) → HMI `IDD_DLG_SUB_MANUAL_AGING_SET` (`CDlgSubManual_AgingSet`)

## 1. 목적 및 범위

OCP Metrosys의 Aging 목표값 알람 설정 다이얼로그 기능을 HMI ALPHA의 기존 부분 이식 다이얼로그에 완성 이식한다.
**기존 HMI 구조(CMySpreadDll 그리드, 클래스/멤버 구성, UI 리소스)는 유지**하고, 주석 처리로 비활성화된 기능과 이식 과정에서 생긴 버그만 수정/추가한다.

기능 요약 (OCP 원본 기준):
- Aging 목표 알람 레시피(`.rcp` INI, `AgingTargetRcp\{RecipeID}.rcp`)의 편집/저장/로드
- 그리드: Use(체크) / 점등패턴(콤보 0~20) / 기준값 / 상한값 / 하한값 (mA)
- START: Aging 결과 파일(`T:\AGING.txt`)을 파싱하여 패턴별 전류값이 기준±상하한 범위를 벗어나는 Cell 검출
- AGING ALARM USE 토글 (레지스트리 `CONFIG\AGING_ALARM_USE` 저장)
- (OCP) JOB_Proc 자동 시퀀스에서 AGING 측정 전후 훅 호출 — HMI 시퀀스 연동은 Phase 4(별도 확인 후)

## 2. 현황 비교 (2026-08-05 검증 기준)

| 항목 | OCP (원본, 711줄) | HMI (현재, 738줄) | 판정 |
|---|---|---|---|
| 다이얼로그 리소스 | FPSpread OCX 직접 배치 | 전 컨트롤 존재 (BUTTON_2~8, EDIT_1~3, CHECK_1, FPS placeholder) | **완료 — 수정 불필요** |
| 클래스 골격/멤버/메시지맵 | — | 동일 구성 | 완료 |
| 그리드 생성/초기화 | DDX + OnInitDialog 셀 설정 | `CMySpreadDll` + `InitMySpread_AGINGSET()` | 완료 (HMI 방식 유지) |
| AGING ALARM USE 토글 | OnAgingAlarmUse | 동일 동작 | 완료 |
| 파일 탐지 (`CmAgingFileFind/TimeOut`) | 구현 | 동일 이식됨 | 완료 |
| **Aging Rcp 경로 초기화** | OnInitDialog에서 레지스트리+EQType 기본경로 | **주석 처리 → `m_strAgingRcpPath` 빈 문자열** | **누락 P1** |
| **Load/Save 경로 구성** | `dfcStr("%s%s.rcp", 경로, 레시피명)` | **주석 처리 → 빈 경로로 INI API 호출(무효)** | **누락 P1** |
| **AgingItemAdd()** | 체크/콤보/에디트 셀타입 행 추가 | **본문 전체 주석 → ITEM 추가 무동작** | **누락 P1** |
| **OnAgingProcStrart()** | 검증→그리드 수집→FindAgingAlarm→결과 표시 | **본문 전체 주석 → START 무동작** | **누락 P1** |
| **IsOfflineStatus()** | HostMode 검사 | **항상 TRUE (검사 주석)** | **누락 P2** |
| AgingItemClearAll() 헤더 재구성 | 수동 헤더 셀 작성 | 헤더 재작성 주석 (InitMySpread가 대체 가능) | P2 (재호출로 대체) |
| OnRcpOpen 초기 폴더 | 레시피 루트 경로 | 주석 (현재 폴더에서 열림) | P3 |
| LoadAgingInfo 그리드 기록 | SetRow/SetCol/SetValue | **행/열 인덱스 오류 (버그)** | **버그 B1** |
| Load 값 파싱 | `atof` | **`_ttoi` (소수점 절삭)** | **버그 B2** |
| SaveAgingInfo 그리드 읽기 | 열 2~6, 행 i+2 | **행 인덱스 불일치 + `%d`에 CString (버그)** | **버그 B3** |
| FindAgingAlarm 값 비교 | `atof` | **`_ttoi` (소수점 절삭)** | **버그 B4** |
| UpdateListNum No 기록 | 열1(행헤더) | **col 1 = Use 컬럼 침범 의심** | **버그 B5 (확인)** |
| JOB_Proc 시퀀스 훅 | AGING 측정 전후 호출 (2개소) | 없음 | Phase 4 (확인 필요) |

## 3. 핵심 설계 결정

### 3.1 그리드 행/열 좌표계 통일 (가장 중요)

OCP FPSpread 직접 제어와 HMI CMySpreadDll 래퍼는 좌표계가 다르다. 현재 HMI 코드는 OCP 좌표계 산식을 그대로 들고 와서 어긋나 있다.

| | OCP (FPSpread 직접) | HMI (CMySpreadDll) |
|---|---|---|
| 헤더 | row 1 (데이터 영역 첫 행을 헤더로 사용) | `SS_HEADER` (row 0, 별도) |
| 데이터 행 | row 2 .. MaxRows | **row 1 .. MaxRows** |
| 항목 수 | `GetMaxRows() - 1` | **`GetMaxRows()`** |
| No 컬럼 | col 1 | **col 0 (행 헤더)** |
| Use/패턴/기준/상한/하한 | col 2/3/4/5/6 | **col 1/2/3/4/5** |

**변환 규칙 (모든 함수에 일괄 적용):**
- OCP `row = i + 2` → HMI `row = i + 1`
- OCP `count = GetMaxRows() - 1` → HMI `count = GetMaxRows()`
- OCP `col j (2..6)` → HMI `col j-1 (1..5)`
- 가드 OCP `nMaxRow <= 1` → HMI `nMaxRow < 1` (또는 `<= 0`)
- `InitMySpread_AGINGSET()`이 `SetMaxRows(0)`으로 시작하므로 빈 리스트 = MaxRows 0

### 3.2 셀타입 생성 — AgingTest 검증 패턴 재사용

`DlgSubManual_AgingTest.cpp:533-586` (`AgingItemAdd`)에 이미 동작하는 패턴이 있다. 그대로 차용:

```cpp
SS_CELLTYPE cellType;
// checkbox (col 1 = Use)
m_spreadAgAlarmList->SetTypeCheckBox(&cellType, SSS_ALIGN_VCENTER | SSS_ALIGN_CENTER,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
m_spreadAgAlarmList->SetCellType(1, nNewRow, &cellType);
// combobox (col 2 = PTN, "0\t1\t...\t20\t")
m_spreadAgAlarmList->SetTypeComboBox(&cellType, SSS_ALIGN_VCENTER | SSS_ALIGN_CENTER, sComboList);
// edit (col 3~5, 초기값 "0.0000"은 SetData로)
m_spreadAgAlarmList->SetTypeEdit(&cellType, SSS_ALIGN_VCENTER | SSS_ALIGN_CENTER, 20, NULL, SS_CASE_NOCASE);
```

주의: OCP 콤보는 20→0 역순 삽입 후 CurSel(0)=20이 아니라... 원본은 `for (j=20; j>=0; j--)` 후 `SetTypeComboBoxCurSel(0)` → 표시 순서 20,19,...,0에서 첫 항목 선택. **HMI 콤보 문자열은 저장값과 표시값이 동일한 정수이므로 `"0\t1\t...\t20\t"` 오름차순으로 단순화**하고, Save 시 콤보 선택 인덱스가 아닌 셀 텍스트(`GetDataString`)를 그대로 저장 (AgingTest가 이미 이 방식 사용 — PTN_RGB를 텍스트로 저장). OCP는 CurSel 인덱스를 저장했으므로 **역순 콤보에서는 인덱스≠값이었으나, HMI는 텍스트=값으로 일관** → 데이터 호환성 문제 없음 (INI에는 항상 패턴 번호 자체가 저장됨).

### 3.3 환경 의존점의 HMI 대응

| OCP | HMI 대체 (검증 완료) |
|---|---|
| `m_pSystem->stTitle.m_nHostMode != df_HOST_OFFLINE` | `theApp.m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_MachineMode() != df_HOST_OFFLINE` (ALPHA.h:20, ShareMemoryMgr.h:35) |
| `SMEM_GetSys_ProcessStatus() == SPS_PROCESS` | `theApp.m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_ProcessStatus() == SPS_PROCESS` (ShareMemoryMgr.h:27) |
| `theApp.GetProfileString("DIR","AGING_TARGET_RCP", EQType별 기본경로)` | 동일 API. 기본경로는 AgingTest 선례(`D:\KMAC\PARAM_ICP\PARAM\AgingRcp\`) 따라 `D:\KMAC\PARAM_ICP\PARAM\AgingTargetRcp\` — **TODO: AGING_CONFIRM (설비 실경로 확인)** |
| `m_pSystem->stLotInfo.m_szRecipeID` | Phase 1~3에서는 불필요 (EDIT_1 텍스트 사용). Phase 4 시퀀스 연동 시 HMI 현재 레시피명 접근자 확정 필요 |
| `dfcStr` / `dfcCreateFolder` | 동일 함수 존재 (CommonFunc.h:867-868) |
| `S_AgingTargetAlarm` | 동일 struct 존재 (CommonFunc.h:2114) |
| `atoi`/`atof` | `_ttoi`/`_tstof` (Unicode) |

## 4. 실행 Phase

### Phase 1 — 경로/설정 복원 + 버그 수정 (기존 코드 정정) ✅ 완료 (2026-08-05)

파일: `Source/ALPHA/DlgSubManual_AgingSet.cpp` (한글 포함 → **Perl raw byte 편집, Edit 도구 금지**)

**실행 결과**: 아래 1~9 전부 적용 + 추가로 ctor 주입 방식 채택(.h 변경 — MultiMeas 선례). `m_pAuroraMgr`를 `CDlgSubManual`이 생성 시 전달, `IsOfflineStatus`는 `GetShareMemoryMgrPtr()->GetSys_MachineMode()` 사용. `df_HOST_OFFLINE`은 CommonFunc.h:937 정의를 사용(AlarmDefine.h include 시 C4005 재정의 경고 발생하여 제거). 한글 메시지는 원본 주석의 바이트를 그대로 스플라이스하여 보존. damaged=0 (3파일), Debug/Release x64 빌드 통과. 백업: `*.bak_agingset`.

1. **OnInitDialog**: `m_strAgingRcpPath = theApp.GetProfileString(_T("DIR"), _T("AGING_TARGET_RCP"), _T("D:\\KMAC\\PARAM_ICP\\PARAM\\AgingTargetRcp\\"));` + `IDC_EDIT_2` 표시 복원
2. **LoadAgingInfo / SaveAgingInfo**: `strAgingRcpPath = dfcStr(_T("%s%s.rcp"), m_strAgingRcpPath, strRcp);` 복원, Save에 `dfcCreateFolder(m_strAgingRcpPath)` 복원
3. **B1**: LoadAgingInfo 내부 루프 변수 `i` 섀도잉 제거(외부 `iItem` / 내부 `iCol` 분리), `SetData(iCol-1, iItem+1, ...)` — 3.1 좌표계 적용. PTN_RGB(col 2)도 SetData로 그리드 기록 추가
4. **B2**: `m_fPtnBase/Up/Lo = _tstof(cValue)` (`_ttoi` → `_tstof`)
5. **B3**: SaveAgingInfo 행 인덱스 `i+1`로 통일, col 1~5, `j==2`(PTN)의 `Format(%d, GetDataString...)` 제거 → `strValue = GetDataString(2, i+1)` 직접 저장. 값 컬럼은 `Format(_T("%.4lf"), _tstof(GetDataString(...)))` 유지
6. **B4**: FindAgingAlarm 비교식 `_ttoi` → `_tstof` (2개소)
7. **B5**: UpdateListNum — No를 col 0(행 헤더)에 기록: `SetDataString(0, iRow, ...)`, 루프 범위 `1..GetMaxRows()`. (col 0 기록이 래퍼에서 불가하면 InitMySpread_AGINGSET 쪽 SS_HEADER 열 사용 방식 확인 후 결정)
8. **IsOfflineStatus** 복원 (3.3 대응식). 단, 알람 메시지 한글 리터럴은 기존 주석 내 문자열 재사용
9. 카운트/가드 좌표계 정리: OnItemDel `nMaxRow < 1`, Save/Load `nMaxCount = GetMaxRows()`

### Phase 2 — AgingItemAdd() 구현 ✅ 완료 (2026-08-05)

- 3.2 패턴으로 col1 체크박스 / col2 콤보(0~20) / col3~5 에디트 + 초기값 `SetData(col, nNewRow, _T("0.0000"))`
- 마지막에 `UpdateListNum()` 호출 복원

**실행 결과**: 본문 전체(ASCII 주석뿐, 한글 없음 확인) 교체. 콤보는 오름차순 `"0\t..\t20\t"` 생성 후 초기값 "0", 체크박스 초기값 "0", 에디트 초기값 "0.0000". damaged=0, Debug/Release x64 빌드 통과.

### Phase 3 — OnAgingProcStrart() 구현 ✅ 완료 (2026-08-05)

OCP 202-323 흐름을 HMI 좌표계로 이식:
1. `IsOfflineStatus()` / `GetSys_ProcessStatus() == SPS_PROCESS` 가드
2. 확인 메시지 → EDIT_1 레시피 검증 (OCP 원본의 `strRcp == "" && strRcp == "UNKNOWN"` 은 **원본 버그(&&)** — `||`로 교정하여 이식)
3. 그리드 → `m_vecAgTgAlarm` 수집 (col 1~5, row 1..MaxRows)
4. `FindAgingAlarm()` → `AGING_NOT_FILE` / `AGING_FIND_ALARM` 메시지 표시 (기존 주석 내 한글 문자열 재사용)

**실행 결과**: 위 1~4 전부 적용. 한글 메시지 5종은 주석 본문에서 바이트 추출·재사용(자동 검증: 정확히 5개 매칭 확인 후 스플라이스). 그리드 수집은 `GetDataInt`/`GetDataDbl` 사용. `SPS_PROCESS`는 include 체인의 `ShareMemFunction\ShareMem.h` 정의 사용(추가 include 없음). damaged=0, Debug/Release x64 빌드 통과.

**경로 확정 (2026-08-05, 사용자 지시)**: `AGING_TARGET_RCP` 기본 경로 = `D:\HBS\PARAM_OCP\PARAM\AgingTargetRcp\` — §3.3의 TODO: AGING_CONFIRM 해소.

### Phase 4 — 자동 시퀀스 연동 ✅ 구현 완료 (2026-08-05, 런타임 검증 대기)

**구현 내역**:
- 신규 `DoWork/AgingAlarmCheck.h/.cpp` — 비-UI 헬퍼 (`GetTargetRcpPath/GetAgingFilePath/GetAlarmUse/DeleteAgingResultFile/LoadTargetRcp/Find`). 파서는 다이얼로그 `FindAgingAlarm`과 동일 로직 + 10초 파일 대기. ALPHA.vcxproj/.filters 등록 완료(2.5.Measure 필터).
- `DoWorkGlassMeasureStep.cpp` AGING 스텝 훅 2개 (`bIsOCP` 가드 — HMI 모드 무영향):
  - Pre-hook (:5678, PG Start 직전): `AGING_ALARM_USE=1`이면 결과파일 삭제 + `GetRecipeID()` 기준 Target Rcp 로드, 실패 시 `SetFail(true, 7495)` + return false
  - Post-hook (:5982, PG Complate 성공 직후): `Find()` → 파일없음 7496 / Spec Out 7497 → `lRet` 대입으로 기존 실패 경로(`SetFail`) 경유
- `AlarmIDList.h`: `PROPERTY_AGING_RCP_OPEN_FAIL=7495 / FILE_OPEN_FAIL / SPEC_OUT_ALARM` — **OCP와 동일 ID** (HMI 7495~7497 빈 대역 확인). pch 포함 헤더라 전체 리빌드 수행.
- `Alarm.xls` 3행 추가 (Level L, Module AGING, 한/영 문안, 백업 `.bak_agingset`)
- 검증: damaged=0 (4파일), Debug/Release x64 빌드 통과. 백업 `*.bak_agingset`.
- **런타임 검증 (PG 미연동 가능)**: OOS 상태에서 레시피 로드 + `m_bUseAgingTest_2=1` 레시피로 측정 시퀀스 기동 → ① TargetRcp 없음 → 알람 7495 ② TargetRcp 있음+AGING.txt 없음 → 가상 대기 후 7496 ③ SpecOUT 픽스처 배치 → 7497 ④ `AGING_ALARM_USE=0` → 훅 미동작 확인

#### (참고) 당초 구조 분석 (2026-08-05)

OCP JOB_Proc.cpp 6907-6970 / 13580-13650: AGING 측정(`ExeSubMeas(iTop,3)`) 직전 `DeleteAgingFile()`+`LoadAgingInfo(RecipeID)`, 성공 후 `FindAgingAlarm()` → 알람 7495(RCP_OPEN_FAIL)/7496(FILE_OPEN_FAIL)/7497(SPEC_OUT).

**연동 지점 확정 (사용자 2026-08-05)**: ~~P1~P4 AGING 시퀀스~~ → **P5 측정 시퀀스의 `CDoWorkGlassMeasureStep_AGING::Run()`** (`DoWork/DoWorkGlassMeasureStep.cpp:5638`, LYS 2026-06-30 OCP 분기 기적용 클래스). 실행 조건: 레시피 `m_sInspectItem.m_bUseAgingTest_2` == true.

**구조 분석 결론 — HMI 구조 수정 필요 사항**:
1. **UI 비의존 헬퍼 분리 필수**: OCP는 JOB_Proc이 `m_pDlgNaviManual->m_dlgNaviManualAgingAlarm.*()`을 직접 호출하지만, HMI DoWork 스텝(`m_pShareItem`)은 다이얼로그 접근 경로가 없고 시퀀스 스레드에서 FarPoint 그리드 갱신은 크로스스레드 위험. → 신규 비-UI 헬퍼(예: `AgingAlarmCheck.h/.cpp`)로 분리:
   - `LoadTargetRcp(path, rcpName, vector<S_AgingTargetAlarm>&)` — INI 파싱만 (그리드 갱신 없음)
   - `Find(agingFilePath, vector, CString& strPtn, int& nCell)` — AGING.txt 파싱 (다이얼로그 FindAgingAlarm과 동일 로직 + 10초 파일 대기)
   - 설정 읽기: `AfxGetApp()->GetProfileInt/String` (CONFIG\AGING_ALARM_USE, DIR\AGING_FILE, DIR\AGING_TARGET_RCP) — 워커 스레드에서 레지스트리 읽기 안전
   - 다이얼로그는 단기적으로 현행 유지(로직 중복 허용), 추후 헬퍼 호출로 리팩터 가능
2. **훅 위치**:
   - Pre-hook: `// Start` 블록 직전(≈:5677) — `AGING_ALARM_USE=1`이면 AGING.txt 삭제 + Target Rcp 로드, 실패 시 RCP_OPEN_FAIL 알람
   - Post-hook: `DoWorkInspectComplate(enumTCPIpFactorytID_PG, 8, nTimeOut)` 성공 직후(≈:5955), `bIsOCP` 가드 — `Find()` → NOT_FILE/SPEC_OUT 알람
   - 실패 전파: 기존 스텝 패턴 `m_pShareItem->SetFail(true, <alarmID>)` (:6045 참조)
3. **알람 ID**: HMI `AlarmIDList.h`에 AGING 알람 부재 → 신규 3개 배정 + Alarm.xls 등록 필요 (ELP 8550 대역 선례, 빈 대역 확인 후)
4. **레시피명 접근자**: `GetRecipeEditMgrPtr()->GetRecipeInfo()`에 레시피 이름 필드 존재 여부 확인 필요 (TODO: AGING_CONFIRM)
5. **PG 미연동 영향**: `CCommTcpIpObjectPG`는 OutOfService 시 전 명령 성공 반환 + 스텝 내 가상 대기 루프(:5926-5945, iAgingTime초) 존재 → **PG 없이도 시퀀스는 통과**하지만 AGING.txt는 생성되지 않음. OOS에서 Post-hook 검증은 테스트 파일을 수동 배치하여 가능 (OCP도 OOS 시 7496 발생 구조 동일).

**테스트 픽스처 (2026-08-05 생성, PG 미연동 대응)**:
- 레지스트리 `DIR\AGING_FILE` = `D:\HBS\PARAM_OCP\PARAM\AGING.txt` (T: 드라이브 부재 대응; 삭제 시 기본값 T:\AGING.txt 복귀)
- `D:\HBS\PARAM_OCP\PARAM\AgingTargetRcp\TEST.rcp` — PTN3: 기준10±상2/하2 (스펙 8.0~12.0, USE=1), PTN5: USE=0
- `AGING_SpecIN.txt` (활성 `AGING.txt`로 복사됨): PTN3 셀값 10.5/9.8/11.2 → 무알람, PTN5 라인은 USE=0으로 스킵
- `AGING_SpecOUT.txt`: PTN3 셀값 10.5/**13.1**/11.2 → 기대 결과 `Aging Spec Out : Ptn(3(R)) Cell(2)`
- 파일 제거 시: 10초 대기 후 "Aging 파일을 찾을수 없습니다" 기대

## 5. 수정 파일 목록

| Phase | 파일 | 작업 |
|---|---|---|
| 1~3 | `Source/ALPHA/DlgSubManual_AgingSet.cpp` | Perl raw byte 편집 (한글 포함) |
| 1~3 | `Source/ALPHA/DlgSubManual_AgingSet.h` | 변경 불필요 예상 (멤버 완비) |
| 4 | DoWork AGING 시퀀스 파일 + AlarmIDList.h + Alarm.xls | 별도 승인 후 |

리소스(.rc)/resource.h/신규 파일: **없음** (UI 완비 확인됨)

## 6. 검증 계획

1. 빌드: Debug/Release x64, 경고 확인, `damaged=0` 인코딩 검증
2. ITEM 추가/제거/초기화: 행 증감, No 열 번호, 셀타입(체크/콤보/에디트) 동작
3. 저장 → `AgingTargetRcp\{이름}.rcp` INI 내용 확인 (MaxCount, ITEM_n의 USE_ALARM/PTN_RGB/BASE_VAL/UP_VAL/LO_VAL — OCP 저장 파일과 키 호환)
4. 이전값 불러오기: 저장 파일 재로드 시 그리드 복원 일치 (왕복 테스트)
5. START: 테스트용 `AGING.txt` (패턴별 CSV 라인) 작성 → Spec Out Cell 검출/정상 판정 확인
6. AGING ALARM USE 토글 → 레지스트리 반영, 재기동 후 유지
7. OCP 저장 `.rcp` 파일을 HMI에서 로드 (크로스 호환 확인 — 콤보 인덱스 저장 차이 검증 포함)

## 7. 리스크

| 리스크 | 완화 |
|---|---|
| CMySpreadDll col 0(행 헤더) 데이터 기록 불가 가능성 | B5에서 래퍼 소스 확인 후 대안(SS_HEADER 열 or No 컬럼 추가) 결정 |
| OCP `.rcp`의 PTN_RGB가 역순 콤보 인덱스로 저장된 기존 파일 | OCP 원본도 `SetTypeComboBoxCurSel(atoi(cValue))`로 로드 — 인덱스=20-값. 기존 OCP 파일을 HMI에서 쓸 경우 값 해석 차이 발생 가능 → 검증 6-7에서 실파일로 확인, 필요시 변환 |
| 한글 리터럴 인코딩 손상 | Perl raw byte + damaged=0 검증 + `.bak_agingset` 백업 |
| AgingTest 다이얼로그와의 유사 코드 동반 수정 유혹 | 범위 밖 — 본 플랜은 AgingSet만. AgingTest 결함은 별도 건 |
