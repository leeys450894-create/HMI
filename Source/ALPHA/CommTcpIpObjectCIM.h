#pragma once
#include "CommEthFunction\CommEthFunction.h"
#include ".\Manager\CommTcpIpDefine.h"
#include "AuroraMgrPool.h"
#include "..\\include\\DefinesCIM.h"
#include "common/TimeChecker.h"

class CLogFileAccessor;
class CShareMemoryMgr;
class CMaintControlMgr;
class CTransporterResult;
class CCommTcpIpFactoryMgr;
class CCommTcpIpObjectCIM :public CCommEthFunction
{
	stAuroraMgrPool*  m_MgrPool_link;
	CLogFileAccessor* m_pLogFileAccessor;
	CSystemConfigMgr* m_pSystemConfigMgr;
	CShareMemoryMgr* m_pShareMemory;
	CMaintControlMgr* m_pMaintControlMgr;
	CCommTcpIpFactoryMgr* m_pCommTcpIpFactoryMgr;

	CString					m_strQueueMessage;
	CString					m_strRcvPPID;
	CString					m_strRcvLotID;
	CString					m_strRcvGlsID;
	HANDLE					m_hRecvDEvent[CIM_CMD_SERVER_TO_CIM_END];
	bool					m_bOutOfService;
	int						m_nUserSelectMode;
	volatile bool			m_bRecvEFA0;			// LYS 2026-09-28 EFA: EFA^0_OK$ 수신 시 true

	// FTP 
	CWinThread* m_pThread;
	bool		m_bThreadLive;
	HANDLE		m_hStartEvent;
	HANDLE		m_hCloseEvent;
	static UINT ThreadRun(LPVOID p1);
	UINT ThreadRun();

	CTransporterResult* m_pTransporterResult;

public:
	CLogFileAccessor* GetLogFileAccessorPtr();
	bool IsOutOfService() { return m_bOutOfService; };
	void SetOutOfService(bool bOn) { m_bOutOfService = bOn; };
	CSeqTactTimeMgr* GetSeqTactTimeMgrPtr() { return m_MgrPool_link->pSeqTactTimeMgr; }
	CRecipeEditMgr* GetRecipeEditMgrPtr() { return m_MgrPool_link->pRecipeEditMgr; }

	//=========================================================================
	// INTERFACE
	//=========================================================================
private:
	virtual void xNetCallbackRecieveData(LPVOID ud) override;

public:
	CCommTcpIpObjectCIM(int iSeqItemIndex, stAuroraMgrPool& MgrPool);
	virtual ~CCommTcpIpObjectCIM();

	int Cmd_Send(CString strSend);
	int ProcessComponent();
	int ProcessHandShake(CString strMsg);
	int ProcessHandOff();
	int ProcessHandOn();
	int ProcessTIME(CString strMsg);
	int Process_EQPStart();
	int Process_SetMCCTime(CString strMsg);
	int Process_SetMCCTransfer(bool bOn);
	int Process_SetHostMode(int iMode = -1);
	bool m_bUserBypassGlass;
	int Process_LOTID(CString strMsg);
	int Process_GLSID(CString strMsg);
	int Process_PPID(CString strMsg);
	int Process_Start();
	int ProcessTo_Init();
	int ProcessTo_GDTCPL();
	int ProcessTo_ULDCPL();
	int ProcessTo_ULDREQ();
	int ProcessTo_LDREQ();
	int ProcessTo_LDCPL();
	int ProcessTo_PPIDREQ();
	int ProcessTo_GLSREADY();
	int ProcessTo_MeasStart();
	int ProcessTo_MeasEnd();
	bool SendRecipeListToFTP();
	bool SendAPDFileToFTP();
	int m_iTransporterType;
	CString m_strOldGlsID;
	void GlassCount();
	int SendMainStatus(bool bRemote);
	bool IsValidReply(int nCmd);
	int WaitForCIMResponse_Action(CString strCmd, UINT nAction, UINT nTimeOut = 10);

	int Cmd_Send_AT0();

	// LYS 2026-09-28 EFA: EFA 2-pass 핸드셰이크 (OCP_EFA CDlgNaviMaintCIM::Cmd_Send_EFA0/EFA1/isEFA0RecvCheck)
	int  Cmd_Send_EFA0();
	int  Cmd_Send_EFA1();
	bool WaitEFA0Ack(DWORD dwTimeOutMs = 5000);	// true = EFA^0_OK$ 수신
	int Cmd_Send_STS0104();
	int Cmd_Send_STS0204();
	int Cmd_Send_STS0304();
	int Cmd_Send_STS0004();
	int Cmd_Send_STS0506();

	int Cmd_Send_STS1005();
	int Cmd_Send_STS1006();

	int Cmd_Send_STS1106();

	int Cmd_Send_STS2105();
	int Cmd_Send_STS2106();
	int Cmd_Send_STS2107();
	
	int Cmd_Send_STS2004();

	int Cmd_Send_STS0006();

	int Cmd_Send_GDTREQ();
	int Cmd_Send_GDTCPL(bool bOk);

	int Cmd_Send_LDREQ(bool bOk);
	int Cmd_Send_LDCPL(bool bOk);
	int Cmd_Send_ULDREQ(bool bOk);
	int Cmd_Send_ULDCPL(bool bOk);

	int Cmd_Send_PRCS(bool bOk);
	int Cmd_Send_PPID(bool bOk);

	int Cmd_Send_DataJudge();
	
	int Cmd_Send_PPIDREQ();
	int Cmd_Send_LOTIDREQ();
	int Cmd_Send_GLSIDREQ();

	int	SendAlarmCode(int nCode);
	int	SendAlarmClear(int nCode);
	int	SendWarningCode(int nCode);
	int	SendWarningClear(int nCode);


	void SetUserBypassGlass(bool bPass);
	
	int WaitForORG();
	int CanDoStartPRCS(bool bCheckLoad = true, bool bCheckMeas = true);

	int ProcessPPID();
	int ProcessLOTID();
	int ProcessGLSID();

	void SetUserMode(int iMode) { m_nUserSelectMode = iMode;  }
	int  GetUserMode() {return m_nUserSelectMode;}
	bool Send_Data();
	bool GetThreadLive() { return m_bThreadLive; }
	bool m_bProcessHandOn;
	int	m_nHandShakeYBit[6];
	int	m_nHandShakeXBit[6];
	CTimeChecker tickGlassMeas;
	CString m_strGlassModelName;
};


