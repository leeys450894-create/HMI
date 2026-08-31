// CD_PopUp_Motion_Move.cpp: 구현 파일
//

#include "pch.h"
#include "ALPHA.h"
#include "CD_PopUp_Motion_Move.h"
#include "afxdialogex.h"
#include "Manager/MaintControlMgr.h"
#include "Manager\ShareMemoryMgr.h"

static CString ColumnName[] =
{
	_T("Name."),
	_T("Num"),
	_T("Current"),
	_T("E"),
	_T("L+"),
	_T("L-"),
	_T("REL"),
	_T("<<"),
	_T(">>"),
};
static long ColumnSize[] =
{
	120,
	25,
	70,
	25,
	25,
	25,
	25,
	25,
	25,
};
enum enumColumnName
{
	enumColumnName_Value1,
	enumColumnName_Value2,
	enumColumnName_Value3,
	enumColumnName_Value4,
	enumColumnName_Value5,
	enumColumnName_Value6,
	enumColumnName_Value7,
	enumColumnName_Value8,
	enumColumnName_Value9,
	enumColumnName_End,
};

// CD_PopUp_Motion_Move 대화 상자

IMPLEMENT_DYNAMIC(CD_PopUp_Motion_Move, CDialogEx)

CD_PopUp_Motion_Move::CD_PopUp_Motion_Move(CMaintControlMgr* pMaintControlMgr, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PUPUP_MOTION_MOVE, pParent)
	, m_pMaintControlMgr(pMaintControlMgr)
{
	ZeroMemory(&m_stMoterInfo, sizeof(m_stMoterInfo));

	//----------------------------------------------------------------------
	// Thread 를 미리 생성해 놓고 Event 로 제어 한다.
	//----------------------------------------------------------------------
	m_pThread = AfxBeginThread(ThreadRun, this);
	m_bThreadLive = false;
	m_hStart1Event = CreateEvent(NULL, FALSE, FALSE, NULL);		// auto reset, initial reset
	m_hStart2Event = CreateEvent(NULL, FALSE, FALSE, NULL);		// auto reset, initial reset
	m_hCloseEvent = CreateEvent(NULL, FALSE, FALSE, NULL);		// auto reset, initial reset
	m_hMultiEvent[0] = m_hStart1Event;
	m_hMultiEvent[1] = m_hStart2Event;
	m_hMultiEvent[2] = m_hCloseEvent;
}

CD_PopUp_Motion_Move::~CD_PopUp_Motion_Move()
{
	//----------------------------------------------------------------------
	// Thread 종료
	//----------------------------------------------------------------------
	ResetEvent(m_hStart1Event);
	ResetEvent(m_hStart2Event);
	SetEvent(m_hCloseEvent);

	if (m_pThread && m_bThreadLive)
	{
		WaitForSingleObject(m_pThread->m_hThread, INFINITE);
	}
	if (m_hStart1Event)CloseHandle(m_hStart1Event);
	if (m_hStart2Event)CloseHandle(m_hStart2Event);
	if (m_hCloseEvent) CloseHandle(m_hCloseEvent);
}

void CD_PopUp_Motion_Move::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_MAINT_TITLE, m_LabelInfo[0]);
	DDX_Control(pDX, IDC_STATIC_MAINT_TITLE2, m_LabelInfo[1]);
	DDX_Control(pDX, IDC_STATIC_MAINT_TITLE3, m_LabelInfo[2]);
	DDX_Control(pDX, IDC_STATIC_MAINT_TITLE4, m_LabelInfo[3]);
	DDX_Control(pDX, IDC_GRID_LIST_MAINT_MOTION, m_gridList[0]);
	DDX_Control(pDX, IDC_GRID_LIST_MAINT_MOTION2, m_gridList[1]);
	DDX_Control(pDX, IDC_GRID_LIST_MAINT_MOTION3, m_gridList[2]);
	DDX_Control(pDX, IDC_GRID_LIST_MAINT_MOTION4, m_gridList[3]);
}


BEGIN_MESSAGE_MAP(CD_PopUp_Motion_Move, CDialogEx)
	ON_NOTIFY(NM_CLICK, IDC_GRID_LIST_MAINT_MOTION, &CD_PopUp_Motion_Move::OnGridItemClickList1)
	ON_NOTIFY(NM_CLICK, IDC_GRID_LIST_MAINT_MOTION2, &CD_PopUp_Motion_Move::OnGridItemClickList2)
	ON_NOTIFY(NM_CLICK, IDC_GRID_LIST_MAINT_MOTION3, &CD_PopUp_Motion_Move::OnGridItemClickList3)
	ON_NOTIFY(NM_CLICK, IDC_GRID_LIST_MAINT_MOTION4, &CD_PopUp_Motion_Move::OnGridItemClickList4)
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CD_PopUp_Motion_Move 메시지 처리기


BOOL CD_PopUp_Motion_Move::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	SetBackgroundColor(RGB(128, 128, 128));

	//---------------------------------------------------------------------
	// 타이틀에 색상을 입혀준다.
	//---------------------------------------------------------------------
	for (size_t i = 0; i < 4; i++)
	{
		m_LabelInfo[i].SetBorder(TRUE, TRUE);
		m_LabelInfo[i].SetBkColor(RGB(0, 32, 0));
		m_LabelInfo[i].SetTextColor(RGB(255, 255, 192));
		m_LabelInfo[i].SetFontBold(TRUE);
	}

	//---------------------------------------------------------------------
	// 리스트의 정보를 채워준다.
	//---------------------------------------------------------------------
	__InitListControl();
	__InitialListData();

	__InitListControl2();
	__InitialListData2();

	__InitListControl3();
	__InitialListData3();

	__InitListControl4();
	__InitialListData4();

	SetTimer(1000, 1000, NULL);
	
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CD_PopUp_Motion_Move::__RefreshListData()
{
	__RefreshListData1();
	__RefreshListData2();
	__RefreshListData3();
	__RefreshListData4();
}

void CD_PopUp_Motion_Move::__InitListControl()
{
	m_gridList[0].GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridList[0].GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridList[0].GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridList[0].SetDefCellMargin(0);
	m_gridList[0].SetFixedColumnSelection(FALSE);
	m_gridList[0].SetFixedRowSelection(FALSE);
	m_gridList[0].SetSingleColSelection(FALSE);
	m_gridList[0].SetSingleRowSelection(FALSE);
	m_gridList[0].SetTrackFocusCell(FALSE);
	m_gridList[0].SetRowResize(FALSE);
	m_gridList[0].SetColumnResize(FALSE);
}

void CD_PopUp_Motion_Move::__InitialListData()
{
	// 결과값 리스트를 갱신한다.
	m_gridList[0].DeleteAllItems();
	m_gridList[0].SetColumnCount(enumColumnName_End);
	m_gridList[0].SetFixedRowCount(1);

	for (long n = 0; n < enumColumnName_End; ++n)
	{
		m_gridList[0].SetColumnWidth(n, ColumnSize[n]);
		m_gridList[0].SetItemText(0, n, ColumnName[n]);
		m_gridList[0].SetItemState(0, n, GVIS_READONLY);
		m_gridList[0].SetItemBkColour(0, n, RGB(172, 172, 172));
		m_gridList[0].SetItemFgColour(0, n, RGB(255, 255, 255));
	}

	int nAxis = 0;

	CMotorControlMgr* pImpl = nullptr;
	pImpl = m_pMaintControlMgr->GetMotorControlMgrPtr();
	for (int iAxis = 0; iAxis < AXIS_TYPE_END; iAxis++)
	{
		if (iAxis == AXIS_X || iAxis == AXIS_LOADER_R || iAxis == AXIS_LOADER_Z || iAxis == AXIS_X2 || iAxis == AXIS_X3)
		{
			if (pImpl->GetAxisProperty(iAxis)->bUse == TRUE)
			{
				nAxis++;
			}
		}
	}

	// Items + Header
	m_gridList[0].SetEditable(TRUE);
	m_gridList[0].SetRowCount(nAxis + 1);	// Axis type end 
	m_gridList[0].SetRowHeight(0, 22);

	for (long i = 0; i < nAxis; i++)
	{
		m_gridList[0].SetRowHeight(i + 1, 18);
	}

	__RefreshListData1();
}

void CD_PopUp_Motion_Move::__RefreshListData1()
{
	m_gridList[0].SetRedraw(FALSE);

	int i = 0;
	CString strData;
	CMotorControlMgr* pImpl = nullptr;
	pImpl = m_pMaintControlMgr->GetMotorControlMgrPtr();

	VTM_SYSTEM* pShare = nullptr;
	pShare = m_pMaintControlMgr->GetShareMemoryMgrPtr()->GetSharedMemory();

	/*_T("Name."),
		_T("Num"),
		_T("Current"),
		_T("E"),
		_T("L+"),
		_T("L-"),
		_T("REL"),
		_T("<<"),
		_T(">>"),*/

	for (int iAxis = 0; iAxis < AXIS_TYPE_END; iAxis++)
	{
		if (iAxis == AXIS_X || iAxis == AXIS_LOADER_R || iAxis == AXIS_LOADER_Z || iAxis == AXIS_X2 || iAxis == AXIS_X3)
		{
			if (pImpl->GetAxisProperty(iAxis)->bUse == FALSE)
				continue;

			m_stMoterInfo[iAxis].bEnable = pShare->stMotionStatus[iAxis].bEnable;
			m_stMoterInfo[iAxis].bLimitP = pShare->stMotionStatus[iAxis].bLimitP;
			m_stMoterInfo[iAxis].bLimitN = pShare->stMotionStatus[iAxis].bLimitN;
			m_stMoterInfo[iAxis].bHomeDone = pShare->stMotionStatus[iAxis].bHomeDone;
			m_stMoterInfo[iAxis].dAxisPos = pShare->stMotionStatus[iAxis].dAxisPos;
			m_stMoterInfo[iAxis].dAxisRMS = pShare->stMotionStatus[iAxis].dAxisRMS;
			m_stMoterInfo[iAxis].dPosition = pShare->stMotionStatus[iAxis].dAxisPos;

			// Name 
			strData.Format(_T("%s"), pImpl->GetAxisMapProperty(iAxis)->m_strAxisName);
			m_gridList[0].GetCell(i + 1, enumColumnName_Value1)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_gridList[0].SetItemState(i + 1, enumColumnName_Value1, GVIS_READONLY);
			m_gridList[0].SetItemText(i + 1, enumColumnName_Value1, strData);

			// Num
			strData.Format(_T("%d"), iAxis);
			m_gridList[0].GetCell(i + 1, enumColumnName_Value2)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_gridList[0].SetItemState(i + 1, enumColumnName_Value2, GVIS_READONLY);
			m_gridList[0].SetItemText(i + 1, enumColumnName_Value2, strData);

			// Current
			strData.Format(_T("%.4f"), m_stMoterInfo[iAxis].dAxisPos);
			m_gridList[0].GetCell(i + 1, enumColumnName_Value3)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_gridList[0].SetItemState(i + 1, enumColumnName_Value3, GVIS_READONLY);
			m_gridList[0].SetItemText(i + 1, enumColumnName_Value3, strData);


			// CheckBox - Enable
			CGridCellCheck* pCell;
			m_gridList[0].SetCellType(i + 1, enumColumnName_Value4, RUNTIME_CLASS(CGridCellCheck));
			m_gridList[0].SetItemState(i + 1, enumColumnName_Value4, GVIS_READONLY);
			pCell = (CGridCellCheck*)m_gridList[0].GetCell(i + 1, enumColumnName_Value4);

			if (m_stMoterInfo[iAxis].bEnable)
				pCell->SetCheck(true);
			else
				pCell->SetCheck(false);

			// CheckBox - Limit(+)
			m_gridList[0].SetCellType(i + 1, enumColumnName_Value5, RUNTIME_CLASS(CGridCellCheck));
			m_gridList[0].SetItemState(i + 1, enumColumnName_Value5, GVIS_READONLY);
			pCell = (CGridCellCheck*)m_gridList[0].GetCell(i + 1, enumColumnName_Value5);

			if (m_stMoterInfo[iAxis].bLimitP)
				pCell->SetCheck(true);
			else
				pCell->SetCheck(false);

			// CheckBox - Limit(-)
			m_gridList[0].SetCellType(i + 1, enumColumnName_Value6, RUNTIME_CLASS(CGridCellCheck));
			m_gridList[0].SetItemState(i + 1, enumColumnName_Value6, GVIS_READONLY);
			pCell = (CGridCellCheck*)m_gridList[0].GetCell(i + 1, enumColumnName_Value6);

			if (m_stMoterInfo[iAxis].bLimitN)
				pCell->SetCheck(true);
			else
				pCell->SetCheck(false);

			//// REL
			strData.Format(_T("%.4f"), 1.);
			m_gridList[0].GetCell(i + 1, enumColumnName_Value7)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_gridList[0].SetItemState(i + 1, enumColumnName_Value7, GVIS_READONLY);
			m_gridList[0].SetItemText(i + 1, enumColumnName_Value7, strData);

			// button - MOVE-
			m_gridList[0].SetCellType(i + 1, enumColumnName_Value8, RUNTIME_CLASS(CGridCellButton));
			m_gridList[0].SetItemState(i + 1, enumColumnName_Value8, GVIS_READONLY);
			m_gridList[0].SetItemText(i + 1, enumColumnName_Value8, _T("-"));

			// button - MOVE+
			m_gridList[0].SetCellType(i + 1, enumColumnName_Value9, RUNTIME_CLASS(CGridCellButton));
			m_gridList[0].SetItemState(i + 1, enumColumnName_Value9, GVIS_READONLY);
			m_gridList[0].SetItemText(i + 1, enumColumnName_Value9, _T("+"));

			i++;
		}
	}

	m_gridList[0].ExpandColumnsToFit(TRUE);
	m_gridList[0].SetRedraw(TRUE);
}

void CD_PopUp_Motion_Move::__InitListControl2()
{
	m_gridList[1].GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridList[1].GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridList[1].GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridList[1].SetDefCellMargin(0);
	m_gridList[1].SetFixedColumnSelection(FALSE);
	m_gridList[1].SetFixedRowSelection(FALSE);
	m_gridList[1].SetSingleColSelection(FALSE);
	m_gridList[1].SetSingleRowSelection(FALSE);
	m_gridList[1].SetTrackFocusCell(FALSE);
	m_gridList[1].SetRowResize(FALSE);
	m_gridList[1].SetColumnResize(FALSE);
}

void CD_PopUp_Motion_Move::__InitialListData2()
{
	// 결과값 리스트를 갱신한다.
	m_gridList[1].DeleteAllItems();
	m_gridList[1].SetColumnCount(enumColumnName_End);
	m_gridList[1].SetFixedRowCount(1);

	for (long n = 0; n < enumColumnName_End; ++n)
	{
		m_gridList[1].SetColumnWidth(n, ColumnSize[n]);
		m_gridList[1].SetItemText(0, n, ColumnName[n]);
		m_gridList[1].SetItemState(0, n, GVIS_READONLY);
		m_gridList[1].SetItemBkColour(0, n, RGB(172, 172, 172));
		m_gridList[1].SetItemFgColour(0, n, RGB(255, 255, 255));
	}

	// Items + Header
	m_gridList[1].SetEditable(TRUE);
	m_gridList[1].SetRowCount(AXIS_TYPE_END + 1);	// Axis type end 
	m_gridList[1].SetRowHeight(0, 22);

	for (long i = 0; i < AXIS_TYPE_END; i++)
	{
		m_gridList[1].SetRowHeight(i + 1, 0);
	}

	__RefreshListData2();
}

// FRONT Gantry
void CD_PopUp_Motion_Move::__RefreshListData2()
{
	m_gridList[1].SetRedraw(FALSE, TRUE);

	int i = 0;
	CString strData;
	CMotorControlMgr* pImpl = nullptr;
	pImpl = m_pMaintControlMgr->GetMotorControlMgrPtr();

	VTM_SYSTEM* pShare = nullptr;
	pShare = m_pMaintControlMgr->GetShareMemoryMgrPtr()->GetSharedMemory();

	for (int iAxis = 0; iAxis < AXIS_TYPE_END; iAxis++)
	{
		if (iAxis != AXIS_X && iAxis != AXIS_LOADER_R && iAxis != AXIS_LOADER_Z && iAxis != AXIS_X2 && iAxis != AXIS_X3 &&   // STAGE
			iAxis != AXIS_X4 && iAxis != AXIS_Z9 && iAxis != AXIS_UNIT_Z &&											  // AUTO CHANGE
			iAxis != AXIS_Y3 && iAxis != AXIS_Y4 && iAxis != AXIS_Y5 && iAxis != AXIS_Y6 && iAxis != AXIS_Z1 && iAxis != AXIS_Z5 && iAxis != AXIS_Z6 && iAxis != AXIS_Z7 && iAxis != AXIS_Z8 &&
			iAxis != AXIS_ZOOM1 && iAxis != AXIS_THETA1 && iAxis != AXIS_FILTER1 && iAxis != AXIS_VLENS1 && iAxis != AXIS_ZOOM2 && iAxis != AXIS_THETA2 && iAxis != AXIS_FILTER2 && iAxis != AXIS_VLENS2 &&
			iAxis < AXIS_PROBE_X1)
		{
			if (pImpl->GetAxisProperty(iAxis)->bUse == FALSE)
				continue;

			m_stMoterInfo[iAxis].bEnable = pShare->stMotionStatus[iAxis].bEnable;
			m_stMoterInfo[iAxis].bLimitP = pShare->stMotionStatus[iAxis].bLimitP;
			m_stMoterInfo[iAxis].bLimitN = pShare->stMotionStatus[iAxis].bLimitN;
			m_stMoterInfo[iAxis].bHomeDone = pShare->stMotionStatus[iAxis].bHomeDone;
			m_stMoterInfo[iAxis].dAxisPos = pShare->stMotionStatus[iAxis].dAxisPos;
			m_stMoterInfo[iAxis].dAxisRMS = pShare->stMotionStatus[iAxis].dAxisRMS;

			// Name 
			m_gridList[1].GetCell(i + 1, enumColumnName_Value1)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_gridList[1].SetItemState(i + 1, enumColumnName_Value1, GVIS_READONLY);
			m_gridList[1].SetItemText(i + 1, enumColumnName_Value1, pImpl->GetAxisMapProperty(iAxis)->m_strAxisName);

			// Num
			strData.Format(_T("%d"), iAxis);
			m_gridList[1].GetCell(i + 1, enumColumnName_Value2)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_gridList[1].SetItemState(i + 1, enumColumnName_Value2, GVIS_READONLY);
			m_gridList[1].SetItemText(i + 1, enumColumnName_Value2, strData);

			// Current
			strData.Format(_T("%.4f"), m_stMoterInfo[iAxis].dAxisPos);
			m_gridList[1].GetCell(i + 1, enumColumnName_Value3)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_gridList[1].SetItemState(i + 1, enumColumnName_Value3, GVIS_READONLY);
			m_gridList[1].SetItemText(i + 1, enumColumnName_Value3, strData);


			// CheckBox - Enable
			CGridCellCheck* pCell;
			m_gridList[1].SetCellType(i + 1, enumColumnName_Value4, RUNTIME_CLASS(CGridCellCheck));
			m_gridList[1].SetItemState(i + 1, enumColumnName_Value4, GVIS_READONLY);
			pCell = (CGridCellCheck*)m_gridList[1].GetCell(i + 1, enumColumnName_Value4);

			if (m_stMoterInfo[iAxis].bEnable)
				pCell->SetCheck(true);
			else
				pCell->SetCheck(false);

			// CheckBox - Limit(+)
			m_gridList[1].SetCellType(i + 1, enumColumnName_Value5, RUNTIME_CLASS(CGridCellCheck));
			m_gridList[1].SetItemState(i + 1, enumColumnName_Value5, GVIS_READONLY);
			pCell = (CGridCellCheck*)m_gridList[1].GetCell(i + 1, enumColumnName_Value5);

			if (m_stMoterInfo[iAxis].bLimitP)
				pCell->SetCheck(true);
			else
				pCell->SetCheck(false);

			// CheckBox - Limit(-)
			m_gridList[1].SetCellType(i + 1, enumColumnName_Value6, RUNTIME_CLASS(CGridCellCheck));
			m_gridList[1].SetItemState(i + 1, enumColumnName_Value6, GVIS_READONLY);
			pCell = (CGridCellCheck*)m_gridList[1].GetCell(i + 1, enumColumnName_Value6);

			if (m_stMoterInfo[iAxis].bLimitN)
				pCell->SetCheck(true);
			else
				pCell->SetCheck(false);

			// REL
			strData.Format(_T("%.4f"), 1.);
			m_gridList[1].GetCell(i + 1, enumColumnName_Value7)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_gridList[1].SetItemState(i + 1, enumColumnName_Value7, GVIS_READONLY);
			m_gridList[1].SetItemText(i + 1, enumColumnName_Value7, strData);

				// button - MOVE-
			m_gridList[1].SetCellType(i + 1, enumColumnName_Value8, RUNTIME_CLASS(CGridCellButton));
			m_gridList[1].SetItemState(i + 1, enumColumnName_Value8, GVIS_READONLY);
			m_gridList[1].SetItemText(i + 1, enumColumnName_Value8, _T("-"));

			// button - MOVE+
			m_gridList[1].SetCellType(i + 1, enumColumnName_Value9, RUNTIME_CLASS(CGridCellButton));
			m_gridList[1].SetItemState(i + 1, enumColumnName_Value9, GVIS_READONLY);
			m_gridList[1].SetItemText(i + 1, enumColumnName_Value9, _T("+"));

			m_gridList[1].SetRowHeight(i + 1, 18);

			i++;
		}
	}

	for (int iAxis = i + 1; iAxis < AXIS_TYPE_END; iAxis++)
	{
		m_gridList[1].SetRowHeight(iAxis, 0);
	}

	m_gridList[1].ExpandColumnsToFit(TRUE);
	m_gridList[1].SetRedraw(TRUE, TRUE);
}

void CD_PopUp_Motion_Move::__InitListControl3()
{
	m_gridList[2].GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridList[2].GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridList[2].GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridList[2].SetDefCellMargin(0);
	m_gridList[2].SetFixedColumnSelection(FALSE);
	m_gridList[2].SetFixedRowSelection(FALSE);
	m_gridList[2].SetSingleColSelection(FALSE);
	m_gridList[2].SetSingleRowSelection(FALSE);
	m_gridList[2].SetTrackFocusCell(FALSE);
	m_gridList[2].SetRowResize(FALSE);
	m_gridList[2].SetColumnResize(FALSE);
}

void CD_PopUp_Motion_Move::__InitialListData3()
{
	// 결과값 리스트를 갱신한다.
	m_gridList[2].DeleteAllItems();
	m_gridList[2].SetColumnCount(enumColumnName_End);
	m_gridList[2].SetFixedRowCount(1);

	for (long n = 0; n < enumColumnName_End; ++n)
	{
		m_gridList[2].SetColumnWidth(n, ColumnSize[n]);
		m_gridList[2].SetItemText(0, n, ColumnName[n]);
		m_gridList[2].SetItemState(0, n, GVIS_READONLY);
		m_gridList[2].SetItemBkColour(0, n, RGB(172, 172, 172));
		m_gridList[2].SetItemFgColour(0, n, RGB(255, 255, 255));
	}

	// Items + Header
	m_gridList[2].SetEditable(TRUE);
	m_gridList[2].SetRowCount(AXIS_TYPE_END + 1);	// Axis type end 
	m_gridList[2].SetRowHeight(0, 22);

	for (long i = 0; i < AXIS_TYPE_END; i++)
	{
		m_gridList[2].SetRowHeight(i + 1, 0);
	}

	__RefreshListData3();
}

void CD_PopUp_Motion_Move::__RefreshListData3()
{
	m_gridList[2].SetRedraw(FALSE, TRUE);

	int i = 0;
	CString strData;
	CMotorControlMgr* pImpl = nullptr;
	pImpl = m_pMaintControlMgr->GetMotorControlMgrPtr();

	VTM_SYSTEM* pShare = nullptr;
	pShare = m_pMaintControlMgr->GetShareMemoryMgrPtr()->GetSharedMemory();

	for (int iAxis = 0; iAxis < AXIS_TYPE_END; iAxis++)
	{
		if (iAxis != AXIS_X && iAxis != AXIS_LOADER_R && iAxis != AXIS_LOADER_Z && iAxis != AXIS_X2 && iAxis != AXIS_X3 &&
			iAxis != AXIS_X4 && iAxis != AXIS_Z9 && iAxis != AXIS_UNIT_Z &&
			iAxis != AXIS_Y1 && iAxis != AXIS_Y2 && iAxis != AXIS_Y5 && iAxis != AXIS_Y6 && iAxis != AXIS_Z2 && iAxis != AXIS_Z3 && iAxis != AXIS_Z4 &&
			iAxis != AXIS_ROTATE1 && iAxis != AXIS_FOCUS1 && iAxis != AXIS_COLOR_DEGREE1 && iAxis != AXIS_SPECTO_DEGREE1 && iAxis != AXIS_ROTATE2 && iAxis != AXIS_FOCUS2 && iAxis != AXIS_COLOR_DEGREE2 && iAxis != AXIS_SPECTO_DEGREE2 &&
			iAxis <= AXIS_PROBE_X1
			// LYS 2026-08-19 ADD ANGLE AXIS
			|| (AXIS_ANGLE_SF1 <= iAxis && iAxis <= AXIS_ANGLE_AZM2))
		{
			if (pImpl->GetAxisProperty(iAxis)->bUse == FALSE)
				continue;

			m_stMoterInfo[iAxis].bEnable = pShare->stMotionStatus[iAxis].bEnable;
			m_stMoterInfo[iAxis].bLimitP = pShare->stMotionStatus[iAxis].bLimitP;
			m_stMoterInfo[iAxis].bLimitN = pShare->stMotionStatus[iAxis].bLimitN;
			m_stMoterInfo[iAxis].bHomeDone = pShare->stMotionStatus[iAxis].bHomeDone;
			m_stMoterInfo[iAxis].dAxisPos = pShare->stMotionStatus[iAxis].dAxisPos;
			m_stMoterInfo[iAxis].dAxisRMS = pShare->stMotionStatus[iAxis].dAxisRMS;

			// Name 
			m_gridList[2].GetCell(i + 1, enumColumnName_Value1)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_gridList[2].SetItemState(i + 1, enumColumnName_Value1, GVIS_READONLY);
			m_gridList[2].SetItemText(i + 1, enumColumnName_Value1, pImpl->GetAxisMapProperty(iAxis)->m_strAxisName);

			// Num
			strData.Format(_T("%d"), iAxis);
			m_gridList[2].GetCell(i + 1, enumColumnName_Value2)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_gridList[2].SetItemState(i + 1, enumColumnName_Value2, GVIS_READONLY);
			m_gridList[2].SetItemText(i + 1, enumColumnName_Value2, strData);

			// Current
			strData.Format(_T("%.4f"), m_stMoterInfo[iAxis].dAxisPos);
			m_gridList[2].GetCell(i + 1, enumColumnName_Value3)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_gridList[2].SetItemState(i + 1, enumColumnName_Value3, GVIS_READONLY);
			m_gridList[2].SetItemText(i + 1, enumColumnName_Value3, strData);


			// CheckBox - Enable
			CGridCellCheck* pCell;
			m_gridList[2].SetCellType(i + 1, enumColumnName_Value4, RUNTIME_CLASS(CGridCellCheck));
			m_gridList[2].SetItemState(i + 1, enumColumnName_Value4, GVIS_READONLY);
			pCell = (CGridCellCheck*)m_gridList[2].GetCell(i + 1, enumColumnName_Value4);

			if (m_stMoterInfo[iAxis].bEnable)
				pCell->SetCheck(true);
			else
				pCell->SetCheck(false);

			// CheckBox - Limit(+)
			m_gridList[2].SetCellType(i + 1, enumColumnName_Value5, RUNTIME_CLASS(CGridCellCheck));
			m_gridList[2].SetItemState(i + 1, enumColumnName_Value5, GVIS_READONLY);
			pCell = (CGridCellCheck*)m_gridList[2].GetCell(i + 1, enumColumnName_Value5);

			if (m_stMoterInfo[iAxis].bLimitP)
				pCell->SetCheck(true);
			else
				pCell->SetCheck(false);

			// CheckBox - Limit(-)
			m_gridList[2].SetCellType(i + 1, enumColumnName_Value6, RUNTIME_CLASS(CGridCellCheck));
			m_gridList[2].SetItemState(i + 1, enumColumnName_Value6, GVIS_READONLY);
			pCell = (CGridCellCheck*)m_gridList[2].GetCell(i + 1, enumColumnName_Value6);

			if (m_stMoterInfo[iAxis].bLimitN)
				pCell->SetCheck(true);
			else
				pCell->SetCheck(false);

			// REL
			strData.Format(_T("%.4f"), 1.);
			m_gridList[2].GetCell(i + 1, enumColumnName_Value7)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_gridList[2].SetItemState(i + 1, enumColumnName_Value7, GVIS_READONLY);
			m_gridList[2].SetItemText(i + 1, enumColumnName_Value7, strData);

				// button - MOVE-
			m_gridList[2].SetCellType(i + 1, enumColumnName_Value8, RUNTIME_CLASS(CGridCellButton));
			m_gridList[2].SetItemState(i + 1, enumColumnName_Value8, GVIS_READONLY);
			m_gridList[2].SetItemText(i + 1, enumColumnName_Value8, _T("-"));

			// button - MOVE+
			m_gridList[2].SetCellType(i + 1, enumColumnName_Value9, RUNTIME_CLASS(CGridCellButton));
			m_gridList[2].SetItemState(i + 1, enumColumnName_Value9, GVIS_READONLY);
			m_gridList[2].SetItemText(i + 1, enumColumnName_Value9, _T("+"));

			m_gridList[2].SetRowHeight(i + 1, 18);

			i++;
		}
	}

	for (int iAxis = i + 1; iAxis < AXIS_TYPE_END; iAxis++)
	{
		m_gridList[2].SetRowHeight(iAxis, 0);
	}

	m_gridList[2].ExpandColumnsToFit(TRUE);
	m_gridList[2].SetRedraw(TRUE, TRUE);
}

void CD_PopUp_Motion_Move::__InitListControl4()
{
	m_gridList[3].GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridList[3].GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridList[3].GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridList[3].SetDefCellMargin(0);
	m_gridList[3].SetFixedColumnSelection(FALSE);
	m_gridList[3].SetFixedRowSelection(FALSE);
	m_gridList[3].SetSingleColSelection(FALSE);
	m_gridList[3].SetSingleRowSelection(FALSE);
	m_gridList[3].SetTrackFocusCell(FALSE);
	m_gridList[3].SetRowResize(FALSE);
	m_gridList[3].SetColumnResize(FALSE);
}

void CD_PopUp_Motion_Move::__InitialListData4()
{
	// 결과값 리스트를 갱신한다.
	m_gridList[3].DeleteAllItems();
	m_gridList[3].SetColumnCount(enumColumnName_End);
	m_gridList[3].SetFixedRowCount(1);

	for (long n = 0; n < enumColumnName_End; ++n)
	{
		m_gridList[3].SetColumnWidth(n, ColumnSize[n]);
		m_gridList[3].SetItemText(0, n, ColumnName[n]);
		m_gridList[3].SetItemState(0, n, GVIS_READONLY);
		m_gridList[3].SetItemBkColour(0, n, RGB(172, 172, 172));
		m_gridList[3].SetItemFgColour(0, n, RGB(255, 255, 255));
	}

	// Items + Header
	m_gridList[3].SetEditable(TRUE);
	m_gridList[3].SetRowCount(AXIS_TYPE_END + 1);	// Axis type end 
	m_gridList[3].SetRowHeight(0, 22);

	for (long i = 0; i < AXIS_TYPE_END; i++)
	{
		m_gridList[3].SetRowHeight(i + 1, 0);
	}

	__RefreshListData4();
}

void CD_PopUp_Motion_Move::__RefreshListData4()
{
	m_gridList[3].SetRedraw(FALSE, TRUE);

	int i = 0;
	CString strData;
	CMotorControlMgr* pImpl = nullptr;
	pImpl = m_pMaintControlMgr->GetMotorControlMgrPtr();

	VTM_SYSTEM* pShare = nullptr;
	pShare = m_pMaintControlMgr->GetShareMemoryMgrPtr()->GetSharedMemory();

	for (int iAxis = 0; iAxis < AXIS_TYPE_END; iAxis++)
	{
		if ((iAxis == AXIS_X4 || iAxis == AXIS_Z9 || iAxis == AXIS_UNIT_Z) || (iAxis >= AXIS_PROBE_X1 && iAxis <= AXIS_PROBE_Z5) || iAxis == AXIS_R1)
		{
			if (pImpl->GetAxisProperty(iAxis)->bUse == FALSE)
				continue;

			if (iAxis == AXIS_PROBE_X2 || iAxis == AXIS_PROBE_X4 || iAxis == AXIS_PROBE_X6 || iAxis == AXIS_PROBE_X8 || iAxis == AXIS_PROBE_X10 
				/*		||
				iAxis == AXIS_PROBE_Y1 || iAxis == AXIS_PROBE_Y2 || iAxis == AXIS_PROBE_Y3 || iAxis == AXIS_PROBE_Y4 || iAxis == AXIS_PROBE_Y5  ||
				iAxis == AXIS_PROBE_Z1 || iAxis == AXIS_PROBE_Z2 || iAxis == AXIS_PROBE_Z3 || iAxis == AXIS_PROBE_Z4 || iAxis == AXIS_PROBE_Z5*/)
			{
				m_gridList[3].SetRowHeight(i + 1, 0);
			}
			else
			{
				m_gridList[3].SetRowHeight(i + 1, 18);
			}

			if (iAxis == AXIS_PROBE_X1 || iAxis == AXIS_PROBE_X3 || iAxis == AXIS_PROBE_X5 || iAxis == AXIS_PROBE_X7 || iAxis == AXIS_PROBE_X9)
			{
				pShare->stMotionStatus[iAxis + 1].bHomeDone = pShare->stMotionStatus[iAxis].bHomeDone;
			}

			m_stMoterInfo[iAxis].bEnable = pShare->stMotionStatus[iAxis].bEnable;
			m_stMoterInfo[iAxis].bLimitP = pShare->stMotionStatus[iAxis].bLimitP;
			m_stMoterInfo[iAxis].bLimitN = pShare->stMotionStatus[iAxis].bLimitN;
			m_stMoterInfo[iAxis].bHomeDone = pShare->stMotionStatus[iAxis].bHomeDone;
			m_stMoterInfo[iAxis].dAxisPos = pShare->stMotionStatus[iAxis].dAxisPos;
			m_stMoterInfo[iAxis].dAxisRMS = pShare->stMotionStatus[iAxis].dAxisRMS;



			// Name 
			m_gridList[3].GetCell(i + 1, enumColumnName_Value1)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_gridList[3].SetItemState(i + 1, enumColumnName_Value1, GVIS_READONLY);
			m_gridList[3].SetItemText(i + 1, enumColumnName_Value1, pImpl->GetAxisMapProperty(iAxis)->m_strAxisName);

			// Num
			strData.Format(_T("%d"), iAxis);
			m_gridList[3].GetCell(i + 1, enumColumnName_Value2)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_gridList[3].SetItemState(i + 1, enumColumnName_Value2, GVIS_READONLY);
			m_gridList[3].SetItemText(i + 1, enumColumnName_Value2, strData);

			// Current
			strData.Format(_T("%.4f"), m_stMoterInfo[iAxis].dAxisPos);
			m_gridList[3].GetCell(i + 1, enumColumnName_Value3)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_gridList[3].SetItemState(i + 1, enumColumnName_Value3, GVIS_READONLY);
			m_gridList[3].SetItemText(i + 1, enumColumnName_Value3, strData);


			// CheckBox - Enable
			CGridCellCheck* pCell;
			m_gridList[3].SetCellType(i + 1, enumColumnName_Value4, RUNTIME_CLASS(CGridCellCheck));
			m_gridList[3].SetItemState(i + 1, enumColumnName_Value4, GVIS_READONLY);
			pCell = (CGridCellCheck*)m_gridList[3].GetCell(i + 1, enumColumnName_Value4);

			if (m_stMoterInfo[iAxis].bEnable)
				pCell->SetCheck(true);
			else
				pCell->SetCheck(false);

			// CheckBox - Limit(+)
			m_gridList[3].SetCellType(i + 1, enumColumnName_Value5, RUNTIME_CLASS(CGridCellCheck));
			m_gridList[3].SetItemState(i + 1, enumColumnName_Value5, GVIS_READONLY);
			pCell = (CGridCellCheck*)m_gridList[3].GetCell(i + 1, enumColumnName_Value5);

			if (m_stMoterInfo[iAxis].bLimitP)
				pCell->SetCheck(true);
			else
				pCell->SetCheck(false);

			// CheckBox - Limit(-)
			m_gridList[3].SetCellType(i + 1, enumColumnName_Value6, RUNTIME_CLASS(CGridCellCheck));
			m_gridList[3].SetItemState(i + 1, enumColumnName_Value6, GVIS_READONLY);
			pCell = (CGridCellCheck*)m_gridList[3].GetCell(i + 1, enumColumnName_Value6);

			if (m_stMoterInfo[iAxis].bLimitN)
				pCell->SetCheck(true);
			else
				pCell->SetCheck(false);

			// REL
			strData.Format(_T("%.4f"), 1.);
			m_gridList[3].GetCell(i + 1, enumColumnName_Value7)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_gridList[3].SetItemState(i + 1, enumColumnName_Value7, GVIS_READONLY);
			m_gridList[3].SetItemText(i + 1, enumColumnName_Value7, strData);

				// button - MOVE-
			m_gridList[3].SetCellType(i + 1, enumColumnName_Value8, RUNTIME_CLASS(CGridCellButton));
			m_gridList[3].SetItemState(i + 1, enumColumnName_Value8, GVIS_READONLY);
			m_gridList[3].SetItemText(i + 1, enumColumnName_Value8, _T("-"));

			// button - MOVE+
			m_gridList[3].SetCellType(i + 1, enumColumnName_Value9, RUNTIME_CLASS(CGridCellButton));
			m_gridList[3].SetItemState(i + 1, enumColumnName_Value9, GVIS_READONLY);
			m_gridList[3].SetItemText(i + 1, enumColumnName_Value9, _T("+"));

			i++;
		}
	}

	for (int iAxis = i + 1; iAxis < AXIS_TYPE_END; iAxis++)
	{
		m_gridList[3].SetRowHeight(iAxis, 0);
	}

	m_gridList[3].ExpandColumnsToFit(TRUE);
	m_gridList[3].SetRedraw(TRUE, TRUE);
}

UINT CD_PopUp_Motion_Move::ThreadRun(LPVOID pParam)
{
	CD_PopUp_Motion_Move* pThis = (CD_PopUp_Motion_Move*)pParam;

	pThis->m_bThreadLive = true;

	while (1)
	{
		DWORD dw = WaitForMultipleObjects(3, pThis->m_hMultiEvent, FALSE, INFINITE);

		if (dw == WAIT_OBJECT_0) // Start Event 1가 set될 때까지 기다린다.
		{
			pThis->ThreadFunc1();
		}
		else if (dw == WAIT_OBJECT_0 + 1) // Start Event 2가 set될 때까지 기다린다.
		{
			pThis->ThreadFunc2();
		}
		else if (dw == (WAIT_OBJECT_0 + 2)) // Close Event 가 set될 때까지 기다린다.
		{
			break;
		}
		else // timeout, error state
		{

		}
	}

	pThis->m_bThreadLive = false;

	return 0;
}

UINT CD_PopUp_Motion_Move::ThreadFunc1()
{
	if (m_iCurSel < 0)
		return 0;

	// MoveR (-)
	double dPos = 0.0;
	CString strVal = m_gridList[m_iGridSel].GetItemText(m_iCurSel + 1, enumColumnName_Value7);

	dPos = _ttof(strVal);

	CString strAxis = m_gridList[m_iGridSel].GetItemText(m_iCurSel + 1, enumColumnName_Value2);
	int nAxis = 0;
	nAxis = _ttoi(strAxis);

	{
		CMaintControlMgr* pImpl = nullptr;
		pImpl = m_pMaintControlMgr;
		int nRet = df_MOTION_RV_SUCCESSED;
		if (pImpl != nullptr)
		{
			nRet = pImpl->MoveCmdPosR(nAxis, dPos * -1, false, SPEED_LOW);

			if (nRet == df_MOTION_RV_SUCCESSED)
			{
				nRet = pImpl->DoWork_MotionDone(nAxis);
			}
			else
			{
				;
				//	CAlarmController::GetInstance()->SetAlarm(nRet);
			}
		}
	}

	return 0;
}

UINT CD_PopUp_Motion_Move::ThreadFunc2()
{
	if (m_iCurSel < 0)
		return 0;

	// MoveR (+)
	double dPos = 0.0;
	CString strVal = m_gridList[m_iGridSel].GetItemText(m_iCurSel + 1, enumColumnName_Value7);

	dPos = _ttof(strVal);

	CString strAxis = m_gridList[m_iGridSel].GetItemText(m_iCurSel + 1, enumColumnName_Value2);
	int nAxis = 0;
	nAxis = _ttoi(strAxis);

	CMaintControlMgr* pImpl = nullptr;
	pImpl = m_pMaintControlMgr;
	int nRet = df_MOTION_RV_SUCCESSED;
	if (pImpl != nullptr)
	{
		nRet = pImpl->MoveCmdPosR(nAxis, dPos * 1, false, SPEED_LOW);
		if (nRet == df_MOTION_RV_SUCCESSED)
		{
			nRet = pImpl->DoWork_MotionDone(nAxis);
		}
		else
		{
			;
			//	CAlarmController::GetInstance()->SetAlarm(nRet);
		}
	}

	return 0;
}

UINT CD_PopUp_Motion_Move::ThreadFuncDone(LPARAM lp)
{
	if (IsWindowVisible() == TRUE)
	{
		CString strItemName = CString(LPCTSTR(lp));
		CString str = _T(" - SeqItem Done!!");
		CString strLog = strItemName + str;
		CLogWrapper::GetInstance()->KMLOG(OPER, strLog);

		AfxMessageBox(strLog);
	}

	return 0;
}

void CD_PopUp_Motion_Move::OnGridItemClickList1(NMHDR* pNotifyStruct, LRESULT* result)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*)pNotifyStruct;

	const long row = pNotify->iRow;
	const long col = pNotify->iColumn;

	m_iGridSel = 0;

	if ((0 < row) && (enumColumnName_Value8 == col))	// MOVE+
	{
		m_iCurSel = row - 1;
		SetEvent(m_hStart1Event);
	}

	if ((0 < row) && (enumColumnName_Value9 == col))	// MOVE-
	{
		m_iCurSel = row - 1;
		SetEvent(m_hStart2Event);
	}

	*result = 0;
}

void CD_PopUp_Motion_Move::OnGridItemClickList2(NMHDR* pNotifyStruct, LRESULT* result)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*)pNotifyStruct;

	const long row = pNotify->iRow;
	const long col = pNotify->iColumn;

	m_iGridSel = 1;

	if ((0 < row) && (enumColumnName_Value8 == col))	// MOVE+
	{
		m_iCurSel = row - 1;
		SetEvent(m_hStart1Event);
	}

	if ((0 < row) && (enumColumnName_Value9 == col))	// MOVE-
	{
		m_iCurSel = row - 1;
		SetEvent(m_hStart2Event);
	}

	*result = 0;
}

void CD_PopUp_Motion_Move::OnGridItemClickList3(NMHDR* pNotifyStruct, LRESULT* result)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*)pNotifyStruct;

	const long row = pNotify->iRow;
	const long col = pNotify->iColumn;

	m_iGridSel = 2;

	if ((0 < row) && (enumColumnName_Value8 == col))	// MOVE+
	{
		m_iCurSel = row - 1;
		SetEvent(m_hStart1Event);
	}

	if ((0 < row) && (enumColumnName_Value9 == col))	// MOVE-
	{
		m_iCurSel = row - 1;
		SetEvent(m_hStart2Event);
	}

	*result = 0;
}

void CD_PopUp_Motion_Move::OnGridItemClickList4(NMHDR* pNotifyStruct, LRESULT* result)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*)pNotifyStruct;

	const long row = pNotify->iRow;
	const long col = pNotify->iColumn;

	m_iGridSel = 3;

	if ((0 < row) && (enumColumnName_Value8 == col))	// MOVE+
	{
		m_iCurSel = row - 1;
		SetEvent(m_hStart1Event);
	}

	if ((0 < row) && (enumColumnName_Value9 == col))	// MOVE-
	{
		m_iCurSel = row - 1;
		SetEvent(m_hStart2Event);
	}

	*result = 0;
}

void CD_PopUp_Motion_Move::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	CDialogEx::OnTimer(nIDEvent);

	if (nIDEvent == 1000)
	{
		if(IsWindowVisible())
			__RefreshListData();
	}
}


void CD_PopUp_Motion_Move::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	KillTimer(1000);
}
