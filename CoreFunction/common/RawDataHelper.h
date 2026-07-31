#pragma once

#include <map>
#include <vector>

typedef map<CString, CString>			INI_KEYVALUE_PAIR;
typedef map<CString, INI_KEYVALUE_PAIR>	INI_CATEGORY_PAIR;
#define MAKE_KEYVALUE_PAIR(K, V)		make_pair<CString, CString>(K, V)
#define MAKE_CATEGORY_PAIR(S, V)		make_pair<CString, INI_KEYVALUE_PAIR>(S, V)

class CRawDataHelper
{
	CString				m_strFilePath;
	INI_CATEGORY_PAIR	m_mapData;

public:
	bool	IsReadEnable();
    LPCTSTR GetFilePath();
	void	SetFilePath(LPCTSTR lpszFilePath);
	bool	Parse(int iLimit = INFINITE);
	long	GetCount();

private:
	void	Clear();
	void	Load(LPCTSTR szCategory, LPCTSTR szKey, bool *nValue, bool nDefValue = 0);
	void	Load(LPCTSTR szCategory, LPCTSTR szKey, int *nValue, int nDefValue = 0);
	void	Load(LPCTSTR szCategory, LPCTSTR szKey, long *nValue, long nDefValue = 0);
	void	Load(LPCTSTR szCategory, LPCTSTR szKey, float *fValue, float fDefValue = 0.f);
	void	Load(LPCTSTR szCategory, LPCTSTR szKey, double *fValue, double fDefValue = 0.f);
	void	Load(LPCTSTR szCategory, LPCTSTR szKey, CString *strValue, CString strDefValue = _T(""));
	CString	*GetValue(LPCTSTR szCategory, LPCTSTR szKey);

public:
	template <class T>
	void Link(bool bSave, LPCTSTR szCategory, LPCTSTR szKey, T *value, T init_value)
	{
		//if(bSave)
		//	Save(szCategory, szKey, value);
		//else
			Load(szCategory, szKey, value, init_value);
	}	

public:
	CRawDataHelper(void);
	~CRawDataHelper(void);
};