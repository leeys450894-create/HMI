#include "pch.h"
#include "DoWorkGlassUnloadStep.h"

#include "WindowEventHandler.h"
#include "DoWorkGlassSequenceImpl.h"
#include "..\SystemParamShare.h"
#include "..\Manager\SystemConfigMgr.h"
#include "..\Manager\PositionConfigMgr.h"
#include "..\Manager\MaintControlMgr.h"

//#include "RecipeEditMgr.h"
#include "..\Manager\SequenceManager.h"
#include "..\FunctionModule\FunctionModuleEQStateChange.h"
#include "..\FunctionModule\FunctionModuleEQState.h"
#include "Manager/ShareMemoryMgr.h"
#include "..\Include\DefinesCIM.h"
#include "Manager/MCCLogMgr.h"
#include "..\Manager\CommTcpIpFactoryMgr.h"
#include "Manager/ResultMgr.h"
#include "..\CommTcpIpObjectCIM.h"
#include "..\Recipe\RecipeEditMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

bool CDoWorkGlassUnloadStep_None::Run()
{
	// init
	m_pShareItem->SetComplete(false);
	m_pShareItem->SetAlarmOn(false);
	m_pShareItem->SetFail(false);
	m_pShareItem->SetStopUser(false);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassUnloadStep_None Start."));

	m_pShareItem->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_PROCESS);	// 2023.08.09 add
	
	/*m_pShareItem->SetAlarmOn(false);
    m_pShareItem->SetComplete(false);
	m_pShareItem->SetFail(false);*/

    return true;
}

bool CDoWorkGlassUnloadStep_GetPreAlignParam::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassUnloadStep_GetPreAlignParam Start."));
	
	if(m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
        return true;
	
	//Recipe parameter를 호출하여 PreAlign 사용 시 보정값 적용 필요함
	//1. 사용 확인
	//1.1 사용하지 않을 시 return true;
	//1.2 사용 시 성공 확인 후 적용
	//아래는 참고용(수정필요)
	//m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeDataPtr()->GetParam_CurrentMeasArea();
	//m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeDataPtr()->SetParam_CurrentMeasArea(1);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassUnloadStep_GetPreAlignParam End."));
	
	return true;
}

bool CDoWorkGlassUnloadStep_Move_Safety::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassUnloadStep_Move_Safety Start."));

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_SAFETY_PROC, true);
	
	long nRet = df_MOTION_RV_SUCCESSED;

	// Turn Off
	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_SAFETY_CHK_STEP1, true);

	m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PGEvent_8);

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_SAFETY_CHK_STEP1, false);

	nRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmd_SafetyAll();

	bool bRet = false;
	if (nRet == df_MOTION_RV_SUCCESSED)
	{
		bRet = true;
	}
	else
	{
		m_pShareItem->SetFail(true, nRet);
		bRet = false;
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassUnloadStep_Move_Safety End."));

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_SAFETY_PROC, false);

	return bRet;
}

bool CDoWorkGlassUnloadStep_ChangeUnloadStatus::Run()
{
	return true; 

	if (m_pShareItem->GetShareMemoryMgrPtr()->GetSys_MachineMode() != df_HOST_ONLINE_REMOTE)
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_UNLOAD_READY, true);

		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassUnloadStep_ChangeUnloadStatus Start."));

		int nLoop = 0;
		bool bRet = false;
		int nStatus = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->GetEQState();
		if (nStatus == enumEQStateReady)
		{
			CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->SetEQStatusChangeStep(enumEQSTATUS_R2L);
			CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Start();
		}
		else if (nStatus == enumEQStateMeas)
		{
			CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->SetEQStatusChangeStep(enumEQSTATUS_M2L);
			CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Start();
		}
		else if (nStatus == enumEQStateAbnormal)
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassUnloadStep_ChangeUnloadStatus NG(enumEQStateAbnormal)."));

			return false;
		}
		
	
		if (nStatus != enumEQStateLoad)
		{
			Sleep(500);

			nLoop = 0;
			do
			{
				Sleep(100);

				bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Is_Complete();
				if (bRet == true)
				{
					break;
				}

				nLoop++;

			} while (nLoop < 1000 /*200*/);
		}
		else
		{
			bRet = true;
		}

		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassUnloadStep_ChangeUnloadStatus End."));

		if (bRet != true)
		{
			return false;
		}
	}
	
	
	return true;
}

bool CDoWorkGlassUnloadStep_CheckUnloadStatus::Run()
{
	return true;

	if (m_pShareItem->GetShareMemoryMgrPtr()->GetSys_MachineMode() != df_HOST_ONLINE_REMOTE)
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassUnloadStep_CheckUnloadStatus Start."));

		int nStatus = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->GetEQState();
		if (nStatus != enumEQStateLoad)
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassUnloadStep_CheckUnloadStatus NG."));

			return false;
		}

		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassUnloadStep_CheckUnloadStatus End."));
	}
	
	return true;
}

bool CDoWorkGlassUnloadStep_CheckSaveReuslt::Run()
{
	bool bRet = false;
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassUnloadStep_CheckSaveReuslt Start."));

	bool bUseMeas = false;
	S_RecipeInfo* pRecipeInfo;
	pRecipeInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
	if (pRecipeInfo != nullptr)
	{
		if (pRecipeInfo->m_sInspectItem.m_bUseSpec_3 == true)
			bUseMeas = true;

		if (pRecipeInfo->m_sInspectItem.m_bUseQuality_4 == true)
			bUseMeas = true;

		if (pRecipeInfo->m_sInspectItem.m_bUseColor_5 == true)
			bUseMeas = true;

		if (pRecipeInfo->m_sInspectItem.m_bUseAngle_6 == true)
			bUseMeas = true;

		if (pRecipeInfo->m_sInspectItem.m_bUseTSP_7 == true)
			bUseMeas = true;

		if (pRecipeInfo->m_sInspectItem.m_bUseLCR_8 == true)
			bUseMeas = true;
	}

	if (bUseMeas == true)
	{
		int iLoop = 0;
		do
		{
			bRet = CResultMgr::GetInstance()->SaveResultDone();
			if (bRet == true)
			{
				break;
			}

			Sleep(100);

			iLoop++;

		} while (iLoop < 300);
	}
	else
	{
		// jsyoon 2022.07.22 change
		m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectCIM()->SetUserBypassGlass(true);
		bRet = true;
	}

	
	if (bRet != true)
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassUnloadStep_CheckSaveReuslt NG."));

		bRet = false;
	}
	else
	{ 
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassUnloadStep_CheckSaveReuslt End."));

		bRet = true;
	}
	
	return bRet;
}

bool CDoWorkGlassUnloadStep_SendToCIM::Run()
{
	if (m_pShareItem->GetShareMemoryMgrPtr()->GetSys_MachineMode() == df_HOST_ONLINE_REMOTE)
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassUnloadStep_SendToCIM Start."));

		CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CHANGE_CIM_STATE, TO_SERVER_MEAS_END, NULL);

		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassUnloadStep_SendToCIM End."));
	}

	return true;
}

bool CDoWorkGlassUnloadStep_Idle::Run()
{
    CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_Idle, NULL);

	// 완료
    ResetEvent(m_pShareItem->GetStartEventHandle());

	if (m_pShareItem->GetFail() != true)
	{
		m_pShareItem->SetComplete(true);   // 동작 완료
	}
    
    SetEvent(m_pShareItem->GetCompleteEventHandle()); // 시퀀스 완료 이벤트 SET

	if (m_pShareItem->GetShareMemoryMgrPtr()->GetSys_MachineMode() == df_HOST_ONLINE_REMOTE)
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_UNLOAD_READY, false);
	}

    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("DoWorkGlassUnloadStep_Idle"));

	m_pShareItem->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_IDLE);	// 2023.08.09 add

    return false;
}
