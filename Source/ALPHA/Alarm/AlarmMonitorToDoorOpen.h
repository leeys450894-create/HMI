#pragma once
#include "AlarmMonitorImpl.h"

class CAlarmMonitorToDoorOpenState;
class CAlarmMonitorToDoorVoice;
class CAlarmMonitorToDoorOpen : public CAlarmMonitorImpl
{
	std::vector<CAlarmMonitorToDoorOpenState *> m_vecItem;
	
	CAlarmMonitorToDoorVoice		*m_pDoorVoice;

	//=========================================================================
	// INTERFACE ฐüทร
	//=========================================================================
public:
	virtual void SetStart() override;
	virtual void SetStop() override;

public:
	CAlarmMonitorToDoorOpen(stAuroraMgrPool &MgrPool);
	~CAlarmMonitorToDoorOpen();
};

