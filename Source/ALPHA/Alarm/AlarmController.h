#pragma once

//#include "AlarmIDList.h"  // jsyoon 2022.01,.19 change
#include "..\..\Include\AlarmDefine.h"
#include <afxmt.h>

class CAlarmController
{
	CCriticalSection m_cs_set;
	CCriticalSection m_cs_get;

public:
    BOOL InitializeAlarm(HWND hParentWnd, CRect rtROI, CString strFolderPath);

	UINT SetAlarm(UINT id);
    UINT GetAlarmCount();
    BOOL GetAlarm(UINT id, CString &str);
    void GetAlarmText(UINT id, CString& str);
    BOOL ClearAlarm();
	BOOL ReLoadAlarmList(CString strFolderPath);

public:
    CWnd *GetAlarmWnd();
    CWnd *GetReportAlarmWnd();



    //----------------------------------------------------------------------------------------------------
    // Singleton
    //----------------------------------------------------------------------------------------------------
private:
    static CAlarmController	*m_pInstance;
public:
    static CAlarmController	*GetInstance();
    static void				DelInstance();
private:
    CAlarmController(void);
    ~CAlarmController(void);
};
