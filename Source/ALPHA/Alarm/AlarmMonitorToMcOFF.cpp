#include "pch.h"
#include "AlarmMonitorToMcOFF.h"

#include "AlarmMonitorToMcOFFState.h"
#include "..\Manager\IOMapAddressDefine.h"
#include "..\SystemParamShare.h"
#include "AlarmController.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CAlarmMonitorToMcOFF::CAlarmMonitorToMcOFF(stAuroraMgrPool &MgrPool)
    : CAlarmMonitorImpl(MgrPool)
{
	m_vecItem.clear();

#ifdef _USE_IOBOARD
	m_vecItem.push_back(new CAlarmMonitorToMcOFFState(MgrPool.strLogPath, IO_BOARD_READ_INPUT_007, 0, 0, _T("EQ MOTION MC 01 OFF"), PLC_MC_OFF_ALARM_101));
	m_vecItem.push_back(new CAlarmMonitorToMcOFFState(MgrPool.strLogPath, IO_BOARD_READ_INPUT_008, 0, 0, _T("EQ MOTION MC 02 OFF"), PLC_MC_OFF_ALARM_102));
	m_vecItem.push_back(new CAlarmMonitorToMcOFFState(MgrPool.strLogPath, IO_BOARD_READ_INPUT_009, 0, 0, _T("EFU MC 03 OFF"),		PLC_MC_OFF_ALARM_301));
	m_vecItem.push_back(new CAlarmMonitorToMcOFFState(MgrPool.strLogPath, IO_BOARD_READ_INPUT_002, 0, 0, _T("SAFETYPLC CP OFF"),	PLC_SAFETY_PLC_OFF_ALARM));
	m_vecItem.push_back(new CAlarmMonitorToMcOFFState(MgrPool.strLogPath, IO_BOARD_READ_INPUT_004, 0, 0, _T("SR CP OFF"),			PLC_CP_OFF_ALARM));
	m_vecItem.push_back(new CAlarmMonitorToMcOFFState(MgrPool.strLogPath, IO_BOARD_READ_INPUT_003, 0, 0, _T("MC CP OFF"),			PLC_SMPS1_CP_OFF_ALARM));
#else

	static CString strMCText[10] = {
			_T("MC 101 ON(EQ MOTION)"),
			_T("MC 102 ON(EQ MOTION)"),
			_T("MC 301 ON(EFU)"),
			_T("MC 401 ON(X - RAY IONIZER)"),
			_T("MC 402 ON(X - RAY IONIZER)"),
			_T("MC 501 ON(SACC)"),
			_T("MC 502 ON(SACC)"),
			_T("SR 10 ON(EQ MOTION MC POWER)"),
			_T("SR 50 ON(SACC MC POWER)"),
			_T("SR 40 ON(X - RAY IONIZER MC POWER)")
	};

	CString strLog;
	for (size_t i = 0; i < 10; i++)
	{
		strLog.Format(strMCText[i]);
		m_vecItem.push_back(new CAlarmMonitorToMcOFFState(MgrPool.strLogPath, PLC_BIT_MC_CONTACT_01 + i, 0, 0, strLog, PLC_MC_OFF_ALARM_101 + i)); // jsyoon 20211014 change Muting Switch not Use, EMS Op IO ºÒ·®
	}

#endif // _USE_IOBOARD

	for(size_t i = 0; i < m_vecItem.size(); ++i)
	{
		m_vecItem[i]->SetPara(MgrPool.pSystemConfigMgr, MgrPool.pIOMapControlMgr, MgrPool.pCommTcpIPMgr);
	}
}

CAlarmMonitorToMcOFF::~CAlarmMonitorToMcOFF()
{
	for(size_t i = 0; i < m_vecItem.size(); ++i)
	{
		delete m_vecItem[i];
		m_vecItem[i] = nullptr;
	}
}

void CAlarmMonitorToMcOFF::SetStart()
{
    for(size_t i = 0; i < m_vecItem.size(); ++i)
	{
		m_vecItem[i]->SetStart();
	}
}

void CAlarmMonitorToMcOFF::SetStop()
{
    for(size_t i = 0; i < m_vecItem.size(); ++i)
	{
		m_vecItem[i]->SetStop();		
	}
}

