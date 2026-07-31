// CD_SelectColor.cpp: 구현 파일
//

#include "pch.h"
#include "ALPHA.h"
#include "CD_SelectColor.h"
#include "afxdialogex.h"


// CD_SelectColor 대화 상자
#define BACK_COLOR_OP_MODE		0
#define BACK_COLOR_ENG_MODE		1
#define BACK_COLOR_MAINT_MODE	2
#define BACK_COLOR_SUPER_MODE	3

IMPLEMENT_DYNAMIC(CD_SelectColor, CDialogEx)

CD_SelectColor::CD_SelectColor(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_POPUP_SELECT_COLOR, pParent)
{
	COLORREF clrDefault = ::GetSysColor(COLOR_BTNFACE);

	m_colorWAD    = clrDefault;
	m_colorVision = clrDefault;
	m_colorAngle  = clrDefault;
	m_colorColor  = clrDefault;
}

CD_SelectColor::~CD_SelectColor()
{
}

void CD_SelectColor::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CD_SelectColor, CDialogEx)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BTN_SELECT_COLOR_WAD, &CD_SelectColor::OnBnClickedBtnSelectColorWad)
	ON_BN_CLICKED(IDC_BTN_SELECT_COLOR_VISION, &CD_SelectColor::OnBnClickedBtnSelectColorVision)
	ON_BN_CLICKED(IDC_BTN_SELECT_COLOR_ANGLE, &CD_SelectColor::OnBnClickedBtnSelectColorAngle)
	ON_BN_CLICKED(IDC_BTN_SELECT_COLOR_COLOR, &CD_SelectColor::OnBnClickedBtnSelectColorColor)
END_MESSAGE_MAP()


// CD_SelectColor 메시지 처리기

void CD_SelectColor::FillColor(int iMode)
{
	CRect rect;
	CWnd* pWnd = nullptr;
	COLORREF color = m_colorWAD;

	switch (iMode)
	{
	case 0:
		pWnd = GetDlgItem(IDC_STATIC_COLOR_WAD);
		color = m_colorWAD;
		break;

	case 1:
		pWnd = GetDlgItem(IDC_STATIC_COLOR_VISION);
		color = m_colorVision;
		break;

	case 2:
		pWnd = GetDlgItem(IDC_STATIC_COLOR_ANGLE);
		color = m_colorAngle;
		break;

	case 3:
		pWnd = GetDlgItem(IDC_STATIC_COLOR_COLOR);
		color = m_colorColor;
		break;
	}

	if (pWnd != nullptr)
	{
		pWnd->GetClientRect(&rect);

		CClientDC dc(pWnd);
		dc.FillSolidRect(&rect, color);
	}
}

//void CD_SelectColor::PreInitDialog()
//{
//	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
//
//	CDialogEx::PreInitDialog();
//}


void CD_SelectColor::OnPaint()
{
	CPaintDC dc(this); // device context for painting
					   // TODO: 여기에 메시지 처리기 코드를 추가합니다.
					   // 그리기 메시지에 대해서는 CDialogEx::OnPaint()을(를) 호출하지 마십시오.
	FillColor(BACK_COLOR_OP_MODE);
	FillColor(BACK_COLOR_ENG_MODE);
	FillColor(BACK_COLOR_MAINT_MODE);
	FillColor(BACK_COLOR_SUPER_MODE);
}


BOOL CD_SelectColor::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CD_SelectColor::OnBnClickedBtnSelectColorWad()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CColorDialog colorDlg;
	if (colorDlg.DoModal() == IDOK)
	{
		m_colorWAD = colorDlg.GetColor();
		FillColor(BACK_COLOR_OP_MODE);
	}
}


void CD_SelectColor::OnBnClickedBtnSelectColorVision()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CColorDialog colorDlg;
	if (colorDlg.DoModal() == IDOK)
	{
		m_colorVision = colorDlg.GetColor();
		FillColor(BACK_COLOR_ENG_MODE);
	}
}


void CD_SelectColor::OnBnClickedBtnSelectColorAngle()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CColorDialog colorDlg;
	if (colorDlg.DoModal() == IDOK)
	{
		m_colorAngle = colorDlg.GetColor();
		FillColor(BACK_COLOR_MAINT_MODE);
	}
}


void CD_SelectColor::OnBnClickedBtnSelectColorColor()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CColorDialog colorDlg;
	if (colorDlg.DoModal() == IDOK)
	{
		m_colorColor = colorDlg.GetColor();
		FillColor(BACK_COLOR_SUPER_MODE);
	}
}
