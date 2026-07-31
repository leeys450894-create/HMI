#pragma once

#include "DoWorkGlassSequenceImpl.h"

class CDoWorkGlassMeasureStep;
class CMeasureSequenceItemEngine;
class CDoWorkGlassMeasure : public CDoWorkGlassSequenceImpl
{
	std::vector<CDoWorkGlassMeasureStep*> m_vecSeqStep;
	CDoWorkGlassMeasureStep* GetSeqItemStep();
	CMeasureSequenceItemEngine *m_pMeasSeqItemEngine;
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
	CDoWorkGlassMeasure(stAuroraMgrPool &MgrPool);
	virtual ~CDoWorkGlassMeasure();
};