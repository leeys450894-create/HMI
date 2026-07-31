#include "pch.h"
#include "BaseSequenceBody.h"

#include "LogFunction\LogAccessor\LogFileAccessor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CBaseSequenceBody::CBaseSequenceBody(stAuroraMgrPool &MgrPool)
    : m_MgrPool_link(MgrPool)
	, m_pLogFileAccessor(new CLogFileAccessor)
    , m_strSeqType(_T(""))
    , m_strSeqCorp(_T(""))
    , m_strSeqName(_T(""))
    , m_strSeqGUID(_T(""))
    , m_bVisible(true)
    , m_bComplete(true)
    , m_nFailNum(0)
{
    // sequence index
    m_iStepIndex		= 0;
    m_iStepIndex_backup	= 0;
    m_iStepIndex_exit	= 0;
    m_iStepIndex_total	= 0;

    m_hEvtUserPause = CreateEvent(NULL, TRUE, FALSE, NULL);   // MANUAL reset, initially reset
    m_hEvtUserResume = CreateEvent(NULL, FALSE, FALSE, NULL); // auto reset, initially reset
}

CBaseSequenceBody::~CBaseSequenceBody()
{
    CloseHandle(m_hEvtUserPause);
    CloseHandle(m_hEvtUserResume);

	//----------------------------------------------------------------------
    // Data ªË¡¶
    //----------------------------------------------------------------------
    if(m_pLogFileAccessor)
    {
        delete m_pLogFileAccessor;
        m_pLogFileAccessor = nullptr;
    }
}

bool CBaseSequenceBody::IfPauseEvt_WaitResumeEvt() const
{
    bool bRet = true;
    if (WaitForSingleObject(m_hEvtUserPause, 0)
        == WAIT_OBJECT_0)
    {
        if (WaitForSingleObject(m_hEvtUserResume, INFINITE)
            == WAIT_OBJECT_0)
        {
            //
        }
    }

    if (GetStopUser() == true)
    {
        bRet = false;
    }

    return bRet;
}