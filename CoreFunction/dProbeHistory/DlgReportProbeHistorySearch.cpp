// DlgReportProbeHistorySearch.cpp : implementation file
//

#include "pch.h"
#include "ProbeHistory.h"
#include "DlgReportProbeHistorySearch.h"

//#include "CommonFuncProbe.h"
///#include "../../Include/CommonFunc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgReportProbeHistorySearch dialog


CDlgReportProbeHistorySearch::CDlgReportProbeHistorySearch(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgReportProbeHistorySearch::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgReportProbeHistorySearch)
	//}}AFX_DATA_INIT

	m_strPath.Empty();
}


void CDlgReportProbeHistorySearch::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgReportProbeHistorySearch)
	DDX_Control(pDX, IDC_BUTTON_SEARCH_ProbeHistory, m_btnSearchProbeHistory);
	DDX_Control(pDX, IDC_LIST_REPORT_ProbeHistory_SERCH, m_listReportProbeHistory);
//	DDX_Control(pDX, IDC_CTDATECTRL_FROM, m_ctDateFrom);
//	DDX_Control(pDX, IDC_CTDATECTRL_TO, m_ctDateTo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgReportProbeHistorySearch, CDialog)
	//{{AFX_MSG_MAP(CDlgReportProbeHistorySearch)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH_ProbeHistory, OnButtonSearchProbeHistory)
	ON_BN_CLICKED(IDC_BTN_ProbeHistory_SEARCH_SAVEAS_CSV, OnBtnProbeHistorySearchSaveasCsv)
	//}}AFX_MSG_MAP
	ON_NOTIFY(MCN_SELCHANGE, IDC_MONTHCALENDAR_FROM, &CDlgReportProbeHistorySearch::OnMcnSelchangeMonthcalendarFrom)
	ON_NOTIFY(MCN_SELCHANGE, IDC_MONTHCALENDAR_TO, &CDlgReportProbeHistorySearch::OnMcnSelchangeMonthcalendarTo)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgReportProbeHistorySearch message handlers

BOOL CDlgReportProbeHistorySearch::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_listReportProbeHistory.SetBkColor(RGB(255,255,220));
	m_listReportProbeHistory.SetTextBkColor(RGB(255,255,220));
	LV_COLUMN lvcolumn;

	TCHAR* listcolumnResult[6] = {_T("Probe 장착 시간"),_T("Probe 탈착 시간"),_T("Probe RFID"),_T("Probe Contact Count")};
//	char* listcolumnResult[6] = { "Probe 장착 시간","Probe 탈착 시간","Probe RFID","Probe Contact Count" };
//	TCHAR* listcolumnResult[6] = {"Probe Mount Time","Probe UnMount Time","Probe RFID","Probe Contact Count"};
    int widthResult[6] = {160,160,160,160};
	for(int i=0;i<6;i++)
	{
		lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvcolumn.fmt = LVCFMT_LEFT;
		lvcolumn.pszText = listcolumnResult[i];
		lvcolumn.iSubItem = i;
		lvcolumn.cx = widthResult[i];
		m_listReportProbeHistory.InsertColumn(i,&lvcolumn);
	}

	//lhj del 160621
// 	char* listcolumnResult[9] = {"Happened Time","Cleared Time","Module","ProbeHistory Code","Host Mode","Command","String"};
//     int widthResult[9] = {160,160,80,80,110,100,500};
// 	for(int i=0;i<9;i++)
// 	{
// 		lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
// 		lvcolumn.fmt = LVCFMT_LEFT;
// 		lvcolumn.pszText = listcolumnResult[i];
// 		lvcolumn.iSubItem = i;
// 		lvcolumn.cx = widthResult[i];
// 		m_listReportProbeHistory.InsertColumn(i,&lvcolumn);
// 	}

	m_listReportProbeHistory.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP | LVS_EX_ONECLICKACTIVATE );
	m_listReportProbeHistory.DeleteAllItems();
	

	// 오늘 날짜로 초기화 해 둔다.
	GetLocalTime(&m_stFrom);
	GetLocalTime(&m_stTo);
	SetDateText(m_stFrom, IDC_EDIT_DATE_FROM);
	SetDateText(m_stTo, IDC_EDIT_DATE_TO);

	/*m_ctDateFrom.Today();
	m_ctDateTo.Today();

	m_stFrom.wYear = m_ctDateFrom.GetYear();
	m_stFrom.wMonth = m_ctDateFrom.GetMonth();
	m_stFrom.wDay = m_ctDateFrom.GetDay();

	m_stTo.wYear = m_ctDateTo.GetYear();
	m_stTo.wMonth = m_ctDateTo.GetMonth();
	m_stTo.wDay = m_ctDateTo.GetDay();*/
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// void CDlgReportProbeHistorySearch::SearchProbeHistoryHistory()
// {
// 	SYSTEMTIME stCur;
// 	GetLocalTime(&stCur);
// 
// 	CString sFrom,sTo;
// 	sFrom.Format("%04d.%02d.%02d-00:00:00:000",m_stFrom.wYear ,m_stFrom.wMonth,m_stFrom.wDay );
// 	if (m_stTo.wYear == stCur.wYear && m_stTo.wMonth == stCur.wMonth && m_stTo.wDay == stCur.wDay )
// 	{
// 		sTo.Format("%04d.%02d.%02d-%02d:%02d:%02d:%03d",stCur.wYear ,stCur.wMonth,stCur.wDay,stCur.wHour ,stCur.wMinute ,stCur.wSecond, stCur.wMilliseconds );
// 	}
// 	else
// 	{
// 		sTo.Format("%04d.%02d.%02d-23:59:59:000",m_stTo.wYear ,m_stTo.wMonth,m_stTo.wDay );
// 	}
// 
// 	if (sFrom > sTo)
// 	{
// 		AfxMessageBox("Incorrect period (From ~ To)");
// 		return;
// 	}
// 
// 	int nFromYear  = m_stFrom.wYear;
// 	int	nFromMonth = m_stFrom.wMonth;
// 	int nToYear    = m_stTo.wYear ;
// 	int nToMonth   = m_stTo.wMonth ;
// 
// 	CString sYearMonth[50];
// 	CString sTem;
// 	int nCnt=0;
// 
// 	int i,j;
// 	int nMon1;
// 	int nMon2;
// 	for (i=nFromYear; i<=nToYear; i++)
// 	{
// 		if (nFromYear == nToYear) 
// 		{
// 			nMon1 = nFromMonth;
// 			nMon2 = nToMonth;
// 		}
// 		else if (i==nFromYear)
// 		{	
// 			nMon1 = nFromMonth;
// 			nMon2 = 12;
// 		}
// 		else if (i==nToYear)
// 		{
// 			nMon1 = 1;
// 			nMon2 = nToMonth;
// 		}
// 		else
// 		{
// 			nMon1 = 1;
// 			nMon2 = 12;
// 		}
// 
// 		for (j=nMon1; j<=nMon2; j++)
// 		{
// 			sTem.Format("\\Report_%04d%02d.mdb",i,j);
// 			sYearMonth[nCnt++] = sTem;
// 			if (nCnt >= 50)
// 			{
// 				AfxMessageBox("You can't search (so many data))");
// 				return;
// 			}
// 		}
// 	}
// 
// 	CString Module, ProbeHistoryId, Command, Sql_Statement;
// 	GetDlgItemText(IDC_EDIT_ProbeHistory_SEARCH_MODULE, Module);
// 	GetDlgItemText(IDC_EDIT_ProbeHistory_SEARCH_ID, ProbeHistoryId);
// 	GetDlgItemText(IDC_EDIT_ProbeHistory_SEARCH_CMD, Command);
// 
// 	Sql_Statement.Format("SELECT * FROM ProbeHistoryHistory WHERE ((sHappenedTime >= '%s' AND sHappenedTime <= '%s') OR (sClearedTime >= '%s' AND sClearedTime <= '%s'))",sFrom,sTo,sFrom,sTo);
// 	if (Module != "") 
// 	{
// 		Sql_Statement += " AND ";
// 		sTem.Format("sModule ='%s'",Module);
// 		Sql_Statement += sTem;
// 	}
// 	if (ProbeHistoryId != "")
// 	{
// 		Sql_Statement += " AND ";
// 		sTem.Format("nProbeHistoryId =%d",atoi(ProbeHistoryId));
// 		Sql_Statement += sTem;
// 	}
// 	if (Command != "")
// 	{
// 		Sql_Statement += " AND ";
// 		sTem.Format("sCommand ='%s'",Command);
// 		Sql_Statement += sTem;
// 	}
// 
// //	CString sDBFile_Dest = g_sAppPath + dfDEFAULT_DB_DEST_PATH;
// 	CString sDBFile_Dest, sFileName;
// 	sDBFile_Dest.Format("%s%s", m_strPath, dfDEFAULT_DB_DEST_PATH );		// jsyoon 20160629 경로변경
// 
// 	BeginWaitCursor();
// 	m_listReportProbeHistory.DeleteAllItems();
// 	SetDlgItemText(IDC_CTTEXTCTRL_ProbeHistory_COUNT, "");
// 
// 	m_nProbeHistoryCount = 0;
// 	m_bStop = FALSE;
// 
// 
// 	g_csQuery.Lock();
// 	for (i=0; i<nCnt; i++)
// 	{
// 		sFileName = sDBFile_Dest + sYearMonth[i];
// 		CFileFind finder;
// 		if (finder.FindFile(sFileName) == FALSE)
// 		{
// 			continue;
// 		}
// 		
// 		GetProbeHistoryInfo(sFileName,Sql_Statement);
// 		if (m_bStop) break;
// 	}
// 	g_csQuery.Unlock();
// 
// 	CString sCount;
// 	sCount.Format("%d",m_nProbeHistoryCount);
// 	GetDlgItem(IDC_CTTEXTCTRL_ProbeHistory_COUNT)->SetWindowText(sCount);
// 
// 	EndWaitCursor();
// }

void CDlgReportProbeHistorySearch::SearchProbeHistory()
{
	SYSTEMTIME stCur;
	GetLocalTime(&stCur);

	CString sFrom,sTo;
	sFrom.Format(_T("%04d.%02d.%02d-00:00:00:000"),m_stFrom.wYear ,m_stFrom.wMonth,m_stFrom.wDay );
	if (m_stTo.wYear == stCur.wYear && m_stTo.wMonth == stCur.wMonth && m_stTo.wDay == stCur.wDay )
	{
		sTo.Format(_T("%04d.%02d.%02d-%02d:%02d:%02d:%03d"),stCur.wYear ,stCur.wMonth,stCur.wDay,stCur.wHour ,stCur.wMinute ,stCur.wSecond, stCur.wMilliseconds );
	}
	else
	{
		sTo.Format(_T("%04d.%02d.%02d-23:59:59:000"),m_stTo.wYear ,m_stTo.wMonth,m_stTo.wDay );
	}

	if (sFrom > sTo)
	{
		AfxMessageBox(_T("Incorrect period (From ~ To)"));
		return;
	}

	int nFromYear  = m_stFrom.wYear;
	int	nFromMonth = m_stFrom.wMonth;
	int nToYear    = m_stTo.wYear ;
	int nToMonth   = m_stTo.wMonth ;

	CString sYearMonth[50];
	CString sTem;
	int nCnt=0;

	int i,j;
	int nMon1;
	int nMon2;
	for (i=nFromYear; i<=nToYear; i++)
	{
		if (nFromYear == nToYear) 
		{
			nMon1 = nFromMonth;
			nMon2 = nToMonth;
		}
		else if (i==nFromYear)
		{	
			nMon1 = nFromMonth;
			nMon2 = 12;
		}
		else if (i==nToYear)
		{
			nMon1 = 1;
			nMon2 = nToMonth;
		}
		else
		{
			nMon1 = 1;
			nMon2 = 12;
		}

		for (j=nMon1; j<=nMon2; j++)
		{
			sTem.Format(_T("ProbeReport_%04d.%02d.mdb"),i,j);
			sYearMonth[nCnt++] = sTem;
			if (nCnt >= 50)
			{
				AfxMessageBox(_T("You can't search (so many data))"));
				return;
			}
		}
	}

	CString Module, ProbeHistoryId, Command, Sql_Statement;
	GetDlgItemText(IDC_EDIT_ProbeHistory_SEARCH_MODULE, Module);
	GetDlgItemText(IDC_EDIT_ProbeHistory_SEARCH_ID, ProbeHistoryId);
	GetDlgItemText(IDC_EDIT_ProbeHistory_SEARCH_CMD, Command);

	Sql_Statement.Format(_T("SELECT * FROM ProbeHistory WHERE ((sHappenedTime >= '%s' AND sHappenedTime <= '%s') OR (sClearedTime >= '%s' AND sClearedTime <= '%s'))"),sFrom,sTo,sFrom,sTo);
	if (Module != "") 
	{
		Sql_Statement += _T(" AND ");
		sTem.Format(_T("sModule ='%s'"),Module);
		Sql_Statement += sTem;
	}
	if (ProbeHistoryId != "")
	{
		Sql_Statement += _T(" AND ");
		sTem.Format(_T("nProbeHistoryId =%d"), _ttoi(ProbeHistoryId));
		Sql_Statement += sTem;
	}
	if (Command != "")
	{
		Sql_Statement += _T(" AND ");
		sTem.Format(_T("sCommand ='%s'"),Command);
		Sql_Statement += sTem;
	}

//	CString sDBFile_Dest = g_sAppPath + dfDEFAULT_DB_DEST_PATH;
	CString sDBFile_Dest, sFileName;
	sDBFile_Dest.Format(_T("%s%s"), m_strPath, dfDEFAULT_DB_DEST_PATH );		// jsyoon 20160629 경로변경

	BeginWaitCursor();
	m_listReportProbeHistory.DeleteAllItems();
//	SetDlgItemText(IDC_CTTEXTCTRL_ProbeHistory_COUNT, _T(""));

	m_nProbeHistoryCount = 0;
	m_bStop = FALSE;


	g_csQuery.Lock();
	for (i=0; i<nCnt; i++)
	{
		sFileName = sDBFile_Dest + sYearMonth[i];
		CFileFind finder;
		if (finder.FindFile(sFileName) == FALSE)
		{
			continue;
		}
		
		GetProbeHistoryInfo2(sFileName,Sql_Statement);
		if (m_bStop) break;
	}
	g_csQuery.Unlock();

	//CString sCount;
	//sCount.Format(_T("%d"),m_nProbeHistoryCount);
	//GetDlgItem(IDC_CTTEXTCTRL_ProbeHistory_COUNT)->SetWindowText(sCount);

	EndWaitCursor();
}

// BOOL CDlgReportProbeHistorySearch::GetProbeHistoryInfo(CString sFilePath, CString sSQL)
// {
// 	AfxDaoInit();
// 
// 	COleVariant var;
// 	CString		sHappenedTime,sClearedTime,sModule,sProbeHistoryId,sProbeHistoryCode,sOperationMode,sHostMode,sCommand,sString;
// 	CString		sListData;
// 
// 	int nCount = 0, iTiem = 0;
// 	CDaoDatabase	db;
// 	db.Open(_T(sFilePath));
// 	if(db.IsOpen()) 
// 	{
// 		CDaoRecordset	rs(&db);
// 		rs.Open(dbOpenDynaset, sSQL);
// 		if(rs.IsOpen())
// 		{
// 			nCount = rs.GetRecordCount();
// 			if( nCount > 0)
// 			{
// 				rs.MoveFirst();
// 
// 				while(!rs.IsEOF()) 
// 				{
// 					rs.GetFieldValue(0,var);
// 					sHappenedTime.Format("%s", CString(var.pbVal));
// 
// 					rs.GetFieldValue(1,var);
// 					sClearedTime.Format("%s", CString(var.pbVal));
// 
// 					rs.GetFieldValue(2,var);
// 					sModule.Format("%s", CString(var.pbVal));
// 
// 		//			rs.GetFieldValue(3,var);
// 		//			sProbeHistoryId.Format("%d", int(var.intVal));
// 
// 					rs.GetFieldValue(3,var);
// 					sProbeHistoryCode.Format("%d", int(var.intVal));
// 
// 		//			rs.GetFieldValue(5,var);
// 		//			sOperationMode.Format("%s", CString(var.pbVal));
// 
// 					rs.GetFieldValue(4,var);
// 					sHostMode.Format("%s", CString(var.pbVal));
// 
// 					rs.GetFieldValue(5,var);
// 					sCommand.Format("%s", CString(var.pbVal));
// 
// 					rs.GetFieldValue(6,var);
// 					sString.Format("%s", CString(var.pbVal));
// 
// 					iTiem = m_listReportProbeHistory.GetItemCount();
// 					m_listReportProbeHistory.InsertItem(iTiem,sHappenedTime);
// 					m_listReportProbeHistory.SetItemText(iTiem,1,sClearedTime);
// 					m_listReportProbeHistory.SetItemText(iTiem,2,sModule);
// 					m_listReportProbeHistory.SetItemText(iTiem,3,sProbeHistoryCode);
// 					m_listReportProbeHistory.SetItemText(iTiem,4,sHostMode);
// 					m_listReportProbeHistory.SetItemText(iTiem,5,sCommand);
// 					m_listReportProbeHistory.SetItemText(iTiem,6,sString);
// 
// 					m_nProbeHistoryCount++;
// 					if (m_nProbeHistoryCount == 10000)
// 					{
// 						if (MessageBox("It was collection of 10000 data from database. Do you wish to continue?","Search",MB_ICONQUESTION|MB_YESNO) == IDNO)
// 						{
// 							m_bStop = TRUE;
// 							break;
// 						}
// 					}
// 
// 					rs.MoveNext();
// 				}
// 			}
// 			rs.Close();
// 			db.Close();
// 		}
// 		else
// 		{
// 			db.Close();
// 		}
// 	}
// 
// 	AfxDaoTerm();
// 
// 	if( nCount > 0)
// 		return TRUE;
// 	else
// 		return FALSE;
// }

BOOL CDlgReportProbeHistorySearch::GetProbeHistoryInfo2(CString sFilePath, CString sSQL)
{
//	AfxDaoInit();		// jsyoon 20170124 remove
	
	COleVariant var;
	CString		sHappenedTime,sClearedTime,sModule,sProbeHistoryId,sProbeHistoryCode,sOperationMode,sHostMode,sCommand,sString;
	CString		sListData;
	
	int nCount = 0, iTiem = 0;


	CString strConnection;
	strConnection.Format(_T("Driver={Microsoft Access Driver (*.mdb, *.accdb)};dbq=%s"), sFilePath.GetString());

	BOOL bRet = TRUE;
	CDatabase	db;
	if (db.OpenEx(strConnection) == TRUE)
	{
		CRecordset	rs(&db);
		if (rs.Open(CRecordset::dynaset, sSQL) == TRUE)
		{
			nCount = rs.GetRecordCount();
			if (nCount > 0)
			{
				rs.MoveFirst();

				while (rs.IsEOF() == FALSE)
				{
					rs.GetFieldValue((short)0, sHappenedTime);
					rs.GetFieldValue((short)1, sClearedTime);
					rs.GetFieldValue((short)2, sModule);
					rs.GetFieldValue((short)3, sProbeHistoryCode);

					int iTiem = m_listReportProbeHistory.GetItemCount();
					m_listReportProbeHistory.InsertItem(iTiem, sHappenedTime);
					m_listReportProbeHistory.SetItemText(iTiem, 1, sClearedTime);
					m_listReportProbeHistory.SetItemText(iTiem, 2, sModule);
					m_listReportProbeHistory.SetItemText(iTiem, 3, sProbeHistoryCode);
					rs.MoveNext();
				}
			}
			rs.Close();
			db.Close();
		}
		else
		{
			db.Close();
			bRet = FALSE;
		}
	}
	else
	{
		bRet = FALSE;
	}

	return bRet;

//	CDaoDatabase	db;
//
////	db.Open(_T(sFilePath));
//	if(!db.IsOpen()) 
//	{
//		TRY
//		{
//			db.Open((LPCTSTR)_T(sFilePath), FALSE, FALSE);
//		}
//		CATCH(CDaoException, e)
//		{
//			TCHAR szCause[255];
//			e->GetErrorMessage(szCause, 255);
//			AfxMessageBox(szCause, MB_ICONERROR);
//
//			e->Delete();
//			db.Close();
//		}
//		END_CATCH
//	}
//
//	if(db.IsOpen()) 
//	{
//		CDaoRecordset	rs(&db);
//		rs.Open(dbOpenDynaset, sSQL);
//		if(rs.IsOpen())
//		{
//			nCount = rs.GetRecordCount();
//			if(nCount>0)
//			{
//				rs.MoveFirst();
//
//				while(!rs.IsEOF()) 
//				{
//					rs.GetFieldValue(0,var);
//					sHappenedTime.Format("%s", CString(var.pbVal));
//
//					rs.GetFieldValue(1,var);
//					sClearedTime.Format("%s", CString(var.pbVal));
//
//					rs.GetFieldValue(2,var);
//					sModule.Format("%s", CString(var.pbVal));
//
//					rs.GetFieldValue(3,var);
//					sProbeHistoryCode.Format("%d", int(var.intVal));
//
//					iTiem = m_listReportProbeHistory.GetItemCount();
//					m_listReportProbeHistory.InsertItem(iTiem,sHappenedTime);
//					m_listReportProbeHistory.SetItemText(iTiem,1,sClearedTime);
//					m_listReportProbeHistory.SetItemText(iTiem,2,sModule);
//					m_listReportProbeHistory.SetItemText(iTiem,3,sProbeHistoryCode);
//
//					m_nProbeHistoryCount++;
//					if (m_nProbeHistoryCount == 10000)
//					{
//						if (MessageBox("It was collection of 10000 data from database. Do you wish to continue?","Search",MB_ICONQUESTION|MB_YESNO) == IDNO)
//						{
//							m_bStop = TRUE;
//							break;
//						}
//					}
//
//					rs.MoveNext();
//				}
//			}
//			rs.Close();
//			db.Close();
//		}
//		else
//		{
//			db.Close();
//		}
//	}
//		
//	AfxDaoTerm();
//	
//	if( nCount > 0)
//		return TRUE;
//	else
//		return FALSE;
}

void CDlgReportProbeHistorySearch::OnButtonSearchProbeHistory() 
{
	m_btnSearchProbeHistory.EnableWindow(FALSE);
	//SearchProbeHistoryHistory();
	SearchProbeHistory();
	m_btnSearchProbeHistory.EnableWindow(TRUE);
}

//BEGIN_EVENTSINK_MAP(CDlgReportProbeHistorySearch, CDialog)
//    //{{AFX_EVENTSINK_MAP(CDlgReportProbeHistorySearch)
//	ON_EVENT(CDlgReportProbeHistorySearch, IDC_CTDATECTRL_FROM, 3 /* DateClick */, OnDateClickCtdatectrlFrom, VTS_I2 VTS_I2 VTS_I2 VTS_I2)
//	ON_EVENT(CDlgReportProbeHistorySearch, IDC_CTDATECTRL_TO, 3 /* DateClick */, OnDateClickCtdatectrlTo, VTS_I2 VTS_I2 VTS_I2 VTS_I2)
//	ON_EVENT(CDlgReportProbeHistorySearch, IDC_CTDATECTRL_TO, 7 /* LoadDayInfo */, OnLoadDayInfoCtdatectrlTo, VTS_I2 VTS_I2 VTS_I2)
//	ON_EVENT(CDlgReportProbeHistorySearch, IDC_CTDATECTRL_FROM, 7 /* LoadDayInfo */, OnLoadDayInfoCtdatectrlFrom, VTS_I2 VTS_I2 VTS_I2)
//	//}}AFX_EVENTSINK_MAP
//END_EVENTSINK_MAP()
//
//void CDlgReportProbeHistorySearch::OnDateClickCtdatectrlFrom(short nDOW, short nDay, short nMonth, short nYear) 
//{
//	m_stFrom.wYear = nYear;
//	m_stFrom.wMonth = nMonth;
//	m_stFrom.wDay = nDay;
//
//	SetDateText (nDOW, nDay, nMonth, nYear, IDC_CTTEXT_FROM);
//}
//
//void CDlgReportProbeHistorySearch::OnDateClickCtdatectrlTo(short nDOW, short nDay, short nMonth, short nYear) 
//{
//	m_stTo.wYear = nYear;
//	m_stTo.wMonth = nMonth;
//	m_stTo.wDay = nDay;	
//
//	SetDateText (nDOW, nDay, nMonth, nYear, IDC_CTTEXT_TO);
//}
//
//void CDlgReportProbeHistorySearch::OnLoadDayInfoCtdatectrlTo(short nMonth, short nYear, short nSection) 
//{
//	CString strMonthName;
// //   int nCntr;
//	nYear;
//	
//    if (nMonth == 1)
//        m_ctDateTo.SetDateBackColor(1, nSection, RGB(0, 128, 0));
//    else if (nMonth == 2)
//        m_ctDateTo.SetDateBackColor(14, nSection, RGB(0, 128, 128));
//    else if (nMonth == 3)
//	{
//		//         m_ctDateToday.SetDateFont(1, nSection, 2);
//		//         m_ctDateToday.SetDateFont(5, nSection, 2);
//		//         m_ctDateToday.SetDateFont(6, nSection, 2);
//		//         m_ctDateToday.SetDateFont(7, nSection, 2);
//        m_ctDateTo.SetDateForeColor(1, nSection, RGB(255, 0, 0));
//		//         m_ctDateToday.SetDateForeColor(5, nSection, RGB(255, 0, 0));
//		//         m_ctDateToday.SetDateForeColor(6, nSection, RGB(255, 0, 0));
//		//         m_ctDateToday.SetDateForeColor(7, nSection, RGB(255, 0, 0));
//	}
//    else if (nMonth == 4)
//	{
//        m_ctDateTo.SetDateBackColor(19, nSection, RGB(0, 128, 0));
//        m_ctDateTo.SetDateFont(20, nSection, 1);
//        m_ctDateTo.SetDateFont(21, nSection, 1);
//	}
//    else if (nMonth == 5)
//	{
//        m_ctDateTo.SetDateBackColor(8, nSection, RGB(255, 0, 128));
//        m_ctDateTo.SetDateFont(8, nSection, 1);
//	}
//	else if (nMonth == 6)
//	{
//        m_ctDateTo.SetDateBackColor(28, nSection, RGB(0, 128, 0));
//        m_ctDateTo.SetDateFont(8, nSection, 1);
//	}
//    else if (nMonth == 7)
//	{
//		//         m_ctDateToday.SetDateBackColor(1, nSection, RGB(0, 128, 0));
//		//         m_ctDateToday.SetDateBackColor(4, nSection, RGB(0, 128, 0));
//	}
//    else if (nMonth == 9) 
//	{
//		//         m_ctDateToday.SetDateBackColor(9, nSection, RGB(0, 128, 0));
//		//         m_ctDateToday.SetDateFont(16, nSection, 1);
//		//         m_ctDateToday.SetDateFont(17, nSection, 1);
//		//         m_ctDateToday.SetDateFont(18, nSection, 1);
//	}
//    else if (nMonth == 10)
//	{
//		//         for (nCntr = 1; nCntr<= 31; nCntr++)
//		//             m_ctDateToday.SetDateBackColor(nCntr, nSection, RGB(128 + (nCntr * 4), 128 + (nCntr * 4), 0));  
//	}
//    else if (nMonth == 11)
//	{
//        m_ctDateTo.SetDateBackColor(1, nSection,RGB(0, 128, 0));
//		//       m_ctDateToday.SetDateFont(15, nSection, 1);
//	}
//    else if (nMonth == 12)
//	{
//        m_ctDateTo.SetDateBackColor(24, nSection, RGB(128, 128, 0));
//        m_ctDateTo.SetDateBackColor(25, nSection, RGB(0, 128, 0));
//        m_ctDateTo.SetDateBackColor(26, nSection, RGB(128, 128, 0));
//    }
//}
//
//void CDlgReportProbeHistorySearch::OnLoadDayInfoCtdatectrlFrom(short nMonth, short nYear, short nSection) 
//{
//	
//	CString strMonthName;
//	nYear;
////    int nCntr;
//	
//    if (nMonth == 1)
//        m_ctDateFrom.SetDateBackColor(1, nSection, RGB(0, 128, 0));
//    else if (nMonth == 2)
//        m_ctDateFrom.SetDateBackColor(14, nSection, RGB(0, 128, 128));
//    else if (nMonth == 3)
//	{
//		//         m_ctDateToday.SetDateFont(1, nSection, 2);
//		//         m_ctDateToday.SetDateFont(5, nSection, 2);
//		//         m_ctDateToday.SetDateFont(6, nSection, 2);
//		//         m_ctDateToday.SetDateFont(7, nSection, 2);
//        m_ctDateFrom.SetDateForeColor(1, nSection, RGB(255, 0, 0));
//		//         m_ctDateToday.SetDateForeColor(5, nSection, RGB(255, 0, 0));
//		//         m_ctDateToday.SetDateForeColor(6, nSection, RGB(255, 0, 0));
//		//         m_ctDateToday.SetDateForeColor(7, nSection, RGB(255, 0, 0));
//	}
//    else if (nMonth == 4)
//	{
//        m_ctDateFrom.SetDateBackColor(19, nSection, RGB(0, 128, 0));
//        m_ctDateFrom.SetDateFont(20, nSection, 1);
//        m_ctDateFrom.SetDateFont(21, nSection, 1);
//	}
//    else if (nMonth == 5)
//	{
//        m_ctDateFrom.SetDateBackColor(8, nSection, RGB(255, 0, 128));
//        m_ctDateFrom.SetDateFont(8, nSection, 1);
//	}
//	else if (nMonth == 6)
//	{
//        m_ctDateFrom.SetDateBackColor(28, nSection, RGB(0, 128, 0));
//        m_ctDateFrom.SetDateFont(8, nSection, 1);
//	}
//    else if (nMonth == 7)
//	{
//		//         m_ctDateToday.SetDateBackColor(1, nSection, RGB(0, 128, 0));
//		//         m_ctDateToday.SetDateBackColor(4, nSection, RGB(0, 128, 0));
//	}
//    else if (nMonth == 9) 
//	{
//		//         m_ctDateToday.SetDateBackColor(9, nSection, RGB(0, 128, 0));
//		//         m_ctDateToday.SetDateFont(16, nSection, 1);
//		//         m_ctDateToday.SetDateFont(17, nSection, 1);
//		//         m_ctDateToday.SetDateFont(18, nSection, 1);
//	}
//    else if (nMonth == 10)
//	{
//		//         for (nCntr = 1; nCntr<= 31; nCntr++)
//		//             m_ctDateToday.SetDateBackColor(nCntr, nSection, RGB(128 + (nCntr * 4), 128 + (nCntr * 4), 0));  
//	}
//    else if (nMonth == 11)
//	{
//        m_ctDateFrom.SetDateBackColor(1, nSection,RGB(0, 128, 0));
//		//       m_ctDateToday.SetDateFont(15, nSection, 1);
//	}
//    else if (nMonth == 12)
//	{
//        m_ctDateFrom.SetDateBackColor(24, nSection, RGB(128, 128, 0));
//        m_ctDateFrom.SetDateBackColor(25, nSection, RGB(0, 128, 0));
//        m_ctDateFrom.SetDateBackColor(26, nSection, RGB(128, 128, 0));
//    }
//	
//}
//
//void CDlgReportProbeHistorySearch::SetDateText(short nDOW, short nDay, short nMonth, short nYear, int iD)
//{
//	CString strMonthName;
//    
//    switch(nMonth)
//	{
//	case 1:
//        strMonthName = _T("Jan.");
//		break;
//	case 2:
//		strMonthName = _T("Feb.");
//		break;
//	case 3:
//        strMonthName = _T("Mar.");
//		break;
//    case 4:
//        strMonthName = _T("Apr.");
//		break;
//	case 5:
//        strMonthName = _T("May");
//		break;
//	case 6:
//        strMonthName = _T("June");
//		break;
//	case 7:
//        strMonthName = _T("July");
//		break;
//	case 8:
//        strMonthName = _T("Aug.");
//		break;
//	case 9:
//        strMonthName = _T("Sep.");
//		break;
//	case 10:
//        strMonthName = _T("Oct.");
//		break;
//	case 11:
//        strMonthName = _T("Nov.");
//		break;
//	case 12:
//        strMonthName = _T("Dec.");
//		break;
//	default:
//        strMonthName = _T("???");
//		break;
//	}
//	
//    char strTemp[16];
//	strMonthName = strMonthName + " " + itoa(nDay, (char*)strTemp, 10) + _T(" / ");
//	strMonthName += itoa(nYear, (char*)strTemp, 10);
//	
//    switch(nDOW)
//	{
//	case 1:
//        SetDlgItemText(iD, _T("Sunday ") + strMonthName);
//		break;
//	case 2:
//        SetDlgItemText(iD, _T("Monday ") + strMonthName);
//		break;
//	case 3:
//        SetDlgItemText(iD, _T("Tuesday ") + strMonthName);
//		break;
//	case 4:
//        SetDlgItemText(iD, _T("Wednesday ") + strMonthName);
//		break;
//	case 5:
//        SetDlgItemText(iD, _T("Thursday ") + strMonthName);
//		break;
//	case 6:
//        SetDlgItemText(iD, _T("Friday ") + strMonthName);
//		break;
//	case 7:
//        SetDlgItemText(iD, _T("Saturday ") + strMonthName);
//		break;
//	}
//}

void CDlgReportProbeHistorySearch::OnBtnProbeHistorySearchSaveasCsv() 
{
	CWinApp* pApp = ::AfxGetApp(); 
	
	TCHAR szBuff[_MAX_PATH];
	::GetModuleFileName(pApp->m_hInstance, szBuff, _MAX_PATH);
	
	CString strExePath;
	strExePath.Format(_T("%s%s"), m_strPath, dfDEFAULT_PROBE_FILE_PATH);
	gf_MakeDir(strExePath);
	
	CString filter   = "csv Files (*.csv)|*.csv|";
	
	CString filename;
	filename.Format(_T("%s%s"), strExePath,  _T("Probelist.csv"));
//	CFileDialog dlg(FALSE, "", filename, OFN_FILEMUSTEXIST, filter);
	CFileDialog dlg(FALSE, _T("csv"), filename, OFN_FILEMUSTEXIST, filter);
	dlg.m_ofn.lpstrTitle = _T("Save ProbeHistory ");
	dlg.m_ofn.lpstrInitialDir = strExePath;
	if (dlg.DoModal() != IDOK) return;
	
	CString strResultFile = dlg.GetFileName();
	
	CFile cfgFile(strResultFile, CFile::modeCreate | CFile::modeWrite);
	CArchive ar(&cfgFile, CArchive::store);
	ar.WriteString(_T("장착 시간,탈착 시간,Probe RFID,Probe Contact Count\r\n"));
//	ar.WriteString("Mount Time,UnMount Time,Probe RFID,Probe Contact Count\r\n");
	
	CString strResult;
	for(int i=0;i<m_listReportProbeHistory.GetItemCount();i++)
	{
		strResult.Format(_T("%s,%s,%s,%s\r\n"),
			m_listReportProbeHistory.GetItemText(i,0),
			m_listReportProbeHistory.GetItemText(i,1),
			m_listReportProbeHistory.GetItemText(i,2),
			m_listReportProbeHistory.GetItemText(i,3));
		
		ar.WriteString(strResult);	
	}
	
	ar.WriteString(_T("\r\n"));
	
	ar.Close();
	cfgFile.Close();
	
	ShellExecute(NULL, _T("open"), strResultFile, NULL, NULL, SW_SHOW);  //됨


	//lhj del 160621
// 	CWinApp* pApp = ::AfxGetApp(); 
// 	
// 	TCHAR szBuff[_MAX_PATH];
// 	::GetModuleFileName(pApp->m_hInstance, szBuff, _MAX_PATH);
// 	
// 	CString strExePath = g_sAppPath+dfDEFAULT_ALR_FILE_PATH;
// 	gf_MakeDir(strExePath);
// 	
// 	CString filter   = "CSV Files (*.csv)|*.csv|All Files(*.*)|*.*||";
// 	
// 	CString filename;
// 	
// 	CFileDialog dlg(FALSE, "", filename, OFN_FILEMUSTEXIST, filter);
// 	dlg.m_ofn.lpstrTitle = "Save ProbeHistory ";
// 	dlg.m_ofn.lpstrInitialDir = strExePath;
// 	if (dlg.DoModal() != IDOK) return;
// 	
// 	CString strResultFile = dlg.GetFileName();
// 	
// 	CFile cfgFile(strResultFile, CFile::modeCreate | CFile::modeWrite);
// 	CArchive ar(&cfgFile, CArchive::store);
// 	ar.WriteString("Happened Time,Cleared Time,Module,ProbeHistory Code,Host Mode,Command,Description\r\n");
// 	
// 	CString strResult;
// 	for(int i=0;i<m_listReportProbeHistory.GetItemCount();i++)
// 	{
// 		strResult.Format("%s,%s,%s,%s,%s,%s,%s\r\n",
// 			m_listReportProbeHistory.GetItemText(i,0),
// 			m_listReportProbeHistory.GetItemText(i,1),
// 			m_listReportProbeHistory.GetItemText(i,2),
// 			m_listReportProbeHistory.GetItemText(i,3),
// 			m_listReportProbeHistory.GetItemText(i,4),
// 			m_listReportProbeHistory.GetItemText(i,5),
// 			m_listReportProbeHistory.GetItemText(i,6));
// 		ar.WriteString(strResult);	
// 	}
// 	
// 	ar.WriteString("\r\n");
// 	
// 	ar.Close();
// 	cfgFile.Close();
// 	
// 	ShellExecute(NULL, "open", strResultFile, NULL, NULL, SW_SHOW);  //됨
}

void CDlgReportProbeHistorySearch::SetDateText(SYSTEMTIME time, int iD)
{
	CString strMonthName, strTmp;

	switch (time.wMonth)
	{
	case 1:		strMonthName = _T("Jan.");	break;
	case 2:		strMonthName = _T("Feb.");	break;
	case 3:		strMonthName = _T("Mar.");	break;
	case 4:		strMonthName = _T("Apr.");	break;
	case 5:		strMonthName = _T("May");	break;
	case 6:		strMonthName = _T("June");	break;
	case 7:		strMonthName = _T("July");	break;
	case 8:		strMonthName = _T("Aug.");	break;
	case 9:		strMonthName = _T("Sep.");	break;
	case 10:	strMonthName = _T("Oct.");	break;
	case 11:	strMonthName = _T("Nov.");	break;
	case 12:	strMonthName = _T("Dec.");	break;
	default:	strMonthName = _T("???");	break;
	}

	strTmp.Format(_T(". %s %02d/%04d"), strMonthName, time.wDay, time.wYear);

	switch (time.wDayOfWeek)
	{
	case 0:	SetDlgItemText(iD, _T("Sunday") + strTmp);	break;
	case 1:	SetDlgItemText(iD, _T("Monday") + strTmp);	break;
	case 2:	SetDlgItemText(iD, _T("Tuesday") + strTmp);	break;
	case 3:	SetDlgItemText(iD, _T("Wednesday") + strTmp);	break;
	case 4:	SetDlgItemText(iD, _T("Thursday") + strTmp);	break;
	case 5:	SetDlgItemText(iD, _T("Friday") + strTmp);	break;
	case 6:	SetDlgItemText(iD, _T("Saturday") + strTmp);	break;
	default:
		break;
	}
}

void CDlgReportProbeHistorySearch::OnMcnSelchangeMonthcalendarFrom(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMSELCHANGE pSelChange = reinterpret_cast<LPNMSELCHANGE>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (pSelChange)
	{
		m_stFrom = pSelChange->stSelStart;

		SetDateText(m_stFrom, IDC_EDIT_DATE_FROM);
	}

	*pResult = 0;
}


void CDlgReportProbeHistorySearch::OnMcnSelchangeMonthcalendarTo(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMSELCHANGE pSelChange = reinterpret_cast<LPNMSELCHANGE>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (pSelChange)
	{
		m_stTo = pSelChange->stSelStart;

		SetDateText(m_stTo, IDC_EDIT_DATE_TO);
	}

	*pResult = 0;
}
