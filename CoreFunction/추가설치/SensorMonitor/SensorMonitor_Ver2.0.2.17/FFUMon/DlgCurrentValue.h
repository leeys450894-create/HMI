#if !defined(AFX_DLGCURRENTVALUE_H__4C8120CF_4285_4CB6_B93D_34304193A3A4__INCLUDED_)
#define AFX_DLGCURRENTVALUE_H__4C8120CF_4285_4CB6_B93D_34304193A3A4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgCurrentValue.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgCurrentValue dialog

class CDlgCurrentValue : public CDialog
{
// Construction
public:
	CDlgCurrentValue(CWnd* pParent = NULL, int nEfuTotalNum = 32, int nIdex = 1);   // standard constructor		//20181207 edhan add parameter
	//20181207 edhan add
	int m_nIdex;
	int m_nEfuTotalNum;
	//
// Dialog Data
	//{{AFX_DATA(CDlgCurrentValue)
	enum { IDD = IDD_DIALOG_VALUE };
	CListCtrl	m_list;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgCurrentValue)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgCurrentValue)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCURRENTVALUE_H__4C8120CF_4285_4CB6_B93D_34304193A3A4__INCLUDED_)
