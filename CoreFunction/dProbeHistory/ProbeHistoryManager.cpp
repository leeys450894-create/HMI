// ProbeHistoryManager.cpp : implementation file
//

#include "pch.h"
#include "ProbeHistory.h"
#include "ProbeHistoryManager.h"
//#include "..\..\include\CommonFunc.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//#include "CommonFuncProbe.h"
//include "../Include/API_SMEM.h"

/////////////////////////////////////////////////////////////////////////////
// CProbeHistoryManager

CProbeHistoryManager::CProbeHistoryManager()
{
	m_bInitialized = FALSE;			// pwk 20130602 

	m_pdlgReportProbeHistory = nullptr;
}

CProbeHistoryManager::~CProbeHistoryManager()
{

}


BEGIN_MESSAGE_MAP(CProbeHistoryManager, CWnd)
	//{{AFX_MSG_MAP(CProbeHistoryManager)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CProbeHistoryManager message handlers

// void CProbeHistoryManager::InitializeProbeHistory(HWND hParent)
// {
// 	// ProbeHistoryManager 윈도우 생성.
// 	Create(NULL,NULL,WS_VISIBLE|WS_CHILD,CRect(0,0,0,0),this,1);
// 
// 	// ProbeHistory 다이얼로그 생성.
// 	CreateProbeHistoryDlg(hParent);
// 
// 	// ProbeHistory 파일 로드.
// 	gf_Initialize();
// 	
// 	CString strFileName = g_sAppPath + dfDEFAULT_PROBE_HISTORY_FILE_PATH;
// 	LoadProbeHistoryFile(strFileName);
// 
// 	m_bInitialized = TRUE;			// pwk 20130602 
// }

void CProbeHistoryManager::InitializeReport(HWND hParent, CRect r)
{
 	gf_Initialize();
// 
 //	AfxDaoInit();

	Create(NULL, NULL, WS_VISIBLE | WS_CHILD, CRect(0, 0, 0, 0), this, 1);

	// ProbeHistory Report 다이얼로그 생성.
	CreateProbeHistoryReportDlg(hParent, r);
}

//void CProbeHistoryManager::FinalizeProbeHistory()
//{
// 	// sih 20151005 add 
// 	if(!m_dlgProbeHistory)
// 		return;
// 	//
// 
// 	// 메시지루프 종료.
// 	ProbeHistory_DATA *pProbeHistory;
// 	int nCount = m_dlgProbeHistory.GetListCount();
// 
// 	for(int i=0; i<nCount; i++)
// 	{
// 		pProbeHistory = (ProbeHistory_DATA*)(m_dlgProbeHistory.m_listProbeHistory.GetItemData(i));
// 		pProbeHistory->nRet = ProbeHistory_RV_PROGRAMEXIT;
// 	}
//}

void CProbeHistoryManager::FinalizeReport()
{
//	AfxDaoTerm();

	//if (::IsWindow(m_pdlgReportProbeHistory->GetSafeHwnd()))
	//{
	//	m_pdlgReportProbeHistory->DestroyWindow();
	//}

	// 2023.03.08 change
	if(m_pdlgReportProbeHistory != nullptr)
	{
		m_pdlgReportProbeHistory->DestroyWindow();

		delete  m_pdlgReportProbeHistory;
		m_pdlgReportProbeHistory = nullptr;
	}
}


//////////////////////////////////////////////////////////////////////////
// 다이얼로그들 생성.
//void CProbeHistoryManager::CreateProbeHistoryDlg(HWND hParent)
//{
// 	if (!hParent) return;
// 	
// 	if (::IsWindow(m_dlgProbeHistory.GetSafeHwnd()))
// 	{
// 		m_dlgProbeHistory.DestroyWindow();
// 	}
// 	
// 	OleInitialize(NULL);
// 	
// 	m_dlgProbeHistory.Create(IDD_DLG_ProbeHistory, FromHandle(hParent));
// 	m_dlgProbeHistory.ShowWindow(SW_SHOW); 
//}

void CProbeHistoryManager::CreateProbeHistoryReportDlg(HWND hParent, CRect r)
{
	CRect rect;

	if (!hParent) return;
	
	if (m_pdlgReportProbeHistory != nullptr)
	{
		if (::IsWindow(m_pdlgReportProbeHistory->GetSafeHwnd()))
		{
			m_pdlgReportProbeHistory->DestroyWindow();
		}
	}

	m_pdlgReportProbeHistory = new CDlgReportProbeHistory(FromHandle(hParent));

	m_pdlgReportProbeHistory->Create(IDD_DLG_REPORT_ProbeHistory, FromHandle(hParent));

	m_pdlgReportProbeHistory->GetWindowRect(&rect);
	m_pdlgReportProbeHistory->MoveWindow(r.left + 1, r.top + 18, rect.Width(), rect.Height());	// jsyoon 20160226 add

	if (::IsWindow(m_pdlgReportProbeHistory->GetSafeHwnd()))
		m_pdlgReportProbeHistory->ShowWindow(SW_HIDE);

	
//	OleInitialize(NULL);
	//
	//m_dlgReportProbeHistory.Create(IDD_DLG_REPORT_ProbeHistory, FromHandle(hParent));

	//CRect rect;
	//m_dlgReportProbeHistory.GetWindowRect(rect);
	//m_dlgReportProbeHistory.MoveWindow(r.left,r.top, rect.Width(),rect.Height());	// jsyoon 20160226 add


	//m_dlgReportProbeHistory.ShowWindow(SW_HIDE);
//	m_dlgReportProbeHistory.ShowWindow(SW_SHOW);
}

//////////////////////////////////////////////////////////////////////////

CWnd* CProbeHistoryManager::GetReportProbeHistoryWnd()
{
	return m_pdlgReportProbeHistory;

//	return &m_dlgReportProbeHistory;
}

void CProbeHistoryManager::LoadProbeHistoryFile(CString strFileName /* = "ProbeHistory.csv" */)
{
	CFileFind finder;
	if (finder.FindFile(strFileName) == FALSE)
	{
		return;
	}
	
	CFile ProbeHistoryFile(strFileName, CFile::shareDenyNone );
	CArchive ar(&ProbeHistoryFile, CArchive::load);
	
	// ProbeHistory Table Load
	CString strLine;

	ar.ReadString(strLine);
	ar.ReadString(strLine);

	while(ar.ReadString(strLine))
	{
		if(strLine.Find(_T("[FILE END]")) >= 0)
		{
			break;
		}

		ProbeHistory_DATA ProbeHistoryData;
		ZeroMemory(&ProbeHistoryData, sizeof(ProbeHistoryData));

		int iEnd = 0;
		iEnd = strLine.Find(_T(","));			// id
		ProbeHistoryData.id = _ttoi(strLine.Mid(0, iEnd));
		strLine.Delete(0, iEnd+1);
		
		iEnd	= strLine.Find(_T(","));		// code
		ProbeHistoryData.code = _ttoi(strLine.Mid(0, iEnd));
		strLine.Delete(0, iEnd+1);

		iEnd	= strLine.Find(_T(","));		// level
		ProbeHistoryData.level = strLine.GetAt(0);
		strLine.Delete(0, iEnd+1);

		iEnd	= strLine.Find(_T(","));		// module
		_tcscpy_s(ProbeHistoryData.module, _countof(ProbeHistoryData.module), strLine.Mid(0, iEnd));
		strLine.Delete(0, iEnd+1);

		iEnd	= strLine.Find(_T(","));		// command
		_tcscpy_s(ProbeHistoryData.cmd, _countof(ProbeHistoryData.cmd), strLine.Mid(0, iEnd));
		strLine.Delete(0, iEnd+1);

		iEnd	= strLine.Find(_T(","));		// text
		_tcscpy_s(ProbeHistoryData.text, _countof(ProbeHistoryData.text), strLine.Mid(0, iEnd));
		strLine.Delete(0, iEnd+1);

		_tcscpy_s(ProbeHistoryData.Description, _countof(ProbeHistoryData.Description), strLine);

		// push ProbeHistory table
		m_ProbeHistoryTable.SetAt(ProbeHistoryData.id, ProbeHistoryData);
	}
	
	ar.Close();
	ProbeHistoryFile.Close();
}

BOOL CProbeHistoryManager::FindProbeHistoryTable(UINT id, ProbeHistory_DATA& ProbeHistory)
{
	BOOL bFind = m_ProbeHistoryTable.Lookup(id, ProbeHistory);

	return bFind;
}

// int CProbeHistoryManager::SetProbeHistory(UINT id)
// {
// 	if(!m_bInitialized)			// pwk 20130602
// 		return FALSE;
// 
// 	ProbeHistory_DATA ProbeHistory;
// 	int nRet = 0;
// 
// 	if(!FindProbeHistoryTable(id, ProbeHistory))
// 	{
// 		memset(&ProbeHistory, 0, sizeof(ProbeHistory));
// 		ProbeHistory.id = id;
// 		ProbeHistory.code = id;
// 		ProbeHistory.level = 'H';
// 		strcpy(ProbeHistory.module, "UNKNOWN");
//  		strcpy(ProbeHistory.cmd, "UNKNOWN");
//  		strcpy(ProbeHistory.text, "UNKNOWN_ProbeHistory");
// 	}
// 
// 	if(ProbeHistory.level == 'H')
// 	{
// 		nRet = SetHeavyProbeHistory(ProbeHistory);
// 	}
// 	else
// 	{
// 		nRet = SetLightProbeHistory(ProbeHistory);
// 	}
// 
// 	return nRet;
// }
// 

int CProbeHistoryManager::SetProbeHistoryInsert(CString strRFID,CString strMountTime)
{
	ProbeHistory_DATA ProbeHistory;
	int nRet = 0;
	
	memset(&ProbeHistory, 0, sizeof(ProbeHistory));
	_tcscpy_s(ProbeHistory.module,      _countof(ProbeHistory.module), strRFID);
	_tcscpy_s(ProbeHistory.happendTime, _countof(ProbeHistory.happendTime), strMountTime);
	
	nRet = SetProbeHistoryInsert(ProbeHistory);

	return nRet;
}

int CProbeHistoryManager::SetProbeHistoryUpdateContactCnt(CString strRFID, CString strMountTime,int nContactCount)
{
	ProbeHistory_DATA ProbeHistory;
	int nRet = 0;
	
	memset(&ProbeHistory, 0, sizeof(ProbeHistory));
	ProbeHistory.code = nContactCount;
	_tcscpy_s(ProbeHistory.module,      _countof(ProbeHistory.module), strRFID);
	_tcscpy_s(ProbeHistory.happendTime, _countof(ProbeHistory.happendTime), strMountTime);
	
	nRet = SetProbeHistoryUpdateContactCnt(ProbeHistory);

	
	
	return nRet;
}

int CProbeHistoryManager::SetProbeHistoryUpdateUnmountTime(CString strRFID, CString strMountTime,CString strUnmountTime)
{
	ProbeHistory_DATA ProbeHistory;
	int nRet = 0;
	
	memset(&ProbeHistory, 0, sizeof(ProbeHistory));
	_tcscpy_s(ProbeHistory.module, _countof(ProbeHistory.module), strRFID);
	_tcscpy_s(ProbeHistory.happendTime, _countof(ProbeHistory.happendTime), strMountTime);
	_tcscpy_s(ProbeHistory.clearedTime, _countof(ProbeHistory.clearedTime), strUnmountTime);

//	strcpy(ProbeHistory.module, strRFID);
//	strcpy(ProbeHistory.happendTime, strMountTime);
//	strcpy(ProbeHistory.clearedTime, strUnmountTime);
	
	nRet = SetProbeHistoryUpdateUnmountTime(ProbeHistory);

	return nRet;
}

// int CProbeHistoryManager::SetHeavyProbeHistory(ProbeHistory_DATA ProbeHistory)
// {
// 
// 	// 이미 발생된 알람이면 리턴.
// 	if(m_dlgProbeHistory.FindList(ProbeHistory.id))
// 	{
// 		return ProbeHistory_RV_SUCCESSED;
// 	}
// 
// 	// 알람 리턴값 초기화
// 	ProbeHistory.nRet = 0;
// 	strcpy(ProbeHistory.happendTime, gf_MakeCurTimeString());
// 
// 	// 리스트 컨트롤에 추가.
// 	m_dlgProbeHistory.InsertList(&ProbeHistory);
// 	
// 	// 알람 발생시 메시지 전송.	
// 	//::PostMessage(SMEM_GetMetrosysViewWnd(), WM_ProbeHistory_ACCURED, WPARAM(ProbeHistory.id), NULL); //lhj del 160621
// 
// 	// 메시지루프. 알람의 리턴값이 결정되면 빠져나온다.
// 	MSG msg;
// 	while(TRUE)
// 	{
// 		if (ProbeHistory.nRet > 0) 
// 		{
// 			break;
// 		}
// 		// jsyoon 20140409 change
// // 		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
// // 		{
// // 			TranslateMessage(&msg);
// // 			DispatchMessage(&msg);
// // 		}
// 		while(PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE))
// 		{
// 			if(!AfxGetApp()->PreTranslateMessage(&msg))		   // jsyoon 20131120
// 			{
// 				TranslateMessage(&msg);
// 				Sleep(0);					// jsyoon 20150224 change
// 				DispatchMessage(&msg);
// 				
// 				if (ProbeHistory.nRet > 0)		//이렇게 해 줘도 상관이 없는 것인지... 계속 모니터링 해 볼 것
// 					break;
// 				
// // 				Sleep(0);
// // 				DispatchMessage(&msg);
// 			}
// 			
// 		}
// 		Sleep(0);
// 
// 	}
// 
// 	if(ProbeHistory.nRet == ProbeHistory_RV_PROGRAMEXIT)
// 	{
// 		// 프로그램 종료시, 메시지루프 빠져나온후 종료메시지 날림.
// 		PostQuitMessage(0);
// 	}
// 	else
// 	{
// 		// 리스트컨트롤에서 삭제.
// 		m_dlgProbeHistory.DeleteList(ProbeHistory.id);
// 
// 		strcpy(ProbeHistory.clearedTime, gf_MakeCurTimeString());
// 		m_dlgReportProbeHistory.AddProbeHistoryHistory(&ProbeHistory);
// 
// 		// 알람 클리어시 메시지 전송.
// 		//::PostMessage(SMEM_GetMetrosysViewWnd(), WM_ProbeHistory_CLEARED, WPARAM(ProbeHistory.id), NULL); //lhj del 160621
// 	}
// 
// 	return ProbeHistory.nRet;
// }

int CProbeHistoryManager::SetProbeHistoryInsert(ProbeHistory_DATA ProbeHistory)
{
	int nRtn = 0;
	// 리스트 컨트롤에 추가.
	if(m_pdlgReportProbeHistory != nullptr)
		nRtn = m_pdlgReportProbeHistory->AddProbeHistoryInsert(&ProbeHistory); //lhj add 160621	
	return nRtn;
}

int CProbeHistoryManager::SetProbeHistoryUpdateContactCnt(ProbeHistory_DATA ProbeHistory)
{
	int nRtn = 0;
	// 리스트 컨트롤에 추가.
	if (m_pdlgReportProbeHistory != nullptr)
		nRtn = m_pdlgReportProbeHistory->AddProbeHistoryUpdateContactCnt(&ProbeHistory); //lhj add 160621
	return nRtn;
}

int CProbeHistoryManager::SetProbeHistoryUpdateUnmountTime(ProbeHistory_DATA ProbeHistory)
{
	int nRtn = 0;
	// 리스트 컨트롤에 추가.
	if (m_pdlgReportProbeHistory != nullptr)
		nRtn = m_pdlgReportProbeHistory->AddProbeHistoryUpdateContactUnmountTime(&ProbeHistory); //lhj add 160621
	return nRtn;
}

void CProbeHistoryManager::SetSavePath(CString strPath)
{
	if (m_pdlgReportProbeHistory != nullptr)
	{
		m_pdlgReportProbeHistory->SetSavePath(strPath);

		m_pdlgReportProbeHistory->UpdateProbeInfo();
	}
}

//lhj 20181225 Report Probe 
int CProbeHistoryManager::ProbeUpdateLanguage(HINSTANCE hResource_Lang_Dll)
{
	if (m_pdlgReportProbeHistory != nullptr)
		m_pdlgReportProbeHistory->UpdateLanguage(hResource_Lang_Dll);

	return 0;
}

// int CProbeHistoryManager::SetLightProbeHistory(ProbeHistory_DATA ProbeHistory)
// {
// 	if(m_pDlgWarning.FindList(ProbeHistory.id))
// 	{
// 		m_pDlgWarning.ShowWindow(SW_SHOW);
// 
// 		return ProbeHistory_RV_SUCCESSED;
// 	}
// 
// 	strcpy(ProbeHistory.happendTime, gf_MakeCurTimeString());
// 	strcpy(ProbeHistory.clearedTime, ProbeHistory.happendTime);
// 	m_dlgReportProbeHistory.AddProbeHistoryHistory(&ProbeHistory);
// 
// 	m_pDlgWarning.InsertList(&ProbeHistory);
// 	m_pDlgWarning.ShowWindow(SW_SHOW);
// 
// 	// Warning 발생시 메시지 전송.	
// 	//::PostMessage(SMEM_GetMetrosysViewWnd(), WM_WARNING_ACCURED, WPARAM(ProbeHistory.id), NULL); //lhj del 160621
// 
// 	return ProbeHistory_RV_SUCCESSED;
// }

// int CProbeHistoryManager::GetListCount()
// {
// 	if(!m_dlgProbeHistory)
// 		return 0;
// 	
// 	return m_dlgProbeHistory.GetListCount();
// }

