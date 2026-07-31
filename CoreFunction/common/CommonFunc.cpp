// CommonFunc.cpp : implementation file
//

#include "pch.h"
#include "CommonFunc.h"
#include <direct.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CString		g_sAppPath;
//CDaoDatabase		g_dbReport;
CCriticalSection	g_csQuery;
CCriticalSection	g_csTime;
CCriticalSection	g_csTime2;
CCriticalSection	g_csDir;
CCriticalSection	g_csDay;
CCriticalSection	g_csCTime;
CCriticalSection	g_csSTime;
CCriticalSection	g_csETime;
CCriticalSection	g_chTime;

/////////////////////////////////////////////////////////////////////////////
// CommonFunc

void gf_Initialize() 
{
	TCHAR szBuff[_MAX_PATH];
	::GetModuleFileName(::AfxGetApp()->m_hInstance, szBuff, _MAX_PATH);	
	//char* pch = ::strrchr(szBuff, '\\');
	TCHAR* pch = wcsrchr(szBuff, '\\');
	g_sAppPath = CString(szBuff, pch - szBuff + 1);
}

BOOL gf_MakeDir(CString sPath)
{
	if( sPath.IsEmpty())
		return FALSE;

	g_csDir.Lock();
	if (sPath[1] != ':' || sPath[2] != '\\')
	{
		g_csDir.Unlock();
		return FALSE;
	}

	CString sTem;
	char* cTem;
    int nFind = 2;
	while ((nFind = sPath.Find(_T("\\"),nFind+1)) >= 0)
	{
		sTem = sPath.Left(nFind);
		//_mkdir(sTem);
		CreateDirectory(sTem, NULL);
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
//	sTime.Format("%04d.%02d.%02d-%02d:%02d:%02d:%03d",st.wYear ,st.wMonth ,st.wDay ,st.wHour ,st.wMinute ,st.wSecond, st.wMilliseconds  );
	sTime.Format(_T("%02d%02d_%02d%02d_%02d.%03d"),st.wMonth ,st.wDay ,st.wHour ,st.wMinute ,st.wSecond, st.wMilliseconds);	// sih 20180201 change 
	g_csTime.Unlock();
	return sTime;
}

CString gf_MakeCurTimeString2()
{
//	g_csTime2.Lock();
	CString sTime;
	SYSTEMTIME st;
	::GetLocalTime(&st);
	sTime.Format(_T("[%04d-%02d-%02d %02d:%02d:%02d:%03d]"),st.wYear ,st.wMonth ,st.wDay ,st.wHour ,st.wMinute ,st.wSecond, st.wMilliseconds  );
//	g_csTime2.Unlock();
	return sTime;
}

CString gf_GetCurDayString(int iOption)
{
	g_csDay.Lock();
	CString sTime;
	SYSTEMTIME st;
	::GetLocalTime(&st);
	if( iOption == 0)
		sTime.Format(_T("%02d"),st.wDay);
	else
		sTime.Format(_T("%04d%02d%02d"),st.wYear, st.wMonth, st.wDay);
	g_csDay.Unlock();
	return sTime;
}

CString gf_GetCurTimeString(int itype)
{
	g_csCTime.Lock();
	CString sTime;
	SYSTEMTIME st;
	::GetLocalTime(&st);

	switch (itype)
	{
	case 0:
		sTime.Format(_T("%04d%02d%02d-%02d%02d%02d"),st.wYear ,st.wMonth ,st.wDay ,st.wHour ,st.wMinute ,st.wSecond );
		break;
	case 1:
		sTime.Format(_T("%04d%02d%02d, %02d:%02d:%02d.%03d"),st.wYear ,st.wMonth ,st.wDay ,st.wHour ,st.wMinute ,st.wSecond, st.wMilliseconds );
		break;
	case 2:
		{
			CTime tData(st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);		
			sTime.Format(_T("%ld.%d"), tData, st.wMilliseconds);
		}
		break;
	case 3:
		sTime.Format(_T("%02d%02d%02d"),st.wHour ,st.wMinute ,st.wSecond );
		break;
	case 4:  // 20190619193500
		sTime.Format(_T("%04d%02d%02d%02d%02d%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		break;
	case 5:
		sTime.Format(_T("%04d.%02d.%02d-%02d:%02d:%02d:%03d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
		break;
	}
	
	g_csCTime.Unlock();
	return sTime;
}

CString gf_CharToString(char* pchar)
{
	CString strTemp;
	strTemp.Empty();

#ifdef _UNICODE

	//º¯È¯ÇÒ ¹®ÀÚ¿­ÀÇ Å©±â
	int nLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pchar, -1, NULL, NULL);
	//¹öÆÛÀÇ Å©±â¸¦ µ¿Àû ÇÒ´ç
	WCHAR* pszWide = new WCHAR[nLen];
	//Çüº¯È¯
	if (pszWide)
	{
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pchar, strlen(pchar) + 1, pszWide, nLen);

		strTemp.Format(_T("%s"), pszWide);

		delete[] pszWide;
	}
#else
	strTemp.Format(_T("%s"), pchar);
#endif

	return strTemp;
}

char* gf_StringToChar(CString str)
{
	char* char_str = NULL;

#ifdef _UNICODE

	// Unicode CString -> char* º¯È¯ °úÁ¤ 
	// CString -> wchar* -> char* ¼ø¼­·Î º¯È¯ µÇ¾î¾ß ÇÔ
	wchar_t* wchar_str;
	int char_str_len = 0;

	// 1. CString to wchar * conversion
	wchar_str = str.GetBuffer(str.GetLength());
	char_str_len = WideCharToMultiByte(CP_ACP, 0, wchar_str, -1, NULL, 0, NULL, NULL);
	char_str = new char[char_str_len];

	// 2. wchar_t* to char* conversion
	WideCharToMultiByte(CP_ACP, 0, wchar_str, -1, char_str, char_str_len, 0, 0);

#else
	char sz[4096];
	sz[0] = 0x00;
	//	_tcscpy_s(pchar, sizeof(pchar), str);
	_tcscpy_s(pchar, _countof(pchar), str);
	char_str = sz;
#endif

	return char_str;
}


CString gf_GetStartTime()
{
	g_csSTime.Lock();
	SYSTEMTIME st;
	::GetLocalTime(&st);
	CString sTime;
	sTime.Format(_T("%04d%02d%02d%02d%02d%02d"), st.wYear, st.wMonth, st.wDay, st.wHour,st.wMinute, st.wSecond );
	g_csSTime.Unlock();
	return sTime;
}

double gf_GetCurTSec()
{
	g_csETime.Lock();
	SYSTEMTIME st;
	::GetLocalTime(&st);
	double dTime = 0;
	dTime = 3600 * st.wHour + 60 * st.wMinute + st.wSecond + st.wMilliseconds/1000;
	g_csETime.Unlock();
	return dTime;
}

BOOL gf_CharacterCheck(CString str)
{
	g_chTime.Lock();
	BOOL bRet = FALSE;
	char ch;
	for(int i = 0; i < str.GetLength(); i++)
	{
		ch = str.GetAt(i);
		if( (ch > 31 && ch < 46) || (ch > 57 && ch < 65) || 
			(ch > 90 && ch < 95) || (ch == 96) || (ch > 122 && ch < 127) )		//'_'(95)Æ¯¼ö¹®ÀÚ¸¦ Á¦¿ÜÇÑ ¸ðµç Æ¯¼ö¹®ÀÚ Á¤ÀÇ
		{
			bRet = TRUE;
			break;
		}		
	}
	g_chTime.Unlock();
	return bRet;
}

CString gf_GetFilePath(CString sPath)
{
	CString strRet;
	strRet.Format(_T("%s%s"), g_sAppPath, sPath);
	return strRet;
}

CString gf_GetCnfPath(int nEQType, CString sPath)
{
	CString strRet;
	switch (nEQType)
	{
	case EQU_TYPE_ICE:
		strRet.Format(_T("%s%s"),gf_GetFilePath(_T("CONFIG\\ICE\\")), sPath);
		break;
	case EQU_TYPE_ICM:
		strRet.Format(_T("%s%s"),gf_GetFilePath(_T("CONFIG\\ICM\\")), sPath);
		break;
	case EQU_TYPE_ICP:
		strRet.Format(_T("%s%s"),gf_GetFilePath(_T("CONFIG\\ICP\\")), sPath);
		break;
	case EQU_TYPE_OCP:
		strRet.Format(_T("%s%s"),gf_GetFilePath(_T("CONFIG\\OCP\\")), sPath);
		break;
	default:
		strRet.Format(_T("%s%s"),gf_GetFilePath(_T("CONFIG\\DEFAULT\\")), sPath);
		break;
	}

	return strRet;
}

CString gf_MakeMcrID(CString str)
{
	CString strRet;
	int iCheckSum =0;
	str.TrimLeft();
	str.TrimRight();
	for (int i=0; i<str.GetLength(); i++)
	{
		char a  = str.GetAt(i);
		iCheckSum += a;
	}
	strRet.Format(_T("%s%d"), str, iCheckSum%10);
	//////////////////////////////////////////////////////////////////////////
	return strRet;
}

CString gf_GetFileTime(CString sPath)
{
	CString strRet = _T("");
	HANDLE hFile;
	//	HANDLE hFMap;
// 	wchar_t szFileName[MAX_PATH];
// 	::GetModuleFileNameW(NULL, sPath, MAX_PATH);

	hFile = CreateFile( sPath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	
	SYSTEMTIME write_system_time, write_local_time;
	
	if( hFile != INVALID_HANDLE_VALUE )
	{
		FILETIME create_time, access_time, write_time;
		GetFileTime(hFile, &create_time, &access_time, &write_time);
		
		// FILETIME ±¸Á¶Ã¼Á¤º¸¸¦ SYSTEMTIME ±¸Á¶Ã¼ Á¤º¸·Î º¯È¯ÇÑ´Ù. FILETIME ±¸Á¶Ã¼¿¡ µé¾îÀÖ´Â Á¤º¸´Â
		// ¿ì¸®°¡ Á÷Á¢ÀûÀ¸·Î ÀÌ¿ëÇÏ·Á¸é °è»êÀÌ º¹ÀâÇØÁö±â ¶§¹®¿¡ »ç¿ëÇÏ±â Æí¸®ÇÑ SYSTEMTIME ±¸Á¶Ã¼
		// Çü½ÄÀ¸·Î º¯È¯ÇØ¼­ »ç¿ëÇÑ´Ù.
		FileTimeToSystemTime(&write_time, &write_system_time);
		
		
		// FILETIME ±¸Á¶Ã¼¿¡¼­ SYSTEMTIME ±¸Á¶Ã¼·Î º¯È¯µÇ¸é ½Ã°£Á¤º¸°¡ UTC(Universal Time Coordinated) Çü½ÄÀÇ 
		// ½Ã°£ÀÌ±â ¶§¹®¿¡ Å½»ö±â³ª ±âÅ¸ ÇÁ·Î±×·¥¿¡¼­ º¸´Â ½Ã°£°ú ´Ù¸£´Ù. µû¶ó¼­ Á¤È®ÇÑ ½Ã°£Á¤º¸¸¦ ¾ò±â À§ÇØ¼­
		// UTC Çü½ÄÀÇ ½Ã°£À» Áö¿ª½Ã°£À¸·Î º¯È¯ÇØ¾ß ÇÑ´Ù. ¾Æ·¡ÀÇ ÇÔ¼ö´Â ÇØ´ç ÀÛ¾÷À» ÇÏ´Â ÇÔ¼öÀÌ´Ù.
		SystemTimeToTzSpecificLocalTime(NULL, &write_system_time, &write_local_time);
		strRet.Format(_T("%04d%02d%02d_%02d:%02d:%02d"), write_local_time.wYear, write_local_time.wMonth,write_local_time.wDay,write_local_time.wHour, write_local_time.wMinute, write_local_time.wSecond);
		
		// 	// ÆÄÀÏ ¸ÊÇÎ ¿ÀºêÁ§Æ®¸¦ ¸¸µç´Ù.
		// 	hFMap = CreateFileMappingW( hFile, NULL, PAGE_READONLY, 0, 0, NULL );
		// 	// ÁÖ¼Ò °ø°£¿¡ ¸ÊÇÑ´Ù.
		// 	HMODULE hModule;
		// 	hModule = ( HMODULE )MapViewOfFile( hFMap, FILE_MAP_READ, 0, 0, 0 );
		// 	CTime time( GetTimestampForLoadedLibrary( hModule ) );
		// 	
		// 	UnmapViewOfFile( hModule );
		// 	CloseHandle( hFMap );
		
		CloseHandle( hFile );
	}
	
	return strRet;
}

void DLOG(CString s)
{
#ifdef _DLOG
	LOG_Development(s);
#endif
}

void gLog(HWND hWnd,LPCTSTR pszEvt, int nLevel, ...)
{
	CString szFormat = pszEvt;
	
	PROCESS_INFORMATION ProcessInfo;
	char		Buffer[_MAX_PATH];
	TCHAR		szBuffer[_MAX_PATH];
	COPYDATASTRUCT	cpData;
//	int		nLevel;
	
//	strcpy(szBuffer,"");
	_tcscpy_s(szBuffer,_countof(szBuffer), TEXT(""));
	cpData.dwData = 0;
	cpData.cbData = 1;
	cpData.lpData = "\0";
	
	//PROCESS_INFORMATION ProcessInfo;
	STARTUPINFO StartupInfo;
    
    ZeroMemory(&StartupInfo,sizeof(STARTUPINFO));
    ZeroMemory(&ProcessInfo,sizeof(PROCESS_INFORMATION));
    StartupInfo.cb=sizeof(STARTUPINFO);
    StartupInfo.dwFlags=STARTF_USESHOWWINDOW;
    StartupInfo.wShowWindow=SW_MINIMIZE;
	
	if(!hWnd)
	{
		return;	
	}
	
	va_list valist;
	//char szTmp[_MAX_PATH];
	TCHAR szTmp[_MAX_PATH];
//	strcpy(szTmp, szFormat.GetBuffer(szFormat.GetLength()));
	_tcscpy_s(szTmp,_countof(szTmp), szFormat.GetBuffer(szFormat.GetLength()));
	va_start(valist, szTmp);
	int nResult = wvsprintf(szBuffer,szFormat.GetBuffer(szFormat.GetLength() + 1),
		valist);	
	va_end(valist);
	
	szFormat.ReleaseBuffer();
	
//	nLevel = 0;
	WideCharToMultiByte(CP_ACP, 0, szBuffer, _MAX_PATH, Buffer, _MAX_PATH, NULL, NULL);

	cpData.dwData = (DWORD) nLevel;
	cpData.cbData = strlen(Buffer) + 1;
	cpData.lpData = szBuffer;
	
	//send the structure to Debug via the system
	LRESULT lResult = ::SendMessage(hWnd, WM_COPYDATA, (UINT) NULL, (long) &cpData);
}

unsigned int hextoi(CString strHex)
{
	unsigned char bin[8] = {0,};
	unsigned int m_nDEC = 0;
	int nLen = strHex.GetLength();
	if (nLen > 8)
		return -1;
	for (int i=0 ; i<nLen ; i++) {
		bin[i] = HexToByte(strHex[(nLen-1) - i]);
		if(bin[i] >= 0 && bin[i] <= 15) {    // 0 ~ A ~ F
			m_nDEC += bin[i] << (4*i);     // 10Áø¼ö Ãâ·Â
		}
		else if (bin[i] == 32) {
			return -1;
		}
	}
	return m_nDEC;
}

unsigned char HexToByte(char hex)
{
	switch(hex)
	{
	case '0': return 0;
	case '1': return 1;
	case '2': return 2;
	case '3': return 3;
	case '4': return 4;
	case '5': return 5;
	case '6': return 6;
	case '7': return 7;
	case '8': return 8;
	case '9': return 9;
	case 'A': case 'a': return 10;
	case 'B': case 'b': return 11;
	case 'C': case 'c': return 12;
	case 'D': case 'd': return 13;
	case 'E': case 'e': return 14;
	case 'F': case 'f': return 15;
	}
	return 32;
}

BOOL gf_GetValue(LPCTSTR section, LPCTSTR key, LPCTSTR default, LPTSTR value, DWORD nSize, LPCTSTR filepath) 
{
	BOOL result;
	result = GetPrivateProfileString (section, key, default, value, nSize,filepath);

	return result;
}

BOOL gf_WriteValue(LPCTSTR section, LPCTSTR key, LPCTSTR value, LPCTSTR filepath) 
{
	BOOL result;

	result = WritePrivateProfileString (section, key, value, filepath); 
	
	return result;
}


//¦®¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦³¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¯
//¦­ RADIAN                       ¦­ math.h¿¡¼­ Á¦°øµÇ¾îÁö´Â »ï°¢ÇÔ¼öÀÇ ÀÎÀÚ°ªÀ¸·Î »ç¿ëµÇ¾î Áö´Â ´ÜÀ§ÀÌ´Ù       ¦­
//¦­                              ¦­ ¿ø¿¡¼­ ¹ÝÁö¸§ rÀÇ ±æÀÌ¿Í È£ÀÇ ±æÀÌ°¡ °°µµ·Ï ÇÏ´Â °¢(¥á)À» ±¸ÇØº¸ÀÚ         ¦­
//¦­                              ¦­ ¿øÀÇ Áö¸§ ±¸ÇÏ´Â °ø½Ä : 2¥ðr                                               ¦­
//¦­                              ¦­ µû¶ó¼­ ¹ÝÁö¸§°ú È£ÀÇ ±æÀÌ°¡ °°µµ·Ï ÇÏ´Â ¹æÁ¤½ÄÀº                           ¦­
//¦­                              ¦­      360 : ¥á = 2¥ðr : r                                                   ¦­
//¦­                              ¦­      ==> 360/ ¥á = 2¥ðr / r                                                ¦­
//¦­                              ¦­          ==> ¥á = 180 / ¥ð                                                 ¦­
//¦­                              ¦­ À§ÀÇ ½ÄÃ³·³ ¹ÝÁö¸§ÀÇ ±æÀÌ¸¸Å­ÀÇ È£¿¡ ´ëÇÑ Áß½É°¢ÀÇ Å©±â¸¦ 1¶óµð¾È¶ó°í ÇÑ´Ù ¦­
//¦­                              ¦­                                                                            ¦­
//¦²¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¶¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦´
//¦­<°ø½Ä 1>                      ¦­1 Radian = 180 / ¥ð                                                         ¦­
//¦·¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦À¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¹
//¦­<°ø½Ä 2>                      ¦­1 Degree = ¥ð / 180                                                         ¦­
//¦±¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦µ¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦°


//¦®¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦³¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¯
//¦­ DegreeToRadian ()            ¦­ Degree Data¸¦ Radian Data·Î º¯È¯ ½ÃÅ²´Ù                                    ¦­
//¦­                              ¦­                                                                            ¦­
//¦²¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¶¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦´
//¦­1. INPUT                      ¦­double     dDegree : º¯È¯ ½ÃÅ³ Degree °ª                                    ¦­
//¦·¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦À¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¹
//¦­2. OUTPUT                     ¦­nothing                                                                     ¦­
//¦·¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦À¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¹
//¦­3. RETURN                     ¦­return (double) dRadian : º¯È¯µÈ Radian °ª                                  ¦­
//¦±¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦µ¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦°
double DegreeToRadian (double dDegree)
{
	return dDegree/RADIAN;
}

//¦®¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦³¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¯
//¦­ RadianToDegree ()            ¦­ Radian Data¸¦ Degree Data·Î º¯È¯ ½ÃÅ²´Ù                                    ¦­
//¦­                              ¦­                                                                            ¦­
//¦²¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¶¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦´
//¦­1. INPUT                      ¦­double     dRadian : º¯È¯ ½ÃÅ³ Radian °ª                                    ¦­
//¦·¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦À¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¹
//¦­2. OUTPUT                     ¦­nothing                                                                     ¦­
//¦·¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦À¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¡¦¹
//¦­3. RETURN                     ¦­return (double) dDegree : º¯È¯µÈ Degree °ª                                  ¦­
//¦±¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦µ¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦¬¦°
double RadianToDegree (double dRadian)
{
	return dRadian*RADIAN;
}

//jh.kang 20180223 add Using for AgingTest Using for AgingTest
////20170524 add by Hee (CString ÇÔ¼ö ¼±¾ð ¾øÀÌ Format º¯°æ)
CString	dfcStr (LPCTSTR fmt, ...)
{
	CString	string;
	va_list	argptr;
	va_start (argptr, fmt);
	string.FormatV (fmt, argptr);
	va_end (argptr);
	return string;
}

////20171206 add by Hee (Æú´õ ¾øÀ¸¸é »ý¼º)
BOOL dfcCreateFolder (CString &filePath)
{
	int nPos = -1;
	CString strParsing = filePath.TrimLeft ('\\');
	CString strFolderPath;

	while (1) {
		nPos = strParsing.Find ('\\');
		CString strFolderName;
		if (nPos != -1) {
			strFolderName = strParsing.Left(nPos);
			strParsing = strParsing.Mid (nPos+1);
			if (strFolderName.Find (_T(":")) != -1) strFolderPath += strFolderName + _T("\\");
			else strFolderPath += _T("\\") + strFolderName;
		}
		else {
			strFolderPath += _T("\\") + strParsing;
			strFolderName = strParsing;
			break;
		}

		CFileFind	finder;
		if (!finder.FindFile (strFolderPath)) CreateDirectory (strFolderPath, 0);
		finder.Close ();
	}

	return TRUE;
}
//

void gf_PrepareRetryPos(int nRetry, BOOL bRelativePos, double x0, double y0, double dx, double dy, double *px, double *py)
{
	BOOL bPosi = TRUE;
	int	 i, j, k;

	k = 0;

	if(bRelativePos)
	{
		for (i = 0; k < nRetry ; i++)
		{
			for (j = 0; j < i; j++)
			{				
				px[k] = 0.;
				py[k] = (bPosi) ? (dy) : (dy * -1);
				k++;
				if (k >= nRetry) return;
			}
			for (j = 0; j < i; j++)
			{
				px[k] = (bPosi) ? (dx) : (dx * -1);
				py[k] = 0.;
				k++;
				if (k >= nRetry) return;
			}
			bPosi = !bPosi;
		}
	}
	else
	{
		for (i = 0; k < nRetry ; i++)
		{
			for (j = 0; j < i; j++)
			{
				px[k] = px[k-1];
				py[k] = (bPosi) ? (py[k-1] + dy) : (py[k-1] - dy);
				k++;
				if (k >= nRetry) return;
			}
			for (j = 0; j < i; j++)
			{
				px[k] = (bPosi) ? (px[k-1] + dx) : (px[k-1] - dx);
				py[k] = py[k-1];
				k++;
				if (k >= nRetry) return;
			}
			bPosi = !bPosi;
		}
	}
}


//====================================================================
// CPtrnInfo : ÆÐÅÏ°ü·Ã ÀÌ¹ÌÁö´Â ¹«Á¶°Ç U8 !!
//====================================================================
// CPtrnInfo::CPtrnInfo()
// {
// 	m_strName.Empty();
// 
// 	m_pImgInput		= imaqCreateImage(IMAQ_IMAGE_RGB, INT_IMAGE_BORDER_SIZE);	
// 	m_pImgOrg		= imaqCreateImage(IMAQ_IMAGE_RGB, INT_IMAGE_BORDER_SIZE);	
// 	m_pImgOrgHSL	= imaqCreateImage(IMAQ_IMAGE_HSL, INT_IMAGE_BORDER_SIZE);	//uhkang 20100317 
// 	m_pImgOrgBW		= imaqCreateImage(IMAQ_IMAGE_RGB, INT_IMAGE_BORDER_SIZE);	//uhkang 20100317 
//  	m_pImgScaled	= imaqCreateImage(IMAQ_IMAGE_RGB, INT_IMAGE_BORDER_SIZE);
// 	m_pImgHSLScaled	= imaqCreateImage(IMAQ_IMAGE_HSL, INT_IMAGE_BORDER_SIZE);
// 	m_pImgBWScaled	= imaqCreateImage(IMAQ_IMAGE_RGB, INT_IMAGE_BORDER_SIZE);
// 
// 	m_rectPtrn = CRect(0, 0, 0, 0);
// 
// 	m_dThreshold = 500.0;
// 
// 	m_bDetected = FALSE;
// 	m_dx = m_dy = 0.0;
// 	m_dRealX = m_dRealY = DUMMY_POS;
// 
// 	m_iFindNum = m_iCntrNum = 1;
// 
// 	m_iCenMarginY = m_iCenMarginX = 1;
// 	dCenOffsetY   = dCenOffsetX   = 0;
// 	m_iPtrnMatchPriority		  = 0;
// 
// 	m_bPtrnLearn = FALSE;
// }
// 
// CPtrnInfo::~CPtrnInfo()	
// {
// //	m_strName.Empty();
// 	IMAQ_DISPOSE(m_pImgInput);
// 	IMAQ_DISPOSE(m_pImgOrg);
// 	IMAQ_DISPOSE(m_pImgOrgHSL);	
// 	IMAQ_DISPOSE(m_pImgOrgBW);
// 	IMAQ_DISPOSE(m_pImgScaled);
// 	IMAQ_DISPOSE(m_pImgHSLScaled);
// 	IMAQ_DISPOSE(m_pImgBWScaled);
// }
// 
// void CPtrnInfo::Serialize(CArchive& ar, int iVer)
// {
// 	CObject::Serialize(ar);
// 
// 	if(ar.IsStoring())	// save : <<
// 	{
// 		ar << m_strName;
// 
// 		// rect
// 		ar << m_rectPtrn.left;
// 		ar << m_rectPtrn.top;
// 		ar << m_rectPtrn.right;
// 		ar << m_rectPtrn.bottom;
// 
// 		ar << m_dThreshold;
// 
// 		ar << m_bDetected;
// 		ar << m_dx;
// 		ar << m_dy;
// 		ar << m_iFindNum;
// 		ar << m_iCntrNum;
// 		ar << m_iCenMarginX;
// 		ar << m_iCenMarginY;
// 		ar << dCenOffsetX;
// 		ar << dCenOffsetY;
// 		ar << m_iPtrnMatchPriority;
// 
// 		if( iVer > 0 )		// jsyoon 20170327 add
// 			ar << m_bPtrnLearn;
// 	}
//     else	// open : >>
// 	{
// 		ar >> m_strName;
// 
// 		// rect
// 		ar >> m_rectPtrn.left;
// 		ar >> m_rectPtrn.top;
// 		ar >> m_rectPtrn.right;
// 		ar >> m_rectPtrn.bottom;
// 
// 		ar >> m_dThreshold;
// 
// 		ar >> m_bDetected;
// 		ar >> m_dx;
// 		ar >> m_dy;
// 		ar >> m_iFindNum;
// 		ar >> m_iCntrNum;
// 		ar >> m_iCenMarginX;
// 		ar >> m_iCenMarginY;
// 		ar >> dCenOffsetX;
// 		ar >> dCenOffsetY;
// 		ar >> m_iPtrnMatchPriority;
// 
// 		if( iVer > 0 )		// jsyoon 20170327 add
// 		ar >> m_bPtrnLearn;
// 	}
// }
// 
// CPtrnInfo& CPtrnInfo::operator=(CPtrnInfo& rVal)
// {
// 	this->m_strName = rVal.m_strName;
// 
// 	imaqDuplicate(this->m_pImgOrg,		rVal.m_pImgOrg);
// 	imaqDuplicate(this->m_pImgOrgHSL,	rVal.m_pImgOrgHSL);	//uhkang 20100317
// 	imaqDuplicate(this->m_pImgOrgBW,	rVal.m_pImgOrgBW);	//uhkang 20100317
// 	imaqDuplicate(this->m_pImgScaled,	rVal.m_pImgScaled);
// 	imaqDuplicate(this->m_pImgHSLScaled,rVal.m_pImgHSLScaled);
// 	imaqDuplicate(this->m_pImgBWScaled,	rVal.m_pImgBWScaled);
// 
// 	this->m_rectPtrn.left	= rVal.m_rectPtrn.left;
// 	this->m_rectPtrn.top	= rVal.m_rectPtrn.top;
// 	this->m_rectPtrn.right	= rVal.m_rectPtrn.right;
// 	this->m_rectPtrn.bottom	= rVal.m_rectPtrn.bottom;
// 
// 	this->m_dThreshold	= rVal.m_dThreshold;
// 
// 	this->m_bDetected	= rVal.m_bDetected;
// 
// 	this->m_dx			= rVal.m_dx;
// 	this->m_dy			= rVal.m_dy;
// 
// 	this->m_dRealX		= rVal.m_dRealX;
// 	this->m_dRealY		= rVal.m_dRealY;
// 
// 	this->m_iFindNum	= rVal.m_iFindNum;
// 	this->m_iCntrNum	= rVal.m_iCntrNum;
// 
// 	this->m_iCenMarginX = rVal.m_iCenMarginX;
// 	this->m_iCenMarginY = rVal.m_iCenMarginY;
// 	this->dCenOffsetX	= rVal.dCenOffsetX;
// 	this->dCenOffsetY	= rVal.dCenOffsetY;
// 	this->m_iPtrnMatchPriority	= rVal.m_iPtrnMatchPriority;
// 	this->m_bPtrnLearn = rVal.m_bPtrnLearn;
// 	
// 	return *this;
// }
// 
// void CPtrnInfo::ClearPtrn()
// {	
// 	m_bDetected = FALSE;
// 
// 	imaqSetImageSize(m_pImgOrg,		 0, 0);
// 	imaqSetImageSize(m_pImgOrgHSL,	 0, 0);
// 	imaqSetImageSize(m_pImgOrgBW,	 0, 0);
// 	imaqSetImageSize(m_pImgScaled,	 0, 0);
// 	imaqSetImageSize(m_pImgHSLScaled,0, 0);
// 	imaqSetImageSize(m_pImgBWScaled, 0, 0);
// 
// 	PixelValue val;
// 	val.rgb = IMAQ_RGB_WHITE;			// Show all white to hide color pattern
// 	imaqFillImage(m_pImgOrg,		val, NULL);
// 	imaqFillImage(m_pImgOrgHSL,		val, NULL);
// 	imaqFillImage(m_pImgOrgBW,		val, NULL);
// 	imaqFillImage(m_pImgScaled,		val, NULL);
// 	imaqFillImage(m_pImgHSLScaled,	val, NULL);
// 	imaqFillImage(m_pImgBWScaled,	val, NULL);
// }
// 
// void CPtrnInfo::ClearPosData()
// {
// 	m_dx = m_dy = 0.;
// 	m_dRealX = m_dRealY = DUMMY_POS;
// }
// 
// void CPtrnInfo::ClearInfo()
// {
// 	m_strName.Empty();
// 
// 	m_rectPtrn = CRect(0, 0, 0, 0);
// 
// 	m_dThreshold = 500.0;
// 
// 	m_bDetected = FALSE;
// 	m_dx = m_dy = 0.0;
// 	m_dRealX = m_dRealY = DUMMY_POS;
// 
// 	m_iFindNum = m_iCntrNum = 1;
// 
// 	m_iCenMarginY = m_iCenMarginX = 1;
// 	dCenOffsetY   = dCenOffsetX   = 0;
// 	m_iPtrnMatchPriority		  = 0;
// 
// 	m_bPtrnLearn = FALSE;
// 
// }