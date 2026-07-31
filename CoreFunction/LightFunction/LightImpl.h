#pragma once

#include "..\Comm232Function\Comm232Function.h"

class CLightImpl : public CComm232Function
{

	//------------------------------------------------------------------
	// 공통 인터페이스 함수
	//------------------------------------------------------------------
public:
	virtual BOOL		LinkIniFile(bool bSave) = 0;	// pure virtual
	virtual BOOL		LinkDatFile(bool bSave) = 0;	// pure virtual
    virtual bool		InitDevice(int port) = 0;		// pure virtual
	virtual bool		CloseDevice() = 0;				// pure virtual
    virtual CString		GetDeviceName() = 0;			// pure virtual
    virtual bool		IsConnected() = 0;				// pure virtual
	
    // LED 조명 제어 전용 함수 - 시퀀스 연동
	virtual bool LedLight_Channel_TurnOn(long nChannel, long nIntensity) = 0;				// pure virtual
	virtual bool LedLight_Channel_TurnOff(long nChannel, long nIntensity) = 0;				// pure virtual

	// LED 조명 제어 전용 함수
	virtual bool SetLedLampIntensity(long nChannel, long nIntensity, CString &strCommand);
	virtual bool SetLedLampTurnOnOff(long nChannel, long nIntensity, enumLED_LAMP_CONTROL nCONTROL, CString &strCommand);
	virtual bool GetLedLampIntensity(long nChannel, CString &strCommand);
	virtual bool GetLedLampTurnOnOff(long nChannel, CString &strCommand);
	virtual bool WaitLedLampResponce(long nChannel, long nIntensity, enumRES_CHECK_TYPE nTYPE);
	virtual bool GetCurLedLampIntensity(long nChannel, long &nIntensity, CString &strCommand);

public:
    CLightImpl();
    virtual ~CLightImpl();
};

