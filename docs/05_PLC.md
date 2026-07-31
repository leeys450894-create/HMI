# 05. PLC Communication System Analysis

## 1. Overview

The ALPHA HMI uses **Mitsubishi MELSEC MC Protocol** over TCP/IP (CC-Link Industrial Ethernet) for PLC communication. The system uses multiple TCP/IP connections for different device address areas (M, F, D, PACC, ProbeCard, GlassSensor), each managed by a dedicated scanner thread.

### Key Characteristics
- **Protocol:** Mitsubishi MELSEC MC Protocol (Binary frame over TCP/IP)
- **Transport:** TCP/IP sockets (client mode, default port 2001)
- **Encoding:** Unicode (TCHAR-based commands)
- **Polling:** Continuous cyclic read via background threads per scanner
- **Address Indirection:** `PLCMAdress.csv` maps logical enum indices to actual PLC M-addresses

---

## 2. Architecture & Class Hierarchy

```
CIOMapImpl (abstract base)                    IOMapImpl.h
  +-- CMcProtocolManager                      McProtocol/McProtocolManager.h/.cpp
        |-- m_vecpAddressScanner[]: vector of CMcAddressScannerImpl*
        |     +-- CMcMaddressScanner (M area read/write)    McMaddressScanner.h/.cpp
        |     +-- CMcFaddressScanner (F alarm area)          McFaddressScanner.h/.cpp
        |     +-- CMcDaddressScanner (D area / FDC)          McDaddressScanner.h/.cpp
        |     +-- CMcGaddressScanner (Glass sensor D area)   McGaddressScanner.h/.cpp
        |     +-- CMcPaddressScanner (PACC M area)           McPaddressScanner.h/.cpp
        |     +-- CMcCaddressScanner (Probe card D area)     McCaddressScanner.h/.cpp
        |-- m_piPLCMAdr[2000]: address indirection table
        +-- m_bVirtualBit[2000]: virtual simulation bits

CIOFunction (facade)                          IOMapFunction.h
  +-- wraps CIOMapImpl*

CIOMapControlMgr (application manager)        Manager/IOMapControlMgr.h/.cpp
  +-- CIOFunction* m_pFunction
  +-- m_piPLCMAdr[2000]: address indirection (duplicate)
  +-- Thread-safe via CCriticalSection m_cs_set[4]
```

### Source File Locations

| File | Path | Purpose |
|------|------|---------|
| DefinesPLC.h | `Source/Include/DefinesPLC.h` | Legacy PLC address constants (multi-EQ-type enums) |
| IOMapAddressDefine.h | `Source/ALPHA/Manager/IOMapAddressDefine.h` | Active plc_MBIT_t enum (~500 logical constants), CC-Link IO board enums |
| IOMapControlMgr.cpp/.h | `Source/ALPHA/Manager/IOMapControlMgr.cpp` | Application-level PLC manager (SubMgr) |
| DataStructure.h | `CoreFunction/IOMapFunction/DataStructure.h` | MC Protocol constants, device enums, address ranges |
| McProtocolManager.cpp/.h | `CoreFunction/IOMapFunction/McProtocol/McProtocolManager.cpp` | Multi-scanner manager, INI config, address routing |
| McMaddressScanner.cpp/.h | `CoreFunction/IOMapFunction/McProtocol/McMaddressScanner.cpp` | M-area read/write implementation |
| McFaddressScanner.cpp/.h | `CoreFunction/IOMapFunction/McProtocol/McFaddressScanner.cpp` | F-alarm read implementation |
| McAddressScannerImpl.h | `CoreFunction/IOMapFunction/McProtocol/McAddressScannerImpl.h` | Abstract scanner base (inherits CCommEthFunction) |
| PLCMAdress.csv | `Source/ALPHA/ConfigurationFile/CoreFunction/IOMapConfig/PLCMAdress.csv` | Logical-to-physical M-address mapping table |
| M.csv | `CoreFunction/IOMapConfig/M.csv` | M-address descriptions (loaded at init) |
| F.csv | `CoreFunction/IOMapConfig/F.csv` | F-address (alarm) descriptions |
| FunctionModuleEQState.cpp/.h | `Source/ALPHA/FunctionModule/FunctionModuleEQState.cpp` | Equipment state queries via PLC bits |
| FunctionModuleEQInterlock.cpp/.h | `Source/ALPHA/FunctionModule/FunctionModuleEQInterlock.cpp` | Motion interlock checks via PLC bits |

---

## 3. Communication Structure

### 3.1 Connection Configuration

Configuration is loaded from `CoreFunction/CoreFunction.ini` under the `IO_MAP_CONTROL_MGR` section. Each device area has independent TCP/IP settings:

| Device | INI Keys | Default IP | Default Port | Station# |
|--------|----------|-----------|-------------|----------|
| M Read | `M_TCPIP_IP`, `M_TCPIP_PORT` | 127.0.0.1 | 2001 | 2 |
| M Write | `M_W_TCPIP_IP`, `M_W_TCPIP_PORT` | 127.0.0.1 | 2001 | 2 |
| F Alarm | `F_TCPIP_IP`, `F_TCPIP_PORT` | 127.0.0.1 | 2001 | 3 |
| D (FDC) | `D_TCPIP_IP`, `D_TCPIP_PORT` | 127.0.0.1 | 2001 | 6 |
| PACC | `P_TCPIP_IP`, `P_TCPIP_PORT` | 127.0.0.1 | 2001 | 1 |
| ProbeCard | `C_TCPIP_IP`, `C_TCPIP_PORT` | 127.0.0.1 | 2001 | 5 |
| GlassSensor | `G_TCPIP_IP`, `G_TCPIP_PORT` | 127.0.0.1 | 2001 | 4 |

Each device has `USE` and `OUT_OF_SERVICE` flags. When `OUT_OF_SERVICE=true`, the scanner uses virtual bits (`m_bVirtualBit[]`) for simulation without actual PLC hardware.

### 3.2 Scanner Architecture

Each `CMcAddressScannerImpl` subclass:
1. Inherits from `CCommEthFunction` (TCP/IP socket wrapper)
2. Runs a background thread (`ThreadRun()` called cyclically via `CMcAddressScannerImpl::Start()`)
3. Sends a batch read command (`SendCommand()`) on each cycle
4. Receives response via `xNetCallbackRecieveData()` callback
5. Parses response and updates internal read data arrays
6. The M-area scanner pauses thread during writes (`m_bThreadRunStop`)

### 3.3 Initialization Flow

```
CIOMapControlMgr constructor
  --> CIOFunction(logFile, deviceType, configPath, iniPath, category)
    --> CMcProtocolManager(logFile, configPath, filePath, category)
      --> LinkIniFile(): load IP/port/use from INI
      --> Load CSV address map files (M.csv, F.csv, ProbeCard.csv, SACC.csv)
      --> ReadAdressFileCSV(PLCMAdress.csv): load address indirection table
      --> Create scanner instances:
          - CMcMaddressScanner (READ, "PLC_READ")
          - CMcMaddressScanner (WRITE, "PLC_WRITE", writeOnly=true)
          - CMcFaddressScanner (F alarm)
          - CMcCaddressScanner (ProbeCard, if USE=true)
          - CMcPaddressScanner (PACC, if USE=true)
      --> InitDevice():
          - ReadFileCSV() for each scanner
          - Start() background thread for each scanner
          - Initialize() TCP/IP connection for each scanner
```

---

## 4. MC Protocol Frame Format

### 4.1 Protocol Constants (DataStructure.h)

```cpp
// Read commands
PLC_CMD_ETHERNET_SINGLE_READ     = "500000FF03FF000018009004010001"   // Bit single read
PLC_CMD_ETHERNET_WORD_READ_D     = "500000FF03FF000018009004010000"   // Word read (D device)
PLC_CMD_ETHERNET_WORD_READ       = "500000FF03FF0000180090040300000100" // Word batch read

// Write commands
PLC_CMD_ETHERNET_SINGLE_WRITE    = "500000FF03FF000019009014010001"   // Bit single write

// Response success header
PLC_CMD_SUCCESS                  = "D00000FF03FF0000040000"

// Header length
PLC_CMD_HEADER_LENGTH            = 22
```

### 4.2 Read Command Construction (McMaddressScanner::SendCommand)

**Bit mode read:**
```
"500000FF03FF000018009004010001" + "M*004000" + "03E8"
 |SubHeader|Net#|PC#|Dest|Len|Timeout|Cmd|SubCmd| DevAddr  | Count(1000 dec=0x3E8)
```

**Word mode read:**
```
"500000FF03FF000018009004010000" + "M*004000" + count_hex
```

The M-area scanner reads `dfMADDRESS_TOTAL_COUNT` (1000) bits starting from `dfMADDRESS_RD_START` (M4000).

### 4.3 Write Command Construction (McMaddressScanner::SetMemoryBit)

```
"500000FF03FF00" + dataLen(4hex) + "0090" + "1401" + "0001" + "M*" + addr(6dec) + bitCount(4hex) + bitValues
 |SubHeader     | |DataLength  | |Timeout| |Write| |SubCmd| |Device addr         | |Count       | |Data   |
```

### 4.4 Word Write (McMaddressScanner::SetMemoryWord)

Uses D-device addressing (`D*nnnnnn`), writes float values as two 16-bit words (low/high).

### 4.5 String Write (McMaddressScanner::SetMemoryStr)

Writes character strings to D-device registers, packing 2 characters per word (byte-swapped).

### 4.6 Response Parsing (xNetCallbackRecieveData)

**Bit mode response:**
- Response body = 1000 ASCII characters ('0' or '1'), one per M-bit
- Direct index mapping: position 0 = M4000, position 999 = M4999

**Word mode response:**
- Response body = hex-encoded 16-bit words
- Parsed via hex-to-binary conversion, 4 bits per hex digit
- Word order: H/L swapped before extraction

---

## 5. Device Memory Map

### 5.1 Address Ranges (DataStructure.h)

| Area | Device | Start | End | Count | Purpose |
|------|--------|-------|-----|-------|---------|
| M Read | M | 4000 | 4499 | 500 | PLC-to-PC input bits |
| M Write | M | 4500 | 4999 | 500 | PC-to-PLC output bits |
| F Alarm | F | 0 | 1999 | 2000 | PLC F-alarm bits |
| PACC | M | 3900 | 3999 | 100 | PACC interface (50 read + 50 write) |
| ProbeCard | D | 2100 | 2499 | 400 | Probe card name/buffer/slot data |
| D (FDC) | D | 10000 | 12499 | 2500 | FDC data |
| GlassSensor | D | 1501 | 1599 | 99 | Glass sensor interface |

### 5.2 Address Indirection System

The `PLCMAdress.csv` file provides a **mapping table** from sequential enum index to actual PLC M-address. This allows the software to use logical enum constants (e.g., `PLC_BIT_STATE_READY` = enum 0) which are mapped at runtime to actual PLC addresses (e.g., M4000).

The mapping is loaded into `m_piPLCMAdr[2000]` array. When accessing a PLC bit:
1. If index >= 4000: `nBit = m_piPLCMAdr[index - 4000]`
2. If mapped value < 3000: invalid (returns -1)
3. If mapped value = 9999: unused/disabled signal
4. Otherwise: use mapped value as actual PLC M-address

---

## 6. PLC Address Constants (plc_MBIT_t enum)

### 6.1 Equipment State (M4000-M4009)

| Constant | CSV Address | Purpose |
|----------|-------------|---------|
| `PLC_BIT_STATE_READY` | M4000 | Equipment READY state |
| `PLC_BIT_STATE_LOAD` | M4001 | Equipment LOAD state |
| `PLC_BIT_STATE_MEASURE` | M4002 | Equipment MEASURE state |
| `PLC_BIT_STATE_ALARM` | M4008 | Alarm state |
| `PLC_BIT_STATE_WARN` | M4009 | Warning state |

### 6.2 State Transition Commands (M4500-M4506)

| Constant | CSV Address | Purpose |
|----------|-------------|---------|
| `PLC_BIT_INITIALIZE` | M4500 | Initialize command |
| `PLC_BIT_INITIALIZE_COMPLETE` | M4005 | Initialize complete feedback |
| `PLC_BIT_READY2LOAD_START` | M4501 | READY -> LOAD transition |
| `PLC_BIT_LOAD2MEAS_START` | M4502 | LOAD -> MEASURE transition |
| `PLC_BIT_MEAS2READY_START` | M4503 | MEASURE -> READY transition |
| `PLC_BIT_READY2MEAS_START` | M4504 | READY -> MEASURE transition |
| `PLC_BIT_MEAS2LOAD_START` | M4505 | MEASURE -> LOAD transition |
| `PLC_BIT_LOAD2READY_START` | M4506 | LOAD -> READY transition |

### 6.3 Alarm & Reset (M4509, M4920, M4925)

| Constant | CSV Address | Purpose |
|----------|-------------|---------|
| `PLC_BIT_ALRAM_RESET_STATUS` | M4509 | Alarm reset status |
| `PLC_BIT_EQ_BUZZER` | M4920 | Equipment buzzer control (Write) |
| `PLC_BIT_SIG_TOWER_MELODY` | M4925 | Signal tower buzzer (Write) |

### 6.4 Signal Tower (M4930-M4932)

| Constant | CSV Address | Purpose |
|----------|-------------|---------|
| `PLC_BIT_SIG_TOWER_R` | M4930 | Signal tower RED |
| `PLC_BIT_SIG_TOWER_Y` | M4931 | Signal tower YELLOW |
| `PLC_BIT_SIG_TOWER_G` | M4932 | Signal tower GREEN |
| `PLC_BIT_SIG_TOWER_B` | 9999 | Signal tower BLUE (unused) |

### 6.5 Door Sensors (M4020-M4030 area)

| Constant | CSV Address | Purpose |
|----------|-------------|---------|
| `PLC_BIT_DOOR_SENSOR1` | M4020 | Door 01 open (Left Front) |
| `PLC_BIT_DOOR_SENSOR2` | M4021 | Door 02 open (Right Front) |
| `PLC_BIT_DOOR_SENSOR3` | M4022 | Door 03 open (Right Back) |
| `PLC_BIT_DOOR_SENSOR4` | M4023 | Door 04 open (Left Back) |
| `PLC_BIT_DOOR_SENSOR5` | M4024 | Door 05 open (SACC Lower) |
| `PLC_BIT_DOOR_SENSOR6` | M4025 | Door 06 open (SACC Upper) |
| `PLC_BIT_DOOR_UNLOCK` | M4510 | Door unlock command (Write) |
| `PLC_BIT_DOOR_UNLOCK01`..`08` | M4030+ | Door unlock status per door |

### 6.6 EMO (Emergency) Switches (M4100+)

| Constant | CSV Address | Purpose |
|----------|-------------|---------|
| `PLC_BIT_EMERGENCY_SWITCH01`..`16` | varies | EMO switch status (up to 16 switches) |
| `PLC_R_EMO_LOADER_State01`..`03` | varies | Loader EMO state |

### 6.7 Power & Circuit Breakers (M4070+)

| Constant | CSV Address | Purpose |
|----------|-------------|---------|
| `PLC_BIT_MAIN_GPS_BREAKER_ON_01` | M4070 | GPS main breaker ON |
| `PLC_BIT_MAIN_UPS_BREAKER_ON_01` | M4071 | UPS main breaker ON |
| `PLC_BIT_MAIN_GPS_CIRCUIT_BREAKER_TRIP` | M4072 | GPS breaker trip alarm |
| `PLC_BIT_MAIN_UPS_CIRCUIT_BREAKER_TRIP` | M4073 | UPS breaker trip alarm |
| `PLC_BIT_SMPS1_CP_ON` | M4077 | SMPS1 CP ON |
| `PLC_BIT_MC_CONTACT_01`..`10` | M4080+ | MC contactor status (EQ/EFU/Ionizer/SACC) |
| `PLC_BIT_SAFETY_PLC_CP_ON` | varies | Safety PLC CP ON |
| `PLC_BIT_MC_CP_ON` | varies | MC CP ON |

### 6.8 Glass Sensors (M4090+)

| Constant | CSV Address | Purpose |
|----------|-------------|---------|
| `PLC_BIT_GLASS_EXIST_01` | M4090 | Glass on lift pin 01 |
| `PLC_BIT_GLASS_EXIST_02` | 9999 | Glass on lift pin 02 (unused) |
| `PLC_BIT_GLASS_EXIST_03` | M4095 | Glass on sample stage 01 |
| `PLC_BIT_GLASS_EXIST_04` | 9999 | Glass on sample stage 02 (unused) |
| `PLC_BIT_GLASS_SHORT_01`..`08` | 9999 | Glass edge short sensors (all unused in this config) |

### 6.9 Safety / Unloading Sensors (M4170+)

| Constant | CSV Address | Purpose |
|----------|-------------|---------|
| `PLC_BIT_UNLOAD_SENSOR01` | M4170 | Head Gantry 1 unloading position |
| `PLC_BIT_UNLOAD_SENSOR02` | M4171 | Head Gantry 2 unloading position |
| `PLC_BIT_UNLOAD_SENSOR03` | M4172 | Sample plate safety position |
| `PLC_BIT_UNLOAD_HEAD_SENSOR01`..`04` | M4173-M4176 | Head Y01-Y04 unloading sensors |
| `PLC_BIT_UNLOAD_PROBE_SENSOR01`..`05` | M4490-M4494 | Probe 01-05 unloading sensors |
| `PLC_LOADER_ARM_SENSOR_01` | M4180 | Loader arm detect (area sensor) |
| `PLC_BIT_ARM_DETECT_INTERLOCK_SR` | M4185 | Arm detect interlock SR(150) |

### 6.10 Run/Teach Mode

| Constant | CSV Address | Purpose |
|----------|-------------|---------|
| `PLC_BIT_RUN_TEACH_SWITCH` | M4190 | Run/Teach mode switch |
| `PLC_BIT_HAND_GRIP` | M4285 | Hand grip switch |

### 6.11 Lift Pin Sensors

| Constant | CSV Address | Purpose |
|----------|-------------|---------|
| `PLC_BIT_LIFT_PIN_SENSOR_DOWN01` | varies | Lift pin 01 down sensor |
| `PLC_BIT_LIFT_PIN_SENSOR_UP01` | varies | Lift pin 01 up sensor |
| `PLC_BIT_LIFT_PIN_UPDOWN` | varies | Lift pin up/down command (Write) |

### 6.12 Vacuum System

| Constant | CSV Address | Purpose |
|----------|-------------|---------|
| `PLC_BIT_VACCUM_PRES_SW01`..`08` | varies | Vacuum pressure switch sensors |
| `PLC_BIT_VACCUM_PLATE01`..`06` | varies | Vacuum plate on sensors |
| `PLC_BIT_VACCUM_ON01`..`06` | varies | Vacuum on commands (Write) |
| `PLC_BIT_CHUCK_VACCUM_01`..`06` | varies | Chuck vacuum sensors |
| `PLC_BIT_CHUCK_VACCUM_ON_01`..`06` | varies | Chuck vacuum on commands (Write) |

### 6.13 Air / Blowing System

| Constant | CSV Address | Purpose |
|----------|-------------|---------|
| `PLC_BIT_AIR_PRESSURE_01`..`27` | varies | Air pressure sensors (main, fixture, probe) |
| `PLC_BIT_AIR_BLOW_01`..`10` | varies | Air blow on/off |
| `PLC_BIT_AIR_BLOW_PLATE_01`..`10` | varies | Plate air blow |
| `PLC_BIT_CHUCK_AIRBLOWING_ON_01`..`06` | varies | Chuck air blowing (Write) |

### 6.14 Fixture Control

| Constant | CSV Address | Purpose |
|----------|-------------|---------|
| `PLC_BIT_FIXTURE_FRONT_F01`..`F02` | varies | Front fixture FWD sensors |
| `PLC_BIT_FIXTURE_*_R01`..`R02` | varies | Fixture REV sensors (per side) |
| `PLC_BIT_FIXTURE_*U01`..`U02` | varies | Fixture UP sensors |
| `PLC_BIT_FIXTURE_*D01`..`D02` | varies | Fixture DOWN sensors |
| `PLC_BIT_FIXTURE_W_F_F`..`L` | varies | Fixture FWD/REV commands (Write) |
| `PLC_BIT_FIXTURE_W_U_F`..`L` | varies | Fixture UP/DOWN commands (Write) |

### 6.15 Shutter

| Constant | CSV Address | Purpose |
|----------|-------------|---------|
| `PLC_BIT_SUTTER_SENSOR_CLOSE` | varies | Shutter close sensor |
| `PLC_BIT_SUTTER_SENSOR_OPEN` | varies | Shutter open sensor |
| `PLC_BIT_W_SUTTER_CLOSE_MOVE` | varies | Shutter close command (Write) |
| `PLC_BIT_W_SUTTER_OPEN_MOVE` | varies | Shutter open command (Write) |

### 6.16 CCD / Optical Components

| Constant | CSV Address | Purpose |
|----------|-------------|---------|
| `PLC_BIT_CCD_POWER00`..`03_ONOFF` | varies | CCD camera power (Write) |
| `PLC_BIT_LAMP_FOCUS01`..`04` | varies | Lamp focus control (Write) |
| `PLC_BIT_MAIN_LAMP_SHTTER_OPEN/CLOSE_01` | varies | Main lamp shutter |
| `PLC_BIT_LAMP_OD01` | varies | OD lamp shutter |
| `PLC_BIT_LAMP_TR01`..`03` | varies | TR lamp shutters |
| `PLC_BIT_BEAM_SPLITTER_*` | varies | Beam splitter mode select (6K/8K/50x/75x) |

### 6.17 RS Pico / Roughness

| Constant | CSV Address | Purpose |
|----------|-------------|---------|
| `PLC_BIT_RSPICO_SENSOR_READY` | varies | RS Pico ready sensor |
| `PLC_BIT_RSPICO_SENSOR_MEAS` | varies | RS Pico measure sensor |
| `PLC_BIT_RSPICO_UPDOWN` | varies | RS Pico up/down command (Write) |
| `PLC_BIT_RSPICO_ONOFF` | varies | RS Pico power on/off (Write) |

### 6.18 Ionizer System

| Constant | CSV Address | Purpose |
|----------|-------------|---------|
| `PLC_IONIZER_Ion_OnOFF` | varies | Ionizer on/off state |
| `PLC_IONIZER_Ion_Level_Alram01`..`03` | varies | Ionizer level alarm |
| `PLC_IONIZER_Condition_Alram01`..`03` | varies | Ionizer condition alarm |
| `PLC_IONIZER_Alram01`..`03` | varies | Ionizer alarm |
| `PLC_BIT_IONIZER_RUN_01`..`03` | varies | Ionizer run commands (Write) |
| `PLC_BIT_IONIZER_STOP_01`..`03` | varies | Ionizer stop commands (Write) |
| `PLC_BIT_XIONIZER_*` | varies | X-ray ionizer sensors and controls |

### 6.19 Temperature & Smoke

| Constant | CSV Address | Purpose |
|----------|-------------|---------|
| `PLC_BIT_TEMP_SENSOR_01`..`20` | varies | Temperature sensors (up to 20) |
| `PLC_BIT_TEMP_SENSOR_RESET_SWITCH_01`..`20` | varies | Temp reset switches |
| `PLC_BIT_SMOKE_DETECT_01`..`16` | varies | Smoke detectors |
| `PLC_BIT_SMOKE_DETECT_EMO_SR` | varies | Smoke detect EMO SR |

### 6.20 E-Box / PC-Box Fans

| Constant | CSV Address | Purpose |
|----------|-------------|---------|
| `PLC_BIT_EBOX_FAN_01`..`16` | varies | Electric box fan sensors |
| `PLC_BIT_PCBOX_FAN_01`..`10` | varies | PC box fan sensors |

### 6.21 Loader / Rotation Sensors

| Constant | CSV Address | Purpose |
|----------|-------------|---------|
| `PLC_MOTION_LOADER_DEGREE_0`..`3` | varies | Loader rotation degree sensors (0/90/180/270) |
| `PLC_R_MOTION_LOADER_DEGREE_0`..`7` | varies | Rotation sensors (R0/R1 stage) |
| `PLC_MOTION_LP_POS_UP/DOWN_01`..`04` | varies | Lift pin position sensors |

### 6.22 Probe Unit System (5 probe units)

| Constant Group | Purpose |
|---------------|---------|
| `PLC_R_MOTION_PROBE_UNIT0x_POS_01`..`04` | Probe unit x position sensors (UP/MID/DOWN) |
| `PLC_R_PROBE_UNIT0x_SAFETY_POS` | Probe unit x safety position |
| `PLC_R_PROBE_UNIT0x_CARD_CHK_01`..`04` | Probe card check sensors |
| `PLC_R_PROBE_UNIT0x_CARD_ZERO_01`..`05` | Probe card zero lock/unlock sensors |
| `PLC_R_PROBE_UNIT0x_PCB_LOCK/UNLOCK` | Probe PCB lock/unlock sensors |
| `PLC_R_PROBE_UNIT0x_CLAM_LOCK/UNLOCK` | Probe clamp lock/unlock sensors |
| `PLC_BIT_W_PROBE_CARD_ZERO_UNLOCK_01`..`05` | Probe zero unlock commands (Write) |
| `PLC_BIT_W_PROBE_PCBLOCK/UNLOCK_01`..`05` | Probe PCB lock/unlock commands (Write) |
| `PLC_BIT_W_PROBE_UNIT0x_CLAMP_ON/OFF_01` | Probe clamp on/off commands (Write) |

### 6.23 PACC (Probe Auto Change) Interface (M3900-M3999)

**Input signals (from PACC to EQ):**

| Constant | Purpose |
|----------|---------|
| `PLC_PACC_AUTO_MODE` | PACC auto mode status |
| `PLC_PACC_MANUAL_MODE` | PACC manual mode status |
| `PLC_PACC_ALARM` | PACC alarm |
| `PLC_PACC_STATUS_READY` | PACC ready status |
| `PLC_PACC_LOAD_BUFFER_POS_HIGH/LOW` | Load buffer Z position |
| `PLC_PACC_UNLOAD_BUFFER_POS_HIGH/LOW` | Unload buffer Z position |
| `PLC_PACC_COMMAND_CHECK_ACK` | Command ACK from PACC |
| `PLC_PACC_COMMAND_CHECK_NAK` | Command NAK from PACC |
| `PLC_PACC_STATUS_BUSY` | PACC busy (Gate/Z0 moving) |
| `PLC_PACC_ENTRY_REQUEST` | PACC arm entry request |
| `PLC_PACC_STATUS_COMPLETE_ACK` | PACC operation complete |
| `PLC_PACC_HANDFOLD_COMPLETE_ACK` | PACC hand fold complete |

**Output signals (from EQ to PACC):**

| Constant | Purpose |
|----------|---------|
| `PLC_EQ_AUTO_MODE` | EQ auto mode set |
| `PLC_EQ_MANUAL_MODE` | EQ manual mode set |
| `PLC_EQ_COMMAND_LOAD_SLOT01`..`04` | Load slot select |
| `PLC_EQ_COMMAND_UNLOAD_SLOT01`..`04` | Unload slot select |
| `PLC_EQ_COMMAND_LOADING` | Loading command |
| `PLC_EQ_COMMAND_UNLOADING` | Unloading command |
| `PLC_EQ_COMMAND_EXCHSANGE` | Exchange command |
| `PLC_EQ_COMMAND_START` | Command start trigger |
| `PLC_EQ_STATUS_BUSY` | EQ arm busy |
| `PLC_EQ_STATUS_COMPLETE` | EQ operation complete |

### 6.24 Probe Card D-Memory (D2100-D2499)

| Constant | D Address | Purpose |
|----------|-----------|---------|
| `PLC_PACC_EQ_UNLOAD` | D2100 | Unloading probe data |
| `PLC_PACC_EQ_PROBE_1`..`5` | D2110-D2150 | Probe 1-5 ID data (10 words each) |
| `PLC_PACC_BUFFER_SLOT_LOAD` | D2210 | Buffer load slot probe ID |
| `PLC_PACC_BUFFER_SLOT_UNLOAD` | D2220 | Buffer unload slot probe ID |
| `PLC_PACC_CST_LOAD` | D2400 | CST load data |
| `PLC_PACC_CST_SLOT_1`..`5` | D2410-D2450 | CST slot 1-5 probe ID |

### 6.25 Gate Valve / Chamber

| Constant | Purpose |
|----------|---------|
| `PLC_MAIN_GATE_VALVE_OPEN/CLOSE_01`..`02` | Main gate valve open/close sensors |
| `PLC_MAIN_GATE_VALVE_PUSH/PULL_01` | Gate valve push/pull sensors |
| `PLC_MAIN_GATE_VALVE_LOCK/UNLOCK_01`..`04` | Gate valve lock/unlock sensors |
| `PLC_PACC_GATE_VALVE_*` | PACC gate valve signals |
| `PLC_TM_GATE_OPEN` | TM gate open |

### 6.26 Purifier / N2 / CDA / Exhaust

| Constant | Purpose |
|----------|---------|
| `PLC_PURI_AIR_START/STOP` | EQ purifier air start/stop (Write) |
| `PLC_PURI_N2_START/STOP` | EQ purifier N2 start/stop (Write) |
| `PLC_PURI_QUICK_START/STOP` | Quick purge start/stop (Write) |
| `PLC_CDA_PURGE_OPEN/CLOSE_01` | CDA purge valve (Write) |
| `PLC_N2_PURGE_OPEN/CLOSE_01` | N2 purge valve (Write) |
| `PLC_BIT_EXHAUST_ANGLE_OPEN/CLOSE_01` | Exhaust angle valve (Write) |

### 6.27 Chuck System

| Constant | Purpose |
|----------|---------|
| `PLC_BIT_R_CHUCK_F01`..`06` | Chuck FWD sensors |
| `PLC_BIT_R_CHUCK_R01`..`06` | Chuck REV sensors |
| `PLC_BIT_R_CHUCK_U01`..`06` | Chuck UP sensors |
| `PLC_BIT_R_CHUCK_D01`..`06` | Chuck DOWN sensors |
| `PLC_BIT_R_CHUCK_P01`..`06` | Chuck position detect sensors |
| `PLC_BIT_W_CHUCK_M01`..`06` | Chuck move commands (Write) |
| `PLC_BIT_W_CHUCK_U01`..`06` | Chuck up/down commands (Write) |

### 6.28 Motion Interlock (Write)

| Constant | CSV Address | Purpose |
|----------|-------------|---------|
| `PLC_BIT_ACS_MOTION_INTERLOCK_1`..`4` | varies | ACS motion interlock outputs |

### 6.29 Half-Glass Support (A/B Half)

The system supports half-glass operations with duplicated state transition commands:
- `PLC_BIT_READY2LOAD_START_A`, `_B`, `_AB` (and other transitions)
- Half-glass fixture sensors per side (Front/Right/Back/Left) with `_HALF` suffix

---

## 7. Bit Read/Write Operations

### 7.1 Read Operations

**Primary API chain:**
```
CIOMapControlMgr::GetParam_DIOMapMemoryBitImport(index)
  --> address indirection: nBit = m_piPLCMAdr[index - 4000]
  --> CIOFunction::GetParam_DIOMapMemoryBitImport(nBit)
    --> CMcProtocolManager::GetParam_DIOMapMemoryBitImport(nBit)
      --> if OutOfService: return m_bVirtualBit[nBit - 4000]
      --> else: CMcMaddressScanner::GetAddressImportDataValue(nBit)
        --> return (m_lMcProtocolReadData_clone[nBit - 4000] == 1)
```

Read bits are continuously updated by the background thread and stored in `m_lMcProtocolReadData_clone[]`.

### 7.2 Write Operations

**Primary API chain:**
```
CIOMapControlMgr::SetMemoryBitToDevice(index, value)
  --> CCriticalSection lock (m_cs_set[0])
  --> CIOFunction::SetMemoryBitToDevice(index, value)
    --> CMcProtocolManager::SetMemoryBitToDevice(index, value)
      --> address indirection: nBit = m_piPLCMAdr[index - 4000]
      --> if OutOfService: m_bVirtualBit[nBit - 4000] = value
      --> else: SetMemoryNBitToDevice(strDeviceText, value, 1)
        --> CMcMaddressScanner::SetMemoryBit(addr, val, nBit)
          --> Build MC Protocol write command
          --> SendPacket(strCommand)
          --> Log to PLC_WRITE.log
```

### 7.3 Multi-bit Write

```
CIOMapControlMgr::SetMemoryNBitToDevice(index, value, lSize)
```
Writes multiple consecutive bits. The `value` parameter is treated as a bitmask where each bit position maps to consecutive M-addresses.

### 7.4 CheckActionDone (Wait for bit state)

```
CIOMapControlMgr::CheckActionDone(index, dwTimeout, value)
  --> CMcMaddressScanner::CheckActionDone(addr, dwTimeout, val)
    --> Poll loop: check GetAddressImportDataValue(addr) == val
    --> WaitForSingleObject(m_hWaitTimeEvent, 100ms) between checks
    --> Timeout: dwTimeout * 1000 ms
    --> Returns true if condition met, false on timeout
```

Used extensively in state transitions (e.g., `WaitActionDone` in `FunctionModuleEQState`).

---

## 8. Word Read/Write Operations

### 8.1 Word Write (D-device)

```
CIOMapControlMgr::SetMemoryWordToDevice(index, float value)
  --> CMcProtocolManager::SetMemoryWordToDevice(index, value)
    --> CMcMaddressScanner::SetMemoryWord(addr, &value, 1)
      --> Convert float to FLOAT_FIELD union (two 16-bit words)
      --> Build MC Protocol D-word write command
      --> SendPacket()
```

### 8.2 String Write (D-device)

```
CIOMapControlMgr::SetMemoryNStrToDevice(index, TCHAR* value, long lSize)
  --> CMcMaddressScanner::SetMemoryStr(addr, val, nWord)
    --> Pack characters 2 per word (byte-swapped: high byte = char[2i+1], low byte = char[2i])
    --> Build MC Protocol D-word write command
    --> SendPacket()
```

Maximum 100 characters per write operation.

---

## 9. F-Alarm System

The F-alarm area (F0-F1999) is monitored by `CMcFaddressScanner`:
- Reads 2000 F-device bits continuously
- `GetAddressAlarmCount()` returns count of active alarms
- `IsOccur_FAlarm()` returns true if any F-alarm is active
- Alarm data loaded from `F.csv` with descriptions
- Accessible via `CMcProtocolManager::GetFAlarmCount()` and `IsOccur_FAlarm()`

---

## 10. Equipment State Machine (via PLC)

`CFunctionModuleEQState` (`FunctionModuleEQState.cpp`) queries equipment state through PLC bits:

```
GetEQState()
  --> IsEQStateReady()  --> reads PLC_BIT_STATE_READY (M4000)
  --> IsEQStateMeas()   --> reads PLC_BIT_STATE_MEASURE (M4002)
  --> IsEQStateLoad()   --> reads PLC_BIT_STATE_LOAD (M4001)
```

Returns `PLC_MBIT_READY` (0), `PLC_MBIT_LOAD` (1), `PLC_MBIT_MEAS` (2), or -1 (abnormal).

### State Transition Flow (FunctionModuleEQStateChangeStep.cpp)

State transitions write multiple PLC bits atomically:

**READY state set:**
```cpp
SetMemoryBitToDevice(PLC_BIT_STATE_READY, true);
SetMemoryBitToDevice(PLC_BIT_STATE_LOAD, false);
SetMemoryBitToDevice(PLC_BIT_STATE_MEASURE, false);
WaitActionDone(PLC_BIT_STATE_READY, 1, 1, true, 60); // wait up to 60s
```

**LOAD state set:**
```cpp
SetMemoryBitToDevice(PLC_BIT_STATE_READY, false);
SetMemoryBitToDevice(PLC_BIT_STATE_LOAD, true);
SetMemoryBitToDevice(PLC_BIT_STATE_MEASURE, false);
WaitActionDone(PLC_BIT_STATE_LOAD, 1, 1, true, 60);
```

**MEASURE state set:**
```cpp
SetMemoryBitToDevice(PLC_BIT_STATE_READY, false);
SetMemoryBitToDevice(PLC_BIT_STATE_LOAD, false);
SetMemoryBitToDevice(PLC_BIT_STATE_MEASURE, true);
WaitActionDone(PLC_BIT_STATE_MEASURE, 1, 1, true, 60);
```

---

## 11. Safety Interlock System

`CFunctionModuleEQInterlock` (`FunctionModuleEQInterlock.cpp`) provides motion interlocks:

### 11.1 Interlock Check Functions

| Function | Checks | Purpose |
|----------|--------|---------|
| `GetInterlockMoveX()` | LDX, LDZ loading + Clamp fold/unfold | X-axis move safety |
| `GetInterlockMoveY()` | LDX, LDZ loading + Clamp fold/unfold | Y-axis move safety |
| `GetInterlockMoveZ()` | LDX, LDZ loading + Clamp fold/unfold | Z-axis move safety |
| `GetInterlockMoveLDX()` | Stage+Head safety, LDZ loading, LP up | Loader X move safety |
| `GetInterlockMoveLDY()` | Stage+Head safety, LDZ loading, LP up | Loader Y move safety |
| `GetInterlockMoveLDR()` | Stage+Head safety, LDZ loading, LP up | Loader R move safety |
| `GetInterlockMoveLDZ()` | Stage+Head safety, LDZ contact, LP up | Loader Z move safety |
| `GetinterlockIOLP()` | Stage+Head safety, LDX+LDZ loading | Lift pin I/O safety |
| `GetinterlockIOClamp()` | Stage+Head safety, LDX contact, LDZ loading | Clamp I/O safety |
| `GetinterlockIOVacuum()` | Stage+Head safety | Vacuum I/O safety |
| `GetinterlockIOAir()` | Stage+Head safety | Air I/O safety |

### 11.2 Safety Sensor Queries (FunctionModuleEQState.cpp)

| Function | PLC Bits Used | Purpose |
|----------|--------------|---------|
| `IsSafetySensorOnStage()` | `PLC_BIT_UNLOAD_SENSOR03` | Stage safety position |
| `IsSafetySensorOnHead()` | `PLC_BIT_UNLOAD_SENSOR01`..`02` | Gantry safety position |
| `IsGlassSensorOn()` | `PLC_BIT_GLASS_EXIST_01`..`04` | Glass presence on stage |
| `IsLiftpinSensorUp()` | `PLC_BIT_LIFT_PIN_SENSOR_UP01` | Lift pin up state |
| `IsLiftpinSensorDown()` | `PLC_BIT_LIFT_PIN_SENSOR_DOWN01` | Lift pin down state |
| `IsVacuumSensorOn()` | `PLC_BIT_VACCUM_PLATE01`..`02` | Vacuum engaged |
| `IsDoorSensorClose()` | `PLC_BIT_DOOR_SENSOR1`..n | All doors closed |
| `IsShutterSensorClose()` | `PLC_BIT_SUTTER_SENSOR_CLOSE` | Shutter closed |
| `IsLoaderArmSensorOn()` | `PLC_LOADER_ARM_SENSOR_01` | Loader arm detect (area sensor) |

### 11.3 IOMapControlMgr Interlock

`GetMoveInterlock()` performs a simplified interlock check (used outside the sequence engine):
- Checks `PLC_LOADER_ARM_SENSOR_01` (arm detect) -- returns `PLC_ARM_SENSOR_DETECT` error
- Checks `PLC_BIT_SUTTER_SENSOR_CLOSE` (shutter) -- returns `PLC_LOADER_SHUTTER_UP_ERROR` error

---

## 12. CC-Link IO Board System (Alternative I/O)

When `_USE_IOBOARD` is defined, the system uses CC-Link IO board (ADAM modules) instead of MC Protocol for certain I/O:

### 12.1 IO Board Inputs (ADAM-3951 + ADAM-3937)

| Enum | Description |
|------|-------------|
| `IO_BOARD_READ_INPUT_000` | UPS MAIN CIRCUIT BREAKER TRIP |
| `IO_BOARD_READ_INPUT_004` | SR 10 ON (EQ MOTION MC POWER) |
| `IO_BOARD_READ_INPUT_010` | LIGHT CURTAIN MUTING STATUS |
| `IO_BOARD_READ_INPUT_011` | LIGHT CURTAIN DETECTED |
| `IO_BOARD_READ_INPUT_016`..`017` | DOOR 01/02 OPEN (LEFT/RIGHT) |
| `IO_BOARD_READ_INPUT_022`..`025` | EMO SWITCH 01-04 PUSH |
| `IO_BOARD_READ_INPUT_028`..`029` | MR0 rotation position I/O |
| `IO_BOARD_READ_INPUT_102` | SELECT SWITCH AUTO MODE |
| `IO_BOARD_READ_INPUT_109` | VACUUM 01 PRESSURE NORMAL (STAGE) |
| `IO_BOARD_READ_INPUT_110` | VACUUM 02 PRESSURE NORMAL (ALIGN) |

### 12.2 IO Board Outputs (ADAM-3951)

| Enum | Description |
|------|-------------|
| `IO_BOARD_READ_OUTPUT_000` | BUZZER ON |
| `IO_BOARD_READ_OUTPUT_007`..`008` | CCD 01/02 ON |
| `IO_BOARD_READ_OUTPUT_011` | INTERLOCK S.R 150 ON |
| `IO_BOARD_READ_OUTPUT_012` | DOOR UNLOCK |
| `IO_BOARD_READ_OUTPUT_021` | SIGNAL TOWER RED |
| `IO_BOARD_READ_OUTPUT_022` | SIGNAL TOWER YELLOW |
| `IO_BOARD_READ_OUTPUT_023` | SIGNAL TOWER GREEN |
| `IO_BOARD_READ_OUTPUT_028`..`029` | SAMPLE PLATE VACUUM 01/02 |
| `IO_BOARD_READ_OUTPUT_030`..`031` | SAMPLE PLATE BLOWING 01/02 |

### 12.3 CC-Link System Bit Descriptions (IOMapAddressDefine.h)

Notable read bits with descriptions:
- Bit 017: 288M WORK SLIDE READY POSITION
- Bit 022: LOADING BUFFER GLASS CHECK 01
- Bit 025: CDA 01 PRESSURE NORMAL
- Bit 033: SMOKE DETECT 01 (FRONT EL. BOX)
- Bit 035-036: TEMP. OVER WARNING/ALARM
- Bit 041: EMERGENCY STATUS CHECK 01
- Bit 045: LOADING DOOR CLOSED
- Bit 049-052: DOOR 01-04 OPEN (FRONT/RIGHT/BACK/LEFT)
- Bit 053-056: DOOR 01-04 UNLOCK
- Bit 057-061: EMS SWITCH 01-05 PUSH
- Bit 065-076: ELECTRIC/PC BOX FAN ON sensors
- Bit 081: UPS1 MAIN CIRCUIT BREAKER TRIP
- Bit 083-084: SR 101/102 ON (EQ MOTION)
- Bit 085-086: SYSTEM MC 01/02 ON
- Bit 087: SAFETY PLC CP ON
- Bit 095: SELECT SWITCH AUTO MODE

---

## 13. Signal Tower Control

### Via MC Protocol (PLC)
```cpp
// Signal Tower bits (Write)
PLC_BIT_SIG_TOWER_R     = M4930  // Red
PLC_BIT_SIG_TOWER_Y     = M4931  // Yellow
PLC_BIT_SIG_TOWER_G     = M4932  // Green
PLC_BIT_SIG_TOWER_MELODY = M4925  // Buzzer melody
```

### Via IO Board (_USE_IOBOARD)
```cpp
SetSinalTowerAlarm()  --> OUTPUT_021=R, OUTPUT_022=off, OUTPUT_023=off   // RED
SetSinalTowerIdle()   --> OUTPUT_021=off, OUTPUT_022=Y, OUTPUT_023=off   // YELLOW
SetSinalTowerRun()    --> OUTPUT_021=off, OUTPUT_022=off, OUTPUT_023=G   // GREEN
SetSinalTowerWait()   --> OUTPUT_021=off, OUTPUT_022=Y, OUTPUT_023=G     // YELLOW+GREEN
```

---

## 14. Vacuum/Blowing Sequence (IOMapControlMgr)

### Stage Vacuum On (non-IO-board mode)
```
1. SetMemoryBitToDevice(PLC_BIT_VACCUM_ON01, true)
2. Poll loop (max 10 x 100ms):
   - Check PLC_BIT_VACCUM_PLATE01 == true AND PLC_BIT_VACCUM_PLATE02 == true
3. Return success when vacuum confirmed
```

### Stage Vacuum Off
```
1. SetMemoryBitToDevice(PLC_BIT_VACCUM_ON01, false)
2. Poll loop (max 10 x 100ms):
   - Check PLC_BIT_VACCUM_PLATE01 == false AND PLC_BIT_VACCUM_PLATE02 == false
3. Return success when vacuum released
```

---

## 15. Timeout Constants

| Constant | Value | Purpose |
|----------|-------|---------|
| `PLC_IO_ACTION_DONE_TIME` | 10 sec | Default action done timeout |
| `PLC_IO_ACTION_DONE_TIME_03` | 3 sec | Short timeout |
| `PLC_IO_ACTION_DONE_TIME_30` | 30 sec | Medium timeout |
| `PLC_IO_ACTION_DONE_TIME_60` | 60 sec | Long timeout (state transitions) |

---

## 16. Virtual/Simulation Mode

When `OUT_OF_SERVICE=true` for a device:
- No TCP/IP connection is established
- Read operations return values from `m_bVirtualBit[]` array
- Write operations store values in `m_bVirtualBit[]` array
- `CheckActionDone` returns virtual bit state after a delay (`Sleep(500 * dwTimeout)` or `Sleep(2000)`)
- Allows software testing without physical PLC hardware

Initial virtual values set in `CIOMapControlMgr` constructor:
```cpp
// All door sensors = false (closed)
// PLC_BIT_RUN_TEACH_SWITCH = true (run mode)
// PLC_LOADER_ARM_SENSOR_01 = true (no arm detected)
// PLC_BIT_SUTTER_SENSOR_CLOSE = true (shutter closed)
// PLC_R_PACC_SHUTTER_CLOSE_01/02 = true (PACC shutters closed)
```

---

## 17. Thread Safety

`CIOMapControlMgr` uses 4 separate `CCriticalSection` objects for concurrent access:
- `m_cs_set[0]`: single bit write operations
- `m_cs_set[1]`: multi-bit write operations
- `m_cs_set[2]`: word write operations
- `m_cs_set[3]`: CheckActionDone operations
- `m_cs`: clear operations

The M-scanner uses `m_bThreadRunStop` flag to pause read polling during write operations to avoid bus contention.

---

## 18. Logging

PLC operations are logged to `LogFile/IOControl/`:
- `PLC_READ.log` - M-address read scanner events
- `PLC_WRITE.log` - M-address write scanner events
- `IOControl.log` - IOMapControlMgr general operations

Log categories used: `SYST`, `SEND`, `RECV`, `OPER`, `WARN`

---

## 19. Legacy DefinesPLC.h Constants (Multi-EQ-Type)

`Source/Include/DefinesPLC.h` contains the legacy/shared PLC constants organized by equipment type. These are indexed relative to `PLC_BIT_READ_START` (4000) and `PLC_BIT_WRITE_START` (4500).

### Common Read Area (PLC_MEMORY_MAP_FOR_MONITORING, offset from M4000)

| Enum Value | Absolute Address | Description |
|------------|-----------------|-------------|
| PLC_Ready_State = 0 | M4000 | Ready state |
| PLC_Load_State = 1 | M4001 | Load state |
| PLC_Measure_State = 2 | M4002 | Measure state |
| PLC_Alarm_State = 8 | M4008 | Alarm state |
| PLC_Warn_State = 9 | M4009 | Warning state |
| PLC_Fixture_Module_Up_State = 30 | M4030 | Fixture up |
| PLC_WINDOW_OPEN_01..16 = 70..85 | M4070..M4085 | Window open sensors |
| PLC_Door_Sensor01..06 = 90..95 | M4090..M4095 | Door sensors |
| PLC_EMO_State01..08 = 100..107 | M4100..M4107 | EMO switch states |
| PLC_DOOR_UNLOCK_SENSOR_01..06 = 110..115 | M4110..M4115 | Door unlock sensors |
| PLC_Main_NFB1_ON = 120 | M4120 | UPS NFB ON |
| PLC_MC1_Contact..MC6 = 130..135 | M4130..M4135 | MC contactor status |
| PLC_Glass_Exist_LiftPin_01..02 = 140..141 | M4140..M4141 | Glass on lift pin |
| PLC_Glass_Exist_CHUCK_01..02 = 142..143 | M4142..M4143 | Glass on chuck |
| PLC_BIT_AIR_PRESSURE_01..03 = 160..162 | M4160..M4162 | Air pressure |
| PLC_VACUUM_PRESSURE_01..05 = 170..174 | M4170..M4174 | Vacuum pressure |
| PLC_CHUCK_VACUUM_State_01..05 = 175..179 | M4175..M4179 | Chuck vacuum state |
| PLC_MAIN_RunNTeach_Mode = 220 | M4220 | Run/Teach mode |
| PLC_Temperature_Sensor01..05 = 310..314 | M4310..M4314 | Temperature sensors |
| PLC_SMOKE_DETECT_SENSOR_01 = 315 | M4315 | Smoke detect |
| PLC_BIT_ELECTRIC_BOX_FAN_SENSOR01..08 = 320..327 | M4320..M4327 | E-box fan sensors |
| PLC_LiftPin_LOAD = 360 | M4360 | Lift pin load |
| PLC_BIT_INITIALIZE_COMPLETE = 490 | M4490 | Init complete |

### Common Write Area (offset from M4500)

| Enum Value | Absolute Address | Description |
|------------|-----------------|-------------|
| PLC_DOOR_UNLOCK_01..06 = 500..505 | M4500..M4505 | Door unlock commands |
| PLC_BIT_IONIZER_RUN_01..03 = 510..512 | M4510..M4512 | Ionizer run |
| PLC_CCD_POWER_01..05 = 530..534 | M4530..M4534 | CCD power |
| PLC_BIT_MAINT_LAMP_ONOFF = 570 | M4570 | Maint lamp |
| PLC_BIT_SHUTTER_UP/DOWN = 640..641 | M4640..M4641 | Shutter control |
| PLC_MULTITAP_ON_OFF_01..04 = 720..723 | M4720..M4723 | Multi-tap power |
| PLC_N2_BLOWING_01..04 = 800..803 | M4800..M4803 | N2 blowing |
| PLC_CHUCK_VACUUM_ON_01..05 = 810..814 | M4810..M4814 | Chuck vacuum on |
| PLC_CHUCK_VACUUM_OFF_01..05 = 815..819 | M4815..M4819 | Chuck vacuum off |
| PLC_Signal_Tower_RED..BLUE = 970..973 | M4970..M4973 | Signal tower |
| PLC_BIT_READY2LOAD_START..LOAD2READY = 980..985 | M4980..M4985 | State transitions |
| PLC_BIT_INITIALIZE = 990 | M4990 | Initialize command |
| PLC_BIT_ALARM_RESET = 995 | M4995 | Alarm reset |

---

## 20. Summary: Read vs Write Address Split

| Area | Read (Input) Range | Write (Output) Range |
|------|-------------------|---------------------|
| M-area | M4000 - M4499 (500 bits) | M4500 - M4999 (500 bits) |
| PACC M-area | M3900 - M3949 (50 bits) | M3950 - M3999 (50 bits) |
| F-alarm | F0 - F1999 (read only) | N/A |
| D-area (FDC) | D11500 - D12499 | D10000 - D11499 |
| D-area (GlassSensor) | D1550 - D1599 | D1501 - D1549 |
| D-area (ProbeCard) | D2100 - D2499 (read only) | N/A |
