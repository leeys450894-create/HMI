// D_SelectMeasOrUnload.cpp: 구현 파일
//

#include "pch.h"
#include "ALPHA.h"
#include "D_SelectMeasOrUnload.h"
#include "afxdialogex.h"


// CD_SelectMeasOrUnload 대화 상자

IMPLEMENT_DYNAMIC(CD_SelectMeasOrUnload, CDialogEx)

CD_SelectMeasOrUnload::CD_SelectMeasOrUnload(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_POPUP_SELECT_MEAS_OR_UNLOAD, pParent)
{
	m_nMode = 1;		//Default 
}

CD_SelectMeasOrUnload::~CD_SelectMeasOrUnload()
{
}

void CD_SelectMeasOrUnload::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_GLASS_EXIST_MESSAGE, m_LabelInfo);
}


BEGIN_MESSAGE_MAP(CD_SelectMeasOrUnload, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_GLASS_EXIST_SELECT_MEAS, &CD_SelectMeasOrUnload::OnBnClickedBtnGlassExistSelectMeas)
	ON_BN_CLICKED(IDC_BTN_GLASS_EXIST_SELECT_UNLOAD, &CD_SelectMeasOrUnload::OnBnClickedBtnGlassExistSelectUnload)
	ON_BN_CLICKED(IDC_BTN_GLASS_EXIST_SELECT_ABORT, &CD_SelectMeasOrUnload::OnBnClickedBtnGlassExistSelectAbort)
END_MESSAGE_MAP()


// CD_SelectMeasOrUnload 메시지 처리기


BOOL CD_SelectMeasOrUnload::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	//---------------------------------------------------------------------
	// 로그 타이틀에 색상을 입혀준다.
	//---------------------------------------------------------------------
	m_LabelInfo.SetBorder(TRUE, TRUE);
	m_LabelInfo.SetBkColor(RGB(0, 32, 0));
	m_LabelInfo.SetTextColor(RGB(255, 255, 192));
	m_LabelInfo.SetFontSize(20);
	m_LabelInfo.SetFontBold(TRUE);

	CenterWindow();
	CRect rClient;
	GetWindowRect(&rClient);
	SetWindowPos(&wndTopMost, rClient.left, rClient.top, 0, 0, SWP_NOSIZE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CD_SelectMeasOrUnload::OnBnClickedBtnGlassExistSelectMeas()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nMode = 1;
	CDialog::OnOK();
}


void CD_SelectMeasOrUnload::OnBnClickedBtnGlassExistSelectUnload()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nMode = 2;
	CDialog::OnOK();
}


void CD_SelectMeasOrUnload::OnBnClickedBtnGlassExistSelectAbort()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nMode = 3;
	CDialog::OnOK();
}
