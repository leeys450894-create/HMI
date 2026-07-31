#pragma once

#include <nivision.h>

class CPtrnInfo : public CObject  
{
public:
	CPtrnInfo();
	virtual ~CPtrnInfo();
	
public:
	virtual void	Serialize(CArchive& ar, int iVer = 0);
	//=========================================================
	// not save : m_pImgOrg, m_pImgScaled, m_dRealX, m_dRealY
	//=========================================================
	
	void		ClearPtrn();
	void		ClearPosData();
	void		ClearInfo();
	
	CPtrnInfo&	operator= (CPtrnInfo& rVal);
	
public:
	Image*		m_pImgInput;	
	Image*		m_pImgOrg;		// 패턴 이미지 (original size image)
	Image*		m_pImgOrgHSL;	//uhkang 20100317 Pattern Color Image
	Image*		m_pImgOrgBW;	//uhkang 20100317 Pattern BW Image
	Image*		m_pImgScaled;	// 보여주기 위해 UI에 맞기 크기변경된 이미지
	Image*		m_pImgHSLScaled;// 보여주기 위해 UI에 맞기 크기변경된 이미지
	Image*		m_pImgBWScaled;	// 보여주기 위해 UI에 맞기 크기변경된 이미지
	
	CRect		m_rectPtrn;		// defined Rectangle
	double		m_dThreshold;	// matching threshold
	int			m_iFindNum;		// Retry Number
	int			m_iCntrNum;		// Centering Number

	int			m_iCenMarginX;
	int			m_iCenMarginY;
	double		dCenOffsetX;
	double		dCenOffsetY;
	int			m_iPtrnMatchPriority;
	
	BOOL		m_bDetected;	// 사용하는 패턴 종류에 따라서..detected or defined
	
	double		m_dx, m_dy;			// 나에서 master까지 거리 (offset)
	// master는 m_dx = m_dy = 0.;
	// slave는  m_dx = master.m_dRealX - slave.m_dRealX
	
	double		m_dRealX, m_dRealY;	// 현재 내 좌표.. (temp) 이놈은 저장 안해요~
	// Master/Slave define 시에 사용.

	BOOL		m_bPtrnLearn;	// blue 2009.06.30 움.. 이건 좀 나중에 하세~

	CString		m_strName;		// ptrn name

	TCHAR		m_cMultiPtrn[10][256];

	int			m_iCCDGain;
	int			m_iExposureTime;
	int			m_iLampIntensity;

	int			m_iMethod;
	int			m_nThreshold;
};