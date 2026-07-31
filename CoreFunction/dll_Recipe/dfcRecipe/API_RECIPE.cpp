#include "pch.h"
#include "dfcRecipe.h"
#include "API_RECIPE.h"
#include "RecipeManager.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//C_RecipeMgrDlg *m_pRcpMgrDlg = nullptr;
CRecipeManager g_RecipeManager;
C_OutRecipeFunc *g_outRcpInfo = nullptr;

VTM_RECIPE* WINAPI Recipe_GetSharedMemory ()
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState ());
	return g_pSysMapView;
}

void WINAPI Recipe_Initialize (HWND hParentWnd, CRect rc)
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState ());

	if (g_outRcpInfo == nullptr) {
		g_outRcpInfo = new C_OutRecipeFunc();
	}

	g_RecipeManager.Initialize(hParentWnd, rc);

	/*
	CWnd* hWnd = NULL;
	if ( m_pRcpMgrDlg == nullptr) {
		m_pRcpMgrDlg = new C_RecipeMgrDlg (hWnd->FromHandle (hParentWnd));
		m_pRcpMgrDlg->Create (IDD_PAGE_RECIPE, hWnd->FromHandle (hParentWnd));
		m_pRcpMgrDlg->ShowWindow (SW_SHOW);
	}
	else if (m_pRcpMgrDlg && !m_pRcpMgrDlg->IsWindowVisible ()) {
		m_pRcpMgrDlg->ShowWindow (SW_SHOW);
	}

	
	if (m_pRcpMgrDlg != nullptr)
		m_pRcpMgrDlg->MoveWindow (rc.left, rc.top, rc.Width (), rc.Height ());*/
}

CWnd* WINAPI Recipe_GetRecipehWnd ()
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState ());

	return g_RecipeManager.GetRecipeMgrWnd();
//	return m_pRcpMgrDlg;
}

void WINAPI Recipe_Finalize ()
{
	if (g_outRcpInfo) {
		delete g_outRcpInfo;
		g_outRcpInfo = NULL;
	}

	g_RecipeManager.Finalize();
	//if (m_pRcpMgrDlg) {
	//	delete m_pRcpMgrDlg;
	//	m_pRcpMgrDlg = NULL;
	//}
}

void WINAPI Recipe_SetMsg (CString strMsg)
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState ());

	//if (m_pRcpMgrDlg) m_pRcpMgrDlg->TestEdit (strMsg);
}

C_OutRecipeFunc*	WINAPI GetRecipeMgr ()
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState ());
	return g_outRcpInfo;
}