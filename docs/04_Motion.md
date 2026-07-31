# 04. Motion System Analysis - ALPHA HMI

## 1. Architecture Overview

The motion system uses a layered architecture with Strategy pattern for multi-vendor motor controller support:

```
Application Layer (UI Dialogs, Sequences, DoWork steps)
    |
CMaintControlMgr         -- Orchestrator: combines Motion + IO + Position
    |
CMotorControlMgr         -- Manager: thread-safe wrapper, interlock logic, logging
    |
CMotorFunction            -- Facade: delegates to implementation via polymorphism
    |
CMotorImpl (abstract)     -- Interface: pure virtual base class
    |
CMotorControl_ACS         -- Implementation: ACS SPiiPlus TCP/IP driver
```

### Key Files

| File | Path | Role |
|------|------|------|
| `AxisType.h` | `CoreFunction/MotorFunction/AxisType.h` | Axis enum, speed grades, `axis_data_type` struct, `CAxisType` class |
| `AxisType.cpp` | `CoreFunction/MotorFunction/AxisType.cpp` | Speed switching logic, defaults |
| `MotorImpl.h/.cpp` | `CoreFunction/MotorFunction/` | Abstract base class for motor controllers |
| `MotorFunction.h/.cpp` | `CoreFunction/MotorFunction/` | Facade (Strategy pattern host) |
| `MotorControl_ACS.h/.cpp` | `CoreFunction/MotorFunction/MotorCtrl_ACS/` | ACS SPiiPlus concrete implementation |
| `ACSC.h` | `CoreFunction/MotorFunction/MotorCtrl_ACS/include/` | ACS SPiiPlus C Library header v7.1.1.0 |
| `ACSCL_x64.dll/LIB` | `CoreFunction/MotorFunction/MotorCtrl_ACS/dll/` & `lib/` | ACS runtime library (x64) |
| `MotorControlMgr.h/.cpp` | `Source/ALPHA/Manager/` | Thread-safe manager with interlock logic |
| `MaintControlMgr.h/.cpp` | `Source/ALPHA/Manager/` | High-level orchestrator (Motor+IO+Position) |
| `PositionConfigMgr.h/.cpp` | `Source/ALPHA/Manager/` | Position preset management (INI-based) |
| `SystemInitializeStep.cpp` | `Source/ALPHA/DoWork/` | Homing sequence implementation |

---

## 2. ACS Controller Connection

### Connection Parameters
- **Protocol:** TCP/IP Ethernet
- **Default Address:** `10.0.0.100`
- **Default Port:** `701`
- **Configuration:** `CoreFunction/CoreFunction.ini` section `[MOTOR_CONTROL_MGR]`
  - `ADDRESS` key, `PORT` key

### Connection Flow

```
CMotorControlMgr constructor
  -> new CMotorFunction("ACS", configPath, filePath, "MOTOR_CONTROL_MGR")
    -> new CMotorControl_ACS(appPath, filePath, category)
      -> LinkIniFile(false)    // reads ADDRESS, PORT from INI
      -> LinkDatFile(false)    // reads Config_Motion_ACS.dat
    -> InitDevice()
      -> CMotorControl_ACS::InitDevice()
        -> acsc_GetConnectionsList()         // enumerate existing connections
        -> acsc_TerminateConnection()        // kill non-ACS connections
        -> acsc_OpenCommEthernetTCP(address, port)  // connect
        -> sets m_hAcs handle, m_bBdInstalled=true
```

### Disconnection
```
CMotorControl_ACS::~CMotorControl_ACS()
  -> acsc_CloseComm(m_hAcs)
```

### Connection Check
- `IsConnected()` returns `m_bBdInstalled`
- All motion functions check `m_hAcs == ACSC_INVALID` before proceeding

---

## 3. Axis Definitions and Mapping

### Axis Enum (`axis_type_t` in AxisType.h)

90 logical axes defined (enum values 0..89), `AXIS_TYPE_END = 90`:

| Enum | Value | Physical Name | Description |
|------|-------|---------------|-------------|
| `AXIS_X` / `AXIS_X0` | 0 | Top Gantry | Stage X (main stage) |
| `AXIS_X1` | 1 | | (secondary) |
| `AXIS_Y` / `AXIS_Y0` | 2 | Top Cross | Stage Y |
| `AXIS_X2` | 3 | Bottom Gantry | Head Gantry WAD (back) |
| `AXIS_X3` | 4 | | Head Gantry Vision (front) |
| `AXIS_X4` | 5 | Gantry | Auto Change Gantry |
| `AXIS_X5` / `AXIS_T` | 6 | | Auto Change |
| `AXIS_Y1` | 7 | Bottom Cross Left | Head Cross Left (WAD) |
| `AXIS_Y2` | 8 | Bottom Cross Right | Head Cross Right (WAD) |
| `AXIS_Y3` | 9 | Top Align Left | Head Align Left (Vision) |
| `AXIS_Y4` | 10 | Top Align Right | Head Align Right (Vision) |
| `AXIS_Y5` | 11 | Bottom Align Left | |
| `AXIS_Y6` | 12 | Bottom Align Right | |
| `AXIS_Y7` | 13 | Probe Cross Left | |
| `AXIS_Y8` | 14 | Probe Cross Right | |
| `AXIS_R` / `AXIS_R0` | 15 | Top Chuck Rotate | |
| `AXIS_R1` / `AXIS_PYAW` | 16 | Probe Rotate | |
| `AXIS_R2` / `AXIS_UNIT_CROSS` | 17 | Unit Cross | |
| `AXIS_Z` / `AXIS_Z1` | 18 | Head 1 Vision Z | Front vision head Z |
| `AXIS_Z2` | 19 | Head 1 Review Z | Back review head Z |
| `AXIS_Z3` | 20 | Head 1 Angle Z | |
| `AXIS_Z4` | 21 | Head 2 Vision Z | |
| `AXIS_Z5` | 22 | Head 2 Review Z | AF |
| `AXIS_Z6` | 23 | Head 2 Angle Z | |
| `AXIS_Z7` | 24 | Probe Z Left | |
| `AXIS_Z8` | 25 | Probe Z Right | |
| `AXIS_Z9` | 26 | Auto Change Z | |
| `AXIS_Z10` | 27 | Auto Change Z | |
| `AXIS_Z11` | 28 | SE Z | |
| `AXIS_UNIT_Z` | 29 | Unit Z | |
| `AXIS_UNIT_Z2` | 30 | Unit Z2 | |
| `AXIS_ZOOM1` | 31 | Zoom1 (29M) | |
| `AXIS_THETA1` | 32 | Theta1 (29M) | |
| `AXIS_FILTER1` | 33 | Filter1 (29M) | |
| `AXIS_VLENS1` | 34 | VLens1 (29M) | |
| `AXIS_ZOOM2` | 35 | Zoom2 | |
| `AXIS_THETA2` | 36 | Theta2 | |
| `AXIS_FILTER2` | 37 | Filter2 | |
| `AXIS_VLENS2` | 38 | VLens2 | |
| `AXIS_ROTATE1` | 39 | Rotate1 (4M) | ColorMix |
| `AXIS_FOCUS1` | 40 | Focus1 (4M) | ColorMix |
| `AXIS_ROTATE2` | 41 | Rotate2 | |
| `AXIS_FOCUS2` | 42 | Focus2 | |
| `AXIS_ROTATE3` | 43 | Rotate3 (4M) | |
| `AXIS_FOCUS3` | 44 | Focus3 (4M) | |
| `AXIS_ROTATE4` | 45 | Rotate4 | |
| `AXIS_FOCUS4` | 46 | Focus4 | |
| `AXIS_LOADER_X` | 47 | Loader X | |
| `AXIS_LOADER_Y` | 48 | Loader Y | |
| `AXIS_RJ` / `AXIS_LOADER_Z` | 49 | Rack Jack / Loader Z | Lift pin |
| `AXIS_LOADER_R` | 50 | Loader Rotate | |
| `AXIS_LP` | 51 | Loader LP1 | |
| `AXIS_LP2` | 52 | Loader LP2 | |
| `AXIS_L` | 53 | Lens Left | PPA |
| `AXIS_F` | 54 | Filter | PPA |
| `AXIS_WL1` | 55 | White Light 1 | |
| `AXIS_UL1` | 56 | UV Light 1 | |
| `AXIS_XS` | 57 | Sub X 1 | |
| `AXIS_YS` | 58 | Sub Y 1 | |
| `AXIS_L2` | 59 | Lens Right | |
| `AXIS_WL2` | 60 | White Light 2 | |
| `AXIS_UL2` | 61 | UV Light 2 | |
| `AXIS_XS2` | 62 | Sub X 2 | |
| `AXIS_YS2` | 63 | Sub Y 2 | |
| `AXIS_LENS` | 64 | Lens | |
| `AXIS_COLOR_DEGREE1` | 65 | Color Degree 1 | Tilt |
| `AXIS_COLOR_DEGREE2` | 66 | Color Degree 2 | |
| `AXIS_SPECTO_DEGREE1` | 67 | Specto Degree 1 | PSI |
| `AXIS_SPECTO_DEGREE2` | 68 | Specto Degree 2 | |
| `AXIS_PROBE_X1..X10` | 69..78 | Probe X 1..10 | 5 probes x2 (master/slave) |
| `AXIS_PROBE_Y1..Y5` | 79..83 | Probe Y 1..5 | |
| `AXIS_PROBE_Z1..Z5` | 84..88 | Probe Z 1..5 | |
| `AXIS_TYPE_END` | 90 | - | End marker |
| `df_RUNBUFFER_1` | 91 | - | Buffer run ID |
| `df_RUNBUFFER_2` | 92 | - | Buffer run ID |
| `df_USE_MAX_AXIS` | 100 | - | Array size for `m_cAxis` vector |

### Special Axis Constants

| Constant | Value | Purpose |
|----------|-------|---------|
| `AXIS_ALL` | -1 | All axes |
| `AXIS_ALL_Z` | -7 | All Z axes (probe Z group) |
| `AXIS_SACC` | -6 | SACC |
| `AXIS_LOADER` | -5 | Loader group |
| `AXIS_CAMERA` | -9 | Camera pseudo-axis |
| `AXIS_NONE` | -8 | No axis |

### Logical-to-Physical Axis Mapping

Mapping is stored per-axis in `CAxisType::m_iAxisMap` (int), loaded from `Config_Motion_ACS.dat` file field `HW[xx]`.

- **Set at load time:** `SetAxisMapping(logicalAxis, hwAxis)` stores `m_cAxis[logicalAxis].m_iAxisMap = hwAxis`
- **Used at runtime:** Every ACS API call uses `m_cAxis[iAxis].m_iAxisMap` to translate logical to physical axis number

The `.dat` file maps string keys to logical enums via `g_iMapAxis` CMap in `LinkDatFile()`:
```
"X0-AXIS"   -> AXIS_X0
"X2-AXIS"   -> AXIS_X2
"PBX1-AXIS" -> AXIS_PROBE_X1
...etc (70+ mappings)
```

---

## 4. Configuration Files

### Config_Motion_ACS.dat

Custom text format (not INI). Sections:
- `[SERVO]` - axis parameters with `NUM[n]` count, then per-axis lines:
  ```
  N[1] DIVISION[X0-AXIS] NAME[STAGE_X] USE[1] HW[0] HPLC[0] DIR[1] CLAMP[0] TYPE[1]
       VH[500.0] VM[200.0] VL[100.0] VO[50.0]
       AH[100.0] AM[80.0] AL[50.0]
       STH[50.0] STM[40.0] STL[25.0]
       PLS[1.00000] EL[1.00000]
       CN[0.0000] MX[100.0000] MN[-100.0000]
       OF[0.0000] HOME[0.0000]
  ```
- `[OFFSET]` - offset section
- `[BASEPOS]` - base position section
- `[GAP]` - gap section
- `[FILTER]` - filter section
- `[BUFFERS]` - buffer assignments
- `[END]` - end marker

#### Per-Axis Fields
| Field | Member | Description |
|-------|--------|-------------|
| `DIVISION` | `m_strDivision` | Axis name key (e.g., "X0-AXIS") |
| `NAME` | `m_strAxisName` | Display name |
| `USE` | `bUse` | Axis enabled flag |
| `HW` | `m_iAxisMap` | Physical hardware axis number |
| `HPLC` | `iNumHomePLC` | Homing buffer number on ACS controller |
| `DIR` | `m_iDir` | Direction (1 or -1) |
| `CLAMP` | `iClamp` | Clamp mode (0=no offset, other=apply offset) |
| `TYPE` | `iStepper` | Motor type (1=servo, 2=stepper with pulse conversion) |
| `VH/VM/VL/VO` | `dVelH/M/L/Home` | Velocity settings |
| `AH/AM/AL` | `dAccH/M/L` | Acceleration settings |
| `STH/STM/STL` | `dScvH/M/L` | S-curve/deceleration settings |
| `PLS` | `dPulse` | Pulse conversion factor (stepper type 2) |
| `EL` | `dEncL` | Encoder length factor |
| `CN/MX/MN` | `dDimCen/Max/Min` | SW limits: center, max, min |
| `OF` | `dOffset` | Home offset |
| `HOME` | `dHomePos` | Home position value |

### Position.ini

INI format, sections:
- `[SAFETY]` - SafetyStageX, SafetyHeadX1/X2, SafetyHeadY1/Y2, SafetyZ, SafetyZ2
- `[LOADER_Z]` - UP/MIDDLE/PRE/DOWN positions
- `[LOADER_LOAD]` - Load X/Y/R positions
- `[LOADER_UNLOAD]` - Unload X/Y/R positions
- `[LOADER_MEAS]` - Meas X/Y/R positions
- `[ALIGN_PICKER]` - Stage X, Align-to-Meas gap
- `[HEAD_OFFSET]` - HEAD1/HEAD2 offsets for WAD/VISION/COLOR/ANGLE (X/Y/Z each)
- `[PROBE_SAFETY]` - Safety positions for 5 probes
- `[PROBE_OFFSET]` - X/Y/T offset for each probe

### CoreFunction.ini

INI format, section `[MOTOR_CONTROL_MGR]`:
- `USE` (bool) - device use flag
- `OUT_OF_SERVICE` (bool) - out of service mode
- `DEVICE_TYPE` (string, default "ACS") - controller type
- `INTERLOCKPOS` (double) - interlock threshold position
- `INTERLOCKGANGAP` (double, default 500) - gantry gap interlock
- `INTERLOCKHEADGAP` (double, default 40) - head gap interlock
- `INTERLOCKHEADGAPB` (double, default 710) - head gap B interlock
- `INTERLOCKPROBEGAP` (double, default 200) - probe gap interlock
- `INTERLOCKLIFTPIN` (double, default 575) - lift pin interlock position

---

## 5. Servo On/Off (`SetAmpEnable`)

### Implementation

**`CMotorControl_ACS::SetAmpEnable(int iAxis, bool bOn)`**

- **Servo ON (bOn=true):** Calls `ResetDevice(iAxis)` which:
  1. `acsc_Kill()` - stop any motion
  2. `acsc_Disable()` - disable drive
  3. `acsc_FaultClear()` - clear faults
  4. `acsc_Enable()` - enable drive
  5. `acsc_SetFPosition()` - set feedback position to 0

- **Servo OFF (bOn=false):** Calls `KillDevice(iAxis)` which:
  1. For single axis: `acsc_Disable(m_hAcs, hwAxis, NULL)`
  2. For all axes (iAxis < 0): `acsc_KillAll()` + `acsc_DisableAll()`
  3. Sets `m_bHomeDone = false`

### Call Sites

| Caller | File | Context |
|--------|------|---------|
| `CMaintControlMgr::SetAmpEnable()` | `MaintControlMgr.cpp:86` | Wrapper with shared memory status update |
| `CMotorControlMgr::SetAmpEnable()` | `MotorControlMgr.cpp:2510` | Thread-safe wrapper |
| `CSystemInitializeStep` | `SystemInitializeStep.cpp:1634,1700` | Auto Change axis enable during homing |

---

## 6. Homing Sequences

### Low-Level: `CMotorControl_ACS::MoveHome(int iAxis)`

Executes homing by running a pre-programmed buffer on the ACS controller:
```cpp
acsc_RunBuffer(m_hAcs, m_cAxis[iAxis].m_setting.iNumHomePLC, NULL, NULL);
```
The homing algorithm itself resides in the ACS controller's ACSPL+ program buffers.

### Low-Level: `CMotorControl_ACS::CheckHomeDone(int iAxis, DWORD dwTimeout)`

Polls for homing completion in a loop (500ms interval):
1. Checks motor disable errors (up to 20 retries = 10 sec)
2. Calls `CheckHomeDoneLoop(iAxis)` which:
   - Reads `HomeFlag` array from ACS controller via `acsc_ReadInteger()`
   - Checks `ACSC_MST_INPOS` flag and zero velocity
   - If `iClamp != 0` and offset exists:
     - Moves to offset position
     - Waits for move done
     - Resets feedback position to 0 via `acsc_SetFPosition()`
   - Special case: AXIS_LP also sets AXIS_LP2 home done

### System Initialization Homing Order

The `CSystemInitializeStep::MotionHome(int iStep)` function implements a 12-step homing sequence:

| Step | Axes | Description |
|------|------|-------------|
| 0 | `AXIS_LOADER_Z` | Lift Pin homing |
| 1 | `AXIS_Z1, Z2, ZOOM1/2, THETA1/2, FILTER1/2, VLENS1/2, ROTATE1-4, FOCUS1-4, COLOR_DEGREE1/2, SPECTO_DEGREE1/2` | All Head Z and optical axes |
| 2 | (empty - skip) | Reserved |
| 3 | `AXIS_X2, X3, Y1, Y2, Y3, Y4` | Head Gantry X and Head Cross Y axes |
| 4 | `AXIS_PROBE_Z1..Z5` | Probe Z axes |
| 5 | Probe Z move to position 0 | Move all probe Z to zero |
| 6 | `AXIS_PROBE_Y1..Y5` | Probe Y axes |
| 7 | `AXIS_PROBE_X1,X3,X5,X7,X9` | Probe X master axes (odd numbers) |
| 8 | `AXIS_X` | Stage X |
| 9 | `AXIS_LOADER_R` | Loader Rotate |
| 10 | `AXIS_Z9` then `AXIS_X4` | Auto Change Z then Auto Change Gantry |
| 11 | (additional steps) | Post-homing position moves |

**Homing Timeout:** `df_HOMINGDONE_TIME = 1800` seconds (30 minutes)

### Homing Pattern
Each step follows the pattern:
1. `MoveHome(axis)` - starts homing buffer on ACS
2. `CheckHomeDone(axis, df_HOMINGDONE_TIME)` - waits for completion
3. Axes within a step are homed in parallel (all buffers started, then all checked)

### UI-Triggered Homing Steps

The initialization is split into 12 named step classes (`CSystemInitializeStep_MotionHoming1` through `CSystemInitializeStep_MotionHoming12`), each calling `MotionHome(0)` through `MotionHome(11)`. These are orchestrated by the `CSystemInitialize` sequence engine.

---

## 7. MoveAbs (Absolute Move)

### API Chain
```
CMaintControlMgr::MoveCmdPosA(axis, pos, blockMode, speed, useOffset)
  -> CMotorControlMgr::MoveCmdPosA(axis, pos, blockMode, speed)
    -> MoveCmdPosA_Sub()     // interlock checks
    -> MoveCmdPosA_Move()    // actual move
      -> CMotorFunction::MoveCmdPosA()
        -> CMotorControl_ACS::MoveCmdPosA()
```

### `CMotorControl_ACS::MoveCmdPosA()` Implementation

1. Check connection (`m_hAcs != ACSC_INVALID`)
2. Check axis in use (`bUse`)
3. Check motor enabled (`GetMotorStatus(STATE_MOT_DISABLE)`)
4. Set speed if specified (`SetSpeed(iAxis, iSpeed, 2)`)
5. Check SW limits (`dDimMax`, `dDimMin`)
6. Compute move position: `dMovePos = (dPos - dHomePos) * m_iDir`
7. Call `MoveToPoint(iAxis, dMovePos)`:
   - If stepper type 2: converts to pulse units `(dPulse * pos) / dEncL`
   - Calls `acsc_ToPoint(m_hAcs, 0, hwAxis, point, NULL)`

### Blocking Mode

When `bBlockMode=true` in `CMotorControlMgr::MoveCmdPosA_Sub()`:
1. Waits for `CheckMoveDone()` 
2. Verifies final position within 0.01mm tolerance
3. Retries up to 3 times if position error detected

### Head Offset

`CMaintControlMgr::MoveCmdPosA()` supports `bUseOffset` parameter:
- When true, subtracts `m_dMoveOffset[iAxis]` from target position
- Offset set via `CMotorControlMgr::SetHeadOffset(iAxis, dOffset)`

### Call Sites

| Caller | File | Context |
|--------|------|---------|
| `CMaintControlMgr::MoveCmdPosA()` | `MaintControlMgr.cpp` | Primary high-level API |
| `CMaintControlMgr::MoveCmdPosAging()` | `MaintControlMgr.cpp` | Aging sequence moves |
| `CMaintControlMgr::MoveCmdPosReview()` | `MaintControlMgr.cpp` | Review position moves |
| `CMaintControlMgr::MoveCmdPosWAD()` | `MaintControlMgr.cpp` | WAD head moves |
| `CMaintControlMgr::MoveCmdPosVision()` | `MaintControlMgr.cpp` | Vision head moves |
| `CMaintControlMgr::MoveCmdPosColor()` | `MaintControlMgr.cpp` | Color head moves |
| `CMaintControlMgr::MoveCmdPosAngle()` | `MaintControlMgr.cpp` | Angle head moves |
| `CDlgSubManual_Meas` | `DlgSubManual_Meas.cpp` | Manual measurement UI |
| `CDlgSubOperate` | `DlgSubOperate.cpp` | Operator panel |
| `DoWorkGlassMeasureStep` | `DoWorkGlassMeasureStep.cpp` | Auto measurement |
| `DoWorkGlassPreMeasStep` | `DoWorkGlassPreMeasStep.cpp` | Pre-measurement positioning |
| `DoWorkGlassAdsorptionStep` | `DoWorkGlassAdsorptionStep.cpp` | Glass adsorption |
| `FunctionModuleEQStateChangeStep` | `FunctionModuleEQStateChangeStep.cpp` | State change sequences |
| `FunctionModuleProbeACCStep` | `FunctionModuleProbeACCStep.cpp` | Probe ACC |
| `CD_PopUp_Motion_Move` | `CD_PopUp_Motion_Move.cpp` | Popup move dialog |
| `D_MoveHomePosition` | `D_MoveHomePosition.cpp` | Home position move dialog |

---

## 8. MoveRel (Relative Move)

### `CMotorControl_ACS::MoveCmdPosR()`

1. Check connection, motor enabled
2. Set speed if specified
3. Compute absolute target: `dCur = GetCurPos() + dPos`
4. Check SW limits on absolute target
5. Apply direction: `dPos2 = dPos * m_iDir`
6. Call `MoveToPoint(iAxis, dPos2, ACSC_AMF_RELATIVE)` - uses relative flag

### Interlock Checks

Same interlock logic as absolute move but computes final position as `GetOrgPos(iAxis) + dPos` for limit checks.

### Call Sites

| Caller | File | Context |
|--------|------|---------|
| `CMaintControlMgr::MoveCmdPosR()` | `MaintControlMgr.cpp` | High-level relative move |
| `CSystemInitializeStep` | `SystemInitializeStep.cpp:580` | Auto Change repositioning during init |

---

## 9. Multi-Axis Move

### `CMotorControl_ACS::MoveCmdPosMulti(stMotorPosition *pPos, int iAxis, int iSpeed)`

1. Check all axes enabled
2. Build axis array `Axes[]` and position array `Points[]`
3. Apply speed, direction, pulse conversion for each
4. Terminate axis list with `Axes[iAxis] = -1`
5. Call `acsc_ToPointM(m_hAcs, 0 | ACSC_AMF_MAXIMUM, Axes, Points, NULL)`
   - `ACSC_AMF_MAXIMUM` flag means each axis moves at its own maximum configured speed

### `stMotorPosition` Structure
```cpp
struct stMotorPosition {
    int axis;
    double dPosition;
    int iLens;
    int iTube;
};
```

### Call Sites

Used extensively for synchronized multi-axis moves in:
- Glass measurement positioning
- Probe positioning
- Safety position moves
- Glass align sequences
- All `DoWork*Step` files

---

## 10. Jog

### `CMotorControl_ACS::MoveJog(int iAxis, bool bDir)`

1. Check connection and motor enabled
2. Determine direction based on `m_iDir` (reverse flag)
3. Call `acsc_Jog(m_hAcs, 0, hwAxis, direction, NULL)`

Direction constants: `ACSC_POSITIVE_DIRECTION = 1`, `ACSC_NEGATIVE_DIRECTION = -1`

### Call Sites

| Caller | File |
|--------|------|
| `CMotorControlMgr::MoveJog()` | `MotorControlMgr.cpp:2377` |

Jog is used only through `CMotorControlMgr`. No direct UI call sites found (likely accessed through maintenance dialog controls).

---

## 11. Stop and Emergency Stop

### StopDevice (Controlled Stop)

**`CMotorControl_ACS::StopDevice(int iAxis)`**
- Calls `acsc_Halt(m_hAcs, hwAxis, NULL)` - decelerates to stop
- Checks axis enabled before stopping

### KillDevice (Emergency Stop)

**`CMotorControl_ACS::KillDevice(int iAxis)`**
- Single axis: `acsc_Disable(m_hAcs, hwAxis, NULL)` - immediate disable
- All axes (iAxis < 0): `acsc_KillAll()` + `acsc_DisableAll()`

### StopHome

**`CMotorImpl::StopHome()`** - base implementation is empty; no ACS override found.

### StopBuffer / RunBuffer

- `acsc_StopBuffer(m_hAcs, bufferNum, NULL)` - stops a running ACSPL+ program buffer
- `acsc_RunBuffer(m_hAcs, bufferNum, NULL, NULL)` - starts a buffer (used for homing)

### CMaintControlMgr::StopMove(int iAxis)

High-level stop that delegates to `CMotorControlMgr::StopDevice()`.

### Call Sites

| Function | Caller | File |
|----------|--------|------|
| `StopMove()` | `DlgSubMaint_MotionFile` | `DlgSubMaint_MotionFile.cpp:1139` |
| `StopMove()` | `SystemInitializeStep` | `SystemInitializeStep.cpp:591` |
| `KillDevice()` | `MaintControlMgr` | `MaintControlMgr.cpp` |

---

## 12. Interlock and Safety System

### Software Interlocks in `CMotorControlMgr::MoveCmdPosA_Sub()`

The interlock system is implemented as pre-move checks in `MoveCmdPosA_Sub()` and `MoveCmdPosR_Sub()`. All checks must pass before the actual move command is issued.

#### Interlock Categories

| Category | Axes | Check | Error Code |
|----------|------|-------|------------|
| **Loader Rotate** | `AXIS_X` | Loader R position must be near 0, 90, or 180 degrees | `MOTION_ROTATE_POS_ERROR` |
| **Loader Rotate** | `AXIS_LOADER_R` | Stage X must be > interlock position | `MOTION_ROTATE_POS_ERROR` |
| **Gantry collision** | `AXIS_X2, X3` | Gap between X2 and X3 must be >= `m_dInterlockGantryGap` (500mm) | `MOTION_INTERLOCK_7` |
| **Head collision** | `AXIS_Y1, Y2` | Gap must be >= `m_dInterlockHeadGapB` (710mm) | `MOTION_INTERLOCK_7` |
| **Head collision** | `AXIS_Y3, Y4` | Gap must be >= `m_dInterlockHeadGap` (40mm) | `MOTION_INTERLOCK_7` |
| **Probe collision** | `AXIS_PROBE_X1,X3,X5,X7,X9` | Adjacent probe gap must be >= `m_dInterlockProbeGap` (200mm) | `MOTION_INTERLOCK_7` |
| **Probe master-slave** | `AXIS_PROBE_X2,X4,X6,X8,X10` | Slave must be within 5mm of master (X_n-1) | `MOTION_INTERLOCK_7` |
| **Probe Z contact** | Stage/Probe XY axes | All probe Z must be < 0.1 before XY move | `MOTION_INTERLOCK_8` |
| **Head Z collision** | `AXIS_Z1, Z2` | Probe Z must be > -5.1 when head Z > 1.0 | `MOTION_INTERLOCK_7` |
| **Head Z vs Probe** | `AXIS_Z2` | Head X2 must be > MinSWLimit+35 when Z2 down | `MOTION_INTERLOCK_7` |
| **Head Z vs Probe X** | `AXIS_Z1, Z2` | Head Gantry must be > ProbeX9 + 20mm | `MOTION_INTERLOCK_CHECK_ALARM` |
| **Probe vs Head** | Probe XY axes | Head Z1 and Z2 must be < 1.0 | `MOTION_INTERLOCK_8` |
| **Loader Z down** | All non-LOADER_Z | Loader Z must be < 0.1 | `MOTION_INTERLOCK_1` |
| **Loader Z** | `AXIS_LOADER_Z` | Stage X must be > MaxSWLimit-100 | `MOTION_INTERLOCK_3` |
| **Loader Z vs Probe** | `AXIS_LOADER_Z` | ProbeX9 must be < `m_dInterlockLifePinPos` (575) | `MOTION_INTERLOCK_3` |
| **Probe X9 vs Head** | `AXIS_PROBE_X9` | X9 must not exceed X2+5 | `MOTION_INTERLOCK_7` |
| **Probe Z vs Head Z** | `AXIS_PROBE_Z1..Z5` | If probe Z < -5.1, head Z must be < 1.0 | `MOTION_INTERLOCK_7` |

### Hardware Limit Check

**`CMotorControl_ACS::GetHWLimit(int iAxis, short bDir)`**
- Uses `acsc_GetFault()` to read safety flags
- Positive limit: `ACSC_SAFETY_RL` (0x01)
- Negative limit: `ACSC_SAFETY_LL` (0x02)

### SW Limit Check
- Max: `m_cAxis[iAxis].m_setting.dDimMax`
- Min: `m_cAxis[iAxis].m_setting.dDimMin`
- Checked in `MoveCmdPosA()` before sending move command

### ACS Controller Interlock Check

**`CMotorControl_ACS::CheckInterlock(CString strAxis)`**
- Reads an integer variable from ACS controller: `acsc_ReadInteger(m_hAcs, ACSC_NONE, varName, ...)`
- Used to query controller-side interlock variables

---

## 13. Gantry Configuration

### Dual Gantry System

The HMI equipment uses two independent gantries:
- **AXIS_X2** - "Bottom Gantry" / WAD (back head gantry)
- **AXIS_X3** - Vision (front head gantry)

Anti-collision interlock between them enforces minimum gap of `m_dInterlockGantryGap` (default 500mm).

### Gantry Pair Checking

In `SetSpeedDirect()`, axes `AXIS_UNIT_Z` and `AXIS_Z9` are treated as dual-axis:
```cpp
if (iAxis == AXIS_UNIT_Z || iAxis == AXIS_Z9) {
    for (int i = iAxis_mapping; i <= iAxis_mapping + 1; i++) {
        acsc_SetVelocity(...);
        acsc_SetAcceleration(...);
        // etc. for both axes
    }
}
```

---

## 14. Master/Slave Probe Configuration

### 5-Probe System

The HMI uses 5 probe gantries, each with master (odd) and slave (even) X axes:

| Probe | Master | Slave | Y | Z |
|-------|--------|-------|---|---|
| P1 (AGING1) | AXIS_PROBE_X1 | AXIS_PROBE_X2 | AXIS_PROBE_Y1 | AXIS_PROBE_Z1 |
| P2 (AGING2) | AXIS_PROBE_X3 | AXIS_PROBE_X4 | AXIS_PROBE_Y2 | AXIS_PROBE_Z2 |
| P3 (AGING3) | AXIS_PROBE_X5 | AXIS_PROBE_X6 | AXIS_PROBE_Y3 | AXIS_PROBE_Z3 |
| P4 (AGING4) | AXIS_PROBE_X7 | AXIS_PROBE_X8 | AXIS_PROBE_Y4 | AXIS_PROBE_Z4 |
| P5 (MEAS)   | AXIS_PROBE_X9 | AXIS_PROBE_X10 | AXIS_PROBE_Y5 | AXIS_PROBE_Z5 |

### Master/Slave Interlock

Slave axes (even: X2,X4,X6,X8,X10) are checked against their master (axis-1):
- Max deviation: 5mm
- Error: `MOTION_INTERLOCK_7`

### Out-of-Service Simulation

When `IsOutOfService() == true`, probe master moves also update slave position:
```cpp
if (iAxis == AXIS_PROBE_X1 || ... || AXIS_PROBE_X9) {
    m_dMovePos[iAxis + 1] = dPos;  // slave tracks master
}
```

---

## 15. WaitMotionDone / CheckMoveDone

### `CMotorControl_ACS::WaitMotionEnd(int iAxis, DWORD dwTimeout)`

Direct ACS API call: `acsc_WaitMotionEnd(m_hAcs, hwAxis, timeout)`

### `CMotorControl_ACS::CheckMoveDone(int iAxis, DWORD dwTimeout)`

Custom polling loop with 50ms interval:
1. Check motor still enabled (abort if disabled)
2. Check negative/positive limit hit (abort if hit)
3. Read motor state via `acsc_GetMotorState()`
4. Done when: `NOT MOVING` AND `IN_POSITION`
   - `bMotorState_MOVE != ACSC_MST_MOVE`
   - `bMotorState_INPOS == ACSC_MST_INPOS`
5. Timeout: `dwTimeout * 1000` milliseconds

### Default Timeouts

| Constant | Value | Usage |
|----------|-------|-------|
| `df_HOMINGDONE_TIME` | 1800 sec (30 min) | Homing completion |
| `df_MOVINGDONE_TIME` | 60 sec | Normal move completion |

---

## 16. Position Management

### Current Position

**`CMotorControl_ACS::GetCurPos(int iAxis, int iLens, int iTube)`**

1. Read feedback position: `acsc_GetFPosition(m_hAcs, hwAxis, &dFPosition, NULL)`
2. If stepper type 2: convert from pulse `(dEncL * pos) / dPulse`
3. Apply direction: `dResult = dPos * m_iDir`
4. Add home position: `dResult2 = dHomePos + dResult`
5. Store in `m_cAxis[iAxis].m_dPos`

### Position Types in Manager Layer

| Method | Class | Description |
|--------|-------|-------------|
| `GetCurPos(axis, lens, tube)` | `CMotorControlMgr` | Position with head offset subtracted |
| `GetOrgPos(axis)` | `CMotorControlMgr` | Raw position without offset |
| `GetCurPos(axis)` | `CMaintControlMgr` | Uses offset from `m_dMoveOffset[]` |
| `GetOrgPos(axis)` | `CMaintControlMgr` | Raw position |

### Out-of-Service Mode

When `IsOutOfService() == true`:
- `GetCurPos()` returns `m_dMovePos[iAxis] - m_dMoveOffset[iAxis]` (simulated)
- `GetOrgPos()` returns `m_dMovePos[iAxis]` (simulated)
- Moves update `m_dMovePos[]` array directly without hardware

---

## 17. Head Offset System

### `CMotorControlMgr::SetHeadOffset(int iAxis, double dOffset)`

Stores offset in `m_dMoveOffset[100]` array. Used to compensate for different measurement head positions.

### Position.ini Head Offsets

Stored in `stPositionParam::dHeadOffset[2][4]`:
- Dimension 1: Head index (0=HEAD1, 1=HEAD2)
- Dimension 2: Mode (0=WAD, 1=Angle, 2=Vision, 3=Color)
- Each has X, Y, Z components

### Probe Offsets

- `dProbeXOffsetPos[5]` - X offset per probe
- `dProbeYOffsetPos[5]` - Y offset per probe
- `dProbeTOffsetPos[5]` - Theta offset per probe

---

## 18. Speed / Acceleration / Deceleration

### Speed Grades (`speed_type_t`)

| Grade | Enum | Vel | Acc | Dec/Scv |
|-------|------|-----|-----|---------|
| HIGH | `SPEED_HIGH = 0` | `dVelH` | `dAccH` | `dScvH` |
| MID | `SPEED_MID = 1` | `dVelM` | `dAccM` | `dScvM` |
| LOW | `SPEED_LOW = 2` | `dVelL` | `dAccL` | `dScvL` |
| HOME | `SPEED_HOME = 3` | `dVelHome` | `dAccHome` | `dScvHome` |
| LATCH | `SPEED_LATCH = 4` | `dVelLatch` | `dAccH` | `dScvH` |
| MULTI | `SPEED_MULTI = 5` | `dVelH * 1.5` | `dAccH * 1.5` | `dScvH` |

### Speed Modes (`smode_type_t`)

| Mode | Value | Description |
|------|-------|-------------|
| `SMODE_KEEP` | -1 | Keep current speed profile |
| `SMODE_C` | 0 | Constant velocity (no profile) |
| `SMODE_T` | 1 | Trapezoidal profile |
| `SMODE_S` | 2 | S-curve profile |

### `CMotorControl_ACS::SetSpeed(int iAxis, int iSpeedType, int iSpeedMode)`

1. Selects speed/acc/dec values from `CAxisType::SetSpeed(iSpeedType)`
2. Calls `SetSpeedDirect()` with those values

### `CMotorControl_ACS::SetSpeedDirect()`

Applies speed to ACS controller via 5 API calls:
```cpp
acsc_SetVelocity(m_hAcs, hwAxis, vel, NULL)
acsc_SetAcceleration(m_hAcs, hwAxis, acc, NULL)
acsc_SetDeceleration(m_hAcs, hwAxis, acc, NULL)    // same as acceleration
acsc_SetKillDeceleration(m_hAcs, hwAxis, jerk, NULL)
acsc_SetJerk(m_hAcs, hwAxis, jerk, NULL)
```

### Stepper Pulse Conversion (Type 2)

When `iStepper == 2`, all speed/position values are converted:
```
convertedValue = (dPulse * value) / dEncL
```

### Default Values (from AxisType constructor)

| Parameter | Default |
|-----------|---------|
| VelH | 500 |
| VelM | 200 |
| VelL | 100 |
| VelHome | 50 |
| VelLatch | 50 |
| AccH | 100 |
| AccM | 80 |
| AccL | 50 |
| ScvH | 50 |
| ScvM | 40 |
| ScvL | 25 |

---

## 19. Motor Status Monitoring

### `CMotorControl_ACS::GetMotorStatus(int iAxis, int nStatus)`

Reads status from ACS controller and maps to enum `motor_status_t`:

| Status | ACS API | ACS Flag | Description |
|--------|---------|----------|-------------|
| `STATE_MOT_ENABLE` | `acsc_GetMotorState()` | `ACSC_MST_ENABLE` | Motor enabled |
| `STATE_MOT_DISABLE` | `acsc_GetMotorState()` | `!ACSC_MST_ENABLE` | Motor disabled |
| `STATE_MOT_STOPMOT` | `acsc_GetMotorState()` | `!ACSC_MST_MOVE` | Motor stopped |
| `STATE_MOT_INPOSIT` | `acsc_GetMotorState()` | `ACSC_MST_INPOS` | In position |
| `STATE_MOT_LMTNEG` | `acsc_GetFault()` | `ACSC_SAFETY_LL` | Negative limit |
| `STATE_MOT_LMTPOS` | `acsc_GetFault()` | `ACSC_SAFETY_RL` | Positive limit |
| `STATE_MOT_ERRFLOW` | `acsc_GetFault()` | `ACSC_SAFETY_CPE` | Following error |
| `STATE_MOT_HOMDONE` | `acsc_ReadInteger("HomeFlag")` | Controller variable | Home complete |

### Additional Status APIs

| Function | ACS API | Returns |
|----------|---------|---------|
| `GetMoving()` | `acsc_GetMotorState()` | Raw motor state bits |
| `GetAmpEnable()` | via `GetMotorStatus(STATE_MOT_ENABLE)` | Enable state |
| `GetActualVel()` | `acsc_GetRVelocity()` | Real velocity |
| `GetFollowingErr()` | `acsc_ReadReal("FPOS")` | Feedback position (following error) |
| `GetMotorRMSValue()` | `acsc_ReadReal("RMS")` | RMS current |
| `GetSpeedVal()` | `acsc_GetVelocity()` | Current velocity setting |

---

## 20. ACS SPiiPlus API Usage Summary

### Connection APIs

| API | Where Used | Purpose |
|-----|-----------|---------|
| `acsc_GetConnectionsList()` | `InitDevice()` | Enumerate connections |
| `acsc_TerminateConnection()` | `InitDevice()` | Kill stale connections |
| `acsc_OpenCommEthernetTCP()` | `InitDevice()` | Connect via TCP |
| `acsc_CloseComm()` | Destructor | Disconnect |

### Motion APIs

| API | Where Used | Purpose |
|-----|-----------|---------|
| `acsc_ToPoint()` | `MoveToPoint()` | Single-axis point-to-point move |
| `acsc_ToPointM()` | `MoveCmdPosMulti()` | Multi-axis coordinated move |
| `acsc_Jog()` | `MoveJog()` | Continuous jog |
| `acsc_Halt()` | `StopDevice()` | Controlled stop |
| `acsc_RunBuffer()` | `MoveHome()`, `RunBuffer()` | Run ACSPL+ program buffer |
| `acsc_StopBuffer()` | `StopBuffer()` | Stop ACSPL+ program buffer |
| `acsc_WaitMotionEnd()` | `WaitMotionEnd()` | Wait for motion completion |

### Enable/Disable APIs

| API | Where Used | Purpose |
|-----|-----------|---------|
| `acsc_Enable()` | `ResetDevice()` | Enable motor drive |
| `acsc_Disable()` | `KillDevice()`, `ResetDevice()` | Disable motor drive |
| `acsc_KillAll()` | `ResetDevice()`, `KillDevice()` | Kill all axes |
| `acsc_DisableAll()` | `ResetDevice()`, `KillDevice()` | Disable all axes |
| `acsc_FaultClear()` | `ResetDevice()` | Clear fault |

### Parameter APIs

| API | Where Used | Purpose |
|-----|-----------|---------|
| `acsc_SetVelocity()` | `SetSpeedDirect()` | Set max velocity |
| `acsc_SetAcceleration()` | `SetSpeedDirect()` | Set acceleration |
| `acsc_SetDeceleration()` | `SetSpeedDirect()` | Set deceleration |
| `acsc_SetKillDeceleration()` | `SetSpeedDirect()` | Set emergency deceleration |
| `acsc_SetJerk()` | `SetSpeedDirect()` | Set jerk |
| `acsc_SetFPosition()` | `ResetDevice()`, `CheckHomeDoneLoop()` | Set feedback position |
| `acsc_GetFPosition()` | `GetCurPos()` | Read feedback position |
| `acsc_GetVelocity()` | `GetSpeedVal()` | Read velocity setting |
| `acsc_GetRVelocity()` | `GetActualVel()` | Read real velocity |

### Status APIs

| API | Where Used | Purpose |
|-----|-----------|---------|
| `acsc_GetMotorState()` | `GetMotorStatus()`, `CheckMoveDone()`, `CheckHomeDoneLoop()` | Motor state flags |
| `acsc_GetFault()` | `GetMotorStatus()`, `GetHWLimit()` | Fault/safety flags |
| `acsc_GetMotorError()` | `GetMotorStatus()` | Motor error code |
| `acsc_GetLastError()` | Multiple | Get last API error |
| `acsc_GetErrorString()` | `GetLastErrorString()` | Error description |

### I/O and Variable APIs

| API | Where Used | Purpose |
|-----|-----------|---------|
| `acsc_ReadInteger()` | `CheckInterlock()`, `CheckHomeDoneLoop()`, `GetMotorStatus()` | Read controller integer variable |
| `acsc_WriteInteger()` | `SetWriteBuffer()` | Write controller integer variable |
| `acsc_ReadReal()` | `GetFollowingErr()`, `GetMotorRMSValue()` | Read controller real variable |
| `acsc_GetInput()` | `GetInput()` | Read digital input |

---

## 21. Thread Safety

`CMotorControlMgr` uses 4 `CCriticalSection` objects for thread safety:

| Critical Section | Used By |
|-----------------|---------|
| `m_cs` | `ResetDevice`, `MoveHome`, `MoveCmdPosA_Move`, `MoveCmdPosMulti_Move`, `RunBuffer`, `LinkDatFile`, `SetMaxSWLimit`, `SetMinSWLimit` |
| `m_cs_Get` | All `Get*()` functions |
| `m_cs_Set` | All `Set*()` functions, `SetSpeed`, `SetSpeedDirect`, `SetAmpEnable`, `SetAxisMapping` |
| `m_cs_done` | `CheckMoveDone`, `CheckHomeDone`, `CheckInterlock` |
| `m_cs_Stop` | `StopHome`, `StopDevice`, `KillDevice`, `StopBuffer` |

---

## 22. Out-of-Service (Simulation) Mode

When `m_bOutOfService == true`:
- No hardware communication occurs
- Positions stored in `m_dMovePos[100]` array
- Speed-based delay simulates move time: `Sleep(dSpeed * steps)`
- SW limits still enforced
- `CheckMoveDone()` always returns `true`
- `IsInitialized()` always returns `true`
- Probe master-slave tracking maintained in simulation

---

## 23. Logging

All motion operations are logged via `CLogFileAccessor`:
- Log path: `{LogPath}/Control/Motion/Motion.log`
- Log levels: `SYST` (system), `WARN` (warning), `ERRO` (error)
- Each MoveCmdPosA/R/Multi logs Start (with axis name, position, speed) and End (with result code)

---

## 24. Error Codes (Motion-Related)

Referenced error codes from `AuroraFuncDefine.h` (used in interlock returns):

| Constant | Usage |
|----------|-------|
| `df_MOTION_RV_SUCCESSED` (1) | Success |
| `df_MOTION_RV_ERROR` (0) | General error |
| `VTM_RV_SUCCESSED` | Success (alternate) |
| `MEI_NOT_CREATED` | Controller not connected |
| `MOTION_CMDPOS_ERROR` | Move command failed |
| `MOTION_MOVE_ALARM + axis` | Move failed for specific axis |
| `MOTION_MOVE_DONE_ALARM + axis` | Move done timeout |
| `MOTION_INTERLOCK_1` | Loader Z down interlock |
| `MOTION_INTERLOCK_3` | Loader Z safety interlock |
| `MOTION_INTERLOCK_5` | Probe unit collision |
| `MOTION_INTERLOCK_7` | General collision interlock |
| `MOTION_INTERLOCK_8` | Probe Z contact / Head Z interlock |
| `MOTION_INTERLOCK_CHECK_ALARM` | SW limit violation |
| `MOTION_ROTATE_POS_ERROR` | Loader rotate position error |
| `MTOR_MOVE_DONE_TIME_OUT_ALARM + axis` | Done check timeout |
| `MTOR_HOME_FAIL_ALARM` | Homing failure |
| `MEI_INIT_MC_FAIL` | Motor controller init failure |
