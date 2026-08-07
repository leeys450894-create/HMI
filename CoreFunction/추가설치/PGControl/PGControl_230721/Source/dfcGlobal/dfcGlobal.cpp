#include "pch.h"
#include "dfcGlobal.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifndef	IDS_FONTNAME
#define	IDS_FONTNAME	_T("맑은 고딕")
#endif

#ifndef	IDS_ELAPSEDTIME1
#define	IDS_ELAPSEDTIME1	_T("경과시간: %.1lf초")
#endif

#ifndef	IDS_ELAPSEDTIME2
#define	IDS_ELAPSEDTIME2	_T("경과시간: %d분 %.1lf초")
#endif

__declspec (dllexport)
void	dfcThrow (LPCTSTR fmt, ...)
{
	CString	string;
	if (fmt != 0) {
		va_list	argptr;
		va_start (argptr, fmt);
		string.FormatV (fmt, argptr);
		va_end (argptr);
	}

	throw string;
}

__declspec (dllexport)
void	dfcThrowEx (UINT nID, ...)
{
	CString	fmt;
	fmt.LoadString (nID);

	int		nPos = fmt.ReverseFind ('\n');
	if (nPos != -1) {
		fmt = fmt.Mid (nPos + 1);
	}

	CString	string;
	va_list	argptr;
	va_start (argptr, nID);
	string.FormatV (fmt, argptr);
	va_end (argptr);

	throw string;
}

__declspec (dllexport)
void	dfcThrowEx (LPCTSTR fmt, ...)
{
	CString	string;
	if (fmt != 0) {
		va_list	argptr;
		va_start (argptr, fmt);
		string.FormatV (fmt, argptr);
		va_end (argptr);
	}

	throw string;
}

__declspec (dllexport)
CString	dfcStr (LPCTSTR fmt, ...)
{
	CString	string;
	if (fmt != 0) {
		va_list	argptr;
		va_start (argptr, fmt);
		string.FormatV (fmt, argptr);
		va_end (argptr);
	}
	return string;
}

__declspec (dllexport)
CString	dfcStr (int nVal)
{
	CString	string;
	string.Format (_T("%d"), nVal);
	return string;
}

__declspec (dllexport)
CString	dfcStr (double fVal, BOOL bTrim)
{
	CString	string;
	string.Format (_T("%.4lf"), fVal);

	if (bTrim) {
		string.TrimRight ('0');
		string.TrimRight ('.');
	}
	return string;
}

__declspec (dllexport)
CString	dfcStr (CWnd& wnd)
{
	CString	string;
	wnd.GetWindowText (string);
	return string;
}

__declspec (dllexport)
CString	dfcStrEx (UINT nID, ...)
{
	CString	fmt;
	fmt.LoadString (nID);

	if (fmt.Find (_T('\n')) == 0) {
		fmt = fmt.Mid (1);
	}

	CString	string;
	va_list	argptr;
	va_start (argptr, nID);
	string.FormatV (fmt, argptr);
	va_end (argptr);
	return string;
}

__declspec (dllexport)
CString	dfcStrEx (LPCTSTR fmt, ...)
{
	CString	string;
	if (fmt != 0) {
		va_list	argptr;
		va_start (argptr, fmt);
		string.FormatV (fmt, argptr);
		va_end (argptr);
	}
	return string;
}

__declspec (dllexport)
CString	dfcGetFontName ()
{
	CString	string = dfcStr (IDS_FONTNAME);

	if (string.IsEmpty ()) return _T("맑은 고딕");
	else				   return string;
}

__declspec (dllexport)
CString	dfcTokenize (LPCTSTR lpszSrc, TCHAR chSep, int nIndex)
{
	CString	string = lpszSrc;

	if (nIndex == -1) {
		CString	string = lpszSrc;
		int		nPos = string.ReverseFind (chSep);
		return string.Mid (nPos + 1);
	}
	else {
		return dfcTokenize (lpszSrc, &chSep, nIndex);
	}
}

__declspec (dllexport)
CString	dfcTokenize (LPCTSTR lpszSrc, LPCTSTR lpszSep, int nIndex)
{
	CString	string = lpszSrc;
	string += lpszSep;

	int		nCountSep = INT (_tcslen (lpszSep));
	int		nPosPrev = -nCountSep, nPosNext = string.Find (lpszSep);

	for (int i = 0; nPosNext != -1; i++) {
		if (nIndex == i) {
			string = string.Mid (nPosPrev + nCountSep, nPosNext - nPosPrev - nCountSep);
			break;
		}
		else {
			nPosPrev = nPosNext;
			nPosNext = string.Find (lpszSep, nPosPrev + nCountSep);
		}
	}

	if (nPosNext == -1) return _T("");
	else				return string;
}

__declspec (dllexport)
int		dfcTokenize (LPCTSTR lpszSrc, TCHAR chSep, CStringArray & tokens)
{
	return dfcTokenize (lpszSrc, &chSep, tokens);
}

__declspec (dllexport)
int		dfcTokenize (LPCTSTR lpszSrc, LPCTSTR lpszSep, CStringArray & tokens)
{
	CString	string = lpszSrc;
	int		nPos = 0;

	while (1) {
		CString	token = string.Tokenize (lpszSep, nPos);
		if (token.IsEmpty ()) break;

		tokens.Add (token);
	}

	return INT(tokens.GetSize ());
}

__declspec (dllexport)
int		dfcTokenizeCount (LPCTSTR lpszSrc, LPCTSTR lpszSep)
{
	CString	string = lpszSrc;
	int		nPos = 0;

	int nCount = 0;

	while (1) {
		CString	token = string.Tokenize (lpszSep, nPos);
		if (token.IsEmpty ()) break;

		++nCount;
	}

	return nCount;
}

__declspec (dllexport)
CString	dfcPathToFolder (LPCTSTR lpszPath)
{
	CString	string = lpszPath;
	int		nPos = string.ReverseFind ('\\');

	if (nPos == -1) {
		return string;
	}
	else {
		return string.Left (nPos + 1);
	}
}

__declspec (dllexport)
CString	dfcPathToName (LPCTSTR lpszPath)
{
	CString	string = lpszPath;
	int		nPos = string.ReverseFind ('\\');

	if (nPos == -1) {
		return string;
	}
	else {
		return string.Mid (nPos + 1);
	}
}

__declspec (dllexport)
CString	dfcPathToTitle (LPCTSTR lpszPath)
{
	CString	string = lpszPath;
	int		nPos1 = string.ReverseFind ('\\');
	int		nPos2 = string.ReverseFind ('.');

	if (nPos1 == -1) {
		return string;
	}
	else if (nPos2 == -1) {
		return  string.Mid (nPos1 + 1);
	}
	else {
		return  string.Mid (nPos1 + 1, nPos2 - nPos1 - 1);
	}
}

__declspec (dllexport)
CString	dfcPathToExt (LPCTSTR lpszPath)
{
	CString	string = lpszPath;
	int		nPos = string.ReverseFind ('.');

	if (nPos == -1) {
		return string;
	}
	else {
		return string.Mid (nPos + 1);
	}
}

__declspec (dllexport)
CString	dfcReplaceByFolder (LPCTSTR lpszPath, LPCTSTR lpszFolder)
{
	CString	string = lpszPath;
	int		nPos = string.ReverseFind ('\\');

	if (nPos == -1) {
		return string;
	}
	else {
		return lpszFolder + string.Mid (nPos + 1);
	}
}

__declspec (dllexport)
CString	dfcReplaceByName (LPCTSTR lpszPath, LPCTSTR lpszName)
{
	CString	string = lpszPath;
	int		nPos = string.ReverseFind ('\\');

	if (nPos == -1) {
		return string;
	}
	else {
		return string.Left (nPos + 1) + lpszName;
	}
}

__declspec (dllexport)
CString	dfcReplaceByTitle (LPCTSTR lpszPath, LPCTSTR lpszTitle)
{
	CString	string = lpszPath;
	int		nPos1 = string.ReverseFind ('\\');
	int		nPos2 = string.ReverseFind ('.');

	if (nPos1 == -1 || nPos1 >= nPos2) {
		return string;
	}
	else {
		return string.Left (nPos1 + 1) + lpszTitle + string.Mid (nPos2);
	}
}

__declspec (dllexport)
CString	dfcReplaceByExt (LPCTSTR lpszPath, LPCTSTR lpszExt)
{
	CString	string = lpszPath;
	int		nPos = string.ReverseFind ('.');

	if (nPos == -1) {
		return string;
	}
	else {
		return string.Left (nPos) + lpszExt;
	}
}

__declspec (dllexport)
CString	dfcExeFolder ()
{
	CString	string = AfxGetApp ()->m_pszHelpFilePath;
	return string.Left (string.ReverseFind ('\\') + 1);
}

__declspec (dllexport)
CString	dfcIniFolder ()
{
	CString	folder = dfcExeFolder () + _T("Config\\IniFile\\");
	CreateDirectory (folder, 0);

	return folder;
}

__declspec (dllexport)
CString	dfcTemplateFolder ()
{
	CString	folder = dfcExeFolder () + _T("Config\\Template\\");
	CreateDirectory (folder, 0);

	return folder;
}

__declspec (dllexport)
CString	dfcBackupFolder ()
{
	CString	folder = dfcExeFolder () + _T("Config\\Backup\\");
	CreateDirectory (folder, 0);

	return folder;
}

__declspec (dllexport)
CString	dfcResultFolder ()
{
	CString	folder = _T("D:\\Result\\");
	CreateDirectory (folder, 0);
	return folder;
}

__declspec (dllexport)
CString	dfcLogFolder ()
{
	CString	folder = _T("D:\\Log\\");
	CreateDirectory (folder, 0);
	return folder;
}

__declspec (dllexport)
CString	dfcIniFile (LPCTSTR lpszTitle)
{
	if (lpszTitle == 0) {
		CString	string = dfcIniFolder () + _T("Config");// AfxGetApp()->m_pszExeName;

		return string + _T(".ini");
	}
	else {
		CString	string = lpszTitle;
		int		nPos1 = string.ReverseFind ('\\');
		int		nPos2 = string.ReverseFind ('.');

		if (nPos1 == -1 && nPos2 == -1) {
			string = dfcIniFolder () + string + _T(".ini");
		}
		else if (nPos1 != -1 && nPos2 == -1) {
			string = string + _T(".ini");
		}
		else if (nPos1 == -1 && nPos2 != -1) {
			string = dfcIniFolder () + string;
		}

		return string;
	}
}

__declspec (dllexport)
int		dfcGetFiles (CString strFolder, CStringArray & names, BOOL bIncludeDir, CString strExt)
{
	CFileFind	finder;
	BOOL		bWorking = finder.FindFile (strFolder + _T("*.*"));

	while (bWorking) {
		bWorking = finder.FindNextFile ();
		if (finder.IsDots ()) continue;

		if (!bIncludeDir) {
			if (finder.IsDirectory ())	continue;;
		}

		CString	name;
		if (bIncludeDir)
			name = finder.GetFilePath ();
		else
			name = finder.GetFileName ();
		names.Add(name);
	}

	if (!strExt.IsEmpty ()) {
		strExt = '.' + strExt;
		int		nLen = strExt.GetLength ();

		for (int i = INT(names.GetSize ()) - 1; i >= 0; i--) {
			CString	strTarg = names[i].Right (nLen);
			if (strTarg.CompareNoCase (strExt) != 0) names.RemoveAt (i);
		}
	}

	return INT(names.GetSize ());
}

__declspec (dllexport)
BOOL	dfcIsFileRunning (CString path)
{
	FILE* fp = _tfopen (path, _T("a"));
	BOOL	bRunning = (fp == NULL);

	if (fp) fclose (fp);

	return bRunning;
}

__declspec (dllexport)
HWND	dfcGetExcel (CString path)
{
	CString	name = dfcPathToName (path);
	HWND	hWnd = FindWindow (0, dfcStr(_T("Microsoft Excel - %s"), name));

	return hWnd;
}

__declspec (dllexport)
BOOL	dfcCheckUnicode (LPCTSTR lpszPath)
{
	const	unsigned char	UNICODE_BOM[2]  = { unsigned char(0xFF), unsigned char(0xFE) };
	const	unsigned char	UNICODE_RBOM[2] = { unsigned char(0xFE), unsigned char(0xFF) };

	FILE* fp = _tfopen (lpszPath, _T("r"));
	if (fp == 0) return FALSE;

	unsigned char	uchBOM[3] = { 0 };
	fread (reinterpret_cast<LPVOID> (uchBOM), sizeof (unsigned char), 2, fp);
	fclose(fp);

	return (uchBOM[0] == UNICODE_BOM[0] && uchBOM[1] == UNICODE_BOM[1]) ||
		   (uchBOM[0] == UNICODE_RBOM[0] && uchBOM[1] == UNICODE_RBOM[1]);
}

__declspec (dllexport)
char* dfcUniToAnsi (LPCTSTR lpszStr)
{
	static	char	pBuf[1024];

#ifdef	_UNICODE
	LPCWSTR strUni = CT2CW (lpszStr);
	int		nBufSize = WideCharToMultiByte (CP_ACP, 0, strUni, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte (CP_ACP, 0, strUni, -1, pBuf, nBufSize, 0, 0);
#else
	memcpy (pBuf, lpszStr, strlen (lpszStr) + 1);
#endif

	return pBuf;
}

__declspec (dllexport)
void	dfcPeekAndPump (DWORD dwDelay)
{
	try {
		DWORD cnt = 0;
		if (dwDelay != 0) cnt = GetTickCount ();

		CWinThread * pThread = AfxGetThread ();
		if (pThread == NULL) pThread = AfxGetApp ();

		MSG msg;
		while (::PeekMessage (&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!pThread->PumpMessage ()) {
				return;
			}

			if (dwDelay != 0)
			{
				if ((GetTickCount () - cnt) > dwDelay)
					return;
			}
		}

		LONG idle = 0;
		while (pThread->OnIdle (idle++));
	}
	catch (CException * e) {
		e = NULL;
	}
	catch (...) {
	}
}

__declspec (dllexport)
CString	dfcCurrentTime (BOOL bYMD, BOOL bHMS)
{
	CTime	time = CTime::GetCurrentTime ();

	if (bYMD && bHMS)	return time.Format ("%Y-%m-%d %H:%M:%S");
	else if (bYMD)		return time.Format ("%Y-%m-%d");
	else if (bHMS)		return time.Format ("%H%M%S");//time.Format ("%H:%M:%S");
	else 				return _T("");
}

__declspec (dllexport)
CString	dfcCurrentTime (TCHAR cSep)
{
	CTime	time = CTime::GetCurrentTime ();
	if (cSep == 0) return time.Format ("%Y%m%d-%H%M%S");
	else		   return time.Format ("%Y%m%d") + cSep + time.Format("%H%M%S");
}


__declspec (dllexport)
CString	dfcElapsedTime (DWORD dwTickStart)
{
	DWORD	dwElapsed = GetTickCount () - dwTickStart;
	int		nMinute = dwElapsed / 60000;
	double	fSecond = (dwElapsed % 60000) / 1000.0;

	if (nMinute == 0) return dfcStr (IDS_ELAPSEDTIME1, fSecond);
	else 			  return dfcStr (IDS_ELAPSEDTIME2, nMinute, fSecond);
}

int		StringCompare (const void* pArg1, const void* pArg2)
{
	CString* pString1 = (CString*) pArg1;
	CString* pString2 = (CString*) pArg2;
	return pString1->CompareNoCase (*pString2);
}

__declspec (dllexport)
void	dfcSorting (CStringArray& strBases)
{
	int		nSize = (INT)strBases.GetSize ();
	CString* pData = strBases.GetData ();
	qsort ((void*)pData, (size_t) nSize, sizeof (CString*), StringCompare);
}

__declspec (dllexport)
CRect	dfcWindowRect (CWnd* pWnd, CWnd* pWndMap)
{
	CRect	rcWindow;
	pWnd->GetWindowRect (&rcWindow);

	if (pWndMap) {
		pWnd->MapWindowPoints (pWndMap, &rcWindow);
	}

	return rcWindow;
}

__declspec (dllexport)
CRect	dfcClientRect (CWnd* pWnd, CWnd* pWndMap)
{
	CRect	rcClient;
	pWnd->GetClientRect (&rcClient);

	if (pWndMap) {
		pWnd->MapWindowPoints (pWndMap, &rcClient);
	}

	return rcClient;
}

__declspec (dllexport)
CRect	dfcAdjustRect (CRect rcSrc, CRect rcTarg)
{
	CPoint	point = rcTarg.CenterPoint ();
	double	fRatioW = (double)rcTarg.Width () / rcSrc.Width ();
	double	fRatioH = (double)rcTarg.Height () / rcSrc.Height ();
	double	fRatio = (fRatioW > fRatioH) ? fRatioH : fRatioW;

	rcTarg.right = rcTarg.left + int(rcSrc.Width () * fRatio);
	rcTarg.bottom = rcTarg.top + int(rcSrc.Height () * fRatio);
	rcTarg.OffsetRect(point - rcTarg.CenterPoint ());
	return rcTarg;
}

__declspec (dllexport)
void	dfcPrintOrientation (short nOrientation)
{
	PRINTDLG	pd;
	pd.lStructSize = (DWORD)sizeof (PRINTDLG);

	BOOL	bRet = AfxGetApp()->GetPrinterDeviceDefaults (&pd);
	if (bRet) {
		DEVMODE	FAR* pDevMode = (DEVMODE FAR*)::GlobalLock (pd.hDevMode);
		pDevMode->dmOrientation = nOrientation;
		::GlobalUnlock (pd.hDevMode);
	}
}

__declspec (dllexport)
CString	dfcGetLastErrorString ()
{
	DWORD dwError = GetLastError ();
	CString strMessage;
	LPVOID lpMsgBuf;
	if (!FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)& lpMsgBuf,
		0,
		NULL)) {
		return strMessage;
	}
	strMessage = CString ((LPTSTR)lpMsgBuf);
	LocalFree(lpMsgBuf);

	return strMessage;
}

__declspec (dllexport)
void	dfcMakeFullPath (CString strfullPath)
{
	int nPos2 = 0;
	int nPos = strfullPath.Find ('\\');
	CString csTmp = strfullPath.Mid (0, nPos);

	while (1)
	{
		nPos2 = strfullPath.Find ('\\', nPos + 1);
		if (nPos2 < 0)
		{
			::CreateDirectory ((LPCTSTR)strfullPath, NULL);
			SetFileAttributes (csTmp, FILE_ATTRIBUTE_NORMAL);
			break;
		}
		csTmp = strfullPath.Mid (0, nPos2);
		nPos = nPos2;
		::CreateDirectory ((LPCTSTR)csTmp, NULL);
		SetFileAttributes (csTmp, FILE_ATTRIBUTE_NORMAL);
	}
}

BOOL CALLBACK MonitorEnumProc (HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
	vector <CRect>* pMonitors = ((vector <CRect>*)dwData);
	MONITORINFOEX mi;
	mi.cbSize = sizeof (MONITORINFOEX);
	GetMonitorInfo (hMonitor, &mi);
	if (mi.dwFlags & MONITORINFOF_PRIMARY) {
		if (pMonitors) pMonitors->push_back (*lprcMonitor);
	}
	else {
		if (pMonitors) pMonitors->push_back (*lprcMonitor);
	}
	return TRUE;
}

__declspec (dllexport)
BOOL	dfcIsPositionInMonitor (CPoint pt)
{
	vector <CRect>	vecMonitors;
	EnumDisplayMonitors (NULL, NULL, MonitorEnumProc, (LPARAM)& vecMonitors); // Callback으로 모니터 정보 알아오기
	for (size_t i = 0; i < vecMonitors.size (); i++) {
		if (vecMonitors[i].PtInRect (pt)) return TRUE;
	}
	return FALSE;
}

__declspec (dllexport)
BOOL	dfcCreateDirectory (LPCTSTR szPath)
{
	int nLen = (int)_tcslen (szPath);

	//<<- 길이가 없으면 반환 
	if (nLen == 0)
		return FALSE;

	//<<- 임시 버퍼 생성 
	TCHAR *pBuf = new TCHAR[nLen + 1];
	_tcscpy (pBuf, szPath); // 복사 

	//<<- 확장자 제거 
	PathRemoveExtension (pBuf);

	//<<- Path 설정 
	CString strPath = pBuf;

	//<<- 메모리 제거 
	delete[] pBuf;

	//<<- 파일인지 디렉터리인지 여부 확인 (길이가 달라지면 파일이라고 가정)
	if (_tcslen(szPath) != strPath.GetLength ()) {
		int n = strPath.ReverseFind (_T('\\'));
		strPath = strPath.Left (n);	//<<- 폴더 경로만 추출
	}

	//<<- 폴더 경로가 있는지여부 확인 
	if (_taccess(strPath, 0) != -1)
		return TRUE;

	strPath += _T('\\');

	//<<- 루프를 돌면서 폴더 생성 
	int nIdx;
	int nStart = 0;
	while ((nIdx = strPath.Find (_T('\\'), nStart)) >= 0) {
		CString str = strPath.Left (nIdx);
		if (_taccess (str, 0) == -1) {
			if (!CreateDirectory (str, NULL))
				return FALSE;
		}
		nStart = nIdx + 1;
	}

	return TRUE;
}

__declspec (dllexport)
int		dfcGetDiffernceDay (LPCTSTR szPath)
{
	int nDiffDay = 0;
	HANDLE hFile = CreateFile (szPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		FILETIME	timeCreate, timeAccess, timeModify;
		GetFileTime (hFile, &timeCreate, &timeAccess, &timeModify);

		SYSTEMTIME	systemTime, localTime;
		FileTimeToSystemTime (&timeCreate, &systemTime);

		SystemTimeToTzSpecificLocalTime (NULL, &systemTime, &localTime);

		CTime time (localTime);
		CTimeSpan	span = CTime::GetCurrentTime () - time;

		nDiffDay = INT(span.GetDays ());

		CloseHandle (hFile);
	}
	return nDiffDay;
}

__declspec (dllexport)
BOOL	dfcDeleteAllFolder (CString strPath)
{
	if (strPath.IsEmpty ()) {
		return FALSE;
	}
	BOOL		bRval = FALSE;
	int			nRval = 0;
	CString		strNextDirPath;
	CString		strRootPath;
	CFileFind	ff;

	bRval = ff.FindFile (strPath);
	if (bRval == FALSE) return bRval;

	while (bRval)
	{
		bRval = ff.FindNextFile ();
		if (ff.IsDots () == TRUE) continue;

		if (ff.IsDirectory ()) {
			strNextDirPath.Format (_T("%s\\*.*"), ff.GetFilePath ());
			dfcDeleteAllFolder(strNextDirPath);
		}
		else {
			DeleteFile (ff.GetFilePath ());
		}
	}
	strRootPath = ff.GetRoot ();
	ff.Close ();

	bRval = RemoveDirectory (strRootPath);
	return bRval;
}

__declspec (dllexport)
void	WriteString (CStdioFile *File, CString string)
{
	string += _T("\n");
	File->WriteString (string);
}

__declspec (dllexport)
void	WriteInt (CStdioFile *File, int nVal)
{
	CString string;
	string = dfcStr (_T("%d\n"), nVal);
	File->WriteString (string);
	
}

__declspec (dllexport)
void	WriteDouble (CStdioFile *File, double fVal)
{
	CString string;
	string = dfcStr (_T("%.4lf\n"), fVal);
	File->WriteString (string);

}

__declspec (dllexport)
CString	ReadString (CStdioFile *File)
{
	CString string;
	File->ReadString (string);
	return string;
}

__declspec (dllexport)
int		ReadInt (CStdioFile *File)
{
	CString string;
	File->ReadString (string);
	return _ttoi (string);
}

__declspec (dllexport)
double	ReadDouble (CStdioFile *File)
{
	CString string;
	File->ReadString (string);
	return _wtof (string);
}