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
	bool				AgingCycleBegin();		// gate(bIsOCP + recipe m_bUseAging_Test) + aging rcp load
	void				AgingCycleEnd();
	bool				AgingCycleNextItem();	// advance to next USE_AGING item. FALSE = exhausted
	bool				IsAgingCycleActive() const	{ return m_bAgingCycleActive; }
	int					GetAgingCurItem() const		{ return m_nAgingCurItem; }

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

public:
	CMeasureSequenceItemEngine(stAuroraMgrPool &MgrPool);
	~CMeasureSequenceItemEngine();
};