#pragma once
#include "DoWorkGlassSequenceImpl.h"

class CDoWorkGlassAGINGStep;
class CMeasureSequenceItemEngine;
class CDoWorkGlassAGING : public CDoWorkGlassSequenceImpl
{
	std::vector<CDoWorkGlassAGINGStep*> m_vecSeqStep;
	CDoWorkGlassAGINGStep* GetSeqItemStep();
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
	CDoWorkGlassAGING(stAuroraMgrPool& MgrPool);
	virtual ~CDoWorkGlassAGING();
};

