#pragma once

#include "CommEthFunction\CommEthFunction.h"
#include "CommTcpIpDefine.h"
#include "..\AuroraMgrPool.h"

class CRecipeData;
class CLogFileAccessor;
class CCommTcpIpObjectUTG : public CCommEthFunction
{
	CLogFileAccessor			*m_pLogFileAccessor;
	CSystemConfigMgr		*m_pSystemConfigMgr;
	CRecipeEditMgr			*m_pRecipeEditMgr;
	CResultCollectMgr		*m_pResultCollectMgr;
	CRecipeData				*m_pRecipeDataClone;
	CString					m_strQueueMessage;
	HANDLE					m_hRecvDEvent[enumRecvEventIDCount];
	bool					m_bOutOfService;

public:
	void UpdateRecipeInformation();
	bool SetTime_Sync();
	bool RecipeChange();
	bool SetParameter();
	bool InspectStart(int iMeasType = 0, bool bWait = true);
	bool InspectStartWait();
	bool InspectEnd();
	bool WaitInspectionComplete(DWORD dwTimeOut);
	CLogFileAccessor *GetLogFileAccessorPtr();
	bool IsOutOfService() { return m_bOutOfService; };
	void SetOutOfService(bool bOn) {  m_bOutOfService = bOn; };


	//=========================================================================
	// INTERFACE
	//=========================================================================
private:
	virtual void xNetCallbackRecieveData(LPVOID ud) override;

public:
	CCommTcpIpObjectUTG(int iSeqItemIndex, stAuroraMgrPool &MgrPool);
	virtual ~CCommTcpIpObjectUTG();
};

