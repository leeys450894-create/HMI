#pragma once

class CBaseSequenceImpl;
class CFunctionModuleEQStateChangeStep
{
protected:
	CString m_strName;
	CBaseSequenceImpl *m_pShareItem;
public:
	virtual bool Run() = 0;

	//¿Ãµø
	bool MoveLoaderXYR(int nType);
	long MoveLoaderZ(int nType);
	int MoveSafetyAll();
	int WaitMoveDone(int nType);

public:
	CFunctionModuleEQStateChangeStep(CBaseSequenceImpl *pShareItem) { m_pShareItem = pShareItem; }
	virtual ~CFunctionModuleEQStateChangeStep() {}
};

class CFunctionModuleEQStateChangeStep_None : public CFunctionModuleEQStateChangeStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleEQStateChangeStep_None(CBaseSequenceImpl *pShareItem) : CFunctionModuleEQStateChangeStep(pShareItem) {}
	virtual ~CFunctionModuleEQStateChangeStep_None() {}
};

class CFunctionModuleEQStateChangeStep_LPUp : public CFunctionModuleEQStateChangeStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleEQStateChangeStep_LPUp(CBaseSequenceImpl *pShareItem) : CFunctionModuleEQStateChangeStep(pShareItem) {}
	virtual ~CFunctionModuleEQStateChangeStep_LPUp() {}
};

class CFunctionModuleEQStateChangeStep_LoaderXYRMeas : public CFunctionModuleEQStateChangeStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleEQStateChangeStep_LoaderXYRMeas(CBaseSequenceImpl *pShareItem) : CFunctionModuleEQStateChangeStep(pShareItem) {}
	virtual ~CFunctionModuleEQStateChangeStep_LoaderXYRMeas() {}
};

class CFunctionModuleEQStateChangeStep_ClampOut : public CFunctionModuleEQStateChangeStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleEQStateChangeStep_ClampOut(CBaseSequenceImpl *pShareItem) : CFunctionModuleEQStateChangeStep(pShareItem) {}
	virtual ~CFunctionModuleEQStateChangeStep_ClampOut() {}
};

class CFunctionModuleEQStateChangeStep_LoaderZUp : public CFunctionModuleEQStateChangeStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleEQStateChangeStep_LoaderZUp(CBaseSequenceImpl *pShareItem) : CFunctionModuleEQStateChangeStep(pShareItem) {}
	virtual ~CFunctionModuleEQStateChangeStep_LoaderZUp() {}
};

class CFunctionModuleEQStateChangeStep_VacuumOn : public CFunctionModuleEQStateChangeStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleEQStateChangeStep_VacuumOn(CBaseSequenceImpl *pShareItem) : CFunctionModuleEQStateChangeStep(pShareItem) {}
	virtual ~CFunctionModuleEQStateChangeStep_VacuumOn() {}
};

class CFunctionModuleEQStateChangeStep_VacuumOff : public CFunctionModuleEQStateChangeStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleEQStateChangeStep_VacuumOff(CBaseSequenceImpl *pShareItem) : CFunctionModuleEQStateChangeStep(pShareItem) {}
	virtual ~CFunctionModuleEQStateChangeStep_VacuumOff() {}
};

class CFunctionModuleEQStateChangeStep_LoaderZDown : public CFunctionModuleEQStateChangeStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleEQStateChangeStep_LoaderZDown(CBaseSequenceImpl *pShareItem) : CFunctionModuleEQStateChangeStep(pShareItem) {}
	virtual ~CFunctionModuleEQStateChangeStep_LoaderZDown() {}
};

class CFunctionModuleEQStateChangeStep_LoaderZDown2Step : public CFunctionModuleEQStateChangeStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleEQStateChangeStep_LoaderZDown2Step(CBaseSequenceImpl *pShareItem) : CFunctionModuleEQStateChangeStep(pShareItem) {}
	virtual ~CFunctionModuleEQStateChangeStep_LoaderZDown2Step() {}
};

class CFunctionModuleEQStateChangeStep_ClampIn : public CFunctionModuleEQStateChangeStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleEQStateChangeStep_ClampIn(CBaseSequenceImpl *pShareItem) : CFunctionModuleEQStateChangeStep(pShareItem) {}
	virtual ~CFunctionModuleEQStateChangeStep_ClampIn() {}
};

class CFunctionModuleEQStateChangeStep_LoaderXYRLoad : public CFunctionModuleEQStateChangeStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleEQStateChangeStep_LoaderXYRLoad(CBaseSequenceImpl *pShareItem) : CFunctionModuleEQStateChangeStep(pShareItem) {}
	virtual ~CFunctionModuleEQStateChangeStep_LoaderXYRLoad() {}
};

class CFunctionModuleEQStateChangeStep_LPDown : public CFunctionModuleEQStateChangeStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleEQStateChangeStep_LPDown(CBaseSequenceImpl *pShareItem) : CFunctionModuleEQStateChangeStep(pShareItem) {}
	virtual ~CFunctionModuleEQStateChangeStep_LPDown() {}
};

class CFunctionModuleEQStateChangeStep_Safety : public CFunctionModuleEQStateChangeStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleEQStateChangeStep_Safety(CBaseSequenceImpl *pShareItem) : CFunctionModuleEQStateChangeStep(pShareItem) {}
	virtual ~CFunctionModuleEQStateChangeStep_Safety() {}
};

class CFunctionModuleEQStateChangeStep_Idle : public CFunctionModuleEQStateChangeStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleEQStateChangeStep_Idle(CBaseSequenceImpl *pShareItem) : CFunctionModuleEQStateChangeStep(pShareItem) {}
	virtual ~CFunctionModuleEQStateChangeStep_Idle() {}
};

class CFunctionModuleEQStateChangeStep_ClampUp : public CFunctionModuleEQStateChangeStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleEQStateChangeStep_ClampUp(CBaseSequenceImpl* pShareItem) : CFunctionModuleEQStateChangeStep(pShareItem) {}
	virtual ~CFunctionModuleEQStateChangeStep_ClampUp() {}
};

class CFunctionModuleEQStateChangeStep_ClampDown : public CFunctionModuleEQStateChangeStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleEQStateChangeStep_ClampDown(CBaseSequenceImpl* pShareItem) : CFunctionModuleEQStateChangeStep(pShareItem) {}
	virtual ~CFunctionModuleEQStateChangeStep_ClampDown() {}
};

class CFunctionModuleEQStateChangeStep_Ready : public CFunctionModuleEQStateChangeStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleEQStateChangeStep_Ready(CBaseSequenceImpl* pShareItem) : CFunctionModuleEQStateChangeStep(pShareItem) {}
	virtual ~CFunctionModuleEQStateChangeStep_Ready() {}
};

class CFunctionModuleEQStateChangeStep_Load : public CFunctionModuleEQStateChangeStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleEQStateChangeStep_Load(CBaseSequenceImpl* pShareItem) : CFunctionModuleEQStateChangeStep(pShareItem) {}
	virtual ~CFunctionModuleEQStateChangeStep_Load() {}
};

class CFunctionModuleEQStateChangeStep_Meas : public CFunctionModuleEQStateChangeStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleEQStateChangeStep_Meas(CBaseSequenceImpl* pShareItem) : CFunctionModuleEQStateChangeStep(pShareItem) {}
	virtual ~CFunctionModuleEQStateChangeStep_Meas() {}
};

class CFunctionModuleEQStateChangeStep_X_RAY_ON1 : public CFunctionModuleEQStateChangeStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleEQStateChangeStep_X_RAY_ON1(CBaseSequenceImpl* pShareItem) : CFunctionModuleEQStateChangeStep(pShareItem) {}
	virtual ~CFunctionModuleEQStateChangeStep_X_RAY_ON1() {}
};

class CFunctionModuleEQStateChangeStep_X_RAY_OFF1 : public CFunctionModuleEQStateChangeStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleEQStateChangeStep_X_RAY_OFF1(CBaseSequenceImpl* pShareItem) : CFunctionModuleEQStateChangeStep(pShareItem) {}
	virtual ~CFunctionModuleEQStateChangeStep_X_RAY_OFF1() {}
};

class CFunctionModuleEQStateChangeStep_X_RAY_ON2 : public CFunctionModuleEQStateChangeStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleEQStateChangeStep_X_RAY_ON2(CBaseSequenceImpl* pShareItem) : CFunctionModuleEQStateChangeStep(pShareItem) {}
	virtual ~CFunctionModuleEQStateChangeStep_X_RAY_ON2() {}
};

class CFunctionModuleEQStateChangeStep_X_RAY_OFF2 : public CFunctionModuleEQStateChangeStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleEQStateChangeStep_X_RAY_OFF2(CBaseSequenceImpl* pShareItem) : CFunctionModuleEQStateChangeStep(pShareItem) {}
	virtual ~CFunctionModuleEQStateChangeStep_X_RAY_OFF2() {}
};

class CFunctionModuleEQStateChangeStep_Ready2 : public CFunctionModuleEQStateChangeStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleEQStateChangeStep_Ready2(CBaseSequenceImpl* pShareItem) : CFunctionModuleEQStateChangeStep(pShareItem) {}
	virtual ~CFunctionModuleEQStateChangeStep_Ready2() {}
};

class CFunctionModuleEQStateChangeStep_FixtureUp : public CFunctionModuleEQStateChangeStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleEQStateChangeStep_FixtureUp(CBaseSequenceImpl* pShareItem) : CFunctionModuleEQStateChangeStep(pShareItem) {}
	virtual ~CFunctionModuleEQStateChangeStep_FixtureUp() {}
};

class CFunctionModuleEQStateChangeStep_FixtureDown : public CFunctionModuleEQStateChangeStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleEQStateChangeStep_FixtureDown(CBaseSequenceImpl* pShareItem) : CFunctionModuleEQStateChangeStep(pShareItem) {}
	virtual ~CFunctionModuleEQStateChangeStep_FixtureDown() {}
};

class CFunctionModuleEQStateChangeStep_LPUpPre : public CFunctionModuleEQStateChangeStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleEQStateChangeStep_LPUpPre(CBaseSequenceImpl* pShareItem) : CFunctionModuleEQStateChangeStep(pShareItem) {}
	virtual ~CFunctionModuleEQStateChangeStep_LPUpPre() {}
};

class CFunctionModuleEQStateChangeStep_FixtureUpBack : public CFunctionModuleEQStateChangeStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleEQStateChangeStep_FixtureUpBack(CBaseSequenceImpl* pShareItem) : CFunctionModuleEQStateChangeStep(pShareItem) {}
	virtual ~CFunctionModuleEQStateChangeStep_FixtureUpBack() {}
};