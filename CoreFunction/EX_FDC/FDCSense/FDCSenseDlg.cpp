
// FDCSenseDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "FDCSense.h"
#include "FDCSenseDlg.h"
#include "afxdialogex.h"
#include "..\Include\API_SocketControl.h"
#include "..\Include\API_SerialCtrl.h"
#include "Registry.h"	// jsyoon 20210129 add
/*#include "FDCDefine.h"*/

#include <winver.h>		

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// add xCrash report.
#include <tlhelp32.h>
#include "..\\Include\\ExceptionHandler.h"

#include <MATH.H>
#pragma comment(lib, "..\\Lib\\dSerialctrl.lib")

#define AUTORUN_VALUENAME	"FDCTOOLS"
#define AUTORUN_ADDARGU		"/User"  
#define RUNKEY				"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"
#define EXE_NAME			"FDCSense.exe"	


#define STR_COMPANY_NAME	"K-MAC"
#define REG_SETTINGS		"Settings"
#define INT_DEF_LAMP_MAX	2
#define REG_LAMP_USED		"LampTime_Used"
#define INT_DEF_LAMP_LIMIT			3000

HANDLE				g_hFDCMapFile;
VTM_FOR_FDC*		g_pFDCMap;
FDC_For_Shared*		g_pFDCforShare; 

#include <direct.h>

#define SAFE_DELETE(p) if(p) { delete p; p = NULL; }

CString gf_AppDir()
{
	CString strAppPath;
	TCHAR szBuff[_MAX_PATH];
	::GetModuleFileName(NULL, szBuff, _MAX_PATH);
	char* pch = ::strrchr(szBuff, '\\');
	strAppPath = CString(szBuff, pch-szBuff+1);

	return strAppPath;
}

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
	sTime.Format("%04d.%02d.%02d-%02d:%02d:%02d:%03d",st.wYear ,st.wMonth ,st.wDay ,st.wHour ,st.wMinute ,st.wSecond, st.wMilliseconds );
	return sTime;
}

CString gf_MakeCurTimeString2()
{
	SYSTEMTIME st;
	::GetLocalTime(&st);
	CString sTime;
	sTime.Format("%02d:%02d:%02d:%03d",st.wHour ,st.wMinute ,st.wSecond, st.wMilliseconds );
	return sTime;
}

CCriticalSection m_CsMaintLog;

// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};


CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

static UINT g_uShellReset	=	0;		//	yj.k 20180201 add explorer shotdown -> restart -> TrayIcon Retry show

// CFDCSenseDlg 대화 상자

CFDCSenseDlg::CFDCSenseDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFDCSenseDlg::IDD, pParent)
	, m_nEQTYPE(0)
	, m_bFilteringUse(FALSE)
	, m_strAutoRunName(_T(""))
	, m_bWriteStart(false)
	, m_bInitUpdate(false)
	, m_bSetUpdatedata(false)
	, m_strAPPName(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_bShowLog = FALSE;
	m_bPLCReadDone = FALSE;
	m_bUpdateLed = FALSE;

	m_hEvtThreadKilled			= CreateEvent(NULL, TRUE,  FALSE, NULL);	   // Manual reset,   initially set
	m_hEvtThreadTerminated		= CreateEvent(NULL, FALSE, FALSE, NULL);	   // auto reset,   initially Reset
	m_hEvtThreadReadKilled		= CreateEvent(NULL, TRUE,  FALSE, NULL);   // Manual reset,   initially set
	m_hEvtThreadReadTerminated	= CreateEvent(NULL, FALSE, FALSE, NULL);   // auto reset,   initially Reset
	m_hEvtThreadKill2			= CreateEvent(NULL, TRUE,  FALSE, NULL);	   // Manual reset,   initially set
	m_hEvtThreadStart			= CreateEvent(NULL, TRUE,  FALSE, NULL);	   // Manual reset,   initially set

	m_pThreadFDC	 = NULL;
	m_pThreadFDCRead = NULL;
	m_pThreadEfu	 = NULL;

	ZeroMemory(&m_ffu, sizeof(m_ffu));

	m_pFDCMap = NULL;

	m_bUseDisplayPPM = FALSE;

	int i=0;
	for (i=0; i<dfMAXFDCITEM; i++)
	{	
		m_spredFDC[i].dFilterMin = NULL;					
		m_spredFDC[i].dFilterMax  = NULL;	
		m_spredFDC[i].dDefaultVal = 0;	
		m_spredFDC[i].bUse = FALSE;	
		m_spredFDC[i].nSoureType = SOURCE_TYPE_DEFAULT;	
		m_spredFDC[i].ShareMem_num = 0;
		m_spredFDC[i].uSVType = 0;		// jsyoon 20181220 add

		m_dBfFDCValue[i] = -9999.;
	}

	for(i = 0 ; i < serial_End; i++)
	{
		m_dSENSOR[i] = 0.0;	
	}

	for (i=0; i<dfMAXMONITER; i++)
	{
		m_dlgMonitoring[i] = NULL;
	}

	ZeroMemory(&m_stAccuraData, sizeof(m_stAccuraData));

	ZeroMemory(&m_pCAPA, sizeof(m_pCAPA));
	ZeroMemory(&m_pLampTime, sizeof(m_pLampTime));

	m_strAppPath.Format("%s", gf_AppDir());
}

void CFDCSenseDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_FPSPREAD_FDCLIST, m_sprdFDCResult);
	DDX_Control(pDX, IDC_FPSPREAD_READ,    m_sprdFDCList);
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_VERSION, m_btnVersion);
}

BEGIN_MESSAGE_MAP(CFDCSenseDlg, CDialogEx)
	ON_MESSAGE(WM_TRAYICON_MSG, TrayIconMSG)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_REGISTERED_MESSAGE(g_uShellReset, OnTrayReset)	//	yj.k 20180201 add explorer shotdown -> restart -> TrayIcon Retry show
	ON_BN_CLICKED(IDC_BTN_SHOW_LOG, &CFDCSenseDlg::OnClickedBtnShowLog)
	ON_BN_CLICKED(IDC_BTN_SHOW_MONITORING, &CFDCSenseDlg::OnBnClickedBtnShowMonitoring)
	ON_BN_CLICKED(IDC_BUTTON_VERSION, &CFDCSenseDlg::OnBnClickedButtonVersion)
	ON_BN_CLICKED(IDC_RELOAD2, &CFDCSenseDlg::OnBnClickedReload2)
	ON_BN_CLICKED(ID_BTN_EXIT, &CFDCSenseDlg::OnBnClickedBtnExit)
	ON_COMMAND(ID_MENUITEM_EXIT, &CFDCSenseDlg::OnMenuitemExit)
	ON_COMMAND(ID_MENUITEM_OPEN, &CFDCSenseDlg::OnMenuitemOpen)
	ON_COMMAND(ID_MENUITEM_WINDOWSTARTAUTORUN,  &CFDCSenseDlg::OnMenuitemWindowstartautorun)
	ON_BN_CLICKED(ID_BTN_MINI, &CFDCSenseDlg::OnBnClickedBtnMini)
END_MESSAGE_MAP()


// CFDCSenseDlg 메시지 처리기

BOOL CFDCSenseDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.
	
	m_nPLCCnt = 0;
	m_nSENSORCnt = 0;
	m_nSHAREDCnt = 0;
	m_nSELFCnt = 0;
	m_nCAPACnt = 0;
	m_nRegCnt = 0;
	m_nSharedMemoryCnt = 0;
	m_nFDCListCnt = 0;

	// sih 20160726 add 
	m_bLedUse = 1;
	m_uiLedPort = 0;
	m_uiLedBaudRate = 0;
	m_uiLedParity = 0;
	m_uiLedDataBits = 0;
	m_uiLedStopBit  = 0;
	m_pGetStateThread = NULL;
	m_dO2ppm = 0.0;
	m_dOldO2ppm = 0.0;
	//

	//jhseo 20170627 add for LED Display 2
	m_bLedUse2 = 2;
	m_uiLedPort2 = 0;
	m_uiLedBaudRate2 = 0;
	m_uiLedParity2 = 0;
	m_uiLedDataBits2 = 0;
	m_uiLedStopBit2  = 0;
	//////////////////////////////////////////////////////////////////////////

	//jhseo 20171107 add for monitoring
	m_bUseMonitoring = FALSE;
	//////////////////////////////////////////////////////////////////////////

	//sypark 20180608 add
	for(int i=0; i<dfMAXMONITER;i++)
	{
		m_nUiMonitorData[i][0] = 0;
		m_nUiMonitorData[i][1] = 0;
		m_nUiMonitorData[i][2] = 0;
	}
	//

	GetConfig();
	CreateMemoryMap();

	// eh.kim 20170419 add.
	m_dO2Min = 20.0;
	m_dO2Max = 23.0;
	//

	CRect rect;
	GetDlgItem(IDC_STC_MAIN_UIPOS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STC_MAIN_UIPOS)->GetWindowRect(&rect);
	ScreenToClient(rect);

	m_dlgFDCView.SetMainPoint(this);
	m_dlgFDCView.SetAppPath(m_strAppPath);
	m_dlgFDCView.Create(IDD_DLG_FDC_MAINUI,this);
	m_dlgFDCView.MoveWindow(rect);
	m_dlgFDCView.ShowWindow(SW_SHOW);

	GetDlgItem(IDC_STC_PLC_POS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STC_PLC_POS)->GetWindowRect(&rect);
	ScreenToClient(rect);

	m_dlgPLC.SetMainPoint(this);
	m_dlgPLC.SetAppPath(m_strAppPath);
	m_dlgPLC.Create(IDD_DIG_PLC,this);
	m_dlgPLC.MoveWindow(rect);
	m_dlgPLC.ShowWindow(SW_SHOW);

	GetDlgItem(IDC_STC_PLCREAD_POS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STC_PLCREAD_POS)->GetWindowRect(&rect);
	ScreenToClient(rect);

	m_dlgPLCRead.SetMainPoint(this);
	m_dlgPLCRead.SetAppPath(m_strAppPath);
	m_dlgPLCRead.Create(IDD_DLG_DREAD_PLC,this);
	m_dlgPLCRead.MoveWindow(rect);
	m_dlgPLCRead.ShowWindow(SW_SHOW);


// 	//jhseo 20171107 add for monitoring
// 	m_dlgMonitoring = new CDlgMonitoring(m_dlgPLC, m_dlgPLCRead);
// 	m_dlgMonitoring->Create(IDD_DLG_MONITORING, this);
// 	//	m_dlgMonitoring->MoveWindow(0, 0, 1920, 1080);		// mn.bong 20180327 remove for temp
// 	m_dlgMonitoring->ShowWindow(m_bUseMonitoring);		//jhseo 20180306 change FALSE -> m_bUseMonitoring
// 	//////////////////////////////////////////////////////////////////////////
	int i=0;
	int nPosX=0, nPosY=0, nWidth=604, nHeight=480;
	//sypark 20180502 change
	for(i = 0; i < dfMAXMONITER; i++)
	{
		if( m_bUseMonitoring == TRUE)
		{
			if(i < m_nUseMonitorNum)
			{
				if( m_dlgMonitoring[i] == NULL)
				{
					m_dlgMonitoring[i] = new CDlgMonitoring(m_dlgPLC, m_dlgPLCRead);
					m_dlgMonitoring[i]->Prepare(i+1);
					m_dlgMonitoring[i]->Create(IDD_DLG_MONITORING, this);
				}

				nPosX = m_dlgMonitoring[i]->m_nMonitorPosX;
				nPosY = m_dlgMonitoring[i]->m_nMonitorPosY;

				nWidth = m_dlgMonitoring[i]-> m_nMonitorWidth;
				nHeight = m_dlgMonitoring[i]-> m_nMonitorHeight;

				m_dlgMonitoring[i]->SetWindowPos(&wndTopMost, nPosX, nPosY, nWidth, nHeight, SWP_NOSIZE);	
				m_dlgMonitoring[i]->ShowWindow(SW_HIDE/*m_bUseMonitoring*/);	
			}
		}		
	}
	//
 	
 	this->GetWindowRect(m_rectMaindlg);
 
 //	SetShowLog(m_bShowLog);
 
 	ZeroMemory(&m_spredFDC, sizeof(m_spredFDC));
 
 	UploadFDCListToExcelFile();

	////////////////////////////////////////////
//	SetShowLog(m_bShowLog);

	if(m_bLedUse)
	{
		OpenPort(m_uiLedPort, m_uiLedBaudRate, m_uiLedDataBits, m_uiLedParity, m_uiLedStopBit); 
		m_nPort = m_uiLedPort;		//jhseo 20170627 add

		Sleep(1000);

		if( api_SerialCom_IsOpened(m_nPort))
		{
			char szMsg[128];
			BYTE byteuf[128];
			byteuf[0] = 0x00;
			szMsg[0] = 0x00;
			int iLength = 0;
			CString strMsg;

			strMsg = _T("[TRESET]");	
			iLength = strMsg.GetLength();
			strcpy_s(szMsg,strMsg);
			memcpy(&byteuf,&szMsg, iLength);
			byteuf[iLength] = 0x00;
			api_SerialCom_WriteByte(m_nPort, byteuf, iLength);
		}	
	}

// 	for (i=0; i< dfMAXFDCITEM; i++)
// 	{
// 		m_spredFDC[i].bUse = m_dlgPLC.m_bFilterUse[i];
// 		m_spredFDC[i].dFilterMin = m_dlgPLC.m_bFilter_Min[i];
// 		m_spredFDC[i].dFilterMax = m_dlgPLC.m_bFilter_Max[i];
// 	}

	for (i=0; i< dfMAXFDCITEM; i++)
	{
		m_dlgPLC.m_bFilterUse[i] = m_spredFDC[i].bUse;
		m_dlgPLC.m_bFilter_Min[i] = m_spredFDC[i].dFilterMin;
		m_dlgPLC.m_bFilter_Max[i] = m_spredFDC[i].dFilterMax;
		m_dlgPLC.m_bFilter_Standard[i] = m_spredFDC[i].dDefaultVal;
	}

	m_dlgPLC.ReadPLC_DData();

	m_bTrayIcon = FALSE; 
	ResTrayIcon();  

	if (m_pThreadFDCRead == NULL) 
	{
		m_pThreadFDCRead = AfxBeginThread(ThreadFDCReadProc,this);
	}

	if (m_pThreadFDC == NULL) 
	{
		m_pThreadFDC = AfxBeginThread(ThreadFDCProc,this);
	}

//	m_btnVersion.SetWindowText("Ver : " + GetVersionInfo());	// jsyoon 20181119 remove

	for(i = 0; i < dfMAXMONITER; i++)
	{
		if(i < m_nUseMonitorNum)
		{
			if( m_dlgMonitoring[i] != NULL)
			{
				m_dlgMonitoring[i]->ShowWindow(m_bUseMonitoring);	
			}
		}		
	}

	GetDiskFree();

	SetTimer(100, 1000, NULL);			// UI UPDATE

	SetTimer(101, 10000, NULL);

	SetTimer(102, 10000, NULL);			// LOG 

//	SetTimer(104, 3000, NULL);	// jsyoon 20180813 add
	SetTimer(104, 3000, NULL);	// jsyoon 20180829 change
	SetTimer(105, 1000, NULL);	// jsyoon 20180829 change

	if (m_pThreadEfu == NULL) 
	{
		m_pThreadEfu = AfxBeginThread(ThreadEFUProc,this);
	}

	UpdateVersionInfo();

	g_uShellReset	=	RegisterWindowMessage(_T("TaskbarCreated"));	//	yj.k 20180201 add explorer shotdown -> restart -> TrayIcon Retry show

	SetShowLog(FALSE);

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

int CFDCSenseDlg::CreateMemoryMap()
{
	if (!g_hFDCMapFile && !g_pFDCMap)
	{	
		g_hFDCMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,
			sizeof( VTM_FOR_FDC ), VTM_SHARE_MEM_STRING_FDC);

		if(!g_hFDCMapFile)
		{
			AfxMessageBox("CreateFileMapping Failed (VTM_SHARE_MEM_STRING_FDC)");
			return FALSE;
		}

// 		if(m_strEQName.Find("ICE")>=0)
// 		{
			g_pFDCMap = (VTM_FOR_FDC*) MapViewOfFile(g_hFDCMapFile, FILE_MAP_ALL_ACCESS, 0, 0,
			sizeof( VTM_FOR_FDC ));

			m_pFDCMap = g_pFDCMap;

			ZeroMemory(&m_pFDCMap->FDCValue, sizeof(m_pFDCMap->FDCValue));	//ksk 20171031 add
//		}
		
		//		long l = GetLastError();
		//		if(!g_pFDCMap&&!g_pFDCMapICE&&!g_pFDCMapICM&&!g_pFDCMap_BP&&!g_pFDCMap_EVEN&&!g_pFDCMap_TFE&&!g_pFDCMapICP&&!m_pFDCMapBP_8K)
		if(!m_pFDCMap)	// sih 20161220 change  // 20161226 kks 
		{
			//	UnmapViewOfFile(g_pSysMapView);
			AfxMessageBox("MapViewOfFile Failed (VTM_SHARE_MEM_STRING_FDC)");
			return FALSE;
		}

		//		m_btnOpenSM.EnableWindow(FALSE);
	}

	return TRUE;
}

int CFDCSenseDlg::gf_CloseMemoryMap()
{
	if (g_pFDCMap)
		UnmapViewOfFile(g_pFDCMap);
	if (g_hFDCMapFile)
		CloseHandle(g_hFDCMapFile);

	return TRUE;
}
void CFDCSenseDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else if(nID == SC_MINIMIZE)		//sypark 20180525 add start
	{
		ShowWindow(SW_HIDE);
	}		
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CFDCSenseDlg::OnPaint()
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
HCURSOR CFDCSenseDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CFDCSenseDlg::OnClickedBtnShowLog()
{
	SetShowLog(m_bShowLog);
}

void CFDCSenseDlg::OnBnClickedBtnShowMonitoring()
{
	//m_dlgMonitoring->ShowWindow(SW_SHOW);
	//sypark 20180502 change
	for(int i = 0; i < dfMAXMONITER; i++)
	{
		if(i < m_nUseMonitorNum)
		{
			if( m_dlgMonitoring[i] == NULL)
			{
				m_dlgMonitoring[i] = new CDlgMonitoring(m_dlgPLC, m_dlgPLCRead);
				m_dlgMonitoring[i]->Prepare(i+1);
				m_dlgMonitoring[i]->Create(IDD_DLG_MONITORING, this);
			}

			int nPosX, nPosY, nWidth, nHeight;
			nPosX = m_dlgMonitoring[i]->m_nMonitorPosX;
			nPosY = m_dlgMonitoring[i]->m_nMonitorPosY;

			nWidth = m_dlgMonitoring[i]-> m_nMonitorWidth;
			nHeight = m_dlgMonitoring[i]-> m_nMonitorHeight;
			
//			m_dlgMonitoring[i]->MoveWindow(nPosX, nPosY, nWidth, nHeight);		
			m_dlgMonitoring[i]->SetWindowPos(&wndTopMost, nPosX, nPosY, nWidth, nHeight, SWP_NOSIZE);	

			if( m_bUseMonitoring )
			{
				if( m_dlgMonitoring[i]->IsWindowVisible() != TRUE)
					m_dlgMonitoring[i]->ShowWindow(SW_SHOW);	

// 				if( m_dlgMonitoring[i]->IsWindowVisible() != TRUE)
// 					m_dlgMonitoring[i]->ShowWindow(SW_SHOW);	
// 				else
// 					m_dlgMonitoring[i]->ShowWindow(SW_HIDE);	
			}
		}
	}
	//
}

void CFDCSenseDlg::OnBnClickedButtonVersion()
{
	HWND handle=NULL;
	CString strPath = "D:\\FDC_History.xlsx";
	strPath.Format("%sFDC_History.txt", m_strAppPath);
	//////////////////////////////////////////////////////////////////////////

	CFileFind file;
	if (!file.FindFile(strPath)) 
	{
		CString strMsg;
		strMsg.Format("File is not exist!\nPlease Create '%s' ", strPath);
		AfxMessageBox(strMsg);
		//////////////////////////////////////////////////////////////////////////

		return;
	}

	ShellExecute(handle, "open", strPath, NULL, NULL, SW_SHOWNORMAL);

	UpdateVersionInfo();
}


void CFDCSenseDlg::SetShowLog(BOOL bShow)
{
	CRect rect, rect2;
	this->GetWindowRect(rect);

	m_sprdFDCResult.GetWindowRect(rect2);

	if (!bShow)
	{
		rect.right = rect.left + rect2.Width()+ 50;
	}
	else
	{
		rect.right = rect.left + m_rectMaindlg.Width();
	}

	this->MoveWindow(rect,TRUE);

	m_bShowLog = !bShow;
}

void CFDCSenseDlg::SetSpredData(int iRow,FDCSPRED SpredData)
{
	if( iRow < dfMAXFDCITEM)
	{
		m_spredFDC[iRow].dVal = SpredData.dVal;
		//m_spredFDC[iRow].sDevice = SpredData.sDevice;		
		//strcpy(m_spredFDC[iRow].cName, SpredData.cName);
		//_tcscpy_s(m_spredFDC[iRow].cPARARMName, _countof(m_spredFDC[iRow].cPARARMName), SpredData.cPARARMName);
	}
}

void CFDCSenseDlg::UpdateSpreadHeader()
{
	CString strSprd;

	for (int iRow=0; iRow<dfMAXFDCITEM; iRow++)
	{
		m_sprdFDCResult.SetCol(1);		
		m_sprdFDCResult.SetRow(iRow+1);
		strSprd.Format("%s",m_spredFDC[iRow].cPARARMName);
		m_sprdFDCResult.SetCellType(5);
		m_sprdFDCResult.SetTypeHAlign(2);
		m_sprdFDCResult.SetTypeVAlign(2);
		m_sprdFDCResult.SetText(strSprd);
//		m_sprdFDCResult.SetEnabled(FALSE);

		m_sprdFDCResult.SetCol(2);		
		m_sprdFDCResult.SetRow(iRow+1);
		strSprd.Format("%.1f",m_spredFDC[iRow].dVal);
		m_sprdFDCResult.SetCellType(5);
		m_sprdFDCResult.SetTypeHAlign(2);
		m_sprdFDCResult.SetTypeVAlign(2);
		m_sprdFDCResult.SetText(strSprd);
//		m_sprdFDCResult.SetEnabled(FALSE);

		m_sprdFDCResult.SetCol(3);		
		m_sprdFDCResult.SetRow(iRow+1);
		strSprd.Format("%s",m_spredFDC[iRow].sDevice);
		m_sprdFDCResult.SetCellType(5);
		m_sprdFDCResult.SetTypeHAlign(2);
		m_sprdFDCResult.SetTypeVAlign(2);
		m_sprdFDCResult.SetText(strSprd);
//		m_sprdFDCResult.SetEnabled(FALSE);

		m_sprdFDCResult.SetCol(4);		
		m_sprdFDCResult.SetRow(iRow+1);
//		m_sprdFDCResult.SetCellType(5);
		m_sprdFDCResult.SetCellType(13);
		m_sprdFDCResult.SetTypeHAlign(2);
		m_sprdFDCResult.SetTypeVAlign(2);
		m_sprdFDCResult.SetTypeNumberDecPlaces(4);
		strSprd.Format("%.3f",m_spredFDC[iRow].dFilterMin);
		m_sprdFDCResult.SetValue(strSprd);
//		m_sprdFDCResult.SetEnabled(FALSE);

		m_sprdFDCResult.SetCol(5);		
		m_sprdFDCResult.SetRow(iRow+1);
//		m_sprdFDCResult.SetCellType(5);
		m_sprdFDCResult.SetCellType(13);
		m_sprdFDCResult.SetTypeHAlign(2);
		m_sprdFDCResult.SetTypeVAlign(2);
		m_sprdFDCResult.SetTypeNumberDecPlaces(4);
		strSprd.Format("%.3f",m_spredFDC[iRow].dFilterMax);
		m_sprdFDCResult.SetValue(strSprd);
//		m_sprdFDCResult.SetEnabled(FALSE);

		m_sprdFDCResult.SetCol(6);		
		m_sprdFDCResult.SetRow(iRow+1);
// 		m_sprdFDCResult.SetCellType(10);
// 		m_sprdFDCResult.SetTypeCheckCenter(TRUE);
// 		m_sprdFDCResult.SetTypeCheckType(0);
		m_sprdFDCResult.SetCellType(13);
		m_sprdFDCResult.SetTypeHAlign(2);
		m_sprdFDCResult.SetTypeVAlign(2);
		m_sprdFDCResult.SetTypeNumberDecPlaces(4);
		strSprd.Format("%.3f",m_spredFDC[iRow].dDefaultVal);
		m_sprdFDCResult.SetValue(strSprd);
//		m_sprdFDCResult.SetEnabled(FALSE);

// 		m_sprdFDCResult.SetCol(6);		
// 		m_sprdFDCResult.SetRow(iRow+1);
// 		m_sprdFDCResult.SetCellType(10);						// Define cell type as check box
// 		m_sprdFDCResult.SetTypeCheckCenter(TRUE);			// Make it a three state check box
// 		m_sprdFDCResult.SetTypeCheckType(0);	
	}
}
void CFDCSenseDlg::UpdateSpreadData()
{
	BOOL bUpdatedata = FALSE;	
	int nCnt = 0;

	m_sprdFDCResult.SetCol(1);
	m_sprdFDCResult.SetMaxRows(dfMAXFDCITEM);

	m_sprdFDCResult.SetRow(1);
	m_sprdFDCResult.SetCol(COL_FILTER_USE);
	m_sprdFDCResult.SetColHidden(TRUE);
//	m_sprdFDCResult.SetLock(TRUE);
//	m_sprdFDCResult.SetEnabled(FALSE);

	double dValue = 0.;
	CString strSprd;
	for (int iRow=0; iRow < dfMAXFDCITEM; iRow++)
	{
		strSprd.Format("%s",m_spredFDC[iRow].cPARARMName);

		m_sprdFDCResult.SetRow(1+iRow);

		if(strSprd.IsEmpty() != TRUE)
		{
			m_sprdFDCResult.SetRowHidden(FALSE);

			m_sprdFDCResult.SetCol(1);			
			m_sprdFDCResult.SetText(strSprd);
			m_sprdFDCResult.SetLock(TRUE);		// jsyoon 20200518 change

			m_sprdFDCResult.SetCol(2);		
			if(m_spredFDC[iRow].dVal < m_spredFDC[iRow].dFilterMax || m_spredFDC[iRow].dVal > m_spredFDC[iRow].dFilterMin )
			{
				strSprd.Format("%.2f",m_spredFDC[iRow].dVal);
			}
			else
			{
				strSprd.Format("%.1f",0);
			}
			m_sprdFDCResult.SetText(strSprd);
			m_sprdFDCResult.SetLock(TRUE);

			m_sprdFDCResult.SetCol(3);		
			strSprd.Format("%s",m_spredFDC[iRow].cDestination);
			m_sprdFDCResult.SetText(strSprd);
			m_sprdFDCResult.SetLock(TRUE);

			m_sprdFDCResult.SetCol(4);		
			m_sprdFDCResult.SetCellType(13);
			dValue = m_spredFDC[iRow].dFilterMin;
			if( dValue >= 10 || dValue <= -10 )
			{
				m_sprdFDCResult.SetTypeNumberDecPlaces(1);
				strSprd.Format("%.1f",dValue);
			}
			else
			{
				m_sprdFDCResult.SetTypeNumberDecPlaces(4);
				strSprd.Format("%.4f",dValue);
			}
			m_sprdFDCResult.SetLock(TRUE);

			m_sprdFDCResult.SetValue(strSprd);

			m_sprdFDCResult.SetCol(5);		
			m_sprdFDCResult.SetCellType(13);
			dValue = m_spredFDC[iRow].dFilterMax;
			if( dValue >= 10 || dValue <= -10 )
			{
				m_sprdFDCResult.SetTypeNumberDecPlaces(1);
				strSprd.Format("%.1f",dValue);
			}
			else
			{
				m_sprdFDCResult.SetTypeNumberDecPlaces(4);
				strSprd.Format("%.4f",dValue);
			}

			m_sprdFDCResult.SetValue(strSprd);

			m_sprdFDCResult.SetLock(TRUE);


			m_sprdFDCResult.SetCol(6);		
			m_sprdFDCResult.SetCellType(13);
			dValue = m_spredFDC[iRow].dDefaultVal;
			if( dValue >= 10 || dValue <= -10 )
			{
				m_sprdFDCResult.SetTypeNumberDecPlaces(1);
				strSprd.Format("%.1f",dValue);
			}
			else
			{
				m_sprdFDCResult.SetTypeNumberDecPlaces(4);
				strSprd.Format("%.4f",dValue);
			}
			
			m_sprdFDCResult.SetValue(strSprd);

			m_sprdFDCResult.SetLock(TRUE);


			//////////////////////////////////////////////////////////////////////////
			// jsyoon 20200511 add
			m_sprdFDCResult.SetCol(7);		
			m_sprdFDCResult.SetCellType(5);
			m_sprdFDCResult.SetTypeHAlign(2);
			m_sprdFDCResult.SetTypeVAlign(2);
			strSprd.Format("%s",m_spredFDC[iRow].cDestination);
			m_sprdFDCResult.SetValue(strSprd);

			m_sprdFDCResult.SetLock(TRUE);

// 			m_sprdFDCResult.SetCol(4);		
// 			strSprd.Format("%.1f",m_spredFDC[iRow].dFilterMin);
// 			m_sprdFDCResult.SetText(strSprd);
// 
// 			m_sprdFDCResult.SetCol(5);		
// 			strSprd.Format("%.1f",m_spredFDC[iRow].dFilterMax);
// 			m_sprdFDCResult.SetText(strSprd);
// 
// 			m_sprdFDCResult.SetCol(6);		
// 			strSprd.Format("%.1f",m_spredFDC[iRow].dDefaultVal);
// 			m_sprdFDCResult.SetText(strSprd);
		}
		else
		{
			m_sprdFDCResult.SetRowHidden(TRUE);
		}

		bUpdatedata = TRUE;		
	}

	if(!bUpdatedata)
	{
		m_sprdFDCResult.SetMaxCols(1);
		m_sprdFDCResult.SetMaxRows(1);
		m_sprdFDCResult.SetColWidth(1, 64);
		m_sprdFDCResult.SetRowHeight(1, 550);

		m_sprdFDCResult.SetCol(1);
		m_sprdFDCResult.SetRow(0);
		m_sprdFDCResult.SetText("Monitoring Mode");

		m_sprdFDCResult.SetRow(1);
		m_sprdFDCResult.SetText("Click \"Show Monitoring UI\" Button");
	}

	int i=0;
	for (i=0; i< dfMAXFDCITEM; i++)
	{
		m_dlgPLC.m_bFilterUse[i]  = m_spredFDC[i].bUse;
		m_dlgPLC.m_bFilter_Min[i] = m_spredFDC[i].dFilterMin;
		m_dlgPLC.m_bFilter_Max[i] = m_spredFDC[i].dFilterMax;
		m_dlgPLC.m_bFilter_Standard[i] = m_spredFDC[i].dDefaultVal;
	}

// 	for (int i=0; i< dfMAXFDCITEM; i++)
// 	{
// 		if( m_dlgPLC.m_strArray.GetSize() > i)
// 		{
// 			//_tcscpy_s(m_spredFDC[i].strPARARMName, _countof(m_spredFDC[i].strPARARMName), m_dlgPLC.m_strArray.GetAt(i));
// 			m_spredFDC[i].bUse = m_dlgPLC.m_bFilterUse[i];
// 			m_spredFDC[i].dFilterMin = m_dlgPLC.m_bFilter_Min[i];
// 			m_spredFDC[i].dFilterMax = m_dlgPLC.m_bFilter_Max[i];
// 		}
// 	}
}

void CFDCSenseDlg::UpdateSpredDate()
{
//	BOOL bUpdatedata = FALSE;

	if( m_bSetUpdatedata != true)
		return;

	CString strSprd;
	for (int iRow=0; iRow<dfMAXFDCITEM; iRow++)
	{
		strSprd.Format("%s",LPCTSTR(m_spredFDC[iRow].cPARARMName));

		if(strSprd.IsEmpty() != TRUE)
		{
			if( m_dBfFDCValue[iRow] !=  m_spredFDC[iRow].dVal )
			{
				m_sprdFDCResult.SetCol(2);		
				m_sprdFDCResult.SetRow(1+iRow);
				m_sprdFDCResult.SetCellType(13);

///				if(m_spredFDC[iRow].dVal < 999999999 || m_spredFDC[iRow].dVal > -1000 )
//				if(m_spredFDC[iRow].dVal < m_spredFDC[iRow].dFilterMax || m_spredFDC[iRow].dVal > m_spredFDC[iRow].dFilterMin )
				if( m_dlgPLC.m_bFilterValue[iRow] == FALSE)
				{
					//if( m_dlgPLC.m_bFilterValue[iRow])
					if(m_spredFDC[iRow].dVal > m_spredFDC[iRow].dFilterMax || m_spredFDC[iRow].dVal < m_spredFDC[iRow].dFilterMin )
					{
					//	m_sprdFDCResult.SetBackColor(dfCOLOR_LIGHTYELLOW);
						m_sprdFDCResult.SetBackColor(dfCOLOR_RED);
//						strSprd.Format("%.1f",0);
					}
					else
					{
						m_sprdFDCResult.SetBackColor(dfCOLOR_WHITE);
					}
				}
				else
				{
					m_sprdFDCResult.SetBackColor(dfCOLOR_LIGHTYELLOW);
// 					m_sprdFDCResult.SetBackColor(dfCOLOR_RED);
// 					strSprd.Format("%.1f",0);
				}

				if( m_spredFDC[iRow].dVal > 1 || m_spredFDC[iRow].dVal < -1 )
				{
					m_sprdFDCResult.SetTypeNumberDecPlaces(2);
					strSprd.Format("%.2f",m_spredFDC[iRow].dVal);
				}
				else
				{
					m_sprdFDCResult.SetTypeNumberDecPlaces(4);
					strSprd.Format("%.4f",m_spredFDC[iRow].dVal);
				}

				m_sprdFDCResult.SetValue(strSprd);

				m_dBfFDCValue[iRow] = m_spredFDC[iRow].dVal;
			}
		}
		else
		{
			;
		}

//		bUpdatedata = TRUE;		//jhseo 20180312 add
	}

// 	//jhseo 20180312 add
// 	if(!bUpdatedata)
// 	{
// 		m_sprdFDCResult.SetMaxCols(1);
// 		m_sprdFDCResult.SetMaxRows(1);
// 		m_sprdFDCResult.SetColWidth(1, 64);
// 		m_sprdFDCResult.SetRowHeight(1, 550);
// 
// 		m_sprdFDCResult.SetCol(1);
// 		m_sprdFDCResult.SetRow(0);
// 		m_sprdFDCResult.SetText("Monitoring Mode");
// 
// 		m_sprdFDCResult.SetRow(1);
// 		m_sprdFDCResult.SetText("Click \"Show Monitoring UI\" Button");
// 	}
// 	//////////////////////////////////////////////////////////////////////////

	CString str;
	DWORD lLiveCheck = 0;
	if( m_pFDCMap !=NULL)
		lLiveCheck = m_pFDCMap->FDCValue.lLiveCheck;

	str.Format("%s, LIVE:%d", m_strEQName, lLiveCheck);
	GetDlgItem(IDC_STATIC_LIVECHECK)->SetWindowText(str);
}

CString CFDCSenseDlg::GetVersionInfo()
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

void CFDCSenseDlg::UpdateVersionInfo()
{
	CString strVer = GetVersionInfo();
	strVer.Replace(",", ".");
	CString ver_str = "Ver: " + strVer;

	m_btnVersion.SetWindowText(ver_str);

	CString strAppName;
	if( m_strEQName.IsEmpty() == TRUE)
		strAppName.Format("KMAC TOOLS FDC");
	else
		strAppName.Format("KMAC TOOLS FDC_%s", m_strEQName);


	CWnd *ptr = AfxGetMainWnd();
	CString str;
	str.Format(_T("%s, %s"), strAppName, ver_str);
	ptr->SetWindowText(str);
}

int CFDCSenseDlg::UploadFDCListToExcelFile(void)
{
	m_nPLCCnt = 0;
	m_nSENSORCnt = 0;
	m_nSHAREDCnt = 0;
	m_nSELFCnt = 0;
	m_nCAPACnt = 0;
	m_nRegCnt = 0;
	m_nSharedMemoryCnt = 0;
	m_nFDCListCnt = 0;

	CString strFDCListPath;
	strFDCListPath.Format("%s%s%s",m_strAppPath, dfDEFAULT_FDCLIST_PATH, dfDEFAULT_FDCLIST_FILE_PATH);
	CFileFind finder;
	if (finder.FindFile(strFDCListPath) == FALSE)
	{
		strFDCListPath.Format("%s%s%s",m_strAppPath, dfDEFAULT_FDCLIST_PATH, "FDC_LIST.xls");		// jsyoon 20201008 add
		if (finder.FindFile(strFDCListPath) == FALSE)
		{
			CString str = "Can not fined 'FDC_List.xlsx' file : ";
		 	str += strFDCListPath;
		 	AfxMessageBox(str);
		 	return 0;
		}
	}

	CString strLog = m_strAppPath + _T("\\ExportRecipe.txt");

	VARIANT varr, varg;
	short	iRes, iCount, iHandle;
	SAFEARRAYBOUND rgsabound[1];
	long	lG		= 0;
	BOOL	bRes	= FALSE;
	VariantClear(&varr);
	VariantInit(&varr);
	VariantClear(&varg);
	VariantInit(&varg);
	varg.vt = VT_BSTR;
	varr.vt = VT_ARRAY | VT_BSTR;
	rgsabound[0].lLbound = 0;
	rgsabound[0].cElements = 4;
	varr.parray = SafeArrayCreate(VT_BSTR, 1, rgsabound);

	m_sprdFDCList.ClearRange(1, 1, -1, -1, FALSE);	//기존의 데이터를 클리어 시켜주고 다시 그려준다
	iRes = m_sprdFDCList.IsExcelFile(strFDCListPath);
	if ( iRes == 1)
	{	
		bRes = m_sprdFDCList.GetExcelSheetList(strFDCListPath, varr, &iCount,strLog, &iHandle, TRUE);
		if ( bRes)
		{
			SafeArrayGetElement(varr.parray, &lG, &varg.bstrVal);
			bRes = m_sprdFDCList.ImportExcelSheet(iHandle, varg);

			if( bRes)
			{
				CreateFDCListFromSheet();
			}
			else		
			{
				AfxMessageBox(_T("Import did not succeed."));
			}
		}
		else
		{
			AfxMessageBox(_T("Cannot return information for Excel file."));
		}
	}
	else
	{
		iRes = m_sprdFDCList.IsExcel2007File(strFDCListPath); 
		if ( iRes == 1)
		{
			bRes = m_sprdFDCList.OpenExcel2007File(strFDCListPath, "", 0, 0, strLog);
			if ( bRes)
			{
				CreateFDCListFromSheet();
			}
			else
			{
				AfxMessageBox(_T("Import did not succeed."));
			}
		}
		else
		{
			AfxMessageBox(_T("File is not an Excel file or is locked and cannot be imported."));
		}
	}

/*
	CFile fileAlarm(strFDCListPath, CFile::shareDenyNone );
	CArchive ar(&fileAlarm, CArchive::load);

	CXLEzAutomation FDCListExcel(FALSE);
	FDCListExcel.OpenExcelFile(strFDCListPath);             

	CString str;
	for (int i = 0; i< dfMAXFDCITEM; i++)
	{
		nIndexNum_Share[i] = -1;

		str = FDCListExcel.GetCellValue(PARARM_NAME_ROW, 2+i);
		strcpy_s(m_spredFDC[i].cPARARMName, str.GetBuffer(0));
		str.ReleaseBuffer();

		if(str != "")
		{
			nFDCListCnt++; 
		}
		else
			continue;

		str = FDCListExcel.GetCellValue(FILTER_MIN_ROW, 2+i);
		m_spredFDC[i].dFilterMin = _ttof(str);

		str = FDCListExcel.GetCellValue(FILTER_MAX_ROW, 2+i);
		m_spredFDC[i].dFilterMax = _ttof(str);

		str = FDCListExcel.GetCellValue(FILTER_DEFAULT_ROW, 2+i);
		m_spredFDC[i].dDefaultVal = _ttof(str);

		str = FDCListExcel.GetCellValue(USE_ROW, 2+i);
		m_spredFDC[i].bUse = _ttoi(str);

		str = FDCListExcel.GetCellValue(SOURCE_TYPE_ROW, 2+i);	

		if(str == "PLC")
		{
			m_spredFDC[i].nSoureType = SOURCE_TYPE_PLC;
			nPLCCnt++;
		}
		else if(str =="SENSOR")
		{
			m_spredFDC[i].nSoureType = SOURCE_TYPE_SENSOR;
			nSENSORCnt++;
		}
		else if(str =="SHARE")
		{
			m_spredFDC[i].nSoureType = SOURCE_TYPE_SHAHREMEMORY;
			nSHAREDCnt++;
		}
		else if(str =="CAPA")
		{
			m_spredFDC[i].nSoureType = SOURCE_TYPE_CAPA;
			nIndexNum_CAPA[nCAPACnt] = i;
			nCAPACnt++;
		}
		else if(str =="SELF")
		{
			m_spredFDC[i].nSoureType = SOURCE_TYPE_SELF;
			nSELFCnt++;
		}
		else 
			m_spredFDC[i].nSoureType = SOURCE_TYPE_DEFAULT;
		
		str = FDCListExcel.GetCellValue(SOURCE_ADDRESS_ROW, 2+i);
		int nlength = str.GetLength() + 1;			//add for test
		//strcpy(m_spredFDC[i].cSourceAddress,  str.GetBuffer(0));		//sypark 20180514 change
		strcpy_s(m_spredFDC[i].cSourceAddress, nlength, str.GetBuffer(0));  
		str.ReleaseBuffer();

		str = FDCListExcel.GetCellValue(DESTINATIO_ROW, 2+i);
		nlength = str.GetLength()+1;	 
		//strcpy(m_spredFDC[i].cDestination , str.GetBuffer(0));			//sypark 20180514 change
		strcpy_s(m_spredFDC[i].cDestination ,nlength, str.GetBuffer(0));
		str.ReleaseBuffer();

		str = FDCListExcel.GetCellValue(SHAREMEM_NUM, 2+i);
		if(str !="")
		{
			m_spredFDC[i].ShareMem_num = _ttoi(str);			//Sharememory FDC_WDATA number	
			nIndexNum_Share[i] = m_spredFDC[i].ShareMem_num;			//index number
			nSharedMemoryCnt++;
		}
		else
		{
			m_spredFDC[i].ShareMem_num = -1;
			nIndexNum_Share[i] = -1;
		}

		//sypark 20180529 add
		str = FDCListExcel.GetCellValue(VALUE_MAGNIFICATION_ROW, 2+i);
		m_spredFDC[i].uiMagnificaion = _ttoi(str);	
		//

		// jsyoon 20181220 add
		str = FDCListExcel.GetCellValue(SV_TYPE_ROW, 2+i);
		m_spredFDC[i].uSVType = _ttoi(str);	
	}

	FDCListExcel.ReleaseExcel();
	//usrTerminateProcess("EXCEL.EXE");		//sypark 20180514 remove
	
*/
	return TRUE;
}

int CFDCSenseDlg::SetMStateCodeToSpread()
{
	return 0;
}


void CFDCSenseDlg::OnBnClickedReload2()
{
//	SetEvent(m_hEvtThreadStart);

	UploadFDCListToExcelFile();
	return;

// 	int i=0;
// 	for(i = 0; i < dfMAXMONITER; i++)
// 	{
// 		if(i < m_nUseMonitorNum)
// 		{
// 			if( m_dlgMonitoring[i] == NULL)
// 			{
// 				m_dlgMonitoring[i] = new CDlgMonitoring(m_dlgPLC, m_dlgPLCRead);
// 				m_dlgMonitoring[i]->Prepare(i+1);
// 				m_dlgMonitoring[i]->Create(IDD_DLG_MONITORING, this);
// 			}
// 
// 			m_dlgMonitoring[i]->GetConfig();
// 
// 			int nPosX, nPosY, nWidth, nHeight;
// 			nPosX = m_dlgMonitoring[i]->m_nMonitorPosX;
// 			nPosY = m_dlgMonitoring[i]->m_nMonitorPosY;
// 
// 			nWidth = m_dlgMonitoring[i]-> m_nMonitorWidth;
// 			nHeight = m_dlgMonitoring[i]-> m_nMonitorHeight;
// 
// 			m_dlgMonitoring[i]->SetWindowPos(&wndTopMost, nPosX, nPosY, nWidth, nHeight, SWP_NOSIZE);	
// 			m_dlgMonitoring[i]->ShowWindow(SW_HIDE/*m_bUseMonitoring*/);			
// 		}		
// 	}
// 	for ()
// 	{
// 		if( m_dlgMonitoring[0] != NULL)
// 			m_dlgMonitoring[0]->GetConfig();
// 	}
	

}

void CFDCSenseDlg::ResTrayIcon()
{
	NOTIFYICONDATA nid;
	nid.cbSize = sizeof(nid);
	nid.hWnd = m_hWnd;
	nid.uID = IDI_TRAYICON;
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	nid.uCallbackMessage = WM_TRAYICON_MSG;
	nid.hIcon = AfxGetApp()->LoadIcon(IDI_TRAYICON); 

	CString strAppName;
	strAppName.Format("KMAC FDC TOOLS_%s", m_strEQName);
	m_strAutoRunName.Format("%s", strAppName);

	lstrcpy(nid.szTip,strAppName);
	Shell_NotifyIcon(NIM_ADD, &nid);

	SendMessage(WM_SETICON, (WPARAM)TRUE, (LPARAM)nid.hIcon);
	m_bTrayIcon = TRUE;
}


void CFDCSenseDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
	gf_CloseMemoryMap();

	for(int i = 0; i <m_nUseMonitorNum; i ++)
	{
		if(m_dlgMonitoring[i] != NULL)		//jhseo 20171107 add for monitoring
		{
			delete m_dlgMonitoring[i];

		}
	}

	//CloseTerminal();


	//sypark 20180504 remove, 프로그램 죽을때 Assert error 발생해서 종료버튼으로 변경함. 
// 	for(int i=0;i<3;i++)
// 	{
// 		if (WaitForSingleObject(m_hEvtThreadTerminated, 0)
// 			== WAIT_OBJECT_0)
// 		{
// 			break;
// 		}
// 
// 		Sleep(1000);
// 	}

//	KillTimer(100);	
//	KillTimer(101);	

	if (m_hEvtThreadKilled)
		CloseHandle(m_hEvtThreadKilled);

	if (m_hEvtThreadTerminated)
		CloseHandle(m_hEvtThreadTerminated);

	if (m_hEvtThreadReadTerminated)
		CloseHandle(m_hEvtThreadReadTerminated);

	if (m_hEvtThreadReadKilled)
		CloseHandle(m_hEvtThreadReadKilled);

	if (m_hEvtThreadKill2)
		CloseHandle(m_hEvtThreadKill2);

	if( m_hEvtThreadStart)
		CloseHandle(m_hEvtThreadStart);

}

UINT CFDCSenseDlg::ThreadFDCProc(LPVOID pParam) 
{
	CFDCSenseDlg* pProc = (CFDCSenseDlg*) pParam;

	while (TRUE)
	{	
		if (WaitForSingleObject(pProc->m_hEvtThreadKilled, 0)
			== WAIT_OBJECT_0)
		{
			break;
		} 
	
		if( pProc->m_bWriteStart == true)
		{
			pProc->m_dlgPLC.WriteFDC_Filtering(pProc->m_bFilteringUse);

			pProc->m_dlgPLC.WriteFDC_OffsetData();

			// INT
			pProc->m_dlgPLC.WriteFDC_Data();

			// UINT
			pProc->m_dlgPLC.WriteFDC_Data(1);

			// FLOAT
			pProc->m_dlgPLC.WriteFDC_Data(2);

			// ASCII
			pProc->m_dlgPLC.WriteFDC_Data(3);		// jsyoon 20200522 add
			
		}
		
		Sleep(500);
	}

	SetEvent(pProc->m_hEvtThreadTerminated);
	TRACE("ThreadFDCProc OUT\n");

	return 0;
}
UINT CFDCSenseDlg::ThreadFDCReadProc(LPVOID pParam) 
{
	CFDCSenseDlg* pProc = (CFDCSenseDlg*) pParam;

	BOOL bRet = FALSE;
	while (TRUE)
	{	
		if (WaitForSingleObject(pProc->m_hEvtThreadReadKilled, 0)
			== WAIT_OBJECT_0)
		{
			break;
		}

		bRet = pProc->m_dlgPLCRead.DoWorkStartGetPLCMemoryBitAll2();			// D Word 읽기

		pProc->m_bPLCReadDone = bRet;

		if( bRet == TRUE )
		{
			Sleep(500);
		}
		else
		{
			Sleep(200);
		}
	}

	pProc->m_bPLCReadDone = FALSE;

	SetEvent(pProc->m_hEvtThreadReadTerminated);
	TRACE("ThreadFDCReadProc OUT\n");

	return 0;
}

UINT CFDCSenseDlg::ThreadEFUProc(LPVOID pParam) 
{
	CFDCSenseDlg* pProc = (CFDCSenseDlg*) pParam;

//	Sleep(5000);		// jsyoon 20180827 remove

	while (TRUE)
	{	
		WaitForSingleObject(pProc->m_hEvtThreadStart, INFINITE);

		if (WaitForSingleObject(pProc->m_hEvtThreadKill2, 0)
			== WAIT_OBJECT_0)
		{
			break;
		}

		pProc->m_dlgFDCView.GetReceiveData();

		Sleep(1);		// jsyoon 20180829 change 100 to 1
	}

	TRACE("ThreadEFUProc OUT\n");
	return 0;
}

LRESULT CFDCSenseDlg::TrayIconMSG(WPARAM wParam, LPARAM lParam)
{
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

			if(SHRegReadString(HKEY_CURRENT_USER, RUNKEY, m_strAutoRunName/*AUTORUN_VALUENAME*/, "", szStr, MAX_PATH*2))
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

LRESULT	CFDCSenseDlg::OnTrayReset(WPARAM wParam, LPARAM lParam)	//	yj.k 20180201 add explorer shotdown -> restart -> TrayIcon Retry show
{
	ResTrayIcon();
	return	0;
}

BOOL CFDCSenseDlg::SHRegReadString(HKEY hKey, LPCTSTR lpKey, LPCTSTR lpValue, LPCTSTR lpDefault, LPTSTR lpRet, DWORD nSize)
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

BOOL CFDCSenseDlg::SHRegWriteString(HKEY hKey, LPCTSTR lpKey, LPCTSTR lpValue, LPCTSTR lpData)
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

BOOL CFDCSenseDlg::SHRegDeleteValue(HKEY hKey, LPCTSTR lpKey, LPCTSTR lpValue)
{
	HKEY  key;

	if(RegOpenKeyEx(hKey, lpKey, 0, KEY_ALL_ACCESS, &key)!=ERROR_SUCCESS) 
		return FALSE;

	if(RegDeleteValue(key, lpValue)!=ERROR_SUCCESS) 
		return FALSE;

	RegCloseKey(key);

	return TRUE;
}

void CFDCSenseDlg::GetDiskFree()
{
	DWORD		dwFreeGegaBytes; 
	DWORD		dwTotalGegaBytes; 

	ULARGE_INTEGER FreeBytesAvailable;		// bytes available to caller
	ULARGE_INTEGER TotalNumberOfBytes;		// bytes on disk
	ULARGE_INTEGER TotalNumberOfFreeBytes;	// free bytes on disk
	ULARGE_INTEGER GegaByte;				

	GegaByte.QuadPart = 1024*1024*1024;
	
	for(int i = 0; i <m_nCAPACnt; i++)
	{
		BOOL bRet = FALSE;
		LPCSTR lpDriveName = LPCTSTR(m_spredFDC[nIndexNum_CAPA[i]].cSourceAddress);

		bRet = GetDiskFreeSpaceEx(lpDriveName,&FreeBytesAvailable,&TotalNumberOfBytes,&TotalNumberOfFreeBytes);
		if(bRet)
		{
			dwTotalGegaBytes = (DWORD)(TotalNumberOfBytes.QuadPart/GegaByte.QuadPart);
			dwFreeGegaBytes = (DWORD)(FreeBytesAvailable.QuadPart/GegaByte.QuadPart);
		}
		else	
		{
			dwTotalGegaBytes = 0;
			dwFreeGegaBytes = 0;
		}

		m_pCAPA[i] = (int)((float)dwFreeGegaBytes/(float)dwTotalGegaBytes * 100);
	}
}

void CFDCSenseDlg::OpenPort(UINT nPort, DWORD dwRate,BYTE bDataBits, BYTE bParity, BYTE bStopBits)
{
	m_nPort = nPort;

	if(api_SerialCom_Open(nPort, dwRate, bDataBits, bParity, bStopBits))
	{
		if(api_SerialCom_IsOpened(nPort))
			StartThreadGetState();	
	}
}

void CFDCSenseDlg::StartThreadGetState()
{
	if(!m_pGetStateThread)
	{
		m_hEvtGetStateStop = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_pGetStateThread = AfxBeginThread(WorkThreadProcGetState, this);
		m_pGetStateThread->m_bAutoDelete = FALSE;
	}
}

UINT CFDCSenseDlg::WorkThreadProcGetState(LPVOID pParam)
{
	CFDCSenseDlg *pProc = (CFDCSenseDlg*) pParam;
	volatile BOOL bLoop  = TRUE;

	Sleep(5000);

	DWORD dTime = 5000;

	if( pProc->m_bUseDisplayPPM == TRUE)
		dTime = 10000;

	while(bLoop)
	{
		DWORD res = WaitForSingleObject(pProc->m_hEvtGetStateStop, dTime);		// jsyoon 20170103 change

		switch(res)
		{
		case WAIT_OBJECT_0:
			bLoop = FALSE;
			break;
		case WAIT_TIMEOUT:
			pProc->GetState();
		}
	}

	pProc->m_pGetStateThread = NULL;
	return TRUE;
}

void CFDCSenseDlg::GetState()
{
// 	if(m_strEQName.Find("ICE")>=0)
// 		m_dO2ppm = m_dlgPLCRead.m_dPLCReadData[PLC_ICE_ICM_CHAMBER_O2_PPM_1];	
// 	if(m_strEQName.Find("ICM")>=0)
// 		m_dO2ppm = m_dlgPLCRead.m_dPLCReadData[PLC_ICE_ICM_CHAMBER_O2_PPM_1];	
// 	if(m_strEQName.Find("ICP")>=0)
// 		m_dO2ppm = m_dlgPLCRead.m_dPLCReadData[PLC_ICP_CHAMBER_O2_PPM];		// m_pFDCMapICP->FDCValue.dPuriO2ppm;
// 	if(m_strEQName.Find("TM")>=0)
// 		m_dO2ppm = m_dlgPLCRead.m_dPLCReadData[INS_CHAMBER_O2_PPM_DATA_1];  // 20161226 kks

	//	if(fabs(m_dO2ppm - m_dOldO2ppm) > 0.0001)
	
	if(m_bPLCReadDone)
	{
		if(m_dO2ppm > 10000)
			SendToLed(m_nPort, 1, m_dO2ppm);
		else
			SendToLed(m_nPort, 0, m_dO2ppm);

		if(m_dO2ppm > 10000)
			SendToLed(m_nPort, 1, m_dO2ppm2);
		else
			SendToLed(m_nPort2, 0, m_dO2ppm2);		//jhseo 20170627 add for LED Display 2

		m_dOldO2ppm = m_dO2ppm;
	}
}

void CFDCSenseDlg::GetConfig() 
{
	CString g_sAppPath;
	TCHAR szBuff[_MAX_PATH];
	::GetModuleFileName(NULL, szBuff, _MAX_PATH);
	char* pch = ::strrchr(szBuff, '\\');
	g_sAppPath = CString(szBuff, pch-szBuff+1);

	char szValue[MAX_PATH];
	ZeroMemory(szValue,MAX_PATH);

	CString sPath;
	sPath = g_sAppPath + dfDEFAULT_CONFIG_FILE_PATH;

	char szConfigFile[MAX_PATH];
	strcpy_s(szConfigFile,sPath);

	GetPrivateProfileString ("DIR","ROOT","C:\\K-MAC\\FDC\\",szValue,MAX_PATH,szConfigFile);
	m_strRootPath.Format("%s",szValue); 
	gf_MakeDir(m_strRootPath);

	SetExceptionSaveDir(m_strRootPath); 

	GetPrivateProfileString ("EQNAME","NAME","ICE",szValue,MAX_PATH,szConfigFile);
	m_strEQName.Format("%s",szValue); 

	GetPrivateProfileString ("EQNAME","FILTER","0",szValue,MAX_PATH,szConfigFile);
	m_bFilteringUse = _ttoi(szValue);

	GetPrivateProfileString ("LED DISPLAY","Use","1",szValue,MAX_PATH,szConfigFile);
	m_bLedUse = _ttoi(szValue);

	GetPrivateProfileString ("LED DISPLAY","Port","2",szValue,MAX_PATH,szConfigFile);
	m_uiLedPort = _ttoi(szValue);

	GetPrivateProfileString ("LED DISPLAY","BAUDRATE","9600",szValue,MAX_PATH,szConfigFile);
	m_uiLedBaudRate = _ttoi(szValue);

	GetPrivateProfileString ("LED DISPLAY","PARITY","0",szValue,MAX_PATH,szConfigFile);
	m_uiLedParity = _ttoi(szValue);

	GetPrivateProfileString ("LED DISPLAY","DATABIT","8",szValue,MAX_PATH,szConfigFile);
	m_uiLedDataBits = _ttoi(szValue);

	GetPrivateProfileString ("LED DISPLAY","STOPBIT","0",szValue,MAX_PATH,szConfigFile);
	m_uiLedStopBit = _ttoi(szValue);

	// jsyoon 20170103 add 
	GetPrivateProfileString ("LED DISPLAY","PPM","0",szValue,MAX_PATH,szConfigFile);
	m_bUseDisplayPPM = _ttoi(szValue);
	//////////////////////////////////////////////////////////////////////////

	//jhseo 20170627 add for LED Display 2
	GetPrivateProfileString ("LED DISPLAY2","Use","1",szValue,MAX_PATH,szConfigFile);
	m_bLedUse2 = _ttoi(szValue);

	GetPrivateProfileString ("LED DISPLAY2","Port","2",szValue,MAX_PATH,szConfigFile);
	m_uiLedPort2 = _ttoi(szValue);

	GetPrivateProfileString ("LED DISPLAY2","BAUDRATE","9600",szValue,MAX_PATH,szConfigFile);
	m_uiLedBaudRate2 = _ttoi(szValue);

	GetPrivateProfileString ("LED DISPLAY2","PARITY","0",szValue,MAX_PATH,szConfigFile);
	m_uiLedParity2 = _ttoi(szValue);

	GetPrivateProfileString ("LED DISPLAY2","DATABIT","8",szValue,MAX_PATH,szConfigFile);
	m_uiLedDataBits2 = _ttoi(szValue);

	GetPrivateProfileString ("LED DISPLAY2","STOPBIT","0",szValue,MAX_PATH,szConfigFile);
	m_uiLedStopBit2 = _ttoi(szValue);
	//////////////////////////////////////////////////////////////////////////

	//jhseo 20171107 add for monitoring
	GetPrivateProfileString ("UI DISPLAY","Use","0",szValue,MAX_PATH,szConfigFile);
	m_bUseMonitoring = _ttoi(szValue);
	GetDlgItem(IDC_BTN_SHOW_MONITORING)->ShowWindow(_ttoi(szValue));


// 	for(int i = 0; i < 3; i++)
// 	{
// 		CString strData;
// 
// 		strData.Format("DATA%d", i+1);
// 
// 		GetPrivateProfileString ("UI DISPLAY",strData,"0",szValue,MAX_PATH,szConfigFile);
// 
// 		int nValue = _ttoi(szValue);
//  
// // 		if( nValue < 11500 || nValue >= 12523)
// // 		{
// // 			nValue = 11500;
// // 		}
// 		if( nValue < 0 || nValue >= 300)
// 		{
// 			nValue = 1;
// 		}
// 
// 		//m_nUiData[i] = nValue;
// 		m_nUiMonitorData[0][i] = nValue -1;
// 	}
	//////////////////////////////////////////////////////////////////////////

	//sypark 20180502 add
	GetPrivateProfileString ("UI DISPLAY","UseMonitorNum","0",szValue,MAX_PATH,szConfigFile);
	m_nUseMonitorNum = _ttoi(szValue);
	//

	// jsyoon 20210129 add
	GetPrivateProfileString ("REGISTRY","NAME","MetroSys",szValue,MAX_PATH,szConfigFile);
	m_strAPPName.Format("%s",szValue); 

	CString str;
	int i = 0;
	for(i = 0; i < dfMAXNUM_PCTEMP; i++)
	{
		str.Format("USE_PCTEMP%d", i+1);
		GetPrivateProfileString ("PCTEMP",str,"0",szValue,MAX_PATH,szConfigFile);
		m_bUseTemp[i] = _ttoi(szValue);
	}

	//sypark 20180608 add
	for(i = 0; i < dfMAXMONITER; i++)
	{
		str.Format("MONITORING_%d", i+1);

		for(int j = 0; j < 3; j++)
		{
			CString strData;

			strData.Format("DATA%d", j+1);

			GetPrivateProfileString (str,strData,"11500",szValue,MAX_PATH,szConfigFile);
			
			int nValue = _ttoi(szValue);

// 			if( nValue < 11500 || nValue >= 12523)
// 			{
// 				nValue = 11500;
// 			}
			if( nValue < 0 || nValue >= df_MAX_ITEM)
			{
				nValue = 1;
			}

			m_nUiMonitorData[i][j] = nValue-1;
		}	
	}
}

void CFDCSenseDlg::SendToLed(int iPort, int i, double dO2)
{
	if(api_SerialCom_IsOpened(iPort))
	{
		char szMsg[128];
		BYTE byteuf[128];
		byteuf[0] = 0x00;
		szMsg[0] = 0x00;
		int iLength = 0;
		CString strMsg;

		strMsg = _T("[TMODE]\r");	
		iLength = strMsg.GetLength();
		//		strcpy(szMsg,strMsg);
		_tcscpy_s(szMsg, _countof(szMsg), strMsg);
		memcpy(&byteuf,&szMsg, iLength);
		byteuf[iLength] = 0x00;
		api_SerialCom_WriteByte(m_nPort, byteuf, iLength);

		strMsg = _T("[COMON]");	
		iLength = strMsg.GetLength();
		//		strcpy(szMsg,strMsg);
		_tcscpy_s(szMsg, _countof(szMsg), strMsg);
		memcpy(&byteuf,&szMsg, iLength);
		byteuf[iLength] = 0x00;
		api_SerialCom_WriteByte(m_nPort, byteuf, iLength);

		strMsg.Format(_T("[PX0 PY0 CR%d]*"), ( m_bUpdateLed )? 3:2);		
		iLength = strMsg.GetLength();
		//		strcpy(szMsg,strMsg);
		_tcscpy_s(szMsg, _countof(szMsg), strMsg);
		memcpy(&byteuf,&szMsg, iLength);
		byteuf[iLength] = 0x00;
		api_SerialCom_WriteByte(m_nPort, byteuf, iLength);

		if(m_bUpdateLed)
			m_bUpdateLed = FALSE;
		else
			m_bUpdateLed = TRUE;

		//		strMsg.Format(_T("[CR%d]산소농도 "), ( dO2/10000 > 20.0 && dO2/10000 < 23.0 )? 2:1);		// eh.kim 20170419 change.
		strMsg.Format(_T("[CR%d]산소농도 "), ( dO2/10000 > m_dO2Min && dO2/10000 < m_dO2Max )? 2:1);
		iLength = strMsg.GetLength();
		//		strcpy(szMsg,strMsg);
		_tcscpy_s(szMsg, _countof(szMsg), strMsg);
		memcpy(&byteuf,&szMsg, iLength);
		byteuf[iLength] = 0x00;
		api_SerialCom_WriteByte(m_nPort, byteuf, iLength);

		// jsyoon 20170103 change
		double dO2PERC = 0.0;
		dO2PERC = dO2/10000;
		if( dO2PERC < 0.1)
			dO2PERC = 0.1;

		if( m_bUseDisplayPPM == FALSE)
		{
			//			strMsg.Format("[CR%d]%.1f%%[ACT0]",	( dO2PERC > 20.0 && dO2PERC < 23.0)? 2:1,  dO2PERC);	 			// eh.kim 20170419 change.
			strMsg.Format("[CR%d]%.1f%%[ACT0]",	( dO2PERC > m_dO2Min && dO2PERC < m_dO2Max)? 2:1,  dO2PERC);
		}
		else
		{
			if(i == 1)	
			{
				//				strMsg.Format("[CR%d]%.1f%%[ACT0]",	( dO2PERC > 20.0 && dO2PERC < 23.0)? 2:1,  dO2PERC);	 		// eh.kim 20170419 change.
				strMsg.Format("[CR%d]%.1f%%[ACT0]",	( dO2PERC > m_dO2Min && dO2PERC < m_dO2Max)? 2:1,  dO2PERC);
			}
			else
			{		
				if(dO2 < 0.1)
					dO2 = 0.1;

				strMsg.Format("[CR1]%.1f%%(%.1fppm)[ACT1]", dO2PERC, dO2);
			}
		}


		// 		if(i == 1)	
		// 			strMsg.Format("[CR%d]%.1f%%[ACT0]",	( dO2/10000 > 20.0 && dO2/10000 < 23.0)? 2:1,  dO2/10000);	 	
		// 		else
		// 		{
		// 			if(dO2 < 0.11)
		// 				dO2 = 0.1;
		// 			
		// 			strMsg.Format("[CR1]%.5f%%(%.1fppm)[ACT1]", dO2/10000, dO2);
		// 		}

		iLength = strMsg.GetLength();
		//		strcpy(szMsg,strMsg);
		_tcscpy_s(szMsg, _countof(szMsg), strMsg);
		memcpy(&byteuf,&szMsg, iLength);
		byteuf[iLength] = 0x00;
		api_SerialCom_WriteByte(m_nPort, byteuf, iLength);
	}
}


void CFDCSenseDlg::OnTimer(UINT_PTR nIDEvent)
{
	if( nIDEvent == 100 )
	{
		UpdateSpredDate();
	}
	else if( nIDEvent == 101 )
	{
		GetDiskFree();

		GetReadRegistry();	// jsyoon 20210129 add

		if( m_bInitUpdate == true)
		{
			int i=0;
			for (i=0; i<dfMAXMONITER; i++)
			{
				if( m_dlgMonitoring[i] != NULL)
					m_dlgMonitoring[i]->StartUIMonitor();
			}

			m_bInitUpdate = false;
		}
	} 
	else if (nIDEvent == 102)	// jsyoon 20150129 add
	{
		CString str, strSprd;
		CString strResult;
		strResult.Empty();
		for(int j = 0; j < dfMAXFDCITEM; j++)
		{
			if(m_dlgPLC.m_bShowFLog[j])
			{
				strSprd.Format("%s",LPCTSTR(m_spredFDC[j].cPARARMName));
				if( strSprd.IsEmpty() == TRUE)
					continue;

				str.Format("%s:%.3f, ", strSprd, m_dlgPLC.m_valArray[j]);
				strResult += str;
			}
		}

// 		for(int j = 0; j < m_dlgPLC.m_strArray.GetSize(); j++)
// 		{
// 			if(m_dlgPLC.m_bShowFLog[j])
// 			{
// 				str.Format("%s:%.3f, ",m_dlgPLC.m_strArray.GetAt(j), m_dlgPLC.m_valArray[j]);
// 				strResult += str;
// 			}
// 		}

		if(strResult.IsEmpty() != TRUE )
			WriteInfoLog(strResult);
	}

	else if( nIDEvent == 103 )
	{
		KillTimer(103);

		m_dlgPLC.CloseSocket();
		m_dlgPLCRead.CloseSocket();

		CloseDialog();
	}
	else if( nIDEvent == 104 )
	{
		KillTimer(104);

		m_dlgFDCView.ConnectSock();

		m_bWriteStart = true;

		m_bInitUpdate = true;
	}
	else if( nIDEvent == 105 )
	{
		// Monitoring
		double dData[3] = {0., 0., 0.};
		BOOL bLive = FALSE;
		for(int i = 0; i < dfMAXMONITER; i++)
		{
			if(i < m_nUseMonitorNum)
			{
				if( m_dlgMonitoring[i] != NULL)
				{
					dData[0] = m_dlgPLC.m_valArray[m_nUiMonitorData[i][0]];
					dData[1] = m_dlgPLC.m_valArray[m_nUiMonitorData[i][1]];
					dData[2] = m_dlgPLC.m_valArray[m_nUiMonitorData[i][2]];
					bLive = m_dlgPLC.m_bSendOK;

					m_dlgMonitoring[i]->SetUIUpdateData(dData[0], dData[1], dData[2], bLive);
				}
			}
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}

void CFDCSenseDlg::WriteInfoLog(CString strMsg)
{
	SYSTEMTIME st;
	::GetLocalTime(&st);
	CTime ct(st);

	CFile	fileLog;
	char szLog[6400];
	char szLogPath[6400];
	ZeroMemory(szLog,6400);	
	ZeroMemory(szLogPath,6400);

//	wsprintf(szLogPath,"%s%s\\InfoLog-%02d%02d%02d.log",m_strRootPath,dfDEFAULT_ILOG_PATH, st.wMonth, st.wDay,st.wHour);
	wsprintf(szLogPath,"%s%s%02d\\InfoLog-%02d%02d%02d.log",m_strRootPath,dfDEFAULT_ILOG_PATH, st.wMonth, st.wMonth, st.wDay,st.wHour);
	wsprintf(szLog,"%s, %s",gf_MakeCurTimeString2(), strMsg);
	WriteFile(&fileLog,(CString)szLogPath,szLog);
}

void CFDCSenseDlg::WriteELog(CString strMsg, int iType/* = 0*/)
{
	CSingleLock sLock(&m_CsMaintLog);				// jsyoon 20131119 add must be locked to access plc resource owned by only one thread at the same time
	sLock.Lock();
	if( sLock.IsLocked())
	{
		SYSTEMTIME st;
		::GetLocalTime(&st);
		CTime ct(st);

		CFile	fileLog;
		char szLog[8000];
		char szLogPath[6400];
		ZeroMemory(szLog,8000);	
		ZeroMemory(szLogPath,6400);

		switch (iType)
		{
		case 0:
			wsprintf(szLogPath,"%s%s%02d\\TCP_RECEIVE.log",m_strRootPath,dfDEFAULT_CLOG_PATH, st.wDay);
			break;
		case 1:
			wsprintf(szLogPath,"%s%s%02d\\PLC_WRITE.log",  m_strRootPath,dfDEFAULT_CLOG_PATH, st.wDay);
			break;
		case 2:
			wsprintf(szLogPath,"%s%s%02d\\PLC_READ.log",   m_strRootPath,dfDEFAULT_CLOG_PATH, st.wDay);
			break;
		default:
			break;
		}

		wsprintf(szLog,"%s", strMsg);
		WriteFile2((CString)szLogPath,szLog);

		sLock.Unlock();
	}
}

int CFDCSenseDlg::WriteFile(CFile *pFile, CString &sPath, CString sLog)
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

int CFDCSenseDlg::WriteFile2(CString sPath, CString sLog)
{
	int nRet = 1;

	{
		int iLen = 0;
		BOOL bNew = FALSE;
		CFileStatus fs;
		CFile cfile;

		TCHAR szPath[1024];
		TCHAR szLog[9000];
		TCHAR szTmp[9000];

		szTmp[0] = 0x00;
		szPath[0] = 0x00;
		szLog[0] = 0x00;

		_stprintf_s(szTmp,  _countof(szTmp), "%s", sPath);
		_tcscat_s(szPath,_countof(szPath), szTmp);

		iLen = sLog.GetLength();
		if( iLen > 8000)
			_stprintf_s(szTmp,  _countof(szTmp), "%s", sLog.Left(8000));
		else
			_stprintf_s(szTmp,  _countof(szTmp), "%s", sLog);
		_tcscat_s(szLog,_countof(szLog), szTmp);	
		_tcscat_s(szLog, _countof(szLog), _T("\r\n"));

		iLen = strlen(szLog);
		szLog[iLen] = 0;

		if (!CFile::GetStatus(szPath, fs))  //if not exist yet
		{
			gf_MakeDir(szPath);// _mkdir;
			bNew = TRUE;
		}
		else if (cfile.m_hFile != CFile::hFileNull)
		{
			CString s1 = fs.m_szFullName ;
			if ((cfile.GetFilePath() != s1))
			{
				bNew = TRUE;
			}
		}
		else
		{
			bNew = FALSE;
		}

		if (cfile.m_hFile != CFile::hFileNull && bNew)
		{
			cfile.Close();
		}

		CFileException* pError = NULL;
		if (cfile.m_hFile == CFile::hFileNull)
		{
			UINT nOpenFlags = CFile::modeReadWrite | CFile::shareDenyWrite;
			if (bNew)
				nOpenFlags |= CFile::modeCreate;

			if (!cfile.Open(szPath , nOpenFlags, pError))
				nRet = 0;
		}

		if( nRet == 1)
		{
			if (cfile.m_hFile != CFile::hFileNull)
			{
				try
				{
					cfile.SeekToEnd();
					cfile.Write(szLog, iLen) ;
					cfile.Flush();
				}
				catch (...)
				{
					nRet = 0;
				}
			}
		}
	}

	return nRet;
}

void CFDCSenseDlg::CloseDialog()
{
	CDialogEx::OnOK();
}

void CFDCSenseDlg::OnMenuitemExit()
{
// 	if( AfxMessageBox("Do you want to close FDC program?", MB_YESNO|MB_ICONQUESTION) != IDYES)
// 	{
// 		return;
// 	}
// 	else
	{
		NOTIFYICONDATA  nid;
		nid.cbSize = sizeof(nid);
		nid.hWnd = m_hWnd;
		nid.uID = IDI_TRAYICON;	
		Shell_NotifyIcon(NIM_DELETE, &nid);

		CloseTerminal();
	}
}

void CFDCSenseDlg::OnMenuitemOpen() 
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

void CFDCSenseDlg::OnMenuitemWindowstartautorun() 
{
	// TODO: Add your command handler code here
	CString strWrite;
	char    szStr[MAX_PATH*3]={0};

	if(SHRegReadString(HKEY_CURRENT_USER, RUNKEY, m_strAutoRunName/*AUTORUN_VALUENAME*/, " ", szStr, MAX_PATH*2))
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
			SHRegWriteString(HKEY_CURRENT_USER, RUNKEY, m_strAutoRunName/*AUTORUN_VALUENAME*/, strWrite);
			AfxMessageBox("SHRegWriteString");
		}
		else
		{
			SHRegDeleteValue(HKEY_CURRENT_USER, RUNKEY, m_strAutoRunName/*AUTORUN_VALUENAME*/);
		}
	}
	else
	{
		strWrite="\""; 
		strWrite+= __argv[0];
		strWrite+= "\" ";
		strWrite+= AUTORUN_ADDARGU;
		SHRegWriteString(HKEY_CURRENT_USER, RUNKEY, m_strAutoRunName/*AUTORUN_VALUENAME*/, strWrite);
	}
}

void CFDCSenseDlg::CloseTerminal()
{
	KillTimer(100);	
	KillTimer(101);	
	KillTimer(105);	

	if (m_pThreadFDCRead)
	{
		m_dlgPLCRead.m_bReceiveOut = TRUE;
		SetEvent(m_hEvtThreadReadKilled);
	}

	if (m_pThreadFDC)
	{
		SetEvent(m_hEvtThreadKilled);
	}

	if (m_pThreadEfu)
	{
		SetEvent(m_hEvtThreadKill2);
		
		SetEvent(m_hEvtThreadStart);
	}

	m_dlgFDCView.DisConnect();

	SetTimer(103, 3000, NULL);
}

void CFDCSenseDlg::OnOK()
{
	if( AfxMessageBox("Do you want to close FDC program?", MB_YESNO|MB_ICONQUESTION) != IDYES)
	{
		return;
	}
	else
	{
		CloseTerminal();
	}

	CDialogEx::OnOK();
}


void CFDCSenseDlg::OnCancel()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if( AfxMessageBox("Do you want to close FDC program?", MB_YESNO|MB_ICONQUESTION) != IDYES)
	{
		return;
	}
	else
	{
		NOTIFYICONDATA  nid;
		nid.cbSize = sizeof(nid);
		nid.hWnd = m_hWnd;
		nid.uID = IDI_TRAYICON;	
		Shell_NotifyIcon(NIM_DELETE, &nid);	

		CloseTerminal();
		
	}

//	CDialogEx::OnCancel();
}

void CFDCSenseDlg::OnBnClickedBtnExit()
{
	;
}


int CFDCSenseDlg::StartRecive(void)
{
	SetEvent(m_hEvtThreadStart);

	return 0;
}

int CFDCSenseDlg::StopRecive(void)
{
	ResetEvent(m_hEvtThreadStart);

	return 0;
}

BOOL CFDCSenseDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	switch( pMsg->wParam )
	{
	case VK_ESCAPE:
	case VK_RETURN:
		return TRUE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


int CFDCSenseDlg::CreateFDCListFromSheet(void)
{
	m_bSetUpdatedata = false;

	SetFDCHeader();

	CString str;
	int iStarRow = 2;
	int i = 0, j = 0;

	// jsyoon 20200511 add
	str.Empty();
	for (i=0; i<dfMAXFDCITEM; i++)
	{	
		strcpy_s(m_spredFDC[i].cPARARMName, str.GetBuffer(0));
		m_spredFDC[i].dFilterMin = NULL;					
		m_spredFDC[i].dFilterMax  = NULL;	
		m_spredFDC[i].dDefaultVal = 0;	
		m_spredFDC[i].bUse = FALSE;	
		m_spredFDC[i].nSoureType = SOURCE_TYPE_DEFAULT;	
		m_spredFDC[i].ShareMem_num = 0;
		m_spredFDC[i].uSVType = 0;		// jsyoon 20181220 add

		m_dBfFDCValue[i] = -9999.;
	}

	i = 0; 
	for (j = 0; j< dfMAXFDCITEM; j++)
	{
		nIndexNum_Share[i] = -1;

		str = GetFDCListValue(iStarRow + j, COL_FILTER_USE);
		if(_ttoi(str) > 0)
		{
			m_nFDCListCnt++; 
		}
		else
		{
			continue;
		}

		str = GetFDCListValue(iStarRow + j, COL_PARARM_NAME);
		strcpy_s(m_spredFDC[i].cPARARMName, str.GetBuffer(0));
		
		str = GetFDCListValue(iStarRow + j, COL_FILTER_MIN);
		m_spredFDC[i].dFilterMin = _ttof(str);

		str = GetFDCListValue(iStarRow + j, COL_FILTER_MAX);
		m_spredFDC[i].dFilterMax = _ttof(str);

		str = GetFDCListValue(iStarRow + j, COL_FILTER_DEFAULT);
		m_spredFDC[i].dDefaultVal = _ttof(str);

		str =  GetFDCListValue(iStarRow + j, COL_USE);
		m_spredFDC[i].bUse = _ttoi(str);

		//////////////////////////////////////////////////////////////////////////
		str = GetFDCListValue(iStarRow + j, COL_SOURCE_TYPE);
		if(str == "PLC")
		{
			m_spredFDC[i].nSoureType = SOURCE_TYPE_PLC;
			m_nPLCCnt++;
		}
		else if(str =="SENSOR")
		{
			m_spredFDC[i].nSoureType = SOURCE_TYPE_SENSOR;
			m_nSENSORCnt++;
		}
		else if(str =="SHARE")
		{
			m_spredFDC[i].nSoureType = SOURCE_TYPE_SHAHREMEMORY;
			m_nSHAREDCnt++;
		}
		else if(str =="CAPA")
		{
			m_spredFDC[i].nSoureType = SOURCE_TYPE_CAPA;
			nIndexNum_CAPA[m_nCAPACnt] = i;
			m_nCAPACnt++;
		}
		else if(str =="SELF")
		{
			m_spredFDC[i].nSoureType = SOURCE_TYPE_SELF;
			m_nSELFCnt++;
		}
		else if(str =="REG")
		{
			m_spredFDC[i].nSoureType = SOURCE_TYPE_REG;
			nIndexNum_Reg[m_nRegCnt] = i;
			m_nRegCnt++;
		}
		else 
		{
			m_spredFDC[i].nSoureType = SOURCE_TYPE_DEFAULT;
		}
		//////////////////////////////////////////////////////////////////////////

		str = GetFDCListValue(iStarRow + j, COL_SOURCE_ADDRESS);
		int nlength = str.GetLength() + 1;			//add for test
		strcpy_s(m_spredFDC[i].cSourceAddress, nlength, str.GetBuffer(0));  
		str.ReleaseBuffer();

		str = GetFDCListValue(iStarRow + j, COL_DESTINATIO);
		nlength = str.GetLength()+1;	 
		strcpy_s(m_spredFDC[i].cDestination ,nlength, str.GetBuffer(0));
		str.ReleaseBuffer();

		str = GetFDCListValue(iStarRow + j, COL_SHAREMEM_NUM);
		if(str.IsEmpty() != TRUE)
		{
			m_spredFDC[i].ShareMem_num = _ttoi(str);			//Sharememory FDC_WDATA number	
			nIndexNum_Share[i] = m_spredFDC[i].ShareMem_num;			//index number
			m_nSharedMemoryCnt++;
		}
		else
		{
			m_spredFDC[i].ShareMem_num = -1;
			nIndexNum_Share[i] = -1;
		}

		//sypark 20180529 add
		str = GetFDCListValue(iStarRow + j, COL_VALUE_MAGNIFICATION);
		m_spredFDC[i].uiMagnificaion = _ttoi(str);	
		//

		// jsyoon 20181220 add
		str = GetFDCListValue(iStarRow + j, COL_SV_TYPE);
		m_spredFDC[i].uSVType = _ttoi(str);	

		i++;
	}

	UpdateSpreadData();

	m_bSetUpdatedata = true;

	return 0;
}


CString CFDCSenseDlg::GetFDCListValue(int iRow, int iCol)
{
	CString str;
	m_sprdFDCList.SetRow(iRow);
	m_sprdFDCList.SetCol(iCol);
	str = m_sprdFDCList.GetText();

	return str;
}


int CFDCSenseDlg::SetFDCHeader(void)
{
	m_sprdFDCResult.ClearRange(-1, -1, -1, -1, TRUE);

	CString str;
	m_sprdFDCResult.SetCol(1);		
	m_sprdFDCResult.SetRow(0);
	str.Format("Parameter Name");
	m_sprdFDCResult.SetText(str);

	m_sprdFDCResult.SetCol(2);		
	m_sprdFDCResult.SetRow(0);
	str.Format("FDC Value");
	m_sprdFDCResult.SetText(str);

	m_sprdFDCResult.SetCol(3);		
	m_sprdFDCResult.SetRow(0);
	str.Format("Device Num.");
	m_sprdFDCResult.SetText(str);

	m_sprdFDCResult.SetCol(4);		
	m_sprdFDCResult.SetRow(0);
	str.Format("Min.");
	m_sprdFDCResult.SetText(str);

	m_sprdFDCResult.SetCol(5);		
	m_sprdFDCResult.SetRow(0);
	str.Format("Max.");
	m_sprdFDCResult.SetText(str);

	m_sprdFDCResult.SetCol(6);		
	m_sprdFDCResult.SetRow(0);
	str.Format("Default");
	m_sprdFDCResult.SetText(str);

	// jsyoon 20200511 add
	m_sprdFDCResult.SetCol(7);		
	m_sprdFDCResult.SetRow(0);
	str.Format("PLC");
	m_sprdFDCResult.SetText(str);

	return 0;
}


void CFDCSenseDlg::OnBnClickedBtnMini()
{
	// TODO: Add your control notification handler code here
	ShowWindow(SW_HIDE);
}

// jsyoon 20210129 add Lamp Time
int CFDCSenseDlg::GetReadRegistry(void)
{
	CRegistry* pRegistry = new CRegistry(STR_COMPANY_NAME, m_strAPPName);
	
	DWORD dwUsed[INT_DEF_LAMP_MAX], dwLimit[INT_DEF_LAMP_MAX];
	CString strUse, strLimit;
	CTimeSpan spanUsed;

	for (int iLamp=0; iLamp < m_nRegCnt; iLamp++)
	{
	//	strUse.Format("%s%02d", REG_LAMP_USED, iLamp);
		strUse.Format("%s", m_spredFDC[nIndexNum_Reg[iLamp]].cSourceAddress);

		pRegistry->ReadDWord(REG_SETTINGS, strUse,  dwUsed[iLamp],  INT_DEF_LAMP_LIMIT);

		spanUsed	  = dwUsed[iLamp];
		m_pLampTime[iLamp] = (int)spanUsed.GetTotalHours();
	}

	SAFE_DELETE(pRegistry);

	return 0;
}
