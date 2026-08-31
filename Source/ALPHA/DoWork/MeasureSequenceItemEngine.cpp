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
bool CMeasureSequenceItemEngine::AgingCycleBegin()
{
	m_bAgingCycleActive = false;
	m_nAgingCurItem = -1;
	ZeroMemory(m_nAgingRepeat, sizeof(m_nAgingRepeat));	// LYS 2026-08-12 AgingCycle R1

	if (m_MgrPool_link.pSystemConfigMgr == nullptr || m_MgrPool_link.pSystemConfigMgr->IsOCP() == false)
		return false;

	S_RecipeInfo* pRecipeInfo = GetRecipeEditMgrPtr()->GetRecipeInfo();
	// LYS 2026-08-12 게이트는 전용 레시피 플래그 m_bUseAging_Test를 쓴다.
	// m_bUseAgingTest_2는 AGING 측정 항목이라 에이징 사이클을 켜면 안 된다.
	if (pRecipeInfo == nullptr || pRecipeInfo->m_sInspectItem.m_bUseAging_Test == FALSE)
		return false;

	if (m_AgingCycleInfo.Load(GetRecipeEditMgrPtr()->GetRecipeID()) == false)
	{
		m_pLogFileAccessor->KMLOG(WARN, _T("AgingCycle: aging rcp not found - run standard sequence"));
		return false;
	}

	m_bAgingCycleActive = AgingCycleNextItem();

	CString strLog;
	strLog.Format(_T("AgingCycle: begin, items(%d) active(%d)"), m_AgingCycleInfo.GetItemCount(), (int)m_bAgingCycleActive);
	m_pLogFileAccessor->KMLOG(OPER, strLog);

	return m_bAgingCycleActive;
}

bool CMeasureSequenceItemEngine::AgingCycleNextItem()
{
	for (int i = m_nAgingCurItem + 1; i < m_AgingCycleInfo.GetItemCount(); i++)
	{
		const S_AgingTest* pItem = m_AgingCycleInfo.GetItem(i);
		if (pItem != nullptr && pItem->m_bUseAging)
		{
			m_nAgingCurItem = i;
			return true;
		}
	}

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

void CMeasureSequenceItemEngine::AgingCycleEnd()
{
	if (m_bAgingCycleActive)
		m_pLogFileAccessor->KMLOG(OPER, _T("AgingCycle: end"));

	m_bAgingCycleActive = false;
	m_nAgingCurItem = -1;
	ZeroMemory(m_nAgingRepeat, sizeof(m_nAgingRepeat));	// LYS 2026-08-12 AgingCycle R1

	if (m_MgrPool_link.pShareMemoryMgr != nullptr && m_MgrPool_link.pShareMemoryMgr->GetSharedMemory() != nullptr)
	{
		m_MgrPool_link.pShareMemoryMgr->GetSharedMemory()->m_nAgingListRow = 0;
		m_MgrPool_link.pShareMemoryMgr->GetSharedMemory()->m_nAgingListCol = 0;
	}
}
