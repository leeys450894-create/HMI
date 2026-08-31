#pragma once

class	AFX_EXT_CLASS	C_SerialComm : public C_IniFile
{
	public:
		HANDLE		m_hComm;
		HANDLE		m_hThread;
		OVERLAPPED	m_osRead, m_osWrite;
		CString		m_strDelim;
		TCHAR		m_cTail;
		TCHAR		m_queue[COMM_QUEUE_SIZE];
		char		m_mbBuf[1024];
		BOOL		m_bReceived;
		int			m_nMustReadBytes;
		int			m_nReadLen;

	protected:
		BOOL		m_bConnected;

	public:
		C_SerialComm ();
		virtual	~C_SerialComm ();

		BOOL	GetConnected () {return m_bConnected;}
		void	SetConnect (BOOL bConnect)	{	m_bConnected = bConnect; }

	public:
		BOOL	OpenPort (CString string);
		BOOL	OpenPort (CString section, CString key);
		BOOL	OpenPort (CString portName, int baudRate, TCHAR cParity, int nByteSize, int nStopBits);
		void	ClosePort ();

	protected:
		DWORD	ReadComm  (LPVOID pBuf, DWORD dwBytesToRead);
		DWORD	WriteComm (LPVOID pBuf, DWORD dwBytesToWrite);
		DWORD	WriteComm (TCHAR ch);
		DWORD	WriteComm (int nVal);
		DWORD	WriteComm (CString string, BOOL bIncludeDelim = TRUE);

	public:
		virtual	void	OnReceive ();
		virtual	void	ReceiveCode (TCHAR *pBuf) = 0;
};