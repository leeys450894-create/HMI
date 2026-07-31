#include "pch.h"
#include "AlarmMonitorToSensorFAN.h"

#include "..\Manager\SystemConfigMgr.h"
//#include "..\Manager\IOMapControlMgr.h"
#include "..\Manager\IOMapAddressDefine.h"
//#include "..\Manager\MotorControlMgr.h"
#include "..\Manager\MaintControlMgr.h"
#include "AlarmController.h"
#include "..\SystemParamShare.h"
#include "..\AuroraFuncDefine.h"
#include "LogFunction\LogAccessor\LogFileAccessor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CAlarmMonitorToSensorFAN::CAlarmMonitorToSensorFAN(CString strLogPath, int iMonitorIONumber, int iEMOPosST, int iEMOPosED, CString strLogMsg, int iAlarmID)
    : m_iMonitorIONumber(iMonitorIONumber)
    , m_iEMOPosST(iEMOPosST)
    , m_iEMOPosED(iEMOPosED)
    , m_strLogMsg(strLogMsg)
    , m_iAlarmID(iAlarmID)
	, m_pLogFileAccessor(new CLogFileAccessor)
	, m_bPushHandBtn(false)
	, m_nNGCount(0)
{
    const CString strFolderPath	= strLogPath + _T("\\Event\\");
    const CString strLogFile	= strFolderPath + _T("sensor_fan.log");
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

CAlarmMonitorToSensorFAN::~CAlarmMonitorToSensorFAN()
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

UINT CAlarmMonitorToSensorFAN::ThreadRun(LPVOID p1)
{
    CAlarmMonitorToSensorFAN *pThis = (CAlarmMonitorToSensorFAN *)p1;

    pThis->m_bThreadLive = true;

    while (1)
    {
        // Event 가 set될 때까지 기다린다.
        if (WaitForSingleObject(pThis->m_hStartEvent, 10) == WAIT_OBJECT_0)
        {
			// 설비의 FAN 이 멈춘거는 IO 체크를 천천히 해도 될거 같다.
			Sleep(1000);

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

UINT CAlarmMonitorToSensorFAN::ThreadRun()
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
	if (m_pMaintControlMgr->GetMotorControlMgrPtr()->IsConnected() != true)
		return 0;

	////--------------------------------------------------------
	//// Isolater 센서, A접
	////--------------------------------------------------------
	//int nUnit = 0, nPort = 0;
	//int nInputUnit[2] = { 0, 2 };
	//int nInputPort[2] = { 3, 1 };

	//nUnit = nInputUnit[m_iMonitorIONumber];
	//nPort = nInputPort[m_iMonitorIONumber];
	//if (m_pMaintControlMgr->GetMotorControlMgrPtr()->GetInput(nUnit, nPort) != 1)
	//{
	//	m_nNGCount++;
	//}
	//else
	//{
	//	m_nNGCount = 0;
	//}

	//if (m_nNGCount > 2)
	//{
	//	m_pLogFileAccessor->KMLOG(WARN, m_strLogMsg);
	//	CAlarmController::GetInstance()->SetAlarm(m_iAlarmID);

	//	m_nNGCount = 0;
	//}

 //   //--------------------------------------------------------
	//// FAN 의 상태를 체크 한다.
 //   //--------------------------------------------------------
	//if( m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_009) == false)
	//{
	//	m_pLogFileAccessor->KMLOG(SYST, _T("ELECTRIC BOX FAN 01 OFF"));
 //       CAlarmController::GetInstance()->SetAlarm(PLC_ELECTRIC_BOX_FAN_ERROR_01);
	//}
	//if( m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(CCLINK_IO_BOARD_SYSTEM_R_BIT_066) == false)
	//{
	//	m_pLogFileAccessor->KMLOG(SYST, _T("ELECTRIC BOX FAN 02 OFF"));
 //       CAlarmController::GetInstance()->SetAlarm(PLC_ELECTRIC_BOX_FAN_ERROR_02);
	//}
	//if( m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(CCLINK_IO_BOARD_SYSTEM_R_BIT_067) == false)
	//{
	//	m_pLogFileAccessor->KMLOG(SYST, _T("ELECTRIC BOX FAN 03 OFF"));
 //       CAlarmController::GetInstance()->SetAlarm(PLC_ELECTRIC_BOX_FAN_ERROR_03);
	//}
	//if( m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(CCLINK_IO_BOARD_SYSTEM_R_BIT_068) == false)
	//{
	//	m_pLogFileAccessor->KMLOG(SYST, _T("ELECTRIC BOX FAN 04 OFF"));
 //       CAlarmController::GetInstance()->SetAlarm(PLC_ELECTRIC_BOX_FAN_ERROR_04);
	//}
	//
	//if( m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(CCLINK_IO_BOARD_SYSTEM_R_BIT_069) == false)
	//{
	//	m_pLogFileAccessor->KMLOG(SYST, _T("PC BOX FAN 01 OFF"));
 //       CAlarmController::GetInstance()->SetAlarm(PLC_PC_BOX_FAN_ERROR_01);
	//}
	//if( m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(CCLINK_IO_BOARD_SYSTEM_R_BIT_070) == false)
	//{
	//	m_pLogFileAccessor->KMLOG(SYST, _T("PC BOX FAN 02 OFF"));
 //       CAlarmController::GetInstance()->SetAlarm(PLC_PC_BOX_FAN_ERROR_02);
	//}
	//if( m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(CCLINK_IO_BOARD_SYSTEM_R_BIT_071) == false)
	//{
	//	m_pLogFileAccessor->KMLOG(SYST, _T("PC BOX FAN 03 OFF"));
 //       CAlarmController::GetInstance()->SetAlarm(PLC_PC_BOX_FAN_ERROR_03);
	//}
	//if( m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(CCLINK_IO_BOARD_SYSTEM_R_BIT_072) == false)
	//{
	//	m_pLogFileAccessor->KMLOG(SYST, _T("PC BOX FAN 04 OFF"));
 //       CAlarmController::GetInstance()->SetAlarm(PLC_PC_BOX_FAN_ERROR_04);
	//}
	//if( m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(CCLINK_IO_BOARD_SYSTEM_R_BIT_073) == false)
	//{
	//	m_pLogFileAccessor->KMLOG(SYST, _T("PC BOX FAN 05 OFF"));
 //       CAlarmController::GetInstance()->SetAlarm(PLC_PC_BOX_FAN_ERROR_05);
	//}
	//if( m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(CCLINK_IO_BOARD_SYSTEM_R_BIT_074) == false)
	//{
	//	m_pLogFileAccessor->KMLOG(SYST, _T("PC BOX FAN 06 OFF"));
 //       CAlarmController::GetInstance()->SetAlarm(PLC_PC_BOX_FAN_ERROR_06);
	//}
	//if( m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(CCLINK_IO_BOARD_SYSTEM_R_BIT_075) == false)
	//{
	//	m_pLogFileAccessor->KMLOG(SYST, _T("PC BOX FAN 07 OFF"));
 //       CAlarmController::GetInstance()->SetAlarm(PLC_PC_BOX_FAN_ERROR_07);
	//}
	//if( m_pIOMapControlMgr->GetParam_DIOMapMemoryBitImport(CCLINK_IO_BOARD_SYSTEM_R_BIT_076) == false)
	//{
	//	m_pLogFileAccessor->KMLOG(SYST, _T("PC BOX FAN 08 OFF"));
 //       CAlarmController::GetInstance()->SetAlarm(PLC_PC_BOX_FAN_ERROR_08);
	//}
	
	return 0;
}

void CAlarmMonitorToSensorFAN::SetPara(CSystemConfigMgr* pSystemConfigMgr, CMaintControlMgr* pMaintControlMgr) // SetPara(CSystemConfigMgr *pSystemConfigMgr, CIOMapControlMgr *pIOMapControlMgr, CMotorControlMgr *pMotorControlMgr)
{
    m_pSystemConfigMgr = pSystemConfigMgr;
	m_pMaintControlMgr = pMaintControlMgr;
//    m_pIOMapControlMgr = pIOMapControlMgr;
//		m_pMotorControlMgr = pMotorControlMgr;
}

void CAlarmMonitorToSensorFAN::SetStart()
{
    SetEvent(m_hStartEvent);
}

void CAlarmMonitorToSensorFAN::SetStop()
{
    ResetEvent(m_hStartEvent);
}