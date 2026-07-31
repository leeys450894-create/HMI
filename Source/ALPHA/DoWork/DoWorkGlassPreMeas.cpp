#include "pch.h"
#include "DoWorkGlassPreMeas.h"

#include "DoWorkGlassPreMeasStep.h"

#include "..\SystemParamShare.h"
#include "LogFunction\LogAccessor\LogFileAccessor.h"
#include "..\AuroraFuncDefine.h"

#include "MeasureSequenceItemEngine.h"
#include "..\Manager\SeqTactTimeMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CDoWorkGlassPreMeas::CDoWorkGlassPreMeas(stAuroraMgrPool& MgrPool)
	: CDoWorkGlassSequenceImpl(MgrPool, enumSeqItemID_GLASS_PRE_MEAS)
	, m_pMeasSeqItemEngine(new CMeasureSequenceItemEngine(MgrPool))
	, m_pShareItem(this)
{
	m_strSeqType = _T("SEQUENCE_ITEM_DWGLASS_SEQ_GLASS_PRE_MEAS");
	m_strSeqComm = _T("TCPIP");
	m_strSeqCorp = _T("KMAC");
	m_strSeqName = _T("GLASS 4_PRE_MEAS");
	m_strSeqGUID = _T("C415D3D0-E006-4963-ABF3-DEC16888CA24");
	m_strSeqDesc = _T("GLASS PRE_MEAS");

	// 로그 생성하기
	const CString strFolderPath = CSystemParamShare::GetInstance()->GetLoggerFolderPath() + _T("\\LogFile\\");
	const CString strLogFile = strFolderPath + m_strSeqName + _T(".log");
	MakeDir(strFolderPath);
	m_pLogFileAccessor->Initialize(strLogFile);

	// sequence step -
	m_vecSeqStep.clear();
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassPreMeasStep*>(new CDoWorkGlassPreMeasStep_None(this, m_pMeasSeqItemEngine)));

	// jsyoon 2022.09.27 change 순서 변경 PG 모델 다운 시간 오래 걸림
/* Before
//	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassPreMeasStep*>(new CDoWorkGlassPreMeasStep_RcpChangeDone(this, m_pMeasSeqItemEngine)));  // 모듈 레시피 변경완료 확인
//	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassPreMeasStep*>(new CDoWorkGlassPreMeasStep_P2P(this, m_pMeasSeqItemEngine)));			// Probe Pin to Pin
//	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassPreMeasStep*>(new CDoWorkGlassPreMeasStep_Move(this, m_pMeasSeqItemEngine)));			// PAD Pattern 위치 이동
//	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassPreMeasStep*>(new CDoWorkGlassPreMeasStep_PtrnSearch(this, m_pMeasSeqItemEngine)));		// PAD Pattern Search 
// NEW */
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassPreMeasStep*>(new CDoWorkGlassPreMeasStep_Move(this, m_pMeasSeqItemEngine)));			// PAD Pattern 위치 이동
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassPreMeasStep*>(new CDoWorkGlassPreMeasStep_PtrnSearch(this, m_pMeasSeqItemEngine)));		// PAD Pattern Search 
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassPreMeasStep*>(new CDoWorkGlassPreMeasStep_RcpChangeDone(this, m_pMeasSeqItemEngine)));  // 모듈 레시피 변경완료 확인
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassPreMeasStep*>(new CDoWorkGlassPreMeasStep_P2P(this, m_pMeasSeqItemEngine)));			// Probe Pin to Pin
///////////////

	//m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassPreMeasStep*>(new CDoWorkGlassPreMeasStep_Contact(this, m_pMeasSeqItemEngine)));			// Probe Connect
	//m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassPreMeasStep*>(new CDoWorkGlassPreMeasStep_OST(this, m_pMeasSeqItemEngine)));				// Probe Open Short Test
	//m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassPreMeasStep*>(new CDoWorkGlassPreMeasStep_RTEST(this, m_pMeasSeqItemEngine)));			// Probe R-Test
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassPreMeasStep*>(new CDoWorkGlassPreMeasStep_Idle(this, m_pMeasSeqItemEngine)));

	
	SetTotalStepCount((long)m_vecSeqStep.size());
	SetLoopStepIndex(0);  // LYS 2026-06-30 Stick 수 초기화 (ThreadRun에서 사용)
	SetExitStepIndex(GetTotalStepCount() - 1);
}

CDoWorkGlassPreMeas::~CDoWorkGlassPreMeas()
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

CDoWorkGlassPreMeasStep* CDoWorkGlassPreMeas::GetSeqItemStep()
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
bool CDoWorkGlassPreMeas::CreateDoWorkSeq()
{
	return true;
}

void CDoWorkGlassPreMeas::DeleteDoWorkSeq()
{
}

bool CDoWorkGlassPreMeas::ChangeState()
{
	DeleteDoWorkSeq();

	return CreateDoWorkSeq();
}

bool CDoWorkGlassPreMeas::UpdateState()
{
	return true;
}

bool CDoWorkGlassPreMeas::LinkDataBase(CBinaryHelper* pBinaryHelper, bool bSave)
{
	return true;
}

void CDoWorkGlassPreMeas::ThreadRun()
{
	GoToInitStep();

	for (size_t i = 0; i < m_vecSeqStep.size(); ++i)
	{
		CDoWorkGlassPreMeasStep* pSeqItem = m_vecSeqStep[i];
		if (pSeqItem)
		{
			SetBackupStepIndex(GetCurStep());

			m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeST(1, pSeqItem->GetName(), GetCurStep());

			if (pSeqItem->Run() == true) // 성공
			{
				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(1, pSeqItem->GetName(), GetCurStep());

				GoToNextStep();
			}
			else if (GetCurStep() == GetExitStepIndex())
			{
				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(1, pSeqItem->GetName(), GetCurStep());

				CString strLog;
				strLog.Format(_T("CDoWorkGlassPreMeasStep Thread End"));
				m_pLogFileAccessor->KMLOG(INSP, strLog);
				break;
			}
			else
			{
				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(1, pSeqItem->GetName(), GetCurStep());

				CString strLog;
				strLog.Format(_T("CDoWorkGlassPreMeasStep Thread NG, Step = %d, Exit = %d"), GetCurStep(), GetExitStepIndex());
				m_pLogFileAccessor->KMLOG(WARN, strLog);

				GoToExitStep();
				CDoWorkGlassPreMeasStep* pSeqItem = GetSeqItemStep();
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
				strLog.Format(_T("CDoWorkGlassPreMeasStep Thread Stop, Step = %d, Exit = %d"), GetBackupStepIndex(), GetExitStepIndex());
				m_pLogFileAccessor->KMLOG(WARN, strLog);

				GoToExitStep();
				CDoWorkGlassPreMeasStep* pSeqItem = GetSeqItemStep();
				if (pSeqItem)
				{
					pSeqItem->Run();
				}

				return;
			}
		}
	}
}