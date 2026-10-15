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


// LYS 2026-10-14 multi-line popups: CLabel draws an SS_CENTERIMAGE text without "\r\n" as DT_SINGLELINE (Label.cpp:351-363), so the "\n" lines of
//   the new refusal popups (hybrid / manual Meas / TestMode) ran into one line and were cut at the label edge. A message with a line break:
//   CRLF, SS_CENTERIMAGE off (CLabel then draws SS_CENTER | DT_WORDBREAK: centred lines, long ones wrapped at spaces), one empty line above and
//   below as the margin, and the label / Close button / dialog follow the wrapped height (<= 85% of the work area, then the font shrinks).
//   Single-line messages keep the original 600 x 95 DLU layout (18 pt, vertically centred)
static void FitMultiLineMessage(CDialogEx* pDlg, CLabel& label, const CString& strMessage)
{
	CString strShow = strMessage;
	strShow.Replace(_T("\r\n"), _T("\n"));
	strShow.Replace(_T("\n"), _T("\r\n"));
	strShow = _T("\r\n") + strShow + _T("\r\n");	// margin lines: a multi-line CLabel draws from the top
	label.ModifyStyle(SS_CENTERIMAGE, 0);

	CWnd* pBtn = pDlg->GetDlgItem(IDC_BUTTON_INSP_COMPLETE);
	CRect rcLabel, rcLabelClient, rcBtn, rcDlg, rcClient, rcWork;
	label.GetWindowRect(&rcLabel);
	pDlg->ScreenToClient(&rcLabel);
	label.GetClientRect(&rcLabelClient);
	pDlg->GetWindowRect(&rcDlg);
	pDlg->GetClientRect(&rcClient);
	if (pBtn != NULL)
	{
		pBtn->GetWindowRect(&rcBtn);
		pDlg->ScreenToClient(&rcBtn);
	}
	if (::SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0) == FALSE)
		rcWork.SetRect(0, 0, ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN));
	int nFrameH = rcLabel.Height() - rcLabelClient.Height();	// label border
	int nOtherH = rcDlg.Height() - rcLabel.Height();			// caption, frame, Close button
	int nMaxTextH = rcWork.Height() * 85 / 100 - nOtherH - nFrameH;

	LOGFONT lfBase;										// = the CLabel font: the dialog font (PreSubclassWindow) + SetFontBold + SetFontSize
	CFont* pBaseFont = label.GetFont();
	if (pBaseFont != NULL)
		pBaseFont->GetLogFont(&lfBase);
	else
		::GetObject(::GetStockObject(SYSTEM_FONT), sizeof(lfBase), &lfBase);
	lfBase.lfWeight = FW_BOLD;
	int nSize = 18, nTextH = 0;
	for (; nSize >= 10; nSize--)
	{
		CFont fontPt;
		LOGFONT lfPt, lf = lfBase;
		fontPt.CreatePointFont(nSize * 10, lfBase.lfFaceName);	// same height rule as CLabel::SetFontSize
		fontPt.GetLogFont(&lfPt);
		lf.lfHeight = lfPt.lfHeight;
		lf.lfWidth = lfPt.lfWidth;
		CFont fontMeasure;
		fontMeasure.CreateFontIndirect(&lf);
		CClientDC dc(&label);
		CFont* pOldFont = dc.SelectObject(&fontMeasure);
		CRect rcCalc(0, 0, rcLabelClient.Width(), 0);
		dc.DrawText(strShow, &rcCalc, DT_CALCRECT | DT_CENTER | DT_WORDBREAK);	// = CLabel SS_CENTER | DT_WORDBREAK
		dc.SelectObject(pOldFont);
		nTextH = rcCalc.Height() + 4;
		if (nTextH <= nMaxTextH)
			break;
	}
	if (nSize < 10)
		nSize = 10;
	if (nSize != 18)
		label.SetFontSize(nSize);
	if (nTextH > nMaxTextH)
		nTextH = nMaxTextH;

	int nDelta = (nTextH + nFrameH) - rcLabel.Height();
	label.SetWindowPos(NULL, 0, 0, rcLabel.Width(), rcLabel.Height() + nDelta, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	if (pBtn != NULL)
		pBtn->SetWindowPos(NULL, rcBtn.left, rcBtn.top + nDelta, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	pDlg->SetWindowPos(NULL, 0, 0, rcDlg.Width(), rcDlg.Height() + nDelta, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	pDlg->CenterWindow();
	label.SetText(strShow);
}

BOOL CD_DisplayMessage::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	m_labelInfo.SetBorder(TRUE, TRUE);
	m_labelInfo.SetBkColor(RGB(0, 32, 0));
	m_labelInfo.SetTextColor(RGB(192, 255, 255));
	m_labelInfo.SetFontBold(TRUE);
	m_labelInfo.SetFontSize(18);
	if (m_strMessage.Find(_T('\n')) >= 0)
		FitMultiLineMessage(this, m_labelInfo, m_strMessage);	// LYS 2026-10-14 multi-line popups (see above)
	else
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
