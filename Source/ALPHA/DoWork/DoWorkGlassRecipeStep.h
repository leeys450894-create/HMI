#pragma once

class CBaseSequenceImpl;
class CDoWorkGlassRecipeStep
{
protected:
	CBaseSequenceImpl* m_pShareItem;
	CString m_strNmae;

public:
	virtual bool Run() = 0;

	CString GetName() { return m_strNmae; };
	void SetName(CString str) { m_strNmae = str; };
	bool GetComplete();

public:
	CDoWorkGlassRecipeStep(CBaseSequenceImpl* pShareItem) { m_pShareItem = pShareItem; }
	virtual ~CDoWorkGlassRecipeStep() {}


};

class CDoWorkGlassRecipeStep_None : public CDoWorkGlassRecipeStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassRecipeStep_None(CBaseSequenceImpl* pShareItem) : CDoWorkGlassRecipeStep(pShareItem) { m_strNmae = _T(""); }
	virtual ~CDoWorkGlassRecipeStep_None() {}
};

class CDoWorkGlassRecipeStep_Idle : public CDoWorkGlassRecipeStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassRecipeStep_Idle(CBaseSequenceImpl* pShareItem) : CDoWorkGlassRecipeStep(pShareItem) {}
	virtual ~CDoWorkGlassRecipeStep_Idle() {}
};

class CDoWorkGlassRecipeStep_Change : public CDoWorkGlassRecipeStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassRecipeStep_Change(CBaseSequenceImpl* pShareItem) : CDoWorkGlassRecipeStep(pShareItem) { m_strNmae = _T("Recipe Change"); }
	virtual ~CDoWorkGlassRecipeStep_Change() {}
};

class CDoWorkGlassRecipeStep_ProbeCheck : public CDoWorkGlassRecipeStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassRecipeStep_ProbeCheck(CBaseSequenceImpl* pShareItem) : CDoWorkGlassRecipeStep(pShareItem) { m_strNmae = _T("Probe Change"); }
	virtual ~CDoWorkGlassRecipeStep_ProbeCheck() {}
};