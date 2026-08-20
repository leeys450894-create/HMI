# Aging Cycle 결과 회차 분리 구현 플랜 (OCP 패리티)

작성: 2026-08-12 (LYS)
선행: [AgingCycleEngine_Design_Plan.md](AgingCycleEngine_Design_Plan.md) §C5 (1차 last-write-wins 확정분의 보완), [AgingTest_Porting_Plan.md](AgingTest_Porting_Plan.md)
대상: OCP `DoWorkStartMeasOCP_A`의 회차별 결과 슬롯 분리를 HMI에 이식

## 1. 목적

Aging Cycle 모드에서 같은 스틱을 리스트 아이템 N개로 반복 측정할 때, **현재 HMI는 회차 결과가 전부 소실**된다(스프레드 행 + 장비 raw 파일 동시 덮어쓰기). OCP와 동일하게 회차별 별도 행/별도 파일로 분리해 APD.txt에 전 회차가 실리도록 한다.

**범위**: R1(행 오프셋) + R2(InnerID 재생성) 2건. 회차 식별 컬럼(OCP `REPEAT`/`SEQUENCE` 대응) 신설은 MES 협의 필요 항목으로 **본 플랜 제외** (§7-1).

## 2. 현황 (2026-08-12 코드 조사 결과)

### 2.1 OCP 방식

| 항목 | 내용 | 근거 |
|---|---|---|
| 행 산식 | `nAreaNum = stProbe[n].m_iAreaNum + 1 + (nMaxArea × 항목별_회차카운터)` | `JOB_Proc.cpp:13934`(WAD) `:17133`(VISION) `:15171`(LCR) |
| `nMaxArea` | `nMaxBottom × nMaxTop` = 글라스 전체 셀 수 | `JOB_Proc.cpp:12549` |
| 회차 카운터 | `nPropertyRepeat`/`nQualityRepeat`/`nLCRRepeat` — **항목별 독립·조밀(dense)**, 실제 측정한 경우에만 증가 | 선언 `:12676-12683`(iTop 루프 안), 증가 `:14995` `:17514` `:15531` |
| MultiWAD 공존 | `+ (nRepeatCount × nMaxArea) + (i × nMaxArea) + (nPropertyRepeat × nMeasSize × nMaxArea)` — 회차 블록 **뒤에** 멀티 블록 배치 | `JOB_Proc.cpp:14530` |
| InnerID | 측정마다 신규 생성 (`"%d%s", nProbe+1, 타임스탬프`) → 회차별 raw 파일 보존 | `JOB_Proc.cpp:14116` |
| APD 반영 | `[RAW_DATA]` 전 행 순회, **CELLID 빈 행은 skip** | `DlgNaviData.cpp:10504-10513` |
| `nTGroup` | **결과 버퍼 아님** — `pStInnerID` 할당·`stMeasGrp.nGroup` 대입에만 쓰이고 `_A` 안에서 한 번도 인덱싱되지 않는 dead allocation | `JOB_Proc.cpp:12583-12586` |

> ⚠ 선행 문서가 근거로 삼았던 "`nTGroup = MEASGROUP × nRepeatCount` 결과 버퍼 확장"은 **부정확**했다. 실제 회차 분리는 전적으로 행 산식이 담당한다.

### 2.2 HMI 현황

| 항목 | 내용 | 근거 |
|---|---|---|
| 행 산식 | `nUseCELLID[p] + (nArea × nMeasStick) + 1` — **회차 항 없음** | `DoWorkGlassMeasureStep.cpp:2433`(WAD) `:3338/:3347`(VISION) `:4592`(ANGLE) |
| 저장 키 | `RSRITEMDATA::iMeasArea` 단일 (`iCurArea`/`iCurStep`/`iPanelNo`/`m_iProbe`는 세팅 코드 없는 사장 필드) | `CDlgResult.cpp:519` |
| 덮어쓰기 | 같은 `nMeasMode`+같은 `iMeasArea` → 같은 시트·같은 행 완전 덮어쓰기 | `CDlgResult.cpp:519, 664/982/1682` |
| InnerID | **셀 인덱스 캐시 재사용** (길이<5일 때만 신규) → 회차마다 동일 ID | `DoWorkGlassMeasureStep.cpp:237-245` |
| raw 파일 | 동일 파일명 + `CopyFile(..., FALSE)` → **덮어쓰기** | `CommTcpIpObjectWAD.cpp:1274` / `Vision.cpp:1494` / `Angle.cpp:831` |
| APD 반영 | `[RAW_DATA]` 전 행 순회, **빈 행 skip 없음** | `CDlgResult.cpp:2019-2062` |
| 기존 선례 | WAD MultiPoint가 이미 행 블록 오프셋으로 동일 문제 해결 | `DoWorkGlassMeasureStep.cpp:2586-2587` |

### 2.3 기존 선례 (그대로 따를 패턴)

```cpp
// DoWorkGlassMeasureStep.cpp:2586-2587  (WAD MultiPoint)
nAreaNumM[0] = nUseCELLID[0] + (nArea * nMeasStick) + 1 + (nArea * nMaxStick * ip);
```
`nArea × nMaxStick` = 글라스 전체 셀 수 = OCP `nMaxArea`와 동일 의미. **블록 크기 정의가 이미 일치**한다.

## 3. 설계

### 3.1 행 레이아웃 (블록 모델)

```
행 번호 = (셀 오프셋) + 1 + (블록크기 × 블록인덱스)
블록크기 = nArea × nMaxStick          // 글라스 전체 셀 수 (기존 MultiPoint와 동일)
```

측정 종류별로 **다른 시트**에 기록되므로(WAD/VISION/ANGLE 각각), 블록 인덱스는 **종류별 독립**으로 관리한다.

| 종류 | 시트 | 블록 인덱스 |
|---|---|---|
| WAD 기본점 | 특성 | `rWAD` |
| WAD MultiPoint (`ip`) | 특성 | `nWadMax + rWAD × (nPtTotal-1) + (ip-1)` |
| VISION | 화질 | `rVISION` |
| ANGLE | 시야각 | `rANGLE` |

- `rXXX` = 해당 종류의 **조밀 회차 카운터** (0-base)
- `nWadMax` = aging 리스트 중 WAD 사용 아이템 수 (aging 비활성 시 1)
- `nPtTotal` = MultiPoint 총 점수

**하위호환**: VISION/ANGLE/WAD 기본점은 가산항이 `IsAgingCycleActive()` 가드 안이라 표준 경로가 코드를 밟지 않는다.

MultiPoint 블록만은 **기존 실행 라인을 공유**하므로 주의가 필요하다. 산술적으로는 aging 비활성 시 `rWAD=0`, `nWadMax=1` → `1 + 0 + (ip-1) = ip` 로 기존과 항등이지만, **항등 성립이 `nWadMax`의 구현(미로드 시 0이 아닌 1 반환)에 의존**한다. 이런 암묵 의존은 회귀 위험이므로, **기존 라인을 그대로 두고 aging 분기에서만 새 산식을 쓰는 if/else 구조**로 격리한다 (§6-1).

### 3.2 회차 카운터 정의 (조밀 인덱싱)

OCP와 동일하게 **"실제 측정한 경우에만 증가"** 하는 조밀 카운터를 쓴다. 아이템 인덱스(`GetAgingCurItem()`)를 그대로 쓰면 안 되는 이유:

- 리스트가 `item1=WAD만 / item2=VISION만` 인 경우 아이템 인덱스 사용 시 VISION이 블록 1에 기록 → **블록 0이 비어 빈 행 발생**
- HMI APD는 OCP와 달리 **빈 행 skip이 없어** 공백 라인이 그대로 MES로 전송됨 (`CDlgResult.cpp:2019`)

카운터 위치는 이미 aging 컨텍스트를 보유한 `CMeasureSequenceItemEngine` (스틱 단위 수명 = OCP `iTop` 루프 스코프와 일치).

```
AgingCycleBegin()  → 3종 카운터 0 리셋   (스틱 진입 시 호출됨, 기존 코드)
AgingCycleEnd()    → 3종 카운터 0 리셋   (스틱 종료 시 호출됨, 기존 코드)
Measure_XXX::Run() → 진입 시 r 캡처 → nAreaNum에 반영 → 측정 성공 시 r 증가
```

`r`은 **한 Run() 안에서 불변**이어야 한다 (스텝 내부 셀 루프가 같은 블록을 써야 하므로). 캡처 후 사용, 증가는 Run() 말미 1회.

> **OCP와 동일한 특성 (의도된 동작)**: 종류별 카운터가 독립이므로 `item1=WAD, item2=VISION` 조합에서는 둘 다 블록 0에 기록되어 병합 시 한 행으로 합쳐진다. OCP도 같은 결과이며(`JOB_Proc.cpp:13934` vs `:17133` 카운터 분리), 시트가 달라 데이터 자체는 깨지지 않는다.

### 3.3 InnerID 회차별 재생성

`GenMeasID()` 호출 직전 해당 셀의 InnerID 캐시를 클리어해, 회차마다 새 ID → 새 raw 파일이 생성되게 한다.

기존 동일 idiom (2023.02.23 `bOneCell` 처리)을 그대로 사용:
```cpp
// DoWorkGlassMeasureStep.cpp:2221
if (bOneCell) memset(m_pMeasSeqItemEngine->m_pStInnerID[nUseCELLID[0]], 0x00,
                     sizeof(m_pMeasSeqItemEngine->m_pStInnerID[nUseCELLID[0]]));
```
`sizeof(TCHAR*)`=8byte(=4 TCHAR)만 0으로 만들지만, 선두가 `\0`이 되어 `GetLength() < 5` 판정을 통과하므로 재생성된다. 할당부 초기화(`MeasureSequenceItemEngine.cpp:704`)도 같은 idiom을 쓰고 있어 **일관성 유지 차원에서 동일하게 간다**.

`bOneCell` 선례는 `[0]`만 클리어하지만, aging은 **양 헤드(`nUseCELLID[0]`, `[1]`)** 를 모두 클리어해야 한다 (`>= 0` 가드 필수 — VISION은 `-1`이 들어옴).

## 4. 실행 Phase

> **R1 / R2 구현 완료 (2026-08-12, Debug+Release x64 빌드 통과, `damaged=0`)** — 백업 `.bak_r1r2`(DoWorkGlassMeasureStep.cpp/.h), `.bak_agingflag`(MeasureSequenceItemEngine.cpp/.h). 구현 결과는 §4-R 참조.

### Phase R1 — 행 블록 오프셋

| # | 파일 | 위치 | 변경 |
|---|---|---|---|
| R1-1 | `DoWork/AgingCycleInfo.h/.cpp` | — | `int GetKindUseCount(int kind) const` 추가 — 리스트 중 해당 종류 사용(`USE_AGING=1` && 종류 플래그) 아이템 수 |
| R1-2 | `DoWork/MeasureSequenceItemEngine.h` | `:92-102` aging 블록 | `int GetAgingRepeat(int kind) const` / `void AgingRepeatNext(int kind)` / `int GetAgingKindMax(int kind) const` + `int m_nAgingRepeat[3]` 멤버 |
| R1-3 | `DoWork/MeasureSequenceItemEngine.cpp` | `AgingCycleBegin():716` / `AgingCycleEnd():758` | 카운터 3종 0 리셋 |
| R1-4 | `DoWork/DoWorkGlassMeasureStep.cpp` | WAD `:2423-2424`(bOneCell), `:2433-2434`(기본) | `+ (nArea * nMaxStick * rWAD)` 가산 |
| R1-5 | 〃 | WAD MultiPoint `:2586-2587` | **기존 라인은 그대로 두고** `if (IsAgingCycleActive())` 분기 안에서만 `nWadMax + rWAD*(nPtTotal-1) + (ip-1)` 적용 (§6-1 참조) |
| R1-6 | 〃 | VISION `:3338`, `:3347` | `+ (nArea * nMaxStick * rVISION)` 가산 |
| R1-7 | 〃 | ANGLE `:4592-4593` | `+ (nArea * nMaxStick * rANGLE)` 가산 |
| R1-8 | 〃 | 각 스텝 Run() 말미 | 측정 수행(`bUseMeas==true`) 시 `AgingRepeatNext(kind)` |

- `nArea`/`nMaxStick`은 세 스텝 모두 함수 스코프에 이미 선언되어 있음 (WAD `:1911-1912`, VISION `:2716-2717`, ANGLE `:4112-4113`) — 추가 조회 불필요
- 모든 가산은 `IsAgingCycleActive()` 가드 안에서만. 비활성 시 `r=0`이라 산술적으로도 무변화지만, **표준 경로 코드 무영향 원칙**을 위해 명시 분기

### Phase R2 — InnerID 회차별 재생성

| # | 파일 | 위치 | 변경 |
|---|---|---|---|
| R2-1 | `DoWorkGlassMeasureStep.cpp` | WAD `GenMeasID` 직전 `:2223` | aging 활성 시 `nUseCELLID[0]`, `[1]` InnerID 캐시 클리어 (`>=0` 가드) |
| R2-2 | 〃 | VISION `:3086` | 동일 |
| R2-3 | 〃 | ANGLE `:4416` | 동일 |

**R2-4 (선택 — 사용자 판단 필요)**: MultiPoint 루프의 `GenMeasID`(`:2521`)는 주석에 `// new InnerID per point (separate result file per point)` 라고 되어 있으나 **캐시 재사용으로 실제로는 점마다 같은 파일에 덮어쓰고 있다.** 원 의도대로 고치면 raw 파일이 점 수만큼 늘어나므로, aging과 별개 건으로 분리해 결정한다.

### 4-R. 구현 결과 (2026-08-12)

**R1 — 행 블록 오프셋**

| 파일 | 내용 |
|---|---|
| `DoWork/AgingCycleInfo.h/.cpp` | `GetKindUseCount(iKind)` 신설 — `USE_AGING=1` && 종류 플래그인 아이템 수 (OCP `nMaxProperty/nMaxeQuality/nMaxLCR` 대응) |
| `DoWork/MeasureSequenceItemEngine.h/.cpp` | `m_nAgingRepeat[enumAgingMeasKind_MAX]` + `GetAgingRepeat/AgingRepeatNext/GetAgingKindMax`. ctor·`AgingCycleBegin`·`AgingCycleEnd` 3곳에서 0 리셋 (스틱 단위 수명 = OCP `iTop` 루프 스코프) |
| `DoWork/DoWorkGlassMeasureStep.cpp` | WAD/VISION/ANGLE 각 스텝에 `nAgingRow` 산출 후 `nAreaNum[]`에 가산, 스텝 말미에 `AgingRepeatNext(kind)` 1회 (셀 루프 밖) |

- 각 스텝의 `nAgingRow`는 **함수 진입 후 한 번만 산출**되어 셀 루프 전체에서 불변 — 같은 회차의 셀들이 같은 블록에 들어간다
- 증가는 `bUseMeas == true`일 때만 → OCP의 "실측한 경우에만 증가" 규약과 동일 (조밀 인덱싱 유지)

**R1-5 MultiPoint 격리 (§6-1 적용)** — 기존 두 줄을 `else` 안에 **문자 그대로 보존**하고, aging 활성 분기에서만 새 블록 산식 사용:
```cpp
if (aging active) { nBlock = GetAgingKindMax(WAD) + GetAgingRepeat(WAD) * (nPtTotal-1) + (ip-1); ... }
else              { /* 기존 라인 그대로: (nArea * nMaxStick * ip) */ }
```

**구현 중 발견·수정 — `bOneCell` 블록 충돌**: `bOneCell`(= `nMeasArea == 1`) 경로는 sub-point `k`를 **`nMaxStick` 단위** 블록으로 쓰는데(`nOneCellCnt`=5), 여기에 `nArea * nMaxStick * repeat`를 그냥 더하면 **`nArea == 1`일 때 두 블록이 정확히 겹친다** — 그리고 `bOneCell`이 서는 조건이 바로 `nArea == 1`인 경우다. 따라서 이 경로만 회차를 `k`와 **같은 단위로 중첩**시킨다:
```cpp
nAreaNum[i] = ... + (nMaxStick * (k + nOneCellCnt * nAgingRep));   // 비활성 시 nAgingRep=0 → nMaxStick*k (기존과 동일)
```

**R2 — InnerID 회차별 재생성**

- `CDoWorkGlassMeasureStep::ClearInnerIDForAging(int* pnCellNum)` 신설 — aging 활성 시에만 동작, 양 헤드 순회, `pnCellNum[p] < 0` 및 `>= GetMeasureArea()` 가드
- 호출: WAD `GenMeasID` 직전(기존 `bOneCell` memset 바로 뒤), VISION 두 분기(`nUseHead`/`nUseCELLID`) 각각, ANGLE `GenMeasID` 직전
- **R2-4(MultiPoint InnerID)는 미적용** — 기존 동작 변경이라 별도 판단 항목으로 유지

**미적용 / 범위 밖**: 회차 식별 컬럼(§7-1), MultiPoint InnerID(R2-4), `VISION_COLOR`(비활성 스텝 — 동일 코드 패턴이 있으나 스코프 지정 편집으로 미접촉), COLOR 스텝(HMI 미사용)

## 5. 검증 계획

### 5.0 검증 방법 정정 (2026-08-12 실측) — APD diff는 OOS에서 무효

원안의 1순위 검증(APD.txt diff)을 실행해보니 **모든 APD가 `AREA_CNT=0` / `[RAW_DATA]` 비어 있다.** OOS에는 장비 raw 파일이 없어 dResult가 시트에 아무것도 쓰지 않기 때문이며, 따라서 **행 번호 변경은 APD로 관측되지 않는다.**

대체 관측 수단 2가지를 확보:

1. **InnerID 공유율 (하위호환용 정량 불변식)** — 표준 런에서는 InnerID가 셀 단위 캐시로 재사용되므로 WAD가 만든 ID를 ANGLE·VISION이 그대로 쓴다. R2가 오발화하면 공유율이 0으로 붕괴한다. 스크립트: `scratchpad/aging_compat_check.sh <YYYYMMDD>`
2. **`LogAgingRow` 진단 로그 (R1 검증용, 신설)** — `nAreaNum`은 코드 어디에서도 로깅되지 않아 블록 배치가 관측 불가였다. `CDoWorkGlassMeasureStep::LogAgingRow()`를 추가해 **aging 활성 시에만** `AgingCycle row: <kind> item(n) repeat(r) stick(s) row(a,b)` 를 남긴다 (표준 경로 무영향). 스크립트: `scratchpad/aging_repeat_check.sh <YYYYMMDD>`

### 5.0-1 하위호환 검증 결과 ✅ 통과 (2026-08-12, 레시피 `1_OCP_TEST_Long_260720`, 글라스 1장)

| 항목 | 기준선 08-10 (수정 전) | 08-12 (수정 후) |
|---|---|---|
| AGING_PTN / `AgingCycle: begin` | 0 / 0 | 0 / 0 |
| 스텝 체인 | None→Move→PtrnSearch→Contact→OST→RTEST→AGING→WAD→ANGLE→VISION | 동일 |
| 스텝 합계 | 24~25 (글라스 2장) | 12 (글라스 1장) — 스케일 일치 |
| **VISION ↔ WAD InnerID 공유** | 120/120 (100%) | **60/60 (100%)** |
| **ANGLE ↔ WAD InnerID 공유** | 120/120 (100%) | **60/60 (100%)** |
| Thread NG / WARN / ERR | 1 (중단 기록) | 0 / 0 / 0 |

**게이트 동작 직접 증거**: 08-12 시점에 `AgingRcp\1_OCP_TEST_Long_260720.rcp`가 **이미 존재**했음에도 엔진 로그가 **0줄**이었다 — `Load()` 성공/실패 로그가 둘 다 없다는 것은 그 이전에 리턴했다는 뜻이고, `IsOCP()`는 참이므로 **`m_bUseAging_Test` 플래그 검사에서 정확히 차단**된 것이다. (기준선의 `aging rcp not found` 25건 → 0건도 같은 이유의 정상 차이)

### 5.0-2 회차 분리 검증 결과 ✅ 통과 (2026-08-12, 레시피 `2_OCP_TEST_Short_260720`, 21스틱×5셀, WAD 3회차)

픽스처: `AgingRcp\2_OCP_TEST_Short_260720.rcp` = 3 아이템 전부 `USE_PROPERTY=1`(WAD만), PTN_RGB 1/2/3, PTN_SEC 2, `[MASECELL]` 21스틱 전부 `1,1,1,1,1,`

| 항목 | 결과 |
|---|---|
| `AgingCycle: begin, items(3) active(1)` | 21스틱 × 3런 = 63 |
| repeat 0/1/2 분포 | 315 / 315 / 315 (균등) |
| 행 범위 | repeat0 = 1~105, repeat1 = 106~210, repeat2 = 211~315 |
| 블록 크기 | **105 = 5셀 × 21스틱** = `nArea × nMaxStick` 설계값 일치 |
| **행 충돌** | **0건** |
| PG 점등 | `08010100` → `08010300` × (PTN-1) [0/1/2회] → `1003` — 아이템별 정확 |
| 실패 | 0건 |

회전 반전(rotate=2)도 함께 검증됨 — 스틱 0은 `nMeasStick = 21-0-1 = 20` → 기저 100 → 회차별 101~105 / 206~210 / 311~315.

> `Measure_WAD` 카운트가 201인 것은 오늘 로그에 표준 Long 런(12) + aging 런 3회(63×3)가 누적됐기 때문. AGING_PTN은 189(=63×3)로 aging 런만 계상.

### 5.0-3 InnerID 1초 해상도 충돌 — 발견 및 수정 (2026-08-12)

**발견**: 회차 분리 1차 런에서 WAD 315건 중 **16건(5%) InnerID 중복**. 원인은 R2가 아니라 **`gf_GetCurTimeString(3)`의 1초 해상도**(`CommonFunc.cpp:123`, `%02d%02d%02d`) — 같은 초 안의 두 측정이 **서로 다른 셀인데 같은 ID**를 받는다. 같은 ID → 같은 장비 raw 파일명 → 실기에서 뒤 결과가 앞을 덮어쓴다.

**기존 결함이며 aging 무관** (OCP도 동일 생성기 `JOB_Proc.cpp:14116`). 측정 밀도에 비례해 발현:

| 런 | 측정 수 | 중복 |
|---|---|---|
| 08-10 표준 | 216 | 0 |
| 08-12 Long 표준 (9분) | 108 | 0 |
| 08-12 Short aging 1차 (18분) | 315 | **16** |

**수정 (길이 불변 = 장비 패킷 필드 무영향)**: `CMeasureSequenceItemEngine::MakeUniqueInnerID()` — 발급된 ID와 중복이면 **끝 2자리만** 재매핑(60~99 우선: 실제 초로는 나올 수 없는 값), 유일성 판정은 발급 목록이 담당. `GenInnerID`의 생성 경로 2곳에 연결. 재매핑 시 `InnerID duplicate remapped: A -> B` WARN 기록.

**1차 수정의 결함과 재수정**: 발급 목록을 `SetMeasureArea()`에서 클리어했는데, 이 함수는 **글라스가 아니라 스틱 단위**로 호출된다(`CDoWorkGlassMeasureStep_None::Run()` → `DoWorkGlassMeasureStep.cpp:329`). 목록이 스틱마다 리셋되어 **스틱 간 충돌이 남았다**(중복 16→4, 서로 다른 소스가 같은 타깃 `1165360`으로 재매핑되는 로그로 확인). → **롤링 윈도우로 전환**: `PushIssuedInnerID()`가 최근 `DEFINE_INNERID_KEEP_MAX`(512)개만 유지. ID에 HHMMSS가 들어 있어 오래된 항목은 충돌 불가하므로 생명주기 훅 자체가 불필요해지고, "너무 일찍 리셋"이 구조적으로 불가능해진다.

**최종 검증 (3차 런)**: sends 306 / distinct **306** / **중복 0건**, 재매핑 15건(타깃 중복 0), 재매핑 실패 0건.

### 5.0-4 스틱 매핑 검증 ✅ 1:1 확정 (2026-08-14, rotate=2)

`[MASECELL]`에서 `STICK_1/2/5`만 활성한 비대칭 픽스처로 실행:

```
측정된 GetMeasureStick() 인덱스 = [0, 1, 4]   → 1:1 (STICK_(i+1) ↔ index i)
반전이었다면 [16, 19, 20]이 나왔어야 함
```

**08-06 E4의 1:1 매핑이 rotate=2에서도 성립** — 셀맵 소비 측에 rotate 변환 불필요. 근거: 다이얼로그 `UpdateAgingCell`이 셀맵 그리드를 `RecipeData::m_vecStick` 순서 그대로 만들고 `GetMeasureStick()`도 같은 벡터 인덱스라, 08-07 반전 규약(`nMaxStick-iStick-1`)이 적용되는 **nCellStatus/CellID 도메인과 별개**다.

두 도메인이 같은 로그에 동시에 찍혀 명확히 구분된다:

| 셀맵 | `GetMeasureStick()` | 행 기저 | 유도 `nMeasStick` |
|---|---|---|---|
| STICK_1 | 0 | 100 (행 101~105) | 20 = 21−0−1 |
| STICK_2 | 1 | 95 (행 96~100) | 19 = 21−1−1 |
| STICK_5 | 4 | 80 (행 81~85) | 16 = 21−4−1 |

스틱 필터는 `AGING_PTN`을 막지 않는다(63회 = 3×21 그대로) — OCP와 동일.

### 5.0-5 MultiPoint × 회차 블록 검증 ✅ 통과 (2026-08-14)

**선행 발견**: 그 전까지 **MultiPoint 루프는 한 번도 실행된 적이 없었다.** `WAD Multi Point mode active`는 찍혔지만(모드 ON) `WAD-MP` 행 로그가 0건 — MultiMeas 파일에 포인트가 base 1개뿐이라 `for (ip=1; ip<nPtTotal; ip++)`가 돌지 않았기 때문. 픽스처를 3점(`ENABLE=1,1,1`)으로 바꿔 처음으로 경로를 태웠다.

픽스처: AgingRcp 3아이템(WAD) × 21스틱 전체 / MultiMeasRcp 3점, 다중점 셀 = STICK_1·5·11의 셀 0

블록 = `GetAgingKindMax(WAD)=3 + repeat×(nPtTotal-1=2) + (ip-1)`, 단위 105

| repeat | ip=1 | ip=2 | 실측 행 (stick 0 / 4 / 10) |
|---|---|---|---|
| 0 | blk 3 (+315) | blk 4 (+420) | 416·521 / 396·501 / 366·471 |
| 1 | blk 5 (+525) | blk 6 (+630) | 626·731 / 606·711 / 576·681 |
| 2 | blk 7 (+735) | blk 8 (+840) | 836·941 / 816·921 / 786·891 |

기저 = `nArea × nMeasStick` + 셀 + 1 (stick0=101, stick4=81, stick10=51). **18개 행 전부 계산식과 정확히 일치**, base 블록(0~2)과 MP 블록(3~8) **충돌 0건**, InnerID 345/345 중복 0, 실패 0.

**⚠ `AREA_MAX_CNT` 상한 근접 (실측 941 / 1000)**: 최대 행 = `kindMax × nPtTotal × nArea × nMaxStick`. 이번 조합은 `3 × 3 × 105 = 945`로 상한의 94.5%다. **아이템(회차)이나 다중점을 하나만 더 늘려도 초과**한다 (4×3 = 1260, 3×4 = 1260). 실사용 조합을 정할 때 `kindMax × nPtTotal ≤ 9`(셀 105 기준)를 넘지 않는지 확인하거나, `CDlgResult.cpp:1944`의 하드코딩 1000을 MES와 협의해 상향해야 한다.

### 5.1 OOS 검증 (실기 불필요)

전제: AgingRcp가 **현재 레시피명**으로 존재해야 함 (§7-2 선행).

1. **하위호환 (최우선)** — aging 비활성 표준 사이클 1런: 결과 행 번호·APD `[RAW_DATA]` 행 수가 수정 전과 **완전 동일**한지 (수정 전 APD.txt 보관 후 diff)
2. **MultiPoint 하위호환** — MultiWAD 레시피 1런: 행 블록이 기존과 동일한지 (`ip` → 블록 `ip`)
3. **aging 3아이템 런** — 리스트 `item1=WAD, item2=VISION, item3=ANGLE` (기존 `0_OCP_TEST_260720.rcp` 구성):
   - 각 종류 카운터가 모두 0 → 블록 0 → 병합 후 셀당 1행 (OCP와 동일 결과)
4. **회차 중복 런 (핵심)** — 리스트를 `item1=WAD, item2=WAD, item3=WAD` 로 임시 구성:
   - 스프레드 특성 시트 행 = 셀수 × 3 블록
   - APD `[RAW_DATA]` 행 수가 3배
   - `RawData\{날짜}\{GlassID}\WAD\` 에 회차별 **서로 다른 InnerID 파일 3벌** 생성 (R2 검증)
   - 빈 행(공백 라인) 미발생
5. **혼합 런** — `item1=WAD+VISION, item2=WAD` : WAD 블록 0/1, VISION 블록 0 — 조밀 인덱싱 확인

### 5.2 확인 로그

- 시퀀스: `D:\HBS\Logs\LogFile\GLASS n_*.log` (`AgingCycle: begin, items(N)`)
- PG 송신: `D:\HBS\Logs\Control\PG\PG_YYYYMMDD.log`
- 결과 파일: `{ResultPath}\RawData\{YYYYMMDD}\{GlassID}\{WAD|VISION|ANGLE}\`
- APD: `{ResultPath}\APD.txt` + 백업 `APD\APD_{time}.txt`

## 6. 리스크

### 6-1. MultiPoint 라인 격리 (필수)

`:2586-2587`은 표준 경로가 실행하는 유일한 공유 라인이다. **덮어쓰지 말고 분기로 감싼다**:

```
기존 라인 유지  ← aging 비활성 (표준/MultiWAD 경로, 문자 그대로 보존)
else 분기       ← aging 활성 시에만 새 블록 산식
```
이렇게 하면 `nWadMax` 구현이 어떻든 표준 경로 결과는 바뀔 수 없다.

| 리스크 | 완화 |
|---|---|
| 표준 시퀀스 행 번호 회귀 (전 결과 경로 영향) | `IsAgingCycleActive()` 가드 + 검증 5.1-1 APD diff 필수 |
| MultiPoint 블록 산식 변경으로 기존 MultiWAD 회귀 | §6-1 if/else 격리 (산술 항등에 의존하지 않음) + 검증 5.1-2 |
| `AREA_MAX_CNT` 초과 | 현재 하드코딩 **1000** (`CDlgResult.cpp:1944`). 소요 = 셀수×스틱수×최대회차. 단변 21스틱×9셀×3 = 567, 장변 12×9×3 = 324 → 여유 있음. **회차 4 이상 또는 셀 증가 시 재확인** |
| raw 파일 수 증가 (회차 배수) | 디스크 사용량 증가 — 기존 보관 정책 확인 |
| 종류별 카운터 독립으로 인한 병합 행 합쳐짐 | OCP 동일 동작 (§3.2) — 설계 특성으로 수용, MES 협의 시 함께 제시 |
| InnerID 클리어 누락 시 raw 덮어쓰기 잔존 | 검증 5.1-4에서 파일 3벌 생성 확인 |

## 7. 미결 / 선행 항목

1. **회차 식별 컬럼 (본 플랜 제외)** — OCP는 시트1에 `REPEAT`(`1_특`) + `SEQUENCE`(누적 `특`/`화`) 컬럼으로 회차를 식별한다(`DlgNaviData.cpp:8506-8561`). HMI에는 대응 컬럼이 없어, R1/R2 적용 후에도 **APD에서 회차 구분이 불가**하다. 행은 `SERIAL`(행 번호)로만 구분되고 CELLID는 회차 간 동일. → MES가 회차를 어떻게 식별할지 협의 후 별도 Phase.
2. **AgingRcp 파일명 불일치 (선행 필수)** — 현재 운용 레시피(`1_OCP_TEST_Long_260720`/`2_OCP_TEST_Short_260720`)용 AgingRcp가 없어 `AgingCycleBegin`이 실패하고 표준 시퀀스로 폴백한다. 검증 전에 다이얼로그에서 재저장 필요. (`AgingTest_Porting_Plan.md` 잔여 R6)

### 7-0. 사이클 진입 게이트 — 전용 레시피 플래그로 분리 ✅ 완료 (2026-08-12)

**변경 전 문제**: 게이트가 `m_bUseAgingTest_2`(= AGING **측정 항목** 사용 여부)였다. 이 플래그는 현재 운용 레시피 대부분에서 켜져 있어(`1_OCP_TEST_Long_260720`/`2_OCP_TEST_Short_260720` 모두 1), AgingRcp 파일이 생기는 순간 **AgingTest START를 누르지 않은 일반 Auto/Online 사이클까지 에이징 리스트 경로로 진입**하게 된다. 지금은 AgingRcp 부재로 폴백 중이라 드러나지 않았을 뿐이며, 검증 선행조건(§7-2)을 충족시키는 순간 발현되는 구조였다.

**조치 (사용자 지시)**: 사용자가 신설한 전용 레시피 플래그 `S_InspectItem::m_bUseAging_Test`(`preDefine.h:119`, RECIPE UI 검사항목 행 8)로 게이트를 교체.

```cpp
// MeasureSequenceItemEngine.cpp:724-727  (LYS 2026-08-12)
if (pRecipeInfo == nullptr || pRecipeInfo->m_sInspectItem.m_bUseAging_Test == FALSE)
    return false;
```

- 헤더 주석도 동기화 (`MeasureSequenceItemEngine.h:93`)
- `m_bUseAgingTest_2`를 쓰는 나머지 지점(`DoWorkGlassAGINGStep.cpp` 7곳, `DoWorkGlassMeasureStep.cpp:5778` AGING 스텝, `DoWorkGlassPreMeasStep`, `DoWorkGlassRecipeStep`, `DoWorkGlassResultStep:75`)은 **AGING 측정 항목 자체의 게이트**라 미변경
- Debug x64 빌드 통과, `damaged=0`. 백업 `.bak_agingflag`

**레시피 파일 현황 (2026-08-12 실측)** — 의도한 분리가 이미 성립:

| 레시피 | `_VERSION_` | `_INSPECTION_ITEM_` ints | `m_bUseAging_Test` |
|---|---|---|---|
| 1_OCP_TEST_Long_260720 | 1006 | 10 | **0** |
| 2_OCP_TEST_Short_260720 | 1007 | 10 | **1** |
| 0_OCP_TEST_260720 | 1006 | 10 | 0 |
| HMI.rcp | 1005 | 9 | 0 (구버전, 가드로 skip) |

> 버전 가드 `if (nVersion >= RECIPE_FILE_V1006)`(`InRecipeFunc.cpp:167`, `OutRecipeFunc.cpp:150`)는 통상 규약(`>`)과 다르지만 **본 건에서는 정상**이다: 필드 추가 시점과 `RCP_VERSION` enum 반영 시점 사이에 저장된 파일들이 버전 1006이면서 필드를 이미 갖고 있기 때문(위 표 ints=10). 1005 이하 구파일은 9 ints로 정상 skip된다.
3. **회전 체계 변경 후 스틱 필터 재검증** — `GetMeasureStick() ↔ STICK_(n+1)` 1:1 매핑 실증은 2026-08-06로, 스틱 반전 규약/`m_nGlsRotate=2` 확정(2026-08-07) **이전**이다. 단변 레시피 재검증 필요.
4. **부수 결함 (aging 무관, 별건)**:
   - VISION `sQualityWait` 슬롯이 동일 InnerID로 중복 점유 시 오매칭 가능, `QUALITY_MAX` 소진 시 **무로그 무시** (`CommTcpIpObjectVision.cpp:1600`) — R2로 InnerID가 회차마다 달라지면 **완화되는 방향**
   - `WM_MSG_DISPLAY_RESULT` 핸들러 미등록 → 결과마다 1초 타임아웃 소진 (`CDlgResult.cpp:601-618`). 회차 배수로 늘면 **tact 악화** — R1 적용 시 영향 확대되므로 함께 검토 권장
