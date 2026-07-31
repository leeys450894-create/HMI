#pragma once

#include "..\AuroraSubMgr.h"
#include "..\AuroraMgrPool.h"

class CCommEthFunction;
class CCommTcpIPMgr : public CAuroraSubMgr
{
	CNetworkConfigMgr			*m_pNetworkConfigMgr;
	std::vector<CCommEthFunction *>	m_vecNetInfo;

public:
	bool	ReConnect();
	void	Disconnect();
	bool	ReConnect(int index);
	void	Disconnect(int index);
	int		GetCount() const;
	bool	IsConnected(CString strName);
	bool	IsOutOfSerice(CString strName);

public:
	void	UpdateNetInfo();
	int		GetCommTcpIPCount() const;
	long    GetCommTcpIP_MeasType(int index) const;
	long    GetCommTcpIP_DeviceIndex(int index) const;
	CString GetCommTcpIP_Name(int index);
	CCommEthFunction *GetCommTcpIP(int index);
	



//------------------------------------------------------------------
// 공통 인터페이스 함수
//------------------------------------------------------------------
private:
	virtual void Finalize() override;
public:
    CCommTcpIPMgr(stAuroraMgrPool &MgrPool);
    virtual ~CCommTcpIPMgr(void);
};

