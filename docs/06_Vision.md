# 06. Vision System Analysis - ALPHA HMI

## 1. System Overview

The ALPHA HMI vision system handles camera-based alignment (Glass and Probe) and pattern matching for display inspection automation equipment. The system uses:

- **Camera Hardware**: Basler GigE cameras (acA2040-25gc model referenced in code)
- **Image Processing SDK**: NI Vision (National Instruments) - `nivision.h`, `niimaq.h`, `NIIMAQdx.h`
- **Frame Grabber Interface**: NI-IMAQdx (GigE Vision driver)
- **Pattern Matching**: NI Vision `imaqMatchPattern3()` / `imaqMatchColorPattern()`
- **External Vision Instruments**: TCP/IP-based measurement devices (spectroscopy, quality inspection)

### Camera Count

Up to 4 align cameras are supported, instantiated as `CAlignCameraMgr` with index 0-3:
- Camera 0: `ALIGN_CAMERA1_MGR` (Front Left - Glass Align)
- Camera 1: `ALIGN_CAMERA2_MGR` (Front Right - Glass Align)
- Camera 2: `ALIGN_CAMERA3_MGR` (Back Left - Probe Align, used during probe change)
- Camera 3: `ALIGN_CAMERA4_MGR` (Back Right - Probe Align, used during probe change)

Normal probe align uses cameras 0-1 (indices `iCCD[0]=0, iCCD[1]=1`). When `ProbeChange==1`, switches to cameras 2-3 (`iCCD[0]=2, iCCD[1]=3`).

---

## 2. Architecture Layers

```
+-----------------------------+
|   Sequence Layer            |
|   DoWorkGlassSWAlign        |  Step machine: None -> MoveAlignPos -> AlignGlass -> Idle
|   DoWorkProbeSWAlign        |  Step machine: None -> Move -> Align -> Idle
+-----------------------------+
         |
+-----------------------------+
|   Manager Layer             |
|   CAlignCameraMgr           |  Per-camera manager, wraps CFrameGrabberFunction
|     - SnapImage()           |  Thread-safe (CCriticalSection)
|     - PtrnMatch()           |  Snap + LoadPattern + SearchPattern
|     - CalcAngle()           |  Angle computation between base and detected points
|     - SaveImage()           |  PNG save with recipe/glass ID path
+-----------------------------+
         |
+-----------------------------+
|   CoreFunction Layer        |
|   CFrameGrabberFunction     |  Device-agnostic grabber abstraction
|     CFrameGrabberImpl       |  Virtual base class (Strategy pattern)
|       CFrameGrabber_GigE_BS4M  -> CFrameGrabber_BS4M  (NI-IMAQdx)
+-----------------------------+
         |
+-----------------------------+
|   Image Processing Layer    |
|   CIMAQImageCtrl            |  NI Vision wrapper: display, overlay, pattern search
|   CPatternMatchFunction     |  Thin wrapper (ProcessDirectMatch, ProcessCorrelation)
|   CGlassAlignFunction       |  Align matrix calculation (SetParamRefPos/DetPos, CalcAlignMatrix)
+-----------------------------+
         |
+-----------------------------+
|   UI Layer                  |
|   CDlgAlignCCD              |  Live view dialog with grab thread
|   CD_CCD_Review             |  Full-screen review dialog
|   CD_RoiSet                 |  ROI definition popup (pattern teach)
+-----------------------------+
```

---

## 3. Frame Grabber System

### 3.1 Class Hierarchy

| Class | File | Role |
|-------|------|------|
| `CFrameGrabberFunction` | `CoreFunction/FrameGrabberFunction/FrameGrabberFunction.h/.cpp` | High-level grabber facade, INI config, delegates to impl |
| `CFrameGrabberImpl` | `CoreFunction/FrameGrabberFunction/FrameGrabberImpl.h/.cpp` | Abstract base class (virtual methods, default no-op) |
| `CFrameGrabber_GigE_BS4M` | `CoreFunction/FrameGrabberFunction/GigECamera_Basler/FrameGrabber_GigE_BS4M.h/.cpp` | GigE adapter, delegates to `CFrameGrabber_BS4M` |
| `CFrameGrabber_BS4M` | `CoreFunction/FrameGrabberFunction/GigECamera_Basler/FrameGrabber_BS4M.h/.cpp` | Actual NI-IMAQdx camera implementation |

Other camera types exist in commented-out code (CL_CAMERA_8M_NI_BOARD, CL_CAMERA_29M_MATROX, CL_CAMERA_SP5000C_NI_BOARD, GIGE_CAMERA_VIEWORKS). Only `GIGE_CAMERA_BASLER` is active.

### 3.2 Camera Initialization Flow

```
CFrameGrabberFunction::CreateGrabber()
  -> CFrameGrabber_BS4M::InitializeGrab()
       1. InitInterface()     - IMAQdxOpenCamera(interfaceName, Controller, &sessionId)
                               - Reads width/height from camera attributes
       2. InitCamSerial()     - 3x IMAQdxSnap() to clear memory lock buffers
       3. InitGrabSetup()     - IMAQdxConfigureGrab() + IMAQdxStartAcquisition()
       4. InitResource()      - Sets pixel format to "YUV 422 Packed"
  -> Sets default exposure=9000, gain=10
```

### 3.3 Image Acquisition

Two acquisition modes:

| Method | Description |
|--------|-------------|
| `SnapImage(short* pbyImage)` | Grabs RGB image via `IMAQdxGrab()`, returns NI `Image*` cast to `short*` via `imaqDuplicate()` |
| `GetSnapImage(long nExpTime_ms, BYTE* pbyImage)` | Grabs U8 image via `IMAQdxGrab()`, converts to RGB, applies rotation if set, copies BGRA pixel data to byte buffer |

### 3.4 Image Data Format

- Camera acquires in IMAQ_IMAGE_U8 (grayscale)
- Converted to IMAQ_IMAGE_RGB via `imaqCast()`
- `CopyImageData()` extracts R/G/B planes and interleaves to BGRA (4 bytes/pixel):
  - Stride = `(width + 3) / 4 * 4 * 4`
  - Layout: `[B, G, R, 0x00]` per pixel
- Optional image rotation via `imaqRotate2()` with bilinear interpolation

### 3.5 Camera Parameters

| Parameter | IMAQdx Attribute | Range |
|-----------|-----------------|-------|
| Gain | `CameraAttributes::AnalogControls::GainRaw` | 300-850 |
| Exposure | `CameraAttributes::AcquisitionTrigger::ExposureTimeRaw` | 16-90000 (microseconds) |
| Bayer Pattern | Local member (4 patterns: GBGB_RGRG, GRGR_BGBG, BGBG_GRGR, RGRG_GBGB) | 0-3 |
| Rotation | Local member | 0, 90, 180, 270 degrees |
| White Balance | R/G/B doubles | Per-channel multiplier |

### 3.6 INI Configuration (per camera section)

Config loaded from `CoreFunction/CoreFunction.ini` under sections `ALIGN_CAMERA1_MGR` through `ALIGN_CAMERA4_MGR`:

| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `USE` | bool | true | Camera enable |
| `OUT_OF_SERVICE` | bool | true | Out-of-service flag |
| `DEVICE_TYPE` | string | `CL_CAMERA_SP5000C_NI_BOARD` | Camera model selector |
| `FILE_SAVE_PATH` | string | `IMAGE` | Image save subfolder |
| `GRAB_WAIT_COUNT` | long | 2 | Grab wait retry count |
| `FRAME_RATE_COUNT(fps)` | double | 1.0 | Frame rate |
| `INTERFACE_NAME` | string | `img0` | NI-IMAQdx interface name |
| `GRAB_MODE` | string | `1SHOT` | Grab mode (1SHOT or 4SHOT) |
| `CCD_BAYER_PATTERN` | long | 1 | Bayer pattern index |
| `CCD_ROTATE_ANGLE` | long | 0 | Image rotation angle |
| `CCD_WHITE_BALANCE_R/G/B` | double | 1.0 | White balance per channel |

### 3.7 Camera Reset

`CFrameGrabber_BS4M::ResetGrabber()`:
1. `IMAQdxStopAcquisition()`
2. `IMAQdxCloseCamera()`
3. `Sleep(1000)`
4. Re-initialize: `InitInterface()` -> `InitCamSerial()` -> `InitGrabSetup()`

Triggered automatically after 3 consecutive grab failures in `CDlgAlignCCD::GrabImage()`.

---

## 4. Pattern Matching System

### 4.1 CIMAQImageCtrl - Core Pattern Engine

**File**: `CoreFunction/common/IMAQImageCtrl/IMAQImageCtrl.h/.cpp`

Central class wrapping NI Vision for display, overlay, and pattern matching.

#### Key Functions

| Function | Description |
|----------|-------------|
| `SearchPattern()` | Main pattern match: learn + search + result selection |
| `PtrnLearning()` | Static: calls `imaqLearnPattern()` on U8 image |
| `LoadPattern()` | Loads PNG pattern file into NI Image |
| `GetDistancePtrnToCntr()` | Calculates distance from match position to image center |
| `CalcAngle2()` | Computes angle between two line segments (base vs detected) |

#### SearchPattern() Flow

```
SearchPattern(pImg, pPtrnImg, minMatchScore, bLearn, iMatchMethod, bMeasPtrn, ...)
  1. LEARN PHASE:
     - If color match: imaqLearnColorPattern()
     - Else: convert to U8, call PtrnLearning() -> imaqLearnPattern()
  
  2. SET ROI:
     - Full image minus 20px margin
     - imaqAddRotatedRectContour2()
  
  3. MATCH PHASE:
     - Convert source to U8 if needed
     - imaqMatchPattern3(pImgBW, pPtrnImgBW, &options, NULL, roi, &nMatch)
     - Options: mode=DEFAULT_MATCH_MODE, subpixel=DEFAULT_SUBPIXEL
     - numMatchesRequested: 10 (for measure) or DEFAULT_NUM_MATCHES (for align)
  
  4. RESULT SELECTION (when nMatch > 1):
     - MATCH_SCORE: Highest score wins, distance tiebreaker
     - MATCH_DISTANCE: Nearest to image center wins
     - MATCH_POSITION: Nearest to specified position, score tiebreaker
  
  5. POST-PROCESSING:
     - Edge detection (Sobel) for refinement
     - Fine centering via FineCentering
```

### 4.2 CPtrnInfo - Pattern Information Structure

**File**: `CoreFunction/ShareMemFunction/CPtrnInfo.h`

| Member | Type | Description |
|--------|------|-------------|
| `m_pImgInput` | Image* | Input (grabbed) image |
| `m_pImgOrg` | Image* | Original pattern template image |
| `m_pImgScaled` | Image* | Scaled pattern for UI display |
| `m_rectPtrn` | CRect | Pattern ROI rectangle |
| `m_dThreshold` | double | Matching threshold |
| `m_iFindNum` | int | Retry number |
| `m_iCntrNum` | int | Centering number |
| `m_dRealX, m_dRealY` | double | Detected real-world coordinates (mm) |
| `m_strName` | CString | Pattern file name |
| `m_cMultiPtrn[10]` | TCHAR[10][256] | Multi-pattern names for sequential search |
| `m_iCCDGain` | int | Per-pattern CCD gain |
| `m_iExposureTime` | int | Per-pattern exposure time |
| `m_iLampIntensity` | int | Per-pattern lamp intensity |
| `m_iMethod` | int | Match method selector |
| `m_nThreshold` | int | Score threshold |

### 4.3 CPatternMatchFunction

**File**: `CoreFunction/PatternMatchFunction/PatternMatchFunction.h`

Wrapper class around `CPatternMatchImpl` (implementation hidden). Provides:

| Method | Description |
|--------|-------------|
| `ProcessDirectMatch()` | Direct pattern match with search ROI |
| `ProcessCorrelation()` | Correlation-based match with search ROI |
| `ProcessMultipMatch()` | Multiple pattern match on source image |
| `GetResultShiftX/Y()` | Pixel shift from center |
| `GetResultScore()` | Match score for given index |
| `GetResultFoundROI()` | Bounding rect of found pattern |
| `GetResultFoundIndex()` | Index of best match |

### 4.4 CAlignCameraMgr::PtrnMatch() - Integrated Match Pipeline

**File**: `Source/ALPHA/Manager/AlignCameraMgr.cpp`

```
PtrnMatch(pPtrnInfo, &Score, nMeasLens, bGlass)
  1. SnapImage(m_pSnapImage)           -- Grab current frame
  2. imaqDuplicate(pPtrnInfo->m_pImgInput, m_pSnapImage)
  3. LoadPattern(recipePath + patternName, pPtrnInfo->m_pImgOrg)
     - Primary path: Recipe/PATTERN/<name>
     - Fallback path: Recipe/ALIGN/<name>
  4. SearchPattern(input, pattern, threshold, bLearn, matchMethod, ...)
     - For LENS_USE_ALIGN: MATCH_SCORE mode, no multi-match
     - For LENS_USE_MEAS: MATCH_DISTANCE mode, multi-match enabled
  5. If multi-pattern (m_strName empty): iterate m_cMultiPtrn[0..9]
  6. Calculate real-world offset:
     - mmPerPixel = 3.78 / 1000.0 = 0.00378 mm/pixel
     - midPoint = center of found rect - center of image
     - pPtrnInfo->m_dRealX = midPoint.x * mmPerPixelX
     - pPtrnInfo->m_dRealY = midPoint.y * mmPerPixelY
  7. Overlay result rectangle on image
  8. SaveImage() with type: 0=Glass, 1=Probe, 2=Measure, 3=Contact, 4-8=Aging
```

---

## 5. Glass Alignment Sequence

### 5.1 Sequence Steps (CDoWorkGlassSWAlign)

**Files**: `Source/ALPHA/DoWork/DoWorkGlassSWAlign.cpp`, `DoWorkGlassSWAlignStep.cpp/.h`

```
Step 0: CDoWorkGlassSWAlignStep_None            -- Initialize flags, set process status
Step 1: CDoWorkGlassSWAlignStep_MoveAlignPosistion -- Move to align position
Step 2: CDoWorkGlassSWAlignStep_AlignGlass       -- Execute glass alignment
Step 3: CDoWorkGlassSWAlignStep_Idle             -- Complete, signal done event
```

### 5.2 AlignGlass() Flow

```
AlignGlass()
  1. AlignGlassSearch()     -- Pattern search with retry
  2. AlignGlassTZero()      -- Theta (rotation) zero alignment
  3. AlignGlassXYZero()     -- X/Y position alignment
  4. SetGlassAlignXYR(dMovX, dMovY, dMovR)  -- Store offset result
```

### 5.3 AlignGlassSearch() - Pattern Search with Shift Retry

Uses 2 cameras (Front Left index=0, Front Right index=1) simultaneously:

```
For each camera j=0,1:
  Load up to 5 pattern names from recipe (m_chPtnFrontLeft/Right[0..4])
  Set matching score threshold from recipe
  For each pattern k=0..df_MAX_PTRNINFO(10):
    PtrnMatch(ptrnInfo[k], &Score, LENS_USE_ALIGN)
    If success and Score > MaxScore: record best match
    Break on first success

If both cameras fail (pattern not found):
  Shift Retry Strategy (25 positions x 6 theta values):
    - 25 XY offset positions: [0,0], [+/-0.5,0], [0,+/-0.5], [+/-0.5,+/-0.5], 
                               [+/-0.8,...], [+/-1.2,...] mm
    - 6 theta offsets: +/-0.01, +/-0.02, +/-0.03 degrees
    - Move stage to each (dCurPos + offset), retry pattern match
    - Check for user pause between retries
```

### 5.4 AlignGlassTZero() - Theta Alignment

Iterative centering loop (configurable retry limit from recipe `m_sMargin.m_sGlassMargin.m_nCentering`):

```
do:
  Snap both cameras, pattern match
  Compute base/detect coordinates from current motor positions + pixel offsets
  dRadian = CalcAngle(base[0], base[1], detect[0], detect[1])
  If |dRadian| < margin: DONE
  Else: move AXIS_LOADER_R by dRadian, retry
while (retries < limit)
```

### 5.5 AlignGlassXYZero() - XY Alignment

Iterative centering loop:

```
do:
  Snap both cameras, pattern match (FrontLeft + FrontRight patterns)
  dX = pixel offset from camera 0 * mmPerPixel (direction corrected)
  dY = pixel offset from camera 0 * mmPerPixel
  If |dX| < marginX AND |dY| < marginY: DONE
  Else: move AXIS_X by dY, AXIS_Y1/Y2 by dX, retry
while (retries < limit)

Result: pdMovX = current AXIS_X org position, pdMovY = AXIS_Y1 org - baseline
```

### 5.6 Recipe Align Parameters (S_Align)

**File**: `Source/Include/preDefine.h`

```cpp
struct S_AlignCcdItem {
    int m_nExposureTime;      // Camera exposure
    int m_nGain;              // Camera gain
    int m_nLed;               // LED lamp intensity
    int m_nMatchingScore;     // Pattern match threshold
};

struct S_AlignCcdSet {
    S_AlignCcdItem m_sFrontLeft;   // Glass align camera L
    S_AlignCcdItem m_sFrontRight;  // Glass align camera R
    S_AlignCcdItem m_sBackLeft;    // Probe align camera L
    S_AlignCcdItem m_sBackRight;   // Probe align camera R
};

struct S_AlignMarginItem {
    double m_fX;           // X margin (mm)
    double m_fY;           // Y margin (mm)
    double m_fR;           // Rotation margin (degrees)
    int    m_nCentering;   // Max centering retry count
    int    m_nFindPtn;     // Pattern find mode
};

struct S_Align {
    wchar_t m_chPtnFrontLeft[5][CH_LEN];    // Pattern names for front-left camera
    wchar_t m_chPtnFrontRight[5][CH_LEN];   // Pattern names for front-right camera
    wchar_t m_chPtnBackLeft[5][CH_LEN];     // Pattern names for back-left camera
    wchar_t m_chPtnBackRight[5][CH_LEN];    // Pattern names for back-right camera
    S_GlsAlign    m_sGlsAlign;              // Glass align positions
    S_ProbeAlign  m_sProbeAlign;            // Probe align positions
    S_AlignCcdSet m_sCcdSet;                // CCD parameters per camera
    S_AlignMargin m_sMargin;                // Alignment margins
};
```

---

## 6. Probe Alignment Sequence

### 6.1 Sequence Steps (CDoWorkProbeSWAlign)

**Files**: `Source/ALPHA/DoWork/DoWorkProbeSWAlign.cpp`, `DoWorkProbeSWAlignStep.cpp/.h`

```
Step 0: CDoWorkProbeSWAlignStep_None   -- Initialize flags
Step 1: CDoWorkProbeSWAlignStep_Move   -- Move to probe align position
Step 2: CDoWorkProbeSWAlignStep_Align  -- Execute probe alignment
Step 3: CDoWorkProbeSWAlignStep_Idle   -- Complete, signal done event
```

### 6.2 Probe Selection Logic

Supports 5 probe gantries (P1-P4 AGING, P5 MEAS). Probe index determines axis mapping:

| Probe | iProbe | X Axis | Y Axis | Theta Axis | Head Y (L/R) |
|-------|--------|--------|--------|------------|---------------|
| AGING1 | 0 | AXIS_PROBE_X1 | AXIS_PROBE_Y1 | AXIS_PROBE_X2 | Y3/Y4 |
| AGING2 | 1 | AXIS_PROBE_X3 | AXIS_PROBE_Y2 | AXIS_PROBE_X4 | Y3/Y4 |
| AGING3 | 2 | AXIS_PROBE_X5 | AXIS_PROBE_Y3 | AXIS_PROBE_X6 | Y3/Y4 |
| AGING4 | 3 | AXIS_PROBE_X7 | AXIS_PROBE_Y4 | AXIS_PROBE_X8 | Y3/Y4 |
| MEAS   | 4 | AXIS_PROBE_X9 | AXIS_PROBE_Y5 | AXIS_PROBE_X10 | Y1/Y2 |

### 6.3 Align_Probe() Flow

```
Align_Probe()
  1. Get current probe index from FunctionModuleProbeACC
  2. Verify probe exists (sensor check)
  3. Align_ProbeSearch(iProbe)   -- Pattern search (no retry search here)
  4. Align_ProbeTZero(iProbe)    -- Theta alignment (only during probe change)
  5. Align_ProbeXYZero(iProbe)   -- XY alignment
  6. SetProbeAlignXYR(dMovX, dMovY, dMovR, iProbe)
```

### 6.4 Probe Align Camera Selection

- Normal operation: `iCCD[0]=0, iCCD[1]=1` (front cameras, same as glass align)
- Probe change (`ProbeChange==1`): `iCCD[0]=2, iCCD[1]=3` (back cameras)
- Light channels also switch: normal `iCh={3,2}`, probe change `iCh={0,1}`
- Pattern names: `m_chPtnBackLeft[k]` / `m_chPtnBackRight[k]` (always back patterns)

### 6.5 Align_ProbeTZero() - Probe Theta Alignment

```
do:
  Match patterns on both cameras
  Compute dRadian from pixel differences: (dPixely[1] - dPixely[0])
  If |diff| < margin: DONE
  Else: move probe theta axis (AXIS_PROBE_Xn+1) by corrected amount
while (retries < limit)
```

### 6.6 Align_ProbeXYZero() - Probe XY Alignment

```
do:
  Match patterns on both cameras
  dY = dPixelx[0] * (-1)   // Probe Y
  dX = dPixely[0] * (+1)   // Probe X
  If |dX| < marginX AND |dY| < marginY: DONE
  Else: move probe X axis + probe Y axis by (dX, dY), retry
while (retries < limit)
```

---

## 7. Retry Logic Summary

### 7.1 Grab Retry

- **Location**: `CDlgAlignCCD::GrabImage()` (line 339)
- **Logic**: After 3 consecutive grab failures (`m_nGrabNgCount > 2`), calls `ResetGrabber()`
- `ResetGrabber()` stops acquisition, closes camera, waits 1s, re-initializes

### 7.2 Pattern Search Retry (Glass Align)

- **Location**: `CDoWorkGlassSWAlignStep::AlignGlassSearch()`
- **Outer loop**: 3 iterations of XY shift positions (0, +/-0.5, +/-0.8, +/-1.2 mm)
- **Inner loop**: 6 theta offsets (+/-0.01 to +/-0.05 degrees)
- **Total combinations**: Up to 25 * 6 = 150 attempts (reduced by early break)
- **User pause check** between theta iterations

### 7.3 Centering Retry (Glass/Probe)

- **TZero**: Configurable from recipe `m_sMargin.m_sGlassMargin.m_nCentering` (default 1)
- **XYZero**: Same configurable retry count
- **Probe TZero**: `m_sMargin.m_sProbeMargin.m_nCentering` retries (up to 5 attempts)
- **Probe XYZero**: Same probe margin retry count

### 7.4 Multi-Pattern Fallback

- Up to `df_MAX_PTRNINFO` (10) patterns per camera
- Only first 5 slots loaded from recipe (`k < 5` check)
- Tries each pattern sequentially, keeps best score
- Breaks on first successful match

---

## 8. Live View & UI Components

### 8.1 CDlgAlignCCD - Live Camera View

**File**: `Source/ALPHA/DlgAlignCCD.h/.cpp`

- Runs dedicated grab thread (`WorkThreadProbeProc`)
- Thread loop: `WaitForSingleObject(killEvent, 100ms)` -> `GrabImage()`
- `GrabImage()`: calls `AlignCameraMgr::SnapImage(Image*)` -> posts `WM_MSG_DISPLAY_IMAGE`
- `DisplayImage()`: duplicates image, adds crosshair overlay, calls `imaqDisplayImage()`
- `DisplayImage2()`: sends to `CD_CCD_Review` dialog for full-screen view
- Supports live/static toggle via `m_bCCDLive`

### 8.2 CD_CCD_Review - Full-Screen Review

**File**: `Source/ALPHA/D_CCD_Review.h/.cpp`

- Separate dialog for enlarged image review
- Receives images from `CDlgAlignCCD::DisplayImage2()`
- Uses NI Vision `imaqDisplayImage()` for rendering

### 8.3 CD_RoiSet - ROI Definition (Pattern Teach)

**File**: `Source/ALPHA/D_RoiSet.h/.cpp`

- Popup dialog for defining pattern ROI regions
- Editable X, Y, W, H fields
- Uses `CIMAQImageCtrl` for interactive ROI drawing
- Event handlers: Click, Draw, Move
- Save pattern image functionality

---

## 9. External Vision Communication (CCommTcpIpObjectVision)

**File**: `Source/ALPHA/CommTcpIpObjectVision.h/.cpp`

TCP/IP client to external vision measurement instruments (not the align cameras).

### 9.1 Key Operations

| Method | Description |
|--------|-------------|
| `SetParameter()` | Send measurement parameters to external vision instrument |
| `RecipeChange()` | Command recipe change on instrument |
| `InspectStart()` | Start measurement inspection |
| `InspectStartMulti()` | Multi-area inspection start |
| `InspectEnd()` | End inspection sequence |
| `WaitInspectionEnd()` | Block until inspection complete (with timeout) |
| `WaitRecipeChangeEnd()` | Block until recipe change complete |
| `SetTime_Sync()` | Synchronize time with instrument |
| `RunMoveFilter()` | Move filter wheel on instrument |

### 9.2 Quality Wait System

Double-buffered quality inspection waiting (2 slots: `QUALITY_1`, `QUALITY_2`):
- `QualityWaitSet()`: Queue a quality check request
- `QualityWaitRun()`: Start processing queued request
- `QualityWaitThread()`: Background thread for waiting
- `TossResultFile()`: Transfer result files

---

## 10. CGlassAlignFunction - Align Matrix

**File**: `CoreFunction/GlassAlignFunction/GlassAlignFunction.h`

Wrapper around `CGlassAlignImpl` for affine transformation calculation:

| Method | Description |
|--------|-------------|
| `SetParamRefPos(index, x, y, z)` | Set reference (teach) position |
| `SetParamDetPos(index, x, y, z)` | Set detected (current) position |
| `CalcAlignMatrix()` | Compute transformation matrix |
| `IsAlignComplete()` | Check if alignment is done |
| `GetAlignedMotionPos(x, y, &newX, &newY)` | Transform position through matrix |
| `GetAlignedMatrixPos(x, y, &newX, &newY)` | Get matrix-transformed position |
| `GetAlignResult(&theta, &deltaX, &deltaY)` | Get alignment offsets |

**Note**: In the current DoWork code, align matrix functions (`m_pShareItem->GetAlignMatrixMgr()->...`) are commented out. The system uses direct centering loops instead of matrix-based alignment.

---

## 11. VisionConfig.ini

**File**: `Source/ALPHA/ConfigurationFile/VisionConfig.ini`

Legacy configuration file (from OCP-style system). Contains:

| Section | Content |
|---------|---------|
| `[CCDINTERFACE]` | Camera interface names: MAIN1=img0, MAIN2=img1, ALIGN1=cam0, ALIGN2=cam1 |
| `[CCD]` | Pixel sizes (5.0um main, 3.75um align), shutter speeds (1/125 to 1/40000) |
| `[LENSUSE]` | 5 lens positions with XY offsets and um/pixel calibration |
| `[LENSDB]` | 9 lens database entries (2.5x to 55x, NA, working distance, AF params) |
| `[WHITEUVFILTER]` | 3 filter positions (UV, MAIN, UV-TEG) |

---

## 12. Image Save Strategy

### 12.1 Save Paths

Images saved as PNG via `imaqWritePNGFile2()` with quality=750:

| Type | Path Pattern | Usage |
|------|-------------|-------|
| 0 - Glass Align | `{Result}/ALIGN/{Recipe}/{GlassID}/{Recipe}_{GlassID}_{Time}_GlassAlign_{CCD#}.png` | Glass align result |
| 1 - Probe Align | `{Result}/ALIGN/{Recipe}/{GlassID}/{Recipe}_{GlassID}_{Time}_ProbeAlign_{CCD#}.png` | Probe align result |
| 2 - Measure Pattern | `{Result}/ALIGN/{Recipe}/{GlassID}/{Recipe}_{GlassID}_{Time}_Meas_Ptrn_{CCD#}.png` | Measurement pattern |
| 3 - Contact | `{Result}/Contact/{Recipe}/{GlassID}/{Recipe}_{GlassID}_{Time}_Meas_Ptrn_{CCD#}.png` | Contact check |
| 4-8 - Aging | `{Result}/AGING/{Recipe}/{GlassID}/...AgingPrbe{1-4}/MeasPrbe_{CCD#}.png` | Aging probe images |

### 12.2 Overlay in Saved Images

Before saving, result rectangles are overlaid:
- `imaqOverlayRect()` draws red rectangle around found pattern
- `imaqMergeOverlay()` merges overlay into saved image

---

## 13. Key Constants and Defines

| Constant | Value | Source |
|----------|-------|--------|
| `df_MAX_PTRNINFO` | 10 | `DefinesSys.h`, `ShareMem.h` |
| `INT_IMAGE_BORDER_SIZE` | 7 | `IMAQImageCtrl.h` |
| `INT_MONITOR_WND_WIDTH` | 640 | `FrameGrabberImplDefine.h` |
| `INT_MONITOR_WND_HEIGHT` | 480 | `FrameGrabberImplDefine.h` |
| `INT_MONI_WND_WIDTH` | 2560 | `IMAQImageCtrl.h` |
| `INT_MONI_WND_HEIGHT` | 2048 | `IMAQImageCtrl.h` |
| `LENS_USE_ALIGN` | 0 | `ShareMem.h` |
| `LENS_USE_MEAS` | 1 | `ShareMem.h` |
| `MATCH_SCORE` | 0 | `IMAQImageCtrl.h` |
| `MATCH_DISTANCE` | 1 | `IMAQImageCtrl.h` |
| `MATCH_POSITION` | 2 | `IMAQImageCtrl.h` |
| `FG_IMAGE_BORDER_SIZE` | 3 | `FrameGrabberImplDefine.h` |

---

## 14. Thread Safety

- `CAlignCameraMgr::SnapImage()` and `GetSnapImage()` protected by `CCriticalSection m_cs_get`
- `CDlgAlignCCD` grab thread uses kill/killed events for clean termination
- NI-IMAQdx calls: `IMAQdxStopAcquisition()` must precede parameter changes (gain/exposure)
- Multiple cameras operate independently (separate `CAlignCameraMgr` instances)

---

## 15. Error Codes (Vision-Related)

| Constant | Description |
|----------|-------------|
| `VTM_RV_SUCCESSED` | Success |
| `VISION_SNAP_FAIL` | Image grab failed |
| `VISION_ALIGN_DETECTED_FAIL` | Pattern not found during alignment |
| `VISION_ALIGN_CENTERING_FAIL` | Centering margin not met (triggers retry) |
| `VISION_PATTERN_DETECTED_FAIL` | Pattern not found during measurement |
| `JOB_PROC_ALIGN_POS_ERROR` | Alignment position move exceeded max retries (25) |
| `JOB_PROC_ALIGN_ERROR` | General alignment error |

---

## 16. File Reference

### CoreFunction Layer

| File | Description |
|------|-------------|
| `CoreFunction/FrameGrabberFunction/FrameGrabberFunction.h/.cpp` | High-level grabber facade |
| `CoreFunction/FrameGrabberFunction/FrameGrabberImpl.h/.cpp` | Abstract grabber interface |
| `CoreFunction/FrameGrabberFunction/FrameGrabberImplDefine.h` | Constants and MakeDir utility |
| `CoreFunction/FrameGrabberFunction/GigECamera_Basler/FrameGrabber_GigE_BS4M.h/.cpp` | GigE adapter |
| `CoreFunction/FrameGrabberFunction/GigECamera_Basler/FrameGrabber_BS4M.h/.cpp` | NI-IMAQdx Basler impl |
| `CoreFunction/GlassAlignFunction/GlassAlignFunction.h` | Align matrix wrapper |
| `CoreFunction/PatternMatchFunction/PatternMatchFunction.h` | Pattern match wrapper |
| `CoreFunction/common/IMAQImageCtrl/IMAQImageCtrl.h/.cpp` | NI Vision display/match/overlay |
| `CoreFunction/ShareMemFunction/CPtrnInfo.h` | Pattern info data class |

### Application Layer

| File | Description |
|------|-------------|
| `Source/ALPHA/Manager/AlignCameraMgr.h/.cpp` | Per-camera manager (snap, match, save) |
| `Source/ALPHA/DoWork/DoWorkGlassSWAlign.h/.cpp` | Glass align sequence engine |
| `Source/ALPHA/DoWork/DoWorkGlassSWAlignStep.h/.cpp` | Glass align step implementations |
| `Source/ALPHA/DoWork/DoWorkProbeSWAlign.h/.cpp` | Probe align sequence engine |
| `Source/ALPHA/DoWork/DoWorkProbeSWAlignStep.h/.cpp` | Probe align step implementations |
| `Source/ALPHA/DlgAlignCCD.h/.cpp` | Live camera view dialog |
| `Source/ALPHA/D_CCD_Review.h/.cpp` | Full-screen image review dialog |
| `Source/ALPHA/D_RoiSet.h/.cpp` | ROI definition / pattern teach dialog |
| `Source/ALPHA/CommTcpIpObjectVision.h/.cpp` | External vision instrument TCP/IP comm |

### Configuration

| File | Description |
|------|-------------|
| `Source/ALPHA/ConfigurationFile/VisionConfig.ini` | Camera, lens, filter config |
| `Source/Include/preDefine.h` | S_Align, S_AlignCcdItem, S_AlignMargin structs |
| `Source/Include/DefinesSys.h` | df_MAX_PTRNINFO and other vision constants |
