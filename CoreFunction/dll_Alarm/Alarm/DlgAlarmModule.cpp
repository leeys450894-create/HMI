// DlgAlarmModule.cpp : implementation file
//

#include "pch.h"
#include "Alarm.h"
#include "DlgAlarmModule.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgAlarmModule dialog

CDlgAlarmModule::CDlgAlarmModule(CWnd* pParent /*= NULL*/)
: CDialog(CDlgAlarmModule::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgAlarmModule)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	
	m_brushBK.CreateSolidBrush(RGB(252,69,92));

	m_pAlarm = NULL;
}


void CDlgAlarmModule::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgAlarmModule)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_STATIC_ALARM_ID, m_LabelInfo[enumStaticID_AlarmID]);
	DDX_Control(pDX, IDC_STATIC_ALARM_MODULE, m_LabelInfo[enumStaticID_AlarmModule]);
	DDX_Control(pDX, IDC_STATIC_ALARM_TYPE, m_LabelInfo[enumStaticID_AlarmType]);
	DDX_Control(pDX, IDC_STATIC_ALARM_COMMAND, m_LabelInfo[enumStaticID_AlarmCommand]);
	DDX_Control(pDX, IDC_STATIC_ALARM_TEXT, m_LabelInfo[enumStaticID_AlarmText]);
	DDX_Control(pDX, IDC_STATIC_ALARM_DESC, m_LabelInfo[enumStaticID_AlarmDesc]);
}


BEGIN_MESSAGE_MAP(CDlgAlarmModule, CDialog)
	//{{AFX_MSG_MAP(CDlgAlarmModule)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_ALARM_MODULE_INITRETRY, &CDlgAlarmModule::OnButtonAlarmModuleInitretry)
	ON_BN_CLICKED(IDC_BUTTON_ALARM_MODULE_RETRY, &CDlgAlarmModule::OnButtonAlarmModuleRetry)
	ON_BN_CLICKED(IDC_BUTTON_ALARM_MODULE_IGNORE, &CDlgAlarmModule::OnButtonAlarmModuleIgnore)
	ON_BN_CLICKED(IDC_BUTTON_ALARM_MODULE_STOP, &CDlgAlarmModule::OnButtonAlarmModuleAbort)
	//}}AFX_MSG_MAP
	ON_WM_CLOSE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgAlarmModule message handlers

HBRUSH CDlgAlarmModule::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	if (pWnd == this)
	{
		return (HBRUSH)m_brushBK;
	}
	
	// TODO: Return a different brush if the default is not desired
	return hbr;
}

BOOL CDlgAlarmModule::OnInitDialog() 
{
	CDialog::OnInitDialog();

	//---------------------------------------------------------------------
	// 로그 타이틀에 색상을 입혀준다.
	//---------------------------------------------------------------------
	for (int i = 0; i < enumStaticID_AlarmCount; i++)
	{
		m_LabelInfo[i].SetBorder(TRUE, TRUE);
		m_LabelInfo[i].SetBkColor(RGB(192, 255, 192));
		m_LabelInfo[i].SetTextColor(RGB(0, 0, 0));
		m_LabelInfo[i].SetFontBold(TRUE);
	}

	m_LabelInfo[enumStaticID_AlarmID].SetBorder(TRUE, TRUE);
	m_LabelInfo[enumStaticID_AlarmID].SetBkColor(RGB(255, 255, 255));
	m_LabelInfo[enumStaticID_AlarmID].SetTextColor(RGB(0, 0, 0));
	m_LabelInfo[enumStaticID_AlarmID].SetFontBold(TRUE);

	m_LabelInfo[enumStaticID_AlarmText].SetBorder(TRUE, TRUE);
	m_LabelInfo[enumStaticID_AlarmText].SetBkColor(RGB(255, 255, 255));
	m_LabelInfo[enumStaticID_AlarmText].SetTextColor(RGB(0, 0, 0));
	m_LabelInfo[enumStaticID_AlarmText].SetFontBold(TRUE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgAlarmModule::ShowAlarm(ALARM_EVENT_DATA *pAlarm)
{
	if (pAlarm == NULL)		// jsyoon 20191101 add
		return;

	m_pAlarm = pAlarm;


	CString s;
	s.Format(_T("ALARM ID : %d"), pAlarm->id);
	m_LabelInfo[enumStaticID_AlarmID].SetText(s);

	s.Format(_T("TYPE : %d"), pAlarm->nType);
	m_LabelInfo[enumStaticID_AlarmType].SetText(s);

	s.Format(_T("MODULE : %s"), pAlarm->module);
	m_LabelInfo[enumStaticID_AlarmModule].SetText(s);

	s.Format(_T("CMD : %s"), pAlarm->cmd);
	m_LabelInfo[enumStaticID_AlarmCommand].SetText(s);

	s.Format(_T("%s"), pAlarm->text);
	m_LabelInfo[enumStaticID_AlarmText].SetText(s);

	s.Format(_T("%s"), pAlarm->Description);
	m_LabelInfo[enumStaticID_AlarmDesc].SetText(s);

	//if (pAlarm->id > 7100 && pAlarm->id < 7800)
	//{
	//	GetDlgItem(IDC_BUTTON_ALARM_MODULE_IGNORE)->ShowWindow(SW_SHOW);
	//}
	//else
	//{
	//	GetDlgItem(IDC_BUTTON_ALARM_MODULE_IGNORE)->ShowWindow(SW_HIDE);
	//}

	if (pAlarm->nRetry == 1)
	{
		GetDlgItem(IDC_BUTTON_ALARM_MODULE_RETRY)->ShowWindow(SW_SHOW);
	}
	else
	{
		GetDlgItem(IDC_BUTTON_ALARM_MODULE_RETRY)->ShowWindow(SW_HIDE);
	}

	GetDlgItem(IDC_BUTTON_ALARM_MODULE_INIT_RETRY)->ShowWindow(SW_HIDE);

	ShowWindow(SW_SHOW);
}

void CDlgAlarmModule::OnButtonAlarmModuleInitretry() 
{
	m_pAlarm->nRet = ALARM_RV_INITRETRY;

	SetEvent(m_pAlarm->hEvent);
}

void CDlgAlarmModule::OnButtonAlarmModuleRetry() 
{
	m_pAlarm->nRet = ALARM_RV_RETRY;

	SetEvent(m_pAlarm->hEvent);
}

void CDlgAlarmModule::OnButtonAlarmModuleIgnore() 
{
	m_pAlarm->nRet = ALARM_RV_IGNORE;

	SetEvent(m_pAlarm->hEvent);
}

void CDlgAlarmModule::OnButtonAlarmModuleAbort() 
{
	m_pAlarm->nRet = ALARM_RV_ABORT;

	SetEvent(m_pAlarm->hEvent);
}


BOOL CDlgAlarmModule::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	switch(pMsg->message)
	{
	case WM_KEYDOWN:
		if(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
			return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}


void CDlgAlarmModule::OnClose()
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	CDialog::OnClose();
}
