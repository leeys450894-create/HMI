#pragma once
#include "CommEthFunction\CommEthFunction.h"
#include ".\Manager\CommTcpIpDefine.h"
#include "AuroraMgrPool.h"

class CRecipeData;
class CLogFileAccessor;
class CCommTcpIpObjectTSP :
    public CCommEthFunction
{
	CLogFileAccessor* m_pLogFileAccessor;
	CSystemConfigMgr* m_pSystemConfigMgr;
	CRecipeEditMgr* m_pRecipeEditMgr;
	CResultCollectMgr* m_pResultCollectMgr;
	CRecipeData* m_pRecipeDataClone;
	CShareMemoryMgr* m_pShareMemory;
	CString					m_strQueueMessage;
	HANDLE					m_hRecvDEvent[enumRecvEventIDCount];
	bool					m_bOutOfService;
	int						m_iDeviceIndex;		// jsyoon 2022.10.19 add

	int m_iThread;
	static UINT ThreadRun(LPVOID pParam);
	void Run_Thread();

public:
	void UpdateRecipeInformation();
	bool SetTime_Sync();
	bool RecipeChange();
	bool RecipeChangeDone(DWORD dwTimeOut);
	bool SetParameter();
	bool InspectStart(int iMeasType = 0, bool bWait = true, int iStepNum = 0);
	bool InspectStartWait();
	bool InspectEnd();
	bool WaitInspectionEnd(DWORD dwTimeOut);
	bool WaitInspectionComplete(DWORD dwTimeOut);
	CLogFileAccessor* GetLogFileAccessorPtr();
	bool IsOutOfService() { return m_bOutOfService; };
	void SetOutOfService(bool bOn) { m_bOutOfService = bOn; };
	void InspectStartReq(int iMeasType = 0);


	//=========================================================================
	// INTERFACE
	//=========================================================================
private:
	virtual void xNetCallbackRecieveData(LPVOID ud) override;
	int	 SetCommand(ELPPROTOCOL stPg, bool bWait = true);
	int  _ExeMeas(int iFunc, bool bWait = true, DWORD dwTimeOut = 1000);
	int WaitCommandRecive(DWORD dwTimeOut);

public:
	CCommTcpIpObjectTSP(int iSeqItemIndex, stAuroraMgrPool& MgrPool);
	virtual ~CCommTcpIpObjectTSP();
	int m_iSeqItemIndex;
};

