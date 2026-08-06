#pragma once

#include "AuroraMgrPool.h"
#include <afxmt.h>

class CLogFileAccessor;
class CBaseSequenceBody
{
protected:
	CCriticalSection	m_cs;
	CLogFileAccessor	*m_pLogFileAccessor;
	stAuroraMgrPool		m_MgrPool_link;
public:
	CLogFileAccessor *GetLogFileAccessorPtr()			{ return m_pLogFileAccessor;					}
	CRecipeEditMgr *GetRecipeEditMgrPtr()				{ return m_MgrPool_link.pRecipeEditMgr;			}
	CSystemConfigMgr *GetSystemConfigMgrPtr()			{ return m_MgrPool_link.pSystemConfigMgr;		}
	CIOMapControlMgr *GetIOMapControlMgrPtr()			{ return m_MgrPool_link.pIOMapControlMgr;		} 
//	CMotorControlMgr *GetMotorControlMgrPtr()			{ return m_MgrPool_link.pMotorControlMgr;		}
	CLightControlMgr *GetLightControlMgrPtr()			{ return m_MgrPool_link.pLightControlMgr;		}
	CVCRControlMgr *GetVCRControlMgrPtr()				{ return m_MgrPool_link.pVCRControlMgr;			}
	CShareMemoryMgr *GetShareMemoryMgrPtr()				{ return m_MgrPool_link.pShareMemoryMgr;		}
	CResultCollectMgr *GetResultCollectMgrPtr()			{ return m_MgrPool_link.pResultCollectMgr;		}
	CPositionConfigMgr *GetPositionConfigMgrPtr()		{ return m_MgrPool_link.pPositionConfigMgr;		}
	CPatternMatchMgr *GetPatternMatchMgr()				{ return m_MgrPool_link.pPatternMatchMgr;		}
	CReviewCameraHead1Mgr *GetReviewCameraHead1Mgr()	{ return m_MgrPool_link.pReviewCameraHead1Mgr;	}
	CReviewCameraHead2Mgr *GetReviewCameraHead2Mgr()	{ return m_MgrPool_link.pReviewCameraHead2Mgr;	}
	CAlignMatrixMgr *GetAlignMatrixMgr()				{ return m_MgrPool_link.pAlignMatrixMgr;		}
	CCommTcpIpFactoryMgr *GetCommTcpIpFactoryMgr()		{ return m_MgrPool_link.pCommTcpIpFactoryMgr;	}
	CTriggerControlMgr   *GetTriggerControlMgr()		{ return m_MgrPool_link.pTriggerControlMgr; }
	CAlignCameraMgr		 *GetAlignCameraMgr(int i)		{ return m_MgrPool_link.pAlignCameraMgr[i]; }
	CSeqTactTimeMgr		 *GetSeqTactTimeMgrPtr()        { return m_MgrPool_link.pSeqTactTimeMgr; }
	CMaintControlMgr     *GetMaintControlMgrPtr()		{ return m_MgrPool_link.pMaintControlMgr; }
	CResumeInfoMgr       *GetResumeInfoMgrPtr()			{ return m_MgrPool_link.pResumeInfoMgr; }
	CWadMultiMeasMgr     *GetWadMultiMeasMgrPtr()		{ return m_MgrPool_link.pWadMultiMeasMgr; }	// LYS 2026-07-23

//=========================================================================
// module config
//=========================================================================
protected:
	CString	m_strSeqType;
	CString	m_strSeqComm;
	CString m_strSeqCorp;
	CString	m_strSeqName;
	CString	m_strSeqGUID;
	CString	m_strSeqDesc;
	int		m_iSeqItemNumber;
	bool	m_bVisible;
	bool	m_bComplete;
	bool    m_bFail;
	bool    m_bAlarmOn;
	bool    m_bStopUser;
	int		m_nFailNum;

	HANDLE		m_hEvtUserPause;            //set by UI		쓰레드 작업 임시 멈춤
	HANDLE		m_hEvtUserResume;           //set by UI		쓰레드 작업 임시 멈춤 해제

public:
	CString GetType() const 			{ return m_strSeqType;				}
	CString GetComm() const 			{ return m_strSeqComm;				}
	CString GetCorp() const 			{ return m_strSeqCorp;				}
	CString GetName() const 			{ return m_strSeqName;				}
	CString GetGUID() const 			{ return m_strSeqGUID;				}
	CString GetDesc() const 			{ return m_strSeqDesc;				}
	int		GetSeqItemNumber() const 	{ return m_iSeqItemNumber;			}
	void	SetType(CString strValue)	{ m_strSeqType = strValue;			}
	void	SetComm(CString strValue)	{ m_strSeqComm = strValue;			}
	void	SetName(CString strValue)	{ m_strSeqName = strValue;			}
	void	SetVisible(bool bValue)		{ m_bVisible = bValue;				}
	void	SetComplete(bool bValue)	{ m_bComplete = bValue;				}
	bool	GetComplete() const			{ return m_bComplete;				}
	void	SetFail(bool bValue, int nNum = 1)		{ m_bFail = bValue; m_nFailNum = nNum;}
	bool	GetFail()	const			{ return m_bFail; }
	void	SetAlarmOn(bool bValue)		{ m_bAlarmOn = bValue; }
	bool	GetAlarmOn() const			{ return m_bAlarmOn; }
	void	SetStopUser(bool bValue)	{ m_bStopUser = bValue; }
	bool	GetStopUser() const			{ return m_bStopUser; }

	int		GetFailNum() const { return m_nFailNum; }

	void	SetEvtPause() const			{ SetEvent(m_hEvtUserPause); }
	void	SetEvtResume() const		{ ResetEvent(m_hEvtUserPause);  SetEvent(m_hEvtUserResume); }
	void	ResetEvtPause() const		{ ResetEvent(m_hEvtUserPause); }

//=========================================================================
// module sequence
//=========================================================================
private:
	long m_iStepIndex;
	long m_iStepIndex_backup;
	long m_iStepIndex_loop;
	long m_iStepIndex_exit;
	long m_iStepIndex_idle;		// 아무것도 하지 않고 COMPLETE 시킨다.
	long m_iStepIndex_total;
public:
	void GoToInitStep()					{ m_iStepIndex = 0;					}
	void GoToNextStep()					{ m_iStepIndex++;					}
	void GoToBackStep()					{ m_iStepIndex--;					}
	void GoToLoopStep()					{ m_iStepIndex = m_iStepIndex_loop;	}
	void GoToExitStep()					{ m_iStepIndex = m_iStepIndex_exit;	}
	void GoToIDLEStep()					{ m_iStepIndex = m_iStepIndex_idle;	}
	long GetCurStep() const 			{ return m_iStepIndex;				}

	void SetBackupStepIndex(long value)	{ m_iStepIndex_backup = value;		}
	void SetLoopStepIndex(long value)	{ m_iStepIndex_loop = value;		}
	void SetExitStepIndex(long value)	{ m_iStepIndex_exit = value;		}
	void SetIDLEStepIndex(long value)	{ m_iStepIndex_idle = value;		}
	void SetTotalStepCount(long value)	{ m_iStepIndex_total = value;		}
	long GetBackupStepIndex() const		{ return m_iStepIndex_backup;		}
	long GetLoopStepIndex() const		{ return m_iStepIndex_loop;			}
	long GetExitStepIndex() const		{ return m_iStepIndex_exit;			}
	long GetIDLEStepIndex() const		{ return m_iStepIndex_idle;			}
	long GetTotalStepCount() const		{ return m_iStepIndex_total;		}
	bool IfPauseEvt_WaitResumeEvt() const;

public:
	CBaseSequenceBody(stAuroraMgrPool &MgrPool);
	virtual ~CBaseSequenceBody();
};
