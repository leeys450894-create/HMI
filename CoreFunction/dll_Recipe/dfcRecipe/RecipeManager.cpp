#include "RecipeManager.h"

#include "afxdialogex.h"
#include "BaseDialog.h"
#include "RecipeMgrDlg.h"

CRecipeManager::CRecipeManager()
{
	m_pRcpMgrDlg = nullptr;
}

CRecipeManager::~CRecipeManager()
{
	//delete m_pRcpMgrDlg; //Finalize 에서 해줌
}

BEGIN_MESSAGE_MAP(CRecipeManager, CWnd)
END_MESSAGE_MAP()

void CRecipeManager::Initialize(HWND hParent, CRect r)
{
	// ResultManager 윈도우 생성.
	Create(NULL, NULL, WS_VISIBLE | WS_CHILD, CRect(0, 0, 0, 0), this, 1);

	// ResultManager 다이얼로그 생성.
	CreateDlg(hParent, r);
}

void CRecipeManager::CreateDlg(HWND hParent, CRect r)
{
	CRect rect;

	if (!hParent) return;

	//if (::IsWindow(m_pRcpMgrDlg->GetSafeHwnd()))
	//{
	//	m_dlgResult->DestroyWindow();
	//}

	//m_dlgResult = new CDlgResult(FromHandle(hParent));

	//m_dlgResult->Create(IDD_DLG_RESULT, FromHandle(hParent));

	//m_dlgResult->GetWindowRect(rect);
	//m_dlgResult->MoveWindow(r.left + 1, r.top + 18, rect.Width(), rect.Height());	// jsyoon 20160226 add

	//if (::IsWindow(m_dlgResult->GetSafeHwnd()))
	//	m_dlgResult->ShowWindow(SW_SHOW);

	CWnd* hWnd = NULL;
	if (m_pRcpMgrDlg == nullptr) {
		m_pRcpMgrDlg = new C_RecipeMgrDlg(FromHandle(hParent));
		m_pRcpMgrDlg->Create(IDD_PAGE_RECIPE, FromHandle(hParent));
		m_pRcpMgrDlg->ShowWindow(SW_SHOW);
	}
	else if (m_pRcpMgrDlg && !m_pRcpMgrDlg->IsWindowVisible()) {
		m_pRcpMgrDlg->ShowWindow(SW_SHOW);
	}

	if (m_pRcpMgrDlg != nullptr)
		m_pRcpMgrDlg->MoveWindow(r.left, r.top, r.Width(), r.Height());
}

CWnd* CRecipeManager::GetRecipeMgrWnd()
{
	return m_pRcpMgrDlg;
}

void CRecipeManager::Finalize()
{
	if (m_pRcpMgrDlg != NULL)
	{
		delete m_pRcpMgrDlg;
		m_pRcpMgrDlg = NULL;
	}
}