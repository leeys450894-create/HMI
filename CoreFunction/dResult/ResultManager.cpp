#include "pch.h"
#include "CDlgResult.h"
#include "ResultManager.h"

CResultManager::CResultManager()
{
	m_dlgResult = nullptr;
}

CResultManager::~CResultManager()
{
	delete m_dlgResult;
}

BEGIN_MESSAGE_MAP(CResultManager, CWnd)
END_MESSAGE_MAP()


void CResultManager::InitializeResult(HWND hParent, CRect r)
{
	// ResultManager 윈도우 생성.
	Create(NULL, NULL, WS_VISIBLE | WS_CHILD, CRect(0, 0, 0, 0), this, 1);

	// ResultManager 다이얼로그 생성.
	CreateResultDlg(hParent, r);
}

void CResultManager::CreateResultDlg(HWND hParent, CRect r)
{
	CRect rect;

	if (!hParent) return;

	if (::IsWindow(m_dlgResult->GetSafeHwnd()))
	{
		m_dlgResult->DestroyWindow();
	}

	m_dlgResult = new CDlgResult(FromHandle(hParent));

	m_dlgResult->Create(IDD_DLG_RESULT, FromHandle(hParent));

	m_dlgResult->GetWindowRect(rect);
	m_dlgResult->MoveWindow(r.left + 1, r.top + 18, rect.Width(), rect.Height());	// jsyoon 20160226 add

	if (::IsWindow(m_dlgResult->GetSafeHwnd()))
		m_dlgResult->ShowWindow(SW_SHOW);
}

CWnd* CResultManager::GetReportResultWnd()
{
	return m_dlgResult;
}

void CResultManager::FinalizeReport()
{
	if (m_dlgResult != NULL)
	{
		m_dlgResult->DestroyWindow();
		delete m_dlgResult;
		m_dlgResult = NULL;
	}
}

bool CResultManager::SaveResult(bool bSaveFile, bool bRemote, CString strStart)
{
	bool bRet = false;

	if (m_dlgResult != NULL)
		bRet = m_dlgResult->SaveResult(bSaveFile, bRemote, strStart);

	return bRet;
}

bool CResultManager::SaveResultDone()
{
	bool bRet = false;

	if (m_dlgResult != NULL)
		bRet = m_dlgResult->m_bSaveResultDone;

	return bRet;
}


int CResultManager::UploadData(PRSRITEMDATA result)
{
	int nRet = VTM_RV_SUCCESSED;
	if (m_dlgResult != NULL)
	{
		if (m_dlgResult->UploadDataToShare(result) != true)
		{
			nRet = VTM_RV_FAILED;
		}
	}

	return nRet;
}

void CResultManager::SetSearchMode(int iMode)
{
	if (m_dlgResult != NULL)
		m_dlgResult->SetSearchMode(iMode);
}
void CResultManager::SetConfigPath(CString str)
{
	if (m_dlgResult != NULL)
		m_dlgResult->SetConfigPath(str);
}

void CResultManager::DataClear()
{
	if (m_dlgResult != NULL)
		m_dlgResult->DataClear();
}