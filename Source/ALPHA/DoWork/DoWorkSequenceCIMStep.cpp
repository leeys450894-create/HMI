#include "pch.h"
#include "DoWorkSequenceCIMStep.h"
#include "DoWorkGlassSequenceImpl.h"
#include "..\SystemParamShare.h"
#include "CommTcpIpObjectCIM.h"
#include "Manager/CommTcpIpFactoryMgr.h"
#include "Manager/ShareMemoryMgr.h"
#include "Manager/MaintControlMgr.h"
#include "Manager/SequenceManager.h"
#include "Manager/SystemConfigMgr.h"
#include "Manager/SeqTactTimeMgr.h"
#include "FunctionModule/FunctionModuleEQState.h"
#include "FunctionModule/FunctionModuleEQStateChange.h"
#include "D_SelectMeasOrUnload.h"
#include "Alarm/AlarmController.h"
#include "Recipe/RecipeEditMgr.h"	// LYS 2026-09-28 EFA
#include "DoWork/AgingCycleInfo.h"	// LYS 2026-09-28 EFA

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

bool CDoWorkSequenceCIMStep::GetComplete()
{
	return m_pShareItem->GetComplete();
}

bool CDoWorkSequenceCIMStep_None::Run()
{
	// init
	m_pShareItem->SetComplete(false);
	m_pShareItem->SetAlarmOn(false);
	m_pShareItem->SetFail(false);
	m_pShareItem->SetStopUser(false);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_None."));

	m_pShareItem->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_PROCESS);	// 2023.08.09 add


	/*m_pShareItem->SetAlarmOn(false);
	m_pShareItem->SetComplete(false);
	m_pShareItem->SetFail(false);*/

	m_pShareItem->GetShareMemoryMgrPtr()->SetSys_Validation(false);
	m_pShareItem->GetSystemConfigMgrPtr()->SetUseCIMAlarmNotReport(false);

	bool bRet = true;

	return bRet;
}

bool CDoWorkSequenceCIMStep_Idle::Run()
{
	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_Idle, NULL);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_Idle"));

	if (m_pShareItem->GetFail() != true)
	{
		m_pShareItem->SetComplete(true);   // 동작 완료
	}

	m_pShareItem->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_IDLE);	// 2023.08.09 add

	// 완료
	ResetEvent(m_pShareItem->GetStartEventHandle());

	SetEvent(m_pShareItem->GetCompleteEventHandle()); // 시퀀스 완료 이벤트 SET
	return false;
}

bool CDoWorkSequenceCIMStep_Initial::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_Initial Start."));

	int nRet = VTM_RV_SUCCESSED;
	nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->SendMainStatus(true);

	if (nRet == VTM_RV_SUCCESSED)
	{
		nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->WaitForCIMResponse_Action(dfAT_1_OK, TO_CIM_AT1, df_WAIT_TIMEOUT);
	}

	if (nRet == VTM_RV_SUCCESSED)
	{

		m_pShareItem->GetShareMemoryMgrPtr()->SetSys_MachineMode(df_HOST_ONLINE_REMOTE);
		nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->WaitForCIMResponse_Action(dfEQPSTART, TO_CIM_EQPSTART, df_WAIT_TIMEOUT);
	}

	if (nRet == VTM_RV_SUCCESSED)
	{
		nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Process_EQPStart();
	}

	if (nRet == VTM_RV_SUCCESSED)
	{
		nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->WaitForCIMResponse_Action(dfALMACL_OK, TO_CIM_ALMACL, df_WAIT_TIMEOUT_LONG);
	}

	if (nRet == VTM_RV_SUCCESSED)
	{
		nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send(dfTIMEREQ);
	}

	if (nRet == VTM_RV_SUCCESSED)
	{
		nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->WaitForCIMResponse_Action(dfTIMESET, TO_CIM_TIMEREQ, df_WAIT_TIMEOUT);
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_Initial End"));

	if( nRet == VTM_RV_SUCCESSED)
		return true;
	else
		return false;
}

bool CDoWorkSequenceCIMStep_Initial2::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_Initial2 Start."));

	int nRet = VTM_RV_FAILED;

	//
	if (m_pShareItem->GetShareMemoryMgrPtr()->GetSys_MachineMode() == df_HOST_ONLINE_REMOTE)
	{
		nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_STS0104();
		if (nRet == VTM_RV_SUCCESSED)
		{
			Sleep(100);
			nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_STS0204();
		}

		if (nRet == VTM_RV_SUCCESSED)
		{
			//ORG
			nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->WaitForORG();
			if (nRet == VTM_RV_SUCCESSED)
			{
				// HOMING
			}

			if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsSafetySensorOn() != true)
			{
				CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->SetEQStatusChangeStep(enumEQSTATUS_SAFETY);
				CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Start();

				int nLoop = 0;
				bool bRet = false;
				Sleep(500);
				nRet = VTM_RV_FAILED;
				do
				{
					Sleep(100);

					bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Is_Complete();
					if (bRet == true)
					{
						nRet = VTM_RV_SUCCESSED;
						break;
					}

					nLoop++;

				} while (nLoop < 1000);
			}
		}

		if (nRet == VTM_RV_SUCCESSED)
		{
			nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_STS0304();
		}

		if (nRet == VTM_RV_SUCCESSED)
		{
			if (m_pShareItem->GetMaintControlMgrPtr()->IsGlassExist() == true)
			{
				CWnd* pW = CWindowEventHandler::GetInstance()->GetHandleWnd(enumWndID_FRAME);

				CD_SelectMeasOrUnload dlg(pW);
				if (dlg.DoModal() == IDOK)
				{
					if (dlg.m_nMode == 1)	//Meas Mode
					{
						m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->SetUserMode(3); 
						m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->SetUserBypassGlass(false);
					}
					else if (dlg.m_nMode == 2)	//Unload Mode	// abort
					{
						m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->SetUserMode(2);
						m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->SetUserBypassGlass(true);
					}
					else
					{
						nRet = VTM_RV_FAILED;
					}
				}	
			}
			else
			{
				m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->SetUserMode(0);
			}
		}
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_Initial2 End"));

	if (nRet == VTM_RV_SUCCESSED)
		return true;
	else
		return false;
}


bool CDoWorkSequenceCIMStep_Loading::Run()
{
	if (m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->GetUserMode() == 3)
	{
		return true;
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_Loading Start."));

	int nRet = VTM_RV_FAILED;

	//
	if (m_pShareItem->GetShareMemoryMgrPtr()->GetSys_MachineMode() == df_HOST_ONLINE_REMOTE)
	{
		// Move Safety
		if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsSafetySensorOn() != true)
		{
			CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->SetEQStatusChangeStep(enumEQSTATUS_SAFETY);
			CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Start();

			int nLoop = 0;
			bool bRet = false;

			Sleep(500);
			nRet = VTM_RV_FAILED;
			do
			{
				Sleep(100);

				bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Is_Complete();
				if (bRet == true)
				{
					nRet = VTM_RV_SUCCESSED;
					break;
				}

				nLoop++;

			} while (nLoop < 1000);
		}
		else
		{
			nRet = VTM_RV_SUCCESSED;
		}

		// Moving Loading Pos
		if (nRet == VTM_RV_SUCCESSED)
		{
	//		if (m_pShareItem->GetMaintControlMgrPtr()->IsGlassExist() != true) // Glass Exist
			{
				long lStep = enumEQSTATUS_NONE;

			 	int nStatus = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->GetEQState();

				if (nStatus == enumEQStateReady)
				{
					lStep = enumEQSTATUS_R2L;
				}
				else if (nStatus == enumEQStateMeas)
				{
					lStep = enumEQSTATUS_M2L;
				}
				else if (nStatus == enumEQStateLoad)
				{
					lStep = enumEQSTATUS_NONE;
				}
				else
				{
					lStep = enumEQSTATUS_NONE;
					nRet = PLC_STATE_NOT_LOAD;
				}

				if (lStep != enumEQSTATUS_NONE)
				{
					CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->SetEQStatusChangeStep(lStep);
					CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Start();

					Sleep(500);

					int nLoop = 0;
					bool bRet = false;
					nRet = VTM_RV_FAILED;
					do
					{
						Sleep(100);

						bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Is_Complete();
						if (bRet == true)
						{
							nRet = VTM_RV_SUCCESSED;
							break;
						}


						nLoop++;

					} while (nLoop < 1000);
				}
			}
		}
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_Loading End"));

	if (nRet == VTM_RV_SUCCESSED)
		return true;
	else
		return false;
}

bool CDoWorkSequenceCIMStep_CheckLoadReq::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_CheckLoadReq Start."));

	int nRet = VTM_RV_FAILED;

	/*if (m_pSystem->bGlassLoadState)
	{
		Alarm_SetAlarm(JOB_LOAD_SEQ_GLASS_DETECTED);
		return;
	}*/

	//if (m_pSystem->stInterLock.m_bGlassExist)
	//{
	//	m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_LDREQ(false);
	//	Alarm_SetAlarm(PLC_GLASS_EXIST_SENSOR_ERROR);	//ksk 20150506 add
	//}

	nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->CanDoStartPRCS();
	if (nRet == VTM_RV_SUCCESSED)
	{
	//	CString strLog;
	//	strLog.Format("Glass 투입");
	//	gLog(m_pSystem->hLog, strLog, LOG_LEVEL_T3);

		nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_LDREQ(true);
	}
	else
	{
		nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_LDREQ(false);

		if (nRet == VTM_RV_SUCCESSED)
			nRet = CIM_RV_HANDSHAKE_FAIL;
	//	gLog(m_pSystem->hLog, "LDREQ But HandShakeComplete FALSE");
	//	Alarm_SetAlarm(CIM_RV_HANDSHAKE_FAIL);
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_CheckLoadReq End"));

	if (nRet == VTM_RV_SUCCESSED)
		return true;
	else
		return false;
}

bool CDoWorkSequenceCIMStep_ChangeLoad::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_ChangeLoad Start."));

	m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeST(1, _T("ChangeToLoad"), 0);

	int nRet = VTM_RV_FAILED;

	int nStatus = enumEQStateAbnormal;

	// Move Safety
	if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsSafetySensorOn() != true)
	{
		CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->SetEQStatusChangeStep(enumEQSTATUS_SAFETY);
		CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Start();

		Sleep(500);

		int nLoop = 0;
		bool bRet = false;
		nRet = VTM_RV_FAILED;
		do
		{
			Sleep(100);

			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Is_Complete();
			if (bRet == true)
			{
				nRet = VTM_RV_SUCCESSED;
				break;
			}

			nLoop++;

		} while (nLoop < 1000);
	}
	else
	{
		nRet = VTM_RV_SUCCESSED;
	}

	// Change Load
	if (nRet == VTM_RV_SUCCESSED)
	{
		long lStep = enumEQSTATUS_NONE;
		nStatus = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->GetEQState();
		if (nStatus == enumEQStateReady)
		{
			lStep = enumEQSTATUS_R2L;
		}
		else if (nStatus == enumEQStateMeas)
		{
			lStep = enumEQSTATUS_M2L;
		}
		else if (nStatus == enumEQStateLoad)
		{
			lStep = enumEQSTATUS_NONE;
		}
		else
		{
			nRet = VTM_RV_FAILED;
		}

		if (lStep != enumEQSTATUS_NONE)
		{
			CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->SetEQStatusChangeStep(lStep);
			CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Start();

			Sleep(500);

			int nLoop = 0;
			bool bRet = false;
			nRet = VTM_RV_FAILED;
			do
			{
				Sleep(100);

				bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Is_Complete();
				if (bRet == true)
				{
					nRet = VTM_RV_SUCCESSED;
					break;
				}

				nLoop++;

			} while (nLoop < 5000);  
		}
	}

	m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(1, _T("ChangeToLoad"), 0);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_ChangeLoad End"));

	if (nRet == VTM_RV_SUCCESSED)
		return true;
	else
		return false;
}

bool CDoWorkSequenceCIMStep_LoadReq::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_LoadReq Start."));

	int nRet = VTM_RV_FAILED;

	nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_STS2105();

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_LoadReq End"));

	if (nRet == VTM_RV_SUCCESSED)
		return true;
	else
		return false;
}

bool CDoWorkSequenceCIMStep_PPIDREQ::Run()
{
	int iUserMode = 0;
	iUserMode = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->GetUserMode();
	if (iUserMode != 1)
	{
		return true;
	}

	// jsyoon 2022.06.13 add
	m_pShareItem->GetSeqTactTimeMgrPtr()->SetDataClear();
	m_pShareItem->GetSeqTactTimeMgrPtr()->SetTotalSeqTimeST();

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_PPIDREQ Start."));

	int nRet = VTM_RV_FAILED;


	nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_PPIDREQ();
	if (nRet == VTM_RV_SUCCESSED)
	{
		nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->ProcessPPID();	
	}

	if (nRet == VTM_RV_SUCCESSED)
	{
		nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_PPID(true);
	}
	else
	{
		nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_PPID(false);
		if (nRet == VTM_RV_SUCCESSED)
		{
			nRet = CIM_RV_PPIDID_NG;
		}
	}

	if (nRet == VTM_RV_SUCCESSED)
	{
		Sleep(200);
		nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_STS2004();
	}

	if (nRet != VTM_RV_SUCCESSED)
	{
		CAlarmController::GetInstance()->SetAlarm(nRet);
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_PPIDREQ End"));

	if (nRet == VTM_RV_SUCCESSED)
		return true;
	else
		return false;
}

bool CDoWorkSequenceCIMStep_LOTIDREQ::Run()
{
	if (m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->GetUserMode() != 1)
	{
		return true;
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_LOTIDREQ Start."));

	int nRet = VTM_RV_FAILED;

	nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_LOTIDREQ();

	if (nRet == VTM_RV_SUCCESSED)
	{
		nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->ProcessLOTID();
	}

	if (nRet != VTM_RV_SUCCESSED)
	{
		nRet = CIM_RV_LOTID_NG;
	}

	if (nRet != VTM_RV_SUCCESSED)
	{
		CAlarmController::GetInstance()->SetAlarm(nRet);
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_LOTIDREQ End"));

	if (nRet == VTM_RV_SUCCESSED)
		return true;
	else
		return false;
}


bool CDoWorkSequenceCIMStep_GLSIDREQ::Run()
{
	if (m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->GetUserMode() != 1)
	{
		return true;
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_GLSIDREQ Start."));

	int nRet = VTM_RV_FAILED;

	nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_GLSIDREQ();

	if (nRet == VTM_RV_SUCCESSED)
	{
		nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->ProcessGLSID();
	}

	if (nRet != VTM_RV_SUCCESSED)
	{
		if( nRet != PLC_GLASS_OBLIVIOUS_WORKPLATE_SENSOR_ALARM)
			nRet = CIM_RV_GLASSID_NG;
	}
	
	if (nRet == VTM_RV_SUCCESSED)
	{
		CString strGlassID, strRecipeID;
		strGlassID.Format(_T("%s"), CSystemParamShare::GetInstance()->GetGlassID());
		strRecipeID.Format(_T("%s"), CSystemParamShare::GetInstance()->GetRecipeID());

		m_pShareItem->GetSeqTactTimeMgrPtr()->SetCycleTimeST(1, strGlassID, strRecipeID);

	//	CWindowEventHandler::GetInstance()->PostMessageToUI(enumWndID_TITLE, WM_MSG_UPDATE_RECIPE_INFORMATION, NULL, NULL);
		CWindowEventHandler::GetInstance()->PostMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_RECIPE_INFORMATION, NULL, NULL);

		nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_STS0006();
	}
	else
	{
		CAlarmController::GetInstance()->SetAlarm(nRet);
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_GLSIDREQ End"));

	if (nRet == VTM_RV_SUCCESSED)
		return true;
	else
		return false;
}


bool CDoWorkSequenceCIMStep_PRCSReq::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_PRCSReq Start."));

	int nRet = VTM_RV_FAILED;

	nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->CanDoStartPRCS();
	if (nRet == VTM_RV_SUCCESSED)
	{
		nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_PRCS(true);
	}
	else
	{
		nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_PRCS(false);

		if (nRet == VTM_RV_SUCCESSED)
		{
			nRet = CIM_RV_HANDSHAKE_FAIL;
		}
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_PRCSReq End"));

	if (nRet == VTM_RV_SUCCESSED)
		return true;
	else
		return false;
}

bool CDoWorkSequenceCIMStep_ChangeMeas::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_ChangeMeas Start."));

	m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeST(1, _T("ChangeToMeas"), 0);

	int nRet = VTM_RV_SUCCESSED;

	// Measure State
	long lStep = enumEQSTATUS_NONE;

	int nStatus = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->GetEQState();
	if (nStatus == enumEQStateReady)
	{
		lStep = enumEQSTATUS_R2M;
	}
	else if (nStatus == enumEQStateLoad)
	{
		lStep = enumEQSTATUS_L2M;
	}
	else if (nStatus == enumEQStateMeas)
	{
		lStep = enumEQSTATUS_NONE;
	}
	else
	{
		lStep = enumEQSTATUS_NONE;
		nRet = PLC_STATE_NOT_MEASURE;
	}

	if (lStep != enumEQSTATUS_NONE)
	{
		CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->SetEQStatusChangeStep(lStep);
		CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Start();

		Sleep(500);

		int nLoop = 0;
		bool bRet = false;
		nRet = VTM_RV_FAILED;
		do
		{
			Sleep(100);

			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Is_Complete();
			if (bRet == true)
			{
				nRet = VTM_RV_SUCCESSED;
				break;
			}


			nLoop++;

		} while (nLoop < 1000);
	}

	m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(1, _T("ChangeToMeas"), 0);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_ChangeMeas End"));

	if (nRet == VTM_RV_SUCCESSED)
		return true;
	else
		return false;
}


bool CDoWorkSequenceCIMStep_MeasStart::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_MeasStart Start."));

	int nRet = VTM_RV_FAILED;

	int nStatus = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->GetEQState();
	if (nStatus == enumEQStateReady || nStatus == enumEQStateMeas)
	{
		nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_STS1005();
	}

	// Measture Start
	if (nRet == VTM_RV_SUCCESSED)
	{
		nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->ProcessTo_MeasStart();
	}

	if (nRet == VTM_RV_SUCCESSED)
	{
		Sleep(200);
		nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_STS1006();
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_MeasStart End"));

	if (nRet == VTM_RV_SUCCESSED)
		return true;
	else
		return false;
}


bool CDoWorkSequenceCIMStep_MeasDone::Run()
{
	if (m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->GetUserMode() != 2)
	{
		return true;
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_MeasDone Start."));

	int nRet = VTM_RV_FAILED;
	
	// LYS 2026-09-28 EFA: EFA 글라스는 STS1106 전에 EFA^1$(T1 완료)를 보낸다 (OCP JOB_Proc.cpp:311: EFA1 -> 500 ms -> WORKSTATE_CHANGE -> STS1106).
	// CMeasureSequenceItemEngine::GetEfaPassCount와 같은 게이트: OCP + (EFA 플래그 또는 레시피 EFA) + 항목이 있는 EFA rcp.
	// WORKSTATE_CHANGE^_CAA$는 보류 (OCP는 Format 인자가 빠진 채 보낸다 - D-3).
	{
		bool bEfaGlass = false;
		// LYS 2026-09-30 Hybrid 3c A5 hybrid glass: EFA^1$ follows the EFA decision taken at measure start (engine AgingGlassLatch) -
		//   no shared flag / EFA rcp re-read here. It can differ from the gate below when a step index was missing (EFA did not run)
		//   or the shared flag was cleared mid-glass. -1 = not latched (1-segment recipe) -> the gate below exactly as before.
		int nEfaLatch = m_pShareItem->GetRecipeEditMgrPtr()->GetEfaGlassLatch();
		if (nEfaLatch >= 0)
		{
			bEfaGlass = (nEfaLatch > 0);
			if (bEfaGlass)	// latch 0 (hybrid glass, not EFA) stays silent like the non-EFA 1-segment path
			{
				CString strLatch;
				strLatch.Format(_T("CDoWorkSequenceCIMStep_MeasDone : [HYBRID] EFA glass latch %d pass(es) - no flag / file re-read"), nEfaLatch);
				m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLatch);
			}
		}
		else if (m_pShareItem->GetSystemConfigMgrPtr() != nullptr && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP())
		{
			bool bFlag = m_pShareItem->GetShareMemoryMgrPtr()->GetSys_NewAgingMode();
			S_RecipeInfo* pEfaRcp = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
			bool bRecipe = (pEfaRcp != nullptr && pEfaRcp->m_sInspectItem.m_bUseEFA != FALSE);
			if (bFlag || bRecipe)
			{
				CAgingCycleInfo efaInfo;
				bEfaGlass = (efaInfo.Load(m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeID(), true) && efaInfo.GetItemCount() > 0);
			}
		}

		if (bEfaGlass)
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_MeasDone : EFA glass - send EFA^1$"));
			m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_EFA1();
			Sleep(500);
		}
	}

	// Mdasure End
	nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_STS1106();
	
	if (nRet == VTM_RV_SUCCESSED)
	{
		nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_DataJudge();
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_MeasDone End"));

	if (nRet == VTM_RV_SUCCESSED)
		return true;
	else
		return false;
}

bool CDoWorkSequenceCIMStep_GDTReq::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_GDTReq Start."));

	int nRet = VTM_RV_FAILED;
	nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_GDTREQ();
	
	// APD Data Send
	if (nRet == VTM_RV_SUCCESSED)
	{
		if (m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Send_Data() == true)
		{
			Sleep(200);

			nRet = CIM_SEND_REULST_ERROR;
			int loop = 0;
			bool bRet = false;

			bRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->SendAPDFileToFTP();

			if (bRet == true)
			{
				Sleep(500);

				do
				{
					Sleep(100);

					bRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->GetThreadLive();
					if (bRet != true)
					{
						nRet = VTM_RV_SUCCESSED;
						break;
					}

					loop++;

				} while (loop < 1000);
			}
		}
		else
		{
			Sleep(200);
			nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_GDTCPL(true);
		}
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_GDTReq End"));

	if (nRet == VTM_RV_SUCCESSED)
	{
		return true;
	}
	else
	{
		m_pShareItem->SetFail(true, nRet);
		return false;
	}
		
}

bool CDoWorkSequenceCIMStep_CheckUnLoadReq::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_CheckUnLoadReq Start."));

	int nRet = VTM_RV_FAILED;

	nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->CanDoStartPRCS(true, false);
	if (nRet == VTM_RV_SUCCESSED)
	{
		nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_ULDREQ(true);
	}
	else
	{
		nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_ULDREQ(false);
		if (nRet == VTM_RV_SUCCESSED)
			nRet = CIM_RV_HANDSHAKE_FAIL;
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_CheckUnLoadReq End"));

	if (nRet == VTM_RV_SUCCESSED)
		return true;
	else
		return false;
}

bool CDoWorkSequenceCIMStep_UnLoadDone::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_UnLoadDone Start."));

	int nRet = VTM_RV_FAILED;

	//nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_STS2106();
	//if (nRet == VTM_RV_SUCCESSED)
	//{
	//	Sleep(200);
	//	nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_STS2107();
	//}

	int nStatus = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->GetEQState();		// jsyoon 2022.07.22 add
	if (nStatus == enumEQStateLoad)
	{
		nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_STS2106();
		if (nRet == VTM_RV_SUCCESSED)
		{
			Sleep(200);
			nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_STS2107();
		}
	}	

// jsyoon 2022.06.13 add
	m_pShareItem->GetSeqTactTimeMgrPtr()->SetCycleTimeED(1);
	m_pShareItem->GetSeqTactTimeMgrPtr()->SetTotalSeqTimeED();

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_UnLoadDone End"));

	if (nRet == VTM_RV_SUCCESSED)
		return true;
	else
		return false;
}

bool CDoWorkSequenceCIMStep_CheckUnLoadCpl::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_CheckUnLoadCpl Start."));

	int nRet = VTM_RV_FAILED;

	int loop = 0;
	bool bRet = false;
	do
	{
		Sleep(100);

		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsGlassSensorOn();
		if (bRet != true)
		{
			break;
		}

		loop++;

	} while (loop < 50);
	
	if (bRet != true)
	{
		nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_ULDCPL(true);
	}
	else
	{
		nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_ULDCPL(false);
		if (nRet == VTM_RV_SUCCESSED)
			nRet = CIM_RV_HANDSHAKE_FAIL;
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_CheckUnLoadCpl End"));

	if (nRet == VTM_RV_SUCCESSED)
		return true;
	else
		return false;
}

bool CDoWorkSequenceCIMStep_LoadReady::Run()
{
	int nRet = VTM_RV_FAILED;
	
	if (m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->GetUserMode() != 0)
	{
		if (m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->GetUserMode() == 3)
		{
			m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_STS0506(); 

			return true;
		}
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_LoadReady Start."));

	bool bRet = false;
	if (m_pShareItem->GetIOMapControlMgrPtr()->IsOutOfService() != true)
		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsSafetySensorOn();
	else
		bRet = true;

	long nStatus = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->GetEQState();

	if (bRet == true && nStatus == enumEQStateLoad)
	{
		if (m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->GetUserMode() == 0)
		{ 
			nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_STS0004();
		}
		else
		{
			// jsyoon 2022.07.22 change
			nRet = VTM_RV_SUCCESSED;
			/*nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_STS1106();
			if (nRet == VTM_RV_SUCCESSED)
			{
				nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->Cmd_Send_DataJudge();
			}*/
		}
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_LoadReady End"));

	if (nRet == VTM_RV_SUCCESSED)
		return true;
	else
		return false;
}

bool CDoWorkSequenceCIMStep_ChangeReady::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_ChangeReady Start."));

	m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeST(1, _T("ChangeToReady"), 0);

	int nRet = VTM_RV_SUCCESSED;

	// Measure State
	long lStep = enumEQSTATUS_NONE;

	int nStatus = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->GetEQState();
	if (nStatus == enumEQStateReady)
	{
		lStep = enumEQSTATUS_NONE;
	}
	else if (nStatus == enumEQStateLoad)
	{
		lStep = enumEQSTATUS_L2R;
	}
	else if (nStatus == enumEQStateMeas)
	{
		lStep = enumEQSTATUS_NONE;
	}
	else
	{
		lStep = enumEQSTATUS_NONE;
		nRet = PLC_STATE_NOT_MEASURE;
	}

	if (lStep != enumEQSTATUS_NONE)
	{
		CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->SetEQStatusChangeStep(lStep);
		CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Start();

		Sleep(500);

		int nLoop = 0;
		bool bRet = false;
		nRet = VTM_RV_FAILED;
		do
		{
			Sleep(100);

			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Is_Complete();
			if (bRet == true)
			{
				nRet = VTM_RV_SUCCESSED;
				break;
			}


			nLoop++;

		} while (nLoop < 1000);
	}

	m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(1, _T("ChangeToReady"), 0);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkSequenceCIMStep_ChangeReady End"));

	if (nRet == VTM_RV_SUCCESSED)
		return true;
	else
		return false;
}
