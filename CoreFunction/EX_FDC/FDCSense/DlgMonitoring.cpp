// DlgMonitoring.cpp : implementation file
//

#include "stdafx.h"
#include "FDCSense.h"
#include "DlgMonitoring.h"
#include "..\Include/FDCDefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgMonitoring dialog

CDlgMonitoring::CDlgMonitoring(CDlgPLC& _pDlgPLC, CDlgPLCDRead& _dlgPLCRead, CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgMonitoring::IDD, pParent)
	, m_pDlgPLC(_pDlgPLC)
	, m_pDlgPLCRead(_dlgPLCRead)
	, m_strTime(_T(""))
{
	//{{AFX_DATA_INIT(CDlgMonitoring)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_ledbLive = FALSE;

	//sypark 20180608 add
	for(int i=0; i<2; i++)
	{
		m_dNewData[i]  = 0;
		m_dPrevData[i] = -1;
	}

	strData[4] = _T("");	
	//
}


void CDlgMonitoring::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgMonitoring)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_CHK_LIVE, m_ledLive);
	//}}AFX_DATA_MAP	
}


BEGIN_MESSAGE_MAP(CDlgMonitoring, CDialogEx)
	//{{AFX_MSG_MAP(CDlgMonitoring)
	ON_WM_TIMER()
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDCANCEL, &CDlgMonitoring::OnBnClickedCancel)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgMonitoring message handlers

BOOL CDlgMonitoring::OnInitDialog() 
{
	CDialogEx::OnInitDialog();

	MoveSubMonitor();	//mn.bong 20180328 add

// 	strcpy(m_szName[0], "default");
// 	strcpy(m_szName[1], "default");
	
	m_bUiUse = FALSE;

	m_bLive = FALSE;

	GetConfig();
	
	if(m_bUiUse)
	{
		m_font.CreatePointFont(1000, TEXT("맑은고딕"));
		GetDlgItem(IDC_STATIC_NAME1)->SetFont(&m_font);
		GetDlgItem(IDC_STATIC_NAME2)->SetFont(&m_font);
		GetDlgItem(IDC_STATIC_LIVE)->SetFont(&m_font);
		
		m_font2.CreatePointFont(1500, TEXT("맑은고딕"));
		GetDlgItem(IDC_STATIC_DATA1)->SetFont(&m_font2);
		GetDlgItem(IDC_STATIC_DATA2)->SetFont(&m_font2);

		CString strName;
		strName.Format(_T("%s"), m_szName[0]);
		SetDlgItemText(IDC_STATIC_NAME1, strName);
		
		strName.Format(_T("%s"), m_szName[1]);
		SetDlgItemText(IDC_STATIC_NAME2, strName);

		LOGFONT lf;
		::ZeroMemory(&lf, sizeof(lf));
		lf.lfHeight = 70;
		lf.lfWeight = FW_BOLD;
		wsprintf(lf.lfFaceName, _T("%s"), _T("Arial"));

		m_font3.CreateFontIndirect(&lf);
//		m_font3.CreatePointFont(300, TEXT("맑은고딕"));
		GetDlgItem(IDC_STATIC_TIME)->SetFont(&m_font3);
	}

	m_ledLive	 .SetImage(IDB_LED_GREENRED20, 24);

	if(m_strEQName == "STER")
	{
		m_bmpBk.LoadBitmap(IDB_STER);
	}
	else if(m_strEQName == "STEC")
	{
		m_bmpBk.LoadBitmap(IDB_STEC);
	}
	else if(m_strEQName == "OVLI")
	{
		m_bmpBk.LoadBitmap(IDB_OVLI);
	}
	else
	{
		m_bmpBk.LoadBitmap(IDB_OTHERS);
	}

	m_bmpBk.GetBitmap(&m_bmBk);

// 	SetTimer(101, 1000, NULL);
 	SetTimer(2,1000,NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgMonitoring::OnTimer(UINT nIDEvent) 
{
	switch (nIDEvent)
	{
	case 1:
		{
//			UpdateMonitoringUI(); 
		}
		break;
	case 2:
		{
			BTN_CHECK(m_ledLive, m_ledbLive );
			m_ledbLive = !m_ledbLive;
		}
		break;
	case 101:			//[MONITORING_1]		// gdh 20180328 add
		{
			if(m_dPrevData[0] != m_dNewData[0] || m_dPrevData[1] != m_dNewData[1] || m_dPrevData[2] != m_dNewData[2] )
			{
				// O2
				if( m_dNewData[0] >= m_dUp[0])
				{
					strData[0].Format(_T("%.2f"), m_dNewData[0]);		
					strData[5].Format(_T("%s"),   m_strDataUnit[0]);
				}
				else
				{
					strData[0].Format(_T("%.2f"), m_dNewData[2]);		
					strData[5].Format(_T("%s"), m_strDataUnit[1]);
				}
				
				// H2O
				strData[1].Format(_T("%.2f"), m_dNewData[1]);
				strData[6].Format(_T("%s"), m_strDataUnit[1]);

				// EQ
				strData[4].Format("%s", m_strMonitorName);

				m_dPrevData[0] = m_dNewData[0];
				m_dPrevData[1] = m_dNewData[1];
				m_dPrevData[2] = m_dNewData[2];

				Invalidate(FALSE);
			}

			SYSTEMTIME st;
			::GetLocalTime(&st);
			m_strTime.Format("%04d.%02d.%02d-%02d:%02d:%02d",st.wYear ,st.wMonth ,st.wDay ,st.wHour ,st.wMinute ,st.wSecond);

			CRect Rect;  
			GetDlgItem(IDC_STATIC_TIME)->GetWindowRect(&Rect); ScreenToClient(&Rect); InvalidateRect(Rect);

			// 			GetDlgItem(IDC_STATIC_TIME)->SetWindowText(m_strTime);
			SetDlgItemText(IDC_STATIC_TIME, m_strTime);
		}
		break;

// 		if(m_nIndex == 1)
// 		{
// 			int nSData2 = dfMAXFDCITEM - (2*m_nIndex);
// 			int nEData1 = dfMAXFDCITEM - (2*m_nIndex -1);
// 			m_pDlgPLC.m_valArray[nSData2] = m_dPrevData[0][m_nIndex-1] + 5;
// 			if(m_dPrevData[0][m_nIndex-1] != m_pDlgPLC.m_valArray[nSData2] || m_dPrevData[1][m_nIndex-1] != m_pDlgPLC.m_valArray[nEData1])
// 			{
// 				m_dPrevData[0][m_nIndex-1] = m_pDlgPLC.m_valArray[nSData2];
// 				m_dPrevData[1][m_nIndex-1] = m_pDlgPLC.m_valArray[nEData1];
// 
// 				strData[0].Format(_T("%.2f"), m_dPrevData[0][m_nIndex-1]);	
// 				strData[1].Format(_T("%.2f"), m_dPrevData[1][m_nIndex-1]);
// 				strData[4] = m_strMonitorName[m_nIndex-1];
// 
// 				Invalidate(FALSE);
// 			}
// 		}
// 		else if(m_nIndex == 2)
// 		{
// 			int nSData2 = dfMAXFDCITEM - (2*m_nIndex);
// 			int nEData1 = dfMAXFDCITEM - (2*m_nIndex -1);
// 			if(m_dPrevData[0][m_nIndex-1] != m_pDlgPLC.m_valArray[nSData2] || m_dPrevData[1][m_nIndex-1] != m_pDlgPLC.m_valArray[nEData1])
// 			{
// 				m_dPrevData[0][m_nIndex-1] = m_pDlgPLC.m_valArray[nSData2];
// 				m_dPrevData[1][m_nIndex-1] = m_pDlgPLC.m_valArray[nEData1];
// 
// 				strData[0].Format(_T("%.2f"), m_dPrevData[0][m_nIndex-1]);	
// 				strData[1].Format(_T("%.2f"), m_dPrevData[1][m_nIndex-1]);
// 				strData[4] = m_strMonitorName[m_nIndex-1];
// 
// 				Invalidate(FALSE);
// 			}	
// 		}
// 		else if(m_nIndex ==3)
// 		{
// 			int nSData2 = dfMAXFDCITEM - (2*m_nIndex);
// 			int nEData1 = dfMAXFDCITEM - (2*m_nIndex -1);
// 			m_pDlgPLC.m_valArray[nSData2] = m_dPrevData[0][m_nIndex-1] + 10;
// 			if(m_dPrevData[0][m_nIndex-1] != m_pDlgPLC.m_valArray[nSData2] || m_dPrevData[1][m_nIndex-1] != m_pDlgPLC.m_valArray[nEData1])
// 			{
// 				m_dPrevData[0][m_nIndex-1] = m_pDlgPLC.m_valArray[nSData2];
// 				m_dPrevData[1][m_nIndex-1] = m_pDlgPLC.m_valArray[nEData1];
// 
// 				strData[0].Format(_T("%.2f"), m_dPrevData[0][m_nIndex-1]);	
// 				strData[1].Format(_T("%.2f"), m_dPrevData[1][m_nIndex-1]);
// 				strData[4] = m_strMonitorName[m_nIndex-1];
// 
// 				Invalidate(FALSE);
// 			}
// 		}
// 
// 	//	Invalidate(FALSE);
// 		break;
	/////////////////////////////////////////////////////////////
	default:
		ASSERT(0);
		break;
	}

	CDialogEx::OnTimer(nIDEvent);
}

void CDlgMonitoring::UpdateMonitoringUI()
{
	CString strMsg;
	m_nIndex =1;

//	strMsg.Format(_T("%.2f %%"), m_pDlgPLC.m_valArray[dfMAXFDCITEM-2]);
	strMsg.Format(_T("%.2f %%"), m_dNewData[0]);
	SetDlgItemText(IDC_STATIC_DATA1, strMsg);

//	strMsg.Format(_T("%.2f %%"), m_pDlgPLC.m_valArray[dfMAXFDCITEM-1]);
	strMsg.Format(_T("%.2f %%"), m_dNewData[1]);
	SetDlgItemText(IDC_STATIC_DATA2, strMsg);

//	m_bLive = !m_bLive;
//
	if(m_bLive)
	{
		SetDlgItemText(IDC_STATIC_LIVE, "LIVE");
//		GetDlgItem(IDC_STATIC_LIVE)->SetTextColor(RGB(255, 255, 0));
	}
	else
	{
//		GetDlgItem(IDC_STATIC_LIVE)->SetTextColor(RGB(0, 255, 0));
		SetDlgItemText(IDC_STATIC_LIVE, "CLOSE");
	}	
}

void CDlgMonitoring::GetConfig() 
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
	
	GetPrivateProfileString ("UI DISPLAY","Use","0",szValue,MAX_PATH,szConfigFile);
	m_bUiUse = atoi(szValue);
	
	//sypark 20180502 add
	GetPrivateProfileString ("UI DISPLAY","UseMonitorNum","0",szValue,MAX_PATH,szConfigFile);
	m_nUseMonitorNum = atoi(szValue);
	GetPrivateProfileString ("UI DISPLAY","MonitorW","0",szValue,MAX_PATH,szConfigFile);
	m_nMonitorWidth = atoi(szValue);
	GetPrivateProfileString ("UI DISPLAY","MonitorH","0",szValue,MAX_PATH,szConfigFile);
	m_nMonitorHeight = atoi(szValue);

	CString str;
	str.Format("MONITORING_%d", m_nIndex);

	GetPrivateProfileString (str,"EQNAME","",szValue,MAX_PATH,szConfigFile);
	m_strMonitorName.Format("%s",szValue);
	GetPrivateProfileString (str,"Monitor_posX","0",szValue,MAX_PATH,szConfigFile);
	m_nMonitorPosX = atoi(szValue);
	GetPrivateProfileString (str,"Monitor_posY","0",szValue,MAX_PATH,szConfigFile);
	m_nMonitorPosY = atoi(szValue);

// 	for(int i = 0; i < 10; i ++)
// 	{
// 		CString str;
// 		str.Format("MONITORING_%d", i+1);
// 
// 		GetPrivateProfileString (str,"EQNAME","",szValue,MAX_PATH,szConfigFile);
// 		m_strMonitorName[i].Format("%s",szValue);
// 
// 		GetPrivateProfileString (str,"Monitor_posX","0",szValue,MAX_PATH,szConfigFile);
// 		m_nMonitorPosX[i] = atoi(szValue);
// 		GetPrivateProfileString (str,"Monitor_posY","0",szValue,MAX_PATH,szConfigFile);
// 		m_nMonitorPosY[i] = atoi(szValue);
// 	}
	//

	CString strData;
	for(int i = 0; i < 2; i++)
	{
		strData.Format("NAME%d", i+1);	
		GetPrivateProfileString ("UI DISPLAY",strData,"11500",m_szName[i],MAX_PATH,szConfigFile);
			
		strData.Format("NAMESIZE", i+1);
		GetPrivateProfileString ("UI DISPLAY",strData,"53",szValue,MAX_PATH,szConfigFile);
		m_nFontSize = atoi(szValue);


		strData.Format("OVER%d", i+1);
		GetPrivateProfileString ("UI DISPLAY",strData,"1",szValue,MAX_PATH,szConfigFile);
		m_dUp[i] = atof(szValue);

		strData.Format("DOWN%d", i+1);
		GetPrivateProfileString ("UI DISPLAY",strData,"0",szValue,MAX_PATH,szConfigFile);	
		m_dDown[i] = atof(szValue);

		strData.Format("UNIT%d",i+1);
		GetPrivateProfileString ("UI DISPLAY",strData,"%",szValue,MAX_PATH,szConfigFile);
		m_strDataUnit[i].Format("%s",szValue);

		strData.Format("BelowIsStable%d",i+1);
		GetPrivateProfileString ("UI DISPLAY",strData,"1",szValue,MAX_PATH,szConfigFile);
		m_bBelowIsStable[i] = atoi(szValue);
		///////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////
	}
}

HBRUSH CDlgMonitoring::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	if(pWnd->GetSafeHwnd() == GetDlgItem(IDC_STATIC_TIME)->m_hWnd)
    {
		pDC->SetTextColor(RGB(255, 255, 255));
		pDC->SetBkMode(TRANSPARENT);
		return (HBRUSH)GetStockObject(NULL_BRUSH);
    }

	// TODO: Return a different brush if the default is not desired
	return hbr;
}

// gdh 20180322 add
void CDlgMonitoring::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	ModifyStyle(WS_CAPTION, 0);

    CDC memDC;                 // 메모리에 상주하기 위해서 DC를 생성한다.
    memDC.CreateCompatibleDC(&dc); 
 	
	LOGFONT lf;
	CFont NewFont, NewFont1, NewFont2, NewFont3, NewFontt;
	CFont * pOldFont;
	{
		strData[2].Format(_T("%s"), m_szName[0]);
		strData[3].Format(_T("%s"), m_szName[1]);

		::ZeroMemory(&lf, sizeof(lf));
		lf.lfHeight = 200;
		lf.lfWeight = FW_BOLD;
		wsprintf(lf.lfFaceName, _T("%s"), _T("Arial"));

		
		NewFont.CreateFontIndirect(&lf);
		pOldFont = dc.SelectObject(&NewFont);

		dc.SetTextAlign(TA_CENTER);									
		dc.SetBkMode(TRANSPARENT);
		dc.SetTextColor(RGB(51, 51, 255));
		dc.TextOut(508, 320, strData[0]);		//DATA1

		dc.SetTextColor(RGB(51, 51, 255));
		dc.TextOut(1405, 320, strData[1]);		//DATA2

		dc.SelectObject(pOldFont);

		::ZeroMemory(&lf, sizeof(lf));
		lf.lfHeight = m_nFontSize;
		lf.lfWeight = FW_BOLD;
		wsprintf(lf.lfFaceName, _T("%s"), _T("Arial"));

		
		NewFont1.CreateFontIndirect(&lf);
		pOldFont = dc.SelectObject(&NewFont1);

		dc.SetTextAlign(TA_CENTER);

		dc.SetTextColor(RGB(255, 255, 255));
		dc.TextOut(348, 700, strData[2]);				//NAME1

		dc.SetTextColor(RGB(255, 255, 255));
		dc.TextOut(1236, 700, strData[3]);				//NAME2

		dc.SelectObject(pOldFont);

		::ZeroMemory(&lf, sizeof(lf));
		lf.lfHeight = 33;
		lf.lfWeight = FW_BOLD;
		wsprintf(lf.lfFaceName, _T("%s"), _T("Arial"));

		NewFont2.CreateFontIndirect(&lf);
		pOldFont = dc.SelectObject(&NewFont2);

		dc.SetTextAlign(TA_RIGHT);

		dc.SetTextColor(RGB(255, 255, 255));
		dc.TextOut( 472, 746, strData[5] /*m_strDataUnit[0]*/);				//UNIT1

		dc.SetTextColor(RGB(255, 255, 255));
		dc.TextOut(1362, 746, strData[6] /*m_strDataUnit[1]*/);			//UNIT2

		dc.SelectObject(pOldFont);

		::ZeroMemory(&lf, sizeof(lf));
		lf.lfHeight = 43;
		lf.lfWeight = FW_BOLD;
		wsprintf(lf.lfFaceName, _T("%s"), _T("Arial"));

		NewFont3.CreateFontIndirect(&lf);
		pOldFont = dc.SelectObject(&NewFont3);

		dc.SetTextAlign(TA_CENTER);

		CString state1, state2;
		SetStateMessage(state1,0);
		if(state1 == _T("STABLE"))
		{
			dc.SetTextColor(RGB(0, 102, 51));
		} 
		else if(state1 == _T("NORMAL"))
		{
			dc.SetTextColor(RGB(163, 163, 163));
		}
		else
		{
			dc.SetTextColor(RGB(255, 0, 0));
		}
		dc.TextOut(660, 718, state1);			// STABLE1

		SetStateMessage(state2,1);
		if(state2 == _T("STABLE"))
		{
			dc.SetTextColor(RGB(0, 102, 51));
		} 
		else if(state2 == _T("NORMAL"))
		{
			dc.SetTextColor(RGB(163, 163, 163));
		}
		else
		{
			dc.SetTextColor(RGB(255, 0, 0));
		}
		dc.TextOut(1547, 718, state2);			// STABLE2

		if(m_strEQName != "STEC" && m_strEQName != "STER" && m_strEQName != "OVLI")
		{
			::ZeroMemory(&lf, sizeof(lf));
			lf.lfHeight = 70;
			lf.lfWeight = FW_BOLD;
			wsprintf(lf.lfFaceName, _T("%s"), _T("Arial"));

			
			NewFontt.CreateFontIndirect(&lf);
			CFont * pOldFont = dc.SelectObject(&NewFontt);

			dc.SetTextAlign(TA_CENTER);
			dc.SetBkMode(TRANSPARENT);

			if( m_bLive)
				dc.SetTextColor(RGB(51, 51, 255));
			else
				dc.SetTextColor(RGB(255, 0, 0));

			//dc.TextOut(950, 880, m_strEQName);		//DATA1		//sypark 20180608 change
			dc.TextOut(950, 880, strData[4]);		//DATA1
		}
	}

	memDC.DeleteDC();
	
}

// gdh 20180322 add

void CDlgMonitoring::OnSize(UINT nType, int cx, int cy) 
{
	CDialogEx::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	return;		// jsyoon 20200511 add

	CRect rt;
	AfxGetMainWnd()->GetWindowRect(&rt);

	CRgn rgn;
	rgn.CreateRectRgn(0,0,rt.Width(), rt.Height());
	AfxGetMainWnd()->SetWindowRgn((HRGN)rgn, TRUE);
}

void CDlgMonitoring::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialogEx::OnCancel();
}


// gdh 20180324 add
void CDlgMonitoring::SetStateMessage(CString &state, int num)
{
	//mn.bong 20180328 add
	int l_nState = num;
// 	int NumberValue = 0;
// 
// 	if (num == 0)
// 		NumberValue = dfMAXFDCITEM-2;
// 	else
// 		NumberValue = dfMAXFDCITEM-1;
	//

	if(m_bBelowIsStable[num])
	{
//		if(m_pDlgPLC.m_valArray[m_nDataNumber[num]-1]<m_dDown[num])
//		if(m_pDlgPLC.m_valArray[NumberValue] < m_dDown[num])	//mn.bong 20180328 change
		if(m_dNewData[num] < m_dDown[num])	// jsyoon 20180907 change
		{
			state = "STABLE";
		}
//		else if(m_pDlgPLC.m_valArray[m_nDataNumber[num]-1]>m_dDown[num] && m_pDlgPLC.m_valArray[m_nDataNumber[num]-1]<m_dUp[num])
//		else if(m_pDlgPLC.m_valArray[NumberValue]>m_dDown[num] && m_pDlgPLC.m_valArray[NumberValue]<m_dUp[num])	//mn.bong 20180328 change
		else if(m_dNewData[num] >m_dDown[num] && m_dNewData[num] < m_dUp[num])	// jsyoon 20180907 change
		{
			state = _T("NORMAL");
		}
		else
		{
			state = _T("WARNNING");
		}
	}

	else
	{
//		if(m_pDlgPLC.m_valArray[m_nDataNumber[num]-1]<m_dDown[num])
//		if(m_pDlgPLC.m_valArray[NumberValue]<m_dDown[num])	//mn.bong 20180328 change
		if(m_dNewData[num] <m_dDown[num])	//mn.bong 20180328 change
		{
			state = _T("WARNNING");
		}
//		else if(m_pDlgPLC.m_valArray[m_nDataNumber[num]-1]>m_dDown[num] && m_pDlgPLC.m_valArray[m_nDataNumber[num]-1]<m_dUp[num])
//		else if(m_pDlgPLC.m_valArray[NumberValue]>m_dDown[num] && m_pDlgPLC.m_valArray[NumberValue]<m_dUp[num])	//mn.bong 20180328 change
		else if(m_dNewData[num]>m_dDown[num] && m_dNewData[num]<m_dUp[num])	//mn.bong 20180328 change
		{
			state = _T("NORMAL");
		}
		else
		{
			state = _T("STABLE");
		}
	}
}

//mn.bong 20180328 add
void CDlgMonitoring::MoveSubMonitor()
{
	int nMonitors = ::GetSystemMetrics(SM_CMONITORS);
	
	int nMainLeft   = ::GetSystemMetrics(SM_XVIRTUALSCREEN);
	
	int nMainTop    = ::GetSystemMetrics(SM_YVIRTUALSCREEN);
	
	int nMainWidth  = ::GetSystemMetrics(SM_CXSCREEN);
	
    int nMainHeight = ::GetSystemMetrics(SM_CYSCREEN);
	
	if (nMonitors == 2)
	{
		if(nMainLeft == 0)
			nMainLeft = nMainWidth;
	}
	
	MoveWindow(nMainLeft, nMainTop, nMainWidth, nMainHeight);
}
//

void CDlgMonitoring::OnBnClickedCancel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CDialogEx::OnCancel();
}


void CDlgMonitoring::StartUIMonitor(void)
{
//	SetTimer(1, 2000, NULL);

	SetTimer(101, 500, NULL);
}


void CDlgMonitoring::Prepare(int nIndex)
{
	m_nIndex= nIndex;
}


int CDlgMonitoring::SetUIUpdateData(double data1, double data2, double data3, BOOL bLive)
{
	m_dNewData[0] = data1;
	m_dNewData[1] = data2;
	m_dNewData[2] = data3;
	m_bLive = bLive;
	return 0;
}


BOOL CDlgMonitoring::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	switch(pMsg->message)
	{
	case WM_KEYDOWN:
		if(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
			return TRUE;
		break;
	case WM_LBUTTONDBLCLK:
		if(pMsg->message == WM_LBUTTONDBLCLK)
		{
//			if (AfxMessageBox(_T("Do you want to Hide?"),MB_YESNO|MB_ICONQUESTION) ==  IDYES)
			{
				this->ShowWindow(SW_HIDE);

				return TRUE;
			}
		}
		break;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CDlgMonitoring::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	DeleteObject(m_bmpBk);
}


BOOL CDlgMonitoring::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	DrawBackGround(pDC);

	return TRUE;

//	return CDialog::OnEraseBkgnd(pDC);
}

/*===========================================================================================
Function : View에서 Back Ground Image를 그려주기 위함
Date : 20120117
Added by:Jay Hong
===========================================================================================*/
void CDlgMonitoring::DrawBackGround(CDC *pDC)
{
	CDC MemDC;
	MemDC.CreateCompatibleDC(pDC);

	CRect rect;
	GetClientRect(&rect);

	CBitmap* pOld = MemDC.SelectObject(&m_bmpBk);
	
	pDC->BitBlt(0 ,0, rect.Width(), rect.Height(), &MemDC, 0, 0, SRCCOPY);
	pDC->StretchBlt(0,0,rect.Width(), rect.Height(), &MemDC, 0, 0, m_bmBk.bmWidth, m_bmBk.bmHeight, SRCCOPY);		// 이미지 사이즈 변경

	MemDC.SelectObject(pOld);
	MemDC.DeleteDC();
}