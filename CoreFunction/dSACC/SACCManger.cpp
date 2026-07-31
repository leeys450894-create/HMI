// MCCManger.cpp : implementation file
//

#include "pch.h"
#include "dSACC.h"
#include "SACCManger.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMCCManger

CSACCManger::CSACCManger()
{
	m_dlgSACC = nullptr;
	m_cManager = nullptr;
}

CSACCManger::~CSACCManger()
{
	if (m_dlgSACC != nullptr)
	{
		delete m_dlgSACC;
	}
		
	if (m_cManager != nullptr)
	{
		m_cManager->FinalizeReport();
		delete m_cManager;
		m_cManager = nullptr;
	}
}


BEGIN_MESSAGE_MAP(CSACCManger, CWnd)
	//{{AFX_MSG_MAP(CMCCManger)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMCCManger message handlers

void CSACCManger::SetLogPath(CString str)
{
	//m_dlgSACC->SetLogPath(str);
}

void CSACCManger::SetConfigPath(CString str)
{
	//m_dlgSACC->SetConfigPath(str);
}

void CSACCManger::SetFilePath(CString str)
{
	//m_dlgSACC->SetFilePath(str);
}

void CSACCManger::SetEQName(CString str)
{
	//m_dlgMCC->SetEQName(str);

	//m_dlgMCC->SetMccILogHeader();
}

void CSACCManger::InitializeSACC(HWND hParent, CRect r)
{
	// ShareManager
	if (m_cManager == NULL)
	{
		m_cManager = new CShareManager();
	}

	m_cManager->InitializeReport(hParent);

	m_cManager->SetHostMode(1);
	////

	// SACCManager 윈도우 생성.
	Create(NULL,NULL,WS_VISIBLE|WS_CHILD,CRect(0,0,0,0),this,1);
	
	// SACC 다이얼로그 생성.
	CreateSACCDlg(hParent, r);
}

void CSACCManger::CreateSACCDlg(HWND hParent, CRect r)
{
	CRect rect;

	if (!hParent) return;
	
	if (::IsWindow(m_dlgSACC->GetSafeHwnd()))
	{
		m_dlgSACC->DestroyWindow();
	}
		
	m_dlgSACC = new CDlgSACC(FromHandle(hParent));

	m_dlgSACC->Create(IDD_DLG_SACCD, FromHandle(hParent));
	
	m_dlgSACC->GetWindowRect(rect);
	m_dlgSACC->MoveWindow(r.left + 1,r.top + 18, rect.Width(),rect.Height());	// jsyoon 20160226 add

	if (::IsWindow(m_dlgSACC->GetSafeHwnd()))
		m_dlgSACC->ShowWindow(SW_HIDE);
}

CWnd* CSACCManger::GetReportSACCWnd()
{
	return m_dlgSACC;
}

void CSACCManger::Finalize()
{
	if (::IsWindow(m_dlgSACC->GetSafeHwnd()))
	{
//		m_dlgMCC.KillThred();
//		m_dlgMCC->EndDialog(0);
	}
}

void CSACCManger::KillThred()
{
	if (::IsWindow(m_dlgSACC->GetSafeHwnd()))
	{
		//m_dlgMCC->KillThred();
		//m_dlgMCC->WaitTerminateWorkerThread();
	}
}


int CSACCManger::AddMccQueue(int iFlag)
{
	//if (::IsWindow(m_dlgMCC->GetSafeHwnd()))
	//{
	//	m_dlgMCC->AddMccQueue(iFlag);
	//}

	return 0;
}

//VTM_MCC* CSACCManger::GetSharedMemory()
//{
//	return m_dlgMCC->m_pMCCMap;
//}

void CSACCManger::Updata(PDITEMDATA data)
{
	if (m_cManager != nullptr)
		m_cManager->Updata(data);
	/*if (pMoniter != NULL)
	{
		memcpy(pMoniter, data, sizeof(DITEMDATA));
	}*/
}

void CSACCManger::GetProbeInfo(int iProbe, CString* pstrRFID, CString* pstrMountTime, int* pnContactCount)
{
	if (m_dlgSACC != nullptr)
		m_dlgSACC->GetProbeInfo( iProbe,  pstrRFID,  pstrMountTime,  pnContactCount);
}

void CSACCManger::SetProbeInfo(int iProbe, CString strRFID, CString strMountTime, int nContactCount)
{
	if (m_dlgSACC != nullptr)
		m_dlgSACC->SetProbeInfo( iProbe,  strRFID,  strMountTime,  nContactCount);
}