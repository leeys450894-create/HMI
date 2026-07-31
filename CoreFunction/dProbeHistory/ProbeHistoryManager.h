#if !defined(AFX_ProbeHistoryMANAGER_H__F8C69B52_B1E1_4FDA_A010_9AE931B9EF41__INCLUDED_)
#define AFX_ProbeHistoryMANAGER_H__F8C69B52_B1E1_4FDA_A010_9AE931B9EF41__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProbeHistoryManager.h : header file
//

#include "API_ProbeHistory.h"
//#include "DlgProbeHistory.h"
#include "DlgReportProbeHistory.h"

#include <afxtempl.h>

/////////////////////////////////////////////////////////////////////////////
// CProbeHistoryManager window

class CProbeHistoryManager : public CWnd
{
// Construction
public:
	CProbeHistoryManager();

// Attributes
public:
//	CDlgProbeHistory			m_dlgProbeHistory;
	CDlgReportProbeHistory	* m_pdlgReportProbeHistory;
	BOOL			m_bInitialized;		// pwk 20130602
// Operations
public:
	// ProbeHistory Init/Final
//	void InitializeProbeHistory(HWND hParent);
//	void FinalizeProbeHistory();

	// ProbeHistory Report Init/Final
	void InitializeReport(HWND hParent, CRect r);
	void FinalizeReport();

	// ProbeHistory dlg
//	void CreateProbeHistoryDlg(HWND hParent);
	void CreateProbeHistoryReportDlg(HWND hParent, CRect r);

	CWnd* GetReportProbeHistoryWnd();

	void LoadProbeHistoryFile(CString strFileName = "ProbeHistory.csv");
	BOOL FindProbeHistoryTable(UINT id, ProbeHistory_DATA& ProbeHistory);

// 	int SetProbeHistory(UINT id);
// 	int SetHeavyProbeHistory(ProbeHistory_DATA ProbeHistory);
// 	int SetLightProbeHistory(ProbeHistory_DATA ProbeHistory);
// 	int GetListCount();

	int SetProbeHistoryInsert(ProbeHistory_DATA ProbeHistory);
	int SetProbeHistoryUpdateContactCnt(ProbeHistory_DATA ProbeHistory);
	int SetProbeHistoryUpdateUnmountTime(ProbeHistory_DATA ProbeHistory);

	int SetProbeHistoryInsert(CString strRFID,CString strMountTime);
	int SetProbeHistoryUpdateContactCnt(CString strRFID, CString strMountTime,int nContactCount);
	int SetProbeHistoryUpdateUnmountTime(CString strRFID, CString strMountTime,CString strUnmountTime);
	void SetSavePath(CString strPath);

	int ProbeUpdateLanguage(HINSTANCE hResource_Lang_Dll);//lhj 20181225 Report Probe 

private:
	CMap<unsigned int, unsigned int, ProbeHistory_DATA, ProbeHistory_DATA&> m_ProbeHistoryTable;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProbeHistoryManager)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CProbeHistoryManager();

	// Generated message map functions
protected:
	//{{AFX_MSG(CProbeHistoryManager)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ProbeHistoryMANAGER_H__F8C69B52_B1E1_4FDA_A010_9AE931B9EF41__INCLUDED_)
