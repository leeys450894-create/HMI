# AutoContact 다이얼로그 비교 및 이식 플랜 (OCP IDD_DLG_NAVI_MANUAL_AUTO_CONTACT → HMI IDD_DLG_SUB_MANUAL_AUTO_CONTACT)

작성: 2026-08-10 (LYS)
선행 이식 선례: AgingSet_Porting_Plan.md, AgingTest_Porting_Plan.md (스텁 완성형 이식 — 본 건과 동일 유형)

---

## 1. 목적 및 범위

**Probe 자동 컨택(Auto Contact)** = 프로브 유닛을 단계적으로 하강시키며 PG R-Test 피드백으로
컨택 위치(PreContact)와 OD(Over Drive)량을 자동 탐색·저장하고, 이후 측정부터는 저장된 위치로
바로 컨택하는 기능. 수동 티칭 없이 프로브 교체/모델 교체 후 컨택 높이를 재확보하는 것이 목적.

- **OCP (원본)**: `CDlgNaviManualAutoContact` (508줄) — 완전 동작. JOB_Proc의 자동 컨택 시퀀스
  (`DoWorkAutoContact`/`DoWorkAutoContactDataMove`/`DoWorkPgDownload`)와 연동.
- **HMI (현재)**: `CDlgSubManual_AutoContact` (542줄) — **스텁 상태**. 동일 골격이 복제되어 있으나
  기능 연결부가 전부 주석 처리 + 이식 결함 존재. UI 껍데기(그리드/체크버튼)만 표시됨.

이식 유형은 AgingSet/AgingTest와 동일: **기존 스텁 다이얼로그를 완성**하고, 시퀀스 엔진 측
자동 컨택 로직을 HMI 구조(DoWork Step + Manager)로 신설한다.

---

## 2. 현황 비교 (2026-08-10 검증)

### 2.1 다이얼로그 기능 요소별 비교

| 기능 요소 | OCP (DlgNaviManualAutoContact) | HMI (DlgSubManual_AutoContact) 현재 상태 |
|-----------|-------------------------------|------------------------------------------|
| 파라미터 그리드 | FPSpread, col2/row2.. (8항목) | ✅ CMySpreadDll `InitMySpread_AUTOCONTACT()` 기구현 (1col×8row, 행헤더 = AUTO_CT_ 순서 일치) — 단 **값 로드가 주석 처리**되어 항상 빈 값 |
| 파라미터 로드/저장 | Config INI `[PROBE_AUTO_CONTACT]` (`gf_GetCnfPath`) → 공유메모리 반영 | ⚠ 키 이름/구조 동일하나 **`m_strIniPath` 미초기화(빈 문자열)** → win.ini로 저장됨. 공유메모리 반영 전부 주석 |
| 사용 플래그 4종 (Use/ModelChange/ProbeChange/Always) | 체크버튼 + INI 저장 | ✅ UI/INI 저장 동작 (경로만 잘못됨) |
| 탐색 결과 보관 (Ready/PreContact/OD, RcpID/ProbeID/Success) | INI 키 ContactData* | ⚠ 로드 시 **`_ttoi` 사용 → double 소수부 절삭 버그** (OCP는 atof) |
| 저장 검증 (Limit>Ready, FGAP≤0.5, SGAP≤0.1, ODMax≤1.0, AddOD≤0.5, %=0~100) | 동작 | ⚠ 골격 있으나 `fTemp[i] = _ttoi(...)` → 소수 입력 검증 불능 |
| Offline 게이트 | `stTitle.m_nHostMode != df_HOST_OFFLINE` 차단 | ❌ **항상 TRUE** (주석 처리) — 게이트 없음 |
| Recipe Open | `LoadPPID2()` 실제 레시피 로드 | ❌ 로드 없이 **무조건 "imported complete" 표시** (가짜 성공) |
| STOP 버튼 | `m_pJobProc->ActionStop()` | ❌ 빈 함수 |
| Auto Contact 시작 | `DoStartAutoContact()` → WORK_AUTO_CONTACT | ❌ 전부 주석 — **시퀀스 자체가 HMI에 없음** |
| PG Recipe Download | `DoStartPgDownload()` → WORK_PG_DOWNLOAD | ❌ 전부 주석 |
| 상태 표시 (UpdateStatus) | AXIS_UNIT_Z 현재위치 + 탐색 결과 | ⚠ 축 표시 주석, 결과 문자열만 |
| 공유메모리 연동 | `stInterLock.m_fAutoContactData[8]` / `nAutoContactStatus[30]` / `m_fProbeOD` | ❌ **HMI VTM_SYSTEM에 해당 멤버 없음** (defines만 CommonFunc.h:2071에 존재) |

### 2.2 시퀀스 연동 비교 (OCP에만 존재 — HMI 이식 대상)

| 연동 지점 | OCP 구현 | HMI 대응 (이식 목표) |
|-----------|----------|---------------------|
| 자동 컨택 탐색 | `DoWorkAutoContact()` (JOB_Proc:32779) | 신설 필요 |
| 저장 위치 재사용 컨택 | `DoWorkAutoContactDataMove()` (:32899) | 신설 필요 |
| 측정 시 분기 | Contact 시점: Use&&Success→DataMove / Use&&!Success→탐색 / !Use→일반 컨택 (:6588, :13250) | `CDoWorkGlassPreMeasStep_Contact::Run()` 분기 추가 |
| 리셋 판정 | 측정 시작 시: Always→매회 리셋 / ModelChange→RcpID 변경 시 / ProbeChange→ProbeCardID 변경 시 (:5943, :12604) | 측정 시퀀스 시작부 (Recipe 스텝) |
| R-Test 피드백 | `ExeSubMeas(-1,2)` → PG 결과 문자열 `CalcPGResult(2,-1)` → `nAutoContactStatus[cell]` = 숫자+20 (**21=OK, 28=신호선불량**) | PG `enumTCPIpFactoryt_PG_TEST_3/13` (R-TEST 명령 기존재) + 결과 파싱 신설 |
| PG Download | `ExePGCtrl(5→6→2→0→1)` (ModelOpen→CellCnt→Recipe→Download→Wait) | `PGEvent_0`(Model Change)/`PGEvent_1`(Wait)/`PGEvent_2`(Recipe Change) 매핑 — 5/6 대응 확인 필요 |
| 성공 시 로그 | `SetAutoCntLog()` — ProbeID 섹션별 Ready/ContactPos/OD 기록 | 경로만 HMI로 변경해 유지 |

### 2.3 OCP 자동 컨택 알고리즘 (DoWorkAutoContact 요지)

```
① Ready 위치로 Z 이동 (AXIS_UNIT_Z)
② 1차 탐색: R-Test 실행 → 셀 중 1개라도 OK(21)면 성공
   실패 시 Z += FirstMoveGap 하강 반복 (FirstLimitPos 초과 → 1STEP_FAIL 알람)
   포트불량(28) 셀 수 > PortFail 설정 → PORT_FAIL 알람
   → 성공 위치 = PreContact
③ 2차 탐색(OD): R-Test OK 셀 비율 ≥ Percent% 될 때까지 Z += SecondMoveGap
   (ODMax 초과 → 2STEP_FAIL 알람) → 성공 시 OD량 확정
④ 최종 컨택: PreContact + OD + AddOD 위치로 이동, m_fProbeOD 갱신
⑤ 결과 저장: 기존 저장값보다 깊을 때만 SaveAutoCntPos (마지막 스틱 성공 시 Success=TRUE + 로그)
* 셀 수 = 레시피 CellCount (rotate에 따라 x/y 선택), 각 단계 STOP 체크
```

---

## 3. HMI 대응 구조 매핑

| 항목 | OCP | HMI |
|------|-----|-----|
| 컨택 축 | `AXIS_UNIT_Z` (단일 프로브 유닛 Z) | **OCP 모드: `AXIS_PROBE_Z5`** (P5 단독) / HMI 모드: 21(전 프로브) — 기존 `MoveCmdPos_ProbeContact()` 사용 축과 동일 |
| 컨택 위치 원천 | AutoContact 저장값 (INI) | 레시피 `m_sContactPos` {PreContactPos, EndContactPos, AddOD} — **자동 컨택은 이 값을 대체(override)하는 구조로 설계** |
| 일반 컨택 실행 | `ExeSubMeas(iTop, 10)` | `MaintControlMgr::MoveCmdPos_ProbeContact(축, dfMove, dEMove, dOD)` (`DoWorkGlassPreMeasStep_Contact`:1056) |
| R-Test 명령 | PG `ExeSubMeas(-1, 2)` | PG `DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_3)` (Ensis PG는 _13) — **명령 기존재, 결과 회수 경로 확인 필요** |
| 파라미터/상태 저장 | `Config/{EQType}/Config.ini` | `Source/Bin/Config/Config.ini` `[PROBE_AUTO_CONTACT]` (키 이름 OCP 동일 유지) — `g_sAppPath + "Config\\Config.ini"` |
| AUTO_CT_* defines | `Include/CommonFunc.h:192~` | ✅ `CoreFunction/common/CommonFunc.h:2071~` 동일 값 기존재 |
| 공유메모리 | `stInterLock.m_fAutoContactData/nAutoContactStatus/m_fProbeOD` | 부재 — **비-UI 헬퍼 내부 보관으로 대체** (VTM_SYSTEM 수정 회피). 타 프로세스가 참조해야 할 경우에만 struct 끝 추가 |
| ProbeCardID | `stProbeInfo.m_szProbeCardID` (PLC ProbeCard 스캐너) | HMI ProbeCard 스캐너/ProbeHistory 존재 — ID 취득 경로 확인 필요 (C3) |
| Offline 판정 | `stTitle.m_nHostMode` | `GetShareMemoryMgrPtr()->GetSys_MachineMode() != df_HOST_OFFLINE` (AgingSet 선례) |
| 알람 | `PROBE_AUTO_CNT_PORT_FAIL/1STEP_FAIL/2STEP_FAIL` | AlarmIDList.h 신규 등록 (OCP와 동일 ID 원칙 — AgingSet 7495~7497 선례) |
| 다이얼로그 진입 | DlgNaviManual BTN | ✅ `IDC_BUTTON_MANUAL_5` → `enumManualTemID_AUTOCONTACT` 기배선 (DlgSubManual.cpp:263) |

---

## 4. 핵심 설계 결정

### 4.1 시퀀스는 다이얼로그에 접근하지 않는다 (AgingCycleInfo/AngleInspInfo와 동일 원칙)
OCP는 JOB_Proc가 다이얼로그 getter를 직접 호출하지만, HMI에서는 비-UI 보관소를 신설한다:
- `DoWork/AutoContactInfo.h/.cpp` (또는 Manager/) — 파라미터 8종 + 상태(Ready/PreContact/OD, RcpID/ProbeID/Success) + 플래그 4종의 로드/저장/리셋 + R-Test 셀 상태 배열(`nAutoContactStatus[RTEST_MAX]` 대체)
- 다이얼로그는 편집/저장 UI만, 시퀀스(PreMeas Contact 스텝)는 AutoContactInfo만 참조 — INI 파일이 단일 계약

### 4.2 컨택 위치 적용 방식 — 레시피 비침습 override
- HMI Contact 스텝은 레시피 `m_sContactPos`를 사용 중. 자동 컨택 활성+성공 상태면
  **스텝에서 AutoContactInfo의 저장값으로 인자만 교체** (레시피 파일은 건드리지 않음 → 버전 변경 없음)
- `!Success`면 탐색 시퀀스 수행 후 저장 (OCP 흐름 동일)

### 4.3 탐색 루프의 위치 — Contact 스텝 내부 서브루틴
- OCP처럼 별도 WORK 커맨드가 아니라, `CDoWorkGlassPreMeasStep_Contact::Run()` 내 분기 + 헬퍼 함수로 구현
  (HMI 시퀀스 구조상 스텝 내에서 이동/통신/STOP 체크 모두 가능 — WAD MultiPoint 루프 선례)
- 다이얼로그 START 버튼(수동 기동)은 AgingTest P4 선례처럼 **사이클 검사 경로가 아닌 단독 실행**이 필요 —
  Manual 계열 워커 스레드(DlgSubManual_Meas 수동 측정 선례) 또는 시퀀스 1스텝 실행으로 연결 (C5에서 확정)
- STOP 버튼 = 해당 실행 경로의 Stop 이벤트 (OCP ActionStop 대응)

### 4.4 R-Test 결과 회수
- 명령은 `PG_TEST_3`(R-TEST) 기존재. OCP `CalcPGResult(2,-1)` 대응 — PG 응답의 셀별 숫자 문자열을
  **+20 오프셋 상태(21=OK, 28=신호선불량)** 로 파싱해 AutoContactInfo 배열에 기록
- HMI PG 수신 객체(CommTcpIpObjectPG 계열)의 R-Test 응답 파싱 현황 확인 후, 부재 시 파싱 추가 (C2)
- 셀 수 = `pRecipeData->m_vecStick[iStick].m_vecCell.size()` 기준 (OCP CellCount.x/y 대응 —
  Glass 방향 신규 체계에서는 rotate 분기 불필요, 스틱 셀 수 직접 사용)

### 4.5 스텁 버그 수정 (기능 연결과 무관하게 선행 수리)
1. `_ttoi` → `_tstof` (double 절삭 — GetAutoContactData 3곳 + OnAutoContactSave 1곳)
2. `m_strIniPath`/`m_strAutoLogPath` 초기화 (빈 경로 → win.ini 오염 중)
3. IsOfflineStatus 게이트 복원
4. OnRcpOpen 가짜 성공 제거 — `RecipeChange()` 사용 (AgingTest P3 선례: OpenRecipe는 SetProbe 부수효과 회피)
5. UpdateStatus 축 표시 = OCP 모드 `AXIS_PROBE_Z5` 현재위치
6. 파라미터 그리드 값 로드 복원 (CMySpreadDll 좌표계: 데이터 (1, row 1..8) — InitMySpread_AUTOCONTACT 배치 확인 완료)
7. 신규 사용자 메시지는 영어 (인코딩 규칙 7 — 이 파일 한글 리터럴 인코딩 확인 후 기존 문자열은 유지)

### 4.6 PG Recipe Download 버튼
- OCP `ExePGCtrl(5→6→2→0→1)` ↔ HMI `PGEvent_0/1/2` 매핑으로 구현.
  Model Open(5)/Cell Count(6)의 HMI 대응 명령 존재 여부 확인 후 필요 시 생략/통합 (C4)

---

## 5. 실행 Phase

### Phase 1 — 스텁 수리 (기능 연결 전 정합화) ✅ 구현 완료 (2026-08-10, 빌드 OK)
- §4.5 항목 1~7 수리 + 경로/게이트 복원 — 적용 내역:
  1. `_ttoi`→`_tstof` 4곳 수정 (GetAutoContactData 3 + OnAutoContactSave 1)
  2. `m_strIniPath` = `{ConfigFolderPath}\Config.ini`, `m_strAutoLogPath` = `{ConfigFolderPath}\AutoContactLog.ini` (C7)
  3. ctor `CAuroraMgr*` 주입 (AgingSet 패턴) + DlgSubManual.cpp 생성부 반영
  4. IsOfflineStatus 게이트 복원 (`GetSys_MachineMode() != df_HOST_OFFLINE`, 신규 메시지 영어)
  5. OnRcpOpen 가짜 성공 제거 → `RecipeEditMgr::RecipeChange()` + EDIT_1 = `GetRecipeID()` (AgingTest 패턴)
  6. 파라미터 8종 로컬 배열 `m_fAutoContactParam[8]` 보관 + 그리드 (1, row1..8) 로드/저장 왕복 (콤보 행 text=값)
  7. UpdateStatus Z 표시 = `AXIS_PROBE_Z5` (C1) + `SetTimer(1,500ms)`/OnTimer 주기 갱신, OnInitDialog에서 현재 레시피 ID 표시
- 백업: `.bak_autocnt` 3파일. 인코딩 검증 damaged=0. 빌드 에러 0
- 잔여 확인(런타임): 파라미터 편집→저장→재로드 왕복 일치, Config.ini [PROBE_AUTO_CONTACT] 생성 확인

### Phase 2 — AutoContactInfo 비-UI 헬퍼 신설 ✅ 구현 완료 (2026-08-10, 빌드 OK)
- `DoWork/AutoContactInfo.h/.cpp` 신설 (싱글톤 GetInstance/DelInstance) — 파라미터 8종/플래그 4종/상태(Ready·PreContact·OD·RcpID·ProbeID·Success) 로드·저장·리셋 + R-Test 셀 상태 배열(`RTEST_MAX`, P3용) + `SaveLog()`(ProbeID 섹션 이력) + `CheckAndReset()`(OCP JOB_Proc 리셋 판정 이식)
- 리셋 판정 호출부: `CDoWorkGlassRecipeStep_ProbeCheck::Run()` 말미 (성공 경로) — RecipeID = `RecipeEditMgr::GetRecipeID()`, ProbeCardID = 공유메모리 `m_szPROBEID[4]`(P5, **CoreFunction/ShareMemFunction/ShareMem.h:2330** — PLC ProbeCard 스캐너가 기록, C3)
- **재배선 설계 확정: INI = 단일 계약** — 다이얼로그 쓰기는 기존 INI 직접 기록 유지, 시퀀스는 헬퍼(LoadAll 후 사용), 다이얼로그 타이머는 **파일 직접 재독** (UI 스레드가 공유 싱글톤을 변조하지 않도록 — 스레드 안전). UpdateStatus가 ContactDataReady/Pos1/Pos2/Sucess를 500ms마다 갱신 표시
- vcxproj/filters 등록 (AgingCycleInfo 블록 복제 방식). 백업 `.bak_autocnt` (RecipeStep/vcxproj/filters) + `.bak_autocnt_p2` (다이얼로그 cpp)
- 잔여 확인(런타임/OOS): 플래그별 리셋 단위 동작 (Always 매회 / ModelChange 레시피 변경 시 / ProbeChange 프로브 변경 시), 시퀀스 기동 시 ProbeCheck 스텝에서 리셋 로그

### Phase 3 — R-Test 피드백 경로 ✅ 구현 완료 (2026-08-10, 빌드 OK)
- **실사 결과**: HMI `CCommTcpIpObjectPG::WaitInspectionComplete(timeout, 7)`이 이미 R-Test 응답을 셀별 파싱 중
  (`m_strPGResult` 숫자 문자열 → 0=OVER/1=PASS/2=UNDER → `nCellStatus[cell][stick]`). 단 switch가 0/1/2만
  처리해 **8(신호선불량) 등 기타 숫자는 버려짐** → 자동 컨택용 raw 캡처를 별도 추가
- **호출 체인 확인**: `DoWorkInspectStart(PG, PG_TEST_3)` 1회 호출 = 송신(S8/F1/C7) + 대기 + 파싱까지 동기 완결
  (`CommTcpIpObjectPG.cpp:2212` DoWorkEvent → SetCommand → WaitInspectionComplete(timeout,7)) — P4 탐색 루프는 이 1회 호출만 하면 됨.
  타임아웃 = 레시피 `m_nTimeOut[1]`, ALL 셀 = iCellNum 0
- 구현 (`CommTcpIpObjectPG.cpp`, 백업 `.bak_autocnt`):
  - 실수신 분기(iMeasType==7): `m_strPGResult` 각 자릿수 + 20 → `CAutoContactInfo::SetCellStatus()` (OCP CalcPGResult(2,-1) 규약 — 21=PASS, 28=신호선불량, 기존 nCellStatus 파싱은 무변경 유지)
  - OOS 분기: 전 셀 21(모의 PASS) 기록 → P4 탐색 루프가 OOS에서 1회에 성공하는 흐름 검증 가능
- 잔여 확인(실기): 실제 PG 응답에 8(신호선불량) 자릿수가 오는지 실사 (C2 — OCP PG와 동일 프로토콜이면 동일)

### Phase 4 — 자동 컨택 탐색/재사용 시퀀스 ✅ 구현 완료 (2026-08-10, 빌드 OK)
- `CAutoContactInfo::DoSearch()` — OCP DoWorkAutoContact 2단계 탐색 이식 (Ready 이동 → 1차 FGAP 하강×R-Test(1셀이라도 21) → 2차 SGAP 하강×성공률≥Percent → PreContact+OD+AddOD 최종 컨택 → 더 깊을 때만 SavePos). 가드: gap≤0 설정 오류 차단, 축=AXIS_PROBE_Z5(C1), R-Test = `DoWorkInspectStart(PG, PG_TEST_3)` 동기 1회
- `CAutoContactInfo::DoDataMove()` — 저장값 직행 컨택 (OCP DataMove; 프로브 사용횟수 북키핑은 미이식, C6)
- 중단: 다이얼로그 STOP → `RequestStop()` 플래그 + 시퀀스 경로는 `IfPauseEvt_WaitResumeEvt` 람다 콜백(일시정지 지원 포함)
- ~~`DoWorkGlassPreMeasStep_Contact`에 분기~~ → **Phase 4b에서 이설 (2026-08-10 사이클 판독 결과)**:
  PreMeas의 Contact 스텝은 **체인에서 주석 처리된 사어 코드** (DoWorkGlassPreMeas.cpp:52 — 실행 체인은 Move→PtrnSearch→RcpChangeDone→P2P→Idle).
  실제 컨택 = **Measure 아이템의 `CDoWorkGlassMeasureStep_Probe_Contact`** — **스틱마다 실행** (OCP iTop 루프와 1:1 대응,
  원저자 주석 `// AutoContactData 나중에 추가`가 있던 바로 그 지점 :1771). 분기 3종을 이곳으로 이설:
  - Use&&Success → 저장 컨택 높이와 현재 Z 비교 후 다르면 `DoDataMove` (기존 skip 규칙 유지)
  - Use&&!Success → `DoSearch` (스틱마다 — OCP 동일) + **마지막 스틱 성공 시** SetSuccess+SaveLog (`GetMeasureStick()==nMaxStick-1` — Measure 스텝은 스틱 인덱스 네이티브 보유)
  - !Use → 기존 `MoveCmdPos_ProbeContact` (무변경)
  - PreMeasStep.cpp는 백업에서 원복 (사어 코드 수정분 제거)
- 알람 3종: `PROBE_AUTO_CNT_PORT_FAIL/1STEP_FAIL/2STEP_FAIL` = **1500~1502 (OCP 동일 ID)** — AlarmIDList.h 등록(전체 리빌드) + Alarm.xls 행 삽입(OCP 한글 원문 이관 + 영문 병기, Level H/Module JOB, Excel COM)
- 다이얼로그: START = 게이트(Offline/SPS_IDLE+EngineCompleteAll/Probe Z≈0/레시피 일치) 후 원샷 워커로 **시퀀스와 동일 루틴(DoSearch) 1회 실행**(C5), STOP = RequestStop, PG Download = `DoWorkRecipeOpen(PG)` 워커(C4), 실행 중 버튼 비활성+STOP 버튼 표시(타이머)
- 백업: `.bak_autocnt` (PreMeasStep/AlarmIDList/Alarm.xls) + `.bak_autocnt_p4` (다이얼로그 h/cpp)

### Phase 4b — 1차 사이클 판독 (2026-08-10 16:10 런, Long 12스틱 완주)
- ✅ **P2 리셋 판정 실증**: ProbeCheck 스텝에서 Always=1 경로 동작 — Config.ini에 `ContactDataProbeID=TEST5`(공유메모리 m_szPROBEID[4], PLC 스캐너 값) 기록 + 상태 리셋 확인
- ❌ **Contact 스텝 미실행 발견** → 원인 = PreMeas 체인에서 Contact 스텝 주석 처리(사어 코드). 실제 컨택은 Measure 아이템 `Probe_Contact` 스텝(스틱당 1회, 12회 실행 확인) → **분기 이설 완료, 빌드 OK**
- 추가 수정: 다이얼로그 IDC_EDIT_1 = 현재 오픈 레시피 **타이머 상시 갱신** (타 화면에서 레시피 열어도 반영, 사용자 요청)

### Phase 4c — 2차 사이클 판독 + PASS 표기 타이밍 수정 (2026-08-11)
- ✅ **탐색 시퀀스 실증** (17:43~17:51 런, Long 12스틱): 스틱마다 `AutoContact: search start` 12회 →
  마지막 스틱 후 `search success (all sticks)` → Config.ini `Sucess=1, Ready=6.9, Pos1=6.9, Pos2=0.0`
  (OOS는 R-Test 전 셀 모의 PASS라 Ready 위치 즉시 성공 = 기대값. 실기에서는 하강 탐색으로 Pos1>Ready, Pos2>0)
- 🐛 **PASS 표기 타이밍 이슈 (사용자 관찰: 측정 후 PASS 표기)** — 원인: PG `WaitInspectionComplete`의
  셀 상태 기록은 `m_nAgingStartStick[4]` 슬롯 기준인데, 이 슬롯은 OST/RTEST 스텝이 **자기 시작부에서** 현재
  스틱으로 갱신함. 자동 컨택 R-Test는 그보다 앞선 Probe_Contact 시점에 실행되어 **이전 스틱 값이 잔류** →
  이전 스틱 셀들이 측정 완료 후 PASS로 덮임. **수정**: 탐색 직전에 슬롯을 현재 스틱으로 설정
  (OST/RTEST와 동일한 rotate 변환: rotate==1 직행 / 그 외 `nMaxStick-iStick-1` 반전) → PASS가
  해당 스틱의 컨택(OST 진입 직전) 시점에 표기됨. 빌드 OK

### Phase 6 — 수동 전용 전환 + 스틱 선택 UI (2026-08-11 사용자 지시, 구현 완료/빌드 OK)
**설계 변경**: 사이클 시퀀스에서는 동작하지 않고 **메뉴얼 전용**으로 사용. 확정: ① 얼라인 체인 미포함(얼라인 완료 상태에서 START) ② 스틱 선택 콤보에 **ALL(일괄 탐색) + 1~N** ③ Success는 표시용 마킹으로 단순화.

- **A1/A2 시퀀스 경로 제거**: Probe_Contact 분기·ProbeCheck 리셋 훅 백업 원복 — 사이클은 AutoContact와 완전 무관. AutoContactInfo/PG raw 캡처는 수동용 유지
- **스틱 선택 UI**: `IDC_COMBO_AC_STICK` 콤보(ALL, 1..N) + `IDC_STATIC_AC_PROG` 진행 표시 라벨 신설 (ALPHA.rc UTF-16 + resource.h 21001/21002). 콤보 갱신 = OnInitDialog + Recipe Open 성공 + 타이머(레시피 변화 추종, 스틱 수 캐시 가드)
- **OnRcpOpen 실제 로드 수정**: P1에서 완료 보고했던 RecipeChange 연결이 **실제로는 스크립트 누락**이었음을 발견(가짜 성공 잔존) → 이번에 정정 (RecipeChange + EDIT_1 + 스틱 목록 갱신)
- **START 워커 재작성**: 게이트 → 스틱 선택 검증(ALL=-1/단일) → CheckAndReset(시퀀스에서 이동해 온 리셋 판정) → 스틱 루프[Z 리프트(0) → 스테이지 R→X 이동(스틱 m_fStageY/m_fStageR+오프셋, MoveMeasureArea OCP 규칙) → PG 스틱 슬롯(rotate 변환) → DoSearch] → 전체 성공 시 Success(표시용)+SaveLog
- **진행 표시**: 상태 머신(volatile int) → 타이머가 "Stick n/N moving/searching / Done / Fail / Stopped" 렌더 (워커→UI 직접 접근 없음)
- 백업: `.bak_autocnt`(resource.h/ALPHA.rc), `.bak_autocnt_p6`(다이얼로그 h/cpp)
- **인포지션 체크 추가 (2026-08-11, 빌드 OK)**: 모든 이동 뒤 명시적 `DoWork_MotionDone` — 워커(Z 리프트/스테이지 R/스테이지 X, ALL 루프 포함) + DoSearch(Ready·1차/2차 스텝 하강·최종 컨택) + DoDataMove(Ready·컨택). 순서 = Stage Move→Done→Contact→Done→기능(R-Test). MoveCmdPosA 블록모드의 자체 완료대기에 더해 이중 확인 (OOS 로그에 MotionDone Start/End 기록되어 검증 용이)

### Phase 5 — OOS 통합 검증 체크리스트
- [ ] Use=0: 기존 컨택 동작 무변화 (하위호환)
- [ ] Use=1 & !Success: Ready→1차(FGAP 스텝)→2차(SGAP 스텝)→AddOD 최종 컨택, 각 한계 초과 시 해당 알람
- [ ] 성공 후 재측정: 저장값 직행 컨택 (탐색 생략)
- [ ] Always/ModelChange/ProbeChange 각각의 리셋 조건
- [ ] STOP 중단 안전, Offline 게이트
- [ ] 저장 검증 6종 (소수 입력 포함 — _tstof 수정 확인)
- 실기 전환 시: R-Test 실응답 포맷 검증(C2), ProbeCardID 연동(C3), 파라미터 실측 티칭

---

## 6. 수정/신규 파일

| 파일 | 구분 | 내용 |
|------|------|------|
| `Source/ALPHA/DlgSubManual_AutoContact.cpp/.h` | 수정 | 스텁 수리 + 헬퍼 재배선 + 버튼 연결 |
| `Source/ALPHA/DoWork/AutoContactInfo.h/.cpp` | 신규 | 비-UI 파라미터/상태 보관소 |
| `Source/ALPHA/DoWork/DoWorkGlassPreMeasStep.cpp` | 수정 | Contact 스텝 분기 + 탐색 루프 |
| `Source/ALPHA/DoWork/DoWorkGlassRecipeStep.cpp` (또는 시퀀스 시작부) | 수정 | 리셋 판정 호출 |
| PG 수신 객체 (`Manager/CommTcpIpObjectPG*`) | 수정(조건부) | R-Test 응답 파싱 (C2 확인 후) |
| `Source/Include/AlarmIDList.h` + Alarm.xls | 수정 | PROBE_AUTO_CNT_* 3종 |
| `Source/Bin/Config/Config.ini` | 데이터 | [PROBE_AUTO_CONTACT] 섹션 |

백업 접미사(제안): `.bak_autocnt`. pch 포함 헤더(AlarmIDList.h) 수정 → 전체 리빌드 1회 각오.

---

## 7. 미확정 항목 → **전건 확정 (2026-08-10 사용자 답변)**

| # | 확정 내용 |
|---|-----------|
| C1 | **P5 단독** — OCP 모드 P5 단독 우선 |
| C2 | 21=OK / 28=신호선불량 체계 유지 — **필요 시 응답 파싱 추가** (Phase 3 실사) |
| C3 | ProbeCardID = **PLC 스캐너 연동** |
| C4 | PGEvent_0/1/2 **매핑으로 구현, 부족 시 협의** |
| C5 | START 수동 기동 = **시퀀스 1스텝 실행** |
| C6 | 공유메모리 노출 **불필요** — 해당 다이얼로그 UI에서만 노출 (헬퍼 내부 보관) |
| C7 | **Config.ini [PROBE_AUTO_CONTACT] 섹션** (OCP 키 호환) |

---

## 8. 리스크

1. **하강 탐색 = 물리 충돌 위험**: FirstLimitPos/ODMax 한계값이 안전망 — 실기 최초 가동은 저속 + 보수적 한계로.
   OOS 검증은 지령 흐름만 확인 가능(실제 접촉 없음)
2. **R-Test 응답 의존**: PG가 R-Test 셀별 결과를 주지 않으면 피드백 루프 불성립 → Phase 3에서 조기 실사 (C2)
3. **레시피 컨택값과의 우선순위 혼동**: override 구조를 로그로 명시 (어느 값으로 컨택했는지 KMLOG 기록)
4. **_ttoi 버그 수정의 파급**: 기존에 저장된 절삭값(정수)이 남아 있으면 수정 후에도 이상값 — 초기 1회 AutoCntPosReset 권장
5. **다중 프로브(HMI 모드) 확장 시**: OCP 알고리즘은 단일 Z 전제 — P1~P4 동시 탐색은 별도 설계 필요 (C1에서 범위 축소)
