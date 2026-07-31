#include "pch.h"
#include "DoWorkGlassAGING.h"

#include "DoWorkGlassAGINGStep.h"

#include "..\SystemParamShare.h"
#include "LogFunction\LogAccessor\LogFileAccessor.h"
#include "..\AuroraFuncDefine.h"

#include "MeasureSequenceItemEngine.h"
#include "..\Manager\SeqTactTimeMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CDoWorkGlassAGING::CDoWorkGlassAGING(stAuroraMgrPool& MgrPool)
	: CDoWorkGlassSequenceImpl(MgrPool, enumSeqItemID_GLASS_AGING)
	, m_pMeasSeqItemEngine(new CMeasureSequenceItemEngine(MgrPool))
	, m_pShareItem(this)
{
	m_strSeqType = _T("SEQUENCE_ITEM_DWGLASS_SEQ_GLASS_AGING");
	m_strSeqComm = _T("TCPIP");
	m_strSeqCorp = _T("KMAC");
	m_strSeqName = _T("GLASS 5_AGING");
	m_strSeqGUID = _T("FEF05587-4BC1-4F12-AE84-8C881B78D8D9");
	m_strSeqDesc = _T("GLASS AGING");

	// 로그 생성하기
	const CString strFolderPath = CSystemParamShare::GetInstance()->GetLoggerFolderPath() + _T("\\LogFile\\");
	const CString strLogFile = strFolderPath + m_strSeqName + _T(".log");
	MakeDir(strFolderPath);
	m_pLogFileAccessor->Initialize(strLogFile);

	// sequence step -
	m_vecSeqStep.clear();
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassAGINGStep*>(new CDoWorkGlassAGINGStep_None(this, m_pMeasSeqItemEngine)));
	//m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassAGINGStep*>(new CDoWorkGlassAGINGStep_Move_First(this, m_pMeasSeqItemEngine)));		// Aging Probe 이동
	//m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassAGINGStep*>(new CDoWorkGlassAGINGStep_Aging(this, m_pMeasSeqItemEngine)));			// Aging Test
	//m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassAGINGStep*>(new CDoWorkGlassAGINGStep_Move_Second(this, m_pMeasSeqItemEngine)));	// 나머지 Stick 이동
	//m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassAGINGStep*>(new CDoWorkGlassAGINGStep_Aging_Second(this, m_pMeasSeqItemEngine)));	// 나머지 Aging Test
	//m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassAGINGStep*>(new CDoWorkGlassAGINGStep_Move_3(this, m_pMeasSeqItemEngine)));			// 나머지 Stick 이동
	//m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassAGINGStep*>(new CDoWorkGlassAGINGStep_Aging_3(this, m_pMeasSeqItemEngine)));		// 나머지 Aging Test
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassAGINGStep*>(new CDoWorkGlassAGINGStep_Idle(this, m_pMeasSeqItemEngine))); 


	SetTotalStepCount((long)m_vecSeqStep.size());
	SetExitStepIndex(GetTotalStepCount() - 1);
}

CDoWorkGlassAGING::~CDoWorkGlassAGING()
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

	// 2023.03.07 add
	if (m_pMeasSeqItemEngine != nullptr)
	{
		delete m_pMeasSeqItemEngine;
		m_pMeasSeqItemEngine = nullptr;
	}

	DeleteDoWorkSeq();
}

CDoWorkGlassAGINGStep* CDoWorkGlassAGING::GetSeqItemStep()
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
bool CDoWorkGlassAGING::CreateDoWorkSeq()
{
	return true;
}

void CDoWorkGlassAGING::DeleteDoWorkSeq()
{
}

bool CDoWorkGlassAGING::ChangeState()
{
	DeleteDoWorkSeq();

	return CreateDoWorkSeq();
}

bool CDoWorkGlassAGING::UpdateState()
{
	return true;
}

bool CDoWorkGlassAGING::LinkDataBase(CBinaryHelper* pBinaryHelper, bool bSave)
{
	return true;
}

void CDoWorkGlassAGING::ThreadRun()
{
	GoToInitStep();

	for (size_t i = 0; i < m_vecSeqStep.size(); ++i)
	{
		CDoWorkGlassAGINGStep* pSeqItem = m_vecSeqStep[i];
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
				strLog.Format(_T("CDoWorkGlassAGING Thread End"));
				m_pLogFileAccessor->KMLOG(INSP, strLog);
				break;
			}
			else
			{
				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(1, pSeqItem->GetName(), GetCurStep());

				CString strLog;
				strLog.Format(_T("CDoWorkGlassAGING Thread NG, Step = %d, Exit = % d"), GetCurStep(), GetExitStepIndex());
				m_pLogFileAccessor->KMLOG(WARN, strLog);

				GoToExitStep();
				CDoWorkGlassAGINGStep* pSeqItem = GetSeqItemStep();
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
				strLog.Format(_T("CDoWorkGlassAGING Thread STop, Step = %d, Exit = % d"), GetBackupStepIndex(), GetExitStepIndex());
				m_pLogFileAccessor->KMLOG(WARN, strLog);

				GoToExitStep();
				CDoWorkGlassAGINGStep* pSeqItem = GetSeqItemStep();
				if (pSeqItem)
				{
					pSeqItem->Run();
				}

				return;
			}
		}
	}
}