#pragma once

#include "AlarmMonitorImpl.h"

class CAlarmMonitorToSensorCurtain;
class CAlarmMonitorToSensorFAN;
class CAlarmMonitorToSensorTemperature;
class CAlarmMonitorToSensorSmoke;
class CAlarmMonitorToSensorIsolater;
class CAlarmMonitorToSensorVacSwitch;
class CAlarmMonitorToSensorPressure;
class CAlarmMonitorToSensorTemperatureLow;
class CAlarmMonitorToLampTime;
class CAlarmMonitorToSafetyPLC;
class CAlarmMonitorToSensor : public CAlarmMonitorImpl
{
	CAlarmMonitorToSensorCurtain		*m_pSensorCurtain;
	CAlarmMonitorToSensorVacSwitch		*m_pVaccumSwitch;
	CAlarmMonitorToSafetyPLC* m_pSafetyPLC;
	std::vector<CAlarmMonitorToSensorIsolater *> m_vecItem;
	std::vector<CAlarmMonitorToSensorFAN *> m_vecItemFan;
	std::vector<CAlarmMonitorToSensorPressure *> m_vecItemPressure;
	std::vector<CAlarmMonitorToSensorSmoke*> m_vecItemSmoke;
	std::vector<CAlarmMonitorToSensorTemperature*> m_vecItemTemperature;
	std::vector<CAlarmMonitorToSensorTemperatureLow*> m_vecItemTemperatureLow;
	std::vector<CAlarmMonitorToLampTime*> m_vecItemLampTime;

//=========================================================================
// INTERFACE ฐüทร
//=========================================================================
public:
	virtual void SetStart() override;
	virtual void SetStop() override;

public:
	CAlarmMonitorToSensor(stAuroraMgrPool &MgrPool);
    virtual ~CAlarmMonitorToSensor();
};