//#include "pch.h"
#include "pch.h"
#include "CPtrnInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//====================================================================
// CPtrnInfo : 패턴관련 이미지는 무조건 U8 !!
//====================================================================
CPtrnInfo::CPtrnInfo()
{
	m_strName.Empty();

	m_pImgInput		= imaqCreateImage(IMAQ_IMAGE_RGB, 4);	
	m_pImgOrg		= imaqCreateImage(IMAQ_IMAGE_RGB, 4);	
	m_pImgOrgHSL	= imaqCreateImage(IMAQ_IMAGE_HSL, 4);
	m_pImgOrgBW		= imaqCreateImage(IMAQ_IMAGE_RGB, 4);
 	m_pImgScaled	= imaqCreateImage(IMAQ_IMAGE_RGB, 4);
	m_pImgHSLScaled	= imaqCreateImage(IMAQ_IMAGE_HSL, 4);
	m_pImgBWScaled	= imaqCreateImage(IMAQ_IMAGE_RGB, 4);

	m_rectPtrn = CRect(0, 0, 0, 0);

	m_dThreshold = 500.0;

	m_bDetected = FALSE;
	m_dx = m_dy = 0.0;
	m_dRealX = m_dRealY = 0.0;

	m_iFindNum = m_iCntrNum = 1;

	m_iCenMarginY = m_iCenMarginX = 1;
	dCenOffsetY   = dCenOffsetX   = 0;
	m_iPtrnMatchPriority		  = 0;

	m_bPtrnLearn = FALSE;

	ZeroMemory(&m_cMultiPtrn, sizeof(m_cMultiPtrn));

	m_iCCDGain = 0;
	m_iExposureTime = 10000;
	m_iLampIntensity = 0;

	m_iMethod = 0;
	m_nThreshold = 30;
}

CPtrnInfo::~CPtrnInfo()	
{
	imaqDispose(m_pImgInput);
	imaqDispose(m_pImgOrg);
	imaqDispose(m_pImgOrgHSL);	
	imaqDispose(m_pImgOrgBW);
	imaqDispose(m_pImgScaled);
	imaqDispose(m_pImgHSLScaled);
	imaqDispose(m_pImgBWScaled);
}

void CPtrnInfo::Serialize(CArchive& ar, int iVer)
{
	CObject::Serialize(ar);

	int i=0;
	CString strBuf;

	if(ar.IsStoring())	// save : <<
	{
		ar << m_strName;

		// rect
		ar << m_rectPtrn.left;
		ar << m_rectPtrn.top;
		ar << m_rectPtrn.right;
		ar << m_rectPtrn.bottom;

		ar << m_dThreshold;

		ar << m_bDetected;
		ar << m_dx;
		ar << m_dy;
		ar << m_iFindNum;
		ar << m_iCntrNum;
		ar << m_iCenMarginX;
		ar << m_iCenMarginY;
		ar << dCenOffsetX;
		ar << dCenOffsetY;
		ar << m_iPtrnMatchPriority;

		ar << m_bPtrnLearn;

		ar << m_iCCDGain;
		ar << m_iExposureTime;
		ar << m_iLampIntensity;

		for (i=0; i<10; i++)
		{
			strBuf.Format(_T("%s"), m_cMultiPtrn[i]);
			ar << strBuf;
		}

		if (iVer > 2)
		{
			ar << m_iMethod;
			ar << m_nThreshold;
		}
		
	}
    else	// open : >>
	{
		ar >> m_strName;

		// rect
		ar >> m_rectPtrn.left;
		ar >> m_rectPtrn.top;
		ar >> m_rectPtrn.right;
		ar >> m_rectPtrn.bottom;

		ar >> m_dThreshold;

		ar >> m_bDetected;
		ar >> m_dx;
		ar >> m_dy;
		ar >> m_iFindNum;
		ar >> m_iCntrNum;
		ar >> m_iCenMarginX;
		ar >> m_iCenMarginY;
		ar >> dCenOffsetX;
		ar >> dCenOffsetY;
		ar >> m_iPtrnMatchPriority;

		if( iVer > 0 )
			ar >> m_bPtrnLearn;

		if( iVer > 1)
		{
			ar >> m_iCCDGain;
			ar >> m_iExposureTime;
			ar >> m_iLampIntensity;

			for (i=0; i<10; i++)
			{
				ar >> strBuf;
				_tcscpy_s(m_cMultiPtrn[i], _countof(m_cMultiPtrn[i]), strBuf);
				if( i==0 )
				{
					if( strBuf.IsEmpty() != TRUE)
					{
						m_bPtrnLearn = TRUE;
						m_strName.Format(_T("%s"), strBuf);
					}
					else
					{
						_tcscpy_s(m_cMultiPtrn[i], _countof(m_cMultiPtrn[i]), m_strName);
					}
				}
			}	
		}

		if (iVer > 2)
		{
			ar >> m_iMethod;
			ar >> m_nThreshold;
		}
	}
}

CPtrnInfo& CPtrnInfo::operator=(CPtrnInfo& rVal)
{
	imaqDuplicate(this->m_pImgOrg,		rVal.m_pImgOrg);
	imaqDuplicate(this->m_pImgOrgHSL,	rVal.m_pImgOrgHSL);	//uhkang 20100317
	imaqDuplicate(this->m_pImgOrgBW,	rVal.m_pImgOrgBW);	//uhkang 20100317
	imaqDuplicate(this->m_pImgScaled,	rVal.m_pImgScaled);
	imaqDuplicate(this->m_pImgHSLScaled,rVal.m_pImgHSLScaled);
	imaqDuplicate(this->m_pImgBWScaled,	rVal.m_pImgBWScaled);

	this->m_rectPtrn.left	= rVal.m_rectPtrn.left;
	this->m_rectPtrn.top	= rVal.m_rectPtrn.top;
	this->m_rectPtrn.right	= rVal.m_rectPtrn.right;
	this->m_rectPtrn.bottom	= rVal.m_rectPtrn.bottom;

	this->m_dThreshold	= rVal.m_dThreshold;

	this->m_bDetected	= rVal.m_bDetected;

	this->m_dx			= rVal.m_dx;
	this->m_dy			= rVal.m_dy;

	this->m_dRealX		= rVal.m_dRealX;
	this->m_dRealY		= rVal.m_dRealY;

	this->m_iFindNum	= rVal.m_iFindNum;
	this->m_iCntrNum	= rVal.m_iCntrNum;

	this->m_iCenMarginX = rVal.m_iCenMarginX;
	this->m_iCenMarginY = rVal.m_iCenMarginY;
	this->dCenOffsetX	= rVal.dCenOffsetX;
	this->dCenOffsetY	= rVal.dCenOffsetY;
	this->m_iPtrnMatchPriority	= rVal.m_iPtrnMatchPriority;
	this->m_bPtrnLearn = rVal.m_bPtrnLearn;

	this->m_strName = rVal.m_strName;

	this->m_iCCDGain = rVal.m_iCCDGain;
	this->m_iExposureTime = rVal.m_iExposureTime;
	this->m_iLampIntensity = rVal.m_iLampIntensity;

	this->m_iMethod = rVal.m_iMethod;
	this->m_nThreshold = rVal.m_nThreshold;

	memcpy(&this->m_cMultiPtrn, &rVal.m_cMultiPtrn, sizeof(m_cMultiPtrn));
	
	return *this;
}

void CPtrnInfo::ClearPtrn()
{	
	m_bDetected = FALSE;

	imaqSetImageSize(m_pImgOrg,		 0, 0);
	imaqSetImageSize(m_pImgOrgHSL,	 0, 0);
	imaqSetImageSize(m_pImgOrgBW,	 0, 0);
	imaqSetImageSize(m_pImgScaled,	 0, 0);
	imaqSetImageSize(m_pImgHSLScaled,0, 0);
	imaqSetImageSize(m_pImgBWScaled, 0, 0);

	PixelValue val;
	val.rgb = IMAQ_RGB_WHITE;			// Show all white to hide color pattern
	imaqFillImage(m_pImgOrg,		val, NULL);
	imaqFillImage(m_pImgOrgHSL,		val, NULL);
	imaqFillImage(m_pImgOrgBW,		val, NULL);
	imaqFillImage(m_pImgScaled,		val, NULL);
	imaqFillImage(m_pImgHSLScaled,	val, NULL);
	imaqFillImage(m_pImgBWScaled,	val, NULL);
}

void CPtrnInfo::ClearPosData()
{
	m_dx = m_dy = 0.;
	m_dRealX = m_dRealY = 0.0;
}

void CPtrnInfo::ClearInfo()
{
	m_strName.Empty(); 

	m_rectPtrn = CRect(0, 0, 0, 0);

	m_dThreshold = 500.0;

	m_bDetected = FALSE;
	m_dx = m_dy = 0.0;
	m_dRealX = m_dRealY = 0.0;

	m_iFindNum = m_iCntrNum = 1;

	m_iCenMarginY = m_iCenMarginX = 1;
	dCenOffsetY   = dCenOffsetX   = 0;
	m_iPtrnMatchPriority		  = 0;

	m_bPtrnLearn = FALSE;

	m_iCCDGain = 0;
	m_iExposureTime = 10000;
	m_iLampIntensity = 0;

	m_iMethod = 0;
	m_nThreshold = 30;

	ZeroMemory(&m_cMultiPtrn, sizeof(m_cMultiPtrn));
}