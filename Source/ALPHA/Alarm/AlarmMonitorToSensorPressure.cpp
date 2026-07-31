#include "pch.h"
#include "AlarmMonitorToSensorPressure.h"

#include "..\Manager\SystemConfigMgr.h"
#include "..\Manager\IOMapControlMgr.h"
#include "..\Manager\IOMapAddressDefine.h"
#include "AlarmController.h"
#include "..\SystemParamShare.h"
#include "..\AuroraFuncDefine.h"
#include "LogFunction\LogAccessor\LogFileAccessor.h"

CAlarmMonitorToSensorPressure::CAlarmMonitorToSensorPressure(CString strLogPath, int iMonitorIONumber, CString strLogMsg, int iAlarmID)
	: m_iMonitorIONumber(iMonitorIONumber)
	, m_strLogMsg(strLogMsg)
	, m_iAlarmID(iAlarmID)
	, m_pLogFileAccessor(new CLogFileAccessor)
	, m_bPushHandBtn(false)
	, m_nNGCount(0)
{
	const CString strFolderPath = strLogPath + _T("\\Event\\");
	const CString strLogFile = strFolderPath + _T("sensor_fan.log");
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


CAlarmMonitorToSensorPressure::~CAlarmMonitorToSensorPressure()
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

UINT CAlarmMonitorToSensorPressure::ThreadRun(LPVOID p1)
{
	CAlarmMonitorToSensorPressure *pThis = (CAlarmMonitorToSensorPressure *)p1;

	pThis->m_bThreadLive = true;

	while (1)
	{
		// Event 가 set될 때까지 기다린다.
		if (WaitForSingleObject(pThis->m_hStartEvent, 10) == WAIT_OBJECT_0)
		{
			// 설비의 FAN 이 멈춘거는 IO 체크를 천천히 해도 될거 같다.
			Sleep(1000);

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

UINT CAlarmMonitorToSensorPressure::ThreadRun()
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

	//--------------------------------------------------------
	// Pressure Off
	//--------------------------------------------------------
	int nRet = 0;
	nRet = m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(m_iMonitorIONumber);
	if (nRet != -1)
	{
		if (nRet == 0)
		{
			m_nNGCount++;
		}
		else
		{
			m_nNGCount = 0;
		}
	}

	if (m_nNGCount > 2)
	{
		m_pLogFileAccessor->KMLOG(WARN, m_strLogMsg);
		CAlarmController::GetInstance()->SetAlarm(m_iAlarmID);

		m_nNGCount = 0;
	}

	/*if (m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(m_iMonitorIONumber) != true)
	{
		m_pLogFileAccessor->KMLOG(WARN, m_strLogMsg);
		CAlarmController::GetInstance()->SetAlarm(m_iAlarmID);
	}*/

	return 0;
}

void CAlarmMonitorToSensorPressure::SetPara(CSystemConfigMgr *pSystemConfigMgr, CIOMapControlMgr *pIOMapControlMgr)
{
	m_pSystemConfigMgr = pSystemConfigMgr;
	m_pIOMapControlMgr = pIOMapControlMgr;
}

void CAlarmMonitorToSensorPressure::SetStart()
{
	SetEvent(m_hStartEvent);
}

void CAlarmMonitorToSensorPressure::SetStop()
{
	ResetEvent(m_hStartEvent);
}