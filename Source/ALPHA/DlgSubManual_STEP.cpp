// DlgSubManual_STEP.cpp: 구현 파일
//

#include "pch.h"
#include "ALPHA.h"
#include "DlgSubManual_STEP.h"
#include "afxdialogex.h"

// CDlgSubManual_STEP 대화 상자

IMPLEMENT_DYNAMIC(CDlgSubManual_STEP, CDialogEx)

CDlgSubManual_STEP::CDlgSubManual_STEP(UINT idTemplate, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_SUB_MANUAL_STEP, pParent)
{
}

CDlgSubManual_STEP::~CDlgSubManual_STEP()
{
}

void CDlgSubManual_STEP::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_CONFIG_TITLE, m_LabelInfo);
}


BEGIN_MESSAGE_MAP(CDlgSubManual_STEP, CDialogEx)
END_MESSAGE_MAP()


// CDlgSubManual_STEP 메시지 처리기


BOOL CDlgSubManual_STEP::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	//---------------------------------------------------------------------
	// 타이틀에 색상을 입혀준다.
	//---------------------------------------------------------------------
	m_LabelInfo.SetBorder(TRUE, TRUE);
	m_LabelInfo.SetBkColor(RGB(0, 32, 0));
	m_LabelInfo.SetTextColor(RGB(255, 255, 192));
	m_LabelInfo.SetFontBold(TRUE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}
