#include "pch.h"
#include "dfcComm.h"
#include "QualityPGComm.h"

C_QualityPGComm::C_QualityPGComm (QUALITY_PG_EVENT_HANDLER pHandler, CString IniPath, int nIndex)
{
	m_pHandler	= pHandler;
	m_path		= IniPath;
	m_cTail		= _T('\n');
	m_nIndex    = nIndex;
	m_nMustReadBytes = 8;

	memset (m_bWaitItem, FALSE, sizeof (BOOL) * QPG_WAIT_MAX);

	m_iPattern = 1;

	ReadIniInfo ();

	OpenPort (m_sInfo.m_strComPort);
}

C_QualityPGComm::~C_QualityPGComm ()
{

}

void	C_QualityPGComm::ReadIniInfo ()
{
	m_sInfo.m_bUse			= GetProfileInt		(_T("SERIAL"), dfcStr (_T("SERIAL_%d_USE"),				m_nIndex), FALSE);
	m_sInfo.m_bOutOfService = GetProfileInt		(_T("SERIAL"), dfcStr (_T("SERIAL_%d_OUT_OF_SERVICE"),	m_nIndex), TRUE);
	m_sInfo.m_strName		= GetProfileString	(_T("SERIAL"), dfcStr (_T("SERIAL_%d_NAME"),			m_nIndex), _T("QualityPG"));
	m_sInfo.m_nHead			= GetProfileInt		(_T("SERIAL"), dfcStr (_T("SERIAL_%d_HEAD_NUM"),		m_nIndex), 1);
	m_sInfo.m_nMaxCH	    = GetProfileInt		(_T("SERIAL"), dfcStr (_T("SERIAL_%d_CHANNEL"),			m_nIndex), 8);
	m_sInfo.m_strComPort	= GetProfileString	(_T("SERIAL"), dfcStr (_T("SERIAL_%d_COMPORT"),			m_nIndex), _T("COM1:9600,n,8,1"));
	m_sInfo.m_nRepDelay     = GetProfileInt		(_T("SERIAL"), dfcStr (_T("SERIAL_%d_REPDELAY"),		m_nIndex), 50);
}

void	C_QualityPGComm::ReConnect ()
{
	if (IsConnected  ()) ClosePort ();
	if (!IsConnected ()) OpenPort  (m_sInfo.m_strComPort);
}

void	C_QualityPGComm::SetWaitItem (int nItem, BOOL bSet, int iPattern)
{
	if (nItem == -1) {
		for (int i = 0; i < QPG_WAIT_MAX; i++) m_bWaitItem[i] = bSet;
	}
	else
	{
		m_bWaitItem[nItem] = bSet;
	}

	m_iPattern = iPattern;
}

BOOL	C_QualityPGComm::GetWaitItem (int nItem)
{
	return m_bWaitItem[nItem];
}

int		C_QualityPGComm::GetPtrnItem()
{
	return m_iPattern;
}


void	C_QualityPGComm::ReceiveCode (TCHAR *pBuf)
{
	if (pBuf == NULL) return;
	CString strRcv;
	for (int i = 0; i < 8; i++) strRcv += dfcStr(_T("%02X "), pBuf[i]);

	CString strMsg = dfcStr (_T("ME ¢¸ [%s%d] : %s"), m_sInfo.m_strName, m_sInfo.m_nHead, strRcv);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_MSG, NULL);

	int iPtrn = 1;
	iPtrn = pBuf[5];


	if		(pBuf[4] == 0x81) SetWaitItem (QPG_WAIT_TURN_ON,  TRUE);
	else if (pBuf[4] == 0x82) SetWaitItem (QPG_WAIT_TURN_OFF, TRUE);
	//else if (pBuf[4] == 0x83) SetWaitItem (QPG_WAIT_NEXT,	  TRUE);
	else if (pBuf[4] == 0x85) SetWaitItem (QPG_WAIT_NEXT,	  TRUE,		iPtrn);
	else if (pBuf[4] == 0xa3) SetWaitItem (QPG_WAIT_VOLTAGE,  TRUE);

}

void	C_QualityPGComm::WriteMsg (CString strCmd)
{
	CString strMsg = dfcStr (_T("ME ¢º [%s%d] : %s"), m_sInfo.m_strName, m_sInfo.m_nHead, strCmd);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_MSG, NULL);
}

BOOL	C_QualityPGComm::SetPG_Ack_Msg ()
{
	if (!m_bConnected)	return FALSE;
	
	CSingleLock lock (&m_section);
	if (!lock.IsLocked ()) lock.Lock ();
	
	BYTE byData[8];
	memset (byData, 0x00, sizeof (BYTE) * 8);
	
	byData[0] = 0x02;
	byData[1] = 0x00;
	byData[2] = 0x80;
	byData[3] = 0x02;
	byData[4] = 0x06;
	byData[5] = 0x00;
	byData[6] = 0x84;
	byData[7] = 0x03;
	
	WriteComm (byData, 8);
	
	CString strCmd;
	for (int i = 0; i < 8; i++) strCmd += dfcStr (_T("%02X "), byData[i]);
	WriteMsg (strCmd);
	
	return TRUE;
}
