//#include "pch.h"
#include "pch.h"
#include "ShareMemFunction.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CShareMemFunction::CShareMemFunction()
{
    m_hVTMSysMap = NULL;
    m_pVTMSystem = nullptr;
}

CShareMemFunction::~CShareMemFunction()
{
    if (m_pVTMSystem)
	{
        UnmapViewOfFile(m_pVTMSystem);
	}
    if (m_hVTMSysMap)
	{
        CloseHandle(m_hVTMSysMap);
	}
}

VTM_SYSTEM *CShareMemFunction::GetSharedMemory()
{
    CCriticalSectionSync single(&m_cs, TRUE);

	if (!m_hVTMSysMap && !m_pVTMSystem)
    {
        m_hVTMSysMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof( VTM_SYSTEM ), WBS_SHARE_MEM_STRING);

        if(!m_hVTMSysMap)
        {
			CString str;
			str.Format(_T("CreateFileMapping Failed ( %s )"), WBS_SHARE_MEM_STRING);
            AfxMessageBox(str);
            return nullptr;
        }

        m_pVTMSystem = (VTM_SYSTEM *) MapViewOfFile(m_hVTMSysMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof( VTM_SYSTEM ));

        if(!m_pVTMSystem)
        {
            CloseHandle(m_hVTMSysMap);

			CString str;
			str.Format(_T("MapViewOfFile Failed ( %s )"), WBS_SHARE_MEM_STRING);
            AfxMessageBox(str);
            return nullptr;
        }
    }

    return m_pVTMSystem;
}

HWND CShareMemFunction::GetMetrosysViewWnd()
{
    if (m_pVTMSystem != nullptr)
        return m_pVTMSystem->hMetrosysView;
    else
        return nullptr;
}

void CShareMemFunction::SetMetrosysViewWnd(HWND hWnd)
{
    if (m_pVTMSystem != nullptr)
        m_pVTMSystem->hMetrosysView = hWnd;
}

void CShareMemFunction::SetSys_AlarmStatus(int nState)
{
    if (m_pVTMSystem != nullptr)
        m_pVTMSystem->stEquip.m_nAlarmStatus = nState;
}

void CShareMemFunction::SetSys_ProcessStatus(int nState)
{
    if (m_pVTMSystem != nullptr)
        m_pVTMSystem->stEquip.m_nProcessStatus = nState;
}

int CShareMemFunction::GetSys_ProcessStatus()
{
    if (m_pVTMSystem != nullptr)
        return m_pVTMSystem->stEquip.m_nProcessStatus;
    else
        return 0;
}

void CShareMemFunction::SetSys_UserPermit(char *szPermit)
{
    if (m_pVTMSystem != nullptr)
        strcpy_s(m_pVTMSystem->stEquip.m_szUserPermit, (size_t)sizeof(m_pVTMSystem->stEquip.m_szUserPermit), szPermit);
}

int CShareMemFunction::GetSys_AlarmStatus()
{
    if (m_pVTMSystem != nullptr)
        return m_pVTMSystem->stEquip.m_nAlarmStatus;
    else
        return 0;
}

CString CShareMemFunction::GetSys_UserPermit()
{
    if (m_pVTMSystem != nullptr)
        return CString(m_pVTMSystem->stEquip.m_szUserPermit);
    else
        return _T("");
}

void CShareMemFunction::SetSys_User(char *szUser)
{
    if (m_pVTMSystem != nullptr)
        strcpy_s(m_pVTMSystem->stEquip.m_szUser, sizeof(m_pVTMSystem->stEquip.m_szUser), szUser);
}

CString CShareMemFunction::GetSys_User()
{
    if (m_pVTMSystem != nullptr)
        return CString(m_pVTMSystem->stEquip.m_szUser);
    else
        return _T("");
}

int CShareMemFunction::GetSys_UserNo()
{
    if (m_pVTMSystem != nullptr)
        return m_pVTMSystem->stEquip.m_NoUser;
    else
        return 0;
}

void CShareMemFunction::SetSys_UserNo(int nNoUser)
{
    if (m_pVTMSystem != nullptr)
        m_pVTMSystem->stEquip.m_NoUser = nNoUser;
}

void CShareMemFunction::SetSys_MachineMode(int nMode)
{
    if (m_pVTMSystem != nullptr)
        m_pVTMSystem->stEquip.m_nEQStatus = nMode;
}

int CShareMemFunction::GetSys_MachineMode()
{
    if (m_pVTMSystem != nullptr)
        return m_pVTMSystem->stEquip.m_nEQStatus;
    else
        return 0;
}

void CShareMemFunction::SetSys_MetrosysVersion(char *szMetrosysVersion)
{
    if (m_pVTMSystem != nullptr)
        strcpy_s(m_pVTMSystem->stEquip.m_szMetrosysVersion, sizeof(m_pVTMSystem->stEquip.m_szMetrosysVersion), szMetrosysVersion);
}

CString CShareMemFunction::GetSys_MetrosysVersion()
{
    if (m_pVTMSystem != nullptr)
        return CString(m_pVTMSystem->stEquip.m_szMetrosysVersion);
    else
        return _T("");
}

void CShareMemFunction::SetSys_MachineType(int nMode)
{
    if (m_pVTMSystem != nullptr)
        m_pVTMSystem->stEquip.m_nEQType = nMode;
}

int CShareMemFunction::GetSys_MachineType()
{
    if (m_pVTMSystem != nullptr)
        return m_pVTMSystem->stEquip.m_nEQType;
    else
        return 0;
}

void CShareMemFunction::SetSys_ProbeChange(int nMode)
{
    if (m_pVTMSystem != nullptr)
        m_pVTMSystem->m_nProbeChange = nMode;
}

int CShareMemFunction::GetSys_ProbeChange()
{
    if (m_pVTMSystem != nullptr)
        return m_pVTMSystem->m_nProbeChange;
    else
        return 0;
}