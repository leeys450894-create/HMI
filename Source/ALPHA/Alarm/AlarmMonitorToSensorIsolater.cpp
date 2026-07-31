#include "pch.h"
#include "AlarmMonitorToSensorIsolater.h"

#include "..\Manager\SystemConfigMgr.h"
#include "..\Manager\IOMapAddressDefine.h"
#include "AlarmController.h"
#include "..\SystemParamShare.h"
#include "..\AuroraFuncDefine.h"
#include "LogFunction\LogAccessor\LogFileAccessor.h"
#include "..\Manager\MaintControlMgr.h"


CAlarmMonitorToSensorIsolater::CAlarmMonitorToSensorIsolater(CString strLogPath, int iMonitorIONumber, int iEMOPosST, int iEMOPosED, CString strLogMsg, int iAlarmID)
	: m_iMonitorIONumber(iMonitorIONumber)
	, m_iEMOPosST(iEMOPosST)
	, m_iEMOPosED(iEMOPosED)
	, m_strLogMsg(strLogMsg)
	, m_iAlarmID(iAlarmID)
	, m_pLogFileAccessor(new CLogFileAccessor)
	, m_nNGCount(0)
{
	const CString strFolderPath = strLogPath + _T("\\Event\\");
	CString strLogFile;
	strLogFile.Format(_T("%s%s_%02d.log"), strFolderPath, _T("sensor_Isolater"), iMonitorIONumber + 1);
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


CAlarmMonitorToSensorIsolater::~CAlarmMonitorToSensorIsolater()
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

UINT CAlarmMonitorToSensorIsolater::ThreadRun(LPVOID p1)
{
	CAlarmMonitorToSensorIsolater *pThis = (CAlarmMonitorToSensorIsolater *)p1;

	pThis->m_bThreadLive = true;

	while (1)
	{
		// Event 가 set될 때까지 기다린다.
		if (WaitForSingleObject(pThis->m_hStartEvent, 10) == WAIT_OBJECT_0)
		{
			// jsyoon 20211019 change
			Sleep(1000);
			//Sleep(2000);

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

UINT CAlarmMonitorToSensorIsolater::ThreadRun()
{
	//***********************************************************************************************************************
	//***********************************************************************************************************************
	// 설비 알람 사용 유/무
	//***********************************************************************************************************************
	//***********************************************************************************************************************
	if (m_pSystemConfigMgr->IsUseAlarmMonitor() == false)
		return 0;

	//--------------------------------------------------------
	// 센서로 연결되어 있는 부분
	//--------------------------------------------------------
	if (m_pSystemConfigMgr->IsUseHardwareSensor() == false)
		return 0;

	///
	// 연결 유무확인
	//
	if (m_pMaintControlMgr->GetIOMapControlMgrPtr()->IsConnected() != true)
		return 0;

	//--------------------------------------------------------
	// Isolater 센서, B접
	//--------------------------------------------------------
	if (m_pMaintControlMgr->GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(m_iMonitorIONumber) != true)
	{
		m_nNGCount++;
	}
	else
	{
		m_nNGCount = 0;
	}

	if (m_nNGCount > 10)  // 1000 * 10 = 10sec  // 500 to 10
	{
		m_pLogFileAccessor->KMLOG(WARN, m_strLogMsg);
		CAlarmController::GetInstance()->SetAlarm(m_iAlarmID);

		m_nNGCount = 0;
	}

	////--------------------------------------------------------
	//// Isolater 센서, A접
	////--------------------------------------------------------
	//int nUnit = 0, nPort = 0;
	//int nInputUnit[4] = { 0, 0, 0, 0};
	//int nInputPort[4] = { 4, 5, 6, 7};

	//nUnit = nInputUnit[m_iMonitorIONumber];
	//nPort = nInputPort[m_iMonitorIONumber];
	//if (m_pMaintControlMgr->GetMotorControlMgrPtr()->GetInput(nUnit, nPort) != 1)
	//{
	//	m_nNGCount++;
	//}
	//else
	//{
	//	m_nNGCount = 0;
	//}

	//if (m_nNGCount > 5)	// jsyoon 20211019 change 2 to 5 
	//{
	//	m_pLogFileAccessor->KMLOG(WARN, m_strLogMsg);
	//	CAlarmController::GetInstance()->SetAlarm(m_iAlarmID);

	//	m_nNGCount = 0;
	//}

	return 0;
}

void CAlarmMonitorToSensorIsolater::SetPara(CSystemConfigMgr* pSystemConfigMgr, CMaintControlMgr* pMaintControlMgr) // (CSystemConfigMgr *pSystemConfigMgr, CIOMapControlMgr *pIOMapControlMgr, CMotorControlMgr *pMotorControlMgr)
{
	m_pSystemConfigMgr = pSystemConfigMgr;
	m_pMaintControlMgr = pMaintControlMgr;
//	m_pIOMapControlMgr = pIOMapControlMgr;
//	m_pMotorControlMgr = pMotorControlMgr;
}

void CAlarmMonitorToSensorIsolater::SetStart()
{
	SetEvent(m_hStartEvent);
}

void CAlarmMonitorToSensorIsolater::SetStop()
{
	ResetEvent(m_hStartEvent);
}