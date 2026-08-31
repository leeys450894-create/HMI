#include "pch.h"
#include "dfcComm.h"
#include "CASLightComm.h"

C_CASLightComm::C_CASLightComm (CString path, CString section, int nIndex)
{
	m_path		= path;
	m_cTail		= _T('\n');
	m_nIndex    = nIndex;
	
	ReadIniInfo ();
	
	m_nLightVal = new int[m_sInfo.m_nMaxCH];
	m_nLightOn  = new int[m_sInfo.m_nMaxCH];
	memset (m_nLightVal, 0, sizeof (int) * m_sInfo.m_nMaxCH);
	memset (m_nLightOn,  0, sizeof (int) * m_sInfo.m_nMaxCH);

	OpenPort (m_sInfo.m_strComPort);
}

C_CASLightComm::~C_CASLightComm ()
{
	SAFE_DELETE (m_nLightVal);
	SAFE_DELETE (m_nLightOn);
}

void	C_CASLightComm::ReadIniInfo ()
{
	m_sInfo.m_bUse			= GetProfileInt		(_T("LED"), dfcStr (_T("LED_%d_USE"),				m_nIndex), FALSE);
	m_sInfo.m_bOutOfService = GetProfileInt		(_T("LED"), dfcStr (_T("LED_%d_OUT_OF_SERVICE"),	m_nIndex), TRUE);
	m_sInfo.m_strName		= GetProfileString	(_T("LED"), dfcStr (_T("LED_%d_NAME"),				m_nIndex), _T("SPO"));
	m_sInfo.m_nHead			= GetProfileInt		(_T("LED"), dfcStr (_T("LED_%d_HEAD_NUM"),			m_nIndex), 1);
	m_sInfo.m_nMaxCH	    = GetProfileInt		(_T("LED"), dfcStr (_T("LED_%d_CHANNEL"),			m_nIndex), 8);
	m_sInfo.m_strComPort	= GetProfileString	(_T("LED"), dfcStr (_T("LED_%d_COMPORT"),			m_nIndex), _T("COM1:9600,n,8,1"));
	m_sInfo.m_nRepDelay     = GetProfileInt		(_T("LED"), dfcStr (_T("LED_%d_REPDELAY"),			m_nIndex), 50);
}

int		C_CASLightComm::CalcValue (int nVal)
{
	double fRatio = 1024 / 255;

	return (int) (nVal * fRatio + 0.4);
}

void	C_CASLightComm::ReceiveCode (TCHAR *pBuf)
{
	if (pBuf == NULL) return;
	CString strBuf = pBuf;

	CStringArray strings;

	dfcTokenize (pBuf, _T(" "), strings);
	if (strBuf.Find (_T("$")) >= 0) {
		if (strings.GetSize () != 4) return;

		m_nLightVal[_ttoi (strings[2])] = _ttoi (strings[3]);
	}
	else if (strBuf.Find (_T("getonex")) >= 0) {
		if (strings.GetSize () != 3) return;

		std::bitset<8> bit = 0;
		bit = strtol ((CStringA) strings[3], NULL, 16);

		for (int i = 0; i < m_sInfo.m_nMaxCH; i++) {
			m_nLightOn[i] = bit[i];
		}
	}
	else if (strBuf.Find (_T("geterrstatus")) >= 0) {
		if (strings.GetSize () != 3) return;

		if (_ttoi (strings[1])) { //error 발생
			//0x02 과온도 보호 상태
			//0x10 과전류 보호 상태 (0~3채널)
			//0x20 과전류 보호 상태 (4~7채널)
			//0x40 과전류 보호 상태 (8~11채널)
			//0x80 과전류 보호 상태 (12~15채널)

			int nAlarm  = _ttoi (strings[2]);
			int nDivide = nAlarm / 10;	//앞자리 알람
			int	nMod    = nAlarm % 10;	//뒷자리 알람
		}
	}
}

BOOL	C_CASLightComm::SetLight (int nChannel, int nVal)
{
	if (!m_bConnected)	return FALSE;
	if (nChannel < 0 || nChannel > m_sInfo.m_nMaxCH) return FALSE;

	CSingleLock lock (&m_section);
	if (!lock.IsLocked ()) lock.Lock ();

	CString strVal;
	strVal.Format (_T("# %d %d"), nChannel, CalcValue (nVal));

	WriteComm (strVal);

	return TRUE;
}

BOOL	C_CASLightComm::SetLightOn (int nChannel, BOOL bOn)
{
	if (!m_bConnected)	return FALSE;
	if (nChannel < 0 || nChannel > m_sInfo.m_nMaxCH) return FALSE;

	CSingleLock lock (&m_section);
	if (!lock.IsLocked ()) lock.Lock ();

	std::bitset<8> bit = 0;

	if (nChannel == -1) {
		for (int i = 0; i < 8; i++) bit[i] = bOn;
	}
	else {
		bit[nChannel] = bOn;
	}

	CString strVal;
	strVal.Format (_T("setonex %02X"), bit.to_ulong ());

	WriteComm (strVal);

	return TRUE;
}

int		C_CASLightComm::GetLight (int nChannel)
{
	if (!m_bConnected)	return -1;
	if (nChannel < 0 || nChannel > m_sInfo.m_nMaxCH) return -1;

	return m_nLightVal[nChannel];
}

BOOL	C_CASLightComm::GetLightOn (int nChannel)
{
	if (!m_bConnected)	return -1;
	if (nChannel < 0 || nChannel > m_sInfo.m_nMaxCH) return -1;

	return m_nLightOn[nChannel];
}

BOOL	C_CASLightComm::QueryLightVal (int nChannel)
{
	if (!m_bConnected)	return FALSE;
	if (nChannel < 0 || nChannel > m_sInfo.m_nMaxCH) return FALSE;

	CSingleLock lock (&m_section);
	lock.Lock ();

	CString strVal;
	strVal.Format (_T("$ %d"), nChannel);

	WriteComm (strVal);

	return TRUE;
}

BOOL	C_CASLightComm::QueryLightOn ()
{
	return TRUE;
}