#include "stdafx.h"
#include "DoWorkGlassUnloading.h"

#include "DoWorkGlassUnloadingStep.h"
#include "SystemParamShare.h"
#include "WindowEventHandler.h"
#include "..\CoreFunction\LogFunction\LogAccessor\LogFileAccessor.h"
#include "AuroraFuncDefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CDoWorkGlassUnloading::CDoWorkGlassUnloading(stAuroraMgrPool &MgrPool)
	: CDoWorkGlassSequenceImpl(MgrPool, enumSeqItemID_UNLOADING)
{
	m_strSeqType = _T("SEQUENCE_ITEM_DWGLASS_SEQ_UNLOADING");
	m_strSeqComm = _T("TCPIP");
	m_strSeqCorp = _T("KMAC");
	m_strSeqName = _T("UNLOADING");
	m_strSeqGUID = _T("9BE4A948-01F9-4B70-B33D-34F8EA073E9C");
	m_strSeqDesc = _T("GLASS UNLOADING");

	const CString strFolderPath	= CSystemParamShare::GetInstance()->GetLoggerFolderPath() + _T("\\LogFile\\");
	const CString strLogFile	= strFolderPath + m_strSeqName + _T(".log");
	MakeDir(strFolderPath);
	m_pLogFileAccessor->Initialize(strLogFile);

	// sequence step -
	m_vecSeqStep.clear();
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassUnloadingStep*>(new CDoWorkGlassUnloadingStep_None(this)));
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassUnloadingStep*>(new CDoWorkGlassUnloadingStep_InitPosition(this)));
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassUnloadingStep*>(new CDoWorkGlassUnloadingStep_InitPosition2(this)));
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassUnloadingStep*>(new CDoWorkGlassUnloadingStep_InitPositionCheck(this)));
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassUnloadingStep*>(new CDoWorkGlassUnloadingStep_Complete(this)));
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassUnloadingStep*>(new CDoWorkGlassUnloadingStep_CompleteCheck(this)));
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassUnloadingStep*>(new CDoWorkGlassUnloadingStep_Exit(this)));
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassUnloadingStep*>(new CDoWorkGlassUnloadingStep_ExitCheck(this)));
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassUnloadingStep*>(new CDoWorkGlassUnloadingStep_Idle(this)));

    SetTotalStepCount((long)m_vecSeqStep.size());
	SetExitStepIndex(GetTotalStepCount() - 1);
}

CDoWorkGlassUnloading::~CDoWorkGlassUnloading()
{	
	// sequence step -
	for(size_t i = 0; i < m_vecSeqStep.size(); ++i)
	{
		if(m_vecSeqStep[i])
		{
			delete m_vecSeqStep[i];
			m_vecSeqStep[i] = nullptr;
		}
	}

	DeleteDoWorkSeq();
}

CDoWorkGlassUnloadingStep* CDoWorkGlassUnloading::GetSeqItemStep()
{
	if(GetCurStep() < (long)m_vecSeqStep.size())
	{
		if(m_vecSeqStep[GetCurStep()])
		{
			return m_vecSeqStep[GetCurStep()];
		}
	}

	return nullptr;
}






//=========================================================================
// INTERFACE ฐüทร
//=========================================================================
bool CDoWorkGlassUnloading::CreateDoWorkSeq()
{
	return true;
}

void CDoWorkGlassUnloading::DeleteDoWorkSeq()
{
}

bool CDoWorkGlassUnloading::ChangeState()
{
	DeleteDoWorkSeq();

	return CreateDoWorkSeq();
}

bool CDoWorkGlassUnloading::UpdateState()
{
	return true;
}

bool CDoWorkGlassUnloading::LinkDataBase(CBinaryHelper *pBinaryHelper, bool bSave)
{
	return true;
}

void CDoWorkGlassUnloading::ThreadRun()
{
	CDoWorkGlassUnloadingStep *pSeqItem = GetSeqItemStep();
	if(pSeqItem)
	{
		if(pSeqItem->Run() == true)
		{
			GoToNextStep();
		}
		else
		{
		}
	}
}
