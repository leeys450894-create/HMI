#include "pch.h"
#include "CommTcpIpObjectAgingPG.h"

#include "Manager\NetworkConfigMgr.h"
#include "Manager\SystemConfigMgr.h"
#include "Manager/ShareMemoryMgr.h"
#include "Recipe\RecipeEditMgr.h"
#include "SystemParamShare.h"
#include "Alarm\AlarmController.h"
#include "AuroraFuncDefine.h"
#include "Manager\SequenceManager.h"

#include "CommEthFunction\EthernetManagerTCP.h"
#include "common/CommonFunc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCommTcpIpObjectAgingPG::CCommTcpIpObjectAgingPG(int iSeqItemIndex, stAuroraMgrPool& MgrPool)
	: m_pSystemConfigMgr(MgrPool.pSystemConfigMgr)
	, m_pRecipeEditMgr(MgrPool.pRecipeEditMgr)
	, m_pResultCollectMgr(MgrPool.pResultCollectMgr)
	, m_pShareMemoryMgr(MgrPool.pShareMemoryMgr)
	, m_pLogFileAccessor(new CLogFileAccessor)
	, m_bOutOfService(false)
	, m_iSeqItemIndex(0)
	, m_bAginging(false)
    , m_bAgingingEven(false)
	, m_bUseEnsisPG(false)
{
	m_strQueueMessage.Empty();
	m_strPGResult1.Empty();
	m_strPGResult2.Empty();
	m_strPGResult3.Empty();
	m_strPGResult4.Empty();

	m_pRecipeDataClone = nullptr;

	int iDeviceIndex = MgrPool.pNetworkConfigMgr->GetParam_DeviceIndex(iSeqItemIndex);

	m_iSeqItemIndex = enumTCPIpFactorytID_AGING;

	CString strObjectName;
	strObjectName.Format(_T("%s"), _T("AGING"));

	SetName(strObjectName); // _T("UTG"));
	const bool& bRunMode = MgrPool.pNetworkConfigMgr->GetParam_Mode(iSeqItemIndex);
	const CString& strIP = MgrPool.pNetworkConfigMgr->GetParam_IPNumber(iSeqItemIndex);
	const long& nPort = MgrPool.pNetworkConfigMgr->GetParam_PortNumger(iSeqItemIndex);
	SetNetInfo(bRunMode, strIP, nPort);

	m_bOutOfService = MgrPool.pNetworkConfigMgr->GetParam_OutOfService(iSeqItemIndex);

	// 로그 생성하기
	const CString strLogFilePath = MgrPool.strLogPath;
	const CString strFolderPath = strLogFilePath + _T("Control\\") + strObjectName + _T("\\");
	const CString strLogFile = strFolderPath + strObjectName + _T(".log");
	MakeDir(strFolderPath);
	m_pLogFileAccessor->Initialize(strLogFile);
	//////

	//Initialize();

	//m_pRecipeDataClone = new CRecipeData;
	//m_pRecipeDataClone->Clone(m_pRecipeEditMgr->GetRecipeDataPtr());

	// 데이터 수신 확인용 event
	for (int i = 0; i < enumRecvEventIDCount; ++i)
	{
		m_hRecvDEvent[i] = CreateEvent(NULL, FALSE, FALSE, NULL);	// auto reset, initial reset
	}

	m_bUseEnsisPG = true;

	ZeroMemory(&m_stAgingResult, sizeof(m_stAgingResult));
}


CCommTcpIpObjectAgingPG::~CCommTcpIpObjectAgingPG()
{
	Finalize();

	if (m_pRecipeDataClone)
	{
		delete m_pRecipeDataClone;
		m_pRecipeDataClone = nullptr;
	}

	for (int i = 0; i < enumRecvEventIDCount; ++i)
	{
		if (m_hRecvDEvent[i])
		{
			CloseHandle(m_hRecvDEvent[i]);
		}
	}

	//----------------------------------------------------------------------
	// Data 삭제
	//----------------------------------------------------------------------
	if (m_pLogFileAccessor)
	{
		delete m_pLogFileAccessor;
		m_pLogFileAccessor = nullptr;
	}
}

void CCommTcpIpObjectAgingPG::UpdateRecipeInformation()
{
	m_pRecipeDataClone->Clone(m_pRecipeEditMgr->GetRecipeDataPtr());
}

bool CCommTcpIpObjectAgingPG::SetTime_Sync()
{
	if (IsConnected() == false)
		return true;

	ResetEvent(m_hRecvDEvent[enumRecvEventID_TIME_SYNC_OK]);
	ResetEvent(m_hRecvDEvent[enumRecvEventID_TIME_SYNC_NG]);

	SYSTEMTIME st;
	::GetLocalTime(&st);
	CString sTime;
	sTime.Format(_T("%04d%02d%02d%02d%02d%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	CString m_strRecipeName;
	CString strReturnCommand = GetStringCMDID(COMM_USER_DEFINE_REQ) + _T("^TIME_SYNC^") + sTime;

	m_pLogFileAccessor->KMLOG(SEND, strReturnCommand);

	if (SendPacket(strReturnCommand) == true)
	{
		HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_TIME_SYNC_OK], m_hRecvDEvent[enumRecvEventID_TIME_SYNC_NG] };

		DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, DEFINE_PARAMETERS_WAIT_TIME);
		if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
		{
			//CAlarmController::GetInstance()->SetAlarm(SDC_DEF_MODULE_SOCKET_TIME_OUT);
		}

		return true;
	}

	return false;
}

bool CCommTcpIpObjectAgingPG::RecipeChange()
{
	int nRet = VTM_RV_SUCCESSED;

	ZeroMemory(&m_stAgingResult, sizeof(m_stAgingResult));

	// LYS 2026-06-30 OCP AGING PG device Download
	if (m_pSystemConfigMgr && m_pSystemConfigMgr->IsOCP())
	{
		nRet = _ExePGCtrl(enumTCPIpFactoryt_PGEvent_0, 5);
		if (nRet != VTM_RV_SUCCESSED)
			return false;
	}
	else
	{
		for (size_t i = 0; i < 4; i++)
		{
			nRet = _ExePGCtrl(enumTCPIpFactoryt_PGEvent_0, i + 1);
			if (nRet != VTM_RV_SUCCESSED)
				break;
		}
	}


	if (nRet == VTM_RV_SUCCESSED)
		return true;
	else
		return false;


#ifdef _USE_NOT
	CString strRecipeName = m_pRecipeEditMgr->GetRecipeID();
	CString strReturnCommand = GetStringCMDID(COMM_USER_DEFINE_REQ) + _T("^RECIPE_CHANGE^") + strRecipeName;

	CString strLog;
	strLog.Format(_T("[%s] : %s"), GetName(), strReturnCommand);
	m_pLogFileAccessor->KMLOG(SEND, strLog);

	ResetEvent(m_hRecvDEvent[enumRecvEventID_RECIPE_CHANGE_OK]);
	ResetEvent(m_hRecvDEvent[enumRecvEventID_RECIPE_CHANGE_NG]);

	if (IsOutOfService() == true)
		return true;

	if (SendPacket(strReturnCommand) == true)
	{
		HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_RECIPE_CHANGE_OK], m_hRecvDEvent[enumRecvEventID_RECIPE_CHANGE_NG] };

		DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, DEFINE_PARAMETERS_WAIT_TIME);
		if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
		{
			strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_TIMEOUT"));
			m_pLogFileAccessor->KMLOG(WARN, strLog);
			CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iSeqItemIndex);
			return false;
		}
		else if (dw == (WAIT_OBJECT_0 + 1)) // NG
		{
			strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_FAIL"));
			m_pLogFileAccessor->KMLOG(WARN, strLog);
			CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_FAIL + m_iSeqItemIndex);
			return false;
		}
		else
		{
			return true;
		}
	}

	strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_NOT_CONNECTED"));
	m_pLogFileAccessor->KMLOG(WARN, strLog);
	CAlarmController::GetInstance()->SetAlarm(HEAD_NOT_CONNECTED + m_iSeqItemIndex);
	return false;
#endif // _USE_NOT
}

bool CCommTcpIpObjectAgingPG::SetParameter()
{
	CString strCellID, strInnerID;
	strCellID = CSystemParamShare::GetInstance()->GetCellID();
	strInnerID = CSystemParamShare::GetInstance()->GetInnerID();

	CString strReturnCommand;
	strReturnCommand.Format(_T("%s^%s^%s^%s"), GetStringCMDID(COMM_USER_DEFINE_REQ), _T("INSP_PARAM"), strCellID, strInnerID);

	CString strLog;
	strLog.Format(_T("[%s] : %s"), GetName(), strReturnCommand);
	m_pLogFileAccessor->KMLOG(SEND, strLog);

	ResetEvent(m_hRecvDEvent[enumRecvEventID_SET_PARAMETER_OK]);
	ResetEvent(m_hRecvDEvent[enumRecvEventID_SET_PARAMETER_NG]);

	if (IsOutOfService() == true)
		return true;

	if (SendPacket(strReturnCommand) == true)
	{
		HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_SET_PARAMETER_OK], m_hRecvDEvent[enumRecvEventID_SET_PARAMETER_NG] };

		DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, DEFINE_RESPONSE_WAIT_TIME);
		if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
		{
			strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_TIMEOUT, SetParameter"));
			m_pLogFileAccessor->KMLOG(WARN, strLog);
			CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iSeqItemIndex);
			return false;
		}
		// jsyoon 2022.08.30 add
		else if (dw == (WAIT_OBJECT_0 + 1)) // NG
		{
			strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_FAIL, SetParameter"));
			m_pLogFileAccessor->KMLOG(WARN, strLog);
			CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_FAIL + m_iSeqItemIndex);
			return HEAD_ACK_FAIL;
		}
		else
		{
			;
		}

		return true;
	}

	strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_NOT_CONNECTED"));
	m_pLogFileAccessor->KMLOG(WARN, strLog);
	CAlarmController::GetInstance()->SetAlarm(HEAD_NOT_CONNECTED + m_iSeqItemIndex);

	return false;
}

bool CCommTcpIpObjectAgingPG::InspectStart(int iMeasType, int iCellNum, bool bWait)
{
	int nRet = VTM_RV_SUCCESSED;

	if (iCellNum == 0)
	{
		m_bAginging = true;

		//	for (size_t i = 0; i < 4; i++)
		//	{
		//		nRet = _ExePGCtrl(enumTCPIpFactoryt_PG_TEST_4, i+1);
		//	}

		//2022.07.31 jsyoon change 
		int nStick = 0;
		S_RecipeData* pRecipeData;
		pRecipeData = m_pRecipeEditMgr->GetRecipeData();
		if (pRecipeData != nullptr)
		{
			nStick = pRecipeData->m_vecStick.size();
		}

		if (iMeasType == 1)
		{
			m_bAgingingEven = true;

			// LYS 2026-06-30 OCP AGING PG Run
			if (m_pSystemConfigMgr && m_pSystemConfigMgr->IsOCP())
			{
				nRet = _ExePGCtrl(enumTCPIpFactoryt_PG_TEST_14, 5);
			}
			else
			{
				for (size_t i = 0; i < 4; i++)
				{
					if (m_pShareMemoryMgr->GetSharedMemory()->m_nAgingStartStick[i] > -1)
					{
						nRet = _ExePGCtrl(enumTCPIpFactoryt_PG_TEST_14, i + 1);
					}
				}
			}

		}
		else
		{
			m_bAgingingEven = false;

			// LYS 2026-06-30 OCP AGING PG Run
			if (m_pSystemConfigMgr && m_pSystemConfigMgr->IsOCP())
			{
				nRet = _ExePGCtrl(enumTCPIpFactoryt_PG_TEST_4, 5);
			}
			else
			{
				for (size_t i = 0; i < 4; i++)
				{
					if (m_pShareMemoryMgr->GetSharedMemory()->m_nAgingStartStick[i] > -1)
					{
						nRet = _ExePGCtrl(enumTCPIpFactoryt_PG_TEST_4, i + 1);
					}
				}
			}

		}

		/*switch (nStick)
		{
		case 2:
			for (size_t i = 3; i < 4; i++)
			{
				nRet = _ExePGCtrl(enumTCPIpFactoryt_PG_TEST_4, i + 1);
			}
			break;
		case 3:
			for (size_t i = 2; i < 4; i++)
			{
				nRet = _ExePGCtrl(enumTCPIpFactoryt_PG_TEST_4, i + 1);
			}
			break;
		case 4:
			for (size_t i = 1; i < 4; i++)
			{
				nRet = _ExePGCtrl(enumTCPIpFactoryt_PG_TEST_4, i + 1);
			}
			break;
		case 5:
			for (size_t i = 0; i < 4; i++)
			{
				nRet = _ExePGCtrl(enumTCPIpFactoryt_PG_TEST_4, i + 1);
			}
			break;
		default:
			for (size_t i = 0; i < 4; i++)
			{
				nRet = _ExePGCtrl(enumTCPIpFactoryt_PG_TEST_4, i + 1);
			}
			break;
		}*/
		////////////////////////////////////////////////////////////////
		if (nRet == VTM_RV_SUCCESSED)
			return true;
		else
			return false;
	}

	nRet = _ExePGCtrl(enumTCPIpFactoryt_PG_TEST_4, iCellNum);

	if (nRet == VTM_RV_SUCCESSED)
		return true;
	else
		return false;
}

bool CCommTcpIpObjectAgingPG::InspectStartWait()
{
	if (IsOutOfService() == true)
		return true;

	HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_INSP_START_OK], m_hRecvDEvent[enumRecvEventID_INSP_START_NG] };
	

	int nWaitTimeOut = DEFINE_PARAMETERS_WAIT_TIME;

	CString strLog;
	DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, nWaitTimeOut);
	if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
	{
		strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_TIMEOUT, InspectStartWait"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iSeqItemIndex);
		return false;
	}
	// jsyoon 2022.01.17 add
	else if (dw == (WAIT_OBJECT_0 + 1)) // NG
	{
		strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_FAIL, InspectStartWait"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_FAIL + m_iSeqItemIndex);
		return false;
	}
	else
	{
		return true;
	}

	return true;
}

bool CCommTcpIpObjectAgingPG::InspectEnd()
{
#ifdef _USE_NOT
	CString strReturnCommand;
	strReturnCommand.Format(_T("%s^%s"), GetStringCMDID(COMM_USER_DEFINE_REQ), _T("INSP_END"));

	CString strLog;
	strLog.Format(_T("[%s] : %s"), GetName(), strReturnCommand);
	m_pLogFileAccessor->KMLOG(SEND, strLog);

	ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_END_OK]);
	ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_END_NG]);

	if (IsOutOfService() == true)
		return true;

	if (SendPacket(strReturnCommand) == true)
	{
		HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_INSP_END_OK], m_hRecvDEvent[enumRecvEventID_INSP_END_NG] };

		DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, DEFINE_RESPONSE_WAIT_TIME);
		if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
		{
			strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_TIMEOUT"));
			m_pLogFileAccessor->KMLOG(WARN, strLog);
			CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iSeqItemIndex);
			return false;
		}
		// jsyoon 2022.01.17 add
		else if (dw == (WAIT_OBJECT_0 + 1)) // NG
		{
			strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_FAIL"));
			m_pLogFileAccessor->KMLOG(WARN, strLog);
			CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_FAIL + m_iSeqItemIndex);
			return false;
		}
		else
		{
			return true;
		}

		//	return true;
	}

	strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_NOT_CONNECTED"));
	m_pLogFileAccessor->KMLOG(WARN, strLog);
	CAlarmController::GetInstance()->SetAlarm(HEAD_NOT_CONNECTED + m_iSeqItemIndex);
	return false;
#endif // _USE_NOT

	return true;
}

bool CCommTcpIpObjectAgingPG::WaitInspectionEnd(int iPG, DWORD dwTimeOut)
{
	if (iPG == 0)
	{
		bool bRet = false;
		for (size_t i = 0; i < 4; i++)
		{
			if( i == 0)
				bRet = WaitInspectionEnd(i + 1, dwTimeOut);
			else
				bRet = WaitInspectionEnd(i + 1, 1);
		}

		return bRet;
	}
	else
	{
		if (IsOutOfService() == true)
			return true;

		if (IsConnected() == false)
			return false;

		CString strLog;

		int nEventOK = enumRecvEventID_INSP_END_OK, nEventNG = enumRecvEventID_INSP_END_NG;

		switch (iPG)
		{
		case 1:
			nEventOK = enumRecvEventID_INSP_END_OK, nEventNG = enumRecvEventID_INSP_END_NG;
			break;
		case 2:
			nEventOK = enumRecvEventID_INSP2_END_OK, nEventNG = enumRecvEventID_INSP2_END_NG;
			break;
		case 3:
			nEventOK = enumRecvEventID_INSP3_END_OK, nEventNG = enumRecvEventID_INSP3_END_NG;
			break;
		case 4:
			nEventOK = enumRecvEventID_INSP4_END_OK, nEventNG = enumRecvEventID_INSP4_END_NG;
			break;
		default:
			break;
		}

		HANDLE hRecvEvent[2] = { m_hRecvDEvent[nEventOK], m_hRecvDEvent[nEventNG] };

		DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, dwTimeOut * 1000);
		if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
		{
			strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_TIMEOUT, WaitInspectionEnd"));
			m_pLogFileAccessor->KMLOG(WARN, strLog);

			CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iSeqItemIndex);

			return false;
		}
		// jsyoon 2022.08.30 add
		else if (dw == (WAIT_OBJECT_0 + 1)) // NG
		{
			strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_FAIL, WaitInspectionEnd"));
			m_pLogFileAccessor->KMLOG(WARN, strLog);
			CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_FAIL + m_iSeqItemIndex);
			return false;
		}
		else
		{
			;
		}

		CString strSubData, strResult;
		strResult.Empty();

		switch (iPG)
		{
		case 1:
		{
			strResult = m_strPGResult1;
		}
			break;
		case 2:
		{
			strResult = m_strPGResult2;
		}
			break;
		case 3:
		{
			strResult = m_strPGResult3;
		}
			break;
		case 4:
		{
			strResult = m_strPGResult4;
		}
			break;
		default:
			break;
		}
		 
		if (strResult.IsEmpty() != true && (strResult.GetLength() == (2 + 60 + 10)))
		{
			// 전류 판정 영역
			// CRrAAA * 10
			for (size_t i = 0; i < 10; i++)
			{
				strSubData = strResult.Mid(2 + 6 * i, 6);

				// Cell Number
				strSubData.Mid(0, 1);

				// Rr
				strSubData.Mid(1, 1);
				strSubData.Mid(2, 1);

				// AAA
				strSubData.Mid(3, 3);
			}

			// 판정값
			// TTttmmmMMM
		}
	}
	

	return true;

	//CString strLog;

	//HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_INSP_END_OK], m_hRecvDEvent[enumRecvEventID_INSP_END_NG] };

	//DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, dwTimeOut);
	//if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
	//{
	//	strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_TIMEOUT"));
	//	m_pLogFileAccessor->KMLOG(WARN, strLog);

	//	CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iSeqItemIndex);

	//	return false;
	//}

	return true;
}

bool CCommTcpIpObjectAgingPG::WaitInspectionComplete(int iPG, DWORD dwTimeOut)
{
	int iPGNum = 0;
	if (iPG == 0)
	{
		bool bRet = false;

		//2022.07.31 jsyoon change 
		int nStick = 0;
		S_RecipeData* pRecipeData;
		pRecipeData = m_pRecipeEditMgr->GetRecipeData();
		if (pRecipeData != nullptr)
		{
			nStick = pRecipeData->m_vecStick.size();
		}
		
		// 2023.05.12 change 
		bool bStart = true;
		for (size_t i = 0; i < 4; i++)
		{
			if (m_pShareMemoryMgr->GetSharedMemory()->m_nAgingStartStick[i] > -1)
			{
				if (bStart == true)
				{
					bRet = WaitInspectionComplete(i + 1, dwTimeOut);
					bStart = false;
				}
				else
				{
					bRet = WaitInspectionComplete(i + 1, 5);
				}
			}
			else
			{
				// 2023.12.22 add
				bRet = true;
			}
		}

		//switch (nStick)
		//{
		//case 2:
		//	for (size_t i = 3; i < 4; i++)
		//	{
		//		bRet = WaitInspectionComplete(i + 1, dwTimeOut);
		//	}
		//	break;
		//case 3:
		//	for (size_t i = 2; i < 4; i++)
		//	{
		//		if (i == 2)
		//			bRet = WaitInspectionComplete(i + 1, dwTimeOut);
		//		else
		//			bRet = WaitInspectionComplete(i + 1, 5);
		//	}
		//	break;
		//case 4:
		//	for (size_t i = 1; i < 4; i++)
		//	{
		//		if (i == 1)
		//			bRet = WaitInspectionComplete(i + 1, dwTimeOut);
		//		else
		//			bRet = WaitInspectionComplete(i + 1, 5);
		//	}
		//	break;
		//case 5:
		//{
		//	// 2022.12.22 change Check
		//	bool bStart = true;
		//	for (size_t i = 0; i < 4; i++)
		//	{
		//		if (m_pShareMemoryMgr->GetSharedMemory()->m_nAgingStartStick[i] > -1)
		//		{
		//			if (bStart == true)
		//			{
		//				bRet = WaitInspectionComplete(i + 1, dwTimeOut);
		//				bStart = false;
		//			}
		//			else
		//			{
		//				bRet = WaitInspectionComplete(i + 1, 5);
		//			}
		//		}
		//	}

		//	//for (size_t i = 0; i < 4; i++)
		//	//{
		//	//	if (i == 0)
		//	//		bRet = WaitInspectionComplete(i + 1, dwTimeOut);
		//	//	else
		//	//		bRet = WaitInspectionComplete(i + 1, 5);
		//	//}
		//}
		//	break;
		//default:
		//{
		//	int j = 0;
		//	for (size_t i = 0; i < 4; i++)
		//	{
		//		if (m_pShareMemoryMgr->GetSharedMemory()->m_nAgingStartStick[i] > -1)
		//		{
		//			if (j == 0)
		//				bRet = WaitInspectionComplete(i + 1, dwTimeOut);
		//			else
		//				bRet = WaitInspectionComplete(i + 1, 5);

		//			j++;
		//		}
		//	}
		//}
		//	
		//	break;
		//}
		/////////////////////////////////////////////////////
		m_bAginging = false;

		return bRet;
	}
	else
	{
		int iStick = 0;
		CString strLog;
		int nEventOK = enumRecvEventID_INSP_COMPLETE_OK, nEventNG = enumRecvEventID_INSP_COMPLETE_NG;

		switch (iPG)
		{
		case 1:
			iPGNum = 0;
			nEventOK = enumRecvEventID_INSP_COMPLETE_OK, nEventNG = enumRecvEventID_INSP_COMPLETE_NG;
			iStick = 4;
			break;
		case 2:
			iPGNum = 1;
			nEventOK = enumRecvEventID_INSP2_COMPLETE_OK, nEventNG = enumRecvEventID_INSP2_COMPLETE_NG;
			iStick = 3;
			break;
		case 3:
			iPGNum = 2;
			nEventOK = enumRecvEventID_INSP3_COMPLETE_OK, nEventNG = enumRecvEventID_INSP3_COMPLETE_NG;
			iStick = 2;
			break;
		case 4:
			iPGNum = 3;
			nEventOK = enumRecvEventID_INSP4_COMPLETE_OK, nEventNG = enumRecvEventID_INSP4_COMPLETE_NG;
			iStick = 1;
			break;
		default:
			break;
		}

		strLog.Format(_T("[%s] : %s, PG %d"), GetName(), _T("WaitInspectionComplete Start"), iPGNum + 1);
		m_pLogFileAccessor->KMLOG(INSP, strLog);

		if (IsOutOfService() == true)
		{
			Sleep(100); 

			S_RecipeMgr* pslData = nullptr;
			pslData = m_pRecipeEditMgr->GetRecipeMgrPtr();
			int nStickCnt = 0, nCellCnt = 0;
			nCellCnt = pslData->m_nCellCnt;

		//	iStick += m_pShareMemoryMgr->GetSharedMemory()->m_nAgingStartStick;
			
			if (m_pShareMemoryMgr->GetSharedMemory()->m_nAgingStartStick[iPGNum] > -1)
				iStick = m_pShareMemoryMgr->GetSharedMemory()->m_nAgingStartStick[iPGNum];
			else
				return true;

			for (size_t i = 0; i < nCellCnt; i++)
			{
				if (nCellCnt > 10)
				{
					if (m_bAgingingEven != true)
					{
						if (i % 2)
						{
							continue;
						}
						else
						{
							if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS ||
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_START)
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_OK;
						}
					}
					else
					{
						if (i % 2)
						{
							if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS ||
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_START)
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_OK;
						}
					}
				}
				else
				{
					if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
						m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS ||
						m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_START)
						m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_OK;
				}
			}

			strLog.Format(_T("[%s] : %s, PG %d"), GetName(), _T("WaitInspectionComplete End"), iPGNum + 1);
			m_pLogFileAccessor->KMLOG(INSP, strLog);

			return true;
		}


		if (IsConnected() == false)
			return false;

		HANDLE hRecvEvent[2] = { m_hRecvDEvent[nEventOK], m_hRecvDEvent[nEventNG] };

		DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, dwTimeOut * 1000);
		if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
		{
			strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_TIMEOUT, WaitInspectionComplete"));
			m_pLogFileAccessor->KMLOG(WARN, strLog);

			CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iSeqItemIndex);

			return false;
		}
		else if (dw == (WAIT_OBJECT_0 + 1)) // NG
		{
			S_RecipeMgr* pslData = nullptr;
			pslData = m_pRecipeEditMgr->GetRecipeMgrPtr();
			int nStickCnt = 0, nCellCnt = 0;
			nCellCnt = pslData->m_nCellCnt;

		//	iStick += m_pShareMemoryMgr->GetSharedMemory()->m_nAgingStartStick;
			if (m_pShareMemoryMgr->GetSharedMemory()->m_nAgingStartStick[iPGNum] > -1)
				iStick = m_pShareMemoryMgr->GetSharedMemory()->m_nAgingStartStick[iPGNum];

			for (size_t i = 0; i < nCellCnt; i++)
			{
				if (nCellCnt > 10)
				{
					if (m_bAgingingEven == true)
					{
						if (i % 2)
						{
							if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS ||
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_START)
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_NG;
						}
					}
					else
					{
						if (i % 2)
						{
							continue;
						}
						else
						{
							if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS ||
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_START)
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_NG;
						}
					}
				}
				else
				{
					if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
						m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS ||
						m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_START)
						m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_NG;
				}
			}
		}
		else
		{
			S_RecipeMgr* pslData = nullptr;
			pslData = m_pRecipeEditMgr->GetRecipeMgrPtr();
			int nStickCnt = 0, nCellCnt = 0;
			nCellCnt = pslData->m_nCellCnt;

		//	iStick += m_pShareMemoryMgr->GetSharedMemory()->m_nAgingStartStick;

			if (m_pShareMemoryMgr->GetSharedMemory()->m_nAgingStartStick[iPGNum] > -1)
				iStick = m_pShareMemoryMgr->GetSharedMemory()->m_nAgingStartStick[iPGNum];
			else
				return true;

			for (size_t i = 0; i < nCellCnt; i++)
			{
				if (nCellCnt > 10)
				{
					if (m_bAgingingEven == true)
					{
						if (i % 2)
						{
							if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS ||
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_START)
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_OK;
						}
					}
					else
					{
						if (i % 2)
						{
							continue;
						}
						else
						{
							if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS ||
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_START)
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_OK;
						}
					}
				}
				else
				{
					if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
						m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS ||
						m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_START)
						m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_OK;
				}
			}

			//for (size_t i = 0; i < nCellCnt; i++)
			//{

			//	if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
			//		m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS ||
			//		m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_START)
			//		m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_OK;
			//}
		}

		strLog.Format(_T("[%s] : %s, PG %d"), GetName(), _T("WaitInspectionComplete End"), iPGNum + 1);
		m_pLogFileAccessor->KMLOG(INSP, strLog);

		
	}

	return true;
}


//=========================================================================
// INTERFACE 관련
//=========================================================================
void CCommTcpIpObjectAgingPG::xNetCallbackRecieveData(LPVOID ud)
{
	if ((IsConnected() == true) && (m_pSystemConfigMgr->IsCommunicationTypeString() == true))
	{
		LPRECEIVEDDATA pRcvData = (LPRECEIVEDDATA)ud;

		if (pRcvData->eCommState == (eXNET_COMM_STATE)XNET_NOTCONNECT)
		{
			CString strLog;
			strLog.Format(_T("[%s] : %s"), GetName(), _T("Close Connected"));
			m_pLogFileAccessor->KMLOG(SYST, strLog);

			SetEvent(m_hRecvDEvent[enumRecvEventID_RECIPE_CHANGE_NG]);
			SetEvent(m_hRecvDEvent[enumRecvEventID_SET_PARAMETER_NG]);
			SetEvent(m_hRecvDEvent[enumRecvEventID_INSP_START_NG]);
			SetEvent(m_hRecvDEvent[enumRecvEventID_INSP_END_NG]);
			SetEvent(m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_NG]);

			SetEvent(m_hRecvDEvent[enumRecvEventID_INSP2_START_NG]);
			SetEvent(m_hRecvDEvent[enumRecvEventID_INSP2_END_NG]);
			SetEvent(m_hRecvDEvent[enumRecvEventID_INSP2_COMPLETE_NG]);

			SetEvent(m_hRecvDEvent[enumRecvEventID_INSP3_START_NG]);
			SetEvent(m_hRecvDEvent[enumRecvEventID_INSP3_END_NG]);
			SetEvent(m_hRecvDEvent[enumRecvEventID_INSP3_COMPLETE_NG]);

			SetEvent(m_hRecvDEvent[enumRecvEventID_INSP4_START_NG]);
			SetEvent(m_hRecvDEvent[enumRecvEventID_INSP4_END_NG]);
			SetEvent(m_hRecvDEvent[enumRecvEventID_INSP4_COMPLETE_NG]);

			SetEvent(m_hRecvDEvent[enumRecvEventID_INSPECTION_READY_OK]);
			SetEvent(m_hRecvDEvent[enumRecvEventID_INSPECTION_READY_NG]);
			SetEvent(m_hRecvDEvent[enumRecvEventID_INSPECTION2_READY_OK]);
			SetEvent(m_hRecvDEvent[enumRecvEventID_INSPECTION2_READY_NG]);
			SetEvent(m_hRecvDEvent[enumRecvEventID_INSPECTION3_READY_OK]);
			SetEvent(m_hRecvDEvent[enumRecvEventID_INSPECTION3_READY_NG]);
			SetEvent(m_hRecvDEvent[enumRecvEventID_INSPECTION4_READY_OK]);
			SetEvent(m_hRecvDEvent[enumRecvEventID_INSPECTION4_READY_NG]);
		}
		else if (pRcvData->eCommState == (eXNET_COMM_STATE)XNET_RECEIVEDATA)
		{
			// jsyoon 2022.08.19 change
			CString strMessage;
			char* pchar = pRcvData->szBody;
			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pchar, -1, NULL, NULL);
			int nLen = pRcvData->sizeBody;

			wchar_t* pszWide = nullptr;
			pszWide = new wchar_t[nLen + 1];

			memset(pszWide, 0, sizeof(wchar_t) * nLen + 1);
			//wchar_t szWide[64000];
			//memset(szWide, 0, sizeof(wchar_t) * 64000);

			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pchar, nLen + 1, pszWide, nLen);

			for (int i = 0; i < nLen; i++)
			{
				if (pszWide[i] == 0X00)
				{
					pszWide[i] = ' ';
				}
			}

			pszWide[nLen] = 0X00;

			strMessage.Format(_T("%s"), pszWide);

			delete pszWide;
			pszWide = nullptr;

		//	CString strMessage(pRcvData->szBody);

			//	CString strLog = _T("[RECV - UTG] : ") + strMessage;
			CString strLog;
			strLog.Format(_T("[%s] : %s"), GetName(), strMessage);

			//		CLogWrapper::GetInstance()->KMLOG(SYST, strLog);
			m_pLogFileAccessor->KMLOG(RECV, strLog);

			strMessage.Replace(_T(" "), _T(""));
			if (strMessage.IsEmpty() == true)
				return;


			if (strMessage.Find(_T("$9")) < 0 && strMessage.Find(_T("@9")) < 0)
				return;

			CString strCmd;

		//	TCHAR c = 0X0D;
		//	strMessage += c;

			std::vector<CString> vecPacketCommandPool = GetExeHextoStrForElpAging(strMessage); //ExeHextoStr(strMessage);

			for (size_t i = 0; i < vecPacketCommandPool.size(); ++i)
			{
				const CString strRevCmd = vecPacketCommandPool[i];

				bool bAck = true;
				bool bOk = true;
				CString strPGCmd, strTmp;
				strPGCmd = strRevCmd;

				AGINGPGPROTOCOL stRcvPg;
				ZeroMemory(&stRcvPg, sizeof(stRcvPg));

				if (strPGCmd.GetLength() < 1)
				{
					SetEvent(m_hRecvDEvent[enumRecvEventID_SET_PARAMETER_NG]);
					return;
				}

				int nEventOK = enumRecvEventID_INSP_START_OK, nEventNG = enumRecvEventID_INSP_START_NG;

				//  시작 문자	  데이터 길이	  명령	     명령응답	  챔버 번호	  에러 사유						 상태코드						종료문자
				//	2 bytes	      2 bytes	    5 bytes	     1 byte	       2 bytes	  2 bytes						  N bytes						1 byte
				//	$9(fix)	        01~99	    명령 문자열	 1 - 명령성공
				//	                                         2 - 명령실패	01~챔버수 에러코드
				//																	  응답이 2일 경우에만 존재	명령이 STATU일 경우만 1 byte 존재
				//																								명령이 SETID인 경우 N bytes 존재	CR(0x0D)(fix)

				int nStart = 0, nError = 0;
				CString strData, strRcvCmd;
				strData.Empty();
				strRcvCmd.Empty();

				if (strPGCmd.Find(rSETID) > 0)
				{
					nStart = strPGCmd.Find(rSTR1) + 2;
					stRcvPg.iDataLen = _ttoi(strPGCmd.Mid(nStart, 2));
					strData = strPGCmd.Mid(nStart + 2 + 5, stRcvPg.iDataLen);

					stRcvPg.iResult = _ttoi(strData.Mid(0, 1));
					stRcvPg.iCellNum = _ttoi(strData.Mid(1, 2));

					if (stRcvPg.iResult == 2) // NG
					{
						stRcvPg.iError = _ttoi(strData.Mid(2, 2));

						bOk = false;
					}
					else
					{
						bOk = true;

						// TSP Aging Code 

					}

					nEventOK = enumRecvEventID_RECIPE_CHANGE_OK, nEventNG = enumRecvEventID_RECIPE_CHANGE_NG;

					strRcvCmd = rSETID;
				}
				else if (strPGCmd.Find(rSTATU) > 0)
				{
					nStart = strPGCmd.Find(rSTR1) + 2;
					stRcvPg.iDataLen = _ttoi(strPGCmd.Mid(nStart, 2));
					strData = strPGCmd.Mid(nStart + 2 + 5, stRcvPg.iDataLen);

					stRcvPg.iResult = _ttoi(strData.Mid(0, 1));
					stRcvPg.iCellNum = _ttoi(strData.Mid(1, 2));

					if (stRcvPg.iResult == 2) // NG
					{
						stRcvPg.iError = _ttoi(strData.Mid(2, 2));

						bOk = false;
					}
					else
					{
						bOk = true;
					}

					switch (stRcvPg.iCellNum)
					{
					case 1:
						nEventOK = enumRecvEventID_INSPECTION_READY_OK, nEventNG = enumRecvEventID_INSPECTION_READY_NG;
						break;
					case 2:
						nEventOK = enumRecvEventID_INSPECTION2_READY_OK, nEventNG = enumRecvEventID_INSPECTION2_READY_NG;
						break;
					case 3:
						nEventOK = enumRecvEventID_INSPECTION3_READY_OK, nEventNG = enumRecvEventID_INSPECTION3_READY_NG;
						break;
					case 4:
						nEventOK = enumRecvEventID_INSPECTION4_READY_OK, nEventNG = enumRecvEventID_INSPECTION4_READY_NG;
						break;
					default:
						break;
					}

				//	nEventOK = enumRecvEventID_INSPECTION_READY_OK, nEventNG = enumRecvEventID_INSPECTION_READY_NG;

					strRcvCmd = rSTATU;
				}
				else if (strPGCmd.Find(rSTART) > 0)
				{
					nStart = strPGCmd.Find(rSTR1) + 2;
					stRcvPg.iDataLen = _ttoi(strPGCmd.Mid(nStart, 2));
					strData = strPGCmd.Mid(nStart + 2 + 5, stRcvPg.iDataLen);

					stRcvPg.iResult = _ttoi(strData.Mid(0, 1));
					stRcvPg.iCellNum = _ttoi(strData.Mid(1, 2));

					if (stRcvPg.iResult == 2) // NG
					{
						stRcvPg.iError = _ttoi(strData.Mid(2, 2));

						bOk = false;
					}
					else
					{
						bOk = true;
						
						switch (stRcvPg.iCellNum)
						{
						case 1:
							nEventOK = enumRecvEventID_INSP_START_OK, nEventNG = enumRecvEventID_INSP_START_NG;
							break;
						case 2:
							nEventOK = enumRecvEventID_INSP2_START_OK, nEventNG = enumRecvEventID_INSP2_START_NG;
							break;
						case 3:
							nEventOK = enumRecvEventID_INSP3_START_OK, nEventNG = enumRecvEventID_INSP3_START_NG;
							break;
						case 4:
							nEventOK = enumRecvEventID_INSP4_START_OK, nEventNG = enumRecvEventID_INSP4_START_NG;
							break;
						default:
							break;
						}

						

					//	nEventOK = enumRecvEventID_INSP_START_OK, nEventNG = enumRecvEventID_INSP_START_NG;
					}

					strRcvCmd = rSTART;
				}
				else if (strPGCmd.Find(rSTOP) > 0)
				{
					nStart = strPGCmd.Find(rSTR1) + 2;
					stRcvPg.iDataLen = _ttoi(strPGCmd.Mid(nStart, 2));
					strData = strPGCmd.Mid(nStart + 2 + 5, stRcvPg.iDataLen);

					stRcvPg.iResult = _ttoi(strData.Mid(0, 1));
					stRcvPg.iCellNum = _ttoi(strData.Mid(1, 2));

					if (stRcvPg.iResult == 2) // NG
					{
						stRcvPg.iError = _ttoi(strData.Mid(2, 2));

						bOk = false;
					}
					else
					{
						bOk = true;
					}

					switch (stRcvPg.iCellNum)
					{
					case 1:
						nEventOK = enumRecvEventID_INSP_COMPLETE_OK, nEventNG = enumRecvEventID_INSP_COMPLETE_NG;
						break;
					case 2:
						nEventOK = enumRecvEventID_INSP2_COMPLETE_OK, nEventNG = enumRecvEventID_INSP2_COMPLETE_NG;
						break;
					case 3:
						nEventOK = enumRecvEventID_INSP3_COMPLETE_OK, nEventNG = enumRecvEventID_INSP3_COMPLETE_NG;
						break;
					case 4:
						nEventOK = enumRecvEventID_INSP4_COMPLETE_OK, nEventNG = enumRecvEventID_INSP4_COMPLETE_NG;
						break;
					default:
						break;
					}

					SetEvent(m_hRecvDEvent[nEventNG]);

					nEventOK = enumRecvEventID_INSPECTION_READY_OK, nEventNG = enumRecvEventID_INSPECTION_READY_NG;

					strRcvCmd = rSTOP;
				}
				else if (strPGCmd.Find(rRDATA) > 0)
				{
					nStart = strPGCmd.Find(rSTR2) + 2;
					stRcvPg.iDataLen = _ttoi(strPGCmd.Mid(nStart, 2));
					strData = strPGCmd.Mid(nStart + 2 + 5, stRcvPg.iDataLen);

					stRcvPg.iCellNum = _ttoi(strData.Mid(0, 2));
					

					if (stRcvPg.iDataLen > 5)	// 전류데이타
					{
						stRcvPg.iSeqNum = _ttoi(strData.Mid(2, 1)); 
						stRcvPg.iCmd = stRcvPg.iSeqNum;

						int nPgNum = 0;
						int nSeqNum = 0;
						nPgNum = stRcvPg.iCellNum - 1;
						nSeqNum = stRcvPg.iSeqNum - 1;
						
						if (nPgNum < 10 && nSeqNum < 30)
						{
							bool bReverse = false;
							if (stRcvPg.iSeqNum == 4)	// Reverse
							{
								bReverse = true;	// uA
							}

							m_stAgingResult[nPgNum][nSeqNum].bRecive = true;

							/*전류 값의 표현은 5자리로 표시되며, 다음과 같은 변환을 거친다.
							12345    1234.5 mA
							만일 시퀀스 번호가 4이면, Reverse 전류를 의미하므로 변환이 달라진다.
							12345    12.345 uA*/
							if (stRcvPg.iDataLen == (3 + 60 + 10))
							{
								//0XXXXX1XXXXX2XXXXX3XXXXX4XXXXX5XXXXX6XXXXX7XXXXX8XXXXX9XXXXX
								for (size_t i = 0; i < 10; i++)
								{
									int iCurrent = _ttoi(strData.Mid(3 + 6 * i + 1, 5));

									if (bReverse != true)
										m_stAgingResult[nPgNum][nSeqNum].dCurentData[i] = (double)iCurrent / 10; // Current mA
									else
										m_stAgingResult[nPgNum][nSeqNum].dCurentData[i] = (double)iCurrent / 1000; // Current uA
								}

								CString strValueMin, strValueMax;
								strValueMin = strData.Mid(3 + 6 * 10, 5);
								strValueMax = strData.Mid(3 + 6 * 10 + 5, 5);

								int iCurrentMin = _ttoi(strValueMin);
								int iCurrentMax = _ttoi(strValueMax);

								if (bReverse != true)
								{
									m_stAgingResult[nPgNum][nSeqNum].dCurentDataMin = (double)iCurrentMin / 10;	// Min

									m_stAgingResult[nPgNum][nSeqNum].dCurentDataMax = (double)iCurrentMax / 10;	// Max
								}
								else
								{
									m_stAgingResult[nPgNum][nSeqNum].dCurentDataMax = (double)iCurrentMin / 10000;	// Min

									m_stAgingResult[nPgNum][nSeqNum].dCurentDataMax = (double)iCurrentMax / 10000;	// Max
								}
							}
							else if (stRcvPg.iDataLen == (3 + 70 + 10))
							{
								// 0XXXXXJ1XXXXXJ2XXXXXJ3XXXXXJ4XXXXXJ5XXXXXJ6XXXXXJ7XXXXXJ8XXXXXJ9XXXXXJ
								for (size_t i = 0; i < 10; i++)
								{
									int iCurrent = _ttoi(strData.Mid(3 + 7 * i + 1, 5));

									if (bReverse != true)
										m_stAgingResult[nPgNum][nSeqNum].dCurentData[i] = (double)iCurrent / 10; // Current mA
									else
										m_stAgingResult[nPgNum][nSeqNum].dCurentData[i] = (double)iCurrent / 1000; // Current uA
								}

								int iCurrentMin = _ttoi(strData.Mid(3 + 7 * 10, 5));
								int iCurrentMax = _ttoi(strData.Mid(3 + 7 * 11, 5));

								if (bReverse != true)
								{
									m_stAgingResult[nPgNum][nSeqNum].dCurentDataMin = (double)iCurrentMin / 10;	// Min

									m_stAgingResult[nPgNum][nSeqNum].dCurentDataMax = (double)iCurrentMax / 10;	// Max
								}
								else
								{
									m_stAgingResult[nPgNum][nSeqNum].dCurentDataMax = (double)iCurrentMin / 10000;	// Min

									m_stAgingResult[nPgNum][nSeqNum].dCurentDataMax = (double)iCurrentMax / 10000;	// Max
								}
							}

							SaveAgingResult(nPgNum);
						}
					//	
					}
					else
					{
						stRcvPg.iCmd = _ttoi(strData.Mid(2, 1));

						if (stRcvPg.iCmd == 9)	// Start
						{
							stRcvPg.iSeqNum = _ttoi(strData.Mid(3, 1));	// 1: Monitoring, 2: TFT Aging1, 3: TFT Aging2, 4: REV Aging, 5: FWD Aging1, 6: FWD Aging2, 7: FWD Aging3, 8: TSP Aging, 9: FWD Aging4
						}
						else if (stRcvPg.iCmd == 8)	// End
						{
							
						} 
					}
					
					bAck = false;

					strRcvCmd.Format(_T("%s%d"), rRDATA, stRcvPg.iCmd);
				}
				else if (strPGCmd.Find(rJIDGE) > 0)
				{
					nStart = strPGCmd.Find(rSTR2) + 2;
					stRcvPg.iDataLen = _ttoi(strPGCmd.Mid(nStart, 2));
					strData = strPGCmd.Mid(nStart + 2 + 5, stRcvPg.iDataLen);

					stRcvPg.iCellNum = _ttoi(strData.Mid(0, 2));
					_stprintf_s(stRcvPg.szModel, _countof(stRcvPg.szModel), _T("%s"), strData.Mid(2, stRcvPg.iDataLen - 2));

					if (stRcvPg.iDataLen == (2 + 60 + 10))
					{
						switch (stRcvPg.iCellNum)
						{
						case 1:
							m_strPGResult1.Format(_T("%s"), strData);
							break;
						case 2:
							m_strPGResult2.Format(_T("%s"), strData);
							break;
						case 3:
							m_strPGResult3.Format(_T("%s"), strData);
							break;
						case 4:
							m_strPGResult4.Format(_T("%s"), strData);
							break;
						default:
							break;
						}

						//CString strSubData;

						//// 전류 판정 영역
						//// CRrAAA * 10
						//for (size_t i = 0; i < 10; i++)
						//{
						//	strSubData = strData.Mid(2 + 6 * i, 6);

						//	// Cell Number
						//	strSubData.Mid(0, 1);

						//	// Rr
						//	strSubData.Mid(1, 1);
						//	strSubData.Mid(2, 1);

						//	// AAA
						//	strSubData.Mid(3, 3);
						//}

						//// 판정값
						//// TTttmmmMMM
					}

					switch (stRcvPg.iCellNum)
					{
					case 1:
						nEventOK = enumRecvEventID_INSP_END_OK, nEventNG = enumRecvEventID_INSP_END_NG;
						break;
					case 2:
						nEventOK = enumRecvEventID_INSP2_END_OK, nEventNG = enumRecvEventID_INSP2_END_NG;
						break;
					case 3:
						nEventOK = enumRecvEventID_INSP3_END_OK, nEventNG = enumRecvEventID_INSP3_END_NG;
						break;
					case 4:
						nEventOK = enumRecvEventID_INSP4_END_OK, nEventNG = enumRecvEventID_INSP4_END_NG;
						break;
					default:
						break;
					}

					strRcvCmd = rJIDGE;
				}
				else if (strPGCmd.Find(rJOBEN) > 0)
				{
					nStart = strPGCmd.Find(rSTR2) + 2;

					stRcvPg.iCellNum = _ttoi(strPGCmd.Mid(nStart + 2 + 5, 2));

					switch (stRcvPg.iCellNum)
					{
					case 1:
						nEventOK = enumRecvEventID_INSP_COMPLETE_OK, nEventNG = enumRecvEventID_INSP_COMPLETE_NG;
						break;
					case 2:
						nEventOK = enumRecvEventID_INSP2_COMPLETE_OK, nEventNG = enumRecvEventID_INSP2_COMPLETE_NG;
						break;
					case 3:
						nEventOK = enumRecvEventID_INSP3_COMPLETE_OK, nEventNG = enumRecvEventID_INSP3_COMPLETE_NG;
						break;
					case 4:
						nEventOK = enumRecvEventID_INSP4_COMPLETE_OK, nEventNG = enumRecvEventID_INSP4_COMPLETE_NG;
						break;
					default:
						break;
					}
					

					strRcvCmd = rJOBEN;
				}
				else if (strPGCmd.Find(rCHANG) > 0)
				{
					nStart = strPGCmd.Find(rSTR2) + 2;
					stRcvPg.iDataLen = _ttoi(strPGCmd.Mid(nStart, 2));
					strData = strPGCmd.Mid(nStart + 2 + 5, stRcvPg.iDataLen);

					stRcvPg.iCellNum = _ttoi(strData.Mid(0, 2));
					_stprintf_s(stRcvPg.szModel, _countof(stRcvPg.szModel), _T("%s"), strData.Mid(2, stRcvPg.iDataLen - 2));
					bAck = false;

					strRcvCmd = rCHANG;
				}

				CString strLog;
				strLog.Format(_T("[%s] : %s, %d, %s, (ACK, %d), (OK, %d)"), GetName(), strRcvCmd, stRcvPg.iCellNum, strData, bAck, bOk);
				m_pLogFileAccessor->KMLOG(OPER, strLog);

				if (bAck == true)
				{
					if (bOk == true)
					{
						SetEvent(m_hRecvDEvent[nEventOK]);
					}
					else
					{
						SetEvent(m_hRecvDEvent[nEventNG]);
					}
				}
			}
		}
	}
}

CLogFileAccessor* CCommTcpIpObjectAgingPG::GetLogFileAccessorPtr()
{
	return m_pLogFileAccessor;
}

int CCommTcpIpObjectAgingPG::SetCommand(AGINGPGPROTOCOL stPg, bool bWait)
{
	TBYTE nByte = 0x30, nLen[2], nCmd[2], nStx[2], nEtx = 0x0D;
	TCHAR	szTmp[1000];
	TCHAR	szCmd[1000];
	szTmp[0] = 0x00;
	szCmd[0] = 0x00;
	nCmd[0] = 0x30;

	nStx[0] = '@';
	nStx[1] = '1';

	if (stPg.iCmd == 0)
		return PG_MODULE_PARAM_ERROR;

	CString strCmd, strData, strReturnCommand, strLog;

	// STX
	_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nStx[0]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
	_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nStx[1]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);

	int iStick = 0;
	int nEventOK = enumRecvEventID_SET_PARAMETER_OK, nEventNG = enumRecvEventID_SET_PARAMETER_NG;
	int iPG = 0;

	if (stPg.iCmd == 1)	// SETID
	{
		strCmd.Format(_T("SETID"));

		// PG Num(2) + Model Name
		strData.Format(_T("%02d%s"), stPg.iCellNum, stPg.szModel);

		nEventOK = enumRecvEventID_RECIPE_CHANGE_OK;
		nEventNG = enumRecvEventID_RECIPE_CHANGE_NG;

		switch (stPg.iCellNum)
		{
		case 1:
		{
			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSPECTION_READY_OK]);
			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSPECTION_READY_NG]);
		}
		break;
		case 2:
		{
			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSPECTION2_READY_OK]);
			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSPECTION2_READY_NG]);
		}
		break;
		case 3:
		{
			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSPECTION3_READY_OK]);
			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSPECTION3_READY_NG]);
		}
		break;
		case 4:
		{
			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSPECTION4_READY_OK]);
			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSPECTION4_READY_NG]);
		}
		break;
		default:
		{
			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSPECTION_READY_OK]);
			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSPECTION_READY_NG]);
		}
		break;
		}
	}
	else if (stPg.iCmd == 2) // START
	{
		strCmd.Format(_T("START"));

		if (m_bUseEnsisPG == true)
		{
	//		strData.Format(_T("%02d%d"), stPg.iCellNum, stPg.iOption);

			/*!
			* 2023.06.09 add
			* bettercell add CellID
			*/

			// GlassID
			CString strGlassID;
			strGlassID.Format(_T("%s"), CSystemParamShare::GetInstance()->GetGlassID());

			int iStick = 0, nMaxSitck = 0;
			
			// Stick Nnm(2)
			
			int iPGNum = stPg.iCellNum - 1;
			iStick = m_pShareMemoryMgr->GetSharedMemory()->m_nAgingStartStick[iPGNum] + 1;

			// Glass 방향(1)
			int iRotate = 1;
			S_RecipeInfo* pReciepInfo;
			pReciepInfo = m_pRecipeEditMgr->GetRecipeInfo();
			if (pReciepInfo != nullptr)
			{
				if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
				{
					iRotate = 0;
				}
			}

			// Stick Max
			CString strPart, strCellID;
			strPart.Format(_T("%s"), strGlassID.Mid(strGlassID.GetLength() - 2, 1));

			if (strPart.CompareNoCase(_T("B")) == 0)	// AB
			{
				S_RecipeData* pRecipeData;
				pRecipeData = m_pRecipeEditMgr->GetRecipeData();
				if (pRecipeData != nullptr)
				{
					if (iRotate == 0)
					{
						nMaxSitck = pRecipeData->m_vecStick.size();
					}
					else
					{
						; // nMaxSitck = pRecipeData->m_vecStick[0].m_vecCell.size();
					}
				}
			}

			strCellID.Format(_T("%s"), strGlassID.Left(strGlassID.GetLength() - 1));
		

			strData.Format(_T("%02d%d%-30s%02d%d"), stPg.iCellNum, stPg.iOption, strCellID, iStick + nMaxSitck, iRotate);
		}
		else
		{
			strData.Format(_T("%02d"), stPg.iCellNum);
		}

		// PG Num(2) + Flag + [모델 Index]
		//strData.Format(_T("%02d,%s,%s[%s]"), stPg.iCellNum, stPg.szModel);

		switch (stPg.iCellNum)
		{
		case 1:
		{
			iPG = 0;
			iStick = 4;
			nEventOK = enumRecvEventID_INSP_START_OK;		// 응답
			nEventNG = enumRecvEventID_INSP_START_NG;

			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_END_OK]);
			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_END_NG]);

			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_OK]);
			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_NG]);
		}
		break;
		case 2:
		{
			iPG = 1;
			iStick = 3;
			nEventOK = enumRecvEventID_INSP2_START_OK;		// 응답
			nEventNG = enumRecvEventID_INSP2_START_NG;

			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP2_END_OK]);
			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP2_END_NG]);


			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP2_COMPLETE_OK]);
			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP2_COMPLETE_NG]);
		}
		break;
		case 3:
		{
			iPG = 2;
			iStick = 2;
			nEventOK = enumRecvEventID_INSP3_START_OK;		// 응답
			nEventNG = enumRecvEventID_INSP3_START_NG;

			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP3_END_OK]);
			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP3_END_NG]);


			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP3_COMPLETE_OK]);
			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP3_COMPLETE_NG]);
		}
		break;
		case 4:
		{
			iPG = 3;
			iStick = 1;
			nEventOK = enumRecvEventID_INSP4_START_OK;		// 응답
			nEventNG = enumRecvEventID_INSP4_START_NG;

			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP4_END_OK]);
			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP4_END_NG]);


			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP4_COMPLETE_OK]);
			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP4_COMPLETE_NG]);
		}
		break;
		default:
			break;
		}

	}
	else if (stPg.iCmd == 3)	// STOP~
	{
		strCmd.Format(_T("STOP~"));

		strData.Format(_T("%02d"), stPg.iCellNum);

		nEventOK = enumRecvEventID_INSPECTION_READY_OK;
		nEventNG = enumRecvEventID_INSPECTION_READY_NG;

	}
	else if (stPg.iCmd == 4)	// STATU
	{
		strCmd.Format(_T("STATU"));

		strData.Format(_T("%02d"), stPg.iCellNum);

		nEventOK = enumRecvEventID_INSPECTION_READY_OK;
		nEventNG = enumRecvEventID_INSPECTION_READY_NG;
	}
	
	// Data Lan
	int len = strData.GetLength();
	nLen[0] = len / 10 + nByte;
	_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nLen[0]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
	nLen[1] = (len % 10) + nByte;
	_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nLen[1]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);

	// Command
	_stprintf_s(szTmp, _countof(szTmp), _T("%s"), strCmd); _tcscat_s(szCmd, _countof(szCmd), szTmp);	// Command

	// Data
	_stprintf_s(szTmp, _countof(szTmp), _T("%s"), strData); _tcscat_s(szCmd, _countof(szCmd), szTmp);	// Data

	_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nEtx);    _tcscat_s(szCmd, _countof(szCmd), szTmp);	// EXT
	int iLen = _tcsclen(szCmd);
	szCmd[iLen] = 0;
	strReturnCommand.Format(_T("%s"), szCmd);

	strLog.Format(_T("[%s] : %s"), GetName(), strReturnCommand);
	m_pLogFileAccessor->KMLOG(SEND, strLog);

	ResetEvent(m_hRecvDEvent[nEventOK]);
	ResetEvent(m_hRecvDEvent[nEventNG]);

	if (IsOutOfService() == true)
	{
		if (stPg.iCmd == 2)	// START
		{
			S_RecipeMgr* pslData = nullptr;
			pslData = m_pRecipeEditMgr->GetRecipeMgrPtr();
			int nStickCnt = 0, nCellCnt = 0;
			nCellCnt = pslData->m_nCellCnt;

		//	iStick += m_pShareMemoryMgr->GetSharedMemory()->m_nAgingStartStick;

			for (size_t i = 0; i < nCellCnt; i++)
			{
				if (m_pShareMemoryMgr->GetSharedMemory()->m_nAgingStartStick[iPG] > -1)
					iStick = m_pShareMemoryMgr->GetSharedMemory()->m_nAgingStartStick[iPG];
				else
					continue;


				if (nCellCnt > 10)
				{
					if (m_bAgingingEven == true)
					{
						if (i % 2)
						{
							if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS)
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_START;
						}
					}
					else
					{
						if (i % 2)
						{
							continue;
						}
						else
						{
							if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS)
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_START;
						}
					}
				}
				else
				{
					if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
						m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS)
						m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_START;
				}
			}
		}

		return true;
	}
		

	if (SendPacket(strReturnCommand) == true)
	{
		if (bWait == true)
		{
			HANDLE hRecvEvent[2] = { m_hRecvDEvent[nEventOK], m_hRecvDEvent[nEventNG] };

			DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, DEFINE_INSPECITON_WAIT_TIME /*DEFINE_PARAMETERS_WAIT_TIME*/);
			if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
			{
				strLog.Format(_T("[%s] : %s, %s"), GetName(), _T("SetCommand, HEAD_ACK_TIMEOUT"), strReturnCommand);
				m_pLogFileAccessor->KMLOG(WARN, strLog);
				CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iSeqItemIndex);
				return HEAD_ACK_TIMEOUT;
			}
			else if (dw == (WAIT_OBJECT_0 + 1)) // NG
			{
				strLog.Format(_T("[%s] : %s, %s"), GetName(), _T("SetCommand, HEAD_ACK_FAIL"), strReturnCommand);
				m_pLogFileAccessor->KMLOG(WARN, strLog);
				CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_FAIL + m_iSeqItemIndex);
				return HEAD_ACK_FAIL;
			}
			else
			{
				if (stPg.iCmd == 2)	// START
				{
					S_RecipeMgr* pslData = nullptr;
					pslData = m_pRecipeEditMgr->GetRecipeMgrPtr();
					int nStickCnt = 0, nCellCnt = 0;
					nCellCnt = pslData->m_nCellCnt;

				//	iStick += m_pShareMemoryMgr->GetSharedMemory()->m_nAgingStartStick;
					if (m_pShareMemoryMgr->GetSharedMemory()->m_nAgingStartStick[iPG] > -1)
						iStick = m_pShareMemoryMgr->GetSharedMemory()->m_nAgingStartStick[iPG];

					for (size_t i = 0; i < nCellCnt; i++)
					{
						if (nCellCnt > 10)
						{
							if (m_bAgingingEven != true)
							{
								if (i % 2)
								{
									continue;
								}
								else
								{
									if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
										m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS)
										m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_START;
								}
							}
							else
							{
								if (i % 2)
								{
									if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
										m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS)
										m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_START;
								}
							}
						}
						else
						{
							if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS)
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_START;
						}

						//if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE    || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
						//	m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS)
						//	m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_START;
					}
				}

				return VTM_RV_SUCCESSED;
			}
		}
		else
		{
			return VTM_RV_SUCCESSED;
		}	
	}

	strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_NOT_CONNECTED"));
	m_pLogFileAccessor->KMLOG(WARN, strLog);
	CAlarmController::GetInstance()->SetAlarm(HEAD_NOT_CONNECTED + m_iSeqItemIndex);
	return HEAD_NOT_CONNECTED;
}

int CCommTcpIpObjectAgingPG::WaitCommandRecive(DWORD dwTimeOut, int iPG)
{
	if (IsOutOfService() == true)
		return VTM_RV_SUCCESSED;

	if (IsConnected() == false)
		return HEAD_NOT_CONNECTED;

	CString strLog;
	int nEventOK = enumRecvEventID_INSP_START_OK, nEventNG = enumRecvEventID_INSP_START_NG;

	switch (iPG)
	{
	case 1:
		nEventOK = enumRecvEventID_INSPECTION_READY_OK, nEventNG = enumRecvEventID_INSPECTION_READY_NG;
		break;
	case 2:
		nEventOK = enumRecvEventID_INSPECTION2_READY_OK, nEventNG = enumRecvEventID_INSPECTION2_READY_NG;
		break;
	case 3:
		nEventOK = enumRecvEventID_INSPECTION3_READY_OK, nEventNG = enumRecvEventID_INSPECTION3_READY_NG;
		break;
	case 4:
		nEventOK = enumRecvEventID_INSPECTION4_READY_OK, nEventNG = enumRecvEventID_INSPECTION4_READY_NG;
		break;
	default:
		break;
	}


//	HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_RECIPE_CHANGE_OK], m_hRecvDEvent[enumRecvEventID_RECIPE_CHANGE_NG] };
	HANDLE hRecvEvent[2] = { m_hRecvDEvent[nEventOK], m_hRecvDEvent[nEventNG] };

	DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, dwTimeOut * 1000);
	if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
	{
		strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_TIMEOUT, WaitCommandRecive"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);

		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iSeqItemIndex);

		return HEAD_ACK_TIMEOUT;
	}
	// jsyoon 2022.08.30 add
	else if (dw == (WAIT_OBJECT_0 + 1)) // NG
	{
		strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_FAIL, WaitCommandRecive"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_FAIL + m_iSeqItemIndex);
		return HEAD_ACK_FAIL;
	}
	else
	{
		;
	}

	return VTM_RV_SUCCESSED;
}

int	 CCommTcpIpObjectAgingPG::_ExePGCtrl(int iEvent, int iCellNum, UINT nTimeOut)
{
	CString str;
	CString strLog;
	int nRtn = VTM_RV_SUCCESSED;

	AGINGPGPROTOCOL stPg;
	ZeroMemory(&stPg, sizeof(stPg));

//	UINT nTimeOut = DEFINE_RESPONSE_WAIT_TIME;	// Sec

	// Recipe Open
	if (iEvent == enumTCPIpFactoryt_PGEvent_0)
	{
		str.Format(_T("%s"), m_pRecipeEditMgr->GetRecipeID());

		// Model
		stPg.iCmd = 1;
		stPg.iCellNum = iCellNum;
		_tcscpy_s(stPg.szModel, _countof(stPg.szModel), str);

		strLog.Format(_T("[%s] : %s, PG %d"), GetName(), _T("Model Change Start"), iCellNum);
		m_pLogFileAccessor->KMLOG(INSP, strLog);

	//	strLog.Format(_T("Model Change Start"));
	//	m_pLogFileAccessor->KMLOG(OPER, strLog);

		nRtn = SetCommand(stPg);
	}
	else if (iEvent == enumTCPIpFactoryt_PGEvent_1)	// Wait
	{
		//	nTimeOut = m_pRecipeEditMgr->GetRecipeDataPtr()->GetParam_RecipeInfo()->   m_stRecipeMeas.nTimeout[0];

		nRtn = WaitCommandRecive(nTimeOut, iCellNum);
		if (nRtn != VTM_RV_SUCCESSED)
		{
			strLog.Format(_T("[%s] : %s, PG %d"), GetName(), _T("Model Change Fail"), iCellNum);
	//		strLog.Format(_T("Model Change Fail"));
		}
		else
		{
			strLog.Format(_T("[%s] : %s, PG %d"), GetName(), _T("Model Change End"), iCellNum);
	//		strLog.Format(_T("Model Change End"));
		}

		m_pLogFileAccessor->KMLOG(INSP, strLog);
	//	m_pLogFileAccessor->KMLOG(OPER, strLog);
	}
	else if (iEvent == enumTCPIpFactoryt_PG_TEST_4)	// AGING
	{
		stPg.iCmd = 2;
		stPg.iCellNum = iCellNum;
		stPg.iOption = 0;

		strLog.Format(_T("[%s] : %s, PG %d"), GetName(), _T("Aging Start"), iCellNum);
		m_pLogFileAccessor->KMLOG(INSP, strLog);

	//	nRtn = SetCommand(stPg, false);
		nRtn = SetCommand(stPg);
	}
	else if (iEvent == enumTCPIpFactoryt_PG_TEST_14)	// AGING EVEN
	{
		stPg.iCmd = 2;
		stPg.iCellNum = iCellNum;
		stPg.iOption = 1;

		strLog.Format(_T("[%s] : %s, PG %d"), GetName(), _T("AGING Even Start "), iCellNum);
		m_pLogFileAccessor->KMLOG(INSP, strLog);

	//	strLog.Format(_T("AGING Even START PG %d"), iCellNum);
	//	m_pLogFileAccessor->KMLOG(OPER, strLog);

		//	nRtn = SetCommand(stPg, false);
		nRtn = SetCommand(stPg);
	}
	else if (iEvent == enumTCPIpFactoryt_PG_TEST_5)	// STOP
	{
		stPg.iCmd = 3;
		stPg.iCellNum = iCellNum;

		nRtn = SetCommand(stPg, false);
	}
	else if (iEvent == enumTCPIpFactoryt_PGEvent_3)	// STATU
	{
		stPg.iCmd = 4;
		stPg.iCellNum = iCellNum;

		nRtn = SetCommand(stPg);
	}

	return nRtn;
}

bool CCommTcpIpObjectAgingPG::SetUserStop(int iCellNum)
{
	int nRet = VTM_RV_SUCCESSED;

	if (m_bAginging == true)
	{
		if (iCellNum == 0)
		{
			// LYS 2026-06-30 OCP AGING STOP
			if (m_pSystemConfigMgr && m_pSystemConfigMgr->IsOCP())
			{
				nRet = _ExePGCtrl(enumTCPIpFactoryt_PG_TEST_5, 5);
			}
			else
			{
				for (size_t i = 0; i < 4; i++)
				{
					nRet = _ExePGCtrl(enumTCPIpFactoryt_PG_TEST_5, i + 1);
				}
			}

	//		m_bAginging = false;

			if (nRet == VTM_RV_SUCCESSED)
				return true;
			else
				return false;
		}

		nRet = _ExePGCtrl(enumTCPIpFactoryt_PG_TEST_5, iCellNum);
	}

	if (nRet == VTM_RV_SUCCESSED)
		return true;
	else
		return false;
}

bool CCommTcpIpObjectAgingPG::RecipeChangeDone(DWORD dwTimeOut)
{
	int nRet = VTM_RV_SUCCESSED;

	// LYS 2026-06-30 OCP AGING PG device Download
	if (m_pSystemConfigMgr && m_pSystemConfigMgr->IsOCP())
	{
		nRet = _ExePGCtrl(enumTCPIpFactoryt_PGEvent_1, 5, dwTimeOut);
	}
	else
	{
		for (size_t i = 0; i < 4; i++)
		{
			nRet = _ExePGCtrl(enumTCPIpFactoryt_PGEvent_1, i + 1, dwTimeOut);
			if (nRet != VTM_RV_SUCCESSED)
				break;
		}
	}

	if (nRet == VTM_RV_SUCCESSED)
		return true;
	else
		return false;
}

int CCommTcpIpObjectAgingPG::SaveAgingResult(int nPgNum)
{
	// TODO: 여기에 구현 코드 추가.
	CString str, strTmp, strCurrent, strValue, strCellID, strGlassID, strMcrID;
	str.Empty();
	bool bRecive = false;

	int nMaxStick = 10, nArea = 0;

	// Cell ID
	strGlassID.Format(_T("%s"), CSystemParamShare::GetInstance()->GetGlassID());

	int iStick = m_pShareMemoryMgr->GetSharedMemory()->m_nAgingStartStick[nPgNum];
	int iRotate = 0;

	S_RecipeInfo* pReciepInfo;
	pReciepInfo = m_pRecipeEditMgr->GetRecipeInfo();
	if (pReciepInfo != nullptr)
	{
		if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
		{
			iRotate = 1;
		}
	}

	S_RecipeData* pRecipeData;
	pRecipeData = m_pRecipeEditMgr->GetRecipeData();
	if (pRecipeData != nullptr)
	{
		nMaxStick = pRecipeData->m_vecStick.size();
	}

	int nMaxTop = 0;
	if (iRotate == 1)
	{
		nMaxTop = pRecipeData->m_vecStick.size();
	}
	else
	{
		nMaxTop = pRecipeData->m_vecStick[0].m_vecCell.size();
	}

	S_RecipeMgr* pslData = nullptr;
	pslData = m_pRecipeEditMgr->GetRecipeMgrPtr();
	int nStickCnt = 0, nCellCnt = 0;
	nCellCnt = pslData->m_nCellCnt;

	std::vector<CString> vecResult;
	SYSTEMTIME st;
	::GetLocalTime(&st);
	CString sTime;
	sTime.Format(_T("%04d-%02d-%02d_%02d%02d%02d,"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	vecResult.push_back(sTime);

	strValue.Format(_T("%s,"), strGlassID);
	vecResult.push_back(strValue);

	for (int i = 0; i < 30; i++)
	{
		bRecive = m_stAgingResult[nPgNum][i].bRecive;
		if (bRecive == true)
		{
			int k = 0;
			for (int j = 0; j < 10; j++)
			{
				if (nCellCnt < 10)
				{
					if (j >= nCellCnt)
						continue;
				}
				else
				{
					if (j >= nCellCnt / 2)
					{
						continue;
					}
				}

				double dCurrent = 0.0;
				dCurrent = m_stAgingResult[nPgNum][i].dCurentData[j];
				if (dCurrent > 0)
				{
					// Generate CELL ID
					if (nCellCnt > 10)
					{
						if (m_bAgingingEven != true) // 홀수
						{
							if (iRotate == 1) // YY, BT
							{
								strTmp.Format(_T("%c%02d"), 65 + (j * 2), iStick + 1);

							}
							else // DG
							{
								strTmp.Format(_T("%c%02d"), 65 + nMaxStick - iStick - 1, (j * 2) + 1);
							}
						}
						else  // 짝수
						{
							if (iRotate == 1) // YY, BT
							{
								strTmp.Format(_T("%c%02d"), 65 + (j * 2) + 1, iStick + 1);

							}
							else // DG
							{
								strTmp.Format(_T("%c%02d"), 65 + nMaxStick - iStick - 1, (j + 1) * 2);
							}
						}
					}
					else
					{
						if (iRotate == 1) // YY, BT
						{
							strTmp.Format(_T("%c%02d"), 65 + j, iStick + 1);
						}
						else // DG
						{
							strTmp.Format(_T("%c%02d"), 65 + nMaxStick - iStick - 1, j + 1);
						}
					}

					// Glass Part A or B
					CString strPart;
					strPart.Format(_T("%s"), strGlassID.Mid(strGlassID.GetLength() - 2, 1));

					if (strPart.CompareNoCase(_T("B")) == 0)	// AB
					{
						CString strBuf;
						strBuf.Format(_T("%s%02d"), strTmp.Mid(0, 1), nMaxTop + _ttoi(strTmp.Mid(1, 2)));  // A01~A10
						strTmp.Format(_T("%s"), strBuf);

						//		strTmp.Format(_T("%s%02d"), strTmp.Mid(0, 1), nMaxTop + nMaxTop - iTop);
					}
					//////////////////////////////////////////////////////////////////////////

					strCellID.Format(_T("%s%s"), strGlassID.Left(strGlassID.GetLength() - 1), strTmp);
					strMcrID.Format(_T("%s"), gf_MakeMcrID(strCellID));

					if (i == 4)
						strValue.Format(_T("%s, %.3f uA, "), strMcrID, dCurrent);
					else
						strValue.Format(_T("%s, %.3f mA, "), strMcrID, dCurrent);

					str += strValue;

					vecResult.push_back(strValue);

				}
			}

			double dCurrent = 0.0;
			dCurrent = m_stAgingResult[nPgNum][i].dCurentDataMin;

			if (i == 4)
				strValue.Format(_T("Min, %.3f uA, "), dCurrent);
			else
				strValue.Format(_T("Min, %.3f mA, "), dCurrent);

			str += strValue;
			vecResult.push_back(strValue);

			dCurrent = m_stAgingResult[nPgNum][i].dCurentDataMax;

			if (i == 4)
				strValue.Format(_T("Max, %.3f uA, "), dCurrent);
			else
				strValue.Format(_T("Max, %.3f mA, "), dCurrent);

			str += strValue;
			vecResult.push_back(strValue);


			vecResult.push_back(_T("\r\n"));
		}
	}

	if (str.IsEmpty() != true)
	{
		m_pLogFileAccessor->KMLOG(INSP, str);

		//-------------------------------------------------------------------
		// CSV 를 저장한다.
		//-------------------------------------------------------------------
		CString strFilePath, strFileFullPath;
		strFilePath.Format(_T("%s\\AgingData\\"), CSystemParamShare::GetInstance()->GetResultFolderPath());
		MakeDir(strFilePath);
		strFileFullPath.Format(_T("%sAgingResult_%04d%02d%02d.csv"), strFilePath, st.wYear, st.wMonth, st.wDay);

		WriteDataFile(strFileFullPath, vecResult);
	}

	return 0;
}
