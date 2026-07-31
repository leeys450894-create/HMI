#include "pch.h"
#include "CCommTcpIpObjectSysTime.h"

#include "AuroraFuncDefine.h"
#include "CommEthFunction\EthernetManagerTCP.h"
#include "common\CommonFunc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCommTcpIpObjectSysTime::CCommTcpIpObjectSysTime(int iSeqItemIndex, stAuroraMgrPool& MgrPool)
	: m_pSystemConfigMgr(MgrPool.pSystemConfigMgr)
	, m_pShareMemory(MgrPool.pShareMemoryMgr)
	, m_pNetworkConfigMgr(MgrPool.pNetworkConfigMgr)
	, m_pLogFileAccessor(new CLogFileAccessor)
	, m_bOutOfService(false)
	, m_MgrPool_link(&MgrPool)
{

	CString strObjectName;
	strObjectName.Format(_T("%s"), _T("SYSTIME"));

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
}

CCommTcpIpObjectSysTime::~CCommTcpIpObjectSysTime()
{
	Finalize();

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
void CCommTcpIpObjectSysTime::xNetCallbackRecieveData(LPVOID ud)
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
		CString strIP(inet_ntoa(pRcvData->cliaddr.sin_addr));
		CString strMessage(pRcvData->szBody);
		CString strLog;
		strLog.Format(_T("[%s, %s, %d] : %s"), GetName(), strIP, ntohs(pRcvData->cliaddr.sin_port), strMessage);
		m_pLogFileAccessor->KMLOG(RECV, strLog);

		strMessage.Replace(_T(" "), _T(""));
		if (strMessage.IsEmpty() == true)
			return;
	}
}

CLogFileAccessor* CCommTcpIpObjectSysTime::GetLogFileAccessorPtr()
{
	return m_pLogFileAccessor;
}


int CCommTcpIpObjectSysTime::Cmd_Send(CString strSend)
{
	// TODO: 여기에 구현 코드 추가.
	int nRtn = VTM_RV_SUCCESSED;
	bool bRet = false;
	CString strLog;
	if (IsOutOfService() != TRUE)
	{
		if (strSend.Find(_T("$")) > 0)
		{
			bRet = SendPacket(strSend, false, false);
		}
		else
		{
			bRet = SendPacket(strSend, true, false);
		}

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

int CCommTcpIpObjectSysTime::Cmd_Send_SysTime(CString strTime)
{
	// TODO: 여기에 구현 코드 추가.
	int nRet = VTM_RV_SUCCESSED;
	nRet = Cmd_Send(strTime);

	return nRet;
}
