#include "pch.h"
#include "DoWorkGlassAdsorption.h"

#include "DoWorkGlassAdsorptionStep.h"
#include "..\SystemParamShare.h"
#include "LogFunction\LogAccessor\LogFileAccessor.h"
#include "..\AuroraFuncDefine.h"

#include "..\Manager\SeqTactTimeMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CDoWorkGlassAdsorption::CDoWorkGlassAdsorption(stAuroraMgrPool &MgrPool)
	: CDoWorkGlassSequenceImpl(MgrPool, enumSeqItemID_GLASS_ADSORPTION)
	, m_pShareItem(this)
{
	m_strSeqType = _T("SEQUENCE_ITEM_DWGLASS_SEQ_GLASS_ADSORPTION");
	m_strSeqComm = _T("TCPIP");
	m_strSeqCorp = _T("KMAC");
	m_strSeqName = _T("GLASS 1_ADSORPTION");
	m_strSeqGUID = _T("78346F9B-C67D-429E-B07F-B48A7D91A901");
	m_strSeqDesc = _T("GLASS ADSORPTION");

	// 로그 생성하기
	const CString strFolderPath	= CSystemParamShare::GetInstance()->GetLoggerFolderPath() + _T("\\LogFile\\");
	const CString strLogFile	= strFolderPath + m_strSeqName + _T(".log");
	MakeDir(strFolderPath);
	m_pLogFileAccessor->Initialize(strLogFile);
	
	// sequence step -
	m_vecSeqStep.clear();
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassAdsorptionStep*>(new CDoWorkGlassAdsorptionStep_None(this)));
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassAdsorptionStep*>(new CDoWorkGlassAdsorptionStep_Move(this)));				// 글라스 흡착 위치 이동
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassAdsorptionStep*>(new CDoWorkGlassAdsorptionStep_ChangeMeasStatus(this)));	// Measure 상태로 변경
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassAdsorptionStep*>(new CDoWorkGlassAdsorptionStep_CheckMeasStatus(this)));    // Measure 상태 확인
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassAdsorptionStep*>(new CDoWorkGlassAdsorptionStep_ProbeAlign(this)));			// Probe Align 확인
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassAdsorptionStep*>(new CDoWorkGlassAdsorptionStep_Idle(this)));

	//202103122
	/*
	DoWorkGlassAdsorption - 글라스 흡착
	1. Pre Align 사용 여부 확인 및 보정값 적용
		사용 여부와 상관 없이 보정값 적용해도 될듯(기본값은 0)
	2. Measure 상태로 변경
		2.1. L2M or R2M
	3. 정상적으로 글래스 흡착되어 있는지 확인
	*/

    SetTotalStepCount((long)m_vecSeqStep.size());
	SetExitStepIndex(GetTotalStepCount() - 1);
}

CDoWorkGlassAdsorption::~CDoWorkGlassAdsorption()
{	
	// sequence step -
	for(size_t i = 0; i < m_vecSeqStep.size(); ++i)
	{
		if(m_vecSeqStep[i])
		{
			delete m_vecSeqStep[i];
			m_vecSeqStep[i] = nullptr;
		}
	}

	DeleteDoWorkSeq();
}

CDoWorkGlassAdsorptionStep* CDoWorkGlassAdsorption::GetSeqItemStep()
{
	if(GetCurStep() < (long)m_vecSeqStep.size())
	{
		if(m_vecSeqStep[GetCurStep()])
		{
			return m_vecSeqStep[GetCurStep()];
		}
	}

	return nullptr;
}






//=========================================================================
// INTERFACE 관련
//=========================================================================
bool CDoWorkGlassAdsorption::CreateDoWorkSeq()
{
	return true;
}

void CDoWorkGlassAdsorption::DeleteDoWorkSeq()
{
}

bool CDoWorkGlassAdsorption::ChangeState()
{
	DeleteDoWorkSeq();

	return CreateDoWorkSeq();
}

bool CDoWorkGlassAdsorption::UpdateState()
{
	return true;
}

bool CDoWorkGlassAdsorption::LinkDataBase(CBinaryHelper *pBinaryHelper, bool bSave)
{
	return true;
}
	
void CDoWorkGlassAdsorption::ThreadRun()
{
	GoToInitStep();  // index Init

	for (size_t i = 0; i < m_vecSeqStep.size(); ++i)
	{
		CDoWorkGlassAdsorptionStep *pSeqItem = m_vecSeqStep[i];
		if (pSeqItem)
		{
			SetBackupStepIndex(GetCurStep());

			m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeST(1, pSeqItem->GetName(), GetCurStep());  // Start Time

			if (pSeqItem->Run() == true) // 동작
			{
				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(1, pSeqItem->GetName(), GetCurStep());  // End Time

				GoToNextStep();		// Next Step
			}
			else if (GetCurStep() == GetExitStepIndex())  // 마지막 Step
			{
				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(1, pSeqItem->GetName(), GetCurStep());  // End Time

				CString strLog;
				strLog.Format(_T("CDoWorkGlassAdsorptionStep Thread End"));
				m_pLogFileAccessor->KMLOG(INSP, strLog);

				return; // 종료
			}
			else  // NG 발생
			{
				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(1, pSeqItem->GetName(), GetCurStep());  // End Time

				CString strLog;
				strLog.Format(_T("CDoWorkGlassAdsorptionStep Thread NG, Step = %d/%d"), GetCurStep(), GetExitStepIndex());
				m_pLogFileAccessor->KMLOG(WARN, strLog);

				GoToExitStep();	// 마지막 Step 셋팅
				CDoWorkGlassAdsorptionStep* pSeqItem = GetSeqItemStep();
				if (pSeqItem)
				{
					pSeqItem->Run();
				}

				return;  // 종료
			}

			if (GetStopUser() == true)	// 2023.08.09 add
			{
				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(1, pSeqItem->GetName(), GetBackupStepIndex());  // End Time

				CString strLog;
				strLog.Format(_T("CDoWorkGlassAdsorptionStep Thread Stop, Step = %d/%d"), GetBackupStepIndex(), GetExitStepIndex());
				m_pLogFileAccessor->KMLOG(WARN, strLog);

				GoToExitStep();	// 마지막 Step 셋팅
				CDoWorkGlassAdsorptionStep* pSeqItem = GetSeqItemStep();
				if (pSeqItem)
				{
					pSeqItem->Run();
				}

				return;  // 종료
			}
		}
	}
}
