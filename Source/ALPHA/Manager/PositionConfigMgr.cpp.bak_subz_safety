//#include "stdafx.h"
#include "pch.h"
#include "PositionConfigMgr.h"

#include "common\IniHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPositionConfigMgr::CPositionConfigMgr(CString strConfigPath)
    : m_pIniHelper(new CIniHelper)
	, m_pIniHelper2(new CIniHelper)
{
    SetName(_T("POSITION_CONFIG_MGR"));

	m_pIniHelper->SetFilePath(strConfigPath + _T("\\Position.ini"));
	m_pIniHelper2->SetFilePath(strConfigPath + _T("\\ProbeChange.ini"));

	LinkIniFile(false);
	LinkIniFile2(false);
}

CPositionConfigMgr::~CPositionConfigMgr()
{
	Finalize();
}

bool CPositionConfigMgr::LinkIniFile(bool bSave)
{
	m_pIniHelper->Link(bSave, _T("SAFETY"), _T("SafetyStageX"),		&m_stPositionParam.dSafetyStageX, 0.0);
	m_pIniHelper->Link(bSave, _T("SAFETY"), _T("SafetyHeadX1"),		&m_stPositionParam.dSafetyHeadX1, 1000.0);
	m_pIniHelper->Link(bSave, _T("SAFETY"), _T("SafetyHeadX2"),		&m_stPositionParam.dSafetyHeadX2, -1000.0);
	m_pIniHelper->Link(bSave, _T("SAFETY"), _T("SafetyHeadY1"),		&m_stPositionParam.dSafetyHeadY1, 1000.0);
	m_pIniHelper->Link(bSave, _T("SAFETY"), _T("SafetyHeadY2"),		&m_stPositionParam.dSafetyHeadY2, -1000.0);
	m_pIniHelper->Link(bSave, _T("SAFETY"), _T("SafetyZ"),			&m_stPositionParam.dSafetyZ, 0.0);
	m_pIniHelper->Link(bSave, _T("SAFETY"), _T("SafetyZ2"),			&m_stPositionParam.dSafetyZ2, -600.0);


	m_pIniHelper->Link(bSave, _T("LOADER_Z"), _T("UP_POSITION"),	&m_stPositionParam.dLoaderZUp, 0.0);
	m_pIniHelper->Link(bSave, _T("LOADER_Z"), _T("MIDDLE_POSITION"),&m_stPositionParam.dLoaderZMiddle, 0.0);
	m_pIniHelper->Link(bSave, _T("LOADER_Z"), _T("PRE_POSITION"),   &m_stPositionParam.dLoaderZPre, 0.0);
	m_pIniHelper->Link(bSave, _T("LOADER_Z"), _T("DOWN_POSITION"),	&m_stPositionParam.dLoaderZDown, 0.0);

	m_pIniHelper->Link(bSave, _T("LOADER_LOAD"), _T("LOAD_X"),		&m_stPositionParam.dLoadPosition[enumAXIS_X], 0.0);
	m_pIniHelper->Link(bSave, _T("LOADER_LOAD"), _T("LOAD_Y"),		&m_stPositionParam.dLoadPosition[enumAXIS_Y], 0.0);
	m_pIniHelper->Link(bSave, _T("LOADER_LOAD"), _T("LOAD_R"),		&m_stPositionParam.dLoadPosition[enumAXIS_R], 0.0);

	m_pIniHelper->Link(bSave, _T("LOADER_UNLOAD"), _T("UNLOAD_X"),	&m_stPositionParam.dUnloadPosition[enumAXIS_X], 0.0);
	m_pIniHelper->Link(bSave, _T("LOADER_UNLOAD"), _T("UNLOAD_Y"),	&m_stPositionParam.dUnloadPosition[enumAXIS_Y], 0.0);
	m_pIniHelper->Link(bSave, _T("LOADER_UNLOAD"), _T("UNLOAD_R"),	&m_stPositionParam.dUnloadPosition[enumAXIS_R], 0.0);

	m_pIniHelper->Link(bSave, _T("LOADER_MEAS"), _T("MEAS_X"),		&m_stPositionParam.dMeasPosition[enumAXIS_X], 0.0);
	m_pIniHelper->Link(bSave, _T("LOADER_MEAS"), _T("MEAS_Y"),		&m_stPositionParam.dMeasPosition[enumAXIS_Y], 0.0);
	m_pIniHelper->Link(bSave, _T("LOADER_MEAS"), _T("MEAS_R"),		&m_stPositionParam.dMeasPosition[enumAXIS_R], 0.0);

	m_pIniHelper->Link(bSave, _T("ALIGN_PICKER"), _T("STAGE_X"),	&m_stPositionParam.dPickerStageX, 0.0);
	m_pIniHelper->Link(bSave, _T("ALIGN_PICKER"), _T("ALIGN2MEAS"), &m_stPositionParam.dAlignToMeasGap, 0.0);

	m_pIniHelper->Link(bSave, _T("HEAD_OFFSET"), _T("HEAD1_WAD_X"), &m_stPositionParam.dHeadOffset[0][0].dX, 0.0);
	m_pIniHelper->Link(bSave, _T("HEAD_OFFSET"), _T("HEAD1_WAD_Y"), &m_stPositionParam.dHeadOffset[0][0].dY, 0.0);
	m_pIniHelper->Link(bSave, _T("HEAD_OFFSET"), _T("HEAD1_WAD_Z"), &m_stPositionParam.dHeadOffset[0][0].dZ, 0.0);

	m_pIniHelper->Link(bSave, _T("HEAD_OFFSET"), _T("HEAD2_WAD_X"), &m_stPositionParam.dHeadOffset[1][0].dX, 0.0);
	m_pIniHelper->Link(bSave, _T("HEAD_OFFSET"), _T("HEAD2_WAD_Y"), &m_stPositionParam.dHeadOffset[1][0].dY, 0.0);
	m_pIniHelper->Link(bSave, _T("HEAD_OFFSET"), _T("HEAD2_WAD_Z"), &m_stPositionParam.dHeadOffset[1][0].dZ, 0.0);

	m_pIniHelper->Link(bSave, _T("HEAD_OFFSET"), _T("HEAD1_VISION_X"), &m_stPositionParam.dHeadOffset[0][2].dX, 0.0);
	m_pIniHelper->Link(bSave, _T("HEAD_OFFSET"), _T("HEAD1_VISION_Y"), &m_stPositionParam.dHeadOffset[0][2].dY, 0.0);
	m_pIniHelper->Link(bSave, _T("HEAD_OFFSET"), _T("HEAD1_VISION_Z"), &m_stPositionParam.dHeadOffset[0][2].dZ, 0.0);

	m_pIniHelper->Link(bSave, _T("HEAD_OFFSET"), _T("HEAD2_VISION_X"), &m_stPositionParam.dHeadOffset[1][2].dX, 0.0);
	m_pIniHelper->Link(bSave, _T("HEAD_OFFSET"), _T("HEAD2_VISION_Y"), &m_stPositionParam.dHeadOffset[1][2].dY, 0.0);
	m_pIniHelper->Link(bSave, _T("HEAD_OFFSET"), _T("HEAD2_VISION_Z"), &m_stPositionParam.dHeadOffset[1][2].dZ, 0.0);

	m_pIniHelper->Link(bSave, _T("HEAD_OFFSET"), _T("HEAD1_COLOR_X"), &m_stPositionParam.dHeadOffset[0][3].dX, 0.0);
	m_pIniHelper->Link(bSave, _T("HEAD_OFFSET"), _T("HEAD1_COLOR_Y"), &m_stPositionParam.dHeadOffset[0][3].dY, 0.0);
	m_pIniHelper->Link(bSave, _T("HEAD_OFFSET"), _T("HEAD1_COLOR_Z"), &m_stPositionParam.dHeadOffset[0][3].dZ, 0.0);

	m_pIniHelper->Link(bSave, _T("HEAD_OFFSET"), _T("HEAD2_COLOR_X"), &m_stPositionParam.dHeadOffset[1][3].dX, 0.0);
	m_pIniHelper->Link(bSave, _T("HEAD_OFFSET"), _T("HEAD2_COLOR_Y"), &m_stPositionParam.dHeadOffset[1][3].dY, 0.0);
	m_pIniHelper->Link(bSave, _T("HEAD_OFFSET"), _T("HEAD2_COLOR_Z"), &m_stPositionParam.dHeadOffset[1][3].dZ, 0.0);

	m_pIniHelper->Link(bSave, _T("HEAD_OFFSET"), _T("HEAD1_ANGLE_X"), &m_stPositionParam.dHeadOffset[0][1].dX, 0.0);
	m_pIniHelper->Link(bSave, _T("HEAD_OFFSET"), _T("HEAD1_ANGLE_Y"), &m_stPositionParam.dHeadOffset[0][1].dY, 0.0);
	m_pIniHelper->Link(bSave, _T("HEAD_OFFSET"), _T("HEAD1_ANGLE_Z"), &m_stPositionParam.dHeadOffset[0][1].dZ, 0.0);

	m_pIniHelper->Link(bSave, _T("HEAD_OFFSET"), _T("HEAD2_ANGLE_X"), &m_stPositionParam.dHeadOffset[1][1].dX, 0.0);
	m_pIniHelper->Link(bSave, _T("HEAD_OFFSET"), _T("HEAD2_ANGLE_Y"), &m_stPositionParam.dHeadOffset[1][1].dY, 0.0);
	m_pIniHelper->Link(bSave, _T("HEAD_OFFSET"), _T("HEAD2_ANGLE_Z"), &m_stPositionParam.dHeadOffset[1][1].dZ, 0.0);



	//m_pIniHelper->Link(bSave, _T("AUTOCHANGE_X"), _T("CHANGE"), &m_stPositionParam.dACPosition[0], 0.0);
	//m_pIniHelper->Link(bSave, _T("AUTOCHANGE_X"), _T("SAFETY"), &m_stPositionParam.dACPosition[1], 0.0);
	//m_pIniHelper->Link(bSave, _T("AUTOCHANGE_X"), _T("SACC"), &m_stPositionParam.dACPosition[2], 0.0);

	//m_pIniHelper->Link(bSave, _T("AUTOCHANGE_X"), _T("CHANGE"), &m_stPositionParam.dACZPosition[0], 0.0);
	//m_pIniHelper->Link(bSave, _T("AUTOCHANGE_X"), _T("SAFETY"), &m_stPositionParam.dACZPosition[1], 0.0);
	//m_pIniHelper->Link(bSave, _T("AUTOCHANGE_X"), _T("SACC"), &m_stPositionParam.dACZPosition[2], 0.0);

	//m_pIniHelper->Link(bSave, _T("PROBECHANGE_Z"), _T("CHANGE"), &m_stPositionParam.dProbeChangePos, 0.0);

	//m_pIniHelper->Link(bSave, _T("PROBE_SAFETY"), _T("PROBE1"), &m_stPositionParam.dProbeSafety[0], 0.0);
	//m_pIniHelper->Link(bSave, _T("PROBE_SAFETY"), _T("PROBE2"), &m_stPositionParam.dProbeSafety[1], 0.0);
	//m_pIniHelper->Link(bSave, _T("PROBE_SAFETY"), _T("PROBE3"), &m_stPositionParam.dProbeSafety[2], 0.0);
	//m_pIniHelper->Link(bSave, _T("PROBE_SAFETY"), _T("PROBE4"), &m_stPositionParam.dProbeSafety[3], 0.0);
	//m_pIniHelper->Link(bSave, _T("PROBE_SAFETY"), _T("PROBE5"), &m_stPositionParam.dProbeSafety[4], 0.0);

	//m_pIniHelper->Link(bSave, _T("PROBECHANGE_Z"), _T("CHANGE"), &m_stPositionParam.dProbeZChangePos[0], 0.0);
	//m_pIniHelper->Link(bSave, _T("PROBECHANGE_Z"), _T("SAFETY"), &m_stPositionParam.dProbeZChangePos[1], 0.0);
	//m_pIniHelper->Link(bSave, _T("PROBECHANGE_Z"), _T("ZERO"), &m_stPositionParam.dProbeZChangePos[2], 0.0);

	//m_pIniHelper->Link(bSave, _T("PROBEOFFSET"), _T("PROBE1_X"), &m_stPositionParam.dProbeXOffsetPos[0], 0.0);
	//m_pIniHelper->Link(bSave, _T("PROBEOFFSET"), _T("PROBE1_Y"), &m_stPositionParam.dProbeYOffsetPos[0], 0.0);

	//m_pIniHelper->Link(bSave, _T("PROBEOFFSET"), _T("PROBE2_X"), &m_stPositionParam.dProbeXOffsetPos[1], 0.0);
	//m_pIniHelper->Link(bSave, _T("PROBEOFFSET"), _T("PROBE2_Y"), &m_stPositionParam.dProbeYOffsetPos[1], 0.0);

	//m_pIniHelper->Link(bSave, _T("PROBEOFFSET"), _T("PROBE3_X"), &m_stPositionParam.dProbeXOffsetPos[2], 0.0);
	//m_pIniHelper->Link(bSave, _T("PROBEOFFSET"), _T("PROBE3_Y"), &m_stPositionParam.dProbeYOffsetPos[2], 0.0);

	//m_pIniHelper->Link(bSave, _T("PROBEOFFSET"), _T("PROBE4_X"), &m_stPositionParam.dProbeXOffsetPos[3], 0.0);
	//m_pIniHelper->Link(bSave, _T("PROBEOFFSET"), _T("PROBE4_Y"), &m_stPositionParam.dProbeYOffsetPos[3], 0.0);

	//m_pIniHelper->Link(bSave, _T("PROBEOFFSET"), _T("PROBE5_X"), &m_stPositionParam.dProbeXOffsetPos[4], 0.0);
	//m_pIniHelper->Link(bSave, _T("PROBEOFFSET"), _T("PROBE5_Y"), &m_stPositionParam.dProbeYOffsetPos[4], 0.0);

	return m_pIniHelper->IsReadEnable();
}

bool CPositionConfigMgr::LinkIniFile2(bool bSave)
{
	m_pIniHelper2->Link(bSave, _T("ACXPOS"), _T("POS2"), &m_stPositionParam.dACPosition[0], 0.0);
	m_pIniHelper2->Link(bSave, _T("ACXPOS"), _T("POS3"), &m_stPositionParam.dACPosition[1], 0.0);
	m_pIniHelper2->Link(bSave, _T("ACXPOS"), _T("POS4"), &m_stPositionParam.dACPosition[2], 0.0);

	m_pIniHelper2->Link(bSave, _T("ACZPOS"), _T("POS2"), &m_stPositionParam.dACZPosition[0], 0.0);
	m_pIniHelper2->Link(bSave, _T("ACZPOS"), _T("POS3"), &m_stPositionParam.dACZPosition[1], 0.0);
	m_pIniHelper2->Link(bSave, _T("ACZPOS"), _T("POS4"), &m_stPositionParam.dACZPosition[2], 0.0);

	m_pIniHelper2->Link(bSave, _T("MEASPOS"), _T("MeaszProbePos"), &m_stPositionParam.dProbeChangePos, 0.0);
	m_pIniHelper2->Link(bSave, _T("ALIGNPOS"), _T("HeadPos"), &m_stPositionParam.dAlignPos, 0.0);


	m_pIniHelper2->Link(bSave, _T("PROBECHANGE_Z"), _T("CHANGE"), &m_stPositionParam.dProbeZChangePos[0], 0.0);
	m_pIniHelper2->Link(bSave, _T("PROBECHANGE_Z"), _T("SAFETY"), &m_stPositionParam.dProbeZChangePos[1], 0.0);
	m_pIniHelper2->Link(bSave, _T("PROBECHANGE_Z"), _T("ZERO"), &m_stPositionParam.dProbeZChangePos[2], 0.0);

	m_pIniHelper2->Link(bSave, _T("PROBE_SAFETY"), _T("PROBE1"), &m_stPositionParam.dProbeSafety[0], 0.0);
	m_pIniHelper2->Link(bSave, _T("PROBE_SAFETY"), _T("PROBE2"), &m_stPositionParam.dProbeSafety[1], 0.0);
	m_pIniHelper2->Link(bSave, _T("PROBE_SAFETY"), _T("PROBE3"), &m_stPositionParam.dProbeSafety[2], 0.0);
	m_pIniHelper2->Link(bSave, _T("PROBE_SAFETY"), _T("PROBE4"), &m_stPositionParam.dProbeSafety[3], 0.0);
	m_pIniHelper2->Link(bSave, _T("PROBE_SAFETY"), _T("PROBE5"), &m_stPositionParam.dProbeSafety[4], 0.0);

	m_pIniHelper2->Link(bSave, _T("PROBEOFFSET"), _T("PROBE1_X"), &m_stPositionParam.dProbeXOffsetPos[0], 0.0);
	m_pIniHelper2->Link(bSave, _T("PROBEOFFSET"), _T("PROBE1_Y"), &m_stPositionParam.dProbeYOffsetPos[0], 0.0);
	m_pIniHelper2->Link(bSave, _T("PROBEOFFSET"), _T("PROBE1_T"), &m_stPositionParam.dProbeTOffsetPos[0], 0.0);

	m_pIniHelper2->Link(bSave, _T("PROBEOFFSET"), _T("PROBE2_X"), &m_stPositionParam.dProbeXOffsetPos[1], 0.0);
	m_pIniHelper2->Link(bSave, _T("PROBEOFFSET"), _T("PROBE2_Y"), &m_stPositionParam.dProbeYOffsetPos[1], 0.0);
	m_pIniHelper2->Link(bSave, _T("PROBEOFFSET"), _T("PROBE2_T"), &m_stPositionParam.dProbeTOffsetPos[1], 0.0);

	m_pIniHelper2->Link(bSave, _T("PROBEOFFSET"), _T("PROBE3_X"), &m_stPositionParam.dProbeXOffsetPos[2], 0.0);
	m_pIniHelper2->Link(bSave, _T("PROBEOFFSET"), _T("PROBE3_Y"), &m_stPositionParam.dProbeYOffsetPos[2], 0.0);
	m_pIniHelper2->Link(bSave, _T("PROBEOFFSET"), _T("PROBE3_T"), &m_stPositionParam.dProbeTOffsetPos[2], 0.0);

	m_pIniHelper2->Link(bSave, _T("PROBEOFFSET"), _T("PROBE4_X"), &m_stPositionParam.dProbeXOffsetPos[3], 0.0);
	m_pIniHelper2->Link(bSave, _T("PROBEOFFSET"), _T("PROBE4_Y"), &m_stPositionParam.dProbeYOffsetPos[3], 0.0);
	m_pIniHelper2->Link(bSave, _T("PROBEOFFSET"), _T("PROBE4_T"), &m_stPositionParam.dProbeTOffsetPos[3], 0.0);

	m_pIniHelper2->Link(bSave, _T("PROBEOFFSET"), _T("PROBE5_X"), &m_stPositionParam.dProbeXOffsetPos[4], 0.0);
	m_pIniHelper2->Link(bSave, _T("PROBEOFFSET"), _T("PROBE5_Y"), &m_stPositionParam.dProbeYOffsetPos[4], 0.0);
	m_pIniHelper2->Link(bSave, _T("PROBEOFFSET"), _T("PROBE5_T"), &m_stPositionParam.dProbeTOffsetPos[4], 0.0);

	return m_pIniHelper2->IsReadEnable();
}


void CPositionConfigMgr::Finalize()
{
	if (m_pIniHelper)
	{
		delete m_pIniHelper;
		m_pIniHelper = nullptr;
	}

	if (m_pIniHelper2)
	{
		delete m_pIniHelper2;
		m_pIniHelper2 = nullptr;
	}
}

stPositionParam CPositionConfigMgr::GetPositionParam() const
{
	return m_stPositionParam;
}

void CPositionConfigMgr::GetSafetyPosition(double *dSafetyStageX, double* dSafetyHeadX1, double* dSafetyHeadX2, double *dSafetyHeadY1, double *dSafetyHeadY2, double *dSafetyZ, double* dSafetyZ2) const
{
	if (dSafetyStageX != nullptr) *dSafetyStageX = m_stPositionParam.dSafetyStageX;
	if (dSafetyHeadX1 != nullptr) *dSafetyHeadX1 = m_stPositionParam.dSafetyHeadX1;
	if (dSafetyHeadX2 != nullptr) *dSafetyHeadX2 = m_stPositionParam.dSafetyHeadX2;
	if (dSafetyHeadY1 != nullptr) *dSafetyHeadY1 = m_stPositionParam.dSafetyHeadY1;
	if (dSafetyHeadY2 != nullptr) *dSafetyHeadY2 = m_stPositionParam.dSafetyHeadY2;
	if (dSafetyZ != nullptr)	  *dSafetyZ		 = m_stPositionParam.dSafetyZ;
	if (dSafetyZ2 != nullptr)	  *dSafetyZ2	 = m_stPositionParam.dSafetyZ2;
}

void CPositionConfigMgr::SetSafetyPosition(double dSafetyStageX, double dSafetyHeadX1, double dSafetyHeadX2, double dSafetyHeadY1, double dSafetyHeadY2, double dSafetyZ, double dSafetyZ2)
{
	m_stPositionParam.dSafetyStageX = dSafetyStageX;
	m_stPositionParam.dSafetyHeadX1 = dSafetyHeadX1;
	m_stPositionParam.dSafetyHeadX2 = dSafetyHeadX2;
	m_stPositionParam.dSafetyHeadY1 = dSafetyHeadY1;
	m_stPositionParam.dSafetyHeadY2 = dSafetyHeadY2;
	m_stPositionParam.dSafetyZ		= dSafetyZ;
	m_stPositionParam.dSafetyZ2     = dSafetyZ2;
}

void CPositionConfigMgr::GetLoaderZPossition(double *dLoaderZUp, double *dLoaderZMiddle, double* dLoaderZPre, double *dLoaderZDown) const
{
	*dLoaderZUp		= m_stPositionParam.dLoaderZUp;
	*dLoaderZMiddle = m_stPositionParam.dLoaderZMiddle;
	*dLoaderZPre	= m_stPositionParam.dLoaderZPre;
	*dLoaderZDown	= m_stPositionParam.dLoaderZDown;
}

void CPositionConfigMgr::SetLoaderZPossition(double dLoaderZUp, double dLoaderZMiddle, double dLoaderZPre, double dLoaderZDown)
{
	m_stPositionParam.dLoaderZUp		= dLoaderZUp;
	m_stPositionParam.dLoaderZMiddle	= dLoaderZMiddle;
	m_stPositionParam.dLoaderZPre		= dLoaderZPre;
	m_stPositionParam.dLoaderZDown = dLoaderZDown;
}

void CPositionConfigMgr::GetLoaderLoadPosition(double *dLoadPositionX, double *dLoadPositionY, double *dLoadPositionR) const
{
	*dLoadPositionX = m_stPositionParam.dLoadPosition[enumAXIS_X];
	*dLoadPositionY = m_stPositionParam.dLoadPosition[enumAXIS_Y];
	*dLoadPositionR = m_stPositionParam.dLoadPosition[enumAXIS_R];
}

void CPositionConfigMgr::SetLoaderLoadPosition(double dLoadPositionX, double dLoadPositionY, double dLoadPositionR)
{
	m_stPositionParam.dLoadPosition[enumAXIS_X] = dLoadPositionX;
	m_stPositionParam.dLoadPosition[enumAXIS_Y] = dLoadPositionY;
	m_stPositionParam.dLoadPosition[enumAXIS_R] = dLoadPositionR;
}

void CPositionConfigMgr::GetLoaderUnloadPosition(double *dUnLoadPositionX, double *dUnLoadPositionY, double *dUnLoadPositionR) const
{
	*dUnLoadPositionX = m_stPositionParam.dUnloadPosition[enumAXIS_X];
	*dUnLoadPositionY = m_stPositionParam.dUnloadPosition[enumAXIS_Y];
	*dUnLoadPositionR = m_stPositionParam.dUnloadPosition[enumAXIS_R];
}

void CPositionConfigMgr::SetLoaderUnloadPosition(double dUnLoadPositionX, double dUnLoadPositionY, double dUnLoadPositionR)
{
	m_stPositionParam.dUnloadPosition[enumAXIS_X] = dUnLoadPositionX;
	m_stPositionParam.dUnloadPosition[enumAXIS_Y] = dUnLoadPositionY;
	m_stPositionParam.dUnloadPosition[enumAXIS_R] = dUnLoadPositionR;
}

void CPositionConfigMgr::GetLoaderMeasPosition(double *dMeasPositionX, double *dMeasPositionY, double *dMeasPositionR) const
{
	*dMeasPositionX = m_stPositionParam.dMeasPosition[enumAXIS_X];
	*dMeasPositionY = m_stPositionParam.dMeasPosition[enumAXIS_Y];
	*dMeasPositionR = m_stPositionParam.dMeasPosition[enumAXIS_R];
}

void CPositionConfigMgr::SetLoaderMeasPosition(double dMeasPositionX, double dMeasPositionY, double dMeasPositionR)
{
	m_stPositionParam.dMeasPosition[enumAXIS_X] = dMeasPositionX;
	m_stPositionParam.dMeasPosition[enumAXIS_Y] = dMeasPositionY;
	m_stPositionParam.dMeasPosition[enumAXIS_R] = dMeasPositionR;
}

void CPositionConfigMgr::GetPickerPosition(double *dSafetyStageX) const
{
	*dSafetyStageX = m_stPositionParam.dPickerStageX;
}

void CPositionConfigMgr::GetAlignToMeasGap(double *dGap) const
{
	*dGap = m_stPositionParam.dAlignToMeasGap;
}

void CPositionConfigMgr::GetACPosition(double* dChangePos, double* dSafetyPos, double* dSaccPos) const
{
	if (dChangePos != nullptr)  *dChangePos = m_stPositionParam.dACPosition[0];
	if (dSafetyPos != nullptr)  *dSafetyPos = m_stPositionParam.dACPosition[1];
	if (dSaccPos != nullptr)	*dSaccPos   = m_stPositionParam.dACPosition[2];
}

void CPositionConfigMgr::SetACPosition(double dChangePos, double dSafetyPos, double dSaccPos)
{
	m_stPositionParam.dACPosition[0] = dChangePos;
	m_stPositionParam.dACPosition[1] = dSafetyPos;
	m_stPositionParam.dACPosition[2] = dSaccPos;
}

void CPositionConfigMgr::GetACZPosition(double* dChangePos, double* dSafetyPos, double* dSaccPos) const
{
	if (dChangePos != nullptr)  *dChangePos = m_stPositionParam.dACZPosition[0];
	if (dSafetyPos != nullptr)  *dSafetyPos = m_stPositionParam.dACZPosition[1];
	if (dSaccPos != nullptr)	*dSaccPos = m_stPositionParam.dACZPosition[2];

//	*dChangePos = m_stPositionParam.dACZPosition;
}

void CPositionConfigMgr::SetACZPosition(double dChangePos, double dSafetyPos, double dSaccPos)
{
	m_stPositionParam.dACZPosition[0] = dChangePos;
	m_stPositionParam.dACZPosition[1] = dSafetyPos;
	m_stPositionParam.dACZPosition[2] = dSaccPos;

//	m_stPositionParam.dACZPosition = dChangePos;
}

void CPositionConfigMgr::GetProbeChangePos(double* dChangePos) const
{
	*dChangePos = m_stPositionParam.dProbeChangePos;
}

void CPositionConfigMgr::SetProbeChangePos(double dChangePos)
{
	m_stPositionParam.dProbeChangePos = dChangePos;
}

void CPositionConfigMgr::GetProbeZChangePos(double* dChangePos, double* dSafetyPos, double* dZeroPos) const
{
	if (dChangePos != nullptr)  *dChangePos = m_stPositionParam.dProbeZChangePos[0];
	if (dSafetyPos != nullptr)  *dSafetyPos = m_stPositionParam.dProbeZChangePos[1];
	if (dZeroPos != nullptr)	*dZeroPos = m_stPositionParam.dProbeZChangePos[2];
}

void CPositionConfigMgr::SetProbeZChangePos(double dChangePos)
{
	m_stPositionParam.dProbeZChangePos[0] = dChangePos;
}

void CPositionConfigMgr::GetProbeSafetyPosition(double* dSafetyProbe1, double* dSafetyProbe2, double* dSafetyProbe3, double* dSafetyProbe4, double* dSafetyProbe5) const
{
	*dSafetyProbe1 = m_stPositionParam.dProbeSafety[0];
	*dSafetyProbe2 = m_stPositionParam.dProbeSafety[1];
	*dSafetyProbe3 = m_stPositionParam.dProbeSafety[2];
	*dSafetyProbe4 = m_stPositionParam.dProbeSafety[3];
	*dSafetyProbe5 = m_stPositionParam.dProbeSafety[4];
}

void CPositionConfigMgr::SetProbeSafetyPosition(double dSafetyProbe1, double dSafetyProbe2, double dSafetyProbe3, double dSafetyProbe4, double dSafetyProbe5)
{
	m_stPositionParam.dProbeSafety[0] = dSafetyProbe1;
	m_stPositionParam.dProbeSafety[1] = dSafetyProbe2;
	m_stPositionParam.dProbeSafety[2] = dSafetyProbe3;
	m_stPositionParam.dProbeSafety[3] = dSafetyProbe4;
	m_stPositionParam.dProbeSafety[4] = dSafetyProbe5;

}

void CPositionConfigMgr::GetHeadOffset(int iHead, int iMode, double* dPosX, double* dPosY, double* dPosZ) const
{
	*dPosX = m_stPositionParam.dHeadOffset[iHead][iMode].dX;
	*dPosY = m_stPositionParam.dHeadOffset[iHead][iMode].dY;
	*dPosZ = m_stPositionParam.dHeadOffset[iHead][iMode].dZ;
}

//void CPositionConfigMgr::GetHeadGap(double* dPos) const
//{
//	*dPos = m_stPositionParam.dHeadGap;
//}
//
//void CPositionConfigMgr::GetGantryGap(double* dPos) const
//{
//	*dPos = m_stPositionParam.dGantryGap;
//}
//
//void CPositionConfigMgr::GetProbeGap(double* dPos) const
//{
//	*dPos = m_stPositionParam.dProbeGap;
//}

void CPositionConfigMgr::GetProbeOffsetPos(int iProbe, double* dProbeX, double* dProbeY, double* dProbeT) const
{
	if (dProbeX != nullptr) *dProbeX = m_stPositionParam.dProbeXOffsetPos[iProbe];
	if (dProbeY != nullptr) *dProbeY = m_stPositionParam.dProbeYOffsetPos[iProbe];
	if (dProbeY != nullptr) *dProbeT = m_stPositionParam.dProbeTOffsetPos[iProbe];
}


void CPositionConfigMgr::GetAlignPosition(double* dAlignPos) const
{
	if (dAlignPos != nullptr) *dAlignPos = m_stPositionParam.dAlignPos;
}
