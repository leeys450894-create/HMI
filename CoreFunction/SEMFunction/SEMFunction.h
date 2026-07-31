#pragma once

#include "..\Comm232Function\Comm232Function.h"

struct stSEMDATA
{
	int nID;
	float fVal;
	TCHAR szSVName[32];
};

class CLogFileAccessor;
class CIniHelper;
class CComm232Config;
class CCommEthFunction;
class CSEMFunction : public CComm232Function
{
	CLogFileAccessor			*m_pLogFileAccessor;
	CIniHelper					*m_pIniHelper;
	CComm232Config				*m_pComm232;
	CCommEthFunction			*m_pCommEth;
	bool m_bUse;
	bool m_bOutOfService;
	int  m_nInterface;
	bool m_bTraceEnable;
	bool m_bIsConnet;
	bool m_bSetAlarm;
	CString m_strConfigPath;
	CString m_strDeviceType;
	int  m_nAlarmTime;
	CString  m_strComPort;
	CString  m_strBaudRate;
	CString  m_strIPNumber;
	CString  m_strPortNumber;
	stSEMDATA m_stSEM_Data[20];
	stSEMDATA m_stSEM_Test;
	std::vector<stSEMDATA>	m_vecData;

	//------------------------------------------------------------------
	// 외부 접근 가능 함수
	//------------------------------------------------------------------
public:
	bool	LinkIniFile(bool bSave);
	bool	Initialize();
	bool	StartDevice();
	bool	CloseDevice();
	bool	IsConnected();
	bool    IsReceive();
	int		GetAlarmTime();

public:
	CSEMFunction(CString strAppPath, CString strConfigPath, CLogFileAccessor *pLogFileAccessor);
	~CSEMFunction();

	static void CallbackRecieveData(LPVOID, LPVOID);
	void CallbackRecieveData(LPVOID ud);
	void AnalysisReceive(CString strRcv);
	void AnalysisReceiveData(CString strData);

	void SetConfigFile(int nInterface, int nPort, int nBaudrate, int nAlarmTime);
	bool GetData(int ID, UINT& uAddr, CString& sName, float& fVal );
};

