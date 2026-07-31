#include "pch.h"
#include "AlarmMonitorImpl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CAlarmMonitorImpl::CAlarmMonitorImpl(stAuroraMgrPool &MgrPool)
    : m_pSystemConfigMgr(MgrPool.pSystemConfigMgr)
	, m_pLogInControlMgr(MgrPool.pLogInControlMgr)
    , m_pIOMapControlMgr(MgrPool.pIOMapControlMgr)
    , m_pMaintControlMgr(MgrPool.pMaintControlMgr)
    , m_pCommTcpIPMgr(MgrPool.pCommTcpIPMgr)
{
}

CAlarmMonitorImpl::~CAlarmMonitorImpl()
{
}

