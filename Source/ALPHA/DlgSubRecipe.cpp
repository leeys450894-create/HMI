// DlgSubRecipe.cpp : 구현 파일입니다.
//

#include "pch.h"
#include "ALPHA.h"
#include "DlgSubRecipe.h"
#include "afxdialogex.h"

//#include "DlgSubRcpFile.h"

#include "AuroraMgr.h"
#include "Recipe\RecipeEditMgr.h"
#include "Manager/RecipeMgr.h"
#include "SystemParamShare.h"
// CDlgSubRecipe 대화 상자입니다.

IMPLEMENT_DYNAMIC(CDlgSubRecipe, CMainSubDlgWin)

CDlgSubRecipe::CDlgSubRecipe(CAuroraMgr *pAuroraMgr, CWnd* pParent /*=NULL*/)
	: CMainSubDlgWin(IDD_DLG_SUB_RECIPE, pParent)
	, m_pAuroraMgr(pAuroraMgr)
	, m_pRecipeEditMgr(pAuroraMgr->GetRecipeEditMgrPtr())
{
	m_pSubRcpFile = nullptr;
}

CDlgSubRecipe::~CDlgSubRecipe()
{
	/*if (m_pSubRcpFile)
	{
		m_pSubRcpFile->DestroyWindow();
		delete m_pSubRcpFile;
		m_pSubRcpFile = nullptr;
	}*/
}

void CDlgSubRecipe::DoDataExchange(CDataExchange* pDX)
{
	CMainSubDlgWin::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_RCP_TITLE, m_LabelInfo);
}


BEGIN_MESSAGE_MAP(CDlgSubRecipe, CMainSubDlgWin)
	ON_BN_CLICKED(IDC_RCP_BTN_NEW, &CDlgSubRecipe::OnBnClickedRcpBtnNew)
	ON_BN_CLICKED(IDC_RCP_BTN_OPEN, &CDlgSubRecipe::OnBnClickedRcpBtnOpen)
	ON_BN_CLICKED(IDC_RCP_BTN_SAVEAS, &CDlgSubRecipe::OnBnClickedRcpBtnSaveas)
	ON_BN_CLICKED(IDC_RCP_BTN_SAVE, &CDlgSubRecipe::OnBnClickedRcpBtnSave)
	ON_BN_CLICKED(IDC_RCP_BTN_DELETE, &CDlgSubRecipe::OnBnClickedRcpBtnDelete)
END_MESSAGE_MAP()


// CDlgSubRecipe 메시지 처리기입니다.


BOOL CDlgSubRecipe::OnInitDialog()
{
	CMainSubDlgWin::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	//---------------------------------------------------------------------
	// 타이틀에 색상을 입혀준다.
	//---------------------------------------------------------------------
	m_LabelInfo.SetBorder(TRUE, TRUE);
	m_LabelInfo.SetBkColor(RGB(0, 32, 0));
	m_LabelInfo.SetTextColor(RGB(255, 255, 192));
	m_LabelInfo.SetFontBold(TRUE);

	CRect rtROI;
	GetDlgItem(IDC_STATIC_RECIPE_FILE)->GetWindowRect(&rtROI);
	ScreenToClient(&rtROI);

	CString strAppPath, strRecipePath, strMCCLogPath;
	strAppPath.Format(_T("%s"), CSystemParamShare::GetInstance()->GetAppFolderPath());
	strRecipePath.Format(_T("%s"), CSystemParamShare::GetInstance()->GetLoggerFolderPath());
	strMCCLogPath.Format(_T("%s%s"), strRecipePath, _T("MCC"));

	CRecipeMgr::GetInstance()->Initialize(GetSafeHwnd(), rtROI, m_pAuroraMgr->GetSystemInfo().name, strAppPath, strAppPath, strMCCLogPath);

	m_pAuroraMgr->GetRecipeEditMgrPtr()->SetRecipePtr();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CDlgSubRecipe::OnBnClickedRcpBtnNew()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_MainRecipe::OnBnClickedRcpBtnNew"));

	/*CString sFilter = _T("Recipe Files (*.rcp) | *.rcp | All Files(*.*) | *.* |");
	CString sFileName = m_pRecipeEditMgr->GetRecipeFolderPath() + _T("*.rcp");

	CFileDialog dlg(FALSE, _T("rcp"), sFileName, OFN_HIDEREADONLY | OFN_NOCHANGEDIR, sFilter);
	dlg.m_ofn.lpstrInitialDir = _T("Recipe");

	if (AfxMessageBox(_T("Create New Recipe"), MB_OKCANCEL) == IDOK)
	{
		if (dlg.DoModal() == IDOK)
		{
			if (0 < m_pRecipeEditMgr->NewRecipe(dlg.GetFileTitle()))
			{
				if (m_pSubRcpFile)
				{
					m_pSubRcpFile->EnableWindow(TRUE);
					m_pSubRcpFile->SetDataToUI(m_pRecipeEditMgr->GetRecipeDataPtr()->GetParam_RecipeInfo());
				}
					

				GetParent()->SendMessage(WM_MSG_UPDATE_RECIPE_INFORMATION, NULL, NULL);

				AfxMessageBox(_T("Make New Recipe OK."));
			}
		}
	}*/
}


void CDlgSubRecipe::OnBnClickedRcpBtnOpen()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_MainRecipe::OnBnClickedRcpBtnOpen"));

	//CString sFilter = _T("Recipe Files (*.rcp) | *.rcp | All Files(*.*) | *.* |");
	//CString sFileName = m_pRecipeEditMgr->GetRecipeFolderPath() + _T("*.rcp");

	//CFileDialog dlg(TRUE, _T("rcp"), sFileName, OFN_HIDEREADONLY | OFN_NOCHANGEDIR, sFilter);
	//dlg.m_ofn.lpstrInitialDir = _T("Recipe");

	//if (dlg.DoModal() == IDOK)
	//{
	//	CString strRcpFile = dlg.GetFileTitle();

	//	if (AfxMessageBox(_T("Load This Recipe : ") + strRcpFile, MB_OKCANCEL) == IDOK)
	//	{
	//		if (0 < m_pRecipeEditMgr->OpenRecipe(strRcpFile))
	//		{
	//			if (m_pSubRcpFile)
	//			{
	//				m_pSubRcpFile->EnableWindow(TRUE);
	//				m_pSubRcpFile->SetDataToUI(m_pRecipeEditMgr->GetRecipeDataPtr()->GetParam_RecipeInfo());
	//			}

	//			GetParent()->SendMessage(WM_MSG_UPDATE_RECIPE_INFORMATION, NULL, NULL);

	//		//	AfxMessageBox(_T("Load OK."));
	//		}
	//		else
	//		{
	//			AfxMessageBox(_T("Fail : Load"));
	//		}
	//	}
	//}
}


void CDlgSubRecipe::OnBnClickedRcpBtnSaveas()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_MainRecipe::OnBnClickedRcpBtnSaveas"));

	//const CString sTemp = m_pRecipeEditMgr->GetRecipeID();
	//if (sTemp.IsEmpty())
	//{
	//	AfxMessageBox(_T("Please, select a recipe file!"));
	//	return;
	//}

	//CString sFilter = _T("Recipe Files (*.rcp) | *.rcp | All Files(*.*) | *.* |");
	//CString sFileName = m_pRecipeEditMgr->GetRecipeFolderPath() + _T("*.rcp");

	//CFileDialog dlg(FALSE, _T("rcp"), sFileName, OFN_HIDEREADONLY | OFN_NOCHANGEDIR, sFilter);
	//dlg.m_ofn.lpstrInitialDir = _T("Recipe");

	//if (AfxMessageBox(_T("SaveAs This Recipe : ") + sTemp, MB_OKCANCEL) == IDOK)
	//{
	//	if (0 < m_pRecipeEditMgr->OpenRecipe(sTemp))
	//	{
	//		if (dlg.DoModal() == IDOK)
	//		{
	//			CString sTempDlg = dlg.GetFileTitle();

	//			if (m_pRecipeEditMgr->SaveRecipeAs(sTempDlg))
	//			{
	//				m_pRecipeEditMgr->OpenRecipe(sTempDlg);
	//				GetParent()->SendMessage(WM_MSG_UPDATE_RECIPE_INFORMATION, NULL, NULL);

	//			//	AfxMessageBox(_T("SaveAs OK."));
	//			}
	//			else
	//			{
	//				AfxMessageBox(_T("Fail : SaveAs"));
	//			}
	//		}
	//	}
	//	else
	//	{
	//		AfxMessageBox(_T("Fail : SaveAs"));
	//	}
	//}
}


void CDlgSubRecipe::OnBnClickedRcpBtnSave()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_MainRecipe::OnBnClickedRcpBtnSave"));

	//const CString sTemp = m_pRecipeEditMgr->GetRecipeID();
	//if (sTemp.IsEmpty())
	//	return;

	//if (AfxMessageBox(_T("Save This Recipe : ") + sTemp, MB_OKCANCEL) == IDOK)
	//{
	//	if (m_pSubRcpFile) m_pSubRcpFile->GetGridData(m_pRecipeEditMgr->GetRecipeDataPtr()->GetParam_RecipeInfo());

	//	if (m_pRecipeEditMgr->SaveRecipe(sTemp) != true)
	//	{
	//		AfxMessageBox(_T("Fail : Save"));
	//	}
	//	/*else
	//	{
	//		AfxMessageBox(_T("Save OK."));
	//	}*/
	//}
}


void CDlgSubRecipe::OnBnClickedRcpBtnDelete()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	/*const CString sTemp = m_pRecipeEditMgr->GetRecipeID();

	if (AfxMessageBox(_T("Do you want delete recipe file?"), MB_OKCANCEL) == IDOK)
	{
		m_pRecipeEditMgr->DeleteRecipe(sTemp);

		GetParent()->SendMessage(WM_MSG_UPDATE_RECIPE_INFORMATION, NULL, NULL);
	}*/
}


void CDlgSubRecipe::UpdateRecipeInformation()
{
	/*if (m_pSubRcpFile)
	{
		m_pSubRcpFile->EnableWindow(TRUE);
		m_pSubRcpFile->SetDataToUI(m_pRecipeEditMgr->GetRecipeDataPtr()->GetParam_RecipeInfo());
	}*/
}
