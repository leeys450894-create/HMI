// AlarmDefine.cpp : implementation file
//

#include "pch.h"
#include "AlarmDefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CString				g_sAppPath;
CCriticalSection	g_csQuery;
CCriticalSection	g_csDir;
CCriticalSection	g_csTime;

/////////////////////////////////////////////////////////////////////////////
// CommonFunc

void gf_Initialize()
{
	TCHAR szBuff[_MAX_PATH];
	memset(szBuff, 0, MAX_PATH);
	::GetModuleFileName(::AfxGetApp()->m_hInstance, szBuff, _MAX_PATH);
	TCHAR* pch = _tcsrchr(szBuff, _T('\\'));
	g_sAppPath = CString(szBuff, pch - szBuff + 1);
	//	g_sAppPath = CString(szBuff, pch - szBuff + 1);

	setlocale(LC_ALL, "");	// 한글표시
}

BOOL gf_MakeDir(CString sPath)
{
	if (sPath.IsEmpty())
		return FALSE;

	g_csDir.Lock();
	if (sPath[1] != ':' || sPath[2] != '\\')
	{
		g_csDir.Unlock();
		return FALSE;
	}

	CString sTem;
	int nFind = 2;
	while ((nFind = sPath.Find(_T("\\"), nFind + 1)) >= 0)
	{
		sTem = sPath.Left(nFind);
		_tmkdir(sTem);
	}

	g_csDir.Unlock();
	return TRUE;
}

CString gf_MakeCurTimeString()
{
	g_csTime.Lock();
	CString sTime;
	SYSTEMTIME st;
	::GetLocalTime(&st);
	sTime.Format(_T("%04d.%02d.%02d-%02d:%02d:%02d:%03d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	g_csTime.Unlock();
	return sTime;
}