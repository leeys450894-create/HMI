// API_SACC.cpp: 구현 파일
//

#include "pch.h"
//#include "stdafx.h"
#include "dSACC.h"
#include "API_SACC.h"
#include "CDlgSACC.h"

#include "SACCManger.h"

CSACCManger* g_cSACCManager = nullptr;

// API_SACC 메시지 처리기
void WINAPI API_SACC_Initialize(HWND hParentWnd, CRect r)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (g_cSACCManager == nullptr)
	{
		g_cSACCManager = new CSACCManger();
	}

	if (g_cSACCManager != nullptr)
		g_cSACCManager->InitializeSACC(hParentWnd, r);
}

void WINAPI API_SACC_Finalize()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (g_cSACCManager != nullptr)
	{
		g_cSACCManager->Finalize();

		delete g_cSACCManager;
		g_cSACCManager = nullptr;
	}
}


CWnd* __stdcall API_SACC_GetReportmWnd()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (g_cSACCManager != nullptr)
		return g_cSACCManager->GetReportSACCWnd();
	else
		return nullptr;
}

void WINAPI API_SACC__Updata_Data(PDITEMDATA data)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (g_cSACCManager != nullptr)
		return g_cSACCManager->Updata(data);
}


void WINAPI API_SACC_GetProbeInfo(int iProbe, CString* pstrRFID, CString* pstrMountTime, int* pnContactCount)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (g_cSACCManager != nullptr)
		g_cSACCManager->GetProbeInfo(iProbe, pstrRFID, pstrMountTime, pnContactCount);
}

void WINAPI API_SACC_SetProbeInfo(int iProbe, CString strRFID, CString strMountTime, int nContactCount)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (g_cSACCManager != nullptr)
		g_cSACCManager->SetProbeInfo(iProbe, strRFID, strMountTime, nContactCount);
}