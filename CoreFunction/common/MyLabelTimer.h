// MyLabelTimer.h: interface for the CMyLabelTimer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYLABELTIMER_H__652DD68C_9CE1_4121_89C1_FB71AC1261CA__INCLUDED_)
#define AFX_MYLABELTIMER_H__652DD68C_9CE1_4121_89C1_FB71AC1261CA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Label.h"

// blue 2009.11.03 ========================================
// 
// CMyLabelTimer : thread 돌면서 time display (1초마다)
// 
// - LAMP_TIMER : 돌때 매번 레지스트리에 현재 사용시간 기록
//				  레지스트리 셋팅은 CSetLampTimeDlg() 에서..
// - CLOCK : 걍 디스플레이용 타이머.
//=========================================================

typedef enum _label_timer_type
{
	LT_LAMP_TIMER = 0,
	LT_CLOCK,
	LT_TYPE_END,
};

class CRegistry;
class CMyLabelTimer : public CLabel  
{
public:
	CMyLabelTimer();
	virtual ~CMyLabelTimer();

	// common..
	void	InitTimer(CString strName, CString strAppName, int iType, int iLamp = 0);
	void	SetDisplayTime_MS(int iDelayTime_ms) { m_iDelayTime = iDelayTime_ms; }

	// for LampTimer
	void	WriteRegistry(int iLamp);
	void	ReloadRegistry(int iLamp);
	void	SetRegistryWrite(BOOL bWrite) { m_bRegWrite = bWrite; }
	BOOL	GetTimeOver() { return m_bTimeOver; }
	BOOL	GetTimeWarring() { return m_bTimeWarring; }
	int		GetCurTime();

protected:
	int			m_iLamp;
	int			m_iType;
	int			m_iDelayTime;
	BOOL		m_bTimeOver;
	BOOL		m_bTimeWarring;

	// for lampTimer
public :
	void DefaultTimer();
	void RefreshTimer();
	void TimerCBBody();
	void StartTimer();
	void StopTimer();

	CRegistry*	m_pRegistry;
	CTime		m_timeStart;

	BOOL		m_bRegWrite;
	CTimeSpan	m_spanStart, m_spanLimit, m_spanCur, m_spanWarring;

	// display thread (common)
	// sg.kim 20110414 Lamp Timer 추가 위해 쓰레드 한 개 더 추가
	CWinThread* m_pTimerThread;
	
	HANDLE		m_hEventStop;
	BOOL		m_bStop;
	static UINT TimerCB(LPVOID pParam);
};

#endif // !defined(AFX_MYLABELTIMER_H__652DD68C_9CE1_4121_89C1_FB71AC1261CA__INCLUDED_)
