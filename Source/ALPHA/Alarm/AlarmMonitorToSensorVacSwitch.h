#pragma once

class CSystemConfigMgr;
class CIOMapControlMgr;
class CCommTcpIPMgr;
class CLogFileAccessor;
class CAlarmMonitorToSensorVacSwitch
{
	CLogFileAccessor *m_pLogFileAccessor;

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
	bool		m_bPushHandBtn;
	DWORD	m_dwSetCount;

public:
	void SetPara(CSystemConfigMgr *pSystemConfigMgr, CIOMapControlMgr *pIOMapControlMgr, CCommTcpIPMgr *pCommTcpIPMgr);
	void SetStart();
	void SetStop();

public:
	CAlarmMonitorToSensorVacSwitch(CString strLogPath);
	~CAlarmMonitorToSensorVacSwitch();
};

