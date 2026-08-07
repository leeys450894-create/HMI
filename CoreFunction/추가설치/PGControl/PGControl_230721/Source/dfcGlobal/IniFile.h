#pragma once 

class	AFX_EXT_CLASS	C_IniFile
{
	public:
		CString	m_path;
		DWORD	m_dwData;

	public:
		C_IniFile (BOOL bOpen = TRUE);
		C_IniFile (CString path, BOOL bOpen = TRUE);
		C_IniFile (const C_IniFile &o);
		virtual ~C_IniFile ();

	public:
		CString	GetProfileString (LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszDefault = 0);
		BOOL	WriteProfileString (LPCTSTR lpszSection, LPCTSTR lpszEntry, LPCTSTR lpszValue);

		int		GetProfileInt (LPCTSTR lpszSection, LPCTSTR lpszEntry, int nDefault = 0);
		BOOL	WriteProfileInt (LPCTSTR lpszSection, LPCTSTR lpszEntry, int nValue);

		INT64	GetProfileInt64 (LPCTSTR lpszSection, LPCTSTR lpszEntry, INT64 nDefault = 0);
		BOOL	WriteProfileInt64 (LPCTSTR lpszSection, LPCTSTR lpszEntry, INT64 nValue);

		double	GetProfileDouble (LPCTSTR lpszSection, LPCTSTR lpszEntry, double fDefault = 0);
		BOOL	WriteProfileDouble (LPCTSTR lpszSection, LPCTSTR lpszEntry, double fValue);

		CPoint	GetProfilePoint (LPCTSTR lpszSection, LPCTSTR lpszEntry, CPoint ptDefault = CPoint(0,0));
		BOOL	WriteProfilePoint (LPCTSTR lpszSection, LPCTSTR lpszEntry, CPoint pt);

		CRect	GetProfileRect (LPCTSTR lpszSection, LPCTSTR lpszEntry, CRect rcDefault = CRect(0,0,0,0));
		BOOL	WriteProfileRect (LPCTSTR lpszSection, LPCTSTR lpszEntry, CRect rc);

		//BOOL	GetProfileTol (LPCTSTR lpszSection, LPCTSTR lpszEntry, C_Tol &tol);
		//BOOL	WriteProfileTol (LPCTSTR lpszSection, LPCTSTR lpszEntry, C_Tol tol);

		//BOOL	GetProfileGdt (LPCTSTR lpszSection, LPCTSTR lpszEntry, C_Gdt &gdt);
		//BOOL	WriteProfileGdt (LPCTSTR lpszSection, LPCTSTR lpszEntry, C_Gdt gdt);

		int		GetProfileSection (LPCTSTR lpszSection, CStringArray &strings);
		void	EmptyProfileSection (LPCTSTR lpszSection);

		int		GetProfileSectionNames (CStringArray &strings);
		void	EmptyFile ();
};
