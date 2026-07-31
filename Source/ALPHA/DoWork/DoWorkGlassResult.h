#pragma once

#include "DoWorkGlassSequenceImpl.h"

class CDoWorkGlassResultStep;
class CDoWorkGlassResult : public CDoWorkGlassSequenceImpl
{
	std::vector<CDoWorkGlassResultStep*> m_vecSeqStep;
	CDoWorkGlassResultStep* GetSeqItemStep();
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
	CDoWorkGlassResult(stAuroraMgrPool &MgrPool);
	virtual ~CDoWorkGlassResult();
};