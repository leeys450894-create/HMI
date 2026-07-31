#include "pch.h"
#include "AlarmMonitorMgr.h"

#include "AlarmMonitorToEmergency.h"
#include "AlarmMonitorToMcOFF.h"
#include "AlarmMonitorToSensor.h"
#include "AlarmMonitorToTeachKEY.h"
#include "AlarmMonitorToTeachKEYLock.h"
#include "AlarmMonitorToDoorOpen.h"
#include "..\Manager\IOMapControlMgr.h"
#include "..\Manager\IOMapAddressDefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CAlarmMonitorMgr::CAlarmMonitorMgr(stAuroraMgrPool &MgrPool)
	: m_pIOMapControlMgr(MgrPool.pIOMapControlMgr)
{
    SetName(_T("ALARM_MONITOR_MGR"));

	m_vecMonitor.clear();
	m_vecMonitor.push_back(dynamic_cast<CAlarmMonitorImpl*>(new CAlarmMonitorToEmergency(MgrPool)));	// EMS 버튼 
	m_vecMonitor.push_back(dynamic_cast<CAlarmMonitorImpl*>(new CAlarmMonitorToMcOFF(MgrPool)));		// MC OFF
	m_vecMonitor.push_back(dynamic_cast<CAlarmMonitorImpl*>(new CAlarmMonitorToSensor(MgrPool)));		// 센서
	m_vecMonitor.push_back(dynamic_cast<CAlarmMonitorImpl*>(new CAlarmMonitorToTeachKEY(MgrPool)));		// Teach Key
	m_vecMonitor.push_back(dynamic_cast<CAlarmMonitorImpl*>(new CAlarmMonitorToDoorOpen(MgrPool)));		// Door Open

//	m_vecMonitor.push_back(dynamic_cast<CAlarmMonitorImpl*>(new CAlarmMonitorToTeachKEYLock(MgrPool)));
}

CAlarmMonitorMgr::~CAlarmMonitorMgr()
{
	Finalize();
}

void CAlarmMonitorMgr::Finalize()
{
    for(size_t i = 0; i < m_vecMonitor.size(); ++i)
	{
		ULONGLONG dwStart = GetTickCount();

		if(m_vecMonitor[i])
		{
			delete m_vecMonitor[i];
			m_vecMonitor[i] = nullptr;
		}

		TRACE("Finalize CAlarmMonitorMgr, %d, %d\n", i, GetTickCount() - dwStart);
	}
}



void CAlarmMonitorMgr::SetMonitoringStart()
{
    for(size_t i = 0; i < m_vecMonitor.size(); ++i)
	{
		if(m_vecMonitor[i])
		{
			m_vecMonitor[i]->SetStart();
		}
	}
}

void CAlarmMonitorMgr::SetMonitoringStop()
{
    for(size_t i = 0; i < m_vecMonitor.size(); ++i)
	{
		if(m_vecMonitor[i])
		{
			m_vecMonitor[i]->SetStop();
		}
	}

	Sleep(1000);
}

bool CAlarmMonitorMgr::IsSafetyResetOn()
{
	bool ret = true;

	if (m_pIOMapControlMgr->IsOutOfService() == true)
	{
		return ret;
	}

#ifdef _USE_IOBOARD
	//bool bSRON_oct_laser1 = m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(CCLINK_IO_BOARD_SYSTEM_R_BIT_090);
	//bool bSRON_oct_laser2 = m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(CCLINK_IO_BOARD_SYSTEM_R_BIT_091);
	bool bSRON_oct_laser1 = m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_IO_BOARD_READ_INPUT_007);
	bool bSRON_oct_laser2 = m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_008);
#else
	bool bSRON_oct_laser1 = m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(PLC_BIT_MC_CONTACT_01);
	bool bSRON_oct_laser2 = m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(PLC_BIT_MC_CONTACT_02);
#endif // _USE_IOBOARD


	return (bSRON_oct_laser1 && bSRON_oct_laser2);
}