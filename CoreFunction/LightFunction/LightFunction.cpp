#include "pch.h"
#include "LightFunction.h"

#include "SPO\CommSerialObjectSPO.h"
#include "CAS\CommSerialObjectCAS.h"
#include "ECS\CommSerialObjectECS.h"
#include "XCITE\CommSerialObjectXCITE.h"

#include "..\common\IniHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CLightFunction::CLightFunction(CString strAppPath, CString strConfigPath, CString strLogFilePath)
	: m_pIniHelper(new CIniHelper)
	, m_pLogFileAccessor(new CLogFileAccessor)
{ 
	// 로그 생성하기
	const CString strFolderPath = strLogFilePath + _T("Light\\");
	const CString strLogFile = strFolderPath + _T("Light.log");
	MakeDirPath(strFolderPath);
	m_pLogFileAccessor->Initialize(strLogFile);
	//////

	m_pIniHelper->SetFilePath(strConfigPath);
	LinkIniFile(false);

	m_vecpCtrlImpl.clear();
	m_vecpCtrlImpl.resize(m_vecLightInfo.size());

	for (size_t nNum = 0; nNum < m_vecLightInfo.size(); nNum++)
	{
		CString strDeviceModel = m_vecLightInfo[nNum].strDevicetype;

		CString strNewCategory;
		strNewCategory.Format(_T("%s_%d"), strDeviceModel, nNum + 1);

		if (strDeviceModel == _T("SPO"))
		{
			m_vecpCtrlImpl[nNum] = dynamic_cast<CLightImpl*>(new CCommSerialObjectSPO(strConfigPath, strNewCategory, m_pLogFileAccessor));
		}
		else if (strDeviceModel == _T("CAS"))
		{
			m_vecpCtrlImpl[nNum] = dynamic_cast<CLightImpl*>(new CCommSerialObjectCAS(strConfigPath, strNewCategory, m_pLogFileAccessor));
		}
		else if (strDeviceModel == _T("ECS"))
		{
			m_vecpCtrlImpl[nNum] = dynamic_cast<CLightImpl*>(new CCommSerialObjectECS(strConfigPath, strNewCategory));
		}
		else if (strDeviceModel == _T("XCITE"))
		{
			m_vecpCtrlImpl[nNum] = dynamic_cast<CLightImpl*>(new CCommSerialObjectXCITE(strConfigPath, strNewCategory));
		}
		else
		{
			m_vecpCtrlImpl[nNum] = dynamic_cast<CLightImpl*>(new CCommSerialObjectSPO(strConfigPath, strNewCategory, m_pLogFileAccessor));
		}
	}

	CString strLog;
	strLog.Format(_T("CLightFunction Initialize Start"));
	m_pLogFileAccessor->KMLOG(SYST, strLog, true);
}

CLightFunction::~CLightFunction()
{
	for (size_t i = 0; i < m_vecpCtrlImpl.size(); ++i)
	{
		if (m_vecpCtrlImpl[i])
		{
			delete m_vecpCtrlImpl[i];
			m_vecpCtrlImpl[i] = nullptr;
		}
	}

	// 2023.03.06 add
	//----------------------------------------------------------------------
	// Data 삭제
	//----------------------------------------------------------------------
	if (m_pIniHelper)
	{
		delete m_pIniHelper;
		m_pIniHelper = nullptr;
	}

	if (m_pLogFileAccessor)
	{
		delete m_pLogFileAccessor;
		m_pLogFileAccessor = nullptr;
	}
}

long CLightFunction::GetDeviceNum(long nChannel)
{
	return nChannel / dfLightChannelMax;
}

long CLightFunction::GetDeviceChannel(long nChannel)
{
	return nChannel % dfLightChannelMax + 1;
}



//------------------------------------------------------------------
// 외부 접근 가능 함수
//------------------------------------------------------------------
bool CLightFunction::LinkIniFile(bool bSave)
{
	m_pIniHelper->Link(bSave, _T("LIGHT_CONTROL_MGR"), _T("NUM"), &m_nNum, 1);

	m_vecLightInfo.resize(m_nNum);

	for (int n = 0; n < m_nNum; n++)
	{
		CString strCategory;
		strCategory.Format(_T("%s_%d"), _T("LIGHT_CONTROL_MGR"), n + 1);
		
		m_pIniHelper->Link(bSave, strCategory, _T("USE"),				&m_vecLightInfo[n].bUse,			false);
		m_pIniHelper->Link(bSave, strCategory, _T("OUT_OF_SERVICE"),	&m_vecLightInfo[n].bOutOfService,	false);
		m_pIniHelper->Link(bSave, strCategory, _T("DEVICE_TYPE"),		&m_vecLightInfo[n].strDevicetype,	(CString)_T("SPO"));
		m_pIniHelper->Link(bSave, strCategory, _T("MAX_USE_CH"),		&m_vecLightInfo[n].lMax, (long)4);
		
		CString strPort;
		m_pIniHelper->Link(bSave, strCategory, _T("RS232_PORTID"), &strPort, (CString)_T("COM99"));
		m_vecLightInfo[n].lPort = _wtol(strPort.TrimLeft(_T("COM")));
	}

	return m_pIniHelper->IsReadEnable();
}

bool CLightFunction::InitDevice(int port)
{
	for (size_t i = 0; i < m_vecpCtrlImpl.size(); ++i)
	{
		if (i == port)
	//	if (m_vecLightInfo[i].lPort == (long)port)
		{
			int iPort = 0;
			iPort = m_vecLightInfo[i].lPort;

			CString strLog;
			strLog.Format(_T("InitDevice. Type = %s, Port = %d"), m_vecLightInfo[i].strDevicetype, iPort);
			m_pLogFileAccessor->KMLOG(OPER, strLog, true);
			
			bool bRet = false;
			bRet = m_vecpCtrlImpl[i]->InitDevice(iPort);
			if (bRet == true)
			{
				CString str;
				str.Empty();
				long lMax = m_vecLightInfo[i].lMax;
				for (int j = 0; j < lMax; j++)
				{
					m_vecpCtrlImpl[i]->SetLedLampIntensity(j+1, 0, str);
				}
			}
		}
	}

	return false;
}

bool CLightFunction::CloseDevice(int port)
{
	for (size_t i = 0; i < m_vecLightInfo.size(); ++i)
	{
//		if (m_vecLightInfo[i].lPort == (long)port)
		if (i == (long)port)
		{
			int iPort = 0;
			iPort = m_vecLightInfo[i].lPort;

			CString strLog;
			strLog.Format(_T("CloseDevice. Type = %s, Port = %d"), m_vecLightInfo[i].strDevicetype, iPort);
			m_pLogFileAccessor->KMLOG(OPER, strLog, true);

			CString str;
			str.Empty();
			long lMax = m_vecLightInfo[i].lMax;
			for (int j = 0; j < lMax; j++)
			{
				m_vecpCtrlImpl[i]->SetLedLampIntensity(j + 1, 0, str);
			}

			return m_vecpCtrlImpl[i]->CloseDevice();
		}
	}

	return true;
}

CString	CLightFunction::GetDeviceName(int port)
{
	for (size_t i = 0; i < m_vecLightInfo.size(); ++i)
	{
//		if (m_vecLightInfo[i].lPort == (long)port)
		if (i == (long)port)
		{
			return m_vecLightInfo[i].strDevicetype; //m_vecpCtrlImpl[i]->GetDeviceName();
		}
	}

	return _T("");
}

bool CLightFunction::IsConnected(int port)
{
	for (size_t i = 0; i < m_vecLightInfo.size(); ++i)
	{
//		if (m_vecLightInfo[i].lPort == (long)port)
		if (i == (long)port)
		{
			m_vecpCtrlImpl[i]->IsConnected();
		}
	}

	return true;
}

bool CLightFunction::IsOutOfService(int port)
{
	bool bRet = false;
	for (size_t i = 0; i < m_vecLightInfo.size(); ++i)
	{
//		if (m_vecLightInfo[i].lPort == (long)port)
		if (i == (long)port)
		{
			bRet = m_vecLightInfo[i].bOutOfService;
		}
	}

	return bRet;
}

long CLightFunction::GetCount()
{
	long nRet = 0;

	nRet = m_vecpCtrlImpl.size();

	return nRet;
}


// LED 조명 제어 전용 함수 - 시퀀스 연동
bool CLightFunction::LedLight_Channel_TurnOn(int port, long nChannel, long nIntensity)
{
	CString strLog;

	strLog.Format(_T("LedLight_Channel_TurnOn. Device = %d, Channel = %d, Intensity = %d"), port + 1, nChannel, nIntensity);
	m_pLogFileAccessor->KMLOG(SEND, strLog, true);

	return m_vecpCtrlImpl[port]->LedLight_Channel_TurnOn(GetDeviceChannel(nChannel), nIntensity);

	//strLog.Format(_T("LedLight_Channel_TurnOn. Channel = %d, Intensity = %d"), nChannel, nIntensity);
	//m_pLogFileAccessor->KMLOG(SEND, strLog, true);
	//strLog.Format(_T("LedLight_Channel_TurnOn. Calc. DeviceNum = %d, Calc. Intensity = %d"), GetDeviceNum(nChannel), GetDeviceChannel(nChannel));
	//m_pLogFileAccessor->KMLOG(SEND, strLog, true);

	//return m_vecpCtrlImpl[GetDeviceNum(nChannel)]->LedLight_Channel_TurnOn(GetDeviceChannel(nChannel), nIntensity);
}

bool CLightFunction::LedLight_Channel_TurnOff(int port, long nChannel, long nIntensity)
{
	CString strLog;
	strLog.Format(_T("LedLight_Channel_TurnOff. Device = %d, Channel = %d, Intensity = %d"), port + 1, nChannel, nIntensity);
	m_pLogFileAccessor->KMLOG(SEND, strLog, true);

	return m_vecpCtrlImpl[port]->LedLight_Channel_TurnOff(GetDeviceChannel(nChannel), nIntensity);

	//strLog.Format(_T("LedLight_Channel_TurnOff. Channel = %d, Intensity = %d"), nChannel, nIntensity);
	//m_pLogFileAccessor->KMLOG(SEND, strLog, true);
	//strLog.Format(_T("LedLight_Channel_TurnOff. Calc. DeviceNum = %d, Calc. Intensity = %d"), GetDeviceNum(nChannel), GetDeviceChannel(nChannel));
	//m_pLogFileAccessor->KMLOG(SEND, strLog, true);

	//return m_vecpCtrlImpl[GetDeviceNum(nChannel)]->LedLight_Channel_TurnOff(GetDeviceChannel(nChannel), nIntensity);
}
    
// LED 조명 제어 전용 함수
bool CLightFunction::SetLedLampIntensity(int port, long nChannel, long nIntensity, CString &strCommand)
{
	CString strLog;
	strLog.Format(_T("SetLedLampIntensity. Device = %d, Channel = %d, Intensity = %d"), port + 1, nChannel, nIntensity);
	m_pLogFileAccessor->KMLOG(SEND, strLog, true);

	return m_vecpCtrlImpl[port]->SetLedLampIntensity(GetDeviceChannel(nChannel), nIntensity, strCommand);

	/*strLog.Format(_T("SetLedLampIntensity. Channel = %d, Intensity = %d"), nChannel, nIntensity);
	m_pLogFileAccessor->KMLOG(SEND, strLog, true);
	strLog.Format(_T("SetLedLampIntensity. Calc. DeviceNum = %d, Calc. Intensity = %d"), GetDeviceNum(nChannel), GetDeviceChannel(nChannel));
	m_pLogFileAccessor->KMLOG(SEND, strLog, true);*/

//	return m_vecpCtrlImpl[GetDeviceNum(nChannel)]->SetLedLampIntensity(GetDeviceChannel(nChannel), nIntensity, strCommand);
}

bool CLightFunction::SetLedLampTurnOnOff(int port, long nChannel, long nIntensity, enumLED_LAMP_CONTROL nCONTROL, CString &strCommand)
{
	CString strLog;
	strLog.Format(_T("SetLedLampTurnOnOff. Device = %d, Channel = %d, Intensity = %d"), port + 1, nChannel, nIntensity);
	m_pLogFileAccessor->KMLOG(SEND, strLog, true);

	return m_vecpCtrlImpl[port]->SetLedLampTurnOnOff(GetDeviceChannel(nChannel), nIntensity, nCONTROL, strCommand);

	//strLog.Format(_T("SetLedLampTurnOnOff. Channel = %d, Intensity = %d, On = %d"), nChannel, nIntensity, nCONTROL);
	//m_pLogFileAccessor->KMLOG(SEND, strLog, true);
	//strLog.Format(_T("SetLedLampTurnOnOff. Calc. DeviceNum = %d, Calc. Intensity = %d"), GetDeviceNum(nChannel), GetDeviceChannel(nChannel));

	//m_pLogFileAccessor->KMLOG(SEND, strLog, true);
	//return m_vecpCtrlImpl[GetDeviceNum(nChannel)]->SetLedLampTurnOnOff(GetDeviceChannel(nChannel), nIntensity, nCONTROL, strCommand);
}

bool CLightFunction::GetLedLampIntensity(int port, long nChannel, CString &strCommand)
{
	CString strLog;
	strLog.Format(_T("GetLedLampIntensity. Device = %d, Channel = %d"), port + 1, nChannel);
	m_pLogFileAccessor->KMLOG(SEND, strLog, true);

	bool bRet = false;
	bRet = m_vecpCtrlImpl[port]->GetLedLampIntensity(GetDeviceChannel(nChannel), strCommand);

	//strLog.Format(_T("GetLedLampIntensity. Channel = %d"), nChannel);
	//m_pLogFileAccessor->KMLOG(SEND, strLog, true);
	//strLog.Format(_T("GetLedLampIntensity. Calc. DeviceNum = %d, Calc. Intensity = %d"), GetDeviceNum(nChannel), GetDeviceChannel(nChannel));
	//m_pLogFileAccessor->KMLOG(SEND, strLog, true);

	//bool bRet = false;
	//bRet = m_vecpCtrlImpl[GetDeviceNum(nChannel)]->GetLedLampIntensity(GetDeviceChannel(nChannel), strCommand);;

	strLog.Format(_T("GetLedLampIntensity. Return : %s"), strCommand);
	m_pLogFileAccessor->KMLOG(RECV, strLog, true);

	return bRet;
}

bool CLightFunction::GetCurLedLampIntensity(int port, long nChannel, long &nIntensity, CString &strCommand)
{
	CString strLog;
	strLog.Format(_T("GetCurLedLampIntensity. Device = %d, Channel = %d"), port + 1, nChannel);
	m_pLogFileAccessor->KMLOG(SEND, strLog, true);

	bool bRet = false;
	bRet = m_vecpCtrlImpl[port]->GetCurLedLampIntensity(GetDeviceChannel(nChannel), nIntensity, strCommand);


	/*strLog.Format(_T("GetCurLedLampIntensity. Channel = %d"), nChannel);
	m_pLogFileAccessor->KMLOG(SEND, strLog, true);
	strLog.Format(_T("GetCurLedLampIntensity. Calc. DeviceNum = %d, Calc. Intensity = %d"), GetDeviceNum(nChannel), GetDeviceChannel(nChannel));
	m_pLogFileAccessor->KMLOG(SEND, strLog, true);

	bool bRet = false;
	bRet = m_vecpCtrlImpl[GetDeviceNum(nChannel)]->GetCurLedLampIntensity(GetDeviceChannel(nChannel), nIntensity, strCommand);*/

	strLog.Format(_T("GetCurLedLampIntensity. Return : %s"), strCommand);
	m_pLogFileAccessor->KMLOG(RECV, strLog, true);

	return bRet;
}

bool CLightFunction::GetLedLampTurnOnOff(int port, long nChannel, CString &strCommand)
{
	CString strLog;
	strLog.Format(_T("GetLedLampTurnOnOff. Device = %d, Channel = %d"), port + 1, nChannel);
	m_pLogFileAccessor->KMLOG(SEND, strLog, true);

	bool bRet = false;
	bRet = m_vecpCtrlImpl[port]->GetLedLampTurnOnOff(GetDeviceChannel(nChannel), strCommand);



	//strLog.Format(_T("GetLedLampTurnOnOff. Channel = %d"), nChannel);
	//m_pLogFileAccessor->KMLOG(SEND, strLog, true);
	//strLog.Format(_T("GetLedLampTurnOnOff. Calc. DeviceNum = %d, Calc. Intensity = %d"), GetDeviceNum(nChannel), GetDeviceChannel(nChannel));
	//m_pLogFileAccessor->KMLOG(SEND, strLog, true);

	//bool bRet = false;
	//bRet = m_vecpCtrlImpl[GetDeviceNum(nChannel)]->GetLedLampTurnOnOff(GetDeviceNum(nChannel), strCommand);

	strLog.Format(_T("GetLedLampTurnOnOff. Return : %s"), strCommand);
	m_pLogFileAccessor->KMLOG(RECV, strLog, true);

	return bRet;
}

bool CLightFunction::WaitLedLampResponce(int port, long nChannel, long nIntensity, enumRES_CHECK_TYPE nTYPE)
{
	CString strLog;
	strLog.Format(_T("WaitLedLampResponce. Device = %d, Channel = %d, Intensity = %d, CheckType = %d"), port + 1, nChannel, nIntensity, nTYPE);
	m_pLogFileAccessor->KMLOG(SEND, strLog, true);

	return m_vecpCtrlImpl[port]->WaitLedLampResponce(GetDeviceChannel(nChannel), nIntensity, nTYPE);


	//strLog.Format(_T("WaitLedLampResponce. Channel = %d, Intensity = %d, CheckType = %d"), nChannel, nIntensity, nTYPE);
	//m_pLogFileAccessor->KMLOG(SEND, strLog, true);
	//strLog.Format(_T("WaitLedLampResponce. Calc. DeviceNum = %d, Calc. Intensity = %d"), GetDeviceNum(nChannel), GetDeviceChannel(nChannel));
	//m_pLogFileAccessor->KMLOG(SEND, strLog, true);

	//return m_vecpCtrlImpl[GetDeviceNum(nChannel)]->WaitLedLampResponce(GetDeviceNum(nChannel), nIntensity, nTYPE);
}

