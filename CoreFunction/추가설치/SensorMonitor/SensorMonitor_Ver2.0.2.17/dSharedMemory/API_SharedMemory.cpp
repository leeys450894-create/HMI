#include "stdafx.h"
#include "API_SharedMemory.h"
#include "dSharedMemory.h"
#include <tlhelp32.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//FFU
void WINAPI API_SM_SetFFU_PV(int iFFUID, int iPV, int nIdex )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	g_pMapView[nIdex]->stFFUMon.iEDUID[iFFUID -1] = iFFUID;
	g_pMapView[nIdex]->stFFUMon.iPV[iFFUID-1] = iPV;
}

void WINAPI API_SM_SetFFU_ALARM(int iFFUID, int iALARM, int nIdex)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	g_pMapView[nIdex]->stFFUMon.iEDUID[iFFUID -1] = iFFUID;
	g_pMapView[nIdex]->stFFUMon.iALARM[iFFUID-1] = iALARM;
}

void WINAPI API_SM_SetFFU_SV(int iFFUID, int iSV, int nIdex)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	g_pMapView[nIdex]->stFFUMon.iEDUID[iFFUID -1] = iFFUID;
	g_pMapView[nIdex]->stFFUMon.iSV[iFFUID-1] = iSV;
}

int WINAPI API_SM_GetFFU_PV(int iFFUID, int nIdex)
{
	int iPV = 0;
	iPV = g_pMapView[nIdex]->stFFUMon.iPV[iFFUID-1];
	
	return iPV;
}

int WINAPI API_SM_GetFFU_ALARM(int iFFUID, int nIdex)
{
	int iALARM = 0;
	iALARM = g_pMapView[nIdex]->stFFUMon.iALARM[iFFUID-1];
	
	return iALARM;
}

int WINAPI API_SM_GetFFU_SV(int iFFUID, int nIdex)
{
	int iSV = 0;
	iSV = g_pMapView[nIdex]->stFFUMon.iSV[iFFUID-1];
	
	return iSV;
}


BOOL WINAPI API_SM_GetFFU_ALARM_CHECK(int iFFUID, int nIdex)
{
	int iALARM = 0;
	
	BOOL bRet = FALSE;
	CString strLog;
	
	iALARM = g_pMapView[nIdex]->stFFUMon.iALARM[iFFUID-1];
	
	switch (iALARM)
	{
	case NORMAL_ALARM: //NO ALARM
		bRet = FALSE;
		break;
	case EC_ALARM: //Excess Current
		bRet = TRUE;
		break;
	case MOTOR_ALARM: //Motor
		bRet = TRUE;
		break;
	case COMM_ALARM: //Communication
		bRet = TRUE;
		break;
	default:
		bRet = FALSE;
		break;
	}
	
	return bRet;
}

//20140721 added by Jay Hong
void WINAPI API_SM_SetFFU_ALARM_CHECK(int iFFUID, int nValue, int nIdex)
{
	g_pMapView[nIdex]->stFFUMon.iALARM[iFFUID] = nValue;
}


void WINAPI API_SM_SetFFU_ALLOFF(BOOL bAllOff, int nIdex)
{
	g_pMapView[nIdex]->stFFUMon.bAllOff = bAllOff;
}

void WINAPI API_SM_SetFFU_RESTORE(BOOL bRestore, int nIdex)
{
	g_pMapView[nIdex]->stFFUMon.bRestore = bRestore;
}

BOOL WINAPI API_SM_GetFFU_ALLOFF(int nIdex)
{
	 return g_pMapView[nIdex]->stFFUMon.bAllOff;
}

BOOL WINAPI API_SM_GetFFU_RESTORE(int nIdex)
{
	 return g_pMapView[nIdex]->stFFUMon.bRestore;
}

void WINAPI API_SM_SetFFU_NUM(int nFFU, int nIdex)
{
	g_pMapView[nIdex]->stFFUMon.nFFU = nFFU;
}

int WINAPI API_SM_GetFFU_NUM(int nIdex)
{
	int iSV = 0;
	iSV = g_pMapView[nIdex]->stFFUMon.nFFU;
	return iSV;
}

int WINAPI API_SM_GetFFU_LiveCheck(int nIdex)
{
	int iSV = 0;
	iSV = g_pMapView[nIdex]->stFFUMon.strTime;
	return iSV;
}

void WINAPI API_SM_SetFFU_LiveCheck(int iTime, int nIdex)
{
	g_pMapView[nIdex]->stFFUMon.strTime = iTime;
}

//Head Position
void WINAPI API_SM_SetHEAD_PosX(int iHead, int iPosX, int nIdex)
{
	g_pMapView[nIdex]->stHeadMon[iHead].iPositionX = iPosX;
}

int WINAPI API_SM_GetHEAD_PosX(int iHead, int nIdex)
{
	return g_pMapView[nIdex]->stHeadMon[iHead].iPositionX;
}

void WINAPI API_SM_SetHEAD_PosY(int iHead, int iPosY, int nIdex)
{
	g_pMapView[nIdex]->stHeadMon[iHead].iPositionY = iPosY;
}

int WINAPI API_SM_GetHEAD_PosY(int iHead, int nIdex)
{
	return g_pMapView[nIdex]->stHeadMon[iHead].iPositionY;
}

void WINAPI API_SM_SetHEAD_PosZ(int iHead, int iPosZ, int nIdex)
{
	g_pMapView[nIdex]->stHeadMon[iHead].iPositionZ = iPosZ;
}

int WINAPI API_SM_GetHEAD_PosZ(int iHead, int nIdex)
{
	return g_pMapView[nIdex]->stHeadMon[iHead].iPositionZ;
}

void WINAPI API_SM_SetHEAD_PosSubX(int iHead, int iPosSubX, int nIdex)
{
	g_pMapView[nIdex]->stHeadMon[iHead].iPositionSubX = iPosSubX;
}

int WINAPI API_SM_GetHEAD_PosSubX(int iHead, int nIdex)
{
	return g_pMapView[nIdex]->stHeadMon[iHead].iPositionSubX;
}

//PLC Data
void WINAPI API_SM_SetPLC_PLCData_M(int iAddr, BOOL bSet, int nIdex)
{
	if( iAddr >= 0)
	{
		if ( iAddr >= 4000)
			g_pMapView[nIdex]->stPLCMon.bPLCData_M[iAddr-4000] = bSet;
		else
			g_pMapView[nIdex]->stPLCMon.bPLCData_M[iAddr] = bSet;
	}
}

BOOL WINAPI API_SM_GetPLC_PLCData_M(int iAddr, int nIdex)
{
	if( iAddr >= 0)
	{
		if ( iAddr >= 4000)
			return g_pMapView[nIdex]->stPLCMon.bPLCData_M[iAddr-4000];
		else
			return g_pMapView[nIdex]->stPLCMon.bPLCData_M[iAddr];
	}
	else
		return FALSE;
}

void WINAPI API_SM_SetPLC_PLCData_F(int iAddr, BOOL bSet, int nIdex)
{
	if( iAddr >= 0)
	{
		g_pMapView[nIdex]->stPLCMon.bPLCData_F[iAddr] = bSet;
	}
}

BOOL WINAPI API_SM_GetPLC_PLCData_F(int iAddr, int nIdex)
{
	if( iAddr >= 0)
		return g_pMapView[nIdex]->stPLCMon.bPLCData_F[iAddr];
	else
		return FALSE;
}

BOOL* WINAPI API_SM_GetPLC_ALL_M(int nIdex)
{
	return g_pMapView[nIdex]->stPLCMon.bPLCData_M;
}

BOOL* WINAPI API_SM_GetPLC_ALL_F(int nIdex)
{
	return g_pMapView[nIdex]->stPLCMon.bPLCData_F;
}

void WINAPI API_SM_SetPLC_ALL(int idex, BOOL* bAddr, int nIdex)
{
	BOOL* bPLC = bAddr;
	
	if( idex == 0)	// M Address
	{
		for(int i = 0; i < MAX_PLC_M; i++)
			g_pMapView[nIdex]->stPLCMon.bPLCData_M[i] = bPLC[i];
	}
	else			// F Address
	{
		for(int i = 0; i < MAX_PLC_F; i++)
			g_pMapView[nIdex]->stPLCMon.bPLCData_F[i] = bPLC[i];
	}	
}

// SeparatePLCPort.exe : PLC Program Name
// PLC 프로그램 실행 여부
BOOL WINAPI API_SM_GetPLC_Check()
{
	CString szProcessName = _T("SeparatePLCPort.exe");
	BOOL bRes = FALSE;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	PROCESSENTRY32 sEntry32;
	sEntry32.dwSize = sizeof(PROCESSENTRY32); // 이거 빠지면 검색 잘 안됨
	
	if((hSnapshot == INVALID_HANDLE_VALUE) || (hSnapshot == NULL)) return FALSE;
	
	int iProcess = 0;
	BOOL bRemain = Process32First(hSnapshot, &sEntry32);
	while(bRemain)
	{
		CString szExeName;
		szExeName.Format(_T("%s"), sEntry32.szExeFile);
		
		if(szExeName.CompareNoCase(szProcessName) == 0) 
		{
			iProcess ++;
		}
		
		bRemain = Process32Next(hSnapshot, &sEntry32);
	}
	
	if(iProcess >= 1) //Running
	{
		CloseHandle(hSnapshot);
		bRes =  TRUE;
	}
	else // Not Running
	{
		CloseHandle(hSnapshot);
		bRes = FALSE;
	}

	return bRes;
}

