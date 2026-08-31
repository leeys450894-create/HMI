#include "pch.h"
#include "dfcComm.h"
#include "PlcCommMgr.h"

C_PlcCommMgr::C_PlcCommMgr ()
{
	m_bMAddressUse	  = FALSE;
	m_pMAddressR	  = NULL;
	m_pMAddressW	  = NULL;

	m_bFAddressUse    = FALSE;
	m_pFAddressR	  = NULL;

	m_bRunThread	  = FALSE;
	m_pAddressThread  = NULL;
}

C_PlcCommMgr::~C_PlcCommMgr ()
{
	m_bRunThread = FALSE;
	while (m_pAddressThread) { Sleep(0); }

	SAFE_DELETE (m_pMAddressR);
	SAFE_DELETE (m_pMAddressW);
	SAFE_DELETE (m_pFAddressR);
}

BOOL	C_PlcCommMgr::CreateCommMgr (PLC_EVENT_HANDLER pHandler, CString path, BOOL bMAddressUse, BOOL bFAddressUse)
{
	m_pHandler	   = pHandler;
	m_bMAddressUse = bMAddressUse;
	m_bFAddressUse = bFAddressUse;

	if (m_bMAddressUse) {
		m_pMAddressR = new C_PlcAddressMComm (pHandler, path, TRUE);
		m_pMAddressW = new C_PlcAddressMComm (pHandler, path, FALSE);
	}

	if (m_bFAddressUse) 
		m_pFAddressR = new C_PlcAddressFComm (pHandler, path);

	CreateThread ();

	BOOL  bConnect = TRUE;
	if (m_bMAddressUse) {
		bConnect &= m_pMAddressR->IsConnected ();
		bConnect &= m_pMAddressW->IsConnected ();
	}

	if (m_bFAddressUse) 
		bConnect &= m_pFAddressR->IsConnected ();

	return bConnect;
}

void	C_PlcCommMgr::CreateThread ()
{
	m_bRunThread = TRUE;

	if (m_pAddressThread == NULL)
		m_pAddressThread = AfxBeginThread (AddressThread, this);
}

UINT	C_PlcCommMgr::AddressThread (LPVOID pParam)
{
	C_PlcCommMgr *pCommMgr = (C_PlcCommMgr*)(pParam);
	if (pCommMgr && pCommMgr->m_bRunThread) {
		pCommMgr->AddressFunc ();
	}
	pCommMgr->m_pAddressThread = NULL;

	return 0;
}

void	C_PlcCommMgr::AddressFunc ()
{
	while (m_bRunThread) {
		if (m_pMAddressR) {
			m_pMAddressR->QueryBitData ();
			Sleep (10);
		}

		if (m_pFAddressR) {
			m_pFAddressR->QueryBitData ();
			Sleep (10);
		}
	}
}

BOOL	C_PlcCommMgr::MAddressSetBit (int nAddress, int nVal, int nBitCnt)
{
	if (!m_pMAddressW) return FALSE;

	return m_pMAddressW->SetBit (nAddress, nVal, nBitCnt);
}

BOOL	C_PlcCommMgr::MAddressGetBit (int nAddress)
{
	if (!m_pMAddressR) return FALSE;

	return m_pMAddressR->GetBit (nAddress);
}

BOOL	C_PlcCommMgr::FAddressGetBit (int nAddress)
{
	if (!m_pFAddressR) return FALSE;

	return m_pFAddressR->GetBit (nAddress);
}