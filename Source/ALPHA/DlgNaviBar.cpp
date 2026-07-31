// DlgNaviBar.cpp : 구현 파일입니다.
//

#include "pch.h"
#include "ALPHA.h"
#include "DlgNaviBar.h"
#include "afxdialogex.h"

#include "SystemParamShare.h"
//#include "AlarmMonitorMgr.h"
//#include "AlarmController.h"
#include "AuroraMgr.h"
#include "MainSubDlgWin.h"

#include "Manager\IOMapControlMgr.h"
#include "Manager\LogInControlMgr.h"

#include "D_LogInMgr.h"
#include "D_PM_MODE.h"

// CDlgNaviBar 대화 상자입니다.

IMPLEMENT_DYNAMIC(CDlgNaviBar, CMainSubDlgWin)

CDlgNaviBar::CDlgNaviBar(CAuroraMgr *pAuroraMgr, CWnd* pParent /*=NULL*/)
	: CMainSubDlgWin(IDD_DLG_MAIN_NAVI, pParent)
	, m_pAuroraMgr(pAuroraMgr)
	, m_iCurrentView(1)
{

}

CDlgNaviBar::~CDlgNaviBar()
{
}

void CDlgNaviBar::DoDataExchange(CDataExchange* pDX)
{
	CMainSubDlgWin::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_LOGIN, m_btnImage[enumNavigateID_LOGIN]);
	DDX_Control(pDX, IDC_CHECK_OPERATE, m_btnImage[enumNavigateID_OPERATE]);
	DDX_Control(pDX, IDC_CHECK_RESULT, m_btnImage[enumNavigateID_RESULT_DATA]);
	DDX_Control(pDX, IDC_CHECK_MAINT, m_btnImage[enumNavigateID_MAINT]);
	DDX_Control(pDX, IDC_CHECK_MANUAL, m_btnImage[enumNavigateID_MANUAL]);
	DDX_Control(pDX, IDC_CHECK_RECIPE, m_btnImage[enumNavigateID_RECIPE]);
	DDX_Control(pDX, IDC_CHECK_LOG, m_btnImage[enumNavigateID_SYSTEM_LOG]);
	DDX_Control(pDX, IDC_CHECK_CONFIG, m_btnImage[enumNavigateID_CONFIG]);
	DDX_Control(pDX, IDC_CHECK_ALARM, m_btnImage[enumNavigateID_ALARM]);
	DDX_Control(pDX, IDC_BUTTON_BUZZER_OFF, m_btnImage[enumNavigateID_BUZZER_OFF]);
	DDX_Control(pDX, IDC_BUTTON_PM_MODE, m_btnImage[enumNavigateID_PM_MODE]);
}


BEGIN_MESSAGE_MAP(CDlgNaviBar, CMainSubDlgWin)
	ON_BN_CLICKED(IDC_BUTTON_LOGIN, &CDlgNaviBar::OnBnClickedButtonLogin)
	ON_BN_CLICKED(IDC_CHECK_OPERATE, &CDlgNaviBar::OnBnClickedButtonOperate)
	ON_BN_CLICKED(IDC_CHECK_RESULT, &CDlgNaviBar::OnBnClickedButtonResult)
	ON_BN_CLICKED(IDC_CHECK_MAINT, &CDlgNaviBar::OnBnClickedButtonMaint)
	ON_BN_CLICKED(IDC_CHECK_RECIPE, &CDlgNaviBar::OnBnClickedButtonRecipe)
	ON_BN_CLICKED(IDC_CHECK_LOG, &CDlgNaviBar::OnBnClickedButtonLog)
	ON_BN_CLICKED(IDC_CHECK_CONFIG, &CDlgNaviBar::OnBnClickedButtonConfig)
	ON_BN_CLICKED(IDC_CHECK_ALARM, &CDlgNaviBar::OnBnClickedButtonAlarm)
	ON_BN_CLICKED(IDC_BUTTON_BUZZER_OFF, &CDlgNaviBar::OnBnClickedButtonBuzzerOff)
	ON_BN_CLICKED(IDC_BUTTON_PM_MODE, &CDlgNaviBar::OnBnClickedButtonPmMode)
	ON_BN_CLICKED(IDC_CHECK_MANUAL, &CDlgNaviBar::OnBnClickedCheckManual)
END_MESSAGE_MAP()


// CDlgNaviBar 메시지 처리기입니다.


void CDlgNaviBar::OnBnClickedButtonLogin()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_MainNavigate::OnBnClickedButtonLogin"));

	CD_LogInMgr dlg(m_pAuroraMgr->GetLogInControlMgrPtr(), this);
	if (dlg.DoModal() == IDOK)
	{
		if (dlg.IsLoginStatus() == true)
		{
			UpdateInitialState(TRUE);
		}
		else
		{
			UpdateInitialState(FALSE);

			m_iCurrentView = enumNavigateID_OPERATE;
			GetParent()->SendMessage(WM_MSG_UPDATE_CHANGE_VIEW_STATE, (WPARAM)m_iCurrentView, NULL);
		}
	}
}


void CDlgNaviBar::OnBnClickedButtonOperate()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_MainNavigate::OnBnClickedButtonOperate"));

	m_iCurrentView = enumNavigateID_OPERATE;
	GetParent()->SendMessage(WM_MSG_UPDATE_CHANGE_VIEW_STATE, (WPARAM)m_iCurrentView, NULL);
}


void CDlgNaviBar::OnBnClickedButtonResult()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_MainNavigate::OnBnClickedButtonResult"));

	m_iCurrentView = enumNavigateID_RESULT_DATA;
	GetParent()->SendMessage(WM_MSG_UPDATE_CHANGE_VIEW_STATE, (WPARAM)m_iCurrentView, NULL);
}


void CDlgNaviBar::OnBnClickedButtonMaint()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_MainNavigate::OnBnClickedButtonMaint"));

	m_iCurrentView = enumNavigateID_MAINT;
	GetParent()->SendMessage(WM_MSG_UPDATE_CHANGE_VIEW_STATE, (WPARAM)m_iCurrentView, NULL);
}


void CDlgNaviBar::OnBnClickedButtonRecipe()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_MainNavigate::OnBnClickedButtonRecipe"));

	m_iCurrentView = enumNavigateID_RECIPE;
	GetParent()->SendMessage(WM_MSG_UPDATE_CHANGE_VIEW_STATE, (WPARAM)m_iCurrentView, NULL);
}


void CDlgNaviBar::OnBnClickedButtonLog()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_MainNavigate::OnBnClickedButtonLog"));

	m_iCurrentView = enumNavigateID_SYSTEM_LOG;
	GetParent()->SendMessage(WM_MSG_UPDATE_CHANGE_VIEW_STATE, (WPARAM)m_iCurrentView, NULL);
}


void CDlgNaviBar::OnBnClickedButtonConfig()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_MainNavigate::OnBnClickedButtonConfig"));

	m_iCurrentView = enumNavigateID_CONFIG;
	GetParent()->SendMessage(WM_MSG_UPDATE_CHANGE_VIEW_STATE, (WPARAM)m_iCurrentView, NULL);
}


void CDlgNaviBar::OnBnClickedButtonAlarm()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_MainNavigate::OnBnClickedButtonAlarm"));

	m_iCurrentView = enumNavigateID_ALARM;
	GetParent()->SendMessage(WM_MSG_UPDATE_CHANGE_VIEW_STATE, (WPARAM)m_iCurrentView, NULL);
}


void CDlgNaviBar::OnBnClickedButtonBuzzerOff()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_MainNavigate::OnBnClickedButtonBuzzerOff"));

	m_pAuroraMgr->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_BIT_EQ_BUZZER, false);

	m_pAuroraMgr->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_ALRAM_RESET_STATUS, 1, 1);
	Sleep(200);
	m_pAuroraMgr->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_ALRAM_RESET_STATUS, 0, 1);
}


void CDlgNaviBar::OnBnClickedButtonPmMode()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_MainNavigate::OnBnClickedButtonPmMode"));

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	//CD_PM_MODE dlg(m_pAuroraMgr->GetIOMapControlMgrPtr(), m_pAuroraMgr->GetLogInControlMgrPtr());
	//dlg.DoModal();
}


BOOL CDlgNaviBar::OnInitDialog()
{
	CMainSubDlgWin::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	SetBackgroundColor(RGB(128, 128, 128));

	//--------------------------------------------------------------------
	// 버튼 이미지 추가
	//--------------------------------------------------------------------
	for (size_t i = 0; i < enumNavigateIDCount; i++)
	{
		if (i == enumNavigateID_BUZZER_OFF)
		{
			m_btnImage[i].SetBitmaps(IDB_MENU_D, NULL, IDB_MENU_D, NULL, IDB_MENU_DIS, NULL);
			m_btnImage[i].SetColor(CButtonST::BTNST_COLOR_FG_IN, CLR_YELLOW);
			m_btnImage[i].SetColor(CButtonST::BTNST_COLOR_FG_OUT, RGB_DARK_BLUE);
			m_btnImage[i].SetColor(CButtonST::BTNST_COLOR_FG_DIS, CLR_WHITE);
			m_btnImage[i].SetAlign(CButtonST::ST_ALIGN_OVERLAP);
		}
		else
		{
			m_btnImage[i].SetBitmaps(IDB_MENU_D, NULL, IDB_MENU_U, NULL, IDB_MENU_DIS, NULL);
			m_btnImage[i].SetColor(CButtonST::BTNST_COLOR_FG_IN, CLR_YELLOW);
			m_btnImage[i].SetColor(CButtonST::BTNST_COLOR_FG_OUT, RGB_DARK_BLUE);
			m_btnImage[i].SetColor(CButtonST::BTNST_COLOR_FG_DIS, CLR_WHITE);
			m_btnImage[i].SetAlign(CButtonST::ST_ALIGN_OVERLAP);
		}
	}

	m_iCurrentView = enumNavigateID_OPERATE;
	GetParent()->SendMessage(WM_MSG_UPDATE_CHANGE_VIEW_STATE, (WPARAM)m_iCurrentView, NULL);

	UpdateInitialState(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgNaviBar::UpdateInitialState(BOOL bEnable)
{
	for (int i = enumNavigateID_RESULT_DATA; i < enumNavigateIDCount; ++i)
	{
		//// maint 창
		//if (i == enumNavigateID_MAINT)
		//{
		//	if (CSystemParamShare::GetInstance()->IsInitialized() == false)
		//		continue;
		//}

		if (i == enumNavigateID_ALARM || i == enumNavigateID_SYSTEM_LOG || i == enumNavigateID_BUZZER_OFF || i == enumNavigateID_RESULT_DATA)
			continue;

		if (i == enumNavigateID_MAINT || i == enumNavigateID_CONFIG)
		//if (i == enumNavigateID_MAINT )
		//{
		//	;
		//	/*if (bEnable == TRUE)
		//	{
		//		if (m_pAuroraMgr->GetLogInControlMgrPtr()->GetLoginLevel() != 0)
		//		{
		//			m_btnImage[i].EnableWindow(TRUE);
		//		}
		//	}
		//	else
		//	{
		//		m_btnImage[i].EnableWindow(bEnable);
		//	}*/
		//}
		//else if (i == enumNavigateID_CONFIG)
		{
			if (bEnable == TRUE)
			{
				if (m_pAuroraMgr->GetLogInControlMgrPtr()->GetLoginLevel() != 0)
				{
					m_btnImage[i].EnableWindow(TRUE);
				}
				else
				{
					m_btnImage[i].EnableWindow(FALSE);
				}
			}
			else
			{
				m_btnImage[i].EnableWindow(bEnable);
			}
		}
		else
		{
			m_btnImage[i].EnableWindow(bEnable);
		}
	}
}

void CDlgNaviBar::ChangeCheck(int index)
{
	for (size_t i = 0; i < enumNavigateIDCount; i++)
	{
		if (i == enumNavigateID_LOGIN || i == enumNavigateID_BUZZER_OFF || i == enumNavigateID_PM_MODE)
			continue;


		if (i == enumNavigateID_ALARM) 
		{
			// int nAlarm = CAlarmController::GetInstance()->GetAlarmCount();
			int nAlarm = 0;
			if (nAlarm>0) 
			{
				m_btnImage[enumNavigateID_ALARM].SetCheck(TRUE);
			}
			else
			{
				if (i != index)
					m_btnImage[i].SetCheck(FALSE);
				else
					m_btnImage[i].SetCheck(TRUE);
			}
		}
		else
		{
			if (i != index)
				m_btnImage[i].SetCheck(FALSE);
			else
				m_btnImage[i].SetCheck(TRUE);
		}
	}
}

void CDlgNaviBar::OnBnClickedCheckManual()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_MainNavigate::OnBnClickedCheckManual"));

	m_iCurrentView = enumNavigateID_MANUAL;
	GetParent()->SendMessage(WM_MSG_UPDATE_CHANGE_VIEW_STATE, (WPARAM)m_iCurrentView, NULL);
}
