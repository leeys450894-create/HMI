#include "pch.h"
#include "CCommTcpIpObjectMotion.h"

#include "AuroraFuncDefine.h"
#include "CommEthFunction\EthernetManagerTCP.h"
#include "common\CommonFunc.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCommTcpIpObjectMotion::CCommTcpIpObjectMotion(int iSeqItemIndex, stAuroraMgrPool& MgrPool)
	: m_pSystemConfigMgr(MgrPool.pSystemConfigMgr)
	, m_pShareMemory(MgrPool.pShareMemoryMgr)
	, m_pNetworkConfigMgr(MgrPool.pNetworkConfigMgr)
	, m_pLogFileAccessor(new CLogFileAccessor)
	, m_bOutOfService(false)
	, m_MgrPool_link(&MgrPool)
{

	CString strObjectName;
	strObjectName.Format(_T("%s"), _T("VisMotion"));

	SetName(strObjectName); // _T("UTG"));
	const bool& bRunMode = m_pNetworkConfigMgr->GetParam_Mode(iSeqItemIndex);
	const CString& strIP = m_pNetworkConfigMgr->GetParam_IPNumber(iSeqItemIndex);
	const long& nPort = m_pNetworkConfigMgr->GetParam_PortNumger(iSeqItemIndex);
	SetNetInfo(bRunMode, strIP, nPort);

	m_bOutOfService = m_pNetworkConfigMgr->GetParam_OutOfService(iSeqItemIndex);

	// 로그 생성하기
	const CString strLogFilePath = MgrPool.strLogPath;
	const CString strFolderPath = strLogFilePath + _T("Control\\") + strObjectName + _T("\\");
	const CString strLogFile = strFolderPath + strObjectName + _T(".log");
	MakeDir(strFolderPath);
	m_pLogFileAccessor->Initialize(strLogFile);
	//////

	// 데이터 수신 확인용 event
	for (int i = 0; i < enumRecvEventIDCount; ++i)
	{
		m_hRecvDEvent[i] = CreateEvent(NULL, FALSE, FALSE, NULL);	// auto reset, initial reset
	}
}

CCommTcpIpObjectMotion::~CCommTcpIpObjectMotion()
{
	Finalize();

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

//=========================================================================
// INTERFACE 관련
//=========================================================================
void CCommTcpIpObjectMotion::xNetCallbackRecieveData(LPVOID ud)
{
	LPRECEIVEDDATA pRcvData = (LPRECEIVEDDATA)ud;

	if (pRcvData->eCommState == (eXNET_COMM_STATE)XNET_NOTCONNECT)
	{
		CString strIP(inet_ntoa(pRcvData->cliaddr.sin_addr));

		CString strLog;
		strLog.Format(_T("[%s, %d] : %s"), strIP, ntohs(pRcvData->cliaddr.sin_port), _T("Close Connected"));
		m_pLogFileAccessor->KMLOG(SYST, strLog);
	}
	else if (pRcvData->eCommState == (eXNET_COMM_STATE)XNET_CONNECTED)
	{
		CString strIP(inet_ntoa(pRcvData->cliaddr.sin_addr));

		CString strLog;
		strLog.Format(_T("[%s, %d] : %s"), strIP, ntohs(pRcvData->cliaddr.sin_port), _T("Connected"));
		m_pLogFileAccessor->KMLOG(SYST, strLog);
	}
	else if (pRcvData->eCommState == (eXNET_COMM_STATE)XNET_SERVER_ACCEPT)
	{
		CString strIP(inet_ntoa(pRcvData->cliaddr.sin_addr));
		CString strLog;
		strLog.Format(_T("[%s, %d] : %s"), strIP, ntohs(pRcvData->cliaddr.sin_port), _T("ACCEPT"));
		m_pLogFileAccessor->KMLOG(SYST, strLog);
	}
	else if (pRcvData->eCommState == (eXNET_COMM_STATE)XNET_RECEIVEDATA)
	{
		CString strMessage, strTemp;

		char* pchar = pRcvData->szBody;
		int nLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pchar, -1, NULL, NULL);

		wchar_t szWide[100];
		memset(szWide, 0, sizeof(wchar_t) * 100);

		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pchar, pRcvData->sizeBody + 1, szWide, pRcvData->sizeBody);

		for (int i = 0; i < pRcvData->sizeBody; i++)
		{
			if (szWide[i] == 0X00)
			{
				szWide[i] = '0';
			}
		}

		strMessage.Format(_T("%s"), szWide);

		CString strIP(inet_ntoa(pRcvData->cliaddr.sin_addr));
		CString strLog;
		strLog.Format(_T("[%s, %s, %d] : %s"), GetName(), strIP, ntohs(pRcvData->cliaddr.sin_port), strMessage);
		m_pLogFileAccessor->KMLOG(RECV, strLog);

		strMessage.Replace(_T(" "), _T(""));
		if (strMessage.IsEmpty() == true)
			return;

		std::vector<CString> vecPacketCommandPool = GetExeHextoStr(strMessage);

		for (size_t i = 0; i < vecPacketCommandPool.size(); ++i)
		{
			const CString strCommand = vecPacketCommandPool[i];

			strLog.Format(_T("[%s] : %s"), GetName(), strCommand);
			m_pLogFileAccessor->KMLOG(OPER, strLog);

			if (strCommand.Find(_T(".NG")) >= 0)	// NG
			{
				SetEvent(m_hRecvDEvent[enumRecvEventID_RECIPE_CHANGE_NG]);
				SetEvent(m_hRecvDEvent[enumRecvEventID_SET_PARAMETER_NG]);
				SetEvent(m_hRecvDEvent[enumRecvEventID_INSP_START_NG]);
				SetEvent(m_hRecvDEvent[enumRecvEventID_INSP_END_NG]);
				SetEvent(m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_NG]);
			}
			else if (strCommand.Find(_T(".OK")) >= 0)	// OK	// jsyoon 2022.10.20 change OK
		//	else
			{
				if (strCommand.Find(_T("CHANGE.")) >= 0)	// 레시피 변경 확인
				{
					SetEvent(m_hRecvDEvent[enumRecvEventID_RECIPE_CHANGE_OK]);
				}
				else if (strCommand.Find(_T("HOMING")) >= 0)
				{
					SetEvent(m_hRecvDEvent[enumRecvEventID_SET_PARAMETER_OK]);
				}
				else if (strCommand.Find(_T("FILTER.")) >= 0)
				{
					int iActiveID = 0;
					if (strCommand.Find(_T(".A1.")) >= 0)
					{
						iActiveID = 0;
					}
					else
					{
						iActiveID = 1;
					}

					if (iActiveID == 0)
					{
						SetEvent(m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_OK]);
					}
					else
					{
						SetEvent(m_hRecvDEvent[enumRecvEventID_INSP2_COMPLETE_OK]);
					}
					
				}
				else if (strCommand.Find(_T("ZOOM.")) >= 0)
				{
					int iActiveID = 0;

					if (strCommand.Find(_T(".A1.")) >= 0)
					{
						iActiveID = 0;
					}
					else
					{
						iActiveID = 1;
					}

					if (iActiveID == 0)
					{
						SetEvent(m_hRecvDEvent[enumRecvEventID_INSP_END_OK]);
					}
					else
					{
						SetEvent(m_hRecvDEvent[enumRecvEventID_INSP2_END_OK]);
					}					
				}
			}
		}

	}
}

CLogFileAccessor* CCommTcpIpObjectMotion::GetLogFileAccessorPtr()
{
	return m_pLogFileAccessor;
}

int CCommTcpIpObjectMotion::Cmd_Send(CString strSend)
{
	// TODO: 여기에 구현 코드 추가.
	int nRtn = VTM_RV_SUCCESSED;
	bool bRet = false;
	CString strLog;
	if (IsOutOfService() != TRUE)
	{
		bRet = SendPacket(strSend, false, false);

		strLog.Format(_T("[%s](%d): %s"), GetName(), bRet, strSend);
	}
	else
	{
		bRet = true;
		strLog.Format(_T("[%s](%d): %s, Skip"), GetName(), bRet, strSend);
	}

	m_pLogFileAccessor->KMLOG(SEND, strLog);

	if (bRet == true)
	{
		nRtn = VTM_RV_SUCCESSED;
	}
	else
	{
		strLog.Format(_T("[%s] : %s"), GetName(), _T("NOT_CONNECTED"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		nRtn = HEAD_NOT_CONNECTED;
	}

	return nRtn;
}

int CCommTcpIpObjectMotion::SetCommand(VisMotionPROTOCOL stPrl, bool bWait)
{
	int nRtn = VTM_RV_SUCCESSED;

	int iLen = 0;
	TBYTE nByte = 0x30, nCmd[2], nStx = 0x02, nEtx = 0x03;
	TCHAR	szTmp[1000];
	TCHAR	szCmd[1000];
	TCHAR	szData[1000];
	szTmp[0] = 0x00;
	szCmd[0] = 0x00;
	szData[0] = 0x00;
	nCmd[0] = 0x30;


	int nEventOK = enumRecvEventID_SET_PARAMETER_OK, nEventNG = enumRecvEventID_SET_PARAMETER_NG;

	CString strFlag, strMsg, strCmd;
	strMsg.Empty();
	strCmd.Empty();

	strFlag.Format(_T("HToA."));

	switch (stPrl.iCmd)
	{
	case enumTCPIpFactoryt_VisMotion_0:
	{
		strCmd.Format(_T("CHANGE."));
		strMsg.Format(_T("%s"), stPrl.szModel);

		nEventOK = enumRecvEventID_RECIPE_CHANGE_OK;
		nEventNG = enumRecvEventID_RECIPE_CHANGE_NG;
	}
	break;
	case enumTCPIpFactoryt_VisMotion_1:
	{
		strCmd.Format(_T("HOMING"));
		strMsg.Format(_T(""));

		nEventOK = enumRecvEventID_SET_PARAMETER_OK;
		nEventNG = enumRecvEventID_SET_PARAMETER_NG;
	}
	break;
	case enumTCPIpFactoryt_VisMotion_2:
	{
		strCmd.Format(_T("FILTER."));
		strMsg.Format(_T("A%d.%02d"), stPrl.iActiveID + 1, stPrl.iFilter + 1);

		if (stPrl.iActiveID == 0)
		{
			nEventOK = enumRecvEventID_INSP_COMPLETE_OK;
			nEventNG = enumRecvEventID_INSP_COMPLETE_NG;
		}
		else
		{
			nEventOK = enumRecvEventID_INSP2_COMPLETE_OK;
			nEventNG = enumRecvEventID_INSP2_COMPLETE_NG;
		}
	}
	break;
	case enumTCPIpFactoryt_VisMotion_3:
	{
		strCmd.Format(_T("ZOOM."));
		strMsg.Format(_T("%02d.%.4f"), stPrl.iActiveID + 1, stPrl.dMotion);

		if (stPrl.iActiveID == 0)
		{
			nEventOK = enumRecvEventID_INSP_END_OK;
			nEventNG = enumRecvEventID_INSP_END_NG;
		}
		else
		{
			nEventOK = enumRecvEventID_INSP2_END_OK;
			nEventNG = enumRecvEventID_INSP2_END_NG;
		}
	}
	break;
	default:
		break;
	}

	if (strCmd.IsEmpty())
	{
		return HEAD_COMMAND_ERROR;
	}

	// STX
	_stprintf_s(szCmd, _countof(szCmd), _T("%c"), nStx);

	// Daat
	_stprintf_s(szTmp, _countof(szTmp), _T("%s"), strFlag);	_tcscat_s(szData, _countof(szData), szTmp);
	_stprintf_s(szTmp, _countof(szTmp), _T("%s"), strCmd);	_tcscat_s(szData, _countof(szData), szTmp);
	_stprintf_s(szTmp, _countof(szTmp), _T("%s"), strMsg);	_tcscat_s(szData, _countof(szData), szTmp);
	_stprintf_s(szTmp, _countof(szTmp), _T("%s"), szData);	_tcscat_s(szCmd, _countof(szCmd), szTmp);

	// EXT
	_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nEtx); _tcscat_s(szCmd, _countof(szCmd), szTmp);	// EXT

	iLen = _tcsclen(szCmd);
	szCmd[iLen] = 0;

	CString strReturnCommand, strLog;
	strReturnCommand.Format(_T("%s"), szCmd);

	ResetEvent(m_hRecvDEvent[nEventOK]);
	ResetEvent(m_hRecvDEvent[nEventNG]);

	nRtn = Cmd_Send(strReturnCommand);

	if (IsOutOfService() == TRUE) // 2023.08.09 add
	{
		return nRtn;
	}

	if (nRtn == VTM_RV_SUCCESSED)
	{	
		if (bWait == true)
		{
			HANDLE hRecvEvent[2] = { m_hRecvDEvent[nEventOK], m_hRecvDEvent[nEventNG] };

			DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, DEFINE_RESPONSE_WAIT_TIME * 10);
			if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
			{
				strLog.Format(_T("[%s] : SetCommand, %s"), GetName(), _T("HEAD_ACK_TIMEOUT"));
				m_pLogFileAccessor->KMLOG(WARN, strLog);
				nRtn = HEAD_ACK_TIMEOUT;
			}
			// jsyoon 2022.01.17 add
			else if (dw == (WAIT_OBJECT_0 + 1)) // NG
			{
				strLog.Format(_T("[%s] : SetCommand, %s"), GetName(), _T("HEAD_ACK_FAIL"));
				m_pLogFileAccessor->KMLOG(WARN, strLog);
				nRtn = HEAD_ACK_FAIL;
			}
		}
	}

	return nRtn;
}

int CCommTcpIpObjectMotion::WaitInspectionEnd(int iActiveID, DWORD dwTimeOut)
{
	if (IsOutOfService() == true)
		return VTM_RV_SUCCESSED;

	HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_INSP_END_OK], m_hRecvDEvent[enumRecvEventID_INSP_END_NG] };

	CString strLog;
	DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, dwTimeOut * 1000);
	if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
	{
		strLog.Format(_T("[%s] : WaitInspectionEnd, %s"), GetName(), _T("HEAD_ACK_TIMEOUT"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		return HEAD_ACK_TIMEOUT;
	}
	// jsyoon 2022.01.17 add
	else if (dw == (WAIT_OBJECT_0 + 1)) // NG
	{
		strLog.Format(_T("[%s] : WaitInspectionEnd, %s"), GetName(), _T("HEAD_ACK_FAIL"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		return HEAD_ACK_FAIL;
	}

	return VTM_RV_SUCCESSED;
}

int CCommTcpIpObjectMotion::WaitInspectionComplete(int iActiveID, DWORD dwTimeOut)
{
	if (IsOutOfService() == true)
		return VTM_RV_SUCCESSED;

	HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_OK], m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_NG] };

	CString strLog;
	DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, dwTimeOut * 1000);
	if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
	{
		strLog.Format(_T("[%s] : WaitInspectionComplete, %s"), GetName(), _T("HEAD_ACK_TIMEOUT"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		return HEAD_ACK_TIMEOUT;
	}
	// jsyoon 2022.01.17 add
	else if (dw == (WAIT_OBJECT_0 + 1)) // NG
	{
		strLog.Format(_T("[%s] : WaitInspectionComplete, %s"), GetName(), _T("HEAD_ACK_FAIL"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		return HEAD_ACK_FAIL;
	}

	return VTM_RV_SUCCESSED;
}