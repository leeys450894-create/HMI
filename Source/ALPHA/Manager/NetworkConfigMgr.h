#pragma once

#include "..\AuroraSubMgr.h"

enum enumNETWORK_INFO
{
	enumNETWORK_INFO_PPA1 = 0,
	enumNETWORK_INFO_PPA2,
	enumNETWORK_INFO_SE,
	enumNETWORK_INFO_TR,
	enumNETWORK_INFO_DFS,
	enumNETWORK_INFO_COUNT,
};

struct SNetInfo
{
	bool	bUse;				// default : false
	bool    bOutOfService;		// default : false
	CString	strName;			// default : 
	CString	strIPNumber;		// default : 192.168.0.100
	CString	strPortNumber;		// default : 6000
	bool	bConnetMode;		// default : false [ Server : 1, Client : 0 ]
	int		iDeviceIndex;		// default : -1
	int     iMeasType;			// default : 0
	CString strNetDrive;
	CString strNetRecipeDrive;

	SNetInfo()
	{
		bUse			= false;
		bOutOfService	= false;
		strName			= _T("");
		strIPNumber		= _T("");
		strPortNumber	= _T("");
		bConnetMode		= false;
		iDeviceIndex	= -1;
		iMeasType = 0;
		strNetDrive		= _T("");
	}
};

class CIniHelper;
class CNetworkConfigMgr : public CAuroraSubMgr
{
    CIniHelper			*m_pIniHelper;	

	std::vector<SNetInfo>	m_vecParam;
	float				m_fResponceTimeLimit_sec;	// 30초 기본

public:
	int		GetCount() const;
	bool	GetParam_Use(int index) const;
	bool	GetParam_OutOfService(int index) const;
	CString GetParam_Name(int index) const;
	CString GetParam_IPNumber(int index) const;
	CString GetParam_PortNumber(int index) const;
	long	GetParam_PortNumger(int index);
    bool	GetParam_Mode(int index) const;
	long	GetParam_DeviceIndex(int index) const;
	long	GetParam_MeasType(int index) const;
	CString GetParam_NetDrive(int index) const;
	CString GetParam_NetRcpDrive(int index) const;

	void	SetParam_Name(int index, CString strIP);
	void	SetParam_IPNumber(int index, CString strIP);
	void	SetParam_PortNumber(int index, CString strPort);
    void	SetParam_Mode(int index, bool mode);

	float	GetCommTcpIpResponceTimeLimit() const
	{ 
		return m_fResponceTimeLimit_sec;	
	}	// 30초 기본



//------------------------------------------------------------------
// 공통 인터페이스 함수
//------------------------------------------------------------------
public:
	virtual bool LinkIniFile(bool bSave) override;
private:
	virtual void Finalize() override;
public:
    CNetworkConfigMgr(CString strConfigPath);
    virtual ~CNetworkConfigMgr();
};