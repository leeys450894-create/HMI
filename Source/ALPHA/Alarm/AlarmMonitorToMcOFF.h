#pragma once

#include "AlarmMonitorImpl.h"

class CAlarmMonitorToMcOFFState;
class CAlarmMonitorToMcOFF : public CAlarmMonitorImpl
{
	std::vector<CAlarmMonitorToMcOFFState *> m_vecItem;

//=========================================================================
// INTERFACE ฐüทร
//=========================================================================
public:
	virtual void SetStart() override;
	virtual void SetStop() override;

public:
	CAlarmMonitorToMcOFF(stAuroraMgrPool &MgrPool);
    virtual ~CAlarmMonitorToMcOFF();
};