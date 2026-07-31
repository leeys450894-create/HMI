#include "pch.h"
#include "RecipeData.h"

#include "..\Manager\SystemConfigMgr.h"
#include "common\BinaryHelper.h"
#include "common\IniHelper.h"				//sklee 20210601 add

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CRecipeData::CRecipeData()
{
	m_iCurrentAreaNum = 0;
}

CRecipeData::~CRecipeData()
{
	m_vecMeasAreaItem.clear();
	m_vecMeasAreaInfo.clear();
	m_vecMeasAreaInfo_clone.clear();
}

void CRecipeData::Clone(CRecipeData *rhs)
{
	m_stRecipeInfo		= rhs->m_stRecipeInfo;
	m_iCurrentAreaNum	= rhs->m_iCurrentAreaNum;
	m_vecMeasAreaItem	= rhs->m_vecMeasAreaItem;
	m_vecMeasAreaInfo	= rhs->m_vecMeasAreaInfo;
}

bool CRecipeData::LinkIniFile(CIniHelper *pIniHelper, bool bSave)
{
	//-----------------------------------------------------------------------
	// Align Ptrn
	//-----------------------------------------------------------------------
	pIniHelper->Link(bSave, _T("ALIGN"), _T("USE"), &m_stRecipeInfo.stRecipeAlignPtrn.bUseAlign, false);
	pIniHelper->Link(bSave, _T("ALIGN"), _T("USE_4POINT"), &m_stRecipeInfo.stRecipeAlignPtrn.bUse4PointAlign, false);

	CString strImgeIndex = _T("");

	for (int i = 0; i < 4; i++)
	{
		CString strText;
		strText.Format(_T("_%d"), i + 1);

		pIniHelper->Link(bSave, _T("ALIGN"), _T("MOVE_POS_X") + strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stCoordAlign.dx, 0.0);
		pIniHelper->Link(bSave, _T("ALIGN"), _T("MOVE_POS_Y") + strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stCoordAlign.dy, 0.0);
		pIniHelper->Link(bSave, _T("ALIGN"), _T("MOVE_POS_SX") + strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stCoordAlign.ds, 0.0);
		pIniHelper->Link(bSave, _T("ALIGN"), _T("MOVE_POS_Z") + strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stCoordAlign.dz, 0.0);
		pIniHelper->Link(bSave, _T("ALIGN"), _T("USE") + strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stCoordAlign.bUse, true);
		pIniHelper->Link(bSave, _T("ALIGN"), _T("USE_AF") + strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stCoordAlign.bAF, true);
		pIniHelper->Link(bSave, _T("ALIGN"), _T("CAD_POS_X") + strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stoffsetAlign.dx, 0.0);
		pIniHelper->Link(bSave, _T("ALIGN"), _T("CAD_POS_Y") + strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stoffsetAlign.dy, 0.0);
		pIniHelper->Link(bSave, _T("ALIGN"), _T("CAD_POS_SX") + strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stoffsetAlign.ds, 0.0);
		pIniHelper->Link(bSave, _T("ALIGN"), _T("CAD_POS_Z") + strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stoffsetAlign.dz, 0.0);
		pIniHelper->Link(bSave, _T("ALIGN"), _T("PTRN_NAME") + strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stPtrnAlign.strName, (CString)_T(""));
		
		pIniHelper->Link(bSave, _T("ALIGN"), _T("PTRN_LEFT") + strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stPtrnAlign.rectPtrn.left, (long)0);
		pIniHelper->Link(bSave, _T("ALIGN"), _T("PTRN_TOP") + strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stPtrnAlign.rectPtrn.top, (long)0);
		pIniHelper->Link(bSave, _T("ALIGN"), _T("PTRN_RIGHT") + strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stPtrnAlign.rectPtrn.right, (long)0);
		pIniHelper->Link(bSave, _T("ALIGN"), _T("PTRN_BOTTOM") + strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stPtrnAlign.rectPtrn.bottom, (long)0);
		
		pIniHelper->Link(bSave, _T("ALIGN"), _T("THRESHOLD") + strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stPtrnAlign.dThreshold, 500.0);
		pIniHelper->Link(bSave, _T("ALIGN"), _T("FIND_NUM") + strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stPtrnAlign.iFindNum, 5);
		pIniHelper->Link(bSave, _T("ALIGN"), _T("CENTERING_NUM") + strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stPtrnAlign.iCntrNum, 5);

		pIniHelper->Link(bSave, _T("ALIGN"), _T("MARGIN_X") + strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stPtrnAlign.iCenMarginX, 10);
		pIniHelper->Link(bSave, _T("ALIGN"), _T("MARGIN_Y") + strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stPtrnAlign.iCenMarginY, 10);
		pIniHelper->Link(bSave, _T("ALIGN"), _T("OFFSET_X") + strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stPtrnAlign.dCenOffsetX, 0.0);
		pIniHelper->Link(bSave, _T("ALIGN"), _T("OFFSET_Y") + strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stPtrnAlign.dCenOffsetY, 0.0);

		pIniHelper->Link(bSave, _T("ALIGN"), _T("GAIN") + strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stPtrnAlign.iCCDGain, 0);
		pIniHelper->Link(bSave, _T("ALIGN"), _T("EXPOSURE_TIME") + strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stPtrnAlign.iExposureTime, 500);
		pIniHelper->Link(bSave, _T("ALIGN"), _T("INTENSITY") + strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stPtrnAlign.iLampIntensity, 90);

		pIniHelper->Link(bSave, _T("ALIGN"), _T("LENS") + strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stPtrnAlign.iLens, 0);
	}

	//-----------------------------------------------------------------------
	// Meas Ptrn
	//-----------------------------------------------------------------------
	for (int i = 0; i < 8; ++i)
	{
		CString strText;
		strText.Format(_T("_%d"), i + 1);

		pIniHelper->Link(bSave, _T("MEAS"), _T("WB_RED") + strText, &m_stRecipeInfo.stRecipeMeasPtrn[i].dWhiteBalanceRed, (double)0.0);
		pIniHelper->Link(bSave, _T("MEAS"), _T("WB_GREEN") + strText, &m_stRecipeInfo.stRecipeMeasPtrn[i].dWhiteBalanceGreen, (double)0.0);
		pIniHelper->Link(bSave, _T("MEAS"), _T("WB_BLUE") + strText, &m_stRecipeInfo.stRecipeMeasPtrn[i].dWhiteBalanceBlue, (double)0.0);

		pIniHelper->Link(bSave, _T("MEAS"), _T("PTRN_NAME") + strText, &m_stRecipeInfo.stRecipeMeasPtrn[i].strName, (CString)_T(""));

		pIniHelper->Link(bSave, _T("MEAS"), _T("PTRN_LEFT") + strText, &m_stRecipeInfo.stRecipeMeasPtrn[i].rectPtrn.left, (long)0);
		pIniHelper->Link(bSave, _T("MEAS"), _T("PTRN_TOP") + strText, &m_stRecipeInfo.stRecipeMeasPtrn[i].rectPtrn.top, (long)0);
		pIniHelper->Link(bSave, _T("MEAS"), _T("PTRN_RIGHT") + strText, &m_stRecipeInfo.stRecipeMeasPtrn[i].rectPtrn.right, (long)0);
		pIniHelper->Link(bSave, _T("MEAS"), _T("PTRN_BOTTOM") + strText, &m_stRecipeInfo.stRecipeMeasPtrn[i].rectPtrn.bottom, (long)0);

		pIniHelper->Link(bSave, _T("MEAS"), _T("THRESHOLD") + strText, &m_stRecipeInfo.stRecipeMeasPtrn[i].dThreshold, 500.0);
		pIniHelper->Link(bSave, _T("MEAS"), _T("FIND_NUM") + strText, &m_stRecipeInfo.stRecipeMeasPtrn[i].iFindNum, 5);
		pIniHelper->Link(bSave, _T("MEAS"), _T("CENTERING_NUM") + strText, &m_stRecipeInfo.stRecipeMeasPtrn[i].iCntrNum, 5);

		pIniHelper->Link(bSave, _T("MEAS"), _T("MARGIN_X") + strText, &m_stRecipeInfo.stRecipeMeasPtrn[i].iCenMarginX, 10);
		pIniHelper->Link(bSave, _T("MEAS"), _T("MARGIN_Y") + strText, &m_stRecipeInfo.stRecipeMeasPtrn[i].iCenMarginY, 10);
		pIniHelper->Link(bSave, _T("MEAS"), _T("OFFSET_X") + strText, &m_stRecipeInfo.stRecipeMeasPtrn[i].dCenOffsetX, 0.0);
		pIniHelper->Link(bSave, _T("MEAS"), _T("OFFSET_Y") + strText, &m_stRecipeInfo.stRecipeMeasPtrn[i].dCenOffsetY, 0.0);

		pIniHelper->Link(bSave, _T("MEAS"), _T("GAIN") + strText, &m_stRecipeInfo.stRecipeMeasPtrn[i].iCCDGain, 0);
		pIniHelper->Link(bSave, _T("MEAS"), _T("EXPOSURE_TIME") + strText, &m_stRecipeInfo.stRecipeMeasPtrn[i].iExposureTime, 500);
		pIniHelper->Link(bSave, _T("MEAS"), _T("INTENSITY") + strText, &m_stRecipeInfo.stRecipeMeasPtrn[i].iLampIntensity, 90);

		pIniHelper->Link(bSave, _T("MEAS"), _T("INTENSITY") + strText, &m_stRecipeInfo.stRecipeMeasPtrn[i].iLens, 0);

		for (int j = 0; j < 10; ++j)
		{
			strImgeIndex.Format(_T("_%d"), j + 1);
			pIniHelper->Link(bSave, _T("MEAS"), _T("PTRN_NAME") + strText+ strImgeIndex, &m_stRecipeInfo.stRecipeMeasPtrn[i].strPtrnImgName[j], (CString)_T(""));
		}
	}

	//-----------------------------------------------------------------------
	// Option
	//-----------------------------------------------------------------------
	pIniHelper->Link(bSave, _T("MEAS"), _T("CELL_SIZE_W"), &m_stRecipeInfo.stRecipeOptionParam.dCellSizeW, (double)0.0);
	pIniHelper->Link(bSave, _T("MEAS"), _T("CELL_SIZE_H"), &m_stRecipeInfo.stRecipeOptionParam.dCellSizeH, (double)0.0);
	pIniHelper->Link(bSave, _T("MEAS"), _T("HEAD_GAP"), &m_stRecipeInfo.stRecipeOptionParam.dHeadGap, (double)0.0);

	pIniHelper->Link(bSave, _T("MEAS"), _T("THETA_LX"), &m_stRecipeInfo.stRecipeOptionParam.stPPAThetaCalc.Lx, (double)0.0);
	pIniHelper->Link(bSave, _T("MEAS"), _T("THETA_LY"), &m_stRecipeInfo.stRecipeOptionParam.stPPAThetaCalc.Ly, (double)0.0);
	pIniHelper->Link(bSave, _T("MEAS"), _T("THETA_L1"), &m_stRecipeInfo.stRecipeOptionParam.stPPAThetaCalc.L1, (double)0.0);
	pIniHelper->Link(bSave, _T("MEAS"), _T("THETA_L2"), &m_stRecipeInfo.stRecipeOptionParam.stPPAThetaCalc.L2, (double)0.0);
	pIniHelper->Link(bSave, _T("MEAS"), _T("THETA_L3"), &m_stRecipeInfo.stRecipeOptionParam.stPPAThetaCalc.L3, (double)0.0);
	pIniHelper->Link(bSave, _T("MEAS"), _T("THETA_L4"), &m_stRecipeInfo.stRecipeOptionParam.stPPAThetaCalc.L4, (double)0.0);
	pIniHelper->Link(bSave, _T("MEAS"), _T("THETA_L"), &m_stRecipeInfo.stRecipeOptionParam.stPPAThetaCalc.L, (double)0.0);

	pIniHelper->Link(bSave, _T("MEAS"), _T("AA_Z_OFFSET"), &m_stRecipeInfo.stRecipeOptionParam.dZOffsetAA, (double)0.0);

	pIniHelper->Link(bSave, _T("OPTION"), _T("MEAS_MODE"), &m_stRecipeInfo.stRecipeOptionParam.nMeasMode, 0);
	pIniHelper->Link(bSave, _T("OPTION"), _T("MEAS_HEAD"), &m_stRecipeInfo.stRecipeOptionParam.nUseMeasHead, 0);
	pIniHelper->Link(bSave, _T("OPTION"), _T("MEAS_SHOT_PPA"), &m_stRecipeInfo.stRecipeOptionParam.nMeasShotPPA, 0);
	pIniHelper->Link(bSave, _T("OPTION"), _T("MEAS_SHOT_AA"), &m_stRecipeInfo.stRecipeOptionParam.nMeasShotAA, 0);

	pIniHelper->Link(bSave, _T("OPTION"), _T("MEAS_BSH1"), &m_stRecipeInfo.stRecipeOptionParam.nBeamSplitterH1, 0);
	pIniHelper->Link(bSave, _T("OPTION"), _T("MEAS_BSH2"), &m_stRecipeInfo.stRecipeOptionParam.nBeamSplitterH2, 0);

	pIniHelper->Link(bSave, _T("OPTION"), _T("MEAS_GLSMODEL"), &m_stRecipeInfo.stRecipeOptionParam.nGlassModel, 0);

	pIniHelper->Link(bSave, _T("OPTION"), _T("MEAS_USE_ALIGN"), &m_stRecipeInfo.stRecipeOptionParam.bUseAlign, false);
	pIniHelper->Link(bSave, _T("OPTION"), _T("MEAS_USE_PATTERN"), &m_stRecipeInfo.stRecipeOptionParam.bUsePatternMatch, false);
	pIniHelper->Link(bSave, _T("OPTION"), _T("MEAS_USE_PPA"), &m_stRecipeInfo.stRecipeOptionParam.bUsePPA, false);
	pIniHelper->Link(bSave, _T("OPTION"), _T("MEAS_N2_MOTOR_AGING"), &m_stRecipeInfo.stRecipeOptionParam.bUseN2MotorAging, false);

	//-----------------------------------------------------------------------
	// Meas Area
	//-----------------------------------------------------------------------
	int nMaxMeasArea = (int)m_vecMeasAreaInfo.size();

	if (bSave)
	{
		pIniHelper->Link(bSave, _T("MEASURFEMENT"), _T("MAX_AREA"), &nMaxMeasArea, 0);
	}
	else
	{
		pIniHelper->Link(bSave, _T("MEASURFEMENT"), _T("MAX_AREA"), &nMaxMeasArea, 0);

		m_vecMeasAreaInfo.clear();
		m_vecMeasAreaInfo.resize(nMaxMeasArea);
	}

	for (int i = 0; i < nMaxMeasArea; ++i)
	{
		CString strText;
		strText.Format(_T("_%03d"), i + 1);

		pIniHelper->Link(bSave, _T("MEASURFEMENT"), _T("AREA") + strText, &m_vecMeasAreaInfo[i].iAreaNumber, i + 1);

		pIniHelper->Link(bSave, _T("MEASURFEMENT"), _T("XPOS") + strText, &m_vecMeasAreaInfo[i].stMeasCoordinate.dx, 0.0);
		pIniHelper->Link(bSave, _T("MEASURFEMENT"), _T("YPOS") + strText, &m_vecMeasAreaInfo[i].stMeasCoordinate.dy, 0.0);
		pIniHelper->Link(bSave, _T("MEASURFEMENT"), _T("SXPOS") + strText, &m_vecMeasAreaInfo[i].stMeasCoordinate.ds, 0.0);
		pIniHelper->Link(bSave, _T("MEASURFEMENT"), _T("ZPOS") + strText, &m_vecMeasAreaInfo[i].stMeasCoordinate.dz, 0.0);
		pIniHelper->Link(bSave, _T("MEASURFEMENT"), _T("USE") + strText, &m_vecMeasAreaInfo[i].stMeasCoordinate.bUse, false);
		pIniHelper->Link(bSave, _T("MEASURFEMENT"), _T("AF") + strText, &m_vecMeasAreaInfo[i].stMeasCoordinate.bAF, true);

		pIniHelper->Link(bSave, _T("MEASURFEMENT"), _T("SKIP") + strText, &m_vecMeasAreaInfo[i].stMeasCoordinate.bSkip, false);

		pIniHelper->Link(bSave, _T("MEASURFEMENT"), _T("CELL_ID") + strText, &m_vecMeasAreaInfo[i].strCellID, (CString)_T(""));
		pIniHelper->Link(bSave, _T("MEASURFEMENT"), _T("MEAS_OPTION") + strText, &m_vecMeasAreaInfo[i].iMeasOption, 1);
	}
	// Recipe End


	//-----------------------------------------------------------------------
	// Sequence Data
	//-----------------------------------------------------------------------
	int nSeqCount = (int)m_vecMeasAreaItem.size();

	if (bSave)
	{
		pIniHelper->Link(bSave, _T("MEAS_AREA_ITEM_LIST"), _T("COUNT"), &nSeqCount, 0);
	}
	else
	{
		pIniHelper->Link(bSave, _T("MEAS_AREA_ITEM_LIST"), _T("COUNT"), &nSeqCount, 0);

		m_vecMeasAreaItem.clear();
		m_vecMeasAreaItem.resize(nSeqCount);
	}

	for (int i = 0; i < nSeqCount; ++i)
	{
		CString strText;
		strText.Format(_T("ITEM_DATA_%02d_PARA_"), i);

		pIniHelper->Link(bSave, _T("MEAS_AREA_ITEM_LIST"), strText + _T("NAME"), &m_vecMeasAreaItem[i].strName, (CString)_T(""));
		pIniHelper->Link(bSave, _T("MEAS_AREA_ITEM_LIST"), strText + _T("WAIT_TIME"), &m_vecMeasAreaItem[i].iWaitTime_sec, 20);
	}

	return pIniHelper->IsReadEnable();
}

bool CRecipeData::LinkDataBase(CBinaryHelper *pBinaryHelper, bool bSave)
{
	//-----------------------------------------------------------------------
	// Align Ptrn
	//-----------------------------------------------------------------------
	pBinaryHelper->Link(bSave, _T("ALIGN"), _T("USE"),			&m_stRecipeInfo.stRecipeAlignPtrn.bUseAlign, false);
	pBinaryHelper->Link(bSave, _T("ALIGN"), _T("USE_4POINT"),	&m_stRecipeInfo.stRecipeAlignPtrn.bUse4PointAlign, false);

	for (int i = 0; i < 4; i++)
	{
		CString strText;
		strText.Format(_T("_%d"), i + 1);

		pBinaryHelper->Link(bSave, _T("ALIGN"), _T("MOVE_POS_X")	+ strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stCoordAlign.dx, 0.0);
		pBinaryHelper->Link(bSave, _T("ALIGN"), _T("MOVE_POS_Y")	+ strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stCoordAlign.dy, 0.0);
		pBinaryHelper->Link(bSave, _T("ALIGN"), _T("MOVE_POS_SX")	+ strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stCoordAlign.ds, 0.0);
		pBinaryHelper->Link(bSave, _T("ALIGN"), _T("MOVE_POS_Z")	+ strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stCoordAlign.dz, 0.0);
		pBinaryHelper->Link(bSave, _T("ALIGN"), _T("USE")			+ strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stCoordAlign.bUse, true);
		pBinaryHelper->Link(bSave, _T("ALIGN"), _T("USE_AF")		+ strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stCoordAlign.bAF, true);

		pBinaryHelper->Link(bSave, _T("ALIGN"), _T("CAD_POS_X")		+ strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stoffsetAlign.dx, 0.0);
		pBinaryHelper->Link(bSave, _T("ALIGN"), _T("CAD_POS_Y")		+ strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stoffsetAlign.dy, 0.0);
		pBinaryHelper->Link(bSave, _T("ALIGN"), _T("CAD_POS_SX")	+ strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stoffsetAlign.ds, 0.0);
		pBinaryHelper->Link(bSave, _T("ALIGN"), _T("CAD_POS_Z")		+ strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stoffsetAlign.dz, 0.0);

		pBinaryHelper->Link(bSave, _T("ALIGN"), _T("PTRN_NAME")		+ strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stPtrnAlign.strName,(CString) _T(""));

		pBinaryHelper->Link(bSave, _T("ALIGN"), _T("PTRN_LEFT")		+ strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stPtrnAlign.rectPtrn.left, (long)0);
		pBinaryHelper->Link(bSave, _T("ALIGN"), _T("PTRN_TOP")		+ strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stPtrnAlign.rectPtrn.top, (long)0);
		pBinaryHelper->Link(bSave, _T("ALIGN"), _T("PTRN_RIGHT")	+ strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stPtrnAlign.rectPtrn.right, (long)0);
		pBinaryHelper->Link(bSave, _T("ALIGN"), _T("PTRN_BOTTOM")	+ strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stPtrnAlign.rectPtrn.bottom, (long)0);

		pBinaryHelper->Link(bSave, _T("ALIGN"), _T("THRESHOLD")		+ strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stPtrnAlign.dThreshold, 500.0);
		pBinaryHelper->Link(bSave, _T("ALIGN"), _T("FIND_NUM")		+ strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stPtrnAlign.iFindNum, 5);
		pBinaryHelper->Link(bSave, _T("ALIGN"), _T("CENTERING_NUM") + strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stPtrnAlign.iCntrNum, 5);

		pBinaryHelper->Link(bSave, _T("ALIGN"), _T("MARGIN_X")		+ strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stPtrnAlign.iCenMarginX, 10);
		pBinaryHelper->Link(bSave, _T("ALIGN"), _T("MARGIN_Y")		+ strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stPtrnAlign.iCenMarginY, 10);
		pBinaryHelper->Link(bSave, _T("ALIGN"), _T("OFFSET_X")		+ strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stPtrnAlign.dCenOffsetX, 0.0);
		pBinaryHelper->Link(bSave, _T("ALIGN"), _T("OFFSET_Y")		+ strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stPtrnAlign.dCenOffsetX, 0.0);

		pBinaryHelper->Link(bSave, _T("ALIGN"), _T("GAIN")			+ strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stPtrnAlign.iCCDGain, 0);
		pBinaryHelper->Link(bSave, _T("ALIGN"), _T("EXPOSURE_TIME") + strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stPtrnAlign.iExposureTime, 500);
		pBinaryHelper->Link(bSave, _T("ALIGN"), _T("INTENSITY")		+ strText, &m_stRecipeInfo.stRecipeAlignPtrn.stAlignPtrn[i].stPtrnAlign.iLampIntensity, 90);
	}

	//-----------------------------------------------------------------------
	// Meas Ptrn
	//-----------------------------------------------------------------------
	for (int i = 0; i < 8; i++)
	{
		pBinaryHelper->Link(bSave, _T("MEAS"), _T("PTRN_NAME"), &m_stRecipeInfo.stRecipeMeasPtrn[i].strName, (CString)_T(""));

		pBinaryHelper->Link(bSave, _T("MEAS"), _T("PTRN_LEFT"), &m_stRecipeInfo.stRecipeMeasPtrn[i].rectPtrn.left, (long)0);
		pBinaryHelper->Link(bSave, _T("MEAS"), _T("PTRN_TOP"), &m_stRecipeInfo.stRecipeMeasPtrn[i].rectPtrn.top, (long)0);
		pBinaryHelper->Link(bSave, _T("MEAS"), _T("PTRN_RIGHT"), &m_stRecipeInfo.stRecipeMeasPtrn[i].rectPtrn.right, (long)0);
		pBinaryHelper->Link(bSave, _T("MEAS"), _T("PTRN_BOTTOM"), &m_stRecipeInfo.stRecipeMeasPtrn[i].rectPtrn.bottom, (long)0);

		pBinaryHelper->Link(bSave, _T("MEAS"), _T("THRESHOLD"), &m_stRecipeInfo.stRecipeMeasPtrn[i].dThreshold, 500.0);
		pBinaryHelper->Link(bSave, _T("MEAS"), _T("FIND_NUM"), &m_stRecipeInfo.stRecipeMeasPtrn[i].iFindNum, 5);
		pBinaryHelper->Link(bSave, _T("MEAS"), _T("CENTERING_NUM"), &m_stRecipeInfo.stRecipeMeasPtrn[i].iCntrNum, 5);

		pBinaryHelper->Link(bSave, _T("MEAS"), _T("MARGIN_X"), &m_stRecipeInfo.stRecipeMeasPtrn[i].iCenMarginX, 10);
		pBinaryHelper->Link(bSave, _T("MEAS"), _T("MARGIN_Y"), &m_stRecipeInfo.stRecipeMeasPtrn[i].iCenMarginY, 10);
		pBinaryHelper->Link(bSave, _T("MEAS"), _T("OFFSET_X"), &m_stRecipeInfo.stRecipeMeasPtrn[i].dCenOffsetX, 0.0);
		pBinaryHelper->Link(bSave, _T("MEAS"), _T("OFFSET_Y"), &m_stRecipeInfo.stRecipeMeasPtrn[i].dCenOffsetX, 0.0);

		pBinaryHelper->Link(bSave, _T("MEAS"), _T("GAIN"), &m_stRecipeInfo.stRecipeMeasPtrn[i].iCCDGain, 0);
		pBinaryHelper->Link(bSave, _T("MEAS"), _T("EXPOSURE_TIME"), &m_stRecipeInfo.stRecipeMeasPtrn[i].iExposureTime, 500);
		pBinaryHelper->Link(bSave, _T("MEAS"), _T("INTENSITY"), &m_stRecipeInfo.stRecipeMeasPtrn[i].iLampIntensity, 90);
	}
	


	//-----------------------------------------------------------------------
	// Option
	//-----------------------------------------------------------------------
	pBinaryHelper->Link(bSave, _T("OPTION"), _T("MEAS_MODE"),	&m_stRecipeInfo.stRecipeOptionParam.nMeasMode, 0);
	pBinaryHelper->Link(bSave, _T("OPTION"), _T("MEAS_HEAD"),	&m_stRecipeInfo.stRecipeOptionParam.nUseMeasHead, 0);
	pBinaryHelper->Link(bSave, _T("OPTION"), _T("MEAS_SHOT"),	&m_stRecipeInfo.stRecipeOptionParam.nMeasShotPPA, 0);
	pBinaryHelper->Link(bSave, _T("OPTION"), _T("MEAS_SHOT"),	&m_stRecipeInfo.stRecipeOptionParam.nMeasShotAA, 0);

	//-----------------------------------------------------------------------
	// Meas Area
	//-----------------------------------------------------------------------
	int nMaxMeasArea = (int)m_vecMeasAreaInfo.size();

	if (bSave)
	{
		pBinaryHelper->Link(bSave, _T("MEASURFEMENT"), _T("MAX_AREA"), &nMaxMeasArea, 0);
	}
	else
	{
		pBinaryHelper->Link(bSave, _T("MEASURFEMENT"), _T("MAX_AREA"), &nMaxMeasArea, 0);

		m_vecMeasAreaInfo.clear();
		m_vecMeasAreaInfo.resize(nMaxMeasArea);
	}

	for (int i = 0; i < nMaxMeasArea; ++i)
	{
		CString strText;
		strText.Format(_T("_%03d"), i + 1);

		pBinaryHelper->Link(bSave, _T("MEASURFEMENT"), _T("AREA")			+ strText, &m_vecMeasAreaInfo[i].iAreaNumber, i + 1);
																					   
		pBinaryHelper->Link(bSave, _T("MEASURFEMENT"), _T("XPOS")			+ strText, &m_vecMeasAreaInfo[i].stMeasCoordinate.dy, 0.0);
		pBinaryHelper->Link(bSave, _T("MEASURFEMENT"), _T("YPOS")			+ strText, &m_vecMeasAreaInfo[i].stMeasCoordinate.dy, 0.0);
		pBinaryHelper->Link(bSave, _T("MEASURFEMENT"), _T("SXPOS")			+ strText, &m_vecMeasAreaInfo[i].stMeasCoordinate.ds, 0.0);
		pBinaryHelper->Link(bSave, _T("MEASURFEMENT"), _T("ZPOS")			+ strText, &m_vecMeasAreaInfo[i].stMeasCoordinate.dz, 0.0);
		pBinaryHelper->Link(bSave, _T("MEASURFEMENT"), _T("USE")			+ strText, &m_vecMeasAreaInfo[i].stMeasCoordinate.bUse, false);
		pBinaryHelper->Link(bSave, _T("MEASURFEMENT"), _T("AF")				+ strText, &m_vecMeasAreaInfo[i].stMeasCoordinate.bAF, true);
																					   
		pBinaryHelper->Link(bSave, _T("MEASURFEMENT"), _T("CELL_ID")		+ strText, &m_vecMeasAreaInfo[i].strCellID, (CString)_T(""));
		pBinaryHelper->Link(bSave, _T("MEASURFEMENT"), _T("MEAS_OPTION")	+ strText, &m_vecMeasAreaInfo[i].iMeasOption, 1);
	}
	// Recipe End


	//-----------------------------------------------------------------------
	// Sequence Data
	//-----------------------------------------------------------------------
	int nSeqCount = (int)m_vecMeasAreaItem.size();

	if (bSave)
	{
		pBinaryHelper->Link(bSave, _T("MEAS_AREA_ITEM_LIST"), _T("COUNT"), &nSeqCount, 0);
	}
	else
	{
		pBinaryHelper->Link(bSave, _T("MEAS_AREA_ITEM_LIST"), _T("COUNT"), &nSeqCount, 0);

		m_vecMeasAreaItem.clear();
		m_vecMeasAreaItem.resize(nSeqCount);
	}

	for (int i = 0; i < nSeqCount; ++i)
	{
		CString strText;
		strText.Format(_T("ITEM_DATA_%02d_PARA_"), i);

		pBinaryHelper->Link(bSave, _T("MEAS_AREA_ITEM_LIST"), strText + _T("NAME"),		&m_vecMeasAreaItem[i].strName,		(CString)_T(""));
		pBinaryHelper->Link(bSave, _T("MEAS_AREA_ITEM_LIST"), strText + _T("WAIT_TIME"), &m_vecMeasAreaItem[i].iWaitTime_sec, 20);
	}

	return pBinaryHelper->IsReadEnable();
}

/*
void CRecipeData::ChangeAreaInfoRecipeToMeas()
{
	//0 : Only Head1, 1 : Only Head2, 2 : Head1 + Head2
	if (m_stRecipeInfo.stRecipeOptionParam.nUseMeasHead == 0 || m_stRecipeInfo.stRecipeOptionParam.nUseMeasHead == 1)
	{
		for (int i = 0; i < (int)m_vecMeasAreaInfo.size(); i++)
		{
			m_vecMeasAreaInfo[i].nMultiHeadMeasArea = i;
			m_vecMeasAreaInfo[i].iMeasHeadID_linked = m_stRecipeInfo.stRecipeOptionParam.nUseMeasHead;
		}

		return;
	}

	for (int i = 0; i < (int)m_vecMeasAreaInfo.size(); i++)
	{
		m_vecMeasAreaInfo[i].nMultiHeadMeasArea = -1;
	}

	int nMultiHeadIndex = 0;

	for (int i = 0; i < (int)m_vecMeasAreaInfo.size(); i++)
	{
		if (m_vecMeasAreaInfo[i].nMultiHeadMeasArea >= 0) continue;

		double dOldPos = 10000.0;
		int nIndex = 0;
		bool bFlag = false;

		m_vecMeasAreaInfo[i].nMultiHeadMeasArea = nMultiHeadIndex;

		double dRefX = m_vecMeasAreaInfo[i].stMeasCoordinate.dx;
		double dRefY = m_vecMeasAreaInfo[i].stMeasCoordinate.dy;

		for (int j = 0; j < (int)m_vecMeasAreaInfo.size(); j++)
		{
			if (m_vecMeasAreaInfo[j].nMultiHeadMeasArea >= 0) continue;
			if (i == j) continue;

			double dX = m_vecMeasAreaInfo[j].stMeasCoordinate.dx;
			double dY = m_vecMeasAreaInfo[j].stMeasCoordinate.dy;

			if (fabs(dRefX - dX) < df_Gantry_Compare)
			{
				double dPos = fabs(dRefY - dY);
				
				if (dPos > df_Head_Gap)
				{
					if (dOldPos > dPos)
					{
						dOldPos = dPos;
						nIndex = j;
						bFlag = true;
					}
				}
			}
		}

		if (bFlag)
			m_vecMeasAreaInfo[nIndex].nMultiHeadMeasArea = nMultiHeadIndex;

		nMultiHeadIndex++;
	}

	memcpy(&m_vecMeasAreaInfo_clone, &m_vecMeasAreaInfo, sizeof(stMeasAreaInfo) * m_vecMeasAreaInfo.size());

	//for (int i = 0; i < (int)m_vecMeasAreaInfo.size(); i++)
	//{
	//	CString strTest;
	//	strTest.Format(_T("%d\t%d"), i+1, m_vecMeasAreaInfo_clone[i].nMultiHeadMeasArea+1);
	//	AfxMessageBox(strTest);
	//}
}
*/


//----------------------------------------------------
// Teach
//----------------------------------------------------
int CRecipeData::GetParam_RecipeMeasAreaCount()
{
	return (int)m_vecMeasAreaInfo.size();
}

void CRecipeData::AddParam_RecipeMeasArea()
{
	stMeasAreaInfo temp;
	//m_vecMeasAreaInfo.push_back(m_vecMeasAreaInfo.back());
	m_vecMeasAreaInfo.push_back(temp);							//sklee 20210601 change
}

void CRecipeData::DelParam_RecipeMeasArea(int index)
{
	if ((0 <= index) && (index < GetParam_RecipeMeasAreaCount()))
	{
		m_vecMeasAreaInfo.erase(m_vecMeasAreaInfo.begin() + index);
	}
}

stMeasAreaInfo *CRecipeData::GetParam_RecipeMeasArea(int index)
{
	if ((0 <= index) && (index < GetParam_RecipeMeasAreaCount()))
	{
		return &(m_vecMeasAreaInfo[index]);
	}

	return nullptr;
}

void CRecipeData::SetParam_RecipeMeasArea(int index, stMeasAreaInfo &data)
{
	if ((0 <= index) && (index < GetParam_RecipeMeasAreaCount()))
	{
		m_vecMeasAreaInfo[index] = data;
	}
}

stRecipeInfo *CRecipeData::GetParam_RecipeInfo()
{
	return &m_stRecipeInfo;
}







int CRecipeData::GetParam_CurrentMeasArea()
{
	return m_iCurrentAreaNum;
}

void CRecipeData::SetParam_CurrentMeasArea(int value)
{
	m_iCurrentAreaNum = value;
}


S_RecipeMgr* CRecipeData::GetRecipe()
{
	return &m_stRecipeInfo.stRecipeHMI;
}

S_RecipeInfo* CRecipeData::GetRecipeInfo()
{
	return &m_stRecipeInfo.stRecipeHMI.m_sInfo;
}

S_RecipeData* CRecipeData::GetRecipeData()
{
	return &m_stRecipeInfo.stRecipeHMI.m_sData;
}

S_RecipeMap* CRecipeData::GetRecipeMap()
{
	return &m_stRecipeInfo.stRecipeHMI.m_sMap;
}

int	CRecipeData::GetParam_MeasAreaItemCount() const
{
	return (int)m_vecMeasAreaItem.size();
}

void CRecipeData::AddParam_MeasAreaItem()
{
	stMeasAreaItem temp;
	m_vecMeasAreaItem.push_back(temp);
}

void CRecipeData::DelParam_MeasAreaItem(int index)
{
	if ((0 <= index) && (index < GetParam_MeasAreaItemCount()))
	{
		m_vecMeasAreaItem.erase(m_vecMeasAreaItem.begin() + index);
	}
}

CString CRecipeData::GetParam_MeasAreaItem(int index)
{
	if ((0 <= index) && (index < GetParam_MeasAreaItemCount()))
	{
		return m_vecMeasAreaItem[index].strName;
	}

	return _T("");
}

void CRecipeData::SetParam_MeasAreaItem(int index, CString value)
{
	if ((0 <= index) && (index < GetParam_MeasAreaItemCount()))
	{
		m_vecMeasAreaItem[index].strName = value;
	}
}

long CRecipeData::GetParam_MeasAreaItemWaitTime(int index)
{
	if ((0 <= index) && (index < GetParam_MeasAreaItemCount()))
	{
		return m_vecMeasAreaItem[index].iWaitTime_sec;
	}

	return 20;
}

long CRecipeData::GetParam_MeasAreaItemWaitTime(CString strName)
{
	for (int i = 0; i < GetParam_MeasAreaItemCount(); ++i)
	{
		if (m_vecMeasAreaItem[i].strName == strName)
		{
			return m_vecMeasAreaItem[i].iWaitTime_sec;
		}
	}

	return 20;
}

void CRecipeData::SetParam_MeasAreaItemWaitTime(int index, long value)
{
	if ((0 <= index) && (index < GetParam_MeasAreaItemCount()))
	{
		m_vecMeasAreaItem[index].iWaitTime_sec = value;
	}
}

bool CRecipeData::LinkIniFileUTG(CIniHelper *pIniHelper, bool bSave)
{
	pIniHelper->Link(bSave, _T("RECIPE"), _T("VERSION"), &m_stRecipeInfo.stRecipeUTG.nVersion, 0);

	//-----------------------------------------------------------------------
	// Option
	//-----------------------------------------------------------------------
	pIniHelper->Link(bSave, _T("RECIPE"), _T("MEAS_MODE"), &m_stRecipeInfo.stRecipeUTG.nMeasMode, 1);
	pIniHelper->Link(bSave, _T("RECIPE"), _T("MEAS_HEAD"), &m_stRecipeInfo.stRecipeUTG.nUseMeasHead, 0);
	pIniHelper->Link(bSave, _T("RECIPE"), _T("CELL_SIZE_W"), &m_stRecipeInfo.stRecipeUTG.dCellSizeW, (double)0.0);
	pIniHelper->Link(bSave, _T("RECIPE"), _T("CELL_SIZE_H"), &m_stRecipeInfo.stRecipeUTG.dCellSizeH, (double)0.0);
	pIniHelper->Link(bSave, _T("RECIPE"), _T("STAGE_SIZE_W"), &m_stRecipeInfo.stRecipeUTG.dStageSizeW, (double)0.0);
	pIniHelper->Link(bSave, _T("RECIPE"), _T("STAGE_SIZE_H"), &m_stRecipeInfo.stRecipeUTG.dStageSizeH, (double)0.0);

	//-----------------------------------------------------------------------
	// Align Param
	//-----------------------------------------------------------------------
	// POS
	pIniHelper->Link(bSave, _T("ALIGN"), _T("MOVE_POS_X"), &m_stRecipeInfo.stRecipeUTG.stCoordAlign.dx, (double)0.0);
	pIniHelper->Link(bSave, _T("ALIGN"), _T("MOVE_POS_Y"), &m_stRecipeInfo.stRecipeUTG.stCoordAlign.dy, (double)0.0);
	pIniHelper->Link(bSave, _T("ALIGN"), _T("MOVE_POS_Z"), &m_stRecipeInfo.stRecipeUTG.stCoordAlign.dz, (double)0.0);

	// 
	pIniHelper->Link(bSave, _T("ALIGN"), _T("CENTERING_NUM"), &m_stRecipeInfo.stRecipeUTG.nAlignRetry, 5);
	pIniHelper->Link(bSave, _T("ALIGN"), _T("MARGIN_X"), &m_stRecipeInfo.stRecipeUTG.dAlignMarginX, (double)0.010);
	pIniHelper->Link(bSave, _T("ALIGN"), _T("MARGIN_Y"), &m_stRecipeInfo.stRecipeUTG.dAlignMarginY, (double) 0.010);
	pIniHelper->Link(bSave, _T("ALIGN"), _T("MARGIN_R"), &m_stRecipeInfo.stRecipeUTG.dAlignMarginR, (double)0.005);
	pIniHelper->Link(bSave, _T("ALIGN"), _T("CAM_DISTANCE"), &m_stRecipeInfo.stRecipeUTG.dAlignCamDistance, (double)50.);

	pIniHelper->Link(bSave, _T("ALIGN"), _T("ALIGN_ROTATE_OFFSET_X"), &m_stRecipeInfo.stRecipeUTG.dAlingRotateOffsetX, (double)0.);
	pIniHelper->Link(bSave, _T("ALIGN"), _T("ALIGN_ROTATE_OFFSET_Y"), &m_stRecipeInfo.stRecipeUTG.dAlingRotateOffsetY, (double)0.);

	CString strImgeIndex = _T("");
	for (int i = 0; i < 3; i++)
	{
		CString strText;
		strText.Format(_T("_%d"), i + 1);

		pIniHelper->Link(bSave, _T("ALIGN_SET") + strText, _T("THRESHOLD_G"), &m_stRecipeInfo.stRecipeUTG.stAlignParam[i].nGlassThreshold, 70);
		pIniHelper->Link(bSave, _T("ALIGN_SET") + strText, _T("THRESHOLD_CANY_U"), &m_stRecipeInfo.stRecipeUTG.stAlignParam[i].nCanyEdgeThreshold1, 250);
		pIniHelper->Link(bSave, _T("ALIGN_SET") + strText, _T("THRESHOLD_CANY_D"), &m_stRecipeInfo.stRecipeUTG.stAlignParam[i].nCanyEdgeThreshold2, 150);
		pIniHelper->Link(bSave, _T("ALIGN_SET") + strText, _T("CAM_RESOLUTION"), &m_stRecipeInfo.stRecipeUTG.stAlignParam[i].dCamResolution, 0.0001);

		
		pIniHelper->Link(bSave, _T("ALIGN_SET") + strText, _T("GAIN"), &m_stRecipeInfo.stRecipeUTG.stAlignParam[i].iCCDGain, 300);
		pIniHelper->Link(bSave, _T("ALIGN_SET") + strText, _T("EXPOSURE_TIME"), &m_stRecipeInfo.stRecipeUTG.stAlignParam[i].iExposureTime, 1000);
		pIniHelper->Link(bSave, _T("ALIGN_SET") + strText, _T("INTENSITY"), &m_stRecipeInfo.stRecipeUTG.stAlignParam[i].iLampIntensity, 50);
	}


	//-----------------------------------------------------------------------
	// Meas 
	//-----------------------------------------------------------------------
	// POS

	if (m_stRecipeInfo.stRecipeUTG.nVersion == 0)
	{
		for (int i = 0; i < 2; i++)
		{
			CString strText;
			strText.Format(_T("_%d"), i + 1);

			pIniHelper->Link(bSave, _T("MEAS"), _T("MOVE_POS_X") + strText, &m_stRecipeInfo.stRecipeUTG.stCoordMeasPos[i].dy, (double)0.0);
			pIniHelper->Link(bSave, _T("MEAS"), _T("MOVE_POS_Z") + strText, &m_stRecipeInfo.stRecipeUTG.stCoordMeasPos[i].dz, (double)0.0);
			pIniHelper->Link(bSave, _T("MEAS"), _T("AF") + strText, &m_stRecipeInfo.stRecipeUTG.stCoordMeasPos[i].bAF, true);

			m_stRecipeInfo.stRecipeUTG.stCoordTopMeasPos[i].dy = m_stRecipeInfo.stRecipeUTG.stCoordMeasPos[i].dy;
			m_stRecipeInfo.stRecipeUTG.stCoordTopMeasPos[i].dz = m_stRecipeInfo.stRecipeUTG.stCoordMeasPos[i].dz;
			m_stRecipeInfo.stRecipeUTG.stCoordTopMeasPos[i].bAF = m_stRecipeInfo.stRecipeUTG.stCoordMeasPos[i].bAF;

			m_stRecipeInfo.stRecipeUTG.stCoordBottomMeasPos[i].dy = m_stRecipeInfo.stRecipeUTG.stCoordMeasPos[i].dy;
			m_stRecipeInfo.stRecipeUTG.stCoordBottomMeasPos[i].dz = m_stRecipeInfo.stRecipeUTG.stCoordMeasPos[i].dz;
			m_stRecipeInfo.stRecipeUTG.stCoordBottomMeasPos[i].bAF = m_stRecipeInfo.stRecipeUTG.stCoordMeasPos[i].bAF;
		}
	}
	else
	{
		for (int i = 0; i < 2; i++)
		{
			CString strText;
			strText.Format(_T("_%d"), i + 1);

			pIniHelper->Link(bSave, _T("MEAS"), _T("TOP_MOVE_POS_X") + strText, &m_stRecipeInfo.stRecipeUTG.stCoordTopMeasPos[i].dy, (double)0.0);
			pIniHelper->Link(bSave, _T("MEAS"), _T("TOP_MOVE_POS_Z") + strText, &m_stRecipeInfo.stRecipeUTG.stCoordTopMeasPos[i].dz, (double)0.0);
			pIniHelper->Link(bSave, _T("MEAS"), _T("TOP_AF") + strText, &m_stRecipeInfo.stRecipeUTG.stCoordTopMeasPos[i].bAF, true);
		}

		for (int i = 0; i < 2; i++)
		{
			CString strText;
			strText.Format(_T("_%d"), i + 1);

			pIniHelper->Link(bSave, _T("MEAS"), _T("BOTTOM_MOVE_POS_X") + strText, &m_stRecipeInfo.stRecipeUTG.stCoordBottomMeasPos[i].dy, (double)0.0);
			pIniHelper->Link(bSave, _T("MEAS"), _T("BOTTOM_MOVE_POS_Z") + strText, &m_stRecipeInfo.stRecipeUTG.stCoordBottomMeasPos[i].dz, (double)0.0);
			pIniHelper->Link(bSave, _T("MEAS"), _T("BOTTOM_AF") + strText, &m_stRecipeInfo.stRecipeUTG.stCoordBottomMeasPos[i].bAF, true);
		}
	}
	
	pIniHelper->Link(bSave, _T("MEAS"), _T("TOP_AF"), &m_stRecipeInfo.stRecipeUTG.nAFChannel[0], 0);
	pIniHelper->Link(bSave, _T("MEAS"), _T("BOTTOM_AF"), &m_stRecipeInfo.stRecipeUTG.nAFChannel[1], 1);

	//-----------------------------------------------------------------------
	// TRIGGER 
	//-----------------------------------------------------------------------
	pIniHelper->Link(bSave, _T("TRIGGER"), _T("MEAS_POS"), &m_stRecipeInfo.stRecipeUTG.stTriggerParam.dStartPos, (double)5.);
	pIniHelper->Link(bSave, _T("TRIGGER"), _T("MEAS_RANGE"), &m_stRecipeInfo.stRecipeUTG.stTriggerParam.dMeseRange, (double)30.);
	pIniHelper->Link(bSave, _T("TRIGGER"), _T("PITCH"), &m_stRecipeInfo.stRecipeUTG.stTriggerParam.dTriggerPitch, (double)10.);
	pIniHelper->Link(bSave, _T("TRIGGER"), _T("ENCODE_RESULUTION"), &m_stRecipeInfo.stRecipeUTG.stTriggerParam.dEncoderResulution, (double) 0.10);
	pIniHelper->Link(bSave, _T("TRIGGER"), _T("PERIOD"), &m_stRecipeInfo.stRecipeUTG.stTriggerParam.iTriggerPeriod, 100);
	pIniHelper->Link(bSave, _T("TRIGGER"), _T("MEAS_READYPOS"), &m_stRecipeInfo.stRecipeUTG.stTriggerParam.dMeasureReadyPos, (double)20.);
	pIniHelper->Link(bSave, _T("TRIGGER"), _T("MEAS_SPEED_VEL"), &m_stRecipeInfo.stRecipeUTG.stTriggerParam.dSpeedVel, (double)30.);
	pIniHelper->Link(bSave, _T("TRIGGER"), _T("MEAS_SPEED_ACC"), &m_stRecipeInfo.stRecipeUTG.stTriggerParam.dSpeedAcc, (double)300.);
	pIniHelper->Link(bSave, _T("TRIGGER"), _T("MEAS_TRIGGER_OFFSET"), &m_stRecipeInfo.stRecipeUTG.stTriggerParam.dTriggerOffset, (double)0.);

	
	// Recipe End

	return pIniHelper->IsReadEnable();
}
