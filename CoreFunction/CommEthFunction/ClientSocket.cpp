#include "pch.h"
#include "ClientSocket.h"

#pragma comment(lib, "ws2_32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CClientSocket::CClientSocket()
{
	// 데이터를 전송하고 받기 위해서 사용하는 thread
	m_pThread			= AfxBeginThread(ThreadRun, this);
    m_bThreadLive		= false;
	m_hStartEvent		= CreateEvent(NULL, TRUE, TRUE, NULL);		// manual set, initial set
    m_hCloseEvent		= CreateEvent(NULL, FALSE, FALSE, NULL);	// auto reset, initial reset
	m_bIsCreated		= false;
    m_bConnected		= false;

	// 통신 송/수신 관련 이벤드
	m_hMesgSocketEvent	= WSACreateEvent();
   	m_client_socket		= INVALID_SOCKET;
	m_pszSendData		= new char[PACKET_BODY_SIZE];
	m_pszRecvData		= new char[PACKET_BODY_SIZE];
	m_pRecvPacket		= new TRECEIVEDDATA;

    m_CallbackPtr		= nullptr;
    m_CallbackFunc		= nullptr;
}

CClientSocket::~CClientSocket()
{
	//-------------------------------------------
	// Thread 관련 제거
	//-------------------------------------------
	ResetEvent(m_hStartEvent);
	SetEvent(m_hCloseEvent);
	if(m_bThreadLive && m_pThread)
	{
		WaitForSingleObject(m_pThread->m_hThread, INFINITE);
	}
	CloseHandle(m_hStartEvent);
	CloseHandle(m_hCloseEvent);
	
	//-------------------------------------------
	// 통신 송/수신 관련 이벤드
	//-------------------------------------------
	WSACloseEvent(m_hMesgSocketEvent);

	//-------------------------------------------
	// 데이터 관련 제거
	//-------------------------------------------
	if(m_pszSendData)
	{
		delete [] m_pszSendData;
		m_pszSendData = nullptr;
	}

	if(m_pszRecvData)
	{
		delete [] m_pszRecvData;
		m_pszRecvData = nullptr;
	}

	if(m_pRecvPacket)
	{
		delete m_pRecvPacket;
		m_pRecvPacket = nullptr;
	}

	// Winsock 2 DLL의 사용을 종료합니다.
	SocketCleanUp();
}

bool CClientSocket::CreateSocket(char *szIPNo, USHORT usPortNo)
{
	//WSAStartup함수는 프로세스에서 Winsock DLL의 사용을 초기화한다.
    WSADATA wsaData;
	if (WSAStartup(0x202, &wsaData) == SOCKET_ERROR)
    {
        return SocketCleanUp();
    }
	
	// 소켓을 생성한다.
	m_client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_client_socket == INVALID_SOCKET)
	{
		return SocketCleanUp();
	}

	// 소켓 정보 - IP, port 정보를 기입한다.
	if (FillAddress(usPortNo, szIPNo) == false)
	{
		return SocketCleanUp();
	}

	//// 소켓을 생성한다.
	//m_client_socket = socket(AF_INET, SOCK_STREAM, 0);
 //   if (m_client_socket == INVALID_SOCKET)
 //   {
 //       return SocketCleanUp();
 //   }

	//// 소켓 정보 - IP, port 정보를 기입한다.
	//if (FillAddress(usPortNo, szIPNo) == false)
	//{
	//	return SocketCleanUp();
	//}
	

	// 접속을 시도 한다.
	if(Connect() == false)
	{
		return SocketCleanUp();
	}
			
	m_bIsCreated = true;

	NotifyReceive(XNET_CONNECTED, 0, 0);

    return true;
}

bool CClientSocket::SendMesgData(LPVOID lpData, int nSize)
{
    if (m_client_socket == INVALID_SOCKET)
        return false;

    ////////////////////////////////////////////////////////////////////////////////
    // Check send a packet size
    ////////////////////////////////////////////////////////////////////////////////
	nSize = min(nSize, sizeof(char) * PACKET_BODY_SIZE);

	ZeroMemory(m_pszSendData, sizeof(char) * PACKET_BODY_SIZE);
    memcpy(m_pszSendData, (char *)lpData, nSize);

	// jsyoon 2022.05.19 change
	for (size_t i = 0; i < nSize; i++)
	{
		if (m_pszSendData[i] == 0x20)
		{
			m_pszSendData[i] = 0x00;
		}
	}

    ////////////////////////////////////////////////////////////////////////////////
    // To send packet
    ////////////////////////////////////////////////////////////////////////////////
    int	nTotalBytes	= nSize;
    int	nBytesSent = 0, nErrorCount = 0, nSendLoopCount = 0;
    
	while (nBytesSent < nTotalBytes)
    {
        int nThisSendSize = send(m_client_socket, m_pszSendData + nBytesSent, nTotalBytes - nBytesSent, 0);

        if (nThisSendSize < 0)
            break;

        if (SOCKET_ERROR == nThisSendSize)
            nErrorCount++;
        else
            nSendLoopCount++;

        nBytesSent += nThisSendSize;

		if( (m_client_socket == INVALID_SOCKET) || (nErrorCount >= 50) || (nSendLoopCount >= 100) )
        {
            return false;
        }

        nErrorCount = 0;
        nSendLoopCount = 0;

        Sleep(1);
    }

	return true;
}

bool CClientSocket::CloseSocket()
{
	return SocketCleanUp();
}

bool CClientSocket::IsCreated() const
{ 
	return m_bIsCreated;	
}

bool CClientSocket::IsThreadLive() const
{ 
	return m_bThreadLive;		
}

bool CClientSocket::GetConnected() const
{ 
	return m_bConnected;	
}








//--------------------------------------------------------------------
// 내부 함수들
//--------------------------------------------------------------------
bool CClientSocket::FillAddress(USHORT usPortNo, char *szServerName)
{
    memset(&m_sin, 0, sizeof(SOCKADDR_IN));

    m_sin.sin_family = AF_INET;
    m_sin.sin_port = htons(usPortNo);

    // In case of domain name
    if (isalpha(szServerName[0]))
    {
		PHOSTENT phe = gethostbyname(szServerName);
	
        if (phe == NULL)
        {
            return false;
        }

        memcpy((char *)&m_sin.sin_addr, phe->h_addr, phe->h_length);
    }
    else
    {
        m_sin.sin_addr.s_addr = inet_addr( szServerName);
    }

    return true;
}

// return 값은 socket 정보를 삭제하였기 때문에 무조건 false 로 리턴시켜서
// MAIN-HOST 에서 재시도 하도록 유도한다.
bool CClientSocket::SocketCleanUp()
{
	m_bConnected = false;
	m_bIsCreated = false;

    if (m_client_socket != INVALID_SOCKET)
	{
		shutdown(m_client_socket, SD_BOTH); 
		closesocket(m_client_socket);
		m_client_socket = INVALID_SOCKET;
	}

	// Winsock 2 DLL(Ws2_32.dll)의 사용을 종료합니다
    WSACleanup();

	return false;
}

bool CClientSocket::Connect()
{
    if (m_client_socket == INVALID_SOCKET)
        return false;

	int res = connect(m_client_socket, (struct sockaddr*)&m_sin, sizeof sockaddr);
	if (res == SOCKET_ERROR)
    {
		return false;
    }
	
    WSAEventSelect(m_client_socket, m_hMesgSocketEvent, FD_READ | FD_CLOSE);

	m_bConnected = true;

	return true;
}

bool CClientSocket::RecvData()
{
    if (m_client_socket == INVALID_SOCKET)
        return false;

    ////////////////////////////////////////////////////////////////////////////////
    // Make recieve a packet
    ////////////////////////////////////////////////////////////////////////////////
    ZeroMemory(m_pszRecvData, sizeof(char) * PACKET_BODY_SIZE);

    int nTotalBytes = PACKET_BODY_SIZE;
    
	int nBytesRecv = recv(m_client_socket, m_pszRecvData, nTotalBytes, 0);

	if ( 0 < nBytesRecv )
	{
	    int nErrorCount = 0, nRecvLoopCount = 0;

		while (nBytesRecv < nTotalBytes)
		{
			int nThisRecvsize = recv(m_client_socket, m_pszRecvData + nBytesRecv, nTotalBytes - nBytesRecv, 0);

			if (nThisRecvsize < 0)
			{
				NotifyReceive(XNET_RECEIVEDATA, m_pszRecvData, nBytesRecv);
				break;
			}

			if (SOCKET_ERROR == nThisRecvsize)
				nErrorCount++;
			else
				nRecvLoopCount++;

			nBytesRecv += nThisRecvsize;

			if( (m_client_socket == INVALID_SOCKET) || (nErrorCount >= 500) || (nRecvLoopCount >= 1000) )
			{
				// 잘못된 패킷 수신으로 재전송 요구
				return false;
			}

			nErrorCount = 0;

			Sleep(1);
		}

        //CString strMsg;
        //strMsg.Format(_T("Bytes received : %d"), nBytesRecv);
		//printf("Bytes received: %d\n", nBytesRecv);
	}
    else if ( 0 == nBytesRecv )
	{
        //CString strMsg;
        //strMsg.Format(_T("Connection closed"));
        //printf("Connection closed\n");
	}
    else
	{
        //CString strMsg;
        //strMsg.Format(_T("recv failed - last error code : %d"), WSAGetLastError());
        //printf("recv failed: %d\n", WSAGetLastError());
	}

    return true;
}

void CClientSocket::NotifyReceive(eXNET_COMM_STATE eCommState, LPVOID pData, size_t nSize)
{
	// jsyoon 2022.03.08 add
	if (eCommState == XNET_NOTCONNECT || eCommState == XNET_CONNECTED)
	{
		m_pRecvPacket->Clear();

		m_pRecvPacket->iClientID = 0;
		m_pRecvPacket->eCommState = eCommState;
		m_pRecvPacket->sizeBody = nSize;

		// jsyoon 2022.08.25 add
		SOCKADDR_IN cliaddr = { 0 };
		int len = sizeof(cliaddr);
		getpeername(m_client_socket, (SOCKADDR*)&cliaddr, &len);
		m_pRecvPacket->cliaddr = cliaddr;

		if (m_CallbackFunc)
		{
			m_CallbackFunc(m_CallbackPtr, (LPVOID)m_pRecvPacket);
		}
	}

    ////////////////////////////////////////////////////////////////////////////////
    // Check send a packet size
    ////////////////////////////////////////////////////////////////////////////////
	if(nSize <= 0)
		return;

	nSize = min(nSize, sizeof(char) * PACKET_BODY_SIZE);

    ////////////////////////////////////////////////////////////////////////////////
    // To recieve packet
    ////////////////////////////////////////////////////////////////////////////////
	if(pData)
	{
		m_pRecvPacket->Clear();

		m_pRecvPacket->iClientID = 0;
		m_pRecvPacket->eCommState = eCommState;
		m_pRecvPacket->sizeBody = nSize;

		// jsyoon 2022.08.25 add
		SOCKADDR_IN cliaddr = { 0 };
		int len = sizeof(cliaddr);
		getpeername(m_client_socket, (SOCKADDR*)&cliaddr, &len);
		m_pRecvPacket->cliaddr = cliaddr;

		memcpy((LPVOID)&m_pRecvPacket->szBody, pData, nSize);
	}

	// 수신받은 데이터를 MAIN-HOST 로 전달한다.
	if(eCommState == XNET_RECEIVEDATA)
	{
		if(m_CallbackFunc)
		{
			m_CallbackFunc(m_CallbackPtr, (LPVOID)m_pRecvPacket);
		}
	}
}

UINT CClientSocket::ThreadRun(LPVOID pVoid)
{
    CClientSocket *pThis = (CClientSocket *)pVoid;

	if (pThis)
	{
		pThis->m_bThreadLive = true;

		while (1)
		{
			// Start Event 가 set될 때까지 기다린다.
			if (WaitForSingleObject(pThis->m_hStartEvent, 10) == WAIT_OBJECT_0)
			{
				pThis->ThreadFunc();
			}

			// Close Event 가 set될 때까지 기다린다.
			if (WaitForSingleObject(pThis->m_hCloseEvent, 10) == WAIT_OBJECT_0)
			{
				break;
			}

			Sleep(1);
		}

		pThis->m_bThreadLive = false;
	}

	return 0;
}

void CClientSocket::ThreadFunc()
{
    if (m_client_socket == INVALID_SOCKET)
        return;

    // 구체적인 네트워크 이벤트 알아내기
	WSANETWORKEVENTS NetworkEvents;

	int nRet = SOCKET_ERROR;
	nRet = WSAEnumNetworkEvents(m_client_socket, m_hMesgSocketEvent, &NetworkEvents);
    if(nRet != SOCKET_ERROR)
	{
		if ((NetworkEvents.lNetworkEvents & FD_READ) == FD_READ)
		{
            if (NetworkEvents.iErrorCode[FD_READ_BIT] != 0)
            {
				//printf("FD_READ failed with error %d\n",NetworkEvents.iErrorCode[FD_READ_BIT]);
                return;
            }

			RecvData();
		}

		if ((NetworkEvents.lNetworkEvents & FD_CLOSE) == FD_CLOSE)	// jsyoon 2022.05.19 change
	//	else if ((NetworkEvents.lNetworkEvents & FD_CLOSE) == FD_CLOSE)
		{
            //if (NetworkEvents.iErrorCode[FD_CLOSE_BIT] != 0)
            //{
            //    //printf("FD_CLOSE failed with error %d\n",  NetworkEvents.iErrorCode[FD_CLOSE_BIT]);
            //    return;
            //}

			// 클라이언트 인 경우에는 서버가 삭제 되었으므로 
			// 새로운 socket 을 만들어서 재 접속을 하도록 한다.

			NotifyReceive(XNET_NOTCONNECT, 0, 0);

			SocketCleanUp();
		}
	}
}
