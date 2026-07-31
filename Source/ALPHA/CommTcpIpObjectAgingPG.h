#pragma once
#include "CommEthFunction\CommEthFunction.h"
#include ".\Manager\CommTcpIpDefine.h"
#include "AuroraMgrPool.h"


#define rSTART _T("START")	
#define rSTATU _T("STATU")
#define	rJIDGE _T("JUDGE")
#define	rJOBEN _T("JOBEN")
#define	rRDATA _T("RDATA")
#define	rSETID _T("SETID")
#define rSTOP _T("STOP~")
#define rCHANG _T("CHANG")

#define	rSTR1 _T("$9")
#define	rSTR2 _T("@9")

typedef struct {
	int			iCmd;
	int			iCellNum;
	int			iOption;			// 
	int			iError;
	int			iResult;		// OST 0-Normal, 1-Short, 2-Open	// PTP 0-Normal, 1-Short	// Status 0-Idle, 1-Test Running
	int			iDataLen;		// Data Length
	int			iSeqNum;		// Aging ½ÃÄö½º ¹øÈ£ ( Step Index)
	double		dResult;
	TCHAR		szModel[100];
	TCHAR		szRecipe[100];
}AGINGPGPROTOCOL, * PAGINGPGPROTOCOL;

typedef struct {
	bool bRecive;
	bool bJudge[10];
	double dCurentData[10];
	double dCurentDataMin;
	double dCurentDataMax;

}AGINGPGRESULT, * PAGINGPGRESULT;



class CRecipeData;
class CLogFileAccessor;
class CShareMemoryMgr;
class CCommTcpIpObjectAgingPG :
    public CCommEthFunction
{
	CLogFileAccessor* m_pLogFileAccessor;
	CSystemConfigMgr* m_pSystemConfigMgr;
	CRecipeEditMgr* m_pRecipeEditMgr;
	CResultCollectMgr* m_pResultCollectMgr;
	CShareMemoryMgr* m_pShareMemoryMgr;
	CRecipeData* m_pRecipeDataClone;
	CString					m_strQueueMessage;
	HANDLE					m_hRecvDEvent[enumRecvEventIDCount];
	bool					m_bOutOfService;
	CString					m_strPGResult1;
	CString					m_strPGResult2;
	CString					m_strPGResult3;
	CString					m_strPGResult4;


public:
	void UpdateRecipeInformation();
	bool SetTime_Sync();
	bool RecipeChange();
	bool RecipeChangeDone(DWORD dwTimeOut);
	bool SetParameter();
	bool InspectStart(int iMeasType = 0, int iStepNum = 0, bool bWait = true);
	bool InspectStartWait();
	bool InspectEnd();
	bool WaitInspectionEnd(int iPG, DWORD dwTimeOut);
	bool WaitInspectionComplete(int iPG, DWORD dwTimeOut);
	CLogFileAccessor* GetLogFileAccessorPtr();
	bool IsOutOfService() { return m_bOutOfService; };
	void SetOutOfService(bool bOn) { m_bOutOfService = bOn; };
	bool SetUserStop(int iCellNum = 0);
	bool m_bAginging;
	bool m_bAgingingEven;
	bool m_bUseEnsisPG;

	//=========================================================================
	// INTERFACE
	//=========================================================================
private:
	virtual void xNetCallbackRecieveData(LPVOID ud) override;
	int	 SetCommand(AGINGPGPROTOCOL stPg, bool bWait = true);
	int	 _ExePGCtrl(int iEvent, int iCellNum = 0, UINT nTimeOut = DEFINE_RESPONSE_WAIT_TIME);
	int WaitCommandRecive(DWORD dwTimeOut, int iPG = 0) ;

public:
	CCommTcpIpObjectAgingPG(int iSeqItemIndex, stAuroraMgrPool& MgrPool);
	virtual ~CCommTcpIpObjectAgingPG();
	int m_iSeqItemIndex;
	AGINGPGRESULT m_stAgingResult[10][30]; // PG Num(10), Seqence Num(30)
	int SaveAgingResult(int nPgNum);
};

