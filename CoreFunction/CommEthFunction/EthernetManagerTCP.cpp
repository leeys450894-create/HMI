#include "pch.h"
#include "EthernetManagerTCP.h"

#include "EthernetSocket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CEthernetManagerTCP::CEthernetManagerTCP()
{
	m_pEthernetSocket = nullptr;
}

CEthernetManagerTCP::~CEthernetManagerTCP()
{
	Finalize();
}

void CEthernetManagerTCP::Initialize(bool bRunServer, LPVOID callbackPtr, CallbackClassFunction2 callbackFunc)
{
	if(m_pEthernetSocket == nullptr)
	{
		m_pEthernetSocket = new CEthernetSocket(bRunServer);
	}

	m_pEthernetSocket->SetCallbackFunction(callbackPtr, callbackFunc);
}

void CEthernetManagerTCP::Finalize()
{
    if(m_pEthernetSocket)
    {
		delete m_pEthernetSocket;
        m_pEthernetSocket = nullptr;
    }
}

bool CEthernetManagerTCP::Start(char *szIPNo, USHORT usPortNo)
{
	//if (m_pEthernetSocket && m_pEthernetSocket->IsCreated())
	//	return true;

	//return m_pEthernetSocket->CreateSocket(szIPNo, usPortNo);

	if (m_pEthernetSocket)
	{
		if (m_pEthernetSocket->IsCreated())
			return true;
		else
			return m_pEthernetSocket->CreateSocket(szIPNo, usPortNo);
	}
	else
	{
		return false;
	}
}

void CEthernetManagerTCP::Stop()
{
	//Finalize();
	m_pEthernetSocket->CloseSocket();
}


bool CEthernetManagerTCP::SendTo(LPVOID lpData, int nSize)
{
	if(m_pEthernetSocket && m_pEthernetSocket->IsCreated())
	{
		return m_pEthernetSocket->SendMesgData(lpData, nSize);
	}

    return false;
}

bool CEthernetManagerTCP::IsLive()
{
	if(m_pEthernetSocket && m_pEthernetSocket->IsCreated())
	{
		return m_pEthernetSocket->IsThreadLive();
	}

	return false;
}

bool CEthernetManagerTCP::IsConnected()
{
	if(m_pEthernetSocket && m_pEthernetSocket->IsCreated())
	{
		return m_pEthernetSocket->GetConnected();
	}

	return false;
}