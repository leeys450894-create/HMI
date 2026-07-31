// D_LogInMgr.cpp : 구현 파일입니다.
//

#include "pch.h"
#include "ALPHA.h"
#include "D_LogInMgr.h"

#include "Manager\LogInControlMgr.h"

static CString ColName[] =
{
	_T("User Name"),
	_T("Level"),
};

static CString strLevel[] =
{
	_T("Operator"),
	_T("Engineer"),
};

enum enumCol
{
	enumCol_Value1	= 1,	// User Name
	enumCol_Value2	= 2		// User Level
};

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CD_LogInMgr 대화 상자입니다.

IMPLEMENT_DYNAMIC(CD_LogInMgr, CDialog)

CD_LogInMgr::CD_LogInMgr(CLogInControlMgr *login, CWnd* pParent /*=NULL*/)
	: CDialog(CD_LogInMgr::IDD, pParent)
	, m_pLoginMgr(login)
	, m_nCurSel(-1)
	, m_strId(_T(""))
	, m_strPW(_T(""))
	, m_nMaxUser(0)
	, m_bLoginCheck(false)
	, m_nUserLv(0)
{
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));
	lf.lfHeight = 18;
	lf.lfWeight = 1000;
	_tcscpy_s(lf.lfFaceName, _countof(lf.lfFaceName), _T("Arial"));
	VERIFY(m_font.CreateFontIndirect(&lf));
}

CD_LogInMgr::~CD_LogInMgr()
{
}

void CD_LogInMgr::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_LOGIN_TITLE,		m_labelLoginTitle);
	DDX_Control(pDX, IDC_LIST_LOG_INFO,				m_gridctrl);
	DDX_Text(pDX, IDC_EDIT_NAME,					m_strId);
	DDX_Text(pDX, IDC_EDIT_PASSWORD,				m_strPW);
}

BEGIN_MESSAGE_MAP(CD_LogInMgr, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_LOGIN,						&CD_LogInMgr::OnBnClickedLoginControl)
	ON_BN_CLICKED(IDC_BUTTON_LOGOUT,					&CD_LogInMgr::OnBnClickedLoginControl)
	ON_NOTIFY(GVN_SELCHANGED, IDC_LIST_LOG_INFO,		&CD_LogInMgr::OnGridItemActivateList)
END_MESSAGE_MAP()

BOOL CD_LogInMgr::PreTranslateMessage(MSG *pMsg)
{
    if (pMsg->message == WM_KEYDOWN)
    {
        // To prevent to close a dialog.
        switch (pMsg->wParam)
        {
            case VK_ESCAPE:
            case VK_RETURN:
            case VK_SPACE:
                return TRUE;
        }
    }

    return CDialog::PreTranslateMessage(pMsg);
}

BOOL CD_LogInMgr::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_nCurSel = -1;
	m_pLoginMgr->LinkIniFile(false);

	m_nMaxUser = m_pLoginMgr->GetLoginInfoCount();

	if (m_pLoginMgr->GetLoginStatus() == true)
	{
		m_bLoginCheck = true;
		GetDlgItem(IDC_BUTTON_LOGOUT)->ShowWindow(SW_SHOW);
	}
	else
	{
		GetDlgItem(IDC_BUTTON_LOGOUT)->ShowWindow(SW_HIDE);
	}
		
	InitLoginTitle();

	InitListControl();

	UpdateLoginInfo();

	CheckLogin();


//	EnableClose();

	return TRUE;
}

void CD_LogInMgr::InitLoginTitle()
{
	m_labelLoginTitle.SetBorder(TRUE, TRUE);
	m_labelLoginTitle.SetBkColor(RGB(200, 200, 200));
	m_labelLoginTitle.SetTextColor(RGB(100, 149, 237));
	m_labelLoginTitle.SetFontBold(TRUE);
	m_labelLoginTitle.SetFontSize(15);
}

void CD_LogInMgr::InitListControl()
{
	m_gridctrl.GetDefaultCell(FALSE, TRUE) ->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridctrl.GetDefaultCell(TRUE,  FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridctrl.GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridctrl.SetDefCellMargin(0);
	m_gridctrl.SetFixedColumnSelection(FALSE);
	m_gridctrl.SetFixedRowSelection(FALSE);
	m_gridctrl.SetSingleColSelection(FALSE);
	m_gridctrl.SetSingleRowSelection(FALSE);
	m_gridctrl.SetTrackFocusCell(FALSE);
	m_gridctrl.SetRowResize(FALSE);
	m_gridctrl.SetColumnResize(FALSE);	
	m_gridctrl.SetFont(&m_font);
}

void CD_LogInMgr::InitListLogInfo()
{
	int nMaxRow = m_nMaxUser+1;
	int nMaxCol	= 3;

	m_gridctrl.DeleteAllItems();
	m_gridctrl.SetColumnCount(nMaxCol);

	m_gridctrl.SetEditable(TRUE);
	m_gridctrl.SetRowCount(nMaxRow);

	// 0,0
	m_gridctrl.SetRowHeight(0, 22);
	m_gridctrl.SetColumnWidth(0, 40);
	m_gridctrl.SetItemState(0, 0, GVIS_READONLY);
	m_gridctrl.SetItemBkColour(0, 0, RGB(0, 0, 0));
	
	CString strData;

	// Row Setting
	for (int row = 1; row < nMaxRow; row++) 
	{
		strData.Format(_T("%02d"), row);
		m_gridctrl.SetItemText(row, 0, strData);
		m_gridctrl.SetItemState(row, 0, GVIS_READONLY);
		m_gridctrl.SetItemBkColour(row, 0, RGB(93, 93, 93));
		m_gridctrl.SetItemFgColour(row, 0, RGB(255, 255, 255));
	}

	// Col Setting
	for (int col = 1; col < nMaxCol; col++)
	{
		strData.Format(_T("%s"), ColName[col-1].GetString());
		m_gridctrl.SetItemText(0, col, strData);
		m_gridctrl.SetColumnWidth(col, 290);
		m_gridctrl.SetItemState(0, col, GVIS_READONLY);
		m_gridctrl.SetItemBkColour(0, col, RGB(93, 93, 93));
		m_gridctrl.SetItemFgColour(0, col, RGB(255, 255, 255));
	}

	for (int row = 1; row < nMaxRow; row++)
	{
		for (int col = 1; col < nMaxCol; col++)
		{
			m_gridctrl.SetItemState(row, col, GVIS_READONLY);
			m_gridctrl.SetItemBkColour(row, col, RGB(253, 254, 220));
			m_gridctrl.SetItemFgColour(row, col, RGB(0, 0, 0));
		}
	}
}

void CD_LogInMgr::UpdateLoginInfo()
{
	InitListLogInfo();

	int nRow = m_pLoginMgr->GetLoginInfoCount() + 1;

	for (int row = 1; row < nRow; row++)
	{
		CString strData;
		strData.Format(_T("%s"), m_pLoginMgr->GetLoginInfo(row-1)->s_strUserID.GetString());
		m_gridctrl.SetItemText(row, enumCol_Value1, strData);

		strData.Format(_T("%s"), strLevel[m_pLoginMgr->GetLoginInfo(row-1)->s_nUserLv].GetString());
		m_gridctrl.SetItemText(row, enumCol_Value2, strData);
	}

	m_gridctrl.Refresh();
}

void CD_LogInMgr::CheckLogin()
{
	if(!m_pLoginMgr->GetLoginStatus())	return;

	m_strId	= m_pLoginMgr->GetSelectedLoginInfo()->s_strUserID;

//	EnableControl(FALSE);

	UpdateData(FALSE);
}

void CD_LogInMgr::OnGridItemActivateList(NMHDR* pNotifyStruct, LRESULT* result)
{
	NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*) pNotifyStruct;
	long row = pNotify->iRow;
	//long col = pNotify->iColumn;

	if(1 <= row)
	{
		m_nCurSel = row - 1;
	}

	if (m_nCurSel >= m_pLoginMgr->GetLoginInfoCount())
	{
		m_strId = _T("");
		m_nUserLv = 0;
		*result = 0;
		return;
	}

	if(m_pLoginMgr->GetLoginInfo(m_nCurSel) == nullptr)
	{
		*result = 0;
		return;
	}

	m_strId = m_pLoginMgr->GetLoginInfo(m_nCurSel)->s_strUserID;
	m_nUserLv = m_pLoginMgr->GetLoginInfo(m_nCurSel)->s_nUserLv;
	m_strPW = _T("");

	UpdateData(FALSE);

	*result = 0;
}

void CD_LogInMgr::OnBnClickedLoginControl()
{
	UpdateData(TRUE);

	if(m_strId == _T(""))
	{
		AfxMessageBox(m_strHelper.STRTRANSLATOR(IDS_D_LOGIN_01));
		return;
	}

	const MSG *pMsg = GetCurrentMessage();
	int btnID = (int)pMsg->wParam;

	switch (btnID)
	{
		case IDC_BUTTON_LOGIN:
			{
				OnLogin();
			}
			break;
		case IDC_BUTTON_LOGOUT:
			{
				OnLogout();
			}
			break;
		default:
			break;
	}

	UpdateLoginInfo();

//	EnableClose();
}

void CD_LogInMgr::OnLogin()
{
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_LogInMgr::OnLogin"));
	
	UpdateData(TRUE);

	int nRet = m_pLoginMgr->SetLoninCheck(m_nCurSel, m_strId, m_strPW, m_nUserLv /*,m_cmbLevel.GetCurSel()*/);
	if(nRet == 1)
	{
	//	EnableControl(FALSE);
		m_bLoginCheck = true;
	//	AfxMessageBox(m_strHelper.STRTRANSLATOR(IDS_D_LOGIN_02));

		OnOK();
	}
	else if (nRet == 0)
		AfxMessageBox(m_strHelper.STRTRANSLATOR(IDS_D_LOGIN_03));
	else
		AfxMessageBox(m_strHelper.STRTRANSLATOR(IDS_D_LOGIN_04));
}

void CD_LogInMgr::OnLogout()
{
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_LogInMgr::OnLogout"));

	m_pLoginMgr->SetLoginStatus(0, false);

	m_strId = _T("");
	m_strPW = _T("");
	m_bLoginCheck = false;

	UpdateData(FALSE);

	AfxMessageBox(m_strHelper.STRTRANSLATOR(IDS_D_LOGIN_05));

	//EnableControl(TRUE);

	UpdateData(FALSE);

	OnOK();
}

void CD_LogInMgr::EnableControl(BOOL bStatus)
{
	GetDlgItem(IDC_EDIT_NAME)->EnableWindow(bStatus);
	GetDlgItem(IDC_EDIT_PASSWORD)->EnableWindow(bStatus);
}

void CD_LogInMgr::EnableClose()
{
	GetDlgItem(IDOK)->EnableWindow(m_pLoginMgr->GetLoginStatus());
}

bool CD_LogInMgr::IsLoginStatus()
{
	bool ret = false;
//	bool ret = ( (m_strId.IsEmpty() == true) || (m_strId.IsEmpty() == true) );
	ret = m_bLoginCheck;

	return ret;
}
