#if !defined(AFX_ALARMMANAGER_H__F8C69B52_B1E1_4FDA_A010_9AE931B9EF41__INCLUDED_)
#define AFX_ALARMMANAGER_H__F8C69B52_B1E1_4FDA_A010_9AE931B9EF41__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#include "AlarmDefine.h"


#include <afxtempl.h>
#endif // _MSC_VER > 1000
// AlarmManager.h : header file
//



/////////////////////////////////////////////////////////////////////////////
// CAlarmManager window
class CAlarmXLSFile;
class CAlarmCSVFile;
class CDlgAlarm;
class CDlgWarning;
class CDlgReportAlarm;
class CAlarmManager : public CWnd
{
// Construction
public:
	CAlarmManager();

	CCriticalSection m_cs_set;
	CCriticalSection m_cs_get;

// Attributes
public:
	CAlarmXLSFile		*m_pAlarmXLSFile;
	CAlarmCSVFile		*m_pAlarmCSVFile;
	CDlgAlarm			*m_pDlgAlarm;
	CDlgWarning			*m_pDlgWarning;
	CDlgReportAlarm		*m_pDlgReportAlarm;
	BOOL				m_bInitialized;		// pwk 20130602
// Operations
public:
	// Alarm Init/Final
	void InitializeAlarm(HWND hParent, CRect r);
	void FinalizeAlarm();
	void ClearAlarm();		// jsyoon 20171204 add 

	// Alarm Report Init/Final
	void InitializeReport(HWND hParent, CRect r);
	void FinalizeReport();

	// Alarm dlg
	void CreateAlarmDlg(HWND hParent, CRect r);
	void CreateWarningDlg(HWND hParent);
	void CreateAlarmReportDlg(HWND hParent, CRect r);

	// jsyoon 20170731 add
	void SetAlarmView(HWND hWnd);
	void SetHostMode(int iMode);
	void SetHostSavePath(TCHAR* pPath);
	void GetMeassgeList(int i, TCHAR* pchar);
	void GetAlarmText(int i, TCHAR* pchar);

	CWnd* GetReportAlarmWnd();
	CWnd* GetAlarmWnd();

	BOOL ReloadAlarmList();
	BOOL LoadAlarmFile(CString strFileName = _T("alarm.csv"));
	BOOL FindAlarmTable(UINT id, ALARM_EVENT_DATA& alarm);

	ALARM_EVENT_DATA m_stalarm;
	int SetAlarm(UINT id);
	int SetAlarmSub(UINT id, CCriticalSection *pCS);
	int SetHeavyAlarm(ALARM_EVENT_DATA &alarm, CCriticalSection *pCS);
	int SetLightAlarm(ALARM_EVENT_DATA &alarm, CCriticalSection *pCS, BOOL bLow = TRUE);
	int GetListCount();

	int AlarmUpdateLanguage(HINSTANCE hResource_Lang_Dll); //lhj 20181225 Report Alarm 

private:
	CMap<unsigned int, unsigned int, ALARM_DATA, ALARM_DATA&> m_alarmTable;
	CMap<UINT, UINT, ALARM_MESSAGE, ALARM_MESSAGE&> m_alarmMessage;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAlarmManager)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CAlarmManager();

	// Generated message map functions
protected:
	//{{AFX_MSG(CAlarmManager)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	HWND m_hAlarmWnd;
public:
	bool m_bFinalizeAlarm;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ALARMMANAGER_H__F8C69B52_B1E1_4FDA_A010_9AE931B9EF41__INCLUDED_)
