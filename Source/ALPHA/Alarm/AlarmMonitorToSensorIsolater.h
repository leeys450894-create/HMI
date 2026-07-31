#pragma once

class CSystemConfigMgr;
//class CMotorControlMgr;
//class CIOMapControlMgr;
class CCommTcpIPMgr;
class CLogFileAccessor;
class CMaintControlMgr;
class CAlarmMonitorToSensorIsolater
{
	CLogFileAccessor *m_pLogFileAccessor;
	int		m_iMonitorIONumber;
	int		m_iEMOPosST;
	int		m_iEMOPosED;
	CString m_strLogMsg;
	int		m_iAlarmID;
	UINT		m_nNGCount;

protected:
	CSystemConfigMgr	*m_pSystemConfigMgr;
	CCommTcpIPMgr		*m_pCommTcpIPMgr;
	CMaintControlMgr	*m_pMaintControlMgr;

private:
	CWinThread	*m_pThread;
	bool		m_bThreadLive;
	HANDLE		m_hStartEvent;
	HANDLE		m_hCloseEvent;
	static UINT ThreadRun(LPVOID p1);
	UINT ThreadRun();

public:
	void SetPara(CSystemConfigMgr *pSystemConfigMgr, CMaintControlMgr* pMaintControlMgr);
	void SetStart();
	void SetStop();

public:
	CAlarmMonitorToSensorIsolater(CString strLogPath, int iMonitorIONumber, int iEMOPosST, int iEMOPosED, CString strLogMsg, int iAlarmID);
	~CAlarmMonitorToSensorIsolater();
};

