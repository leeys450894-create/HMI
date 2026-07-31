#include "pch.h"
#include "DoWorkProbeSWAlign.h"

#include "DoWorkProbeSWAlignStep.h"
#include "..\SystemParamShare.h"
#include "LogFunction\LogAccessor\LogFileAccessor.h"
#include "..\AuroraFuncDefine.h"
#include "..\Manager\SeqTactTimeMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CDoWorkProbeSWAlign::CDoWorkProbeSWAlign(stAuroraMgrPool& MgrPool)
	: CDoWorkGlassSequenceImpl(MgrPool, enumSeqItemID_PROBE_SW_ALIGN)
	, m_pShareItem(this)
{
	m_strSeqType = _T("SEQUENCE_ITEM_DWGLASS_SEQ_PROBE_SW_ALIGN");
	m_strSeqComm = _T("TCPIP");
	m_strSeqCorp = _T("KMAC");
	m_strSeqName = _T("GLASS 3_PROBE_ALIGN");
//	m_strSeqName = _T("SEQ PROBE ALIGN");
	m_strSeqGUID = _T("02D1B398-6EDA-42B1-B990-027FBD8E6E82");
	m_strSeqDesc = _T("PROBE SW ALIGN");

	// 로그 생성하기
	const CString strFolderPath = CSystemParamShare::GetInstance()->GetLoggerFolderPath() + _T("\\LogFile\\");
	const CString strLogFile = strFolderPath + m_strSeqName + _T(".log");
	MakeDir(strFolderPath);
	m_pLogFileAccessor->Initialize(strLogFile);

	// sequence step -
	m_vecSeqStep.clear();
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkProbeSWAlignStep*>(new CDoWorkProbeSWAlignStep_None(this)));
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkProbeSWAlignStep*>(new CDoWorkProbeSWAlignStep_Move(this)));		// 좌표 이동
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkProbeSWAlignStep*>(new CDoWorkProbeSWAlignStep_Align(this)));	// Probe Align
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkProbeSWAlignStep*>(new CDoWorkProbeSWAlignStep_Idle(this)));

	SetTotalStepCount((long)m_vecSeqStep.size());
	SetExitStepIndex(GetTotalStepCount() - 1);
}

CDoWorkProbeSWAlign::~CDoWorkProbeSWAlign()
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

CDoWorkProbeSWAlignStep* CDoWorkProbeSWAlign::GetSeqItemStep()
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
bool CDoWorkProbeSWAlign::CreateDoWorkSeq()
{
	return true;
}

void CDoWorkProbeSWAlign::DeleteDoWorkSeq()
{
}

bool CDoWorkProbeSWAlign::ChangeState()
{
	DeleteDoWorkSeq();

	return CreateDoWorkSeq();
}

bool CDoWorkProbeSWAlign::UpdateState()
{
	return true;
}

bool CDoWorkProbeSWAlign::LinkDataBase(CBinaryHelper* pBinaryHelper, bool bSave)
{
	return true;
}

void CDoWorkProbeSWAlign::ThreadRun()
{
	GoToInitStep();

	while (true)
	{
		CDoWorkProbeSWAlignStep* pSeqItem = GetSeqItemStep();
		if (pSeqItem)
		{
			/*if (GetCurStep() > 0)
			{
				if (pSeqItem->GetComplete() == true)
				{
					GoToExitStep();

					CDoWorkProbeSWAlignStep* pSeqItem = GetSeqItemStep();
					if (pSeqItem)
					{
						pSeqItem->Run();
					}

					CString strLog;
					strLog.Format(_T("CDoWorkGlassSWAlignStep Thread End"));
					m_pLogFileAccessor->KMLOG(INSP, strLog);

					return;
				}
			}*/

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
				strLog.Format(_T("CDoWorkProbeSWAlignStep Thread End"));
				m_pLogFileAccessor->KMLOG(INSP, strLog);

				return;
			}
			else
			{
				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(0, pSeqItem->GetName(), GetCurStep());

				CString strLog;
				strLog.Format(_T("CDoWorkProbeSWAlignStep Thread NG, Step = %d, Exit = %d"), GetCurStep(), GetExitStepIndex());
				m_pLogFileAccessor->KMLOG(WARN, strLog);

				GoToExitStep();
				CDoWorkProbeSWAlignStep* pSeqItem = GetSeqItemStep();
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
				strLog.Format(_T("CDoWorkProbeSWAlignStep Thread Stop, Step = %d, Exit = %d"), GetBackupStepIndex(), GetExitStepIndex());
				m_pLogFileAccessor->KMLOG(WARN, strLog);

				GoToExitStep();
				CDoWorkProbeSWAlignStep* pSeqItem = GetSeqItemStep();
				if (pSeqItem)
				{
					pSeqItem->Run();
				}

				return;
			}
		}
	}
}