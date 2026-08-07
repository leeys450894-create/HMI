#pragma once

#include "SerialComm.h"

typedef	void	(*DKCIO_EVENT_HANDLER)(TCHAR*);

class AFX_EXT_CLASS C_SyncSerialComm : public C_SerialComm
{
	public:
		C_SyncSerialComm ();
		virtual ~C_SyncSerialComm ();
	
	protected:
		char	m_pBuf[COMM_QUEUE_SIZE];
		CEvent *m_pEvent;
		DWORD	m_nRepDelay;
		CString m_strReply;
		DKCIO_EVENT_HANDLER m_pHandler;

		BOOL	m_bRcvByte;
		BYTE	m_byQueue[COMM_QUEUE_SIZE];
		int		m_nRcvCnt;

	public:
				DWORD	WriteComm (LPVOID pBuf, DWORD dwBytesToWrite);
		virtual DWORD	WriteComm (int nVal);
		virtual DWORD	WriteComm (CString string);
		virtual	void	OnReceive ();
		virtual BOOL	WaitReply ();
		virtual void	ReceiveCode (TCHAR *pBuf) {};
};
