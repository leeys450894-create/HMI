#pragma once
#include "CommEthFunction\CommEthFunction.h"
#include ".\Manager\CommTcpIpDefine.h"
#include "AuroraMgrPool.h"


//// Property
//typedef struct
//{
//	int               cmd;
//	int               len;
//} req_head;
//
//typedef struct
//{
//	int		iCmd;
//	int		iError;
//	int		iCause;
//	int		iLen;
//} response_head;
//
//typedef struct {
//	req_head	hrd;
//	TCHAR		szModel[32];
//
//	int       Select;					// PG 0x31 : Left    0x32 : Right     0x33 : Left/Right
//	int       UseAverageMTPLeft;
//	int       UseAverageMTPRight;
//
//	TCHAR       InnerID[2][32];
//	TCHAR       MCRID[2][32];
//
//	//// Ack
//	//int		iError;
//	//int		iCause;
//	//int		iResult;
//
//}ELPPROTOCOL;

class CRecipeData;
class CLogFileAccessor;
class CShareMemoryMgr;
class CWadMultiMeasMgr;		// LYS 2026-07-24 WAD Multi Point (Phase 5)
class CCommTcpIpObjectWAD :
    public CCommEthFunction
{
	CLogFileAccessor* m_pLogFileAccessor;
	CSystemConfigMgr* m_pSystemConfigMgr;
	CRecipeEditMgr* m_pRecipeEditMgr;
	CResultCollectMgr* m_pResultCollectMgr;
	CRecipeData* m_pRecipeDataClone;
	CShareMemoryMgr* m_pShareMemory;
	CCommTcpIpFactoryMgr* m_pCommTcpIpFactoryMgr;
	CPositionConfigMgr* m_pPositionConfigMgr;
	CWadMultiMeasMgr* m_pWadMultiMeasMgr;		// LYS 2026-07-24 WAD Multi Point (Phase 5)
	CString					m_strQueueMessage;
	HANDLE					m_hRecvDEvent[enumRecvEventIDCount];
	bool					m_bOutOfService;
	int						m_iAlarmIndex;
	CString					m_strNetDrive;
	double					m_dMeasPosX;
	double					m_dMeasPosY[2];


public:
	void UpdateRecipeInformation();
	bool SetTime_Sync();
	bool RecipeChange();
	bool SetParameter();
	//bool InspectStart(int iMeasType = 0, bool bWait = true, int iStepNum = 0);		// 측정시작
	bool InspectStart(int iMeasType = 0, bool bWait = true, int iStepNum = 0, bool bWadRHeadUse = false);		// 측정시작
	bool InspectStartWait();	// 측정시작 응답
	bool InspectEnd(DWORD dwTimeOut = DEFINE_INSPECITON_WAIT_TIME);	// 측정완료
	//bool WaitInspectionEnd(DWORD dwTimeOut = DEFINE_INSPECITON_WAIT_TIME);
	bool WaitInspectionEnd(DWORD dwTimeOut = DEFINE_INSPECITON_WAIT_TIME, int nRetry = 1);
	bool WaitInspectionComplete(DWORD dwTimeOut = DEFINE_INSPECITON_WAIT_TIME, int nRetry = 1); // 계산완료
	CLogFileAccessor* GetLogFileAccessorPtr();
	bool IsOutOfService() { return m_bOutOfService; };
	void SetOutOfService(bool bOn) { m_bOutOfService = bOn; };
	bool TossResultFile(CString strInnerID, int nArea, int iProbe, int nRetry = 1);
	void SetMeasPos(double dx, double dy1, double dy2);
	//=========================================================================
	// INTERFACE
	//=========================================================================
private:
	virtual void xNetCallbackRecieveData(LPVOID ud) override;
	//int	 SetCommand(ELPPROTOCOL stPg);
	int	 SetCommand(ELPPROTOCOL stPg, bool bWadRHeadUse = false);
	//int  _ExeMeasWAD(int iFunc, bool bWait = true);
	int  _ExeMeasWAD(int iFunc, bool bWait = true, bool bWadRHeadUse = false);
	
public:
	CCommTcpIpObjectWAD(int iSeqItemIndex, stAuroraMgrPool& MgrPool);
	virtual ~CCommTcpIpObjectWAD();

};

