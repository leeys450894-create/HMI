#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SACCManger.h : header file
//
#include "CDlgSACC.h"
#include "ShareManager.h"
/////////////////////////////////////////////////////////////////////////////
// CMCCManger window

class CSACCManger : public CWnd
{
// Construction
public:
	CSACCManger();
	virtual ~CSACCManger();
// Attributes
public:

// Operations
public:
	CDlgSACC *m_dlgSACC;
	CShareManager* m_cManager;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMCCManger)
	//}}AFX_VIRTUAL

// Implementation
public:
	void KillThred();
	void Finalize();
	void CreateSACCDlg(HWND hParent, CRect r);
	void InitializeSACC(HWND hParent, CRect r); 
	CWnd* GetReportSACCWnd();
	void SetLogPath(CString str);
	void SetConfigPath(CString str);
	void SetFilePath(CString str);
	void SetEQName(CString str);
	void Updata(PDITEMDATA data);
	void GetProbeInfo(int iProbe, CString* pstrRFID, CString* pstrMountTime, int* pnContactCount);
	void SetProbeInfo(int iProbe, CString strRFID, CString strMountTime, int nContactCount);

	// Generated message map functions
protected:
	//{{AFX_MSG(CMCCManger)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	int AddMccQueue(int iFlag);
	//VTM_MCC* GetSharedMemory();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

