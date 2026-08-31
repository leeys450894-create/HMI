#pragma once

typedef	void	(*PLC_EVENT_HANDLER)(TCHAR*, int, int); //메시지 전달

#include "PlcAddressMComm.h"
#include "PlcAddressFComm.h"

//class C_PlcAddressMComm;
class	AFX_EXT_CLASS	C_PlcCommMgr
{
	public:
		C_PlcCommMgr ();
		virtual ~C_PlcCommMgr ();
	
	protected:
		BOOL				m_bMAddressUse;
		C_PlcAddressMComm	*m_pMAddressR;
		C_PlcAddressMComm	*m_pMAddressW;

		BOOL				m_bFAddressUse;
		C_PlcAddressFComm	*m_pFAddressR;

	protected:
		CWinThread			*m_pAddressThread;
		BOOL				m_bRunThread;
		PLC_EVENT_HANDLER	m_pHandler;

	protected:
		void		CreateThread ();
		static UINT AddressThread (LPVOID pParam);
		void		AddressFunc ();

	public:
		BOOL	MAddressSetBit (int nAddress, int nVal, int nBitCnt = 1);
		BOOL	MAddressGetBit (int nAddress);

		BOOL	FAddressGetBit (int nAddress);

	public: //Initialize
		BOOL	CreateCommMgr (PLC_EVENT_HANDLER pHandler, CString path, BOOL bMAddressUse = FALSE, BOOL bFAddressUse = FALSE);
	
};