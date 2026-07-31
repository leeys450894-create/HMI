# OCP HMI - PLC 매핑 구조 및 동작 시퀀스 분석서

## 1. 시스템 개요

| 항목 | 내용 |
|------|------|
| 프로젝트명 | ALPHA (HBS_OS) |
| 용도 | 삼성 디스플레이 Glass 검사 자동화 장비 HMI |
| 프레임워크 | C++ MFC (Microsoft Foundation Classes) |
| PLC 통신 | CC-Link Industrial Ethernet |
| 모션 컨트롤러 | ACS SPiiPlus (Ethernet TCP/IP, 90축) |
| 솔루션 경로 | `Source/ALPHA/ALPHA.sln` |

---

## 2. PLC 매핑 구조

### 2.1 통신 아키텍처

```
+------------------+     CC-Link IE      +------------------+
|                  | <================>  |                  |
|   HMI PC (ALPHA) |                     |   Safety PLC     |
|                  |                     |                  |
+------------------+                     +------------------+
        |
        |  Ethernet TCP/IP
        |
+------------------+
| ACS SPiiPlus     |
| 모션 컨트롤러    |
| (90축)           |
+------------------+
```

### 2.2 I/O 매핑 데이터 구조

**핵심 파일:** `Source/ALPHA/Manager/IOMapAddressDefine.h`

```cpp
struct SIO_MAP_INFOR
{
    CString strHardwareIOAddress;    // 하드웨어 I/O 주소 (예: "X.0.1")
    CString strRealMemoryAddress;    // 실제 메모리 주소 매핑
    CString strName_eng;             // 영문 명칭
    CString strName_kor;             // 한글 명칭
};
```

### 2.3 CC-Link I/O 비트 매핑

#### 2.3.1 Write 비트 (PC -> PLC 출력: 128비트)

| 비트 번호 | 매핑 이름 | 설명 |
|-----------|-----------|------|
| W_BIT_017 | PC ONLINE STATUS | PC 온라인 상태 신호 |
| W_BIT_019 | INTERLOCK S.R ON | 온도 과열 알람 인터록 |
| W_BIT_021 | POL CCD POWER ON 01 | 편광 CCD 카메라 전원 ON |
| W_BIT_024 | DOOR UNLOCK | 도어 잠금 해제 |
| W_BIT_033 | 288M WORK SLIDE REV | 288M 워크 슬라이드 동작 |
| W_BIT_034 | SIDE VIEW WORK SLIDE REV | 사이드 뷰 워크 슬라이드 |
| W_BIT_036 | RBT HANDLER VACUUM ON | 로봇 핸들러 진공 ON |
| W_BIT_037 | BUZZER ON | 부저 ON |
| W_BIT_045 | MUTING SIGNAL ON | 뮤팅 신호 ON |
| W_BIT_046 | LASER ON SIGN ON | 레이저 ON 표시등 |
| W_BIT_047 | SELECT KEY LOCK | 키 선택 잠금 |

#### 2.3.2 Read 비트 (PLC -> PC 입력: 128비트)

| 비트 번호 | 매핑 이름 | 설명 |
|-----------|-----------|------|
| R_BIT_017 | 288M WORK SLIDE READY | 288M 워크 슬라이드 준비 위치 |
| R_BIT_019 | SIDE VIEW WORK SLIDE READY | 사이드 뷰 슬라이드 준비 위치 |
| R_BIT_022 | LOADING BUFFER GLASS CHECK 01 | 로딩 버퍼 글래스 감지 |
| R_BIT_025 | CDA 01 PRESSURE NORMAL | CDA 압력 정상 |
| R_BIT_026 | HANDLER VACUUM ON | 핸들러 진공 ON 상태 |
| R_BIT_033 | SMOKE DETECT 01 | 화재 감지 (전면 E-BOX) |
| R_BIT_035 | TEMP OVER WARNING | 전면 E-BOX 온도 경고 (+3도) |
| R_BIT_036 | TEMP OVER ALARM | 전면 E-BOX 온도 알람 (40도) |
| R_BIT_039 | TEMP ALARM RESET SWITCH | 온도 알람 리셋 스위치 |
| R_BIT_041 | EMERGENCY STATUS CHECK 01 | 비상 상태 확인 |
| R_BIT_043 | CPU ERROR STATUS CHECK 01 | CPU 에러 상태 확인 |
| R_BIT_045 | LOADING DOOR CLOSED | 로딩 도어 닫힘 |
| R_BIT_046 | MUTING SWITCH PUSH | 뮤팅 스위치 눌림 |
| R_BIT_047 | LIGHT CURTAIN MUTING STATUS | 라이트 커튼 뮤팅 상태 |
| R_BIT_048 | LIGHT CURTAIN DETECTED | 라이트 커튼 감지 |
| R_BIT_049~052 | DOOR 01~04 OPEN | 도어 열림 (전면/우/후면/좌) |
| R_BIT_053~056 | DOOR 01~04 UNLOCK | 도어 잠금해제 (전면/우/후면/좌) |
| R_BIT_057~061 | EMS SWITCH 01~05 PUSH | 비상정지 스위치 (OP/우/후/좌/E-BOX) |
| R_BIT_063 | DOOR 05 OPEN (BOTTOM) | 하부 도어 열림 |
| R_BIT_065~068 | ELECTRIC BOX FAN 01~04 ON | 전기함 팬 동작 |
| R_BIT_069~076 | PC BOX FAN 01~08 ON | PC BOX 팬 동작 |
| R_BIT_079 | E-BOX DOOR OPEN | E-BOX 도어 열림 |
| R_BIT_081 | UPS1 MAIN CIRCUIT BREAKER TRIP | UPS1 차단기 트립 |
| R_BIT_083~084 | SR 101/102 ON | 장비 모션 SR |
| R_BIT_085~086 | SYSTEM MC 01/02 ON | 시스템 MC ON |
| R_BIT_087 | SAFETY PLC CP ON | 안전 PLC CP ON |
| R_BIT_090~091 | SR 81/82 ON | OCT 레이저 SR |
| R_BIT_093 | INTERLOCK S.R ON | 온도 알람 인터록 |
| R_BIT_094 | SOUND OFF SWITCH PUSH | 사운드 OFF 스위치 |
| R_BIT_095 | SELECT SWITCH AUTO MODE | AUTO 모드 선택 스위치 |

### 2.4 CC-Link Word 매핑

| 구분 | Word 수 | 용도 |
|------|---------|------|
| Write Word | 16개 (W_WORD_000~015) | PC -> PLC 데이터 전송 |
| Read Word | 16개 (R_WORD_000~015) | PLC -> PC 데이터 수신 |
| System Write Word | 4개 | 시스템 예약 (미사용) |
| System Read Word | R_WORD_000: CDA 01, R_WORD_001: RBT HANDLER VACUUM | 아날로그 값 수신 |

**Word 값 비트마스크:**
```
WORD_VALUE_00 = 0x0001 (Bit 0)  ~  WORD_VALUE_15 = 0x8000 (Bit 15)
WORD_VALUE_16 = 0x00010000       ~  WORD_VALUE_20 = 0x00100000
```

### 2.5 PLC M비트 상태 매핑 (plc_MBIT_t)

| M비트 주소 | 이름 | 설명 |
|-----------|------|------|
| 4000 | PLC_BIT_STATE_READY | 장비 준비 상태 |
| 4001 | PLC_BIT_STATE_LOAD | 장비 로딩 상태 |
| 4002 | PLC_BIT_STATE_MEASURE | 장비 측정 상태 |
| 4008 | PLC_BIT_STATE_ALARM | 알람 상태 |
| 4009 | PLC_BIT_STATE_WARN | 경고 상태 |
| 4500 | PLC_BIT_INITIALIZE | 초기화 명령 |
| 4005 | PLC_BIT_INITIALIZE_COMPLETE | 초기화 완료 |
| 4503 | PLC_BIT_MEAS2READY_START | 측정->준비 전환 |
| 4504 | PLC_BIT_READY2MEAS_START | 준비->측정 전환 |
| 4509 | PLC_BIT_ALRAM_RESET_STATUS | 알람 리셋 상태 |
| 4510 | PLC_BIT_DOOR_UNLOCK | 도어 잠금 해제 |
| 4260 | PLC_BIT_RUN_TEACH_SWITCH | Teach 모드 스위치 |
| 4300 | PLC_BIT_HAND_GRIP | Hand Grip 스위치 |

### 2.6 PLC Import 비트 (PLC -> PC) 주요 영역

| 주소 범위 | 카테고리 | 주요 항목 |
|-----------|---------|-----------|
| 4010~4018 | EMO 스위치 | 비상정지 9개 위치 (OP/RIGHT/BACK/LEFT/E-BOX 등) |
| 4020~4030 | 도어 센서 | 도어 열림/잠금해제 |
| 4050~4054 | 윈도우 센서 | EFU/도어 닫힘 확인 |
| 4070~4071 | 전원 차단기 | GPS/UPS 메인 차단기 트립 |
| 4080~4087 | MC 접점 | EQ 모션/X-Ray 이오나이저/EFU MC |
| 4090~4096 | 글래스 감지 | 리프트핀/샘플 척 글래스 확인 |
| 4100~4103 | 엣지 감지 | 엣지 글래스 확인 4점 |
| 4115~4118 | X-Ray 이오나이저 | POWER/INTERLOCK/EMISSION/TUBE |
| 4120~4131 | CDA/N2/Vacuum | 압력 정상 확인 |
| 4140~4153 | 진공/척 | 진공 압력/샘플 척 진공 |
| 4160~4167 | 리프트 핀 | UP/DOWN 4쌍 |
| 4210~4213 | 온도 감지 | EL BOX/CHAMBER/PC BOX 온도 초과 |
| 4220~4221 | 화재 감지 | 연기 감지 2점 |
| 4230~4245 | 팬 상태 | E-BOX 팬 16개 |
| 4250~4253 | 아이솔레이터 | 4코너 위치 정상 |
| 4350~4357 | LED Auto Change | White/UV LED FWD/REV (Head 1,2) |
| 4430~4465 | 클램프 | Front/Right/Back/Left 클램프 ON/OFF |
| 4495~4498 | 로더 위치 | LDX/LDZ 로딩/접촉 위치 |

### 2.7 PLC Export 비트 (PC -> PLC) 주요 영역

| 주소 범위 | 카테고리 | 주요 항목 |
|-----------|---------|-----------|
| 4509 | 알람 리셋 | PLC 알람 리셋 |
| 4510 | 도어 제어 | 도어 잠금 해제 |
| 4525~4526 | X-Ray | 이오나이저 원격 ON/인터록 |
| 4530~4533 | 리프트 핀 | UP/DOWN 명령 (좌/우) |
| 4540~4541 | 게이트 밸브 | 열림/닫힘 (TM 사이드) |
| 4570~4571 | MCM | 접촉 01/02 ON |
| 4590 | 키 잠금 | SELECT KEY LOCK |
| 4630~4633 | CCD 카메라 | Align CCD / Vision Head CCD ON |
| 4720~4721 | 멀티탭 | 전원 ON |
| 4775~4788 | LED Auto Change | White/UV LED FWD/REV 명령 |
| 4791~4798 | 클램프 제어 | Front/Right/Back/Left Fold/Unfold |
| 4810 | 블로잉 | 샘플 척 블로잉 |
| 4830~4835 | 진공 제어 | 척 진공 ON/OFF 3채널 |
| 4860~4880 | 밸브 제어 | CDA/N2/Exhaust 앵글 밸브 |
| 4920 | 부저 | 부저 ON |
| 4925~4929 | 시그널 타워 | MELODY/RED/YELLOW/GREEN/BLUE |

---

## 3. I/O 보드 직접 매핑 (IO_BOARD)

### 3.1 입력 (ADAM-3951 + ADAM-3937)

| 입력 번호 | 신호명 | 용도 |
|-----------|--------|------|
| 000 | UPS MAIN CIRCUIT BREAKER TRIP | UPS 차단기 트립 감시 |
| 001 | UPS MAIN CIRCUIT BREAKER ON | UPS 차단기 ON 상태 |
| 002 | SAFETY PLC CP ON | 안전 PLC 전원 확인 |
| 003 | MC CP ON | MC 전원 확인 |
| 004 | SR 10 ON (EQ MOTION MC POWER) | 장비 모션 MC 전원 |
| 005 | Vacuum Off | 진공 OFF 상태 |
| 006 | EFU ALARM | EFU 알람 |
| 010 | LIGHT CURTAIN MUTING STATUS | 라이트 커튼 뮤팅 |
| 011 | LIGHT CURTAIN DETECTED | 라이트 커튼 감지 |
| 016~017 | DOOR 01/02 OPEN | 좌/우 도어 열림 |
| 022~025 | EMO SWITCH 01~04 | 비상정지 4개소 |
| 026~027 | SMOKE DETECT 01~02 | 연기 감지 (EL BOX / PC BOX) |
| 100 | INTERLOCK SR 150 ON | 온도 알람 인터록 |
| 109~110 | VACUUM 01~02 NORMAL | 진공 압력 정상 (스테이지/정렬) |
| 111 | CDA 01 NORMAL | CDA 블로잉 압력 정상 |
| 114~115 | TWO-HAND SWITCH | 양손 스위치 좌/우 |

### 3.2 출력 (ADAM-3951)

| 출력 번호 | 신호명 | 용도 |
|-----------|--------|------|
| 000 | BUZZER ON | 부저 |
| 001~004 | VOICE BUZZER CH1~4 | 음성 부저 채널 |
| 005 | SELECT KEY LOCK | 키 잠금 |
| 007~008 | CCD 01/02 ON | Vision Head 1/2 |
| 011 | INTERLOCK SR 150 ON | 온도 알람 인터록 출력 |
| 012 | DOOR UNLOCK | 도어 잠금 해제 |
| 016~017 | ACS MOTION INTERLOCK 1/2 | 모션 인터록 |
| 021~024 | SIGNAL TOWER | RED/YELLOW/GREEN/BUZZER |
| 028~029 | SAMPLE PLATE VACUUM 01/02 | 스테이지/정렬 진공 |
| 030~031 | SAMPLE PLATE BLOWING 01/02 | 스테이지/정렬 블로잉 |

---

## 4. I/O 제어 매니저 (CIOMapControlMgr)

**파일:** `Source/ALPHA/Manager/IOMapControlMgr.h`

### 4.1 주요 인터페이스

```cpp
// 장치 초기화
bool InitDevice(short index);
bool IsConnected() const;
bool IsInitialized() const;

// 비트 제어 (개별 ON/OFF)
bool SetMemoryBitToDevice(int index, bool value);
bool SetMemoryBitToDevice(CString address, bool value);

// 워드 제어
bool SetMemoryWordToDevice(int index, float value);
bool SetMemoryWordToDevice(CString address, float value);

// 다중 비트/워드 제어
bool SetMemoryNBitToDevice(int index, long value, long lSize);
bool SetMemoryNWordToDevice(int index, float* value, long lSize);

// 동작 완료 대기
bool CheckActionDone(int index, DWORD dwTimeout, bool value = true);

// 시그널 타워 제어
bool SetSinalTowerAlarm();   // 알람 (적색)
bool SetSinalTowerIdle();    // 대기 (녹색)
bool SetSinalTowerRun();     // 운전 (녹색 점멸)
bool SetSinalTowerWait();    // 대기 (황색)

// 스테이지 진공/핸들링
bool SetPickertoStage();     // 피커 -> 스테이지 이송
bool SetStagetoPicker();     // 스테이지 -> 피커 이송
bool SetStageVaccumOn();     // 스테이지 진공 ON
bool SetStageVaccumOff();    // 스테이지 진공 OFF
```

### 4.2 I/O 주소 CSV 파일 로드

```cpp
bool ReadAdressFileCSV(CString strFilename);
```
외부 CSV 파일에서 I/O 주소 테이블을 동적으로 로드하여 매핑 구성.

---

## 5. 장비 상태 머신 (Equipment State Machine)

**파일:** `Source/ALPHA/FunctionModule/FunctionModuleEQState.h`

### 5.1 장비 상태 정의

```
+----------+     로딩명령     +----------+     측정시작     +----------+
|          | ------------->  |          | ------------->  |          |
|  Ready   |                 |   Load   |                |   Meas   |
|  (준비)  | <--------------  |  (로딩)  | <--------------  |  (측정)  |
|          |     언로드완료    |          |     측정완료     |          |
+----------+                 +----------+                +----------+
      |                           |                           |
      |         에러 발생          |         에러 발생          |
      +---------->+<--------------+---------->+<--------------+
                  |                           |
             +----------+                     |
             |          | <-------------------+
             | Abnormal |
             |  (이상)  |
             +----------+
```

### 5.2 센서 상태 확인 함수

| 카테고리 | 함수명 | 용도 |
|---------|--------|------|
| **안전** | IsSafetySensorOn() | 전체 안전 센서 |
| | IsSafetySensorOnStage() | 스테이지 안전 센서 |
| | IsSafetySensorOnHead() | 헤드 안전 센서 |
| | IsSafetySensorOnProbe(unit) | 프로브 안전 센서 |
| **글래스** | IsGlassSensorOn() | 글래스 존재 감지 |
| **리프트핀** | IsLiftpinSensorUp() | 리프트핀 상승 |
| | IsLiftpinSensorDown() | 리프트핀 하강 |
| **진공** | IsVacuumSensorOn() | 진공 ON 확인 |
| | IsVacuumSensorOff() | 진공 OFF 확인 |
| **에어** | IsAirSensorOn() | 에어 ON 확인 |
| **클램프** | IsStageClampFold() | 스테이지 클램프 접힘 |
| | IsStageClampUnfold() | 스테이지 클램프 펼침 |
| **로더** | IsLoaderXLoadSensorOn() | 로더 X축 로드 위치 |
| | IsLoaderXMeasSensorOn() | 로더 X축 측정 위치 |
| | IsLoaderZLoadSensorOn() | 로더 Z축 로드 위치 |
| | IsLoaderZMeasSensorOn() | 로더 Z축 측정 위치 |
| **포지션** | IsPosion0_SensorOn() | 0도 위치 |
| | IsPosion90_SensorOn() | 90도 위치 |
| | IsPosion180_SensorOn() | 180도 위치 |
| | IsPosion270_SensorOn() | 270도 위치 |
| **프로브** | IsProbeContactSensorOn(unit) | 프로브 접촉 |
| | IsProbeExistSensorOn(unit) | 프로브 존재 |
| | IsProbeZerolockSensorOn(unit) | 프로브 제로락 |
| | IsProbeClampFold(unit) | 프로브 클램프 접힘 |
| **인터록** | IsEQSensorClose() | 장비 센서 닫힘 |
| | IsShutterSensorClose() | 셔터 센서 닫힘 |
| | IsDoorSensorClose() | 도어 센서 닫힘 |
| **Auto Change** | IsACZxisUp() | AC Z축 상승 |
| | IsACZxisDown() | AC Z축 하강 |
| | IsACShutterOpen() | PACC 셔터 열림 |
| | IsACProbeExistSensorOn() | AC 암 프로브 존재 |

### 5.3 인터록 확인

```cpp
int GetInterlockMoveX();    // X축 이동 인터록
int GetInterlockMoveY();    // Y축 이동 인터록
int GetInterlockMoveZ();    // Z축 이동 인터록
int GetInterlockMoveLDX();  // 로더 X 인터록
int GetInterlockMoveLDY();  // 로더 Y 인터록
int GetInterlockMoveLDR();  // 로더 R 인터록
int GetInterlockMoveLDZ();  // 로더 Z 인터록
int GetinterlockIOLP();     // 리프트핀 I/O 인터록
int GetinterlockIOClamp();  // 클램프 I/O 인터록
int GetinterlockIOVacuum(); // 진공 I/O 인터록
int GetinterlockIOAir();    // 에어 I/O 인터록
```

---

## 6. 동작 시퀀스

### 6.1 시스템 초기화 시퀀스 (22단계)

**파일:** `Source/ALPHA/DoWork/SystemInitializeStep.cpp`

```
Step 0:  플래그 초기화
Step 1:  InitModule (모션 컨트롤러 연결) + InitDevice (PLC I/O 연결)
Step 2:  CheckInterlock (안전 인터록 확인)
Step 3:  InitIO (Fixture/PLC 초기 설정)
Step 4~15: 축별 원점복귀 (Homing) - 12축 순차 실행
Step 16: MotionComplete (모션 완료 확인)
Step 17: InitIO (I/O 재확인)
Step 18: Safety Move (안전 위치 이동)
Step 19: InitComplete (전체 초기화 검증)
Step 20: ProbeAlign (프로브 정렬)
Step 21: Safety Move (안전 위치 복귀)
Step 22: Idle (대기 상태 진입)
```

### 6.2 글래스 자동 처리 시퀀스 (AUTO 모드)

**파일 위치:** `Source/ALPHA/DoWork/`

```
[시작]
   |
   v
DoWorkGlassReady -----> 초기 설정, 장비 상태 확인
   |
   v
DoWorkGlassAdsorption -> 글래스 로딩, 리프트핀 UP, 진공 흡착, 클램프
   |
   v
DoWorkGlassRecipe -----> 레시피 파라미터 적용, 장비별 레시피 전송
   |
   v
DoWorkGlassSWAlign ----> SW 정렬 (비전 카메라 기반 글래스 위치 보정)
   |
   v
DoWorkGlassPreMeas ----> 사전 측정 검증 (Pin-to-Pin, OST, R-TEST)
   |
   v
DoWorkGlassAGING ------> 에이징 테스트 실행
   |
   v
DoWorkGlassMeasure ----> 본 측정 (161KB - 가장 큰 시퀀스 파일)
   |                      - 다중 측정 영역
   |                      - Stick 위치 관리
   |                      - Cell ID 생성
   |                      - Inner ID 생성
   v
DoWorkGlassResult -----> 결과 처리, 데이터 저장
   |
   v
DoWorkGlassUnload -----> 진공 OFF, 클램프 해제, 리프트핀 UP, 글래스 반출
   |
   v
[완료 -> 다음 글래스 or 대기]
```

### 6.3 시퀀스 엔진 구조

**파일:** `Source/ALPHA/DoWork/DoWorkGlassSequenceEngine.h`

```cpp
// 엔진 제어
void EngineStart();    // 시퀀스 시작
void EngineResume();   // 시퀀스 재개
void EnginePause();    // 시퀀스 일시정지
void EngineStop();     // 시퀀스 정지

// 개별 아이템 제어
bool ItemStart(CString strType, bool bCompleteCheck);
void ItemStopAndSkip(CString strType);
void ItemStopAndExit(CString strType);
void ItemPause(int index);
void ItemResume(int index);

// 사용자 제어
void UserResumeState();
void UserPauseState();
void UserStopState(bool bSet);
```

### 6.4 수동 조작 스텝 (MANUAL 모드)

**파일:** `Source/ALPHA/DlgSubManual_Meas.h`

| 카테고리 | StepID | 설명 |
|---------|--------|------|
| **레시피** | RECIPE_OPEN | 레시피 열기 |
| | RECIPE_OPEN_PG | PG 레시피 열기 |
| | RECIPE_OPEN_AGING | 에이징 레시피 열기 |
| **글래스 로딩** | GLASS_GET_01~09 | 글래스 로딩 (9단계) |
| **글래스 언로딩** | GLASS_PUT_01~08 | 글래스 언로딩 (8단계) |
| **PG 제어** | PG_CMD_01 | PG Turn On |
| | PG_CMD_02 | PG Pattern Next |
| | PG_CMD_03 | PG Pattern Prev |
| | PG_CMD_04 | PG Turn Off |
| | PG_CMD_05 | PG On -> Next R,G,B -> Off |
| **정렬** | GLASS_ALIGN | 글래스 정렬 |
| | PROBE_ALIGN | 프로브 정렬 |
| **이동** | MOVE_STICK / STICK_PRE / STICK_NEXT | Stick 이동 |
| | MOVE_CELL / CELL_NEXT | Cell 이동 |
| | MOVE_PROBE | 프로브 이동 |
| **프로브** | PROBE_CONTACT | 프로브 접촉 |
| | PROBE_UNCONTACT | 프로브 비접촉 |
| | PROBE_CONTACT_ALL | 전체 프로브 접촉 |
| | PROBE_UNCONTACT_ALL | 전체 프로브 비접촉 |
| **사전 테스트** | PRE_TEST_01 | Pin-to-Pin 테스트 |
| | PRE_TEST_02 | OST (Open/Short Test) |
| | PRE_TEST_03 | R-TEST (저항 테스트) |
| | PRE_TEST_04 | 에이징 |
| **측정 설정** | MEAS_SET_WAD | WAD 측정 설정 |
| | MEAS_SET_VISION | Vision 측정 설정 |
| | MEAS_SET_COLOR | Color 측정 설정 |
| | MEAS_SET_ANGLE | Angle 측정 설정 |
| | MEAS_SET_TSP | TSP 측정 설정 |
| | MEAS_SET_LCR | LCR 측정 설정 |
| **레시피 변경** | RECIPE_CHANGE_WAD~LCR | 장비별 레시피 변경 |
| **측정 실행** | MEAS_TEST_WAD~LCR | 장비별 측정 실행 |
| **수동 명령** | MANUAL_SHUTTER | 셔터 제어 |
| | MANUAL_CMD_01~13 | 기타 수동 명령 |

---

## 7. TCP/IP 통신 장비 네트워크

**설정 파일:** `Source/ALPHA/ConfigurationFile/Network.ini`

### 7.1 장비 연결 구성

| Index | 장비명 | IP 주소 | Port | 용도 |
|-------|--------|---------|------|------|
| 0 | Vision 1 | 127.0.0.1 | 7001 | 비전 검사 Head 1 |
| 1 | Vision 2 | 192.168.10.11 | 7001 | 비전 검사 Head 2 |
| 2 | Color 1 | 192.168.10.12 | 6001 | 색상 센서 1 |
| 3 | Color 2 | 192.168.10.13 | 6003 | 색상 센서 2 |
| 4 | Angle 1 | 192.168.10.14 | 6001 | 각도 센서 1 |
| 5 | Angle 2 | 192.168.10.15 | 6001 | 각도 센서 2 |
| 6 | WAD | 192.168.10.17 | 2002 | WAD 측정 시스템 |
| 7 | PG | 192.168.10.17 | 7003 | 패턴 제너레이터 |
| 8 | LCR | 192.168.10.18 | 2002 | LCR 미터 |
| 9 | AGING | 192.168.10.17 | 4500 | 에이징 장치 |
| 10 | CIM | 127.0.0.1 | 2050 | CIM 인터페이스 |
| 11 | SYSTIME | 127.0.0.1 | 2051 | 시스템 시간 동기화 |
| 12 | VISMOTION | 127.0.0.1 | 2052 | 비전 모션 |
| 13~18 | TSP1~TSP6 | 127.0.0.1 | 2002~2007 | TSP 측정기 (6대) |

### 7.2 통신 프로토콜 이벤트

```cpp
// 주요 수신 이벤트 (CommTcpIpDefine.h)
TIME_SYNC_OK/NG         // 시간 동기화
SET_PARAMETER_OK/NG      // 파라미터 설정
GET_PARAMETER_OK/NG      // 파라미터 조회
RECIPE_CHANGE_OK/NG      // 레시피 변경
INSPECTION_READY_OK/NG   // 검사 준비
INSP_START_OK/NG         // 검사 시작
INSP_END_OK/NG           // 검사 종료
INSP_COMPLETE_OK/NG      // 검사 완료
```

---

## 8. 알람 모니터링 시스템

**파일 위치:** `Source/ALPHA/Alarm/`

### 8.1 알람 모니터 목록 (24종+)

| 모니터 | 감시 대상 | 주기 |
|--------|----------|------|
| AlarmMonitorToDoorOpen | 도어 열림 감지 | 200ms |
| AlarmMonitorToDoorOpenState | 도어 상태 추적 | 200ms |
| AlarmMonitorToDoorVoice | 도어 음성 경보 | 200ms |
| AlarmMonitorToEmergency | 비상정지 버튼 | 200ms |
| AlarmMonitorToEmergencyButton | 비상정지 버튼 상태 | 200ms |
| AlarmMonitorToLampTime | 램프 사용 시간 | - |
| AlarmMonitorToMcOFF | 모션 컨트롤러 오프라인 | 실시간 |
| AlarmMonitorToMcOFFState | MC 상태 추적 | 실시간 |
| AlarmMonitorToSafetyPLC | Safety PLC 이상 | 실시간 |
| AlarmMonitorToSensor | 일반 센서 | 50~200ms |
| AlarmMonitorToSensorCurtain | 안전 커튼 | 50ms |
| AlarmMonitorToSensorFAN | 팬 동작 | 200ms |
| AlarmMonitorToSensorHandSwitch | 핸드/모드 스위치 | 200ms |
| AlarmMonitorToSensorIsolater | 아이솔레이터 | 200ms |
| AlarmMonitorToSensorPressure | 에어 압력 | 200ms |
| AlarmMonitorToSensorSmoke | 연기 감지 | 200ms |
| AlarmMonitorToSensorTemperature | 온도 감시 (40도) | 200ms |
| AlarmMonitorToSensorTemperatureLow | 저온 감시 | 200ms |
| AlarmMonitorToSensorVacSwitch | 진공 센서 | 200ms |
| AlarmMonitorToTeachKEY | Teach 키 | 200ms |
| AlarmMonitorToTeachKEYLock | 키 잠금 상태 | 200ms |

### 8.2 알람 설정 (Config.ini)

```ini
USETEACH=1         # Teach 모드 알람 (PLC OFF = 알람)
USETEMP=1          # 온도 알람 (PLC ON = 알람)
USEAREAS=0         # 분진/영역 센서
USEVACUUM=0        # 진공 알람
USEAIR=1           # 에어 압력 알람
USEDOOR=1          # 도어 열림 알람 (PLC ON = 알람)
USEEMO=1           # 비상정지 버튼
USESMOKE_DETECT=0  # 연기 감지
TEMPOVER=40        # 온도 임계값 (40도)
O2OVER=20          # O2 임계값 (20%)
```

---

## 9. 모션 컨트롤러 (ACS SPiiPlus)

### 9.1 사양

| 항목 | 값 |
|------|------|
| 라이브러리 | ACSC v7.1.1.0 |
| 통신 | Ethernet TCP/IP |
| 총 축 수 | 90축 (df_USE_MAX_AXIS = 100) |

### 9.2 주요 제어 함수

```cpp
bool ResetDevice(int iAxis);
bool MoveHome(int iAxis);
bool MoveCmdPosA(int iAxis, double dPos, bool bBlockMode, int iSpeed);   // 절대 이동
bool MoveCmdPosR(int iAxis, double dPos, bool bBlockMode, int iSpeed);   // 상대 이동
bool MoveCmdPosMulti(stMotorPosition*, int iAxis, int iSpeed);           // 다축 동시 이동
bool WaitMotionEnd(int iAxis, DWORD dwTimeout);                          // 이동 완료 대기
bool CheckMoveDone(int iAxis, DWORD dwTimeout);                         // 이동 완료 확인
bool CheckInterlock(CString strAxis);                                     // 인터록 확인
```

### 9.3 주요 모션 포지션 (Position.ini)

| 카테고리 | 파라미터 | 값 |
|---------|---------|------|
| SAFETY | SafetyStageX | 1390.0 |
| | SafetyHeadX1/X2 | 560 / -550 |
| | SafetyHeadY1/Y2 | 560.0 / -560.0 |
| LOADER_Z | UP_POSITION | 77.0 |
| | MIDDLE_POSITION | 12.0 |
| | PRE_POSITION | 3.0 |
| | DOWN_POSITION | 0.0 |
| LOADER_LOAD | LOAD_X/Y/R | 1390.0 / 0.0 / 90.0 |
| LOADER_MEAS | MEAS_X/Y/R | 1390.0 / 0.0 / 0.0 |
| AUTOCHANGE_X | CHANGE | 214.0 |
| | SAFETY | 1340.0 |
| PROBE_SAFETY | PROBE1~5 | 800/600/400/200/0 |

---

## 10. 스레드 모델

```
+-------------------+     +-----------------------------+
| UI Thread (Main)  |     | WorkThreadProbeProc (50ms)  |
| - 화면 갱신       |     | - 센서 상태 폴링             |
| - 사용자 입력     |     | - PLC I/O 모니터링           |
+-------------------+     | - 장비 상태 업데이트         |
                          +-----------------------------+

+----------------------------+     +---------------------------+
| SequenceThread             |     | AlarmMonitor x 24 Threads |
| - 초기화 시퀀스 실행       |     | - 도어 모니터 (200ms)     |
| - 글래스 처리 워크플로우    |     | - MC 오프라인 모니터      |
| - 단계별 완료 감시         |     | - 센서 모니터 (50~200ms)  |
+----------------------------+     | - 비상정지 모니터         |
                                   | - 온도 모니터             |
+----------------------------+     +---------------------------+
| TCP/IP Comm x 19 Threads  |
| - Vision 1/2               |
| - Color 1/2                |
| - Angle 1/2                |
| - WAD/PG/LCR/AGING         |
| - CIM/SYSTIME/VISMOTION    |
| - TSP 1~6                  |
+----------------------------+
```

**동기화 방식:**
- `CCriticalSection` - 데이터 접근 보호
- `HANDLE (Event)` - 시퀀스 흐름 제어
- `WaitForSingleObject / WaitForMultipleObjects` - 타임아웃 기반 대기

---

## 11. 핵심 파일 맵

| 역할 | 파일 경로 |
|------|----------|
| I/O 주소 정의 | `Source/ALPHA/Manager/IOMapAddressDefine.h` |
| I/O 제어 매니저 | `Source/ALPHA/Manager/IOMapControlMgr.h/cpp` |
| I/O 함수 라이브러리 | `CoreFunction/IOMapFunction/` |
| 장비 상태 머신 | `Source/ALPHA/FunctionModule/FunctionModuleEQState.h` |
| 시스템 초기화 | `Source/ALPHA/DoWork/SystemInitializeStep.cpp` |
| 글래스 시퀀스 엔진 | `Source/ALPHA/DoWork/DoWorkGlassSequenceEngine.h/cpp` |
| 글래스 측정 시퀀스 | `Source/ALPHA/DoWork/DoWorkGlassMeasureStep.cpp` |
| 수동 조작 스텝 | `Source/ALPHA/DlgSubManual_Meas.h` |
| 모션 컨트롤러 | `Source/ALPHA/Manager/MotorControlMgr.h/cpp` |
| TCP/IP 통신 | `Source/ALPHA/Manager/CommTcpIpFactoryMgr.h/cpp` |
| 통신 프로토콜 정의 | `Source/ALPHA/CommTcpIpDefine.h` |
| 알람 시스템 | `Source/ALPHA/Alarm/` |
| 네트워크 설정 | `Source/ALPHA/ConfigurationFile/Network.ini` |
| 포지션 설정 | `Source/ALPHA/ConfigurationFile/Position.ini` |
| 시스템 설정 | `Source/ALPHA/ConfigurationFile/Config.ini` |
| 레시피 데이터 구조 | `Source/ALPHA/Recipe/RecipeData.h` |
| 중앙 매니저 | `Source/ALPHA/AuroraMgr.h/cpp` |
