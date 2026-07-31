#if !defined(CCREGISTRY_H__INCLUDED_)
#define CCREGISTRY_H__INCLUDED_

// Write or read from/to registry 
// HKEY_CURRENT_USER - Software - STR_COMPANY - STR_APPNAME - strSubKey - strItem 
class CRegistry
{
public:
	CRegistry(CString strCompany, CString strApp);
	~CRegistry();

	// Write
	void	WriteString(CString strSubKey, CString strItem, CString strData);
	void	WriteDWord (CString strSubKey, CString strItem, DWORD dWord);
	void	WriteInt   (CString strSubKey, CString strItem, int n);
	void	WriteDouble(CString strSubKey, CString strItem, double d);

	// Read and return FALSE if fails
	BOOL	ReadString (CString strSubKey, CString strItem,	CString& strRead);
	BOOL	ReadDWord  (CString strSubKey, CString strItem,	DWORD& dwRead);
	BOOL	ReadInt    (CString strSubKey, CString strItem,	int& n);

	// Read with default values
	void	ReadString (CString strSubKey, CString strItem,	CString& strRead, CString strDef);
	void	ReadDWord  (CString strSubKey, CString strItem,	DWORD& dwRead, DWORD dwDef);
	void	ReadInt    (CString strSubKey, CString strItem,	int& n, int iDef);
	void	ReadDouble(CString strSubKey, CString strItem, double& d, double dDef);

protected:
	CString m_strGroup;
	CString m_strCompany;	// Company name
	CString m_strApp;		// Application name
};

#endif // CCREGISTRY_H__INCLUDED_
