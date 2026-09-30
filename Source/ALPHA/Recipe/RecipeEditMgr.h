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
	volatile int			m_nSegRowBaseCache;	//   cell x stick sum of the segments before the active one (Phase 4 segBase preview, printed only)
	volatile int			m_nEfaGlassLatch;	// LYS 2026-09-30 Hybrid 3c A5 EFA glass latch: -1 = not latched, 0 = not EFA, n = EFA passes (reset in RecipeChange)

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
	int		GetSegRowBase() const	{ return m_nSegRowBaseCache; }	// Phase 4 segBase preview - printed only, never added to a row in 3c
	void	SetEfaGlassLatch(int nPass)	{ m_nEfaGlassLatch = nPass; }	// A5 measure thread, once per glass (engine AgingGlassLatch)
	int		GetEfaGlassLatch() const	{ return m_nEfaGlassLatch; }	// A5 -1 = not latched (1-segment recipe / after RecipeChange), 0 = not EFA, n = EFA passes
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
