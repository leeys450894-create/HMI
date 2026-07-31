#pragma once

#include "CommEthFunction\CommEthFunction.h"
#include ".\Manager\CommTcpIpDefine.h"
#include "AuroraMgrPool.h"
#include "common/TimeChecker.h"
#include "Manager\NetworkConfigMgr.h"

class CLogFileAccessor;
class CShareMemoryMgr;
class CCommTcpIpObjectSysTime :public CCommEthFunction
{
	stAuroraMgrPool* m_MgrPool_link;
	CLogFileAccessor* m_pLogFileAccessor;
	CSystemConfigMgr* m_pSystemConfigMgr;
	CNetworkConfigMgr* m_pNetworkConfigMgr;
	CShareMemoryMgr* m_pShareMemory;

	bool m_bOutOfService;

public:
	CLogFileAccessor* GetLogFileAccessorPtr();
	bool IsOutOfService() { return m_bOutOfService; };
	void SetOutOfService(bool bOn) { m_bOutOfService = bOn; };
	int Cmd_Send(CString strSend);

	//=========================================================================
	// INTERFACE
	//=========================================================================
private:
	virtual void xNetCallbackRecieveData(LPVOID ud) override;

public:
	CCommTcpIpObjectSysTime(int iSeqItemIndex, stAuroraMgrPool& MgrPool);
	virtual ~CCommTcpIpObjectSysTime();

	int Cmd_Send_SysTime(CString strTime);	
};

