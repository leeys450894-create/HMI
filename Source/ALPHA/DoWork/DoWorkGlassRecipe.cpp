#include "pch.h"
#include "DoWorkGlassRecipe.h"

#include "DoWorkGlassRecipeStep.h"
#include "..\SystemParamShare.h"
#include "LogFunction\LogAccessor\LogFileAccessor.h"
#include "..\AuroraFuncDefine.h"

#include "..\Manager\SeqTactTimeMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CDoWorkGlassRecipe::CDoWorkGlassRecipe(stAuroraMgrPool& MgrPool)
	: CDoWorkGlassSequenceImpl(MgrPool, enumSeqItemID_GLASS_RECIPE_CHANGE)
	, m_pShareItem(this)
{
	m_strSeqType = _T("SEQUENCE_ITEM_DWGLASS_SEQ_GLASS_RECIPE_CHANGE");
	m_strSeqComm = _T("TCPIP");
	m_strSeqCorp = _T("HBS");
	m_strSeqName = _T("GLASS 2_RECIPE_CHANGE");
	m_strSeqGUID = _T("EFCB7C47-9A68-41D7-BB50-B8BA5D7CFFC9");
	m_strSeqDesc = _T("GLASS RECIPE CHANGE");

	// 로그 생성하기
	const CString strFolderPath = CSystemParamShare::GetInstance()->GetLoggerFolderPath() + _T("\\LogFile\\");
	const CString strLogFile = strFolderPath + m_strSeqName + _T(".log");
	MakeDir(strFolderPath);
	m_pLogFileAccessor->Initialize(strLogFile);

	// sequence step -
	m_vecSeqStep.clear();
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassRecipeStep*>(new CDoWorkGlassRecipeStep_None(this)));
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassRecipeStep*>(new CDoWorkGlassRecipeStep_Change(this)));		// 레시피 변경 및 연결 모듈로 레시피 변경
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassRecipeStep*>(new CDoWorkGlassRecipeStep_ProbeCheck(this))); // probe 변경 확인
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassRecipeStep*>(new CDoWorkGlassRecipeStep_Idle(this)));

	SetTotalStepCount((long)m_vecSeqStep.size());
	SetExitStepIndex(GetTotalStepCount() - 1);
}

CDoWorkGlassRecipe::~CDoWorkGlassRecipe()
{
	// sequence step -
	for (size_t i = 0; i < m_vecSeqStep.size(); ++i)
	{
		if (m_vecSeqStep[i])
		{
			delete m_vecSeqStep[i];
			m_vecSeqStep[i] = nullptr;
		}
	}

	DeleteDoWorkSeq();
}

CDoWorkGlassRecipeStep* CDoWorkGlassRecipe::GetSeqItemStep()
{
	if (GetCurStep() < (long)m_vecSeqStep.size())
	{
		if (m_vecSeqStep[GetCurStep()])
		{
			return m_vecSeqStep[GetCurStep()];
		}
	}

	return nullptr;
}






//=========================================================================
// INTERFACE 관련
//=========================================================================
bool CDoWorkGlassRecipe::CreateDoWorkSeq()
{
	return true;
}

void CDoWorkGlassRecipe::DeleteDoWorkSeq()
{
}

bool CDoWorkGlassRecipe::ChangeState()
{
	DeleteDoWorkSeq();

	return CreateDoWorkSeq();
}

bool CDoWorkGlassRecipe::UpdateState()
{
	return true;
}

bool CDoWorkGlassRecipe::LinkDataBase(CBinaryHelper* pBinaryHelper, bool bSave)
{
	return true;
}

void CDoWorkGlassRecipe::ThreadRun()
{
	GoToInitStep();

	for (size_t i = 0; i < m_vecSeqStep.size(); ++i)
	{
		CDoWorkGlassRecipeStep* pSeqItem = m_vecSeqStep[i];
		if (pSeqItem)
		{
			SetBackupStepIndex(GetCurStep());

			m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeST(1, pSeqItem->GetName(), GetCurStep());

			if (pSeqItem->Run() == true) // 동작
			{
				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(1, pSeqItem->GetName(), GetCurStep());

				GoToNextStep();
			}
			else if (GetCurStep() == GetExitStepIndex())
			{
				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(1, pSeqItem->GetName(), GetCurStep());

				CString strLog;
				strLog.Format(_T("CDoWorkGlassRecipe Thread End"));
				m_pLogFileAccessor->KMLOG(INSP, strLog);
				break;
			}
			else
			{
				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(1, pSeqItem->GetName(), GetCurStep());

				CString strLog;
				strLog.Format(_T("CDoWorkGlassRecipe Thread NG, Step = %d, Exit = %d"), GetCurStep(), GetExitStepIndex());
				m_pLogFileAccessor->KMLOG(WARN, strLog);

				GoToExitStep();
				CDoWorkGlassRecipeStep* pSeqItem = GetSeqItemStep();
				if (pSeqItem)
				{
					pSeqItem->Run();
				}

				return;
			}

			if (GetStopUser() == true)	// 2023.08.09 add
			{
				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(1, pSeqItem->GetName(), GetBackupStepIndex());

				CString strLog;
				strLog.Format(_T("CDoWorkGlassRecipe Thread NG, Step = %d, Exit = %d"), GetBackupStepIndex(), GetExitStepIndex());
				m_pLogFileAccessor->KMLOG(WARN, strLog);

				GoToExitStep();
				CDoWorkGlassRecipeStep* pSeqItem = GetSeqItemStep();
				if (pSeqItem)
				{
					pSeqItem->Run();
				}

				return;
			}
		}
	}
}
