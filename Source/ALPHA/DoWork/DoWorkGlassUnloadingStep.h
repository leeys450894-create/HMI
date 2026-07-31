#pragma once

class CBaseSequenceImpl;
class CDoWorkGlassUnloadingStep
{
protected:
	CBaseSequenceImpl *m_pShareItem;
public:
	virtual bool Run() = 0;

public:
	CDoWorkGlassUnloadingStep(CBaseSequenceImpl *pShareItem){ m_pShareItem = pShareItem; }
	virtual ~CDoWorkGlassUnloadingStep(){}
};

class CDoWorkGlassUnloadingStep_None : public CDoWorkGlassUnloadingStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassUnloadingStep_None(CBaseSequenceImpl *pShareItem) : CDoWorkGlassUnloadingStep(pShareItem) {}
	virtual ~CDoWorkGlassUnloadingStep_None() {}
};

class CDoWorkGlassUnloadingStep_ProcessSelect : public CDoWorkGlassUnloadingStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassUnloadingStep_ProcessSelect(CBaseSequenceImpl *pShareItem) : CDoWorkGlassUnloadingStep(pShareItem) {}
	virtual ~CDoWorkGlassUnloadingStep_ProcessSelect() {}
};

class CDoWorkGlassUnloadingStep_ProcessSelectCheck : public CDoWorkGlassUnloadingStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassUnloadingStep_ProcessSelectCheck(CBaseSequenceImpl *pShareItem) : CDoWorkGlassUnloadingStep(pShareItem) {}
	virtual ~CDoWorkGlassUnloadingStep_ProcessSelectCheck() {}
};

class CDoWorkGlassUnloadingStep_SelectTool : public CDoWorkGlassUnloadingStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassUnloadingStep_SelectTool(CBaseSequenceImpl *pShareItem) : CDoWorkGlassUnloadingStep(pShareItem) {}
	virtual ~CDoWorkGlassUnloadingStep_SelectTool() {}
};

class CDoWorkGlassUnloadingStep_InitPosition : public CDoWorkGlassUnloadingStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassUnloadingStep_InitPosition(CBaseSequenceImpl *pShareItem) : CDoWorkGlassUnloadingStep(pShareItem) {}
	virtual ~CDoWorkGlassUnloadingStep_InitPosition() {}
};

class CDoWorkGlassUnloadingStep_InitPosition2 : public CDoWorkGlassUnloadingStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassUnloadingStep_InitPosition2(CBaseSequenceImpl *pShareItem) : CDoWorkGlassUnloadingStep(pShareItem) {}
	virtual ~CDoWorkGlassUnloadingStep_InitPosition2() {}
};

class CDoWorkGlassUnloadingStep_InitPositionCheck : public CDoWorkGlassUnloadingStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassUnloadingStep_InitPositionCheck(CBaseSequenceImpl *pShareItem) : CDoWorkGlassUnloadingStep(pShareItem) {}
	virtual ~CDoWorkGlassUnloadingStep_InitPositionCheck() {}
};

class CDoWorkGlassUnloadingStep_VacuumOFF : public CDoWorkGlassUnloadingStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassUnloadingStep_VacuumOFF(CBaseSequenceImpl *pShareItem) : CDoWorkGlassUnloadingStep(pShareItem) {}
	virtual ~CDoWorkGlassUnloadingStep_VacuumOFF() {}
};

class CDoWorkGlassUnloadingStep_VacuumOFFCheck : public CDoWorkGlassUnloadingStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassUnloadingStep_VacuumOFFCheck(CBaseSequenceImpl *pShareItem) : CDoWorkGlassUnloadingStep(pShareItem) {}
	virtual ~CDoWorkGlassUnloadingStep_VacuumOFFCheck() {}
};

class CDoWorkGlassUnloadingStep_Complete : public CDoWorkGlassUnloadingStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassUnloadingStep_Complete(CBaseSequenceImpl *pShareItem) : CDoWorkGlassUnloadingStep(pShareItem) {}
	virtual ~CDoWorkGlassUnloadingStep_Complete() {}
};

class CDoWorkGlassUnloadingStep_CompleteCheck : public CDoWorkGlassUnloadingStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassUnloadingStep_CompleteCheck(CBaseSequenceImpl *pShareItem) : CDoWorkGlassUnloadingStep(pShareItem) {}
	virtual ~CDoWorkGlassUnloadingStep_CompleteCheck() {}
};

class CDoWorkGlassUnloadingStep_Exit : public CDoWorkGlassUnloadingStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassUnloadingStep_Exit(CBaseSequenceImpl *pShareItem) : CDoWorkGlassUnloadingStep(pShareItem) {}
	virtual ~CDoWorkGlassUnloadingStep_Exit() {}
};

class CDoWorkGlassUnloadingStep_ExitCheck : public CDoWorkGlassUnloadingStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassUnloadingStep_ExitCheck(CBaseSequenceImpl *pShareItem) : CDoWorkGlassUnloadingStep(pShareItem) {}
	virtual ~CDoWorkGlassUnloadingStep_ExitCheck() {}
};

class CDoWorkGlassUnloadingStep_Idle : public CDoWorkGlassUnloadingStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassUnloadingStep_Idle(CBaseSequenceImpl *pShareItem) : CDoWorkGlassUnloadingStep(pShareItem) {}
	virtual ~CDoWorkGlassUnloadingStep_Idle() {}
};