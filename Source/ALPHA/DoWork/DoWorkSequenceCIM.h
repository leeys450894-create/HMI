#pragma once

#include "BaseSequenceImpl.h"

enum enumCIMStatus
{
	enumCIMStatus_INIT = 0,
	enumCIMStatus_READY,
	enumCIMStatus_LOAD_READY,
	enumCIMStatus_LOAD,
	enumCIMStatus_MEAS,
	enumCIMStatus_MEAS_DONE,
	enumCIMStatus_DATA_UPLOAD,
	enumCIMStatus_UNLOAD_READY,
	enumCIMStatus_UNLOAD,
	enumCIMStatus_CHANGE_COUNT
};

class CDoWorkSequenceCIMStep;
class CDoWorkSequenceCIM : public  CBaseSequenceImpl
{
	std::vector<CDoWorkSequenceCIMStep*> m_vecSeqStepInit;
	std::vector<CDoWorkSequenceCIMStep*> m_vecSeqStepReady;
	std::vector<CDoWorkSequenceCIMStep*> m_vecSeqSteploadReady;
	std::vector<CDoWorkSequenceCIMStep*> m_vecSeqStepload;
	std::vector<CDoWorkSequenceCIMStep*> m_vecSeqStepMeas;
	std::vector<CDoWorkSequenceCIMStep*> m_vecSeqStepMeasDone;
	std::vector<CDoWorkSequenceCIMStep*> m_vecSeqStepDataUpload;
	std::vector<CDoWorkSequenceCIMStep*> m_vecSeqStepUnloadReady;
	std::vector<CDoWorkSequenceCIMStep*> m_vecSeqStepUnload;

	std::vector<CDoWorkSequenceCIMStep*> m_vecSeqStepALL;	// 메모리 삭제용

	CDoWorkSequenceCIMStep* GetSeqItemStep();
	CBaseSequenceImpl* m_pShareItem;

	long m_lCIMStatusStepStart[enumCIMStatus_CHANGE_COUNT];
	long m_lCIMStatusStepEnd[enumCIMStatus_CHANGE_COUNT];

	//=========================================================================
	// INTERFACE 관련
	//=========================================================================
public:
	virtual bool	CreateDoWorkSeq()										override;
	virtual void	DeleteDoWorkSeq()										override;
	virtual bool	ChangeState()											override;
	virtual bool	UpdateState()											override;
	virtual bool	LinkDataBase(CBinaryHelper* pBinaryHelper, bool bSave)	override;
	virtual void	ThreadRun()												override;

public:
	CDoWorkSequenceCIM(stAuroraMgrPool& MgrPool);
	virtual ~CDoWorkSequenceCIM();
	CLogFileAccessor* GetLogFileAccessorPtr() { return m_pLogFileAccessor; }

	bool m_bThreadRun;
	long m_lCurrentStep;
	void SetCIMStatusChangeStep(long lStep);
	long GetCIMStatusChangeStep() { return m_lCurrentStep; }
	void SetCIMStatusExitStepIndex(long lStep);
	bool GetThreadRun() { return m_bThreadRun;  }
};


