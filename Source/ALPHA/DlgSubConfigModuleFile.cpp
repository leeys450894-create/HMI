// DlgSubConfigModuleFile.cpp : 구현 파일입니다.
//

#include "pch.h"
#include "ALPHA.h"
#include "DlgSubConfigModuleFile.h"
#include "afxdialogex.h"

#include "AuroraMgr.h"
#include "Manager\CommTcpIPMgr.h"
#include "Manager\LightControlMgr.h"
#include "Manager\SequenceManager.h"
#include "Manager\CommTcpIpFactoryMgr.h"
#include "CommEthFunction\CommEthFunction.h"

// CDlgSubConfigModuleFile 대화 상자입니다.

static CString ColumnName[] =
{
	_T("No."),
	_T("Name"),
	_T("IP"),
	_T("Port"),
	_T("SKIP"),
	_T("Use"),
	_T("Connect"),
	_T("Disconnect"),
};
static long ColumnSize[] =
{
	65,
	150,
	150,
	100,
	100,
	100,
	100,
	100,
};

enum enumColumnName
{
	enumColumnName_No = 0,
	enumColumnName_Value1,
	enumColumnName_Value2,
	enumColumnName_Value3,
	enumColumnName_Value4,
	enumColumnName_Value5,
	enumColumnName_Value6,
	enumColumnName_Value7,
	enumColumnName_End,
};

static CString ColumnName2[] =
{
	_T("No."),
	_T("Name"),
	_T("Port"),
	_T("BAUDRATE"),
	_T("SKIP"),
	_T("Connect"),
	_T("Disconnect"),
};
static long ColumnSize2[] =
{
	65,
	150,
	100,
	150,
	100,
	100,
	100,
};
enum enumColumnName2
{
	enumColumnName2_No = 0,
	enumColumnName2_Value1,
	enumColumnName2_Value2,
	enumColumnName2_Value3,
	enumColumnName2_Value4,
	enumColumnName2_Value5,
	enumColumnName2_Value6,
	enumColumnName2_End,
};

IMPLEMENT_DYNAMIC(CDlgSubConfigModuleFile, CDialogEx)

CDlgSubConfigModuleFile::CDlgSubConfigModuleFile(CAuroraMgr* pAuroraMgr, CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLG_SUB_CONFIG_MODULE_FILE, pParent)
	, m_pAuroraMgr(pAuroraMgr)
{

}

CDlgSubConfigModuleFile::~CDlgSubConfigModuleFile()
{
}

void CDlgSubConfigModuleFile::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_CONFIG_TITLE, m_LabelInfo);
	DDX_Control(pDX, IDC_GRID_LIST_CONFIG_MODULE, m_gridList);
}


BEGIN_MESSAGE_MAP(CDlgSubConfigModuleFile, CDialogEx)
	ON_NOTIFY(NM_CLICK, IDC_GRID_LIST_CONFIG_MODULE, &CDlgSubConfigModuleFile::OnGridItemClickList)
	ON_BN_CLICKED(IDC_BUTTON_CONNECT_LED, &CDlgSubConfigModuleFile::OnBnClickedButtonConnectLed)
	ON_BN_CLICKED(IDC_BUTTON_DISCONNECT_LED, &CDlgSubConfigModuleFile::OnBnClickedButtonDisconnectLed)
END_MESSAGE_MAP()


// CDlgSubConfigModuleFile 메시지 처리기입니다.


BOOL CDlgSubConfigModuleFile::PreTranslateMessage(MSG* pMsg)
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


BOOL CDlgSubConfigModuleFile::OnInitDialog()
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

	//---------------------------------------------------------------------
	// 리스트의 정보를 채워준다.
	//---------------------------------------------------------------------
	__InitListControl();
	__InitialListData();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgSubConfigModuleFile::__InitListControl()
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

void CDlgSubConfigModuleFile::__InitialListData()
{
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

	int nRow = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->GetCount();

	// Items + Header
	m_gridList.SetEditable(TRUE);
	m_gridList.SetRowCount(nRow + 1);	// Axis type end 

	m_gridList.SetRowHeight(0, 22);
	for (long i = 0; i < nRow; i++)
	{
		m_gridList.SetRowHeight(i + 1, 22);
	}

	__RefreshListData();
}

void CDlgSubConfigModuleFile::__RefreshListData()
{
	m_gridList.SetRedraw(FALSE);

	int i = 0;
	int nRow = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->GetCount();
	for (int n = 0; n < nRow; n++)
	{
		CString strNo, strData;
		strNo.Format(_T("[-%d-]"), i + 1);

		m_gridList.GetCell(i + 1, enumColumnName_No)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridList.SetItemState(i + 1, enumColumnName_No, GVIS_READONLY);
		m_gridList.SetItemText(i + 1, enumColumnName_No, strNo);

		//CCommEthFunction* pCommTcpIpObject;
		//pCommTcpIpObject = m_pAuroraMgr->GetCommTcpIPMgrPtr()->GetCommTcpIP(i);

		//if (pCommTcpIpObject != nullptr)
		//{
		//	// Name 
		//	strData.Format(_T("%s"), pCommTcpIpObject->GetName());
		//	m_gridList.GetCell(i + 1, enumColumnName_Value1)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		//	m_gridList.SetItemState(i + 1, enumColumnName_Value1, GVIS_READONLY);
		//	m_gridList.SetItemText(i + 1, enumColumnName_Value1, strData);

		//	// IP
		//	strData.Format(_T("%s"), pCommTcpIpObject->GetIPNumber());
		//	m_gridList.SetItemState(i + 1, enumColumnName_Value2, GVIS_READONLY);
		//	m_gridList.SetItemText(i + 1, enumColumnName_Value2, strData);

		//	// PORT
		//	strData.Format(_T("%d"), pCommTcpIpObject->GetPortNumber());
		//	m_gridList.SetItemState(i + 1, enumColumnName_Value3, GVIS_READONLY);
		//	m_gridList.SetItemText(i + 1, enumColumnName_Value3, strData);
		//}
		//else
		{
			// Name 
			strData.Format(_T("%s"), m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->GetParam_Name(i));
			m_gridList.GetCell(i + 1, enumColumnName_Value1)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_gridList.SetItemState(i + 1, enumColumnName_Value1, GVIS_READONLY);
			m_gridList.SetItemText(i + 1, enumColumnName_Value1, strData);

			// IP
			strData.Format(_T("%s"), m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->GetParam_IPNumber(i));
			m_gridList.SetItemState(i + 1, enumColumnName_Value2, GVIS_READONLY);
			m_gridList.SetItemText(i + 1, enumColumnName_Value2, strData);

			// PORT
			strData.Format(_T("%d"), m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->GetParam_PortNumger(i));
			m_gridList.SetItemState(i + 1, enumColumnName_Value3, GVIS_READONLY);
			m_gridList.SetItemText(i + 1, enumColumnName_Value3, strData);
		}

		// SKIP
		//CGridCellCheck* pCell;
		//m_gridList.SetCellType(i + 1, enumColumnName_Value4, RUNTIME_CLASS(CGridCellCheck));
		//pCell = (CGridCellCheck*)m_gridList.GetCell(i + 1, enumColumnName_Value4);

		//if (false)
		//	pCell->SetCheck(true);   // 1
		//else
		//	pCell->SetCheck(false);	 // -1 

		// button - SKIP
		m_gridList.SetCellType(i + 1, enumColumnName_Value4, RUNTIME_CLASS(CGridCellButton));
		m_gridList.SetItemState(i + 1, enumColumnName_Value4, GVIS_READONLY);
		m_gridList.SetItemText(i + 1, enumColumnName_Value4, _T("SKIP"));

		// button - USE
		m_gridList.SetCellType(i + 1, enumColumnName_Value5, RUNTIME_CLASS(CGridCellButton));
		m_gridList.SetItemState(i + 1, enumColumnName_Value5, GVIS_READONLY);
		m_gridList.SetItemText(i + 1, enumColumnName_Value5, _T("USE"));

		// button - Connect
		m_gridList.SetCellType(i + 1, enumColumnName_Value6, RUNTIME_CLASS(CGridCellButton));
		m_gridList.SetItemState(i + 1, enumColumnName_Value6, GVIS_READONLY);
		m_gridList.SetItemText(i + 1, enumColumnName_Value6, _T("CONNECT"));

		// button - Disconnect
		m_gridList.SetCellType(i + 1, enumColumnName_Value7, RUNTIME_CLASS(CGridCellButton));
		m_gridList.SetItemState(i + 1, enumColumnName_Value7, GVIS_READONLY);
		m_gridList.SetItemText(i + 1, enumColumnName_Value7, _T("DISCONNECT"));

		for (long n = 0; n < enumColumnName_End; ++n)
		{
			m_gridList.SetItemFgColour(i + 1, n, RGB(0, 0, 0));
		}

		i++;
	}

//	m_gridList.ExpandToFit(TRUE);
	m_gridList.ExpandColumnsToFit(TRUE);
	m_gridList.SetRedraw(TRUE);
}

void CDlgSubConfigModuleFile::__InitListControl2()
{
	//m_gridList2.GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	//m_gridList2.GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	//m_gridList2.GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	//m_gridList2.SetDefCellMargin(0);
	//m_gridList2.SetFixedColumnSelection(FALSE);
	//m_gridList2.SetFixedRowSelection(FALSE);
	//m_gridList2.SetSingleColSelection(FALSE);
	//m_gridList2.SetSingleRowSelection(FALSE);
	//m_gridList2.SetTrackFocusCell(FALSE);
	//m_gridList2.SetRowResize(FALSE);
	//m_gridList2.SetColumnResize(FALSE);
}

void CDlgSubConfigModuleFile::__InitialListData2()
{
	//// 결과값 리스트를 갱신한다.
	//m_gridList2.DeleteAllItems();
	//m_gridList2.SetColumnCount(enumColumnName2_End);
	//m_gridList2.SetFixedRowCount(1);

	//for (long n = 0; n < enumColumnName2_End; ++n)
	//{
	//	m_gridList2.SetColumnWidth(n, ColumnSize2[n]);
	//	m_gridList2.SetItemText(0, n, ColumnName2[n]);
	//	m_gridList2.SetItemState(0, n, GVIS_READONLY);
	//	m_gridList2.SetItemBkColour(0, n, RGB(172, 172, 172));
	//	m_gridList2.SetItemFgColour(0, n, RGB(255, 255, 255));
	//}

	//int nRow = 0;
	//
	//if (m_pAuroraMgr->GetLightControlMgrPtr() != nullptr)
	//	nRow++;

	//// Items + Header
	//m_gridList2.SetEditable(TRUE);
	//m_gridList2.SetRowCount(nRow + 1);	// Axis type end 

	//m_gridList2.SetRowHeight(0, 22);
	//for (long i = 0; i < nRow; i++)
	//{
	//	m_gridList2.SetRowHeight(i + 1, 22);
	//}

	//__RefreshListData2();
}

void CDlgSubConfigModuleFile::__RefreshListData2()
{
	//m_gridList2.SetRedraw(FALSE);

	//int i = 0, nRow = 0;
	//if (m_pAuroraMgr->GetLightControlMgrPtr() != nullptr)
	//	nRow++;

	//for (int n = 0; n < nRow; n++)
	//{
	//	CString strNo, strData;
	//	strNo.Format(_T("[-%d-]"), i + 1);

	//	m_gridList2.GetCell(i + 1, enumColumnName2_No)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	//	m_gridList2.SetItemState(i + 1, enumColumnName2_No, GVIS_READONLY);
	//	m_gridList2.SetItemText(i + 1, enumColumnName2_No, strNo);

	//	m_pAuroraMgr->GetLightControlMgrPtr()

	//	if (pCommTcpIpObject != nullptr)
	//	{
	//		// Name 
	//		strData.Format(_T("%s"), pCommTcpIpObject->GetName());
	//		m_gridList.GetCell(i + 1, enumColumnName_Value1)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	//		m_gridList.SetItemState(i + 1, enumColumnName_Value1, GVIS_READONLY);
	//		m_gridList.SetItemText(i + 1, enumColumnName_Value1, strData);

	//		// IP
	//		strData.Format(_T("%s"), pCommTcpIpObject->GetIPNumber());
	//		m_gridList.SetItemText(i + 1, enumColumnName_Value2, strData);

	//		// PORT
	//		strData.Format(_T("%d"), pCommTcpIpObject->GetPortNumber());
	//		m_gridList.SetItemText(i + 1, enumColumnName_Value3, strData);
	//	}
	//	else
	//	{
	//		// Name 
	//		strData.Format(_T("%s"), _T(""));
	//		m_gridList.GetCell(i + 1, enumColumnName_Value1)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	//		m_gridList.SetItemState(i + 1, enumColumnName_Value1, GVIS_READONLY);
	//		m_gridList.SetItemText(i + 1, enumColumnName_Value1, strData);

	//		// IP
	//		strData.Format(_T("%s"), _T(""));
	//		m_gridList.SetItemText(i + 1, enumColumnName_Value2, strData);

	//		// PORT
	//		strData.Format(_T("%d"), _T(""));
	//		m_gridList.SetItemText(i + 1, enumColumnName_Value3, strData);
	//	}

	//	// SKIP
	//	CGridCellCheck* pCell;
	//	m_gridList.SetCellType(i + 1, enumColumnName_Value4, RUNTIME_CLASS(CGridCellCheck));
	//	pCell = (CGridCellCheck*)m_gridList.GetCell(i + 1, enumColumnName_Value4);

	//	if (false)
	//		pCell->SetCheck(true);   // 1
	//	else
	//		pCell->SetCheck(false);	 // -1 

	//								 // button - Connect
	//	m_gridList.SetCellType(i + 1, enumColumnName_Value5, RUNTIME_CLASS(CGridCellButton));
	//	m_gridList.SetItemState(i + 1, enumColumnName_Value5, GVIS_READONLY);
	//	m_gridList.SetItemText(i + 1, enumColumnName_Value5, _T("RUN"));

	//	// button - Disconnect
	//	m_gridList.SetCellType(i + 1, enumColumnName_Value6, RUNTIME_CLASS(CGridCellButton));
	//	m_gridList.SetItemState(i + 1, enumColumnName_Value6, GVIS_READONLY);
	//	m_gridList.SetItemText(i + 1, enumColumnName_Value6, _T("RUN"));


	//	for (long n = 0; n < enumColumnName_End; ++n)
	//	{
	//		m_gridList.SetItemFgColour(i + 1, n, RGB(0, 0, 0));
	//	}

	//	i++;
	//}

	//m_gridList2.ExpandColumnsToFit(TRUE);
	//m_gridList2.SetRedraw(TRUE);
}


void CDlgSubConfigModuleFile::OnGridItemClickList(NMHDR* pNotifyStruct, LRESULT* result)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*)pNotifyStruct;

	const long row = pNotify->iRow;
	const long col = pNotify->iColumn;

	if ((0 < row) && (enumColumnName_Value4 == col))	// SKIP
	{
		CCommTcpIpFactoryMgr* pCommTcpIpObject;
		pCommTcpIpObject = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr();

	/*	bool bOn = false;
		CGridCellCheck* pCell;
		pCell = (CGridCellCheck*)m_gridList.GetCell(row, enumColumnName_Value4);

		if( pCell->GetCheck() == TRUE)
			bOn = true;*/

		pCommTcpIpObject->SetOutOfService(row - 1, true);

		/*if(row == 1)
			pCommTcpIpObject->SetOutOfService(1, true);
		else if (row == 2)
			pCommTcpIpObject->SetOutOfService(2, true);*/
	}

	if ((0 < row) && (enumColumnName_Value5 == col))	// USE
	{
		CCommTcpIpFactoryMgr* pCommTcpIpObject;
		pCommTcpIpObject = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr();

		pCommTcpIpObject->SetOutOfService(row - 1, false);

		/*if (row == 1)
			pCommTcpIpObject->SetOutOfService(1, false);
		else if (row == 2)
			pCommTcpIpObject->SetOutOfService(2, false);*/
	}

	if ((0 < row) && (enumColumnName_Value6 == col))	// CONNECT
	{
		CCommTcpIpFactoryMgr* pCommTcpIpObject;
		pCommTcpIpObject = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr();

		pCommTcpIpObject->SetTcpConnection(row - 1, true);
	/*	if (row == 1)
			pCommTcpIpObject->SetTcpConnection(1, true);
		else if (row == 2)
			pCommTcpIpObject->SetTcpConnection(2, true);*/
	}

	if ((0 < row) && (enumColumnName_Value7 == col))	// DISCONNECT
	{
		CCommTcpIpFactoryMgr* pCommTcpIpObject;
		pCommTcpIpObject = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr();

		pCommTcpIpObject->SetTcpConnection(row - 1, false);
		/*if (row == 1)
			pCommTcpIpObject->SetTcpConnection(1, false);
		else if (row == 2)
			pCommTcpIpObject->SetTcpConnection(2, false);*/
	}

	*result = 0;
}

void CDlgSubConfigModuleFile::OnBnClickedButtonConnectLed()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CDlgSubConfigModuleFile::OnBnClickedButtonDisconnectLed()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}
