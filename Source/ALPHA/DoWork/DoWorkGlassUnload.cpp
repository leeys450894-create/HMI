#include "pch.h"
#include "DoWorkGlassUnload.h"

#include "DoWorkGlassUnloadStep.h"
#include "..\SystemParamShare.h"
#include "LogFunction\LogAccessor\LogFileAccessor.h"
#include "..\AuroraFuncDefine.h"

#include "..\Manager\SeqTactTimeMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CDoWorkGlassUnload::CDoWorkGlassUnload(stAuroraMgrPool &MgrPool)
	: CDoWorkGlassSequenceImpl(MgrPool, enumSeqItemID_GLASS_UNLOAD)
	, m_pShareItem(this)
{
	m_strSeqType = _T("SEQUENCE_ITEM_DWGLASS_SEQ_GLASS_UNLOAD");
	m_strSeqComm = _T("TCPIP");
	m_strSeqCorp = _T("KMAC");
	m_strSeqName = _T("GLASS 8_UNLOAD");
	m_strSeqGUID = _T("30D1D0A3-7C67-4DDF-AE8E-E510D25B7304");
	m_strSeqDesc = _T("GLASS UNLOAD");

	// 로그 생성하기
	const CString strFolderPath	= CSystemParamShare::GetInstance()->GetLoggerFolderPath() + _T("\\LogFile\\");
	const CString strLogFile	= strFolderPath + m_strSeqName + _T(".log");
	MakeDir(strFolderPath);
	m_pLogFileAccessor->Initialize(strLogFile);

	// sequence step -
	m_vecSeqStep.clear();
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassUnloadStep*>(new CDoWorkGlassUnloadStep_None(this)));
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassUnloadStep*>(new CDoWorkGlassUnloadStep_Move_Safety(this)));		// Safety 위치 이동
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassUnloadStep*>(new CDoWorkGlassUnloadStep_CheckSaveReuslt(this)));	// 결과파일 완료 확인
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassUnloadStep*>(new CDoWorkGlassUnloadStep_SendToCIM(this)));			// CIM 으로 완료 보고
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassUnloadStep*>(new CDoWorkGlassUnloadStep_Idle(this)));

	
	//// sequence step -
	//m_vecSeqStep.clear();
	//m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassUnloadStep*>(new CDoWorkGlassUnloadStep_None(this)));
	//m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassUnloadStep*>(new CDoWorkGlassUnloadStep_Move_Safety(this)));
	////m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassUnloadStep*>(new CDoWorkGlassUnloadStep_ChangeUnloadStatus(this)));
	////m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassUnloadStep*>(new CDoWorkGlassUnloadStep_CheckUnloadStatus(this)));
	//m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassUnloadStep*>(new CDoWorkGlassUnloadStep_CheckSaveReuslt(this)));
	//m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassUnloadStep*>(new CDoWorkGlassUnloadStep_SendToCIM(this)));
	//m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassUnloadStep*>(new CDoWorkGlassUnloadStep_Idle(this)));

    SetTotalStepCount((long)m_vecSeqStep.size());
	SetExitStepIndex(GetTotalStepCount() - 1);
}

CDoWorkGlassUnload::~CDoWorkGlassUnload()
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

CDoWorkGlassUnloadStep* CDoWorkGlassUnload::GetSeqItemStep()
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
bool CDoWorkGlassUnload::CreateDoWorkSeq()
{
	return true;
}

void CDoWorkGlassUnload::DeleteDoWorkSeq()
{
}

bool CDoWorkGlassUnload::ChangeState()
{
	DeleteDoWorkSeq();

	return CreateDoWorkSeq();
}

bool CDoWorkGlassUnload::UpdateState()
{
	return true;
}

bool CDoWorkGlassUnload::LinkDataBase(CBinaryHelper *pBinaryHelper, bool bSave)
{
	return true;
}
	
void CDoWorkGlassUnload::ThreadRun()
{
	GoToInitStep();

	for (size_t i = 0; i < m_vecSeqStep.size(); ++i)
	{
		CDoWorkGlassUnloadStep *pSeqItem = m_vecSeqStep[i];
		if (pSeqItem)
		{
			SetBackupStepIndex(GetCurStep());

			m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeST(1, pSeqItem->GetName(), GetCurStep());

			if (pSeqItem->Run() == true) // 동작
			{
				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(1, pSeqItem->GetName(), GetCurStep());

				GoToNextStep();
			}
			else if (GetCurStep() == GetExitStepIndex())
			{
				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(1, pSeqItem->GetName(), GetCurStep());

				CString strLog;
				strLog.Format(_T("CDoWorkGlassUnloadStep Thread End"));
				m_pLogFileAccessor->KMLOG(INSP, strLog);
				break;
			}
			else
			{
				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(1, pSeqItem->GetName(), GetCurStep());

				CString strLog;
				strLog.Format(_T("CDoWorkGlassUnloadStep Thread NG, Step = %d, Exit = %d"), GetCurStep(), GetExitStepIndex());
				m_pLogFileAccessor->KMLOG(WARN, strLog);

				GoToExitStep();
				CDoWorkGlassUnloadStep* pSeqItem = GetSeqItemStep();
				if (pSeqItem)
				{
					pSeqItem->Run();
				}

				return;
			}

			if (GetStopUser() == true)	// 2023.08.09 add
			{
				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(1, pSeqItem->GetName(), GetBackupStepIndex());

				CString strLog;
				strLog.Format(_T("CDoWorkGlassUnloadStep Thread NG, Step = %d, Exit = %d"), GetBackupStepIndex(), GetExitStepIndex());
				m_pLogFileAccessor->KMLOG(WARN, strLog);

				GoToExitStep();
				CDoWorkGlassUnloadStep* pSeqItem = GetSeqItemStep();
				if (pSeqItem)
				{
					pSeqItem->Run();
				}

				return;
			}
		}
	}
}

