//#include "stdafx.h"
#include "pch.h"
#include "LightControlMgr.h"

#include "common\IniHelper.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CLightControlMgr::CLightControlMgr(CString strAppPath, CString strConfigPath, CString strLogPath)
    : m_pIniHelper(new CIniHelper)
{
    SetName(_T("LIGHT_CONTROL_MGR"));

	const CString strFilePath = strConfigPath + _T("\\CoreFunction\\CoreFunction.ini");
    m_pIniHelper->SetFilePath(strFilePath);

  //  LinkIniFile(false);

	//----------------------------------------------------------------------
	//----------------------------------------------------------------------
//	const CString strLogFilePath = strAppPath + _T("\\Logs");

	const CString strLogFilePath = strLogPath + _T("Control\\");
	MakeDirPath(strLogFilePath);

	m_pFunction = new CLightFunction(strAppPath, strFilePath, strLogFilePath);

	InitDevice(0);
	InitDevice(1);
}

CLightControlMgr::~CLightControlMgr()
{
    Finalize();
}

bool CLightControlMgr::LinkIniFile(bool bSave)
{
    m_pIniHelper->Link(bSave, GetName(), _T("USE"),				&m_bUse,				true);
    m_pIniHelper->Link(bSave, GetName(), _T("OUT_OF_SERVICE"),	&m_bOutOfService,		false);
    m_pIniHelper->Link(bSave, GetName(), _T("DEVICE_TYPE"),		&m_strControlType,		(CString)_T("SPO"));

    return m_pIniHelper->IsReadEnable();
}

void CLightControlMgr::Finalize()
{
    //----------------------------------------------------------------------
    // 데이터 삭제
    //----------------------------------------------------------------------
	if(m_pFunction)
	{
		delete m_pFunction;
		m_pFunction = nullptr;
	}
	
	if(m_pIniHelper)
    {
        delete m_pIniHelper;
        m_pIniHelper = nullptr;
    }
}




bool CLightControlMgr::InitDevice(int port)
{
	return 	m_pFunction->InitDevice(port);
}

bool CLightControlMgr::CloseDevice(int port)
{
	return 	m_pFunction->CloseDevice(port);
}

CString	CLightControlMgr::GetDeviceName(int port)
{
	return m_pFunction->GetDeviceName(port);
}

bool CLightControlMgr::IsConnected(int port)
{
	return m_pFunction->IsConnected(port);
}

bool CLightControlMgr::IsOutOfService(int port)
{
	return m_pFunction->IsOutOfService(port);
}

long CLightControlMgr::GetCount()
{
	return m_pFunction->GetCount();
}

// LED 조명 제어 전용 함수 - 시퀀스 연동
bool CLightControlMgr::LedLight_Channel_TurnOn(int port, long nChannel, long nIntensity)
{
	CCriticalSectionSync single(&m_cs_on, TRUE);

	return m_pFunction->LedLight_Channel_TurnOn(port, nChannel, nIntensity);
}

bool CLightControlMgr::LedLight_Channel_TurnOff(int port, long nChannel, long nIntensity)
{
	CCriticalSectionSync single(&m_cs_off, TRUE);

	return m_pFunction->LedLight_Channel_TurnOff(port, nChannel, nIntensity);
}
    
// LED 조명 제어 전용 함수
bool CLightControlMgr::SetLedLampIntensity(int port, long nChannel, long nIntensity, CString &strCommand)
{
	CCriticalSectionSync single(&m_cs_set, TRUE);

	if (port == 0)
	{
		if (nIntensity > 0)
			CWindowEventHandler::GetInstance()->PostMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_LAMP_TIME, nChannel, true);
		else
			CWindowEventHandler::GetInstance()->PostMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_LAMP_TIME, nChannel, false);
	}
	
	return m_pFunction->SetLedLampIntensity(port, nChannel, nIntensity, strCommand);
}

bool CLightControlMgr::SetLedLampTurnOnOff(int port, long nChannel, long nIntensity, enumLED_LAMP_CONTROL nCONTROL, CString &strCommand)
{
	CCriticalSectionSync single(&m_cs_set, TRUE);

	return m_pFunction->SetLedLampTurnOnOff(port, nChannel, nIntensity, nCONTROL, strCommand);
}

bool CLightControlMgr::GetLedLampIntensity(int port, long nChannel, CString &strCommand)
{
	CCriticalSectionSync single(&m_cs_get, TRUE);

	return m_pFunction->GetLedLampIntensity(port, nChannel, strCommand);
}

bool CLightControlMgr::GetCurLedLampIntensity(int port, long nChannel, long &nIntensity, CString &strCommand)
{
	CCriticalSectionSync single(&m_cs_get, TRUE);

	return m_pFunction->GetCurLedLampIntensity(port, nChannel, nIntensity, strCommand);
}

bool CLightControlMgr::GetLedLampTurnOnOff(int port, long nChannel, CString &strCommand)
{
	CCriticalSectionSync single(&m_cs_get, TRUE);

	return m_pFunction->GetLedLampTurnOnOff(port, nChannel, strCommand);
}

bool CLightControlMgr::WaitLedLampResponce(int port, long nChannel, long nIntensity, enumRES_CHECK_TYPE nTYPE)
{
	CCriticalSectionSync single(&m_cs_res, TRUE);

	return m_pFunction->WaitLedLampResponce(port, nChannel, nIntensity, nTYPE);
}

CLogFileAccessor* CLightControlMgr::GetLogFileAccessorPtr()
{
	return m_pFunction->GetLogFileAccessorPtr();
}