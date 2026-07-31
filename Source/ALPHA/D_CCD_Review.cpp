// D_CCD_Review.cpp: 구현 파일
//

#include "pch.h"
#include "ALPHA.h"
#include "D_CCD_Review.h"
#include "afxdialogex.h"


// CD_CCD_Review 대화 상자

IMPLEMENT_DYNAMIC(CD_CCD_Review, CDialogEx)

CD_CCD_Review::CD_CCD_Review(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_SUB_CCD_FULL, pParent)
	,m_iMoniImageWnd(0)
{

}

CD_CCD_Review::~CD_CCD_Review()
{
}

void CD_CCD_Review::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CD_CCD_Review, CDialogEx)
END_MESSAGE_MAP()


// CD_CCD_Review 메시지 처리기


BOOL CD_CCD_Review::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	SetBackgroundColor(RGB(128, 128, 128));

	imaqGetWindowHandle(&m_iMoniImageWnd);
	m_hMoniImage = (HWND)imaqGetSystemWindowHandle(m_iMoniImageWnd);

	// To make child window
	::SetParent(m_hMoniImage, this->GetSafeHwnd());

	Point pt = { 0, 0 };
	imaqMoveWindow(m_iMoniImageWnd, pt);


	// Window attribute could not be set by imaqSetupWindow as what I want..
	// So, I used SetWindowLong for making child window.
	// But, this scheme support displaying image, but not support ROI tools...
	// So, no way except using extra top-level window for selecting pattern, etc.
	::SetWindowLong(m_hMoniImage, GWL_STYLE, WS_CHILDWINDOW | WS_VISIBLE | WS_OVERLAPPED);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


int CD_CCD_Review::DisplayImage(Image* pImage)
{
	// TODO: 여기에 구현 코드 추가.
	imaqDisplayImage(pImage, m_iMoniImageWnd, TRUE);

	return 0;
}


int CD_CCD_Review::SetWindowSize(int iGrabSizeW, int iGrabSizeH)
{
	// TODO: 여기에 구현 코드 추가.
	CRect rect;
	this->GetWindowRect(rect);
	imaqSetWindowSize(m_iMoniImageWnd, iGrabSizeW, iGrabSizeH);
	
	double dScaleX = 0, dScaleY = 0, dScale = 0;
	dScaleX = (double)rect.Width()/*320*/ / iGrabSizeW;
	dScaleY = (double)rect.Height()/*266*/ / iGrabSizeH;
	dScale = (dScaleX > dScaleY ? dScaleY : dScaleX);
	imaqZoomWindow2(m_iMoniImageWnd, (float)dScale, (float)dScale, IMAQ_NO_POINT);

	return 0;
}
