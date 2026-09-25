---
name: hmi-logs
description: ALPHA HMI 런타임 로그 분석 — D:\HBS\Logs 폴더 구조, ALPHA가 쓰지 않는 구 폴더, 0바이트 로그가 정상인 이유, LOG 화면 버튼↔PARA 매핑. 로그로 동작을 검증하거나 LOG 화면을 고칠 때 읽을 것
---

# hmi-logs

출처: `CLAUDE.md` (2026-09-17 /doctor 정리로 이동, 백업 `CLAUDE.md.bak_20260917_doctor`). 전체 규칙(인코딩 편집 규칙, 빌드, 3벌 인터락 원칙 등)은 루트 `CLAUDE.md` 가 우선한다.

### `D:\HBS\Logs` 밑에서 ALPHA가 쓰지 않는 폴더 (2026-09-11 확인)

`DLog\` · `CLog\` · `Index\` · `ERRORLOG\` 는 **ALPHA가 만들지 않는다** — 전신 프로그램(Metrosys 계열)의 산출물이며 **2026-06-25를 끝으로 갱신이 멈춰 있다**. `DLOG-<일>.log` 내용이 `****** Metrosys Started ******` / `InitPermitFunc()` 이고, `ALPHA.exe` 바이너리에 `DLog\`·`CLog\`·`Index\`·`Metrosys` 문자열이 **ASCII/UTF-16 모두 0건**이다(소스에도 없음 — `dfDEFAULT_DLOG_FILE_PATH`/`dfDEFAULT_CLOG_FILE_PATH`는 `CommonFunc.h`에 정의만 남고 쓰는 곳이 전부 주석). "6월 이후 로그가 안 쌓인다"는 문의는 이 폴더들을 보고 나온 것이므로, **현행 로그가 어디에 쌓이는지부터 확인**할 것.

현행 ALPHA의 로그 출력(모두 정상 갱신 중):

| 종류 | 경로 | 쓰는 곳 |
|---|---|---|
| 메인(D LOG 버튼 계열) | `Logs\<시스템명>_yyyymmdd.log`, `Logs\LogFile\*`, `Logs\Control\*` | `CLogWrapper::m_file`, 시퀀스/매니저별 `CLogFileAccessor` |
| O LOG | `Logs\OLOG_yyyymmdd.log` | `CLogWrapper::m_fileO` (`spdlog::daily_logger_mt("OLOG", …)`) |
| T LOG(Tact) | `Logs\TACT TIME\tact_time_yyyymmdd.log` | `CSeqTactTimeMgr` |

- **메인 로그 파일명 앞부분은 시스템명이라 바뀔 수 있다.** `AuroraMgr`가 `logFile.Format(_T("%s%s.log"), 로그경로, strName)`으로 만들며, 이 설비는 2026-06월경 `HMI` → `OCP`로 바뀌어 `HMI_2026mmdd.log`가 끊기고 `OCP_2026mmdd.log`가 시작됐다. **이름이 바뀐 것이지 로깅이 멈춘 것이 아니다.**
- `CLogFileAccessor::Initialize`는 넘겨받은 `...\이름.log`의 `.log`를 `_yyyymmdd.log`로 치환하고, `KMLOG` 때마다 날짜가 바뀌었으면 스스로 재오픈(롤오버)한다. 그래서 **경로에 `.log`가 없으면 날짜가 안 붙는다.**
- LOG 화면의 `Folder Open` 버튼은 개별 로그 위치가 아니라 `CSystemParamShare::GetLoggerFolderPath()`(= `Logs\` 최상위) 하나만 연다.
- **D LOG 화면에 모인 줄은 `Logs\LogFile\DLOG_yyyymmdd.log` 에 한 벌로도 저장된다 (2026-09-11 신설).** DLOG 창은 초기화·Glass 9단계·EQ State·SACC·CIM 등 **accessor 여러 개가 한 화면으로 모이는** 구조라 각자의 파일은 있어도 **시간순으로 합쳐진 한 벌이 없었고**, 화면 리스트는 100줄이 넘으면 앞에서부터 지워져 추적이 끊겼다. `CDlgSubLogFile::EnableFileSave(경로)` 를 부른 창만 `OnGuiLog` 에서 표시한 줄을 그대로 파일에 덧쓴다(`CFileAccessor`, `"a+"` + 매 줄 `fflush`, 날짜 바뀌면 `__SaveToFile` 이 `_yyyymmdd.log` 로 재오픈). 현재 호출은 `DlgSubLog.cpp` 의 DLOG 한 곳뿐이며, 다른 창(TLOG/MOTION/IO…)도 필요하면 같은 한 줄로 켤 수 있다. **기존 accessor별 파일은 그대로 유지**되므로 내용은 중복 보관된다.

#### 빈 로그 파일(0바이트)은 대부분 정상이다 — 갱신 시각이 아니라 **크기**로 볼 것

`CLogFileAccessor::Initialize()`가 **기동 시점에 `fopen(..., "a+")`로 파일을 먼저 만든다**(`CFileAccessor::Open_Stream`). 그래서 그 기능이 한 번도 안 돌아도 **매일 0바이트 파일이 생기고 갱신 시각만 오늘 날짜로 찍힌다.** 파일 목록의 timestamp만 보고 "로그가 쌓이고 있다"고 판단하면 안 된다(2026-09-11 실제로 이렇게 잘못 판단한 전례). 확인은 `ls -l`의 크기 또는 `find ... -size +0`으로 한다.

2026-09-11 기준 `Logs\LogFile\`에서 **한 번도 내용이 찍힌 적 없는** 파일들과 이유:

| 로그 | 이유 |
|---|---|
| `measure_seq_item_engine_*` | 아래 항목 참조 |
| `AutoContact_*` | AutoContact 미사용(스텁) |
| `GLASS 3_GLASS_ALIGN_REAR_*` | REAR Align 미사용 |
| `SEQ CIM_*` · `SEQ SACC_*` | CIM 시퀀스 / SACC(프로브 자동교체) 시퀀스 미실행 |
| `MonitorToProcs_*` · `SEQ MonitorToFront/Rear_*` · `SEQ PROCS_STEP_*` | 미사용 |

`GLASS n_*` / `GLASS Sequence_Step_*` 이 0바이트인 날은 **그날 글래스 사이클이 안 돈 날**이다(초기화·수동 조작만 한 날). `SEQ INITIALIZE_*` · `SEQ EQSTATE_CHANGE_*` 는 매일 기록되므로 이 둘로 로깅 자체의 생사를 판단하면 된다.

**`measure_seq_item_engine_*.log` 이 항상 비는 이유 (2026-09-11 확인).** `CMeasureSequenceItemEngine`은 `CDoWorkGlassSequenceEngine`과 이름이 비슷하지만 **다른 클래스이고, 로그를 남기는 경로가 현재 전부 닫혀 있다**:
- 이 클래스의 `ItemStart` / `ItemStopAndSkip` / `ItemStopAndExit`(`Seq Item : … - Start/End` 를 찍는 함수)는 **호출부가 한 곳도 없다.** 실제로 도는 것은 동명의 `CDoWorkGlassSequenceEngine::ItemStart` 쪽이며 그 로그는 `GLASS Sequence_Step_*.log` 에 남는다.
- 남은 기록 경로는 에이징 사이클뿐인데 `AgingCycleBegin()`이 **OCP가 아니거나 레시피 `m_sInspectItem.m_bUseAging_Test == FALSE` 면 KMLOG 이전에 return** 한다. 현재 레시피는 이 플래그가 꺼져 있어 `AgingCycle: begin …` 한 줄도 안 남는다.
- `MakeUniqueInnerID()`의 InnerID 중복 WARN 도 중복이 실제로 나야 찍힌다.
- 이 엔진은 **`DoWorkGlassMeasure`와 `DoWorkGlassAGING`이 각각 new 해서 인스턴스가 2개**이고 **둘 다 같은 파일 경로**를 `Initialize` 한다. `"a+"` + 매 write `fflush` + `_SH_DENYNO` 라 깨지지는 않지만 두 시퀀스의 줄이 한 파일에 섞인다.

#### LOG 화면(IDD_DLG_SUB_LOG) 버튼 ↔ PARA 매핑 (2026-09-11 정리)

버튼 배치와 라벨과 내용이 **서로 다른 배열 3개**로 갈려 있어 셋이 어긋나기 쉽다. `CDlgSubLog::OnInitDialog`:
- `nBtnIndex[i]` = 화면 배치 순서(i) → 컨트롤 ID. `nNum[i]` = 그 버튼의 **`enumLogItemID` 값**. `LogName[]`은 **enum 값으로 인덱싱**(배치 순서 아님).
- `bUseBtn[i]`는 **배치 순서 i**로 인덱싱하는데 꼬리 주석은 `LogName`에서 항목이 주석 처리(`면계측`·`TSP / LCR`)되며 밀린 옛 이름 그대로다 → **`bUseBtn` 주석을 믿지 말고 `LogName[nNum[i]]`로 실제 라벨을 계산할 것.** 예: `bUseBtn[8] // 색이상`이 실제로는 "시야각" 버튼(MEAS2), `bUseBtn[11] // PG`가 실제로는 "AGING" 버튼(MEAS5).
- **`LinkParam`에 넘기는 PARA 번호가 `Network.ini` 개편을 따라가지 못해 라벨과 내용이 어긋나 있었다.** `Network.ini`에서 `면계측`(구 PARA_2/3)·`LCR`/`VISMOTION`(구 PARA_8)이 주석 처리되고 뒤 항목이 당겨지면서 코드의 하드코딩 인덱스가 통째로 밀렸다. 2026-09-11 교정: AGING 버튼(MEAS5) PARA_6(=CIM) → **PARA_8**, 특성 버튼(MEAS4) PARA_7(=SYSTIME) → **PARA_4(WAD)**, PG 버튼(MEAS3)에서 **PARA_4 제거**(PARA_5만), CIM 버튼 **PARA_6 추가**, 시야각 버튼 `bUseBtn[8]=true`(숨김 해제).
- **`CDlgSubLogFile::LinkParam`은 accessor의 `SetLogMsgWindow`를 덮어쓴다** — 한 PARA를 두 항목에 링크하면 **나중에 실행된 쪽이 이기고 먼저 링크한 버튼은 빈 화면**이 된다. PARA를 옮길 땐 반드시 옛 링크를 제거할 것(AGING을 MEAS5로 옮기며 숨김 MEAS6의 PARA_8 링크를 주석 처리한 이유).
- **`TCPIP_COUNT`(현재 10) 범위 밖 PARA는 조용히 `nullptr`**: `GetParam_MeasType`이 −1을 반환 → `GetLogFileAccessorPtr`의 `default` → `LinkParam`이 nullptr를 무시. 코드에 남은 PARA 10~18 링크(TSP 6개 루프 포함)는 전부 무동작이며, 이 때문에 **CIM 버튼이 오랫동안 비어 있었다**. 팝업도 로그도 없으니 "버튼이 안 뜬다"가 아니라 "버튼은 있는데 내용이 없다"로 나타난다.
- `GetLogFileAccessorPtr(nID)`의 `switch (lMeasType)`도 23벌 중 하나다 — MEAS_TYPE을 추가하면 이 함수도 채울 것(2026-09-11 PG_MON 누락으로 PG MON 버튼이 nullptr였다).

**현재 확정 매핑 (2026-09-11 기준).** 이 설비의 측정 3종은 **화질=VISION / 시야각=ANGLE / 특성=WAD** 이다.

| 버튼(배치 i) | enum | 라벨 `LogName[]` | PARA | Network.ini NAME |
|---|---|---|---|---|
| 7 | MEAS1 | 화질 | 0, 1 | 화질1, 화질2 |
| 8 | MEAS2 | 시야각 | 2, 3 | 시야각1, 시야각2 |
| 9 | MEAS3 | PG | 5 | PG |
| 10 | MEAS4 | 특성 | 4 | WAD |
| 11 | MEAS5 | AGING | 8 | AGING |
| 12 | MEAS6 | (숨김) | — | — |
| 13 | MEAS7 | PG MON | 9 | PG_MON |
| 6 | CIM | CIM | 6 | CIM |

**PARA_7(SYSTIME)은 대응 버튼이 없다** — 화면에 안 뜰 뿐 로그 파일은 `D:\HBS\Logs` 밑에 계속 쌓인다. 필요하면 숨김 슬롯 MEAS6(`bUseBtn[12]`)에 라벨(`LogName[14]`)을 주고 링크하면 된다.

상세: [docs/02_CallGraph.md](docs/02_CallGraph.md)


### 모션 로그 읽기 (2026-09-18 CLAUDE.md 에서 이관 — 내용 무변경)

- **`DoWork_MotionDone(iAxis)`는 "완료 대기"가 아니라 "완료 대기 + HW 리밋 센서 판정"이다** (`MotorControlMgr.cpp:3174`). 이동 완료 후 `GetHWLimit(±)`를 읽어 실패하면 `MOTION_OVER_LIMIT_POS(3501)`/`_NEG(3601)` 알람을 직접 발생시킨다. 그래서 **모션 로그의 `DoWork_MotionDone NG(AXIS:n)` 한 줄이 리밋 사고의 1차 증거**이며, 직전 `MoveCmdPosM Move(...)` 줄의 목표값이 원인이다(비활성 축·OOS·미연결은 성공 반환).
- **로그의 `AXIS:n`은 축 이름이 아니라 `axis_type_t` 값이다** (`MoveCmdPosA/M Start/End`는 이름, `MotionDone`·일부 `Move`는 숫자). 사이클 로그에 자주 나오는 값: **0**=STAGE X, **3**=X2 BACK GANTRY, **4**=X3 FRONT GANTRY, **7**=Y1, **8**=Y2, **9**=Y3, **10**=Y4, **18**=Z1, **19**=Z2, **20**=Z3, **21**=Z4, **22**=Z5, **23**=Z6, **50**=LOADER_R, **77**=PROBE_X9, **83**=PROBE_Y5, **88**=PROBE_Z5. (`AXIS_X0/Y0/T/R0` 등 alias는 값을 공유하고 번호를 소비하지 않는다.)
- **모션 로그는 "지령값"만 남기고 실제 엔코더값은 기록하지 않는다.** `MoveCmdPosA/M/R Start·End`의 `POS:`는 전부 명령 목표다. 과거 시점의 축 위치를 알아내려면 **`MoveHome CheckHomeDone`(→`.dat`의 `HOME[]`) 을 기준점으로 잡고 이후 `MoveCmdPosA End … RET:1`(절대 대입) / `MoveCmdPosM Move`(멀티, 한 줄에 축·좌표 쌍 나열) / `MoveCmdPosR End … RET:1`(상대 누적)을 순서대로 재생**해야 한다. `RET:1`이 아닌 줄은 거부된 지령이므로 반드시 제외. 절대 지령·호밍이 기준인 축은 신뢰도가 높고, 조그(`MoveCmdPosR`)만 있던 축은 누적 오차가 쌓인다. **X9를 조그하면 X10도 함께 나가지만 로그엔 X9만 남으므로 X10은 재구성 불가**(X9와 같다고 보면 된다). 이동 중 알람으로 멈춘 축은 지령값과 실제가 다르다.
