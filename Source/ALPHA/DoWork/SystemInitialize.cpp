#include "pch.h"
#include "SystemInitialize.h"

#include "LogFunction\LogAccessor\LogFileAccessor.h"

#include "SystemInitializeStep.h"
#include "..\SystemParamShare.h"
#include "..\Manager\ShareMemoryMgr.h"
#include "..\Manager\MotorControlMgr.h"
#include "..\AuroraFuncDefine.h"
#include "..\FunctionModule\FunctionModuleEQState.h"
#include "..\Manager\MCCLogMgr.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CSystemInitialize::CSystemInitialize(stAuroraMgrPool &MgrPool)
    : CBaseSequenceImpl(MgrPool, enumSeqItemID_INITIALIZE)
{
    m_strSeqType = _T("SEQUENCE_ITEM_INITIALIZE");
    m_strSeqComm = _T("TCPIP");
    m_strSeqCorp = _T("HBS");
    m_strSeqName = _T("SEQ INITIALIZE");
    m_strSeqGUID = _T("BFABDAE7-1381-4D6C-BC33-0610D62606F0");
    m_strSeqDesc = _T("SYSTEM INITIALIZE");

    const CString strFolderPath	= CSystemParamShare::GetInstance()->GetLoggerFolderPath() + _T("\\LogFile\\");
    const CString strLogFile	= strFolderPath + m_strSeqName + _T(".log");
    MakeDir(strFolderPath);
    m_pLogFileAccessor->Initialize(strLogFile);

	//jhseo 20210308	이니셜 시퀀스는 계속 수정중이며, 수정 후 불필요한 부분 삭제
/*	OM 기준
	CSystemInitializeStep_InitIO
		1. IO 연결
	CSystemInitializeStep_InitMotion
		2. Motion 연결
		3. 모션 이니셜 - RackJack
		3-1. IO 모니터링 Off
		3-2. RackJack 이니셜
		3-3. IO 모니터링 On
		4. 모션 이니셜 - 모든 축(RackJack 제외)
		4-1. IO 간섭 확인
		4-2. 이니셜
		4-3. 4-1, 4-2 반복
		5. Safety 위치 이동 - 글래스 위치에 따른 상태 변경
	CSystemInitializeStep_ConnectClient
		6. 클라이언트 연결
	CSystemInitializeStep_InitComplete
		7. 완료 체크
*/
	m_vecSeqStep.clear();
	m_vecSeqStep.push_back(dynamic_cast<CSystemInitializeStep*>(new CSystemInitializeStep_None(this)));
	m_vecSeqStep.push_back(dynamic_cast<CSystemInitializeStep*>(new CSystemInitializeStep_InitModule(this)));				// 모듈 연결
	m_vecSeqStep.push_back(dynamic_cast<CSystemInitializeStep*>(new CSystemInitializeStep_CheckInterlock(this)));
	m_vecSeqStep.push_back(dynamic_cast<CSystemInitializeStep*>(new CSystemInitializeStep_InitIO(this)));					// PLC 연결 및 출력 초기화
//	m_vecSeqStep.push_back(dynamic_cast<CSystemInitializeStep*>(new CSystemInitializeStep_InitMotion(this)));				// Motion 연결 및 모션 이니셜
	m_vecSeqStep.push_back(dynamic_cast<CSystemInitializeStep*>(new CSystemInitializeStep_MotionHoming1(this)));				// Motion 호밍
	m_vecSeqStep.push_back(dynamic_cast<CSystemInitializeStep*>(new CSystemInitializeStep_MotionHoming2(this)));				// Motion 호밍
	m_vecSeqStep.push_back(dynamic_cast<CSystemInitializeStep*>(new CSystemInitializeStep_MotionHoming3(this)));				// Motion 호밍
	m_vecSeqStep.push_back(dynamic_cast<CSystemInitializeStep*>(new CSystemInitializeStep_MotionHoming4(this)));				// Motion 호밍
	m_vecSeqStep.push_back(dynamic_cast<CSystemInitializeStep*>(new CSystemInitializeStep_MotionHoming5(this)));				// Motion 호밍
	m_vecSeqStep.push_back(dynamic_cast<CSystemInitializeStep*>(new CSystemInitializeStep_MotionHoming6(this)));				// Motion 호밍
	m_vecSeqStep.push_back(dynamic_cast<CSystemInitializeStep*>(new CSystemInitializeStep_MotionHoming7(this)));				// Motion 호밍
	m_vecSeqStep.push_back(dynamic_cast<CSystemInitializeStep*>(new CSystemInitializeStep_MotionHoming8(this)));				// Motion 호밍
	m_vecSeqStep.push_back(dynamic_cast<CSystemInitializeStep*>(new CSystemInitializeStep_MotionHoming9(this)));				// Motion 호밍
	m_vecSeqStep.push_back(dynamic_cast<CSystemInitializeStep*>(new CSystemInitializeStep_MotionHoming10(this)));				// Motion 호밍
	m_vecSeqStep.push_back(dynamic_cast<CSystemInitializeStep*>(new CSystemInitializeStep_MotionHoming11(this)));				// Motion 호밍
	m_vecSeqStep.push_back(dynamic_cast<CSystemInitializeStep*>(new CSystemInitializeStep_MotionHoming12(this)));				// Motion 호밍
	m_vecSeqStep.push_back(dynamic_cast<CSystemInitializeStep*>(new CSystemInitializeStep_MotionComplete(this)));				// Motion 호밍
	m_vecSeqStep.push_back(dynamic_cast<CSystemInitializeStep*>(new CSystemInitializeStep_InitIO(this)));						// PLC 연결 및 출력 초기화
	m_vecSeqStep.push_back(dynamic_cast<CSystemInitializeStep*>(new CSystemInitializeStep_Safety(this)));						// Move Safety
	m_vecSeqStep.push_back(dynamic_cast<CSystemInitializeStep*>(new CSystemInitializeStep_InitComplete(this)));					// 이니셜 완료 확인
	//m_vecSeqStep.push_back(dynamic_cast<CSystemInitializeStep*>(new CSystemInitializeStep_ProbeAlign(this)));					// ProbeAlign
	//m_vecSeqStep.push_back(dynamic_cast<CSystemInitializeStep*>(new CSystemInitializeStep_Safety(this)));						// Move Safety 
	m_vecSeqStep.push_back(dynamic_cast<CSystemInitializeStep*>(new CSystemInitializeStep_Idle(this)));

    SetTotalStepCount((long)m_vecSeqStep.size());
	SetExitStepIndex(GetTotalStepCount() - 1);
}

CSystemInitialize::~CSystemInitialize()
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

CSystemInitializeStep* CSystemInitialize::GetSeqItemStep()
{
	if (GetCurStep() < (long)m_vecSeqStep.size())
	{
		if (m_vecSeqStep[GetCurStep()])
		{
			return m_vecSeqStep[GetCurStep()];
		}
	}

	return nullptr;
}


//=========================================================================
// INTERFACE 관련
//=========================================================================
bool CSystemInitialize::CreateDoWorkSeq()
{
    return true;
}

void CSystemInitialize::DeleteDoWorkSeq()
{
}

bool CSystemInitialize::ChangeState()
{
    DeleteDoWorkSeq();

    return CreateDoWorkSeq();
}

bool CSystemInitialize::UpdateState()
{
    return true;
}

bool CSystemInitialize::LinkDataBase(CBinaryHelper *pBinaryHelper, bool bSave)
{
    return true;
}

void CSystemInitialize::ThreadRun()
{
	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_INIT_PROC, TRUE);

    for(size_t i = 0; i < m_vecSeqStep.size(); ++i)
    {
        CSystemInitializeStep *pSeqItem = m_vecSeqStep[i];
        if(pSeqItem)
        {
			SetBackupStepIndex(GetCurStep());

			if (pSeqItem->Run() == true) // 동작
			{	
				GoToNextStep();
			}
			else if (GetCurStep() == GetExitStepIndex())
			{
				CString strLog;
				strLog.Format(_T("CSystemInitialize Thread End"));
				m_pLogFileAccessor->KMLOG(SYST, strLog);

				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_INIT_PROC, FALSE);
				return;
			}
			else
			{
				CString strLog;
				strLog.Format(_T("CSystemInitialize Thread NG, Step = %d, Exit = %d"), GetBackupStepIndex(), GetExitStepIndex());
				m_pLogFileAccessor->KMLOG(WARN, strLog);

				GoToExitStep();
				CSystemInitializeStep* pSeqItem = GetSeqItemStep();
				if (pSeqItem)
				{
					pSeqItem->Run();
				}

				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_INIT_PROC, FALSE);
				return;
			}

			if (GetStopUser() == true)	// 2023.08.09 add
			{
				CString strLog;
				strLog.Format(_T("CSystemInitialize Thread Stop, Step = %d, Exit = %d"), GetBackupStepIndex(), GetExitStepIndex());
				m_pLogFileAccessor->KMLOG(WARN, strLog);

				GoToExitStep();
				CSystemInitializeStep* pSeqItem = GetSeqItemStep();
				if (pSeqItem)
				{
					pSeqItem->Run();
				}

				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_INIT_PROC, FALSE);
				return;
			}
        }
    }
}

