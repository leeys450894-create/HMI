#include "stdafx.h"
#include "DoWorkGlassUnloadingStep.h"

#include "WindowEventHandler.h"
#include "DoWorkGlassSequenceImpl.h"
#include "SystemParamShare.h"
#include "SystemConfigMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

bool CDoWorkGlassUnloadingStep_None::Run()
{
    //	if(m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
    //		return true;

    m_pShareItem->SetComplete(false);


	
	
	
	return true;
}

bool CDoWorkGlassUnloadingStep_InitPosition::Run()
{
    if(m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
        return true;

	return true;
}

bool CDoWorkGlassUnloadingStep_InitPosition2::Run()
{
    if(m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
        return true;

	return true;
}

bool CDoWorkGlassUnloadingStep_InitPositionCheck::Run()
{
    if(m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
        return true;

	return true;
}


bool CDoWorkGlassUnloadingStep_Complete::Run()
{
    if(m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
        return true;

	return true;
}

bool CDoWorkGlassUnloadingStep_CompleteCheck::Run()
{
    if(m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
        return true;

	return true;
}

bool CDoWorkGlassUnloadingStep_Exit::Run()
{
    if(m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
        return true;

	return true;
}

bool CDoWorkGlassUnloadingStep_ExitCheck::Run()
{
    if(m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
        return true;

	return true;
}

bool CDoWorkGlassUnloadingStep_Idle::Run()
{
    CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_Idle, NULL);

	// ¿Ï·á
    ResetEvent(m_pShareItem->GetStartEventHandle());
 //   m_pShareItem->GoToInitStep();
    m_pShareItem->SetComplete(true);
    SetEvent(m_pShareItem->GetCompleteEventHandle());

    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(OPER, _T("DoWorkGlassUnloadingStep_Idle"));
    return false;
}
