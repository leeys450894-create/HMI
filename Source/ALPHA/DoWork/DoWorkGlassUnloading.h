#pragma once

#include "DoWorkGlassSequenceImpl.h"

class CDoWorkGlassUnloadingStep;
class CDoWorkGlassUnloading : public CDoWorkGlassSequenceImpl
{
	std::vector<CDoWorkGlassUnloadingStep*> m_vecSeqStep;
	CDoWorkGlassUnloadingStep* GetSeqItemStep();

//=========================================================================
// INTERFACE ฐüทร
//=========================================================================
public:
	virtual bool	CreateDoWorkSeq()										override;
	virtual void	DeleteDoWorkSeq()										override;
	virtual bool	ChangeState()											override;
	virtual bool	UpdateState()											override;
	virtual bool	LinkDataBase(CBinaryHelper *pBinaryHelper, bool bSave)	override;
	virtual void	ThreadRun()												override;

public:
	CDoWorkGlassUnloading(stAuroraMgrPool &MgrPool);
	virtual ~CDoWorkGlassUnloading();
};