#if !defined(AFX_API_ALARM_H__D17C24DF_1D38_489C_B7B3_634EC901EE48__INCLUDED_)
#define AFX_API_ALARM_H__D17C24DF_1D38_489C_B7B3_634EC901EE48__INCLUDED_

#if _MSC_VER > 1000
#pragma once
//#include "AlarmIDList.h"
#endif // _MSC_VER > 1000
// API_Alarm.h : header file
//

//#define SAFE_DELETE(ptr)		if (ptr)	{ delete ptr;	ptr = NULL;	}
//#define SAFE_DELETE_AR(ptr)		if (ptr)	{ delete [] ptr;	ptr = NULL;	}



void WINAPI Alarm_FinalizeAlarm();
void WINAPI Alarm_FinalizeReport();
void WINAPI Alarm_InitializeAlarm(HWND hParentWnd, CRect r);
void WINAPI Alarm_InitializeReport(HWND hParentWnd, CRect r);
CWnd* WINAPI Alarm_GetReportAlarmWnd();
int	WINAPI Alarm_SetAlarm(UINT id);
int	WINAPI Alarm_GetListCount();
int WINAPI Alarm_ExcelFile();
void WINAPI Alarm_SetAlarmView(HWND hWnd);
void WINAPI Alarm_SetHostMode(int iMode);
void WINAPI Alarm_SetSavePath(TCHAR* pPath);
void WINAPI Alarm_ClearAlarm();
void WINAPI Alarm_GetMessage(int i, TCHAR* pChar);
void WINAPI Alarm_GetText(int i, TCHAR* pChar);
CWnd* WINAPI Alarm_GetAlarmWnd();

int WINAPI Alarm_Update_Language(HINSTANCE hResource_Lang_Dll); //lhj 20181225 lang Report Alarm


#endif // !defined(AFX_API_ALARM_H__D17C24DF_1D38_489C_B7B3_634EC901EE48__INCLUDED_)
