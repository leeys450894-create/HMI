# ELP ElecBox 업로드 — 외부 확인 요청서 (B 트랙)

| 항목 | 내용 |
|---|---|
| 작성일 | 2026-07-30 |
| 목적 | `ELP_FDC_MCC_Upload_Plan.md`의 U2·U3 및 잔여 검증 항목 중 **외부 부서 확인 없이는 구현을 확정할 수 없는 질의**를 담당별로 정리 |
| 현 상태 | U1(FDC 공유메모리 발행) 완료 · 아래 4개 확인 항목 해소 후 U2/U3 착수 |
| 관련 문서 | `ELP_ElecBox_Porting_Plan.md`, `ELP_FDC_MCC_Upload_Plan.md`, `FDC_Structure_Comparison.md`, `../../docs/Protocol_Code_Analysis.md` |

> 사용 방법: 아래 4개 절(A~D)은 각각 해당 담당자에게 그대로 전달할 수 있는 형태로 작성됨. 회신 받은 값을 이 문서에 기입하면 곧바로 구현 단계로 진행 가능.

---

## A. ELP(전장박스) 담당 확인 요청 — ACK 프레임 Len 필드 ✅ 【회신 완료 2026-07-30】

> **회신 결과 요약**: ACK는 단순 값 전달용이며 **ELP는 Len/Cause를 검증하지 않고, NAK에 대한 재전송도 없음**. Critical 경로는 **ELP측 Alarm 전송(S10F20)**.
> → 현행 구현(`Len="0000"`, `Cause="0000"`)이 **회신과 정확히 일치**하여 ACK 관련 코드 변경 없음.
> → 단, "재전송 없음 + 알람이 Critical"에서 도출되는 **파서 견고성 결함을 발견해 수정**함 (아래 §A-2).

### 질의
S20F01 / S20F20(HMI→ELP ACK) 프레임의 **Len 필드 값**이 신규 문서와 기존 구현에서 다릅니다. 어느 쪽이 맞습니까?

| 근거 | Len 값 | 비고 |
|---|---|---|
| 신규 문서 `ElecBoxProtocol_SDC HMI(분판검사기) OCP화 개조_20260728.doc` 2.2.2 / 2.2.4 | **`0000`** | 문서 표에 0으로 명기 |
| 기존 GAI(A6 8.6G) 구현 | `0006` | Ack(2) + Cause(4) = 6바이트를 Len으로 계산 |

- 현재 ALPHA 구현은 **문서를 따라 `0000`** 송신 (`CommTcpIpObjectPGMon.cpp` `SendAck()`)
- 실제 프레임: `STX + "20" + Func(2) + Len(4) + Ack(2) + Cause(4) + ETX`
  - 예(ACK): `<STX>2001000000 0000<ETX>` → 실제 송신 `\x02` `20` `01` `0000` `00` `0000` `\x03`

### 확인 필요 사항
1. ELP 수신 측이 Len 필드를 **검증**합니까? (검증한다면 0000은 NAK 처리되는지)
2. `0000`과 `0006` 중 실기에서 정상 처리되는 값은?
3. Cause 필드(4byte)의 값 정의표가 있습니까? (문서에는 `0000: Normal`만 기재 — NAK 시 원인 코드 체계 필요)
4. HMI가 NAK를 보낼 조건(Len 불일치·ElecBox Index 범위 초과·정의되지 않은 Error Code)에 대해 ELP측 재전송 정책이 있습니까?

### 회신 결과 (2026-07-30)
```
1) Len 검증 여부:        검증하지 않음
2) 정상 처리되는 Len 값:  0000
3) Cause 코드표:          0000 (고정)
4) 재전송 정책:           없음
비고: 단순하게 값을 보내는 용도. Critical 부분은 ELP 측 Alarm 전송.
```

| 항목 | 회신 | 현행 구현 | 조치 |
|---|---|---|---|
| Len | `0000` | `SendAck()` `%04d` ← `0` → `"0000"` | ✅ 일치, 변경 없음 |
| Cause | `0000` 고정 | 모든 호출부가 `iCause=0` 전달 | ✅ 일치, 변경 없음 |
| Len 검증 | 안 함 | — | ACK는 정보성. GAI 구판 `0006`으로 되돌리지 말 것(코드 주석 명기) |
| 재전송 | 없음 | — | ⚠ §A-2 조치 |

## A-2. 회신에서 도출된 파서 견고성 수정 【완료 2026-07-30】

### 문제
"**재전송 없음**" + "**Critical은 Alarm 전송**"이라는 회신은, 기존 파서 설계에 다음 위험이 있음을 의미합니다:

기존 `ParseS10F20()`은 Len 필드가 조금이라도 어긋나면 **프레임 전체를 버리고 NAK**했습니다.
ELP가 재전송하지 않으므로 이는 **중요 알람의 영구 소실**입니다. (NAK를 보내도 ELP는 검증조차 하지 않으므로 아무 효과가 없음)

### 수정 내용 (`CommTcpIpObjectPGMon.cpp`)

| 파서 | 기존 | 변경 후 |
|---|---|---|
| `ParseS10F01` | `iLen != 80` 이면 전체 폐기 | **실제 body가 80자 이상이면 파싱**, Len 불일치는 경고 로그만. body가 진짜 짧을 때만 NAK |
| `ParseS10F20` | Len 이상 시 프레임 전체 폐기 | **실제 body 길이에서 파싱 가능한 5자 코드 수만큼 처리**(`nAvail`), 최대 10개 상한 유지. 코드가 하나도 안 나올 때만 NAK |

안전장치는 유지: 수신 body 범위를 넘어 읽지 않음(`nAvail = iBodyLen / 5` 상한), 개별 코드는 `IsValidAlarmCode()`로 계속 검증.

### 검증 (판정 로직 시뮬레이션)

| Len | body | 기존 | 변경 후 | 비고 |
|---|---|---|---|---|
| 5 | 5 | parse 1 | parse 1 | 정상 |
| 50 | 50 | parse 10 | parse 10 | 최대치 정상 |
| 0 | 5 | **폐기** | **parse 1** | 알람 1건 구제 |
| 7 | 10 | **폐기** | **parse 2** | 알람 2건 구제 |
| 50 | 25 | **폐기** | **parse 5** | 알람 5건 구제 |
| 55 | 55 | 폐기 | parse 10 (상한) | 11개 주장 → 10개로 제한 |
| 5 | 4 | NAK | NAK | 파싱 불가, 정상 거부 |
| 0 | 0 | NAK | NAK | 빈 프레임, 정상 거부 |

빌드: `ALPHA.sln Release|x64` 통과. 백업 `.bak_besteffort`.

### 부수 정리 — 줄바꿈 정규화
`CommTcpIpObjectPGMon.cpp/.h`가 **LF**였음을 발견(기존 ALPHA 소스 전체는 **CRLF**). 원 이식 시 유입된 불일치로, 두 파일을 CRLF로 정규화(내용 무변경, ASCII 전용이라 손상 없음).

---

## B. MCC 스펙 담당 확인 요청 — I-Log 컬럼 매핑

### 배경
MCC 주기 로그(I-Log)는 **고정 17열**만 CSV로 기록합니다(`dMCCSpec/DlgnMCCReport.h:48` `MCC_ILOG_COL_ITEM2_E = MCC_ILOG_COL_ITEM2 + 17`, 기록 루프 `DlgnMCCReport.cpp:1690`).
컬럼명은 `Config\MCC\MCCNAME.csv`에 **이미 ElecBox성 이름으로 정의**되어 있습니다(모터 5열 + 데이터 15열 = 20행).

현재 `MCCNAME.csv` 정의 (6번째 행 이후가 dData[0]부터 대응):

| dData 슬롯 | MCCNAME.csv 컬럼명 |
|---|---|
| [0] | `TEMP_PC_RACK(℃)` |
| [1] | `TEMP_EB_RACK_1(℃)` |
| [2] | `TEMP_EB_RACK_2(℃)` |
| [3] | `N01_T_PW(kWh)` |
| [4] | `N01_I_PW(kW)` |
| [5] | `N01_R_VOL(V)` |
| [6] | `N01_T_VOL(V)` |
| [7] | `N01_R_CUR(A)` |
| [8] | `N01_T_CUR(A)` |
| [9] | `N02_T_PW(kWh)` |
| [10] | `N02_I_PW(kW)` |
| [11] | `N02_R_VOL(V)` |
| [12] | `N02_T_VOL(V)` |
| [13] | `N02_R_CUR(A)` |
| [14] | `N02_T_CUR` |

ELP ElecBox가 제공하는 값 (박스당 9종 × 2박스):

| # | ELP 필드 | 단위 |
|---|---|---|
| 1 | 온도 | ℃ |
| 2 | 연기 감지 농도 | % |
| 3 | 전압 | V |
| 4 | **고조파 전압** | V |
| 5 | 전류 | A |
| 6 | **고조파 전류** | mA |
| 7 | **누설 전류** | mA |
| 8 | 순시 전력 | kW |
| 9 | 적산 전력 | kWh |

### 질의 1 (핵심) — R상/T상 컬럼과 ELP 필드의 의미 불일치
`N0x_R_VOL` / `N0x_T_VOL` / `N0x_R_CUR` / `N0x_T_CUR`는 컬럼명상 **R상·T상(3상 전원의 상별) 전압/전류**입니다.
그런데 ELP가 주는 것은 상별 값이 아니라 **전압/고조파전압, 전류/고조파전류**입니다. 1:1 대응이 되지 않습니다.

어느 방식으로 처리할지 결정 필요:

| 안 | 내용 | 장점 | 단점 |
|---|---|---|---|
| **안 1** | 컬럼명을 ELP 실제 의미로 변경 (`N01_VOL`, `N01_HARM_VOL`, `N01_CUR`, `N01_HARM_CUR`) | 데이터 의미 정확 | 기존 MCC 수집 스키마 변경 → 팹 수집측 영향 확인 필요 |
| **안 2** | 컬럼명 유지, R→기본값/T→고조파로 매핑 | 스키마 무변경 | **컬럼명과 실제 값의 의미가 다름** (오해석 위험, 비권장) |
| **안 3** | 해당 4열은 미사용(0 기록), 전압/전류계는 FDC 경로만 사용 | 오해석 없음 | MCC에서 전력 상세 미확인 |

> 담당 회신 없이는 **구현 보류** 중입니다(안 2는 잘못된 데이터가 팹에 올라갈 수 있어 임의 적용 불가).

### 질의 2 — 기존 온도 압축 루프와의 슬롯 충돌
현재 `MCCLogMgr.cpp:249-255`는 dData를 **가변 개수로 순차 점유**합니다:

```cpp
int nItem = 0;
for (size_t i = 0; i < FDC_WDATA_TEMP_END; i++)   // 온도 50개 순회
{
    if (dTemperature[i] > 0)                       // 0 초과인 것만 압축 기록
    {
        m_pSharedMCC->stMCCDataEQ_ILog.dData[nItem] = dTemperature[i];  nItem++;
    }
}
m_pSharedMCC->stMCCDataEQ_ILog.nCntILog = nItem;
```

즉 **살아있는 온도 센서 개수에 따라 dData 슬롯 위치가 매번 달라집니다.**
반면 `MCCNAME.csv`는 슬롯을 고정으로 정의하고 있어 **현 상태로도 이미 컬럼명과 값이 어긋날 수 있는 구조**입니다(ELP와 무관한 기존 사안).

ELP 반영 시 **dData를 고정 슬롯제로 재정렬**해야 하며, 이는 기존 MCC 수집 데이터의 해석에 영향을 줍니다.

확인 필요:
1. 현재 팹 MCC 수집에서 이 컬럼들을 **실제로 사용**하고 있습니까? (사용 중이라면 값이 컬럼명과 일치하는지)
2. 고정 슬롯제로 재정렬해도 됩니까? 재정렬 시 사전 통보/스키마 등록 절차가 필요합니까?
3. `TEMP_PC_RACK`은 PC 랙 온도(ELP 아님)로 유지 예정입니다. 맞습니까?

### 질의 3 — 17열 제한
17열 한도로 인해 ELP 18개 값(2박스×9) 전체는 MCC에 실을 수 없습니다.
**연기 감지 농도·누설 전류·고조파 일부는 FDC 경로(공유메모리→PLC D영역) 전담**으로 역할 분리 예정입니다.

1. 17열 확장이 가능합니까? (불가하면 위 분리안으로 진행)
2. MCC에 반드시 포함되어야 하는 ELP 항목의 **우선순위**를 지정해 주십시오.

### 회신 기입란
```
질의1) 채택 안 (1/2/3):                 컬럼명 변경 시 신규 컬럼명:
질의2-1) 컬럼 실사용 여부:
질의2-2) 고정 슬롯 재정렬 승인:         필요 절차:
질의2-3) TEMP_PC_RACK 유지 확인:
질의3-1) 17열 확장 가능 여부:
질의3-2) MCC 필수 ELP 항목 우선순위:
확인자 / 일자:
```

---

## C. PLC 담당 확인 요청 — FDC 업로드용 D주소 배정

### 배경 (데이터 경로)
```
ELP ElecBox ──TCP/IP(S10F01)──> ALPHA HMI (PGMon)
                                    │ 1초 주기
                                    ▼
                        VTM_FOR_FDC 공유메모리 stMainWData[400~428]   ← U1 완료
                                    │
                        FDCSense(FDC_LIST.xls SOURCE=SHARE로 읽음)
                                    │
                                    ▼
                             PLC D영역 (DESTINATION)  ← ★ 주소 배정 필요
                                    │
                                    ▼
                          팹 FDC / CIM 폴링 수집
```

### 현황 — HMI 측 발행 슬롯 (확정, 변경 없음)
`FDCSheardDefine.h` `NUM_MAIN_WDATA_ELP` 기준, 슬롯 = `400 + box*20 + item`

| 공유메모리 슬롯 | 이름 | 항목 | 단위 |
|---|---|---|---|
| 400 | `EB1_TEMP` | ElecBox1 온도 | ℃ |
| 401 | `EB1_SMOKE` | 연기 농도 | % |
| 402 | `EB1_VOLT` | 전압 | V |
| 403 | `EB1_HARM_VOLT` | 고조파 전압 | V |
| 404 | `EB1_CURR` | 전류 | A |
| 405 | `EB1_HARM_CURR` | 고조파 전류 | mA |
| 406 | `EB1_LEAK_CURR` | 누설 전류 | mA |
| 407 | `EB1_INST_PW` | 순시 전력 | kW |
| 408 | `EB1_ACCUM_PW` | 적산 전력 | kWh |
| 420~428 | `EB2_*` | ElecBox2 동일 9항목 | 동일 |

### 질의 1 — DESTINATION D주소 배정
ELP 18개 값을 쓸 **PLC D영역 주소**를 배정해 주십시오.

- 기존 FDC 업로드 D주소 사용 현황: `D10000` ~ `D10132` 대역 사용 중 (FDC_LIST.xls DESTINATION 열, 2byte씩 증가)
  - 예: `D10000`(CDA_Pressure01), `D10044`(Sample Vacuum 01), `D10046`~`D10078`(Fixture Time), `D10124`~`D10132`(Temp)
- 참고 자료: `docs/PLC_D_Memory_FDC_Comparison.md` (2026-07-24 M/D 메모리맵 정합화 작업 산출물)

필요 주소 개수: **18개** (값당 2byte 가정 시 36byte 연속 대역 권장)

요청 형식 예:
```
EB1: D10200, D10202, D10204, ... D10216  (9개)
EB2: D10220, D10222, ... D10236          (9개)
```

### 질의 2 — 데이터 타입 / 배율
FDC_LIST에는 `Value Magnification`(배율)과 `Type`(0=INT 등) 열이 있습니다.
소수점을 갖는 항목(온도 0.1℃ 단위, 전압/전류 0.001 단위)의 D영역 표현 방식 확인 필요:

| 항목 | HMI 발행 값 예 | 제안 배율 | D영역 정수값 |
|---|---|---|---|
| 온도 | 23.4 | ×10 | 234 |
| 전압 | 220.123 | ×1000 | 220123 (2word 필요) |
| 적산 전력 | 12345 | ×1 | 12345 |

1. 각 항목의 배율을 위 제안대로 해도 됩니까?
2. 32bit(2word)가 필요한 항목(전압·적산전력 등)의 D영역 처리 방식은? (현재 FDC_LIST는 2byte 간격 배치)

### 질의 3 — 기존 D주소 대역 충돌 확인
`docs/PLC_D_Memory_FDC_Comparison.md` 대조 결과 미확인 대역이 있어, 신규 배정 주소가 다른 용도와 겹치지 않는지 최종 확인을 요청합니다.

### 회신 기입란
```
질의1) 배정 D주소  EB1:
                   EB2:
질의2-1) 배율 승인 / 수정:
질의2-2) 32bit 항목 처리 방식:
질의3) 대역 충돌 없음 확인:
확인자 / 일자:
```

---

## D. 사내 확인 — 시운전 조건 (실장비 IP 배정)

`Config\Network.ini` `[PARA_10]`(PG_MON) 현재 설정 상태 (2026-07-30 확인):

| 설정 항목 | 현재 값 | 시운전 시 필요 값 | 상태 |
|---|---|---|---|
| `NETWORK_PARA_10_TCPIP_USE` | 1 | 1 | ✅ |
| `NETWORK_PARA_10_TCPIP_OUT_OF_SERVICE` | **0** (접속 활성) | 0 | ✅ 이미 해제됨 |
| `NETWORK_PARA_10_TCPIP_NAME` | PG_MON | PG_MON | ✅ |
| `NETWORK_PARA_10_TCPIP_IP` | **127.0.0.1** (루프백) | **ELP 장비 실 IP** | ★ 변경 필요 |
| `NETWORK_PARA_10_TCPIP_PORT` | 5001 | ELP 장비 포트 | ★ 확인 필요 |
| `NETWORK_PARA_10_TCPIP_MODE` | 0 (Client) | 0 | ★ 구조 확인 |
| `NETWORK_PARA_10_TCPIP_MEAS_TYPE` | 19 (PG_MON) | 19 | ✅ |
| `TCPIP_COUNT` | 11 (PARA_0~10 포함) | 11 | ✅ |

> 현 설정은 **로컬 시뮬레이터(`docs/tools/elp_elecbox_sim.py`) 연결 상태**입니다. 실장비 연결 시 IP만 교체하면 됩니다.

### 확인 필요
1. ELP ElecBox 실장비의 **IP / Port** 확정값 (현재 루프백 → 실 IP 교체)
2. HMI가 Client, ELP가 Server 구조가 맞습니까? (현 구현 `MODE=0` = HMI가 Client로 접속 시도)
3. 시운전 일정 / 입회 담당
4. 알람 검증 방법: ELP측에서 **테스트 Error Code 강제 발생**이 가능합니까? (알람 8550+code 대역 96종 등록 완료, 실제 발생 검증 필요)

### 회신 기입란
```
1) IP / Port:
2) Client/Server 구조 확인:
3) 시운전 일정 / 입회자:
4) 테스트 알람 발생 가능 여부:
```

---

## 부록 1. FDC_LIST.xls SHARE 행 추가 【적용 완료 2026-07-30 — DESTINATION만 임시】

> **적용 상태**: 18행 추가 완료. `DESTINATION`은 **임시로 `D20000` 일괄 적용** — C절 회신 후 실주소로 교체 필요.
>
> **⚠ 임시 D20000의 동작 의미 (중요)**
> `DlgPLC.cpp:123-138`이 쓰기 가능 주소표를 `m_iRefAddressToWrite[type][i] = 10000 + USESIZE*i` (Config_FDC.ini `USESIZE=2`, `USEGAP=0`)로 생성합니다.
> 즉 **유효 범위는 D10000~D10598(짝수)** 이며, `D20000`은 이 표에 없어 `iSortArray`가 `-1`로 남아 **PLC 쓰기가 조용히 생략**됩니다.
>
> | 경로 | D20000 상태에서 |
> |---|---|
> | ALPHA → 공유메모리 `stMainWData[400~428]` | ✅ 정상 동작 |
> | FDCSense → 공유메모리 읽기 → `m_valArray` / 화면 표시 | ✅ 정상 동작 |
> | FDCSense → `stFDCWData[250~278]` 기록 | ✅ 정상 동작 |
> | FDCSense → **PLC D영역 쓰기** | ❌ **생략됨** (주소표 밖) |
> | 팹 FDC/CIM 수집 | ❌ 미동작 (PLC 쓰기 없으므로) |
>
> 임시값으로는 **오히려 안전**합니다 — 실 D영역을 잘못 덮어쓸 위험이 없고, 공유메모리 구간까지의 배선은 그대로 검증 가능합니다. 크래시·범위초과 없이 스킵만 발생합니다.

### 실제 적용된 행 (Excel 행 190~207)

| SVID | Parameter Name | SOURCE | SOURCE ADDRESS<br>(stMainWData) | DESTINATION | NUM_FDC_WDATA<br>(stFDCWData) | Mag | Type |
|---|---|---|---|---|---|---|---|
| 145 | ELP_EB1_TEMP | SHARE | 400 | D20000 ★임시 | 250 | 0 | 2 |
| 146 | ELP_EB1_SMOKE | SHARE | 401 | D20000 ★임시 | 251 | 0 | 2 |
| 147 | ELP_EB1_VOLT | SHARE | 402 | D20000 ★임시 | 252 | 0 | 2 |
| 148 | ELP_EB1_HARM_VOLT | SHARE | 403 | D20000 ★임시 | 253 | 0 | 2 |
| 149 | ELP_EB1_CURR | SHARE | 404 | D20000 ★임시 | 254 | 0 | 2 |
| 150 | ELP_EB1_HARM_CURR | SHARE | 405 | D20000 ★임시 | 255 | 0 | 2 |
| 151 | ELP_EB1_LEAK_CURR | SHARE | 406 | D20000 ★임시 | 256 | 0 | 2 |
| 152 | ELP_EB1_INST_PW | SHARE | 407 | D20000 ★임시 | 257 | 0 | 2 |
| 153 | ELP_EB1_ACCUM_PW | SHARE | 408 | D20000 ★임시 | 258 | 0 | 2 |
| 154~162 | ELP_EB2_* (동일 9항목) | SHARE | 420~428 | D20000 ★임시 | 270~278 | 0 | 2 |

부수 변경: 정본 `HMI\Source\Include\FDCSheardDefine.h`에 `FDC_WDATA_ELP_START = 250` / `FDC_WDATA_ELP_BOX_STRIDE = 20` / `FDC_WDATA_ELP_END` 대역 추가(EX_FDC는 리다이렉트로 자동 반영). 백업 `.bak_elpband`, `FDC_LIST.xls.bak_elprows`.

### 검증 결과 (FDCSense 파싱 로직 재현)

| 항목 | 결과 |
|---|---|
| 로드되는 행 수 (`m_nFDCListCnt`) | 110 → **128** (`dfMAXFDCITEM=300` 이내) |
| SHARE 타입 행 인식 | **18행** (`i`=110~127) |
| SOURCE ADDRESS ↔ ALPHA 발행 슬롯 | **완전 일치** (`400+box*20+item`) |
| `NUM_FDC_WDATA` 슬롯 기존 행과 충돌 | **없음** |
| 기존 186행 셀 변경 | **없음** (백업 전셀 대조, Excel 재저장 비파괴) |
| 파일 형식 | BIFF8 `.xls` 유지 (Code page 949) |
| 빌드 | `FDC_R\|x64` Rebuild + `Release\|x64`(ALPHA 7프로젝트) 통과 |

### C절 회신 후 남은 작업
`DESTINATION` 18칸을 실주소로 교체(EB1: 9개, EB2: 9개). **각 항목이 서로 다른 주소여야 함** — 현재는 18개 전부 D20000이라 실주소 적용 시 반드시 개별 주소로 분리할 것.

---

## 부록 1-B. 최초 초안 (참고용, 위 적용본으로 대체됨)

`EX_FDC\Bin\FDC_LIST\FDC_LIST.xls` (현재 186행, SHARE 타입 행 **0개** — ELP가 첫 사용 사례)

컬럼 의미 (`FDCSenseDlg.cpp:2509-2610` 파싱 기준):

| 컬럼 | 의미 | SHARE 타입일 때 |
|---|---|---|
| `SVID` | 행 번호 | 187부터 연번 |
| `Parameter Name` | 표시명 | `ELP_EB1_TEMP` 등 |
| `USE`(C열) | 필터 사용 | 1 |
| `MIN`/`MAX`/`DEFAULT` | 필터 범위 | 항목별 설정 |
| `USE`(G열) | 행 활성 | **1** (0이면 행 자체를 건너뜀) |
| `SOURCE` | 소스 종류 | **`SHARE`** |
| `SOURCE ADDRESS` | 소스 주소 | **stMainWData 슬롯 = 400~428** |
| `DESTINATION` | PLC 쓰기 주소 | ★ C절 회신값 |
| `NUM_FDC_WDATA` | stFDCWData 쓰기 슬롯 | ★ 아래 주의 참조 |
| `Value Magnification` | 배율 | ★ C절 회신값 |
| `Type` | 자료형 | 0=INT 등 |

**주의 — `NUM_FDC_WDATA` 슬롯 선정**
현재 사용 중인 stFDCWData 슬롯: `1~8, 14~17, 31~46`
`FDCSheardDefine.h` `NUM_FDC_WDATA` enum 의미 대역:

| 대역 | 용도 |
|---|---|
| 1~50 | TEMP |
| 51~60 | VACUUM |
| 61~70 | CDA |
| 71 | ESD |
| 80~97 | FIXTURE |
| 100~133 | EFU |
| 160~172 | SEM(UPS/GPS) |
| 180~190 | CHAMBER |
| 200~221 | LAMP |
| **222~999** | **미사용 (여유)** |

→ ELP용으로 **`FDC_WDATA_ELP_START = 250`** 신규 대역 제안 (enum 추가 필요, 정본 `HMI\Source\Include\FDCSheardDefine.h`에만 추가하면 EX_FDC는 리다이렉트로 자동 반영)

제안 행 (DESTINATION·배율은 C절 회신 후 채움):

| SVID | Parameter Name | USE | SOURCE | SOURCE ADDRESS | DESTINATION | NUM_FDC_WDATA |
|---|---|---|---|---|---|---|
| 187 | ELP_EB1_TEMP | 1 | SHARE | 400 | ★ | 250 |
| 188 | ELP_EB1_SMOKE | 1 | SHARE | 401 | ★ | 251 |
| 189 | ELP_EB1_VOLT | 1 | SHARE | 402 | ★ | 252 |
| 190 | ELP_EB1_HARM_VOLT | 1 | SHARE | 403 | ★ | 253 |
| 191 | ELP_EB1_CURR | 1 | SHARE | 404 | ★ | 254 |
| 192 | ELP_EB1_HARM_CURR | 1 | SHARE | 405 | ★ | 255 |
| 193 | ELP_EB1_LEAK_CURR | 1 | SHARE | 406 | ★ | 256 |
| 194 | ELP_EB1_INST_PW | 1 | SHARE | 407 | ★ | 257 |
| 195 | ELP_EB1_ACCUM_PW | 1 | SHARE | 408 | ★ | 258 |
| 196~204 | ELP_EB2_* (동일 9항목) | 1 | SHARE | 420~428 | ★ | 270~278 |

> 편집 주의: `.xls`는 FPSpread(구 Excel 97-2003 형식)로 FDCSense가 직접 읽습니다. **행 삽입은 파일 끝에만**, 기존 행 순서·삭제 금지.

---

## 부록 2. 확인 과정에서 발견한 기존 결함 (참고 — 별도 판단 필요)

B 트랙 준비 중 FDC/MCC 경로 조사에서 발견한, **ELP와 무관한 기존 코드/설정 결함**입니다.

### 2-1. 【수정 완료 2026-07-30】 SHARE 소스 읽기 로직 결함
`EX_FDC\FDCSense\DlgPLC.cpp` `WriteFDC_Filtering()`

- **문제**: `SOURCE_TYPE_SHAHREMEMORY` 분기가 `stMainWData[k]`를 **전역 순차 카운터 k**로 읽어, xls의 `SOURCE ADDRESS` 열을 완전히 무시했음(다른 모든 분기는 SOURCE ADDRESS를 사용). 또한 `atoi()`로 읽어 소수점 이하가 절삭됨(Main SW는 `"%.4f"` 문자열로 기록).
- **영향**: SHARE 타입 행이 현재 0개라 실동 영향은 없었으나, **ELP 행 추가 시 즉시 오작동**할 위치였음.
- **수정**: `SOURCE ADDRESS`를 슬롯 인덱스로 사용 + `atof()` + 범위 가드 추가. 빌드 검증 완료(백업 `.bak_sharefix`).

### 2-2. 【수정 완료 2026-07-30】 stFDCWData 쓰기 경계 하드코딩
동일 파일 `if(nIndex < 100)` → `if(nIndex < FDC_WDATA_END)` (실제 배열 크기 1000). 슬롯 100 이상 사용 시 값이 조용히 버려지는 문제. ELP용 250 대역 사용을 위해 필수 선행 수정이었음.

### 2-3. 【미수정 — 확인 필요】 FDC_LIST.xls stFDCWData 슬롯 중복 배정
`NUM_FDC_WDATA` 열에 **동일 슬롯이 여러 행에 배정**되어 있습니다. 공유메모리 슬롯은 1개이므로 **마지막에 쓴 행의 값만 남습니다**.

| 중복 슬롯 | 배정된 행 |
|---|---|
| 1 | `Temp_EBOX_F` (SENSOR SRC=1), `Temp_EBOX_F` (SENSOR SRC=16) |
| 2 | `Temp_EBOX_B` (SENSOR SRC=2), `Temp_EBOX_B` (SENSOR SRC=17) |
| 3 | `Temp_PCBOX` (SENSOR SRC=3), `Temp_PCBOX` (SENSOR SRC=18) |
| 4 | `Temp_PCBOX` (SENSOR SRC=4), `Temp_PCBOX` (SENSOR SRC=19) |
| **5** | **`Sample Vacuum 01`(PLC D11560), `Temp_PCBOX`(SENSOR 5), `Temp_PCBOX`(SENSOR 20)** ← 서로 다른 물리량이 같은 슬롯 |

- 슬롯 1~4는 같은 센서의 이중 등록(1차/2차 계통 추정)이라 값이 유사할 수 있으나, **슬롯 5는 진공압(kPa)과 온도(℃)가 같은 슬롯을 다툼** — 어느 값이 팹에 올라가는지 실행 순서에 의존합니다.
- **판단 필요**: 의도된 설정인지, 설비 담당/FDC 담당 확인 후 정리 필요. (본 ELP 작업 범위 밖이므로 임의 수정하지 않음)

### 2-4. 【미수정 — B절 질의 2로 이관】 MCC dData 가변 슬롯 점유
`MCCLogMgr.cpp:249-255`의 온도 압축 루프가 dData를 가변 개수로 점유하여, 고정 컬럼명(`MCCNAME.csv`)과 어긋날 수 있는 구조. MCC 담당 확인 항목으로 B절에 포함.

---

## 진행 상태 추적

| 절 | 담당 | 발송 | 회신 | 후속 작업 | 상태 |
|---|---|---|---|---|---|
| A. ACK Len | ELP | ☑ | **☑ 2026-07-30** | 코드 변경 불요(현행 일치) + §A-2 파서 견고성 수정 | ✅ 완료 |
| B. MCC 컬럼 | MCC 스펙 | ☐ | ☐ | U2 구현 (MCCLogMgr dData 고정 슬롯화) | ⏸ 대기 |
| C. PLC D주소 | PLC | ☐ | ☐ | FDC_LIST 18행 + enum 250 대역 **적용 완료(2026-07-30)**, `DESTINATION`만 D20000 임시 → 실주소 교체 대기 | 🔶 부분 완료 |
| D. 시운전 | 사내 | ☐ | ☐ | Network.ini 실장비 IP, end-to-end 검증 | ⏸ 대기 |
| 부록2-3 슬롯 중복 | 설비/FDC | ☐ | ☐ | FDC_LIST 정리 | ⏸ 대기 |
