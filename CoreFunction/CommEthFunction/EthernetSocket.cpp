#include "pch.h"
#include "EthernetSocket.h"

#include "ServerSocket.h"
#include "ClientSocket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CEthernetSocket::CEthernetSocket(bool bRunServer)
{
    m_bIsServer		= bRunServer;
	m_pServerSocket	= new CServerSocket;
	m_pClientSocket	= new CClientSocket;
}

CEthernetSocket::~CEthernetSocket()
{
	if(m_pServerSocket)
	{
		delete m_pServerSocket;
		m_pServerSocket = nullptr;
	}

	if(m_pClientSocket)
	{
		delete m_pClientSocket;
		m_pClientSocket = nullptr;
	}
}

bool CEthernetSocket::CreateSocket(char *szIPNo, USHORT usPortNo)
{
	if(m_bIsServer)
	{
		return m_pServerSocket->CreateSocket(szIPNo, usPortNo);
	}
	
	return m_pClientSocket->CreateSocket(szIPNo, usPortNo);
}

bool CEthernetSocket::SendMesgData(LPVOID lpData, int nSize)
{
	if(m_bIsServer)
	{
		return m_pServerSocket->SendMesgData(lpData, nSize);
	}
	
	return m_pClientSocket->SendMesgData(lpData, nSize);
}

bool CEthernetSocket::IsCreated()
{
	if(m_bIsServer)
	{
		return m_pServerSocket->IsCreated();
	}
	
	return m_pClientSocket->IsCreated();
}

bool CEthernetSocket::IsThreadLive()
{
	if(m_bIsServer)
	{
		return m_pServerSocket->IsThreadLive();
	}
	
	return m_pClientSocket->IsThreadLive();
}

bool CEthernetSocket::GetConnected()
{
	if(m_bIsServer)
	{
		return m_pServerSocket->GetConnected();
	}
	
	return m_pClientSocket->GetConnected();
}

bool CEthernetSocket::CloseSocket()
{
	if (m_bIsServer)
	{
		return m_pServerSocket->CloseSocket();
	}

	return m_pClientSocket->CloseSocket();
}

void CEthernetSocket::SetCallbackFunction(LPVOID callbackPtr, CallbackClassFunction2 callbackFunc)
{
	if(m_bIsServer)
	{
		return m_pServerSocket->SetCallbackFunction(callbackPtr, callbackFunc);
	}
	
	return m_pClientSocket->SetCallbackFunction(callbackPtr, callbackFunc);
}
