#pragma once
#include "DoWorkGlassSequenceImpl.h"

class CDoWorkGlassPreMeasStep;
class CMeasureSequenceItemEngine;
class CDoWorkGlassPreMeas : public CDoWorkGlassSequenceImpl
{
	std::vector<CDoWorkGlassPreMeasStep*> m_vecSeqStep;
	CDoWorkGlassPreMeasStep* GetSeqItemStep();
	CMeasureSequenceItemEngine* m_pMeasSeqItemEngine;
	CBaseSequenceImpl* m_pShareItem;


//=========================================================================
// INTERFACE ฐüทร
//=========================================================================
public:
	virtual bool	CreateDoWorkSeq()										override;
	virtual void	DeleteDoWorkSeq()										override;
	virtual bool	ChangeState()											override;
	virtual bool	UpdateState()											override;
	virtual bool	LinkDataBase(CBinaryHelper* pBinaryHelper, bool bSave)	override;
	virtual void	ThreadRun()												override;

public:
	CDoWorkGlassPreMeas(stAuroraMgrPool& MgrPool);
	virtual ~CDoWorkGlassPreMeas();
};

