#pragma once

class CBaseSequenceImpl;
class CDoWorkGlassResultStep
{
protected:
	CBaseSequenceImpl *m_pShareItem;
	CString m_strNmae;
public:
	virtual bool Run() = 0;

	CString GetName() { return m_strNmae; };
	void SetName(CString str) { m_strNmae = str; };

public:
	CDoWorkGlassResultStep(CBaseSequenceImpl *pShareItem){ m_pShareItem = pShareItem; }
	virtual ~CDoWorkGlassResultStep(){}
};

class CDoWorkGlassResultStep_None : public CDoWorkGlassResultStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassResultStep_None(CBaseSequenceImpl *pShareItem) : CDoWorkGlassResultStep(pShareItem) { m_strNmae = _T(""); }
	virtual ~CDoWorkGlassResultStep_None() {}
};

class CDoWorkGlassResultStep_InitPosition : public CDoWorkGlassResultStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassResultStep_InitPosition(CBaseSequenceImpl *pShareItem) : CDoWorkGlassResultStep(pShareItem) { m_strNmae = _T(""); }
	virtual ~CDoWorkGlassResultStep_InitPosition() {}
};

class CDoWorkGlassResultStep_InitPosition2 : public CDoWorkGlassResultStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassResultStep_InitPosition2(CBaseSequenceImpl *pShareItem) : CDoWorkGlassResultStep(pShareItem) { m_strNmae = _T(""); }
	virtual ~CDoWorkGlassResultStep_InitPosition2() {}
};

class CDoWorkGlassResultStep_InitPositionCheck : public CDoWorkGlassResultStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassResultStep_InitPositionCheck(CBaseSequenceImpl *pShareItem) : CDoWorkGlassResultStep(pShareItem) { m_strNmae = _T(""); }
	virtual ~CDoWorkGlassResultStep_InitPositionCheck() {}
};

class CDoWorkGlassResultStep_Complete : public CDoWorkGlassResultStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassResultStep_Complete(CBaseSequenceImpl *pShareItem) : CDoWorkGlassResultStep(pShareItem) { m_strNmae = _T(""); }
	virtual ~CDoWorkGlassResultStep_Complete() {}
};

class CDoWorkGlassResultStep_CompleteCheck : public CDoWorkGlassResultStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassResultStep_CompleteCheck(CBaseSequenceImpl *pShareItem) : CDoWorkGlassResultStep(pShareItem) { m_strNmae = _T("Complete Check"); }
	virtual ~CDoWorkGlassResultStep_CompleteCheck() {}
};

class CDoWorkGlassResultStep_Exit : public CDoWorkGlassResultStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassResultStep_Exit(CBaseSequenceImpl *pShareItem) : CDoWorkGlassResultStep(pShareItem) { m_strNmae = _T(""); }
	virtual ~CDoWorkGlassResultStep_Exit() {}
};

class CDoWorkGlassResultStep_ExitCheck : public CDoWorkGlassResultStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassResultStep_ExitCheck(CBaseSequenceImpl *pShareItem) : CDoWorkGlassResultStep(pShareItem) { m_strNmae = _T(""); }
	virtual ~CDoWorkGlassResultStep_ExitCheck() {}
};

class CDoWorkGlassResultStep_Idle : public CDoWorkGlassResultStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassResultStep_Idle(CBaseSequenceImpl *pShareItem) : CDoWorkGlassResultStep(pShareItem) { m_strNmae = _T(""); }
	virtual ~CDoWorkGlassResultStep_Idle() {}
};
