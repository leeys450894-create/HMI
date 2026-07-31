#pragma once

#include "..\BaseSequenceImpl.h"

enum enumEQStatus
{
	enumEQSTATUS_NONE = 0,
	enumEQSTATUS_R2L,
	enumEQSTATUS_L2M,
	enumEQSTATUS_M2R,
	enumEQSTATUS_R2M,
	enumEQSTATUS_M2L,
	enumEQSTATUS_L2R,
	enumEQSTATUS_SAFETY,
	enumEQSTATUS_CHANGE_COUNT
};



class CFunctionModuleEQStateChangeStep;
class CFunctionModuleEQStateChange : public CBaseSequenceImpl
{
	std::vector<CFunctionModuleEQStateChangeStep*> m_vecSeqStep;

	std::vector<CFunctionModuleEQStateChangeStep*> m_vecSeqStepR2L;
	std::vector<CFunctionModuleEQStateChangeStep*> m_vecSeqStepL2R;
	std::vector<CFunctionModuleEQStateChangeStep*> m_vecSeqStepR2M;
	std::vector<CFunctionModuleEQStateChangeStep*> m_vecSeqStepM2L;
	std::vector<CFunctionModuleEQStateChangeStep*> m_vecSeqStepL2M;
	std::vector<CFunctionModuleEQStateChangeStep*> m_vecSeqStepM2R;
	std::vector<CFunctionModuleEQStateChangeStep*> m_vecSeqStepSafety;



	std::vector<CFunctionModuleEQStateChangeStep*> m_vecSeqStepALL;	// 메모리 삭제용

public:
	CLogFileAccessor *GetLogFileAccessorPtr() { return m_pLogFileAccessor; }


	//상태 전환 명령 시 필요할 듯?
	long m_lCurrentStep;
	void SetEQStatusChangeStep(long lStep);
	long GetEQStatusChangeStep() { return m_lCurrentStep; }
	void SetEQStatusExitStepIndex(long lStep);
	void UpdateStep();

	long m_lEQStatusStepStart[enumEQSTATUS_CHANGE_COUNT];
	long m_lEQStatusStepEnd[enumEQSTATUS_CHANGE_COUNT];
	///

//=========================================================================
// INTERFACE 관련
//=========================================================================
public:
	virtual bool	CreateDoWorkSeq()										override;
	virtual void	DeleteDoWorkSeq()										override;
	virtual bool	ChangeState()											override;
	virtual bool	UpdateState()											override;
	virtual bool	LinkDataBase(CBinaryHelper *pBinaryHelper, bool bSave)	override;
	virtual void	ThreadRun()												override;

public:
	CFunctionModuleEQStateChange(stAuroraMgrPool &MgrPool);
	virtual ~CFunctionModuleEQStateChange();
};