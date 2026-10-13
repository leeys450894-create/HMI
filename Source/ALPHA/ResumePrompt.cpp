// ResumePrompt.cpp
// LYS 2026-10-06 Hybrid Phase 5 resume start prompt - see ResumePrompt.h.
//   CP949 file: the Korean popups go here as CP949 bytes (UTF-8 source in the scratchpad, Encode cp949 FB_CROAK). Logs are English.
// LYS 2026-10-08 Hybrid Phase 5 stage 5-3: prompt bodies (scratchpad p5_plan.md (2-1) (2-2) (2-3), decisions D5-8 a, D5-9 b, D5-10 a, D5-11 a, D5-13 a, D5-14 a, D5-16 a)
//   Checks of a journal (Format=2) bookmark, in this order:
//     0 a glass is being measured: cycle state on / engine item not complete (cancel, nothing cleared - LYS 2026-10-08 review fix)
//     1 IsResumeAvailable (silent)        1b all done + ResultSaved 2 (silent clear)   2 RecipeID (reversible)   3 segment shape (structural)
//     3b recipe keeps no journal (structural)   4 file fingerprint (reversible)   5 journal (structural; unreadable = cancel, files kept)
//     6 resume point (structural)   7 EFA cell status snapshot (structural)   7b non-EFA snapshot missing (note only)
//     8 CIM GlassID (reversible)   9 EFA mode of the START path (reversible)
//   Reversible refusal = MB_OKCANCEL (OK clears and starts fresh, CANCEL keeps everything and cancels the start).
//   Structural refusal = MB_OK, cleared, fresh start. YES never writes the shared EFA flag (D5-9 b).
#include "pch.h"
#include "ResumePrompt.h"
#include "AuroraMgr.h"
#include "SystemParamShare.h"
#include "Manager/ResumeJournal.h"
#include "Manager/ResumeInfoMgr.h"
#include "Manager/SystemConfigMgr.h"
#include "Manager/ShareMemoryMgr.h"
#include "Manager/SequenceManager.h"		// LYS 2026-10-08 review fix: IsEngineCompleteAll
#include "Manager/WadMultiMeasMgr.h"
#include "Recipe/RecipeEditMgr.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//----------------------------------------------------------------------------------------------------
// start prompt guard (review C9/K10): count of prompts open on any thread
//----------------------------------------------------------------------------------------------------
static volatile LONG s_nResumePromptBusy = 0;

bool ResumePromptBusy()
{
	return (InterlockedCompareExchange(&s_nResumePromptBusy, 0, 0) > 0);
}

CResumePromptBusy::CResumePromptBusy()
{
	InterlockedIncrement(&s_nResumePromptBusy);
}

CResumePromptBusy::~CResumePromptBusy()
{
	InterlockedDecrement(&s_nResumePromptBusy);
}

//----------------------------------------------------------------------------------------------------
// helpers
//----------------------------------------------------------------------------------------------------
static void RpLog(enumLogLevel lv, const CString& s)
{
	if (!s.IsEmpty()) CLogWrapper::GetInstance()->KMLOG(lv, s);
}

static LPCTSTR RpEntryName(int nEntry)
{
	switch (nEntry)
	{
	case RESUME_ENTRY_OPERATE:		return _T("OPERATE");
	case RESUME_ENTRY_CIM:			return _T("CIM");
	case RESUME_ENTRY_AGINGTEST:	return _T("AGINGTEST");
	case RESUME_ENTRY_AGINGNEW:		return _T("AGINGNEW");
	default:						break;
	}
	return _T("?");
}

// INI SaveTime "20261006-132210" -> "2026-10-06 13:22:10" (anything else is shown as it is)
static CString RpStopTime(const CString& s)
{
	if (s.GetLength() < 15 || s[8] != _T('-')) return s;
	CString o;
	o.Format(_T("%s-%s-%s %s:%s:%s"), (LPCTSTR)s.Mid(0, 4), (LPCTSTR)s.Mid(4, 2), (LPCTSTR)s.Mid(6, 2),
		(LPCTSTR)s.Mid(9, 2), (LPCTSTR)s.Mid(11, 2), (LPCTSTR)s.Mid(13, 2));
	return o;
}

// "[T1 ][segment 2 ]stick 3" in Korean (T only when the glass has 2+ passes, segment only when it has 2+ segments)
static CString RpTupleText(const S_ResumeTuple& t, int nPassCount, int nSegCount)
{
	CString s, o;
	if (nPassCount >= 2)	{ o.Format(_T("T%d "), t.nPass);				s += o; }
	if (nSegCount > 1)		{ o.Format(_T("세그먼트 %d "), t.nSeg + 1);	s += o; }
	o.Format(_T("스틱 %d"), t.nStick + 1);
	s += o;
	return s;
}

static void RpClearAndFresh(CResumeInfoMgr* pInfo)
{
	if (pInfo != NULL) pInfo->ClearResumeInfo();
	CSystemParamShare::GetInstance()->SetResumeMode(false);
}

// structural refusal: MB_OK, bookmark / journal / snapshots cleared, fresh start
static int RpRefuseStructural(CResumeInfoMgr* pInfo, const CString& strWhyEn, const CString& strWhyKo)
{
	CString strLog;
	strLog.Format(_T("Resume Refuse: %s - fresh start"), (LPCTSTR)strWhyEn);
	RpLog(WARN, strLog);

	CString strMsg;
	strMsg.Format(_T("이전 검사 정보로 이어 검사할 수 없습니다.\n사유 : %s\n\n처음부터 검사합니다."), (LPCTSTR)strWhyKo);
	AfxMessageBox(strMsg, MB_OK | MB_ICONWARNING);

	RpClearAndFresh(pInfo);
	return RESUME_PROMPT_FRESH;
}

// reversible refusal (review K11): OK = clear and start fresh, CANCEL = nothing cleared, the start is cancelled
static int RpRefuseReversible(CResumeInfoMgr* pInfo, const CString& strWhyEn, const CString& strWhyKo)
{
	CString strMsg;
	strMsg.Format(_T("이전 검사 정보로 이어 검사할 수 없습니다.\n사유 : %s\n\n확인 = 이전 기록을 지우고 처음부터 검사\n취소 = 검사 시작 취소 (이전 기록 유지)"), (LPCTSTR)strWhyKo);
	int nRet = AfxMessageBox(strMsg, MB_OKCANCEL | MB_ICONWARNING);

	CString strLog;
	if (nRet == IDOK)
	{
		strLog.Format(_T("Resume Refuse: %s - fresh start"), (LPCTSTR)strWhyEn);
		RpLog(WARN, strLog);
		RpClearAndFresh(pInfo);
		return RESUME_PROMPT_FRESH;
	}
	strLog.Format(_T("Resume Refuse: %s - start cancelled, bookmark kept"), (LPCTSTR)strWhyEn);
	RpLog(WARN, strLog);
	return RESUME_PROMPT_CANCEL;
}

// journal could not be read now (sharing violation, I/O - review C6): MB_OK, nothing cleared, the start is cancelled
static int RpRefuseUnreadable(const CString& strWhyEn)
{
	CString strLog;
	strLog.Format(_T("Resume Refuse: %s - start cancelled, bookmark kept"), (LPCTSTR)strWhyEn);
	RpLog(WARN, strLog);
	AfxMessageBox(_T("결과 기록 파일을 지금 읽을 수 없습니다. 잠시 후 다시 시작하세요.\n\n검사 시작을 취소합니다. (이전 기록 유지)"), MB_OK | MB_ICONWARNING);
	return RESUME_PROMPT_CANCEL;
}

//----------------------------------------------------------------------------------------------------
// journal (Format=2) bookmark prompt
//----------------------------------------------------------------------------------------------------
int ResumeTuplePrompt(CAuroraMgr* pMgr, int nEntry)
{
	CSystemParamShare* pShare = CSystemParamShare::GetInstance();
	CResumeInfoMgr* pInfo = (pMgr != NULL) ? pMgr->GetResumeInfoMgrPtr() : NULL;
	CRecipeEditMgr* pRcp = (pMgr != NULL) ? pMgr->GetRecipeEditMgrPtr() : NULL;
	if (pInfo == NULL || pRcp == NULL || pInfo->IsTupleBookmark() == false)
	{
		pShare->SetResumeMode(false);
		return RESUME_PROMPT_FRESH;
	}

	// 0 LYS 2026-10-08 Hybrid Phase 5 fix (review correctness 2) a glass is being measured - a dialog START between two engine items (the item's
	//   Idle step sets SPS_IDLE), a CIM start during a run: this bookmark may be that glass's own, so it is neither offered nor cleared (NO /
	//   a reversible OK would delete its INI, journal and ring; YES would start the engine twice). Nothing changes, the start is cancelled.
	//   Journal bookmarks only - the legacy (format-1) paths of the callers are untouched (HARD RULE)
	bool bCycleOn = pShare->IsCycleState();
	bool bEngineDone = CSequenceManager::GetInstance()->IsEngineCompleteAll();
	if (bCycleOn || bEngineDone == false)
	{
		CString strBusy;
		strBusy.Format(_T("Resume Refuse: a glass is being measured (cycle %d, engine complete %d, entry %s) - start cancelled, bookmark kept"),
			bCycleOn ? 1 : 0, bEngineDone ? 1 : 0, RpEntryName(nEntry));
		RpLog(WARN, strBusy);
		AfxMessageBox(_T("검사가 진행 중입니다. 검사가 끝난 뒤 다시 시작하세요.\n\n검사 시작을 취소합니다. (이전 기록 유지)"), MB_OK | MB_ICONWARNING);
		return RESUME_PROMPT_CANCEL;
	}

	// 1 not resumable (not running, completed, no stick finished yet, expired): silent, the files stay until the next new glass
	if (pInfo->IsResumeAvailable() == false)
	{
		pShare->SetResumeMode(false);
		return RESUME_PROMPT_FRESH;
	}

	S_ResumeBookmark bm;
	pInfo->GetBookmark(bm);

	// 1b every stick measured and the result already handled (Unload SaveResultDone, or a glass without result files) - before any other check (review C12/K3)
	if (bm.nPoint == 1 && pInfo->GetResultSaved() >= 2)
	{
		RpLog(OPER, _T("Resume: previous glass already saved - bookmark cleared"));
		RpClearAndFresh(pInfo);
		return RESUME_PROMPT_FRESH;
	}

	// 2 / 3 / 3b / 4: the recipe the START opened (cycle dialog / CIM PPID / run recipe of the aging dialogs)
	CString strRcpNow = pRcp->GetRecipeID();
	CString strWadMp;		// same path as the measure entry (stage 5-1 note), '-' when the manager is missing
	if (pMgr->GetWadMultiMeasMgrPtr() != NULL) strWadMp = pMgr->GetWadMultiMeasMgrPtr()->GetRcpFilePath(strRcpNow);
	S_ResumeShape shapeNow;
	CResumeJournal::BuildShape(pRcp, strWadMp, shapeNow);

	CString strWhyEn, strWhyKo;
	int nFail = RESUME_SHAPE_OK;
	pInfo->ValidateTupleShape(strRcpNow, shapeNow, strWhyEn, &nFail);
	if (nFail == RESUME_SHAPE_RECIPE)
	{
		strWhyKo.Format(_T("레시피가 다릅니다 (현재 %s, 이전 %s)"), (LPCTSTR)strRcpNow, (LPCTSTR)bm.strRecipeID);
		return RpRefuseReversible(pInfo, strWhyEn, strWhyKo);
	}
	if (nFail == RESUME_SHAPE_SEGMENTS)
		return RpRefuseStructural(pInfo, strWhyEn, _T("세그먼트 구성(개수/스틱 수/셀 수/회전)이 다릅니다"));
	if (CResumeJournal::GetInstance()->IsTupleGlass(shapeNow.nSegCount) == false)
		return RpRefuseStructural(pInfo, _T("journal bookmark but this recipe keeps none"), _T("이전 검사 이력의 기록 방식이 현재 설정과 다릅니다"));
	if (nFail == RESUME_SHAPE_FILES)
		return RpRefuseReversible(pInfo, strWhyEn, _T("레시피 또는 에이징/EFA/WAD 다점 설정 파일이 바뀌었습니다"));

	// 5 / 6 / 7: journal header, write state, line count, resume point, EFA snapshot (read-only - the measure entry runs the same Analyze again)
	S_ResumePlan plan;
	if (CResumeJournal::GetInstance()->Analyze(bm, plan) == false)
	{
		switch (plan.nWhy)
		{
		case RESUME_WHY_UNREADABLE:
			return RpRefuseUnreadable(plan.strWhyEn);
		case RESUME_WHY_WRITE_FAILED:
			return RpRefuseStructural(pInfo, plan.strWhyEn, _T("결과 기록(저널) 쓰기에 실패한 적이 있어 이어 검사할 수 없습니다"));
		case RESUME_WHY_RANGE:
			return RpRefuseStructural(pInfo, plan.strWhyEn, _T("이전 진행 위치가 올바르지 않습니다"));
		case RESUME_WHY_SNAPSHOT:
			return RpRefuseStructural(pInfo, plan.strWhyEn, _T("셀 상태 기록이 없어 EFA 검사를 이어갈 수 없습니다"));
		default:	// missing, header / broken line, shorter than recorded
			return RpRefuseStructural(pInfo, plan.strWhyEn, _T("결과 기록(저널) 파일이 없거나 손상되었습니다"));
		}
	}

	// 8 CIM: the host names the glass - another GlassID is another glass (D5-8 a). Manual / dialog entries restore the bookmark's ID on YES instead
	CString strGlassNow = pShare->GetGlassID();
	if (nEntry == RESUME_ENTRY_CIM && strGlassNow.Compare(bm.strGlassID) != 0)
	{
		strWhyEn.Format(_T("GlassID mismatch (host %s, saved %s)"), (LPCTSTR)strGlassNow, (LPCTSTR)bm.strGlassID);
		strWhyKo.Format(_T("Glass ID가 다릅니다 (호스트 %s, 이전 %s)"), (LPCTSTR)strGlassNow, (LPCTSTR)bm.strGlassID);
		return RpRefuseReversible(pInfo, strWhyEn, strWhyKo);
	}

	// 9 EFA mode by the START path (D5-9 b, review C8/K13): the shared flag is compared, never written.
	//   OPERATE / CIM: saved flag = current flag; AgingTest (turns the flag off): saved flag 0; AgingNew (turns the flag on): an EFA glass
	CShareMemoryMgr* pShm = pMgr->GetShareMemoryMgrPtr();
	int nFlagNow = (pShm != NULL && pShm->GetSharedMemory() != NULL && pShm->GetSys_NewAgingMode()) ? 1 : 0;
	int nFlagSaved = pInfo->GetEfaFlag();
	bool bModeOK = true;
	if (nEntry == RESUME_ENTRY_AGINGTEST)		bModeOK = (nFlagSaved == 0);
	else if (nEntry == RESUME_ENTRY_AGINGNEW)	bModeOK = (bm.nEfaMode == 1);
	else										bModeOK = (nFlagSaved == nFlagNow);
	if (bModeOK == false)
	{
		strWhyEn.Format(_T("EFA mode mismatch (saved flag %d mode %d, now flag %d, entry %s)"), nFlagSaved, bm.nEfaMode, nFlagNow, RpEntryName(nEntry));
		return RpRefuseReversible(pInfo, strWhyEn, _T("검사 모드(EFA/일반)가 이전과 다릅니다. EFA 시작으로 검사한 Glass는 EFA 화면의 START로 이어서 검사할 수 있습니다"));
	}

	// popup (2-3)
	std::vector<int> vecSticks;
	CResumeJournal::ParseSegShape(bm.strSegShape, vecSticks);		// Analyze checked: bm.nSegCount entries
	int nSegCount = (int)vecSticks.size();
	int nPassCount = (bm.nPassCount < 1) ? 1 : bm.nPassCount;
	int nGlassSticks = 0, nBaseLast = 0;
	for (int k = 0; k < nSegCount; k++)
	{
		if (k < bm.last.nSeg) nBaseLast += vecSticks[k];
		nGlassSticks += vecSticks[k];
	}
	int nSticksLastSeg = (bm.last.nSeg >= 0 && bm.last.nSeg < nSegCount) ? vecSticks[bm.last.nSeg] : 0;
	int nDone = bm.last.nPass * nGlassSticks + nBaseLast + bm.last.nStick + 1;
	int nTotal = nGlassSticks * nPassCount;
	int nSaved = pInfo->GetResultSaved();
	bool bAllDone = plan.bAllDone;		// Analyze clears it when a pending VISION pulls the resume point back (review K1)
	bool bGlassDiff = (nEntry != RESUME_ENTRY_CIM && strGlassNow.Compare(bm.strGlassID) != 0);
	bool bSnapPartial = (bm.nEfaMode != 1 && plan.bHasPred && plan.bSnapExact == false);

	CString strMsg, strLine;
	strMsg = _T("이전 검사 이력이 있습니다.\n\n");
	strLine.Format(_T("Glass ID : %s\n"), (LPCTSTR)bm.strGlassID);						strMsg += strLine;
	strLine.Format(_T("레시피 : %s\n"), (LPCTSTR)bm.strRecipeID);						strMsg += strLine;
	strLine.Format(_T("중단 시각 : %s\n"), (LPCTSTR)RpStopTime(pInfo->GetSaveTime()));	strMsg += strLine;
	if (bAllDone)
	{
		if (nSaved >= 1)	strMsg += _T("결과 저장을 시작한 뒤 저장 확인 전에 멈췄습니다.\n");
		else				strMsg += _T("모든 스틱의 측정을 마쳤지만 결과 저장 전에 멈췄습니다.\n");
	}
	else
	{
		CString strProg, strOne;
		if (nPassCount >= 2)	{ strOne.Format(_T("T%d "), bm.last.nPass);							strProg += strOne; }
		if (nSegCount > 1)		{ strOne.Format(_T("세그먼트 %d/%d, "), bm.last.nSeg + 1, nSegCount);	strProg += strOne; }
		strLine.Format(_T("진행 : %s스틱 %d/%d까지 완료 (전체 %d/%d 스틱)\n"), (LPCTSTR)strProg, bm.last.nStick + 1, nSticksLastSeg, nDone, nTotal);
		strMsg += strLine;
		if (plan.bRewind)
			strLine.Format(_T("VISION 결과가 오지 않은 스틱이 있어 %s부터 다시 측정합니다.\n"), (LPCTSTR)RpTupleText(plan.T, nPassCount, nSegCount));
		else
			strLine.Format(_T("다음 : %s부터 이어서 검사합니다. 이 스틱은 처음부터 다시 측정합니다.\n"), (LPCTSTR)RpTupleText(plan.T, nPassCount, nSegCount));
		strMsg += strLine;
	}
	if (bGlassDiff)
	{
		strLine.Format(_T("현재 Glass ID : %s  (예를 누르면 이전 Glass ID로 검사합니다)\n"), (LPCTSTR)strGlassNow);
		strMsg += strLine;
	}
	if (plan.nMissing > 0)
	{
		strLine.Format(_T("결과 파일 %d개를 찾을 수 없습니다. 해당 행은 비어 있게 됩니다.\n"), plan.nMissing);
		strMsg += strLine;
	}
	if (bSnapPartial)
		strMsg += _T("셀 상태 표시는 일부만 복원됩니다.\n");
	strMsg += _T("\n");
	if (bAllDone && nSaved >= 1)	strMsg += _T("예 = 기록된 결과로 다시 저장 (결과 파일이 한 번 더 만들어질 수 있음)\n");
	else if (bAllDone)				strMsg += _T("예 = 기록된 결과로 저장부터 진행 (다시 측정하지 않음)\n");
	else							strMsg += _T("예 = 이어서 검사\n");
	strMsg += _T("아니요 = 처음부터 검사\n취소 = 검사 시작 취소");

	// logs: what is proposed (the measure entry logs the actual resume point - a late VISION can still move it)
	CString strNext, strLog;
	if (bAllDone)			strNext.Format(_T("all sticks measured (result saved %d)"), nSaved);
	else if (plan.bRewind)	strNext.Format(_T("VISION pending - resume moved back to T%d seg %d stick %d (was T%d seg %d stick %d)"),
								plan.T.nPass, plan.T.nSeg + 1, plan.T.nStick + 1, plan.Tbook.nPass, plan.Tbook.nSeg + 1, plan.Tbook.nStick + 1);
	else					strNext.Format(_T("resume T%d seg %d stick %d"), plan.T.nPass, plan.T.nSeg + 1, plan.T.nStick + 1);
	strLog.Format(_T("Resume Prompt: entry %s, Glass=%s, Recipe=%s, last T%d seg %d/%d stick %d/%d, %s, journal lines %d keep %d torn %d, missing raw %d, snapshot %s"),
		RpEntryName(nEntry), (LPCTSTR)bm.strGlassID, (LPCTSTR)bm.strRecipeID, bm.last.nPass, bm.last.nSeg + 1, nSegCount, bm.last.nStick + 1, nSticksLastSeg,
		(LPCTSTR)strNext, plan.nLines, plan.nKeep, plan.nTorn, plan.nMissing, plan.bHasPred ? (plan.bSnapExact ? _T("exact") : _T("missing")) : _T("n/a"));
	RpLog(OPER, strLog);
	if (bSnapPartial)
	{	// 7b: not a refusal (non-EFA glass: the cell map is display only)
		strLog.Format(_T("Resume: cell status snapshot for T%d seg %d stick %d missing - cell map partly restored"), plan.Tpred.nPass, plan.Tpred.nSeg + 1, plan.Tpred.nStick + 1);
		RpLog(WARN, strLog);
	}

	int nRet = AfxMessageBox(strMsg, MB_YESNOCANCEL | MB_ICONQUESTION);
	if (nRet == IDYES)
	{
		if (nEntry != RESUME_ENTRY_CIM)
		{	// D5-8 a: the raw folder, Cell ID prefix, D-15 latch, dResult GlassID and journal header must match the first attempt
			if (bGlassDiff)
			{
				pShare->SetGlassID(bm.strGlassID);
				strLog.Format(_T("[RESUME] GlassID restored from bookmark: %s (was %s)"), (LPCTSTR)bm.strGlassID, (LPCTSTR)strGlassNow);
				RpLog(OPER, strLog);
			}
			CString strLot = pInfo->GetLotID();
			if (strLot.IsEmpty() == FALSE) pShare->SetLotID(strLot);
		}
		// the measure item resolves the point from the bookmark (Format=2); the stick here only mirrors it
		pShare->SetResumeMode(true, (bm.nPoint == 0) ? bm.T.nStick : 0);
		RpLog(OPER, _T("Resume Start: user selected resume inspection"));
		return RESUME_PROMPT_RESUME;
	}
	if (nRet == IDNO)
	{
		RpClearAndFresh(pInfo);
		RpLog(OPER, _T("Resume Cancel: user selected fresh start"));
		return RESUME_PROMPT_FRESH;
	}
	RpLog(OPER, _T("Resume Abort: user cancelled inspection start"));
	return RESUME_PROMPT_CANCEL;
}

//----------------------------------------------------------------------------------------------------
// format-1 bookmark of a journal recipe (D5-16 a). The caller checked: same RecipeID, the current recipe keeps a journal.
//   Returns true when the start is settled here: refused + cleared (popup), or nothing resumable (resume flag cleared - the same result
//   as the legacy code's else branch, and IsResumeAvailable with its expiry WARN runs only once on this path).
//----------------------------------------------------------------------------------------------------
bool ResumeRefuseLegacyHybrid(CAuroraMgr* pMgr)
{
	CResumeInfoMgr* pInfo = (pMgr != NULL) ? pMgr->GetResumeInfoMgrPtr() : NULL;
	if (pInfo == NULL || pInfo->IsTupleBookmark())
		return false;

	if (pInfo->IsResumeAvailable() == false)
	{
		CSystemParamShare::GetInstance()->SetResumeMode(false);
		return true;
	}

	RpRefuseStructural(pInfo, _T("hybrid recipe with a format-1 bookmark (no journal)"), _T("하이브리드 레시피의 이전 검사 이력에 결과 기록(저널)이 없습니다"));
	return true;
}

//----------------------------------------------------------------------------------------------------
// AgingTest / AgingNew START (D5-11 a): busy guard, journal bookmark -> the same prompt, else only the stale resume flag is cleared (D5-10 a)
//----------------------------------------------------------------------------------------------------
int ResumeDialogEntry(CAuroraMgr* pMgr, int nEntry)
{
	if (ResumePromptBusy())
	{
		RpLog(OPER, _T("Resume: another start prompt is open - dialog START ignored"));
		AfxMessageBox(_T("다른 검사 시작 확인 창이 열려 있습니다. 그 창을 먼저 닫으세요."), MB_OK | MB_ICONWARNING);
		return RESUME_PROMPT_CANCEL;
	}

	CResumeInfoMgr* pInfo = (pMgr != NULL) ? pMgr->GetResumeInfoMgrPtr() : NULL;
	bool bUseResume = (pMgr != NULL && pMgr->GetSystemConfigMgrPtr() != NULL && pMgr->GetSystemConfigMgrPtr()->IsUseResumeInspection());
	if (pInfo != NULL && bUseResume && pInfo->IsTupleBookmark())
	{
		CResumePromptBusy busy;
		return ResumeTuplePrompt(pMgr, nEntry);
	}

	// legacy (format-1) bookmark or none: the dialog START overwrites it as before (no popup)
	CSystemParamShare::GetInstance()->SetResumeMode(false);
	return RESUME_PROMPT_FRESH;
}
