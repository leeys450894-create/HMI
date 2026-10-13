// ResumePrompt.h
// LYS 2026-10-06 Hybrid Phase 5 resume start prompt shared by the 4 START paths (design scratchpad p5_plan.md (2), decisions D5-8/9/10/11/16)
//   Operate CYCLE (UI thread), CIM TO_SERVER_MEAS_START (AfxBeginThread worker), AgingTest START, AgingNew (EFA) START.
//   The prompt guard (ResumePromptBusy / CResumePromptBusy) is complete in stage 5-1: stage 5-3 (entry points) and 5-4 use it.
#pragma once

class CAuroraMgr;

enum enumResumeEntry
{
	RESUME_ENTRY_OPERATE = 0,		// Operate CYCLE button
	RESUME_ENTRY_CIM,				// CIM TO_SERVER_MEAS_START (RunStart, worker thread)
	RESUME_ENTRY_AGINGTEST,			// DlgSubManual_AgingTest START (the dialog turns the EFA flag off)
	RESUME_ENTRY_AGINGNEW,			// DlgSubManual_AgingNew START (the dialog turns the EFA flag on)
};

enum enumResumePrompt
{
	RESUME_PROMPT_RESUME = 0,		// YES: resume flag set (CSystemParamShare::SetResumeMode(true, ...)), start
	RESUME_PROMPT_FRESH,			// NO / refused OK / nothing to resume: start as a new glass
	RESUME_PROMPT_CANCEL,			// CANCEL / reversible refusal cancelled / journal unreadable: do not start, bookmark kept
};

// journal (Format=2) bookmark: checks (2-2), popup (2-3), YES applies GlassID / LotID and the resume flag. Returns enumResumePrompt.
//   LYS 2026-10-08 fix (review correctness 2): CANCEL first (popup, nothing cleared) while a glass is being measured - cycle state on or an engine item not complete
int		ResumeTuplePrompt(CAuroraMgr* pMgr, int nEntry);

// format-1 bookmark of this journal recipe (D5-16): true = refused + cleared (popup shown), false = nothing resumable (caller continues)
bool	ResumeRefuseLegacyHybrid(CAuroraMgr* pMgr);

// AgingTest / AgingNew START: busy guard, journal bookmark -> ResumeTuplePrompt (inside the guard), else SetResumeMode(false). Returns enumResumePrompt.
int		ResumeDialogEntry(CAuroraMgr* pMgr, int nEntry);

// a start prompt is open on any thread (no log)
bool	ResumePromptBusy();

// RAII guard held around a start prompt: CYCLE and dialog START refuse a second start while it is alive
class CResumePromptBusy
{
public:
	CResumePromptBusy();
	~CResumePromptBusy();
private:
	CResumePromptBusy(const CResumePromptBusy&);
	CResumePromptBusy& operator=(const CResumePromptBusy&);
};
