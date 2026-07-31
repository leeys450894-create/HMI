#if !defined(AFX_DLGWARNING_H__765D20FC_1EA7_44FE_8D0F_1B8598A69D2C__INCLUDED_)
#define AFX_DLGWARNING_H__765D20FC_1EA7_44FE_8D0F_1B8598A69D2C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgWarning.h : header file
//

#include "AlarmDefine.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgWarning dialog

class CDlgWarning : public CDialog
{
// Construction
public:
	CDlgWarning(CWnd* pParent = NULL);   // standard constructor
	void InitControls();
	CString GetCurDateTime();

	void InsertList(ALARM_EVENT_DATA *pAlarm);
	void ClearWarning(UINT code);
	BOOL FindList(UINT code);
	void SetAlarmView(HWND hWnd);
	UINT ParsingALARMCode(CString str);

//	int UpdateLanguage(HINSTANCE hResource_Lang_Dll); //lhj 20181225 lang Report Warning

// Dialog Data
	//{{AFX_DATA(CDlgWarning)
	enum { IDD = IDD_DLG_ALARM_WARNING};
	CListCtrl	m_listWarning;
	//}}AFX_DATA

	CBrush	m_brsStatic;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgWarning)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgWarning)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnButtonAlarmClear();
	virtual BOOL OnInitDialog();
	afx_msg void OnClickListWarning(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	HWND m_hAlarmWnd;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGWARNING_H__765D20FC_1EA7_44FE_8D0F_1B8598A69D2C__INCLUDED_)
