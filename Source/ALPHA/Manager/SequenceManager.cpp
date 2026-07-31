//#include "stdafx.h"
#include "pch.h"
#include "SequenceManager.h"

//#include "..\AutoFocusControl\AutoFocusController.h"
#include "..\DoWork\SystemInitialize.h"
#include "..\DoWork\DoWorkGlassSequence.h"
#include "..\DoWork\DoWorkProbeSWAlign.h"
#include "..\DoWork\DoWorkSequenceCIM.h"
#include "..\FunctionModule\FunctionModuleProbeACC.h"
#include "..\FunctionModule\FunctionModuleEQStateChange.h"
#include "..\FunctionModule\FunctionModuleEQState.h"

//#include "..\FunctionModule\FunctionModuleAutoFocus.h"
//#include "..\FunctionModule\FunctionModuleAFControl.h"
//#include "FunctionModuleCentering.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//----------------------------------------------------------------------------------------------------
// Singleton
//----------------------------------------------------------------------------------------------------
CSequenceManager *CSequenceManager::m_pInstance = nullptr;
CSequenceManager *CSequenceManager::GetInstance()
{
    return m_pInstance ? m_pInstance : (m_pInstance = new CSequenceManager);
}
void CSequenceManager::DelInstance()
{
    if (m_pInstance)
    {
        delete m_pInstance;
        m_pInstance = nullptr;
    }
}
//----------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------


CSequenceManager::CSequenceManager()
{
//	m_pAutoFocusController = nullptr;
	m_vecBaseSeqItem.clear();
	m_pSystemInitialize = nullptr;
	m_pDoWorkGlassSequence = nullptr;
	m_pFunctionModuleProbeACC = nullptr;
	m_pFunctionModuleAutoFocus = nullptr;
	m_pFunctionModuleEQStateChange = nullptr;
	m_pFunctionModuleEQState = nullptr;
	m_pFunctionModuleAFControl = nullptr;
	m_pFunctionModuleCentering = nullptr;
//	m_pDoWorkProbeSWAlign = nullptr;
	m_pDoWorkSequenceCIM = nullptr;
}

CSequenceManager::~CSequenceManager()
{
	//if(m_pAutoFocusController)
	//{
	//	delete m_pAutoFocusController;
	//	m_pAutoFocusController = nullptr;
	//}

	for(size_t i = 0; i < m_vecBaseSeqItem.size(); ++i)
	{
		if(m_vecBaseSeqItem[i])
		{
			delete m_vecBaseSeqItem[i];
			m_vecBaseSeqItem[i] = nullptr;
		}
	}
}
	
CSystemInitialize *CSequenceManager::GetSystemInitialize()
{
	return m_pSystemInitialize;
}
CDoWorkGlassSequence *CSequenceManager::GeDoWorkGlassSequence()
{
    return m_pDoWorkGlassSequence;
}	
CFunctionModuleProbeACC	*CSequenceManager::GetFunctionModuleProbeACC()
{
	return m_pFunctionModuleProbeACC;
}
CFunctionModuleAutoFocus *CSequenceManager::GetFunctionModuleAutoFocus()
{
	return m_pFunctionModuleAutoFocus;
}
CFunctionModuleEQStateChange *CSequenceManager::GetFunctionModuleEQStateChange()
{
	return m_pFunctionModuleEQStateChange;
}
CFunctionModuleEQState *CSequenceManager::GetFunctionModuleEQState()
{
	return m_pFunctionModuleEQState;
}
CFunctionModuleAFControl *CSequenceManager::GetFunctionModuleAFControl()
{
	return m_pFunctionModuleAFControl;
}
CFunctionModuleCentering *CSequenceManager::GetFunctionModuleCentering()
{
	return m_pFunctionModuleCentering;
}
//CDoWorkProbeSWAlign* CSequenceManager::GetDoWorkProbeSWAlign()
//{
//	return m_pDoWorkProbeSWAlign;
//}
CDoWorkSequenceCIM* CSequenceManager::GetDoWorkSequenceCIM()
{
	return m_pDoWorkSequenceCIM;
}

bool CSequenceManager::LinkDataBase(CBinaryHelper *pBinaryHelper, bool bSave)
{
    bool ret = true;
	for(size_t i = 0; i < m_vecBaseSeqItem.size(); ++i)
	{
		if(m_vecBaseSeqItem[i])
		{
			ret &= m_vecBaseSeqItem[i]->LinkDataBase(pBinaryHelper, bSave);
		}
	}

    return ret;
}

void CSequenceManager::UpdateDataBaseInformation()
{
	for(size_t i = 0; i < m_vecBaseSeqItem.size(); ++i)
	{
		if(m_vecBaseSeqItem[i])
		{
			m_vecBaseSeqItem[i]->UpdateDataBaseInformation();
		}
	}
}

void CSequenceManager::LinkParam(stAuroraMgrPool &MgrPool)
{
    m_MgrPool_link	= MgrPool;
	
	m_pSystemInitialize				= new CSystemInitialize(MgrPool);			 // 이니셜
	m_pDoWorkGlassSequence			= new CDoWorkGlassSequence(MgrPool);		 // 측정 시퀀스
	m_pFunctionModuleEQStateChange	= new CFunctionModuleEQStateChange(MgrPool); // Stage 상태 변경
	m_pFunctionModuleEQState		= new CFunctionModuleEQState(MgrPool);		 // 장비상태 Check(접점)
	m_pDoWorkSequenceCIM			= new CDoWorkSequenceCIM(MgrPool);			 // CIM

	// PROBE EQ
	m_pFunctionModuleProbeACC		= new CFunctionModuleProbeACC(MgrPool);		// SACC 

	m_vecBaseSeqItem.clear();
	m_vecBaseSeqItem.push_back(dynamic_cast<CBaseSequenceImpl*>(m_pSystemInitialize));
	m_vecBaseSeqItem.push_back(dynamic_cast<CBaseSequenceImpl*>(m_pDoWorkGlassSequence));
	m_vecBaseSeqItem.push_back(dynamic_cast<CBaseSequenceImpl*>(m_pFunctionModuleEQStateChange));
	m_vecBaseSeqItem.push_back(dynamic_cast<CBaseSequenceImpl*>(m_pFunctionModuleEQState));
	m_vecBaseSeqItem.push_back(dynamic_cast<CBaseSequenceImpl*>(m_pDoWorkSequenceCIM));

	// PROBE EQ
	m_vecBaseSeqItem.push_back(dynamic_cast<CBaseSequenceImpl*>(m_pFunctionModuleProbeACC)); // SACC
}

void CSequenceManager::UpdateLinkParamInformation()
{
    // 통신 설정이 변경되면 update 해줘야 한다.
	for(size_t i = 0; i < m_vecBaseSeqItem.size(); ++i)
	{
		if(m_vecBaseSeqItem[i])
		{
			m_vecBaseSeqItem[i]->UpdateLinkParamInformation(m_MgrPool_link.pCommTcpIPMgr);
		}
	}
}


//=========================================================================
// SequenceItem 관련
//=========================================================================
bool CSequenceManager::EngineInitialize()
{
    return m_pDoWorkGlassSequence->CreateDoWorkSeq();
}

void CSequenceManager::EngineFinalize()
{
    m_pDoWorkGlassSequence->DeleteDoWorkSeq();
}

bool CSequenceManager::IsEngineCompleteAll()
{
    return m_pDoWorkGlassSequence->Is_CompleteAll();
}

bool CSequenceManager::IsEngineComplete(int index)
{
    return m_pDoWorkGlassSequence->Is_Complete(index);
}

void CSequenceManager::EngineStart()
{
    m_pDoWorkGlassSequence->EngineStart();
}

bool CSequenceManager::EngineStart(CString strType, bool bCompleteCheck)
{
    return m_pDoWorkGlassSequence->ItemStart(strType, bCompleteCheck);
}

void CSequenceManager::EngineResume()
{
    m_pDoWorkGlassSequence->EngineResume();
}

void CSequenceManager::EnginePause()
{
    m_pDoWorkGlassSequence->EnginePause();
}

void CSequenceManager::EngineStop()
{
    m_pDoWorkGlassSequence->EngineStop();
}

void CSequenceManager::EngineStopAndSkip(CString strType)
{
    m_pDoWorkGlassSequence->ItemStopAndSkip(strType);
}

void CSequenceManager::EngineStopAndExit(CString strType)
{
    m_pDoWorkGlassSequence->ItemStopAndExit(strType);
}

void CSequenceManager::EngineCycleInit()
{
	m_pDoWorkGlassSequence->EngineStop();
}
