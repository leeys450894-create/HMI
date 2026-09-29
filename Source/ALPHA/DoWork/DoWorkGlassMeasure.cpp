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

	// LYS 2026-09-28 EFA: 2-pass 루프 (OCP DoWorkStartMeasICP_EFA: 항목(pass) 루프가 스틱 루프 바깥에 있다).
	// nPassCount = EFA rcp 항목 수(보통 2: T0 에이징 전 / T1 에이징 후), 1 = 일반 측정.
	// Resume 시작 스틱은 pass 0에만 적용하고, T1 이후는 항상 스틱 0부터 다시 돈다.
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

		for (size_t i = ((nPass == 0) ? nStartStick : 0); i < (size_t)nStepIndex; i++)		// N Stick (// LYS 2026-09-28 EFA: pass별 시작 스틱)
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
					// LYS 2026-09-28 EFA: T1 이후 pass는 사전검사 스텝을 건너뛴다(OCP _EFA bInspCheck). 대상 = Pattern Search / OSTest / R-Test.
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

					// LYS 2026-09-28 EFA: 종료/Idle 스텝(SetComplete + 완료 이벤트)은 마지막 pass에서만 실행한다.
					// T0에서는 마지막 스틱도 다른 스틱처럼 Exit-1에서 끝내야 T1 도중에 RESULT/UNLOAD가 시작되지 않는다.
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
		if (bExis == true) break;	// LYS 2026-09-28 EFA: 남은 pass도 중단
	}
}

// LYS 2026-09-28 EFA: T1 이후 반복하지 않는 스텝 (OCP _EFA bInspCheck가 ReviewSearch / 사전검사를 건너뛴다).
// 2026-09-28 사용자 결정으로 Pattern Search, OST, R-Test 고정. Contact와 측정 스텝은 pass마다 반복한다.
bool CDoWorkGlassMeasure::IsEfaSkipStep(const CString& strName)
{
	return (strName == _T("Pattern Search") || strName == _T("OSTest") || strName == _T("R-Test"));
}

// LYS 2026-09-28 EFA: T0 -> T1 경계 (OCP _EFA AgingCnt==1 블록: 결과 저장 + APD FTP + EFA^0$ / EFA^0_OK$ 핸드셰이크).
// 1) T0 결과 저장: CDoWorkGlassResultStep_CompleteCheck와 같은 호출(XLS + APD.txt). dResult는 그리드를 지우지 않으므로
//    T1이 같은 행을 그대로 덮어쓰고, 마지막 Result 스텝이 T1 파일을 쓴다(OCP 동작).
//    SaveResult는 비동기 내보내기 스레드를 시작하므로 T1이 그리드에 쓰기 전에 SaveResultDone을 기다린다.
// 2) ONLINE REMOTE일 때만: APD FTP 업로드 -> 500 ms -> EFA^0$ -> EFA^0_OK$ 5초 대기, 타임아웃이어도 진행(D-2).
// false = 사용자 정지(글라스 중단). 저장/CIM 실패는 로그만 남기고 OCP처럼 계속 진행한다.
bool CDoWorkGlassMeasure::EfaPassBoundary(int nPass)
{
	CString strLog;
	strLog.Format(_T("EFA: pass boundary before pass %d (T0 done) - result flush + CIM handshake"), nPass + 1);
	m_pLogFileAccessor->KMLOG(OPER, strLog);

	if (m_pShareItem->IfPauseEvt_WaitResumeEvt() != true) return false;	// pass 사이 사용자 정지

	// ---- 1) T0 결과 저장 ----
	CString strStart;
	strStart.Format(_T("%s"), m_pShareItem->GetSeqTactTimeMgrPtr()->GetStartTime2());
	bool bSave = CResultMgr::GetInstance()->SaveResult(true, true, strStart);
	if (bSave)
	{
		// 내보내기 스레드가 SaveResultDone을 false -> true로 바꾼다. 시작까지 최대 1초, 완료까지 30초 대기
		DWORD dwS = GetTickCount();
		while (CResultMgr::GetInstance()->SaveResultDone() == true && GetTickCount() - dwS < 1000) Sleep(20);
		while (CResultMgr::GetInstance()->SaveResultDone() == false && GetTickCount() - dwS < 30000) Sleep(100);
		bSave = CResultMgr::GetInstance()->SaveResultDone();
	}
	strLog.Format(_T("EFA: T0 result flush %s"), bSave ? _T("OK") : _T("NG (UploadDataDone/export timeout) - continue"));
	m_pLogFileAccessor->KMLOG(bSave ? OPER : WARN, strLog);

	// ---- 2) CIM 핸드셰이크 (OCP처럼 ONLINE REMOTE일 때만) ----
	if (m_pShareItem->GetShareMemoryMgrPtr()->GetSys_MachineMode() != df_HOST_ONLINE_REMOTE)
	{
		m_pLogFileAccessor->KMLOG(OPER, _T("EFA: not ONLINE REMOTE - skip APD upload / EFA^0$"));
		return true;
	}

	CCommTcpIpObjectCIM* pCIM = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM();
	if (pCIM == NULL) return true;

	if (bSave && pCIM->SendAPDFileToFTP())
	{
		// CDoWorkSequenceCIMStep_GDTReq와 같은 완료 대기 (전송 스레드 살아 있음 -> 완료)
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
	pCIM->WaitEFA0Ack(5000);	// OK / TIMEOUT 로그를 남긴다. 어느 쪽이든 T1은 계속 진행 (OCP)

	return (m_pShareItem->IfPauseEvt_WaitResumeEvt() == true);
}
