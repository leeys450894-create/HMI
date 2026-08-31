# Glass 방향(Stage Rotate 각도 체계) 변경 플랜

작성: 2026-08-06 (LYS) / 사용자 확정 요구사항 기반

## 1. 요구사항 (사용자 확정)

Stage Rotate H/W 축 = `AXIS_LOADER_R`. 전 각도 **-90° 시프트**:

| 상태 | 현재 R | 변경 R | 관련 레시피 |
|---|---|---|---|
| Glass Load | 90 | **0** | - |
| Glass 단변 측정 | 270 | **180** | `2_OCP_TEST_Short_260720.rcp` |
| Glass 장변 측정 | 180 | **90** | `1_OCP_TEST_Long_260720.rcp` |

+ Recipe UI 탭 **OPTION INFO**(GLASS ROTATE)에도 반영.

## 2. 현재 구조 조사 결과 (각도 값의 소재지 전수 조사)

### 2.1 설정/티칭 데이터 (코드 수정 없이 값 변경)

| # | 소재지 | 현재 값 | 변경 값 | 비고 |
|---|---|---|---|---|
| D1 | `Bin/Config/Position.ini [LOADER_LOAD] LOAD_R` | 90 | **0** | EQState LOAD 전환 시 `MoveLoaderXYR(0)`→`DoWork_RotateMove` 사용 (FunctionModuleEQStateChangeStep.cpp:36) |
| D2 | `Position.ini [LOADER_UNLOAD] UNLOAD_R` | 90 | **0** | Load와 동일 방향 가정 — 확인 C1 |
| D3 | `Position.ini [LOADER_MEAS] MEAS_R` | 0 | **?** | 흡착 위치 회전각. -90 시프트면 270(=-90)이나 용도 재확인 필요 — 확인 C1 |
| D4 | 측정 레시피 스틱별 `m_fStageR` (STAGE MOVE DATA 그리드 5행) | 180(장변)/270(단변) | **90/180** | 측정 시 `pRecipeData->m_vecStick[iStick].m_fStageR + m_fStageRotate(오프셋)` 로 이동 (DoWorkGlassMeasureStep.cpp:500, PreMeasStep.cpp:108). 신규 레시피 2본 생성/티칭 |
| D5 | Align 레시피 `m_sGlsAlign.m_fStageRotate` | (현행 90 추정) | **0** | Glass Align 진입 시 Stage R 이동 목표 (DoWorkGlassSWAlignStep.cpp:85→188). Align은 Load 방향 수행 가정 — 확인 C1 |
| D6 | 레시피 `m_sMotionOffsetPos.m_fStageRotate` | 오프셋(상대값) | 변경 불요 | USER OFFSET — 절대각 아님 |

수동 UI(DlgSubManual_Meas:1735/4271, DlgTestMode:440)는 위 데이터를 읽어 쓰므로 데이터 변경으로 자동 반영.

### 2.2 코드 로직 (수정 필요/검토)

| # | 소재지 | 현재 동작 | 변경 필요 사항 |
|---|---|---|---|
| C-a | `MotorControlMgr::MoveCmdPosA_Sub` :228-244 — **X 이동 시 R 위치 인터록** | `fabs(R)<2 \|\| fabs(R)-90<2 \|\| fabs(R)-180<2` — 조건식 결함으로 사실상 R<182 전부 허용, **270만 차단** (현행 단변 270에서 X 이동 불가였던 상태) | 신규 각(0/90/180)은 우연히 전부 통과하지만, 의도대로 **허용각 리스트 {0,90,180}±2 정확 판정**으로 수리 권장 |
| C-b | `MotorControlMgr` :1962-1979 — **멀티 이동 X 포함 시 R 인터록** | 조건 부호가 반대(0/90/180 근처면 `MOTION_ROTATE_POS_ERROR`) — 신규 체계에서 **모든 정상각이 에러** | 호출 경로 존재 확인 후 C-a와 동일 기준으로 수리 (필수) |
| C-c | `MaintControlMgr::DoWork_RotateMove` :1225-1269 — OOS 가상 DEGREE 비트 | `nType`(0=Load/1=Unload/2=Meas)을 DEGREE_0/1/2(0/90/180도 센서)에 그대로 매핑 — 현행에서도 Load(90도)↔DEGREE_0(0도) 불일치 | **각도(dPos) 기준으로 비트 선택**하도록 변경: 0→DEGREE_0, 90→DEGREE_1, 180→DEGREE_2, 270→DEGREE_3. 신규 체계에서 Load=0→DEGREE_0 정합 |
| C-d | `FunctionModuleEQState::IsPosion0/90/180/270_SensorOn` | 호출부 전부 주석 처리(활성 소비자 없음) | 코드 변경 불요. **실기 PLC의 DEGREE 센서 물리 배선이 새 각도와 일치하는지 하드웨어 확인 노트** |
| C-e | `m_nGlsRotate` 논리 분기 (MakeCellID 스틱 순서, ChartOrbit, Resume 검증, AGING 스텝 :5763 rotate 변환, DoWorkGlassRecipeStep:118) | 0(0도)/1(90도) 2값 분기 | 물리각 -90 시프트와 논리 rotate의 대응 재정의 — 확인 C2. 장·단변 레시피가 각각 어느 rotate 값을 쓰는지에 따라 분기 영향 없음/있음 판단 |

### 2.3 Recipe UI — OPTION INFO 탭 (dll_Recipe/RecipeMgrDlg.cpp)

- GLASS ROTATE 콤보(:756, :775-776): 현재 옵션 **"0도"/"90도" 2개**, 셀 **READONLY**(:765) — 값은 레시피 로드 시 표시만 됨 (`m_nGlsRotate` 인덱스 저장, Save :445).
- 변경안(확인 C2 결과에 따라 확정):
  - **U1**: 콤보 옵션을 새 체계 표기로 확장 — 예: "0도(Load)"/"90도(장변)"/"180도(단변)" 3개 또는 0/90/180/270 4개. `m_nGlsRotate` 주석의 0~3 정의와 일치시킴
  - **U2**: READONLY 해제 여부 결정(장·단변 레시피 저장 시 UI에서 선택 가능해야 하면 해제)
  - **U3**: 옵션 확장 시 소비 코드가 0/1만 분기하므로(2.2 C-e) 2/3 값 유입 시의 분기 정의 필수 — 최소한 가드/로그
- Recipe 파일 버전: `m_nGlsRotate`는 기존 필드라 Save/Load 순서 변경 없음 → **RCP_VERSION 증가 불요** (3-file sync 규칙 비적용)

## 3. 확인 항목 — P0 사용자 확정 결과 (2026-08-07)

| # | 확정 내용 |
|---|---|
| C1 | **Align 각도도 -90 시프트 대상.** Recipe 측정 각도는 확정 표(장변 90/단변 180)를 따름. MEAS_R 0→**270 가동 가능** 확인 |
| C2 | **장변=1(90도) / 단변=2(180도)** |
| C3 | **편집 가능화** (4옵션 0/90/180/270 적용) |
| C4 | **가능.** R0 POSITION 01~04 = M4400(0도)/M4401(90도)/M4402(180도)/M4403(270도) — `PLCMAdress.csv`의 `PLC_R_MOTION_LOADER_DEGREE_0~3 = 4400~4403`과 **완전 일치 확인, CSV 변경 불요** |

참고 도면: `HMI/Doc/07_설계문서/HMI 설비 Contact 방향.pdf` — Port→Index(R)→HMI 반입, 0도 Load, 장변 Contact 90도, 단변 Contact 180도.

## 4. 실행 Phase (각 Phase 승인 후 진행)

| Phase | 내용 | 상태 |
|---|---|---|
| **P0** | §3 확인 항목 확정 (C1~C4) | ✅ 확정 (2026-08-07) |
| **P1** | 데이터 변경 | ✅ 완료 (2026-08-07, 아래 결과) |
| **P2** | 인터록/OOS 코드 수리 | ✅ 완료 (2026-08-07, 아래 결과) |
| **P3** | Recipe UI OPTION INFO | ✅ 완료 (2026-08-07, 아래 결과) |
| **P4** | OOS 통합 검증 | 실행 대기 (아래 체크리스트) |

### P1 결과 (2026-08-07) — 데이터 변경 (백업 `.bak_rot`)

- `Position.ini`: `LOAD_R 90→0`, `UNLOAD_R 90→0`, `MEAS_R 0→270`
- `D:\HBS\Recipe\ALIGN_RECIPE\HMI_90.alg`: GlassAlign StageRotate(6행) `90→0` (HMI.alg는 이미 0)
- **측정 레시피는 이미 존재 확인** (`D:\HBS\Recipe\Recipe\`): `1_OCP_TEST_Long_260720.rcp` — 스틱 StageR 전부 0→**90**(×12), GlsRotate=1(기존값 유지), Align=HMI.alg / `2_OCP_TEST_Short_260720.rcp` — StageR 0→**180**(×21), GlsRotate 0→**2**, Align=HMI_90.alg
- 주의: 두 레시피의 세부 티칭값(StageY/Cell 좌표 등)은 구 좌표계 기준일 수 있음 — 실기 티칭 시 재확인

### P2 결과 (2026-08-07) — 코드 수리 (백업 `.bak_rot`, 빌드 OK)

- **C-a**: 허용각 검사 `fabs(dCurPos-N)<2` 정식 판정으로 수리 — 동일 결함이 3곳 존재하여 전부 수정: `MoveCmdPosA_Sub` AXIS_X(:236), `MoveCmdPosR_Sub` AXIS_X(:1134) / AXIS_LOADER_R(:1160). 허용각 {0,90,180,270}
- **C-b**: 멀티 이동 X 인터록(:1967) 조건 반전 수리 — 스테이션 각이 아닐 때만 `MOTION_ROTATE_POS_ERROR`
- **C-c**: `DoWork_RotateMove` OOS DEGREE 비트를 호출자 타입이 아닌 **명령 각도(dPos) 기준**으로 선택 (0/90/180/270→DEGREE_0~3, 그 외 각도는 비트 미설정)

### P3 결과 (2026-08-07) — Recipe UI (백업 `RecipeMgrDlg.cpp.bak_rot`, dfcRecipe64d.dll 빌드 OK)

- OPTION INFO GLASS ROTATE 콤보: READONLY 해제 + 옵션 4개(0°/90°/180°/270°, EUC-KR 도 기호 0xA2AA 사용 — 파일 인코딩 보존)
- USER OFFSET 탭의 QUALITY ROTATE 콤보(0°/90°)는 별개 필드(m_nQualityRotate)로 미변경
- **U3 잔여**: `m_nGlsRotate` 소비 코드는 `==1` 분기만 존재 → 단변(2)은 0과 동일 경로로 동작. 단변 스틱 순서/CellID 정합은 P4 검증 항목 (필요 시 소비 코드에 ==2 분기 추가)

### P3b 결과 (2026-08-07) — IDC_STATIC_PATH 경로 UI 구도 반영 (백업 `DlgSubOperate.cpp.bak_rot`)

Cycle Test 1차 실행 피드백 반영. **"Rotate 동작 없음" 판독 결과는 코드 정상**: 09:22 런은 구형 `0_OCP_TEST_Long_260630`(StageR=0)이라 0도 명령이었고, 09:25 런(신형 Long)은 Probe Align에서 사용자 STOP되어 회전 지점(4_PRE_MEAS Move) 전에 종료됨. PreMeas가 STAGE ROTATE를 정상 명령하는 것은 Motion 로그로 확인(09:22:55.236). 조치: 구형 `0_OCP_TEST_Long_260630.rcp` StageR도 0→90(×5)으로 정렬.

경로 UI(DlgSubOperate → CUIMeasPathDlg/ChartOrbit) 구도 변경 4건:
- **E1**: 초기 뷰 `RotateGlassPath(1)`→`(0)` — 기동 시 Load 0도 구도(PDF)
- **E2**: `SetLoaderSize` X/Y 스왑 제거 — 로더 대기 글래스를 0도 자세로 표시(구 90도 세로 표시), 컷 코너 = `m_pnGlassEdgeCut[0]`
- **E3**: 레시피 반영 분기 이진(0/1)→`m_nGlsRotate` 0~3 전체 (범위 가드 포함) — 단변(2)/270(3) 구도 지원 (`RotateGlassPath`는 이미 0~3 처리)
- **E4**: `Config.ini [GLASS EDGECUT]` 코너 배열 -90 시프트: `4,1,2,3`→`1,2,3,4` (원본값 4,1,2,3 — 표시 코너가 어긋나면 이 config만 조정, 리빌드 불요)

### P4 진행 중 발견/조치 (2026-08-07)

- ✅ **장변 90도 회전 정상 확인** (사용자 실측)
- 단변 270도 회전 문제: Short 레시피의 **USER OFFSET `m_fStageRotate`=90 잔재** (구 체계에서 180+90=270을 만들던 값)가 스틱 StageR 180에 가산됨 → `_HEAD_OFFSET_` 2번째 값 90→0 정리. 전 레시피 일괄 점검 결과 다른 레시피는 모두 0. **오프셋은 Recipe UI USER OFFSET 탭의 STAGE ROTATE 항목** — 구 체계 보정치가 남아 있으면 동일 증상 재발하므로 신규 레시피 작성 시 주의

- ✅ **단변 180도 회전 정상 확인** (사용자 실측, USER OFFSET 정리 후)
- **단변 AGING START/END 표시 반전 수정** (2026-08-07, 백업 `DoWorkGlassMeasureStep.cpp.bak_rot2`): AGING 상태는 `m_nAgingStartStick[4]` 스틱 슬롯에 기록되는데, AGING 스텝의 스틱 변환(LYS 2026-07-16)이 `rotate==0`일 때만 반전 — `MakeCellID` 규약(rotate==1만 직행, **0/2/3 전부 `nMaxStick-iStick-1` 반전**)과 어긋나 rotate=2(단변)에서 AGING만 거울 스틱에 표시됨. 조건을 `!= 1`로 정정하여 전 rotate에서 MakeCellID와 일치. Long(rotate=1)은 영향 없음

- **장변 녹색 오버레이(스테이지 글래스 외곽) 불일치 분석/수정 (2026-08-07)**: IDC_STATIC_PATH의 녹색 = ChartOrbit 시리즈0 글래스 외곽(컷 코너 포함), 노란색 = 로더 글래스/경로 라인. 컷 코너 번호 = `ChartOrbit::SetGlassSize` 기준 **1=우상, 2=좌상, 3=좌하, 4=우하**. Config.ini [GLASS EDGECUT]가 09:39 외부 요인으로 원래값(4,1,2,3)으로 되돌아가 있었고, 그 상태의 관찰 결과(0도=4 정상, 180도=2 정상, 90도=1 불일치)로 회전 표현이 **시계 방향**임을 확정 → 올바른 배열 = **{0도:4, 90도:3, 180도:2, 270도:1}**. EDGECUT2 1→3, EDGECUT4 3→1 적용 (E4의 {1,2,3,4} 가정은 오류였음 — 되돌림+실측으로 교정). 리빌드 불요, 앱 재기동 시 반영. ⚠ Config.ini를 되돌린 주체 불명(사용자 수동 복원 추정) — 재발 시 확인 필요

- **셀 ID 라벨 거울상 수정 → ⛔ 원복됨 (2026-08-07 사용자 지시)**: 아래 내용은 구현 후 즉시 원복(.bak_cellid에서 복원, 리빌드 완료). 셀 ID 방향 처리는 재논의 필요 — 분석 결과(원인=인덱스→ID 부여 규칙의 거울상, 수정 지점 4곳)는 유효하므로 기록 유지: 장변 측정 시 물리 첫 측정 셀(-X측)이 I01이어야 하는데 A01로 표기(좌우 반전) — 원인은 측정 좌표가 아니라 **셀 인덱스→ID 부여 규칙**(인덱스 0='A' 오름차순)이 신규 반입 방향과 거울상. 판단: 측정 좌표/순서 유지 + 라벨 규칙만 반전 (레시피 좌표 반전은 측정 방향을 바꿔 부적절, 표시만 고치면 결과/CIM CellID가 물리와 계속 반대). 수정 4곳 (데이터·화면 동일 규칙):
  - `DoWorkGlassMeasureStep::MakeCellID` — rotate==1: 문자부 `'A'+(셀수-1-idx)` / rotate≠1: 숫자부 `셀수-idx` (결과/CIM/MCR ID 원천)
  - `DlgSubOperate::UpdateCellStatus` — 표시 라벨 동일 규칙 (IDC_STATIC_PATH 셀 오버레이 + IDC_STATIC_CELL_MEASURE 공용 데이터)
  - `DlgSubManual_Meas` 수동 측정 2곳 (TSP식 블록 + MakeCellID 클론)
  - 단변(rotate≠1)의 스틱 문자부 역순(nMaxStick-iStick-1)은 검증된 규약이라 유지. 단변 숫자부 반전 = 사용자가 예상한 '상하 반전' 해소 — 실측 재검증 필요
  - ⚠ CellID 체계 변경으로 기존 OOS 테스트 결과와 불연속. "측정 시작측=I01"은 제품 도면 기준 확정값(사용자 승인 2026-08-07)

- **AXIS_Y1~Y4 측정 위치 매칭 수정 (2026-08-07, 승인 후 적용)**: 셀 라벨은 도면 기준 정상 — 반전 대상은 **셀 X좌표(=헤드 Y 목표)**. 셀 좌표는 별도 변환 없이 헤드 Y1~Y4 목표로 직결되며, 생성기(MakeCellDlg:645)가 A01 앵커에서 +방향 누적이라 신규 배치(도면상 A01=+Y측)와 거울. 수정: ① 생성기 셀 분배를 `- fCellOffset - fBlockOffset`(A01 앵커에서 -Y 방향, 백업 `MakeCellDlg.cpp.bak_cellmirror`) ② 기존 레시피 `_CELL_` X행 전체 부호 반전 (Long 12스틱: A01=+641→I01=-641 / Short 21스틱: U01=+313→U05=-313, 백업 `.bak_cellmirror`) ③ 라벨/에어리어/표시/CellID/StageY 무수정. 빌드 OK
  - 기대 동작: 장변 WAD 2헤드 페어 (A01,E01)→(B01,F01)→(C01,G01)→(D01,H01)→(I01 단독) — H2측이 A01부터, 물리·화면·데이터 일치. 5칸 페어(A01,F01)를 원하면 레시피 BACK HEAD GAP 700 설정(조정 범위 632~788, 현재 500→유효 540)
  - 검증 항목: ① 헤드 Y 로그 (+641=A01부터) ② 오버레이 진행 방향 ③ 2헤드 페어링/그룹 순서 ④ 하드코딩 `±50` 그룹 분기 동작 (DoWorkGlassMeasureStep:1965 등)
  - ⚠ 실기 티칭 시: 생성기가 A01 앵커에서 -Y 방향 분배로 바뀌었으므로 MakeCell 재생성 시 앵커(AlignKey+CellCenter)를 **A01 물리 위치(+Y측)** 기준으로 티칭할 것

### P4 체크리스트 (OOS 검증 — 실행 대기)

1. 기동 후 READY→LOAD 전환: Motion 로그 `LOADER R → 0.000` + OOS DEGREE_0(M4400) 비트 확인
2. 장변 레시피(1_OCP_TEST_Long) 시퀀스: PreMeas에서 `LOADER R → 90.000`, X 이동 인터록 무알람
3. 단변 레시피(2_OCP_TEST_Short) 시퀀스: `LOADER R → 180.000`, GlsRotate=2 로 스틱 순서/CellID/ChartOrbit 표시 정합 확인 (U3)
4. MEASURE 상태 전환 시 `LOADER R → 270.000` (MEAS_R) 동작 확인
5. Align 시퀀스(HMI_90.alg 참조 레시피): Stage R → 0 이동 확인
6. RecipeManager OPTION INFO: GLASS ROTATE 콤보 4옵션 표시/선택/저장 확인 (단변 레시피 재저장 시 180° 표시)

## 5. 리스크

| 리스크 | 완화 |
|---|---|
| C-b 멀티 인터록이 신규 각 전부를 차단 → 시퀀스 기동 즉시 알람 | P2에서 필수 수리 + P4 OOS 선검증 |
| 논리 rotate(m_nGlsRotate) 확장값(2/3)이 기존 0/1 분기에 유입 → 스틱 순서/CellID 오배치 | C2 확정 전 콤보 확장 보류, 유입 가드 |
| 실기 DEGREE 센서 배선 불일치 → LOAD 상태 판정 불가 | C4 하드웨어 확인 병행, SW는 각도 기준 매핑(C-c) |
| Align 레시피 각도 미변경 시 Align이 구 각도로 회전 | P1에 포함, P4에서 Align 시퀀스 각도 로그 확인 |
| Resume 저장분(구 각도 기준 GlsRotate)과 신 레시피 불일치 | ResumeInfo 검증 로직(ValidateResume)이 rotate 불일치 감지 — 기존 방어 활용 |
