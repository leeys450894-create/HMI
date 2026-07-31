// DlgReportAlarm.cpp : implementation file
//

#include "pch.h"
#include "Alarm.h"
#include "DlgReportAlarm.h"
#include "DlgReportAlarmSearch.h"

//#include "..\\include\\resource_lang.h" //lhj 20181225 lang

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
// CDlgReportAlarm dialog

CDlgReportAlarm::CDlgReportAlarm(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgReportAlarm::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgReportAlarm)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_hbrushBK = ::CreateSolidBrush(RGB(225, 231, 253));

	m_strRootPath.Empty();

	m_nHostMode = df_HOST_OFFLINE;
}


void CDlgReportAlarm::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgReportAlarm)
	DDX_Control(pDX, IDC_LIST_REPORT_ALARM, m_listReportAlarm);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgReportAlarm, CDialog)
	//{{AFX_MSG_MAP(CDlgReportAlarm)
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_ALARM_REPORT_CLEAR, &CDlgReportAlarm::OnBnClickedButtonAlarmReportClear)
	ON_BN_CLICKED(IDC_BUTTON_ALARM_REPORT_SEARCH, &CDlgReportAlarm::OnBnClickedButtonAlarmReportSearch)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgReportAlarm message handlers

HBRUSH CDlgReportAlarm::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	if (pWnd == this)	return m_hbrushBK;

	return hbr;
}

void CDlgReportAlarm::OnDestroy() 
{
	CDialog::OnDestroy();
	
	::DeleteObject(m_hbrushBK);
}

BOOL CDlgReportAlarm::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_listReportAlarm.SetBkColor(RGB(255,255,220));
	m_listReportAlarm.SetTextBkColor(RGB(255,255,220));
	LV_COLUMN lvcolumn;
	TCHAR* listcolumnResult[9] = { _T("Happened Time"),_T("Cleared Time"),_T("Module"),_T("Alarm ID"),_T("Alarm Code"),_T("Host Mode"),_T("Command"),_T("String")};
    int widthResult[9] = {160,160,80,80,80,110,200,500};
	for(int i=0;i<9;i++)
	{
		lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvcolumn.fmt = LVCFMT_LEFT;
		lvcolumn.pszText = listcolumnResult[i];
		lvcolumn.iSubItem = i;
		lvcolumn.cx = widthResult[i];
		m_listReportAlarm.InsertColumn(i,&lvcolumn);
	}
	m_listReportAlarm.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP | LVS_EX_ONECLICKACTIVATE );
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CDlgReportAlarm::AddAlarmHistory(ALARM_EVENT_DATA &alarm, bool bClear, bool bLightAlarm)
{
	CString sHappenedTime, sClearedTime, sModule, sAlarmId, sAlarmCode, sOperMode, sHostMode, sCommand, sString;

	sHappenedTime.Format(_T("%s"), alarm.happendTime);
	sClearedTime.Format(_T("%s"), alarm.clearedTime);
	sModule.Format(_T("%s"), alarm.module);
	sAlarmId.Format(_T("%d"), alarm.id);
	sAlarmCode.Format(_T("%d"), alarm.code);
	sCommand.Format(_T("%s"), alarm.cmd);
	sString.Format(_T("%s"), alarm.text);

 	int nHostMode = df_HOST_OFFLINE;
	nHostMode = m_nHostMode;
	switch (m_nHostMode)
	{
		case df_HOST_OFFLINE:		sHostMode = _T("OFFLINE");		break;
		case df_HOST_ONLINE_REMOTE:	sHostMode = _T("ONLINE-REMOTE");	break;
		case df_HOST_ONLINE_LOCAL:	sHostMode = _T("ONLINE-LOCAL");	break;
		default:				    sHostMode = _T("UNKNOWN");		break;
	}

	// Add List
	int nCount = 0;
	if (bLightAlarm != true)
	{
		if (bClear != true)
		{
			nCount = m_listReportAlarm.GetItemCount();
			alarm.nInsertLine = nCount;	// jsyoon 20210817 add
			m_listReportAlarm.InsertItem(nCount, sHappenedTime);

			m_listReportAlarm.SetItemText(nCount, 2, sModule);
			m_listReportAlarm.SetItemText(nCount, 3, sAlarmCode);
			m_listReportAlarm.SetItemText(nCount, 4, sHostMode);
			m_listReportAlarm.SetItemText(nCount, 5, sCommand);
			m_listReportAlarm.SetItemText(nCount, 6, sString);

			if (nCount > 500)
			{
				m_listReportAlarm.DeleteItem(0);
			}
		}
		else
		{
			nCount = alarm.nInsertLine;

			m_listReportAlarm.SetItemText(nCount, 1, sClearedTime);
		}
	}
	else
	{
		nCount = m_listReportAlarm.GetItemCount();	// jsyoon 2022.08.17 add
		m_listReportAlarm.InsertItem(nCount, sHappenedTime);
		m_listReportAlarm.SetItemText(nCount, 1, sClearedTime);
		m_listReportAlarm.SetItemText(nCount, 2, sModule);
		m_listReportAlarm.SetItemText(nCount, 3, sAlarmCode);
		m_listReportAlarm.SetItemText(nCount, 4, sHostMode);
		m_listReportAlarm.SetItemText(nCount, 5, sCommand);
		m_listReportAlarm.SetItemText(nCount, 6, sString);
		bClear = true;

		if (nCount > 500)
		{
			m_listReportAlarm.DeleteItem(0);
		}
	}

	if (bClear != true)
	{
		return TRUE;
	}
	else
	{

		// Add DB
		CString sSQL;
		sSQL.Format(_T("INSERT INTO AlarmHistory (sHappenedTime, sClearedTime, sModule, nAlarmCode, sHostMode, sCommand, sString) VALUES('%s','%s','%s', %s,'%s','%s','%s')"),
			sHappenedTime.GetString(), sClearedTime.GetString(), sModule.GetString(), sAlarmCode.GetString(), sHostMode.GetString(), sCommand.GetString(), sString.GetString());

		return Queryexe(sSQL);
	}
	
//	// Add List
//	int nCount = m_listReportAlarm.GetItemCount();
//	m_listReportAlarm.InsertItem(nCount, sHappenedTime);
//	m_listReportAlarm.SetItemText(nCount,1,sClearedTime);
//	m_listReportAlarm.SetItemText(nCount,2,sModule);
//	m_listReportAlarm.SetItemText(nCount,3,sAlarmId);
//	m_listReportAlarm.SetItemText(nCount,4,sAlarmCode);
////	m_listReportAlarm.SetItemText(nCount,5,sOperMode);
//	m_listReportAlarm.SetItemText(nCount,5,sHostMode);
//	m_listReportAlarm.SetItemText(nCount,6,sCommand);
//	m_listReportAlarm.SetItemText(nCount,7,sString);
//	
//	if (nCount > 500)	m_listReportAlarm.DeleteItem(0);
//	
//	// Add DB
//	CString sSQL;
//	//sSQL.Format(_T("INSERT INTO AlarmHistory (sHappenedTime, sClearedTime, sModule, nAlarmID, nAlarmCode, sHostMode, sCommand, sString) VALUES('%s','%s','%s', %d, %d,'%s','%s','%s')"),
//	//				sHappenedTime, sClearedTime, sModule, nAlarmId, nAlarmCode, sHostMode, sCommand, sString);
//
//	// jsyoon 20210310 change
//	sSQL.Format(_T("INSERT INTO AlarmHistory (sHappenedTime, sClearedTime, sModule, nAlarmID, nAlarmCode, sHostMode, sCommand, sString) VALUES('%s','%s','%s', %d, %d,'%s','%s','%s')"),
//		sHappenedTime, sClearedTime, sModule, nAlarmId, nAlarmCode, sHostMode, sCommand, sCommand);
//	
//	return Queryexe(sSQL);
}


BOOL CDlgReportAlarm::Queryexe(CString strSQL)
{
	if (m_strRootPath.IsEmpty())
		return FALSE;

	g_csQuery.Lock();		//크리티컬 섹션 동기화 Lock, by Jay Hong : 2009.05.27

	BOOL b = TRUE;
	SYSTEMTIME st;
	::GetLocalTime(&st);
	
	CString sDBFile_Dest;
	CString sDBFile_Sour = g_sAppPath + dfDEFAULT_DB_SOURCE_PATH_NEW;	//원본 파일, 절대로 건드리지 않는다
//	CString sDBFile_Dest = g_sAppPath + dfDEFAULT_DB_DEST_PATH;			//월단위로 DB파일 생성
//	sDBFile_Dest.Format(_T("%s%s"), /*m_psystem->*/m_strRootPath, dfDEFAULT_DB_DEST_PATH );		// jsyoon 20160629 경로변경
	sDBFile_Dest.Format(_T("%s%s"), /*m_psystem->*/m_strRootPath, _T("ALARM\\"));					// jsyoon 20200720 change
	CString sFileName;
	
	//"Report_200905.mdb" 형태로 DB생성
	if (sDBFile_Sour.Find(_T(".accdb")) > 0)
	{
		sFileName.Format(_T("%sReport_%04d%02d.accdb"), sDBFile_Dest, st.wYear, st.wMonth);	// 2007~
	}
	else
	{
		sFileName.Format(_T("%sReport_%04d%02d.mdb"), sDBFile_Dest, st.wYear, st.wMonth); // ~2003
	}
	
	CFileFind finder;
	CString m_sDBFilePath;
	
	BOOL bFind = finder.FindFile(sFileName); 
	if (bFind == FALSE)
	{
		bFind = finder.FindFile(sDBFile_Sour);	// jsyoon 20141231 add
		if( bFind == FALSE)
		{
			g_csQuery.Unlock();		// jsyoon 20150224 add
			return FALSE;
		}

		CString str;		
		if(gf_MakeDir(sDBFile_Dest))
		{
			str.Format(_T("%s : was successfully created"), sDBFile_Dest);
			CopyFile(sDBFile_Sour, sFileName, FALSE);
		}
		else
		{
			str.Format(_T("%s : Problem creating directory"), sDBFile_Dest);
		}
	}

	CString strConnection; 
	strConnection.Format(_T("Driver={Microsoft Access Driver (*.mdb, *.accdb)};dbq=%s"), sFileName.GetString());

	try
	{
		b = m_db.OpenEx(strConnection);
		if (b == TRUE)
		{
			m_db.ExecuteSQL(strSQL.GetString());
			m_db.Close();
		}
	}
	catch (CException* e)
	{
		e->ReportError();
		e->Delete();
	}
	catch (LPCTSTR msg) 
	{ 
		AfxMessageBox(msg); 
	}

	//CDatabase db;
	//if (db.OpenEx(strConnection) == FALSE)
	//{
	//	b = FALSE;
	//}

	//db.ExecuteSQL(strSQL.GetString());
	//db.Close();

	//CDaoDatabase	db;
	//if(!db.IsOpen()) 
	//{
	//	db.Open(sFileName);
	//	
	//	if(!db.IsOpen()) 
	//	{
	//		AfxMessageBox(_T("Report Database Open Failed"));
	//		b = FALSE;
	//	}
	//}
	//else
	//{
	//	m_sDBFilePath = sFileName;
	//	b = TRUE;
	//}

	//TRY
	//{
	//	db.Execute(strSQL, dbFailOnError|dbSeeChanges);
	//}
	//CATCH(CDaoException, e)
	//{
	//	TCHAR szCause[255];
	//	
	//	e->GetErrorMessage(szCause, 255);
	//	AfxMessageBox(szCause, MB_ICONERROR);
	//	
	//	e->Delete();
	//	
	//	b = FALSE;
	//}
	//END_CATCH

	//db.Close();
	//AfxDaoTerm();
		
	g_csQuery.Unlock();

	return b;
}


void CDlgReportAlarm::OnBnClickedButtonAlarmReportClear()
{
	m_listReportAlarm.DeleteAllItems();
}

void CDlgReportAlarm::OnBnClickedButtonAlarmReportSearch()
{
	CDlgReportAlarmSearch dlg;

	//dlg.m_strPath.Format(_T("%s"), /*m_psystem->*/m_strRootPath);
	dlg.SetSavePath(m_strRootPath);
	dlg.DoModal();
}

//lhj 20181225 lang Report Alarm
int CDlgReportAlarm::UpdateLanguage(HINSTANCE hResource_Lang_Dll)
{
	TCHAR strText[MAX_PATH]={0,};
	TCHAR strText100[MAX_PATH]={0,};
	TCHAR strText101[MAX_PATH]={0,};
	TCHAR strText102[MAX_PATH]={0,};
	TCHAR strText103[MAX_PATH]={0,};
	TCHAR strText104[MAX_PATH]={0,};
	TCHAR strText105[MAX_PATH]={0,};

	if(hResource_Lang_Dll == NULL)
		return 0;

//	LoadString(hResource_Lang_Dll,IDS_STRING302_CLEAR, strText,MAX_PATH);
//	GetDlgItem(IDC_CTPUSHCTRL_ALARM_REPORT_CLEAR)->SetWindowText((CString)strText);	

// 	//LoadString(hResource_Lang_Dll,IDS_STRING134_RECIPE, strText100,MAX_PATH);
// 	//LoadString(hResource_Lang_Dll,IDS_STRING106_DATE, strText101,MAX_PATH);
// 	LoadString(hResource_Lang_Dll,IDS_STRING145_HAPPENED_TIME, strText100,MAX_PATH);
// 	LoadString(hResource_Lang_Dll,IDS_STRING146_CLEARED_TIME, strText101,MAX_PATH);
// 	LoadString(hResource_Lang_Dll,IDS_STRING141_MODULE, strText102,MAX_PATH);
// 	LoadString(hResource_Lang_Dll,IDS_STRING147_COMMAND, strText103,MAX_PATH);
// 	LoadString(hResource_Lang_Dll,IDS_STRING148_STRING, strText104,MAX_PATH);
// 	LoadString(hResource_Lang_Dll,IDS_STRING104_ALARM, strText,MAX_PATH);

	CString strAlarmCode;
	strAlarmCode.Format(_T("%s Code"),strText);
	_tcscpy_s(strText, _countof(strText), strAlarmCode);
//	sprintf(strText,strAlarmCode);

	LV_COLUMN lvcolumn;
	TCHAR* listcolumnResult[9] = {strText100, strText101,strText102,strText,_T("Host Mode"),strText103,strText104};
	int widthResult[9] = {160,160,80,80,110,200,500};
	for(int i=0;i<9;i++)
	{
//		TCHAR name[40];
		LVCOLUMN col;

		col.mask=LVCF_TEXT;
		col.pszText=listcolumnResult[i];
		col.cchTextMax=40;

		// Set column name
		m_listReportAlarm.SetColumn(i, &col);
	}

	return 0;
}