#pragma once
#include <afxmt.h>			//CCriticalSection
#include "locale.h"			//한글표시

#define WM_ALARM_ACCURED		(WM_USER + 1300)
#define WM_ALARM_CLEARED		(WM_USER + 1301)
#define WM_WARNING_ACCURED		(WM_USER + 1302)
#define WM_WARNING_CLEARED		(WM_USER + 1303)

#define ALARM_RV_ERR 100
#define ALARM_MAX_COUNT 9999

#define df_HOST_OFFLINE				0
#define	df_HOST_ONLINE_REMOTE		1
#define	df_HOST_ONLINE_LOCAL		2
#define	df_HOST_MAINT_MODE			3
#define	df_HOST_PM_MODE				4

#define dfDEFAULT_DB_SOURCE_PATH		_T("Config\\DB\\ReportSource.mdb")
#define dfDEFAULT_DB_SOURCE_PATH_NEW	_T("Config\\DB\\ReportSource.accdb") 

const enum ALARM_LEVEL_VALUE {
	ALARM_LEVLE_HIGH = 0,
	ALARM_LEVLE_LOW,
	ALARM_LEVLE_MSG,
};

const enum ALARM_RETURN_VALUE {
	ALARM_RV_SUCCESSED = 0,
	ALARM_RV_IGNORE,
	ALARM_RV_RETRY,
	ALARM_RV_INITRETRY,
	ALARM_RV_ABORT,
	ALARM_RV_PROGRAMEXIT,
	ALARM_RV_ALARM_NOT_EXIST = (ALARM_RV_ERR + 1),
};

struct ALARM_DATA
{
	UINT id;		// key
	UINT code;
	UINT nRet;
	UINT nRetry;		// Retry On, OFF
	UINT nType;			// Type(1 --> Danger for human, 2--> Machine Hardware Error, 3, 4...)
	UINT nInsertLine;

	UINT level;
	TCHAR module[256];
	TCHAR cmd[1024];
	TCHAR text[1024];
	TCHAR Description[1024];
	TCHAR Num[256];
	TCHAR happendTime[32];
	TCHAR clearedTime[32];
	
	ALARM_DATA()
	{
		id = 0;
		code = 0;
		nRet = 0;
		nRetry = 0;
		nType = 0;
		nInsertLine = 0;
		level = ALARM_LEVLE_HIGH;

		_tcscpy_s(module, _countof(module), _T("UNKNOWN"));
		_tcscpy_s(cmd, _countof(cmd), _T("UNKNOWN"));
		_tcscpy_s(text, _countof(text), _T("UNKNOWN"));
		_tcscpy_s(Description, _countof(Description), _T(""));
		_tcscpy_s(happendTime, _countof(happendTime), _T(""));
		_tcscpy_s(clearedTime, _countof(clearedTime), _T(""));
		_tcscpy_s(Num, _countof(Num), _T(""));
	}
};

struct ALARM_EVENT_DATA : public ALARM_DATA
{
	HANDLE hEvent;

	ALARM_EVENT_DATA()
	{
		hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);		// manual set, initial reset
	}
	~ALARM_EVENT_DATA()
	{
		if (hEvent)
		{
			CloseHandle(hEvent);
		}
	}

	void Copy(ALARM_DATA rhs)
	{
		id = rhs.id;
		code = rhs.code;
		level = rhs.level;
		nRetry = rhs.nRetry;
		nType = rhs.nType;
		nRet = rhs.nRet;

		_tcscpy_s(module, _countof(module), rhs.module);
		_tcscpy_s(cmd, _countof(cmd), rhs.cmd);
		_tcscpy_s(text, _countof(text), rhs.text);
		_tcscpy_s(Description, _countof(Description), rhs.Description);
		_tcscpy_s(happendTime, _countof(happendTime), rhs.happendTime);
		_tcscpy_s(clearedTime, _countof(clearedTime), rhs.clearedTime);
		_tcscpy_s(Num, _countof(Num), rhs.Num);
	}
};

struct ALARM_MESSAGE
{
	UINT code;
	UINT nType;			// Type(1 --> Danger for human, 2--> Machine Hardware Error, 3, 4...)
	CString use;
	CString text1;
	CString text2;

	ALARM_MESSAGE()
	{
		code = 0;
		nType = 0;
		use = _T("E");
		text1 = _T("Unknown Alarm");
		text2 = _T("Unknown Alarm");
	}
};

extern CString			g_sAppPath;
extern CCriticalSection	g_csQuery;
extern void gf_Initialize();
extern BOOL gf_MakeDir(CString sPath);
extern CString gf_MakeCurTimeString();