#pragma once

#include "..\BaseSequenceBody.h"

class CBinaryHelper;
class CRecipeData;
class CCommEthFunction;
class CMeasureSequenceItemImpl : public CBaseSequenceBody
{
protected:
	CRecipeData			*m_pRecipeDataClone;
	CCommEthFunction	*m_pCommTcpIpObject;
public:
	CRecipeData	*GetRecipeDataPtr()				{ return m_pRecipeDataClone;		}
	CCommEthFunction *GetCommTcpIpObject()		{ return m_pCommTcpIpObject;		}


//=========================================================================
// Thread Run
//=========================================================================
protected:
	CWinThread	*m_pThread;
	bool		m_bThreadLive;
	HANDLE		m_hStartEvent;
	HANDLE		m_hCloseEvent;
	HANDLE		m_hMultiEvent[2];
	HANDLE		m_hWaitTimeEvent;
    HANDLE		m_hCompleteEvent;
public:
	static UINT ThreadRun(LPVOID p1);
	HANDLE	GetStartEventHandle()				{ return m_hStartEvent;				}
	HANDLE	GetWaitTimeEventHandle()			{ return m_hWaitTimeEvent;			}
	HANDLE	GetCompleteEventHandle()			{ return m_hCompleteEvent;			}
		


//=========================================================================
// INTERFACE 관련
//=========================================================================
public:
	virtual void	Start();
	virtual void	Stop();
	virtual void	Skip();
	virtual void	Exit();
	virtual void	Pause();
	virtual void	Resume();
	virtual bool	Is_Complete();

	virtual bool	CreateSeqItem() = 0;										// pure virtual
	virtual bool	LinkDataBase(CBinaryHelper *pBinaryHelper, bool bSave) = 0;	// pure virtual
	virtual bool	UpdateState() = 0;											// pure virtual
	virtual void	ThreadRun() = 0;											// pure virtual
	
	// 검사 관련
	virtual bool	Run_AutoFocus() = 0;										// pure virtual
	virtual bool	Run_Centering() = 0;										// pure virtual
	virtual bool	Run_GetImage() = 0;											// pure virtual
	virtual bool	Run_Measure() = 0;											// pure virtual
	virtual bool	Run_MeasureEnd() = 0;										// pure virtual
	virtual bool	Run_Complete() = 0;											// pure virtual

public:
	CMeasureSequenceItemImpl(stAuroraMgrPool &MgrPool, int nIndex);
	virtual ~CMeasureSequenceItemImpl();
};
