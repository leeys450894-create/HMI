#include "pch.h"
#include "DoWorkGlassSWAlign.h"

#include "DoWorkGlassSWAlignStep.h"
#include "..\SystemParamShare.h"
#include "LogFunction\LogAccessor\LogFileAccessor.h"
#include "..\AuroraFuncDefine.h"
#include "..\Manager\SeqTactTimeMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CDoWorkGlassSWAlign::CDoWorkGlassSWAlign(stAuroraMgrPool &MgrPool)
	: CDoWorkGlassSequenceImpl(MgrPool, enumSeqItemID_GLASS_SW_ALIGN)
	, m_pShareItem(this)
{
	m_strSeqType = _T("SEQUENCE_ITEM_DWGLASS_SEQ_GLASS_SW_ALIGN");
	m_strSeqComm = _T("TCPIP");
	m_strSeqCorp = _T("KMAC");
	m_strSeqName = _T("GLASS 3_GLASS_ALIGN");
	m_strSeqGUID = _T("7538403D-9121-442D-82BE-540A5BF40633");
	m_strSeqDesc = _T("GLASS SW ALIGN");

	// 로그 생성하기
	const CString strFolderPath	= CSystemParamShare::GetInstance()->GetLoggerFolderPath() + _T("\\LogFile\\");
	const CString strLogFile	= strFolderPath + m_strSeqName + _T(".log");
	MakeDir(strFolderPath);
	m_pLogFileAccessor->Initialize(strLogFile);
	
	// sequence step -
	m_vecSeqStep.clear();
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassSWAlignStep*>(new CDoWorkGlassSWAlignStep_None(this)));
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassSWAlignStep*>(new CDoWorkGlassSWAlignStep_MoveAlignPosistion(this)));	// 좌표 이동
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassSWAlignStep*>(new CDoWorkGlassSWAlignStep_AlignGlass(this)));			// Glass Align
	m_vecSeqStep.push_back(dynamic_cast<CDoWorkGlassSWAlignStep*>(new CDoWorkGlassSWAlignStep_Idle(this)));

    SetTotalStepCount((long)m_vecSeqStep.size());
	SetExitStepIndex(GetTotalStepCount() - 1);
}

CDoWorkGlassSWAlign::~CDoWorkGlassSWAlign()
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

CDoWorkGlassSWAlignStep* CDoWorkGlassSWAlign::GetSeqItemStep()
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
bool CDoWorkGlassSWAlign::CreateDoWorkSeq()
{
	return true;
}

void CDoWorkGlassSWAlign::DeleteDoWorkSeq()
{
}

bool CDoWorkGlassSWAlign::ChangeState()
{
	DeleteDoWorkSeq();

	return CreateDoWorkSeq();
}

bool CDoWorkGlassSWAlign::UpdateState()
{
	return true;
}

bool CDoWorkGlassSWAlign::LinkDataBase(CBinaryHelper *pBinaryHelper, bool bSave)
{
	return true;
}
	
void CDoWorkGlassSWAlign::ThreadRun()
{
	GoToInitStep();

	while (true)
	{
		CDoWorkGlassSWAlignStep *pSeqItem = GetSeqItemStep();
		if (pSeqItem) 
		{
			//if (GetCurStep() > 0)
			//{
			//	if (pSeqItem->GetComplete() == true)
			//	{
			//		GoToExitStep();

			//		CDoWorkGlassSWAlignStep *pSeqItem = GetSeqItemStep();
			//		if (pSeqItem)
			//		{
			//			pSeqItem->Run();
			//		}

			//		CString strLog;
			//		strLog.Format(_T("CDoWorkGlassSWAlignStep Thread End"));
			//		m_pLogFileAccessor->KMLOG(INSP, strLog);

			//		return;
			//	}
			//}

			SetBackupStepIndex(GetCurStep());

			m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeST(0, pSeqItem->GetName(), GetCurStep());

			if (pSeqItem->Run() == true) // 동작
			{
				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(0, pSeqItem->GetName(), GetCurStep());

				GoToNextStep();
			}
			else if (GetCurStep() == GetExitStepIndex())
			{
				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(0, pSeqItem->GetName(), GetCurStep());

				CString strLog;
				strLog.Format(_T("CDoWorkGlassSWAlignStep Thread End"));
				m_pLogFileAccessor->KMLOG(INSP, strLog);

				return;
			}
			else
			{
				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(0, pSeqItem->GetName(), GetCurStep());

				CString strLog;
				strLog.Format(_T("CDoWorkGlassSWAlignStep Thread NG, Step = %d, Exit = %d"), GetCurStep(), GetExitStepIndex());
				m_pLogFileAccessor->KMLOG(WARN, strLog);

				GoToExitStep();
				CDoWorkGlassSWAlignStep *pSeqItem = GetSeqItemStep();
				if (pSeqItem)
				{
					pSeqItem->Run();
				}

				return;
			}

			if (GetStopUser() == true)	// 2023.08.09 add
			{
				m_pShareItem->GetSeqTactTimeMgrPtr()->SetSeqItemTimeED(0, pSeqItem->GetName(), GetBackupStepIndex());

				CString strLog;
				strLog.Format(_T("CDoWorkGlassSWAlignStep Thread Stop, Step = %d, Exit = %d"), GetBackupStepIndex(), GetExitStepIndex());
				m_pLogFileAccessor->KMLOG(WARN, strLog);

				GoToExitStep();
				CDoWorkGlassSWAlignStep* pSeqItem = GetSeqItemStep();
				if (pSeqItem)
				{
					pSeqItem->Run();
				}

				return;
			}
		}
	}
}