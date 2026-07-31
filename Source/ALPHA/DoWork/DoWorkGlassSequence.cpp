#include "pch.h"
#include "DoWorkGlassSequence.h"

#include "LogFunction\LogAccessor\LogFileAccessor.h"
#include "..\AuroraFuncDefine.h"
#include "..\SystemParamShare.h"

#include "DoWorkGlassSequenceEngine.h"
#include "DoWorkGlassSequenceImpl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CDoWorkGlassSequence::CDoWorkGlassSequence(stAuroraMgrPool &MgrPool)
    : CBaseSequenceImpl(MgrPool, enumSeqItemID_REQ_AUTO_FOCUS)
{
    m_strSeqType = _T("SEQUENCE_ITEM_DWGLASS_SEQ");
    m_strSeqComm = _T("TCPIP");
    m_strSeqCorp = _T("KMAC");
    m_strSeqName = _T("REQ_GLASS_SEQUENCE");
    m_strSeqGUID = _T("609AA557-3AD1-4886-B616-0C3F8637D39B");
    m_strSeqDesc = _T("Request Glass Sequence");

    //const CString strFolderPath	= MgrPool.strLogPath + _T("\\LogFile\\");
    //const CString strLogFile	= strFolderPath + m_strSeqName + _T(".log");
    //MakeDir(strFolderPath);
    //m_pLogFileAccessor->Initialize(strLogFile);

    // sequence step -
    m_pSeqEngine = new CDoWorkGlassSequenceEngine(MgrPool);
}

CDoWorkGlassSequence::~CDoWorkGlassSequence()
{
    // sequence step -
    if(m_pSeqEngine)
	{
		delete m_pSeqEngine;
		m_pSeqEngine = nullptr;
    }
}



CString CDoWorkGlassSequence::GetItemName(int index)
{
	if(m_pSeqEngine->GetItem(index))
	{
		return m_pSeqEngine->GetItem(index)->GetName();
	}

	return _T("");
}

CLogFileAccessor *CDoWorkGlassSequence::GetIteLogFileAccessorPtr(int index)
{
	if(m_pSeqEngine->GetItem(index))
	{
		return m_pSeqEngine->GetItem(index)->GetLogFileAccessorPtr();
	}

	return nullptr;
}

void CDoWorkGlassSequence::EngineStart()
{
	m_pSeqEngine->EngineStart();
}
void CDoWorkGlassSequence::EngineResume()
{
	m_pSeqEngine->EngineResume();
}
void CDoWorkGlassSequence::EnginePause()
{
	m_pSeqEngine->EnginePause();
}
void CDoWorkGlassSequence::EngineStop()
{
	m_pSeqEngine->EngineStop();
}
bool CDoWorkGlassSequence::Is_CompleteAll()
{
	return m_pSeqEngine->Is_CompleteAll();
}
bool CDoWorkGlassSequence::Is_Complete(int index)
{
	return m_pSeqEngine->Is_Complete(index);
}
bool CDoWorkGlassSequence::ItemStart(CString strType, bool bCompleteCheck)
{
	return m_pSeqEngine->ItemStart(strType, bCompleteCheck);
}
void CDoWorkGlassSequence::ItemStopAndSkip(CString strType)
{
	m_pSeqEngine->ItemStopAndSkip(strType);
}
void CDoWorkGlassSequence::ItemStopAndExit(CString strType)
{
	m_pSeqEngine->ItemStopAndExit(strType);
}






//=========================================================================
// INTERFACE ฐüทร
//=========================================================================
bool CDoWorkGlassSequence::CreateDoWorkSeq()
{
    return m_pSeqEngine->ItemInitialize();
}

void CDoWorkGlassSequence::DeleteDoWorkSeq()
{
	m_pSeqEngine->ItemFinalize();
}

bool CDoWorkGlassSequence::ChangeState()
{
    DeleteDoWorkSeq();

    return CreateDoWorkSeq();
}

bool CDoWorkGlassSequence::UpdateState()
{
    return true;
}

bool CDoWorkGlassSequence::LinkDataBase(CBinaryHelper *pBinaryHelper, bool bSave)
{
	m_pSeqEngine->LinkDataBase(pBinaryHelper, bSave);

    return true;
}

void CDoWorkGlassSequence::ThreadRun()
{
}
