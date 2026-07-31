#pragma once

#include <nivision.h>

class CPatternMatchImpl;
class CBinaryHelper;
class CPatternMatchFunction
{
	CPatternMatchImpl	*m_pCtrlImpl;
	
	//------------------------------------------------------------------
	// 외부 접근 가능 함수
	//------------------------------------------------------------------
public:
    bool	LinkDataBase(CBinaryHelper *pBinaryHelper, bool bSave, CString strCategory);
	bool	ProcessDirectMatch(Image *pSrcImage, Image *pTmpImage, Rect rtSearchROI, Rect &rtFoundROI);
	bool	ProcessCorrelation(Image *pSrcImage, Image *pTmpImage, Rect rtSearchROI, Rect &rtFoundROI);
	bool	ProcessMultipMatch(Image *pSrcImage);
	long	GetResultDataCount() const;
	long	GetResultTotal() const;
	CString	GetResultText() const;
	long	GetResultFoundIndex() const;
	long	GetResultShiftX() const;
	long	GetResultShiftY() const;
	float	GetResultScore(long nIndex);
	CString GetResultScoreText() const;
	CRect	GetResultFoundROI(long nIndex) const;
	
public:
	CPatternMatchFunction(CString strDeviceModel, CString strFilePath, CString strCategory);
	~CPatternMatchFunction();	
};

