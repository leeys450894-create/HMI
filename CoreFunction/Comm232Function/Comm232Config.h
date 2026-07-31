#pragma once

class CIniHelper;
class CComm232Config
{
	CIniHelper *m_pIniHelper;
	CString		m_strCategory;

public:
	CString		m_strModelType;						// COM Model Type		[ WH : white, UV : ultra violet	]
	CString		m_strModelName;						// COM Model Name		[ SPO, CAS						]
    CString		m_strPortID;						// COM Port				[ COM1, COM2...					]
	CString		m_strBaudRate;						// COM Baud rate		[ 9600, 14400...				]
	CString		m_strDataBits;						// COM Data bit
	CString		m_strParity;						// COM Parity bit
	CString		m_strStopBits;						// COM Stop bit
	CString		m_strFlowControl;					// COM Flow control
	bool		m_bCarriageReturn;					// 명령어에 붙여서 보내는 특수 문자 '\r'	: 0x0A
	bool		m_bLineFeed;						// 명령어에 붙여서 보내는 특수 문자 '\n'	: 0x0D
	bool		m_bRecordSaparator;					// 명령어에 붙여서 보내는 문자       rs		: 0x1E
	long		m_nSerialCommHoldTime_msec;			// 명령어에 대한 대기 시간					: 100ms 기본
	bool		m_bResponceCheck;					// 명령어에 대한 응답 확인					: 확인 기본
	long		m_nResponceWaitTime_msec;			// 명령어에 대한 응답 시간					: 100ms 기본
	float		m_fResponceTimeLimit_sec;			// 명령어에 대한 응답 시간					: 30초 기본

public:
	CString	GetParam_ModelType() const;
	CString	GetParam_ModelName() const;
	CString	GetParam_PortID() const;
	CString	GetParam_BaudRate() const;
	CString	GetParam_DataBits() const;
	CString GetParam_Parity() const;
	CString	GetParam_StopBits() const;
	CString	GetParam_FlowControl() const;
	bool	GetParam_CarriageReturn() const;
	bool	GetParam_LineFeed() const;
	bool	GetParam_RecordSaparator() const;
	long	GetParam_SerialCommHoldTime() const;
	bool	GetParam_ResponceCheck() const;
	long	GetParam_ResponceWaitTime() const;
	float	GetParam_ResponceTimeLimit() const;

	void	SetParam_ModelName(CString value);
	void	SetParam_PortID(CString value);
	void	SetParam_BaudRate(CString value);
	void	SetParam_DataBits(CString value);
	void	SetParam_Parity(CString value);
	void	SetParam_StopBits(CString value);
	void	SetParam_FlowControl(CString value);
	void	SetParam_CarriageReturn(bool value);
	void	SetParam_LineFeed(bool value);
	void	SetParam_RecordSaparator(bool value);
	void	SetParam_SerialCommHoldTime(long value);
	void	SetParam_ResponceCheck(bool value);
	void	SetParam_ResponceWaitTime(long value);
	

	// 특수한 목적으로 사용하는 함수
	BYTE	GetSerialStringToValue(CString value) const;

public:
	BOOL	LinkIniFile(bool bSave);

public:
    CComm232Config(CString strFilePath, CString strCategory);
    ~CComm232Config();
};