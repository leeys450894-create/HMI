#pragma once

#include "EthernetManagerTCPDefine.h"

class CEthernetSocket;
class CEthernetManagerTCP
{
    CEthernetSocket *m_pEthernetSocket;
	
public:
    void Initialize(bool bRunServer, LPVOID callbackPtr, CallbackClassFunction2 callbackFunc);
    void Finalize();

	bool Start(char *szIPNo, USHORT usPortNo);    
	void Stop();
	bool SendTo(LPVOID lpData, int nSize);

	bool IsLive();
	bool IsConnected();

public:
    CEthernetManagerTCP();
    ~CEthernetManagerTCP();
};
