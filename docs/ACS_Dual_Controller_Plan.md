# ACS Dual Controller 추가 구현 플랜

> 작성일: 2026-07-13
> 상태: Phase 1~9 구현 완료 / Phase 10 미진행 (Config_Motion_ACS.dat ACS[] 매핑 데이터)
> 참조 프로젝트: `D:\IT8.6G\GMI\GMI\IT_8.6G_HBS_20260331`

---

## 1. 현재 상태 (AS-IS)

### 1.1 HMI 단일 컨트롤러 구조

```
CMotorFunction (인터페이스 레이어)
  └── CMotorImpl (가상 기반 클래스)
        └── CMotorControl_ACS (ACS 구현)
              ├── m_hAcs         : HANDLE (단일)
              ├── m_strAddress   : CString (단일 IP)
              ├── m_iPort        : int (단일 Port)
              └── m_cAxis[]      : vector<CAxisType> (축 매핑)
```

**현재 멤버 변수 (MotorControl_ACS.h):**
```cpp
CString   m_strAddress;    // 단일 IP ("10.0.0.100")
int       m_iPort;         // 단일 Port (701)
HANDLE    m_hAcs;          // 단일 핸들
```

**현재 INI (CoreFunction.ini):**
```ini
[MOTOR_CONTROL_MGR]
ADDRESS=10.0.0.100
ADDRESS2=10.0.0.101    ← 이미 추가됨 (아직 코드에서 미사용)
PORT=701
```

**이미 완료된 작업:**
- `MotorControl_ACS.h:5-6` — `#define df_MAX_CTRL 2` 추가됨
- `CoreFunction.ini:7` — `ADDRESS2=10.0.0.101` 추가됨

### 1.2 IT_8.6G_HBS 듀얼 컨트롤러 구조 (참조)

```
CMotorControl_ACS
  ├── m_hAcs[df_MAX_CTRL]          : HANDLE 배열 [2]
  ├── m_strAddress[df_MAX_CTRL]    : CString 배열 [2]
  ├── m_iPort                      : int (공유)
  ├── m_dMOTOR_LOAD[df_MAX_CTRL][64]  : 컨트롤러별 모터 부하
  ├── m_dG_SUB_POS[df_MAX_CTRL][64]   : 컨트롤러별 서브 위치
  └── m_cAxis[]
        ├── m_iAxisMap    : 물리 축 번호 (0~63)
        └── m_iACSMap     : 컨트롤러 인덱스 (0 또는 1)  ← 핵심
```

### 1.3 핵심 차이점 비교

| 항목 | HMI (현재) | IT_8.6G_HBS (목표) |
|------|-----------|-------------------|
| 컨트롤러 핸들 | `m_hAcs` (단일) | `m_hAcs[2]` (배열) |
| IP 주소 | `m_strAddress` (단일) | `m_strAddress[2]` (배열) |
| 축→컨트롤러 매핑 | 없음 | `CAxisType::m_iACSMap` (0 or 1) |
| 접속 | 1회 `acsc_OpenCommEthernetTCP` | 2회 루프 |
| 모션 디스패치 | `m_hAcs` 직접 사용 | `m_hAcs[m_cAxis[iAxis].m_iACSMap]` |
| 전체 축 Reset | `acsc_KillAll(m_hAcs)` | 컨트롤러별 분류 후 각각 호출 |
| GetInput | `GetInput(port, bit)` | `GetInput(port, bit, iAcs_mapping)` |
| CheckInterlock | `CheckInterlock(strAxis)` | `CheckInterlock(strAxis, iAcs_mapping, iFrom)` |

---

## 2. 수정 계획

### Phase 1: CAxisType에 m_iACSMap 추가

**파일:** `CoreFunction/MotorFunction/AxisType.h`

```
현재:
  int   m_iDir;
  int   m_iAxisMap;
  bool  m_bHomeDone;

수정:
  int   m_iDir;
  int   m_iAxisMap;
  int   m_iACSMap;       // ← 추가: 컨트롤러 인덱스 (0 or 1)
  bool  m_bHomeDone;
```

**파일:** `CoreFunction/MotorFunction/AxisType.cpp`
- 생성자에서 `m_iACSMap = 0;` 초기화 추가

### Phase 2: MotorControl_ACS 멤버 배열화

**파일:** `CoreFunction/MotorFunction/MotorCtrl_ACS/MotorControl_ACS.h`

```
현재:
  CString   m_strAddress;
  int       m_iPort;
  HANDLE    m_hAcs;

수정:
  CString   m_strAddress[df_MAX_CTRL];
  int       m_iPort;                        // Port는 공유 (IT_8.6G_HBS 동일)
  HANDLE    m_hAcs[df_MAX_CTRL];
```

### Phase 3: LinkIniFile — ADDRESS2 읽기

**파일:** `MotorControl_ACS.cpp` — `LinkIniFile()`

```
현재:
  m_pIniHelper->Link(bSave, m_strCategory, _T("ADDRESS"), &m_strAddress, (CString)_T("10.0.0.100"));
  m_pIniHelper->Link(bSave, m_strCategory, _T("PORT"),    &m_iPort, 701);

수정:
  m_pIniHelper->Link(bSave, m_strCategory, _T("ADDRESS"),  &m_strAddress[0], (CString)_T("10.0.0.100"));
  m_pIniHelper->Link(bSave, m_strCategory, _T("ADDRESS2"), &m_strAddress[1], (CString)_T("10.0.0.101"));
  m_pIniHelper->Link(bSave, m_strCategory, _T("PORT"),     &m_iPort, 701);
```

### Phase 4: 생성자/소멸자 수정

**생성자:**
```
현재:  m_hAcs = ACSC_INVALID;
수정:  for (int i = 0; i < df_MAX_CTRL; i++) m_hAcs[i] = ACSC_INVALID;
```

**소멸자:**
```
현재:  if (m_hAcs != ACSC_INVALID) acsc_CloseComm(m_hAcs);
수정:  for (int i = 0; i < df_MAX_CTRL; i++)
         if (m_hAcs[i] != ACSC_INVALID) acsc_CloseComm(m_hAcs[i]);
```

### Phase 5: InitDevice — 듀얼 접속

**패턴 (IT_8.6G_HBS:509-582 참조):**
```cpp
int nAcs = 0;
CString strAddressOld;
do {
    CString strAddressCur = m_strAddress[nAcs];
    if (strAddressOld.CompareNoCase(strAddressCur) == 0)
        continue;   // 동일 주소 스킵

    m_hAcs[nAcs] = acsc_OpenCommEthernetTCP(T2A(strAddressCur), m_iPort);

    if (m_hAcs[nAcs] == ACSC_INVALID) { bRet = false; break; }
    bRet = true;

    strAddressOld = strAddressCur;
    nAcs++;
    if (nAcs >= df_MAX_CTRL) break;
} while (true);
```

### Phase 6: LinkDatFile — ACS[] 매핑 파싱

**파일:** `MotorControl_ACS.cpp` — `LinkDatFile()`

Config_Motion_ACS.dat 파일에서 각 축의 ACS 매핑을 읽어야 함:
```
AXIS[0]  ... ACS[0] HW[0]  ...    → m_cAxis[AXIS_X0].m_iACSMap = 0
AXIS[15] ... ACS[1] HW[2]  ...    → m_cAxis[AXIS_Y1].m_iACSMap = 1
```

**추가 파싱 코드:**
```cpp
m_cAxis[j].m_iACSMap = _ttoi(GetDataFromString(strLine, _T("ACS["), _T("]")));
```

### Phase 7: 모션 명령 디스패치 수정 (30+ 메서드)

모든 acsc_* API 호출을 아래 패턴으로 변경:

**단축 디스패치 패턴:**
```cpp
// 현재
if (m_hAcs == ACSC_INVALID) return -1;
int iAxis_mapping = m_cAxis[iAxis].m_iAxisMap;
acsc_ToPoint(m_hAcs, 0, iAxis_mapping, dPos, NULL);

// 수정
int iAcs_mapping = m_cAxis[iAxis].m_iACSMap;
HANDLE hAcs = (iAcs_mapping < df_MAX_CTRL) ? m_hAcs[iAcs_mapping] : ACSC_INVALID;
if (hAcs == ACSC_INVALID) return -1;
int iAxis_mapping = m_cAxis[iAxis].m_iAxisMap;
acsc_ToPoint(hAcs, 0, iAxis_mapping, dPos, NULL);
```

**수정 대상 메서드 목록:**

| # | 메서드 | 유형 | 변경 내용 |
|---|--------|------|-----------|
| 1 | `ResetDevice(int iAxis)` | 전체축/단축 | 전체: 컨트롤러별 분류 후 각각 KillAll/DisableAll/FaultClearM/EnableM |
| 2 | `MoveHome(int iAxis)` | 단축 | iACSMap 디스패치 |
| 3 | `MoveCmdPosA(...)` | 단축 | iACSMap 디스패치 |
| 4 | `MoveCmdPosR(...)` | 단축 | iACSMap 디스패치 |
| 5 | `MoveCmdPosMulti(...)` | 다축 | 다축→컨트롤러별 분류 필요 여부 검토 |
| 6 | `MoveJog(...)` | 단축 | iACSMap 디스패치 |
| 7 | `WaitMotionEnd(...)` | 단축 | iACSMap 디스패치 |
| 8 | `CheckMoveDone(...)` | 단축 | iACSMap 디스패치 |
| 9 | `CheckHomeDone(...)` | 단축 | iACSMap 디스패치 |
| 10 | `CheckInterlock(...)` | 단축 | 시그니처 변경: `iAcs_mapping` 파라미터 추가 |
| 11 | `StopDevice(...)` | 단축 | iACSMap 디스패치 |
| 12 | `KillDevice(...)` | 단축 | iACSMap 디스패치 |
| 13 | `RunBuffer(...)` | 단축 | iACSMap 디스패치 |
| 14 | `StopBuffer(...)` | 단축 | iACSMap 디스패치 |
| 15 | `SetSpeed(...)` | 단축 | iACSMap 디스패치 |
| 16 | `SetSpeedDirect(...)` | 단축 | iACSMap 디스패치 |
| 17 | `SetAmpEnable(...)` | 단축 | iACSMap 디스패치 |
| 18 | `SetWriteBuffer(...)` | 특수 | 컨트롤러 선택 로직 검토 |
| 19 | `GetCurPos(...)` | 단축 | iACSMap 디스패치 |
| 20 | `GetMotorStatus(...)` | 단축 | iACSMap 디스패치 |
| 21 | `GetSpeedVal(...)` | 단축 | iACSMap 디스패치 |
| 22 | `GetSpeed(...)` | 단축 | iACSMap 디스패치 |
| 23 | `GetAmpEnable(...)` | 단축 | iACSMap 디스패치 |
| 24 | `GetHWLimit(...)` | 단축 | iACSMap 디스패치 |
| 25 | `GetMoving(...)` | 단축 | iACSMap 디스패치 |
| 26 | `GetHomeDone(...)` | 단축 | iACSMap 디스패치 |
| 27 | `GetMotorRMSValue(...)` | 단축 | iACSMap 디스패치 |
| 28 | `GetInput(...)` | 특수 | 시그니처 변경: `iAcs_mapping` 파라미터 추가 |
| 29 | `GetFollowingErr(...)` | 단축 | iACSMap 디스패치 |
| 30 | `GetActualVel(...)` | 단축 | iACSMap 디스패치 |
| 31 | `GetHomeVel(...)` | 단축 | iACSMap 디스패치 |
| 32 | `SetMaxSWLimit(...)` | 단축 | iACSMap 디스패치 |
| 33 | `SetMinSWLimit(...)` | 단축 | iACSMap 디스패치 |
| 34 | `MoveToPoint(...)` | 내부 | iACSMap 디스패치 |
| 35 | `CheckHomeDoneLoop(...)` | 내부 | iACSMap 디스패치 |
| 36 | `IsConnected()` | 상태 | 전체 핸들 유효성 체크 |

### Phase 8: MotorImpl 기반 클래스 시그니처 변경

**파일:** `CoreFunction/MotorFunction/MotorImpl.h`

변경이 필요한 virtual 메서드 시그니처:
```cpp
// 현재
virtual long GetInput(int port, int bit);
virtual long CheckInterlock(CString strAxis);

// 수정 (IT_8.6G_HBS 패턴)
virtual long GetInput(int port, int bit, int iAcs_mapping = 0);
virtual long CheckInterlock(CString strAxis, int iAcs_mapping = 0, int iFrom = 0);
```

**주의:** 기본값 `= 0`으로 기존 호출 코드 호환성 유지.

### Phase 9: MotorFunction 레이어 시그니처 전파

**파일:** `CoreFunction/MotorFunction/MotorFunction.h/.cpp`

MotorImpl 시그니처 변경에 맞춰 pass-through 메서드도 수정:
```cpp
// 현재
long GetInput(int port, int bit);

// 수정
long GetInput(int port, int bit, int iAcs_mapping = 0);
```

### Phase 10: Config_Motion_ACS.dat 수정

각 축 정의 라인에 `ACS[n]` 필드 추가:
```
; 현재 (ACS 정보 없음)
AXIS[0] NAME[X0-AXIS] USE[1] HW[0] ...

; 수정 (ACS 매핑 추가)
AXIS[0] NAME[X0-AXIS] USE[1] ACS[0] HW[0] ...
AXIS[15] NAME[Y1-AXIS] USE[1] ACS[1] HW[15] ...
```

---

## 3. 수정 파일 목록

### CoreFunction (DLL 프로젝트)

| # | 파일 | 변경 | 영향도 |
|---|------|------|--------|
| 1 | `MotorFunction/AxisType.h` | `m_iACSMap` 멤버 추가 | 중 |
| 2 | `MotorFunction/AxisType.cpp` | `m_iACSMap = 0` 초기화 | 낮 |
| 3 | `MotorFunction/MotorCtrl_ACS/MotorControl_ACS.h` | 멤버 배열화 | 중 |
| 4 | `MotorFunction/MotorCtrl_ACS/MotorControl_ACS.cpp` | 전체 메서드 수정 (35+) | **높** |
| 5 | `MotorFunction/MotorImpl.h` | GetInput/CheckInterlock 시그니처 변경 | 중 |
| 6 | `MotorFunction/MotorImpl.cpp` | GetInput/CheckInterlock 기본 구현 수정 | 낮 |
| 7 | `MotorFunction/MotorFunction.h` | GetInput/CheckInterlock 시그니처 전파 | 낮 |
| 8 | `MotorFunction/MotorFunction.cpp` | GetInput/CheckInterlock pass-through 수정 | 낮 |

### Config 파일

| # | 파일 | 변경 |
|---|------|------|
| 1 | `Source/Bin/Config/CoreFunction/CoreFunction.ini` | ADDRESS2 이미 추가됨 |
| 2 | `Source/Bin/Config/CoreFunction/MotorConfig/Config_Motion_ACS.dat` | 각 축에 ACS[n] 필드 추가 |

### ALPHA (App 프로젝트) — 영향 파일 (CheckInterlock/GetInput 호출부)

시그니처 변경 시 기본값 `= 0`으로 기존 호출 코드 변경 불필요.
단, `GetInput`을 컨트롤러 지정하여 호출하는 곳이 있다면 파라미터 추가 필요.

---

## 4. 구현 순서 (권장)

```
Phase 1:  AxisType.h/cpp — m_iACSMap 추가
  ↓
Phase 2:  MotorControl_ACS.h — 멤버 배열화
  ↓
Phase 3:  MotorControl_ACS.cpp — LinkIniFile (ADDRESS2 읽기)
  ↓
Phase 4:  MotorControl_ACS.cpp — 생성자/소멸자
  ↓
Phase 5:  MotorControl_ACS.cpp — InitDevice (듀얼 접속)
  ↓
Phase 6:  MotorControl_ACS.cpp — LinkDatFile (ACS[] 파싱)
  ↓
Phase 7:  MotorControl_ACS.cpp — 모션 메서드 디스패치 (35+ 메서드)
  ↓
Phase 8:  MotorImpl.h/cpp — 시그니처 변경 (GetInput, CheckInterlock)
  ↓
Phase 9:  MotorFunction.h/cpp — 시그니처 전파
  ↓
Phase 10: Config_Motion_ACS.dat — ACS[] 매핑 데이터 추가
  ↓
빌드 & 검증
```

---

## 5. 핵심 디스패치 패턴 (IT_8.6G_HBS 기준)

### 5.1 단축 명령 (대부분의 메서드)

```cpp
long CMotorControl_ACS::MoveCmdPosA(int iAxis, double dPos, ...)
{
    // 1. 축 → 컨트롤러 매핑
    int iAcs_mapping = m_cAxis[iAxis].m_iACSMap;

    // 2. 핸들 선택
    HANDLE hAcs;
    if (iAcs_mapping < df_MAX_CTRL)
        hAcs = m_hAcs[iAcs_mapping];
    else
        hAcs = ACSC_INVALID;

    if (hAcs == ACSC_INVALID)
        return -1;

    // 3. 물리 축 번호
    int iAxis_mapping = m_cAxis[iAxis].m_iAxisMap;

    // 4. 해당 컨트롤러에 명령 실행
    acsc_ToPoint(hAcs, 0, iAxis_mapping, dPos, NULL);
}
```

### 5.2 전체 축 명령 (ResetDevice, KillDevice 등)

```cpp
long CMotorControl_ACS::ResetDevice(int iAxis)
{
    if (iAxis < 0)  // 전체 축
    {
        int nAxes[df_MAX_CTRL][200];
        ZeroMemory(nAxes, sizeof(nAxes));
        int ihAcs[df_MAX_CTRL] = {0};

        // 1. 축을 컨트롤러별로 분류
        for (int i = 0; i < df_USE_MAX_AXIS; i++)
        {
            if (!m_cAxis[i].m_setting.bUse) continue;
            int iMap = m_cAxis[i].m_iACSMap;
            nAxes[iMap][ihAcs[iMap]++] = m_cAxis[i].m_iAxisMap;
        }
        for (int i = 0; i < df_MAX_CTRL; i++)
            nAxes[i][ihAcs[i]] = -1;  // 터미네이터

        // 2. 각 컨트롤러에 배치 명령
        for (int i = 0; i < df_MAX_CTRL; i++)
        {
            if (m_hAcs[i] == ACSC_INVALID) continue;
            acsc_KillAll(m_hAcs[i], NULL);
            acsc_DisableAll(m_hAcs[i], NULL);
            acsc_FaultClearM(m_hAcs[i], nAxes[i], NULL);
            acsc_EnableM(m_hAcs[i], nAxes[i], NULL);
        }
    }
    else  // 단축 → 5.1 패턴
    { ... }
}
```

### 5.3 IsConnected 수정

```cpp
bool CMotorControl_ACS::IsConnected()
{
    // 최소 1개 컨트롤러 연결 시 true (또는 정책에 따라 전체 연결 필요)
    return m_bBdInstalled;
}
```

---

## 6. 위험 요소 및 주의사항

| 위험 | 설명 | 대응 |
|------|------|------|
| **acsc_* 핸들 혼용** | 잘못된 컨트롤러 핸들에 명령 전송 시 다른 장비 축이 동작 | 모든 메서드에서 iACSMap 디스패치 패턴 일관 적용 |
| **부분 연결 실패** | 컨트롤러 1개만 연결되고 다른 1개 실패 시 동작 정의 | IT_8.6G_HBS: 하나라도 실패 시 전체 실패 (`if (!bRet) break`) |
| **Config_Motion_ACS.dat 미수정** | ACS[] 필드 없으면 기본값 0으로 전체 축이 컨트롤러 0에 매핑 | 하드웨어 구성 확정 후 dat 파일에 ACS 매핑 반드시 추가 |
| **MoveCmdPosMulti 다축 명령** | 여러 축이 서로 다른 컨트롤러에 있을 때 동시 이동 처리 | 컨트롤러별 분류 후 각각 호출 또는 동일 컨트롤러 축만 Multi 허용 |
| **CAxisType 구조체 크기 변경** | m_iACSMap 추가로 메모리 레이아웃 변경 | 공유 메모리 사용 여부 확인 (CoreFunction DLL 내부만 사용 → 안전) |
| **MotorImpl 시그니처 변경** | GetInput/CheckInterlock 파라미터 추가 | 기본값 `= 0`으로 기존 호출 호환성 유지 |
| **EUC-KR 인코딩** | CoreFunction 파일에 한글 주석 존재 | Perl raw byte 편집 사용 |

---

## 7. 검증 계획

### 7.1 단일 컨트롤러 호환성 (OUT_OF_SERVICE=1 상태)
- ACS[0]에만 모든 축 매핑 → 기존 동작과 동일해야 함
- ADDRESS2가 ADDRESS와 동일한 IP면 중복 연결 스킵 (`strAddressOld` 체크)

### 7.2 듀얼 컨트롤러 동작
- 시뮬레이션 모드에서 InitDevice 2회 호출 확인
- 축별 iACSMap 값에 따라 올바른 핸들 선택 확인
- ResetDevice(-1) 시 양 컨트롤러 모두 초기화 확인

### 7.3 빌드 검증
- CoreFunction DLL 빌드 (에러 0)
- ALPHA EXE 빌드 (에러 0)
- 시그니처 변경에 따른 호출부 컴파일 에러 없음 확인
