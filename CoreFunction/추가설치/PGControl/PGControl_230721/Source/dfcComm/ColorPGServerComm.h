#pragma once

typedef	void	(*COLOR_PG_EVENT_HANDLER)(TCHAR*, int, int); //메시지 전달

enum ColorPGItem {
	CPG_WAIT_CLEAR			= -1,
	CPG_WAIT_TURN_ON		= 0,
	CPG_WAIT_TURN_OFF,
	CPG_WAIT_NEXT,
	CPG_WAIT_VOLTAGE,

	CPG_WAIT_MAX,
};

#define PATTERN_ACK		0
#define PATTERN_END		1
#define PATTERN_NG		2

class	AFX_EXT_CLASS	C_ColorPGServerComm : public C_ServerSocket
{
	public:
		C_ColorPGServerComm (COLOR_PG_EVENT_HANDLER pHandler, CString IniPath, int nIndex);
		virtual ~C_ColorPGServerComm ();

	public:
		int m_nRunning;
		int	m_nIndex;	//Socket 통신 순서
		COLOR_PG_EVENT_HANDLER	m_pHandler;

		BOOL			m_bWaitItem[CPG_WAIT_MAX];
		S_TcpIpInfo		m_sInfo;

	protected:
		int				m_nPatternNum;
		CString			m_strPatternName;

	public:
		virtual	void	ReceiveCode (TCHAR* pBuf);

	public:
		void			ConnectCtrl (BOOL bConnect);
		void			ReadIniInfo ();
		void			WriteIniInfo (S_TcpIpInfo *sInfo);
		S_TcpIpInfo		GetIniInfo () { return m_sInfo; };

		void	SetWaitItem (int nItem, BOOL bSet);
		BOOL	GetWaitItem (int nItem);

		//명령
		BOOL	SetPatternAck ();
		BOOL	SetPatternEnd (bool bOk = true);
		BOOL	SendItem (int nItem);

		//응답 처리
		void	WriteComm (LPCTSTR lpszMsg);

		//
		BOOL	GetCommUse () { return m_sInfo.m_bUse; };
		BOOL	GetCommOutOfService () { return m_sInfo.m_bOutOfService; };
		CString	GetIPAddress() { return m_sInfo.m_strIP; };
		int		GetPortNum() { return m_sInfo.m_nPortNum; };

		int		GetExeHextoStr(CString& strData, CStringArray& vecResult);
};

