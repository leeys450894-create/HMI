// API_MCC.cpp : implementation file
//

#include "pch.h"
#include "dMCCSpec.h"
#include "API_MCC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#include "MCCManger.h"

CMCCManger g_cMCCManager;
/////////////////////////////////////////////////////////////////////////////
// API_OPN


void WINAPI API_MCC_Initialize(HWND hParentWnd, CRect r)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	g_cMCCManager.InitializeMCC(hParentWnd, r);
}

void WINAPI API_MCC_KillThred()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	g_cMCCManager.KillThred();
}

void WINAPI API_MCC_FinalizeReport()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	g_cMCCManager.Finalize();
}

void WINAPI	API_MCC_SetLogPath(CString str)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	g_cMCCManager.SetLogPath(str);
}

void WINAPI	API_MCC_SetConfigPath(CString str)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	g_cMCCManager.SetConfigPath(str);
}

void WINAPI	API_MCC_SetFilePath(CString str)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	g_cMCCManager.SetFilePath(str);
}


CWnd* WINAPI API_MCC_GetReportmWnd()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return g_cMCCManager.GetReportMCCWnd();
}

int WINAPI API_MCC_AddQueue(int iFlag)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return g_cMCCManager.AddMccQueue(iFlag);
}

VTM_MCC* WINAPI API_MCCR_GetSharedMemory()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return g_cMCCManager.GetSharedMemory();
}

void WINAPI	API_MCC_SetEQName(CString str)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	g_cMCCManager.SetEQName(str);
}
