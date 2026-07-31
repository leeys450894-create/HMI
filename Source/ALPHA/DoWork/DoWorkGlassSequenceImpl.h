#pragma once

#include "..\BaseSequenceImpl.h"

// 빈 껍데이 이다. 이름을 분리 시키기 위해서 사용하려고 한다.
class CDoWorkGlassSequenceImpl : public CBaseSequenceImpl
{
	
public:
	CDoWorkGlassSequenceImpl(stAuroraMgrPool &MgrPool, int iSeqItemNumber);
	virtual ~CDoWorkGlassSequenceImpl();
};
