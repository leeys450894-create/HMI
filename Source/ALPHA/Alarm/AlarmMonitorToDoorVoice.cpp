#include "pch.h"
#include "AlarmMonitorToDoorVoice.h"

#include "..\Manager\SystemConfigMgr.h"
#include "..\Manager\LogInControlMgr.h"
#include "..\Manager\IOMapControlMgr.h"
#include "..\Manager\IOMapAddressDefine.h"
#include "..\AuroraFuncDefine.h"
#include "LogFunction\LogAccessor\LogFileAccessor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CAlarmMonitorToDoorVoice::CAlarmMonitorToDoorVoice(stAuroraMgrPool &MgrPool)
	: CAlarmMonitorImpl(MgrPool)
	, m_pLogFileAccessor(new CLogFileAccessor)
{
	const CString strFolderPath = MgrPool.strLogPath + _T("\\Event\\");
	const CString strLogFile = strFolderPath + _T("DoorVoice.log");
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


CAlarmMonitorToDoorVoice::~CAlarmMonitorToDoorVoice()
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

UINT CAlarmMonitorToDoorVoice::ThreadRun(LPVOID p1)
{
	CAlarmMonitorToDoorVoice *pThis = (CAlarmMonitorToDoorVoice *)p1;

	pThis->m_bThreadLive = true;

	while (1)
	{
		// Event 가 set될 때까지 기다린다.
		if (WaitForSingleObject(pThis->m_hStartEvent, 10) == WAIT_OBJECT_0)
		{
			// 설비를 PM 모드로 변경하는거는 IO 체크를 천천히 해도 될거 같다.
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

UINT CAlarmMonitorToDoorVoice::ThreadRun()
{
	//***********************************************************************************************************************
	//***********************************************************************************************************************
	// 설비 알람 사용 유/무
	//***********************************************************************************************************************
	//***********************************************************************************************************************
	if (m_pSystemConfigMgr->IsUseTeachKey() == false)
		return 0;

	///
	// 연결 유무확인
	//
	if (m_pIOMapControlMgr->IsConnected() != true)
		return 0;


#ifdef _USE_IOBOARD
	// jsyoon 20210928 add
	// DOOR Sound
	bool bDoorSount = true;
	// Door1 Open
	if (m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_016) == true)
	{
		// Door Key Inside insert 
		if (m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_030) == true)
		{
			bDoorSount = false;
		}
	}
	else // Door Close
	{
		bDoorSount = false;
	}

	if (bDoorSount == true)
	{
		if (m_pIOMapControlMgr->GetParam_DIOMapMemoryBitExport(IO_BOARD_READ_OUTPUT_001) != true)
			m_pIOMapControlMgr->SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_001, true);
	}
	else
	{
		if (m_pIOMapControlMgr->GetParam_DIOMapMemoryBitExport(IO_BOARD_READ_OUTPUT_001) == true)
			m_pIOMapControlMgr->SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_001, false);
	}

	// Door2 Open
	bDoorSount = true;
	if (m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_017) == true)
	{
		// Door Key Inside insert 
		if (m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_031) == true)
		{
			bDoorSount = false;
		}
	}
	else // Door Close
	{
		bDoorSount = false;
	}

	if (bDoorSount == true)
	{
		if (m_pIOMapControlMgr->GetParam_DIOMapMemoryBitExport(IO_BOARD_READ_OUTPUT_001) != true)
			m_pIOMapControlMgr->SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_001, true);
	}
	else
	{
		if (m_pIOMapControlMgr->GetParam_DIOMapMemoryBitExport(IO_BOARD_READ_OUTPUT_001) == true)
			m_pIOMapControlMgr->SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_001, false);
	}
#endif // _USE_IOBOARD

	return 0;
}


void CAlarmMonitorToDoorVoice::SetStart()
{
	SetEvent(m_hStartEvent);
}

void CAlarmMonitorToDoorVoice::SetStop()
{
	ResetEvent(m_hStartEvent);
}