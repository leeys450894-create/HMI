#if !defined(AFX_MCCMANGER_H__B1318211_BC0D_4397_BF25_A9FA8CC46183__INCLUDED_)
#define AFX_MCCMANGER_H__B1318211_BC0D_4397_BF25_A9FA8CC46183__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MCCManger.h : header file
//
#include "DlgnMCCReport.h"

/////////////////////////////////////////////////////////////////////////////
// CMCCManger window

class CMCCManger : public CWnd
{
// Construction
public:
	CMCCManger();

// Attributes
public:

// Operations
public:
	CDlgnMCCReport *m_dlgMCC;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMCCManger)
	//}}AFX_VIRTUAL

// Implementation
public:
	void KillThred();
	void Finalize();
	void CreateMCCLogDlg(HWND hParent, CRect r);
	void InitializeMCC(HWND hParent, CRect r); 
	CWnd* GetReportMCCWnd();
	void SetLogPath(CString str);
	void SetConfigPath(CString str);
	void SetFilePath(CString str);
	void SetEQName(CString str);

	virtual ~CMCCManger();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMCCManger)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	int AddMccQueue(int iFlag);
	VTM_MCC* GetSharedMemory();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MCCMANGER_H__B1318211_BC0D_4397_BF25_A9FA8CC46183__INCLUDED_)
