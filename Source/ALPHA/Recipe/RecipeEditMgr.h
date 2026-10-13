#pragma once

#include "..\AuroraSubMgr.h"
#include "RecipeData.h"
#include "..\AuroraMgrPool.h"
#include "..\Include\API_RECIPE.h"

#define _RCP_EXT_FILE_NAME	_T(".rcp")
#define _RCP_ALL_FILE_NAME	_T("*.rcp")


enum enumMEASURE_MODE_INDEX
{
	enumMEASURE_MODE_WAD = 0,
	enumMEASURE_MODE_VISION,
	enumMEASURE_MODE_COLOR,
	enumMEASURE_MODE_ANGLE,
	enumMEASURE_MODE_TSP,
	enumMEASURE_MODE_LCR,
	enumMEASURE_MODE_PG,
	enumMEASURE_MODE_AGING,
	enumMEASURE_MODE_END,
};

/*
struct stGlassMeasAreaInfo
{
	bool			bUse;
	int				nCurrentArea;
	int				nCurrentHead;
	stMeasAreaInfo	stGlassMeasInfo;

	stGlassMeasAreaInfo()
	{
		bUse = false;
		nCurrentArea = -1;
	}
};
*/
struct SRECIPE_FILEINFO
{
	CString strName;
	CString strDate;
};

class CIniHelper;									//sklee 20210601 add
class CBinaryHelper;
class CPositionConfigMgr;
class CRecipeEditMgr : public CAuroraSubMgr
{
	CIniHelper				*m_pIniHelper;					//sklee 20210601 add
	CBinaryHelper			*m_pBinaryHelper;
	CRecipeData				*m_pRecipeData;
	CPositionConfigMgr		*m_pPositionConfigMgr;
	stAuroraMgrPool			*m_pMgrPool_link;
	CPtrnInfo				m_ptrnReview;
	C_OutRecipeFunc			* m_pRecipeFunc;
	volatile int			m_nCellStatusStickBase;	// LYS 2026-09-29 Hybrid 3b A8 set on the measure thread, read by the PG receive / UI threads
	volatile int			m_nSegCountCache;	// LYS 2026-09-30 Hybrid 3c segment log tag caches - int reads only, so the PG receive / UI threads may call GetSegLogTag
	volatile int			m_nActiveSegCache;	//   1 / 0 / 0 = not hybrid. Written only by SyncCellStatusStickBase (RecipeChange, SegmentSwitchData)
	volatile int			m_nSegRowBaseCache;	//   cell x stick sum of the segments before the active one (LYS 2026-10-01 Hybrid Phase 4 (4-0) segBase of the active segment - GetResultRowBase adds it to every live result row)
	volatile int			m_nEfaGlassLatch;	// LYS 2026-09-30 Hybrid 3c A5 EFA glass latch: -1 = not latched, 0 = not EFA, n = EFA passes (reset in RecipeChange)
	volatile int			m_nUiPass;		// LYS 2026-10-12 Hybrid Phase 6 measure-loop progress for the screens, written only by CDoWorkGlassMeasure::ThreadRun:
	volatile int			m_nUiPassCount;	//   pass (-1 = outside the measure loop) / EFA passes (0 = not EFA, any segment count) /
	volatile int			m_nUiSeg;		//   segment being measured (-1 = none yet) / 1 = its boundary (turn + re-align) is done, 0 = switching
	volatile int			m_nUiSegReady;	//   Not m_nEfaCurPass: that one is 0 for T0 / not EFA / idle alike, RecipeChange resets it, and its setter clears m_byEfaReAged
	volatile int			m_nSegLetterBaseCache;	// LYS 2026-10-01 Hybrid Phase 4 (4-0) cell ID letters of the segments before the active one (written only by SyncCellStatusStickBase)
	volatile int			m_nResultSaveLatch;	// LYS 2026-10-01 Hybrid Phase 4 (4-0) glass result save gate: -1 = not latched, 0 = bypass, 1 = save (reset in RecipeChange)
	volatile int			m_nVisionRcpWaitLatch;	// LYS 2026-10-01 Hybrid Phase 4 (4-7) VISION recipe-done wait taken this glass: 0 = not yet, 1 = taken (reset in RecipeChange, read on hybrid recipes only)
	volatile int			m_nCellIdLatch;	// LYS 2026-10-01 Hybrid Phase 4b D-15: -1 = not checked, 0 = Auto/Auto, 1 = manual config validated, 2 = invalidated mid-glass (warned once)
	volatile int			m_nEfaCurPass;	// LYS 2026-10-12 EFA pass the measure thread is running: 0 = T0 / not EFA / idle, 1 = T1 .. (read by the PG threads)
	BYTE					m_byEfaReAged[100][50];	// LYS 2026-10-12 [cell][status column] = nCellStatus[100][50]: re-aged in this EFA pass from a T0 measurement end state
	int						m_nCellIdLatchSeq;	// LYS 2026-10-01 Hybrid Phase 4b C_OutRecipeFunc::GetRcpLoadSeq() the latch-1 check saw
	CString					m_strCellIdLatchGlass;	// LYS 2026-10-01 Hybrid Phase 4b Glass ID the latch-1 check used

	CShareMemoryMgr* GetShareMemoryMgrPtr() { return m_pMgrPool_link->pShareMemoryMgr; }

public:

	void SetRecipePtr();
	CRecipeData	*GetRecipeDataPtr()	{ return m_pRecipeData;	}
	C_OutRecipeFunc* GetRecipeFuncPtr() { return m_pRecipeFunc; }
	S_RecipeMgr* GetRecipeMgrPtr();
	S_RecipeInfo* GetRecipeInfo();
	S_RecipeData* GetRecipeData();
	S_RecipeMap* GetRecipeMap();
	double GetStickProbeZOffset(int iStick);	// LYS 2026-09-09 Stick별 PROBE Z CONTACT OFFSET(mm, +=하강). 범위 밖/미로드=0, |값|>1.0mm 클램프

	void ChangeAreaInfoRecipeToMeas();
//	std::vector<stGlassMeasAreaInfo>		m_vecGlassMeasAreaHead1;
//	std::vector<stGlassMeasAreaInfo>		m_vecGlassMeasAreaHead2;

private:
	CString m_strRecipeID;
	CString m_strRecipeFolderPath;
	CString m_strRecipeFileFullPath;
	bool	m_bRecipeDeleteFlag;

private:
	CWinThread* m_pThread;
	bool		m_bThreadLive;
	HANDLE		m_hCloseEvent;
	static UINT ThreadRun(LPVOID p1);
	UINT ThreadRun();

	// PG device 에 Download 를 수행하기 떄문에
	// PG device 와 연동해서 화면에 표시하기 위해서 필요한 함수
public:
	int		RecipeChange(CString strRecipeID);
	void	RecipeUpdateUI();

public:
	int		NewRecipe(CString strRecipeID);
	int		OpenRecipe(CString strRecipeID);
	int		OpenAlignRecipe(CString strRecipeID);
	bool	SaveRecipe(CString strRecipeID);
	bool	SaveRecipeAs(CString strRecipeID);
	BOOL	DeleteRecipe(CString strRecipeID);
	bool	IsRecipeOpened() const;
	CString	GetRecipeID() const;
	CString	GetPGRecipeName();		// LYS 2026-09-29 Hybrid Phase 3 활성 세그먼트 PG 모델명 (공란 = RecipeID)
	CString	GetWADRecipeName();		// LYS 2026-09-29 Hybrid Phase 3 활성 세그먼트 WAD 모델명 (공란 = RecipeID)
	// LYS 2026-09-29 Hybrid Phase 3b segment helpers - each returns the 1-segment value when the recipe has <= 1 segment
	int		GetSegmentLoopCount();									// segments in the measure loop (1 = not hybrid)
	int		GetActiveSegment();										// A3 mask segment = active segment (0 = not hybrid)
	void	SyncCellStatusStickBase();								// A8 recompute the cached base - call right after the active segment changes
	int		GetCellStatusStickBase() const	{ return m_nCellStatusStickBase; }	// A8 nCellStatus column base of the active segment (0 = not hybrid)
	int		ToCellStatusStick(int nLocalStick) const;				// A8 segment-local status stick -> nCellStatus column (negative unchanged)
	int		GetGlassRowBlock(int nAreaActive, int nStickActive);	// A7 result-row block G (not hybrid = nAreaActive * nStickActive)
	// LYS 2026-09-30 Hybrid Phase 3c segment identity for logs (int caches only - any thread may call these)
	CString	GetSegLogTag(BOOL bRowBase = FALSE) const;				// "" when the recipe has <= 1 segment, else " seg(k/N)" (+ " rowbase(b)")
	int		GetSegRowBase() const	{ return m_nSegRowBaseCache; }	// LYS 2026-10-01 Hybrid Phase 4 (4-0) segBase of the active segment (raw cache, logs) - GetResultRowBase adds it to every live result row
	void	SetEfaGlassLatch(int nPass)	{ m_nEfaGlassLatch = nPass; }	// A5 measure thread, once per glass (engine AgingGlassLatch)
	int		GetEfaGlassLatch() const	{ return m_nEfaGlassLatch; }	// A5 -1 = not latched (1-segment recipe / after RecipeChange), 0 = not EFA, n = EFA passes
	void	SetEfaCurPass(int nPass)	{ if (nPass != m_nEfaCurPass) memset(m_byEfaReAged, 0, sizeof(m_byEfaReAged)); m_nEfaCurPass = nPass; }	// LYS 2026-10-12 measure thread: each pass start (EFA pass index, else 0) and after the pass loop (0)
	void	MarkEfaReAged(int nCell, int nCol)	{ if (nCell >= 0 && nCell < 100 && nCol >= 0 && nCol < 50) m_byEfaReAged[nCell][nCol] = 1; }	// LYS 2026-10-12 PG AGING START (EFA T1..)
	bool	IsEfaReAged(int nCell, int nCol) const	{ return m_nEfaCurPass >= 1 && nCell >= 0 && nCell < 100 && nCol >= 0 && nCol < 50 && m_byEfaReAged[nCell][nCol] != 0; }	// LYS 2026-10-12 measured even after AGING NG (OCP_EFA)
	int		GetEfaCurPass() const	{ return m_nEfaCurPass; }	// LYS 2026-10-12 >= 1: EFA T1.. - PG AGING START also marks cells left in a T0 measurement end state
	// LYS 2026-10-12 Hybrid Phase 6 screen progress (ints only - UI timers read these, never the segment vectors). No log, no lock.
	//   GetUiPass() -1 = outside the measure loop (GetEfaCurPass() above cannot tell idle from T0, so the screens use these)
	//   LYS 2026-10-12 Hybrid Phase 6 fix (review correctness 2): write order ready 0, seg, ready, pass - a reader taking pass, seg, ready in that order never pairs a new seg with the old ready
	void	SetUiProgress(int nPass, int nPassCount, int nSeg, int nSegReady)	{ m_nUiPassCount = nPassCount; m_nUiSegReady = 0; m_nUiSeg = nSeg; m_nUiSegReady = nSegReady; m_nUiPass = nPass; }
	int		GetUiPass() const		{ return m_nUiPass; }
	int		GetUiPassCount() const	{ return m_nUiPassCount; }
	int		GetUiSeg() const		{ return m_nUiSeg; }
	int		GetUiSegReady() const	{ return m_nUiSegReady; }
	int		GetSegCountCache() const	{ return m_nSegCountCache; }	// 1 = not hybrid (RecipeChange / SegmentSwitchData refresh it)
	// LYS 2026-10-01 Hybrid Phase 4 (4-0) result rows / cell ID letters / save gate (docs/HybridGlass_Porting_Plan.md 5.7). 1-segment recipe: 0 / -1 / -1
	int		GetResultRowBase() const;								// segBase added to every live result row (0 = recipe with <= 1 segment)
	int		GetCellIdLetterBase() const;							// cell ID letters of the segments before the active one (-1 = <= 1 segment -> legacy "%c" IDs)
	static CString	CellIdLetterPrefix(int nIndex);						// bijective base-26: 0 A .. 25 Z, 26 AA, 702 AAA; negative -> "@"
	static int	SegCellIdLetterExtent(C_OutRecipeFunc* pFunc, int nSeg);	// letters one segment uses (rotate 1 -> cells per stick, else sticks) - reads the segment vectors
	void	SetResultSaveLatch(int nSave)	{ m_nResultSaveLatch = nSave; }	// Recipe Change hybrid block, once per glass: 0 = bypass, 1 = save
	int		GetResultSaveLatch() const;								// -1 = not latched / <= 1 segment -> the original segment-0 gate
	// LYS 2026-10-01 Hybrid Phase 4 (4-7) hybrid device recipes (D4-18). 1-segment recipe: FALSE with pbMeasMode untouched / always true
	BOOL	OrSegmentMeasMode(bool* pbMeasMode, CString* pstrLog = NULL);	// OR every segment's inspect flags into pbMeasMode[enumMEASURE_MODE_*] - reads the segment vectors (sequence thread)
	bool	TakeVisionRcpWait();								// true = do the VISION recipe-done wait now (hybrid: the first time this glass only, measure thread)
	// LYS 2026-10-01 Hybrid Phase 4b Cell ID A/B (docs/HybridGlass_Porting_Plan.md 5.7) - any segment count (the manual mode is recipe-global)
	void	SetCellIdLatch(int nLatch, int nSeq = 0, const CString& strGlassID = CString())	{ m_nCellIdLatchSeq = nSeq; m_strCellIdLatchGlass = strGlassID; m_nCellIdLatch = nLatch; }	// Recipe Change step (D-15)
	int		GetCellIdLatch() const	{ return m_nCellIdLatch; }
	bool	GetManualCellToken(bool bPanelB, int nStick, int nCell, const CString& strGlassID, CString& strToken);	// measure thread: true = final token (manual, or B derived from A)
	CString	GetRecipeFolderPath() const;
	int		DefaultRecipeCopy(CString strRecipeID);

private:
	std::vector<SRECIPE_FILEINFO> m_vecRcpList;
	// LYS 2026-09-18 TestMode MOVE : 직전 이동에 쓴 스틱 번호 (-1 = 아직 없음).
	//                같은 스틱에서 셀만 바뀔 때 프로브 UnContact 를 건너뛰는 판정에 쓴다.
	int		m_iTestModeLastStick;
public:
	void	UpdateRecipeList();
	int		GetRecipeListCount() const;
	CString GetSelectedRecipeID(int nIndex);
	CString GetSelectedRecipeDate(int nIndex);
	void	SetRecipePosDeleteFlag(bool bFlag);
	bool	GetRecipePosDeleteFlag();
	bool	SetROIAlignPtrn(int nIndex);
	bool	SetROIMeasPtrn(int nIndex);
	int		SetDetectAlignPtrn(int nIndex);
	int		SetDetectMeasPtrn(int nIndex);

public:
	bool LinkDataBase(bool bSave);
	



//------------------------------------------------------------------
// 공통 인터페이스 함수
//------------------------------------------------------------------
public:
	virtual bool Initialize() override;
private:
	virtual void Finalize() override;
public:
	CRecipeEditMgr(stAuroraMgrPool &MgrPool);
	virtual ~CRecipeEditMgr();
};
