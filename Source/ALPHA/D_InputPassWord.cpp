// D_InputPassWord.cpp : 구현 파일입니다.
//

#include "pch.h"
#include "ALPHA.h"
#include "D_InputPassWord.h"
#include "afxdialogex.h"

#include "Manager\LogInControlMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CD_InputPassWord 대화 상자입니다.

IMPLEMENT_DYNAMIC(CD_InputPassWord, CDialogEx)

CD_InputPassWord::CD_InputPassWord(CLogInControlMgr *pLogInControlMgr, CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_POPUP_INPUT_PASSWORD, pParent)
	, m_pLogInControlMgr(pLogInControlMgr)
{
    //----------------------------------------------------------
    // FONT 만들기
    //----------------------------------------------------------
    LOGFONT lf;
    memset(&lf, 0, sizeof(LOGFONT));
    lf.lfHeight = 15;
    _tcscpy_s(lf.lfFaceName, _countof(lf.lfFaceName), _T("Arial"));
    VERIFY(m_logFont.CreateFontIndirect(&lf));
}

CD_InputPassWord::~CD_InputPassWord()
{
    m_logFont.DeleteObject();
}

void CD_InputPassWord::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CD_InputPassWord, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_INPUTTEXT, &CD_InputPassWord::OnBnClickedButtonInputtext)
END_MESSAGE_MAP()


// CD_InputPassWord 메시지 처리기입니다.

BOOL CD_InputPassWord::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	GetDlgItem(IDC_EDIT_INPUTTEXT)->SetFont(&m_logFont);


	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CD_InputPassWord::OnBnClickedButtonInputtext()
{
    CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_InputPassWord::OnBnClickedButtonInputtext"));

	CString strText;
	GetDlgItem(IDC_EDIT_INPUTTEXT)->GetWindowText(strText);

	if(strText.IsEmpty())
	{
		AfxMessageBox(_T("Is Empty. Please type in password!"));
		return;
	}

	if(m_pLogInControlMgr)
	{
		if(strText == m_pLogInControlMgr->GetPMmodePW())
		{
			OnOK();
			return;
		}
	}

	OnCancel();
}

BOOL CD_InputPassWord::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		// To prevent to close a dialog.
		switch (pMsg->wParam)
		{
            case VK_ESCAPE:
            case VK_RETURN:
            case VK_SPACE:
			{
				OnBnClickedButtonInputtext();
				return TRUE;
			}
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}