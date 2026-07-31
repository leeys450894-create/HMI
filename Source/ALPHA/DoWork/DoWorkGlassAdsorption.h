#pragma once

#include "DoWorkGlassSequenceImpl.h"

class CDoWorkGlassAdsorptionStep;
class CDoWorkGlassAdsorption : public CDoWorkGlassSequenceImpl
{
	std::vector<CDoWorkGlassAdsorptionStep*> m_vecSeqStep;
	CDoWorkGlassAdsorptionStep* GetSeqItemStep();
	CBaseSequenceImpl* m_pShareItem;

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
	CDoWorkGlassAdsorption(stAuroraMgrPool &MgrPool);
	virtual ~CDoWorkGlassAdsorption();
};