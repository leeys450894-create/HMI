#include "pch.h"
#include "dfcComm.h"
#include "ServerSocket.h"

C_ServerSocket::C_ServerSocket ()
{
	m_pChild = 0;
	m_strDelim = _T("\r");
}

C_ServerSocket::~C_ServerSocket ()
{
	if (m_pChild) Destroy (m_pChild);
}

void	C_ServerSocket::OnAccept (int nErrorCode)
{
	CSocket::OnAccept (nErrorCode);

	if (m_pChild) {
		m_pChild->ShutDown ();
		m_pChild->Close ();
		delete m_pChild;
		m_pChild = 0;
	}

	m_pChild = new C_ChildSocket (this);
	if (!Accept (*m_pChild)) {
		delete m_pChild;
		m_pChild = 0;
		return;
	}
}

void	C_ServerSocket::Create (UINT nPort)
{
	if (!CSocket::Create (nPort)) return;

	Listen ();
}

void	C_ServerSocket::Send (LPCTSTR lpszMsg)
{
	if (!IsConnected ()) return;

	CString	string = lpszMsg;
	SendNoDelim (string + m_strDelim);
}

void	C_ServerSocket::SendNoDelim (LPCTSTR lpszMsg)
{
	if (!IsConnected ()) return;

	CString	string = lpszMsg;
	
#ifdef _UNICODE
	char	mbBuf[COMM_QUEUE_SIZE];
	wcstombs (mbBuf, (LPCTSTR)string, string.GetLength ());
	m_pChild->Send (mbBuf, string.GetLength ());
#else
	m_pChild->Send ((LPCTSTR)string, string.GetLength ());
#endif
}

void	C_ServerSocket::Destroy (CSocket *pChild)
{
	m_pChild->ShutDown ();
	m_pChild->Close ();
	delete m_pChild;
	m_pChild = 0;
}

//	End of C_ServerSocket


//-------------------------------------------------------------------------------------------------
//	C_ChildSocket
//-------------------------------------------------------------------------------------------------
C_ChildSocket::C_ChildSocket (C_ServerSocket *pServer)
{
	m_pServer = pServer;
	memset (m_queue, 0, COMM_QUEUE_SIZE);
	m_nReadLen = 0;
}

C_ChildSocket::~C_ChildSocket ()
{
}

void	C_ChildSocket::OnReceive (int nErrorCode)
{
	static	char	buf[COMM_QUEUE_SIZE];
	int		nRead = Receive (buf, COMM_QUEUE_SIZE);
	if (!nRead) return;

	for (int i = 0; i < nRead; i++) {
	//	if (buf[i] == 0x03 || buf[i] == '\r') {
	//		if (buf[i] == '\r') {

		if( i + 1 == nRead )
		{ 
			m_pServer->ReceiveCode (m_queue);

			m_queue[m_nReadLen] = 0;
			m_nReadLen = 0;

			break;
		}
		else if (buf[i] == '\n') {
			m_nReadLen = 0;
		}
		else {

#ifdef _UNICODE

			mbtowc(&m_queue[m_nReadLen++], &buf[i], 1);
#else
			m_queue[m_nReadLen++] = buf[i];
#endif
		}
	}

	CSocket::OnReceive (nErrorCode);
}

//void	C_ChildSocket::OnReceive (int nErrorCode)
//{
//	static	char	buf[COMM_QUEUE_SIZE];
//	int		nRead = Receive (buf, COMM_QUEUE_SIZE);
//	if (!nRead) return;
//
//	for (int i = 0; i < nRead; i++) {
//
//#ifdef _UNICODE
//		mbtowc (&m_queue[m_nReadLen++], &buf[i], 1);
//#else
//		m_queue[m_nReadLen++] = buf[i];
//#endif
//		if (buf[i] == 0x03 || buf[i] == '\r') {
//			m_queue[m_nReadLen] = 0;
//			m_nReadLen = 0;
//			m_pServer->ReceiveCode (m_queue);
//		}
//	}
//
//	CSocket::OnReceive (nErrorCode);
//}

void	C_ChildSocket::OnClose (int nErrorCode)
{
	m_pServer->ReceiveCode (_T("Client Closed"));

	m_pServer->Destroy (this);
	CSocket::OnClose (nErrorCode);
}
//	End of C_ChildSocket