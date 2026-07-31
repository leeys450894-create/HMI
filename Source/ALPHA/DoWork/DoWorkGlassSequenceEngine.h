#pragma once

#include "..\AuroraMgrPool.h"

class CBinaryHelper;
class CLogFileAccessor;
class CDoWorkGlassSequenceImpl;
class CDoWorkGlassSequenceEngine
{
	CLogFileAccessor *m_pLogFileAccessor;
	stAuroraMgrPool				m_MgrPool_link;
	std::vector<CDoWorkGlassSequenceImpl *>	m_vecItems;
	bool						m_bEngineBreak;	

public:
	bool LinkDataBase(CBinaryHelper *pBinaryHelper, bool bSave);
	void UpdateLinkParamInformation(CCommTcpIPMgr *pCommTcpIPMgr);
	CRecipeEditMgr	*GetRecipeEditMgrPtr()		{ return m_MgrPool_link.pRecipeEditMgr;			}
	CSeqTactTimeMgr	*GetSeqTactTimeMgrPtr()		{ return m_MgrPool_link.pSeqTactTimeMgr;		}
	CIOMapControlMgr *GetIOMapControlMgrPtr()	{ return m_MgrPool_link.pIOMapControlMgr; }



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
	CDoWorkGlassSequenceImpl *GetItem(int index);
	CDoWorkGlassSequenceImpl *FindItem(CString strSeq);
	void				ChangeItemState();
	void				UpdateItemState();
	void				UpdateDataBaseInformation();

	void				ResetResumeState();
	void				UserResumeState();
	void				UserPauseState();
	void				UserStopState(bool bSet);

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
	bool ThreadRu_nSequence(int cycle_index, bool bFirst);
	bool IsThreadStopState();

public:
	CDoWorkGlassSequenceEngine(stAuroraMgrPool &MgrPool);
	~CDoWorkGlassSequenceEngine();
};