#pragma once

class CBaseSequenceImpl;
class CSystemInitializeStep
{
protected:
	CString m_strName;
	CBaseSequenceImpl *m_pShareItem;
public:
	virtual bool Run() = 0;

public:
	CSystemInitializeStep(CBaseSequenceImpl *pShareItem){ m_pShareItem = pShareItem; }
	virtual ~CSystemInitializeStep(){}
	int MotionHome(int iStep);
};

class CSystemInitializeStep_None : public CSystemInitializeStep
{
public:
	virtual bool Run() override;
public:
	CSystemInitializeStep_None(CBaseSequenceImpl *pShareItem) : CSystemInitializeStep(pShareItem) {}
	virtual ~CSystemInitializeStep_None() {}
};

class CSystemInitializeStep_CheckInterlock : public CSystemInitializeStep
{
public:
	virtual bool Run() override;
public:
	CSystemInitializeStep_CheckInterlock(CBaseSequenceImpl* pShareItem) : CSystemInitializeStep(pShareItem) {}
	virtual ~CSystemInitializeStep_CheckInterlock() {}
};

class CSystemInitializeStep_InitIO : public CSystemInitializeStep
{
public:
	virtual bool Run() override;
public:
	CSystemInitializeStep_InitIO(CBaseSequenceImpl *pShareItem) : CSystemInitializeStep(pShareItem) {}
	virtual ~CSystemInitializeStep_InitIO() {}

private:
	int ResetIO();
};

class CSystemInitializeStep_InitMotion : public CSystemInitializeStep
{
public:
	virtual bool Run() override;
public:
	CSystemInitializeStep_InitMotion(CBaseSequenceImpl *pShareItem) : CSystemInitializeStep(pShareItem) {}
	virtual ~CSystemInitializeStep_InitMotion() {}

private:
	int InitDevice();
	int MotionHome(int iAxis/* = AXIS_ALL*/);
};

class CSystemInitializeStep_InitModule : public CSystemInitializeStep
{
public:
	virtual bool Run() override;
public:
	CSystemInitializeStep_InitModule(CBaseSequenceImpl *pShareItem) : CSystemInitializeStep(pShareItem) {}
	virtual ~CSystemInitializeStep_InitModule() {}
};

class CSystemInitializeStep_InitComplete : public CSystemInitializeStep
{
public:
	virtual bool Run() override;
public:
	CSystemInitializeStep_InitComplete(CBaseSequenceImpl *pShareItem) : CSystemInitializeStep(pShareItem) {}
	virtual ~CSystemInitializeStep_InitComplete() {}
};

class CSystemInitializeStep_Idle : public CSystemInitializeStep
{
public:
	virtual bool Run() override;
public:
	CSystemInitializeStep_Idle(CBaseSequenceImpl *pShareItem) : CSystemInitializeStep(pShareItem) {}
	virtual ~CSystemInitializeStep_Idle() {}
};

// Z รเ
class CSystemInitializeStep_MotionHoming1 : public CSystemInitializeStep
{
public:
	virtual bool Run() override;
public:
	CSystemInitializeStep_MotionHoming1(CBaseSequenceImpl* pShareItem) : CSystemInitializeStep(pShareItem) {}
	virtual ~CSystemInitializeStep_MotionHoming1() {}
};

// cross
class CSystemInitializeStep_MotionHoming2 : public CSystemInitializeStep
{
public:
	virtual bool Run() override;
public:
	CSystemInitializeStep_MotionHoming2(CBaseSequenceImpl* pShareItem) : CSystemInitializeStep(pShareItem) {}
	virtual ~CSystemInitializeStep_MotionHoming2() {}
};

// Probe
class CSystemInitializeStep_MotionHoming3 : public CSystemInitializeStep
{
public:
	virtual bool Run() override;
public:
	CSystemInitializeStep_MotionHoming3(CBaseSequenceImpl* pShareItem) : CSystemInitializeStep(pShareItem) {}
	virtual ~CSystemInitializeStep_MotionHoming3() {}
};

// Stage
class CSystemInitializeStep_MotionHoming4 : public CSystemInitializeStep
{
public:
	virtual bool Run() override;
public:
	CSystemInitializeStep_MotionHoming4(CBaseSequenceImpl* pShareItem) : CSystemInitializeStep(pShareItem) {}
	virtual ~CSystemInitializeStep_MotionHoming4() {}
};

// AC
class CSystemInitializeStep_MotionHoming5 : public CSystemInitializeStep
{
public:
	virtual bool Run() override;
public:
	CSystemInitializeStep_MotionHoming5(CBaseSequenceImpl* pShareItem) : CSystemInitializeStep(pShareItem) {}
	virtual ~CSystemInitializeStep_MotionHoming5() {}
};

class CSystemInitializeStep_MotionHoming6 : public CSystemInitializeStep
{
public:
	virtual bool Run() override;
public:
	CSystemInitializeStep_MotionHoming6(CBaseSequenceImpl* pShareItem) : CSystemInitializeStep(pShareItem) {}
	virtual ~CSystemInitializeStep_MotionHoming6() {}
};

class CSystemInitializeStep_MotionHoming7 : public CSystemInitializeStep
{
public:
	virtual bool Run() override;
public:
	CSystemInitializeStep_MotionHoming7(CBaseSequenceImpl* pShareItem) : CSystemInitializeStep(pShareItem) {}
	virtual ~CSystemInitializeStep_MotionHoming7() {}
};

class CSystemInitializeStep_MotionHoming8 : public CSystemInitializeStep
{
public:
	virtual bool Run() override;
public:
	CSystemInitializeStep_MotionHoming8(CBaseSequenceImpl* pShareItem) : CSystemInitializeStep(pShareItem) {}
	virtual ~CSystemInitializeStep_MotionHoming8() {}
};

class CSystemInitializeStep_MotionHoming9 : public CSystemInitializeStep
{
public:
	virtual bool Run() override;
public:
	CSystemInitializeStep_MotionHoming9(CBaseSequenceImpl* pShareItem) : CSystemInitializeStep(pShareItem) {}
	virtual ~CSystemInitializeStep_MotionHoming9() {}
};

class CSystemInitializeStep_MotionHoming10 : public CSystemInitializeStep
{
public:
	virtual bool Run() override;
public:
	CSystemInitializeStep_MotionHoming10(CBaseSequenceImpl* pShareItem) : CSystemInitializeStep(pShareItem) {}
	virtual ~CSystemInitializeStep_MotionHoming10() {}
};

class CSystemInitializeStep_MotionHoming11 : public CSystemInitializeStep
{
public:
	virtual bool Run() override;
public:
	CSystemInitializeStep_MotionHoming11(CBaseSequenceImpl* pShareItem) : CSystemInitializeStep(pShareItem) {}
	virtual ~CSystemInitializeStep_MotionHoming11() {}
};

class CSystemInitializeStep_MotionHoming12 : public CSystemInitializeStep
{
public:
	virtual bool Run() override;
public:
	CSystemInitializeStep_MotionHoming12(CBaseSequenceImpl* pShareItem) : CSystemInitializeStep(pShareItem) {}
	virtual ~CSystemInitializeStep_MotionHoming12() {}
};

class CSystemInitializeStep_MotionComplete : public CSystemInitializeStep
{
public:
	virtual bool Run() override;
public:
	CSystemInitializeStep_MotionComplete(CBaseSequenceImpl* pShareItem) : CSystemInitializeStep(pShareItem) {}
	virtual ~CSystemInitializeStep_MotionComplete() {}
};


class CSystemInitializeStep_Safety : public CSystemInitializeStep
{
public:
	virtual bool Run() override;
public:
	CSystemInitializeStep_Safety(CBaseSequenceImpl* pShareItem) : CSystemInitializeStep(pShareItem) {}
	virtual ~CSystemInitializeStep_Safety() {}
};

class CSystemInitializeStep_ProbeAlign : public CSystemInitializeStep
{
public:
	virtual bool Run() override;
public:
	CSystemInitializeStep_ProbeAlign(CBaseSequenceImpl* pShareItem) : CSystemInitializeStep(pShareItem) {}
	virtual ~CSystemInitializeStep_ProbeAlign() {}
};
