#include "pch.h"
#include "DoWorkGlassResultStep.h"

#include "DoWorkGlassSequenceImpl.h"
#include "..\SystemParamShare.h"
#include "..\Manager\SystemConfigMgr.h"
#include "..\Manager\SeqTactTimeMgr.h"
#include "Manager/ResultMgr.h"
#include "Recipe/RecipeEditMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

bool CDoWorkGlassResultStep_None::Run()
{
    //	if(m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
    //		return true;

    // init
    m_pShareItem->SetComplete(false);
    m_pShareItem->SetAlarmOn(false);
    m_pShareItem->SetFail(false);
    m_pShareItem->SetStopUser(false);

    m_pShareItem->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_PROCESS);	// 2023.08.09 add

	/*m_pShareItem->SetAlarmOn(false);
	m_pShareItem->SetComplete(false);
	m_pShareItem->SetFail(false);*/

    return true;
}

bool CDoWorkGlassResultStep_InitPosition::Run()
{
    if(m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
        return true;
	
    return true;
}

bool CDoWorkGlassResultStep_InitPosition2::Run()
{
    if(m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
        return true;

    return true;
}

bool CDoWorkGlassResultStep_InitPositionCheck::Run()
{
    if(m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
        return true;

    return true;
}

bool CDoWorkGlassResultStep_Complete::Run()
{
    if(m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
        return true;

    return true;
}

bool CDoWorkGlassResultStep_CompleteCheck::Run()
{
    bool bRet = true;
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
        CString strStartTime;
        strStartTime.Format(_T("%s"), m_pShareItem->GetSeqTactTimeMgrPtr()->GetStartTime2());
        bRet = CResultMgr::GetInstance()->SaveResult(true, true, strStartTime);
    }
	

    return bRet;
}

bool CDoWorkGlassResultStep_Exit::Run()
{
    if(m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
        return true;

    return true;
}

bool CDoWorkGlassResultStep_ExitCheck::Run()
{
    if(m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
        return true;

    return true;
}

bool CDoWorkGlassResultStep_Idle::Run()
{
    CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_Idle, NULL);

	// 완료
    ResetEvent(m_pShareItem->GetStartEventHandle());
    m_pShareItem->SetComplete(true);   // 동작 완료
    SetEvent(m_pShareItem->GetCompleteEventHandle()); // 시퀀스 완료 이벤트 SET

    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("DoWorkGlassResultStep_Idle"));

    m_pShareItem->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_IDLE);	// 2023.08.09 add

    return false;
}
