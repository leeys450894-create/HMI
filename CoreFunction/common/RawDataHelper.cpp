#include "pch.h"
#include "RawDataHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CRawDataHelper::CRawDataHelper(void)
{
}

CRawDataHelper::~CRawDataHelper(void)
{
}

bool CRawDataHelper::IsReadEnable()
{
	return (PathFileExists(m_strFilePath) == TRUE) ? true : false;
}

LPCTSTR CRawDataHelper::GetFilePath()
{
	return m_strFilePath;
}

void CRawDataHelper::SetFilePath(LPCTSTR lpszFilePath)
{
	m_strFilePath = lpszFilePath;
}


bool CRawDataHelper::Parse(int iLimit)
{
	CStdioFile file;
	if(file.Open(m_strFilePath, CFile::modeRead) == FALSE)
		return false;
	
	Clear();

	auto itr = m_mapData.begin();

	int iCount = 0;

	while (1)
	{
		if (iCount >= iLimit && iLimit != INFINITE)
			break;
		
		CString strLine;
		if(file.ReadString(strLine) == TRUE)
		{
			// 문장 길이가 1개이거나 첫번째 문자를 확인한다.
			if (strLine.GetAt(0) == _T(';') || strLine.GetLength() == 1)
				continue;

			if (strLine.GetAt(0) == _T('['))
			{
				iCount++;

				strLine.Replace(_T("["), _T(""));

				CString strData;
				AfxExtractSubString(strData, strLine, 0, _T(']'));

				itr = m_mapData.find(strData);
				if (itr == m_mapData.end())
				{
					INI_KEYVALUE_PAIR kvPair;
					m_mapData.insert(make_pair(strData, kvPair));

					itr = m_mapData.find(strData);
				}
			}
			else
			{
				strLine.Replace(_T(" "), _T(""));
				strLine.Replace(_T("\n"), _T(""));

				CString k, v;
				AfxExtractSubString(k, strLine, 0, _T('='));
				AfxExtractSubString(v, strLine, 1, _T('='));

				INI_KEYVALUE_PAIR& kvPair = itr->second;

				auto kvItr = kvPair.find(k);
				if (kvItr == kvPair.end())
				{
					kvPair.insert(make_pair(k, v));
				}
				else
				{
					kvItr->second = v;
				}
			}
		}
		else
		{
			break;
		}
	}

	file.Close();
	return true;
}

long CRawDataHelper::GetCount()
{
	return (long)m_mapData.size();
}



void CRawDataHelper::Clear()
{
	// 기존에 갖고 있는 데이터 정보를 모두 삭제한다.
	for (auto itr = m_mapData.begin(); itr != m_mapData.end(); itr++)
	{
		itr->second.clear();
	}
	m_mapData.clear();
}

void CRawDataHelper::Load(LPCTSTR szCategory, LPCTSTR szKey, bool *nValue, bool nDefValue)
{
	CString* v = GetValue(szCategory, szKey);
	if (v == nullptr)
	{
		*nValue = nDefValue;
		return;
	}
	*nValue = (v->Compare(_T("0")) == 0) ? false : true;
}

void CRawDataHelper::Load(LPCTSTR szCategory, LPCTSTR szKey, int *nValue, int nDefValue)
{
	CString* v = GetValue(szCategory, szKey);
	if (v == nullptr)
	{
		*nValue = nDefValue;
		return;
	}
	*nValue = _ttoi(*v);
}

void CRawDataHelper::Load(LPCTSTR szCategory, LPCTSTR szKey, long *nValue, long nDefValue)
{
	CString* v = GetValue(szCategory, szKey);
	if (v == nullptr)
	{
		*nValue = nDefValue;
		return;
	}
	*nValue = _ttoi(*v);
}

void CRawDataHelper::Load(LPCTSTR szCategory, LPCTSTR szKey, float *fValue, float fDefValue)
{
	CString* v = GetValue(szCategory, szKey);
	if (v == nullptr)
	{
		*fValue = fDefValue;
		return;
	}
	*fValue = (float)_ttof(*v);
}

void CRawDataHelper::Load(LPCTSTR szCategory, LPCTSTR szKey, double *fValue, double fDefValue)
{
	CString* v = GetValue(szCategory, szKey);
	if (v == nullptr)
	{
		*fValue = fDefValue;
		return;
	}
	*fValue = _ttof(*v);
}

void CRawDataHelper::Load(LPCTSTR szCategory, LPCTSTR szKey, CString *strValue, CString strDefValue)
{
	CString* v = GetValue(szCategory, szKey);
	if (v == nullptr)
	{
		*strValue = strDefValue;
		return;
	}
	strValue->Format(_T("%s"), (*v).GetString());
}

CString* CRawDataHelper::GetValue(LPCTSTR szCategory, LPCTSTR szKey)
{
	auto secItr = m_mapData.find(szCategory);
	if (secItr == m_mapData.end())
		return nullptr;

	auto kvItr = secItr->second.find(szKey);
	if (kvItr == secItr->second.end())
		return nullptr;

	return &kvItr->second;
}
