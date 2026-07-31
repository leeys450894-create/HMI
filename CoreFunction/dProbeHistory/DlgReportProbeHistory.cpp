// DlgReportProbeHistory.cpp : implementation file
//

#include "pch.h"
#include "ProbeHistory.h"
#include "DlgReportProbeHistory.h"
#include "DlgReportProbeHistorySearch.h"
//#include "..\..\include\CommonFunc.h"

//#include "..\\..\\include\\resource_lang.h" //lhj 20181210 lang
#include <vector>	//jw.lee 20220105 add

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//#include "CommonFuncProbe.h"

/////////////////////////////////////////////////////////////////////////////
// CDlgReportProbeHistory dialog

CDlgReportProbeHistory::CDlgReportProbeHistory(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgReportProbeHistory::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgReportProbeHistory)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_hbrushBK = ::CreateSolidBrush(RGB(128, 128, 192));

// 	m_pSystem = NULL; 
// 	m_pSystem = SMEM_GetSharedMemory();
}


void CDlgReportProbeHistory::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgReportProbeHistory)
	DDX_Control(pDX, IDC_LIST_REPORT_ProbeHistory, m_listReportProbeHistory);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgReportProbeHistory, CDialog)
	//{{AFX_MSG_MAP(CDlgReportProbeHistory)
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_ALARM_REPORT_CLEAR, &CDlgReportProbeHistory::OnBnClickedButtonAlarmReportClear)
	ON_BN_CLICKED(IDC_BUTTON_ALARM_REPORT_SEARCH, &CDlgReportProbeHistory::OnBnClickedButtonAlarmReportSearch)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgReportProbeHistory message handlers

HBRUSH CDlgReportProbeHistory::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	if (pWnd == this)	return m_hbrushBK;

	return hbr;
}

void CDlgReportProbeHistory::OnDestroy() 
{
	CDialog::OnDestroy();
	
	::DeleteObject(m_hbrushBK);
}

BOOL CDlgReportProbeHistory::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_listReportProbeHistory.SetBkColor(RGB(255,255,220));
	m_listReportProbeHistory.SetTextBkColor(RGB(255,255,220));
	LV_COLUMN lvcolumn;
//	char* listcolumnResult[6] = {"Probe 장착 시간","Probe 탈착 시간","Probe RFID","Probe Contact Count"};
	TCHAR* listcolumnResult[6] = {_T("Probe 장착 시간"),_T("Probe 탈착 시간"),_T("Probe RFID"),_T("Probe Contact Count")};
//	TCHAR* listcolumnResult[6] = {"Probe Mount Time","Probe UnMount Time","Probe RFID","Probe Contact Count"};
    int widthResult[6] = {160,160,160,160,500,500};
	for(int i=0;i<6;i++)
	{
		lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lvcolumn.fmt = LVCFMT_LEFT;
		lvcolumn.pszText = listcolumnResult[i];
		lvcolumn.iSubItem = i;
		lvcolumn.cx = widthResult[i];
		m_listReportProbeHistory.InsertColumn(i,&lvcolumn);
	}

	m_listReportProbeHistory.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP | LVS_EX_ONECLICKACTIVATE );

	//// jsyoon 20170201 add 
	//ProbeHistory_DATA ProbeData;
	//ZeroMemory(&ProbeData, sizeof(ProbeData));

	//CString strMountTime;
	//strMountTime = gf_MakeCurTimeString();
	//_tcscpy_s(ProbeData.happendTime, _countof(ProbeData.clearedTime), _T("2016.10.31-00:00:00"));
	//_tcscpy_s(ProbeData.clearedTime, _countof(ProbeData.clearedTime), strMountTime);

	//GetProbeHistory(&ProbeData);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CDlgReportProbeHistory::AddProbeHistoryHistory(ProbeHistory_DATA *pAlarm)
{
	CString sHappenedTime, sClearedTime, sModule, sAlarmId, sProbeHistoryCode, sOperMode, sHostMode, sCommand, sString;
	UINT nAlarmId = pAlarm->id;
	UINT nAlarmCode = pAlarm->code;
	
	sHappenedTime.Format(_T("%s"), pAlarm->happendTime);
	sClearedTime.Format(_T("%s"), pAlarm->clearedTime);
	sModule.Format(_T("%s"), pAlarm->module);
	sAlarmId.Format(_T("%d"), pAlarm->id);
	sProbeHistoryCode.Format(_T("%d"), pAlarm->code);
	sCommand.Format(_T("%s"), pAlarm->cmd);
	sString.Format(_T("%s"), pAlarm->text);
	
	
	// 	int nOperMode = SMEM_GetSys_ProcessStatus();
	// 	
	// 	switch(nOperMode)
	// 	{
	// 	case SPS_IDLE:			sOperMode = "IDLE";				break;
	// 	case SPS_PROCESS:		sOperMode = "PROCESS";			break;
	// 	case SPS_INITIAL:		sOperMode = "INITIAL";			break;
	// 	case SPS_MAINT:			sOperMode = "MAINT";			break;
	// 	case SPS_WAFERTRANSFER:	sOperMode = "WAFERTRANSFER";	break;
	// 	case SPS_CYCLETEST:		sOperMode = "SYCLETEST";		break;
	// 	case SPS_PARTICLETEST:	sOperMode = "PARTICLETEST";		break;
	// 	case SPS_MAINTING:		sOperMode = "MAINTING";			break;
	// 	case SPS_EXITING:		sOperMode = "EXITING";			break;
	// 	default:				sOperMode = "UNKNOWN";			break;
	// 	}
	// 

	//int nHostMode = m_psystem->stTitle.m_nHostMode;	// lhj del 160621
	int nHostMode = df_HOST_OFFLINE;	// lhj add 160621
	switch(nHostMode)
	{
	case df_HOST_OFFLINE:		sHostMode = _T("OFFLINE");			break;
	case df_HOST_ONLINE_REMOTE:	sHostMode = _T("ONLINE-REMOTE");	break;
	default:				    sHostMode = _T("UNKNOWN");			break;
	}
	
	// Add List
	int nCount = m_listReportProbeHistory.GetItemCount();
	m_listReportProbeHistory.InsertItem(nCount, sHappenedTime);
	m_listReportProbeHistory.SetItemText(nCount,1,sClearedTime);
	m_listReportProbeHistory.SetItemText(nCount,2,sModule);
	m_listReportProbeHistory.SetItemText(nCount,3,sProbeHistoryCode);
	m_listReportProbeHistory.SetItemText(nCount,4,sHostMode);
	m_listReportProbeHistory.SetItemText(nCount,5,sCommand);
	m_listReportProbeHistory.SetItemText(nCount,6,sString);
	
	if (nCount > 500)	m_listReportProbeHistory.DeleteItem(0);
	
	// Add DB
	CString sSQL;
	sSQL.Format(_T("INSERT INTO AlarmHistory (sHappenedTime, sClearedTime, sModule, nAlarmCode, sHostMode, sCommand, sString) VALUES('%s','%s','%s', %d,'%s','%s','%s')"),
		sHappenedTime, sClearedTime, sModule, nAlarmCode, sHostMode, sCommand, sString);
	
	return Queryexe(sSQL);
}

BOOL CDlgReportProbeHistory::GetProbeHistoryInfo2(CString sFilePath, CString sSQL)
{
	COleVariant var;
	CString		sHappenedTime,sClearedTime,sModule,sProbeHistoryCode;
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


//	AfxDaoInit();

//	CDaoDatabase	db;
	//db.Open(_T(sFilePath));
	//if(db.IsOpen()) 
	//{
	//	CDaoRecordset	rs(&db);
	//	rs.Open(dbOpenDynaset, sSQL);
	//	if(rs.IsOpen())
	//	{
	//		nCount = rs.GetRecordCount();
	//		if(nCount > 0)
	//		{
	//			rs.MoveFirst();

	//			while(!rs.IsEOF()) 
	//			{
	//				rs.GetFieldValue(0,var);
	//				sHappenedTime.Format("%s", CString(var.pbVal));

	//				rs.GetFieldValue(1,var);
	//				sClearedTime.Format("%s", CString(var.pbVal));

	//				rs.GetFieldValue(2,var);
	//				sModule.Format("%s", CString(var.pbVal));

	//				rs.GetFieldValue(3,var);
	//				sProbeHistoryCode.Format("%d", int(var.intVal));

	//				iTiem = m_listReportProbeHistory.GetItemCount();
	//				m_listReportProbeHistory.InsertItem(iTiem,sHappenedTime);
	//				m_listReportProbeHistory.SetItemText(iTiem,1,sClearedTime);
	//				m_listReportProbeHistory.SetItemText(iTiem,2,sModule);
	//				m_listReportProbeHistory.SetItemText(iTiem,3,sProbeHistoryCode);
	//				rs.MoveNext();
	//			}
	//		}
	//		
	//		rs.Close();
	//		db.Close();
	//	}
	//	else
	//	{
	//		db.Close();
	//	}	
	//}
	//
	//AfxDaoTerm();

	//if( nCount > 0)
	//	return TRUE;
	//else
	//	return FALSE;
}

BOOL CDlgReportProbeHistory::GetProbeHistory(ProbeHistory_DATA *pProbeHistory)
{
	BOOL bRet = FALSE;
	CString sFrom,sTo, sTem, Sql_Statement, sProbeName;
	sFrom.Format(_T("%s"), pProbeHistory->happendTime);
	sTo.Format(_T("%s")  , pProbeHistory->clearedTime);
	sProbeName.Format(_T("%s"), pProbeHistory->module);
	
	Sql_Statement.Format(_T("SELECT * FROM ProbeHistory WHERE ((sHappenedTime >= '%s' AND sHappenedTime <= '%s') OR (sClearedTime >= '%s' AND sClearedTime <= '%s'))"),sFrom,sTo,sFrom,sTo);
	if (!sProbeName.IsEmpty())
	{
		Sql_Statement += _T(" AND ");
		sTem.Format(_T("sModule ='%s'"),sProbeName);
		Sql_Statement += sTem;
	}

//	CString sDBFile_Dest = g_sAppPath + dfDEFAULT_DB_DEST_PATH;
	CString sDBFile_Dest, sFileName;
	CFileFind finder;
	sDBFile_Dest.Format(_T("%s%s"), m_strRootPath, dfDEFAULT_DB_DEST_PATH );		// jsyoon 20160629 경로변경


	int nFromYear  = _ttol(sFrom.Left(4));
	int	nFromMonth = _ttol(sFrom.Mid(5,2));
	int nToYear    = _ttol(sTo.Left(4));
	int nToMonth   = _ttol(sTo.Mid(5,2));

	//CString sYearMonth[100];		// y.yun 20210110 50 -> 100
	//jw.lee 20220105 change
	std::vector<CString> vecStrDBfile;
	std::vector<CString>().swap(vecStrDBfile);
	//

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
			// y.yun 20210110 move pos	//jw.lee 20220105 remove.
			//if (nCnt >= 50)
			//{
			//	break;
			//}
			sTem.Format(_T("ProbeReport_%04d.%02d.mdb"),i,j);
			//sYearMonth[nCnt++] = sTem;
			//jw.lee 20220105 change.
			vecStrDBfile.push_back(sTem);
			//
		}
	}

	//for (i=0; i<nCnt; i++)a
	for (i=0; i < vecStrDBfile.size(); i++)	//jw.lee 20220105 change.
	{
//		sTem.Format("ProbeReport_%s.mdb",sFrom.Left(7));
		//sFileName.Format("%s%s", sDBFile_Dest, sYearMonth[i]);
		//jw.lee 20220105 change.
		sFileName.Format(_T("%s%s"), sDBFile_Dest,vecStrDBfile.at(i));
		//
		if (finder.FindFile(sFileName) == TRUE)
			bRet = GetProbeHistoryInfo2(sFileName,Sql_Statement);

	}

	return bRet;
}

BOOL CDlgReportProbeHistory::AddProbeHistoryInsert(ProbeHistory_DATA *pProbeHistory)
{
	if( GetProbeHistory(pProbeHistory) == TRUE )
		return TRUE;

	CString sHappenedTime, sClearedTime, sModule, sProbeHistoryCount;
	UINT nProbeHistoryCount = pProbeHistory->code;

	sHappenedTime.Format(_T("%s"), pProbeHistory->happendTime);
	sClearedTime.Format(_T("%s"), pProbeHistory->clearedTime);
	sModule.Format(_T("%s"), pProbeHistory->module);

	// Add List
	int nCount = m_listReportProbeHistory.GetItemCount();
	m_listReportProbeHistory.InsertItem(nCount, sHappenedTime);
	m_listReportProbeHistory.SetItemText(nCount,1,sClearedTime);
	m_listReportProbeHistory.SetItemText(nCount,2,sModule);
	m_listReportProbeHistory.SetItemText(nCount,3,sProbeHistoryCount);
	
	if (nCount > 500)	m_listReportProbeHistory.DeleteItem(0);
	
	// Add DB
	CString sSQL;
 	sSQL.Format(_T("INSERT INTO ProbeHistory (sHappenedTime, sClearedTime, sModule, nProbeHistoryCount) VALUES('%s','%s','%s', %d)"),
 					sHappenedTime, sClearedTime, sModule, nProbeHistoryCount);
// 	CString str;
// 	str = "2016.06.22-10:45:47";
// 	nProbeHistoryCode = 70000;
// 	sSQL.Format("UPDATE ProbeHistory SET nProbeHistoryCount = %d WHERE sModule = '%s' AND sHappenedTime = '%s'",
//  					nProbeHistoryCode,sModule,str);
	
	return Queryexe(sSQL);
}

BOOL CDlgReportProbeHistory::AddProbeHistoryUpdateContactCnt(ProbeHistory_DATA *pProbeHistory)
{
	CString sHappenedTime, sClearedTime, sModule, sProbeHistoryCount;
	UINT nProbeHistoryCount = pProbeHistory->code;
	
	sHappenedTime.Format(_T("%s"), pProbeHistory->happendTime);
	sClearedTime.Format(_T("%s"), pProbeHistory->clearedTime);
	sModule.Format(_T("%s"), pProbeHistory->module);
	sProbeHistoryCount.Format(_T("%d"),nProbeHistoryCount);
	
	
	// Update List
	int nCount = m_listReportProbeHistory.GetItemCount();
	if (nCount == 0)
	{
		m_listReportProbeHistory.InsertItem(nCount, sHappenedTime);
		m_listReportProbeHistory.SetItemText(nCount, 1, sClearedTime);
		m_listReportProbeHistory.SetItemText(nCount, 2, sModule);
		m_listReportProbeHistory.SetItemText(nCount, 3, sProbeHistoryCount);
	}
	else
	{
		for (int i = 0; i < nCount; i++)
		{
			CString strHappenedTime = m_listReportProbeHistory.GetItemText(i, 0);
			CString strModule = m_listReportProbeHistory.GetItemText(i, 2);

			if ((sHappenedTime == strHappenedTime) && (sModule == strModule))
			{
				m_listReportProbeHistory.SetItemText(i, 3, sProbeHistoryCount);
				break;
			}
		}
	}

	if (nCount > 500)	m_listReportProbeHistory.DeleteItem(0);
	
	// Update DB
	CString sSQL;
	CString str;
	sSQL.Format(_T("UPDATE ProbeHistory SET nProbeHistoryCount = %d WHERE sModule = '%s' AND sHappenedTime = '%s'"),
	  					nProbeHistoryCount,sModule,sHappenedTime);
	
	return Queryexe(sSQL);
}

BOOL CDlgReportProbeHistory::AddProbeHistoryUpdateContactUnmountTime(ProbeHistory_DATA *pProbeHistory)
{
	CString sHappenedTime, sClearedTime, sModule, sProbeHistoryCount;
	UINT nProbeHistoryCount = pProbeHistory->code;
	
	sHappenedTime.Format(_T("%s"), pProbeHistory->happendTime);
	sClearedTime.Format(_T("%s"), pProbeHistory->clearedTime);
	sModule.Format(_T("%s"), pProbeHistory->module);
	
	
	// Update List
	int nCount = m_listReportProbeHistory.GetItemCount();
	for(int i=0; i<nCount; i++)
	{
		CString strHappenedTime =  m_listReportProbeHistory.GetItemText(i,0);
		CString strModule = m_listReportProbeHistory.GetItemText(i,2);
		
		if( (sHappenedTime == strHappenedTime) && (sModule == strModule) )
		{
			m_listReportProbeHistory.SetItemText(i,1,sClearedTime);
			break;
		}
	}
	if (nCount > 500)	m_listReportProbeHistory.DeleteItem(0);
	
	// Update DB
	CString sSQL;
	CString str;
	sSQL.Format(_T("UPDATE ProbeHistory SET sClearedTime = '%s' WHERE sModule = '%s' AND sHappenedTime = '%s'"),
	  					sClearedTime,sModule,sHappenedTime);
	
	return Queryexe(sSQL);
}


BOOL CDlgReportProbeHistory::Queryexe(CString strSQL)
{
	g_csQuery.Lock();		//크리티컬 섹션 동기화 Lock, by Jay Hong : 2009.05.27

	BOOL b = TRUE;
	SYSTEMTIME st;
	::GetLocalTime(&st);
	
	CString sDBFile_Sour = g_sAppPath + dfDEFAULT_DB_PROBESOURCE_PATH;		//원본 파일, 절대로 건드리지 않는다
//	CString sDBFile_Dest = g_sAppPath + dfDEFAULT_DB_DEST_PATH;			//월단위로 DB파일 생성
	CString sDBFile_Dest, sFileName;
	sDBFile_Dest.Format(_T("%s%s"), /*m_pSystem->*/m_strRootPath, dfDEFAULT_DB_DEST_PATH );		// jsyoon 20160629 경로변경
	
	//"Report_200905.mdb" 형태로 DB생성
	sFileName.Format(_T("%sProbeReport_%04d.%02d.mdb"), sDBFile_Dest, st.wYear, st.wMonth);
	
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

	CDatabase db;
	b = db.OpenEx(strConnection);
	if (b == TRUE)
	{
		db.ExecuteSQL(strSQL.GetString());
		db.Close();
		
	}

	g_csQuery.Unlock();		// jsyoon 20150224 add

//	AfxDaoInit();
//
//	CDaoDatabase	db;
//	if(!db.IsOpen()) 
//	{
//		
////		db.Open(_T(sFileName));
//
//		TRY
//		{
//			db.Open((LPCTSTR)_T(sFileName), FALSE, FALSE);
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
//		
//		if(!db.IsOpen()) 
//		{
//			AfxMessageBox("Report Database Open Failed");
//			b = FALSE;
//		}
//		else
//		{
//			b = TRUE;
//		}
//	}
//	else
//	{
//		m_sDBFilePath = sFileName;
//		b = TRUE;
//	}
//
//	if( b == TRUE)
//	{
//		TRY
//		{
//			db.Execute(strSQL, dbFailOnError|dbSeeChanges);
//		}
//		CATCH(CDaoException, e)
//		{
//			TCHAR szCause[255];
//
//			e->GetErrorMessage(szCause, 255);
//			AfxMessageBox(szCause, MB_ICONERROR);
//
//			e->Delete();
//
//			b = FALSE;
//		}
//		END_CATCH
//
//		db.Close();
//	}
//
//	AfxDaoTerm();
//		
//	g_csQuery.Unlock();

	return b;
}

//BEGIN_EVENTSINK_MAP(CDlgReportProbeHistory, CDialog)
//    //{{AFX_EVENTSINK_MAP(CDlgReportProbeHistory)
//	ON_EVENT(CDlgReportProbeHistory, IDC_CTPUSHCTRL_ProbeHistory_REPORT_CLEAR, -600 /* Click */, OnClickCtpushctrlProbeHistoryReportClear, VTS_NONE)
//	ON_EVENT(CDlgReportProbeHistory, IDC_CTPUSHCTRL_ProbeHistory_REPORT_SEARCH, -600 /* Click */, OnClickCtpushctrlProbeHistoryReportSearch, VTS_NONE)
//	//}}AFX_EVENTSINK_MAP
//END_EVENTSINK_MAP()
//
//void CDlgReportProbeHistory::OnClickCtpushctrlProbeHistoryReportClear() 
//{
//	m_listReportProbeHistory.DeleteAllItems();
//}
//
//void CDlgReportProbeHistory::OnClickCtpushctrlProbeHistoryReportSearch() 
//{
//	CDlgReportProbeHistorySearch dlg;
//
//	dlg.m_strPath.Format( _T("%s"), /*m_pSystem->*/m_strRootPath);
//	dlg.DoModal();
//}

//lhj 20181225 lang Report Probe
int CDlgReportProbeHistory::UpdateLanguage(HINSTANCE hResource_Lang_Dll)
{
	//TCHAR strText[MAX_PATH]={0,};
	//TCHAR strText100[MAX_PATH]={0,};
	//TCHAR strText101[MAX_PATH]={0,};
	//TCHAR strText102[MAX_PATH]={0,};
	//TCHAR strText103[MAX_PATH]={0,};
	//TCHAR strText104[MAX_PATH]={0,};


	//if(hResource_Lang_Dll == NULL)
	//	return 0;

	//LoadString(hResource_Lang_Dll,IDS_STRING302_CLEAR, strText,MAX_PATH);
	//GetDlgItem(IDC_CTPUSHCTRL_ProbeHistory_REPORT_CLEAR)->SetWindowText((CString)strText);	

 //	LoadString(hResource_Lang_Dll,IDS_STRING316_Time, strText100,MAX_PATH);
 //	LoadString(hResource_Lang_Dll,IDS_STRING230_COUNT, strText101,MAX_PATH);

	//CString strProbeMountTime;
	//strProbeMountTime.Format("Probe Mount %s",strText100);
	//sprintf(strText102,strProbeMountTime);

	//CString strProbeUnMountTime;
	//strProbeUnMountTime.Format("Probe UnMount %s",strText100);
	//sprintf(strText103,strProbeUnMountTime);

	//CString strProbeContactCount;
	//strProbeContactCount.Format("Probe Contact  %s",strText101);
	//sprintf(strText104,strProbeContactCount);


	//TCHAR* listcolumnResult[6] = {strText102,strText103,"Probe RFID",strText104};
	//int widthResult[6] = {160,160,160,160,500,500};
	//for(int i=0;i<6;i++)
	//{
	//	TCHAR name[40];
	//	LVCOLUMN col;

	//	col.mask=LVCF_TEXT;
	//	col.pszText=listcolumnResult[i];
	//	col.cchTextMax=40;

	//	// Set column name
	//	m_listReportProbeHistory.SetColumn(i, &col);
	//}

	return 0;
}




void CDlgReportProbeHistory::OnBnClickedButtonAlarmReportClear()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_listReportProbeHistory.DeleteAllItems();
}


void CDlgReportProbeHistory::OnBnClickedButtonAlarmReportSearch()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CDlgReportProbeHistorySearch dlg;

	dlg.m_strPath.Format(_T("%s"), /*m_pSystem->*/m_strRootPath);
	dlg.DoModal();
}

void CDlgReportProbeHistory::UpdateProbeInfo()
{
	// jsyoon 20170201 add 
	ProbeHistory_DATA ProbeData;
	ZeroMemory(&ProbeData, sizeof(ProbeData));

	CString strMountTime;
	strMountTime = gf_MakeCurTimeString();
	_tcscpy_s(ProbeData.happendTime, _countof(ProbeData.clearedTime), _T("2016.10.31-00:00:00"));
	_tcscpy_s(ProbeData.clearedTime, _countof(ProbeData.clearedTime), strMountTime);

	GetProbeHistory(&ProbeData);
}
