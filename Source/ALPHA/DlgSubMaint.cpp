// DlgSubMaint.cpp : 구현 파일입니다.
//

#include "pch.h"
#include "ALPHA.h"
#include "DlgSubMaint.h"
#include "afxdialogex.h"

#include "AuroraMgr.h"
//#include "Manager\IOMapControlMgr.h"
//#include "Manager\MotorControlMgr.h"
#include "Manager\MaintControlMgr.h"
#include "Manager\LightControlMgr.h"
#include "Manager\SequenceManager.h"
//#include "AlarmController.h"
#include "FunctionModule\FunctionModuleAFControl.h"
#include "DlgSubMaint_MotionFile.h"
#include "DlgSubMaint_IOFile.h"
#include "DlgSubMaint_ETCFile.h"

#include "DlgSubMeasure_Progress.h"

// CDlgSubMaint 대화 상자입니다.
#define _df_MAX_IO_OUTPUT			32
#define _df_MAX_MOTION			20
#define _df_MAX_LIGHT			3

static CString ColumnName[] =
{
	_T("Name."),
	_T("HOME"),
	_T("ABS"),
	_T("MOVE"),
	_T("Relative"),
	_T("-"),
	_T("+"),
	_T(""),
};
static long ColumnSize[] =
{
	120,
	70,
	50,
	50,
	70,
	50,
	50,
	0,
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
	enumColumnName_End,
};

static CString ColumnName2[] =
{
	_T("No."),
	_T("Description"),
	_T("-"),
	_T("+"),
};
static long ColumnSize2[] =
{
	40,
	50,
	50,
	50,
};
enum enumColumnName2
{
	enumColumnName2_No = 0,
	enumColumnName2_Value1,
	enumColumnName2_Value2,
	enumColumnName2_Value3,
	enumColumnName2_End,
};

static CString ColumnName3[] =
{
	_T("No."),
	_T("Description"),
	_T("ON"),
	_T("OFF"),
};
static long ColumnSize3[] =
{
	40,
	200,
	50,
	50,
};
enum enumColumnName3
{
	enumColumnName3_No = 0,
	enumColumnName3_Value1,
	enumColumnName3_Value2,
	enumColumnName3_Value3,
	enumColumnName3_End,
};

static CString ColumnIOOutput[] =
{
	_T("BUZZER ON"),
	_T("VOICE BUZZER CH1 ON"),
	_T("VOICE BUZZER CH2 ON"),
	_T("VOICE BUZZER CH3 ON"),
	_T("VOICE BUZZER CH4 ON"),
	_T("SELECT KEY LOCK"),
	_T(""),
	_T("CCD 01 ON (MEAS 1)"),
	_T("CCD 02 ON (MEAS 2)"),
	_T("MCM+ 01 ON (MEAS 1)"),
	_T("MCM+ 02 ON (MEAS 2)"),
	_T("INTERLOCK ON(Temperature Alarm)"),
	_T("DOOR UNLOCK"),
	_T(""),
	_T("MULTITAP ON"),
	_T(""),
	_T("ACS MOTION INTERLOCK 1"),
	_T("ACS MOTION INTERLOCK 2"),
	_T(""),
	_T(""),
	_T(""),
	_T("SIGNAL TOWER RED"),
	_T("SIGNAL TOWER YELLOW"),
	_T("SIGNAL TOWER GREEN"),
	_T("SIGNAL TOWER BUZZER 01"),
	_T(""),
	_T(""),
	_T(""),
	_T("VACCUM ON(STAGE)"),
	_T("VACCUM ON(ALIGN)"),
	_T("BLOWING ON(STAGE)"),
	_T("BLOWING ON(ALIGN)"),
};

IMPLEMENT_DYNAMIC(CDlgSubMaint, CMainSubDlgWin)

CDlgSubMaint::CDlgSubMaint(CAuroraMgr *pAuroraMgr, CWnd* pParent /*=NULL*/)
	: CMainSubDlgWin(IDD_DLG_SUB_MAINT, pParent)
	, m_pAuroraMgr(pAuroraMgr)
	, m_iCurSel(-1)
	, m_iCurSelLight(-1)
{
	//----------------------------------------------------------------------
	// Thread 를 미리 생성해 놓고 Event 로 제어 한다.
	//----------------------------------------------------------------------
	m_pThread = AfxBeginThread(ThreadRun, this);
	m_bThreadLive = false;
	m_lMotionBusy = 0;	// LYS 2026-09-17 motion busy flag (duplicate click guard)
	m_hStart1Event = CreateEvent(NULL, FALSE, FALSE, NULL);		// auto reset, initial reset
	m_hStart2Event = CreateEvent(NULL, FALSE, FALSE, NULL);		// auto reset, initial reset
	m_hStart3Event = CreateEvent(NULL, FALSE, FALSE, NULL);		// auto reset, initial reset
	m_hStart4Event = CreateEvent(NULL, FALSE, FALSE, NULL);		// auto reset, initial reset
	m_hCloseEvent = CreateEvent(NULL, FALSE, FALSE, NULL);		// auto reset, initial reset
	m_hMultiEvent[0] = m_hStart1Event;
	m_hMultiEvent[1] = m_hStart2Event;
	m_hMultiEvent[2] = m_hStart3Event;
	m_hMultiEvent[3] = m_hStart4Event;
	m_hMultiEvent[4] = m_hCloseEvent;

	m_pDlgSub_Motion = nullptr;
	m_pDlgSub_IO = nullptr;
	m_pDlgSub_Etc = nullptr;
}

CDlgSubMaint::~CDlgSubMaint()
{
	if (m_pDlgSub_Motion)
	{
		m_pDlgSub_Motion->DestroyWindow();
		delete m_pDlgSub_Motion;
		m_pDlgSub_Motion = nullptr;
	}

	if (m_pDlgSub_IO)
	{
		m_pDlgSub_IO->DestroyWindow();
		delete m_pDlgSub_IO;
		m_pDlgSub_IO = nullptr;
	}

	if (m_pDlgSub_Etc)
	{
		m_pDlgSub_Etc->DestroyWindow();
		delete m_pDlgSub_Etc;
		m_pDlgSub_Etc = nullptr;
	}

	//----------------------------------------------------------------------
	// Thread 종료
	//----------------------------------------------------------------------
	ResetEvent(m_hStart1Event);
	ResetEvent(m_hStart2Event);
	ResetEvent(m_hStart3Event);
	ResetEvent(m_hStart4Event);
	SetEvent(m_hCloseEvent);
	if (m_pThread && m_bThreadLive)
	{
		WaitForSingleObject(m_pThread->m_hThread, INFINITE);
	}
	if (m_hStart1Event)CloseHandle(m_hStart1Event);
	if (m_hStart2Event)CloseHandle(m_hStart2Event);
	if (m_hStart3Event)CloseHandle(m_hStart3Event);
	if (m_hStart4Event)CloseHandle(m_hStart4Event);
	if (m_hCloseEvent) CloseHandle(m_hCloseEvent);
}

void CDlgSubMaint::DoDataExchange(CDataExchange* pDX)
{
	CMainSubDlgWin::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgSubMaint, CMainSubDlgWin)

	ON_BN_CLICKED(IDC_BUTTON_MAINT_MOTION, &CDlgSubMaint::OnBnClickedButtonMaintMotion)
	ON_BN_CLICKED(IDC_BUTTON_MAINT_IO, &CDlgSubMaint::OnBnClickedButtonMaintIo)
	ON_BN_CLICKED(IDC_BUTTON_MAINT_ETC, &CDlgSubMaint::OnBnClickedButtonMaintSerial)
	ON_BN_CLICKED(IDC_BUTTON_MAINT_ETC2, &CDlgSubMaint::OnBnClickedButtonMaintEtc2)
END_MESSAGE_MAP()


// CDlgSubMaint 메시지 처리기입니다.


BOOL CDlgSubMaint::OnInitDialog()
{
	CMainSubDlgWin::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	CRect rtROI;
	GetDlgItem(IDC_STATIC_MAIN_TITLE)->GetWindowRect(&rtROI);
	ScreenToClient(&rtROI);

	m_pDlgSub_Motion = new CDlgSubMaint_MotionFile(m_pAuroraMgr);
	m_pDlgSub_Motion->Create(IDD_DLG_SUB_MAINT_MOTION, this);
	m_pDlgSub_Motion->ShowWindow(SW_HIDE);
//	m_pDlgSub_Motion->MoveWindow(rtROI);

	m_pDlgSub_IO = new CDlgSubMaint_IOFile(m_pAuroraMgr);
	m_pDlgSub_IO->Create(IDD_DLG_SUB_MAINT_IO, this);
	m_pDlgSub_IO->ShowWindow(SW_HIDE);
//	m_pDlgSub_IO->MoveWindow(rtROI);

	m_pDlgSub_Etc = new CDlgSubMaint_ETCFile();
	m_pDlgSub_Etc->Create(IDD_DLG_SUB_MAINT_ETC, this);
	m_pDlgSub_Etc->ShowWindow(SW_HIDE);
//	m_pDlgSub_Etc->MoveWindow(rtROI);

	m_iCurFileView = 1;
	RefreshFileView(TRUE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


//void CDlgSubMaint::__InitListControl()
//{
//	m_gridListIOOutp.GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
//	m_gridListIOOutp.GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
//	m_gridListIOOutp.GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
//	m_gridListIOOutp.SetDefCellMargin(0);
//	m_gridListIOOutp.SetFixedColumnSelection(FALSE);
//	m_gridListIOOutp.SetFixedRowSelection(FALSE);
//	m_gridListIOOutp.SetSingleColSelection(FALSE);
//	m_gridListIOOutp.SetSingleRowSelection(FALSE);
//	m_gridListIOOutp.SetTrackFocusCell(FALSE);
//	m_gridListIOOutp.SetRowResize(FALSE);
//	m_gridListIOOutp.SetColumnResize(FALSE);
//}
//
//
//void CDlgSubMaint::__InitialListData()
//{
//	// 결과값 리스트를 갱신한다.
//	m_gridListIOOutp.DeleteAllItems();
//	m_gridListIOOutp.SetColumnCount(enumColumnName3_End);
//	m_gridListIOOutp.SetFixedRowCount(1);
//
//	for (long n = 0; n < enumColumnName3_End; ++n)
//	{
//		m_gridListIOOutp.SetColumnWidth(n, ColumnSize3[n]);
//		m_gridListIOOutp.SetItemText(0, n, ColumnName3[n]);
//		m_gridListIOOutp.SetItemState(0, n, GVIS_READONLY);
//		m_gridListIOOutp.SetItemBkColour(0, n, RGB(172, 172, 172));
//		m_gridListIOOutp.SetItemFgColour(0, n, RGB(255, 255, 255));
//	}
//
//	// Items + Header
//	m_gridListIOOutp.SetEditable(TRUE);
//	m_gridListIOOutp.SetRowCount(1 + _df_MAX_IO_OUTPUT);
//	m_gridListIOOutp.SetRowHeight(0, 18);
//
//	for (long i = 0; i < _df_MAX_IO_OUTPUT; i++)
//	{
//		m_gridListIOOutp.SetRowHeight(i + 1, 20);
//
//		CString strNo;
//		strNo.Format(_T("[%d]"), i + 1);
//
//		m_gridListIOOutp.GetCell(i + 1, enumColumnName3_No)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
//		m_gridListIOOutp.SetItemState(i + 1, enumColumnName3_No, GVIS_READONLY);
//		m_gridListIOOutp.SetItemText(i + 1, enumColumnName3_No, strNo);
//
//		// description
//		if (i < _df_MAX_IO_OUTPUT)
//		{
//			CString strBitImportName;
//			strBitImportName.Format(_T("%s"), ColumnIOOutput[i]);
//			m_gridListIOOutp.SetItemText(i + 1, enumColumnName3_Value1, strBitImportName);
//			m_gridListIOOutp.SetItemState(i + 1, enumColumnName3_Value1, GVIS_READONLY);
//
//			// button - on
//			m_gridListIOOutp.SetCellType(i + 1, enumColumnName3_Value2, RUNTIME_CLASS(CGridCellButton));
//			m_gridListIOOutp.SetItemState(i + 1, enumColumnName3_Value2, GVIS_READONLY);
//			m_gridListIOOutp.SetItemText(i + 1, enumColumnName3_Value2, _T("ON"));
//
//			// button - Off
//			m_gridListIOOutp.SetCellType(i + 1, enumColumnName3_Value3, RUNTIME_CLASS(CGridCellButton));
//			m_gridListIOOutp.SetItemState(i + 1, enumColumnName3_Value3, GVIS_READONLY);
//			m_gridListIOOutp.SetItemText(i + 1, enumColumnName3_Value3, _T("OFF"));
//		}
//	}
//
//	__RefreshListData();
//}
//
//
//void CDlgSubMaint::__RefreshListData()
//{
//	m_gridListIOOutp.SetRedraw(FALSE);
//
//	for (long i = 0; i < _df_MAX_IO_OUTPUT; i++)
//	{
//		if (i < _df_MAX_IO_OUTPUT)
//		{
//			// 활성화 되어 있는 상태를 표시해준다.
//			if (m_pAuroraMgr->GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitExport(i) == true)
//			{
//				for (long n = enumColumnName3_Value1; n < enumColumnName3_End; ++n)
//				{
//					m_gridListIOOutp.SetItemBkColour(i + 1, n, CLR_LIGHT_YELLOW);
//				}
//			}
//			else
//			{
//				for (long n = enumColumnName3_Value1; n < enumColumnName3_End; ++n)
//				{
//					m_gridListIOOutp.SetItemBkColour(i + 1, n, CLR_GRAY);
//				}
//			}
//		}
//	}
//
//	m_gridListIOOutp.SetRedraw(TRUE);
//}
//
//void CDlgSubMaint::__InitListControl2()
//{
//	m_gridListMotionTest.GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
//	m_gridListMotionTest.GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
//	m_gridListMotionTest.GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
//	m_gridListMotionTest.SetDefCellMargin(0);
//	m_gridListMotionTest.SetFixedColumnSelection(FALSE);
//	m_gridListMotionTest.SetFixedRowSelection(FALSE);
//	m_gridListMotionTest.SetSingleColSelection(FALSE);
//	m_gridListMotionTest.SetSingleRowSelection(FALSE);
//	m_gridListMotionTest.SetTrackFocusCell(FALSE);
//	m_gridListMotionTest.SetRowResize(FALSE);
//	m_gridListMotionTest.SetColumnResize(FALSE);
//}
//
//
//void CDlgSubMaint::__InitialListData2()
//{
//	// 결과값 리스트를 갱신한다.
//	m_gridListMotionTest.DeleteAllItems();
//	m_gridListMotionTest.SetColumnCount(enumColumnName_End);
//	m_gridListMotionTest.SetFixedRowCount(1);
//
//	for (long n = 0; n < enumColumnName_End; ++n)
//	{
//		m_gridListMotionTest.SetColumnWidth(n, ColumnSize[n]);
//		m_gridListMotionTest.SetItemText(0, n, ColumnName[n]);
//		m_gridListMotionTest.SetItemState(0, n, GVIS_READONLY);
//		m_gridListMotionTest.SetItemBkColour(0, n, RGB(172, 172, 172));
//		m_gridListMotionTest.SetItemFgColour(0, n, RGB(255, 255, 255));
//	}
//
//	// Items + Header
//	m_gridListMotionTest.SetEditable(TRUE);
//	m_gridListMotionTest.SetRowCount(_df_MAX_MOTION + 1);	// Axis type end 
//
//	m_gridListMotionTest.SetRowHeight(0, 18);
//	for (long i = 0; i < _df_MAX_MOTION; i++)
//	{
//		m_gridListMotionTest.SetRowHeight(i + 1, 30);
//	}
//
//	__RefreshListData2();
//}
//
//
//void CDlgSubMaint::__RefreshListData2()
//{
//	m_gridListMotionTest.SetRedraw(FALSE);
//
//	int i = 0;
//	CString strData;
//	CMotorControlMgr *pImpl = nullptr;
//	pImpl = m_pAuroraMgr->GetMaintControlMgr()->GetMotorControlMgrPtr();
//	for (int iAxis = 0; iAxis < AXIS_TYPE_END; iAxis++)
//	{
//		if (pImpl->GetAxisProperty(iAxis)->bUse == FALSE)
//			continue;
//
//		// Name 
//		m_gridListMotionTest.GetCell(i + 1, enumColumnName_Value1)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
//		m_gridListMotionTest.SetItemState(i + 1, enumColumnName_Value1, GVIS_READONLY);
//		m_gridListMotionTest.SetItemText(i + 1, enumColumnName_Value1, pImpl->GetAxisMapProperty(iAxis)->m_strAxisName);
//
//		// Num
//		strData.Format(_T("%d"), iAxis);
//		m_gridListMotionTest.GetCell(i + 1, enumColumnName_Value8)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
//		m_gridListMotionTest.SetItemState(i + 1, enumColumnName_Value8, GVIS_READONLY);
//		m_gridListMotionTest.SetItemText(i + 1, enumColumnName_Value8, strData);
//
//		// button - MOVE
//		m_gridListMotionTest.SetCellType(i + 1, enumColumnName_Value2, RUNTIME_CLASS(CGridCellButton));
//		m_gridListMotionTest.SetItemState(i + 1, enumColumnName_Value2, GVIS_READONLY);
//		m_gridListMotionTest.SetItemText(i + 1, enumColumnName_Value2, _T("HOME"));
//
//		strData.Format(_T("%.4f"), 0);
//		m_gridListMotionTest.GetCell(i + 1, enumColumnName_Value3)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
//		m_gridListMotionTest.SetItemState(i + 1, enumColumnName_Value3, GVIS_MODIFIED);
//		m_gridListMotionTest.SetItemText(i + 1, enumColumnName_Value3, strData);
//
//		// button - MOVE
//		m_gridListMotionTest.SetCellType(i + 1, enumColumnName_Value4, RUNTIME_CLASS(CGridCellButton));
//		m_gridListMotionTest.SetItemState(i + 1, enumColumnName_Value4, GVIS_READONLY);
//		m_gridListMotionTest.SetItemText(i + 1, enumColumnName_Value4, _T("MOVE"));
//
//		strData.Format(_T("%.4f"), 0);
//		m_gridListMotionTest.GetCell(i + 1, enumColumnName_Value5)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
//		m_gridListMotionTest.SetItemState(i + 1, enumColumnName_Value5, GVIS_MODIFIED);
//		m_gridListMotionTest.SetItemText(i + 1, enumColumnName_Value5, strData);
//
//		// button - MOVE-
//		m_gridListMotionTest.SetCellType(i + 1, enumColumnName_Value6, RUNTIME_CLASS(CGridCellButton));
//		m_gridListMotionTest.SetItemState(i + 1, enumColumnName_Value6, GVIS_READONLY);
//		m_gridListMotionTest.SetItemText(i + 1, enumColumnName_Value6, _T("-"));
//
//		// button - MOVE+
//		m_gridListMotionTest.SetCellType(i + 1, enumColumnName_Value7, RUNTIME_CLASS(CGridCellButton));
//		m_gridListMotionTest.SetItemState(i + 1, enumColumnName_Value7, GVIS_READONLY);
//		m_gridListMotionTest.SetItemText(i + 1, enumColumnName_Value7, _T("+"));
//
//		i++;
//	}
//
//	for (int iAxis = i; iAxis < _df_MAX_MOTION; iAxis++)
//	{
//		m_gridListMotionTest.SetRowHeight(iAxis + 1, 0);
//	}
//
//	m_gridListMotionTest.ExpandColumnsToFit(TRUE);
//	m_gridListMotionTest.SetRedraw(TRUE);
//}

void CDlgSubMaint::OnGridItemClickList(NMHDR* pNotifyStruct, LRESULT* result)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*)pNotifyStruct;

	const long row = pNotify->iRow;
	const long col = pNotify->iColumn;

	if ((0 < row) && (enumColumnName3_Value2 == col))	// ON
	{
		m_pAuroraMgr->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(row-1, true);
	}

	if ((0 < row) && (enumColumnName3_Value3 == col))	// OFF
	{
		m_pAuroraMgr->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(row - 1, false);
	}

	*result = 0;
}

void CDlgSubMaint::OnGridItemClickList2(NMHDR* pNotifyStruct, LRESULT* result)
{
	// LYS 2026-09-17 ignore the click while a motion is running (double click = duplicate move)
	if (::InterlockedCompareExchange(&m_lMotionBusy, 0, 0) != 0)
	{
		*result = 0;
		return;
	}

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*)pNotifyStruct;

	const long row = pNotify->iRow;
	const long col = pNotify->iColumn;

	if ((0 < row) && (enumColumnName_Value2 == col))	// Homing
	{
		SetEvent(m_hStart1Event);
	}

	if ((0 < row) && (enumColumnName_Value4 == col))	// MOVE
	{
		SetEvent(m_hStart2Event);
	}

	if ((0 < row) && (enumColumnName_Value6 == col))	// MOVE+
	{
		SetEvent(m_hStart3Event);
	}

	if ((0 < row) && (enumColumnName_Value7 == col))	// MOVE-
	{
		SetEvent(m_hStart4Event);
	}

	*result = 0;
}

void CDlgSubMaint::OnGridItemClickList3(NMHDR* pNotifyStruct, LRESULT* result)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*)pNotifyStruct;

	const long row = pNotify->iRow;
	const long col = pNotify->iColumn;

	int nLampIntensity = 0;
	CString strVal = m_gridListLightCtrl.GetItemText(m_iCurSelLight, enumColumnName2_Value1);
	nLampIntensity = _ttoi(strVal);

	if ((0 < row) && (enumColumnName2_Value2 == col))	// -
	{
		nLampIntensity -= 1;

		if( nLampIntensity >= 0)
			m_pAuroraMgr->GetLightControlMgrPtr()->SetLedLampIntensity(0, row-1, nLampIntensity);
	}

	if ((0 < row) && (enumColumnName2_Value3 == col))	// +
	{
		nLampIntensity += 1;

		if (nLampIntensity <= 255)
			m_pAuroraMgr->GetLightControlMgrPtr()->SetLedLampIntensity(0, row - 1, nLampIntensity);
	}

//	__RefreshListData3();

	*result = 0;
}

void CDlgSubMaint::OnGridItemActivateList(NMHDR *pNotifyStruct, LRESULT *result)
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

void CDlgSubMaint::OnGridItemActivateList3(NMHDR *pNotifyStruct, LRESULT *result)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	NM_GRIDVIEW *pNotify = (NM_GRIDVIEW *)pNotifyStruct;

	const long row = pNotify->iRow;
	const long col = pNotify->iColumn;

	if (1 <= row)
	{
		m_iCurSelLight = row;

//		__RefreshListData3();
	}

	*result = 0;
}

void CDlgSubMaint::OnGridItemChangedList(NMHDR *pNotifyStruct, LRESULT *result)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	NM_GRIDVIEW *pNotify = (NM_GRIDVIEW *)pNotifyStruct;

	const long row = pNotify->iRow;
	const long col = pNotify->iColumn;

	const CString strValue = m_gridListMotionTest.GetItemText(row, col);

	switch (col)
	{
	case enumColumnName_Value1:
	{
		;
	}
	break;

	default:
		break;
	}

	*result = 0;
}


void CDlgSubMaint::UpdateInformation()
{
	m_pDlgSub_Motion->__RefreshListData();
//	__RefreshListData();
}

void CDlgSubMaint::RefreshListData()
{
//	__RefreshListData();
//	__RefreshListData3();
}

UINT CDlgSubMaint::ThreadRun(LPVOID pParam)
{
	CDlgSubMaint *pThis = (CDlgSubMaint *)pParam;

	pThis->m_bThreadLive = true;

	while (1)
	{
		DWORD dw = WaitForMultipleObjects(5, pThis->m_hMultiEvent, FALSE, INFINITE);

		// LYS 2026-09-17 a click landing while a motion runs must not queue a second move.
		//                the start events are auto reset, so a double click would otherwise
		//                fire the same ThreadFunc again as soon as the first one returns.
		::InterlockedExchange(&pThis->m_lMotionBusy, 1);

		if (dw == WAIT_OBJECT_0) // Start Event 1가 set될 때까지 기다린다.
		{
			pThis->ThreadFunc1();
		}
		else if (dw == WAIT_OBJECT_0 + 1) // Start Event 2가 set될 때까지 기다린다.
		{
			pThis->ThreadFunc2();
		}
		else if (dw == (WAIT_OBJECT_0 + 2)) // Start Event 3가 set될 때까지 기다린다.
		{
			pThis->ThreadFunc3();
		}
		else if (dw == (WAIT_OBJECT_0 + 3)) // Start Event 4가 set될 때까지 기다린다.
		{
			pThis->ThreadFunc4();
		}
		else if (dw == (WAIT_OBJECT_0 + 4)) // Close Event 가 set될 때까지 기다린다.
		{
			break;
		}
		else // timeout, error state
		{
		}

		// LYS 2026-09-17 drop the clicks that piled up during the motion, then release the flag.
		ResetEvent(pThis->m_hStart1Event);
		ResetEvent(pThis->m_hStart2Event);
		ResetEvent(pThis->m_hStart3Event);
		ResetEvent(pThis->m_hStart4Event);
		::InterlockedExchange(&pThis->m_lMotionBusy, 0);
	}

	pThis->m_bThreadLive = false;

	return 0;
}


UINT CDlgSubMaint::ThreadFunc1()
{
	if (m_iCurSel < 0)
		return 0;


	CString str;
	str.Format(_T("Do you want Homing [%d - %s Axis] ???"), m_iCurSel + 1, m_gridListMotionTest.GetItemText(m_iCurSel + 1, enumColumnName_Value1));
	 
	if (AfxMessageBox(str, MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
		CLogWrapper::GetInstance()->KMLOG(OPER, str + _T("-START"));

		CString strAxis = m_gridListMotionTest.GetItemText(m_iCurSel + 1, enumColumnName_Value8);
		int nAxis = 0;
		nAxis = _ttoi(strAxis);

		long lRet = 0;
		// Homing Start

		/*if (nAxis == AXIS_Z7 || nAxis == AXIS_Z8)
		{
			if(nAxis == AXIS_Z7)
				lRet = CSequenceManager::GetInstance()->GetFunctionModuleAFControl()->HomingAFModule(0);
			else
				lRet = CSequenceManager::GetInstance()->GetFunctionModuleAFControl()->HomingAFModule(1);
		}
		else*/
		{	
			CMaintControlMgr *pImpl = nullptr;
			pImpl = m_pAuroraMgr->GetMaintControlMgr();
			if (pImpl != nullptr)
			{
				if (nAxis == AXIS_X || nAxis == AXIS_Y || nAxis == AXIS_R || nAxis == AXIS_LOADER_R)
				{
					if (pImpl->GetCurPos(AXIS_UNIT_Z, 0, 0) < 0.1)
					{
						//pImpl->MoveHome(nAxis);
						//lRet = pImpl->CheckHomeDone(nAxis, df_MOVINGDONE_TIME);
						// LYS 2026-09-21 원점복귀가 거부되면(인터락 - 알람은 MoveHome 이 이미 냈다) 완료를 기다리지 않는다.
						//                전에는 반환값을 버리고 CheckHomeDone 으로 최대 대기했다(그동안 화면 워커가 묶인다).
						long lHome = pImpl->MoveHome(nAxis);
						if (lHome > 0 && lHome < 1000)
							lRet = pImpl->CheckHomeDone(nAxis, df_MOVINGDONE_TIME);
						else
							lRet = lHome;
					}
					else
					{
						CLogWrapper::GetInstance()->KMLOG(ERRO, str + _T("-PICKER CHECK"));

						return 0;
					}
				}
				else
				{
					//pImpl->MoveHome(nAxis);
					//lRet = pImpl->CheckHomeDone(nAxis, df_MOVINGDONE_TIME);
					// LYS 2026-09-21 원점복귀가 거부되면(인터락 - 알람은 MoveHome 이 이미 냈다) 완료를 기다리지 않는다.
					//                전에는 반환값을 버리고 CheckHomeDone 으로 최대 대기했다(그동안 화면 워커가 묶인다).
					long lHome = pImpl->MoveHome(nAxis);
					if (lHome > 0 && lHome < 1000)
						lRet = pImpl->CheckHomeDone(nAxis, df_MOVINGDONE_TIME);
					else
						lRet = lHome;
				}
			}
		}

		if (lRet == 1)
		{
			CLogWrapper::GetInstance()->KMLOG(OPER, str + _T("-END"));
		}
			
		else
			CLogWrapper::GetInstance()->KMLOG(ERRO, str + _T("-FAIL"));
	}
	else
	{
		CLogWrapper::GetInstance()->KMLOG(OPER, str + _T("-STOP"));
	}

	return 0;
}


UINT CDlgSubMaint::ThreadFunc2()
{
	if (m_iCurSel < 0)
		return 0;

	CString str;
	str.Format(_T("Do you want Moving [%d - %s Axis] ???"), m_iCurSel + 1, m_gridListMotionTest.GetItemText(m_iCurSel + 1, enumColumnName_Value1));

	if (AfxMessageBox(str, MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
		// MoveA
		double dPos = 0.0;
		CString strVal = m_gridListMotionTest.GetItemText(m_iCurSel + 1, enumColumnName_Value3);

		dPos = _ttof(strVal);

		CString strAxis = m_gridListMotionTest.GetItemText(m_iCurSel + 1, enumColumnName_Value8);
		int nAxis = 0;
		nAxis = _ttoi(strAxis);

		//if (nAxis == AXIS_Z7 || nAxis == AXIS_Z8)
		//{
		//	int iIndex = 0;
		//	if (nAxis == AXIS_Z7)
		//	{
		//		iIndex = 0;
		//	}
		//	else
		//	{
		//		iIndex = 1;
		//	}

		//	CMotorControlMgr *pImpl = nullptr;
		//	pImpl = m_pAuroraMgr->GetMotorControlMgrPtr();
		//	double dSWMax = 0, dSWMin = 0;
		//	dSWMax = pImpl->GetMaxSWLimit(nAxis);
		//	dSWMin = pImpl->GetMinSWLimit(nAxis);

		//	if (dSWMax > (dPos) && dSWMin < (dPos))
		//	{
		//		CSequenceManager::GetInstance()->GetFunctionModuleAFControl()->MovePosition(iIndex, dPos);
		//	}

		////	CSequenceManager::GetInstance()->GetFunctionModuleAFControl()->MovePosition(iIndex, dPos);
		//}
		//else
		{
			int nRet = df_MOTION_RV_SUCCESSED;
			CMaintControlMgr *pImpl = nullptr;
			pImpl = m_pAuroraMgr->GetMaintControlMgr();
			if (pImpl != nullptr)
			{
				nRet = pImpl->MoveCmdPosA(nAxis, dPos, false, SPEED_LOW, 0, 0);
				if (nRet == df_MOTION_RV_SUCCESSED)
				{
					nRet = pImpl->DoWork_MotionDone(nAxis);
				}
				else
				{
					;
			//		CAlarmController::GetInstance()->SetAlarm(nRet);
				}
			}
		}
	}

	return 0;
}


UINT CDlgSubMaint::ThreadFunc3()
{
	if (m_iCurSel < 0)
		return 0;

	// MoveR (-)
	double dPos = 0.0;
	CString strVal = m_gridListMotionTest.GetItemText(m_iCurSel + 1, enumColumnName_Value5);

	dPos = _ttof(strVal);

	CString strAxis = m_gridListMotionTest.GetItemText(m_iCurSel + 1, enumColumnName_Value8);
	int nAxis = 0;
	nAxis = _ttoi(strAxis);

	//if (nAxis == AXIS_Z7 || nAxis == AXIS_Z8)
	//{
	//	double dCurPos = 0;
	//	int iIndex = 0;
	//	if (nAxis == AXIS_Z7)
	//	{
	//		iIndex = 0;
	//	}
	//	else
	//	{
	//		iIndex = 1;
	//	}

	//	CMotorControlMgr *pImpl = nullptr;
	//	pImpl = m_pAuroraMgr->GetMotorControlMgrPtr();
	//	double dSWMax = 0, dSWMin = 0;
	//	dSWMax = pImpl->GetMaxSWLimit(nAxis);
	//	dSWMin = pImpl->GetMinSWLimit(nAxis);
	//	dCurPos = CSequenceManager::GetInstance()->GetFunctionModuleAFControl()->GetPos(iIndex);

	//	if (dSWMax > (dCurPos - dPos) && dSWMin < (dCurPos - dPos))
	//	{
	//		CSequenceManager::GetInstance()->GetFunctionModuleAFControl()->MovePosition(iIndex, dCurPos - dPos);
	//	}
	//}
	//else
	{
		CMaintControlMgr *pImpl = nullptr;
		pImpl = m_pAuroraMgr->GetMaintControlMgr();
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

UINT CDlgSubMaint::ThreadFunc4()
{
	if (m_iCurSel < 0)
		return 0;

	// MoveR (+)
	double dPos = 0.0;
	CString strVal = m_gridListMotionTest.GetItemText(m_iCurSel + 1, enumColumnName_Value5);

	dPos = _ttof(strVal);

	CString strAxis = m_gridListMotionTest.GetItemText(m_iCurSel + 1, enumColumnName_Value8);
	int nAxis = 0;
	nAxis = _ttoi(strAxis);

	//if (nAxis == AXIS_Z7 || nAxis == AXIS_Z8)
	//{
	//	double dCurPos = 0;
	//	int iIndex = 0;
	//	if (nAxis == AXIS_Z7)
	//	{
	//		iIndex = 0;
	//	}
	//	else
	//	{
	//		iIndex = 1;
	//	}

	//	CMotorControlMgr *pImpl = nullptr;
	//	pImpl = m_pAuroraMgr->GetMotorControlMgrPtr();
	//	double dSWMax = 0, dSWMin = 0;
	//	dSWMax = pImpl->GetMaxSWLimit(nAxis);
	//	dSWMin = pImpl->GetMinSWLimit(nAxis);
	//	dCurPos = CSequenceManager::GetInstance()->GetFunctionModuleAFControl()->GetPos(iIndex);

	//	if (dSWMax > (dCurPos + dPos) && dSWMin < (dCurPos + dPos))
	//	{
	//		CSequenceManager::GetInstance()->GetFunctionModuleAFControl()->MovePosition(iIndex, dCurPos + dPos);
	//	}
	//}
	//else
	{
		CMaintControlMgr *pImpl = nullptr;
		pImpl = m_pAuroraMgr->GetMaintControlMgr();
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
	}

	

	return 0;
}


UINT CDlgSubMaint::ThreadFuncDone(LPARAM lp)
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

//void CDlgSubMaint::__InitListControl3()
//{
//	m_gridListLightCtrl.GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
//	m_gridListLightCtrl.GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
//	m_gridListLightCtrl.GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
//	m_gridListLightCtrl.SetDefCellMargin(0);
//	m_gridListLightCtrl.SetFixedColumnSelection(FALSE);
//	m_gridListLightCtrl.SetFixedRowSelection(FALSE);
//	m_gridListLightCtrl.SetSingleColSelection(FALSE);
//	m_gridListLightCtrl.SetSingleRowSelection(FALSE);
//	m_gridListLightCtrl.SetTrackFocusCell(FALSE);
//	m_gridListLightCtrl.SetRowResize(FALSE);
//	m_gridListLightCtrl.SetColumnResize(FALSE);
//}
//
//
//void CDlgSubMaint::__InitialListData3()
//{
//	// 결과값 리스트를 갱신한다.
//	m_gridListLightCtrl.DeleteAllItems();
//	m_gridListLightCtrl.SetColumnCount(enumColumnName2_End);
//	m_gridListLightCtrl.SetFixedRowCount(1);
//
//	for (long n = 0; n < enumColumnName2_End; ++n)
//	{
//		m_gridListLightCtrl.SetColumnWidth(n, ColumnSize2[n]);
//		m_gridListLightCtrl.SetItemText(0, n, ColumnName2[n]);
//		m_gridListLightCtrl.SetItemState(0, n, GVIS_READONLY);
//		m_gridListLightCtrl.SetItemBkColour(0, n, RGB(172, 172, 172));
//		m_gridListLightCtrl.SetItemFgColour(0, n, RGB(255, 255, 255));
//	}
//
//	// Items + Header
//	m_gridListLightCtrl.SetEditable(TRUE);
//	m_gridListLightCtrl.SetRowCount(1 + _df_MAX_LIGHT);
//	m_gridListLightCtrl.SetRowHeight(0, 18);
//
//	__RefreshListData3();
//}
//
//
//void CDlgSubMaint::__RefreshListData3()
//{
//	m_gridListLightCtrl.SetRedraw(FALSE);
//
//	for (long i = 0; i < _df_MAX_LIGHT; i++)
//	{
//		m_gridListLightCtrl.SetRowHeight(i + 1, 20);
//
//		CString strNo;
//		strNo.Format(_T("[%d]"), i + 1);
//
//		m_gridListLightCtrl.GetCell(i + 1, enumColumnName2_No)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
//		m_gridListLightCtrl.SetItemState(i + 1, enumColumnName2_No, GVIS_READONLY);
//		m_gridListLightCtrl.SetItemText(i + 1, enumColumnName2_No, strNo);
//
//		// description
//		if (i < _df_MAX_LIGHT)
//		{
//			CString strBitImportName;
//			long nIntensity = 0;
//
//			m_pAuroraMgr->GetLightControlMgrPtr()->GetCurLedLampIntensity(i, nIntensity);
//		
//			strBitImportName.Format(_T("%d"), nIntensity);
//			m_gridListLightCtrl.SetItemText(i + 1, enumColumnName2_Value1, strBitImportName);
//			m_gridListLightCtrl.SetItemState(i + 1, enumColumnName2_Value1, GVIS_READONLY);
//
//			// button - -
//			m_gridListLightCtrl.SetCellType(i + 1, enumColumnName2_Value2, RUNTIME_CLASS(CGridCellButton));
//			m_gridListLightCtrl.SetItemState(i + 1, enumColumnName2_Value2, GVIS_READONLY);
//			m_gridListLightCtrl.SetItemText(i + 1, enumColumnName2_Value2, _T("-"));
//
//			// button - +
//			m_gridListLightCtrl.SetCellType(i + 1, enumColumnName2_Value3, RUNTIME_CLASS(CGridCellButton));
//			m_gridListLightCtrl.SetItemState(i + 1, enumColumnName2_Value3, GVIS_READONLY);
//			m_gridListLightCtrl.SetItemText(i + 1, enumColumnName2_Value3, _T("+"));
//		}
//	}
//
//	m_gridListLightCtrl.SetRedraw(TRUE);
//}

void CDlgSubMaint::OnBnClickedButtonMaintMotion()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CDlgSubMaint::OnBnClickedButtonMaintMotion"));

	m_iCurFileView = 1;
	RefreshFileView(TRUE);
}


void CDlgSubMaint::OnBnClickedButtonMaintIo()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CDlgSubMaint::OnBnClickedButtonMaintIo"));

	m_iCurFileView = 2;
	RefreshFileView(TRUE);
}


void CDlgSubMaint::OnBnClickedButtonMaintSerial()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CDlgSubMaint::OnBnClickedButtonMaintSerial"));

	m_iCurFileView = 3;
	RefreshFileView(TRUE);
}


void CDlgSubMaint::RefreshFileView(BOOL bShow)
{
	// TODO: 여기에 구현 코드 추가.
	m_pDlgSub_Motion->ShowWindow(SW_HIDE);
	m_pDlgSub_IO->ShowWindow(SW_HIDE);
	m_pDlgSub_Etc->ShowWindow(SW_HIDE);

	if (bShow == TRUE)
	{
		switch (m_iCurFileView)
		{
		case 0:
			//	m_pSubConfigFile->ShowWindow(SW_SHOW);
			break;
		case 1:
			m_pDlgSub_Motion->ShowWindow(SW_SHOW);
			break;
		case 2:
			m_pDlgSub_IO->ShowWindow(SW_SHOW);
			break;
		case 3:
			m_pDlgSub_Etc->ShowWindow(SW_SHOW);
			break;
		default:
			break;
		}
	}
}


void CDlgSubMaint::OnBnClickedButtonMaintEtc2()
{
	////20220119 JCY Progress DLG TEST
	CDlgSubMeasure_Progress* TEST = new CDlgSubMeasure_Progress();
	TEST->DoModal();
	//////////////////////////////////
}
