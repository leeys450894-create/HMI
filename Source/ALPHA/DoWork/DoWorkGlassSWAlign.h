#pragma once

#include "DoWorkGlassSequenceImpl.h"

class CDoWorkGlassSWAlignStep;
class CDoWorkGlassSWAlign : public CDoWorkGlassSequenceImpl
{
	std::vector<CDoWorkGlassSWAlignStep*> m_vecSeqStep;
	CDoWorkGlassSWAlignStep* GetSeqItemStep();
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
	CDoWorkGlassSWAlign(stAuroraMgrPool &MgrPool);
	virtual ~CDoWorkGlassSWAlign();
};