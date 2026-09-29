#pragma once

#include "DoWorkGlassSequenceImpl.h"

class CDoWorkGlassMeasureStep;
class CMeasureSequenceItemEngine;
class CDoWorkGlassMeasure : public CDoWorkGlassSequenceImpl
{
	std::vector<CDoWorkGlassMeasureStep*> m_vecSeqStep;
	CDoWorkGlassMeasureStep* GetSeqItemStep();
	bool EfaPassBoundary(int nPass);			// LYS 2026-09-28 EFA: T0->T1 경계 (결과 저장 + CIM 핸드셰이크)
	bool IsEfaSkipStep(const CString& strName);	// LYS 2026-09-28 EFA: T1 이후 반복하지 않는 스텝
	CMeasureSequenceItemEngine *m_pMeasSeqItemEngine;
	CBaseSequenceImpl *m_pShareItem;

//=========================================================================
// INTERFACE 관련
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