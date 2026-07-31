#pragma once

class CBaseSequenceImpl;
class CMeasureSequenceItemEngine;
class CDoWorkGlassAGINGStep
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
	CDoWorkGlassAGINGStep(CBaseSequenceImpl* pShareItem, CMeasureSequenceItemEngine* pMeasSeqItemEngine) { m_pShareItem = pShareItem; m_pMeasSeqItemEngine = pMeasSeqItemEngine; }
	virtual ~CDoWorkGlassAGINGStep() {}
};


class CDoWorkGlassAGINGStep_None : public CDoWorkGlassAGINGStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassAGINGStep_None(CBaseSequenceImpl* pShareItem, CMeasureSequenceItemEngine* pMeasSeqItemEngine) : CDoWorkGlassAGINGStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T(""); }
	virtual ~CDoWorkGlassAGINGStep_None() {}
};

class CDoWorkGlassAGINGStep_Idle : public CDoWorkGlassAGINGStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassAGINGStep_Idle(CBaseSequenceImpl* pShareItem, CMeasureSequenceItemEngine* pMeasSeqItemEngine) : CDoWorkGlassAGINGStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T(""); }
	virtual ~CDoWorkGlassAGINGStep_Idle() {}
};

class CDoWorkGlassAGINGStep_Aging : public CDoWorkGlassAGINGStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassAGINGStep_Aging(CBaseSequenceImpl* pShareItem, CMeasureSequenceItemEngine* pMeasSeqItemEngine) : CDoWorkGlassAGINGStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T("AGING"); }
	virtual ~CDoWorkGlassAGINGStep_Aging() {}
};


class CDoWorkGlassAGINGStep_AgingDone : public CDoWorkGlassAGINGStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassAGINGStep_AgingDone(CBaseSequenceImpl* pShareItem, CMeasureSequenceItemEngine* pMeasSeqItemEngine) : CDoWorkGlassAGINGStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T("AGING"); }
	virtual ~CDoWorkGlassAGINGStep_AgingDone() {}
};

class CDoWorkGlassAGINGStep_Move_Second : public CDoWorkGlassAGINGStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassAGINGStep_Move_Second(CBaseSequenceImpl* pShareItem, CMeasureSequenceItemEngine* pMeasSeqItemEngine) : CDoWorkGlassAGINGStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T("MOVE2"); }
	virtual ~CDoWorkGlassAGINGStep_Move_Second() {}
};

class CDoWorkGlassAGINGStep_Aging_Second : public CDoWorkGlassAGINGStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassAGINGStep_Aging_Second(CBaseSequenceImpl* pShareItem, CMeasureSequenceItemEngine* pMeasSeqItemEngine) : CDoWorkGlassAGINGStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T("AGING2"); }
	virtual ~CDoWorkGlassAGINGStep_Aging_Second() {}
};

class CDoWorkGlassAGINGStep_Move_3 : public CDoWorkGlassAGINGStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassAGINGStep_Move_3(CBaseSequenceImpl* pShareItem, CMeasureSequenceItemEngine* pMeasSeqItemEngine) : CDoWorkGlassAGINGStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T("MOVE3"); }
	virtual ~CDoWorkGlassAGINGStep_Move_3() {}
};

class CDoWorkGlassAGINGStep_Aging_3 : public CDoWorkGlassAGINGStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassAGINGStep_Aging_3(CBaseSequenceImpl* pShareItem, CMeasureSequenceItemEngine* pMeasSeqItemEngine) : CDoWorkGlassAGINGStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T("AGING3"); }
	virtual ~CDoWorkGlassAGINGStep_Aging_3() {}
};

class CDoWorkGlassAGINGStep_Move_First : public CDoWorkGlassAGINGStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassAGINGStep_Move_First(CBaseSequenceImpl* pShareItem, CMeasureSequenceItemEngine* pMeasSeqItemEngine) : CDoWorkGlassAGINGStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T("MOVE2"); }
	virtual ~CDoWorkGlassAGINGStep_Move_First() {}
};