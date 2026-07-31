// DlgSubConfigLoginFile.cpp : 구현 파일입니다.
//

#include "pch.h"
#include "ALPHA.h"
#include "DlgSubConfigLoginFile.h"
#include "afxdialogex.h"

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
	enumCol_Value1 = 1,	// User Name
	enumCol_Value2 = 2		// User Level
};

// CDlgSubConfigLoginFile 대화 상자입니다.

IMPLEMENT_DYNAMIC(CDlgSubConfigLoginFile, CDialogEx)

CDlgSubConfigLoginFile::CDlgSubConfigLoginFile(CLogInControlMgr* pLoginMgr, CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLG_SUB_CONFIG_USER, pParent)
	, m_pLoginMgr(pLoginMgr)
	, m_nCurSel(-1)
	, m_strId(_T(""))
	, m_strPW(_T(""))

{
	LOGFONT lf;
	memset(&lf, 0, sizeof(LOGFONT));
	lf.lfHeight = 18;
	lf.lfWeight = 1000;
	_tcscpy_s(lf.lfFaceName, _countof(lf.lfFaceName), _T("Arial"));
	VERIFY(m_font.CreateFontIndirect(&lf));
}

CDlgSubConfigLoginFile::~CDlgSubConfigLoginFile()
{
}

void CDlgSubConfigLoginFile::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_STATIC_LOGIN_TITLE, m_LabelInfo);
	DDX_Control(pDX, IDC_LIST_LOG_INFO, m_gridList);
	DDX_Control(pDX, IDC_COMBO_LEVEL, m_cmbLevel);
	DDX_Text(pDX, IDC_EDIT_NAME, m_strId);
	DDX_Text(pDX, IDC_EDIT_PASSWORD, m_strPW);
}


BEGIN_MESSAGE_MAP(CDlgSubConfigLoginFile, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CDlgSubConfigLoginFile::OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, &CDlgSubConfigLoginFile::OnBnClickedButtonDelete)
	ON_NOTIFY(GVN_SELCHANGED, IDC_LIST_LOG_INFO, &CDlgSubConfigLoginFile::OnGridItemActivateList)
	ON_BN_CLICKED(IDC_BUTTON_LOAD, &CDlgSubConfigLoginFile::OnBnClickedButtonLoad)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CDlgSubConfigLoginFile::OnBnClickedButtonSave)
END_MESSAGE_MAP()


// CDlgSubConfigLoginFile 메시지 처리기입니다.


BOOL CDlgSubConfigLoginFile::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
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

	return CDialogEx::PreTranslateMessage(pMsg);
}


BOOL CDlgSubConfigLoginFile::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	SetBackgroundColor(RGB(128, 128, 128));

	//---------------------------------------------------------------------
	// 타이틀에 색상을 입혀준다.
	//---------------------------------------------------------------------
	m_LabelInfo.SetBorder(TRUE, TRUE);
	m_LabelInfo.SetBkColor(RGB(0, 32, 0));
	m_LabelInfo.SetTextColor(RGB(255, 255, 192));
	m_LabelInfo.SetFontBold(TRUE);

	m_nCurSel = -1;
	m_pLoginMgr->LinkIniFile(false);

	//---------------------------------------------------------------------
	// 리스트의 정보를 채워준다.
	//---------------------------------------------------------------------
	__InitListControl();
	__RefreshListData();

	InitLoginLevel();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgSubConfigLoginFile::__InitListControl()
{
	m_gridList.GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridList.GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridList.GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridList.SetDefCellMargin(0);
	m_gridList.SetFixedColumnSelection(FALSE);
	m_gridList.SetFixedRowSelection(FALSE);
	m_gridList.SetSingleColSelection(FALSE);
	m_gridList.SetSingleRowSelection(FALSE);
	m_gridList.SetTrackFocusCell(FALSE);
	m_gridList.SetRowResize(FALSE);
	m_gridList.SetColumnResize(FALSE);
}

void CDlgSubConfigLoginFile::__InitialListData()
{
	int nMaxRow = m_pLoginMgr->GetLoginInfoCount() + 1;
	int nMaxCol = 3;

	m_gridList.DeleteAllItems();
	m_gridList.SetColumnCount(nMaxCol);

	m_gridList.SetEditable(TRUE);
	m_gridList.SetRowCount(nMaxRow);

	// 0,0
	m_gridList.SetRowHeight(0, 22);
	m_gridList.SetColumnWidth(0, 40);
	m_gridList.SetItemState(0, 0, GVIS_READONLY);
	m_gridList.SetItemBkColour(0, 0, RGB(0, 0, 0));

	CString strData;

	// Row Setting
	for (int row = 1; row < nMaxRow; row++)
	{
		strData.Format(_T("%02d"), row);
		m_gridList.SetItemText(row, 0, strData);
		m_gridList.SetItemState(row, 0, GVIS_READONLY);
		m_gridList.SetItemBkColour(row, 0, RGB(93, 93, 93));
		m_gridList.SetItemFgColour(row, 0, RGB(255, 255, 255));
	}

	// Col Setting
	for (int col = 1; col < nMaxCol; col++)
	{
		strData.Format(_T("%s"), ColName[col - 1].GetString());
		m_gridList.SetItemText(0, col, strData);
		m_gridList.SetColumnWidth(col, 290);
		m_gridList.SetItemState(0, col, GVIS_READONLY);
		m_gridList.SetItemBkColour(0, col, RGB(93, 93, 93));
		m_gridList.SetItemFgColour(0, col, RGB(255, 255, 255));
	}

	for (int row = 1; row < nMaxRow; row++)
	{
		for (int col = 1; col < nMaxCol; col++)
		{
			m_gridList.SetItemState(row, col, GVIS_READONLY);
			m_gridList.SetItemBkColour(row, col, RGB(253, 254, 220));
			m_gridList.SetItemFgColour(row, col, RGB(0, 0, 0));
		}
	}
}

void CDlgSubConfigLoginFile::__RefreshListData()
{
	__InitialListData();

	int nRow = m_pLoginMgr->GetLoginInfoCount() + 1;

	for (int row = 1; row < nRow; row++)
	{
		CString strData;
		strData.Format(_T("%s"), m_pLoginMgr->GetLoginInfo(row - 1)->s_strUserID.GetString());
		m_gridList.SetItemText(row, enumCol_Value1, strData);

		strData.Format(_T("%s"), strLevel[m_pLoginMgr->GetLoginInfo(row - 1)->s_nUserLv].GetString());
		m_gridList.SetItemText(row, enumCol_Value2, strData);
	}

	m_gridList.Refresh();
}

void CDlgSubConfigLoginFile::InitLoginLevel()
{
	m_cmbLevel.ResetContent();

	for (int i = 0; i < sizeof(strLevel) / sizeof(CString); i++)
	{
		m_cmbLevel.AddString(strLevel[i]);
	}

	m_cmbLevel.SetCurSel(0);
}


void CDlgSubConfigLoginFile::OnBnClickedButtonAdd()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_LogInMgr::OnAdd"));

	UpdateData(TRUE);

	if (!m_pLoginMgr->SetParam_Add(m_strId, m_strPW, m_cmbLevel.GetCurSel()))
	{
		AfxMessageBox(m_strHelper.STRTRANSLATOR(IDS_D_LOGIN_06));
		m_strId = _T("");
		UpdateData(FALSE);

		return;
	}
	m_strPW = _T("");

	UpdateData(FALSE);

	__RefreshListData();
}


void CDlgSubConfigLoginFile::OnBnClickedButtonDelete()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_LogInMgr::OnDelete"));

	if (!m_pLoginMgr->SetParam_del(m_nCurSel))
	{
		AfxMessageBox(m_strHelper.STRTRANSLATOR(IDS_D_LOGIN_07));
		return;
	}

	if (AfxMessageBox(m_strHelper.STRTRANSLATOR(IDS_D_LOGIN_08), MB_YESNO | MB_ICONQUESTION) != IDYES)
		return;

	m_strId = _T("");
	m_strPW = _T("");

	m_nCurSel = -1;

	UpdateData(FALSE);

	__RefreshListData();
}

void CDlgSubConfigLoginFile::OnGridItemActivateList(NMHDR* pNotifyStruct, LRESULT* result)
{
	NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*)pNotifyStruct;
	long row = pNotify->iRow;

	if (1 <= row)
	{
		m_nCurSel = row - 1;
	}

	if (m_nCurSel >= m_pLoginMgr->GetLoginInfoCount())
	{
		m_strId = _T("");

		*result = 0;
		return;
	}

	if (m_pLoginMgr->GetLoginInfo(m_nCurSel) == nullptr)
	{
		*result = 0;
		return;
	}

	m_strId = m_pLoginMgr->GetLoginInfo(m_nCurSel)->s_strUserID;
	m_strPW = m_pLoginMgr->GetLoginInfo(m_nCurSel)->s_strPassword;
	m_cmbLevel.SetCurSel(m_pLoginMgr->GetLoginInfo(m_nCurSel)->s_nUserLv);

	UpdateData(FALSE);

	*result = 0;
}


void CDlgSubConfigLoginFile::OnBnClickedButtonLoad()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CDlgSubConfigLoginFile::OnBnClickedButtonSave()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
