// AutoContactInfo.cpp
// LYS 2026-08-10 AutoContact Phase 2: 비UI 프로브 오토 컨택 저장소.
// INI 키는 OCP CDlgNaviManualAutoContact([PROBE_AUTO_CONTACT])와 동일.
#include "pch.h"
#include "AutoContactInfo.h"
#include "..\SystemParamShare.h"
#include "..\Manager\MaintControlMgr.h"		// LYS 2026-08-10 Phase 4
#include "..\Manager\CommTcpIpFactoryMgr.h"	// LYS 2026-08-10 Phase 4

static LPCTSTR s_szSection = _T("PROBE_AUTO_CONTACT");

// param INI keys, AUTO_CT_* order (load accepts legacy "Portfail" case - INI is case-insensitive)
static LPCTSTR s_szParamKey[AUTO_CONTACT_ITEM_MAX] =
{
	_T("ReadyPos"),			// AUTO_CT_READY
	_T("FirstLimitPos"),	// AUTO_CT_FLIMIT
	_T("FirstMoveGap"),		// AUTO_CT_FGAP
	_T("SecondMoveGap"),	// AUTO_CT_SGAP
	_T("ODMax"),			// AUTO_CT_OD_MAX
	_T("Percent"),			// AUTO_CT_PERCENT
	_T("PortFail"),			// AUTO_CT_PORT_FAIL
	_T("AddOD"),			// AUTO_CT_ADD_OD
};

CAutoContactInfo* CAutoContactInfo::m_pInstance = NULL;

CAutoContactInfo* CAutoContactInfo::GetInstance()
{
	if (m_pInstance == NULL)
	{
		m_pInstance = new CAutoContactInfo;
		m_pInstance->LoadAll();
	}
	return m_pInstance;
}

void CAutoContactInfo::DelInstance()
{
	if (m_pInstance != NULL)
	{
		delete m_pInstance;
		m_pInstance = NULL;
	}
}

CAutoContactInfo::CAutoContactInfo()
{
	ZeroMemory(m_fParam, sizeof(m_fParam));
	m_bUse = m_bModelChange = m_bProbeChange = m_bAlways = false;
	m_fReady = m_fPreContact = m_fOD = 0.0;
	m_strRcpID = _T("UNKNOWN");
	m_strProbeID = _T("UNKNOWN");
	m_bSuccess = false;
	ZeroMemory(m_nCellStatus, sizeof(m_nCellStatus));
	m_bStopReq = false;		// LYS 2026-08-10 Phase 4
}

CAutoContactInfo::~CAutoContactInfo()
{
}

CString CAutoContactInfo::GetIniPath() const
{
	CString strPath;
	strPath.Format(_T("%s\\Config.ini"), CSystemParamShare::GetInstance()->GetConfigFolderPath());
	return strPath;
}

CString CAutoContactInfo::GetLogPath() const
{
	CString strPath;
	strPath.Format(_T("%s\\AutoContactLog.ini"), CSystemParamShare::GetInstance()->GetConfigFolderPath());
	return strPath;
}

void CAutoContactInfo::WriteIniDouble(LPCTSTR szKey, double fVal)
{
	CString strTemp;
	strTemp.Format(_T("%.4lf"), fVal);
	WritePrivateProfileString(s_szSection, szKey, strTemp, GetIniPath());
}

void CAutoContactInfo::WriteIniInt(LPCTSTR szKey, int nVal)
{
	CString strTemp;
	strTemp.Format(_T("%d"), nVal);
	WritePrivateProfileString(s_szSection, szKey, strTemp, GetIniPath());
}

void CAutoContactInfo::WriteIniStr(LPCTSTR szKey, LPCTSTR szVal)
{
	WritePrivateProfileString(s_szSection, szKey, szVal, GetIniPath());
}

void CAutoContactInfo::LoadAll()
{
	CString strPath = GetIniPath();
	TCHAR szValue[MAX_PATH];

	for (int i = 0; i < AUTO_CONTACT_ITEM_MAX; i++)
	{
		ZeroMemory(szValue, sizeof(szValue));
		GetPrivateProfileString(s_szSection, s_szParamKey[i], _T("0.0"), szValue, MAX_PATH, strPath);
		m_fParam[i] = _tstof(szValue);
	}

	ZeroMemory(szValue, sizeof(szValue));
	GetPrivateProfileString(s_szSection, _T("ContactDataReady"), _T("0.0"), szValue, MAX_PATH, strPath);
	m_fReady = _tstof(szValue);
	ZeroMemory(szValue, sizeof(szValue));
	GetPrivateProfileString(s_szSection, _T("ContactDataPos1"), _T("0.0"), szValue, MAX_PATH, strPath);
	m_fPreContact = _tstof(szValue);
	ZeroMemory(szValue, sizeof(szValue));
	GetPrivateProfileString(s_szSection, _T("ContactDataPos2"), _T("0.0"), szValue, MAX_PATH, strPath);
	m_fOD = _tstof(szValue);

	ZeroMemory(szValue, sizeof(szValue));
	GetPrivateProfileString(s_szSection, _T("ContactDataRcpID"), _T("UNKNOWN"), szValue, MAX_PATH, strPath);
	m_strRcpID = szValue;
	ZeroMemory(szValue, sizeof(szValue));
	GetPrivateProfileString(s_szSection, _T("ContactDataProbeID"), _T("UNKNOWN"), szValue, MAX_PATH, strPath);
	m_strProbeID = szValue;

	m_bSuccess     = GetPrivateProfileInt(s_szSection, _T("ContactDataSucess"),      0, strPath) != 0;
	m_bUse         = GetPrivateProfileInt(s_szSection, _T("ContactDataCntUse"),      0, strPath) != 0;
	m_bModelChange = GetPrivateProfileInt(s_szSection, _T("ContactDataModelChange"), 0, strPath) != 0;
	m_bProbeChange = GetPrivateProfileInt(s_szSection, _T("ContactDataProbeChange"), 0, strPath) != 0;
	m_bAlways      = GetPrivateProfileInt(s_szSection, _T("ContactDataAlways"),      0, strPath) != 0;
}

void CAutoContactInfo::SaveParams()
{
	for (int i = 0; i < AUTO_CONTACT_ITEM_MAX; i++)
	{
		WriteIniDouble(s_szParamKey[i], m_fParam[i]);
	}
}

double CAutoContactInfo::GetParam(int iIdx) const
{
	if (iIdx < 0 || iIdx >= AUTO_CONTACT_ITEM_MAX) return 0.0;
	return m_fParam[iIdx];
}

void CAutoContactInfo::SetParam(int iIdx, double fVal)
{
	if (iIdx < 0 || iIdx >= AUTO_CONTACT_ITEM_MAX) return;
	m_fParam[iIdx] = fVal;
}

void CAutoContactInfo::SetUse(bool bUse)
{
	m_bUse = bUse;
	WriteIniInt(_T("ContactDataCntUse"), bUse ? 1 : 0);
}

void CAutoContactInfo::SetModelChangeUse(bool bUse)
{
	m_bModelChange = bUse;
	WriteIniInt(_T("ContactDataModelChange"), bUse ? 1 : 0);
}

void CAutoContactInfo::SetProbeChangeUse(bool bUse)
{
	m_bProbeChange = bUse;
	WriteIniInt(_T("ContactDataProbeChange"), bUse ? 1 : 0);
}

void CAutoContactInfo::SetAlwaysUse(bool bUse)
{
	m_bAlways = bUse;
	WriteIniInt(_T("ContactDataAlways"), bUse ? 1 : 0);
}

void CAutoContactInfo::SavePos(double fReady, double fPreContact, double fOD)
{
	m_fReady = fReady;
	m_fPreContact = fPreContact;
	m_fOD = fOD;
	WriteIniDouble(_T("ContactDataReady"), m_fReady);
	WriteIniDouble(_T("ContactDataPos1"), m_fPreContact);
	WriteIniDouble(_T("ContactDataPos2"), m_fOD);
}

void CAutoContactInfo::SetRcpID(CString strID)
{
	m_strRcpID = strID;
	WriteIniStr(_T("ContactDataRcpID"), m_strRcpID);
}

void CAutoContactInfo::SetProbeID(CString strID)
{
	m_strProbeID = strID;
	WriteIniStr(_T("ContactDataProbeID"), m_strProbeID);
}

void CAutoContactInfo::SetSuccess(bool bSuccess)
{
	m_bSuccess = bSuccess;
	WriteIniInt(_T("ContactDataSucess"), bSuccess ? 1 : 0);
}

void CAutoContactInfo::Reset()
{
	// OCP CDlgNaviManualAutoContact::AutoCntPosReset()
	SavePos(0.0, 0.0, 0.0);
	SetSuccess(false);
}

void CAutoContactInfo::SaveLog()
{
	// OCP CDlgNaviManualAutoContact::SetAutoCntLog(): per-probe section history
	if (m_strProbeID.IsEmpty()) return;

	CString strPath = GetLogPath();
	CString strTemp;
	strTemp.Format(_T("%.4lf"), m_fReady);
	WritePrivateProfileString(m_strProbeID, _T("Ready"), strTemp, strPath);
	strTemp.Format(_T("%.4lf"), m_fPreContact);
	WritePrivateProfileString(m_strProbeID, _T("ContactPos"), strTemp, strPath);
	strTemp.Format(_T("%.4lf"), m_fOD);
	WritePrivateProfileString(m_strProbeID, _T("OD"), strTemp, strPath);
}

void CAutoContactInfo::CheckAndReset(CString strRecipeID, CString strProbeCardID)
{
	// OCP JOB_Proc measure-start block (20200131 add by Hee)
	if (m_bAlways)
	{
		Reset();
		SetProbeID(strProbeCardID);
	}
	else
	{
		if (m_bModelChange)
		{
			if (m_strRcpID != strRecipeID)
			{
				Reset();
				SetRcpID(strRecipeID);
			}
		}
		if (m_bProbeChange)
		{
			if (m_strProbeID != strProbeCardID)
			{
				Reset();
				SetProbeID(strProbeCardID);
			}
		}
	}
}

void CAutoContactInfo::ClearCellStatus()
{
	ZeroMemory(m_nCellStatus, sizeof(m_nCellStatus));
}

void CAutoContactInfo::SetCellStatus(int iCell, int nStatus)
{
	if (iCell < 0 || iCell >= RTEST_MAX) return;
	m_nCellStatus[iCell] = nStatus;
}

int CAutoContactInfo::GetCellStatus(int iCell) const
{
	if (iCell < 0 || iCell >= RTEST_MAX) return 0;
	return m_nCellStatus[iCell];
}

// LYS 2026-08-10 Phase 4: 2단계 컨택 탐색(OCP CJOB_Proc::DoWorkAutoContact 이식).
// 1단계: FirstMoveGap씩 하강, 셀 하나라도 R-Test 통과까지(한계 FirstLimitPos).
// 2단계: SecondMoveGap씩 하강, 통과율 >= Percent까지(한계 ODMax).
// 종료: PreContact + OD + AddOD로 이동. 더 깊은 결과만 저장.
// 축 = AXIS_PROBE_Z5 (C1: OCP 모드 P5 전용. OCP 원본은 AXIS_UNIT_Z).
int CAutoContactInfo::DoSearch(CMaintControlMgr* pMaint, CCommTcpIpFactoryMgr* pComm,
							   int nCellCount, std::function<bool()> fnStopCheck /*= nullptr*/)
{

	Sleep(1000);
	if (pMaint == NULL || pComm == NULL) return VTM_RV_FAILED;
	if (nCellCount <= 0) return VTM_RV_FAILED;
	if (nCellCount > RTEST_MAX) nCellCount = RTEST_MAX;

	long nRtn = VTM_RV_FAILED;
	double fTempGap = 0.0;
	double fFirstGap = m_fParam[AUTO_CT_FGAP];
	double fSecondGap = m_fParam[AUTO_CT_SGAP];

	// zero gap would never advance - treat as setup error
	if (fFirstGap <= 0.0 || fSecondGap <= 0.0) return PROBE_AUTO_CNT_1STEP_FAIL;

	// move to ready position
	nRtn = pMaint->MoveCmdPosA(AXIS_PROBE_Z5, m_fParam[AUTO_CT_READY], true);
	if (nRtn != VTM_RV_SUCCESSED) return (int)nRtn;
	// LYS 2026-08-11: explicit in-position check after the move
	nRtn = pMaint->DoWork_MotionDone(AXIS_PROBE_Z5);
	if (nRtn != VTM_RV_SUCCESSED) return (int)nRtn;

	int nPortFailCnt = 0;
	int nPortFail = (int)m_fParam[AUTO_CT_PORT_FAIL] + 1;

	// step 1: find the first contact position (any cell R-Test pass)
	bool bRtestOK = false;
	while (1)
	{
		if (m_bStopReq) return VTM_RV_STOP_BY_USER;
		if (fnStopCheck && fnStopCheck()) return VTM_RV_STOP_BY_USER;

		nRtn = pComm->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_3);
		if (nRtn != VTM_RV_SUCCESSED) break;

		for (int i = 0; i < nCellCount; i++)
		{
			if (m_nCellStatus[i] == 21) bRtestOK = true;
			if (m_nCellStatus[i] == 28) nPortFailCnt++;
		}

		if (nPortFailCnt > nPortFail)
		{
			nRtn = PROBE_AUTO_CNT_PORT_FAIL;
			break;
		}
		nPortFailCnt = 0;

		if (bRtestOK) break;

		fTempGap += fFirstGap;
		if (m_fParam[AUTO_CT_READY] + fTempGap > m_fParam[AUTO_CT_FLIMIT])
		{
			nRtn = PROBE_AUTO_CNT_1STEP_FAIL;
			break;
		}

		nRtn = pMaint->MoveCmdPosA(AXIS_PROBE_Z5, m_fParam[AUTO_CT_READY] + fTempGap, true, SPEED_LOW);
		if (nRtn != VTM_RV_SUCCESSED) break;
		nRtn = pMaint->DoWork_MotionDone(AXIS_PROBE_Z5);	// LYS 2026-08-11 in-position check
		if (nRtn != VTM_RV_SUCCESSED) break;
	}
	if (!bRtestOK) return (int)nRtn;

	double fFirstContactPos = m_fParam[AUTO_CT_READY] + fTempGap;
	fTempGap = 0.0;

	// step 2: find OD amount (pass ratio >= Percent)
	int nOkCnt = 0;
	double fPercent = m_fParam[AUTO_CT_PERCENT];

	bRtestOK = false;
	while (1)
	{
		if (m_bStopReq) return VTM_RV_STOP_BY_USER;
		if (fnStopCheck && fnStopCheck()) return VTM_RV_STOP_BY_USER;

		nRtn = pComm->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_3);
		if (nRtn != VTM_RV_SUCCESSED) break;

		for (int i = 0; i < nCellCount; i++)
		{
			if (m_nCellStatus[i] == 21) nOkCnt++;
		}

		if (((double)nOkCnt / (double)nCellCount) * 100.0 >= fPercent) bRtestOK = true;
		nOkCnt = 0;

		if (bRtestOK) break;

		fTempGap += fSecondGap;
		if (fTempGap > m_fParam[AUTO_CT_OD_MAX])
		{
			nRtn = PROBE_AUTO_CNT_2STEP_FAIL;
			break;
		}

		nRtn = pMaint->MoveCmdPosA(AXIS_PROBE_Z5, fFirstContactPos + fTempGap, true, SPEED_LOW);
		if (nRtn != VTM_RV_SUCCESSED) break;
		nRtn = pMaint->DoWork_MotionDone(AXIS_PROBE_Z5);	// LYS 2026-08-11 in-position check
		if (nRtn != VTM_RV_SUCCESSED) break;
	}
	if (!bRtestOK) return (int)nRtn;

	double fEndOD = fTempGap;

	// final contact: PreContact + OD + AddOD
	double fAddODMove = fEndOD + m_fParam[AUTO_CT_ADD_OD];
	nRtn = pMaint->MoveCmdPosA(AXIS_PROBE_Z5, fFirstContactPos + fAddODMove, true, SPEED_LOW);
	if (nRtn != VTM_RV_SUCCESSED) return (int)nRtn;
	// LYS 2026-08-11: contact done (in-position) check before finishing
	nRtn = pMaint->DoWork_MotionDone(AXIS_PROBE_Z5);
	if (nRtn != VTM_RV_SUCCESSED) return (int)nRtn;

	// persist only when deeper than the saved result (OCP rule)
	if (!m_bSuccess)
	{
		double fContactMax = m_fPreContact + m_fOD;
		if (fContactMax < (fFirstContactPos + fEndOD))
		{
			SavePos(m_fParam[AUTO_CT_READY], fFirstContactPos, fEndOD);
		}
	}

	return VTM_RV_SUCCESSED;
}

// LYS 2026-08-10 Phase 4: 저장된 탐색 결과로 컨택
// (OCP CJOB_Proc::DoWorkAutoContactDataMove 이식. 프로브 사용 기록은 미이식 - C6)
int CAutoContactInfo::DoDataMove(CMaintControlMgr* pMaint)
{
	if (pMaint == NULL) return VTM_RV_FAILED;

	long nRtn = pMaint->MoveCmdPosA(AXIS_PROBE_Z5, m_fReady, true);
	if (nRtn != VTM_RV_SUCCESSED) return (int)nRtn;
	nRtn = pMaint->DoWork_MotionDone(AXIS_PROBE_Z5);	// LYS 2026-08-11 in-position check
	if (nRtn != VTM_RV_SUCCESSED) return (int)nRtn;

	double fContactOD = m_fPreContact + m_fOD + m_fParam[AUTO_CT_ADD_OD];
	nRtn = pMaint->MoveCmdPosA(AXIS_PROBE_Z5, fContactOD, true, SPEED_LOW);
	if (nRtn != VTM_RV_SUCCESSED) return (int)nRtn;
	nRtn = pMaint->DoWork_MotionDone(AXIS_PROBE_Z5);	// LYS 2026-08-11 contact done check
	return (int)nRtn;
}
