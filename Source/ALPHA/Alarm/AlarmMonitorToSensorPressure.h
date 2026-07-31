#pragma once

class CSystemConfigMgr;
class CIOMapControlMgr;
class CLogFileAccessor;

class CAlarmMonitorToSensorPressure
{
	CLogFileAccessor *m_pLogFileAccessor;
	int		m_iMonitorIONumber;
	CString m_strLogMsg;
	int		m_iAlarmID;

protected:
	CSystemConfigMgr	*m_pSystemConfigMgr;
	CIOMapControlMgr	*m_pIOMapControlMgr;

private:
	CWinThread	*m_pThread;
	bool		m_bThreadLive;
	HANDLE		m_hStartEvent;
	HANDLE		m_hCloseEvent;
	static UINT ThreadRun(LPVOID p1);
	UINT ThreadRun();
	bool		m_bPushHandBtn;
	DWORD		m_dwSetCount;
	UINT		m_nNGCount;

public:
	void SetPara(CSystemConfigMgr *pSystemConfigMgr, CIOMapControlMgr *pIOMapControlMgr);
	void SetStart();
	void SetStop();

public:
	CAlarmMonitorToSensorPressure(CString strLogPath, int iMonitorIONumber, CString strLogMsg, int iAlarmID);
	~CAlarmMonitorToSensorPressure();
};

