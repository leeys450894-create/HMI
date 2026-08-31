#include "pch.h"
#include "dfcComm.h"
#include "Serialcomm.h"

///////////////////////////////////////////////////////////////////////////////
DWORD	WINAPI	ThreadWatchComm (LPVOID lpParam)
{
	OVERLAPPED	os;
	memset (&os, 0, sizeof (OVERLAPPED));
	if (!(os.hEvent = CreateEvent (NULL, TRUE, FALSE, NULL))) return FALSE;

	C_SerialComm	*pComm = (C_SerialComm*)lpParam;
	if (!SetCommMask (pComm->m_hComm, EV_RXCHAR)) {
		CloseHandle (os.hEvent);
		return FALSE;
	}

	DWORD	dwEvent = 0;
	while (pComm->GetConnected()) {
		dwEvent = 0;
		if (!WaitCommEvent (pComm->m_hComm, &dwEvent, NULL)) break;


		if ((dwEvent & EV_RXCHAR) == EV_RXCHAR) {
			pComm->m_bReceived = TRUE;
			pComm->OnReceive ();
		}
	}	
	
	CloseHandle (os.hEvent);
	pComm->m_hThread = 0;
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////


C_SerialComm::C_SerialComm ()
{
	m_hComm = 0;
	m_bConnected = FALSE;
	m_bReceived = FALSE;
	memset (&m_osRead, 0, sizeof (OVERLAPPED));
	memset (&m_osWrite, 0, sizeof (OVERLAPPED));
	m_hThread = 0;

	memset (m_queue, 0, COMM_QUEUE_SIZE);
	memset (m_mbBuf, 0, 1024);
	m_nReadLen = 0;
	m_strDelim = _T("\r");
	m_cTail = _T('\r');
	m_nMustReadBytes = 0;
}

C_SerialComm::~C_SerialComm ()
{
	if (m_bConnected) ClosePort ();

	while (m_hThread) {
		Sleep (0);
	}
	m_hThread = NULL;
}

BOOL	C_SerialComm::OpenPort (CString string)
{
	int		nPos = string.Find (':');
	if (nPos == -1) return FALSE;

	int		nBaudRate = 0;
	TCHAR	cParity = 0;
	int		nByteSize = 0;
	int		nStopBits = 0;
	_stscanf (string.Mid (nPos+1), _T("%d,%c,%d,%d"), &nBaudRate, &cParity, &nByteSize, &nStopBits);
	return OpenPort (string.Left (nPos), nBaudRate, cParity, nByteSize, nStopBits);
}

BOOL	C_SerialComm::OpenPort (CString section, CString key)
{
	CString	string = GetProfileString (section, key);
	int		nPos = string.Find (':');
	if (nPos == -1) return FALSE;

	int		nBaudRate = 0;
	TCHAR	cParity = 0;
	int		nByteSize = 0;
	int		nStopBits = 0;
	_stscanf (string.Mid (nPos+1), _T("%d,%c,%d,%d"), &nBaudRate, &cParity, &nByteSize, &nStopBits);
	return OpenPort (string.Left (nPos), nBaudRate, cParity, nByteSize, nStopBits);
}

BOOL	C_SerialComm::OpenPort (CString portName, int baudRate, TCHAR cParity, int nByteSize, int nStopBits)
{
	m_bConnected = FALSE;
	if (portName.GetLength () > 4) portName = _T("\\\\.\\") + portName;

	m_osRead.Offset = 0;
	m_osRead.OffsetHigh = 0;
	if (!(m_osRead.hEvent = CreateEvent (NULL, TRUE, FALSE, NULL)))	return FALSE;

	m_osWrite.Offset = 0;
	m_osWrite.OffsetHigh = 0;
	if (!(m_osWrite.hEvent = CreateEvent (NULL, TRUE, FALSE, NULL))) return FALSE;
	
	m_hComm = CreateFile (portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 
						  FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
	if (m_hComm == (HANDLE) -1) return FALSE;

	SetCommMask (m_hComm, EV_RXCHAR);	
	SetupComm (m_hComm, 4096, 4096);	
	PurgeComm (m_hComm, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR);

	COMMTIMEOUTS	timeouts;
	timeouts.ReadIntervalTimeout = MAXDWORD;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.ReadTotalTimeoutConstant = 100;
	timeouts.WriteTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 1000;
	SetCommTimeouts (m_hComm, &timeouts);

	DCB		dcb;
	dcb.DCBlength = sizeof (DCB);
	GetCommState (m_hComm, &dcb);
	dcb.BaudRate = baudRate;
	if		(cParity == 'e') dcb.Parity = EVENPARITY;
	else if (cParity == 'o') dcb.Parity = ODDPARITY;
	else					 dcb.Parity = NOPARITY;
	dcb.ByteSize = nByteSize;
	if		(nStopBits == 1) dcb.StopBits = ONESTOPBIT;
	else if (nStopBits == 2) dcb.StopBits = TWOSTOPBITS;
	else					 dcb.StopBits = ONE5STOPBITS;

	if (!SetCommState (m_hComm, &dcb)) return FALSE;
	else {
		m_bConnected = TRUE;
		m_hThread = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE)ThreadWatchComm, this, 0, 0);

		if (!m_hThread) {
			ClosePort ();
			return FALSE;
		}
	}

	return TRUE;
}
	
void	C_SerialComm::ClosePort ()
{
	m_bConnected = FALSE;

	SetCommMask (m_hComm, 0);
	PurgeComm (m_hComm, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR);
	CloseHandle (m_hComm);
	CloseHandle (m_osRead.hEvent);
	CloseHandle (m_osWrite.hEvent);
}

DWORD	C_SerialComm::ReadComm (LPVOID pBuf, DWORD dwBytesToRead)
{
	DWORD	dwRead = 0, dwErrorFlags = 0;
	COMSTAT	comstat;

	ClearCommError (m_hComm, &dwErrorFlags, &comstat);
	if (comstat.cbInQue == 0) return 0;

	dwRead = comstat.cbInQue;
	if (dwBytesToRead > dwRead) dwBytesToRead = dwRead;
	
	if (!ReadFile (m_hComm, pBuf, dwBytesToRead, &dwRead, &m_osRead)) {
		if (GetLastError () == ERROR_IO_PENDING)	{
			while (!GetOverlappedResult (m_hComm, &m_osRead, &dwRead, TRUE)) {
				if (GetLastError () != ERROR_IO_INCOMPLETE)	{
					ClearCommError (m_hComm, &dwErrorFlags, &comstat);
					break;
				}
			}
		} else {
			dwRead = 0;
			ClearCommError (m_hComm, &dwErrorFlags, &comstat);
		}
	}
	return dwRead;
}

DWORD	C_SerialComm::WriteComm (LPVOID pBuf, DWORD dwBytesToWrite)
{
	DWORD	dwWritten = 0;

	BOOL	bWrite = WriteFile (m_hComm, pBuf, dwBytesToWrite, &dwWritten, &m_osWrite);

	if (!bWrite) {
		DWORD	dwErrorFlags = 0;
		COMSTAT	comstat;
		if (GetLastError () == ERROR_IO_PENDING) {
			while (!GetOverlappedResult (m_hComm, &m_osWrite, &dwWritten, TRUE)) {
				if (GetLastError () != ERROR_IO_INCOMPLETE) {
					ClearCommError (m_hComm, &dwErrorFlags, &comstat);
					break;
				}
			}
		} else {
			dwWritten = 0;
			ClearCommError (m_hComm, &dwErrorFlags, &comstat);
		}
	}
	return dwWritten;
}

DWORD	C_SerialComm::WriteComm (TCHAR ch)
{
#ifdef _UNICODE
	wctomb (m_mbBuf, ch);
	return WriteComm (m_mbBuf, 1);
#else
	return WriteComm (&ch, 1);
#endif
}

DWORD	C_SerialComm::WriteComm (int nVal)
{
	return WriteComm (&nVal, 1);
}

DWORD	C_SerialComm::WriteComm (CString string, BOOL bIncludeDelim)
{
	if (bIncludeDelim) string += m_strDelim;

#ifdef _UNICODE
	memset (m_mbBuf, 0, 1024);
	wcstombs (m_mbBuf, (LPCTSTR)string, string.GetLength ());
	return WriteComm (m_mbBuf, string.GetLength ());
#else
	return WriteComm ((LPTSTR)(LPCTSTR)string, string.GetLength ());
#endif
}

void	C_SerialComm::OnReceive ()
{
	static	char	buf[COMM_QUEUE_SIZE];
	DWORD	dwRead = 0;

	while (dwRead = ReadComm (buf, COMM_QUEUE_SIZE)) {
		for (DWORD i = 0; i < dwRead; i++) {
			if (m_nMustReadBytes == 0) {
				if (buf[i] == '\r' || buf[i] == '\n') {
					m_queue[m_nReadLen] = 0;
					m_nReadLen = 0;
					ReceiveCode (m_queue);
				} else {
#ifdef _UNICODE
					mbtowc (&m_queue[m_nReadLen++], &buf[i], 1);
#else
					m_queue[m_nReadLen++] = buf[i];
#endif
				}
			} else {
#ifdef _UNICODE
				mbtowc (&m_queue[m_nReadLen++], &buf[i], 1);
#else
				m_queue[m_nReadLen++] = buf[i];
#endif
				if (m_nReadLen == m_nMustReadBytes) {
					m_queue[m_nReadLen] = 0;
					m_nReadLen = 0;
					ReceiveCode (m_queue);
				}
			}
		}
		buf[0] = 0;
	}
}
