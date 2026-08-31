#include "pch.h"
#include "dfcComm.h"
#include "SPOLightComm.h"

C_SPOLightComm::C_SPOLightComm (CString path, CString section, int nIndex)
{
	m_path		= path;
	//m_cTail		= _T('\r');
	m_nIndex    = nIndex;

	ReadIniInfo ();

	m_nRepDelay = m_sInfo.m_nRepDelay;

	m_nLightVal = new int[m_sInfo.m_nMaxCH];
	m_nLightOn  = new int[m_sInfo.m_nMaxCH];
	m_bRcvByte  = TRUE;
	m_nRcvCnt   = 3;

	memset (m_nLightVal, 0, sizeof (int) * m_sInfo.m_nMaxCH);
	memset (m_nLightOn,  0, sizeof (int) * m_sInfo.m_nMaxCH);

	OpenPort (m_sInfo.m_strComPort);
}

C_SPOLightComm::~C_SPOLightComm ()
{
	SAFE_DELETE (m_nLightVal);
	SAFE_DELETE (m_nLightOn);
}

void	C_SPOLightComm::ReadIniInfo ()
{
	m_sInfo.m_bUse			= GetProfileInt		(_T("LED"), dfcStr (_T("LED_%d_USE"),				m_nIndex), FALSE);
	m_sInfo.m_bOutOfService = GetProfileInt		(_T("LED"), dfcStr (_T("LED_%d_OUT_OF_SERVICE"),	m_nIndex), TRUE);
	m_sInfo.m_strName		= GetProfileString	(_T("LED"), dfcStr (_T("LED_%d_NAME"),				m_nIndex), _T("SPO"));
	m_sInfo.m_nHead			= GetProfileInt		(_T("LED"), dfcStr (_T("LED_%d_HEAD_NUM"),			m_nIndex), 1);
	m_sInfo.m_nMaxCH	    = GetProfileInt		(_T("LED"), dfcStr (_T("LED_%d_CHANNEL"),			m_nIndex), 8);
	m_sInfo.m_strComPort    = GetProfileString	(_T("LED"), dfcStr (_T("LED_%d_COMPORT"),			m_nIndex), _T("COM1:9600,n,8,1"));
	m_sInfo.m_nRepDelay     = GetProfileInt		(_T("LED"), dfcStr (_T("LED_%d_REPDELAY"),			m_nIndex), 50);
}

BOOL	C_SPOLightComm::SetLight (int nChannel, int nVal)
{
	if (!m_bConnected)	return FALSE;
	if (nChannel < 0 || nChannel > m_sInfo.m_nMaxCH) return FALSE;

	CSingleLock lock (&m_section);
	if (!lock.IsLocked ()) lock.Lock ();

	BYTE byData[3];
	memset (byData, 0x00, sizeof (BYTE) * 3);

	int nCmdVal = nVal;
	if (nVal == 0) nCmdVal = 1;

	byData[0] = 'C'; //0x43
	byData[1] = 0x31 + nChannel; //0x30 + nChannel;
	byData[2] = nCmdVal;

	WriteComm (byData, m_nRcvCnt);

	if (!WaitReply ()) return FALSE;

	if (m_byQueue[0] != 'R')		return FALSE;
	if (m_byQueue[1] != byData[1])	return FALSE;
	if (m_byQueue[2] != byData[2])  return FALSE;

	m_nLightVal[nChannel] = nCmdVal;

	return TRUE;
}

BOOL	C_SPOLightComm::SetLightOn (int nChannel, BOOL bOn)
{
	if (!m_bConnected)	return FALSE;
	if (nChannel < 0 || nChannel > m_sInfo.m_nMaxCH) return FALSE;

	CSingleLock lock (&m_section);
	if (!lock.IsLocked ()) lock.Lock ();

	BYTE byData[3];
	memset (byData, 0x00, sizeof (BYTE) * 3);

	std::bitset<8> bit = 0;

	if (nChannel == -1) {
		for (int i = 0; i < 8; i++) bit[i] = bOn;
	}
	else {
		bit[nChannel] = bOn;
	}

	byData[0] = 'O'; //0x4F
	byData[1] = 'N'; //0x4E
	byData[2] = (int) bit.to_ulong ();

	//WriteComm (byData, m_nRcvCnt);

	//if (!WaitReply ()) return FALSE;

	//if (m_byQueue[0] != 'R')		return FALSE;
	//if (m_byQueue[1] != byData[1])	return FALSE;
	//if (m_byQueue[2] != byData[2])  return FALSE;

	//m_nLightOn[nChannel] = bOn;

	return TRUE;
}

int		C_SPOLightComm::GetLight (int nChannel)
{
	if (!m_bConnected)	return -1;
	if (nChannel < 0 || nChannel > m_sInfo.m_nMaxCH) return -1;

	return m_nLightVal[nChannel];
}

BOOL	C_SPOLightComm::GetLightOn (int nChannel)
{
	if (!m_bConnected)	return FALSE;
	if (nChannel < 0 || nChannel > m_sInfo.m_nMaxCH) return FALSE;

	return m_nLightOn[nChannel];
}

BOOL	C_SPOLightComm::QueryLightVal (int nChannel)
{
	if (!m_bConnected)	return FALSE;
	if (nChannel < 0 || nChannel > m_sInfo.m_nMaxCH) return FALSE;

	BYTE byData[3];
	memset (byData, 0x00, sizeof (BYTE) * 3);

	byData[0] = 'R'; //0x43
	byData[1] = 0x31 + nChannel; //0x30 + nChannel;
	byData[2] = 'D'; //0x44

	WriteComm (byData, m_nRcvCnt);

	if (!WaitReply ()) return FALSE;

	if (m_byQueue[0] != 'R')		return FALSE;
	if (m_byQueue[1] != byData[1])	return FALSE;

	CString strTemp;
	strTemp.Format (_T("%X"), m_byQueue[2]);

	m_nLightVal[nChannel] = strtol ((CStringA) strTemp, NULL, 16);

	return TRUE;
}

BOOL	C_SPOLightComm::QueryLightOn ()
{
	if (!m_bConnected)	return FALSE;

	BYTE byData[3];
	memset (byData, 0x00, sizeof (BYTE) * 3);

	byData[0] = 'R'; //0x43
	byData[1] = 'O'; //0x4F
	byData[2] = 'N'; //0x4E

	WriteComm (byData, m_nRcvCnt);

	if (!WaitReply ()) return FALSE;

	if (m_byQueue[0] != 'O') return FALSE;
	if (m_byQueue[1] != 'N') return FALSE;

	CString strTemp;
	strTemp.Format (_T("%X"), m_byQueue[2]);

	int nVal = 0;
	nVal = strtol ((CStringA) strTemp, NULL, 16);

	std::bitset <8> bit = nVal;
	for (int i = 0; i < m_sInfo.m_nMaxCH; i++) {
		m_nLightOn[i] = bit[i];
	}

	return TRUE;
}