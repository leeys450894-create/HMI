#pragma once
#include "CommEthFunction\CommEthFunction.h"
#include ".\Manager\CommTcpIpDefine.h"
#include "AuroraMgrPool.h"


#define eS1F2 _T("0102")	
#define eS1F6 _T("0106")
#define	eS3F2 _T("0302")
#define	eS3F3 _T("0303")
#define	eS3F6 _T("0306")
#define	eS3F7 _T("0307")
#define	eS3F12 _T("0312")
#define	eS3F14 _T("0314")
#define	eS3F16 _T("0316")
#define	eS3F18 _T("0318")
#define	eS3F20 _T("0320")
#define	eS5F2  _T("0502")
#define eS2F2  _T("0202")
#define eS2F3  _T("0203")
#define eS2F10	_T("0210")
#define eS2F12	_T("0212")
#define eS6F2	_T("0602")
#define eS6F3	_T("0603")
#define eS8F2	_T("0802")
#define eS8F3	_T("0803")
#define eS10F2	_T("1002")
#define eS10F4	_T("1004")

typedef struct {
	int			iStream;
	int			iFunc;
	int			iCmd;
	int			iCellNum;
	int			iOption;
	int			iAck;		// 0-ACX / 1-NAK
	int			iError;
	int			iResult;	// OST 0-Normal, 1-Short, 2-Open	// PTP 0-Normal, 1-Short	// Status 0-Idle, 1-Test Running
	double		dResult;
	TCHAR		szModel[100];
	TCHAR		szRecipe[100];
}PGPROTOCOL, * PPGPROTOCOL;

class CRecipeData;
class CLogFileAccessor;
class CShareMemoryMgr;
class CCommTcpIpObjectPG :
    public CCommEthFunction
{
	CLogFileAccessor* m_pLogFileAccessor;
	CSystemConfigMgr* m_pSystemConfigMgr;
	CRecipeEditMgr* m_pRecipeEditMgr;
	CResultCollectMgr* m_pResultCollectMgr;
	CRecipeData* m_pRecipeDataClone;
	CShareMemoryMgr* m_pShareMemoryMgr;
	CString					m_strQueueMessage;
	HANDLE					m_hRecvDEvent[enumRecvEventIDCount];
	bool					m_bOutOfService;
	int						m_iSeqItemIndex;
	CString					m_strPGResult;
	int						m_nAgingTime;
	bool					m_bAginging;
	bool					m_bAgingingEven;
	bool 					m_bUseEnsisPG;

public:
	void UpdateRecipeInformation();
	bool SetTime_Sync();
	bool RecipeChange();
	bool SetParameter();
	bool InspectStart(int iMeasType = 0, int iCellNum = 0, bool bWait = true);
	bool InspectStartWait();
	bool InspectEnd();
	bool WaitInspectionEnd(DWORD dwTimeOut);
	bool WaitInspectionComplete(DWORD dwTimeOut, int iMeasType = 0);
	CLogFileAccessor* GetLogFileAccessorPtr();
	bool IsOutOfService() { return m_bOutOfService; };
	void SetOutOfService(bool bOn) { m_bOutOfService = bOn; };
	CString GetResult() { return m_strPGResult; };
	bool SetUserStop(int iCellNum);
	int  GetAgingTime() { return m_nAgingTime; };

	//=========================================================================
	// INTERFACE
	//=========================================================================
private:
	virtual void xNetCallbackRecieveData(LPVOID ud) override;
	int	 SetCommand(PGPROTOCOL stPg, bool bWait = true);
	int	 _ExePGCtrl(int iEvent, int iCellNum = 0);
	int WaitCommandRecive(DWORD dwTimeOut);
	CCriticalSection	m_cs_Set;

public:
	CCommTcpIpObjectPG(int iSeqItemIndex, stAuroraMgrPool& MgrPool);
	virtual ~CCommTcpIpObjectPG();
	
};

