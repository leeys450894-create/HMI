// ChartOrbit.cpp : implementation file
//

#include "pch.h"

//#include "Resource.h"
#include "ChartOrbit.h"


#include "math.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifndef		PI
#define		PI			3.1415926535897932384626433832795
#endif		//#ifndef		PI
#ifndef		RADIAN
#define		RADIAN		57.295779513082320876798154814105
#endif		//#ifndef		RADIAN

/////////////////////////////////////////////////////////////////////////////
// CChartOrbit
double GetDegreeToRadian(double dDegree)
{
	return dDegree / RADIAN;
}

CChartOrbit::CChartOrbit()
{
	m_pData1 = NULL;
	m_pData2 = NULL;
	m_pTextItem	= NULL;
	m_pTextCell = NULL;
	m_pTextCellStatus = NULL;
	m_pTextMeasResult = NULL;
	m_chartMarker = NULL;
	m_pParent = NULL;

	m_dXMax =  100.0;
	m_dXMin = -100.0;
	m_dYMax =  100.0;
	m_dYMin = -100.0;

	m_dLXMax =  100.0;
	m_dLXMin = -100.0;
	m_dLYMax =  100.0;
	m_dLYMin = -100.0;

	m_iLoaderCCut = 0;

	m_dMcXMax =  100.0;
	m_dMcXMin = -100.0;
	m_dMcYMax =  100.0;
	m_dMcYMin = -100.0;

	m_iGlsCCut = -1;

	m_bCurPosDraw = FALSE;
	m_dCurPosX = 0.0;
	m_dCurPosY = 0.0;

	m_nProbe = 1;

	// LYS 2026-08-26 기본은 전 카드 사용. 미장착 카드는 호출부에서 끈다(P5만).
	for (int ipc = 0; ipc < INT_USE_MAX_PROBE_DATA; ipc++)
		m_bProbeCardUse[ipc] = TRUE;

	m_px1	= NULL;
	m_ppy1	= NULL;

	m_pColorProbe	= NULL;
	
	m_bInitSetup = FALSE;
	m_bUpdateResult = FALSE;

	m_nGrpMarginLeft = 30;
	m_nGrpMarginRight = 30;
	m_nGrpMarginTop = 30;
	m_nGrpMarginBottom = 30;

	m_iChart = 0;

	m_nData1 = 1;
	m_dCardWidth = 200;

	m_bUseGlassVcm = FALSE;
	m_nCntGlassVcm = 5;
	ZeroMemory(&m_dGlassVcm, sizeof(m_dGlassVcm));

	m_nMaxCell = 100;

	m_bUseFixtuerTime = FALSE;
	m_nCntFixtuerTime = 8;
	ZeroMemory(&m_dFixtuerTimeF, sizeof(m_dFixtuerTimeF));
	ZeroMemory(&m_dFixtuerTimeR, sizeof(m_dFixtuerTimeR));

	GetConfig();
}

CChartOrbit::~CChartOrbit()
{
	if (m_pData1)		delete m_pData1;
	if (m_pData2)		delete m_pData2;

	int i = 0;

	if( m_chartMarker)
	{
		for (i = 0; i < INT_MEASE_AREA_MAX; i++)	
			delete m_chartMarker[i];
		delete m_chartMarker;
		m_chartMarker = NULL;
	}

	if (m_pTextItem)
	{
		for(i = 0; i < CHART_TEXT_END; i++)
			delete m_pTextItem[i];
		delete m_pTextItem;
	}
	
	if (m_pTextCell)
	{
		for(i = 0; i < m_nMaxCell; i++)		// INT_MAX_CELL_LAYOUT
			delete m_pTextCell[i];
		delete m_pTextCell;
	}

	if (m_pTextCellStatus)
	{
		for (i = 0; i < m_nMaxCell; i++)		// INT_MAX_CELL_LAYOUT
			delete m_pTextCellStatus[i];
		delete m_pTextCellStatus;
	}

	if (m_px1)			delete[] m_px1;
	if (m_pColorProbe)	delete[] m_pColorProbe;

	if (m_ppy1 != NULL)
	{
		for (int ip = 0; ip < m_nData1; ip++)
			delete[] m_ppy1[ip];

		delete[] m_ppy1;
	}
}

BEGIN_MESSAGE_MAP(CChartOrbit, CWnd)
	//{{AFX_MSG_MAP(CChartOrbit)
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChartOrbit message handlers

// Helper function to initialize the chart
void CChartOrbit::InitialSetup(int nProbe, COLORREF* pColorProbe, BOOL bShowBORDER)
{
	int ip = 0;
	int i=0;

	if (m_ppy1)
	{
		for (ip = 0; ip < m_nData1; ip++)
			delete[] m_ppy1[ip];

		delete[] m_ppy1;
	}

	if (m_pTextItem)
	{
		for(i = 0; i < CHART_TEXT_END; i++)
			delete m_pTextItem[i];
		delete m_pTextItem;
		m_pTextItem = NULL;
	}

	if (m_pTextCell)
	{
		for(i = 0; i < m_nMaxCell; i++)		// INT_MAX_CELL_LAYOUT
			delete m_pTextCell[i];
		delete m_pTextCell;
		m_pTextCell = NULL;
	}

	if (m_pTextCellStatus)
	{
		for (i = 0; i < m_nMaxCell; i++)		// INT_MAX_CELL_LAYOUT
			delete m_pTextCellStatus[i];
		delete m_pTextCellStatus;
		m_pTextCellStatus = NULL;
	}

	if (m_chartMarker)
	{
		for(i = 0; i < INT_MEASE_AREA_MAX; i++)
			delete m_chartMarker[i];
		delete m_chartMarker;
		m_chartMarker = NULL;
	}

	m_nProbe = nProbe;

//	m_nPts1 = nProbe + 5;		// Glass size 5 + 1 per Probe
	m_nPts1 = CHART_DATA_ITEM_END;
	m_nData1 = INT_USE_DEFAULT_DATA + INT_USE_MEASPOS_DATA + nProbe + m_nMaxCell; // INT_MAX_CELL_LAYOUT;		// 0  : Glass 외곽선, // 1~ : Probe 위치 +  Probe Card + CellLayout + GLOBAL MARK

	if (m_px1)		delete[] m_px1;
	m_px1	= new double[m_nPts1];
	
	if (m_pData1)	delete m_pData1;
	m_pData1 = new CChart2DData(XRT_DATA_ARRAY, m_nData1, m_nPts1);		 
																								
	if (m_pData2)	delete m_pData2;
	m_pData2 = new CChart2DData(XRT_DATA_ARRAY, nProbe, 2);				// Meas Line

	if (m_pColorProbe)	delete m_pColorProbe;
	m_pColorProbe = new COLORREF[m_nProbe];

	m_pTextItem = new CChart2DTextArea*[CHART_TEXT_END];
	for(i = 0; i < CHART_TEXT_END; i++)
		m_pTextItem[i] = new CChart2DTextArea(this);

	m_pTextCell = new CChart2DTextArea*[m_nMaxCell]; //INT_MAX_CELL_LAYOUT];
	for(i = 0; i < m_nMaxCell; i++)		// INT_MAX_CELL_LAYOUT
		m_pTextCell[i] = new CChart2DTextArea(this);

	m_pTextCellStatus = new CChart2DTextArea * [m_nMaxCell]; //INT_MAX_CELL_LAYOUT];
	for (i = 0; i < m_nMaxCell; i++)		// INT_MAX_CELL_LAYOUT
		m_pTextCellStatus[i] = new CChart2DTextArea(this);

	m_chartMarker = new CChartMarker*[INT_MEASE_AREA_MAX];
	for(i = 0; i < INT_MEASE_AREA_MAX; i++)
		m_chartMarker[i] = new CChartMarker(this);

	for (ip = 0; ip < m_nProbe; ip++)
		m_pColorProbe[ip] = (pColorProbe) ? pColorProbe[ip] : CLR_RED;


	// Batched output
	SetRepaint(FALSE);

	SetType (XRT_TYPE_PLOT);
	SetType2(XRT_TYPE_PLOT);

	// Background color
	COLORREF clrBgnd = ::GetSysColor(COLOR_BTNFACE);
	SetBackgroundColor(clrBgnd);
	SetDataAreaBackgroundColor(CLR_BLACK);	// CLR_WHITE

	// Border
	SetBorderWidth(1);
	SetGraphBorder(XRT_BORDER_NONE);
	SetAxisBoundingBox(TRUE);

	//

	// Axis 
	SetPropString(XRT_AXIS_FONT, _T("Tahoma, 8,"));

	// Data Style
	XrtDataStyle* pds = GetNthDataStyle(0);

	// Glass 
	pds->lpat  = XRT_LPAT_SOLID;
	pds->point = XRT_POINT_NONE;
	pds->color = CLR_GREEN;		
	pds->width = 2;
	SetNthDataStyle(0, pds);

	// GLOBAL MARK
	pds->lpat = XRT_LPAT_NONE;
	pds->point = XRT_POINT_CROSS;		//XRT_POINT_CIRCLE;		//XRT_POINT_SQUARE;	
	pds->pcolor= CLR_VIOLET;		
	pds->psize = INT_MEAS_PATH_PROBE_SIZE;	//8;
	pds->width = 2;
	SetNthDataStyle(1, pds);

	// LOADER
	pds->lpat = XRT_LPAT_SOLID;
	pds->point = XRT_POINT_NONE;		//XRT_POINT_CIRCLE;		//XRT_POINT_SQUARE;	
	pds->color = CLR_YELLOW;
	pds->width = 3;
	SetNthDataStyle(2, pds);

	// ACX
	pds->lpat = XRT_LPAT_SOLID;
	pds->point = XRT_POINT_NONE;		//XRT_POINT_CIRCLE;		//XRT_POINT_SQUARE;	
	pds->color = CLR_YELLOW;
	pds->width = 2;
	SetNthDataStyle(3, pds);

	// Probe PAD 
	for (ip = 0; ip < INT_USE_MAX_PROBE_DATA; ip++)
	{
		pds->lpat = XRT_LPAT_SOLID;
		pds->point = XRT_POINT_NONE;		//XRT_POINT_CIRCLE;		//XRT_POINT_SQUARE;	
		pds->color = CLR_RED;
		pds->width = 2;
		SetNthDataStyle(4 + ip, pds);
	}

	//	Probe Position
	for (ip = 0; ip < nProbe; ip++)
	{
		pds->lpat = XRT_LPAT_NONE;
		pds->point = XRT_POINT_BOX;	//XRT_POINT_SQUARE;		//XRT_POINT_CIRCLE;		//XRT_POINT_SQUARE;	
		pds->pcolor= m_pColorProbe[ip];				
		pds->psize = 20;	//INT_MEAS_PATH_PROBE_SIZE;	//8;
		pds->width = 1;
		SetNthDataStyle(INT_USE_DEFAULT_DATA + ip, pds);
	}

	// Cell Layout Display
	pds->lpat  = XRT_LPAT_SOLID;
	pds->point = XRT_POINT_NONE;
	pds->color = CLR_LIGHT_BLUE;
	pds->width = 1;
	for (ip = 0; ip < m_nMaxCell; ip++)
	{
		SetNthDataStyle(INT_USE_DEFAULT_DATA + nProbe + ip, pds);
	}
	//////////////////////////////////////////////////////////////////////////

	// MEAS POS
	pds->lpat = XRT_LPAT_SOLID;
	pds->point = XRT_POINT_NONE;		//XRT_POINT_CIRCLE;		//XRT_POINT_SQUARE;	
	pds->color = CLR_YELLOW;
	pds->width = 2;
	for (ip = 0; ip < INT_USE_MEASPOS_DATA; ip++)
	{
		SetNthDataStyle(INT_USE_DEFAULT_DATA + nProbe + m_nMaxCell + ip, pds);
	}


	// Meas Group //////////////////////////////////////////////////////////////////////////
//	pds->lpat  = XRT_LPAT_NONE;	// XRT_LPAT_SOLID;	// jsyoon 20160819 line no show
	pds->lpat  = XRT_LPAT_SOLID;	// jsyoon 20160819 line no show
	pds->point = XRT_POINT_DOT;		
	pds->color = CLR_GREEN;				
	pds->pcolor= CLR_BLUE;	
	pds->width = 1;
	pds->psize = 5;	
	for (ip = 0; ip < nProbe; ip++)
		SetNthDataStyle2(ip, pds);

	// double buffering
	SetDoubleBuffer(TRUE);

	// set the border properties
	//SetBorder(XRT3D_BORDER_PLAIN);
	SetBorderWidth(1);
	SetLegendShow(FALSE);		

	//////////////////////////////////////////////////////////////////////////
	m_pData1->SetType(XRT_GENERAL);
	m_pData1->SetNSets(m_nData1);
	for (ip = 0; ip < m_nData1; ip++)
		m_pData1->SetNPoints(ip, m_nPts1);	
	SetData(*m_pData1);
	//
	m_pData2->SetType(XRT_GENERAL);
	m_pData2->SetNSets(nProbe);	
	for (ip = 0; ip < nProbe; ip++)
		m_pData2->SetNPoints(ip, 0);		
	SetData2(*m_pData2);

	// Layout	//////////////////////////////////////////////////////////////////////////
	CRect rect;
	GetClientRect(&rect);
	SetGraphX(0);					// Graph location
	SetGraphY(0);
	SetGraphWidth (rect.right);
	SetGraphHeight(rect.bottom);

	if(bShowBORDER)
	{
		SetGraphMarginLeft(m_nGrpMarginLeft);		// Modified by Jay Hong : 2009.07.17
		SetGraphMarginRight(m_nGrpMarginRight);
		SetGraphMarginTop(m_nGrpMarginTop);
		SetGraphMarginBottom(m_nGrpMarginBottom);

		SetFooterBorder(XRT_BORDER_3D_OUT);
		SetFooterBackgroundColor(CLR_BLACK);
		SetFooterX(rect.right - m_nGrpMarginRight + 2);
		SetFooterY(int(rect.bottom / 2) - 50);
		TCHAR* pStr[7] = { _T(" L "), _T(" O "), _T(" A "), _T(" D "), _T(" E "), _T(" R "), NULL };
		SetFooterForegroundColor(CLR_WHITE);
		SetFooterStrings(pStr);
	}
	else
	{
		SetGraphMarginLeft(1);
		SetGraphMarginRight(1);
		SetGraphMarginTop(m_nGrpMarginTop);
		SetGraphMarginBottom(1);

		TCHAR* pStr[7] = { _T("") };
		SetFooterStrings(pStr);
	}
	
	CString strName;
	
	HFONT hFont;
	if(m_iChart == 0 || m_iChart == 3)
	{
		hFont = CreateFont(9, 0, 0, 0, 0, /* Set only Size & */
			0, 0, 0, 0, 0, 0, 0, 0, _T("Tahoma")); /* Typeface */
	}
	else
	{
		hFont = CreateFont(18, 0, 0, 0, 0, /* Set only Size & */
			0, 0, 0, 0, 0, 0, 0, 0, _T("Tahoma")); /* Typeface */
	}
	
	DeleteObject(hFont);

	//
	SetXMin		(m_dXMin);	SetXMax		(m_dXMax);
	SetXAxisMin (m_dXMin);	SetXAxisMax (m_dXMax);
	SetYMin		(m_dYMin);	SetYMax		(m_dYMax);
	SetYAxisMin (m_dYMin);	SetYAxisMax (m_dYMax);
	SetY2Min	(m_dYMin);	SetY2Max	(m_dYMax);
	SetY2AxisMin(m_dYMin);	SetY2AxisMax(m_dYMax);

	m_ppy1 = new double*[m_nData1];
	for (ip = 0; ip < m_nData1; ip++)
		m_ppy1[ip] = new double[m_nPts1];
	
	for (ip = 0; ip < m_nData1; ip++)
	{
		for(int j = 0; j < m_nPts1; j++)
			m_ppy1[ip][j] = XRT_HUGE_VAL;
	}
	
	for (int j = 0; j < m_nPts1; j++)
		m_px1[j] = XRT_HUGE_VAL;
	
	// YR 050617
	for (ip = 0; ip < m_nData1; ip++)
	{
		m_pData1->SetNPoints(ip, m_nPts1);
		m_pData1->SetXData  (ip, m_px1,		m_nPts1);
		m_pData1->SetYData  (ip, m_ppy1[ip],m_nPts1);
	}

	// jsyoon 20160128 add
	XrtDataStyle * pStyle;
	pStyle = GetXAxisDataStyle();
	pStyle->color = CLR_WHITE;
	SetXAxisDataStyle(pStyle);

	pStyle = GetYAxisDataStyle();
	pStyle->color = CLR_WHITE;
	SetYAxisDataStyle(pStyle);
	//////////////////////////////////////////////////////////////////////////

	SetXAxisShow(TRUE);
	SetYAxisShow(TRUE);
	SetY2AxisShow(FALSE);

	SetRepaint(TRUE);

	m_bInitSetup = TRUE;
}



void CChartOrbit::SetMaxSize(double dXMax, double dXMin, double dYMax, double dYMin, 
							 BOOL bXReverse, BOOL bYReverse, BOOL bXYInvert, int iAngle/* = 0*/)
{
	double dXAxisMax, dXAxisMin, dYAxisMax, dYAxisMin;
	if(bXYInvert)	//uhkang 20110105 
	{
		dXAxisMax = dYMax;
		dXAxisMin = dYMin;
		dYAxisMax = dXMax;
		dYAxisMin = dXMin;
	}
	else
	{
		dXAxisMax = dXMax;
		dXAxisMin = dXMin;
		dYAxisMax = dYMax;
		dYAxisMin = dYMin;
	}

	m_dMcXMax = dXAxisMax;
	m_dMcXMin = dXAxisMin;
	m_dMcYMax = dYAxisMax;
	m_dMcYMin =	dYAxisMin;

	SetXMin		(dXAxisMin);	SetXMax		(dXAxisMax);
	SetXAxisMin	(dXAxisMin);	SetXAxisMax	(dXAxisMax);
	SetYMin		(dYAxisMin);	SetYMax		(dYAxisMax);
	SetYAxisMin (dYAxisMin);	SetYAxisMax (dYAxisMax);
	SetY2Min	(dYAxisMin);	SetY2Max	(dYAxisMax);
	SetY2AxisMin(dYAxisMin);	SetY2AxisMax(dYAxisMax);

	SetXAxisReversed(bXReverse);
	SetYAxisReversed(bYReverse);
	SetY2AxisReversed(bYReverse);
	SetInvertOrientation(bXYInvert);

	
	CRect rect;
	GetClientRect(&rect);	
	switch (iAngle)
	{
	case 0:	// 0
		{
			SetFooterX(rect.right-m_nGrpMarginRight + 1);
			SetFooterY((int)(rect.top+rect.Height()/2)-m_nGrpMarginTop-m_nGrpMarginBottom);
			TCHAR* pStr[7] = { _T(" L "), _T(" O "), _T(" A "), _T(" D "), _T(" E "), _T(" R "), NULL};
			SetFooterStrings(pStr);
		}
		
		break;
	case 1:	// 90
		{
			SetFooterX((int)(rect.left+rect.Width()/2)-m_nGrpMarginLeft-m_nGrpMarginRight);
			SetFooterY(rect.bottom-m_nGrpMarginBottom);
			TCHAR *pStr[] = { _T("  L O A D E R  "), NULL };
			SetFooterStrings(pStr);
		}
		
		break;
	case 2:	// 180
		{
			SetFooterX(rect.left + 1);
			SetFooterY((int)(rect.top+rect.Height()/2)-m_nGrpMarginTop-m_nGrpMarginBottom);
			TCHAR* pStr[7] = { _T(" L "), _T(" O "), _T(" A "), _T(" D "), _T(" E "), _T(" R "), NULL};
			SetFooterStrings(pStr);
		}
		
		break;
	case 3: // 270
		{
			SetFooterX((int)(rect.left+rect.Width()/2)-m_nGrpMarginLeft-m_nGrpMarginRight);
			SetFooterY(rect.top);
			TCHAR *pStr[] = { _T("  L O A D E R  "), NULL };
			SetFooterStrings(pStr);
		}
		
		break;
	default:
		break;
	}

}

void CChartOrbit::SetGlassSize(double dXMax, double dXMin, double dYMax, double dYMin, int iGlsCCut)
{	
	SetRepaint(FALSE);

	m_dXMax = dXMax;
	m_dXMin = dXMin;
	m_dYMax = dYMax;
	m_dYMin = dYMin;

	double dEdgeCutX = 100, dEdgeCutY = 100;
	dEdgeCutX = fabs(m_dXMax - m_dXMin) * 0.05;
	dEdgeCutY = fabs(m_dYMax - m_dYMin) * 0.05;

	if(iGlsCCut == 1)
	{
		m_px1[CHART_DATA_ITEM0] = dXMin;			 m_ppy1[0][CHART_DATA_ITEM0] = dYMin;
		m_px1[CHART_DATA_ITEM1] = dXMax;			 m_ppy1[0][CHART_DATA_ITEM1] = dYMin;
		m_px1[CHART_DATA_ITEM2] = dXMax;			 m_ppy1[0][CHART_DATA_ITEM2] = dYMax - dEdgeCutY;
		m_px1[CHART_DATA_ITEM3] = dXMax - dEdgeCutX; m_ppy1[0][CHART_DATA_ITEM3] = dYMax;
		m_px1[CHART_DATA_ITEM4] = dXMin;			 m_ppy1[0][CHART_DATA_ITEM4] = dYMax;
		m_px1[CHART_DATA_ITEM5] = dXMin;			 m_ppy1[0][CHART_DATA_ITEM5] = dYMin;
	}
	else if(iGlsCCut == 2)
	{
		m_px1[CHART_DATA_ITEM0] = dXMin;			 m_ppy1[0][CHART_DATA_ITEM0] = dYMin;
		m_px1[CHART_DATA_ITEM1] = dXMax;			 m_ppy1[0][CHART_DATA_ITEM1] = dYMin;
		m_px1[CHART_DATA_ITEM2] = dXMax;			 m_ppy1[0][CHART_DATA_ITEM2] = dYMax;
		m_px1[CHART_DATA_ITEM3] = dXMin + dEdgeCutX; m_ppy1[0][CHART_DATA_ITEM3] = dYMax;
		m_px1[CHART_DATA_ITEM4] = dXMin;			 m_ppy1[0][CHART_DATA_ITEM4] = dYMax - dEdgeCutY;
		m_px1[CHART_DATA_ITEM5] = dXMin;			 m_ppy1[0][CHART_DATA_ITEM5] = dYMin;
	}
	else if(iGlsCCut == 3)
	{
		m_px1[CHART_DATA_ITEM0] = dXMin + dEdgeCutX; m_ppy1[0][CHART_DATA_ITEM0] = dYMin;
		m_px1[CHART_DATA_ITEM1] = dXMax;			 m_ppy1[0][CHART_DATA_ITEM1] = dYMin;
		m_px1[CHART_DATA_ITEM2] = dXMax;			 m_ppy1[0][CHART_DATA_ITEM2] = dYMax;
		m_px1[CHART_DATA_ITEM3] = dXMin;			 m_ppy1[0][CHART_DATA_ITEM3] = dYMax;
		m_px1[CHART_DATA_ITEM4] = dXMin;			 m_ppy1[0][CHART_DATA_ITEM4] = dYMin + dEdgeCutY;
		m_px1[CHART_DATA_ITEM5] = dXMin + dEdgeCutX; m_ppy1[0][CHART_DATA_ITEM5] = dYMin;
	}
	else if(iGlsCCut == 4)
	{
		m_px1[CHART_DATA_ITEM0] = dXMin;			 m_ppy1[0][CHART_DATA_ITEM0] = dYMin;
		m_px1[CHART_DATA_ITEM1] = dXMax - dEdgeCutX; m_ppy1[0][CHART_DATA_ITEM1] = dYMin;
		m_px1[CHART_DATA_ITEM2] = dXMax;			 m_ppy1[0][CHART_DATA_ITEM2] = dYMin + dEdgeCutY;
		m_px1[CHART_DATA_ITEM3] = dXMax;			 m_ppy1[0][CHART_DATA_ITEM3] = dYMax;
		m_px1[CHART_DATA_ITEM4] = dXMin;			 m_ppy1[0][CHART_DATA_ITEM4] = dYMax;
		m_px1[CHART_DATA_ITEM5] = dXMin;			 m_ppy1[0][CHART_DATA_ITEM5] = dYMin;
	}
	else
	{
		m_px1[CHART_DATA_ITEM0] = dXMin;			 m_ppy1[0][CHART_DATA_ITEM0] = dYMin;
		m_px1[CHART_DATA_ITEM1] = dXMax;			 m_ppy1[0][CHART_DATA_ITEM1] = dYMin;
		m_px1[CHART_DATA_ITEM2] = dXMax;			 m_ppy1[0][CHART_DATA_ITEM2] = dYMax;
		m_px1[CHART_DATA_ITEM3] = dXMin;			 m_ppy1[0][CHART_DATA_ITEM3] = dYMax;
		m_px1[CHART_DATA_ITEM4] = dXMin;			 m_ppy1[0][CHART_DATA_ITEM4] = dYMin;
		m_px1[CHART_DATA_ITEM5] = dXMin;			 m_ppy1[0][CHART_DATA_ITEM5] = dYMin;
	}
	
	m_iGlsCCut = iGlsCCut;
	
	//YR 070312 22:33 - Chart message 이용 - OnOC2Repaint 참조 
	// Glass
	m_pData1->SetXData(0, m_px1,	CHART_DATA_ITEM_END);
	m_pData1->SetYData(0, m_ppy1[0],CHART_DATA_ITEM_END);

	SetRepaint(TRUE);
}

void CChartOrbit::SetLoaderSize(double dXMax, double dXMin, double dYMax, double dYMin, int iLoaderCCut)
{
	m_iLoaderCCut = iLoaderCCut;
	m_dLXMax = dXMax;
	m_dLYMax = dYMax;
	m_dLXMin = dXMin;
	m_dLYMin = dYMin;
}

// LYS 2026-08-26 프로브 카드(시리즈 4..8 = P1..P5)를 표시 범위 밖으로 옮기고 라벨을 지운다.
// SetProbeCardSize()가 5개를 전부 그리므로 미장착 카드는 제거
void CChartOrbit::HideProbeCard(int nProbeID)
{
	if (nProbeID < 0 || nProbeID >= INT_USE_MAX_PROBE_DATA)
		return;

	if (m_pData1 != NULL)
	{
		for (int i = 0; i < CHART_DATA_ITEM_END; i++)
		{
			m_pData1->SetXElement(4 + nProbeID, i, XRT_HUGE_VAL);
			m_pData1->SetYElement(4 + nProbeID, i, XRT_HUGE_VAL);
		}
	}

	if (m_pTextItem != NULL && m_pTextItem[CHART_TEXT_PROBE1 + nProbeID] != NULL)
	{
		CString strEmpty;	strEmpty.Empty();
		TCHAR* pStr[2];
		pStr[0] = (TCHAR*)(LPCTSTR)strEmpty;
		pStr[1] = NULL;
		m_pTextItem[CHART_TEXT_PROBE1 + nProbeID]->SetStrings(pStr);
	}
}

void CChartOrbit::SetProbeCardUse(int nProbeID, BOOL bUse)
{
	if (nProbeID < 0 || nProbeID >= INT_USE_MAX_PROBE_DATA)
		return;

	m_bProbeCardUse[nProbeID] = bUse;

	// 차트 데이터 생성 전에도 호출 가능. 그 경우 이후 SetProbeCardSize()가 플래그를 따른다.
	if (bUse != TRUE)
	{
		SetRepaint(FALSE);
		HideProbeCard(nProbeID);
		SetRepaint(TRUE);
	}
}

void CChartOrbit::SetProbeCardSize(double dXMax, double dXMin, double dYMax, double dYMin)
{	
	SetRepaint(FALSE);

	double* px = new double[CHART_DATA_ITEM_END];
	double* py = new double[CHART_DATA_ITEM_END];

	px[0] = dXMin;			py[0] = dYMax;
	px[1] = dXMax;			py[1] = dYMax;
	px[2] = dXMax;			py[2] = dYMin;
	px[3] = dXMin;			py[3] = dYMin;
	px[4] = dXMin;			py[4] = dYMax;
	px[5] = XRT_HUGE_VAL;	py[5] = XRT_HUGE_VAL;

	for (size_t i = 0; i < INT_USE_MAX_PROBE_DATA + 1; i++)		// ACX + PROBE
	{
		// LYS 2026-08-26 i==0은 ACX, i>=1이 프로브 카드. 미장착 카드는 건너뛴다
		if (i > 0 && m_bProbeCardUse[i - 1] != TRUE)
			continue;

		m_pData1->SetXData(3 + i, px, CHART_DATA_ITEM_END);
		m_pData1->SetYData(3 + i, py, CHART_DATA_ITEM_END);
	}
	
	m_dCardHeight = dYMax - dYMin;
	m_dCardWidth = dXMax - dXMin;

	delete[] px;
	delete[] py;
	px = NULL;
	py = NULL;

	SetRepaint(TRUE);
}

void CChartOrbit::SetGlobalMakrPos(double x, double y)
{
	if(x < m_dMcXMax && x > m_dMcXMin && y < m_dMcYMax && y > m_dMcYMin)
	{
		SetRepaint(FALSE);

		m_pData1->SetXElement(1, 0, x);
		m_pData1->SetYElement(1, 0, y);

		SetRepaint(TRUE);
	}
}


void CChartOrbit::SetCurPos(int iProbe, BOOL bDraw, double x, double y)
{
	m_bCurPosDraw = bDraw;
	
	SetRepaint(FALSE);

	if(iProbe == -1 || iProbe == -2 || iProbe == -3 || iProbe == -4 || iProbe == -5 )		// PROBE
	{
		m_dCurPosX = x;
		m_dCurPosY = (m_bCurPosDraw) ? y : XRT_HUGE_VAL;

		CString strHead;
		strHead.Empty();
		int nProbeID = 0;
		switch (iProbe)
		{
		case -1:
	//		strHead.Format(_T("P%d"), 1);
			strHead.Format(_T("A%d"), 1);
			nProbeID = 0;
			break;
		case -2:
	//		strHead.Format(_T("P%d"), 2);
			strHead.Format(_T("A%d"), 2);
			nProbeID = 1;
			break;
		case -3:
	//		strHead.Format(_T("P%d"), 3);
			strHead.Format(_T("A%d"), 3);
			nProbeID = 2;
			break;
		case -4:
	//		strHead.Format(_T("P%d"), 4);
			strHead.Format(_T("A%d"), 4);
			nProbeID = 3;
			break;
		case -5:
	//		strHead.Format(_T("P%d"), 5);
			strHead.Format(_T("P"));
			nProbeID = 4;
			break;
		default:
			break;
		}
		// LYS 2026-08-26 미장착 카드는 건너뛰지 않고 숨김 처리한다(이미 그려져 있을 수 있음).
		if (m_bProbeCardUse[nProbeID] != TRUE)
		{
			HideProbeCard(nProbeID);
			SetRepaint(TRUE);
			return;
		}

		int iProbeCard = 4;
		iProbeCard += nProbeID; // ((iProbe * -1) - 1);

		if (m_dCurPosX < m_dMcXMax && m_dCurPosX > m_dMcXMin && m_dCurPosY < m_dMcYMax && m_dCurPosY > m_dMcYMin)
		{
			m_pData1->SetXElement(iProbeCard, 0, m_dCurPosX);
			m_pData1->SetXElement(iProbeCard, 1, m_dCurPosX + m_dCardWidth);
			m_pData1->SetXElement(iProbeCard, 2, m_dCurPosX + m_dCardWidth);
			m_pData1->SetXElement(iProbeCard, 3, m_dCurPosX);
			m_pData1->SetXElement(iProbeCard, 4, m_dCurPosX);

			TCHAR* pStr[2];
			pStr[0] = (TCHAR*)(LPCTSTR)strHead;
			pStr[1] = NULL;

			HFONT hFont;
			if (m_iChart == 0 || m_iChart == 3)
			{
				hFont = CreateFont(18, 0, 0, 0, 0, /* Set only Size & */
					0, 0, 0, 0, 0, 0, 0, 0, _T("Tahoma")); /* Typeface */
			}
			else
			{
				hFont = CreateFont(18, 0, 0, 0, 0, /* Set only Size & */
					0, 0, 0, 0, 0, 0, 0, 0, _T("Tahoma")); /* Typeface */
			}

			m_pTextItem[CHART_TEXT_PROBE1 + nProbeID]->SetFont(hFont);
			m_pTextItem[CHART_TEXT_PROBE1 + nProbeID]->SetForegroundColor(CLR_YELLOW);
			m_pTextItem[CHART_TEXT_PROBE1 + nProbeID]->SetBackgroundColor(CLR_RED);
			m_pTextItem[CHART_TEXT_PROBE1 + nProbeID]->SetStrings(pStr);

			m_pTextItem[CHART_TEXT_PROBE1 + nProbeID]->SetAttachValueX(m_dCurPosX);
			m_pTextItem[CHART_TEXT_PROBE1 + nProbeID]->SetAttachValueY(m_dCardHeight / 2 + 30);

			DeleteObject(hFont);
		}
		else
		{
			m_pData1->SetXElement(iProbeCard, 0, m_dCurPosX);
			m_pData1->SetXElement(iProbeCard, 1, m_dCurPosX + m_dCardWidth);
			m_pData1->SetXElement(iProbeCard, 2, m_dCurPosX + m_dCardWidth);
			m_pData1->SetXElement(iProbeCard, 3, m_dCurPosX);
			m_pData1->SetXElement(iProbeCard, 4, m_dCurPosX);

			CString strHead;	strHead.Format(_T(""));
			TCHAR* pStr[2];
			pStr[0] = (TCHAR*)(LPCTSTR)strHead;
			pStr[1] = NULL;
			m_pTextItem[CHART_TEXT_PROBE1 + nProbeID]->SetStrings(pStr);
		}
		
	}
	else if (iProbe == -11)		// AutoChange
	{
		m_dCurPosX = x;
		m_dCurPosY = (m_bCurPosDraw) ? y : XRT_HUGE_VAL;

		int nProbeID = CHART_TEXT_X;
		CString strHead;
		strHead.Empty();
		strHead.Format(_T("AC"));
		int iACX = 3;

		if (m_dCurPosX < m_dMcXMax && m_dCurPosX > m_dMcXMin && m_dCurPosY < m_dMcYMax && m_dCurPosY > m_dMcYMin)
		{
			m_pData1->SetXElement(iACX, 0, m_dCurPosX);
			m_pData1->SetXElement(iACX, 1, m_dCurPosX + 30);
			m_pData1->SetXElement(iACX, 2, m_dCurPosX + 30);
			m_pData1->SetXElement(iACX, 3, m_dCurPosX);
			m_pData1->SetXElement(iACX, 4, m_dCurPosX);

			TCHAR* pStr[2];
			pStr[0] = (TCHAR*)(LPCTSTR)strHead;
			pStr[1] = NULL;

			HFONT hFont;
			if (m_iChart == 0 || m_iChart == 3)
			{
				hFont = CreateFont(18, 0, 0, 0, 0, /* Set only Size & */
					0, 0, 0, 0, 0, 0, 0, 0, _T("Tahoma")); /* Typeface */
			}
			else
			{
				hFont = CreateFont(18, 0, 0, 0, 0, /* Set only Size & */
					0, 0, 0, 0, 0, 0, 0, 0, _T("Tahoma")); /* Typeface */
			}

			m_pTextItem[nProbeID]->SetFont(hFont);
			m_pTextItem[nProbeID]->SetForegroundColor(CLR_YELLOW);
			m_pTextItem[nProbeID]->SetBackgroundColor(CLR_RED);
			m_pTextItem[nProbeID]->SetStrings(pStr);

			m_pTextItem[nProbeID]->SetAttachValueX(m_dCurPosX);
			m_pTextItem[nProbeID]->SetAttachValueY(m_dCardHeight / 2 + 30);

			DeleteObject(hFont);
		}
		else
		{
			m_pData1->SetXElement(iACX, 0, m_dCurPosX);
			m_pData1->SetXElement(iACX, 1, m_dCurPosX + 30);
			m_pData1->SetXElement(iACX, 2, m_dCurPosX + 30);
			m_pData1->SetXElement(iACX, 3, m_dCurPosX);
			m_pData1->SetXElement(iACX, 4, m_dCurPosX);

			CString strHead;	strHead.Format(_T(""));
			TCHAR* pStr[2];
			pStr[0] = (TCHAR*)(LPCTSTR)strHead;
			pStr[1] = NULL;
			m_pTextItem[nProbeID]->SetStrings(pStr);
		}

	}
	else if( iProbe == -10)		// STAGE
	{
	//	m_dCurPosX = x;
	//	m_dCurPosY = (m_bCurPosDraw) ? y : XRT_HUGE_VAL;

		double dCos = 0, dSin = 0, dDegree = 0;
		double* px = new double[CHART_DATA_ITEM_END];
		double* py = new double[CHART_DATA_ITEM_END];
		double dXMin = 0, dYMin = 0, dXMax = 0, dYMax = 0;
		dXMin = m_dLXMin-10;
		dYMin = m_dLYMin-10;
		dXMax = m_dLXMax+10;
		dYMax = m_dLYMax+10;


		dDegree = GetDegreeToRadian(y);
		dCos = cos(dDegree);
		dSin = sin(dDegree);

		double dEdgeCutX = 100, dEdgeCutY = 100;
		dEdgeCutX = fabs(m_dXMax - m_dXMin) * 0.05;
		dEdgeCutY = fabs(m_dYMax - m_dYMin) * 0.05;

		switch (m_iLoaderCCut)
		{
		case 1:
			px[CHART_DATA_ITEM0] = x + (dCos * dXMin - dSin * dYMin);			py[CHART_DATA_ITEM0] = (dCos * dYMin + dSin * dXMin);
			px[CHART_DATA_ITEM1] = x + (dCos * dXMax - dSin * dYMin);			py[CHART_DATA_ITEM1] = (dCos * dYMin + dSin * dXMax);
			px[CHART_DATA_ITEM2] = x + (dCos * dXMax - dSin * (dYMax - dEdgeCutX));	py[CHART_DATA_ITEM2] = (dCos * (dYMax - dEdgeCutY) + dSin * dXMax);
			px[CHART_DATA_ITEM3] = x + (dCos * (dXMax - dEdgeCutX) - dSin * dYMax);	py[CHART_DATA_ITEM3] = (dCos * dYMax + dSin * (dXMax - dEdgeCutY));
			px[CHART_DATA_ITEM4] = x + (dCos * dXMin - dSin * dYMax);			py[CHART_DATA_ITEM4] = (dCos * dYMax + dSin * dXMin);
			px[CHART_DATA_ITEM5] = x + (dCos * dXMin - dSin*dYMin);			    py[CHART_DATA_ITEM5] = (dCos*dYMin + dSin*dXMin);
			break;
		case 2:
			px[CHART_DATA_ITEM0] = x + (dCos * dXMin - dSin*dYMin);				py[CHART_DATA_ITEM0] = (dCos*dYMin + dSin*dXMin);
			px[CHART_DATA_ITEM1] = x + (dCos * dXMax - dSin*dYMin);				py[CHART_DATA_ITEM1] = (dCos*dYMin + dSin*dXMax);
			px[CHART_DATA_ITEM2] = x + (dCos * dXMax - dSin*dYMax);				py[CHART_DATA_ITEM2] = (dCos*dYMax + dSin*dXMax);
			px[CHART_DATA_ITEM3] = x + (dCos * (dXMin+ dEdgeCutX) - dSin*dYMax);		py[CHART_DATA_ITEM3] = (dCos*dYMax + dSin*(dXMin + dEdgeCutY));
			px[CHART_DATA_ITEM4] = x + (dCos * dXMin - dSin*(dYMax- dEdgeCutX));		py[CHART_DATA_ITEM4] = (dCos*(dYMax - dEdgeCutY) + dSin*dXMin);
			px[CHART_DATA_ITEM5] = x + (dCos * dXMin - dSin*dYMin);				py[CHART_DATA_ITEM5] = (dCos*dYMin + dSin*dXMin);
			break;
		case 3:
			px[CHART_DATA_ITEM0] = x + (dCos * (dXMin + dEdgeCutX) - dSin * dYMin);	py[CHART_DATA_ITEM0] = (dCos * dYMin + dSin * (dXMin + dEdgeCutY));
			px[CHART_DATA_ITEM1] = x + (dCos * dXMax - dSin * dYMin);			py[CHART_DATA_ITEM1] = (dCos * dYMin + dSin * dXMax);
			px[CHART_DATA_ITEM2] = x + (dCos * dXMax - dSin * dYMax);			py[CHART_DATA_ITEM2] = (dCos * dYMax + dSin * dXMax);
			px[CHART_DATA_ITEM3] = x + (dCos * dXMin - dSin * dYMax);			py[CHART_DATA_ITEM3] = (dCos * dYMax + dSin * dXMin);
			px[CHART_DATA_ITEM4] = x + (dCos * dXMin - dSin * (dYMin + dEdgeCutX));	py[CHART_DATA_ITEM4] = (dCos * (dYMin + dEdgeCutY) + dSin * dXMin);
			px[CHART_DATA_ITEM5] = x + (dCos * (dXMin+ dEdgeCutX) - dSin*dYMin);	    py[CHART_DATA_ITEM5] = (dCos*dYMin + dSin*(dXMin+ dEdgeCutY));
			break;
		case 4:
			px[CHART_DATA_ITEM0] = x + (dCos * dXMin - dSin * dYMin);			py[CHART_DATA_ITEM0] = (dCos * dYMin + dSin * dXMin);
			px[CHART_DATA_ITEM1] = x + (dCos * (dXMax - dEdgeCutX) - dSin * dYMin);	py[CHART_DATA_ITEM1] = (dCos * dYMin + dSin * (dXMax - dEdgeCutY));
			px[CHART_DATA_ITEM2] = x + (dCos * dXMax - dSin * (dYMin + dEdgeCutX));	py[CHART_DATA_ITEM2] = (dCos * (dYMin + dEdgeCutY) + dSin * dXMax);
			px[CHART_DATA_ITEM3] = x + (dCos * dXMax - dSin * dYMax);			py[CHART_DATA_ITEM3] = (dCos * dYMax + dSin * dXMax);
			px[CHART_DATA_ITEM4] = x + (dCos * dXMin - dSin * dYMax);			py[CHART_DATA_ITEM4] = (dCos * dYMax + dSin * dXMin);
			px[CHART_DATA_ITEM5] = x + (dCos * dXMin - dSin*dYMin);			    py[CHART_DATA_ITEM5] = (dCos*dYMin + dSin*dXMin);
			break;
		default:
			px[CHART_DATA_ITEM0] = x + (dCos * dXMin - dSin * dYMin);			py[CHART_DATA_ITEM0] = (dCos * dYMin + dSin * dXMin);
			px[CHART_DATA_ITEM1] = x + (dCos * dXMax - dSin * dYMin);			py[CHART_DATA_ITEM1] = (dCos * dYMin + dSin * dXMax);
			px[CHART_DATA_ITEM2] = x + (dCos * dXMax - dSin * dYMax);			py[CHART_DATA_ITEM2] = (dCos * dYMax + dSin * dXMax);
			px[CHART_DATA_ITEM3] = x + (dCos * dXMin - dSin * dYMax);			py[CHART_DATA_ITEM3] = (dCos * dYMax + dSin * dXMin);
			px[CHART_DATA_ITEM4] = x + (dCos * dXMin - dSin * dYMin);			py[CHART_DATA_ITEM4] = (dCos * dYMin + dSin * dXMin);
			px[CHART_DATA_ITEM5] = x + (dCos * dXMin - dSin*dYMin);			    py[CHART_DATA_ITEM5] = (dCos*dYMin + dSin*dXMin);

			break;
		}

		m_pData1->SetXData(2, px, CHART_DATA_ITEM_END);
		m_pData1->SetYData(2, py, CHART_DATA_ITEM_END);

		delete[] px;
		delete[] py;
		px = NULL;
		py = NULL;
	}
	else
	{
		m_dCurPosX = x;
		m_dCurPosY = (m_bCurPosDraw) ? y : XRT_HUGE_VAL;

		if(m_dCurPosX < m_dMcXMax && m_dCurPosX > m_dMcXMin && m_dCurPosY < m_dMcYMax && m_dCurPosY > m_dMcYMin)	// jsyoon 201401015 change
		{
			m_pData1->SetXElement(iProbe + INT_USE_DEFAULT_DATA, 0, m_dCurPosX);
			m_pData1->SetYElement(iProbe + INT_USE_DEFAULT_DATA, 0, m_dCurPosY);

			CString strHead;	
//			strHead.Format("H%d", iProbe+1);		// jsyoon 20161018 change
// 			if( iProbe < 2 )
// 				strHead.Format("H%d", iProbe+1);
// 			else if( iProbe < 4 )
// 				strHead.Format("P%d", iProbe-1);	// PROBE
// 			else if( iProbe < 6 )
// 				strHead.Format("G%d", iProbe-3);	// GLASS
// 			else
				strHead.Format(_T("H%d"), iProbe+1);

			TCHAR* pStr[2];
			pStr[0] = (TCHAR*)(LPCTSTR)strHead;
			pStr[1] = NULL;

			HFONT hFont;
			if(m_iChart == 0 || m_iChart == 3)
			{
				hFont = CreateFont(18, 0, 0, 0, 0, /* Set only Size & */
					0, 0, 0, 0, 0, 0, 0, 0, _T("Tahoma")); /* Typeface */
// 				m_pTextItem[CHART_TEXT_H1+iProbe]->SetAttachValueX(m_dCurPosX);
// 				m_pTextItem[CHART_TEXT_H1+iProbe]->SetAttachValueY(m_dCurPosY);
		//		m_pTextItem[CHART_TEXT_H1+iProbe]->SetAttachValueY(m_dCurPosY-20);
			}
			else
			{
				hFont = CreateFont(18, 0, 0, 0, 0, /* Set only Size & */
					0, 0, 0, 0, 0, 0, 0, 0, _T("Tahoma")); /* Typeface */

// 				m_pTextItem[CHART_TEXT_H1+iProbe]->SetAttachValueX(m_dCurPosX);
// 				m_pTextItem[CHART_TEXT_H1+iProbe]->SetAttachValueY(m_dCurPosY);
		//		m_pTextItem[CHART_TEXT_H1+iProbe]->SetAttachValueY(m_dCurPosY-50);
			}

			m_pTextItem[CHART_TEXT_H1+iProbe]->SetFont(hFont);
			m_pTextItem[CHART_TEXT_H1+iProbe]->SetForegroundColor(CLR_YELLOW);
			m_pTextItem[CHART_TEXT_H1+iProbe]->SetBackgroundColor(CLR_RED);
			m_pTextItem[CHART_TEXT_H1+iProbe]->SetStrings(pStr);

			m_pTextItem[CHART_TEXT_H1+iProbe]->SetAttachValueX(m_dCurPosX);
			m_pTextItem[CHART_TEXT_H1+iProbe]->SetAttachValueY(m_dCurPosY);

			DeleteObject(hFont);
		}
		else
		{
			m_pData1->SetXElement(iProbe + INT_USE_DEFAULT_DATA, 0, m_dCurPosX);
			m_pData1->SetYElement(iProbe + INT_USE_DEFAULT_DATA, 0, m_dCurPosY);
			CString strHead;	strHead.Format(_T(""));
			TCHAR* pStr[2];
			pStr[0] = (TCHAR*)(LPCTSTR)strHead;
			pStr[1] = NULL;
			m_pTextItem[CHART_TEXT_H1+iProbe]->SetStrings(pStr);
		}
	}

	SetRepaint(TRUE);
}

void CChartOrbit::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	// Because mouse click on this chart does not change active view to AutoStageView,
	// I set active view explicitly.
	//((CFrameWnd*)gl_pMainFrame)->SetActiveView((CView*)GetParent());

	SetFocus();		// Get key focus

	XrtMapResult map;
	XrtRegion region = Map(XRT_DATASET1, point.x, point.y, &map);
	if (region == XRT_RGN_IN_GRAPH)
	{
//		if (map.x > m_dXMax)	map.x = m_dXMax;
//		if (map.x < m_dXMin)	map.x = m_dXMin;
//		if (map.y > m_dYMax)	map.y = m_dYMax;
//		if (map.y < m_dYMin)	map.y = m_dYMin;
//
////		float fv;
////		fv = (float)map.x;	WPARAM wp = *(WPARAM*)&fv;
////		fv = (float)map.y;	LPARAM lp = *(LPARAM*)&fv;
////		GetParent()->PostMessage(UWM_MAPCURSORCLICK, wp, lp);
		TRACE("ChartOrbit clicked at (%.2f,%.2f)\n", map.x, map.y);
	}

	CWnd::OnLButtonDown(nFlags, point);
}

void CChartOrbit::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here
//	// Because mouse click on this chart does not change active view to AutoStageView,
//	// I set active view explicitly.
//	((CFrameWnd*)gl_pMainFrame)->SetActiveView((CView*)GetParent());
//
//	SetFocus();		// Get key focus
//
//	CMenu *subMenu;
//	subMenu = m_menuContext.GetSubMenu(0);
//
//	CRect rect = CRect(point.x, point.y, point.x, point.y);
//	ClientToScreen(&rect); 
//	// This does not work, maybe because Chart3D could not handle menu messages...
//	// So, I used parent window, which is CFormView, instead of "this".
//	//subMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rect.left, rect.top, this);	
//	subMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rect.left, rect.top, GetParent());	

	SetFocus();		// Get key focus

	CWnd::OnRButtonDown(nFlags, point);
}

BOOL CChartOrbit::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*message*/) 
{
	// TODO: Add your message handler code here and/or call default
	::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_CROSS));
	
	return TRUE;
	//return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

void CChartOrbit::SetProbeColor(COLORREF* pColorProbe)
{
	XrtDataStyle* pds;	

	for (int i = 0; i < m_nProbe; i++)
	{
		m_pColorProbe[i] = (pColorProbe) ? pColorProbe[i] : CLR_RED;

		pds = GetNthDataStyle(i + 2);
		pds->pcolor= m_pColorProbe[i];				
		SetNthDataStyle(i + 2, pds);
	}
}

void CChartOrbit::SetPanelParam(BOOL bPanelDraw, int nPanelNx, int nPanelNy, double dPanelX0, double dPanelY0,
												double dPanelLx, double dPanelLy, double dPanelDx, double dPanelDy)
{
	m_bPanelDraw= bPanelDraw;
	m_nPanelNx	= nPanelNx;
	m_nPanelNy	= nPanelNy;
	m_dPanelX0	= dPanelX0;
	m_dPanelY0	= dPanelY0;
	m_dPanelLx	= dPanelLx;
	m_dPanelLy	= dPanelLy;
	m_dPanelDx	= dPanelDx;
	m_dPanelDy	= dPanelDy;
}

void CChartOrbit::SetRemoveGlassCellLayout(int iIndex)
{
	SetRepaint(FALSE);
	
	double px[CHART_DATA_ITEM_END];
	double py[CHART_DATA_ITEM_END];
	for(int i = 0; i < CHART_DATA_ITEM_END; i++)
	{
		px[i] = XRT_HUGE_VAL;
		py[i] = XRT_HUGE_VAL;
	}

	TCHAR* MM[2] = {_T(""), NULL};
	if(iIndex == -1)
	{
		for(int i = 0; i < m_nMaxCell; i++)
		{
			m_pData1->SetNPoints(m_nProbe + INT_USE_DEFAULT_DATA + i, CHART_DATA_ITEM_END);
			m_pData1->SetXData(m_nProbe + INT_USE_DEFAULT_DATA +i, px, CHART_DATA_ITEM_END);
			m_pData1->SetYData(m_nProbe + INT_USE_DEFAULT_DATA +i, py, CHART_DATA_ITEM_END);
			m_pTextCell[i]->SetStrings(MM);

			m_pTextCellStatus[i]->SetStrings(MM);
		}
	}
	else
	{
		m_pData1->SetNPoints(m_nProbe + INT_USE_DEFAULT_DATA, CHART_DATA_ITEM_END);
		m_pData1->SetXData(iIndex + m_nProbe + INT_USE_DEFAULT_DATA, px, CHART_DATA_ITEM_END);
		m_pData1->SetYData(iIndex + m_nProbe + INT_USE_DEFAULT_DATA, py, CHART_DATA_ITEM_END);
		m_pTextCell[iIndex]->SetStrings(MM);

		m_pTextCellStatus[iIndex]->SetStrings(MM);
	}
	
	SetRepaint(TRUE);
}

void CChartOrbit::SetGlassCellLayout(int iIndex, CString strCellName, double dSX, double dSY, double dEX, double dEY)
{
	SetRepaint(FALSE);
	
	m_pData1->SetNPoints(m_nProbe + INT_USE_DEFAULT_DATA + iIndex, CHART_DATA_ITEM_END);
	
	double* px = new double[CHART_DATA_ITEM_END];
	double* py = new double[CHART_DATA_ITEM_END];
	
	double dXMin, dXMax, dYMin, dYMax;
	if(dSX > dEX)
	{
		dXMin = dEX;
		dXMax = dSX;
	}
	else
	{
		dXMin = dSX;
		dXMax = dEX;
	}
	if(dSY > dEY)
	{
		dYMin = dEY;
		dYMax = dSY;
	}
	else
	{
		dYMin = dSY;
		dYMax = dEY;
	}
	
	px[0] = dXMin;			py[0] = dYMax;
	px[1] = dXMax;			py[1] = dYMax;
	px[2] = dXMax;			py[2] = dYMin;
	px[3] = dXMin;			py[3] = dYMin;
	px[4] = dXMin;			py[4] = dYMax;
	px[5] = XRT_HUGE_VAL;	py[5] = XRT_HUGE_VAL;
	
	m_pData1->SetXData(m_nProbe + INT_USE_DEFAULT_DATA + iIndex, px, CHART_DATA_ITEM_END);
	m_pData1->SetYData(m_nProbe + INT_USE_DEFAULT_DATA + iIndex, py, CHART_DATA_ITEM_END);

	delete[] px;
	delete[] py;
	px = NULL;
	py = NULL;
	
	TCHAR* pStrAxis[2];
	m_pTextCell[iIndex]->SetAttachValueX( (dXMax + dXMin)/2);
	m_pTextCell[iIndex]->SetAttachValueY(dYMax - 20);
	pStrAxis[0] = (TCHAR*)(LPCTSTR)strCellName;
	pStrAxis[1] = NULL;
	
	HFONT hFont;
	if(m_iChart == 0)
	{
		hFont = CreateFont(12, 0, 0, 0, 0, /* Set only Size & */
			0, 0, 0, 0, 0, 0, 0, 0, _T("Tahoma")); /* Typeface */
	}
	else
	{
		hFont = CreateFont(18, 0, 0, 0, 0, /* Set only Size & */
			0, 0, 0, 0, 0, 0, 0, 0, _T("Tahoma")); /* Typeface */
	}
	
	m_pTextCell[iIndex]->SetFont(hFont);
	m_pTextCell[iIndex]->SetForegroundColor(CLR_GRAY);
	m_pTextCell[iIndex]->SetStrings(pStrAxis);
	DeleteObject(hFont);
	
	
	SetRepaint(TRUE);
}


//YR 070312 22:33 - Chart message 이용 
//	Flickering 없애려면 Bitmap 에 그린 후 copy
LRESULT CChartOrbit::OnOC2Repaint(HDC hdc)
{
	if (FALSE == m_bInitSetup)
		return 0;

	XrtMapResult map;	//, map2;
	double* px = m_px1;
	double* py = m_ppy1[0];

	int piNext[4] = { 1, 2, 3, 0 };
	int piPrev[4] = { 3, 0, 1, 2 };

	// Panel Draw
	if (m_bPanelDraw)
	{
		for (int i = 0; i < m_nPanelNx; i++)
		{
			double x1 = m_dPanelX0 + (m_dPanelLx + m_dPanelDx) * i;
			double x2 = m_dPanelX0 - m_dPanelDx + (m_dPanelLx + m_dPanelDx) * (i + 1);
			for (int j = 0; j < m_nPanelNy; j++)
			{
				double y1 = m_dPanelY0 + (m_dPanelLy + m_dPanelDy) * j;
				double y2 = m_dPanelY0 - m_dPanelDy + (m_dPanelLy + m_dPanelDy) * (j + 1);

				int ix[4], iy[4];
				Unmap(XRT_DATASET1, x1, y1, &map);	ix[0] = ix[3] = map.pix_x;	iy[0] = iy[1] = map.pix_y;
				Unmap(XRT_DATASET1, x2, y2, &map);	ix[1] = ix[2] = map.pix_x;	iy[2] = iy[3] = map.pix_y;

				for (int k = 0; k < 4; k++)
				{
					::MoveToEx(hdc, ix[k], iy[k], NULL);
					::LineTo(hdc, ix[piNext[k]], iy[piNext[k]]);
				}
			}
		}
	}

	CRect rect;
	GetParent()->GetWindowRect(&rect);

	// Glass Size X, Y	/////////////////////////////////////////////////
	CString strGlassSize;
	if (m_bInvertXY == TRUE)
		strGlassSize.Format(_T("Glass Size( Y:%.1f, X:%.1f )"), (fabs(m_dXMax) + fabs(m_dXMin)), (fabs(m_dYMax) + fabs(m_dYMin)));
	else
		strGlassSize.Format(_T("Glass Size( X:%.1f, Y:%.1f )"), (fabs(m_dXMax) + fabs(m_dXMin)), (fabs(m_dYMax) + fabs(m_dYMin)));
	::TextOut(hdc, 5, 20, strGlassSize, strGlassSize.GetLength());

	// jsyoon 20170428 add
	CString strInfo;
	int i = 0, nCnt = 0;
	::SetTextColor(hdc, CLR_WHITE);
	
	// Vacuum 
	if( m_bUseGlassVcm == TRUE)
	{
		for (i=0; i<m_nCntGlassVcm; i++)
		{
			strInfo.Format(_T("VAC(%d) = %.2f"), i+1, m_dGlassVcm[i]);
			::TextOut(hdc, 40,40+14* nCnt, strInfo, strInfo.GetLength());
			nCnt++;
		}
	}

	// Fixture Time
	if (m_bUseFixtuerTime == TRUE)
	{
		TCHAR* szName[4] = { _T("FRONT"),_T("RIGHT"),_T("BACK"),_T("LEFT")};
		if (m_nCntFixtuerTime == 8)
		{
			for (i = 0; i<m_nCntFixtuerTime/2; i++)
			{
				strInfo.Format(_T("FIX %s 01 FWD = %.2f"), szName[i], m_dFixtuerTimeF[2*i]);
				::TextOut(hdc, 40, 40 + 14 * nCnt, strInfo, strInfo.GetLength());
				nCnt++;

				strInfo.Format(_T("FIX %s 01 REV = %.2f"), szName[i], m_dFixtuerTimeR[2*i]);
				::TextOut(hdc, 40, 40 + 14 * nCnt, strInfo, strInfo.GetLength());
				nCnt++;

				strInfo.Format(_T("FIX %s 02 FWD = %.2f"), szName[i], m_dFixtuerTimeF[2*i+1]);
				::TextOut(hdc, 40, 40 + 14 * nCnt, strInfo, strInfo.GetLength());
				nCnt++;

				strInfo.Format(_T("FIX %s 02 REV = %.2f"), szName[i], m_dFixtuerTimeR[2*i+1]);
				::TextOut(hdc, 40, 40 + 14 * nCnt, strInfo, strInfo.GetLength());
				nCnt++;
			}
		}
		else
		{
			for (i = 0; i<m_nCntFixtuerTime; i++)
			{
				strInfo.Format(_T("FIX %s FWD = %.2f"), szName[i], m_dFixtuerTimeF[i]);
				::TextOut(hdc, 40, 40 + 14 * nCnt, strInfo, strInfo.GetLength());
				nCnt++;

				strInfo.Format(_T("FIX %s REV = %.2f"), szName[i], m_dFixtuerTimeR[i]);
				::TextOut(hdc, 40, 40 + 14 * nCnt, strInfo, strInfo.GetLength());
				nCnt++;
			}
		}
	}


	
	// Arrow X, Y	//////////////////////////////////////////////////////
// 	CString strX="";
// 	CString strY="";
// 	CString strDirUp="";
// 	CString strDirDown="";
// 	CString strDirRight="";
// 	CString strDirLeft="";
	
// 	strX = "X";
// 	strY = "Y";
// 	strDirUp	= "↑";
// 	strDirDown  = "↓";
// 	strDirRight = "→";
// 	strDirLeft  = "←";
	
// 	ScreenToClient(&rect);
// 	::SetTextColor(hdc,CLR_WHITE);

//	::TextOut(hdc, rect.CenterPoint().x, rect.CenterPoint().y+7, strDirRight, strDirRight.GetLength());
//	::TextOut(hdc, rect.CenterPoint().x+10, rect.CenterPoint().y+20, strX, strX.GetLength());
//	::TextOut(hdc, rect.CenterPoint().x-7, rect.CenterPoint().y, strDirUp, strDirUp.GetLength());
//	::TextOut(hdc, rect.CenterPoint().x-10, rect.CenterPoint().y-10, strY, strY.GetLength());

// 	int iX = 0, iY = 0;
// 	iX = rect.CenterPoint().x * 2;
// 	iY = rect.CenterPoint().y * 2;
// 	::TextOut(hdc, iX-10, rect.CenterPoint().y-10, strX, strX.GetLength());
// 	::TextOut(hdc, rect.CenterPoint().x, iY-10, strY, strY.GetLength());
// 	//////////////////////////////////////////////
// 	
// 	//
// 	::SelectObject(hdc, hOldPen);
// 	::DeleteObject(hPen);

	return 1;
}

void CChartOrbit::UpdateArea(int iMarker, int SetNum, double X, double Y)
{
	// Meas Area
	m_chartMarker[iMarker]->SetXY(X+16, Y/*X*/+16);
	m_chartMarker[iMarker]->SetAreaNum(SetNum);
	m_chartMarker[iMarker]->SetLayer(0);
	m_chartMarker[iMarker]->SetShow(TRUE);
};

BOOL CChartOrbit::SetGlassCell(int nIndex, CStringArray* strCellName, double* ppSX, double* ppSY, double* ppEX, double* ppEY, COLORREF* ppColor)
{
	SetRepaint(FALSE);

	int iIndex=0;

	if( nIndex > 0)
	{
		for (iIndex=0; iIndex<nIndex; iIndex++)
		{
			if( iIndex >= m_nMaxCell)	// jsyoon 20200103 add
				continue;

			m_pData1->SetNPoints(m_nProbe + INT_USE_DEFAULT_DATA + iIndex, CHART_DATA_ITEM_END);
			
			double* px = new double[CHART_DATA_ITEM_END];
			double* py = new double[CHART_DATA_ITEM_END];
			
			double dSX = 0, dEX = 0, dSY = 0, dEY = 0;
			double dXMin = 0, dXMax = 0, dYMin = 0, dYMax = 0;
			COLORREF color;

			dSX = ppSX[iIndex];
			dSY = ppSY[iIndex];
			dEX = ppEX[iIndex];
			dEY = ppEY[iIndex];
			color = ppColor[iIndex];

			if(dSX > dEX)
			{
				dXMin = dEX;
				dXMax = dSX;
			}
			else
			{
				dXMin = dSX;
				dXMax = dEX;
			}
			if(dSY > dEY)
			{
				dYMin = dEY;
				dYMax = dSY;
			}
			else
			{
				dYMin = dSY;
				dYMax = dEY;
			}

			
			

			/*
			double dSize = 0, dW = 0, dH = 0;
			dW = fabs((dXMax - dXMin) / 4);
			dH = fabs((dYMax - dYMin) / 4);

			if (dW < dH)
				dSize = dW;
			else
				dSize = dH;
			
			px[0] = dXMin + dW;			py[0] = dYMax - dH;
			px[1] = dXMax - dW;			py[1] = dYMax - dH;
			px[2] = dXMax - dW;			py[2] = dYMin + dH;
			px[3] = dXMin + dW;			py[3] = dYMin + dH;
			px[4] = dXMin + dW;			py[4] = dYMax - dH;
			px[5] = XRT_HUGE_VAL;			py[5] = XRT_HUGE_VAL;
			
 			XrtDataStyle* pds = GetNthDataStyle(m_nProbe + INT_USE_DEFAULT_DATA + iIndex);
 			pds->fpat = XRT_FPAT_SOLID;
 			pds->point = XRT_POINT_BOX;
 			pds->color = CLR_GRAY;
 			pds->pcolor = CLR_GRAY;
			pds->psize = dSize * 4;
 			pds->width = dSize * 4;
 			SetNthDataStyle(m_nProbe + INT_USE_DEFAULT_DATA + iIndex, pds);*/

			// jsyoon 2022.08.09 change
			//double dCenX = 0, dCenY = 0, dSizeX = 0, dSizeY = 0, dSize = 0;
			//dCenX = (dXMax + dXMin) / 2;
			//dCenY = (dYMax + dYMin) / 2;

			//dSizeX = fabs((dXMax - dXMin));
			//dSizeY = fabs((dYMax - dYMin));


			//dSize = dSizeX;
			//px[0] = dCenX;			py[0] = dYMin;
			//px[1] = dCenX;			py[1] = dYMax;

			///*if (dSizeX < dSizeY)
			//{
			//	dSize = dSizeX;
			//	px[0] = dCenX;			py[0] = dYMin;
			//	px[1] = dCenX;			py[1] = dYMax;
			//}
			//else
			//{
			//	dSize = dSizeY;
			//	px[0] = dXMin + (dSize / 2);			py[0] = dCenY;
			//	px[1] = dXMax - (dSize / 2);			py[1] = dCenY;
			//}*/
			//
			//px[2] = XRT_HUGE_VAL;	py[2] = XRT_HUGE_VAL;
			//px[3] = XRT_HUGE_VAL;	py[3] = XRT_HUGE_VAL;
			//px[4] = XRT_HUGE_VAL;	py[4] = XRT_HUGE_VAL;
			//px[5] = XRT_HUGE_VAL;	py[5] = XRT_HUGE_VAL;


			px[0] = dXMin;			py[0] = dYMax;
			px[1] = dXMax;			py[1] = dYMax;
			px[2] = dXMax;			py[2] = dYMin;
			px[3] = dXMin;			py[3] = dYMin;
			px[4] = dXMin;			py[4] = dYMax;
			px[5] = XRT_HUGE_VAL;	py[5] = XRT_HUGE_VAL;

			XrtDataStyle* pds = GetNthDataStyle(m_nProbe + INT_USE_DEFAULT_DATA + iIndex);
			pds->lpat = XRT_LPAT_SOLID;
			pds->point = XRT_POINT_BOX; 
			pds->color = color;
			pds->pcolor = color;
			pds->psize = 1;
			pds->width = 3;
		//	pds->width = dSizeX * 0.5; // / 3;
			SetNthDataStyle(m_nProbe + INT_USE_DEFAULT_DATA + iIndex, pds);

			m_pData1->SetXData(m_nProbe + INT_USE_DEFAULT_DATA + iIndex, px, CHART_DATA_ITEM_END);
			m_pData1->SetYData(m_nProbe + INT_USE_DEFAULT_DATA + iIndex, py, CHART_DATA_ITEM_END);

			delete[] px;
			delete[] py;
			
			if (GetYAxisReversed() == TRUE)	// jsyoon 2022.07.18 add
			{
				TCHAR* pStrAxis[2];
				m_pTextCell[iIndex]->SetAttachValueX((dXMin + dXMax) / 2);
				m_pTextCell[iIndex]->SetAttachValueY((dYMin + dYMax) / 2 - 30);
				pStrAxis[0] = (TCHAR*)(LPCTSTR)strCellName->GetAt(2 * iIndex);
				pStrAxis[1] = NULL;

				HFONT hFont;
				if (m_iChart == 0)
				{
					if (nIndex < 200)
					{
						hFont = CreateFont(12, 0, 0, 0, 0, /* Set only Size & */
							0, 0, 0, 0, 0, 0, 0, 0, _T("Tahoma")); /* Typeface */
					}
					else
					{
						hFont = CreateFont(8, 0, 0, 0, 0, /* Set only Size & */
							0, 0, 0, 0, 0, 0, 0, 0, _T("Tahoma")); /* Typeface */
					}
				}
				else
				{
					hFont = CreateFont(18, 0, 0, 0, 0, /* Set only Size & */
						0, 0, 0, 0, 0, 0, 0, 0, _T("Tahoma")); /* Typeface */
				}

				m_pTextCell[iIndex]->SetFont(hFont);
				m_pTextCell[iIndex]->SetForegroundColor(CLR_WHITE);
				m_pTextCell[iIndex]->SetStrings(pStrAxis);


				m_pTextCellStatus[iIndex]->SetAttachValueX((dXMin + dXMax) / 2);
				m_pTextCellStatus[iIndex]->SetAttachValueY((dYMin + dYMax) / 2);
				pStrAxis[0] = (TCHAR*)(LPCTSTR)strCellName->GetAt(2 * iIndex + 1);
				pStrAxis[1] = NULL;

				m_pTextCellStatus[iIndex]->SetFont(hFont);
				m_pTextCellStatus[iIndex]->SetForegroundColor(color);
			//	m_pTextCellStatus[iIndex]->SetForegroundColor(CLR_BLACK);
				m_pTextCellStatus[iIndex]->SetStrings(pStrAxis);

				DeleteObject(hFont);
			}
			else
			{
				TCHAR* pStrAxis[2];
				m_pTextCell[iIndex]->SetAttachValueX((dXMin + dXMax) / 2);
				m_pTextCell[iIndex]->SetAttachValueY((dYMin + dYMax) / 2 + 30);
				pStrAxis[0] = (TCHAR*)(LPCTSTR)strCellName->GetAt(2 * iIndex);
				pStrAxis[1] = NULL;

				HFONT hFont;
				if (m_iChart == 0)
				{
					if (nIndex < 200)
					{
						hFont = CreateFont(12, 0, 0, 0, 0, /* Set only Size & */
							0, 0, 0, 0, 0, 0, 0, 0, _T("Tahoma")); /* Typeface */
					}
					else
					{
						hFont = CreateFont(8, 0, 0, 0, 0, /* Set only Size & */
							0, 0, 0, 0, 0, 0, 0, 0, _T("Tahoma")); /* Typeface */
					}
				}
				else
				{
					hFont = CreateFont(18, 0, 0, 0, 0, /* Set only Size & */
						0, 0, 0, 0, 0, 0, 0, 0, _T("Tahoma")); /* Typeface */
				}

				m_pTextCell[iIndex]->SetFont(hFont);
				m_pTextCell[iIndex]->SetForegroundColor(CLR_WHITE);
				m_pTextCell[iIndex]->SetStrings(pStrAxis);


				m_pTextCellStatus[iIndex]->SetAttachValueX((dXMin + dXMax) / 2);
				m_pTextCellStatus[iIndex]->SetAttachValueY((dYMin + dYMax) / 2);
				pStrAxis[0] = (TCHAR*)(LPCTSTR)strCellName->GetAt(2 * iIndex + 1);
				pStrAxis[1] = NULL;

				m_pTextCellStatus[iIndex]->SetFont(hFont);
			//	m_pTextCellStatus[iIndex]->SetForegroundColor(CLR_BLACK);
				m_pTextCellStatus[iIndex]->SetForegroundColor(color);
				m_pTextCellStatus[iIndex]->SetStrings(pStrAxis);

				DeleteObject(hFont);
			}
		}
	}
	else
	{
		;
	}

	SetRepaint(TRUE);
	
	return TRUE;
}

BOOL CChartOrbit::UpdateChart(BOOL bRepaint, int nPlot, int nPts, double** ppX, double** ppY)
{
	SetRepaint(FALSE);

	// jsyoon 20121008 add
	for (int i = 0; i < INT_MEASE_AREA_MAX; i++)
	{
		m_chartMarker[i]->SetLayer(0);
		m_chartMarker[i]->SetShow(FALSE);
	}

	if (nPts > 0)
	{
		int nPtsAll = nPts * nPlot;

		double* pX = new double[nPtsAll];
		double* pY = new double[nPtsAll];

		int i, j, k, iPlot;
		
		k = 0;
		for (j = 0; j < nPlot; j++)
		{
			for (i = 0; i < nPts; i++)
			{
			//	pX[k++] = ppX[j][i];
//				if( ppX[j][i] < XRT_HUGE_VAL )	// jsyoon 20191007 remove
				{
					pX[k++] = ppX[j][i];
				}
			}
		}
		
		for (iPlot = 0; iPlot < nPlot; iPlot++)
		{
			k = 0;
			for (j = 0; j < nPlot; j++)
			{
				for (i = 0; i < nPts; i++)
					pY[k++] = XRT_HUGE_VAL;
			}
				
			k = nPts * iPlot;
			for (i = 0; i < nPts; i++)
			{
//				pY[k++] = ppY[iPlot][i];
//				if( ppY[iPlot][i] < XRT_HUGE_VAL)		// jsyoon 20191007 remove
					pY[k++] = ppY[iPlot][i];
			}

			m_pData2->SetNPoints(iPlot	,	nPtsAll);
			m_pData2->SetXData  (iPlot, pX, nPtsAll);
			m_pData2->SetYData  (iPlot, pY, nPtsAll);
// 			m_pData2->SetNPoints(iPlot, nPtsAll);
// 			m_pData2->SetXData  (iPlot, pX, nPtsAll);
// 			m_pData2->SetYData  (iPlot, pY, nPtsAll);

			if(iPlot ==0)
			{
				m_chartMarker[0]->SetXY(pX[0], pY[0]+30);
				m_chartMarker[0]->SetAreaNum(1);
				m_chartMarker[0]->SetLayer(0);
				m_chartMarker[0]->SetShow(TRUE);
				
// 				m_chartMarker[1]->SetXY(pX[k-1]+16, pY[k-1]+16);
// 				m_chartMarker[1]->SetAreaNum(nPtsAll-1);
// 				m_chartMarker[1]->SetLayer(0);
// 				m_chartMarker[1]->SetShow(TRUE);
			}
			
			if(iPlot == nPlot-1)
			{
// 				m_chartMarker[2]->SetXY(pX[0]+16, pY[nPts]+16);
// 				m_chartMarker[2]->SetAreaNum(2);
// 				m_chartMarker[2]->SetLayer(0);
// 				m_chartMarker[2]->SetShow(TRUE);
				
				m_chartMarker[3]->SetXY(pX[k-1], pY[k-1]+30);
				m_chartMarker[3]->SetAreaNum(nPtsAll);
				m_chartMarker[3]->SetLayer(0);
				m_chartMarker[3]->SetShow(TRUE);
			}
		}

		delete[] pX;
		delete[] pY;
	}
	else
	{
		nPts = 1;
		double x = 0.0;
		double y = XRT_HUGE_VAL;	// Set hole data
		
		for (int iPlot = 0; iPlot < nPlot; iPlot++)
		{
			m_pData2->SetNPoints(iPlot, nPts);
			m_pData2->SetXData  (iPlot, &x, nPts);
			m_pData2->SetYData  (iPlot, &y, nPts);
		}
	}

	if (bRepaint) SetRepaint(TRUE);

	return TRUE;
}

void CChartOrbit::GetConfig()
{
	TCHAR szValue[MAX_PATH];
	ZeroMemory(szValue,MAX_PATH);

	CString szAppPath, szFile;
	TCHAR szBuff[_MAX_PATH];
	memset(szBuff, 0, MAX_PATH);
	::GetModuleFileName(::AfxGetApp()->m_hInstance, szBuff, _MAX_PATH);
	TCHAR* pch = _tcsrchr(szBuff, _T('\\'));
	szAppPath = CString(szBuff, pch - szBuff + 1);

	szFile.Format(_T("%s%s"), szAppPath, _T("Config\\DEFAULT\\Config.ini"));

	GetPrivateProfileString (_T("Graph Margin"),_T("Left"),_T("30"), szValue,6,szFile);
	m_nGrpMarginLeft = _tstoi(szValue);

	GetPrivateProfileString (_T("Graph Margin"), _T("Right"), _T("30"), szValue,6,szFile);
	m_nGrpMarginRight = _tstoi(szValue);

	GetPrivateProfileString (_T("Graph Margin"), _T("Top"), _T("30"), szValue,6,szFile);
	m_nGrpMarginTop = _tstoi(szValue);

	GetPrivateProfileString (_T("Graph Margin"), _T("Bottom"), _T("30"), szValue,6,szFile);
	m_nGrpMarginBottom = _tstoi(szValue);

	// jsyoon 20200103
	GetPrivateProfileString (_T("Graph Margin"), _T("CELL_MAX"), _T("150"), szValue,6,szFile);
	m_nMaxCell = _tstoi(szValue);

	GetPrivateProfileString (_T("Glass Size"), _T("InvertXY"), _T("0"), szValue,6,szFile);
	m_bInvertXY = _tstoi(szValue);
}

void CChartOrbit::SetAxisShow(BOOL bVal)
{
	SetXAxisShow(bVal);
	SetYAxisShow(bVal);
	
	SetRepaint(TRUE);
}

void CChartOrbit::SetGlassVacuum(double* pdVacuum, BOOL bUse, int nCnt)
{
	m_bUseGlassVcm = bUse;
	m_nCntGlassVcm = nCnt;
	memcpy(&m_dGlassVcm, pdVacuum, sizeof(double)*10);
}

void CChartOrbit::Prepare(int iIndex)
{
	m_iChart = iIndex;
}

void CChartOrbit::ClearResultText()
{
	int nCnt = 0;
	if( !m_bUpdateResult )
		return;

	if( m_pTextMeasResult == NULL)
	{
		return;
	}

	SetRepaint(FALSE);

	for(int i = 0; i < INT_MAX_MEAS_RESULT; i++)
	{
		if(m_pTextMeasResult[i]->GetIsShowing() )	// jsyoon 20140122 change
		{
			m_pTextMeasResult[i]->SetIsShowing(FALSE);
			nCnt = 0;
		}
		else
		{
			nCnt++;
		}

		// 		if( nCnt > 50 )	// jsyoon 20140123 add
		// 		{
		// 			return;
		// 		}
	}

	SetRepaint(TRUE);
}

void CChartOrbit::SetParentWnd(CWnd *pWnd)
{
	m_pParent = pWnd;
}

void CChartOrbit::SetCurPosA(int nProbe, BOOL bDraw, double* px, double* py)
{
	m_bCurPosDraw = bDraw;
	
	SetRepaint(FALSE);

	int i=0;
	CString strHead;	
	double	dCurPosX[10];
	double	dCurPosY[10];
	ZeroMemory(&dCurPosX, sizeof(dCurPosX));
	ZeroMemory(&dCurPosY, sizeof(dCurPosY));

	for(i=0; i<m_nProbe; i++)
	{
		dCurPosX[i] = XRT_HUGE_VAL;
		dCurPosY[i] = XRT_HUGE_VAL;

		if( i< nProbe )
		{
			dCurPosX[i] = px[i];
			dCurPosY[i] = (m_bCurPosDraw) ? py[i] : XRT_HUGE_VAL;

			if(dCurPosX[i] < m_dMcXMax && dCurPosX[i] > m_dMcXMin && dCurPosY[i] < m_dMcYMax && dCurPosY[i] > m_dMcYMin)	// jsyoon 201401015 change
			{
				m_pData1->SetXElement(i + INT_USE_DEFAULT_DATA, 0, dCurPosX[i]);
				m_pData1->SetYElement(i + INT_USE_DEFAULT_DATA, 0, dCurPosY[i]);

				strHead.Format(_T("H%d"), i+1);

				TCHAR* pStr[2];
				pStr[0] = (TCHAR*)(LPCTSTR)strHead;
				pStr[1] = NULL;

				HFONT hFont;
				if(m_iChart == 0 || m_iChart == 3)
				{
					hFont = CreateFont(18, 0, 0, 0, 0, /* Set only Size & */
						0, 0, 0, 0, 0, 0, 0, 0, _T("Tahoma")); /* Typeface */
				}
				else
				{
					hFont = CreateFont(18, 0, 0, 0, 0, /* Set only Size & */
						0, 0, 0, 0, 0, 0, 0, 0, _T("Tahoma")); /* Typeface */
				}

				m_pTextItem[CHART_TEXT_H1+i]->SetFont(hFont);
				m_pTextItem[CHART_TEXT_H1+i]->SetForegroundColor(CLR_YELLOW);
				m_pTextItem[CHART_TEXT_H1+i]->SetBackgroundColor(CLR_RED);
				m_pTextItem[CHART_TEXT_H1+i]->SetStrings(pStr);

				m_pTextItem[CHART_TEXT_H1+i]->SetAttachValueX(dCurPosX[i]);
				m_pTextItem[CHART_TEXT_H1+i]->SetAttachValueY(dCurPosY[i]);

				DeleteObject(hFont);
			}
			else
			{
				m_pData1->SetXElement(i + INT_USE_DEFAULT_DATA, 0, dCurPosX[i]);
				m_pData1->SetYElement(i + INT_USE_DEFAULT_DATA, 0, dCurPosY[i]);
				CString strHead;	strHead.Format(_T(""));
				TCHAR* pStr[2];
				pStr[0] = (TCHAR*)(LPCTSTR)strHead;
				pStr[1] = NULL;
				m_pTextItem[CHART_TEXT_H1+i]->SetStrings(pStr);
			}
		}
		else
		{
			m_pData1->SetXElement(i + INT_USE_DEFAULT_DATA, 0, dCurPosX[i]);
			m_pData1->SetYElement(i + INT_USE_DEFAULT_DATA, 0, dCurPosY[i]);
			CString strHead;	strHead.Format(_T(""));
			TCHAR* pStr[2];
			pStr[0] = (TCHAR*)(LPCTSTR)strHead;
			pStr[1] = NULL;
			m_pTextItem[CHART_TEXT_H1+i]->SetStrings(pStr);
		}
	}

	SetRepaint(TRUE);
}

void CChartOrbit::SetOriginLine (double dXMax, double dXMin, double dYMax, double dYMin) //이거 이용해서 원점 선 그리기
{	
	SetRepaint(FALSE);

	double* px = new double[CHART_DATA_ITEM_END];
	double* py = new double[CHART_DATA_ITEM_END];

	px[0] = dXMin;			py[0] = dYMax;
	px[1] = dXMax;			py[1] = dYMax;
	px[2] = dXMax;			py[2] = dYMin;
	px[3] = dXMin;			py[3] = dYMin;
	px[4] = dXMin;			py[4] = dYMax;
	px[5] = XRT_HUGE_VAL;	py[5] = XRT_HUGE_VAL;

	m_pData1->SetXData (2, px,	CHART_DATA_ITEM_END);
	m_pData1->SetYData (2, py,	CHART_DATA_ITEM_END);

	//m_dCardWidth = dXMax - dXMin;

	delete[] px;
	delete[] py;
	px = NULL;
	py = NULL;

	SetRepaint(TRUE);
}

BOOL CChartOrbit::SetGlassMeasPos(int nIndex, double* ppSX, double* ppSY, double* ppEX, double* ppEY)
{
	SetRepaint(FALSE);

	int iIndex = 0;

	if (nIndex > 0)
	{
		for (iIndex = 0; iIndex<nIndex; iIndex++)
		{
			if (iIndex > INT_USE_MEASPOS_DATA)	// jsyoon 20200103 add
				continue;

			m_pData1->SetNPoints(m_nProbe + INT_USE_DEFAULT_DATA + m_nMaxCell + iIndex, CHART_DATA_ITEM_END);

			double* px = new double[CHART_DATA_ITEM_END];
			double* py = new double[CHART_DATA_ITEM_END];

			double dSX = 0, dEX = 0, dSY = 0, dEY = 0;
			double dXMin = 0, dXMax = 0, dYMin = 0, dYMax = 0;

			dSX = ppSX[iIndex];
			dSY = ppSY[iIndex];
			dEX = ppEX[iIndex];
			dEY = ppEY[iIndex];

			if (dSX > dEX)
			{
				dXMin = dEX;
				dXMax = dSX;
			}
			else
			{
				dXMin = dSX;
				dXMax = dEX;
			}
			if (dSY > dEY)
			{
				dYMin = dEY;
				dYMax = dSY;
			}
			else
			{
				dYMin = dSY;
				dYMax = dEY;
			}

			px[0] = dXMin;			py[0] = dYMax;
			px[1] = dXMax;			py[1] = dYMax;
			px[2] = dXMax;			py[2] = dYMin;
			px[3] = dXMin;			py[3] = dYMin;
			px[4] = dXMin;			py[4] = dYMax;
			px[5] = XRT_HUGE_VAL;	py[5] = XRT_HUGE_VAL;

			m_pData1->SetXData(m_nProbe + INT_USE_DEFAULT_DATA + m_nMaxCell + iIndex, px, CHART_DATA_ITEM_END);
			m_pData1->SetYData(m_nProbe + INT_USE_DEFAULT_DATA + m_nMaxCell + iIndex, py, CHART_DATA_ITEM_END);

			delete[] px;
			delete[] py;
		}
	}

	SetRepaint(TRUE);

	return TRUE;
}

void	CChartOrbit::SetFixtureMoveTime(double* pVacuum, BOOL bUse, int nCnt)
{
	m_bUseFixtuerTime = bUse;
	m_nCntFixtuerTime = nCnt;

	for (int i = 0; i < nCnt; i++)
	{
		m_dFixtuerTimeF[i] = pVacuum[(2*i)];
		m_dFixtuerTimeR[i] = pVacuum[(2*i)+1];
	}
}