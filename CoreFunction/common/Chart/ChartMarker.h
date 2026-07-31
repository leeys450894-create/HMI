// ChartMarker.h: interface for the CChartMarker class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHARTMARKER_H__1EE10C2D_9D17_4655_9075_6CE8505E4325__INCLUDED_)
#define AFX_CHARTMARKER_H__1EE10C2D_9D17_4655_9075_6CE8505E4325__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "och2dmfc.h"

enum CHART_KIND
{
	CHART_TRANSMITTANCE,
		CHART_THICKNESS=0,
		CHART_N,
		CHART_K,
		CHART_NUM
};

class CChartMarker   : public CChart2DTextArea
{
public:
	void SetXY(double x, double y);
	int GetAreaNum();
	void SetAreaNum(int a);
	void SetLayer(int iLayer, int nChart = CHART_NUM);
	void SetShow(BOOL bShow);
	CChartMarker(CChart2D *chart);
	virtual ~CChartMarker();

	CChartMarker& operator=(CChartMarker&);

protected:
	int		m_iId;
	int		m_iLayer;
	double	m_dX;
	double	m_dY;
	double	m_dZ;
	int		m_nMeasArea;
};

#endif // !defined(AFX_CHARTMARKER_H__1EE10C2D_9D17_4655_9075_6CE8505E4325__INCLUDED_)
