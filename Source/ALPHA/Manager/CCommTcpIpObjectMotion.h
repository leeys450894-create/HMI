#pragma once
#include "CommEthFunction\CommEthFunction.h"
#include ".\Manager\CommTcpIpDefine.h"
#include "AuroraMgrPool.h"
#include "common/TimeChecker.h"
#include "Manager\NetworkConfigMgr.h"





class CLogFileAccessor;
class CShareMemoryMgr;
class CCommTcpIpObjectMotion :public CCommEthFunction
{
	stAuroraMgrPool* m_MgrPool_link;
	CLogFileAccessor* m_pLogFileAccessor;
	CSystemConfigMgr* m_pSystemConfigMgr;
	CNetworkConfigMgr* m_pNetworkConfigMgr;
	CShareMemoryMgr* m_pShareMemory;

	bool m_bOutOfService;

	HANDLE					m_hRecvDEvent[enumRecvEventIDCount];

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
	CCommTcpIpObjectMotion(int iSeqItemIndex, stAuroraMgrPool& MgrPool);
	virtual ~CCommTcpIpObjectMotion();

	int SetCommand(VisMotionPROTOCOL stPrl, bool bWait = true);
	int WaitInspectionEnd(int iActiveID, DWORD dwTimeOut);
	int WaitInspectionComplete(int iActiveID, DWORD dwTimeOut);

};