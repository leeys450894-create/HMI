// CD_DisplayMessage.cpp : 구현 파일입니다.
//

#include "pch.h"
#include "ALPHA.h"
#include "D_DisplayMessage.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CD_DisplayMessage 대화 상자입니다.

IMPLEMENT_DYNAMIC(CD_DisplayMessage, CDialogEx)

CD_DisplayMessage::CD_DisplayMessage(CString strMessage, CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_POPUP_DIPLAY_MESSAGE, pParent)
	, m_strMessage(strMessage)
{

}

CD_DisplayMessage::~CD_DisplayMessage()
{
}

void CD_DisplayMessage::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_MESSAGE, m_labelInfo);
}


BEGIN_MESSAGE_MAP(CD_DisplayMessage, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_INSP_COMPLETE, &CD_DisplayMessage::OnBnClickedButtonInspComplete)
END_MESSAGE_MAP()


// CD_DisplayMessage 메시지 처리기입니다.


BOOL CD_DisplayMessage::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	m_labelInfo.SetBorder(TRUE, TRUE);
	m_labelInfo.SetBkColor(RGB(0, 32, 0));
	m_labelInfo.SetTextColor(RGB(192, 255, 255));
	m_labelInfo.SetFontBold(TRUE);
	m_labelInfo.SetFontSize(18);
	m_labelInfo.SetText(m_strMessage);
	
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

BOOL CD_DisplayMessage::PreTranslateMessage(MSG *pMsg)
{
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


void CD_DisplayMessage::OnBnClickedButtonInspComplete()
{
    CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_DisplayMessage::OnBnClickedButtonInspComplete"));

	OnOK();
}
