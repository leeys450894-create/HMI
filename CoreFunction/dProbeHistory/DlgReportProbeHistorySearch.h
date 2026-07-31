//{{AFX_INCLUDES()
//#include "ctdate.h"
//}}AFX_INCLUDES
#if !defined(AFX_DLGREPORTProbeHistorySEARCH_H__5F33D3E5_BB23_4898_85D3_3E64B819AAC7__INCLUDED_)
#define AFX_DLGREPORTProbeHistorySEARCH_H__5F33D3E5_BB23_4898_85D3_3E64B819AAC7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgReportProbeHistorySearch.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDlgReportProbeHistorySearch dialog

class CDlgReportProbeHistorySearch : public CDialog
{
// Construction
public:
	void SetDateText(SYSTEMTIME time, int iD);
//	void SetDateText(short nDOW, short nDay, short nMonth, short nYear, int iD);
	CDlgReportProbeHistorySearch(CWnd* pParent = NULL);   // standard constructor
//	BOOL GetProbeHistoryInfo(CString sFilePath, CString sSQL);
	BOOL GetProbeHistoryInfo2(CString sFilePath, CString sSQL);
//	void SearchProbeHistoryHistory();
	void SearchProbeHistory();

// Dialog Data
	//{{AFX_DATA(CDlgReportProbeHistorySearch)
	enum { IDD = IDD_DLG_REPORT_ProbeHistory_SEARCH };
	CButton	m_btnSearchProbeHistory;
	CListCtrl	m_listReportProbeHistory;
//	CctDate	m_ctDateFrom;
//	CctDate	m_ctDateTo;
	//}}AFX_DATA

	int			m_nProbeHistoryCount;
	BOOL		m_bStop;

	CDateTimeCtrl	m_ctDateFrom;
	CDateTimeCtrl	m_ctDateTo;
	SYSTEMTIME m_stFrom;
	SYSTEMTIME m_stTo;

	CString		m_strRootPath;
	CString		m_sStartTime;
	CString		m_sEndTime;
	CString		m_strPath;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgReportProbeHistorySearch)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgReportProbeHistorySearch)
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonSearchProbeHistory();
	afx_msg void OnBtnProbeHistorySearchSaveasCsv();

	/*afx_msg void OnDateClickCtdatectrlFrom(short nDOW, short nDay, short nMonth, short nYear);
	afx_msg void OnDateClickCtdatectrlTo(short nDOW, short nDay, short nMonth, short nYear);
	afx_msg void OnLoadDayInfoCtdatectrlTo(short nMonth, short nYear, short nSection);
	afx_msg void OnLoadDayInfoCtdatectrlFrom(short nMonth, short nYear, short nSection);*/
	//DECLARE_EVENTSINK_MAP()
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnMcnSelchangeMonthcalendarFrom(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMcnSelchangeMonthcalendarTo(NMHDR* pNMHDR, LRESULT* pResult);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGREPORTProbeHistorySEARCH_H__5F33D3E5_BB23_4898_85D3_3E64B819AAC7__INCLUDED_)
