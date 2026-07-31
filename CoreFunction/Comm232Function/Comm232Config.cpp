#include "pch.h"
#include "Comm232Config.h"

#include "..\common\IniHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CComm232Config::CComm232Config(CString strFilePath, CString strCategory)
	: m_pIniHelper(new CIniHelper)
	, m_strCategory(strCategory)
{	
	m_pIniHelper->SetFilePath(strFilePath);

	LinkIniFile(false);
}

CComm232Config::~CComm232Config()
{
	if(m_pIniHelper)
	{
		delete m_pIniHelper;
		m_pIniHelper = nullptr;
	}
}

BOOL CComm232Config::LinkIniFile(bool bSave)
{
	//-----------------------------------------------------------------------
    // 파라미터 연동
    //-----------------------------------------------------------------------
	m_pIniHelper->Link(bSave, m_strCategory,	_T("RS232_MODEL_TYPE"),				&m_strModelType,				(CString)_T("WH"));
	m_pIniHelper->Link(bSave, m_strCategory,	_T("RS232_MODEL_NAME"),				&m_strModelName,				(CString)_T("CAS"));
	m_pIniHelper->Link(bSave, m_strCategory,	_T("RS232_PORTID"),					&m_strPortID,					(CString)_T("COM1"));
	m_pIniHelper->Link(bSave, m_strCategory,	_T("RS232_BAUD_RATE"),				&m_strBaudRate,					(CString)_T("9600"));
	m_pIniHelper->Link(bSave, m_strCategory,	_T("RS232_DATA_BIT"),				&m_strDataBits,					(CString)_T("8"));
	m_pIniHelper->Link(bSave, m_strCategory,	_T("RS232_PARITY"),					&m_strParity,					(CString)_T("NOPARITY"));
	m_pIniHelper->Link(bSave, m_strCategory,	_T("RS232_STOP_BIT"),				&m_strStopBits,					(CString)_T("ONESTOPBIT"));
	m_pIniHelper->Link(bSave, m_strCategory,	_T("RS232_FLOW_CONTROL"),			&m_strFlowControl,				(CString)_T("none"));
	m_pIniHelper->Link(bSave, m_strCategory,	_T("RS232_CR"),						&m_bCarriageReturn,				true);
	m_pIniHelper->Link(bSave, m_strCategory,	_T("RS232_LF"),						&m_bLineFeed,					false);
	m_pIniHelper->Link(bSave, m_strCategory,	_T("RS232_RS"),						&m_bRecordSaparator,			false);
	m_pIniHelper->Link(bSave, m_strCategory,	_T("RS232_SERIAL_COMM_HOLD_TIME"),	&m_nSerialCommHoldTime_msec,	(long)100);
	m_pIniHelper->Link(bSave, m_strCategory,	_T("RS232_RESPONCE_CHECK"),			&m_bResponceCheck,				false);
	m_pIniHelper->Link(bSave, m_strCategory,	_T("RS232_RESPONCE_WAIT_TIME"),		&m_nResponceWaitTime_msec,		(long)100);
	m_pIniHelper->Link(bSave, m_strCategory,	_T("RS232_RESPONCE_TIME_LIMIT"),	&m_fResponceTimeLimit_sec,		30.f);

	return m_pIniHelper->IsReadEnable();
}

//-------------------------------------------------------------------
// Get parameter
//-------------------------------------------------------------------
CString	CComm232Config::GetParam_ModelType() const
{
	return m_strModelType;
}

CString	CComm232Config::GetParam_ModelName() const
{
	return m_strModelName;
}

CString	CComm232Config::GetParam_PortID() const
{
	return m_strPortID;
}

CString	CComm232Config::GetParam_BaudRate() const
{
	return m_strBaudRate;
}

CString	CComm232Config::GetParam_DataBits() const
{
	return m_strDataBits;
}

CString CComm232Config::GetParam_Parity() const
{
	return m_strParity;
}

CString	CComm232Config::GetParam_StopBits() const
{
	return m_strStopBits;
}

CString	CComm232Config::GetParam_FlowControl() const
{
	return m_strFlowControl;
}

bool CComm232Config::GetParam_CarriageReturn() const
{
	return m_bCarriageReturn;
}

bool CComm232Config::GetParam_LineFeed() const
{
	return m_bLineFeed;
}

bool CComm232Config::GetParam_RecordSaparator() const
{
	return m_bRecordSaparator;
}

long CComm232Config::GetParam_SerialCommHoldTime() const
{
	return m_nSerialCommHoldTime_msec;
}

bool CComm232Config::GetParam_ResponceCheck() const
{
	return m_bResponceCheck;
}

long CComm232Config::GetParam_ResponceWaitTime() const
{
	return m_nResponceWaitTime_msec;
}

float CComm232Config::GetParam_ResponceTimeLimit() const
{
	return m_fResponceTimeLimit_sec;
}


//-------------------------------------------------------------------
// Set parameter
//-------------------------------------------------------------------
void CComm232Config::SetParam_ModelName(CString value)
{
	m_strModelName = value;
}

void CComm232Config::SetParam_PortID(CString value)
{
	m_strPortID = value;
}

void CComm232Config::SetParam_BaudRate(CString value)
{
	m_strBaudRate = value;
}

void CComm232Config::SetParam_DataBits(CString value)
{
	m_strDataBits = value;
}

void CComm232Config::SetParam_Parity(CString value)
{
	m_strParity = value;
}

void CComm232Config::SetParam_StopBits(CString value)
{
	m_strStopBits = value;
}

void CComm232Config::SetParam_FlowControl(CString value)
{
	m_strFlowControl = value;
}

void CComm232Config::SetParam_CarriageReturn(bool value)
{
	m_bCarriageReturn = value;
}

void CComm232Config::SetParam_LineFeed(bool value)
{
	m_bLineFeed = value;
}

void CComm232Config::SetParam_RecordSaparator(bool value)
{
	m_bRecordSaparator = value;
}

void CComm232Config::SetParam_SerialCommHoldTime(long value)
{
	m_nSerialCommHoldTime_msec = value;
}

void CComm232Config::SetParam_ResponceCheck(bool value)
{
	m_bResponceCheck = value;
}

void CComm232Config::SetParam_ResponceWaitTime(long value)
{
	m_nResponceWaitTime_msec = value;
}


//-------------------------------------------------------------------------
// 특수한 목적으로 사용하는 함수
//-------------------------------------------------------------------------
BYTE CComm232Config::GetSerialStringToValue(CString value) const
{
	if(value == _T("NOPARITY"))		return 0;
	if(value == _T("ODDPARITY"))	return 1;
	if(value == _T("EVENPARITY"))	return 2;
	if(value == _T("MARKPARITY"))	return 3;
	if(value == _T("SPACEPARITY"))	return 4;

	if(value == _T("ONESTOPBIT"))	return 0;
	if(value == _T("ONE5STOPBITS"))	return 1;
	if(value == _T("TWOSTOPBITS"))	return 2;

	return 0;
}
