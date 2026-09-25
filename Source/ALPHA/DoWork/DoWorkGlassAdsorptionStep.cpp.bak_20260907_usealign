#include "pch.h"
#include "DoWorkGlassAdsorptionStep.h"

#include "DoWorkGlassSequenceImpl.h"
#include "..\SystemParamShare.h"
#include "..\Manager\SystemConfigMgr.h"

//#include "RecipeEditMgr.h"
#include "..\Manager\SequenceManager.h"
#include "..\FunctionModule\FunctionModuleEQStateChange.h"
#include "..\FunctionModule\FunctionModuleEQState.h"
#include "..\FunctionModule\FunctionModuleProbeACC.h"
#include "..\Manager\MCCLogMgr.h"
#include "..\Manager\MaintControlMgr.h"
#include "Recipe/RecipeEditMgr.h"
#include "DoWorkGlassSequence.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 시작
bool CDoWorkGlassAdsorptionStep_None::Run()
{
	// init
	m_pShareItem->SetComplete(false);
	m_pShareItem->SetAlarmOn(false);
	m_pShareItem->SetFail(false);
	m_pShareItem->SetStopUser(false);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAdsorptionStep_None Start."));

	m_pShareItem->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_PROCESS);	// 2023.08.09 add

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_READY, true);

	

    return true;
}

// 종료
bool CDoWorkGlassAdsorptionStep_Idle::Run()
{
	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_Idle, NULL);

	// 완료
	ResetEvent(m_pShareItem->GetStartEventHandle()); 

	if (m_pShareItem->GetFail() != true)
	{
		m_pShareItem->SetComplete(true);   // 동작 완료
	}

	SetEvent(m_pShareItem->GetCompleteEventHandle()); // 시퀀스 완료 이벤트 SET

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_READY, false);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAdsorptionStep_Idle"));

	m_pShareItem->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_IDLE);	// 2023.08.09 add

	return false;
}

bool CDoWorkGlassAdsorptionStep_GetPreAlignParam::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAdsorptionStep_GetPreAlignParam Start."));
	
	//Recipe parameter를 호출하여 PreAlign 사용 시 보정값 적용 필요함
	//1. 사용 확인
	//1.1 사용하지 않을 시 return true;
	//1.2 사용 시 성공 확인 후 적용
	//아래는 참고용(수정필요)
	//m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeDataPtr()->GetParam_CurrentMeasArea();
	//m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeDataPtr()->SetParam_CurrentMeasArea(1);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAdsorptionStep_GetPreAlignParam End."));
	
	return true;
}

bool CDoWorkGlassAdsorptionStep_Move::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAdsorptionStep_Move Start."));

	int nRet = VTM_RV_SUCCESSED;
	bool bRet = false;
	
	m_pShareItem->GetMaintControlMgrPtr()->ResetAlign();	// jsyoon 2022.06.24 add

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_READY_CHK_STEP1, true);

	bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLoaderArmSensorOn();
	if (bRet == true)
	{
		nRet = PLC_ARM_SENSOR_DETECT;
	}

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_READY_CHK_STEP1, false);

	if (nRet == VTM_RV_SUCCESSED)
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_READY_CHK_STEP2, true);

		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsShutterSensorClose();
		if (bRet != true)
		{
			nRet = PLC_LOADER_SHUTTER_UP_ERROR;
		}

		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_READY_CHK_STEP2, false);
	}

	if (nRet == VTM_RV_SUCCESSED)
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_READY_CHK_STEP3, true);

		int nStatus = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->GetEQState();
		if (nStatus != enumEQStateMeas)
		{
			bool bSafety = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsSafetySensorOnStage();
			if (bSafety != true)
			{
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

				if (nRet == VTM_RV_SUCCESSED)
				{
					double dSafetyStageX = 0.0;
					m_pShareItem->GetPositionConfigMgrPtr()->GetSafetyPosition(&dSafetyStageX, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);

					nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_X, dSafetyStageX, true);
				}

				if (nRet == VTM_RV_SUCCESSED)
				{
					bSafety = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsSafetySensorOnStage();
					if (bSafety == true)
					{
						nRet = VTM_RV_SUCCESSED;
					}
					else
					{
						nRet = PLC_CANNOT_DETECT_SAFETY_SENSOR_X;
					}
				}	
			}
			else
			{
				nRet = VTM_RV_SUCCESSED;
			}
		}
		else
		{
			nRet = VTM_RV_SUCCESSED;
		}

		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_MEAS_READY_CHK_STEP3, false);
	}


	if (nRet == VTM_RV_SUCCESSED)
	{
		bRet = true;

		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAdsorptionStep_Move End."));
	}
	else
	{
		bRet = false;
		m_pShareItem->SetFail(true, nRet);

		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAdsorptionStep_Move NG."));
	}

	return bRet;
}

bool CDoWorkGlassAdsorptionStep_ChangeMeasStatus::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAdsorptionStep_ChangeMeasStatus Start."));


	int nStatus = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->GetEQState();

	// jsyoon 2022.08.23 add
	bool bGlassExist = false;
	bGlassExist = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsGlassSensorOn();
	if (bGlassExist != true)
	{
		if (nStatus == enumEQStateReady)
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAdsorptionStep_ChangeMeasStatus Skip."));
			return true;
		}
	}
	///

	if (nStatus == enumEQStateReady)
	{
		CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->SetEQStatusChangeStep(enumEQSTATUS_R2M);
		CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Start();
	}
	else if (nStatus == enumEQStateLoad)
	{
		CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->SetEQStatusChangeStep(enumEQSTATUS_L2M);
		CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Start();
	}
	else if (nStatus == enumEQStateAbnormal)
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAdsorptionStep_ChangeMeasStatus NG(enumEQStateAbnormal)."));

		m_pShareItem->SetFail(true, PLC_STATE_UNKNOWN_ERROR);

		return false;
	}

	int nRet = VTM_RV_FAILED;
	bool bRet = false;
	if (nStatus == enumEQStateReady || nStatus == enumEQStateLoad)
	{
		int nLoop = 0;
		Sleep(500);

		do
		{
		//	Sleep(500);
			Sleep(100);

			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Is_Complete();
			if (bRet == true)
			{
				nRet = VTM_RV_SUCCESSED;
				break;
			}

			nLoop++;

		} while (nLoop < 1000 /*200*/);
	}
	else if (nStatus == enumEQStateMeas)
	{
		nRet = VTM_RV_SUCCESSED;
	}

	if (nRet == VTM_RV_SUCCESSED)
	{
		bRet = true;

		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAdsorptionStep_ChangeMeasStatus End."));
	}
	else
	{
		bRet = false;
		m_pShareItem->SetFail(true, nRet);

		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAdsorptionStep_ChangeMeasStatus NG."));
	}

	return bRet;
}

bool CDoWorkGlassAdsorptionStep_CheckMeasStatus::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAdsorptionStep_CheckMeasStatus Start."));
	
	// jsyoon 2022.08.23 add
	bool bGlassExist = false;
	bGlassExist = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsGlassSensorOn();
	if (bGlassExist != true)
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAdsorptionStep_CheckMeasStatus Skip."));
		return true;
	}

	int nRet = VTM_RV_FAILED;
	bool bRet = false;
	int nStatus = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->GetEQState();
	if (nStatus != enumEQStateMeas)
	{
		int nLoop = 0;
		do
		{
			Sleep(100);

			nStatus = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->GetEQState();
			if (nStatus == enumEQStateMeas)
			{
				nRet = VTM_RV_SUCCESSED;
				break;
			}

			nLoop++;

		} while (nLoop < 100);
	}
	else
	{
		nRet = VTM_RV_SUCCESSED;
	}
		
	if (nRet != VTM_RV_SUCCESSED)
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAdsorptionStep_CheckMeasStatus NG."));

		m_pShareItem->SetFail(true, PLC_STATE_NOT_MEASURE);
		return false;
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAdsorptionStep_CheckMeasStatus End."));
	
	return true;
}



bool CDoWorkGlassAdsorptionStep_ProbeAlign::Run()
{
	bool bRet = true;
	int nRet = VTM_RV_SUCCESSED;

	if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsGlassSensorOn() == true)
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAdsorptionStep_ProbeAlign Start."));

		// LYS 2026-06-26 OCP: only P5 (MEAS)
		int nAdsorpProbeStart = (m_pShareItem->GetSystemConfigMgrPtr() && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP()) ? df_PROBE_MEAS : df_PROBE_AGING1;
		for (int iProbe = nAdsorpProbeStart; iProbe <= df_PROBE_MEAS; iProbe++)
		{
			if (m_pShareItem->IfPauseEvt_WaitResumeEvt() != true)
			{
				bRet = false;
				break;
			}

			// Probe exist check
			if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsProbeExistSensorOn(iProbe) == true)
			{
				if (CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->GetProbeAlign(iProbe - 1) == true)
				{
					continue;
				}

				if (m_pShareItem->GetStopUser() == true)	// 2023.08.09 add
				{
					break;
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

					CDoWorkGlassSequence* pDoWorkGlassSeq = CSequenceManager::GetInstance()->GeDoWorkGlassSequence();
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
					bRet = false;
					break;
				}
			}
		}

		if (bRet == true)
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAdsorptionStep_ProbeAlign OK."));
		}
		else
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAdsorptionStep_ProbeAlign NG."));
			m_pShareItem->SetFail(true, nRet);
		}
	}

	return bRet;
}