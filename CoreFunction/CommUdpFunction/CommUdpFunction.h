#pragma once

#include "EthernetManagerUDPDefine.h"

class CEthernetManagerUDP;
class CCommUdpFunction
{
    CEthernetManagerUDP *m_pEthernetUDP;

public:
    void Initialize(LPVOID callbackPtr, CallbackClassFunction2 callbackFunc);
    void Finalize();

    long Start(CString strOwnIP, UINT uOwnPort, CString strTargetIP, UINT uTargetPort);
    void Stop();

    bool SendTo(LPVOID lpData, int nSize);
    bool IsConnected();

public:
    CCommUdpFunction(void);
    virtual ~CCommUdpFunction(void);
};
