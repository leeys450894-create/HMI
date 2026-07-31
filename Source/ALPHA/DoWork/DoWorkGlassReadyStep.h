#pragma once

class CBaseSequenceImpl;
class CDoWorkGlassReadyStep
{
protected:
	CBaseSequenceImpl *m_pShareItem;
	CString m_strNmae;

public:
	virtual bool Run() = 0;

	CString GetName() { return m_strNmae; };

public:
	CDoWorkGlassReadyStep(CBaseSequenceImpl *pShareItem){ m_pShareItem = pShareItem; }
	virtual ~CDoWorkGlassReadyStep(){}
};

class CDoWorkGlassReadyStep_None : public CDoWorkGlassReadyStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassReadyStep_None(CBaseSequenceImpl *pShareItem) : CDoWorkGlassReadyStep(pShareItem) {}
	virtual ~CDoWorkGlassReadyStep_None() {}
};

class CDoWorkGlassReadyStep_ChangeLoadStatus : public CDoWorkGlassReadyStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassReadyStep_ChangeLoadStatus(CBaseSequenceImpl *pShareItem) : CDoWorkGlassReadyStep(pShareItem) {}
	virtual ~CDoWorkGlassReadyStep_ChangeLoadStatus() {}
};

class CDoWorkGlassReadyStep_CheckLoadStatus : public CDoWorkGlassReadyStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassReadyStep_CheckLoadStatus(CBaseSequenceImpl *pShareItem) : CDoWorkGlassReadyStep(pShareItem) {}
	virtual ~CDoWorkGlassReadyStep_CheckLoadStatus() {}
};

class CDoWorkGlassReadyStep_LoadGlassInfoFromCIM : public CDoWorkGlassReadyStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassReadyStep_LoadGlassInfoFromCIM(CBaseSequenceImpl *pShareItem) : CDoWorkGlassReadyStep(pShareItem) {}
	virtual ~CDoWorkGlassReadyStep_LoadGlassInfoFromCIM() {}
};

class CDoWorkGlassReadyStep_LoadRecipe : public CDoWorkGlassReadyStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassReadyStep_LoadRecipe(CBaseSequenceImpl *pShareItem) : CDoWorkGlassReadyStep(pShareItem) { m_strNmae = _T("RECIPE CHANGE"); }
	virtual ~CDoWorkGlassReadyStep_LoadRecipe() {}
};

class CDoWorkGlassReadyStep_Idle : public CDoWorkGlassReadyStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassReadyStep_Idle(CBaseSequenceImpl *pShareItem) : CDoWorkGlassReadyStep(pShareItem) {}
	virtual ~CDoWorkGlassReadyStep_Idle() {}
};
