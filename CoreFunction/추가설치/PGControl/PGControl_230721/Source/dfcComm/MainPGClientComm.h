#pragma once

typedef	void	(*MAIN_PG_EVENT_HANDLER)(TCHAR*, int, int); //메시지 전달

class	AFX_EXT_CLASS	C_MainPGClientComm : public C_ClientSocket
{
	public:
		C_MainPGClientComm (MAIN_PG_EVENT_HANDLER pHandler, CString IniPath, int nIndex);
		virtual ~C_MainPGClientComm ();

	public:
		int m_nRunning;
		int	m_nIndex;	//Socket 통신 순서
		MAIN_PG_EVENT_HANDLER	m_pHandler;

		S_TcpIpInfo		m_sInfo;

	public:
		virtual	void	ReceiveCode (TCHAR* pBuf);
		virtual void	OnClose (int nErrorCode);
		virtual void	OnConnect (int nErrorCode);

	protected:
		virtual void	OnReceive (int nErrorCode);

	public:
		void			ConnectCtrl (BOOL bConnect);
		void			ReadIniInfo ();
		void			WriteIniInfo (S_TcpIpInfo *sInfo);
		S_TcpIpInfo		GetIniInfo () { return m_sInfo; };

		//명령

		//응답 처리
		void	WriteComm (LPCTSTR lpszMsg);

		//
		BOOL	GetCommUse () { return m_sInfo.m_bUse; };
		BOOL	GetCommOutOfService () { return m_sInfo.m_bOutOfService; };
		CString	GetIPAddress() { return m_sInfo.m_strIP; };
		int		GetPortNum() { return m_sInfo.m_nPortNum; };
};

