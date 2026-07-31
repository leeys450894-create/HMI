// FFUMon.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "FFUMon.h"
#include "FFUMonDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFFUMonApp

BEGIN_MESSAGE_MAP(CFFUMonApp, CWinApp)
	//{{AFX_MSG_MAP(CFFUMonApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFFUMonApp construction

CFFUMonApp::CFFUMonApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CFFUMonApp object

CFFUMonApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CFFUMonApp initialization

BOOL CFFUMonApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	HANDLE hEvent = CreateEvent(NULL, FALSE, TRUE, AfxGetAppName());
	if( GetLastError() == ERROR_ALREADY_EXISTS)
	{
		AfxMessageBox("이미 실행 상태 입니다.");
		return FALSE;
	}

#ifdef _AFXDLL
//	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CFFUMonDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
