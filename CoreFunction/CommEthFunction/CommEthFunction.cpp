#include "pch.h"
#include "CommEthFunction.h"

#include "EthernetManagerTCP.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCommEthFunction::CCommEthFunction()
{
 //   m_pEthernet		= new CEthernetManagerTCP;
	m_pEthernet     = nullptr;
    m_pThread		= nullptr;
    m_bLive			= false;
    m_hCloseEvent	= CreateEvent(NULL, FALSE, FALSE, NULL);	// auto reset, initial reset
	m_hWaitsEvent	= CreateEvent(NULL, FALSE, FALSE, NULL);
	m_strName		= _T("");
    m_bRunMode		= false;
    m_strIP			= _T("192.168.103.254");
    m_nPort			= 7777;
	m_bConnected = false;
	m_bForcedDisconnect = true;
	m_bNetDelete = false;
}

CCommEthFunction::~CCommEthFunction(void)
{
	Finalize();

	if(m_hCloseEvent)
	{
		CloseHandle(m_hCloseEvent);
	}

	if(m_hWaitsEvent)
	{
		CloseHandle(m_hWaitsEvent);
	}
}

bool CCommEthFunction::NetStart()
{
	if (IsOutOfService())
		return true;

	if (m_bNetDelete == true)
		return false;

    if(m_pEthernet)
    {
		if(m_bNetDelete == true)
			return false;

		bool bRet = false;
		ULONGLONG dwStart = 0, dwEnd = 0;

		dwStart = GetTickCount();

		//m_strIP = _T("192.168.10.5");

		USES_CONVERSION;
		bRet = m_pEthernet->Start(T2A(m_strIP), (USHORT)m_nPort);

		if (bRet)
			m_bConnected = true;
		else
			m_bConnected = false;

		dwEnd = GetTickCount();
		TRACE(_T("NetStart(%d) IP:%s,Port:%d, %d,%d,%d\n"), bRet, m_strIP, m_nPort, dwEnd - dwStart, dwStart, dwEnd );

		return bRet;
    }

    return false;
}

void CCommEthFunction::NetStop()
{
    if(m_pEthernet)
    {
        m_pEthernet->Stop();
    }
}

void CCommEthFunction::NetDelete()
{
	if(m_pEthernet != nullptr )
	{
		m_bNetDelete = true;

		m_pEthernet->Finalize();

		delete m_pEthernet;
		m_pEthernet = nullptr;
	}
}

bool CCommEthFunction::SendPacket(void *pData, int iSize)
{
	CCriticalSectionSync single(&m_cs, TRUE);

    if(IsConnected() == false)
    {
        return false;
    }

    bool bRet = false;

	if(m_pEthernet)
    {
		USES_CONVERSION;
        bRet = m_pEthernet->SendTo(&pData, iSize);

        WaitForSingleObject(m_hWaitsEvent, 10);
    }

    return bRet;
}

bool CCommEthFunction::SendPacket(CString strMsg, bool bUseTail/* = false*/, bool bUseLog/* = false*/)
{
	CCriticalSectionSync single(&m_cs, TRUE);

    if(IsConnected() == false)
    {
		if (bUseLog) CLogWrapper::GetInstance()->KMLOG(SEND, strMsg);
        return false;
    }

    bool bRet = false;

	if(m_pEthernet)
    {
        // 통신 프로토콜 규약 - 2016 06 27 에 정의됨.
        // 명령어나 키워드의 끝에 $ 표시를 붙이고 통신한다.
		if(bUseTail) strMsg += _T("$");

		USES_CONVERSION;
        bRet = m_pEthernet->SendTo(T2A(strMsg), strMsg.GetLength());

		if (bUseLog)
		{
			CString strLog;
			strLog.Format(_T("[%s,%d]:%s"), m_strIP, m_nPort, strMsg);
			CLogWrapper::GetInstance()->KMLOG(SEND, strLog);
		}
		
        WaitForSingleObject(m_hWaitsEvent, 10);
    }

    return bRet;
}

CString CCommEthFunction::GetStringCMDID(int iCmdID)
{
    CString strCMDID;

    switch(iCmdID)
    {
        case COMM_UNKNOWN:				strCMDID = ENUM2STR(COMM_UNKNOWN);				break;
        case COMM_DISCONNECT:			strCMDID = ENUM2STR(COMM_DISCONNECT);			break;
        case COMM_CONNECT:				strCMDID = ENUM2STR(COMM_CONNECT);				break;
        case COMM_INIT_REQ:				strCMDID = ENUM2STR(COMM_INIT_REQ);				break;
        case COMM_INIT_RES:				strCMDID = ENUM2STR(COMM_INIT_RES);				break;
        case COMM_GRAB_START_REQ:		strCMDID = ENUM2STR(COMM_GRAB_START_REQ);		break;
        case COMM_GRAB_START_RES:		strCMDID = ENUM2STR(COMM_GRAB_START_RES);		break;
        case COMM_GRAB_END_REQ:			strCMDID = ENUM2STR(COMM_GRAB_END_REQ);			break;
        case COMM_GRAB_END_RES:			strCMDID = ENUM2STR(COMM_GRAB_END_RES);			break;
        case COMM_GRAB_STOP_REQ:		strCMDID = ENUM2STR(COMM_GRAB_STOP_REQ);		break;
        case COMM_GRAB_STOP_RES:		strCMDID = ENUM2STR(COMM_GRAB_STOP_RES);		break;
        case COMM_USER_DEFINE_REQ:		strCMDID = ENUM2STR(COMM_USER_DEFINE_REQ);		break;
        case COMM_USER_DEFINE_RES:		strCMDID = ENUM2STR(COMM_USER_DEFINE_RES);		break;
        default:						strCMDID = ENUM2STR(COMM_UNKNOWN);				break;
    }

    return strCMDID;
}

std::vector<CString> CCommEthFunction::GetCommand(CString &strData)
{
	CString strMessage = strData;
	
	std::vector<CString> vecResult;

	while(1)
	{
		if(strMessage.IsEmpty() == true)
			break;

		const int ipos = strMessage.Find('$');		
		if(ipos < 0)
			break;

		CString strParam = strMessage.Left(ipos + 1);
		if(strParam.IsEmpty() == true)
			break;

		vecResult.push_back(strParam);

		strMessage.Replace(strParam, _T(""));
	}

	strData = strMessage;

	return vecResult;
}

std::vector<CString> CCommEthFunction::GetCommand_SDC(CString& strData)
{
	CString strMessage = strData;

	std::vector<CString> vecResult;

	while (1)
	{
		if (strMessage.IsEmpty() == true)
			break;

		const int ipos = strMessage.Find(_T("FDB."));

		if (ipos < 0)
		{
			if (strMessage.Find(_T("ARE_YOU_")) >= 0 || strMessage.Find(_T("PROGRAM_VERSION")) >= 0 || strMessage.Find(_T("READY_FINISH")) >= 0 || strMessage.Find(_T("SYSTEM_ERROR")) >= 0)
			{
				vecResult.push_back(strMessage);
			}
			
			break;
		}
		else if (ipos == 0)
		{
			CString strCmd = strMessage;
			strCmd.Delete(0, 4);
			const int ipos2 = strCmd.Find(_T("FDB."));

			if (ipos2 < 0)
			{
				CString strParam = strMessage;

				vecResult.push_back(strParam);

				strMessage.Replace(strParam, _T(""));
			}
			else
			{
				CString strParam = strMessage.Mid(ipos, ipos2 + 4);
				vecResult.push_back(strParam);

				strMessage.Replace(strParam, _T(""));
			}
		}
		else
		{
			CString strParam = strMessage.Left(ipos + 1);
			if (strParam.IsEmpty() == true)
				break;

			vecResult.push_back(strParam);

			strMessage.Replace(strParam, _T(""));
		}
	}

	strData = strMessage;

	return vecResult;
}

long CCommEthFunction::ParseMsg(CString strMsg)
{
    strMsg.Replace(_T("$"), _T(""));
    strMsg.Replace(_T(" "), _T(""));

	if(strMsg.IsEmpty() == false)
	{
		CString strParam;
		AfxExtractSubString(strParam, strMsg, 0, '^');

		for(long n = 0; n < COMM_TYPE_COUNT; ++n)
		{
			CString strComm = GetStringCMDID(n);

			if(strParam.CompareNoCase(strComm) == 0)
			{
				return n;
			}
		}
	}

    return COMM_UNKNOWN;
}

// jsyoon 2022.01.21 add
CString CCommEthFunction::ExeHextoStr(CString strRcv)
{
	CString strFunc, strTemp;
	strFunc.Empty();

	int s = 1;

	for (int i = 0; i < strRcv.GetLength(); i++)
	{
		TCHAR c = (TCHAR)strRcv.GetAt(i);

		if (c == 0X02)	// STX
			continue;

		if (c == 0X03)	// ETX
			break;

		strTemp.Format(_T("%c"), c);
		strFunc += strTemp;
	}

	TRACE("%s\n", strFunc);

	return strFunc;
}

std::vector<CString> CCommEthFunction::GetExeHextoStr(CString& strData)
{
	CString strMessage = strData;

	std::vector<CString> vecResult;

	int iStart = 0, iEnd = 0, nLen = 0;

	while (1)
	{
		if (strMessage.IsEmpty() == true)
			break;

		nLen = strMessage.GetLength();
		for (int i = 0; i < nLen; i++)
		{
			TCHAR c = (TCHAR)strMessage.GetAt(i);

			if (c == 0X02)	// STX
			{
				iStart = i;
			}

			if (c == 0X03)	// ETX
			{
				iEnd = i;

				if (iStart < iEnd)
				{
					CString strParam = strMessage.Mid(iStart, iEnd + 1 - iStart);
					CString strPush = strParam.Mid(1, strParam.GetLength() - 2);
					vecResult.push_back(strPush);

					strMessage.Replace(strParam, _T(""));

					break;
				}
			}

			if (i + 1 == nLen)
			{
				strMessage.Empty();
				break;
			}
		}
	}

	return vecResult;
}

std::vector<CString> CCommEthFunction::GetExeHextoStr2(CString& strData)
{
	CString strMessage = strData;

	std::vector<CString> vecResult;

	int iStart = 0, iEnd = 0, nLen = 0;

	while (1)
	{
		if (strMessage.IsEmpty() == true)
			break;

		nLen = strMessage.GetLength();
		for (int i = 0; i < nLen; i++)
		{
			TCHAR c = (TCHAR)strMessage.GetAt(i);

			if (c == 0X04)	// STX
			{
				iStart = i;
			}

			if (c == 0X08)	// ETX
			{
				iEnd = i;

				if (iStart < iEnd)
				{
					CString strParam = strMessage.Mid(iStart, iEnd + 1 - iStart);
					CString strPush = strParam.Mid(1, strParam.GetLength() - 2);
					vecResult.push_back(strPush);

					strMessage.Replace(strParam, _T(""));

					break;
				}
			}

			if (i + 1 == nLen)
			{
				strMessage.Empty();
				break;
			}
		}
	}

	return vecResult;
}

std::vector<CString> CCommEthFunction::GetExeHextoStrForElpAging(CString& strData)
{
	CString strMessage = strData;

	std::vector<CString> vecResult;

	int iStart = 0, iEnd = 0, nLen = 0;

	while (1)
	{
		if (strMessage.IsEmpty() == true)
			break;

		nLen = strMessage.GetLength();
		for (int i = 0; i < nLen; i++)
		{
			TCHAR c = (TCHAR)strMessage.GetAt(i);


			if (c == '@' || c == '$') // STX
			{
				iStart = i;
			}

			if (c == 0X0D)	// ETX
			{
				iEnd = i;

				if (iStart < iEnd)
				{
					CString strParam = strMessage.Mid(iStart, iEnd + 1 - iStart);
					CString strPush = strParam.Mid(0, strParam.GetLength());
					vecResult.push_back(strPush);

					strMessage.Replace(strParam, _T(""));

					break;
				}
			}

			if (i + 1 == nLen)
			{
				strMessage.Empty();
				break;
			}
		}
	}

	return vecResult;
}


void CCommEthFunction::xNetCallbackRecieveData(LPVOID ud1, LPVOID ud2)
{
	CCommEthFunction *pThis = (CCommEthFunction *)ud1;
	if(pThis)
	{
		pThis->xNetCallbackRecieveData(ud2);
	}
}

void CCommEthFunction::xNetCallbackRecieveData(LPVOID ud)
{
}

UINT CCommEthFunction::ThreadCheckConnect(LPVOID pParam)
{
    CCommEthFunction *pThis = (CCommEthFunction *)pParam;

    if(pThis == nullptr)
        return 0;

    pThis->m_bLive = true;

	// Server / Client : 접속 시도를 2.5초 마다 다시 수행한다.
	const DWORD dwWaitTime = 1000;

	TRACE(_T("ThreadCheckConnect Start IP:%s,Port:%d\n"), pThis->m_strIP, pThis->m_nPort);

    while (1)
    {
		// WaitForSingleObject의 기능 검사 지정된 객체의 현재 상태. 
		// 객체의 상태가 신호가 아닌 경우 호출 스레드는 효율적인 대기 상태가됩니다. 
		// 스레드는 오브젝트 상태가 신호를 받거나 시간 종료 간격이 경과하기를 기다리는 동안 
		// 프로세서 시간을 거의 소비하지 않습니다.
		if(WaitForSingleObject(pThis->m_hCloseEvent, dwWaitTime) == WAIT_OBJECT_0)
		{
			// 객체를 삭제하고 thread 를 닫도록 하자.
			pThis->NetDelete();
			break;
		}
		else
		{
			if (pThis->m_bRunMode != true)	// client Mode
			{
				if (pThis->GetForcedDisconnect() == true)
				{
					if (pThis->IsConnected() == false)
					{
						pThis->NetStart(); // 2000 msec 걸림
					}
				}
			}
		}

		Sleep(1);
    }

	TRACE(_T("ThreadCheckConnect Close IP:%s,Port:%d\n"), pThis->m_strIP, pThis->m_nPort);

    pThis->m_bLive = false;

    return 0;
}




bool CCommEthFunction::Initialize(bool bDefault)
{
	if(m_pEthernet == nullptr)
	{
		m_pEthernet = new CEthernetManagerTCP;
	}

	if(m_pEthernet != nullptr)
	{
		if( bDefault == true)
			m_pEthernet->Initialize(m_bRunMode, this, xNetCallbackRecieveData);
		else
			m_pEthernet->Initialize(m_bRunMode, m_CallbackPtr, m_CallbackFunc);
	}

	//// jsyoon 2022.05.17 change
	//if (m_bRunMode != true)
	//{
	//	if (m_pThread == nullptr)
	//	{
	//		m_pThread = AfxBeginThread(ThreadCheckConnect, this);
	//	}
	//}
	//else
	//{
	//	NetStart();
	//}

	// 2023.03.07 change
	if (m_bRunMode == true)
	{
		NetStart();
	}
	
	if (m_pThread == nullptr)
	{
		m_pThread = AfxBeginThread(ThreadCheckConnect, this);
	}
    
	/*if (m_pThread == nullptr)
	{
		m_pThread = AfxBeginThread(ThreadCheckConnect, this);
	}*/

    return true;
}

// jsyoon 20211130 add
void CCommEthFunction::CloseConnect()
{
	// 먼저 객체를 삭제하고 thread 를 닫도록 하자.
	SetEvent(m_hCloseEvent);
}

void CCommEthFunction::Finalize()
{
	// 먼저 객체를 삭제하고 thread 를 닫도록 하자.
	SetEvent(m_hCloseEvent);

	if(m_pThread && m_bLive)
	{
		WaitForSingleObject(m_pThread->m_hThread, 500);
	}

	m_pThread = nullptr;
}

void CCommEthFunction::SetName(CString strName)
{
	m_strName = strName;
}

void CCommEthFunction::SetNetInfo(bool bRunMode, CString strIP, long nPort)
{
	m_bRunMode	=	bRunMode;
	m_strIP		=	strIP;
	m_nPort		= 	nPort;
}

bool CCommEthFunction::IsConnected() const
{
    if(m_pEthernet)
    {
		if (m_bConnected == true)
		{
			return m_pEthernet->IsConnected();
		}
		else
		{
			return false;
		}
    }

    return false;
}

bool CCommEthFunction::IsLive()
{
    return m_bLive;
}

void CCommEthFunction::SetConnect(bool bStart)
{
	if (bStart == true)
	{
		if(IsConnected() == false)
			NetStart();
	}
	else
	{
	//	if (IsConnected() == true)	// jsyoon 2022.05.17 remove
			NetStop();
	}
}

bool CCommEthFunction::IsOutOfService() const
{
	return false;
}

bool CCommEthFunction::SendCommand(CString strMsg, bool bUseTail, bool bUseLog)
{
	return SendPacket(strMsg, bUseTail, bUseLog);
}


bool CCommEthFunction::GetForcedDisconnect()
{
	return m_bForcedDisconnect;
}

void CCommEthFunction::SetForcedDisconnect(bool bFlag)
{
	m_bForcedDisconnect = bFlag;
}
