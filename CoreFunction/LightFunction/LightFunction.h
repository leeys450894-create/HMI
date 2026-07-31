#pragma once

#include "..\Comm232Function\CommSerialDefine.h"

inline BOOL MakeDirPath(CString sPath)
{
	if (sPath == _T(""))
	{
		return FALSE;
	}

	if (sPath[1] != ':' || sPath[2] != '\\')
	{
		return FALSE;
	}

	int nFind = 2;
	while ((nFind = sPath.Find(_T("\\"), nFind + 1)) >= 0)
	{
		CString sTem = sPath.Left(nFind);
		CreateDirectory(sTem, NULL);
	}

	CreateDirectory(sPath, NULL);

	return TRUE;
}

#define dfLightChannelMax	4		//4개 채널로 고정

struct stLightInfo
{
	bool bUse;
	bool bOutOfService;

	CString strDevicetype;
	long lPort;
	long lMax;

	stLightInfo()
	{
		bUse = false;
		bOutOfService = false;

		strDevicetype = _T("SPO");
		lPort = 99;
		lMax = 4;
	}
};

class CLogFileAccessor;
class CIniHelper;
class CLightImpl;
class CLightFunction
{
	CLogFileAccessor			*m_pLogFileAccessor;
	CIniHelper					*m_pIniHelper;
	std::vector<CLightImpl*>	m_vecpCtrlImpl;
	std::vector<stLightInfo>	m_vecLightInfo;

	int m_nNum;		//LED 수량

	long GetDeviceNum(long nChannel);
	long GetDeviceChannel(long nChannel);
	

	//------------------------------------------------------------------
	// 외부 접근 가능 함수
	//------------------------------------------------------------------
public:
	bool	LinkIniFile(bool bSave);
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
	CLogFileAccessor *GetLogFileAccessorPtr() { return m_pLogFileAccessor; }

public:
	CLightFunction(CString strAppPath, CString strConfigPath, CString strLogFilePath);
	~CLightFunction();
};

