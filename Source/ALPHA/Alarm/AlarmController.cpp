#include "pch.h"
#include "AlarmController.h"

#include "..\..\Include\API_Alarm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//----------------------------------------------------------------------------------------------------
// Singleton
//----------------------------------------------------------------------------------------------------
CAlarmController *CAlarmController::m_pInstance = nullptr;
CAlarmController *CAlarmController::GetInstance()
{
    return m_pInstance ? m_pInstance : (m_pInstance = new CAlarmController);
}
void CAlarmController::DelInstance()
{	
    if(m_pInstance)
    {
        delete m_pInstance;
        m_pInstance = nullptr;
    }
}

CAlarmController::CAlarmController(void)
{
    
}

CAlarmController::~CAlarmController(void)
{
	::Alarm_FinalizeAlarm();
	::Alarm_FinalizeReport();
}

BOOL CAlarmController::InitializeAlarm(HWND hParentWnd, CRect rtROI, CString strFolderPath)
{
	::Alarm_InitializeAlarm(hParentWnd, rtROI);
	::Alarm_SetAlarmView(hParentWnd);

	CString strPath;
	strPath.Format(_T("%s"), strFolderPath);
	::Alarm_SetSavePath((TCHAR*)(LPCTSTR)strPath);

	::Alarm_GetAlarmWnd()->ShowWindow(SW_HIDE);
	::Alarm_GetReportAlarmWnd()->ShowWindow(SW_SHOW);

	return FALSE;
}

UINT CAlarmController::SetAlarm(UINT id)
{
	// 알람을 1개씩 순차적으로 발생하기 위해서 사용한다.
	return Alarm_SetAlarm(id);
}

UINT CAlarmController::GetAlarmCount()
{
	return Alarm_GetListCount();
}

BOOL CAlarmController::GetAlarm(UINT id, CString &str)
{
	// 알람을 1개씩 순차적으로 추출하기 위해서 사용한다.
    CCriticalSectionSync single(&m_cs_get, TRUE);

	TCHAR szBuff[1024];
	memset(szBuff, 0, 1024);
	::Alarm_GetMessage(id, szBuff);
	str.Format(_T("%s"), szBuff);

	return FALSE;
}

void CAlarmController::GetAlarmText(UINT id, CString& str)
{
	// 알람을 1개씩 순차적으로 추출하기 위해서 사용한다.
	CCriticalSectionSync single(&m_cs_get, TRUE);

	TCHAR szBuff[1024];
	memset(szBuff, 0, 1024);
	::Alarm_GetText(id, szBuff);
	str.Format(_T("%s"), szBuff);
}

BOOL CAlarmController::ClearAlarm()
{
	Alarm_ClearAlarm();

	return FALSE;
}

CWnd *CAlarmController::GetAlarmWnd()
{
	return Alarm_GetAlarmWnd();
}

CWnd *CAlarmController::GetReportAlarmWnd()
{
	return Alarm_GetReportAlarmWnd();
}

BOOL CAlarmController::ReLoadAlarmList(CString strFolderPath)
{
	return Alarm_ExcelFile();
	//if (m_pAlarmFunction)
	//{
	//	return m_pAlarmFunction->ReLoadAlarmList(strFolderPath);
	//}

	//return FALSE;
}