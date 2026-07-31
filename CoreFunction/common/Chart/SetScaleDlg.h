#if !defined(AFX_SETSCALEDLG_H__32C037AC_E378_41FE_9E15_19FB04B9D898__INCLUDED_)
#define AFX_SETSCALEDLG_H__32C037AC_E378_41FE_9E15_19FB04B9D898__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#include "resource.h"
#endif // _MSC_VER > 1000
// SetScaleDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSetScaleDlg dialog

class CSetScaleDlg : public CDialog
{
// Construction
public:
	CSetScaleDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSetScaleDlg)
	enum { IDD = IDD_SET_SCALE };
	CString	m_editXMax;
	CString	m_editXMin;
	CString	m_editYMax;
	CString	m_editYMin;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetScaleDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSetScaleDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETSCALEDLG_H__32C037AC_E378_41FE_9E15_19FB04B9D898__INCLUDED_)
