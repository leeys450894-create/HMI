// CDlgSubSACC_Progress.cpp: 구현 파일
//

#include "pch.h"
#include "ALPHA.h"
#include "DlgSubMeasure_Progress.h"
#include "afxdialogex.h"

#include "Manager/SequenceManager.h"
#include "FunctionModule/FunctionModuleProbeACC.h"
#include "FunctionModule/FunctionModuleEQState.h"

// CDlgSubSACC_Progress 대화 상자

IMPLEMENT_DYNAMIC(CDlgSubMeasure_Progress, CDialogEx)

CDlgSubMeasure_Progress::CDlgSubMeasure_Progress(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_SUB_SACC_PROGRESS, pParent)
{

}

CDlgSubMeasure_Progress::~CDlgSubMeasure_Progress()
{
}

void CDlgSubMeasure_Progress::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_PROGRESS_TITLE, m_LabelInfo);
	DDX_Control(pDX, IDC_STATIC_PROGRESS_SUCCESS, m_LabelSuccess);
}


BEGIN_MESSAGE_MAP(CDlgSubMeasure_Progress, CDialogEx)
END_MESSAGE_MAP()


// CDlgSubSACC_Progress 메시지 처리기


BOOL CDlgSubMeasure_Progress::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//---------------------------------------------------------------------
	// 타이틀에 색상을 입혀준다.
	//---------------------------------------------------------------------
	m_LabelInfo.SetBorder(TRUE, TRUE);
	m_LabelInfo.SetBkColor(RGB(0, 32, 0));
	m_LabelInfo.SetTextColor(RGB(255, 255, 192));
	m_LabelInfo.SetFontBold(TRUE);
	m_LabelInfo.SetFontSize(15);

	m_LabelSuccess.SetBorder(TRUE, TRUE);
	m_LabelSuccess.SetBkColor(RGB(0, 32, 0));
	m_LabelSuccess.SetTextColor(RGB(32, 255, 32));
	m_LabelSuccess.SetFontBold(TRUE);
	m_LabelSuccess.SetFontSize(15);

	//---------------------------------------------------------------------
	// Farpoint Spread 생성 
	//---------------------------------------------------------------------
	m_spreadProgress = new CMySpreadDll(IDC_FS_MEASURE_PROGRESS, FPS_RESOURCE_ID + 23, this, TRUE);

	if (m_spreadProgress->GetSafeHwnd() != NULL)
	{
		m_spreadProgress->InitDefault_Progress();

		SS_CELLTYPE cellType;

		m_spreadProgress->SetTypeEdit(&cellType, SSS_ALIGN_VCENTER | SSS_ALIGN_CENTER, 20, NULL, SS_CASE_NOCASE);
		m_spreadProgress->SetCellType(SS_ALLCOLS, SS_ALLROWS, &cellType);

		m_spreadProgress->SetBool(SSB_AUTOCLIPBOARD, TRUE);
		m_spreadProgress->SetClipboardOptions(SS_CLIP_NOHEADERS);

		m_spreadProgress->SetColHeaderRows(0);

		int iCell = 10;
		m_spreadProgress->InitMySpread_MEAS_PROGRESS(iCell); // Glass Cell에 따른 Col 개수 가변 필요
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

int CDlgSubMeasure_Progress::GetProbeID()
{
	return 0;
}

int CDlgSubMeasure_Progress::GetGlassID()
{
	return 0;
}

int CDlgSubMeasure_Progress::GetAgingResult()
{
	return 0;
}

void CDlgSubMeasure_Progress::InitStart(int iCell)
{
	m_spreadProgress->InitMySpread_MEAS_PROGRESS(iCell); // Glass Cell에 따른 Col 개수 가변 필요

	ShowWindow(SW_SHOW);
}

void CDlgSubMeasure_Progress::InitStop()
{
	ShowWindow(SW_HIDE);
}

BOOL CDlgSubMeasure_Progress::PreTranslateMessage(MSG* pMsg)
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
		{
			return TRUE;
		}
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CDlgSubMeasure_Progress::UpdateAgingCell()
{
	//for (int iRow = 1; iRow <= m_spreadProgress->GetMaxRows() / 3; iRow++)
	//{
	//	//---------------------------------------------------------------------
	//	//Spread CELL Color 지정 
	//	//---------------------------------------------------------------------
	//	SetColorRange(1, iCurRow, 1, iCurRow + 2, RGB_LIGHT_GRAY, RGB_DARK_PINK);		  //Probe 부분 
	//	SetColorRange(2, iCurRow, GetMaxCols(), iCurRow + 2, RGB_BLACK, RGB(210, 210, 0));  //CELL  부분 					
	//	SetBorderRange(1, iCurRow + 2, GetMaxCols(), iCurRow + 2, SS_BORDERTYPE_BOTTOM, SS_BORDERSTYLE_SOLID, RGB_GRAY_120); //셀 사이 라인 색(BOTTOM)

	//	//---------------------------------------------------------------------
	//	//PROBE Number 
	//	//---------------------------------------------------------------------
	//	strTemp.Format(_T("PROBE %d"), iRow);
	//	SetData(1, iCurRow, strTemp);

	//	//---------------------------------------------------------------------
	//	//CELL Number 
	//	//---------------------------------------------------------------------
	//	for (int iCol = 2; iCol <= GetMaxCols(); iCol++)
	//	{
	//		strTemp.Format(_T("%d"), iCol - 1);
	//		SetData(iCol, iCurRow, strTemp);
	//	}
	//	iCurRow += 3
}
