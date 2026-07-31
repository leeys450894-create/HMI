#pragma once

#include "AlarmMonitorImpl.h"

class CLogFileAccessor;
class CAlarmMonitorToDoorVoice : public CAlarmMonitorImpl
{
	CLogFileAccessor *m_pLogFileAccessor;
	CWinThread	*m_pThread;
	bool		m_bThreadLive;
	HANDLE		m_hStartEvent;
	HANDLE		m_hCloseEvent;
	static UINT ThreadRun(LPVOID p1);
	UINT ThreadRun();

	//=========================================================================
	// INTERFACE ฐüทร
	//=========================================================================
public:
	virtual void SetStart() override;
	virtual void SetStop() override;

public:
	CAlarmMonitorToDoorVoice(stAuroraMgrPool &MgrPool);
	~CAlarmMonitorToDoorVoice();
};

