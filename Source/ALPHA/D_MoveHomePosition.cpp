// D_MoveHomePosition.cpp : 구현 파일입니다.
//

#include "pch.h"
#include "ALPHA.h"
#include "D_MoveHomePosition.h"
#include "afxdialogex.h"

#include "Manager\SystemConfigMgr.h"
#include "Manager\IOMapControlMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CD_MoveHomePosition 대화 상자입니다.

IMPLEMENT_DYNAMIC(CD_MoveHomePosition, CDialogEx)

CD_MoveHomePosition::CD_MoveHomePosition(CSystemConfigMgr *pSystemConfigMgr, CIOMapControlMgr *pIOMapControlMgr, CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_POPUP_MOVE_HOME_POSITION, pParent)
	, m_pSystemConfigMgr(pSystemConfigMgr)
	, m_pIOMapControlMgr(pIOMapControlMgr)
{
}

CD_MoveHomePosition::~CD_MoveHomePosition()
{
}

void CD_MoveHomePosition::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_HOME_POSITION_MESSAGE,	m_LabelInfo);
}

BEGIN_MESSAGE_MAP(CD_MoveHomePosition, CDialogEx)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_HOME_POSITION_DONE, &CD_MoveHomePosition::OnBnClickedButtonHomePositionDone)
END_MESSAGE_MAP()


// CD_MoveHomePosition 메시지 처리기입니다.


BOOL CD_MoveHomePosition::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	//---------------------------------------------------------------------
	// 로그 타이틀에 색상을 입혀준다.
	//---------------------------------------------------------------------
	m_LabelInfo.SetBorder(TRUE, TRUE);
	m_LabelInfo.SetBkColor(RGB(0, 32, 0));
	m_LabelInfo.SetTextColor(RGB(255, 255, 0));	
	m_LabelInfo.SetFontSize(80);
	m_LabelInfo.SetFontBold(TRUE);
	m_LabelInfo.SetText(_T("\r\nIs it arrived home position?\r\n\r\nPlease, move to home position."));

	//---------------------------------------------------------------------
	// 로그 타이틀에 색상을 입혀준다.
	//---------------------------------------------------------------------
	SetTimer(POPUP_OPERATE_HOME_POSITION_TIMER_ID1,  1000, NULL);
	SetTimer(POPUP_OPERATE_HOME_POSITION_TIMER_ID2, 15000, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

BOOL CD_MoveHomePosition::PreTranslateMessage(MSG* pMsg)
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


void CD_MoveHomePosition::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if(nIDEvent == POPUP_OPERATE_HOME_POSITION_TIMER_ID1)
	{
		if(m_pSystemConfigMgr->IsUseHardwareMotor() == false)
		{
			m_LabelInfo.SetText(_T("\r\n\r\nSimulation mode!\r\n\r\nInitials are possible."));
			GetDlgItem(IDC_BUTTON_HOME_POSITION_DONE)->EnableWindow(TRUE);
		
			KillTimer(POPUP_OPERATE_HOME_POSITION_TIMER_ID1);
			KillTimer(POPUP_OPERATE_HOME_POSITION_TIMER_ID2);
		}
		else
		{
			m_LabelInfo.SetText(_T("\r\nIs it arrived home position?\r\n\r\nPlease, move to home position.\r\n\r\nIf you have not manually adjusted \r\nthe motor's position, wait."));
			GetDlgItem(IDC_BUTTON_HOME_POSITION_DONE)->EnableWindow(FALSE);
		}
	}

	if(nIDEvent == POPUP_OPERATE_HOME_POSITION_TIMER_ID2)
	{
		GetDlgItem(IDC_BUTTON_HOME_POSITION_DONE)->EnableWindow(TRUE);

		KillTimer(POPUP_OPERATE_HOME_POSITION_TIMER_ID1);
		KillTimer(POPUP_OPERATE_HOME_POSITION_TIMER_ID2);
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CD_MoveHomePosition::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	KillTimer(POPUP_OPERATE_HOME_POSITION_TIMER_ID1);
	KillTimer(POPUP_OPERATE_HOME_POSITION_TIMER_ID2);
}


void CD_MoveHomePosition::OnBnClickedButtonHomePositionDone()
{
    CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_MoveHomePosition::OnBnClickedButtonHomePositionDone"));

	OnOK();
}
