#include "pch.h"
#include "FunctionModuleProbeACCStep.h"

#include "BaseSequenceImpl.h"
#include "SystemParamShare.h"
#include "..\Manager\SystemConfigMgr.h"
#include "..\Manager\MaintControlMgr.h"
#include "..\Manager\LightControlMgr.h"
#include "..\Manager\ShareMemoryMgr.h"
#include "..\Manager\SequenceManager.h"
#include "..\Manager\SACCMgr.h"
#include "FunctionModule/FunctionModuleProbeACC.h"
#include "FunctionModule/FunctionModuleEQState.h"
#include "FunctionModule\FunctionModuleEQStateChange.h"
#include "DoWork/DoWorkGlassSequence.h"
#include "..\Recipe\RecipeEditMgr.h"
#include "common/CommonFunc.h"
//#include "..\CoreFunction\CoreFunction.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

bool CFunctionModuleProbeACCStep_None::Run()
{
	// init
	m_pShareItem->SetComplete(false);
	m_pShareItem->SetAlarmOn(false);
	m_pShareItem->SetFail(false);
	m_pShareItem->SetStopUser(false);

    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleProbeACCStep_None Start."));

	m_pShareItem->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_PROCESS); // 2023.08.09 add

	/*m_pShareItem->SetAlarmOn(false);
	m_pShareItem->SetComplete(false);
	m_pShareItem->SetFail(false);*/

    m_pShareItem->GetMaintControlMgrPtr()->GetPositionConfigMgr()->LinkIniFile2();

    return true;
}

bool CFunctionModuleProbeACCStep_CheckIO::Run()
{
    // LED 조명을 OFF 시킨다.
    m_pShareItem->GetLightControlMgrPtr()->LedLight_Channel_TurnOff(0, 0, 0); // SPO
    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(OPER, _T("Do_UVLedShutterOn(), Start"));

    // 모션을 초기화 한다.
    VTM_SYSTEM *pVTMSystem = m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory();
    if(pVTMSystem == nullptr)
        return false;

	return true;
}

bool CFunctionModuleProbeACCStep_RecipeCheck::Run()
{
    return true;
}

bool CFunctionModuleProbeACCStep_MoveOrigin::Run()
{
    // 대기
    WaitForSingleObject(m_pShareItem->GetWaitTimeEventHandle(), 500);

    return true;
}

bool CFunctionModuleProbeACCStep_MoveOriginCheck::Run()
{

    return true;
}

bool CFunctionModuleProbeACCStep_Complete::Run()
{

    return true;
}

bool CFunctionModuleProbeACCStep_CompleteCheck::Run()
{

    return true;
}

bool CFunctionModuleProbeACCStep_Idle::Run()
{
    // 완료
    ResetEvent(m_pShareItem->GetStartEventHandle());
  //  m_pShareItem->GoToInitStep();

	if (m_pShareItem->GetFail() == false)
	{
		m_pShareItem->SetComplete(true);   // 동작 완료
	}

    SetEvent(m_pShareItem->GetCompleteEventHandle()); // 시퀀스 완료 이벤트 SET

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_WORK_SACC_End, NULL);

    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("FunctionModuleProbeACCStep_Idle."));

	m_pShareItem->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_IDLE);	// 2023.08.09 add

    return false;
}

bool CFunctionModuleProbeACCStep_ProbeChangePosition::Run()
{
    // AC Y축 이동, 위치 센서 감지 확인(PROBE 교체 위치), Probe X, Y축 이동, 위치 센서 감지 확인(PROBE 교체 위치)
    int nRet = VTM_RV_SUCCESSED;

    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleProbeACCStep_ProbeChangePosition Start."));

    double dPosProbeZ = 0;

     // Probe Z 교체 위치 받음
    m_pShareItem->GetMaintControlMgrPtr()->GetPositionConfigMgr()->GetProbeZChangePos(nullptr, &dPosProbeZ, nullptr);

    // Probe Zero
    nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeMove(AXIS_ALL_Z, dPosProbeZ, SPEED_HIGH);

    if (nRet != VTM_RV_SUCCESSED)
    {
        return false;
    }

    // Probe 확인 
    int iProbe = CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->GetProbe();
    if (iProbe < df_PROBE_AGING1)
    {
        return false;
    }

    // AC Y PROBE 교체 위치 받음
    double dChangePos = 0;
    m_pShareItem->GetMaintControlMgrPtr()->GetPositionConfigMgr()->GetACPosition(&dChangePos, nullptr, nullptr);

    // AC Y 교체 위치 이동
    nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_X4, dChangePos, true);
    if (nRet != VTM_RV_SUCCESSED)
    {
        return false;
    }
    
	Sleep(1000);

    // 위치 센서 감지 확인(PROBE 교체 위치)
    if (!CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsPositionOnAC(0) == true)
        return false;

    // Probe X, Y축 이동
    double dChangeProbePos = 0;
    m_pShareItem->GetMaintControlMgrPtr()->GetPositionConfigMgr()->GetProbeChangePos(&dChangeProbePos);

    // Probe 교체 위치 이동
    nRet = m_pShareItem->GetMaintControlMgrPtr()->Move_SelectProbe(iProbe, dChangeProbePos);

    if (nRet != VTM_RV_SUCCESSED)
    {
        return false;
    }

    return true;
}

bool CFunctionModuleProbeACCStep_SACCChangePosition::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleProbeACCStep_SACCChangePosition Start."));

    // Auto Mode
	if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_PACC_AUTO_MODE, 1, 1, true, PLC_IO_ACTION_DONE_TIME_03) == true)
	{
		// AC Y SACC 교체 위치로 움직이기전 "Entry Request" BIT 명령 대기 
		if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_PACC_ENTRY_REQUEST, 1, 1, true, PLC_IO_ACTION_DONE_TIME_60 * 20) != true)
		{
			return false;
		}

		//if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_PACC_ENTRY_REQUEST, 1, 1, true, PLC_IO_ACTION_DONE_TIME_60 * 20) == true)
		//{
		//	//// Shutter Open 확인 
		//	//if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsACShutterOpen() != true)
		//	//{
		//	//	return false;
		//	//}
		//}
  //      else
  //      {
  //          return false;
  //      }
	}
    //else
    //{
    //    // Shutter Open 확인 
    //    if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsACShutterOpen() != true)
    //    {
    //        return false;
    //    }
    //}

	// Shutter Open 확인 
	bool bShutterOpen = false;
	bShutterOpen = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsACShutterOpen();
	if (bShutterOpen != true)
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(WARN, _T("CFunctionModuleProbeACCStep_SACCChangePosition Shutter Open Error."));
		return false;
	}

	// Buffer Pos 확인
	bool bBufferLoadPos = false;
	bool bBufferUnloadPos = false;
	bBufferLoadPos = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsBufferLoadPos();
	bBufferUnloadPos = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsBufferUnloadPos();
	if (bBufferLoadPos != true && bBufferUnloadPos != true)
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(WARN, _T("CFunctionModuleProbeACCStep_SACCChangePosition Buffer Pos Error."));
		return false;
	}


    // Buffer 위치 확인 추가

	// jsyoon 2022.05.03 change position
    //// Status Busy On
    //CSequenceManager::GetInstance()->GetFunctionModuleEQState()->SetStatusbusyOn();
    
    int nRet = VTM_RV_SUCCESSED;
    double dPosProbeZ = 0;
    // Probe Z 교체 위치 받음
    m_pShareItem->GetMaintControlMgrPtr()->GetPositionConfigMgr()->GetProbeZChangePos(nullptr, &dPosProbeZ, nullptr);

    // Probe Zero
    nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeMove(AXIS_ALL_Z, dPosProbeZ, SPEED_HIGH);

   /* stMotorPosition stPositionProbeZ[5];
    ZeroMemory(&stPositionProbeZ, sizeof(stPositionProbeZ));
    stPositionProbeZ[0].axis = AXIS_PROBE_Z1;
    stPositionProbeZ[0].dPosition = dPosProbeZ;
    stPositionProbeZ[1].axis = AXIS_PROBE_Z2;
    stPositionProbeZ[1].dPosition = dPosProbeZ;
    stPositionProbeZ[2].axis = AXIS_PROBE_Z3;
    stPositionProbeZ[2].dPosition = dPosProbeZ;
    stPositionProbeZ[3].axis = AXIS_PROBE_Z4;
    stPositionProbeZ[3].dPosition = dPosProbeZ;
    stPositionProbeZ[4].axis = AXIS_PROBE_Z5;
    stPositionProbeZ[4].dPosition = dPosProbeZ;

    nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosMulti(stPositionProbeZ, 5, SPEED_HIGH, true);*/
    if (nRet != VTM_RV_SUCCESSED)
    {
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(WARN, _T("CFunctionModuleProbeACCStep_SACCChangePosition Probe Z Move Error."));
        return false;
    }

    // AC Z SACC 교체 위치 받음
    double dSACCPosZ = 0;
    m_pShareItem->GetMaintControlMgrPtr()->GetPositionConfigMgr()->GetACZPosition(nullptr, nullptr, &dSACCPosZ);

    // AC Z SACC 교체 위치 이동
    nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_Z9, dSACCPosZ, true);
    if (nRet != VTM_RV_SUCCESSED)
    {
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(WARN, _T("CFunctionModuleProbeACCStep_SACCChangePosition AC Z Move Error."));
        return false;
    }

	// jsyoon 2022.05.03 change position
	// Status Busy On
	CSequenceManager::GetInstance()->GetFunctionModuleEQState()->SetStatusbusyOn();

    // AC Y SACC 교체 위치 받음
    double dSACCPos = 0;
    m_pShareItem->GetMaintControlMgrPtr()->GetPositionConfigMgr()->GetACPosition(nullptr, nullptr, &dSACCPos);

    // AC Y SACC 교체 위치 이동
    nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_X4, dSACCPos, true, SPEED_MID);
    if (nRet != VTM_RV_SUCCESSED)
    {
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(WARN, _T("CFunctionModuleProbeACCStep_SACCChangePosition AC X Move Error."));
        return false;
    }

	Sleep(1000);

    // 위치센서 감지 확인(SACC 교체 위치)
	if (!CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsPositionOnAC(2) == true)
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(WARN, _T("CFunctionModuleProbeACCStep_SACCChangePosition AC X SACC Pos Sensor Error."));
		return false;
	}
        
    // Status Busy Off
    CSequenceManager::GetInstance()->GetFunctionModuleEQState()->SetStatusbusyOff();

    return true;
}

bool CFunctionModuleProbeACCStep_ACReadyPosition::Run()
{ 
    bool bRet = true;  
    int nRet = VTM_RV_SUCCESSED;
    double dPosProbeZ = 0;

    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleProbeACCStep_ACReadyPosition Start."));
    
    // Probe Z 교체 위치 받음
    m_pShareItem->GetMaintControlMgrPtr()->GetPositionConfigMgr()->GetProbeZChangePos(nullptr ,&dPosProbeZ, nullptr);

    // Probe Zero
    nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeMove(AXIS_ALL_Z, dPosProbeZ, SPEED_HIGH);

    /*stMotorPosition stPositionProbeZ[5];
    ZeroMemory(&stPositionProbeZ, sizeof(stPositionProbeZ));
    stPositionProbeZ[0].axis = AXIS_PROBE_Z1;
    stPositionProbeZ[0].dPosition = dPosProbeZ;
    stPositionProbeZ[1].axis = AXIS_PROBE_Z2;
    stPositionProbeZ[1].dPosition = dPosProbeZ;
    stPositionProbeZ[2].axis = AXIS_PROBE_Z3;
    stPositionProbeZ[2].dPosition = dPosProbeZ;
    stPositionProbeZ[3].axis = AXIS_PROBE_Z4;
    stPositionProbeZ[3].dPosition = dPosProbeZ;
    stPositionProbeZ[4].axis = AXIS_PROBE_Z5;
    stPositionProbeZ[4].dPosition = dPosProbeZ;

    nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosMulti(stPositionProbeZ, 5, SPEED_HIGH, true);*/
    if (nRet != VTM_RV_SUCCESSED)
    {
		m_pShareItem->SetFail(true, nRet);
        return false;
    }

    // AC Y 대기 위치 받음
    double dReadyPos = 0;
    m_pShareItem->GetMaintControlMgrPtr()->GetPositionConfigMgr()->GetACPosition(nullptr, &dReadyPos, nullptr);

    if (m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_X4) > dReadyPos)        // SACC to Safety
    {
        // Move Y -> Z
        // AC Y 대기 위치 이동
        nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_X4, dReadyPos, true, SPEED_MID);

        // AC Z AC 대기 위치 이동
        if (nRet == VTM_RV_SUCCESSED)
        {
            nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_Z9, 0, true);
        }
    }
    else if (m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_X4) < dReadyPos) // Probe Card to Safety
    {
        // Move Z -> Y
        // AC Z AC 대기 위치 이동
        if (nRet == VTM_RV_SUCCESSED)
        {
            nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_Z9, 0, true);
        }

        // AC Y 대기 위치 이동
        if (nRet == VTM_RV_SUCCESSED)
        {
            nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_X4, dReadyPos, true);
        }
    }
    else 
    {
        // Move Z -> Y
        // AC Z AC 대기 위치 이동
        if (nRet == VTM_RV_SUCCESSED)
        {
            nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_Z9, 0, true);
        }

        // AC Y 대기 위치 이동
        if (nRet == VTM_RV_SUCCESSED)
        {
            nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_X4, dReadyPos, true);
        }
    }

	Sleep(1000);

    // 위치센서 감지 확인(AC 대기 위치)
    if (nRet == VTM_RV_SUCCESSED)
    {
		if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsPositionOnAC(1) != true)
		{
			nRet = AC_ABNORMAL_POS;
		}
	}
	
	if( nRet != VTM_RV_SUCCESSED)
	{
		m_pShareItem->SetFail(true, nRet);
		bRet = false;
	}

	return bRet;
}

bool CFunctionModuleProbeACCStep_ReadyPos::Run()
{
	double dPosProbe = 0, dPosACZ = 0;

	// Unit Z 대기 위치 받음
	m_pShareItem->GetMaintControlMgrPtr()->GetPositionConfigMgr()->GetProbeChangePos(&dPosProbe);

	// Unit Z 대기 위치 이동 
	if (!m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_UNIT_Z, 0, true))
		return false;

	// AC Z 대기 위치 이동(0)
	if (m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_Z9, 0, true))
		return true;
	else
		return false;
}

bool CFunctionModuleProbeACCStep_MeasurePos::Run()
{
	double dPosProbe = 0, dPosACZ = 0;

	// Uniz Z 측정 위치 받음
	m_pShareItem->GetMaintControlMgrPtr()->GetPositionConfigMgr()->GetProbeChangePos(&dPosProbe);

	//AC Z 측정 위치 받음 
	m_pShareItem->GetMaintControlMgrPtr()->GetPositionConfigMgr()->GetACZPosition(&dPosACZ, nullptr, nullptr);

	// Uniz Z 측정 위치 이동
	if (!m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_UNIT_Z, dPosProbe, true))
		return false;

	// AC Z 측정 위치 이동
	if (!m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_Z9, dPosACZ, true))
		return false;

	return true;
}

bool CFunctionModuleProbeACCStep_ACZxisUp::Run()
{
	bool bRet = false;
	double dPosProbeZ = 0, dPosACZ = 0;
	int nRet = VTM_RV_SUCCESSED;

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleProbeACCStep_ACZxisUp Start."));

	// Probe 확인 
	int iProbe = CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->GetProbe();
	if (iProbe < df_PROBE_AGING1)
	{
		return false;
	}

	int Axis_Z = AXIS_PROBE_Z1;
	switch (iProbe)
	{
	case df_PROBE_AGING1:
	{
		Axis_Z = AXIS_PROBE_Z1;
	}
	break;
	case df_PROBE_AGING2:
	{
		Axis_Z = AXIS_PROBE_Z2;
	}
	break;
	case df_PROBE_AGING3:
	{
		Axis_Z = AXIS_PROBE_Z3;
	}
	break;
	case df_PROBE_AGING4:
	{
		Axis_Z = AXIS_PROBE_Z4;
	}
	break;
	case df_PROBE_MEAS:
	{
		Axis_Z = AXIS_PROBE_Z5;
	}
	break;
	}

	// Probe Exist
	if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsProbeExistSensorOn(iProbe) != true)
	{
		// PCB unlock
		if (nRet == VTM_RV_SUCCESSED)
		{
			nRet = m_pShareItem->GetMaintControlMgrPtr()->Do_PCB_UNIT_Unlock(iProbe);

			if (nRet != VTM_RV_SUCCESSED)
			{
				bRet = false;
			}
		}

		// Probe Clamp Check
		if (nRet == VTM_RV_SUCCESSED)
		{
			nRet = m_pShareItem->GetMaintControlMgrPtr()->Do_PROBE_CLAM_Unlock(iProbe);

			if (nRet != VTM_RV_SUCCESSED)
			{
				bRet = false;
			}
		}

		// Zero Unlock
		if (nRet == VTM_RV_SUCCESSED)
		{
			nRet = m_pShareItem->GetMaintControlMgrPtr()->Do_Zero_Position_UnLock(iProbe);

			if (nRet != VTM_RV_SUCCESSED)
			{
				bRet = false;
			}
		}
	}

	// AC Z 교체 위치 받음
	m_pShareItem->GetMaintControlMgrPtr()->GetPositionConfigMgr()->GetACZPosition(&dPosACZ, nullptr, nullptr);

	if (nRet == VTM_RV_SUCCESSED)
	{
		// AC Z 교체 위치 이동
		nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_Z9, dPosACZ - 5, true);
		if (nRet != VTM_RV_SUCCESSED)
		{
			bRet = false;
		}
	}


	if (nRet == VTM_RV_SUCCESSED)
	{
		// jsyoon 2022.05.10 add
		nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_Z9, dPosACZ, true, SPEED_LOW);
		if (nRet != VTM_RV_SUCCESSED)
		{
			bRet = false;
		}
	}

	// AC Z
	if (nRet == VTM_RV_SUCCESSED)
	{
		// Probe Z 교체 위치 받음
		m_pShareItem->GetMaintControlMgrPtr()->GetPositionConfigMgr()->GetProbeZChangePos(&dPosProbeZ, nullptr, nullptr);

		nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(Axis_Z, dPosProbeZ, true);

		if (nRet == VTM_RV_SUCCESSED)
		{
			Sleep(1000);

			if (m_pShareItem->GetMaintControlMgrPtr()->IsOutOfServiceMC() != true)
			{
				// AC Z Up 센서 감지 확인 
				if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsACZxisUp() == true &&
					CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsACZxisDown() == false)
				{
					bRet = true;
				}
				else
				{
					bRet = false;
				}
			}
			else
			{
				bRet = true;
			}
		}
		else
		{
			bRet = false;
		}
	}

	if (nRet != VTM_RV_SUCCESSED)
	{
		m_pShareItem->SetFail(true, nRet);
	}
    
    return bRet;
}

bool CFunctionModuleProbeACCStep_ACZxisDown::Run()
{
    bool bRet = false;

    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleProbeACCStep_ACZxisDown Start."));

    int nRet = VTM_RV_SUCCESSED;
    // Probe 확인 
    int iProbe = CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->GetProbe();
    if (iProbe < df_PROBE_AGING1)
    {
        return false;
    }

    int Axis_Z = AXIS_PROBE_Z1;
    switch (iProbe)
    {
    case df_PROBE_AGING1:
    {
        Axis_Z = AXIS_PROBE_Z1;
    }
    break;
    case df_PROBE_AGING2:
    {
        Axis_Z = AXIS_PROBE_Z2;
    }
    break;
    case df_PROBE_AGING3:
    {
        Axis_Z = AXIS_PROBE_Z3;
    }
    break;
    case df_PROBE_AGING4:
    {
        Axis_Z = AXIS_PROBE_Z4;
    }
    break;
    case df_PROBE_MEAS:
    {
        Axis_Z = AXIS_PROBE_Z5;
    }
    break;
    }

    // Probe Z 교체 위치 받음
    double dPosProbeZ = 0;
    m_pShareItem->GetMaintControlMgrPtr()->GetPositionConfigMgr()->GetProbeZChangePos(nullptr, &dPosProbeZ, nullptr);

    nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(Axis_Z, dPosProbeZ, true);

    if (nRet == VTM_RV_SUCCESSED)
    {
		double dCurZ = 0.;
		dCurZ = m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_Z9);
		nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_Z9, dCurZ - 5, true, SPEED_MID);

		if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsProbeExistSensorOn(iProbe) == true && CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsACProbeExistSensorOn() == true)
			nRet = PROBE_EXIST_ERROR;
		else if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsProbeExistSensorOn(iProbe) != true && CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsACProbeExistSensorOn() != true)
			nRet = PROBE_EXIST_ERROR;

		if (nRet == VTM_RV_SUCCESSED)
		{
			// AC Z 대기 위치(0) 이동
			nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_Z9, 0, true);
			if (nRet == VTM_RV_SUCCESSED)
			{
				Sleep(1000);

				// AC Z Down 센서 감지 확인
				if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsACZxisUp() == false &&
					CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsACZxisDown() == true)
				{
					bRet = true;
				}
				else
				{
					bRet = false;
				}
			}
			else
			{
				bRet = false;
			}
		}
		else
		{
			bRet = false;
		}  
    }
	else
	{
		bRet = false;
	}

    return bRet;
}

bool CFunctionModuleProbeACCStep_ShutterOpen::Run()
{
    bool bRet = false;

    if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsACShutterOpen() == true)
        bRet = true;
    else
        bRet = false;

    return bRet;
}

bool CFunctionModuleProbeACCStep_SACCGetReq::Run()
{
    bool bRet = false;
	int nRet = VTM_RV_SUCCESSED;

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleProbeACCStep_SACCGetReq Start."));

    // CST 위치 확인 
    int iLoadCassette = CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->GetLoadCassette();

    if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_PACC_AUTO_MODE, 1, 1, true, 1) == true)
    {
		if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_PACC_PACC_ALL_SEQ_DONE, 1, 1, true, 10 * 60) == true)
		{
			// CST Load 명령  
			if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->SetEQLoad(iLoadCassette) == true)
				bRet = true;
			else
				bRet = false;
		}
		else
		{
			bRet = false;
		}
    }
    else
    {
        bRet = false;
    }

	if (bRet != true)
	{
		m_pShareItem->SetFail(true, JOB_PROC_SACC_SEQ_ERROR);
	}
    
    return bRet;
}

bool CFunctionModuleProbeACCStep_SACCPutReq::Run()
{
    bool bRet = false;

    // CST 위치 확인 
    int iUnLoadCassette = CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->GetUnLoadCassette();

    int iProbe = CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->GetProbe();
    if (iProbe < df_PROBE_AGING1)
    {
        return false;
    }

    // Probe Data 셋팅
    bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->SetEQSetPrboeData(iProbe - df_PROBE_AGING1);
    if (bRet == true)
    {
        if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_PACC_AUTO_MODE, 1, 1, true, 1) == true)
        {
			if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_PACC_PACC_ALL_SEQ_DONE, 1, 1, true, 10 * 60) == true)
			{
				// CST UnLoad 명령  
				if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->SetEQUnLoad(iUnLoadCassette) == true)
					bRet = true;
				else
					bRet = false;
			}
			else
			{
				bRet = false;
			}
        }
        else
        {
            bRet = false;
        }
    }

    return bRet;
}


bool CFunctionModuleProbeACCStep_SACCExChangeReq::Run()
{
    bool bRet = false;

    // CST 위치 확인 
    int iLoadCassette = CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->GetLoadCassette();
    int iUnLoadCassette = CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->GetUnLoadCassette();

    int iProbe = CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->GetProbe();
    if (iProbe < df_PROBE_AGING1)
    {
        return false;
    }

    // Probe Data 셋팅
    bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->SetEQSetPrboeData(iProbe - df_PROBE_AGING1);
    if (bRet == true)
    {
        if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_PACC_AUTO_MODE, 1, 1, true, 1) == true)
        {
			if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_PACC_PACC_ALL_SEQ_DONE, 1, 1, true, 10 * 60) == true)
			{
				// CST ExChange 명령  
				if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->SetEQExchange(iLoadCassette, iUnLoadCassette) == true)
					bRet = true;
				else
					bRet = false;
			}
			else
			{
				bRet = false;
			}
        }
        else
        {
			bRet = false;
        }
    }

    return bRet;
}

bool CFunctionModuleProbeACCStep_SACCCompleteReq::Run()
{
    bool bRet = false;   

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleProbeACCStep_SACCCompleteReq Start."));

	if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_PACC_AUTO_MODE, 1, 1, true, 1) == true)
	{
		// Complete On 명령 전달 
		if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->SetEQCompleteOn() == true)
		{
			// Complete Ack 대기 
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_PACC_STATUS_COMPLETE_ACK);

			if (bRet)
			{
				// Complete Off 명령 전달 
				bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->SetEQCompleteOff();
			}
			else
			{
				bRet = false;
			}
		}
		else
		{
			bRet = false;
		}
	}
    
 //   if (bRet == true)
 //   {
 //       int iProbe = CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->GetProbe();
 //       if (iProbe < df_PROBE_AGING1)
 //       {
 //           return false;
 //       }

 //       CSequenceManager::GetInstance()->GetFunctionModuleEQState()->SetEQClearPrboeData(iProbe - df_PROBE_AGING1);

	////	CSequenceManager::GetInstance()->GetFunctionModuleEQState()->SetEQGetPrboeData(iProbe - df_PROBE_AGING1);	
 //   }

	CSequenceManager::GetInstance()->GetFunctionModuleEQState()->SetEQClear(bRet);

	// jsyoon 2022.08.10 add
	int nRet = VTM_RV_SUCCESSED;
	nRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_MotionDone(AXIS_Z9);
	if (nRet != VTM_RV_SUCCESSED)
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(WARN, _T("CFunctionModuleProbeACCStep_SACCToACReadyPosition AC Z Move Error."));
		return false;
	}

	Sleep(1000);

	// AC Z Down 센서 감지 확인
	if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsACZxisUp() == false &&
		CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsACZxisDown() == true)
	{
		bRet = true;
	}
	else
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(WARN, _T("CFunctionModuleProbeACCStep_SACCToACReadyPosition AC Z Down Sensor Error."));
		bRet = false;
	}
	///

    return bRet;
}

bool CFunctionModuleProbeACCStep_ACProbeExistSensorOn::Run()
{
    bool bRet = false;

    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleProbeACCStep_ACProbeExistSensorOn Start."));

    // AC Arm에 Probe 감시 센서 확인(감지)
    if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsACProbeExistSensorOn() == true)
        bRet = true;
    else
        bRet = false;

    return bRet;
}

bool CFunctionModuleProbeACCStep_ACProbeExistSensorOff::Run()
{
    bool bRet = false;

    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleProbeACCStep_ACProbeExistSensorOff Start."));

    // AC Arm에 Probe 감시 센서 확인(미감지)
    if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsACProbeExistSensorOff() == true)
        bRet = true;
    else
        bRet = false;

    return bRet;
}

bool CFunctionModuleProbeACCStep_CardProbeExistSensorOn::Run()
{
    bool bRet = false;

    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleProbeACCStep_CardProbeExistSensorOn Start."));

    // Probe 위치 확인 
    int iProbe = CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->GetProbe();
    if (iProbe < df_PROBE_AGING1)
    {
        return false;
    }

    // Probe Card에 Probe 감지 센서 확인(감지)
    if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsProbeExistSensorOn(iProbe) == true)
        bRet = true;
    else
        bRet = false;
   
    return bRet;
}

bool CFunctionModuleProbeACCStep_CardProbeExistSensorOff::Run()
{
    bool bRet = false;

    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleProbeACCStep_CardProbeExistSensorOff Start."));

    // Probe 위치 확인 
    int iProbe = CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->GetProbe();
    if (iProbe < df_PROBE_AGING1)
    {
		m_pShareItem->SetFail(true, JOB_PROC_SACC_SEQ_ERROR);
        return false;
    }

    // Probe Card에 Probe 감지 센서 확인(미감지)
	if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsProbeExistSensorOff(iProbe) == true)
	{
		bRet = true;
	}
	else
	{
		bRet = false;
		m_pShareItem->SetFail(true, PROBE_EXIST_ERROR);
	}
        

    return bRet;
}

bool CFunctionModuleProbeACCStep_CardExistProbe_ZMOVE::Run()
{
    bool bRet = false;
    int nRtn = VTM_RV_SUCCESSED;

    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleProbeACCStep_CardExistProbe_ZMOVE Start."));

    double dPosACZ = 0;
    // AC Z 교체 위치 받음
    m_pShareItem->GetMaintControlMgrPtr()->GetPositionConfigMgr()->GetACZPosition(&dPosACZ, nullptr, nullptr);

    // AC Z 교체 위치 이동
    nRtn = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_Z9, dPosACZ - 5, true);
    if (nRtn == VTM_RV_SUCCESSED)
    {
        bRet = true;
    }      

    return bRet;
}

bool CFunctionModuleProbeACCStep_Align::Run()
{
    bool bRet = false;

	if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsGlassSensorOn() == true)
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleProbeACCStep_Align Start."));

		// Probe 확인 
		int iProbe = CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->GetProbe();
		if (iProbe < df_PROBE_AGING1)
		{
			return false;
		}

		double dChangePos = 0, dTempPos = 0;
		double dPosProbeOffsetX = 0;
		
		//CString strRecipeName;
				//switch (iProbe)
				//{
				//case df_PROBE_AGING1:
				//	strRecipeName = _T("PROBE_ALIGN1");
				//	break;
				//case df_PROBE_AGING2:
				//	strRecipeName = _T("PROBE_ALIGN2");
				//	break;
				//case df_PROBE_AGING3:
				//	strRecipeName = _T("PROBE_ALIGN3");
				//	break;
				//case df_PROBE_AGING4:
				//	strRecipeName = _T("PROBE_ALIGN4");
				//	break;
				//case df_PROBE_MEAS:
				//	strRecipeName = _T("PROBE_MEAS");
				//	break;
				//default:
				//	break;
				//}

				// jsyoon 2022.08.16 change
		CString strRecipeName;
		CString strData, strProbeName;

		switch (iProbe)
		{
		case df_PROBE_AGING1:
			strData.Format(_T("%s"), m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_szPROBEID[0]);
			if (strData.IsEmpty() != true)
			{
				strProbeName.Format(_T("%s"), strData.Left(2));
				strRecipeName.Format(_T("%s_%s"), _T("PROBE_ALIGN1"), strProbeName);
			}
			else
			{
				strRecipeName.Format(_T("%s"), _T("PROBE_ALIGN1"));
			}

			break;
		case df_PROBE_AGING2:
			strData.Format(_T("%s"), m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_szPROBEID[1]);
			if (strData.IsEmpty() != true)
			{
				strProbeName.Format(_T("%s"), strData.Left(2));
				strRecipeName.Format(_T("%s_%s"), _T("PROBE_ALIGN2"), strProbeName);
			}
			else
			{
				strRecipeName.Format(_T("%s"), _T("PROBE_ALIGN2"));
			}
			break;
		case df_PROBE_AGING3:
			strData.Format(_T("%s"), m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_szPROBEID[2]);
			if (strData.IsEmpty() != true)
			{
				strProbeName.Format(_T("%s"), strData.Left(2));
				strRecipeName.Format(_T("%s_%s"), _T("PROBE_ALIGN3"), strProbeName);
			}
			else
			{
				strRecipeName.Format(_T("%s"), _T("PROBE_ALIGN3"));
			}
			break;
		case df_PROBE_AGING4:
			strData.Format(_T("%s"), m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_szPROBEID[3]);
			if (strData.IsEmpty() != true)
			{
				strProbeName.Format(_T("%s"), strData.Left(2));
				strRecipeName.Format(_T("%s_%s"), _T("PROBE_ALIGN4"), strProbeName);
			}
			else
			{
				strRecipeName.Format(_T("%s"), _T("PROBE_ALIGN4"));
			}
			break;
		case df_PROBE_MEAS:
			strData.Format(_T("%s"), m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_szPROBEID[4]);
			if (strData.IsEmpty() != true)
			{
				strProbeName.Format(_T("%s"), strData.Left(2));
				strRecipeName.Format(_T("%s_%s"), _T("PROBE_MEAS"), strProbeName);
			}
			else
			{
				strRecipeName.Format(_T("%s"), _T("PROBE_MEAS"));
			}
			break;
		default:
			strRecipeName = _T("PROBE_MEAS");
			break;
		}

		if (m_pShareItem->GetRecipeEditMgrPtr()->OpenAlignRecipe(strRecipeName) == VTM_RV_SUCCESSED)
		{
			m_pShareItem->GetMaintControlMgrPtr()->GetShareMemoryMgrPtr()->SetSys_ProbeChange(1);

			// Align Sequence 호출 
			CDoWorkGlassSequence* pDoWorkGlassSeq = CSequenceManager::GetInstance()->GeDoWorkGlassSequence();
			bRet = pDoWorkGlassSeq->ItemStart(_T("SEQUENCE_ITEM_DWGLASS_SEQ_PROBE_SW_ALIGN"), true);

			m_pShareItem->GetMaintControlMgrPtr()->GetShareMemoryMgrPtr()->SetSys_ProbeChange(0);

			// jsyoon 2022.09.26 add Probe Align Set
			if (bRet == true)
			{
				CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetProbeAlign(iProbe - 1);
			}
			
		}
	}
	else
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleProbeACCStep_Align Skip."));
		bRet = true;
	}

    return bRet;
}

bool CFunctionModuleProbeACCStep_ZEROLOCK::Run()
{
    bool bRet = false;

    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleProbeACCStep_ZEROLOCK Start."));

    // Probe 위치 확인 
    int iProbe = CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->GetProbe();
    if (iProbe >= df_PROBE_AGING1)
    {
        int nRtn = VTM_RV_SUCCESSED;
        nRtn = m_pShareItem->GetMaintControlMgrPtr()->Do_Zero_Position_Lock(iProbe);
        if (nRtn == VTM_RV_SUCCESSED)
        {
            bRet = true;
        }
    }

    

    //// PROBE CARD ZERO LOCK
    //if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsProbeZerolockSensorOn(iProbe) == true)
    //    bRet = true;
    //else
    //    bRet = false;

    return bRet;
}

bool CFunctionModuleProbeACCStep_CLAMPLOCK::Run()
{
    bool bRet = false;

    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleProbeACCStep_CLAMPLOCK Start."));

    // Probe 위치 확인 
    int iProbe = CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->GetProbe();
    if (iProbe >= df_PROBE_AGING1)
    {
        int nRtn = VTM_RV_SUCCESSED;
        nRtn = m_pShareItem->GetMaintControlMgrPtr()->Do_PROBE_CLAM_Lock(iProbe);
        if (nRtn == VTM_RV_SUCCESSED)
        {
            bRet = true;
        }
    }
   

    //// PROBE CARD CLAMP LOCK
    //if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsProbeClampFold(iProbe) == true)
    //    bRet = true;
    //else
    //    bRet = false;

    return bRet;    
}

bool CFunctionModuleProbeACCStep_PCBLOCK::Run()
{
    bool bRet = false;

    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleProbeACCStep_PCBLOCK Start."));

    // Probe 위치 확인 
    int iProbe = CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->GetProbe();
    if (iProbe >= df_PROBE_AGING1)
    {
        int nRtn = VTM_RV_SUCCESSED;
        nRtn = m_pShareItem->GetMaintControlMgrPtr()->Do_PCB_UNIT_Lock(iProbe);
        if (nRtn == VTM_RV_SUCCESSED)
        {
            bRet = true;
        }
    }
   

   /*   PROBE CARD PCB LOCK
    if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsProbePCBFold(iProbe) == true)
        bRet = true;
    else
        bRet = false;*/

    return bRet;
}

bool CFunctionModuleProbeACCStep_ZEROUNLOCK::Run()
{
    bool bRet = false;

    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleProbeACCStep_ZEROUNLOCK Start."));

    // Probe 위치 확인 
    int iProbe = CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->GetProbe();
    if (iProbe >= df_PROBE_AGING1)
    {
        int nRtn = VTM_RV_SUCCESSED;
        nRtn = m_pShareItem->GetMaintControlMgrPtr()->Do_Zero_Position_UnLock(iProbe);
        if (nRtn == VTM_RV_SUCCESSED)
        {
			Sleep(1000);
            bRet = true;
        }
    }
    

    //// PROBE CARD ZERO UNLOCK
    //if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsProbeZeroUnlockSensorOn(iProbe) == true)
    //    bRet = true;
    //else
    //    bRet = false;

    return bRet;
}

bool CFunctionModuleProbeACCStep_CLAMPUNLOCK::Run()
{
    bool bRet = false;

    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleProbeACCStep_CLAMPUNLOCK Start."));

    // Probe 위치 확인 
    int iProbe = CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->GetProbe();
    if (iProbe >= df_PROBE_AGING1)
    {
        int nRtn = VTM_RV_SUCCESSED;
        nRtn = m_pShareItem->GetMaintControlMgrPtr()->Do_PROBE_CLAM_Unlock(iProbe);
        if (nRtn == VTM_RV_SUCCESSED)
        {
            bRet = true;
        }
    }
   

    //// PROBE CARD CLAMP UNLOCK
    //if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsProbeClampUnFold(iProbe) == true)
    //    bRet = true;
    //else
    //    bRet = false;

    return bRet;
}

bool CFunctionModuleProbeACCStep_PCBUNLOCK::Run()
{
    bool bRet = false;

    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleProbeACCStep_PCBUNLOCK Start."));

    // Probe 위치 확인 
    int iProbe = CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->GetProbe();
    if (iProbe >= df_PROBE_AGING1)
    {
        int nRtn = VTM_RV_SUCCESSED;
        nRtn = m_pShareItem->GetMaintControlMgrPtr()->Do_PCB_UNIT_Unlock(iProbe);
        if (nRtn == VTM_RV_SUCCESSED)
        {
            bRet = true;
        }
    }
   

    ////  PROBE CARD PCB UNLOCK
    //if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsProbePCBUnFold(iProbe) == true)
    //    bRet = true;
    //else
    //    bRet = false;

    return bRet;
}

bool CFunctionModuleProbeACCStep_SACCToACReadyPosition::Run()
{
    bool bRet = false;

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleProbeACCStep_SACCToACReadyPosition Start."));

    if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_PACC_AUTO_MODE, 1, 1, true, 1) == true)
    {
        // AC Y AC 대기 위치로 움직이기전 "Hand Fold Request" BIT 명령 대기 
        if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_PACC_HANDFOLD_COMPLETE_ACK, 1, 1, true, PLC_IO_ACTION_DONE_TIME_60 * 5) != true)
        {
            return false;
        }
    }


	// Buffer Pos 확인
	bool bBufferLoadPos = false;
	bool bBufferUnloadPos = false;
	bBufferLoadPos = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsBufferLoadPosDone();
	bBufferUnloadPos = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsBufferUnloadPosDone();
	if (bBufferLoadPos != true && bBufferUnloadPos != true)
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(WARN, _T("CFunctionModuleProbeACCStep_SACCToACReadyPosition Buffer Pos Error."));
		return false;
	}

    // Status Busy On
    bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->SetStatusbusyOn();

    int nRet = VTM_RV_SUCCESSED;

    // ACY -> ACZ
    if (bRet == true)
    {
        // AC Y 대기 위치 받음
        double dReadyPos = 0;
        m_pShareItem->GetMaintControlMgrPtr()->GetPositionConfigMgr()->GetACPosition(nullptr, &dReadyPos, nullptr);

        // AC Y 대기 위치 이동
        nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_X4, dReadyPos, true, SPEED_MID);
        if (nRet != VTM_RV_SUCCESSED)
        {
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(WARN, _T("CFunctionModuleProbeACCStep_SACCToACReadyPosition AC X Move Error."));
            return false;
        }
    }

	Sleep(1000);

	// 위치센서 감지 확인(AC 대기 위치)
	if (!CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsPositionOnAC(1) == true)
		return false;

	// Status Busy Off
	CSequenceManager::GetInstance()->GetFunctionModuleEQState()->SetStatusbusyOff();

    // AC Z AC 대기 위치 이동
    nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_Z9, 0, false);
    if(nRet != VTM_RV_SUCCESSED)
    {
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(WARN, _T("CFunctionModuleProbeACCStep_SACCToACReadyPosition AC Z Move Error."));
        return false;
    }
     
    return true;
}

bool CFunctionModuleProbeACCStep_SetProbeUnloadClear::Run()
{
    bool bRet = false;

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleProbeACCStep_SetProbeUnloadClear Start."));

    int iProbe = CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->GetProbe();
    if (iProbe < df_PROBE_AGING1)
    {
        return false;
    }

    bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->SetEQClearPrboeData(iProbe);

    return bRet;
}

bool CFunctionModuleProbeACCStep_SetProbeloadData::Run()
{
    bool bRet = false;

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleProbeACCStep_SetProbeloadData Start."));

    int iProbe = CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->GetProbe();
    if (iProbe < df_PROBE_AGING1)
    {
        return false;
    }

    bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->SetEQGetPrboeData(iProbe - df_PROBE_AGING1);

    return bRet;
}

bool CFunctionModuleProbeACCStep_Mount::Run()
{
	bool bRet = true;

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleProbeACCStep_Mount Start."));

	int iProbe = CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->GetProbe();
	if (iProbe < df_PROBE_AGING1)
	{
		return false;
	}

	int i = 0;
	i = iProbe - df_PROBE_AGING1;

	_tcscpy_s(m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->stProbeInfo[i].m_szProbeCardIDTime, _countof(m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->stProbeInfo[i].m_szProbeCardIDTime), gf_GetCurTimeString(5));

	if (CSACCMgr::GetInstance() != nullptr)
	{
		CString strProbeID, strProbeMountTime;
		strProbeID.Format(_T("%s"), m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_szPROBEID[i]);
		strProbeMountTime.Format(_T("%s"), m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->stProbeInfo[i].m_szProbeCardIDTime);
		CSACCMgr::GetInstance()->Probe_Mount(i, strProbeID, strProbeMountTime);
	}
		

	return bRet;
}

bool CFunctionModuleProbeACCStep_UnMount::Run()
{
	bool bRet = true;

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleProbeACCStep_UnMount Start."));

	int iProbe = CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->GetProbe();
	if (iProbe < df_PROBE_AGING1)
	{
		return false;
	}

	int i = 0;
	i = iProbe - df_PROBE_AGING1;
	
	if (CSACCMgr::GetInstance() != nullptr)
	{
		CString strProbeID, strProbeMountTime;
		strProbeID.Format(_T("%s"), m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_szPROBEID[i]);
		strProbeMountTime.Format(_T("%s"), m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->stProbeInfo[i].m_szProbeCardIDTime);
		CSACCMgr::GetInstance()->Probe_Unmount(i, strProbeID, strProbeMountTime, gf_MakeCurTimeString());
	}

	_tcscpy_s(m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->stProbeInfo[i].m_szProbeCardIDTime, _countof(m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->stProbeInfo[i].m_szProbeCardIDTime), _T(""));

	m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->stProbeInfo[i].m_nUseCnt = 0;	// jsyoom 2022.08.16 add

	// jsyoon 2022.09.19 add
	// Probe Unit to AC
	_tcscpy_s(m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_szACPROBEID, _countof(m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_szACPROBEID), m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_szPROBEID[i]);

	return bRet;
}


bool CFunctionModuleProbeACCStep_Safety::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleProbeACCStep_Safety Start."));

	bool bRet = false;
	int nRet = VTM_RV_SUCCESSED;
	if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsSafetySensorOn() != true)
	{
		CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->SetEQStatusChangeStep(enumEQSTATUS_SAFETY);
		CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Start();

		Sleep(500);

		int iLoop = 0;
		nRet = JOB_PROC_SAFETY_POS_ERROR;
		do
		{
			Sleep(100);

			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Is_Complete();
			if (bRet == true)
			{
				nRet = VTM_RV_SUCCESSED;
				break;
			}

			if (CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Is_Fail() == true)
			{
				nRet = CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->GetFailNum();
				m_pShareItem->SetFail(true, nRet);
				bRet = false;
				break;
			}

			iLoop++;

		} while (iLoop < 1000);
	}
	else
	{
		bRet = true;
	}

	return bRet;
}

// jsyoon 2022.08.10 add
bool CFunctionModuleProbeACCStep_WaitForMoveACZDown::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleProbeACCStep_WaitForMoveACZDown Start."));

	bool bRet = false;
	int nRet = VTM_RV_SUCCESSED;
	nRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_MotionDone(AXIS_Z9);
	if (nRet != VTM_RV_SUCCESSED)
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(WARN, _T("CFunctionModuleProbeACCStep_WaitForMoveACZDown AC Z Move Error."));
		return false;
	}

	Sleep(1000);

	// AC Z Down 센서 감지 확인
	if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsACZxisUp() == false &&
		CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsACZxisDown() == true )
	{
		bRet = true;
	}
	else
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(WARN, _T("CFunctionModuleProbeACCStep_WaitForMoveACZDown AC Z Down Sensor Error."));
		bRet = false;
	}

	return bRet;
}

bool CFunctionModuleProbeACCStep_ProbeChangeMove::Run()
{
	// PROBE 교체 위치 이동
	int nRet = VTM_RV_SUCCESSED;

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleProbeACCStep_ProbeChangeMove Start."));

	// Probe 확인 
	int iProbe = CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->GetProbe();
	if (iProbe < df_PROBE_AGING1)
	{
		return false;
	}

	// Probe X, Y축 이동
	double dChangeProbePos = 0;
	m_pShareItem->GetMaintControlMgrPtr()->GetPositionConfigMgr()->GetProbeChangePos(&dChangeProbePos);

	// Probe 교체 위치 이동
	nRet = m_pShareItem->GetMaintControlMgrPtr()->Move_SelectProbe(iProbe, dChangeProbePos, false);

	if (nRet != VTM_RV_SUCCESSED)
	{
		return false;
	}

	return true;
}
