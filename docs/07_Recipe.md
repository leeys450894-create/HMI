# 07. Recipe System Analysis

## 1. Overview

The ALPHA HMI recipe system manages product-specific parameters for glass display inspection. It operates through a **dual-layer architecture**:

1. **DLL Layer (`dfcRecipe64.dll`)** -- Handles `.rcp` and `.alg` file I/O, provides shared memory for inter-process communication, and hosts the Recipe Manager UI dialog.
2. **Application Layer (`CRecipeEditMgr` + `CRecipeData`)** -- Manages recipe lifecycle (New/Open/Save/Delete), UTG-mode INI persistence, measurement area configuration, and multi-head assignment logic.

Recipe files use two distinct formats:
- **`.rcp` files** -- Sequential text format (CStdioFile), loaded by the DLL layer
- **`.rcp` files (INI mode)** -- INI-style format via `CIniHelper`, loaded by the application layer (currently active for UTG mode)
- **`.alg` files** -- Align recipe, sequential text format

---

## 2. File Locations

### Source Files

| File | Path | Purpose |
|------|------|---------|
| `RecipeData.h` | `Source/ALPHA/Recipe/RecipeData.h` | Data model structs, `CRecipeData` class |
| `RecipeData.cpp` | `Source/ALPHA/Recipe/RecipeData.cpp` | INI/Binary serialization, measurement area management |
| `RecipeEditMgr.h` | `Source/ALPHA/Recipe/RecipeEditMgr.h` | Recipe lifecycle manager class |
| `RecipeEditMgr.cpp` | `Source/ALPHA/Recipe/RecipeEditMgr.cpp` | Open/Save/Delete, thread loop for shared-mem commands |
| `RecipeMgr.h` | `Source/ALPHA/Manager/RecipeMgr.h` | Singleton wrapper for DLL initialization |
| `RecipeMgr.cpp` | `Source/ALPHA/Manager/RecipeMgr.cpp` | `CRecipeMgr` singleton, calls `Recipe_Initialize()` |
| `DlgSubRecipe.h/cpp` | `Source/ALPHA/DlgSubRecipe.*` | Recipe UI tab (buttons: New, Open, Save, SaveAs, Delete) |
| `DoWorkGlassRecipe.cpp` | `Source/ALPHA/DoWork/DoWorkGlassRecipe.cpp` | Sequence step: recipe change during auto-run |
| `DoWorkGlassRecipeStep.cpp` | `Source/ALPHA/DoWork/DoWorkGlassRecipeStep.cpp` | Step implementations: Change, ProbeCheck, Idle |
| `preDefine.h` | `Source/Include/preDefine.h` | Core recipe structs (`S_RecipeMgr`, `S_RecipeInfo`, etc.) |
| `RecipeShareMem.h` | `Source/Include/RecipeShareMem.h` | `VTM_RECIPE` shared memory struct |
| `API_RECIPE.h` | `Source/Include/API_RECIPE.h` | DLL API function declarations |
| `OutRecipeFunc.h` | `Source/Include/OutRecipeFunc.h` | `C_OutRecipeFunc` -- DLL-exported recipe accessor class |

### DLL Source Files

| File | Path | Purpose |
|------|------|---------|
| `dfcRecipe.cpp/h` | `CoreFunction/dll_Recipe/dfcRecipe/` | DLL entry, shared memory creation, config loading |
| `InRecipeFunc.cpp/h` | `CoreFunction/dll_Recipe/dfcRecipe/` | `C_InRecipeFunc` -- internal recipe load/save logic |
| `OutRecipeFunc.cpp/h` | `CoreFunction/dll_Recipe/dfcRecipe/` | `C_OutRecipeFunc` -- exported recipe accessor |
| `RecipeManager.cpp/h` | `CoreFunction/dll_Recipe/dfcRecipe/` | `CRecipeManager` CWnd wrapper for DLL UI |
| `RecipeMgrDlg.h` | `CoreFunction/dll_Recipe/dfcRecipe/` | `C_RecipeMgrDlg` -- Recipe editor dialog with grids |

### Configuration Files

| File | Path | Purpose |
|------|------|---------|
| `Recipe.ini` | `Source/ALPHA/ConfigurationFile/Recipe.ini` | DLL config: folder paths, contact limit, color settings |
| `Recipe.ini` | `CoreFunction/dll_Recipe/Recipe.ini` | Same content (copy) |

### Runtime File Paths (from Recipe.ini)

| Key | Path | Purpose |
|-----|------|---------|
| `RECIPE_FOLDER` | `D:\HBS\RECIPE\Recipe\` | `.rcp` recipe files |
| `BACKUP_FOLDER` | `D:\HBS\RECIPE\RECIPEBAK\` | Recipe backup storage |
| `ALIGN_FOLDER` | `D:\HBS\RECIPE\ALIGN_RECIPE\` | `.alg` align recipe files |
| `REF_IMG_FOLDER` | `D:\HBS\RECIPE\ALIGN\` | Pattern reference images |
| `PPID_FOLDER` | `D:\CIMDATA\PPID.ini` | CIM PPID mapping |
| `RECIPE_LIST` | `D:\CIMDATA\RcpList.txt` | Recipe list for CIM |

---

## 3. Data Model (Struct Hierarchy)

### 3.1 Top-Level: `stRecipeInfo` (RecipeData.h)

```
stRecipeInfo                          -- Master container in CRecipeData
  +-- stRecipeAlign stRecipeAlignPtrn -- Align pattern data
  +-- stPtrnInfo stRecipeMeasPtrn[8]  -- Measurement patterns (8 slots)
  +-- stRecipeMeas stRecipeMeas       -- Measurement pattern wrapper
  +-- stRecipeOption stRecipeOptionParam -- Measurement options
  +-- stRecipeUTGParam stRecipeUTG    -- UTG-mode measurement parameters
  +-- S_RecipeMgr stRecipeHMI         -- HMI recipe data (DLL-compatible struct)
```

### 3.2 DLL-Level: `S_RecipeMgr` (preDefine.h)

```
S_RecipeMgr                           -- Root recipe struct used by DLL
  +-- int m_nVersion                  -- Recipe file version
  +-- int m_nStickCnt                 -- Number of sticks
  +-- int m_nCellCnt                  -- Number of cells per stick
  +-- S_RecipeInfo m_sInfo            -- Inspection & option parameters
  |     +-- S_InspectItem m_sInspectItem  -- Inspection enable flags
  |     +-- S_OptionItem m_sOptionItem    -- Option flags & head gaps
  |     +-- S_LampItem m_sLampItem        -- Lamp intensity per head
  |     +-- S_ContactPos m_sContactPos    -- Contact Z positions
  |     +-- S_MotionOffsetPos m_sMotionOffsetPos -- Motion offsets
  |     +-- S_ProbeInfo m_sProbeInfo      -- Probe ID & priority
  +-- S_RecipeData m_sData            -- Runtime cell/stick data
  |     +-- vector<S_StickInfo> m_vecStick -- Sticks with cells
  |     +-- S_HeadZ m_sHeadZ          -- Head Z positions per mode
  +-- S_RecipeMap m_sMap              -- Cell map / geometry
  +-- S_Align m_sAlign                -- Align configuration
```

### 3.3 Shared Memory: `VTM_RECIPE` (RecipeShareMem.h)

```
VTM_RECIPE                            -- Inter-process shared memory
  +-- S_RecipeRunState m_sRunState    -- Command/status for recipe DLL thread
  +-- S_Align m_sAlign                -- Align data passed between processes
```

Shared memory name: `"HBS_RECIPE_SHARED_MEMORY"`, created via `CreateFileMapping` in `dfcRecipe.cpp`.

---

## 4. Parameter Details

### 4.1 Inspection Items (`S_InspectItem`)

| Field | Type | Description |
|-------|------|-------------|
| `m_bUsePre_0` | BOOL | Pre-inspection enable |
| `m_bUseElect_1` | BOOL | Electrical inspection enable |
| `m_bUseRTest` | BOOL | R-Test enable |
| `m_bUseAgingTest_2` | BOOL | Aging test enable |
| `m_bUseSpec_3` | BOOL | Spec inspection (WAD) enable |
| `m_bUseQuality_4` | BOOL | Quality (vision) inspection enable |
| `m_bUseColor_5` | BOOL | Color inspection enable |
| `m_bUseAngle_6` | BOOL | Viewing angle inspection enable |
| `m_bUseTSP_7` | BOOL | TSP inspection enable |
| `m_bUseLCR_8` | BOOL | LCR (cell impedance) enable |
| `m_nTimeOut[9]` | int[9] | Timeout per inspection item (seconds) |

### 4.2 Option Parameters (`S_OptionItem`)

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `m_bUseRtestNgPass` | BOOL | FALSE | Pass on R-Test NG |
| `m_bUseSampling` | BOOL | FALSE | Sampling inspection mode |
| `m_bUseAlign` | BOOL | FALSE | Glass align enable |
| `m_bUseReviewSnap` | BOOL | FALSE | Contact position review image |
| `m_nGlsRotate` | int | 1 | Glass rotation: 0/1/2/3 = 0/90/180/270 deg |
| `m_fFrontHeadGap` | double | 0.0 | Front head gap (mm) |
| `m_fBackHeadGap` | double | 0.0 | Back head gap (mm) |
| `m_chAlignName[1024]` | wchar_t | "" | Align recipe filename |

### 4.3 Contact Position (`S_ContactPos`)

| Field | Type | Description |
|-------|------|-------------|
| `m_fReadyPos` | double | Ready position (mm) |
| `m_fPreContactPos` | double | Pre-contact position (mm) |
| `m_fEndContactPos` | double | End-contact position (mm) |
| `m_fAddOD` | double | Additional overdrive (mm) |

### 4.4 Motion Offsets (`S_MotionOffsetPos`)

| Field | Type | Description |
|-------|------|-------------|
| `m_fStageY` | double | Stage Y offset |
| `m_fStageRotate` | double | Stage rotation offset |
| `m_fHeadReviewY` | double | Review head Y offset |
| `m_fHeadReviewLeft` | double | Review head left offset |
| `m_fHeadReviewRight` | double | Review head right offset |
| `m_nQualityRotate` | int | Quality rotation (0=0deg, 1=90deg) |
| `m_fQualityLeftRot` | double | Quality left rotation offset |
| `m_fQualityRightRot` | double | Quality right rotation offset |
| `m_fProbeShift` | double | Probe shift offset (added in V1004+) |

### 4.5 Lamp Settings (`S_LampItem`)

| Field | Type | Description |
|-------|------|-------------|
| `m_nFrontLeft` | int | Front-left lamp intensity |
| `m_nFrontRight` | int | Front-right lamp intensity |
| `m_nBackLeft` | int | Back-left lamp intensity |
| `m_nBackRight` | int | Back-right lamp intensity |
| `m_nSRMLamp[4]` | int[4] | SRM lamp intensity (4 channels) |

### 4.6 Probe Info (`S_ProbeInfo`)

| Field | Type | Description |
|-------|------|-------------|
| `m_fProbeW` | double | Probe width |
| `m_fProbeH` | double | Probe height |
| `m_nProbePriority` | int | Priority selection: 0=First, 1=Second |
| `m_chProbeFirst[5][1024]` | wchar_t | First-priority probe IDs (5 probes) |
| `m_chProbeSecond[5][1024]` | wchar_t | Second-priority probe IDs (5 probes) |

### 4.7 Head Z Positions (`S_HeadZ`)

| Field | Type | Description |
|-------|------|-------------|
| `m_fReviewZ` | double | Review mode Z position |
| `m_fSpecZ` | double | Spec (WAD) mode Z position |
| `m_fQualityZ` | double | Quality (vision) mode Z position |
| `m_fColorZ` | double | Color mode Z position |
| `m_fAngleZ` | double | Angle mode Z position |
| `m_fZoom` | double | Zoom position |
| `m_fFocus` | double | Focus position |

### 4.8 Cell Map (`S_RecipeMap`)

| Field | Type | Description |
|-------|------|-------------|
| `m_chModelInfoName[1024]` | wchar_t | Model/product name |
| `m_fGlobalMark` | S_Point | Glass key position (x, y) |
| `m_fCellCenter` | S_Point | Cell center coordinate |
| `m_fCellCount` | S_Point | Cell count (columns, rows) |
| `m_fCellDistance` | S_Point | Cell pitch (x, y) in mm |
| `m_fCellSize` | S_Point | Cell size (w, h) in mm |
| `m_fAlignKey` | S_Point | Align recipe key position |
| `m_fContactOffset` | double | Contact offset |
| `m_fCadData[32]` | double | CAD data rows A~X7 |
| `m_chCadData[10][1024]` | wchar_t | CAD data rows P1_1~P5_2 |

### 4.9 UTG Mode Parameters (`stRecipeUTGParam`)

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `nVersion` | int | 1 | UTG recipe version |
| `nMeasMode` | int | 1 | Measurement mode (1-4: direction/side) |
| `nUseMeasTopBtm` | int | 1 | 0=both, 1=top only, 2=bottom only |
| `nUseMeasHead` | int | 0 | 0=H1+H2, 1=H1 only, 2=H2 only |
| `nUseMeasMode` | int | 0 | 0=0+180, 1=0 only, 2=180 only |
| `dCellSizeW/H` | double | 0.0 | Cell size (mm) |
| `dStageSizeW/H` | double | 0.0 | Stage size (mm) |
| `dCellHWCenter` | double | 0.0 | Cell height-width center |
| `nAFChannel[2]` | int | 0,1 | AF channel: top, bottom |
| `stCoordAlign` | stCoordinate | -- | Align position (x, y, z) |
| `dAlignCamDistance` | double | 50.0 | Align camera distance (mm) |
| `dAlignMarginX/Y/R` | double | 0.01 | Align margin (mm) |
| `nAlignRetry` | int | 3 | Align retry count |
| `stAlignParam[3]` | stAlignSetParam | -- | 3 sets of align camera params |
| `stCoordTopMeasPos[2]` | stCoordinate | -- | Top measurement positions |
| `stCoordBottomMeasPos[2]` | stCoordinate | -- | Bottom measurement positions |
| `stTriggerParam` | stTriggerSetParam | -- | Trigger parameters |

### 4.10 Trigger Parameters (`stTriggerSetParam`)

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `iDirection` | int | 1 | Scan direction |
| `dEncoderResulution` | double | 0.1 | Encoder resolution |
| `iTriggerPeriod` | int | 100 | Trigger period |
| `iTriggerCount` | int | 0 | Trigger count |
| `dMeasureReadyPos` | double | 0.0 | Measurement ready position |
| `dStartPos` | double | 0.0 | Start position |
| `dMeseRange` | double | 100.0 | Measurement range |
| `dTriggerPitch` | double | 10.0 | Trigger pitch |
| `dSpeedVel` | double | 30.0 | Speed velocity |
| `dSpeedAcc` | double | 300.0 | Speed acceleration |
| `dTriggerOffset` | double | 0.0 | Trigger offset |

### 4.11 Measurement Area (`stMeasAreaInfo` -- application layer)

| Field | Type | Description |
|-------|------|-------------|
| `strCellID` | CString | Cell identifier |
| `iMeasHeadID_linked` | int | Assigned head (0 or 1, -1=unassigned) |
| `iAreaNumber` | int | Area number (1-based) |
| `iMeasOption` | int | Measurement option (PPA/AA/Snap) |
| `stMeasCoordinate` | stCoordinate | Position (x, y, s, z) + use/AF/skip flags |
| `nMultiHeadMeasArea` | int | Multi-head grouping index |

### 4.12 Measurement Option (`stRecipeOption` -- application layer)

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `nMeasMode` | int | 0 | PPA measurement mode |
| `nUseMeasHead` | int | 0 | 0=H1+H2, 1=H1, 2=H2 |
| `nMeasShotPPA` | int | 0 | PPA shot mode (2-shot, 1-shot White, 1-shot UV) |
| `nMeasShotAA` | int | 0 | AA shot mode |
| `dHeadGap` | double | 0.0 | Head gap (mm) |
| `dZOffsetAA` | double | 0.0 | AA Z offset |
| `dCellSizeW/H` | double | 0.0 | Cell size |
| `nBeamSplitterH1/H2` | int | 0 | Beam splitter for Head1/Head2 |
| `nGlassModel` | int | 0 | Glass model type |
| `bUseAlign` | bool | false | Use alignment |
| `bUsePatternMatch` | bool | false | Use pattern matching |
| `bUsePPA` | bool | false | Use PPA measurement |
| `bUseN2MotorAging` | bool | false | Use N2 motor aging |
| `stPPAThetaCalc` | stThetaCalc | -- | PPA theta calculation params (Lx, Ly, L1-L4, L) |

### 4.13 Align Pattern (`stPtrnInfo`)

| Field | Type | Default | Description |
|-------|------|---------|-------------|
| `strName` | CString | "" | Pattern name |
| `strPtrnImgName[10]` | CString | "" | Multi-pattern image names |
| `rectPtrn` | CRect | (0,0,0,0) | Pattern ROI rectangle |
| `dThreshold` | double | 500.0 | Pattern matching threshold |
| `iFindNum` | int | 5 | Search count |
| `iCntrNum` | int | 5 | Centering count |
| `iCenMarginX/Y` | int | 10 | Centering margin (pixels) |
| `dCenOffsetX/Y` | double | 0.0 | Centering offset (mm) |
| `dWhiteBalanceRed/Green/Blue` | double | 0.0 | White balance |
| `dOffsetAfterCenteringX/Y` | double | 0.0 | Post-centering offset |
| `iCCDGain` | int | 0 | CCD gain |
| `iExposureTime` | int | 500 | Exposure time (ms) |
| `iLampIntensity` | int | 90 | Lamp intensity |
| `iLens` | int | 0 | Lens selection |

---

## 5. Recipe File Versions

Defined in `preDefine.h`:

| Enum | Value | Description |
|------|-------|-------------|
| `RECIPE_FILE_V1000` | 1000 | Base version |
| `RECIPE_FILE_V1001` | 1001 | Added CAD info section |
| `RECIPE_FILE_V1002` | 1002 | Added GlobalMark, CellCenter, CellCount/Distance strings |
| `RECIPE_FILE_V1003` | 1003 | Added CellCount.y (2D cell count) |
| `RECIPE_FILE_V1004` | 1004 | Added ProbeShift offset |

---

## 6. Recipe File Format (`.rcp` -- DLL Text Format)

Loaded by `C_InRecipeFunc::LoadRcpFile()` / `C_OutRecipeFunc::LoadRcpFile()` using `CStdioFile`.

Sequential sections with header lines and data lines:

```
[Version Header]
<version integer>
[CellInfo Header]
<ModelInfoName>
<CellSize.x> <CellSize.y>
<CellDistance.x> <CellDistance.y>
<CellCount.x> [CellCount.y if V1003+]
<ContactOffset>
[Option Header]
<UseRtestNgPass> <UseSampling> <UseAlign> <UseReviewSnap>
<GlsRotate> <FrontHeadGap> <BackHeadGap> <AlignName>
[InspectionItem Header]
<UsePre> <UseElect> <UseAging> <UseSpec> <UseQuality> <UseColor> <UseAngle> <UseTSP> <UseLCR>
<TimeOut comma-separated>
[ProbeID Header]
<ProbeFirst comma-separated x5>
<ProbeSecond comma-separated x5>
<ProbePriority>
[ContactPos Header]
<ReadyPos> <PreContactPos> <EndContactPos> <AddOD>
[Lamp Header]
<FrontLeft> <FrontRight> <BackLeft> <BackRight>
[HeadOffset Header]
<StageY> <StageRotate> <HeadReviewY> <HeadReviewLeft> <HeadReviewRight>
<QualityRotate> <QualityLeftRot> <QualityRightRot>
[ProbeShift if V1004+]
[Stage Header]
<StickCount>
<StickNum comma-separated>
<StageY comma-separated>
<StageR comma-separated>
[HeadZ Header]
<ReviewZ> <SpecZ> <QualityZ> <ColorZ> <AngleZ> <Zoom> <Focus>
[Head Header]
<CellCount>
  For each stick:
    <CellID comma-separated>
    <CellY comma-separated>
    <CellX comma-separated>
    <Area comma-separated>
[CellSelect Header]
  For each stick:
    <Spec select comma-separated>
    <Quality select comma-separated>
    <Color select comma-separated>
    <Angle select comma-separated>
[CAD Info Header if V1001+]
  <CadData comma-separated (32 doubles)>
  <CadData strings comma-separated (10 strings)>
[GlobalMark/CellCenter if V1002+]
  <GlobalMark.x> <GlobalMark.y> <CellCenter.x> <CellCenter.y>
  <CellCount string> <CellDistance string>
```

---

## 7. Align Recipe File Format (`.alg`)

Loaded by `C_InRecipeFunc::LoadAlignRcpFile()` / `C_OutRecipeFunc::LoadAlignRcpFile()`.

```
[Glass Align Header]
  Base: StageY, FrontLeft, FrontRight
  Pos:  StageY, StageRotate, FrontY, FrontLeft, FrontRight, FrontZ
[Probe Align Header]
  Base: ProbeY, BackLeft, BackRight
  Pos:  ProbeY, ProbeX, BackY, BackLeft, BackRight, BackZ, StageY
[CCD Set Header]
  ExposureTime: FrontLeft, FrontRight, BackLeft, BackRight
  Gain:         FrontLeft, FrontRight, BackLeft, BackRight
  Led:          FrontLeft, FrontRight, BackLeft, BackRight
  MatchingScore: FrontLeft, FrontRight, BackLeft, BackRight
[Reference Image Header]
  PtnFrontLeft comma-separated x5
  PtnFrontRight comma-separated x5
  PtnBackLeft comma-separated x5
  PtnBackRight comma-separated x5
[Margin Header]
  Glass: X, Y, R, Centering
  Probe: X, Y, R, Centering, FindPtn
```

---

## 8. INI Format (UTG Mode -- `CRecipeData::LinkIniFileUTG`)

Currently the **active serialization method** (the binary and standard INI paths are commented out in `CRecipeEditMgr::LinkDataBase()`).

### INI Sections

| Section | Keys | Description |
|---------|------|-------------|
| `[RECIPE]` | VERSION, MEAS_MODE, MEAS_HEAD, CELL_SIZE_W/H, STAGE_SIZE_W/H | UTG recipe identity & cell/stage geometry |
| `[ALIGN]` | MOVE_POS_X/Y/Z, CENTERING_NUM, MARGIN_X/Y/R, CAM_DISTANCE, ALIGN_ROTATE_OFFSET_X/Y | Align position & centering parameters |
| `[ALIGN_SET_1..3]` | THRESHOLD_G, THRESHOLD_CANY_U/D, CAM_RESOLUTION, GAIN, EXPOSURE_TIME, INTENSITY | 3 sets of align camera parameters |
| `[MEAS]` | TOP_MOVE_POS_X/Z, TOP_AF, BOTTOM_MOVE_POS_X/Z, BOTTOM_AF (x2 each), TOP_AF ch, BOTTOM_AF ch | Measurement positions (top/bottom, 2 points each) |
| `[TRIGGER]` | MEAS_POS, MEAS_RANGE, PITCH, ENCODE_RESULUTION, PERIOD, MEAS_READYPOS, MEAS_SPEED_VEL/ACC, MEAS_TRIGGER_OFFSET | Trigger/scan parameters |

### Standard INI Sections (LinkIniFile -- currently unused but available)

| Section | Keys | Description |
|---------|------|-------------|
| `[ALIGN]` | USE, USE_4POINT, per-point MOVE_POS/CAD_POS/PTRN params (x4 points) | Pattern-based alignment |
| `[MEAS]` | Per-pattern WB/PTRN/THRESHOLD/MARGIN/OFFSET/GAIN/EXPOSURE/INTENSITY (x8 patterns), CELL_SIZE, HEAD_GAP, THETA params, AA_Z_OFFSET | Measurement pattern config |
| `[OPTION]` | MEAS_MODE/HEAD/SHOT_PPA/SHOT_AA, BSH1/BSH2, GLSMODEL, USE_ALIGN/PATTERN/PPA/N2_MOTOR_AGING | Measurement mode options |
| `[MEASURFEMENT]` | MAX_AREA, per-area AREA/XPOS/YPOS/SXPOS/ZPOS/USE/AF/SKIP/CELL_ID/MEAS_OPTION | Variable-count measurement areas |
| `[MEAS_AREA_ITEM_LIST]` | COUNT, per-item NAME/WAIT_TIME | Measurement area sequence items |

---

## 9. Load Flow

### 9.1 Application Startup

```
CAuroraMgr::Initialize()
  -> CRecipeEditMgr::Initialize()
       -> Sets m_pIniHelper path to DEFAULT.rcp
       -> LinkDataBase(false)  // load
            -> CRecipeData::LinkIniFileUTG(pIniHelper, false)

CDlgSubRecipe::OnInitDialog()
  -> CRecipeMgr::GetInstance()->Initialize()
       -> Recipe_Initialize() [DLL export]
            -> CRecipeManager::Initialize()
                 -> Creates C_RecipeMgrDlg (Recipe UI)
       -> Recipe_GetSharedMemory() [DLL export]
            -> Returns g_pSysMapView (VTM_RECIPE*)
  -> CRecipeEditMgr::SetRecipePtr()
       -> m_pRecipeFunc = GetRecipeMgr() [DLL export]
            -> Returns C_OutRecipeFunc* (g_inRcpInfo cast)
```

### 9.2 Recipe Open (Auto-Run Sequence)

```
CDoWorkGlassRecipeStep_Change::Run()
  -> CRecipeEditMgr::OpenRecipe(strPPID)
       -> CRecipeEditMgr::RecipeChange(strRecipeID)
            -> C_OutRecipeFunc::LoadRcpFile(strRecipeID)
                 -> Reads {RECIPE_FOLDER}/{name}.rcp (CStdioFile)
                 -> Populates m_recipeMgr (S_RecipeMgr)
            -> C_OutRecipeFunc::LoadAlignRcpFile(strAlignRcpName)
                 -> Reads {ALIGN_FOLDER}/{name}.alg (CStdioFile)
                 -> Populates m_recipeMgr.m_sAlign
            -> RecipeUpdateUI()
                 -> Posts WM_MSG_UPDATE_RECIPE_INFORMATION to UI
       -> CSystemParamShare::SetRecipeID(strRecipeID)
  -> Recipe data flows to measurement devices via TCP:
       DoWorkRecipeOpen(PG)
       DoWorkRecipeOpen(AGING)
       DoWorkRecipeOpen(WAD)
       DoWorkRecipeOpen(LCR)
       DoWorkRecipeOpen(TSP)
       DoWorkRecipeOpen(VISION)
```

### 9.3 Recipe Open (Manual/Thread-based -- currently dead code path)

```
CRecipeEditMgr::RecipeChange() [after return at line 145]
  -> m_pIniHelper->SetFilePath(fullPath)
  -> LinkDataBase(false)
       -> CRecipeData::LinkIniFileUTG(pIniHelper, false)
```

---

## 10. Save Flow

```
CRecipeEditMgr::SaveRecipe(strRecipeID)
  1. Builds full path: {RecipeFolderPath}/{RecipeID}.rcp
  2. Creates backup: {RecipeFolderPath}/BACKUP/{RecipeID}/{RecipeID}_{timestamp}.rcp
  3. CopyFile(original -> backup)
  4. m_pIniHelper->SetFilePath(original)
  5. LinkDataBase(true)  // save mode
       -> CRecipeData::LinkIniFileUTG(pIniHelper, true)
           -> Writes all UTG parameters to INI file

CRecipeEditMgr::SaveRecipeAs(strRecipeID)
  1. Builds new path: {RecipeFolderPath}/{NewRecipeID}.rcp
  2. m_pIniHelper->SetFilePath(newPath)
  3. LinkDataBase(true)  // save mode (no backup)
```

Note: The DLL-layer `C_RecipeMgrDlg` also has its own Save functionality (`SaveFile()`) that saves through the DLL-internal `C_InRecipeFunc` using the text-based `.rcp` format.

---

## 11. Shared Memory Communication (Recipe DLL Thread)

`CRecipeEditMgr::ThreadRun()` polls `VTM_RECIPE` shared memory every 10ms.

### Command Protocol (`S_RecipeRunState`)

| Field | Usage |
|-------|-------|
| `m_nRunStatus` | M_RUN_NONE(0) / M_RUN_REQ(1) / M_RUN_PROC(2) / M_RUN_END(3) / M_RUN_NG(4) |
| `m_dwData1` | Command code (M_CMD_*) |
| `m_dwData2` | Head selection (M_HEAD_FRONT_LEFT, etc.) |
| `m_dwData3` | Mode type (M_TYPE_REVIEW, SPEC, QUALITY, COLOR, ANGLE) |
| `m_fData[100]` | Position data (input/output) |
| `m_nData[10]` | Integer parameters |
| `m_chData1[1024]` | String parameter (pattern name, etc.) |

### Supported Commands

| Command | Code | Description |
|---------|------|-------------|
| `M_CMD_SHOW_REF_SET` | 0x01 | Set ROI for align pattern |
| `M_CMD_PATTERN_SEARCH` | 0x02 | Detect align pattern |
| `M_CMD_CCD_SET` | 0x03 | Set CCD exposure/gain/lamp |
| `M_CMD_GLASS_GET_POS` | 0x04 | Get glass align axis positions |
| `M_CMD_GLASS_MOVE` | 0x05 | Move to glass align position |
| `M_CMD_GLASS_PROC` | 0x08 | Execute glass SW alignment |
| `M_CMD_PROBE_GET_POS` | 0x06 | Get probe align axis positions |
| `M_CMD_PROBE_MOVE` | 0x07 | Move to probe align position |
| `M_CMD_PROBE_PROC` | 0x09 | Execute probe SW alignment |
| `M_CMD_RESET` | 0x10 | Reset alignment |
| `M_CMD_LAMP_SET` | 0x11 | Set LED lamp intensity |
| `M_CMD_CONTACT_SET` | 0x12 | Move probe Z contact position |
| `M_CMD_STAGE_GET_POS` | 0x13 | Get stage position |
| `M_CMD_STAGE_MOVE` | 0x14 | Move stage |
| `M_CMD_STAGE_PROBE_MOVE` | 0x15 | Move stage + all 5 probes |
| `M_CMD_HEAD_GET_POS` | 0x16 | Get head position |
| `M_CMD_HEAD_MOVE` | 0x17 | Move head |
| `M_CMD_SRM_LAMP_SET` | 0x18 | Set SRM lamp intensity |

### Interlocks (checked before command execution)

- Must be in `df_HOST_OFFLINE` mode
- Process status must be `SPS_IDLE`
- All sequence engines must be complete

---

## 12. Recipe Change Sequence (Auto-Run)

`CDoWorkGlassRecipe` is a sequence item (`enumSeqItemID_GLASS_RECIPE_CHANGE`) with steps:

| Step | Class | Description |
|------|-------|-------------|
| 0 | `CDoWorkGlassRecipeStep_None` | Init: clear flags, set process status to SPS_PROCESS, clear result data |
| 1 | `CDoWorkGlassRecipeStep_Change` | Load recipe, distribute to measurement devices via TCP |
| 2 | `CDoWorkGlassRecipeStep_ProbeCheck` | Verify/exchange probes per recipe's probe ID settings |
| 3 | `CDoWorkGlassRecipeStep_Idle` | Complete: set SPS_IDLE, signal completion event |

### ProbeCheck Logic (Step 2)

For each of 5 probes:
1. Gets probe name from recipe (first or second priority based on `m_nProbePriority`)
2. Compares with current probe ID in shared memory (`m_szPROBEID[i]`)
3. If mismatch:
   - Checks if probe slot has a card (via PLC DIO bits `PLC_R_PROBE_UNITxx_CARD_CHK`)
   - If empty: triggers probe **Loading** from cassette
   - If occupied: triggers probe **Exchange** (unload current, load required)
4. Waits up to 30 minutes for PACC (Probe Auto Card Changer) completion

---

## 13. Multi-Head Area Assignment (`CRecipeEditMgr::ChangeAreaInfoRecipeToMeas`)

Assigns measurement areas to Head1 or Head2 based on spatial proximity.

**Algorithm:**
1. If single-head mode (`nUseMeasHead == 1 or 2`): all areas assigned to that head
2. If dual-head mode (`nUseMeasHead == 0`):
   - Step 1: Groups areas with same X coordinate and Y distance > `df_Head_Gap` (300mm)
   - Step 2: Within each group, assigns heads based on safety position comparison (`dSafetyHeadY1` vs `dSafetyHeadY2`)

---

## 14. DLL Exported Functions

Defined in `dfcRecipe.def`:

| Function | Signature | Description |
|----------|-----------|-------------|
| `Recipe_GetSharedMemory` | `VTM_RECIPE* WINAPI` | Returns pointer to shared memory |
| `Recipe_Initialize` | `void WINAPI (HWND, CRect)` | Creates shared memory, initializes DLL UI |
| `Recipe_GetRecipehWnd` | `CWnd* WINAPI` | Returns recipe dialog window handle |
| `Recipe_Finalize` | `void WINAPI` | Cleanup |
| `Recipe_SetMsg` | `void WINAPI (CString)` | Set message to recipe dialog |
| `GetRecipeMgr` | `C_OutRecipeFunc* WINAPI` | Returns recipe function object |

---

## 15. Usage Sites (49 files reference recipe)

### Core Recipe Files
- `Source/ALPHA/Recipe/RecipeEditMgr.cpp` -- Recipe lifecycle management
- `Source/ALPHA/Recipe/RecipeData.cpp` -- Data serialization
- `Source/ALPHA/Manager/RecipeMgr.cpp` -- Singleton DLL wrapper
- `Source/ALPHA/DlgSubRecipe.cpp` -- Recipe UI tab

### Sequence/DoWork (recipe data consumers)
- `Source/ALPHA/DoWork/DoWorkGlassRecipeStep.cpp` -- Recipe change & probe check
- `Source/ALPHA/DoWork/DoWorkGlassSequenceEngine.cpp` -- Glass sequence references recipe
- `Source/ALPHA/DoWork/DoWorkGlassMeasureStep.cpp` -- Measurement uses recipe params
- `Source/ALPHA/DoWork/DoWorkGlassPreMeasStep.cpp` -- Pre-measurement uses recipe
- `Source/ALPHA/DoWork/DoWorkGlassReadyStep.cpp` -- Ready step uses recipe
- `Source/ALPHA/DoWork/DoWorkGlassAGINGStep.cpp` -- Aging test uses recipe
- `Source/ALPHA/DoWork/DoWorkGlassResultStep.cpp` -- Result step uses recipe
- `Source/ALPHA/DoWork/DoWorkGlassUnloadStep.cpp` -- Unload step uses recipe
- `Source/ALPHA/DoWork/DoWorkGlassSWAlignStep.cpp` -- Glass align uses recipe
- `Source/ALPHA/DoWork/DoWorkProbeSWAlignStep.cpp` -- Probe align uses recipe
- `Source/ALPHA/DoWork/DoWorkGlassAdsorptionStep.cpp` -- Adsorption uses recipe
- `Source/ALPHA/DoWork/SystemInitializeStep.cpp` -- System init references recipe
- `Source/ALPHA/DoWork/MeasureSequenceItemEngine.cpp` -- Measurement engine

### TCP/IP Communication (recipe sent to devices)
- `Source/ALPHA/CommTcpIpObjectPG.cpp/h` -- Pattern Generator
- `Source/ALPHA/CommTcpIpObjectAgingPG.cpp/h` -- Aging PG
- `Source/ALPHA/CommTcpIpObjectWAD.cpp/h` -- WAD (spec)
- `Source/ALPHA/CommTcpIpObjectVision.cpp/h` -- Vision quality
- `Source/ALPHA/CommTcpIpObjectColor.cpp/h` -- Color
- `Source/ALPHA/CommTcpIpObjectAngle.cpp/h` -- Angle
- `Source/ALPHA/CommTcpIpObjectTSP.cpp/h` -- TSP
- `Source/ALPHA/CommTcpIpObjectLCR.cpp/h` -- LCR
- `Source/ALPHA/CommTcpIpObjectCIM.cpp/h` -- CIM/MES
- `Source/ALPHA/CommTcpIpObjectUTG.h` -- UTG
- `Source/ALPHA/Manager/CommTcpIpFactoryMgr.cpp` -- Factory dispatches recipe open

### UI
- `Source/ALPHA/DlgTitleBar.cpp` -- Displays recipe ID in title
- `Source/ALPHA/DlgSubOperate.cpp` -- Operate page references recipe
- `Source/ALPHA/DlgSubManual_Meas.cpp` -- Manual measurement page
- `Source/ALPHA/D_MeasureCycle.cpp` -- Cycle test page

### Infrastructure
- `Source/ALPHA/AuroraMgr.h` -- Owns RecipeEditMgr pointer
- `Source/ALPHA/BaseSequenceBody.h` -- Base class exposes recipe accessors
- `Source/ALPHA/FunctionModule/FunctionModuleProbeACCStep.cpp` -- Probe ACC references recipe

---

## 16. Key Constants

```cpp
#define M_RCP_PROBE_MAX      5      // Max probe IDs per recipe
#define M_RCP_INS_ITEM       9      // Number of inspection items
#define CH_LEN            1024      // String buffer length
#define df_Gantry_Compare 0.00001   // Position comparison epsilon
#define df_Head_Gap        300      // Multi-head minimum gap (mm)
#define RCP_GLASS_SIZE_X  1500.0    // Glass width (mm)
#define RCP_GLASS_SIZE_Y   925.0    // Glass height (mm)
#define _RCP_EXT_FILE_NAME  ".rcp"  // Recipe file extension
#define _RCP_ALL_FILE_NAME  "*.rcp" // Recipe file wildcard
```

### Measurement Mode Enum (`enumMEASURE_MODE_INDEX`)

| Value | Name | Description |
|-------|------|-------------|
| 0 | `enumMEASURE_MODE_WAD` | Spec (WAD) measurement |
| 1 | `enumMEASURE_MODE_VISION` | Vision quality measurement |
| 2 | `enumMEASURE_MODE_COLOR` | Color measurement |
| 3 | `enumMEASURE_MODE_ANGLE` | Viewing angle measurement |
| 4 | `enumMEASURE_MODE_TSP` | TSP measurement |
| 5 | `enumMEASURE_MODE_LCR` | LCR measurement |
| 6 | `enumMEASURE_MODE_PG` | Pattern Generator |
| 7 | `enumMEASURE_MODE_AGING` | Aging test |

---

## 17. Architecture Notes

1. **Dual persistence**: The DLL layer reads `.rcp` files (text format) while the application layer currently uses INI format (UTG mode). The binary helper (`CBinaryHelper`) path is commented out.

2. **Two recipe data instances**: `C_OutRecipeFunc::m_recipeMgr` (DLL side, populated by `LoadRcpFile`) and `CRecipeData::m_stRecipeInfo.stRecipeHMI` (app side, populated by `LinkIniFileUTG`). The DLL-side data is the primary source during auto-run.

3. **Recipe DLL thread**: `CRecipeEditMgr` runs a dedicated thread that processes shared-memory commands from the recipe DLL dialog. This enables the recipe editor UI (which lives in the DLL process context) to request motion, lamp, and alignment operations from the main application.

4. **UI mostly disabled**: The `CDlgSubRecipe` button handlers are almost entirely commented out. Recipe Open/Save/Delete operations happen primarily through the auto-run sequence and CIM commands, not through the manual UI.

5. **Backup on save**: Every `SaveRecipe()` creates a timestamped backup copy before overwriting.
