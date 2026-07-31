//#include "pch.h"
#include "pch.h"
#include "LogFileAccessor.h"

#include "file_access\FileAccessor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CLogFileAccessor::CLogFileAccessor()
{
	m_strFilePath = _T("c:\\ICD_20190000.log");
	m_strDataTime = _T("00000000");
	m_pFAccessor = new CFileAccessor;
	m_hWnd = NULL;
}

CLogFileAccessor::~CLogFileAccessor()
{
	if(m_pFAccessor)
	{
		delete m_pFAccessor;
		m_pFAccessor = nullptr;
	}
}

void CLogFileAccessor::Initialize(CString strPath, int dailyLogHour, int dailyLogMinute)
{
	if(m_pFAccessor)
	{
		m_pFAccessor->CloseStream();
	}

	strPath.Replace(_T("\\\\"), _T("\\"));	// 2023.03.07 add

	SYSTEMTIME st;
	::GetLocalTime(&st);		
	CString strDate;
	strDate.Format(_T("%04d%02d%02d"), st.wYear, st.wMonth, st.wDay);

	CString strTemp1 = _T("_") + strDate + _T(".log");
	CString strTemp2 = strPath;
	strTemp2.Replace(_T(".log"), strTemp1);

	if(m_pFAccessor)
	{
		if(m_pFAccessor->Open_Stream(strTemp2) == true)
		{
			m_strDataTime = strDate;
			m_strFilePath = strPath;
		}
	}
}

void CLogFileAccessor::SetLogMsgWindow(HWND hWnd)
{
	m_hWnd = hWnd;
}

void CLogFileAccessor::KMLOG(enumLogLevel level, CString strValue, bool bVisible)
{
	CCriticalSectionSync single(&m_cs, TRUE);

	SYSTEMTIME st;
	::GetLocalTime(&st);		

	CString strDataTime;
	strDataTime.Format(_T("%04d%02d%02d"), st.wYear, st.wMonth, st.wDay);
	if(m_strDataTime != strDataTime)
	{
		Initialize(m_strFilePath);
	}

	CString sLog;
	sLog.Format(_T("[%04d-%02d-%02d %02d:%02d:%02d:%03d] - %s - %s"), 
				st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
				__Lv_To_string(level).GetString(), strValue.GetString()); 
		
	__OutputMsgHDD(level, sLog);
	
	if(bVisible == true)
	{
		__OutputMsgGUI(level, sLog);
	}
}

CString CLogFileAccessor::GetFilePath()
{
	return m_strFilePath;
}





CString CLogFileAccessor::__Lv_To_string(enumLogLevel level)
{
    static LPCTSTR const buffer[] =
    {
		_T("ERRO"),
		_T("WARN"), 
		_T("SYST"),		
		_T("INSP"),
		_T("OPER"),
		_T("SEND"),
		_T("RECV"),
    };

    return buffer[level];
}

void CLogFileAccessor::__OutputMsgHDD(enumLogLevel level, const CString &msg)
{
	if(m_pFAccessor)
	{
		m_pFAccessor->WriteStream(msg);
	}
}

void CLogFileAccessor::__OutputMsgGUI(enumLogLevel level, const CString &msg)
{
    if(m_hWnd)
	{
		ULONGLONG dwStart = GetTickCount();

		LRESULT dwResult = 0;
		PDWORD_PTR res = 0;
		dwResult = ::SendMessageTimeout(m_hWnd, WM_GUILOG, (WPARAM)(LPCTSTR)msg, MAKELPARAM(level, 0), SMTO_BLOCK | SMTO_NOTIMEOUTIFNOTHUNG, 5000, res);
//		dwResult = ::SendMessageTimeout(m_hWnd, WM_GUILOG, (WPARAM)(LPCTSTR)msg, MAKELPARAM(level, 0), SMTO_NORMAL, 500, res);
		 
		TRACE("__OutputMsgGUI, %d, %d\n", GetTickCount() - dwStart, dwResult); 
	}
}
