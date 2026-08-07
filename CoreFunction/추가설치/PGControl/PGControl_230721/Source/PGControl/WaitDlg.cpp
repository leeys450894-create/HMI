#include "pch.h"
#include "PGControl.h"
#include "WaitDlg.h"

#ifdef _DEBUG 
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Wait Dialog
UINT PROC_RUN_MESSAGE = RegisterWindowMessage (_T("PROC_RUN_MESSAGE"));

UINT RunThread (LPVOID pParam)
{
	int nMsg = WM_FUNC_OK_MESSAGE;

	if (theWaitDlg->m_pWaitFunc) {
		if (theWaitDlg->m_pWaitFunc ()) {
			nMsg = WM_FUNC_OK_MESSAGE;
		}
		else {
			nMsg = WM_FUNC_CANCEL_MESSAGE;
		}
	}

	if (theWaitDlg->m_bUserStop) theWaitDlg->PostMessage (WM_FUNC_CANCEL_MESSAGE);
	else						 theWaitDlg->PostMessage (nMsg);
	
	theApp.m_runState.Empty ();

	return 0;
}

IMPLEMENT_DYNAMIC (C_WaitDlg, CDialogEx)
BEGIN_MESSAGE_MAP (C_WaitDlg, CDialogEx)
	ON_REGISTERED_MESSAGE (PROC_RUN_MESSAGE, OnRun)
	ON_WM_CTLCOLOR ()
	ON_WM_DESTROY ()
	ON_WM_SIZE ()
	ON_MESSAGE (WM_FUNC_OK_MESSAGE, OnWaitOK)
	ON_MESSAGE (WM_FUNC_CANCEL_MESSAGE, OnWaitCancel)

END_MESSAGE_MAP ()

C_WaitDlg::C_WaitDlg (CWnd *pParent) : CDialogEx (IDD_WAIT, pParent)
{
	Create (IDD_WAIT, pParent);
	ShowWindow (SW_HIDE);
	m_pProcThread = NULL;
	m_pWaitFunc = NULL;
}

C_WaitDlg::~C_WaitDlg ()
{

}

void	C_WaitDlg::OnDestroy ()
{
	SAFE_DELETE (m_pProcThread);
	CDialogEx::OnDestroy ();
}

void	C_WaitDlg::DoDataExchange (CDataExchange *pDX)
{
	CDialogEx::DoDataExchange (pDX);
	DDX_Control (pDX, IDCANCEL, m_btnCancel);
}

BOOL	C_WaitDlg::OnInitDialog ()
{
	CDialogEx::OnInitDialog ();
	m_btnCancel.SetWindowText (_T(""));
	m_btnCancel.SetFaceColor (COLOR_USER_FRAME);
	m_btnCancel.m_nFlatStyle = CMFCButton::BUTTONSTYLE_3D;
	m_btnCancel.SetImage (IDB_CANCEL);

	m_bUserStop	   = FALSE;

	return TRUE;
}

void	C_WaitDlg::OnSize (UINT nType, int cx, int cy)
{
	CWnd *pWnd = GetDlgItem (IDCANCEL);
	if (pWnd->GetSafeHwnd ()) {
		CRect rc;
		GetWindowRect (&rc);
		
		pWnd->SetWindowPos (NULL, 0, 0, rc.Width (), rc.Height (), SWP_NOZORDER);
	}
	
	CDialogEx::OnSize(nType, cx, cy);
}

void	C_WaitDlg::SetWaitFunc (WaitFunc pWaitFunc)
{
	if (m_pWaitFunc) return;
	m_pWaitFunc = pWaitFunc;
	PostMessage (PROC_RUN_MESSAGE);
}

LRESULT	C_WaitDlg::OnWaitCancel (WPARAM wParam, LPARAM lParam)
{
	if (m_pWaitFunc) CmStop ();
	m_pWaitFunc = NULL;
	theApp.EnableControls (TRUE);

	while (m_pProcThread != NULL) {
		DWORD dwRet = WaitForSingleObject (m_pProcThread->m_hThread, 1);
		if (dwRet == WAIT_OBJECT_0 || dwRet == STATUS_TIMEOUT)	break;
		dfcPeekAndPump ();
	}

	SAFE_DELETE (m_pProcThread);

	CDialogEx::OnCancel ();

	return 0;
}

LRESULT	C_WaitDlg::OnWaitOK (WPARAM wParam, LPARAM lParam)
{
	m_pWaitFunc = NULL;
	theApp.m_runState.m_nRun = RS_NONE;
	theApp.EnableControls (TRUE);

	while (m_pProcThread != NULL) {
		DWORD dwRet = WaitForSingleObject (m_pProcThread->m_hThread, 1);
		if (dwRet == WAIT_OBJECT_0 || dwRet == STATUS_TIMEOUT)	break;
		dfcPeekAndPump ();
	}

	SAFE_DELETE (m_pProcThread);

	CDialogEx::OnOK ();

	return 0;

}

void	C_WaitDlg::OnCancel ()
{
	CmStop (TRUE);

	m_bUserStop = TRUE;
	m_pWaitFunc = NULL;

	CDialogEx::OnCancel ();
}

LRESULT	C_WaitDlg::OnRun (WPARAM wParam, LPARAM lParam)
{
	theApp.m_runState.m_nRun = RS_UNITFUNC;
	m_bUserStop = FALSE;

	theApp.EnableControls (FALSE);
	ShowWindow (SW_SHOW);
	GetDlgItem (IDCANCEL)->EnableWindow (TRUE);
	
	if (m_pWaitFunc) {
		if (m_pProcThread == NULL) {
			m_pProcThread = AfxBeginThread (RunThread, this);
			m_pProcThread->m_bAutoDelete = FALSE;
		}
	}

	return 0;
}
