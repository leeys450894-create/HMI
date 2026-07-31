#pragma once

#include "..\LogDefine.h"

class CFileAccessor;
class CLogFileAccessor
{
	CCriticalSection	m_cs;
	CString				m_strFilePath;
	CString				m_strDataTime;
	CFileAccessor		*m_pFAccessor;
	HWND				m_hWnd;

public:
	void Initialize(CString strPath, int dailyLogHour = 0, int dailyLogMinute = 0);
    void SetLogMsgWindow(HWND hWnd);
	void KMLOG(enumLogLevel level, CString strValue, bool bVisible = true);
	CString GetFilePath();
	
private:
	CString	__Lv_To_string(enumLogLevel level);
	void	__OutputMsgHDD(enumLogLevel level, const CString &msg);
	void	__OutputMsgGUI(enumLogLevel level, const CString &msg);

public:
	CLogFileAccessor();
	~CLogFileAccessor();
};
