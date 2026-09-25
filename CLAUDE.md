# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

ALPHA HMI — Samsung 디스플레이 Glass 검사 자동화 설비 제어 소프트웨어. MFC C++ Dialog-based 애플리케이션.
이 디렉토리에서 Claude Code를 실행하는 경우, HMI 코드 자체의 분석·수정·빌드·운용이 목적이다.

상세 분석: `docs/` 폴더 (핵심 아키텍처 분석 문서 00~12번 + 기능별 포팅/이슈 문서) 및 `PROJECT_INDEX.md` 참조.

## Build System

- **IDE:** Visual Studio 2022 Professional (toolset v143), MFC Dynamic DLL, **Unicode** character set
- **Platform:** x64
- **Solution:** `Source/ALPHA/ALPHA.sln` (8 projects: ALPHA EXE + 6 DLLs + FDCSense EXE)
- **Output:** `Source/Bin/ALPHA.exe`
- **DLL Projects:** Alarm, dSACC, dResult, dfcRecipe, dMCCSpec, ProbeHistory
- **EXE Project (embedded):** FDCSense (`CoreFunction/EX_FDC/FDCSense/`) — separate FDC data-collection process, built alongside ALPHA but launched independently
- **Repo path:** active checkout is `D:\0.SW\HMI` (sln at `Source\ALPHA\ALPHA.sln`). Older docs referenced `D:\OCP\0.SW_Matching\HMI` (stale) — use the current path. Dated backups live in `D:\0.SW\Backup\` and as `D:\0.SW\HMI_<yyyymmdd>.zip` snapshots (newest: `HMI_20260916.zip`) — never the working tree.
- **MSBuild 경로는 PC마다 다르다.** 이 PC(2026-09 기준)의 실제 경로는 **`2022/Professional/MSBuild/Current/Bin/MSBuild.exe`** (아래 명령은 이 경로 기준). 다른 빌드 PC에는 **VS2022 IDE(devenv)만 있고 CLI MSBuild 컴포넌트가 없는 경우**가 있다(그런 PC엔 `Program Files (x86)` 밑 VS2019 `MSBuild.exe`만 존재). 스크립트로 빌드하기 전에 실제 경로를 확인하거나 IDE에서 빌드할 것.

- **빌드 전 `ALPHA.exe`가 떠 있는지 확인할 것 (`tasklist | grep -i ALPHA`).** 실행 중이면 DLL 프로젝트의 post-build `copy → Source\Bin`이 잠긴 DLL 때문에 `MSB3073`으로 실패하고 ALPHA 링크가 건너뛰어진다 — 컴파일 오류처럼 보이지만 원인은 파일 잠금. `FDCSense.exe`/`MCCLog.exe`는 ALPHA의 자식이라 함께 내려간다. 사용자 설비 앱이므로 임의로 kill하지 말고 종료를 요청할 것.
- **Visual Studio(`devenv.exe`)가 열려 있으면 CLI 컴파일이 `error C1041`로 막힌다** — `vc143.idb`(컴파일러 중간 DB)를 VS가 쥐고 있어서이며, `cl.exe`가 안 떠 있어도 발생한다. **VS를 닫으라고 요구하지 말고 중간 디렉토리를 따로 주면 된다**(문법 검증 용도):

  ```bash
  "/c/Program Files/Microsoft Visual Studio/2022/Professional/MSBuild/Current/Bin/MSBuild.exe" "D:\\0.SW\\HMI\\Source\\ALPHA\\ALPHA.vcxproj" //t:ClCompile //p:Configuration=Debug //p:Platform=x64 //p:IntDir="<임시경로>\\" //m //nologo //v:minimal
  ```

  `IntDir`은 **반드시 `\`로 끝나야** 하고, 새 폴더라 pch부터 전체 재컴파일한다(수 분, 약 176MB). 검증이 끝나면 그 폴더는 지울 것 — 산출물은 쓰지 않는다.
- **ALPHA.exe가 떠 있어 링크가 막힐 때는 `//t:ClCompile`로 문법 검증만 먼저 할 수 있다** (`ALPHA.vcxproj`/`dfcRecipe.vcxproj` 대상, 링크·DLL 복사 없이 컴파일만 수행). 종료 후 전체 빌드로 마무리할 것.
- **자동화 테스트가 없다.** 솔루션 8개 프로젝트 중 테스트 프로젝트는 없고 테스트 러너도 없다. 검증 = 빌드 성공 → 설비에서 실행 → `D:\HBS\Logs\` 로그 확인. 코드 변경 후 "테스트 실행"을 찾지 말고 로그로 검증할 것.

Debug 빌드:

```bash
"/c/Program Files/Microsoft Visual Studio/2022/Professional/MSBuild/Current/Bin/MSBuild.exe" "D:\\0.SW\\HMI\\Source\\ALPHA\\ALPHA.sln" //p:Configuration=Debug //p:Platform=x64 //m //nologo //v:minimal
```

Release 빌드:

```bash
"/c/Program Files/Microsoft Visual Studio/2022/Professional/MSBuild/Current/Bin/MSBuild.exe" "D:\\0.SW\\HMI\\Source\\ALPHA\\ALPHA.sln" //p:Configuration=Release //p:Platform=x64 //m //nologo //v:minimal
```

컴파일만(링크·복사 없음, ALPHA 실행 중일 때 문법 검증용):

```bash
"/c/Program Files/Microsoft Visual Studio/2022/Professional/MSBuild/Current/Bin/MSBuild.exe" "D:\\0.SW\\HMI\\Source\\ALPHA\\ALPHA.vcxproj" //t:ClCompile //p:Configuration=Debug //p:Platform=x64 //m //nologo //v:minimal
```

Clean:

```bash
"/c/Program Files/Microsoft Visual Studio/2022/Professional/MSBuild/Current/Bin/MSBuild.exe" "D:\\0.SW\\HMI\\Source\\ALPHA\\ALPHA.sln" //t:Clean //p:Configuration=Debug //p:Platform=x64 //nologo
```

- **`Clean-Solution.ps1` (저장소 루트, `Clean-Solution.bat`이 래핑)** — `x64`/`Debug`/`Release`/`.vs`/`ipch`/`obj` 산출물 폴더를 재귀 삭제하되 **배포본이 있는 `Source\Bin`, `CoreFunction\dMCCSpec\MCC\BIN`, `CoreFunction\EX_FDC\Bin`은 제외**한다(`-ListOnly` 미리보기, `-Force` 무확인, `-WhatIf` 지원). MSBuild `//t:Clean`은 이 Bin 폴더들을 건드리지 않으므로 용도가 다르다.

## Runtime Dependencies & Deployment (`Source/Bin`)

`ALPHA.exe` is x64 and **statically imports** several third-party DLLs not built by the solution. A wrong-bitness or missing copy kills the process in the OS loader **before any code runs** — `0xc000007b` (STATUS_INVALID_IMAGE_FORMAT = wrong bitness) or `0xc0000135` (DLL not found). No app logging fires; it looks like "nothing happens" / instant exit.

| Dependency | Kind | Source |
|---|---|---|
| `ACSCL_x64.dll` | ACS SPiiPlus motion | shipped in `Source/Bin` (x64) |
| `SPR64DU80.dll` | FarPoint Spread grid (x64) | shipped in `Source/Bin` |
| `imaq.dll` / `NIIMAQDX.dll` / `nivision.dll` | NI Vision runtime | NI install → `C:\Windows\System32` (x64); SysWOW64 holds the x86 copies — don't confuse them |
| `olch2du8.dll` | ComponentOne/Olectra **Chart 2D, DLL edition** (used only by `UIMeasPathDlg`) | **must be x64**; links via `Source/Lib/olch2du8.lib` (x64 import lib). Deploy the x64 runtime to `Source/Bin` |
| `mfc140ud.dll`, `VCRUNTIME140*D`, `ucrtbased` | VS **Debug** CRT/MFC | Debug builds need the VS debug runtime on the run PC |

**DLL search order:** for a non-KnownDLL static import, Windows searches the **application directory (`Source/Bin`) before `System32`** — so drop the correct DLL next to `ALPHA.exe` to override a bad system copy (no admin, no risk to other apps). Diagnose bitness via the PE machine word (`e_lfanew` @ 0x3C, machine @ PE+4: `8664`=x64, `014c`=x86) or `dumpbin /dependents ALPHA.exe`.

**Known 0xc000007b incident (2026-08):** a 32-bit `olch2du8.dll` (from extracting `Downloads\Util.zip`'s `System32\` folder into System32) overwrote the correct x64 runtime → x64 process loaded a 32-bit DLL → crash. Fix: put a verified x64 `olch2du8.dll` in `Source/Bin`. See memory `olch2du8-0xc000007b`.

## Runtime Data & Logs (분석 시 첫 확인 위치)

**런타임 데이터는 저장소 밖에 있다** — 경로는 `Source/Bin/Config/SystemConfig.ini`의 `[OUTPUT_PATH]`가 정하며 현재 `RECIPE_PATH`/`LOG_PATH` 모두 `D:\HBS\`:

| 데이터 | 경로 | 비고 |
|---|---|---|
| 측정 레시피 | `D:\HBS\Recipe\Recipe\*.rcp` | 버전 있는 순차 텍스트 (아래 Recipe File Versioning) |
| Align 레시피 | `D:\HBS\Recipe\ALIGN_RECIPE\*.alg` | 버전 없음, 레시피 `m_chAlignName`으로 참조 |
| 패턴 이미지 | `D:\HBS\Recipe\Pattern\` | 서치 패턴 + 미연결 카메라용 시뮬 이미지 `Align<N>.png` |
| 결과 | `D:\HBS\RESULT\` | `RESULT_PATH` |
| 로그 | `D:\HBS\Logs\` | `LOG_PATH` + `\Logs\` |

로그 파일명은 `<이름>_yyyymmdd.log`, 날짜 바뀌면 자동 롤오버. `Source/Bin` 밑에는 로그가 없다. **로그는 CP949**라 읽을 때 `iconv -f CP949 -t UTF-8` 필요(그냥 읽으면 한글이 깨져 알람명을 못 알아본다).

| 로그 | 경로 | 내용 |
|---|---|---|
| 메인 SYST/WARN/ERRO | `OCP_yyyymmdd.log` (시스템명 = `OCP`) | `m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, ...)` 전부. 시퀀스 디버깅용 임시 로그는 이 API로 넣는다 |
| Glass SW Align 전용 | `LogFile/GLASS 3_GLASS_ALIGN_*.log` | Step Start/End, `AlignGlassTZero`/`XYZero` 잔차, 최종 `AlignGlass (XYR:...)` |
| Glass 시퀀스 스텝 | `LogFile/GLASS Sequence_Step_*.log` | 9단계 전이 |
| 모션 | `Control/Motion/Motion_*.log` | 축 지령/완료/알람 |
| PLC | `Control/IOControl/LogFile/PLC_READ_*.log`, `PLC_WRITE_*.log`, `F_Address_*.log` | 비트 read/write, F알람 |
| 센서/이벤트 | `Event/*.log` (알람 모니터별 파일) | |

## 작업별 상세 지침 (스킬, 필요할 때만 로드)

다음 절들은 2026-09-17 `/doctor` 정리로 `.claude/skills/<name>/SKILL.md` 로 옮겼다. 해당 코드 영역을 다룰 때 자동으로 읽히며, 직접 열어도 된다(`Skill` 도구 또는 파일).

- `hmi-logs` — ALPHA HMI 런타임 로그 분석 — D:\HBS\Logs 폴더 구조, ALPHA가 쓰지 않는 구 폴더, 0바이트 로그가 정상인 이유, LOG 화면 버튼↔PARA 매핑. 로그로 동작을 검증하거나 LOG 화면을 고칠 때 읽을 것
- `hmi-init-motion` — ALPHA HMI 초기화 22단계·호밍 완료확인, 비활성 축(bUse) 래퍼 차이, 모션·IO 표시 그리드 복제. SystemInitializeStep, MaintControlMgr 래퍼, Maint/메인 모션 그리드를 수정할 때 읽을 것
- `hmi-head-gantry-interlock` — ALPHA HMI 헤드 갠트리(X2/X3)↔Z축(Z1~Z4) 인터락 체계, P5 프로브 유닛 겹침 대역, Sub Z 이동 규칙, Z2 부호 규약, Position.ini/ProbeChange.ini 죽은 섹션. MotorControlMgr/MaintControlMgr 이동·인터락 코드를 수정할 때 읽을 것
- `hmi-plc-address` — ALPHA HMI PLC 주소 매핑 — PLCMAdress.csv↔plc_MBIT_t enum positional 바인딩, 9999(-1 참) 함정, M.csv/F.csv, 상태 비트 출력, 비트 쓰기 오버로드. PLC 비트를 추가·수정·판정할 때 읽을 것
- `hmi-comm-pg` — ALPHA HMI TCP/IP 통신 팩토리(MEAS_TYPE 분기, switch 23벌)와 PG 패턴발생기 프로토콜(S8F1/S10F3, 로그 검증). 통신 객체 추가나 PG 동작을 다룰 때 읽을 것
- `hmi-align` — ALPHA HMI Glass/Probe SW Align — .alg 레시피 구조, Sub Z 매핑, 센터링 부호·잔차 오프셋, Probe Align 보상 경로(X9/Y5), CCD 카메라 안정화, 수동 Align 실패 후 잠김 수정. Align 코드(DoWork*SWAlignStep, AlignRcpDlg, DlgAlignCCD)를 다룰 때 읽을 것
- `hmi-measure-grouping` — ALPHA HMI 측정 셀 그룹핑(헤드 페어링 순번 방식), 유휴 헤드 Safety, HEAD_OFFSET 센서 배치와 오프셋 축 규약. DoWorkGlassMeasureStep 측정 스텝·MoveCmdPos* 헤드 이동을 수정할 때 읽을 것
- `hmi-sacc-testmode` — ALPHA HMI SACC(프로브 자동교체) 화면·P5 단일 프로브·카세트 7슬롯·EQ 프로브 ID(D2100/D2150)·수동 ID 등록, Test Mode 다이얼로그(dfcRecipe DLL). dSACC/ProbeACC/TestMode 코드를 다룰 때 읽을 것

## Encoding & Source File Editing Rules (Critical)

HMI는 Unicode 프로젝트이며 파일은 대부분 **BOM 없는 ANSI/UTF-8** (`ALPHA.cpp`는 UTF-8 BOM).

**한글 텍스트 손상 방지:** 소스 파일에 한글 주석/문자열 리터럴이 다수 포함. 수정 시 반드시 준수:

1. **한글 포함 소스 파일(.cpp/.h)은 Edit 도구 사용 금지.** Edit 도구는 파일 전체를 UTF-8로 재인코딩하여 기존 한글(EUC-KR/ANSI)을 `\xEF\xBF\xBD`(U+FFFD)로 손상시킨다. 반드시 Perl raw byte 편집을 사용할 것.
2. **Perl raw byte 편집 방법:**
   ```bash
   perl -e '
     open(my $in, "<:raw", "file.cpp") or die;
     my $data = do { local $/; <$in> }; close $in;
     my $old = "exact ASCII string to find";
     my $pos = index($data, $old);
     die "not found" if $pos < 0;
     my $new = "replacement string";
     substr($data, $pos, length($old), $new);
     open(my $out, ">:raw", "file.cpp") or die;
     print $out $data; close $out;
   '
   ```
3. **수정 후 검증 (필수):** `perl -e 'open(F,"<:raw",$ARGV[0]);$d=do{local $/;<F>};$c=()=$d=~/\xef\xbf\xbd/g;print"damaged=$c\n"' file.cpp` — `damaged=0`이 아니면 백업에서 복원 후 재작업.
4. **손상 복구 (2026-09-18 경로 정정 — 구 경로는 실재하지 않는다):** ① **추적 파일이면 git이 가장 빠르다** — `git checkout -- <path>` 로 마지막 커밋 상태를 되돌린 뒤 Perl로 변경사항 재적용. ② 커밋 이후의 작업까지 살려야 하면 **당일 `.bak_yyyymmdd_<topic>`** (편집 전 항상 만든다)에서 복사. ③ 그것도 없으면 `D:\0.SW\Backup\HMI_20260831\` 또는 최신 스냅샷 `D:\0.SW\HMI_20260916.zip` 추출본. (사라진 구 경로: `D:\0.SW\HMI_20260831\`, `D:\0.SW\merge\`, `D:\OCP\0.SW_Matching\HMI_20260716\HMI\`.)
5. **새 한글 주석은 "그 파일의 기존 한글 인코딩"에 맞춰 넣을 것 (핵심 — 파일마다 다름).** 이 저장소는 인코딩이 섞여 있다: `Manager/MotorControlMgr.cpp`는 **CP949**, `DlgSubMaint_MotionFile.cpp`·`CD_PopUp_Motion_Move.cpp` 등은 **BOM 없는 UTF-8**. 인코딩을 틀리면(예: UTF-8 파일에 CP949 주석) 깨지는데 **`damaged=0`(U+FFFD) 검사로는 못 잡는다** (CP949 바이트는 U+FFFD가 아님). 2026-09-02 실제로 UTF-8 파일에 CP949를 넣어 깨진 전례.
   - **① 인코딩 판별:** 대상 파일의 기존 한글 리드 바이트 확인 — `EA~ED`(3바이트)=UTF-8, `B0~C8`(2바이트)=CP949. 빠른 판별: `perl -e 'open(F,"<:raw",$ARGV[0]);$d=do{local $/;<F>};$u=()=$d=~/[\xea-\xed][\x80-\xbf]{2}/g;print "utf8han=$u\n"' file.cpp` — `utf8han`이 크면 UTF-8, 0이면 CP949.
   - **② 삽입:** Perl에 `use utf8; use Encode;` 후, **UTF-8 파일이면 `Encode::encode("utf-8","한글")`, CP949 파일이면 `Encode::encode("cp949","한글")`**. Edit/Write 도구로 한글을 직접 넣으면 항상 UTF-8 바이트가 들어가므로 CP949 파일에서 깨진다.
   - **③ 삽입 후 검증(필수):** 넣은 주석 바이트를 그 파일 인코딩으로 되디코딩해 원문이 복원되는지 확인 — `Encode::decode("utf-8"|"cp949", $bytes)`. `damaged=0`만 믿지 말 것.
   - 기존 한글 주석/문자열은 절대 삭제·변경하지 말 것.
6. **수정 주석 형식:** `// LYS yyyy-mm-dd brief description` — 모든 소스코드 수정 시 반드시 추가.
7. **신규 사용자 메시지는 영어로 작성.** 일부 파일(`DlgSubManual_AgingTest.cpp` 등)은 한글이 BOM 없는 UTF-8인데 vcxproj에 `/utf-8`이 없어 CP949로 컴파일됨 → 해당 파일의 한글 리터럴은 런타임 표시가 깨짐. 파일별 인코딩 확인: 한글 리터럴 헥사 덤프(EC/EB 시작=UTF-8, B0~C8 대역=CP949, A2AA=EUC-KR 도 기호 °).

8. **이 PC의 도구 제약 (2026-09-10 확인).** `python`/`python3`은 **버전도 못 찍는 깨진 스텁**이라 쓰지 말 것(`python --version` → `Python`만 출력, 스크립트 실행 불가). `markitdown`·`python-pptx`도 없다. **텍스트/바이너리 가공은 Perl**, 압축은 PowerShell `System.IO.Compression`으로 한다(`zip` 명령도 없음 — `ZipFile::CreateFromDirectory`는 엔트리 경로에 `\`를 써서 OOXML이 깨지므로 `ZipArchive::CreateEntry`로 `/`로 정규화해 직접 넣을 것). LibreOffice(`soffice`)도 없어 문서 렌더링 QA 불가.
9. **설정 파일이 Excel/편집기에 열려 있으면 쓰기가 막힌다.** `Bin/Config` 밑 `.csv`를 Perl로 열 때 `Device or resource busy`로 실패한다(읽기는 됨). 편집 스크립트는 **쓰기 실패를 삼키지 말고 명시적으로 보고**하고, 사용자에게 해당 파일을 닫아 달라고 요청한 뒤 재시도할 것. `open(...) or die` 만 쓰면 "Died at line N"으로만 나와 U+FFFD 검사 실패와 구분이 안 되니 `or die "$f 쓰기 열기 실패: $!"`처럼 이유를 남긴다.

10. **Perl 편집은 인라인 one-liner 대신 `.pl` 파일로 작성해 실행할 것.** C++ 내용의 `{`, `//`, `$`, `@`가 Bash 인라인 인용과 충돌해 반복적으로 실패한다(2026-09 다수 사례). 스크래치패드에 Write 도구로 `.pl`을 만들고(`use utf8; use Encode;`, 패턴은 `s{...}{...}` 또는 `q{}`), 치환 횟수를 세어 기대값과 다르면 `die`, 편집 전후 한글 바이트 카운트(`[\xb0-\xc8][\xa1-\xfe]` 또는 `[\xea-\xed][\x80-\xbf]{2}`)가 같은지 출력한다. 대상 파일은 편집 전에 `.bak_yyyymmdd_<topic>` 백업.
11. **이 PC 의 perl 은 `local $/` 슬러프가 "루프 안에서" 통하지 않는다 (2026-09-11 확인 — 조용히 빈 문자열이 된다).** `foreach` 안에서 `local $/; my $d=<$fh>` 를 쓰거나, **그런 `sub` 를 `foreach` 안에서 호출해도** `length($d) == 0` 이 되고 `open`/`die` 는 통과하므로 **에러 없이 "검사 대상 0건, 실패 0건"** 이 나온다. 위 3번의 `$d=do{local $/;<F>}` 예시도 루프 밖에서만 믿을 것.
    - **파일 스코프나 단독 블록 `{ }` 에서는 정상 동작**한다 — 실제 편집 스크립트(`sub rd` 를 블록 안에서 1회 호출)가 문제없이 도는 이유.
    - **검증 스크립트는 셸 도구로 짜는 편이 안전하다**: 전체 인코딩 확인 `iconv -f CP949 -t UTF-8 file > /dev/null`, 손상 확인 `grep -c $'\xef\xbf\xbd' file`, 삽입줄 확인 `grep -n "LYS <날짜>" file | iconv -f CP949 -t UTF-8`.
    - "0건" 결과가 나오면 **먼저 읽은 바이트 수를 출력해 슬러프가 됐는지 확인**할 것. 2026-09-11 에 이 함정으로 멀쩡한 편집을 세 번 반복 검증했다.
12. **`Encode::decode($enc, $str, Encode::FB_CROAK)` 처럼 CHECK 플래그를 주면 원본 문자열을 소비해 빈 문자열로 만든다 (2026-09-14 실제 사고).** 삽입 검증용으로 편집 결과 `$d` 를 그대로 `decode(..., $d, FB_CROAK)` 에 넘긴 뒤 `$d` 를 파일에 썼더니 **0바이트 소스가 저장**됐다(`CommTcpIpObjectPG.cpp`, 백업에서 복원). 검증은 반드시 **복사본**(`my $copy = $d; decode(..., $copy, FB_CROAK)`)으로 하고, 쓰기 직전에 `length($d)` 가 편집 전 크기 이상인지 `die` 로 확인할 것(`Encode::LEAVE_SRC` 를 OR 해도 된다). 편집 스크립트가 찍는 "한글 바이트 N -> 0" 도 이 사고의 서명이다.
13. **`CLAUDE.md`·`docs/*.md`는 BOM 없는 UTF-8이라 Edit 도구로 고쳐도 된다.** 1번의 Edit 금지는 한글이 든 `.cpp/.h`(대부분 CP949)와 CP949 `.ini` 대상이다. `Bin/Config/*.ini`는 **파일마다 인코딩이 다르므로** 편집 전에 반드시 판별할 것 (2026-09-18 실측): `Config.ini`·`CoreFunction.ini`는 **CP949**, `Position.ini`·`SystemConfig.ini`는 **UTF-8**이다. 그래서 `iconv -f CP949`로 `Position.ini`를 읽으면 주석에서 실패하는데 이는 정상이며 손상이 아니다. 한글을 넣지 않고 **ASCII 값만 바꾸는 편집이면 인코딩과 무관**하므로, 값 한 줄 수정은 Perl raw byte로 그 줄만 치환하고 `diff`로 다른 줄이 안 바뀌었는지 확인하면 된다.
14. **Perl `s{...}{...}e` 치환문 안에서 `$1`을 쓰고 같은 식 안에서 또 정규식 매치를 하면 `$1`이 덮여 앞부분이 사라진다 (2026-09-17 실제 사고).** `s{(prefix\r\n\t+)nRet = ...}{$1."//old".(($1=~/(\t+)$/)[0])."new"}e` 로 `MaintControlMgr.cpp` Probe Align MOVE 의 `stPosition[1].dPosition = dPosY1;` 줄이 통째로 지워져 **헤드 LEFT(Y2) 목표가 0 이 되어 Y1/Y2 헤드가 충돌**했다(14:10·14:18·14:24 모션 로그 `HEAD BACK LEFT, POS:0.000`). 연결 연산자의 피연산자가 뒤늦게 평가되어 첫 `$1`도 안쪽 매치 결과(탭)로 바뀐다. **치환 표현식 안에서는 정규식을 다시 쓰지 말고, 캡처는 먼저 `my ($a,$b)=($1,$2)` 로 복사한 뒤 쓸 것.** 편집 후에는 반드시 바뀐 블록 전체를 `sed -n` 으로 다시 읽어 지워진 줄이 없는지 눈으로 확인한다(치환 횟수 검사만으로는 못 잡는다).

## OCP Option Branching (Runtime Equipment Type)

HMI는 `SystemConfig.ini`의 `[EQUIP_TYPE] MACHINE_TYPE` 값으로 런타임 설비 타입을 결정한다.
- `HMI` (기본값): 기존 5-Probe 동작 100% 유지
- `OCP`: 1-Probe(P5 MEAS only), Probe Rotate, Stage 이동 방식

### 접근 패턴
```cpp
// Manager 클래스에서 (m_pSystemConfigMgrPtr 보유)
bool bIsOCP = m_pSystemConfigMgrPtr->IsOCP();

// DoWork Step 클래스에서 (주의: GetSystemConfigMgrPtr() 직접 호출 불가)
bool bIsOCP = m_pShareItem->GetSystemConfigMgrPtr()->IsOCP();

// MotorControlMgr (자체 플래그 보유)
bool bIsOCP = m_pMotorControlMgr->IsOCP();
```

### OCP 핵심 차이
| 항목 | HMI | OCP |
|------|-----|-----|
| Probe Gantry | 5개 (P1~P4 AGING + P5 MEAS) | 1개 (P5 MEAS only) |
| Probe X축 | AXIS_PROBE_X9/X10 (Master/Slave) | **동일** — 2026-08-25 HW 컨셉 변경으로 가동. `AXIS_R1`은 미사용 |
| Probe Y축 | AXIS_PROBE_Y5 | **동일 — 2026-09-05 활성화** (`PBY5` "PROBE CROSS", ACS0/HW28, ±50mm). Probe Align XYZero의 Y 센터링이 이 축을 움직인다. `DisableAxesForOCP()`는 이제 P1~P4 + BACK Z(Z1)만 끈다 |
| 측정 이동 | Probe X 이동 → Contact → Head 측정 | Stage(AXIS_X) 이동 → Contact → Head 측정 |
| ANGLE Head | X2 주 제어 → X3 종속 | X3 주 제어 → X2 종속 |
| AGING | P1~P4 수행 | **P5에서 ODD 라운드만 수행** (EVEN 미수행, 리뷰 스냅 1회) |

**Stick 간 이송은 OCP에서도 계속 Stage Y가 담당한다** (2026-08-25 확정). Probe X(X9/X10)가 움직이게 된 것은 인터락·얼라인·Safety 경로 때문이며, stick 이송 주체가 바뀐 것이 아니다. 따라서 `DoWorkGlassMeasureStep`(stick 루프 / Probe X 이동 skip), `DoWorkGlassPreMeasStep`, `DlgSubManual_Meas::ManualMoveStick`의 OCP 분기는 **유지**한다.

상세: [docs/OCP_Option_ChangeHistory.md](docs/OCP_Option_ChangeHistory.md)

## Glass 방향/좌표 체계 (2026-08-07 확정 — 수정 시 반드시 준수)

Stage Rotate H/W 축 = `AXIS_LOADER_R`. 도면: `Doc/07_설계문서/HMI 설비 Contact 방향.pdf`, 상세: [docs/Glass_Rotation_Change_Plan.md](docs/Glass_Rotation_Change_Plan.md)

| 자세 | 각도 | 비고 |
|------|------|------|
| Load / Unload / Align | 0° | Position.ini LOAD_R/UNLOAD_R, Align 레시피(.alg) StageRotate |
| 장변 측정 | 90° | `m_nGlsRotate=1`, 레시피 스틱별 `m_fStageR` |
| 단변 측정 | 180° | `m_nGlsRotate=2` (신규값 — 기존 분기는 0/1만 처리, ==1 여부로 판정됨) |
| MEASURE 전환(흡착) | 270° | Position.ini MEAS_R |

- **측정 회전각 = 스틱 `m_fStageR` + USER OFFSET `m_fStageRotate` 합산** — 각도 이상 시 두 곳 모두 확인
- **스틱 인덱스 규약**: `MakeCellID` 기준 — rotate==1만 직행, **그 외 전부 `nMaxStick-iStick-1` 반전**. nCellStatus/AGING 스틱 슬롯(`m_nAgingStartStick`) 등 셀 상태 인덱싱은 모두 이 규약을 따라야 함
- **셀 좌표 = 헤드 Y1~Y4 목표 직결** (변환 없음). 셀 분배는 A01(첫 셀) 앵커 = **+Y측**에서 −Y 방향 (MakeCellDlg). 셀 라벨(A01..)은 제품 도면 기준 고정 — 방향 문제는 좌표를 고칠 것, 라벨을 뒤집지 말 것
- **MakeCell 셀 X 생성식 (2026-09-07 수정)**: `셀X = −(GLOBALX + CELLOX) − i×CELLDX − 블록갭` (`MakeCellDlg.cpp:656`). ini/자동채움의 GLOBALX(`−(1500/2−F)`)·CELLOX(`X2−F`)는 **−Y 코너 기준**이고 2026-08-07부터 셀은 A01(+Y측)에서 −Y로 진행하므로 앵커를 부호 반전해야 한다. 구식(+앵커)은 A01=−690에서 더 −로 나가 셀이 글래스 밖(Y1 MN −800 초과)에 생겼다. ROTATE_0 레시피는 08-07에 `_CELL_`을 수작업 반전한 것이라 ini 기준 재생성 검증엔 못 쓴다. HMI 규약: **CELLROW/CELLDX/CELLOX/GLOBALX = 헤드 Y(1500 장변) 방향 셀**, **CELLSTICK/CELLDY/CELLOY/GLOBALY = Stage X(925) 방향 스틱**(스틱 Y는 ContactOffset `X1−E−I+M`을 뺀 값). FY 시트의 "stick count(COL) 10,10"은 HMI의 CELLROW에 해당(전치 주의).
- **레시피 옵션 ALIGN USE(`m_sOptionItem.m_bUseAlign`)**: 2026-09-07부터 자동 시퀀스에서만 게이팅 — 엔진 `ThreadRu_nSequence`가 Glass/Probe SW Align 아이템을 건너뛰고(`Cycle Seq Item : … - Skip`), 흡착 단계 `_ProbeAlign` 서브스텝도 skip. 수동 `M_CMD_GLASS/PROBE_PROC`·초기화·프로브 교체 경로는 옵션과 무관. 스킵 시 기존 `m_dAlignOffset`은 리셋하지 않는다(직전 수동 정렬값 유지).
- R축 DEGREE 센서: `PLC_R_MOTION_LOADER_DEGREE_0~3` = M4400~4403 (0/90/180/270)
- 경로 UI 컷 코너: Config.ini `[GLASS EDGECUT]` = {0°:4, 90°:3, 180°:2, 270°:1} (1=우상/2=좌상/3=좌하/4=우하, 회전 표현은 화면상 시계 방향)
- X 이동 인터록: R이 스테이션 각 {0,90,180,270}±2 이내일 때만 허용 (MotorControlMgr)

## Architecture

레이어·클래스·9단계 시퀀스 개요는 [docs/01_ClassArchitecture.md](docs/01_ClassArchitecture.md), [docs/02_CallGraph.md](docs/02_CallGraph.md), [docs/03_AutoSequence.md](docs/03_AutoSequence.md) 참조 (`CAuroraMgr` 오케스트레이터 → `AuroraMgrPool` 매니저 풀 → `CDlgALPHA`/`CDlgMainForm` UI, 시퀀스는 `BaseSequenceImpl`→`BaseSequenceBody` 기반 `DoWork/*`).

### Motion (ACS SPiiPlus)

- **Controller:** ACS SPiiPlus TCP/IP (10.0.0.100:701), ACSCL_x64.dll v7.1.1.0
- **Axes:** 90 logical axes (`axis_type_t` enum in `CoreFunction/MotorFunction/AxisType.h`)
- **Speed grades:** HIGH/MID/LOW/HOME/LATCH/MULTI (per-axis velocity, acceleration, S-curve)
- **Probe Gantry:** 5개 (P1~P4 AGING, P5 MEAS), master/slave X-axis pairs
- **Interlock:** gantry/head/probe/loader collision 체크가 `MoveCmdPosA_Sub()`(절대) / `MoveCmdPosR_Sub()`(상대) / `MoveCmdPosMulti_Sub()`(멀티) **3벌로 복제**되어 있다. 한 곳만 고치면 나머지 경로로 빠져나가므로 인터락 수정은 항상 3벌을 함께 볼 것 (게이트 조건은 미묘하게 다르다 — 예: Z1 Down이 A/R_Sub는 `> 1`, Multi_Sub는 `> 10`). **게이트가 실동작 범위를 덮는지 호출부까지 추적할 것** — 시퀀스 계층(`MoveCmdPosWAD` 등)이 Z1 목표를 `>1 → 0`으로 클램프하는 바람에 모션 계층의 `dPos > 1` 게이트가 한 번도 열리지 않아 그 안의 인터락 전체가 죽은 코드였던 전례가 있다(2026-08-26 발견, Z1은 안전 -600 → 측정 0으로 상승하며 하강)
- **Homing:** ACSPL+ buffer 프로그램 실행 (`acsc_RunBuffer`), HomeFlag 변수로 완료 확인
- **Thread safety:** 5 critical sections (move/get/set/done/stop 분리)
- **Simulation:** `OUT_OF_SERVICE` 플래그로 하드웨어 없이 소프트웨어 테스트 가능

- **모션 로그 읽는 법**(`AXIS:n` 이 축 번호인 점, `DoWork_MotionDone NG` 가 리밋 사고의 1차 증거인 점, 지령값만 남아 과거 위치를 재구성해야 하는 점)은 스킬 `hmi-logs`.
- **SW 리밋(`.dat` MX/MN)이 HW 센서보다 넓은 축이 있다** — `.dat` 값만 믿지 말 것. 실측·사고 사례는 스킬 `hmi-init-motion`.

- **Dual Controller (OCP mode):** `df_MAX_CTRL=2` (defined in `MotorControl_ACS.h`), each axis maps to controller via `m_cAxis[iAxis].m_iACSMap` (0 or 1). Dispatch pattern: `m_hAcs[m_cAxis[iAxis].m_iACSMap]`. HMI mode uses `df_MAX_CTRL=1`.

Layer: `CMaintControlMgr` → `CMotorControlMgr` → `CMotorFunction` → `CMotorControl_ACS`
상세: [docs/04_Motion.md](docs/04_Motion.md), [docs/ACS_Dual_Controller_Plan.md](docs/ACS_Dual_Controller_Plan.md)

### PLC (Mitsubishi MELSEC MC Protocol)

Layer: `CIOMapControlMgr` → `CIOFunction` → `CMcProtocolManager` → `CMcAddressScannerImpl` subclasses. 개요는 [docs/05_PLC.md](docs/05_PLC.md), 주소 매핑 규칙·함정은 스킬 `hmi-plc-address`.

### Vision (NI Vision + Basler GigE)

Layer: `CAlignCameraMgr` → `CFrameGrabberFunction` → `CFrameGrabber_BS4M` (IMAQdx). 개요는 [docs/06_Vision.md](docs/06_Vision.md), Align·CCD 함정은 스킬 `hmi-align`.

### Alarm System

- **3-layer:** Alarm DLL (`CAlarmManager`) → `CAlarmController` (Singleton facade) → `CAlarmMonitorMgr` (120+ 모니터 스레드)
- **Monitor groups:** Emergency(16), MC_OFF(10), Sensor(73+), TeachKEY(1), DoorOpen(7)
- **Alarm flow:** Monitor thread → `SetAlarm(id)` → Heavy: 블로킹 대화상자 / Light: 비블로킹 알림
- **Alarm codes:** `enumAlarmID` (1000~10000) in `Source/Include/AlarmIDList.h`
- **Limit alarms — 센서 vs SW 분리 (2026-09-02):** HW 리밋 **센서** 감지 = `MOTION_OVER_LIMIT_POS=3501` / `..._NEG=3601` (기존 유지, `DoWork_MotionDone`에서 `GetHWLimit()` 실패 시 발생). **SW 리밋**(목표값이 `GetMax/MinSWLimit` 초과) = `MOTION_SW_OVER_LIMIT_POS=3901` / `..._NEG=3902` (신규, A/R/Multi_Sub 진입부에서 목표 검사). **새 알람 ID는 반드시 `Alarm.xls`에 행을 추가**할 것 — `FindAlarmTable(id)==FALSE`여도 조용히 넘어가지 않는다(2026-09-17 정정). `ALARM_DATA` 생성자가 `level = ALARM_LEVLE_HIGH`, `text = "UNKNOWN"`으로 초기화하므로 **본문이 `UNKNOWN`인 Heavy 알람이 뜨고, `SetHeavyAlarm`의 `WaitForSingleObject(hEvent, INFINITE)`가 호출 스레드를 작업자가 해제할 때까지 막는다.** `id < 1000`만 무시된다.
- **이동·호밍·시퀀스 실패는 어느 계층이 알리는가 (2026-09-18 정리).** 계층마다 로그 형식이 달라 그것으로 구분한다. **경로는 배타적이라 같은 실패가 두 번 뜨지 않는다** — 위 계층이 거부하면 아래로 내려가지 않기 때문이다.

  | 계층 | 함수 | 실패 시 동작 | 로그(실패) |
  |---|---|---|---|
  | 모션 | `CMotorControlMgr::MoveCmdPosA` / `MoveCmdPosR` / `MoveCmdPosMulti` | WARN + `SetAlarm(nRet)` | `…End(AXIS:<축이름>, … RET:nnnn)` |
  | Maint | `CMaintControlMgr::MoveCmdPosA/R/M` · `MoveOrgPosA/R/Multi` (6벌) | WARN + 범위 검사 후 `SetAlarm(nRet)` | `…Fail(AXIS:<축번호>, … RET:nnnn)` |
  | Maint | `CMaintControlMgr::MoveHome` | WARN + `SetAlarm`(같은 코드 연속이면 1회) | `MoveHome Start(AXIS:<축이름>, Ret:nnnn)` |
  | 시퀀스 | `CSystemInitializeStep` 마지막 `_Idle` | `SetAlarm(GetFailNum())` | `SystemInitializeStep - Fail Exit (alarm n)` |
  | 시퀀스 | `DoWorkGlassSequenceEngine::ItemStart` (**`bCompleteCheck==true` 일 때만**) | `SetAlarm(GetFailNum())` + complete 복구 | `Seq Item : … - Fail Exit (alarm n)` |

  - **`_Sub` 안에서는 직접 `SetAlarm` 을 부르지 말 것.** 실패 코드를 `return`/`nRet=` 로 올리기만 하면 래퍼가 낸다. 부르면 같은 ID 가 두 번 떠 팝업이 두 번 나온다(`MoveCmdPosMulti_Sub` 에 직접 호출 4곳이 있었고 2026-09-17 주석 처리).
  - **범위 가드**: Maint 계층은 `nRet >= 1000 && nRet < ID_END` 일 때만 그 코드로 낸다. 실패 상수 0/−1 이 `UNKNOWN` Heavy 알람으로 뜨는 것을 막는다. `MoveHome` 은 성공을 **1~999** 로 보고(인터락은 알람 ID ≥1000, ACS `acsc_RunBuffer` 는 실패 시 0/−1), 범위 밖이면 `MTOR_HOME_FAIL_ALARM` 을 쓴다.
  - **초기화는 `nRet &= MoveHome(...)` 로 AND 를 걸어 알람 ID 를 뭉갠다**(`1 & 4062 = 0`). 그래서 `MoveHome` 이 코드가 살아 있는 유일한 지점이고, 상위에서 다시 낼 수도 없어 중복 걱정이 없다. 수동 경로(Maint 화면)는 반환값을 받는 곳 자체가 없다.
  - **`MoveHome` 은 같은 코드가 이어지면 한 번만 띄운다**(`m_nLastHomeAlarm`, 성공 시 리셋) — 초기화가 축 20개 이상을 루프로 호밍하며 같은 인터락을 반복해 만나기 때문.
  - **초기화는 엔진 알람 블록을 타지 않는다.** 시작이 `MoveCommandProcess(_T("SEQUENCE_ITEM_INITIALIZE"), false)` 라 `ItemStart` 의 `bCompleteCheck` 가 false 이고, 엔진은 완료를 기다리지 않는다. 그래서 `_Idle` 스텝에서 직접 낸다(이미 알람이 떠 있으면 `GetAlarmOn()` 으로 덧씌우지 않음).
  - 반환 코드는 전부 `AlarmIDList.h` 의 알람 ID(3012, 3021~3032, 3301+축, 3701+축, 3901/3902, PLC 4001~ 등)이며 **`Alarm.xls` 에 행이 있어야 정상 문구가 나온다**(위 항목 참조).

상세: [docs/08_Alarm.md](docs/08_Alarm.md)

## Thread / Global State

런타임 스레드 120~180개(알람 모니터·시퀀스·PLC 스캐너·TCP 수신·UI 워커)와 8 싱글턴·4 공유메모리 구조는 [docs/09_Thread.md](docs/09_Thread.md), [docs/10_GlobalState.md](docs/10_GlobalState.md) 참조.

## Recipe File Versioning (Critical)

Recipe Save/Load uses sequential `ReadString`/`ReadDouble`/`ReadInt` — **order must match exactly** between Save (`RecipeMgrDlg.cpp`) and both Load files (`OutRecipeFunc.cpp`, `InRecipeFunc.cpp`).

- **Version enum:** `RCP_VERSION` in `Source/Include/preDefine.h` (also mirrored in `CoreFunction/dll_Recipe/dfcRecipe/preDefine.h`)
- **Pattern:** New fields go at FILE END (before `File.Close()`), guarded by `if (nVersion > RECIPE_FILE_Vxxxx)`
- **RECIPE_FILE_END** doubles as the current version number written during Save
- **Backward compat:** Old files without new fields hit the `else` branch which calls `.Clear()` on the new struct
- **Three-file sync rule:** Any Save/Load field addition must be updated in all 3 files simultaneously with identical ordering
- **Gate direction is `>`, not `>=`.** Since Save writes `RECIPE_FILE_END`, adding a field means adding `RECIPE_FILE_V100n` (END becomes n+1) and gating with `if (nVersion > RECIPE_FILE_V100n)`. `>=` would make already-saved files parse data they do not contain.
- **Stick별 필드는 `_STAGE_` 블록에 넣지 말고 꼬리 블록으로 (2026-09-09 `m_fProbeZOffset` 선례).** `_STAGE_`는 스틱 수 + CSV 문자열 3줄(번호/Y/R)을 순차 읽기라 줄을 끼우면 구파일이 깨진다. `S_StickInfo::m_fProbeZOffset`(PROBE Z CONTACT OFFSET, mm, **+=하강**)은 `RECIPE_FILE_V1009` 게이트의 꼬리 블록 `_STICK_CONTACT_OFFSET_` + CSV 1줄로 저장하고, 로드는 이미 `AddStick`된 `m_sData.m_vecStick[i]`에 대입한다. UI는 레시피 STAGE MOVE 그리드 **5행 "CONTACT OFFSET"**(구 "PROBE CARD MOVE" 죽은 행 재사용, `.rc` 무변경). 적용은 `CRecipeEditMgr::GetStickProbeZOffset(iStick)`(범위 밖=0, |값|>1.0mm 클램프+WARN) → **End Contact 목표(End+OD)에만 가산**, PreContact/UnContact 높이는 스틱 공통. 호출부: Measure `_Probe_Contact`(`GetMeasureStick()`), PreMeas `_Contact`(스틱 0 고정 — Stage가 `m_vecStick[0]`에 있음), AGING Move_Second/Move_3(OCP만, `m_nAgingStartStick[4]` P5 슬롯, -1이면 0), Manual `enumStepID_PROBE_CONTACT`(`m_iNumStick`). TestMode/레시피 테스트 컨택은 미적용. OCP 로그 `Probe Contact stick=n offset=… -> Z5 target=…`로 확인.
- **The two `preDefine.h` copies are an ABI contract, not just a convention.** `CRecipeEditMgr::GetRecipeInfo()` (`Recipe/RecipeEditMgr.cpp`) returns `&m_pRecipeFunc->m_recipeMgr.m_sInfo` — a raw pointer into the dfcRecipe DLL's own struct. A struct present in only one copy compiles fine and reads past the DLL's allocation at runtime. Diff both files after any `S_RecipeInfo` change; only comments should differ.
- **UI grids:** `InitGridUI(grid, id, nRow, nCol, bSmall, bHeader, PIX_UNIFORMITY)` pins every column to **61px** regardless of the control's width, leaving any excess as a blank strip that reads as one extra cell. `.rc` widths run ≈1.48px per dialog unit, so N columns need ≥ N×61/1.48 units — or re-spread the widths after `InitGridUI`.
- **resource.h 중복 ID (버튼 오작동 주의):** `resource.h`에 **같은 값의 IDC_ 다수 중복**(예 `1202`가 3개). MFC는 `ON_BN_CLICKED`을 **커맨드 ID로** 디스패치하므로 중복 시 **메시지맵에서 먼저 나온 핸들러가 선점** → 버튼을 눌러도 엉뚱한 핸들러 실행(뒤에 있는 진짜 핸들러는 죽은 코드). 증상 발견 시 해당 버튼에 **유일 ID 부여**(관례상 `21003`부터). `.rc`는 심볼을 참조하므로 `resource.h` `#define`만 고치면 `.rc`도 함께 갱신되지만 **전체 리빌드 필요**. (2026-09-03 `IDC_BTN_MANUAL_PLC_BIT_OFF` 사고.)

## Resume Inspection

Allows resuming an interrupted Glass inspection from where it stopped (e.g., after Alarm/Stop).

- **ResumeInfo.ini:** Persisted via `CResumeInfoMgr` (owned by `CAuroraMgr`'s manager pool, not a singleton — access via `m_pAuroraMgr->GetResumeInfoMgrPtr()` / `m_pShareItem->GetResumeInfoMgrPtr()`) — saves InnerID, last completed step, measurement index
- **Entry points:** Manual start (`DlgSubOperate.cpp`) and CIM Online (`TO_SERVER_MEAS_START`) both call `CheckAndApplyResumeMode()`
- **Recipe step:** In Resume mode, skips `ResultClear` and reloads previous raw data files into FPSpread via `Updata_Data()`
- 상세: [docs/Resume_Inspection_Plan.md](docs/Resume_Inspection_Plan.md)

## Key Rules for Modification

### 고위험 수정 영역

1. **`pch.h` 수정** — 전체 프로젝트 리빌드 유발 (`AlarmIDList.h`, `WindowEventHandler.h` 포함)
2. **`ShareMem.h` (VTM_SYSTEM)** — 멤버 추가 시 반드시 struct 끝에 추가 (공유 메모리 오프셋 보존)
3. **`AlarmIDList.h` 수정** — pch.h에 포함되어 있어 전체 리빌드
4. **`IOMapAddressDefine.h` (plc_MBIT_t enum)** — PLC 주소 상수는 물리 I/O에 직접 매핑, 변경 시 하드웨어 영향
5. **`AxisType.h` 수정** — 모든 모션 코드의 기반, 축 추가/변경 시 Position.ini와 Config_Motion_ACS.dat도 함께 수정.
   - `.dat`의 `HW[]`는 ACS 컨트롤러 축 번호이고, **`USE[0]` 축과 `USE[1]` 축이 같은 번호를 공유하는 경우가 있다** (예: `PBY3`/`PBY4`가 ACS0의 30/31을 `Z3`/`Z4`와 공유). `USE` 플래그를 우회해 축 배열을 컨트롤러로 넘기면 엉뚱한 축이 움직인다. 충돌 확인은 `ACS[]`+`HW[]` 조합으로 `USE[1]`끼리만 비교할 것.
   - **`stMotorPosition` 배열을 `ZeroMemory`로 초기화하지 말 것.** `AXIS_X == 0`이라 채우지 않은 슬롯이 "Stage Gantry를 0으로 이동"이라는 유효한 지령이 된다. `CMaintControlMgr::ClearMotorPos(pPos, n)`(슬롯을 `AXIS_NONE`으로 채움)을 쓰면 배열 길이를 그대로 넘겨도 `PackUsedAxes()`가 범위 밖 id(`< 0` 또는 `>= AXIS_TYPE_END`)를 버린다. 실제 사고: `MoveCmd_SafetyAll`이 P1~P4 슬롯 채움을 주석 처리하면서 카운트 5와 무초기화 상태를 유지해 쓰레기 슬롯이 전달, 초기화 중 크래시(2026-08-26). **슬롯 채움을 주석 처리할 때는 반드시 `ClearMotorPos`를 남길 것.** 참고로 `CMotorImpl::GetAxisProperty()`는 상한만 검사하고 enum 밖 id에는 `nullptr`를 반환한다.
6. **`preDefine.h` (Recipe 구조체)** — App(`Source/Include/`)과 DLL(`CoreFunction/dll_Recipe/dfcRecipe/`) 양쪽에 동일 파일 존재, 구조체 변경 시 반드시 양쪽 동기화. `RCP_VERSION` enum과 `S_RecipeInfo` struct 포함
7. **`FDCSheardDefine.h` / `SheardDefineMCC.h` (FDC/MCC 공유메모리 계약)** — `Source/Include/`와 DLL 프로젝트 내부에 수기 사본 존재, 배포된 외부 프로세스(FDCSense.exe, MCCLog.exe)와 struct 레이아웃이 바이트 단위로 일치해야 함. 필드 추가는 반드시 struct 끝에만

### 수정 영향도 (Impact Matrix)

| 수정 대상 | 주요 영향 파일 |
|-----------|---------------|
| Motion | MotorControlMgr → MaintControlMgr → SystemInitializeStep → DoWork/*(전체) → UI(Manual/Maint) |
| PLC | IOMapControlMgr → FunctionModule/*(전체) → AlarmMonitor*(전체) → DoWork/*(전체) |
| Vision | AlignCameraMgr → DlgAlignCCD → DoWorkGlassSWAlign/ProbeSWAlign → FrameGrabber*(CoreFunction) |
| Recipe | RecipeData → RecipeEditMgr → CommTcpIpFactoryMgr(배포) → DoWorkGlassRecipe → 49개 소스 파일 |
| Alarm | AlarmController → AlarmMonitorMgr → 120+ 모니터 스레드 → DlgSubAlarm → Alarm.dll |
| Sequence | BaseSequenceImpl → DoWork/*(전체) → SequenceManager → FunctionModule/*(전체) |

상세: [docs/12_ImpactAnalysis.md](docs/12_ImpactAnalysis.md)

### 대용량 파일 (반드시 특정 라인 범위만 읽을 것)

| File | Path | Lines |
|------|------|-------|
| MaintControlMgr.cpp | `Source/ALPHA/Manager/` | ~8,900 |
| DoWorkGlassMeasureStep.cpp | `Source/ALPHA/DoWork/` | ~6,800 (측정 스텝 5종이 거의 같은 코드로 복제) |
| MotorControlMgr.cpp | `Source/ALPHA/Manager/` | ~3,300 |
| SystemInitializeStep.cpp | `Source/ALPHA/DoWork/` | ~2,300 |
| DlgMainForm.cpp | `Source/ALPHA/` | ~2,350 |
| ShareMem.h | `Source/Include/` | ~2,000 |
| DefinesPLC.h | `Source/Include/` | 1,000+ |
| preDefine.h | `Source/Include/` + `CoreFunction/dll_Recipe/dfcRecipe/` | ~830 (dual copy, kept in sync) |
| CDlgSACC.cpp | `CoreFunction/dSACC/` | ~5,200 (SACC 화면 전체 + 도달 불가한 구 시퀀스 약 2,000줄) |

(줄 수는 2026-09-16 기준. 함수 위치는 줄 번호보다 `grep -n "::함수명"`으로 찾을 것 — 이 문서의 `파일:줄` 참조는 편집이 쌓이며 계속 밀린다.)

`CommTcpIpFactoryMgr.cpp` (`Source/ALPHA/Manager/`) was previously ~26,000 lines but has since been refactored down to ~2,400 — no longer needs line-range-only reads.

## Knowledge Base (docs/)

`docs/00~12_*.md` 핵심 아키텍처 분석, `docs/*_Plan.md`·`*_Comparison.md` 기능별 포팅/이슈 문서, `PROJECT_INDEX.md` 마스터 인덱스 — 목록은 `ls docs/`. 저장소 루트의 `Initial.md`·`ALPHA_System_Analysis.md`·`PLC_Mapping_and_Sequence.md`는 2026-05-22 작성본이라 경로(`D:\OCP\HMI\...`)와 구조(OCP 분기, Z1→Z3/Z4, 축 추가 이전)가 구식 — 배경 이해용으로만 볼 것. `docs/Weekly_Report_*.md`는 주간 보고 스냅샷이라 현재 동작의 근거로 쓰지 말 것.
