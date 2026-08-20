# bOneCell(1셀 5점) → WAD Multi Point 대체 플랜

작성: 2026-08-18 (LYS)
관련: [WAD_MultiMeas_Porting_Plan.md](WAD_MultiMeas_Porting_Plan.md), [Result_Identification_MES_Agreement_Plan.md](Result_Identification_MES_Agreement_Plan.md), [AgingResult_RepeatSlot_Plan.md](AgingResult_RepeatSlot_Plan.md)

## 1. 결론 먼저

결과 시트의 **`REPEAT` 컬럼은 "재시도 횟수"가 아니라 `bOneCell` 서브 포인트 번호(`k+1`)** 였다. 인자 이름이 `nRetry`, 구조체 필드가 `m_iRetryCnt`여서 재시도로 오인되어 있었을 뿐이다.

`bOneCell`은 하드코딩된 **1측정그룹 × 5점** 다점 측정이고, WAD Multi Point가 이를 대체하는 기능이다. 따라서:

- `bOneCell`을 **폐지**하고
- `REPEAT` 컬럼의 소스를 **Multi Point 측정점 번호**로 전환하면

**스키마 변경 없이 다점 식별자가 확보된다.** 이는 MES 협의안 §7 **Q3(신규 컬럼 수용 여부)를 회피**하고 **Q4(현재 REPEAT 사용 여부)의 답을 명확히** 만든다 — 컬럼의 의미가 처음부터 "측정점 번호"였기 때문이다.

## 2. 현행 체인 (전수 확인)

### 2.1 `bOneCell` 발동 조건 — **셀 1개가 아니라 측정그룹 1개**

```cpp
// DoWorkGlassMeasureStep.cpp:2142
nMeasArea = (int)vecMeasGroup.size();      // 2Head 페어링 후의 "그룹" 수

// :2156-2160
BOOL bOneCell = FALSE;  int nOneCellCnt = 1;
if (nMeasArea == 1) { bOneCell = TRUE; nOneCellCnt = 5; }   // 1그룹 -> 5점 반복
```

> ⚠ 기존 코드 주석과 본인이 2026-08-12에 추가한 주석(`:2515`)이 이를 "`nArea == 1`(셀 1개)"로 서술했는데 **부정확**하다. `nMeasArea`는 페어링 후 그룹 수이므로 셀 1~2개 스틱에서 성립한다. 본 작업에서 함께 정정한다.

MultiWad가 활성이면 강제 해제된다:
```cpp
// :2174-2175
bMultiWad = true;  bOneCell = FALSE;  nOneCellCnt = 1;
```
즉 **두 기능은 이미 상호배타**이고, MultiWad가 우선한다.

### 2.2 `bOneCell` 루프가 하는 일

| 라인 | 동작 |
|---|---|
| `:2181` | `for (int k = 0; k < nOneCellCnt; k++)` — 5회 반복 |
| `:2301` | 매 회 InnerID 강제 재생성(`memset`) → 회차별 raw 파일 분리 |
| `:2512-2517` | 결과 행 블록 `+ (nMaxStick * k)` |
| `:2519-2523` | `k >= 3`이면 `iMeasMode = 2`, `bOneCellWadRight = TRUE` (오른쪽 헤드 전용 측정) |
| `:2553` | `DoWorkInspectComplate(WAD, iMeasMode, nTimeOut, **k + 1**)` |

### 2.3 `REPEAT` 컬럼까지의 값 전달

```
k + 1                                              DoWorkGlassMeasureStep.cpp:2553
  -> DoWorkInspectComplate(..., int nRetry)         CommTcpIpFactoryMgr.h:86
  -> TossResultFile(innerID, area, probe, nRetry)   CommTcpIpObjectWAD.cpp:486 / :390
  -> resultData.m_iRetryCnt = nRetry                CommTcpIpObjectWAD.cpp:1338
  -> SetDataString(6, row, m_iRetryCnt)             CDlgResult.cpp:988
     헤더 6번 = "REPEAT"                            CDlgResult.cpp:970
```

**Multi Point 경로는 이 인자에 리터럴 `1`을 넘긴다**(`:2702`) → 다점 측정이어도 `REPEAT`는 항상 1.

### 2.4 부수 확인

| 항목 | 상태 |
|---|---|
| `m_iRetryReason` (컬럼 5, 헤더 `EL_PTP`) | **대입처 0건** — 항상 0 |
| `RSRITEMDATA::nMultiPoint` | `GetRunCurrent(iProbe)`로 채워지나 **`> 1`일 때만**(기본점은 0). 소비처는 색좌표 오프셋 가산뿐(`CDlgResult.cpp:1295-1302`) — **컬럼 기록 없음** |
| Multi Point 실행 상태 | 시퀀스가 이미 `SetRunCurrent(p, 1)`(기본점, `:2541`) / `SetRunCurrent(p, nRunCnt)`(다점, `:2612`)로 관리 중 |
| COLOR 스텝의 `bOneCell` | `:3800~`에 동일 구조 존재. **스텝 자체가 미등록(주석)** 이라 미실행 |

## 3. 대체 설계

### 3.1 목표 상태

| 항목 | 현행 | 목표 |
|---|---|---|
| 다점 측정 수단 | `bOneCell`(하드코딩 5점) + Multi Point(레시피) 이원화 | **Multi Point 단일화** |
| `REPEAT` 컬럼 값 | `bOneCell` 스틱만 1~5, 그 외 항상 1 | **모든 스틱에서 측정점 번호** (1=기본점, 2..N=다점) |
| 값의 출처 | `nRetry` 인자 | `CWadMultiMeasMgr::GetRunCurrent(iProbe)` |

### 3.2 값 소스 — 두 경로 비교

**안 (a) 인자 경로 유지 (최소 변경)**
`:2702`의 리터럴 `1`을 `nRunCnt`로 교체.
- 장점: 1줄. 단점: 기본점 경로(`:2553`)도 `k+1`을 넘기고 있어 이원화가 남는다

**안 (b) 수신부에서 실행 상태를 읽어 채움 (권장)**
`TossResultFile`에서 `m_iRetryCnt`를 `GetRunCurrent(iProbe)` 기반으로 세팅하고, `nRetry` 인자 의존을 끊는다.
- 장점: **이미 `nMultiPoint`가 같은 방식으로 동작 중**이라 새 규약이 아님. 기본점/다점이 한 곳에서 일관 처리
- 단점: 수신 시점에 `RunCurrent`가 유효해야 함 → 기존 `nMultiPoint`가 이미 이 전제 위에서 동작하므로 추가 위험은 없음
- 부수 정리: `nMultiPoint`의 `> 1` 가드를 풀어 기본점 = 1로 통일하면 `m_iRetryCnt`와 `nMultiPoint`가 같은 값이 되어 **둘 중 하나로 정리 가능**

→ **안 (b) 채택**, 안 (a)는 폴백.

### 3.3 `bOneCell` 폐지 방식

**단계적 폐지**를 권장한다.

| 단계 | 방식 | 되돌리기 |
|---|---|---|
| 1차 | `nOneCellCnt = 5` → `1` (루프 1회화). 조건/분기는 남김 | 상수 1자 복원 |
| 2차 | 검증 후 `bOneCell`/`nOneCellCnt`/`bOneCellWadRight` 및 관련 분기 제거 | 백업 복원 |

1차만으로 **동작은 완전히 목표 상태**가 된다(1그룹 스틱도 1회만 측정). 2차는 정리 작업이다.

## 3.4 확인 결과 (2026-08-18 사용자 확정)

| 질문 | 답 | 영향 |
|---|---|---|
| Q1 `k >= 3` 오른쪽 헤드 전용 측정이 필요한가 | **불필요** | 대응 기능 신설 없이 폐지 |
| Q2 1그룹 스틱 5회 측정이 실사용 중인가 | **해당 레시피 없음** | 측정 횟수 감소 리스크 소멸 |

두 리스크가 모두 해소되어 M1·M2를 즉시 적용했다.

## 4. 작업 Phase

### Phase M1 — `REPEAT` 소스 전환

| # | 파일 | 변경 |
|---|---|---|
| M1-1 | `CommTcpIpObjectWAD.cpp:1294~1338` | `nMultiPoint`의 `if (nCurPoint > 1)` 가드 완화 → 기본점 1 포함. `m_iRetryCnt`를 `GetRunCurrent(iProbe)` 결과로 세팅(Multi 비활성 시 1) |
| M1-2 | `DoWorkGlassMeasureStep.cpp:2702` | 리터럴 `1` → 실제 점 번호(안 (a) 병행 시). 안 (b) 단독이면 불필요 |
| M1-3 | — | `m_iRetryCnt` 필드명이 의미와 불일치 — **개명하지 않는다**(구조체는 dResult DLL과 공유되는 계약). 주석으로 실제 의미를 명시 |

### Phase M2 — `bOneCell` 1차 폐지

| # | 파일 | 변경 |
|---|---|---|
| M2-1 | `DoWorkGlassMeasureStep.cpp:2159-2160` | `nOneCellCnt = 5` → `1` + 폐지 사유 주석 |
| M2-2 | `:2512-2517` | aging 블록 산식의 `bOneCell` 분기 단순화 (`nOneCellCnt == 1`이면 일반 분기와 동일) |
| M2-3 | `:2515` | 2026-08-12 주석의 `nMeasArea == 1` ≠ 셀 1개 오기 정정 |

### Phase M3 — 정리 (선택)

| # | 내용 |
|---|---|
| M3-1 | `bOneCell` / `bOneCellWadRight` / `k` 루프 제거, `:2301` InnerID memset은 **유지**(aging 회차용 `ClearInnerIDForAging`과 별개 경로) |
| M3-2 | COLOR 스텝(`:3800~`)의 동일 구조 — 스텝이 비활성이므로 **미접촉 권장** |
| M3-3 | `m_iRetryReason`(컬럼 5, 헤더 `EL_PTP`) 미사용 확정 → 헤더명 정정 또는 용도 부여를 MES 협의에 포함 |

### M1·M2 구현 결과 ✅ 완료 (2026-08-18, Debug/Release x64 빌드 통과, `damaged=0`)

백업: `CommTcpIpObjectWAD.cpp.bak_repeatsrc`, `DoWorkGlassMeasureStep.cpp.bak_onecell`

**M1 — `REPEAT` 소스 전환** (`CommTcpIpObjectWAD.cpp`)

```cpp
:1299   int nPointNo = 1;                       // 기본점도 1 (기존 nMultiPoint는 0)
:1311   nPointNo = nCurPoint;                   // 다점일 때만 갱신 (오프셋 조회 성공 시)
:1320   resultData.nMultiPoint = nPointNo;
:1347   resultData.m_iRetryCnt = nPointNo;      // 결과 시트 6열 "REPEAT"
```
- 오프셋 가산 게이트는 `> 1` 유지 → 기본점은 여전히 오프셋 없음. `API_Result.h:120` 주석이 이미 `0/1 = no offset`으로 규정하고 있어 소비처(`CDlgResult.cpp:1295`) 영향 없음
- `nRetry` 인자는 시그니처만 유지(호출부 무변경), 값은 더 이상 사용하지 않음
- `m_iRetryCnt` 필드명은 dResult DLL 공유 계약이라 **개명하지 않고 주석으로 실제 의미 명시**

**M2 — `bOneCell` 1차 폐지** (`DoWorkGlassMeasureStep.cpp`)

```cpp
:2157-2164   nOneCellCnt = 5  ->  1        // 루프 1회화, 분기 구조는 보존(되돌리기 1자)
:2517-2522   aging 블록 오프셋을 일반 분기와 동일한 nAgingRow로 통일
```
- `k >= 3` 오른쪽 헤드 전용 분기(`bOneCellWadRight`)는 `k`가 항상 0이 되어 자연히 미도달
- 2026-08-12 주석의 `nMeasArea == 1` = "셀 1개" 오기를 **2Head 그룹 수**로 정정

### Phase M4 — 검증

1. **하위호환**: Multi Point 미사용 레시피(Long) 1런 → 모든 행의 `REPEAT` = 1, 행 번호·측정 횟수 수정 전과 동일
2. **1그룹 스틱**: 셀 1~2개 스틱을 가진 레시피로 1런 → 측정 **1회**(기존 5회), `REPEAT` = 1
3. **Multi Point**: 3점 레시피 1런 → 같은 셀에 `REPEAT` = 1/2/3, 행 블록 분리 유지
4. **Aging 병행**: 3회차 × 3점 → 회차 블록 × 점 블록 충돌 0 (2026-08-14 검증 스크립트 재사용)
5. APD `[RAW_DATA]`에서 `REPEAT` 값 확인

## 5. 리스크

| 리스크 | 내용 | 완화 |
|---|---|---|
| **1그룹 스틱 측정 횟수 감소** | 현재 5회 → 1회. **의도된 변경**이나 기존 측정 데이터량이 줄어든다 | 대상 스틱이 실제로 존재하는지 레시피 조사 선행. 필요하면 Multi Point 레시피로 5점을 명시 구성해 동등화 |
| `k >= 3` 오른쪽 헤드 전용 분기 소실 | `bOneCell` 전용 로직(`iMeasMode = 2`) | 1그룹 스틱에서만 동작하던 것 — Multi Point에는 대응 개념이 없음. **동등 기능이 필요한지 확인 필요** (§6 Q1) |
| MES가 `REPEAT`를 재시도로 해석 중 | 값 범위는 그대로 작은 정수라 파싱은 깨지지 않으나 **의미가 달라짐** | 협의안 Q4로 확인 |
| `m_iRetryCnt` 이름/의미 불일치 지속 | 구조체는 dResult DLL과 공유 계약이라 개명 불가 | 주석 명시 (M1-3) |

## 6. 확인 필요

| # | 항목 | 판단 주체 |
|---|---|---|
| Q1 | `bOneCell`의 `k >= 3` **오른쪽 헤드 전용 측정**(`iMeasMode = 2`)에 대응하는 기능이 Multi Point에 필요한가? | 사내 |
| Q2 | 1그룹 스틱을 5회 측정하던 기존 동작이 **실사용 중이었는가**? (측정 이력/레시피 확인) | 사내 |
| Q3 | `REPEAT` 의미 확정(측정점 번호)을 MES에 통지 → 협의안 Q4 갱신 | MES |
| Q4 | 에이징 회차 식별자는 여전히 별도 필요 — `REPEAT`는 점 번호로 확정되므로 회차는 신규 컬럼이 필요하다 | MES (협의안 §6 안 A로 축소) |

## 7. 협의안에 미치는 영향

본 작업이 반영되면 [Result_Identification_MES_Agreement_Plan.md](Result_Identification_MES_Agreement_Plan.md)가 다음과 같이 정리된다.

| 협의안 항목 | 변경 |
|---|---|
| §6 안 B(REPEAT 재사용) | **폐기** — `REPEAT`는 원래부터 점 번호였고 다점 식별에 그대로 쓴다 |
| §6 안 A | **회차(`REPEAT_NO`) 1개만** 신규 추가로 축소 (`POINT_ID` 불필요) |
| Q3 | 신규 컬럼 1개로 축소 |
| Q4 | "현재 REPEAT = bOneCell 점 번호였고, Multi Point 점 번호로 이관한다"로 답이 확정 |
