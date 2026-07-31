# WAD Multi Point 측정 — HMI(ALPHA) 이식 플랜

- **작성일:** 2026-07-23
- **기준 분석 문서:** `D:\OCP\0.SW_Matching\docs\Dialog_Analysis_IDD_DLG_NAVI_MANUAL_MULTI_MEAS.md`
- **원본(OCP):** `CDlgNaviManualMultiMeas` + `JOB_Proc::DoWorkStartMeasOCP()` Multi 루프 + ELP Multi 프레임 + 결과 오프셋
- **진행 방식:** Phase 단위 점진 수정. **각 Phase 완료 시 빌드 + 동작 확인 후 다음 Phase 진행.**

---

## 0. 사전 확인된 현황 (2026-07-23 기준)

| 항목 | 상태 |
|---|---|
| `IDC_BUTTON_MANUAL_WAD_MULTI` (1032) | ✅ 사용자 추가 완료 — IDD_DLG_SUB_MANUAL에 "Multi Meas" 버튼 배치됨 (654,425,50,32) |
| `IDD_DLG_SUB_MANUAL_MULTI_MEAS` (220) | ⚠️ 템플릿 존재하나 **기본 POPUP 309×176 + 확인/취소 상태** → WS_CHILD 648×429로 재작성 필요 |
| 버튼 클릭 핸들러 | ❌ 없음 (Phase 1) |
| 다이얼로그 클래스 | ❌ 없음 (Phase 1) |
| 그리드 컨트롤 | ✅ **`CMySpreadDll` 사용 가능** — FarPoint DLL판(`CoreFunction/common/MySpreadDll.h`, FarPoint64), `new CMySpreadDll(id, uid, this)` 동적 생성. AgingSet(`DlgSubManual_AgingSet`)이 동일 용도로 사용 중 |
| WAD Multi 프로토콜 필드 | ❌ HMI `ELPPROTOCOL`(`Manager/CommTcpIpDefine.h:253`)에 `nMode/Degree/nTotalAngle/nCurrentAngle/nTotal/nCurrent` 없음 (구버전 프레임) |
| 시퀀스 통합 지점 | ✅ `CDoWorkGlassMeasureStep_Measure_WAD::Run()` (DoWorkGlassMeasureStep.cpp:1830) — 기존 `bOneCell`/`nOneCellCnt=5` 하드코딩 다점 루프 존재 (2052~) |

### 그리드 대안 결정 (권장)

사용자 요청은 "Text Control 대응"이었으나, **HMI에는 FPSpread OCX가 아닌 FarPoint DLL판 래퍼 `CMySpreadDll`이 이미 존재**하며 AgingSet이 같은 구조(리스트 편집 + INI 저장 + Offline 인터록)로 사용 중이다.

| 후보 | 장점 | 단점 |
|---|---|---|
| **A. CMySpreadDll (권장)** | OCP UI와 1:1 대응(셀타입/체크박스/헤더), .rc DLGINIT 불필요(코드 동적 생성), AgingSet 선례 존재 | FarPoint64 DLL 의존 지속 |
| B. CListCtrl(Report) + Edit/Combo | 표준 컨트롤, 의존성 없음 | 셀 인라인 편집/체크박스 직접 구현 필요, Cell 격자(체크 맵)에는 부적합 |
| C. Static/Edit 텍스트 나열 | 가장 단순 | 행 가변 리스트·격자 표현 불가, 20점+Cell맵 표현 non-viable |

→ **A안(CMySpreadDll)으로 진행** (Phase 1에서 최종 확정). B안은 FarPoint 제거 방침이 있을 경우의 대체안.

---

## 1. OCP → HMI 대응 매핑

| OCP | HMI 대응 | 비고 |
|---|---|---|
| `CDlgNaviManual` + `ChangeDlgMaint()` | `CDlgSubManual` + `m_iCurFileView`/`RefreshFileView()` | 동일 Show/Hide 구조 |
| `CDlgNaviManualMultiMeas` | **신규 `CDlgSubManual_MultiMeas`** | AgingSet 구조 복제 |
| FPSpread OCX 4개 | `CMySpreadDll` 3~4개 동적 생성 | DLGINIT 불필요 |
| `struct S_MultiMeas` (JOB_Proc.h) | **신규 `S_WadMultiPoint`** (WadMultiMeasMgr.h) | 필드 동일, vector 저장 |
| UI 다이얼로그를 JOB_Proc이 직접 접근 (안티패턴) | **신규 `CWadMultiMeasMgr`** (Manager) — 데이터/파일IO 전담, UI는 뷰만 | ResumeInfoMgr 선례 |
| `theApp.GetProfileString("DIR","MULTI_MEAS_RCP")` | `SystemConfigMgr` 키 + 기본값 **`D:\HBS\PARAM_OCP\PARAM\MultiMeasRcp\`** | 사용자 지정 경로 |
| `{RecipeID}.rcp` INI 스키마 ([DATA]/[MASECELL]) | **동일 스키마 유지** | OCP 파일 호환 |
| `m_aryMeasGroup[].m_iUsedMeas` bit4(+16) | `CWadMultiMeasMgr::IsMultiCell(iStick,iCell)` — [MASECELL] 자체 보관 | **레시피 구조체 무변경** (RCP_VERSION 불필요) |
| `DoWorkStartMeasOCP()` Multi 루프 (7827~) | `CDoWorkGlassMeasureStep_Measure_WAD::Run()` 내 통합 | 기존 bOneCell 루프 자리 |
| `DoWork_MoveXYZ`: X2 -=dPos_Y, Y1/Y2 +=dPos_X | `MoveCmdPosWAD(stPosition,...)`: 갠트리 `AXIS_X`(OCP모드)/`AXIS_X2` += -dPos_Y, 헤드 `AXIS_Y1/Y2` += dPos_X | 축 방향 부호 Phase 4에서 실측 검증 |
| `ExeMeasProperty()` ELP 0x36 len146 nTotal/nCurrent | `CCommTcpIpObjectWAD::SetCommand(ELPPROTOCOL)` 확장 | 장비 F/W 협의 필요 (Phase 5) |
| InnerID 점별 재발급 | `GenMeasID()` 재호출 or 점별 ID 부여 | Phase 4 |
| `GetMeasData→UpdateICPData(iType=1)` + RX/GX/BY 가산 | `CCommTcpIpObjectWAD::TossResultFile()` → `CResultMgr::Updata_Data()` 경로에 가산 | Phase 6 |
| `isOffLineStatus()` | AgingSet `IsOfflineStatus()` 패턴 재사용 | |
| Interest Glass N매 주기 옵션 | 보류 (필요 시 SystemConfigMgr 옵션) | 결정 항목 |

### 이식하며 교정할 OCP 원본 결함 (분석 문서 §14)

| # | 원본 결함 | 교정 방법 |
|---|---|---|
| 1 | `S_MultiMeas aData[20]` 고정 배열 오버플로 | `std::vector` 직접 push |
| 2 | INI 콤마병합 260자 버퍼 잘림 | 버퍼 4096 확대 (스키마는 유지) |
| 3 | `m_iUsedMeas += 16` 누적 오염 | 레시피 구조 미사용 — Mgr 내부 bool 맵 |
| 4 | `iArea > iAreaMax` 경계 오류 | `>=` 로 작성 |
| 7 | 워커 스레드의 OCX 접근 | **시퀀스는 Mgr 데이터만 접근**, UI 갱신은 PostMessage |
| 8 | `m_vMeasData` 무동기화 | Mgr에 `CCriticalSection` + 측정 중 Save 금지 인터록 |
| 9 | 점0 Enable 해제 시 기준행 누락 | i==0 처리를 Enable 검사보다 앞에 배치(사양 확인 후) |
| 10 | WX/WY 오프셋 죽은 설정 | UI/파일에는 유지, 적용은 RX/GX/BY만 (OCP 동일) — 주석 명기 |

---

## 2. Phase 계획

> 각 Phase는 독립 빌드/검증 가능. 수정 주석 `// LYS yyyy-mm-dd` 필수.
> **인코딩 주의:** 한글 포함 기존 .cpp/.h는 Perl raw byte 편집. **신규 파일은 ASCII(영문 주석)로 작성하면 Write/Edit 사용 가능.**
> **`ALPHA.rc`는 UTF-16LE** — Edit 도구 사용 금지, VS 리소스 편집기(사용자) 또는 UTF-16 인지 Perl 스크립트로만 수정.
> **`WindowEventHandler.h`는 pch 포함** — enum 추가 시 전체 리빌드 발생(1회로 몰아서 진행).

---

### Phase 1 — UI 스캐폴드 (버튼 → 다이얼로그 Show)

**목표:** Multi Meas 버튼 클릭 시 빈 다이얼로그가 기존 Manual 서브화면과 동일하게 표시/전환.

| 구분 | 파일 | 작업 |
|---|---|---|
| 수정 | `ALPHA.rc` (UTF-16LE) | `IDD_DLG_SUB_MANUAL_MULTI_MEAS` 템플릿 재작성: `DIALOGEX 0,0,648,429`, `STYLE DS_SETFONT | WS_CHILD`, FONT 9 굴림, IDOK/IDCANCEL 제거. 컨트롤 배치는 Phase 1에서는 GroupBox 골격만 |
| 수정 | `resource.h` | 신규 컨트롤 ID 예약: `IDC_EDIT_MM_RECIPE`, `IDC_BUTTON_MM_OPEN`, `IDC_EDIT_MM_RCP_PATH`, `IDC_BUTTON_MM_SAVE`, `IDC_BUTTON_MM_LIST_INIT`, `IDC_BUTTON_MM_ADD_ROW`, `IDC_BUTTON_MM_DEL_ROW`, `IDC_STATIC_MM_IMG`(추후 이미지), `IDC_FPS_MM_LIST/MODE/CELLSIZE/CELLMAP`(스프레드 앵커용 Static 또는 코드 생성 좌표) |
| 신규 | `DlgSubManual_MultiMeas.h/.cpp` | `CDlgSubManual_MultiMeas : CDialogEx` — AgingSet 구조 복제(생성자/OnInitDialog/UpdateStatus/IsOfflineStatus 골격). ASCII 작성 |
| 수정 | `WindowEventHandler.h` | `enumManualTemID`에 `enumManualTemID_WADMULTI` 추가 (**전체 리빌드**) |
| 수정 | `DlgSubManual.h` | 전방선언 + `CDlgSubManual_MultiMeas* m_pDlgSub_MultiMeas;` + 핸들러 선언 |
| 수정 | `DlgSubManual.cpp` | ① include ② `ON_BN_CLICKED(IDC_BUTTON_MANUAL_WAD_MULTI, OnBnClickedButtonManualWadMulti)` ③ OnInitDialog에서 `Create(IDD_DLG_SUB_MANUAL_MULTI_MEAS, this)` + MoveWindow(rtROI 기준, 기존과 동일) + SW_HIDE ④ `RefreshFileView()` hide 목록 + `case enumManualTemID_WADMULTI` ⑤ 핸들러: KMLOG + `m_iCurFileView = enumManualTemID_WADMULTI; RefreshFileView(TRUE);` ⑥ 소멸자 정리 |
| 수정 | `ALPHA.vcxproj(.filters)` | 신규 파일 등록 |

**검증:** Debug 빌드 → Manual 화면 → Multi Meas 클릭 → 빈 다이얼로그 표시, 타 버튼 전환 시 정상 숨김.

---

### Phase 2 — 데이터 계층 (Manager + Multi 레시피 파일 IO)

**목표:** UI 없이도 시퀀스가 사용할 수 있는 데이터 계층 완성. OCP INI 파일 포맷 호환.

| 구분 | 파일 | 작업 |
|---|---|---|
| 신규 | `Manager/WadMultiMeasMgr.h/.cpp` | `CWadMultiMeasMgr` — ResumeInfoMgr 패턴. 내용: `struct S_WadMultiPoint {CString strMainClass, strSubClass; double dPosX, dPosY; bool bEnable; double fOffsetRX, fOffsetGX, fOffsetBY, fOffsetWX, fOffsetWY;}` / `std::vector<S_WadMultiPoint> m_vecPoint` / `bool m_bInspMode` / `double m_dCellX, m_dCellY` / `std::vector<std::vector<int>> m_vecCellUse` ([MASECELL]) / `CCriticalSection m_cs` |
| | | API: `LoadMeasData(strRecipeID)`, `SaveMeasData(strRecipeID)`, `LoadMeasInfo/SaveMeasInfo`, `MeasCellLoad/Save`, `GetUseSize()`, `GetCheckNumData(n)`, `IsMultiCell(iStick,iCell)`, `IsMultiMode()`, `GetPointCount()`, `GetPoint(i)` |
| | | INI 스키마 OCP 동일: `[DATA] CLASS,SUB_CLASS,X,Y,ENABLE,RX,GX,BY,WX,WY,INSP_MODE,CELL_X,CELL_Y` + `[MASECELL] STICK_n`. 버퍼 4096, vector 파싱(§1 결함 교정 1/2/3/8 적용) |
| | | 경로: `SystemConfigMgr`에 `[USER_OPTION] WAD_MULTI_MEAS_RCP_PATH` 키 신설, 기본값 `D:\HBS\PARAM_OCP\PARAM\MultiMeasRcp\` |
| 수정 | `Manager/SystemConfigMgr.cpp/.h` | 경로 키 + `IsUseWadMultiMeas()` 사용 여부 플래그(선택) |
| 수정 | `AuroraMgrPool.h` / `AuroraMgr.cpp` | Mgr 생성/등록/해제 (ResumeInfoMgr과 동일 위치) |
| 수정 | `BaseSequenceImpl.h`(또는 MgrPool 접근 경유) | `GetWadMultiMeasMgrPtr()` 접근자 |

**검증:** 단독 테스트 — 임시 코드로 Load→Save 라운드트립, OCP에서 만든 .rcp 파일 로드 확인 (수동으로 파일 복사).

---

### Phase 3 — 다이얼로그 기능 구현 (편집/저장/Cell 격자)

**목표:** OCP 다이얼로그 기능 전체를 HMI UI로 재현.

**UI 배치 (648×429, OCP 663×435 대응):**

```
[Recipe 그룹 (상단)]
  "Recipe :" + IDC_EDIT_MM_RECIPE(ReadOnly) + [레시피 열기] IDC_BUTTON_MM_OPEN
  "WAD Multi Recipe 경로 :" + IDC_EDIT_MM_RCP_PATH(ReadOnly)
[Cell 격자 그룹 "WAD Multi 대상 Cell"] — CMySpreadDll #4 (stick×cell 체크박스 맵, 헤더 더블클릭 일괄토글)
[검사 설정 그룹] — CMySpreadDll #2 (1×2: "Single(0)/Multi(1)" | 값)
[Cell Size 그룹] — CMySpreadDll #3 (1×2: CELL_X | CELL_Y)
[측정 점 리스트 그룹 "WAD Multi Point 설정"] — CMySpreadDll #1
  11열: NO|CLASS|SUB_CLASS|X|Y|ENABLE(체크)|RX|GX|BY|WX|WY
[IDC_STATIC_MM_IMG] — 좌표계 안내 이미지 자리(추후 리소스 추가 예정, Static 예약만)
[버튼] 리스트 초기화 | 저장 | 추가 | 제거
```

| 구분 | 파일 | 작업 |
|---|---|---|
| 수정 | `DlgSubManual_MultiMeas.cpp/.h` | ① OnInitDialog: `new CMySpreadDll(IDC_FPS_MM_*, FPS_RESOURCE_ID + <미사용 uid>, this, TRUE)` ×4 — **uid는 전 프로젝트 grep으로 미사용 번호 확정**(AgingSet=+19) ② 버튼 5종 핸들러 (OCP 로직 이식: Open→Mgr.Load→그리드 표시 / Save→CellSize 저장→`IsCellRangeCheck()`(±Cell/2)→Mgr.Save / 초기화 / 행 추가·제거) ③ 그리드 이벤트(클릭 행선택, 격자 헤더 더블클릭 일괄토글 — CMySpreadDll 이벤트 메시지 사용) ④ `IsOfflineStatus()` 인터록 (AgingSet 패턴) |
| | | **Open 동작 (OCP 대비 단순화):** OCP는 메인 레시피 LoadPPID2까지 수행했으나, HMI는 현재 로드된 레시피(`RecipeEditMgr::GetRecipeID()`) 기준으로 Multi 파일만 Open/Save (메인 레시피 로드는 DlgSubRecipe 소관 — 결정 항목 D-2) |
| | | Cell 격자 크기: `RecipeEditMgr::GetRecipeData()->m_vecStick.size()` × `m_vecCell.size()`, 회전(`m_nGlsRotate`) 반영 (SelectCellDlg 참고) |

**검증:** 화면에서 점 5개 입력→저장→재시작→로드 유지, Cell 범위 초과 저장 거부, Offline 아닐 때 저장 거부.

---

### Phase 4 — 측정 시퀀스 통합 (핵심)

**목표:** Auto 측정에서 Multi 대상 Cell에 대해 점별 이동+WAD 측정 반복.

**통합 지점:** `CDoWorkGlassMeasureStep_Measure_WAD::Run()` (DoWorkGlassMeasureStep.cpp:1830~)

```
[기존 흐름]  Stick 루프 → 2Head 그룹핑 → (k: bOneCell 5점) → 이동(MoveCmdPosWAD) → PG 설정
             → InspectStart(WAD) → InspectEnd/Complate 대기 → TossResultFile

[추가 흐름]  기준(1차) 측정 완료 후, bMultiMode && IsMultiCell(해당 Cell) 이면:
  for (각 Enable 점 i≥1):
      STOP/PAUSE 체크 (IfPauseEvt_WaitResumeEvt / GetStopUser)
      InnerID 재발급 (GenMeasID 상당 — 점별 결과 파일 분리)
      stPosition: AXIS_Y1/Y2 = 기준 + dPosX (미측정 probe는 0)
                  갠트리(OCP: AXIS_X / HMI: AXIS_X2) = 기준 - dPosY   ← 부호 실측 검증
                  Z 유지
      MoveCmdPosWAD(...)
      PG 번호 재설정 (기존 PG_SET 로직 재사용)
      DoWorkInspectStart(WAD, iMeasMode, ..., nCurrent=i+1)   ← Phase 5 전까지는 기존 프레임
      DoWorkInspectEnd / InspectComplate 대기
      nCellStatus 갱신
```

| 구분 | 파일 | 작업 |
|---|---|---|
| 수정 | `DoWork/DoWorkGlassMeasureStep.cpp` | 위 루프 추가. **기존 `bOneCell`(nArea==1 시 5점 하드코딩) 루프와의 관계는 결정 항목 D-1** — 권장: OCP 모드에서 Multi 레시피 존재 시 bOneCell 비활성 + Multi 루프 사용 |
| 수정 | 동일 | Multi 게이트: `GetWadMultiMeasMgrPtr()->LoadMeasData(RecipeID)` (Stick 루프 진입 전 1회) → `IsMultiMode()` && 점수>0 |
| 수정 | `Manager/CommTcpIpFactoryMgr.cpp` | `DoWorkInspectStart(WAD)` 경로에 nTotal/nCurrent 전달 인자 추가(기본값 0 — 기존 호출 무영향) |
| 참고 | 각도(ANGLE) 연동 | OCP는 점별 각도 시퀀스(GetAngleInspCnt_S) 수행 — HMI ANGLE은 별도 Step(`Measure_ANGLE`)이므로 **1차 범위 제외**, WAD 단일 각도만 (결정 항목 D-3) |

**검증:** OutOfService 시뮬레이션으로 점 3개 레시피 → 로그에서 점별 Move/InspectStart/InnerID 발급 순서 확인. 실장비 전 모션 좌표 dry-run(모터 로그 좌표 검산).

---

### Phase 5 — WAD 프로토콜 Multi 확장 (장비 협의 필요)

**목표:** WAD 장비에 총 점수/현재 점 번호 통지 (OCP: cmd 0x36, len 144→146, nTotal/nCurrent).

| 구분 | 파일 | 작업 |
|---|---|---|
| 수정 | `Manager/CommTcpIpDefine.h` | `ELPPROTOCOL`에 `int nTotal; int nCurrent;` 추가 (**구조체 끝**, 기존 필드 순서 불변) |
| 수정 | `CommTcpIpObjectWAD.cpp` | `SetCommand()` 송신 프레임 빌드: Multi 시 len 146 + 필드 직렬화 (OCP `ExeMeasProperty` case 0/1 참조) |
| 선행 | — | **WAD 장비 F/W가 Multi 프레임(len 146) 수신 가능한지 확인** — 불가 시 이 Phase 보류하고 기존 프레임으로 점별 개별 측정(파일명으로 구분)만 운용 |

**검증:** 장비 시뮬레이터 or 실장비 패킷 캡처로 len/필드 확인, 기존 Single 측정 무영향(회귀) 확인.

---

### Phase 6 — 결과 데이터 오프셋 적용

**목표:** 점별 결과(.dat)의 색좌표에 R_X/G_X/B_Y 오프셋 가산 (OCP `UpdateICPData` iType=1 로직).

| 구분 | 파일 | 작업 |
|---|---|---|
| 수정 | `CommTcpIpObjectWAD.cpp` `TossResultFile()` | RawData 복사 후 `resultData`에 Multi 점 번호 부여(현재 진행 점 카운터는 Mgr 또는 FactoryMgr 보관) |
| 수정 | `Manager/ResultMgr` (`Updata_Data` WAD 분기) 또는 결과 표시 계층 | 점 번호 ≥2 이면 `GetCheckNumData(nCurrent)`로 해당 점 조회 → R_X += fOffsetRX, G_X += fOffsetGX, B_Y += fOffsetBY (WX/WY는 OCP와 동일하게 미적용, 주석 명기) |
| 결정 | — | **가산 위치 결정(D-4):** 원본 .dat 보존 + 표시/전송 값만 보정(OCP 방식, 권장) vs .dat 자체 수정 |

**검증:** 오프셋 0.01 지정 후 결과 그리드 값이 원본 .dat 대비 정확히 가산되는지 비교.

---

### Phase 7 — 통합 검증 및 마무리

1. **시뮬레이션(OUT_OF_SERVICE):** Multi ON/OFF × Cell 선택 유/무 × 점 Enable 조합 회귀
2. **실장비:** 좌표 이동 실측(Cell 이탈 인터록 ±Cell/2 확인), 점별 결과 파일 생성/오프셋 확인
3. Single 모드(기존 흐름) 회귀 — Multi 파일 없을 때 기존과 완전 동일 동작
4. Resume 검사와의 상호작용 확인 (Multi 점 중간 중단 시 재개 정책 — 최소: Stick 단위 재개 유지)
5. 문서화: `docs/OCP_Option_ChangeHistory.md`에 추가, CLAUDE.md 갱신

---

## 3. 결정 필요 항목 (Phase 착수 전/중 확인)

| ID | 항목 | 권장안 |
|---|---|---|
| D-1 | 기존 `bOneCell` 5점 하드코딩 루프와 신규 Multi 루프 관계 | Multi 레시피 존재+Multi모드 시 bOneCell 대체(비활성), 아니면 기존 유지 |
| D-2 | 다이얼로그 [레시피 열기]가 메인 레시피까지 로드할지(OCP 방식) | HMI에서는 현재 로드된 레시피 기준으로 Multi 파일만 관리 (레시피 로드는 DlgSubRecipe 소관) |
| D-3 | 점별 각도(0/15/30/45/60°) 반복 포함 여부 | 1차 제외 (WAD 단일 각도), 필요 시 후속 Phase |
| D-4 | 오프셋 가산 위치 | 원본 .dat 보존, 표시/집계 단계 가산 (OCP 동일) |
| D-5 | 그리드 컨트롤 | CMySpreadDll (A안) |
| D-6 | WAD 장비 F/W Multi 프레임(len 146) 지원 여부 | 장비 담당 확인 후 Phase 5 진행/보류 |

## 3-1. Phase 4b — Multi 게이트를 레시피 옵션(m_bUseSpec_Multi)으로 전환 (2026-07-23 추가)

**요구:** `SystemConfig.ini [OPT_PARA] USE_WAD_MULTI_MEAS` 설비 설정 대신, RecipeMgrDlg의 검사항목 `m_bUseSpec_Multi` 체크로 Multi 측정 여부를 결정.

**현황 조사 결과 — `m_bUseSpec_Multi`는 이미 완전 배선됨 (LYS 2026-07-14):**

| 구성 요소 | 상태 |
|---|---|
| `S_InspectItem.m_bUseSpec_Multi` (preDefine.h **양쪽** App/DLL) | ✅ 존재 (m_bUseEFA와 함께 추가) |
| RecipeMgrDlg 검사항목 그리드 UI (행7, 콤보 0/1) | ✅ SetItemCurSel/GetItemCurSel(7,2) 연동 |
| Save (RecipeMgrDlg.cpp:461 WriteInt) | ✅ |
| Load (InRecipeFunc.cpp:164 / OutRecipeFunc.cpp:147 ReadInt) | ✅ 3-파일 순서 동기 |

**수정 내용 (1개소):**

`DoWorkGlassMeasureStep.cpp` — Measure_WAD Multi 게이트 조건 교체:

```cpp
// 변경 전
if (pMultiMgr != nullptr && m_pShareItem->GetSystemConfigMgrPtr()->IsUseWadMultiMeas())
// 변경 후 (레시피 검사항목으로 판정)
if (pMultiMgr != nullptr && pReciepInfo != nullptr && pReciepInfo->m_sInspectItem.m_bUseSpec_Multi == TRUE)
```

- `pReciepInfo`는 Run() 상단에서 이미 취득된 함수 스코프 변수 — 추가 조회 불필요
- Multi 레시피 파일의 `INSP_MODE`(다이얼로그 검사 설정)와 AND 관계 유지: 레시피 옵션 ON + Multi 파일 INSP_MODE=1 + Enable 점 존재 시에만 동작

**부수 정리:**
- `SystemConfigMgr`의 `USE_WAD_MULTI_MEAS` 키/`IsUseWadMultiMeas()`: 참조처가 이 게이트 1곳뿐 → **키·멤버·getter 제거** (죽은 설정 방지). `WAD_MULTI_RCP_PATH` 키는 유지(경로는 계속 필요)
- RecipeMgrDlg 그리드 행7 라벨 텍스트가 "특성 Multi" 등으로 표시되는지 확인(라벨만 별도 초기화 함수에 있음)

**리스크 메모:** 2026-07-14 추가 당시 EFA/Spec_Multi가 파일 **중간**(InspectionItem 블록)에 버전 가드 없이 삽입됨 — CLAUDE.md의 "신규 필드는 파일 끝 + 버전 가드" 규칙과 다름. 2026-07-14 이전 저장 레시피 로드 시 필드 밀림 가능성 있으므로 구버전 레시피 재저장 필요 여부 별도 확인 권장(본 수정 범위 밖, 기존 이슈).

## 4. 리스크

- `WindowEventHandler.h` enum 추가 → **전체 리빌드 1회** (Phase 1에 포함, 다른 pch 변경과 병합 금지)
- `ALPHA.rc` UTF-16LE — 도구 편집 실수 시 리소스 전체 손상. 수정 전 백업 필수
- `CommTcpIpFactoryMgr.cpp`(~26,000줄) 수정은 최소 diff로 (라인 범위 읽기 규칙 준수)
- `ELPPROTOCOL`은 LCR/TSP도 공유(`SetCommand(ELPPROTOCOL)`) — 필드는 반드시 끝에 추가, len은 WAD 송신부에서만 확장
- 측정 중 다이얼로그 저장 → Mgr critical section + 시퀀스 Run 중 Save 거부 인터록

## 5. Phase별 산출물 요약

| Phase | 신규 파일 | 수정 파일 | 리빌드 규모 |
|---|---|---|---|
| 1 | DlgSubManual_MultiMeas.h/.cpp | ALPHA.rc, resource.h, WindowEventHandler.h, DlgSubManual.h/.cpp, vcxproj | 전체(1회) |
| 2 | Manager/WadMultiMeasMgr.h/.cpp | SystemConfigMgr, AuroraMgrPool.h, AuroraMgr.cpp | 중간 |
| 3 | — | DlgSubManual_MultiMeas.h/.cpp | 소 |
| 4 | — | DoWorkGlassMeasureStep.cpp, CommTcpIpFactoryMgr.cpp/.h | 소 |
| 5 | — | CommTcpIpDefine.h, CommTcpIpObjectWAD.cpp | 중간 |
| 6 | — | CommTcpIpObjectWAD.cpp, ResultMgr | 소 |
| 7 | 문서 | — | — |
