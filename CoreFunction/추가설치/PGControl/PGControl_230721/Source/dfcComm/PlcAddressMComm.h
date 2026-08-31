#pragma once

typedef	void	(*PLC_EVENT_HANDLER)(TCHAR*, int, int); //메시지 전달

class	AFX_EXT_CLASS	C_PlcAddressMComm : public C_ClientSocket
{

	public:
		C_PlcAddressMComm (PLC_EVENT_HANDLER pHandler, CString IniPath, BOOL bRead);
		virtual ~C_PlcAddressMComm ();

	public:
		int				  m_nRunning;
		BOOL			  m_bRead;
		PLC_EVENT_HANDLER m_pHandler;
		S_TcpIpInfo		  m_sInfo;
		CString			  m_strOldData;
		BOOL			  m_bBitData[PLC_M_END_BIT - PLC_M_BEGIN_BIT];

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
		BOOL	SetBit (int nAddress, int nVal, int nBitCnt = 1);
		BOOL	GetBit (int nAddress);

		//
		void	WriteComm (LPCTSTR lpszMsg);

		BOOL	GetCommUse () { return m_sInfo.m_bUse; };
		BOOL	GetCommOutOfService () { return m_sInfo.m_bOutOfService; };
};


