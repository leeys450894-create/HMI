// DlgWarning.cpp : implementation file
//
#include "pch.h"
#include "Alarm.h"
#include "DlgWarning.h"
#include <afxmt.h>			//CCriticalSection

//#include "..\\include\\resource_lang.h" //lhj 20181225 lang

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgWarning dialog
CCriticalSection	g_csWarring;

CDlgWarning::CDlgWarning(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgWarning::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgWarning)
	//}}AFX_DATA_INIT

	m_brsStatic.CreateSolidBrush( RGB(243,39,22) );
}


void CDlgWarning::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgWarning)
	DDX_Control(pDX, IDC_LIST_WARNING, m_listWarning);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgWarning, CDialog)
	//{{AFX_MSG_MAP(CDlgWarning)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON_ALARM_CLEAR, OnButtonAlarmClear)
	ON_NOTIFY(NM_CLICK, IDC_LIST_WARNING, OnClickListWarning)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgWarning message handlers

BOOL CDlgWarning::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	InitControls();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CDlgWarning::InitControls()
{
	LV_COLUMN lvcolumn;
	lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	lvcolumn.fmt = LVCFMT_LEFT;
	lvcolumn.pszText = _T("");
	lvcolumn.iSubItem = 0;
	lvcolumn.cx = 800;
	m_listWarning.InsertColumn(0, &lvcolumn);

	m_listWarning.SetExtendedStyle(LVS_EX_FULLROWSELECT);	
}

HBRUSH CDlgWarning::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	switch (nCtlColor) 
	{ 
	case CTLCOLOR_BTN:
	/*		switch (pWnd->GetDlgCtrlID())  
	{
	case IDC_BTN_LOGIN:
	pDC->SetBkColor( RGB(142,142,255) );
	pDC->SetTextColor( RGB(255,255,255) );
	hbr = (HBRUSH) m_brsGemMode;
	break;
	}
		*/
	case CTLCOLOR_STATIC:
	/*		switch (pWnd->GetDlgCtrlID()) {     
	case IDC_STATIC_MESSAGE:
	pDC->SetBkColor( RGB(142,142,255) );
	pDC->SetTextColor( RGB(255,255,255) );
	hbr = (HBRUSH) m_brsStatic;
	break;
	}
		*/
	case CTLCOLOR_EDIT: 
		switch (pWnd->GetDlgCtrlID()) {     
		case IDC_EDIT_WARNING_MSG:
			//pDC->SetBkColor( RGB(240,147,191) );
			pDC->SetBkColor( RGB(243,39,22) );
			pDC->SetTextColor( RGB(255,255,255) );
			hbr = (HBRUSH) m_brsStatic;
			break;
		}
		default:  
			break;
	}


	return hbr;
}

CString CDlgWarning::GetCurDateTime()
{
	CString		sResult = _T("");
	SYSTEMTIME	LocalTime;
	
	::GetLocalTime( &LocalTime );
	sResult.Format(_T("%04d-%02d-%02d %02d:%02d:%02d"),
		LocalTime.wYear, LocalTime.wMonth, LocalTime.wDay, 
		LocalTime.wHour, LocalTime.wMinute, LocalTime.wSecond );
	
	return sResult;
}

void CDlgWarning::InsertList(ALARM_EVENT_DATA *pAlarm)
{
	CString strMsg;
	CString strWarning;
	CString strTime;
	CString strDescription;
	
//	strWarning.Format(_T("[%04d] %s"), pAlarm->id, pAlarm->text);
	strWarning.Format(_T("<%04d> %s"), pAlarm->id, pAlarm->text);
	strMsg.Format(_T("[%s] %s"), pAlarm->happendTime, strWarning);

	int nIndex = m_listWarning.GetItemCount();
// 	if (nCount > 100000)
// 	{
// 		m_listWarning.ResetContent();
// 		nCount = 0;
// 	}
	
	m_listWarning.InsertItem(nIndex, strMsg);

	// 추가된 아이템 선택.
	m_listWarning.SetItemState( -1, 0, LVIS_SELECTED|LVIS_FOCUSED );
	m_listWarning.SetItemState(nIndex, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
	m_listWarning.SetFocus();
	
//	strDescription.Format(_T("<%04d> %s"), pAlarm->code, pAlarm->Description);
//	SetDlgItemText(IDC_EDIT_WARNING_MSG, strDescription);
	SetDlgItemText(IDC_EDIT_WARNING_MSG, strMsg);
}

BOOL CDlgWarning::FindList(UINT code)
{
	g_csWarring.Lock();

	BOOL bRet = FALSE;
	int	nCount = m_listWarning.GetItemCount();
	
	CString str;
	for(int i=0; i<nCount; i++)
	{
		str = m_listWarning.GetItemText(i, 0);
		
	//	unsigned int nCode = _ttoi(str.Mid(23, 4));
		UINT nCode = ParsingALARMCode(str);
		
		if(code == nCode)
		{
			bRet = TRUE;
			break;
		}
	}

	g_csWarring.Unlock();
	
	return bRet;
}

void CDlgWarning::ClearWarning(UINT code)
{
	if(code < 0)
	{
		return;
	}
	
	int	nCount = m_listWarning.GetItemCount();
	
	CString str;
	for(int i=0;i<nCount;i++)
	{
		str = m_listWarning.GetItemText(i, 0);
		
	//	unsigned int nCode = _ttoi(str.Mid(23, 4));
		UINT nCode = ParsingALARMCode(str);
		
		if(code == nCode)
		{
			m_listWarning.DeleteItem(i);
			
			if(m_listWarning.GetItemCount() < 1)
			{
				EndDialog( 1 );
			}
		}
	}
}

void CDlgWarning::OnButtonAlarmClear() 
{

	POSITION pos = m_listWarning.GetFirstSelectedItemPosition();
	int nItem = -1;

	if(pos != NULL)
	{
        while (pos)
        {
			nItem = m_listWarning.GetNextSelectedItem(pos);
			break;
        }
	}

	if(nItem < 0)
	{
		return;
	}

	CString str = m_listWarning.GetItemText(nItem, 0);
	UINT nCode = ParsingALARMCode(str);

//	unsigned int nCode = _ttoi(str.Mid(23, 4));

	// Warning 클리어시 메시지 전송.
	::PostMessage(m_hAlarmWnd, WM_WARNING_CLEARED, WPARAM(nCode), NULL);		// jsyoon 20170731 change

	m_listWarning.DeleteItem(nItem);
	SetDlgItemText(IDC_EDIT_WARNING_MSG, _T(""));

 	if(!m_listWarning.GetItemCount())
 	{
 		EndDialog( 1 );
 		CDialog::OnOK();
 	}
}

void CDlgWarning::OnClickListWarning(NMHDR* pNMHDR, LRESULT* pResult) 
{
	CListCtrl* p=(CListCtrl*)FromHandle(pNMHDR->hwndFrom);
	POSITION pos = p->GetFirstSelectedItemPosition();
	int nItem = p->GetNextSelectedItem(pos);

	CString strMsg = m_listWarning.GetItemText(nItem, 0);
	SetDlgItemText(IDC_EDIT_WARNING_MSG, strMsg);
	
	*pResult = 0;
}

void CDlgWarning::SetAlarmView(HWND hWnd)
{
	m_hAlarmWnd = hWnd;
}

//lhj 20181225 lang Report Warning
// int CDlgWarning::UpdateLanguage(HINSTANCE hResource_Lang_Dll)
/*
{
	TCHAR strText[MAX_PATH]={0,};
	TCHAR strText2[MAX_PATH]={0,};

	if(hResource_Lang_Dll == NULL)
		return 0;

	LoadString(hResource_Lang_Dll,IDS_STRING303_WARNING_MESSAGE, strText,MAX_PATH);
	SetWindowText((CString)strText);

	LoadString(hResource_Lang_Dll,IDS_STRING104_ALARM, strText,MAX_PATH);
	LoadString(hResource_Lang_Dll,IDS_STRING302_CLEAR, strText2,MAX_PATH);
	GetDlgItem(IDC_BUTTON_ALARM_CLEAR)->SetWindowText((CString)strText + (CString) " " + (CString)strText2);	



	return 0;
}
*/

UINT CDlgWarning::ParsingALARMCode(CString str)
{
	int iStart = str.Find(_T("<"));
	int iEnd = str.Find(_T(">"));

	CString strTemp = str.Mid(iStart + 1, iEnd - (iStart + 1));

	return _ttoi(strTemp);
}