#pragma once

#include "..\CommEthFunction\EthernetManagerTCPDefine.h"

class CCIMImpl;
class CCIMFunction
{
	CCIMImpl	*m_pCtrlImpl;
	

	//------------------------------------------------------------------
	// 외부 접근 가능 함수
	//------------------------------------------------------------------
public:
	void		SetCallbackFunction(LPVOID callbackPtr, CallbackClassFunction2 callbackFunc);
    bool		InitDevice();
	void		CloseDevice();
    CString		GetDeviceName();
    bool		IsConnected();
    bool		SendToMsgData(CString strMsg);

public:
	CCIMFunction(CString strDeviceModel, CString strFilePath, CString strCategory);
	~CCIMFunction();	
};

