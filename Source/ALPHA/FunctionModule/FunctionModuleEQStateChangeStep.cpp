//#include "stdafx.h"
#include "pch.h"

#include "FunctionModuleEQStateChangeStep.h"

#include "..\BaseSequenceImpl.h"
#include "..\SystemParamShare.h"
#include "..\Manager\SystemConfigMgr.h"
//#include "..\Manager\MotorControlMgr.h"
//#include "..\Manager\IOMapControlMgr.h"
#include "..\Manager\MaintControlMgr.h"
#include "..\Manager\LightControlMgr.h"
#include "..\Manager\ShareMemoryMgr.h"
#include "CoreFunction.h"

#include "..\Manager\SequenceManager.h"
#include "FunctionModuleEQState.h"
#include "..\Manager\PositionConfigMgr.h"
#include "Manager/MCCLogMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//0 : Loader Load, 1 : Loader Unload, 2 : Loader Meas(Contact)
bool CFunctionModuleEQStateChangeStep::MoveLoaderXYR(int nType)
{
	double dPosX = 0.0;
	double dPosY = 0.0;
	double dPosR = 0.0;

	m_pShareItem->GetMaintControlMgrPtr()->ResetAlign();

	if (nType == 0)  // Load
	{
		m_pShareItem->GetPositionConfigMgrPtr()->GetLoaderLoadPosition(&dPosX, &dPosY, &dPosR); 
	}
	else if (nType == 1) // Unlaod
	{
		m_pShareItem->GetPositionConfigMgrPtr()->GetLoaderUnloadPosition(&dPosX, &dPosY, &dPosR);
	}
	else if (nType == 2) // Meas
	{
		m_pShareItem->GetPositionConfigMgrPtr()->GetLoaderMeasPosition(&dPosX, &dPosY, &dPosR);
	}
	else
	{
		return false;
	}

	long lRet = VTM_RV_SUCCESSED;
	bool bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLiftpinSensorDown();
	if (bRet != true)
	{
		lRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_LPMove(false);

		if (lRet == VTM_RV_SUCCESSED)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLiftpinSensorDown();
		}
	}

	if (bRet == true)
	{
		
		lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_X, dPosX, true);

		if (lRet == VTM_RV_SUCCESSED)
		{
			lRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_RotateMove(dPosR, nType); // MoveCmdPosA(AXIS_LOADER_R, dPosR, true);
		}

		if (lRet != VTM_RV_SUCCESSED)
		{
			bRet = false;
		}
	}

	/*stMotorPosition stPosition[3];

	stPosition[0].axis = AXIS_LOADER_X;
	stPosition[0].dPosition = dPosX;
	stPosition[1].axis = AXIS_LOADER_Y;
	stPosition[1].dPosition = dPosY;
	stPosition[2].axis = AXIS_LOADER_R;
	stPosition[2].dPosition = dPosZ;

	m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosMulti(stPosition, 3, 1);*/

	return bRet;
}

//0 : Loader Up, 1 : Loader Middle, 2 : Loader Down
long CFunctionModuleEQStateChangeStep::MoveLoaderZ(int nType)
{
	long lRet = VTM_RV_SUCCESSED;
	lRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_LPMove(false);
	return lRet;

	/*double dPosUp = 0.0;
	double dPosMiddle = 0.0;
	double dPosPre = 0.0;
	double dPosDown = 0.0;

	m_pShareItem->GetPositionConfigMgrPtr()->GetLoaderZPossition(&dPosUp, &dPosMiddle, &dPosPre, &dPosDown);

	stMotorPosition stPosition[1];

	stPosition[0].axis = AXIS_RJ;

	if (nType == 0)
	{
		stPosition[0].dPosition = dPosUp;
	}
	else if (nType == 1)
	{
		stPosition[0].dPosition = dPosMiddle;
	}
	else if (nType == 2)
	{
		stPosition[0].dPosition = dPosDown;
	}
	else
	{
		return false;
	}

	

	m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosMulti(stPosition, 1, 1);

	return true;*/
}

int CFunctionModuleEQStateChangeStep::MoveSafetyAll()
{
	int nRet = df_MOTION_RV_SUCCESSED;

	if (m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->bSystemInitOk == TRUE)
		nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmd_SafetyAll();
	else
		nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmd_SafetyAll(true);

	return nRet;
}

//0 : Loader, 1 : RackJack, 2 : AllAxis
int CFunctionModuleEQStateChangeStep::WaitMoveDone(int nType)
{
	int nErrorAxis = 0;

	if (nType == 0)
	{
		for (int nAxis = 0; nAxis < AXIS_TYPE_END; nAxis++)
		{
			if (nAxis == AXIS_LOADER_X || nAxis == AXIS_LOADER_Y || nAxis == AXIS_LOADER_R)
			{
				if (m_pShareItem->GetMaintControlMgrPtr()->DoWork_MotionDone(nAxis) != df_MOTION_RV_SUCCESSED)
				{
					//실패한 축번호 리턴
					nErrorAxis = nAxis;
					return nErrorAxis;
				}
			}
		}
	}
	else if (nType == 1)
	{
		for (int nAxis = 0; nAxis < AXIS_TYPE_END; nAxis++)
		{
			if (nAxis == AXIS_RJ)
			{
				if (m_pShareItem->GetMaintControlMgrPtr()->DoWork_MotionDone(nAxis) != df_MOTION_RV_SUCCESSED)
				{
					//실패한 축번호 리턴
					nErrorAxis = nAxis;
					return nErrorAxis;
				}
			}
		}
	}
	else if (nType == 2)
	{
		for (int nAxis = 0; nAxis < AXIS_TYPE_END; nAxis++)
		{
			if (m_pShareItem->GetMaintControlMgrPtr()->DoWork_MotionDone(nAxis) != df_MOTION_RV_SUCCESSED)
			{
				//실패한 축번호 리턴
				nErrorAxis = nAxis;
				return nErrorAxis;
			}
		}
	}
	else
	{
		nErrorAxis = -1;
	}

	if (nErrorAxis == 0)
		return df_MOTION_RV_SUCCESSED;
	else
		return nErrorAxis;

//	return nErrorAxis;		//nErrorAxis = 0 (정상)
}

bool CFunctionModuleEQStateChangeStep_None::Run()
{
	// init
	m_pShareItem->SetComplete(false);
	m_pShareItem->SetAlarmOn(false);
	m_pShareItem->SetFail(false);
	m_pShareItem->SetStopUser(false);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_None Start."));

	m_pShareItem->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_PROCESS); // 2023.08.09 add
	
	/*m_pShareItem->SetAlarmOn(false);
	m_pShareItem->SetComplete(false);
	m_pShareItem->SetFail(false);*/


	//상태 전환 명령을 가져와서 Set 하면 되지 않을까?
	//ex. R2L 동작

//	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_None End."));

	return true;
}

bool CFunctionModuleEQStateChangeStep_LPUp::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LPUp Start."));
	
	DWORD dTactStart, dTactEnd, dTactTime;
	dTactStart = GetTickCount();
	CString strTactTime, strLog;

	bool bGlassExist = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsGlassSensorOn();
	if (bGlassExist == true)
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_UNLOAD_READY_CHANGE_STEP8, true);
	}
	else
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_LOAD_READY_CHANGE_STEP1, true);
	}

	//PLC IO 동작
	//1. LP가 Up이 아닐 경우 Up
	//2. Up 동작 확인
	bool bRet = false;

	if (m_pShareItem->GetMaintControlMgrPtr()->IsOutOfServiceMC() == true)
	{
		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLiftpinSensorUp();
		if (!bRet)
		{
			bRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_LPMove(true);

			if (bRet == true)
			{
				//동작이 완료 되기까지 기다리는게 필요할 것 같은데....
				bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_LIFT_PIN_SENSOR_UP01);
			}

			if (bRet == false)
			{
				int nErrorAddress = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->GetLastCheckAddress();

				dTactEnd = GetTickCount();
				if (dTactEnd >= dTactStart)
					dTactTime = dTactEnd - dTactStart;
				else
					dTactTime = ((DWORD)0xFFFFFFFF) - (dTactStart - dTactEnd);
				strTactTime.Format(_T("%d"), dTactTime);
				strLog.Format(_T("CFunctionModuleEQStateChangeStep_LPUp NG, Address = M%d, T=%s msec"), nErrorAddress, strTactTime);

				m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);

				//strLog.Format(_T("CFunctionModuleEQStateChangeStep_LPUp NG, Address = M%d"), nErrorAddress);
				//m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);

				return false;
			}
		}
	}
	else
	{
		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsSafetySensorOn();
		{
			if (bRet == true)
			{
				bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLiftpinSensorUp();
				if (!bRet)
				{
					bRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_LPMove(true);

					if (bRet == true)
					{
						//동작이 완료 되기까지 기다리는게 필요할 것 같은데....
						bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_LIFT_PIN_SENSOR_UP01);
					}

					if (bRet == false)
					{
						int nErrorAddress = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->GetLastCheckAddress();

						dTactEnd = GetTickCount();
						if (dTactEnd >= dTactStart)
							dTactTime = dTactEnd - dTactStart;
						else
							dTactTime = ((DWORD)0xFFFFFFFF) - (dTactStart - dTactEnd);
						strTactTime.Format(_T("%d"), dTactTime);
						strLog.Format(_T("CFunctionModuleEQStateChangeStep_LPUp NG, Address = M%d, T=%s msec"), nErrorAddress, strTactTime);

						m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);

						
						//strLog.Format(_T("CFunctionModuleEQStateChangeStep_LPUp NG, Address = M%d"), nErrorAddress);
						//m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);

						return false;
					}
				}
			}
		}
	}

	dTactEnd = GetTickCount();
	if (dTactEnd >= dTactStart)
		dTactTime = dTactEnd - dTactStart;
	else
		dTactTime = ((DWORD)0xFFFFFFFF) - (dTactStart - dTactEnd);
	strTactTime.Format(_T("%d"), dTactTime);
	strLog.Format(_T("CFunctionModuleEQStateChangeStep_LPUp End, %d, T=%s msec"), bRet, strTactTime);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);

//	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LPUp End."));

	if (bGlassExist == true)
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_UNLOAD_READY_CHANGE_STEP8, false);
	}
	else
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_LOAD_READY_CHANGE_STEP1, false);
	}
	
	return bRet;
}

bool CFunctionModuleEQStateChangeStep_LoaderXYRMeas::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LoaderXYRMeas Start."));
	
	bool bRet = false;
	bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLoaderPosMeas();
	if (bRet != true)
	{
		bRet = MoveLoaderXYR(2);
	}

	if (bRet != true)
	{
		//알람
		CString strLog;
		strLog.Format(_T("CFunctionModuleEQStateChangeStep_LoaderXYRLoad NG. MoveLoaderXYR Done Failed, Axis = %d."), bRet);
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(ERRO, strLog);
	}

#ifdef _USE_NOT
	//Motion 동작
	//1. LP가 Up인지 확인
	//2. Loader Z down인지 확인
	//3. X, Y, R 동시 이동
	//4. Loader Measure 위치 인지 확인
	bool bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLiftpinSensorUp();
	if (bRet)
	{
		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLoaderZLoadSensorOn();
		if (bRet)
		{
			//Config 파일에서 좌표를 가져와서 이동
			//3. X, Y, R 동시 이동
			//4. Loader Measure 위치 인지 확인
			bRet = MoveLoaderXYR(2);
			int nRet = WaitMoveDone(0);
			if (nRet != 0)
			{
				//알람
				CString strLog;
				strLog.Format(_T("CFunctionModuleEQStateChangeStep_LoaderXYRMeas NG. MoveLoaderXYR Done Failed, Axis = %d."), nRet);
				m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);
				return false;
			}
		}
		else
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LoaderXYRMeas NG. LoaderZLoadSensorOn is off."));
			return false;
		}
	}
	else
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LoaderXYRMeas NG. LiftPinSensor is off."));
		return false;
	}
#endif // _USE_NOT

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LoaderXYRMeas End."));
	
	return bRet;
}

bool CFunctionModuleEQStateChangeStep_ClampOut::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_ClampOut Start."));
	
#ifdef _USE_NOT
	//PLC IO 동작
	//1. Loader Z가 Safety 인지 확인
	//2. Sample Stage가 Safety 인지 확인
	//3. Clamp Out
	//4. Clamp Out 확인
	bool bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsSafetySensorOn();
	if (bRet)
	{
		m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_EXP_B_FRONT_SAMPLE_CHUCK_CLAMP_FOLD, 85, 8);

		//동작이 완료 되기까지 기다리는게 필요할 것 같은데....
		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_IMP_B_FRONT_CLAMP_OFF_01, 2, 2);
		if(bRet) bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_IMP_B_RIGHT_CLAMP_OFF_01, 2, 4);
		if(bRet) bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_IMP_B_BACK_CLAMP_OFF_01, 2, 2);
		if(bRet) bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_IMP_B_LEFT_CLAMP_OFF_01, 2, 4);
		if (bRet == false)
		{
			int nErrorAddress = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->GetLastCheckAddress();
			
			CString strLog;
			strLog.Format(_T("CFunctionModuleEQStateChangeStep_ClampOut Clamp Unfold NG, Address = M%d"), nErrorAddress);
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);

			return false;
		}
	}
	else
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_ClampOut SafetySensorOn NG."));
		return false;
	}
#endif // _USE_NOT

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_ClampOut End."));
	
	return true;
}

bool CFunctionModuleEQStateChangeStep_LoaderZUp::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LoaderZUp Start."));
	
	//Motion 동작
	//1. Loader X가 Meas. 위치 인지 확인
	//2. Sample Stage가 Safety 인지 확인
	//3. Loader Z Up
	//4. Loader Z Up 확인
	bool bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsSafetySensorOn();
	if (bRet)
	{
		int nRet = df_MOTION_RV_SUCCESSED;
		nRet = MoveLoaderZ(0);

		if (nRet == df_MOTION_RV_SUCCESSED)
		{
			nRet = WaitMoveDone(1);
		}
		
		if (nRet != df_MOTION_RV_SUCCESSED)
		{
			//알람
			CString strLog;
			strLog.Format(_T("CFunctionModuleEQStateChangeStep_LoaderZUp NG. MoveLoaderZ Done NG, Axis = %d."), nRet);
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);

			return false;
		}
	}
	else
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LoaderZUp SafetySensorOn NG."));
		return false;
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LoaderZUp End."));
	
	return true;
}

bool CFunctionModuleEQStateChangeStep_VacuumOn::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_VacuumOn Start."));
	
#ifdef _USE_NOT
	//PLC IO 동작
	//1. Loader X가 Meas. 위치 인지 확인
	//2. Sample Stage가 Safety 인지 확인
	//3. Loader Z가 Up인지 확인
	//4. Vacuum On
	//5. Vacuum On 확인
	bool bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLoaderXMeasSensorOn();
	if (bRet)
	{
		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsSafetySensorOn();
		if (bRet)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLoaderZMeasSensorOn();
			if (bRet)
			{
				bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsVacuumSensorOn();
				if (bRet)
				{
					//정상
					;
				}
				else
				{
					m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_EXP_B_SAMPLE_CHUCK_VACUUM_01_ON, 21, 6);

					//동작이 완료 되기까지 기다리는게 필요할 것 같은데....
					bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_IMP_B_SAMPLE_CHUCK_VACUUM_01_ON, 3, 1);
					if (bRet == false)
					{
						int nErrorAddress = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->GetLastCheckAddress();

						CString strLog;
						strLog.Format(_T("CFunctionModuleEQStateChangeStep_ClampOut VaccumOn NG, Address = M%d"), nErrorAddress);
						m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);

						return false;
					}
				}
			}
			else
			{
				m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_ClampOut LoaderZMeasSensorOn NG."));
				return false;
			}
		}
		else
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_ClampOut LoaderXMeasSensorOn NG."));
			return false;
		}
	}
	else
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_ClampOut LoaderXMeasSensor NG."));
		return false;
	}
#endif // _USE_NOT

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_VacuumOn End."));
	
	return true;
}

bool CFunctionModuleEQStateChangeStep_VacuumOff::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_VacuumOff Start."));
	
#ifdef _USE_NOT
	//PLC IO 동작
	//1. Loader X가 Meas. 위치 인지 확인
	//2. Sample Stage가 Safety 인지 확인
	//3. Loader Z가 Up인지 확인
	//4. Vacuum Off
	//5. Air On
	//6. Air Off
	//7. Vacuum Off 확인
	bool bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLoaderXMeasSensorOn();
	if (bRet)
	{
		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsSafetySensorOn();
		if (bRet)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLoaderZMeasSensorOn();
			if (bRet)
			{
				bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsVacuumSensorOff();
				if (bRet)
				{
					//정상
					;
				}
				else
				{
					m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_EXP_B_SAMPLE_CHUCK_VACUUM_01_ON, 42, 6);

					m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_EXP_B_SAMPLE_CHUCK_BLOWING, 1, 1);
					
					Sleep(2000);

					m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_EXP_B_SAMPLE_CHUCK_BLOWING, 0, 1);

					//동작이 완료 되기까지 기다리는게 필요할 것 같은데....
					bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_IMP_B_SAMPLE_CHUCK_VACUUM_01_ON, 3, 1, false);
					if (bRet == false)
					{
						int nErrorAddress = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->GetLastCheckAddress();

						CString strLog;
						strLog.Format(_T("CFunctionModuleEQStateChangeStep_ClampOut Clamp Unfold NG, Address = M%d"), nErrorAddress);
						m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);

						return false;
					}
				}
			}
			else
			{
				m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_VacuumOff LoaderZMeasSensorOn NG."));
				return false;
			}
		}
		else
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_VacuumOff SafetySensorOn NG."));
			return false;
		}
	}
	else
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_VacuumOff LoaderXMeasSensorOn NG."));
		return false;
	}
#endif // _USE_NOT

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_VacuumOff End."));
	
	return true;
}

bool CFunctionModuleEQStateChangeStep_LoaderZDown::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LoaderZDown Start."));
	
	//Motion 동작
	//1. Loader X가 Meas. 위치 인지 확인
	//2. Sample Stage가 Safety 인지 확인
	//3. Loader Z가 Up인지 확인
	//4. Vacuum On 확인
	//5. Loader Z 이동
	//6. Loader Z 이동 확인
	bool bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLoaderXMeasSensorOn();
	if (bRet)
	{
		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsSafetySensorOn();
		if (bRet)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLoaderZMeasSensorOn();
			if (bRet)
			{
				bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsVacuumSensorOff();
				if (bRet)
				{
					//5. Loader Z 이동
					//6. Loader Z 이동 확인
					int nRet = VTM_RV_SUCCESSED;
					nRet = MoveLoaderZ(2);

					if (nRet == VTM_RV_SUCCESSED)
					{
						nRet = WaitMoveDone(1);
					}

					if (nRet != VTM_RV_SUCCESSED)
					{
						//알람
						CString strLog;
						strLog.Format(_T("CFunctionModuleEQStateChangeStep_LoaderZDown NG. MoveLoaderZ Done NG, Axis = %d."), nRet);
						m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);

						return false;
					}
				}
				else
				{
					int nErrorAddress = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->GetLastCheckAddress();
					
					CString strLog;
					strLog.Format(_T("CFunctionModuleEQStateChangeStep_LoaderZDown VacuumSensorOff NG, Address = M%d"), nErrorAddress);
					m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);
					return false;
				}
			}
			else
			{
				m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LoaderZDown LoaderZMeasSensorOn NG."));
				return false;
			}
		}
		else
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LoaderZDown SafetySensorOn NG."));
			return false;
		}
	}
	else
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LoaderZDown LoaderXMeasSensorOn NG."));
		return false;
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LoaderZDown End."));
	
	return true;
}

bool CFunctionModuleEQStateChangeStep_LoaderZDown2Step::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LoaderZDown2Step Start."));
	
	//Motion 동작
	//1. Loader X가 Meas. 위치 인지 확인
	//2. Sample Stage가 Safety 인지 확인
	//3. Loader Z가 Up인지 확인
	//4. Loader Z 이동(-10)
	//5. Vacuum Off 확인
	//6. Loader Z 이동(2번이동)
	//7. Loader Z 이동 확인
	bool bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLoaderXMeasSensorOn();
	if (bRet)
	{
		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsSafetySensorOn();
		if (bRet)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLoaderZMeasSensorOn();
			if (bRet)
			{
				bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsVacuumSensorOff();
				if (bRet)
				{
					//Config 파일에서 좌표를 가져와서 이동
					//6. Loader Z 이동(2번이동)
					//7. Loader Z 이동 확인
					int nRet = VTM_RV_SUCCESSED;
					nRet = MoveLoaderZ(1);
					if (nRet == VTM_RV_SUCCESSED)
					{
						nRet = WaitMoveDone(1);
					}
					
					if (nRet != VTM_RV_SUCCESSED)
					{
						//알람
						CString strLog;
						strLog.Format(_T("CFunctionModuleEQStateChangeStep_LoaderZDown2Step Step1 NG. MoveLoaderZ Done NG, Axis = %d."), nRet);
						m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);

						return false;
					}

					//대기 시간
					Sleep(2000);

					nRet = MoveLoaderZ(2);

					if (nRet == VTM_RV_SUCCESSED)
					{
						nRet = WaitMoveDone(1);
					}
					
					if (nRet != VTM_RV_SUCCESSED)
					{
						//알람
						CString strLog;
						strLog.Format(_T("CFunctionModuleEQStateChangeStep_LoaderZDown2Step Step2 NG. MoveLoaderZ Done NG, Axis = %d."), nRet);
						m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);

						return false;
					}
				}
				else
				{
					int nErrorAddress = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->GetLastCheckAddress();

					CString strLog;
					strLog.Format(_T("CFunctionModuleEQStateChangeStep_LoaderZDown2Step VacuumSensorOff NG, Address = M%d"), nErrorAddress);
					m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);
					return false;
				}
			}
			else
			{
				m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LoaderZDown2Step LoaderZMeasSensorOn NG."));
				return false;
			}
		}
		else
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LoaderZDown2Step SafetySensorOn NG."));
			return false;
		}
	}
	else
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LoaderZDown2Step LoaderXMeasSensorOn NG."));
		return false;
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LoaderZDown2Step End."));
	
	return true;
}

bool CFunctionModuleEQStateChangeStep_ClampIn::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_ClampIn Start."));
	
#ifdef _USE_NOT
	//PLC IO 동작
	//1. Loader X가 Meas. 위치 인지 확인
	//2. Sample Stage가 Safety 인지 확인
	//3. Loader Z가 Down인지 확인
	//4. Clamp In
	//5. Clamp In 확인
	bool bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLoaderXMeasSensorOn();
	if (bRet)
	{
		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsSafetySensorOn();
		if (bRet)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLoaderZLoadSensorOn();
			if (bRet)
			{
				m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_EXP_B_FRONT_SAMPLE_CHUCK_CLAMP_FOLD, 0, 1);

				bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_IMP_B_FRONT_CLAMP_ON_01, 2, 2);
				if (bRet) bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_IMP_B_RIGHT_CLAMP_ON_01, 2, 4);
				if (bRet) bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_IMP_B_BACK_CLAMP_ON_01, 2, 2);
				if (bRet) bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_IMP_B_LEFT_CLAMP_ON_01, 2, 4);
				if (bRet == false)
				{
					int nErrorAddress = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->GetLastCheckAddress();

					CString strLog;
					strLog.Format(_T("CFunctionModuleEQStateChangeStep_ClampOut Clamp Unfold NG, Address = M%d"), nErrorAddress);
					m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);

					return false;
				}
			}
			else
			{
				m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LoaderZDown2Step LoaderZLoadSensorOn NG."));
				return false;
			}
		}
		else
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LoaderZDown2Step SafetySensorOn NG."));
			return false;
		}
	}
	else
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LoaderZDown2Step LoaderXMeasSensorOn NG."));
		return false;
	}
#endif // _USE_NOT

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_ClampIn End."));
	
	return true;
}

bool CFunctionModuleEQStateChangeStep_LoaderXYRLoad::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LoaderXYRLoad Start."));
	
	bool bRet = false, bGlassExist = false;
	int nRet = VTM_RV_SUCCESSED;
	int nMCC_Action = MCC_A_LOAD_READY_CHK_STEP1;
	bGlassExist = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsGlassSensorOn();

	if (bGlassExist == true)
	{
		nMCC_Action = MCC_A_UNLOAD_READY_CHK_STEP1;
	}

	CMCCLogMgr::GetInstance()->SetMCC_Action(nMCC_Action, true);

	bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLoaderArmSensorOn();
	if (bRet == true)
	{
		nRet = PLC_ARM_SENSOR_DETECT;
	}

	CMCCLogMgr::GetInstance()->SetMCC_Action(nMCC_Action, false);

	if (nRet == VTM_RV_SUCCESSED)
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(nMCC_Action + 1, true);

		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsShutterSensorClose();
		if (bRet != true)
		{
			nRet = PLC_LOADER_SHUTTER_UP_ERROR;
		}

		CMCCLogMgr::GetInstance()->SetMCC_Action(nMCC_Action + 1, false);
	}

	if (nRet == VTM_RV_SUCCESSED)
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(nMCC_Action + 2, true);

		bool bLiftDown = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLiftpinSensorDown();
		if (bLiftDown != true)
		{
			nRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_LPMove(false);

			if (nRet == VTM_RV_SUCCESSED)
			{
				bLiftDown = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLiftpinSensorDown();
			}
		}

		if (bLiftDown == true)
		{
			nRet = VTM_RV_SUCCESSED;
		}
		else
		{
			nRet = PLC_LIFTPIN_DOWN_POS_ERROR;
		}

		CMCCLogMgr::GetInstance()->SetMCC_Action(nMCC_Action + 2, true);
	}
	

	if (nRet == VTM_RV_SUCCESSED)
	{
		if (bGlassExist != true)	// load
		{
			CMCCLogMgr::GetInstance()->SetMCC_Action(nMCC_Action + 3, true);

			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLoaderPosLoad();

			CMCCLogMgr::GetInstance()->SetMCC_Action(nMCC_Action + 3, false);
			CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_UNLOAD_READY_MOVE_STEP1, true);

			if (bRet != true)
			{
				bRet = MoveLoaderXYR(0);
			}

			CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_UNLOAD_READY_MOVE_STEP1, true);

		}
		else // UnLoad
		{
			CMCCLogMgr::GetInstance()->SetMCC_Action(nMCC_Action + 3, true);

			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLoaderPosUnLoad();

			CMCCLogMgr::GetInstance()->SetMCC_Action(nMCC_Action + 3, false);

			CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_LOAD_READY_MOVE_STEP1, true);

			if (bRet != true)
			{
				bRet = MoveLoaderXYR(1);
			}

			CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_LOAD_READY_MOVE_STEP1, false);
		}

		if (bRet != true)
		{
			//알람
			CString strLog;
			strLog.Format(_T("CFunctionModuleEQStateChangeStep_LoaderXYRLoad NG. MoveLoaderXYR Done Failed, Axis = %d."), bRet);
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(ERRO, strLog);
		}

		CMCCLogMgr::GetInstance()->SetMCC_Action(nMCC_Action + 4, true);
		CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsGlassSensorOn();
		CMCCLogMgr::GetInstance()->SetMCC_Action(nMCC_Action + 4, true);
	}
	else
	{
		bRet = false;
	}
	

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LoaderXYRLoad End."));
	
	return bRet;
}

bool CFunctionModuleEQStateChangeStep_LPDown::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LPDown Start."));

	DWORD dTactStart, dTactEnd, dTactTime;
	dTactStart = GetTickCount();
	CString strTactTime, strLog;
	
	bool bGlassExist = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsGlassSensorOn();
	if (bGlassExist == true)
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_READY_CHANGE_STEP2, true);
	}

	//PLC IO 동작
	//1. LP가 Down이 아닐 경우 Down
	//2. Up 동작 확인
	bool bRet = false;
	bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsSafetySensorOn();
	if (bRet == true)
	{
		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLiftpinSensorDown();
		if (!bRet)
		{
			bRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_LPMove(false);
			if (bRet == true)
			{
				//동작이 완료 되기까지 기다리는게 필요할 것 같은데....
				bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_LIFT_PIN_SENSOR_DOWN01);
			}

			if (bRet == false)
			{
				int nErrorAddress = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->GetLastCheckAddress();

				dTactEnd = GetTickCount();
				if (dTactEnd >= dTactStart)
					dTactTime = dTactEnd - dTactStart;
				else
					dTactTime = ((DWORD)0xFFFFFFFF) - (dTactStart - dTactEnd);
				strTactTime.Format(_T("%d"), dTactTime);
				strLog.Format(_T("CFunctionModuleEQStateChangeStep_LPDown NG, Address = M%d, T=%s msec"), bRet, strTactTime);

				m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);

				//CString strLog;
				//strLog.Format(_T("CFunctionModuleEQStateChangeStep_LPDown NG, Address = M%d"), nErrorAddress);
				//m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);

				return false;
			}
		}
	}
	
	dTactEnd = GetTickCount();
	if (dTactEnd >= dTactStart)
		dTactTime = dTactEnd - dTactStart;
	else
		dTactTime = ((DWORD)0xFFFFFFFF) - (dTactStart - dTactEnd);
	strTactTime.Format(_T("%d"), dTactTime);
	strLog.Format(_T("CFunctionModuleEQStateChangeStep_LPDown End, %d, T=%s msec"), bRet, strTactTime);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);

	if (bGlassExist == true)
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_READY_CHANGE_STEP2, false);
	}
	
	return true;
}

bool CFunctionModuleEQStateChangeStep_Safety::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_Safety Start."));

	//Safety 위치 이동 동작
	bool bRet = false;
	int nRet = VTM_RV_SUCCESSED;
	//1. Interlock Check - 문제 시 알람
	//2. 모든 축 이동
	bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLiftpinSensorDown();
	if(bRet != true)
	{
		//Life Pin down
		nRet = MoveLoaderZ(2);
		if (nRet != VTM_RV_SUCCESSED)
		{
			//알람
			CString strLog;
			strLog.Format(_T("CFunctionModuleEQStateChangeStep_Safety NG. MoveLoaderZ Done NG, Ret = %d."), nRet);
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(ERRO, strLog);

			m_pShareItem->SetFail(true, nRet);

			bRet = false;
		}
	}
	
	if(nRet == VTM_RV_SUCCESSED)
	{
		bRet = true;

		if (m_pShareItem->GetIOMapControlMgrPtr()->IsOutOfService() == true)
		{
			m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_BIT_STATE_READY, true);
			m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_BIT_STATE_LOAD, false);
			m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_BIT_STATE_MEASURE, false);
		}

		nRet = MoveSafetyAll();

		if (nRet != VTM_RV_SUCCESSED)
		{
			//알람
			CString strLog;
			strLog.Format(_T("CFunctionModuleEQStateChangeStep_Safety NG. MoveSafetyAll Done NG, Ret = %d."), nRet);
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(ERRO, strLog);

			m_pShareItem->SetFail(true, nRet);

			bRet = false;
		}
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_Safety End."));
	
	return bRet;
}

bool CFunctionModuleEQStateChangeStep_Idle::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_Idle."));

	bool bRet = false;
	bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsGlassSensorOn();
	if (bRet == true)
	{
		if (bRet == true)
		{
			bRet = m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_XIONIZER_INTERLOCK_ON_01, 0, 1);
		}

		if (bRet == true)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_XIONIZER_REMOTE_STATE_01, 1, 1, false, PLC_IO_ACTION_DONE_TIME_60);
		}
	}

	ResetEvent(m_pShareItem->GetStartEventHandle());
//	m_pShareItem->GoToInitStep();

	if(m_pShareItem->GetFail() == false)
		m_pShareItem->SetComplete(true);   // 동작 완료

	m_pShareItem->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_IDLE);	// 2023.08.09 add

	SetEvent(m_pShareItem->GetCompleteEventHandle()); // 시퀀스 완료 이벤트 SET

	return false;
}

bool CFunctionModuleEQStateChangeStep_ClampUp::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_ClampIn Start."));

#ifdef _USE_NOT
	//PLC IO 동작
	//1. Loader X가 Meas. 위치 인지 확인
	//2. Sample Stage가 Safety 인지 확인
	//3. Loader Z가 Down인지 확인
	//4. Clamp In
	//5. Clamp In 확인
	bool bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLoaderXMeasSensorOn();
	if (bRet)
	{
		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsSafetySensorOn();
		if (bRet)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLoaderZLoadSensorOn();
			if (bRet)
			{
				m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_EXP_B_FRONT_SAMPLE_CHUCK_CLAMP_FOLD, 0, 1);

				bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_IMP_B_FRONT_CLAMP_ON_01, 2, 2);
				if (bRet) bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_IMP_B_RIGHT_CLAMP_ON_01, 2, 4);
				if (bRet) bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_IMP_B_BACK_CLAMP_ON_01, 2, 2);
				if (bRet) bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_IMP_B_LEFT_CLAMP_ON_01, 2, 4);
				if (bRet == false)
				{
					int nErrorAddress = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->GetLastCheckAddress();

					CString strLog;
					strLog.Format(_T("CFunctionModuleEQStateChangeStep_ClampOut Clamp Unfold NG, Address = M%d"), nErrorAddress);
					m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);

					return false;
				}
			}
			else
			{
				m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LoaderZDown2Step LoaderZLoadSensorOn NG."));
				return false;
			}
		}
		else
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LoaderZDown2Step SafetySensorOn NG."));
			return false;
		}
	}
	else
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LoaderZDown2Step LoaderXMeasSensorOn NG."));
		return false;
	}

#endif // _USE_NOT

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_ClampIn End."));

	return true;
}

bool CFunctionModuleEQStateChangeStep_ClampDown::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_ClampIn Start."));


#ifdef _USE_NOT
	//PLC IO 동작
	//1. Loader X가 Meas. 위치 인지 확인
	//2. Sample Stage가 Safety 인지 확인
	//3. Loader Z가 Down인지 확인
	//4. Clamp In
	//5. Clamp In 확인
	bool bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLoaderXMeasSensorOn();
	if (bRet)
	{
		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsSafetySensorOn();
		if (bRet)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLoaderZLoadSensorOn();
			if (bRet)
			{
				m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_EXP_B_FRONT_SAMPLE_CHUCK_CLAMP_FOLD, 0, 1);

				bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_IMP_B_FRONT_CLAMP_ON_01, 2, 2);
				if (bRet) bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_IMP_B_RIGHT_CLAMP_ON_01, 2, 4);
				if (bRet) bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_IMP_B_BACK_CLAMP_ON_01, 2, 2);
				if (bRet) bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_IMP_B_LEFT_CLAMP_ON_01, 2, 4);
				if (bRet == false)
				{
					int nErrorAddress = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->GetLastCheckAddress();

					CString strLog;
					strLog.Format(_T("CFunctionModuleEQStateChangeStep_ClampOut Clamp Unfold NG, Address = M%d"), nErrorAddress);
					m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);

					return false;
				}
			}
			else
			{
				m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LoaderZDown2Step LoaderZLoadSensorOn NG."));
				return false;
			}
		}
		else
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LoaderZDown2Step SafetySensorOn NG."));
			return false;
		}
	}
	else
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LoaderZDown2Step LoaderXMeasSensorOn NG."));
		return false;
	}

#endif // _USE_NOT

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_ClampIn End."));

	return true;
}

bool CFunctionModuleEQStateChangeStep_Ready::Run()
{
	//PLC IO 동작
	//1. Sample Stage가 Safety 인지 확인
	//2. 상태변경 IO Set
	//3. 상태변경 완료 확인
	DWORD dTactStart, dTactEnd, dTactTime;
	dTactStart = GetTickCount();

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_Ready Start."));

	bool bRet = false;

	if (m_pShareItem->GetIOMapControlMgrPtr()->IsOutOfService() == true)
	{
		m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_BIT_STATE_READY, true);
		m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_BIT_STATE_LOAD, false);
		m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_BIT_STATE_MEASURE, false);

		bRet = true;

		Sleep(500);
	}
	else
	{
		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsSafetySensorOnStage();

		if (bRet == true)
		{
			if (m_pShareItem->GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(PLC_BIT_STATE_ALARM) == 1)
			{
				bRet = m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_ALRAM_RESET_STATUS, 1, 1);

				Sleep(200);
				m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_ALRAM_RESET_STATUS, 0, 1);
			}
		}

		if (bRet == true)
		{
			// 상태 변경
			bool bGlassExist = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsGlassSensorOn();

			bRet = m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_MEAS2READY_START, 1, 1);
		}
	}

	if (bRet == true)
	{
		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_STATE_READY, 1, 1, true, PLC_IO_ACTION_DONE_TIME_60);
	}

	m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_MEAS2READY_START, 0, 1);

	CString strTactTime, strLog;
	dTactEnd = GetTickCount();
	if (dTactEnd >= dTactStart)
		dTactTime = dTactEnd - dTactStart;
	else
		dTactTime = ((DWORD)0xFFFFFFFF) - (dTactStart - dTactEnd);
	strTactTime.Format(_T("%d"), dTactTime);
	strLog.Format(_T("CFunctionModuleEQStateChangeStep_Ready End, %d, T=%s msec"), bRet, strTactTime);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);

	return bRet;
}

bool CFunctionModuleEQStateChangeStep_Load::Run()
{
	//PLC IO 동작
	//1. Sample Stage가 Safety 인지 확인
	//2. 상태변경 IO Set
	//3. 상태변경 완료 확인
	DWORD dTactStart, dTactEnd, dTactTime;
	dTactStart = GetTickCount();

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_Load Start."));


	bool bGlassExist = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsGlassSensorOn();
	if (bGlassExist == true)
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_UNLOAD_READY_CHANGE_STEP10, true);
	}
	else
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_LOAD_READY_CHANGE_STEP2, true);
	}

	bool bRet = false;

	if (m_pShareItem->GetIOMapControlMgrPtr()->IsOutOfService() == true)
	{
		m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_BIT_STATE_READY, false);
		m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_BIT_STATE_LOAD, true);
		m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_BIT_STATE_MEASURE, false);

		bRet = true;

		Sleep(500);
	}
	else
	{
		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsSafetySensorOn();
	}

	if (bRet == true)
	{
		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_STATE_LOAD, 1, 1, true, PLC_IO_ACTION_DONE_TIME_60);	
	}

	CString strTactTime, strLog;
	dTactEnd = GetTickCount();
	if (dTactEnd >= dTactStart)
		dTactTime = dTactEnd - dTactStart;
	else
		dTactTime = ((DWORD)0xFFFFFFFF) - (dTactStart - dTactEnd);
	strTactTime.Format(_T("%d"), dTactTime);
	strLog.Format(_T("CFunctionModuleEQStateChangeStep_Load End, %d, T=%s msec"), bRet, strTactTime);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);

	if (bGlassExist == true)
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_UNLOAD_READY_CHANGE_STEP10, false);
	}
	else
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_LOAD_READY_CHANGE_STEP2, false);
	}

	return bRet;
}

bool CFunctionModuleEQStateChangeStep_Meas::Run()
{
	//PLC IO 동작
	//1. Sample Stage가 Safety 인지 확인
	//2. 상태변경 IO Set
	//3. 상태변경 완료 확인
	DWORD dTactStart, dTactEnd, dTactTime;
	dTactStart = GetTickCount();

	CString strLog;
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_Meas Start."));
	bool bRet = false;

	if (m_pShareItem->GetIOMapControlMgrPtr()->IsOutOfService() == true)
	{
		m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_BIT_STATE_READY, false);
		m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_BIT_STATE_LOAD, false);
		m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_BIT_STATE_MEASURE, true);

		bRet = true;

		Sleep(500);
	}
	else
	{
		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsSafetySensorOnStage();
	
		if (bRet == true)
		{
			if (m_pShareItem->GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(PLC_BIT_STATE_ALARM) == 1)
			{
				bRet = m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_ALRAM_RESET_STATUS, 1, 1);
				Sleep(200);
				m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_ALRAM_RESET_STATUS, 0, 1);
			}
		}

		if (bRet == true)
		{
			bool bGlassExist = false;
			bGlassExist = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsGlassSensorOn();

			if (bGlassExist == true)
			{
				bRet = m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_READY2MEAS_START, 1, 1);

				// MEAS STAGE
				if (bRet == true)
				{
					//// Clamp Up
					//CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_READY_CHANGE_STEP4, true);

					//CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_FIXTURE_LU01, 1, 1, true, PLC_IO_ACTION_DONE_TIME_60);

					//CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_READY_CHANGE_STEP4, false);

					// Air On
					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_READY_CHANGE_STEP4, true);

					CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsAirSensorOn();

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_READY_CHANGE_STEP4, false);

					// Clamp In
					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_READY_CHANGE_STEP5, true);

					CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_FIXTURE_FRONT_F01, 1, 1, true, PLC_IO_ACTION_DONE_TIME_60);

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_READY_CHANGE_STEP5, false);

					// Air Off
					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_READY_CHANGE_STEP6, true);

					CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsAirSensorOff();

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_READY_CHANGE_STEP6, false);

					// Vac On
					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_READY_CHANGE_STEP7, true);

					CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsVacuumSensorOn();

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_READY_CHANGE_STEP7, false);

					// Clamp Out
					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_READY_CHANGE_STEP8, true);

					CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_FIXTURE_FRONT_R01, 1, 1, true, PLC_IO_ACTION_DONE_TIME_60);

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_READY_CHANGE_STEP8, false);

					// Clamp Down
					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_READY_CHANGE_STEP9, true);

					CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_FIXTURE_FD01, 1, 1, true, PLC_IO_ACTION_DONE_TIME_60);

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_READY_CHANGE_STEP9, false);
				}

				if (bRet == true)
				{
					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_READY_CHANGE_STEP10, true);

					bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_STATE_MEASURE, 1, 1, true, PLC_IO_ACTION_DONE_TIME_60);

					CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_READY_CHANGE_STEP10, false);
				}

				m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_READY2MEAS_START, 0, 1);		// OFF
			}
			else
			{
				strLog.Format(_T("CFunctionModuleEQStateChangeStep_Meas Glass Not Exist"));
				m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);
			}
		}	
	}

	CString strTactTime;
	dTactEnd = GetTickCount();
	if (dTactEnd >= dTactStart)
		dTactTime = dTactEnd - dTactStart;
	else
		dTactTime = ((DWORD)0xFFFFFFFF) - (dTactStart - dTactEnd);
	strTactTime.Format(_T("%d"), dTactTime);
	strLog.Format(_T("CFunctionModuleEQStateChangeStep_Meas End, %d, T=%s msec"), bRet, strTactTime);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);

	return bRet;
}

bool CFunctionModuleEQStateChangeStep_X_RAY_ON1::Run()
{
	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_READY_CHANGE_STEP1, true);

	//PLC IO 동작
	//1. Sample Stage가 Safety 인지 확인
	//2. 상태변경 IO Set
	//3. 상태변경 완료 확인
	bool bRet = false;
	bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsGlassSensorOn();
	if (bRet == true)
	{
		DWORD dTactStart, dTactEnd, dTactTime;
		dTactStart = GetTickCount();

		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_X_RAY_ON Start."));

		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsEQSensorClose();
		if (bRet == true)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsSafetySensorOn();
		}

		if (bRet == true)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_XIONIZER_RUN_STATE_01, 1, 1, true, PLC_IO_ACTION_DONE_TIME_03);
		}

		if (bRet == true)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_XIONIZER_RUN_01, 1, 1, true, PLC_IO_ACTION_DONE_TIME_03);
		}

		if (bRet == true)
		{
			bRet = m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_XIONIZER_REMOTE_ON_01, 1, 1);
		}

		if (bRet == true)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_XIONIZER_REMOTE_STATE_01, 1, 1, true, PLC_IO_ACTION_DONE_TIME_60);
		}

		CString strTactTime, strLog;
		dTactEnd = GetTickCount();
		if (dTactEnd >= dTactStart)
			dTactTime = dTactEnd - dTactStart;
		else
			dTactTime = ((DWORD)0xFFFFFFFF) - (dTactStart - dTactEnd);
		strTactTime.Format(_T("%d"), dTactTime);
		strLog.Format(_T("CFunctionModuleEQStateChangeStep_X_RAY_ON End, %d, T=%s msec"), bRet, strTactTime);

		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);
	}
	else
	{
		bRet = true;
	}

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_READY_CHANGE_STEP1, false);

	return bRet;
}

bool CFunctionModuleEQStateChangeStep_X_RAY_OFF1::Run()
{
	//PLC IO 동작
	//1. Sample Stage가 Safety 인지 확인
	//2. 상태변경 IO Set
	//3. 상태변경 완료 확인

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_READY_CHANGE_STEP3, true);

	bool bRet = false;
	bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsGlassSensorOn();
	if (bRet == true)
	{
		DWORD dTactStart, dTactEnd, dTactTime;
		dTactStart = GetTickCount();

		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_X_RAY_OFF Start."));

		
		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsEQSensorClose();

		if (bRet == true)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsSafetySensorOn();
		}

		if (bRet == true)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_XIONIZER_RUN_STATE_01, 1, 1, true, PLC_IO_ACTION_DONE_TIME_03);
		}

		if (bRet == true)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_XIONIZER_RUN_01, 1, 1, true, PLC_IO_ACTION_DONE_TIME_03);
		}
		
		if (bRet == true)
		{
			bRet = m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_XIONIZER_REMOTE_ON_01, 0, 1);
		}

		if (bRet == true)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_XIONIZER_REMOTE_STATE_01, 1, 1, false, PLC_IO_ACTION_DONE_TIME_60);
		}

		CString strTactTime, strLog;
		dTactEnd = GetTickCount();
		if (dTactEnd >= dTactStart)
			dTactTime = dTactEnd - dTactStart;
		else
			dTactTime = ((DWORD)0xFFFFFFFF) - (dTactStart - dTactEnd);
		strTactTime.Format(_T("%d"), dTactTime);
		strLog.Format(_T("CFunctionModuleEQStateChangeStep_X_RAY_OFF End, %d, T=%s msec"), bRet, strTactTime);

		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);	
	}
	else
	{
		bRet = true;
	}

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_READY_CHANGE_STEP3, false);

	return bRet;
}

bool CFunctionModuleEQStateChangeStep_X_RAY_ON2::Run()
{
	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_UNLOAD_READY_CHANGE_STEP7, true);

	//PLC IO 동작
	//1. Sample Stage가 Safety 인지 확인
	//2. 상태변경 IO Set
	//3. 상태변경 완료 확인
	bool bRet = false;
	bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsGlassSensorOn();
	if (bRet == true)
	{
		DWORD dTactStart, dTactEnd, dTactTime;
		dTactStart = GetTickCount();

		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_X_RAY_ON2 Start."));

		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsEQSensorClose();
		if (bRet == true)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsSafetySensorOn();
		}

		if (bRet == true)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_XIONIZER_RUN_STATE_01, 1, 1, true, PLC_IO_ACTION_DONE_TIME_03);
		}

		if (bRet == true)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_XIONIZER_RUN_01, 1, 1, true, PLC_IO_ACTION_DONE_TIME_03);
		}

		if (bRet == true)
		{
			bRet = m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_XIONIZER_REMOTE_ON_01, 1, 1);
		}

		if (bRet == true)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_XIONIZER_REMOTE_STATE_01, 1, 1, true, PLC_IO_ACTION_DONE_TIME_60);
		}

		//if (bRet == true)
		//{
		//	Sleep(2000);
		//}

		CString strTactTime, strLog;
		dTactEnd = GetTickCount();
		if (dTactEnd >= dTactStart)
			dTactTime = dTactEnd - dTactStart;
		else
			dTactTime = ((DWORD)0xFFFFFFFF) - (dTactStart - dTactEnd);
		strTactTime.Format(_T("%d"), dTactTime);
		strLog.Format(_T("CFunctionModuleEQStateChangeStep_X_RAY_ON2 End, %d, T=%s msec"), bRet, strTactTime);

		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);
	}
	else
	{
		bRet = true;
	}

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_UNLOAD_READY_CHANGE_STEP7, false);

	return bRet;
}

bool CFunctionModuleEQStateChangeStep_X_RAY_OFF2::Run()
{
	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_UNLOAD_READY_CHANGE_STEP9, true);

	//PLC IO 동작
	//1. Sample Stage가 Safety 인지 확인
	//2. 상태변경 IO Set
	//3. 상태변경 완료 확인
	bool bRet = false;
	bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsGlassSensorOn();
	if (bRet == true)
	{
		DWORD dTactStart, dTactEnd, dTactTime;
		dTactStart = GetTickCount();

		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_X_RAY_OFF Start."));

		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsEQSensorClose();

		if (bRet == true)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsSafetySensorOn();
		}

		//if (bRet == true)
		//{
		//	Sleep(30000);
		//}
		
		if (bRet == true)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_XIONIZER_RUN_STATE_01, 1, 1, true, PLC_IO_ACTION_DONE_TIME_03);
		}

		if (bRet == true)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_XIONIZER_RUN_01, 1, 1, true, PLC_IO_ACTION_DONE_TIME_03);
		}

		if (bRet == true) 
		{
			bRet = m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_XIONIZER_REMOTE_ON_01, 0, 1);
		}

		if (bRet == true)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_XIONIZER_REMOTE_STATE_01, 1, 1, false, PLC_IO_ACTION_DONE_TIME_60);
		}

		CString strTactTime, strLog;
		dTactEnd = GetTickCount();
		if (dTactEnd >= dTactStart)
			dTactTime = dTactEnd - dTactStart;
		else
			dTactTime = ((DWORD)0xFFFFFFFF) - (dTactStart - dTactEnd);
		strTactTime.Format(_T("%d"), dTactTime);
		strLog.Format(_T("CFunctionModuleEQStateChangeStep_X_RAY_OFF End, %d, T=%s msec"), bRet, strTactTime);

		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);
	}
	else
	{
		bRet = true;
	}

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_UNLOAD_READY_CHANGE_STEP9, false);

	return bRet;
}

bool CFunctionModuleEQStateChangeStep_Ready2::Run()
{
	//PLC IO 동작
	//1. Sample Stage가 Safety 인지 확인
	//2. 상태변경 IO Set
	//3. 상태변경 완료 확인
	DWORD dTactStart, dTactEnd, dTactTime;
	dTactStart = GetTickCount();

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_Ready2 Start."));

	bool bRet = false;

	if (m_pShareItem->GetIOMapControlMgrPtr()->IsOutOfService() == true)
	{
		m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_BIT_STATE_READY, true);
		m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_BIT_STATE_LOAD, false);
		m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_BIT_STATE_MEASURE, false);

		bRet = true;

		Sleep(500);
	}
	else
	{
		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsSafetySensorOnStage();

		if (bRet == true)
		{
			if (m_pShareItem->GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(PLC_BIT_STATE_ALARM) == 1)
			{
				bRet = m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_ALRAM_RESET_STATUS, 1, 1);

				Sleep(200);
				m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_ALRAM_RESET_STATUS, 0, 1);
			}
		}

		if (bRet == true)
		{
			// 상태 변경
			bRet = m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_MEAS2READY_START, 1, 1);

			// UNLOAD STAGE
			if (bRet == true)
			{
				// Clamp Up
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_UNLOAD_READY_CHANGE_STEP1, true);

				CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_FIXTURE_FU01, 1, 1, true, PLC_IO_ACTION_DONE_TIME_60);

				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_UNLOAD_READY_CHANGE_STEP1, false);

				// Vac Off
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_UNLOAD_READY_CHANGE_STEP2, true);
				
				CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsVacuumSensorOff();

				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_UNLOAD_READY_CHANGE_STEP2, false);

				// Air On
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_UNLOAD_READY_CHANGE_STEP3, true);
				
				CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsAirSensorOn();

				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_UNLOAD_READY_CHANGE_STEP3, false);

				// Clamp In
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_UNLOAD_READY_CHANGE_STEP4, true);

				CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_FIXTURE_FRONT_F01, 1, 1, true, PLC_IO_ACTION_DONE_TIME_60);

				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_UNLOAD_READY_CHANGE_STEP4, false);

				// Air Off
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_UNLOAD_READY_CHANGE_STEP5, true);

				CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsAirSensorOff();

				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_UNLOAD_READY_CHANGE_STEP5, false);

				// Clamp Out
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_UNLOAD_READY_CHANGE_STEP6, true);

				CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_FIXTURE_FRONT_R01, 1, 1, true, PLC_IO_ACTION_DONE_TIME_60);

				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_UNLOAD_READY_CHANGE_STEP6, false);


			}
		}
	}

	if (bRet == true)
	{
		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_STATE_READY, 1, 1, true, PLC_IO_ACTION_DONE_TIME_60);
	}

	m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_MEAS2READY_START, 0, 1);

	CString strTactTime, strLog;
	dTactEnd = GetTickCount();
	if (dTactEnd >= dTactStart)
		dTactTime = dTactEnd - dTactStart;
	else
		dTactTime = ((DWORD)0xFFFFFFFF) - (dTactStart - dTactEnd);
	strTactTime.Format(_T("%d"), dTactTime);
	strLog.Format(_T("CFunctionModuleEQStateChangeStep_Ready2 End, %d, T=%s msec"), bRet, strTactTime);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);

	return bRet;
}

bool CFunctionModuleEQStateChangeStep_FixtureUp::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_FixtureUp Start."));

	bool bRet = false;

	bool bGlassExist = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsGlassSensorOn();
	if (bGlassExist == true)
	{
		bRet = m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_W_FIXTURE_UPDOWN_ALL, 1, 1);
		if (bRet == true)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_FIXTURE_FU01, 8, 1, true, PLC_IO_ACTION_DONE_TIME_60);
		}

		if (bRet == true)
		{
			bRet = m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_W_FIXTURE_FWD_ALL, 1, 1);
		}
		
		if (bRet == true)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_FIXTURE_FRONT_F01, 8, 1, true, PLC_IO_ACTION_DONE_TIME_60);
		}
	}
	else
	{
		bRet = true;
	}


	return bRet;
}

bool CFunctionModuleEQStateChangeStep_FixtureDown::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_FixtureDown Start."));

	bool bRet = false;

	bool bGlassExist = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsGlassSensorOn();
	if (bGlassExist == true)
	{
		bRet = m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_W_FIXTURE_FWD_ALL, 0, 1);

		if (bRet == true)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_FIXTURE_FRONT_R01, 8, 1, true, PLC_IO_ACTION_DONE_TIME_60);
		}

		if (bRet == true)
		{
			bRet = m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_W_FIXTURE_UPDOWN_ALL, 0, 1);
		}
		
		if (bRet == true)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_FIXTURE_FD01, 8, 1, true, PLC_IO_ACTION_DONE_TIME_60);
		}
	}
	else
	{
		bRet = true;
	}

	return bRet;
}

bool CFunctionModuleEQStateChangeStep_LPUpPre::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LPUpPre Start."));

	DWORD dTactStart, dTactEnd, dTactTime;
	dTactStart = GetTickCount();
	CString strTactTime, strLog;

	bool bGlassExist = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsGlassSensorOn();
	if (bGlassExist == true)
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_UNLOAD_READY_CHANGE_STEP8, true);
	}
	else
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_LOAD_READY_CHANGE_STEP1, true);
	}

	//PLC IO 동작
	//1. LP가 Up이 아닐 경우 Up
	//2. Up 동작 확인
	bool bRet = false;
	if (m_pShareItem->GetMaintControlMgrPtr()->IsOutOfServiceMC() == true)
	{
		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLiftpinSensorUp();
		if (!bRet)
		{
			bRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_LPPreMove();

			if (bRet == false)
			{
				int nErrorAddress = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->GetLastCheckAddress();

				dTactEnd = GetTickCount();
				if (dTactEnd >= dTactStart)
					dTactTime = dTactEnd - dTactStart;
				else
					dTactTime = ((DWORD)0xFFFFFFFF) - (dTactStart - dTactEnd);
				strTactTime.Format(_T("%d"), dTactTime);
				strLog.Format(_T("CFunctionModuleEQStateChangeStep_LPUpPre NG, Address = M%d, T=%s msec"), nErrorAddress, strTactTime);

				m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);

				//CString strLog;
				//strLog.Format(_T("CFunctionModuleEQStateChangeStep_LPUpPre NG, Address = M%d"), nErrorAddress);
				//m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);

				return false;
			}
		}
	}
	else
	{
		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsSafetySensorOn();
		{
			if (bRet == true)
			{
				bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLiftpinSensorUp();
				if (!bRet)
				{
					bRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_LPPreMove();

					if (bRet == false)
					{
						int nErrorAddress = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->GetLastCheckAddress();

						dTactEnd = GetTickCount();
						if (dTactEnd >= dTactStart)
							dTactTime = dTactEnd - dTactStart;
						else
							dTactTime = ((DWORD)0xFFFFFFFF) - (dTactStart - dTactEnd);
						strTactTime.Format(_T("%d"), dTactTime);
						strLog.Format(_T("CFunctionModuleEQStateChangeStep_LPUpPre NG, Address = M%d, T=%s msec"), nErrorAddress, strTactTime);

						m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);

						//CString strLog;
						//strLog.Format(_T("CFunctionModuleEQStateChangeStep_LPUpPre NG, Address = M%d"), nErrorAddress);
						//m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);

						return false;
					}
				}
			}
		}
	}

	dTactEnd = GetTickCount();
	if (dTactEnd >= dTactStart)
		dTactTime = dTactEnd - dTactStart;
	else
		dTactTime = ((DWORD)0xFFFFFFFF) - (dTactStart - dTactEnd);
	strTactTime.Format(_T("%d"), dTactTime);
	strLog.Format(_T("CFunctionModuleEQStateChangeStep_LPUpPre End, %d, T=%s msec"), bRet, strTactTime);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog); 

//	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_LPUpPre End."));

	if (bGlassExist == true)
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_UNLOAD_READY_CHANGE_STEP8, false);
	}
	else
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_LOAD_READY_CHANGE_STEP1, false);
	}

	return bRet;
}

bool CFunctionModuleEQStateChangeStep_FixtureUpBack::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CFunctionModuleEQStateChangeStep_FixtureUpBack Start."));

	bool bRet = false;

	bool bGlassExist = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsGlassSensorOn();
	if (bGlassExist == true)
	{
		bRet = m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_W_FIXTURE_UPDOWN_ALL, 1, 1);
		if (bRet == true)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_FIXTURE_FU01, 8, 1, true, PLC_IO_ACTION_DONE_TIME_60);
		}

		if (bRet == true)
		{
			bRet = m_pShareItem->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_W_FIXTURE_FWD_ALL, 0, 1);
		}

		if (bRet == true)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_FIXTURE_FRONT_R01, 8, 1, true, PLC_IO_ACTION_DONE_TIME_60);
		}
	}
	else
	{
		bRet = true;
	}


	return bRet;
}