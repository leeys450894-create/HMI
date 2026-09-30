#include "pch.h"
#include "dfcRecipe.h"
#include "OutRecipeFunc.h"
#include <memory>	// LYS 2026-09-29 unique_ptr

#ifdef _DEBUG 
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

C_OutRecipeFunc::C_OutRecipeFunc ()
{
	m_nActiveSegment = -1;	// LYS 2026-09-29 Hybrid Phase 1
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

	// LYS 2026-09-29 Hybrid Phase 1 로드 전 상태 보관 - 본문/꼬리 어디서든 실패(태그 불일치, 토큰 부족 예외)하면 통째로 되돌린다
	//                (지금까지는 GetAt 범위 초과 예외가 그대로 튀어 앱이 죽었다 - P0-3 HMI.rcp 사례)
	std::unique_ptr<S_RecipeMgr> pBakMgr (new S_RecipeMgr (m_recipeMgr));	// LYS 2026-09-29 스택 오버플로 방지 - S_RecipeMgr는 고정 문자 배열만 약 88KB라 스택에 두지 않는다
	std::vector<S_RecipeMgr>	 vecBakSeg	 = m_vecSegments;
	std::vector<S_SegmentHeader> vecBakHdr	 = m_vecSegHeader;
	std::vector<BOOL>			 vecBakAlign = m_vecSegAlignOK;
	int nBakActive = m_nActiveSegment;
	BOOL bOK = FALSE;

	try {
		int nVersion = 0;

		//Version Header
		ReadString (&File);
		//Version Data
		nVersion = ReadInt (&File);//if (nVersion > RECIPE_FILE_V1000)
		m_recipeMgr.m_nVersion = nVersion;	// LYS 2026-09-29 P0-4 파일 버전 보관 (지금까지 대입이 없어 GetRcpVer()가 항상 0)

		// 본문 = 세그먼트 0 (구 빌드도 여기까지 읽는다)
		bOK = LoadRcpSections (&File, nVersion);

		// 1세그먼트로 초기화 - 꼬리가 없는 구파일은 이대로 끝(N=1)
		m_vecSegments.clear ();
		m_vecSegHeader.clear ();
		m_vecSegAlignOK.clear ();
		m_nActiveSegment = -1;
		EnsureSegments ();

		if (bOK && nVersion > RECIPE_FILE_V1011) bOK = LoadHybridTail (&File, nVersion);

		File.Close ();
	}
	catch (CException *e) {
		e->Delete ();
		bOK = FALSE;
	}

	if (!bOK) {
		m_recipeMgr	  = *pBakMgr;
		m_vecSegments  = vecBakSeg;
		m_vecSegHeader = vecBakHdr;
		m_vecSegAlignOK = vecBakAlign;
		m_nActiveSegment = nBakActive;
		return FALSE;
	}

	return TRUE;
}

// LYS 2026-09-29 Hybrid Phase 1 레시피 본문 섹션 읽기 - LoadRcpFile에서 코드 이동(논리 무변경). m_recipeMgr에 쓴다.
//   세그먼트 0과 꼬리 블록의 세그먼트 k>=1이 같은 코드를 쓴다.
BOOL	C_OutRecipeFunc::LoadRcpSections (CStdioFile *pFile, int nVersion)
{
	CString strTemp, strTemp2, strTemp3, strTemp4;
	CStringArray strArr, strArr2, strArr3, strArr4;

	//CellInfo Header
	ReadString (pFile);
	//CellInfo Data
	SetModelInfoName (ReadString (pFile));
	m_recipeMgr.m_sMap.m_fCellSize.x	 = ReadDouble (pFile);
	m_recipeMgr.m_sMap.m_fCellSize.y	 = ReadDouble (pFile);
	m_recipeMgr.m_sMap.m_fCellDistance.x = ReadDouble (pFile);
	m_recipeMgr.m_sMap.m_fCellDistance.y = ReadDouble (pFile);
	m_recipeMgr.m_sMap.m_fCellCount.x    = ReadInt (pFile);
	if (nVersion > RECIPE_FILE_V1003) m_recipeMgr.m_sMap.m_fCellCount.y = ReadInt (pFile);
	m_recipeMgr.m_sMap.m_fContactOffset  = ReadDouble (pFile);

	//Option Header
	ReadString (pFile);
	//Option Data
	m_recipeMgr.m_sInfo.m_sOptionItem.m_bUseRtestNgPass = ReadInt (pFile); 
	m_recipeMgr.m_sInfo.m_sOptionItem.m_bUseSampling	= ReadInt (pFile); 
	m_recipeMgr.m_sInfo.m_sOptionItem.m_bUseAlign		= ReadInt (pFile); 
	m_recipeMgr.m_sInfo.m_sOptionItem.m_bUseReviewSnap  = ReadInt (pFile); 
	m_recipeMgr.m_sInfo.m_sOptionItem.m_nGlsRotate		= ReadInt (pFile); 
	m_recipeMgr.m_sInfo.m_sOptionItem.m_fFrontHeadGap   = ReadDouble (pFile);
	m_recipeMgr.m_sInfo.m_sOptionItem.m_fBackHeadGap    = ReadDouble (pFile);
	wcscpy_s (m_recipeMgr.m_sInfo.m_sOptionItem.m_chAlignName, ReadString (pFile));

	//InspectionItem Header
	ReadString (pFile);
	//InspectionInfo Data
	m_recipeMgr.m_sInfo.m_sInspectItem.m_bUsePre_0		 = ReadInt (pFile); 
	m_recipeMgr.m_sInfo.m_sInspectItem.m_bUseElect_1	 = ReadInt (pFile); 
	m_recipeMgr.m_sInfo.m_sInspectItem.m_bUseAgingTest_2 = ReadInt (pFile); 
	m_recipeMgr.m_sInfo.m_sInspectItem.m_bUseSpec_3		 = ReadInt (pFile); 
	m_recipeMgr.m_sInfo.m_sInspectItem.m_bUseQuality_4   = ReadInt (pFile); 
	m_recipeMgr.m_sInfo.m_sInspectItem.m_bUseColor_5	 = ReadInt (pFile); 
	m_recipeMgr.m_sInfo.m_sInspectItem.m_bUseAngle_6	 = ReadInt (pFile); 
	m_recipeMgr.m_sInfo.m_sInspectItem.m_bUseEFA		 = ReadInt (pFile);
	m_recipeMgr.m_sInfo.m_sInspectItem.m_bUseSpec_Multi  = ReadInt (pFile);

	// LYS 2026-08-06 강화검사
	if (nVersion >= RECIPE_FILE_V1006) {
		m_recipeMgr.m_sInfo.m_sInspectItem.m_bUseAging_Test = ReadInt(pFile);
	}
	else {
		m_recipeMgr.m_sInfo.m_sInspectItem.m_bUseAging_Test = 0; // 기본값
	}	

	strTemp = ReadString (pFile);
	dfcArryParsing (&strArr, strTemp);
	// LYS 2026-09-29 P0-3 V1005 이하 파일은 타임아웃이 9개 - GetAt 범위 초과는 CInvalidArgException, 모자라는 칸은 0
	for (int i = 0; i < M_RCP_INS_ITEM; i++) {
		m_recipeMgr.m_sInfo.m_sInspectItem.m_nTimeOut[i] = (i < strArr.GetSize ()) ? _wtoi (strArr.GetAt (i)) : 0;
	}

	//ProbeID Header
	ReadString (pFile);
	//ProbeID Data
	strTemp = ReadString (pFile);
	dfcArryParsing (&strArr, strTemp);
	strTemp2 = ReadString (pFile);
	dfcArryParsing (&strArr2, strTemp2);
	for (int i = 0; i < M_RCP_PROBE_MAX; i++) {
		wcscpy_s (m_recipeMgr.m_sInfo.m_sProbeInfo.m_chProbeFirst[i], strArr.GetAt (i));
		wcscpy_s (m_recipeMgr.m_sInfo.m_sProbeInfo.m_chProbeSecond[i], strArr2.GetAt (i));
	}
	m_recipeMgr.m_sInfo.m_sProbeInfo.m_nProbePriority = ReadInt (pFile);

	//ContactPos Header
	ReadString (pFile);
	//ContactPos Data
	m_recipeMgr.m_sInfo.m_sContactPos.m_fReadyPos	   = ReadDouble (pFile);
	m_recipeMgr.m_sInfo.m_sContactPos.m_fPreContactPos = ReadDouble (pFile);
	m_recipeMgr.m_sInfo.m_sContactPos.m_fEndContactPos = ReadDouble (pFile);
	m_recipeMgr.m_sInfo.m_sContactPos.m_fAddOD         = ReadDouble (pFile);

	//Lamp Header
	ReadString (pFile);
	//Lamp Data
	m_recipeMgr.m_sInfo.m_sLampItem.m_nFrontLeft  = ReadInt (pFile); 
	m_recipeMgr.m_sInfo.m_sLampItem.m_nFrontRight = ReadInt (pFile); 
	m_recipeMgr.m_sInfo.m_sLampItem.m_nBackLeft	  = ReadInt (pFile); 
	m_recipeMgr.m_sInfo.m_sLampItem.m_nBackRight  = ReadInt (pFile); 

	//HeadOffset Header
	ReadString (pFile);
	//HeadOffset Data
	m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fStageY		  = ReadDouble (pFile);
	m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fStageRotate	  = ReadDouble (pFile);
	m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fHeadReviewY	  = ReadDouble (pFile);
	m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fHeadReviewLeft  = ReadDouble (pFile);
	m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fHeadReviewRight = ReadDouble (pFile);
	m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_nQualityRotate	  = ReadInt (pFile);
	m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fQualityLeftRot  = ReadDouble (pFile);
	m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fQualityRightRot = ReadDouble (pFile);

	if (nVersion > RECIPE_FILE_V1004) m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fProbeShift = ReadDouble(pFile);	// jsyon 2022.05.13 add
	else m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fProbeShift = 0.;


	//데이터 초기화
	ClearRcpData ();

	//Stage Header
	ReadString (pFile);
	//Stage Data
	strTemp = _T(""), strTemp2 = _T(""), strTemp3 = _T("");
	m_recipeMgr.m_nStickCnt = ReadInt (pFile);
	strTemp = ReadString (pFile);
	dfcArryParsing (&strArr, strTemp);
	strTemp2 = ReadString (pFile);
	dfcArryParsing (&strArr2, strTemp2);
	strTemp3 = ReadString (pFile);
	dfcArryParsing (&strArr3, strTemp3);

	S_StickInfo sStick;
	for (int i = 0; i < m_recipeMgr.m_nStickCnt; i++) {
		sStick.m_nStickNum = _wtoi (strArr.GetAt (i));
		sStick.m_fStageY   = _wtof (strArr2.GetAt (i));
		sStick.m_fStageR   = _wtof (strArr3.GetAt (i));

		AddStick (sStick);
	}

	//HeadZ Header
	ReadString (pFile);
	//HeadZ Data
	m_recipeMgr.m_sData.m_sHeadZ.m_fReviewZ	 = ReadDouble (pFile);
	m_recipeMgr.m_sData.m_sHeadZ.m_fSpecZ	 = ReadDouble (pFile);
	m_recipeMgr.m_sData.m_sHeadZ.m_fQualityZ = ReadDouble (pFile);
	m_recipeMgr.m_sData.m_sHeadZ.m_fColorZ	 = ReadDouble (pFile);
	m_recipeMgr.m_sData.m_sHeadZ.m_fAngleZ	 = ReadDouble (pFile);
	m_recipeMgr.m_sData.m_sHeadZ.m_fZoom	 = ReadDouble (pFile);
	m_recipeMgr.m_sData.m_sHeadZ.m_fFocus	 = ReadDouble (pFile);

	//Head Header
	ReadString (pFile);
	//Head Data
	m_recipeMgr.m_nCellCnt = ReadInt (pFile);

	S_CellInfo sCell;
	for (int i = 0; i < m_recipeMgr.m_nStickCnt; i++) {
		strTemp = ReadString (pFile);
		dfcArryParsing (&strArr, strTemp);
		strTemp2 = ReadString (pFile);
		dfcArryParsing (&strArr2, strTemp2);
		strTemp3 = ReadString (pFile);
		dfcArryParsing (&strArr3, strTemp3);
		strTemp4 = ReadString (pFile);
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
	ReadString (pFile);
	//Cell Select Data
	for (int i = 0; i < m_recipeMgr.m_nStickCnt; i++) {
		strTemp = ReadString (pFile);
		dfcArryParsing (&strArr, strTemp);
		strTemp2 = ReadString (pFile);
		dfcArryParsing (&strArr2, strTemp2);
		strTemp3 = ReadString (pFile);
		dfcArryParsing (&strArr3, strTemp3);
		strTemp4 = ReadString (pFile);
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
		ReadString (pFile);
		//CAD Info Data
		strTemp = ReadString (pFile);
		dfcArryParsing (&strArr, strTemp);
		strTemp2 = ReadString (pFile);
		dfcArryParsing (&strArr2, strTemp2);

		for (int i = 0; i < CAD_ROW_P1_1; i++) {//CAD_ROW_A ~ CAD_ROW_X7
			m_recipeMgr.m_sMap.m_fCadData[i] = _wtof (strArr.GetAt (i));
		}
		for (int i = 0; i < CAD_ROW_END - CAD_ROW_P1_1; i++) {//CAD_ROW_P1_1 ~ CAD_ROW_P5_2
			wcscpy_s (m_recipeMgr.m_sMap.m_chCadData[i], strArr2.GetAt (i));
		}
	}

	if (nVersion > RECIPE_FILE_V1002) {
		m_recipeMgr.m_sMap.m_fGlobalMark.x = ReadDouble (pFile);
		m_recipeMgr.m_sMap.m_fGlobalMark.y = ReadDouble (pFile);
		m_recipeMgr.m_sMap.m_fCellCenter.x = ReadDouble (pFile);
		m_recipeMgr.m_sMap.m_fCellCenter.y = ReadDouble (pFile);

		wcscpy_s (m_recipeMgr.m_sMap.m_chCellCount,    ReadString (pFile));
		wcscpy_s (m_recipeMgr.m_sMap.m_chCellDistance, ReadString (pFile));
	}

	// LYS 2026-07-08 Sub Z Load (V1005+)
	if (nVersion > RECIPE_FILE_V1005) {
		ReadString(pFile);	//Head Sub Z Header
		m_recipeMgr.m_sInfo.m_sMotionSubZ.m_fWadSubzRight = ReadDouble(pFile);
		m_recipeMgr.m_sInfo.m_sMotionSubZ.m_fWadSubzLeft = ReadDouble(pFile);
		m_recipeMgr.m_sInfo.m_sMotionSubZ.m_fVisionSubzRight = ReadDouble(pFile);
		m_recipeMgr.m_sInfo.m_sMotionSubZ.m_fVisionSubzLeft = ReadDouble(pFile);
		//m_recipeMgr.m_sInfo.m_sMotionSubZ.m_fAngleSubzRight = ReadDouble(pFile);
		//m_recipeMgr.m_sInfo.m_sMotionSubZ.m_fAngleSubzLeft = ReadDouble(pFile);
	} else {
		m_recipeMgr.m_sInfo.m_sMotionSubZ.Clear();
	}

	// LYS 2026-08-19 Angle Axis Load (V1007+)
	if (nVersion > RECIPE_FILE_V1007) {
		ReadString(pFile);	//Angle Axis Header
		m_recipeMgr.m_sInfo.m_sMotionAngle.m_fAngleScheimpflug1 = ReadDouble(pFile);
		m_recipeMgr.m_sInfo.m_sMotionAngle.m_fAngleRotate1 = ReadDouble(pFile);
		m_recipeMgr.m_sInfo.m_sMotionAngle.m_fAngleZoom1 = ReadDouble(pFile);
		m_recipeMgr.m_sInfo.m_sMotionAngle.m_fAngleTilt1 = ReadDouble(pFile);
		m_recipeMgr.m_sInfo.m_sMotionAngle.m_fAngleAzimuth1 = ReadDouble(pFile);
		m_recipeMgr.m_sInfo.m_sMotionAngle.m_fAngleScheimpflug2 = ReadDouble(pFile);
		m_recipeMgr.m_sInfo.m_sMotionAngle.m_fAngleRotate2 = ReadDouble(pFile);
		m_recipeMgr.m_sInfo.m_sMotionAngle.m_fAngleZoom2 = ReadDouble(pFile);
		m_recipeMgr.m_sInfo.m_sMotionAngle.m_fAngleTilt2 = ReadDouble(pFile);
		m_recipeMgr.m_sInfo.m_sMotionAngle.m_fAngleAzimuth2 = ReadDouble(pFile);
	} else {
		m_recipeMgr.m_sInfo.m_sMotionAngle.Clear();
	}

	// LYS 2026-09-09 Stick별 PROBE Z CONTACT OFFSET 로드 (V1009+ 꼬리 블록). 구파일은 S_StickInfo 생성자 기본값 0.0 유지
	if (nVersion > RECIPE_FILE_V1009) {
		ReadString (pFile);	// _STICK_CONTACT_OFFSET_ Header
		CStringArray strArrZ;
		CString strZ = ReadString (pFile);
		dfcArryParsing (&strArrZ, strZ);
		for (int i = 0; i < (int)m_recipeMgr.m_sData.m_vecStick.size (); i++) {
			if (i < strArrZ.GetSize ()) m_recipeMgr.m_sData.m_vecStick[i].m_fProbeZOffset = _wtof (strArrZ.GetAt (i));
			else                        m_recipeMgr.m_sData.m_vecStick[i].m_fProbeZOffset = 0.0;
		}
	}

	// LYS 2026-09-18 ANGLE Sub Z 로드 (V1010+ 꼬리 블록).
	//                기존 _HEAD_SUBZ_ 블록(V1005)에 두 값을 되살려 끼우면 V1005~V1009 로 저장된
	//                레시피가 그 뒤 항목부터 통째로 밀려 깨진다. 그래서 꼬리에 새 블록으로 붙인다.
	if (nVersion > RECIPE_FILE_V1010) {
		ReadString (pFile);	// _ANGLE_SUBZ_ Header
		m_recipeMgr.m_sInfo.m_sMotionSubZ.m_fAngleSubzRight = ReadDouble (pFile);
		m_recipeMgr.m_sInfo.m_sMotionSubZ.m_fAngleSubzLeft  = ReadDouble (pFile);
	}
	else {
		// 구파일에는 값이 없다. VISION 값을 물려받지 않고 0 으로 둔다(예전엔 VISION 값으로 같이 움직였다).
		m_recipeMgr.m_sInfo.m_sMotionSubZ.m_fAngleSubzRight = 0.0;
		m_recipeMgr.m_sInfo.m_sMotionSubZ.m_fAngleSubzLeft  = 0.0;
	}


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

	// LYS 2026-09-04 SUB Z trailing block + EOF detect (versionless backward compat)
	{
		CString strSubZHdr;
		if (File.ReadString (strSubZHdr) && strSubZHdr.Find (_T("_SUB_Z_")) >= 0)   // new file
		{
			m_recipeMgr.m_sAlign.m_sGlsAlign.m_fSubZRight   = ReadDouble (&File);
			m_recipeMgr.m_sAlign.m_sGlsAlign.m_fSubZLeft    = ReadDouble (&File);
			m_recipeMgr.m_sAlign.m_sProbeAlign.m_fSubZRight = ReadDouble (&File);
			m_recipeMgr.m_sAlign.m_sProbeAlign.m_fSubZLeft  = ReadDouble (&File);
		}
		else   // old file : copy legacy BACK Z into both SUB Z
		{
			m_recipeMgr.m_sAlign.m_sGlsAlign.m_fSubZRight   = m_recipeMgr.m_sAlign.m_sGlsAlign.m_fFrontZ;
			m_recipeMgr.m_sAlign.m_sGlsAlign.m_fSubZLeft    = m_recipeMgr.m_sAlign.m_sGlsAlign.m_fFrontZ;
			m_recipeMgr.m_sAlign.m_sProbeAlign.m_fSubZRight = m_recipeMgr.m_sAlign.m_sProbeAlign.m_fBackZ;
			m_recipeMgr.m_sAlign.m_sProbeAlign.m_fSubZLeft  = m_recipeMgr.m_sAlign.m_sProbeAlign.m_fBackZ;
		}
	}

	File.Close ();

	return TRUE;
}

// LYS 2026-09-29 Hybrid Phase 1 세그먼트 모델 (docs/HybridGlass_Porting_Plan.md 5.2)
//   범위 밖 인덱스는 전부 기본값(0/빈 문자열/NULL)을 돌려준다 - ICP처럼 live 값으로 대체하지 않는다(I11 교정).
void	C_OutRecipeFunc::EnsureSegments ()
{
	if (!m_vecSegments.empty ()) return;

	m_vecSegments.push_back (m_recipeMgr);
	m_vecSegHeader.assign (1, S_SegmentHeader ());
	swprintf_s (m_vecSegHeader[0].m_chName, _T("Segment#%d"), 1);
	m_vecSegAlignOK.assign (1, FALSE);
	m_nActiveSegment = 0;
}

void	C_OutRecipeFunc::ResetSegments ()
{
	m_vecSegments.clear ();
	m_vecSegHeader.clear ();
	m_vecSegAlignOK.clear ();
	m_nActiveSegment = -1;
	EnsureSegments ();
}

int		C_OutRecipeFunc::GetSegmentCount ()
{
	return (int)m_vecSegments.size ();
}

int		C_OutRecipeFunc::GetSegmentLoopCount ()
{
	return m_vecSegments.empty () ? 1 : (int)m_vecSegments.size ();
}

int		C_OutRecipeFunc::GetActiveSegmentIndex ()
{
	return (m_nActiveSegment < 0) ? 0 : m_nActiveSegment;
}

// m_recipeMgr의 주소는 그대로 두고 내용만 교체한다 - GetRecipeInfo() 등 원시 포인터는 유지, m_vecStick 요소 포인터/반복자는 무효
BOOL	C_OutRecipeFunc::SetActiveSegment (int nSeg, BOOL bWriteBack)
{
	EnsureSegments ();
	if (nSeg < 0 || nSeg >= (int)m_vecSegments.size ()) return FALSE;
	if (nSeg == m_nActiveSegment) return TRUE;

	if (bWriteBack) m_vecSegments[m_nActiveSegment] = m_recipeMgr;
	m_recipeMgr		 = m_vecSegments[nSeg];
	m_nActiveSegment = nSeg;
	return TRUE;
}

void	C_OutRecipeFunc::CommitActiveSegment ()
{
	EnsureSegments ();
	m_vecSegments[m_nActiveSegment] = m_recipeMgr;
}

// 활성 세그먼트는 작업 슬롯(m_recipeMgr)을 돌려준다. 벡터 요소 포인터는 다음 로드까지만 유효
S_RecipeMgr*	C_OutRecipeFunc::GetSegmentMgr (int nSeg)
{
	if (nSeg < 0 || nSeg >= (int)m_vecSegments.size ()) return NULL;
	if (nSeg == m_nActiveSegment) return &m_recipeMgr;
	return &m_vecSegments[nSeg];
}

S_SegmentHeader	C_OutRecipeFunc::GetSegmentHeader (int nSeg)
{
	if (nSeg < 0 || nSeg >= (int)m_vecSegHeader.size ()) return S_SegmentHeader ();
	return m_vecSegHeader[nSeg];
}

CString	C_OutRecipeFunc::GetSegmentName (int nSeg)
{
	if (nSeg < 0 || nSeg >= (int)m_vecSegHeader.size ()) return CString ();
	return CString (m_vecSegHeader[nSeg].m_chName);
}

int		C_OutRecipeFunc::GetSegmentRotate (int nSeg)
{
	S_RecipeMgr* pSeg = GetSegmentMgr (nSeg);
	return pSeg ? pSeg->m_sInfo.m_sOptionItem.m_nGlsRotate : 0;
}

int		C_OutRecipeFunc::GetSegmentStickCount (int nSeg)
{
	S_RecipeMgr* pSeg = GetSegmentMgr (nSeg);
	return pSeg ? (int)pSeg->m_sData.m_vecStick.size () : 0;
}

// 스틱당 셀 수 (포맷상 세그먼트 안에서는 균일 - 저장이 GetCellCount(0)을 쓴다)
int		C_OutRecipeFunc::GetSegmentCellCount (int nSeg)
{
	S_RecipeMgr* pSeg = GetSegmentMgr (nSeg);
	if (!pSeg || pSeg->m_sData.m_vecStick.empty ()) return 0;
	return (int)pSeg->m_sData.m_vecStick[0].m_vecCell.size ();
}

// 앞 세그먼트들의 스틱 수 합 (nCellStatus 등 글라스 전체 스틱 인덱스의 base, 5.7)
int		C_OutRecipeFunc::GetSegmentStickBase (int nSeg)
{
	int nBase = 0;
	for (int k = 0; k < nSeg && k < (int)m_vecSegments.size (); k++) nBase += GetSegmentStickCount (k);
	return nBase;
}

CString	C_OutRecipeFunc::GetSegmentAlignName (int nSeg)
{
	S_RecipeMgr* pSeg = GetSegmentMgr (nSeg);
	return pSeg ? CString (pSeg->m_sInfo.m_sOptionItem.m_chAlignName) : CString ();
}

// 세그먼트마다 자기 .alg를 읽어 m_vecSegments[k].m_sAlign에 넣는다. 반환 = 실패한 세그먼트 수.
// 같은 .alg 이름은 앞 세그먼트 값을 복사. 활성 세그먼트 m_sAlign은 성공 시 그 세그먼트 값, 실패 시 호출 전 값 유지.
int		C_OutRecipeFunc::LoadSegmentAlignRcpFiles ()
{
	EnsureSegments ();
	CommitActiveSegment ();

	int nSeg = (int)m_vecSegments.size ();
	m_vecSegAlignOK.assign (nSeg, FALSE);
	S_Align sKeep = m_recipeMgr.m_sAlign;
	int nFail = 0;

	for (int k = 0; k < nSeg; k++) {
		CString strName = m_vecSegments[k].m_sInfo.m_sOptionItem.m_chAlignName;
		BOOL bOK = FALSE;

		for (int j = 0; j < k && !bOK; j++) {
			if (m_vecSegAlignOK[j] && strName.CompareNoCase (m_vecSegments[j].m_sInfo.m_sOptionItem.m_chAlignName) == 0) {
				m_vecSegments[k].m_sAlign = m_vecSegments[j].m_sAlign;
				bOK = TRUE;
			}
		}
		if (!bOK && !strName.IsEmpty () && LoadAlignRcpFile (strName)) {
			m_vecSegments[k].m_sAlign = m_recipeMgr.m_sAlign;
			bOK = TRUE;
		}

		m_vecSegAlignOK[k] = bOK;
		if (!bOK) nFail++;
	}

	m_recipeMgr.m_sAlign = m_vecSegAlignOK[m_nActiveSegment] ? m_vecSegments[m_nActiveSegment].m_sAlign : sKeep;
	return nFail;
}

BOOL	C_OutRecipeFunc::IsSegmentAlignLoaded (int nSeg)
{
	if (nSeg < 0 || nSeg >= (int)m_vecSegAlignOK.size ()) return FALSE;
	return m_vecSegAlignOK[nSeg];
}

// LYS 2026-09-29 Hybrid Phase 1 V1011 꼬리 블록 읽기 (5.3). 태그가 하나라도 어긋나면 FALSE - 호출자가 로드 전 상태로 되돌린다(I8 교정).
//   진입 시 m_recipeMgr = 세그먼트 0(방금 로드한 본문).
//   _HYBRID_ n / _SEGMENT_HEADER_ {name, pg, wad} x n / {_SEGMENT_ k <본문>} x (n-1) / _HYBRID_END_
BOOL	C_OutRecipeFunc::LoadHybridTail (CStdioFile *pFile, int nVersion)
{
	if (ReadString (pFile).Trim () != _T("_HYBRID_")) return FALSE;
	int nSeg = ReadInt (pFile);
	if (nSeg < 1 || nSeg > M_SEGMENT_MAX) return FALSE;

	if (ReadString (pFile).Trim () != _T("_SEGMENT_HEADER_")) return FALSE;
	std::vector<S_SegmentHeader> vecHdr (nSeg);
	for (int k = 0; k < nSeg; k++) {
		CString strName = ReadString (pFile).Trim ();
		CString strPG	= ReadString (pFile).Trim ();
		CString strWAD	= ReadString (pFile).Trim ();
		if (strName.IsEmpty ()) strName.Format (_T("Segment#%d"), k + 1);

		wcsncpy_s (vecHdr[k].m_chName,	  strName, _TRUNCATE);
		wcsncpy_s (vecHdr[k].m_chPGName,  strPG,   _TRUNCATE);
		wcsncpy_s (vecHdr[k].m_chWADName, strWAD,  _TRUNCATE);

		for (int j = 0; j < k; j++) {
			if (_wcsicmp (vecHdr[j].m_chName, vecHdr[k].m_chName) == 0) return FALSE;	// 세그먼트 이름 중복
		}
	}

	std::unique_ptr<S_RecipeMgr> pSeg0 (new S_RecipeMgr (m_recipeMgr));	// LYS 2026-09-29 스택 오버플로 방지 - S_RecipeMgr는 고정 문자 배열만 약 88KB라 스택에 두지 않는다
	std::vector<S_RecipeMgr> vecSeg;
	vecSeg.push_back ((*pSeg0));

	for (int k = 1; k < nSeg; k++) {
		if (ReadString (pFile).Trim () != _T("_SEGMENT_")) return FALSE;
		if (ReadInt (pFile) != k) return FALSE;

		// 기준값 = 세그먼트 0 사본: 파일에 없는 필드(SRM 램프 등)는 지금처럼 "직전 값 유지" 의미로 세그먼트 0 값을 물려받는다
		m_recipeMgr = (*pSeg0);
		if (!LoadRcpSections (pFile, nVersion)) return FALSE;
		vecSeg.push_back (m_recipeMgr);
	}
	m_recipeMgr = (*pSeg0);

	if (ReadString (pFile).Trim () != _T("_HYBRID_END_")) return FALSE;

	m_vecSegments  = vecSeg;
	m_vecSegHeader = vecHdr;
	m_vecSegAlignOK.assign (nSeg, FALSE);
	m_nActiveSegment = 0;
	return TRUE;
}
