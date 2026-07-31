// ChartMarker.cpp: implementation of the CChartMarker class.
//
//////////////////////////////////////////////////////////////////////

#include "pch.h"
//#include "RecipeManager.h"
#include "ChartMarker.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CChartMarker::CChartMarker(CChart2D *chart) : CChart2DTextArea(chart)
{

	m_nMeasArea = 0;
	SetForegroundColor(RGB(255,94,0));	// color
}

CChartMarker::~CChartMarker()
{

}

void CChartMarker::SetShow(BOOL bShow)
{
	CChart2DTextArea::SetIsShowing(bShow);
}

void CChartMarker::SetLayer(int iLayer, int nChart)
{
	m_iLayer = iLayer;
	
	CString cStr;
	TCHAR*	pStr[2];
	
	switch( nChart )
	{ 
 
	case CHART_NUM:
		cStr.Format(_T("[%d]"), m_nMeasArea);
		break;

// 	case CHART_THICKNESS:
// 		cStr.Format("%.1f", m_dT[m_iLayer]);
// 		break;
// 		
// 	case CHART_N:
// 		cStr.Format(_T("%.3f"), m_pdblN[m_iLayer]);
// 		break;
// 		
// 	case CHART_K:
// 		cStr.Format(_T("%.3f"), m_pdblK[m_iLayer]);
// 		break;

	case 0:
		cStr.Format(_T(""));
		break;
	}
	
	pStr[0] = (TCHAR*)(LPCTSTR)cStr;
	pStr[1] = NULL;

	HFONT hFont;
	hFont = CreateFont(12, 0, 0, 0, 0, /* Set only Size & */
		0, 0, 0, 0, 0, 0, 0, 0, _T("Tahoma")); /* Typeface */	
	SetFont(hFont);
	SetStrings(pStr);

	DeleteObject(hFont);
}

void CChartMarker::SetAreaNum(int a)
{
	m_nMeasArea = a;
}

int CChartMarker::GetAreaNum()
{
	return m_nMeasArea;
}

CChartMarker& CChartMarker::operator=(CChartMarker& rValue)
{
	this->m_iId 	= rValue.m_iId;
	this->m_iLayer 	= rValue.m_iLayer;
	this->m_dX 		= rValue.m_dX;
	this->m_dY 		= rValue.m_dY;
	this->m_dZ 		= rValue.m_dZ;
	
	this->m_nMeasArea = rValue.m_nMeasArea;

	
	return *this;
}

void CChartMarker::SetXY(double x, double y)
{
	m_dX = x;	SetAttachValueX(m_dX);
	m_dY = y;	SetAttachValueY(m_dY);
}
