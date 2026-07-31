#include "pch.h"
#include "DoWorkGlassReadyStep.h"

#include "DoWorkGlassSequenceImpl.h"
#include "..\SystemParamShare.h"
#include "..\Manager\SystemConfigMgr.h"

#include "..\Manager\SequenceManager.h"
#include "..\FunctionModule\FunctionModuleEQStateChange.h"
#include "..\FunctionModule\FunctionModuleEQState.h"
#include "..\Manager\CommTcpIpFactoryMgr.h"

#include "..\Recipe\RecipeEditMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

bool CDoWorkGlassReadyStep_None::Run()
{
	// init
	m_pShareItem->SetComplete(false);
	m_pShareItem->SetAlarmOn(false);
	m_pShareItem->SetFail(false);
	m_pShareItem->SetStopUser(false);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassReadyStep_None Start."));

	m_pShareItem->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_PROCESS);	// 2023.08.09 add
	
	if(m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
    	return true;
	
	/*m_pShareItem->SetAlarmOn(false);
	m_pShareItem->SetComplete(false);
	m_pShareItem->SetFail(false);*/

//	CSystemParamShare::GetInstance()->SetInitialized(false);

//	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassReadyStep_None End."));

    return true;
}

bool CDoWorkGlassReadyStep_ChangeLoadStatus::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassReadyStep_ChangeLoadStatus Start."));
	
	if (m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassReadyStep_ChangeLoadStatus End. TEST"));
		return true;
	}

	int nStatus = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->GetEQState();
	if (nStatus == enumEQStateReady)
	{
		CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->SetEQStatusChangeStep(enumEQSTATUS_R2L);
		CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Start();

		Sleep(500); // 2023.08.09 add
	}
	else if (nStatus == enumEQStateMeas)
	{
		CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->SetEQStatusChangeStep(enumEQSTATUS_M2L);
		CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Start();

		Sleep(500); // 2023.08.09 add
	}
	else if (nStatus == enumEQStateAbnormal)
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassReadyStep_ChangeLoadStatus NG(enumEQStateAbnormal)."));

		return false;
	}

	bool bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Is_Complete();
	if (!bRet)
	{
		return false;
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassReadyStep_ChangeLoadStatus End."));
	
	return true;
}

bool CDoWorkGlassReadyStep_CheckLoadStatus::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassReadyStep_CheckLoadStatus Start."));

	if (m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassReadyStep_CheckLoadStatus End. TEST"));
		return true;
	}

	int nStatus = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->GetEQState();
	if (nStatus != enumEQStateLoad)
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassReadyStep_CheckLoadStatus NG."));
	
		return false;
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassReadyStep_CheckLoadStatus End."));

	return true;
}

bool CDoWorkGlassReadyStep_LoadGlassInfoFromCIM::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassReadyStep_LoadGlassInfoFromCIM Start."));
	
	if (m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassReadyStep_LoadGlassInfoFromCIM End. TEST"));
		return true;
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassReadyStep_LoadGlassInfoFromCIM End."));
	
	return true;
}

bool CDoWorkGlassReadyStep_LoadRecipe::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassReadyStep_LoaadRecipe Start."));
	
	if(m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
        return true;

	bool bRet = true;

//	//현재 Recipe를 가져와야 됨
//	m_pShareItem->GetRecipeEditMgrPtr()->OpenRecipe(_T("default.rcp"));

	//stRecipeUTGParam    stRecipeUTG = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeDataPtr()->GetParam_RecipeInfo()->stRecipeUTG;

	//long nRet = 0;
	//int nMeasHead = 0;
	//nMeasHead = stRecipeUTG.nUseMeasHead;

	////// Glass ID
	////m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassSWAlignStep_None SET_PARAMETER Set."));
	////nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkSetParameter(nMeasHead);

	////// Recipe Open
	////if (nRet == 0)
	////{
	////	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassSWAlignStep_None RECIPE_CHANGE Set."));
	////	CString strRcp;
	////	strRcp.Format(_T(""), m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeID());
	////	nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkRecipeOpen(nMeasHead);
	////}

	//if (nRet != 0)
	//{
	//	m_pShareItem->SetFail(true);
	//	bRet = false;
	//}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassReadyStep_LoaadRecipe End."));
	
	return bRet;
}

bool CDoWorkGlassReadyStep_Idle::Run()
{
    CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_Idle, NULL);

	// 완료
    ResetEvent(m_pShareItem->GetStartEventHandle());
 //   m_pShareItem->GoToInitStep();
    m_pShareItem->SetComplete(true);   // 동작 완료
    SetEvent(m_pShareItem->GetCompleteEventHandle()); // 시퀀스 완료 이벤트 SET

    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("DoWorkGlassReadyStep_Idle"));

	m_pShareItem->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_IDLE);	// 2023.08.09 add

    return false;
}
