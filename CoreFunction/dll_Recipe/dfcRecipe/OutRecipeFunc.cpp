#include "pch.h"
#include "dfcRecipe.h"
#include "OutRecipeFunc.h"

#ifdef _DEBUG 
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

C_OutRecipeFunc::C_OutRecipeFunc ()
{
	
}

C_OutRecipeFunc::~C_OutRecipeFunc ()
{
	
}

int		C_OutRecipeFunc::GetStickCount ()
{
	return (int)m_recipeMgr.m_sData.m_vecStick.size ();
}

S_StickInfo C_OutRecipeFunc::GetStickItem (int nStick)
{
	if (GetStickCount () > nStick) return m_recipeMgr.m_sData.m_vecStick.at (nStick);
	else						   return S_StickInfo ();
}

int		C_OutRecipeFunc::GetCellCount (int nStick)
{
	if (GetStickCount () > nStick) return (int)m_recipeMgr.m_sData.m_vecStick.at (nStick).m_vecCell.size ();
	else					       return 0;
}

S_CellInfo C_OutRecipeFunc::GetCellItem (int nStick, int nCell)
{
	if (GetStickCount () > nStick && GetCellCount (nStick) > nCell) return m_recipeMgr.m_sData.m_vecStick.at (nStick).m_vecCell.at (nCell);
	else															return S_CellInfo ();
}

void	C_OutRecipeFunc::SetCellItem (int nStick, int nCell, S_CellInfo cellInfo)
{
	if (GetStickCount () > nStick && GetCellCount (nStick) > nCell) {
		m_recipeMgr.m_sData.m_vecStick.at (nStick).m_vecCell.at (nCell) = cellInfo;
	}
}

void	C_OutRecipeFunc::AddStick (S_StickInfo stickInfo)
{
	m_recipeMgr.m_sData.m_vecStick.push_back (stickInfo);
}

void	C_OutRecipeFunc::AddCell (int nStick, S_CellInfo cellInfo)
{
	if (GetStickCount () > nStick) m_recipeMgr.m_sData.m_vecStick.at (nStick).m_vecCell.push_back (cellInfo);
}

CString	C_OutRecipeFunc::GetCellID (int nStick, int nCell)
{
	if (GetStickCount () > nStick) return m_recipeMgr.m_sData.m_vecStick.at (nStick).m_vecCell.at (nCell).m_chCellID;
	else						   return CString ();
}

int		C_OutRecipeFunc::GetCellArea (int nStick, int nCell)
{
	if (GetStickCount () > nStick) return m_recipeMgr.m_sData.m_vecStick.at (nStick).m_vecCell.at (nCell).m_nArea;
	else						   return -1;
}

void	C_OutRecipeFunc::SetCellStatus (int nStick, int nCell, int nMode, int nStatus)
{
	if (GetStickCount () > nStick) m_recipeMgr.m_sData.m_vecStick.at (nStick).m_vecCell.at (nCell).m_nCurStatus[nMode] = nStatus;
}

int		C_OutRecipeFunc::GetCellStatus (int nStick, int nCell, int nMode)
{
	if (GetStickCount () > nStick) return m_recipeMgr.m_sData.m_vecStick.at (nStick).m_vecCell.at (nCell).m_nCurStatus[nMode];
	else						   return -1;
}

void	C_OutRecipeFunc::SetSelectCell (int nStick, int nCell, int nMode, int nSelect)
{
	if (GetStickCount () > nStick) m_recipeMgr.m_sData.m_vecStick.at (nStick).m_vecCell.at (nCell).m_nSelectCell[nMode] = nSelect;
}

int		C_OutRecipeFunc::GetSelectCell (int nStick, int nCell, int nMode)
{
	if (GetStickCount () > nStick) return m_recipeMgr.m_sData.m_vecStick.at (nStick).m_vecCell.at (nCell).m_nSelectCell[nMode];
	else						   return -1;
}

BOOL	C_OutRecipeFunc::LoadRcpFile (CString strRcpName)
{
	CString strPath;
	strPath = theApp.m_strRcpFolder + strRcpName + _T(".rcp");

	CStdioFile File;
	if (!File.Open (strPath, CFile::modeRead)) return FALSE;

	CString strTemp, strTemp2, strTemp3, strTemp4;
	CStringArray strArr, strArr2, strArr3, strArr4;
	int nVersion = 0;//, nTemp = 0;

	//Version Header
	ReadString (&File);
	//Version Data
	nVersion = ReadInt (&File);//if (nVersion > RECIPE_FILE_V1000)

	//CellInfo Header
	ReadString (&File);
	//CellInfo Data
	SetModelInfoName (ReadString (&File));
	m_recipeMgr.m_sMap.m_fCellSize.x	 = ReadDouble (&File);
	m_recipeMgr.m_sMap.m_fCellSize.y	 = ReadDouble (&File);
	m_recipeMgr.m_sMap.m_fCellDistance.x = ReadDouble (&File);
	m_recipeMgr.m_sMap.m_fCellDistance.y = ReadDouble (&File);
	m_recipeMgr.m_sMap.m_fCellCount.x    = ReadInt (&File);
	if (nVersion > RECIPE_FILE_V1003) m_recipeMgr.m_sMap.m_fCellCount.y = ReadInt (&File);
	m_recipeMgr.m_sMap.m_fContactOffset  = ReadDouble (&File);

	//Option Header
	ReadString (&File);
	//Option Data
	m_recipeMgr.m_sInfo.m_sOptionItem.m_bUseRtestNgPass = ReadInt (&File); 
	m_recipeMgr.m_sInfo.m_sOptionItem.m_bUseSampling	= ReadInt (&File); 
	m_recipeMgr.m_sInfo.m_sOptionItem.m_bUseAlign		= ReadInt (&File); 
	m_recipeMgr.m_sInfo.m_sOptionItem.m_bUseReviewSnap  = ReadInt (&File); 
	m_recipeMgr.m_sInfo.m_sOptionItem.m_nGlsRotate		= ReadInt (&File); 
	m_recipeMgr.m_sInfo.m_sOptionItem.m_fFrontHeadGap   = ReadDouble (&File);
	m_recipeMgr.m_sInfo.m_sOptionItem.m_fBackHeadGap    = ReadDouble (&File);
	wcscpy_s (m_recipeMgr.m_sInfo.m_sOptionItem.m_chAlignName, ReadString (&File));

	//InspectionItem Header
	ReadString (&File);
	//InspectionInfo Data
	m_recipeMgr.m_sInfo.m_sInspectItem.m_bUsePre_0		 = ReadInt (&File); 
	m_recipeMgr.m_sInfo.m_sInspectItem.m_bUseElect_1	 = ReadInt (&File); 
	m_recipeMgr.m_sInfo.m_sInspectItem.m_bUseAgingTest_2 = ReadInt (&File); 
	m_recipeMgr.m_sInfo.m_sInspectItem.m_bUseSpec_3		 = ReadInt (&File); 
	m_recipeMgr.m_sInfo.m_sInspectItem.m_bUseQuality_4   = ReadInt (&File); 
	m_recipeMgr.m_sInfo.m_sInspectItem.m_bUseColor_5	 = ReadInt (&File); 
	m_recipeMgr.m_sInfo.m_sInspectItem.m_bUseAngle_6	 = ReadInt (&File); 
	m_recipeMgr.m_sInfo.m_sInspectItem.m_bUseEFA		 = ReadInt (&File);
	m_recipeMgr.m_sInfo.m_sInspectItem.m_bUseSpec_Multi  = ReadInt (&File);

	// LYS 2026-08-06 강화검사
	if (nVersion >= RECIPE_FILE_V1006) {
		m_recipeMgr.m_sInfo.m_sInspectItem.m_bUseAging_Test = ReadInt(&File);
	}
	else {
		m_recipeMgr.m_sInfo.m_sInspectItem.m_bUseAging_Test = 0; // 기본값
	}	

	strTemp = ReadString (&File);
	dfcArryParsing (&strArr, strTemp);
	for (int i = 0; i < M_RCP_INS_ITEM; i++) {
		m_recipeMgr.m_sInfo.m_sInspectItem.m_nTimeOut[i] = _wtoi (strArr.GetAt (i));
	}

	//ProbeID Header
	ReadString (&File);
	//ProbeID Data
	strTemp = ReadString (&File);
	dfcArryParsing (&strArr, strTemp);
	strTemp2 = ReadString (&File);
	dfcArryParsing (&strArr2, strTemp2);
	for (int i = 0; i < M_RCP_PROBE_MAX; i++) {
		wcscpy_s (m_recipeMgr.m_sInfo.m_sProbeInfo.m_chProbeFirst[i], strArr.GetAt (i));
		wcscpy_s (m_recipeMgr.m_sInfo.m_sProbeInfo.m_chProbeSecond[i], strArr2.GetAt (i));
	}
	m_recipeMgr.m_sInfo.m_sProbeInfo.m_nProbePriority = ReadInt (&File);

	//ContactPos Header
	ReadString (&File);
	//ContactPos Data
	m_recipeMgr.m_sInfo.m_sContactPos.m_fReadyPos	   = ReadDouble (&File);
	m_recipeMgr.m_sInfo.m_sContactPos.m_fPreContactPos = ReadDouble (&File);
	m_recipeMgr.m_sInfo.m_sContactPos.m_fEndContactPos = ReadDouble (&File);
	m_recipeMgr.m_sInfo.m_sContactPos.m_fAddOD         = ReadDouble (&File);

	//Lamp Header
	ReadString (&File);
	//Lamp Data
	m_recipeMgr.m_sInfo.m_sLampItem.m_nFrontLeft  = ReadInt (&File); 
	m_recipeMgr.m_sInfo.m_sLampItem.m_nFrontRight = ReadInt (&File); 
	m_recipeMgr.m_sInfo.m_sLampItem.m_nBackLeft	  = ReadInt (&File); 
	m_recipeMgr.m_sInfo.m_sLampItem.m_nBackRight  = ReadInt (&File); 

	//HeadOffset Header
	ReadString (&File);
	//HeadOffset Data
	m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fStageY		  = ReadDouble (&File);
	m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fStageRotate	  = ReadDouble (&File);
	m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fHeadReviewY	  = ReadDouble (&File);
	m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fHeadReviewLeft  = ReadDouble (&File);
	m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fHeadReviewRight = ReadDouble (&File);
	m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_nQualityRotate	  = ReadInt (&File);
	m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fQualityLeftRot  = ReadDouble (&File);
	m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fQualityRightRot = ReadDouble (&File);

	if (nVersion > RECIPE_FILE_V1004) m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fProbeShift = ReadDouble(&File);	// jsyon 2022.05.13 add
	else m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fProbeShift = 0.;


	//데이터 초기화
	ClearRcpData ();

	//Stage Header
	ReadString (&File);
	//Stage Data
	strTemp = _T(""), strTemp2 = _T(""), strTemp3 = _T("");
	m_recipeMgr.m_nStickCnt = ReadInt (&File);
	strTemp = ReadString (&File);
	dfcArryParsing (&strArr, strTemp);
	strTemp2 = ReadString (&File);
	dfcArryParsing (&strArr2, strTemp2);
	strTemp3 = ReadString (&File);
	dfcArryParsing (&strArr3, strTemp3);

	S_StickInfo sStick;
	for (int i = 0; i < m_recipeMgr.m_nStickCnt; i++) {
		sStick.m_nStickNum = _wtoi (strArr.GetAt (i));
		sStick.m_fStageY   = _wtof (strArr2.GetAt (i));
		sStick.m_fStageR   = _wtof (strArr3.GetAt (i));

		AddStick (sStick);
	}

	//HeadZ Header
	ReadString (&File);
	//HeadZ Data
	m_recipeMgr.m_sData.m_sHeadZ.m_fReviewZ	 = ReadDouble (&File);
	m_recipeMgr.m_sData.m_sHeadZ.m_fSpecZ	 = ReadDouble (&File);
	m_recipeMgr.m_sData.m_sHeadZ.m_fQualityZ = ReadDouble (&File);
	m_recipeMgr.m_sData.m_sHeadZ.m_fColorZ	 = ReadDouble (&File);
	m_recipeMgr.m_sData.m_sHeadZ.m_fAngleZ	 = ReadDouble (&File);
	m_recipeMgr.m_sData.m_sHeadZ.m_fZoom	 = ReadDouble (&File);
	m_recipeMgr.m_sData.m_sHeadZ.m_fFocus	 = ReadDouble (&File);

	//Head Header
	ReadString (&File);
	//Head Data
	m_recipeMgr.m_nCellCnt = ReadInt (&File);

	S_CellInfo sCell;
	for (int i = 0; i < m_recipeMgr.m_nStickCnt; i++) {
		strTemp = ReadString (&File);
		dfcArryParsing (&strArr, strTemp);
		strTemp2 = ReadString (&File);
		dfcArryParsing (&strArr2, strTemp2);
		strTemp3 = ReadString (&File);
		dfcArryParsing (&strArr3, strTemp3);
		strTemp4 = ReadString (&File);
		dfcArryParsing (&strArr4, strTemp4);

		for (int j = 0; j < m_recipeMgr.m_nCellCnt; j++) {
			wcscpy_s (sCell.m_chCellID, strArr.GetAt (j));
			sCell.m_fCellY = _wtof (strArr2.GetAt (j));
			sCell.m_fCellX = _wtof (strArr3.GetAt (j));
			sCell.m_nArea   = _wtoi (strArr4.GetAt (j));

			AddCell (i, sCell);
		}
	}

	//Cell Select Header
	ReadString (&File);
	//Cell Select Data
	for (int i = 0; i < m_recipeMgr.m_nStickCnt; i++) {
		strTemp = ReadString (&File);
		dfcArryParsing (&strArr, strTemp);
		strTemp2 = ReadString (&File);
		dfcArryParsing (&strArr2, strTemp2);
		strTemp3 = ReadString (&File);
		dfcArryParsing (&strArr3, strTemp3);
		strTemp4 = ReadString (&File);
		dfcArryParsing (&strArr4, strTemp4);

		for (int j = 0; j < m_recipeMgr.m_nCellCnt; j++) {
			sCell = GetCellItem (i, j);
			sCell.m_nSelectCell[M_SPEC]	   = _wtoi (strArr.GetAt (j));
			sCell.m_nSelectCell[M_QUALITY] = _wtoi (strArr2.GetAt (j));
			sCell.m_nSelectCell[M_COLOR]   = _wtoi (strArr3.GetAt (j));
			sCell.m_nSelectCell[M_ANGLE]   = _wtoi (strArr4.GetAt (j));

			SetCellItem (i, j, sCell);
		}
	}

	if (nVersion > RECIPE_FILE_V1001) {
		//CAD Info Header
		ReadString (&File);
		//CAD Info Data
		strTemp = ReadString (&File);
		dfcArryParsing (&strArr, strTemp);
		strTemp2 = ReadString (&File);
		dfcArryParsing (&strArr2, strTemp2);

		for (int i = 0; i < CAD_ROW_P1_1; i++) {//CAD_ROW_A ~ CAD_ROW_X7
			m_recipeMgr.m_sMap.m_fCadData[i] = _wtof (strArr.GetAt (i));
		}
		for (int i = 0; i < CAD_ROW_END - CAD_ROW_P1_1; i++) {//CAD_ROW_P1_1 ~ CAD_ROW_P5_2
			wcscpy_s (m_recipeMgr.m_sMap.m_chCadData[i], strArr2.GetAt (i));
		}
	}

	if (nVersion > RECIPE_FILE_V1002) {
		m_recipeMgr.m_sMap.m_fGlobalMark.x = ReadDouble (&File);
		m_recipeMgr.m_sMap.m_fGlobalMark.y = ReadDouble (&File);
		m_recipeMgr.m_sMap.m_fCellCenter.x = ReadDouble (&File);
		m_recipeMgr.m_sMap.m_fCellCenter.y = ReadDouble (&File);

		wcscpy_s (m_recipeMgr.m_sMap.m_chCellCount,    ReadString (&File));
		wcscpy_s (m_recipeMgr.m_sMap.m_chCellDistance, ReadString (&File));
	}

	// LYS 2026-07-08 Sub Z Load (V1005+)
	if (nVersion > RECIPE_FILE_V1005) {
		ReadString(&File);	//Head Sub Z Header
		m_recipeMgr.m_sInfo.m_sMotionSubZ.m_fWadSubzRight = ReadDouble(&File);
		m_recipeMgr.m_sInfo.m_sMotionSubZ.m_fWadSubzLeft = ReadDouble(&File);
		m_recipeMgr.m_sInfo.m_sMotionSubZ.m_fVisionSubzRight = ReadDouble(&File);
		m_recipeMgr.m_sInfo.m_sMotionSubZ.m_fVisionSubzLeft = ReadDouble(&File);
		//m_recipeMgr.m_sInfo.m_sMotionSubZ.m_fAngleSubzRight = ReadDouble(&File);
		//m_recipeMgr.m_sInfo.m_sMotionSubZ.m_fAngleSubzLeft = ReadDouble(&File);
	} else {
		m_recipeMgr.m_sInfo.m_sMotionSubZ.Clear();
	}

	// LYS 2026-08-19 Angle Axis Load (V1007+)
	if (nVersion > RECIPE_FILE_V1007) {
		ReadString(&File);	//Angle Axis Header
		m_recipeMgr.m_sInfo.m_sMotionAngle.m_fAngleScheimpflug1 = ReadDouble(&File);
		m_recipeMgr.m_sInfo.m_sMotionAngle.m_fAngleRotate1 = ReadDouble(&File);
		m_recipeMgr.m_sInfo.m_sMotionAngle.m_fAngleZoom1 = ReadDouble(&File);
		m_recipeMgr.m_sInfo.m_sMotionAngle.m_fAngleTilt1 = ReadDouble(&File);
		m_recipeMgr.m_sInfo.m_sMotionAngle.m_fAngleAzimuth1 = ReadDouble(&File);
		m_recipeMgr.m_sInfo.m_sMotionAngle.m_fAngleScheimpflug2 = ReadDouble(&File);
		m_recipeMgr.m_sInfo.m_sMotionAngle.m_fAngleRotate2 = ReadDouble(&File);
		m_recipeMgr.m_sInfo.m_sMotionAngle.m_fAngleZoom2 = ReadDouble(&File);
		m_recipeMgr.m_sInfo.m_sMotionAngle.m_fAngleTilt2 = ReadDouble(&File);
		m_recipeMgr.m_sInfo.m_sMotionAngle.m_fAngleAzimuth2 = ReadDouble(&File);
	} else {
		m_recipeMgr.m_sInfo.m_sMotionAngle.Clear();
	}

	File.Close ();

	return TRUE;
}

BOOL	C_OutRecipeFunc::LoadAlignRcpFile (CString strAlignRcpName)
{
	CString strPath;
	strPath = theApp.m_strAlignFolder + strAlignRcpName + _T(".alg");

	CStdioFile File;
	if (!File.Open (strPath, CFile::modeRead)) return FALSE;

	CString strTemp, strTemp2, strTemp3, strTemp4;
	CStringArray strArr, strArr2, strArr3, strArr4;
	int nVersion = 0;//, nTemp = 0;

	////Version Header
	//ReadString (&File);
	////Version Data
	//nVersion = ReadInt (&File);//if (nVersion > RECIPE_FILE_V1000)

	m_recipeMgr.m_sAlign.Clear ();

	//Glass Align Header
	ReadString (&File);
	//Glass Align Data
	m_recipeMgr.m_sAlign.m_sGlsAlign.m_fBase_StageY		= ReadDouble (&File);
	m_recipeMgr.m_sAlign.m_sGlsAlign.m_fBase_FrontLeft	= ReadDouble (&File);
	m_recipeMgr.m_sAlign.m_sGlsAlign.m_fBase_FrontRight	= ReadDouble (&File);

	m_recipeMgr.m_sAlign.m_sGlsAlign.m_fStageY			= ReadDouble (&File);
	m_recipeMgr.m_sAlign.m_sGlsAlign.m_fStageRotate		= ReadDouble (&File);
	m_recipeMgr.m_sAlign.m_sGlsAlign.m_fFrontY			= ReadDouble (&File);
	m_recipeMgr.m_sAlign.m_sGlsAlign.m_fFrontLeft		= ReadDouble (&File);
	m_recipeMgr.m_sAlign.m_sGlsAlign.m_fFrontRight		= ReadDouble (&File);
	m_recipeMgr.m_sAlign.m_sGlsAlign.m_fFrontZ			= ReadDouble (&File);

	//Probe Align Header
	ReadString (&File);
	//Probe Align Data
	m_recipeMgr.m_sAlign.m_sProbeAlign.m_fBase_ProbeY	 = ReadDouble (&File);
	m_recipeMgr.m_sAlign.m_sProbeAlign.m_fBase_BackLeft	 = ReadDouble (&File);
	m_recipeMgr.m_sAlign.m_sProbeAlign.m_fBase_BackRight = ReadDouble (&File);

	m_recipeMgr.m_sAlign.m_sProbeAlign.m_fProbeY		 = ReadDouble (&File);
	m_recipeMgr.m_sAlign.m_sProbeAlign.m_fProbeX		 = ReadDouble (&File);
	m_recipeMgr.m_sAlign.m_sProbeAlign.m_fBackY			 = ReadDouble (&File);
	m_recipeMgr.m_sAlign.m_sProbeAlign.m_fBackLeft		 = ReadDouble (&File);
	m_recipeMgr.m_sAlign.m_sProbeAlign.m_fBackRight		 = ReadDouble (&File);
	m_recipeMgr.m_sAlign.m_sProbeAlign.m_fBackZ			 = ReadDouble (&File);
	m_recipeMgr.m_sAlign.m_sProbeAlign.m_fStageY		 = ReadDouble (&File);

	//CCD Set Header
	ReadString (&File);
	//CCD Set Data
	m_recipeMgr.m_sAlign.m_sCcdSet.m_sFrontLeft.m_nExposureTime	  = ReadInt (&File);
	m_recipeMgr.m_sAlign.m_sCcdSet.m_sFrontRight.m_nExposureTime  = ReadInt (&File);
	m_recipeMgr.m_sAlign.m_sCcdSet.m_sBackLeft.m_nExposureTime	  = ReadInt (&File);
	m_recipeMgr.m_sAlign.m_sCcdSet.m_sBackRight.m_nExposureTime	  = ReadInt (&File);
																			  
	m_recipeMgr.m_sAlign.m_sCcdSet.m_sFrontLeft.m_nGain			  = ReadInt (&File);
	m_recipeMgr.m_sAlign.m_sCcdSet.m_sFrontRight.m_nGain		  = ReadInt (&File);
	m_recipeMgr.m_sAlign.m_sCcdSet.m_sBackLeft.m_nGain			  = ReadInt (&File);
	m_recipeMgr.m_sAlign.m_sCcdSet.m_sBackRight.m_nGain			  = ReadInt (&File);
																			  
	m_recipeMgr.m_sAlign.m_sCcdSet.m_sFrontLeft.m_nLed			  = ReadInt (&File);
	m_recipeMgr.m_sAlign.m_sCcdSet.m_sFrontRight.m_nLed			  = ReadInt (&File);
	m_recipeMgr.m_sAlign.m_sCcdSet.m_sBackLeft.m_nLed			  = ReadInt (&File);
	m_recipeMgr.m_sAlign.m_sCcdSet.m_sBackRight.m_nLed			  = ReadInt (&File);

	m_recipeMgr.m_sAlign.m_sCcdSet.m_sFrontLeft.m_nMatchingScore  = ReadInt (&File);
	m_recipeMgr.m_sAlign.m_sCcdSet.m_sFrontRight.m_nMatchingScore = ReadInt (&File);
	m_recipeMgr.m_sAlign.m_sCcdSet.m_sBackLeft.m_nMatchingScore	  = ReadInt (&File);
	m_recipeMgr.m_sAlign.m_sCcdSet.m_sBackRight.m_nMatchingScore  = ReadInt (&File);

	//Reference Image Header
	ReadString (&File);
	//Reference Image Data
	strTemp = ReadString (&File);
	dfcArryParsing (&strArr, strTemp);
	strTemp2 = ReadString (&File);
	dfcArryParsing (&strArr2, strTemp2);
	strTemp3 = ReadString (&File);
	dfcArryParsing (&strArr3, strTemp3);
	strTemp4 = ReadString (&File);
	dfcArryParsing (&strArr4, strTemp4);

	for (int i = 0; i < M_RCP_PROBE_MAX; i++) {
		wcscpy_s (m_recipeMgr.m_sAlign.m_chPtnFrontLeft[i],  strArr.GetAt (i));
		wcscpy_s (m_recipeMgr.m_sAlign.m_chPtnFrontRight[i], strArr2.GetAt (i));
		wcscpy_s (m_recipeMgr.m_sAlign.m_chPtnBackLeft[i],   strArr3.GetAt (i));
		wcscpy_s (m_recipeMgr.m_sAlign.m_chPtnBackRight[i],  strArr4.GetAt (i));
	}

	//Margin Header
	ReadString (&File);
	//Margin Data
	m_recipeMgr.m_sAlign.m_sMargin.m_sGlassMargin.m_fX			= ReadDouble (&File);
	m_recipeMgr.m_sAlign.m_sMargin.m_sGlassMargin.m_fY			= ReadDouble (&File);
	m_recipeMgr.m_sAlign.m_sMargin.m_sGlassMargin.m_fR			= ReadDouble (&File);
	m_recipeMgr.m_sAlign.m_sMargin.m_sGlassMargin.m_nCentering  = ReadInt (&File);

	m_recipeMgr.m_sAlign.m_sMargin.m_sProbeMargin.m_fX			= ReadDouble (&File);
	m_recipeMgr.m_sAlign.m_sMargin.m_sProbeMargin.m_fY			= ReadDouble (&File);
	m_recipeMgr.m_sAlign.m_sMargin.m_sProbeMargin.m_fR			= ReadDouble (&File);
	m_recipeMgr.m_sAlign.m_sMargin.m_sProbeMargin.m_nCentering  = ReadInt (&File);
	m_recipeMgr.m_sAlign.m_sMargin.m_sProbeMargin.m_nFindPtn	= ReadInt (&File);

	File.Close ();

	return TRUE;
}