//#include "stdafx.h"
#include "pch.h"
#include "BaseSequenceImpl.h"

#include "LogFunction\LogAccessor\LogFileAccessor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CBaseSequenceImpl::CBaseSequenceImpl(stAuroraMgrPool &MgrPool, int iSeqItemNumber)
	: CBaseSequenceBody(MgrPool)
{
	m_iSeqItemNumber = iSeqItemNumber;

	//----------------------------------------------------------------------
    // Thread 생성
    //----------------------------------------------------------------------
    m_bThreadLive	 = false;
    m_bIsThreadRun = false;
    m_pThread		 = AfxBeginThread(ThreadRun, this);
    m_hStartEvent	 = CreateEvent(NULL, FALSE, FALSE, NULL);	// auto reset, initial reset
    m_hCloseEvent	 = CreateEvent(NULL, FALSE, FALSE, NULL);	// auto reset, initial reset
	m_hWaitTimeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);	// auto reset, initial reset
    m_hCompleteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);	// auto reset, initial reset
    m_hMultiEvent[0] = m_hStartEvent;
	m_hMultiEvent[1] = m_hCloseEvent;

	m_bFail = false;
//	m_bComplete = false;
    m_bComplete = true; // 2023.08.09 change
	m_bAlarmOn = false;
}

CBaseSequenceImpl::~CBaseSequenceImpl()
{
    //----------------------------------------------------------------------
    // Thread 종료
    //----------------------------------------------------------------------
    ResetEvent(m_hWaitTimeEvent);
    ResetEvent(m_hCompleteEvent);
    ResetEvent(m_hStartEvent);
    SetEvent(m_hCloseEvent);
    if (m_pThread && m_bThreadLive)
    {
        WaitForSingleObject(m_pThread->m_hThread, INFINITE);
    }
    if(m_hStartEvent)CloseHandle(m_hStartEvent);
    if(m_hCloseEvent)CloseHandle(m_hCloseEvent);	
	if(m_hWaitTimeEvent) CloseHandle(m_hWaitTimeEvent);
	if(m_hCompleteEvent) CloseHandle(m_hCompleteEvent);
}

//=========================================================================
// Thread Run
//=========================================================================
UINT CBaseSequenceImpl::ThreadRun(LPVOID p1)
{
    CBaseSequenceImpl *pThis = (CBaseSequenceImpl *)p1;

    pThis->m_bThreadLive = true;

    while (1)
    {
        DWORD dw = WaitForMultipleObjects(2, pThis->m_hMultiEvent, FALSE, INFINITE);

        if (dw == WAIT_OBJECT_0) // Start Event 가 set될 때까지 기다린다.
        {
            pThis->m_bIsThreadRun = true;

            pThis->ThreadRun();

            pThis->m_bIsThreadRun = false;
        }
        else if (dw == (WAIT_OBJECT_0 + 1)) // Close Event 가 set될 때까지 기다린다.
        {
            break;
        }
        else // timeout, error state
        {
        }

        
    }

    pThis->m_bThreadLive = false;

    return 0;
}


//=========================================================================
// INTERFACE 관련
//=========================================================================
// Link DataBase
void CBaseSequenceImpl::UpdateDataBaseInformation()
{
}

void CBaseSequenceImpl::UpdateLinkParamInformation(CCommTcpIPMgr *pCommTcpIPMgr)
{
	m_MgrPool_link.pCommTcpIPMgr = pCommTcpIPMgr;
}

// Thread 관련
void CBaseSequenceImpl::ThreadRun()
{
}

void CBaseSequenceImpl::Start()
{
    GoToInitStep();

    ResetEvent(m_hCompleteEvent);
    SetEvent(m_hStartEvent);
}

void CBaseSequenceImpl::Stop()
{
    SetStopUser(true);

    ResetEvent(m_hStartEvent);
    SetEvent(m_hCompleteEvent);

	GoToInitStep();
}

void CBaseSequenceImpl::Skip()
{
	GoToExitStep();

    ResetEvent(m_hCompleteEvent);
    SetEvent(m_hStartEvent);
}

void CBaseSequenceImpl::Exit()
{
	GoToExitStep();

    ResetEvent(m_hCompleteEvent);
    SetEvent(m_hStartEvent);
}

void CBaseSequenceImpl::Pause()
{
    if (m_pThread)
    {
        m_pThread->SuspendThread();
    }
}

void CBaseSequenceImpl::Resume()
{
    if (m_pThread)
    {
        m_pThread->ResumeThread();
    }
}

bool CBaseSequenceImpl::Is_Complete()
{
    return m_bComplete;
}

bool CBaseSequenceImpl::Is_Fail()
{
	return m_bFail;
}

bool CBaseSequenceImpl::Is_AlarmOn()
{
	return m_bAlarmOn;
}

bool CBaseSequenceImpl::Is_ThreadRun()
{
    return m_bIsThreadRun;
}

