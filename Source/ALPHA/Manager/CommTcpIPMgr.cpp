//#include "stdafx.h"
#include "pch.h"
#include "CommTcpIPMgr.h"

#include "NetworkConfigMgr.h"
#include "CommTcpIpFactoryMgr.h"
#include "CommEthFunction\CommEthFunction.h"
//#include "CommTcpIpObjectPPA.h"
//#include "CommTcpIpObjectSE.h"
//#include "CommTcpIpObjectTR.h"
//#include "CommTcpIpObjectDFS.h"
#include "CommTcpIpObjectUTG.h"
#include "CommTcpIpObjectVision.h"
#include "CommTcpIpObjectColor.h"
#include "CommTcpIpObjectWAD.h"
#include "CommTcpIpObjectAngle.h"
#include "CommTcpIpObjectPG.h"
#include "CommTcpIpObjectAgingPG.h"
#include "CommTcpIpObjectLCR.h"
#include "CommTcpIpObjectTSP.h"
#include "CommTcpIpObjectCIM.h"
#include "CCommTcpIpObjectMotion.h"
#include "CCommTcpIpObjectSysTime.h"
#include "CommTcpIpObjectPGMon.h"	// LYS 2026-07-29 ELP ElecBox monitor



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/**
 *
 * @brief TCP/IP 통신연결 
 * @details 외부프로그램과 통신방식이 TCP/IP 인 경우 추가
 * @author 
 * @date 
 * @version 0.0.1
 *
 */
CCommTcpIPMgr::CCommTcpIPMgr(stAuroraMgrPool &MgrPool)
	: m_pNetworkConfigMgr(MgrPool.pNetworkConfigMgr)
{
    SetName(_T("COMM_TCPIP_MGR"));

	m_vecNetInfo.clear();

	for (int i = 0; i < GetCommTcpIPCount(); i++)
	{
		int iMeasType = GetCommTcpIP_MeasType(i);
		int iDeviceIndex = GetCommTcpIP_DeviceIndex(i);
		
		switch (iMeasType)
		{
		case enumTCPIpFactorytID_VISION:
		{
			m_vecNetInfo.push_back(dynamic_cast<CCommEthFunction*>(MgrPool.pCommTcpIpFactoryMgr->GetCommTcpIpObjectVision(iDeviceIndex)));
		}
		break;
		case enumTCPIpFactorytID_COLOR:
		{
			m_vecNetInfo.push_back(dynamic_cast<CCommEthFunction*>(MgrPool.pCommTcpIpFactoryMgr->GetCommTcpIpObjectColor(iDeviceIndex)));
		}
		break;
		case enumTCPIpFactorytID_ANGLE:
		{
			m_vecNetInfo.push_back(dynamic_cast<CCommEthFunction*>(MgrPool.pCommTcpIpFactoryMgr->GetCommTcpIpObjectAngle(iDeviceIndex)));
		}
		break;
		case enumTCPIpFactorytID_WAD:
		{
			m_vecNetInfo.push_back(dynamic_cast<CCommEthFunction*>(MgrPool.pCommTcpIpFactoryMgr->GetCommTcpIpObjectWAD()));
		}
		break;
		case enumTCPIpFactorytID_PG:
		{
			m_vecNetInfo.push_back(dynamic_cast<CCommEthFunction*>(MgrPool.pCommTcpIpFactoryMgr->GetCommTcpIpObjectPG()));
		}
		break;
		case enumTCPIpFactorytID_AGING:
		{
			m_vecNetInfo.push_back(dynamic_cast<CCommEthFunction*>(MgrPool.pCommTcpIpFactoryMgr->GetCommTcpIpObjectAGING()));
		}
		break;
		case enumTCPIpFactorytID_TSP:
		{
			// jsyoon  2022.10.19 change
			m_vecNetInfo.push_back(dynamic_cast<CCommEthFunction*>(MgrPool.pCommTcpIpFactoryMgr->GetCommTcpIpObjectTSP(iDeviceIndex)));
			//m_vecNetInfo.push_back(dynamic_cast<CCommEthFunction*>(MgrPool.pCommTcpIpFactoryMgr->GetCommTcpIpObjectTSP()));
		}
		break;
		case enumTCPIpFactorytID_LCR:
		{
			m_vecNetInfo.push_back(dynamic_cast<CCommEthFunction*>(MgrPool.pCommTcpIpFactoryMgr->GetCommTcpIpObjectLCR()));
		}
		break;
		case enumTCPIpFactorytID_CIM:
		{
			m_vecNetInfo.push_back(dynamic_cast<CCommEthFunction*>(MgrPool.pCommTcpIpFactoryMgr->GetCommTcpIpObjectCIM()));
		}
		break;
		case enumTCPIpFactorytID_TIME:
		{
			m_vecNetInfo.push_back(dynamic_cast<CCommEthFunction*>(MgrPool.pCommTcpIpFactoryMgr->GetCommTcpIpObjectSystime()));
		}
		break;
		case enumTCPIpFactorytID_MOTION:
		{
			m_vecNetInfo.push_back(dynamic_cast<CCommEthFunction*>(MgrPool.pCommTcpIpFactoryMgr->GetCommTcpIpObjectVisMotion()));
		}
		break;

		case enumTCPIpFactorytID_PG_MON: // LYS 2026-07-29 ELP ElecBox monitor
		{
			m_vecNetInfo.push_back(dynamic_cast<CCommEthFunction*>(MgrPool.pCommTcpIpFactoryMgr->GetCommTcpIpObjectPGMon()));
		}
		break;

			//case enumNETWORK_INFO_PPA1:	
			//	m_vecNetInfo.push_back(dynamic_cast<CCommEthFunction *>(MgrPool.pCommTcpIpFactoryMgr->GetCommTcpIpObjectPPA1()));
			//	break;
			//case enumNETWORK_INFO_PPA2:
			//	m_vecNetInfo.push_back(dynamic_cast<CCommEthFunction *>(MgrPool.pCommTcpIpFactoryMgr->GetCommTcpIpObjectPPA2()));
			//	break;
			//case enumNETWORK_INFO_SE:
			//	m_vecNetInfo.push_back(dynamic_cast<CCommEthFunction *>(MgrPool.pCommTcpIpFactoryMgr->GetCommTcpIpObjectSE()));	
			//	break;
			//case enumNETWORK_INFO_TR:	
			//	m_vecNetInfo.push_back(dynamic_cast<CCommEthFunction *>(MgrPool.pCommTcpIpFactoryMgr->GetCommTcpIpObjectTR()));	
			//	break;
			//case enumNETWORK_INFO_DFS:
			//	m_vecNetInfo.push_back(dynamic_cast<CCommEthFunction *>(MgrPool.pCommTcpIpFactoryMgr->GetCommTcpIpObjectDFS()));
			//	break;
			default:
				break;
		}
	}

	for (size_t i = 0; i < m_vecNetInfo.size(); ++i)
	{
		if (m_vecNetInfo[i])
		{
			if (m_pNetworkConfigMgr->GetParam_Use((int)i) && m_pNetworkConfigMgr->GetParam_OutOfService((int)i) != true)
			{
				m_vecNetInfo[i]->Initialize();
			}
		}
	}
}

CCommTcpIPMgr::~CCommTcpIPMgr(void)
{
	Finalize();
}

void CCommTcpIPMgr::Finalize()
{

}

bool CCommTcpIPMgr::ReConnect()
{
	for (size_t i = 0; i < m_vecNetInfo.size(); ++i)
	{
		if (m_vecNetInfo[i])
		{
			m_vecNetInfo[i]->Initialize();
		}
	}

    return true;
}

void CCommTcpIPMgr::Disconnect()
{
	for (size_t i = 0; i < m_vecNetInfo.size(); ++i)
	{
		if (m_vecNetInfo[i])
		{
			m_vecNetInfo[i]->Finalize();
		}
	}
}

bool CCommTcpIPMgr::ReConnect(int index)
{
	if ((0 <= index) && (index < GetCount()))
	{
		return m_vecNetInfo[index]->Initialize();
	}

	return false;
}

void CCommTcpIPMgr::Disconnect(int index)
{
	if ((0 <= index) && (index < GetCount()))
	{
		m_vecNetInfo[index]->Finalize();
	}
}


int CCommTcpIPMgr::GetCount() const 
{
	return (int)m_vecNetInfo.size();
}

bool CCommTcpIPMgr::IsConnected(CString strName)
{
	for (size_t i = 0; i < m_vecNetInfo.size(); ++i)
	{
		if (m_vecNetInfo[i])
		{
			if(m_vecNetInfo[i]->GetName() == strName)
			{
				return m_vecNetInfo[i]->IsConnected();
			}
		}
	}

	return false;
}

bool CCommTcpIPMgr::IsOutOfSerice(CString strName)
{
	for (size_t i = 0; i < m_vecNetInfo.size(); ++i)
	{
		if (m_vecNetInfo[i])
		{
			if (m_vecNetInfo[i]->GetName() == strName)
			{
				return m_vecNetInfo[i]->IsOutOfService();
			}
		}
	}

	return false;
}


void CCommTcpIPMgr::UpdateNetInfo()
{
	for(int n = 0; n < m_pNetworkConfigMgr->GetCount(); ++n)
	{
		for (size_t i = 0; i < m_vecNetInfo.size(); ++i)
		{
			if(m_vecNetInfo[i])
			{
				if(m_vecNetInfo[i]->GetName() == m_pNetworkConfigMgr->GetParam_Name(n))
				{
					m_vecNetInfo[i]->SetNetInfo(m_pNetworkConfigMgr->GetParam_Mode(n), m_pNetworkConfigMgr->GetParam_IPNumber(n), m_pNetworkConfigMgr->GetParam_PortNumger(n));
				}
			}
		}
	}
}

int CCommTcpIPMgr::GetCommTcpIPCount() const
{
	return m_pNetworkConfigMgr->GetCount();
}

CString CCommTcpIPMgr::GetCommTcpIP_Name(int index)
{
	if ((0 <= index) && (index < GetCommTcpIPCount()))
	{
		return m_pNetworkConfigMgr->GetParam_Name(index);
	}

	return _T("");
}
	
CCommEthFunction *CCommTcpIPMgr::GetCommTcpIP(int index)
{
	if ((0 <= index) && (index < GetCount()))
	{
		return m_vecNetInfo[index];
	}

	return nullptr;
}

long CCommTcpIPMgr::GetCommTcpIP_MeasType(int index) const
{
	if ((0 <= index) && (index < GetCommTcpIPCount()))
	{
		return m_pNetworkConfigMgr->GetParam_MeasType(index);
	}

	return -1;
}

long CCommTcpIPMgr::GetCommTcpIP_DeviceIndex(int index) const
{
	if ((0 <= index) && (index < GetCommTcpIPCount()))
	{
		return m_pNetworkConfigMgr->GetParam_DeviceIndex(index);
	}

	return -1;
}

