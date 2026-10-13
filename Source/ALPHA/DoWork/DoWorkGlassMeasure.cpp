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
#include "..\Manager\ResumeJournal.h"		// LYS 2026-10-08 Hybrid Phase 5 journal / cell status ring
#include "..\Manager\WadMultiMeasMgr.h"		// LYS 2026-10-08 Hybrid Phase 5 BuildShape (WAD multi-point rcp fingerprint)
#include "..\Manager\SystemConfigMgr.h"
#include "..\Recipe\RecipeEditMgr.h"
#include "..\Manager\ShareMemoryMgr.h"		// LYS 2026-09-28 EFA
#include "..\Manager\CommTcpIpFactoryMgr.h"	// LYS 2026-09-28 EFA
#include "..\Manager\ResultMgr.h"			// LYS 2026-09-28 EFA
#include "..\CommTcpIpObjectCIM.h"			// LYS 2026-09-28 EFA
#include "..\Manager\MaintControlMgr.h"		// LYS 2026-09-29 Hybrid Phase 3
#include "..\Manager\SequenceManager.h"		// LYS 2026-09-29 Hybrid Phase 3
#include "..\FunctionModule\FunctionModuleEQState.h"	// LYS 2026-09-29 Hybrid Phase 3
#include "DoWorkGlassSequence.h"				// LYS 2026-09-29 Hybrid Phase 3

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
	// LYS 2026-10-08 Hybrid Phase 5 a journal (Format=2) bookmark is resolved below, after the EFA / segment decision (scratchpad p5_plan.md (3-2) A)
	CResumeInfoMgr* pResumeFmt = m_pShareItem->GetResumeInfoMgrPtr();
	bool bTupleResume = (CSystemParamShare::GetInstance()->IsResumeMode() && pResumeFmt != NULL && pResumeFmt->IsTupleBookmark());
	bool bLegacyResumeFlag = false;		// format-1 resume flag consumed here (Recipe None already ran RS1) - review C13
	if (CSystemParamShare::GetInstance()->IsResumeMode() && bTupleResume == false)	// LYS 2026-10-08 Hybrid Phase 5 (was: IsResumeMode() only)
	{
		bLegacyResumeFlag = true;
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

	// LYS 2026-09-29 Hybrid Phase 3 세그먼트 루프 (docs/HybridGlass_Porting_Plan.md 5.4): pass -> seg -> stick.
	//   1세그먼트 레시피는 nSegCount = 1 이라 기존 루프와 같다. 스틱 인덱스는 세그먼트 안의 로컬 값이다 -
	//   측정 스텝은 모두 활성 세그먼트의 m_vecStick[GetMeasureStick()]을 읽으므로 스텝은 바꾸지 않는다.
	//   Resume 시작 스틱은 (pass 0, seg 0)에만 적용, 진행 저장도 세그먼트 0만 (세그먼트 튜플은 Phase 5).
	C_OutRecipeFunc* pSegFunc = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeFuncPtr();
	int nSegCount = (pSegFunc != NULL) ? pSegFunc->GetSegmentLoopCount() : 1;
	m_nCurSeg = (pSegFunc != NULL) ? pSegFunc->GetActiveSegmentIndex() : 0;

	// LYS 2026-10-08 Hybrid Phase 5 journal glass (scratchpad p5_plan.md (3-2) B). Every glass first drops a stale arm and the D5-15 flag
	//   (no lock, no I/O, no log when nothing is armed - review C7). A journal glass (IsTupleGlass: N > 1, or every glass with RESUME_TUPLE_ALL=1)
	//   then starts a new Format 2 bookmark + journal, or turns its journal bookmark into a resume tuple (checks, compaction, replay, cell status).
	//   Other glasses keep every value below at its default, so the loops run exactly as before.
	CResumeJournal::GetInstance()->Disarm();
	CResumeInfoMgr* pResumeT = m_pShareItem->GetResumeInfoMgrPtr();
	bool bTupleGlass = (pResumeT != NULL && m_pShareItem->GetSystemConfigMgrPtr()->IsUseResumeInspection()
						&& CResumeJournal::GetInstance()->IsTupleGlass(nSegCount));
	int  nResumePass = 0, nResumeSeg = 0;
	bool bSkipPassBoundary = false, bAllDone = false, bForceFirstBoundary = false, bTupleLastSaved = false, bEnterExit = false;
	// LYS 2026-10-08 Hybrid Phase 5 fix (review correctness 1) a journal resume can leave before the first stick's None step - the only place this
	//   item clears its fail / complete flags: drop what the previous run left (a Measure NG is the usual reason to resume), or a STOP during the
	//   replay or the first resumed segment change ends as "Fail Exit" with the old alarm. Same reset as the None step; StopUser is left alone
	if (bTupleResume)
	{
		SetFail(false);
		SetComplete(false);
	}
	if (bTupleGlass)
	{
		int nEnter = RESUME_ENTER_NEW;
		if (bTupleResume)
			nEnter = ResumeTupleEnter(nPassCount, bEfaMode, nSegCount, nResumePass, nResumeSeg, nStartStick, bSkipPassBoundary, bAllDone, bForceFirstBoundary);
		if (nEnter == RESUME_ENTER_NEW)		// new glass, or a VISION pull-back to the glass start (nothing to keep)
		{
			nResumePass = nResumeSeg = 0;
			nStartStick = 0;
			bSkipPassBoundary = bAllDone = bForceFirstBoundary = false;
			ResumeTupleBegin(nPassCount, bEfaMode, nSegCount, bTupleResume || bLegacyResumeFlag);	// demoted: replayed / RS1 rows dropped
		}
		else if (nEnter != RESUME_ENTER_OK)	// STOP / FAIL: bookmark, journal and snapshots kept (a compaction is idempotent), the glass stops
		{
			if (nEnter == RESUME_ENTER_FAIL && GetFail() != true && GetStopUser() != true) SetFail(true, 0);	// no alarm id (< 1000 ignored), the engine logs "Fail Exit"
			bEnterExit = true;
			bExis = true;
		}
	}
	else if (bTupleResume)
	{	// journal bookmark but this glass keeps no journal (the start popup refuses it): keep the files, stop the glass (review C2 / C13)
		m_pLogFileAccessor->KMLOG(WARN, _T("[RESUME] journal bookmark but this recipe keeps no journal - glass stopped, bookmark kept"));
		if (GetFail() != true && GetStopUser() != true) SetFail(true, 0);
		bEnterExit = true;
		bExis = true;
	}
	if (nSegCount > 1)
	{
		CString strSegLog;
		strSegLog.Format(_T("[HYBRID] measure start: %d segments, %d pass, active segment %d, resume start stick %d"), nSegCount, nPassCount, m_nCurSeg + 1, (int)nStartStick);
		m_pLogFileAccessor->KMLOG(OPER, strSegLog);

		// LYS 2026-10-01 Hybrid Phase 4 (4-1) result rows / cell ID letters per segment (rows carry segBase from Phase 4 on)
		//   prefix is not "[HYBRID] result row" on purpose - that one is the per-SetMeasUseInfo line (review B4: grep counts)
		CString strRowLog = _T("[HYBRID] rows per segment:"), strRowOne;
		int nRowFrom = 1, nLetterFrom = 0;
		for (int k = 0; k < nSegCount; k++)
		{
			int nRows = pSegFunc->GetSegmentCellCount(k) * pSegFunc->GetSegmentStickCount(k);
			int nLetters = CRecipeEditMgr::SegCellIdLetterExtent(pSegFunc, k);
			strRowOne.Format(_T(" seg%d rows %d..%d letters %s..%s"), k + 1, nRowFrom, nRowFrom + nRows - 1,
				(LPCTSTR)CRecipeEditMgr::CellIdLetterPrefix(nLetterFrom), (LPCTSTR)CRecipeEditMgr::CellIdLetterPrefix(nLetterFrom + nLetters - 1));
			strRowLog += strRowOne;
			nRowFrom += nRows;
			nLetterFrom += nLetters;
		}
		strRowOne.Format(_T(" (G %d)"), nRowFrom - 1);
		strRowLog += strRowOne;
		m_pLogFileAccessor->KMLOG(OPER, strRowLog);
	}

	// LYS 2026-09-29 Hybrid Phase 3b A4 AgingTest flag latched once per glass from segment 0 (1-segment recipes: no latch)
	// LYS 2026-09-30 Hybrid 3c A5 the same call publishes this glass's EFA decision for MeasDone (N>1: passes actually run, 0 = not EFA; 1 segment: -1)
	if (m_pMeasSeqItemEngine != NULL) m_pMeasSeqItemEngine->AgingGlassLatch(bEfaMode ? nPassCount : 0);
	int nSsStickRun = 0, nSsSkipRun = 0, nSsAnySkipRun = 0;	// LYS 2026-10-08 Sampling stick skip: sticks entered / Sampling-skipped / skipped for any reason (3c mask too) by this measure run (D-SS-5 WARN at the glass end)
	// LYS 2026-10-08 Sampling stick skip: measuring ends on a Sampling-skipped stick (the glass end, or a non-final EFA pass right before its flush).
	//   The VISION step waits for all results only at the last stick's last group; OCP waits once per glass after the stick loop
	//   (JOB_Proc CmQualityEmptyTimeOut(LAST_ITEM)). Empty buffers return at once. D-SS-6: timeout = WARN, go on. No STOP check (as in the VISION step)
	auto SsVisionWait = [this](LPCTSTR szWhere)
	{
		S_RecipeInfo* pVisInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
		int nVisTimeOut = (pVisInfo != nullptr) ? pVisInfo->m_sInspectItem.m_nTimeOut[4] : 30;	// seconds, the VISION step's value (DEFINE_INSPECITON_WAIT_TIME is ms)
		if (m_pShareItem->GetCommTcpIpFactoryMgr()->CmQualityEmptyTimeOut(LAST_ITEM, nVisTimeOut))
		{
			CString strWait;
			strWait.Format(_T("Sampling: %s ends on a skipped stick - VISION result wait timed out, the last VISION results may be missing"), szWhere);
			m_pLogFileAccessor->KMLOG(WARN, strWait);
		}
	};

	// LYS 2026-10-08 Hybrid Phase 5 journal resume entry stopped / failed (bookmark, journal and snapshots kept): the exit step releases
	//   the engine - Idle completes on a stop, not on a fail - like the stop / NG paths below
	if (bEnterExit)
	{
		SegmentRestore0();
		GoToExitStep();
		CDoWorkGlassMeasureStep* pExitItem = GetSeqItemStep();
		if (pExitItem)
		{
			pExitItem->Run();
		}
	}

	for (int nPass = nResumePass; nPass < nPassCount && bAllDone == false && bExis == false; nPass++)	// LYS 2026-10-08 Hybrid Phase 5 resume pass (was: nPass = 0; nPass < nPassCount)
	{
		m_pShareItem->GetRecipeEditMgrPtr()->SetEfaCurPass(bEfaMode ? nPass : 0);	// LYS 2026-10-12 EFA T1 AGING S: the PG AGING START reads the running pass
		m_pShareItem->GetRecipeEditMgrPtr()->SetUiProgress(nPass, bEfaMode ? nPassCount : 0, -1, 0);	// LYS 2026-10-12 Hybrid Phase 6 screen progress (ints only, no log)
		if (bEfaMode)
		{
			CString strPassLog;
			strPassLog.Format(_T("EFA: pass %d/%d start (T%d)"), nPass + 1, nPassCount, nPass);
			if (nSegCount > 1)	// LYS 2026-09-30 Hybrid 3c the pass line names the segment order (1 segment: unchanged)
			{
				CString strPassSeg;
				strPassSeg.Format(_T(" - %d segments, segment 1 first"), nSegCount);
				strPassLog += strPassSeg;
			}
			m_pLogFileAccessor->KMLOG(OPER, strPassLog);

			if (nPass >= 1 && bSkipPassBoundary && nPass == nResumePass)	// LYS 2026-10-08 Hybrid Phase 5 (D5-7) the boundary finished before the stop (flag set inside the boundary)
				m_pLogFileAccessor->KMLOG(OPER, _T("EFA: pass boundary skipped on resume (T0 already flushed, EFA^0$ already sent)"));
			else if (nPass >= 1 && EfaPassBoundary(nPass, bTupleGlass) == false)
			{
				m_pLogFileAccessor->KMLOG(WARN, _T("EFA: pass boundary failed/stopped - glass measure aborted"));
				// LYS 2026-10-13 EFA pass boundary STOP: run the Exit (Idle) step like the other abort paths - without it the complete
				//   event never fires and the engine waits forever (IsEngineCompleteAll stuck false, 1-segment EFA too)
				if (GetFail() != true && GetStopUser() != true) SetFail(true, 0);	// false = user stop today; keeps Idle from reporting a pass otherwise
				SegmentRestore0();
				GoToExitStep();
				CDoWorkGlassMeasureStep* pExitItem = GetSeqItemStep();
				if (pExitItem)
				{
					pExitItem->Run();
				}
				bExis = true;
				break;
			}
		}

		// LYS 2026-10-08 Hybrid Phase 5 the resumed pass starts at the resumed segment (normal runs: nResumePass = nResumeSeg = 0)
		for (int nSeg = (nPass == nResumePass) ? nResumeSeg : 0; nSeg < nSegCount; nSeg++)	// LYS 2026-09-29 Hybrid Phase 3 세그먼트 루프
		{
			m_pShareItem->GetRecipeEditMgrPtr()->SetUiProgress(nPass, bEfaMode ? nPassCount : 0, nSeg, 0);	// LYS 2026-10-12 Hybrid Phase 6 segment nSeg: switching
			if ((nSeg != m_nCurSeg || bForceFirstBoundary) && SegmentBoundary(m_nCurSeg, nSeg, bForceFirstBoundary) == false)	// LYS 2026-10-08 Hybrid Phase 5 (D5-6 b) forced re-align only on the first resumed segment, when needed
			{
				m_pLogFileAccessor->KMLOG(WARN, _T("[HYBRID] segment boundary failed/stopped - glass measure aborted"));
				if (GetFail() != true && GetStopUser() != true) SetFail(true, 0);	// Idle이 완료로 처리하지 않게 (알람은 원인 쪽에서 이미 발생)
				SegmentRestore0();
				GoToExitStep();
				CDoWorkGlassMeasureStep* pExitItem = GetSeqItemStep();
				if (pExitItem)
				{
					pExitItem->Run();
				}
				bExis = true;
				break;
			}

			bForceFirstBoundary = false;	// LYS 2026-10-08 Hybrid Phase 5 the first resumed segment only
			m_pShareItem->GetRecipeEditMgrPtr()->SetUiProgress(nPass, bEfaMode ? nPassCount : 0, nSeg, 1);	// LYS 2026-10-12 Hybrid Phase 6 segment nSeg: turned and re-aligned

			S_RecipeData* pSegData = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeData();
			int nStickCnt = (pSegData != nullptr) ? (int)pSegData->m_vecStick.size() : 0;	// 활성 세그먼트 스틱 수 (세그먼트 0 = nStepIndex)

			// LYS 2026-10-08 Hybrid Phase 5 resume tuple start (normal runs: (pass 0, seg 0) as before)
			for (size_t i = ((nPass == nResumePass && nSeg == nResumeSeg) ? nStartStick : 0); i < (size_t)nStickCnt; i++)		// N Stick (// LYS 2026-09-28 EFA: pass별 시작 스틱)
			{
				if (bTupleGlass) CResumeJournal::GetInstance()->SetStick(nPass, nSeg, (int)i, ((nSegCount > 1) ? pSegFunc->GetSegmentStickBase(nSeg) : 0) + (int)i);	// LYS 2026-10-08 Hybrid Phase 5 journal context of this stick
				GoToInitStep();  // Init
				long nInitStep = GetCurStep();	// LYS 2026-09-30

				// LYS 2026-09-30 Hybrid 3c one header line per stick, so the step Start/End lines can be told apart by segment (1-segment recipes: no line)
				if (nSegCount > 1)
				{
					CString strStickLog;
					strStickLog.Format(_T("[HYBRID] seg %d/%d [%s] stick %d/%d (glass stick %d/%d)"), nSeg + 1, nSegCount, pSegFunc->GetSegmentName(nSeg).GetString(),
						(int)i + 1, nStickCnt, m_pShareItem->GetRecipeEditMgrPtr()->ToCellStatusStick((int)i) + 1, pSegFunc->GetSegmentStickBase(nSegCount));
					if (bEfaMode)
					{
						CString strPass;
						strPass.Format(_T(" pass %d/%d (T%d)"), nPass + 1, nPassCount, nPass);
						strStickLog += strPass;
					}
					m_pLogFileAccessor->KMLOG(OPER, strStickLog);
				}

				// LYS 2026-08-06 AgingCycle E3: per-stick aging-list cycle begin (OCP DoWorkStartMeasOCP_A)
				bool bAgingCycle = false;
				if (m_pMeasSeqItemEngine != NULL && nStepIdxAging >= 0 && nStepIdxVision >= 0)
					bAgingCycle = bEfaMode ? m_pMeasSeqItemEngine->AgingCycleBegin(nPass, (int)i) : m_pMeasSeqItemEngine->AgingCycleBegin(-1, (int)i);	// LYS 2026-09-28 EFA, 2026-09-30 stick for the cell skip

				// LYS 2026-09-30 no USE_AGING item has a cell on this stick (OCP nGroup < 1): run only the init step (flags, process state),
				//   then the next stick - or the exit step when this is the glass's last stick. No MCC measure step for it (OCP emits none).
				bool bAgingStickSkip = (m_pMeasSeqItemEngine != NULL && m_pMeasSeqItemEngine->IsAgingStickSkip());
				// LYS 2026-10-08 Sampling stick skip (OCP nGroup < 1): no Sampling-selected WAD/VISION/ANGLE cell on this stick -> the same init-only skip
				//   as above. Standard glass: IsSamplingStickSkip; AgingTest / EFA cycle glass: AgingCycleBegin already made it a skip (IsSamplingCycleSkip).
				//   bAgingStickSkip now carries both skips; bSamplingSkip alone gates the VISION waits below (glass end, EFA pass end; the 3c path is unchanged)
				bool bSamplingSkip = (m_pMeasSeqItemEngine != NULL && (bAgingStickSkip ? m_pMeasSeqItemEngine->IsSamplingCycleSkip()
										: (bAgingCycle == false && m_pMeasSeqItemEngine->IsSamplingStickSkip((int)i))));
				nSsStickRun++;
				if (bSamplingSkip) { bAgingStickSkip = true; nSsSkipRun++; }
				if (bAgingStickSkip) nSsAnySkipRun++;	// LYS 2026-10-08 Sampling stick skip D-SS-5: skipped for either reason (3c mask, Sampling)
				if (bAgingStickSkip == false) CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_STEP_MEAS_N, true, 0);

				while (true)
				{
					CDoWorkGlassMeasureStep* pSeqItem = GetSeqItemStep();
					if (pSeqItem)
					{
						if (bTupleGlass && bTupleLastSaved == false && GetCurStep() == GetExitStepIndex()) { TupleStickDone(nPass, nSeg, (int)i); bTupleLastSaved = true; }	// LYS 2026-10-08 Hybrid Phase 5 (review C10) the glass's last stick is saved before Idle releases the engine
						if (GetCurStep() == GetExitStepIndex()) SegmentRestore0();	// LYS 2026-09-29 Hybrid Phase 3 마지막 Idle 전에 세그먼트 0 복귀
						// LYS 2026-09-28 EFA: T1 이후 pass는 사전검사 스텝을 건너뛴다(OCP _EFA bInspCheck). 대상 = Pattern Search / OSTest / R-Test.
						if (bEfaMode && nPass >= 1 && IsEfaSkipStep(pSeqItem->GetName()))
						{
							CString strSkip;
							strSkip.Format(_T("EFA pass %d: skip step %s%s"), nPass + 1, (LPCTSTR)pSeqItem->GetName(), (LPCTSTR)m_pShareItem->GetRecipeEditMgrPtr()->GetSegLogTag());	// LYS 2026-09-30 Hybrid 3c seg tag (1 segment: empty)
							m_pLogFileAccessor->KMLOG(INSP, strSkip);
							GoToNextStep();
							continue;
						}

						SetBackupStepIndex(GetCurStep());

						if (GetCurStep() > 0 && GetCurStep() < GetExitStepIndex() )
						{
							if (nSegCount > 1 && bEfaMode)	// LYS 2026-09-30 Hybrid 3c tact names carry pass and segment (1 segment: unchanged)
								strName.Format(_T("T%d_Seg%d_Stick%d_%s"), nPass, nSeg + 1, (int)i + 1, (LPCTSTR)pSeqItem->GetName());
							else if (nSegCount > 1)
								strName.Format(_T("Seg%d_Stick%d_%s"), nSeg + 1, (int)i + 1, (LPCTSTR)pSeqItem->GetName());
							else
								strName.Format(_T("Stick%d_%s"), i + 1, pSeqItem->GetName());
						}
						else
							strName.Empty();

						pSeqItem->SetMeasureStick(i);  // 현재측정 Stick 설정

						m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeST(1, strName, GetCurStep());	// Start Time

						// LYS 2026-09-28 EFA: 종료/Idle 스텝(SetComplete + 완료 이벤트)은 마지막 pass에서만 실행한다.
						// T0에서는 마지막 스틱도 다른 스틱처럼 Exit-1에서 끝내야 T1 도중에 RESULT/UNLOAD가 시작되지 않는다.
						if (i + 1 == (size_t)nStickCnt && nPass == nPassCount - 1 && nSeg == nSegCount - 1)	// Last Stick (// LYS 2026-09-29 Hybrid Phase 3 마지막 세그먼트의 마지막 스틱만 완료 이벤트)
						{
							if (pSeqItem->Run() == true) // 동작
							{
								m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(1, strName, GetCurStep());	// End Time

								// LYS 2026-09-30 skipped stick: straight to the exit step (Idle = complete event); a pending stop takes the normal path
								if (bAgingStickSkip && GetCurStep() == nInitStep && GetStopUser() != true)
								{
									// LYS 2026-10-08 Sampling stick skip: the glass ends on skipped stick(s). Sampling skip: wait for the VISION results still on the way (before the
									//   loop-head TupleStickDone, so the journal commit holds what arrives meanwhile). D-SS-5: one WARN when no stick of this run was contacted
									//   and at least one was a Sampling skip (3c mask skips count too - a glass skipped by the 3c mask alone stays as before)
									if (bSamplingSkip) SsVisionWait(_T("glass"));
									if (nSsSkipRun > 0 && nSsAnySkipRun == nSsStickRun)
									{
										CString strAllSkip;
										if (nSsSkipRun == nSsStickRun)
											strAllSkip.Format(_T("Sampling: no stick of this glass has a selected cell for the enabled items - the glass is not contacted or measured (sticks run %d)"), nSsStickRun);
										else
											strAllSkip.Format(_T("Sampling: every stick of this glass was skipped (Sampling %d, cycle mask %d) - the glass is not contacted or measured (sticks run %d)"),
												nSsSkipRun, nSsAnySkipRun - nSsSkipRun, nSsStickRun);
										m_pLogFileAccessor->KMLOG(WARN, strAllSkip);
									}
									GoToExitStep();
									continue;
								}

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

								SegmentRestore0();	// LYS 2026-09-29 Hybrid Phase 3 Idle(완료 이벤트) 전에 세그먼트 0 복귀 - Result가 활성 세그먼트를 읽는다
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

								// LYS 2026-09-30 skipped stick: leave after the init step; a pending stop takes the normal path (the next init step would clear it)
								if (bAgingStickSkip && GetCurStep() == nInitStep && GetStopUser() != true)
								{
									// LYS 2026-10-08 Sampling stick skip: a non-final EFA pass ends on a Sampling-skipped stick -> EfaPassBoundary flushes the pass next
									//   (SaveResult, APD before EFA^0$): wait for VISION as at the glass end. The 3c skip keeps its gap (ss_plan.md F1)
									if (bSamplingSkip && bEfaMode && nPass < nPassCount - 1 && i + 1 == (size_t)nStickCnt && nSeg == nSegCount - 1)
									{
										CString strPassEnd;
										strPassEnd.Format(_T("EFA pass %d/%d (T%d)"), nPass + 1, nPassCount, nPass);
										SsVisionWait(strPassEnd);
									}
									break;	// Next Stick
								}

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

								SegmentRestore0();	// LYS 2026-09-29 Hybrid Phase 3 Idle(완료 이벤트) 전에 세그먼트 0 복귀 - Result가 활성 세그먼트를 읽는다
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

							SegmentRestore0();	// LYS 2026-09-29 Hybrid Phase 3 Idle(완료 이벤트) 전에 세그먼트 0 복귀 - Result가 활성 세그먼트를 읽는다
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

				if (bAgingStickSkip == false) CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_STEP_MEAS_N, false, 0);	// LYS 2026-09-30

				// LYS 2026-08-06 AgingCycle E3: per-stick aging cycle end (progress reset)
				if (m_pMeasSeqItemEngine != NULL) m_pMeasSeqItemEngine->AgingCycleEnd();

				// LYS 2026-07-03 Resume Save: stick completed
				if (!bExis)
				{
					CResumeInfoMgr* pResume = m_pShareItem->GetResumeInfoMgrPtr();
					if (pResume && m_pShareItem->GetSystemConfigMgrPtr()->IsUseResumeInspection())
					{
						if (bTupleGlass) { if (bTupleLastSaved == false) TupleStickDone(nPass, nSeg, (int)i); }	// LYS 2026-10-08 Hybrid Phase 5 journal glass: journal Commit -> cell status snapshot -> INI tuple progress
						else if (nSeg == 0) pResume->UpdateStickProgress((int)i);	// LYS 2026-09-29 Hybrid Phase 3 세그먼트 1 이후 진행은 Phase 5에서 튜플로 저장
					}
				}

				if (bExis == true)
				{
					break;
				}
			}
			if (bExis == true) break;
		}
		if (bExis == true) break;	// LYS 2026-09-28 EFA: 남은 pass도 중단
	}
	m_pShareItem->GetRecipeEditMgrPtr()->SetEfaCurPass(0);	// LYS 2026-10-12 EFA T1 AGING S: no pass running after the loop (stop / alarm / glass end)

	// LYS 2026-10-08 Hybrid Phase 5 (D5-13) every stick of this glass was measured before the stop: the replayed grid goes to Result / Unload
	if (bAllDone && bExis == false)
	{
		m_pLogFileAccessor->KMLOG(OPER, _T("[RESUME] all sticks already measured - results replayed, measure item completes for Result / Unload"));
		S_RecipeData* pDoneData = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeData();
		GoToInitStep();
		CDoWorkGlassMeasureStep* pInitItem = GetSeqItemStep();
		if (pInitItem && pDoneData != nullptr && pDoneData->m_vecStick.empty() == false)
		{
			pInitItem->SetMeasureStick(0);	// stick 0 is always valid (a stale index could exceed segment 0)
			pInitItem->Run();				// resets Fail / Stop / Complete like the first stick of a glass
		}
		GoToExitStep();
		CDoWorkGlassMeasureStep* pExitItem = GetSeqItemStep();
		if (pExitItem)
		{
			pExitItem->Run();				// Idle: SetComplete + complete event, so the engine never waits forever (seq (0)-4)
		}
	}
	m_pShareItem->GetRecipeEditMgrPtr()->SetUiProgress(-1, 0, -1, 0);	// LYS 2026-10-12 Hybrid Phase 6 every exit path passes here
	CResumeJournal::GetInstance()->Disarm();	// LYS 2026-10-08 Hybrid Phase 5 every glass (nothing to do when not armed)

	SegmentRestore0();	// LYS 2026-09-29 Hybrid Phase 3 어떤 경로로 끝나도 세그먼트 0으로 (중복 호출 무해)
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
bool CDoWorkGlassMeasure::EfaPassBoundary(int nPass, bool bTupleGlass)	// LYS 2026-10-08 Hybrid Phase 5 bTupleGlass: PassBoundaryDone is set where the side effects end
{
	CString strLog;
	strLog.Format(_T("EFA: pass boundary before pass %d (T0 done) - result flush + CIM handshake"), nPass + 1);
	int nBoundarySeg = m_pShareItem->GetRecipeEditMgrPtr()->GetSegmentLoopCount();	// LYS 2026-09-30 Hybrid 3c (1 segment: both lines unchanged)
	if (nBoundarySeg > 1)
	{
		CString strSegAll;
		strSegAll.Format(_T(" (all %d segments)"), nBoundarySeg);
		strLog += strSegAll;
	}
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
	if (nBoundarySeg > 1)	// LYS 2026-10-01 Hybrid Phase 4 (4-1) the T0 file holds every segment's T0 rows (segBase); 3c said "rows overlap"
	{
		CString strT0Rows;
		strT0Rows.Format(_T(" - hybrid: T0 rows of all %d segments (G %d)"), nBoundarySeg, m_pShareItem->GetRecipeEditMgrPtr()->GetGlassRowBlock(0, 0));
		strLog += strT0Rows;
	}
	m_pLogFileAccessor->KMLOG(bSave ? OPER : WARN, strLog);

	// ---- 2) CIM 핸드셰이크 (OCP처럼 ONLINE REMOTE일 때만) ----
	if (m_pShareItem->GetShareMemoryMgrPtr()->GetSys_MachineMode() != df_HOST_ONLINE_REMOTE)
	{
		m_pLogFileAccessor->KMLOG(OPER, _T("EFA: not ONLINE REMOTE - skip APD upload / EFA^0$"));
		if (bTupleGlass && m_pShareItem->GetResumeInfoMgrPtr() != NULL) m_pShareItem->GetResumeInfoMgrPtr()->SetPassBoundaryDone(nPass);	// LYS 2026-10-08 Hybrid Phase 5 T0 flushed
		return true;
	}

	CCommTcpIpObjectCIM* pCIM = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM();
	if (pCIM == NULL)
	{
		if (bTupleGlass && m_pShareItem->GetResumeInfoMgrPtr() != NULL) m_pShareItem->GetResumeInfoMgrPtr()->SetPassBoundaryDone(nPass);	// LYS 2026-10-08 Hybrid Phase 5 T0 flushed
		return true;
	}

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
	if (bTupleGlass && m_pShareItem->GetResumeInfoMgrPtr() != NULL) m_pShareItem->GetResumeInfoMgrPtr()->SetPassBoundaryDone(nPass);	// LYS 2026-10-08 Hybrid Phase 5 right after the send: a stop / crash during the ACK wait never sends EFA^0$ twice
	pCIM->WaitEFA0Ack(5000);	// OK / TIMEOUT 로그를 남긴다. 어느 쪽이든 T1은 계속 진행 (OCP)

	return (m_pShareItem->IfPauseEvt_WaitResumeEvt() == true);
}

// LYS 2026-09-29 Hybrid Phase 3 활성 세그먼트 + 세그먼트 .alg 전환 (모션 없음)
//   .alg 는 파일에서 다시 읽는다 - 흡착 단계 Probe Align이 작업 슬롯 m_sAlign을 PROBE 레시피로 덮어쓴 적이 있어
//   세그먼트 벡터의 사본을 그대로 믿지 않는다. 측정 스텝(PtrnSearch)과 Glass SW Align이 이 m_sAlign을 쓴다.
bool CDoWorkGlassMeasure::SegmentSwitchData(int nSeg)
{
	CRecipeEditMgr*  pRcp  = m_pShareItem->GetRecipeEditMgrPtr();
	C_OutRecipeFunc* pFunc = pRcp->GetRecipeFuncPtr();
	CString strLog;

	if (pFunc == NULL || pFunc->SetActiveSegment(nSeg) == FALSE)
	{
		strLog.Format(_T("[HYBRID] SetActiveSegment(%d) failed"), nSeg + 1);
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		return false;
	}
	m_nCurSeg = nSeg;
	pRcp->SyncCellStatusStickBase();	// LYS 2026-09-29 Hybrid 3b A8 nCellStatus column base follows the active segment

	CString strAlign = pFunc->GetSegmentAlignName(nSeg);
	if (pRcp->OpenAlignRecipe(strAlign) != VTM_RV_SUCCESSED)
	{
		strLog.Format(_T("[HYBRID] Segment %d align recipe [%s] load failed"), nSeg + 1, strAlign.GetString());
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		return false;
	}

	strLog.Format(_T("[HYBRID] active segment -> %d/%d [%s] rot=%d stick=%d cell=%d align=%s PG=%s WAD=%s"),
		nSeg + 1, pFunc->GetSegmentLoopCount(), pFunc->GetSegmentName(nSeg).GetString(), pFunc->GetSegmentRotate(nSeg),
		pFunc->GetSegmentStickCount(nSeg), pFunc->GetSegmentCellCount(nSeg), strAlign.GetString(),
		pRcp->GetPGRecipeName().GetString(), pRcp->GetWADRecipeName().GetString());
	m_pLogFileAccessor->KMLOG(OPER, strLog);
	return true;
}

// LYS 2026-09-29 Hybrid Phase 3 결과 저장·다음 글라스 전에 세그먼트 0으로 (데이터만, 모션 없음)
//   Result 아이템이 활성 세그먼트의 검사 항목을 읽고, 엔진 시작부 Resume 저장/UI가 스틱 수를 읽는다.
void CDoWorkGlassMeasure::SegmentRestore0()
{
	if (m_nCurSeg == 0) return;

	C_OutRecipeFunc* pFunc = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeFuncPtr();
	if (pFunc == NULL || pFunc->GetSegmentCount() <= 1)
	{
		m_nCurSeg = 0;
		return;
	}

	m_pLogFileAccessor->KMLOG(OPER, _T("[HYBRID] restore segment 1 (data only, no motion)"));
	if (SegmentSwitchData(0) == false)
	{
		// LYS 2026-09-30 Hybrid 3c a failed restore used to be silent - Result then reads the segment left active
		CString strRestore;
		strRestore.Format(_T("[HYBRID] restore segment 1 failed - active segment stays %d (Result reads it, EFA^1$ uses the glass latch)"), pFunc->GetActiveSegmentIndex() + 1);
		m_pLogFileAccessor->KMLOG(WARN, strRestore);
		m_nCurSeg = 0;
	}
}

// LYS 2026-09-29 Hybrid Phase 3 세그먼트 경계 (docs/HybridGlass_Porting_Plan.md 5.4, D-1 확정 순서)
//   임의 쌍(from -> to)을 받는다 - EFA T1 시작 때 N-1 -> 0 복귀가 있다.
//   1) 사용자 정지  2) 흡착 상태(글라스가 있으면 EQ MEASURE)  3) 활성 세그먼트 + 세그먼트 .alg
//   4) PG/WAD 레시피명이 이전 세그먼트와 다를 때만 재송신 (공란 = RecipeID 라 1세그먼트 레시피는 해당 없음)
//   5) GLASS ROTATE가 다르면: Safety 위치(MoveCmd_SafetyAll - 측정 Safety 스텝과 같은 경로) -> Stage R 회전 -> Glass SW Align 재실행
//      (R 2도 넘는 회전은 Stage X >= INTERLOCKPOS 에서만 허용 - Safety 위치 1064.7 이 이를 만족)
//      같은 방향이면 5)는 생략 (ICP와 같은 규칙). 자동 재흡착은 하지 않는다(D-1).
//   false = 중단. 원인 쪽에서 SetFail(알람) 또는 사용자 정지가 세워져 있다.
bool CDoWorkGlassMeasure::SegmentBoundary(int nFrom, int nTo, bool bForce)	// LYS 2026-10-08 Hybrid Phase 5 bForce: first resumed segment (D5-6 b)
{
	CRecipeEditMgr*  pRcp  = m_pShareItem->GetRecipeEditMgrPtr();
	C_OutRecipeFunc* pFunc = pRcp->GetRecipeFuncPtr();
	if (pFunc == NULL) return false;

	DWORD dwStart	= GetTickCount();
	int	  nRotFrom	= pFunc->GetSegmentRotate(nFrom);
	int	  nRotTo	= pFunc->GetSegmentRotate(nTo);
	bool  bTurn		= (nRotFrom != nRotTo) || bForce;	// LYS 2026-10-08 Hybrid Phase 5 bForce: safety + rotate + re-align in the same orientation
	CString strPGFrom  = pRcp->GetPGRecipeName();	// 지금 활성 = nFrom
	CString strWADFrom = pRcp->GetWADRecipeName();

	CString strLog;
	strLog.Format(_T("[HYBRID] Segment boundary %d -> %d/%d [%s] rot %d -> %d : %s"), nFrom + 1, nTo + 1, pFunc->GetSegmentLoopCount(),
		pFunc->GetSegmentName(nTo).GetString(), nRotFrom, nRotTo, bTurn ? _T("safety + rotate + re-align") : _T("same orientation"));
	if (bForce) strLog += _T(" (resume: forced re-align)");	// LYS 2026-10-08 Hybrid Phase 5
	m_pLogFileAccessor->KMLOG(OPER, strLog);

	// 1) 사용자 정지
	if (m_pShareItem->IfPauseEvt_WaitResumeEvt() != true || GetStopUser() == true) return false;

	// 2) 흡착 상태 - 흡착 단계 CheckMeasStatus와 같은 기준
	CFunctionModuleEQState* pEQ = CSequenceManager::GetInstance()->GetFunctionModuleEQState();
	if (pEQ != NULL && pEQ->IsGlassSensorOn() == true && pEQ->GetEQState() != enumEQStateMeas)
	{
		m_pLogFileAccessor->KMLOG(WARN, _T("[HYBRID] Segment boundary: glass exists but EQ state is not MEASURE"));
		SetFail(true, PLC_STATE_NOT_MEASURE);
		return false;
	}

	// 3) 활성 세그먼트 + .alg
	if (SegmentSwitchData(nTo) == false)
	{
		SetFail(true, JOB_RECIPE_NOT_EXIST);
		return false;
	}

	S_RecipeInfo* pInfo = pRcp->GetRecipeInfo();
	S_RecipeData* pData = pRcp->GetRecipeData();
	if (pInfo == nullptr || pData == nullptr || pData->m_vecStick.empty())
	{
		m_pLogFileAccessor->KMLOG(WARN, _T("[HYBRID] Segment boundary: segment has no stick"));
		SetFail(true, JOB_RECIPE_NOT_EXIST);
		return false;
	}

	long nRet = VTM_RV_SUCCESSED;

	// 4) 외부 레시피 - 이름이 바뀔 때만 (ICP는 매 경계마다 다시 보냈다: I-결함9 비이식)
	//    PG: 글라스 시작과 같은 두 단계 (Recipe 아이템 DoWorkRecipeOpen = Turn Off + Model 송신, PreMeas RcpChangeDone = ACK 대기)
	bool bUsePG  = (pInfo->m_sInspectItem.m_bUsePre_0 == TRUE || pInfo->m_sInspectItem.m_bUseElect_1 == TRUE || pInfo->m_sInspectItem.m_bUseAgingTest_2 == TRUE);
	bool bUseWAD = (pInfo->m_sInspectItem.m_bUseSpec_3 == TRUE);
	CString strPGTo  = pRcp->GetPGRecipeName();
	CString strWADTo = pRcp->GetWADRecipeName();

	if (bUsePG && strPGTo.CompareNoCase(strPGFrom) != 0)
	{
		strLog.Format(_T("[HYBRID] PG recipe change [%s] -> [%s]"), strPGFrom.GetString(), strPGTo.GetString());
		m_pLogFileAccessor->KMLOG(OPER, strLog);
		nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkRecipeOpen(enumTCPIpFactorytID_PG);
		if (nRet == VTM_RV_SUCCESSED) nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PGEvent_1);
	}
	if (nRet == VTM_RV_SUCCESSED && bUseWAD && strWADTo.CompareNoCase(strWADFrom) != 0)
	{
		strLog.Format(_T("[HYBRID] WAD recipe change [%s] -> [%s]"), strWADFrom.GetString(), strWADTo.GetString());
		m_pLogFileAccessor->KMLOG(OPER, strLog);
		nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkRecipeOpen(enumTCPIpFactorytID_WAD);
	}
	if (nRet != VTM_RV_SUCCESSED)
	{
		strLog.Format(_T("[HYBRID] Segment boundary: external recipe change NG (%d)"), (int)nRet);
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		SetFail(true, (int)nRet);
		return false;
	}

	// 5) 방향 전환 (D-1): Safety -> Stage R -> 재Align
	if (bTurn)
	{
		CMaintControlMgr* pMaint = m_pShareItem->GetMaintControlMgrPtr();
		bool   bAlign = (pInfo->m_sOptionItem.m_bUseAlign == TRUE);
		double dR	  = pData->m_vecStick[0].m_fStageR + pInfo->m_sMotionOffsetPos.m_fStageRotate;	// MoveMeasureArea와 같은 식
		double dAlgR  = pRcp->GetRecipeMgrPtr()->m_sAlign.m_sGlsAlign.m_fStageRotate;
		if (bAlign && fabs(dR - dAlgR) > 0.5)
		{
			strLog.Format(_T("[HYBRID] WARN stage R %.3f differs from .alg StageRotate %.3f - align runs at the .alg angle"), dR, dAlgR);
			m_pLogFileAccessor->KMLOG(WARN, strLog);
		}

		// 5-1) Safety 위치
		m_pLogFileAccessor->KMLOG(OPER, _T("[HYBRID] 1/3 move to safety position (MoveCmd_SafetyAll)"));
		nRet = pMaint->MoveCmd_SafetyAll();
		if (nRet != df_MOTION_RV_SUCCESSED)
		{
			strLog.Format(_T("[HYBRID] safety move NG (%d)"), (int)nRet);
			m_pLogFileAccessor->KMLOG(WARN, strLog);
			SetFail(true, (int)nRet);
			return false;
		}
		if (m_pShareItem->IfPauseEvt_WaitResumeEvt() != true || GetStopUser() == true) return false;

		// 5-2) 이전 방향에서 잰 Glass Align 오프셋 해제 (Probe Align 오프셋은 유지 - ResetAlign은 쓰지 않는다)
		if (bAlign)
		{
			pMaint->SetGlassAlignXYR(0.0, 0.0, 0.0);
			pMaint->SetGlassAlignDone(false);
		}

		// 5-3) Stage R 회전
		strLog.Format(_T("[HYBRID] 2/3 rotate AXIS_LOADER_R -> %.3f (stick R %.3f + user offset %.3f)"), dR, pData->m_vecStick[0].m_fStageR, pInfo->m_sMotionOffsetPos.m_fStageRotate);
		m_pLogFileAccessor->KMLOG(OPER, strLog);
		nRet = pMaint->MoveCmdPosA(AXIS_LOADER_R, dR, true, SPEED_HIGH);
		if (nRet != df_MOTION_RV_SUCCESSED)
		{
			strLog.Format(_T("[HYBRID] stage rotate NG (%d)"), (int)nRet);
			m_pLogFileAccessor->KMLOG(WARN, strLog);
			SetFail(true, (int)nRet);
			return false;
		}
		if (m_pShareItem->IfPauseEvt_WaitResumeEvt() != true || GetStopUser() == true) return false;

		// 5-4) Glass SW Align 재실행 (흡착 단계 Probe Align 중첩 실행과 같은 방식). 레시피 ALIGN USE가 OFF면 엔진처럼 생략
		if (bAlign)
		{
			m_pLogFileAccessor->KMLOG(OPER, _T("[HYBRID] 3/3 glass SW align (segment .alg)"));
			CDoWorkGlassSequence* pGlassSeq = CSequenceManager::GetInstance()->GeDoWorkGlassSequence();
			bool bAligned = (pGlassSeq != NULL) && pGlassSeq->ItemStart(_T("SEQUENCE_ITEM_DWGLASS_SEQ_GLASS_SW_ALIGN"), true);
			m_pShareItem->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_PROCESS);	// SW Align Idle 스텝이 SPS_IDLE로 바꾼다
			if (bAligned == false)
			{
				m_pLogFileAccessor->KMLOG(WARN, _T("[HYBRID] glass SW align NG (alarm raised by the align item)"));
				if (GetStopUser() != true) SetFail(true, 0);
				return false;
			}
		}
		else
		{
			m_pLogFileAccessor->KMLOG(OPER, _T("[HYBRID] 3/3 recipe ALIGN USE = OFF - re-align skipped (glass align offset kept)"));
		}
	}

	strLog.Format(_T("[HYBRID] Segment %d/%d ready (%.1f s)"), nTo + 1, pFunc->GetSegmentLoopCount(), (GetTickCount() - dwStart) / 1000.0);
	m_pLogFileAccessor->KMLOG(OPER, strLog);

	return (m_pShareItem->IfPauseEvt_WaitResumeEvt() == true && GetStopUser() != true);
}

// LYS 2026-10-08 Hybrid Phase 5 journal glass helpers (scratchpad p5_plan.md (3-2) (3-3) (3-5) (5-1)). Measure thread only.
//   Lock order inside the managers: CResumeInfoMgr::m_cs -> journal lock; nothing here holds either one while calling the other.

// D5-6 (b): the first resumed segment k is re-aligned in the same orientation only where the uninterrupted run re-aligned after segment 0 -
//   rot(k) == rot(0) and a rotation change inside 1..k (e.g. rotations 1 -> 2 -> 1, segment 3). rot(k) != rot(0) already turns and re-aligns.
bool CDoWorkGlassMeasure::ResumeForceBoundary(int nSeg)
{
	C_OutRecipeFunc* pFunc = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeFuncPtr();
	if (pFunc == NULL || nSeg < 1 || nSeg >= pFunc->GetSegmentLoopCount()) return false;
	if (pFunc->GetSegmentRotate(nSeg) != pFunc->GetSegmentRotate(0)) return false;
	for (int j = 1; j <= nSeg; j++)
	{
		if (pFunc->GetSegmentRotate(j) != pFunc->GetSegmentRotate(j - 1)) return true;
	}
	return false;
}

// stick done (plan (1-3)): journal Commit -> cell status snapshot -> INI tuple progress. With this order pred(T) is always in the ring (5-1).
void CDoWorkGlassMeasure::TupleStickDone(int nPass, int nSeg, int nStick)
{
	CResumeInfoMgr* pResumeT = m_pShareItem->GetResumeInfoMgrPtr();
	if (pResumeT == NULL) return;
	CResumeJournal* pJ = CResumeJournal::GetInstance();
	bool bJournalOK = pJ->Commit();
	VTM_SYSTEM* pShm = (m_pShareItem->GetShareMemoryMgrPtr() != NULL) ? m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory() : NULL;
	if (pShm != NULL) pJ->SaveCellStatus(pShm->nCellStatus, nPass, nSeg, nStick);
	pResumeT->UpdateTupleProgress(nPass, nSeg, nStick, bJournalOK, pJ->GetLineCount());
}

// new journal glass (plan (3-2)): Format 2 bookmark with the engine's segment-0 values (so a 1-segment journal glass keeps the legacy
//   "Resume Update: Stick=%d/%d" line), journal header (truncate, snapshot ring deleted), arm. bDemoted: a journal resume that became
//   a new glass, or a format-1 resume flag whose RS1 rows are in the grid (review C13) - the grid is cleared again.
void CDoWorkGlassMeasure::ResumeTupleBegin(int nPassCount, bool bEfaMode, int nSegCount, bool bDemoted)
{
	CResumeInfoMgr* pResumeT = m_pShareItem->GetResumeInfoMgrPtr();
	if (pResumeT == NULL) return;
	CRecipeEditMgr*		pRcp = m_pShareItem->GetRecipeEditMgrPtr();
	CSystemParamShare*	pSP	 = CSystemParamShare::GetInstance();
	CResumeJournal*		pJ	 = CResumeJournal::GetInstance();

	CString strRecipeID = pRcp->GetRecipeID();
	CString strWadMp;		// the same WAD multi-point rcp path as the start popup - another path would refuse every resume as "files changed"
	if (m_pShareItem->GetWadMultiMeasMgrPtr() != NULL) strWadMp = m_pShareItem->GetWadMultiMeasMgrPtr()->GetRcpFilePath(strRecipeID);
	S_ResumeShape shape;
	CResumeJournal::BuildShape(pRcp, strWadMp, shape);

	S_RecipeData* pRD = pRcp->GetRecipeData();		// segment 0 is active here: the values the engine passes to SaveResumeInfo
	S_RecipeInfo* pRI = pRcp->GetRecipeInfo();
	int  nTotalStick0 = (pRD != nullptr) ? (int)pRD->m_vecStick.size() : 0;
	int  nGlsRotate0  = (pRI != nullptr) ? pRI->m_sOptionItem.m_nGlsRotate : -1;
	bool bEfaFlag	  = (m_pShareItem->GetShareMemoryMgrPtr() != NULL && m_pShareItem->GetShareMemoryMgrPtr()->GetSys_NewAgingMode());

	pResumeT->BeginTupleGlass(pSP->GetGlassID(), strRecipeID, pSP->GetLotID(), pSP->GetInnerID(), pSP->GetResultFolderPath(),
		nTotalStick0, nGlsRotate0, shape, nPassCount, bEfaMode ? 1 : 0, bEfaFlag ? 1 : 0, nSegCount > 1);
	pJ->Begin(pSP->GetGlassID(), strRecipeID, shape.nSegCount, nPassCount, shape.nGlassRows);
	if (bDemoted) CResultMgr::GetInstance()->ClearData();
	pJ->Arm(0);

	if (nSegCount > 1)
	{
		CString strLog;
		strLog.Format(_T("[RESUME] new glass bookmark: GlassID %s, %d segments, %d pass, G %d"), pSP->GetGlassID().GetString(), shape.nSegCount, nPassCount, shape.nGlassRows);
		m_pLogFileAccessor->KMLOG(OPER, strLog);
	}
}

// journal bookmark at measure entry (plan (3-2) steps 1-10). It never turns the bookmark into a new glass and never cuts the journal on a
//   failure (review C2 / K4): FAIL / STOP leave the files as they are (or compacted - idempotent) and the next START asks again.
//   NEW only for a VISION pull-back to the glass start (nothing to keep). Outputs: resume tuple (pass, seg, segment-local stick).
int CDoWorkGlassMeasure::ResumeTupleEnter(int nPassCount, bool bEfaMode, int nSegCount, int& rPass, int& rSeg, size_t& rStick,
	bool& bSkipPassBoundary, bool& bAllDone, bool& bForceFirstBoundary)
{
	rPass = rSeg = 0;
	rStick = 0;
	bSkipPassBoundary = bAllDone = bForceFirstBoundary = false;

	CResumeInfoMgr*	pResumeT = m_pShareItem->GetResumeInfoMgrPtr();
	CRecipeEditMgr*	pRcp	 = m_pShareItem->GetRecipeEditMgrPtr();
	CResumeJournal*	pJ		 = CResumeJournal::GetInstance();
	ResumeContinueFn fnContinue = [this]() -> bool { return m_pShareItem->IfPauseEvt_WaitResumeEvt(); };	// STOP -> false; a pause blocks inside
	CString strLog, strWhy;
	if (pResumeT == NULL) return RESUME_ENTER_FAIL;

	// 1) the bookmark against this glass - the popup checked it, a reload / CIM PPID can change things in between (pass count, EFA mode, shape)
	S_ResumeBookmark bm;
	pResumeT->GetBookmark(bm);
	CString strRecipeID = pRcp->GetRecipeID();
	CString strWadMp;
	if (m_pShareItem->GetWadMultiMeasMgrPtr() != NULL) strWadMp = m_pShareItem->GetWadMultiMeasMgrPtr()->GetRcpFilePath(strRecipeID);
	S_ResumeShape shapeNow;
	CResumeJournal::BuildShape(pRcp, strWadMp, shapeNow);
	std::vector<int> vecSticks;
	if (bm.strRecipeID.CompareNoCase(strRecipeID) != 0)
		strWhy.Format(_T("RecipeID mismatch (saved %s, now %s)"), bm.strRecipeID.GetString(), strRecipeID.GetString());
	else if (bm.strGlassID != CSystemParamShare::GetInstance()->GetGlassID())		// the journal header carries the bookmark GlassID (Analyze)
		strWhy.Format(_T("GlassID mismatch (saved %s, now %s)"), bm.strGlassID.GetString(), CSystemParamShare::GetInstance()->GetGlassID().GetString());
	else if (bm.nPassCount != nPassCount || bm.nEfaMode != (bEfaMode ? 1 : 0))
		strWhy.Format(_T("EFA pass mismatch (saved pass %d mode %d, now pass %d mode %d)"), bm.nPassCount, bm.nEfaMode, nPassCount, bEfaMode ? 1 : 0);
	else if (bm.nSegCount != nSegCount || shapeNow.nSegCount != nSegCount || bm.strSegShape != shapeNow.strSegShape || bm.nGlassRows != shapeNow.nGlassRows
			 || CResumeJournal::ParseSegShape(bm.strSegShape, vecSticks) != nSegCount)
		strWhy.Format(_T("segment shape mismatch (saved %d %s G %d, now %d %s G %d)"), bm.nSegCount, bm.strSegShape.GetString(), bm.nGlassRows,
			nSegCount, shapeNow.strSegShape.GetString(), shapeNow.nGlassRows);
	else if (bm.nPoint < 0)
		strWhy = _T("bookmark out of range");
	if (strWhy.IsEmpty() == false)
	{
		strLog.Format(_T("[RESUME] measure entry refused: %s - glass stopped, bookmark and journal kept"), strWhy.GetString());
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		return RESUME_ENTER_FAIL;
	}

	// 2) + 3) resume point and journal analysis (read-only, the same function as the popup): header, JournalOK, line count, torn line,
	//    InnerID pairing, VISION pull-back (4-5), snapshot ring (EFA glass: the exact pred(T) slot is required)
	S_ResumePlan plan;
	if (pJ->Analyze(bm, plan) == false)
	{
		strLog.Format(_T("[RESUME] measure entry refused: %s - glass stopped, bookmark and journal kept"), plan.strWhyEn.GetString());
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		return RESUME_ENTER_FAIL;
	}
	if (plan.nPendingNoPull > 0)
	{
		strLog.Format(_T("[RESUME] VISION never arrived for %d grab(s) (first %s T%d seg %d stick %d) - not pulled back (%s), rows stay empty"),
			plan.nPendingNoPull, plan.strNoPullInner.GetString(), plan.TnoPull.nPass, plan.TnoPull.nSeg + 1, plan.TnoPull.nStick + 1, plan.strNoPullWhyEn.GetString());
		m_pLogFileAccessor->KMLOG(WARN, strLog);
	}

	// 4) pulled back to the glass start: nothing to keep, the glass restarts as a new glass (the only demotion left - review C1)
	if (plan.T.nPass == 0 && plan.T.nSeg == 0 && plan.T.nStick == 0)
	{
		m_pLogFileAccessor->KMLOG(OPER, _T("[RESUME] VISION pending on T0 seg 1 stick 1 - glass restarts as a new glass"));
		return RESUME_ENTER_NEW;
	}
	if (plan.bRewind)
	{
		CString strWas;
		if (bm.nPoint == 1) strWas = _T("all done");
		else strWas.Format(_T("T%d seg %d stick %d"), plan.Tbook.nPass, plan.Tbook.nSeg + 1, plan.Tbook.nStick + 1);
		strLog.Format(_T("[RESUME] VISION pending on T%d seg %d stick %d - resume moved back to that stick (was %s)"), plan.T.nPass, plan.T.nSeg + 1, plan.T.nStick + 1, strWas.GetString());
		m_pLogFileAccessor->KMLOG(OPER, strLog);
	}

	// 5) EFA glass without the exact pred(T) snapshot: Analyze already refused it (RESUME_WHY_SNAPSHOT); non-EFA only warns at step 8

	// 6) compaction in three steps (lock order (1-3)): tmp journal -> INI first (Attempt + 1, JournalLines; pull-back: Last* = pred(T),
	//    ResultSaved 0) -> journal swap. A stop in any gap gives the same result on the next resume (review C1 / K1)
	std::vector<CStringA> vecKeep;
	if (pJ->WriteCompacted(bm, plan, vecKeep) == false)
	{
		m_pLogFileAccessor->KMLOG(WARN, _T("[RESUME] measure entry refused: journal compaction failed - glass stopped, bookmark and journal kept"));
		return RESUME_ENTER_FAIL;
	}
	if (pResumeT->ApplyCompaction(plan.bRewind, plan.Tpred, (int)vecKeep.size()) == false)
	{	// the INI and its members are unchanged - the journal must not be swapped
		m_pLogFileAccessor->KMLOG(WARN, _T("[RESUME] measure entry refused: bookmark write failed at the compaction - glass stopped, bookmark and journal kept"));
		return RESUME_ENTER_FAIL;
	}
	if (pJ->SwapCompacted() == false)
	{	// INI already compacted, old journal kept: the next resume compacts the old journal again to the same result
		m_pLogFileAccessor->KMLOG(WARN, _T("[RESUME] measure entry refused: journal swap failed - glass stopped, bookmark and journal kept"));
		return RESUME_ENTER_FAIL;
	}

	// 7) journal replay (4-4): dResult fast mode, queue-depth pacing, STOP / pause polled per line and per wait, poll-count timeouts
	S_ResumeReplayStat st;
	int nReplay = RESUME_REPLAY_OK;
	pJ->Replay(vecKeep, st, nReplay, fnContinue);
	if (nReplay == RESUME_REPLAY_STOPPED)
	{
		strLog.Format(_T("[RESUME] journal replay stopped by user at %d/%d - bookmark and journal kept"), st.nRows, (int)vecKeep.size());
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		return RESUME_ENTER_STOP;
	}
	if (nReplay != RESUME_REPLAY_OK)
	{
		strLog.Format(_T("[RESUME] journal replay failed (%d of %d lines uploaded, see ResumeJournal) - glass stopped, bookmark and journal kept"), st.nRows, (int)vecKeep.size());
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		return RESUME_ENTER_FAIL;
	}
	strLog.Format(_T("[RESUME] journal replay: %d rows (WAD %d, VISION %d, ANGLE %d, sim %d), dropped %d, torn %d, missing raw %d, %.1f s"),
		st.nRows, st.nWad, st.nVision, st.nAngle, st.nSim, st.nDropped, st.nTorn, st.nMissing, st.dwMs / 1000.0);
	m_pLogFileAccessor->KMLOG(OPER, strLog);

	// 8) cell status (5-1): after the UI reset Recipe Change posted (GenGlassPath counter, marked by Recipe None), the exact pred(T) slot
	int nWait = pJ->WaitCellStatusReset(5000, fnContinue);
	if (nWait < 0)
	{
		m_pLogFileAccessor->KMLOG(WARN, _T("[RESUME] stopped while waiting for the cell status reset - bookmark and journal kept"));
		return RESUME_ENTER_STOP;
	}
	if (nWait == 0) m_pLogFileAccessor->KMLOG(WARN, _T("[RESUME] cell status reset not seen within 5 s - restored anyway"));
	if (plan.bHasPred)
	{
		VTM_SYSTEM* pShm = (m_pShareItem->GetShareMemoryMgrPtr() != NULL) ? m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory() : NULL;
		S_ResumeTuple got = { 0, 0, 0 };
		int nSnap = (pShm != NULL) ? pJ->LoadCellStatus(pShm->nCellStatus, plan.Tpred, bEfaMode, plan.T, bm.strSegShape, got) : RESUME_SNAP_NONE;
		if (nSnap == RESUME_SNAP_EXACT)
		{
			strLog.Format(_T("[RESUME] cell status restored (snapshot after T%d seg %d stick %d)"), got.nPass, got.nSeg + 1, got.nStick + 1);
			m_pLogFileAccessor->KMLOG(OPER, strLog);
		}
		else if (bEfaMode)
		{	// the T1 sticks read the T0 NG cells from nCellStatus only (review K2)
			strLog.Format(_T("[RESUME] measure entry refused: cell status snapshot for T%d seg %d stick %d missing (EFA glass) - glass stopped, bookmark and journal kept"),
				plan.Tpred.nPass, plan.Tpred.nSeg + 1, plan.Tpred.nStick + 1);
			m_pLogFileAccessor->KMLOG(WARN, strLog);
			return RESUME_ENTER_FAIL;
		}
		else
		{
			CString strGot = _T("none restored");
			if (nSnap == RESUME_SNAP_OLDER) strGot.Format(_T("snapshot after T%d seg %d stick %d"), got.nPass, got.nSeg + 1, got.nStick + 1);
			strLog.Format(_T("[RESUME] cell status snapshot for T%d seg %d stick %d missing - cell map partly restored (%s)"),
				plan.Tpred.nPass, plan.Tpred.nSeg + 1, plan.Tpred.nStick + 1, strGot.GetString());
			m_pLogFileAccessor->KMLOG(WARN, strLog);
		}
	}

	// a STOP pressed since the replay would be cleared by the first stick's init step (SetStopUser(false)) - take it here (pause: waits)
	if (fnContinue() == false)
	{
		m_pLogFileAccessor->KMLOG(WARN, _T("[RESUME] stopped at the measure entry - bookmark and journal kept"));
		return RESUME_ENTER_STOP;
	}

	// 9) resume tuple, EFA boundary skip, forced re-align (D5-6 b), D5-15 VISION recipe wait (bound to this arm), arm
	rPass		= plan.T.nPass;
	rSeg		= plan.T.nSeg;
	rStick		= (size_t)plan.T.nStick;
	bAllDone	= plan.bAllDone;
	bSkipPassBoundary	= (rPass >= 1 && bm.nPassBoundaryDone >= rPass);
	bForceFirstBoundary	= (bAllDone == false && ResumeForceBoundary(rSeg));
	if (bAllDone == false) pJ->ArmResumeVisionWait();
	pJ->Arm(pResumeT->GetAttempt());

	// 10)
	int nGlassSticks = 0, nBase = 0;
	for (int k = 0; k < (int)vecSticks.size(); k++)
	{
		if (k < rSeg) nBase += vecSticks[k];
		nGlassSticks += vecSticks[k];
	}
	if (bAllDone)
		strLog.Format(_T("[RESUME] resume point: all %d glass sticks measured (end marker T%d), pass boundary done %d, attempt %d"),
			nGlassSticks, rPass, bm.nPassBoundaryDone, pResumeT->GetAttempt());
	else
		strLog.Format(_T("[RESUME] resume point T%d seg %d/%d stick %d/%d (glass stick %d/%d), pass boundary done %d, attempt %d"),
			rPass, rSeg + 1, nSegCount, (int)rStick + 1, (rSeg < (int)vecSticks.size()) ? vecSticks[rSeg] : 0, nBase + (int)rStick + 1, nGlassSticks,
			bm.nPassBoundaryDone, pResumeT->GetAttempt());
	m_pLogFileAccessor->KMLOG(OPER, strLog);
	return RESUME_ENTER_OK;
}
