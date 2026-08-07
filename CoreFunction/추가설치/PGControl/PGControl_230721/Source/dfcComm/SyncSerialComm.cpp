#include "pch.h"
#include "dfcComm.h"
#include "SyncSerialComm.h"

C_SyncSerialComm::C_SyncSerialComm ()
{
	m_pHandler = NULL;
	m_pEvent = new CEvent (FALSE, TRUE);
	m_pEvent->ResetEvent ();
	memset (m_pBuf, 0, sizeof (char) * COMM_QUEUE_SIZE);

	m_bRcvByte = FALSE;
	m_nRcvCnt = 3;
	memset (m_byQueue, 0, sizeof (BYTE) * COMM_QUEUE_SIZE);

	m_nReadLen = 0;
	m_nRepDelay = 200;
}

C_SyncSerialComm::~C_SyncSerialComm ()
{
	if (m_pEvent) {
		m_pEvent->SetEvent ();
		delete m_pEvent;
	}
	m_pEvent = NULL;
}

void	C_SyncSerialComm::OnReceive ()
{
	static BYTE		buf[COMM_QUEUE_SIZE];
	DWORD	dwRead = 0;

	if (m_bRcvByte) {
		while (dwRead = ReadComm (buf, COMM_QUEUE_SIZE)) {
			for (DWORD i = 0; i < dwRead; i++) {
				m_byQueue[m_nReadLen++] = buf[i];

				if (m_nReadLen == m_nRcvCnt) {
					m_nReadLen = 0;
					m_pEvent->SetEvent ();
				}
			}
			buf[0] = 0;
		}
	}
	else {
		while (dwRead = ReadComm (m_pBuf, COMM_QUEUE_SIZE)) {
			for (DWORD i = 0; i < dwRead; i++) {
				if (m_nMustReadBytes > 0 && m_nReadLen == m_nMustReadBytes) {
					m_queue[m_nReadLen] = 0;
					m_nReadLen = 0;
				} else if (m_nMustReadBytes == 0 && (m_pBuf[i] == m_cTail)) {
					m_queue[m_nReadLen] = 0;
					m_nReadLen = 0;
					m_strReply = CString (m_queue);
					m_pEvent->SetEvent ();
				} else {
	#ifdef _UNICODE
					mbtowc (&m_queue[m_nReadLen++], &m_pBuf[i], 1);
	#else
					m_queue[m_nReadLen++] = m_pBuf[i];
	#endif
				}
			}
			m_pBuf[0] = 0;
		}
	}
}

BOOL	C_SyncSerialComm::WaitReply ()
{
	if (WaitForSingleObject (m_pEvent->m_hObject, m_nRepDelay) == WAIT_OBJECT_0) return TRUE;
	return FALSE;
}

DWORD	C_SyncSerialComm::WriteComm (LPVOID pBuf, DWORD dwBytesToWrite)
{
	m_pEvent->ResetEvent ();
	return C_SerialComm::WriteComm (pBuf, dwBytesToWrite);
}

DWORD	C_SyncSerialComm::WriteComm (CString string) 
{
	m_pEvent->ResetEvent ();
	return C_SerialComm::WriteComm (string);
}

DWORD	C_SyncSerialComm::WriteComm (int nVal)
{
	return C_SerialComm::WriteComm (nVal);
}
