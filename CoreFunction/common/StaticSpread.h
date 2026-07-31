#if !defined(AFX_STATICSPREAD_H__384BDBED_3901_4BDC_B39C_7E79ED042946__INCLUDED_)
#define AFX_STATICSPREAD_H__384BDBED_3901_4BDC_B39C_7E79ED042946__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StaticSpread.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStaticSpread window



#include ".\FarPoint64\SSDLLMFC.h"


class CStaticSpread : public CStatic
{
// Construction
public:
	CStaticSpread();

// Attributes
public:
    TSpread m_Spread;	
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStaticSpread)
	protected:
	virtual void PreSubclassWindow();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	BOOL GetActiveCell(int *Col, int *Row);
	BOOL SetActiveCell(int Col, int Row);
	BOOL SetSelectBlock(int iColS, int iRowS, int iColE, int iRowE);
	void SetMaxRows(int nRows);
	void SetMaxCols(int nCols);
	void SetWindowPos(CRect rect);
	BOOL SetRowHeight(int iRow, double dHeight);
	BOOL SetColWidth(int iCol, double dWidth);
	BOOL SetData(int x, int y, const TCHAR *Format, ...);
    double GetData(int x, int y);
	CString GetDataString(int x, int y);

    BOOL SaveCSVFile  (CString sFileName);
    BOOL SaveExcelFile(CString sFileName);//주의:헤더는 저장되지않음!!
	BOOL ImportExeclFile(CString sFileName);

	virtual ~CStaticSpread();

	// Generated message map functions
protected:
	//{{AFX_MSG(CStaticSpread)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STATICSPREAD_H__384BDBED_3901_4BDC_B39C_7E79ED042946__INCLUDED_)
