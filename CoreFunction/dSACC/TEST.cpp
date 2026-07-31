// TEST.cpp: 구현 파일
//

#include "pch.h"
#include "dSACC.h"
#include "TEST.h"
#include "afxdialogex.h"


// TEST 대화 상자

IMPLEMENT_DYNAMIC(TEST, CDialog)

TEST::TEST(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DIALOG1, pParent)
{

}

TEST::~TEST()
{
}

void TEST::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(TEST, CDialog)
END_MESSAGE_MAP()


// TEST 메시지 처리기


BOOL TEST::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}
