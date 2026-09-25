#include "pch.h"
#include "DoWorkGlassMeasure.h"

#include "MeasureSequenceItemEngine.h"
#include "DoWorkGlassMeasureStep.h"
#include "..\SystemParamShare.h"
#include "LogFunction\LogAccessor\LogFileAccessor.h"
#include "..\AuroraFuncDefine.h"
#include "..\Manager\SeqTactTimeMgr.h"
#include "..\Manager\MCCLogMgr.h"
#include "..\Manager\ResumeInfoMgr.h"
#include "..\Manager\SystemConfigMgr.h"
#include "..\Recipe\RecipeEditMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CDoWorkGlassMeasure::CDoWorkGlassMeasure(stAuroraMgrPool &MgrPool)
	: CDoWorkGlassSequenceImpl(MgrPool, enumSeqItemID_MEASURE)
	, m_pMeasSeqItemEngine(new CMeasureSequenceItemEngine(MgrPool))
	, m_pShareItem(this)
{
	m_strSeqType = _T("SEQUENCE_ITEM_DWGLASS_SEQ_MEASURE");
	m_strSeqComm = _T("TCPIP");
	m_strSeqCorp = _T("HBS");
	m_strSeqName = _T("GLASS 6_MEASURE");
	m_strSeqGUID = _T("03F88644-473D-4B03-9BC5-E09B679A4C57");
	m_strSeqDesc = _T("GLASS MEASURE");

	const CString strFolderPath	= MgrPool.strLogPath + _T("\\LogFile\\");
	const CString strLogFile	= strFolderPath + m_strSeqName + _T(".log");
	MakeDir(strFolderPath);
	m_pLogFileAccessor->Initialize(strLogFile);

	// sequence step -
	m_vecSeqStep.clear();
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassMeasureStep*>(new CDoWorkGlassMeasureStep_None(this, m_pMeasSeqItemEngine)));
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassMeasureStep*>(new CDoWorkGlassMeasureStep_MoveMeasureArea(this, m_pMeasSeqItemEngine)));		// Stick 이동(Probe UnContact)
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassMeasureStep*>(new CDoWorkGlassMeasureStep_PtrnSearch(this, m_pMeasSeqItemEngine)));				// Pattern Search
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassMeasureStep*>(new CDoWorkGlassMeasureStep_Probe_Contact(this, m_pMeasSeqItemEngine)));			// Probe Contact

	// LYS 2026-06-30 Measure Step 추가 (OST, RTEST, AGING)
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassMeasureStep*>(new CDoWorkGlassMeasureStep_OST(this, m_pMeasSeqItemEngine))); 
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassMeasureStep*>(new CDoWorkGlassMeasureStep_RTEST(this, m_pMeasSeqItemEngine)));
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassMeasureStep*>(new CDoWorkGlassMeasureStep_AGING(this, m_pMeasSeqItemEngine)));
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassMeasureStep*>(new CDoWorkGlassMeasureStep_AGING_PTN(this, m_pMeasSeqItemEngine)));	// LYS 2026-08-06 AgingCycle E3: per-item lighting

	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassMeasureStep*>(new CDoWorkGlassMeasureStep_Measure_WAD(this, m_pMeasSeqItemEngine)));			// 특성모드 이동 & 검사
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassMeasureStep*>(new CDoWorkGlassMeasureStep_Measure_ANGLE(this, m_pMeasSeqItemEngine)));			// 시야각위치 이동 & 검사
	//m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassMeasureStep*>(new CDoWorkGlassMeasureStep_Measure_TSP(this, m_pMeasSeqItemEngine)));			// TSP 모드 검사
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassMeasureStep*>(new CDoWorkGlassMeasureStep_Measure_VISION(this, m_pMeasSeqItemEngine)));			// 화질모드 이동 & 검사
	//m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassMeasureStep*>(new CDoWorkGlassMeasureStep_Measure_COLOR(this, m_pMeasSeqItemEngine)));			// 면휘도모드 이동 & 검사
	//m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassMeasureStep*>(new CDoWorkGlassMeasureStep_Measure_VISION_COLOR(this, m_pMeasSeqItemEngine)));	// 화질모드 + 면휘도모드 이동 & 검사
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassMeasureStep*>(new CDoWorkGlassMeasureStep_Idle(this, m_pMeasSeqItemEngine)));				

    SetTotalStepCount((long)m_vecSeqStep.size());
	SetLoopStepIndex(0);
	SetExitStepIndex(GetTotalStepCount() - 1);
}

CDoWorkGlassMeasure::~CDoWorkGlassMeasure()
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

	if(m_pMeasSeqItemEngine)
	{
		delete m_pMeasSeqItemEngine;
		m_pMeasSeqItemEngine = nullptr;
	}

	DeleteDoWorkSeq();
}

CDoWorkGlassMeasureStep* CDoWorkGlassMeasure::GetSeqItemStep()
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
bool CDoWorkGlassMeasure::CreateDoWorkSeq()
{
	return true;
}

void CDoWorkGlassMeasure::DeleteDoWorkSeq()
{
}

bool CDoWorkGlassMeasure::ChangeState()
{
	DeleteDoWorkSeq();

	return CreateDoWorkSeq();
}

bool CDoWorkGlassMeasure::UpdateState()
{
	return true;
}

bool CDoWorkGlassMeasure::LinkDataBase(CBinaryHelper *pBinaryHelper, bool bSave)
{
	if(m_pMeasSeqItemEngine)
	{
		return m_pMeasSeqItemEngine->LinkDataBase(pBinaryHelper, bSave);
	}

	return false;
}

void CDoWorkGlassMeasure::ThreadRun()
{
	bool bExis = false;
	CString strName;
	int nStepIndex = 0;
	nStepIndex = GetLoopStepIndex();

	if (nStepIndex == 0)
	{
		ResetEvent(m_pShareItem->GetStartEventHandle());  

		SetEvent(m_pShareItem->GetCompleteEventHandle()); // 시퀀스 완료 이벤트 SET
		return;
	}

	// LYS 2026-07-03 Resume: start from saved stick index if resume mode
	size_t nStartStick = 0;
	if (CSystemParamShare::GetInstance()->IsResumeMode())
	{
		nStartStick = (size_t)CSystemParamShare::GetInstance()->GetResumeStartStick();
		if (nStartStick >= (size_t)nStepIndex) nStartStick = 0;
		CString strLog;
		strLog.Format(_T("Resume: starting from stick %d (total %d)"), (int)nStartStick, nStepIndex);
		m_pLogFileAccessor->KMLOG(OPER, strLog);
	}

	// LYS 2026-07-03 Resume: clear resume mode flag after applying start index
	CSystemParamShare::GetInstance()->SetResumeMode(false);

	// LYS 2026-08-06 AgingCycle E3: locate aging loop-back step indexes by name
	long nStepIdxAging = -1, nStepIdxVision = -1;
	for (size_t k = 0; k < m_vecSeqStep.size(); ++k)
	{
		if (m_vecSeqStep[k]->GetName() == _T("Aging"))  nStepIdxAging = (long)k;
		if (m_vecSeqStep[k]->GetName() == _T("VISION")) nStepIdxVision = (long)k;
	}

	for (size_t i = nStartStick; i < nStepIndex; i++)		// N Stick
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_STEP_MEAS_N, true, 0);
		
		GoToInitStep();  // Init

		// LYS 2026-08-06 AgingCycle E3: per-stick aging-list cycle begin (OCP DoWorkStartMeasOCP_A)
		bool bAgingCycle = false;
		if (m_pMeasSeqItemEngine != NULL && nStepIdxAging >= 0 && nStepIdxVision >= 0)
			bAgingCycle = m_pMeasSeqItemEngine->AgingCycleBegin();

		while (true)
		{
			CDoWorkGlassMeasureStep* pSeqItem = GetSeqItemStep();
			if (pSeqItem)
			{
				SetBackupStepIndex(GetCurStep());

				if (GetCurStep() > 0 && GetCurStep() < GetExitStepIndex() )
					strName.Format(_T("Stick%d_%s"), i + 1, pSeqItem->GetName());
				else
					strName.Empty();

				pSeqItem->SetMeasureStick(i);  // 현재측정 Stick 설정

				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeST(1, strName, GetCurStep());	// Start Time

				if (i + 1 == GetLoopStepIndex())	// Last Stick
				{
					if (pSeqItem->Run() == true) // 동작
					{
						m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(1, strName, GetCurStep());	// End Time

						// LYS 2026-08-06 AgingCycle E3: more aging items -> jump back to AGING step
						if (bAgingCycle && GetCurStep() == nStepIdxVision && m_pMeasSeqItemEngine->AgingCycleNextItem())
						{
							GoToStep(nStepIdxAging);
							continue;
						}

						GoToNextStep();
					}
					else if (GetCurStep() == GetExitStepIndex())
					{
						m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(1, strName, GetCurStep());	// End Time

						CString strLog;
						strLog.Format(_T("CDoWorkGlassMeasure Thread End"));
						m_pLogFileAccessor->KMLOG(INSP, strLog);
						break;
					}
					else
					{
						m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(1, strName, GetCurStep());	// End Time

						CString strLog;
						strLog.Format(_T("CDoWorkGlassMeasure Thread NG, Step = %d, Exit = %d"), GetCurStep(), GetExitStepIndex());
						m_pLogFileAccessor->KMLOG(WARN, strLog);

						GoToExitStep();
						CDoWorkGlassMeasureStep* pSeqItem = GetSeqItemStep();
						if (pSeqItem)
						{
							pSeqItem->Run();
						}

						bExis = true;
						break;
					}
				}
				else
				{
					if (pSeqItem->Run() == true) // 동작
					{
						m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(1, strName, GetCurStep());  // End Time

							// LYS 2026-08-06 AgingCycle E3: more aging items -> jump back to AGING step
							if (bAgingCycle && GetCurStep() == nStepIdxVision && m_pMeasSeqItemEngine->AgingCycleNextItem())
							{
								GoToStep(nStepIdxAging);
								continue;
							}

						if (GetCurStep() == GetExitStepIndex() - 1)  
						{
							break;	// Next Stick 측정
						}
						else
						{
							GoToNextStep();
						}
					}
					else
					{
						m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(1, strName, GetCurStep());	// End Time

						CString strLog;
						strLog.Format(_T("CDoWorkGlassMeasure Thread NG, Step = %d, Exit = %d"), GetCurStep(), GetExitStepIndex());
						m_pLogFileAccessor->KMLOG(WARN, strLog);

						GoToExitStep();
						CDoWorkGlassMeasureStep* pSeqItem = GetSeqItemStep();
						if (pSeqItem)
						{
							pSeqItem->Run();
						}

						// NG 발생 시 측정종료
						bExis = true;
						break;
					}
				}

				if (GetStopUser() == true)	// 2023.08.09 add
				{
					m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(1, strName, GetBackupStepIndex());	// End Time

					CString strLog;
					strLog.Format(_T("CDoWorkGlassMeasure Thread Stop, Step = %d, Exit = %d"), GetBackupStepIndex(), GetExitStepIndex());
					m_pLogFileAccessor->KMLOG(WARN, strLog);

					GoToExitStep();
					CDoWorkGlassMeasureStep* pSeqItem = GetSeqItemStep();
					if (pSeqItem)
					{
						pSeqItem->Run();
					}

					// NG 발생 시 측정종료
					bExis = true;
					break;
				}
			}
		}

		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_STEP_MEAS_N, false, 0);

		// LYS 2026-08-06 AgingCycle E3: per-stick aging cycle end (progress reset)
		if (m_pMeasSeqItemEngine != NULL) m_pMeasSeqItemEngine->AgingCycleEnd();

		// LYS 2026-07-03 Resume Save: stick completed
		if (!bExis)
		{
			CResumeInfoMgr* pResume = m_pShareItem->GetResumeInfoMgrPtr();
			if (pResume && m_pShareItem->GetSystemConfigMgrPtr()->IsUseResumeInspection())
			{
				pResume->UpdateStickProgress((int)i);
			}
		}

		if (bExis == true)
		{
			break;
		}
	}
}
