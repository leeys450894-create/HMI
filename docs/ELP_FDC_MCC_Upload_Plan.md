# ELP ElecBox 데이터 FDC/MCC 업로드 준비 플랜

| 항목 | 내용 |
|---|---|
| 배경 | ELP ElecBox 프로토콜 이식(2026-07-29, `docs/ELP_ElecBox_Porting_Plan.md`)에서 수신 데이터가 `CCommTcpIpObjectPGMon::GetElpData(box, item)`(2박스×9항목)까지만 연결되고 FDC/MCC 업로드는 **보류**됨 |
| 목적 | 보류 항목의 실행 플랜 확정 — FDC 공유메모리 경로(U1) + MCC I-Log 경로(U2) + 외부 FDC 프로그램 연동(U3) |
| 상태 | **U1 완료(2026-07-29, 빌드 통과)** / U2·U3는 **외부 확인 대기** — 질의서 발송 후 회신 시 착수 |
| 참조 | GAI 선행 구현 분석 `D:\OCP\0.SW_Matching\docs\Protocol_Code_Analysis.md` §4.2, 구조 비교 `docs/FDC_Structure_Comparison.md` |
| **확인 요청서** | **`docs/ELP_Upload_Confirmation_Requests.md` (2026-07-30 작성)** — U2/U3 착수에 필요한 MCC·PLC·ELP 질의 및 회신 기입란, FDC_LIST SHARE 행 추가 초안, 조사 중 발견한 기존 결함 4건 포함 |

---

## 0. 현황 요약 (조사 완료)

| 필요 요소 | ALPHA 현황 |
|---|---|
| VTM_FOR_FDC 매핑 + stMainWData 기록부 | ✅ `MCCLogMgr.cpp:409-435` 생성("KMAC_Shared_Memory_For_FDC_V2"), `UpdateSensor():307-403`이 슬롯 20~79 사용 — **80~999 빈 슬롯** |
| 업로드 구동 주기 | ✅ `DlgSubOperate.cpp:565/1832` — UI 타이머 1초마다 `UpdateSensor()` |
| `MAIN_WDATA_*` enum | ❌ 없음 (ALPHA `FDCSheardDefine.h`에는 FDC_WDATA_*만 존재, stMainWData 슬롯은 리터럴) |
| `SetDataToELP` + 보관 배열 | ❌ 없음 — 유사 setter 선례는 `SetLampTime`/`m_uLampTime[10]`(MCCLogMgr.cpp:618) |
| MCC I-Log dData 경로 | ✅ `stMCCDataEQ_ILog.dData[100]` 중 **17열만 CSV 기록**(dMCCSpec `DlgnMCCReport.cpp:1690`, `MCC_ILOG_COL_ITEM2_E`=+17). `Config\MCC\MCCNAME.csv`가 이미 ElecBox성 15컬럼명 정의 |
| 데이터 소스 | ✅ `CommTcpIpObjectPGMon.cpp` `GetElpData(2×9)` + `m_cs_Data` + `IsElpDataValid()` |
| 싱글턴 푸시 선례 | ✅ `CommTcpIpObjectCIM.cpp`가 `CMCCLogMgr::GetInstance()` 직접 호출 |

주의 사전지식:
- FDC 프로그램은 `stMainWData`를 `nMainDataCount`로 순회하지 않고 **FDC_LIST.xls의 SOURCE_ADDRESS로 직접 인덱싱** → 슬롯 400대 사용 시 count 계산(`nNext-20`) 변경 불필요 (GAI가 이 방식으로 동작함을 실증)
- 외부 FDCSense(보유 소스 1.0.1.24)는 현행 HMI와 **비호환** — U1/U2는 FDC 없이도 무해하게 선행 가능(공유메모리 기록 자체는 소비자 없어도 안전), 실제 팹 연계는 U3에서

## U1. FDC 경로 — stMainWData 슬롯 400~ 기록 【완료 2026-07-29】

**목표**: ELP 18개 값(2박스×9항목)을 GAI 호환 슬롯 400~에 1초 주기로 발행.

**실행 결과**: 아래 1~4항 전부 적용, ALPHA.sln 빌드 통과, 수정 파일 damaged=0, HMI·FDC(1.0.1.25) 헤더 사본 바이트 동일 유지(`.bak_u1` 백업). 슬롯 배치 확정: `400 + box*20 + item` → EB1=400~408, EB2=420~428, 이름 `EB{1|2}_{TEMP|SMOKE|VOLT|HARM_VOLT|CURR|HARM_CURR|LEAK_CURR|INST_PW|ACCUM_PW}`. 검증 리더: `docs/tools/fdc_sharemem_reader.py` (stMainWData 오프셋 자동감지, 슬롯 20 "VERSION" 기준).

1. `HMI\Source\Include\FDCSheardDefine.h` — `MAIN_WDATA_ELP_START = 400` 및 항목 enum 추가 (GAI `FDCSheardDefine.h:265` 대비 분판검사기 축소판: E_BOX1/2 TEMP·SMOKE + 전력 7종×2박스). GAI 슬롯 호환 이유: 기존 20~79와 무충돌, GAI FDC_LIST의 "SHARE/400~" 관행 재사용.
   - 슬롯 배치안: `400+box*20+item` (box0: 400~408, box1: 420~428, 각 박스 여유 11슬롯) — GAI의 19슬롯 압축 배치보다 확장 여유 확보
2. `MCCLogMgr.h/.cpp` — GAI 패턴 이식 + 결함 개선:
   - `void SetDataToELP(int iBox, int iItem, double dData)` + `double m_dDataToELP[2][9]` + **`CCriticalSection m_cs_ELP`** (GAI에는 락이 없었음 — ALPHA에서는 소켓 수신 스레드가 쓰고 UI 타이머 스레드가 읽으므로 필수)
   - `UpdateSensor()`의 stMainWData 기록부(`:403` `nMainDataCount` 계산 앞) 끝에 ELP 블록 추가. `nNext` 커서와 분리해 `MAIN_WDATA_ELP_START + ...` 인덱스 직접 기록, `nMainDataCount`는 불변(주석으로 "FDC는 count 미사용, 직접 인덱싱" 명기)
   - szName은 의미명 권장: `EB1_TEMP`, `EB1_SMOKE`, `EB1_VOLT`, `EB1_HARM_VOLT`, `EB1_CURR`, `EB1_HARM_CURR`, `EB1_LEAK_CURR`, `EB1_INST_PW`, `EB1_ACCUM_PW` / `EB2_*` (GAI 호환이 필요하면 `ELP_DATA%d` 대안 — FDC_LIST는 인덱스 기반이라 이름은 표시용)
3. `CommTcpIpObjectPGMon.cpp` — `ParseS10F01()`의 데이터 저장 블록 직후 푸시:
   `CMCCLogMgr::GetInstance()->SetDataToELP(iBox, i, dData[i]);` (CIM 선례와 동일한 직접 호출, `#include "Manager/MCCLogMgr.h"` 추가)
4. 편집 방식: 두 파일 모두 한글 주석 포함 CP949 → **Perl raw byte 편집** + damaged=0 검증, `// LYS` 주석. `FDCSheardDefine.h`는 pch 미포함이라 부분 빌드.

**검증**: `docs/tools/elp_elecbox_sim.py`로 S10F01 주입(OUT_OF_SERVICE=0 전환 필요) → 1초 후 아래 파이썬 스니펫으로 슬롯 확인:

```python
import mmap, struct
SLOT = 400; RDATA = 80; HDR = 4  # nMainDataCount(int)
mm = mmap.mmap(-1, 4 + 1000*80 + 4 + 1000*80, "KMAC_Shared_Memory_For_FDC_V2")
for i in range(400, 429):
    off = HDR + i * RDATA
    name = mm[off:off+32].split(b"\0")[0].decode("cp949", "replace")
    data = mm[off+32:off+64].split(b"\0")[0].decode("ascii", "replace")
    if name: print(i, name, data)
```

## U2. MCC I-Log 경로 — dData 컬럼 매핑

**목표**: MCC 주기 로그(I-Log CSV, 1초)에 ElecBox 온도/전력 반영. `Config\MCC\MCCNAME.csv`(20행 = 모터 5 + dData 15)가 이미 컬럼명을 정의하고 있어 **스펙 골격은 준비된 상태**:

| dData 슬롯(제안) | MCCNAME 컬럼 | ELP 소스 (`enumElpMonData`) | 비고 |
|---|---|---|---|
| [0] | TEMP_PC_RACK(℃) | — (기존 PC 온도 유지) | ELP 아님 |
| [1] | TEMP_EB_RACK_1(℃) | GetElpData(0, TEMP) | |
| [2] | TEMP_EB_RACK_2(℃) | GetElpData(1, TEMP) | |
| [3] | N01_T_PW(kWh) | (0, ACCUM_PWR) | 적산 전력 |
| [4] | N01_I_PW(kW) | (0, INST_PWR) | 순시 전력 |
| [5] | N01_R_VOL(V) | (0, VOLT) **?** | ⚠ 컬럼명은 R상 전압, ELP 필드는 전압/고조파전압 — 1:1 의미 불일치 |
| [6] | N01_T_VOL(V) | (0, HARM_VOLT) **?** | ⚠ 상동 |
| [7] | N01_R_CUR(A) | (0, CURR) **?** | ⚠ 상동(전류/고조파전류) |
| [8] | N01_T_CUR(A) | (0, HARM_CURR) **?** | ⚠ 상동 |
| [9]~[14] | N02_* | (1, …) 동일 패턴 | |
| [15]~[16] | (여유 2열) | SMOKE 1/2 후보 | 17열 제한 내 |

**확인 필요 항목(실행 전 결정)**:
1. ⚠ `N0x_R_VOL/T_VOL/R_CUR/T_CUR` 컬럼(R상/T상)과 ELP 필드(전압/고조파/누설)의 의미 매핑 — **MCC 스펙 담당 확인 필수**. 잠정안은 위 표(?, 표시), 확정 전 구현 금지
2. 기존 `UpdateSensor():247-253`의 온도 압축 루프가 dData[0..n]을 가변 점유 중 — MCCNAME 컬럼과 이미 어긋난 잠재 불일치. ELP 반영 시 **dData 고정 슬롯제로 재정렬** 필요(위 표), 기존 MCC 수집 데이터 해석에 영향 가능 → MCC 담당과 컬럼 재정의 합의 후 적용
3. 17열 제한으로 연기감도·누설전류·고조파 일부는 MCC에 못 실림 → **FDC 경로(U1) 전담**으로 역할 분리 명시

**구현(확정 후)**: `UpdateSensor()`의 dData 기록부를 고정 슬롯 매핑으로 교체(`m_dDataToELP` 재사용, 락 하에 스냅샷), `nCntILog` 영향 없음(dMCCSpec은 고정 17열 기록 — `DlgnMCCReport.cpp:1690-1698`).

## U3. 외부 FDC 프로그램 연동 (팹 수집까지)

U1까지는 HMI 단독으로 완결(공유메모리 발행). 팹 FDC 수집까지 이으려면:

1. **FDC 프로그램 소스 — 확보 완료(2026-07-29 확인)**: `FDC\FDC_Ver1.0.1.25\`가 현행 계약 소스임을 검증 — 헤더가 HMI `Source\Include\FDCSheardDefine.h`와 완전 동일(`_V2`/[1000]/TEMP_1=1), Bin exe(2022-04-04)에 `_V2` 문자열 확인. 상세: `docs/FDC_Structure_Comparison.md` §5-1
   - 정비 완료(2026-07-30): `if(nIndex<100)` → `FDC_WDATA_END` 상수화 **완료**, `D:\EXTRN` 의존 이관 **완료**, ALPHA.sln 편입(`HMI\CoreFunction\EX_FDC`, FDC_D/FDC_R 구성) + 헤더 공유 include 단일화 **완료**
   - 추가 발견·수정(2026-07-30): `SOURCE_TYPE_SHAHREMEMORY` 분기가 `SOURCE ADDRESS` 열을 무시하고 전역 순차 카운터로 `stMainWData[k]`를 읽던 결함 + `atoi()` 소수점 절삭 → SOURCE ADDRESS 인덱싱 + `atof()` + 범위 가드로 수정(ELP 행 추가 시 즉시 오작동할 위치였음). 상세: `ELP_Upload_Confirmation_Requests.md` 부록 2-1
2. **FDC_LIST.xls 행 추가**: `SOURCE_TYPE=SHARE`, `SOURCE_ADDRESS=400~428`(U1 슬롯), `SHAREMEM_NUM`(stFDCWData 슬롯 — 기존 사용 슬롯과 중복 금지), `DESTINATION`(PLC D주소) 지정
   - 조사 결과(2026-07-30): 현행 `FDC_LIST.xls` 186행 중 **SHARE 타입 행은 0개** — ELP가 첫 사용 사례. 사용 중 stFDCWData 슬롯은 `1~8, 14~17, 31~46`. ELP용은 미사용 대역인 **250~(신규 `FDC_WDATA_ELP_START` enum)** 제안. 행 추가 초안표: `ELP_Upload_Confirmation_Requests.md` 부록 1
   - ⚠ 기존 결함 발견: `NUM_FDC_WDATA` 슬롯 1~5가 여러 행에 중복 배정(슬롯 5는 진공압과 온도가 동일 슬롯을 다툼) — ELP 무관 기존 사안, 설비/FDC 담당 확인 필요(부록 2-3)
3. **PLC D주소 협의**: DESTINATION D영역은 공장 FDC/CIM이 폴링하는 실주소 — **PLC 담당과 주소 배정 협의 필수** (M/D맵 정합화 작업 `docs/PLC_D_Memory_FDC_*` 참조, 2026-07-24)
4. FDC측 `stMainWData[iData]` 인덱스 범위 가드 추가 — **완료 2026-07-30** (부록 2-1 수정에 포함)

## 실행 순서 및 승인 게이트

| 순서 | Phase | 선행 조건 | HMI 빌드 영향 | 상태 |
|---|---|---|---|---|
| 1 | U1 (FDC 슬롯 발행) | 없음 | MCCLogMgr/PGMon/FDCSheardDefine 부분 빌드 | ✅ 완료 2026-07-29 |
| 1b | FDC 현행화·편입·결함수정 | 없음 | FDC측 빌드만 | ✅ 완료 2026-07-30 |
| 2 | U2 (MCC 컬럼) | ⚠ 확인요청서 B절 회신(MCC 담당) | MCCLogMgr 부분 빌드 | ⏸ 대기 |
| 3 | U3 (외부 FDC 실연동) | ⚠ 확인요청서 C절 회신(PLC D주소) | HMI 무영향(FDC측 빌드 + xls) | ⏸ 대기 |
| 4 | 실장비 시운전 | ⚠ 확인요청서 A·D절 회신 | 설정만(Network.ini IP) | ⏸ 대기 |

각 Phase 실행 전 사용자 승인, 실행 시 `.bak` 백업 + damaged=0 검증 + 빌드 확인은 기존 작업 규칙과 동일.

### 회신 도착 시 즉시 실행 가능한 작업 (사전 준비 완료분)

| 회신 항목 | 실행 내용 | 예상 규모 |
|---|---|---|
| ~~A. ACK Len~~ | ✅ **회신 완료 2026-07-30**: Len/Cause `0000` 확정 = 현행 구현 일치로 변경 불요. 단 "재전송 없음 + 알람이 Critical" 회신에서 도출된 **파서 견고성 결함 수정 완료**(Len 불일치 시 프레임 전체 폐기 → 실제 body 기준 best-effort 파싱). 상세: `ELP_Upload_Confirmation_Requests.md` §A-2 | 완료 |
| B. MCC 매핑 | `MCCLogMgr.cpp` 온도 압축 루프를 고정 슬롯 대입으로 교체 + ELP 값 대입 | ~30줄 |
| C. PLC D주소 | 🔶 **부분 완료 2026-07-30**: enum `FDC_WDATA_ELP_START=250` 대역 + `FDC_LIST.xls` 18행(Excel 190~207) **적용 완료**. `DESTINATION`은 **D20000 일괄 임시** — 주소표(`10000+2i`) 밖이라 PLC 쓰기만 생략되고 공유메모리 경로는 전부 정상. 회신 시 18칸을 **개별 실주소로** 교체 | 잔여: xls 18칸 |
| D. 실장비 IP | `Network.ini` `NETWORK_PARA_10_TCPIP_IP` 1줄 변경 | 1줄 |
