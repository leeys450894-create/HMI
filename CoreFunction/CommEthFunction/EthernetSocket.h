#pragma once

#include "EthernetManagerTCPDefine.h"

class CServerSocket;
class CClientSocket;
class CEthernetSocket
{
    bool			m_bIsServer;
	CServerSocket	*m_pServerSocket;
	CClientSocket	*m_pClientSocket;

public:
    bool	CreateSocket(char *szServerName, USHORT usPortNo = 9999 );

	bool	SendMesgData(LPVOID lpData, int nSize);
	bool	IsCreated();
	bool	IsThreadLive();
	bool	GetConnected();
	
	bool	CloseSocket();


    //************************************************************************************************
    // 리시브 함수를 위한 콜백함수 설정 함수들
    //************************************************************************************************
public:
	void SetCallbackFunction(LPVOID callbackPtr, CallbackClassFunction2 callbackFunc);

public:
    CEthernetSocket(bool bRunServer);
    ~CEthernetSocket();
};
