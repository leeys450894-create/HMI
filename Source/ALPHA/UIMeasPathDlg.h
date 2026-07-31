#if !defined(AFX_UIMEASPATHDLG_H__03ABA978_A847_4304_8293_D1206A8FECFF__INCLUDED_)
#define AFX_UIMEASPATHDLG_H__03ABA978_A847_4304_8293_D1206A8FECFF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
//#include "CommonFunc.h"
#include "common\Chart\\ChartOrbit.h"			//Added by Jay Hong : 2009.05.14
#include "..\\Include\\DefinesSys.h"
#endif // _MSC_VER > 1000
// UIMeasPathDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CUIMeasPathDlg dialog

class CUIMeasPathDlg : public CDialog
{
// Construction
public:
	void SetGlassMeasPos(int nIndex, double* dSX, double* dSY, double* dEX, double* dEY);
	void UpdateMeasResultChart(int nMeasArea, int iCurMeasArea, double dXPos, double dYPos, CString strSH, int iColor = 0);
	void ClearMeasResultChart();
	void SetGlassVacuum(double* pVacuum, BOOL bUse, int nCnt);
	void SetAxisShow(BOOL bVal);
	void UpdateOrbitArea(int iMarker, int SetNum, double X, double Y);
//	void UpdateCellPosition2(RECIPEPATH* pRecipe, int iCellDir = 0);
	void UpdateCellPosition3(RECIPELAYOUT* pCellinfo);
//	void UpdateCellPosition(RECIPEPATH* pRecipe);
	void SetRemoveGlassCellLayout(int iIndex);
	void SetGlassCell(int nIndex, CStringArray* strCellName, double* dSX, double* dSY, double* dEX, double* dEY, COLORREF* pColor);
	void SetGlassCellLayout(int iIndex, CString strCellName, double dSX, double dSY, double dEX, double dEY);
	void SetPanelParam(BOOL bPanelDraw,	int nPanelNx, int nPanelNy, double dPanelX0, double dPanelY0, double dPanelLx, double dPanelLy, double dPanelDx, double dPanelDy);
//	void UpdateGroupPosition(RECIPEPATH* pRecipe);
	void UpdateMarkCoord(double x, double y);
	void UpdateCurCoord(int iProbe, double x, double y, double z, double s);
	void UpdateCurCoordA(int nProbe, double* px, double* py);
	void SetGlassSize(double dXMax, double dXMin, double dYMax, double dYMin, int iGlsCCut);
	void SetCardSize(double dXMax, double dXMin, double dYMax, double dYMin);
	void SetLoaderSize(double dXMax, double dXMin, double dYMax, double dYMin, int iLoaderCCut);
	void DrawChart(CWnd *pWnd);
	void SetParentWnd(CWnd *pWnd);
	void SetProbeColor(COLORREF *pColorProbe);
	BOOL AttachToView(CWnd *pParentWnd);
	void SetMaxSize(double dXMax, double dXMin, double dYMax, double dYMin, BOOL bXReverse, BOOL bYReverse, BOOL bXYInvert, int iAngle = 0 );
	void UpdateOrbitChart(int nProbe, int nGroup, double **ppX, double **ppY);
	void SetNProbe(int nProbe, COLORREF* pColorProbe = NULL, BOOL bShowBORDER = TRUE);
	CUIMeasPathDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CUIMeasPathDlg)
	enum { IDD = IDD_DLG_UI_MEAS_PATH };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	CChartOrbit		m_chartOrbit;	//Added by Jay Hong : 2009.05.14
	CWnd*			m_pParent;
//	VTM_SYSTEM*		m_pSystem;
	BOOL			m_bRepaint;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUIMeasPathDlg)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	int m_nOldCell;

	// Generated message map functions
	//{{AFX_MSG(CUIMeasPathDlg)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
//	afx_msg LRESULT	OnUpdateCoord		(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnPathNProbe		(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnPathProbeColor	(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnPathMaxSize		(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnPathGlsSize		(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnPathUpdateOrbit	(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	afx_msg LRESULT	OnOC2Repaint		(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
public:
	int SetFixtureMoveTime(double* pTime, BOOL bUse, int nCnt);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UIMEASPATHDLG_H__03ABA978_A847_4304_8293_D1206A8FECFF__INCLUDED_)
