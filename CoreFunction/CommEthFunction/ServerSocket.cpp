#include "pch.h"
#include "ServerSocket.h"

#pragma comment(lib, "ws2_32.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CServerSocket::CServerSocket()
{
	// 데이터를 전송하고 받기 위해서 사용하는 thread
	m_pThread			= AfxBeginThread(ThreadRun, this);
    m_bThreadLive		= false;
	m_hAcptDEvent		= CreateEvent(NULL, TRUE, FALSE, NULL);		// manual set, initial set
	m_hStartEvent		= CreateEvent(NULL, TRUE, FALSE, NULL);		// manual set, initial reset
    m_hCloseEvent		= CreateEvent(NULL, FALSE, FALSE, NULL);	// auto reset, initial reset
	m_bIsCreated		= false;
    m_bConnected		= false;

	// 통신 송/수신 관련 이벤드
    m_hAcptSocketEvent	= WSACreateEvent();
	m_client_socket		= INVALID_SOCKET;
	m_pszSendData		= new char[PACKET_BODY_SIZE];
	m_pszRecvData		= new char[PACKET_BODY_SIZE];
	m_pRecvPacket		= new TRECEIVEDDATA;
	m_hMesgSocketEvent	= WSACreateEvent();
    m_server_socket		= INVALID_SOCKET;

    m_CallbackPtr		= nullptr;
    m_CallbackFunc		= nullptr;

	m_szIPNo[0] = 0X00;		// jsyoon 2022.05.17 add

	m_nClientcnt = 0;


	for (size_t i = 0; i < FD_SETSIZE; i++)
	{
		m_hev_base[i] = NULL;
		m_sock_base[i] = INVALID_SOCKET;
	}
}

CServerSocket::~CServerSocket()
{
	//-------------------------------------------
	// Thread 관련 제거
	//-------------------------------------------
	SetEvent(m_hCloseEvent);
	ResetEvent(m_hAcptDEvent);
	ResetEvent(m_hStartEvent);
	
	if(m_bThreadLive && m_pThread)
	{
		WaitForSingleObject(m_pThread->m_hThread, INFINITE);
	}

	CloseHandle(m_hAcptDEvent);
	CloseHandle(m_hStartEvent);
	CloseHandle(m_hCloseEvent);

	//-------------------------------------------
	// 통신 송/수신 관련 이벤드
	//-------------------------------------------
	WSACloseEvent(m_hAcptSocketEvent);
	WSACloseEvent(m_hMesgSocketEvent);

	//// jsyoon 2022.08.25 add
	//for (size_t i = 0; i < FD_SETSIZE; i++)
	//{
	//	if (m_sock_base[i] != INVALID_SOCKET)
	//	{
	//		WSACloseEvent(m_sock_base[i]);
	//	}
	//}
	/////

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

bool CServerSocket::CreateSocket(char *szIPNo, USHORT usPortNo)
{
	//WSAStartup함수는 프로세스에서 Winsock 2 DLL의 사용을 초기화한다.
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

	// 통신 관련 정보를 서버 소켓에 등록한다.
	int ret = ::bind(m_client_socket, (struct sockaddr *)&m_sin, sizeof sockaddr);
    if (ret == SOCKET_ERROR)
    {
        return SocketCleanUp();
    }

    // 접속을 기다린다. - SOMAXCONN : 최대 접속 개수값
    if (listen(m_client_socket, SOMAXCONN) == SOCKET_ERROR)
    {
        return SocketCleanUp();
    }

	AddNetworkEvent(m_client_socket, FD_ACCEPT );
 //   WSAEventSelect(m_client_socket, m_hAcptSocketEvent, FD_ACCEPT | FD_CLOSE);

	m_bIsCreated = true;
	
	// accept 를 받기 위해서...
	ResetEvent(m_hStartEvent);
	SetEvent(m_hAcptDEvent);

	// jsyoon 2022.05.17 add
	strcpy_s(m_szIPNo, sizeof(m_szIPNo), szIPNo);
	m_usPortNo = usPortNo;


    return true;
}

bool CServerSocket::SendMesgData(LPVOID lpData, int nSize)
{
	for (size_t i = 1; i < FD_SETSIZE; i++)
	{
		if (m_sock_base[i] == INVALID_SOCKET)
			continue;;

		////////////////////////////////////////////////////////////////////////////////
		// Check send a packet size
		////////////////////////////////////////////////////////////////////////////////
		nSize = min(nSize, sizeof(char) * PACKET_BODY_SIZE);

		ZeroMemory(m_pszSendData, sizeof(char) * PACKET_BODY_SIZE);
		memcpy(m_pszSendData, (char*)lpData, nSize);

		////////////////////////////////////////////////////////////////////////////////
		// To send packet
		////////////////////////////////////////////////////////////////////////////////
		int	nTotalBytes = nSize;
		int	nBytesSent = 0, nErrorCount = 0, nSendLoopCount = 0;

		while (nBytesSent < nTotalBytes)
		{
			int nThisSendSize = send(m_sock_base[i], m_pszSendData + nBytesSent, nTotalBytes - nBytesSent, 0);

			if (nThisSendSize < 0)
				break;

			if (SOCKET_ERROR == nThisSendSize)
				nErrorCount++;
			else
				nSendLoopCount++;

			nBytesSent += nThisSendSize;

			if ((m_sock_base[i] == INVALID_SOCKET) || (nErrorCount >= 50) || (nSendLoopCount >= 100))
			{
				return false;
			}

			nErrorCount = 0;
			nSendLoopCount = 0;

			Sleep(1);
		}
	}

	return true;

 //   if (m_server_socket == INVALID_SOCKET)
 //       return false;

 //   ////////////////////////////////////////////////////////////////////////////////
 //   // Check send a packet size
 //   ////////////////////////////////////////////////////////////////////////////////
	//nSize = min(nSize, sizeof(char) * PACKET_BODY_SIZE);

	//ZeroMemory(m_pszSendData, sizeof(char) * PACKET_BODY_SIZE);
 //   memcpy(m_pszSendData, (char *)lpData, nSize);

 //   ////////////////////////////////////////////////////////////////////////////////
 //   // To send packet
 //   ////////////////////////////////////////////////////////////////////////////////
 //   int	nTotalBytes	= nSize;
 //   int	nBytesSent = 0, nErrorCount = 0, nSendLoopCount = 0;
 //   
	//while (nBytesSent < nTotalBytes)
 //   {
 //       int nThisSendSize = send(m_server_socket, m_pszSendData + nBytesSent, nTotalBytes - nBytesSent, 0);

 //       if (nThisSendSize < 0)
 //           break;

 //       if (SOCKET_ERROR == nThisSendSize)
 //           nErrorCount++;
 //       else
 //           nSendLoopCount++;

 //       nBytesSent += nThisSendSize;

	//	if( (m_server_socket == INVALID_SOCKET) || (nErrorCount >= 50) || (nSendLoopCount >= 100) )
 //       {
 //           return false;
 //       }

 //       nErrorCount = 0;
 //       nSendLoopCount = 0;

 //       Sleep(1);
 //   }

	//return true;
}

bool CServerSocket::CloseSocket()
{
	return SocketCleanUp();
}

bool CServerSocket::IsCreated() const
{ 
	return m_bIsCreated;	
}

bool CServerSocket::IsThreadLive() const
{ 
	return m_bThreadLive;		
}

bool CServerSocket::GetConnected() const
{ 
	return m_bConnected;	
}








//--------------------------------------------------------------------
// 내부 함수들
//--------------------------------------------------------------------
bool CServerSocket::FillAddress(USHORT usPortNo, char *szServerName)
{
    memset(&m_sin, 0, sizeof(SOCKADDR_IN));

    m_sin.sin_family = AF_INET;
    m_sin.sin_port = htons(usPortNo);
    m_sin.sin_addr.s_addr = htonl(INADDR_ANY);

    return true;
}

// return 값은 socket 정보를 삭제하였기 때문에 무조건 false 로 리턴시켜서
// MAIN-HOST 에서 재시도 하도록 유도한다.
bool CServerSocket::SocketCleanUp()
{
	m_bConnected = false;
	m_bIsCreated = false;

    if (m_client_socket != INVALID_SOCKET)
	{
		shutdown(m_client_socket, SD_BOTH); 
		closesocket(m_client_socket);
		m_client_socket = INVALID_SOCKET;
	}
    if (m_server_socket != INVALID_SOCKET)
	{
		shutdown(m_server_socket, SD_BOTH); 
	    closesocket(m_server_socket);
		m_server_socket = INVALID_SOCKET;
	}

	for (size_t i = 0; i < FD_SETSIZE; i++)
	{
		shutdown(m_sock_base[i], SD_BOTH);
		closesocket(m_sock_base[i]);
		m_sock_base[i] = INVALID_SOCKET;
	}

	// Winsock 2 DLL(Ws2_32.dll)의 사용을 종료합니다
    WSACleanup();

	return false;
}

HANDLE CServerSocket::AddNetworkEvent(SOCKET sock, long net_event)
{
	HANDLE hev = WSACreateEvent();

	m_sock_base[m_nClientcnt] = sock;
	m_hev_base[m_nClientcnt] = hev;
	m_nClientcnt++;

	WSAEventSelect(sock, hev, net_event);

	return hev;
}

bool CServerSocket::Accept()
{
    if (m_client_socket == INVALID_SOCKET)
        return false;

	m_nAceeptAddrLen = sizeof sockaddr;

	SOCKET dosock = accept(m_client_socket, (struct sockaddr*)&m_sAceeptAddr, &m_nAceeptAddrLen);

	if (m_nClientcnt == FD_SETSIZE)
	{
		closesocket(dosock);
		return false;
	}

	AddNetworkEvent(dosock, FD_READ | FD_CLOSE);

	//if (m_server_socket != INVALID_SOCKET)
	//{
	//	shutdown(m_server_socket, SD_BOTH);
	//	closesocket(m_server_socket);
	//	m_server_socket = INVALID_SOCKET;
	//}

 //   m_server_socket = accept(m_client_socket, (struct sockaddr *)&m_sAceeptAddr, &m_nAceeptAddrLen);

 //   if (m_server_socket == INVALID_SOCKET)
 //   {
 //       return false;
 //   }

	//WSAEventSelect(m_server_socket, m_hMesgSocketEvent, FD_READ | FD_CLOSE);

	m_bConnected = true;

    return true;
}

bool CServerSocket::RecvData()
{
	if (m_server_socket == INVALID_SOCKET)
        return false;

    ////////////////////////////////////////////////////////////////////////////////
    // Make recieve a packet
    ////////////////////////////////////////////////////////////////////////////////
    ZeroMemory(m_pszRecvData, sizeof(char) * PACKET_BODY_SIZE);

    int nTotalBytes = PACKET_BODY_SIZE;
    
	int nBytesRecv = recv(m_server_socket, m_pszRecvData, nTotalBytes, 0);

	if ( 0 < nBytesRecv )
	{
		int nErrorCount = 0, nRecvLoopCount = 0;
		
		while (nBytesRecv < nTotalBytes)
		{
			int nThisRecvsize = recv(m_server_socket, m_pszRecvData + nBytesRecv, nTotalBytes - nBytesRecv, 0);

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

			if( (m_server_socket == INVALID_SOCKET) || (nErrorCount >= 500) || (nRecvLoopCount >= 1000) )
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
	// jsyoon 2022.05.18 remove
	//else if (0xffffffff == nBytesRecv)
	//{
	//	// jsyoon 2022.05.17 add 
	//	SocketCleanUp();
	//	CreateSocket(m_szIPNo, m_usPortNo);
	//}
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

void CServerSocket::NotifyReceive(eXNET_COMM_STATE eCommState, LPVOID pData, size_t nSize, int iClientID)
{
	// jsyoon 2022.03.08 add
	if (eCommState == XNET_NOTCONNECT )
	{
		m_pRecvPacket->Clear();

		m_pRecvPacket->iClientID = iClientID;
		m_pRecvPacket->eCommState = eCommState;
		m_pRecvPacket->sizeBody = 0;

		SOCKADDR_IN cliaddr = { 0 };
		int len = sizeof(cliaddr);
		getpeername(m_sock_base[iClientID], (SOCKADDR*)&cliaddr, &len);

		m_pRecvPacket->cliaddr = cliaddr;
	//	m_pRecvPacket->szClientIP = inet_ntoa(cliaddr.sin_addr);
	//	m_pRecvPacket->iClientPort = ntohs(cliaddr.sin_port);

		if (m_CallbackFunc)
		{
			m_CallbackFunc(m_CallbackPtr, (LPVOID)m_pRecvPacket);
		}
	}

	else if ( eCommState == XNET_SERVER_ACCEPT)
	{
		m_pRecvPacket->Clear();

		m_pRecvPacket->iClientID = iClientID;
		m_pRecvPacket->eCommState = eCommState;
		m_pRecvPacket->sizeBody = 0;

		SOCKADDR_IN cliaddr = { 0 };
		int len = sizeof(cliaddr);
		getpeername(m_sock_base[iClientID], (SOCKADDR*)&cliaddr, &len);

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

		SOCKADDR_IN cliaddr = { 0 };
		int len = sizeof(cliaddr);
		getpeername(m_sock_base[iClientID], (SOCKADDR*)&cliaddr, &len);

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

UINT CServerSocket::ThreadRun(LPVOID pVoid)
{
    CServerSocket *pThis = (CServerSocket *)pVoid;

	if (pThis)
	{
		pThis->m_bThreadLive = true;

		while (1)
		{
			// Accept Event 가 set될 때까지 기다린다.
			if (WaitForSingleObject(pThis->m_hAcptDEvent, 10) == WAIT_OBJECT_0)
			{
				pThis->AcceptFunc();
			}

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

void CServerSocket::AcceptFunc()
{
    if (m_client_socket == INVALID_SOCKET)
        return;
	
    // 구체적인 네트워크 이벤트 알아내기

	int index = WSAWaitForMultipleEvents(m_nClientcnt, m_hev_base, false, INFINITE, false);
	if (index >= 0)
	{
		WSANETWORKEVENTS net_events;
		WSAEnumNetworkEvents(m_sock_base[index], m_hev_base[index], &net_events);

		switch (net_events.lNetworkEvents)
		{
		case FD_ACCEPT: AcceptProc(index);  break;
		case FD_READ:   ReadProc(index); break;
		case FD_CLOSE:  CloseProc(index); break;
		}
	}


	//WSANETWORKEVENTS NetworkEvents;
	//if(WSAEnumNetworkEvents(m_client_socket, m_hAcptSocketEvent, &NetworkEvents) != SOCKET_ERROR)
	//{
	//	// FD_ACCEPT Event Handling
	//	if ((NetworkEvents.lNetworkEvents & FD_ACCEPT) == FD_ACCEPT) 
	//	{
	//		if (NetworkEvents.iErrorCode[FD_ACCEPT_BIT] != 0)
	//		{
	//			//printf("FD_ACCEPT failed with error %d\n", NetworkEvents.iErrorCode[FD_ACCEPT_BIT]);
	//			return;
	//		}

	//		if(Accept() == true)
	//		{
	//			// message 를 받기 위해서...
	//		//	ResetEvent(m_hAcptDEvent);
	//			SetEvent(m_hStartEvent);
	//		}
	//	}
	//	else if ((NetworkEvents.lNetworkEvents & FD_CLOSE) == FD_CLOSE)
	//	{
 //           //if (NetworkEvents.iErrorCode[FD_CLOSE_BIT] != 0)
 //           //{
 //           //    //printf("FD_CLOSE failed with error %d\n",  NetworkEvents.iErrorCode[FD_CLOSE_BIT]);
 //           //    return;
 //           //}

	////		NotifyReceive(XNET_NOTCONNECT, 0, 0);

	//		// 클라이언트의 접속이 끊겨져 있으므로
	//		// 새로운 socket 을 만들어서 재 접속을 하도록 한다.
	//		SocketCleanUp();
	//	}
	//	else if ((NetworkEvents.lNetworkEvents == 0))
	//	{
	//		; 
	//	}
	//	else
	//	{
	//		int i = 0;
	//		;
	//	}
	//}
}

void CServerSocket::ThreadFunc()
{
    if (m_server_socket == INVALID_SOCKET)
        return;

    // 구체적인 네트워크 이벤트 알아내기
	WSANETWORKEVENTS NetworkEvents;

	if (WSAEnumNetworkEvents(m_server_socket, m_hMesgSocketEvent, &NetworkEvents) != SOCKET_ERROR)
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
	//	else if ((NetworkEvents.lNetworkEvents & FD_CLOSE) == FD_CLOSE)
		if ((NetworkEvents.lNetworkEvents & FD_CLOSE) == FD_CLOSE)	// jsyoon 2022.05.18 change
		{
            //if (NetworkEvents.iErrorCode[FD_CLOSE_BIT] != 0)
            //{
            //    //printf("FD_CLOSE failed with error %d\n",  NetworkEvents.iErrorCode[FD_CLOSE_BIT]);
            //    return;
            //}

			NotifyReceive(XNET_NOTCONNECT, 0, 0);
			// 클라이언트의 접속이 끊겨져 있으므로
			// 새로운 socket 을 만들어서 재 접속을 하도록 한다.
			SocketCleanUp();

			// jsyoon 2022.05.17 add 
			CreateSocket(m_szIPNo, m_usPortNo);
		}
		else if ((NetworkEvents.lNetworkEvents == 0))
		{
			;
		}
		else
		{
			int i = 0;
			;
		}
	}
}

bool CServerSocket::AcceptProc(int index)
{
	if (m_client_socket == INVALID_SOCKET)
		return false;

	m_nAceeptAddrLen = sizeof sockaddr;

	SOCKET dosock = accept(m_client_socket, (struct sockaddr*)&m_sAceeptAddr, &m_nAceeptAddrLen);

	if (m_nClientcnt == FD_SETSIZE)
	{
		closesocket(dosock);
		return false;
	}

	m_bConnected = true;

	AddNetworkEvent(dosock, FD_READ | FD_CLOSE);

	NotifyReceive(XNET_SERVER_ACCEPT, 0, 0, m_nClientcnt -1);

	CString strIP(inet_ntoa(m_sAceeptAddr.sin_addr));
	TRACE(_T("%s:%d 입장\n"), strIP, ntohs(m_sAceeptAddr.sin_port));

	
	return true;
}

void CServerSocket::CloseProc(int index)
{
	NotifyReceive(XNET_NOTCONNECT, 0, 0, index);

	SOCKADDR_IN cliaddr = { 0 };
	int len = sizeof(cliaddr);
	getpeername(m_sock_base[index], (SOCKADDR*)&cliaddr, &len);

	CString strIP(inet_ntoa(cliaddr.sin_addr));
	TRACE(_T("%s:%d 나감\n"), strIP, ntohs(cliaddr.sin_port));

	closesocket(m_sock_base[index]);
	WSACloseEvent(m_hev_base[index]);


	m_nClientcnt--;
	m_sock_base[index] = m_sock_base[m_nClientcnt];
	m_hev_base[index] = m_hev_base[m_nClientcnt];

	m_sock_base[m_nClientcnt] = INVALID_SOCKET;
	m_hev_base[m_nClientcnt]  = NULL;


	bool bConnect = false;
	for (size_t i = 1; i < FD_SETSIZE; i++)
	{
		if (m_hev_base[i] != NULL)
		{
			bConnect = true;
			break;
		}
	}

	if (bConnect != true)  
	{
		m_bConnected = false;
	}

	
	/*char msg[256];
	sprintf(msg, "[%s:%d]님 나감~\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
	for (int i = 1; i < m_nClientcnt; i++)
	{
		send(m_sock_base[i], msg, 256, 0);
	}*/
}

bool CServerSocket::ReadProc(int index)
{
	/*char msg[MAX_MSG_LEN];
	recv(m_sock_base[index], msg, MAX_MSG_LEN, 0);

	SOCKADDR_IN cliaddr = { 0 };
	int len = sizeof(cliaddr);
	getpeername(m_sock_base[index], (SOCKADDR*)&cliaddr, &len);

	char smsg[MAX_MSG_LEN];
	sprintf(smsg, "[%s:%d]:%s", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), msg);*/


	if (m_sock_base[index] == INVALID_SOCKET)
		return false;

	////////////////////////////////////////////////////////////////////////////////
	// Make recieve a packet
	////////////////////////////////////////////////////////////////////////////////
	ZeroMemory(m_pszRecvData, sizeof(char) * PACKET_BODY_SIZE);

	int nTotalBytes = PACKET_BODY_SIZE;

	int nBytesRecv = recv(m_sock_base[index], m_pszRecvData, nTotalBytes, 0);

	if (0 < nBytesRecv)
	{
		int nErrorCount = 0, nRecvLoopCount = 0;

		while (nBytesRecv < nTotalBytes)
		{
			int nThisRecvsize = recv(m_sock_base[index], m_pszRecvData + nBytesRecv, nTotalBytes - nBytesRecv, 0);

			if (nThisRecvsize < 0)
			{
				NotifyReceive(XNET_RECEIVEDATA, m_pszRecvData, nBytesRecv, index);
				break;
			}

			if (SOCKET_ERROR == nThisRecvsize)
				nErrorCount++;
			else
				nRecvLoopCount++;

			nBytesRecv += nThisRecvsize;

			if ((m_server_socket == INVALID_SOCKET) || (nErrorCount >= 500) || (nRecvLoopCount >= 1000))
			{
				// 잘못된 패킷 수신으로 재전송 요구
				return false;
			}

			nErrorCount = 0;

			Sleep(1);
		}
	}
	else if (0 == nBytesRecv)
	{

	}
	else
	{

	}

	return true;
}
