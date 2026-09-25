// D_SetLampTime.cpp : 구현 파일입니다.
//

#include "pch.h"
#include "ALPHA.h"
#include "D_SetLampTime.h"
#include "afxdialogex.h"
#include "common\Registry.h"
#include "AuroraMgr.h"

// CD_SetLampTime 대화 상자입니다.
#define STR_COMPANY_NAME			_T("K-MAC")
#define REG_SETTINGS		_T("Settings")
#define REG_LAMP_USED		_T("LampTime_Used")
#define	REG_LAMP_LIMIT		_T("LampTime_Limit")
#define	REG_LAMP_WARRING	_T("LampTime_Warring")
#define INT_LED_LAMP_LIMIT_MAX		30000
#define INT_LAMP_LIMIT_MAX			30000


#define df_MAX_LIGHT				2	// LYS 2026-09-25 4 -> 2 : ALIGN CCD1/2 = SPO ch 3/2 (DlgSubOperate 램프 타이머와 동일)
//#define df_MAX_LIGHT				4
static const int    s_nLampChannel[df_MAX_LIGHT] = { 3, 2 };	// 행 -> SPO 채널(레지스트리 키 번호)
static const TCHAR* s_szLampName[df_MAX_LIGHT]   = { _T("ALIGN CCD1 (CH4)"), _T("ALIGN CCD2 (CH3)") };

#define _df_MAX_ROW		df_MAX_LIGHT + 1


static CString ColumnName[] =
{
	_T("No"),
	_T("NAME"),
	_T("HOUR"),
	_T("MIN"),
	_T("SEC"),
	_T("WARRING HOUR"),
	_T("LIMIT HOUR"),
	_T("RESET"),

};

static long ColumnSize[] =
{
	40,
	120,
	40,
	40,
	40,
	120,
	120,
	120,
};
enum enumColumnName
{
	enumColumnName_NO = 0,
	enumColumnName_Value1,
	enumColumnName_Value2,
	enumColumnName_Value3,
	enumColumnName_Value4,
	enumColumnName_Value5,
	enumColumnName_Value6,
	enumColumnName_Value7,
	enumColumnName_End,
};

IMPLEMENT_DYNAMIC(CD_SetLampTime, CDialogEx)

CD_SetLampTime::CD_SetLampTime(CAuroraMgr *pAuroraMgr, CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_POPUP_LAMP_TIME, pParent)
	, m_pAuroraMgr(pAuroraMgr)
{

}

CD_SetLampTime::~CD_SetLampTime()
{
}

void CD_SetLampTime::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_TITLE_LAMP_TIME, m_LabelInfo);
	DDX_Control(pDX, IDC_GRID_LIST_LAMP_INFO, m_gridList);
}


BEGIN_MESSAGE_MAP(CD_SetLampTime, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_RESET, &CD_SetLampTime::OnBnClickedButtonReset)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, &CD_SetLampTime::OnBnClickedButtonApply)
	ON_NOTIFY(NM_CLICK, IDC_GRID_LIST_LAMP_INFO, &CD_SetLampTime::OnGridItemClickList)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CD_SetLampTime 메시지 처리기입니다.


void CD_SetLampTime::OnBnClickedButtonReset()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_gridList.SetRedraw(FALSE);

	int nSetRow = 0;
	CString str;

	for (long iLamp = 0; iLamp < df_MAX_LIGHT; iLamp++)
	{
		nSetRow = iLamp + 1;

		// HOUR
		str.Format(_T("%d"), 0);
		m_gridList.GetCell(nSetRow, enumColumnName_Value2)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridList.SetItemState(nSetRow, enumColumnName_Value2, GVIS_READONLY);
		m_gridList.SetItemText(nSetRow, enumColumnName_Value2, str);

		// MIN
		str.Format(_T("%d"), 0);
		m_gridList.GetCell(nSetRow, enumColumnName_Value3)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridList.SetItemState(nSetRow, enumColumnName_Value3, GVIS_READONLY);
		m_gridList.SetItemText(nSetRow, enumColumnName_Value3, str);

		// SEC
		str.Format(_T("%d"), 0);
		m_gridList.GetCell(nSetRow, enumColumnName_Value4)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridList.SetItemState(nSetRow, enumColumnName_Value4, GVIS_READONLY);
		m_gridList.SetItemText(nSetRow, enumColumnName_Value4, str);
	}

	m_gridList.SetRedraw(TRUE);
}


void CD_SetLampTime::OnBnClickedButtonApply()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString strHour, strMin, strSec, strUse, strWarring, strLimit;

	int nSetRow = 0;
	CString str;

	for (long iLamp = 0; iLamp < df_MAX_LIGHT; iLamp++)
	{
		nSetRow = iLamp + 1;

		
		// HOUR
		strHour = m_gridList.GetItemText(nSetRow, enumColumnName_Value2);

		// MIN
		strMin = m_gridList.GetItemText(nSetRow, enumColumnName_Value3);

		// SEC
		strSec = m_gridList.GetItemText(nSetRow, enumColumnName_Value4);

		// WARRING
		strWarring = m_gridList.GetItemText(nSetRow, enumColumnName_Value5);

		// LIMIT
		strLimit = m_gridList.GetItemText(nSetRow, enumColumnName_Value6);

		CTimeSpan spanUsed = CTimeSpan(0, _tstoi(strHour), _tstoi(strMin), _tstoi(strSec));
		LONGLONG Time = 0;
		Time = spanUsed.GetTotalSeconds();
		strUse.Format(_T("%s%02d"), REG_LAMP_USED, s_nLampChannel[iLamp]);
		m_pRegistry->WriteDWord(REG_SETTINGS, strUse, Time);

		Time = _tstoi(strWarring);
		strUse.Format(_T("%s%02d"), REG_LAMP_WARRING, s_nLampChannel[iLamp]);
		m_pRegistry->WriteDWord(REG_SETTINGS, strUse, Time);

		Time = _tstoi(strLimit);
		strUse.Format(_T("%s%02d"), REG_LAMP_LIMIT, s_nLampChannel[iLamp]);
		m_pRegistry->WriteDWord(REG_SETTINGS, strUse, Time);
	}
}


BOOL CD_SetLampTime::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	//---------------------------------------------------------------------
	// 타이틀에 색상을 입혀준다.
	//---------------------------------------------------------------------
	m_LabelInfo.SetBorder(TRUE, TRUE);
	m_LabelInfo.SetBkColor(RGB(0, 32, 0));
	m_LabelInfo.SetTextColor(RGB(255, 255, 192));
	m_LabelInfo.SetFontBold(TRUE);

	__InitListControl();
	__InitialListData();


	

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


BOOL CD_SetLampTime::PreTranslateMessage(MSG* pMsg)
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

void CD_SetLampTime::__InitListControl()
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


void CD_SetLampTime::__InitialListData()
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
	m_gridList.SetRowCount(_df_MAX_ROW);
	m_gridList.SetRowHeight(0, 22);

	for (long i = 1; i < _df_MAX_ROW; i++)
	{
		m_gridList.SetRowHeight(i, 22);
	}

	__RefreshListData();
}


void CD_SetLampTime::__RefreshListData()
{
	m_gridList.SetRedraw(FALSE);

	long nSetRow = 0;
	CString strAppName;
	strAppName.Format(_T("%s"), m_pAuroraMgr->GetSystemInfo().name);

	m_pRegistry = new CRegistry(STR_COMPANY_NAME, strAppName);

	DWORD dwUsed[df_MAX_LIGHT], dwWarring[df_MAX_LIGHT], dwLimit[df_MAX_LIGHT];

	CString strUse;
	CString str;

	for (long iLamp = 0; iLamp < df_MAX_LIGHT; iLamp++)
	{
		strUse.Format(_T("%s%02d"), REG_LAMP_USED, s_nLampChannel[iLamp]);
		m_pRegistry->ReadDWord(REG_SETTINGS, strUse, dwUsed[iLamp], 0);

		strUse.Format(_T("%s%02d"), REG_LAMP_WARRING, s_nLampChannel[iLamp]);
		m_pRegistry->ReadDWord(REG_SETTINGS, strUse, dwWarring[iLamp], INT_LED_LAMP_LIMIT_MAX);

		strUse.Format(_T("%s%02d"), REG_LAMP_LIMIT, s_nLampChannel[iLamp]);
		m_pRegistry->ReadDWord(REG_SETTINGS, strUse, dwLimit[iLamp], INT_LED_LAMP_LIMIT_MAX);

		CTimeSpan spanUsed = dwUsed[iLamp];

		nSetRow = iLamp + 1;

		for (long i = 0; i < enumColumnName_End; i++)
		{
			// No
			str.Format(_T("%d"), iLamp +1);
			m_gridList.GetCell(nSetRow, enumColumnName_NO)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_gridList.SetItemState(nSetRow, enumColumnName_NO, GVIS_READONLY);
			m_gridList.SetItemText(nSetRow, enumColumnName_NO, str);

			// NAME
			str.Format(_T("%s"), s_szLampName[iLamp]);	// LYS 2026-09-25
			//str.Format(_T("CH%d"), iLamp + 1);
			m_gridList.GetCell(nSetRow, enumColumnName_Value1)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_gridList.SetItemState(nSetRow, enumColumnName_Value1, GVIS_READONLY);
			m_gridList.SetItemText(nSetRow, enumColumnName_Value1, str);

			// HOUR
			str.Format(_T("%d"), spanUsed.GetTotalHours());
			m_gridList.GetCell(nSetRow, enumColumnName_Value2)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_gridList.SetItemState(nSetRow, enumColumnName_Value2, GVIS_READONLY);
			m_gridList.SetItemText(nSetRow, enumColumnName_Value2, str);

			// MIN
			str.Format(_T("%d"), spanUsed.GetMinutes());
			m_gridList.GetCell(nSetRow, enumColumnName_Value3)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_gridList.SetItemState(nSetRow, enumColumnName_Value3, GVIS_READONLY);
			m_gridList.SetItemText(nSetRow, enumColumnName_Value3, str);

			// SEC
			str.Format(_T("%d"), spanUsed.GetSeconds());
			m_gridList.GetCell(nSetRow, enumColumnName_Value4)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_gridList.SetItemState(nSetRow, enumColumnName_Value4, GVIS_READONLY);
			m_gridList.SetItemText(nSetRow, enumColumnName_Value4, str);

			// WARRING HOUR
			str.Format(_T("%d"), dwWarring[iLamp]);
			m_gridList.GetCell(nSetRow, enumColumnName_Value5)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_gridList.SetItemState(nSetRow, enumColumnName_Value5, GVIS_MODIFIED);
			m_gridList.SetItemText(nSetRow, enumColumnName_Value5, str);

			// LIMIT HOUR
			str.Format(_T("%d"), dwLimit[iLamp]);
			m_gridList.GetCell(nSetRow, enumColumnName_Value6)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_gridList.SetItemState(nSetRow, enumColumnName_Value6, GVIS_MODIFIED);
			m_gridList.SetItemText(nSetRow, enumColumnName_Value6, str);

			// RESET
			m_gridList.SetCellType(nSetRow, enumColumnName_Value7, RUNTIME_CLASS(CGridCellButton));
			m_gridList.SetItemState(nSetRow, enumColumnName_Value7, GVIS_READONLY);
			m_gridList.SetItemText(nSetRow, enumColumnName_Value7, _T("RESET"));

			for (long n = enumColumnName_NO; n < enumColumnName_Value6; ++n)
			{
				m_gridList.SetItemFgColour(nSetRow, n, RGB(0, 0, 0));
			}
		}
	}

	m_gridList.ExpandColumnsToFit(TRUE);
	m_gridList.SetRedraw(TRUE);
}

void CD_SetLampTime::OnGridItemClickList(NMHDR* pNotifyStruct, LRESULT* result)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*)pNotifyStruct;

	const long row = pNotify->iRow;
	const long col = pNotify->iColumn;

	if ((0 < row) && (enumColumnName_Value7 == col))	// ON
	{
		m_gridList.SetRedraw(FALSE);

		int nSetRow = 0;
		CString str;

		for (long iLamp = 0; iLamp < df_MAX_LIGHT; iLamp++)
		{
			nSetRow = iLamp + 1;
			if (nSetRow != row)
			{
				continue;
			}

			// HOUR
			str.Format(_T("%d"), 0);
			m_gridList.GetCell(nSetRow, enumColumnName_Value2)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_gridList.SetItemState(nSetRow, enumColumnName_Value2, GVIS_READONLY);
			m_gridList.SetItemText(nSetRow, enumColumnName_Value2, str);

			// MIN
			str.Format(_T("%d"), 0);
			m_gridList.GetCell(nSetRow, enumColumnName_Value3)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_gridList.SetItemState(nSetRow, enumColumnName_Value3, GVIS_READONLY);
			m_gridList.SetItemText(nSetRow, enumColumnName_Value3, str);

			// SEC
			str.Format(_T("%d"), 0);
			m_gridList.GetCell(nSetRow, enumColumnName_Value4)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_gridList.SetItemState(nSetRow, enumColumnName_Value4, GVIS_READONLY);
			m_gridList.SetItemText(nSetRow, enumColumnName_Value4, str);
		}

		m_gridList.SetRedraw(TRUE);
	}

	*result = 0;
}

void CD_SetLampTime::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	if (m_pRegistry) 
	{ 
		delete m_pRegistry;	
	
		m_pRegistry = NULL; 
	}
}
