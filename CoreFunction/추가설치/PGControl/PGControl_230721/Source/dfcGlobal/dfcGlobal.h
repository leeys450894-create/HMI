#pragma once

#include "../dfcGlobal/IniFile.h"
#include "../dfcGlobal/AlarmList.h"
#include "../dfcGlobal/SystemParamShare.h"
#include "../dfcGlobal/ButtonEx.h"

#ifndef	DFC_GLOBAL_LIB
	#ifdef	_DEBUG
		#ifdef _M_X64
			#pragma comment (lib, "../../x64/Debug/dfcGlobal/dfcGlobald.lib")
		#else
			#pragma comment (lib, "../../x86/Debug/dfcGlobal/dfcGlobald.lib")
		#endif 
	#else
		#ifdef _M_X64
			#pragma comment (lib, "../../x64/Release/dfcGlobal/dfcGlobal.lib")
		#else
			#pragma comment (lib, "../../x86/Release/dfcGlobal/dfcGlobal.lib")
		#endif 
	#endif
#endif

#ifndef WM_ALARM_EVENT_MESSAGE
#define WM_ALARM_EVENT_MESSAGE			WM_USER + 101
#endif

#ifndef WM_FUNC_OK_MESSAGE
#define WM_FUNC_OK_MESSAGE				WM_USER + 102
#endif

#ifndef WM_FUNC_CANCEL_MESSAGE
#define WM_FUNC_CANCEL_MESSAGE			WM_USER + 103
#endif

#ifndef WM_FUNC_UPDATE_GRABDISPLY
#define WM_FUNC_UPDATE_GRABDISPLY		WM_USER + 104
#endif

__declspec (dllexport)	void	dfcThrow (LPCTSTR fmt, ...);
__declspec (dllexport)	void	dfcThrowEx (UINT nID, ...);
__declspec (dllexport)	void	dfcThrowEx (LPCTSTR fmt, ...);

__declspec (dllexport)	CString	dfcStr (LPCTSTR fmt, ...);
__declspec (dllexport)	CString	dfcStr (int nVal);
__declspec (dllexport)	CString	dfcStr (double fVal, BOOL bTrim = 0);
__declspec (dllexport)	CString	dfcStr (CWnd& wnd);
__declspec (dllexport)	CString	dfcStrEx (UINT nID, ...);
__declspec (dllexport)	CString	dfcStrEx (LPCTSTR fmt, ...);

__declspec (dllexport)  CString	dfcGetFontName ();

__declspec (dllexport)	CString	dfcTokenize (LPCTSTR lpszSrc, TCHAR chSep, int nIndex = -1);
__declspec (dllexport)	CString	dfcTokenize (LPCTSTR lpszSrc, LPCTSTR lpszSep, int nIndex);
__declspec (dllexport)	int		dfcTokenize (LPCTSTR lpszSrc, TCHAR chSep, CStringArray& tokens);
__declspec (dllexport)	int		dfcTokenize (LPCTSTR lpszSrc, LPCTSTR lpszSep, CStringArray& tokens);
__declspec (dllexport)	int		dfcTokenizeCount (LPCTSTR lpszSrc, LPCTSTR lpszSep);
__declspec (dllexport)	CString	dfcPathToFolder (LPCTSTR lpszPath);
__declspec (dllexport)	CString	dfcPathToName (LPCTSTR lpszPath);
__declspec (dllexport)	CString	dfcPathToTitle (LPCTSTR lpszPath);
__declspec (dllexport)	CString	dfcPathToExt (LPCTSTR lpszPath);
__declspec (dllexport)	CString	dfcReplaceByFolder (LPCTSTR lpszPath, LPCTSTR lpszFolder);
__declspec (dllexport)	CString	dfcReplaceByName (LPCTSTR lpszPath, LPCTSTR lpszName);
__declspec (dllexport)	CString	dfcReplaceByTitle (LPCTSTR lpszPath, LPCTSTR lpszTitle);
__declspec (dllexport)	CString	dfcReplaceByExt (LPCTSTR lpszPath, LPCTSTR lpszExt);

__declspec (dllexport)	CString	dfcExeFolder ();
__declspec (dllexport)	CString	dfcIniFolder ();
__declspec (dllexport)	CString	dfcTemplateFolder();
__declspec (dllexport)	CString	dfcBackupFolder ();
__declspec (dllexport)	CString	dfcResultFolder ();
__declspec (dllexport)  CString	dfcLogFolder ();
__declspec (dllexport)	CString	dfcIniFile (LPCTSTR lpszTitle = 0);
__declspec (dllexport)	int		dfcGetFiles (CString strFolder, CStringArray& names, BOOL bIncludeDir = FALSE, CString strExt = _T(""));

__declspec (dllexport)	BOOL	dfcIsFileRunning (CString path);
__declspec (dllexport)	HWND	dfcGetExcel (CString path);

__declspec (dllexport)	BOOL	dfcCheckUnicode (LPCTSTR lpszPath);
__declspec (dllexport)	char*	dfcUniToAnsi (LPCTSTR lpszStr);
__declspec (dllexport)	void	dfcPeekAndPump (DWORD dwDelay = 0);
__declspec (dllexport)	CString	dfcCurrentTime (BOOL bYMD, BOOL bHMS);
__declspec (dllexport)	CString	dfcCurrentTime (TCHAR cSep = 0);
__declspec (dllexport)	CString	dfcElapsedTime (DWORD dwTickStart);

__declspec (dllexport)	void	dfcSorting (CStringArray& strBases);

__declspec (dllexport)	CRect	dfcWindowRect (CWnd* pWnd, CWnd* pWndMap = 0);
__declspec (dllexport)	CRect	dfcClientRect (CWnd* pWnd, CWnd* pWndMap = 0);
__declspec (dllexport)	CRect	dfcAdjustRect (CRect rcSrc, CRect rcTarg);

__declspec (dllexport)	void	dfcPrintOrientation (short nOrientation = DMORIENT_PORTRAIT);
__declspec (dllexport)	CString	dfcGetLastErrorString ();
__declspec (dllexport)	void	dfcMakeFullPath (CString strfullPath);
__declspec (dllexport)	BOOL	dfcIsPositionInMonitor (CPoint pt);
__declspec (dllexport)	BOOL	dfcCreateDirectory (LPCTSTR szPath);
__declspec (dllexport)	int		dfcGetDiffernceDay (LPCTSTR szPath);	// 현재시간과 파일생성시간의 날짜차이를 반환
__declspec (dllexport)	BOOL	dfcDeleteAllFolder (CString strPath);	// 경로내의 하위폴더 및 파일 전부삭제 

__declspec (dllexport)	void	WriteString (CStdioFile *File, CString string);
__declspec (dllexport)	void	WriteInt (CStdioFile *File, int nVal);
__declspec (dllexport)	void	WriteDouble (CStdioFile *File, double fVal);
__declspec (dllexport)	CString	ReadString (CStdioFile *File);
__declspec (dllexport)	int		ReadInt (CStdioFile *File);
__declspec (dllexport)	double	ReadDouble (CStdioFile *File);

#define STATIC_CLASS					_T("Static")
#define BUTTON_CLASS					_T("Button")
#define LIST_CLASS						_T("SysListView32")
#define EDIT_CLASS						_T("Edit")
#define GRID_CLASS						_T("MFCGridCtrl")
#define COMBOBOX_CLASS					_T("ComboBox")
#define SCROLLBAR_CLASS					_T("ScrollBar")
#define MFCEDITBROWSE_CLASS				_T("MFCEditBrowse")

#define SAFE_DELETE(x)	if(x) {delete   x; x=0;}
#define SAFE_DELETES(x) if(x) {delete []x; x=0;}
#define SAFE_DESTROY(x) if(x) { (x)->DestroyWindow (); delete x; x=0;}

#define TIMEOUT_3					3000
#define TIMEOUT_5					5000
#define TIMEOUT_10					10000
#define TIMEOUT_15					15000
#define TIMEOUT_20					20000
#define TIMEOUT_30					30000
#define TIMEOUT_60					60000
#define TIMEOUT_180					180000
#define TIMEOUT_300					300000

#define COLOR_BLACK					RGB (0,0,0)
#define COLOR_WHITE					RGB (255,255,255)
#define COLOR_GRAY_BRIGHT			RGB (234,234,234)
#define COLOR_GRAY					RGB (192,192,192)
#define COLOR_GRAY_DARK				RGB (128,128,128)
#define COLOR_RED					RGB (255,0,0)
#define COLOR_RED_DARK				RGB (128,0,0)
#define COLOR_GREEN					RGB (0,255,0)
#define COLOR_GREEN_DARK			RGB (0,51,0)
#define COLOR_GREEN_LIGHT			RGB (181,255,181)
#define COLOR_YELLOW				RGB (255,255,0)
#define COLOR_YELLOW_DARK			RGB (255,210,0)
#define COLOR_BLUE					RGB (0,0,255)
#define COLOR_STEEL_BLUE			RGB (99, 184, 255)
#define COLOR_SKYBLUE_BRIGHT		RGB (152,192,235)
#define COLOR_SKYBLUE				RGB (0,130,200)
#define COLOR_SKYBLUE_DARK			RGB (49,85,153)

#define COLOR_USER_FRAME			RGB (85, 85, 85)//RGB (100, 100, 100)
#define COLOR_USER_BUTTON			RGB (124, 124, 124)
#define COLOR_USER_BOTTOM			RGB (128,128,128)
#define COLOR_USER_GREEN			RGB (181,225,181)