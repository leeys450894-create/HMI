#pragma once

#include "..\AuroraMgrPool.h"

//class CAutoFocusController;
class CSystemInitialize;
class CDoWorkGlassSequence;
class CFunctionModuleProbeACC;
class CFunctionModuleAutoFocus;
class CFunctionModuleEQStateChange;
class CFunctionModuleEQState;
class CFunctionModuleAFControl;
class CFunctionModuleCentering;
//class CDoWorkProbeSWAlign;
class CBaseSequenceImpl;
class CBinaryHelper;
class CDoWorkSequenceCIM;
class CSequenceManager
{
//	CAutoFocusController				*m_pAutoFocusController;
	CSystemInitialize					*m_pSystemInitialize;
	CDoWorkGlassSequence				*m_pDoWorkGlassSequence;	
	CFunctionModuleProbeACC				*m_pFunctionModuleProbeACC;
	CFunctionModuleAutoFocus			*m_pFunctionModuleAutoFocus;
	CFunctionModuleEQStateChange		*m_pFunctionModuleEQStateChange;
	CFunctionModuleEQState				*m_pFunctionModuleEQState;
	CFunctionModuleAFControl			*m_pFunctionModuleAFControl;
	CFunctionModuleCentering			*m_pFunctionModuleCentering;
//	CDoWorkProbeSWAlign					*m_pDoWorkProbeSWAlign;
	CDoWorkSequenceCIM					*m_pDoWorkSequenceCIM;
public:
	CSystemInitialize					*GetSystemInitialize();
	CDoWorkGlassSequence				*GeDoWorkGlassSequence();
	CFunctionModuleProbeACC				*GetFunctionModuleProbeACC();
	CFunctionModuleAutoFocus			*GetFunctionModuleAutoFocus();
	CFunctionModuleEQStateChange		*GetFunctionModuleEQStateChange();
	CFunctionModuleEQState				*GetFunctionModuleEQState();
	CFunctionModuleAFControl			*GetFunctionModuleAFControl();
	CFunctionModuleCentering			*GetFunctionModuleCentering();
//	CDoWorkProbeSWAlign					*GetDoWorkProbeSWAlign();
	CDoWorkSequenceCIM					*GetDoWorkSequenceCIM();

private:
	stAuroraMgrPool	m_MgrPool_link;
	std::vector<CBaseSequenceImpl*>	m_vecBaseSeqItem;
public:
	bool	LinkDataBase(CBinaryHelper *pBinaryHelper, bool bSave);
	void	UpdateDataBaseInformation();
	void	LinkParam(stAuroraMgrPool &MgrPool);
	void	UpdateLinkParamInformation();


//=========================================================================
// SequenceItem ฐüทร
//=========================================================================
	bool	EngineInitialize();
	void	EngineFinalize();
	bool	IsEngineCompleteAll();
	bool	IsEngineComplete(int index);
	
	void	EngineStart();
	bool	EngineStart(CString strType, bool bCompleteCheck);
	void	EngineResume();
	void	EnginePause();
	void	EngineStop();
	void	EngineStopAndSkip(CString strType);
	void	EngineStopAndExit(CString strType);
	void	EngineCycleInit();



    //----------------------------------------------------------------------------------------------------
    // Singleton
    //----------------------------------------------------------------------------------------------------
private:
    static CSequenceManager	*m_pInstance;
public:
    static CSequenceManager	*GetInstance();
    static void				DelInstance();
    //----------------------------------------------------------------------------------------------------
    //----------------------------------------------------------------------------------------------------
private:
	CSequenceManager(void);
	~CSequenceManager(void);
};