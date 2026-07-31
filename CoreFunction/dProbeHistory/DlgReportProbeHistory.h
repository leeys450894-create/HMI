#if !defined(AFX_DLGREPORTProbeHistory_H__C606AC6C_140A_4D4A_93EF_C238A1BC48DD__INCLUDED_)
#define AFX_DLGREPORTProbeHistory_H__C606AC6C_140A_4D4A_93EF_C238A1BC48DD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#include "API_ProbeHistory.h"
#endif // _MSC_VER > 1000
// DlgReportProbeHistory.h : header file
//

//#include "..\\include\\API_SMEM.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgReportProbeHistory dialog

class CDlgReportProbeHistory : public CDialog
{
// Construction
public:
	CDlgReportProbeHistory(CWnd* pParent = NULL);  // standard constructor
	BOOL AddProbeHistoryHistory(ProbeHistory_DATA *pProbeHistory);
	BOOL Queryexe(CString strSQL);			// Äõ¸® ½ÇÇà.
	
	BOOL GetProbeHistoryInfo2(CString sFilePath, CString sSQL);
	BOOL GetProbeHistory(ProbeHistory_DATA *pProbeHistory);
	BOOL AddProbeHistoryInsert(ProbeHistory_DATA *pProbeHistory);
	BOOL AddProbeHistoryUpdateContactCnt(ProbeHistory_DATA *pProbeHistory);
	BOOL AddProbeHistoryUpdateContactUnmountTime(ProbeHistory_DATA *pProbeHistory);

	int UpdateLanguage(HINSTANCE hResource_Lang_Dll); //lhj 20181225 lang Report Probe
	void UpdateProbeInfo();

// Dialog Data
	//{{AFX_DATA(CDlgReportProbeHistory)
	enum { IDD = IDD_DLG_REPORT_ProbeHistory };
	CListCtrl	m_listReportProbeHistory;
	//}}AFX_DATA
	
	HBRUSH 			m_hbrushBK;
	CString			m_strRootPath;
	void			SetSavePath(CString strRootPath)	{ m_strRootPath.Format(_T("%s"),strRootPath); }
//	VTM_SYSTEM*		m_pSystem; 

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgReportProbeHistory)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgReportProbeHistory)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonAlarmReportClear();
	afx_msg void OnBnClickedButtonAlarmReportSearch();
//	afx_msg void OnClickCtpushctrlProbeHistoryReportClear();
//	afx_msg void OnClickCtpushctrlProbeHistoryReportSearch();
//	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGREPORTProbeHistory_H__C606AC6C_140A_4D4A_93EF_C238A1BC48DD__INCLUDED_)
