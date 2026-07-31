// API_ProbeHistory.cpp : implementation file
//

#include "pch.h"
#include "ProbeHistory.h"
#include "API_ProbeHistory.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#include "ProbeHistoryManager.h"

CProbeHistoryManager g_cProbeHistoryManager;
/////////////////////////////////////////////////////////////////////////////
// API_OPN


// void WINAPI ProbeHistory_InitializeProbeHistory(HWND hParentWnd)
// {
// 	AFX_MANAGE_STATE(AfxGetStaticModuleState());
// 	
// 	g_cProbeHistoryManager.InitializeProbeHistory(hParentWnd);
// }

void WINAPI ProbeHistory_InitializeReport(HWND hParentWnd, CRect r)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	g_cProbeHistoryManager.InitializeReport(hParentWnd, r);
}

CWnd* WINAPI ProbeHistory_GetReportProbeHistoryWnd()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	return g_cProbeHistoryManager.GetReportProbeHistoryWnd();
}

// void WINAPI ProbeHistory_FinalizeProbeHistory()
// {
// 	AFX_MANAGE_STATE(AfxGetStaticModuleState());
// 	
// 	g_cProbeHistoryManager.FinalizeProbeHistory();
// }

void WINAPI ProbeHistory_FinalizeReport()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	g_cProbeHistoryManager.FinalizeReport();
}

int WINAPI ProbeHistory_SetProbeHistory(UINT id)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
//	return g_cProbeHistoryManager.SetProbeHistory(id);
	return 0;
}

// int	WINAPI	ProbeHistory_GetListCount()
// {
// 	AFX_MANAGE_STATE(AfxGetStaticModuleState());
// 
// 	return g_cProbeHistoryManager.GetListCount();
// }

int WINAPI ProbeHistory_Insert(CString strRFID,CString strMountTime)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	return g_cProbeHistoryManager.SetProbeHistoryInsert(strRFID, strMountTime);
}

int WINAPI ProbeHistory_Update_ContactCnt(CString strRFID, CString strMountTime,int nContactCount)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	return g_cProbeHistoryManager.SetProbeHistoryUpdateContactCnt(strRFID, strMountTime, nContactCount);
}

int WINAPI ProbeHistory_Update_UnmountTime(CString strRFID, CString strMountTime,CString strUnmountTime)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	return g_cProbeHistoryManager.SetProbeHistoryUpdateUnmountTime(strRFID, strMountTime, strUnmountTime);
}

void WINAPI ProbeHistory_SetSavePath(CString strPath)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	g_cProbeHistoryManager.SetSavePath(strPath);
}


int WINAPI Probe_Update_Language(HINSTANCE hResource_Lang_Dll) //lhj 20181225 lang Report Probe
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	int nRtn =0;

	g_cProbeHistoryManager.ProbeUpdateLanguage(hResource_Lang_Dll);

	return nRtn;
}