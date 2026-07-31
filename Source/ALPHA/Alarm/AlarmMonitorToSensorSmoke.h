#pragma once

class CSystemConfigMgr;
class CMaintControlMgr;
class CCommTcpIPMgr;
class CLogFileAccessor;
class CAlarmMonitorToSensorSmoke
{
	CLogFileAccessor *m_pLogFileAccessor;
	int		m_iMonitorIONumber;
	int		m_iEMOPosST;
	int		m_iEMOPosED;
	CString m_strLogMsg;
	int		m_iAlarmID;

protected:
	CSystemConfigMgr	*m_pSystemConfigMgr;
	CMaintControlMgr	*m_pMaintControlMgr;
	CCommTcpIPMgr		*m_pCommTcpIPMgr;

private:
	CWinThread	*m_pThread;
	bool		m_bThreadLive;
	HANDLE		m_hStartEvent;
	HANDLE		m_hCloseEvent;
	static UINT ThreadRun(LPVOID p1);
	UINT ThreadRun();

public:
	void SetPara(CSystemConfigMgr *pSystemConfigMgr, CMaintControlMgr *pMaintControlMgr);
	void SetStart();
	void SetStop();

public:
	CAlarmMonitorToSensorSmoke(CString strLogPath, int iMonitorIONumber, int iEMOPosST, int iEMOPosED, CString strLogMsg, int iAlarmID);
	~CAlarmMonitorToSensorSmoke();
};

