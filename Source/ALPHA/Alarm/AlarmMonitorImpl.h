#pragma once

#include "..\AuroraMgrPool.h"

class CAlarmMonitorImpl
{
protected:
	CSystemConfigMgr	*m_pSystemConfigMgr;
	CLogInControlMgr	*m_pLogInControlMgr;
	CIOMapControlMgr	*m_pIOMapControlMgr;
	CMaintControlMgr	*m_pMaintControlMgr;
	CCommTcpIPMgr		*m_pCommTcpIPMgr;

//=========================================================================
// INTERFACE ฐüทร
//=========================================================================
public:
	virtual void SetStart() = 0;
	virtual void SetStop() = 0;

public:
	CAlarmMonitorImpl(stAuroraMgrPool &MgrPool);
    virtual ~CAlarmMonitorImpl();
};