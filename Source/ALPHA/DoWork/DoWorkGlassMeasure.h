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
	bool SegmentBoundary(int nFrom, int nTo);	// LYS 2026-09-29 Hybrid Phase 3 세그먼트 경계 (Safety -> Stage R 회전 -> 재Align, PG/WAD 재송신)
	bool SegmentSwitchData(int nSeg);			// LYS 2026-09-29 Hybrid Phase 3 활성 세그먼트 + 세그먼트 .alg 전환 (모션 없음)
	void SegmentRestore0();						// LYS 2026-09-29 Hybrid Phase 3 결과/다음 글라스용 세그먼트 0 복귀 (모션 없음)
	int  m_nCurSeg;								// LYS 2026-09-29 Hybrid Phase 3 지금 Stage·데이터가 맞춰진 세그먼트
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