#pragma once

#include <map>
#include <vector>

typedef void(*CallbackClassFunction1)(LPVOID);					// 연결 관련
typedef void(*CallbackClassFunction2)(LPVOID, LPVOID);			// UI  관련
typedef void(*CallbackClassFunction3)(LPVOID, LPVOID, LPVOID);	// UI  관련

inline BOOL MakeDir(CString strPath)
{
	if (strPath.GetLength() <= 0)
		return FALSE;

	if (strPath[1] != ':' || strPath[2] != '\\')
	{
		return FALSE;
	}

	int nFind = 2;

	while ((nFind = strPath.Find(_T("\\"), nFind + 1)) >= 0)
	{
		CString sTem = strPath.Left(nFind); 

		_tmkdir(sTem);
	//	CreateDirectory(sTem, NULL);
	}

	return CreateDirectory(strPath, NULL);
}

inline BOOL DeleteDir(CString strPath)
{
	if (strPath.GetLength() <= 0)
		return FALSE;

	CString szRoot = _T("");
	szRoot.Format(_T("%s\\*.*"), strPath.GetString());
	
	CFileFind finder;
	
	BOOL bRet = finder.FindFile(szRoot);
	
	if (bRet == FALSE)
	{
		finder.Close();
		return bRet;
	}

	while (bRet)
	{
		bRet = finder.FindNextFileW();

		if (finder.IsDots() == FALSE)
		{
			//디렉토리일 경우 재귀 호출
			if (finder.IsDirectory())
			{
				DeleteDir(finder.GetFilePath());
			}
			else                   
			{
				bRet = DeleteFile(finder.GetFilePath());
			}
		}
	}

	finder.Close();

	bRet = RemoveDirectory(strPath);	//자신의 폴더 및 하위 폴더 삭제
	bRet = MakeDir(strPath);			//자신의 폴더를 생성

	return bRet;
}

inline BOOL WriteDataFile(CString strFileName, std::vector<CString> &vecData)
{
	//------------------------------------------------------------------------------------
	// 파일내용쓰기 - 한글 포함한 경우에는 이렇게 쓰자.
	//------------------------------------------------------------------------------------
	CFileFind finder;
	BOOL bNew = FALSE;

	if (finder.FindFile(strFileName))
	{
		bNew = FALSE;
	}
	else
	{
		bNew = TRUE;
	}

	CFile file;
	CFileException e;

	UINT nOpenFlags = CFile::modeReadWrite | CFile::shareDenyWrite;
	if (bNew)
		nOpenFlags |= CFile::modeCreate;
	
	if(file.Open(strFileName, nOpenFlags/*CFile::modeCreate | CFile::modeWrite*/, &e) == FALSE)
	{
		//CString strErrorMessage;
		//strErrorMessage.Format(_T("fail : [%s] create, write, cause [%d]"), strFileName, e.m_cause);
		//AfxMessageBox(strErrorMessage);
		return FALSE;
	}

	file.SeekToEnd();

	for (size_t n = 0; n < vecData.size(); n++)
	{
		const long nStrLength = vecData[n].GetLength();

		TCHAR *pBuffer = new TCHAR[(nStrLength + 1) * 2];
		memset(pBuffer, 0, (nStrLength + 1) * 2);

		WideCharToMultiByte(CP_ACP,
			0,
			vecData[n].GetString(),
			-1,
			(LPSTR)pBuffer,
			(nStrLength + 1) * 2,
			NULL,
			NULL);
		
		pBuffer[nStrLength] = 0;

		file.Write(pBuffer, (UINT)strlen((char *)pBuffer));
		
		delete[]pBuffer;
		pBuffer = nullptr;
	}

	file.Flush();
	file.Close();
	return TRUE;
}

inline CString GetCurTimeStr()
{
	CString sTime;
	SYSTEMTIME st;
	::GetLocalTime(&st);
	sTime.Format(_T("%04d%02d%02d-%02d%02d%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	return sTime;
}

inline char* CStrToChar(CString& str)
{
	long len = str.GetLength();
	len = len * 2;
	char *szTemp = new char[len + 1];

	memset(szTemp, 0, len + 1);

	USES_CONVERSION;

//	strcpy(szTemp, T2A(str));
	// LYS 2026-09-16 sizeof(szTemp) 는 포인터 크기(8)라 7자 넘는 문자열에서 strcpy_s 가 버퍼 초과로 중단(Test Mode Save Image 경로 저장 시 발생). 실제 버퍼 길이(len+1) 사용
	//strcpy_s(szTemp, (size_t)sizeof(szTemp), T2A(str));
	strcpy_s(szTemp, (size_t)(len + 1), T2A(str));

	return szTemp;
}

inline CString CharToString(char* pchar)
{
	CString strTemp;
	strTemp.Empty();

#ifdef _UNICODE

	//변환할 문자열의 크기
	int nLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pchar, -1, NULL, NULL);
	//버퍼의 크기를 동적 할당
	WCHAR* pszWide = new WCHAR[nLen];
	//형변환
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