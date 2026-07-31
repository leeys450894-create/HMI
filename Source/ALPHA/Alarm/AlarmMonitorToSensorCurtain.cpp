#include "pch.h"
#include "AlarmMonitorToSensorCurtain.h"

#include "..\Manager\SystemConfigMgr.h"
#include "..\Manager\IOMapControlMgr.h"
#include "..\Manager\IOMapAddressDefine.h"
#include "AlarmController.h"
#include "..\SystemParamShare.h"
#include "..\AuroraFuncDefine.h"
#include "LogFunction\LogAccessor\LogFileAccessor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CAlarmMonitorToSensorCurtain::CAlarmMonitorToSensorCurtain(CString strLogPath, int iMonitorIONumber, int iEMOPosST, int iEMOPosED, CString strLogMsg, int iAlarmID)
    : m_iMonitorIONumber(iMonitorIONumber)
    , m_iEMOPosST(iEMOPosST)
    , m_iEMOPosED(iEMOPosED)
    , m_strLogMsg(strLogMsg)
    , m_iAlarmID(iAlarmID)
	, m_pLogFileAccessor(new CLogFileAccessor)
{
    const CString strFolderPath	= strLogPath + _T("\\Event\\");
    const CString strLogFile	= strFolderPath + _T("sensor_cutain.log");
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

CAlarmMonitorToSensorCurtain::~CAlarmMonitorToSensorCurtain()
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

UINT CAlarmMonitorToSensorCurtain::ThreadRun(LPVOID p1)
{
    CAlarmMonitorToSensorCurtain *pThis = (CAlarmMonitorToSensorCurtain *)p1;

    pThis->m_bThreadLive = true;

    while (1)
    {
        // Event 가 set될 때까지 기다린다.
        if (WaitForSingleObject(pThis->m_hStartEvent, 10) == WAIT_OBJECT_0)
        {
            pThis->ThreadRun();

            // 최대한 빠르게 IO 를 확인하자.
            Sleep(200);
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

UINT CAlarmMonitorToSensorCurtain::ThreadRun()
{
    //***********************************************************************************************************************
    //***********************************************************************************************************************
	// 설비 알람 사용 유/무
    //***********************************************************************************************************************
    //***********************************************************************************************************************
	if(m_pSystemConfigMgr->IsUseAlarmMonitor() == false)
        return 0;

    //--------------------------------------------------------
	// 센서로 연결되어 있는 부분
    //--------------------------------------------------------
	if(m_pSystemConfigMgr->IsUseHardwareSensor() == false)
        return 0;

 
	///
	// 연결 유무확인
	//
	if (m_pIOMapControlMgr->IsConnected() != true)
		return 0;

    //--------------------------------------------------------
    // 설비 Light Curtain
    // LIGHT CURTAIN DETECTED ON & LIGHT CURTAIN MUTING STATUS OFF
    //--------------------------------------------------------

#ifdef _USE_IOBOARD
	if( (m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_011) == false) &&
		(m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_010) == false))
    {
		m_pLogFileAccessor->KMLOG(WARN, _T("LIGHT CURTAIN DETECTED!!"));
        CAlarmController::GetInstance()->SetAlarm(PLC_ARM_SENSOR_DETECT);
	}
#else

	int nRet = 0;
	nRet = m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(PLC_BIT_ARM_DETECT_INTERLOCK_SR);
	if (nRet != -1)
	{
		if (nRet == 1)
		{
			m_pLogFileAccessor->KMLOG(WARN, m_strLogMsg);
			CAlarmController::GetInstance()->SetAlarm(PLC_ARM_SENSOR_DETECT);
		}
	}

   /* if ((m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(PLC_BIT_ARM_DETECT_INTERLOCK_SR) == false) )
    {
        m_pLogFileAccessor->KMLOG(WARN, _T("LIGHT CURTAIN DETECTED!!"));
        CAlarmController::GetInstance()->SetAlarm(PLC_ARM_SENSOR_DETECT);
    }*/
#endif // _USE_IOBOARD
	
	return 0;
}

void CAlarmMonitorToSensorCurtain::SetPara(CSystemConfigMgr *pSystemConfigMgr, CIOMapControlMgr *pIOMapControlMgr, CCommTcpIPMgr *pCommTcpIPMgr)
{
    m_pSystemConfigMgr = pSystemConfigMgr;
    m_pIOMapControlMgr = pIOMapControlMgr;
    m_pCommTcpIPMgr = pCommTcpIPMgr;
}

void CAlarmMonitorToSensorCurtain::SetStart()
{
    SetEvent(m_hStartEvent);
}

void CAlarmMonitorToSensorCurtain::SetStop()
{
    ResetEvent(m_hStartEvent);
}