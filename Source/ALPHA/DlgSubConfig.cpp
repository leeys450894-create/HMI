// DlgSubConfig.cpp : 구현 파일입니다.
//

#include "pch.h"
#include "ALPHA.h"
#include "DlgSubConfig.h"
#include "afxdialogex.h"

#include "AuroraMgr.h"
//#include "DlgSubConfigFile.h"
#include "DlgSubConfigMotionFile.h"
#include "DlgSubConfigModuleFile.h"
#include "DlgSubConfigLoginFile.h"
#include "Manager\PositionConfigMgr.h"
#include "Manager\MaintControlMgr.h"
#include "Manager\SystemConfigMgr.h"
#include "D_SetLampTime.h"
// CDlgSubConfig 대화 상자입니다.

static CString ColumnName[] =
{
	_T("No."),
	_T("Name"),
	_T("Pos"),
};
static CString RowName[] =
{
	_T("Safety Y"),
	_T("Safety Head1 X"),
	_T("Safety Head2 X"),
	_T("Safety Z"),

	_T("ALIGN POS"),
	_T("ALIGN To MEAS GAP"),

};
static long ColumnSize[] =
{
	65,
	130,
	100,
};
enum enumColumnName
{
	enumColumnName_No = 0,
	enumColumnName_Value1,
	enumColumnName_Value2,
	enumColumnName_End,
};

static CString ColumnNameHeadOffset[] =
{
	_T("NAME"),
	_T("HEAD1 X"),
	_T("HEAD1 Y"),
	_T("HEAD1 Z"),
	_T("HEAD2 X"),
	_T("HEAD2 Y"),
	_T("HEAD2 Z"),
};
static CString RowNameHeadOffset[] =
{
	_T("WAD"),
	_T("시야각"),
	_T("화질"),
	_T("면계측"),		// LYS 2026-07-02 안쓰는 Mode 삭제
};
static long ColumnSizeHeadOffset[] =
{
	100,
	100,
	100,
	100,
	100,
	100,
	100,
};
enum enumColumnNameHeadOffset
{
	enumColumnNameHeadOffset_No = 0,
	enumColumnNameHeadOffset_Value1,
	enumColumnNameHeadOffset_Value2,
	enumColumnNameHeadOffset_Value3,
	enumColumnNameHeadOffset_Value4,
	enumColumnNameHeadOffset_Value5,
	enumColumnNameHeadOffset_Value6,
	enumColumnNameHeadOffset_End,
};

#define RowItem 6
//#define RowItemHeadOffset 4
#define RowItemHeadOffset 3

IMPLEMENT_DYNAMIC(CDlgSubConfig, CMainSubDlgWin)

CDlgSubConfig::CDlgSubConfig(CAuroraMgr *pAuroraMgr, CWnd* pParent /*=NULL*/)
	: CMainSubDlgWin(IDD_DLG_SUB_CONFIG, pParent)
	, m_pAuroraMgr(pAuroraMgr)
	, m_iCurFileView(0)
	, m_pPositionConfigMgr(pAuroraMgr->GetPositionConfigMgrPtr())
	, m_pMaintControlMgr(pAuroraMgr->GetMaintControlMgr())
	, m_pSystemConfigMgr(pAuroraMgr->GetSystemConfigMgrPtr())
{
	m_pSubConfigFile = nullptr;
	m_pSubConfigMotionFile = nullptr;
	m_pSubConfigModuleFile = nullptr;
	m_pSubConfigLogInFile = nullptr;
}

CDlgSubConfig::~CDlgSubConfig()
{
	/*if (m_pSubConfigFile)
	{
		m_pSubConfigFile->DestroyWindow();
		delete m_pSubConfigFile;
		m_pSubConfigFile = nullptr;
	}
	*/

	if (m_pSubConfigMotionFile)
	{
		m_pSubConfigMotionFile->DestroyWindow();
		delete m_pSubConfigMotionFile;
		m_pSubConfigMotionFile = nullptr;
	}

	if (m_pSubConfigModuleFile)
	{
		m_pSubConfigModuleFile->DestroyWindow();
		delete m_pSubConfigModuleFile;
		m_pSubConfigModuleFile = nullptr;
	}

	if (m_pSubConfigLogInFile)
	{
		m_pSubConfigLogInFile->DestroyWindow();
		delete m_pSubConfigLogInFile;
		m_pSubConfigLogInFile = nullptr;
	}
}

void CDlgSubConfig::DoDataExchange(CDataExchange* pDX)
{
	CMainSubDlgWin::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_GRID_LIST_CONFIG_LOADING_POS, m_gridList);
	DDX_Control(pDX, IDC_STATIC_CONFIG_LOADING_POS, m_LabelInfo[0]);
	DDX_Control(pDX, IDC_GRID_LIST_CONFIG_HEAD_OFFSET, m_gridHeadOffset);
	DDX_Control(pDX, IDC_STATIC_CONFIG_HEAD_OFFSET, m_LabelInfo[1]);
}


BEGIN_MESSAGE_MAP(CDlgSubConfig, CMainSubDlgWin)
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_BUTTON_CONFIG, &CDlgSubConfig::OnBnClickedButtonConfig)
	ON_BN_CLICKED(IDC_BUTTON_CONFIG_MOTION, &CDlgSubConfig::OnBnClickedButtonConfigMotion)
	ON_BN_CLICKED(IDC_BUTTON_CONFIG_MODULE, &CDlgSubConfig::OnBnClickedButtonConfigModule)
	ON_BN_CLICKED(IDC_BUTTON_LOAD, &CDlgSubConfig::OnBnClickedButtonLoad)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CDlgSubConfig::OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_CONFIG_USER, &CDlgSubConfig::OnBnClickedButtonConfigUser)
	ON_BN_CLICKED(IDC_BUTTON_LAMP_TIME, &CDlgSubConfig::OnBnClickedButtonLampTime)
	ON_BN_CLICKED(IDC_BUTTON_LOAD_MOTION, &CDlgSubConfig::OnBnClickedButtonLoadMotion)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_MOTION, &CDlgSubConfig::OnBnClickedButtonSaveMotion)
	ON_BN_CLICKED(IDC_BUTTON_SET_COLOR, &CDlgSubConfig::OnBnClickedButtonSetColor)
END_MESSAGE_MAP()


// CDlgSubConfig 메시지 처리기입니다.


BOOL CDlgSubConfig::OnInitDialog()
{
	CMainSubDlgWin::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	CRect rtROI;
	GetDlgItem(IDC_STATIC_CONFIG_FILE)->GetWindowRect(&rtROI);
	ScreenToClient(&rtROI);

	/*m_pSubConfigFile = new CDlgSubConfigFile;
	m_pSubConfigFile->Create(IDD_DLG_SUB_CONFIG_FILE, this);
	m_pSubConfigFile->ShowWindow(SW_HIDE);
	m_pSubConfigFile->MoveWindow(rtROI);
	*/
	m_pSubConfigMotionFile = new CDlgSubConfigMotionFile(m_pAuroraMgr->GetMaintControlMgr()->GetMotorControlMgrPtr());
	m_pSubConfigMotionFile->Create(IDD_DLG_SUB_CONFIG_MOTION_FILE, this);
	m_pSubConfigMotionFile->ShowWindow(SW_HIDE);
	m_pSubConfigMotionFile->MoveWindow(rtROI);

	m_pSubConfigModuleFile = new CDlgSubConfigModuleFile(m_pAuroraMgr);
	m_pSubConfigModuleFile->Create(IDD_DLG_SUB_CONFIG_MODULE_FILE, this);
	m_pSubConfigModuleFile->ShowWindow(SW_HIDE);
	m_pSubConfigModuleFile->MoveWindow(rtROI);

	m_pSubConfigLogInFile = new CDlgSubConfigLoginFile(m_pAuroraMgr->GetLogInControlMgrPtr());
	m_pSubConfigLogInFile->Create(IDD_DLG_SUB_CONFIG_USER, this);
	m_pSubConfigLogInFile->ShowWindow(SW_HIDE);
	m_pSubConfigLogInFile->MoveWindow(rtROI);

	

	m_iCurFileView = 1;
	RefreshFileView(TRUE);

	//---------------------------------------------------------------------
	// 타이틀에 색상을 입혀준다.
	//---------------------------------------------------------------------
	for (size_t i = 0; i < 2; i++)
	{
		m_LabelInfo[i].SetBorder(TRUE, TRUE);
		m_LabelInfo[i].SetBkColor(RGB(0, 32, 0));
		m_LabelInfo[i].SetTextColor(RGB(255, 255, 192));
		m_LabelInfo[i].SetFontBold(TRUE);
	}
	

	//---------------------------------------------------------------------
	// 리스트를 초기화 한다.
	//---------------------------------------------------------------------
	__InitListControl_LoadPos();
	__InitialListData_LoadPos();

	__InitListControl_HeadOffset();
	__InitialListData_HeadOffset();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CDlgSubConfig::RefreshFileView(BOOL bShow)
{
	//m_pSubConfigFile->ShowWindow(SW_HIDE);
	m_pSubConfigMotionFile->ShowWindow(SW_HIDE);
	m_pSubConfigModuleFile->ShowWindow(SW_HIDE);
	m_pSubConfigLogInFile->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_BUTTON_LOAD_MOTION)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_BUTTON_SAVE_MOTION)->ShowWindow(SW_HIDE);

	if (bShow == TRUE)
	{
		switch (m_iCurFileView)
		{
		case 0:
		//	m_pSubConfigFile->ShowWindow(SW_SHOW);
			break;
		case 1:
			m_pSubConfigMotionFile->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_BUTTON_LOAD_MOTION)->ShowWindow(SW_SHOW);
//			GetDlgItem(IDC_BUTTON_SAVE_MOTION)->ShowWindow(SW_SHOW);	// jsyoon 2022.05.13 remove
			break;
		case 2:
			m_pSubConfigModuleFile->ShowWindow(SW_SHOW);
			break;
		case 3:
			m_pSubConfigLogInFile->ShowWindow(SW_SHOW);
			break;
		default:
			break;
		}
	}
}


void CDlgSubConfig::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CMainSubDlgWin::OnShowWindow(bShow, nStatus);

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	if (bShow == TRUE)
	{
		RefreshFileView(bShow);
	}
}


void CDlgSubConfig::OnBnClickedButtonConfig()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CDlgSubConfig::OnBnClickedButtonConfig"));

	m_iCurFileView = 0;
	RefreshFileView(TRUE);
}


void CDlgSubConfig::OnBnClickedButtonConfigMotion()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CDlgSubConfig::OnBnClickedButtonConfigMotion"));

	m_iCurFileView = 1;
	RefreshFileView(TRUE);
}


void CDlgSubConfig::OnBnClickedButtonConfigModule()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CDlgSubConfig::OnBnClickedButtonConfigModule"));

	m_iCurFileView = 2;
	RefreshFileView(TRUE);
}

void CDlgSubConfig::__InitListControl_LoadPos()
{
	m_gridList.GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridList.GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridList.GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridList.SetEditable(FALSE);
	m_gridList.SetDefCellMargin(0);
	m_gridList.SetFixedColumnSelection(FALSE);
	m_gridList.SetFixedRowSelection(FALSE);
	m_gridList.SetSingleColSelection(FALSE);
	m_gridList.SetSingleRowSelection(FALSE);
	m_gridList.SetTrackFocusCell(FALSE);
	m_gridList.SetRowResize(FALSE);
	m_gridList.SetColumnResize(FALSE);
}


void CDlgSubConfig::__InitialListData_LoadPos()
{
	//=====================================================================
	//	Grid Control header 부
	//=====================================================================
	m_gridList.DeleteAllItems();
	m_gridList.SetColumnCount(enumColumnName_End);
	m_gridList.SetFixedRowCount(1);

	for (long n = 0; n < enumColumnName_End; ++n)
	{
		m_gridList.SetColumnWidth(n, ColumnSize[n]);
		m_gridList.SetItemText(0, n, ColumnName[n]);
		m_gridList.SetItemState(0, n, GVIS_READONLY);
		m_gridList.SetItemBkColour(0, n, RGB(172, 172, 172));
		m_gridList.SetItemFgColour(0, n, RGB(255, 255, 255));
	}

	// Items + Header
	m_gridList.SetEditable(TRUE);
	m_gridList.SetRowCount(RowItem + 1);

	m_gridList.SetRowHeight(0, 28);
	for (long i = 0; i < RowItem; i++)
	{
		m_gridList.SetRowHeight(i + 1, 22);
	}

	__RefreshListData_LoadPos();
}


void CDlgSubConfig::__RefreshListData_LoadPos()
{
	m_gridList.SetRedraw(FALSE);

	double dPos[RowItem];
	CString strPos = _T("");

	stPositionParam stPos = m_pPositionConfigMgr->GetPositionParam();
	dPos[0] = stPos.dSafetyStageX;
	dPos[1] = stPos.dSafetyHeadY1;
	dPos[2] = stPos.dSafetyHeadY2;
	dPos[3] = stPos.dSafetyZ;
	dPos[4] = stPos.dPickerStageX;
	dPos[5] = stPos.dAlignToMeasGap;

	for (long i = 0; i < RowItem; i++)
	{
		CString strNo;
		strNo.Format(_T("[-%d-]"), i + 1);

		m_gridList.GetCell(i + 1, enumColumnName_No)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridList.SetItemState(i + 1, enumColumnName_No, GVIS_READONLY);
		m_gridList.SetItemText(i + 1, enumColumnName_No, strNo);

		// Name		
		m_gridList.SetItemText(i + 1, enumColumnName_Value1, RowName[i]);
		m_gridList.SetItemState(i + 1, enumColumnName_Value1, GVIS_READONLY);

		// Position
		strPos.Format(_T("%.4f"), dPos[i]);
		m_gridList.SetItemText(i + 1, enumColumnName_Value2, strPos);
		m_gridList.SetItemState(i + 1, enumColumnName_Value2, GVIS_READONLY);

		for (long n = 0; n < enumColumnName_End; ++n)
		{
			m_gridList.SetItemFgColour(i + 1, n, RGB(256, 0, 192));		// RGB(0, 0, 0)
		}
	}

//	m_gridList.ExpandToFit(TRUE);
	m_gridList.ExpandColumnsToFit(TRUE);
	m_gridList.SetRedraw(TRUE);
}

void CDlgSubConfig::OnBnClickedButtonLoad()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_pPositionConfigMgr->LinkIniFile(false);
	m_pMaintControlMgr->LinkIniFile(false);
	m_pSystemConfigMgr->LinkIniFile(false);

	__RefreshListData_LoadPos();
	__RefreshListData_HeadOffset();
}


void CDlgSubConfig::OnBnClickedButtonSave()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CDlgSubConfig::OnBnClickedButtonConfigUser()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CDlgSubConfig::OnBnClickedButtonConfigUser"));

	m_iCurFileView = 3;
	RefreshFileView(TRUE);
}

void CDlgSubConfig::RefreshListData()
{
	if(m_pSubConfigMotionFile != nullptr) m_pSubConfigMotionFile->__RefreshListData();
}

void CDlgSubConfig::OnBnClickedButtonLampTime()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	CWindowEventHandler::GetInstance()->PostMessageToUI(enumWndID_FRAME, WM_MSG_WRITE_LAMP_TIME, -1, false);
	CD_SetLampTime dlg(m_pAuroraMgr, this);
	if (dlg.DoModal() == IDOK)
	{
		;
	}

	CWindowEventHandler::GetInstance()->PostMessageToUI(enumWndID_FRAME, WM_MSG_WRITE_LAMP_TIME, -1, true);
}


void CDlgSubConfig::OnBnClickedButtonLoadMotion()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if(m_pSubConfigMotionFile != nullptr) m_pSubConfigMotionFile->LoadConfig();
}


void CDlgSubConfig::OnBnClickedButtonSaveMotion()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (m_pSubConfigMotionFile != nullptr) m_pSubConfigMotionFile->SaveConfig();
}

void CDlgSubConfig::__InitListControl_HeadOffset()
{
	m_gridHeadOffset.GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridHeadOffset.GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridHeadOffset.GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridHeadOffset.SetEditable(FALSE);
	m_gridHeadOffset.SetDefCellMargin(0);
	m_gridHeadOffset.SetFixedColumnSelection(FALSE);
	m_gridHeadOffset.SetFixedRowSelection(FALSE);
	m_gridHeadOffset.SetSingleColSelection(FALSE);
	m_gridHeadOffset.SetSingleRowSelection(FALSE);
	m_gridHeadOffset.SetTrackFocusCell(FALSE);
	m_gridHeadOffset.SetRowResize(FALSE);
	m_gridHeadOffset.SetColumnResize(FALSE);
}


void CDlgSubConfig::__InitialListData_HeadOffset()
{
	//=====================================================================
	//	Grid Control header 부
	//=====================================================================
	m_gridHeadOffset.DeleteAllItems();
	m_gridHeadOffset.SetColumnCount(enumColumnNameHeadOffset_End);
	m_gridHeadOffset.SetFixedRowCount(1);

	for (long n = 0; n < enumColumnNameHeadOffset_End; ++n)
	{
		m_gridHeadOffset.SetColumnWidth(n, ColumnSizeHeadOffset[n]);
		m_gridHeadOffset.SetItemText(0, n, ColumnNameHeadOffset[n]);
		m_gridHeadOffset.SetItemState(0, n, GVIS_READONLY);
		m_gridHeadOffset.SetItemBkColour(0, n, RGB(172, 172, 172));
		m_gridHeadOffset.SetItemFgColour(0, n, RGB(255, 255, 255));
	}

	// Items + Header
	m_gridHeadOffset.SetEditable(TRUE);
	m_gridHeadOffset.SetRowCount(RowItemHeadOffset + 1);

	m_gridHeadOffset.SetRowHeight(0, 28);
	for (long i = 0; i < RowItemHeadOffset; i++)
	{
		m_gridHeadOffset.SetRowHeight(i + 1, 22);
	}

	__RefreshListData_HeadOffset();
}


void CDlgSubConfig::__RefreshListData_HeadOffset()
{
	m_gridHeadOffset.SetRedraw(FALSE);

	stPositionAxis dPos[2][RowItemHeadOffset];
	ZeroMemory(dPos, sizeof(dPos));

	stPositionParam stPos = m_pPositionConfigMgr->GetPositionParam();

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < RowItemHeadOffset; j++)
		{
			dPos[i][j] = stPos.dHeadOffset[i][j];
		}
	}

	for (long i = 0; i < RowItemHeadOffset; i++)
	{
		CString strValue;
		strValue.Format(_T("%s"), RowNameHeadOffset[i]);

		// Name		
		m_gridHeadOffset.GetCell(i + 1, enumColumnNameHeadOffset_No)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridHeadOffset.SetItemState(i + 1, enumColumnNameHeadOffset_No, GVIS_READONLY);
		m_gridHeadOffset.SetItemText(i + 1, enumColumnNameHeadOffset_No, strValue);

		// Value
		strValue.Format(_T("%.4f"), dPos[0][i].dX);
		m_gridHeadOffset.SetItemText(i + 1, enumColumnNameHeadOffset_Value1, strValue);
		m_gridHeadOffset.SetItemState(i + 1, enumColumnNameHeadOffset_Value1, GVIS_READONLY);

		strValue.Format(_T("%.4f"), dPos[0][i].dY);
		m_gridHeadOffset.SetItemText(i + 1, enumColumnNameHeadOffset_Value2, strValue);
		m_gridHeadOffset.SetItemState(i + 1, enumColumnNameHeadOffset_Value2, GVIS_READONLY);

		strValue.Format(_T("%.4f"), dPos[0][i].dZ);
		m_gridHeadOffset.SetItemText(i + 1, enumColumnNameHeadOffset_Value3, strValue);
		m_gridHeadOffset.SetItemState(i + 1, enumColumnNameHeadOffset_Value3, GVIS_READONLY);

		strValue.Format(_T("%.4f"), dPos[1][i].dX);
		m_gridHeadOffset.SetItemText(i + 1, enumColumnNameHeadOffset_Value4, strValue);
		m_gridHeadOffset.SetItemState(i + 1, enumColumnNameHeadOffset_Value4, GVIS_READONLY);

		strValue.Format(_T("%.4f"), dPos[1][i].dY);
		m_gridHeadOffset.SetItemText(i + 1, enumColumnNameHeadOffset_Value5, strValue);
		m_gridHeadOffset.SetItemState(i + 1, enumColumnNameHeadOffset_Value5, GVIS_READONLY);

		strValue.Format(_T("%.4f"), dPos[1][i].dZ);
		m_gridHeadOffset.SetItemText(i + 1, enumColumnNameHeadOffset_Value6, strValue);
		m_gridHeadOffset.SetItemState(i + 1, enumColumnNameHeadOffset_Value6, GVIS_READONLY);

		for (long n = 0; n < enumColumnNameHeadOffset_End; ++n)
		{
			m_gridHeadOffset.SetItemFgColour(i + 1, n, RGB(256, 0, 192));		// RGB(0, 0, 0)
		}
	}

	m_gridHeadOffset.ExpandColumnsToFit(TRUE);
	m_gridHeadOffset.SetRedraw(TRUE);
}

void CDlgSubConfig::OnBnClickedButtonSetColor()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CWindowEventHandler::GetInstance()->PostMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_COLOR, NULL, NULL);
}
