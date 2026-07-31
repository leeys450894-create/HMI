#pragma once

#include "AlarmMonitorImpl.h"

class CAlarmMonitorToEmergencyButton;
class CAlarmMonitorToEmergency : public CAlarmMonitorImpl
{
	std::vector<CAlarmMonitorToEmergencyButton *> m_vecItem;

//=========================================================================
// INTERFACE ฐüทร
//=========================================================================
public:
	virtual void SetStart() override;
	virtual void SetStop() override;

public:
	CAlarmMonitorToEmergency(stAuroraMgrPool &MgrPool);
    virtual ~CAlarmMonitorToEmergency();
};