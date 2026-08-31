# ANGLE 다이얼로그 이식 플랜 (OCP IDD_DLG_NAVI_MANUAL_ANGLE → HMI)

작성: 2026-08-10 (LYS)
선행 이식 선례: AgingSet_Porting_Plan.md, AgingTest_Porting_Plan.md, WAD_MultiMeas_Porting_Plan.md, AgingCycleEngine_Design_Plan.md

> **⛔ 보류 (2026-08-10 사용자 결정)**
> 기존 Metrosys는 Angle 조작을 **축 직접 운용**(AXIS_ANGLE1/2 + Y/Z 기구학 보정)으로 구현했으나,
> **ALPHA(HMI 설비)는 Angle 조작을 프로토콜 통신으로 구현해야 한다** (외부 장치에 각도 지령 송신).
> 해당 **통신 사양 문서가 아직 존재하지 않아** 이식 작업 전체 보류.
> → 통신 문서 입수 시 §2.5 기구학/축 직접 제어 전제(P3, P4의 이동 부분)를 통신 명령 송수신 방식으로
> 재설계한다. 통신 계층은 `CCommTcpIpFactoryMgr` Factory 패턴에 신규 오브젝트 추가(PGMon 선례) 또는
> 기존 WAD/ANGLE 오브젝트 명령 확장이 후보. §3 축 매핑 표와 §2.5~2.6 기구학·파라미터 분석은
> OCP 동작 이해용 참고 자료로 유지 (HMI 구현에는 미적용).
> 다이얼로그 UI(P1)·데이터 계층(P2)은 통신 방식과 무관하므로 문서 입수 전 선행 가능.

---

## 1. 목적 및 범위

OCP Metrosys의 `CDlgNaviManualAngle` (IDD_DLG_NAVI_MANUAL_ANGLE, `DlgNaviManualAngle.cpp` 1,619줄 + `.h` 155줄)을
HMI ALPHA의 `CDlgSubManual` 서브 다이얼로그 구조로 이식한다.

이 다이얼로그의 실체는 **"특성(WAD) 측정 각도 스케줄러"** 다:

1. **각도 레시피 편집**: 측정할 시야각 구간(Start/End/Interval/Enable)을 그리드로 편집, 레시피별 INI 파일로 저장
2. **검사 각도 리스트 산출**: 구간을 전개→중복제거→오름차순 정렬한 최종 각도 리스트 표시
3. **시퀀스 공급**: 특성 측정 루프가 이 리스트를 읽어 **각도별로 헤드를 틸트하며 반복 측정**
4. **수동 각도 이동**: 지정 각도로 H1/H2 헤드 Y/Z/Angle 3축 연동 이동 (티칭/확인용)

HMI에는 대응물이 전무하다 — 다이얼로그, 각도 레시피, 시퀀스 각도 스윕 모두 없음. **완전 신규 이식**이며,
AgingSet/AgingTest와 동일 패턴(ctor 주입 + CMySpreadDll + 비-UI 헬퍼 분리)을 따른다.

---

## 2. OCP 기능 분석

### 2.1 데이터 모델 (DlgNaviManualAngle.h)

```cpp
#define MAX_ANGLE_ITEM   7        // 그리드 컬럼 수 (No 포함)
#define MAX_ANGLE_COUNT  100      // 각도 항목 최대

typedef struct {                  // 편집 그리드 1행
    CString strStartAngle;        // 시작 클래스 (C/L/R/S/W 또는 A{각도})
    CString strEndAngle;          // 종료 클래스
    double  dStartAngle;          // 시작 각도
    double  dEndAngle;            // 종료 각도
    double  dInterval;            // 전개 간격
    BOOL    bEnable;              // 사용 여부 (col7 체크박스)
} angleData;

typedef struct {                  // 산출 리스트 1행
    CString strClass;             // 각도 클래스명
    double  dAngleValue;          // 각도값
} InspAngleData;
```

- **세트 2개**: 메인 세트(`m_vAngleData`/`m_vInspAngleData`) + **_S 세트**(`m_vAngleData_S`/`..._S`)
  - _S의 용도 = **WAD 다점측정(MultiWad)의 2번째 이후 포인트 전용 각도 리스트**
  - 근거: `DlgNaviMaintHEAD.cpp:2112~2117` — `bMultiWad && nCurrent > 1`일 때만 `GetAngleInspCnt_S()` 사용,
    `JOB_Proc.cpp:8043` — 다점 루프(`m_dlgNaviManualMultiMeas.m_vMeasData` 순회) 내부에서 `GetAngleValue_S(i)`
  - 즉 기본 포인트는 전체 각도(메인), 추가 포인트는 축약 각도(_S)로 택트 절감하는 구조
- **각도 클래스 규약** (`ChangeAngleClass`): `C`=0° / `L`=45° / `R`=30° / `S`=60° / `W`=15°.
  소수부가 0인 각도는 위 문자로 치환, 그 외는 `A{각도}` 표기. `InitSetData()` 기본 5행 = C/L/R/S/W (C, L만 Enable)
- **AngleCalc()**: Enable 행만 → Start==End && Interval==0이면 단일 각도, 아니면 Start부터 Interval씩 전개(End 초과분은 End로 클램프)
  → 전체 병합 후 **중복 제거 + 오름차순 삽입 정렬** → `m_vInspAngleData`
- `GetAngleValue(i)` 호출 시 `m_nAngleCurrentCnt = i+1` 갱신 → ELP 진행 보고용(`GetAngleCurrentCnt`)

### 2.2 파일 I/O

- 경로: 레지스트리 `DIR\ANGLE_RCP` (OCP 기본 `D:\KMAC\PARAM_OCP\PARAM\AngleRcp\`)
- 파일: `{AngleRcpPath}\{RecipeID}.rcp` — **INI 포맷**, 섹션 `[DATA]`
- 키: `START_CLASS/START_ANGLE/END_CLASS/END_ANGLE/INTERVAL/ENABLE` (+ `_S` 접미 6종) — 각 키에 행 값들을 콤마로 병합
- 저장 시 CLASS 키는 각도값으로부터 `ChangeAngleClass(A{각도})`로 재생성 (문자 규약 강제)
- OPEN 버튼: 측정 레시피 폴더에서 `.rcp` 선택 → 확장자 제거한 이름으로 `LoadAngleData(_S)` 호출 (각도 파일 없으면 기본값 Init)
- 레시피 연동 로드: `DlgNaviMaintHEAD.cpp:2201` — 측정 시작 시 `LoadAngleData(m_szRecipeID)` 자동 호출

### 2.3 시퀀스 소비 (JOB_Proc `DoWorkStartMeasOCP`, :7575 / :8043 / :10004)

특성(WAD) 측정 시퀀스에서 **각도 루프가 측정 루프를 감싼다**:

```
기본 포인트 측정:
  for i in 0..GetAngleInspCnt():
    dMove = GetAngleValue(i)
    현재각과 0.01 이내면 이동 생략
    WadMoveAngle(dMove, head, TRUE, fPos)     // Y/Z/ANGLE 3축 연동 틸트
    WaitForResponse_MovingDone(AXIS_ALL)
    if ANGLECHECK: CheckCurrentAngle(...)      // PLC 각도 센서 확인 (0°/45°만)
    SetAngle(dMove)                            // HEAD 표시 갱신
    ExeProperty(...)                           // 특성 측정 (각도당 1회)

다점(멀티) 포인트: _S 리스트로 동일 루프 (JOB_Proc:8043)
```

- ELP(신판) 상태 보고: `stELP.nCurrentAngle = GetAngleCurrentCnt()`, `stELP.nTotalAngle = GetAngleInspCnt()`(기본) / `_S`(다점 2+), `stELP.Degree` = 현재 각도 문자열 (`DlgNaviMaintHEAD.cpp:2106~2120`)

### 2.4 수동 각도 이동 (다이얼로그 하단)

- `IDC_EDIT_ANGLE` 입력 + `ANGLE_MOVE`(H1) / `ANGLE_MOVE2`(H2) 버튼 → `MoveCmd_WadZ(head, angle)`
  - 가드: `ONLINE_REMOTE` 차단, 각도 범위 `0~75`
  - `Cmd_WadMoveAngle(CMDMOVE)` → 모션 스레드 `WORK_MOVE_WAD_ANGLE` → `WadMoveAngle()` 실행
- 위치 표시 `EDIT_3~7`: H1/H2 각각 지령위치(GetCurPos)와 현재위치(stMotionStatus)의 Y/Z/A/X + 계산 목표값(SetPosition 콜백)

### 2.5 각도 틸트 기구학 (`WadMoveAngle`, DlgNaviMaintMotionACS.cpp:9200~9386)

헤드가 암(Arm) 회전축에 매달린 구조 — 각도 변경 시 측정점 유지를 위해 Y/Z 보정 이동:

```
dX = HWLength*cos((-현재각+HWAngle)*π/180) - HWLength*cos((-목표각+HWAngle)*π/180) + 각도별오프셋
dZ = HWLength*sin((-현재각+HWAngle)*π/180) - HWLength*sin((-목표각+HWAngle)*π/180)
→ Y축(Y1/Y2) += dX,  Z축(Z2/Z5) -= dZ,  ANGLE축(ANGLE1/ANGLE2) = 목표각
순서: Z를 AngleReadyPos로 선이동 → ANGLE축 이동 → Z 리밋 검증(CheckWadLimitZ) → 3축 지령
가드: X2 겐트리 위치 > AngleGantryLimit이면 이동 거부 (충돌 방지)
```

- 각도별 Y 오프셋: `m_dMoveOffset[6][h]=30°, [7][h]=45°, [8][h]=60°` (모션 오프셋 테이블 — 출처 확인 필요)

### 2.6 파라미터 (레지스트리 `ANGLE_PARAM` 섹션, JOB_Proc.cpp:33130~)

| 키 | 기본값 | 의미 |
|----|--------|------|
| `ANGLECHECK` | 0 | PLC 각도 센서 확인 사용 |
| `HWLength` | 241.6 | 헤드 암 길이 (mm) |
| `HWAngle` | -69.02 | 암 기준각 (deg) |
| `AngleReadyPos` | 700.0 | 틸트 전 Z 대기 위치 |
| `AngleGantryLimit` | -30.0 | 틸트 허용 X2 한계 |

### 2.7 PLC 각도 센서 (`CheckCurrentAngle`)

- `stInterLock.m_bLeftAngleChk[2]/m_bRightAngleChk[2]` ← `PLC_LEFT/RIGHT_ANGLE_30_SENSOR + i` (DlgNaviMaintPLC.cpp:2507)
- 0°/45° 두 지점만 센서 확인, 불일치 시 `PLC_{LEFT|RIGHT}_ANGLE_{30|45}_SENSOR_ERROR` 알람 리턴

---

## 3. HMI 대응 구조 매핑

| 항목 | OCP | HMI 대응 |
|------|-----|----------|
| 부모/전환 | `DlgNaviManual` + BTN10 | `CDlgSubManual` + 신규 버튼 + `enumManualTemID_ANGLE` (WindowEventHandler.h) |
| 그리드 | FPSpread OCX ×3 (DLGINIT) | **CTEXT 스태틱 + CMySpreadDll attach** (MultiMeas 선례 — DLGINIT 불필요) |
| 다이얼로그 리소스 | Metrosys.rc | `ALPHA.rc` (**UTF-16 LE** — MultiMeas 신설 선례 있음) |
| 생성 패턴 | 멤버 직접 Create | `new CDlgSubManual_Angle(m_pAuroraMgr)` ctor 주입 + Create + MoveWindow (AgingSet 선례) |
| 설정 저장 | `theApp.GetProfile*` | `AfxGetApp()->GetProfile*` 동일 (AgingAlarmCheck 선례) |
| 각도 레시피 경로 | `D:\KMAC\PARAM_OCP\PARAM\AngleRcp\` | **`D:\HBS\PARAM_OCP\PARAM\AngleRcp\`** + 레지스트리 `DIR\ANGLE_RCP` (AgingRcp 관례) |
| 레시피명 | EDIT_1 텍스트 | **항상 `RecipeEditMgr::GetRecipeID()`** (AgingTest 교훈) |
| Offline 판정 | `stTitle.m_nHostMode` | `GetShareMemoryMgrPtr()->GetSys_MachineMode() != df_HOST_OFFLINE` (AgingSet 선례) |
| 시퀀스 접근 | JOB_Proc가 다이얼로그 포인터 직접 호출 | **금지 — 비-UI 헬퍼 `DoWork/AngleInspInfo` 신설** (AgingCycleInfo 선례) |
| 특성 측정 스텝 | DoWorkStartMeasOCP 인라인 | `CDoWorkGlassMeasureStep_Measure_WAD` (base 측정 + MultiPoint 루프 기구현, LYS 2026-07-23) |
| 다점 각도(_S) | JOB_Proc:8043 | WAD 스텝 MultiPoint 루프 (`CWadMultiMeasMgr`) 내부 |
| ELP 보고 | stELP.nCurrentAngle/nTotalAngle/Degree | HMI WAD/PGMon 프로토콜 대응 필드 **확인 필요 (C4)** |

### 축 매핑 (핵심)

| 용도 | OCP | HMI | 비고 |
|------|-----|-----|------|
| WAD 헤드 Y | `AXIS_Y1` / `AXIS_Y2` | `AXIS_Y1` / `AXIS_Y2` | 동일 명칭 (Bottom Cross L/R) |
| WAD 헤드 Z | `AXIS_Z2` / `AXIS_Z5` | `AXIS_Z2` / `AXIS_Z5` | ⚠ AxisType.h 주석 "2026-07-14 특성↔화질 스왑" — 실축 확인 필요 (C6) |
| 각도(틸트) 축 | `AXIS_ANGLE1/2` (=98/99, LYS 2026-05-21 TEMP 추가) | **`AXIS_SPECTO_DEGREE1/2`** (`VT3-AXIS`/`VT4-AXIS`) | HMI 정식 축 — OCP TEMP 축의 원본 |
| 겐트리 리밋 체크 | `AXIS_X2` | `AXIS_X2` (OCP 옵션 시 ANGLE 주제어가 X3인 점 주의) | |

> OCP의 `AXIS_ANGLE1/2`는 매칭 작업 중 편의로 임시 추가된 축(98/99)이다. HMI에는 분광계 틸트 축
> `AXIS_SPECTO_DEGREE1/2`가 원래부터 정의되어 있고 ACS 축 이름 `VT3-AXIS`/`VT4-AXIS`로 매핑된다
> (`MotorControl_ACS.cpp:111`). 현재 HMI 코드에서 이 축은 초기화 제외 목록에만 등장 — 시퀀스 사용 전무.

---

## 4. 핵심 설계 결정

### 4.1 그리드 좌표계 — AgingSet §3.1 규칙 그대로
- CMySpreadDll: 헤더 = `SS_HEADER`(row 0), 데이터 행 1..MaxRows, No = col 0, 데이터 col 1..N
- OCP FPSpread 좌표(헤더 row1/데이터 row2..)를 그대로 옮기면 안 됨 — AgingSet 이식 초기 버그의 원인
- 그리드 3개: AngleSet(메인 7col), AngleSet_S(7col), AngleList(산출 2col: CLASS/ANGLE)
- ENABLE 체크박스: CMySpreadDll 체크박스 셀타입 사용 (AgingTest col6 ANGLE 체크박스 선례)

### 4.2 시퀀스는 다이얼로그에 접근하지 않는다
- `DoWork/AngleInspInfo.h/.cpp` 신설 (AgingCycleInfo 패턴):
  - `Load(strRecipeID)` — INI 파싱 + AngleCalc 전개 (다이얼로그와 동일 산출 로직 공유)
  - `GetCount()/GetValue(i)/GetClass(i)` + `GetCount_S()/GetValue_S(i)` + `GetCurrentCnt()`
  - 다이얼로그는 편집/저장 UI, 시퀀스는 AngleInspInfo만 사용 — 파일이 단일 계약
- 레시피 오픈 시점 로드: OCP는 측정 시작 시 자동 `LoadAngleData(RecipeID)` — HMI는 WAD 스텝 진입부에서 `AngleInspInfo.Load(GetRecipeID())`

### 4.3 게이트 및 하위호환 (중요)
- OCP는 각도 루프가 측정 루프를 감싸므로 **리스트가 비면 측정 자체가 스킵**되는 구조
- HMI는 하위호환 우선: **각도 파일 부재 또는 산출 리스트 0개 → 기존 1회 측정(각도 이동 없음) 유지**
- 활성 조건(제안): `AngleRcp\{RecipeID}.rcp` 존재 && 산출 각도 ≥ 1 && 레지스트리 `ANGLE_PARAM\USE_ANGLE_SWEEP=1` (기본 0) — 명시 플래그로 실기 안전 확보 (C7에서 확정)
- AgingCycle 경로: WAD 스텝을 그대로 재사용하므로 게이트만 통과하면 에이징 리스트 측정에도 자동 적용

### 4.4 결과 데이터 — 1차 last-write-wins
- HMI 결과 계보: CommTcpIpObject* → `CResultMgr::Updata_Data`(위치 키) → dResult DLL — **각도 회차 차원 없음**
- OCP는 각도별 결과를 별도 관리(DlgNaviData 각도 루프) — 1차 이식에서는 마지막 각도 결과만 잔존(AgingCycle C5와 동일 정책)
- 각도 회차 분리가 필요해지면 RSRITEMDATA 끝 필드 추가(WAD MultiPoint 선례) + dResult 확장으로 후속 진행

### 4.5 PLC 각도 센서 — 보류 골격만
- HMI `IOMapAddressDefine.h`에 헤드 각도 센서 비트 없음 (밸브 ANGLE/R0 DEGREE만 존재)
- `CheckCurrentAngle` 로직은 이식하되 `ANGLECHECK` 기본 0으로 비활성 — PLC 협의(M주소 배정) 후 비트 연결
- 알람 ID: 활성화 시점에 OCP와 동일 값으로 AlarmIDList.h + Alarm.xls 등록 (AgingSet 7495~7497 선례)

### 4.6 인코딩/빌드 규칙
- 신규 사용자 메시지는 **영어** (HMI/CLAUDE.md 인코딩 규칙 7)
- 신규 파일(AngleInspInfo, DlgSubManual_Angle)은 새 파일이므로 Edit 도구 사용 가능, 기존 파일 수정은 Perl raw byte
- `WindowEventHandler.h`(enum 추가)는 pch 포함 → **전체 리빌드** 발생
- `ALPHA.rc`는 UTF-16 LE — PowerShell(-Encoding Unicode) 또는 Perl(UTF-16 처리)로 편집

### 4.7 수동 이동 안전
- OCP 가드 유지: ONLINE_REMOTE 차단, 각도 0~75 범위, X2 겐트리 리밋, Z Ready 선이동, Z 리밋 검증
- HMI 추가 가드: 이동 명령은 `MaintControlMgr` 경유(시퀀스와 동일 인터록 경로) — Cross Interlock(3027 계열)과 정합
- OOS: SPECTO_DEGREE 축도 OUT_OF_SERVICE 시뮬레이션으로 소프트웨어 검증 가능

---

## 5. 실행 Phase

### Phase 1 — 다이얼로그 UI 신설
1. `ALPHA.rc`에 `IDD_DLG_SUB_MANUAL_ANGLE` 템플릿 추가 (MultiMeas 템플릿 복제 후 수정 — CTEXT 스태틱 `IDC_FPS_ANGLE_SET/SET_S/LIST` 3개 + 버튼군 + EDIT군)
2. `resource.h` ID 등록 (IDD + IDC 일괄)
3. `CDlgSubManual_Angle` 클래스 신설 (`DlgSubManual_Angle.h/.cpp`) — ctor 주입 `CAuroraMgr*`, CMySpreadDll 3개 attach, InitSetData(기본 C/L/R/S/W 5행)
4. `DlgSubManual` 배선: 포인터/Create/ShowWindow 스위치/신규 버튼 핸들러, `WindowEventHandler.h`에 `enumManualTemID_ANGLE`
5. 버튼: OPEN / LIST INIT / SAVE / ADD / DEL (메인·_S 각각) + ANGLE MOVE H1/H2 + 각도 EDIT
- 완료 기준: 빌드 OK, 화면 전환/그리드 편집/기본값 표시 동작

### Phase 2 — 데이터 계층 (파일 I/O + 각도 산출)
1. `DoWork/AngleInspInfo.h/.cpp` 신설 — INI 로드/파싱, AngleCalc 전개(전개·클램프·중복제거·정렬 OCP 로직 동일 이식), 메인+_S
2. 다이얼로그 Save/Load — OCP와 **동일 INI 포맷** 유지 (OCP 각도 파일 복사 시 그대로 동작해야 함)
3. OPEN = 측정 레시피 선택 → 각도 파일 로드, SAVE = `GetRecipeID()` 기준 저장, 산출 리스트 그리드 갱신
4. 경로 초기화: `DIR\ANGLE_RCP` 기본 `D:\HBS\PARAM_OCP\PARAM\AngleRcp\` + `dfcCreateFolder`
- 완료 기준: OCP에서 만든 각도 .rcp 파일 그대로 로드 → 동일 산출 리스트 확인 (데스크 검증)

### Phase 3 — 수동 각도 이동
1. `MaintControlMgr::MoveCmd_WadAngle(int nHead, double dAngle, bool bAuto, double* fPos)` 신설
   - §2.5 기구학 이식: Y1/Y2, Z2/Z5(C6 확정 축), SPECTO_DEGREE1/2
   - 파라미터: 레지스트리 `ANGLE_PARAM` 동일 키 (§2.6) + 각도별 Y 오프셋(30/45/60)
   - 가드: X2 리밋, Z Ready 선이동, 각도 0~75, Z 리밋 검증
2. 다이얼로그 MOVE H1/H2 버튼 연결 + 위치 표시(지령/현재/계산목표) — SetPosition 콜백 대신 **500ms 타이머** 갱신 (AgingTest E4 선례)
3. OOS 동작 확인 (가상 축 이동)
- 완료 기준: OOS에서 45° 지령 시 Y/Z/DEGREE 3축 연동 지령값 로그 일치

### Phase 4 — 시퀀스 통합 (WAD 스텝 각도 루프)
1. `CDoWorkGlassMeasureStep_Measure_WAD::Run()` 진입부에서 `AngleInspInfo.Load(GetRecipeID())` + 게이트 판정 (§4.3)
2. **base 측정 감싸기**: 셀 그룹 이동 완료 후 `for 각도 in 메인 리스트: MoveCmd_WadAngle → MotionDone → (ANGLECHECK 시 센서 체크) → WAD InspectStart/End/Complate` — 각도 0개/게이트 off면 기존 코드 경로 그대로
3. **MultiPoint 루프**(LYS 2026-07-23 구간): 동일 방식으로 _S 리스트 적용
4. 측정 종료/중단 시 각도 원복(0°) 처리 — OCP 동작 확인 후 동일하게
5. ELP 보고(C4 확정 시): 현재/전체 각도 카운트 전달
- 완료 기준: OOS 표준 시퀀스에서 각도 2개(C, L) 레시피로 각도별 WAD 측정 2회 반복 로그 확인

### Phase 5 — OOS 통합 검증
- [ ] 각도 파일 없는 기존 레시피: 동작 변화 없음 (하위호환)
- [ ] 각도 2개 레시피: base 포인트에서 0°→45° 순서 이동+측정, Motion 로그에서 Y/Z 보정값 기구학 일치
- [ ] WAD 다점 레시피 + _S 세트: 포인트 2부터 _S 각도만 사용
- [ ] AgingCycle 경로에서 WAD 항목 측정 시 각도 루프 동작
- [ ] STOP/PAUSE 중단 시 안전 (IfPauseEvt_WaitResumeEvt 루프 내 반영)
- [ ] X2 리밋 가드: 리밋 초과 위치에서 이동 거부 알람
- 실기 전환 시: HWLength/HWAngle/오프셋 실측 티칭(C5), 센서 배선 후 ANGLECHECK=1 검증(C3)

---

## 6. 수정/신규 파일

| 파일 | 구분 | 내용 |
|------|------|------|
| `Source/ALPHA/DlgSubManual_Angle.h/.cpp` | 신규 | 다이얼로그 클래스 |
| `Source/ALPHA/DoWork/AngleInspInfo.h/.cpp` | 신규 | 비-UI 각도 리스트 로더/산출 |
| `Source/ALPHA/ALPHA.rc` (UTF-16) | 수정 | IDD_DLG_SUB_MANUAL_ANGLE 템플릿 + 버튼 |
| `Source/ALPHA/resource.h` | 수정 | ID 등록 |
| `Source/ALPHA/DlgSubManual.h/.cpp` | 수정 | 포인터/Create/전환/버튼 |
| `Source/ALPHA/WindowEventHandler.h` | 수정 | `enumManualTemID_ANGLE` (전체 리빌드 유발) |
| `Source/ALPHA/Manager/MaintControlMgr.h/.cpp` | 수정 | `MoveCmd_WadAngle` 신설 |
| `Source/ALPHA/DoWork/DoWorkGlassMeasureStep.cpp` | 수정 | WAD 스텝 각도 루프 (base + MultiPoint) |
| (조건부) `Source/Include/AlarmIDList.h` + Alarm.xls | 수정 | 각도 센서 알람 (C3 확정 시) |

백업 접미사(제안): `.bak_angle`

---

## 7. 미확정 항목 (진행 전 확인 필요)

| # | 질문 | 기본 가정 |
|---|------|-----------|
| **C0** | **(블로킹) Angle 조작 통신 프로토콜 사양서** — 대상 장치, 명령 셋(각도 지령/완료 응답/현재각 조회), 포트/프레임 포맷 | **문서 부재 — 전체 보류 사유.** 입수 시 P3/P4 재설계 |
| C1 | HMI 실기에 SPECTO_DEGREE(VT3/VT4) 축이 실제 배선되어 있는가? (`Config_Motion_ACS.dat` 매핑 존재 여부) | 배선됨 (OOS 검증은 무관하게 진행 가능) |
| C2 | _S 세트(다점용 축약 각도)도 이식하는가? | 이식 (OCP 동일 — WAD 다점측정과 연동) |
| C3 | 헤드 각도 센서(0°/45° L/R) PLC 배선 존재? M주소? | 없음 — ANGLECHECK=0 골격만 이식 |
| C4 | HMI WAD/ELP 프로토콜에 현재각/전체각/Degree 보고 필드가 있는가? | 확인 후 Phase 4에서 반영 |
| C5 | HWLength=241.6 / HWAngle=-69.02 / 각도별 오프셋 — HMI 실기 값? | OCP 값 이관 후 실기 티칭 |
| C6 | 2026-07-14 특성↔화질 Z 스왑 이후 특성 헤드 Z 실축 (Z2/Z5 vs Z1/Z4) | 현행 WAD 스텝이 쓰는 Z 배정 따름 |
| C7 | 각도 스윕 게이트: 명시 플래그(`USE_ANGLE_SWEEP`) vs 파일 존재만으로 활성 | 명시 플래그 (기본 0, 실기 안전) |

---

## 8. 리스크

1. **택트 증가**: 각도 N개 = WAD 측정 N배. 게이트 기본 off + 각도 파일 없으면 무영향으로 설계
2. **기구학 파라미터 오차**: HWLength/HWAngle이 실기와 다르면 틸트 시 측정점 이탈·충돌 여지 → OOS에서 지령값 검증 후 실기는 저속/단독축부터
3. **Z 축 배정 혼선** (C6): 스왑 주석 미확인 상태로 이식 시 화질 헤드가 틸트됨 — Phase 3 착수 전 확정 필수
4. **X2/X3 인터록**: 각도 틸트 중 Y/Z 이동이 Cross Interlock(3027 계열)과 간섭 가능 — MaintControlMgr 경유로 동일 인터록 경로 사용, OOS 검증 항목에 포함
5. **WindowEventHandler.h 수정 → 전체 리빌드**: 빌드 시간 증가 감수, 다른 작업과 병행 금지
6. **결과 덮어쓰기**: 각도 회차 미분리(last-write-wins) — 운용상 각도별 결과가 필요하면 후속 확장 필요함을 사전 공지
