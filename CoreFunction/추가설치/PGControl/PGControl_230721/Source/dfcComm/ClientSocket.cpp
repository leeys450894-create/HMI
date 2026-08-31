#include "pch.h"
#include "dfcComm.h"
#include "ClientSocket.h"

C_ClientSocket::C_ClientSocket ()
{
	m_bConnected  = FALSE;
	m_strDelim    = _T("\r\n");
	m_dwTimeout   = 1000;
	m_dwTickCount = 0;
	m_nReadLen	  = 0;

	memset (m_queue, 0, COMM_QUEUE_SIZE);
}

C_ClientSocket::~C_ClientSocket ()
{
	if (IsConnected ()) Close ();
}

BOOL	C_ClientSocket::OpenPort (CString nameHost, UINT nPort, BOOL bSocketCreate /*= TRUE*/)
{
	m_dwTickCount = GetTickCount ();

	if (bSocketCreate)
		CSocket::Create ();

	if (!Connect (nameHost, nPort)) {
		Close ();
		return m_bConnected = FALSE;
	} 
	else {
		m_nReadLen = 0;
		return m_bConnected = TRUE;
	}
}

void	C_ClientSocket::Send (LPCTSTR lpszMsg)
{
	if (!IsConnected ()) return;

	CString	string = lpszMsg;
	SendNoDelim (string + m_strDelim);
}

void	C_ClientSocket::SendNoDelim (LPCTSTR lpszMsg)
{
	if (!IsConnected ()) return;

	CString	string = lpszMsg;
	
#ifdef _UNICODE
	char	mbBuf[COMM_QUEUE_SIZE];
	wcstombs (mbBuf, (LPCTSTR)string, string.GetLength ());
	CSocket::Send (mbBuf, string.GetLength ());
#else
	CSocket::Send ((LPCTSTR)string, string.GetLength ());
#endif
}

void	C_ClientSocket::Close ()
{
	memset (m_queue, 0, COMM_QUEUE_SIZE);
	m_nReadLen = 0;
	m_bConnected = FALSE;

	CSocket::Close ();
}

void	C_ClientSocket::OnReceive (int nErrorCode)
{
	static	char	buf[COMM_QUEUE_SIZE];
	int		nRead = Receive (buf, COMM_QUEUE_SIZE);
	if (!nRead) return;

	for (int i = 0; i < nRead; i++) {
		if (buf[i] == '\r') {

			m_queue[m_nReadLen] = 0;
			m_nReadLen = 0;
			ReceiveCode (m_queue);
		}
		else if (buf[i] == '\n') {
			m_nReadLen = 0;
		} 
		else {
#ifdef _UNICODE
			mbtowc (&m_queue[m_nReadLen++], &buf[i], 1);
#else
			m_queue[m_nReadLen++] = buf[i];
#endif
		}
	}

	CSocket::OnReceive (nErrorCode);
}

void	C_ClientSocket::OnClose (int nErrorCode)
{
	m_bConnected = FALSE;

	CSocket::OnClose (nErrorCode);
}

int		C_ClientSocket::Send(const void* lpBuf, int nBufLen, int nFlags)
{
	return CSocket::Send (lpBuf, nBufLen , nFlags);
}

void	C_ClientSocket::OnConnect (int nErrorCode)
{
	CSocket::OnConnect (nErrorCode);
}

BOOL	C_ClientSocket::OnMessagePending ()
{
	DWORD dwDiff = GetTickCount () - m_dwTickCount;
	if (dwDiff > m_dwTimeout) {
		m_bTimeout = FALSE;
		CancelBlockingCall ();
	}
	return TRUE;
}

void	C_ClientSocket::CancelBlockingCall ()
{
	CSocket::CancelBlockingCall ();
}

void	C_ClientSocket::StringToken (LPCTSTR lpszData, LPCTSTR lpszToken, CStringArray &strings)
{
	strings.RemoveAll ();
	CString	 string(lpszData);
	CString	 resToken;
	int		 nPos = 0;	
	resToken = string.Tokenize(lpszToken, nPos);
	while (!resToken.IsEmpty ()) {
		strings.Add (resToken.Trim ());
		resToken = string.Tokenize(lpszToken, nPos);
	};   	
}