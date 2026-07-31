#include "pch.h"
#include "AlarmMonitorToTeachKEY.h"

#include "..\Manager\SystemConfigMgr.h"
#include "..\Manager\LogInControlMgr.h"
#include "..\Manager\MaintControlMgr.h"
#include "..\Manager\IOMapAddressDefine.h"
//#include "CommTcpIPMgr.h"
#include "AlarmController.h"
#include "..\SystemParamShare.h"
#include "..\AuroraFuncDefine.h"
#include "LogFunction\LogAccessor\LogFileAccessor.h"
#include "..\D_PM_MODE.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CAlarmMonitorToTeachKEY::CAlarmMonitorToTeachKEY(stAuroraMgrPool &MgrPool)
    : CAlarmMonitorImpl(MgrPool)
	, m_pLogFileAccessor(new CLogFileAccessor)
{
    const CString strFolderPath	= MgrPool.strLogPath + _T("\\Event\\");
    const CString strLogFile	= strFolderPath + _T("teach_key.log");
    MakeDir(strFolderPath);
    m_pLogFileAccessor->Initialize(strLogFile);

	//----------------------------------------------------------------------
    // Thread 생성
    //----------------------------------------------------------------------
    m_pThread		= AfxBeginThread(ThreadRun, this);
    m_bThreadLive	= false;
    m_hStartEvent	= CreateEvent(NULL, TRUE, FALSE, NULL);		// manual set, initial reset
    m_hCloseEvent	= CreateEvent(NULL, FALSE, FALSE, NULL);	// auto reset, initial reset
}

CAlarmMonitorToTeachKEY::~CAlarmMonitorToTeachKEY()
{
    //----------------------------------------------------------------------
    // Thread 종료
    //----------------------------------------------------------------------
    ResetEvent(m_hStartEvent);
    SetEvent(m_hCloseEvent);
    if(m_pThread && m_bThreadLive)
    {
        WaitForSingleObject(m_pThread->m_hThread, INFINITE);
    }
    if(m_hStartEvent)CloseHandle(m_hStartEvent);
    if(m_hCloseEvent)CloseHandle(m_hCloseEvent);

	//----------------------------------------------------------------------
    // Data 삭제
    //----------------------------------------------------------------------
    if(m_pLogFileAccessor)
    {
        delete m_pLogFileAccessor;
        m_pLogFileAccessor = nullptr;
    }
}

UINT CAlarmMonitorToTeachKEY::ThreadRun(LPVOID p1)
{
    CAlarmMonitorToTeachKEY *pThis = (CAlarmMonitorToTeachKEY *)p1;

    pThis->m_bThreadLive = true;

    while (1)
    {
        // Event 가 set될 때까지 기다린다.
        if (WaitForSingleObject(pThis->m_hStartEvent, 10) == WAIT_OBJECT_0)
        {
            // 설비를 PM 모드로 변경하는거는 IO 체크를 천천히 해도 될거 같다.
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

UINT CAlarmMonitorToTeachKEY::ThreadRun()
{
    //***********************************************************************************************************************
    //***********************************************************************************************************************
	// 설비 알람 사용 유/무
    //***********************************************************************************************************************
    //***********************************************************************************************************************
	if(m_pSystemConfigMgr->IsUseTeachKey() == false)
        return 0;

	///
	// 연결 유무확인
	//
    if (m_pIOMapControlMgr->IsConnected() != true)
    { 
        return 0;
    }
	
    //--------------------------------------------------------
    // 키박스를 AUTO->TEACH 로 변경했다. PM MODE 발생
    //--------------------------------------------------------

#ifdef _USE_IOBOARD
    if(m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_102) == false)
    {
		m_pIOMapControlMgr->SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_016, true);  // ACS Interlock

        // cycle mode 를 진행할 수 없다.
        CSystemParamShare::GetInstance()->SetCycleState(false);
	
		//m_pCommTcpIPMgr->SetAlarmOccurBecauseStop();	// 특수한 경우 - 모듈 쪽으로 ALARM 을 전달해준다.
	
        m_pLogFileAccessor->KMLOG(SYST, _T("SELECT SWITCH [ AUTO -> TEACH ] MODE"));
        CD_PM_MODE dlg2(m_pIOMapControlMgr, m_pLogInControlMgr);
        dlg2.DoModal();
    }
	else
	{
		if (m_pIOMapControlMgr->GetParam_DIOMapMemoryBitExport(IO_BOARD_READ_OUTPUT_016) == true)
			m_pIOMapControlMgr->SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_016, false); // ACS Interlock
	}
#else

    if (m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(PLC_BIT_RUN_TEACH_SWITCH) == false)
    {
        // cycle mode 를 진행할 수 없다.
        CSystemParamShare::GetInstance()->SetCycleState(false);

        //m_pCommTcpIPMgr->SetAlarmOccurBecauseStop();	// 특수한 경우 - 모듈 쪽으로 ALARM 을 전달해준다.

        m_pLogFileAccessor->KMLOG(SYST, _T("SELECT SWITCH [ AUTO -> TEACH ] MODE"));
        CD_PM_MODE dlg2(m_pMaintControlMgr, m_pLogInControlMgr);
        dlg2.DoModal(); //__TEST

		m_pLogFileAccessor->KMLOG(SYST, _T("SELECT SWITCH [ TEACH -> AUTO ] MODE"));
    }

#endif // _USE_IOBOARD

    return 0;
}


void CAlarmMonitorToTeachKEY::SetStart()
{
    SetEvent(m_hStartEvent);
}

void CAlarmMonitorToTeachKEY::SetStop()
{
    ResetEvent(m_hStartEvent);
}
