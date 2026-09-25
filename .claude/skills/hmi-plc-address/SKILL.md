---
name: hmi-plc-address
description: ALPHA HMI PLC 주소 매핑 — PLCMAdress.csv↔plc_MBIT_t enum positional 바인딩, 9999(-1 참) 함정, M.csv/F.csv, 상태 비트 출력, 비트 쓰기 오버로드. PLC 비트를 추가·수정·판정할 때 읽을 것
---

# hmi-plc-address

출처: `CLAUDE.md` (2026-09-17 /doctor 정리로 이동, 백업 `CLAUDE.md.bak_20260917_doctor`). 전체 규칙(인코딩 편집 규칙, 빌드, 3벌 인터락 원칙 등)은 루트 `CLAUDE.md` 가 우선한다.

#### PLCMAdress.csv ↔ plc_MBIT_t enum = positional 바인딩 (오매핑 최고위험)

`ReadAdressFileCSV`가 CSV를 **행 순서대로** `m_piPLCMAdr[nNext++] = 주소`로 채운다 → **CSV 행 순서 = `plc_MBIT_t` enum 선언 순서**가 계약(ABI). Description 컬럼은 라벨일 뿐 바인딩과 무관.
- **해석 공식**: enum값 = `4000 + (그 enum의 advancing 슬롯 인덱스)`. 실주소 = `m_piPLCMAdr[enum값 - 4000]`. 읽기 `CIOMapControlMgr::GetParam_DIOMapMemoryBitImport(enum)`이 이 변환 후 M 스캐너 read데이터를 조회.
- **alias 6개**(`X = 다른enum`, 예 `PLC_BIT_W_MUTIONG_ON = PLC_BIT_W_SR_51_ON`)는 값을 공유하고 **슬롯을 소비하지 않음** → 개수/슬롯 계산 시 제외 (advancing 슬롯 = 이름 수 − alias 수).
- **시작 검증은 개수만 본다**: `nNext + 4000 == PLC_MBIT_COUNT`(`IOMapControlMgr.cpp` verification)만 확인. **CSV 중간에 행을 삽입하고 enum엔 같은 위치로 안 넣으면 개수는 맞아 검증 통과하지만 삽입점 이후 전 매핑이 한 칸씩 밀려 조용히 오매핑**된다 → 팝업 없이 엉뚱한 비트 read/write. 실제 사고: 2026-09-03 `EMS SWITCH 17,4426`을 CSV 중간(SWITCH16 뒤)에 삽입했는데 enum엔 꼬리에 있어 154행 이후 438개 오매핑, `PLC_AUTO_CHANGE_X0_POSITION_04`가 M4343 대신 M4344를 읽어 항상 0.
- **주소 추가 규칙**: PLCMAdress.csv와 **enum 2사본에 같은 위치로 동시** 추가. **꼬리 추가(tail-append)가 가장 안전**(기존 슬롯 불변). 개수만 맞추지 말고 "advancing 슬롯 인덱스 ↔ CSV 행 인덱스"가 각 주소에서 일치하는지 검증할 것.
- **PLCMAdress.csv는 사본이 2개다.** 런타임이 읽는 실사용본은 **`Source/Bin/Config/CoreFunction/IOMapConfig/PLCMAdress.csv`**이고, 배포 템플릿 `Source/ALPHA/ConfigurationFile/CoreFunction/IOMapConfig/PLCMAdress.csv`는 **2026-05-18에서 멈춘 구버전(1085행, 슬롯 6·7 비트 9999)이었던 것을 2026-09-16에 실사용본으로 덮어써 동일하게 맞췄다**(구 템플릿은 `.bak_20260916_pre_sync`). 주소 작업은 항상 `Bin/Config` 쪽에서 하고 **끝나면 템플릿에 복사해 `cmp`로 동일 확인**할 것 — 둘이 어긋난 채 구 템플릿으로 배포하면 전 매핑이 밀린다.
- **enum 2사본은 시작 번호가 다르다.** 컴파일 대상 `Manager/IOMapAddressDefine.h`의 활성 블록은 `PLC_BIT_STATE_READY = 4000`으로 시작해 값이 4000~이고, `Source/Include/` 사본의 활성 블록은 `= 4000`이 없어 0~이다. **순서(상대 위치)만 계약이므로 추가 위치만 같으면 되고, 시작값을 임의로 맞추려 들지 말 것.** Include 사본은 참조본이 아니다 — **dSACC DLL(`CDlgSACC.h`)이 이 사본을 include 해 `PLCRDATA::bMData[]`(앱이 Manager 순서로 채움)를 인덱싱**하므로 순서가 어긋나면 SACC 화면의 비트 판정이 통째로 밀린다.
- **2026-09-17 점검에서 Include 사본이 4슬롯 어긋나 있었다.** 슬롯 152 에 `PLC_BIT_EMERGENCY_SWITCH17`(CSV·Manager 어디에도 없는 항목)이 남아 있고 꼬리 5개(`PLC_BIT_VOICE_BUZZER_CH1~4`, `PLC_BIT_SHUTTER_CLOSE_NON_CONTACT_SWITCH`)가 빠져, **슬롯 152 이후 DLL 의 모든 비트(`PLC_PACC_PACC_ALL_SEQ_DONE`, 카세트/버퍼 존재, AC 프로브 감지 등)가 한 칸씩 밀려 읽히고 있었다.** Manager 활성 블록을 그대로 복사(`= 4000` 만 제거)해 동기화했고(백업 `.bak_20260917_enumsync`), dSACC 리빌드가 필요하다. 검증 스크립트 `docs/tools/plc_enum_csv_check.pl`(`perl docs/tools/plc_enum_csv_check.pl` — Manager·Include·CSV 3자 대조, 이름 불일치 309건은 라벨 차이이며 `shift-signature`·`longest consecutive mismatch run`·SACC 비트 슬롯 비교로 밀림을 판정)을 enum/CSV 편집 후 항상 돌릴 것. Manager 사본은 앞쪽 `#ifdef _USE_NOT` 블록에도 `PLC_MBIT_COUNT`가 있으니 **활성 블록(마지막 `enum plc_MBIT_t {`)** 을 대상으로 편집한다.
- **정합성 자동 검증 방법**: 편집 전후로 "enum 슬롯[i] ↔ CSV 행[i]"을 전 구간 대조해 **불일치 건수가 변하지 않았는지**로 밀림을 판정한다(Description은 라벨이라 이름 자체는 원래 229건이 다름 — 개수 변화가 없으면 밀림 없음). 2026-09-10 `PLC_BIT_W_AC_X_SAFETY_POS`(M4981) / `PLC_BIT_W_EQ_BUSY_ON`(M4982) 꼬리 추가 시 이 방식으로 검증했다.
- **슬롯을 셀 때 `PLC_MBIT_COUNT`(마지막 센티넬)를 빼야 한다.** enum 항목을 그대로 세면 CSV 행수보다 **항상 1 많아** 없는 불일치를 보게 된다. 현재 정상 상태(2026-09-16 확인) = **CSV 데이터 1098행 / 이름 슬롯 1098개(alias 6개 제외) / `PLC_MBIT_COUNT` = 5098** (헤더 1줄 제외, 이 중 737행이 주소 `9999`). **파일 마지막 줄에 개행이 없어 `wc -l`은 1098(=헤더+1097)로 나온다** — 데이터 행을 셀 때는 `wc -l` 대신 split 결과를 쓸 것.
- **이름 변경은 enum 2사본 모두에 해야 한다.** 2026-09-10에 `PLC_BIT_W_*`로 바꿀 때 `Source/Include/` 사본에만 적용되고 **컴파일 대상인 `Manager/` 사본은 옛 이름 그대로**여서 코드가 옛 이름을 참조하는 상태가 된 전례가 있다. 어느 쪽을 고쳤는지 항상 양쪽 `grep`으로 확인할 것.
- **enum은 2사본**: 컴파일 대상은 `Source/ALPHA/Manager/IOMapAddressDefine.h`(`..\Manager\` include). `Source/Include/IOMapAddressDefine.h` 사본과 **동기 필수**(ODR). Manager 사본엔 `#ifdef _USE_NOT`(라인 721~2432) **비활성 enum 블록**이 있어 `enum plc_MBIT_t {`가 파일에 2번 등장 — **활성은 2번째(2436~)**. 카운트/편집 시 비활성 블록을 반드시 제외.
- **CSV 주소 `9999` = 미사용인데, 읽기 함수는 이때 `-1`을 반환하고 `-1`은 C++ 조건식에서 참이다 (2026-09-15 사고).** 로드 시 9999는 주소 0으로 저장되고 `GetParam_DIOMapMemoryBitImport/Export`는 주소 < 3000이면 `-1`을 돌려준다. `if (GetParam...(X))`나 `bool b = GetParam...(X)`로 쓰는 코드는 **막힌 비트를 항상 ON으로 본다.** 실제로 `PLC_AUTO_CHANGE_X0_POSITION_03`(AC X Safety, M4344)이 막혀 Stage X/R 이동·Probe Align의 "AC Safety/SACC 위치" 인터락이 항상 통과했고, 타이틀바 PROBE SAFETY ON은 막힌 두 비트를 `|` 로 합쳐 항상 노랑이다. 새 코드는 반드시 `== 1`로 비교할 것.
- **09-14~15에 PLCMAdress.csv 214행이 `9999,LYS 2026-09-14 disabled (Mxxxx not in IO LIST.txt …)`로 일괄 비활성화됐다.** 근거는 PLC 코멘트 비교(`D:\0.SW\PLC_MAP\compare_report.txt`)에서 코멘트가 비어 있던 주소였는데, **코멘트가 없어도 PLC가 출력하는 비트가 있다**(M4344·M4244·M4245는 스캔값 1로 확인 후 09-15 복구, M4255 온도 11 OVER·M4259 연기 11도 스캔값 1). 비트를 막거나 되살릴 때는 `Doc/15_PLC Address/SDC A3 EVEN HMI_M-Memory Map_260804_SW2.xlsx`의 `3. M-Memory Map`(이 설비는 **LO(PACC) 열**) · `IO List Map` 시트와 **`Logs\Control\IOControl\LogFile\PLC_READ_*.log`의 M영역 응답**을 함께 볼 것. 응답 데이터는 4자리 hex 워드가 M4000부터 이어지며 워드 내 **LSB = 낮은 주소**다(워드 k, 비트 b → M(4000+16k+b)). CSV는 기동 시에만 읽으므로 수정 후 ALPHA 재기동이 필요하다.
- `IOMapAddressDefine.h`는 pch 경유 → **수정 시 전체 리빌드**. CoreFunction(`McProtocolManager`)은 enum을 안 쓰고 `PLC_MBIT_MAX=2000` 배열만 사용(현재 1098 슬롯 < 2000, 여유 있음).

#### M.csv / F.csv = 표시용(주소-키), 스캐너 범위 주의
- `M.csv`/`F.csv`는 `CMcM/FaddressScanner::ReadFileCSV`가 **주소-키**(`vecRData.vecData[주소-RD_START].strDescription`)로 읽는 **표시 라벨 전용**. 실제 값은 `M*004000` 블록 리드로 전체 스캔 → 행 순서/추가는 **표시에만 영향, 제어 로직 무관** (PLCMAdress.csv의 positional 계약과 완전히 별개).
- **M.csv는 `dfMADDRESS_RD_START(4000) ≤ 주소 < dfMADDRESS_LIMIT_COUNT` 만 허용**(`CoreFunction/IOMapFunction/DataStructure.h`). 범위 밖 주소는 `"There are 'N' MAddress Numbers Out of range"` 팝업. **M3900대(PACC/EQ)는 M.csv가 아니라 PACC/ProbeCard 스캐너(SACC.csv 등) 담당** — M.csv에 넣으면 out-of-range 팝업(2026-09-03 M39xx 50개 넣어 발생).

- **M.csv는 read 대역만이 아니라 write 대역(4500~4997) 라벨도 담는다** (현재 데이터 564행, M4000~M4997). 새 write 비트를 추가하면 Manual PLC Bit 화면 표시를 위해 여기에도 `M<주소>,라벨` 한 줄을 넣는 것이 좋다(주소-키라 순서 무관, 리빌드 불필요).

#### 설비 상태 비트 주기 출력 (2026-09-10 신설)

`CDlgMainForm::SetEqStatusBit()` — **모니터 스레드(`WorkThreadProbeProc` 50ms) → `GetMonitor()`** 에서 호출되며, 값이 **바뀔 때만** PLC에 쓴다(`m_lOldACSafety` / `m_lOldEqBusy` 엣지 판정 — 매 틱 쓰면 PLC 쓰기 로그가 폭주한다). 새 상태 출력 비트는 이 함수에 붙일 것.
- **M4981 `PLC_BIT_W_AC_X_SAFETY_POS`** : `fabs(GetOrgPos(AXIS_X4) − GetACPosition()의 safety) <= 0.1` 이면 ON. Safety 값 출처는 위 "위치 설정 ini" 절 참조(`ProbeChange.ini [ACXPOS] POS3`).
- **M4982 `PLC_BIT_W_EQ_BUSY_ON`** : `GetSystemInitialize()->Is_ThreadRun()` 또는 `GeDoWorkGlassSequence()->Is_ThreadRun()` 이면 ON, 둘 다 false(IDLE)면 OFF. 전이 시 OCP 로그 `EQ BUSY bit -> n (Init:n, Cycle:n)`.
- **`CBaseSequenceImpl::Is_ThreadRun()`은 "스레드 생존"이 아니라 "`ThreadRun()` 실행 중"이다.** 스레드는 평소 `WaitForMultipleObjects`로 블록되어 있고 Start 이벤트가 와야 플래그가 선다(`BaseSequenceImpl.cpp:69/73`). 그래서 시퀀스 진행 여부 판정에 그대로 쓸 수 있다.

#### 비트 쓰기 오버로드 (수동 On/Off — 주의)
`SetMemoryNBitToDevice`는 2종 오버로드가 **의미가 반대**: `(int index)`는 index≥4000이면 **enum 인덱스로 해석**(`m_piPLCMAdr[index-4000]` lookup), `(CString "M4509")`는 **첫 글자(M) 떼고 raw M-주소로 직접** 쓴다. write 주소(4500~4999)는 항상 ≥4000이라 int 오버로드로는 raw 쓰기가 불가 → **수동 M-주소 On/Off는 반드시 CString 오버로드** `SetMemoryNBitToDevice(_T("M")+주소문자열, val, 1)` 사용.


#### IOMap 표시 라벨 조회의 빈 벡터 크래시 (2026-09-11 분석, 미수정)
- `CMcProtocolManager::GetParam_DIOMapAddressBitImport/BitImportName/WordImport/WordImportName`(McProtocolManager.cpp 585/600/687/702)은 `GetAddressImportDataPtr(dev)->vecData[nAddress]`를 **크기 검사 없이** 인덱싱한다. 2023.12.22의 null 가드는 스캐너가 없을 때만 걸린다 — 스캐너 쪽 `GetAddressImportDataPtr()`는 `&m_vecRData`(멤버 주소)를 돌려주므로 **절대 null이 아니다**.
- `SADDRESSMAPDATA` 생성자가 `vecData.clear()`하고 `resize`는 **`ReadFileCSV`/`ReadFileXLS` 안에만** 있다(스캐너 생성자의 `ReadFileCSV` 호출은 주석 처리). `InitDevice`가 그 스캐너의 CSV를 읽지 못하면 벡터가 size 0으로 남고 Release(`operator[]` 무검사)에서 null 역참조 → **Maint IO File 화면의 F 그리드가 `nStart=0`에서 죽는다**(F 영역만 `dfFADDRESS_RD_START=0`이라 첫 반복이 곧 index 0).
- 트리거: ① `CoreFunction.ini` `F_ADDRESS_USE=0` → F 스캐너 skip → 영구 empty. ② `CStdioFile File(strFileName, CFile::modeRead)`(McFaddressScanner.cpp:186, M/P/C 스캐너 동일)가 파일 부재·잠김에 `CFileException*`을 **던지는데 try/catch가 없어** `InitDevice` for 루프가 통째로 중단 — M.csv(i=0)에서 던지면 F 포함 이후 스캐너가 전부 empty.
- 곁가지 두 건: `GetAddressBitType`(336)의 default가 READ_M이라 2000~3899·5000 이상 인덱스는 `nAddress = index − 4000`이 **음수**가 된다(`CD_PopUp_Alarm_F.cpp`, `DlgMainForm.cpp`도 같은 함수를 쓴다). `m_vecpAddressScanner`는 ProbeCard/PACC를 **조건부 push_back**하는데 접근은 전부 `[enumIO_DEVICE_XXX]`라 `C_ADDRESS_USE=0, P_ADDRESS_USE=1`이면 PACC가 위치 3에 들어가 조회가 nullptr → 조용히 빈 값(현재 ini는 둘 다 1이라 우연히 일치).
- 수정안: 스캐너 생성자에서 `vecData.resize(총개수)`, 접근자 4개에 `nAddress` 범위 검사(없으면 `_T("")`), CSV open을 `File.Open(..., &fe)` 반환값 처리로. 1·2번은 부작용 없음(빈 값은 기존 `IsEmpty()→continue`가 흡수).
