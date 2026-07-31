#include "pch.h"
#include "AlarmMonitorToEmergency.h"

#include "AlarmController.h"
#include "AlarmMonitorToEmergencyButton.h"
#include "..\Manager\IOMapAddressDefine.h"
#include "..\SystemParamShare.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CAlarmMonitorToEmergency::CAlarmMonitorToEmergency(stAuroraMgrPool &MgrPool)
    : CAlarmMonitorImpl(MgrPool)
{
	m_vecItem.clear();


#ifdef _USE_IOBOARD
	m_vecItem.push_back(new CAlarmMonitorToEmergencyButton(MgrPool.strLogPath, IO_BOARD_READ_INPUT_012 /*IO_BOARD_READ_INPUT_022*/, enumEMOPositionID_OP,		enumDoWorkStateID_Idle, _T("PRESS - EMS SWITCH 01 PUSH (OP)"),			PLC_EMO_SWITCH_PUSHED_02)); // jsyoon 20211014 change Muting Switch not Use, EMS Op IO 불량
	m_vecItem.push_back(new CAlarmMonitorToEmergencyButton(MgrPool.strLogPath, IO_BOARD_READ_INPUT_024, enumEMOPositionID_RIGHT,	enumDoWorkStateID_Idle, _T("PRESS - EMS SWITCH 03 PUSH (RIGHT)"),		PLC_EMO_SWITCH_PUSHED_04));
	m_vecItem.push_back(new CAlarmMonitorToEmergencyButton(MgrPool.strLogPath, IO_BOARD_READ_INPUT_023, enumEMOPositionID_FRONT,	enumDoWorkStateID_Idle, _T("PRESS - EMS SWITCH 02 PUSH (FRONT)"),		PLC_EMO_SWITCH_PUSHED_09));
	m_vecItem.push_back(new CAlarmMonitorToEmergencyButton(MgrPool.strLogPath, IO_BOARD_READ_INPUT_025, enumEMOPositionID_EBOX,		enumDoWorkStateID_Idle, _T("PRESS - EMS SWITCH 04 PUSH (EL.BOX)"),		PLC_EMO_SWITCH_PUSHED_05));

#else

	static CString strEMSName[16] = {
		_T("EMS SWITCH 01 PUSH(OP)"),
		_T("EMS SWITCH 02 PUSH(LEFT FRONT)"),
		_T("EMS SWITCH 03 PUSH(RIGHT FRONT)"),
		_T("EMS SWITCH 04 PUSH(RIGHT BACK)"),
		_T("EMS SWITCH 05 PUSH(LEFT BACK)"),
		_T("EMS SWITCH 06 PUSH(INNER LEFT FRONT)"),
		_T("EMS SWITCH 07 PUSH(INNER RIGHT FRONT)"),
		_T("EMS SWITCH 08 PUSH(INNER RIGHT BACK)"),
		_T("EMS SWITCH 09 PUSH(INNER LEFT BACK)"),
		_T("EMS SWITCH 10 PUSH(SACC)"),
		_T("EMS SWITCH 11 PUSH(INNER SACC)"),
		_T("EMS SWITCH 12 PUSH(E - BOX FRONT(UPS))"),
		_T("EMS SWITCH 13 PUSH(E - BOX REAR(GPS))"),
		_T("EMS SWITCH 14 PUSH(ENABLE SWITCH)"),
		_T("EMS SWITCH 15 (S - BOX FRONT)"),
		_T("EMS SWITCH 16 (S - BOX REAR)"),
	};

	CString strLog;
	for (size_t i = 0; i < 16; i++)
	{
		strLog.Format(strEMSName[i]);
		m_vecItem.push_back(new CAlarmMonitorToEmergencyButton(MgrPool.strLogPath, PLC_BIT_EMERGENCY_SWITCH01 + i, enumEMOPositionID_OP + i, enumDoWorkStateID_Idle, strLog, PLC_EMO_SWITCH_PUSHED_01 + i)); // jsyoon 20211014 change Muting Switch not Use, EMS Op IO 불량
	}

#endif // _USE_IOBOARD


	for(size_t i = 0; i < m_vecItem.size(); ++i)
	{
		m_vecItem[i]->SetPara(MgrPool.pSystemConfigMgr, MgrPool.pIOMapControlMgr, MgrPool.pCommTcpIPMgr);
	}
}

CAlarmMonitorToEmergency::~CAlarmMonitorToEmergency()
{
	for(size_t i = 0; i < m_vecItem.size(); ++i)
	{
		delete m_vecItem[i];
		m_vecItem[i] = nullptr;
	}
}

void CAlarmMonitorToEmergency::SetStart()
{
    for(size_t i = 0; i < m_vecItem.size(); ++i)
	{
		m_vecItem[i]->SetStart();
	}
}

void CAlarmMonitorToEmergency::SetStop()
{
    for(size_t i = 0; i < m_vecItem.size(); ++i)
	{
		m_vecItem[i]->SetStop();		
	}
}
