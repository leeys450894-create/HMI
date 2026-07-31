#include "stdafx.h"
#include "Registry.h"

CRegistry::CRegistry(CString strCompany, CString strApp)
{
	m_strGroup	 = "Software";
	m_strCompany = strCompany;
	m_strApp	 = strApp;
}

CRegistry::~CRegistry()
{
}

//
void CRegistry::WriteString(CString strSubKey, CString strItem, CString strData)
{
	HKEY	hKey;
	DWORD	dw;

	CString	strKey = m_strGroup + "\\" + m_strCompany + "\\" + m_strApp + "\\" + strSubKey;

	if (::RegOpenKeyEx  (HKEY_CURRENT_USER, strKey, 0, KEY_WRITE, &hKey) != ERROR_SUCCESS)
		::RegCreateKeyEx(HKEY_CURRENT_USER, strKey,	0, REG_NONE, 
						REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dw);

	DWORD dwType = REG_SZ;
	BYTE* lpData = (BYTE*)(LPCTSTR(strData));
	dw = strData.GetLength();

	::RegSetValueEx(hKey, strItem, 0, dwType, lpData, dw);
	
	::RegCloseKey(hKey);
}

void CRegistry::WriteDWord(CString strSubKey, CString strItem, DWORD dWord)
{
	HKEY	hKey;
	DWORD	dw;

	CString	strKey = m_strGroup + "\\" + m_strCompany + "\\" + m_strApp + "\\" + strSubKey;

	if (::RegOpenKeyEx  (HKEY_CURRENT_USER, strKey, 0, KEY_WRITE, &hKey) != ERROR_SUCCESS)
		::RegCreateKeyEx(HKEY_CURRENT_USER, strKey,	0, REG_NONE, 
						REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dw);

	DWORD dwType = REG_DWORD;
	BYTE* lpData = (BYTE*)&dWord;
	dw = sizeof(dWord);

	::RegSetValueEx(hKey, strItem, 0, dwType, lpData, dw);
	
	::RegCloseKey (hKey);
}

BOOL CRegistry::ReadString(CString strSubKey, CString strItem, CString& strRead)
{
	HKEY  hKey;

	CString	strKey = m_strGroup + "\\" + m_strCompany + "\\" + m_strApp + "\\" + strSubKey;

	if (::RegOpenKeyEx  (HKEY_CURRENT_USER, strKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
		return FALSE;

	DWORD dw = 1024;
	DWORD dwType;
	BYTE* lpData = (BYTE*)calloc(dw, sizeof(TCHAR));

	BOOL bRet = ::RegQueryValueEx(hKey, strItem, NULL, &dwType, lpData, &dw) == ERROR_SUCCESS;

	if (bRet) 
	{
		if (dwType != REG_SZ)
			bRet = FALSE;
		else
			strRead = lpData;
	}

	::RegCloseKey (hKey);

	free(lpData); 

	return bRet;
}

BOOL CRegistry::ReadDWord(CString strSubKey, CString strItem, DWORD& dwRead)
{
	HKEY  hKey;

	CString	strKey = m_strGroup + "\\" + m_strCompany + "\\" + m_strApp + "\\" + strSubKey;

	if (::RegOpenKeyEx  (HKEY_CURRENT_USER, strKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS)
		return FALSE;

	DWORD dw = sizeof(DWORD);
	DWORD dwType;
	BYTE* lpData = (BYTE*)&dwRead;

	BOOL bRet = ::RegQueryValueEx(hKey, strItem, NULL, &dwType, lpData, &dw) == ERROR_SUCCESS;

	if (bRet)
	{
		if (dwType != REG_DWORD)
			bRet = FALSE;
		else
			;
	}

	::RegCloseKey (hKey);

	return bRet;
}

//
void CRegistry::WriteInt(CString strSubKey, CString strItem, int n)
{
	WriteDWord(strSubKey, strItem, (DWORD)n);
}

void CRegistry::WriteDouble(CString strSubKey, CString strItem, double d)
{
	CString cStr;
	cStr.Format("%f", d);
	WriteString(strSubKey, strItem, cStr);
}

//
void CRegistry::ReadString(CString strSubKey, CString strItem, CString& strRead, CString strDef)
{
	if (!ReadString(strSubKey, strItem, strRead))
		strRead = strDef;
}

void CRegistry::ReadDWord(CString strSubKey, CString strItem, DWORD& dwRead, DWORD dwDef)
{
	if (!ReadDWord(strSubKey, strItem, dwRead))
		dwRead = dwDef;
}

BOOL CRegistry::ReadInt(CString strSubKey, CString strItem, int& n)
{
	DWORD dw = n;
	BOOL bRet = ReadDWord(strSubKey, strItem, dw);
	
	n = dw;
	return bRet;
}

void CRegistry::ReadInt(CString strSubKey, CString strItem, int& n, int iDef)
{
	if (!ReadInt(strSubKey, strItem, n))
		n = iDef;
}

void CRegistry::ReadDouble(CString strSubKey, CString strItem, double& d, double dDef)
{
	CString cStr, cStrDef;

	cStrDef.Format("%f", dDef);
	ReadString(strSubKey, strItem,	cStr, cStrDef);
	
	d = atof(cStr);
}
