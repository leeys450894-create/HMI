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
#include "..\Manager\ShareMemoryMgr.h"		// LYS 2026-09-28 EFA
#include "..\Manager\CommTcpIpFactoryMgr.h"	// LYS 2026-09-28 EFA
#include "..\Manager\ResultMgr.h"			// LYS 2026-09-28 EFA
#include "..\CommTcpIpObjectCIM.h"			// LYS 2026-09-28 EFA

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

	// LYS 2026-09-28 EFA: 2-pass loop (OCP DoWorkStartMeasICP_EFA: item(pass) loop OUTSIDE the stick loop).
	// nPassCount = EFA rcp item count (normally 2: T0 before aging / T1 after aging), 1 = standard run.
	// Resume start stick applies to pass 0 only; T1+ always restarts from stick 0.
	int  nPassCount = 1;
	bool bEfaMode = false;
	if (m_pMeasSeqItemEngine != NULL && nStepIdxAging >= 0 && nStepIdxVision >= 0)
	{
		int nEfaPass = m_pMeasSeqItemEngine->GetEfaPassCount();
		if (nEfaPass > 0) { nPassCount = nEfaPass; bEfaMode = true; }
	}

	for (int nPass = 0; nPass < nPassCount; nPass++)
	{
		if (bEfaMode)
		{
			CString strPassLog;
			strPassLog.Format(_T("EFA: pass %d/%d start (T%d)"), nPass + 1, nPassCount, nPass);
			m_pLogFileAccessor->KMLOG(OPER, strPassLog);

			if (nPass >= 1 && EfaPassBoundary(nPass) == false)
			{
				m_pLogFileAccessor->KMLOG(WARN, _T("EFA: pass boundary failed/stopped - glass measure aborted"));
				bExis = true;
				break;
			}
		}

		for (size_t i = ((nPass == 0) ? nStartStick : 0); i < (size_t)nStepIndex; i++)		// N Stick (// LYS 2026-09-28 EFA: pass-aware start)
		{
			CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_STEP_MEAS_N, true, 0);
			
			GoToInitStep();  // Init

			// LYS 2026-08-06 AgingCycle E3: per-stick aging-list cycle begin (OCP DoWorkStartMeasOCP_A)
			bool bAgingCycle = false;
			if (m_pMeasSeqItemEngine != NULL && nStepIdxAging >= 0 && nStepIdxVision >= 0)
				bAgingCycle = bEfaMode ? m_pMeasSeqItemEngine->AgingCycleBegin(nPass) : m_pMeasSeqItemEngine->AgingCycleBegin();	// LYS 2026-09-28 EFA

			while (true)
			{
				CDoWorkGlassMeasureStep* pSeqItem = GetSeqItemStep();
				if (pSeqItem)
				{
					// LYS 2026-09-28 EFA: T1+ pass skips pre-inspection steps (OCP _EFA bInspCheck). Set = Pattern Search / OSTest / R-Test.
					if (bEfaMode && nPass >= 1 && IsEfaSkipStep(pSeqItem->GetName()))
					{
						CString strSkip;
						strSkip.Format(_T("EFA pass %d: skip step %s"), nPass + 1, (LPCTSTR)pSeqItem->GetName());
						m_pLogFileAccessor->KMLOG(INSP, strSkip);
						GoToNextStep();
						continue;
					}

					SetBackupStepIndex(GetCurStep());

					if (GetCurStep() > 0 && GetCurStep() < GetExitStepIndex() )
						strName.Format(_T("Stick%d_%s"), i + 1, pSeqItem->GetName());
					else
						strName.Empty();

					pSeqItem->SetMeasureStick(i);  // 현재측정 Stick 설정

					m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeST(1, strName, GetCurStep());	// Start Time

					// LYS 2026-09-28 EFA: the exit/Idle step (SetComplete + complete event) may run only on the FINAL pass.
					// On T0 the last stick ends like any other stick (break at Exit-1) so RESULT/UNLOAD cannot start while T1 runs.
					if (i + 1 == GetLoopStepIndex() && nPass == nPassCount - 1)	// Last Stick
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
		if (bExis == true) break;	// LYS 2026-09-28 EFA: stop remaining passes
	}
}

// LYS 2026-09-28 EFA: steps not repeated on T1+ (OCP _EFA bInspCheck skips ReviewSearch / pre-inspection).
// Fixed per user decision 2026-09-28: Pattern Search, OST, R-Test. Contact and the measurements repeat every pass.
bool CDoWorkGlassMeasure::IsEfaSkipStep(const CString& strName)
{
	return (strName == _T("Pattern Search") || strName == _T("OSTest") || strName == _T("R-Test"));
}

// LYS 2026-09-28 EFA: T0 -> T1 boundary (OCP _EFA AgingCnt==1 block: result save + APD FTP + EFA^0$ / EFA^0_OK$ handshake).
// 1) T0 result flush: same call as CDoWorkGlassResultStep_CompleteCheck (XLS + APD.txt). dResult does NOT clear
//    its grid, so T1 simply overwrites the same rows and the final Result step writes the T1 file (OCP behaviour).
//    SaveResult starts an async export thread -> wait for SaveResultDone before T1 writes into the grid.
// 2) ONLINE REMOTE only: APD FTP upload -> 500 ms -> EFA^0$ -> wait EFA^0_OK$ 5 s, continue on timeout (D-2).
// false = user stop (abort the glass). Save/CIM failures are logged and the glass continues, as in OCP.
bool CDoWorkGlassMeasure::EfaPassBoundary(int nPass)
{
	CString strLog;
	strLog.Format(_T("EFA: pass boundary before pass %d (T0 done) - result flush + CIM handshake"), nPass + 1);
	m_pLogFileAccessor->KMLOG(OPER, strLog);

	if (m_pShareItem->IfPauseEvt_WaitResumeEvt() != true) return false;	// user stop between passes

	// ---- 1) T0 result flush ----
	CString strStart;
	strStart.Format(_T("%s"), m_pShareItem->GetSeqTactTimeMgrPtr()->GetStartTime2());
	bool bSave = CResultMgr::GetInstance()->SaveResult(true, true, strStart);
	if (bSave)
	{
		// the export thread flips SaveResultDone false -> true; give it up to 1 s to start, then 30 s to finish
		DWORD dwS = GetTickCount();
		while (CResultMgr::GetInstance()->SaveResultDone() == true && GetTickCount() - dwS < 1000) Sleep(20);
		while (CResultMgr::GetInstance()->SaveResultDone() == false && GetTickCount() - dwS < 30000) Sleep(100);
		bSave = CResultMgr::GetInstance()->SaveResultDone();
	}
	strLog.Format(_T("EFA: T0 result flush %s"), bSave ? _T("OK") : _T("NG (UploadDataDone/export timeout) - continue"));
	m_pLogFileAccessor->KMLOG(bSave ? OPER : WARN, strLog);

	// ---- 2) CIM handshake (ONLINE REMOTE only, like OCP) ----
	if (m_pShareItem->GetShareMemoryMgrPtr()->GetSys_MachineMode() != df_HOST_ONLINE_REMOTE)
	{
		m_pLogFileAccessor->KMLOG(OPER, _T("EFA: not ONLINE REMOTE - skip APD upload / EFA^0$"));
		return true;
	}

	CCommTcpIpObjectCIM* pCIM = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM();
	if (pCIM == NULL) return true;

	if (bSave && pCIM->SendAPDFileToFTP())
	{
		// same completion wait as CDoWorkSequenceCIMStep_GDTReq (transfer thread alive -> done)
		Sleep(500);
		int nLoop = 0;
		while (pCIM->GetThreadLive() == true && nLoop < 1000) { Sleep(100); nLoop++; }
		m_pLogFileAccessor->KMLOG(OPER, (nLoop < 1000) ? _T("EFA: T0 APD FTP done") : _T("EFA: T0 APD FTP wait timeout"));
	}
	else
	{
		m_pLogFileAccessor->KMLOG(WARN, _T("EFA: T0 APD FTP not sent"));
	}

	Sleep(500);
	pCIM->Cmd_Send_EFA0();
	pCIM->WaitEFA0Ack(5000);	// logs OK / TIMEOUT; T1 continues either way (OCP)

	return (m_pShareItem->IfPauseEvt_WaitResumeEvt() == true);
}
