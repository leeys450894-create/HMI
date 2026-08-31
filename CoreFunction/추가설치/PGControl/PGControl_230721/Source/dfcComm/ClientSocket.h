#pragma once

#include <WinSock2.h>

class	AFX_EXT_CLASS	C_ClientSocket : public CSocket, public C_IniFile
{
	public:
		CString	m_nameHost;
		TCHAR	m_queue[COMM_QUEUE_SIZE];
		int		m_nReadLen;

	protected:
		BOOL	m_bConnected;
		CString	m_strDelim;
		BOOL	m_bTimeout;
		DWORD   m_dwTimeout;
		DWORD	m_dwTickCount;

	public:
				 C_ClientSocket ();
		virtual ~C_ClientSocket ();

	public :
		BOOL	IsConnected ()	{return m_bConnected;}
		BOOL	IsTimeout	()	{return m_bTimeout;}
		BOOL	OpenPort (CString nameHost, UINT nPort ,BOOL bSocketCreate = TRUE);
		void	SetConnect (BOOL bConnect)	{m_bConnected = bConnect;}
		void	Send (LPCTSTR lpszMsg);
		void	SendNoDelim (LPCTSTR lpszMsg);
		void	Close ();
		void	ChangeDelim (CString strDelim) {m_strDelim = strDelim;}

	protected:
		virtual BOOL	OnMessagePending ();
		virtual void	CancelBlockingCall ();
		virtual int		Send (const void* lpBuf, int nBufLen, int nFlags = 0);
		virtual void	StringToken (LPCTSTR lpszData, LPCTSTR lpszToken, CStringArray &strings);
		virtual void	OnReceive (int nErrorCode);
		virtual void	OnClose (int nErrorCode);
		virtual void	OnConnect (int nErrorCode);
		virtual	void	ReceiveCode (TCHAR *pBuf) = 0;
};

