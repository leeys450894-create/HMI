#pragma once

#include "DoWorkGlassSequenceImpl.h"

class CDoWorkGlassUnloadStep;
class CDoWorkGlassUnload : public CDoWorkGlassSequenceImpl
{
	std::vector<CDoWorkGlassUnloadStep*> m_vecSeqStep;
	CDoWorkGlassUnloadStep* GetSeqItemStep();
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
	CDoWorkGlassUnload(stAuroraMgrPool &MgrPool);
	virtual ~CDoWorkGlassUnload();
};