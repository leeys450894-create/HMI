#include "pch.h"
#include "DoWorkGlassReady.h"

#include "DoWorkGlassReadyStep.h"
#include "..\SystemParamShare.h"
#include "LogFunction\LogAccessor\LogFileAccessor.h"
#include "..\AuroraFuncDefine.h"
#include "..\Manager\SeqTactTimeMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CDoWorkGlassReady::CDoWorkGlassReady(stAuroraMgrPool &MgrPool)
	: CDoWorkGlassSequenceImpl(MgrPool, enumSeqItemID_GLASS_LOAD)
	, m_pShareItem(this)
{
	m_strSeqType = _T("SEQUENCE_ITEM_DWGLASS_SEQ_GLASS_LOAD");
	m_strSeqComm = _T("TCPIP");
	m_strSeqCorp = _T("KMAC");
	m_strSeqName = _T("GLASS LOAD");
	m_strSeqGUID = _T("FCE18847-3A50-476B-B190-456E870735F5");
	m_strSeqDesc = _T("GLASS LOAD");

	// 로그 생성하기
	const CString strFolderPath	= CSystemParamShare::GetInstance()->GetLoggerFolderPath() + _T("\\LogFile\\");
	const CString strLogFile	= strFolderPath + m_strSeqName + _T(".log");
	MakeDir(strFolderPath);
	m_pLogFileAccessor->Initialize(strLogFile);
	
	// sequence step -
	m_vecSeqStep.clear();
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassReadyStep*>(new CDoWorkGlassReadyStep_None(this)));
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassReadyStep*>(new CDoWorkGlassReadyStep_ChangeLoadStatus(this)));
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassReadyStep*>(new CDoWorkGlassReadyStep_CheckLoadStatus(this)));
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassReadyStep*>(new CDoWorkGlassReadyStep_LoadGlassInfoFromCIM(this)));
//	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassReadyStep*>(new CDoWorkGlassReadyStep_LoadRecipe(this)));
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassReadyStep*>(new CDoWorkGlassReadyStep_Idle(this)));

    SetTotalStepCount((long)m_vecSeqStep.size());
	SetExitStepIndex(GetTotalStepCount() - 1);
}

CDoWorkGlassReady::~CDoWorkGlassReady()
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

CDoWorkGlassReadyStep* CDoWorkGlassReady::GetSeqItemStep()
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
// INTERFACE 관련
//=========================================================================
bool CDoWorkGlassReady::CreateDoWorkSeq()
{
	return true;
}

void CDoWorkGlassReady::DeleteDoWorkSeq()
{
}

bool CDoWorkGlassReady::ChangeState()
{
	DeleteDoWorkSeq();

	return CreateDoWorkSeq();
}

bool CDoWorkGlassReady::UpdateState()
{
	return true;
}

bool CDoWorkGlassReady::LinkDataBase(CBinaryHelper *pBinaryHelper, bool bSave)
{
	return true;
}
	
void CDoWorkGlassReady::ThreadRun()
{
	GoToInitStep();

	while (true)
	{
		CDoWorkGlassReadyStep *pSeqItem = GetSeqItemStep();
		if (pSeqItem)
		{
			SetBackupStepIndex(GetCurStep());

			m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeST(0, pSeqItem->GetName(), GetCurStep());

			if (pSeqItem->Run() == true) // 동작
			{
				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(0, pSeqItem->GetName(), GetCurStep());

				GoToNextStep();
			}
			else if (GetCurStep() == GetExitStepIndex())
			{
				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(0, pSeqItem->GetName(), GetCurStep());

				CString strLog;
				strLog.Format(_T("CDoWorkGlassReadyStep Thread End"));
				m_pLogFileAccessor->KMLOG(INSP, strLog);

				return;
			}
			else
			{
				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(0, pSeqItem->GetName(), GetCurStep());

				CString strLog;
				strLog.Format(_T("CDoWorkGlassReadyStep Thread NG, Step = %d, Exit = %d"), GetCurStep(), GetExitStepIndex());
				m_pLogFileAccessor->KMLOG(WARN, strLog);

				GoToExitStep();
				CDoWorkGlassReadyStep *pSeqItem = GetSeqItemStep();
				if (pSeqItem)
				{
					pSeqItem->Run();
				}

				return;
			}

			if (GetStopUser() == true)	// 2023.08.09 add
			{
				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(0, pSeqItem->GetName(), GetBackupStepIndex());

				CString strLog;
				strLog.Format(_T("CDoWorkGlassReadyStep Thread Stop, Step = %d, Exit = %d"), GetBackupStepIndex(), GetExitStepIndex());
				m_pLogFileAccessor->KMLOG(WARN, strLog);

				GoToExitStep();
				CDoWorkGlassReadyStep* pSeqItem = GetSeqItemStep();
				if (pSeqItem)
				{
					pSeqItem->Run();
				}

				return;
			}
		}
	}

	//for (size_t i = 0; i < m_vecSeqStep.size(); ++i)
	//{
	//	CDoWorkGlassReadyStep *pSeqItem = m_vecSeqStep[i];
	//	if (pSeqItem)
	//	{
	//		if (pSeqItem->Run() == true)
	//		{
	//			GoToNextStep();
	//		}
	//		else
	//		{
	//			CString strLog;
	//			strLog.Format(_T("CDoWorkGlassReady Thread NG, Step = %d"), GetCurStep());
	//			m_pLogFileAccessor->KMLOG(INSP, strLog);

	//			return;
	//		}
	//	}
	//}
}
