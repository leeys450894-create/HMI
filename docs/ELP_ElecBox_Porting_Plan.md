# ELP ElecBox Protocol → ALPHA HMI 이식 Plan

| 항목 | 내용 |
|---|---|
| 기준 문서 | `HMI\Doc\07_설계문서\통신문서\ELP_2026\ElecBoxProtocol_SDC HMI(분판검사기) OCP화 개조_20260728.doc` (2026-07-28, 이현도, A6 8.6G GAI 프로토콜[24.08.20] 기반) |
| 참조 구현 | `D:\IT8.6G\GAI\IT_8.6G_HBS_GAI` CCommTcpIpObjectPG 모니터 분기 (분석: `D:\OCP\0.SW_Matching\docs\Protocol_Code_Analysis.md`) |
| 대상 | `HMI\Source\ALPHA` (ALPHA.sln, VS2022 x64 Unicode) |
| 작성일 | 2026-07-29 |

---

## 1. 신규 문서(2026-07-28) 요약 — GAI(2024-08-20) 대비 변경점

| 항목 | GAI 구판 | 분판검사기 신판 | 비고 |
|---|---|---|---|
| S10F01 Len | 112 | **80** | PG1~4 온도/연기 8필드(32byte) 삭제 |
| S10F01 Body | Index+온도/연기 5쌍+전력7종 | Index(2)+ElecBox온도(4)+연기(4)+전력7종(10×7) | 스케일 동일(온도÷10, 전압류÷1000, 적산 정수) |
| 전장박스 수 | 3개 | **2개** (Error Code 기준) | Index 01/02 |
| S20F01/S20F20 | Len+Ack+Cause | 동일 | Len=0 명기 (GAI 코드는 6 송신했음 → **문서대로 "0000" 송신**) |
| S10F20 | 최대 10건 × 5byte | 동일 | — |
| Error Code | 0~30/100~/200~(박스3) + 300~311 + 400~619 | **0~39/100~139**(박스2까지) + **200~207**(소켓) + **300~303/400~403**(상세) | GPS SHUTDOWN·INTERFACE EMS·DC BUFFER 삭제, POWER METER CP·PG RACK DOOR/FAN01~20 신설 |
| Port | 5001(문서)/4501(실제) | 5001 | PARA_10 = 5001 ✓ |

문서 자체 오기(구판 계승, ELP 측 정정 요청 대상):
- §2.2.4 제목 "S10F02 응답" → S10F20 응답
- §2.2.4 표 Function=`01` → `20` (구현은 수신 Function을 그대로 반사)

## 2. ALPHA 현황 조사 결과

- `CCommTcpIpObjectPG`(ALPHA)에는 모니터 분기(m_lOption/eS10F1/eS10F20)가 **없음** → 순수 신규 이식
- Factory(`CommTcpIpFactoryMgr`)는 타입별 **단일 포인터** 구조 (`m_pCommTcpIpObj_PG` 1개) → PG 타입 재사용 불가, **신규 타입 필요**
- `CCommTcpIPMgr`(Manager)가 기동 시 Network.ini 순회하며 `Initialize()` 호출 (USE=1 && OUT_OF_SERVICE=0 조건)
- `CCommEthFunction::GetExeHextoStr()` STX/ETX 프레이밍 재사용 가능 (GAI와 동일)
- Alarm DLL은 `Config\CoreFunction\AlarmConfig\Alarm.xls` 직접 로드 (col1 ID, col3 Level H/L/O, col4 Module, col5 Cmd, col6 Text, col13 USE=1)
- **Heavy 알람 SetAlarm은 호출 스레드를 리셋까지 블로킹** (`AlarmManager.cpp:550`) → 수신 콜백에서 직접 SetAlarm 금지, 알람별 스레드 분리 필요 (GAI가 모니터 스레드 141개를 둔 이유)
- AlarmIDList.h 빈 대역: 9004~9997 → **ELP 대역 9100~9603 배정** (`알람ID = 9100 + Error Code`)
- Network.ini PARA_10 등록됨(사용자, 임시): PG_MON/127.0.0.1:5001/MODE=0(Client)/**MEAS_TYPE=3(→19로 정정 필요)**/OUT_OF_SERVICE=1

## 3. 설계 결정

1. **신규 클래스 `CCommTcpIpObjectPGMon`** (파일 `CommTcpIpObjectPGMon.h/.cpp`, ASCII 신규 파일)
   - GAI처럼 PG 클래스에 옵션 분기를 넣지 않고 분리 — ALPHA Factory의 단일 포인터 패턴과 정합, Protocol_Code_Analysis.md 개선안 #6 반영
   - `CCommEthFunction` 상속, 기존 comm object 생성자 패턴(MgrPool, Network.ini, 로그 Control\PGMON\) 그대로
2. **신규 타입 `enumTCPIpFactorytID_PG_MON`** — enumTCPIpFactorytID_HMI 끝(CIM 다음, =19)에 추가, Network.ini `TCPIP_MEAS_TYPE=19`
3. **알람 경로**: 수신 파싱 → 코드 검증 → 코드별 `AfxBeginThread`로 분리된 raiser 스레드에서 `CAlarmController::SetAlarm(9100+code)` 호출 (Heavy 블로킹이 수신 스레드에 영향 없음, Alarm DLL의 중복 ID 필터가 재발생 프레임 흡수)
4. **데이터 목적지**: 클래스 내부 `m_dElpData[2][9]`(CS 보호) + Getter + OPER 로그. FDC 업로드는 분판검사기에 Ex_FDC 소비자가 없어 **보류** (Getter로 추후 UI/FDC 연결 가능)
5. **GAI 결함 개선 반영** (Protocol_Code_Analysis.md §7):
   - #1/#2 오프셋 누적 버그 → `iStart = 8 + i*5` / 단일 박스 프레임 고정 파싱
   - #4 알람 코드 무검증 → 문서 정의 대역(0~39/100~139/200~207/300~303/400~403)만 허용, 그 외 WARN 로그
   - #5 `delete[]` / memset 괄호 정정
   - #6 자기비교 죽은 코드 미이식
   - Len 필드 검증 추가 → 불일치 시 NAK(Ack=01) 회신 + WARN 로그 (개선안 #4)
   - 수신 시각 기록(`GetLastRecvTick`) — 무수신 워치독은 추후 확장용 Getter만

## 4. Phase 구성

| Phase | 내용 | 파일 | 편집 방식 |
|---|---|---|---|
| 1 | 본 Plan 문서 | docs/ELP_ElecBox_Porting_Plan.md | Write(신규) |
| 2 | 신규 클래스 + enum | CommTcpIpObjectPGMon.h/.cpp(신규), Manager\CommTcpIpDefine.h | Write / Perl |
| 3 | Factory·접속·프로젝트 통합 | CommTcpIpFactoryMgr.h/.cpp, Manager\CommTcpIPMgr.cpp, ALPHA.vcxproj(+filters), Bin\Config\Network.ini | Perl |
| 4 | 알람 등록 | Source\Include\AlarmIDList.h(pch 포함→전체 리빌드), Bin\Config\CoreFunction\AlarmConfig\Alarm.xls(100행) | Perl / Excel COM |
| 5 | 빌드 + damaged=0 검증 + 매핑 보고 | — | MSBuild |

각 Phase 수정 전 `.bak_elp` 백업 생성. 수정 주석 `// LYS 2026-07-29`.

## 5. 문서 ↔ 구현 매핑 (목표)

### S10F01 (ELP→HMI, Len=80)

| 문서 필드 | Byte | 구조체 멤버 | 변환 | 저장 |
|---|---|---|---|---|
| ElecBox Index | 2 | szIndex[2] | _ttoi−1 (0/1) | 박스 선택 |
| ElecBox 온도 | 4 | szTemp[4] | ÷10 ℃ | m_dElpData[box][0] |
| ElecBox 연기 감도 | 4 | szSmoke[4] | 그대로 % | [1] |
| 전압 | 10 | szVolt[10] | ÷1000 V | [2] |
| 고조파 전압 | 10 | szHarmVolt[10] | ÷1000 V | [3] |
| 전류 | 10 | szCurr[10] | ÷1000 A | [4] |
| 고조파 전류 | 10 | szHarmCurr[10] | ÷1000 mA | [5] |
| 누설 전류 | 10 | szLeakCurr[10] | ÷1000 mA | [6] |
| 순시 전력 | 10 | szInstPwr[10] | ÷1000 kW | [7] |
| 적산 전력 | 10 | szAccumPwr[10] | 정수 kWh | [8] |

### S20F01/S20F20 (HMI→ELP ACK)
`STX + "20" + ("01"|"20") + "0000"(Len, 문서 준수) + Ack("00"/"01") + Cause("0000") + ETX`

### S10F20 → 알람
`코드(5자리) → 검증 → SetAlarm(9100+code)`, Alarm.xls ID 9100~9139 / 9200~9239 / 9300~9307 / 9400~9403 / 9500~9503

## 6. 검증 계획

1. 빌드 무오류 + 수정 파일 damaged=0
2. OUT_OF_SERVICE=1 기동: 객체 생성만, 미접속(현행 임시 설정 유지)
3. (선택) 로컬 ELP 시뮬레이터(Python TCP 서버, 127.0.0.1:5001)로 S10F01/S10F20 프레임 송신 → Control\PGMON\PGMON.log 파싱/ACK 확인, 알람 팝업 확인 — OUT_OF_SERVICE=0 전환 필요
