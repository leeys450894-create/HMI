// CD_PopUp_Alarm_F.cpp: 구현 파일
//

#include "pch.h"
#include "ALPHA.h"
#include "CD_PopUp_Alarm_F.h"
#include "afxdialogex.h"
#include "Manager/MaintControlMgr.h"
#include "IOMapFunction\DataStructure.h"
#include "AuroraMgrPool.h"


// CD_PopUp_Alarm_F 대화 상자

static CString ColumnName[] =
{
	_T("No"),
	_T("Description"),
};

static long ColumnSize[] =
{
	50,
	226,
};

enum enumColumnName
{
	enumColumnName_No = 0,
	enumColumnName_Des,
	enumColumnName_End,
};

IMPLEMENT_DYNAMIC(CD_PopUp_Alarm_F, CDialogEx)

CD_PopUp_Alarm_F::CD_PopUp_Alarm_F(CMaintControlMgr* pMaintControlMgr, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PUPUP_ALARM_F, pParent)
	, m_pMaintControlMgr(pMaintControlMgr)
{

}

CD_PopUp_Alarm_F::~CD_PopUp_Alarm_F()
{
}

void CD_PopUp_Alarm_F::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_MAINT_TITLE, m_LabelInfo);
	DDX_Control(pDX, IDC_GRID_LIST_POPUP_ALARM, m_gridList);
}


BEGIN_MESSAGE_MAP(CD_PopUp_Alarm_F, CDialogEx)
	ON_WM_DESTROY()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CD_PopUp_Alarm_F 메시지 처리기


BOOL CD_PopUp_Alarm_F::OnInitDialog()
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

	SetTimer(1000, 1000, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CD_PopUp_Alarm_F::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	KillTimer(1000);
}


void CD_PopUp_Alarm_F::__InitListControl()
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

void CD_PopUp_Alarm_F::__InitialListData()
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

	// 각 Spread별로 Row 개수를 정한다
	int nStart = 0, nEnd = 0;
	CIOMapControlMgr* pImpl = nullptr;
	pImpl = m_pMaintControlMgr->GetIOMapControlMgrPtr();

	nStart = dfFADDRESS_RD_START, nEnd = dfFADDRESS_TOTAL_COUNT;

	int iMaxIO = 0;
	if (pImpl != nullptr)
	{
		CString strData;
		for (nStart; nStart < nEnd; nStart++)
		{
			strData.Format(_T("%s"), pImpl->GetParam_DIOMapAddressBitImport(nStart));
			strData.TrimRight();
			if (strData.IsEmpty())
				continue;

			iMaxIO++;
		}
	}

	// Items + Header
	m_gridList.SetEditable(TRUE);
	m_gridList.SetRowCount(iMaxIO + 1);	// Axis type end 

	m_gridList.SetRowHeight(0, 18);
	for (long i = 0; i < iMaxIO; i++)
	{
		m_gridList.SetRowHeight(i + 1, 18);
	}

	__RefreshListData();
}

void CD_PopUp_Alarm_F::__RefreshListData()
{
	m_gridList.SetRedraw(FALSE);

	bool bHomeDone = false, bChkPos = false, bChkNeg = false;
	int i = 0, nStart = 0, nEnd = 0;
	CString strData;
	CIOMapControlMgr* pImpl = nullptr;
	pImpl = m_pMaintControlMgr->GetIOMapControlMgrPtr();

	nStart = dfFADDRESS_RD_START, nEnd = dfFADDRESS_TOTAL_COUNT;

	if (pImpl != nullptr)
	{
		for (nStart; nStart < nEnd; nStart++)
		{
			// Number 
			strData.Format(_T("%s"), pImpl->GetParam_DIOMapAddressBitImport(nStart));
			strData.TrimRight();
			if (strData.IsEmpty())
				continue;

			m_gridList.GetCell(i + 1, enumColumnName_No)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_gridList.SetItemState(i + 1, enumColumnName_No, GVIS_READONLY);
			m_gridList.SetItemText(i + 1, enumColumnName_No, strData);

			// Name
			strData.Format(_T("%s"), pImpl->GetParam_DIOMapAddressBitImportName(nStart));
			m_gridList.GetCell(i + 1, enumColumnName_Des)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_gridList.SetItemState(i + 1, enumColumnName_Des, GVIS_READONLY);
			m_gridList.SetItemText(i + 1, enumColumnName_Des, strData);

			// 활성화 되어 있는 상태를 표시해준다.
			bool bRet = false;
			bRet = pImpl->GetParam_DIOMapMemoryBitImport2(nStart);
			int nHeight = 0;
			if (bRet == true)
			{
				for (long n = enumColumnName_Des; n < enumColumnName_End; ++n)
				{
					m_gridList.SetItemBkColour(i + 1, n, CLR_LIGHT_YELLOW);
				}
				
				nHeight = 18;
			}
			else
			{
				for (long n = enumColumnName_Des; n < enumColumnName_End; ++n)
				{
					m_gridList.SetItemBkColour(i + 1, n, CLR_GRAY);
				}

				nHeight = 0;
			}

			m_gridList.SetRowHeight(i + 1, nHeight);

			i++;
		}
	}

	m_gridList.ExpandColumnsToFit(TRUE);

	m_gridList.SetRedraw(TRUE, TRUE);
}

void CD_PopUp_Alarm_F::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	CDialogEx::OnTimer(nIDEvent);

	if (nIDEvent = 1000)
	{
		if (IsWindowVisible())
		{
			__RefreshListData();
		}
	}
}
