// ChartSpec.cpp : implementation file
//

#include "pch.h"
//#include "MetroSys.h"
#include "ChartSpec.h"
#include "SetScaleDlg.h"

#include "och2dmfc.h"
#include "resource.h"


#define RGB_WHITE			RGB(255, 255, 255)
#define RGB_BLACK			RGB(0, 0, 0)
#define RGB_RED				RGB(255, 0, 0)
#define RGB_GREEN			RGB(0, 255, 0)
#define RGB_BLUE			RGB(0, 0, 255)
#define RGB_CYAN			RGB(0, 255, 255)
#define RGB_MAGENTA			RGB(255, 0, 255)
#define RGB_YELLOW			RGB(255, 255, 0)
#define RGB_DEEP_GREEN		RGB(, 128, 0)
#define	RGB_LIGHT_GRAY		RGB(192, 192, 192)
#define	RGB_DARK_GRAY		RGB(70, 70, 70)
#define	RGB_ORANGE			RGB(255, 128, 0)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DECLARE_USER_MESSAGE(UWM_CHARTRESIZE)
DECLARE_USER_MESSAGE(UWM_CHARTPICK)

/////////////////////////////////////////////////////////////////////////////
// CChartSpec

CChartSpec::CChartSpec()
{
	m_pChartData  = new CChart2DData(XRT_DATA_ARRAY, 1, 1);
	m_bMapCIM		= TRUE;

	m_pChartMenu = NULL;
}

CChartSpec::~CChartSpec()
{
	delete m_pChartData;

	if (m_pChartMenu)	
	{
		m_pChartMenu->DestroyMenu();
		delete m_pChartMenu;
	}	
}


BEGIN_MESSAGE_MAP(CChartSpec, CChart2D)
	//{{AFX_MSG_MAP(CChartSpec)
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_CHAR()
	ON_WM_GETDLGCODE()
	ON_WM_CREATE()
	ON_COMMAND(IDM_CHART_SET_SIZE, OnChartSetSize)
	ON_COMMAND(IDM_CHART_AUTO_SIZE, OnChartAutoSize)
	ON_COMMAND(IDM_COPY_CLIPBOARD,	OnCopyClipboard)
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
	
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CChartSpec message handlers

void CChartSpec::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// Because mouse click on this chart does not change active view to AutoStageView,
	// I set active view explicitly.
	//((CFrameWnd*)gl_pMainFrame)->SetActiveView((CView*)GetParent());

	SetFocus();		// Get key focus

	CChart2D::OnLButtonDown(nFlags, point);
}

void CChartSpec::OnRButtonDown(UINT /*nFlags*/, CPoint point) 
{
	// Because mouse click on this chart does not change active view to AutoStageView,
	// I set active view explicitly.
	//((CFrameWnd*)gl_pMainFrame)->SetActiveView((CView*)GetParent());

	SetFocus();		// Get key focus
	
	if (m_pChartMenu)
	{
		CMenu *subMenu;
		subMenu = m_pChartMenu->GetSubMenu(0);

		CRect rect = CRect(point.x, point.y, point.x, point.y);
		ClientToScreen(&rect); 
		subMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rect.left, rect.top, this);	
	}

	//CChart2D::OnRButtonDown(nFlags, point);
}

// bMapCIM = TRUE  : called from VTMapCIM
//         = FALSE :             VThickCM
void CChartSpec::InitialSetup(int nMeasPts, double* pdXMeas, double dYMax, BOOL bMapCIM)
{
//	CFrameWnd*	 pFrame = (CFrameWnd*) gl_pMainFrame;
//	CVTMapCIMDoc* pVTDoc  = (CVTMapCIMDoc*) pFrame->GetActiveDocument();

	m_nMeasPts = nMeasPts;
	m_bMapCIM = bMapCIM;

	//
	SetRepaint(FALSE);

	//
	SetType (XRT_TYPE_PLOT);

	// Background color
	COLORREF clrBgnd = ::GetSysColor(COLOR_BTNFACE);
	SetBackgroundColor(clrBgnd);
	SetGraphBackgroundColor		(RGB(223, 234, 253));
	SetDataAreaBackgroundColor	(RGB_WHITE);
	SetForegroundColor			(RGB(90, 90, 90));

	// Border
	SetBorder(XRT_BORDER_3D_IN);	// XRT_BORDER_PLAIN, XRT_BORDER_3D_OUT,XRT_BORDER_PLAIN
	SetBorderWidth(1);		// 1
	SetGraphBorder(XRT_BORDER_NONE);
	SetAxisBoundingBox(TRUE);

	// Header
	TCHAR *header[] = { _T("Red-Dark, Blue-Ref, Green-Sample"), NULL };
    SetHeaderStrings(header);
	SetPropString(XRT_HEADER_FONT, _T("Tahoma, 10,")); // "MS Sans Serif, 11, Bold"

	// Footer
	//TCHAR *footer[] = { _T("Filter No."), NULL };
    //SetFooterStrings(footer);
	//SetPropString(XRT_FOOTER_FONT, _T("MS Sans Serif, 10,")); 

	// Axis 
	SetPropString      (XRT_AXIS_TITLE_FONT, _T("Tahoma, 10,"));
	//SetPropString(XRT_AXIS_FONT, _T("MS Sans Serif, 9,"));
	SetPropString(XRT_AXIS_FONT, _T("Tahoma, 8,"));
	SetXAnnoPlacement(XRT_ANNO_MIN);
	SetXOriginPlacement(XRT_ORIGIN_MIN);
	SetYAnnoPlacement(XRT_ANNO_MIN);
	SetYOriginPlacement(XRT_ORIGIN_MIN);

	SetXTitle(_T("Wavelength (nm)"));
	SetYTitle(_T("Intensity (counts)"));
	SetYTitleRotation(XRT_ROTATE_90);
	
	// Data Style
	XrtDataStyle* pds = GetNthDataStyle(0);
	pds->lpat  = XRT_LPAT_SOLID;
	pds->point = (m_bMapCIM) ? XRT_POINT_DOT : XRT_POINT_NONE;		
	pds->color = RGB_RED;				
	pds->pcolor= RGB_RED;				
	pds->psize = 5;				
	SetNthDataStyle(0, pds);

	pds->color = RGB_BLUE;
	pds->pcolor= RGB_BLUE;
	SetNthDataStyle(1, pds);

	pds->color = RGB_GREEN;
	pds->pcolor= RGB_GREEN;
	SetNthDataStyle(2, pds);

	// Grid
	SetXGridUseDefault(TRUE);			// Make the default XY grid lines visible
	SetYGridUseDefault(TRUE);

	pds = GetXGridDataStyle();
	pds->lpat  = XRT_LPAT_SOLID;		// solid line
	pds->point = XRT_POINT_NONE;
	pds->color = RGB_BLACK;
	SetXGridDataStyle(pds);
	SetYGridDataStyle(pds);

	// Layout
	CRect rect;
	GetClientRect(&rect);
	int xIndent = -5;
	SetGraphX(xIndent);					// Graph location
	SetGraphY(0);
	SetGraphWidth(rect.right - xIndent);
	SetGraphHeight(rect.bottom);
	//SetHeaderX(50);				-> Use default. centered by default
	SetHeaderY(3);					// Header location
	//SetFooterX(rect.right / 2 - 16);	// Footer location
	//SetFooterY(rect.bottom - 16);	// Footer location
	//SetGraphMarginLeft(0);		-> Use default
	SetGraphMarginLeft(65);
	SetGraphMarginRight (10);
	SetGraphMarginTop   (25);
	SetGraphMarginBottom(45);
	
	// Data
	// blue 2009.07.23 temp..
	double	xMax=0.0, xMin=0.0;
	if(pdXMeas)
	{
		xMax = xMin = pdXMeas[0];
		for (int i = 0; i < m_nMeasPts; i++)
		{
			if (pdXMeas[i] > xMax) xMax = pdXMeas[i];
			if (pdXMeas[i] < xMin) xMin = pdXMeas[i];
		}
	}

	m_pChartData->SetNSets  (3);
	m_pChartData->SetNPoints(0, m_nMeasPts);
	m_pChartData->SetNPoints(1, m_nMeasPts);
	m_pChartData->SetNPoints(2, m_nMeasPts);

	SetData (*m_pChartData);

	// Range
//	SetXMin(xMin);		// To fit max and min to measured reflectance
//	SetXMax(xMax);
	SetXAxisMin(xMin);
	SetXAxisMax(xMax);
	//SetXNum(5.0);
	SetYAxisMin(0.0);
	//SetYAxisMax(10.0);
	SetYAxisMax(dYMax);
	//
	SetRepaint(TRUE);
}

void CChartSpec::UpdateData(double* pXMeas, double* pDark, double* pRef, double* pSpl, BOOL bRepaint)
{
	SetRepaint(FALSE);

	m_pChartData->SetXData(0, pXMeas, m_nMeasPts);
	m_pChartData->SetYData(0, pDark,  m_nMeasPts);

	m_pChartData->SetXData(1, pXMeas, m_nMeasPts);
	m_pChartData->SetYData(1, pRef,   m_nMeasPts);
	
	m_pChartData->SetXData(2, pXMeas, m_nMeasPts);
	m_pChartData->SetYData(2, pSpl,   m_nMeasPts);

	if (bRepaint) SetRepaint(TRUE);
}

void CChartSpec::OnLButtonDblClk(UINT /*nFlags*/, CPoint /*point*/) 
{
	OnChartSetSize();
	
	//CChart2D::OnLButtonDblClk(nFlags, point);
}

// Not called at all -> fixed by introducing WindowProc
void CChartSpec::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
    switch (nChar)
    {
	// Pressing c copies to clipboard.
	// (Because ctrl-c is mapped as an accelerator key, it is stripped out by framework.
	//  If you insist to adopt ctrl-c, then make appropriate handler in view or frame class)
    case 'C':	
    case 'c':
        OnEditCopy();
		break;
	}
	
	CChart2D::OnChar(nChar, nRepCnt, nFlags);
}

LRESULT CChartSpec::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	return CChart2D::WindowProc(message, wParam, lParam);
}

void CChartSpec::OnEditCopy()
{
	DrawToClipboard(XRT_DRAW_ENHMETAFILE);
}

UINT CChartSpec::OnGetDlgCode() 
{
	// TODO: Add your message handler code here and/or call default
    UINT nCode = DLGC_WANTARROWS | DLGC_WANTCHARS | DLGC_WANTALLKEYS; //

    return nCode;
	//return CWnd::OnGetDlgCode();
}

int CChartSpec::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CChart2D::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	m_pChartMenu = new CMenu();
	m_pChartMenu->LoadMenu(IDR_CHART_CONTEXT); 
	
	return 0;
}

void CChartSpec::OnChartSetSize() 
{
	CSetScaleDlg dlg(this);
	double x1, y1, x2, y2, temp;

	SetRepaint(FALSE);

	x1 = GetXAxisMin();	x2 = GetXAxisMax();
	y1 = GetYAxisMin();	y2 = GetYAxisMax();
	dlg.m_editXMax.Format(_T("%g"), x2);
	dlg.m_editXMin.Format(_T("%g"), x1);
	dlg.m_editYMax.Format(_T("%g"), y2);
	dlg.m_editYMin.Format(_T("%g"), y1);

	if (dlg.DoModal() == IDOK)
	{
		x2 = _ttof((TCHAR*)LPCTSTR(dlg.m_editXMax));
		x1 = _ttof((TCHAR*)LPCTSTR(dlg.m_editXMin));
		y2 = _ttof((TCHAR*)LPCTSTR(dlg.m_editYMax));
		y1 = _ttof((TCHAR*)LPCTSTR(dlg.m_editYMin));

		if (x2 < x1) { temp = x2; x2 = x1; x1 = temp; }
		if (y2 < y1) { temp = y2; y2 = y1; y1 = temp; }

		SetXAxisMin(x1);	SetXAxisMax(x2);
		SetYAxisMin(y1);	SetYAxisMax(y2);
		SetY2AxisMin(y1);	SetY2AxisMax(y2);
	}	
	
	SetRepaint(TRUE);

	GetParent()->PostMessage(UWM_CHARTRESIZE);
}

void CChartSpec::OnChartAutoSize() 
{
	SetRepaint(FALSE);

	SetXAxisMaxUseDefault(TRUE);	SetXAxisMinUseDefault(TRUE);
	SetYAxisMaxUseDefault(TRUE);	SetYAxisMinUseDefault(TRUE);

	SetRepaint(TRUE);

	GetParent()->PostMessage(UWM_CHARTRESIZE);
}

void CChartSpec::OnCopyClipboard()
{
	OnEditCopy();
}

void CChartSpec::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	XrtPickResult xPick;
	XrtRegion xRegion = Pick(XRT_DATASET1, point.x, point.y, &xPick, XRT_XFOCUS | XRT_YFOCUS);

	if (xRegion == XRT_RGN_IN_GRAPH && xPick.point != XRT_RGN_NOWHERE)
		GetParent()->PostMessage(UWM_CHARTPICK, xPick.set, xPick.point);
	
	CChart2D::OnMouseMove(nFlags, point);
}

void CChartSpec::SetXData(int nMeasPts, double* pdXMeas) 
{
	m_nMeasPts = nMeasPts;
	double	xMax=0.0, xMin=0.0;
	if(pdXMeas)
	{
		xMax = xMin = pdXMeas[0];
		for (int i = 0; i < m_nMeasPts; i++)
		{
			if (pdXMeas[i] > xMax) xMax = pdXMeas[i];
			if (pdXMeas[i] < xMin) xMin = pdXMeas[i];
		}
	}

	SetXAxisMin(xMin);
	SetXAxisMax(xMax);
}
