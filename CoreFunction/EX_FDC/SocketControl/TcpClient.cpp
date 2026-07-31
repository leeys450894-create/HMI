// TcpClient.cpp: implementation of the CTcpClient class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TcpClient.h"
//#include "TcpManager.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif



//////////////////////////////////////////////////////////////////////////
// CCSocket
CCSocket::CCSocket(CWnd *pParent)
{
	::InitializeCriticalSection (&m_csCSock);

	m_pParent = pParent;
	m_bCreated = FALSE;
	m_bConnected = FALSE;
	m_nSizeReceiveData = 0;		// jsyoon 20170209 add

	ZeroMemory(m_byteReceiveData, sizeof(m_byteReceiveData));					// 수신버퍼를 초기화 한다.
}

CCSocket::~CCSocket(void)
{

}

BOOL CCSocket::CreateSocket()
{
	BOOL rv = TRUE;

	if (!m_bCreated)
	{
		rv = CAsyncSocket::Create();
		SetCreated(rv);
	}

	return rv;
}

BOOL CCSocket::Connect(LPCTSTR lpszHostAddress, UINT nHostPort)
{
	if (m_bConnected)
	{
		return TRUE;
	}

//	TRACE("CCSocket::Connect =>IP:%s,Port:%d\n", lpszHostAddress, nHostPort); // superman 2011.04.27 add trace

	return CAsyncSocket::Connect(lpszHostAddress, nHostPort);
}

void CCSocket::Close()
{
	if (m_bConnected)
	{
		OnClose(0);
	}
}

BOOL CCSocket::IsConnected()
{
	return m_bConnected;
}

void CCSocket::SetConnected(BOOL bConnected)
{
	::EnterCriticalSection (&m_csCSock);
	m_bConnected = bConnected;
	::LeaveCriticalSection (&m_csCSock);
}

void CCSocket::SetCreated(BOOL bCreated)
{
	::EnterCriticalSection (&m_csCSock);
	m_bCreated = bCreated;
	::LeaveCriticalSection (&m_csCSock);
}

int CCSocket::SendData(BYTE *pbySend, UINT nSize)
{
	return Send(pbySend, nSize);
}

void CCSocket::OnReceive(int nErrCode)
{
	nErrCode;

//	BYTE byteRecv[MAX_RECEIVE_SIZE] = {0};
	long size = 0;

	size = CAsyncSocket::Receive(m_byteReceiveData, MAX_RECEIVE_SIZE);      // 데이터를 수신한다.
	m_nSizeReceiveData = size;

//	m_pParent->SendMessage(SOCKET_MSG_RECEIVE, (WPARAM)size, (LPARAM)byteRecv);
	m_pParent->SendMessage(SOCKET_MSG_RECEIVE, (WPARAM)0, (LPARAM)size);

//	ZeroMemory(m_byteReceiveData, sizeof(m_byteReceiveData));					// 수신버퍼를 초기화 한다.
}

UINT CCSocket::GetSizeReceiveData()
{
	return m_nSizeReceiveData;
}

void CCSocket::SetSizeReceiveData(UINT iSize)
{
	m_nSizeReceiveData = iSize;
}

void CCSocket::OnClose(int nErrorCode)
{
	// close
	CAsyncSocket::Close();

	if( m_bConnected )
	{
		SetConnected(FALSE);
		m_pParent->SendMessage(SOCKET_MSG_CLOSE, NULL, NULL);

		// re-create
		SetCreated(FALSE);
		CreateSocket();
	}

	CAsyncSocket::OnClose(nErrorCode);
}

void CCSocket::OnConnect(int nErrorCode)
{
	if (nErrorCode == 0)
	{
		SetConnected(TRUE);
		m_pParent->SendMessage(SOCKET_MSG_CONNECT, NULL, NULL);
	}

	CAsyncSocket::OnConnect(nErrorCode);
}

int CTcpClient::GetReceiveData(BYTE* pData, UINT nBufSize)
{
	int RetVal = 0;

	UINT nSize = 0;
	nSize = m_pClient->GetSizeReceiveData();

	if(nSize == 0)           // 수신한 데이터가 없는 경우 
	{
		return -1;
	}

	if(nBufSize > nSize)
	{
		RetVal = (int)nSize;
	}
	else
	{
		RetVal = (int)nBufSize;
	}

	memcpy(pData, m_pClient->m_byteReceiveData, RetVal);

	m_pClient->SetSizeReceiveData(0);                 // 한번 읽어가면 사이즈를 0으로 초기화 시켜놓는다.

	return RetVal;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTcpClient::CTcpClient(CWnd *pParent)
{
	m_pClient = new CCSocket(pParent);

	m_pThread = NULL;
}

CTcpClient::~CTcpClient(void)
{
	if (m_pClient)
	{
		delete(m_pClient);
		m_pClient = NULL;
	}
}

BOOL CTcpClient::Initialize()
{
	if (!m_pClient->CreateSocket())
	{
		return FALSE;
	}

	return TRUE;
}

void CTcpClient::Finalize()
{
	EndThread();
}

BOOL CTcpClient::ConnectToServer(CString lpszIpAddr, UINT nPort)
{
	m_lpszIpAddr = lpszIpAddr;
	m_nPort = nPort;

	TRACE("ConnectToServer::Connect =>IP:%s,Port:%d\n", m_lpszIpAddr, m_nPort); // superman 2011.04.27 add trace
 
	StartThread();

	return TRUE;
}

void CTcpClient::StartThread()
{
	m_bFlagThreadStop = FALSE;

	if (m_pThread == NULL)
	{	
		m_pThread = AfxBeginThread(ThreadProc, this);
	}
}

UINT CTcpClient::ThreadProc(LPVOID pParam)
{
	CTcpClient* pProc = (CTcpClient*)pParam;

	while (!pProc->m_bFlagThreadStop)
	{
		// 100ms마다 한번씩 connection retry
		pProc->Connect(pProc->m_lpszIpAddr, pProc->m_nPort);
		Sleep(100);
	} 


	pProc->m_bFlagThreadStop = FALSE;
//	SetEvent(pProc->m_hThreadKilled);	//쓰레드가 종료되었음

	return 0;
}

void CTcpClient::EndThread()
{
	m_bFlagThreadStop = TRUE;

// 	if (::WaitForSingleObject(m_pThread->m_hThread, 3000))
// 	{
// 		delete m_pThread;
// 		m_pThread = NULL;
// 	}

	if( m_pThread != NULL)
	{
		int nCount = 0;
		do 
		{
			Sleep(100);
			if( nCount> 20)
			{
				TerminateThread(m_pThread->m_hThread, 0);
				break;
			}

			nCount++;
		} while (m_bFlagThreadStop == TRUE);

// 		DWORD dwResult = WaitForSingleObject(m_pThread->m_hThread, 3000);
// 		if (dwResult == WAIT_TIMEOUT)
// 			TerminateThread(m_pThread->m_hThread, 0);

//		delete m_pThread;
		m_pThread = NULL;
	}

	TRACE("CTcpClient::EndThread =>IP:%s,Port:%d\n", m_lpszIpAddr, m_nPort); // superman 2011.04.27 add trace
}


BOOL CTcpClient::Connect(LPCTSTR lpszIpAddr, UINT nPort)
{
	BOOL rv = FALSE;

	if( m_pClient != NULL)
		rv = m_pClient->Connect(lpszIpAddr, nPort);

	return rv;
}

void CTcpClient::Disconnect()
{
	if( m_pClient != NULL)
		m_pClient->Close();
}

BOOL CTcpClient::IsConnected()
{
	if( m_pClient != NULL)
		return m_pClient->IsConnected();
	else
		return FALSE;
}

int CTcpClient::SendData(BYTE* pbySend, UINT nSize)
{
	if( m_pClient != NULL)
		return m_pClient->SendData(pbySend, nSize);
	else
		return -1;
}

UINT CTcpClient::GetReceiveDataSize(void)
{

	UINT nSize = 0;

	if( m_pClient != NULL)
	{
		if( IsConnected() == TRUE)
			nSize = m_pClient->GetSizeReceiveData();
	}

	return nSize;
}

CString CTcpClient::GetServerIPAddress()
{
	return m_lpszIpAddr;
}

CString CTcpClient::GetClientIPAddress()
{
	CString RetVal = "";
                                
	RetVal = GetLocalIP();		 // 클라이언트 모드인 경우 

	return RetVal;
}

CString CTcpClient::GetLocalIP()
{
	WSADATA wsaData;
	char name[255];

	CString ip; // 여기에 lcoal ip가 저장됩니다.

	PHOSTENT hostinfo;

	if( WSAStartup( MAKEWORD( 2, 0 ), &wsaData ) == 0 )
	{
		if( gethostname ( name, sizeof(name)) == 0)
		{
			if((hostinfo = gethostbyname(name)) != NULL)
			{
				ip = inet_ntoa (*(struct in_addr *)*hostinfo->h_addr_list);
			}
		}      
		WSACleanup( );
	} 

	return ip;
}