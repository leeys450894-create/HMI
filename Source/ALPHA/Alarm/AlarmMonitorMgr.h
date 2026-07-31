#pragma once

#include "..\AuroraSubMgr.h"
#include "..\AuroraMgrPool.h"

class CAlarmMonitorImpl;
class CAlarmMonitorMgr : public CAuroraSubMgr
{
	CIOMapControlMgr *m_pIOMapControlMgr;
	std::vector<CAlarmMonitorImpl *> m_vecMonitor;

public:
	void SetMonitoringStart();
	void SetMonitoringStop();
	bool IsSafetyResetOn();



//------------------------------------------------------------------
// 공통 인터페이스 함수
//------------------------------------------------------------------
private:
	virtual void Finalize() override;
public:
	CAlarmMonitorMgr(stAuroraMgrPool &MgrPool);
    virtual ~CAlarmMonitorMgr();
};