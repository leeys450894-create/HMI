#pragma once

#include "..\BaseSequenceImpl.h"

class CDoWorkGlassSequenceEngine;
class CDoWorkGlassSequence : public CBaseSequenceImpl
{
	CDoWorkGlassSequenceEngine *m_pSeqEngine;
	

public:
	CString GetItemName(int index);
	CLogFileAccessor *GetIteLogFileAccessorPtr(int index);
	void EngineStart();
	void EngineResume();
	void EnginePause();
	void EngineStop();
	bool Is_CompleteAll();
	bool Is_Complete(int index);
	bool ItemStart(CString strType, bool bCompleteCheck);
	void ItemStopAndSkip(CString strType);
	void ItemStopAndExit(CString strType);


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
	CDoWorkGlassSequence(stAuroraMgrPool &MgrPool);
	virtual ~CDoWorkGlassSequence();

};