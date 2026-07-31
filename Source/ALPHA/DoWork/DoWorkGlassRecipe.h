#pragma once

#include "DoWorkGlassSequenceImpl.h"

class CDoWorkGlassRecipeStep;
class CDoWorkGlassRecipe : public CDoWorkGlassSequenceImpl
{
	std::vector<CDoWorkGlassRecipeStep*> m_vecSeqStep;
	CDoWorkGlassRecipeStep* GetSeqItemStep();
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
	CDoWorkGlassRecipe(stAuroraMgrPool& MgrPool);
	virtual ~CDoWorkGlassRecipe();
};

