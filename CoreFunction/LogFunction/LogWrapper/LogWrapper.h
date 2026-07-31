#pragma once

#include "..\LogDefine.h"
#include "spdlog/spdlog.h"

class CLogWrapper
{
	CCriticalSection			m_cs;
private:
	shared_ptr<spdlog::logger>	m_file;
	shared_ptr<spdlog::logger>	m_fileO;
	HWND						m_hWnd;	
	HWND						m_hWndO;

public:
	void Initialize(CString strPath, CString strOPath, int dailyLogHour = 0, int dailyLogMinute = 0, CString strFile = _T("file") );
    void SetLogMsgWindow(HWND hWnd);
	void SetOLogMsgWindow(HWND hWnd);
	void KMLOG(enumLogLevel level, CString strValue);

private:
	CString			__ToString(enumLogLevel level);
	enumLogLevel	__FromString(const CString level);
	void			__OutputMsgHDD(enumLogLevel level, const char* pFormat, ...);
	void			__OutputMsgGUI(enumLogLevel level, const CString &msg);



	//----------------------------------------------------------------------------------------------------
	// Singleton
	//----------------------------------------------------------------------------------------------------
private:
	static CLogWrapper	*m_pInstance;
public:
	static CLogWrapper	*GetInstance();
	static void			DelInstance();
	//----------------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------------
private:
	CLogWrapper(void);
	~CLogWrapper(void);
};
