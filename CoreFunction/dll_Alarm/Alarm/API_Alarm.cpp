// API_Alarm.cpp : implementation file
//

#include "pch.h"
#include "Alarm.h"
#include "API_Alarm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#include "AlarmManager.h"

CAlarmManager* g_pcAlarmManager = nullptr;
static CCriticalSection g_csAlarmManager;
/////////////////////////////////////////////////////////////////////////////
// API_OPN


void WINAPI Alarm_InitializeAlarm(HWND hParentWnd, CRect r)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	g_pcAlarmManager = new CAlarmManager;
	
	g_pcAlarmManager->InitializeAlarm(hParentWnd, r);
}

void WINAPI Alarm_InitializeReport(HWND hParentWnd, CRect r)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	g_pcAlarmManager->InitializeReport(hParentWnd,r);
}

CWnd* WINAPI Alarm_GetReportAlarmWnd()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	return g_pcAlarmManager->GetReportAlarmWnd();
}

void WINAPI Alarm_FinalizeAlarm()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	g_pcAlarmManager->FinalizeAlarm();

	CSingleLock lock(&g_csAlarmManager, TRUE); // LYS 2026-07-06 락 획득 후 null 처리
	delete g_pcAlarmManager;
	g_pcAlarmManager = nullptr;
}

void WINAPI Alarm_FinalizeReport()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	g_pcAlarmManager->FinalizeReport();
}

int WINAPI Alarm_SetAlarm(UINT id)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	if (g_pcAlarmManager == nullptr)
		return FALSE; // LYS 2026-06-30 종료 중이므로 무시

	return g_pcAlarmManager->SetAlarm(id);
}

int	WINAPI	Alarm_GetListCount()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CSingleLock lock(&g_csAlarmManager, TRUE); // LYS 2026-07-06 null 체크 + 호출을 원자적으로
	if (g_pcAlarmManager == nullptr)
		return 0; // LYS 2026-06-30 방식과 동일하게 처리
	return g_pcAlarmManager->GetListCount();
}

int WINAPI Alarm_ExcelFile()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	g_pcAlarmManager->ReloadAlarmList();
	//g_pcAlarmManager->LoadAlarmFile(_T(""));
	return 1;
}

void WINAPI Alarm_SetAlarmView(HWND hWnd)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	g_pcAlarmManager->SetAlarmView(hWnd);
}

void WINAPI Alarm_SetHostMode(int iMode)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	g_pcAlarmManager->SetHostMode(iMode);
}

void WINAPI Alarm_SetSavePath(TCHAR* pPath)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	g_pcAlarmManager->SetHostSavePath(pPath);
}

void WINAPI Alarm_ClearAlarm()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	g_pcAlarmManager->ClearAlarm();
}

void WINAPI Alarm_GetMessage(int i, TCHAR* pChar)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	g_pcAlarmManager->GetMeassgeList(i, pChar);
}

void WINAPI Alarm_GetText(int i, TCHAR* pChar)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	g_pcAlarmManager->GetAlarmText(i, pChar);
}

//lhj 20181225 lang Report Alarm
int WINAPI Alarm_Update_Language(HINSTANCE hResource_Lang_Dll)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	int nRtn =0;

	g_pcAlarmManager->AlarmUpdateLanguage(hResource_Lang_Dll);

	return nRtn;
}

CWnd* WINAPI Alarm_GetAlarmWnd()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	return g_pcAlarmManager->GetAlarmWnd();
}