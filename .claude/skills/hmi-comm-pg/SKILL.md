---
name: hmi-comm-pg
description: ALPHA HMI TCP/IP 통신 팩토리(MEAS_TYPE 분기, switch 23벌)와 PG 패턴발생기 프로토콜(S8F1/S10F3, 로그 검증). 통신 객체 추가나 PG 동작을 다룰 때 읽을 것
---

# hmi-comm-pg

출처: `CLAUDE.md` (2026-09-17 /doctor 정리로 이동, 백업 `CLAUDE.md.bak_20260917_doctor`). 전체 규칙(인코딩 편집 규칙, 빌드, 3벌 인터락 원칙 등)은 루트 `CLAUDE.md` 가 우선한다.

### Communication (TCP/IP Factory)

14종 장비 통신 — Factory 패턴 (`CCommTcpIpFactoryMgr`, `Source/ALPHA/Manager/CommTcpIpFactoryMgr.cpp`, ~2,400줄):
Vision, Color, WAD, Angle, PG, AgingPG, LCR, TSP, CIM, SysTime, Motion, UTG(x2), **PGMon**(ELP ElecBox 모니터링, 2026-07-29 추가)
모두 `CCommEthFunction` 상속, 개별 send/receive 프로토콜 구현.

- **팩토리 분기 = `MEAS_TYPE`** (PARA 번호도 NAME도 아님): `Network.ini`의 `NETWORK_PARA_N_TCPIP_MEAS_TYPE` 값이 `enumTCPIpFactorytID_*`([CommTcpIpDefine.h])와 대응. `CreateComm(nID)`/`CCommTcpIPMgr` 루프 모두 `GetParam_MeasType(nID)`로 `switch`. `NAME`(예 `SYSTIME`)은 `SetName()` 라벨(로그 폴더/태그)일 뿐 분기에 안 쓰임. PARA 번호는 `m_vecParam[]` 배열 인덱스로 IP/Port를 읽을 때만 사용. (예: `NETWORK_PARA_7 NAME=SYSTIME, MEAS_TYPE=10` → `enumTCPIpFactorytID_TIME` → `CCommTcpIpObjectSysTime`.)
- **새 MEAS_TYPE(통신 객체)을 추가하면 `CommTcpIpFactoryMgr.cpp`의 `switch (lMeasType)` 23벌을 전부 점검할 것.** `CreateComm/DeleteComm/DisConnect/SetTcpConnection`만 채우고 `IsConnected(nID)`·`IsOutOfService(nID)`를 빠뜨리면 `default → false`로 떨어져 **연결돼 있어도 타이틀바 램프(`DlgTitleBar::SetConnectStatus`)가 빨간색**으로 남는다(2026-09-07 PG_MON=19 사고, 알람은 정상 수신 중이었음). 램프 색: 파랑=OUT_OF_SERVICE, 초록=IsConnected, 빨강=그 외.
- `OUT_OF_SERVICE=1`이면 `Cmd_Send`가 실제 송신을 **SKIP**(시뮬). `MODE`는 `bRunMode`로 전달(예: SysTime MODE=1 = HMI가 서버).
- **PG(패턴 발생기) 프로토콜 — `CommTcpIpObjectPG.cpp`, 사양서 `Doc/07_설계문서/통신문서/ELP-ICP-PG-Control-LanProtocol(SDC, 160310).pdf` (2026-09-14 정리).** 패킷은 `STX + SS FF [본문] + ETX`(스트림/펑션 각 2자리 ASCII), `SetCommand(stPg, bWait)`가 조립·송신하고 응답은 `xNetCallbackRecieveData`가 `eS<S>F<F>` 접두로 분기해 이벤트를 세운다. 로그 `D:\HBS\Logs\Control\PG\PG_yyyymmdd.log`(SEND/RECV/OPER — **STX/ETX 제어문자가 그대로 들어가므로 grep 은 `: .1003` 처럼 한 글자 띄울 것**). `_ExePGCtrl(iEvent)` 매핑: `PGEvent_0` Model Change(S10F3 → 1초 → S2F1) · `_3` Status(S5F1) · `_5` Turn On(**S8F1 cmd=1 cell=00**, S8F3 완료 대기) · `_6`/`_7` Pattern Next/Prev(S8F1 cmd 3/4) · `_8` Turn Off · `_9~16` 셀 단위(S3F11/13/15/17) · `PG_TEST_1~4` OST/PTP/RTest/Aging(S8F1 cmd 5/6/7, S6F1).
  - **`PGEvent_8`(Turn Off)은 OCP 에서 S10F3 이 아니라 `S8F1 cmd=2 cell=00` 으로 보낸다 (2026-09-14).** 이 설비의 OCP PG 는 S10F3(`"1003"`, 전체 Turn Off) 에 **한 번도 응답하지 않았다**(PG 로그 09-08~09-14: 1003 송신 후 RECV 없음, 09-12 의 RECV 2건은 전부 S2F10 NAK). Turn On 이 S8F1 cmd=1 로 정상이고 사양서 2.8.1 S8F1 Command 표가 **1 Turn On / 2 Turn Off / 3 Next / 4 Prev / 5 OST / 6 PTP / 7 RTest, Cell 00 = 전체**라 같은 형식으로 바꿨다. 완료(S8F3) 대기는 Turn On 과 동일하며, PG 가 cmd=2 에 S8F3 을 안 주면 PG 로그에 `WaitInspectionComplete, HEAD_ACK_TIMEOUT` 이 남으니 그때는 대기를 빼면 된다. `PGEvent_0`(Model Change) 앞의 선행 Turn Off 도 OCP 면 같은 S8F1 cmd=2 로 보내되 기존처럼 응답 대기 없이(`bWait=false`) 1초 쉰다(뒤 Model 명령에 `iCmd/iCellNum` 잔존값이 섞이지 않게 `ZeroMemory`). HMI 모드는 둘 다 S10F3 유지. 호출부: 수동 화면 PG CMD 04 버튼(`enumStepID_PG_CMD_04`)·CMD 05 시퀀스 말미, `DoWorkGlassUnloadStep`, `DoWorkGlassMeasureStep` AGING_PTN 말미·4416행.
  - 참고: 오늘(09-14) PG 로그에 RECV 가 0건인데도 PTP/OST 가 100ms 만에 `END` 로 끝난 회차가 있다 — `WaitInspectionComplete` 는 `IsConnected()==false` 면 즉시 false 를 돌려주고 호출부가 로그만 남기므로, **PG 미연결 상태의 사이클은 결과 없이 통과**한다. PG 동작 검증은 반드시 RECV 줄(`0802`/`0803`)로 할 것.


### PGMON(ELP 전장박스 모니터) 알람 보고 — S10F20 (2026-09-21)

- **구조.** `CCommTcpIpObjectPGMon::ParseS10F20` 이 한 프레임(최대 10개 × 5byte 코드)을 훑으며 **코드마다 단명 스레드**(`ThreadRaiseAlarm`)를 띄워 `SetAlarm(ELP_ELECBOX_ALARM_START + code)` 를 부른다. 스레드를 나누는 이유는 Heavy 알람의 `SetHeavyAlarm` 이 작업자가 리셋할 때까지 호출 스레드를 `WaitForSingleObject(INFINITE)` 로 막기 때문 — 수신 스레드에서 직접 부르면 통신이 멎는다.
- **알람 계층은 동시 다발 Heavy 알람을 이미 제대로 처리한다.** `CAlarmManager::SetAlarmSub` 는 `INFINITE` 대기 **전에** `m_cs_set` 을 풀고, `ALARM_EVENT_DATA` 는 생성자에서 자기 이벤트를 만들며(복사 시 `Copy()` 가 `hEvent` 를 건드리지 않는다), `CDlgAlarm` 은 리스트 컨트롤이라 행이 여러 개여도 된다. 실증: 2026-09-10 14:41:44 한 프레임에 코드 10개(21~30) → `OCP_20260910.log` 에 `ALARM OCCURED` 10줄. **"여러 개가 하나로 합쳐진다" 는 여기가 원인이 아니다.**
- **진짜 원인은 수신단의 코드 필터였다 (2026-09-21 수정).** `IsValidAlarmCode` 가 문서 표(0~39 / 100~139 / 200~207 / 300~303 / 400~403)만 통과시켜, ELP 가 실제로 보내는 **문서 미등재 코드 40·41·140·141 을 조용히 버렸다**. 그 결과 코드 2개짜리 보고에서 알람이 1개만 떴다 — PGMON 로그에 `S(10), F(20), LEN(00010), Error Code(00101, )` 처럼 **LEN 은 2개분(10)인데 Error Code 는 1개**로 남고, 바로 위에 `undefined error code(141) skipped` WARN 이 있는 것이 서명이다. 코드 141 단독 프레임은 `Error Code()` 로 비어 있다. 필터를 **블록당 50**(0~49 / 100~149 / 200~249 / 300~349 / 400~449)으로 넓혔다.
- **블록당 50 이 상한인 이유.** 알람 ID = `ELP_ELECBOX_ALARM_START`(8550) + 코드인데 **빈 대역이 8550~8999 뿐**이다(9001 부터 `SDC_PPA_MODULE_FILE_EXIST_ERROR`). `AlarmIDList.h` 에 `static_assert(ELP_ELECBOX_ALARM_END < SDC_PPA_MODULE_FILE_EXIST_ERROR, ...)` 를 걸어 더 넓히면 컴파일이 깨지게 해 뒀다. 더 필요하면 먼저 빈 대역을 찾을 것.
- **필터를 통과해도 `Alarm.xls` 에 행이 없으면 본문 `UNKNOWN` 인 Heavy 알람이 뜬다** (루트 CLAUDE.md 알람 절 참조 — `FindAlarmTable` 실패 시 `ALARM_DATA` 생성자 기본값이 그대로 쓰인다). 그래서 실제 관측된 40/41/140/141 은 `Source/Bin/Config/CoreFunction/AlarmConfig/Alarm.xls` 에 `8590/8591/8690/8691` 행으로 넣었고 문구는 **"전장 박스N 미정의 에러코드 nnn"** 이다(ELP 에서 정의를 받으면 텍스트 열만 고치면 된다). **`Alarm.xls` 는 `Source/Bin/Config/...` 사본만 살아 있다** — `CoreFunction/dll_Alarm/Files/` 와 `Source/ALPHA/ConfigurationFile/...` 사본은 2026-05-18 에서 멈춰 있어 동기화 대상이 아니다. 로더는 **USE 열(13번)이 1 인 행만** 읽고 1번 열이 `[FILE END]` 인 행에서 멈추므로, 행 추가는 그 마커 위에 끼워 넣을 것.
- 코드 정의는 `Doc/07_설계문서/통신문서/ELP_2026/ElecBoxProtocol_SDC HMI(분판검사기) OCP화 개조_20260728.doc` 3장. 같은 폴더의 `ElecBoxProtocol_GAI_20240820.doc` 는 **전장박스 3대 기준의 다른 코드 체계**(100단위 박스, 300 소켓, 400/500/600 상세)라 이 설비에 적용되지 않는다 — 헷갈리지 말 것. 문서 스스로 "이 값은 개발 상황에 따라 추가 혹은 변형된다" 고 적고 있다.
