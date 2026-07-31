#include "pch.h"
#include "AlarmMonitorToSensorVacSwitch.h"

#include "..\Manager\SystemConfigMgr.h"
#include "..\Manager\IOMapControlMgr.h"
#include "..\Manager\IOMapAddressDefine.h"
#include "AlarmController.h"
#include "..\SystemParamShare.h"
#include "..\AuroraFuncDefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CAlarmMonitorToSensorVacSwitch::CAlarmMonitorToSensorVacSwitch(CString strLogPath)
	: m_pLogFileAccessor(new CLogFileAccessor)
{
	const CString strFolderPath = strLogPath + _T("\\Event\\");
	const CString strLogFile = strFolderPath + _T("sensor_VaccumSwitch.log");
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


CAlarmMonitorToSensorVacSwitch::~CAlarmMonitorToSensorVacSwitch()
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

UINT CAlarmMonitorToSensorVacSwitch::ThreadRun(LPVOID p1)
{
	CAlarmMonitorToSensorVacSwitch *pThis = (CAlarmMonitorToSensorVacSwitch *)p1;

	pThis->m_bThreadLive = true;

	while (1)
	{
		// Event 가 set될 때까지 기다린다.
		if (WaitForSingleObject(pThis->m_hStartEvent, 10) == WAIT_OBJECT_0)
		{
			// 설비의 FAN 이 멈춘거는 IO 체크를 천천히 해도 될거 같다.
			//Sleep(100);
			Sleep(200);	// jsyoon 20211020 change

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

UINT CAlarmMonitorToSensorVacSwitch::ThreadRun()
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
	//Vaccum Switch
//	if (m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_012) == true)	// MUTING SWITCH On
	// jsyoon 20210928 change
	if (m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_010) == true)	// MUTING STATUS
	{
		if (m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_029) == true) // MR0 POSITION I/O
		{
			if (m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_005) != true) // Vac Switch On
			{
				if (m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_109) == true)	// Vac On
				{
					m_pIOMapControlMgr->SetStageVaccumOff();
				}
				else
				{
					m_pIOMapControlMgr->SetStageVaccumOn();
				}

				Sleep(1000);
			}
		}
	}
#endif // _USE_IOBOARD


	return 0;
}

void CAlarmMonitorToSensorVacSwitch::SetPara(CSystemConfigMgr *pSystemConfigMgr, CIOMapControlMgr *pIOMapControlMgr, CCommTcpIPMgr *pCommTcpIPMgr)
{
	m_pSystemConfigMgr = pSystemConfigMgr;
	m_pIOMapControlMgr = pIOMapControlMgr;
	m_pCommTcpIPMgr = pCommTcpIPMgr;
}

void CAlarmMonitorToSensorVacSwitch::SetStart()
{
	SetEvent(m_hStartEvent);
}

void CAlarmMonitorToSensorVacSwitch::SetStop()
{
	ResetEvent(m_hStartEvent);
}