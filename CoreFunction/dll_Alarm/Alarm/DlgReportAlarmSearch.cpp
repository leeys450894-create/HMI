// DlgReportAlarmSearch.cpp : implementation file
//

#include "pch.h"
#include "Alarm.h"
#include "AlarmDefine.h"
#include "DlgReportAlarmSearch.h"

#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>                      // MFC ODBC 데이터베이스 클래스입니다.
#endif // _AFX_NO_DB_SUPPORT


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgReportAlarmSearch dialog


CDlgReportAlarmSearch::CDlgReportAlarmSearch(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgReportAlarmSearch::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgReportAlarmSearch)
	//}}AFX_DATA_INIT

	m_strPath.Empty();
	m_strRootPath.Empty();
	m_pRs = nullptr;
}


void CDlgReportAlarmSearch::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgReportAlarmSearch)
	DDX_Control(pDX, IDC_BUTTON_SEARCH_ALARM, m_btnSearchAlarm);
	DDX_Control(pDX, IDC_LIST_REPORT_ALARM_SERCH, m_listReportAlarm);
	DDX_Control(pDX, IDC_MONTHCALENDAR_FROM, m_ctDateFrom);
	DDX_Control(pDX, IDC_MONTHCALENDAR_TO, m_ctDateTo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgReportAlarmSearch, CDialog)
	//{{AFX_MSG_MAP(CDlgReportAlarmSearch)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH_ALARM, OnButtonSearchAlarm)
	ON_BN_CLICKED(IDC_BTN_ALARM_SEARCH_SAVEAS_CSV, OnBtnAlarmSearchSaveasCsv)
	ON_NOTIFY(MCN_SELECT, IDC_MONTHCALENDAR_FROM, &CDlgReportAlarmSearch::OnMcnSelectMonthcalendarFrom)
	ON_NOTIFY(MCN_SELECT, IDC_MONTHCALENDAR_TO, &CDlgReportAlarmSearch::OnMcnSelectMonthcalendarTo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgReportAlarmSearch message handlers

BOOL CDlgReportAlarmSearch::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_listReportAlarm.SetBkColor(RGB(255,255,220));
	m_listReportAlarm.SetTextBkColor(RGB(255,255,220));
	LV_COLUMN lvcolumn;
	TCHAR* listcolumnResult[7] = {_T("Happened Time"),_T("Cleared Time"),_T("Module"),_T("Alarm Code"),_T("Host Mode"),_T("Command"),_T("String")};
    int widthResult[7] = {160,160,80,80,110,100,500};
	for(int i=0;i<7;i++)
	{
		lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvcolumn.fmt = LVCFMT_LEFT;
		lvcolumn.pszText = listcolumnResult[i];
		lvcolumn.iSubItem = i;
		lvcolumn.cx = widthResult[i];
		m_listReportAlarm.InsertColumn(i,&lvcolumn);
	}
	m_listReportAlarm.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP | LVS_EX_ONECLICKACTIVATE );
	m_listReportAlarm.DeleteAllItems();
	
	// 오늘 날짜로 초기화 해 둔다.
	GetLocalTime(&m_stFrom);
	GetLocalTime(&m_stTo);
	SetDateText(m_stFrom, IDC_EDIT_DATE_FROM);
	SetDateText(m_stTo, IDC_EDIT_DATE_TO);
	
	return TRUE;  // return TRUE unless you set the focus to a control
}

void CDlgReportAlarmSearch::SearchAlarmHistory()
{
	BeginWaitCursor();

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

	m_listReportAlarm.DeleteAllItems();

	int nFromYear  = m_stFrom.wYear;
	int	nFromMonth = m_stFrom.wMonth;
	int nToYear    = m_stTo.wYear ;
	int nToMonth   = m_stTo.wMonth ;

	CStringArray vecYearMonth;
	vecYearMonth.RemoveAll();
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

		for (j = nMon1; j <= nMon2; j++)
		{
			CString sTem;
			sTem.Format(_T("\\Report_%04d%02d.mdb"), i, j);

			vecYearMonth.Add(sTem);

			if (nCnt >= 50)
			{
				AfxMessageBox(_T("You can't search (so many data))"));

				EndWaitCursor();

				return;
			}
		}

		// New accdb file
		for (j = nMon1; j <= nMon2; j++)
		{
			CString sTem;
			sTem.Format(_T("\\Report_%04d%02d.accdb"), i, j);

			vecYearMonth.Add(sTem);

			if (nCnt >= 50)
			{
				AfxMessageBox(_T("You can't search (so many data))"));

				EndWaitCursor();

				return;
			} 
		}
	}

	CString AlarmCode, sTem;
	GetDlgItemText(IDC_EDIT_ALARM_SEARCH_ID, AlarmCode);

	CString Sql_Statement;
	Sql_Statement.Format(_T("SELECT * FROM AlarmHistory WHERE ((sHappenedTime >= '%s' AND sHappenedTime <= '%s') OR (sClearedTime >= '%s' AND sClearedTime <= '%s'))"),sFrom,sTo,sFrom,sTo);

	if (AlarmCode.IsEmpty() != TRUE)
	{
		Sql_Statement += _T(" AND ");
		sTem.Format(_T("nAlarmCode =%d") ,_ttoi(AlarmCode));
		Sql_Statement += sTem;
	}

	CString sDBFile_Dest, sFileName;
//	CString sDBFile_Dest = g_sAppPath + dfDEFAULT_DB_DEST_PATH;
//	sDBFile_Dest.Format(_T("%s%s"), m_strPath, dfDEFAULT_DB_DEST_PATH );		// jsyoon 20160629 경로변경
//	sDBFile_Dest.Format(_T("%s%s"), m_strPath, _T("ALARM\\"));					// jsyoon 20200720 change

	m_nAlarmCount = 0;
	m_bStop = FALSE;

	g_csQuery.Lock();

	// jsyoon 2022.06.28
	sDBFile_Dest.Format(_T("%s%s"), m_strRootPath, _T("ALARM\\"));

	for (i=0; i<vecYearMonth.GetSize(); i++)
	{
		sFileName.Format(_T("%s%s"), sDBFile_Dest, vecYearMonth.GetAt(i));
	//	sFileName = m_strRootPath + vecYearMonth.GetAt(i);

		CFileFind finder;
		if (finder.FindFile(sFileName) == FALSE)
		{
			continue;
		}
		
		GetAlarmInfo(sFileName,Sql_Statement);

		if (m_bStop) break;
	}
	g_csQuery.Unlock();

	vecYearMonth.RemoveAll();

	//CString sCount;
	//sCount.Format(_T("%d"),m_nAlarmCount);
	//GetDlgItem(IDC_CTTEXTCTRL_ALARM_COUNT)->SetWindowText(sCount);

	EndWaitCursor();
}

BOOL CDlgReportAlarmSearch::GetAlarmInfo(CString sFilePath, CString sSQL)
{
	CString		sHappenedTime, sClearedTime, sModule, sAlarmId, sAlarmCode, sOperationMode, sHostMode, sCommand, sString;
	int nCount = 0;

	CString strConnection;
	strConnection.Format(_T("Driver={Microsoft Access Driver (*.mdb, *.accdb)};dbq=%s"), sFilePath.GetString());

	BOOL bRet = TRUE;

	try
	{
		if (m_db.OpenEx(strConnection) == TRUE)
		{
			m_pRs = new CRecordset(&m_db);

			if (m_pRs != nullptr)
			{
				if (m_pRs->Open(CRecordset::dynaset, sSQL) == TRUE)
				{
					nCount = m_pRs->GetRecordCount();
					if (nCount > 0)
					{
						m_pRs->MoveFirst();

						while (m_pRs->IsEOF() == FALSE)
						{
							m_pRs->GetFieldValue((short)0, sHappenedTime);
							m_pRs->GetFieldValue((short)1, sClearedTime);
							m_pRs->GetFieldValue((short)2, sModule);
							m_pRs->GetFieldValue((short)3, sAlarmCode);
							m_pRs->GetFieldValue((short)4, sHostMode);
							m_pRs->GetFieldValue((short)5, sCommand);
							m_pRs->GetFieldValue((short)6, sString);

							int iTiem = m_listReportAlarm.GetItemCount();
							m_listReportAlarm.InsertItem(iTiem, sHappenedTime);
							m_listReportAlarm.SetItemText(iTiem, 1, sClearedTime);
							m_listReportAlarm.SetItemText(iTiem, 2, sModule);
							m_listReportAlarm.SetItemText(iTiem, 3, sAlarmCode);
							m_listReportAlarm.SetItemText(iTiem, 4, sHostMode);
							m_listReportAlarm.SetItemText(iTiem, 5, sCommand);
							m_listReportAlarm.SetItemText(iTiem, 6, sString);

							m_nAlarmCount++;
							if (m_nAlarmCount == 10000)
							{
								if (MessageBox(_T("It was collection of 10000 data from database. Do you wish to continue?"), _T("Search"), MB_ICONQUESTION | MB_YESNO) == IDNO)
								{
									m_bStop = TRUE;
									break;
								}
							}

							m_pRs->MoveNext();
						}
					}

					m_pRs->Close();
				}
				else
				{
					bRet = FALSE;
				}
			}
			else
			{
				bRet = FALSE;
			}

			

			/*CRecordset	rs(&m_db);
			if (rs.Open(CRecordset::dynaset, sSQL) == TRUE)
			{
				nCount = rs.GetRecordCount();
				if(nCount>0)
				{
					rs.MoveFirst();

					while (rs.IsEOF() == FALSE)
					{
						rs.GetFieldValue((short)0, sHappenedTime);
						rs.GetFieldValue((short)1, sClearedTime);
						rs.GetFieldValue((short)2, sModule);
						rs.GetFieldValue((short)3, sAlarmCode);
						rs.GetFieldValue((short)4, sHostMode);
						rs.GetFieldValue((short)5, sCommand);
						rs.GetFieldValue((short)6, sString);

						int iTiem = m_listReportAlarm.GetItemCount();
						m_listReportAlarm.InsertItem(iTiem, sHappenedTime);
						m_listReportAlarm.SetItemText(iTiem, 1, sClearedTime);
						m_listReportAlarm.SetItemText(iTiem, 2, sModule);
						m_listReportAlarm.SetItemText(iTiem, 3, sAlarmCode);
						m_listReportAlarm.SetItemText(iTiem, 4, sHostMode);
						m_listReportAlarm.SetItemText(iTiem, 5, sCommand);
						m_listReportAlarm.SetItemText(iTiem, 6, sString);

						m_nAlarmCount++;
						if (m_nAlarmCount == 10000)
						{
							if (MessageBox(_T("It was collection of 10000 data from database. Do you wish to continue?"),_T("Search"), MB_ICONQUESTION|MB_YESNO) == IDNO)
							{
								m_bStop = TRUE;
								break;
							}
						}

						rs.MoveNext();
					}
				}
				rs.Close();
				m_db.Close();

			}
			else
			{
				m_db.Close();
				bRet = FALSE;
			}
			*/
		}
		else
		{
			bRet = FALSE;
		}
	}
	catch (CException* e)
	{
		e->ReportError();
	}

	if (m_pRs != nullptr)
	{
		delete m_pRs;
		m_pRs = nullptr;
	}

	if(m_db.IsOpen())
		m_db.Close();
	
	return bRet;
}

void CDlgReportAlarmSearch::OnButtonSearchAlarm() 
{
	m_btnSearchAlarm.EnableWindow(FALSE);
	SearchAlarmHistory();
	m_btnSearchAlarm.EnableWindow(TRUE);
}

void CDlgReportAlarmSearch::OnMcnSelectMonthcalendarFrom(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMSELCHANGE pSelChange = reinterpret_cast<LPNMSELCHANGE>(pNMHDR);

	if (pSelChange)
	{
		m_stFrom = pSelChange->stSelStart;

		SetDateText(m_stFrom, IDC_EDIT_DATE_FROM);
	}

	*pResult = 0;
}

void CDlgReportAlarmSearch::OnMcnSelectMonthcalendarTo(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMSELCHANGE pSelChange = reinterpret_cast<LPNMSELCHANGE>(pNMHDR);

	if (pSelChange)
	{
		m_stTo = pSelChange->stSelStart;

		SetDateText(m_stTo, IDC_EDIT_DATE_TO);
	}

	*pResult = 0;
}

//void CDlgReportAlarmSearch::OnLoadDayInfoCtdatectrlTo(short nMonth, short nYear, short nSection) 
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
//void CDlgReportAlarmSearch::OnLoadDayInfoCtdatectrlFrom(short nMonth, short nYear, short nSection) 
//{
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
//}

void CDlgReportAlarmSearch::SetDateText(SYSTEMTIME time, int iD)
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

void CDlgReportAlarmSearch::OnBtnAlarmSearchSaveasCsv() 
{
	CFileDialog dlg(FALSE, _T("csv"), _T(""), OFN_FILEMUSTEXIST, _T("csv Files (*.csv)|*.csv|"));
	dlg.m_ofn.lpstrTitle = _T("Save Alarm ");

	if (dlg.DoModal() != IDOK)
		return;

	CString strResultFile = dlg.GetFolderPath() + _T("\\") + dlg.GetFileName();

	CFile cfgFile(strResultFile, CFile::modeCreate | CFile::modeWrite);
	CArchive ar(&cfgFile, CArchive::store);

	ar.WriteString(_T("Happened Time,Cleared Time,Module,Alarm Code,Host Mode,Command,Description\r\n"));

	for (int i = 0; i < m_listReportAlarm.GetItemCount(); i++)
	{
		CString strResult;
		strResult.Format(_T("%s,%s,%s,%s,%s,%s,%s\r\n"),
			m_listReportAlarm.GetItemText(i, 0).GetString(),
			m_listReportAlarm.GetItemText(i, 1).GetString(),
			m_listReportAlarm.GetItemText(i, 2).GetString(),
			m_listReportAlarm.GetItemText(i, 3).GetString(),
			m_listReportAlarm.GetItemText(i, 4).GetString(),
			m_listReportAlarm.GetItemText(i, 5).GetString(),
			m_listReportAlarm.GetItemText(i, 6).GetString());
		ar.WriteString(strResult);
	}

	ar.WriteString(_T("\r\n"));

	ar.Close();
	cfgFile.Close();

	ShellExecute(NULL, _T("open"), strResultFile, NULL, NULL, SW_SHOW);
}
