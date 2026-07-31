#pragma once

#include "..\..\LightFunction\LightImpl.h"

class CComm232Config;
class CCommSerialObjectECS : public CLightImpl
{
	CComm232Config *m_pConfig;

	//------------------------------------------------------------------
	// 공통 인터페이스 함수
	//------------------------------------------------------------------
public:
	virtual BOOL		LinkIniFile(bool bSave) override;
	virtual BOOL		LinkDatFile(bool bSave) override;
    virtual bool		InitDevice(int port) override;
	virtual bool		CloseDevice() override;
    virtual CString		GetDeviceName() override;
    virtual bool		IsConnected() override;

    // LED 조명 제어 전용 함수 - 시퀀스 연동
	virtual bool		LedLight_Channel_TurnOn(long nChannel, long nIntensity) override;
	virtual bool		LedLight_Channel_TurnOff(long nChannel, long nIntensity) override;

    // LED 조명 제어 전용 함수
	virtual bool		SetLedLampIntensity(long nChannel, long nIntensity, CString &strCommand = CString(_T(""))) override;
	virtual bool		SetLedLampTurnOnOff(long nChannel, long nIntensity, enumLED_LAMP_CONTROL nCONTROL, CString &strCommand = CString(_T(""))) override;
	virtual bool		GetLedLampIntensity(long nChannel, CString &strCommand = CString(_T(""))) override;
	virtual bool		GetLedLampTurnOnOff(long nChannel, CString &strCommand = CString(_T(""))) override;
	virtual bool		WaitLedLampResponce(long nChannel, long nIntensity, enumRES_CHECK_TYPE nTYPE) override;

private:
	virtual DWORD		WriteComm(CString strBuf) override;	// Serial Port에 데이터를 쓴다.
	virtual void		ThreadWorkingLoop() override;  




private:
	bool GetLedLampStateCompare(long nChannel, long nIntensity, enumRES_CHECK_TYPE nTYPE);
	CString MakeCommand_SetIntensity(long nChannel, long nIntensity);
	CString MakeCommand_GetIntensity(long nChannel);
	CString MakeCommand_SetTurnOnOff(long nChannel, long nIntensity, enumLED_LAMP_CONTROL nCONTROL);
	CString MakeCommand_GetTurnOnOff(long nChannel);
	bool IsSameLedIntensity(long nChannel, long nIntensity, CString strVal);
	bool IsSameLedTurnOnOff(long nChannel, bool bTurnOnOff, CString strVal);
	
public :
    CCommSerialObjectECS(CString strFilePath, CString strCategory);
    virtual ~CCommSerialObjectECS();
};
