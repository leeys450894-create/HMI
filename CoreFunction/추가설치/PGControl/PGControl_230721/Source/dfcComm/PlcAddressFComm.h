#pragma once

typedef	void	(*PLC_EVENT_HANDLER)(TCHAR*, int, int); //메시지 전달

class	AFX_EXT_CLASS	C_PlcAddressFComm : public C_ClientSocket
{
	public:
		C_PlcAddressFComm (PLC_EVENT_HANDLER pHandler, CString IniPath);
		virtual ~C_PlcAddressFComm ();

	public:
		int				  m_nRunning;
		PLC_EVENT_HANDLER m_pHandler;
		S_TcpIpInfo		  m_sInfo;

		BOOL			  m_bBitData[PLC_F_END_BIT - PLC_F_BEGIN_BIT];

	public:
		virtual	void	ReceiveCode (TCHAR* pBuf);
		virtual void	OnClose (int nErrorCode);
		virtual void	OnConnect (int nErrorCode);

	public:
		void			ConnectCtrl (BOOL bConnect);
		void			ReadIniInfo ();
		S_TcpIpInfo		GetIniInfo () { return m_sInfo; };

		//명령
		BOOL	QueryBitData ();
		BOOL	GetBit (int nAddress);

		//
		void	WriteComm (LPCTSTR lpszMsg);

		BOOL	GetCommUse () { return m_sInfo.m_bUse; };
		BOOL	GetCommOutOfService () { return m_sInfo.m_bOutOfService; };
};



