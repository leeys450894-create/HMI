//#include "pch.h"
#include "pch.h"
#include "LogWrapper.h"

#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/daily_file_sink.h"
#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//----------------------------------------------------------------------------------------------------
// Singleton
//----------------------------------------------------------------------------------------------------
CLogWrapper *CLogWrapper::m_pInstance = nullptr;
CLogWrapper *CLogWrapper::GetInstance()
{
	return m_pInstance ? m_pInstance : (m_pInstance = new CLogWrapper);
}
void CLogWrapper::DelInstance()
{
    if (m_pInstance)
    {
        delete m_pInstance;
        m_pInstance = nullptr;
    }
}
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------

CLogWrapper::CLogWrapper()
{
	m_hWnd = NULL;
}

CLogWrapper::~CLogWrapper()
{
}

void CLogWrapper::Initialize(CString strPath, CString strOPath, int dailyLogHour, int dailyLogMinute, CString strFile)
{
	CCriticalSectionSync single(&m_cs, TRUE);

	USES_CONVERSION;
    string pPath = T2A(strPath);
	string pFile = T2A(strFile);
	string pOPath = T2A(strOPath);

//	m_file = spdlog::daily_logger_mt("file", pPath, dailyLogHour, dailyLogMinute);
	m_file = spdlog::daily_logger_mt(pFile, pPath, dailyLogHour, dailyLogMinute);  // jsyoon 20210714 change
	m_fileO = spdlog::daily_logger_mt("OLOG", pOPath, dailyLogHour, dailyLogMinute);
}

void CLogWrapper::SetLogMsgWindow(HWND hWnd)
{
	CCriticalSectionSync single(&m_cs, TRUE);

	m_hWnd = hWnd;
}

void CLogWrapper::SetOLogMsgWindow(HWND hWnd)
{
	CCriticalSectionSync single(&m_cs, TRUE);

	m_hWndO = hWnd;
}

void CLogWrapper::KMLOG(enumLogLevel level, CString strValue)
{	
	CCriticalSectionSync single(&m_cs, TRUE);

	{
		CString sLog;
		//sLog.Format(_T("- %s - %s"), __ToString(level).GetString(), strValue.GetString()); 
		sLog.Format(_T("%s"), strValue.GetString());
	
		USES_CONVERSION;
		__OutputMsgHDD(level, T2A(sLog));
	}

	{
		SYSTEMTIME st;
		::GetLocalTime(&st);
	
		TCHAR result[256] = {0};
		_sntprintf_s(result, _countof(result), _countof(result) - 1, _T("%04d-%02d-%02d %02d:%02d:%02d:%03d"),
					 st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	
		CString sLog;
		sLog.Format(_T("[%s] - %s - %s"), result, __ToString(level).GetString(), strValue.GetString()); 

		__OutputMsgGUI(level, sLog);
	}
}

CString CLogWrapper::__ToString(enumLogLevel level)
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

enumLogLevel CLogWrapper::__FromString(const CString level)
{
	if(level == _T("ERRO"))	 return ERRO;
	if(level == _T("WARN"))	 return WARN;
	if(level == _T("SYST"))	 return SYST;
	if(level == _T("INSP"))	 return INSP;
	if(level == _T("OPER"))	 return OPER;
	if(level == _T("SEND"))	 return SEND;
	if(level == _T("RECV"))	 return RECV;
		
	return OPER;
}

void CLogWrapper::__OutputMsgHDD(enumLogLevel level, const char* pFormat, ...)
{
	const int bufSize = SHORT_MAX;
	char buf[bufSize];

	va_list vList;
	va_start(vList, pFormat);

	int size = vsnprintf(buf, bufSize, pFormat, vList);
	if (bufSize <= size)
		return;

	string logString(buf);

	switch (level)
	{
	case  ERRO:
		m_file->error(logString);
		break;
	case  WARN:
		m_file->warn(logString);
		break;
	case  SYST:
		m_file->debug(logString);
		break;
	case  INSP:
		m_file->debug(logString);
		break;
	case  OPER:
	//	m_file->info(logString);
		m_fileO->info(logString);
		break;
	case  SEND:
		m_file->debug(logString);
		break;
	case  RECV:
		m_file->debug(logString);
		break;
	default:
		m_file->info(logString);
		break;
	}

	if (level == OPER)
	{
		m_fileO->flush();
	}
	else
	{
		//	m_file->info(logString);
		m_file->flush();
	}

}

void CLogWrapper::__OutputMsgGUI(enumLogLevel level, const CString &msg)
{
	if (level == OPER)
	{
		if (m_hWndO)
		{
			PDWORD_PTR res = 0;
			::SendMessageTimeout(m_hWndO, WM_GUILOG, (WPARAM)(LPCTSTR)msg, MAKELPARAM(level, 0), SMTO_BLOCK | SMTO_NOTIMEOUTIFNOTHUNG, 1000, res);
		}
	}
	else
	{
		if (m_hWnd)
		{
			PDWORD_PTR res = 0;
			::SendMessageTimeout(m_hWnd, WM_GUILOG, (WPARAM)(LPCTSTR)msg, MAKELPARAM(level, 0), SMTO_BLOCK | SMTO_NOTIMEOUTIFNOTHUNG, 1000, res);
		}
	}
    
}
