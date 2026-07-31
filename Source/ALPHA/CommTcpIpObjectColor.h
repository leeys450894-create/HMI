#pragma once
#include "CommEthFunction\CommEthFunction.h"
#include ".\Manager\CommTcpIpDefine.h"
#include "AuroraMgrPool.h"

class CRecipeData;
class CLogFileAccessor;
class CShareMemoryMgr;
class CCommTcpIpFactoryMgr;
class CCommTcpIpObjectColor :
    public CCommEthFunction
{
	CLogFileAccessor* m_pLogFileAccessor;
	CSystemConfigMgr* m_pSystemConfigMgr;
	CRecipeEditMgr* m_pRecipeEditMgr;
	CResultCollectMgr* m_pResultCollectMgr;
	CRecipeData* m_pRecipeDataClone;
	CShareMemoryMgr* m_pShareMemory;
	CCommTcpIpFactoryMgr* m_pCommTcpIpFactoryMgr;
	CString					m_strQueueMessage;
	HANDLE					m_hRecvDEvent[enumRecvEventIDCount];
	bool					m_bOutOfService;
	CString					m_strPatternName;
	bool					m_nPatternOn;
	int						m_nPatternNum;
	int						m_RevCh;
	int						m_RevError;
	CString					m_strNetDrive;
	CString					m_strNetRcpDrive;

	CCommTcpIpFactoryMgr* GetCommTcpIpFactoryMgrPtr() { return m_pCommTcpIpFactoryMgr; }

public:
	void UpdateRecipeInformation();
	bool SetTime_Sync();
	bool RecipeChange();
	bool SetParameter();
	bool InspectStart(int iMeasType = 0, bool bWait = true, int iStepNum = 0);
	bool InspectStartWait();
	bool InspectEnd();
	bool WaitRecipeChangeEnd(DWORD dwTimeOut);
	bool WaitInspectionEnd(DWORD dwTimeOut);
	bool WaitInspectionComplete(DWORD dwTimeOut);
	CLogFileAccessor* GetLogFileAccessorPtr();
	bool IsOutOfService() { return m_bOutOfService; };
	void SetOutOfService(bool bOn) { m_bOutOfService = bOn; };
	BOOL TossResultFile(CString strInnerID, int nArea);

	//=========================================================================
	// INTERFACE
	//=========================================================================
private:
	virtual void xNetCallbackRecieveData(LPVOID ud) override;
	int	 SetCommand(int iCmd, bool bWait = false);

public:
	CCommTcpIpObjectColor(int iSeqItemIndex, stAuroraMgrPool& MgrPool);
	virtual ~CCommTcpIpObjectColor();
	int m_iSeqItemIndex;
	int m_iDeviceIndex;
	int WriteRecipeFile();
	int CopyRecipeFile();
	int SetRunToThread(int iPattern);
	int m_iPattern;
	static UINT ThreadRun(LPVOID pParam);
	void RunTurnOnToPG();
	void RunTurnOffToPG();
};

