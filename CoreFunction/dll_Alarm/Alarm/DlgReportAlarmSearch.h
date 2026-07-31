//{{AFX_INCLUDES()
//}}AFX_INCLUDES
#if !defined(AFX_DLGREPORTALARMSEARCH_H__5F33D3E5_BB23_4898_85D3_3E64B819AAC7__INCLUDED_)
#define AFX_DLGREPORTALARMSEARCH_H__5F33D3E5_BB23_4898_85D3_3E64B819AAC7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgReportAlarmSearch.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgReportAlarmSearch dialog

class CDlgReportAlarmSearch : public CDialog
{
// Construction
public:
	void SetDateText(SYSTEMTIME time, int iD);
	CDlgReportAlarmSearch(CWnd* pParent = NULL);   // standard constructor
	BOOL GetAlarmInfo(CString sFilePath, CString sSQL);
	void SearchAlarmHistory();
	void SetSavePath(CString strRootPath) { m_strRootPath.Format(_T("%s"), strRootPath); }

// Dialog Data
	//{{AFX_DATA(CDlgReportAlarmSearch)
	enum { IDD = IDD_DLG_REPORT_ALARM_SEARCH };
	CButton	m_btnSearchAlarm;
	CListCtrl	m_listReportAlarm;
	CDateTimeCtrl	m_ctDateFrom;
	CDateTimeCtrl	m_ctDateTo;
	SYSTEMTIME		m_stFrom;
	SYSTEMTIME		m_stTo;
	//}}AFX_DATA

	int			m_nAlarmCount;
	BOOL		m_bStop;
	
	CString		m_strRootPath;
	CString		m_sStartTime;
	CString		m_sEndTime;
	CString		m_strPath;

	CDatabase	m_db;
	CRecordset* m_pRs;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgReportAlarmSearch)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgReportAlarmSearch)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonSearchAlarm();
	afx_msg void OnBtnAlarmSearchSaveasCsv();
	afx_msg void OnMcnSelectMonthcalendarFrom(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMcnSelectMonthcalendarTo(NMHDR *pNMHDR, LRESULT *pResult);	
//	DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGREPORTALARMSEARCH_H__5F33D3E5_BB23_4898_85D3_3E64B819AAC7__INCLUDED_)
