#if !defined(AFX_CHARTORBIT_H__104371B3_0616_4D23_8661_B5579F57AC00__INCLUDED_)
#define AFX_CHARTORBIT_H__104371B3_0616_4D23_8661_B5579F57AC00__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChartOrbit.h : header file
//

//#include "..\\..\\include\\CommonFunc.h"
#include "och2dmfc.h"
#include "ChartMarker.h"

//Added by Jay Hong : 2009.05.14
#define		CLR_WHITE					RGB(0xFF, 0xFF, 0xFF)
#define		CLR_BLACK					RGB(0x00, 0x00, 0x00)
#define		CLR_GRAY					RGB(0x80, 0x80, 0x80)
#define		CLR_LIGHT_GRAY				RGB(0xC0, 0xC0, 0xC0)
#define		CLR_RED						RGB(0xFF, 0x00, 0x00)
#define		CLR_LIGHT_RED				RGB(0xFF, 0x40, 0x40)
#define		CLR_GREEN					RGB(0x00, 0xFF, 0x00)
#define		CLR_BLUE					RGB(0x00, 0x00, 0xFF)
#define		CLR_LIGHT_BLUE				RGB(0x80, 0x80, 0xFF)
#define		CLR_YELLOW					RGB(0xFF, 0xFF, 0x00)
#define		CLR_LIGHT_YELLOW			RGB(0xFF, 0xFF, 0xB0)
#define		CLR_VIOLET					RGB(0xFF, 0x00, 0xFF)

#define		INT_MEAS_PATH_PROBE_SIZE		8
#define		DBL_GLASS_CCUT_RATIO			0.05

#define INT_MAX_CELL_LAYOUT		1000		// MAX Measure Area 2000 to 500
#define INT_MEASE_AREA_MAX		4
#define INT_USE_MAX_PROBE_DATA	5
#define INT_USE_DEFAULT_DATA	9  // 4		// Glass + GLOBAL MARK + LOADER + AC + PROBE(INT_USE_MAX_PROBE_DATA)
#define INT_USE_MEASPOS_DATA	5
#define	INT_MAX_MEAS_RESULT		2500

enum CHART_TEXT_ITEM {
	CHART_TEXT_Y = 0,
		CHART_TEXT_Y_NAME,
		CHART_TEXT_X,
		CHART_TEXT_H1,
		CHART_TEXT_H2,
		CHART_TEXT_H3,
		CHART_TEXT_H4,
		CHART_TEXT_H5,
		CHART_TEXT_H6,
		CHART_TEXT_H7,
		CHART_TEXT_H8,
		CHART_TEXT_PROBE1,
		CHART_TEXT_PROBE2,
		CHART_TEXT_PROBE3,
		CHART_TEXT_PROBE4,
		CHART_TEXT_PROBE5,
		CHART_TEXT_END
};

enum CHART_DATA_ITEM {
	CHART_DATA_ITEM0 = 0,
		CHART_DATA_ITEM1,
		CHART_DATA_ITEM2,
		CHART_DATA_ITEM3,
		CHART_DATA_ITEM4,
		CHART_DATA_ITEM5,
		CHART_DATA_ITEM_END,
};

/////////////////////////////////////////////////////////////////////////////
// CChartOrbit window

class  CChartOrbit : public CChart2D
{
// Construction
public:
	CChartOrbit();
	virtual ~CChartOrbit();

// Attributes
public:
	BOOL			m_bUpdateResult;
	int				m_iChart;
	BOOL			m_bInitSetup;

	CChart2DData*	m_pData1;	// 0:Glass, 1~ Probe Pos + Probe Card, Cell
	CChart2DData*	m_pData2;	// Meas Pos
	CChartMarker**	m_chartMarker;

	CChart2DTextArea** m_pTextItem;
	CChart2DTextArea** m_pTextCell;
	CChart2DTextArea** m_pTextCellStatus;
	CChart2DTextArea** m_pTextMeasResult;

	int				m_nProbe;
	BOOL			m_bCurPosDraw;
	double			m_dCurPosX;
	double			m_dCurPosY;

	double			m_dCardHeight;
	double			m_dCardWidth;

	int				m_nData1;
	int				m_nPts1;
	double*			m_px1;
	double**		m_ppy1;
	
	COLORREF*		m_pColorProbe;

	// Glass Size
	double			m_dXMax;
	double			m_dXMin;
	double			m_dYMax;
	double			m_dYMin;

	// Loader Size
	double			m_dLXMax;
	double			m_dLXMin;
	double			m_dLYMax;
	double			m_dLYMin;

	int				m_iLoaderCCut;

	// Machine Size
	double			m_dMcXMax;
	double			m_dMcXMin;
	double			m_dMcYMax;
	double			m_dMcYMin;

	int				m_iGlsCCut;

	//YR 070315 14:57 
	BOOL			m_bPanelDraw;
	int				m_nPanelNx;
	int				m_nPanelNy;
	double			m_dPanelX0;
	double			m_dPanelY0;
	double			m_dPanelLx;
	double			m_dPanelLy;
	double			m_dPanelDx;
	double			m_dPanelDy;

	int			m_nGrpMarginLeft;
	int			m_nGrpMarginRight;
	int			m_nGrpMarginTop;
	int			m_nGrpMarginBottom;

	BOOL        m_bUseGlassVcm;
	int         m_nCntGlassVcm;
	double		m_dGlassVcm[10];

	BOOL        m_bUseFixtuerTime;
	int         m_nCntFixtuerTime;
	double		m_dFixtuerTimeF[8];
	double		m_dFixtuerTimeR[8];

	int			m_nMaxCell;
	BOOL		m_bInvertXY;

	CWnd* 	m_pParent;
	
// Operations
public:
	void	SetAxisShow(BOOL bVal);
	void	GetConfig();
	void	InitialSetup(int nProbe, COLORREF* pColorProbe, BOOL bShowBORDER = TRUE);

	BOOL	UpdateChart(BOOL bRepaint, int nPlot, int nPts, double** ppX, double** ppY);
	void	UpdateArea(int iMarker, int SetNum, double X, double Y);
	void	SetMaxSize(double dXMax, double dXMin, double dYMax, double dYMin, 
								BOOL bXReverse, BOOL bYReverse, BOOL bXYInvert, int iAngle = 0);

	void	SetLoaderSize(double dXMax, double dXMin, double dYMax, double dYMin, int iLoaderCCut);
	void	SetProbeCardSize(double dXMax, double dXMin, double dYMax, double dYMin);
	void	SetGlassSize(double dXMax, double dXMin, double dYMax, double dYMin, int iGlsCCut);
	void	SetGlobalMakrPos(double x, double y);
	void	SetCurPos(int iProbe, BOOL bDraw, double x, double y);
	void	SetCurPosA(int nProbe, BOOL bDraw, double* px, double* py);
	
	void	SetProbeColor(COLORREF* pColorProbe);

	void	SetPanelParam(BOOL bPanelDraw,	int nPanelNx, int nPanelNy, double dPanelX0, double dPanelY0,
													double dPanelLx, double dPanelLy, double dPanelDx, double dPanelDy);

	BOOL    SetGlassMeasPos(int nIndex, double* ppSX, double* ppSY, double* ppEX, double* ppEY);
	BOOL	SetGlassCell(int nIndex, CStringArray* strCellName, double* ppSX, double* ppSY, double* ppEX, double* ppEY, COLORREF* ppColor);
	void	SetGlassCellLayout(int iIndex, CString strCellName, double dSX, double dSY, double dEX, double dEY);
	void	SetRemoveGlassCellLayout(int iIndex);
	
	//YR 070312 22:18 
	LRESULT			OnOC2Repaint(HDC hdc);

	void	SetGlassVacuum(double* pdVacuum, BOOL bUse = TRUE, int nCnt = 5);
	void	SetFixtureMoveTime(double* pVacuum, BOOL bUse = TRUE, int nCnt = 8);
	void	Prepare(int iIndex);
	void	SetParentWnd(CWnd *pWnd);
	void	ClearResultText();
	void	SetOriginLine (double dXMax, double dXMin, double dYMax, double dYMin);
	
protected:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChartOrbit)
	//}}AFX_VIRTUAL

// Implementation

	// Generated message map functions
protected:
	//{{AFX_MSG(CChartOrbit)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHARTORBIT_H__104371B3_0616_4D23_8661_B5579F57AC00__INCLUDED_)
