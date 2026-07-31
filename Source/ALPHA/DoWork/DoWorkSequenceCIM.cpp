#include "pch.h"
#include "DoWorkSequenceCIM.h"

#include "LogFunction\LogAccessor\LogFileAccessor.h"

#include "DoWorkSequenceCIMStep.h"
#include "..\SystemParamShare.h"
#include "..\AuroraFuncDefine.h"
#include "..\Manager\MCCLogMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CDoWorkSequenceCIM::CDoWorkSequenceCIM(stAuroraMgrPool& MgrPool)
	: CBaseSequenceImpl(MgrPool, enumSeqItemID_SEQ_CIM)
	, m_pShareItem(this)
{
	m_strSeqType = _T("SEQUENCE_ITEM_CIM");
	m_strSeqComm = _T("TCPIP");
	m_strSeqCorp = _T("KMAC");
	m_strSeqName = _T("SEQ CIM");
	m_strSeqGUID = _T("56908D50-62F9-4310-9C3E-F44A656A42AE");
	m_strSeqDesc = _T("SEQ CIM");

	m_lCurrentStep = 0;
	m_bThreadRun = false;

	// 로그 생성하기
	const CString strFolderPath = CSystemParamShare::GetInstance()->GetLoggerFolderPath() + _T("\\LogFile\\");
	const CString strLogFile = strFolderPath + m_strSeqName + _T(".log");
	MakeDir(strFolderPath);
	m_pLogFileAccessor->Initialize(strLogFile);

	// sequence step -
	//
	// enumCIMStatus_INIT
	// 1. Change Online
	m_vecSeqStepInit.clear();
	m_vecSeqStepInit.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_None(this)));
	m_vecSeqStepInit.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_Initial(this)));  // TIMEREQ$
	m_vecSeqStepInit.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_Initial2(this)));   // STS0304$  Glass not Exist -> SetUserMode(0) : Load, Glass Exist -> SetUserMode(3) : Meas, SetUserMode(2) : Unalod
	m_vecSeqStepInit.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_Loading(this)));    // Loading State  SetUserMode(0) or SetUserMode(2) 
	m_vecSeqStepInit.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_LoadReady(this)));  // SetUserMode(0) : STS0004$, SetUserMode(3) : STS0506$  
	m_vecSeqStepInit.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_MeasDone(this)));	// SetUserMode(2) : STS1106$
	m_vecSeqStepInit.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_Idle(this)));

	// enumCIMStatus_READY
	// 2. Change Load
	m_vecSeqStepReady.clear();
	m_vecSeqStepReady.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_None(this)));
	m_vecSeqStepReady.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_Loading(this)));
	m_vecSeqStepReady.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_LoadReady(this)));
	m_vecSeqStepReady.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_Idle(this)));

	// enumCIMStatus_LOAD_READY
	// LOAD Ready
	// 1. Load Ready
	// 2. LoadReq  
	//  LDREQ 
	// -> LDREQ_OK -> Change Load -> dfSTS_21_05
	// enumCIMStatus_LOAD
	m_vecSeqSteploadReady.clear();
	m_vecSeqSteploadReady.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_None(this)));
	m_vecSeqSteploadReady.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_CheckLoadReq(this)));
	m_vecSeqSteploadReady.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_ChangeLoad(this)));
	m_vecSeqSteploadReady.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_LoadReq(this)));
	m_vecSeqSteploadReady.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_Idle(this)));

	// enumCIMStatus_LOAD
	// LDCPL or GLSREADY or MULTIRECIPE
	// PPID
	// -> PPIDREQ -> PPID_OK -> STS2004 
	// -> LOTREQ 
	// -> GLSREQ -> dfSTS_00_06
	m_vecSeqStepload.clear();
	m_vecSeqStepload.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_None(this)));
	m_vecSeqStepload.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_PPIDREQ(this)));
	m_vecSeqStepload.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_LOTIDREQ(this)));
	m_vecSeqStepload.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_GLSIDREQ(this)));
	m_vecSeqStepload.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_Idle(this)));

	// enumCIMStatus_MEAS
	// Meas
	// dfPRCS
	// -> PRCS_OK 
	// -> STS1005 
	// -> STS1006
	m_vecSeqStepMeas.clear();
	m_vecSeqStepMeas.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_None(this)));
	m_vecSeqStepMeas.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_PRCSReq(this)));
	m_vecSeqStepMeas.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_ChangeReady(this)));
	m_vecSeqStepMeas.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_MeasStart(this)));
	m_vecSeqStepMeas.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_Idle(this)));

	// enumCIMStatus_MEAS_DONE
	// Meas Done
	// STS1106 -> 
	m_vecSeqStepMeasDone.clear();
	m_vecSeqStepMeasDone.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_None(this)));
	m_vecSeqStepMeasDone.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_MeasDone(this)));
	m_vecSeqStepMeasDone.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_Idle(this)));

	//
	// GDTREQ
	m_vecSeqStepDataUpload.clear();
	m_vecSeqStepDataUpload.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_None(this)));
	m_vecSeqStepDataUpload.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_GDTReq(this)));
	m_vecSeqStepDataUpload.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_Idle(this)));

	// enumCIMStatus_UNLOAD_READY
	// ULDREQ
	// STS2106 -> STS2107
	m_vecSeqStepUnloadReady.clear();
	m_vecSeqStepUnloadReady.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_None(this)));
	m_vecSeqStepUnloadReady.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_CheckUnLoadReq(this)));
	m_vecSeqStepUnloadReady.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_ChangeLoad(this)));
	m_vecSeqStepUnloadReady.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_UnLoadDone(this)));
	m_vecSeqStepUnloadReady.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_Idle(this)));

	// enumCIMStatus_UNLOAD
	// ULDCPL
	// ULDCPL_OK-> STS0004_OK
	m_vecSeqStepUnload.clear();
	m_vecSeqStepUnload.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_None(this)));
	m_vecSeqStepUnload.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_CheckUnLoadCpl(this)));
	m_vecSeqStepUnload.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_ChangeLoad(this)));
	m_vecSeqStepUnload.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_LoadReady(this)));
	m_vecSeqStepUnload.push_back(dynamic_cast<CDoWorkSequenceCIMStep*>(new CDoWorkSequenceCIMStep_Idle(this)));


	// 0.
	m_vecSeqStepALL.insert(m_vecSeqStepALL.end(), m_vecSeqStepInit.begin(), m_vecSeqStepInit.end());
	m_lCIMStatusStepStart[enumCIMStatus_INIT] = (long)m_vecSeqStepALL.size() - (long)m_vecSeqStepInit.size();
	m_lCIMStatusStepEnd[enumCIMStatus_INIT] = (long)m_vecSeqStepInit.size();

	// 1.
	m_vecSeqStepALL.insert(m_vecSeqStepALL.end(), m_vecSeqStepReady.begin(), m_vecSeqStepReady.end());
	m_lCIMStatusStepStart[enumCIMStatus_READY] = (long)m_vecSeqStepALL.size() - (long)m_vecSeqStepReady.size();
	m_lCIMStatusStepEnd[enumCIMStatus_READY] = (long)m_vecSeqStepReady.size();

	// 2.
	m_vecSeqStepALL.insert(m_vecSeqStepALL.end(), m_vecSeqSteploadReady.begin(), m_vecSeqSteploadReady.end());
	m_lCIMStatusStepStart[enumCIMStatus_LOAD_READY] = (long)m_vecSeqStepALL.size() - (long)m_vecSeqSteploadReady.size();
	m_lCIMStatusStepEnd[enumCIMStatus_LOAD_READY] = (long)m_vecSeqStepALL.size();

	// 3.
	m_vecSeqStepALL.insert(m_vecSeqStepALL.end(), m_vecSeqStepload.begin(), m_vecSeqStepload.end());
	m_lCIMStatusStepStart[enumCIMStatus_LOAD] = (long)m_vecSeqStepALL.size() - (long)m_vecSeqStepload.size();
	m_lCIMStatusStepEnd[enumCIMStatus_LOAD] = (long)m_vecSeqStepALL.size();

	// 4.
	m_vecSeqStepALL.insert(m_vecSeqStepALL.end(), m_vecSeqStepMeas.begin(), m_vecSeqStepMeas.end());
	m_lCIMStatusStepStart[enumCIMStatus_MEAS] = (long)m_vecSeqStepALL.size() - (long)m_vecSeqStepMeas.size();
	m_lCIMStatusStepEnd[enumCIMStatus_MEAS] = (long)m_vecSeqStepALL.size();

	// 5.
	m_vecSeqStepALL.insert(m_vecSeqStepALL.end(), m_vecSeqStepMeasDone.begin(), m_vecSeqStepMeasDone.end());
	m_lCIMStatusStepStart[enumCIMStatus_MEAS_DONE] = (long)m_vecSeqStepALL.size() - (long)m_vecSeqStepMeasDone.size();
	m_lCIMStatusStepEnd[enumCIMStatus_MEAS_DONE] = (long)m_vecSeqStepALL.size();

	// 6.
	m_vecSeqStepALL.insert(m_vecSeqStepALL.end(), m_vecSeqStepDataUpload.begin(), m_vecSeqStepDataUpload.end());
	m_lCIMStatusStepStart[enumCIMStatus_DATA_UPLOAD] = (long)m_vecSeqStepALL.size() - (long)m_vecSeqStepDataUpload.size();
	m_lCIMStatusStepEnd[enumCIMStatus_DATA_UPLOAD] = (long)m_vecSeqStepALL.size();

	// 7. 
	m_vecSeqStepALL.insert(m_vecSeqStepALL.end(), m_vecSeqStepUnloadReady.begin(), m_vecSeqStepUnloadReady.end());
	m_lCIMStatusStepStart[enumCIMStatus_UNLOAD_READY] = (long)m_vecSeqStepALL.size() - (long)m_vecSeqStepUnloadReady.size();
	m_lCIMStatusStepEnd[enumCIMStatus_UNLOAD_READY] = (long)m_vecSeqStepALL.size();

	// 8.
	m_vecSeqStepALL.insert(m_vecSeqStepALL.end(), m_vecSeqStepUnload.begin(), m_vecSeqStepUnload.end());
	m_lCIMStatusStepStart[enumCIMStatus_UNLOAD] = (long)m_vecSeqStepALL.size() - (long)m_vecSeqStepUnload.size();
	m_lCIMStatusStepEnd[enumCIMStatus_UNLOAD] = (long)m_vecSeqStepALL.size();
}

CDoWorkSequenceCIM::~CDoWorkSequenceCIM()
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

CDoWorkSequenceCIMStep* CDoWorkSequenceCIM::GetSeqItemStep()
{
	if (GetCurStep() < (long)m_vecSeqStepALL.size())
	{
		if (m_vecSeqStepALL[GetCurStep()])
		{
			return m_vecSeqStepALL[GetCurStep()];
		}
	}

	return nullptr;
}







//=========================================================================
// INTERFACE 관련
//=========================================================================
bool CDoWorkSequenceCIM::CreateDoWorkSeq()
{
	return true;
}

void CDoWorkSequenceCIM::DeleteDoWorkSeq()
{
}

bool CDoWorkSequenceCIM::ChangeState()
{
	DeleteDoWorkSeq();

	return CreateDoWorkSeq();
}

bool CDoWorkSequenceCIM::UpdateState()
{
	return true;
}

bool CDoWorkSequenceCIM::LinkDataBase(CBinaryHelper* pBinaryHelper, bool bSave)
{
	return true;
}

void CDoWorkSequenceCIM::ThreadRun()
{
	GoToInitStep();

	m_bThreadRun = true;
	for (size_t i = m_lCIMStatusStepStart[m_lCurrentStep]; i < m_lCIMStatusStepEnd[m_lCurrentStep]; ++i)
	{
		CDoWorkSequenceCIMStep* pSeqItem = m_vecSeqStepALL[i];
		if (pSeqItem)
		{
			if (pSeqItem->Run() == true) // 동작
			{
				GoToNextStep();
			}
			else if (GetCurStep() == GetExitStepIndex())
			{
				CString strLog;
				strLog.Format(_T("CDoWorkSequenceCIM Thread End"));
				m_pLogFileAccessor->KMLOG(INSP, strLog);

				m_bThreadRun = false;
				return;
			}
			else
			{
				int nCurStep = GetCurStep();
				GoToExitStep();

				int nExitStep = m_lCIMStatusStepStart[m_lCurrentStep] + GetCurStep();
				CString strLog;
				strLog.Format(_T("CDoWorkSequenceCIM Thread NG, Start = %d, Step = %d, Exit = %d"), m_lCIMStatusStepStart[m_lCurrentStep], nCurStep, nExitStep);
				m_pLogFileAccessor->KMLOG(WARN, strLog);

				//   CFunctionModuleProbeACCStep* pSeqItem = m_vecSeqStepALL[m_lEQStatusStepEnd[m_lCurrentStep] - 1];
				CDoWorkSequenceCIMStep* pSeqItem = m_vecSeqStepALL[nExitStep];
				if (pSeqItem)
				{
					pSeqItem->Run();
				}

				//CString strLog;
				//strLog.Format(_T("CDoWorkSequenceCIM Thread NG, Step = %d, Exit = %d"), GetCurStep(), GetExitStepIndex());
				//m_pLogFileAccessor->KMLOG(WARN, strLog);

				//GoToExitStep();
				//CDoWorkSequenceCIMStep* pSeqItem = GetSeqItemStep();
				//if (pSeqItem)
				//{
				//	pSeqItem->Run();
				//}
				
				m_bThreadRun = false;
				return;
			}

			if (GetStopUser() == true)	// 2023.08.09 add
			{
				int nCurStep = GetCurStep();
				GoToExitStep();

				int nExitStep = m_lCIMStatusStepStart[m_lCurrentStep] + GetCurStep();
				CString strLog;
				strLog.Format(_T("CDoWorkSequenceCIM Thread Stop, Start = %d, Step = %d, Exit = %d"), m_lCIMStatusStepStart[m_lCurrentStep], nCurStep, nExitStep);
				m_pLogFileAccessor->KMLOG(WARN, strLog);

				CDoWorkSequenceCIMStep* pSeqItem = m_vecSeqStepALL[nExitStep];
				if (pSeqItem)
				{
					pSeqItem->Run();
				}

				m_bThreadRun = false;
				return;
			}
		}
	}


	//GoToInitStep();

	//while (true)
	//{
	//	CDoWorkSequenceCIMStep* pSeqItem = GetSeqItemStep();
	//	if (pSeqItem)
	//	{
	//		if (GetCurStep() > 0)
	//		{
	//			if (pSeqItem->GetComplete() == true)
	//			{
	//				GoToExitStep();

	//				CDoWorkSequenceCIMStep* pSeqItem = GetSeqItemStep();
	//				if (pSeqItem)
	//				{
	//					pSeqItem->Run();
	//				}

	//				CString strLog;
	//				strLog.Format(_T("CDoWorkGlassSWAlignStep Thread End"));
	//				m_pLogFileAccessor->KMLOG(INSP, strLog);

	//				return;
	//			}
	//		}

	//	//	m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeST(0, pSeqItem->GetName(), GetCurStep());

	//		if (pSeqItem->Run() == true)
	//		{
	//	//		m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(0, pSeqItem->GetName(), GetCurStep());

	//			GoToNextStep();
	//		}
	//		else if (GetCurStep() == GetExitStepIndex())
	//		{
	//	//		m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(0, pSeqItem->GetName(), GetCurStep());

	//			CString strLog;
	//			strLog.Format(_T("CDoWorkSequenceCIMStep Thread End"));
	//			m_pLogFileAccessor->KMLOG(INSP, strLog);

	//			return;
	//		}
	//		else
	//		{
	//	//		m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(0, pSeqItem->GetName(), GetCurStep());

	//			CString strLog;
	//			strLog.Format(_T("CDoWorkSequenceCIMStep Thread NG, Step = %d"), GetCurStep());
	//			m_pLogFileAccessor->KMLOG(WARN, strLog);

	//			GoToExitStep();
	//			CDoWorkSequenceCIMStep* pSeqItem = GetSeqItemStep();
	//			if (pSeqItem)
	//			{
	//				pSeqItem->Run();
	//			}

	//			return;
	//		}
	//	}
	//}
}

void CDoWorkSequenceCIM::SetCIMStatusChangeStep(long lStep)
{
	m_lCurrentStep = lStep;

	SetCIMStatusExitStepIndex(lStep);
}

void CDoWorkSequenceCIM::SetCIMStatusExitStepIndex(long lStep)
{
	// TODO: 여기에 구현 코드 추가.
	switch (lStep)
	{
	case enumCIMStatus_INIT:
		SetExitStepIndex(m_vecSeqStepInit.size() - 1);
		break;
	case enumCIMStatus_READY:
		SetExitStepIndex(m_vecSeqStepReady.size() - 1);
		break;
	case enumCIMStatus_LOAD_READY:
		SetExitStepIndex(m_vecSeqSteploadReady.size() - 1);
		break;
	case enumCIMStatus_LOAD:
		SetExitStepIndex(m_vecSeqStepload.size() - 1);
		break;
	case enumCIMStatus_MEAS:
		SetExitStepIndex(m_vecSeqStepMeas.size() - 1);
		break;
	case enumCIMStatus_MEAS_DONE:
		SetExitStepIndex(m_vecSeqStepMeasDone.size() - 1);
		break;
	case enumCIMStatus_DATA_UPLOAD:
		SetExitStepIndex(m_vecSeqStepDataUpload.size() - 1);
		break;
	case enumCIMStatus_UNLOAD_READY:
		SetExitStepIndex(m_vecSeqStepUnloadReady.size() - 1);
		break;
	case enumCIMStatus_UNLOAD:
		SetExitStepIndex(m_vecSeqStepUnload.size() - 1);
		break;
	default:
		break;
	}
}
