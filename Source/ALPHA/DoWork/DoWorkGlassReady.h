#pragma once

#include "DoWorkGlassSequenceImpl.h"

class CDoWorkGlassReadyStep;
class CDoWorkGlassReady : public CDoWorkGlassSequenceImpl
{
	std::vector<CDoWorkGlassReadyStep*> m_vecSeqStep;
	CDoWorkGlassReadyStep* GetSeqItemStep();
	CBaseSequenceImpl *m_pShareItem;

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
	CDoWorkGlassReady(stAuroraMgrPool &MgrPool);
	virtual ~CDoWorkGlassReady();
};