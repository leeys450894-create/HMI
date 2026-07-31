#include "pch.h"
#include "AlarmMonitorToMcOFFState.h"

#include "..\Manager\SystemConfigMgr.h"
#include "..\Manager\IOMapControlMgr.h"
#include "..\Manager\IOMapAddressDefine.h"
#include "AlarmController.h"
#include "..\AuroraFuncDefine.h"
#include "LogFunction\LogAccessor\LogFileAccessor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CAlarmMonitorToMcOFFState::CAlarmMonitorToMcOFFState(CString strLogPath, int iMonitorIONumber, int iEMOPosST, int iEMOPosED, CString strLogMsg, int iAlarmID)
    : m_iMonitorIONumber(iMonitorIONumber)
    , m_iEMOPosST(iEMOPosST)
    , m_iEMOPosED(iEMOPosED)
    , m_strLogMsg(strLogMsg)
    , m_iAlarmID(iAlarmID)
	, m_pLogFileAccessor(new CLogFileAccessor)
{
    const CString strFolderPath	= strLogPath + _T("\\Event\\");
    CString strLogFile;
	strLogFile.Format(_T("%s%s_%02d.log"), strFolderPath, _T("mc_off_state"), iMonitorIONumber);
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

CAlarmMonitorToMcOFFState::~CAlarmMonitorToMcOFFState()
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

UINT CAlarmMonitorToMcOFFState::ThreadRun(LPVOID p1)
{
    CAlarmMonitorToMcOFFState *pThis = (CAlarmMonitorToMcOFFState *)p1;

    pThis->m_bThreadLive = true;

    while (1)
    {
        // Event 가 set될 때까지 기다린다.
        if (WaitForSingleObject(pThis->m_hStartEvent, 10) == WAIT_OBJECT_0)
        {
            // MC 가 내려간거는 IO 체크를 천천히 해도 될거 같다.
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

UINT CAlarmMonitorToMcOFFState::ThreadRun()
{
    //***********************************************************************************************************************
    //***********************************************************************************************************************
	// 설비 알람 사용 유/무
    //***********************************************************************************************************************
    //***********************************************************************************************************************
	if(m_pSystemConfigMgr->IsUseAlarmMonitor() == false)
        return 0;

	//--------------------------------------------------------
	// 센서로 연결되어 있는 부분
	//--------------------------------------------------------
	if (m_pSystemConfigMgr->IsUseHardwareSensor() == false)
		return 0;

	///
	// 연결 유무확인
	//
	if (m_pIOMapControlMgr->IsConnected() != true)
		return 0;
	
	//--------------------------------------------------------
	// User 가 MC OFF 가 발생한 경우
    //--------------------------------------------------------
	int nRet = 0;
	nRet = m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(m_iMonitorIONumber);
	if (nRet != -1)
	{
		if (nRet == 0)
		{
			m_pLogFileAccessor->KMLOG(WARN, m_strLogMsg);
			CAlarmController::GetInstance()->SetAlarm(m_iAlarmID);
		}
	}
	
	
	return 0;
}

void CAlarmMonitorToMcOFFState::SetPara(CSystemConfigMgr *pSystemConfigMgr, CIOMapControlMgr *pIOMapControlMgr, CCommTcpIPMgr *pCommTcpIPMgr)
{
    m_pSystemConfigMgr = pSystemConfigMgr;
    m_pIOMapControlMgr = pIOMapControlMgr;
    m_pCommTcpIPMgr = pCommTcpIPMgr;
}

void CAlarmMonitorToMcOFFState::SetStart()
{
    SetEvent(m_hStartEvent);
}

void CAlarmMonitorToMcOFFState::SetStop()
{
    ResetEvent(m_hStartEvent);
}