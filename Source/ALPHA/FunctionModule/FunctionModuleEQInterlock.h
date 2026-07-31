#pragma once

#include "..\BaseSequenceImpl.h"

class CFunctionModuleEQInterlock : public CBaseSequenceImpl
{
public:
	//Interlock
	int GetInterlockMoveX();
	int GetInterlockMoveY();
	int GetInterlockMoveZ();

	int GetInterlockMoveLDX();
	int GetInterlockMoveLDY();
	int GetInterlockMoveLDR();
	int GetInterlockMoveLDZ();

	int GetinterlockIOLP();
	int GetinterlockIOClamp();
	int GetinterlockIOVacuum();
	int GetinterlockIOAir();


	int ChkMotionInterlock(int nType);					//ACS Buffer 내의 인터락 체크용

private:
	//FunctionModuleEQState로 옮겨야 될 것 같음
	int IsStageSafety();
	int IsHeadSafety(int nAxis = -1);

	int IsLDXLoading();
	int IsLDXContact();
	int IsLDZLoading();
	int IsLDZContact();

	int IsLPUp();
	int IsLPDown();

	int IsClampFold();
	int IsClampUnfold();

	int IsGlassExist(int nState = -1);

	//=========================================================================
	// INTERFACE 관련
	//=========================================================================
public:
	// Create / Delete
	virtual bool	CreateDoWorkSeq()										override;
	virtual void	DeleteDoWorkSeq()										override;
	virtual bool	ChangeState()											override;
	virtual bool	UpdateState()											override;
	virtual bool	LinkDataBase(CBinaryHelper *pBinaryHelper, bool bSave)	override;
	virtual void	ThreadRun()												override;

public:
	CFunctionModuleEQInterlock(stAuroraMgrPool &MgrPool);
	virtual ~CFunctionModuleEQInterlock(void);
};

