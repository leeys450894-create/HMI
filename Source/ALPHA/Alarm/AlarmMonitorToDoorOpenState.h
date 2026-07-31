#pragma once

class CSystemConfigMgr;
class CIOMapControlMgr;
class CCommTcpIPMgr;
class CLogFileAccessor;
class CAlarmMonitorToDoorOpenState
{
	CLogFileAccessor *m_pLogFileAccessor;
	int		m_iMonitorIONumber;
	int		m_iEMOPosST;
	int		m_iEMOPosED;
	CString m_strLogMsg;
	int		m_iAlarmID;
private:
	CWinThread	*m_pThread;
	bool		m_bThreadLive;
	HANDLE		m_hStartEvent;
	HANDLE		m_hCloseEvent;
	static UINT ThreadRun(LPVOID p1);
	UINT ThreadRun();


protected:
	CSystemConfigMgr	*m_pSystemConfigMgr;
	CIOMapControlMgr	*m_pIOMapControlMgr;
	CCommTcpIPMgr		*m_pCommTcpIPMgr;

public:
	void SetPara(CSystemConfigMgr *pSystemConfigMgr, CIOMapControlMgr *pIOMapControlMgr, CCommTcpIPMgr *pCommTcpIPMgr);
	void SetStart();
	void SetStop();

public:
	CAlarmMonitorToDoorOpenState(CString strLogPath, int iMonitorIONumber, int iEMOPosST, int iEMOPosED, CString strLogMsg, int iAlarmID);
	~CAlarmMonitorToDoorOpenState();
};

