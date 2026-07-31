#pragma once

#include "..\..\LightFunction\LightImpl.h"

#define XCITE_TT			_T("tt\r")	// Connect, enables PC control of the unit.
#define XCITE_II			_T("ii\r")	// Get Intensity Level
#define XCITE_HH			_T("hh\r")	// Get Lamp Hours
#define XCITE_VV			_T("vv\r")	// Get Software Version
#define XCITE_UU			_T("uu\r")	// Get Unit Status
#define XCITE_IX			_T("i")		// Set Intensity Level 0~4
#define XCITE_ZZ			_T("zz\r")	// Close Shutter
#define XCITE_MM			_T("mm\r")	// Open Shutter
#define XCITE_SS			_T("ss\r")	// Turn Lamp Off
#define XCITE_BB			_T("bb\r")	// Turn Lamp On
#define XCITE_AA			_T("aa\r")	// Clear Alarm
#define XCITE_LL			_T("ll\r")	// Lock Front Panel
#define XCITE_NN			_T("nn\r")	// UnLock Front Panel

class CComm232Config;
class CCommSerialObjectXCITE : public CLightImpl
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
	bool IsLedTurnOnOff(long nChannel, bool bTurnOnOff, CString strVal);
	bool IsShutterOpenClose(long nChannel, bool bShutterOpenClose, CString strVal);

	bool ConnectDevice(CString &strCommand = CString(_T("")));
	CString MakeCommand_ConnectDevice();
	bool OpenShutter(CString &strCommand = CString(_T("")));
	CString MakeCommand_OpenShutter();
	bool CloseShutter(CString &strCommand = CString(_T("")));
	CString MakeCommand_CloseShutter();
	bool GetUnitStatus(CString &strCommand = CString(_T("")));
	CString MakeCommand_GetUnitStatus();

public :
	CCommSerialObjectXCITE(CString strFilePath, CString strCategory);
    virtual ~CCommSerialObjectXCITE();
};
