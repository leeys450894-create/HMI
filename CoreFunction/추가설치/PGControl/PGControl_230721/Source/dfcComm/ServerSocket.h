#pragma	once

class	C_ChildSocket;
class	AFX_EXT_CLASS	C_ServerSocket : public CSocket, public C_IniFile
{
	protected:
		C_ChildSocket	*m_pChild;
		CString	m_strDelim;

	public:
		C_ServerSocket ();
		virtual ~C_ServerSocket ();

		BOOL	IsConnected () {return m_pChild != 0;}
		void	Create (UINT nPort);
		void	Send (LPCTSTR lpszMsg);
		void	SendNoDelim (LPCTSTR lpszMsg);
		void	Destroy (CSocket *pChild);
		void	ChangeDelim(CString strDelim) { m_strDelim = strDelim; }

	public:
		virtual void	OnAccept (int nErrorCode);
		virtual	void	ReceiveCode (TCHAR *pBuf) = 0;
};


class	AFX_EXT_CLASS	C_ChildSocket : public CSocket
{
	public:
		C_ServerSocket	*m_pServer;
		TCHAR	m_queue[COMM_QUEUE_SIZE];
		int		m_nReadLen;

	public:
		C_ChildSocket (C_ServerSocket *pServer);
		virtual ~C_ChildSocket ();

	protected:
		virtual void	OnReceive (int nErrorCode);
		virtual void	OnClose (int nErrorCode);
};
