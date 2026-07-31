#include "pch.h"
#include "IniFile.h"

#ifdef _DEBUG 
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

C_IniFile::C_IniFile (BOOL bOpen)
{
	m_dwData = bOpen;
}

C_IniFile::C_IniFile (CString path, BOOL bOpen)
{
	m_path = path;
	m_dwData = bOpen;
}

C_IniFile::C_IniFile (const C_IniFile &o)
{
	m_path		= o.m_path;
	m_dwData	= o.m_dwData;
}

C_IniFile::~C_IniFile ()
{
}

CString	C_IniFile::GetProfileString (LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault)
{
	if (m_path.IsEmpty ()) return 0;

	TCHAR	pBuf[1024];
	::GetPrivateProfileString (lpszSection, lpszEntry, lpszDefault, pBuf, 1024, m_path);
	return CString(pBuf);
}

BOOL	C_IniFile::WriteProfileString (LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue)
{
	if (m_path.IsEmpty ()) return 0;

	return ::WritePrivateProfileString (lpszSection, lpszEntry, lpszValue, m_path);
}

int		C_IniFile::GetProfileInt (LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault)
{
	CString	string = GetProfileString (lpszSection, lpszEntry, dfcStr (_T("%d"), nDefault));
	return _tstoi (string);
}

BOOL	C_IniFile::WriteProfileInt (LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue)
{
	return WriteProfileString (lpszSection, lpszEntry, dfcStr (nValue));
}

INT64	C_IniFile::GetProfileInt64 (LPCTSTR lpszSection, LPCTSTR lpszEntry, INT64 nDefault)
{
	CString	string = GetProfileString (lpszSection, lpszEntry, dfcStr (_T("%I64d"), nDefault));
	return ::_tstoi64 (string);
}

BOOL	C_IniFile::WriteProfileInt64 (LPCTSTR lpszSection, LPCTSTR lpszEntry, INT64 nValue)
{
	return WriteProfileString (lpszSection, lpszEntry, dfcStr (_T("%I64d"), nValue));
}

double	C_IniFile::GetProfileDouble (LPCTSTR lpszSection, LPCTSTR lpszEntry, double fDefault)
{
	CString	string = GetProfileString (lpszSection, lpszEntry, dfcStr (_T("%lf"), fDefault));
	return _tstof (string);
}

BOOL	C_IniFile::WriteProfileDouble (LPCTSTR lpszSection, LPCTSTR lpszEntry, double fValue)
{
	return WriteProfileString (lpszSection, lpszEntry, dfcStr (_T("%lf"), fValue));
}

CPoint	C_IniFile::GetProfilePoint (LPCTSTR lpszSection, LPCTSTR lpszEntry, CPoint ptDefault)
{
	CString	string = GetProfileString (lpszSection, lpszEntry, dfcStr (_T("%d %d"), ptDefault.x, ptDefault.y));
	_stscanf (string, _T("%d %d"), &ptDefault.x, &ptDefault.y);
	return ptDefault;
}

BOOL	C_IniFile::WriteProfilePoint (LPCTSTR lpszSection, LPCTSTR lpszEntry, CPoint pt)
{
	return WriteProfileString (lpszSection, lpszEntry, dfcStr (_T("%d %d"), pt.x, pt.y));
}

CRect	C_IniFile::GetProfileRect (LPCTSTR lpszSection, LPCTSTR lpszEntry, CRect rcDefault)
{
	CString	string = GetProfileString (lpszSection, lpszEntry,
									   dfcStr (_T("%d %d %d %d"), rcDefault.left, rcDefault.top, rcDefault.right, rcDefault.bottom));
	_stscanf (string, _T("%d %d %d %d"), &rcDefault.left, &rcDefault.top, &rcDefault.right, &rcDefault.bottom);
	return rcDefault;
}

BOOL	C_IniFile::WriteProfileRect (LPCTSTR lpszSection, LPCTSTR lpszEntry, CRect rc)
{
	return WriteProfileString (lpszSection, lpszEntry, dfcStr (_T("%d %d %d %d"), rc.left, rc.top, rc.right, rc.bottom));
}

//BOOL	C_IniFile::GetProfileTol (LPCTSTR lpszSection, LPCTSTR lpszEntry, C_Tol &tol)
//{
//	CString	string = GetProfileString (lpszSection, lpszEntry);
//	if (string.IsEmpty ()) return FALSE;
//
//	return _stscanf (string, _T("%lf %lf"), &tol.m_fUp, &tol.m_fLo) == 2;
//}
//
//BOOL	C_IniFile::WriteProfileTol (LPCTSTR lpszSection, LPCTSTR lpszEntry, C_Tol tol)
//{
//	return WriteProfileString (lpszSection, lpszEntry, dfcStr (_T("%lf %lf"), tol.m_fUp, tol.m_fLo));
//}
//
//BOOL	C_IniFile::GetProfileGdt (LPCTSTR lpszSection, LPCTSTR lpszEntry, C_Gdt &gdt)
//{
//	CString	string = GetProfileString (lpszSection, lpszEntry);
//	if (string.IsEmpty ()) return FALSE;
//
//	return _stscanf (string, _T("%lf"), &gdt.m_fTolZon) == 1;
//}
//
//BOOL	C_IniFile::WriteProfileGdt (LPCTSTR lpszSection, LPCTSTR lpszEntry, C_Gdt gdt)
//{
//	return WriteProfileString (lpszSection, lpszEntry, dfcStr (_T("%lf"), gdt.m_fTolZon));
//}

int		C_IniFile::GetProfileSection (LPCTSTR lpszSection, CStringArray &strings)
{
	const	int		nBuf = 10000;
	TCHAR	pBuf[nBuf];
	DWORD	nReadCount = GetPrivateProfileSection (lpszSection, pBuf, nBuf, m_path), dwPos = 0;

	while (dwPos < nReadCount) {
		CString	string = pBuf + dwPos;
		dwPos += string.GetLength () + 1;
		string.Trim ();
		if (string.IsEmpty () || string[0] == ';' || string.Find ('=') == -1) continue;

		strings.Add (string);
	}

	return (INT)strings.GetSize ();
}

void	C_IniFile::EmptyProfileSection (LPCTSTR lpszSection)
{
	WritePrivateProfileSection (lpszSection, 0, m_path);
}

int		C_IniFile::GetProfileSectionNames (CStringArray &strings)
{
	const	int		nBuf = 10000;
	TCHAR	pBuf[nBuf];
	memset (pBuf, 0, sizeof (TCHAR)*nBuf);

	DWORD	nReadCount = GetPrivateProfileSectionNames (pBuf, nBuf, m_path), dwPos = 0;
	if (nReadCount == 0) return 0;

	while (dwPos < nReadCount) {
		CString	string = pBuf + dwPos;
		dwPos += string.GetLength () + 1;
		string.Trim ();
		if (string.IsEmpty ()) continue;

		strings.Add (string);
	}

	return (INT)strings.GetSize ();
}

void	C_IniFile::EmptyFile ()
{
	CStringArray	names;
	GetProfileSectionNames (names);

	for (int i = 0; i < names.GetSize (); i++) {
		EmptyProfileSection (names[i]);
	}
}
