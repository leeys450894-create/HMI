#if !defined(AFX_DLGREPORTALARM_H__C606AC6C_140A_4D4A_93EF_C238A1BC48DD__INCLUDED_)
#define AFX_DLGREPORTALARM_H__C606AC6C_140A_4D4A_93EF_C238A1BC48DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#include "AlarmDefine.h"
#endif // _MSC_VER > 1000
// DlgReportAlarm.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgReportAlarm dialog

class CDlgReportAlarm : public CDialog
{
// Construction
public:
	CDlgReportAlarm(CWnd* pParent = NULL);  // standard constructor
	BOOL AddAlarmHistory(ALARM_EVENT_DATA &alarm, bool bClear = false, bool bLightAlarm = false);
	BOOL Queryexe(CString strSQL);			// Äõ¸® ½ÇÇà.

	int UpdateLanguage(HINSTANCE hResource_Lang_Dll); //lhj 20181225 lang Report Alarm


// Dialog Data
	//{{AFX_DATA(CDlgReportAlarm)
	enum { IDD = IDD_DLG_REPORT_ALARM };
	CListCtrl	m_listReportAlarm;
	//}}AFX_DATA
	
	HBRUSH 			m_hbrushBK;
	CString			m_strRootPath;
	int				m_nHostMode;
	CDatabase m_db;
	void	SetHostMode(int iMode)	{ m_nHostMode = iMode; }
	void	SetSavePath(CString strRootPath)	{ m_strRootPath.Format(_T("%s"), strRootPath); }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgReportAlarm)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgReportAlarm)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonAlarmReportClear();
	afx_msg void OnBnClickedButtonAlarmReportSearch();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGREPORTALARM_H__C606AC6C_140A_4D4A_93EF_C238A1BC48DD__INCLUDED_)
