#include "pch.h"
#include "CommTcpIpObjectPG.h"

#include "Manager\NetworkConfigMgr.h"
#include "Manager\SystemConfigMgr.h"
#include "Manager/ShareMemoryMgr.h"
#include "Recipe\RecipeEditMgr.h"
#include "SystemParamShare.h"
#include "Alarm\AlarmController.h"
#include "AuroraFuncDefine.h"
#include "Manager\SequenceManager.h"
#include "DoWork\AutoContactInfo.h"	// LYS 2026-08-10 AutoContact Phase 3

#include "CommEthFunction\EthernetManagerTCP.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCommTcpIpObjectPG::CCommTcpIpObjectPG(int iSeqItemIndex, stAuroraMgrPool& MgrPool)
	: m_pSystemConfigMgr(MgrPool.pSystemConfigMgr)
	, m_pRecipeEditMgr(MgrPool.pRecipeEditMgr)
	, m_pResultCollectMgr(MgrPool.pResultCollectMgr)
	, m_pShareMemoryMgr(MgrPool.pShareMemoryMgr)
	, m_pLogFileAccessor(new CLogFileAccessor)
	, m_bOutOfService(false)
	, m_iSeqItemIndex(0)
	, m_bAginging(false)
	, m_nAgingTime(0)
	, m_bUseEnsisPG(false)
	, m_bAgingingEven(false)
{
	m_strQueueMessage.Empty();
	m_strPGResult.Empty();
	m_pRecipeDataClone = nullptr;

//	m_iSeqItemIndex = MgrPool.pNetworkConfigMgr->GetParam_DeviceIndex(iSeqItemIndex);

	m_iSeqItemIndex = enumTCPIpFactorytID_PG;

	CString strObjectName;
	strObjectName.Format(_T("%s"), _T("PG"));

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
}


CCommTcpIpObjectPG::~CCommTcpIpObjectPG()
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

void CCommTcpIpObjectPG::UpdateRecipeInformation()
{
	m_pRecipeDataClone->Clone(m_pRecipeEditMgr->GetRecipeDataPtr());
}

bool CCommTcpIpObjectPG::SetTime_Sync()
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

bool CCommTcpIpObjectPG::RecipeChange()
{
	int nRet = VTM_RV_SUCCESSED;

	nRet = _ExePGCtrl(enumTCPIpFactoryt_PGEvent_2);

	if (nRet == VTM_RV_SUCCESSED)
		nRet = _ExePGCtrl(enumTCPIpFactoryt_PGEvent_0);

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
#endif

	
}

bool CCommTcpIpObjectPG::SetParameter()
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
			strLog.Format(_T("[%s] : %s, %s"), GetName(), _T("SetParameter, HEAD_ACK_TIMEOUT"), strReturnCommand);
			m_pLogFileAccessor->KMLOG(WARN, strLog);
			CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iSeqItemIndex);
			return false;
		}
		// jsyoon 2022.08.30 add
		else if (dw == (WAIT_OBJECT_0 + 1)) // NG
		{
			strLog.Format(_T("[%s] : %s, %d"), GetName(), _T("SetParameter, HEAD_ACK_FAIL"), strReturnCommand);
			m_pLogFileAccessor->KMLOG(WARN, strLog);
			CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_FAIL + m_iSeqItemIndex);
			return false;
		}
		else
		{
			;
		}

		return true;
	}

	strLog.Format(_T("[%s] : %s, %s"), GetName(), _T("HEAD_NOT_CONNECTED"), strReturnCommand);
	m_pLogFileAccessor->KMLOG(WARN, strLog);
	CAlarmController::GetInstance()->SetAlarm(HEAD_NOT_CONNECTED + m_iSeqItemIndex);

	return false;
}

bool CCommTcpIpObjectPG::InspectStart(int iMeasType, int iCellNum, bool bWait)
{
	int nRet = VTM_RV_SUCCESSED;

	nRet = _ExePGCtrl(iMeasType, iCellNum);
	
	if (nRet == VTM_RV_SUCCESSED)
		return true;
	else
		return false;
}

bool CCommTcpIpObjectPG::InspectStartWait()
{
	if (IsOutOfService() == true)
		return true;

	HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_INSP_START_OK], m_hRecvDEvent[enumRecvEventID_INSP_START_NG] };

	CString strLog;
	DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, DEFINE_INSPECITON_WAIT_TIME);
	if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
	{
		strLog.Format(_T("[%s] : %s"), GetName(), _T("InspectStartWait, HEAD_ACK_TIMEOUT"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iSeqItemIndex);
		return false;
	}
	// jsyoon 2022.01.17 add
	else if (dw == (WAIT_OBJECT_0 + 1)) // NG
	{
		strLog.Format(_T("[%s] : %s"), GetName(), _T("InspectStartWait, HEAD_ACK_FAIL"));
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

bool CCommTcpIpObjectPG::InspectEnd()
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

bool CCommTcpIpObjectPG::WaitInspectionEnd(DWORD dwTimeOut)
{
	if (IsOutOfService() == true)
		return true;

	if (IsConnected() == false)
		return false;

	CString strLog;

	HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_SET_PARAMETER_OK], m_hRecvDEvent[enumRecvEventID_SET_PARAMETER_NG] };

	DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, dwTimeOut * 1000);
	if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
	{
		strLog.Format(_T("[%s] : %s, %d"), GetName(), _T("WaitInspectionEnd, HEAD_ACK_TIMEOUT"), dwTimeOut);
		m_pLogFileAccessor->KMLOG(WARN, strLog);

		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iSeqItemIndex);

		return false;
	}
	// jsyoon 2022.08.30 add
	else if (dw == (WAIT_OBJECT_0 + 1)) // NG
	{
		strLog.Format(_T("[%s] : %s, %d"), GetName(), _T("WaitInspectionEnd, HEAD_ACK_FAIL"), dwTimeOut);
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_FAIL + m_iSeqItemIndex);
		return false;
	}
	else
	{
		;
	}

	return true;
}

bool CCommTcpIpObjectPG::WaitInspectionComplete(DWORD dwTimeOut, int iMeasType)
{
	CString strLog;

	S_RecipeMgr* pslData = nullptr;
	pslData = m_pRecipeEditMgr->GetRecipeMgrPtr();
	int nCellCnt = 0;
	nCellCnt = pslData->m_nCellCnt;

	int iStick = 0;
	iStick = m_pShareMemoryMgr->GetSharedMemory()->m_nAgingStartStick[4];

	// 2022.12.22 add
	if (iStick < 0)	return true;

	if (IsOutOfService() == true)
	{
		Sleep(100);

		if (iMeasType == 5)			// OST
		{
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
						else   // UI 표시는 홀수
						{
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_OST_NOR;	//	Normal
						}

						strLog.Format(_T("OST END"));
					}
					else
					{
						if (i % 2)  // UI 표시는 짝수
						{
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_OST_NOR;	//	Normal
						}

						strLog.Format(_T("OST EVEN END"));
					}
				}
				else
				{
					m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_OST_NOR;	//	Normal

					strLog.Format(_T("OST END"));
				}
			}
			
			//for (size_t i = 0; i < 10; i++)
			//{
			//	m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][0] = df_CELLSTATE_OST_NOR;	//	Normal
			//}

			//strLog.Format(_T("OST END"));
			m_pLogFileAccessor->KMLOG(OPER, strLog);
			
		}
		else if (iMeasType == 6)	// PTP
		{
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
						else   // UI 표시는 홀수
						{
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_PTP_NOR;	//	Normal
						}

						strLog.Format(_T("PTP END"));
					}
					else
					{
						if (i % 2)  // UI 표시는 짝수
						{
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_PTP_NOR;	//	Normal
						}
						
						strLog.Format(_T("PTP EVNE END"));
					}
				}
				else
				{
					m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_PTP_NOR;	//	Normal

					strLog.Format(_T("PTP END"));
				}
			}

			//for (size_t i = 0; i < 10; i++)
			//	m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][0] = df_CELLSTATE_PTP_NOR;	//	Normal

			//strLog.Format(_T("PTP END"));
			m_pLogFileAccessor->KMLOG(OPER, strLog);
		}
		else if (iMeasType == 7)	// RTEST
		{
			// LYS 2026-08-10 AutoContact Phase 3: OOS sim - raw all-pass status (21)
			{
				CAutoContactInfo* pAutoCnt = CAutoContactInfo::GetInstance();
				pAutoCnt->ClearCellStatus();
				for (int iCnt = 0; iCnt < (int)nCellCnt && iCnt < RTEST_MAX; iCnt++)
					pAutoCnt->SetCellStatus(iCnt, 21);
			}
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
						else   // UI 표시는 홀수
						{
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_PASS;	//	PASS
						}

						strLog.Format(_T("RTEST END"));
					}
					else
					{
						if (i % 2)  // UI 표시는 짝수
						{
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_PASS;	//	PASS
						}

						strLog.Format(_T("RTEST EVEN END"));
					}
				}
				else
				{
					m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_PASS;	//	PASS

					strLog.Format(_T("RTEST END"));
				}
			}

			//for (size_t i = 0; i < 10; i++)
			//	m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][0] = df_CELLSTATE_PASS;	//	PASS

			//strLog.Format(_T("RTEST END"));
			m_pLogFileAccessor->KMLOG(OPER, strLog);
		}
		else if (iMeasType == 8)	// AGING
		{
			int iStick = 0;
			iStick = m_pShareMemoryMgr->GetSharedMemory()->m_nAgingStartStick[4];

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
						else   // UI 표시는 홀수
						{
							if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS ||
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_START)
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_OK;
						}

						strLog.Format(_T("AGING END"));
					}
					else
					{
						if (i % 2)  // UI 표시는 짝수
						{
							if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS ||
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_START)
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_OK;;
						}

						strLog.Format(_T("AGING EVEN END"));
					}
				}
				else
				{
					if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
						m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS ||
						m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_START)
						m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_OK;

					strLog.Format(_T("AGING END"));
				}
			}

			m_pLogFileAccessor->KMLOG(OPER, strLog);
		}

		m_bAginging = false;

		return true;
	}
	

	if (IsConnected() == false)
		return false;

//	CString strLog;

	HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_OK], m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_NG] };

	DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, dwTimeOut * 1000);
	if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
	{
		strLog.Format(_T("[%s] : %s, %d"), GetName(), _T("WaitInspectionComplete, HEAD_ACK_TIMEOUT"), dwTimeOut);
		m_pLogFileAccessor->KMLOG(WARN, strLog);

		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iSeqItemIndex);
		return false;
	}
	// jsyoon 2022.08.30 add
	else if (dw == (WAIT_OBJECT_0 + 1)) // NG
	{
		strLog.Format(_T("[%s] : %s, %d"), GetName(), _T("WaitInspectionComplete, HEAD_ACK_FAIL"), dwTimeOut);
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_FAIL + m_iSeqItemIndex);
		return false;
	}
	else
	{
		;
	}

	//	OST / PTP, 결과 부분은 1 byte 의 크기를 가지며, Normal(0) / Short(1) / Open(2)
	//	RTest, 결과 부분은 1 byte 를 가지며 해당 Pass(1) / Over(0) / Under(2)
	int iResult = 0;
	TCHAR szResult;
	if (iMeasType == 5)			// OST
	{
		if (nCellCnt > 10)
		{
			if (m_bAgingingEven != true)
			{
				int j = 0;
				for (size_t i = 0; i < nCellCnt; i++)
				{
					if (i % 2)
					{
						continue;
					}
					else   // UI 표시는 홀수
					{
						iResult = _ttoi(m_strPGResult.Mid(j, 1));

						switch (iResult)
						{
						case 0:
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_OST_NOR;	//	Normal
							break;
						case 1:
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_OST_SHORT;
							break;
						case 2:
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_OST_OPEN;
							break;
						default:
							break;
						}

						j++;
					}

				}


				strLog.Format(_T("OST END"));
			}
			else
			{
				int j = 0;
				for (size_t i = 0; i < nCellCnt; i++)
				{
					if (i % 2)  // UI 표시는 짝수
					{
						iResult = _ttoi(m_strPGResult.Mid(j, 1));

						switch (iResult)
						{
						case 0:
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_OST_NOR;	//	Normal
							break;
						case 1:
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_OST_SHORT;
							break;
						case 2:
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_OST_OPEN;
							break;
						default:
							break;
						}

						j++;
					}
				}

				strLog.Format(_T("OST EVNE END"));
			}
		}
		else
		{
			for (size_t i = 0; i < m_strPGResult.GetLength(); i++)
			{
				iResult = _ttoi(m_strPGResult.Mid(i, 1));

				switch (iResult)
				{
				case 0:
					m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_OST_NOR;	//	Normal
					break;
				case 1:
					m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_OST_SHORT;
					break;
				case 2:
					m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_OST_OPEN;
					break;
				default:
					break;
				}
			}

			strLog.Format(_T("OST END"));
		}

		//for (size_t i = 0; i < m_strPGResult.GetLength(); i++)
		//{
		//	iResult = _ttoi(m_strPGResult.Mid(i, 1));
		//	
		//	switch (iResult)
		//	{
		//	case 0:
		//		m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][0] = df_CELLSTATE_OST_NOR;	//	Normal
		//		break;
		//	case 1:
		//		m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][0] = df_CELLSTATE_OST_SHORT;
		//		break;
		//	case 2:
		//		m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][0] = df_CELLSTATE_OST_OPEN;
		//		break;
		//	default:
		//		break;
		//	}
		//}

		//strLog.Format(_T("OST END"));
		m_pLogFileAccessor->KMLOG(OPER, strLog);
	}
	else if (iMeasType == 6)	// PTP
	{
		if (nCellCnt > 10)
		{
			if (m_bAgingingEven != true)
			{
				int j = 0;
				for (size_t i = 0; i < nCellCnt; i++)
				{
					if (i % 2)
					{
						continue;
					}
					else   // UI 표시는 홀수
					{
						iResult = _ttoi(m_strPGResult.Mid(j, 1));

						switch (iResult)
						{
						case 0:
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_PTP_NOR;	//	Normal
							break;
						case 1:
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_PTP_SHORT;
							break;
						case 2:
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_PTP_OPEN;
							break;
						default:
							break;
						}

						j++;
					}
						
				}
					

				strLog.Format(_T("PTP END"));
			}
			else
			{
				int j = 0;
				for (size_t i = 0; i < nCellCnt; i++)
				{
					if (i % 2)  // UI 표시는 짝수
					{
						iResult = _ttoi(m_strPGResult.Mid(j, 1));

						switch (iResult)
						{
						case 0:
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_PTP_NOR;	//	Normal
							break;
						case 1:
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_PTP_SHORT;
							break;
						case 2:
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_PTP_OPEN;
							break;
						default:
							break;
						}

						j++;
					}
				}

				strLog.Format(_T("PTP EVNE END"));
			}
		}
		else
		{
			for (size_t i = 0; i < m_strPGResult.GetLength(); i++)
			{
				iResult = _ttoi(m_strPGResult.Mid(i, 1));

				switch (iResult)
				{
				case 0:
					m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_PTP_NOR;	//	Normal
					break;
				case 1:
					m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_PTP_SHORT;
					break;
				case 2:
					m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_PTP_OPEN;
					break;
				default:
					break;
				}
			}
				
			strLog.Format(_T("PTP END"));
		}


		//for (size_t i = 0; i < m_strPGResult.GetLength(); i++)
		//{
		//	iResult = _ttoi(m_strPGResult.Mid(i, 1));

		//	switch (iResult)
		//	{
		//	case 0:
		//		m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][0] = df_CELLSTATE_PTP_NOR;	//	Normal
		//		break;
		//	case 1:
		//		m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][0] = df_CELLSTATE_PTP_SHORT;
		//		break;
		//	case 2:
		//		m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][0] = df_CELLSTATE_PTP_OPEN;
		//		break;
		//	default:
		//		break;
		//	}
		//}

	//	strLog.Format(_T("PTP END"));
		m_pLogFileAccessor->KMLOG(OPER, strLog);
	}
	else if (iMeasType == 7)	// RTEST
	{
		// LYS 2026-08-10 AutoContact Phase 3: 셀별 raw 값 + 20 유지
		// (OCP CalcPGResult(2,-1) 규약: 21=PASS, 28=포트 실패)
		{
			CAutoContactInfo* pAutoCnt = CAutoContactInfo::GetInstance();
			pAutoCnt->ClearCellStatus();
			for (int iCnt = 0; iCnt < m_strPGResult.GetLength() && iCnt < RTEST_MAX; iCnt++)
				pAutoCnt->SetCellStatus(iCnt, _ttoi(m_strPGResult.Mid(iCnt, 1)) + 20);
		}
		if (nCellCnt > 10)
		{
			if (m_bAgingingEven != true)
			{
				int j = 0;
				for (size_t i = 0; i < nCellCnt; i++)
				{
					if (i % 2)
					{
						continue;
					}
					else   // UI 표시는 홀수
					{
						iResult = _ttoi(m_strPGResult.Mid(j, 1));

						switch (iResult)
						{
						case 0:
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_OVER;
							break;
						case 1:
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_PASS;	// PASS
							break;
						case 2:
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_UNDER;
							break;
						default:
							break;
						}

						j++;
					}

				}


				strLog.Format(_T("RTEST END"));
			}
			else
			{
				int j = 0;
				for (size_t i = 0; i < nCellCnt; i++)
				{
					if (i % 2)  // UI 표시는 짝수
					{
						iResult = _ttoi(m_strPGResult.Mid(j, 1));

						switch (iResult)
						{
						case 0:
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_OVER;
							break;
						case 1:
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_PASS;	// PASS
							break;
						case 2:
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_UNDER;
							break;
						default:
							break;
						}

						j++;
					}
				}

				strLog.Format(_T("RTEST EVNE END"));
			}
		}
		else
		{
			for (size_t i = 0; i < m_strPGResult.GetLength(); i++)
			{
				iResult = _ttoi(m_strPGResult.Mid(i, 1));

				switch (iResult)
				{
				case 0:
					m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_OVER;
					break;
				case 1:
					m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_PASS;	// PASS
					break;
				case 2:
					m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_UNDER;
					break;
				default:
					break;
				}
			}

			strLog.Format(_T("RTEST END"));
		}

		//for (size_t i = 0; i < m_strPGResult.GetLength(); i++)
		//{
		//	iResult = _ttoi(m_strPGResult.Mid(i, 1));

		//	switch (iResult)
		//	{
		//	case 0:
		//		m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][0] = df_CELLSTATE_OVER;
		//		break;
		//	case 1:
		//		m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][0] = df_CELLSTATE_PASS;	// PASS
		//		break;
		//	case 2:
		//		m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][0] = df_CELLSTATE_UNDER;
		//		break;
		//	default:
		//		break;
		//	}
		//}

		//strLog.Format(_T("RTEST END"));
		m_pLogFileAccessor->KMLOG(OPER, strLog);
	}
	else if (iMeasType == 8)	// AGING
	{
		int iStick = 0;
		iStick = m_pShareMemoryMgr->GetSharedMemory()->m_nAgingStartStick[4];

		if (nCellCnt > 10)
		{
			if (m_bAgingingEven != true)
			{
				int j = 0;
				for (size_t i = 0; i < nCellCnt; i++)
				{
					if (i % 2)  // UI 표시는 짝수
					{
						continue;
					}
					else
					{
						iResult = _ttoi(m_strPGResult.Mid(j, 1));

						switch (iResult)
						{
						case 0:	// NG
						{
							if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS ||
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_START)
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_NG;
						}
						break;
						case 1: // OK
						{
							if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS ||
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_START)
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_OK;
						}
						break;
						default:
							break;
						}

						j++;
					}
				}

				strLog.Format(_T("AGING END"));
			}
			else
			{
				int j = 0;
				for (size_t i = 0; i < nCellCnt; i++)
				{
					if (i % 2) // UI 표시는 홀수
					{
						iResult = _ttoi(m_strPGResult.Mid(j, 1));

						switch (iResult)
						{
						case 0:	// NG
						{
							if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS ||
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_START)
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_NG;
						}
						break;
						case 1: // OK
						{
							if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS ||
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_START)
								m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_OK;
						}
						break;
						default:
							break;
						}

						j++;
					}
				}

				strLog.Format(_T("AGING EVEN END"));
			}
		}
		else
		{
			for (size_t i = 0; i < m_strPGResult.GetLength(); i++)
			{
				iResult = _ttoi(m_strPGResult.Mid(i, 1));

				switch (iResult)
				{
				case 0:	// NG
				{
					if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
						m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS ||
						m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_START)
						m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_NG;
				}
				break;
				case 1: // OK
				{
					if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
						m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS ||
						m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_START)
						m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_OK;
				}
				break;
				default:
					break;
				}
			}
			
			strLog.Format(_T("AGING END"));
		}

//		strLog.Format(_T("AGING END"));
		m_pLogFileAccessor->KMLOG(OPER, strLog);

		m_bAginging = false;
	}
	else
	{
		;
	}

	return true;
}

// Reciep 응답 완료 확인
int CCommTcpIpObjectPG::WaitCommandRecive(DWORD dwTimeOut)
{
	if (IsOutOfService() == true)
		return VTM_RV_SUCCESSED;

	if (IsConnected() == false)
		return HEAD_NOT_CONNECTED;

	CString strLog;

	HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_RECIPE_CHANGE_OK], m_hRecvDEvent[enumRecvEventID_RECIPE_CHANGE_NG] };

	DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, dwTimeOut * 1000);
	if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
	{
		strLog.Format(_T("[%s] : %s, %d"), GetName(), _T("WaitCommandRecive, HEAD_ACK_TIMEOUT"), dwTimeOut);
		m_pLogFileAccessor->KMLOG(WARN, strLog);

		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iSeqItemIndex);

		return HEAD_ACK_TIMEOUT;
	}
	// jsyoon 2022.08.30 add
	else if (dw == (WAIT_OBJECT_0 + 1)) // NG
	{
		strLog.Format(_T("[%s] : %s, %d"), GetName(), _T("WaitCommandRecive, HEAD_ACK_FAIL"), dwTimeOut);
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_FAIL + m_iSeqItemIndex);
		return HEAD_ACK_FAIL;
	}
	else
	{
		return VTM_RV_SUCCESSED;
	}

	return VTM_RV_SUCCESSED;
}



//=========================================================================
// INTERFACE 관련
//=========================================================================
void CCommTcpIpObjectPG::xNetCallbackRecieveData(LPVOID ud)
{
	//if (CSystemParamShare::GetInstance()->IsAutoMode() == false)
	//	return;

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
		}
		else if (pRcvData->eCommState == (eXNET_COMM_STATE)XNET_RECEIVEDATA)
		{
	//		CString strMessage(pRcvData->szBody);

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

			CString strLog;
			strLog.Format(_T("[%s] : %s"), GetName(), strMessage);
			//		CLogWrapper::GetInstance()->KMLOG(SYST, strLog);
			m_pLogFileAccessor->KMLOG(RECV, strLog);

			strMessage.Replace(_T(" "), _T(""));
			if (strMessage.IsEmpty() == true)
				return;

			BOOL bAck = TRUE;
		//	CString strPGCmd, strTmp;
		//	strPGCmd = ExeHextoStr(strMessage);

			//---------------------------------------------------------------------------
			// 패킷 단위로 응답에 대한 처리를 해야 한다.
			// socket 에 데이터가 진입하는데 2개 이상 붙어서 1개의 event 로 들어온다.
			// 모든 패킷에 대해서 command 단위로 분리해서 수행해야 한다.
			//---------------------------------------------------------------------------
			std::vector<CString> vecPacketCommandPool = GetExeHextoStr(strMessage); //ExeHextoStr(strMessage);

			for (size_t i = 0; i < vecPacketCommandPool.size(); ++i)
			{
				bAck = TRUE;
				const CString strPGCmd = vecPacketCommandPool[i];

				PGPROTOCOL stRcvPg;
				ZeroMemory(&stRcvPg, sizeof(stRcvPg));

				if (strPGCmd.GetLength() < 1)
				{
					SetEvent(m_hRecvDEvent[enumRecvEventID_SET_PARAMETER_NG]);
					return;
				}

				int nEventOK = enumRecvEventID_SET_PARAMETER_OK, nEventNG = enumRecvEventID_SET_PARAMETER_NG;

				if (strPGCmd.Find(eS1F2) == 0)			// Test Program ACK
				{
					stRcvPg.iStream = _ttoi(strPGCmd.Left(2));
					stRcvPg.iFunc = _ttoi(strPGCmd.Mid(2, 2));
					stRcvPg.iAck = _ttoi(strPGCmd.Mid(4, 2));	// 0-ACX / 1-NAK
					stRcvPg.iError = _ttoi(strPGCmd.Mid(6, 2));
				}
				else if (strPGCmd.Find(eS1F6) == 0)		// PG Select ACK
				{
					stRcvPg.iStream = _ttoi(strPGCmd.Left(2));
					stRcvPg.iFunc = _ttoi(strPGCmd.Mid(2, 2));
					stRcvPg.iAck = _ttoi(strPGCmd.Mid(4, 2));	// 0-ACX / 1-NAK
					stRcvPg.iError = _ttoi(strPGCmd.Mid(6, 2));
				}
				else if (strPGCmd.Find(eS3F2) == 0)		// OST Test ACK
				{
					bAck = FALSE;
					stRcvPg.iStream = _ttoi(strPGCmd.Left(2));
					stRcvPg.iFunc = _ttoi(strPGCmd.Mid(2, 2));
					stRcvPg.iAck = _ttoi(strPGCmd.Mid(4, 2));
					stRcvPg.iError = _ttoi(strPGCmd.Mid(6, 2));

					nEventOK = enumRecvEventID_INSP_START_OK;
					nEventNG = enumRecvEventID_INSP_START_NG;
				}
				else if (strPGCmd.Find(eS3F3) == 0)
				{
					stRcvPg.iStream = _ttoi(strPGCmd.Left(2));
					stRcvPg.iFunc = _ttoi(strPGCmd.Mid(2, 2));
					stRcvPg.iAck = _ttoi(strPGCmd.Mid(4, 2));
					stRcvPg.iError = _ttoi(strPGCmd.Mid(6, 2));
					stRcvPg.iResult = _ttoi(strPGCmd.Mid(8, 1));	// OST Result

					nEventOK = enumRecvEventID_INSP_END_OK;
					nEventNG = enumRecvEventID_INSP_END_NG;
				}
				else if (strPGCmd.Find(eS3F6) == 0)		// PTP Test ACK
				{
					bAck = FALSE;
					stRcvPg.iStream = _ttoi(strPGCmd.Left(2));
					stRcvPg.iFunc = _ttoi(strPGCmd.Mid(2, 2));
					stRcvPg.iAck = _ttoi(strPGCmd.Mid(4, 2));
					stRcvPg.iError = _ttoi(strPGCmd.Mid(6, 2));

					nEventOK = enumRecvEventID_INSP_START_OK;
					nEventNG = enumRecvEventID_INSP_START_NG;
				}
				else if (strPGCmd.Find(eS3F7) == 0)
				{
					stRcvPg.iStream = _ttoi(strPGCmd.Left(2));
					stRcvPg.iFunc = _ttoi(strPGCmd.Mid(2, 2));
					stRcvPg.iAck = _ttoi(strPGCmd.Mid(4, 2));
					stRcvPg.iError = _ttoi(strPGCmd.Mid(6, 2));
					stRcvPg.iResult = _ttoi(strPGCmd.Mid(8, 1));	// PTP Result

					nEventOK = enumRecvEventID_INSP_END_OK;
					nEventNG = enumRecvEventID_INSP_END_NG;
				}
				else if (strPGCmd.Find(eS3F12) == 0)		// PG Pattern Turn On ACK
				{
					stRcvPg.iStream = _ttoi(strPGCmd.Left(2));
					stRcvPg.iFunc = _ttoi(strPGCmd.Mid(2, 2));
					stRcvPg.iAck = _ttoi(strPGCmd.Mid(4, 2));
					stRcvPg.iError = _ttoi(strPGCmd.Mid(6, 2));

					nEventOK = enumRecvEventID_SET_PARAMETER_OK;
					nEventNG = enumRecvEventID_SET_PARAMETER_NG;
				}
				else if (strPGCmd.Find(eS3F14) == 0)		// PG Pattern Next ACK
				{
					stRcvPg.iStream = _ttoi(strPGCmd.Left(2));
					stRcvPg.iFunc = _ttoi(strPGCmd.Mid(2, 2));
					stRcvPg.iAck = _ttoi(strPGCmd.Mid(4, 2));
					stRcvPg.iError = _ttoi(strPGCmd.Mid(6, 2));
				}
				else if (strPGCmd.Find(eS3F16) == 0)		// PG Pattern Previous ACK
				{
					stRcvPg.iStream = _ttoi(strPGCmd.Left(2));
					stRcvPg.iFunc = _ttoi(strPGCmd.Mid(2, 2));
					stRcvPg.iAck = _ttoi(strPGCmd.Mid(4, 2));
					stRcvPg.iError = _ttoi(strPGCmd.Mid(6, 2));
				}
				else if (strPGCmd.Find(eS3F18) == 0)		// PG Pattern Turn Off
				{
					stRcvPg.iStream = _ttoi(strPGCmd.Left(2));
					stRcvPg.iFunc = _ttoi(strPGCmd.Mid(2, 2));
					stRcvPg.iAck = _ttoi(strPGCmd.Mid(4, 2));
					stRcvPg.iError = _ttoi(strPGCmd.Mid(6, 2));
				}
				else if (strPGCmd.Find(eS3F20) == 0)		// Current
				{
					stRcvPg.iStream = _ttoi(strPGCmd.Left(2));
					stRcvPg.iFunc = _ttoi(strPGCmd.Mid(2, 2));
					stRcvPg.iAck = _ttoi(strPGCmd.Mid(4, 2));
					stRcvPg.iError = _ttoi(strPGCmd.Mid(6, 2));
					stRcvPg.dResult = _ttof(strPGCmd.Mid(8, 10));	// Current Value 0x20 
				}
				else if (strPGCmd.Find(eS5F2) == 0)		// Status check
				{
					stRcvPg.iStream = _ttoi(strPGCmd.Left(2));
					stRcvPg.iFunc = _ttoi(strPGCmd.Mid(2, 2));
					stRcvPg.iAck = _ttoi(strPGCmd.Mid(4, 2));
					stRcvPg.iError = _ttoi(strPGCmd.Mid(6, 2));
					stRcvPg.iResult = _ttoi(strPGCmd.Mid(8, 1));	// Run State

					if (stRcvPg.iResult != 0)  // 1:Running
					{
						nEventOK = enumRecvEventID_SET_PARAMETER_NG;
						nEventNG = enumRecvEventID_SET_PARAMETER_NG;
					}
				}
				else if (strPGCmd.Find(eS2F2) == 0)		// Model Set ACK
				{
					bAck = FALSE;
					stRcvPg.iStream = _ttoi(strPGCmd.Left(2));
					stRcvPg.iFunc = _ttoi(strPGCmd.Mid(2, 2));
					stRcvPg.iAck = _ttoi(strPGCmd.Mid(4, 2));
					stRcvPg.iError = _ttoi(strPGCmd.Mid(6, 2));

					nEventOK = enumRecvEventID_RECIPE_CHANGE_OK;
					nEventNG = enumRecvEventID_RECIPE_CHANGE_NG;
				}
				else if (strPGCmd.Find(eS2F3) == 0)		// Model ACK
				{
					stRcvPg.iStream = _ttoi(strPGCmd.Left(2));
					stRcvPg.iFunc = _ttoi(strPGCmd.Mid(2, 2));
					stRcvPg.iAck = _ttoi(strPGCmd.Mid(4, 2));
					stRcvPg.iError = _ttoi(strPGCmd.Mid(6, 2));

					nEventOK = enumRecvEventID_RECIPE_CHANGE_OK;
					nEventNG = enumRecvEventID_RECIPE_CHANGE_NG;
				}
				else if (strPGCmd.Find(eS2F10) == 0)		// Recipe ACK
				{
					stRcvPg.iStream = _ttoi(strPGCmd.Left(2));
					stRcvPg.iFunc = _ttoi(strPGCmd.Mid(2, 2));
					stRcvPg.iAck = _ttoi(strPGCmd.Mid(4, 2));
					stRcvPg.iError = _ttoi(strPGCmd.Mid(6, 2));

					nEventOK = enumRecvEventID_RECIPE_CHANGE_OK;
					nEventNG = enumRecvEventID_RECIPE_CHANGE_NG;
				}
				else if (strPGCmd.Find(eS2F12) == 0)		// Aging 시나리오 ACK
				{
					stRcvPg.iStream = _ttoi(strPGCmd.Left(2));
					stRcvPg.iFunc = _ttoi(strPGCmd.Mid(2, 2));
					stRcvPg.iAck = _ttoi(strPGCmd.Mid(4, 2));
					stRcvPg.iError = _ttoi(strPGCmd.Mid(6, 2));

					nEventOK = enumRecvEventID_RECIPE_CHANGE_OK;
					nEventNG = enumRecvEventID_RECIPE_CHANGE_NG;
				}
				else if (strPGCmd.Find(eS6F2) == 0)		// Aging Start ACK
				{
					bAck = TRUE;
					stRcvPg.iStream = _ttoi(strPGCmd.Left(2));
					stRcvPg.iFunc = _ttoi(strPGCmd.Mid(2, 2));
					stRcvPg.iAck = _ttoi(strPGCmd.Mid(4, 2));
					stRcvPg.iError = _ttoi(strPGCmd.Mid(6, 2));
					m_nAgingTime = _ttoi(strPGCmd.Mid(8, 10));		// Aging Time(Sec) 

					nEventOK = enumRecvEventID_INSP_START_OK;
					nEventNG = enumRecvEventID_INSP_START_NG;

					if (stRcvPg.iAck == 0)
					{
						S_RecipeMgr* pslData = nullptr;
						pslData = m_pRecipeEditMgr->GetRecipeMgrPtr();
						int nStickCnt = 0, nCellCnt = 0;
						nCellCnt = pslData->m_nCellCnt;

						int iStick = 0;
						iStick = m_pShareMemoryMgr->GetSharedMemory()->m_nAgingStartStick[4];

						if (nCellCnt > 10)
						{
							int j = 0;
							for (size_t i = 0; i < nCellCnt; i++)
							{
								if (m_bAgingingEven != true )
								{
									if (i % 2)
									{
										continue;
									}
									else   // UI 표시는 홀수
									{
										if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
											m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS ||
											m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_NG || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_OK)
											m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_START;
									}
								}
								else
								{
									if (i % 2)  // UI 표시는 짝수
									{

										if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
											m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS ||
											m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_NG || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_OK)
											m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_START;
									}
								}
							}
						}
						else
						{
							for (size_t i = 0; i < nCellCnt; i++)
							{
								if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
									m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS ||
									m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_NG || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_OK)
									m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_START;
							}
						}

						//for (size_t i = 0; i < nCellCnt; i++)
						//{
						//	if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE    || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
						//		m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS)
						//		m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_START; 
						//} 
					}
				}
				else if (strPGCmd.Find(eS6F3) == 0)		// Aging End ACK
				{
					stRcvPg.iStream = _ttoi(strPGCmd.Left(2));
					stRcvPg.iFunc = _ttoi(strPGCmd.Mid(2, 2));
					stRcvPg.iAck = _ttoi(strPGCmd.Mid(4, 2));
					stRcvPg.iError = _ttoi(strPGCmd.Mid(6, 2));
					// Aging result
					CString strResult;
					strResult = strPGCmd.Mid(8, strPGCmd.GetLength() - 8);	// 1-OK, 0-NG
					m_strPGResult.Empty();
					m_strPGResult = strResult;

					nEventOK = enumRecvEventID_INSP_COMPLETE_OK;
					nEventNG = enumRecvEventID_INSP_COMPLETE_NG;
				}
				else if (strPGCmd.Find(eS8F2) == 0)		// Cell Command ACK
				{
					//	bAck = FALSE;
					stRcvPg.iStream = _ttoi(strPGCmd.Left(2));
					stRcvPg.iFunc = _ttoi(strPGCmd.Mid(2, 2));
					stRcvPg.iAck = _ttoi(strPGCmd.Mid(4, 2));
					stRcvPg.iError = _ttoi(strPGCmd.Mid(6, 2));

					nEventOK = enumRecvEventID_INSP_START_OK;
					nEventNG = enumRecvEventID_INSP_START_NG;
				}
				else if (strPGCmd.Find(eS8F3) == 0)		// Cell Command End ACK
				{
					stRcvPg.iStream = _ttoi(strPGCmd.Left(2));
					stRcvPg.iFunc = _ttoi(strPGCmd.Mid(2, 2));
					stRcvPg.iAck = _ttoi(strPGCmd.Mid(4, 2));
					stRcvPg.iError = _ttoi(strPGCmd.Mid(6, 2));
					stRcvPg.iCmd = _ttoi(strPGCmd.Mid(8, 2));			// S8F1
					stRcvPg.iCellNum = _ttoi(strPGCmd.Mid(10, 2));		// S8F1

					if (stRcvPg.iCmd == 5 || stRcvPg.iCmd == 6 || stRcvPg.iCmd == 7)	// OST, PTP, RTest
					{
						if (stRcvPg.iCellNum != 0)
						{
							stRcvPg.iResult = _ttoi(strPGCmd.Mid(12, 1));		// 1Byte
						}
						else
						{
							CString strResult;
							strResult = strPGCmd.Mid(12, strPGCmd.GetLength() - 12);	// 1-OK, 0-NG	
							m_strPGResult.Empty();
							m_strPGResult = strResult;

						//	OST / PTP, 결과 부분은 1 byte 의 크기를 가지며, Normal(0) / Short(1) / Open(2)

						//	RTest, 결과 부분은 1 byte 를 가지며 해당 Pass(1) / Over(0) / Under(2)
						}
					}

					nEventOK = enumRecvEventID_INSP_COMPLETE_OK;
					nEventNG = enumRecvEventID_INSP_COMPLETE_NG;
				}
				else if (strPGCmd.Find(eS10F2) == 0)		// Test Stop ACK
				{
					stRcvPg.iStream = _ttoi(strPGCmd.Left(2));
					stRcvPg.iFunc = _ttoi(strPGCmd.Mid(2, 2));
					stRcvPg.iAck = _ttoi(strPGCmd.Mid(4, 2));
					stRcvPg.iError = _ttoi(strPGCmd.Mid(6, 2));

					nEventOK = enumRecvEventID_INSP_COMPLETE_NG;
					nEventNG = enumRecvEventID_INSP_COMPLETE_NG; 
				}
				else if (strPGCmd.Find(eS10F4) == 0)		// Turn Off ACK
				{
					stRcvPg.iStream = _ttoi(strPGCmd.Left(2));
					stRcvPg.iFunc = _ttoi(strPGCmd.Mid(2, 2));
					stRcvPg.iAck = _ttoi(strPGCmd.Mid(4, 2));
					stRcvPg.iError = _ttoi(strPGCmd.Mid(6, 2));
				}

				if (stRcvPg.iAck != 0)
				{
					SetEvent(m_hRecvDEvent[nEventNG]);
					//		::PostMessage(this->GetParent()->m_hWnd, WM_MSG_MAINT_ALARM, (WPARAM)VTM_PROBE1, (LPARAM)PG_MODULE_NO_ERROR + stRcvPg.iError);
				}
				else
				{
					if (bAck == TRUE)	SetEvent(m_hRecvDEvent[nEventOK]);
				}

				strLog.Format(_T("S(%02d), F(%02d), ACK(%02d), Error Code(%02d), ACK(%d)"), stRcvPg.iStream, stRcvPg.iFunc, stRcvPg.iAck, stRcvPg.iError, bAck);
				m_pLogFileAccessor->KMLOG(OPER, strLog);
			}
		}
	}
}

CLogFileAccessor* CCommTcpIpObjectPG::GetLogFileAccessorPtr()
{
	return m_pLogFileAccessor;
}

int CCommTcpIpObjectPG::SetCommand(PGPROTOCOL stPg, bool bWait)
{
//	int iRet = VTM_RV_SUCCESSED;
	int iLen = 0;
	TBYTE nByte = 0x30, nCmd[2], nStx = 0x02, nEtx = 0x03;
	TCHAR	szTmp[1000];
	TCHAR	szCmd[1000];
	szTmp[0] = 0x00;
	szCmd[0] = 0x00;
	nCmd[0] = 0x30;

	if (stPg.iStream == 0)
		return PG_MODULE_PARAM_ERROR;

	_stprintf_s(szCmd, _countof(szCmd), _T("%c"), nStx);		// STX

	nCmd[0] = stPg.iStream / 10 + nByte;
	nCmd[1] = stPg.iStream % 10 + nByte;
	_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nCmd[0]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
	_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nCmd[1]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);

	nCmd[0] = stPg.iFunc / 10 + nByte;
	nCmd[1] = stPg.iFunc % 10 + nByte;
	_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nCmd[0]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
	_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nCmd[1]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);

	int nEventOK = enumRecvEventID_SET_PARAMETER_OK, nEventNG = enumRecvEventID_SET_PARAMETER_NG;

	if (stPg.iStream == 10) // Turn OFF or TEST STOP
	{
		;
	}
	else
	{
		if (stPg.iStream == 2)	// Recipe
		{
			if (stPg.iFunc == 1)	// Model
			{
				//				nCmd[0] = stPg.iCmd / 10 + nByte;
				nCmd[0] = stPg.iCmd + nByte;
				nCmd[1] = 0x00;
				_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nCmd[0]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);

				iLen = _tcsclen(stPg.szModel);
				for (int i = 0; i < iLen; i++)
				{
					_stprintf_s(szTmp, _countof(szTmp), _T("%c"), stPg.szModel[i]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
				}
			}
			else if (stPg.iFunc == 9 || stPg.iFunc == 11)	// Recipe , Aging
			{
				iLen = _tcsclen(stPg.szRecipe);
				for (int i = 0; i < iLen; i++)
				{
					_stprintf_s(szTmp, _countof(szTmp), _T("%c"), stPg.szRecipe[i]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
				}
			}

			nEventOK = enumRecvEventID_RECIPE_CHANGE_OK;
			nEventNG = enumRecvEventID_RECIPE_CHANGE_NG;
		}
		else if (stPg.iStream == 6)	// AGING
		{
			nCmd[0] = stPg.iCmd + nByte;
			_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nCmd[0]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);

			// Ensis Offset add
			if (m_bUseEnsisPG == true)
			{
				nCmd[1] = stPg.iOption + nByte;
				_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nCmd[1]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
			}

			/*!
			* 2023.06.09 add 
			* bettercell add CellID 
			*/ 
			if (m_bUseEnsisPG == true)
			{
				// GlassID
				TCHAR szGlassID[256];
				ZeroMemory(&szGlassID, sizeof(szGlassID));
				CString strGlassID;
				strGlassID.Format(_T("%s"), CSystemParamShare::GetInstance()->GetGlassID());

				// Stick Nnm(2)
				int iStick = 0, nMaxSitck = 0;
				iStick = m_pShareMemoryMgr->GetSharedMemory()->m_nAgingStartStick[4] + 1;

				// Glass 방향(1)
				int iRotate = 1;	 // A01, A02
				S_RecipeInfo* pReciepInfo;
				pReciepInfo = m_pRecipeEditMgr->GetRecipeInfo();
				if (pReciepInfo != nullptr)
				{
					if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
					{
						iRotate = 0;  // A01, B01
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

				_tcscpy(szGlassID, strCellID);
				iLen = _tcsclen(szGlassID);
				for (int i = 0; i < 30; i++)
				{
					if (i < iLen)
					{
						_stprintf_s(szTmp, _countof(szTmp), _T("%c"), szGlassID[i]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
					}
					else
					{
						_stprintf_s(szTmp, _countof(szTmp), _T("%s"), _T(" "));			_tcscat_s(szCmd, _countof(szCmd), szTmp);
					}
				}

				nCmd[0] = (iStick + nMaxSitck) / 10 + nByte;
				nCmd[1] = (iStick + nMaxSitck) % 10 + nByte;
				_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nCmd[0]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
				_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nCmd[1]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);

				nCmd[0] = iRotate + nByte;
				_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nCmd[0]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
			}


			nEventOK = enumRecvEventID_INSP_START_OK;
			nEventNG = enumRecvEventID_INSP_START_NG;

			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_END_OK]);
			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_END_NG]);

			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_OK]);
			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_NG]);
		}
		else if (stPg.iStream == 8)		// Cell Command
		{
			// Commnad
			nCmd[0] = stPg.iCmd / 10 + nByte;
			nCmd[1] = stPg.iCmd % 10 + nByte;
			_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nCmd[0]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
			_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nCmd[1]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);

			// Cell Num
			nCmd[0] = stPg.iCellNum / 10 + nByte;
			nCmd[1] = stPg.iCellNum % 10 + nByte;
			_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nCmd[0]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
			_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nCmd[1]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);

			// Ensis Offset add
			if (stPg.iCmd == 5 || stPg.iCmd == 6 || stPg.iCmd == 7)	// OST, PTP, RTEST
			{
				if (m_bUseEnsisPG == true)
				{
					nCmd[0] = stPg.iOption + nByte;
					_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nCmd[0]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
				}
			}
			

			nEventOK = enumRecvEventID_INSP_START_OK;
			nEventNG = enumRecvEventID_INSP_START_NG;

			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_OK]);
			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_NG]);

			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_END_OK]);
			ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_END_NG]);
		}
		else
		{
			if (stPg.iFunc == 5)	// Cell 선택
			{
				nCmd[0] = stPg.iCmd + nByte;
				_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nCmd[0]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);

				nCmd[0] = stPg.iCellNum / 10 + nByte;
				nCmd[1] = stPg.iCellNum % 10 + nByte;
				_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nCmd[0]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
				_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nCmd[1]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);

				nEventOK = enumRecvEventID_SET_PARAMETER_OK; // enumRecvEventID_RECIPE_CHANGE_OK;
				nEventNG = enumRecvEventID_SET_PARAMETER_NG; //  enumRecvEventID_RECIPE_CHANGE_NG;
			}
			else
			{
				nCmd[0] = stPg.iCmd / 10 + nByte;
				nCmd[1] = stPg.iCmd % 10 + nByte;
				_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nCmd[0]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
				_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nCmd[1]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
			}
		}
	}

	ResetEvent(m_hRecvDEvent[nEventOK]);
	ResetEvent(m_hRecvDEvent[nEventNG]);

	_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nEtx); _tcscat_s(szCmd, _countof(szCmd), szTmp);	// EXT

//	iLen = strlen(szCmd);
	iLen = _tcsclen(szCmd);
	szCmd[iLen] = 0;

	CString strReturnCommand;
	strReturnCommand.Format(_T("%s"), szCmd);

	CString strLog;
	strLog.Format(_T("[%s] : %s"), GetName(), strReturnCommand);
	m_pLogFileAccessor->KMLOG(SEND, strLog);

	if (IsOutOfService() == true)
		return VTM_RV_SUCCESSED;

	if (SendPacket(strReturnCommand) == true)
	{
		if (bWait == true)
		{
			HANDLE hRecvEvent[2] = { m_hRecvDEvent[nEventOK], m_hRecvDEvent[nEventNG] };

			DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, DEFINE_RESPONSE_WAIT_TIME * 10);
			if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
			{
				strLog.Format(_T("[%s] : %s"), GetName(), _T("SetCommand, HEAD_ACK_TIMEOUT"));
				m_pLogFileAccessor->KMLOG(WARN, strLog);
				CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iSeqItemIndex);
				return HEAD_ACK_TIMEOUT;
			}
			// jsyoon 2022.01.17 add
			else if (dw == (WAIT_OBJECT_0 + 1)) // NG
			{
				strLog.Format(_T("[%s] : %s"), GetName(), _T("SetCommand, HEAD_ACK_FAIL"));
				m_pLogFileAccessor->KMLOG(WARN, strLog);
				CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_FAIL + m_iSeqItemIndex);
				return HEAD_ACK_FAIL;
			}
			else
			{
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

int	 CCommTcpIpObjectPG::_ExePGCtrl(int iEvent, int iCellNum)
{
	CString str;
	CString strLog;
	int nRtn = VTM_RV_SUCCESSED;

	PGPROTOCOL stPg;
	ZeroMemory(&stPg, sizeof(stPg));

	UINT nTimeOut = DEFINE_RESPONSE_WAIT_TIME;	// Sec

	// PtoP, OST, R-Test, Aging Check
	if (iEvent == enumTCPIpFactoryt_PG_TEST_1 || iEvent == enumTCPIpFactoryt_PG_TEST_2 || iEvent == enumTCPIpFactoryt_PG_TEST_3 || iEvent == enumTCPIpFactoryt_PG_TEST_4)
	{
		nRtn = _ExePGCtrl(enumTCPIpFactoryt_PGEvent_3, 0);
		if (nRtn != VTM_RV_SUCCESSED)
		{
			if (nRtn != HEAD_NOT_CONNECTED)
			{
				nRtn = PG_MODULE_RUNNING;
			}
			
			return nRtn;
		}
	}

	// Recipe Open
	if (iEvent == enumTCPIpFactoryt_PGEvent_0)
	{	
		// Turn Off
		stPg.iStream = 10;
		stPg.iFunc = 3;
		SetCommand(stPg, false);
		Sleep(1000);

		str.Format(_T("%s"), m_pRecipeEditMgr->GetRecipeID()); 

		// Model
		stPg.iStream = 2;
		stPg.iFunc = 1;
		stPg.iCmd = 1;
		_tcscpy_s(stPg.szModel, _countof(stPg.szModel), str);

		strLog.Format(_T("Model Change Start"));
		m_pLogFileAccessor->KMLOG(OPER, strLog);

		nRtn = SetCommand(stPg, false);
	}
	else if (iEvent == enumTCPIpFactoryt_PGEvent_1)	// Wait
	{
		nTimeOut = DEFINE_RECIPE_WAIT_TIME;

		nRtn = WaitCommandRecive(nTimeOut);
		if (nRtn != VTM_RV_SUCCESSED)
		{
			strLog.Format(_T("Model Change Fail"));
		}
		else
		{
			strLog.Format(_T("Model Change End"));
		}

		m_pLogFileAccessor->KMLOG(OPER, strLog);
	}
	else if (iEvent == enumTCPIpFactoryt_PGEvent_2)
	{
		// Recipe
		strLog.Format(_T("Recipe Change START"));
		m_pLogFileAccessor->KMLOG(OPER, strLog);

		stPg.iStream = 2;
		stPg.iFunc = 9;

	//	str.Format("%s", m_pSystem->stLotInfo.m_szRecipeID);

		str.Format(_T("%s"), m_pRecipeEditMgr->GetRecipeID());
		_tcscpy_s(stPg.szRecipe, _countof(stPg.szRecipe), str);

		nRtn = SetCommand(stPg, false);
		if (nRtn == VTM_RV_SUCCESSED)
		{
			nRtn = WaitCommandRecive(nTimeOut);
		}
			
		if (nRtn != VTM_RV_SUCCESSED)
		{
			strLog.Format(_T("Recipe Change Fail"));
		}
		else
		{
			strLog.Format(_T("Recipe Change End"));
		}

		m_pLogFileAccessor->KMLOG(OPER, strLog);

		if (nRtn != VTM_RV_SUCCESSED)
		{
			return nRtn;
		}

		// Aging
		stPg.iStream = 2;
		stPg.iFunc = 11;
		stPg.szRecipe;
	//	str.Format("%s", m_pSystem->stLotInfo.m_szRecipeID);

		str.Format(_T("%s"), m_pRecipeEditMgr->GetRecipeID());
		_tcscpy_s(stPg.szRecipe, _countof(stPg.szRecipe), str);

		strLog.Format(_T("Aging Recipe Change START"));
		m_pLogFileAccessor->KMLOG(OPER, strLog);
	
		nRtn = SetCommand(stPg, false);
		if (nRtn == VTM_RV_SUCCESSED)
		{
			nRtn = WaitCommandRecive(nTimeOut);
		}

		if (nRtn != VTM_RV_SUCCESSED)
		{
			strLog.Format(_T("Aging Recipe Change Fail"));
		}
		else
		{
			strLog.Format(_T("Aging Recipe Change End"));
		}

		m_pLogFileAccessor->KMLOG(OPER, strLog);
	}

	else if (iEvent == enumTCPIpFactoryt_PGEvent_3)
	{
		stPg.iStream = 5;
		stPg.iFunc = 1;

		nRtn = SetCommand(stPg);
	}
	
	else if (iEvent == enumTCPIpFactoryt_PGEvent_5)
	{
		stPg.iStream = 8;
		stPg.iFunc = 1;
		stPg.iCmd = 1;
		stPg.iCellNum = 0;

		nRtn = SetCommand(stPg);
		if (nRtn == VTM_RV_SUCCESSED)
		{
			nRtn = WaitInspectionComplete(nTimeOut);
		}

	}
	else if (iEvent == enumTCPIpFactoryt_PGEvent_6)
	{
		stPg.iStream = 8;
		stPg.iFunc = 1;
		stPg.iCmd = 3;
		stPg.iCellNum = 0;

		nRtn = SetCommand(stPg);
		if (nRtn == VTM_RV_SUCCESSED)
		{
			nRtn = WaitInspectionComplete(nTimeOut);
		}
	}
	else if (iEvent == enumTCPIpFactoryt_PGEvent_7)
	{
		stPg.iStream = 8;
		stPg.iFunc = 1;
		stPg.iCmd = 4;
		stPg.iCellNum = 0;

		nRtn = SetCommand(stPg);
		if (nRtn == VTM_RV_SUCCESSED)
		{
			nRtn = WaitInspectionComplete(nTimeOut);
		}
	}
	else if (iEvent == enumTCPIpFactoryt_PGEvent_8)		// Turn Off 
	{
		stPg.iStream = 10;
		stPg.iFunc = 3;

		nRtn = SetCommand(stPg);
	}

	else if (iEvent == enumTCPIpFactoryt_PGEvent_9)
	{
		CCriticalSectionSync single(&m_cs_Set, TRUE);

		stPg.iStream = 3;
		stPg.iFunc = 11;
		stPg.iCmd = 1;

		nRtn = SetCommand(stPg);
		/*if (nRtn == VTM_RV_SUCCESSED)
		{
			nRtn = WaitInspectionEnd(nTimeOut);
		}*/
	}
	else if (iEvent == enumTCPIpFactoryt_PGEvent_10)
	{
		CCriticalSectionSync single(&m_cs_Set, TRUE);

		stPg.iStream = 3;
		stPg.iFunc = 13;
		stPg.iCmd = 1;

		nRtn = SetCommand(stPg);
		/*if (nRtn == VTM_RV_SUCCESSED)
		{
			nRtn = WaitInspectionEnd(nTimeOut);
		}*/
	}
	else if (iEvent == enumTCPIpFactoryt_PGEvent_11)
	{
		CCriticalSectionSync single(&m_cs_Set, TRUE);

		stPg.iStream = 3;
		stPg.iFunc = 15;
		stPg.iCmd = 1;

		nRtn = SetCommand(stPg);
		/*if (nRtn == VTM_RV_SUCCESSED)
		{
			nRtn = WaitInspectionComplete(nTimeOut);
		}*/
	}
	else if (iEvent == enumTCPIpFactoryt_PGEvent_12)		// Turn Off
	{
		CCriticalSectionSync single(&m_cs_Set, TRUE);

		stPg.iStream = 3;
		stPg.iFunc = 17;
		stPg.iCmd = 1;

		nRtn = SetCommand(stPg, true);
	}
	else if (iEvent == enumTCPIpFactoryt_PGEvent_13)
	{
		CCriticalSectionSync single(&m_cs_Set, TRUE);

		stPg.iStream = 3;
		stPg.iFunc = 11;
		stPg.iCmd = 2;

		nRtn = SetCommand(stPg);
		/*if (nRtn == VTM_RV_SUCCESSED)
		{
			nRtn = WaitInspectionComplete(nTimeOut);
		}*/
	}
	else if (iEvent == enumTCPIpFactoryt_PGEvent_14)
	{
		CCriticalSectionSync single(&m_cs_Set, TRUE);

		stPg.iStream = 3;
		stPg.iFunc = 13;
		stPg.iCmd = 2;

		nRtn = SetCommand(stPg);
		/*if (nRtn == VTM_RV_SUCCESSED)
		{
			nRtn = WaitInspectionComplete(nTimeOut);
		}*/
	}
	else if (iEvent == enumTCPIpFactoryt_PGEvent_15)
	{
		CCriticalSectionSync single(&m_cs_Set, TRUE);
		stPg.iStream = 3;
		stPg.iFunc = 15;
		stPg.iCmd = 2;

		nRtn = SetCommand(stPg);
		/*if (nRtn == VTM_RV_SUCCESSED)
		{
			nRtn = WaitInspectionComplete(nTimeOut);
		}*/
	}
	else if (iEvent == enumTCPIpFactoryt_PGEvent_16)		// Turn Off
	{
		CCriticalSectionSync single(&m_cs_Set, TRUE);

		stPg.iStream = 3;
		stPg.iFunc = 17;
		stPg.iCmd = 2;

		nRtn = SetCommand(stPg, true);
	}


	else if (iEvent == enumTCPIpFactoryt_PG_TEST_1)		// 사전검사	// Not Contact
	{
		stPg.iStream = 8;
		stPg.iFunc = 1;
		stPg.iCmd = 6;		// PTP
		stPg.iCellNum = iCellNum;	// ALL: 0

		strLog.Format(_T("PTP START"));
		m_pLogFileAccessor->KMLOG(OPER, strLog);
	
		nTimeOut = DEFINE_AGING_WAIT_TIME;

		nTimeOut = m_pRecipeEditMgr->GetRecipeInfo()->m_sInspectItem.m_nTimeOut[0];

		m_bAgingingEven = false;

		nRtn = SetCommand(stPg, false);
		if (nRtn == VTM_RV_SUCCESSED)
		{
			nRtn = WaitInspectionComplete(nTimeOut, 6);
		}

		if (nRtn != VTM_RV_SUCCESSED)
		{
			strLog.Format(_T("PTP FAIL"));
			m_pLogFileAccessor->KMLOG(WARN, strLog);
		}
		/*else
		{
			strLog.Format(_T("PTP END"));
		}

		m_pLogFileAccessor->KMLOG(OPER, strLog); */
	}
	else if (iEvent == enumTCPIpFactoryt_PG_TEST_2)		// 사전검사 // Contact
	{
		stPg.iStream = 8;
		stPg.iFunc = 1;
		stPg.iCmd = 5;		// OST
		stPg.iCellNum = iCellNum;	// ALL: 0

		strLog.Format(_T("OST START"));
		m_pLogFileAccessor->KMLOG(OPER, strLog);

		nTimeOut = DEFINE_AGING_WAIT_TIME;
		nTimeOut = m_pRecipeEditMgr->GetRecipeInfo()->m_sInspectItem.m_nTimeOut[0];

		m_bAgingingEven = false;

		nRtn = SetCommand(stPg, false);
		if (nRtn == VTM_RV_SUCCESSED)
		{
			nRtn = WaitInspectionComplete(nTimeOut, 5);
		}

		if (nRtn != VTM_RV_SUCCESSED)
		{
			strLog.Format(_T("OST FAIL"));
			m_pLogFileAccessor->KMLOG(WARN, strLog);
		}
		/*else
		{
			strLog.Format(_T("OST END"));
		}

		m_pLogFileAccessor->KMLOG(OPER, strLog);*/
	}
	else if (iEvent == enumTCPIpFactoryt_PG_TEST_3)		// 사전검사 // Contact
	{
		stPg.iStream = 8;
		stPg.iFunc = 1;
		stPg.iCmd = 7;		// RTEST
		stPg.iCellNum = iCellNum;	// ALL: 0

		strLog.Format(_T("RTEST START"));
		m_pLogFileAccessor->KMLOG(OPER, strLog);

		nTimeOut = DEFINE_AGING_WAIT_TIME;
		nTimeOut = m_pRecipeEditMgr->GetRecipeInfo()->m_sInspectItem.m_nTimeOut[1];

		m_bAgingingEven = false;

		nRtn = SetCommand(stPg, false);
		if (nRtn == VTM_RV_SUCCESSED)
		{
			nRtn = WaitInspectionComplete(nTimeOut, 7);
		}

		if (nRtn != VTM_RV_SUCCESSED)
		{
			strLog.Format(_T("R-TEST FAIL"));
			m_pLogFileAccessor->KMLOG(WARN, strLog);
		}
	/*	else
		{
			strLog.Format(_T("R-TEST END"));
		}

		m_pLogFileAccessor->KMLOG(OPER, strLog);*/
	}
	else if (iEvent == enumTCPIpFactoryt_PG_TEST_4)		// 전기검사
	{
		// AGING
		stPg.iStream = 6;
		stPg.iFunc = 1;
		
	//	stPg.iCmd = 1;
		if (m_pRecipeEditMgr->GetRecipeInfo()->m_sInspectItem.m_bUseRTest == TRUE)	// jsyoon 2022.08.19 add
		{
			stPg.iCmd = 1;
		}
		else
		{
			stPg.iCmd = 0;
		}
		
		stPg.iCellNum = 0;	// ODD

		strLog.Format(_T("AGING START"));
		m_pLogFileAccessor->KMLOG(OPER, strLog);

		nTimeOut = DEFINE_AGING_WAIT_TIME;
		nTimeOut = m_pRecipeEditMgr->GetRecipeInfo()->m_sInspectItem.m_nTimeOut[2];

		m_bAginging = true;
		m_bAgingingEven = false;

		if (IsOutOfService() == true)
		{
			S_RecipeMgr* pslData = nullptr;
			pslData = m_pRecipeEditMgr->GetRecipeMgrPtr();
			int nStickCnt = 0, nCellCnt = 0;
			nCellCnt = pslData->m_nCellCnt;

			int iStick = 0;
			iStick = m_pShareMemoryMgr->GetSharedMemory()->m_nAgingStartStick[4];

			if (nCellCnt > 10)
			{
				for (size_t i = 0; i < nCellCnt; i++)
				{
					if (i % 2)
					{
						continue;
					}
					else
					{
						if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS ||
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_NG || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_OK)
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_START;
					}
				}
			}
			else
			{
				for (size_t i = 0; i < nCellCnt; i++)
				{
						if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS ||
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_NG || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_OK )
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_START;
				}
			}
		}

		nRtn = SetCommand(stPg, true);
	}
	
	else if (iEvent == enumTCPIpFactoryt_PG_TEST_11)		// 사전검사	// Not Contact
	{
		stPg.iStream = 8;
		stPg.iFunc = 1;
		stPg.iCmd = 6;		// PTP
		stPg.iCellNum = iCellNum;	// ALL: 0
		stPg.iOption = 1; // EVEN

		strLog.Format(_T("PTP Even START"));
		m_pLogFileAccessor->KMLOG(OPER, strLog);

		nTimeOut = DEFINE_AGING_WAIT_TIME;

		nTimeOut = m_pRecipeEditMgr->GetRecipeInfo()->m_sInspectItem.m_nTimeOut[0];

		m_bAgingingEven = true;

		nRtn = SetCommand(stPg, false);
		if (nRtn == VTM_RV_SUCCESSED)
		{
			nRtn = WaitInspectionComplete(nTimeOut, 6);
		}

		if (nRtn != VTM_RV_SUCCESSED)
		{
			strLog.Format(_T("PTP Even FAIL"));
			m_pLogFileAccessor->KMLOG(WARN, strLog);
		}
	/*	else
		{
			strLog.Format(_T("PTP Even END"));
		}

		m_pLogFileAccessor->KMLOG(OPER, strLog);*/
	}
	else if (iEvent == enumTCPIpFactoryt_PG_TEST_12)		// 사전검사 // Contact
	{
		stPg.iStream = 8;
		stPg.iFunc = 1;
		stPg.iCmd = 5;		// OST
		stPg.iCellNum = iCellNum;	// ALL: 0
		stPg.iOption = 1; // EVEN

		strLog.Format(_T("OST Even START"));
		m_pLogFileAccessor->KMLOG(OPER, strLog);

		nTimeOut = DEFINE_AGING_WAIT_TIME;
		nTimeOut = m_pRecipeEditMgr->GetRecipeInfo()->m_sInspectItem.m_nTimeOut[0];

		m_bAgingingEven = true;

		nRtn = SetCommand(stPg, false);
		if (nRtn == VTM_RV_SUCCESSED)
		{
			nRtn = WaitInspectionComplete(nTimeOut, 5);
		}

		if (nRtn != VTM_RV_SUCCESSED)
		{
			strLog.Format(_T("OST Even FAIL"));
			m_pLogFileAccessor->KMLOG(WARN, strLog);
		}
		/*else
		{
			strLog.Format(_T("OST Even END"));
		}

		m_pLogFileAccessor->KMLOG(OPER, strLog);*/
	}
	else if (iEvent == enumTCPIpFactoryt_PG_TEST_13)		// 사전검사 // Contact
	{
		stPg.iStream = 8;
		stPg.iFunc = 1;
		stPg.iCmd = 7;		// RTEST
		stPg.iCellNum = iCellNum;	// ALL: 0
		stPg.iOption = 1; // EVEN

		strLog.Format(_T("R-TEST Even START"));
		m_pLogFileAccessor->KMLOG(OPER, strLog);

		nTimeOut = DEFINE_AGING_WAIT_TIME;
		nTimeOut = m_pRecipeEditMgr->GetRecipeInfo()->m_sInspectItem.m_nTimeOut[1];

		m_bAgingingEven = true;

		nRtn = SetCommand(stPg, false);
		if (nRtn == VTM_RV_SUCCESSED)
		{
			nRtn = WaitInspectionComplete(nTimeOut, 7);
		}

		if (nRtn != VTM_RV_SUCCESSED)
		{
			strLog.Format(_T("R-TEST Even FAIL")); 
			m_pLogFileAccessor->KMLOG(WARN, strLog);
		}
		//else
		//{
		//	strLog.Format(_T("R-TEST Even END"));
		//}

		//m_pLogFileAccessor->KMLOG(OPER, strLog);
	}
	else if (iEvent == enumTCPIpFactoryt_PG_TEST_14)		// 전기검사
	{
		// AGING
		stPg.iStream = 6;
		stPg.iFunc = 1;	
	//	stPg.iCmd = 1;
		if (m_pRecipeEditMgr->GetRecipeInfo()->m_sInspectItem.m_bUseRTest == TRUE)	// jsyoon 2022.08.19 add
		{
			stPg.iCmd = 1;
		}
		else
		{
			stPg.iCmd = 0;
		}
		
		stPg.iOption = 1; // EVEN

		strLog.Format(_T("AGING Even START"));
		m_pLogFileAccessor->KMLOG(OPER, strLog);

		nTimeOut = DEFINE_AGING_WAIT_TIME;
		nTimeOut = m_pRecipeEditMgr->GetRecipeInfo()->m_sInspectItem.m_nTimeOut[2];

		m_bAginging = true;
		m_bAgingingEven = true;

		if (IsOutOfService() == true)
		{
			S_RecipeMgr* pslData = nullptr;
			pslData = m_pRecipeEditMgr->GetRecipeMgrPtr();
			int nStickCnt = 0, nCellCnt = 0;
			nCellCnt = pslData->m_nCellCnt;

			int iStick = 0;
			iStick = m_pShareMemoryMgr->GetSharedMemory()->m_nAgingStartStick[4];

			for (size_t i = 0; i < nCellCnt; i++)
			{
				if (nCellCnt > 10)
				{
					if (i % 2)
					{
						if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS ||
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_NG || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_OK)
							m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_START;
					}
					else
					{
						continue;
					}
				}
				else
				{
					if (m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_ILDE || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PTP_NOR ||
						m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_OST_NOR || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_PASS ||
						m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_NG || m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] == df_CELLSTATE_AGING_OK)
						m_pShareMemoryMgr->GetSharedMemory()->nCellStatus[i][iStick] = df_CELLSTATE_AGING_START;
				}
			}
		}

		nRtn = SetCommand(stPg, true);
	}
	else if (iEvent == enumTCPIpFactoryt_PG_TEST_5)		// TEST STOP
	{
		// STOP
		stPg.iStream = 10;
		stPg.iFunc = 1;

		nRtn = SetCommand(stPg, false);

//		m_bAginging = false;
	}
	else if (iEvent == enumTCPIpFactoryt_PG_TP_1)		// 특성검사 TP
	{
		stPg.iStream = 1;
		stPg.iFunc = 1;
		stPg.iCmd = 1;

		nRtn = SetCommand(stPg);
	}
	else if (iEvent == enumTCPIpFactoryt_PG_TP_2)		// 화질검사 TP
	{
		stPg.iStream = 1;
		stPg.iFunc = 1;
		stPg.iCmd = 2;

		nRtn = SetCommand(stPg);
	}
	else if (iEvent == enumTCPIpFactoryt_PG_TP_3)		// 색이상검사 TP
	{
		stPg.iStream = 1;
		stPg.iFunc = 1;
		stPg.iCmd = 3;

		nRtn = SetCommand(stPg);
	}
	else if (iEvent == enumTCPIpFactoryt_PG_TP_4)		//  TP
	{
		stPg.iStream = 1;
		stPg.iFunc = 1;
		stPg.iCmd = 4;

		nRtn = SetCommand(stPg);
	}
	else if (iEvent == enumTCPIpFactoryt_PG_TP_5)		//  TP
	{
		stPg.iStream = 1;
		stPg.iFunc = 1;
		stPg.iCmd = 5;

		nRtn = SetCommand(stPg);
	}
	//else if (iEvent == enumTCPIpFactoryt_PG_TP_6)		//  TP
	//{
	//	stPg.iStream = 1;
	//	stPg.iFunc = 1;
	//	stPg.iCmd = 6;

	//	nRtn = SetCommand(stPg);
	//}


	else if (iEvent == enumTCPIpFactoryt_PG_SET_LEFT)		// Cell 선택 Left
	{
		stPg.iStream = 1;
		stPg.iFunc = 5;
		stPg.iCmd = 0;
		stPg.iCellNum = iCellNum;

		nRtn = SetCommand(stPg);
	}
	else if (iEvent == enumTCPIpFactoryt_PG_SET_RIGHT)		// Cell 선택 Right
	{
		stPg.iStream = 1;
		stPg.iFunc = 5;
		stPg.iCmd = 1;
		stPg.iCellNum = iCellNum;

		nRtn = SetCommand(stPg);
	}
	

	return nRtn;
}

bool CCommTcpIpObjectPG::SetUserStop(int iCellNum)
{
	int nRet = VTM_RV_SUCCESSED;

	if (m_bAginging == true)
	{
		nRet = _ExePGCtrl(enumTCPIpFactoryt_PG_TEST_5, iCellNum);

	//	m_bAginging = false;
	}
	
	if (nRet == VTM_RV_SUCCESSED)
		return true;
	else
		return false;
}