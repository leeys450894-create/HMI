#pragma once
#include "CommEthFunction\CommEthFunction.h"
#include ".\Manager\CommTcpIpDefine.h"
#include "AuroraMgrPool.h"

class CRecipeData;
class CLogFileAccessor;
class CCommTcpIpObjectLCR :
    public CCommEthFunction
{
	CLogFileAccessor* m_pLogFileAccessor;
	CSystemConfigMgr* m_pSystemConfigMgr;
	CRecipeEditMgr* m_pRecipeEditMgr;
	CResultCollectMgr* m_pResultCollectMgr;
	CShareMemoryMgr* m_pShareMemory;
	CRecipeData* m_pRecipeDataClone;
	CString					m_strQueueMessage;
	HANDLE					m_hRecvDEvent[enumRecvEventIDCount];
	bool					m_bOutOfService;


public:
	void UpdateRecipeInformation();
	bool SetTime_Sync();
	bool RecipeChange();
	bool SetParameter();
	bool InspectStart(int iMeasType = 0, bool bWait = true, int iStepNum = 0);
	bool InspectStartWait();
	bool InspectEnd();
	bool WaitInspectionEnd(DWORD dwTimeOut);
	bool WaitInspectionComplete(DWORD dwTimeOut);
	CLogFileAccessor* GetLogFileAccessorPtr();
	bool IsOutOfService() { return m_bOutOfService; };
	void SetOutOfService(bool bOn) { m_bOutOfService = bOn; };


	//=========================================================================
	// INTERFACE
	//=========================================================================
private:
	virtual void xNetCallbackRecieveData(LPVOID ud) override;
	int	 SetCommand(ELPPROTOCOL stPg, bool bWait = true);
	int  _ExeMeas(int iFunc, bool bWait = true);

public:
	CCommTcpIpObjectLCR(int iSeqItemIndex, stAuroraMgrPool& MgrPool);
	virtual ~CCommTcpIpObjectLCR();
	int m_iSeqItemIndex;
};

