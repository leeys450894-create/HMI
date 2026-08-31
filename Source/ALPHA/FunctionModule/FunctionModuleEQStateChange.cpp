//#include "stdafx.h"
#include "pch.h"
#include "FunctionModuleEQStateChange.h"

#include "LogFunction\LogAccessor\LogFileAccessor.h"

#include "FunctionModuleEQStateChangeStep.h"
#include "..\Manager\SystemConfigMgr.h"
#include "..\SystemParamShare.h"
#include "..\AuroraFuncDefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CFunctionModuleEQStateChange::CFunctionModuleEQStateChange(stAuroraMgrPool &MgrPool)
	: CBaseSequenceImpl(MgrPool, enumSeqItemID_REQ_EQ_STATE_CHANGE)
{
	m_strSeqType = _T("SEQUENCE_ITEM_FUNC_MODULE_EQ_STATE_CHANGE");
	m_strSeqComm = _T("TCPIP");
	m_strSeqCorp = _T("HBS");
	m_strSeqName = _T("SEQ EQSTATE_CHANGE");
	m_strSeqGUID = _T("436C83A3-FD1F-4B35-BE5E-89B0FA2E1B7D");
	m_strSeqDesc = _T("EQ State Change");

	const CString strFolderPath = CSystemParamShare::GetInstance()->GetLoggerFolderPath() + _T("\\LogFile\\");
	const CString strLogFile = strFolderPath + m_strSeqName + _T(".log");
	MakeDir(strFolderPath);
	m_pLogFileAccessor->Initialize(strLogFile);

	/*
	1. LPUp
	2. MoveLoaderXYZ(Meas)
	3. ClampOut
	4. MoveLoaderZUp
	5. Vacuum
	6. MoveLoaderZDown
	7. ClampIn
	8. MoveLoaderXYZ(Load)
	9. LPDown
	*/

	// R2L, L2M, M2R, R2M, M2L, L2R
	//시퀀스는 정의 되었는데 동작은 어떻게 할 것인가?
	//20210524 SetEQStatusChangeStep 명령으로 설정 후 동작


	UpdateStep();

	
}

CFunctionModuleEQStateChange::~CFunctionModuleEQStateChange()
{
	// sequence step -
	for (size_t i = 0; i < m_vecSeqStepALL.size(); ++i)
	{
		if (m_vecSeqStepALL[i])
		{
			delete m_vecSeqStepALL[i];
			m_vecSeqStepALL[i] = nullptr;
		}
	}

	DeleteDoWorkSeq();
}

void CFunctionModuleEQStateChange::SetEQStatusChangeStep(long lStep)
{ 
	m_lCurrentStep = lStep;

	SetEQStatusExitStepIndex(lStep);
}

void CFunctionModuleEQStateChange::SetEQStatusExitStepIndex(long lStep)
{
//	UpdateStep();

	switch (lStep)
	{
	case enumEQSTATUS_NONE:
		SetExitStepIndex(m_vecSeqStep.size()-1);
		break;
	case enumEQSTATUS_R2L:
		SetExitStepIndex(m_vecSeqStepR2L.size() - 1); 
		break;
	case enumEQSTATUS_L2M:
		SetExitStepIndex(m_vecSeqStepL2M.size() - 1);
		break;
	case enumEQSTATUS_M2R:
		SetExitStepIndex(m_vecSeqStepM2R.size() - 1);
		break;
	case enumEQSTATUS_R2M:
		SetExitStepIndex(m_vecSeqStepR2M.size() - 1);
		break;
	case enumEQSTATUS_M2L:
		SetExitStepIndex(m_vecSeqStepM2L.size() - 1);
		break;
	case enumEQSTATUS_L2R:
		SetExitStepIndex(m_vecSeqStepL2R.size() - 1);
		break;
	case enumEQSTATUS_SAFETY:
		SetExitStepIndex(m_vecSeqStepSafety.size() - 1);
		break;
	default:
		break;
	}
}

void CFunctionModuleEQStateChange::UpdateStep()
{
	// Idle
	m_vecSeqStep.clear();
	m_vecSeqStep.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_None(this)));
	m_vecSeqStep.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_LPDown(this)));
	m_vecSeqStep.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_Ready(this)));
	m_vecSeqStep.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_Idle(this)));

	SetTotalStepCount((long)m_vecSeqStep.size());
	SetExitStepIndex(GetTotalStepCount() - 1);

	//1. R2L
	// enumEQSTATUS_R2L
	m_vecSeqStepR2L.clear();
	m_vecSeqStepR2L.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_None(this)));
	m_vecSeqStepR2L.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_Meas(this)));			// Change Meas
	m_vecSeqStepR2L.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_LoaderXYRLoad(this)));
	m_vecSeqStepR2L.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_Ready(this)));			// Change Ready
	m_vecSeqStepR2L.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_FixtureUp(this)));
	m_vecSeqStepR2L.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_X_RAY_ON2(this)));
	m_vecSeqStepR2L.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_LPUpPre(this)));
	m_vecSeqStepR2L.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_FixtureDown(this)));
	m_vecSeqStepR2L.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_LPUp(this)));
	m_vecSeqStepR2L.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_X_RAY_OFF2(this)));
	m_vecSeqStepR2L.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_Load(this)));
	m_vecSeqStepR2L.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_Idle(this)));

	//2. L2M
	// enumEQSTATUS_l2M
	m_vecSeqStepL2M.clear();
	m_vecSeqStepL2M.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_None(this)));
	m_vecSeqStepL2M.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_X_RAY_ON1(this)));
		m_vecSeqStepL2M.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_FixtureUpBack(this)));	//Ionizer 영향으로 글라스 틀어짐 방지
	m_vecSeqStepL2M.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_LPDown(this)));
	m_vecSeqStepL2M.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_X_RAY_OFF1(this)));
		m_vecSeqStepL2M.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_FixtureDown(this)));
	m_vecSeqStepL2M.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_Meas(this)));
	m_vecSeqStepL2M.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_LoaderXYRMeas(this)));
	m_vecSeqStepL2M.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_Idle(this)));

	//3. M2R
	m_vecSeqStepM2R.clear();
	m_vecSeqStepM2R.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_None(this)));
	m_vecSeqStepM2R.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_Ready(this)));
	m_vecSeqStepM2R.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_Idle(this)));

	//4. R2M
	m_vecSeqStepR2M.clear();
	m_vecSeqStepR2M.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_None(this)));
	m_vecSeqStepR2M.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_Meas(this)));
	m_vecSeqStepR2M.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_Idle(this)));

	//5. M2L
	m_vecSeqStepM2L.clear();
	m_vecSeqStepM2L.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_None(this)));
	m_vecSeqStepM2L.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_LoaderXYRLoad(this)));
	m_vecSeqStepM2L.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_Ready2(this)));

	m_vecSeqStepM2L.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_FixtureUp(this)));
	m_vecSeqStepM2L.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_X_RAY_ON2(this)));
	m_vecSeqStepM2L.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_LPUpPre(this)));
	m_vecSeqStepM2L.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_FixtureDown(this)));
	m_vecSeqStepM2L.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_LPUp(this)));
	m_vecSeqStepM2L.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_X_RAY_OFF2(this)));

	//m_vecSeqStepM2L.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_X_RAY_ON2(this)));
	//m_vecSeqStepM2L.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_LPUp(this)));
	//m_vecSeqStepM2L.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_X_RAY_OFF2(this)));
	m_vecSeqStepM2L.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_Load(this)));
	m_vecSeqStepM2L.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_Idle(this)));

	//6. L2R
	m_vecSeqStepL2R.clear();
	m_vecSeqStepL2R.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_None(this)));
	m_vecSeqStepL2R.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_X_RAY_ON1(this)));
		m_vecSeqStepL2R.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_FixtureUpBack(this)));	//Ionizer 영향으로 글라스 틀어짐 방지
	m_vecSeqStepL2R.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_LPDown(this)));
	m_vecSeqStepL2R.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_X_RAY_OFF1(this)));
		m_vecSeqStepL2R.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_FixtureDown(this)));
	m_vecSeqStepL2R.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_Ready(this)));
	m_vecSeqStepL2R.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_LoaderXYRLoad(this)));
	m_vecSeqStepL2R.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_Idle(this)));

	//7. Safety
	m_vecSeqStepSafety.clear();
	m_vecSeqStepSafety.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_None(this)));
	m_vecSeqStepSafety.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_Safety(this)));
	m_vecSeqStepSafety.push_back(dynamic_cast<CFunctionModuleEQStateChangeStep*>(new CFunctionModuleEQStateChangeStep_Idle(this)));


	m_vecSeqStepALL.insert(m_vecSeqStepALL.end(), m_vecSeqStep.begin(), m_vecSeqStep.end());
	m_lEQStatusStepStart[enumEQSTATUS_NONE] = (long)m_vecSeqStepALL.size() - (long)m_vecSeqStep.size();
	m_lEQStatusStepEnd[enumEQSTATUS_NONE] = (long)m_vecSeqStep.size(); 

	//1. R2L
	m_vecSeqStepALL.insert(m_vecSeqStepALL.end(), m_vecSeqStepR2L.begin(), m_vecSeqStepR2L.end());
	m_lEQStatusStepStart[enumEQSTATUS_R2L] = (long)m_vecSeqStepALL.size() - (long)m_vecSeqStepR2L.size();
	m_lEQStatusStepEnd[enumEQSTATUS_R2L] = (long)m_vecSeqStepALL.size();

	//2. L2M
	m_vecSeqStepALL.insert(m_vecSeqStepALL.end(), m_vecSeqStepL2M.begin(), m_vecSeqStepL2M.end());
	m_lEQStatusStepStart[enumEQSTATUS_L2M] = (long)m_vecSeqStepALL.size() - (long)m_vecSeqStepL2M.size();
	m_lEQStatusStepEnd[enumEQSTATUS_L2M] = (long)m_vecSeqStepALL.size();

	//3. M2R
	m_vecSeqStepALL.insert(m_vecSeqStepALL.end(), m_vecSeqStepM2R.begin(), m_vecSeqStepM2R.end());
	m_lEQStatusStepStart[enumEQSTATUS_M2R] = (long)m_vecSeqStepALL.size() - (long)m_vecSeqStepM2R.size();
	m_lEQStatusStepEnd[enumEQSTATUS_M2R] = (long)m_vecSeqStepALL.size();

	//4. R2M
	m_vecSeqStepALL.insert(m_vecSeqStepALL.end(), m_vecSeqStepR2M.begin(), m_vecSeqStepR2M.end());
	m_lEQStatusStepStart[enumEQSTATUS_R2M] = (long)m_vecSeqStepALL.size() - (long)m_vecSeqStepR2M.size();
	m_lEQStatusStepEnd[enumEQSTATUS_R2M] = (long)m_vecSeqStepALL.size();

	//5. M2L
	m_vecSeqStepALL.insert(m_vecSeqStepALL.end(), m_vecSeqStepM2L.begin(), m_vecSeqStepM2L.end());
	m_lEQStatusStepStart[enumEQSTATUS_M2L] = (long)m_vecSeqStepALL.size() - (long)m_vecSeqStepM2L.size();
	m_lEQStatusStepEnd[enumEQSTATUS_M2L] = (long)m_vecSeqStepALL.size();

	//6. L2R
	m_vecSeqStepALL.insert(m_vecSeqStepALL.end(), m_vecSeqStepL2R.begin(), m_vecSeqStepL2R.end());
	m_lEQStatusStepStart[enumEQSTATUS_L2R] = (long)m_vecSeqStepALL.size() - (long)m_vecSeqStepL2R.size();
	m_lEQStatusStepEnd[enumEQSTATUS_L2R] = (long)m_vecSeqStepALL.size();

	//7. Safety
	m_vecSeqStepALL.insert(m_vecSeqStepALL.end(), m_vecSeqStepSafety.begin(), m_vecSeqStepSafety.end());
	m_lEQStatusStepStart[enumEQSTATUS_SAFETY] = (long)m_vecSeqStepALL.size() - (long)m_vecSeqStepSafety.size();
	m_lEQStatusStepEnd[enumEQSTATUS_SAFETY] = (long)m_vecSeqStepALL.size();
}




//=========================================================================
// INTERFACE 관련
//=========================================================================
bool CFunctionModuleEQStateChange::CreateDoWorkSeq()
{
	return true;
}

void CFunctionModuleEQStateChange::DeleteDoWorkSeq()
{
}

bool CFunctionModuleEQStateChange::ChangeState()
{
	DeleteDoWorkSeq();

	return CreateDoWorkSeq();
}

bool CFunctionModuleEQStateChange::UpdateState()
{
	return true;
}

bool CFunctionModuleEQStateChange::LinkDataBase(CBinaryHelper *pBinaryHelper, bool bSave)
{
	return true;
}

void CFunctionModuleEQStateChange::ThreadRun()
{
	GoToInitStep();

	for (size_t i = m_lEQStatusStepStart[m_lCurrentStep]; i < m_lEQStatusStepEnd[m_lCurrentStep]; ++i) 
	{
		//CFunctionModuleEQStateChangeStep *pSeqItem = m_vecSeqStep[i];
		CFunctionModuleEQStateChangeStep *pSeqItem = m_vecSeqStepALL[i];
		if (pSeqItem)
		{
			if (pSeqItem->Run() == true) // 동작
			{
				GoToNextStep();
			}
			else if (GetCurStep() == GetExitStepIndex())
			{
				CString strLog;
				strLog.Format(_T("CFunctionModuleEQStateChangeStep Thread End"));
				m_pLogFileAccessor->KMLOG(INSP, strLog);

				return;
			}
			else 
			{
				int nCurStep = GetCurStep();

				GoToExitStep();
				int nExitStep = m_lEQStatusStepStart[m_lCurrentStep] + GetCurStep();
				CString strLog;
				strLog.Format(_T("CFunctionModuleEQStateChangeStep Thread NG,  Start = %d, Step = %d, Exit = %d"), m_lEQStatusStepStart[m_lCurrentStep], nCurStep, nExitStep);
				m_pLogFileAccessor->KMLOG(WARN, strLog);

				/*CString strLog;
				strLog.Format(_T("CFunctionModuleEQStateChangeStep Thread NG, Step = %d/%d"), GetCurStep(), GetExitStepIndex());
				m_pLogFileAccessor->KMLOG(WARN, strLog);

				GoToExitStep();*/ 
				CFunctionModuleEQStateChangeStep* pSeqItem = m_vecSeqStepALL[nExitStep];  // m_vecSeqStepALL[m_lEQStatusStepEnd[m_lCurrentStep]-1];
				if (pSeqItem)
				{
					pSeqItem->Run();
				}

				return;
			}
			/*else
			{
				CString strLog;
				strLog.Format(_T("CFunctionModuleEQStateChangeStep Thread NG, Step = %d"), GetCurStep());
				m_pLogFileAccessor->KMLOG(ERRO, strLog);

				return;
			}*/
		}
	}
}