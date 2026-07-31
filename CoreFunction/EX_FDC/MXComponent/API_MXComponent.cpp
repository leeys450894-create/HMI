// API_MXComponent.cpp: implementation of the API_MXComponent class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MXComponent.h"
#include "API_MXComponent.h"
#include "MXComponentCtrl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/*===========================================================================================
Function : MXComponent Interface DLL Module CPP
Using	 : MXComponent
Date : 2018.03.16
Added by:sy.kim
===========================================================================================*/


CMXComponentCtrl* g_pMXComponentCtrl[5] = {NULL,NULL,NULL,NULL,NULL};
BOOL g_bMXComponentIni = FALSE;

long WINAPI API_MXComponent_Initialize(UINT index, int iStationNum, HWND hParentWnd)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	AfxEnableControlContainer();

// 	if(g_bMXComponentIni == MX_COMPONENT_SUCCESS)
// 		return MX_COMPONENT_SUCCESS;
	long bRet = MX_COMPONENT_SUCCESS;
	 
	if(g_pMXComponentCtrl[index] == NULL)
		g_pMXComponentCtrl[index] = new CMXComponentCtrl;

	if( g_pMXComponentCtrl[index]->Is_Connact() != TRUE)
		bRet = g_pMXComponentCtrl[index]->Initialize(iStationNum, hParentWnd);

	if(bRet == MX_COMPONENT_SUCCESS)
		g_bMXComponentIni = MX_COMPONENT_SUCCESS;
	else
		g_bMXComponentIni = FALSE;
		
	return bRet;
}

long WINAPI API_MXComponent_Close(UINT index)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if(g_pMXComponentCtrl[index] == NULL)
		return MX_COMPONENT_NOT_INITIALIZE;

	long bRet = MX_COMPONENT_SUCCESS;
	
	bRet = g_pMXComponentCtrl[index]->Close();

	g_bMXComponentIni = FALSE;

	delete g_pMXComponentCtrl[index];
	g_pMXComponentCtrl[index] = NULL;
	
	return bRet;
}

CString WINAPI API_MXComponent_GetErrorText(UINT index, long lErrorCode)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	CString strErrorText;
	strErrorText.Empty();
	
	strErrorText = g_pMXComponentCtrl[index]->GetErrorText(lErrorCode);
	
	return strErrorText;
}

long WINAPI API_MXComponent_GetMemoryBit(UINT index, CString strDeviceText, long* lplData)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if(g_bMXComponentIni == FALSE)
		return MX_COMPONENT_NOT_INITIALIZE;

	long bRet = MX_COMPONENT_SUCCESS;
	
	bRet = g_pMXComponentCtrl[index]->GetMemoryBit(strDeviceText, lplData);
	
	return bRet;
}

long WINAPI API_MXComponent_SetMemoryBit(UINT index, CString strDeviceText, BOOL bSet)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if(g_bMXComponentIni == FALSE)
		return MX_COMPONENT_NOT_INITIALIZE;
	
	long bRet = MX_COMPONENT_SUCCESS;
	
	bRet = g_pMXComponentCtrl[index]->SetMemoryBit(strDeviceText, bSet);
	
	return bRet;
}

long WINAPI API_MXComponent_SetMemoryBitBlock(UINT index, CString strDeviceText, long lSize, BOOL bSet)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	if(g_bMXComponentIni == FALSE)
		return MX_COMPONENT_NOT_INITIALIZE;
	
	long bRet = MX_COMPONENT_SUCCESS;
	
	bRet = g_pMXComponentCtrl[index]->SetMemoryBitBlock(strDeviceText, lSize, bSet);
	
	return bRet;
}

long WINAPI API_MXComponent_SetMemoryBitBlock2(UINT index, CString strDeviceText, long lSize, long* lplData)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if(g_bMXComponentIni == FALSE)
		return MX_COMPONENT_NOT_INITIALIZE;

	long bRet = MX_COMPONENT_SUCCESS;

	bRet = g_pMXComponentCtrl[index]->SetMemoryBitBlock2(strDeviceText, lSize, lplData);

	return bRet;
}

long WINAPI API_MXComponent_GetMemoryBitBlock(UINT index, CString strDeviceText, long lSize, long* lplBitData)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	if(g_bMXComponentIni == FALSE)
		return MX_COMPONENT_NOT_INITIALIZE;
	
	long bRet = MX_COMPONENT_SUCCESS;
	
	bRet = g_pMXComponentCtrl[index]->GetMemoryBitBlock(strDeviceText, lSize, lplBitData);
	
	return bRet;
}

long WINAPI API_MXComponent_GetMemoryWordBlock(UINT index, CString strDeviceText, long lSize, float* fpData, UINT nSize)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

// 	if(g_bMXComponentIni == FALSE)
// 		return MX_COMPONENT_NOT_INITIALIZE;
// 	
	long bRet = MX_COMPONENT_SUCCESS;

	if( g_pMXComponentCtrl[index] != NULL )
		bRet = g_pMXComponentCtrl[index]->GetMemoryWordBlock(strDeviceText, lSize, fpData, nSize);
	else
		bRet = MX_COMPONENT_NOT_INITIALIZE;
	
	return bRet;
}

long WINAPI API_MXComponent_SetMemoryWordBlock(UINT index, CString strDeviceText, long lSize, float* fpData, UINT nSize)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

// 	if(g_bMXComponentIni == FALSE)
// 		return MX_COMPONENT_NOT_INITIALIZE;
	
	long bRet = MX_COMPONENT_SUCCESS;
	
	if( g_pMXComponentCtrl[index] != NULL )
		bRet = g_pMXComponentCtrl[index]->SetMemoryWordBlock(strDeviceText, lSize, fpData, nSize);
	
	return bRet;
}

long WINAPI API_MXComponent_GetMemoryWordBlock2(UINT index, CString strDeviceText, long lSize, TCHAR* tpData)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// 	if(g_bMXComponentIni == FALSE)
	// 		return MX_COMPONENT_NOT_INITIALIZE;
	// 	
	long bRet = MX_COMPONENT_SUCCESS;

	if( g_pMXComponentCtrl[index] != NULL )
		bRet = g_pMXComponentCtrl[index]->GetMemoryWordBlock2(strDeviceText, lSize, tpData);

	return bRet;
}

long WINAPI API_MXComponent_SetMemoryWordBlock2(UINT index, CString strDeviceText, long lSize, TCHAR* tpData)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// 	if(g_bMXComponentIni == FALSE)
	// 		return MX_COMPONENT_NOT_INITIALIZE;

	long bRet = MX_COMPONENT_SUCCESS;

	if( g_pMXComponentCtrl[index] != NULL )
		bRet = g_pMXComponentCtrl[index]->SetMemoryWordBlock2(strDeviceText, lSize, tpData);

	return bRet;
}

long WINAPI API_MXComponent_GetMemoryWordBlock3(UINT index, CString strDeviceText, long lSize, long* lpData)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	long bRet = MX_COMPONENT_SUCCESS;

	if( g_pMXComponentCtrl[index] != NULL )
		bRet = g_pMXComponentCtrl[index]->GetMemoryWordBlock3(strDeviceText, lSize, lpData);

	return bRet;
}

long WINAPI API_MXComponent_SetMemoryWordBlock3(UINT index, CString strDeviceText, long lSize, long* lpData, UINT nSize)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	long bRet = MX_COMPONENT_SUCCESS;

	if( g_pMXComponentCtrl[index] != NULL )
		bRet = g_pMXComponentCtrl[index]->SetMemoryWordBlock3(strDeviceText, lSize, lpData, nSize);

	return bRet;
}