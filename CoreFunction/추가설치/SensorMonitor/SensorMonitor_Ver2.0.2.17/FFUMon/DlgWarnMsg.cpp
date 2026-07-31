// DlgWarnMsg.cpp : implementation file
//

#include "stdafx.h"
#include "FFUMon.h"
#include "DlgWarnMsg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgWarnMsg dialog


CDlgWarnMsg::CDlgWarnMsg(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgWarnMsg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgWarnMsg)
	m_strWarnMsg = _T("");
	//}}AFX_DATA_INIT

	m_hbrushBK = ::CreateSolidBrush(RGB(115, 141, 172));
	m_lWarnStatus = 0;
}


void CDlgWarnMsg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgWarnMsg)
	DDX_Control(pDX, IDC_LIST_WARMMSG, m_lstbxMsg);
	DDX_Text(pDX, IDC_EDIT_WARN_MESSAGE, m_strWarnMsg);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgWarnMsg, CDialog)
	//{{AFX_MSG_MAP(CDlgWarnMsg)
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CDlgWarnMsg::OnBnClickedOk)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgWarnMsg message handlers

BOOL CDlgWarnMsg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_brsStatic.CreateSolidBrush( RGB(243,39,22) );	
	
	//	CenterWindow();	// jsyoon 20140212 remove
	//CRect rClient;
//	GetParentFrame()->GetWindowRect(&rClient);
	//	SetWindowPos(&wndTopMost, rClient.right+5, rClient.top+2, 0, 0, SWP_NOSIZE);
	//SetWindowPos(&wndTopMost, 6, rClient.top+2, 0, 0, SWP_NOSIZE);	// jsyoon 20140327 change

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

HBRUSH CDlgWarnMsg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	switch (nCtlColor) 
	{ 
	case CTLCOLOR_BTN:
	case CTLCOLOR_STATIC:
	case CTLCOLOR_EDIT: 
		switch (pWnd->GetDlgCtrlID()) {     
		case IDC_EDIT_WARN_MESSAGE:
			//pDC->SetBkColor( RGB(240,147,191) );
			pDC->SetBkColor( RGB(243,39,22) );
			pDC->SetTextColor( RGB(255,255,255) );
			hbr = (HBRUSH) m_brsStatic;
			break;
		}
		default:  
			break;
	}
	
	if (pWnd == this)
		return m_hbrushBK;

	return hbr;
}

CString CDlgWarnMsg::GetCurDateTime()
{
	CString		sResult = _T("");
	SYSTEMTIME	LocalTime;
	
	::GetLocalTime( &LocalTime );
	sResult.Format("%04d%02d%02d%02d%02d%02d",
		LocalTime.wYear, LocalTime.wMonth, LocalTime.wDay, LocalTime.wHour,
		LocalTime.wMinute, LocalTime.wSecond );
	
	return sResult;
}

void CDlgWarnMsg::SetMsg(const char *pszMsg)
{
	TCHAR	szTemp[256];
	CString sTime;
	
	sTime = GetCurDateTime();
	
// 	//	sprintf( szTemp, "[%s-%s-%s %s:%s:%s] %s",
// 	sprintf( szTemp, "%s-%s-%s %s:%s:%s>>%s",	
// 		sTime.Mid(0,4), sTime.Mid(4,2),
// 		sTime.Mid(6,2), sTime.Mid(8,2),
// 		sTime.Mid(10,2), sTime.Mid(12,2), pszMsg );

	_stprintf_s(szTemp, _countof(szTemp), "%s-%s-%s %s:%s:%s>>%s",	
		sTime.Mid(0,4), sTime.Mid(4,2),
		sTime.Mid(6,2), sTime.Mid(8,2),
		sTime.Mid(10,2), sTime.Mid(12,2), pszMsg );

	m_strWarnMsg = szTemp;
	
	//	UpdateData( FALSE );
	GetDlgItem(IDC_EDIT_WARN_MESSAGE)->SetWindowText(m_strWarnMsg);
}

void CDlgWarnMsg::AddMsg(char szMsg[])
{
	int	nCount = 0;
	TCHAR	szTemp[256];
	CString sTime;
	
	sTime = GetCurDateTime();
	
	//	sprintf( szTemp, "[%s-%s-%s %s:%s:%s] %s",
// 	sprintf( szTemp, "%s-%s-%s %s:%s:%s>>%s",
// 		sTime.Mid(0,4), sTime.Mid(4,2),
// 		sTime.Mid(6,2), sTime.Mid(8,2),
// 		sTime.Mid(10,2), sTime.Mid(12,2), szMsg );

	_stprintf_s(szTemp, _countof(szTemp), "%s-%s-%s %s:%s:%s>>%s",	
		sTime.Mid(0,4), sTime.Mid(4,2),
		sTime.Mid(6,2), sTime.Mid(8,2),
		sTime.Mid(10,2), sTime.Mid(12,2), szMsg );

	if ( (nCount = m_lstbxMsg.GetCount()) > 10 ) {
		m_lstbxMsg.ResetContent();
	}
	m_lstbxMsg.InsertString( 0, szTemp );
}

void CDlgWarnMsg::OnCancel() 
{
	EndDialog( 0 );
	
	CDialog::OnCancel();
}

void CDlgWarnMsg::OnOK() 
{
	int n = m_lstbxMsg.GetCurSel();
	
	if(n<0)
	{
		//		//2010.07.29								// pwk 20101102 remove.
		//		g_pSysMapView->m_lWarnStatus = 0;
		//
		//		EndDialog( 1 );
		//		CDialog::OnOK();
		return;
	}
	
	CString str;
	m_lstbxMsg.GetText( n, str );
	
	//	CString strAlarmCode = str.Mid(23,4);
	CString strAlarmCode = GetDataFromString(str, "[", "]");	// jsyoon 20140212 change
	
	long nAlarmCode = 0;
	nAlarmCode = _tstoi(strAlarmCode);

	m_lstbxMsg.DeleteString(n);
	if(!m_lstbxMsg.GetCount())
	{
		m_lWarnStatus = 0;
		
		EndDialog( 1 );
		CDialog::OnOK();
	}
	
	CDialog::OnOK();
}

void CDlgWarnMsg::ClearWarning(long nWarnCode)
{
	int n = nWarnCode;
	
	if(n<0)
		return;
	
	int	nCount;
	nCount = m_lstbxMsg.GetCount();
	
	CString str = _T("");
	CString strAlarmCode = _T("");
	for(int i=0;i<nCount;i++)
	{
		m_lstbxMsg.GetText( i, str );
		
		//		CString strAlarmCode = str.Mid(23,4);
		strAlarmCode = GetDataFromString(str, "[", "]");	// jsyoon 20140212 change
		
		long n = atoi(strAlarmCode);
		
		if(nWarnCode == n)
		{
			m_lstbxMsg.DeleteString(n);
			
			//api_FA_LGD_SetAlarm(g_pSysMapView->nECSGlassStatus,FALSE, FALSE, nWarnCode);
			//			m_pDlgNaviMaint->m_dlgNaviMaintPLC.SetMelody(PLC_OUTPUT_SIGNAL_TOWER_MELODY,FALSE);	// jsyoon 20140218 remove
			
			if(!m_lstbxMsg.GetCount())
			{
				EndDialog( 1 );
			}
		}
	}
}

CString CDlgWarnMsg::GetDataFromString(CString inStr, CString startStr, CString endStr)
{
	CString strReturn;
	int iStart	= inStr.Find(startStr);	
	int iEnd	= inStr.Find(endStr, iStart);
	if (iStart < iEnd)
	{
		strReturn = inStr.Mid(iStart + startStr.GetLength(), iEnd - iStart - startStr.GetLength());
		return strReturn;
	}
	else if (iStart > iEnd)
	{
		ASSERT(FALSE);
	}
	
	return "";
}

void CDlgWarnMsg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	::DeleteObject(m_hbrushBK);
	
}

BOOL CDlgWarnMsg::IsAlreadyAlarmHappened(long nWarnCode)
{
	int	nCount;
	nCount = m_lstbxMsg.GetCount();
	
	CString str = _T("");;
	CString strAlarmCode = _T("");
	for(int i=0;i<nCount;i++)
	{
		m_lstbxMsg.GetText( i, str );
		
		//		CString strAlarmCode = str.Mid(23,4);
		strAlarmCode = GetDataFromString(str, "[", "]");	// jsyoon 20140212 change
		
		long n = atoi(strAlarmCode);
		
		if(nWarnCode == n)
		{
			//			m_lstbxMsg.DeleteString(i);	// jsyoon20140212 change
			return TRUE;
		}
	}
	
	return FALSE;
}


void CDlgWarnMsg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialog::OnOK();
}
