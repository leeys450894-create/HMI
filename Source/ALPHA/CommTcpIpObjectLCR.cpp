#include "pch.h"
#include "CommTcpIpObjectLCR.h"

#include "Manager\NetworkConfigMgr.h"
#include "Manager\SystemConfigMgr.h"
#include "Recipe\RecipeEditMgr.h"
#include "SystemParamShare.h"
#include "Alarm\AlarmController.h"
#include "AuroraFuncDefine.h"
#include "Manager\SequenceManager.h"

#include "CommEthFunction\EthernetManagerTCP.h"
#include "common\CommonFunc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCommTcpIpObjectLCR::CCommTcpIpObjectLCR(int iSeqItemIndex, stAuroraMgrPool& MgrPool)
	: m_pSystemConfigMgr(MgrPool.pSystemConfigMgr)
	, m_pRecipeEditMgr(MgrPool.pRecipeEditMgr)
	, m_pResultCollectMgr(MgrPool.pResultCollectMgr)
	, m_pShareMemory(MgrPool.pShareMemoryMgr)
	, m_pLogFileAccessor(new CLogFileAccessor)
	, m_bOutOfService(false)
	, m_iSeqItemIndex(0)
{
	m_strQueueMessage.Empty();
	m_pRecipeDataClone = nullptr;

//	m_iSeqItemIndex = MgrPool.pNetworkConfigMgr->GetParam_DeviceIndex(iSeqItemIndex);
	CString strObjectName;
	strObjectName.Format(_T("%s"), _T("LCR"));

	m_iSeqItemIndex = enumTCPIpFactorytID_LCR;

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
}


CCommTcpIpObjectLCR::~CCommTcpIpObjectLCR()
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

void CCommTcpIpObjectLCR::UpdateRecipeInformation()
{
	m_pRecipeDataClone->Clone(m_pRecipeEditMgr->GetRecipeDataPtr());
}

bool CCommTcpIpObjectLCR::SetTime_Sync()
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

bool CCommTcpIpObjectLCR::RecipeChange()
{
	int nRet = VTM_RV_SUCCESSED;
	nRet = _ExeMeas(enumTCPIpFactoryt_Event_ELP_1);
	if (nRet != VTM_RV_SUCCESSED)
		return false;

	return true;
}

bool CCommTcpIpObjectLCR::SetParameter()
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
			strLog.Format(_T("[%s] : %s"), GetName(), _T("SetParameter, HEAD_ACK_TIMEOUT"));
			m_pLogFileAccessor->KMLOG(WARN, strLog);
			CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iSeqItemIndex);
			return false;
		}
		else if (dw == (WAIT_OBJECT_0 + 1)) // NG
		{
			strLog.Format(_T("[%s] : %s"), GetName(), _T("SetParameter, HEAD_ACK_FAIL"));
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

	strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_NOT_CONNECTED"));
	m_pLogFileAccessor->KMLOG(WARN, strLog);
	CAlarmController::GetInstance()->SetAlarm(HEAD_NOT_CONNECTED + m_iSeqItemIndex);

	return false;
}

bool CCommTcpIpObjectLCR::InspectStart(int iMeasType, bool bWait, int iStepNum)
{
	int nRet = VTM_RV_SUCCESSED;

	if (iMeasType == 1)		// Left
	{
		nRet = _ExeMeas(enumTCPIpFactoryt_Event_ELP_TEST_LEFT, bWait);
	}
	else if (iMeasType == 2) // Right
	{
		nRet = _ExeMeas(enumTCPIpFactoryt_Event_ELP_TEST_RIGHT, bWait);
	}
	else // All
	{
		nRet = _ExeMeas(enumTCPIpFactoryt_Event_ELP_TEST_LEFT, false);
		if (nRet == VTM_RV_SUCCESSED)
		{
			nRet = _ExeMeas(enumTCPIpFactoryt_Event_ELP_TEST_RIGHT, false);
		}

		if (bWait == true)
		{
			if (nRet == VTM_RV_SUCCESSED)
			{
				nRet = InspectStartWait();
			}
		}
	}

	if (nRet == VTM_RV_SUCCESSED)
		return true;
	else
		return false;
}

bool CCommTcpIpObjectLCR::InspectStartWait()
{
	if (IsOutOfService() == true)
		return true;

	int nRet = VTM_RV_SUCCESSED;
	CString strLog;
	HANDLE hRecvEvent1[2] = { m_hRecvDEvent[enumRecvEventID_INSP_START_OK], m_hRecvDEvent[enumRecvEventID_INSP_START_NG] };
	DWORD dw = WaitForMultipleObjects(2, hRecvEvent1, FALSE, DEFINE_INSPECITON_WAIT_TIME);
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
		nRet = VTM_RV_SUCCESSED;
	}

	if (nRet == VTM_RV_SUCCESSED)
	{
		HANDLE hRecvEvent2[2] = { m_hRecvDEvent[enumRecvEventID_INSP2_START_OK], m_hRecvDEvent[enumRecvEventID_INSP2_START_NG] };
		DWORD dw2 = WaitForMultipleObjects(2, hRecvEvent2, FALSE, DEFINE_INSPECITON_WAIT_TIME);
		if (dw2 == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
		{
			strLog.Format(_T("[%s] : %s"), GetName(), _T("InspectStartWait, HEAD_ACK_TIMEOUT"));
			m_pLogFileAccessor->KMLOG(WARN, strLog);
			CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iSeqItemIndex);
			nRet = HEAD_ACK_FAIL;
		}
		else if (dw2 == (WAIT_OBJECT_0 + 1)) // NG
		{
			strLog.Format(_T("[%s] : %s"), GetName(), _T("InspectStartWait, HEAD_ACK_FAIL"));
			m_pLogFileAccessor->KMLOG(WARN, strLog);
			CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_FAIL + m_iSeqItemIndex);
			nRet = HEAD_ACK_FAIL;
		}
		else
		{
			nRet = VTM_RV_SUCCESSED;
		}
	}

	if (nRet == VTM_RV_SUCCESSED)
		return true;
	else
		return false;
}

bool CCommTcpIpObjectLCR::InspectEnd()
{
	return true;

	//CString strReturnCommand;
	//strReturnCommand.Format(_T("%s^%s"), GetStringCMDID(COMM_USER_DEFINE_REQ), _T("INSP_END"));

	//CString strLog;
	//strLog.Format(_T("[%s] : %s"), GetName(), strReturnCommand);
	//m_pLogFileAccessor->KMLOG(SEND, strLog);

	//ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_END_OK]);
	//ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_END_NG]);

	//if (IsOutOfService() == true)
	//	return true;

	//if (SendPacket(strReturnCommand) == true)
	//{
	//	HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_INSP_END_OK], m_hRecvDEvent[enumRecvEventID_INSP_END_NG] };

	//	DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, DEFINE_RESPONSE_WAIT_TIME);
	//	if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
	//	{
	//		strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_TIMEOUT"));
	//		m_pLogFileAccessor->KMLOG(WARN, strLog);
	//		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iSeqItemIndex);
	//		return false;
	//	}
	//	// jsyoon 2022.01.17 add
	//	else if (dw == (WAIT_OBJECT_0 + 1)) // NG
	//	{
	//		strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_FAIL"));
	//		m_pLogFileAccessor->KMLOG(WARN, strLog);
	//		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_FAIL + m_iSeqItemIndex);
	//		return false;
	//	}
	//	else
	//	{
	//		return true;
	//	}

	//	//	return true;
	//}

	//strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_NOT_CONNECTED"));
	//m_pLogFileAccessor->KMLOG(WARN, strLog);
	//CAlarmController::GetInstance()->SetAlarm(HEAD_NOT_CONNECTED + m_iSeqItemIndex);
	//return false;
}

bool CCommTcpIpObjectLCR::WaitInspectionEnd(DWORD dwTimeOut)
{
	if (IsOutOfService() == true)
		return true;

	int nRet = VTM_RV_SUCCESSED;
	
	if (nRet == VTM_RV_SUCCESSED)
		return true;
	else
		return false;
}

bool CCommTcpIpObjectLCR::WaitInspectionComplete(DWORD dwTimeOut)
{
	if (IsOutOfService() == true)
		return true;

	if (IsConnected() != true) return false;

	CString strLog;
	int nRet = VTM_RV_SUCCESSED;

	if (nRet == VTM_RV_SUCCESSED)
	{
		HANDLE hRecvEvent3[2] = { m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_OK], m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_NG] };
		DWORD dw3 = WaitForMultipleObjects(2, hRecvEvent3, FALSE, dwTimeOut * 1000);
		if (dw3 == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
		{
			strLog.Format(_T("[%s] : %s"), GetName(), _T("WaitInspectionComplete, HEAD_ACK_TIMEOUT"));
			m_pLogFileAccessor->KMLOG(WARN, strLog);
			CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iSeqItemIndex);
			nRet = HEAD_ACK_FAIL;
		}
		else if (dw3 == (WAIT_OBJECT_0 + 1)) // NG
		{
			strLog.Format(_T("[%s] : %s"), GetName(), _T("WaitInspectionComplete, HEAD_ACK_FAIL"));
			m_pLogFileAccessor->KMLOG(WARN, strLog);
			CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_FAIL + m_iSeqItemIndex);
			nRet = HEAD_ACK_FAIL;
		}
		else
		{
			nRet = VTM_RV_SUCCESSED;
		}
	}

	if (nRet == VTM_RV_SUCCESSED)
	{
		HANDLE hRecvEvent4[2] = { m_hRecvDEvent[enumRecvEventID_INSP2_COMPLETE_OK], m_hRecvDEvent[enumRecvEventID_INSP2_COMPLETE_NG] };
		DWORD dw4 = WaitForMultipleObjects(2, hRecvEvent4, FALSE, dwTimeOut * 1000);
		if (dw4 == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
		{
			strLog.Format(_T("[%s] : %s"), GetName(), _T("WaitInspectionComplete, HEAD_ACK_TIMEOUT"));
			m_pLogFileAccessor->KMLOG(WARN, strLog);
			CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iSeqItemIndex);
			nRet = HEAD_ACK_FAIL;
		}
		else if (dw4 == (WAIT_OBJECT_0 + 1)) // NG
		{
			strLog.Format(_T("[%s] : %s"), GetName(), _T("WaitInspectionComplete, HEAD_ACK_FAIL"));
			m_pLogFileAccessor->KMLOG(WARN, strLog);
			CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_FAIL + m_iSeqItemIndex);
			nRet = HEAD_ACK_FAIL;
		}
		else
		{
			nRet = VTM_RV_SUCCESSED;
		}
	}

	if (nRet == VTM_RV_SUCCESSED)
		return true;
	else
		return false;
}


//=========================================================================
// INTERFACE 관련
//=========================================================================
void CCommTcpIpObjectLCR::xNetCallbackRecieveData(LPVOID ud)
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

			//	CString strLog = _T("[RECV - UTG] : ") + strMessage;
			CString strLog;
			strLog.Format(_T("[%s] : %s"), GetName(), strMessage);

			//		CLogWrapper::GetInstance()->KMLOG(SYST, strLog);
			m_pLogFileAccessor->KMLOG(RECV, strLog);

			strMessage.Replace(_T(" "), _T(""));
			if (strMessage.IsEmpty() == true)
				return;

			//	m_strQueueMessage += strMessage;

			BOOL bAck = TRUE;
			//---------------------------------------------------------------------------
			// 패킷 단위로 응답에 대한 처리를 해야 한다.
			// socket 에 데이터가 진입하는데 2개 이상 붙어서 1개의 event 로 들어온다.
			// 모든 패킷에 대해서 command 단위로 분리해서 수행해야 한다.
			//---------------------------------------------------------------------------
			CString strCmd;

			std::vector<CString> vecPacketCommandPool = GetExeHextoStr(strMessage);

			for (size_t i = 0; i < vecPacketCommandPool.size(); ++i)
			{
				const CString strRevCmd = vecPacketCommandPool[i];

				response_head stRcvPg;
				ZeroMemory(&stRcvPg, sizeof(stRcvPg));

				if (strRevCmd.GetLength() < 1)
				{
					SetEvent(m_hRecvDEvent[enumRecvEventID_SET_PARAMETER_NG]);
					return;
				}

				int nEventOK = enumRecvEventID_SET_PARAMETER_OK, nEventNG = enumRecvEventID_SET_PARAMETER_NG;

				// Response Command Header
				if (strRevCmd.GetLength() >= 6)
				{
					TCHAR c;
					c = (TCHAR)strRevCmd.GetAt(0);
					strCmd.Format(_T("%x"), c);

					stRcvPg.iCmd = _ttoi(strCmd);

					//	stRcvPg.iCmd   = _ttoi(strRevCmd.Mid(0, 1));
					stRcvPg.iError = _ttoi(strRevCmd.Mid(1, 1));
					stRcvPg.iCause = _ttoi(strRevCmd.Mid(2, 1));
					stRcvPg.iLen = _ttoi(strRevCmd.Mid(3, 3));
				}

				// Response Command
				if (stRcvPg.iCmd == 51)	// Model Change 0x51
				{
					nEventOK = enumRecvEventID_RECIPE_CHANGE_OK, nEventNG = enumRecvEventID_RECIPE_CHANGE_NG;
				}
				if (stRcvPg.iCmd == 57)			// Start Res
				{
					if (stRcvPg.iCause == 1)
					{
						nEventOK = enumRecvEventID_INSP_START_OK, nEventNG = enumRecvEventID_INSP_START_NG;
					}
					if (stRcvPg.iCause == 2)
					{
						nEventOK = enumRecvEventID_INSP2_START_OK, nEventNG = enumRecvEventID_INSP2_START_NG;
					}

					if (stRcvPg.iLen == 32)
					{

					}

					/*	if (strRevCmd.GetLength() > 38)
							_tcscpy_s(stRcvPg.InnerID[0], _countof(stRcvPg.InnerID[0]), strPGCmd.Mid(7, 32));*/
				}
				else if (stRcvPg.iCmd == 58)	// End Res
				{
					if (stRcvPg.iCause == 1)
					{
						nEventOK = enumRecvEventID_INSP_COMPLETE_OK, nEventNG = enumRecvEventID_INSP_COMPLETE_NG;
					}
					if (stRcvPg.iCause == 2)
					{
						nEventOK = enumRecvEventID_INSP2_COMPLETE_OK, nEventNG = enumRecvEventID_INSP2_COMPLETE_NG;
					}

					/*
					  0x30 : 결과 OK(양품) or BIN1
					  0x31 : 결과 NG or REJECT
					  0x32 : 결과 BIN2
					  */

					if (stRcvPg.iLen == 33)
					{
						stRcvPg.iResult = _ttoi(strRevCmd.Mid(7, 1));

			//			_tcscpy_s(stRcvPg.InnerID[0], _countof(stRcvPg.InnerID[0]), strPGCmd.Mid(8, 32));
					}
				}
				else
				{
					;
				}

				if (stRcvPg.iError != 0)
				{
					SetEvent(m_hRecvDEvent[nEventNG]);
				}
				else
				{
					if (bAck)	SetEvent(m_hRecvDEvent[nEventOK]);
				}

				strLog.Format(_T("[ELP] Cmd(%2d), Error Code(%2d) Cause(%2d) Len(%03d)"), stRcvPg.iCmd, stRcvPg.iError, stRcvPg.iCause, stRcvPg.iLen);
				m_pLogFileAccessor->KMLOG(OPER, strLog);
			}
		}
	}
}

CLogFileAccessor* CCommTcpIpObjectLCR::GetLogFileAccessorPtr()
{
	return m_pLogFileAccessor;
}

int CCommTcpIpObjectLCR::SetCommand(ELPPROTOCOL stPg, bool bWait)
{
	int iRet = VTM_RV_SUCCESSED;
	int i = 0, iLen = 0;
	BYTE nByte = 0x30, nCmd, nLen[3], nStx = 0x02, nEtx = 0x03;
	TCHAR	szTmp[1000];
	TCHAR	szCmd[1000];
	szTmp[0] = 0x00;
	szCmd[0] = 0x00;
	nCmd = 0x30;

	if (stPg.hrd.cmd == 0)
		return VTM_RV_FAILED;

	_stprintf_s(szCmd, _countof(szCmd), _T("%c"), nStx);		// STX

	// Head
	nCmd = stPg.hrd.cmd + nByte;
	_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nCmd);	_tcscat_s(szCmd, _countof(szCmd), szTmp);

	nLen[0] = stPg.hrd.len / 100 + nByte;
	_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nLen[0]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
	nLen[1] = (stPg.hrd.len % 100) / 10 + nByte;
	_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nLen[1]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
	nLen[2] = (stPg.hrd.len % 100) % 10 + nByte;
	_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nLen[2]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
	//////////////////////////////////////////////////////////////////////////

	if (stPg.hrd.cmd == 1)			// Model Change
	{
		iLen = _tcsclen(stPg.szModel);
		for (i = 0; i < 32; i++)
		{
			if (i < iLen)
			{
				_stprintf_s(szTmp, _countof(szTmp), _T("%c"), stPg.szModel[i]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
			}
			else
			{
				_stprintf_s(szTmp, _countof(szTmp), _T("%s"), _T("0"));				_tcscat_s(szCmd, _countof(szCmd), szTmp);
			}
		}
	}
	else if (stPg.hrd.cmd == 5)	 // Test Start
	{
		// Select PG
		nCmd = stPg.Select + nByte;
		_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nCmd);	_tcscat_s(szCmd, _countof(szCmd), szTmp);

		// ReTest
		nCmd = stPg.Retest + nByte;
		_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nCmd);	_tcscat_s(szCmd, _countof(szCmd), szTmp);

		iLen = _tcsclen(stPg.InnerID[0]);
		for (i = 0; i < 32; i++)
		{
			if (i < iLen)
			{
				_stprintf_s(szTmp, _countof(szTmp), _T("%c"), stPg.InnerID[0][i]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
			}
			else
			{
				_stprintf_s(szTmp, _countof(szTmp), _T("%s"), _T(" "));				_tcscat_s(szCmd, _countof(szCmd), szTmp);
			}
		}

		iLen = _tcsclen(stPg.InnerID[1]);
		for (i = 0; i < 32; i++)
		{
			if (i < iLen)
			{
				_stprintf_s(szTmp, _countof(szTmp), _T("%c"), stPg.InnerID[1][i]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
			}
			else
			{
				_stprintf_s(szTmp, _countof(szTmp), _T("%s"), _T(" "));				_tcscat_s(szCmd, _countof(szCmd), szTmp);
			}
		}

		iLen = _tcsclen(stPg.MCRID[0]);
		for (i = 0; i < 32; i++)
		{
			if (i < iLen)
			{
				_stprintf_s(szTmp, _countof(szTmp), _T("%c"), stPg.MCRID[0][i]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
			}
			else
			{
				_stprintf_s(szTmp, _countof(szTmp), _T("%s"), _T(" "));				_tcscat_s(szCmd, _countof(szCmd), szTmp);
			}
		}

		iLen = _tcsclen(stPg.MCRID[1]);
		for (i = 0; i < 32; i++)
		{
			if (i < iLen)
			{
				_stprintf_s(szTmp, _countof(szTmp), _T("%c"), stPg.MCRID[1][i]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
			}
			else
			{
				_stprintf_s(szTmp, _countof(szTmp), _T("%s"), _T(" "));				_tcscat_s(szCmd, _countof(szCmd), szTmp);
			}
		}
	}

	_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nEtx);
	_tcscat_s(szCmd, _countof(szCmd), szTmp);	// EXT

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
		return VTM_RV_SUCCESSED;
	}
	else
	{
		strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_NOT_CONNECTED"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		CAlarmController::GetInstance()->SetAlarm(HEAD_NOT_CONNECTED + m_iSeqItemIndex);

		return HEAD_NOT_CONNECTED;
	}
}

int  CCommTcpIpObjectLCR::_ExeMeas(int iFunc, bool bWait)
{
	int iRet = 0;
	CString str, strCellID, strGlassID, strInnerID, strMcrID;
	ELPPROTOCOL stELP;
	ZeroMemory(&stELP, sizeof(stELP));

	int nEventOK = 0, nEventNG = 0;

	switch (iFunc)
	{
	case enumTCPIpFactoryt_Event_ELP_TEST_LEFT:		// Test Start	Left
	{
		nEventOK = enumRecvEventID_INSP_START_OK;
		nEventNG = enumRecvEventID_INSP_START_NG;

		ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_END_OK]);
		ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_END_NG]);
		ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_OK]);
		ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_NG]);

		strMcrID.Format(_T("%s"), m_pShareMemory->GetSharedMemory()->stLotInfo.m_szCellID[0]);

		stELP.hrd.cmd = 5;		// Ox35
		stELP.hrd.len = 130;
		stELP.Select = 1;

		strInnerID.Format(_T("%s"), m_pShareMemory->GetSharedMemory()->stLotInfo.m_szInnerID[0]);
		if (strInnerID.IsEmpty())
		{
			strInnerID.Format(_T("%d%s"), 1, gf_GetCurTimeString(3));
			_tcscpy_s(m_pShareMemory->GetSharedMemory()->stLotInfo.m_szInnerID[0], 32, strInnerID);
		}

		_tcscpy_s(stELP.InnerID[0], 32, strInnerID);
		_tcscpy_s(stELP.MCRID[0], 32, strMcrID);

		_tcscpy_s(stELP.InnerID[1], 32, _T(""));
		_tcscpy_s(stELP.MCRID[1], 32, _T(""));
	}
	break;

	case enumTCPIpFactoryt_Event_ELP_TEST_RIGHT:		// Test Start	Right
	{
		nEventOK = enumRecvEventID_INSP2_START_OK;
		nEventNG = enumRecvEventID_INSP2_START_NG;

		ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP2_END_OK]);
		ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP2_END_NG]);
		ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP2_COMPLETE_OK]);
		ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP2_COMPLETE_NG]);

		strMcrID.Format(_T("%s"), m_pShareMemory->GetSharedMemory()->stLotInfo.m_szCellID[1]);

		stELP.hrd.cmd = 5;		// Ox35
		stELP.hrd.len = 130;
		stELP.Select = 2;

		_tcscpy_s(stELP.InnerID[0], 32, _T(""));
		_tcscpy_s(stELP.MCRID[0], 32, _T(""));

		strInnerID.Format(_T("%s"), m_pShareMemory->GetSharedMemory()->stLotInfo.m_szInnerID[1]);
		if (strInnerID.IsEmpty())
		{
			strInnerID.Format(_T("%d%s"), 2, gf_GetCurTimeString(3));
			_tcscpy_s(m_pShareMemory->GetSharedMemory()->stLotInfo.m_szInnerID[1], 32, strInnerID);
		}

		_tcscpy_s(stELP.InnerID[1], 32, strInnerID);
		_tcscpy_s(stELP.MCRID[1], 32, strMcrID);
	}
	break;
	case enumTCPIpFactoryt_Event_ELP_1:		// Recipe Change
	{
		nEventOK = enumRecvEventID_RECIPE_CHANGE_OK;
		nEventNG = enumRecvEventID_RECIPE_CHANGE_NG;

		stELP.hrd.cmd = 1;		// Ox31
		stELP.hrd.len = 32;		// FIX
		str.Format(_T("%s"), m_pRecipeEditMgr->GetRecipeID());
		_tcscpy_s(stELP.szModel, _countof(stELP.szModel), str);
	}
	break;
	}

	ResetEvent(m_hRecvDEvent[nEventOK]);
	ResetEvent(m_hRecvDEvent[nEventNG]);

	CString strLog;
	iRet = SetCommand(stELP);
	if (iRet == VTM_RV_SUCCESSED)
	{
		if (bWait == true)
		{
			if (IsOutOfService() != true)
			{
				HANDLE hRecvEvent[2] = { m_hRecvDEvent[nEventOK], m_hRecvDEvent[nEventNG] };

				DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, DEFINE_RESPONSE_WAIT_TIME);
				if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
				{
					strLog.Format(_T("[%s] : %s"), GetName(), _T("_ExeMeas, HEAD_ACK_TIMEOUT"));
					m_pLogFileAccessor->KMLOG(WARN, strLog);
					CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iSeqItemIndex);
					return HEAD_ACK_TIMEOUT;
				}
				// jsyoon 2022.01.17 add
				else if (dw == (WAIT_OBJECT_0 + 1)) // NG
				{
					strLog.Format(_T("[%s] : %s"), GetName(), _T("_ExeMeas, HEAD_ACK_FAIL"));
					m_pLogFileAccessor->KMLOG(WARN, strLog);
					CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_FAIL + m_iSeqItemIndex);
					return HEAD_ACK_FAIL;
				}
				else
				{
					return VTM_RV_SUCCESSED;
				}
			}
		}
	}

	return iRet;
}