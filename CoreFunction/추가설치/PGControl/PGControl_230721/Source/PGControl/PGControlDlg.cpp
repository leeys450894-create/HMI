#include "pch.h"
#include "PGControl.h"
#include "PGControlDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

C_WaitDlg* theWaitDlg = NULL;

BEGIN_MESSAGE_MAP(CPGControlDlg, CDialogEx)
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_BN_CLICKED(IDC_BUTTON_1, &CPGControlDlg::OnBtnBeginPG)
	ON_COMMAND_RANGE(IDC_RADIO_1, IDC_RADIO_3, OnSelProcMode)
END_MESSAGE_MAP()

CPGControlDlg::CPGControlDlg(CWnd* pParent /*=nullptr*/) : C_BaseDialog(IDD_PGCONTROL_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	theWaitDlg = NULL;
}

CPGControlDlg::~CPGControlDlg()
{

}

void CPGControlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_1, m_lstBox);
	DDX_Control(pDX, IDC_GRID_1, m_gridInfo);
	DDX_Control(pDX, IDC_GRID_2, m_gridStatus);
	DDX_Control(pDX, IDC_BUTTON_1, m_btnRunProc);
	for (int i = 0; i < 3; i++) DDX_Control(pDX, IDC_RADIO_1 + i, m_rdProcMode[i]);
}

BOOL CPGControlDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.
	
	//생성
	theWaitDlg = new C_WaitDlg(this);
	//

	//초기화
	InitGridInfo();
	InitGridStatus();

	InitButtons(m_btnRunProc);

	for (int i = 0; i < 3; i++) InitButtons(m_rdProcMode[i]);
	m_rdProcMode[2].SetCheck(TRUE);
	m_rdProcMode[2].SetTextColor(COLOR_YELLOW_DARK);
	m_nAutoMode = DF_MODE_SYNC;
	//

	m_bQualityRun = false;
	m_bColorPGOn = false;

	SetTimer(1, 100, NULL);

	SendMessage	 (WM_SIZE, (WPARAM)SIZE_RESTORED, MAKELPARAM(0, 0));
	SetWindowPos (NULL, 1, 1, 997, 525, SWP_NOZORDER);

	CmPGControlBegin();

	return TRUE;
}

void	CPGControlDlg::OnCancel()
{
	if (AfxMessageBox(_T("프로그램을 종료 합니다.\n 계속 하시겠습니까?"), MB_YESNO) == IDNO) return;
	CDialogEx::OnCancel();
}

void CPGControlDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	CmStop ();

	KillTimer(1);

	SAFE_DESTROY(theWaitDlg);
}

void CPGControlDlg::OnPaint()
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

HCURSOR CPGControlDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CPGControlDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	static BOOL bFirst = FALSE;

	if (theWaitDlg && !bFirst) {
		CRect rc;
		GetWindowRect(&rc);
		ScreenToClient(&rc);
		CRect rcWait;
		theWaitDlg->GetWindowRect(&rcWait);
		theWaitDlg->SetWindowPos(NULL, 3, rc.bottom - rcWait.Height() - 10, rcWait.Width() - 45, rcWait.Height(), SWP_NOZORDER);

		bFirst = TRUE;
	}
}

void	CPGControlDlg::OnTimer(UINT_PTR nIDEvent)
{
	CDialogEx::OnTimer(nIDEvent);

	static int nTime = 1;
	static int nSec = 0;

	//0.2sec
	if (nTime % 2) {

	}
	else {

	}

	//0.5sec
	if (nTime % 5 == 0) {
		DisplayConnectState();
	}

	nTime++;

	if (nTime > 10) {
		nSec++;
		nTime = 1;
	}

	//1시간단위 타이머
	if (nSec == 3600) {
		nSec = 0;
	}

}

#define df_MAX_LIST 50

void CPGControlDlg::WriteLog(CString strMsg)
{
	if (!m_lstBox) return;
//	if (100 < m_lstBox.GetCount ()) m_lstBox.ResetContent ();	//	2023.07.17 remove

	SYSTEMTIME st;
	::GetLocalTime(&st);

	CString sList;
	sList.Format(_T("%02d-%02d %02d:%02d:%02d:%03d : %s "), st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, strMsg);

	//최신 로그가 위에 기록되도록 변경
	m_lstBox.InsertString(0, sList);
	m_lstBox.SetCurSel(0);

	int Count = m_lstBox.GetCount();
	if (df_MAX_LIST < Count) m_lstBox.DeleteString(df_MAX_LIST);	//	2023.07.17 change

	//if (m_lstBox.GetCount() > 100) m_lstBox.ResetContent();
	//m_lstBox.InsertString(-1, sTime + strMsg);
}

void CPGControlDlg::InitGridInfo()
{
	InitGridUI(m_gridInfo, IDC_GRID_1, DF_INFO_PARM_ITEM_MAX, DF_INFO_PARM_COL, SETTING_GRID_SMALL);

	m_gridInfo.SetItemText(0, 0, _T("화질"));
	m_gridInfo.SetItemText(1, 0, _T("면휘도"));
	m_gridInfo.SetItemText(2, 0, _T("운영"));
	m_gridInfo.SetItemText(3, 0, _T("PG"));

	for (int i = 0; i < DF_INFO_PARM_ITEM_MAX; i++) {
		m_gridInfo.SetItemFgColour	(i, 0, COLOR_BLACK);
		m_gridInfo.SetItemFgColour	(i, 1, COLOR_BLACK);
		m_gridInfo.SetItemFgColour	(i, 2, COLOR_BLACK);
	}

	//for (int i = 0; i < DF_INFO_PARM_ITEM_MAX; i++) {
	//	m_gridInfo.SetItemState		(i, 1, m_gridInfo.GetItemState(i, 1) | GVIS_READONLY);
	//	m_gridInfo.SetItemFgColour	(i, 1, COLOR_GREEN_DARK);

	//	m_gridInfo.SetItemState		(i, 2, m_gridInfo.GetItemState(i, 2) | GVIS_READONLY);
	//	m_gridInfo.SetItemFgColour	(i, 2, COLOR_GREEN_DARK);
	//}
}

void CPGControlDlg::InitGridStatus()
{
	InitGridUI(m_gridStatus, IDC_GRID_2, DF_STATUS_PARM_ITEM_MAX, DF_STATUS_PARM_COL, -1);

	m_gridStatus.SetItemText(0, 0, _T("화질 TURN ON"));
	m_gridStatus.SetItemText(1, 0, _T("화질 TURN OFF"));
	m_gridStatus.SetItemText(2, 0, _T("화질 NEXT"));

	m_gridStatus.SetItemText(0, 1, _T("면휘도 TURN ON"));
	m_gridStatus.SetItemText(1, 1, _T("면휘도 TURN OFF"));
	m_gridStatus.SetItemText(2, 1, _T("면휘도 NEXT"));

	for (int i = 0; i < DF_STATUS_PARM_ITEM_MAX; i++) {
		m_gridStatus.SetItemFgColour(i, 0, COLOR_BLACK);
		m_gridStatus.SetItemFgColour(i, 1, COLOR_BLACK);
	}

}

void CPGControlDlg::DisplayConnectState()
{
	//화질 PG
	if (theApp.m_pQualityPG232Comm) {
		if (theApp.m_pQualityPG232Comm->IsConnected ()) m_gridInfo.SetItemBkColour (0, 1, COLOR_GREEN);
		else										    m_gridInfo.SetItemBkColour (0, 1, COLOR_RED);

		m_gridInfo.SetItemText (0, 1, theApp.m_pQualityPG232Comm->GetComPort ());

		if (theApp.m_pQualityPG232Comm->GetWaitItem (QPG_WAIT_TURN_ON))  m_gridStatus.SetItemBkColour(0, 0, COLOR_GREEN);
		else														     m_gridStatus.SetItemBkColour(0, 0, COLOR_WHITE);

		if (theApp.m_pQualityPG232Comm->GetWaitItem (QPG_WAIT_TURN_OFF)) m_gridStatus.SetItemBkColour(1, 0, COLOR_GREEN);
		else														     m_gridStatus.SetItemBkColour(1, 0, COLOR_WHITE);

		if (theApp.m_pQualityPG232Comm->GetWaitItem (QPG_WAIT_NEXT))	 m_gridStatus.SetItemBkColour(2, 0, COLOR_GREEN);
		else														     m_gridStatus.SetItemBkColour(2, 0, COLOR_WHITE);
	}

	//PG
	if (theApp.m_pPG232Comm) {
		if (theApp.m_pPG232Comm->IsConnected ()) m_gridInfo.SetItemBkColour (3, 1, COLOR_GREEN);
		else									 m_gridInfo.SetItemBkColour (3, 1, COLOR_RED);

		m_gridInfo.SetItemText (3, 1, theApp.m_pPG232Comm->GetComPort ());
	}

	//Color PG
	if (theApp.m_pColorPGServerComm) {
		if (theApp.m_pColorPGServerComm->IsConnected ()) m_gridInfo.SetItemBkColour (1, 1, COLOR_GREEN);
		else											 m_gridInfo.SetItemBkColour (1, 1, COLOR_RED);
		
		m_gridInfo.SetItemText (1, 1, dfcStr (_T("%s:%d"), theApp.m_pColorPGServerComm->GetIPAddress(), theApp.m_pColorPGServerComm->GetPortNum()));

		if (theApp.m_pColorPGServerComm->GetWaitItem (CPG_WAIT_TURN_ON))  m_gridStatus.SetItemBkColour(0, 1, COLOR_GREEN);
		else														      m_gridStatus.SetItemBkColour(0, 1, COLOR_WHITE);

		if (theApp.m_pColorPGServerComm->GetWaitItem (CPG_WAIT_TURN_OFF)) m_gridStatus.SetItemBkColour(1, 1, COLOR_GREEN);
		else														      m_gridStatus.SetItemBkColour(1, 1, COLOR_WHITE);

		if (theApp.m_pColorPGServerComm->GetWaitItem (CPG_WAIT_NEXT))	  m_gridStatus.SetItemBkColour(2, 1, COLOR_GREEN);
		else														      m_gridStatus.SetItemBkColour(2, 1, COLOR_WHITE);
	}

	//Main PG
	if (theApp.m_pMainPGClientComm) {
		if (theApp.m_pMainPGClientComm->IsConnected ()) m_gridInfo.SetItemBkColour (2, 1, COLOR_GREEN);
		else											m_gridInfo.SetItemBkColour (2, 1, COLOR_RED);
		
		m_gridInfo.SetItemText (2, 1, dfcStr (_T("%s:%d"), theApp.m_pMainPGClientComm->GetIPAddress(), theApp.m_pMainPGClientComm->GetPortNum()));
	}

	Invalidate(FALSE);
}

void CPGControlDlg::OnBtnBeginPG()
{
	CmPGControlBegin();
}

void	CPGControlDlg::OnSelProcMode(UINT nID)
{
	int nOldItem = 0;

	switch (nID) {
	case IDC_RADIO_1:	m_nAutoMode = DF_MODE_ONLY_QUALITY;	 break;
	case IDC_RADIO_2:	m_nAutoMode = DF_MODE_ONLY_COLOR;	 break;
	case IDC_RADIO_3:	m_nAutoMode = DF_MODE_SYNC;			 break;
	}

	for (int i = 0; i < 3; i++) {

		if (i == m_nAutoMode) {
			m_rdProcMode[i].SetTextColor(COLOR_YELLOW_DARK);
			m_rdProcMode[i].SetTextHotColor(COLOR_YELLOW_DARK);
		}
		else {
			m_rdProcMode[i].SetTextColor(COLOR_GRAY_BRIGHT);
			m_rdProcMode[i].SetTextHotColor(COLOR_YELLOW_DARK);
		}
	}

	Invalidate(FALSE);
}