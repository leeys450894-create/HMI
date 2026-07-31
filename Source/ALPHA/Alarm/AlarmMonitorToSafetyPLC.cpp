#include "pch.h"
#include "AlarmMonitorToSafetyPLC.h"

#include "..\Manager\SystemConfigMgr.h"
#include "..\Manager\IOMapControlMgr.h"
#include "..\Manager\IOMapAddressDefine.h"
#include "AlarmController.h"
#include "..\SystemParamShare.h"
#include "..\AuroraFuncDefine.h"
#include "LogFunction\LogAccessor\LogFileAccessor.h"
#include "common/PingTest.h"
#include "common/CommonFunc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CAlarmMonitorToSafetyPLC::CAlarmMonitorToSafetyPLC(CString strLogPath, int iMonitorIONumber, CString strLogMsg, int iAlarmID)
	: m_iMonitorIONumber(iMonitorIONumber)
		, m_strLogMsg(strLogMsg)
		, m_iAlarmID(iAlarmID)
		, m_pLogFileAccessor(new CLogFileAccessor)
		, m_nNGCount(0)
	{
		const CString strFolderPath = strLogPath + _T("\\Event\\");
		const CString strLogFile = strFolderPath + _T("Safety_PLC.log");
		MakeDir(strFolderPath);
		m_pLogFileAccessor->Initialize(strLogFile);

		//----------------------------------------------------------------------
		// Thread 생성
		//----------------------------------------------------------------------
		m_pThread = AfxBeginThread(ThreadRun, this);
		m_bThreadLive = false;
		m_hStartEvent = CreateEvent(NULL, TRUE, FALSE, NULL);		// manual set, initial reset
		m_hCloseEvent = CreateEvent(NULL, FALSE, FALSE, NULL);	// auto reset, initial reset


		m_pPingTest = nullptr;
		m_pPingTest = new CPingTest();
}


CAlarmMonitorToSafetyPLC::~CAlarmMonitorToSafetyPLC()
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

	if (m_pPingTest)
	{
		delete m_pPingTest;
		m_pPingTest = nullptr;
	}
}

UINT CAlarmMonitorToSafetyPLC::ThreadRun(LPVOID p1)
{
	CAlarmMonitorToSafetyPLC* pThis = (CAlarmMonitorToSafetyPLC*)p1;

	pThis->m_bThreadLive = true;

	while (1)
	{
		// Event 가 set될 때까지 기다린다.
		if (WaitForSingleObject(pThis->m_hStartEvent, 10) == WAIT_OBJECT_0)
		{
			// 최대한 빠르게 IO 를 확인하자.
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

UINT CAlarmMonitorToSafetyPLC::ThreadRun()
{
	//***********************************************************************************************************************
	//***********************************************************************************************************************
	// 설비 알람 사용 유/무
	//***********************************************************************************************************************
	//***********************************************************************************************************************
	if (m_pSystemConfigMgr->IsUseAlarmMonitor() == false)
		return 0;

	//--------------------------------------------------------
	// 설비 Safety PLC Connect
	// PING TEST
	//--------------------------------------------------------
	bool bRet = false;
//	nRet = m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(PLC_BIT_ARM_DETECT_INTERLOCK_SR);
	CString strAddress;
	strAddress = _T("192.168.250.1 -n 1");
	USES_CONVERSION;
	bRet = m_pPingTest->PingTest(T2A(strAddress));
	if (bRet != true)
	{
		/*m_pLogFileAccessor->KMLOG(WARN, m_strLogMsg);
		CAlarmController::GetInstance()->SetAlarm(m_iAlarmID);*/

		m_nNGCount++;
	}
	else
	{
		m_nNGCount = 0;
	}

	if (m_nNGCount > 3)
	{
		m_pLogFileAccessor->KMLOG(WARN, m_strLogMsg);
		CAlarmController::GetInstance()->SetAlarm(m_iAlarmID);

		m_nNGCount = 0;
	}

	return 0;
}

void CAlarmMonitorToSafetyPLC::SetPara(CSystemConfigMgr* pSystemConfigMgr, CIOMapControlMgr* pIOMapControlMgr, CCommTcpIPMgr* pCommTcpIPMgr)
{
	m_pSystemConfigMgr = pSystemConfigMgr;
	m_pIOMapControlMgr = pIOMapControlMgr;
	m_pCommTcpIPMgr = pCommTcpIPMgr;
}

void CAlarmMonitorToSafetyPLC::SetStart()
{
	SetEvent(m_hStartEvent);
}

void CAlarmMonitorToSafetyPLC::SetStop()
{
	ResetEvent(m_hStartEvent);
}