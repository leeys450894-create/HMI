#include "pch.h"
#include "dfcComm.h"
#include "SPOLightComm.h"

C_SPOLightComm::C_SPOLightComm (CString path, CString section, int nIndex)
{
	m_path		= path;
	m_cTail		= _T('\r');
	m_nPort		= nIndex;
	m_nRepDelay = GetProfileInt		(_T("SERIAL"), dfcStr (_T("SERIAL_%d_REPDELAY"), nIndex), 50);
	m_strPort	= GetProfileString	(_T("SERIAL"), dfcStr (_T("SERIAL_%d_COMPORT"), nIndex), _T("COM1:9600,n,8,2"));
	m_nMaxCH	= GetProfileInt		(_T("SERIAL"), dfcStr (_T("SERIAL_%d_CHANNEL"), nIndex), 8);

	m_nLightVal = new int[m_nMaxCH];
	m_bRcvByte  = TRUE;
	m_nRcvCnt   = 3;

	OpenPort (m_strPort);
}

C_SPOLightComm::~C_SPOLightComm ()
{
	SAFE_DELETE (m_nLightVal);
}

BOOL	C_SPOLightComm::SetLight (int nChannel, int nVal)
{
	if (!m_bConnected)	return FALSE;

	BYTE byData[3];
	memset (byData, 0x00, sizeof (BOOL) * 3);

	int nCmdVal = nVal;
	if (nVal == 0) nCmdVal = 1;

	byData[0] = 'C'; //0x43
	byData[1] = 0x30 + nChannel;
	byData[2] = nCmdVal;

	WriteComm (byData, 3);

	if (!WaitReply ()) return FALSE;

	if (m_byQueue[0] != 'R')		return FALSE;
	if (m_byQueue[1] != byData[1])	return FALSE;
	if (m_byQueue[2] != byData[2])  return FALSE;

	m_nLightVal[nChannel] = nCmdVal;

	//CString strTemp;
	//strTemp.Format (_T("%c"), m_byQueue[1]);

	//strTemp.Format(_T("%X"), byData[2]);
	//int nNum = strtol((CStringA) strTemp, NULL, 16);

	return TRUE;
}

int		C_SPOLightComm::GetLight (int nChannel)
{
	if (!m_bConnected)	return -1;

	if (nChannel < 0 || nChannel > m_nMaxCH) return -1;

	return m_nLightVal[nChannel];
}
