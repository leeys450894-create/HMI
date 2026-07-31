#pragma once

class CBaseSequenceImpl;
class CMeasureSequenceItemEngine;
class CDoWorkGlassPreMeasStep
{
protected:
	CBaseSequenceImpl* m_pShareItem;
	CMeasureSequenceItemEngine* m_pMeasSeqItemEngine;
	CString m_strNmae;
public:
	virtual bool Run() = 0;

	CString GetName() { return m_strNmae; };
	void SetName(CString str) { m_strNmae = str; };

public:
CDoWorkGlassPreMeasStep(CBaseSequenceImpl* pShareItem, CMeasureSequenceItemEngine* pMeasSeqItemEngine) { m_pShareItem = pShareItem; m_pMeasSeqItemEngine = pMeasSeqItemEngine;  }
	virtual ~CDoWorkGlassPreMeasStep() {}
};

class CDoWorkGlassPreMeasStep_None : public CDoWorkGlassPreMeasStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassPreMeasStep_None(CBaseSequenceImpl* pShareItem, CMeasureSequenceItemEngine* pMeasSeqItemEngine) : CDoWorkGlassPreMeasStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T(""); }
	virtual ~CDoWorkGlassPreMeasStep_None() {}
};

class CDoWorkGlassPreMeasStep_Idle : public CDoWorkGlassPreMeasStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassPreMeasStep_Idle(CBaseSequenceImpl* pShareItem, CMeasureSequenceItemEngine* pMeasSeqItemEngine) : CDoWorkGlassPreMeasStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T(""); }
	virtual ~CDoWorkGlassPreMeasStep_Idle() {}
};


class CDoWorkGlassPreMeasStep_Move : public CDoWorkGlassPreMeasStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassPreMeasStep_Move(CBaseSequenceImpl* pShareItem, CMeasureSequenceItemEngine* pMeasSeqItemEngine) : CDoWorkGlassPreMeasStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T("Move"); }
	virtual ~CDoWorkGlassPreMeasStep_Move() {}
};


// Pattern Search
class CDoWorkGlassPreMeasStep_PtrnSearch : public CDoWorkGlassPreMeasStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassPreMeasStep_PtrnSearch(CBaseSequenceImpl* pShareItem, CMeasureSequenceItemEngine* pMeasSeqItemEngine) : CDoWorkGlassPreMeasStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T("Pattern Search"); }
	virtual ~CDoWorkGlassPreMeasStep_PtrnSearch() {}
};

class CDoWorkGlassPreMeasStep_RcpChangeDone : public CDoWorkGlassPreMeasStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassPreMeasStep_RcpChangeDone(CBaseSequenceImpl* pShareItem, CMeasureSequenceItemEngine* pMeasSeqItemEngine) : CDoWorkGlassPreMeasStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T("Recipe Change Done"); }
	virtual ~CDoWorkGlassPreMeasStep_RcpChangeDone() {}
};



// Pin to Pin
class CDoWorkGlassPreMeasStep_P2P : public CDoWorkGlassPreMeasStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassPreMeasStep_P2P(CBaseSequenceImpl* pShareItem, CMeasureSequenceItemEngine* pMeasSeqItemEngine) : CDoWorkGlassPreMeasStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T("PToP Test"); }
	virtual ~CDoWorkGlassPreMeasStep_P2P() {}
};

class CDoWorkGlassPreMeasStep_OST : public CDoWorkGlassPreMeasStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassPreMeasStep_OST(CBaseSequenceImpl* pShareItem, CMeasureSequenceItemEngine* pMeasSeqItemEngine) : CDoWorkGlassPreMeasStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T("OSTest"); }
	virtual ~CDoWorkGlassPreMeasStep_OST() {}
};

class CDoWorkGlassPreMeasStep_RTEST : public CDoWorkGlassPreMeasStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassPreMeasStep_RTEST(CBaseSequenceImpl* pShareItem, CMeasureSequenceItemEngine* pMeasSeqItemEngine) : CDoWorkGlassPreMeasStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T("R-Test"); }
	virtual ~CDoWorkGlassPreMeasStep_RTEST() {}
};

// Probe Contact
class CDoWorkGlassPreMeasStep_Contact : public CDoWorkGlassPreMeasStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassPreMeasStep_Contact(CBaseSequenceImpl* pShareItem, CMeasureSequenceItemEngine* pMeasSeqItemEngine) : CDoWorkGlassPreMeasStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T("Probe Contact"); }
	virtual ~CDoWorkGlassPreMeasStep_Contact() {}
};