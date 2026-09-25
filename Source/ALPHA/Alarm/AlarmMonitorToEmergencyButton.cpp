#include "pch.h"
#include "AlarmMonitorToEmergencyButton.h"

#include "..\Manager\SystemConfigMgr.h"
#include "..\Manager\IOMapControlMgr.h"
#include "..\Manager\IOMapAddressDefine.h"
//#include "CommTcpIPMgr.h"
#include "AlarmController.h"
#include "..\AuroraFuncDefine.h"
#include "LogFunction\LogAccessor\LogFileAccessor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CAlarmMonitorToEmergencyButton::CAlarmMonitorToEmergencyButton(CString strLogPath, int iMonitorIONumber, int iEMOPosST, int iEMOPosED, CString strLogMsg, int iAlarmID)
    : m_iMonitorIONumber(iMonitorIONumber)
    , m_iEMOPosST(iEMOPosST)
    , m_iEMOPosED(iEMOPosED)
    , m_strLogMsg(strLogMsg)
    , m_iAlarmID(iAlarmID)
	, m_pLogFileAccessor(new CLogFileAccessor)
	, m_nNGCount(0)
	, m_bAlarmOn(false)
{
    const CString strFolderPath	= strLogPath + _T("\\Event\\");
    CString strLogFile;
	strLogFile.Format(_T("%s%s_%02d.log"), strFolderPath, _T("emergency_button"), iMonitorIONumber);
    MakeDir(strFolderPath);
    m_pLogFileAccessor->Initialize(strLogFile);

	//----------------------------------------------------------------------
    // Thread 생성
    //----------------------------------------------------------------------
    m_pThread		= AfxBeginThread(ThreadRun, this);
    m_bThreadLive	= false;
    m_hStartEvent	= CreateEvent(NULL, TRUE, FALSE, NULL);		// manual set, initial reset
    m_hCloseEvent	= CreateEvent(NULL, FALSE, FALSE, NULL);	// auto reset, initial reset
}

CAlarmMonitorToEmergencyButton::~CAlarmMonitorToEmergencyButton()
{
    //----------------------------------------------------------------------
    // Thread 종료
    //----------------------------------------------------------------------
    ResetEvent(m_hStartEvent);
    SetEvent(m_hCloseEvent);
    if(m_pThread && m_bThreadLive)
    {
        WaitForSingleObject(m_pThread->m_hThread, INFINITE);
    }
    if(m_hStartEvent)CloseHandle(m_hStartEvent);
    if(m_hCloseEvent)CloseHandle(m_hCloseEvent);

	//----------------------------------------------------------------------
    // Data 삭제
    //----------------------------------------------------------------------
    if(m_pLogFileAccessor)
    {
        delete m_pLogFileAccessor;
        m_pLogFileAccessor = nullptr;
    }
}

UINT CAlarmMonitorToEmergencyButton::ThreadRun(LPVOID p1)
{
    CAlarmMonitorToEmergencyButton *pThis = (CAlarmMonitorToEmergencyButton *)p1;

    pThis->m_bThreadLive = true;

    while (1)
    {
        // Event 가 set될 때까지 기다린다.
        if (WaitForSingleObject(pThis->m_hStartEvent, 10) == WAIT_OBJECT_0)
        {
			// EMERGENCY 버튼은 매우 위험하므로 최대한 빠르게 체크 한다.
			//Sleep(100);
			Sleep(50);	// jsyoon 20211020 change

            pThis->ThreadRun();
        }

        // Close Event 가 set될 때까지 기다린다.
        if (WaitForSingleObject(pThis->m_hCloseEvent, 10) == WAIT_OBJECT_0)
        {
            break;
        }

		Sleep(1);
    }

    pThis->m_bThreadLive = false;

    return 0;
}

UINT CAlarmMonitorToEmergencyButton::ThreadRun()
{
    //***********************************************************************************************************************
    //***********************************************************************************************************************
	// 설비 알람 사용 유/무
    //***********************************************************************************************************************
    //***********************************************************************************************************************
	if(m_pSystemConfigMgr->IsUseAlarmMonitor() == false)
        return 0;
	
	///
	// 연결 유무확인
	//
	if (m_pIOMapControlMgr->IsConnected() != true)
		return 0;

	//--------------------------------------------------------
    // EMO 스위치를 눌렀다. 알람 발생
    //--------------------------------------------------------

	int nRet = 0;
	nRet = m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(m_iMonitorIONumber);
	if (nRet != -1)
	{
		if (nRet == 1)
		{
			m_nNGCount++;
		}
		else
		{
			m_nNGCount = 0;
		}
	}

	// LYS 2026-09-22 EMS 는 한 번만 1 로 읽혀도 즉시 중알람 (구: 4회 연속 1 이어야 발생).
	//   읽는 값은 PLC 가 논리 처리한 M 비트 캐시라 전기적 채터링이 없고, 통신 실패(-1)는 위에서 이미 제외된다.
	//   캐시는 M 스캐너가 약 220~250ms 마다 갱신하고 이 스레드는 약 70~95ms 마다 읽어서, PLC 가 한 스캔만 1 로 잡은
	//   신호는 2~3 회만 읽혀 카운트가 3 을 못 넘었다 -> IO 그리드(같은 캐시, 1초 갱신)에는 보이는데 알람은 안 떴다.
	//   EMS 01~16(M4010~4019, M4420~4425) 16개 모두 이 클래스라 전부 적용된다.
	//   스캔 주기보다 짧은 펄스는 캐시에도 안 남으므로 그런 신호까지 잡으려면 PLC 측 래치가 필요하다.
	//if (m_nNGCount > 3)
	if (m_nNGCount >= 1)
	{
		CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, m_iEMOPosST, NULL);

		m_pLogFileAccessor->KMLOG(WARN, m_strLogMsg);
		CAlarmController::GetInstance()->SetAlarm(m_iAlarmID);

		m_nNGCount = 0;

		m_bAlarmOn = true;
	}
	else
	{
		// EMO BUTTON 상태화면을 변경한다.
		if (m_bAlarmOn == true)
		{
			CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, m_iEMOPosED, NULL);
			m_bAlarmOn = false;
		}
	}

 //   if(m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(m_iMonitorIONumber) == true)
 //   {
	//	// EMO BUTTON 상태화면을 변경한다.
	//	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, m_iEMOPosST, NULL);
	//
	//	//m_pCommTcpIPMgr->SetAlarmOccurBecauseStop();	// 특수한 경우 - 모듈 쪽으로 ALARM 을 전달해준다.
	//
	//	m_pLogFileAccessor->KMLOG(WARN, m_strLogMsg);
 //       CAlarmController::GetInstance()->SetAlarm(m_iAlarmID);
	//
	//	// 버튼 해제가 발생한 경우에는
	//	if(m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(m_iMonitorIONumber) == false)
	//	{
	//		// EMO BUTTON 상태화면을 변경한다.
	//		CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, m_iEMOPosED, NULL);
	//	}
	//}
	//
	return 0;
}

void CAlarmMonitorToEmergencyButton::SetPara(CSystemConfigMgr *pSystemConfigMgr, CIOMapControlMgr *pIOMapControlMgr, CCommTcpIPMgr *pCommTcpIPMgr)
{
    m_pSystemConfigMgr = pSystemConfigMgr;
    m_pIOMapControlMgr = pIOMapControlMgr;
    m_pCommTcpIPMgr = pCommTcpIPMgr;
}

void CAlarmMonitorToEmergencyButton::SetStart()
{
    SetEvent(m_hStartEvent);
}

void CAlarmMonitorToEmergencyButton::SetStop()
{
    ResetEvent(m_hStartEvent);
}