#pragma once

#include "..\BaseSequenceImpl.h"

class CFunctionModuleCenteringStep;
class CFunctionModuleCentering : public CBaseSequenceImpl
{
	std::vector<CFunctionModuleCenteringStep*> m_vecSeqStep;
	CFunctionModuleCenteringStep* GetSeqItemStep();

public:
	void SetAlignType(int nType = 1);
	int m_nAlignType;

	//Head
	void SetHead(int nHead = 0);
	int m_nHead;

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
	CFunctionModuleCentering(stAuroraMgrPool &MgrPool);
	virtual ~CFunctionModuleCentering();
};