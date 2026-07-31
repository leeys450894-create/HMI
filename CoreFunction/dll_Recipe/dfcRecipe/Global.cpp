#include "pch.h"
#include "Global.h"

CString	dfcStr (LPCTSTR fmt, ...)
{
	CString	string;
	if (fmt != 0) {
		va_list	argptr;
		va_start (argptr, fmt);
		string.FormatV (fmt, argptr);
		va_end (argptr);
	}
	return string;
}

CString	dfcStr (int nVal)
{
	CString	string;
	string.Format (_T("%d"), nVal);
	return string;
}

CString	dfcStr (double fVal)
{
	CString	string;
	string.Format (_T("%.4lf"), fVal);
	return string;
}

CString	dfcGetFontName ()
{
	CString	string = dfcStr (IDS_FONTNAME);

	if (string.IsEmpty ()) return _T("맑은 고딕");
	else				   return string;
}

CRect	dfcClientRect (CWnd *pWnd, CWnd *pWndMap)
{
	CRect	rcClient;
	pWnd->GetClientRect (&rcClient);

	if (pWndMap) {
		pWnd->MapWindowPoints (pWndMap, &rcClient);
	}

	return rcClient;
}

CString	dfcIniFolder ()
{
	CString strFolder;

	TCHAR szBuff[_MAX_PATH];
	::GetModuleFileName(::AfxGetApp()->m_hInstance, szBuff, _MAX_PATH);		

	strFolder.Format (_T("%s"), szBuff);
	int nPos = strFolder.ReverseFind ('\\');

	if (nPos != -1) {
		strFolder = strFolder.Mid (0, nPos) + _T("\\Config\\");
	}

	return strFolder;
}

CString	dfcIniFile ()
{
	return dfcIniFolder () + _T("Recipe.ini");
}

BOOL	dfcCreateDirectory (LPCTSTR szPath)
{
	int nLen = (int)_tcslen (szPath);

	//<<- 길이가 없으면 반환 
	if (nLen == 0)
		return FALSE;

	//<<- 임시 버퍼 생성 
	TCHAR* pBuf = new TCHAR[nLen+1];
	_tcscpy (pBuf, szPath); // 복사 

	//<<- 확장자 제거 
	PathRemoveExtension (pBuf);

	//<<- Path 설정 
	CString strPath = pBuf;

	//<<- 메모리 제거 
	delete [] pBuf;

	//<<- 파일인지 디렉터리인지 여부 확인 (길이가 달라지면 파일이라고 가정)
	if (_tcslen (szPath) != strPath.GetLength()) {
		int n = strPath.ReverseFind (_T('\\'));
		strPath = strPath.Left (n);	//<<- 폴더 경로만 추출
	}

	//<<- 폴더 경로가 있는지여부 확인 
	if (_taccess (strPath,0) != -1) 
		return TRUE;

	strPath += _T('\\');

	//<<- 루프를 돌면서 폴더 생성 
	int nIdx; 
	int nStart = 0;
	while ((nIdx = strPath.Find (_T('\\'),nStart)) >= 0) {
		CString str = strPath.Left (nIdx);
		if (_taccess (str,0) == -1) {
			if  (!CreateDirectory (str,NULL))
				return FALSE;
		}
		nStart = nIdx+1;
	}

	return TRUE;
}

void	dfcArryParsing (CStringArray *strArry, CString str)
{
	int nBegin = 0, nEnd = 0;
	strArry->RemoveAll ();
	for (int i =0; i <= str.GetLength (); i++) {
		if (str.GetAt (i) == ',') {
			nEnd = i - nBegin;
			strArry->Add (str.Mid (nBegin, nEnd).Trim ());
			nBegin = i + 1;
		}
	}
}

void	WriteString (CStdioFile *File, CString string)
{
	string += _T("\n");
	File->WriteString (string);
}

void	WriteInt (CStdioFile *File, int nVal)
{
	CString string;
	string = dfcStr (_T("%d\n"), nVal);
	File->WriteString (string);
	
}

void	WriteDouble (CStdioFile *File, double fVal)
{
	CString string;
	string = dfcStr (_T("%.4lf\n"), fVal);
	File->WriteString (string);

}

CString	ReadString (CStdioFile *File)
{
	CString string;
	File->ReadString (string);
	return string;
}
int		ReadInt (CStdioFile *File)
{
	CString string;
	File->ReadString (string);
	return _ttoi (string);
}

double	ReadDouble (CStdioFile *File)
{
	CString string;
	File->ReadString (string);
	return _wtof (string);
}