#if !defined(AFX_API_ProbeHistory_H__D17C24DF_1D38_489C_B7B3_634EC901EE48__INCLUDED_)
#define AFX_API_ProbeHistory_H__D17C24DF_1D38_489C_B7B3_634EC901EE48__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// API_ProbeHistory.h : header file
//

#define SAFE_DELETE(ptr)		if (ptr)	{ delete ptr;	ptr = NULL;	}
#define SAFE_DELETE_AR(ptr)		if (ptr)	{ delete [] ptr;	ptr = NULL;	}

#define WM_ProbeHistory_ACCURED		(WM_USER + 1300)
#define WM_ProbeHistory_CLEARED		(WM_USER + 1301)
#define WM_WARNING_ACCURED		(WM_USER + 1302)
#define WM_WARNING_CLEARED		(WM_USER + 1303)

#define ProbeHistory_RV_ERR 100

const enum ProbeHistory_RETURN_VALUE{
	ProbeHistory_RV_SUCCESSED		= 0,
	ProbeHistory_RV_IGNORE			,
	ProbeHistory_RV_RETRY			,
	ProbeHistory_RV_INITRETRY		,
	ProbeHistory_RV_ABORT			,
	ProbeHistory_RV_PROGRAMEXIT	,
	ProbeHistory_RV_ProbeHistory_NOT_EXIST = (ProbeHistory_RV_ERR + 1),
};

//void WINAPI ProbeHistory_FinalizeProbeHistory();
void WINAPI ProbeHistory_FinalizeReport();
//void WINAPI ProbeHistory_InitializeProbeHistory(HWND hParentWnd);
void WINAPI ProbeHistory_InitializeReport(HWND hParentWnd, CRect r);
CWnd* WINAPI ProbeHistory_GetReportProbeHistoryWnd();
int	WINAPI ProbeHistory_SetProbeHistory(UINT id);
//int	WINAPI ProbeHistory_GetListCount();
int	WINAPI ProbeHistory_Insert(CString strRFID, CString strMountTime);
int	WINAPI ProbeHistory_Update_ContactCnt(CString strRFID, CString strMountTime,int nContactCount);
int WINAPI ProbeHistory_Update_UnmountTime(CString strRFID, CString strMountTime,CString strUnmountTime);
void WINAPI ProbeHistory_SetSavePath(CString strPath);

int WINAPI Probe_Update_Language(HINSTANCE hResource_Lang_Dll); //lhj 20181225 lang Report Probe

struct ProbeHistory_DATA
{
	UINT id;		// key
	UINT code;
	TCHAR level;
	TCHAR module[256];
	TCHAR cmd[256];
	TCHAR text[256];
	TCHAR Description[1024];

	UINT nRet;
	TCHAR happendTime[32];
	TCHAR clearedTime[32];
};

#endif // !defined(AFX_API_ProbeHistory_H__D17C24DF_1D38_489C_B7B3_634EC901EE48__INCLUDED_)
