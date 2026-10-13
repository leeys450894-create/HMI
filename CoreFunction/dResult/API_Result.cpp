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

// LYS 2026-10-06 Hybrid Phase 5 resume replay: fast upload switch - the monitor thread skips the 1 s display wait only while on
void __stdcall API_Result_SetFastUpload(bool bOn)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	g_cResultManager.SetFastUpload(bOn);
}

// LYS 2026-10-06 Hybrid Phase 5 resume replay: rows waiting in the shared ring (*pQueue) / rows the monitor thread finished (*pDone, monotonic)
void __stdcall API_Result_GetUploadState(int* pQueue, long* pDone)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	g_cResultManager.GetUploadState(pQueue, pDone);
}