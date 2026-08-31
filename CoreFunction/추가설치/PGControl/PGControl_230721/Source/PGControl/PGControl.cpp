#include "pch.h"
#include "PGControl.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CPGControlApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

CPGControlApp theApp;
C_LogMgr	  theLogMgr;

CPGControlApp::CPGControlApp()
{
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	m_pEventDelay = new CEvent(TRUE, FALSE);
	m_pEventDelay->SetEvent();
}

BOOL CPGControlApp::InitInstance()
{
	if (!CheckOverlapExcute()) {
		AfxMessageBox(dfcStr(_T("(%s) 이 실행중 입니다."), CLASSNAME_PGCONTROL));
		return FALSE;
	}

	CWinApp::InitInstance();

	SetRegistryKey(CLASSNAME_PGCONTROL);
	AfxSocketInit();

	// 2023.07.17 change
	CString strPath;
	TCHAR	pBuf[1024];
	::GetPrivateProfileString(_T("PATH"), _T("LOG"), _T("D:\\LOG\\PGControl\\"), pBuf, 1024, dfcIniFile(_T("Config.ini")));
	strPath = CString(pBuf);

	theLogMgr.SetPath(strPath/*_T("D:\\LOG\\PGControl\\")*/, 0);
	//

	m_pQualityPG232Comm  = new C_QualityPGComm		(CmEvent_CommEvt, dfcIniFile(_T("Comm.ini")), 0);
	m_pPG232Comm         = new C_SendElpPGComm		(CmEvent_CommEvt, dfcIniFile(_T("Comm.ini")), 1);
	m_pColorPGServerComm = new C_ColorPGServerComm	(CmEvent_CommEvt, dfcIniFile(_T("Comm.ini")), 0);
	m_pMainPGClientComm  = new C_MainPGClientComm	(CmEvent_CommEvt, dfcIniFile(_T("Comm.ini")), 1);

	CPGControlDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{

	}
	else if (nResponse == IDCANCEL)
	{

	}

	return FALSE;
}

int		CPGControlApp::ExitInstance()
{
	SAFE_DELETE (m_pEventDelay);
	SAFE_DELETE (m_pQualityPG232Comm);
	SAFE_DELETE (m_pPG232Comm);
	SAFE_DELETE (m_pColorPGServerComm);
	SAFE_DELETE (m_pMainPGClientComm);

	return CWinApp::ExitInstance();
}

BOOL	CPGControlApp::CheckOverlapExcute()
{
	//<<- 중복실행 방지 코드
	HANDLE hMutex = CreateMutex(NULL, TRUE, CLASSNAME_PGCONTROL);
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		ReleaseMutex(hMutex);
		CWnd* pWndPrev, * pWndChild;
		if (pWndPrev = CWnd::FindWindow(NULL, CLASSNAME_PGCONTROL))
		{
			pWndChild = pWndPrev->GetLastActivePopup();
			if (pWndPrev->IsIconic())
				pWndPrev->ShowWindow(SW_RESTORE);
			pWndChild->SetForegroundWindow();
		}
		return FALSE;
	}
	ReleaseMutex(hMutex);
	return TRUE;
}

void	CPGControlApp::EnableControls(BOOL bEnable)
{
	if (GetMainPtr()) GetMainPtr()->EnableControls(bEnable);
}