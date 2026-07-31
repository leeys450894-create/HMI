#pragma once

#include "..\BaseSequenceImpl.h"

class CSystemInitializeStep;
class CSystemInitialize : public CBaseSequenceImpl
{
	std::vector<CSystemInitializeStep*> m_vecSeqStep;


//=========================================================================
// INTERFACE ฐüทร
//=========================================================================
public:
	// Create / Delete
	virtual bool	CreateDoWorkSeq()										override;
	virtual void	DeleteDoWorkSeq()										override;
	virtual bool	ChangeState()											override;
	virtual bool	UpdateState()											override;
	virtual bool	LinkDataBase(CBinaryHelper *pBinaryHelper, bool bSave)	override;
	virtual void	ThreadRun()												override;

public:
	CSystemInitialize(stAuroraMgrPool &MgrPool);
	virtual ~CSystemInitialize();
	CSystemInitializeStep* GetSeqItemStep();
	CLogFileAccessor *GetLogFileAccessorPtr() { return m_pLogFileAccessor; }
};