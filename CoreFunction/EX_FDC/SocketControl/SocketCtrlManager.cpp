// SocketCtrlManager.cpp : implementation file
//

#include "stdafx.h"
#include "SocketControl.h"
#include "SocketCtrlManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSocketCtrlManager

CSocketCtrlManager::CSocketCtrlManager()
	: m_pThread(NULL)
	, m_bFlagThreadStop(false)
// 	, m_szIPAddress(_T(""))
// 	, m_nSocketPort(0)
	, m_nSocketType(0)
{
	m_pNetWallTalk = NULL;
	m_bCreated =  FALSE;
	m_pClient = NULL;
}

CSocketCtrlManager::~CSocketCtrlManager()
{
// 	if(m_pNetWallTalk) 
// 		delete m_pNetWallTalk;	//나중에 해결하자 . 2009.07.21

	if( m_pClient != NULL )
	{
		delete m_pClient;
		m_pClient = NULL;
	}

	if(m_pNetWallTalk != NULL)
	{
		delete m_pNetWallTalk;
		m_pNetWallTalk = NULL;
	}
}


BEGIN_MESSAGE_MAP(CSocketCtrlManager, CWnd)
	//{{AFX_MSG_MAP(CSocketCtrlManager)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSocketCtrlManager message handlers

BOOL CSocketCtrlManager::Initialize(CWnd *pParentWnd, UINT nServer, UINT nPort, CString szIPAddress)
{
	pParent = pParentWnd;
	if(m_pNetWallTalk != NULL) 
	{
		delete m_pNetWallTalk;
		m_pNetWallTalk = NULL;
	}

	BOOL bRet = FALSE;

	switch (nServer)
	{
	case SOCKET_TYPE_CLIENT:
		{
			if( m_pClient == NULL )
			{
				m_pClient = new CTcpClient(pParent);
			}

			if( m_pClient != NULL )
			{
				bRet = m_pClient->Initialize();
				m_bCreated = TRUE;
			}

			if( bRet )
			{
				bRet = m_pClient->ConnectToServer(szIPAddress, nPort);
			}
		}
		break;
	case SOCKET_TYPE_SERVER:
		{
			if( m_pNetWallTalk !=NULL ) 
			{
				delete m_pNetWallTalk;
				m_pNetWallTalk = NULL;
			}

			if( m_pNetWallTalk == NULL ) 
			{
				m_pNetWallTalk = new CNetWallTalk(nServer, nPort, szIPAddress);
			}
		
			if(m_pNetWallTalk != NULL)
			{
				bRet = m_pNetWallTalk->Create(pParent);
				m_bCreated = TRUE;
			}
		}
		break;
	}

	m_nSocketType = nServer;

	return bRet;
}

BOOL CSocketCtrlManager::Close()
{
	if( m_bCreated == TRUE)
	{
		EndThread();

		if( m_pClient )
		{
			delete m_pClient;
			m_pClient = NULL;
		}

		if(m_pNetWallTalk != NULL)
		{
			m_pNetWallTalk->Close();
			delete m_pNetWallTalk;
			m_pNetWallTalk = NULL;
		}

		m_bCreated = FALSE;
	}


	return TRUE;
}

BOOL CSocketCtrlManager::Create(CWnd *pParentWnd)
{
	m_wndParent = pParentWnd;

	return TRUE;
}

// 
// int CSocketCtrlManager::StartThread(void)
// {
// 	if (m_pThread == NULL)
// 	{
// 		m_bFlagThreadStop = FALSE;
// 		m_pThread = AfxBeginThread(ThreadProc, this);
// 	}
// 
// 	return 0;
// }
// 
// UINT CSocketCtrlManager::ThreadProc(LPVOID pParam)
// {
// 	CSocketCtrlManager* pProc = (CSocketCtrlManager*)pParam;
// 
// 	while (!pProc->m_bFlagThreadStop)
// 	{
// 		// 100ms마다 한번씩 connection retry
// 		pProc->ConnectToServer();
// 		Sleep(100);
// 	}
// 
// 	return 0;
// }

void CSocketCtrlManager::EndThread()
{
	if( m_pClient != NULL )
	{
		m_pClient->Finalize();
		m_pClient->Disconnect();
		m_bCreated = FALSE;
	}
}


// int CSocketCtrlManager::ConnectToServer(void)
// {
// 	if( m_pNetWallTalk != NULL)
// 	{
// 		m_pNetWallTalk->ConnectSocket();
// // 		if( m_pNetWallTalk->IsConnected() )
// // 		{
// // 			return 0;
// // 		}
// // 
// // 		 
// // 		 m_pNetWallTalk->Connect(m_szIPAddress, m_nSocketPort);
// 	}
// 
// 	return 0;
// }


BOOL CSocketCtrlManager::IsConnected(void)
{
	BOOL bRet = FALSE;

	switch (m_nSocketType)
	{
	case SOCKET_TYPE_CLIENT:
		if( m_pClient != NULL)
			bRet = m_pClient->IsConnected();
		break;
	case SOCKET_TYPE_SERVER:
		if(m_pNetWallTalk != NULL)
			bRet = m_pNetWallTalk->IsConnected();
		break;
	}

	return bRet;
}

int CSocketCtrlManager::SendData(BYTE *pSendData, UINT nDataSize)
{
	int nRet = -1;

	switch (m_nSocketType)
	{
	case SOCKET_TYPE_CLIENT:
		if( m_pClient != NULL)
			nRet = m_pClient->SendData(pSendData, nDataSize);
		break;
	case SOCKET_TYPE_SERVER:
		if(m_pNetWallTalk != NULL)
			nRet = m_pNetWallTalk->SendData(pSendData, nDataSize);
		break;
	}

	return nRet;
}

int CSocketCtrlManager::GetReceiveData(BYTE* pData, UINT nBufSize)
{
	int nRet = 0;

	switch (m_nSocketType)
	{
	case SOCKET_TYPE_CLIENT:
		if( m_pClient != NULL)
			nRet = m_pClient->GetReceiveData(pData, nBufSize);
		break;
	case SOCKET_TYPE_SERVER:
		if(m_pNetWallTalk != NULL)
			nRet = m_pNetWallTalk->GetReceiveData(pData, nBufSize);
		break;
	}

	return nRet;
}

UINT CSocketCtrlManager::GetReceiveDataSize(void)
{
	int nRet = 0;

	switch (m_nSocketType)
	{
	case SOCKET_TYPE_CLIENT:
		if( m_pClient != NULL)
			nRet = m_pClient->GetReceiveDataSize();
		break;
	case SOCKET_TYPE_SERVER:
		if(m_pNetWallTalk != NULL)
			nRet = m_pNetWallTalk->GetReceiveDataSize();
		break;
	}

	return nRet;
}
