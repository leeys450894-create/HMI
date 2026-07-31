#pragma once

class CSystemConfigMgr;
class CIOMapControlMgr;
class CCommTcpIPMgr;
class CLogFileAccessor;
class CAlarmMonitorToSensorCurtain
{
	CLogFileAccessor *m_pLogFileAccessor;
	int		m_iMonitorIONumber;
	int		m_iEMOPosST;
	int		m_iEMOPosED;
	CString m_strLogMsg;
	int		m_iAlarmID;

protected:
	CSystemConfigMgr	*m_pSystemConfigMgr;
	CIOMapControlMgr	*m_pIOMapControlMgr;
	CCommTcpIPMgr		*m_pCommTcpIPMgr;

private:
	CWinThread	*m_pThread;
	bool		m_bThreadLive;
	HANDLE		m_hStartEvent;
	HANDLE		m_hCloseEvent;
	static UINT ThreadRun(LPVOID p1);
	UINT ThreadRun();

public:
	void SetPara(CSystemConfigMgr *pSystemConfigMgr, CIOMapControlMgr *pIOMapControlMgr, CCommTcpIPMgr *pCommTcpIPMgr);
	void SetStart();
	void SetStop();

public:
	CAlarmMonitorToSensorCurtain(CString strLogPath, int iMonitorIONumber, int iEMOPosST, int iEMOPosED, CString strLogMsg, int iAlarmID);
    ~CAlarmMonitorToSensorCurtain();
};