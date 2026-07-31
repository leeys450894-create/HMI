#pragma once

class CBaseSequenceImpl;
class CDoWorkGlassUnloadStep
{
protected:
	CBaseSequenceImpl *m_pShareItem;
	CString m_strNmae;
public:
	virtual bool Run() = 0;

	CString GetName() { return m_strNmae; };
	void SetName(CString str) { m_strNmae = str; };

public:
	CDoWorkGlassUnloadStep(CBaseSequenceImpl *pShareItem){ m_pShareItem = pShareItem; }
	virtual ~CDoWorkGlassUnloadStep(){}
};

class CDoWorkGlassUnloadStep_None : public CDoWorkGlassUnloadStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassUnloadStep_None(CBaseSequenceImpl *pShareItem) : CDoWorkGlassUnloadStep(pShareItem) { m_strNmae = _T(""); }
	virtual ~CDoWorkGlassUnloadStep_None() {}
};

class CDoWorkGlassUnloadStep_GetPreAlignParam : public CDoWorkGlassUnloadStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassUnloadStep_GetPreAlignParam(CBaseSequenceImpl *pShareItem) : CDoWorkGlassUnloadStep(pShareItem) { m_strNmae = _T(""); }
	virtual ~CDoWorkGlassUnloadStep_GetPreAlignParam() {}
};

class CDoWorkGlassUnloadStep_Move_Safety : public CDoWorkGlassUnloadStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassUnloadStep_Move_Safety(CBaseSequenceImpl* pShareItem) : CDoWorkGlassUnloadStep(pShareItem) { m_strNmae = _T("Move Safety"); }
	virtual ~CDoWorkGlassUnloadStep_Move_Safety() {}
};

class CDoWorkGlassUnloadStep_ChangeUnloadStatus : public CDoWorkGlassUnloadStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassUnloadStep_ChangeUnloadStatus(CBaseSequenceImpl *pShareItem) : CDoWorkGlassUnloadStep(pShareItem) { m_strNmae = _T("Change Unload Stauts"); }
	virtual ~CDoWorkGlassUnloadStep_ChangeUnloadStatus() {}
};

class CDoWorkGlassUnloadStep_CheckUnloadStatus : public CDoWorkGlassUnloadStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassUnloadStep_CheckUnloadStatus(CBaseSequenceImpl *pShareItem) : CDoWorkGlassUnloadStep(pShareItem) { m_strNmae = _T("Check Unload Stauts"); }
	virtual ~CDoWorkGlassUnloadStep_CheckUnloadStatus() {}
};

class CDoWorkGlassUnloadStep_CheckSaveReuslt : public CDoWorkGlassUnloadStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassUnloadStep_CheckSaveReuslt(CBaseSequenceImpl* pShareItem) : CDoWorkGlassUnloadStep(pShareItem) { m_strNmae = _T("Check Save Result"); }
	virtual ~CDoWorkGlassUnloadStep_CheckSaveReuslt() {}
};

class CDoWorkGlassUnloadStep_SendToCIM : public CDoWorkGlassUnloadStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassUnloadStep_SendToCIM(CBaseSequenceImpl* pShareItem) : CDoWorkGlassUnloadStep(pShareItem) { m_strNmae = _T("Unload to CIM"); }
	virtual ~CDoWorkGlassUnloadStep_SendToCIM() {}
};

class CDoWorkGlassUnloadStep_Idle : public CDoWorkGlassUnloadStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassUnloadStep_Idle(CBaseSequenceImpl *pShareItem) : CDoWorkGlassUnloadStep(pShareItem) { m_strNmae = _T(""); }
	virtual ~CDoWorkGlassUnloadStep_Idle() {}
};
