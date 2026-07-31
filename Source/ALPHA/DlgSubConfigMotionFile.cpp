// DlgSubConfigMotionFile.cpp : 구현 파일입니다.
//

#include "pch.h"
#include "ALPHA.h"
#include "DlgSubConfigMotionFile.h"
#include "afxdialogex.h"

#include "Manager\SystemConfigMgr.h"
#include "Manager\MotorControlMgr.h"

static CString ColumnName[] =
{
	_T("No"),
	_T("Name"),
	_T("+Dir"),
	_T("Type"),
	_T("Buffer"),
	_T("VH"),
	_T("VM"),
	_T("VL"),
	_T("VO"),
	_T("AH"),
	_T("AM"),
	_T("AL"),
	_T("STH"),
	_T("STM"),
	_T("STL"),
	_T("MAX"),
	_T("MIN"),
	_T("OFFSET"),
	_T("HOME"),
	_T("AXIS"),
};

static long ColumnSize[] =
{
	65,
	160,
	65,
	0, // 90 to 0
	65,
	65,
	65,
	65,
	65,
	65,
	65,
	65,
	65,
	65,
	65,
	80,
	80,
	80,
	80,
	65,
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
	enumColumnName_Value8,
	enumColumnName_Value9,
	enumColumnName_Value10,
	enumColumnName_Value11,
	enumColumnName_Value12,
	enumColumnName_Value13,
	enumColumnName_Value14,
	enumColumnName_Value15,
	enumColumnName_Value16,
	enumColumnName_Value17,
	enumColumnName_Value18,
	enumColumnName_Value19,
	enumColumnName_End,
};
// CDlgSubConfigMotionFile 대화 상자입니다.

IMPLEMENT_DYNAMIC(CDlgSubConfigMotionFile, CDialogEx)

CDlgSubConfigMotionFile::CDlgSubConfigMotionFile(CMotorControlMgr *pMotorControlMgr, CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DLG_SUB_CONFIG_MOTION_FILE, pParent)
	, m_pMotorControlMgr(pMotorControlMgr)
	, m_iCurSel(-1)
{

}

CDlgSubConfigMotionFile::~CDlgSubConfigMotionFile()
{
}

void CDlgSubConfigMotionFile::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_CONFIG_TITLE, m_LabelInfo);
	DDX_Control(pDX, IDC_GRID_LIST_CONFIG_MOTION, m_gridList);
}


BEGIN_MESSAGE_MAP(CDlgSubConfigMotionFile, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_LOAD, &CDlgSubConfigMotionFile::OnBnClickedButtonLoad)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CDlgSubConfigMotionFile::OnBnClickedButtonSave)
	ON_NOTIFY(GVN_SELCHANGED, IDC_GRID_LIST_CONFIG_MOTION, &CDlgSubConfigMotionFile::OnGridItemActivateList)
	ON_NOTIFY(GVN_ENDLABELEDIT, IDC_GRID_LIST_CONFIG_MOTION, &CDlgSubConfigMotionFile::OnGridItemChangedList)
END_MESSAGE_MAP()


// CDlgSubConfigMotionFile 메시지 처리기입니다.


BOOL CDlgSubConfigMotionFile::PreTranslateMessage(MSG* pMsg)
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


BOOL CDlgSubConfigMotionFile::OnInitDialog()
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

	m_pMotorControlMgr->LinkDatFile();

	__InitListControl();
	__InitialListData();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgSubConfigMotionFile::__InitListControl()
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

void CDlgSubConfigMotionFile::__InitialListData()
{
	// 결과값 리스트를 갱신한다.
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
	m_gridList.SetRowCount(AXIS_TYPE_END + 1);	// Axis type end 
	m_gridList.SetRowHeight(0, 22);
	
	for (long i = 0; i < AXIS_TYPE_END; i++)
	{
		m_gridList.SetRowHeight(i + 1, 0);
	}

	__RefreshListData();
}

void CDlgSubConfigMotionFile::__RefreshListData()
{
	m_gridList.SetRedraw(FALSE);

	int i = 0;
	for (int iAxis = 0; iAxis < AXIS_TYPE_END; iAxis++)
	{
		if (m_pMotorControlMgr->GetAxisProperty(iAxis)->bUse != TRUE)
			continue;

		if (iAxis == AXIS_PROBE_X2 || iAxis == AXIS_PROBE_X4 || iAxis == AXIS_PROBE_X6 || iAxis == AXIS_PROBE_X8 || iAxis == AXIS_PROBE_X10)
		{
			m_gridList.SetRowHeight(i + 1, 0);
		}
		else
		{
			m_gridList.SetRowHeight(i + 1, 14);
		}

		CString strNo, strData;
		strNo.Format(_T("[-%d-]"), i + 1);

		m_gridList.GetCell(i + 1, enumColumnName_No)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridList.SetItemState(i + 1, enumColumnName_No, GVIS_READONLY);
		m_gridList.SetItemText(i + 1, enumColumnName_No, strNo);


		// Name 
		strData.Format(_T("%s"), m_pMotorControlMgr->GetAxisMapProperty(iAxis)->m_strAxisName);
		m_gridList.GetCell(i + 1, enumColumnName_Value1)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridList.SetItemState(i + 1, enumColumnName_Value1, GVIS_READONLY);
		m_gridList.SetItemText(i + 1, enumColumnName_Value1, strData);

		// +Dir
		CGridCellCheck* pCell;
		m_gridList.SetCellType(i + 1, enumColumnName_Value2, RUNTIME_CLASS(CGridCellCheck));
		pCell = (CGridCellCheck*)m_gridList.GetCell(i + 1, enumColumnName_Value2);

		if (m_pMotorControlMgr->GetAxisMapProperty(iAxis)->m_iDir == 1)
			pCell->SetCheck(true);   // 1
		else
			pCell->SetCheck(false);	 // -1 		


									 // Type 
		CStringArray arrTypeName;
		arrTypeName.Add(_T("LINEAR"));
		arrTypeName.Add(_T("SERVO"));
		arrTypeName.Add(_T("STEPPING"));
		m_gridList.SetCellType(i + 1, enumColumnName_Value3, RUNTIME_CLASS(CGridCellCombo));
		((CGridCellCombo *)m_gridList.GetCell(i + 1, enumColumnName_Value3))->SetOptions(arrTypeName);
		((CGridCellCombo *)m_gridList.GetCell(i + 1, enumColumnName_Value3))->SetStyle(CBS_DROPDOWNLIST); //CBS_DROPDOWN, CBS_DROPDOWNLIST, CBS_SIMPLE

		if (m_pMotorControlMgr->GetAxisProperty(iAxis)->iStepper == 0)
			strData = _T("LINEAR");
		else if (m_pMotorControlMgr->GetAxisProperty(iAxis)->iStepper == 1)
			strData = _T("SERVO");
		else
			strData = _T("STEPPING");
		m_gridList.SetItemText(i + 1, enumColumnName_Value3, strData);
		arrTypeName.RemoveAll();


		// Buffer
		for (int iHomePLC = 0; iHomePLC < AXIS_TYPE_END; iHomePLC++)
		{
			strData.Format(_T("%d"), iHomePLC);
			arrTypeName.Add(strData);
		}

		m_gridList.SetCellType(i + 1, enumColumnName_Value4, RUNTIME_CLASS(CGridCellCombo));
		((CGridCellCombo *)m_gridList.GetCell(i + 1, enumColumnName_Value4))->SetOptions(arrTypeName);
		((CGridCellCombo *)m_gridList.GetCell(i + 1, enumColumnName_Value4))->SetStyle(CBS_DROPDOWNLIST); //CBS_DROPDOWN, CBS_DROPDOWNLIST, CBS_SIMPLE
		strData.Format(_T("%d"), m_pMotorControlMgr->GetAxisProperty(iAxis)->iNumHomePLC);
		m_gridList.SetItemText(i + 1, enumColumnName_Value4, strData);
		arrTypeName.RemoveAll();


		// VH
		strData.Format(_T("%.1f"), m_pMotorControlMgr->GetAxisProperty(iAxis)->dVelH);
		m_gridList.GetCell(i + 1, enumColumnName_Value5)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridList.SetItemState(i + 1, enumColumnName_Value5, GVIS_MODIFIED);
		m_gridList.SetItemText(i + 1, enumColumnName_Value5, strData);

		// VM
		strData.Format(_T("%.1f"), m_pMotorControlMgr->GetAxisProperty(iAxis)->dVelM);
		m_gridList.GetCell(i + 1, enumColumnName_Value6)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridList.SetItemState(i + 1, enumColumnName_Value6, GVIS_MODIFIED);
		m_gridList.SetItemText(i + 1, enumColumnName_Value6, strData);


		// VL
		strData.Format(_T("%.1f"), m_pMotorControlMgr->GetAxisProperty(iAxis)->dVelL);
		m_gridList.GetCell(i + 1, enumColumnName_Value7)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridList.SetItemState(i + 1, enumColumnName_Value7, GVIS_MODIFIED);
		m_gridList.SetItemText(i + 1, enumColumnName_Value7, strData);

		// VO
		strData.Format(_T("%.1f"), m_pMotorControlMgr->GetAxisProperty(iAxis)->dVelHome);
		m_gridList.GetCell(i + 1, enumColumnName_Value8)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridList.SetItemState(i + 1, enumColumnName_Value8, GVIS_MODIFIED);
		m_gridList.SetItemText(i + 1, enumColumnName_Value8, strData);

		// AH
		strData.Format(_T("%.1f"), m_pMotorControlMgr->GetAxisProperty(iAxis)->dAccH);
		m_gridList.GetCell(i + 1, enumColumnName_Value9)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridList.SetItemState(i + 1, enumColumnName_Value9, GVIS_MODIFIED);
		m_gridList.SetItemText(i + 1, enumColumnName_Value9, strData);

		// AM
		strData.Format(_T("%.1f"), m_pMotorControlMgr->GetAxisProperty(iAxis)->dAccM);
		m_gridList.GetCell(i + 1, enumColumnName_Value10)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridList.SetItemState(i + 1, enumColumnName_Value10, GVIS_MODIFIED);
		m_gridList.SetItemText(i + 1, enumColumnName_Value10, strData);


		// AL
		strData.Format(_T("%.1f"), m_pMotorControlMgr->GetAxisProperty(iAxis)->dAccL);
		m_gridList.GetCell(i + 1, enumColumnName_Value11)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridList.SetItemState(i + 1, enumColumnName_Value11, GVIS_MODIFIED);
		m_gridList.SetItemText(i + 1, enumColumnName_Value11, strData);


		// STH
		strData.Format(_T("%.1f"), m_pMotorControlMgr->GetAxisProperty(iAxis)->dScvH);
		m_gridList.GetCell(i + 1, enumColumnName_Value12)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridList.SetItemState(i + 1, enumColumnName_Value12, GVIS_MODIFIED);
		m_gridList.SetItemText(i + 1, enumColumnName_Value12, strData);

		// STM
		strData.Format(_T("%.1f"), m_pMotorControlMgr->GetAxisProperty(iAxis)->dScvM);
		m_gridList.GetCell(i + 1, enumColumnName_Value13)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridList.SetItemState(i + 1, enumColumnName_Value13, GVIS_MODIFIED);
		m_gridList.SetItemText(i + 1, enumColumnName_Value13, strData);


		// STL
		strData.Format(_T("%.1f"), m_pMotorControlMgr->GetAxisProperty(iAxis)->dScvL);
		m_gridList.GetCell(i + 1, enumColumnName_Value14)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridList.SetItemState(i + 1, enumColumnName_Value14, GVIS_MODIFIED);
		m_gridList.SetItemText(i + 1, enumColumnName_Value14, strData);

		// MAX
		strData.Format(_T("%.4f"), m_pMotorControlMgr->GetAxisProperty(iAxis)->dDimMax);
		m_gridList.GetCell(i + 1, enumColumnName_Value15)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridList.SetItemState(i + 1, enumColumnName_Value15, GVIS_MODIFIED);
		m_gridList.SetItemText(i + 1, enumColumnName_Value15, strData);

		// MIN
		strData.Format(_T("%.4f"), m_pMotorControlMgr->GetAxisProperty(iAxis)->dDimMin);
		m_gridList.GetCell(i + 1, enumColumnName_Value16)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridList.SetItemState(i + 1, enumColumnName_Value16, GVIS_MODIFIED);
		m_gridList.SetItemText(i + 1, enumColumnName_Value16, strData);

		// OFFSET
		strData.Format(_T("%.4f"), m_pMotorControlMgr->GetAxisProperty(iAxis)->dOffset);
		m_gridList.GetCell(i + 1, enumColumnName_Value17)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridList.SetItemState(i + 1, enumColumnName_Value17, GVIS_MODIFIED);
		m_gridList.SetItemText(i + 1, enumColumnName_Value17, strData);

		// Home
		strData.Format(_T("%.4f"), m_pMotorControlMgr->GetAxisProperty(iAxis)->dHomePos);
		m_gridList.GetCell(i + 1, enumColumnName_Value18)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridList.SetItemState(i + 1, enumColumnName_Value18, GVIS_MODIFIED);
		m_gridList.SetItemText(i + 1, enumColumnName_Value18, strData);

		// AXIS 
		strData.Format(_T("%d"), iAxis);
		m_gridList.GetCell(i + 1, enumColumnName_Value19)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridList.SetItemState(i + 1, enumColumnName_Value19, GVIS_READONLY);
		m_gridList.SetItemText(i + 1, enumColumnName_Value19, strData);


		for (long n = 0; n < enumColumnName_End; ++n)
		{
			m_gridList.SetItemFgColour(i + 1, n, RGB(0, 0, 0));
		}

		i++;
	}

//	m_gridList.ExpandToFit(TRUE);
	m_gridList.ExpandColumnsToFit(TRUE);
	m_gridList.SetRedraw(TRUE, TRUE);
}


void CDlgSubConfigMotionFile::OnBnClickedButtonLoad()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	LoadConfig();
}


void CDlgSubConfigMotionFile::OnBnClickedButtonSave()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	SaveConfig();
}

void CDlgSubConfigMotionFile::OnGridItemActivateList(NMHDR * pNotifyStruct, LRESULT * result)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	NM_GRIDVIEW *pNotify = (NM_GRIDVIEW *)pNotifyStruct;

	const long row = pNotify->iRow;
	const long col = pNotify->iColumn;

	if (1 <= row)
	{
		m_iCurSel = row - 1;

//		__RefreshListData();
	}

	*result = 0;
}


void CDlgSubConfigMotionFile::OnGridItemChangedList(NMHDR * pNotifyStruct, LRESULT * result)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	NM_GRIDVIEW *pNotify = (NM_GRIDVIEW *)pNotifyStruct;

	const long row = pNotify->iRow;
	const long col = pNotify->iColumn;

	const CString strValue = m_gridList.GetItemText(row, col);

	switch (col)
	{
	case enumColumnName_Value1:
		;
		break;

	default:
		break;
	}

	*result = 0;
}

void CDlgSubConfigMotionFile::GetCurAxisInfo(int nGurAxis)
{
	CString strVal;
	int iAxis = 0;

	if (nGurAxis == -1)
	{
		for (int i = 0; i < AXIS_TYPE_END; i++)
		{
			strVal = m_gridList.GetItemText(i + 1, enumColumnName_No);
			if( strVal.IsEmpty() == TRUE)
				continue;

			GetCurAxisInfo(i);
		}

		return;
	}

	int i = nGurAxis;

	// AXIS
	strVal = m_gridList.GetItemText(i + 1, enumColumnName_Value19);
	iAxis = _ttoi(strVal);

	// VH
	strVal = m_gridList.GetItemText(i + 1, enumColumnName_Value5);
	m_pMotorControlMgr->GetAxisProperty(iAxis)->dVelH = _ttof(strVal);
	

	// VM
	strVal = m_gridList.GetItemText(i + 1, enumColumnName_Value6);
	m_pMotorControlMgr->GetAxisProperty(iAxis)->dVelM = _ttof(strVal);


	// VL
	strVal = m_gridList.GetItemText(i + 1, enumColumnName_Value7);
	m_pMotorControlMgr->GetAxisProperty(iAxis)->dVelL = _ttof(strVal);


	// VO
	strVal = m_gridList.GetItemText(i + 1, enumColumnName_Value8);
	m_pMotorControlMgr->GetAxisProperty(iAxis)->dVelHome = _ttof(strVal);


	// AH
	strVal = m_gridList.GetItemText(i + 1, enumColumnName_Value9);
	m_pMotorControlMgr->GetAxisProperty(iAxis)->dAccH = _ttof(strVal);

	// AM
	strVal = m_gridList.GetItemText(i + 1, enumColumnName_Value10);
	m_pMotorControlMgr->GetAxisProperty(iAxis)->dAccM = _ttof(strVal);

	// AL
	strVal = m_gridList.GetItemText(i + 1, enumColumnName_Value11);
	m_pMotorControlMgr->GetAxisProperty(iAxis)->dAccL = _ttof(strVal);

	// STH
	strVal = m_gridList.GetItemText(i + 1, enumColumnName_Value12);
	m_pMotorControlMgr->GetAxisProperty(iAxis)->dScvH = _ttof(strVal);

	// STM
	strVal = m_gridList.GetItemText(i + 1, enumColumnName_Value13);
	m_pMotorControlMgr->GetAxisProperty(iAxis)->dScvM = _ttof(strVal);

	// STL
	strVal = m_gridList.GetItemText(i + 1, enumColumnName_Value14);
	m_pMotorControlMgr->GetAxisProperty(iAxis)->dScvL = _ttof(strVal);

	// MAX
	strVal = m_gridList.GetItemText(i + 1, enumColumnName_Value15);
	m_pMotorControlMgr->GetAxisProperty(iAxis)->dDimMax = _ttof(strVal);

	// MIN
	strVal = m_gridList.GetItemText(i + 1, enumColumnName_Value16);
	m_pMotorControlMgr->GetAxisProperty(iAxis)->dDimMin = _ttof(strVal);

	// OFFSET
	strVal = m_gridList.GetItemText(i + 1, enumColumnName_Value17);
	m_pMotorControlMgr->GetAxisProperty(iAxis)->dOffset = _ttof(strVal);

	// Home
	strVal = m_gridList.GetItemText(i + 1, enumColumnName_Value18);
	m_pMotorControlMgr->GetAxisProperty(iAxis)->dHomePos = _ttof(strVal);
}


void CDlgSubConfigMotionFile::LoadConfig()
{
	// TODO: 여기에 구현 코드 추가.
	m_pMotorControlMgr->LinkDatFile();

	__RefreshListData();
}


void CDlgSubConfigMotionFile::SaveConfig()
{
	// TODO: 여기에 구현 코드 추가.	// Get Modify
	GetCurAxisInfo(-1);

	// save File
	m_pMotorControlMgr->LinkDatFile(true);

	// Reload
	m_pMotorControlMgr->LinkDatFile();
}
