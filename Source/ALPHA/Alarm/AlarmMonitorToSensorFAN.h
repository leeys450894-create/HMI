#pragma once

class CSystemConfigMgr;
//class CMotorControlMgr;
//class CIOMapControlMgr;
class CCommTcpIPMgr;
class CLogFileAccessor;
class CMaintControlMgr;
class CAlarmMonitorToSensorFAN
{
	CLogFileAccessor *m_pLogFileAccessor;
	int		m_iMonitorIONumber;
	int		m_iEMOPosST;
	int		m_iEMOPosED;
	CString m_strLogMsg;
	int		m_iAlarmID;

protected:
	CSystemConfigMgr	*m_pSystemConfigMgr;
//	CIOMapControlMgr	*m_pIOMapControlMgr;
	CCommTcpIPMgr		*m_pCommTcpIPMgr;
//	CMotorControlMgr	*m_pMotorControlMgr;
	CMaintControlMgr*	m_pMaintControlMgr;

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
//	void SetPara(CSystemConfigMgr *pSystemConfigMgr, CIOMapControlMgr *pIOMapControlMgr, CMotorControlMgr *pMotorControlMgr);
	void SetPara(CSystemConfigMgr* pSystemConfigMgr, CMaintControlMgr* pMaintControlMgr);
	void SetStart();
	void SetStop();

public:
	CAlarmMonitorToSensorFAN(CString strLogPath, int iMonitorIONumber, int iEMOPosST, int iEMOPosED, CString strLogMsg, int iAlarmID);
    ~CAlarmMonitorToSensorFAN();
};