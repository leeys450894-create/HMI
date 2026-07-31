// FFUMonDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FFUMon.h"
#include "FFUMonDlg.h"

#include "DlgCurrentValue.h"
#include "DlgLogin.h"
#include "DlgComm.h"	//yhjoo 20141118 add
#include <MATH.H>
#include <winver.h>		

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About
#include "../include/API_SerialCtrl.h"
#pragma comment(lib, "../Lib/dSerialctrl.lib")

#include "../include/API_SharedMemory.h"
#pragma comment(lib, "../Lib/dSharedMemory.lib")

#include "../include/API_SocketControl.h"
#pragma comment(lib, "../Lib/SocketControl.lib")


#define AUTORUN_VALUENAME	"TOTALTOOLS"
#define AUTORUN_ADDARGU		"/User"  
#define RUNKEY				"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"
#define EXE_NAME			"FFUMon.exe"	

#include <direct.h>

static UINT g_uShellReset	=	0;		//	yj.k 20180201 add explorer shotdown -> restart -> TrayIcon Retry show

BOOL gf_MakeDir(CString sPath)
{
	if (sPath[1] != ':' || sPath[2] != '\\')
		return FALSE;
	
	CString sTem;
    int nFind = 2;
	while ((nFind = sPath.Find("\\",nFind+1)) >= 0)
	{
		sTem = sPath.Left(nFind);
		_mkdir(sTem);
	}
	return TRUE;
}

CString gf_MakeCurTimeString()
{
	SYSTEMTIME st;
	::GetLocalTime(&st);
	CString sTime;
	sTime.Format("%04d.%02d.%02d-%02d:%02d:%02d:%03d",st.wYear ,st.wMonth ,st.wDay ,st.wHour ,st.wMinute ,st.wSecond, st.wMilliseconds  );
	return sTime;
}

unsigned int __stdcall CltthreadRecv(void* pParam);	//yhjoo 20141120 add

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP

	ON_BN_CLICKED(IDOK, &CAboutDlg::OnBnClickedOk)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFFUMonDlg dialog

CFFUMonDlg::CFFUMonDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFFUMonDlg::IDD, pParent)
	,m_rAcClient(*AccuraClients::Create())
	, m_nNumChk(0)
	, m_strEQName(_T(""))
	, m_bShowEFU(true)
	, m_nCnt_ESD(3)
	, m_nType_ESD(0)
	, m_bUse_PM5500(FALSE)
	, m_bUse_AZBIL(FALSE)
	, m_iH2OSensorType(0)
{
	//{{AFX_DATA_INIT(CFFUMonDlg)
	m_nRPM = 0;
	m_strCMD = _T("");
	m_bTrayIcon = FALSE; 
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	//lhj20171224 /////////////////
	m_pThreadESDRead = NULL; 
	m_bESDReadDone = FALSE;
	m_hEvtThreadESDReadKilled		= CreateEvent(NULL, TRUE,  FALSE, NULL);  // auto reset,   initially set
	m_hEvtThreadESDReadTerminated	= CreateEvent(NULL, FALSE, FALSE, NULL);  // auto reset,   initially Reset
	/////////////////////////////

	m_hEvtGetsDataStop = CreateEvent(NULL, FALSE, FALSE, NULL);

	m_pGetsDataThread = NULL;
	m_cDATA.m_nStop = FALSE;
	m_bUse_ACCURA = FALSE;
	m_pAccuraData = NULL;

	m_nEFUCnt[0] = 0;
	m_nEFUCnt[1] = 0;
	m_nEFUAllCnt = 0;

	m_nLocalAlarm = 0;

	m_pWarnMsgDlg = NULL; //20140729 added by Jay Hong
	m_strPassWord.Empty();

	int i=0;
	for(i=0; i<dfMAX_EFU; i++)
	{
		m_uEFUSpeedMax[i] =  120;
		m_nEFUStatus[i]   = 0;		// IDLE	// jsyoon 20201208 add
	}

	m_strReceive.Empty();
	m_fValue = 0.0;

	//yhjoo 20141120 add
	m_bRecvThreadRun = FALSE;
	m_hRecvThread = NULL;
	////////////////////////////

	m_iOldModule = -1;

	for(i=0; i<dfMAX_USE_CONNECT; i++)
	{
		m_pDlgComm[i] = NULL;
	}

	for(i=0; i<10; i++)
	{
		m_pDlgESD[i] = NULL;
	}

	for(i=0; i<SENSOR_MODE_END; i++)
	{
		m_stSerial[i].m_uiPort = 1;
		m_stSerial[i].m_uiBaudRate = 9600;
		m_stSerial[i].m_uiDataBits = 0;
		m_stSerial[i].m_uiParity = 0;
		m_stSerial[i].m_uiStopBit = 0;
	}

	m_strIPESD.RemoveAll();

	ZeroMemory(&m_iAzbliValue, sizeof(m_iAzbliValue));

	ZeroMemory(&m_cDATA_PM55, sizeof(m_cDATA_PM55));

	ZeroMemory(&m_cDATA_IONIZER, sizeof(m_cDATA_IONIZER));
	
}

void CFFUMonDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFFUMonDlg)
	DDX_Control(pDX, IDC_STATIC_CNT, m_ledConnected);
	DDX_Control(pDX, IDC_EDIT_CMD, m_edtCMD);
	DDX_Control(pDX, IDC_COMBO_LV32ID, m_cboLVID);
	DDX_Control(pDX, IDC_COMBO_BLSTART, m_cboBLIDEnd);
	DDX_Control(pDX, IDC_COMBO_BLEND, m_cboBLIDStart);
	DDX_Control(pDX, IDC_SPIN_RPM, m_spRPM);
	DDX_Control(pDX, IDC_COMBO_PORT, m_cboPort);
	DDX_Control(pDX, IDC_COMBO_BAUDRATE, m_cboBaudRate);
	DDX_Control(pDX, IDC_COMBO_PARITY, m_cboParity);
	DDX_Control(pDX, IDC_COMBO_DATABIT, m_cboDataBit);
	DDX_Control(pDX, IDC_COMBO_STOPBIT, m_cboStopBit);
	DDX_Control(pDX, IDC_COMBO_STARTDELIMITER, m_cboStartDelimiter);
	DDX_Control(pDX, IDC_COMBO_ENDDELIMITER, m_cboEndDelimiter);
	DDX_Control(pDX, IDC_EDIT_SEND, m_editSendMsg);
	DDX_Control(pDX, IDC_LIST_VIEW, m_ListBox);
	DDX_Text(pDX, IDC_EDIT_RPM, m_nRPM);
	DDX_Text(pDX, IDC_EDIT_CMD, m_strCMD);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_CHK_1, m_chkModule[0]);
	DDX_Control(pDX, IDC_CHK_2, m_chkModule[1]);
	DDX_Control(pDX, IDC_CHK_3, m_chkModule[2]);
	DDX_Control(pDX, IDC_CHK_4, m_chkModule[3]);
	DDX_Control(pDX, IDC_CHK_5, m_chkModule[4]);
	DDX_Control(pDX, IDC_CHK_6, m_chkModule[5]);
	DDX_Control(pDX, IDC_CHK_7, m_chkModule[6]);
	DDX_Control(pDX, IDC_CHK_8, m_chkModule[7]);
	DDX_Control(pDX, IDC_CHK_9, m_chkModule[8]);
	DDX_Control(pDX, IDC_CHK_10, m_chkModule[9]);
	DDX_Control(pDX, IDC_CHK_11, m_chkModule[10]);
	DDX_Control(pDX, IDC_CHK_12, m_chkModule[11]);
	DDX_Control(pDX, IDC_CHK_13, m_chkModule[12]);
	DDX_Control(pDX, IDC_CHK_14, m_chkModule[13]);
	DDX_Control(pDX, IDC_CHK_15, m_chkModule[14]);
	DDX_Control(pDX, IDC_CHK_16, m_chkModule[15]);
	DDX_Control(pDX, IDC_CHK_17, m_chkModule[16]);
	DDX_Control(pDX, IDC_CHK_18, m_chkModule[17]);
	DDX_Control(pDX, IDC_CHK_19, m_chkModule[18]);
	DDX_Control(pDX, IDC_CHK_20, m_chkModule[19]);
	DDX_Control(pDX, IDC_CHK_21, m_chkModule[20]);
	DDX_Control(pDX, IDC_CHK_22, m_chkModule[21]);
	DDX_Control(pDX, IDC_CHK_23, m_chkModule[22]);
	DDX_Control(pDX, IDC_CHK_24, m_chkModule[23]);
	DDX_Control(pDX, IDC_CHK_25, m_chkModule[24]);
	DDX_Control(pDX, IDC_CHK_26, m_chkModule[25]);
	DDX_Control(pDX, IDC_CHK_27, m_chkModule[26]);
	DDX_Control(pDX, IDC_CHK_28, m_chkModule[27]);
	DDX_Control(pDX, IDC_CHK_29, m_chkModule[28]);
	DDX_Control(pDX, IDC_CHK_30, m_chkModule[29]);
	DDX_Control(pDX, IDC_CHK_31, m_chkModule[30]);
	DDX_Control(pDX, IDC_CHK_32, m_chkModule[31]);
	DDX_Control(pDX, IDC_CHK_33, m_chkModule[32]);
	DDX_Control(pDX, IDC_CHK_34, m_chkModule[33]);
	DDX_Control(pDX, IDC_CHK_35, m_chkModule[34]);
	DDX_Control(pDX, IDC_CHK_36, m_chkModule[35]);
	DDX_Control(pDX, IDC_CHK_37, m_chkModule[36]);
	DDX_Control(pDX, IDC_CHK_38, m_chkModule[37]);
	DDX_Control(pDX, IDC_CHK_39, m_chkModule[38]);
	DDX_Control(pDX, IDC_CHK_40, m_chkModule[39]);
	//20181207 edhan add
	DDX_Control(pDX, IDC_CHK_41, m_chkModule[40]);
	DDX_Control(pDX, IDC_CHK_42, m_chkModule[41]);
	DDX_Control(pDX, IDC_CHK_43, m_chkModule[42]);
	DDX_Control(pDX, IDC_CHK_44, m_chkModule[43]);
	DDX_Control(pDX, IDC_CHK_45, m_chkModule[44]);
	DDX_Control(pDX, IDC_CHK_46, m_chkModule[45]);
	//
	DDX_Check(pDX, IDC_CHK_1, m_nNumChk);
	DDX_Control(pDX, IDC_LIST_VIEW2, m_ListBox2);
}

BEGIN_MESSAGE_MAP(CFFUMonDlg, CDialog)
	//{{AFX_MSG_MAP(CFFUMonDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_TRAYICON_MSG, TrayIconMSG)
	ON_COMMAND(ID_MENUITEM_EXIT, OnMenuitemExit)
	ON_COMMAND(ID_MENUITEM_OPEN, OnMenuitemOpen)
	ON_COMMAND(ID_MENUITEM_WINDOWSTARTAUTORUN, OnMenuitemWindowstartautorun)
	ON_MESSAGE(UWM_RCVDATA, OnRcvSerialData)
	ON_REGISTERED_MESSAGE(g_uShellReset, OnTrayReset)	//	yj.k 20180201 add explorer shotdown -> restart -> TrayIcon Retry show
	ON_BN_CLICKED(IDC_BUTTON_OPEN, OnButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, OnButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_SEND, OnButtonSend)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, OnButtonSave)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_RPM, OnButtonRpm)
	ON_BN_CLICKED(IDC_BUTTON_STATUS, OnButtonStatus)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_STATUS2, OnButtonStatus2)
	ON_CBN_SELCHANGE(IDC_COMBO_PORT, OnSelchangeComboPort)
	ON_BN_CLICKED(IDC_BUTTON_RESTORE_VALUE, OnButtonRestoreValue)
	ON_BN_CLICKED(IDC_BUTTON_ALL_OFF, OnButtonAllOff)
	ON_BN_CLICKED(IDC_BUTTON_ALL_SHOW, OnButtonAllShow)
	ON_BN_CLICKED(IDC_BUTTON_GETVALUE, OnButtonGetvalue)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, OnButtonClear)
	ON_EN_CHANGE(IDC_EDIT_CMD, OnChangeEditCmd)
	ON_BN_CLICKED(IDC_MINI, OnMini)
	ON_BN_CLICKED(IDC_CHK_1, &CFFUMonDlg::OnButtonChk1)
	ON_BN_CLICKED(IDC_CHK_2, &CFFUMonDlg::OnButtonChk2)
	ON_BN_CLICKED(IDC_CHK_3, &CFFUMonDlg::OnButtonChk3)
	ON_BN_CLICKED(IDC_CHK_4, &CFFUMonDlg::OnButtonChk4)
	ON_BN_CLICKED(IDC_CHK_5, &CFFUMonDlg::OnButtonChk5)
	ON_BN_CLICKED(IDC_CHK_6, &CFFUMonDlg::OnButtonChk6)
	ON_BN_CLICKED(IDC_CHK_7, &CFFUMonDlg::OnButtonChk7)
	ON_BN_CLICKED(IDC_CHK_8, &CFFUMonDlg::OnButtonChk8)
	ON_BN_CLICKED(IDC_CHK_9, &CFFUMonDlg::OnButtonChk9)
	ON_BN_CLICKED(IDC_CHK_10, &CFFUMonDlg::OnButtonChk10)
	ON_BN_CLICKED(IDC_CHK_11, &CFFUMonDlg::OnButtonChk11)
	ON_BN_CLICKED(IDC_CHK_12, &CFFUMonDlg::OnButtonChk12)
	ON_BN_CLICKED(IDC_CHK_13, &CFFUMonDlg::OnButtonChk13)
	ON_BN_CLICKED(IDC_CHK_14, &CFFUMonDlg::OnButtonChk14)
	ON_BN_CLICKED(IDC_CHK_15, &CFFUMonDlg::OnButtonChk15)
	ON_BN_CLICKED(IDC_CHK_16, &CFFUMonDlg::OnButtonChk16)
	ON_BN_CLICKED(IDC_CHK_17, &CFFUMonDlg::OnButtonChk17)
	ON_BN_CLICKED(IDC_CHK_18, &CFFUMonDlg::OnButtonChk18)
	ON_BN_CLICKED(IDC_CHK_19, &CFFUMonDlg::OnButtonChk19)
	ON_BN_CLICKED(IDC_CHK_20, &CFFUMonDlg::OnButtonChk20)
	ON_BN_CLICKED(IDC_CHK_21, &CFFUMonDlg::OnButtonChk21)
	ON_BN_CLICKED(IDC_CHK_22, &CFFUMonDlg::OnButtonChk22)
	ON_BN_CLICKED(IDC_CHK_23, &CFFUMonDlg::OnButtonChk23)
	ON_BN_CLICKED(IDC_CHK_24, &CFFUMonDlg::OnButtonChk24)
	ON_BN_CLICKED(IDC_CHK_25, &CFFUMonDlg::OnButtonChk25)
	ON_BN_CLICKED(IDC_CHK_26, &CFFUMonDlg::OnButtonChk26)
	ON_BN_CLICKED(IDC_CHK_27, &CFFUMonDlg::OnButtonChk27)
	ON_BN_CLICKED(IDC_CHK_28, &CFFUMonDlg::OnButtonChk28)
	ON_BN_CLICKED(IDC_CHK_29, &CFFUMonDlg::OnButtonChk29)
	ON_BN_CLICKED(IDC_CHK_30, &CFFUMonDlg::OnButtonChk30)
	ON_BN_CLICKED(IDC_CHK_31, &CFFUMonDlg::OnButtonChk31)
	ON_BN_CLICKED(IDC_CHK_32, &CFFUMonDlg::OnButtonChk32)
	ON_BN_CLICKED(IDC_CHK_33, &CFFUMonDlg::OnButtonChk33)
	ON_BN_CLICKED(IDC_CHK_34, &CFFUMonDlg::OnButtonChk34)
	ON_BN_CLICKED(IDC_CHK_35, &CFFUMonDlg::OnButtonChk35)
	ON_BN_CLICKED(IDC_CHK_36, &CFFUMonDlg::OnButtonChk36)
	ON_BN_CLICKED(IDC_CHK_37, &CFFUMonDlg::OnButtonChk37)
	ON_BN_CLICKED(IDC_CHK_38, &CFFUMonDlg::OnButtonChk38)
	ON_BN_CLICKED(IDC_CHK_39, &CFFUMonDlg::OnButtonChk39)
	ON_BN_CLICKED(IDC_CHK_40, &CFFUMonDlg::OnButtonChk40)
	ON_BN_CLICKED(IDC_CHK_41, &CFFUMonDlg::OnButtonChk41)
	ON_BN_CLICKED(IDC_CHK_42, &CFFUMonDlg::OnButtonChk42)
	ON_BN_CLICKED(IDC_CHK_43, &CFFUMonDlg::OnButtonChk43)
	ON_BN_CLICKED(IDC_CHK_44, &CFFUMonDlg::OnButtonChk44)
	ON_BN_CLICKED(IDC_BUTTON_GET_SPEED, &CFFUMonDlg::OnBnClickedButtonGetSpeed)
	ON_BN_CLICKED(IDC_BUTTON_GET_STATE, &CFFUMonDlg::OnBnClickedButtonGetState)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CHK_EFU, &CFFUMonDlg::OnBnClickedChkEfu)
	ON_BN_CLICKED(IDC_CHK_45, &CFFUMonDlg::OnBnClickedChk45)
	ON_BN_CLICKED(IDC_CHK_46, &CFFUMonDlg::OnBnClickedChk46)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFFUMonDlg message handlers

BOOL CFFUMonDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here


	//yhjoo 20141118 add
	m_edtCMD.SetLimitText(2);

	font.CreateFont(14,10,0,0,0,0,0,0,0,0,0,0,0,"Arial");
	GetDlgItem(IDC_STATIC_STATE)->SetFont(&font);
	SetDlgItemText(IDC_STATIC_STATE,"H2O");

	UINT i=0, index = 0;;
	m_ledConnected.SetBkColor(CLR_GRAY);
	for (i=0; i<dfMAX_USE_CONNECT; i++)
	{
		m_chkModule[i].SetFlat(FALSE);	m_chkModule[i].SetColor(CButtonST::BTNST_COLOR_BK_IN, CLR_GREEN);	
	}

	for (i=0; i<dfMAX_THERMETER; i++)
	{
		m_bUseTherm[i] = FALSE;
		m_uiPort485[i] = 1;
		m_uThermometerSub[i] = 1;
	}

	ZeroMemory(&m_dThermometer, sizeof(m_dThermometer));

	for (i=0; i<dfMAX_H2O; i++)
	{
		m_bUse_H2O[i] = FALSE;
		m_dH2O[i] = 0.0;
	}

	for (i=0; i<10; i++)
	{
		m_uiPortSensor[i] = 1;
		m_bUseSensor[i]	= FALSE;
	}
	
	for (i=0; i<2; i++)
	{
		m_uiPortEFU[i] = 1;
	}
	
	m_dO2 = 0.0;
	m_dOldO2 = -9999;
// 	m_dTemp1 = 0.0;
// 	m_dTemp2 = 0.0;	
// 	m_dTemp3 = 0.0;
	m_dPressure = 0.0;
	//ksk 20150608 change
//	memset(&m_dEFU,0.0,sizeof(m_dEFU));
	memset(&m_dEFU, 0, sizeof(double) * 160);		//20181207 edhan change 32 to 160
	memset(&m_dEFU_old, 0, sizeof(double) * 160);	//ksk 20150708 ADD

	m_bUse_EFU = FALSE;
	m_bUse_Ionizer = FALSE;
	m_bSetSendData = FALSE;

	m_nPort = 0;

	m_uiPort = 0;
	m_uiBaudRate = 0;
	m_uiBaudRate8 = 115200;
	m_uiParity = 0;
	m_uiDataBits = 8;
	m_uiStopBit  = 2;

	//ksk 20150628 add
	
	m_dO2_old = 0.;
	m_dPressure_old = 0.;

	//jhseo 20170920 add for KM-N2
	for(i = 0; i < 2; i++)
	{
		m_dKMN2Result[i] = 0.0;
	}
	//////////////////////////////////////////////////////////////////////////

	m_bSocketConnect = FALSE; //20150604 added 

	m_spRPM.SetRange(0,1000);	//yhjoo 20141209 change 140 -> 1000
	UDACCEL accels[] = {{0,5}};
	m_spRPM.SetAccel(1, accels);
	
	SetCboData();
	GetSettingFile();

	CString strName;
	for (i=0; i<dfMAX_USE_CONNECT; i++)
	{
		m_chkModule[i].SetWindowText(_T("O2"));
	}

	for (i=0; i<dfMAX_H2O; i++)
	{
		strName.Format("%s", m_szH2O[i] );
		m_chkModule[SENSOR_MODE_H2O+i].SetWindowText(strName);
	}

	for (i=0; i<dfMAX_THERMETER; i++)
	{
		strName.Format("%s", m_szThermometer[i] );
		m_chkModule[SENSOR_MODE_TEMP1+i].SetWindowText(strName);
	}

	m_chkModule[SENSOR_MODE_EFU].SetWindowText(_T("EFU"));

	m_chkModule[SENSOR_MODE_EFU2].SetWindowText(_T("EFU2"));

	//20181207 edhan add
	m_chkModule[SENSOR_MODE_EFU3].SetWindowText(_T("EFU3"));

	m_chkModule[SENSOR_MODE_EFU4].SetWindowText(_T("EFU4"));

	m_chkModule[SENSOR_MODE_EFU5].SetWindowText(_T("EFU5"));
	//

	m_chkModule[SENSOR_MODE_ESD].SetWindowText(_T("ESD"));
	m_chkModule[SENSOR_MODE_ESD2].SetWindowText(_T("ESD"));
	m_chkModule[SENSOR_MODE_ESD3].SetWindowText(_T("ESD"));

	m_chkModule[SENSOR_MODE_ACCURA].SetWindowText(_T("ACCURA"));

	m_chkModule[SENSOR_MODE_ACCURA2].SetWindowText(_T("ACCURA"));

	m_chkModule[SENSOR_MODE_PM5500].SetWindowText(_T("PM5500"));

	m_chkModule[SENSOR_MODE_AZBIL].SetWindowText(_T("AZBIL"));

	m_chkModule[SENSOR_MODE_IONIZER].SetWindowText(_T("IONIZER"));

	if( m_pWarnMsgDlg == NULL)
	{
		m_pWarnMsgDlg = new CDlgWarnMsg;						//20140729 added by Jay Hong
		m_pWarnMsgDlg->Create(IDD_DIALOG_WARN, this);
		m_pWarnMsgDlg->ShowWindow(SW_HIDE);
	}

	for (i=0; i<10; i++)
	{
		if( m_bUseSensor[i])
		{
			if (m_pDlgComm[i] == NULL)
			{
				m_pDlgComm[i] = new CDlgComm(this);

				m_pDlgComm[i]->m_nMode = i;
				m_pDlgComm[i]->m_nNum  = 0;
				m_pDlgComm[i]->m_cEndDelimiter = m_cEndDelimiter;	//yhjoo 20141118 add

				m_pDlgComm[i]->Create(IDD_DIALOG_COMM,this);
				m_pDlgComm[i]->SetWindowPos(NULL,14,60,0,0,SWP_NOSIZE);
				m_pDlgComm[i]->ShowWindow(SW_HIDE);
			}

			if (m_pDlgComm[i] != NULL)
				m_pDlgComm[i]->OpenPort(m_uiPortSensor[i], m_uiBaudRate, m_uiDataBits, m_uiParity, m_uiStopBit,SENSOR_MODE_O2+i);
		}
	}

	/////////////////////////////////////////////////

	//yhjoo 20141118 add
	for(i=0; i<dfMAX_H2O; i++)
	{
		if( m_bUse_H2O[i] )
		{
			if (m_pDlgComm[SENSOR_MODE_H2O+i] == NULL)
			{
				m_pDlgComm[SENSOR_MODE_H2O+i] = new CDlgComm(this);
				m_pDlgComm[SENSOR_MODE_H2O+i]->Create(IDD_DIALOG_COMM,this);
				m_pDlgComm[SENSOR_MODE_H2O+i]->SetWindowPos(NULL,14,60,0,0,SWP_NOSIZE);
				m_pDlgComm[SENSOR_MODE_H2O+i]->ShowWindow(SW_HIDE);

				m_pDlgComm[SENSOR_MODE_H2O+i]->m_nMode = SENSOR_MODE_H2O;
				m_pDlgComm[SENSOR_MODE_H2O+i]->m_nNum  = i;
				m_pDlgComm[SENSOR_MODE_H2O+i]->m_cEndDelimiter = m_cEndDelimiter;	//yhjoo 20141118 add
			}

			if (m_pDlgComm[SENSOR_MODE_H2O+i] != NULL)
				m_pDlgComm[SENSOR_MODE_H2O+i]->OpenPort(m_uiPortH2O[i], m_uiBaudRate, m_uiDataBits, m_uiParity, m_uiStopBit,SENSOR_MODE_H2O);
		}
	}

	for(i=0; i<dfMAX_THERMETER; i++)
	{
		if (m_bUseTherm[i])
		{
			if (m_pDlgComm[SENSOR_MODE_TEMP1+i] == NULL)
			{
				m_pDlgComm[SENSOR_MODE_TEMP1+i] = new CDlgComm(this);
				m_pDlgComm[SENSOR_MODE_TEMP1+i]->Create(IDD_DIALOG_COMM,this);
				m_pDlgComm[SENSOR_MODE_TEMP1+i]->SetWindowPos(NULL,14,60,0,0,SWP_NOSIZE);
				m_pDlgComm[SENSOR_MODE_TEMP1+i]->ShowWindow(SW_HIDE);

				m_pDlgComm[SENSOR_MODE_TEMP1+i]->m_nMode = SENSOR_MODE_TEMP1;
				m_pDlgComm[SENSOR_MODE_TEMP1+i]->m_nNum  = i;
				m_pDlgComm[SENSOR_MODE_TEMP1+i]->m_nMaxChannel  = m_uThermometerSub[i];
				m_pDlgComm[SENSOR_MODE_TEMP1+i]->m_cEndDelimiter = m_cEndDelimiter;	//yhjoo 20141118 add
			}

			if (m_pDlgComm[SENSOR_MODE_TEMP1+i] != NULL)
				m_pDlgComm[SENSOR_MODE_TEMP1+i]->OpenPort(m_uiPort485[i], m_uiBaudRate, m_uiDataBits, m_uiParity, m_uiStopBit,SENSOR_MODE_TEMP1);
		}
	}
	//20181207 edhan change
	for(i=0; i<5; i++)
	{
		if (m_bUse_EFU)
		{
			Sleep(10);
			if( m_uiPortEFU[i] != 0 && m_bUseEfuNum[i])
			{
				if (m_pDlgComm[SENSOR_MODE_EFU+i] == NULL)
				{
					m_pDlgComm[SENSOR_MODE_EFU+i] = new CDlgComm(this);
					m_pDlgComm[SENSOR_MODE_EFU+i]->m_nMode = SENSOR_MODE_EFU + i;
					m_pDlgComm[SENSOR_MODE_EFU+i]->Create(IDD_DIALOG_COMM,this);
					m_pDlgComm[SENSOR_MODE_EFU+i]->SetWindowPos(NULL,14,60,0,0,SWP_NOSIZE);
					m_pDlgComm[SENSOR_MODE_EFU+i]->ShowWindow(SW_HIDE);

					m_pDlgComm[SENSOR_MODE_EFU+i]->m_nNum  = i;
					m_pDlgComm[SENSOR_MODE_EFU+i]->m_cEndDelimiter = m_cEndDelimiter;	//yhjoo 20141118 add
				}

				if (m_pDlgComm[SENSOR_MODE_EFU+i] != NULL)
					m_pDlgComm[SENSOR_MODE_EFU+i]->OpenPort(m_uiPortEFU[i], m_uiBaudRate, m_uiDataBits, m_uiParity, m_uiStopBit,SENSOR_MODE_EFU + i);
			}
		}
	}


	if( m_bUse_AZBIL )
	{
		index = SENSOR_MODE_AZBIL;
		if (m_pDlgComm[index] == NULL)
		{
			m_pDlgComm[index] = new CDlgComm(this);
			m_pDlgComm[index]->m_nMode = index;
			m_pDlgComm[index]->Create(IDD_DIALOG_COMM,this);
			m_pDlgComm[index]->SetWindowPos(NULL,14,60,0,0,SWP_NOSIZE);
			m_pDlgComm[index]->ShowWindow(SW_HIDE);

			m_pDlgComm[index]->m_nNum  = 0;
			m_pDlgComm[index]->m_cEndDelimiter = m_cEndDelimiter;	//yhjoo 20141118 add
		}

		if (m_pDlgComm[index] != NULL)
			m_pDlgComm[index]->OpenPort(m_stSerial[index].m_uiPort, m_stSerial[index].m_uiBaudRate, m_stSerial[index].m_uiDataBits, m_stSerial[index].m_uiParity, m_stSerial[index].m_uiStopBit,index);
	}

	if( m_bUse_PM5500 )
	{
		index = SENSOR_MODE_PM5500;
		if (m_pDlgComm[index] == NULL)
		{
			m_pDlgComm[index] = new CDlgComm(this);
			m_pDlgComm[index]->m_nMode = index;
			m_pDlgComm[index]->Create(IDD_DIALOG_COMM,this);
			m_pDlgComm[index]->SetWindowPos(NULL,14,60,0,0,SWP_NOSIZE);
			m_pDlgComm[index]->ShowWindow(SW_HIDE);

			m_pDlgComm[index]->m_nNum  = 0;
			m_pDlgComm[index]->m_cEndDelimiter = m_cEndDelimiter;	//yhjoo 20141118 add
		}

		if (m_pDlgComm[index] != NULL)
			m_pDlgComm[index]->OpenPort(m_stSerial[index].m_uiPort, m_stSerial[index].m_uiBaudRate, m_stSerial[index].m_uiDataBits, m_stSerial[index].m_uiParity, m_stSerial[index].m_uiStopBit,index);
	}

	if( m_bUse_Ionizer )
	{
		for (i=0; i<2; i++)
		{
			index = SENSOR_MODE_IONIZER + i;

			if( m_stSerial[index].m_uiPort > 0)
			{
				if (m_pDlgComm[index] == NULL)
				{
					m_pDlgComm[index] = new CDlgComm(this);
					m_pDlgComm[index]->m_nMode = index;
					m_pDlgComm[index]->Create(IDD_DIALOG_COMM,this);
					m_pDlgComm[index]->SetWindowPos(NULL,14,60,0,0,SWP_NOSIZE);
					m_pDlgComm[index]->ShowWindow(SW_HIDE);

					m_pDlgComm[index]->m_nNum  = 0;
					m_pDlgComm[index]->m_cEndDelimiter = m_cEndDelimiter;	//yhjoo 20141118 add
				}
			}
		
			if (m_pDlgComm[index] != NULL)
				m_pDlgComm[index]->OpenPort(m_stSerial[index].m_uiPort, m_stSerial[index].m_uiBaudRate, m_stSerial[index].m_uiDataBits, m_stSerial[index].m_uiParity, m_stSerial[index].m_uiStopBit,index);
		}
	}

	//
	//lhj 20171223 add /////////////////////////////////////////////////
	CRect rect;
	GetDlgItem(IDC_STATIC_ESD)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_ESD)->GetWindowRect(&rect);
	ScreenToClient(rect);

	if (m_bUse_ESD == TRUE)
	{
		for (i=0; i<m_nCnt_ESD; i++)
		{	
			if( m_pDlgESD[i] == NULL)
			{
				m_pDlgESD[i] = new CDlgESD(this);
				m_pDlgESD[i]->SetMainPoint(this);
				m_pDlgESD[i]->Create(IDD_DLG_ESD2,this);
				m_pDlgESD[i]->MoveWindow(rect);
				m_pDlgESD[i]->ShowWindow(SW_SHOW);
			}
		}
	}
	
	// 	m_dlgESD.SetMainPoint(this);
	// 	m_dlgESD.Create(IDD_DLG_ESD2,this);
	// 	m_dlgESD.MoveWindow(rect);
	// 	m_dlgESD.ShowWindow(SW_SHOW);
	///////////////////////////////////////////////////////////////////


	//lhj 20171223 add
	if (m_bUse_ESD)
	{
		// 소켓 통신 연결 
		for(i=0; i<m_nCnt_ESD; i++)
		{
			if( m_pDlgESD[i] != NULL)
			{
				m_pDlgESD[i]->SetType(m_nType_ESD);

				if( i < (UINT)m_strIPESD.GetSize() )
				{
					m_pDlgESD[i]->SetConnectInfo(m_strIPESD.GetAt(i), m_strPortESD, m_stSerial[SENSOR_MODE_ESD+i]);
				}
				
				m_pDlgESD[i]->ConnectESD(SENSOR_MODE_ESD+i, 10+i);
			}

		}

 //		m_dlgESD.SocketConnectESD(m_strIPESD,m_strPortESD);

		if (m_pThreadESDRead == NULL) 
		{
			m_pThreadESDRead = AfxBeginThread(ThreadESDReadProc,this);
		}
	}
	//
	//////////////////////////////////////////////////////////////////////////

	if(m_bUse_ACCURA)        //jh.lee 20160722 add
	{		 
		StartThreadGetsData();		 
	}
	//////////////////////////////////////////////

	////////////////////	
// 	// H2O 
// 	if( m_bUse_H2O )
// 	{
// 		if( api_SerialCom_IsOpened(m_uiPort) )
// 		{
// 			StartThreadGetState();
// 			SetSendData();
// 		}
// 	}
	
	//20140729 added by Jay Hong
	if(m_nLocalAlarm)
	{
		//20150604 removed
		//SetTimer(100, 1000, NULL);
	}

	ResTrayIcon();  

	CRect r2;
	CString strLabal;
	int nMoveH = 0, nCnt = 0, nMoveW = 0;
	GetDlgItem(IDC_CHK_1)->GetWindowRect(r2);
	ScreenToClient(&r2);
	nMoveH = r2.Height()+10;
	nMoveW = r2.Width()+10;
	
	BOOL bUse[dfMAX_USE_CONNECT];
	ZeroMemory(&bUse, sizeof(bUse));

	for (i=0; i<10; i++)
	{
		bUse[i] =  m_bUseSensor[i];
	}

	for(i=0; i<dfMAX_H2O; i++)
	{
		bUse[SENSOR_MODE_H2O+i] = m_bUse_H2O[i];
	}

	for(i=0; i<dfMAX_THERMETER; i++)
	{
		bUse[SENSOR_MODE_TEMP1+i] = m_bUseTherm[i];
	}

	for(i = 0; i < 5; i++)
	{
		if(m_bUseEfuNum[i] == TRUE)
			bUse[SENSOR_MODE_EFU + i]		= m_bUse_EFU;
	}

	bUse[SENSOR_MODE_ESD]		= m_bUse_ESD;
	bUse[SENSOR_MODE_ESD2]		= m_bUse_ESD;
	bUse[SENSOR_MODE_ESD3]		= m_bUse_ESD;

	bUse[SENSOR_MODE_ACCURA]	= m_bUse_ACCURA;

	bUse[SENSOR_MODE_PM5500]	= m_bUse_PM5500;
	bUse[SENSOR_MODE_AZBIL]	    = m_bUse_AZBIL;
	bUse[SENSOR_MODE_IONIZER]	= m_bUse_Ionizer;

	int nCntW = 0; 
	for (i=0; i<dfMAX_USE_CONNECT; i++)
	{
		if(bUse[i] == TRUE)
		{
			GetDlgItem(IDC_CHK_1+i)->ShowWindow(SW_SHOW);
			rect = r2;
			if( nCnt > 7)
			{
				nCnt = 0;
				nCntW = 1;
			}
		
			rect.top    += (nMoveH*nCnt);
			rect.bottom += (nMoveH*nCnt);
			
			rect.left   += (nMoveW*nCntW);
			rect.right  += (nMoveW*nCntW);

			GetDlgItem(IDC_CHK_1+i)->MoveWindow(rect);
			GetDlgItem(IDC_CHK_1)->GetWindowRect(r2);
			ScreenToClient(&r2);

			nMoveH = r2.Height()+10;
			nCnt++;
		}
		else
		{
			GetDlgItem(IDC_CHK_1+i)->ShowWindow(SW_HIDE);
		}
	}	

	if(bUse[SENSOR_MODE_TEMP1] != TRUE )
	{
		m_nMode = SENSOR_MODE_ACCURA;	//SENSOR_MODE_H2O;
		UpdateBtn(SENSOR_MODE_ACCURA);	// Base UI
	}
	else
	{
		m_nMode = SENSOR_MODE_TEMP1;	//SENSOR_MODE_H2O;
		UpdateBtn(SENSOR_MODE_TEMP1);	// Base UI
	}
	

	//20150604 changed
	ConnectTCP();

	m_bTimer = FALSE;

////	SetTimer(10,  1000, NULL);
//	SetTimer(102, 1000, NULL);	// jsyoon 20150129 add UI Update
//	SetTimer(103, 10,   NULL);

	//SetTimer(104, 3000, NULL);

	SetTimer (TIMER_GET_DATA, 100, NULL);
	SetTimer(102, 1000, NULL);

	g_uShellReset	=	RegisterWindowMessage(_T("TaskbarCreated"));	//	yj.k 20180201 add explorer shotdown -> restart -> TrayIcon Retry show


	if (m_bEFU_Type == EFU_TYPE_HKC_H5)
		m_pDlgComm[SENSOR_MODE_EFU]->TestSendEFU(0, 1, 1, m_nEFUCnt[0], 1);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CFFUMonDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else if((nID & 0xFFF0) == SC_MINIMIZE)		//sypark 20180525 add start
	{
		ShowWindow(SW_HIDE);
	}
	else if((nID & 0xFFF0) == SC_CLOSE)			//jsyoon 20180725 add
	{
//		ShowWindow(SW_HIDE);
		if( FFUMonDlgExit() == true )
			CDialog::OnOK();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CFFUMonDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CFFUMonDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CFFUMonDlg::ExitThreadAll()
{
	DWORD dwExitCode;
	BOOL bRes = FALSE;
// 	if(m_pRcvThread != NULL)
// 	{
// 		bRes = GetExitCodeThread(m_pRcvThread->m_hThread, &dwExitCode);
// 		if(bRes || dwExitCode == STILL_ACTIVE)
// 		{
// 			SetEvent(m_hEvtRcvStop);
// 			ExitThread(m_pRcvThread->m_hThread, 2000);
// 			CloseHandle(m_hEvtRcvStop);
// 		}
// 	}

	if(m_pGetsDataThread !=NULL )
	{
		bRes = GetExitCodeThread(m_pGetsDataThread->m_hThread, &dwExitCode);
		if(bRes || dwExitCode == STILL_ACTIVE)
		{
			SetEvent(m_hEvtGetsDataStop);
			ExitThread(m_pGetsDataThread->m_hThread, 2000);
		}
	}
}

BOOL CFFUMonDlg::ExitThread(HANDLE hThread, DWORD dwDelay)
{
	DWORD dwTick = GetTickCount();
	while(TRUE)
	{
		if(GetTickCount() - dwTick >= dwDelay)
		{
			TerminateThread(hThread, 0);
			return FALSE;
		}
		if(WaitForSingleObject(hThread, 0) != WAIT_TIMEOUT)
			return TRUE;
		
		MSG msg;
		while(::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}
}

void CFFUMonDlg::SetCboData()
{
	int i = 0;
	CString strParam;

	// 1. Port.
	for(i = 0; i < 30; i++)
	{
		strParam.Format(_T("%d"), i+1);
		m_cboPort.AddString(strParam);
	}
	m_cboPort.SetCurSel(0); // default setting.

	// 2. Baud Rate.
	CString strParamArray[] = {_T("9600"), _T("19200"), _T("38400"), _T("57600"), _T("115200")};
	for(i = 0; i < (sizeof(strParamArray)/sizeof(strParamArray[0])); i++)
		m_cboBaudRate.AddString(strParamArray[i]);
	m_cboBaudRate.SetCurSel(0);

	// 3. Parity
	CString strPrity[] = {_T("None"), _T("Odd"), _T("Even"), _T("Mark"), _T("Space")};
	for(i = 0; i < (sizeof(strPrity)/sizeof(strPrity[0])); i++)
		m_cboParity.AddString(strPrity[i]);
	m_cboParity.SetCurSel(0);

	// 4. Data Bits
	CString strDataBits[] = {_T("5"), _T("6"), _T("7"), _T("8")};
	for(i = 0; i < (sizeof(strDataBits)/sizeof(strDataBits[0])); i++)
		m_cboDataBit.AddString(strDataBits[i]);
	m_cboDataBit.SetCurSel(3);

	// 5. Stop Bits
	CString strStopBits[] = {_T("ONESTOPBIT"), _T("ONE5STOPBITS"), _T("TWOSTOPBITS")};
	for(i = 0; i < (sizeof(strStopBits)/sizeof(strStopBits[0])); i++)
		m_cboStopBit.AddString(strStopBits[i]);
	m_cboStopBit.SetCurSel(0);

	// 6. Start Delimeter
	CString strDelimiter[] = {_T("None"), _T("STX"), _T("ETX"), _T("CR"), _T("LF"), _T("CRLF")};

	for(i = 0; i < (sizeof(strDelimiter)/sizeof(strDelimiter[0])); i++)
		m_cboStartDelimiter.AddString(strDelimiter[i]);
	m_cboStartDelimiter.SetCurSel(1);

	// 7. End Delimeter
	for(i = 0; i < (sizeof(strDelimiter)/sizeof(strDelimiter[0])); i++)
		m_cboEndDelimiter.AddString(strDelimiter[i]);
	m_cboEndDelimiter.SetCurSel(2);


	CString strBLID;
	for(i = 1; i<=32; i++)
	{
		strBLID.Format(_T("%d"), i);
//		m_cboBLIDStart.AddString(strBLID);		//20181207 edhan remove
//		m_cboBLIDEnd.AddString(strBLID);
		m_cboLVID.AddString(strBLID);
	}
	//m_cboBLIDStart.SetCurSel(0);
	//m_cboBLIDEnd.SetCurSel(0);
	m_cboLVID.SetCurSel(0);
}

void CFFUMonDlg::GetSettingFile()
{
	UINT i=0, j=0;
	CString strTemp;
	CWinApp* pApp = ::AfxGetApp(); 
	TCHAR szBuff[MAX_PATH];
	::GetModuleFileName(pApp->m_hInstance, szBuff, MAX_PATH);
	strTemp = szBuff;
	int iLen = 0, iLen2 = 0, iLen3 = 0;
	iLen =	strTemp.ReverseFind('\\');
	CString strAppPath = strTemp.Mid(0, iLen+1);

	iLen2 =	strTemp.ReverseFind('_')+1;
	iLen3 = strTemp.GetLength();
	if( iLen2 > iLen )
	{
		if( iLen2 > 0)
		{
			iLen =	strTemp.ReverseFind('.');
			m_strEQName = strTemp.Mid(iLen2, iLen - iLen2);
		}
		else
			m_strEQName.Empty();
	}
	else
	{
		m_strEQName.Empty();
	}
	
	CString strConfigFile = _T("");
	strConfigFile =	strAppPath + _T("\\Config\\Setting.ini");
	m_strConfigFile =strConfigFile;

	CString strPort = _T("");
	CString strUse = _T("");	//yhjoo 20141120 add
	CString strBaudRate = _T("");
	CString strParity = _T("");
	CString strDataBit = _T("");
	CString strStopBit = _T("");
	CString strStartDelimiter = _T("");
	CString strEndDelimiter = _T("");
	CString strEFUCnt = _T("");
	CString strName = _T("");
	CString strSetName = _T("");
	CString strEFUEnable = _T("");

	//20140729 added by Jay Hong
	CString strLocalAlarm =  _T("");

	// jsyoon 20150129 add
	strSetName = _T("SYSTEMINFO");
	m_strLogPath = GetFileValue(strSetName, _T("LOGPATH"), strConfigFile);
	gf_MakeDir(m_strLogPath);
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	//yhjoo 20141120 add
	strSetName = _T("TCPSET");

	strUse = GetFileValue(strSetName, _T("USE_FDC"), strConfigFile);	//yhjoo 20160704 add
	m_bUse_FDC = _ttoi(strUse);

	m_strIP = GetFileValue(strSetName, _T("IP"), strConfigFile);

	strPort = GetFileValue(strSetName, _T("PORT"), strConfigFile);
	m_nPort = _ttoi(strPort);
	///////////////////////////////////////

	//yhjoo 20141118 add
	strSetName = _T("USESET");
	for (i=0; i<dfMAX_H2O; i++)
	{
		strName.Format("USE_H2O_%d", i+1);
		strPort = GetFileValue(strSetName, strName, strConfigFile);
		m_bUse_H2O[i] = _ttoi(strPort);

		if( m_bUse_H2O[i] == TRUE)
		{
			strName.Format("NAME_H20_%d", i+1);
			strPort = GetFileValue(strSetName, strName, strConfigFile);
			_tcscpy_s(m_szH2O[i], _countof(m_szH2O[i]), strPort);
		}
	}

	// nk.lee 2020.06.15
	strName = _T("H20_TYPE");
	strTemp = GetFileValue(strSetName, strName, strConfigFile);
	m_iH2OSensorType = _ttoi(strTemp);
	// nk.lee end


	for (i=0; i<dfMAX_THERMETER; i++)
	{
		strName.Format("USE_TEMP_%d", i+1);
		strPort = GetFileValue(strSetName, strName, strConfigFile);
		m_bUseTherm[i] = _ttoi(strPort);

		if( m_bUseTherm[i] == TRUE)
		{
			strName.Format("NAME_TEMP_%d", i+1);
			strPort = GetFileValue(strSetName, strName, strConfigFile);
			_tcscpy_s(m_szThermometer[i], _countof(m_szThermometer[i]), strPort);

			strName.Format("USE_TEMP_SUB_%d", i+1);
			strPort = GetFileValue(strSetName, strName, strConfigFile);
			m_uThermometerSub[i] = _ttoi(strPort);

			for (j=0; j<m_uThermometerSub[i]; j++)
			{
				strName.Format("NAME_TEMP_SUB_%d_%d", i+1, j+1);
				strPort = GetFileValue(strSetName, strName, strConfigFile);
				_tcscpy_s(m_szThermometerSub[i][j], _countof(m_szThermometerSub[i][j]), strPort);
			}
		}
	}

	i=0;
	strUse = GetFileValue(strSetName, _T("USE_O2"), strConfigFile);
	m_bUseSensor[i] = _ttoi(strUse);	i++;

	strUse = GetFileValue(strSetName, _T("USE_PRESSURE"), strConfigFile);
	m_bUseSensor[i] = _ttoi(strUse);	i++;

	strUse = GetFileValue(strSetName, _T("USE_LED"), strConfigFile);
	m_bUseSensor[i] = _ttoi(strUse);	i++;

	strUse = GetFileValue(strSetName, _T("USE_KMN2"), strConfigFile);		//jhseo 20170920 add for KM-N2
	m_bUseSensor[i] = _ttoi(strUse);	i++;

	strUse = GetFileValue(strSetName, _T("USE_ACCURA_485"), strConfigFile);		//jhseo 20170920 add for KM-N2
	m_bUseSensor[i] = _ttoi(strUse);	i++;
	
	
	for (i=0; i<dfMAX_H2O; i++)
	{
		if( m_bUse_H2O[i] == TRUE)
		{
			strName.Format("H2O_PORT_%d", i+1);
			strPort = GetFileValue(strSetName, strName, strConfigFile);
			m_uiPortH2O[i] = _ttoi(strPort);
		}
	}

	for (i=0; i<dfMAX_THERMETER; i++)
	{
		if( m_bUseTherm[i] == TRUE)
		{
			strName.Format("TEMP_PORT_%d", i+1);
			strPort = GetFileValue(strSetName, strName, strConfigFile);
			m_uiPort485[i] = _ttoi(strPort);
		}
	}

	i=0;

	strPort = GetFileValue(strSetName, _T("O2_PORT"), strConfigFile);
	m_uiPortSensor[i] = _ttoi(strPort);	i++;

	strPort = GetFileValue(strSetName, _T("PRESSURE_PORT"), strConfigFile);
	m_uiPortSensor[i] = _ttoi(strPort);	i++;

	strPort = GetFileValue(strSetName, _T("LED_PORT"), strConfigFile);
	m_uiPortSensor[i] = _ttoi(strPort);	i++;

	strPort = GetFileValue(strSetName, _T("KMN2_PORT"), strConfigFile);		//jhseo 20170920 add for KM-N2
	m_uiPortSensor[i] = _ttoi(strPort);	i++;

	strPort = GetFileValue(strSetName, _T("ACCURA_485_PORT"), strConfigFile);		//jhseo 20170920 add for KM-N2
	m_uiPortSensor[i] = _ttoi(strPort);	i++;
	//////////////////////////////////////////////////////////////////////////
	
	//////////////////////////////////////////////////////////////////////////
	// EFU TYPE
	strSetName = _T("EFUSET");

	strUse = GetFileValue(strSetName, _T("USE_EFU"), strConfigFile);
	m_bUse_EFU = _ttoi(strUse);

	strUse = GetFileValue(strSetName, _T("SW_NUM"), strConfigFile);
	m_nIdex = _ttoi(strUse);

	strPort = GetFileValue(strSetName, _T("EFU_PORT"), strConfigFile);
	m_uiPortEFU[0] = _ttoi(strPort);

	//ksk 20161201 add
	strPort = GetFileValue(strSetName, _T("EFU_PORT2"), strConfigFile);
	m_uiPortEFU[1] = _ttoi(strPort);

	//20181207 edhan add
	strPort = GetFileValue(strSetName, _T("EFU_PORT3"), strConfigFile);
	m_uiPortEFU[2] = _ttoi(strPort);

	strPort = GetFileValue(strSetName, _T("EFU_PORT4"), strConfigFile);
	m_uiPortEFU[3] = _ttoi(strPort);

	strPort = GetFileValue(strSetName, _T("EFU_PORT5"), strConfigFile);
	m_uiPortEFU[4] = _ttoi(strPort);

	strEFUEnable = GetFileValue(strSetName, _T("ENABLE_EFU_1"), strConfigFile);
	m_bUseEfuNum[0] = _ttoi(strEFUEnable);

	strEFUEnable = GetFileValue(strSetName, _T("ENABLE_EFU_2"), strConfigFile);
	m_bUseEfuNum[1] = _ttoi(strEFUEnable);

	strEFUEnable = GetFileValue(strSetName, _T("ENABLE_EFU_3"), strConfigFile);
	m_bUseEfuNum[2] = _ttoi(strEFUEnable);

	strEFUEnable = GetFileValue(strSetName, _T("ENABLE_EFU_4"), strConfigFile);
	m_bUseEfuNum[3] = _ttoi(strEFUEnable);

	strEFUEnable = GetFileValue(strSetName, _T("ENABLE_EFU_5"), strConfigFile);
	m_bUseEfuNum[4] = _ttoi(strEFUEnable);

	//

	strUse = GetFileValue(strSetName, _T("EFU_TYPE"), strConfigFile);
	m_bEFU_Type = _ttoi(strUse);

	strBaudRate = GetFileValue(strSetName, _T("BAUDRATE"), strConfigFile);
	m_uiBaudRate = _ttoi(strBaudRate);

	strBaudRate = GetFileValue(strSetName, _T("BAUDRATE2"), strConfigFile);
	m_uiBaudRate8 = _ttoi(strBaudRate);

	strParity = GetFileValue(strSetName, _T("PARITY"), strConfigFile);
	//m_uiParity = _ttoi(strParity);

	strDataBit = GetFileValue(strSetName, _T("DATABIT"), strConfigFile);
	m_uiDataBits = _ttoi(strDataBit);

	strStopBit = GetFileValue(strSetName, _T("STOPBIT"), strConfigFile);
	//m_uiStopBit = _ttoi(strStopBit);

	strStartDelimiter = GetFileValue(strSetName, _T("STARTDELIMITER"), strConfigFile);
	m_cStartDelimiter = GetDelimiter(strStartDelimiter);

	strEndDelimiter = GetFileValue(strSetName, _T("ENDDELIMITER"), strConfigFile);
	m_cEndDelimiter = GetDelimiter(strEndDelimiter);

	strLocalAlarm = GetFileValue(strSetName, _T("SHOW_LOCAL_ALARM"), strConfigFile);
	m_nLocalAlarm = _ttoi(strLocalAlarm);

	m_strPassWord = GetFileValue(strSetName, _T("PASSWORD"), strConfigFile);

	m_strPressureCMD = GetFileValue(strSetName, _T("PRESSURE_CMD"), strConfigFile);//yhjoo 20141209 add PRESSURE Command
	SetDlgItemText(IDC_EDIT_CMD,m_strPressureCMD);

	strPort.Format("%d",m_uiPortEFU[0]);	//yhjoo 20141118 add
	SetcboCurSel(&m_cboPort, strPort);
	SetcboCurSel(&m_cboBaudRate, strBaudRate);
	m_uiParity = SetcboCurSel(&m_cboParity, strParity);
	SetcboCurSel(&m_cboDataBit, strDataBit);
	m_uiStopBit = SetcboCurSel(&m_cboStopBit, strStopBit);
	SetcboCurSel(&m_cboStartDelimiter, strStartDelimiter);
	SetcboCurSel(&m_cboEndDelimiter, strEndDelimiter);

	////////////////////////////////////////////////////////////////////////////////////////////

	strEFUCnt = GetFileValue(strSetName, _T("EFU_COUNT "), strConfigFile);
	m_nEFUCnt[0] = _ttoi(strEFUCnt);
	strEFUCnt = GetFileValue(strSetName, _T("EFU_COUNT2 "), strConfigFile);
	m_nEFUCnt[1] = _ttoi(strEFUCnt);
	//20181207 edhan add
	strEFUCnt = GetFileValue(strSetName, _T("EFU_COUNT3 "), strConfigFile);
	m_nEFUCnt[2] = _ttoi(strEFUCnt);
	strEFUCnt = GetFileValue(strSetName, _T("EFU_COUNT4 "), strConfigFile);
	m_nEFUCnt[3] = _ttoi(strEFUCnt);
	strEFUCnt = GetFileValue(strSetName, _T("EFU_COUNT5 "), strConfigFile);
	m_nEFUCnt[4] = _ttoi(strEFUCnt);
	//
	for(int i = 0; i < 5; i ++)
	{
		if(!m_bUseEfuNum[i])
			m_nEFUCnt[i] = 0;

			m_nEFUAllCnt += m_nEFUCnt[i];
	}
//	m_nEFUAllCnt = m_nEFUCnt[0] + m_nEFUCnt[1] + m_nEFUCnt[2] + m_nEFUCnt[3] + m_nEFUCnt[4];

//	API_SM_SetFFU_NUM(m_nEFUCnt);		// jsyoon 20140625 add
	API_SM_SetFFU_NUM(m_nEFUAllCnt, m_nIdex);		// jsyoon 20140625 add

	char szValue[MAX_PATH];
	CString strKey;
	for(i=0; i<dfMAX_EFU; i++)
	{
		strKey.Format(_T("EFU_SPEED%d"), i+1);
		GetPrivateProfileString (strSetName,strKey,"100",szValue,4,strConfigFile);
		m_uEFUSpeed[i] =  atoi(szValue);
	}

	// jsyoon 20140819 add Max SPEED
	for(i=0; i<dfMAX_EFU; i++)
	{
		strKey.Format(_T("MAX_EFU_SPEED%d"), i+1);
		GetPrivateProfileString (strSetName,strKey,"1000",szValue,5,strConfigFile);
		m_uEFUSpeedMax[i] =  atoi(szValue);
	}
	
	for(i=0; i<m_nEFUAllCnt; i++)
	{
		API_SM_SetFFU_ALARM_CHECK(i,NORMAL_ALARM, m_nIdex);
	}
	//////////////////////////////////////////////////////////////////////////
	
	// ESD
	//lhj 20171223 add /////////////////////////////////////////////////////////////////////////
	strSetName = _T("ESDSET");

	strUse = GetFileValue(strSetName, _T("USE_ESD"), strConfigFile);		
	m_bUse_ESD = _ttoi(strUse);
	strUse = GetFileValue(strSetName, _T("CNT_ESD"), strConfigFile, _T("1"));		
	m_nCnt_ESD = _ttoi(strUse);
	strUse = GetFileValue(strSetName, _T("TYPE_ESD"), strConfigFile, _T("0"));		
	m_nType_ESD = _ttoi(strUse);

	m_strPortESD = GetFileValue(strSetName, _T("ESD_STRING_PORT"), strConfigFile);
	m_strIPESD.RemoveAll();

	strKey.Format(_T("ESD_STRING_IP"));
	strUse = GetFileValue(strSetName, strKey, strConfigFile);

	BOOL bFlag = FALSE;
	CString strSubText;
	i=0;
	while (1)
	{
		bFlag = AfxExtractSubString(strSubText,strUse, i, ',');
		if (bFlag == 0)break;

		m_strIPESD.Add(strSubText);
		i++;
	}

	strPort = GetFileValue(strSetName, _T("PORT"), strConfigFile);		//jhseo 20170920 add for KM-N2
	m_stSerial[SENSOR_MODE_ESD].m_uiPort = _ttoi(strPort);

	strBaudRate = GetFileValue(strSetName, _T("BAUDRATE"), strConfigFile);
	m_stSerial[SENSOR_MODE_ESD].m_uiBaudRate = _ttoi(strBaudRate);

	strParity = GetFileValue(strSetName, _T("PARITY"), strConfigFile);
	m_stSerial[SENSOR_MODE_ESD].m_uiParity  = _ttoi(strParity);

	strDataBit = GetFileValue(strSetName, _T("DATABIT"), strConfigFile);
	m_stSerial[SENSOR_MODE_ESD].m_uiDataBits = _ttoi(strDataBit);

	strStopBit = GetFileValue(strSetName, _T("STOPBIT"), strConfigFile);
	m_stSerial[SENSOR_MODE_ESD].m_uiStopBit = _ttoi(strStopBit);

//	m_strIPESD   = GetFileValue(strSetName, _T("ESD_STRING_IP"), strConfigFile);
	
	//////////////////////////////////////////////////////////////////////////
	
	// jyoon 20180625 add
	// ACCURA
	strSetName = _T("ACCURASET");
	strUse = GetFileValue(strSetName, _T("ACCURA_USE"), strConfigFile);		//jhseo 20170920 add for KM-N2
	m_bUse_ACCURA = _ttoi(strUse);
	
	strUse = GetFileValue(strSetName, _T("ACCURA_COUNT"), strConfigFile);		//jhseo 20170920 add for KM-N2
	m_bCnt_ACCURA = _ttoi(strUse);
	
	if( m_bUse_ACCURA )
	{
		for (i=0; i<m_bCnt_ACCURA; i++)
		{
			strKey.Format(_T("IP_%d"), i+1);
			strUse = GetFileValue(strSetName, strKey, strConfigFile);		//jhseo 20170920 add for KM-N2
			AddDevice(strUse);
		}
	}

	if( m_pAccuraData != NULL)
	{
		delete[] m_pAccuraData;
		m_pAccuraData = NULL;
	}

	if( m_pAccuraData == NULL)
	{
		m_pAccuraData = new ADDR_DATA[m_bCnt_ACCURA];
		ZeroMemory(m_pAccuraData, sizeof(ADDR_DATA)*m_bCnt_ACCURA);
	}
	//////////////////////////////////////////////////////////////////////////

	// AZBIL
	strSetName = _T("AZBIL");
	strUse = GetFileValue(strSetName, _T("USE"), strConfigFile);		//jhseo 20170920 add for KM-N2
	m_bUse_AZBIL = _ttoi(strUse);

	strPort = GetFileValue(strSetName, _T("PORT"), strConfigFile);		//jhseo 20170920 add for KM-N2
	m_stSerial[SENSOR_MODE_AZBIL].m_uiPort = _ttoi(strPort);

	strBaudRate = GetFileValue(strSetName, _T("BAUDRATE"), strConfigFile);
	m_stSerial[SENSOR_MODE_AZBIL].m_uiBaudRate = _ttoi(strBaudRate);

	strParity = GetFileValue(strSetName, _T("PARITY"), strConfigFile);
	m_stSerial[SENSOR_MODE_AZBIL].m_uiParity  = _ttoi(strParity);

	strDataBit = GetFileValue(strSetName, _T("DATABIT"), strConfigFile);
	m_stSerial[SENSOR_MODE_AZBIL].m_uiDataBits = _ttoi(strDataBit);

	strStopBit = GetFileValue(strSetName, _T("STOPBIT"), strConfigFile);
	m_stSerial[SENSOR_MODE_AZBIL].m_uiStopBit = _ttoi(strStopBit);
	//////////////////////////////////////////////////////////////////////////

	// PM5500
	strSetName = _T("PM5500");
	strUse = GetFileValue(strSetName, _T("USE"), strConfigFile);		//jhseo 20170920 add for KM-N2
	m_bUse_PM5500 = _ttoi(strUse);

	strPort = GetFileValue(strSetName, _T("PORT"), strConfigFile);		//jhseo 20170920 add for KM-N2
	m_stSerial[SENSOR_MODE_PM5500].m_uiPort = _ttoi(strPort);

	strBaudRate = GetFileValue(strSetName, _T("BAUDRATE"), strConfigFile);
	m_stSerial[SENSOR_MODE_PM5500].m_uiBaudRate = _ttoi(strBaudRate);

	strParity = GetFileValue(strSetName, _T("PARITY"), strConfigFile);
	m_stSerial[SENSOR_MODE_PM5500].m_uiParity  = _ttoi(strParity);

	strDataBit = GetFileValue(strSetName, _T("DATABIT"), strConfigFile);
	m_stSerial[SENSOR_MODE_PM5500].m_uiDataBits = _ttoi(strDataBit);

	strStopBit = GetFileValue(strSetName, _T("STOPBIT"), strConfigFile);
	m_stSerial[SENSOR_MODE_PM5500].m_uiStopBit = _ttoi(strStopBit);
	//////////////////////////////////////////////////////////////////////////

	// XIONIZER
	strSetName = _T("XIONIZER");
	strUse = GetFileValue(strSetName, _T("USE"), strConfigFile);
	m_bUse_Ionizer = _ttoi(strUse);

	strSetName = _T("XIONIZER1");
	strPort = GetFileValue(strSetName, _T("PORT"), strConfigFile);
	m_stSerial[SENSOR_MODE_IONIZER].m_uiPort = _ttoi(strPort);

	strBaudRate = GetFileValue(strSetName, _T("BAUDRATE"), strConfigFile);
	m_stSerial[SENSOR_MODE_IONIZER].m_uiBaudRate = _ttoi(strBaudRate);

	strParity = GetFileValue(strSetName, _T("PARITY"), strConfigFile);
	m_stSerial[SENSOR_MODE_IONIZER].m_uiParity  = _ttoi(strParity);

	strDataBit = GetFileValue(strSetName, _T("DATABIT"), strConfigFile);
	m_stSerial[SENSOR_MODE_IONIZER].m_uiDataBits = _ttoi(strDataBit);

	strStopBit = GetFileValue(strSetName, _T("STOPBIT"), strConfigFile);
	m_stSerial[SENSOR_MODE_IONIZER].m_uiStopBit = _ttoi(strStopBit);

	strSetName = _T("XIONIZER2");
	strPort = GetFileValue(strSetName, _T("PORT"), strConfigFile);
	m_stSerial[SENSOR_MODE_IONIZER2].m_uiPort = _ttoi(strPort);

	strBaudRate = GetFileValue(strSetName, _T("BAUDRATE"), strConfigFile);
	m_stSerial[SENSOR_MODE_IONIZER2].m_uiBaudRate = _ttoi(strBaudRate);

	strParity = GetFileValue(strSetName, _T("PARITY"), strConfigFile);
	m_stSerial[SENSOR_MODE_IONIZER2].m_uiParity  = _ttoi(strParity);

	strDataBit = GetFileValue(strSetName, _T("DATABIT"), strConfigFile);
	m_stSerial[SENSOR_MODE_IONIZER2].m_uiDataBits = _ttoi(strDataBit);

	strStopBit = GetFileValue(strSetName, _T("STOPBIT"), strConfigFile);
	m_stSerial[SENSOR_MODE_IONIZER2].m_uiStopBit = _ttoi(strStopBit);
}

int CFFUMonDlg::SetcboCurSel(CComboBox *pcbo, CString strValue)
{
	CString strtmp;
	int i =0;
	int iCurValue=0;
	for(i =0; i < pcbo->GetCount(); i++)
	{
		pcbo->GetLBText(i, strtmp);
		if(strtmp == strValue)
		{
			pcbo->SetCurSel(i);
			iCurValue =i;
			break;
		}
	}
	return iCurValue;
}

CString CFFUMonDlg::GetFileValue(CString strAPPName, CString strKeyName, CString strConfigFile, CString strDefault)
{
	char szValue[MAX_PATH];
	CString strRetValue = _T("");

	ZeroMemory(szValue,MAX_PATH);
	::GetPrivateProfileString(strAPPName,strKeyName, strDefault, (LPTSTR)szValue, sizeof(szValue)-1, strConfigFile);
	strRetValue.Format(_T("%s"), (LPCTSTR)szValue);

	return strRetValue;
}

char CFFUMonDlg::GetDelimiter(CString strDelimiter)
{
	char cRet;

	if(strDelimiter == _T("None"))
		cRet = NULL;
	else if(strDelimiter == _T("STX"))
		cRet = dfSTX;
	else if(strDelimiter == _T("LF") || strDelimiter == _T("CRLF"))
		cRet = dfLF;
	else if(strDelimiter == _T("CR"))
		cRet = dfCR;
	else if(strDelimiter == _T("ETX"))
		cRet = dfETX;
	else
		cRet  = NULL;

	return cRet ;
}

BOOL CFFUMonDlg::WriteValue(LPCTSTR section, LPCTSTR key, LPCTSTR value) 
{
	BOOL result;
	
	result = WritePrivateProfileString (section, key, value, m_strConfigFile); 
	
	return result = TRUE;
}

CString CFFUMonDlg::GetCboValue(CComboBox *pcbo)
{
	CString strValue= _T("");
	
	pcbo->GetLBText(pcbo->GetCurSel(), strValue);
	
	return strValue;
}

// void CFFUMonDlg::ThreadBegin_RcvData(void)
// {
// 	if(!m_pRcvThread)
// 	{
// 		m_hEvtRcvStop = CreateEvent(NULL, FALSE, FALSE, NULL);
// 		m_pRcvThread = AfxBeginThread(ThreadRcvData, this);
// 		m_pRcvThread->m_bAutoDelete = FALSE;
// 	}
// }
// 
// UINT CFFUMonDlg::ThreadRcvData(LPVOID pParam)
// {
// 	CFFUMonDlg *pProc = (CFFUMonDlg*) pParam;
// 	volatile BOOL bLoop  = TRUE;
// 	while(bLoop)
// 	{
// 		DWORD res = WaitForSingleObject(pProc->m_hEvtRcvStop, 100);
// 		switch(res)
// 		{
// 		case WAIT_OBJECT_0:
// 			bLoop = FALSE;
// 			break;
// 		case WAIT_TIMEOUT:
// 			pProc->RcvData();
// 			break;
// 		}
// 	}
// 	return TRUE;
// }

LRESULT CFFUMonDlg::OnRcvSerialData(WPARAM wParam, LPARAM lParam)
{
	wParam;
	lParam;

	vector<BYTE> vecRvcData;
	vecRvcData = m_vecRvcData;
	ReceiveMsgProcess(vecRvcData);
	m_vecRvcData.clear();	// jsyoon 20150113 change
	return TRUE;
}



// void CFFUMonDlg::SetEFUData(vector<BYTE> vecRvcData)
// {
// 	FFU_DATA stEFUData;
// 
// 	int iMsgSize = 0, i =0;
// 	CString strHEX = _T("");
// 	BOOL bSTX = FALSE, bETX = FALSE ;
// 	BOOL bINFOMODE1 = FALSE, bINFOMODE2 = FALSE, bLV32_ID = FALSE, bDPU_ID = FALSE, bResponse = FALSE;
// 	BOOL bRPMMODE1 = FALSE, bRPMMODE2 = FALSE, bRPMFlagOK = FALSE;
// 
// 	int nId = 0;
// 	if(m_nMode == SENSOR_MODE_EFU)
// 		nId = 0;
// 	else
// 		nId = 1;
// 
// 	CString strLog = _T("");
// 	int iPV =0, iALARM = 0, iSV =0;
// 	int iEFUDataStart = 0;
// 	int iEDUCnt = 0;
// 	int iEFUNum = 0;
// 	UINT iEFUID = 0;
// 	BOOL bAlarm = FALSE;
// 
// 	iMsgSize = vecRvcData.size();
// 
// 	if(iMsgSize > 2)
// 	{
// 		if(vecRvcData[0]  == dfSTX)
// 		{
// 			bSTX = TRUE;
// 		}
// 
// 		if(vecRvcData[1]  == dfINFOMODE1)
// 		{
// 			bINFOMODE1 = TRUE;
// 		}
// 
// 		if(vecRvcData[2]  == dfINFOMODE2)
// 		{
// 			bINFOMODE2 = TRUE;
// 		}
// 
// 		if(vecRvcData[3]  == dfLV32_ID)
// 		{
// 			bLV32_ID = TRUE;
// 		}
// 
// 		if(vecRvcData[4]  == dfDPU_ID)
// 		{
// 			bDPU_ID = TRUE;
// 		}
// 		
// 		if(vecRvcData[iMsgSize-1]  == dfETX)
// 		{
// 			bETX = TRUE;
// 		}
// 
// 		if(bSTX && bETX && bINFOMODE1 && bINFOMODE2 && bLV32_ID && bDPU_ID)
// 		{
// 			bResponse = TRUE;
// 		}
// 	}
// 
// 	if(bResponse)
// 	{
// 		iEFUDataStart = 5;
// 	}
// 
// 
// 	iEDUCnt = (iMsgSize - 7) / 4 ;
// 
// 	if(vecRvcData[0]  == dfSTX)
// 	{
// 		bSTX = TRUE;
// 	}
// 	if(bSTX && vecRvcData[1]  == dfINFOMODE1)
// 	{
// 		bINFOMODE1 = TRUE;
// 	}
// 	if(bINFOMODE1 && vecRvcData[2]  == dfINFOMODE2)
// 	{
// 		bINFOMODE2 = TRUE;
// 	}
// 	if(bSTX && vecRvcData[1]  == dfRPMMODE1)
// 	{
// 		bRPMMODE1 = TRUE;
// 	}
// 	if(bRPMMODE1 && vecRvcData[2]  == dfRPMMODE2)
// 	{
// 		bRPMMODE2 = TRUE;
// 	}
// 	if(bRPMMODE2 && vecRvcData[iMsgSize - 2]  == dfRPMFlagOK)
// 	{
// 		bRPMFlagOK = TRUE;
// 	}
// 	if((bINFOMODE2 || bRPMMODE2) && vecRvcData[4]  == dfDPU_ID)
// 	{
// 		bDPU_ID = TRUE;
// 		iEFUDataStart = 5;
// 	}
// 
// 	for(i =0; i < iMsgSize; i++)
// 	{
// 		strHEX.Format(_T("%02X"), vecRvcData[i] );
// 
// 		//Success Start Bit 
// 		if(bDPU_ID && bINFOMODE1)
// 		{
// 			if(i >= iEFUDataStart)
// 			{
// 				iEFUNum ++;
// 
// 				iEFUID = GetByteToIDec(vecRvcData[i]);;
// 				iEFUID -= 128;
// 
// 				if(iEFUID == 31)
// 				{
// 					iEFUID = iEFUID;
// 				}
// 
// 				if( iEFUID > 0  && m_nEFUCnt[nId] >= iEFUID)
// 				{
// 					iPV = GetByteToIDec(vecRvcData[i+1]);
// 					API_SM_SetFFU_PV(iEFUID, iPV);
// 					
// 					bAlarm = CheckAlarm(vecRvcData[i+2]);
// 					iALARM =GetByteToIDec(vecRvcData[i+2]);
// 					API_SM_SetFFU_ALARM(iEFUID, iALARM);
// 					
// 					iSV = GetByteToIDec(vecRvcData[i+3]);
// 					API_SM_SetFFU_SV(iEFUID, iSV);
// 
// 					strLog.Format(_T("=== ID(%d) PV(%d) SV(%d)==="), iEFUID, iPV, iSV);
// 					WriteLogList(strLog, SENSOR_MODE_EFU);
// 				}
// 
// 				iEFUDataStart = i + 4;
// 
// 				if(iEDUCnt == iEFUNum)
// 				{
// 					break;
// 				}
// 			}
// 		}
// 	}
// 
// 	if(bDPU_ID && bETX)
// 	{
// 		strLog = "== Message Success ==";
// 		WriteLogList(strLog, SENSOR_MODE_EFU);
// 		if(bINFOMODE1 && bINFOMODE2)
// 		{
// 			strLog = "== INFO Message ==";
// 			WriteLogList(strLog, SENSOR_MODE_EFU);
// 
// 		}
// 		else if(bRPMMODE1 && bRPMMODE2 && bRPMFlagOK)
// 		{
// 			strLog = "== RPM Message ==";
// 			WriteLogList(strLog, SENSOR_MODE_EFU);
// 
// 		}
// 	}
// 	else
// 	{
// 		strLog.Format(_T("== Message Fail : STX(%d) LV32_ID(%d) DPU_ID(%d) ETX(%d) =="), 
// 			bSTX, bLV32_ID, bDPU_ID, bETX);
// 		WriteLogList(strLog, SENSOR_MODE_EFU);
// 	}
// 
// }

//Byte 에서 Dec 로 변환
int CFFUMonDlg::GetByteToIDec(BYTE byteValue)
{
	int iValue = 0;
	CString strValue = _T("");
	CString strHex = _T("");

	strHex.Format(_T("%02X"), byteValue );

	strValue.Format(_T("%d"), strtol(strHex, NULL, 16));

	iValue = atoi(strValue);

	return iValue;
}


BOOL CFFUMonDlg::CheckAlarm(BYTE byteAlarm)
{
	BOOL bRet = FALSE;
	CString strLog;
	
	switch (byteAlarm)
	{
	case NORMAL_ALARM:
		bRet = TRUE;
		strLog.Format(_T("NO ALARM"));
		break;
	case EC_ALARM:
		strLog.Format(_T("Occur ALARM : Excess Current"));
		break;
	case MOTOR_ALARM:
		strLog.Format(_T("Occur ALARM : Motor"));
		break;
	case COMM_ALARM:
		strLog.Format(_T("Occur ALARM : Communication"));
		break;
	default:
		strLog.Format(_T("Occur ALARM"));
		break;
	}
	
	if( NORMAL_ALARM != byteAlarm)
		WriteLogList(strLog, SENSOR_MODE_H2O);

	return bRet;
}

void CFFUMonDlg::WriteLogList(CString strMsg, int nMode)
{
	CTime ct = CTime::GetCurrentTime();
	CString strLog = _T(""); 
	CString strDate = _T("");
	CString strPort = _T(""); 
	CString strTmp = _T("");

	//CListBox* pLB = (CListBox*) GetDlgItem(IDC_LIST_VIEW);

	BOOL  bOn  = ((CButton*)GetDlgItem(IDC_CHECK_SAVE_LOG))->GetCheck();
	if(bOn)
	{
		DLOG(strMsg, nMode);
	}

	strDate = ct.Format(_T("[%m/%d %H:%M:%S"));
	strPort.Format(_T("]:(%d)%s"), strMsg.GetLength(), strMsg);

	strLog.Format("%s%s", strDate, strPort);

	if( nMode != DEFAULT_LOG)
	{
		//yhjoo 20141118 add
//		if(!(m_chkModule[SENSOR_MODE_EFU].GetCheck() || m_chkModule[SENSOR_MODE_EFU2].GetCheck() || m_chkModule[SENSOR_MODE_EFU3].GetCheck() || m_chkModule[SENSOR_MODE_EFU4].GetCheck() || m_chkModule[SENSOR_MODE_EFU5].GetCheck()))
//		{
			if (m_nMode != nMode)
				return;
//		}
		
		if(strLog.GetLength() > 201)
		{
			for(int i =0; i < strLog.GetLength(); i++)
			{
				strTmp += strLog[i];
				if(i % 201 == 0 && i>0 )
				{
					m_ListBox.InsertString(-1, strTmp);
					strLog.Delete(0,i);
					i  = 0;
					strTmp.Empty();
				}
			}
			m_ListBox.InsertString(-1, strTmp);
		}
		else
		{
			m_ListBox.InsertString(-1, strLog);
		}

		if (m_ListBox.GetCount() > 4096)
			m_ListBox.DeleteString(0);

		m_ListBox.SetCurSel(m_ListBox.GetCount()-1);
		m_ListBox.UpdateWindow();
	}
	else
	{
		if(strLog.GetLength() > 201)
		{
			for(int i =0; i < strLog.GetLength(); i++)
			{
				strTmp += strLog[i];
				if(i % 201 == 0 && i>0 )
				{
					m_ListBox2.InsertString(-1, strTmp);
					strLog.Delete(0,i);
					i  = 0;
					strTmp.Empty();
				}
			}
			m_ListBox2.InsertString(-1, strTmp);
		}
		else
		{
			m_ListBox2.InsertString(-1, strLog);
		}

		if (m_ListBox2.GetCount() > 100)
			m_ListBox2.DeleteString(0);

		m_ListBox2.SetCurSel(m_ListBox2.GetCount()-1);
		m_ListBox2.UpdateWindow();
	}
}


void CFFUMonDlg::OnButtonOpen() 
{
	//20140530 Jay Hong : for test
	//m_uiPort = 2;
// 	if(api_SerialCom_Open(m_uiPort, m_uiBaudRate, m_uiDataBits, m_uiParity, m_uiStopBit))	//h2o
// 	{
// 		
// 		if(api_SerialCom_IsOpened(m_uiPort))
// 		{
// 			ThreadBegin_RcvData();
// 			WriteLogList(_T("Port open success!"), SENSOR_MODE_H2O);
// 		}
// 		else
// 			WriteLogList(_T("Port open fail!"), SENSOR_MODE_H2O);
// 	}
// 	else
// 	{
// 		WriteLogList(_T("Port open fail!"), SENSOR_MODE_H2O);
// 	}
	
}

BOOL CFFUMonDlg::SendMsg(BYTE *byteBuff, int iLength)
{
	BOOL bRet = FALSE;

	CString strLog, strMsg;

	for(int i = 0; i<iLength; i++)
	{
		strLog.Format(_T("%02X "), byteBuff[i] ); 
		strMsg += strLog;
	}

	WriteLogList(strMsg, SENSOR_MODE_H2O);
	
	if(!api_SerialCom_IsOpened(m_uiPort))
	{
//		AfxMessageBox(_T("Please, open port first!"));
		WriteLogList(_T("Port open fail!"), SENSOR_MODE_H2O);
		return FALSE;
	}
	
	bRet = api_SerialCom_WriteByte(m_uiPort, byteBuff, iLength);
	
	return bRet;
	
}

void CFFUMonDlg::OnButtonClose() 
{
// 	ConnectTCP( FALSE );
// 
// 	SetEvent(m_hEvtGetStateStop);
// 	SetEvent(m_hEvtGetsDataStop);  //jh.lee 20160722 add
// 	SetEvent(m_hEvtMonitoringStop);
// 	api_SerialCom_Close(m_uiPort);
// 	ExitThreadAll();
// 	SAFE_DELETE(m_pRcvThread);
// 	WriteLogList(_T("Port Close!!"), SENSOR_MODE_H2O);
// 	//yhjoo 20141120 add
// 	m_bRecvThreadRun = FALSE;
// 	if(m_hRecvThread)
// 	{
// 		::WaitForSingleObject(m_hRecvThread, 1000);
// 		if (m_hRecvThread)
// 		{
// 			CloseHandle(m_hRecvThread);
// 			m_hRecvThread = NULL;
// 			
// 			closesocket(m_socetClient);
// 			WSACleanup();
// 		}
// 	}
// 	/////////////////////////////////////
// 
// 	if (m_bUse_ESD)
// 	{
// 		// 소켓 통신 연결 
//  		m_dlgESD.SocketDisConnectESD(m_strIPESD,m_strPortESD);
// 	}
}


void CFFUMonDlg::OnButtonSend() 
{
	//(1) Tx : HOST -> LV32 (Ask PV & ALARM & SV)
	//FFU INFO : 0x02 0x8A 0x87 0x81 0x9F 0x01 0x03 0x35 0x03
	//FFU RPM(RPM = 140)  : 0x02 0x89 0x84 0x81 0x9F 0x01 0x03 0x8C 0xBD 0x03
	//- 현재 구동 속도가 700RPM 이고 정상 일때 (PV는 70 . 0x46)
	//(2) Rx : LV32 -> HOST (Send PV & ALARM & SV)
	//0x02 0x8A 0x87 0x81 0x9F 0x01 0x46 0x80 0x46 0x02 0x46 0x80 0x46 0x03 0x46 0x80 0x46 0x5D 0x03

	CString strSendMsg = _T("");
	CString strMsg;
	CString strout;
	BYTE byteuf[256];
	int nCnt = 0, i =0;
	GetDlgItem(IDC_EDIT_SEND)->GetWindowText(strSendMsg);
	
	int iMsgLength = strSendMsg.GetLength();
	int iLength = strSendMsg.GetLength()/2;
	
	for(i=0; i<iMsgLength; i++)
	{
		strMsg = strMsg + strSendMsg[i];
		if((i+1) %2 ==0 &&  i >0)
		{
			nCnt ++;
			strout.Format(_T("%d"), strtol(strMsg, NULL, 16));
			byteuf[nCnt] = (BYTE)_tstoi(strout);
			strMsg.Empty();
		}
	}
	
	byteuf[0] = dfSTX;
	byteuf[iLength+1] = 0x00;
	for(i =0; i<nCnt; i++)
	{
		byteuf[iLength+1] += byteuf[i+1];
		strMsg.Format(_T("%02X"), byteuf[i+1] ); 
	}
	byteuf[iLength+1] = byteuf[iLength+1] & 0xff;
	
	strMsg.Format(_T("%02X"), byteuf[iLength+1] ); 
	byteuf[iLength+2] = dfETX;
	
	SendMsg(byteuf, iLength+3);
	
}

void CFFUMonDlg::OnButtonSave() 
{
	CString strAPPName = _T("SYSTEMINFO");
	
	CString strPort = _T("");
	CString strBaudRate = _T("");
	CString strParity = _T("");
	CString strDataBit = _T("");
	CString strStopBit = _T("");
	CString strStartDelimiter = _T("");
	CString strEndDelimiter = _T("");
	
	strPort = GetCboValue(&m_cboPort);
	WriteValue(strAPPName, _T("H2O_PORT"), strPort);
	
	strBaudRate = GetCboValue(&m_cboBaudRate);
	WriteValue(strAPPName, _T("BAUDRATE"), strBaudRate);
	
	strParity = GetCboValue(&m_cboParity);
	WriteValue(strAPPName, _T("PARITY"), strParity);
	
	strDataBit = GetCboValue(&m_cboDataBit);
	WriteValue(strAPPName, _T("DATABIT"), strDataBit);
	
	strStopBit = GetCboValue(&m_cboStopBit);
	WriteValue(strAPPName, _T("STOPBIT"), strStopBit);
	
	strStartDelimiter = GetCboValue(&m_cboStartDelimiter);
	WriteValue(strAPPName, _T("STARTDELIMITER"), strStartDelimiter);
	
	strEndDelimiter = GetCboValue(&m_cboEndDelimiter);
	WriteValue(strAPPName, _T("ENDDELIMITER"), strEndDelimiter);
	
}

void CFFUMonDlg::OnButtonRpm() 
{
	CString strcboLVID, strcboBLIDStart, strcboBLIDEnd, strcboSV;
	GetDlgItem(IDC_COMBO_LV32ID)->GetWindowText(strcboLVID);
	GetDlgItem(IDC_COMBO_BLSTART)->GetWindowText(strcboBLIDStart);
	GetDlgItem(IDC_COMBO_BLEND)->GetWindowText(strcboBLIDEnd);
	GetDlgItem(IDC_EDIT_RPM)->GetWindowText(strcboSV);

	int iValue = 0;
	int iELIDStart = 0, iBLIDEnd = 0;
	iELIDStart = _tstoi(strcboBLIDStart);
	iBLIDEnd   = _tstoi(strcboBLIDEnd);
	iValue	   = _tstoi(strcboSV);

	if( iELIDStart > iBLIDEnd)
	{
		AfxMessageBox("NG BL ID Select!!");
		return;
	}

	if( iValue > (int) m_uEFUSpeedMax[0])
	{
		AfxMessageBox("NG SPEED is Over Max EFU!!");
		return;
	}

	CDlgLogin dlgLogin;
	if( dlgLogin.DoModal() == IDOK )
	{
		// STOP
		for (int i=SENSOR_MODE_EFU; i<=SENSOR_MODE_EFU5; i++)
		{
			if(m_pDlgComm[i] != NULL)
				m_pDlgComm[i]->AllStop();
		}

		if( strcmp( dlgLogin.GetInput(), m_strPassWord) == 0)
		{
			if (m_bEFU_Type == EFU_TYPE_EBM)
			{	
				if(m_chkModule[SENSOR_MODE_EFU].GetCheck())
				{
					if(m_pDlgComm[SENSOR_MODE_EFU] != NULL)
					{
						m_pDlgComm[SENSOR_MODE_EFU]->SetEFUSpeed(iValue, 1, iELIDStart, iBLIDEnd);
					}
				}
				else if(m_chkModule[SENSOR_MODE_EFU2].GetCheck())
				{
					if(m_pDlgComm[SENSOR_MODE_EFU2] != NULL)
					{
						m_pDlgComm[SENSOR_MODE_EFU2]->SetEFUSpeed(iValue, 1, iELIDStart, iBLIDEnd);
					}
				}
				else if(m_chkModule[SENSOR_MODE_EFU3].GetCheck())
				{
					if(m_pDlgComm[SENSOR_MODE_EFU3] != NULL)
					{
						m_pDlgComm[SENSOR_MODE_EFU3]->SetEFUSpeed(iValue, 1, iELIDStart, iBLIDEnd);
					}
				}
				else if(m_chkModule[SENSOR_MODE_EFU4].GetCheck())
				{
					if(m_pDlgComm[SENSOR_MODE_EFU4] != NULL)
					{
						m_pDlgComm[SENSOR_MODE_EFU4]->SetEFUSpeed(iValue, 1, iELIDStart, iBLIDEnd);
					}
				}
				else if(m_chkModule[SENSOR_MODE_EFU5].GetCheck())
				{
					if(m_pDlgComm[SENSOR_MODE_EFU5] != NULL)
					{
						m_pDlgComm[SENSOR_MODE_EFU5]->SetEFUSpeed(iValue, 1, iELIDStart, iBLIDEnd);
					}
				}
			}
			else if (m_bEFU_Type == EFU_TYPE_HKC_H5)
			{
				m_pDlgComm[SENSOR_MODE_EFU]->TestSendEFU(iValue, 1, iELIDStart, iBLIDEnd, 3);
			}
			else
			{
				CString strSendMsg = _T("");
				CString strMsg;
				CString strout;
				BYTE byteuf[256];
				int nCnt = 0, i =0;
				CString strMode1, strMode2, strLVID, strDPUID, strBLIDStart, strBLIDEnd, strSV;

				CString strValue = _T("");
				CString strHex   = _T("");

				int iLVID = iValue;
				strHex.Format(_T("%02X"), iLVID | 0x80);
				strLVID = strHex;

				// 			iValue = atoi(strcboBLIDStart);
				// 			int iELIDStart = iValue;
				strHex.Format(_T("%02X"), iELIDStart);
				strBLIDStart = strHex;

				// 			iValue = atoi(strcboBLIDEnd);
				// 			int iBLIDEnd = iValue;
				strHex.Format(_T("%02X"), iBLIDEnd);
				strBLIDEnd = strHex;

				iValue = _tstoi(strcboSV);
				UINT iSV = iValue;
				strHex.Format(_T("%02X"), iSV);
				strSV = strHex;

				strMode1 = _T("89");
				strMode2 = _T("84");
				//	strLVID = "";
				strDPUID = _T("9F");
				//	strBLIDStart = "";
				//	strBLIDEnd = "";
				//	strSV = "";

				BOOL bOK = TRUE;
				for(i=iELIDStart; i<=iBLIDEnd; i++)
				{
					if ( iSV > m_uEFUSpeedMax[i-1] )	// check Max Limit
					{
						bOK = FALSE;
						break;
					}
				}

				if ( bOK )
				{
					strSendMsg = strMode1 + strMode2 + strLVID + strDPUID + strBLIDStart + strBLIDEnd + strSV;

					int iMsgLength = strSendMsg.GetLength();
					int iLength = strSendMsg.GetLength()/2;

					for(i=0; i<iMsgLength; i++)
					{
						strMsg = strMsg + strSendMsg[i];
						if((i+1) %2 == 0 &&  i >0)
						{
							nCnt ++;
							strout.Format(_T("%d"), strtol(strMsg, NULL, 16));
							byteuf[nCnt] = (BYTE)_tstoi(strout);
							strMsg .Empty();
						}
					}

					byteuf[0] = dfSTX;
					byteuf[iLength+1] = 0x00;
					for(i =0; i<nCnt; i++)
					{
						byteuf[iLength+1] += byteuf[i+1];
						strMsg.Format(_T("%02X"), byteuf[i+1]); 
					}
					byteuf[iLength+1] = byteuf[iLength+1] & 0xff;

					strMsg.Format(_T("%02X"), byteuf[iLength+1]); 
					byteuf[iLength+2] = dfETX;

					CString strSumMsg = _T("Send : ");
					for(i =0; i<iLength+3; i++)
					{
						strMsg.Format(_T("%02X"), byteuf[i]); 

						strMsg = strMsg + " ";
						strSumMsg = strSumMsg + strMsg;
					}

					if(m_pDlgComm[SENSOR_MODE_EFU] != NULL)
						m_pDlgComm[SENSOR_MODE_EFU]->SendMsg(byteuf, iLength+3);
				}
				else
				{
					AfxMessageBox("Insert RPM Limit Over!!");
				}	
			}		
		}
		else
		{
			AfxMessageBox("Insert Password wrong!!");
		}

		// START
		for (int i=SENSOR_MODE_EFU; i<=SENSOR_MODE_EFU5; i++)
		{
			if(m_pDlgComm[i] != NULL)
				m_pDlgComm[i]->StartThreadGetState();
		}
	}	
}

void CFFUMonDlg::OnDestroy() 
{
	CDialog::OnDestroy();
}

void CFFUMonDlg::OnButtonStatus() 
{
//	StartThreadGetState();
	for (int i=SENSOR_MODE_H2O; i<SENSOR_MODE_END; i++)
	{
		if( i == SENSOR_MODE_ACCURA)
			m_cDATA.m_nStop = FALSE;
		else
		{
			if(m_pDlgComm[i] != NULL)
				m_pDlgComm[i]->StartThreadGetState();
		}
	}

	//SetTimer(10, 1000, NULL);
	m_bTimer = TRUE;
}

CString CFFUMonDlg::GetTempData ()
{
	//MAIN 10개 SUB 10개 (최대 100개)
	int nAdd = 0;
	CString strTemp, strData;

	for (int i = 0; i < dfMAX_THERMETER; i++) {
		if (m_bUseTherm[i]) {
			for(int j = 0; j < m_uThermometerSub[i]; j++) {
				////TEST
				//static double fAdd = 0.01;
				//m_dThermometer[i][j] = fAdd;
				//fAdd += 0.01;
				////

				strTemp.Format (_T("TEMP%02d^%.2f,"), nAdd + 1, m_dThermometer[i][j]);
				strData += strTemp;
				nAdd++;
			}
		}
	}

	return strData;
}

CString CFFUMonDlg::GetEFUData ()
{
	//MAIN 10개 SUB 10개 (최대 100개)
	int nAdd = 0;
	CString strTemp, strData;

	if (m_bUse_EFU) {
		for (int i = 0; i < m_nEFUAllCnt; i++) {
			if (i < dfMAX_EFU) {
				////TEST
				//static double fAdd2 = 0.01;
				//m_dEFU[i] = fAdd2;
				//fAdd2 += 0.01;
				//

				strTemp.Format (_T("EFU%02d^%.1f,"), i + 1, m_dEFU[i]);
				strData += strTemp;
			}
		}
	}


	return strData;
}

CString CFFUMonDlg::GetEtcData ()
{
	int nAdd = 0;
	CString strTemp, strData;

	if (m_bUseSensor[SENSOR_MODE_O2]) {
		strTemp.Format (_T("O2^%.4f,"), m_dO2);
		strData += strTemp;
	}	

	if (m_bUseSensor[SENSOR_MODE_PRESSURE]) {
		strTemp.Format (_T("PRESS^%.4f,"), m_dPressure);
		strData += strTemp;
	}

	if (m_bUseSensor[SENSOR_MODE_KMN2]) {
		for (int i = 0; i < 2; i++) {
			strTemp.Format ("KMN2_%d^%.4f,",i + 1, m_dKMN2Result[i]);
			strData += strTemp;
		}
	}

	for (int i = 0; i < dfMAX_H2O; i++) {
		if (m_bUse_H2O[i]) {
			strTemp.Format (_T("H2O%d^%.4f,"), i + 1, m_dH2O[i]);
			strData += strTemp;
		}		
	}

	if (m_bUse_ESD) {
		for (int i = 0; i < m_nCnt_ESD; i++) {
			if (m_pDlgESD[i] != NULL) {
				strTemp.Format (_T("ESD%d^%.4f,"), i + 1, m_pDlgESD[i]->m_dStaticVolt[0]);
				strData += strTemp;
			}
		}
	}

	if (m_bUse_Ionizer) {
		for (int i = 0; i < 2; i++) {
			if (m_pDlgComm[SENSOR_MODE_IONIZER + i] != NULL) {
				for (int j = 0; j < 4; j++) {
					strTemp.Format ("ION_CONNECT_%d^%d,",  j + 1, m_cDATA_IONIZER[(i * 4) + j].bConnected);	
					strData += strTemp;
					strTemp.Format ("ION_ALARM_%d^%d,",    j + 1, m_cDATA_IONIZER[(i * 4) + j].bAlarm);		
					strData += strTemp;
					strTemp.Format ("ION_RUNTIME_%d^%.4f,",j + 1, m_cDATA_IONIZER[(i * 4) + j].dRunTime);		
					strData += strTemp;
				}
			}
		}
	}

	if (m_bUse_ACCURA || m_bUseSensor[SENSOR_MODE_ACCURA2]) {
		for (int i = 0; i < m_bCnt_ACCURA; i++) {
			if (m_pAccuraData != NULL) {
				//if (TRUE)
				strTemp.Format (_T("ACCURA%d^%.4f^%.4f^%.4f^%d"), i + 1, m_pAccuraData[i].Vavg_ll_1, m_pAccuraData[i].lavg_1, m_pAccuraData[i].Ptot_1, m_pAccuraData[i].Net_1); 

				for(int j = 0; j < m_pAccuraData[i].nValueCnt; j++) {
					strTemp.Format ("^%d^%d",m_pAccuraData[i].nValue[j], m_pAccuraData[i].nValueTotal[j]);
					strData += strTemp;
				}
				//else str.Format("ACCURA%d^%.4f^%.4f^%.4f^%d*",i+1, m_pAccuraData[i].Vavg_ll_1, m_pAccuraData[i].lavg_1, m_pAccuraData[i].Ptot_1, m_pAccuraData[i].Net_1);                               
			}
		}
	}

	//앞에 이름 하나만 쓰고 보냄?????
	//if (m_bUse_PM5500) {
	//	str.Empty();
	//	str.Format("PM5500"); 
	//	double dTHDi = 0.;

	//	strTmp.Format("^%.4f,", m_cDATA_PM55.m_dTotalPow);			str += strTmp;
	//	strTmp.Format("^%.4f,", m_cDATA_PM55.m_dInstantPow);		str += strTmp;
	//	strTmp.Format("^%.4f,", m_cDATA_PM55.m_dRS_line_volt);		str += strTmp;
	//	strTmp.Format("^%.4f,", m_cDATA_PM55.m_dST_line_volt);		str += strTmp;
	//	strTmp.Format("^%.4f,", m_cDATA_PM55.m_dTR_line_volt);		str += strTmp;
	//	strTmp.Format("^%.4f,", m_cDATA_PM55.m_dR_Phase_curr);		str += strTmp;
	//	strTmp.Format("^%.4f,", m_cDATA_PM55.m_dS_Phase_curr);		str += strTmp;
	//	strTmp.Format("^%.4f,", m_cDATA_PM55.m_dT_Phase_curr);		str += strTmp;
	//	strTmp.Format("^%.4f,", m_cDATA_PM55.m_dElect_cur_I);		str += strTmp;
	//	strTmp.Format("^%.4f,", m_cDATA_PM55.m_dElect_volt_U);		str += strTmp;
	//	strTmp.Format("^%.4f,", m_cDATA_PM55.m_dElect_act_pow);		str += strTmp;
	//	strTmp.Format("^%.4f,", m_cDATA_PM55.m_dPower_factor_cso);	str += strTmp;
	//	strTmp.Format("^%.4f,", m_cDATA_PM55.m_dFrequency);			str += strTmp;

	//	dTHDi = ( m_cDATA_PM55.m_dTHDi_A + m_cDATA_PM55.m_dTHDi_B + m_cDATA_PM55.m_dTHDi_C ) / 3;
	//	if( dTHDi < 0)
	//		dTHDi = 0;

	//	strTmp.Format("^%.4f,", dTHDi);								str += strTmp;

	//	strTmp.Format("^%.4f,", m_cDATA_PM55.m_dTHDu);				str += strTmp;
	//			
	//	str += "*";
	//}

	//얘도????
	//if( m_bUse_AZBIL )
	//{
	//	str.Empty();
	//	str.Format("AZBIL"); 

	//	strTmp.Format("^%.4f,", m_iAzbliValue[0]);			str += strTmp;
	//	strTmp.Format("^%.4f,", m_iAzbliValue[1]);			str += strTmp;
	//	strTmp.Format("^%.4f,", m_iAzbliValue[2]);			str += strTmp;
	//	strTmp.Format("^%.4f,", m_iAzbliValue[3]);			str += strTmp;
	//	strTmp.Format("^%.4f,", m_iAzbliValue[4]);			str += strTmp;

	//	str += "*";
	//	if( api_SocketCtrl_IsConnected(0) )
	//	{
	//		api_SocketCtrl_SendData(0, (BYTE*)(LPCTSTR)str, strlen((LPCTSTR)str));	
	//	}

	//	WriteLogList(str, DEFAULT_LOG);
	//}

	return strData;
}

void	CFFUMonDlg::SetMonitorSensorData ()
{
	CString str, strTemp, strTemp2;

	if (m_bUseSensor[SENSOR_MODE_O2]) {
		strTemp.Format (_T("\tO2, %8.3f, "), m_dO2);
		str += strTemp;
	}

	for (int i = 0; i < dfMAX_H2O; i++) {
		if (m_bUse_H2O[i]) {
			//if (strTemp.IsEmpty()) strTemp.Format (_T("H2O , %8.3f, "), m_dH2O[i]);
			//else				   strTemp.Format (_T("\tH2O , %8.3f, "), m_dH2O[i]);
			strTemp.Format (_T("\tH2O , %8.3f, "), m_dH2O[i]);
			str += strTemp;
		}
	}
		
	if (m_bUseSensor[SENSOR_MODE_PRESSURE]) {
		strTemp.Format (_T("\tPRESS, %8.3f, "), m_dPressure);
		str += strTemp;
	}

	for (int i = 0; i < dfMAX_THERMETER; i++) {
		if (m_bUseTherm[i]) {
			for (int j = 0; j < m_uThermometerSub[i]; j++ ) {
				strTemp.Format (_T("\t%s , %5.1f, "), m_szThermometerSub[i][j], m_dThermometer[i][j]);
				str += strTemp;
			}
		}
	}

	if (m_bUseSensor[SENSOR_MODE_KMN2]) {
		strTemp.Format (_T("\tKM-N2(1), %8.3f, KM-N2(2), %8.3f"), m_dKMN2Result[0], m_dKMN2Result[1]);
		str += strTemp;
	}

	if (m_bUse_ESD) {
		for (int i = 0; i < m_nCnt_ESD; i++) {
			if (m_pDlgESD[i] != NULL) {
				strTemp.Format (_T("\tESD%d, %8.3f"), i + 1, m_pDlgESD[i]->m_dStaticVolt[0]);
				str += strTemp;
			}
		}
	}

	if (m_bUse_ACCURA || m_bUseSensor[SENSOR_MODE_ACCURA2]) {
		if (m_pAccuraData != NULL) {
			for (int i = 0; i < m_bCnt_ACCURA; i++) {
				if (m_pAccuraData[i].nSubDev != 0) {
					strTemp.Format (_T("\tACCURA%d, %.4f, %.4f, %.4f, %d"), i + 1, m_pAccuraData[i].Vavg_ll_1, m_pAccuraData[i].lavg_1, m_pAccuraData[i].Ptot_1, m_pAccuraData[i].Net_1); 

					for (int k = 0; k < m_pAccuraData[i].nValueCnt; k++) {
						strTemp2.Format(", %d, %d",m_pAccuraData[i].nValue[k], m_pAccuraData[i].nValueTotal[k]);  
						strTemp += strTemp2;
					}
				}
				else {
					strTemp.Format (_T("\tACCURA%d, %.4f, %.4f, %.4f, %d"), i + 1, m_pAccuraData[i].Vavg_ll_1, m_pAccuraData[i].lavg_1, m_pAccuraData[i].Ptot_1, m_pAccuraData[i].Net_1);
				}

 				str += strTemp;
			}
		}
	}

	SetDlgItemText(IDC_STATIC_STATE2,str);
	WriteInfoLog(str);

	m_bSocketConnect = api_SocketCtrl_IsConnected(0);

	if( m_bSocketConnect)
		m_ledConnected.SetBkColor(CLR_GREEN);
	else
		m_ledConnected.SetBkColor(CLR_GRAY);
}

void	CFFUMonDlg::SendSensorData (CString strData)
{
	strData += "*";
	if (api_SocketCtrl_IsConnected (0)) {
		api_SocketCtrl_SendData (0, (BYTE*)(LPCTSTR)strData, strlen((LPCTSTR)strData));	
	}

	WriteLogList (strData, DEFAULT_LOG);
}

void CFFUMonDlg::OnTimer(UINT nIDEvent) 
{
	static int nTime = 0;
	static int nSec	 = 0;
	static bool bOdd = false;

	//프로그램 Delay??
	static int nCnt = 0;

	if (nIDEvent == TIMER_GET_DATA) {
		if (m_bTimer) {
			switch (nTime%3) {//각 case 마다 100ms 주기
				case 0://Get -> Send
					m_strData = GetTempData ();
					SendSensorData (m_strData);
					break;

				case 1://Get -> Send
					m_strData = GetEFUData () + GetEtcData ();
					SendSensorData (m_strData);
					break;

			//	case 2://UI Update
			//		SetMonitorSensorData ();
			//		break;

			//	case 3:
			//		if (bOdd) {//2배
			//		}
			//		bOdd = !bOdd;
			//		break;
			}

			nTime++;
			if (nTime >= 3) {
				nTime = 0;
			}
		}

		if (nCnt > 30) m_bTimer = TRUE;
		else		   nCnt++;
	}


	if (nIDEvent == 102) {
		//UI Update
		SetMonitorSensorData ();
	}
	

	//if (nIDEvent == 103)
	//{
	//	KillTimer(103);

	//	ShowBtnEFUCtrol(false);
	//}

	//else if (nIDEvent == 104 )
	//{
	//	KillTimer(104);

	//	SetTimer(10, 1000, NULL);	// jsyoon 20150129 add UI Update
	//}
}

void CFFUMonDlg::OnButtonStatus2() 
{
	SetEvent(m_hEvtGetStateStop);

	for (int i=SENSOR_MODE_H2O; i<SENSOR_MODE_END; i++)
	{
		if( i == SENSOR_MODE_ACCURA )
		{
			m_cDATA.m_nStop = TRUE;
		}
		else
		{
			if(m_pDlgComm[i] != NULL)
				m_pDlgComm[i]->AllStop();
		}
	}

	//KillTimer(10);		// jsyoon 20150130 add
	m_bTimer = FALSE;
}

void CFFUMonDlg::OnSelchangeComboPort() 
{
		
}


// void CFFUMonDlg::SetEFUValue(UINT nEfu, UINT nValue)
// {
// 
// 	CString strValue = _T("");
// 	CString strHex = _T("");
// 	CString strMode1, strMode2, strLVID, strDPUID, strBLIDStart, strBLIDEnd, strSV;
// 	CString strSendMsg = _T("");
// 
// 	CString strMsg;
// 	CString strout;
// 	BYTE byteuf[256];
// 	int nCnt = 0, i =0;
// 
// 	int iValue = 1;	//고정
// 	int iLVID = iValue | 0x80;
// 	strHex.Format(_T("%02X"), iLVID);
// 	strLVID = strHex;
// 	
// 	iValue = nEfu;
// 	int iELIDStart = iValue;
// 	strHex.Format(_T("%02X"), iELIDStart);
// 	strBLIDStart = strHex;
// 	
// 	iValue = nEfu;
// 	int iBLIDEnd = iValue;
// 	strHex.Format(_T("%02X"), iBLIDEnd);
// 	strBLIDEnd = strHex;
// 	
// 	iValue = nValue;
// 	int iSV = iValue;
// 	strHex.Format(_T("%02X"), iSV);
// 	strSV = strHex;
// 	
// 	strMode1 = _T("89");
// 	strMode2 = _T("84");
// 	//	strLVID = "";
//     strDPUID = _T("9F");
// 	//	strBLIDStart = "";
// 	//	strBLIDEnd = "";
// 	//	strSV = "";
// 	
// 	strSendMsg = strMode1 + strMode2 + strLVID + strDPUID + strBLIDStart + strBLIDEnd + strSV;
// 	
// 	
// 	int iMsgLength = strSendMsg.GetLength();
// 	int iLength = strSendMsg.GetLength()/2;
// 	
// 	for(i=0; i<iMsgLength; i++)
// 	{
// 		strMsg = strMsg + strSendMsg[i];
// 		if((i+1) %2 == 0 &&  i >0)
// 		{
// 			nCnt ++;
// 			strout.Format(_T("%d"), strtol(strMsg, NULL, 16));
// 			byteuf[nCnt] = (BYTE)_tstoi(strout);
// 			strMsg .Empty();
// 		}
// 	}
// 	
// 	byteuf[0] = dfSTX;
// 	byteuf[iLength+1] = 0x00;
// 	for(i =0; i<nCnt; i++)
// 	{
// 		byteuf[iLength+1] += byteuf[i+1];
// 		strMsg.Format(_T("%02X"), byteuf[i+1]); 
// 	}
// 	byteuf[iLength+1] = byteuf[iLength+1] & 0xff;
// 	
// 	strMsg.Format(_T("%02X"), byteuf[iLength+1]); 
// 	byteuf[iLength+2] = dfETX;
// 	
// 	CString strSumMsg = _T("Send : ");
// 	for(i =0; i<iLength+3; i++)
// 	{
// 		strMsg.Format(_T("%02X"), byteuf[i]); 
// 		
// 		strMsg = strMsg + " ";
// 		strSumMsg = strSumMsg + strMsg;
// 	}	
// 	
// 	byteuf[iLength+3] = 0x00;
// 	SendMsg(byteuf, iLength+3);
// }


// void CFFUMonDlg::SetRestoreEFUValue()
// {
// // 	for(int i=0;i<2;i++)	//혹시 몰라서 2번
// // 	{
// 		for(UINT nEfu=0;nEfu<m_nEFUCnt;nEfu++)
// 		{
// 			if( m_uEFUSpeed[nEfu] <= m_uEFUSpeedMax[nEfu] )	// jsyoon 20140811
// 			{
// 				SetEFUValue(nEfu+1, m_uEFUSpeed[nEfu]);
// 				Sleep(500);
// 			}
// 		}
// //	}
// }



void CFFUMonDlg::OnButtonAllShow() 
{
// 	CDlgCurrentValue dlg;
// 	dlg.DoModal();
	int nEFUNum = 0;

	if(m_chkModule[SENSOR_MODE_EFU].GetCheck())
		nEFUNum = 0;
	else if(m_chkModule[SENSOR_MODE_EFU2].GetCheck())
		nEFUNum = 1;
	else if(m_chkModule[SENSOR_MODE_EFU3].GetCheck())
		nEFUNum = 2;
	else if(m_chkModule[SENSOR_MODE_EFU4].GetCheck())
		nEFUNum = 3;
	else if(m_chkModule[SENSOR_MODE_EFU5].GetCheck())
		nEFUNum = 4;

	CDlgCurrentValue *dlg = new CDlgCurrentValue(NULL, m_nEFUAllCnt, m_nIdex);
	
	dlg->Create(IDD_DIALOG_VALUE, this);
	
	dlg->ShowWindow(SW_SHOW);	

}

// void CFFUMonDlg::AllOff()
// {
// // 	for(int i=0;i<2;i++)	//혹시 몰라서 2번
// // 	{
// 		for(UINT nEfu=0;nEfu<m_nEFUCnt;nEfu++)
// 		{
// 			SetEFUValue(nEfu+1, 1);
// 			Sleep(500);
// 		}
// //	}
// }

void CFFUMonDlg::OnButtonGetvalue() 
{
// 	BYTE byteuf[9];
// 	BYTE byteCheckSum = 0x00;
// 
// 	//0x02 0x8A 0x87 0x81 0x9F 0x01 0x03 0x35 0x03
// 	byteuf[0] = 0x02;
// 	byteuf[1] = 0x8A;
// 	byteuf[2] = 0x87;
// 	byteuf[3] = 0x81;
// 	byteuf[4] = 0x9F;
// 	byteuf[5] = 0x01;	//ECU #1
// 	byteuf[6] = 0x20;	//ECU #32
// 
// 	for(int i=1; i<=6; i++)
// 	{
// 		byteCheckSum += byteuf[i];
// 	}
// 
// 	byteCheckSum = byteCheckSum & 0xff;
// 
// 	byteuf[7] = byteCheckSum;
// 	byteuf[8] = 0x03;
// 	
// 
// 	CString strLog, strMsg;
// 	
// 	for(i = 0; i<9; i++)
// 	{
// 		strLog.Format(_T("%02X "), byteuf[i] ); 
// 		strMsg += strLog;
// 	}
// 	WriteLogList(strMsg, SENSOR_MODE_H2O);
// 
// 	api_SerialCom_WriteByte(m_uiPort, byteuf, 9);
}

int CFFUMonDlg::WriteFile(CFile *pFile, CString sPath, CString sLog)
{

	sLog += "\r\n";
    BOOL bNew = FALSE;
    CFileStatus fs;
    if (!CFile::GetStatus(sPath, fs))  //if not exist yet
    {
		gf_MakeDir(sPath);// _mkdir;
        bNew = TRUE;
    }
    else if (pFile->m_hFile != CFile::hFileNull)
    {
		CString s1 = fs.m_szFullName ;
		if ((pFile->GetFilePath() != s1))
		{
			bNew = TRUE;
		}
	}
	
	if (pFile->m_hFile != CFile::hFileNull && bNew)
	{
		pFile->Close();
	}
	
	
	CFileException* pError = NULL;
	if (pFile->m_hFile == CFile::hFileNull)
	{
		UINT nOpenFlags = CFile::modeReadWrite | CFile::shareDenyWrite;
        if (bNew)
			nOpenFlags |= CFile::modeCreate;
		
		if (!pFile->Open(sPath , nOpenFlags, pError))
			return 0;
	}
	if (pFile->m_hFile != CFile::hFileNull)
	{
		pFile->SeekToEnd();
		pFile->Write(sLog, sLog.GetLength()) ;
		pFile->Flush();
	}
	return 1;
}

int CFFUMonDlg::DLOG(CString s, UINT nMode)
{
	TCHAR szLogPath[6400];
	ZeroMemory(szLogPath,6400);

	SYSTEMTIME st;
	::GetLocalTime(&st);
	CTime ct(st);

	CString strMode;

	if( nMode >= 0 && nMode < dfMAX_USE_CONNECT)
	{
		m_chkModule[nMode].GetWindowText(strMode);
	}
	else
	{
		strMode.Format(_T("LOG"));
	}

// 	//yhjoo 20141118 add
// 	if (nMode == SENSOR_MODE_H2O)
// //		sprintf(szLogPath,"%sLOG_H2O-%02d.log"			,m_strLogPath,st.wDay);
// 		strMode.Format("_H2O");
// 	else if (nMode == SENSOR_MODE_O2)
// //		sprintf(szLogPath,"%sLOG_O2-%02d.log"			,m_strLogPath,st.wDay);
// 		strMode.Format("_O2");
// 	else if (nMode == SENSOR_MODE_TEMP1)
// //		sprintf(szLogPath,"%sLOG_PCRACK_Temp-%02d.log"	,m_strLogPath,st.wDay);
// 		strMode.Format("_TEMP_PCRACK");
// // 	else if (nMode == SENSOR_MODE_TEMP2)
// // //		sprintf(szLogPath,"%sLOG_CHAMBER_Temp-%02d.log"	,m_strLogPath,st.wDay);
// // 		strMode.Format("_TEMP_CHAMBER");
// // 	else if (nMode == SENSOR_MODE_TEMP3)
// //		sprintf(szLogPath,"%sLOG_ELBOX_Temp-%02d.log"	,m_strLogPath,st.wDay);
// //		strMode.Format("_TEMP_ELBOX");
// 	else if (nMode == SENSOR_MODE_PRESSURE)
// //		sprintf(szLogPath,"%sLOG_Pressure-%02d.log"		,m_strLogPath,st.wDay);
// 		strMode.Format("_Pressure");
// 	else if (nMode == SENSOR_MODE_EFU)
// //		sprintf(szLogPath,"%sLOG_EFU-%02d.log"			,m_strLogPath,st.wDay);
// 		strMode.Format("_EFU");
// 	else if (nMode == SENSOR_MODE_KMN2)		//jhseo 20170920 add for KM-N2
// //		sprintf(szLogPath,"%sLOG_KMN2-%02d.log"			,m_strLogPath,st.wDay);
// 		strMode.Format("_KMN2");
// 	else if (nMode == SENSOR_MODE_ESD)		//lhj 20171224 add
// //		sprintf(szLogPath,"%sLOG_ESD-%02d.log"			,m_strLogPath,st.wDay);
// 		strMode.Format("_ESD");
// 	else
// 		strMode.Format("");
//		sprintf(szLogPath,"%sLOG-%02d.log"			,m_strLogPath,st.wDay);

	_stprintf_s(szLogPath, _countof(szLogPath), "%s\\%s\\LOG-%02d.log", m_strLogPath, strMode, st.wDay);
	
	TCHAR szLog[6400];
	ZeroMemory(szLog,6400);
	//	sprintf(szLog,"%s   %s",gf_MakeCurTimeString(), s);
	_stprintf_s(szLog, _countof(szLog), "%s   %s",gf_MakeCurTimeString(), s);
	
	CFile fileLog;
	int nR = WriteFile(&fileLog,szLogPath,szLog);

	return nR;
}

void CFFUMonDlg::OnButtonRestoreValue() 
{
// 	AfxBeginThread(WorkThreadProcRestoreValue, this);
	if(m_pDlgComm[SENSOR_MODE_EFU] != NULL)
		m_pDlgComm[SENSOR_MODE_EFU]->RestoreValue();

	if(m_pDlgComm[SENSOR_MODE_EFU2] != NULL)
		m_pDlgComm[SENSOR_MODE_EFU2]->RestoreValue();
}

void CFFUMonDlg::OnButtonAllOff() 
{
// 	AfxBeginThread(WorkThreadProcAllOff, this);
	if(m_pDlgComm[SENSOR_MODE_EFU] != NULL)
		m_pDlgComm[SENSOR_MODE_EFU]->SetAllOff();

	if(m_pDlgComm[SENSOR_MODE_EFU2] != NULL)
		m_pDlgComm[SENSOR_MODE_EFU2]->SetAllOff();
	//20181207 edhan add
	if(m_pDlgComm[SENSOR_MODE_EFU3] != NULL)
		m_pDlgComm[SENSOR_MODE_EFU3]->SetAllOff();

	if(m_pDlgComm[SENSOR_MODE_EFU4] != NULL)
		m_pDlgComm[SENSOR_MODE_EFU4]->SetAllOff();

	if(m_pDlgComm[SENSOR_MODE_EFU5] != NULL)
		m_pDlgComm[SENSOR_MODE_EFU5]->SetAllOff();
	//

}

// UINT CFFUMonDlg::WorkThreadProcRestoreValue(LPVOID pParam)
// {
// 	CFFUMonDlg* pProc = (CFFUMonDlg*)pParam;
// 
// 	SetEvent(pProc->m_hEvtGetStateStop);
// 	Sleep(1000);
// 
// 	pProc->SetRestoreEFUValue();
// 
// 	Sleep(1000);
// 	pProc->StartThreadGetState();
// 
// 	return 1;
// }

// UINT CFFUMonDlg::WorkThreadProcAllOff(LPVOID pParam)
// {
// 	CFFUMonDlg* pProc = (CFFUMonDlg*)pParam;
// 
// 	SetEvent(pProc->m_hEvtGetStateStop);
// 	Sleep(1000);
// 	
// 	pProc->AllOff();
// 	
// 	Sleep(1000);
// 	pProc->StartThreadGetState();
// 
// 	return 1;
// }

// void CFFUMonDlg::StartThreadGetState()
// {
// 	//AfxBeginThread(WorkThreadProcGetState, this);
// 
// 	if(!m_pGetStateThread)
// 	{
// 		m_hEvtGetStateStop = CreateEvent(NULL, FALSE, FALSE, NULL);
// 		m_pGetStateThread = AfxBeginThread(WorkThreadProcGetState, this);
// 		m_pGetStateThread->m_bAutoDelete = FALSE;
// 	}
// }

// UINT CFFUMonDlg::WorkThreadProcGetState(LPVOID pParam)
// {
// 	CFFUMonDlg *pProc = (CFFUMonDlg*) pParam;
// 	volatile BOOL bLoop  = TRUE;
// 	while(bLoop)
// 	{
// 		DWORD res = WaitForSingleObject(pProc->m_hEvtGetStateStop, 1000);
// 		switch(res)
// 		{
// 		case WAIT_OBJECT_0:
// 			bLoop = FALSE;
// 			break;
// 		case WAIT_TIMEOUT:
// 			pProc->GetState();
// 			//break;
// 		}
// 	}
// 
// 	pProc->m_pGetStateThread = NULL;
// 	return TRUE;
// }




// void CFFUMonDlg::StartThreadMonitoring()
// {
// 	//AfxBeginThread(WorkThreadProcGetState, this);
// 	
// 	if(!m_pMonitoringThread)
// 	{
// 		m_hEvtMonitoringStop = CreateEvent(NULL, FALSE, FALSE, NULL);
// 		m_pMonitoringThread = AfxBeginThread(WorkThreadProcMonitoring, this);
// 		m_pMonitoringThread->m_bAutoDelete = FALSE;
// 	}
// }

// UINT CFFUMonDlg::WorkThreadProcMonitoring(LPVOID pParam)
// {
// 	CFFUMonDlg *pProc = (CFFUMonDlg*) pParam;
// 	volatile BOOL bLoop  = TRUE;
// 	BOOL bAllOff = FALSE;
// 	BOOL bRestore = FALSE;
// 	while(bLoop)
// 	{
// 		DWORD res = WaitForSingleObject(pProc->m_hEvtMonitoringStop, 1000);
// 		switch(res)
// 		{
// 		case WAIT_OBJECT_0:
// 			bLoop = FALSE;
// 			break;
// 		case WAIT_TIMEOUT:
// 			bAllOff = API_SM_GetFFU_ALLOFF();
// 			if(bAllOff)
// 			{
// 				pProc->WriteLogList(_T("=== All Off TRUE ==="), SENSOR_MODE_H2O);
// 
// 				SetEvent(pProc->m_hEvtGetStateStop);
// 				Sleep(1000);
// 				
// 				pProc->AllOff();
// 				
// 				Sleep(1000);
// 				pProc->StartThreadGetState();
// 			}
// 
// 			bRestore = API_SM_GetFFU_RESTORE();
// 			if(bRestore)
// 			{
// 				pProc->WriteLogList(_T("=== Restore Off TRUE ==="), SENSOR_MODE_H2O);
// 
// 				SetEvent(pProc->m_hEvtGetStateStop);
// 				Sleep(1000);
// 				
// 				pProc->SetRestoreEFUValue();
// 				
// 				Sleep(1000);
// 				pProc->StartThreadGetState();
// 			}
// 
// 			API_SM_SetFFU_ALLOFF(FALSE);		// jsyoon 20140708 add
// 			API_SM_SetFFU_RESTORE(FALSE);
// 		}
// 	}
// 	
// 	pProc->m_pMonitoringThread = NULL;
// 	return TRUE;
// }


void CFFUMonDlg::OnButtonClear() 
{
	m_ListBox.ResetContent();
}

BOOL CFFUMonDlg::SetWarning(BOOL bSet, int nAlarmID)
{
	AfxEnableControlContainer();					// pwk 20101102 add.
	
	if(bSet)
	{
		if(!m_pWarnMsgDlg)
		{
			m_pWarnMsgDlg = new CDlgWarnMsg;
			m_pWarnMsgDlg->Create(IDD_DIALOG_WARN, this);
			m_pWarnMsgDlg->ShowWindow(SW_NORMAL);
		}
		else
		{
			m_pWarnMsgDlg->ShowWindow(SW_SHOW);
		}
		
		if(m_pWarnMsgDlg)
		{
			if(m_pWarnMsgDlg->IsAlreadyAlarmHappened(nAlarmID))
			{
				return FALSE;
			}
			
			
			CString strWarn;
			strWarn.Format("[%d] : %s%d",nAlarmID, "EFU #", nAlarmID);
			
			TCHAR sz[256];
			sz[0] = 0x00;
//			strcpy(sz, strWarn);
			_tcscpy_s(sz, _countof(sz), strWarn);
			m_pWarnMsgDlg->AddMsg( sz );
			m_pWarnMsgDlg->SetMsg( sz);
			
			m_pWarnMsgDlg->m_lWarnStatus = nAlarmID;
			
		}
	}
	
	return TRUE;
}

// void CFFUMonDlg::GetState()
// {
// 	BYTE byteuf[5];
// 	
// 	byteuf[0] = 'N';	//
// 	byteuf[1] = '0';	//
// 	byteuf[2] = 'T';	//Transmit Value(read)
// 	byteuf[3] = 'A';
// 	byteuf[4] = '*';
// 
// 
// 
// 
// 	CString strLog, strMsg;
// 	
// 	for(int i = 0; i < 5; i++)
// 	{
// 		strLog.Format(_T("%c"), byteuf[i] ); 
// 		strMsg += strLog;
// 	}
// 	
// 	strLog = "SEND : ";
// 	strLog += strMsg;
// 
// 	WriteLogList(strLog, SENSOR_MODE_H2O);
// 	
// 	api_SerialCom_WriteByte(m_uiPort, byteuf, 5);
// }

BOOL CFFUMonDlg::RcvData(void)
{
	int nRcvCnt = 0;
//	UINT nCurIdx = 0;
	BYTE byteReadChar;
	BYTE byteCheckSum = 0x00;
	BYTE byteBuffer[dfBUFFER_SIZE];
	CString strRcvData = _T("");
	CString strTotalRcvData = _T("");
	BOOL bSTX= FALSE, bETX= FALSE;
	BOOL bDPUID= FALSE;
	int nCount=0;
	vector<BYTE> vecRvcData;

//	int iMsgSize = 0;
	CString strTmp, strMsg;

//	ZeroMemory(byteBuffer, sizeof(byteBuffer));
	ZeroMemory(byteBuffer, sizeof(BYTE)*dfBUFFER_SIZE);
	
	vecRvcData.clear();

	while(TRUE)
	{
		if(!api_SerialCom_IsOpened(m_uiPort))
			return FALSE; // return FALSE;

		nRcvCnt = api_SerialCom_GetDataInQueue(m_uiPort);
		if(nRcvCnt >0)
		{
//			CString strLog, strMsg;
//			for(int n=0;n<nRcvCnt;n++)
//			{
//				strLog.Format(_T("%02X "), byteuf[i] ); 
//				strMsg += strLog;
//			}
			
			if(m_cEndDelimiter == NULL) // Receive delimeter : NULL
			{
				TRACE("REVData = %d\r\n", nRcvCnt);	//ksk 20150110 for test
				
				if( nRcvCnt < dfBUFFER_SIZE )	// jsyoon 20150113 add
				{
					api_SerialCom_ReadByte(m_uiPort, byteBuffer, nRcvCnt);
					for(int i = 0; i<nRcvCnt; i++)
					{
						m_vecRvcData.push_back(byteBuffer[i]);
					}
					
					//				::PostMessage(this->GetSafeHwnd(), UWM_RCVDATA, NULL, NULL);
					::SendMessage(this->GetSafeHwnd(), UWM_RCVDATA, NULL, NULL);	// jsyoon 20150113 change
					//				ZeroMemory(byteBuffer, sizeof(byteBuffer));
					ZeroMemory(byteBuffer, sizeof(BYTE)*dfBUFFER_SIZE);
				}
			}
			else // Receive delimiter : CR, LF, CRLF, ETX, STX.
			{
				Sleep(100);
				for(int i=0;i<nRcvCnt;i++)
				{
					api_SerialCom_ReadByte(m_uiPort, &byteReadChar, 1);
					
					if(bSTX)
					{
						vecRvcData.push_back(byteReadChar);
					}
					
					//Check Start Bit
					if(byteReadChar == dfSTX && !bSTX && !bDPUID)
					{
						bSTX = TRUE;
						vecRvcData.push_back(byteReadChar);
					}
					if(bSTX && byteReadChar == dfDPU_ID)
					{
						bDPUID = TRUE;
					}
					
					//Check End Bit
					if(byteReadChar == m_cEndDelimiter)
					{
						byteCheckSum = 0x00;

						int Len = vecRvcData.size();
				//		if(Len>1)
						if(Len>2)
						{
							//for(int i=1; i<vecRvcData.size()-2; i++)

							Len = vecRvcData.size()-2;
							for(int j=1; j<Len; j++)
							{
								byteCheckSum += vecRvcData[j];
								strRcvData.Format(_T("%02X"), vecRvcData[j] ); 
							}

							byteCheckSum = byteCheckSum & 0xff;
							strRcvData.Format(_T("%02X"), byteCheckSum ); // 계산한 CheckSum
							strRcvData.Format(_T("%02X"),  vecRvcData[Len]); //실제로 받은 CheckSum
				//			strRcvData.Format(_T("%02X"),  vecRvcData[vecRvcData.size()-2]); //실제로 받은 CheckSum
							
							//Check CheckCum
				//			if(byteCheckSum == vecRvcData[vecRvcData.size()-2])
							if(byteCheckSum == vecRvcData[Len])
							{
								bETX = TRUE;
							}
						}
// 						else
// 						{
// 							bETX = TRUE;
// 						}
					}
					
					if(bSTX && bDPUID && bETX)
					{
						//20131230 twkoo
						//m_vecRvcData = vecRvcData;
						//::PostMessage(this->GetSafeHwnd(), UWM_RCVDATA, NULL, NULL);
						
						bSTX = FALSE;
						bDPUID = FALSE;
						bETX = FALSE;
						nCount = 0;
						
						//20131230 twkoo
						ReceiveMsgProcess(vecRvcData);
						vecRvcData.clear();
					}
					nCount ++;
				}
			}
		}
		else
		{
			return FALSE; //break;
		}
	}
	return TRUE;
}

void CFFUMonDlg::ReceiveMsgProcess(vector<BYTE> vecRvcData)
{
	int iMsgSize = 0;
	CString strMsg = _T("");
	CString strTmp = _T("");
	CString strLog = _T("");
	int i = 0;
	iMsgSize = vecRvcData.size();

	byte bDelimeter = 0x0;
	for( i = 0; i<iMsgSize; i++)
	{
		strTmp.Format(_T("%c"), vecRvcData[i] ); 
		strMsg += strTmp;
//		bDelimeter	 = vecRvcData[i];
	}

	strLog.Format(_T("RCV: %s (%d)"), strMsg,iMsgSize);
	WriteLogList(strLog, DEFAULT_LOG);

//	WriteLogList(strLog);

// 	m_strReceive += strMsg;
// 	if(bDelimeter == 0x0a && m_strReceive.GetLength() == 14)
// 	{
// 		double dTate = 0;
// 		dTate = GetIntputValue(m_strReceive);
// 		m_dH2O = dTate;	//yhjoo 20141120 add
// 
// 		strLog.Format(_T("Get VALUE = %.4f"), dTate);
// 		WriteLogList(strLog, SENSOR_MODE_H2O);
// 		m_strReceive.Empty();
// 	}
	

//	SetEFUData(vecRvcData);

	//20131230 twkoo
	//vecRvcData.clear();
}

double CFFUMonDlg::GetIntputValue(CString strRcv)
{
	CString strValue;
	strValue = strRcv.Mid(0,strRcv.GetLength()-2);
	strValue.TrimLeft();

	return atof(strValue);

}

void CFFUMonDlg::OnButtonChk1() 
{
	UpdateBtn(0);
}

void CFFUMonDlg::OnButtonChk2() 
{
	UpdateBtn(1);
}

void CFFUMonDlg::OnButtonChk3() 
{
	UpdateBtn(2);
}

void CFFUMonDlg::OnButtonChk4() 
{
	UpdateBtn(3);
}

void CFFUMonDlg::OnButtonChk5() 
{
	UpdateBtn(4);
}

void CFFUMonDlg::OnButtonChk6() 
{
	UpdateBtn(5);
}

void CFFUMonDlg::OnButtonChk7() 
{
	UpdateBtn(6);
}

void CFFUMonDlg::OnButtonChk8() 
{
	UpdateBtn(7);
}

void CFFUMonDlg::OnButtonChk9() 
{
	UpdateBtn(8);
}

void CFFUMonDlg::OnButtonChk10() 
{
	UpdateBtn(9);
}

void CFFUMonDlg::OnButtonChk11() 
{
	UpdateBtn(10);
}

void CFFUMonDlg::OnButtonChk12() 
{
	UpdateBtn(11);
}

void CFFUMonDlg::OnButtonChk13() 
{
	UpdateBtn(12);
}

void CFFUMonDlg::OnButtonChk14() 
{
	UpdateBtn(13);
}

void CFFUMonDlg::OnButtonChk15() 
{
	UpdateBtn(14);
}

void CFFUMonDlg::OnButtonChk16() 
{
	UpdateBtn(15);
}

void CFFUMonDlg::OnButtonChk17() 
{
	UpdateBtn(16);
}

void CFFUMonDlg::OnButtonChk18() 
{
	UpdateBtn(17);
}

void CFFUMonDlg::OnButtonChk19() 
{
	UpdateBtn(18);
}

void CFFUMonDlg::OnButtonChk20() 
{
	UpdateBtn(19);
}

void CFFUMonDlg::OnButtonChk21() 
{
	UpdateBtn(20);
}

void CFFUMonDlg::OnButtonChk22() 
{
	UpdateBtn(21);
}

void CFFUMonDlg::OnButtonChk23() 
{
	UpdateBtn(22);
}

void CFFUMonDlg::OnButtonChk24() 
{
	UpdateBtn(23);
}

void CFFUMonDlg::OnButtonChk25() 
{
	UpdateBtn(24);
}

void CFFUMonDlg::OnButtonChk26() 
{
	UpdateBtn(25);
}

void CFFUMonDlg::OnButtonChk27() 
{
	UpdateBtn(26);
}

void CFFUMonDlg::OnButtonChk28() 
{
	UpdateBtn(27);
}

void CFFUMonDlg::OnButtonChk29() 
{
	UpdateBtn(28);
}

void CFFUMonDlg::OnButtonChk30() 
{
	UpdateBtn(29);
}

void CFFUMonDlg::OnButtonChk31() 
{
	UpdateBtn(30);
}

void CFFUMonDlg::OnButtonChk32() 
{
	UpdateBtn(31);
}

void CFFUMonDlg::OnButtonChk33() 
{
	UpdateBtn(32);
}

void CFFUMonDlg::OnButtonChk34() 
{
	UpdateBtn(33);
}

void CFFUMonDlg::OnButtonChk35() 
{
	UpdateBtn(34);
}

void CFFUMonDlg::OnButtonChk36() 
{
	UpdateBtn(35);
}

void CFFUMonDlg::OnButtonChk37() 
{
	UpdateBtn(36);
}

void CFFUMonDlg::OnButtonChk38() 
{
	UpdateBtn(SENSOR_MODE_ESD);
}

void CFFUMonDlg::OnButtonChk39() 
{
	UpdateBtn(SENSOR_MODE_ESD2);
}

void CFFUMonDlg::OnButtonChk40() 
{
	UpdateBtn(SENSOR_MODE_ESD3);
}

void CFFUMonDlg::OnButtonChk41() 
{
	UpdateBtn(SENSOR_MODE_ACCURA);
}

void CFFUMonDlg::OnButtonChk42() 
{
	UpdateBtn(SENSOR_MODE_ELECTRICITY_METER);
}


void CFFUMonDlg::OnButtonChk43() 
{
	UpdateBtn(SENSOR_MODE_PM5500);
}


void CFFUMonDlg::OnButtonChk44() 
{
	UpdateBtn(SENSOR_MODE_AZBIL);
}


void CFFUMonDlg::OnButtonH2o() 
{
	UpdateBtn(SENSOR_MODE_H2O);
}

void CFFUMonDlg::OnButtonO2() 
{
	UpdateBtn(SENSOR_MODE_O2);
}

void CFFUMonDlg::OnButtonTemp1() 
{
	UpdateBtn(SENSOR_MODE_TEMP1);
}

void CFFUMonDlg::OnButtonTemp2() 
{
	UpdateBtn(SENSOR_MODE_TEMP1);
}

void CFFUMonDlg::OnButtonTemp3() 
{
	UpdateBtn(SENSOR_MODE_TEMP1);
}

void CFFUMonDlg::OnButtonTemp4() 
{
	UpdateBtn(SENSOR_MODE_TEMP1);
}

void CFFUMonDlg::OnButtonTemp5() 
{
	UpdateBtn(SENSOR_MODE_TEMP1);
}

void CFFUMonDlg::OnButtonPressure() 
{
	UpdateBtn(SENSOR_MODE_PRESSURE);
}

void CFFUMonDlg::OnButtonEfu() 
{
	UpdateBtn(SENSOR_MODE_EFU);
}

void CFFUMonDlg::SetControlShow(BOOL bShow)
{
	GetDlgItem(IDC_BUTTON_ALL_SHOW)->EnableWindow(bShow);
	GetDlgItem(IDC_COMBO_BLSTART)->EnableWindow(bShow);
	GetDlgItem(IDC_COMBO_BLEND)->EnableWindow(bShow);
	GetDlgItem(IDC_EDIT_RPM)->EnableWindow(bShow);
	GetDlgItem(IDC_BUTTON_RPM)->EnableWindow(bShow);
	GetDlgItem(IDC_BUTTON_RESTORE_VALUE)->EnableWindow(bShow);
	GetDlgItem(IDC_BUTTON_ALL_OFF)->EnableWindow(bShow);
	GetDlgItem(IDC_EDIT_RESTORE)->EnableWindow(bShow);
	GetDlgItem(IDC_EDIT_ALLOFF)->EnableWindow(bShow);

// 	if( m_bUse_H2O[0] == TRUE ) GetDlgItem(IDC_CHK_1)->ShowWindow(SW_SHOW);  else GetDlgItem(IDC_CHK_1)->ShowWindow(SW_HIDE);
// 	if( m_bUse_H2O[1] == TRUE ) GetDlgItem(IDC_CHK_2)->ShowWindow(SW_SHOW);  else GetDlgItem(IDC_CHK_2)->ShowWindow(SW_HIDE);

	int i=0;
	for (i=0; i<10; i++)
	{
		if( m_bUseSensor[i] == TRUE )  GetDlgItem(IDC_CHK_1+i)->ShowWindow(SW_SHOW);  else GetDlgItem(IDC_CHK_1+i)->ShowWindow(SW_HIDE);
	}

	for (i=0; i<dfMAX_H2O; i++)
	{
		if( m_bUse_H2O[i] == TRUE )  GetDlgItem(IDC_CHK_1+SENSOR_MODE_H2O+i)->ShowWindow(SW_SHOW);  else GetDlgItem(IDC_CHK_1+SENSOR_MODE_H2O+i)->ShowWindow(SW_HIDE);
	}

	for (i=0; i<dfMAX_THERMETER; i++)
	{
		if( m_bUseTherm[i] == TRUE )  GetDlgItem(IDC_CHK_1+SENSOR_MODE_TEMP1+i)->ShowWindow(SW_SHOW);  else GetDlgItem(IDC_CHK_1+SENSOR_MODE_TEMP1+i)->ShowWindow(SW_HIDE);
	}

	if( m_bUse_EFU == TRUE )		GetDlgItem(IDC_CHK_1+SENSOR_MODE_EFU)->ShowWindow(SW_SHOW);	    else GetDlgItem(IDC_CHK_1+SENSOR_MODE_EFU)->ShowWindow(SW_HIDE);

	for (i=0; i<3; i++)	
	{
		if( i < (int)m_nCnt_ESD && m_bUse_ESD == TRUE )	GetDlgItem(IDC_CHK_1+SENSOR_MODE_ESD+i)->ShowWindow(SW_SHOW);		
		else										    GetDlgItem(IDC_CHK_1+SENSOR_MODE_ESD+i)->ShowWindow(SW_HIDE);	
	}

	if( m_bUse_ACCURA == TRUE )		GetDlgItem(IDC_CHK_1+SENSOR_MODE_ACCURA)->ShowWindow(SW_SHOW);  else GetDlgItem(IDC_CHK_1+SENSOR_MODE_ACCURA)->ShowWindow(SW_HIDE);
	

// 	if( m_bUseTherm[0] == TRUE )  GetDlgItem(IDC_CHK_3)->ShowWindow(SW_SHOW);  else GetDlgItem(IDC_CHK_3)->ShowWindow(SW_HIDE);
// 	if( m_bUseTherm[1] == TRUE )  GetDlgItem(IDC_CHK_4)->ShowWindow(SW_SHOW);  else GetDlgItem(IDC_CHK_4)->ShowWindow(SW_HIDE);
// 	if( m_bUseTherm[2] == TRUE )  GetDlgItem(IDC_CHK_5)->ShowWindow(SW_SHOW);  else GetDlgItem(IDC_CHK_5)->ShowWindow(SW_HIDE);
// 	if( m_bUseTherm[3] == TRUE )  GetDlgItem(IDC_CHK_6)->ShowWindow(SW_SHOW);  else GetDlgItem(IDC_CHK_6)->ShowWindow(SW_HIDE);
// 	if( m_bUseTherm[4] == TRUE )  GetDlgItem(IDC_CHK_7)->ShowWindow(SW_SHOW);  else GetDlgItem(IDC_CHK_7)->ShowWindow(SW_HIDE);
// 	if( m_bUseTherm[5] == TRUE )  GetDlgItem(IDC_CHK_8)->ShowWindow(SW_SHOW);  else GetDlgItem(IDC_CHK_8)->ShowWindow(SW_HIDE);

//	if( m_bUse_Pressure == TRUE )  GetDlgItem(IDC_CHK_11)->ShowWindow(SW_SHOW); else  GetDlgItem(IDC_CHK_11)->ShowWindow(SW_HIDE);
//	if( m_bUse_KMN2 == TRUE )  GetDlgItem(IDC_CHK_KMN2)->ShowWindow(SW_SHOW);  else GetDlgItem(IDC_CHK_KMN2)->ShowWindow(SW_HIDE);
// 	if( m_bUse_EFU == TRUE )  GetDlgItem(IDC_CHK_12)->ShowWindow(SW_SHOW);	   else  GetDlgItem(IDC_CHK_12)->ShowWindow(SW_HIDE);
// 	if( m_bUse_ESD == TRUE )  GetDlgItem(IDC_CHK_ESD)->ShowWindow(SW_SHOW);  else GetDlgItem(IDC_CHK_ESD)->ShowWindow(SW_HIDE);
// 	if( m_bUse_ACCURA == TRUE )  GetDlgItem(IDC_CHK_ACCURA)->ShowWindow(SW_SHOW);  else GetDlgItem(IDC_CHK_ACCURA)->ShowWindow(SW_HIDE);
}

//void CFFUMonDlg::SetSendData()
//{
//	if (!m_bSetSendData)
//	{
//		m_bSetSendData = TRUE;
//		//SetTimer(10, 1000, NULL);
//		m_bTimer = TRUE;
//	}
//}

BOOL CFFUMonDlg::ConnectTCP(BOOL bConnect)
{
	if (!m_bUse_FDC)	//yhjoo 20160704 add
		return FALSE;

	UINT nSocketIndex = 0;
	CString strLog;

	CString strIP;
	SOCKET_OPEN stSockOpen;
	stSockOpen.nSockMode = SOCKET_MODE_CLIENT;
	stSockOpen.nPort	 = m_nPort;
	strIP.Format("%s",m_strIP);
//	strcpy(stSockOpen.szIPAddr, strIP);
	_tcscpy_s(stSockOpen.szIPAddr, _countof(stSockOpen.szIPAddr), strIP);
	
	BOOL bConnected = FALSE;
	bConnected = api_SocketCtrl_IsConnected(nSocketIndex);
	if(!bConnected)							// 설정이 되어 있지 않을 경우 
	{		
		BOOL bInitialize  = FALSE;
		int nSockMode	  = stSockOpen.nSockMode;
		int nPort		  = stSockOpen.nPort;
		CString strIPAddr = stSockOpen.szIPAddr;
		
		if(nSockMode == SOCKET_MODE_CLIENT)
		{
			if(api_SocketCtrl_IsConnected(nSocketIndex))
			{
				// 현재 연결되었으면, 연결 해제.
				api_SocketCtrl_Close(nSocketIndex);
				strLog.Format("[CLOSED] by user");	
				WriteLogList(strLog, DEFAULT_LOG);
			}

			if( bConnect )
			{
				TCHAR szIp[32];
				_tcscpy_s(szIp, _countof(szIp), strIPAddr);
				bInitialize = api_SocketCtrl_Initialize(nSocketIndex, this, nSockMode, nPort, szIp);			// jsyoon 20180913 change
//				bInitialize = api_SocketCtrl_Initialize(nSocketIndex, this, nSockMode, nPort, strIPAddr);
				
				if(bInitialize)
				{
					strLog.Format("[CONNECTED] Done");
				}
				else
				{
					api_SocketCtrl_Close(nSocketIndex);
					strLog.Format("[CONNECTED] Fail!!!");
				}
			}
			else
			{
				api_SocketCtrl_Close(nSocketIndex);
				strLog = "[Closed] by User";	
			}
		}
		else if(nSockMode == SOCKET_MODE_SERVER)
		{
			TCHAR szIp[32];
			_tcscpy_s(szIp, _countof(szIp), strIPAddr);
			bInitialize = api_SocketCtrl_Initialize(nSocketIndex, this, nSockMode, nPort, szIp);			// jsyoon 20180913 change
//			bInitialize = api_SocketCtrl_Initialize(nSocketIndex, this, nSockMode, nPort, strIPAddr);
			if(!bInitialize)
			{
				strLog.Format("[CONNECTED] Fail!!!");
			}
		}
	}
	else
	{
		api_SocketCtrl_Close(nSocketIndex);
		strLog = "[Closed] by User";	
	}

	WriteLogList(strLog, DEFAULT_LOG);


// 	CString str;
// 
// 	WSAStartup(MAKEWORD(2,2),&m_wsaData);
// 	
// 	m_socetClient = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
// 		
// 	struct sockaddr_in target;
// 	target.sin_family = AF_INET;
// 	target.sin_addr.s_addr = inet_addr(m_strIP);
// 	target.sin_port = htons(m_nPort);
// 	
// 	if(connect(m_socetClient,(sockaddr*)&target,sizeof(target))==SOCKET_ERROR)
// 	{
// // 		AfxMessageBox("connect() error!");
// // 		for (int i=0; i<7; i++)
// // 		{
// // 			WriteLogList("Server connect() error!", SENSOR_MODE_H2O+i);
// // 		}
// 		bRtn = FALSE;
// 	}
// 	else
// 	{
// 		int bytes=0;
// 		char buf[1024];
// 		memset(buf, 0, sizeof(char) * 1024);	//ksk 20150608 change
// 		
// 		m_bRecvThreadRun = TRUE;
// 		if( m_hRecvThread == NULL)
// 			m_hRecvThread = (HANDLE)_beginthreadex(NULL, 0, CltthreadRecv, this, 0, NULL);
// 
// 		GetDlgItem(IDC_BUTTON_TCP)->EnableWindow(FALSE);
// 		for (int i=0; i<7; i++)
// 		{
// 			WriteLogList("Server connect() OK", SENSOR_MODE_H2O+i);
// 		}
// 		m_bSetSendData = FALSE;
// 		bRtn = TRUE;
// 	}

	return bConnected;
}

unsigned int __stdcall CltthreadRecv(void* pParam)
{
	char szRecvBuf[2048];
	int nRecvByte = 0;	

	CFFUMonDlg *pComm = (CFFUMonDlg*)pParam;
	while(pComm->m_bRecvThreadRun)
	{
		memset(szRecvBuf, 0, 2048);
		nRecvByte = recv(pComm->m_socetClient, (char*)szRecvBuf, 2048, 0);

		// 통신 단절
		if(nRecvByte == 0)
		{
			for (int i=0; i<7; i++)
			{
				pComm->WriteLogList("Server Disconnect", SENSOR_MODE_H2O+i);
			}
			pComm->m_bRecvThreadRun = FALSE;
			pComm->m_hRecvThread = NULL;	//yhjoo 20150128 add
/*			pComm->GetDlgItem(IDC_BUTTON_TCP)->EnableWindow(TRUE);*/
			break;
		}
		else if(nRecvByte == SOCKET_ERROR)
		{
// 			AfxMessageBox("통신중단");
			for (int i=0; i<7; i++)
			{
				pComm->WriteLogList("Server Socket Error", SENSOR_MODE_H2O+i);
			}
			pComm->m_bRecvThreadRun = FALSE;
			pComm->m_hRecvThread = NULL;	//yhjoo 20150128 add
/*			pComm->GetDlgItem(IDC_BUTTON_TCP)->EnableWindow(TRUE);*/
			break;	
		}

		for (int i=0; i<nRecvByte; i++)
		{
		}
	}

	return 0;
}

void CFFUMonDlg::OnButtonTcp() 
{
	// TODO: Add your control notification handler code here
//	ConnectTCP();
}

//yhjoo 20141209 add
void CFFUMonDlg::OnChangeEditCmd() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	const int BeforeLength = m_strCMD.GetLength(); 
	const char HasToInlcudeData[17] = "0123456789ABCDEF"; 
	UpdateData(TRUE); 
	const int NewLength = m_strCMD.GetLength(); 
	if(BeforeLength < NewLength)
	{
		int i;
		for(i=0; i<17; i++)
		{
			if(m_strCMD[NewLength-1] == HasToInlcudeData[i])
				break;
		}
		if(i==17)
		{
			m_strCMD.Delete(NewLength-1, 1);
		}
	}
	UpdateData(FALSE);
	m_edtCMD.SetSel(-2, -1);
}

WORD CFFUMonDlg::GetCRC16(const WORD *nData, WORD wLength)
{
	static const WORD wCRCTable[256] = {
			0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
			0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
			0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
			0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
			0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
			0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
			0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
			0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
			0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
			0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
			0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
			0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
			0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
			0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
			0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
			0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
			0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
			0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
			0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
			0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
			0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
			0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
			0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
			0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
			0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
			0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
			0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
			0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
			0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
			0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
			0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
			0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
	};

//	#define  CRC_COMPUTE(x_crc,x_byte) (((x_crc)>>8)^wCRCTable[((x_crc)^(x_byte))&0x00ff])

 	WORD nTemp;
 	WORD wCRCWord = 0xFFFF; 
// 	while (wLength--) { 
//         nTemp = *nData++ ^ wCRCWord;
// 		wCRCWord >>= 8;
// 		wCRCWord ^= wCRCTable[nTemp]; 
//     } 

	WORD byteCheckSum2 = *nData >>8;
	WORD bTable =(*nData^wLength) & 0x00ff;

	nTemp = wCRCTable[bTable];
	wCRCWord = byteCheckSum2^nTemp;

	return wCRCWord;
}

void CFFUMonDlg::WriteInfoLog(CString strMsg)
{
	SYSTEMTIME st;
	::GetLocalTime(&st);
	CTime ct(st);

	CFile	fileLog;
	TCHAR szLog[6400];
	TCHAR szLogPath[6400];
	ZeroMemory(szLog,6400);	
	ZeroMemory(szLogPath,6400);

// 	sprintf(szLogPath,"%sInfoLog-%02d%02d%02d.log",m_strLogPath, st.wMonth, st.wDay,st.wHour);
// 	sprintf(szLog,"%s,  %s",gf_MakeCurTimeString(), strMsg);

	_stprintf_s(szLogPath, _countof(szLogPath), "%s\\%02d\\InfoLog-%02d%02d%02d.log",m_strLogPath, st.wMonth, st.wMonth,st.wDay,st.wHour);
	_stprintf_s(szLog,     _countof(szLog), "%s,  %s",gf_MakeCurTimeString(), strMsg);

	WriteFile(&fileLog,szLogPath,szLog);
}

BOOL CFFUMonDlg::PreTranslateMessage(MSG* pMsg) 
{
	switch( pMsg->wParam )
	{
	case VK_ESCAPE:
	case VK_RETURN:
		return TRUE;
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}

void CFFUMonDlg::OnOK() 
{
	OnMenuitemExit();
// 	if( AfxMessageBox("센서통신이 종료됩니다. 종료하시겠습니까?", MB_YESNO|MB_ICONQUESTION) != IDYES)
// 	if( AfxMessageBox("Do you want to close Sensor Viewer?", MB_YESNO|MB_ICONQUESTION) != IDYES)	// yj.nam 20170503 changed to English
// 	{
// 		return;
// 	}
// 	else
// 	{
// 		ConnectTCP( FALSE );
// 		SetEvent(m_hEvtGetStateStop);
// 		SetEvent(m_hEvtGetsDataStop);  //jh.lee 20160722 add
// 		SetEvent(m_hEvtMonitoringStop);
// 		ExitThreadAll();
// 
// 		for (int i=SENSOR_MODE_H2O; i<SENSOR_MODE_END; i++)
// 		{
// 			if(m_pDlgComm[i] != NULL)
// 				m_pDlgComm[i]->ExitThreadAll();
// 		}
// 
// 		//lhj 20171224 add ///////////////////////////
// 		if (m_pThreadESDRead)
// 		{
// 			m_dlgESD.m_bReceiveOut = TRUE;
// 			SetEvent(m_hEvtThreadESDReadKilled);
// 		}
// 		//////////////////////////////////////////////
// 
// 		CDialog::OnOK();
// 	}
}

void CFFUMonDlg::OnMini() 
{
	this->ShowWindow(SW_HIDE);
}

void CFFUMonDlg::SendEFUValue(int nEfu, double dVal, int nMode)
{
	CString str;
	str.Format("EFU%d^%.1f*", nEfu+1, m_dEFU[nEfu]);

	//20181217 edhan add
	/*int nID = 0;
	if(m_nMode == SENSOR_MODE_EFU)
	nID = 0;
	else if(m_nMode == SENSOR_MODE_EFU2)
	nID = 1;
	else if(m_nMode == SENSOR_MODE_EFU3)
	nID = 2;
	else if(m_nMode == SENSOR_MODE_EFU4)
	nID = 3;
	else if(m_nMode == SENSOR_MODE_EFU5)
	nID = 4;*/
	//
// jsyoon 20180803 remove
 	//if(m_bSocketConnect) //20150604 added
 	//	send(m_socetClient, str, str.GetLength(), 0);
	
	//if( nEfu > 32)
	//	WriteLogList(str, SENSOR_MODE_EFU2);
	//else
		WriteLogList(str, nMode);
}

void CFFUMonDlg::UpdateBtn(int iModule)
{
	CString str, strTitle;
	str.Empty();
	strTitle.Empty();
	bool bShow = false;

	if( iModule != m_iOldModule)
	{
		if( m_iOldModule != -1)
			m_chkModule[m_iOldModule].SetCheck(FALSE);
		else 
			m_chkModule[iModule].SetCheck(TRUE);

// 		switch (m_iOldModule)
// 		{
// 		case SENSOR_MODE_H2O:
// 			m_chkModule1.SetCheck(FALSE);
// 			break;
// 		case SENSOR_MODE_H2O2:
// 			m_chkModule2.SetCheck(FALSE);
// 			break;
// 		case SENSOR_MODE_TEMP1:
// 			m_chkModule3.SetCheck(FALSE);
// 			break;
// 		case SENSOR_MODE_TEMP2:
// 			m_chkModule4.SetCheck(FALSE);
// 			break;
// 		case SENSOR_MODE_TEMP3:
// 			m_chkModule5.SetCheck(FALSE);
// 			break;
// 		case SENSOR_MODE_TEMP4:
// 			m_chkModule6.SetCheck(FALSE);
// 			break;
// 		case SENSOR_MODE_TEMP5:
// 			m_chkModule7.SetCheck(FALSE);
// 			break;
// 		case SENSOR_MODE_TEMP6:
// 			m_chkModule8.SetCheck(FALSE);
// 			break;
// 		case SENSOR_MODE_TEMP7:
// 			m_chkModule9.SetCheck(FALSE);
// 			break;
// 		case SENSOR_MODE_TEMP8:
// 			m_chkModule10.SetCheck(FALSE);
// 			break;
// 		case SENSOR_MODE_PRESSURE:
// 			m_chkModule11.SetCheck(FALSE);
// 			break;
// 		case SENSOR_MODE_EFU:
// 			m_chkModule12.SetCheck(FALSE);
// 			break;
// 		case SENSOR_MODE_EFU2:
// 			m_chkModule19.SetCheck(FALSE);
// 			break;
// 		case SENSOR_MODE_KMN2:		//jhseo 20170920 add for KM-N2
// 			m_chkModuleKMN2.SetCheck(FALSE);
// 			break;
// 		case SENSOR_MODE_ESD:		//lhj 20171223 add
// 			m_chkModuleESD.SetCheck(FALSE);
// 			break;
// 		case SENSOR_MODE_ACCURA:
// 			m_chkModuleAccura.SetCheck(FALSE);
// 			break;
// 		case SENSOR_MODE_O2:
// 			m_chkModule16.SetCheck(FALSE);
// 			break;
// 		default:
// 			m_chkModule3.SetCheck(TRUE);
// 			break;
// 		}


// 		switch (iModule)
// 		{
// 		case SENSOR_MODE_H2O:
// //			str.Format("%d",m_uiPort);
// 			strTitle.Format("%s", _T("H20"));
// 			m_nMode = iModule/*SENSOR_MODE_H2O*/;
// 			break;
// // 		case SENSOR_MODE_H2O2:
// // 			str.Format("%d",m_uiPort2);
// // 			strTitle.Format("%s", _T("H20"));
// // 			m_nMode = iModule/*SENSOR_MODE_H2O*/;
// // 			break;
// 		case SENSOR_MODE_O2:
// //			str.Format("%d",m_uiPortO2);
// 			strTitle.Format("%s", _T("O2"));
// 			m_nMode = SENSOR_MODE_O2	;
// 			break;
// 		case SENSOR_MODE_TEMP1:
// // 		case SENSOR_MODE_TEMP2:
// // 		case SENSOR_MODE_TEMP3:
// // 		case SENSOR_MODE_TEMP4:
// // 		case SENSOR_MODE_TEMP5:
// // 		case SENSOR_MODE_TEMP6:
// // 		case SENSOR_MODE_TEMP7:
// // 		case SENSOR_MODE_TEMP8:
// // 			str.Format("%d",m_uiPort485[iModule-SENSOR_MODE_TEMP1]);
// //			strTitle.Format("%s", _T("온도"));
// 			strTitle.Format("%s", _T("Temperature"));	// yj.nam 20170503 changed to English
// 			m_nMode = iModule;
// 			break;
// 		case SENSOR_MODE_PRESSURE:
// //			str.Format("%d",m_uiPort6);
// //			strTitle.Format("%s", _T("압력"));
// 			strTitle.Format("%s", _T("Pressure"));	// yj.nam 20170503 changed to English
// 			m_nMode = SENSOR_MODE_PRESSURE;
// 			break;
// 		case SENSOR_MODE_EFU:
// //			str.Format("%d",m_uiPort7);
// 			strTitle.Format("%s", _T("EFU"));
// 			m_nMode = SENSOR_MODE_EFU;
// 			bShow = TRUE;
// 			break;
// 		case SENSOR_MODE_EFU2:
// //			str.Format("%d",m_uiPort9);
// 			strTitle.Format("%s", _T("EFU"));
// 			m_nMode = SENSOR_MODE_EFU2;
// 			bShow = TRUE;
// 			break;
// 		case SENSOR_MODE_KMN2:		//jhseo 20170920 add for KM-N2
// //			str.Format("%d",m_uiPortKMN2);
// 			strTitle.Format("%s", _T("KM-N2"));
// 			m_nMode = SENSOR_MODE_KMN2;
// 			break;
// 		case SENSOR_MODE_ESD:		//lhj 20171223 add 
// 			str= "";
// 			strTitle.Format("%s", _T("ESD"));
// 			m_nMode = SENSOR_MODE_ESD;
// 			break;
// 		case SENSOR_MODE_ACCURA:		//lhj 20171223 add 
// 			str= "";
// 			strTitle.Format("%s", _T("ACCURA"));
// 			m_nMode = SENSOR_MODE_ACCURA;
// 			break;
// 		}

		if( iModule == SENSOR_MODE_ACCURA)
		{
			str= "";
			strTitle.Format("%s", _T("ACCURA"));
		}
		else if( iModule == SENSOR_MODE_ESD || iModule == SENSOR_MODE_ESD2 || iModule == SENSOR_MODE_ESD3)
		{
			str= "";
			strTitle.Format("%s", _T("ESD"));
		}
		else if( iModule == SENSOR_MODE_KMN2)
		{
			str= "";
			strTitle.Format("%s", _T("KM-N2"));
		}
		else if( iModule == SENSOR_MODE_EFU)
		{
			str= "";
			strTitle.Format("%s", _T("EFU"));
			bShow = true;
		}
		//20181207 edhan add
		else if( iModule == SENSOR_MODE_EFU2)
		{
			str= "";
			strTitle.Format("%s", _T("EFU2"));
			bShow = true;
		}
		else if( iModule == SENSOR_MODE_EFU3)
		{
			str= "";
			strTitle.Format("%s", _T("EFU3"));
			bShow = true;
		}
		else if( iModule == SENSOR_MODE_EFU4)
		{
			str= "";
			strTitle.Format("%s", _T("EFU4"));
			bShow = true;
		}
		else if( iModule == SENSOR_MODE_EFU5)
		{
			str= "";
			strTitle.Format("%s", _T("EFU5"));
			bShow = true;
		}
		//
		else if( iModule == SENSOR_MODE_O2)
		{
			strTitle.Format("%s", _T("O2"));
		}
		else if( iModule == SENSOR_MODE_PRESSURE)
		{
			strTitle.Format("%s", _T("Pressure"));
		}
		else if( iModule >= SENSOR_MODE_H2O && iModule <= SENSOR_MODE_H2O_END  )
		{
			strTitle.Format("%s", _T("H20"));
		}
		else if( iModule >= SENSOR_MODE_TEMP1 && iModule <= SENSOR_MODE_TEMP_END  )
		{
			strTitle.Format("%s", _T("Temperature"));
		}
		else if( iModule == SENSOR_MODE_PM5500)
		{
			strTitle.Format("%s", _T("PM5500"));
		}
		else if( iModule == SENSOR_MODE_AZBIL)
		{
			strTitle.Format("%s", _T("AZBIL"));
		}

		m_nMode = iModule;
	
//		SetcboCurSel(&m_cboPort, str);
		strTitle.MakeUpper();
		SetDlgItemText(IDC_STATIC_STATE,strTitle);
		m_ListBox.ResetContent();
		SetControlShow(bShow);

		ShowBtnEFUCtrol(bShow);
		//20181207 edhan add
		if(bShow)
		{	
			m_cboBLIDStart.ResetContent();
			m_cboBLIDEnd.ResetContent();
		
			CString strIndx;
			for(int i = 0; i < 5; i++)
			{
				if(m_chkModule[SENSOR_MODE_EFU+i].GetCheck()) 
				{
					for(UINT j = 1; j <= m_nEFUCnt[i]; j++)
					{
						strIndx.Format(_T("%d"), j);
						m_cboBLIDStart.AddString(strIndx);
						m_cboBLIDEnd.AddString(strIndx);
					}
				}
			}
			m_cboBLIDStart.SetCurSel(0);
			m_cboBLIDEnd.SetCurSel(0);
		}
		//
	}
	else
	{
		m_chkModule[iModule].SetCheck(TRUE);

// 		switch (iModule)
// 		{
// 		case SENSOR_MODE_H2O:
// 			m_chkModule1.SetCheck(TRUE);
// 			break;
// 		case SENSOR_MODE_H2O2:
// 			m_chkModule2.SetCheck(TRUE);
// 			break;
// 		case SENSOR_MODE_TEMP1:
// 			m_chkModule3.SetCheck(TRUE);
// 			break;
// 		case SENSOR_MODE_TEMP2:
// 			m_chkModule4.SetCheck(TRUE);
// 			break;
// 		case SENSOR_MODE_TEMP3:
// 			m_chkModule5.SetCheck(TRUE);
// 			break;
// 		case SENSOR_MODE_TEMP4:
// 			m_chkModule6.SetCheck(TRUE);
// 			break;
// 		case SENSOR_MODE_TEMP5:
// 			m_chkModule7.SetCheck(TRUE);
// 			break;
// 		case SENSOR_MODE_TEMP6:
// 			m_chkModule8.SetCheck(TRUE);
// 			break;
// 		case SENSOR_MODE_TEMP7:
// 			m_chkModule9.SetCheck(TRUE);
// 			break;
// 		case SENSOR_MODE_TEMP8:
// 			m_chkModule10.SetCheck(TRUE);
// 			break;
// 		case SENSOR_MODE_PRESSURE:
// 			m_chkModule11.SetCheck(TRUE);
// 			break;
// 		case SENSOR_MODE_EFU:
// 			m_chkModule12.SetCheck(TRUE);
// 			break;
// 		case SENSOR_MODE_KMN2:		//jhseo 20170920 add for KM-N2
// 			m_chkModuleKMN2.SetCheck(TRUE);
// 			break;
// 		case SENSOR_MODE_ESD:		//lhj 20171223 add
// 			m_chkModuleESD.SetCheck(TRUE);
// 			break;
// 		case SENSOR_MODE_ACCURA:
// 			m_chkModuleAccura.SetCheck(TRUE);
// 			break;
// 		case SENSOR_MODE_O2:
// 			m_chkModule16.SetCheck(TRUE);
// 			break;
// 		case SENSOR_MODE_EFU2:
// 			m_chkModule19.SetCheck(TRUE);
// 			break;
// 		default:
// 			m_chkModule3.SetCheck(TRUE);
// 			break;
//		}
	}

	m_iOldModule = iModule;

}

void CFFUMonDlg::OnChkKmn2() 		//jhseo 20170920 add for KM-N2
{
	// TODO: Add your control notification handler code here
	UpdateBtn(SENSOR_MODE_KMN2);
}

void CFFUMonDlg::OnChkEsd() 
{
	UpdateBtn(SENSOR_MODE_ESD); //lhj 20171223 add
	
}

UINT CFFUMonDlg::ThreadESDReadProc(LPVOID pParam) 
{
	CFFUMonDlg* pProc = (CFFUMonDlg*) pParam;
	
	BOOL bRet = FALSE;
	UINT i=0;
	while (TRUE)
	{	
		bRet = TRUE;

		for(i=0; i<pProc->m_nCnt_ESD;i++)
		{
			if( pProc->m_pDlgESD[i] != NULL)
				bRet &= pProc->m_pDlgESD[i]->DoWorkStartGetESD();
		}

		//		bRet = pProc->m_dlgESD.DoWorkStartGetESD();
		
		pProc->m_bESDReadDone = bRet;
		
		if (WaitForSingleObject(pProc->m_hEvtThreadESDReadKilled, 1000)
			== WAIT_OBJECT_0)
		{
			break;
		}
	}
	
	pProc->m_bESDReadDone = FALSE;
	
	SetEvent(pProc->m_hEvtThreadESDReadTerminated);
	TRACE("ThreadESDReadProc OUT\n");

	pProc->m_pThreadESDRead = NULL;
	
	return 0;
}


//jh.lee 20160722 add - AccuraData Read & Save
void CFFUMonDlg::ReadMeasurement(AccuraModbusTcpClient* client, int index)  
{
	CString csStr;

	try {
		unsigned short productId = client->GetProductId();
		CString s, str;

		if (productId == 2300) 
		{
			unsigned short fetchAndValidity[42];
			ZeroMemory(&fetchAndValidity, sizeof(fetchAndValidity));
//			unsigned short* selectedIndex = &fetchAndValidity[0];
			unsigned short* ptValidity = &fetchAndValidity[1];
			unsigned short* subunitValidity = &fetchAndValidity[2];
			int i = 0;

			ZeroMemory(&m_sData, sizeof(m_sData));

			client->ReadHoldingRegisters(11044, 42, fetchAndValidity);

			if (*ptValidity == 1) 
			{
				unsigned short ptData[56];

				client->ReadHoldingRegisters(11101, 56, ptData);
				
				m_sData.Vavg_ll_1 = ToFloat(ptData, 14);

// 				if(index == 0)
// 				{
// 					m_sData.Vavg_ll_1 = ToFloat(ptData, 14);  //삼상의 선간전압 평균(Acura#1)
// //					API_SM_GetAcuraData_Vol_1(sData.Vavg_ll_1);
// 				}
// 				else if(index == 1)
// 				{
// 					m_sData.Vavg_ll_1 = ToFloat(ptData, 14);  //삼상의 선간전압 평균(Acura#2)
// //					API_SM_GetAcuraData_Vol_2(sData.Vavg_ll_2);
// 				}
// 				else
// 				{
// 					;
// 				}
			}
			else 
			{
				s = _T("pt data invalid.");
			}
						
			for (i=0; i<40; i++)
			{
				unsigned short subunitData[150];

				if (subunitValidity[i] == 0)
				{
					client->ReadHoldingRegisters(11201 + i*150, 141, subunitData);

//					if(index == 0)
					{
						m_sData.lavg_1 = ToFloat(subunitData, 6);   //삼상의 전류 평균(Acura#1)
						m_sData.Ptot_1 = ToFloat(subunitData, 66);  //삼상의 유효전력 종합(Acura#1)
						m_sData.Net_1  = ToInt32(subunitData, 92);  //수전 유효전력량과 송전 유효전력량의 차(Acura#1)
						s.Format(_T("Vavg = %f, IAvg = %f, P_Total=%f, E=%d"), m_sData.Vavg_ll_1, m_sData.lavg_1, m_sData.Ptot_1, m_sData.Net_1);
//						API_SM_GetAcuraData_Cur_1(sData.lavg_1, sData.Ptot_1, sData.Net_1);
						
						csStr.Format("[Accura Panel#%d Dev#0] : data%d : %s", index+1, i, s);
						WriteLogList(csStr, SENSOR_MODE_ACCURA);
						
					}
				}
				else if (subunitValidity[i] == 1) 
				{
					client->ReadHoldingRegisters(11201 + i*150, 56, subunitData);
					
					m_sData.lavg_1 = ToFloat(subunitData, 0);   //삼상의 전류 평균(Acura#2)
					m_sData.Ptot_1 = ToFloat(subunitData, 16);  //삼상의 유효전력 종합(Acura#2)
					m_sData.Net_1  = ToInt32(subunitData, 30);  //수전 유효전력량과 송전 유효전력량의 차(Acura#2)
					s.Format(_T("Vavg = %f, IAvg = %f, P_Total=%f, E=%d"), m_sData.Vavg_ll_1, m_sData.lavg_1, m_sData.Ptot_1, m_sData.Net_1);
//						API_SM_GetAcuraData_Cur_2(sData.lavg_2, sData.Ptot_2, sData.Net_2);

					csStr.Format("[Accura Panel#%d Dev#0] : data%d : %s", index+1, i, s);
					WriteLogList(csStr, SENSOR_MODE_ACCURA);
				}
				else if (subunitValidity[i] == 3) 
				{
					client->ReadHoldingRegisters(11201 + i*150, 140, subunitData);
				
					s.Empty();
					m_sData.nValueCnt = 0;
//					if(index == 0)
					{
						// jsyoon 20190318 change
						for (int iVal=0; iVal<20; iVal++)
						{
							m_sData.nValue[iVal]      = ToInt32(subunitData, 20+(iVal*4));
							m_sData.nValueTotal[iVal] = ToInt32(subunitData, 22+(iVal*4));

							if( m_sData.nValueTotal[iVal] > 0)
							{
								m_sData.nValueCnt++;
								str.Format(_T("Value#%d_Cur=%d, Value#%d_Total = %d,"), iVal+1, m_sData.nValue[iVal], iVal+1, m_sData.nValueTotal[iVal]);
								s += str;
							}
						}
// 						m_sData.N2_1 =		ToInt32(subunitData, 20);  //N2_1 수치(Acura#1)		
// 						m_sData.N2_1_Total =  ToInt32(subunitData, 22);  //N2_1 총합수치(Acura#1)  
// 						m_sData.N2_2 =		ToInt32(subunitData, 24);  //N2_2 수치(Acura#1)
// 						m_sData.N2_2_Total =  ToInt32(subunitData, 26);  //N2_2 총합수치(Acura#1)
// 
// 						m_sData.CDA_1 =       ToInt32(subunitData, 28);  //CDA_1 수치(Acura#1) 
// 						m_sData.CDA_1_Total = ToInt32(subunitData, 30);  //CDA_1 총합수치(Acura#1)

						m_sData.nSubDev++;
					}
										
					// jsyoon 20190318 change
// 					s.Format(_T("Value#1_Cur=%d, Value#1_Total = %d, Value#2_Cur=%d, Value#2_Total = %d, Value#3_Cur=%d, Value#3_Total=%d")
// 						, m_sData.N2_1, m_sData.N2_1_Total, m_sData.N2_2, m_sData.N2_2_Total, m_sData.CDA_1, m_sData.CDA_1_Total);

// 					s.Format(_T("Value#1_Cur=%d, Value#1_Total = %d, Value#2_Cur=%d, Value#2_Total = %d, Value#3_Cur=%d, Value#3_Total=%d")
// 						, m_sData.N2_1, m_sData.N2_1_Total, m_sData.N2_2, m_sData.N2_2_Total, m_sData.CDA_1, m_sData.CDA_1_Total);

					csStr.Format("[Accura Panel#%d Dev#1] : data%d : %s", index+1, i, s);
					WriteLogList(csStr, SENSOR_MODE_ACCURA);
				}
				else
				{
					;
// 					csStr.Format("[Accura Panel#%d Dev#1] : data%d : %d", index+1, i, subunitValidity[i] );
// 					WriteLogList(csStr, SENSOR_MODE_ACCURA);
				}
			}

			m_pAccuraData[index] = m_sData;		// jsyoon 20180625
		} 
		else 
		{
			s.Format(_T("unsupported device. productId = %d"), productId);
		}
		//SendMessage(RECEIVE_INFO, index, (LPARAM)&s);
	} catch (enum ModbusTcpError) {
		//SendMessage(RECEIVE_INFO, index, (LPARAM)NULL);
		csStr.Format("no data at %d", index+1);
		WriteLogList(csStr, SENSOR_MODE_ACCURA);
	}
}

//jh.lee 20160722 add 
void CFFUMonDlg::AcuraReadMeasure()  
{
	if( m_cDATA.m_nStop == TRUE)
		return;

	int i=0;
	int numOfClients = 0;	
	clientsArray = m_rAcClient.GetDevicesAsAllocatedArray(numOfClients);
	
	CString csStr;
	csStr.Format("Number of Clients : %d", numOfClients);
	WriteLogList(csStr, SENSOR_MODE_ACCURA);
	
	for (i=0; i<numOfClients; i++) {
		AccuraModbusTcpClient* client = clientsArray[i];
		client->EnableException();
	}
	
	for (i=0; i<numOfClients; i++) {
		AccuraModbusTcpClient* client = clientsArray[i];
		ReadMeasurement(client, i);
	}
}

BOOL CFFUMonDlg::AddDevice(CString ipAddr)  
{
	Extract_Convert(ipAddr);
	m_rAcClient.AddDevice(m_cDATA.m_Address);
	return TRUE;


	CString ipText;
	ipText.Format(_T("%d.%d.%d.%d"), (m_cDATA.m_Address >> 24) & 0xff, (m_cDATA.m_Address >> 16) & 0xff, (m_cDATA.m_Address >> 8) & 0xff, (m_cDATA.m_Address >> 0) & 0xff);
	TRACE("Ip address %s\n", ipText);
}

//jh.lee 20160722 add
void CFFUMonDlg::ReadDevinfo()  
{
	if(m_rAcClient.Count()<1)
	{
		TRACE("no client\n");
		return;
	}
	
	AccuraModbusTcpClient* client = m_rAcClient.At(0);
//	AccuraModbusTcpClient* client2 = m_rAcClient.At(1);
	unsigned short buffer[10];
	bool success = false;
	
	client->DisableException();
	success = client->ReadHoldingRegisters(1, 10, buffer);
	
	if (success == true) 
	{
		CString s;
		s.Format(_T("device=%d"), buffer[0]);
		TRACE("Device info %s\n", s);
	} 
	else
		TRACE("no Device info\n");
}

float CFFUMonDlg::ToFloat(unsigned short* buffer, int index)  
{
	unsigned short reversed[2];
	
	reversed[0] = buffer[index + 1];
	reversed[1] = buffer[index + 0];
	
	return *(float*)reversed;
}

//jh.lee 20160722 add 
int CFFUMonDlg::ToInt32(unsigned short* buffer, int index)   
{
	unsigned short reversed[2];
	
	reversed[0] = buffer[index + 1];
	reversed[1] = buffer[index + 0];
	
	return *(int*)reversed;
}

//jh.lee 20160722 add
int CFFUMonDlg::ToInt32new(unsigned short* buffer, int index)  
{
	unsigned short reversed[2];
	
	reversed[0] = buffer[index + 0];
	reversed[1] = buffer[index + 1];
	
	return *(int*)reversed;
}

//jh.lee 20160722 add
CString CFFUMonDlg::IntToHex_String(int Addr_int)  
{
	char Temp[40];
	
//	_itoa(Addr_int, Temp, 16);
	_itoa_s(Addr_int, Temp, 16);	// jsyoon 20180725 change
	
	return Temp;
}

//jh.lee 20160725 add
DWORD CFFUMonDlg::HexStrToInt(CString m_Addr)
{
	DWORD        rVal;
	char         * string, * stopstring;
	
	string = (char *)(LPCTSTR)m_Addr;
	
	strtod(string, &stopstring);
	
	rVal = (DWORD)strtoul(string, &stopstring, 16);
	
	return rVal;
}

//jh.lee 20160722 add - Convert Cstring -> DWORD for Ip_Address
void CFFUMonDlg::Extract_Convert(CString m_strIP_Acura)  
{
	int cnt = 0;
	
	m_cDATA.m_Addr.Empty();
	
	while(true)
	{
		if(!AfxExtractSubString(m_cDATA.m_Addr_str, m_strIP_Acura, cnt++, '.'))  //delemeter 점(.) 기준으로 문자열 나누기
		{
			break;
		}
		else
		{
			m_cDATA.m_Addr_int = _ttoi(m_cDATA.m_Addr_str);  //나눈 문자열 CString -> int형으로 변환
			
			m_cDATA.m_Addr_hex = IntToHex_String(m_cDATA.m_Addr_int);  //int변환형을 Hex(CString)값으로 변환
			
			if(m_cDATA.m_Addr_hex.GetLength() < 2)  
			{
				m_cDATA.m_Addr_hex.Insert(0,"0");  //Hex(CString)변환값이 한자리이면 앞에 0을 붙여준다.(Hex변환값은 2자리수로 유지)
			}

			m_cDATA.m_Addr.Insert(cnt*2, m_cDATA.m_Addr_hex);  //변환된 Hex(CString)값들을 문자열에 순서대로 추가한다.
		}
	}
	
	//	if(m_Addr.Find("0x",0))
	//	{
	//		m_Addr.Insert(0, "0x");  //문자열에 Hex(CString)값들을 넣어준 후 헤더에 0x를 추가
	//	}
	
	m_cDATA.m_Address = HexStrToInt(m_cDATA.m_Addr);  //CString 16진수를 DWORD10진수로 변환
	//	m_Address = (DWORD)_ttoi((LPCTSTR)m_Addr);
	//	m_Address = atol((char*)(LPCTSTR)m_Addr);  
	//	m_Address = static_cast<DWORD>(_ttoi((LPCTSTR)m_Addr)); 
}

//jh.lee 20160722 add - Get Data Thread
void CFFUMonDlg::StartThreadGetsData()
{
	//AfxBeginThread(WorkThreadProcGetState, this);
	
	if(m_pGetsDataThread == NULL)
	{
//		m_hEvtGetsDataStop = CreateEvent(NULL, FALSE, FALSE, NULL);		//jsyoon 20181124 remove
		m_pGetsDataThread = AfxBeginThread(WorkThreadProcGetsData, this);
		m_pGetsDataThread->m_bAutoDelete = FALSE;
	}
}

//jh.lee 20160722 add - Get Data Thread
UINT CFFUMonDlg::WorkThreadProcGetsData(LPVOID pParam)
{
	CFFUMonDlg *pProc = (CFFUMonDlg*) pParam;
	volatile BOOL bLoop  = TRUE;
	while(bLoop)
	{
		DWORD res = WaitForSingleObject(pProc->m_hEvtGetsDataStop, 1000);
		switch(res)
		{
		case WAIT_OBJECT_0:
			bLoop = FALSE;
			break;
		case WAIT_TIMEOUT:
			pProc->AcuraReadMeasure();
		}
	}
	pProc->m_pGetsDataThread = NULL;
	return TRUE;
}

void CFFUMonDlg::OnButtonTemp6() 
{
	// TODO: Add your control notification handler code here
	UpdateBtn(SENSOR_MODE_TEMP1);
}

void CFFUMonDlg::OnChkAccura() 
{
	// TODO: Add your control notification handler code here
	UpdateBtn(SENSOR_MODE_ACCURA);
}

BOOL CFFUMonDlg::DestroyWindow() 
{
	// TODO: Add your specialized code here and/or call the base class

	NOTIFYICONDATA  nid;
	nid.cbSize = sizeof(nid);
	nid.hWnd = m_hWnd;
	nid.uID = IDI_TRAYICON;	
	Shell_NotifyIcon(NIM_DELETE, &nid);

	//yhjoo 20141120 add
	m_bRecvThreadRun = FALSE;
	if(m_hRecvThread)
	{
		::WaitForSingleObject(m_hRecvThread, 1000);
		if (m_hRecvThread)
		{
			CloseHandle(m_hRecvThread);
			m_hRecvThread = NULL;
			
			closesocket(m_socetClient);
			WSACleanup();
		}
	}
	/////////////////////////////////////



	for (int i=SENSOR_MODE_H2O; i<SENSOR_MODE_END; i++)
	{
		if(m_pDlgComm[i] != NULL)
			m_pDlgComm[i]->ExitThreadAll();
	}

	ExitThreadAll();
	
	//20140729 added by Jay Hong
	if( m_pWarnMsgDlg ) {
		m_pWarnMsgDlg->DestroyWindow();
		delete m_pWarnMsgDlg;		
		m_pWarnMsgDlg = NULL;
	}

	int i= 0;
	for (i=SENSOR_MODE_H2O; i<SENSOR_MODE_END; i++)
	{
		if (m_pDlgComm[i] != NULL)
		{
			m_pDlgComm[i]->DestroyWindow();
			delete m_pDlgComm[i];
			m_pDlgComm[i] = NULL;
		}
	}

	for (i=0; i<10; i++)
	{	
		if( m_pDlgESD[i] != NULL)
		{
			m_pDlgESD[i]->DestroyWindow();
			delete m_pDlgESD[i];
			m_pDlgESD[i] = NULL;
		}
	}

	//lhj 20141224 /////////////////////////////////////
//	if (m_hEvtThreadESDReadTerminated)
		CloseHandle(m_hEvtThreadESDReadTerminated);

//	if (m_hEvtThreadESDReadKilled)
		CloseHandle(m_hEvtThreadESDReadKilled);

	////////////////////////////////////////////////

	if( m_pAccuraData != NULL)
	{
		delete[] m_pAccuraData;
		m_pAccuraData = NULL;
	}

	CloseHandle(m_hEvtGetsDataStop);
	SAFE_DELETE(m_pThreadESDRead);
//	SAFE_DELETE(m_pGetStateThread);
//	SAFE_DELETE(m_pMonitoringThread);
	SAFE_DELETE(m_pGetsDataThread);
//	SAFE_DELETE(m_pRcvThread);
	
	return CDialog::DestroyWindow();
}

BOOL CFFUMonDlg::SHRegDeleteValue(HKEY hKey, LPCTSTR lpKey, LPCTSTR lpValue)
{
	HKEY  key;
	
	if(RegOpenKeyEx(hKey, lpKey, 0, KEY_ALL_ACCESS, &key)!=ERROR_SUCCESS) 
		return FALSE;
	
	if(RegDeleteValue(key, lpValue)!=ERROR_SUCCESS) 
		return FALSE;
	
	RegCloseKey(key);
	
	return TRUE;
}

BOOL CFFUMonDlg::SHRegWriteString(HKEY hKey, LPCTSTR lpKey, LPCTSTR lpValue, LPCTSTR lpData)
{
	HKEY  key;
	DWORD dwDisp;
	
	if(RegCreateKeyEx(hKey, lpKey,0,NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE,NULL,&key,&dwDisp)!=ERROR_SUCCESS) 
		return FALSE;
	
	if(RegSetValueEx(key, lpValue,0,REG_SZ,(LPBYTE)lpData,strlen(lpData)+1)!=ERROR_SUCCESS) 
		return FALSE;
	
	RegCloseKey(key);
	
	return TRUE;
}

BOOL CFFUMonDlg::SHRegReadString(HKEY hKey, LPCTSTR lpKey, LPCTSTR lpValue, LPCTSTR lpDefault, LPTSTR lpRet, DWORD nSize)
{
	HKEY key;	
	DWORD dwDisp;	
	DWORD Size;	
	BOOL bRet = FALSE;	
	
	if(RegCreateKeyEx(hKey, lpKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ, NULL, &key, &dwDisp) != ERROR_SUCCESS)		
	{		
		return FALSE;		
	}	
	
	Size = nSize;
	
	if(RegQueryValueEx(key, lpValue, 0, NULL,(LPBYTE)lpRet, &Size) != ERROR_SUCCESS)		
	{		
		lstrcpy(lpRet, lpDefault);		
	}	
	else		
	{		
		bRet = TRUE;		
	}	
	
	RegCloseKey(key);
	
	return bRet;
}


LRESULT CFFUMonDlg::TrayIconMSG(WPARAM wParam, LPARAM lParam)
{
	wParam;

	switch(lParam)
	{
	case WM_LBUTTONDBLCLK:
		{ 
			NOTIFYICONDATA nid;
			
			nid.cbSize = sizeof(NOTIFYICONDATA);
			nid.hWnd = this->m_hWnd;
			nid.uID = 0;
			Shell_NotifyIcon(NIM_DELETE, &nid);
			
			ShowWindow(SW_RESTORE);
			SetForegroundWindow();
		}
		break;
	case WM_RBUTTONDOWN:     
		{
			CPoint ptMouse;
			::GetCursorPos(&ptMouse);
			
			char szStr[MAX_PATH*3]={0};
			CMenu menu;
			menu.LoadMenu(IDR_MENU_TRAYICON);			
			CMenu *pMenu = menu.GetSubMenu(0);
			
			if(SHRegReadString(HKEY_CURRENT_USER, RUNKEY, AUTORUN_VALUENAME, "", szStr, MAX_PATH*2))
			{
				CString str, strTmp;
				str=szStr;
				str.TrimLeft();
				str.TrimRight();
				
				if(str.GetLength() > 0)
				{
					if(str.GetAt(0)=='\"') str.Delete(0);
					
					// 					int index1=str.Find(' ');
					// 					str.Delete(index1-1, str.GetLength()+1-index1);
					int index1=str.Find(_T(".exe"));
					strTmp = str.Left(index1+4);
				}
				
				if(strTmp.CompareNoCase(__argv[0])==0)
				{
					pMenu->CheckMenuItem(ID_MENUITEM_WINDOWSTARTAUTORUN, MF_CHECKED);
				}
			}			
			pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptMouse.x, ptMouse.y, AfxGetMainWnd());
		}
	}
	return 0;
}

void CFFUMonDlg::ResTrayIcon()
{
	NOTIFYICONDATA nid;
	nid.cbSize = sizeof(nid);
	nid.hWnd = m_hWnd;
	nid.uID = IDI_TRAYICON;
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	nid.uCallbackMessage = WM_TRAYICON_MSG;
	nid.hIcon = AfxGetApp()->LoadIcon(IDI_TRAYICON); 

	CString strAppName;
	if( m_strEQName.IsEmpty() == TRUE)
		strAppName.Format("KMAC TOOLS");
	else
		strAppName.Format("KMAC TOOLS_%s", m_strEQName);

	
	lstrcpy(nid.szTip,strAppName);
	Shell_NotifyIcon(NIM_ADD, &nid);
	
	SendMessage(WM_SETICON, (WPARAM)TRUE, (LPARAM)nid.hIcon);
	m_bTrayIcon = TRUE;


	CString strVer = GetVersionInfo();
	strVer.Replace(",", ".");
	CString ver_str = "Ver: " + strVer;

	// jsyoon 20181115 add
	CWnd *ptr = AfxGetMainWnd();
	CString str;
	str.Format(_T("%s, %s"), strAppName, ver_str);
	ptr->SetWindowText(str);
	//////////////////////////////////////////////////////////////////////////

}

void CFFUMonDlg::OnMenuitemExit() 
{
	// TODO: Add your command handler code here
//	WriteLogFile("OnMenuitemExit() Exit Button Click!");
	if( FFUMonDlgExit() == true )
		CDialog::OnOK();
}

void CFFUMonDlg::OnMenuitemOpen() 
{
	// TODO: Add your command handler code here
	NOTIFYICONDATA nid;
	
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = this->m_hWnd;
	nid.uID = 0;
	Shell_NotifyIcon(NIM_DELETE, &nid);
	
	ShowWindow(SW_RESTORE);
	SetForegroundWindow();
}

void CFFUMonDlg::OnMenuitemWindowstartautorun() 
{
	// TODO: Add your command handler code here
	CString strWrite;
	char    szStr[MAX_PATH*3]={0};
	
	if(SHRegReadString(HKEY_CURRENT_USER, RUNKEY, AUTORUN_VALUENAME, " ", szStr, MAX_PATH*2))
	{
		CString str, strTmp;
		str=szStr;
		str.TrimLeft();
		str.TrimRight();
		if (str.GetLength() > 0)
		{
			if (str.GetAt(0)=='\"') str.Delete(0);
			
			//			int index1=str.Find(' ');
			//			str.Delete(index1-1, str.GetLength()+1-index1);			
			int index1=str.Find(_T(".exe"));
			strTmp = str.Left(index1+4);
		}
		
		if(strTmp.CompareNoCase(__argv[0])!=0)
		{
			strWrite="\""; 
			strWrite+= __argv[0];
			strWrite+= "\" ";
			strWrite+= AUTORUN_ADDARGU;
			SHRegWriteString(HKEY_CURRENT_USER, RUNKEY, AUTORUN_VALUENAME, strWrite);
			AfxMessageBox("Register WriteString");
		}
		else
		{
			SHRegDeleteValue(HKEY_CURRENT_USER, RUNKEY, AUTORUN_VALUENAME);
		}
	}
	else
	{
		strWrite="\""; 
		strWrite+= __argv[0];
		strWrite+= "\" ";
		strWrite+= AUTORUN_ADDARGU;
		SHRegWriteString(HKEY_CURRENT_USER, RUNKEY, AUTORUN_VALUENAME, strWrite);
	}
}


bool CFFUMonDlg::FFUMonDlgExit()
{
	if( AfxMessageBox("Do you want to close Sensor Viewer?", MB_YESNO|MB_ICONQUESTION) != IDYES)	// yj.nam 20170503 changed to English
	{
		return false;
	}
	else
	{
		KillTimer (TIMER_GET_DATA);
		KillTimer(102);

		//KillTimer(1);
		//KillTimer(2);
		//KillTimer(10);	//yhjoo 20141120 add
		//KillTimer(100);	//20140729 added by Jay Hong
		//KillTimer(102);

		api_SerialCom_Close(m_uiPort);
		ConnectTCP( FALSE );
		SetEvent(m_hEvtGetStateStop);
		SetEvent(m_hEvtGetsDataStop);  //jh.lee 20160722 add
//		SetEvent(m_hEvtMonitoringStop);

		for (int i=SENSOR_MODE_H2O; i<SENSOR_MODE_END; i++)
		{
			if(m_pDlgComm[i] != NULL)
				m_pDlgComm[i]->ExitThreadAll();
		}

		UINT i=0;
		//lhj 20171224 add ///////////////////////////
		if ( m_pThreadESDRead != NULL)
		{
			for (i=0; i<m_nCnt_ESD; i++)
			{
				if( m_pDlgESD[i] != NULL)
					m_pDlgESD[i]->m_bReceiveOut = TRUE;
			}

//			m_dlgESD.m_bReceiveOut = TRUE;
			SetEvent(m_hEvtThreadESDReadKilled);
		}
		//////////////////////////////////////////////

		if (m_bUse_ESD)
		{
			// 소켓 통신 연결 
			for (i=0; i<m_nCnt_ESD; i++)
			{
				if( m_pDlgESD[i] != NULL)
					m_pDlgESD[i]->DisConnectESD(10+i);
			}
//			m_dlgESD.SocketDisConnectESD(m_strIPESD,m_strPortESD);
		}

//		WriteLogList(_T("Port Close!!"), SENSOR_MODE_H2O);
	}

	return true;
}

void CFFUMonDlg::OnBnClickedChk10()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateBtn(SENSOR_MODE_TEMP1);
}


void CFFUMonDlg::OnBnClickedChk9()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateBtn(SENSOR_MODE_TEMP1);
}


void CFFUMonDlg::OnBnClickedChk16()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateBtn(SENSOR_MODE_O2);
}


void CFFUMonDlg::OnBnClickedChk19()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateBtn(SENSOR_MODE_EFU2);
}


int CFFUMonDlg::ShowBtnEFUCtrol(bool bShow)
{
	if( m_bShowEFU != bShow)
	{
		CRect rect, rect2;
		this->GetWindowRect(rect);

		GetDlgItem(IDC_STATIC_EFU)->GetWindowRect(rect2);
		LONG  lWidth = 0;
		lWidth = rect2.Width() + 10;

		if (bShow == false)
		{
			rect.right = rect.right - lWidth;
		}
		else
		{
			rect.right = rect.right + lWidth;
		}

		this->MoveWindow(rect,TRUE);

		m_bShowEFU = bShow;
	}

	return 0;
}

LRESULT	CFFUMonDlg::OnTrayReset(WPARAM wParam, LPARAM lParam)	//	yj.k 20180201 add explorer shotdown -> restart -> TrayIcon Retry show
{
	ResTrayIcon();
	return	0;
}

CString CFFUMonDlg::GetVersionInfo()
{
	//	HMODULE		hLib;
	CString		csRet;
	CString     szVersion, szString;
	//	CString		strAppPath;
	CString		strSubBlock;
	UINT		cbTranslate;

	TCHAR szBuff[_MAX_PATH];
	szVersion.Empty();

	::GetModuleFileName(::AfxGetApp()->m_hInstance, szBuff, _MAX_PATH);	

	char* pch = ::strrchr(szBuff, '\\');
	//	strAppPath = CString(szBuff, pch - szBuff + 1);

	DWORD dummy;
	CString strFilePath;

	//예제	
	//	strFilePath.Format("%s\\%s", "E:\\1. Project\\2015\\1.SDC\\A3\\ICM\\Source\\ICM_20150128\\Bin", "Metrosys.exe");
	//	strFilePath.Format("%s%s", strAppPath, EXE_NAME);	
	strFilePath.Format("%s", szBuff);
	DWORD dwSize = GetFileVersionInfoSize((LPSTR)(LPCTSTR)strFilePath, &dummy);
	if (dwSize == 0)	
	{
		TRACE("GetFileVersionInfoSize failed with error %d\n", GetLastError());
		return szVersion;
	}

	BYTE*	pData = new BYTE[ dwSize ];

	// 파일버전 정보 읽기
	if (!GetFileVersionInfo( (LPSTR)(LPCTSTR)strFilePath, NULL, dwSize, pData ))
	{
		TRACE("GetFileVersionInfo failed with error %d\n", GetLastError());
		delete pData;
		return szVersion;	
	}

	// 파일버전 정보를 읽기위한 변수설정 ( 파일버전 정보는 언어 리소스별로 구분되어 저장됨. )
	LPVOID pvProductVersion = NULL;
	LPVOID pvFileVersion = NULL;

	struct LANGANDCODEPAGE 	
	{
		WORD wLanguage;
		WORD wCodePage;
	}*lpTranslate;

	VerQueryValue( pData, "\\VarFileInfo\\Translation", (LPVOID*)&lpTranslate, (PUINT)&cbTranslate);

	// 파일버전 정보에서 첫번째 언어에 해당하는 data를 읽어옴
	// 다중언어 리소스에서는 정확한 동작을 위해서는 data를 읽기 전에 언어코드, 코드페이지 정보를 지정하여야 함.
	// 버전에 관한 언어, 코드페이지 정보는 리소스파일의 "Block Header"에 기입되어 있음.

	//	strSubBlock.Format( "\\StringFileInfo\\%04x%04x\\ProductVersion", lpTranslate[0].wLanguage, lpTranslate[0].wCodePage);
	strSubBlock.Format( "\\StringFileInfo\\%04x%04x\\ProductVersion", lpTranslate[0].wLanguage, lpTranslate[0].wCodePage);
	if (!VerQueryValue(pData, (LPSTR)(LPCTSTR)strSubBlock, &pvProductVersion, (PUINT)&dummy))
	{
		TRACE("Can't obtain ProductVersion from resources\n");
		delete pData;
		return szVersion;
	}

	strSubBlock.Format( "\\StringFileInfo\\%04x%04x\\FileVersion", lpTranslate[0].wLanguage, lpTranslate[0].wCodePage);
	if (!VerQueryValue(pData, (LPSTR)(LPCTSTR)strSubBlock, &pvFileVersion, (PUINT)&dummy))
	{
		TRACE("Can't obtain FileVersion from resources\n");
		delete pData;
		return szVersion;
	}

	szVersion = (LPCTSTR)pvFileVersion;
	delete pData;

	return szVersion;
}

void CFFUMonDlg::OnBnClickedButtonGetSpeed()
{
	// TODO: Add your control notification handler code here
	CString strcboLVID, strcboBLIDStart, strcboBLIDEnd, strcboSV;
	GetDlgItem(IDC_COMBO_LV32ID)->GetWindowText(strcboLVID);
	GetDlgItem(IDC_COMBO_BLSTART)->GetWindowText(strcboBLIDStart);
	GetDlgItem(IDC_COMBO_BLEND)->GetWindowText(strcboBLIDEnd);
	GetDlgItem(IDC_EDIT_RPM)->GetWindowText(strcboSV);

	int iValue = 0;
	int iELIDStart = 0, iBLIDEnd = 0;
	iELIDStart = atoi(strcboBLIDStart);
	iBLIDEnd   = atoi(strcboBLIDEnd);

	if( iELIDStart > iBLIDEnd)
	{
		AfxMessageBox("NG BL ID Select!!");
		return;
	}

	if (m_bEFU_Type == EFU_TYPE_EBM)
	{
		if(m_chkModule[SENSOR_MODE_EFU].GetCheck())
		{
			if(m_pDlgComm[SENSOR_MODE_EFU] != NULL)
				m_pDlgComm[SENSOR_MODE_EFU]->GetEFUSpeed(1, iELIDStart, iBLIDEnd);
		}
		else if(m_chkModule[SENSOR_MODE_EFU2].GetCheck())
		{
			if(m_pDlgComm[SENSOR_MODE_EFU2] != NULL)
				m_pDlgComm[SENSOR_MODE_EFU2]->GetEFUSpeed(1, iELIDStart, iBLIDEnd);
		}
		else if(m_chkModule[SENSOR_MODE_EFU3].GetCheck())
		{
			if(m_pDlgComm[SENSOR_MODE_EFU3] != NULL)
				m_pDlgComm[SENSOR_MODE_EFU3]->GetEFUSpeed(1, iELIDStart, iBLIDEnd);
		}
		else if(m_chkModule[SENSOR_MODE_EFU4].GetCheck())
		{
			if(m_pDlgComm[SENSOR_MODE_EFU4] != NULL)
				m_pDlgComm[SENSOR_MODE_EFU4]->GetEFUSpeed(1, iELIDStart, iBLIDEnd);
		}
		else if(m_chkModule[SENSOR_MODE_EFU5].GetCheck())
		{
			if(m_pDlgComm[SENSOR_MODE_EFU5] != NULL)
				m_pDlgComm[SENSOR_MODE_EFU5]->GetEFUSpeed(1, iELIDStart, iBLIDEnd);
		}
	}
	else if (m_bEFU_Type == EFU_TYPE_HKC_H5)
	{
		m_pDlgComm[SENSOR_MODE_EFU]->TestSendEFU(0, 1, iELIDStart, iBLIDEnd, 0);
	}
}


void CFFUMonDlg::OnBnClickedButtonGetState()
{
// 	if(m_pDlgComm[SENSOR_MODE_EFU] != NULL)
// 		m_pDlgComm[SENSOR_MODE_EFU]->GetState();

	if(m_pDlgComm[SENSOR_MODE_EFU] != NULL)		//hh.joo 20181212 add
	{
		if (m_bEFU_Type == EFU_TYPE_HKC_H5)
		{
			CString strcboLVID, strcboBLIDStart, strcboBLIDEnd, strcboSV;
			GetDlgItem(IDC_COMBO_BLSTART)->GetWindowText(strcboBLIDStart);
			GetDlgItem(IDC_COMBO_BLEND)->GetWindowText(strcboBLIDEnd);

			int iValue = 0;
			int iELIDStart = 0, iBLIDEnd = 0;
			iELIDStart = _tstoi(strcboBLIDStart);
			iBLIDEnd   = _tstoi(strcboBLIDEnd);


			m_pDlgComm[SENSOR_MODE_EFU]->TestSendEFU(0, 1, iELIDStart, iBLIDEnd, 1);

			m_pDlgComm[SENSOR_MODE_EFU]->TestSendEFU(0, 1, iELIDStart, iBLIDEnd, 2);
		}
		else
		{
			m_pDlgComm[SENSOR_MODE_EFU]->GetEFUStatus();
		}
	}
		

}

void CFFUMonDlg::OnBnClickedChkEfu()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}


void CAboutDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();
}


void CFFUMonDlg::OnBnClickedChk45()
{
	// TODO: Add your control notification handler code here
	UpdateBtn(SENSOR_MODE_IONIZER);
}


void CFFUMonDlg::OnBnClickedChk46()
{
	// TODO: Add your control notification handler code here
	UpdateBtn(SENSOR_MODE_IONIZER);
}
