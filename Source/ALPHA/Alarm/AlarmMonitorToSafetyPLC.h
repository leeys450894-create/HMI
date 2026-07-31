#pragma once

class CSystemConfigMgr;
class CIOMapControlMgr;
class CCommTcpIPMgr;
class CLogFileAccessor;
class CPingTest;
class CAlarmMonitorToSafetyPLC
{
	CLogFileAccessor* m_pLogFileAccessor;
	int		m_iMonitorIONumber;
	CString m_strLogMsg;
	int		m_iAlarmID;
	CPingTest* m_pPingTest;
	int		m_nNGCount;

protected:
	CSystemConfigMgr* m_pSystemConfigMgr;
	CIOMapControlMgr* m_pIOMapControlMgr;
	CCommTcpIPMgr* m_pCommTcpIPMgr;

private:
	CWinThread* m_pThread;
	bool		m_bThreadLive;
	HANDLE		m_hStartEvent;
	HANDLE		m_hCloseEvent;
	static UINT ThreadRun(LPVOID p1);
	UINT ThreadRun();

public:
	void SetPara(CSystemConfigMgr* pSystemConfigMgr, CIOMapControlMgr* pIOMapControlMgr, CCommTcpIPMgr* pCommTcpIPMgr);
	void SetStart();
	void SetStop();

public:
	CAlarmMonitorToSafetyPLC(CString strLogPath, int iMonitorIONumber, CString strLogMsg, int iAlarmID);
	~CAlarmMonitorToSafetyPLC();
};

