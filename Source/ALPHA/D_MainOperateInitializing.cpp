// D_MainOperateInitializing.cpp : 구현 파일입니다.
//

#include "pch.h"
#include "ALPHA.h"
#include "D_MainOperateInitializing.h"
#include "afxdialogex.h"

#include "Manager\SequenceManager.h"
#include "Manager\SACCMgr.h"
#include "DoWork\SystemInitialize.h"
#include "Manager\ShareMemoryMgr.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CD_MainOperateInitializing 대화 상자입니다.
typedef BOOL(WINAPI* SetLayer)(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);

IMPLEMENT_DYNAMIC(CD_MainOperateInitializing, CDialogEx)

CD_MainOperateInitializing::CD_MainOperateInitializing(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_POPUP_OPERATE_SYSTEM_INITIALIZE, pParent)
	, m_bEngineBreak(false)
	, m_nAgingTime(10)
	, m_nTimeCount(0)
	, m_nInitShow(false)
{
}

CD_MainOperateInitializing::~CD_MainOperateInitializing()
{
}

void CD_MainOperateInitializing::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_INITBOX_TITLE,			m_stclabelTitle);
	DDX_Control(pDX, IDC_INITBOX_PROGRESS,		m_prgInitState);
	DDX_Control(pDX, IDC_INITBOX_PERCENT,		m_stcPercent);
}


BEGIN_MESSAGE_MAP(CD_MainOperateInitializing, CDialogEx)
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CD_MainOperateInitializing 메시지 처리기입니다.

BOOL CD_MainOperateInitializing::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	m_stclabelTitle.SetBorder(TRUE, TRUE);
	m_stclabelTitle.SetBkColor(RGB(200, 200, 200));
	m_stclabelTitle.SetTextColor(RGB(100, 149, 237));
	m_stclabelTitle.SetFontBold(TRUE);
	m_stclabelTitle.SetFontSize(25);

	m_prgInitState.SetPos(0);
	m_stcPercent.SetWindowText(_T("0"));

	HMODULE hUser32 = GetModuleHandle(_T("USER32.DLL"));
	SetLayer pSetLayer = (SetLayer)GetProcAddress(hUser32, "SetLayeredWindowAttributes");

	if (pSetLayer != NULL)
	{
		int alphaValue = 200; //투명도 설정 0 ~ 255 ( 0은 완전투명 )
		SetWindowLong(this->m_hWnd, GWL_EXSTYLE,
			GetWindowLong(this->m_hWnd, GWL_EXSTYLE) | 0x80000);
		pSetLayer(this->m_hWnd, 0, alphaValue, LWA_ALPHA);
	}
	
	
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CD_MainOperateInitializing::InitStart(int iType)
{
	m_bEngineBreak = false;
	CString str;

	ShowWindow(SW_SHOW);

	// jsyoon 2022.08.02
	if (m_nInitShow == false)
	{
		CRect rect;
		this->GetWindowRect(rect);
		this->MoveWindow(rect.left + 200, rect.top, rect.Width(), rect.Height());
		m_nInitShow = true;
	}
	
	if (iType == 0)
	{
		SetTimer(MAIN_OPERATE_SUB_SYSTEM_INITIALIZE_TIMER_ID, 500, NULL);

		str.Format(_T("Initializing..."));
		m_stclabelTitle.SetText(str);
	}
	else if (iType == 1)
	{
		SetTimer(MAIN_OPERATE_SUB_SYSTEM_SACC_TIMER_ID, 500, NULL);

		str.Format(_T("SACC Processing..."));
		m_stclabelTitle.SetText(str);

		m_prgInitState.SetPos(0);
	}
	else if (iType == 2)
	{
		m_nTimeCount = 1;
		SetTimer(MAIN_OPERATE_SUB_SYSTEM_AGING_TIMER_ID, 1000, NULL);

		str.Format(_T("AGING Processing..."));
		m_stclabelTitle.SetText(str);
	}
}

void CD_MainOperateInitializing::InitStop()
{
	m_bEngineBreak = true;
}

void CD_MainOperateInitializing::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if(nIDEvent == MAIN_OPERATE_SUB_SYSTEM_INITIALIZE_TIMER_ID)
	{
		// INITIALIZE SEQUENCE ITEM
		CSystemInitialize *pSystemInitialize = CSequenceManager::GetInstance()->GetSystemInitialize();
		
		if(pSystemInitialize)
		{
			CString m_strDesc = pSystemInitialize->GetDesc();
			int m_iCurStepPos = pSystemInitialize->GetCurStep();
			int m_iTotalStepCount = pSystemInitialize->GetTotalStepCount();

			if(0 < m_iTotalStepCount)
			{		
				const int cnt = m_iTotalStepCount;
				const int percent = (int)((m_iCurStepPos + 1) * 100.f / cnt);

				CString tmpVal;
				tmpVal.Format(_T("%d %%"), percent);
				m_stcPercent.SetWindowText(tmpVal);
				m_prgInitState.SetPos(percent);

				CString str;

				switch (m_iCurStepPos)
				{
				case 0:
					str.Format(_T("Initializing..."));
					break;
				case 1:
					str.Format(_T("Initializing...InitModule"));
					break;
				case 2:
					str.Format(_T("Initializing...CheckInterlock"));	
					break;
				case 3:
					str.Format(_T("Initializing...InitIO"));		
					break;
				case 4:
					str.Format(_T("Initializing...MotionHoming 1. Lift Pin"));			
					break;
				case 5:
					str.Format(_T("Initializing...MotionHoming 2.Z Axis")); 
					break;
				case 6:
					str.Format(_T("Initializing...MotionHoming"));
					break;
				case 7:
					str.Format(_T("Initializing...MotionHoming 3.Head X / Y" ));
					break;
				case 8:
					str.Format(_T("Initializing...MotionHoming 4.Probe Z"));
					break;
				case 9:
					str.Format(_T("Initializing...MotionHoming 5.Probe Cross"));
					break;
				case 10:
					str.Format(_T("Initializing...MotionHoming 6.Probe Gantry"));
					break;
				case 11:
					str.Format(_T("Initializing...MotionHoming 7.Stage"));
					break;
				case 12:
					str.Format(_T("Initializing...MotionHoming 8.Stage Rotate"));
					break;
				case 13:
					str.Format(_T("Initializing...MotionHoming 9.Auto Change Pos Check"));
					break;
				case 14:
					str.Format(_T("Initializing...MotionHoming 10.Auto Change Z"));
					break;
				case 15:
					str.Format(_T("Initializing...MotionHoming 11.Auto Change"));
					break;
				case 16:
					str.Format(_T("Initializing...MotionComplete"));
					break;
				case 17:
					str.Format(_T("Initializing...InitIO"));
					break;
				case 18:
					str.Format(_T("Initializing...Move Safety"));
					break;
				case 19:
					str.Format(_T("Initializing...InitComplete"));
					break;
				case 20:
					str.Format(_T("Initializing...Probe Align"));
					break;
				case 21:
					str.Format(_T("Initializing...Move Safety"));
					break;
				default:
					str.Format(_T("Initializing..."));
					break;
				}
				
				m_stclabelTitle.SetText(str);
				//CString str;
				//str.Format(_T("Initializing Step(%d) : %s"), m_iCurStepPos, m_strDesc);
				//CLogWrapper::GetInstance()->KMLOG(OPER, m_strDesc);
			}
			
			if((m_bEngineBreak == true) || pSystemInitialize->Is_Complete())
			{
				KillTimer(MAIN_OPERATE_SUB_SYSTEM_INITIALIZE_TIMER_ID);

				if(m_bEngineBreak == true)
				{
					pSystemInitialize->Stop();
					pSystemInitialize->Exit();	// exit 시켜줘야 한다.

					SetTimer(MAIN_OPERATE_SUB_SYSTEM_INITIALIZE_TIMER_ID + 1, 500, NULL);
				}
				else
				{
					m_prgInitState.SetPos(100);
					ShowWindow(SW_HIDE);

					CString str = _T("Initializing Complete!!");
					//	AfxMessageBox(str);
					CLogWrapper::GetInstance()->KMLOG(OPER, str);

					CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CHANGE_INITIALIZE, NULL, true);
					//	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_TITLE, WM_MSG_UPDATE_CHANGE_INITIALIZE, NULL, NULL);
				}

				m_bEngineBreak = false;
			}
			else if (pSystemInitialize->Is_Fail())
			{
				KillTimer(MAIN_OPERATE_SUB_SYSTEM_INITIALIZE_TIMER_ID);
				ShowWindow(SW_HIDE);

				CString str = _T("Initializing Fail!!");
//AfxMessageBox(str);
				CLogWrapper::GetInstance()->KMLOG(ERRO, str);

				CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CHANGE_INITIALIZE, NULL, false);
			//	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_TITLE, WM_MSG_UPDATE_CHANGE_INITIALIZE, NULL, NULL);
			}
			else if (pSystemInitialize->Is_AlarmOn())
			{
				ShowWindow(SW_HIDE);
			}
		}
	}

	if (nIDEvent == MAIN_OPERATE_SUB_SYSTEM_INITIALIZE_TIMER_ID + 1)
	{
		CSystemInitialize* pSystemInitialize = CSequenceManager::GetInstance()->GetSystemInitialize();
		if (pSystemInitialize)
		{
			if (pSystemInitialize->Is_ThreadRun() != true)
			{
				KillTimer(MAIN_OPERATE_SUB_SYSTEM_INITIALIZE_TIMER_ID + 1);

				m_prgInitState.SetPos(100);
				ShowWindow(SW_HIDE);

				CString str = _T("Initializing Complete!!");
				CLogWrapper::GetInstance()->KMLOG(OPER, str);
				CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CHANGE_INITIALIZE, NULL, true);
			}
		}
	}

	if (nIDEvent == MAIN_OPERATE_SUB_SYSTEM_SACC_TIMER_ID)
	{
		bool bComplete = CSACCMgr::GetInstance()->Is_Complete();
		bool bFail     = CSACCMgr::GetInstance()->Is_Fail();
		bool bAlarmOn  = CSACCMgr::GetInstance()->Is_AlarmOn();

		if ((m_bEngineBreak == true) || bComplete == true)
		{
			KillTimer(MAIN_OPERATE_SUB_SYSTEM_SACC_TIMER_ID);
			ShowWindow(SW_HIDE);

			m_bEngineBreak = false;

			CString str = _T("SACC Process Complete!!");
		//	AfxMessageBox(str);
			CLogWrapper::GetInstance()->KMLOG(OPER, str);

	//		CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CHANGE_INITIALIZE, NULL, true);
		}
		else if (bFail == true)
		{
			KillTimer(MAIN_OPERATE_SUB_SYSTEM_SACC_TIMER_ID);
			ShowWindow(SW_HIDE);

			CString str = _T("SACC Process Fail!!");
	//		AfxMessageBox(str);
			CLogWrapper::GetInstance()->KMLOG(ERRO, str);

	//		CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CHANGE_INITIALIZE, NULL, false);
			//	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_TITLE, WM_MSG_UPDATE_CHANGE_INITIALIZE, NULL, NULL);
		}
		else if (bAlarmOn)
		{
			ShowWindow(SW_HIDE);
		}
	}

	if (nIDEvent == MAIN_OPERATE_SUB_SYSTEM_AGING_TIMER_ID)
	{
		CString str;
		str.Format(_T("AGING Process... %d / %d sec"), m_nTimeCount, m_nAgingTime);
		m_stclabelTitle.SetText(str);

		const int cnt = m_nAgingTime; // 100
		const int percent = (int)((m_nTimeCount) * 100.f / cnt);

		CString tmpVal;
		tmpVal.Format(_T("%d %%"), percent);
		m_stcPercent.SetWindowText(tmpVal);
		m_prgInitState.SetPos(percent);

		if (m_bEngineBreak == true)
		{
			KillTimer(MAIN_OPERATE_SUB_SYSTEM_AGING_TIMER_ID);
			ShowWindow(SW_HIDE);

			m_bEngineBreak = false;

			CString str = _T("AGING Process Close!!");
			CLogWrapper::GetInstance()->KMLOG(OPER, str);
		}

		m_nTimeCount++;
	}

	CDialogEx::OnTimer(nIDEvent);
}

void CD_MainOperateInitializing::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	KillTimer(MAIN_OPERATE_SUB_SYSTEM_INITIALIZE_TIMER_ID);
	KillTimer(MAIN_OPERATE_SUB_SYSTEM_SACC_TIMER_ID);
	KillTimer(MAIN_OPERATE_SUB_SYSTEM_AGING_TIMER_ID);
}
