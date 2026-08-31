# HW 연동 검증 체크리스트

작성 2026-08-27. 대상: ALPHA HMI (분판검사기, OCP 개조). 소스: 메모리/CLAUDE.md/코드의 미검증 항목 전수 취합.

**우선순위 표기**: ★ = 필수(안전·신규기능 직결, 첫 방문에 반드시) / ○ = 권장(기능 완결) / △ = 정보(확인만, 조치 불필요할 수 있음)

**진행 표기**: 각 항목 앞 `[ ]`에 결과 기입 — `[OK]` / `[NG-메모]` / `[보류]`

---

## 0. 사전 준비 (출발 전 사무실에서)

- [ ] ★ **배포 3종 세트 일치 확인** — ALPHA.exe / FDCSense.exe / MCCLog.exe.
  공유메모리 매핑명이 `KMAC_...` → `HBS_Shared_Memory_For_FDC_V2`로 변경되어 **2022 배포 구형 FDC와 비호환**. 신 ALPHA를 올리면 FDC도 반드시 동반 교체. (MCCLog.exe는 VTM_MCC 레이아웃 불변이라 기존 배포본 호환)
- [ ] ★ **Config 스냅샷 백업** — 테스트 시작 전 아래 파일 통째로 복사(끝나고 diff):
  `Config.ini`, `CoreFunction.ini`, `Config_Motion_ACS.dat`, `Network.ini`, `Position.ini`, `Config_FDC.ini`, `Config_MCC.ini`, `FDC_LIST.xls`, `MCCNAME.csv`
  ※ 과거 Config.ini EDGECUT가 외부 요인으로 되돌아간 전례 있음 — 값 이상 시 파일부터 확인.
- [ ] ★ **테스트 픽스처 잔재 원복 확인**:
  - AgingRcp `PTN_SEC` — OOS 검증용 5초로 줄였던 것, 실사용 **100** 원복 여부
  - MultiMeas 3점 픽스처(`ENABLE=1,1,1`) — 검증용, 운용 레시피와 분리 확인
- [ ] ○ **Network.ini 상태 점검**: PARA_0~9 OOS 플래그(시뮬레이션 검증 때 1로 둔 항목), PARA_10(ELP PG_MON) `IP=127.0.0.1` → **실장비 IP로 교체**, `TCPIP_COUNT=11` 유지.
- [ ] ○ 노트북에 **Wireshark** 설치 + 시야각 모듈 통신 캡처 준비(§5.3), 스틱/레시피는 검증용 Long(90°)·Short(180°) 두 벌 준비.

**로그 위치 (현장에서 볼 곳)**

| 로그 | 경로 |
|---|---|
| 모션/인터락 (기동 시 INTERLOCK 설정값도 기록) | `D:\HBS\Logs\Control\Motion\Motion_YYYYMMDD.log` |
| PG 송수신 | `D:\HBS\Logs\Control\PG\PG_YYYYMMDD.log` |
| ELP ElecBox 수신 | `D:\HBS\Logs\Control\PGMON\PGMON_YYYYMMDD.log` |
| WAD 측정 | `D:\HBS\Logs\Control\WAD\WAD_*.log` |
| 글라스 시퀀스 | `D:\HBS\Logs\LogFile\GLASS 6_MEASURE_*.log` |

---

## 1. 기동 / 이니셜라이즈

- [ ] ★ **1.1 이니셜 완주 (Step 0~22)** — 예외/크래시 없이 완료.
  특히 **Safety 스텝**: 2026-08-26 수정한 `MoveCmd_SafetyAll` 미초기화 배열 크래시(`i=4` 예외)가 재발하지 않는지. 미장착 축은 `PackUsedAxes`가 걸러 SUCCESS로 통과해야 함.
- [ ] ★ **1.2 X9/X10 호밍** (이니셜 case 6):
  - HOME 위치 X9=-863.188 / X10=-862.2992, HPLC 둘 다 16 → **마스터 홈 1회에 슬레이브까지 홈** 완료되는지
  - P1~P4 갠트리는 `bUse=FALSE` 스킵으로 **에러 없이 건너뛰는지** (루프 중단 없음)
- [ ] ★ **1.3 호밍 후 X9 파킹 위치 = 0(±1mm)** — P5 간섭 인터락(§3)의 전제 조건. 0에서 벗어나면 밴드 기준점이 밀림.
- [ ] ○ **1.4 MCCLog.exe 자동 기동** — ALPHA 기동 후 13초 내 재기동 확인(강제 종료 테스트 1회).

## 2. Probe Gantry X 단독 모션 (수동, 저속부터)

- [ ] ★ **2.1 Maintenance 축 목록** — X9 노출 확인. X10은 목록에 **없는 것이 정상**(슬레이브, 의도적 제외). Move 팝업도 동일.
- [ ] ★ **2.2 X9 실 스트로크 대조** — SW 리밋 **-51.0 ~ 0.1**. 저속(SPEED_LOW)으로 -10 → -30 → -50 순 이동, 기구 간섭·리밋센서 위치와 대조. 구 HW(MN -53 / MX 795.1)와 좌표 대역이 완전히 다르므로 **첫 이동은 반드시 소폭**.
- [ ] ★ **2.3 M-S 정렬** — X9 이동 시 X10 추종 확인(Master-Slave). 이동 후 두 축 편차 확인.
- [ ] △ **2.4 AXIS_R1(Probe Rotate) 미사용 확인** — 호밍/이동 대상에서 빠져 있는지(폐기됨). AXIS_PROBE_Y5도 코드상 비활성 유지.

## 3. P5 구조물 간섭 인터락 ★핵심 신규 — 반드시 저속/단독 상태에서

파라미터: `CoreFunction.ini [MOTOR_CONTROL_MGR]` — `INTERLOCKPROBESIZEX=300.0` / `INTERLOCKPROBEZSAFETY=-600.0` (둘 다 **잠정값**)
밴드 = [X9-1 .. X9+300] (X9 파킹 0 기준 ≈ -1~300). Z1은 **+방향이 하강**(안전 -600 → 측정 0).
알람 = **3778** (MOTION_INTERLOCK_CHECK_ALARM 3701 + AXIS_PROBE_X9 77).

- [ ] ★ **3.1 파라미터 실측 대조** — P5 유닛 X방향 실폭이 300mm인지, Z1=-600이 실제로 P5 상단을 여유 있게 회피하는 높이인지 **기구도/실측 대조 후 INI 확정**. (틀리면 아래 전부 무의미)
- [ ] ★ **3.2 Z1 하강 거부 (점 판정)** — X2를 밴드 내(예: 150)에 두고 Z1 하강 지령:
  - 기대: 이동 거부 + 알람 3778 + Motion 로그 `MoveCmdPosA_Sub Z1 DOWN NG - PROBE UNIT(X9:.., SIZE:..) vs BACK GANTRY(X2:..)`
  - 상대이동/멀티이동 경로도 각 1회 (`MoveCmdPosR_Sub` / `MoveCmdPosMulti_Sub` 동일 문구)
- [ ] ★ **3.3 Z1 상승은 항상 허용** — 같은 위치에서 Z1 상승(-방향) 지령 → 정상 동작.
- [ ] ★ **3.4 X2 이동 시 Z1 선상승 (경로 판정)** — Z1을 내린 상태(예: -100)에서:
  - (a) 밴드 밖 → 밴드 안: X2 500 → 100
  - (b) 밴드 횡단(목표는 밴드 밖): X2 350 → -15
  - 기대 둘 다: 로그 `... X2 path crosses PROBE UNIT band(X9:.., SIZE:..), X2 .. -> .. - retract Z1 .. -> -600` → **Z1 -600 상승 완료 후** X2 이동. Z1 상승 실패 시 X2 이동 포기되는지도 확인.
  - (c) 회귀: 경로가 밴드 전혀 안 걸치는 이동(X2 400 → 800)은 Z1 그대로 즉시 이동.
- [ ] ★ **3.5 특성 측정 정상동작 충돌 여부 — 최우선 우려 항목**
  특성 측정 시 X2=82.84(밴드 내). WAD 측정 1사이클 실행 → 알람 3778 또는 `MoveCmdPosWAD Z1 DOWN NG` 발생 여부 확인.
  - 발생 시: **정상 측정을 인터락이 막고 있는 것** → 실측 Z1 하강 깊이 vs P5 높이를 재서 밴드 폭(`INTERLOCKPROBESIZEX`) 축소 또는 판정 방향 재설계 필요. 현장에서 임시로 INI 값 조정 가능하나 **반드시 실측 근거 기록** 후 조정.
  - 미발생 시: 정상 측정의 Z1 목표가 현재보다 위(상승)라는 뜻 — 설계 의도대로.
- [ ] ○ **3.6 시퀀스 계층 인터락** — Review/Aging 경로도 각 1회 (`MoveCmdPosReview Z1 DOWN NG` / `MoveCmdPosAging Z1 DOWN NG` 문구).
- [ ] ○ **3.7 Z2(FRONT Z)는 인터락 없음이 정상** — 전제 "X3 스트로크는 P5와 무간섭"을 **실기에서 육안 확인**(X3 전 구간에서 FRONT Z 최하강 시 P5와의 여유 거리). 전제가 틀리면 Z2에도 밴드 체크 추가 필요.
- [ ] △ **3.8 Z2 분기 X2 범위 체크 교차 의심** — Z2 이동 조건에 `X2 < MinSWLimit(X2)+35` 검사가 남아 있는데 짝이 X3라면 이것도 교차 배선일 수 있음(코드 확인만 된 상태). FRONT 헤드 동작 이상 시 이 지점부터 확인.
- [ ] △ **3.9 죽은 검사 2건 (조치 불필요, 인지만)** — Loader Z↔X9 및 SafetyAll Lift pin 인터락은 임계값 575mm 기준이라 X9 최대 0.1mm에서는 절대 안 걸림. 향후 X9 대역이 바뀌면 재검토.

## 4. 측정 시퀀스 (Auto / Cycle)

- [ ] ★ **4.1 특성(WAD) 표준 측정 완주** — 주의: WAD 갠트리 축이 `AXIS_X`(no-op) → **`AXIS_X2` 통일**로 바뀌어 **X2가 실제로 움직이기 시작한 상태**. 첫 사이클은 저속/비상정지 대기하며 관찰. §3.5와 동일 런에서 확인 가능.
- [ ] ○ **4.2 WAD MultiPoint 오프셋 실장비 검증** (Phase 6 잔여) — 다점 측정 시 색좌표 오프셋(R_X/G_X/B_Y 가산)이 결과에 정상 반영되는지. 프레임 len 132(nTotal/nCurrent 포함) 장비측 수신 정상 여부.
- [ ] ○ **4.3 Glass 회전 잔여 검증** (rotate 개편 08-07):
  - 장변 90° / 단변 180° 측정 정상(기존 실측 완료 — 회귀 확인만)
  - Y1~Y4 페어링: 헤드 Y 로그 +641=A01부터, 페어 (B01,F01)... 순서, `m_fCellX ±50` 하드코딩 분기 영향
  - 경로 UI 컷 코너: EDGECUT {0°:4, 90°:3, 180°:2, 270°:1}
- [ ] ○ **4.4 Aging Cycle 실기** — OOS 검증 완료분의 실기 재현:
  - PG 점등 순서: TurnOn(`08010100`) → Next(`08010300`)×(PTN-1) → PtnSec 대기 → Off(`1003`) — PG 로그 대조
  - 회차 분리: 결과 행 블록(105행 단위) 충돌 0, InnerID 중복 0
  - 진행 하이라이트(노란 셀) 육안 확인 — AgingTest START 기동 시에만 표시 (유일한 미확인 항목)
  - △ AREA_MAX_CNT 1000 대비 현재 최대 945행(94.5%) — 아이템/다점 추가 시 초과. 레시피 확장 전 반드시 계산.
- [ ] ○ **4.5 AgingSet 알람 경로** — 7495~7497. `AGING.txt`는 **외부 PG 프로그램이 생성**(레지스트리 `DIR\AGING_FILE` 경로). 실기에서 PG가 실제로 파일을 쓰는지 + 미생성 시 7496 발생 확인.
- [ ] ○ **4.6 AutoContact (수동 전용)** — OOS 잔여 + 실기:
  - OOS 잔여분: 콤보 ALL/1~N 표기, 단일 스틱 START(이동+Done+탐색+진행표시), ALL 일괄, STOP, 셀 PASS 표기(해당 스틱), 알람 경로(FLIMIT 축소 → 1501)
  - 실기: **ReadyPos 재티칭 필수**(현 값은 OCP UNIT_Z 좌표 이관분 — 그대로 쓰면 위험, 저속·소폭으로 탐색 시작), PG R-Test 응답 "8"(신호선불량 → 28) 실수신 파싱 확인

## 5. 통신

- [ ] ★ **5.1 시야각 모듈 프로토콜 세대 판별** — 실기 접속 시 Wireshark 캡처(또는 접속 로그):
  - 구형 dot 방식: ASCII `MOT.A1.GRAB.START` 형태 → 현행 HMI 구현 그대로 사용 가능
  - Vicon2.0: `STX + 제어코드 + CRC16` 바이너리 → **프로토콜 이식 필요**(별도 작업 착수)
  - 패킷 몇 개만 캡처하면 즉시 결론. **테스트 당일 최우선 확보 대상.**
- [ ] ○ **5.2 ELP ElecBox 실장비** — IP 교체 후:
  - 접속 + S10F01 수신 로그(`PGMON_*.log`, `S(10),F(01),LEN(00080)`), ACK `Len 0000/Cause 0000`
  - FDC 공유메모리 슬롯 400~408/420~428 값 반영 (`docs/tools/fdc_sharemem_reader.py` — 매핑명 HBS)
  - 알람: ID = **8550 + Error Code**. 가능하면 ELP측 테스트 알람 강제발생 의뢰(질의서 D항, 미회신)
  - △ B트랙(MCC 컬럼 매핑)·C트랙(FDC_LIST DESTINATION 실주소, 현재 D20000 임시 = PLC 쓰기 스킵 상태) — **회신 전 구현 금지** 유지, 회신 받으면 사전 준비분 즉시 적용
- [ ] ○ **5.3 PG 실장비** — R-Test(PG_TEST_3) 응답 셀별 파싱(21=OK/28=신호선불량), 타임아웃 레시피 `m_nTimeOut[1]` 적정성.
- [ ] ○ **5.4 PLC** —
  - 기동 시 CSV 검증 메시지(유효 행수 ≠ enum 수 AfxMessageBox) **미출현** = 정상
  - DEGREE 센서 M4400~4403 (0/90/180/270) 실비트 확인
  - △ M4338 주소 충돌(TEMP_SENSOR_16 vs RESET_SWITCH_16, 둘 다 미사용) — PLC 담당에게 현장 확인
  - 독립 확인이 필요하면 pymcprotocol로 M/D 직접 읽기(SW 무관 절연 진단)

## 6. 데이터 / 외부 프로세스

- [ ] ★ **6.1 FDC 3자 일치** — ALPHA/FDCSense(D) 모두 매핑명 `HBS_..._V2`. 현장에 **구 KMAC FDC가 남아 있으면 데이터 무발행**(조용히 실패) — 배포 시 반드시 교체 확인. 리더 스크립트로 슬롯 발행 확인 가능.
- [ ] ○ **6.2 MCC 실기동** —
  - `Config_MCC.ini USE=1` 상태에서 T-Log/I-Log CSV 생성 + FTP 확인
  - USE=0 완전 비활성(잔여 검증 항목) — 로그/큐 접근 전무 확인
  - I-Log dData 15슬롯(온도 3 + ELP EB1/EB2 각 6) 값 유입 — ELP 실접속(§5.2)과 연동
  - △ MCCNAME.csv 변경 시 **MCCLog.exe 재시작 필요**(자기 시작 시점 파일명 유지)
- [ ] ○ **6.3 결과/APD** — 실기 측정 후 raw 파일 덮어쓰기 없음(InnerID 유일성), APD `[RAW_DATA]` 행 수 = 측정 수 확인. △ DLL/APD Format 회신 대기 중(주간보고 3p 4번, 변동 없음).

## 7. 재티칭 필요 목록 (실기에서만 가능)

| 항목 | 현재 값의 출처 | 위험도 |
|---|---|---|
| AutoContact ReadyPos 등 `[PROBE_AUTO_CONTACT]` | OCP UNIT_Z 좌표 이관 | ★ 그대로 사용 금지 — 저속 재탐색 |
| 측정 레시피 세부 티칭값(`m_fStageR`, Cell 좌표) | 구 좌표계 OOS 값 | ★ 실기 재티칭 |
| MakeCell 앵커 | A01 물리(+Y) 기준 | ○ 생성 시 기준 준수 |
| P5 간섭 파라미터(SIZE 300 / SAFETY -600) | 잠정값 | ★ §3.1에서 확정 |
| ANGLE 관련(레지스트리 ANGLE_PARAM) | OCP 기구학 값 | △ 이식 보류 중 — 통신 사양서 대기 |

---

## 부록 A. 신규 인터락 로그 문구 (grep 용)

```
Z1 DOWN NG - PROBE UNIT        ← 하강 거부 6경로 공통 (A_Sub/R_Sub/Multi_Sub/WAD/Review/Aging)
X2 path crosses PROBE UNIT band ← 경로 판정 선상승 (RetractHeadZForProbeUnit)
retract Z1                      ← 선상승 실행부
InnerID duplicate remapped      ← InnerID 충돌 재매핑 (WARN, 발생 자체는 정상 처리)
```

## 부록 B. 관련 알람 번호

| 알람 | 의미 |
|---|---|
| 3778 | Z1 하강 인터락 (3701 + AXIS_PROBE_X9=77) |
| 3021~3029 | MOTION_INTERLOCK_1~9 (3027 = X2↔X3 Cross 갭 385mm) |
| 1500~1502 | AutoContact (PORT/1STEP/2STEP FAIL) |
| 7495~7497 | AgingSet |
| 8550+code | ELP ElecBox Error Code |

## 부록 C. 관련 INI/설정 파라미터

| 파일 | 키 | 값(현재) |
|---|---|---|
| CoreFunction.ini [MOTOR_CONTROL_MGR] | INTERLOCKPROBESIZEX | 300.0 (잠정) |
| CoreFunction.ini [MOTOR_CONTROL_MGR] | INTERLOCKPROBEZSAFETY | -600.0 (잠정) |
| Config_Motion_ACS.dat PBX9/PBX10 | MN/MX/HOME | -51.0 / 0.1 / -863.188·-862.2992 |
| Network.ini PARA_10 | IP/PORT/MODE | 127.0.0.1→실장비 / 5001 / 0(Client) |
| Config.ini [GLASS EDGECUT] | 배열 | {4,3,2,1} (0/90/180/270°) |
| Config.ini [PROBE_AUTO_CONTACT] | ReadyPos 등 | OCP 이관값 — 재티칭 |
| 레지스트리 DIR\AGING_FILE | 경로 | PG 프로그램이 기록 |
