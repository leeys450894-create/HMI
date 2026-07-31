#if !defined(AFX_DLGWARNMSG_H__8A4E672A_8649_49AB_B668_53134DEFA19D__INCLUDED_)
#define AFX_DLGWARNMSG_H__8A4E672A_8649_49AB_B668_53134DEFA19D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgWarnMsg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgWarnMsg dialog

class CDlgWarnMsg : public CDialog
{
// Construction
public:
	BOOL IsAlreadyAlarmHappened(long nWarnCode);
	CString GetDataFromString(CString inStr, CString startStr, CString endStr);
	void ClearWarning(long nWarnCode);
	void AddMsg(char szMsg[]);
	void SetMsg(const char *pszMsg);
	CString GetCurDateTime();
	CDlgWarnMsg(CWnd* pParent = NULL);   // standard constructor


// Dialog Data
	//{{AFX_DATA(CDlgWarnMsg)
	enum { IDD = IDD_DIALOG_WARN };
	CListBox	m_lstbxMsg;
	CString	m_strWarnMsg;
	//}}AFX_DATA

	CBrush	m_brsStatic;
	HBRUSH 		m_hbrushBK;

	long	m_lWarnStatus;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgWarnMsg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgWarnMsg)
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual void OnCancel();
	virtual void OnOK();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGWARNMSG_H__8A4E672A_8649_49AB_B668_53134DEFA19D__INCLUDED_)
