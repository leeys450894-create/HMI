#include "pch.h"
#include "CommTcpIpObjectColor.h"

#include "Manager\NetworkConfigMgr.h"
#include "Manager\SystemConfigMgr.h"
#include "Recipe\RecipeEditMgr.h"
#include "SystemParamShare.h"
#include "Alarm\AlarmController.h"
#include "AuroraFuncDefine.h"
#include "Manager\SequenceManager.h"
#include "Manager/ResultMgr.h"
#include "CommEthFunction\EthernetManagerTCP.h"
#include "common\CommonFunc.h"
#include "Manager/CommTcpIpFactoryMgr.h"
#include "common/crc16.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCommTcpIpObjectColor::CCommTcpIpObjectColor(int iSeqItemIndex, stAuroraMgrPool& MgrPool)
	: m_pSystemConfigMgr(MgrPool.pSystemConfigMgr)
	, m_pRecipeEditMgr(MgrPool.pRecipeEditMgr)
	, m_pResultCollectMgr(MgrPool.pResultCollectMgr)
	, m_pShareMemory(MgrPool.pShareMemoryMgr)
	, m_pCommTcpIpFactoryMgr(MgrPool.pCommTcpIpFactoryMgr)
	, m_pLogFileAccessor(new CLogFileAccessor)
	, m_bOutOfService(false)
	, m_iSeqItemIndex(0)
	, m_iDeviceIndex(0)
	, m_nPatternNum(0)
	, m_RevCh(0)
{
	m_strQueueMessage.Empty();
	m_strPatternName.Empty();
	m_nPatternOn = false;

	m_pRecipeDataClone = nullptr;

	int iDeviceIndex = MgrPool.pNetworkConfigMgr->GetParam_DeviceIndex(iSeqItemIndex);
	m_iDeviceIndex = iDeviceIndex;

	m_iSeqItemIndex = enumTCPIpFactorytID_COLOR + iDeviceIndex;

	CString strObjectName;
	strObjectName.Format(_T("%s%d"), _T("면계측"), iDeviceIndex + 1);

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

	m_strNetDrive = MgrPool.pNetworkConfigMgr->GetParam_NetDrive(iSeqItemIndex);

	m_strNetRcpDrive = MgrPool.pNetworkConfigMgr->GetParam_NetRcpDrive(iSeqItemIndex);

}


CCommTcpIpObjectColor::~CCommTcpIpObjectColor()
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

void CCommTcpIpObjectColor::UpdateRecipeInformation()
{
	m_pRecipeDataClone->Clone(m_pRecipeEditMgr->GetRecipeDataPtr());
}

bool CCommTcpIpObjectColor::SetTime_Sync()
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

bool CCommTcpIpObjectColor::RecipeChange()
{
	int iRet = VTM_RV_SUCCESSED;
	CString strLog;
	bool bRet = false;

	iRet = SetCommand(2001, false);
	if (iRet == VTM_RV_SUCCESSED)
	{
		bRet = true;
	}
	
	return bRet;
}

bool CCommTcpIpObjectColor::SetParameter()
{
	return true;

	//CString strCellID, strInnerID;
	//strCellID = CSystemParamShare::GetInstance()->GetCellID();
	//strInnerID = CSystemParamShare::GetInstance()->GetInnerID();

	//CString strReturnCommand;
	//strReturnCommand.Format(_T("%s^%s^%s^%s"), GetStringCMDID(COMM_USER_DEFINE_REQ), _T("INSP_PARAM"), strCellID, strInnerID);

	//CString strLog;
	//strLog.Format(_T("[%s] : %s"), GetName(), strReturnCommand);
	//m_pLogFileAccessor->KMLOG(SEND, strLog);

	//ResetEvent(m_hRecvDEvent[enumRecvEventID_SET_PARAMETER_OK]);
	//ResetEvent(m_hRecvDEvent[enumRecvEventID_SET_PARAMETER_NG]);

	//if (IsOutOfService() == true)
	//	return true;

	//if (SendPacket(strReturnCommand) == true)
	//{
	//	HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_SET_PARAMETER_OK], m_hRecvDEvent[enumRecvEventID_SET_PARAMETER_NG] };

	//	DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, DEFINE_RESPONSE_WAIT_TIME);
	//	if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
	//	{
	//		strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_TIMEOUT"));
	//		m_pLogFileAccessor->KMLOG(WARN, strLog);
	//		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iSeqItemIndex);
	//		return false;
	//	}

	//	return true;
	//}

	//strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_NOT_CONNECTED"));
	//m_pLogFileAccessor->KMLOG(WARN, strLog);
	//CAlarmController::GetInstance()->SetAlarm(HEAD_NOT_CONNECTED + m_iSeqItemIndex);

	//return false;
}

bool CCommTcpIpObjectColor::InspectStart(int iMeasType, bool bWait, int iStepNum)
{
	CString strData, strReturnCommand;

	bool bRet = false;

	m_nPatternOn = false;

	bRet = SetCommand(2003, bWait);

	return bRet;
}

bool CCommTcpIpObjectColor::InspectStartWait()
{
	if (IsOutOfService() == true)
	{
		Sleep(100);
		return true;
	}
		

	HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_INSP_START_OK], m_hRecvDEvent[enumRecvEventID_INSP_START_NG] };

	CString strLog;
	DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, DEFINE_INSPECITON_WAIT_TIME);
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

bool CCommTcpIpObjectColor::InspectEnd()
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

bool CCommTcpIpObjectColor::WaitRecipeChangeEnd(DWORD dwTimeOut)
{
	if (IsOutOfService() == true)
		return true;

	HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_RECIPE_CHANGE_OK], m_hRecvDEvent[enumRecvEventID_RECIPE_CHANGE_NG] };

	CString strLog;
	DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, dwTimeOut * 1000);
	if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
	{
		strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_TIMEOUT, WaitRecipeChangeEnd"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iSeqItemIndex);
		return false;
	}
	// jsyoon 2022.01.17 add
	else if (dw == (WAIT_OBJECT_0 + 1)) // NG
	{
		strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_FAIL, WaitRecipeChangeEnd"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_FAIL + m_iSeqItemIndex);
		return false;
	}

	return true;
}

bool CCommTcpIpObjectColor::WaitInspectionEnd(DWORD dwTimeOut)
{
	if (IsOutOfService() == true)
		return true;

	HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_INSP_END_OK], m_hRecvDEvent[enumRecvEventID_INSP_END_NG] };

	CString strLog;
	DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, dwTimeOut * 1000);
	if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
	{
		strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_TIMEOUT, WaitInspectionEnd"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iSeqItemIndex);
		return false;
	}
	// jsyoon 2022.01.17 add
	else if (dw == (WAIT_OBJECT_0 + 1)) // NG
	{
		strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_FAIL, WaitInspectionEnd"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_FAIL + m_iSeqItemIndex);
		return false;
	}

	return true;
}

bool CCommTcpIpObjectColor::WaitInspectionComplete(DWORD dwTimeOut)
{
	if (IsOutOfService() == true)
		return true;

	HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_OK], m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_NG] };

	CString strLog;
	DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, dwTimeOut * 1000);
	if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
	{
		strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_TIMEOUT, WaitInspectionComplete"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iSeqItemIndex);
		return false;
	}
	// jsyoon 2022.01.17 add
	else if (dw == (WAIT_OBJECT_0 + 1)) // NG
	{
		strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_FAIL, WaitInspectionComplete"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_FAIL + m_iSeqItemIndex);
		return false;
	}

	return true;
}


//=========================================================================
// INTERFACE 관련
//=========================================================================
void CCommTcpIpObjectColor::xNetCallbackRecieveData(LPVOID ud)
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
			CString strMessage(pRcvData->szBody);
			//	CString strLog = _T("[RECV - UTG] : ") + strMessage;
			CString strLog;
			strLog.Format(_T("[%s] : %s"), GetName(), strMessage);

			//		CLogWrapper::GetInstance()->KMLOG(SYST, strLog);
			m_pLogFileAccessor->KMLOG(RECV, strLog);

			strMessage.Replace(_T(" "), _T(""));
			if (strMessage.IsEmpty() == true)
				return;

			CString sCmd, sLen,  sData;
			int nLen = 0;

			std::vector<CString> vecPacketCommandPool = GetExeHextoStr(strMessage); //   GetCommand_SDC(strMessage);

			for (size_t i = 0; i < vecPacketCommandPool.size(); ++i)
			{
				const CString strCommand = vecPacketCommandPool[i];

				TCHAR c = (TCHAR)strCommand.GetAt(0);

				if (c != 0x08)
				{
					continue;
				}

				sCmd = strCommand.Mid(1, 4);
				sLen = strCommand.Mid(5, 4);



				for (size_t i = 0; i < 4; i++)
				{
					if (i == 0)
						nLen += HexToByte(sLen.GetAt(i)) * (16 * 16 * 16);
					else if (i == 1)
						nLen += HexToByte(sLen.GetAt(i)) * (16 * 16);
					else if (i == 2)
						nLen += HexToByte(sLen.GetAt(i)) * (16 * 1);
					else
						nLen += HexToByte(sLen.GetAt(i));
				}

				sData = strCommand.Mid(9, nLen);


				strLog.Format(_T("[%s] >>> %s, %s"), GetName(), sCmd, sData);
				m_pLogFileAccessor->KMLOG(OPER, strLog);

				if (sCmd.Find(_T("1001")) == 0)
				{
					SetRunToThread(1002);
				}
				else if (sCmd.Find(_T("1003")) == 0)
				{
					SetRunToThread(1004);
				}
				else if (sCmd.Find(_T("1005")) == 0)
				{
					SetRunToThread(1006);
				}
				else if (sCmd.Find(_T("1007")) == 0)
				{
					SetRunToThread(1008);
				}
				else if (sCmd.Find(_T("2002")) == 0)
				{
					SetEvent(m_hRecvDEvent[enumRecvEventID_RECIPE_CHANGE_OK]);
				}
				else if (sCmd.Find(_T("2004")) == 0)
				{
					SetEvent(m_hRecvDEvent[enumRecvEventID_INSP_START_OK]);
				}
				else if (sCmd.Find(_T("2005")) == 0)	// Pattern Next
				{
					CString strParam;
					CStringArray strCodeArray;
					strCodeArray.RemoveAll();

					int cnt = 0;
					while (AfxExtractSubString(strParam, sData, cnt, ','))  // 문자를 잘라준다. (AfxExtractSubString = strTok)
					{
						strCodeArray.Add(strParam);
						cnt++;
					}

					if (strCodeArray.GetSize() > 1)
					{
						m_nPatternNum = _ttoi(strCodeArray.GetAt(1).GetString());
					}

					if (strCodeArray.GetSize() > 2)
					{
						m_strPatternName = strCodeArray.GetAt(2).GetString();
					}

					SetRunToThread(2006);
				}
				else if (sCmd.Find(_T("2008")) == 0)	// Pattern On Ack
				{

				}
				else if (sCmd.Find(_T("2009")) == 0)	// 
				{
					SetRunToThread(2010);

					//TossResultFile(m_pShareMemory->GetSharedMemory()->stLotInfo.m_szInnerID[m_iDeviceIndex], m_pCommTcpIpFactoryMgr->m_nAreaNum[m_iDeviceIndex]);

					//SetEvent(m_hRecvDEvent[enumRecvEventID_INSP_END_OK]);
					//SetEvent(m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_OK]);
				}

				// SI-MCA
				else if (sCmd.Find(_T("3002")) == 0)	// 
				{

				}
				else if (sCmd.Find(_T("3003")) == 0)	// 
				{
					SetRunToThread(3004);
				}

				// Cal Pattern
				else if (sCmd.Find(_T("8002")) == 0)
				{

				}
				else if (sCmd.Find(_T("8003")) == 0)
				{
					CString strParam;
					CStringArray strCodeArray;
					strCodeArray.RemoveAll();

					int cnt = 0;
					while (AfxExtractSubString(strParam, sData, cnt, ','))  // 문자를 잘라준다. (AfxExtractSubString = strTok)
					{
						strCodeArray.Add(strParam);
						cnt++;
					}

					if (strCodeArray.GetSize() > 1)
					{
						m_nPatternNum = _ttoi(strCodeArray.GetAt(1).GetString());
					}

					if (strCodeArray.GetSize() > 2)
					{
						m_strPatternName = strCodeArray.GetAt(2).GetString();
					}

					SetRunToThread(8004);
				}
				else if (sCmd.Find(_T("8006")) == 0)
				{

				}
				else if (sCmd.Find(_T("8007")) == 0)
				{

				}

				// Cal Make Cal File
				else if (sCmd.Find(_T("8011")) == 0)
				{
					// ack

				}
				else if (sCmd.Find(_T("8012")) == 0)
				{
					SetRunToThread(8013);
				}

				// 
				else if (sCmd.Find(_T("9002")) == 0)
				{

				}
				else if (sCmd.Find(_T("9011")) == 0)
				{

				}

				else if (sCmd.Find(_T("9044")) == 0)
				{
					CString strParam;
					CStringArray strCodeArray;
					strCodeArray.RemoveAll();

					int cnt = 0;
					while (AfxExtractSubString(strParam, sData, cnt, ','))  // 문자를 잘라준다. (AfxExtractSubString = strTok)
					{
						strCodeArray.Add(strParam);
						cnt++;
					}

					if (strCodeArray.GetSize() > 1)
					{
						m_RevCh = _ttoi(strCodeArray.GetAt(1).GetString());
					}

					if (strCodeArray.GetSize() > 2)
					{
						m_RevError = _ttoi(strCodeArray.GetAt(2).GetString());
					}

				//	SetRunToThread(9045);
				}

				else if (sCmd.Find(_T("9050")) == 0)
				{
					SetRunToThread(9051);
				}
			}
		}
	}
}

CLogFileAccessor* CCommTcpIpObjectColor::GetLogFileAccessorPtr()
{
	return m_pLogFileAccessor;
}

int CCommTcpIpObjectColor::WriteRecipeFile(void)
{
	TCHAR szValue[MAX_PATH];
	TCHAR	szGlassExistFile[MAX_PATH];
	ZeroMemory(szValue, MAX_PATH);

	CString str;
	str.Format(_T("%s%s"), m_pSystemConfigMgr->GetConfigFolderPath(), dfDEFAULT_CHANGERCP_FILE_PATH);

	_tcscpy_s(szGlassExistFile, _countof(szGlassExistFile), str);

	str.Format(_T("%s"), m_pRecipeEditMgr->GetRecipeID());

	BOOL bRet = FALSE;
	bRet = gf_WriteValue(_T("Current Recipe"), _T("Recipe Name"), str, szGlassExistFile);
	return bRet;
}

int CCommTcpIpObjectColor::CopyRecipeFile()
{
	// TODO: 여기에 구현 코드 추가.
	int nRet = VTM_RV_SUCCESSED;
	BOOL bRet = FALSE;
	WriteRecipeFile();

	CString strOrg, strNew;
	strOrg.Format(_T("%s%s"), m_pSystemConfigMgr->GetConfigFolderPath(), dfDEFAULT_CHANGERCP_FILE_PATH);

	//// 네트워크 드라이브 연결
	////nRet = NetUseDisConnection(m_Type, flase); //lhj temp 161203
	//nRet = NO_ERROR;
	//if (nRet == NO_ERROR || nRet == ERROR_NOT_CONNECTED)
	//{
	//	//nRet = NetUseConnection(m_Type, false); //lhj temp 161203
	//	if (nRet == NO_ERROR)		// 성공했을 시
	//	{
	//		strNew.Format(_T("%s\\%s"), , dfDEFAULT_CHANGERCP_FILE_PATH);	// jsyoon 20170619 change
	//		gf_MakeDir(strNew);
	//		bRet = CopyFile(strOrg, strNew, FALSE);
	//		if (bRet != TRUE)
	//			nRet = QUALITY_MODULE_CHANGE_RCP_ERROR;
	//		else
	//			nRet = VTM_RV_SUCCESSED;

	//		//NetUseDisConnection(m_Type, flase); //lhj temp 161203
	//	}
	//	else
	//	{
	//		return QUALITY_MODULE_CHANGE_RCP_ERROR;
	//	}
	//}
	//else
	//{
	//	return QUALITY_MODULE_DOWNLOAD_ERROR;
	//}

	return nRet;
}

UINT CCommTcpIpObjectColor::ThreadRun(LPVOID pParam)
{
	CCommTcpIpObjectColor* pProc = (CCommTcpIpObjectColor*)pParam;

	pProc->RunTurnOnToPG();

	return 0;
}

int CCommTcpIpObjectColor::SetRunToThread(int iPattern)
{
	// TODO: 여기에 구현 코드 추가.
	m_iPattern = iPattern;

	AfxBeginThread(ThreadRun, this);

	return 0;
}

void CCommTcpIpObjectColor::RunTurnOffToPG()
{
//	m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PGEvent_8);
}

void CCommTcpIpObjectColor::RunTurnOnToPG()
{
	long nRet = VTM_RV_SUCCESSED;

	switch (m_iPattern)
	{
	case 1002:
		SetCommand(1002);
		break;
	case 1004:
		SetCommand(1004);
		break;
	case 1006:
		SetCommand(1006);
		break;
	case 1008:
		SetCommand(1008);
		break;
	case 2001:
		SetCommand(2001);
		break;
	case 2003:
		SetCommand(2003);
		break;
	case 2006:		// Turn On
	{
		SetCommand(2006);

		if (m_nPatternOn == false)
		{
			if (GetCommTcpIpFactoryMgrPtr() != nullptr)
			{
				nRet = GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PGEvent_9 + (4 * m_iDeviceIndex));
			}

			m_nPatternOn = true;
		}
		else
		{
			if (GetCommTcpIpFactoryMgrPtr() != nullptr)
			{
				nRet = GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PGEvent_10 + (4 * m_iDeviceIndex));
			}
		}

		if (nRet == VTM_RV_SUCCESSED)
		{
			SetCommand(2007);
		}
	}
		break;
	case 2010:
	{
		// Turn OFF
		if (m_iDeviceIndex == 0)
		{
			if (GetCommTcpIpFactoryMgrPtr() != nullptr)
			{
				GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PGEvent_12 );
			}
		}
		else
		{
			if (GetCommTcpIpFactoryMgrPtr() != nullptr)
			{
				GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PGEvent_16 );
			}
		}

		m_nPatternOn = false;

		SetEvent(m_hRecvDEvent[enumRecvEventID_INSP_END_OK]);
		SetEvent(m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_OK]);
		
		SetCommand(2010);
	}
	break;
	
	case 3001:
	{
		SetCommand(3001);
	}
	break;
	case 3004:
	{
		SetCommand(3004);
	}
	break;

	case 8001:
	{
		SetCommand(8001);
	}
	break;
	case 8004:
	{
		SetCommand(2006);  // ptn ack  // 8004

		if (m_nPatternNum == 1)
		{
			if (GetCommTcpIpFactoryMgrPtr() != nullptr)
			{
				nRet = GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PGEvent_9 + (4 * m_iDeviceIndex));
			}
		}
		else
		{
			if (GetCommTcpIpFactoryMgrPtr() != nullptr)
			{
				nRet = GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PGEvent_10 + (4 * m_iDeviceIndex));
			}
		}

		if (nRet == VTM_RV_SUCCESSED)
		{
			SetCommand(2007);	// ptn On	// 8005
		}
	}
	break;
	case 8008:
	{
		// Turn OFF
		if (m_iDeviceIndex == 0)
		{
			if (GetCommTcpIpFactoryMgrPtr() != nullptr)
			{
				GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PGEvent_12 );
			}
		}
		else
		{
			if (GetCommTcpIpFactoryMgrPtr() != nullptr)
			{
				GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PGEvent_16 );
			}
		}

		SetCommand(8008);
	}
	break;
	case 8010:
	{
		SetCommand(8010);
	}
	break;
	case 8013:
	{
		SetCommand(8013);
	}
	break;
	case 9001:
	{
		SetCommand(9001);
	}
	break;
	case 9010:
	{
		SetCommand(9010);
	}
	break;
	case 9045:
	{
		SetCommand(9045);
	}
		break;
	case 9051:
	{
		SetCommand(9051);
	}
	break;
	default:
		break;
	}
}

int CCommTcpIpObjectColor::SetCommand(int iCmd, bool bWait)
{
	//	int iRet = VTM_RV_SUCCESSED;
	int iLen = 0;
	TBYTE nByte = 0x30, nCmd[2], /* nStx0 = 0x02, nStx1 = 0x08,*/  nStx0 = 0x02, nStx1 = 0x01, nEtx = 0x03; // ;
	TCHAR	szTmp[1000];
	TCHAR	szCmd[1000];
	TCHAR	szData[1000];
	szTmp[0] = 0x00;
	szCmd[0] = 0x00;
	szData[0] = 0x00;
	nCmd[0] = 0x30;

	int nEventOK = enumRecvEventID_SET_PARAMETER_OK, nEventNG = enumRecvEventID_SET_PARAMETER_NG;

	CString strMsg, strCmd, strCRC;
	strMsg.Empty();

	switch (iCmd)
	{
	case 1002:
		strCmd.Format(_T("1002"));
		strMsg.Format(_T("ACK,CONNECT"));
		break;
	case 1004:
		strCmd.Format(_T("1004"));
		strMsg.Format(_T("ACK,PROGRAM_VERSION"));
		break;
	case 1006:
		strCmd.Format(_T("1006"));
		strMsg.Format(_T("ACK,CONNECT_ALL_CAM"));
		break;
	case 1008:
		strCmd.Format(_T("1008"));
		strMsg.Format(_T("ACK,DEFECT_LIST"));
		break;

		//// Measure
	case 2001:
	{
		nEventOK = enumRecvEventID_RECIPE_CHANGE_OK;
		nEventNG = enumRecvEventID_RECIPE_CHANGE_NG;

		strCmd.Format(_T("2001"));
		strMsg.Format(_T("RECIPE_LOAD,%s"), m_pRecipeEditMgr->GetRecipeID());
	}
	break;
	case 2003:
	{
		ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_OK]);
		ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_NG]);

		ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_END_OK]);
		ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_END_NG]);

		nEventOK = enumRecvEventID_INSP_START_OK;
		nEventNG = enumRecvEventID_INSP_START_NG;

		int iUseMTP = 0;
		CString str, strCellID, strGlassID, strInnerID, strMcrID;
		int iMeasType = 0;
		iMeasType = m_iDeviceIndex;
		strInnerID.Format(_T("%s"), m_pShareMemory->GetSharedMemory()->stLotInfo.m_szInnerID[iMeasType]);
		if (strInnerID.IsEmpty())
		{
			strInnerID.Format(_T("%d%s"), 1 + iMeasType, gf_GetCurTimeString(3));
			_tcscpy_s(m_pShareMemory->GetSharedMemory()->stLotInfo.m_szInnerID[iMeasType], 32, strInnerID);
		}

		strMcrID.Format(_T("%s"), m_pShareMemory->GetSharedMemory()->stLotInfo.m_szCellID[iMeasType]);

		strCmd.Format(_T("2003"));
	//	strMsg.Format(_T("INSPECTION_START,%d,%d,%s,%s,%d"), m_iDeviceIndex + 1, 0, strMcrID, strInnerID, iUseMTP);
		strMsg.Format(_T("INSPECTION_START,%d,%d,%s,%s,%d"), 1, 0, strMcrID, strInnerID, iUseMTP);
	}
	break;
	case 2006:
	{
		strCmd.Format(_T("2006"));
		strMsg.Format(_T("ACK,PTN,%d"), m_nPatternNum);
	}
	break;
	case 2007:
	{
		strCmd.Format(_T("2007"));
		strMsg.Format(_T("PTN_ON,%d,%s,GOOD"), m_nPatternNum, m_strPatternName);
	}
	break;
	case 2010:
	{
		strCmd.Format(_T("2010"));
	//	strMsg.Format(_T("ACK,INSPECTION_END,%d"), m_iDeviceIndex + 1);
		strMsg.Format(_T("ACK,INSPECTION_END,%d"), 1);
	}
	break;
	////

	//// SI-MCA
	case 3001:		// SI-MCA Start
	{
		CString str, strCellID, strGlassID, strInnerID, strMcrID, strVectorFile;
		int iMeasType = 0;
		iMeasType = m_iDeviceIndex;
		strInnerID.Format(_T("%s"), m_pShareMemory->GetSharedMemory()->stLotInfo.m_szInnerID[iMeasType]);
		if (strInnerID.IsEmpty())
		{
			strInnerID.Format(_T("%d%s"), 1 + iMeasType, gf_GetCurTimeString(3));
			_tcscpy_s(m_pShareMemory->GetSharedMemory()->stLotInfo.m_szInnerID[iMeasType], 32, strInnerID);
		}

		strMcrID.Format(_T("%s"), m_pShareMemory->GetSharedMemory()->stLotInfo.m_szCellID[iMeasType]);

		strVectorFile.Format(_T("%s"), _T("ABCD"));

		strCmd.Format(_T("3001"));
		strMsg.Format(_T("SI_MCA_START,%d,%s,%s,%s"), m_iDeviceIndex + 1, strMcrID, strInnerID, _T("ABCD"));
		//		strMsg.Format(_T("SI_MCA_START,Ch,CellID,InnerID,VectorFile"));
	}
	break;
	case 3004:		// SI-MCA End
	{
		strCmd.Format(_T("3001"));
		strMsg.Format(_T("ACK,SI_MCA_FINISH,%d"), m_iDeviceIndex + 1);
	}
	break;

	case 8008:
	{
		strCmd.Format(_T("8008"));
		strMsg.Format(_T("ACK,INSPECTION_CAL_END,%d"), m_iDeviceIndex + 1);
	}
	break;

	case 8010:
	{
		CString str, strCellID, strGlassID, strInnerID, strMcrID, strVectorFile;
		int iMeasType = 0;
		iMeasType = m_iDeviceIndex;
		strInnerID.Format(_T("%s"), m_pShareMemory->GetSharedMemory()->stLotInfo.m_szInnerID[iMeasType]);
		if (strInnerID.IsEmpty())
		{
			strInnerID.Format(_T("%d%s"), 1 + iMeasType, gf_GetCurTimeString(3));
			_tcscpy_s(m_pShareMemory->GetSharedMemory()->stLotInfo.m_szInnerID[iMeasType], 32, strInnerID);
		}

		strMcrID.Format(_T("%s"), m_pShareMemory->GetSharedMemory()->stLotInfo.m_szCellID[iMeasType]);

		strCmd.Format(_T("8010"));
		strMsg.Format(_T("CAL_FILE_MAKE_START,%d,%d,%s,%s"), m_iDeviceIndex + 1, 1, strMcrID, strInnerID);
	}
	break;

	case 8013:
	{
		strCmd.Format(_T("8013"));
		strMsg.Format(_T("ACK,MAKE_CAL_FILE_FINISH,%d"), m_iDeviceIndex + 1);
	}
	break;

	case 9001:
	{
		strCmd.Format(_T("9001"));
		strMsg.Format(_T("ARE_YOU_THERE"));
	}
	break;
	case 9010:
	{
		strCmd.Format(_T("9010"));
		strMsg.Format(_T("SYSTEM_TIME,%s"), gf_GetCurTimeString(4));
	}
	break;
	case 9045:
	{
		strCmd.Format(_T("9045"));
		strMsg.Format(_T("ACK,SYSTEM_ERROR,%d"), m_iDeviceIndex + 1); // m_RevCh, ACK, SYSTEM_ERROR, 2"));
	}
	break;
	case 9051:
	{
		strCmd.Format(_T("9051"));
		strMsg.Format(_T("ACK,PROGRAM_CLOSE_USER")); // ACK, SYSTEM_ERROR, 2"));
	}
	break;

	//case 11:		// SI-MCA Start
	//{
	//	CString strInnerID, strCellID, strVector;
	//	strInnerID.Format(_T("%s"), m_pShareMemory->GetSharedMemory()->stLotInfo.m_szInnerID_SI[m_iDeviceIndex]);
	//	if (strInnerID.IsEmpty())
	//	{
	//		strInnerID.Format(_T("%d%s"), 1 + m_iDeviceIndex, gf_GetCurTimeString(3));
	//		_tcscpy_s(m_pShareMemory->GetSharedMemory()->stLotInfo.m_szInnerID_SI[m_iDeviceIndex], 32, strInnerID);
	//	}

	//	strCellID.Format(_T("%s"), m_pShareMemory->GetSharedMemory()->stLotInfo.m_szCellID[m_iDeviceIndex]);

	//	strVector.Format(_T("%s"), _T("Vector File"));

	//	// MOT.A1.POC.START.InnerID_SI.CellID.Vector File 이름
	//	strMsg.Format(_T("MOT.A%d.POC.START.%s.%s.%s"), m_iDeviceIndex + 1, strInnerID, strCellID, strVector);
	//}
	//break;
	//case 12:		// SI-MCA Finish
	//{
	//	CString strInnerID, strCellID, strVector;
	//	strInnerID.Format(_T("%s"), m_pShareMemory->GetSharedMemory()->stLotInfo.m_szInnerID_SI[m_iDeviceIndex]);
	//	if (strInnerID.IsEmpty())
	//	{
	//		strInnerID.Format(_T("%d%s"), 1 + m_iDeviceIndex, gf_GetCurTimeString(3));
	//		_tcscpy_s(m_pShareMemory->GetSharedMemory()->stLotInfo.m_szInnerID_SI[m_iDeviceIndex], 32, strInnerID);
	//	}

	//	strCellID.Format(_T("%s"), m_pShareMemory->GetSharedMemory()->stLotInfo.m_szCellID[m_iDeviceIndex]);

	//	strVector.Format(_T("%s"), _T("Vector File"));

	//	// MOT.A1.POC.FINISH.OK.InnerID_SI.CellID.Vector File 이름
	//	strMsg.Format(_T("MOT.A%d.POC.FINISH.OK.%s.%s.%s"), m_iDeviceIndex + 1, strInnerID, strCellID, strVector);
	//}
	//break;

	//case 13:		// 후활상 Start
	//{
	//	ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_OK]);
	//	ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_NG]);

	//	ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_END_OK]);
	//	ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_END_NG]);

	//	nEventOK = enumRecvEventID_INSP_START_OK;
	//	nEventNG = enumRecvEventID_INSP_START_NG;

	//	CString strInnerID, strCellID;
	//	strInnerID.Format(_T("%s"), m_pShareMemory->GetSharedMemory()->stLotInfo.m_szInnerID_SI[m_iDeviceIndex]);
	//	if (strInnerID.IsEmpty())
	//	{
	//		strInnerID.Format(_T("%d%s"), 1 + m_iDeviceIndex, gf_GetCurTimeString(3));
	//		_tcscpy_s(m_pShareMemory->GetSharedMemory()->stLotInfo.m_szInnerID_SI[m_iDeviceIndex], 32, strInnerID);
	//	}

	//	strCellID.Format(_T("%s"), m_pShareMemory->GetSharedMemory()->stLotInfo.m_szCellID[m_iDeviceIndex]);

	//	// MOT.A1.GRAB.START.InnerID_SI.CellID 
	//	strMsg.Format(_T("MOT.A%d.GRAB.START.%s.%s"), m_iDeviceIndex + 1, strInnerID, strCellID);
	//}
	//break;
	//case 14:
	//{
	//	CString strProductID;
	//	strProductID.Format(_T("%s"), _T("ProductID"));
	//	// MOT.A1.JOBFILEVALIDATION.ProductID
	//	strMsg.Format(_T("MOT.A%d.JOBFILEVALIDATION.%s"), m_iDeviceIndex + 1, strProductID);
	//}
	//break;
	default:
		break;
	}

	// Code
	_stprintf_s(szTmp, _countof(szTmp), _T("%s"), strCmd);	_tcscat_s(szData, _countof(szData), szTmp);

	int len = 0;
	len = strMsg.GetLength();

	// Len
	_stprintf_s(szTmp, _countof(szTmp), _T("%04X"), len);	_tcscat_s(szData, _countof(szData), szTmp);

	// Data
	_stprintf_s(szTmp, _countof(szTmp), _T("%s"), strMsg);	_tcscat_s(szData, _countof(szData), szTmp);

	unsigned short crc16;
	iLen = _tcsclen(szData);
	crc16 = crc_xmodem((unsigned char*)szData, iLen);

	// STX
	_stprintf_s(szCmd, _countof(szCmd), _T("%c"), nStx0);
	_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nStx1);	_tcscat_s(szCmd, _countof(szCmd), szTmp);

	// Daat
	_stprintf_s(szTmp, _countof(szTmp), _T("%s"), szData);	_tcscat_s(szCmd, _countof(szCmd), szTmp);

	// CRC
	_stprintf_s(szTmp, _countof(szTmp), _T("%04X"), crc16);	_tcscat_s(szCmd, _countof(szCmd), szTmp);

	_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nEtx);	_tcscat_s(szCmd, _countof(szCmd), szTmp);

	ResetEvent(m_hRecvDEvent[nEventOK]);
	ResetEvent(m_hRecvDEvent[nEventNG]);

	iLen = _tcsclen(szCmd);
	szCmd[iLen] = 0;

	CString strReturnCommand;
	strReturnCommand.Format(_T("%s"), szCmd);

	CString strLog;
	if (IsOutOfService() == true)
	{
		strLog.Format(_T("[%s] : %s, -> %s, %s, OutOfService"), GetName(), strReturnCommand, strCmd, strMsg);
		m_pLogFileAccessor->KMLOG(SEND, strLog); 

		return VTM_RV_SUCCESSED;
	}
		
	if (SendPacket(strReturnCommand) == true)
	{
		strLog.Format(_T("[%s] : %s, -> %s, %s"), GetName(), strReturnCommand, strCmd, strMsg);
		m_pLogFileAccessor->KMLOG(SEND, strLog);

		if (bWait == true)
		{
			HANDLE hRecvEvent[2] = { m_hRecvDEvent[nEventOK], m_hRecvDEvent[nEventNG] };

			DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, DEFINE_RESPONSE_WAIT_TIME);
			if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
			{
				strLog.Format(_T("[%s] : %s, %s"), GetName(), _T("SetCommand, HEAD_ACK_TIMEOUT"), strReturnCommand);
				m_pLogFileAccessor->KMLOG(WARN, strLog);
				CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iSeqItemIndex);
				return HEAD_ACK_TIMEOUT;
			}
			// jsyoon 2022.01.17 add
			else if (dw == (WAIT_OBJECT_0 + 1)) // NG
			{
				strLog.Format(_T("[%s] : %s, %s"), GetName(), _T("SetCommand, HEAD_ACK_FAIL"), strReturnCommand);
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

BOOL CCommTcpIpObjectColor::TossResultFile(CString strInnerID, int nArea)
{
	//RawData 복사 진행
	CString strNewPath, strNewFile, strRawFile;
	CString strIDFile;
	CString strTemp;

	strIDFile.Format(_T("%s.ini"), strInnerID);

	// 복사할 경로 위치 확인 필요
	strNewPath.Format(_T("%sRawData\\%s\\%s\\%s\\"), CSystemParamShare::GetInstance()->GetResultFolderPath(), gf_GetCurDayString(), CSystemParamShare::GetInstance()->GetGlassID(), _T("COLOR"));
	strNewFile.Format(_T("%s%s"), strNewPath, strIDFile);
	//	strNewFile.Format(_T("%sRawData\\%s\\%s\\%s"), CSystemParamShare::GetInstance()->GetResultFolderPath(), gf_GetCurDayString(), CSystemParamShare::GetInstance()->GetGlassID(), strIDFile);
	strRawFile.Format(_T("%s\\%s"), m_strNetDrive, strIDFile);

	//화질 제외 결과파일 생성될때까지 기다리기
	strTemp.Format(_T("%s\\%s"), m_strNetDrive, strIDFile);
	//CmFileFindTimeOut(strTemp, 5000);

	m_pCommTcpIpFactoryMgr->CmFileFindTimeOut(strTemp, DEFINE_PARAMETERS_WAIT_TIME);

	//폴더 생성, 파일 복사
	gf_MakeDir(strNewFile);
	if (CopyFile(strRawFile, strNewFile, FALSE))
	{
		//if (m_pJobProc->m_iUseDelData == 1)
//		DeleteFile(strRawFile);
	}

	//결과 파일 정보 전달
	RSRITEMDATA resultData;
	ZeroMemory(&resultData, sizeof(resultData));

	resultData.nMeasMode = PGM_MODE_COLOR; //확인 필요
	resultData.iMeasArea = nArea;

	_tcscpy(resultData.szLotID, CSystemParamShare::GetInstance()->GetLotID());
	_tcscpy(resultData.szGlassID, CSystemParamShare::GetInstance()->GetGlassID());
	_tcscpy(resultData.szRecipeID, CSystemParamShare::GetInstance()->GetRecipeID());
	//_tcscpy(resultData.szGlassID, CSystemParamShare::GetInstance()->GetGlassID());

	_tcscpy(resultData.szResultFile, strIDFile /*strInnerID*/);

	_tcscpy(resultData.szResultPath, strNewPath);

	CResultMgr::GetInstance()->Updata_Data(&resultData);

	return TRUE;
}