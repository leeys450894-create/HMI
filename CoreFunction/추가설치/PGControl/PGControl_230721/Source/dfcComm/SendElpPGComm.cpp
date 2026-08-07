#include "pch.h"
#include "dfcComm.h"
#include "SendElpPGComm.h"

C_SendElpPGComm::C_SendElpPGComm (SEND_PG_EVENT_HANDLER pHandler, CString IniPath, int nIndex)
{
	m_pHandler	= pHandler;
	m_path		= IniPath;
	m_cTail		= _T('\n');
	m_nIndex    = nIndex;
	m_bRcvByte  = TRUE;
	m_nRcvCnt   = 8;

	ReadIniInfo ();

	m_nRepDelay = m_sInfo.m_nRepDelay;

	OpenPort (m_sInfo.m_strComPort);
}

C_SendElpPGComm::~C_SendElpPGComm ()
{

}

void	C_SendElpPGComm::ReadIniInfo ()
{
	m_sInfo.m_bUse			= GetProfileInt		(_T("SERIAL"), dfcStr (_T("SERIAL_%d_USE"),				m_nIndex), FALSE);
	m_sInfo.m_bOutOfService = GetProfileInt		(_T("SERIAL"), dfcStr (_T("SERIAL_%d_OUT_OF_SERVICE"),	m_nIndex), TRUE);
	m_sInfo.m_strName		= GetProfileString	(_T("SERIAL"), dfcStr (_T("SERIAL_%d_NAME"),			m_nIndex), _T("PG"));
	m_sInfo.m_nHead			= GetProfileInt		(_T("SERIAL"), dfcStr (_T("SERIAL_%d_HEAD_NUM"),		m_nIndex), 1);
	m_sInfo.m_nMaxCH	    = GetProfileInt		(_T("SERIAL"), dfcStr (_T("SERIAL_%d_CHANNEL"),			m_nIndex), 8);
	m_sInfo.m_strComPort	= GetProfileString	(_T("SERIAL"), dfcStr (_T("SERIAL_%d_COMPORT"),			m_nIndex), _T("COM1:9600,n,8,1"));
	m_sInfo.m_nRepDelay     = GetProfileInt		(_T("SERIAL"), dfcStr (_T("SERIAL_%d_REPDELAY"),		m_nIndex), 50);
}

void	C_SendElpPGComm::ReConnect ()
{
	if (IsConnected  ()) ClosePort ();
	if (!IsConnected ()) OpenPort  (m_sInfo.m_strComPort);
}

void	C_SendElpPGComm::WriteMsg (CString strCmd)
{
	CString strMsg = dfcStr (_T("ME ¢º [%s%d] : %s"), m_sInfo.m_strName, m_sInfo.m_nHead, strCmd);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_MSG, NULL);
}

void	C_SendElpPGComm::WriteMsg2(CString strCmd)
{
	CString strMsg = dfcStr(_T("ME ¢¸ [%s%d] : %s"), m_sInfo.m_strName, m_sInfo.m_nHead, strCmd);
	if (m_pHandler) m_pHandler(strMsg.GetBuffer(), EVENT_COMM_MSG, NULL);
}

BOOL	C_SendElpPGComm::ActPGOn ()
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
	byData[4] = 0x81;
	byData[5] = 0x00;
	byData[6] = 0x03;
	byData[7] = 0x03;

	WriteComm (byData, m_nRcvCnt);

	CString strCmd;
	for (int i = 0; i < 8; i++) strCmd += dfcStr (_T("%02X "), byData[i]);
	WriteMsg (strCmd);

	if (!WaitReply()) return FALSE;

	strCmd.Empty();
	for (int i = 0; i < 8; i++) strCmd += dfcStr(_T("%02X "), m_byQueue[i]);
	WriteMsg2(strCmd);

	if (m_byQueue[4] != 0x06/*0x81*/) return FALSE;

	return TRUE;
}

BOOL	C_SendElpPGComm::ActPGOff()
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
	byData[4] = 0x82;
	byData[5] = 0x00;
	byData[6] = 0x04;//0x00;
	byData[7] = 0x03;

	WriteComm (byData, m_nRcvCnt);

	CString strCmd;
	for (int i = 0; i < 8; i++) strCmd += dfcStr (_T("%02x "), byData[i]);
	WriteMsg (strCmd);

	if (!WaitReply()) return FALSE;

	strCmd.Empty();
	for (int i = 0; i < 8; i++) strCmd += dfcStr(_T("%02X "), m_byQueue[i]);
	WriteMsg2(strCmd);

	if (m_byQueue[4] != 0x06/*0x82*/) return FALSE;

	return TRUE;
}

BOOL	C_SendElpPGComm::ActPGNext()
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
	byData[4] = 0x83;
	byData[5] = 0x00;
	byData[6] = 0x05;//0x01;
	byData[7] = 0x03;

	WriteComm (byData, m_nRcvCnt);

	CString strCmd;
	for (int i = 0; i < 8; i++) strCmd += dfcStr (_T("%02x "), byData[i]);
	WriteMsg (strCmd);

	if (!WaitReply()) return FALSE;

	strCmd.Empty();
	for (int i = 0; i < 8; i++) strCmd += dfcStr(_T("%02X "), m_byQueue[i]);
	WriteMsg2(strCmd);

	if (m_byQueue[4] != 0x06/*0x83*/) return FALSE;

	return TRUE;
}

BOOL	C_SendElpPGComm::ActPGSet(int i)
{
	if (!m_bConnected)	return FALSE;

	CSingleLock lock(&m_section);
	if (!lock.IsLocked()) lock.Lock();

	BYTE byData[8];
	memset(byData, 0x00, sizeof(BYTE) * 8);

	if( i < 1)
		i = 1;

	byData[0] = 0x02;
	byData[1] = 0x00;
	byData[2] = 0x80;
	byData[3] = 0x02;
	byData[4] = 0x85;
	byData[5] = 0x00 + i;
	byData[6] = 0x07 + i;
	byData[7] = 0x03;

	WriteComm(byData, m_nRcvCnt);

	CString strCmd;
	for (int i = 0; i < 8; i++) strCmd += dfcStr(_T("%02X "), byData[i]);
	WriteMsg(strCmd);

	if (!WaitReply()) return FALSE;

	strCmd.Empty();
	for (int i = 0; i < 8; i++) strCmd += dfcStr(_T("%02X "), m_byQueue[i]);
	WriteMsg2(strCmd);

	if (m_byQueue[4] != 0x06/*0x83*/) return FALSE;

	return TRUE;
}