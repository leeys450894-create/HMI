// MyLabelTimer.cpp: implementation of the CMyLabelTimer class.
//
//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "MyLabelTimer.h"
#include "Registry.h"
//#include "..\\include\\CommonFunc.h"
// #include "..\\Include\\API_Alarm.h"				// pwk 20130712

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define RGB_WHITE			RGB(255, 255, 255)
#define RGB_BLACK			RGB(0, 0, 0)
#define	RGB_LIGHT_GRAY		RGB(192, 192, 192)
#define RGB_LIGHT_BLUE		RGB(190, 210, 240)	// grid header
#define	RGB_LIGHT_GRAY		RGB(192, 192, 192)
#define RGB_DARK_BLUE3		RGB(60, 101, 193)
#define RGB_LIGHT_YELLOW	RGB(0xFF, 0xFF, 0xB0)
#define RGB_LIGHT_RED		RGB(0xFF, 0x40, 0x40)
#define RGB_ORANGE			RGB(255, 187, 0)
#define REG_SETTINGS		_T("Settings")
#define REG_LAMP_USED		_T("LampTime_Used")
#define	REG_LAMP_LIMIT		_T("LampTime_Limit")
#define	REG_LAMP_WARRING	_T("LampTime_Warring")
#define INT_LAMP_LIMIT_MAX			30000

// sg.kim 20110414
CMyLabelTimer::CMyLabelTimer()
{
	m_iType = LT_LAMP_TIMER;
	m_iLamp = 0;
	m_pRegistry		= NULL;
	
	m_spanStart = CTimeSpan(0);
	m_spanLimit = CTimeSpan(0);
	m_spanWarring = CTimeSpan(0);
	m_timeStart = CTime::GetCurrentTime();

	m_pTimerThread	= NULL;
	m_hEventStop	= CreateEvent(NULL, TRUE, FALSE, NULL);	// manual reset, Initially nonsignaled
	
	m_iDelayTime = 1000;
	m_bStop		 = TRUE;
	m_bTimeOver  = FALSE;
	m_bTimeWarring = FALSE;
}

CMyLabelTimer::~CMyLabelTimer()
{
	::CloseHandle(m_hEventStop);

	if( m_pRegistry != NULL)
	{
		delete m_pRegistry;
		m_pRegistry = NULL;
	}

//	SAFE_DELETE(m_pRegistry);
}

void CMyLabelTimer::InitTimer(CString strName, CString strAppName, int iType, int iLamp) 
{
	m_iType = iType;
	m_iLamp = iLamp;
	
	SetFontSize(9);
//	SetFontName("Times New Roman");
	SetFontName(_T("Tahoma"));
	SetFontBold(FALSE);
	SetBkColor(RGB_LIGHT_BLUE);
	SetTextColor(RGB_LIGHT_GRAY);
	SetFont3D(TRUE, CLabel::Raised);
	SetText3DHiliteColor(RGB_DARK_BLUE3);
	SetInvalidate(FALSE);
	
	if(m_iType == LT_LAMP_TIMER)
	{
		m_pRegistry		= new CRegistry(strName/*STR_COMPANY_NAME*/, strAppName/*STR_APP_NAME*/);

		ReloadRegistry(iLamp);
		m_bRegWrite	= TRUE;

		DefaultTimer();
//		RefreshTimer();	// jsyoon 20160314 add
	}
}

void CMyLabelTimer::StopTimer()
{
	DWORD dwResult;
	
	if(m_pTimerThread)
	{
		m_pTimerThread->SuspendThread();
		
		m_bStop = TRUE;
		SetEvent(m_hEventStop);
		
		m_pTimerThread->ResumeThread();
		
		dwResult = WaitForSingleObject(m_pTimerThread->m_hThread, 2000);
		if(dwResult == WAIT_TIMEOUT)
		{
			TRACE("Stop TimeOut\n");
			TerminateThread(m_pTimerThread->m_hThread, 0);
		}

		m_pTimerThread = NULL;

		SetBkColor(RGB_LIGHT_BLUE);		// jsyoon 20180419 add
		
//		::CloseHandle(m_hEventStop);
	}

	TRACE("Stop Timer@@@@@@@@@@@\n");
}

void CMyLabelTimer::StartTimer()
{
	if( !m_bStop )
		return;

	if(m_pTimerThread == NULL )
	{
		m_pTimerThread = AfxBeginThread(TimerCB, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);	

		m_bStop = FALSE;
		ResetEvent(m_hEventStop);
	
		m_pTimerThread->ResumeThread();

		SetBkColor(RGB_LIGHT_YELLOW);		// jsyoon 20180419 add
	}
}

UINT CMyLabelTimer::TimerCB(LPVOID pParam) 
{
	CMyLabelTimer* me = (CMyLabelTimer*)pParam;
	
	me->TimerCBBody();
	
	return 0;
}

void CMyLabelTimer::TimerCBBody()
{
	while(!m_bStop)
	{
		DWORD dwResult = WaitForSingleObject(m_hEventStop, m_iDelayTime);
		switch(dwResult)
		{
		case WAIT_OBJECT_0 :
			break;
			
		case WAIT_TIMEOUT :
			RefreshTimer();
			break;
		}
	}
}

void CMyLabelTimer::DefaultTimer()
{
	CString str;
	str.Empty();
	int iState = 0;

	if(m_iType == LT_LAMP_TIMER)
	{
//		CTimeSpan spanCur = CTime::GetCurrentTime() - m_timeStart;
		m_spanCur = m_spanStart;
		if (!m_strLabel.IsEmpty())	// LYS 2026-09-25 caller-given label (ALIGN CCD1/2) instead of CH<n>
		{
			str.Format(_T("%s:%05d:%02d:%02d"), m_strLabel, (LONG)m_spanCur.GetTotalHours(), m_spanCur.GetMinutes(), m_spanCur.GetSeconds());
		}
		else if( m_iLamp == 14 )
		{
			str.Format(_T("SE A:%05d:%02d:%02d"), (LONG)m_spanCur.GetTotalHours(), m_spanCur.GetMinutes(), m_spanCur.GetSeconds());	
		}
		else if( m_iLamp == 15 )
		{
			str.Format(_T("SE B:%05d:%02d:%02d"), (LONG)m_spanCur.GetTotalHours(), m_spanCur.GetMinutes(), m_spanCur.GetSeconds());	
		}
		else
		{
			str.Format(_T("CH%d:%05d:%02d:%02d"), m_iLamp+1, (LONG)m_spanCur.GetTotalHours(), m_spanCur.GetMinutes(), m_spanCur.GetSeconds());	
		}
		

		if(m_spanCur > m_spanLimit)	// over
		{
			SetTextColor(RGB_BLACK);
			SetText3DHiliteColor(RGB_WHITE);
			SetBkColor(RGB_LIGHT_RED);

			iState = 2;
			m_bTimeOver = TRUE;
		}
		else if (m_spanCur > m_spanWarring)	// warring
		{
			SetTextColor(RGB_BLACK);
			SetText3DHiliteColor(RGB_WHITE);
			SetBkColor(RGB_ORANGE);

			iState = 2;
			m_bTimeWarring = TRUE;
		}
		else	// normal
		{
			iState = 1;
			m_bTimeOver = FALSE;
			m_bTimeWarring = FALSE;
		}	
	}
	else		// LT_CLOCK
	{		
		SYSTEMTIME sysTime;
		::GetLocalTime(&sysTime);
		str.Format(_T("%02d/%02d/%04d\n%02d:%02d:%02d"),
			sysTime.wMonth, sysTime.wDay, sysTime.wYear,
			sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
	}

 	SetText(str);
}

void CMyLabelTimer::RefreshTimer()
{
	CString str;
	str.Empty();
	int iState = 0;
	
	if(m_iType == LT_LAMP_TIMER)
	{
		CTimeSpan spanCur = CTime::GetCurrentTime() - m_timeStart;
		m_spanCur = m_spanStart + spanCur;
		if (!m_strLabel.IsEmpty())	// LYS 2026-09-25 caller-given label (ALIGN CCD1/2) instead of CH<n>
		{
			m_strTimeText.Format(_T("%s:%05d:%02d:%02d"), m_strLabel, (LONG)m_spanCur.GetTotalHours(), m_spanCur.GetMinutes(), m_spanCur.GetSeconds());
		}
		else if( m_iLamp == 14 )
		{
			m_strTimeText.Format(_T("SE A:%05d:%02d:%02d"),
				(LONG)m_spanCur.GetTotalHours(), m_spanCur.GetMinutes(), m_spanCur.GetSeconds());	
		}
		else if( m_iLamp == 15 )
		{
			m_strTimeText.Format(_T("SE B:%05d:%02d:%02d"),
				(LONG)m_spanCur.GetTotalHours(), m_spanCur.GetMinutes(), m_spanCur.GetSeconds());	
		}
		else
		{
			m_strTimeText.Format(_T("CH%d:%05d:%02d:%02d"), m_iLamp+1,
				(LONG)m_spanCur.GetTotalHours(), m_spanCur.GetMinutes(), m_spanCur.GetSeconds());	
		}

	

		if(m_bRegWrite)
		{
			WriteRegistry(m_iLamp);
		}

//		TRACE("%s, %d \n", str, m_spanCur);

		if(m_spanCur > m_spanLimit)	// over
		{
			if( m_bTimeOver == FALSE)
			{
				SetTextColor(RGB_BLACK);
				SetText3DHiliteColor(RGB_WHITE);
				SetBkColor(RGB_LIGHT_RED);
				iState = 2;
				m_bTimeOver = TRUE;
			}
		}	
		else if (m_spanCur > m_spanWarring)	// over
		{
			if (m_bTimeWarring == FALSE)
			{
				SetTextColor(RGB_BLACK);
				SetText3DHiliteColor(RGB_WHITE);
				SetBkColor(RGB_ORANGE);
				iState = 2;
				m_bTimeWarring = TRUE;
			}
		}
		else	// normal
		{
			if( m_bTimeOver == TRUE )
			{
				SetTextColor(RGB_LIGHT_GRAY);
				SetText3DHiliteColor(RGB_DARK_BLUE3);	//blue
				SetBkColor(RGB_LIGHT_YELLOW);
				iState = 1;
				m_bTimeOver = FALSE;
			}
		}	
	}
	else		// LT_CLOCK
	{		
		SYSTEMTIME sysTime;
		::GetLocalTime(&sysTime);
		m_strTimeText.Format(_T("%02d/%02d/%04d\n%02d:%02d:%02d"),
			sysTime.wMonth, sysTime.wDay, sysTime.wYear,
			sysTime.wHour, sysTime.wMinute, sysTime.wSecond);
	}

 	// jsyoon 20150126 add
 //	SetTimeText(str);
//	m_strTimeText.Format(_T("%s"), str);
 	::PostMessage(this->m_hWnd,WM_UPDATETEXT,(WPARAM)iState,(LPARAM)0);
// 	SetText(str);
}

void CMyLabelTimer::ReloadRegistry(int iLamp)
{
	if(m_iType == LT_CLOCK || !m_pRegistry) return;
	
	DWORD dwUsed, dwLimit, dwWarring;

	CString strUse, strLimit, strWarring;
	strUse.Format(_T("%s%02d"), REG_LAMP_USED, iLamp);
	strLimit.Format(_T("%s%02d"), REG_LAMP_LIMIT, iLamp);
	strWarring.Format(_T("%s%02d"), REG_LAMP_WARRING, iLamp);

	m_pRegistry->ReadDWord(REG_SETTINGS, strUse, dwUsed, 0);
	m_pRegistry->ReadDWord(REG_SETTINGS, strLimit, dwLimit, INT_LAMP_LIMIT_MAX);
	m_pRegistry->ReadDWord(REG_SETTINGS, strWarring, dwWarring, INT_LAMP_LIMIT_MAX);
	
	m_timeStart = CTime::GetCurrentTime();
	m_spanStart = dwUsed;
	m_spanLimit = CTimeSpan(0, dwLimit, 0, 0);
	m_spanWarring = CTimeSpan(0, dwWarring, 0, 0);
}

void CMyLabelTimer::WriteRegistry(int iLamp)
{
	CString strUse;
	LONGLONG Time = 0;
	Time = m_spanCur.GetTotalSeconds();
	if( Time >= 0)
	{
		strUse.Format(_T("%s%02d"), REG_LAMP_USED, iLamp);
		m_pRegistry->WriteDWord(REG_SETTINGS, strUse, (DWORD)Time);
	}
}

int CMyLabelTimer::GetCurTime()
{
	int nTime = 0;
	CString strTimeText;
	strTimeText.Format(_T("%08d"), (LONG)m_spanCur.GetTotalHours());
	nTime = _ttoi(strTimeText);
	return nTime;
}