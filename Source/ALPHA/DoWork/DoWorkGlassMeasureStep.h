#pragma once

#include "..\Recipe\RecipeData.h"

struct stGlassMeasAreaInfo
{
	bool			bUse;
	int				nCurrentArea;
	int				nCurrentHead;
	stMeasAreaInfo	stGlassMeasInfo;

	stGlassMeasAreaInfo()
	{
		bUse = false;
		nCurrentArea = -1;
		nCurrentHead = -1;
	}
};

class CBaseSequenceImpl;
class CMeasureSequenceItemEngine;
class CDoWorkGlassMeasureStep
{
protected:
	CBaseSequenceImpl *m_pShareItem;
	CMeasureSequenceItemEngine *m_pMeasSeqItemEngine;
	CString m_strNmae;
public:
	virtual bool Run() = 0;

	// Stick
	void SetMeasureStick(int nMeasureStick);
	int GetMeasureStick();
	int m_nMeasureStick;

	//Area
	void SetMeasureArea(int nMeasureArea);
	int GetMeasureArea();
	int m_nMeasureArea;

	//시퀀스 상태
	//m_bMeasureLoop : false = 완료, true = 반복
	void SetMeasureLoop(bool bLoop);
	bool GetMeasureLoop();
	bool m_bMeasureLoop;
	int  MakeCellID(int* pnCellNum, bool* pbMeasProbe, TCHAR* ppcCellID, int iRotate, int iStick, int nMaxStick);	// jsyoon 2022.09.29 add
	void GenMeasID(int* pnCellNum, bool* pbMeasProbe, TCHAR* ppcCellID);
	void GenCellID(int* pnCellNum, bool* pbMeasProbe, TCHAR* ppcCellID);
	void GenInnerID(int* pnCellNum, bool* pbMeasProbe);
	

	void ResetGlassMeasAreaInfo();
	stGlassMeasAreaInfo stGlassMeasAreaInfo[2];

	CString GetName() { return m_strNmae; };
	void SetName(CString str) { m_strNmae = str; };

public:
	CDoWorkGlassMeasureStep(CBaseSequenceImpl* pShareItem, CMeasureSequenceItemEngine* pMeasSeqItemEngine) {
		m_pShareItem = pShareItem; m_pMeasSeqItemEngine = pMeasSeqItemEngine;  m_nMeasureArea = 0; m_bMeasureLoop = true, m_nMeasureStick = 0;
	}
	virtual ~CDoWorkGlassMeasureStep(){}
};

class CDoWorkGlassMeasureStep_None : public CDoWorkGlassMeasureStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassMeasureStep_None(CBaseSequenceImpl *pShareItem, CMeasureSequenceItemEngine *pMeasSeqItemEngine) : CDoWorkGlassMeasureStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T(""); }
	virtual ~CDoWorkGlassMeasureStep_None() {}
};

class CDoWorkGlassMeasureStep_CheckMeasureComplete : public CDoWorkGlassMeasureStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassMeasureStep_CheckMeasureComplete(CBaseSequenceImpl *pShareItem, CMeasureSequenceItemEngine *pMeasSeqItemEngine) : CDoWorkGlassMeasureStep(pShareItem, pMeasSeqItemEngine) {}
	virtual ~CDoWorkGlassMeasureStep_CheckMeasureComplete() {}
};

class CDoWorkGlassMeasureStep_SetMeasureArea : public CDoWorkGlassMeasureStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassMeasureStep_SetMeasureArea(CBaseSequenceImpl *pShareItem, CMeasureSequenceItemEngine *pMeasSeqItemEngine) : CDoWorkGlassMeasureStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T("MEAS IDLE"); }
	virtual ~CDoWorkGlassMeasureStep_SetMeasureArea() {}
};

class CDoWorkGlassMeasureStep_MoveMeasureArea : public CDoWorkGlassMeasureStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassMeasureStep_MoveMeasureArea(CBaseSequenceImpl *pShareItem, CMeasureSequenceItemEngine *pMeasSeqItemEngine) : CDoWorkGlassMeasureStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T("MEAS MOVE"); }
	virtual ~CDoWorkGlassMeasureStep_MoveMeasureArea() {}
};


class CDoWorkGlassMeasureStep_PtrnSearch : public CDoWorkGlassMeasureStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassMeasureStep_PtrnSearch(CBaseSequenceImpl* pShareItem, CMeasureSequenceItemEngine* pMeasSeqItemEngine) : CDoWorkGlassMeasureStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T("Pattern Search"); }
	virtual ~CDoWorkGlassMeasureStep_PtrnSearch() {}
};

class CDoWorkGlassMeasureStep_Autofocus : public CDoWorkGlassMeasureStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassMeasureStep_Autofocus(CBaseSequenceImpl *pShareItem, CMeasureSequenceItemEngine *pMeasSeqItemEngine) : CDoWorkGlassMeasureStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T("MEAS AF"); }
	virtual ~CDoWorkGlassMeasureStep_Autofocus() {}
};

class CDoWorkGlassMeasureStep_PatternCentering : public CDoWorkGlassMeasureStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassMeasureStep_PatternCentering(CBaseSequenceImpl *pShareItem, CMeasureSequenceItemEngine *pMeasSeqItemEngine) : CDoWorkGlassMeasureStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T("MEAS CENTERING"); }
	virtual ~CDoWorkGlassMeasureStep_PatternCentering() {}
};

class CDoWorkGlassMeasureStep_Measure : public CDoWorkGlassMeasureStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassMeasureStep_Measure(CBaseSequenceImpl *pShareItem, CMeasureSequenceItemEngine *pMeasSeqItemEngine) : CDoWorkGlassMeasureStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T("MEASURE"); }
	virtual ~CDoWorkGlassMeasureStep_Measure() {}
};

class CDoWorkGlassMeasureStep_Safety : public CDoWorkGlassMeasureStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassMeasureStep_Safety(CBaseSequenceImpl *pShareItem, CMeasureSequenceItemEngine *pMeasSeqItemEngine) : CDoWorkGlassMeasureStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T("MOVE SAFETY"); }
	virtual ~CDoWorkGlassMeasureStep_Safety() {}
};

class CDoWorkGlassMeasureStep_Idle : public CDoWorkGlassMeasureStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassMeasureStep_Idle(CBaseSequenceImpl *pShareItem, CMeasureSequenceItemEngine *pMeasSeqItemEngine) : CDoWorkGlassMeasureStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T(""); }
	virtual ~CDoWorkGlassMeasureStep_Idle() {}
};

// Contact
class CDoWorkGlassMeasureStep_Probe_Contact : public CDoWorkGlassMeasureStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassMeasureStep_Probe_Contact(CBaseSequenceImpl* pShareItem, CMeasureSequenceItemEngine* pMeasSeqItemEngine) : CDoWorkGlassMeasureStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T("Contact"); }
	virtual ~CDoWorkGlassMeasureStep_Probe_Contact() {}
};

// 특성
class CDoWorkGlassMeasureStep_Measure_WAD : public CDoWorkGlassMeasureStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassMeasureStep_Measure_WAD(CBaseSequenceImpl* pShareItem, CMeasureSequenceItemEngine* pMeasSeqItemEngine) : CDoWorkGlassMeasureStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T("WAD"); }
	virtual ~CDoWorkGlassMeasureStep_Measure_WAD() {}
};

// 화질
class CDoWorkGlassMeasureStep_Measure_VISION : public CDoWorkGlassMeasureStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassMeasureStep_Measure_VISION(CBaseSequenceImpl* pShareItem, CMeasureSequenceItemEngine* pMeasSeqItemEngine) : CDoWorkGlassMeasureStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T("VISION"); }
	virtual ~CDoWorkGlassMeasureStep_Measure_VISION() {}
};

// 색이상
class CDoWorkGlassMeasureStep_Measure_COLOR : public CDoWorkGlassMeasureStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassMeasureStep_Measure_COLOR(CBaseSequenceImpl* pShareItem, CMeasureSequenceItemEngine* pMeasSeqItemEngine) : CDoWorkGlassMeasureStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T("COLOR"); }
	virtual ~CDoWorkGlassMeasureStep_Measure_COLOR() {}
};

// 시야각 + LCR
class CDoWorkGlassMeasureStep_Measure_ANGLE : public CDoWorkGlassMeasureStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassMeasureStep_Measure_ANGLE(CBaseSequenceImpl* pShareItem, CMeasureSequenceItemEngine* pMeasSeqItemEngine) : CDoWorkGlassMeasureStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T("ANGLE"); }
	virtual ~CDoWorkGlassMeasureStep_Measure_ANGLE() {}
};

// TSP
class CDoWorkGlassMeasureStep_Measure_TSP : public CDoWorkGlassMeasureStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassMeasureStep_Measure_TSP(CBaseSequenceImpl* pShareItem, CMeasureSequenceItemEngine* pMeasSeqItemEngine) : CDoWorkGlassMeasureStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T("TSP"); }
	virtual ~CDoWorkGlassMeasureStep_Measure_TSP() {}
};

class CDoWorkGlassMeasureStep_Measure_VISION_COLOR : public CDoWorkGlassMeasureStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassMeasureStep_Measure_VISION_COLOR(CBaseSequenceImpl* pShareItem, CMeasureSequenceItemEngine* pMeasSeqItemEngine) : CDoWorkGlassMeasureStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T("화질+면계측"); }
	virtual ~CDoWorkGlassMeasureStep_Measure_VISION_COLOR() {}
};

// OST
class CDoWorkGlassMeasureStep_OST : public CDoWorkGlassMeasureStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassMeasureStep_OST(CBaseSequenceImpl* pShareItem, CMeasureSequenceItemEngine* pMeasSeqItemEngine) : CDoWorkGlassMeasureStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T("OSTest"); }
	virtual ~CDoWorkGlassMeasureStep_OST() {}
};

// R-Test
class CDoWorkGlassMeasureStep_RTEST : public CDoWorkGlassMeasureStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassMeasureStep_RTEST(CBaseSequenceImpl* pShareItem, CMeasureSequenceItemEngine* pMeasSeqItemEngine) : CDoWorkGlassMeasureStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T("R-Test"); }
	virtual ~CDoWorkGlassMeasureStep_RTEST() {}
};

// AGING
class CDoWorkGlassMeasureStep_AGING : public CDoWorkGlassMeasureStep
{
public:
	virtual bool Run() override;
public:
	CDoWorkGlassMeasureStep_AGING(CBaseSequenceImpl* pShareItem, CMeasureSequenceItemEngine* pMeasSeqItemEngine) : CDoWorkGlassMeasureStep(pShareItem, pMeasSeqItemEngine) { m_strNmae = _T("Aging"); }
	virtual ~CDoWorkGlassMeasureStep_AGING() {}
};