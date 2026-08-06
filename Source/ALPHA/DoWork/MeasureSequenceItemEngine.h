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
	bool				AgingCycleBegin();		// gate(bIsOCP + recipe m_bUseAgingTest_2) + aging rcp load
	void				AgingCycleEnd();
	bool				AgingCycleNextItem();	// advance to next USE_AGING item. FALSE = exhausted
	bool				IsAgingCycleActive() const	{ return m_bAgingCycleActive; }
	int					GetAgingCurItem() const		{ return m_nAgingCurItem; }
	CAgingCycleInfo*	GetAgingCycleInfo()			{ return &m_AgingCycleInfo; }
private:
	CAgingCycleInfo		m_AgingCycleInfo;
	bool				m_bAgingCycleActive;
	int					m_nAgingCurItem;

public:
	CMeasureSequenceItemEngine(stAuroraMgrPool &MgrPool);
	~CMeasureSequenceItemEngine();
};