
// DlgALPHA.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "ALPHA.h"
#include "DlgALPHA.h"
#include "afxdialogex.h"

// jsyoon 20211124 add
#include "AuroraMgr.h"
#include "D_AuroraDlgWin.h"
#include "DlgMainForm.h"
#include "DlgTitleBar.h"
#include "Alarm\AlarmMonitorMgr.h"
#include "Alarm/AlarmController.h"
#include "Manager/CommTcpIpFactoryMgr.h"
#include "Manager/MaintControlMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDlgALPHA 대화 상자



CDlgALPHA::CDlgALPHA(CAuroraMgr* pAuroraMgr, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_ALPHA_DIALOG, pParent)
	, m_pAuroraMgr(pAuroraMgr)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDlgALPHA::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgALPHA, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CDlgALPHA 메시지 처리기

BOOL CDlgALPHA::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	//------------------------------------------------------------------------------
	// 화면 크기에 따라서 표시해주는 영역을 달라 하고자 한다. 19" 와 WIDE 형태
	//------------------------------------------------------------------------------
	MoveWindow(0, 0, 1920, 1050);
	UpdateWindow();
	SetWindowText(_T("HBS_OS"));


	CRect rcTitle(0, 0, 1920, 80), rcMainForm(0, 81, 1920, 1049);
	GetDlgItem(IDC_STATIC_TITLE)->MoveWindow(rcTitle);
	GetDlgItem(IDC_STATIC_MAIN_FORM)->MoveWindow(rcMainForm);

	//-------------------------------------------------------------------------------------
	// 다이얼로그 생성 시작
	//-------------------------------------------------------------------------------------
	CRect rtROI;
	GetDlgItem(IDC_STATIC_TITLE)->GetWindowRect(&rtROI);
	ScreenToClient(&rtROI);
	CD_AuroraDlgWin* pInfoWin = dynamic_cast<CD_AuroraDlgWin*>(new CDlgTitleBar(m_pAuroraMgr, this));
	pInfoWin->Create(IDD_DLG_MAIN_TITLE);
	pInfoWin->MoveWindow(rtROI);
	m_pAuroraMgr->PushSubDlg(enumWndID_TITLE, pInfoWin);

	GetDlgItem(IDC_STATIC_MAIN_FORM)->GetWindowRect(&rtROI);
	ScreenToClient(&rtROI);
	pInfoWin = dynamic_cast<CD_AuroraDlgWin*>(new CDlgMainForm(m_pAuroraMgr, this));
	pInfoWin->Create(IDD_DLG_MAIN_FORM);
	pInfoWin->MoveWindow(rtROI);
	m_pAuroraMgr->PushSubDlg(enumWndID_FRAME, pInfoWin);



	//--------------------------------------------------------------------
	// 화면에 표시되는 내용을 갱신한다.
	//--------------------------------------------------------------------
	ShowWindow(SW_MAXIMIZE);

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CDlgALPHA::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CDlgALPHA::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



BOOL CDlgALPHA::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
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


void CDlgALPHA::OnClose()
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (AfxMessageBox(_T("Would you like to exit the program?"), MB_YESNO | MB_ICONQUESTION) != IDYES)
	{
		return;
	}

	// 알람을 처리 하지 않도록 Monitoring 을 정지한다.
	m_pAuroraMgr->GetAlarmMonitorMgrPtr()->SetMonitoringStop();
	Sleep(1500);	// Alarm Stop Delay
	// 알람이 미리 발생되어 있는 경우에는 종료가 안될수 있다.

	CString strLog;
	strLog.Format(_T("This app stop - User [ %s ], Level : [ %s ]"), m_pAuroraMgr->GetLoginInfo().user.GetString(), m_pAuroraMgr->GetLoginInfo().level.GetString());
	CLogWrapper::GetInstance()->KMLOG(OPER, strLog);
	strLog.Format(_T("This app stop - Name [ %s ], Version : [ %s ]"), m_pAuroraMgr->GetSystemInfo().name.GetString(), m_pAuroraMgr->GetSystemInfo().version.GetString());
	CLogWrapper::GetInstance()->KMLOG(OPER, strLog);

	//--------------------------------------------------------------------
	// 알람을 해제 하기 위해서 창을 모두 HIDE 시킨다.
	//--------------------------------------------------------------------
	for (int i = 0; i < m_pAuroraMgr->GetSubDlgCount(); ++i)
	{
		m_pAuroraMgr->Notify_Direct(i, WM_MSG_UPDATE_CLOSE_SYSTEM, 0, 0);

		m_pAuroraMgr->GetSubDlgPtr(i)->ShowWindow(SW_HIDE);
	}

	int nAlarm = CAlarmController::GetInstance()->ClearAlarm();

//	Sleep(1000);
	// jsyoon 2022.07.06 add
	if (m_pAuroraMgr)
	{
		m_pAuroraMgr->Finalize();
	}

	CAlarmController::GetInstance()->DelInstance();

	/*int nAlarm = CAlarmController::GetInstance()->ClearAlarm();
	SetTimer(101, 1000, NULL);*/
	//CAlarmController::GetInstance()->DelInstance();

	if (m_pAuroraMgr)
	{
		delete m_pAuroraMgr;
		m_pAuroraMgr = nullptr;
	}

	CDialogEx::OnClose();
}
