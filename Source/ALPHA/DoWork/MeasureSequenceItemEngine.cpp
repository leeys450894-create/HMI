#include "pch.h"
#include "MeasureSequenceItemEngine.h"

// LYS 2026-08-12 InnerID uniqueness: rolling window of recently issued ids.
#define DEFINE_INNERID_KEEP_MAX	512


#include "..\Recipe\RecipeEditMgr.h"
#include "..\Manager\CommTcpIPMgr.h"
#include "MeasureSequenceItemFactory.h"
#include "MeasureSequenceItemImpl.h"
#include "..\SystemParamShare.h"
#include "..\Manager\SeqTactTimeMgr.h"
#include "..\AuroraFuncDefine.h"

//#include "D_REQ_LoadingSkip.h"
#include "common\BinaryHelper.h"
#include "LogFunction\LogAccessor\LogFileAccessor.h"

#include "common\IniHelper.h"
#include "..\Manager\NetworkConfigMgr.h"
#include "..\Manager\SystemConfigMgr.h"	// LYS 2026-08-06 AgingCycle E3

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CMeasureSequenceItemEngine::CMeasureSequenceItemEngine(stAuroraMgrPool &MgrPool)
    : m_MgrPool_link(MgrPool)
	, m_pItemFactory(new CMeasureSequenceItemFactory(MgrPool))
    , m_bEngineBreak(false)
	, m_pLogFileAccessor(new CLogFileAccessor)
    , m_pStInnerID(nullptr)
    , m_nMeasureArea(0)
	, m_bAgingCycleActive(false)	// LYS 2026-08-06 AgingCycle E3
	, m_nAgingCurItem(-1)			// LYS 2026-08-06 AgingCycle E3
	, m_bEfaCycle(false)				// LYS 2026-09-28 EFA
	, m_nAgingGlassLatch(-1)			// LYS 2026-09-29 Hybrid 3b A4
	, m_nAgingMaskChecked(0)			// LYS 2026-09-30
	, m_nAgingStick(-1)				// LYS 2026-09-30
	, m_bAgingStickSkip(false)		// LYS 2026-09-30
	, m_bSamplingCycleSkip(false)	// LYS 2026-10-08 Sampling stick skip
{
	ZeroMemory(m_nAgingRepeat, sizeof(m_nAgingRepeat));	// LYS 2026-08-12 AgingCycle R1

    const CString strFolderPath	= MgrPool.strLogPath + _T("\\LogFile\\");
    const CString strLogFile	= strFolderPath + _T("measure_seq_item_engine.log");
    MakeDir(strFolderPath);
    m_pLogFileAccessor->Initialize(strLogFile);

    //----------------------------------------------------------------------
    // Thread 생성
    //----------------------------------------------------------------------
    m_bThreadLive	= false;
    m_pThread		= AfxBeginThread(ThreadRun, this);
    m_hStartEvent	= CreateEvent(NULL, FALSE, FALSE, NULL);	// auto reset, initial reset
	m_hExitsEvent	= CreateEvent(NULL, FALSE, FALSE, NULL);	// auto reset, initial reset
    m_hCloseEvent	= CreateEvent(NULL, FALSE, FALSE, NULL);	// auto reset, initial reset
    m_hWaitsEvent	= CreateEvent(NULL, FALSE, FALSE, NULL);	// auto reset, initial reset
    m_hBreakEvent_link = NULL;	// 연동용 - 생성/삭제 하지 않아야 한다.
    m_hMultiEvent[0] = m_hStartEvent;
	m_hMultiEvent[1] = m_hExitsEvent;
    m_hMultiEvent[2] = m_hCloseEvent;

	m_nHead = 0;
}

CMeasureSequenceItemEngine::~CMeasureSequenceItemEngine()
{
    //----------------------------------------------------------------------
    // Thread 종료
    //----------------------------------------------------------------------
    ResetEvent(m_hStartEvent);
    ResetEvent(m_hExitsEvent);
    SetEvent(m_hCloseEvent);
    ResetEvent(m_hWaitsEvent);
    if (m_pThread && m_bThreadLive)
    {
        WaitForSingleObject(m_pThread->m_hThread, INFINITE);
    }
    if(m_hStartEvent)CloseHandle(m_hStartEvent);
    if(m_hExitsEvent)CloseHandle(m_hExitsEvent);
    if(m_hCloseEvent)CloseHandle(m_hCloseEvent);
    if(m_hWaitsEvent)CloseHandle(m_hWaitsEvent);

    //----------------------------------------------------------------------
    // Data 삭제
    //----------------------------------------------------------------------
    ItemFinalize();

    if(m_pItemFactory)
    {
        delete m_pItemFactory;
        m_pItemFactory = nullptr;
    }

	if(m_pLogFileAccessor)
    {
        delete m_pLogFileAccessor;
        m_pLogFileAccessor = nullptr;
    }

}

bool CMeasureSequenceItemEngine::LinkDataBase(CBinaryHelper *pBinaryHelper, bool bSave)
{
	CString str = pBinaryHelper->GetFilePath();

    //-----------------------------------------------------------------------
    // Item Data
    //-----------------------------------------------------------------------
    int iCount = (int)m_vecItems.size();

    if (bSave)
    {
        pBinaryHelper->Link(bSave, _T("SEQUENCE_ITEM"), _T("COUNT"), &iCount, 0);
    }
    else
    {
        pBinaryHelper->Link(bSave, _T("SEQUENCE_ITEM"), _T("COUNT"), &iCount, 0);

        ItemFinalize();

        m_vecItems.clear();
        m_vecItems.resize(iCount);
    }

    // module create
    for (int i = 0; i < iCount; ++i)
    {
        CString strText;
        strText.Format(_T("SEQ_ITEM_%02d_PARA_"), i);

        CString strType, strName, strGUID;
        if (bSave == true)
        {
            strType	= m_vecItems[i]->GetType();
            strName	= m_vecItems[i]->GetName();
            strGUID	= m_vecItems[i]->GetGUID();
        }

        pBinaryHelper->Link(bSave, _T("SEQUENCE_ITEM"), strText + _T("TYPE"),	&strType,	(CString)_T(""));
        pBinaryHelper->Link(bSave, _T("SEQUENCE_ITEM"), strText + _T("NAME"),	&strName,	(CString)_T(""));
        pBinaryHelper->Link(bSave, _T("SEQUENCE_ITEM"), strText + _T("GUID"),	&strGUID,	(CString)_T(""));

        if (bSave == false)
        {
            m_vecItems[i] = m_pItemFactory->GetItem(strType, strName);
        }
    }


	ItemFinalize();
	m_vecItems.clear();
	m_vecItems.resize(m_MgrPool_link.pNetworkConfigMgr->GetCount());

	for (int i = 0; i < m_MgrPool_link.pNetworkConfigMgr->GetCount(); ++i)
	{
		CString strName = m_MgrPool_link.pNetworkConfigMgr->GetParam_Name(i);
		m_vecItems[i] = m_pItemFactory->GetItem(strName, strName);
	}

    //-----------------------------------------------------------------------
    // Module PARAMETER
    //-----------------------------------------------------------------------
    bool bRet = pBinaryHelper->IsReadEnable();

    for (size_t i = 0; i < m_vecItems.size(); ++i)
    {
        if (m_vecItems[i])
        {
            bRet &= m_vecItems[i]->LinkDataBase(pBinaryHelper, bSave);
            bRet &= m_vecItems[i]->UpdateState();
        }
    }

    return bRet;
}

void CMeasureSequenceItemEngine::SetHead(int nHead)
{
	m_nHead = nHead;
}

int CMeasureSequenceItemEngine::GetHead()
{
	return m_nHead;
}





//=========================================================================
// Item 관련
//=========================================================================
bool CMeasureSequenceItemEngine::ItemInitialize()
{
    bool bRet = true;
    for (size_t i = 0; i < m_vecItems.size(); ++i)
    {
        if (m_vecItems[i])
        {
            bRet &= m_vecItems[i]->CreateSeqItem();
        }
    }

    return bRet;
}

void CMeasureSequenceItemEngine::ItemFinalize()
{
    for (size_t i = 0; i < m_vecItems.size(); ++i)
    {
        if (m_vecItems[i])
        {
            delete m_vecItems[i];
            m_vecItems[i] = nullptr;
        }
    }
}


bool CMeasureSequenceItemEngine::ItemStart(CString strType, bool bCompleteCheck)
{
    CMeasureSequenceItemImpl *pItem = FindItem(strType);

    if(pItem)
    {
        pItem->Start();

        CString strLog;
        strLog.Format(_T("Seq Item : %s - Start"), pItem->GetName().GetString());
        m_pLogFileAccessor->KMLOG(OPER, strLog);

        if(bCompleteCheck == true)
        {
            m_hBreakEvent_link = pItem->GetCompleteEventHandle();

            if(WaitForSingleObject(m_hBreakEvent_link, INFINITE) == WAIT_OBJECT_0)
            {
                // 멈추기를 원하는 경우
                if(m_bEngineBreak == true)
                {
                    pItem->Stop();
                    pItem->Exit();	// exit 시켜줘야 한다.

                    CString strLog;
                    strLog.Format(_T("Seq Item : %s - User Force Exit"), pItem->GetName().GetString());
                    m_pLogFileAccessor->KMLOG(OPER, strLog);
                    //return;
                }
            }

            strLog.Format(_T("Seq Item : %s - End"), pItem->GetName().GetString());
            m_pLogFileAccessor->KMLOG(OPER, strLog);

            //CWindowEventHandler::GetInstance()->PostMessageToUI((enumSeqItemID)pItem->GetSeqItemNumber(), WM_MSG_SEQ_ITEM_COMPLTE, NULL, NULL);
        }

        return true;
    }

    return false;
}

void CMeasureSequenceItemEngine::ItemStopAndSkip(CString strType)
{
    CMeasureSequenceItemImpl *pItem = FindItem(strType);

    if(pItem)
    {
        pItem->Stop();
        pItem->Skip();	// exit 시켜줘야 한다.

        CString strLog;
        strLog.Format(_T("Seq Item : %s - User Force Skip"), pItem->GetName().GetString());
        m_pLogFileAccessor->KMLOG(OPER, strLog);
    }
}

void CMeasureSequenceItemEngine::ItemStopAndExit(CString strType)
{
    CMeasureSequenceItemImpl *pItem = FindItem(strType);

    if(pItem)
    {
        pItem->Stop();
        pItem->Exit();	// exit 시켜줘야 한다.

        CString strLog;
        strLog.Format(_T("Seq Item : %s - User Force Exit"), pItem->GetName().GetString());
        m_pLogFileAccessor->KMLOG(OPER, strLog);
    }
}

void CMeasureSequenceItemEngine::ItemPause(int index)
{
    if ((0 <= index) && (index < GetItemCount()))
    {
        m_vecItems[index]->Pause();
    }
}

void CMeasureSequenceItemEngine::ItemResume(int index)
{
    if ((0 <= index) && (index < GetItemCount()))
    {
        m_vecItems[index]->Resume();
    }
}

bool CMeasureSequenceItemEngine::Is_CompleteAll()
{
    bool ret = true;
    for(int i = 0; i < GetItemCount(); ++i)
    {
        ret &= m_vecItems[i]->Is_Complete();
    }

    return ret;
}

bool CMeasureSequenceItemEngine::Is_Complete(int index)
{
    if ((0 <= index) && (index < GetItemCount()))
    {
        return m_vecItems[index]->Is_Complete();
    }

    return true;
}





std::vector<CString> CMeasureSequenceItemEngine::GetItemType()
{
    return m_pItemFactory->GetItemType();
}

std::vector<CString> CMeasureSequenceItemEngine::GetItemDesc()
{
    return m_pItemFactory->GetItemDesc();
}

int CMeasureSequenceItemEngine::GetItemCount() const
{
    return (int)m_vecItems.size();
}

void CMeasureSequenceItemEngine::SetItemVisible(int index)
{
    for(int i = 0; i < GetItemCount(); ++i)
    {
        m_vecItems[i]->SetVisible(false);
    }

    if ((0 <= index) && (index < GetItemCount()))
    {
        m_vecItems[index]->SetVisible(true);
    }
}

void CMeasureSequenceItemEngine::AddItem(CString strType, CString strName)
{
    CMeasureSequenceItemImpl *ptempImpl = m_pItemFactory->GetItem(strType, strName);

    if (ptempImpl)
    {
        ptempImpl->SetType(strType);
        ptempImpl->SetName(strName);

        m_vecItems.push_back(ptempImpl);
    }
}

void CMeasureSequenceItemEngine::DelItem(int index)
{
    if ((0 <= index) && (index < GetItemCount()))
    {
        delete m_vecItems[index];
        m_vecItems[index] = nullptr;

        m_vecItems.erase(m_vecItems.begin() + index);
    }
}

CMeasureSequenceItemImpl *CMeasureSequenceItemEngine::GetItem(int index)
{
    if ((0 <= index) && (index < GetItemCount()))
    {
        return m_vecItems[index];
    }

    return nullptr;
}

CMeasureSequenceItemImpl *CMeasureSequenceItemEngine::FindItem(CString strSeq)
{
    for(size_t seq = 0; seq < m_vecItems.size(); ++seq)
    {
        if( (strSeq == m_vecItems[seq]->GetType()) ||
            (strSeq == m_vecItems[seq]->GetGUID()))
        {
            return m_vecItems[seq];
        }
    }

    for(size_t seq = 0; seq < m_vecItems.size(); ++seq)
    {
        if( 0 <= m_vecItems[seq]->GetType().Find(strSeq))
        {
            return m_vecItems[seq];
        }
    }

    return nullptr;
}

void CMeasureSequenceItemEngine::UpdateItemState()
{
    for (size_t i = 0; i < m_vecItems.size(); ++i)
    {
        if (m_vecItems[i])
        {
            m_vecItems[i]->UpdateState();
        }
    }
}


void CMeasureSequenceItemEngine::EngineStart()
{
    m_bEngineBreak = false;

	ResetEvent(m_hBreakEvent_link);
    SetEvent(m_hStartEvent);
}

void CMeasureSequenceItemEngine::EngineResume()
{
	// 엔진의 thread 를 재시작 시킨다.
    m_pThread->ResumeThread();

	// 시퀀스의 thread 를 재시작 시킨다.
	for(int index = 0; index < GetRecipeEditMgrPtr()->GetRecipeDataPtr()->GetParam_MeasAreaItemCount(); ++index)
    {
        const CString strSeq = GetRecipeEditMgrPtr()->GetRecipeDataPtr()->GetParam_MeasAreaItem(index);

        CMeasureSequenceItemImpl *pItem = FindItem(strSeq);
        if(pItem)
        {
			pItem->Resume();
        }
    }
}

void CMeasureSequenceItemEngine::EnginePause()
{
	// 엔진의 thread 를 중지 시킨다.
    m_pThread->SuspendThread();

	// 시퀀스의 thread 를 중지 시킨다.
	for(int index = 0; index < GetRecipeEditMgrPtr()->GetRecipeDataPtr()->GetParam_MeasAreaItemCount(); ++index)
    {
        const CString strSeq = GetRecipeEditMgrPtr()->GetRecipeDataPtr()->GetParam_MeasAreaItem(index);

        CMeasureSequenceItemImpl *pItem = FindItem(strSeq);
        if(pItem)
        {
			pItem->Pause();
        }
    }
}

void CMeasureSequenceItemEngine::EngineStop()
{
    m_bEngineBreak = true;

	ResetEvent(m_hStartEvent);
    SetEvent(m_hBreakEvent_link);
}














//=========================================================================
// Item Run Thread
//=========================================================================
UINT CMeasureSequenceItemEngine::ThreadRun(LPVOID p1)
{
    CMeasureSequenceItemEngine *pThis = (CMeasureSequenceItemEngine *)p1;

    pThis->m_bThreadLive = true;

    while (1)
    {
        DWORD dw = WaitForMultipleObjects(3, pThis->m_hMultiEvent, FALSE, INFINITE);

        if (dw == WAIT_OBJECT_0) // Start Event 가 set될 때까지 기다린다.
        {
            pThis->ThreadRun();
        }
        else if (dw == (WAIT_OBJECT_0 + 1)) // Exit Event 가 set될 때까지 기다린다.
        {
            pThis->ThreadRunExit();
        }
        else if (dw == (WAIT_OBJECT_0 + 2)) // Close Event 가 set될 때까지 기다린다.
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

void CMeasureSequenceItemEngine::ThreadRun()
{
    // tact time 기록을 위해서
    GetSeqTactTimeMgrPtr()->SetDataClear();
    GetSeqTactTimeMgrPtr()->SetTotalSeqTimeST();

	CString strGlassID, strRecipeID;
	strGlassID = CSystemParamShare::GetInstance()->GetGlassID();
	strRecipeID = GetRecipeEditMgrPtr()->GetRecipeID();

	int nCycleCount = 1;
	if (m_MgrPool_link.pShareMemoryMgr->GetSys_MachineMode() == df_HOST_OFFLINE)
	{
		nCycleCount = CSystemParamShare::GetInstance()->GetCycleCount();
	}

	for (int index = 0; index < nCycleCount; ++index)
    {
        const int cycle_index = index + 1;
        GetSeqTactTimeMgrPtr()->SetCycleTimeST(cycle_index, strGlassID, strRecipeID);

        ThreadRu_nSequence(cycle_index);

        // 시퀀스를 멈춰야 하는 경우
        if(IsThreadStopState() == true)
		{
			break;
		}

        GetSeqTactTimeMgrPtr()->SetCycleTimeED(cycle_index);

        // cycle 을 돌려주기 위해서는 5s 정도 기다려주자.
        WaitForSingleObject(m_hWaitsEvent, 5000);
    }

    GetSeqTactTimeMgrPtr()->SetTotalSeqTimeED();

	//--------------------------------------------------------------
	// 시퀀스를 멈춰야 하는 경우
	//--------------------------------------------------------------
    if(IsThreadStopState() == true)
	{
		SetEvent(m_hExitsEvent);
	}
	else
	{
	    CWindowEventHandler::GetInstance()->PostMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CYCLE_INSPECTION_COMPLETE, NULL, NULL);
	}
}

void CMeasureSequenceItemEngine::ThreadRu_nSequence(int cycle_index)
{
    for(int index = 0; index < GetRecipeEditMgrPtr()->GetRecipeDataPtr()->GetParam_MeasAreaItemCount(); ++index)
    {
        const CString strSeq = GetRecipeEditMgrPtr()->GetRecipeDataPtr()->GetParam_MeasAreaItem(index);

        CMeasureSequenceItemImpl *pItem = FindItem(strSeq);
        if(pItem)
        {
            const int iseq_index = index + 1;

			m_hBreakEvent_link = pItem->GetCompleteEventHandle();
			

			//---------------------------------------------------------------------
			// LOADING 시퀀스를 동작하려고 하는데.. 이미 Vaccum 이 잡혀 있다면
			// Skip 이 가능하도록 하자.
			//---------------------------------------------------------------------
			if(pItem->GetGUID() == _T("5D2C9B4D-BF44-478A-8700-A51FAC470750"))
			{
				//// Vaccum 이 잡혀 있다면
				//if(m_pIOMapControlMgr->IsVacuumOnState() == true)
				//{
				//	CD_REQ_LoadingSkip dlg;
				//	if(dlg.DoModal() == IDOK)
				//	{
			    //        m_pSeqTactTimeMgr->SetSeqItemTimeST(cycle_index, pItem->GetName() + _T("_SKIP"), iseq_index);
			    //        m_pSeqTactTimeMgr->SetSeqItemTimeED(cycle_index, pItem->GetName() + _T("_SKIP"), iseq_index);
				//		continue;
				//	}
				//	else
				//	{
				//		m_bEngineBreak = true;
				//
				//		CString strLog;
				//		strLog.Format(_T("Cycle Seq Item : %s - User Force Exit"), pItem->GetName().GetString());
				//		m_pLogFileAccessor->KMLOG(OPER, strLog);
				//		return;
				//	}
				//}
			}
			//---------------------------------------------------------------------
			//---------------------------------------------------------------------


            pItem->Start();

            GetSeqTactTimeMgrPtr()->SetSeqItemTimeST(cycle_index, pItem->GetName(), iseq_index);

            if(WaitForSingleObject(m_hBreakEvent_link, INFINITE) == WAIT_OBJECT_0)
            {
                // 시퀀스를 멈춰야 하는 경우
                if(IsThreadStopState() == true)
                {
					// 사용자에 의해서 강제로 종료 되었다.
					if(m_bEngineBreak == true) 
					{
						CSystemParamShare::GetInstance()->SetForceStopByUser(true);						
					}

                    pItem->Stop();
                    pItem->Exit();	// exit 시켜줘야 한다.

                    CString strLog;
                    strLog.Format(_T("Cycle Seq Item : %s - User Force Exit"), pItem->GetName().GetString());
                    m_pLogFileAccessor->KMLOG(OPER, strLog);
                    return;
                }
            }

            GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(cycle_index, pItem->GetName(), iseq_index);
        }
    }
}

void CMeasureSequenceItemEngine::ThreadRunExit()
{
	// 시퀀스를 종료하기 위해서는 500ms 정도 기다려주자.
    WaitForSingleObject(m_hWaitsEvent, 500);

	for(int index = 0; index < GetRecipeEditMgrPtr()->GetRecipeDataPtr()->GetParam_MeasAreaItemCount(); ++index)
    {
        const CString strSeq = GetRecipeEditMgrPtr()->GetRecipeDataPtr()->GetParam_MeasAreaItem(index);

        CMeasureSequenceItemImpl *pItem = FindItem(strSeq);
        if(pItem)
        {
            pItem->Stop();
            pItem->Exit();	// exit 시켜줘야 한다.
        }
    }

	CWindowEventHandler::GetInstance()->PostMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CYCLE_INSPECTION_COMPLETE, NULL, NULL);
}


bool CMeasureSequenceItemEngine::IsThreadStopState()
{
	// 센서 감지로 사이클이 멈춰야 하는 상황
	if(CSystemParamShare::GetInstance()->IsCycleState() == false)
	{
		return true;
	}

    // 사용자에 의해서 강제로 종료 되었다.
    if(m_bEngineBreak == true) 
    {
        return true;
    }

	return false;
}

void CMeasureSequenceItemEngine::SetMeasureArea(int nMeasureArea)
{
    if (m_pStInnerID != nullptr)
    {
        for (int k = 0; k < m_nMeasureArea; k++)
            delete m_pStInnerID[k];

        delete[] m_pStInnerID;
        m_pStInnerID = NULL;
    }

    if (nMeasureArea > 0)
    {
        m_pStInnerID = new TCHAR * [nMeasureArea];
        for (int i = 0; i < nMeasureArea; i++)
        {
            m_pStInnerID[i] = new TCHAR[32];
            memset(m_pStInnerID[i], 0x00, sizeof(m_pStInnerID[i]));			// INNER ID 저장 공간
        }
    }

    m_nMeasureArea = nMeasureArea;
}

//=========================================================================
// LYS 2026-08-12 InnerID 유일성.
// gf_GetCurTimeString(3)은 HHMMSS라 같은 초의 측정은 동일 id -> 동일 raw 파일명 -> 조용한 덮어쓰기.
// 길이는 고정 폭 장비 패킷 필드라 유지해야 하므로, 중복 시 끝 2자리만 재배정한다.
// 실제 초로는 나올 수 없는 60..99를 먼저 시도하고 발급 목록이 최종 판단하므로,
// 나중에 진짜 타임스탬프가 같은 문자열을 만들어도 유일성은 유지된다.
//=========================================================================
void CMeasureSequenceItemEngine::PushIssuedInnerID(const CString& strID)
{
	m_vecIssuedInnerID.push_back(strID);

	// rolling window: ids carry HHMMSS, so anything older than the recent window can
	// no longer collide. Bounding here avoids tying the list to a glass/stick lifecycle.
	if (m_vecIssuedInnerID.size() > DEFINE_INNERID_KEEP_MAX)
	{
		m_vecIssuedInnerID.erase(m_vecIssuedInnerID.begin(),
			m_vecIssuedInnerID.begin() + (m_vecIssuedInnerID.size() - DEFINE_INNERID_KEEP_MAX));
	}
}

CString CMeasureSequenceItemEngine::MakeUniqueInnerID(const CString& strID)
{
	if (strID.GetLength() < 3) return strID;

	bool bUsed = false;
	for (size_t i = 0; i < m_vecIssuedInnerID.size(); i++)
	{
		if (m_vecIssuedInnerID[i] == strID) { bUsed = true; break; }
	}

	if (bUsed == false)
	{
		PushIssuedInnerID(strID);
		return strID;
	}

	CString strHead = strID.Left(strID.GetLength() - 2);
	CString strTry;
	for (int nSeq = 60; nSeq < 160; nSeq++)
	{
		strTry.Format(_T("%s%02d"), (LPCTSTR)strHead, nSeq % 100);

		bUsed = false;
		for (size_t i = 0; i < m_vecIssuedInnerID.size(); i++)
		{
			if (m_vecIssuedInnerID[i] == strTry) { bUsed = true; break; }
		}

		if (bUsed == false)
		{
			PushIssuedInnerID(strTry);

			CString strLog;
			strLog.Format(_T("InnerID duplicate remapped: %s -> %s"), (LPCTSTR)strID, (LPCTSTR)strTry);
			m_pLogFileAccessor->KMLOG(WARN, strLog);

			return strTry;
		}
	}

	m_pLogFileAccessor->KMLOG(WARN, _T("InnerID duplicate could not be remapped - reusing"));

	return strID;
}

int CMeasureSequenceItemEngine::GetMeasureArea()
{
    return m_nMeasureArea;
}
//=========================================================================
// LYS 2026-08-06 AgingCycle E3: 에이징 리스트 사이클 컨텍스트(OCP DoWorkStartMeasOCP_A)
//=========================================================================
bool CMeasureSequenceItemEngine::AgingCycleBegin(int nEfaPass, int nStick)	// LYS 2026-09-28 EFA: nEfaPass >= 0이면 고정 항목
{
	m_bAgingCycleActive = false;
	m_nAgingCurItem = -1;
	m_bEfaCycle = false;	// LYS 2026-09-28 EFA
	m_nAgingStick = nStick;		// LYS 2026-09-30 OCP nGroup < 1: items / sticks without a cell are skipped
	m_bAgingStickSkip = false;
	m_bSamplingCycleSkip = false;	// LYS 2026-10-08 Sampling stick skip
	ZeroMemory(m_nAgingRepeat, sizeof(m_nAgingRepeat));	// LYS 2026-08-12 AgingCycle R1
	m_strCycleSegTag = (GetRecipeEditMgrPtr() != nullptr) ? GetRecipeEditMgrPtr()->GetSegLogTag() : CString();	// LYS 2026-09-30 Hybrid 3c this cycle's begin/skip/end lines (1 segment: empty)

	if (m_MgrPool_link.pSystemConfigMgr == nullptr || m_MgrPool_link.pSystemConfigMgr->IsOCP() == false)
		return false;

	// LYS 2026-09-28 EFA: 고정 항목 pass. OCP DoWorkStartMeasICP_EFA는 항목(pass) 루프를 스틱 루프 바깥에서 돌리므로
	// 한 스틱은 항목 하나만 보고 스틱 안 루프백이 없다(AgingCycleNextItem -> false).
	if (nEfaPass >= 0)
	{
		if (m_AgingCycleInfo.Load(GetRecipeEditMgrPtr()->GetRecipeID(), true, GetRecipeEditMgrPtr()->GetSegmentLoopCount()) == false)	// LYS 2026-09-29 Hybrid 3b A1
		{
			m_pLogFileAccessor->KMLOG(WARN, _T("EFA: efa rcp not found - run standard sequence"));
			return false;
		}
		if (nEfaPass >= m_AgingCycleInfo.GetItemCount()) return false;

		m_nAgingCurItem = nEfaPass;
		m_bEfaCycle = true;
		m_bAgingCycleActive = true;
		CheckAgingMaskShape();	// LYS 2026-09-30

		// LYS 2026-09-30 OCP DoWorkStartMeasICP_EFA skips a stick whose group is empty (nGroup < 1: no contact move, no measure)
		if (AgingItemHasCell(nEfaPass) == false)
		{
			m_bAgingCycleActive = false;
			m_bEfaCycle = false;
			m_nAgingCurItem = -1;
			m_bAgingStickSkip = true;

			CString strSkip;
			strSkip.Format(_T("EFA: pass %d/%d stick %d skipped - no cell for the item in [%s]%s"), nEfaPass + 1, m_AgingCycleInfo.GetItemCount(),
				nStick + 1, CAgingCycleInfo::GetMaskSection(GetRecipeEditMgrPtr()->GetActiveSegment()).GetString(), m_strCycleSegTag.GetString());	// LYS 2026-09-30 Hybrid 3c seg tag
			m_pLogFileAccessor->KMLOG(OPER, strSkip);
			return false;
		}

		// LYS 2026-10-08 Sampling stick skip (D-SS-3 b1s): the pass item has mask cells on this stick, but Sampling may select none of them
		if (SamplingCycleSkip(nEfaPass)) return false;

		CString strEfaLog;
		strEfaLog.Format(_T("EFA: begin pass %d/%d%s"), nEfaPass + 1, m_AgingCycleInfo.GetItemCount(), m_strCycleSegTag.GetString());	// LYS 2026-09-30 Hybrid 3c seg tag
		m_pLogFileAccessor->KMLOG(OPER, strEfaLog);
		return true;
	}

	S_RecipeInfo* pRecipeInfo = GetRecipeEditMgrPtr()->GetRecipeInfo();
	// LYS 2026-08-12 게이트는 전용 레시피 플래그 m_bUseAging_Test를 쓴다.
	// m_bUseAgingTest_2는 AGING 측정 항목이라 에이징 사이클을 켜면 안 된다.
	if (m_nAgingGlassLatch >= 0)	// LYS 2026-09-29 Hybrid 3b A4 hybrid glass: flag latched from segment 0 at glass start
	{
		if (m_nAgingGlassLatch == 0)
			return false;
	}
	else if (pRecipeInfo == nullptr || pRecipeInfo->m_sInspectItem.m_bUseAging_Test == FALSE)
		return false;

	if (m_AgingCycleInfo.Load(GetRecipeEditMgrPtr()->GetRecipeID(), false, GetRecipeEditMgrPtr()->GetSegmentLoopCount()) == false)	// LYS 2026-09-29 Hybrid 3b A1 all segment mask sections
	{
		m_pLogFileAccessor->KMLOG(WARN, _T("AgingCycle: aging rcp not found - run standard sequence"));
		return false;
	}

	CheckAgingMaskShape();	// LYS 2026-09-30
	m_bAgingCycleActive = AgingCycleNextItem();

	// LYS 2026-09-30 OCP DoWorkStartMeasOCP_A skips an item whose group is empty on the stick (nGroup < 1 -> continue: no contact
	//   move, no pattern, no measure). AgingCycleNextItem already passes over such items; a stick where no USE_AGING item has a
	//   cell is flagged here and the caller runs only its init step. No USE_AGING item at all = standard sequence (unchanged).
	if (m_bAgingCycleActive == false && m_nAgingStick >= 0)
	{
		for (int i = 0; i < m_AgingCycleInfo.GetItemCount(); i++)
		{
			const S_AgingTest* pUse = m_AgingCycleInfo.GetItem(i);
			if (pUse != nullptr && pUse->m_bUseAging)
			{
				m_bAgingStickSkip = true;
				break;
			}
		}

		if (m_bAgingStickSkip)
		{
			CString strSkip;
			strSkip.Format(_T("AgingCycle: stick %d skipped - no cell for the active items in [%s]%s"),	// LYS 2026-09-30 Hybrid 3c seg tag
				m_nAgingStick + 1, CAgingCycleInfo::GetMaskSection(GetRecipeEditMgrPtr()->GetActiveSegment()).GetString(), m_strCycleSegTag.GetString());
			m_pLogFileAccessor->KMLOG(OPER, strSkip);
			return false;
		}
	}

	// LYS 2026-10-08 Sampling stick skip (D-SS-3 b1s): USE_AGING items have mask cells on this stick, but Sampling may select none of them
	if (m_bAgingCycleActive && m_nAgingStick >= 0 && SamplingCycleSkip(-1)) return false;

	CString strLog;
	strLog.Format(_T("AgingCycle: begin, items(%d) active(%d)%s"), m_AgingCycleInfo.GetItemCount(), (int)m_bAgingCycleActive, m_strCycleSegTag.GetString());	// LYS 2026-09-30 Hybrid 3c seg tag
	m_pLogFileAccessor->KMLOG(OPER, strLog);

	return m_bAgingCycleActive;
}

// LYS 2026-09-28 EFA: CDoWorkGlassMeasure::ThreadRun이 쓰는 pass 수. 0 = EFA 아님(일반 단일 pass).
// 게이트 = OCP + (EFA START / CIM 'B'가 켜는 공유 m_bNewAgingMode 또는 레시피 m_bUseEFA) + 항목이 있는 EFA rcp.
int CMeasureSequenceItemEngine::GetEfaPassCount()
{
	if (m_MgrPool_link.pSystemConfigMgr == nullptr || m_MgrPool_link.pSystemConfigMgr->IsOCP() == false) return 0;

	bool bFlag = false;
	if (m_MgrPool_link.pShareMemoryMgr != nullptr) bFlag = m_MgrPool_link.pShareMemoryMgr->GetSys_NewAgingMode();

	S_RecipeInfo* pRecipeInfo = GetRecipeEditMgrPtr()->GetRecipeInfo();
	bool bRecipe = (pRecipeInfo != nullptr && pRecipeInfo->m_sInspectItem.m_bUseEFA != FALSE);

	if (bFlag == false && bRecipe == false) return 0;

	CAgingCycleInfo info;
	if (info.Load(GetRecipeEditMgrPtr()->GetRecipeID(), true) == false || info.GetItemCount() < 1)
	{
		m_pLogFileAccessor->KMLOG(WARN, _T("EFA: mode requested but efa rcp missing/empty - run standard sequence"));
		return 0;
	}

	CString strLog;
	strLog.Format(_T("EFA: mode on (flag %d, recipe %d), passes %d"), (int)bFlag, (int)bRecipe, info.GetItemCount());
	m_pLogFileAccessor->KMLOG(OPER, strLog);

	return info.GetItemCount();
}

// LYS 2026-09-29 Hybrid Phase 3b A4: AgingTest on/off is glass-global (D-4 / D-14). Called once per glass by
//   CDoWorkGlassMeasure::ThreadRun before the pass/segment loop, i.e. before any SegmentBoundary.
//   Hybrid (N>1): latch segment 0's m_bUseAging_Test so every segment sees the same value
//   (saving already refuses segments that differ - InRecipeFunc ValidateSegments; the latch covers unvalidated files).
//   1 segment: -1 = not latched, AgingCycleBegin reads the recipe exactly as before.
// LYS 2026-09-30 Hybrid Phase 3c A5 nEfaPassCount = ThreadRun's measure-start EFA decision (EFA passes actually run, 0 = not EFA).
//   Hybrid: published to CRecipeEditMgr as the EFA glass latch, so MeasDone sends EFA^1$ from this decision and does not re-read
//   the shared flag / EFA rcp at glass end (they can change mid-glass: OnStop, every CIM PPID). 1 segment: latch -1 = MeasDone's own gate.
void CMeasureSequenceItemEngine::AgingGlassLatch(int nEfaPassCount)
{
	m_nAgingGlassLatch = -1;
	m_nAgingMaskChecked = 0;	// LYS 2026-09-30 new glass: check each segment's mask shape again (1-segment recipes too)
	GetRecipeEditMgrPtr()->SetEfaGlassLatch(-1);	// LYS 2026-09-30 Hybrid 3c A5 1-segment recipe: no latch, MeasDone keeps its own gate

	C_OutRecipeFunc* pFunc = GetRecipeEditMgrPtr()->GetRecipeFuncPtr();
	if (pFunc == nullptr || pFunc->GetSegmentCount() <= 1)
		return;

	S_RecipeMgr* pSeg0 = pFunc->GetSegmentMgr(0);	// read at once - the pointer is valid only until the next SetActiveSegment
	m_nAgingGlassLatch = (pSeg0 != nullptr && pSeg0->m_sInfo.m_sInspectItem.m_bUseAging_Test != FALSE) ? 1 : 0;

	CString strLog;
	strLog.Format(_T("[HYBRID] AgingTest flag latched from segment 1 for this glass: %d%s"), m_nAgingGlassLatch,
		(nEfaPassCount > 0) ? _T(" (unused - EFA glass)") : _T(""));	// LYS 2026-09-30 Hybrid 3c an EFA pass never reads this latch
	m_pLogFileAccessor->KMLOG(OPER, strLog);

	// LYS 2026-09-30 Hybrid 3c A5 EFA glass latch for MeasDone (EFA^1$): the decision taken at measure start
	GetRecipeEditMgrPtr()->SetEfaGlassLatch(nEfaPassCount);
	strLog.Format(_T("[HYBRID] EFA latched for this glass: passes %d (0 = not EFA) - EFA^1$ at MeasDone follows this"), nEfaPassCount);
	m_pLogFileAccessor->KMLOG(OPER, strLog);
}

// LYS 2026-09-30 The measure steps read a mask row by index: cells past the row's end are measured (IsAgingCellSkip),
//   sticks without a row are skipped (IsStickUse). A mask saved from a grid of another size is therefore silent -
//   2026-09-30 10:29 a 5 x 20 recipe got 10-cell rows from the dialog's default grid and cells K..T were all measured.
//   Log it once per glass and segment (reset in AgingGlassLatch). Log only: the measure rule does not change.
void CMeasureSequenceItemEngine::CheckAgingMaskShape()
{
	int nSeg = GetRecipeEditMgrPtr()->GetActiveSegment();
	if (nSeg < 0 || nSeg >= 32) return;
	if (m_nAgingMaskChecked & (1u << nSeg)) return;
	m_nAgingMaskChecked |= (1u << nSeg);

	int nRows = m_AgingCycleInfo.GetStickCount(nSeg);
	if (nRows <= 0)	// no mask section = every cell measured (intended)
	{
		if (GetRecipeEditMgrPtr()->GetSegmentLoopCount() > 1)	// LYS 2026-09-30 Hybrid 3c say it once per glass and segment (1 segment: silent as before)
		{
			CString strNoMask;
			strNoMask.Format(_T("%s: mask [%s] not found - segment %d/%d measures every cell of the item's kinds (no mask = measure all)"),
				m_bEfaCycle ? _T("EFA") : _T("AgingCycle"), CAgingCycleInfo::GetMaskSection(nSeg).GetString(), nSeg + 1, GetRecipeEditMgrPtr()->GetSegmentLoopCount());
			m_pLogFileAccessor->KMLOG(OPER, strNoMask);
		}
		return;
	}

	S_RecipeData* pRD = GetRecipeEditMgrPtr()->GetRecipeData();
	if (pRD == nullptr) return;
	int nStick = (int)pRD->m_vecStick.size();

	CString strBad, strOne;
	int nBad = 0;
	for (int i = 0; i < nStick && i < nRows; i++)
	{
		int nCell = (int)pRD->m_vecStick[i].m_vecCell.size();
		int nMask = m_AgingCycleInfo.GetCellCount(nSeg, i);
		if (nMask == nCell) continue;
		if (nBad < 8)
		{
			strOne.Format(_T("%s%d(%d/%d)"), (nBad > 0) ? _T(",") : _T(""), i + 1, nMask, nCell);
			strBad += strOne;
		}
		nBad++;
	}
	if (nBad == 0 && nRows == nStick) return;

	CString strLog;
	strLog.Format(_T("%s: mask [%s] does not fit the recipe - rows %d / sticks %d, cell count differs on %d row(s)%s%s - cells past a row are measured, sticks without a row are skipped"),
		m_bEfaCycle ? _T("EFA") : _T("AgingCycle"), CAgingCycleInfo::GetMaskSection(nSeg).GetString(), nRows, nStick, nBad,
		(nBad > 0) ? _T(", stick(mask/recipe) ") : _T(""), strBad.GetString());
	m_pLogFileAccessor->KMLOG(WARN, strLog);
}

bool CMeasureSequenceItemEngine::AgingCycleNextItem()
{
	if (m_bEfaCycle) return false;	// LYS 2026-09-28 EFA: pass당 고정 항목 하나, 루프백 없음

	for (int i = m_nAgingCurItem + 1; i < m_AgingCycleInfo.GetItemCount(); i++)
	{
		const S_AgingTest* pItem = m_AgingCycleInfo.GetItem(i);
		if (pItem != nullptr && pItem->m_bUseAging && AgingItemHasCell(i))	// LYS 2026-09-30 no cell on this stick = skip the item (OCP nGroup < 1)
		{
			m_nAgingCurItem = i;
			return true;
		}
	}

	return false;
}

// LYS 2026-09-30 Does item iItem measure anything on the cycle's stick? The same test as the WAD/VISION/ANGLE step gates
//   (item flag x IsStickUse of the active segment; no mask section = every stick), so a skipped item is exactly one whose
//   three measure steps would all have skipped - result rows and repeat indexes do not move. OCP: DoWorkStartMeasOCP_A nGroup.
bool CMeasureSequenceItemEngine::AgingItemHasCell(int iItem)
{
	if (m_nAgingStick < 0) return true;	// no stick given: item-only selection as before

	const S_AgingTest* pItem = m_AgingCycleInfo.GetItem(iItem);
	if (pItem == nullptr) return false;

	int nSeg = GetRecipeEditMgrPtr()->GetActiveSegment();
	bool bAll = (m_AgingCycleInfo.GetStickCount(nSeg) == 0);
	if (pItem->m_bUseProperty && (bAll || m_AgingCycleInfo.IsStickUse(nSeg, m_nAgingStick, enumAgingMeasKind_WAD)))		return true;
	if (pItem->m_bUseQuality  && (bAll || m_AgingCycleInfo.IsStickUse(nSeg, m_nAgingStick, enumAgingMeasKind_VISION)))	return true;
	if (pItem->m_bUseAngle    && (bAll || m_AgingCycleInfo.IsStickUse(nSeg, m_nAgingStick, enumAgingMeasKind_ANGLE)))		return true;
	return false;
}

//=========================================================================
// LYS 2026-08-12 AgingCycle R1: 종류별 반복 인덱스(결과 행 블록).
// OCP 대응 = nPropertyRepeat / nQualityRepeat / nLCRRepeat
// (JOB_Proc.cpp:12676-12683, stick마다 리셋하고 측정했을 때만 증가).
// 의도적으로 조밀하게 매긴다: 성기면 빈 결과 행이 생기는데 HMI APD writer에
// 빈 행 스킵이 없어(CDlgResult.cpp:2019) 공백이 MES로 넘어간다.
//=========================================================================
int CMeasureSequenceItemEngine::GetAgingRepeat(int iKind) const
{
	if (m_bAgingCycleActive == false) return 0;
	if (iKind < 0 || iKind >= enumAgingMeasKind_MAX) return 0;

	return m_nAgingRepeat[iKind];
}

void CMeasureSequenceItemEngine::AgingRepeatNext(int iKind)
{
	if (m_bAgingCycleActive == false) return;
	if (iKind < 0 || iKind >= enumAgingMeasKind_MAX) return;

	m_nAgingRepeat[iKind]++;
}

int CMeasureSequenceItemEngine::GetAgingKindMax(int iKind) const
{
	if (m_bAgingCycleActive == false) return 1;

	int nMax = m_AgingCycleInfo.GetKindUseCount(iKind);

	return (nMax > 0) ? nMax : 1;
}

// LYS 2026-10-08 Sampling stick skip (OCP DoWorkStartMeasOCP: nGroup < 1 -> continue, before the contact move).
//   Standard glass only: an AgingTest item list / EFA pass decides its sticks in AgingCycleBegin (SamplingCycleSkip), ThreadRun also checks bAgingCycle.
//   OCP (D-SS-2) + recipe Sampling on + at least one of WAD (m_bUseSpec_3 / M_SPEC), VISION (m_bUseQuality_4 / M_QUALITY), ANGLE (m_bUseAngle_6 / M_ANGLE)
//   + no cell of the stick selected for any of them (m_nSelectCell == 1, the steps' own test) -> true: ThreadRun runs only the init step
//   (no move, pattern search, contact, OST, R-TEST, AGING, measure). Active segment, segment-local stick, no rotation reversal.
//   Not OCP / Sampling off / no optical item (electrical-only recipe, D-SS-7) / a 1-cell stick (D-SS-4) / stick out of range -> false, no log.
bool CMeasureSequenceItemEngine::IsSamplingStickSkip(int nStick)
{
	if (m_MgrPool_link.pSystemConfigMgr == nullptr || m_MgrPool_link.pSystemConfigMgr->IsOCP() == false) return false;
	if (m_bAgingCycleActive || m_bAgingStickSkip) return false;
	if (GetRecipeEditMgrPtr() == nullptr) return false;

	S_RecipeInfo* pInfo = GetRecipeEditMgrPtr()->GetRecipeInfo();
	S_RecipeData* pData = GetRecipeEditMgrPtr()->GetRecipeData();
	if (pInfo == nullptr || pData == nullptr) return false;
	if (pInfo->m_sOptionItem.m_bUseSampling == FALSE) return false;

	bool bWad = (pInfo->m_sInspectItem.m_bUseSpec_3 != FALSE);
	bool bVis = (pInfo->m_sInspectItem.m_bUseQuality_4 != FALSE);
	bool bAng = (pInfo->m_sInspectItem.m_bUseAngle_6 != FALSE);
	if (bWad == false && bVis == false && bAng == false) return false;	// D-SS-7 electrical-only recipe: the stick is always contacted
	if (nStick < 0 || nStick >= (int)pData->m_vecStick.size()) return false;

	const std::vector<S_CellInfo>& vecCell = pData->m_vecStick[nStick].m_vecCell;
	if (vecCell.size() == 1) return false;	// D-SS-4 a 1-cell stick is measured as before (the steps' remaining-cell pass); a 0-cell stick measures nothing and is skipped
	for (size_t c = 0; c < vecCell.size(); c++)
	{
		if (bWad && vecCell[c].m_nSelectCell[M_SPEC] == 1)		return false;
		if (bVis && vecCell[c].m_nSelectCell[M_QUALITY] == 1)	return false;
		if (bAng && vecCell[c].m_nSelectCell[M_ANGLE] == 1)		return false;
	}

	CString strItems;
	if (bWad) strItems += _T("WAD,");
	if (bVis) strItems += _T("VISION,");
	if (bAng) strItems += _T("ANGLE,");
	strItems.TrimRight(_T(','));

	CString strSkip;
	strSkip.Format(_T("Sampling: stick %d skipped - no selected cell for the enabled items (%s) - no move/contact/OST/R-TEST/AGING%s"),
		nStick + 1, strItems.GetString(), GetRecipeEditMgrPtr()->GetSegLogTag().GetString());	// 1 segment: empty tag
	m_pLogFileAccessor->KMLOG(OPER, strSkip);	// measure_seq_item_engine log, next to the 3c "AgingCycle: stick n skipped" lines
	return true;
}

// LYS 2026-10-08 Sampling stick skip (D-SS-3 b1s) for a cycle stick (AgingTest item list / EFA pass), called by AgingCycleBegin after the 3c test.
//   The steps measure a cell of kind k for an item when the item uses k on the stick (flag x IsStickUse, no mask section = every stick), the cell
//   is in the mask (IsAgingCellSkip: no section / past the row's end = in) and, with Sampling on, m_nSelectCell == 1. When no item the cycle runs
//   on this stick (iOnlyItem = the EFA pass item; -1 = every USE_AGING item with a cell = the items AgingCycleNextItem visits) has such a cell,
//   the cycle becomes a 3c-style stick skip: no begin / end line, one OPER line, true (the caller returns false).
//   Sampling off / a 1-cell stick (D-SS-4) / stick out of range / no optical kind (D-SS-7) -> false: nothing changed, no log.
//   Item order, repeat indexes and the 3c lines stay as they are (a skipped stick writes no result row; repeats restart per stick).
bool CMeasureSequenceItemEngine::SamplingCycleSkip(int iOnlyItem)
{
	if (m_MgrPool_link.pSystemConfigMgr == nullptr || m_MgrPool_link.pSystemConfigMgr->IsOCP() == false) return false;	// D-SS-2 (AgingCycleBegin checks it too)
	if (GetRecipeEditMgrPtr() == nullptr) return false;

	S_RecipeInfo* pInfo = GetRecipeEditMgrPtr()->GetRecipeInfo();
	S_RecipeData* pData = GetRecipeEditMgrPtr()->GetRecipeData();
	if (pInfo == nullptr || pData == nullptr) return false;
	if (pInfo->m_sOptionItem.m_bUseSampling == FALSE) return false;
	if (m_nAgingStick < 0 || m_nAgingStick >= (int)pData->m_vecStick.size()) return false;

	const std::vector<S_CellInfo>& vecCell = pData->m_vecStick[m_nAgingStick].m_vecCell;
	if (vecCell.size() == 1) return false;	// D-SS-4 a 1-cell stick is measured as before

	int nSeg = GetRecipeEditMgrPtr()->GetActiveSegment();
	int nRows = m_AgingCycleInfo.GetStickCount(nSeg);
	int nMaskCell = (nRows > 0) ? m_AgingCycleInfo.GetCellCount(nSeg, m_nAgingStick) : 0;
	const int nSelMode[enumAgingMeasKind_MAX] = { M_SPEC, M_QUALITY, M_ANGLE };	// enumAgingMeasKind_WAD / _VISION / _ANGLE
	bool bAnyKind = false;

	for (int i = 0; i < m_AgingCycleInfo.GetItemCount(); i++)
	{
		if (iOnlyItem >= 0 && i != iOnlyItem) continue;
		const S_AgingTest* pItem = m_AgingCycleInfo.GetItem(i);
		if (pItem == nullptr) continue;
		if (iOnlyItem < 0 && (pItem->m_bUseAging == FALSE || AgingItemHasCell(i) == false)) continue;

		const BOOL bKind[enumAgingMeasKind_MAX] = { pItem->m_bUseProperty, pItem->m_bUseQuality, pItem->m_bUseAngle };
		for (int k = 0; k < enumAgingMeasKind_MAX; k++)
		{
			if (bKind[k] == FALSE) continue;
			if (nRows != 0 && m_AgingCycleInfo.IsStickUse(nSeg, m_nAgingStick, k) == false) continue;	// the step's bUseMeas
			bAnyKind = true;
			for (int c = 0; c < (int)vecCell.size(); c++)
			{
				bool bMask = (nRows <= 0 || c >= nMaskCell || m_AgingCycleInfo.IsCellUse(nSeg, m_nAgingStick, c, k));	// IsAgingCellSkip() == false
				if (bMask && vecCell[c].m_nSelectCell[nSelMode[k]] == 1) return false;
			}
		}
	}
	if (bAnyKind == false) return false;	// D-SS-7 no optical kind in the cycle items: never a Sampling skip. Defensive - AgingCycleBegin calls this only after the 3c test found a used kind; in a cycle the steps read the item flags, not Spec_3 / Quality_4 / Angle_6

	CString strWhat;
	if (m_bEfaCycle) strWhat.Format(_T("EFA pass %d/%d"), iOnlyItem + 1, m_AgingCycleInfo.GetItemCount());
	else			 strWhat = _T("AgingTest items");

	m_bAgingCycleActive = false;
	m_bEfaCycle = false;
	m_nAgingCurItem = -1;
	m_bAgingStickSkip = true;
	m_bSamplingCycleSkip = true;

	CString strSkip;
	strSkip.Format(_T("Sampling: stick %d skipped - no selected cell for the cycle items in [%s] (%s) - no move/contact/OST/R-TEST/AGING%s"),
		m_nAgingStick + 1, CAgingCycleInfo::GetMaskSection(nSeg).GetString(), strWhat.GetString(), m_strCycleSegTag.GetString());
	m_pLogFileAccessor->KMLOG(OPER, strSkip);
	return true;
}

void CMeasureSequenceItemEngine::AgingCycleEnd()
{
	if (m_bAgingCycleActive)
		m_pLogFileAccessor->KMLOG(OPER, _T("AgingCycle: end") + m_strCycleSegTag);	// LYS 2026-09-30 Hybrid 3c tag cached at AgingCycleBegin (the last stick ends after the segment-0 restore)
	m_strCycleSegTag.Empty();

	m_bAgingCycleActive = false;
	m_nAgingCurItem = -1;
	ZeroMemory(m_nAgingRepeat, sizeof(m_nAgingRepeat));	// LYS 2026-08-12 AgingCycle R1
	m_bEfaCycle = false;	// LYS 2026-09-28 EFA
	m_nAgingStick = -1;			// LYS 2026-09-30
	m_bAgingStickSkip = false;
	m_bSamplingCycleSkip = false;	// LYS 2026-10-08 Sampling stick skip

	if (m_MgrPool_link.pShareMemoryMgr != nullptr && m_MgrPool_link.pShareMemoryMgr->GetSharedMemory() != nullptr)
	{
		m_MgrPool_link.pShareMemoryMgr->GetSharedMemory()->m_nAgingListRow = 0;
		m_MgrPool_link.pShareMemoryMgr->GetSharedMemory()->m_nAgingListCol = 0;
	}
}
