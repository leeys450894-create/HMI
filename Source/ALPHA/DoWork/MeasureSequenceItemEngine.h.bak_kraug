#pragma once

#include "..\AuroraMgrPool.h"
#include "AgingCycleInfo.h"	// LYS 2026-08-06 AgingCycle E3

class CIniHelper;
class CBinaryHelper;
class CLogFileAccessor;
class CMeasureSequenceItemFactory;
class CMeasureSequenceItemImpl;
class CMeasureSequenceItemEngine
{
	stAuroraMgrPool						m_MgrPool_link;
	CLogFileAccessor					*m_pLogFileAccessor;
	CMeasureSequenceItemFactory			*m_pItemFactory;
	std::vector<CMeasureSequenceItemImpl *>	m_vecItems;
	bool								m_bEngineBreak;
	CIniHelper							*m_pIniHelper;

	void SetHead(int nHead);
	int GetHead();
	int m_nHead;

public:
	bool LinkDataBase(CBinaryHelper *pBinaryHelper, bool bSave);
	CRecipeEditMgr	*GetRecipeEditMgrPtr()		{ return m_MgrPool_link.pRecipeEditMgr;			}
	CSeqTactTimeMgr	*GetSeqTactTimeMgrPtr()		{ return m_MgrPool_link.pSeqTactTimeMgr;		}
	CShareMemoryMgr* GetShareMemoryMgrPtr()		{ return m_MgrPool_link.pShareMemoryMgr;		}


	//Area
	void SetMeasureArea(int nMeasureArea);
	int GetMeasureArea();
	int m_nMeasureArea;
	TCHAR** m_pStInnerID;

	// LYS 2026-08-12 InnerID uniqueness: gf_GetCurTimeString(3) is HHMMSS (1 second
	// resolution), so two measurements inside the same second produced the SAME id ->
	// same device raw file name -> the later result overwrote the earlier one.
	// Observed 16/315 (5%) on a 3-repeat aging run. Kept at the original length so the
	// device packet layout is untouched: only the trailing 2 digits are remapped, and
	// the issued list guarantees uniqueness for the glass.
	std::vector<CString>	m_vecIssuedInnerID;
public:
	CString MakeUniqueInnerID(const CString& strID);
	void PushIssuedInnerID(const CString& strID);

//=========================================================================
// SeqItem ฐüทร
//=========================================================================
public:
	bool				ItemInitialize();
	void				ItemFinalize();
	bool				ItemStart(CString strType, bool bCompleteCheck);
	void				ItemStopAndSkip(CString strType);
	void				ItemStopAndExit(CString strType);
	void				ItemPause(int index);
	void				ItemResume(int index);
	bool				Is_CompleteAll();
	bool				Is_Complete(int index);

	std::vector<CString>		GetItemType();
	std::vector<CString>		GetItemDesc();
	int					GetItemCount() const;
	void				SetItemVisible(int index);
	void				AddItem(CString strType, CString strName);
	void				DelItem(int index);
	CMeasureSequenceItemImpl	*GetItem(int index);
	CMeasureSequenceItemImpl	*FindItem(CString strSeq);
	void				UpdateItemState();


public:
	void EngineStart();
	void EngineResume();
	void EnginePause();
	void EngineStop();


//=========================================================================
// SeqItem Run Thread
//=========================================================================
protected:
	CWinThread	*m_pThread;
	bool		m_bThreadLive;
	HANDLE		m_hStartEvent;
	HANDLE		m_hExitsEvent;
	HANDLE		m_hWaitsEvent;
	HANDLE		m_hCloseEvent;
	HANDLE		m_hBreakEvent_link;	// link item event
	HANDLE		m_hMultiEvent[3];
public:
	static UINT ThreadRun(LPVOID p1);
	void ThreadRun();
	void ThreadRunExit();
	void ThreadRu_nSequence(int cycle_index);
	bool IsThreadStopState();

//=========================================================================
// LYS 2026-08-06 AgingCycle E3: aging-list cycle context (OCP DoWorkStartMeasOCP_A)
// driven by CDoWorkGlassMeasure::ThreadRun on the measure thread (single-thread use)
//=========================================================================
public:
	bool				AgingCycleBegin();		// gate(bIsOCP + recipe m_bUseAging_Test) + aging rcp load
	void				AgingCycleEnd();
	bool				AgingCycleNextItem();	// advance to next USE_AGING item. FALSE = exhausted
	bool				IsAgingCycleActive() const	{ return m_bAgingCycleActive; }
	int					GetAgingCurItem() const		{ return m_nAgingCurItem; }

	// LYS 2026-08-12 AgingCycle R1: dense per-kind repeat index for the result row block.
	// Capture at step entry, use for nAreaNum, advance once after the kind actually measured.
	int					GetAgingRepeat(int iKind) const;
	void				AgingRepeatNext(int iKind);
	int					GetAgingKindMax(int iKind) const;	// reserved blocks for the kind (>= 1)
	CAgingCycleInfo*	GetAgingCycleInfo()			{ return &m_AgingCycleInfo; }
private:
	CAgingCycleInfo		m_AgingCycleInfo;
	bool				m_bAgingCycleActive;
	int					m_nAgingCurItem;
	int					m_nAgingRepeat[enumAgingMeasKind_MAX];	// LYS 2026-08-12 AgingCycle R1

public:
	CMeasureSequenceItemEngine(stAuroraMgrPool &MgrPool);
	~CMeasureSequenceItemEngine();
};