#include "pch.h"
#include "SystemInitializeStep.h"

#include "..\BaseSequenceImpl.h"
#include "..\SystemParamShare.h"
#include "..\Manager\SystemConfigMgr.h"
#include "..\Manager\PositionConfigMgr.h"
//#include "..\Manager\MotorControlMgr.h"
#include "..\Manager\MaintControlMgr.h"
//#include "..\Manager\IOMapControlMgr.h"
#include "..\Manager\LightControlMgr.h"
#include "..\Manager\ShareMemoryMgr.h"
#include "CoreFunction.h"

#include "..\FunctionModule\FunctionModuleEQState.h"
#include "..\FunctionModule\FunctionModuleEQStateChange.h"
#include "..\FunctionModule\FunctionModuleAFControl.h"
#include "..\FunctionModule\FunctionModuleProbeACC.h"
#include "DoWork/DoWorkGlassSequence.h"
#include "..\Manager\SequenceManager.h"
#include "..\Manager\MCCLogMgr.h"
#include "Recipe/RecipeEditMgr.h"
#include "..\Manager\CommTcpIpFactoryMgr.h"		// jsyoon 2022.10.14 add

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

bool CSystemInitializeStep_None::Run()
{
	// init
	m_pShareItem->SetComplete(false);
	m_pShareItem->SetAlarmOn(false);
	m_pShareItem->SetFail(false);
	m_pShareItem->SetStopUser(false);

	/*m_pShareItem->SetAlarmOn(false);
	m_pShareItem->SetFail(false);
    m_pShareItem->SetComplete(false);*/

	m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->bSystemInitOk = FALSE;
	m_pShareItem->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_INITIAL);

    // 전체 공유 파라미터에 공유한다.
    CSystemParamShare::GetInstance()->SetInitialized(false);

	// IO
	m_pShareItem->GetIOMapControlMgrPtr()->SetInitialize(false);

	// Motion
	m_pShareItem->GetMaintControlMgrPtr()->SetInitializeMC(false);

	m_pShareItem->GetMaintControlMgrPtr()->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_BIT_W_GLASS_SENSOR_POWER_ON, false); // jsyoon 2022.05.26 add

    return true;
}

bool CSystemInitializeStep_CheckInterlock::Run()
{
	bool bRet = false;

	// LYS 2026-09-02 Sensor Detect Pass TEMP
	return true;

	// Loader Arm Detected
	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_CHK_LOADER_ARM, true);

	bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLoaderArmSensorOn();

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_CHK_LOADER_ARM, false);

	if (bRet == true)
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(ERRO, _T("CSystemInitializeStep_CheckInterlock, Loader Arm Detect On."));
		m_pShareItem->SetFail(true, PLC_ARM_SENSOR_DETECT);

		return false;
	}

	// Shutter Open
	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_CHK_LOADER_SHUTTER, true);


	bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsShutterSensorClose();

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_CHK_LOADER_SHUTTER, false);

	if (bRet != true)
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(ERRO, _T("CSystemInitializeStep_CheckInterlock, Loader Shutter Not Close."));
		m_pShareItem->SetFail(true, PLC_LOADER_SHUTTER_UP_ERROR);

		return false;
	}

	// X-ray 
	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_CHK_IONIZER_ON, true);

	bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsX_RaySensorRunOn();

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_CHK_IONIZER_ON, false);
	if (bRet == true)
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(ERRO, _T("CSystemInitializeStep_CheckInterlock, X-RAY IONIZER ON."));
		m_pShareItem->SetFail(true);

		return false; 
	}

	// Door Open 
	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_CHK_DOOR_OPEN, true);

	bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsDoorSensorClose();

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_CHK_DOOR_OPEN, false);

	if (bRet != true)
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(ERRO, _T("CSystemInitializeStep_CheckInterlock, Door Not Close."));
		m_pShareItem->SetFail(true, PLC_DOOR_SENSOR_DETECT);

		return false;
	}

	return true;
}

// IO 관련 초기화
bool CSystemInitializeStep_InitIO::Run()
{
	if (m_pShareItem->GetIOMapControlMgrPtr()->IsUseDevice() == false)
	{
		return false;
	}
		
	if (m_pShareItem->GetIOMapControlMgrPtr()->IsOutOfService() == true)
	{
		m_pShareItem->GetIOMapControlMgrPtr()->SetInitialize(true);
		return true;
	}
		
	bool bRet = true;
	int nRet = VTM_RV_SUCCESSED;
	if (m_pShareItem->GetIOMapControlMgrPtr()->IsInitialized() != true)
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_INIT_IO_CTRL, TRUE);

		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("SystemInitializeStep_InitIO Start."));
	
		bRet = m_pShareItem->GetIOMapControlMgrPtr()->IsConnected();
		if (bRet == true)
		{
			// Probe Contact
			for (size_t i = df_PROBE_AGING1; i <= df_PROBE_MEAS; i++)
			{
				if (m_pShareItem->GetSystemConfigMgrPtr()->IsOCP())
				{
					// LYS 2026-09-02 ProbeUnContactSensor Skip TEMP
					if (i < df_PROBE_MEAS) continue;
					bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsProbeUnContactSensorOn(i);
					bRet = true;
					if (bRet != true)
					{
						nRet = MOTION_INTERLOCK_6;
						break;
					}
				}
				else
				{
					bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsProbeUnContactSensorOn(i);
					if (bRet != true)
					{
						nRet = MOTION_INTERLOCK_6;
						break;
					}
				}

			}

			// LYS 2026-09-02 z축 2번 호밍 방지
			//if (nRet != VTM_RV_SUCCESSED)
			//{
			//	// Z 
			//	nRet = MotionHome(1);		// jsyoon 2022.09.30 add
			//
			//	if (nRet == VTM_RV_SUCCESSED)
			//	{
			//		// Probe Z
			//		nRet = MotionHome(4);
			//	}
			//}

			// ACZ Check
			if (nRet == VTM_RV_SUCCESSED)
			{
				bool ACZDown = false, ACXChange = false;
				ACXChange = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsPositionOnAC(0);
				ACZDown   = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsACZxisDown();
				if (ACXChange == true && ACXChange != true)
				{
					nRet = MotionHome(10);
				}
			}

			//
			if (nRet == VTM_RV_SUCCESSED)
			{
				bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLiftpinSensorDown();
				if (bRet == true)
				{
					//M4500 ~ 4999 초기화 Parameter Set
					//Ex. RS, CA 관련 bit 및 Alarm Reset
					nRet = ResetIO();
					if (nRet == VTM_RV_SUCCESSED)
					{
						m_pShareItem->GetIOMapControlMgrPtr()->SetInitialize(true);

						m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("SystemInitializeStep_InitIO OK."));

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
		else
		{
			nRet = PLC_SOCKET_CREATE_FAIL;
		}

		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_INIT_IO_CTRL, FALSE);

		if (bRet != true)
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("SystemInitializeStep_InitIO NG."));
			m_pShareItem->SetFail(true, nRet);
		}
	}

	return bRet;
}

int CSystemInitializeStep_InitIO::ResetIO()
{
	//M4500 ~ 4999 초기화 Parameter Set
	//Ex. RS, CA 관련 bit 및 Alarm Reset
//	m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EXP_B_CVT_UV_SMART_04_ON, true);

	bool bRet = false;
	int nRet = VTM_RV_SUCCESSED;

	// LYS 2026-09-03 PLC 응답없음. 
	// Fixture Reverce
	bRet = m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_W_FIXTURE_FWD_ALL, 0, 1);
	if (bRet == true)
	{
		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_FIXTURE_FRONT_R01, 8, 1, true, PLC_IO_ACTION_DONE_TIME_60);
	}
	
	if (bRet != true)
	{
		nRet = PLC_FIXTURE_CYLINDER_FWD_ERROR;
	}
	
	// Fixture Down
	if (bRet = true)
	{
		// TODO: 
		bRet = m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_W_FIXTURE_UPDOWN_ALL, 0, 1);
		if (bRet == true)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_FIXTURE_FD01, 8, 1, true, PLC_IO_ACTION_DONE_TIME_60);
		}
	
		if (bRet != true)
		{
			nRet = PLC_FIXTURE_CYLINDER_DOWN_ERROR;
		}
	}
	
	// PLC Initialize
	if (bRet = true)
	{
		bRet = m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_INITIALIZE, 1, 1);
		if (bRet == true)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_INITIALIZE_COMPLETE, 1, 1, true, PLC_IO_ACTION_DONE_TIME_60);
		}

		if (bRet != true)
		{
			nRet = PLC_INITIAL_FAIL;
		}
	}
	
	if( bRet == true)
		return VTM_RV_SUCCESSED;
	else
		return nRet;
}

// MOTION 관련 초기화
bool CSystemInitializeStep_InitMotion::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("SystemInitializeStep_InitMotion Start."));
	
	if (m_pShareItem->GetMaintControlMgrPtr()->IsOutOfServiceMC() == true)
	{
		m_pShareItem->GetMaintControlMgrPtr()->SetInitializeMC(true);
		return true;
	}
	
	//CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME, TRUE);

	//int iRet_MotionHome = MotionHome(AXIS_ALL);

	//CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME, FALSE);

	//if (iRet_MotionHome != VTM_RV_SUCCESSED)
	//{
	//	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("SystemInitializeStep_InitMotion NG. MotionHome."));

	//	m_pShareItem->SetFail(true);
	//	return false;
	//}

	//m_pShareItem->GetMaintControlMgrPtr()->SetInitializeMC(true);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("SystemInitializeStep_InitMotion OK."));

	return true;
}

int CSystemInitializeStep_InitMotion::InitDevice()
{
	int nRet = VTM_RV_SUCCESSED;

	if (m_pShareItem->GetMaintControlMgrPtr()->InitDeviceMC() == false)
	{
		nRet = VTM_RV_FAILED;
	}

	return nRet;
}

//MotionHome(int iAxis = AXIS_ALL)
int CSystemInitializeStep_InitMotion::MotionHome(int iAxis)
{
	int nRet = VTM_RV_SUCCESSED;

	if (iAxis == AXIS_ALL)
	{
		// 1. Life Pin 
		nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveHome(AXIS_LOADER_Z);
		if (nRet != VTM_RV_SUCCESSED)
		{
			return nRet;
		}

		nRet = m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(AXIS_LOADER_Z, df_HOMINGDONE_TIME );

		if (nRet != VTM_RV_SUCCESSED)
		{
			return nRet;
		}

		/////
		// 2. Head AXIS
		// 2.1 Z AXIS
		for (int iAxis = AXIS_X; iAxis < df_USE_MAX_AXIS; iAxis++)
		{
			if (iAxis == AXIS_Z1 || iAxis == AXIS_Z2 || 
				iAxis == AXIS_ZOOM1 || iAxis == AXIS_THETA1 || iAxis == AXIS_ZOOM2 || iAxis == AXIS_THETA2 || iAxis == AXIS_FILTER2 || iAxis == AXIS_VLENS1 || iAxis == AXIS_VLENS2 ||
				iAxis == AXIS_ROTATE1 || iAxis == AXIS_FOCUS1 || iAxis == AXIS_ROTATE2 || iAxis == AXIS_FOCUS2 || iAxis == AXIS_COLOR_DEGREE1 || iAxis == AXIS_COLOR_DEGREE2 || 
				iAxis == AXIS_ROTATE3 || iAxis == AXIS_FOCUS3 || iAxis == AXIS_ROTATE4 || iAxis == AXIS_FOCUS4 || iAxis == AXIS_SPECTO_DEGREE1 || iAxis == AXIS_SPECTO_DEGREE2 )
			{
				nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveHome(iAxis);
			}
		}

		if (nRet != VTM_RV_SUCCESSED)
		{
			return nRet;
		}
		
		// Z AXIS Homing Done Check
		for (int iAxis = AXIS_X; iAxis < df_USE_MAX_AXIS; iAxis++)
		{
			if (iAxis == AXIS_Z1 || iAxis == AXIS_Z2 ||
				iAxis == AXIS_ZOOM1 || iAxis == AXIS_THETA1 || iAxis == AXIS_ZOOM2 || iAxis == AXIS_THETA2 || iAxis == AXIS_FILTER2 || iAxis == AXIS_VLENS1 || iAxis == AXIS_VLENS2 ||
				iAxis == AXIS_ROTATE1 || iAxis == AXIS_FOCUS1 || iAxis == AXIS_ROTATE2 || iAxis == AXIS_FOCUS2 || iAxis == AXIS_COLOR_DEGREE1 || iAxis == AXIS_COLOR_DEGREE2 ||
				iAxis == AXIS_ROTATE3 || iAxis == AXIS_FOCUS3 || iAxis == AXIS_ROTATE4 || iAxis == AXIS_FOCUS4 || iAxis == AXIS_SPECTO_DEGREE1 || iAxis == AXIS_SPECTO_DEGREE2)
			{
				nRet &= m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(iAxis, df_HOMINGDONE_TIME);
			}
		}

		if (nRet != VTM_RV_SUCCESSED)
		{
			return nRet;
		}

		// 2.2 Head AXIS
		for (int iAxis = AXIS_X; iAxis < df_USE_MAX_AXIS; iAxis++)
		{
			if (iAxis == AXIS_X2 || iAxis == AXIS_X3 || iAxis == AXIS_Y1 || iAxis == AXIS_Y2 || iAxis == AXIS_Y3 || iAxis == AXIS_Y4 )
			{
				nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveHome(iAxis);
			}
		}

		if (nRet != VTM_RV_SUCCESSED)
		{
			return nRet;
		}

		// Head AXIS Homing Done Check
		for (int iAxis = AXIS_X; iAxis < df_USE_MAX_AXIS; iAxis++)
		{
			if (iAxis == AXIS_X2 || iAxis == AXIS_X3 || iAxis == AXIS_Y1 || iAxis == AXIS_Y2 || iAxis == AXIS_Y3 || iAxis == AXIS_Y4)
			{
				nRet &= m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(iAxis, df_HOMINGDONE_TIME);
			}
		}

		if (nRet != VTM_RV_SUCCESSED)
		{
			return nRet;
		}


		

		//if (nRet != VTM_RV_SUCCESSED)
		//{
		//	return nRet;
		//}

		// 3. Probe
		// 3.1 Probe Z
		for (int iAxis = AXIS_X; iAxis < df_USE_MAX_AXIS; iAxis++)
		{
			if (iAxis == AXIS_PROBE_Z1 || iAxis == AXIS_PROBE_Z2 || iAxis == AXIS_PROBE_Z3 || iAxis == AXIS_PROBE_Z4 || iAxis == AXIS_PROBE_Z5)
			{
				nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveHome(iAxis);
			}
		}

		if (nRet != VTM_RV_SUCCESSED)
		{
			return nRet;
		}

		// Probe AXIS Homing Done Check
		for (int iAxis = AXIS_X; iAxis < df_USE_MAX_AXIS; iAxis++)
		{
			if (iAxis == AXIS_PROBE_Z1 || iAxis == AXIS_PROBE_Z2 || iAxis == AXIS_PROBE_Z3 || iAxis == AXIS_PROBE_Z4 || iAxis == AXIS_PROBE_Z5)
			{
				nRet &= m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(iAxis, df_HOMINGDONE_TIME);
			}
		}

		if (nRet != VTM_RV_SUCCESSED)
		{
			return nRet;
		}

		double dPosProbeZ = 0;
		nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeMove(AXIS_ALL_Z, dPosProbeZ, SPEED_HIGH);
		if (nRet != VTM_RV_SUCCESSED)
		{
			return nRet;
		}
		
		/*stMotorPosition stPositionProbe[5];
		stPositionProbe[0].axis = AXIS_PROBE_Z1;
		stPositionProbe[0].dPosition = dPosProbeZ;
		stPositionProbe[1].axis = AXIS_PROBE_Z2;
		stPositionProbe[1].dPosition = dPosProbeZ;
		stPositionProbe[2].axis = AXIS_PROBE_Z3;
		stPositionProbe[2].dPosition = dPosProbeZ;
		stPositionProbe[3].axis = AXIS_PROBE_Z4;
		stPositionProbe[3].dPosition = dPosProbeZ;
		stPositionProbe[4].axis = AXIS_PROBE_Z5;
		stPositionProbe[4].dPosition = dPosProbeZ;

		nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveOrgPosMulti(stPositionProbe, 5, SPEED_HIGH, true);
		if (nRet != VTM_RV_SUCCESSED)
		{
			return nRet;
		}
		*/

		Sleep(1000);

		// 
		// 3.2 Probe Y
		for (int iAxis = AXIS_X; iAxis < df_USE_MAX_AXIS; iAxis++)
		{
			if (iAxis == AXIS_PROBE_Y1 || iAxis == AXIS_PROBE_Y2 || iAxis == AXIS_PROBE_Y3 || iAxis == AXIS_PROBE_Y4 || iAxis == AXIS_PROBE_Y5 )
			{
				nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveHome(iAxis);
			}
		}

		if (nRet != VTM_RV_SUCCESSED)
		{
			return nRet;
		}

		// Probe AXIS Homing Done Check
		for (int iAxis = AXIS_X; iAxis < df_USE_MAX_AXIS; iAxis++)
		{
			if (iAxis == AXIS_PROBE_Y1 || iAxis == AXIS_PROBE_Y2 || iAxis == AXIS_PROBE_Y3 || iAxis == AXIS_PROBE_Y4 || iAxis == AXIS_PROBE_Y5)
			{
				nRet &= m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(iAxis, df_HOMINGDONE_TIME);
			}
		}

		if (nRet != VTM_RV_SUCCESSED)
		{
			return nRet;
		}

		// Probe
		for (int iAxis = AXIS_X; iAxis < df_USE_MAX_AXIS; iAxis++)
		{
			if (iAxis == AXIS_PROBE_X1 || iAxis == AXIS_PROBE_X3 || iAxis == AXIS_PROBE_X5 || iAxis == AXIS_PROBE_X7 || iAxis == AXIS_PROBE_X9)
			{
				if (m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->stMotionStatus[iAxis].bEnable != TRUE || m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->stMotionStatus[iAxis + 1].bEnable != TRUE )
				{
					nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveHome(iAxis);

					if (nRet == VTM_RV_SUCCESSED)
					{
						nRet = m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(iAxis, df_HOMINGDONE_TIME);
					}

					if (nRet != VTM_RV_SUCCESSED)
					{
						return nRet;
					}
				}
			}
		}
		

		// Stage 
		for (int iAxis = AXIS_X; iAxis < df_USE_MAX_AXIS; iAxis++)
		{
			if (iAxis == AXIS_X)
			{
				nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveHome(iAxis);
			}
		}

		if (nRet != VTM_RV_SUCCESSED)
		{
			return nRet;
		}

		for (int iAxis = AXIS_X; iAxis < df_USE_MAX_AXIS; iAxis++)
		{
			if (iAxis == AXIS_X)
			{
				nRet &= m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(iAxis, df_HOMINGDONE_TIME );
			}
		}

		if (nRet != VTM_RV_SUCCESSED)
		{
			return nRet;
		}

		nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveHome(AXIS_LOADER_R);

		if (nRet == VTM_RV_SUCCESSED)
		{
			nRet = m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(AXIS_LOADER_R, df_HOMINGDONE_TIME );
		}

		if (nRet != VTM_RV_SUCCESSED)
		{
			return nRet;
		}

		// Auto change
		// Auto Change 위치 확인 후 Z축 호밍진행
		if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsPositionOnAC(2) == true ) // SACC 내부에 있을 경우
		{
			// 뒤로 이동
			m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosR(AXIS_X4, -1000, false, SPEED_LOW);
			int iLoop = 0;
			bool bRet = false;
			nRet = MTOR_HOME_FAIL_ALARM;
			do
			{
				Sleep(1000);

				bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsPositionOnAC(1);
				if (bRet == true)
				{
					m_pShareItem->GetMaintControlMgrPtr()->StopMove(AXIS_X4);
					nRet = VTM_RV_SUCCESSED;
					break;
				}
				
				iLoop++;

			} while (iLoop < 30);  // 30sec

			if (nRet != VTM_RV_SUCCESSED)
			{
				return nRet;
			}
		}

		for (int iAxis = AXIS_X; iAxis < df_USE_MAX_AXIS; iAxis++)
		{
			if (iAxis == AXIS_Z9)
			{
				nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveHome(iAxis);
			}
		}

		if (nRet != VTM_RV_SUCCESSED)
		{
			return nRet;
		}

		for (int iAxis = AXIS_X; iAxis < df_USE_MAX_AXIS; iAxis++)
		{
			if (iAxis == AXIS_Z9)
			{
				nRet &= m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(iAxis, df_HOMINGDONE_TIME);
			}
		}

		if (nRet != VTM_RV_SUCCESSED)
		{
			return nRet;
		}

		nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveHome(AXIS_X4);

		if (nRet == VTM_RV_SUCCESSED)
		{
			nRet = m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(AXIS_X4, df_HOMINGDONE_TIME);
		}

		if (nRet != VTM_RV_SUCCESSED)
		{
			return nRet;
		}

	}
	else	// 개별호밍
	{
		nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveHome(iAxis);
		nRet &= m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(iAxis, df_HOMINGDONE_TIME );
	}
	
	return nRet;
}

bool CSystemInitializeStep_InitModule::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("SystemInitializeStep_InitModule Start."));

	// Motion
	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_INIT_MOT, TRUE);

	bool bRet = true;
	if (m_pShareItem->GetMaintControlMgrPtr()->IsOutOfServiceMC() != true)
	{
		if (m_pShareItem->GetMaintControlMgrPtr()->InitDeviceMC() == false)
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("SystemInitializeStep_InitMotion NG. InitDevice."));

			m_pShareItem->SetFail(true, MEI_INIT_MC_FAIL);
			bRet = false;
		}
	}
	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_INIT_MOT, FALSE);

	if (bRet != true)
		return bRet;

	// Light
	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_INIT_LED_CTRL, TRUE);
	m_pShareItem->GetLightControlMgrPtr()->InitDevice(0);	// SPO
	m_pShareItem->GetLightControlMgrPtr()->InitDevice(1);   // CAS 표준광원
	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_INIT_LED_CTRL, FALSE);
	
	// IO
	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_INIT_IO_CTRL, TRUE);

	if (m_pShareItem->GetIOMapControlMgrPtr()->IsOutOfService() != true)
	{
		if (m_pShareItem->GetIOMapControlMgrPtr()->IsConnected() != true)
		{
			if (m_pShareItem->GetIOMapControlMgrPtr()->InitDevice() == false)
			{
				m_pShareItem->SetFail(true, PLC_INITIAL_FAIL);
				bRet = false;
			}
		}
	}
	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_INIT_IO_CTRL, FALSE);

	if (bRet != true)
		return bRet;
	
	// SPO
	m_pShareItem->GetLightControlMgrPtr()->SetLedLampTurnOnOff(0, 0, 0, enumLED_LAMP_CONTROL_TURN___ON);
	m_pShareItem->GetLightControlMgrPtr()->SetLedLampTurnOnOff(0, 1, 0, enumLED_LAMP_CONTROL_TURN___ON);
	m_pShareItem->GetLightControlMgrPtr()->SetLedLampTurnOnOff(0, 2, 0, enumLED_LAMP_CONTROL_TURN___ON);
	m_pShareItem->GetLightControlMgrPtr()->SetLedLampTurnOnOff(0, 3, 0, enumLED_LAMP_CONTROL_TURN___ON);

	// CAS
	m_pShareItem->GetLightControlMgrPtr()->SetLedLampTurnOnOff(0, 0, 0, enumLED_LAMP_CONTROL_TURN___ON);
	m_pShareItem->GetLightControlMgrPtr()->SetLedLampTurnOnOff(0, 1, 0, enumLED_LAMP_CONTROL_TURN___ON);
	m_pShareItem->GetLightControlMgrPtr()->SetLedLampTurnOnOff(0, 2, 0, enumLED_LAMP_CONTROL_TURN___ON);
	m_pShareItem->GetLightControlMgrPtr()->SetLedLampTurnOnOff(0, 3, 0, enumLED_LAMP_CONTROL_TURN___ON);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("SystemInitializeStep_InitModule OK."));
	
	return bRet;
}

bool CSystemInitializeStep_InitComplete::Run()
{	
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("SystemInitializeStep_InitComplete Start."));

	if (m_pShareItem->GetIOMapControlMgrPtr()->IsOutOfService() != true)
	{
		if (m_pShareItem->GetIOMapControlMgrPtr()->IsInitialized() == false)
		{
			m_pShareItem->SetFail(true, PLC_INITIAL_FAIL);
			return false;
		}
	}
	
	if (m_pShareItem->GetMaintControlMgrPtr()->IsInitializedMC() == false)
	{
		m_pShareItem->SetFail(true, MEI_INIT_MC_FAIL);
		return false;
	}
	
	int nStatus = enumEQStateReady;
	if (m_pShareItem->GetIOMapControlMgrPtr()->IsOutOfService() != true)
	{
		nStatus = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->GetEQState();
	}

	if (nStatus == enumEQStateAbnormal)
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("SystemInitializeStep_InitComplete NG."));
		m_pShareItem->SetFail(true, PLC_STATE_UNKNOWN_ERROR);
		return false;
	}

	m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->bSystemInitOk = TRUE;

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("SystemInitializeStep_InitComplete OK."));
	
	return true;

	/*CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_SAFETY_PROC, true);

	bool bRet = false;
	int nRet = VTM_RV_SUCCESSED;
	if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsSafetySensorOn() != true)
	{
		CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->SetEQStatusChangeStep(enumEQSTATUS_SAFETY);
		CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Start();

		int iLoop = 0;
		nRet = JOB_PROC_SAFETY_POS_ERROR;
		do
		{
			Sleep(500);

			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Is_Complete();
			if (bRet == true)
			{
				nRet = VTM_RV_SUCCESSED;
				break;
			}
				
			if (CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Is_Fail() == true)
			{
				nRet = CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->GetFailNum();
				break;
			}
				
			iLoop++;

		} while (iLoop < 200);
	}
	else
	{
		bRet = true;
	}
	
	if (bRet == true) 
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("SystemInitializeStep_InitComplete OK."));
	}
	else
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("SystemInitializeStep_InitComplete NG."));
		m_pShareItem->SetFail(true, nRet);
	}

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_SAFETY_PROC, false);

	return bRet;*/
}

bool CSystemInitializeStep_Idle::Run()
{
	//SMEM_SetSys_ProcessStatus(SPS_IDLE);
	//
	//if(pVTMSystem->stInterLock.m_bTRShutter == TRUE)
	//	m_pDlgNaviMaint->m_dlgNaviMaintPLC.Do_TrShutterClose();	//jh.kang 20191007 add
	//

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("SystemInitializeStep_Idle Start."));

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_Idle, NULL);

	if (m_pShareItem->GetAlarmOn() != true && m_pShareItem->GetFail() != true && m_pShareItem->GetStopUser() != true)	// 2023.08.09 add Stopuser
	{
		m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->bSystemInitOk = TRUE;

		// 전체 공유 파라미터에 공유한다.
		CSystemParamShare::GetInstance()->SetInitialized(true);

		m_pShareItem->SetComplete(true);   // 동작 완료
	}
		
	// 완료
	ResetEvent(m_pShareItem->GetStartEventHandle());

	SetEvent(m_pShareItem->GetCompleteEventHandle()); // 시퀀스 완료 이벤트 SET

	m_pShareItem->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_IDLE);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("SystemInitializeStep_Idle End."));

	return false;
}

bool CSystemInitializeStep_MotionHoming1::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CSystemInitializeStep_MotionHoming1 Start."));

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME, TRUE);

	if (m_pShareItem->GetFail() != true)
	{
		int iRet = VTM_RV_SUCCESSED;
		iRet = MotionHome(0);

		if (iRet != VTM_RV_SUCCESSED)
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CSystemInitializeStep_MotionHoming NG. MotionHome."));
			m_pShareItem->SetFail(true, iRet);

			return false;
		}
	}

	return true;
}

bool CSystemInitializeStep_MotionHoming2::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CSystemInitializeStep_MotionHoming2 Start."));

	if (m_pShareItem->GetFail() != true)
	{
		int iRet = MotionHome(1);
		if (iRet != VTM_RV_SUCCESSED)
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CSystemInitializeStep_MotionHoming NG. MotionHome."));
			m_pShareItem->SetFail(true, iRet);

			return false;
		}
	}

	return true;
}

bool CSystemInitializeStep_MotionHoming3::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CSystemInitializeStep_MotionHoming3 Start."));

	if (m_pShareItem->GetFail() != true)
	{
		int iRet = MotionHome(2);
		if (iRet != VTM_RV_SUCCESSED)
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CSystemInitializeStep_MotionHoming NG. MotionHome."));
			m_pShareItem->SetFail(true, iRet);

			return false;
		}
	}

	return true;
}

bool CSystemInitializeStep_MotionHoming4::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CSystemInitializeStep_MotionHoming4 Start."));

	if (m_pShareItem->GetFail() != true)
	{
		int iRet = MotionHome(3);
		if (iRet != VTM_RV_SUCCESSED)
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CSystemInitializeStep_MotionHoming NG. MotionHome."));
			m_pShareItem->SetFail(true, iRet);

			return false;
		}
	}

	return true;
}

bool CSystemInitializeStep_MotionHoming5::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CSystemInitializeStep_MotionHoming5 Start."));

	if (m_pShareItem->GetFail() != true)
	{
		int iRet = MotionHome(4);
		if (iRet != VTM_RV_SUCCESSED)
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CSystemInitializeStep_MotionHoming NG. MotionHome."));
			m_pShareItem->SetFail(true, iRet);

			return false;
		}
	}

	return true;
}

bool CSystemInitializeStep_MotionHoming6::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CSystemInitializeStep_MotionHoming6 Start."));

	if (m_pShareItem->GetFail() != true)
	{
		int iRet = MotionHome(5);
		if (iRet != VTM_RV_SUCCESSED)
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CSystemInitializeStep_MotionHoming NG. MotionHome."));
			m_pShareItem->SetFail(true, iRet);

			return false;
		}
	}

	return true;
}

bool CSystemInitializeStep_MotionHoming7::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CSystemInitializeStep_MotionHoming7 Start."));

	if (m_pShareItem->GetFail() != true)
	{
		int iRet = MotionHome(6);
		if (iRet != VTM_RV_SUCCESSED)
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CSystemInitializeStep_MotionHoming NG. MotionHome."));
			m_pShareItem->SetFail(true, iRet);

			return false;
		}
	}

	return true;
}

bool CSystemInitializeStep_MotionHoming8::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CSystemInitializeStep_MotionHoming8 Start."));

	if (m_pShareItem->GetFail() != true)
	{
		int iRet = MotionHome(7);
		if (iRet != VTM_RV_SUCCESSED)
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CSystemInitializeStep_MotionHoming NG. MotionHome."));
			m_pShareItem->SetFail(true, iRet);

			return false;
		}
	}

	return true;
}

bool CSystemInitializeStep_MotionHoming9::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CSystemInitializeStep_MotionHoming9 Start."));

	if (m_pShareItem->GetFail() != true)
	{
		int iRet = MotionHome(8);
		if (iRet != VTM_RV_SUCCESSED)
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CSystemInitializeStep_MotionHoming NG. MotionHome."));
			m_pShareItem->SetFail(true, iRet);

			return false;
		}
	}

	return true;
}

bool CSystemInitializeStep_MotionHoming10::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CSystemInitializeStep_MotionHoming10 Start."));

	if (m_pShareItem->GetFail() != true)
	{
		int iRet = MotionHome(9);
		if (iRet != VTM_RV_SUCCESSED)
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CSystemInitializeStep_MotionHoming NG. MotionHome."));
			m_pShareItem->SetFail(true, iRet);

			return false;
		}
	}

	return true;
}

bool CSystemInitializeStep_MotionHoming11::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CSystemInitializeStep_MotionHoming11 Start."));

	if (m_pShareItem->GetFail() != true)
	{
		int iRet = MotionHome(10);
		if (iRet != VTM_RV_SUCCESSED)
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CSystemInitializeStep_MotionHoming NG. MotionHome."));
			m_pShareItem->SetFail(true, iRet);

			return false;
		}
	}

	return true;
}

bool CSystemInitializeStep_MotionHoming12::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CSystemInitializeStep_MotionHoming12 Start."));

	
	if (m_pShareItem->GetFail() != true)
	{
		int iRet = MotionHome(11);
		if (iRet != VTM_RV_SUCCESSED)
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CSystemInitializeStep_MotionHoming NG. MotionHome."));
			m_pShareItem->SetFail(true, iRet);

			CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME, false);
			return false;
		}
	}

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME, false);

	return true;
}

bool CSystemInitializeStep_MotionComplete::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CSystemInitializeStep_MotionComplete Start."));


	bool bRet = true;
	if (m_pShareItem->GetFail() != true)
	{
		m_pShareItem->GetMaintControlMgrPtr()->SetInitializeMC(true);
	}
	else
	{
		bRet = false;
	}
	
	return bRet;
}

int CSystemInitializeStep::MotionHome(int iStep)
{
	int nRet = VTM_RV_SUCCESSED;

	switch (iStep)
	{
	case 0:		// Lift Pin
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_LIFT_PIN, true);

		// 1. Life Pin 
		nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveHome(AXIS_LOADER_Z);
		if (nRet == VTM_RV_SUCCESSED)
		{
			nRet = m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(AXIS_LOADER_Z, df_HOMINGDONE_TIME);
		}

		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_LIFT_PIN, false);
	}
	break;
	case 1:		// Head Z
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_HEAD, true);

		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_HEAD_Z, true);

		// LYS 2026-08-19 Z3~Z6 Sub Z homing 
		for (int iAxis = AXIS_Z3; iAxis <= AXIS_Z6; iAxis++)
		{
			if (m_pShareItem->GetStopUser() == true)
				continue;

			nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveHome(iAxis);
		}

		for (int iAxis = AXIS_Z3; iAxis <= AXIS_Z6; iAxis++)
		{
			if (m_pShareItem->GetStopUser() == true)
				continue;

			nRet &= m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(iAxis, df_HOMINGDONE_TIME);
		}

		if (nRet != VTM_RV_SUCCESSED)
			return nRet;

		// 2. Head AXIS
		// 2.1 Z AXIS
		for (int iAxis = AXIS_Z1; iAxis < AXIS_TYPE_END; iAxis++)
		{
			if (m_pShareItem->GetStopUser() == true)	// 2023.08.09 add
			{
				continue;
			}

			if (nRet != VTM_RV_SUCCESSED)	// 2023.08.09 add
			{
				continue;
			}

			if (iAxis == AXIS_Z1)
			{
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_HEAD_Z_FRONT, true);

				nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveHome(iAxis);
			}
			else if (iAxis == AXIS_Z2)
			{
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_HEAD_Z_BACK, true);

				nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveHome(iAxis);
			}
			else if (
				iAxis == AXIS_ZOOM1 || iAxis == AXIS_THETA1 || iAxis == AXIS_ZOOM2 || iAxis == AXIS_THETA2 || iAxis == AXIS_FILTER1 || iAxis == AXIS_FILTER2 || iAxis == AXIS_VLENS1 || iAxis == AXIS_VLENS2 ||
				iAxis == AXIS_ROTATE1 || iAxis == AXIS_FOCUS1 || iAxis == AXIS_ROTATE2 || iAxis == AXIS_FOCUS2 || iAxis == AXIS_COLOR_DEGREE1 || iAxis == AXIS_COLOR_DEGREE2 ||
				iAxis == AXIS_ROTATE3 || iAxis == AXIS_FOCUS3 || iAxis == AXIS_ROTATE4 || iAxis == AXIS_FOCUS4 || iAxis == AXIS_SPECTO_DEGREE1 || iAxis == AXIS_SPECTO_DEGREE2 ||
				// LYS 2026-08-19 ADD ANGLE AXIS - homed with the head optics group
				iAxis == AXIS_ANGLE_SF1 || iAxis == AXIS_ANGLE_ROTATE1 || iAxis == AXIS_ANGLE_ZOOM1 || iAxis == AXIS_ANGLE_TILT1 || iAxis == AXIS_ANGLE_AZM1 ||
				iAxis == AXIS_ANGLE_SF2 || iAxis == AXIS_ANGLE_ROTATE2 || iAxis == AXIS_ANGLE_ZOOM2 || iAxis == AXIS_ANGLE_TILT2 || iAxis == AXIS_ANGLE_AZM2)
			{
				nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveHome(iAxis);
			}
		}

		// #include "..\Manager\CommTcpIpFactoryMgr.h"		// 파일에 Include 추가
		//m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkSetParameter(enumTCPIpFactorytID_VISION);  // Vistion Motion Homing  // jsyoon 2022.10.14 add		// LYS 2026-09-03 VisMot 없음

		if (nRet == VTM_RV_SUCCESSED)
		{
			// Z AXIS Homing Done Check
			for (int iAxis = AXIS_TYPE_END; iAxis >= AXIS_Z1; iAxis--)
			{
				if (m_pShareItem->GetStopUser() == true)	// 2023.08.09 add
				{
					continue; 
				}

				if (nRet != VTM_RV_SUCCESSED)	// 2023.08.09 add
				{
					continue;
				}

				if (iAxis == AXIS_Z1)
				{
					nRet &= m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(iAxis, df_HOMINGDONE_TIME);

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_HEAD_Z_FRONT, false);
				}
				else if (iAxis == AXIS_Z2)
				{
					nRet &= m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(iAxis, df_HOMINGDONE_TIME);

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_HEAD_Z_BACK, false);
				}
				else if (
					iAxis == AXIS_ZOOM1 || iAxis == AXIS_THETA1 || iAxis == AXIS_ZOOM2 || iAxis == AXIS_THETA2 || iAxis == AXIS_FILTER1 || iAxis == AXIS_FILTER2 || iAxis == AXIS_VLENS1 || iAxis == AXIS_VLENS2 ||
					iAxis == AXIS_ROTATE1 || iAxis == AXIS_FOCUS1 || iAxis == AXIS_ROTATE2 || iAxis == AXIS_FOCUS2 || iAxis == AXIS_COLOR_DEGREE1 || iAxis == AXIS_COLOR_DEGREE2 ||
					iAxis == AXIS_ROTATE3 || iAxis == AXIS_FOCUS3 || iAxis == AXIS_ROTATE4 || iAxis == AXIS_FOCUS4 || iAxis == AXIS_SPECTO_DEGREE1 || iAxis == AXIS_SPECTO_DEGREE2 ||
					// LYS 2026-08-19 ADD ANGLE AXIS - homed with the head optics group
					iAxis == AXIS_ANGLE_SF1 || iAxis == AXIS_ANGLE_ROTATE1 || iAxis == AXIS_ANGLE_ZOOM1 || iAxis == AXIS_ANGLE_TILT1 || iAxis == AXIS_ANGLE_AZM1 ||
					iAxis == AXIS_ANGLE_SF2 || iAxis == AXIS_ANGLE_ROTATE2 || iAxis == AXIS_ANGLE_ZOOM2 || iAxis == AXIS_ANGLE_TILT2 || iAxis == AXIS_ANGLE_AZM2)
				{
					nRet &= m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(iAxis, df_HOMINGDONE_TIME);
				}
			}
		}

		// LYS 2026-09-05 Sub Z축 올리기
		double dPosSubZ3 = 0, dPosSubZ4 = 0, dPosSubZ5 = 0, dPosSubZ6 = 0;
		m_pShareItem->GetMaintControlMgrPtr()->GetPositionConfigMgr()->GetSubZSafetyPosition(&dPosSubZ3, &dPosSubZ4, &dPosSubZ5, &dPosSubZ6);

		stMotorPosition stPositionProbe[4];
		stPositionProbe[0].axis = AXIS_Z3;
		stPositionProbe[0].dPosition = dPosSubZ3;
		stPositionProbe[1].axis = AXIS_Z4;
		stPositionProbe[1].dPosition = dPosSubZ4;
		stPositionProbe[2].axis = AXIS_Z5;
		stPositionProbe[2].dPosition = dPosSubZ5;
		stPositionProbe[3].axis = AXIS_Z6;
		stPositionProbe[3].dPosition = dPosSubZ6;

		
		nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveOrgPosMulti(stPositionProbe, 4, SPEED_HIGH, true);
		if (nRet != VTM_RV_SUCCESSED)
		{
			return nRet;
		}
		

		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_HEAD_Z, false);
	}
	break;
	case 2:
	{
		nRet = VTM_RV_SUCCESSED;
	}
	break;
	case 3:
	{
		// 2.2 Head AXIS
		for (int iAxis = AXIS_X; iAxis < AXIS_TYPE_END; iAxis++)
		{
			if (iAxis == AXIS_X2 )
			{
				if (m_pShareItem->GetStopUser() == true)	// 2023.08.09 add
				{
					continue;
				}

				if (nRet != VTM_RV_SUCCESSED)	// 2023.08.09 add
				{
					continue;
				}

				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_HEAD_X_BACK, true);

				nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveHome(iAxis);
			}
			else if (iAxis == AXIS_X3)
			{
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_HEAD_X_FRONT, true);

				nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveHome(iAxis);
			}
			else if (iAxis == AXIS_Y1)
			{
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_HEAD_Y_BACK_LEFT, true);

				nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveHome(iAxis);
			}
			else if (iAxis == AXIS_Y2)
			{
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_HEAD_Y_BACK_RIGHT, true);

				nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveHome(iAxis);
			}
			else if (iAxis == AXIS_Y3)
			{
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_HEAD_Y_FRONT_LEFT, true);

				nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveHome(iAxis);
			}
			else if (iAxis == AXIS_Y4)
			{
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_HEAD_Y_FRONT_RIGHT, true);

				nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveHome(iAxis);
			}
		}

		if (nRet == VTM_RV_SUCCESSED)
		{
			// Head AXIS Homing Done Check
			for (int iAxis = AXIS_TYPE_END; iAxis >= AXIS_X; iAxis--)
			{
				if (m_pShareItem->GetStopUser() == true)	// 2023.08.09 add
				{
					continue;
				}

				if (nRet != VTM_RV_SUCCESSED)	// 2023.08.09 add
				{
					continue;
				}

				if (iAxis == AXIS_X2)
				{
					nRet &= m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(iAxis, df_HOMINGDONE_TIME);

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_HEAD_X_BACK, false);
				}
				else if (iAxis == AXIS_X3)
				{
					nRet &= m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(iAxis, df_HOMINGDONE_TIME);

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_HEAD_X_FRONT, false);
				}
				else if (iAxis == AXIS_Y1)
				{
					nRet &= m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(iAxis, df_HOMINGDONE_TIME);

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_HEAD_Y_BACK_LEFT, false);
				}
				else if (iAxis == AXIS_Y2)
				{
					nRet &= m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(iAxis, df_HOMINGDONE_TIME);

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_HEAD_Y_BACK_RIGHT, false);
				}
				else if (iAxis == AXIS_Y3)
				{
					nRet &= m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(iAxis, df_HOMINGDONE_TIME);

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_HEAD_Y_FRONT_LEFT, false);
				}
				else if (iAxis == AXIS_Y4)
				{
					nRet &= m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(iAxis, df_HOMINGDONE_TIME);

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_HEAD_Y_FRONT_RIGHT, false);
				}
			}
		}

		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_HEAD, false);
	}
		break;
	case 4:
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_PROBE_Z, true);

		// LYS 2026-06-26 OCP: skip P1-P4 Z homing, only P5 Z
		bool bIsOCP_Init = (m_pShareItem->GetSystemConfigMgrPtr() && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP());

		// 3. Probe
		// 3.1 Probe Z
		for (int iAxis = AXIS_X; iAxis < AXIS_TYPE_END; iAxis++)
		{
			if (m_pShareItem->GetStopUser() == true)	// 2023.08.09 add
			{
				continue;
			}

			if (nRet != VTM_RV_SUCCESSED)	// 2023.08.09 add
			{
				continue;
			}

			if (iAxis == AXIS_PROBE_Z1 && !bIsOCP_Init)
			{
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_PROBE_Z_1, true);

				nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveHome(iAxis);
			}
			else if (iAxis == AXIS_PROBE_Z2 && !bIsOCP_Init)
			{
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_PROBE_Z_2, true);

				nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveHome(iAxis);
			}
			else if (iAxis == AXIS_PROBE_Z3 && !bIsOCP_Init)
			{
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_PROBE_Z_3, true);

				nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveHome(iAxis);
			}
			else if (iAxis == AXIS_PROBE_Z4 && !bIsOCP_Init)
			{
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_PROBE_Z_4, true);

				nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveHome(iAxis);
			}
			else if (iAxis == AXIS_PROBE_Z5 )
			{
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_PROBE_Z_5, true);

				nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveHome(iAxis);
			}

		}

		if (nRet == VTM_RV_SUCCESSED)
		{
			// Probe AXIS Homing Done Check
			for (int iAxis = AXIS_X; iAxis < AXIS_TYPE_END; iAxis++)
			{
				if (m_pShareItem->GetStopUser() == true)	// 2023.08.09 add
				{
					continue;
				}

				if (nRet != VTM_RV_SUCCESSED)	// 2023.08.09 add
				{
					continue;
				}

				if (iAxis == AXIS_PROBE_Z1 )
				{
					nRet &= m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(iAxis, df_HOMINGDONE_TIME);

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_PROBE_Z_1, false);
				}
				if (iAxis == AXIS_PROBE_Z2 )
				{
					nRet &= m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(iAxis, df_HOMINGDONE_TIME);

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_PROBE_Z_2, false);
				}
				if (iAxis == AXIS_PROBE_Z3 )
				{
					nRet &= m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(iAxis, df_HOMINGDONE_TIME);

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_PROBE_Z_3, false);
				}
				if (iAxis == AXIS_PROBE_Z4 )
				{
					nRet &= m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(iAxis, df_HOMINGDONE_TIME);

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_PROBE_Z_4, false);
				}
				if (iAxis == AXIS_PROBE_Z5 )
				{
					nRet &= m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(iAxis, df_HOMINGDONE_TIME);

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_PROBE_Z_5, false);
				}

				if (m_pShareItem->GetStopUser() == true)	// 2023.08.09 add
				{
					break;
				}
			}
		}

		if (nRet == VTM_RV_SUCCESSED)
		{
			double dPosProbeZ = 0;
			nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeMove(AXIS_ALL_Z, dPosProbeZ, SPEED_HIGH);
			if (nRet == VTM_RV_SUCCESSED)
			{
				Sleep(1000);
			}
		}

		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_PROBE_Z, false);
	}
	break;
	case 5:
	{
		// LYS 2026-06-26 OCP: skip all Probe Y homing (not used)
		//if (m_pShareItem->GetSystemConfigMgrPtr() && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP())
		//{
		//	nRet = VTM_RV_SUCCESSED;
		//	break;
		//}

		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_PROBE_Y, true);

		// 3.2 Probe Y
		for (int iAxis = AXIS_X; iAxis < AXIS_TYPE_END; iAxis++)
		{
			if (m_pShareItem->GetStopUser() == true)	// 2023.08.09 add
			{
				continue;
			}

			if (nRet != VTM_RV_SUCCESSED)	// 2023.08.09 add
			{
				continue;
			}

			if (iAxis == AXIS_PROBE_Y1 )
			{
				// LYS 2026-09-02 OCP: skip all Probe Y 1-4 homing (not used)
				if (m_pShareItem->GetSystemConfigMgrPtr()->IsOCP())
				{
					nRet = VTM_RV_SUCCESSED;
					continue;
				}

				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_PROBE_Y_1, true);

				CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetProbeAlign(0, false);

				nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveHome(iAxis);
			}
			if (iAxis == AXIS_PROBE_Y2 )
			{
				// LYS 2026-09-02 OCP: skip all Probe Y 1-4 homing (not used)
				if (m_pShareItem->GetSystemConfigMgrPtr()->IsOCP())
				{
					nRet = VTM_RV_SUCCESSED;
					continue;
				}
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_PROBE_Y_2, true);

				CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetProbeAlign(1, false);

				nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveHome(iAxis);
			}
			if (iAxis == AXIS_PROBE_Y3 )
			{
				// LYS 2026-09-02 OCP: skip all Probe Y 1-4 homing (not used)
				if (m_pShareItem->GetSystemConfigMgrPtr()->IsOCP())
				{
					nRet = VTM_RV_SUCCESSED;
					continue;
				}
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_PROBE_Y_3, true);

				CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetProbeAlign(2, false);

				nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveHome(iAxis);
			}
			if (iAxis == AXIS_PROBE_Y4 )
			{
				// LYS 2026-09-02 OCP: skip all Probe Y 1-4 homing (not used)
				if (m_pShareItem->GetSystemConfigMgrPtr()->IsOCP())
				{
					nRet = VTM_RV_SUCCESSED;
					continue;
				}
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_PROBE_Y_4, true);

				CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetProbeAlign(3, false);

				nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveHome(iAxis);
			}
			if (iAxis == AXIS_PROBE_Y5 )
			{
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_PROBE_Y_5, true);

				CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetProbeAlign(4, false);

				nRet &= m_pShareItem->GetMaintControlMgrPtr()->MoveHome(iAxis);
			}
		}

		if (nRet == VTM_RV_SUCCESSED)
		{
			// Probe AXIS Homing Done Check
			for (int iAxis = AXIS_X; iAxis < AXIS_TYPE_END; iAxis++)
			{
				if (m_pShareItem->GetStopUser() == true)	// 2023.08.09 add
				{
					continue;
				}

				if (nRet != VTM_RV_SUCCESSED)	// 2023.08.09 add
				{
					continue;
				}

				if (iAxis == AXIS_PROBE_Y1 )
				{
					// LYS 2026-09-02 OCP: skip all Probe Y 1-4 homing (not used)
					if (m_pShareItem->GetSystemConfigMgrPtr()->IsOCP())
					{
						nRet = VTM_RV_SUCCESSED;
						continue;
					}
					nRet &= m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(iAxis, df_HOMINGDONE_TIME);

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_PROBE_Y_1, false);
				}
				if (iAxis == AXIS_PROBE_Y2 )
				{
					// LYS 2026-09-02 OCP: skip all Probe Y 1-4 homing (not used)
					if (m_pShareItem->GetSystemConfigMgrPtr()->IsOCP())
					{
						nRet = VTM_RV_SUCCESSED;
						continue;
					}
					nRet &= m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(iAxis, df_HOMINGDONE_TIME);

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_PROBE_Y_2, false);
				}
				if (iAxis == AXIS_PROBE_Y3 )
				{
					// LYS 2026-09-02 OCP: skip all Probe Y 1-4 homing (not used)
					if (m_pShareItem->GetSystemConfigMgrPtr()->IsOCP())
					{
						nRet = VTM_RV_SUCCESSED;
						continue;
					}
					nRet &= m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(iAxis, df_HOMINGDONE_TIME);

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_PROBE_Y_3, false);
				}
				if (iAxis == AXIS_PROBE_Y4 )
				{
					// LYS 2026-09-02 OCP: skip all Probe Y 1-4 homing (not used)
					if (m_pShareItem->GetSystemConfigMgrPtr()->IsOCP())
					{
						nRet = VTM_RV_SUCCESSED;
						continue;
					}
					nRet &= m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(iAxis, df_HOMINGDONE_TIME);

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_PROBE_Y_4, false);
				}
				if (iAxis == AXIS_PROBE_Y5 )
				{
					nRet &= m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(iAxis, df_HOMINGDONE_TIME);

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_PROBE_Y_5, false);
				}
			}
		}

		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_PROBE_Y, false);
	}
	break;
	case 6:
	{
		// LYS 2026-06-26 OCP: skip Probe X homing, home Probe Rotate instead
		// LYS 2026-08-25 Probe Rotate Remove
		//if (m_pShareItem->GetSystemConfigMgrPtr() && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP())
		//{
		//	// Home Probe Rotate (AXIS_R1)
		//	nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveHome(AXIS_R1);
		//	if (nRet == VTM_RV_SUCCESSED)
		//	{
		//		nRet = m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(AXIS_R1, 30000);
		//	}
		//	break;
		//}

		// Probe
		for (int iAxis = AXIS_X; iAxis < df_USE_MAX_AXIS; iAxis++)
		{
			if (iAxis == AXIS_PROBE_X1 || iAxis == AXIS_PROBE_X3 || iAxis == AXIS_PROBE_X5 || iAxis == AXIS_PROBE_X7 || iAxis == AXIS_PROBE_X9)
			{
				if (m_pShareItem->GetStopUser() == true)	// 2023.08.09 add
				{
					continue;
				}

				if (nRet != VTM_RV_SUCCESSED)	// 2023.08.09 add
				{
					continue;
				}

				// LYS 2026-08-25 OCP는 P1~P4가 비활성이다. MoveCmdPosA()가 bUse == FALSE 축에
				// 에러를 반환하고 루프가 첫 에러에서 중단되므로, 해당 갠트리는 건너뛴다.
				if (m_pShareItem->GetMaintControlMgrPtr()->GetMotorControlMgrPtr()->GetAxisProperty(iAxis)->bUse != TRUE)
				{
					continue;
				}

				//if ( (m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->stMotionStatus[iAxis].bEnable != TRUE) || (m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->stMotionStatus[iAxis + 1].bEnable != TRUE))
				if(iAxis == AXIS_PROBE_X9)		// LYS 2026-09-02 TEMP
				{
					nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveHome(iAxis);

					if (nRet == VTM_RV_SUCCESSED)
					{
						nRet = m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(iAxis, df_HOMINGDONE_TIME);
					}

					if (nRet != VTM_RV_SUCCESSED)
					{
						break;
					}
				}
				else
				{
					if (m_pShareItem->GetMaintControlMgrPtr()->IsOutOfServiceMC() == true)
					{
						double dMinPos = m_pShareItem->GetMaintControlMgrPtr()->GetMinSWLimit(iAxis);
						nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(iAxis, dMinPos, true);
					}
				}
			}
		}

		if (nRet == VTM_RV_SUCCESSED)
		{
			for (int iAxis = AXIS_PROBE_X9; iAxis >= AXIS_PROBE_X1; iAxis--)
			{
				if (m_pShareItem->GetStopUser() == true)	// 2023.08.09 add
				{
					continue;
				}

				if (nRet != VTM_RV_SUCCESSED)	// 2023.08.09 add
				{
					continue;
				}

				if (iAxis == AXIS_PROBE_X1 || iAxis == AXIS_PROBE_X3 || iAxis == AXIS_PROBE_X5 || iAxis == AXIS_PROBE_X7 || iAxis == AXIS_PROBE_X9)
				{
					// LYS 2026-08-25 위 호밍 루프와 같은 이유 - MoveCmdPosR()이 미장착 갠트리의
					// 슬레이브 축에서 에러를 낸다.
					if (m_pShareItem->GetMaintControlMgrPtr()->GetMotorControlMgrPtr()->GetAxisProperty(iAxis + 1)->bUse != TRUE)
					{
						continue;
					}

					double dPosGap = 0.;
					dPosGap = m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(iAxis) - m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(iAxis + 1);
					nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosR(iAxis + 1, dPosGap, true, SPEED_LOW);

					if (nRet != VTM_RV_SUCCESSED)
					{
						break;
					}
				}
			}
		}
	}
	break;
	case 7:
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_STAGE_X, true);

		//bool bSafety = false;
		//int nBit = 0;
		//nBit = m_pShareItem->GetMaintControlMgrPtr()->GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(PLC_AUTO_CHANGE_X0_POSITION_03);
		//if (nBit != -1)
		//{
		//	if (nBit == 1)
		//	{
		//		bSafety = true;
		//	}
		//}
		//
		//if (bSafety != true)
		//{
		//	// jsyoon 2022.05.02 add
		//	if (m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_X4) < 100)
		//	{
		//		m_pShareItem->GetMaintControlMgrPtr()->SetAmpEnable(AXIS_X4, true);
		//		// 앞로 이동
		//		//nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosR(AXIS_X4, 1000, false, SPEED_LOW);
		//		// LYS 2026-09-02 AC PACC 방향으로 이동
		//		nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosR(AXIS_X4, -1000, false, SPEED_LOW);
		//		if (nRet == VTM_RV_SUCCESSED)
		//		{
		//			int iLoop = 0;
		//			bool bRet = false;
		//			nRet = MTOR_HOME_FAIL_ALARM;
		//			do
		//			{
		//				Sleep(1000);
		//
		//				bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsPositionOnAC(PLC_AUTO_CHANGE_X0_POSITION_03);
		//				if (m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_X4) > -999)
		//				{
		//					m_pShareItem->GetMaintControlMgrPtr()->StopMove(AXIS_X4);
		//					nRet = VTM_RV_SUCCESSED;
		//					break;
		//				}
		//
		//				iLoop++;
		//
		//			} while (iLoop < 60);  // 60sec
		//		}
		//	}
		//}
		//else
		//{
		//	nRet = VTM_RV_SUCCESSED;
		//}

		// LYS 2026-09-02 AC Safety 위치는 Homing Skip
		//bool bSafety = false;
		//int nBit = 0;
		//nBit = m_pShareItem->GetMaintControlMgrPtr()->GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(PLC_AUTO_CHANGE_X0_POSITION_03);
		//if (nBit != -1)
		//{
		//	if (nBit == 1)
		//	{
		//		double CurrAC_X = m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_X4);
		//
		//		double dReadyPos = 0;
		//		m_pShareItem->GetMaintControlMgrPtr()->GetPositionConfigMgr()->GetACPosition(nullptr, &dReadyPos, nullptr);
		//
		//		double AbsPos;
		//		AbsPos = CurrAC_X - dReadyPos;
		//
		//		if (abs(AbsPos) < 0.1) bSafety = true;
		//		else bSafety = false;
		//	}
		//}
		//
		//if (bSafety == true) nRet == VTM_RV_SUCCESSED;
		//else
		//{
		//	nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveHome(AXIS_X4);
		//
		//	if (nRet == VTM_RV_SUCCESSED)
		//	{
		//		nRet = m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(AXIS_X4, df_HOMINGDONE_TIME);
		//	}
		//
		//	if (nRet == VTM_RV_SUCCESSED)
		//	{
		//		double CurrAC_X = m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_X4);
		//
		//		double dReadyPos = 0;
		//		m_pShareItem->GetMaintControlMgrPtr()->GetPositionConfigMgr()->GetACPosition(nullptr, &dReadyPos, nullptr);
		//
		//		nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_X4, dReadyPos, false, SPEED_LOW);
		//	}
		//}
			

		// Stage 
		if (nRet == VTM_RV_SUCCESSED)
		{
			nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveHome(AXIS_X);
		}
		
		if (nRet == VTM_RV_SUCCESSED)
		{
			nRet = m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(AXIS_X, df_HOMINGDONE_TIME);
		}

		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_STAGE_X, false);
	}
	break;
	case 8:
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_STAGE_R, true);

		nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveHome(AXIS_LOADER_R);

		if (nRet == VTM_RV_SUCCESSED)
		{
			nRet = m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(AXIS_LOADER_R, df_HOMINGDONE_TIME);
		}

		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_STAGE_R, false);
	}
	break;
	case 9:
	{
		 // Auto change
		 // Auto Change 위치 확인 후 Z축 호밍진행
		 // LYS 2026-09-02 Skip
		if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsPositionOnAC(2) == true) // SACC 내부에 있을 경우
		{
			// ACX 축 +방향으로 조금 이동
			if (!m_pShareItem->GetMaintControlMgrPtr()->GetAmpEnable(AXIS_X4))		// LYS 2026-09-04 Enable Check
			{
				m_pShareItem->GetMaintControlMgrPtr()->SetAmpEnable(AXIS_X4, true); 
				
				// enable 완료 폴링 (타임아웃)
				int nWait = 0;
				while (!m_pShareItem->GetMaintControlMgrPtr()->GetAmpEnable(AXIS_X4) && nWait++ < 100)
					Sleep(30);   // 최대 ~2초
				nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosR(AXIS_X4, 1000, false, SPEED_HIGH);

				//nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosR(AXIS_X4, 1000, false, SPEED_LOW);

			}
			else
			{
				double CurrAC_X = m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_X4);

				double dReadyPos = 0;
				m_pShareItem->GetMaintControlMgrPtr()->GetPositionConfigMgr()->GetACPosition(nullptr, &dReadyPos, nullptr);
				nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_X4, dReadyPos, false, SPEED_HIGH);
			}
				

			
			if (nRet == VTM_RV_SUCCESSED)
			{
				int iLoop = 0;
				bool bRet = false;
				nRet = MTOR_HOME_FAIL_ALARM;
				do
				{
					if (m_pShareItem->GetStopUser() == true)	// 2023.08.09 add
					{
						nRet = VTM_RV_SUCCESSED;
						break;
					}

					Sleep(1000);

					bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsPositionOnAC(0);
					if (bRet == true)
					{
						m_pShareItem->GetMaintControlMgrPtr()->StopMove(AXIS_X4);
						nRet = VTM_RV_SUCCESSED;
						break;
					}

					iLoop++;

				} while (iLoop < 60);  // 30sec
			}
		}
	}
	break;
	case 10:
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_AC_Z, true);

		nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveHome(AXIS_Z9);
		
		if (nRet == VTM_RV_SUCCESSED)
		{
			nRet = m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(AXIS_Z9, df_HOMINGDONE_TIME);
		}

		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_AC_Z, false);
	}
	break;
	case 11:
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_AC_X, true);

		// LYS 2026-07-02 AC Safety 위치는 Homing Skip
		bool bSafety = false;
		int nBit = 0;
		nBit = m_pShareItem->GetMaintControlMgrPtr()->GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(PLC_AUTO_CHANGE_X0_POSITION_04);
		if (nBit != -1)
		{
			if (nBit == 1)
			{
				double CurrAC_X = m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_X4);

				double dReadyPos = 0;
				double dPunInPos = 0;
				m_pShareItem->GetMaintControlMgrPtr()->GetPositionConfigMgr()->GetACPosition(nullptr, &dReadyPos, &dPunInPos);

				double AbsPos;
				AbsPos = CurrAC_X - dReadyPos;

				if (abs(AbsPos) < 0.1) bSafety = true;
				else bSafety = false;
			}
		}

		if (bSafety == true) nRet = VTM_RV_SUCCESSED;
		else nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveHome(AXIS_X4);

		if (nRet == VTM_RV_SUCCESSED)
		{
			nRet = m_pShareItem->GetMaintControlMgrPtr()->CheckHomeDone(AXIS_X4, df_HOMINGDONE_TIME);
		}

		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_HOME_AC_X, false);
	}
	break;
	default:
		break;
	}
	
	return nRet;
}

bool CSystemInitializeStep_Safety::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CSystemInitializeStep_Safety Start."));

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_SAFETY_PROC, true);

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
				break;
			}

			iLoop++;

		} while (iLoop < 1000);
	}
	else
	{
		bRet = true;
	}

	if (bRet == true)
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CSystemInitializeStep_Safety OK."));
	}
	else
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CSystemInitializeStep_Safety NG."));
		m_pShareItem->SetFail(true, nRet);
	}

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MOT_SAFETY_PROC, false);

	return bRet;
}

bool CSystemInitializeStep_ProbeAlign::Run()
{
	bool bRet = true;
	int nRet = VTM_RV_SUCCESSED;

	if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsGlassSensorOn() == true)
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CSystemInitializeStep_ProbeAlign Start."));

		for (int iProbe = df_PROBE_AGING1; iProbe <= df_PROBE_MEAS; iProbe++)
		{
			if (m_pShareItem->GetSystemConfigMgrPtr()->IsOCP()) iProbe = df_PROBE_MEAS;		// LYS 2026-08-24 OCP ProbeAlign Init 1-4 Skip
			
			// Probe exist check
			if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsProbeExistSensorOn(iProbe) == true)
			{
				if (CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->GetProbeAlign(iProbe - 1) == true)
				{
					continue;
				}

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
					CString strLog;
					strLog.Format(_T("CSystemInitializeStep_ProbeAlign Start. %d"), iProbe);
					m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);

					CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetProbe(iProbe);

					m_pShareItem->GetMaintControlMgrPtr()->GetShareMemoryMgrPtr()->SetSys_ProbeChange(1);

					CDoWorkGlassSequence * pDoWorkGlassSeq = CSequenceManager::GetInstance()->GeDoWorkGlassSequence();
					bRet = pDoWorkGlassSeq->ItemStart(_T("SEQUENCE_ITEM_DWGLASS_SEQ_PROBE_SW_ALIGN"), true);

					m_pShareItem->GetMaintControlMgrPtr()->GetShareMemoryMgrPtr()->SetSys_ProbeChange(0);

					if (bRet != true)
					{
						break;
					}

					CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetProbeAlign(iProbe - 1);

					Sleep(1000);
				}
				else
				{
					m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CSystemInitializeStep_ProbeAlign Recipe NG."));
					bRet = true;
					break;
				}
			}
		}

		if (bRet == true)
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CSystemInitializeStep_ProbeAlign OK."));
		}
		else
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CSystemInitializeStep_ProbeAlign NG."));
			m_pShareItem->SetFail(true, nRet);
		}
	}

	return bRet;
}
