# Resume Inspection (이어 검사) 구현 문서

> 최종 업데이트: 2026-07-07
> 상태: Phase 1~7 전체 구현 완료 + Phase 6 검증 완료

---

## 1. 현재 검사 Flow 분석

### 1.1 Auto Sequence (9-Step)

```
EngineStart() → ThreadRun() → ThreadRu_nSequence()
  [0] Adsorption     — Glass 흡착
  [1] Recipe         — Recipe Load + ResultClear ★
  [2] GlassSWAlign   — Glass Align
  [3] ProbeSWAlign   — Probe Align
  [4] PreMeas        — PAD Search, Contact, OST, R-Test
  [5] AGING          — P1~P4 AGING
  [6] Measure        — N-Stick 측정 루프 ★★★ (핵심)
  [7] Result         — 결과 저장, 판정
  [8] Unload         — Safety 이동, CIM
```

### 1.2 Stick 루프 구조

**파일:** `DoWorkGlassMeasure.cpp:134`

```cpp
int nStepIndex = GetLoopStepIndex();  // = 총 Stick 수 (Recipe에서 설정)
for (size_t i = 0; i < nStepIndex; i++)  // N-Stick 루프
{
    pSeqItem->SetMeasureStick(i);
    // 각 Stick: [0]None → [1]Move → [2]Ptrn → [3]Contact → [4]OST
    //           → [5]RTEST → [6]AGING → [7]WAD → [8]ANGLE → [9]VISION → [10]Idle
}
```

- `nStepIndex` = `GetLoopStepIndex()` = `pRecipeData->m_vecStick.size()`
- `SetLoopStepIndex(nStick)` — `DoWorkGlassSequenceEngine.cpp:856`에서 설정

### 1.3 Result 저장 구조

```
Comm Object (WAD/Angle/Vision/Color)
  → TossResultFile(): raw 파일을 디스크에 복사
    경로: {ResultPath}/RawData/{YYYYMMDD}/{GlassID}/{TYPE}/
  → Updata_Data(RSRITEMDATA): 경로+메타데이터를 Ring Buffer에 전달
  → Monitor Thread → ReadResultData(): raw 파일 파싱 → FPSpread Grid
  → Step[7] SaveResult(): FPSpread → Excel/APD 파일 출력
```

- **ClearData()**: 메모리(FPSpread/Ring Buffer)만 초기화, **디스크 raw 파일 삭제 안 함**
- **ClearData() 호출 시점**: `DoWorkGlassRecipeStep.cpp:50` (Step[1] Recipe 시작 시)

---

## 2. Resume 기능 설계

### 2.1 ResumeInfo.ini

**경로:** `{ExeDir}\Config\ResumeInfo.ini`

```ini
[RESUME]
; --- Identification ---
GlassID=GLASS001
RecipeID=RCP_A01
LotID=LOT001
InnerID=143025

; --- Progress ---
LastStick=15
TotalStick=30
Running=1
Completed=0

; --- Time/Path ---
SaveTime=20260703-150530
StartTime=20260703-140000
InspDate=20260703
ResultPath=D:\Result\
```

| 항목 | 타입 | 용도 |
|------|------|------|
| `GlassID` | CString | Glass 식별자 |
| `RecipeID` | CString | Recipe 식별자 |
| `LotID` | CString | Lot 추적 |
| `InnerID` | CString | raw 파일 식별 (`{InnerID}_1.dat` 등) |
| `LastStick` | int | 마지막 완료 Stick 번호 (0-based, -1=미시작) |
| `TotalStick` | int | 전체 Stick 수 (검증용) |
| `Running` | bool(0/1) | 검사 진행 중 플래그 |
| `Completed` | bool(0/1) | 검사 완료 플래그 |
| `SaveTime` | CString | 최종 저장 시간 (`YYYYMMDD-HHMMSS`) |
| `StartTime` | CString | 검사 시작 시간 (최초 Save 시 1회 기록) |
| `InspDate` | CString | 검사 날짜 (`YYYYMMDD`, raw 파일 경로용) |
| `ResultPath` | CString | Result 폴더 경로 (raw 파일 위치 복원용) |

### 2.2 SystemConfig.ini 옵션

```ini
[OPT_PARA]
USE_RESUME_INSPECTION=0        ; Resume 기능 ON/OFF (0=OFF, 1=ON)
RESUME_EXPIRE_HOURS=0          ; Resume 유효 시간 (0=무제한, 24=24시간 후 자동 폐기)
```

### 2.3 State Diagram

```
[검사 시작 버튼 클릭] (DlgSubOperate)
    │
    ▼
[USE_RESUME_INSPECTION 확인]
    │
    ├─ OFF → SetResumeMode(false) → [정상 시작]
    │
    └─ ON → [ResumeInfo.ini 확인]
              │
              ├─ 파일 없음 / Running=false → SetResumeMode(false) → [정상 시작]
              │
              └─ Running=true, Completed=false, 유효기간 내
                    │
                    ▼
              [ValidateResume(RecipeID, TotalStick)]
                    │
                    ├─ 실패 → "Recipe/Stick mismatch" 알림 → ClearResumeInfo → [정상 시작]
                    │
                    └─ 성공 → [Resume 팝업 표시 (Yes/No/Cancel)]
                                │
                                ├─ Yes (이어 검사) → SetResumeMode(true, LastStick+1)
                                │
                                ├─ No (처음부터) → ClearResumeInfo → SetResumeMode(false)
                                │
                                └─ Cancel → return (검사 시작 안 함)
```

### 2.4 Resume Sequence Flow

```
[검사 시작]
  → EngineStart()
    → SaveResumeInfo(GlassID, RecipeID, -1, Running=true, ...)  ← 검사 시작 기록

[Step 1: Recipe]
  → DoWorkGlassRecipeStep_None::Run()
    → if (IsResumeMode())
        → ResultClear 건너뛰기
        → 이전 Stick Raw 파일 재주입 (Updata_Data)  ← 2026-07-07 추가
    → else → CResultMgr::ClearData() 정상 실행

[Step 6: Measure]
  → DoWorkGlassMeasure::ThreadRun()
    → if (IsResumeMode()) → nStartStick = GetResumeStartStick()
    → SetResumeMode(false)  ← 플래그 리셋 (이후 정상 모드)
    → for (i = nStartStick; i < nStepIndex; i++)
        → 각 Stick 측정 실행
        → Stick 완료 시 → UpdateStickProgress(i)  ← INI 갱신

[정상 완료]
  → ClearResumeInfo()  ← 파일 삭제

[Cancel / NG / Alarm / 강제 종료]
  → ResumeInfo.ini 유지 (Running=true, Completed=false)
  → 다음 실행 시 이어검사 가능
```

---

## 3. 구현 결과

### 3.1 추가 파일

| 파일 | 역할 |
|------|------|
| `Manager/ResumeInfoMgr.h` | Resume 정보 관리 클래스 헤더 |
| `Manager/ResumeInfoMgr.cpp` | Load/Save/Clear/Validate + Atomic Save + Expire |

### 3.2 수정 파일

| 파일 | 변경 내용 |
|------|-----------|
| `AuroraMgrPool.h` | `CResumeInfoMgr*` 포인터 + forward declare 추가 |
| `AuroraMgr.h` | `GetResumeInfoMgrPtr()` getter 추가 |
| `AuroraMgr.cpp` | ResumeInfoMgr include, 생성(`new`), `SetExpireHours()`, 소멸(`delete`) |
| `BaseSequenceBody.h` | `GetResumeInfoMgrPtr()` getter 추가 |
| `SystemConfigMgr.h` | `m_bUseResumeInspection`, `m_nResumeExpireHours` 멤버 + getter |
| `SystemConfigMgr.cpp` | `USE_RESUME_INSPECTION`, `RESUME_EXPIRE_HOURS` INI Link |
| `SystemParamShare.h` | `m_bResumeMode`, `m_nResumeStartStick` 멤버 + `SetResumeMode()` / `IsResumeMode()` / `GetResumeStartStick()` |
| `SystemParamShare.cpp` | 생성자/Initialize 초기화 |
| `DlgSubOperate.h` | `CheckAndApplyResumeMode()` 선언 추가 |
| `DlgSubOperate.cpp` | Resume 체크 공통 함수 추출, 수동/CIM Online 양쪽 경로에서 호출 |
| `DoWorkGlassSequenceEngine.cpp` | 검사 시작 시 `SaveResumeInfo()`, 정상 완료 시 `ClearResumeInfo()` |
| `DoWorkGlassMeasure.cpp` | Stick 루프 시작 인덱스 Resume 적용, Stick 완료 시 `UpdateStickProgress()`, ResumeMode 플래그 리셋 |
| `DoWorkGlassRecipeStep.cpp` | Resume 모드 시 `ResultClear` 건너뛰기 + 이전 Raw 파일 FPSpread 재주입 |
| `ALPHA.vcxproj` | ResumeInfoMgr.h/.cpp 프로젝트 등록 |
| `ALPHA.vcxproj.filters` | ResumeInfoMgr.h/.cpp 필터 등록 |

### 3.3 CResumeInfoMgr 주요 API

| 메서드 | 용도 |
|--------|------|
| `LoadResumeInfo()` | INI 파일에서 12개 필드 로드 |
| `SaveResumeInfo(...)` | Full Save (검사 시작 시, 9개 파라미터) + Atomic Write |
| `UpdateStickProgress(nLastStick)` | 경량 Save (Stick 완료 시, LastStick+SaveTime만 갱신) |
| `ClearResumeInfo()` | 파일 삭제 (정상 완료 시) |
| `IsResumeAvailable()` | Running=true, Completed=false, 유효기간 내 체크 |
| `ValidateResume(RecipeID, TotalStick)` | Recipe/Stick 수 일치 검증 |
| `SetExpireHours(n)` | 유효 시간 설정 (SystemConfig에서 로드) |
| `GetResumeStartStick()` | `LastStick + 1` 반환 |

### 3.4 Atomic Save 구현

```
WriteToFile(tmp 경로)
  → WritePrivateProfileString × 12
  → Flush (NULL, NULL, NULL)
  → PathFileExists 검증 ← 실패 시 false 반환, 원본 보존
  ↓ 성공
DeleteFile(원본)
MoveFile(tmp → 원본) ← NTFS atomic rename
  ↓ 실패 시
CopyFile(tmp → 원본) + DeleteFile(tmp)
```

---

## 4. Resume 정보 저장 시점

| 시점 | 동작 | 저장 내용 |
|------|------|-----------|
| 검사 시작 | `SaveResumeInfo()` (Full) | LastStick=-1, Running=true, TotalStick, LotID, InnerID, ResultPath |
| Stick N 완료 | `UpdateStickProgress(N)` (경량) | LastStick=N, SaveTime 갱신 |
| 정상 완료 | `ClearResumeInfo()` | 파일 삭제 |
| Cancel / NG | (변경 없음) | Running=true 유지 → 다음 실행 시 Resume 가능 |
| 강제 종료 | (변경 없음) | 마지막 `UpdateStickProgress` 상태 유지 |

---

## 5. ResumeMode 플래그 생명주기

```
DlgSubOperate     : CheckAndApplyResumeMode()
                    → SetResumeMode(true, startStick)  ← 사용자 "이어 검사" 선택
      ↓               (수동 버튼 + CIM Online 양쪽 경로에서 호출)
Step[1] Recipe     : IsResumeMode() → true
                    → ResultClear 건너뛰기
                    → 이전 Raw 파일 FPSpread 재주입 (WAD/ANGLE/VISION)
      ↓
Step[6] Measure    : IsResumeMode() → true → nStartStick = GetResumeStartStick()
                     SetResumeMode(false)              ← 플래그 리셋
      ↓
이후 Steps         : IsResumeMode() → false → 정상 동작
```

플래그가 Measure 진입 후 즉시 리셋되므로 다음 Cycle이나 다음 Glass에 영향 없음.

---

## 6. Result 누적 처리

### 6.1 시나리오별 동작

| 시나리오 | Raw 파일 (디스크) | FPSpread (메모리) | 최종 Report |
|----------|-------------------|-------------------|-------------|
| 정상 검사 | 전체 Stick 존재 | 전체 Stick 데이터 | **완전** |
| Cancel 후 Resume (재시작 없음) | 전체 Stick 존재 | 이전+신규 모두 유지 | **완전** |
| 강제 종료 후 Resume (재시작) | 이전 Stick 존재 | 이전 Raw 재주입 + 신규 추가 | **완전** (2026-07-07 구현) |

### 6.2 Raw 파일 경로 패턴 (모든 측정 모드 공통)

```
{ResultPath}RawData\{InspDate}\{GlassID}\WAD\{InnerID}_1.dat
{ResultPath}RawData\{InspDate}\{GlassID}\ANGLE\{InnerID}.ini
{ResultPath}RawData\{InspDate}\{GlassID}\VISION\{InnerID}.txt
{ResultPath}RawData\{InspDate}\{GlassID}\COLOR\{InnerID}_1.dat
```

- `ResultPath`, `InspDate`, `GlassID` 3개 필드로 모든 측정 모드의 raw 파일 위치 특정 가능
- 측정 모드별 별도 경로 저장 불필요 (하위 폴더 `WAD/ANGLE/VISION/COLOR`로 자동 분기)

### 6.3 이전 Raw 파일 재주입 (2026-07-07 구현)

**파일:** `DoWorkGlassRecipeStep.cpp` — Step[1] Recipe에서 IsResumeMode() 시 실행

```
[Resume 모드 진입]
  → ResultClear 건너뛰기
  → 경로 구성: {ResultPath}RawData\{InspDate}\{GlassID}\
  → PathFileExists 확인 (실패 시 WARN 로그 + skip)
  → nPrevCells = CellsPerStick × (LastStick + 1)
  → WAD\  폴더 스캔 → *.dat 파일 정렬 → nPrevCells개 Updata_Data()
  → ANGLE\ 폴더 스캔 → *.ini 파일 정렬 → nPrevCells개 Updata_Data()
  → VISION\ 폴더 스캔 → *.txt 파일 정렬 → nPrevCells개 Updata_Data()
  → FPSpread에 이전 Stick 데이터 로드 완료
  → 이후 Step[6] Measure에서 신규 Stick 추가 → APD 완전
```

**핵심 설계:**
- 기존 `CResultMgr::Updata_Data()` 공개 API 사용 (DLL 수정 없음)
- `iMeasArea = i + 1` (파일 알파벳 순서 = InnerID 시간순 = 측정 순서)
- 폴더/파일 미존재 시 안전 skip + WARNING 로그
- 파일 수 불일치 시 WARNING 로그 (`nReload < nPrevCells`)

---

## 7. 검증 결과 (Phase 7)

### 7.1 코드 리뷰 — 수정된 이슈

| 심각도 | 파일 | 이슈 | 수정 내용 |
|--------|------|------|-----------|
| **Critical** | `ResumeInfoMgr.cpp` | `WriteToFile()` 실패 감지 불가 | `PathFileExists()` 검증 추가, 실패 시 `false` 반환 |
| **Critical** | `ResumeInfoMgr.cpp` | Atomic Save 실패 시 원본 파일 삭제됨 | 쓰기 실패 시 원본 보존 + 경고 로그 |
| **Critical** | `DlgSubOperate.cpp` | `ValidateResume()` 미호출 | Recipe/TotalStick 불일치 감지 + 사용자 알림 추가 |

### 7.2 코드 리뷰 — 확인된 안전 항목

| 항목 | 판단 |
|------|------|
| ResumeMode 플래그 순서 | Recipe(Step1) → Measure(Step6) 순서 정상 |
| 스레드 안전성 | `SetResumeMode()` → `EngineStart()` 순서적 호출, 경합 없음 |
| 팝업 블로킹 | 엔진 미실행 상태에서 팝업 표시 — 안전 |
| nullptr 체크 | 모든 `pResumeInfoMgr` 사용처에서 nullptr 검사 확인 |
| DST 경계 | 공장 환경에서 DST 전환 가능성 극히 낮음 — 수용 |

### 7.3 빌드 검증

- **빌드**: 성공 (에러 0)
- **한글 손상**: 전체 수정 파일 검증 완료 (damage=0)
- **예외**: `BaseSequenceBody.h` 한글 주석 3줄 손상 (Edit 도구 원인, 코드 동작 무관)

---

## 8. 변경 영향도

| 영역 | 변경 범위 | 위험도 |
|------|-----------|--------|
| Stick 루프 (DoWorkGlassMeasure) | 시작 인덱스 변수화 | 낮음 |
| ResultClear + Raw 재주입 (DoWorkGlassRecipeStep) | if/else 조건 + Raw 파일 스캔/재주입 | 낮음 |
| SequenceEngine | SaveResumeInfo/ClearResumeInfo 호출 추가 | 낮음 |
| DlgSubOperate | CheckAndApplyResumeMode 공통 함수 + 수동/CIM 경로 통합 | 낮음 |
| AuroraMgr/Pool | 포인터 + 생성/소멸 추가 | 낮음 |
| SystemConfigMgr | INI 키 2개 추가 | 낮음 |
| SystemParamShare | 멤버 2개 + getter/setter 추가 | 낮음 |
| BaseSequenceBody | getter 1개 추가 | 낮음 |
| 신규 클래스 1개 (ResumeInfoMgr) | 기존 코드 무관 | 없음 |

**기존 Sequence/Motion Logic 변경: 최소**
- Stick 루프 시작점 1줄 변경
- ResultClear 조건문 1개 추가
- 나머지는 기존 흐름 전/후에 Resume 정보 저장/삭제만 수행

---

## 9. Phase 6: 예외처리 검증 결과 (2026-07-07)

### 9.1 중단 시나리오별 Resume 동작 검증

| 중단 시나리오 | bExis | bRet | UpdateStick | ClearResume | INI 보존 | 이어검사 |
|-------------|-------|------|-------------|-------------|---------|---------|
| 정상 완료 | false | true | O (매 Stick) | O (삭제) | X | X (불필요) |
| 측정 NG (Step fail) | true | false | X (skip) | X | **O** | **O** |
| Motion Alarm | true | false | X (skip) | X | **O** | **O** |
| PLC Disconnect | true | false | X (skip) | X | **O** | **O** |
| Vision Error | true | false | X (skip) | X | **O** | **O** |
| 사용자 Stop 버튼 | true | false | X (skip) | X | **O** | **O** |
| Pause 버튼 | - | - | - | X | **O** | **O** |
| 프로그램 강제 종료 | - | - | - | X | **O** (디스크) | **O** |
| 정전/크래시 | - | - | - | X | **O** (디스크) | **O** |

### 9.2 핵심 보호 메커니즘

**UpdateStickProgress 조건 (DoWorkGlassMeasure.cpp:282):**
```cpp
if (!bExis)  // bExis=true(NG/Alarm/Stop) → skip → 마지막 성공 Stick만 유지
    pResume->UpdateStickProgress((int)i);
```

**ClearResumeInfo 조건 (DoWorkGlassSequenceEngine.cpp:842):**
```cpp
if (bRet == true)  // 전체 정상 완료일 때만 삭제
    m_MgrPool_link.pResumeInfoMgr->ClearResumeInfo();
```

### 9.3 CIM Online 경로 Resume 체크 (2026-07-07 구현)

**수정 내용:**
- `DlgSubOperate.h` — `bool CheckAndApplyResumeMode()` 공통 함수 선언
- `DlgSubOperate.cpp` — Resume 체크 로직을 공통 함수로 추출
  - 수동 버튼 경로 (`OnBnClickedButtonCycle`) → `CheckAndApplyResumeMode()` 호출
  - CIM Online 경로 (`RunStart` → `TO_SERVER_MEAS_START`) → `CheckAndApplyResumeMode()` 호출

```
수동 버튼 경로:                    CIM Online 경로:
OnBnClickedButtonCycle()           RunStart() → TO_SERVER_MEAS_START
  │                                  │
  ▼                                  ▼
CheckAndApplyResumeMode()  ←  공통 함수  →  CheckAndApplyResumeMode()
  │                                  │
  ├─ true → EngineStart()            ├─ true → EngineStart()
  └─ false → return                  └─ false → break
```

### 9.4 완료 항목

- [x] PLC Disconnect 시 Resume 정보 보존 확인
- [x] Motion Alarm 시 Resume 정보 보존 확인
- [x] Vision Error 시 Resume 정보 보존 확인
- [x] CIM Online 경로에 Resume 체크 추가 (`CheckAndApplyResumeMode`)
- [x] 프로그램 재시작 후 이전 Raw 파일 FPSpread 재주입 (APD 완전성)

---

## 10. 향후 개선 과제

- [ ] Resume 전용 다이얼로그 (`CDlgResumePrompt`) — 현재는 `AfxMessageBox` 사용
- [ ] Resume 이력 로그 파일 (별도 `ResumeHistory.log`)
- [ ] 다중 Cycle 모드(`nCycleCount > 1`)에서 Resume 동작 검증
