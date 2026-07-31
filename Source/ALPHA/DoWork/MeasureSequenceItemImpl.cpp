#include "pch.h"
#include "MeasureSequenceItemImpl.h"

//#include "AlarmController.h"
#include "..\SystemParamShare.h"
#include "..\Manager\SystemConfigMgr.h"
#include "..\Recipe\RecipeData.h"
//#include "ResultCollectMgr.h"
#include "CommEthFunction\CommEthFunction.h"
#include "..\AuroraFuncDefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CMeasureSequenceItemImpl::CMeasureSequenceItemImpl(stAuroraMgrPool &MgrPool, int nIndex)
    : CBaseSequenceBody(MgrPool)
	, m_pRecipeDataClone(new CRecipeData)
    , m_pCommTcpIpObject(new CCommEthFunction)
{
    //----------------------------------------------------------------------
    // Thread 생성
    //----------------------------------------------------------------------
    m_bThreadLive		= false;
    m_pThread			= AfxBeginThread(ThreadRun, this);
    m_hStartEvent		= CreateEvent(NULL, FALSE, FALSE, NULL);	// manual set, initial reset
    m_hCloseEvent		= CreateEvent(NULL, FALSE, FALSE, NULL);	// auto reset, initial reset
	m_hWaitTimeEvent	= CreateEvent(NULL, FALSE, FALSE, NULL);	// auto reset, initial reset
    m_hCompleteEvent	= CreateEvent(NULL, FALSE, FALSE, NULL);	// auto reset, initial reset
    m_hMultiEvent[0] = m_hStartEvent;
	m_hMultiEvent[1] = m_hCloseEvent;
}

CMeasureSequenceItemImpl::~CMeasureSequenceItemImpl()
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

	//----------------------------------------------------------------------
    // Data 삭제
    //----------------------------------------------------------------------
    if(m_pRecipeDataClone)
    {
        delete m_pRecipeDataClone;
        m_pRecipeDataClone = nullptr;
    }

	if(m_pCommTcpIpObject)
	{
		delete m_pCommTcpIpObject;
		m_pCommTcpIpObject = nullptr;
	}
}

//=========================================================================
// Thread Run
//=========================================================================
UINT CMeasureSequenceItemImpl::ThreadRun(LPVOID p1)
{
    CMeasureSequenceItemImpl *pThis = (CMeasureSequenceItemImpl *)p1;

    pThis->m_bThreadLive = true;

    while (1)
    {
        DWORD dw = WaitForMultipleObjects(2, pThis->m_hMultiEvent, FALSE, INFINITE);

        if (dw == WAIT_OBJECT_0) // Start Event 가 set될 때까지 기다린다.
        {
            pThis->ThreadRun();
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
void CMeasureSequenceItemImpl::Start()
{
    ResetEvent(m_hCompleteEvent);
    SetEvent(m_hStartEvent);
}

void CMeasureSequenceItemImpl::Stop()
{
    ResetEvent(m_hStartEvent);
    SetEvent(m_hCompleteEvent);
}

void CMeasureSequenceItemImpl::Skip()
{
    ResetEvent(m_hCompleteEvent);
    SetEvent(m_hStartEvent);
}

void CMeasureSequenceItemImpl::Exit()
{
    ResetEvent(m_hCompleteEvent);
    SetEvent(m_hStartEvent);
}

void CMeasureSequenceItemImpl::Pause()
{
    if (m_pThread)
    {
        m_pThread->SuspendThread();
    }
}

void CMeasureSequenceItemImpl::Resume()
{
    if (m_pThread)
    {
        m_pThread->ResumeThread();
    }
}

bool CMeasureSequenceItemImpl::Is_Complete()
{
    return m_bComplete;
}
