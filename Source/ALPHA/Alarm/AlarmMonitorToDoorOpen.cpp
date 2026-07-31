#include "pch.h"
#include "AlarmMonitorToDoorOpen.h"

#include "AlarmMonitorToDoorOpenState.h"
#include "..\Manager\IOMapAddressDefine.h"
#include "..\SystemParamShare.h"
#include "AlarmController.h"
#include "AlarmMonitorToDoorVoice.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CAlarmMonitorToDoorOpen::CAlarmMonitorToDoorOpen(stAuroraMgrPool &MgrPool)
	: CAlarmMonitorImpl(MgrPool)
{
	m_vecItem.clear();


#ifdef _USE_IOBOARD
	m_vecItem.push_back(new CAlarmMonitorToDoorOpenState(MgrPool.strLogPath, IO_BOARD_READ_INPUT_016, 0, 0, _T("DOOR 01 OPEN (LEFT)"), PLC_DOOR_OPENED_01));
	m_vecItem.push_back(new CAlarmMonitorToDoorOpenState(MgrPool.strLogPath, IO_BOARD_READ_INPUT_017, 0, 0, _T("DOOR 02 OPEN (RIGHT)"), PLC_DOOR_OPENED_02));
#else
	static CString strDoor[6] = {
		_T("DOOR 01 OPEN(LEFT FRONT)"),
		_T("DOOR 02 OPEN(RIGHT FRONT)"),
		_T("DOOR 03 OPEN(RIGHT BACK)"),
		_T("DOOR 04 OPEN(LEFT BACK)"),
		_T("DOOR 05 OPEN(PACC LOWER)"),
		_T("DOOR 06 OPEN(PACC UPPER)"),
	};

	CString strLog;
	for (size_t i = 0; i < 6; i++)
	{
		strLog.Format(strDoor[i]);
		m_vecItem.push_back(new CAlarmMonitorToDoorOpenState(MgrPool.strLogPath, PLC_BIT_DOOR_SENSOR1 + i, enumDoorPositionID_LEFT_FRONT + i, 0, strLog, PLC_DOOR_OPENED_01+i));
	}
#endif // _USE_IOBOARD

	for (size_t i = 0; i < m_vecItem.size(); ++i)
	{
		m_vecItem[i]->SetPara(MgrPool.pSystemConfigMgr, MgrPool.pIOMapControlMgr, MgrPool.pCommTcpIPMgr);
	}

	m_pDoorVoice = new CAlarmMonitorToDoorVoice(MgrPool);
}


CAlarmMonitorToDoorOpen::~CAlarmMonitorToDoorOpen()
{
	for (size_t i = 0; i < m_vecItem.size(); ++i)
	{
		delete m_vecItem[i];
		m_vecItem[i] = nullptr;
	}

	delete m_pDoorVoice;
	m_pDoorVoice = nullptr;
}

void CAlarmMonitorToDoorOpen::SetStart()
{
	for (size_t i = 0; i < m_vecItem.size(); ++i)
	{
		m_vecItem[i]->SetStart();
	}

	m_pDoorVoice->SetStart();
}

void CAlarmMonitorToDoorOpen::SetStop()
{
	for (size_t i = 0; i < m_vecItem.size(); ++i)
	{
		m_vecItem[i]->SetStop();
	}

	m_pDoorVoice->SetStop();
}