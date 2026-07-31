#include "pch.h"
#include "AlarmMonitorToSensorHandSwitch.h"

#include "..\Manager\SystemConfigMgr.h"
#include "..\Manager\IOMapControlMgr.h"
#include "..\Manager\IOMapAddressDefine.h"
#include "AlarmController.h"
#include "..\SystemParamShare.h"
#include "..\AuroraFuncDefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CAlarmMonitorToSensorHandSwitch::CAlarmMonitorToSensorHandSwitch(CString strLogPath)
	: m_pLogFileAccessor(new CLogFileAccessor)
{
	const CString strFolderPath = strLogPath + _T("\\Event\\");
	const CString strLogFile = strFolderPath + _T("sensor_HandSwitch.log");
	MakeDir(strFolderPath);
	m_pLogFileAccessor->Initialize(strLogFile);

	//----------------------------------------------------------------------
	// Thread 생성
	//----------------------------------------------------------------------
	m_pThread = AfxBeginThread(ThreadRun, this);
	m_bThreadLive = false;
	m_hStartEvent = CreateEvent(NULL, TRUE, FALSE, NULL);		// manual set, initial reset
	m_hCloseEvent = CreateEvent(NULL, FALSE, FALSE, NULL);	// auto reset, initial reset
}


CAlarmMonitorToSensorHandSwitch::~CAlarmMonitorToSensorHandSwitch()
{
	//----------------------------------------------------------------------
	// Thread 종료
	//----------------------------------------------------------------------
	ResetEvent(m_hStartEvent);
	SetEvent(m_hCloseEvent);
	if (m_pThread && m_bThreadLive)
	{
		WaitForSingleObject(m_pThread->m_hThread, INFINITE);
	}
	if (m_hStartEvent)CloseHandle(m_hStartEvent);
	if (m_hCloseEvent)CloseHandle(m_hCloseEvent);

	//----------------------------------------------------------------------
	// Data 삭제
	//----------------------------------------------------------------------
	if (m_pLogFileAccessor)
	{
		delete m_pLogFileAccessor;
		m_pLogFileAccessor = nullptr;
	}
}

UINT CAlarmMonitorToSensorHandSwitch::ThreadRun(LPVOID p1)
{
	CAlarmMonitorToSensorHandSwitch *pThis = (CAlarmMonitorToSensorHandSwitch *)p1;

	pThis->m_bThreadLive = true;

	while (1)
	{
		// Event 가 set될 때까지 기다린다.
		if (WaitForSingleObject(pThis->m_hStartEvent, 10) == WAIT_OBJECT_0)
		{
			// 설비의 FAN 이 멈춘거는 IO 체크를 천천히 해도 될거 같다.
			Sleep(500);

			pThis->ThreadRun();
		}

		// Close Event 가 set될 때까지 기다린다.
		if (WaitForSingleObject(pThis->m_hCloseEvent, 10) == WAIT_OBJECT_0)
		{
			break;
		}

		Sleep(1);
	}

	pThis->m_bThreadLive = false;

	return 0;
}

UINT CAlarmMonitorToSensorHandSwitch::ThreadRun()
{
	//***********************************************************************************************************************
	//***********************************************************************************************************************
	// 설비 알람 사용 유/무
	//***********************************************************************************************************************
	//***********************************************************************************************************************
	if (m_pSystemConfigMgr->IsUseAlarmMonitor() == false)
		return 0;

	///
	// 연결 유무확인
	//
	if (m_pIOMapControlMgr->IsConnected() != true)
		return 0;

#ifdef _USE_IOBOARD
	//--------------------------------------------------------
	// TWO-HAND SWITCH 체크
	//--------------------------------------------------------
	bool bTimeOver = false;
	if (m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_114) != true && m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_115) != true)
	{
		m_bPushHandBtn = false;
	}
	else
	{
		if (m_bPushHandBtn == false)
		{
			m_dwSetCount = GetTickCount();
			m_bPushHandBtn = true;
		}

		if (m_bPushHandBtn == true)
		{
			DWORD dwR, dwT = 0;
			dwR = GetTickCount();
			if (dwR >= m_dwSetCount)
				dwT = dwR - m_dwSetCount;
			else
				dwT = ((DWORD)0xFFFFFFFF) - (m_dwSetCount - dwR);

			if (dwT > 30 * 1000)		// 10sec
			{
				bTimeOver = true;
			}
		}
	}

	if (bTimeOver)
	{
		m_pLogFileAccessor->KMLOG(WARN, _T("TWO-HAND SWITCH ON is TIME OVER(30sec)"));
		CAlarmController::GetInstance()->SetAlarm(PLC_TWO_HAND_SWITCH_ALARM);
	}

#endif // _USE_IOBOARD

	return 0;
}

void CAlarmMonitorToSensorHandSwitch::SetPara(CSystemConfigMgr *pSystemConfigMgr, CIOMapControlMgr *pIOMapControlMgr, CCommTcpIPMgr *pCommTcpIPMgr)
{
	m_pSystemConfigMgr = pSystemConfigMgr;
	m_pIOMapControlMgr = pIOMapControlMgr;
	m_pCommTcpIPMgr = pCommTcpIPMgr;
}

void CAlarmMonitorToSensorHandSwitch::SetStart()
{
	SetEvent(m_hStartEvent);
}

void CAlarmMonitorToSensorHandSwitch::SetStop()
{
	ResetEvent(m_hStartEvent);
}
