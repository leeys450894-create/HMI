// NetWallTalk.cpp : implementation file
//

#include "stdafx.h"
#include "SocketControl.h"
#include "NetWallTalk.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNetWallTalk

// 생성자 초기에는 디폴트값으로 셋팅을 한다.
CNetWallTalk::CNetWallTalk()
{   
    m_bIsServer = TRUE;                                                         // 기본은 서버로 운영한다.
    ZeroMemory(m_byteReceiveData, sizeof(m_byteReceiveData));					// 수신버퍼를 초기화 한다.

	m_nSocketPort = NET_DEFAULT_SOCKET_PORT;                                    // 기본 소켓포트를 초기화 한다.
    m_szIPAddress = NET_DEFAULT_SOCKET_IP;                                      // 기본 IP어드레스를 초기화 한다.
    m_wndParent = NULL;                                                         // 기본 부모클래스를 초기화 한다.

    m_pSubNetWallTalk = NULL;                                                   // 서버모드일때 생성이 되는 서브클래스 
    m_nReceiveDataSize = 0;                                                     // 수신한 데이터의 사이즈 

    m_IsConnected = FALSE;                                                      // 초기의 연결상태를 False로 설정한다.
    m_bCreated = FALSE;		                                                    // 소켓이 생성되었는지 알려주는 변수 
	m_pThread = NULL;
	m_nBytesSent = 0;
}

// 생성자 초기값을 입력할수 있다.
CNetWallTalk::CNetWallTalk(BOOL bServer, UINT nPort, CString szIPAddress)
{
    m_bIsServer = bServer;                                                      // 운영모드를 설정한다.
	ZeroMemory(m_byteReceiveData, sizeof(m_byteReceiveData));					// 수신버퍼를 초기화 한다.

    m_nSocketPort = nPort;                                                      // 포트를 초기화 한다.
    m_szIPAddress = szIPAddress;                                                // IP주소를 초기화 한다.
    m_wndParent = NULL;                                                         // 기본 부모클래스를 초기화 한다.

    m_pSubNetWallTalk = NULL;                                                   // 서버 모드일때 생성이 되는 서브클래스 
    m_nReceiveDataSize = 0;                                                     // 수신한 데이터의 사이즈 

    m_IsConnected = FALSE;                                                      // 초기의 연결상태를 False로 설정한다.
    m_bCreated = FALSE;                                                         // 소켓이 생성되었는지 알려주는 변수 
	m_pThread = NULL;
	m_nBytesSent = 0;
}

CNetWallTalk::~CNetWallTalk()
{
	// jsyoon 20180223 add
	if(TRUE == m_IsConnected)               // 연결상태일 경우 
	{
		if(TRUE == m_bIsServer)             // 서버 모드일 경우 
		{
			if( m_pSubNetWallTalk != NULL )
			{
				delete m_pSubNetWallTalk;
				m_pSubNetWallTalk = NULL;
			}

			m_IsConnected = FALSE;  
		}
		else                                // 클라이언트 모드일 경우 
		{
			// 특별히 해줄일이 읍네요...
		}
	}
// 	// 서브소켓을 해제.
// 	SAFE_DELETE(m_pSubNetWallTalk);
}


// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CNetWallTalk, CSocket)
	//{{AFX_MSG_MAP(CNetWallTalk)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// CNetWallTalk member functions

// 소케의 포트를 설정하는 함수 
BOOL CNetWallTalk::SetSocketPort(UINT nSocketPort)
{

    if(TRUE == m_bIsServer)           // 서버 모드일 경우 
    {
        //
        // 서버의 경우 CSocket::Create()시 포트를 설정하기 때문에 
        // 
        //
        if(TRUE == m_bCreated)                      // 이미 소켓이 생성이 되어 있는 경우에는 
        {
            return FALSE;
        }
    }
    else                            // 클라이언트 모드일 경우 
    {
        //
        // 클라이언트의 경우 CSocket::Create()시에는 설정하는 것이 없고
        // CSocket::Connect()할때 포트를 설정한다.
        //
        if(TRUE == m_IsConnected)
        {
            return FALSE;
        }
    }

    m_nSocketPort = nSocketPort;

    return TRUE;
}

// 소켓의 포를 얻는 함수 
UINT CNetWallTalk::GetSocketPort()
{
    return m_nSocketPort;
}

// 동작하는 모드를 바꾸는 함수 
// 소켓이 생성이 되어지기 전에 바꾸어 주어야 한다.
BOOL CNetWallTalk::SetModeChange(BOOL bIsServer)
{
    if(TRUE == m_bCreated)                      // 이미 소켓이 생성이 되어 있는 상태인경우 
    {
        return FALSE;
    }

    m_bIsServer = bIsServer;                    // 모드를 셋팅한다. TRUE = 서버모드                   
                                                //                  FALSE = 클라이언트 모드 
    return TRUE;
}

// IP어드레스를 설정하는 함수 
BOOL CNetWallTalk::SetIPAddress(CString szIPAddress)
{
    if(TRUE == m_bIsServer)           // 서버 모드일 경우 
    {
        if(TRUE == m_bCreated)                      // 이미 소켓이 생성이 되어 있는 경우에는 
        {
            return FALSE;
        }
    }
    else                            // 클라이언트 모드일 경우 
    {
        if(TRUE == m_IsConnected)                   // 이미 연결이 되어 있는 경우 
        {
            return FALSE;
        }
    }
    

    m_szIPAddress = szIPAddress;

    return TRUE;
}

// IP어드레스를 리턴하는 함수 
CString CNetWallTalk::GetIPAddress()
{
    return m_szIPAddress;
}


UINT CNetWallTalk::ThreadSockAccept(LPVOID pParam) 
{
	CNetWallTalk* pProc = (CNetWallTalk*) pParam;

	BOOL bRet = FALSE;

	pProc->SubNetAccept();

	return 0;
}

void CNetWallTalk::SubNetAccept() 
{
	if(NULL != m_pSubNetWallTalk) 
	{
		if(!Accept(*m_pSubNetWallTalk))                    // 서브소켓의 연결에 실패하였을 경우 
		{
			AfxMessageBox("Error : Accept() in socket");
			delete m_pSubNetWallTalk;
			m_pSubNetWallTalk = NULL;
			return;
		}
		else
		{
			m_IsConnected = TRUE;                       // 서버 모드일 경우에는 이때에 연결이 되었다고 판단.
			m_wndParent->SendMessage(NET_MSG_ACCEPT, WPARAM(0), NULL);                 // 서브소켓을 연결함을 알림    
		}

		CSocket::OnAccept(0);
	}
}

// 서버모드일때만 발생하는 함수 
void CNetWallTalk::OnAccept(int nErrorCode) 
{
	// TODO: Add your specialized code here and/or call the base class
	if(NULL == m_pSubNetWallTalk)               // 연결되어진 소켓이 없는 경우 
	{ 
		m_pSubNetWallTalk = new CSubNetWallTalk(this, m_byteReceiveData, &m_nReceiveDataSize);

		// 쓰레드로 처리하자
//		AfxBeginThread(ThreadSockAccept,this);

//		AfxMessageBox("1");

		if(!Accept(*m_pSubNetWallTalk))                    // 서브소켓의 연결에 실패하였을 경우 
		{
			AfxMessageBox("Error : Accept() in socket");
			delete m_pSubNetWallTalk;
			m_pSubNetWallTalk = NULL;
			return;
		}
		else
		{
//			AfxMessageBox("2");
			m_IsConnected = TRUE;                       // 서버 모드일 경우에는 이때에 연결이 되었다고 판단.

			CString szClientIP = GetClientIPAddress();
			m_wndParent->SendMessage(NET_MSG_ACCEPT, WPARAM(nErrorCode), (LPARAM)(LPSTR)(LPCTSTR)szClientIP); 
//			m_wndParent->SendMessage(NET_MSG_ACCEPT, WPARAM(nErrorCode), NULL);                 // 서브소켓을 연결함을 알림    		

			
		}

//		AfxMessageBox("3");

		CSocket::OnAccept(nErrorCode);
		
	}
	else
	{
		return;
	}


		
// 	if(NULL == m_pSubNetWallTalk)               // 연결되어진 소켓이 없는 경우 
// 	{ 
// 		m_pSubNetWallTalk = new CSubNetWallTalk(this, m_byteReceiveData, &m_nReceiveDataSize);
// 
// 		if(!CSocket::Accept(*m_pSubNetWallTalk))                    // 서브소켓의 연결에 실패하였을 경우 
// 		{
// 			AfxMessageBox("Error : Accept() in socket");
// 			delete m_pSubNetWallTalk;
// 			m_pSubNetWallTalk = NULL;
// 			return;
// 		}
// 		else
// 		{
// 			m_IsConnected = TRUE;                       // 서버 모드일 경우에는 이때에 연결이 되었다고 판단.
// 			m_wndParent->SendMessage(NET_MSG_ACCEPT, WPARAM(nErrorCode), NULL);                 // 서브소켓을 연결함을 알림    
// 		}
// 
// 		CSocket::OnAccept(nErrorCode);
// 	}
// 	else                 ㄴ               // 이미 연결되어진 소켓이 있는 경우 
// 	{
// 		CSubNetWallTalk *SubNetWallTalk = new CSubNetWallTalk(this, m_byteReceiveData, &m_nReceiveDataSize);
// 
// 		if(!CSocket::Accept(*SubNetWallTalk))                    // 서브소켓의 연결에 실패하였을 경우 
// 		{
// 			AfxMessageBox("Error : Accept() in socket");
// 			return;
// 		}
// 
// 		CSocket::OnAccept(nErrorCode);
// 
// 		delete SubNetWallTalk;
// 		//SubNetWallTalk.Close();
// 		return;
// 	}

// 	// 이미 연결된 경우, 연결 해제.
// 	if(m_pSubNetWallTalk != NULL)
// 	{
// 		m_pSubNetWallTalk->OnClose(0);
// 
// 		SAFE_DELETE(m_pSubNetWallTalk);
// //		m_pSubNetWallTalk = NULL;
// 	}
// 
// 	// 소켓 연결.
// 	if( m_pSubNetWallTalk == NULL)
// 		m_pSubNetWallTalk = new CSubNetWallTalk(this, m_byteReceiveData, &m_nReceiveDataSize);
// 
// 	BOOL bRet = FALSE;
// 	bRet = CSocket::Accept(*m_pSubNetWallTalk);
// 
// //	if(!CSocket::Accept(*m_pSubNetWallTalk))		// 서브소켓의 연결에 실패하였을 경우 
// 	if( bRet != TRUE)		// 서브소켓의 연결에 실패하였을 경우 
// 	{
// 		AfxMessageBox("Error : Accept() in socket");
// 		SAFE_DELETE(m_pSubNetWallTalk);
// 	}
//     else
//     {
// 		m_IsConnected = TRUE;
// 		CString szClientIP = GetClientIPAddress();
// 		m_wndParent->SendMessage(NET_MSG_ACCEPT, WPARAM(nErrorCode), (LPARAM)(LPSTR)(LPCTSTR)szClientIP);                 // 서브소켓을 연결함을 알림
// //		CSocket::OnAccept(nErrorCode);
// 	}
}

// 클라이언트 모드에서 발생이 되어지는 함수 
void CNetWallTalk::OnClose(int nErrorCode) 
{
	m_IsConnected = FALSE;

	m_wndParent->SendMessage(NET_MSG_CLOSE, WPARAM(nErrorCode), (LPARAM)(LPSTR)(LPCTSTR)m_szIPAddress);                  // 소켓이 종료되었다고 알림
   
	CSocket::OnClose(nErrorCode);
}

// 소켓을 생성하는 부분 ===========================================================================================
//CSocket을 쓰기 위해서는, 모든 쓰레드 마다 AfxSocketInit함수를 각각 호출해함
//AfxSocketInit 함수는, WSAStartup 을 호출하기도 하지만, Socket 핸들 생성과 파괴때 사용할 맵을 
//TLS(Thread Local Storage)에 생성함
//그렇기 때문에, 쓰레드 안에서 CSocket 혹은 CAsycnSocket을 사용하기 위해서는 AfxSocketInit 함수를 먼저 호출해서, 
//TLS에 각각 필요한 맵을 먼저 생성 시켜야 함
//CSocket에서 Create 할 때, 이 TLS에 있는 맵을 찾게 되는데, 맵이 생성이 안되어 있어서, 에러가 발생.
//따라서, AfxSocketInit함수를 호출해 주시기 바랍니다.
//=================================================================================================================

BOOL CNetWallTalk::Create(CWnd *pParentWnd)
{
	//쓰레드내에서 소켓 생성시 가끔 디버깅 에러 발생함. 혹시나 해서 추가해 보았음
//	AfxSocketInit();	//Added by Jay Hong : 2009.06.16

	m_wndParent = pParentWnd;

    if(TRUE == m_bIsServer)                         // 서버모드로 동작중일 경우 
    {    
        if(TRUE == m_bCreated)                       // 이미생성이 되어져 있는 경우에는 
        {
            return TRUE;
        }

        if (!CSocket::Create(m_nSocketPort, SOCK_STREAM))
	    {
		    return FALSE;
	    }

	    if (!CSocket::Listen())
	    {
		    return FALSE;
	    }
    }
    else                                            // 클라이언트 모드로 동작중일 경우 
    {
//         if(TRUE == m_IsConnected)       // 이미 연결이 되어 있는 상태라면 
//         {
//             return TRUE;				// KJOH 20120322 change.
//         }

		TRACE("CSocket::Connect =>IP:%s,Port:%d\n", m_szIPAddress, m_nSocketPort); // superman 2011.04.27 add trace

		if( !m_bCreated )
		{
			if (!CSocket::Create())
			{
//				m_bCreated = TRUE;
				return FALSE;
			}
		}

// 	    if (!CSocket::Connect(m_szIPAddress, m_nSocketPort))
// 	    {
// 			TRACE("CSocket::Connect Fail=>IP:%s,Port:%d, %d\n", m_szIPAddress, m_nSocketPort, GetLastError()); // superman 2011.04.27 add trace
//             CSocket::Close();
// 		    return FALSE;
// 	    }
// 
//         m_IsConnected = TRUE;           // 클라이언트 모드일 경우에는 이때 연결이 되었다 판단.
    }

	m_bCreated = TRUE;                  // 소켓이 생성이 되면 True

    return TRUE;
}

// 데이터를 가져가기 위한 함수 
int CNetWallTalk::GetReceiveData(BYTE *pData, UINT nBufSize)
{
    int RetVal = 0;

    if(GetReceiveDataSize() == 0)           // 수신한 데이터가 없는 경우 
    {
        return -1;
    }

    if(nBufSize > GetReceiveDataSize())
    {
        RetVal = (int)GetReceiveDataSize();
    }
    else
    {
        RetVal = (int)nBufSize;
    }
    
    memcpy(pData, m_byteReceiveData, RetVal);
    
    m_nReceiveDataSize = 0;                 // 한번 읽어가면 사이즈를 0으로 초기화 시켜놓는다.

    return RetVal;
}

// 데이터를 전송하는 함수
int CNetWallTalk::SendData(BYTE *pSendData, UINT nDataSize)
{
    if(FALSE == m_IsConnected)              // 연결상태가 아닌 경우는 전송하지 않는다.
    {
        return -1;
    }

    if(TRUE == m_bIsServer)                 // 서버모드인경우 
    {
        m_nBytesSent = m_pSubNetWallTalk->SendData(pSendData, nDataSize);
    }
    else                                    // 클라인언트 모드인경우 
    {	
		m_nBytesBufferSize = (int)nDataSize;
		m_szSendData = pSendData;
		m_nBytesSent = Send(m_szSendData, m_nBytesBufferSize);
    }
    
 //   return TRUE;
	return m_nBytesSent;	// jsyoon 20131119 change
}


void CNetWallTalk::OnSend(int nErrorCode) 
{
	while (m_nBytesSent < m_nBytesBufferSize)
    {
        int dwBytes;

        if ((dwBytes = Send((BYTE*)m_szSendData + m_nBytesSent, m_nBytesBufferSize - m_nBytesSent)) == SOCKET_ERROR)
        {
            if (GetLastError() == WSAEWOULDBLOCK) break;
            else
            {
                TCHAR szError[256];
                wsprintf(szError, "Server Socket failed to send: %d", GetLastError());
//                Close();
            }
        }
        else
        {
            m_nBytesSent += dwBytes;
        }
    }

    if(m_nBytesSent == m_nBytesBufferSize)
    {
        m_nBytesSent = m_nBytesBufferSize = 0;
    }

	CSocket::OnSend(nErrorCode);
}

// 서브소켓을 제거하는 함수 
void CNetWallTalk::SubClose(int nErrorCode)
{
	CString szClientIP = GetClientIPAddress();
    m_wndParent->SendMessage(NET_MSG_CLOSE, WPARAM(nErrorCode), (LPARAM)(LPSTR)(LPCTSTR)szClientIP);
	
	SAFE_DELETE(m_pSubNetWallTalk);
	m_IsConnected = FALSE;
}

// 로컬 아이피를 알아내는 함수 
CString CNetWallTalk::GetLocalIP()
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

// 현재 연결되었는지 검사를 하는 함수 
BOOL CNetWallTalk::IsConnected()
{
    return m_IsConnected;
}

// 클라이언트의 포트를 알아낸다.
UINT CNetWallTalk::GetClientPort()
{
    UINT RetVal;

    if(TRUE == m_bIsServer)             // 서버 모드인 경우 
    {
        if(TRUE == m_IsConnected)       // 연결상태일 경우 
        {
			CString strClientIP = "";
			UINT TempPort = 0;
			m_pSubNetWallTalk->GetPeerName(strClientIP, TempPort);

            RetVal = TempPort;
        }
        else                            // 연결상태가 아닐경우 
        {
            RetVal = 0;
        }
    }
    else                                    // 클라이언트 모드인 경우 
    {
        RetVal = GetSocketPort();
    }

    return RetVal;
}

// 서버쪽의 포트를 알아낸다.
UINT CNetWallTalk::GetServerPort()
{
    UINT RetVal;

    if(TRUE == m_bIsServer)                 // 서버 모드인 경우 
    {
        RetVal = GetSocketPort();
    }
    else                                    // 클라이언트 모드인 경우 
    {
        if(TRUE == m_IsConnected)       // 연결상태일 경우 
        {
            RetVal = GetSocketPort();
        }
        else                            // 연결상태가 아닐경우 
        {
            RetVal = 0;
        }
    }

    return RetVal;
}

// 클라이언트의 IP어드레스를 알아낸다.
CString CNetWallTalk::GetClientIPAddress()
{
    CString RetVal = "";

    if(TRUE == m_bIsServer)                 // 서버 모드인 경우 
    {
        if(TRUE == m_IsConnected)       // 연결상태일 경우 
        {
            UINT TempPort = 0;

            m_pSubNetWallTalk->GetPeerName(RetVal, TempPort);
        }
    }
    else                                    // 클라이언트 모드인 경우 
    {
        RetVal = GetLocalIP();
    }

    return RetVal;
}

// 서버쪽의 IP어드레스를 알아낸다.
CString CNetWallTalk::GetServerIPAddress()
{
    CString RetVal = "";

    if(TRUE == m_bIsServer)                 // 서버 모드인 경우 
    {
        RetVal = GetLocalIP();
    }
    else                                    // 클라이언트 모드인 경우 
    {
        if(TRUE == m_IsConnected)       // 연결상태일 경우 
        {
            RetVal = GetIPAddress();
        }
    }

    return RetVal;
}

// 서버의 Port를 설정하는 함수 
BOOL CNetWallTalk::SetServerPort(UINT nPort)
{
    return SetSocketPort(nPort);
}

// 서버의 IP어드레스를 설정하는 함수 
BOOL CNetWallTalk::SetServerIPAddress(CString sIPAddress)
{
    return SetIPAddress(sIPAddress);
}

// 클라이언트로 생성이 되었을 경우에만 발생하는 함수 
void CNetWallTalk::OnReceive(int nErrorCode) 
{
//	ZeroMemory(m_byteReceiveData, sizeof(m_byteReceiveData));							// 수신버퍼를 초기화 한다.	// jsyoon 20150116 add
	m_nReceiveDataSize = (UINT)Receive(m_byteReceiveData, NET_MAX_RECEIVE_SIZE);		// 데이터를 수신한다

	UINT uSize = GetReceiveDataSize();
    m_wndParent->SendMessage(NET_MSG_RECEIVE, WPARAM(nErrorCode), LPARAM(uSize));		// 데이터를 수신했다고 알림

	CSocket::OnReceive(nErrorCode);
}

// 수신한 데이터의 사이즈를 리턴하는 함수 
UINT CNetWallTalk::GetReceiveDataSize()
{
    return m_nReceiveDataSize;
}

/////////////////////////////////////////////////////////////////////////////
// CSubNetWallTalk

// 생성자 
CSubNetWallTalk::CSubNetWallTalk(CSocket *pwndParant, BYTE* pReceiveDataPtr, UINT* pReceiveDataSize)
{
    m_pwndParent = pwndParant;                                                  // 부모클래스의 핸들을 가져온다.
    m_pReceiveData = pReceiveDataPtr;                                           // 수신한 버퍼의 포인터를 저장한다.
    m_pReceiveDataSize = pReceiveDataSize;                                      // 수신한 데이터사이즈를 저장한 변수포인터 

	// jsyoon 20180912 add
	m_szSendData = NULL;
	m_nBytesBufferSize = 0;
	m_nBytesSent = 0;
}

// 소멸자 
CSubNetWallTalk::~CSubNetWallTalk()
{
	
}


// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CSubNetWallTalk, CSocket)
//{{AFX_MSG_MAP(CSubNetWallTalk)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// CSubNetWallTalk member functions

// 소켓이 종료되었을 경우 호출되는 함수 
void CSubNetWallTalk::OnClose(int nErrorCode) 
{
	CSocket::OnClose(nErrorCode);
	
	((CNetWallTalk*)m_pwndParent)->SubClose(nErrorCode);
}

// 서브소켓에서 데이터를 전송하는 함수 
int CSubNetWallTalk::SendData(BYTE *pSendData, UINT nDataSize)
{
	int nRtn = 0;
    m_nBytesBufferSize = (int)nDataSize;
    m_szSendData = pSendData;
    nRtn = Send(m_szSendData, m_nBytesBufferSize);
	m_nBytesSent = nRtn;

	return nRtn;
}

// 서브소켓의 전송중에 발생하는 함수 
void CSubNetWallTalk::OnSend(int nErrorCode) 
{
	// TODO: Add your specialized code here and/or call the base class
    while (m_nBytesSent < m_nBytesBufferSize)
    {
        int dwBytes;

        if ((dwBytes = Send((BYTE*)m_szSendData + m_nBytesSent, m_nBytesBufferSize - m_nBytesSent)) == SOCKET_ERROR)
        {
            if (GetLastError() == WSAEWOULDBLOCK) break;
            else
            {
                TCHAR szError[256];
                wsprintf(szError, "Server Socket failed to send: %d", GetLastError());
//                Close();
//                AfxMessageBox (szError);
            }
        }
        else
        {
            m_nBytesSent += dwBytes;
        }
    }

    if(m_nBytesSent == m_nBytesBufferSize)
    {
        m_nBytesSent = m_nBytesBufferSize = 0;
    }

	CSocket::OnSend(nErrorCode);
}

////////////////////////////////////////////////////////////////////////////////
// 데이터가 수신되었을 경우에 호출하는 함수
// Modified by Jay Hong : 2009.04.29
// buffer size가 수신되는 데이터 사이즈보다 작은 경우 처리해야 함.
// 아직 생각중 - Jay Hong : 2009.04.29
//
// Buffer Size보다 많은 데이터가 들어오는 경우 NET_MAX_RECEIVE_SIZE 갯수 만큼만 수신처리하고
// 상위단으로 수신된 데이터 갯수를 보낸다.
// 
////////////////////////////////////////////////////////////////////////////////
void CSubNetWallTalk::OnReceive(int nErrorCode) 
{
	*m_pReceiveDataSize = (UINT)Receive(m_pReceiveData, NET_MAX_RECEIVE_SIZE);      // 데이터를 수신한다.

	UINT uSize = ((CNetWallTalk*)m_pwndParent)->GetReceiveDataSize();

	m_pReceiveData[uSize] = 0x00;

    ((CNetWallTalk*)m_pwndParent)->m_wndParent->SendMessage(NET_MSG_RECEIVE, WPARAM(nErrorCode), LPARAM(uSize));
                                                                                    
	CSocket::OnReceive(nErrorCode);
}

void CNetWallTalk::ConnectSocket(void)
{
	if( m_IsConnected )
	{
		return;
	}

// 	if (!CSocket::Create())
// 	{
// 		m_IsConnected = FALSE;
// 		return;
// 	}

// 	if( !m_bCreated )
// 	if (CSocket::Create())
// 	{
// 		m_bCreated = TRUE;                  // 소켓이 생성이 되면 True
// 	}

	if (!CSocket::Connect(m_szIPAddress, m_nSocketPort))
	{
		m_IsConnected = FALSE;
		TRACE("CSocket::Connect Fail=>IP:%s,Port:%d, %d\n", m_szIPAddress, m_nSocketPort, GetLastError()); // superman 2011.04.27 add trace
//		CSocket::Close();
		return;
	}

// 	if( !m_bCreated )
// 	{
// 		if (Create())
// 		{
// 			m_bCreated = TRUE;                  // 소켓이 생성이 되면 True
// 		}
// 	}
// 
// 	if (!Connect(m_szIPAddress, m_nSocketPort))
// 	{
// 		m_IsConnected = FALSE;
// 		TRACE("CSocket::Connect Fail=>IP:%s,Port:%d, %d\n", m_szIPAddress, m_nSocketPort, GetLastError()); // superman 2011.04.27 add trace
// 		return;
// 	}

	m_IsConnected = TRUE;           // 클라이언트 모드일 경우에는 이때 연결이 되었다 판단.

	return;
}