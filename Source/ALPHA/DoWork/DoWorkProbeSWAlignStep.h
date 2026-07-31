#pragma once

class CBaseSequenceImpl;
class CDoWorkProbeSWAlignStep
{
protected:
	CBaseSequenceImpl* m_pShareItem;
	CString m_strNmae;

public:
	virtual bool Run() = 0;

	CString GetName() { return m_strNmae; };
	void SetName(CString str) { m_strNmae = str; };
	bool GetComplete();

	int Align_MovePosition(int nHead, int nIndex);
	int Align_Probe();
	int Align_ProbeSearch(int iProbe);
	int Align_ProbeTZero(int iProbe, double* pdMovR);
	int Align_ProbeXYZero(int iProbe, double* pdMovX, double* pdMovY);

public:
	CDoWorkProbeSWAlignStep(CBaseSequenceImpl* pShareItem) { m_pShareItem = pShareItem; }
	virtual ~CDoWorkProbeSWAlignStep() {}

};

class CDoWorkProbeSWAlignStep_None : public CDoWorkProbeSWAlignStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkProbeSWAlignStep_None(CBaseSequenceImpl* pShareItem) : CDoWorkProbeSWAlignStep(pShareItem) { m_strNmae = _T(""); }
	virtual ~CDoWorkProbeSWAlignStep_None() {}
};

class CDoWorkProbeSWAlignStep_Idle : public CDoWorkProbeSWAlignStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkProbeSWAlignStep_Idle(CBaseSequenceImpl* pShareItem) : CDoWorkProbeSWAlignStep(pShareItem) {}
	virtual ~CDoWorkProbeSWAlignStep_Idle() {}
};

class CDoWorkProbeSWAlignStep_Move : public CDoWorkProbeSWAlignStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkProbeSWAlignStep_Move(CBaseSequenceImpl* pShareItem) : CDoWorkProbeSWAlignStep(pShareItem) { m_strNmae = _T("PROBE ALIGN MOVE"); }
	virtual ~CDoWorkProbeSWAlignStep_Move() {}
};

class CDoWorkProbeSWAlignStep_Align : public CDoWorkProbeSWAlignStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkProbeSWAlignStep_Align(CBaseSequenceImpl* pShareItem) : CDoWorkProbeSWAlignStep(pShareItem) { m_strNmae = _T("PROBE ALIGN"); }
	virtual ~CDoWorkProbeSWAlignStep_Align() {}
};