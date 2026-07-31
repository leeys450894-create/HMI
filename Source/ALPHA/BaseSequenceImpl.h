#pragma once

#include "BaseSequenceBody.h"

class CBinaryHelper;
class CBaseSequenceImpl : public CBaseSequenceBody
{

//=========================================================================
// Thread Run
//=========================================================================
protected:
	CWinThread	*m_pThread;
	bool		m_bThreadLive;
	bool		m_bIsThreadRun;
	HANDLE		m_hStartEvent;
	HANDLE		m_hCloseEvent;
	HANDLE		m_hMultiEvent[2];
	HANDLE		m_hWaitTimeEvent;
    HANDLE		m_hCompleteEvent;
public:
	static UINT ThreadRun(LPVOID p1);
	HANDLE	GetStartEventHandle()		{ return m_hStartEvent;				}
	HANDLE	GetWaitTimeEventHandle()	{ return m_hWaitTimeEvent;			}
	HANDLE	GetCompleteEventHandle()	{ return m_hCompleteEvent;			}


//=========================================================================
// INTERFACE 관련
//=========================================================================
public:
	// Create / Delete
	virtual bool	CreateDoWorkSeq() = 0;										// pure virtual
	virtual void	DeleteDoWorkSeq() = 0;										// pure virtual
	virtual bool	ChangeState() = 0;											// pure virtual
	virtual bool	UpdateState() = 0;											// pure virtual
	virtual bool	LinkDataBase(CBinaryHelper *pBinaryHelper, bool bSave) = 0;	// pure virtual

	// Link DataBase, Link Param
	virtual void	UpdateDataBaseInformation();
	virtual	void	UpdateLinkParamInformation(CCommTcpIPMgr *pCommTcpIPMgr);
	// Thread 관련
	virtual void	ThreadRun();
	virtual void	Start();
	virtual void	Stop();
	virtual void	Skip();
	virtual void	Exit();
	virtual void	Pause();
	virtual void	Resume();
	virtual bool	Is_Complete();
	virtual bool	Is_Fail();
	virtual bool	Is_AlarmOn();
	virtual bool	Is_ThreadRun();

public:
	CBaseSequenceImpl(stAuroraMgrPool &MgrPool, int iSeqItemNumber);
	virtual ~CBaseSequenceImpl();
};
