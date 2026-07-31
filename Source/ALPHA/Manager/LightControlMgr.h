#pragma once

#include "..\AuroraSubMgr.h"
#include "LightFunction\LightFunction.h"
#include <afxmt.h>

class CIniHelper;
class CLightFunction;
class CLightControlMgr : public CAuroraSubMgr
{
    CIniHelper				*m_pIniHelper;
	bool					m_bUse;				// 장치 사용여부, UI상에서도 보이지 않게 처리함. OutOfService와는 다름
	bool					m_bOutOfService;	// 모듈 사용 여부 0: USE
	CString					m_strControlType;
	CLightFunction			*m_pFunction;
	CCriticalSection		m_cs_on;
	CCriticalSection		m_cs_off;
	CCriticalSection		m_cs_set;
	CCriticalSection		m_cs_get;
	CCriticalSection		m_cs_res;


public:
    bool	InitDevice(int port);
	bool	CloseDevice(int port);
    CString	GetDeviceName(int port);
    bool	IsConnected(int port);
	bool	IsOutOfService(int port);
	long	GetCount();

    // LED 조명 제어 전용 함수 - 시퀀스 연동
	bool	LedLight_Channel_TurnOn(int port, long nChannel, long nIntensity);
	bool	LedLight_Channel_TurnOff(int port, long nChannel, long nIntensity);
    
	// LED 조명 제어 전용 함수
	bool	SetLedLampIntensity(int port, long nChannel, long nIntensity, CString &strCommand = CString(_T("")));
	bool	SetLedLampTurnOnOff(int port, long nChannel, long nIntensity, enumLED_LAMP_CONTROL nCONTROL, CString &strCommand = CString(_T("")));
	bool	GetLedLampIntensity(int port, long nChannel, CString &strCommand = CString(_T("")));
	bool	GetLedLampTurnOnOff(int port, long nChannel, CString &strCommand = CString(_T("")));
	bool	WaitLedLampResponce(int port, long nChannel, long nIntensity, enumRES_CHECK_TYPE nTYPE);
	bool	GetCurLedLampIntensity(int port, long nChannel, long &nIntensity, CString &strCommand = CString(_T("")));

	CLogFileAccessor *GetLogFileAccessorPtr();

//------------------------------------------------------------------
// 공통 인터페이스 함수
//------------------------------------------------------------------
public:
	virtual bool LinkIniFile(bool bSave) override;
private:
	virtual void Finalize() override;
public:
	CLightControlMgr(CString strAppPath, CString strConfigPath, CString strLogPath);
	virtual ~CLightControlMgr();
};

