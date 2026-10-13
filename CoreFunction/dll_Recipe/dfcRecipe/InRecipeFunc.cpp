#include "pch.h"
#include "dfcRecipe.h"
#include "InRecipeFunc.h"
#include "CellIdRule.h"	// LYS 2026-10-01 Hybrid Phase 4b Cell ID rule core (shared with the other recipe class)

#include <memory>	// LYS 2026-09-29 unique_ptr
#ifdef _DEBUG 
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

C_InRecipeFunc::C_InRecipeFunc ()
{
	m_nActiveSegment = -1;	// LYS 2026-09-29 Hybrid Phase 1
	m_nCellIdCfgSeq = 0;	// LYS 2026-10-01 Hybrid Phase 4b
}

C_InRecipeFunc::~C_InRecipeFunc ()
{
	
}

int		C_InRecipeFunc::GetStickCount ()
{
	return (int)m_recipeMgr.m_sData.m_vecStick.size ();
}

S_StickInfo C_InRecipeFunc::GetStickItem (int nStick)
{
	if (GetStickCount () > nStick) return m_recipeMgr.m_sData.m_vecStick.at (nStick);
	else						   return S_StickInfo ();
}

void	C_InRecipeFunc::SetStickItem (int nStick, S_StickInfo sStick)
{
	if (GetStickCount () > nStick) {
		m_recipeMgr.m_sData.m_vecStick.at (nStick) = sStick;
	}
}

int		C_InRecipeFunc::GetCellCount (int nStick)
{
	if (GetStickCount () > nStick) return (int)m_recipeMgr.m_sData.m_vecStick.at (nStick).m_vecCell.size ();
	else					       return 0;
}

S_CellInfo C_InRecipeFunc::GetCellItem (int nStick, int nCell)
{
	if (GetStickCount () > nStick && GetCellCount (nStick) > nCell) return m_recipeMgr.m_sData.m_vecStick.at (nStick).m_vecCell.at (nCell);
	else															return S_CellInfo ();
}

void	C_InRecipeFunc::SetCellItem (int nStick, int nCell, S_CellInfo cellInfo)
{
	if (GetStickCount () > nStick && GetCellCount (nStick) > nCell) {
		m_recipeMgr.m_sData.m_vecStick.at (nStick).m_vecCell.at (nCell) = cellInfo;
	}
}

void	C_InRecipeFunc::AddStick (S_StickInfo stickInfo)
{
	m_recipeMgr.m_sData.m_vecStick.push_back (stickInfo);
}

void	C_InRecipeFunc::AddCell (int nStick, S_CellInfo cellInfo)
{
	if (GetStickCount () > nStick) m_recipeMgr.m_sData.m_vecStick.at (nStick).m_vecCell.push_back (cellInfo);
}

void	C_InRecipeFunc::SetCellID (int nStick, int nCell, CString strCellID)
{
	if (GetStickCount () > nStick) wcscpy_s (m_recipeMgr.m_sData.m_vecStick.at (nStick).m_vecCell.at (nCell).m_chCellID, strCellID);
}

CString	C_InRecipeFunc::GetCellID (int nStick, int nCell)
{
	if (GetStickCount () > nStick) return m_recipeMgr.m_sData.m_vecStick.at (nStick).m_vecCell.at (nCell).m_chCellID;
	else						   return CString ();
}

void	C_InRecipeFunc::SetCellArea (int nStick, int nCell, int nArea)
{
	if (GetStickCount () > nStick) m_recipeMgr.m_sData.m_vecStick.at (nStick).m_vecCell.at (nCell).m_nArea = nArea;
}

int		C_InRecipeFunc::GetCellArea (int nStick, int nCell)
{
	if (GetStickCount () > nStick) return m_recipeMgr.m_sData.m_vecStick.at (nStick).m_vecCell.at (nCell).m_nArea;
	else						   return -1;
}

void	C_InRecipeFunc::SetCellStatus (int nStick, int nCell, int nMode, int nStatus)
{
	if (GetStickCount () > nStick) m_recipeMgr.m_sData.m_vecStick.at (nStick).m_vecCell.at (nCell).m_nCurStatus[nMode] = nStatus;
}

int		C_InRecipeFunc::GetCellStatus (int nStick, int nCell, int nMode)
{
	if (GetStickCount () > nStick) return m_recipeMgr.m_sData.m_vecStick.at (nStick).m_vecCell.at (nCell).m_nCurStatus[nMode];
	else						   return -1;
}

void	C_InRecipeFunc::SetSelectCell (int nStick, int nCell, int nMode, int nSelect)
{
	if (GetStickCount () > nStick) m_recipeMgr.m_sData.m_vecStick.at (nStick).m_vecCell.at (nCell).m_nSelectCell[nMode] = nSelect;
}

int		C_InRecipeFunc::GetSelectCell (int nStick, int nCell, int nMode)
{
	if (GetStickCount () > nStick) return m_recipeMgr.m_sData.m_vecStick.at (nStick).m_vecCell.at (nCell).m_nSelectCell[nMode];
	else						   return -1;
}

BOOL	C_InRecipeFunc::LoadRcpFile (CString strRcpName)
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
	S_CellIdConfig sBakCid = m_sCellIdCfg;	// LYS 2026-10-01 Hybrid Phase 4b
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
		m_sCellIdCfg.Clear ();	// LYS 2026-10-01 Hybrid Phase 4b file <= V1012 -> Auto/Auto

		if (bOK && nVersion > RECIPE_FILE_V1011) bOK = LoadHybridTail (&File, nVersion);
		if (bOK && nVersion > RECIPE_FILE_V1012) bOK = LoadCellIdTail (&File);	// LYS 2026-10-01 Hybrid Phase 4b trailer after _HYBRID_END_

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
		m_sCellIdCfg = sBakCid;	// LYS 2026-10-01 Hybrid Phase 4b
		m_nActiveSegment = nBakActive;
		return FALSE;
	}
	m_nCellIdCfgSeq++;	// LYS 2026-10-01 Hybrid Phase 4b config generation (editor APPLY check)

	return TRUE;
}

// LYS 2026-09-29 Hybrid Phase 1 레시피 본문 섹션 읽기 - LoadRcpFile에서 코드 이동(논리 무변경). m_recipeMgr에 쓴다.
//   세그먼트 0과 꼬리 블록의 세그먼트 k>=1이 같은 코드를 쓴다.
BOOL	C_InRecipeFunc::LoadRcpSections (CStdioFile *pFile, int nVersion)
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
	if (nVersion > RECIPE_FILE_V1003) m_recipeMgr.m_sMap.m_fCellCount.y	 = ReadInt (pFile);
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
	// LYS 2026-07-15 Probe ID 읽기
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

		wcscpy_s (m_recipeMgr.m_sMap.m_chCellCount,	   ReadString (pFile));
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
	}
	else {
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
	}
	else {
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

BOOL	C_InRecipeFunc::LoadAlignRcpFile (CString strAlignRcpName)
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
void	C_InRecipeFunc::EnsureSegments ()
{
	if (!m_vecSegments.empty ()) return;

	m_vecSegments.push_back (m_recipeMgr);
	m_vecSegHeader.assign (1, S_SegmentHeader ());
	swprintf_s (m_vecSegHeader[0].m_chName, _T("Segment#%d"), 1);
	m_vecSegAlignOK.assign (1, FALSE);
	m_nActiveSegment = 0;
}

void	C_InRecipeFunc::ResetSegments ()
{
	m_vecSegments.clear ();
	m_vecSegHeader.clear ();
	m_vecSegAlignOK.clear ();
	m_nActiveSegment = -1;
	EnsureSegments ();
}

int		C_InRecipeFunc::GetSegmentCount ()
{
	return (int)m_vecSegments.size ();
}

int		C_InRecipeFunc::GetSegmentLoopCount ()
{
	return m_vecSegments.empty () ? 1 : (int)m_vecSegments.size ();
}

int		C_InRecipeFunc::GetActiveSegmentIndex ()
{
	return (m_nActiveSegment < 0) ? 0 : m_nActiveSegment;
}

// m_recipeMgr의 주소는 그대로 두고 내용만 교체한다 - GetRecipeInfo() 등 원시 포인터는 유지, m_vecStick 요소 포인터/반복자는 무효
BOOL	C_InRecipeFunc::SetActiveSegment (int nSeg, BOOL bWriteBack)
{
	EnsureSegments ();
	if (nSeg < 0 || nSeg >= (int)m_vecSegments.size ()) return FALSE;
	if (nSeg == m_nActiveSegment) return TRUE;

	if (bWriteBack) m_vecSegments[m_nActiveSegment] = m_recipeMgr;
	m_recipeMgr		 = m_vecSegments[nSeg];
	m_nActiveSegment = nSeg;
	return TRUE;
}

void	C_InRecipeFunc::CommitActiveSegment ()
{
	EnsureSegments ();
	m_vecSegments[m_nActiveSegment] = m_recipeMgr;
}

// 활성 세그먼트는 작업 슬롯(m_recipeMgr)을 돌려준다. 벡터 요소 포인터는 다음 로드까지만 유효
S_RecipeMgr*	C_InRecipeFunc::GetSegmentMgr (int nSeg)
{
	if (nSeg < 0 || nSeg >= (int)m_vecSegments.size ()) return NULL;
	if (nSeg == m_nActiveSegment) return &m_recipeMgr;
	return &m_vecSegments[nSeg];
}

S_SegmentHeader	C_InRecipeFunc::GetSegmentHeader (int nSeg)
{
	if (nSeg < 0 || nSeg >= (int)m_vecSegHeader.size ()) return S_SegmentHeader ();
	return m_vecSegHeader[nSeg];
}

CString	C_InRecipeFunc::GetSegmentName (int nSeg)
{
	if (nSeg < 0 || nSeg >= (int)m_vecSegHeader.size ()) return CString ();
	return CString (m_vecSegHeader[nSeg].m_chName);
}

int		C_InRecipeFunc::GetSegmentRotate (int nSeg)
{
	S_RecipeMgr* pSeg = GetSegmentMgr (nSeg);
	return pSeg ? pSeg->m_sInfo.m_sOptionItem.m_nGlsRotate : 0;
}

int		C_InRecipeFunc::GetSegmentStickCount (int nSeg)
{
	S_RecipeMgr* pSeg = GetSegmentMgr (nSeg);
	return pSeg ? (int)pSeg->m_sData.m_vecStick.size () : 0;
}

// 스틱당 셀 수 (포맷상 세그먼트 안에서는 균일 - 저장이 GetCellCount(0)을 쓴다)
int		C_InRecipeFunc::GetSegmentCellCount (int nSeg)
{
	S_RecipeMgr* pSeg = GetSegmentMgr (nSeg);
	if (!pSeg || pSeg->m_sData.m_vecStick.empty ()) return 0;
	return (int)pSeg->m_sData.m_vecStick[0].m_vecCell.size ();
}

// 앞 세그먼트들의 스틱 수 합 (nCellStatus 등 글라스 전체 스틱 인덱스의 base, 5.7)
int		C_InRecipeFunc::GetSegmentStickBase (int nSeg)
{
	int nBase = 0;
	for (int k = 0; k < nSeg && k < (int)m_vecSegments.size (); k++) nBase += GetSegmentStickCount (k);
	return nBase;
}

CString	C_InRecipeFunc::GetSegmentAlignName (int nSeg)
{
	S_RecipeMgr* pSeg = GetSegmentMgr (nSeg);
	return pSeg ? CString (pSeg->m_sInfo.m_sOptionItem.m_chAlignName) : CString ();
}

// 세그먼트마다 자기 .alg를 읽어 m_vecSegments[k].m_sAlign에 넣는다. 반환 = 실패한 세그먼트 수.
// 같은 .alg 이름은 앞 세그먼트 값을 복사. 활성 세그먼트 m_sAlign은 성공 시 그 세그먼트 값, 실패 시 호출 전 값 유지.
int		C_InRecipeFunc::LoadSegmentAlignRcpFiles ()
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

BOOL	C_InRecipeFunc::IsSegmentAlignLoaded (int nSeg)
{
	if (nSeg < 0 || nSeg >= (int)m_vecSegAlignOK.size ()) return FALSE;
	return m_vecSegAlignOK[nSeg];
}

// LYS 2026-09-29 Hybrid Phase 1 V1011 꼬리 블록 읽기 (5.3). 태그가 하나라도 어긋나면 FALSE - 호출자가 로드 전 상태로 되돌린다(I8 교정).
//   진입 시 m_recipeMgr = 세그먼트 0(방금 로드한 본문).
//   _HYBRID_ n / _SEGMENT_HEADER_ {name, pg, wad} x n / {_SEGMENT_ k <본문>} x (n-1) / _HYBRID_END_
BOOL	C_InRecipeFunc::LoadHybridTail (CStdioFile *pFile, int nVersion)
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

// LYS 2026-09-29 Hybrid Phase 1 레시피 본문 섹션 쓰기 - C_RecipeMgrDlg::SaveFile에서 코드 이동(논리 무변경). m_recipeMgr를 쓴다.
void	C_InRecipeFunc::SaveRcpSections (CStdioFile *pFile)
{
	CString str, strTemp, strTemp2, strTemp3, strTemp4;
	int nTemp = 0;

	//CellInfo Header
	WriteString (pFile, _T("_CELL_INFO_"));
	//CellInfo Data
	WriteString (pFile, GetModelInfoName ());
	WriteDouble (pFile, m_recipeMgr.m_sMap.m_fCellSize.x);
	WriteDouble (pFile, m_recipeMgr.m_sMap.m_fCellSize.y);
	WriteDouble (pFile, m_recipeMgr.m_sMap.m_fCellDistance.x);
	WriteDouble (pFile, m_recipeMgr.m_sMap.m_fCellDistance.y);
	WriteInt	(pFile, (int)m_recipeMgr.m_sMap.m_fCellCount.x);
	WriteInt	(pFile, (int)m_recipeMgr.m_sMap.m_fCellCount.y);
	WriteDouble (pFile, m_recipeMgr.m_sMap.m_fContactOffset);

	//Option Header
	WriteString (pFile, _T("_OPTION_INFO_"));
	//Option Data
	WriteInt	(pFile, (int)m_recipeMgr.m_sInfo.m_sOptionItem.m_bUseRtestNgPass);
	WriteInt	(pFile, (int)m_recipeMgr.m_sInfo.m_sOptionItem.m_bUseSampling);
	WriteInt	(pFile, (int)m_recipeMgr.m_sInfo.m_sOptionItem.m_bUseAlign);
	WriteInt	(pFile, (int)m_recipeMgr.m_sInfo.m_sOptionItem.m_bUseReviewSnap);
	WriteInt	(pFile, (int)m_recipeMgr.m_sInfo.m_sOptionItem.m_nGlsRotate);
	WriteDouble (pFile, m_recipeMgr.m_sInfo.m_sOptionItem.m_fFrontHeadGap);
	WriteDouble (pFile, m_recipeMgr.m_sInfo.m_sOptionItem.m_fBackHeadGap);
	WriteString (pFile, m_recipeMgr.m_sInfo.m_sOptionItem.m_chAlignName);

	//InspectionItem Header
	WriteString (pFile, _T("_INSPECTION_ITEM_"));
	//InspectionInfo Data
	WriteInt (pFile, (int)m_recipeMgr.m_sInfo.m_sInspectItem.m_bUsePre_0);
	WriteInt (pFile, (int)m_recipeMgr.m_sInfo.m_sInspectItem.m_bUseElect_1);
	WriteInt (pFile, (int)m_recipeMgr.m_sInfo.m_sInspectItem.m_bUseAgingTest_2);
	WriteInt (pFile, (int)m_recipeMgr.m_sInfo.m_sInspectItem.m_bUseSpec_3);
	WriteInt (pFile, (int)m_recipeMgr.m_sInfo.m_sInspectItem.m_bUseQuality_4);
	WriteInt (pFile, (int)m_recipeMgr.m_sInfo.m_sInspectItem.m_bUseColor_5);
	WriteInt (pFile, (int)m_recipeMgr.m_sInfo.m_sInspectItem.m_bUseAngle_6);
	WriteInt (pFile, (int)m_recipeMgr.m_sInfo.m_sInspectItem.m_bUseEFA);
	WriteInt (pFile, (int)m_recipeMgr.m_sInfo.m_sInspectItem.m_bUseSpec_Multi);
	WriteInt (pFile, (int)m_recipeMgr.m_sInfo.m_sInspectItem.m_bUseAging_Test);

	for (int i = 0; i < M_RCP_INS_ITEM; i++) {
		str.Format (_T("%d,"), m_recipeMgr.m_sInfo.m_sInspectItem.m_nTimeOut[i]);
		strTemp += str;
	}
	WriteString (pFile, strTemp);

	//ProbeID Header
	WriteString (pFile, _T("_PROBE_ID_"));
	//ProbeID Data
	strTemp = _T(""), strTemp2 = _T("");
	// LYS 2026-07-15 Probe ID 기록
	for (int i = 0; i < M_RCP_PROBE_MAX; i++) {
		str.Format (_T("%s,"), m_recipeMgr.m_sInfo.m_sProbeInfo.m_chProbeFirst[i]);
		strTemp += str;
		str.Format (_T("%s,"), m_recipeMgr.m_sInfo.m_sProbeInfo.m_chProbeSecond[i]);
		strTemp2 += str;
	}
	WriteString (pFile, strTemp);
	WriteString (pFile, strTemp2);
	WriteInt (pFile, m_recipeMgr.m_sInfo.m_sProbeInfo.m_nProbePriority);

	//ContactPos Header
	WriteString (pFile, _T("_CONTACT_POS_"));
	//ContactPos Data
	WriteDouble (pFile, m_recipeMgr.m_sInfo.m_sContactPos.m_fReadyPos);
	WriteDouble (pFile, m_recipeMgr.m_sInfo.m_sContactPos.m_fPreContactPos);
	WriteDouble (pFile, m_recipeMgr.m_sInfo.m_sContactPos.m_fEndContactPos);
	WriteDouble (pFile, m_recipeMgr.m_sInfo.m_sContactPos.m_fAddOD);

	//Lamp Header
	WriteString (pFile, _T("_LAMP_"));
	//Lamp Data
	WriteInt (pFile, m_recipeMgr.m_sInfo.m_sLampItem.m_nFrontLeft);
	WriteInt (pFile, m_recipeMgr.m_sInfo.m_sLampItem.m_nFrontRight);
	WriteInt (pFile, m_recipeMgr.m_sInfo.m_sLampItem.m_nBackLeft);
	WriteInt (pFile, m_recipeMgr.m_sInfo.m_sLampItem.m_nBackRight);

	//HeadOffset Header
	WriteString (pFile, _T("_HEAD_OFFSET_"));
	//HeadOffset Data
	WriteDouble (pFile, m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fStageY);
	WriteDouble (pFile, m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fStageRotate);
	WriteDouble (pFile, m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fHeadReviewY);
	WriteDouble (pFile, m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fHeadReviewLeft);
	WriteDouble (pFile, m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fHeadReviewRight);
	WriteInt	(pFile, m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_nQualityRotate);
	WriteDouble (pFile, m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fQualityLeftRot);
	WriteDouble (pFile, m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fQualityRightRot);
	WriteDouble(pFile, m_recipeMgr.m_sInfo.m_sMotionOffsetPos.m_fProbeShift);	// jsyoon 2022.05.13 add

	//Stage Header
	WriteString (pFile, _T("_STAGE_"));
	//Stage Data
	strTemp = _T(""), strTemp2 = _T(""), strTemp3 = _T("");
	nTemp = GetStickCount ();

	S_StickInfo sStick;
	for (int i = 0; i < nTemp; i++) {
		sStick = GetStickItem (i);
		str.Format (_T("%d,"), sStick.m_nStickNum);
		strTemp += str;

		str.Format (_T("%.4lf,"), sStick.m_fStageY);
		strTemp2 += str;

		str.Format (_T("%.4lf,"), sStick.m_fStageR);
		strTemp3 += str;
	}
	WriteInt (pFile, nTemp);
	WriteString (pFile, strTemp);
	WriteString (pFile, strTemp2);
	WriteString (pFile, strTemp3);

	//HeadZ Header
	WriteString (pFile, _T("_HEAD_Z_"));
	//HeadZ Data
	WriteDouble (pFile, m_recipeMgr.m_sData.m_sHeadZ.m_fReviewZ);
	WriteDouble (pFile, m_recipeMgr.m_sData.m_sHeadZ.m_fSpecZ);
	WriteDouble (pFile, m_recipeMgr.m_sData.m_sHeadZ.m_fQualityZ);
	WriteDouble (pFile, m_recipeMgr.m_sData.m_sHeadZ.m_fColorZ);
	WriteDouble (pFile, m_recipeMgr.m_sData.m_sHeadZ.m_fAngleZ);
	WriteDouble (pFile, m_recipeMgr.m_sData.m_sHeadZ.m_fZoom);
	WriteDouble (pFile, m_recipeMgr.m_sData.m_sHeadZ.m_fFocus);

	//Head Header
	WriteString (pFile, _T("_CELL_"));
	//Head Data
	nTemp = GetCellCount (0);
	WriteInt    (pFile, nTemp);

	S_CellInfo sCell;
	for (int i = 0; i < GetStickCount (); i++) {
		strTemp = _T(""), strTemp2 = _T(""), strTemp3 = _T(""), strTemp4 = _T("");
		for (int j = 0; j < nTemp; j++) {
			sCell = GetCellItem (i, j);

			str.Format (_T("%s,"), sCell.m_chCellID);
			strTemp += str;

			str.Format (_T("%.4lf,"), sCell.m_fCellY);
			strTemp2 += str;

			str.Format (_T("%.4lf,"), sCell.m_fCellX);
			strTemp3 += str;

			str.Format (_T("%d,"), sCell.m_nArea);
			strTemp4 += str;
		}
		WriteString (pFile, strTemp);
		WriteString (pFile, strTemp2);
		WriteString (pFile, strTemp3);
		WriteString (pFile, strTemp4);
	}

	//Cell Select Header
	WriteString (pFile, _T("_CELL_SELECT_"));
	//Cell Select Data
	for (int i = 0; i < GetStickCount (); i++) {
		strTemp = _T(""), strTemp2 = _T(""), strTemp3 = _T(""), strTemp4 = _T("");
		for (int j = 0; j < nTemp; j++) {
			sCell = GetCellItem (i, j);

			str.Format (_T("%d,"), sCell.m_nSelectCell[M_SPEC]);
			strTemp += str;

			str.Format (_T("%d,"), sCell.m_nSelectCell[M_QUALITY]);
			strTemp2 += str;

			str.Format (_T("%d,"), sCell.m_nSelectCell[M_COLOR]);
			strTemp3 += str;

			str.Format (_T("%d,"), sCell.m_nSelectCell[M_ANGLE]);
			strTemp4 += str;
		}
		WriteString (pFile, strTemp);
		WriteString (pFile, strTemp2);
		WriteString (pFile, strTemp3);
		WriteString (pFile, strTemp4);
	}

	//CAD Info Header
	WriteString (pFile, _T("_CAD_INFO_"));
	//CAD Info Data
	strTemp = _T(""), strTemp2 = _T("");
	for (int i = 0; i < CAD_ROW_P1_1; i++) {//CAD_ROW_A ~ CAD_ROW_X7
		str.Format (_T("%.4lf,"), m_recipeMgr.m_sMap.m_fCadData[i]);
		strTemp += str;
	}
	for (int i = 0; i < CAD_ROW_END - CAD_ROW_P1_1; i++) {//CAD_ROW_P1_1 ~ CAD_ROW_P5_2
		str.Format (_T("%s,"), m_recipeMgr.m_sMap.m_chCadData[i]);
		strTemp2 += str;
	}
	WriteString (pFile, strTemp);
	WriteString (pFile, strTemp2);

	//Cell Map 용 추가 저장
	WriteDouble (pFile, m_recipeMgr.m_sMap.m_fGlobalMark.x);
	WriteDouble (pFile, m_recipeMgr.m_sMap.m_fGlobalMark.y);
	WriteDouble (pFile, m_recipeMgr.m_sMap.m_fCellCenter.x);
	WriteDouble (pFile, m_recipeMgr.m_sMap.m_fCellCenter.y);

	strTemp.Format (_T("%s"), m_recipeMgr.m_sMap.m_chCellCount);
	WriteString (pFile, strTemp);
	strTemp.Format (_T("%s"), m_recipeMgr.m_sMap.m_chCellDistance);
	WriteString (pFile, strTemp);


	// LYS 2026-07-08 Sub Z Save (appended at end for V1005+ compatibility)
	WriteString(pFile, _T("_HEAD_SUBZ_"));
	WriteDouble(pFile, m_recipeMgr.m_sInfo.m_sMotionSubZ.m_fWadSubzRight);
	WriteDouble(pFile, m_recipeMgr.m_sInfo.m_sMotionSubZ.m_fWadSubzLeft);
	WriteDouble(pFile, m_recipeMgr.m_sInfo.m_sMotionSubZ.m_fVisionSubzRight);
	WriteDouble(pFile, m_recipeMgr.m_sInfo.m_sMotionSubZ.m_fVisionSubzLeft);
	// LYS 2026-09-18 ANGLE Sub Z 는 여기서 되살리지 않는다 - 이 블록 중간에 값을 끼우면
	//                V1005~V1009 로 저장된 레시피의 뒤쪽 항목이 전부 밀려 깨진다.
	//                아래 _ANGLE_SUBZ_ 꼬리 블록(V1010)에서 저장한다.
	//WriteDouble(pFile, m_recipeMgr.m_sInfo.m_sMotionSubZ.m_fAngleSubzRight);
	//WriteDouble(pFile, m_recipeMgr.m_sInfo.m_sMotionSubZ.m_fAngleSubzLeft);

	// LYS 2026-08-19 Angle Axis Save (appended at end for V1007+ compatibility)
	WriteString(pFile, _T("_ANGLE_AXIS_"));
	WriteDouble(pFile, m_recipeMgr.m_sInfo.m_sMotionAngle.m_fAngleScheimpflug1);
	WriteDouble(pFile, m_recipeMgr.m_sInfo.m_sMotionAngle.m_fAngleRotate1);
	WriteDouble(pFile, m_recipeMgr.m_sInfo.m_sMotionAngle.m_fAngleZoom1);
	WriteDouble(pFile, m_recipeMgr.m_sInfo.m_sMotionAngle.m_fAngleTilt1);
	WriteDouble(pFile, m_recipeMgr.m_sInfo.m_sMotionAngle.m_fAngleAzimuth1);
	WriteDouble(pFile, m_recipeMgr.m_sInfo.m_sMotionAngle.m_fAngleScheimpflug2);
	WriteDouble(pFile, m_recipeMgr.m_sInfo.m_sMotionAngle.m_fAngleRotate2);
	WriteDouble(pFile, m_recipeMgr.m_sInfo.m_sMotionAngle.m_fAngleZoom2);
	WriteDouble(pFile, m_recipeMgr.m_sInfo.m_sMotionAngle.m_fAngleTilt2);
	WriteDouble(pFile, m_recipeMgr.m_sInfo.m_sMotionAngle.m_fAngleAzimuth2);

	// LYS 2026-09-09 Stick별 PROBE Z CONTACT OFFSET 저장 (V1009+, 꼬리 블록 - _STAGE_ 블록 안에 넣으면 구파일 파싱이 깨짐)
	WriteString(pFile, _T("_STICK_CONTACT_OFFSET_"));
	strTemp = _T("");
	nTemp = GetStickCount ();
	for (int i = 0; i < nTemp; i++) {
		str.Format (_T("%.4lf,"), GetStickItem (i).m_fProbeZOffset);
		strTemp += str;
	}
	WriteString(pFile, strTemp);

	// LYS 2026-09-18 ANGLE Sub Z 저장 (V1010+, 꼬리 블록)
	WriteString(pFile, _T("_ANGLE_SUBZ_"));
	WriteDouble(pFile, m_recipeMgr.m_sInfo.m_sMotionSubZ.m_fAngleSubzRight);
	WriteDouble(pFile, m_recipeMgr.m_sInfo.m_sMotionSubZ.m_fAngleSubzLeft);

}

// LYS 2026-09-29 Hybrid Phase 1 세그먼트 0 본문 + V1011 꼬리 블록 쓰기 (5.3). 세그먼트가 1개여도 꼬리는 항상 쓴다.
//   _VERSION_ 은 호출자(SaveFile)가 쓴다. 작업 슬롯(활성 세그먼트)은 먼저 벡터에 되쓰고, 끝나면 그대로 복원한다.
void	C_InRecipeFunc::SaveRcpSegments (CStdioFile *pFile)
{
	CommitActiveSegment ();
	std::unique_ptr<S_RecipeMgr> pStage (new S_RecipeMgr (m_recipeMgr));	// LYS 2026-09-29 스택 오버플로 방지 - S_RecipeMgr는 고정 문자 배열만 약 88KB라 스택에 두지 않는다
	int nSeg = (int)m_vecSegments.size ();

	m_recipeMgr = m_vecSegments[0];
	SaveRcpSections (pFile);

	WriteString (pFile, _T("_HYBRID_"));
	WriteInt	(pFile, nSeg);
	WriteString (pFile, _T("_SEGMENT_HEADER_"));
	for (int k = 0; k < nSeg; k++) {
		S_SegmentHeader sHdr = GetSegmentHeader (k);
		CString strName = sHdr.m_chName;
		if (strName.IsEmpty ()) strName.Format (_T("Segment#%d"), k + 1);
		WriteString (pFile, strName);
		WriteString (pFile, CString (sHdr.m_chPGName));
		WriteString (pFile, CString (sHdr.m_chWADName));
	}
	for (int k = 1; k < nSeg; k++) {
		WriteString (pFile, _T("_SEGMENT_"));
		WriteInt	(pFile, k);
		m_recipeMgr = m_vecSegments[k];
		SaveRcpSections (pFile);
	}
	WriteString (pFile, _T("_HYBRID_END_"));

	m_recipeMgr = *pStage;
	SaveCellIdTail (pFile);	// LYS 2026-10-01 Hybrid Phase 4b right after _HYBRID_END_; needs the restored working slot (GetSegmentMgr(active))
}

// LYS 2026-09-29 Hybrid Phase 1 저장 전 검사. 빈 문자열 = 통과. 1세그먼트 레시피는 검사하지 않는다(기존 동작 불변).
//   D-7: 세그먼트 간 Probe 카드 동일 전제 - _PROBE_ID_ 가 세그먼트 0과 다르면 거부
// LYS 2026-09-29 Hybrid Phase 2 설비 타입 - ALPHA와 같은 SystemConfig.ini [EQUIP_TYPE] MACHINE_TYPE (DLL과 같은 Config 폴더)
static BOOL IsOCPMachine ()
{
	TCHAR szType[32] = {0};
	::GetPrivateProfileString (_T("EQUIP_TYPE"), _T("MACHINE_TYPE"), _T("HMI"), szType, _countof (szType), dfcIniFolder () + _T("SystemConfig.ini"));
	return (_tcsicmp (szType, _T("OCP")) == 0);
}

CString	C_InRecipeFunc::ValidateSegments ()
{
	CString strErr;
	int nSeg = GetSegmentCount ();
	if (nSeg <= 1) return strErr;

	if (nSeg > M_SEGMENT_MAX) {
		strErr.Format (_T("세그먼트 수 %d개가 최대 %d개를 넘습니다."), nSeg, M_SEGMENT_MAX);
		return strErr;
	}

	S_RecipeMgr* pSeg0 = GetSegmentMgr (0);
	int nTotalStick = 0;
	for (int k = 0; k < nSeg; k++) {
		S_RecipeMgr* pSeg = GetSegmentMgr (k);
		CString strName = GetSegmentName (k);
		if (!pSeg || !pSeg0) {
			strErr.Format (_T("세그먼트 %d 데이터가 없습니다."), k + 1);
			return strErr;
		}
		for (int j = 0; j < k; j++) {
			if (strName.CompareNoCase (GetSegmentName (j)) == 0) {
				strErr.Format (_T("세그먼트 이름 [%s]이(가) 중복됩니다 (세그먼트 %d, %d)."), strName.GetString (), j + 1, k + 1);
				return strErr;
			}
		}

		int nStick = (int)pSeg->m_sData.m_vecStick.size ();
		if (nStick <= 0) {
			strErr.Format (_T("세그먼트 %d [%s]에 스틱이 없습니다."), k + 1, strName.GetString ());
			return strErr;
		}
		nTotalStick += nStick;

		// LYS 2026-09-29 셀/스틱 좌표가 글라스 안에 있는지 (세그먼트 2 헤드 Y -1492.668 사고 재발 방지)
		{
			CString strWhat;
			strWhat.Format (_T("세그먼트 %d [%s]"), k + 1, strName.GetString ());
			strErr = CheckRecipeLayout (pSeg, strWhat);
			if (!strErr.IsEmpty ()) return strErr;
		}

		if (k == 0) continue;
		// LYS 2026-09-29 Hybrid Phase 2 런타임 ProbeCheck와 같은 슬롯만 비교 - OCP는 P5(인덱스 4)만 쓴다(MakeCell도 P5만 기록).
		//                P1~P4에는 예전 레시피 값이 남아 있어 전부 비교하면 같은 카드인데도 거부됐다
		for (int i = IsOCPMachine () ? M_RCP_PROBE_MAX - 1 : 0; i < M_RCP_PROBE_MAX; i++) {
			if (wcscmp (pSeg->m_sInfo.m_sProbeInfo.m_chProbeFirst[i],  pSeg0->m_sInfo.m_sProbeInfo.m_chProbeFirst[i])  != 0 ||
				wcscmp (pSeg->m_sInfo.m_sProbeInfo.m_chProbeSecond[i], pSeg0->m_sInfo.m_sProbeInfo.m_chProbeSecond[i]) != 0) {
				strErr.Format (_T("세그먼트 %d [%s]의 PROBE ID가 세그먼트 1과 다릅니다 (P%d: %s / %s).\n모든 세그먼트는 같은 프로브 카드를 사용해야 합니다."),
							   k + 1, strName.GetString (), i + 1,
							   pSeg->m_sInfo.m_sProbeInfo.m_chProbeFirst[i], pSeg0->m_sInfo.m_sProbeInfo.m_chProbeFirst[i]);
				return strErr;
			}
		}
		// // LYS 2026-09-29 Hybrid Phase 2 AGING TEST / EFA 사용 여부는 글라스 전역(D-14) - 세그먼트마다 다르면 거부
		if ((!pSeg->m_sInfo.m_sInspectItem.m_bUseAging_Test) != (!pSeg0->m_sInfo.m_sInspectItem.m_bUseAging_Test) ||
			(!pSeg->m_sInfo.m_sInspectItem.m_bUseEFA) != (!pSeg0->m_sInfo.m_sInspectItem.m_bUseEFA)) {
			strErr.Format (_T("세그먼트 %d [%s]의 AGING TEST / EFA 사용 설정이 세그먼트 1과 다릅니다.\n이 설정은 글라스 전체에 적용되므로 모든 세그먼트가 같아야 합니다."), k + 1, strName.GetString ());
			return strErr;
		}
		if (pSeg->m_sInfo.m_sProbeInfo.m_nProbePriority != pSeg0->m_sInfo.m_sProbeInfo.m_nProbePriority) {
			strErr.Format (_T("세그먼트 %d [%s]의 PROBE 우선순위가 세그먼트 1과 다릅니다."), k + 1, strName.GetString ());
			return strErr;
		}
	}

	if (nTotalStick > M_SEGMENT_STICK_MAX) {
		strErr.Format (_T("전체 스틱 수 %d개가 최대 %d개를 넘습니다."), nTotalStick, M_SEGMENT_STICK_MAX);
		return strErr;
	}
	// LYS 2026-09-29 Hybrid Phase 3b A7 result-row block G = sum of cells x sticks over all segments.
	//   AgingTest repeat / WAD Multi Point blocks are G rows each, so G itself must fit in the APD AREA_MAX_CNT.
	//   (K x P is checked at auto start in ALPHA - the aging / WAD multi sidecar files are not read here.)
	{
		int nGlassArea = 0;
		for (int k = 0; k < nSeg; k++) nGlassArea += GetSegmentCellCount (k) * GetSegmentStickCount (k);
		if (nGlassArea > M_RESULT_AREA_MAX) {
			strErr.Format (_T("전체 셀 수(세그먼트별 스틱 수 x 셀 수의 합) %d개가 결과 행 최대 %d개를 넘습니다."), nGlassArea, M_RESULT_AREA_MAX);
			return strErr;
		}
	}
	return strErr;
}

// LYS 2026-09-29 Hybrid Phase 2 편집기 세그먼트 편집
BOOL	C_InRecipeFunc::SetSegmentHeader (int nSeg, S_SegmentHeader sHdr)
{
	EnsureSegments ();
	if (nSeg < 0 || nSeg >= (int)m_vecSegHeader.size ()) return FALSE;
	m_vecSegHeader[nSeg] = sHdr;
	return TRUE;
}

// MakeCell RECIPE APPLY(하이브리드): 세그먼트 전체 교체, 활성 = 0
BOOL	C_InRecipeFunc::ReplaceSegments (const std::vector<S_RecipeMgr> &vecSeg, const std::vector<S_SegmentHeader> &vecHdr)
{
	if (vecSeg.empty () || vecSeg.size () != vecHdr.size () || (int)vecSeg.size () > M_SEGMENT_MAX) return FALSE;

	m_vecSegments  = vecSeg;
	m_vecSegHeader = vecHdr;
	m_vecSegAlignOK.assign (vecSeg.size (), FALSE);
	m_nActiveSegment = 0;
	m_recipeMgr		 = m_vecSegments[0];
	return TRUE;
}

// MakeCell RECIPE APPLY(하이브리드 해제): 활성 세그먼트(작업 슬롯) 하나만 남긴다
void	C_InRecipeFunc::CollapseToActiveSegment ()
{
	EnsureSegments ();
	S_SegmentHeader sHdr = GetSegmentHeader (m_nActiveSegment);

	m_vecSegments.assign (1, m_recipeMgr);
	m_vecSegHeader.assign (1, sHdr);
	m_vecSegAlignOK.assign (1, FALSE);
	m_nActiveSegment = 0;
}

// LYS 2026-09-29 셀/스틱 좌표 글라스 범위 검사 (MakeCell 셀 정보 오류 재발 방지)
//   사고: GlassInfo [OCP_TEST_SHORT_GLASS]의 셀/스틱 수가 다른 제품 값이라 180도 세그먼트에 셀 12개 x 156.75 (1724 mm)가 생겼고
//         헤드 Y 목표 -1492.668이 SW 리밋(MN -916)에 걸려 세그먼트 1 측정 13분 뒤에 멈췄다.
//   셀 방향(헤드 Y) 길이 = GLASS ROTATE 90/270이면 1500, 0/180이면 925. 스틱 방향은 나머지.
//   셀 X +- 셀 크기 X/2, 스틱 글라스 좌표(StageY + CONTACT GAP) +- 셀 크기 Y/2 가 글라스 반폭 + 1 mm 안이어야 한다.
//   (MakeCell 생성식: StageY = 글라스 좌표 - CONTACT GAP). 빈 문자열 = 통과
CString	C_InRecipeFunc::CheckRecipeLayout (const S_RecipeMgr *pMgr, CString strWhat)
{
	CString strErr;
	if (pMgr == NULL || pMgr->m_sData.m_vecStick.empty ()) return strErr;

	int	   nRot			  = pMgr->m_sInfo.m_sOptionItem.m_nGlsRotate;
	double dCellLen		  = (nRot % 2 == 1) ? RCP_GLASS_SIZE_X : RCP_GLASS_SIZE_Y;
	double dStickLen	  = (nRot % 2 == 1) ? RCP_GLASS_SIZE_Y : RCP_GLASS_SIZE_X;
	double dHalfCellSize  = fabs (pMgr->m_sMap.m_fCellSize.x) / 2.0;
	double dHalfStickSize = fabs (pMgr->m_sMap.m_fCellSize.y) / 2.0;
	const double dTol	  = 1.0;

	for (size_t s = 0; s < pMgr->m_sData.m_vecStick.size (); s++) {
		const S_StickInfo &sStick = pMgr->m_sData.m_vecStick[s];

		double dStickPos = sStick.m_fStageY + pMgr->m_sMap.m_fContactOffset;
		if (fabs (dStickPos) + dHalfStickSize > dStickLen / 2.0 + dTol) {
			strErr.Format (_T("%s 스틱 %d 위치가 글라스 범위를 벗어났습니다.\n\n스틱 글라스 좌표 %.3f (셀 크기 %.3f), 허용 +-%.1f (GLASS ROTATE %d도, %.0f mm 방향)\n\n셀 정보(GlassInfo)의 스틱 수(CELL STICK COUNT)·CELL to CELL Y·BLOCK DISTANCE를 확인하세요."),
						   strWhat.GetString (), sStick.m_nStickNum, dStickPos, pMgr->m_sMap.m_fCellSize.y, dStickLen / 2.0, nRot * 90, dStickLen);
			return strErr;
		}

		for (size_t c = 0; c < sStick.m_vecCell.size (); c++) {
			const S_CellInfo &sCell = sStick.m_vecCell[c];
			if (fabs (sCell.m_fCellX) + dHalfCellSize > dCellLen / 2.0 + dTol) {
				strErr.Format (_T("%s 스틱 %d 셀 %s(%d번째) 위치가 글라스 범위를 벗어났습니다.\n\n셀 X %.3f (셀 크기 %.3f), 허용 +-%.1f (GLASS ROTATE %d도, %.0f mm 방향)\n\n셀 정보(GlassInfo)의 셀 수(CELL COUNT)·CELL to CELL X·BLOCK DISTANCE를 확인하세요."),
							   strWhat.GetString (), sStick.m_nStickNum, sCell.m_chCellID, (int)c + 1, sCell.m_fCellX, pMgr->m_sMap.m_fCellSize.x, dCellLen / 2.0, nRot * 90, dCellLen);
				return strErr;
			}
		}
	}
	return strErr;
}

// LYS 2026-10-01 Hybrid Phase 4b Cell ID A/B (docs/HybridGlass_Porting_Plan.md 5.7, plan p4b (1-3)(3)(5)) - thin wrappers over CellIdRule.cpp
//   (the same rule copy as C_OutRecipeFunc). The geometry is rebuilt from the editor segments on every call; the active segment reads the
//   working slot (m_recipeMgr), so an unsaved GLASS ROTATE flushed from the option grid is what the Auto preview and the checks see.
BOOL	C_InRecipeFunc::IsCellIdDefault ()
{
	return m_sCellIdCfg.IsDefault ();
}

int		C_InRecipeFunc::GetCellIdMode (int nPanel)
{
	if (nPanel < 0 || nPanel >= M_CELLID_PANEL_MAX) return M_CELLID_MODE_AUTO;
	return m_sCellIdCfg.m_nMode[nPanel];
}

// 1 = manual token, or panel B derived from a manual A ("" when it cannot be derived); 0 = Auto (token only with bAutoFallback) / out of range
int		C_InRecipeFunc::ResolveCellToken (int nSeg, int nStick, int nCell, int nPanel, CString &strToken, BOOL bAutoFallback)
{
	CCellIdGeo geo;
	CellId_BuildGeo (*this, geo);
	return CellId_Resolve (m_sCellIdCfg, geo, nSeg, nStick, nCell, nPanel, strToken, bAutoFallback);
}

// SaveFile / editor pass -1 (token cap M_CELLID_TOKEN_MAX). TRUE = OK (Auto/Auto always - nothing is checked, no popup)
BOOL	C_InRecipeFunc::ValidateCellIdConfig (int nGlassIdLen, std::vector<S_CellIdError> &vecErr, int *pnLongest)
{
	CCellIdGeo geo;
	CellId_BuildGeo (*this, geo);
	return CellId_Validate (m_sCellIdCfg, geo, CellId_TokenMax (nGlassIdLen), vecErr, pnLongest);
}

// _CELL_ID_PANEL_ ... _CELL_ID_END_ right after _HYBRID_END_ (file version 1013 and later), on the segments the hybrid tail just set up.
//   FALSE -> LoadRcpFile rolls everything back (body, segments, Cell ID config)
BOOL	C_InRecipeFunc::LoadCellIdTail (CStdioFile *pFile)
{
	CCellIdGeo geo;
	CellId_BuildGeo (*this, geo);
	return CellId_LoadTail (pFile, geo, m_sCellIdCfg);
}

// MakeCell RECIPE APPLY (both paths): rebuilt cells are not proven to be the same physical cells (ICP 4.13, plan 5.7) -> Auto/Auto.
//   TRUE = a non-default config was cleared (the caller shows the info popup). The generation moves on every call, default config or not:
//   the cells were rebuilt, so an open editor's working copy is stale (review C1/K1 - also covers a default recipe being switched to Manual)
BOOL	C_InRecipeFunc::ResetCellIdConfig ()
{
	BOOL bWas = m_sCellIdCfg.IsDefault () ? FALSE : TRUE;
	m_sCellIdCfg.Clear ();
	m_nCellIdCfgSeq++;
	return bWas;
}

// editor working copy (OnInitDialog)
S_CellIdConfig	C_InRecipeFunc::GetCellIdConfig ()
{
	return m_sCellIdCfg;
}

// editor APPLY (validated there): Auto panels emptied, Manual panels shaped to the current geometry (values kept by index). The shaped copy
//   is built first, so the stored config is replaced only once it is complete
void	C_InRecipeFunc::SetCellIdConfig (const S_CellIdConfig &cfg)
{
	CCellIdGeo geo;
	CellId_BuildGeo (*this, geo);
	S_CellIdConfig sNew = cfg;
	CellId_EnsureShape (sNew, geo);
	m_sCellIdCfg = sNew;
	m_nCellIdCfgSeq++;
}

// LYS 2026-10-01 Hybrid Phase 4b DLL screens: 1-segment Auto/Auto (every 1-segment recipe saved before 4b) -> the recipe's m_chCellID exactly
//   as before (HARD RULE); N > 1 Auto/Auto -> the carried letters (LYS 2026-10-12 Hybrid Phase 6 D6-9 = D4b-7 (b); under (a) it showed m_chCellID);
//   a manual config -> the panel-A token the runtime sends (manual A, else the MakeCellID token with the
//   Phase 4 letter carry). nSeg = the active segment (GetCellID reads m_recipeMgr = the active segment)
CString	C_InRecipeFunc::GetDisplayCellId (int nSeg, int nStick, int nCell)
{
	if (GetSegmentLoopCount () <= 1 && m_sCellIdCfg.IsDefault ()) return GetCellID (nStick, nCell);	// LYS 2026-10-12 Hybrid Phase 6 D4b-7 (b): N > 1 Auto/Auto shows the carried letters (resolver Auto token) like the editor / runtime / operate map
	CString strTok;
	ResolveCellToken (nSeg, nStick, nCell, 0, strTok, TRUE);
	return strTok;
}

// Always written right after _HYBRID_END_ (N = 1 and Auto/Auto too: 5 lines). SaveRcpSegments calls it after the working slot is restored,
//   so the geometry = the segments just written and the trailer lines / tokens always match the body
void	C_InRecipeFunc::SaveCellIdTail (CStdioFile *pFile)
{
	CCellIdGeo geo;
	CellId_BuildGeo (*this, geo);
	CellId_SaveTail (pFile, geo, m_sCellIdCfg);
}

// editor: kept at OnInitDialog, APPLY is refused when it moved (successful load, MakeCell reset, another APPLY)
int		C_InRecipeFunc::GetCellIdCfgSeq ()
{
	return m_nCellIdCfgSeq;
}
