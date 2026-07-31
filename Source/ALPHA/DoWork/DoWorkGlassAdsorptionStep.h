#pragma once

class CBaseSequenceImpl;
class CDoWorkGlassAdsorptionStep
{
protected:
	CBaseSequenceImpl *m_pShareItem;
	CString m_strNmae;
public:
	virtual bool Run() = 0;

	CString GetName() { return m_strNmae; };
	void SetName(CString str) { m_strNmae = str; };

public:
	CDoWorkGlassAdsorptionStep(CBaseSequenceImpl *pShareItem){ m_pShareItem = pShareItem; }
	virtual ~CDoWorkGlassAdsorptionStep(){}
};

class CDoWorkGlassAdsorptionStep_None : public CDoWorkGlassAdsorptionStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassAdsorptionStep_None(CBaseSequenceImpl *pShareItem) : CDoWorkGlassAdsorptionStep(pShareItem) {}
	virtual ~CDoWorkGlassAdsorptionStep_None() {}
};

class CDoWorkGlassAdsorptionStep_GetPreAlignParam : public CDoWorkGlassAdsorptionStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassAdsorptionStep_GetPreAlignParam(CBaseSequenceImpl *pShareItem) : CDoWorkGlassAdsorptionStep(pShareItem) { m_strNmae = _T(""); }
	virtual ~CDoWorkGlassAdsorptionStep_GetPreAlignParam() {}
};


class CDoWorkGlassAdsorptionStep_Move : public CDoWorkGlassAdsorptionStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassAdsorptionStep_Move(CBaseSequenceImpl* pShareItem) : CDoWorkGlassAdsorptionStep(pShareItem) { m_strNmae = _T("Move Measure Pos"); }
	virtual ~CDoWorkGlassAdsorptionStep_Move() {}
};

class CDoWorkGlassAdsorptionStep_ChangeMeasStatus : public CDoWorkGlassAdsorptionStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassAdsorptionStep_ChangeMeasStatus(CBaseSequenceImpl *pShareItem) : CDoWorkGlassAdsorptionStep(pShareItem) { m_strNmae = _T("Change Measure Status"); }
	virtual ~CDoWorkGlassAdsorptionStep_ChangeMeasStatus() {}
};

class CDoWorkGlassAdsorptionStep_CheckMeasStatus : public CDoWorkGlassAdsorptionStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassAdsorptionStep_CheckMeasStatus(CBaseSequenceImpl *pShareItem) : CDoWorkGlassAdsorptionStep(pShareItem) { m_strNmae = _T("Check Measure Status"); }
	virtual ~CDoWorkGlassAdsorptionStep_CheckMeasStatus() {}
};

class CDoWorkGlassAdsorptionStep_Idle : public CDoWorkGlassAdsorptionStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassAdsorptionStep_Idle(CBaseSequenceImpl *pShareItem) : CDoWorkGlassAdsorptionStep(pShareItem) { m_strNmae = _T(""); }
	virtual ~CDoWorkGlassAdsorptionStep_Idle() {}
};

class CDoWorkGlassAdsorptionStep_ProbeAlign : public CDoWorkGlassAdsorptionStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassAdsorptionStep_ProbeAlign(CBaseSequenceImpl* pShareItem) : CDoWorkGlassAdsorptionStep(pShareItem) { m_strNmae = _T("Probe Align Check"); }
	virtual ~CDoWorkGlassAdsorptionStep_ProbeAlign() {}
};