//#include "stdafx.h"
#include "pch.h"

#include "NetworkConfigMgr.h"

#include "common\IniHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CNetworkConfigMgr::CNetworkConfigMgr(CString strConfigPath)
    : m_pIniHelper(new CIniHelper)
{
    SetName(_T("NETWORK_CONTROL_MGR"));

	m_pIniHelper->SetFilePath(strConfigPath + _T("\\Network.ini"));
	
	LinkIniFile(false);
}

CNetworkConfigMgr::~CNetworkConfigMgr()
{
	Finalize();
}

bool CNetworkConfigMgr::LinkIniFile(bool bSave)
{
	//-----------------------------------------------------------------------
    // Network Communication 파라미터 연동
    //-----------------------------------------------------------------------
	m_pIniHelper->Link(bSave, _T("COMMUNICATION_PARA"), _T("RESPONCE_TIME_LIMIT"), &m_fResponceTimeLimit_sec, 30.f);	// 30초 기본

	//-----------------------------------------------------------------------
    // Network TcpIp 파라미터 연동
    //-----------------------------------------------------------------------
	int iCount = (int)m_vecParam.size();

	if(bSave)
	{
		m_pIniHelper->Link(bSave, _T("NETWORK_PARA"), _T("TCPIP_COUNT"), &iCount, 5);
	}
	else
	{
		m_pIniHelper->Link(bSave, _T("NETWORK_PARA"), _T("TCPIP_COUNT"), &iCount, 5);

		m_vecParam.clear();
		m_vecParam.resize(iCount);
	}

    for(int i = 0; i < iCount; ++i)
    {
        CString strText;
        strText.Format(_T("NETWORK_PARA_%d_"), i);

		m_pIniHelper->Link(bSave, _T("NETWORK_PARA"), strText + _T("TCPIP_USE"),		&m_vecParam[i].bUse,			false);
		m_pIniHelper->Link(bSave, _T("NETWORK_PARA"), strText + _T("TCPIP_OUT_OF_SERVICE"),	&m_vecParam[i].bOutOfService,	false);
		m_pIniHelper->Link(bSave, _T("NETWORK_PARA"), strText + _T("TCPIP_NAME"),		&m_vecParam[i].strName,			(CString)_T(""));
		m_pIniHelper->Link(bSave, _T("NETWORK_PARA"), strText + _T("TCPIP_IP"),			&m_vecParam[i].strIPNumber,		(CString)_T("127.0.0.1"));
		m_pIniHelper->Link(bSave, _T("NETWORK_PARA"), strText + _T("TCPIP_PORT"),		&m_vecParam[i].strPortNumber,	(CString)_T("2001"));
		m_pIniHelper->Link(bSave, _T("NETWORK_PARA"), strText + _T("TCPIP_MODE"),		&m_vecParam[i].bConnetMode,		false);
		m_pIniHelper->Link(bSave, _T("NETWORK_PARA"), strText + _T("TCPIP_DEVICEID"),	&m_vecParam[i].iDeviceIndex,	i);
		m_pIniHelper->Link(bSave, _T("NETWORK_PARA"), strText + _T("TCPIP_MEAS_TYPE"),	&m_vecParam[i].iMeasType,	0);

		m_pIniHelper->Link(bSave, _T("NETWORK_PARA"), strText + _T("TCPIP_NET_DRIVE"),  &m_vecParam[i].strNetDrive,		(CString)_T("Z:"));
		m_pIniHelper->Link(bSave, _T("NETWORK_PARA"), strText + _T("TCPIP_RECIPE_DRIVE"), &m_vecParam[i].strNetRecipeDrive, (CString)_T("Z:"));
    }

	return m_pIniHelper->IsReadEnable();
}


void CNetworkConfigMgr::Finalize()
{
	if (m_pIniHelper)
	{
		delete m_pIniHelper;
		m_pIniHelper = nullptr;
	}
}



int CNetworkConfigMgr::GetCount() const
{
	return (int)m_vecParam.size();
}

bool CNetworkConfigMgr::GetParam_Use(int index) const
{
	if ((0 <= index) && (index < GetCount()))
	{
		return m_vecParam[index].bUse;
	}

	return false;
}

bool CNetworkConfigMgr::GetParam_OutOfService(int index) const
{
	if ((0 <= index) && (index < GetCount()))
	{
		return m_vecParam[index].bOutOfService;
	}

	return false;
}

CString CNetworkConfigMgr::GetParam_Name(int index) const	
{
	if( (0 <= index) && (index < GetCount()) )
	{
		return m_vecParam[index].strName;			
	}

	return _T("");
}

CString CNetworkConfigMgr::GetParam_IPNumber(int index) const	
{
	if( (0 <= index) && (index < GetCount()) )
	{
		return m_vecParam[index].strIPNumber;			
	}

	return _T("");
}

CString CNetworkConfigMgr::GetParam_PortNumber(int index) const
{ 
	if( (0 <= index) && (index < GetCount()) )
	{
		return m_vecParam[index].strPortNumber;			
	}

	return _T("");
}

long CNetworkConfigMgr::GetParam_PortNumger(int index)
{ 
	if( (0 <= index) && (index < GetCount()) )
	{
		return _ttoi(m_vecParam[index].strPortNumber);			
	}

	return 0;	
}

bool CNetworkConfigMgr::GetParam_Mode(int index) const
{ 
	if( (0 <= index) && (index < GetCount()) )
	{
		return m_vecParam[index].bConnetMode;			
	}

	return false;			
}

long CNetworkConfigMgr::GetParam_DeviceIndex(int index) const
{ 
	if( (0 <= index) && (index < GetCount()) )
	{
		return m_vecParam[index].iDeviceIndex;			
	}

	return -1;			
}

long CNetworkConfigMgr::GetParam_MeasType(int index) const
{
	if ((0 <= index) && (index < GetCount()))
	{
		return m_vecParam[index].iMeasType;
	}

	return -1;
}

CString CNetworkConfigMgr::GetParam_NetDrive(int index) const
{
	if ((0 <= index) && (index < GetCount()))
	{
		return m_vecParam[index].strNetDrive;
	}

	return _T("");
}

CString CNetworkConfigMgr::GetParam_NetRcpDrive(int index) const
{
	if ((0 <= index) && (index < GetCount()))
	{
		return m_vecParam[index].strNetRecipeDrive;
	}

	return _T("");
}

void CNetworkConfigMgr::SetParam_Name(int index, CString strIP)
{
	if( (0 <= index) && (index < GetCount()) )
	{
		m_vecParam[index].strName = strIP;			
	}
}

void CNetworkConfigMgr::SetParam_IPNumber(int index, CString strIP)
{
	if( (0 <= index) && (index < GetCount()) )
	{
		m_vecParam[index].strIPNumber = strIP;			
	}
}

void CNetworkConfigMgr::SetParam_PortNumber(int index, CString strPort)
{ 
	if( (0 <= index) && (index < GetCount()) )
	{
		m_vecParam[index].strPortNumber = strPort;			
	}
}

void CNetworkConfigMgr::SetParam_Mode(int index, bool mode)
{ 
	if( (0 <= index) && (index < GetCount()) )
	{
		m_vecParam[index].bConnetMode = mode;			
	}
}

