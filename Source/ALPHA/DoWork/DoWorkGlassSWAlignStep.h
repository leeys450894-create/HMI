#pragma once

typedef struct tagAlignPos
{
	double dX;
	double dY;
	double dTheta;
} tagAlignPos;

class CBaseSequenceImpl;
class CDoWorkGlassSWAlignStep
{
protected:
	CBaseSequenceImpl *m_pShareItem;
	CString m_strNmae;

public:
	virtual bool Run() = 0;

	int MoveAlignPosition(int nHead, int nIndex);
	int AlignAF(int nHead);
	int CenteringAlignPattern(int nHead);
	int AlignGlass();
	int AlignGlassSearch();
	int AlignGlassTZero(double* pdMovR);
	int AlignGlassXYZero(double* pdMovX, double* pdMovY);
	int AlignGlassMoveShift(double* pdMovOrg, int nFailCnt, double dTheta);
	CString GetName() { return m_strNmae; };
	void SetName(CString str) { m_strNmae = str; };
	bool GetComplete();

public:
	CDoWorkGlassSWAlignStep(CBaseSequenceImpl *pShareItem){ m_pShareItem = pShareItem; }
	virtual ~CDoWorkGlassSWAlignStep(){}


};

class CDoWorkGlassSWAlignStep_None : public CDoWorkGlassSWAlignStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassSWAlignStep_None(CBaseSequenceImpl *pShareItem) : CDoWorkGlassSWAlignStep(pShareItem) { m_strNmae = _T(""); }
	virtual ~CDoWorkGlassSWAlignStep_None() {}
};

class CDoWorkGlassSWAlignStep_CheckSWAlignParam : public CDoWorkGlassSWAlignStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassSWAlignStep_CheckSWAlignParam(CBaseSequenceImpl *pShareItem) : CDoWorkGlassSWAlignStep(pShareItem) {}
	virtual ~CDoWorkGlassSWAlignStep_CheckSWAlignParam() {}
};

class CDoWorkGlassSWAlignStep_MoveAlignPosistion_1st : public CDoWorkGlassSWAlignStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassSWAlignStep_MoveAlignPosistion_1st(CBaseSequenceImpl *pShareItem) : CDoWorkGlassSWAlignStep(pShareItem) {}
	virtual ~CDoWorkGlassSWAlignStep_MoveAlignPosistion_1st() {}
};

class CDoWorkGlassSWAlignStep_AF_1st : public CDoWorkGlassSWAlignStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassSWAlignStep_AF_1st(CBaseSequenceImpl *pShareItem) : CDoWorkGlassSWAlignStep(pShareItem) {}
	virtual ~CDoWorkGlassSWAlignStep_AF_1st() {}
};

class CDoWorkGlassSWAlignStep_CenteringAlignPattern_1st : public CDoWorkGlassSWAlignStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassSWAlignStep_CenteringAlignPattern_1st(CBaseSequenceImpl *pShareItem) : CDoWorkGlassSWAlignStep(pShareItem) {}
	virtual ~CDoWorkGlassSWAlignStep_CenteringAlignPattern_1st() {}
};

class CDoWorkGlassSWAlignStep_MoveAlignPosistion_2nd : public CDoWorkGlassSWAlignStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassSWAlignStep_MoveAlignPosistion_2nd(CBaseSequenceImpl *pShareItem) : CDoWorkGlassSWAlignStep(pShareItem) {}
	virtual ~CDoWorkGlassSWAlignStep_MoveAlignPosistion_2nd() {}
};

class CDoWorkGlassSWAlignStep_AF_2nd : public CDoWorkGlassSWAlignStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassSWAlignStep_AF_2nd(CBaseSequenceImpl *pShareItem) : CDoWorkGlassSWAlignStep(pShareItem) {}
	virtual ~CDoWorkGlassSWAlignStep_AF_2nd() {}
};

class CDoWorkGlassSWAlignStep_CenteringAlignPattern_2nd : public CDoWorkGlassSWAlignStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassSWAlignStep_CenteringAlignPattern_2nd(CBaseSequenceImpl *pShareItem) : CDoWorkGlassSWAlignStep(pShareItem) {}
	virtual ~CDoWorkGlassSWAlignStep_CenteringAlignPattern_2nd() {}
};

class CDoWorkGlassSWAlignStep_CalcAlignOffset : public CDoWorkGlassSWAlignStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassSWAlignStep_CalcAlignOffset(CBaseSequenceImpl *pShareItem) : CDoWorkGlassSWAlignStep(pShareItem) {}
	virtual ~CDoWorkGlassSWAlignStep_CalcAlignOffset() {}
};

class CDoWorkGlassSWAlignStep_Idle : public CDoWorkGlassSWAlignStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassSWAlignStep_Idle(CBaseSequenceImpl *pShareItem) : CDoWorkGlassSWAlignStep(pShareItem) {}
	virtual ~CDoWorkGlassSWAlignStep_Idle() {}
};

class CDoWorkGlassSWAlignStep_MoveAlignPosistion : public CDoWorkGlassSWAlignStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassSWAlignStep_MoveAlignPosistion(CBaseSequenceImpl *pShareItem) : CDoWorkGlassSWAlignStep(pShareItem) { m_strNmae = _T("GLASS ALIGN MOVE"); }
	virtual ~CDoWorkGlassSWAlignStep_MoveAlignPosistion() {}
};

class CDoWorkGlassSWAlignStep_AlignGlass : public CDoWorkGlassSWAlignStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassSWAlignStep_AlignGlass(CBaseSequenceImpl *pShareItem) : CDoWorkGlassSWAlignStep(pShareItem) { m_strNmae = _T("GLASS ALIGN"); }
	virtual ~CDoWorkGlassSWAlignStep_AlignGlass() {}
};
