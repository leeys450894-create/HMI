// DlgCurrentValue.cpp : implementation file
//

#include "stdafx.h"
#include "FFUMon.h"
#include "DlgCurrentValue.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#include "../include/API_SharedMemory.h"
/////////////////////////////////////////////////////////////////////////////
// CDlgCurrentValue dialog


CDlgCurrentValue::CDlgCurrentValue(CWnd* pParent /*=NULL*/, int nEfuTotalNum, int nIdex )
	: CDialog(CDlgCurrentValue::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgCurrentValue)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nIdex = nIdex;
	m_nEfuTotalNum = nEfuTotalNum;
}


void CDlgCurrentValue::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCurrentValue)
	DDX_Control(pDX, IDC_LIST1, m_list);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgCurrentValue, CDialog)
	//{{AFX_MSG_MAP(CDlgCurrentValue)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgCurrentValue message handlers

BOOL CDlgCurrentValue::OnInitDialog() 
{
	CDialog::OnInitDialog();

	m_list.SetBkColor(RGB(255,255,220));
	m_list.SetTextBkColor(RGB(255,255,220));
	LV_COLUMN lvcolumn;
	
	int i = 0;
	TCHAR* listcolumnResult[9] = {"ID","PV","Alarm","SV","Alarm Check"};
    int widthResult[9] = {120,120,120,120,120};
	for(i=0;i<9;i++)
	{
		lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvcolumn.fmt = LVCFMT_LEFT;
		lvcolumn.pszText = listcolumnResult[i];
		lvcolumn.iSubItem = i;
		lvcolumn.cx = widthResult[i];
		m_list.InsertColumn(i,&lvcolumn);
	}
	m_list.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT |
		                        LVS_EX_HEADERDRAGDROP | LVS_EX_ONECLICKACTIVATE);
	
	SetTimer(1, 100, NULL);

	for(i=0;i<m_nEfuTotalNum;i++)		//20181207 edhan change 32 to 50
	{
		CString strID;
		strID.Format("ID %2d", i+1);
		m_list.InsertItem(i,strID);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgCurrentValue::OnTimer(UINT nIDEvent) 
{
	int iEFUID = 0;
	int iPV = 0;
	int iALARM =0;
	int iSV = 0;

	BOOL bAlarmCheck = FALSE;
	CString strFFUID = _T("");
	CString strPV = _T("");
	CString strALARM = _T("");
	CString strSV = _T("");
	CString strALARMCheck = _T("");


	for(int i=0;i<m_nEfuTotalNum;i++)
	{
		iPV = API_SM_GetFFU_PV(i+1, m_nIdex);
		strPV.Format(_T("%d"), iPV);
		m_list.SetItemText(i,1,strPV);

		iALARM = API_SM_GetFFU_ALARM(i+1, m_nIdex);
		strALARM.Format(_T("%d"), iALARM);
		m_list.SetItemText(i,2,strALARM);

		iSV = API_SM_GetFFU_SV(i+1, m_nIdex);
		strSV.Format(_T("%d"), iSV);
		m_list.SetItemText(i,3,strSV);

		bAlarmCheck = API_SM_GetFFU_ALARM_CHECK(i+1, m_nIdex);
		if(bAlarmCheck)
		{
			strALARMCheck = _T("TRUE");
		}
		else
		{
			strALARMCheck = _T("FALSE");
		}
		m_list.SetItemText(i,4,strALARMCheck);

		int nCount = m_list.GetItemCount();
		if (nCount > m_nEfuTotalNum)		//20181207 edhan change nCount to m_nEfuTotalNum
		m_list.DeleteItem(0);
	}
	
	CDialog::OnTimer(nIDEvent);
}
