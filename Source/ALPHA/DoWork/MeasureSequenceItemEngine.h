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

	// LYS 2026-08-12 InnerID 유일성: gf_GetCurTimeString(3)이 HHMMSS(1초 해상도)라
	// 같은 초의 측정 두 건이 같은 id -> 같은 raw 파일명 -> 나중 결과가 앞을 덮어썼다.
	// 3회 반복 에이징 실행에서 16/315(5%) 관측. 장비 패킷 배치를 건드리지 않도록
	// 원래 길이를 유지하고 끝 2자리만 재배정하며, 발급 목록이 글라스 단위 유일성을 보장한다.
	std::vector<CString>	m_vecIssuedInnerID;
public:
	CString MakeUniqueInnerID(const CString& strID);
	void PushIssuedInnerID(const CString& strID);

//=========================================================================
// SeqItem 관련
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
// LYS 2026-08-06 AgingCycle E3: 에이징 리스트 사이클 컨텍스트(OCP DoWorkStartMeasOCP_A).
// CDoWorkGlassMeasure::ThreadRun이 측정 스레드에서 구동한다(단일 스레드 사용).
//=========================================================================
public:
	bool				AgingCycleBegin(int nEfaPass = -1, int nStick = -1);	// 게이트(bIsOCP + 레시피 m_bUseAging_Test) + 에이징 rcp 로드. nEfaPass>=0: EFA 고정 항목 (// LYS 2026-09-28 EFA)
	bool				IsAgingStickSkip() const	{ return m_bAgingStickSkip; }	// LYS 2026-09-30 AgingCycleBegin(nStick >= 0) found no USE_AGING item with a cell on the stick
	bool				IsSamplingStickSkip(int nStick);	// LYS 2026-10-08 Sampling stick skip: standard glass, OCP, recipe Sampling on, no cell of the stick selected for the enabled WAD/VISION/ANGLE items (a 1-cell stick never, D-SS-4; a 0-cell stick has none) -> ThreadRun skips the stick like the 3c skip (OCP nGroup < 1). Logs the skip
	bool				IsSamplingCycleSkip() const	{ return m_bSamplingCycleSkip; }	// LYS 2026-10-08 Sampling stick skip: the last AgingCycleBegin turned its cycle stick into this skip (IsAgingStickSkip() is true as well)
	void				AgingCycleEnd();
	bool				AgingCycleNextItem();	// advance to next USE_AGING item. FALSE = exhausted
	bool				IsAgingCycleActive() const	{ return m_bAgingCycleActive; }
	int					GetAgingCurItem() const		{ return m_nAgingCurItem; }
	void				AgingGlassLatch(int nEfaPassCount = 0);				// LYS 2026-09-29 Hybrid 3b A4 once per glass, before the segment loop. LYS 2026-09-30 3c A5 + publishes the EFA glass latch (N>1), -1 (1 segment)

	// LYS 2026-09-28 EFA: 2-pass EFA 검사(OCP DoWorkStartMeasICP_EFA). Pass = 리스트 항목, 항목 루프는 스틱 루프 바깥.
	int					GetEfaPassCount();				// 0 = EFA 아님(일반 단일 pass). {EFA_RCP}\{rcp}.rcp를 읽는다
	bool				IsEfaCycle() const			{ return m_bEfaCycle; }

	// LYS 2026-08-12 AgingCycle R1: 결과 행 블록용 조밀 종류별 반복 인덱스.
	// 스텝 진입 시 캡처해 nAreaNum에 쓰고, 해당 종류를 실제 측정한 뒤 1회 증가.
	int					GetAgingRepeat(int iKind) const;
	void				AgingRepeatNext(int iKind);
	int					GetAgingKindMax(int iKind) const;	// reserved blocks for the kind (>= 1)
	CAgingCycleInfo*	GetAgingCycleInfo()			{ return &m_AgingCycleInfo; }
private:
	CAgingCycleInfo		m_AgingCycleInfo;
	bool				m_bAgingCycleActive;
	int					m_nAgingCurItem;
	int					m_nAgingRepeat[enumAgingMeasKind_MAX];	// LYS 2026-08-12 AgingCycle R1
	bool				m_bEfaCycle;						// LYS 2026-09-28 EFA
	int					m_nAgingGlassLatch;				// LYS 2026-09-29 Hybrid 3b A4 -1 = not latched (1 segment), 0/1 = segment 0 m_bUseAging_Test
	unsigned int		m_nAgingMaskChecked;			// LYS 2026-09-30 bit k = segment k mask shape already checked on this glass
	void				CheckAgingMaskShape();			// LYS 2026-09-30 WARN once per glass and segment when the loaded mask does not fit the recipe
	int					m_nAgingStick;					// LYS 2026-09-30 stick of the running cycle (-1 = not given: item-only selection)
	bool				m_bAgingStickSkip;				// LYS 2026-09-30 set by AgingCycleBegin, cleared by AgingCycleEnd
	bool				m_bSamplingCycleSkip;			// LYS 2026-10-08 Sampling stick skip: set (with m_bAgingStickSkip) by SamplingCycleSkip, cleared by AgingCycleBegin / AgingCycleEnd
	bool				SamplingCycleSkip(int iOnlyItem);	// LYS 2026-10-08 Sampling stick skip (D-SS-3 b1s): cycle stick without a mask cell selected by Sampling -> 3c-style stick skip + log
	bool				AgingItemHasCell(int iItem);	// LYS 2026-09-30 same test as the WAD/VISION/ANGLE step gates
	CString				m_strCycleSegTag;				// LYS 2026-09-30 Hybrid 3c segment tag of the running cycle (the last AgingCycleEnd logs after the segment-0 restore)

public:
	CMeasureSequenceItemEngine(stAuroraMgrPool &MgrPool);
	~CMeasureSequenceItemEngine();
};