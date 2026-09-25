#include "pch.h"
#include "SACCMgr.h"

#include "common\IniHelper.h"
#include "..\..\Include\API_SACC.h"
#include "..\..\Include\API_ProbeHistory.h"
#include "Manager/SequenceManager.h"
#include "FunctionModule/FunctionModuleProbeACC.h"
#include "FunctionModule/FunctionModuleEQState.h"	// LYS 2026-09-16 SetEQManualProbeID
#include "SystemParamShare.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
//----------------------------------------------------------------------------------------------------
// Singleton
//----------------------------------------------------------------------------------------------------
CSACCMgr* CSACCMgr::m_pInstance = nullptr;
CSACCMgr* CSACCMgr::GetInstance()
{
	return m_pInstance ? m_pInstance : (m_pInstance = new CSACCMgr);
}

void CSACCMgr::DelInstance()
{
	if (m_pInstance)
	{
		delete m_pInstance;
		m_pInstance = nullptr;
	}
}

void CSACCMgr::Initialize(HWND hParentWnd, CRect rtROI, CString strAppName, CString strAppPath, CString strConfigPath, CString strLogPath)
{
	const CString strFilePath = strConfigPath + _T("\\CoreFunction\\CoreFunction.ini");
	m_pIniHelper->SetFilePath(strFilePath);

	LinkIniFile(false);

	if (m_bUse == true)
	{
		::API_SACC_Initialize(hParentWnd, rtROI);

		CString strProbeID, strProbeMountTime;
		int nProbeContact = 0;
		for (size_t i = 0; i < 5; i++)
		{
			// 파일에서 Probe 정보 읽어오기
			::API_SACC_GetProbeInfo(i, &strProbeID, &strProbeMountTime, &nProbeContact);

			m_strProbeCardID[i] = strProbeID;
			m_strProbeCardIDTime[i] = strProbeMountTime;
			m_nUseCnt[i] = nProbeContact;
		}
	}
}

void CSACCMgr::InitializeProbeLog(HWND hParentWnd, CRect rtROI, CString strAppName, CString strAppPath, CString strConfigPath, CString strLogPath)
{
	const CString strFilePath = strConfigPath + _T("\\CoreFunction\\CoreFunction.ini");
	m_pIniHelper->SetFilePath(strFilePath);

	LinkIniFile(false);

	if (m_bUse == true)
	{
		::ProbeHistory_InitializeReport(hParentWnd, rtROI);

		::ProbeHistory_SetSavePath(strLogPath);
	}
}

CWnd* CSACCMgr::GetReportmWnd()
{
	if (m_bUse == TRUE)
	{
		return API_SACC_GetReportmWnd();
	}
	else
	{
		return nullptr;
	}
}

CWnd* CSACCMgr::GetProbeLogWnd()
{
	if (m_bUse == TRUE)
	{
		return ProbeHistory_GetReportProbeHistoryWnd();
	}
	else
	{
		return nullptr;
	}
}

bool CSACCMgr::LinkIniFile(bool bSave)
{
	m_pIniHelper->Link(bSave, _T("SACC"), _T("USE"), &m_bUse, true);
	m_pIniHelper->Link(bSave, _T("SACC"), _T("OUT_OF_SERVICE"), &m_bOutOfService, true);

	return m_pIniHelper->IsReadEnable();
}

CSACCMgr::CSACCMgr(void)
	: m_pIniHelper(new CIniHelper)
	, m_bUse(false)
	, m_bOutOfService(false)
	, m_bSaccRun(false)
{
}

CSACCMgr::~CSACCMgr(void)
{
	Finalize();
}

bool CSACCMgr::IsOutOfService()
{
	return m_bOutOfService;
}

void CSACCMgr::Finalize()
{
	if (m_bUse == TRUE)
	{
		API_SACC_Finalize();

		::ProbeHistory_FinalizeReport();	// 2023.03.07 add
	}

	//----------------------------------------------------------------------
	// 데이터 삭제
	//----------------------------------------------------------------------
	if (m_pIniHelper)
	{
		delete m_pIniHelper;
		m_pIniHelper = nullptr;
	}
}

/**
*@brief Function : 공유메모리에서 수신된 이벤트 처리
*@details Using	 : SACC 동작 처리
*@date :
*@author Added by:
*/
void CSACCMgr::OnControlMsgReceive(WPARAM wParam, LPARAM lParam)
{
	PRITEMDATA nData = (PRITEMDATA)wParam;	

	int iEvent = 0, iMode = 0;
	iEvent = nData->nEventCmd;
	iMode = nData->nMeasMode;

	m_bSaccRun = true;

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_WORK_SACC_Start, NULL);

	switch (iEvent)
	{
	case SACC_COMMAND_OPERATION_GET_INSIDE:		// Motion
	{
		//if (m_nRcvCnt < 100)
		//{
		//	SetChuckMode(iMode);
		//	int i = 0;
		//	m_nMotionSeq[m_nRcvCnt] = nData->nWTSequence;
		//
		//	for (i = 0; i < 100; i++)
		//	{
		//		m_stMove[m_nRcvCnt].stAxis[i].bUse = nData->stDataM[i].bUse;
		//		m_stMove[m_nRcvCnt].stAxis[i].iAxis = nData->stDataM[i].nAxis;
		//		m_stMove[m_nRcvCnt].stAxis[i].dPos = nData->stDataM[i].dPos;
		//	}
		//
		//	m_nRcvCnt++;
		//
		//	SetEvent(m_hEvtWorkStart[EVT_CTRL_MOTION_W]);
		//}
		int iProbe = nData->nWTSequence;
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetPACCStatusChangeStep(enumPACCSTATUS_GETEQPROBE);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetProbe(iProbe);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->Start();
		break;
	}
	case SACC_COMMAND_OPERATION_PUT_INSIDE:
	{
		int iProbe = nData->nWTSequence;
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetPACCStatusChangeStep(enumPACCSTATUS_PUTEQPROBE);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetProbe(iProbe);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->Start();
		break;
	}
	// jsyoon 2022.05.09 add Probe Align
	case SACC_COMMAND_OPERATION_PROBE_ALIGN:
	{
		int iProbe = nData->nWTSequence;
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetPACCStatusChangeStep(enumPACCSTATUS_ALIGNPROBE);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetProbe(iProbe);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->Start();
		break;
	}

	case SACC_COMMAND_OPERATION_TEACH_AC_CHANGE:
	{
		int iProbe = nData->nWTSequence;
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetPACCStatusChangeStep(enumPACCSTATUS_AC_Y_CHANGEPOS);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetProbe(iProbe);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->Start();
		break;
	}
	case SACC_COMMAND_OPERATION_TEACH_AC_SAFETY: 
	{
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetPACCStatusChangeStep(enumPACCSTATUS_AC_Y_SAFETYPOS);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->Start();
		break;
	}
	case SACC_COMMAND_OPERATION_TEACH_AC_SACC:
	{
		int iProbe = nData->nWTSequence;
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetPACCStatusChangeStep(enumPACCSTATUS_AC_Y_SACCPOS);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->Start();
		break;
	}
	case SACC_COMMAND_OPERATION_AC_UP:
	{
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetPACCStatusChangeStep(enumPACCSTATUS_AC_Z_UP);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->Start();
		break;
	}
	case SACC_COMMAND_OPERATION_AC_DOWN:
	{
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetPACCStatusChangeStep(enumPACCSTATUS_AC_Z_DOWN);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->Start();
		break;
	}
	case SACC_COMMAND_OPERATION_PLC_BIT_PROBE_CARD_LOCK_ON:
	{
		int iProbe = nData->nWTSequence;		
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetPACCStatusChangeStep(enumPACCSTATUS_ZERO_LOCK_ON);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetProbe(iProbe);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->Start();
		break;
	}
	case SACC_COMMAND_OPERATION_PLC_BIT_PROBE_CARD_LOCK_OFF:
	{
		int iProbe = nData->nWTSequence;
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetPACCStatusChangeStep(enumPACCSTATUS_ZERO_LOCK_OFF);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetProbe(iProbe);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->Start();
		break;
	}
	case SACC_COMMAND_OPERATION_PLC_BIT_PROBE_UNIT_CLAMP_ON:
	{
		int iProbe = nData->nWTSequence;
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetPACCStatusChangeStep(enumPACCSTATUS_PROBE_CLAMP_ON);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetProbe(iProbe);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->Start();
		break;
	}
	case SACC_COMMAND_OPERATION_PLC_BIT_PROBE_UNIT_CLAMP_OFF:
	{
		int iProbe = nData->nWTSequence;
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetPACCStatusChangeStep(enumPACCSTATUS_PROBE_CLAMP_OFF);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetProbe(iProbe);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->Start();
		break;
	}
	case SACC_COMMAND_OPERATION_PLC_BIT_PROBE_PCB_LOCK_ON:
	{
		int iProbe = nData->nWTSequence;
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetPACCStatusChangeStep(enumPACCSTATUS_PROBE_PCB_ON);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetProbe(iProbe);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->Start();
		break;
	}
	case SACC_COMMAND_OPERATION_PLC_BIT_PROBE_PCB_LOCK_OFF:
	{
		int iProbe = nData->nWTSequence;
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetPACCStatusChangeStep(enumPACCSTATUS_PROBE_PCB_OFF);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetProbe(iProbe);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->Start();

		break;
	}
	case SACC_COMMAND_OPERATION_LODING:
	{
		int iProbe = nData->nWTSequence;
		int iLoadCassette = nData->nMeasMode;
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetPACCStatusChangeStep(enumPACCSTATUS_LODING);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetProbe(iProbe);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetLoadCassette(iLoadCassette);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->Start();
		break;
	}
	case SACC_COMMAND_OPERATION_UNLODING:
	{
		int iProbe = nData->nWTSequence;
		int iUnLoadCassette = nData->nMeasMode;
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetPACCStatusChangeStep(enumPACCSTATUS_UNLODING);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetProbe(iProbe);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetUnLoadCassette(iUnLoadCassette);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->Start();
		break;
	}
	// LYS 2026-09-16 수동 장착 프로브 ID 등록(SACC 화면 SET ID / CLEAR ID) -> 해당 프로브 모듈 워드 블록(P5 = D2150) 직접 쓰기. 시퀀스 스레드 없이 1회 쓰기라 Start() 없음.
	case SACC_COMMAND_OPERATION_SET_PROBEID:
	{
		int iProbe = nData->nWTSequence;
		CString strID(nData->szText);
		strID.Trim();
		bool bOK = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->SetEQManualProbeID(iProbe, strID);
		CString strLog;
		strLog.Format(_T("SACC SET_PROBEID probe=%d D%d id=[%s] -> %s"), iProbe, 2100 + 10 * iProbe, strID, bOK ? _T("OK") : _T("NG"));
		if (CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->GetLogFileAccessorPtr() != nullptr)
			CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->GetLogFileAccessorPtr()->KMLOG(bOK ? SYST : WARN, strLog);	// SEQ SACC 로그
		// 위에서 보낸 SACC_Start 표시를 되돌린다(시퀀스 명령은 Idle 스텝이 End 를 보낸다)
		CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_WORK_SACC_End, NULL);
		break;
	}
	case SACC_COMMAND_OPERATION_EXCHANGE:
	{
		int iProbe = nData->nWTSequence, iLoadCassette = nData->nMeasMode, iUnLoadCassette;
		iUnLoadCassette = iLoadCassette;
	//	int iLoadCassette = nData->nWTSequence, iUnLoadCassette = nData->nMeasMode;

		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetPACCStatusChangeStep(enumPACCSTATUS_EXCHANGE);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetProbe(iProbe);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetLoadCassette(iLoadCassette);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetUnLoadCassette(iUnLoadCassette);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->Start();
		break;
	}
	// Cassette 에서 선택한 Probe를 load Buffer에 넣기
	case SACC_COMMAND_OPERATION_CHANGE_SLOT_GET:
	{
		int iLoadCassette = nData->nWTSequence, iUnLoadCassette = nData->nMeasMode;
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetPACCStatusChangeStep(enumPACCSTATUS_SLOT_GET);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetLoadCassette(iLoadCassette);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->Start();
	}
	break;
	// load Buffer에서 Probe 가져오고 Safety로 이동
	case SACC_COMMAND_OPERATION_CHANGE_SACC_GET:
	{
		int iLoadCassette = nData->nWTSequence;
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetPACCStatusChangeStep(enumPACCSTATUS_PACC_GET);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetLoadCassette(iLoadCassette);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->Start();
	}
	break;
	// Cassette 에 Unload 선택, Unload Buffer 동작
	case SACC_COMMAND_OPERATION_CHANGE_SLOT_PUT:
	{
		int iProbe = nData->nWTSequence;
		int iUnLoadCassette = nData->nMeasMode;
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetPACCStatusChangeStep(enumPACCSTATUS_SLOT_PUT);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetProbe(iProbe);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetUnLoadCassette(iUnLoadCassette);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->Start();
	}
	break;
	// Probe 를 Unload Buffer 로 넣고 Safety로 이동
	case SACC_COMMAND_OPERATION_CHANGE_SACC_PUT:
	{
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetPACCStatusChangeStep(enumPACCSTATUS_PACC_PUT);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->Start();
	}
	break;
	// 동작완료
	case SACC_COMMAND_OPERATION_CHANGE_SACC_CLOSE:
	{
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetPACCStatusChangeStep(enumPACCSTATUS_PACC_DONE);
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->Start();
	}
	break;

	

	break;
	}
}

void CSACCMgr::Updata_Data()
{
	DITEMDATA data;
	ZeroMemory(&data, sizeof(data));

	// PLC
	CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->GetIOModuleProbeACC(&data.stDataP);

	//// Motion
	CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->GetMotionModuleProbeACC(data.stDataM);

	API_SACC__Updata_Data(&data);
}

// Probe ProbeHistory_Insert
void CSACCMgr::Probe_Mount(int iProbe, CString strRFID, CString strMountTime)
{
	::API_SACC_SetProbeInfo(iProbe, strRFID, strMountTime, 0);

	::ProbeHistory_Insert(strRFID, strMountTime);
}

// Probe ProbeHistory_Update_ContactCnt
void CSACCMgr::Probe_ContactCount(int iProbe, CString strRFID, CString strMountTime, int nContactCount)
{
	::API_SACC_SetProbeInfo(iProbe, strRFID, strMountTime, nContactCount);

	::ProbeHistory_Update_ContactCnt( strRFID,  strMountTime,  nContactCount);
}

// Probe ProbeHistory_Update_UnmountTime
void CSACCMgr::Probe_Unmount(int iProbe, CString strRFID, CString strMountTime, CString strUnmountTime)
{
	::API_SACC_SetProbeInfo(iProbe, _T(""), _T(""), 0);

	::ProbeHistory_Update_UnmountTime( strRFID,  strMountTime,  strUnmountTime);
}

void CSACCMgr::Probe_GetInfo(int iProbe, CString* strRFID, CString* strMountTime, int* nContactCount)
{
	if(strRFID !=nullptr)			*strRFID = m_strProbeCardID[iProbe];
	if (strMountTime != nullptr)   *strMountTime = m_strProbeCardIDTime[iProbe];
	if (nContactCount != nullptr)  *nContactCount = m_nUseCnt[iProbe];
}

bool CSACCMgr::Is_Complete()
{
	bool bRet = false;
	bRet = CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->Is_Complete();
	return bRet;
}

bool CSACCMgr::Is_Fail()
{
	bool bRet = false;
	bRet = CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->Is_Fail();
	return bRet;
}

bool CSACCMgr::Is_AlarmOn()
{
	bool bRet = false;
	bRet = CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->Is_AlarmOn();
	return bRet;
}