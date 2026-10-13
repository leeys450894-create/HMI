// ResumeJournal.h
// LYS 2026-10-06 Hybrid Phase 5 resume journal (docs/HybridGlass_Porting_Plan.md 5.8, design scratchpad p5_plan.md (4) (5) (6-1))
//   {Config}\ResumeJournal.csv          one line per result row the grid took while the journal was armed (R), VISION grab
//                                       pending / timed out (P / X), OOS synthetic rows (S, RESUME_JOURNAL_OOS_SIM only)
//   {Config}\ResumeCellStatus{0,1,2}.dat nCellStatus snapshot ring, one slot per finished stick of a journal glass
//   Nothing is written while the journal is not armed: a 1-segment glass with RESUME_TUPLE_ALL=0 never arms it, manual
//   measures, the RS1 reload and the journal replay run unarmed, so they are never journaled.
//   Lock order: CResumeInfoMgr::m_cs -> journal lock. The journal lock is never held while calling dResult or CResumeInfoMgr.
#pragma once
#include <vector>
#include <functional>
#include "..\Include\API_Result.h"		// RSRITEMDATA (same include as ResultMgr.h)

class CRecipeEditMgr;

#define RESUME_CELL_X			100		// nCellStatus[cell][stick] (ShareMem.h VTM_SYSTEM)
#define RESUME_CELL_Y			50
#define RESUME_QUALITY_MAX		2		// VISION wait slots per head (CommTcpIpObjectVision.h QUALITY_MAX)
#define RESUME_SNAP_SLOTS		3		// cell status ring slots
#define RESUME_REPLAY_QUEUE_CAP	8		// dResult ring depth the replay keeps below (ring = 20)
#define RESUME_POLL_IDLE_MAX	1000	// polls x 10 ms without progress -> replay FAIL (poll count, not wall clock: a pause suspends the thread)

// S_ResumePlan::nWhy
enum enumResumeWhy
{
	RESUME_WHY_OK = 0,
	RESUME_WHY_MISSING,			// journal file not found
	RESUME_WHY_UNREADABLE,		// open failed for another reason (sharing violation, I/O) - keep the files, cancel the start
	RESUME_WHY_HEADER,			// header unreadable / differs from the bookmark, or a broken line before the last one
	RESUME_WHY_WRITE_FAILED,	// bookmark JournalOK = 0
	RESUME_WHY_SHORT,			// fewer intact lines than bookmark JournalLines
	RESUME_WHY_SNAPSHOT,		// EFA glass without the exact pred(T) cell status snapshot
	RESUME_WHY_RANGE,			// bookmark resume point invalid (GetResumePoint -1) or SegShape unreadable
};

// Replay nResult
enum enumResumeReplay
{
	RESUME_REPLAY_OK = 0,
	RESUME_REPLAY_STOPPED,		// user STOP seen while replaying - bookmark and journal kept
	RESUME_REPLAY_FAILED,		// no dResult progress for RESUME_POLL_IDLE_MAX polls, or an exception
};

// LoadCellStatus return
enum enumResumeSnap
{
	RESUME_SNAP_NONE = -1,
	RESUME_SNAP_OLDER = 0,		// non-EFA only: newest slot with tuple < T restored (T0: columns of the sticks >= T cleared to IDLE)
	RESUME_SNAP_EXACT = 1,		// slot with tuple == pred(T) restored (T0: columns of the sticks >= T cleared to IDLE)
};

// Journal context (POD - embedded in CommTcpIpObjectVision S_QualityWait). All zero = not armed / dropped.
struct S_ResumeCtx { int nGen; int nAttempt; int nPass; int nSeg; int nStick; int nGlassStick; };

// (pass, seg, stick): segment-local stick, lexicographic order (CResumeJournal::CompareTuple)
struct S_ResumeTuple { int nPass; int nSeg; int nStick; };

// Segment shape + file fingerprint of the recipe (BuildShape)
struct S_ResumeShape
{
	int		nSegCount;			// N (1 = not hybrid)
	CString	strSegShape;		// "{sticks}x{cells}r{rotate}" per segment joined by '|', e.g. "5x3r1|5x4r2"
	int		nGlassRows;			// G = sum(cells x sticks)
	CString	strFingerprint;		// "{rcp}:{aging}:{efa}:{wadmp}", FNV-1a 32 bit lower hex each, '-' = file missing
	S_ResumeShape() : nSegCount(1), nGlassRows(0) {}
};

// One consistent copy of the INI bookmark (CResumeInfoMgr::GetBookmark, stage 5-2) for Analyze / WriteCompacted / popup
struct S_ResumeBookmark
{
	CString	strGlassID, strRecipeID, strSegShape;
	int		nSegCount, nPassCount, nEfaMode, nGlassRows, nPassBoundaryDone, nJournalOK, nJournalLines, nAttempt;
	S_ResumeTuple last;			// LastPass / LastSegment / LastStick
	int		nPoint;				// GetResumePoint: 0 point, 1 all done, -1 invalid
	S_ResumeTuple T;			// resume point (all done: end marker (PassCount, 0, 0))
	S_ResumeBookmark() : nSegCount(1), nPassCount(1), nEfaMode(0), nGlassRows(0), nPassBoundaryDone(0), nJournalOK(1), nJournalLines(0), nAttempt(0), nPoint(-1)
	{ last.nPass = last.nSeg = 0; last.nStick = -1; T.nPass = T.nSeg = T.nStick = 0; }
};

// Analyze result (read-only analysis shared by the popup and the measure entry)
struct S_ResumePlan
{
	S_ResumeTuple T;			// resume point after the VISION pull-back ((0,0,0) = restart as a new glass)
	bool	bAllDone;			// T is the end marker (every stick measured, no pull-back)
	bool	bRewind;			// T moved back from the bookmark point (pull-back)
	int		nKeep;				// data lines the compacted journal keeps
	int		nLines;				// intact data lines in the journal (header and torn line excluded)
	int		nTorn;				// torn last line (0 / 1)
	int		nMissing;			// kept R lines whose raw file is missing now (rows stay empty)
	int		nPendingNoPull;		// unpaired P lines before T that are not allowed to pull back (dropped by the compaction)
	bool	bSnapExact;			// the ring has the exact pred(T) snapshot (T = (0,0,0): false, not needed)
	int		nWhy;				// enumResumeWhy
	CString	strWhyEn;			// English reason for logs ("journal missing or broken", ...)
	// LYS 2026-10-06 stage 5-1 additions for the logs of the popup / measure entry (not in the plan's sketch)
	S_ResumeTuple Tbook;		// bookmark resume point before the pull-back
	bool	bHasPred;			// Tpred valid (T != (0,0,0))
	S_ResumeTuple Tpred;		// pred(T) - the snapshot tuple the restore needs
	int		nPullHead;			// head of the P that pulled T back (-1 = none)
	CString	strPullInner;		// its InnerID
	int		nNoPullHead;		// first not-pulled unpaired P: head / InnerID / tuple / reason (valid when nPendingNoPull > 0)
	CString	strNoPullInner;
	S_ResumeTuple TnoPull;
	CString	strNoPullWhyEn;		// "older than the last 2 grabs of head %d" / "pass already flushed" / "no cell status snapshot"
	int		nIoError;			// GetLastError of a failed open (RESUME_WHY_UNREADABLE)
	S_ResumePlan() : bAllDone(false), bRewind(false), nKeep(0), nLines(0), nTorn(0), nMissing(0), nPendingNoPull(0), bSnapExact(false),
		nWhy(RESUME_WHY_OK), bHasPred(false), nPullHead(-1), nNoPullHead(-1), nIoError(0)
	{ T.nPass = T.nSeg = T.nStick = 0; Tbook = Tpred = TnoPull = T; }
};

struct S_ResumeReplayStat
{
	int		nRows, nWad, nVision, nAngle, nSim, nDropped, nTorn, nMissing;	// nDropped / nTorn: of the last WriteCompacted / SwapCompacted
	DWORD	dwMs;
	S_ResumeReplayStat() : nRows(0), nWad(0), nVision(0), nAngle(0), nSim(0), nDropped(0), nTorn(0), nMissing(0), dwMs(0) {}
};

// false = stop (user STOP). A pause blocks inside the callback (measure item IfPauseEvt_WaitResumeEvt).
typedef std::function<bool()> ResumeContinueFn;

class CResumeJournal
{
public:
	static CResumeJournal* GetInstance();

	void	SetFolder(const CString& strConfigPath);		// CResumeInfoMgr ctor (stage 5-2); reads [OPT_PARA] RESUME_TUPLE_ALL / RESUME_JOURNAL_OOS_SIM of {Config}\SystemConfig.ini (log only when non-zero)
	CString	GetFolder() const								{ return m_strFolder; }
	CString	GetJournalPath() const							{ return m_strJournal; }
	bool	IsTupleGlass(int nSegCount) const;				// nSegCount > 1 || RESUME_TUPLE_ALL
	int		GetSimMode() const								{ return m_nSimMode; }	// 0 off, 1 sim rows, 2 sim rows without VISION, 3 VISION closed alternately by S / X, 4 = 1 + the 5th journal write fails
	bool	IsArmed() const;

	// pure helpers (no state)
	static bool	BuildShape(CRecipeEditMgr* pRcp, const CString& strWadMpFile, S_ResumeShape& out);	// 1 segment: active recipe; N>1: segment getters; FNV-1a of 4 files (strWadMpFile = full path, empty = '-')
	static bool	PredTuple(const S_ResumeTuple& t, int nSegCount, const int* pnSticks, int nPassCount, S_ResumeTuple& out);	// false = (0,0,0) has none
	static int	CompareTuple(const S_ResumeTuple& a, const S_ResumeTuple& b);	// <0 / 0 / >0
	static int	ParseSegShape(const CString& strSegShape, std::vector<int>& vecSticks, std::vector<int>* pvecCells = NULL, std::vector<int>* pvecRot = NULL);	// segment count, -1 = unreadable
	static int	ResumePointFromLast(const S_ResumeTuple& last, int nSegCount, const int* pnSticks, int nPassCount, S_ResumeTuple& out);	// plan (1-2): 0 point, 1 all done (out = (PassCount,0,0)), -1 invalid
	static DWORD Fnv1a(const void* p, size_t n, DWORD dwHash = 2166136261UL);

	// lifecycle (journal lock; the writer handle is closed first)
	bool	Begin(const CString& strGlassID, const CString& strRecipeID, int nSegCount, int nPassCount, int nGlassRows);	// truncate + header + delete ring, nGen++
	void	DeleteFiles();									// journal + ring + tmp, nGen++, disarm (silent; WARN only when a delete fails); no file call while m_bFilesMayExist is false

	// context (measure thread)
	void	Arm(int nAttempt);								// opens the writer handle (FILE_APPEND_DATA, FILE_SHARE_READ | FILE_SHARE_DELETE)
	void	SetStick(int nPass, int nSeg, int nStick, int nGlassStick);
	void	Disarm();										// closes the handle under the lock, clears the D5-15 flag; no-op when not armed
	S_ResumeCtx Capture() const;							// nGen 0 when not armed
	static void SetThreadCtx(const S_ResumeCtx* pCtx);		// thread-local override (QualityWaitThread), NULL clears; an override with nGen 0 drops the row

	// append (drop when nGen == 0 or nGen != the glass generation; after Disarm with the current generation: open-append-close)
	void	OnUpload(const RSRITEMDATA* p);					// CResultMgr::Updata_Data hook; lock-free return when not armed and no override
	void	AppendPending(const S_ResumeCtx& c, int nHead, int nArea, const CString& strInnerID);	// P line (VISION grab, QualityWaitSet)
	void	AppendClose(const S_ResumeCtx& c, int nHead, int nArea, const CString& strInnerID);		// X line (VISION file timeout)
	void	AppendSim(const TCHAR* szKind, const int* pnArea, const bool* pbUse, const TCHAR* szCell0, const TCHAR* szCell1);	// S lines (2 heads), RESUME_JOURNAL_OOS_SIM > 0 and armed only
	bool	Commit();										// FlushFileBuffers; false once any write of this glass failed (sticky)
	int		GetLineCount() const;							// data lines written OK since Begin / kept by the last compaction
	bool	IsWriteOK() const;

	// resume (Analyze: read-only, popup and measure entry; never holds the lock while calling other managers)
	bool	Analyze(const S_ResumeBookmark& bm, S_ResumePlan& plan) const;	// false = plan.nWhy != OK
	bool	WriteCompacted(const S_ResumeBookmark& bm, const S_ResumePlan& plan, std::vector<CStringA>& vecKeep);	// tmp file (header Attempt+1, kept lines unchanged), cut = plan.T
	bool	SwapCompacted();								// late lines of this generation appended to the tmp, MoveFileEx tmp -> journal, nGen++ (call after the INI ApplyCompaction)
	bool	Replay(const std::vector<CStringA>& vecKeep, S_ResumeReplayStat& st, int& nResult, ResumeContinueFn fnContinue);	// (4-4); nResult = enumResumeReplay

	// cell status ring (3 slots, measure thread)
	bool	SaveCellStatus(int (*pnStatus)[RESUME_CELL_Y], int nPass, int nSeg, int nStick);
	// LYS 2026-10-08 stage 5-1 completion: T + SegShape replace the plan's glass-stick / pass arguments - nCellStatus columns run reversed inside a segment
	//   whose rotate is not 1 (MakeCellID), so one "glass stick" bound cannot say which columns are still unmeasured. pred = S_ResumePlan::Tpred, T = S_ResumePlan::T
	int		LoadCellStatus(int (*pnStatus)[RESUME_CELL_Y], const S_ResumeTuple& pred, bool bEfa, const S_ResumeTuple& T, const CString& strSegShape, S_ResumeTuple& got) const;	// enumResumeSnap
	void	NoteCellStatusReset();							// DlgSubOperate::GenGlassPath after its reset loop (UI thread)
	void	GetReplayProgress(int& nDone, int& nTotal) const	{ nDone = (int)m_nReplayDone; nTotal = (int)m_nReplayTotal; }	// LYS 2026-10-12 Hybrid Phase 6 any thread, no lock (fix: done first - Replay clears total first, so a torn read shows nothing, never 0/m)
	void	MarkCellStatusReset();							// Recipe None, journal resume branch
	int		WaitCellStatusReset(DWORD dwMs, ResumeContinueFn fnContinue);	// 1 seen, 0 timeout / not marked, -1 stopped (polls of 10 ms)

	// VISION recipe wait on resume (D5-15), bound to the glass generation the next Arm uses
	void	ArmResumeVisionWait();
	bool	ConsumeResumeVisionWait();						// true once: flag set, armed and same generation; always clears the flag

	~CResumeJournal();
private:
	CResumeJournal();
	CResumeJournal(const CResumeJournal&);
	CResumeJournal& operator=(const CResumeJournal&);

	struct S_SimX { S_ResumeCtx c; int nArea; };			// sim 3: grab whose synthetic close is an X line

	bool	WriteLineLocked(const CStringA& strLine, bool bArmedPath, CString& strWarn);	// lock held; sticky failure flag
	void	WriteFailedLocked(DWORD dwErr, CString& strWarn);
	void	CloseWriterLocked();
	int		NextSeqLocked()									{ return m_nNextSeq++; }
	CString	SnapPath(int nSlot) const;

	mutable CRITICAL_SECTION m_cs;		// journal lock
	CString	m_strFolder, m_strJournal, m_strJournalTmp, m_strSnapTmp;
	int		m_nTupleAll;				// RESUME_TUPLE_ALL
	int		m_nSimMode;					// RESUME_JOURNAL_OOS_SIM

	volatile LONG m_nGlassGen;			// generation of the journal file content (Begin / SwapCompacted / DeleteFiles ++)
	volatile LONG m_nArmedGen;			// 0 = not armed, else the generation Arm saw
	S_ResumeCtx m_ctx;					// armed context (lock)
	HANDLE	m_hWriter;					// append handle while armed
	int		m_nNextSeq;					// next line seq
	int		m_nLineCount;				// data lines written OK (GetLineCount)
	bool	m_bWriteOK;					// sticky per glass
	bool	m_bWarnedWrite;				// WARN once per glass
	int		m_nWriteCount;				// data line writes since Begin / swap (sim 4)
	CString	m_strGlassID, m_strRecipeID;	// of the journal (Begin / swap header)

	// compaction pending between WriteCompacted and SwapCompacted
	bool	m_bPend;
	S_ResumeTuple m_tPend;				// cut
	int		m_nPendSeq, m_nPendLines, m_nPendAttempt, m_nPendDropped, m_nPendTorn;
	LONGLONG m_llPendReadSize;			// journal bytes WriteCompacted read (SwapCompacted appends what came after)
	CString	m_strPendGlassID, m_strPendRecipeID;
	int		m_nLastDropped, m_nLastTorn;	// of the last swap (replay stats)
	std::vector<CStringA> m_vecSwapExtra;	// late lines SwapCompacted kept - Replay uploads them after its vecKeep

	// cell status ring
	int		m_nSnapSerial;				// last serial written for this glass
	bool	m_bWarnedSnap;

	// GenGlassPath reset counter
	volatile LONG m_nResetCount;
	volatile LONG m_nResetMark;			// -1 = not marked

	// D5-15
	bool	m_bVisWait;
	LONG	m_nVisWaitGen;

	// sim 3: VISION grabs closed alternately by S / X (the X is written right after the grab's P)
	int		m_nSimVisToggle;
	std::vector<S_SimX> m_vecSimX;

	// LYS 2026-10-08 Hybrid Phase 5 fix (N=1 review 1) "a journal / journal tmp / ring / ring tmp file may exist" (lock). SetFolder sets it from an
	//   existence check (boot, no log); Begin, Arm, armed WriteLineLocked, WriteCompacted and SaveCellStatus set it before they can create a file.
	//   DeleteFiles makes its 6 DeleteFile calls only while it is true and clears it when nothing is left, so a glass that never keeps a journal
	//   (1 segment, RESUME_TUPLE_ALL=0) makes no file call at engine start (SaveResumeInfo) or completion (ClearResumeInfo)
	bool	m_bFilesMayExist;
	// LYS 2026-10-12 Hybrid Phase 6 replay progress for the operate screen: R / S lines reached / to handle (0 / 0 = no replay). Replay only writes
	volatile LONG m_nReplayDone;
	volatile LONG m_nReplayTotal;
};
