#include "pch.h"
#include "API_Result.h"

#include "ResultManager.h"

CResultManager g_cResultManager;

void __stdcall API_Result_Initialize(HWND hParentWnd, CRect r)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	g_cResultManager.InitializeResult(hParentWnd, r);
}

void __stdcall API_Result_FinalizeReport()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	g_cResultManager.FinalizeReport();
}

int __stdcall API_Result_Upload(PRSRITEMDATA result)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return g_cResultManager.UploadData(result);
}

bool __stdcall API_Result_SaveResult(bool bSaveFile, bool bRemote, CString strStart)
{
	bool bRet = false;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	bRet = g_cResultManager.SaveResult(bSaveFile, bRemote, strStart);

	return bRet;
}

bool __stdcall API_Result_SaveResultDone()
{
	bool bRet = false;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	bRet = g_cResultManager.SaveResultDone();

	return bRet;
}


void __stdcall API_Result_SetConfigPath(CString str)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	g_cResultManager.SetConfigPath(str);
}

void __stdcall API_Result_SetSearchMode(int iMode)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	g_cResultManager.SetSearchMode(iMode);
}

CWnd* __stdcall API_Result_GetReportmWnd()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return g_cResultManager.GetReportResultWnd();
}

void __stdcall API_Result_DataClear()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	g_cResultManager.DataClear();
}