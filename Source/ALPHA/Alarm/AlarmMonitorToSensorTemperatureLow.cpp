#include "pch.h"
#include "AlarmMonitorToSensorTemperatureLow.h"

#include "..\Manager\SystemConfigMgr.h"
#include "..\Manager\IOMapAddressDefine.h"
#include "AlarmController.h"
#include "..\SystemParamShare.h"
#include "..\AuroraFuncDefine.h"
#include "LogFunction\LogAccessor\LogFileAccessor.h"
#include "..\Manager\MaintControlMgr.h"
#include "..\Manager\ShareMemoryMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CAlarmMonitorToSensorTemperatureLow::CAlarmMonitorToSensorTemperatureLow(CString strLogPath, int iMonitorIONumber, int iEMOPosST, int iEMOPosED, CString strLogMsg, int iAlarmID)
	: m_iMonitorIONumber(iMonitorIONumber)
	, m_iEMOPosST(iEMOPosST)
	, m_iEMOPosED(iEMOPosED)
	, m_strLogMsg(strLogMsg)
	, m_iAlarmID(iAlarmID)
	, m_pLogFileAccessor(new CLogFileAccessor)
	, m_bCloseSystem(false)
	, m_bSetAlarm(false)
	, m_nResetCnt(0)
{
	const CString strFolderPath = strLogPath + _T("\\Event\\");
	const CString strLogFile = strFolderPath + _T("sensor_temperature.log");
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


CAlarmMonitorToSensorTemperatureLow::~CAlarmMonitorToSensorTemperatureLow()
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

UINT CAlarmMonitorToSensorTemperatureLow::ThreadRun(LPVOID p1)
{
	CAlarmMonitorToSensorTemperatureLow* pThis = (CAlarmMonitorToSensorTemperatureLow*)p1;

	pThis->m_bThreadLive = true;

	while (1)
	{
		// Event 가 set될 때까지 기다린다.
		if (WaitForSingleObject(pThis->m_hStartEvent, 10) == WAIT_OBJECT_0)
		{
			// 설비의 온도가 변화된거는 IO 체크를 천천히 해도 될거 같다.
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

UINT CAlarmMonitorToSensorTemperatureLow::ThreadRun()
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


	//--------------------------------------------------------
	//	// 설비 온도가 너무 높다. 경 알람 발생 - 40도 이상이다.
	//--------------------------------------------------------
	if (m_pMaintControlMgr->GetShareMemoryMgrPtr()->GetSys_TemperOver(m_iMonitorIONumber) == true)
	{
		m_bSetAlarm = true;
	}
	
	int nRet = 0;
	nRet = m_pMaintControlMgr->GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(m_iEMOPosST);
	if (nRet != -1)
	{
		if (nRet == 1)
		{
			m_nResetCnt++;
		}
	}

	if (m_nResetCnt > 2)  // 3sec
	{
		m_bSetAlarm = false;
		m_nResetCnt = 0;
	}

///
// 연결 유무확인
//
	//if (m_pMaintControlMgr->GetIOMapControlMgrPtr()->IsConnected() != true)
	//	return 0;

	if (m_bSetAlarm == true)
	{
		m_pLogFileAccessor->KMLOG(WARN, m_strLogMsg);
		CAlarmController::GetInstance()->SetAlarm(m_iAlarmID);
	}

	//	if (m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_105) == false)
	//	{
	//		m_pLogFileAccessor->KMLOG(WARN, _T("PC. BOX TEMP. OVER WARNING (+3℃)"));
	//		CAlarmController::GetInstance()->SetAlarm(PLC_TEMPERATURE_SENSOR_OVER_03);
	//	}
	//
	//	if(m_bCloseSystem == true)
	//		return 0;
	//
	//	//--------------------------------------------------------
	//	// 설비 온도가 너무 높다. 중 알람 발생 - 40도 이상이다. 설비를 멈춘다.
	//    //--------------------------------------------------------
	//	if(m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_104) == false)
	//	{
	//		// Y0012	INTERLOCK S.R ON (TEMPERATURE OVER ALARM) 비트를 켜야 MC 가 꺼진다.
	//		m_pIOMapControlMgr->SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_011, true);
	//	
	//	
	//		m_pLogFileAccessor->KMLOG(WARN, _T("FRONT EL. BOX TEMP. OVER ALARM (40℃)"));
	//		CAlarmController::GetInstance()->SetAlarm(PLC_TEMPERATURE_SENSOR_OVER_02);
	//	
	//		if(m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_104) == true)
	//		{
	//			// Y0012	INTERLOCK S.R ON (TEMPERATURE OVER ALARM) 비트를 꺼야 MC 가 켜진다.
	//			m_pIOMapControlMgr->SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_011, false);
	//		}
	//	}
	//
	//	if (m_bCloseSystem == true)
	//		return 0;
	//
	//	//--------------------------------------------------------
	//	// PC 온도가 너무 높다. 중 알람 발생 - 40도 이상이다. 설비를 멈춘다.
	//	//--------------------------------------------------------
	//	if (m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_106) == false)
	//	{
	//		// Y0012	INTERLOCK S.R ON (TEMPERATURE OVER ALARM) 비트를 켜야 MC 가 꺼진다.
	//		m_pIOMapControlMgr->SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_011, true);
	//
	//
	//		m_pLogFileAccessor->KMLOG(WARN, _T("PC. BOX TEMP. OVER ALARM (40℃)"));
	//		CAlarmController::GetInstance()->SetAlarm(PLC_TEMPERATURE_SENSOR_OVER_04);
	//
	//		if (m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_106) == true)
	//		{
	//			// Y0012	INTERLOCK S.R ON (TEMPERATURE OVER ALARM) 비트를 꺼야 MC 가 켜진다.
	//			m_pIOMapControlMgr->SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_011, false);
	//		}
	//	}

	return 0;
}

void CAlarmMonitorToSensorTemperatureLow::SetPara(CSystemConfigMgr* pSystemConfigMgr, CMaintControlMgr* pMaintControlMgr)
{
	m_pSystemConfigMgr = pSystemConfigMgr;
	m_pMaintControlMgr = pMaintControlMgr;
}

void CAlarmMonitorToSensorTemperatureLow::SetStart()
{
	SetEvent(m_hStartEvent);
}

void CAlarmMonitorToSensorTemperatureLow::SetStop()
{
	m_bCloseSystem = true;
	ResetEvent(m_hStartEvent);
}