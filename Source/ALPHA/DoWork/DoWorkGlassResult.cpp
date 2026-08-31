#include "pch.h"
#include "DoWorkGlassResult.h"

#include "DoWorkGlassResultStep.h"
#include "..\SystemParamShare.h"
#include "LogFunction\LogAccessor\LogFileAccessor.h"
#include "..\AuroraFuncDefine.h"

#include "..\Manager\SeqTactTimeMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CDoWorkGlassResult::CDoWorkGlassResult(stAuroraMgrPool &MgrPool)
	: CDoWorkGlassSequenceImpl(MgrPool, enumSeqItemID_GLASS_RESULT)
	, m_pShareItem(this)
{
	m_strSeqType = _T("SEQUENCE_ITEM_DWGLASS_SEQ_GLASS_RESULT");
	m_strSeqComm = _T("TCPIP");
	m_strSeqCorp = _T("HBS");
	m_strSeqName = _T("GLASS 7_RESULT");
	m_strSeqGUID = _T("01628587-C19B-490A-A20B-7696BD54A9ED");
	m_strSeqDesc = _T("GLASS RESULT");

	// 로그 생성하기
	const CString strFolderPath	= CSystemParamShare::GetInstance()->GetLoggerFolderPath() + _T("\\LogFile\\");
	const CString strLogFile	= strFolderPath + m_strSeqName + _T(".log");
	MakeDir(strFolderPath);
	m_pLogFileAccessor->Initialize(strLogFile);

	// sequence step -
	m_vecSeqStep.clear();
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassResultStep*>(new CDoWorkGlassResultStep_None(this)));
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassResultStep*>(new CDoWorkGlassResultStep_CompleteCheck(this)));	// 결과파일 및 APD 파일 생성 호출
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassResultStep*>(new CDoWorkGlassResultStep_Idle(this)));
	
	//// sequence step -
	//m_vecSeqStep.clear();
	//m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassResultStep*>(new CDoWorkGlassResultStep_None(this)));
	////m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassResultStep*>(new CDoWorkGlassResultStep_InitPosition(this)));
	////m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassResultStep*>(new CDoWorkGlassResultStep_InitPosition2(this)));
	////m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassResultStep*>(new CDoWorkGlassResultStep_InitPositionCheck(this)));
	////m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassResultStep*>(new CDoWorkGlassResultStep_Complete(this)));
	//m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassResultStep*>(new CDoWorkGlassResultStep_CompleteCheck(this)));
	////m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassResultStep*>(new CDoWorkGlassResultStep_Exit(this)));
	////m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassResultStep*>(new CDoWorkGlassResultStep_ExitCheck(this)));
	//m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassResultStep*>(new CDoWorkGlassResultStep_Idle(this)));

	//202103122
	/*
	DoWorkGlassResult - 결과 파일 취합 및 업로드
						결과는 이전 측정 시퀀스에서 모드별로 각자 저장 해야 될듯
	1. 결과 취합
	2. APD 또는 DFS 관련 파일 생성
	2.1. DVList 파일
	2.2. DFS 파일
	3. 업로드(조건)
	3.1. PC CIM인 경우 CIM PC로 업로드(파일, 이미지)
	3.2. PLC CIM인 경우 DFS 프로그램에서 업로드(설정 필요함)
	*/

    SetTotalStepCount((long)m_vecSeqStep.size());
	SetExitStepIndex(GetTotalStepCount() - 1);
}

CDoWorkGlassResult::~CDoWorkGlassResult()
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

CDoWorkGlassResultStep* CDoWorkGlassResult::GetSeqItemStep()
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
bool CDoWorkGlassResult::CreateDoWorkSeq()
{
	return true;
}

void CDoWorkGlassResult::DeleteDoWorkSeq()
{
}

bool CDoWorkGlassResult::ChangeState()
{
	DeleteDoWorkSeq();

	return CreateDoWorkSeq();
}

bool CDoWorkGlassResult::UpdateState()
{
	return true;
}

bool CDoWorkGlassResult::LinkDataBase(CBinaryHelper *pBinaryHelper, bool bSave)
{
	return true;
}
	
void CDoWorkGlassResult::ThreadRun()
{
	GoToInitStep();

	for (size_t i = 0; i < m_vecSeqStep.size(); ++i)
	{
		CDoWorkGlassResultStep *pSeqItem = m_vecSeqStep[i];
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
				strLog.Format(_T("CDoWorkGlassResult Thread End"));
				m_pLogFileAccessor->KMLOG(INSP, strLog);
				break;
			}
			else
			{
				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(1, pSeqItem->GetName(), GetCurStep());

				CString strLog;
				strLog.Format(_T("CDoWorkGlassResult Thread NG, Step = %d, Exit = %d"), GetCurStep(), GetExitStepIndex());
				m_pLogFileAccessor->KMLOG(WARN, strLog);

				GoToExitStep();
				CDoWorkGlassResultStep* pSeqItem = GetSeqItemStep();
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
				strLog.Format(_T("CDoWorkGlassResult Thread NG, Step = %d, Exit = %d"), GetBackupStepIndex(), GetExitStepIndex());
				m_pLogFileAccessor->KMLOG(WARN, strLog);

				GoToExitStep();
				CDoWorkGlassResultStep* pSeqItem = GetSeqItemStep();
				if (pSeqItem)
				{
					pSeqItem->Run();
				}

				return;
			}
		}
	}
}
