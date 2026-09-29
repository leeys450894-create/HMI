//#include "stdafx.h"
#include "pch.h"
#include "ShareMemoryMgr.h"

#include "common\IniHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CShareMemoryMgr::CShareMemoryMgr(CString strAppPath, CString strConfigPath)
    : m_pIniHelper(new CIniHelper)
    , m_strAppPath(strConfigPath)
{
    SetName(_T("SHARE_MEMORY_MGR"));

	const CString strFilePath = strConfigPath + _T("\\CoreFunction\\CoreFunction.ini");
    m_pIniHelper->SetFilePath(strFilePath);

    LinkIniFile(false);

    //----------------------------------------------------------------------
    //----------------------------------------------------------------------
	m_pFunction = new CShareMemFunction;
}

CShareMemoryMgr::~CShareMemoryMgr()
{
    Finalize();
}

bool CShareMemoryMgr::LinkIniFile(bool bSave)
{
    m_pIniHelper->Link(bSave, GetName(), _T("USE"),				&m_bUse,				true);
    m_pIniHelper->Link(bSave, GetName(), _T("OUT_OF_SERVICE"),	&m_bOutOfService,		false);
    m_pIniHelper->Link(bSave, GetName(), _T("DEVICE_TYPE"),		&m_strControlType,		(CString)_T("OM"));

    return m_pIniHelper->IsReadEnable();
}

void CShareMemoryMgr::Finalize()
{
    //----------------------------------------------------------------------
    // 데이터 삭제
    //----------------------------------------------------------------------
	if(m_pFunction)
	{
		delete m_pFunction;
		m_pFunction = nullptr;
	}
	
	if(m_pIniHelper)
    {
        delete m_pIniHelper;
        m_pIniHelper = nullptr;
    }
}




VTM_SYSTEM *CShareMemoryMgr::GetSharedMemory()
{
	CCriticalSectionSync single(&m_cs_get, TRUE);

	return m_pFunction->GetSharedMemory();
}

HWND CShareMemoryMgr::GetMetrosysViewWnd()
{
	CCriticalSectionSync single(&m_cs_get, TRUE);

	return m_pFunction->GetMetrosysViewWnd();
}

void CShareMemoryMgr::SetMetrosysViewWnd(HWND hWnd)
{
	CCriticalSectionSync single(&m_cs_set, TRUE);

    return m_pFunction->SetMetrosysViewWnd(hWnd);
}

void CShareMemoryMgr::SetSys_AlarmStatus(int nState)
{
	CCriticalSectionSync single(&m_cs_set, TRUE);

    return m_pFunction->SetSys_AlarmStatus(nState);
}

void CShareMemoryMgr::SetSys_ProcessStatus(int nState)
{
	CCriticalSectionSync single(&m_cs_set, TRUE);

    return m_pFunction->SetSys_ProcessStatus(nState);
}

int CShareMemoryMgr::GetSys_ProcessStatus()
{
	CCriticalSectionSync single(&m_cs_get, TRUE);

    return m_pFunction->GetSys_ProcessStatus();
}

void CShareMemoryMgr::SetSys_UserPermit(char *szPermit)
{
	CCriticalSectionSync single(&m_cs_set, TRUE);

    return m_pFunction->SetSys_UserPermit(szPermit);
}

int CShareMemoryMgr::GetSys_AlarmStatus()
{
	CCriticalSectionSync single(&m_cs_get, TRUE);

    return m_pFunction->GetSys_AlarmStatus();
}

CString CShareMemoryMgr::GetSys_UserPermit()
{
	CCriticalSectionSync single(&m_cs_get, TRUE);

    return m_pFunction->GetSys_UserPermit();
}

void CShareMemoryMgr::SetSys_User(char *szUser)
{
	CCriticalSectionSync single(&m_cs_set, TRUE);

    return m_pFunction->SetSys_User(szUser);
}

CString CShareMemoryMgr::GetSys_User()
{
	CCriticalSectionSync single(&m_cs_get, TRUE);

    return m_pFunction->GetSys_User();
}

int CShareMemoryMgr::GetSys_UserNo()
{
	CCriticalSectionSync single(&m_cs_get, TRUE);

    return m_pFunction->GetSys_UserNo();
}

void CShareMemoryMgr::SetSys_UserNo(int nNoUser)
{
	CCriticalSectionSync single(&m_cs_set, TRUE);

    return m_pFunction->SetSys_UserNo(nNoUser);
}

void CShareMemoryMgr::SetSys_MachineMode(int nMode)
{
	CCriticalSectionSync single(&m_cs_set, TRUE);

    return m_pFunction->SetSys_MachineMode(nMode);
}

int CShareMemoryMgr::GetSys_MachineMode()
{
	CCriticalSectionSync single(&m_cs_get, TRUE);

    return m_pFunction->GetSys_MachineMode();
}

void CShareMemoryMgr::SetSys_MetrosysVersion(char *szMetrosysVersion)
{
	CCriticalSectionSync single(&m_cs_set, TRUE);

    return m_pFunction->SetSys_MetrosysVersion(szMetrosysVersion);
}

CString CShareMemoryMgr::GetSys_MetrosysVersion()
{
	CCriticalSectionSync single(&m_cs_get, TRUE);

    return m_pFunction->GetSys_MetrosysVersion();
}

CString CShareMemoryMgr::GetSys_AppPath()
{
    CCriticalSectionSync single(&m_cs_get, TRUE);

    return m_strAppPath;
}

void CShareMemoryMgr::SetSys_MachineType(int nMode)
{
    CCriticalSectionSync single(&m_cs_set, TRUE);

    return m_pFunction->SetSys_MachineType(nMode);
}

int CShareMemoryMgr::GetSys_MachineType()
{
    CCriticalSectionSync single(&m_cs_get, TRUE);

    return m_pFunction->GetSys_MachineType();
}

void CShareMemoryMgr::SetSys_Validation(bool bOn)
{
    CCriticalSectionSync single(&m_cs_get, TRUE);

    m_pFunction->GetSharedMemory()->m_bValidation = bOn;
}

bool CShareMemoryMgr::GetSys_Validation()
{
    CCriticalSectionSync single(&m_cs_get, TRUE);

    return m_pFunction->GetSharedMemory()->m_bValidation;
}

void CShareMemoryMgr::SetSys_GlassJudge(bool bOn)
{
    CCriticalSectionSync single(&m_cs_get, TRUE);

    m_pFunction->GetSharedMemory()->m_bGlassJudge = bOn;
}

bool CShareMemoryMgr::GetSys_GlassJudge()
{
    CCriticalSectionSync single(&m_cs_get, TRUE);

    return m_pFunction->GetSharedMemory()->m_bGlassJudge;
}

void CShareMemoryMgr::SetSys_AutoAgingMode(bool bOn)
{
    CCriticalSectionSync single(&m_cs_get, TRUE);

    m_pFunction->GetSharedMemory()->m_bAutoAgingMode = bOn;
}

bool CShareMemoryMgr::GetSys_AutoAgingMode()
{
    CCriticalSectionSync single(&m_cs_get, TRUE);

    return m_pFunction->GetSharedMemory()->m_bAutoAgingMode;
}

// LYS 2026-09-28 EFA: EFA 2-pass 검사 모드 플래그 (OCP_EFA m_bNewAgingMode)
void CShareMemoryMgr::SetSys_NewAgingMode(bool bOn)
{
    CCriticalSectionSync single(&m_cs_get, TRUE);

    m_pFunction->GetSharedMemory()->m_bNewAgingMode = bOn;
}

bool CShareMemoryMgr::GetSys_NewAgingMode()
{
    CCriticalSectionSync single(&m_cs_get, TRUE);

    return m_pFunction->GetSharedMemory()->m_bNewAgingMode;
}

void CShareMemoryMgr::SetSys_LampTimeOver(int index, bool bOn)
{
	CCriticalSectionSync single(&m_cs_get, TRUE);

	m_pFunction->GetSharedMemory()->m_bAlarm_LampTimeOver[index] = bOn;
}

bool CShareMemoryMgr::GetSys_LampTimeOver(int index)
{
	CCriticalSectionSync single(&m_cs_get, TRUE);

	return m_pFunction->GetSharedMemory()->m_bAlarm_LampTimeOver[index];
}

void CShareMemoryMgr::SetSys_TemperOver(int index, bool bOn)
{
	CCriticalSectionSync single(&m_cs_get, TRUE);

	m_pFunction->GetSharedMemory()->m_bAlarm_TemperOver[index] = bOn;
}

bool CShareMemoryMgr::GetSys_TemperOver(int index)
{
	CCriticalSectionSync single(&m_cs_get, TRUE);

	return m_pFunction->GetSharedMemory()->m_bAlarm_TemperOver[index];
}

void CShareMemoryMgr::SetSys_ProbeChange(int index)
{
	CCriticalSectionSync single(&m_cs_get, TRUE);

	return m_pFunction->SetSys_ProbeChange(index);
}

int CShareMemoryMgr::GetSys_ProbeChange()
{
	CCriticalSectionSync single(&m_cs_get, TRUE);

	return m_pFunction->GetSys_ProbeChange();
}

// LYS 2026-09-25 설비 내 프로브 ID 접근자 (헤더 주석 참조)
static int __EqProbeIndex(int iProbeUnit)
{
	return (iProbeUnit >= CShareMemoryMgr::EQ_PROBE_UNIT_MIN && iProbeUnit <= CShareMemoryMgr::EQ_PROBE_UNIT_MAX) ? (iProbeUnit - 1) : -1;
}

CString CShareMemoryMgr::GetEqProbeID(int iProbeUnit)
{
	CCriticalSectionSync single(&m_cs_get, TRUE);
	const int idx = __EqProbeIndex(iProbeUnit);
	VTM_SYSTEM* p = (m_pFunction != nullptr) ? m_pFunction->GetSharedMemory() : nullptr;
	if (idx < 0 || p == nullptr) return _T("");
	CString str(p->m_szPROBEID[idx]);
	str.Trim();
	return str;
}

bool CShareMemoryMgr::SetEqProbeID(int iProbeUnit, LPCTSTR pszID)
{
	CCriticalSectionSync single(&m_cs_get, TRUE);
	const int idx = __EqProbeIndex(iProbeUnit);
	VTM_SYSTEM* p = (m_pFunction != nullptr) ? m_pFunction->GetSharedMemory() : nullptr;
	if (idx < 0 || p == nullptr) return false;
	_tcscpy_s(p->m_szPROBEID[idx], _countof(p->m_szPROBEID[idx]), (pszID != nullptr) ? pszID : _T(""));
	return true;
}

CString CShareMemoryMgr::GetEqProbeMountTime(int iProbeUnit)
{
	CCriticalSectionSync single(&m_cs_get, TRUE);
	const int idx = __EqProbeIndex(iProbeUnit);
	VTM_SYSTEM* p = (m_pFunction != nullptr) ? m_pFunction->GetSharedMemory() : nullptr;
	if (idx < 0 || p == nullptr) return _T("");
	CString str(p->stProbeInfo[idx].m_szProbeCardIDTime);
	str.Trim();
	return str;
}

UINT CShareMemoryMgr::GetEqProbeUseCnt(int iProbeUnit)
{
	CCriticalSectionSync single(&m_cs_get, TRUE);
	const int idx = __EqProbeIndex(iProbeUnit);
	VTM_SYSTEM* p = (m_pFunction != nullptr) ? m_pFunction->GetSharedMemory() : nullptr;
	if (idx < 0 || p == nullptr) return 0;
	return p->stProbeInfo[idx].m_nUseCnt;
}

// Probe Align 레시피명: P1~P4 = PROBE_ALIGN<n>, P5(및 범위 밖) = PROBE_MEAS. ID 가 있으면 "_" + 앞 2자를 붙인다.
//   SystemInitializeStep / DoWorkGlassAdsorptionStep / FunctionModuleProbeACCStep 에 있던 같은 switch 3벌을 대신한다.
CString CShareMemoryMgr::GetEqProbeAlignRecipeName(int iProbeUnit)
{
	CString strName;
	if (iProbeUnit >= EQ_PROBE_UNIT_MIN && iProbeUnit < EQ_PROBE_UNIT_MEAS)
		strName.Format(_T("PROBE_ALIGN%d"), iProbeUnit);
	else
		strName = _T("PROBE_MEAS");

	const CString strID = GetEqProbeID(iProbeUnit);
	if (strID.IsEmpty() == FALSE)
		strName.AppendFormat(_T("_%s"), (LPCTSTR)strID.Left(2));

	return strName;
}
