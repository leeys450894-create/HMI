//#include "pch.h"
#include "pch.h"
#include "IniHelper.h"

#ifndef _UNICODE
#include <conio.h>
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CIniHelper::CIniHelper(void)
{
}

CIniHelper::~CIniHelper(void)
{
}

bool CIniHelper::IsReadEnable()
{
	return (PathFileExistsW(m_strFilePath) == TRUE) ? true : false;

   //return ::FileExists(m_strFilePath);
}

LPCTSTR CIniHelper::GetFilePath()
{
    return m_strFilePath;
}

void CIniHelper::SetFilePath(LPCTSTR lpszFilePath)
{
    m_strFilePath = lpszFilePath;
}

CString CIniHelper::TrimSymbol(LPCTSTR szName)
{
	// [] 은 Category 로 쓰이므로 분류명으로 사용되서는 안되는 내용이다.
	// <> 로 바꾸어주자
	CString strName(szName);

	strName.Replace(_T("["), _T("$#<"));
	strName.Replace(_T("]"), _T("$#>"));

	return strName;
}


short CIniHelper::__LoadSHORT(LPCTSTR szCategory, LPCTSTR szKey, short nDefValue)
{
	return (short)GetPrivateProfileInt(TrimSymbol(szCategory), TrimSymbol(szKey), nDefValue, m_strFilePath);
}
bool CIniHelper::__LoadBOOL(LPCTSTR szCategory, LPCTSTR szKey, bool nDefValue)
{
	return (GetPrivateProfileInt(TrimSymbol(szCategory), TrimSymbol(szKey), nDefValue, m_strFilePath) == 1) ? true : false;
}
int CIniHelper::__LoadINT(LPCTSTR szCategory, LPCTSTR szKey, int nDefValue)
{
	return GetPrivateProfileInt(TrimSymbol(szCategory), TrimSymbol(szKey), nDefValue, m_strFilePath);
}
CString CIniHelper::__LoadSTRING(LPCTSTR szCategory, LPCTSTR szKey, CString nDefValue)
{
	TCHAR strValue[MAX_PATH];
	GetPrivateProfileString(TrimSymbol(szCategory), TrimSymbol(szKey), nDefValue, strValue, MAX_PATH - 1, m_strFilePath);
	return strValue;
}
float CIniHelper::__LoadFLOAT(LPCTSTR szCategory, LPCTSTR szKey, float dDefValue)
{
	TCHAR szVal[MAX_PATH - 1];

	CString strTemp;
	strTemp.Format(_T("%f"), dDefValue);
	GetPrivateProfileString(TrimSymbol(szCategory), TrimSymbol(szKey), strTemp, szVal, MAX_PATH - 1, m_strFilePath);

	float dResult;
	swscanf_s(szVal, _T("%f"), &dResult);

	return dResult;
}
double CIniHelper::__LoadDOUBLE(LPCTSTR szCategory, LPCTSTR szKey, double dDefValue)
{
	TCHAR szVal[MAX_PATH - 1];

	CString strTemp;
	strTemp.Format(_T("%lf"), dDefValue);
	GetPrivateProfileString(TrimSymbol(szCategory), TrimSymbol(szKey), strTemp, szVal, MAX_PATH - 1, m_strFilePath);

	double dResult;
	swscanf_s(szVal, _T("%lf"), &dResult);

	return dResult;
}
CPoint CIniHelper::__LoadPOINT(LPCTSTR szCategory, LPCTSTR szKey, CPoint nDefValue)
{
	TCHAR szVal[MAX_PATH - 1];

	CString strTemp;
	strTemp.Format(_T("%d,%d"), nDefValue.x, nDefValue.y);
	GetPrivateProfileString(TrimSymbol(szCategory), TrimSymbol(szKey), strTemp, szVal, MAX_PATH - 1, m_strFilePath);

	CPoint ptResult;
	swscanf_s(szVal, _T("%d,%d"), &ptResult.x, &ptResult.y);

	return ptResult;
}
CRect CIniHelper::__LoadRECT(LPCTSTR szCategory, LPCTSTR szKey, CRect nDefValue)
{
	TCHAR szVal[MAX_PATH - 1];

	CString strTemp;
	strTemp.Format(_T("%d,%d,%d,%d"), nDefValue.left, nDefValue.top, nDefValue.right, nDefValue.bottom);
	GetPrivateProfileString(TrimSymbol(szCategory), TrimSymbol(szKey), strTemp, szVal, MAX_PATH - 1, m_strFilePath);

	CRect rtResult(0, 0, 0, 0);
	swscanf_s(szVal, _T("%d,%d,%d,%d"), &rtResult.left, &rtResult.top, &rtResult.right, &rtResult.bottom);

	return rtResult;
}
ULONGLONG CIniHelper::__LoadULONGLONG(LPCTSTR szCategory, LPCTSTR szKey, ULONGLONG nDefValue)
{
	TCHAR szVal[MAX_PATH - 1];
	
	CString strDefault;
	strDefault.Format(_T("%l64d"), nDefValue);
	GetPrivateProfileString(TrimSymbol(szCategory), TrimSymbol(szKey), strDefault, szVal, MAX_PATH - 1, m_strFilePath);

	return _ttoi64(szVal);
}

CString CIniHelper::__LoadTCHAR(LPCTSTR szCategory, LPCTSTR szKey, TCHAR nDefValue[])
{
	TCHAR strValue[MAX_PATH];
	GetPrivateProfileString(TrimSymbol(szCategory), TrimSymbol(szKey), nDefValue, strValue, MAX_PATH - 1, m_strFilePath);
	return strValue;
}

void CIniHelper::__SaveSHORT(LPCTSTR szCategory, LPCTSTR szKey, short nValue)
{
	CString strValue;
	strValue.Format(_T("%d"), nValue);
	WritePrivateProfileString(TrimSymbol(szCategory), TrimSymbol(szKey), strValue, m_strFilePath);
}
void CIniHelper::__SaveBOOL(LPCTSTR szCategory, LPCTSTR szKey, bool nValue)
{
	CString strValue;
	strValue.Format(_T("%d"), nValue);
	WritePrivateProfileString(TrimSymbol(szCategory), TrimSymbol(szKey), strValue, m_strFilePath);
}
void CIniHelper::__SaveINT(LPCTSTR szCategory, LPCTSTR szKey, int nValue)
{
	CString strValue;
	strValue.Format(_T("%d"), nValue);
	WritePrivateProfileString(TrimSymbol(szCategory), TrimSymbol(szKey), strValue, m_strFilePath);
}
void CIniHelper::__SaveSTRING(LPCTSTR szCategory, LPCTSTR szKey, CString strValue)
{
	WritePrivateProfileString(TrimSymbol(szCategory), TrimSymbol(szKey), strValue, m_strFilePath);
}
void CIniHelper::__SaveFLOAT(LPCTSTR szCategory, LPCTSTR szKey, float dValue)
{
	CString strValue;
	strValue.Format(_T("%f"), dValue);
	WritePrivateProfileString(TrimSymbol(szCategory), TrimSymbol(szKey), strValue, m_strFilePath);
}
void CIniHelper::__SaveDOUBLE(LPCTSTR szCategory, LPCTSTR szKey, double dValue)
{
	CString strValue;
	strValue.Format(_T("%lf"), dValue);
	WritePrivateProfileString(TrimSymbol(szCategory), TrimSymbol(szKey), strValue, m_strFilePath);
}
void CIniHelper::__SavePOINT(LPCTSTR szCategory, LPCTSTR szKey, CPoint ptValue)
{
	CString strValue;
	strValue.Format(_T("%d,%d"), ptValue.x, ptValue.y);
	WritePrivateProfileString(TrimSymbol(szCategory), TrimSymbol(szKey), strValue, m_strFilePath);
}
void CIniHelper::__SaveRECT(LPCTSTR szCategory, LPCTSTR szKey, CRect rtValue)
{
	CString strValue;
	strValue.Format(_T("%d,%d,%d,%d"), rtValue.left, rtValue.top, rtValue.right, rtValue.bottom);
	WritePrivateProfileString(TrimSymbol(szCategory), TrimSymbol(szKey), strValue, m_strFilePath);
}
void CIniHelper::__SaveULONGLONG(LPCTSTR szCategory, LPCTSTR szKey, ULONGLONG nValue)
{
	CString strValue;
	strValue.Format(_T("%l64d"), nValue);
	WritePrivateProfileString(TrimSymbol(szCategory), TrimSymbol(szKey), strValue, m_strFilePath);
}




void CIniHelper::Read(LPCTSTR szCategory, LPCTSTR szKey, short *nValue, short nDefValue)
{
	*nValue = __LoadSHORT(szCategory, szKey, nDefValue);
}
void CIniHelper::Read(LPCTSTR szCategory, LPCTSTR szKey, bool *nValue, bool nDefValue)
{
	*nValue = __LoadBOOL(szCategory, szKey, nDefValue);
}
void CIniHelper::Read(LPCTSTR szCategory, LPCTSTR szKey, int *nValue, int nDefValue)
{
	*nValue = __LoadINT(szCategory, szKey, nDefValue);
}
void CIniHelper::Read(LPCTSTR szCategory, LPCTSTR szKey, ULONGLONG *nValue, ULONGLONG nDefValue)
{
	*nValue = __LoadULONGLONG(szCategory, szKey, nDefValue);
}
void CIniHelper::Read(LPCTSTR szCategory, LPCTSTR szKey, long *nValue, long nDefValue)
{
	*nValue = __LoadINT(szCategory, szKey, (int)nDefValue);
}
void CIniHelper::Read(LPCTSTR szCategory, LPCTSTR szKey, unsigned short *nValue, unsigned short nDefValue)
{
	*nValue = (unsigned short)__LoadINT(szCategory, szKey, (int)nDefValue);
}
void CIniHelper::Read(LPCTSTR szCategory, LPCTSTR szKey, CString *strValue, CString strDefValue)
{
	*strValue = __LoadSTRING(szCategory, szKey, strDefValue);
}
void CIniHelper::Read(LPCTSTR szCategory, LPCTSTR szKey, float *fValue, float fDefValue)
{
	*fValue = __LoadFLOAT(szCategory, szKey, fDefValue);
}
void CIniHelper::Read(LPCTSTR szCategory, LPCTSTR szKey, double *fValue, double fDefValue)
{
	*fValue = __LoadDOUBLE(szCategory, szKey, fDefValue);
}
void CIniHelper::Read(LPCTSTR szCategory, LPCTSTR szKey, CPoint *ptValue, CPoint ptDefValue)
{
	*ptValue = __LoadPOINT(szCategory, szKey, ptDefValue);
}
void CIniHelper::Read(LPCTSTR szCategory, LPCTSTR szKey, CRect *rtValue, CRect rtDefValue)
{
	*rtValue = __LoadRECT(szCategory, szKey, rtDefValue);
}

void CIniHelper::Write(LPCTSTR szCategory, LPCTSTR szKey, short *nValue)
{
	__SaveSHORT(szCategory, szKey, *nValue);
}
void CIniHelper::Write(LPCTSTR szCategory, LPCTSTR szKey, bool *nValue)
{
	__SaveBOOL(szCategory, szKey, *nValue);
}
void CIniHelper::Write(LPCTSTR szCategory, LPCTSTR szKey, int *nValue)
{
	__SaveINT(szCategory, szKey, *nValue);
}
void CIniHelper::Write(LPCTSTR szCategory, LPCTSTR szKey, ULONGLONG *nValue)
{
	__SaveULONGLONG(szCategory, szKey, *nValue);
}
void CIniHelper::Write(LPCTSTR szCategory, LPCTSTR szKey, long *nValue)
{
	__SaveINT(szCategory, szKey, (int)*nValue);
}
void CIniHelper::Write(LPCTSTR szCategory, LPCTSTR szKey, unsigned short *nValue)
{
	__SaveINT(szCategory, szKey, (int)*nValue);
}
void CIniHelper::Write(LPCTSTR szCategory, LPCTSTR szKey, CString *strValue)
{
	__SaveSTRING(szCategory, szKey, *strValue);
}
void CIniHelper::Write(LPCTSTR szCategory, LPCTSTR szKey, float *fValue)
{
	__SaveFLOAT(szCategory, szKey, *fValue);
}
void CIniHelper::Write(LPCTSTR szCategory, LPCTSTR szKey, double *fValue)
{
	__SaveDOUBLE(szCategory, szKey, *fValue);
}
void CIniHelper::Write(LPCTSTR szCategory, LPCTSTR szKey, CPoint *ptValue)
{
	__SavePOINT(szCategory, szKey, *ptValue);
}
void CIniHelper::Write(LPCTSTR szCategory, LPCTSTR szKey, CRect *rtValue)
{
	__SaveRECT(szCategory, szKey, *rtValue);
}


void CIniHelper::Delete(LPCTSTR szCategory)
{
//	CString strCategory(TrimSymbol(szCategory));
//
//	long	nFileLength = 0;
//	BYTE	*pBuffer	= NULL;
//
//	//------------------------------------------------------------------------------------
//	// 파일내용읽기
//	//------------------------------------------------------------------------------------
//	{
//		CFile file(m_strFilePath, CFile::modeRead);
//
//		nFileLength =  (long)file.GetLength();
//		pBuffer		= new BYTE[nFileLength + 1];
//		pBuffer[nFileLength] = 0;
//
//		file.Read(pBuffer, nFileLength);
//		file.Close();
//	}
//
//	//------------------------------------------------------------------------------------
//	// 줄별로 STRING 으로 만들기
//	//------------------------------------------------------------------------------------
//
//	CString strTotal;
//	strTotal = pBuffer;
//
//	std::vector<CString> vecLine;
//	long nStartLine = -1;
//
//	for(long n = 0; n <= strTotal.GetLength(); n++)
//	{
//		if((strTotal.GetAt(n) == 13 || n == strTotal.GetLength()) && nStartLine >= 0)
//		{
//			CString str = strTotal.Mid(nStartLine, n - nStartLine);
//			str = str.TrimLeft();
//			str = str.TrimRight();
//
//			vecLine.push_back(str);
//			nStartLine = -1;
//		}
//		else if(n != strTotal.GetLength() && nStartLine == -1 && strTotal.GetAt(n) != 13 && strTotal.GetAt(n) != 10)
//		{
//			nStartLine = n;
//		}
//	}
//
//	CString strNCategory = _T("[") + strCategory + _T("]");
//	long nDelete1 = -1;
//	long nDelete2 = -1;
//
//	for(long n = 0; n < (long)vecLine.size(); n++)
//	{
//		if(nDelete1 == -1 && vecLine[n] == strNCategory)
//		{
//			nDelete1 = n;
//			continue;
//		}
//
//		if(nDelete1 != -1 && vecLine[n].Left(1) == _T("["))
//		{
//			nDelete2 = n;
//			break;
//		}
//	}
//
//	if(nDelete1 != -1 && nDelete2 == -1)
//	{
//		vecLine.erase(vecLine.begin() + nDelete1, vecLine.end());
//	}
//
//	if(nDelete1 != -1 && nDelete2 != -1)
//	{
//		vecLine.erase(vecLine.begin() + nDelete1, vecLine.begin() + nDelete2);
//	}
//
//	CString strNewLine;
//	strNewLine.Format(_T("%c%c"), 13, 10);
//
//	for(long n = 0; n < (long)vecLine.size(); n++)
//	{
//		vecLine[n] += strNewLine;
//	}
//
//	//------------------------------------------------------------------------------------
//	// 파일내용쓰기
//	//------------------------------------------------------------------------------------
//	{
//		CFile file(m_strFilePath, CFile::modeCreate | CFile::modeWrite);
//
//		for(long n = 0; n < (long)vecLine.size(); n++)
//		{
//			BYTE *pBuffer	= NULL;
//			long nStrLength	= vecLine[n].GetLength();
//
//#ifdef _UNICODE
//			pBuffer = new BYTE[(nStrLength + 1) * 2];
//			memset(pBuffer, 0, (nStrLength + 1) * 2);
//			WideCharToMultiByte(CP_ACP,
//			                    0,
//			                    vecLine[n].GetString(),
//			                    -1,
//			                    (LPSTR)pBuffer,
//			                    (nStrLength + 1) * 2,
//			                    NULL,
//			                    NULL);
//
//			pBuffer[nStrLength] = 0;
//			file.Write(pBuffer, (UINT)strlen((char *)pBuffer));
//			delete []pBuffer;
//
//#else
//			pBuffer = (BYTE *)vecLine[n].GetString();
//			file.Write(pBuffer, nStrLength);
//#endif
//		}
//		file.Close();
//	}
//
//	delete []pBuffer;
}
