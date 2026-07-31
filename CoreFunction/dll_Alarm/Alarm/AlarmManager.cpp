// AlarmManager.cpp : implementation file
//
#include "pch.h"
#include "Alarm.h"
#include "AlarmManager.h"
#include "AlarmFiles\AlarmXLSFile.h"
#include "AlarmFiles\AlarmCSVFile.h"
#include "DlgAlarm.h"
#include "DlgWarning.h"
#include "DlgReportAlarm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAlarmManager


CAlarmManager::CAlarmManager()
	: m_bFinalizeAlarm(false)
{
	m_bInitialized = FALSE;			// pwk 20130602 

	m_pAlarmXLSFile = new CAlarmXLSFile;
	m_pAlarmCSVFile = new CAlarmCSVFile;
	m_pDlgAlarm		  = nullptr;
	m_pDlgWarning	  = nullptr;
	m_pDlgReportAlarm = nullptr;


	m_alarmTable.RemoveAll();
	m_alarmMessage.RemoveAll();
}

CAlarmManager::~CAlarmManager()
{

}


BEGIN_MESSAGE_MAP(CAlarmManager, CWnd)
	//{{AFX_MSG_MAP(CAlarmManager)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CAlarmManager message handlers

void CAlarmManager::InitializeAlarm(HWND hParent, CRect r)
{
	// AlarmManager 윈도우 생성.
	Create(NULL,NULL,WS_VISIBLE|WS_CHILD,CRect(0,0,0,0),this,1);


	// Alarm, Warning 다이얼로그 생성.
//	CreateAlarmDlg(hParent, r);
//	CreateWarningDlg(hParent);

	BOOL ret = TRUE;

	// Alarm 다이얼로그 생성.
	m_pDlgAlarm = new CDlgAlarm(FromHandle(hParent));
	ret &= m_pDlgAlarm->Create(IDD_DLG_ALARM, FromHandle(hParent));
	m_pDlgAlarm->ShowWindow(SW_HIDE);
	m_pDlgAlarm->MoveWindow(r);

	// Warning 다이얼로그 생성.
	m_pDlgWarning = new CDlgWarning(FromHandle(hParent));
	ret &= m_pDlgWarning->Create(IDD_DLG_ALARM_WARNING, FromHandle(hParent));
	m_pDlgWarning->CenterWindow(m_pDlgAlarm);		// 중간에 맞춤
	m_pDlgWarning->ShowWindow(SW_HIDE);
	m_pDlgWarning->SetAlarmView(hParent);

	// Report 다이얼로그 생성.
	m_pDlgReportAlarm = new CDlgReportAlarm(FromHandle(hParent));
	ret &= m_pDlgReportAlarm->Create(IDD_DLG_REPORT_ALARM, FromHandle(hParent));
//	m_pDlgReportAlarm->SetSavePath(strFolderPath);
	m_pDlgReportAlarm->ShowWindow(SW_HIDE);
	m_pDlgReportAlarm->MoveWindow(r);

	// Alarm 파일 로드.
	gf_Initialize();
	
	CString strFileName;
	strFileName.Format(_T("%s%s"), g_sAppPath, +_T("Config\\CoreFunction\\AlarmConfig\\Alarm.xls"));
	LoadAlarmFile(strFileName);

	// alarm file 을 read 한다.
	
//	ret &= m_pAlarmXLSFile->LoadFile(g_sAppPath + _T("Config\\CoreFunction\\AlarmConfig\\Alarm.dat"));
	ret &= m_pAlarmCSVFile->LoadFile(g_sAppPath + _T("Config\\CoreFunction\\AlarmConfig\\AlarmMessage.dat"));

	ALARM_DATA m_stalarm;
	ZeroMemory(&m_stalarm, sizeof(m_stalarm));

	m_bInitialized = TRUE;			// pwk 20130602 
}

void CAlarmManager::InitializeReport(HWND hParent, CRect r)
{
	// Alarm Report 다이얼로그 생성.
	//CreateAlarmReportDlg(hParent, r);
}

void CAlarmManager::FinalizeAlarm()
{
	// 메시지루프 종료.
	m_bFinalizeAlarm = true;

	if (m_pDlgAlarm != nullptr)
	{
		ALARM_EVENT_DATA *pAlarm = nullptr;
		int nCount = m_pDlgAlarm->GetListCount();

		for (int i = 0; i<nCount; i++)
		{
			pAlarm = (ALARM_EVENT_DATA*)(m_pDlgAlarm->m_listAlarm.GetItemData(i));
			if (pAlarm != nullptr)
			{
				pAlarm->nRet = ALARM_RV_PROGRAMEXIT;
			}
		}
	}
	
	if (m_pAlarmXLSFile)
	{
		delete m_pAlarmXLSFile;
		m_pAlarmXLSFile = nullptr;
	}
	if (m_pAlarmCSVFile)
	{
		delete m_pAlarmCSVFile;
		m_pAlarmCSVFile = nullptr;
	}

	if (m_pDlgAlarm)
	{
		m_pDlgAlarm->DestroyWindow();
		delete m_pDlgAlarm;
		m_pDlgAlarm = nullptr;
	}

	if (m_pDlgWarning)
	{
		m_pDlgWarning->DestroyWindow();
		delete m_pDlgWarning;
		m_pDlgWarning = nullptr;
	}

	if (m_pDlgReportAlarm)
	{
		m_pDlgReportAlarm->DestroyWindow();
		delete m_pDlgReportAlarm;
		m_pDlgReportAlarm = nullptr;
	}
}

void CAlarmManager::ClearAlarm()
{
	// sih 20151005 add 
	if(m_pDlgAlarm == nullptr)
		return;
	//

	// 메시지루프 종료.
	ALARM_EVENT_DATA *pAlarm = nullptr;

	int nCount = m_pDlgAlarm->GetListCount();
	for(int i=0; i<nCount; i++)
	{
		pAlarm = (ALARM_EVENT_DATA*)(m_pDlgAlarm->m_listAlarm.GetItemData(i));
		if (pAlarm != nullptr)
		{
			pAlarm->nRet = ALARM_RV_ABORT;
		}
	}

	if (m_pDlgWarning == nullptr)
		return;

	if (m_pDlgWarning->IsWindowVisible())
	{
		m_pDlgWarning->DestroyWindow();
	}

}

void CAlarmManager::FinalizeReport()
{
//	AfxDaoTerm();
}


//////////////////////////////////////////////////////////////////////////
// 다이얼로그들 생성.
void CAlarmManager::CreateAlarmDlg(HWND hParent, CRect r)
{
	//if (!hParent) return;

	//if (m_pDlgAlarm != nullptr)
	//
	//if (::IsWindow(m_pDlgAlarm->GetSafeHwnd()))
	//{
	//	m_pDlgAlarm->DestroyWindow();
	//}
	//
	//OleInitialize(NULL);
	//
	//m_pDlgAlarm->Create(IDD_DLG_ALARM, FromHandle(hParent));
	//CRect rect;
	//m_pDlgAlarm->GetWindowRect(rect);
	//m_pDlgAlarm->MoveWindow(r.left,r.top, rect.Width(),rect.Height());	// jsyoon 20160226 add
	//m_pDlgAlarm->ShowWindow(SW_SHOW);
}

void CAlarmManager::CreateWarningDlg(HWND hParent)
{
	//if (!hParent) return;
	//
	//if (::IsWindow(m_pDlgWarning.GetSafeHwnd()))
	//{
	//	m_pDlgWarning.DestroyWindow();
	//}
	//
	//OleInitialize(NULL);
	//
	//m_pDlgWarning.Create(IDD_DLG_WARNING, FromHandle(hParent));
	//m_pDlgWarning.CenterWindow(FromHandle(m_dlgAlarm));		// 중간에 맞춤
	//m_pDlgWarning.ShowWindow(SW_HIDE);
}

void CAlarmManager::CreateAlarmReportDlg(HWND hParent, CRect r)
{
	//if (!hParent) return;
	//
	//if (::IsWindow(m_dlgReportAlarm.GetSafeHwnd()))
	//{
	//	m_dlgReportAlarm.DestroyWindow();
	//}
	//
	//OleInitialize(NULL);
	//
	//m_dlgReportAlarm.Create(IDD_DLG_REPORT_ALARM, FromHandle(hParent));

	//CRect rect;
	//m_dlgReportAlarm.GetWindowRect(rect);
	//m_dlgReportAlarm.MoveWindow(r.left,r.top, rect.Width(),rect.Height());	// jsyoon 20160226 add

	//m_dlgReportAlarm.ShowWindow(SW_HIDE);
}

//////////////////////////////////////////////////////////////////////////

CWnd* CAlarmManager::GetReportAlarmWnd()
{
	if (m_pDlgReportAlarm != nullptr)
		return m_pDlgReportAlarm;
	else
		return nullptr;

}

CWnd* CAlarmManager::GetAlarmWnd()
{
	if (m_pDlgAlarm != nullptr)
		return m_pDlgAlarm;
	else
		return nullptr;

}


BOOL CAlarmManager::LoadAlarmFile(CString strFileName /* = "alarm.csv" */)
{
	// jsyoon 20210506 change
	BOOL bRet = FALSE;
	if (strFileName.IsEmpty() == TRUE) 
	{
		CString strPaht;
		strPaht.Format(_T("%s%s"), g_sAppPath, _T("*.xls"));
		bRet = m_pDlgAlarm->LoadSpred(strPaht, true);
	}
	else
	{
		bRet = m_pDlgAlarm->LoadSpred(strFileName);
	}

	if (bRet == FALSE)
		return bRet;

	m_alarmTable.RemoveAll();

	// Get alarm count
	ALARM_DATA alarmData;
	CString strLine;
	CString strText;
	CString str;
	int iUse = 0;
	int iRow=0;
	do 
	{
		ZeroMemory(&alarmData, sizeof(alarmData));

		str = m_pDlgAlarm->GetText(iRow, 1);

		if( str.CompareNoCase(_T("[FILE END]")) == 0 )
			break;

		if( str.Find(_T("Alarm")) != 0 && !str.IsEmpty())
		{
			iUse = _ttoi(m_pDlgAlarm->GetText(iRow, 13));	// USE
			if (iUse != 1)
			{
				iRow++;
				continue;
			}

			// ID	
			alarmData.id = _ttoi(m_pDlgAlarm->GetText(iRow, 1));

			// CODE
			alarmData.code = _ttoi(m_pDlgAlarm->GetText(iRow, 2));

			// LEVEL
			str = m_pDlgAlarm->GetText(iRow, 3).GetAt(0);
			if (str.CompareNoCase(_T("H")) == 0)
				alarmData.level = ALARM_LEVLE_HIGH;
			else if (str.CompareNoCase(_T("L")) == 0)
				alarmData.level = ALARM_LEVLE_LOW;
			else  if (str.CompareNoCase(_T("O")) == 0)
				alarmData.level = ALARM_LEVLE_MSG;
			else
				alarmData.level = ALARM_LEVLE_HIGH;

			// module
			strText = m_pDlgAlarm->GetText(iRow, 4);
			strText.TrimLeft();
			strText.TrimRight();
			strText.Replace(_T("?"), _T(""));
			_tcscpy_s(alarmData.module, _countof(alarmData.module), strText);

			//Alarm Command
			strText = m_pDlgAlarm->GetText(iRow, 5);
			strText.TrimLeft();
			strText.TrimRight();
			strText.Replace(_T("?"), _T(""));
			_tcscpy_s(alarmData.cmd, _countof(alarmData.cmd), strText);


			

			// Alarm Test Type
			CString strUse;
			strUse.Format(_T("%s"), m_pDlgAlarm->GetText(iRow, 10) );

			if( strUse.CompareNoCase(_T("E")) == 0)
			{
				//Alarm Text
				strText = m_pDlgAlarm->GetText(iRow, 11);
				strText.TrimLeft();
				strText.TrimRight();
				strText.Replace(_T("?"), _T(""));
				_tcscpy_s(alarmData.text, _countof(alarmData.text), strText);


				//Alarm Description
				strText = m_pDlgAlarm->GetText(iRow, 12);
				strText.TrimLeft();
				strText.TrimRight();
				strText.Replace(_T("?"), _T(""));
				_tcscpy_s(alarmData.Description, _countof(alarmData.Description), strText);
			}
			else
			{
				//Alarm Text
				strText = m_pDlgAlarm->GetText(iRow, 6);
				strText.TrimLeft();
				strText.TrimRight();
				strText.Replace(_T("?"), _T(""));
				_tcscpy_s(alarmData.text, _countof(alarmData.text), strText);


				//Alarm Description
				strText = m_pDlgAlarm->GetText(iRow, 7);
				strText.TrimLeft();
				strText.TrimRight();
				strText.Replace(_T("?"), _T(""));
				_tcscpy_s(alarmData.Description, _countof(alarmData.Description), strText);
			}

			// Retry
			alarmData.nRetry = _ttoi(m_pDlgAlarm->GetText(iRow, 8));		
			//////////////////////////////////////////////////////////////////////////

			// Alarm Type
			alarmData.nType = _ttoi(m_pDlgAlarm->GetText(iRow, 9));	

			// 			//Alarm Num
			m_alarmTable.SetAt(alarmData.id, alarmData);
		}

		iRow++;

	} while (iRow < ALARM_MAX_COUNT);

	return bRet;
}


BOOL CAlarmManager::ReloadAlarmList()
{
	BOOL ret = TRUE;
	// alarm file 을 read 한다.

	CString strFileName; 
	strFileName.Format(_T("%s%s"), g_sAppPath, +_T("Config\\CoreFunction\\AlarmConfig\\Alarm.xls")); 
	ret = LoadAlarmFile(strFileName);

	/*ret &= m_pAlarmXLSFile->XLStoDAT(g_sAppPath + _T("Config\\CoreFunction\\AlarmConfig\\Alarm.xls"));
	if (ret) ret &= m_pAlarmXLSFile->LoadFile(g_sAppPath + _T("Config\\CoreFunction\\AlarmConfig\\Alarm.dat"));
	if (ret) ret &= m_pAlarmCSVFile->CSVtoDAT(g_sAppPath + _T("Config\\CoreFunction\\AlarmConfig\\AlarmMessage.csv"));
	if (ret) ret &= m_pAlarmCSVFile->LoadFile(g_sAppPath + _T("Config\\CoreFunction\\AlarmConfig\\AlarmMessage.dat"));*/

	return ret;
}


BOOL CAlarmManager::FindAlarmTable(UINT id, ALARM_EVENT_DATA& alarm)
{
	ALARM_DATA temp;

	//if (m_pAlarmXLSFile->Lookup(id, temp) == TRUE)
	//{
	//	alarm.Copy(temp);

	//	return TRUE;
	//}

	//return FALSE;

	BOOL bFind = m_alarmTable.Lookup(id, temp);
	
	if (bFind == TRUE)
	{
		alarm.Copy(temp);
		
		return TRUE;
	}
	else
	{
		return FALSE;
	}

//	return bFind;
}

int CAlarmManager::SetAlarm(UINT id)
{
	m_cs_set.Lock();

	return SetAlarmSub(id, &m_cs_set);
}

int CAlarmManager::SetAlarmSub(UINT id, CCriticalSection *pCS)
{
	if (m_bInitialized == FALSE)
	{
		pCS->Unlock();	// 다음 알람을 받기 위해서 lock 을 해제 한다.  // jsyoon 2022.10.04 add
		return FALSE;
	}
		
	if( id < 1000)
	{
		pCS->Unlock();	// 다음 알람을 받기 위해서 lock 을 해제 한다. // jsyoon 2022.10.04 add
		return FALSE;
	}

	//if( id == m_stalarm.id)
	//	return FALSE;

	int nRet = FALSE;
	//if(!FindAlarmTable(id, m_stalarm))
	//{
	//	memset(&m_stalarm, 0, sizeof(m_stalarm));
	//	m_stalarm.id = id;
	//	m_stalarm.code = id;
	//	m_stalarm.level = 'H';
	//	_tcscpy_s(m_stalarm.module, _countof(m_stalarm.module), _T("UNKNOWN"));
	//	_tcscpy_s(m_stalarm.cmd,    _countof(m_stalarm.cmd),    _T("UNKNOWN"));
	//	_tcscpy_s(m_stalarm.text,   _countof(m_stalarm.text),   _T("UNKNOWN"));
	//}

	ALARM_EVENT_DATA alarm;

	if (FindAlarmTable(id, alarm) == FALSE)
	{
		alarm.id = id;
		alarm.code = id;
	}

	switch (alarm.level)
	{
	case ALARM_LEVLE_HIGH:
		nRet = SetHeavyAlarm(alarm, pCS);
		break;
	case ALARM_LEVLE_LOW:
		nRet = SetLightAlarm(alarm, pCS);
		break;
	case ALARM_LEVLE_MSG:
		nRet = SetLightAlarm(alarm, pCS, FALSE);
		break;
	default:
		break;
	}

	return nRet;
}

int CAlarmManager::SetHeavyAlarm(ALARM_EVENT_DATA &alarm, CCriticalSection *pCS)
{
	// 이미 발생된 알람이면 리턴.
	if(m_pDlgAlarm->FindList(alarm.id))
	{
		pCS->Unlock();	// 다음 알람을 받기 위해서 lock 을 해제 한다.
		return ALARM_RV_SUCCESSED;
	}

	// 알람 리턴값 초기화
	alarm.nRet = 0;
	_tcscpy_s(alarm.happendTime, _countof(alarm.happendTime), gf_MakeCurTimeString());

	// 리포트에 기록
	m_pDlgReportAlarm->AddAlarmHistory(alarm);

	// 리스트 컨트롤에 추가.
	m_pDlgAlarm->InsertList(&alarm);

	// 알람 발생시 메시지 전송.	
	::PostMessage(m_hAlarmWnd, WM_ALARM_ACCURED, WPARAM(alarm.id), NULL);		// jsyoon 20170731 change

	pCS->Unlock();	// 다음 알람을 받기 위해서 lock 을 해제 한다.

	// 메시지루프. 알람의 리턴값이 결정되면 빠져나온다.
	// 메시지루프. 알람의 리턴값이 결정되면 빠져나온다.
	ResetEvent(alarm.hEvent);
	WaitForSingleObject(alarm.hEvent, INFINITE);

	if (m_bFinalizeAlarm == false)
	{
		_tcscpy_s(alarm.clearedTime, _countof(alarm.clearedTime), gf_MakeCurTimeString());

		// 리스트컨트롤에서 삭제.
		m_pDlgAlarm->DeleteList(alarm.id);

		// 리포트에 기록
		m_pDlgReportAlarm->AddAlarmHistory(alarm, true);

		// 알람 클리어시 메시지 전송.
		::PostMessage(m_hAlarmWnd, WM_ALARM_CLEARED, WPARAM(alarm.id), NULL);

		return alarm.nRet;
	}

	return ALARM_RV_PROGRAMEXIT;


//	MSG msg;
//	while(TRUE)
//	{
//		if (alarm.nRet > 0) 
//		{
//			break;
//		}
//		// jsyoon 20140409 change
//// 		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
//// 		{
//// 			TranslateMessage(&msg);
//// 			DispatchMessage(&msg);
//// 		}
//		while(PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))
//		{
//			if(!AfxGetApp()->PreTranslateMessage(&msg))		   // jsyoon 20131120
//			{
//				TranslateMessage(&msg);
//				Sleep(0);					// jsyoon 20150224 change
//				DispatchMessage(&msg);
//				
//				if (alarm.nRet > 0)		//이렇게 해 줘도 상관이 없는 것인지... 계속 모니터링 해 볼 것
//					break;
//				
//// 				Sleep(0);
//// 				DispatchMessage(&msg);
//			}
//			
//		}
//		Sleep(0);
//	}
//
//	if(alarm.nRet == ALARM_RV_PROGRAMEXIT)
//	{
//		// 프로그램 종료시, 메시지루프 빠져나온후 종료메시지 날림.
//		PostQuitMessage(0);
//	}
//	else
//	{
//		// 리스트컨트롤에서 삭제.
//		m_dlgAlarm.DeleteList(alarm.id);
//
////		strcpy(alarm.clearedTime, gf_MakeCurTimeString());
//		_tcscpy_s(alarm.clearedTime, _countof(alarm.clearedTime), gf_MakeCurTimeString());
//		m_dlgReportAlarm.AddAlarmHistory(&alarm);
//
//		// 알람 클리어시 메시지 전송.
////		::PostMessage(SMEM_GetMetrosysViewWnd(), WM_ALARM_CLEARED, WPARAM(alarm.id), NULL);		// jsyoon 20170731 change
//		::PostMessage(m_hAlarmWnd, WM_ALARM_CLEARED, WPARAM(alarm.code), NULL);		// jsyoon 20170731 change
//	}
//
//	return alarm.nRet;
}

int CAlarmManager::SetLightAlarm(ALARM_EVENT_DATA &alarm, CCriticalSection *pCS, BOOL bLow)
{
	if(m_pDlgWarning->FindList(alarm.id))
	{
		m_pDlgWarning->ShowWindow(SW_SHOW);
		pCS->Unlock();	// 다음 알람을 받기 위해서 lock 을 해제 한다.
		return ALARM_RV_SUCCESSED;
	}

// 	strcpy(alarm.happendTime, gf_MakeCurTimeString());
// 	strcpy(alarm.clearedTime, alarm.happendTime);

	_tcscpy_s(alarm.happendTime, _countof(alarm.happendTime), gf_MakeCurTimeString());
	_tcscpy_s(alarm.clearedTime, _countof(alarm.clearedTime), alarm.happendTime);

	m_pDlgReportAlarm->AddAlarmHistory(alarm, true, true);

	m_pDlgWarning->InsertList(&alarm);
	m_pDlgWarning->ShowWindow(SW_SHOW);

	// Warning 발생시 메시지 전송.	
	if( bLow)
	{
		::PostMessage(m_hAlarmWnd, WM_WARNING_ACCURED, WPARAM(alarm.id), NULL);		// jsyoon 20170731 change
	}

	pCS->Unlock();	// 다음 알람을 받기 위해서 lock 을 해제 한다.
    return ALARM_RV_SUCCESSED;
}

int CAlarmManager::GetListCount()
{
	if(m_pDlgAlarm == nullptr)
		return 0;
	
	return m_pDlgAlarm->GetListCount();
}

void CAlarmManager::SetAlarmView(HWND hWnd)
{
	m_hAlarmWnd = hWnd;

	m_pDlgWarning->SetAlarmView(hWnd);
}

void CAlarmManager::SetHostMode(int iMode)
{
	m_pDlgReportAlarm->SetHostMode(iMode);
}

void CAlarmManager::SetHostSavePath(TCHAR* pPath)
{
	CString strPath;
	strPath.Format(_T("%s"), pPath);
	m_pDlgReportAlarm->SetSavePath(strPath);
}

void CAlarmManager::GetMeassgeList(int i, TCHAR* pchar)
{
	CString strFilePath;
//	strFilePath.Format("%s%s", g_sAppPath, STR_DIR_CONFIG);
	strFilePath.Format(_T("%s"), g_sAppPath);

	CString str;
	str = m_pDlgAlarm->MessageCode2Str(i, strFilePath);

	_tcscpy_s(pchar, 1024, str);

}

void CAlarmManager::GetAlarmText(int i, TCHAR* pchar)
{
	CString str;

	ALARM_EVENT_DATA alarm;
	FindAlarmTable(i, alarm);
	str.Format(_T("%s"), alarm.text);

	_tcscpy_s(pchar, 1024, str);
}

//lhj 20181225 Report Alarm 
int CAlarmManager::AlarmUpdateLanguage(HINSTANCE hResource_Lang_Dll)
{
//	m_dlgReportAlarm.UpdateLanguage(hResource_Lang_Dll);
//	m_pDlgWarning.UpdateLanguage(hResource_Lang_Dll);
//	m_dlgAlarm.UpdateLanguage(hResource_Lang_Dll);
	return 0;
}