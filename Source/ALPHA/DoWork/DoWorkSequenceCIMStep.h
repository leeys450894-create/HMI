#pragma once

class CBaseSequenceImpl;
class CShareMemoryMgr;
class CDoWorkSequenceCIMStep
{
protected:
	CBaseSequenceImpl* m_pShareItem;
	CShareMemoryMgr* m_pShareMemory;
	CString m_strNmae;

public:
	virtual bool Run() = 0;

	CString GetName() { return m_strNmae; };
	void SetName(CString str) { m_strNmae = str; };
	bool GetComplete();

public:
	CDoWorkSequenceCIMStep(CBaseSequenceImpl* pShareItem) { m_pShareItem = pShareItem; }
	virtual ~CDoWorkSequenceCIMStep() {}

};

class CDoWorkSequenceCIMStep_None : public CDoWorkSequenceCIMStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkSequenceCIMStep_None(CBaseSequenceImpl* pShareItem) : CDoWorkSequenceCIMStep(pShareItem) { m_strNmae = _T(""); }
	virtual ~CDoWorkSequenceCIMStep_None() {}
};

class CDoWorkSequenceCIMStep_Idle : public CDoWorkSequenceCIMStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkSequenceCIMStep_Idle(CBaseSequenceImpl* pShareItem) : CDoWorkSequenceCIMStep(pShareItem) {}
	virtual ~CDoWorkSequenceCIMStep_Idle() {}
};


class CDoWorkSequenceCIMStep_Initial : public CDoWorkSequenceCIMStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkSequenceCIMStep_Initial(CBaseSequenceImpl* pShareItem) : CDoWorkSequenceCIMStep(pShareItem) {}
	virtual ~CDoWorkSequenceCIMStep_Initial() {}
};

class CDoWorkSequenceCIMStep_Initial2 : public CDoWorkSequenceCIMStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkSequenceCIMStep_Initial2(CBaseSequenceImpl* pShareItem) : CDoWorkSequenceCIMStep(pShareItem) {}
	virtual ~CDoWorkSequenceCIMStep_Initial2() {}
};

class CDoWorkSequenceCIMStep_Loading : public CDoWorkSequenceCIMStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkSequenceCIMStep_Loading(CBaseSequenceImpl* pShareItem) : CDoWorkSequenceCIMStep(pShareItem) {}
	virtual ~CDoWorkSequenceCIMStep_Loading() {}
};

class CDoWorkSequenceCIMStep_CheckLoadReq : public CDoWorkSequenceCIMStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkSequenceCIMStep_CheckLoadReq(CBaseSequenceImpl* pShareItem) : CDoWorkSequenceCIMStep(pShareItem) {}
	virtual ~CDoWorkSequenceCIMStep_CheckLoadReq() {}
};

class CDoWorkSequenceCIMStep_ChangeLoad : public CDoWorkSequenceCIMStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkSequenceCIMStep_ChangeLoad(CBaseSequenceImpl* pShareItem) : CDoWorkSequenceCIMStep(pShareItem) {}
	virtual ~CDoWorkSequenceCIMStep_ChangeLoad() {}
};

class CDoWorkSequenceCIMStep_LoadReq : public CDoWorkSequenceCIMStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkSequenceCIMStep_LoadReq(CBaseSequenceImpl* pShareItem) : CDoWorkSequenceCIMStep(pShareItem) {}
	virtual ~CDoWorkSequenceCIMStep_LoadReq() {}
};


class CDoWorkSequenceCIMStep_PPIDREQ : public CDoWorkSequenceCIMStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkSequenceCIMStep_PPIDREQ(CBaseSequenceImpl* pShareItem) : CDoWorkSequenceCIMStep(pShareItem) {}
	virtual ~CDoWorkSequenceCIMStep_PPIDREQ() {}
};

class CDoWorkSequenceCIMStep_LOTIDREQ : public CDoWorkSequenceCIMStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkSequenceCIMStep_LOTIDREQ(CBaseSequenceImpl* pShareItem) : CDoWorkSequenceCIMStep(pShareItem) {}
	virtual ~CDoWorkSequenceCIMStep_LOTIDREQ() {}
};

class CDoWorkSequenceCIMStep_GLSIDREQ : public CDoWorkSequenceCIMStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkSequenceCIMStep_GLSIDREQ(CBaseSequenceImpl* pShareItem) : CDoWorkSequenceCIMStep(pShareItem) {}
	virtual ~CDoWorkSequenceCIMStep_GLSIDREQ() {}
};

class CDoWorkSequenceCIMStep_PRCSReq : public CDoWorkSequenceCIMStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkSequenceCIMStep_PRCSReq(CBaseSequenceImpl* pShareItem) : CDoWorkSequenceCIMStep(pShareItem) {}
	virtual ~CDoWorkSequenceCIMStep_PRCSReq() {}
};

class CDoWorkSequenceCIMStep_ChangeReady : public CDoWorkSequenceCIMStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkSequenceCIMStep_ChangeReady(CBaseSequenceImpl* pShareItem) : CDoWorkSequenceCIMStep(pShareItem) {}
	virtual ~CDoWorkSequenceCIMStep_ChangeReady() {}
};

class CDoWorkSequenceCIMStep_ChangeMeas : public CDoWorkSequenceCIMStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkSequenceCIMStep_ChangeMeas(CBaseSequenceImpl* pShareItem) : CDoWorkSequenceCIMStep(pShareItem) {}
	virtual ~CDoWorkSequenceCIMStep_ChangeMeas() {}
};

class CDoWorkSequenceCIMStep_MeasStart : public CDoWorkSequenceCIMStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkSequenceCIMStep_MeasStart(CBaseSequenceImpl* pShareItem) : CDoWorkSequenceCIMStep(pShareItem) {}
	virtual ~CDoWorkSequenceCIMStep_MeasStart() {}
};


class CDoWorkSequenceCIMStep_MeasDone : public CDoWorkSequenceCIMStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkSequenceCIMStep_MeasDone(CBaseSequenceImpl* pShareItem) : CDoWorkSequenceCIMStep(pShareItem) {}
	virtual ~CDoWorkSequenceCIMStep_MeasDone() {}
};

class CDoWorkSequenceCIMStep_GDTReq : public CDoWorkSequenceCIMStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkSequenceCIMStep_GDTReq(CBaseSequenceImpl* pShareItem) : CDoWorkSequenceCIMStep(pShareItem) {}
	virtual ~CDoWorkSequenceCIMStep_GDTReq() {}
};

class CDoWorkSequenceCIMStep_CheckUnLoadReq : public CDoWorkSequenceCIMStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkSequenceCIMStep_CheckUnLoadReq(CBaseSequenceImpl* pShareItem) : CDoWorkSequenceCIMStep(pShareItem) {}
	virtual ~CDoWorkSequenceCIMStep_CheckUnLoadReq() {}
};

class CDoWorkSequenceCIMStep_UnLoadDone : public CDoWorkSequenceCIMStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkSequenceCIMStep_UnLoadDone(CBaseSequenceImpl* pShareItem) : CDoWorkSequenceCIMStep(pShareItem) {}
	virtual ~CDoWorkSequenceCIMStep_UnLoadDone() {}
};

class CDoWorkSequenceCIMStep_CheckUnLoadCpl : public CDoWorkSequenceCIMStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkSequenceCIMStep_CheckUnLoadCpl(CBaseSequenceImpl* pShareItem) : CDoWorkSequenceCIMStep(pShareItem) {}
	virtual ~CDoWorkSequenceCIMStep_CheckUnLoadCpl() {}
};

class CDoWorkSequenceCIMStep_LoadReady : public CDoWorkSequenceCIMStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkSequenceCIMStep_LoadReady(CBaseSequenceImpl* pShareItem) : CDoWorkSequenceCIMStep(pShareItem) {}
	virtual ~CDoWorkSequenceCIMStep_LoadReady() {}
};