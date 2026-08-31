#include "pch.h"
#include "dfcComm.h"
#include "PlcAddressMComm.h"

C_PlcAddressMComm::C_PlcAddressMComm (PLC_EVENT_HANDLER pHandler, CString IniPath, BOOL bRead)
{
	m_nRunning	= COMM_RS_FINISH;
	m_path		= IniPath;
	m_bRead		= bRead;
	m_pHandler  = pHandler;

	memset (m_bBitData,  FALSE, sizeof (UINT) * (PLC_M_END_BIT - PLC_M_BEGIN_BIT));

	ReadIniInfo ();

	if (GetCommUse ()) ConnectCtrl (TRUE);
}

C_PlcAddressMComm::~C_PlcAddressMComm ()
{

}

void	C_PlcAddressMComm::OnClose (int nErrorCode)
{
	C_ClientSocket::OnClose (nErrorCode);

	CString strMsg;
	strMsg = dfcStr (_T("★ [%s] : Close"), m_sInfo.m_strName);
	m_pHandler (strMsg.GetBuffer (), EVENT_COMM_DISCONNECT, NULL);
}

void	C_PlcAddressMComm::OnConnect (int nErrorCode)
{
	C_ClientSocket::OnConnect (nErrorCode);
}

void	C_PlcAddressMComm::ConnectCtrl (BOOL bConnect)
{
	C_ClientSocket::Close ();

	CString strMsg;
	BOOL bOpen = FALSE;

	if (bConnect) {
		bOpen = OpenPort (m_sInfo.m_strIP, m_sInfo.m_nPortNum);

		if (bOpen)	strMsg = dfcStr (_T("★ [%s] : Connect Sucess"), m_sInfo.m_strName);
		else		strMsg = dfcStr (_T("★ [%s] : Connect Fail"),   m_sInfo.m_strName);
	}
	else			strMsg = dfcStr (_T("★ [%s] : Close"),		     m_sInfo.m_strName);

	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_MSG, NULL);
}

void	C_PlcAddressMComm::ReadIniInfo ()
{
	CString str;

	if (m_bRead) {
		str = _T("M_READ");
		m_sInfo.m_strName = _T("PLC(M)READ");
	}
	else {
		str = _T("M_WRITE");
		m_sInfo.m_strName = _T("PLC(M)WRITE");
	}

	m_sInfo.m_bUse			= GetProfileInt		(_T("PLC"), dfcStr (_T("%s_ADDRESS_USE"),			 str), FALSE);
	m_sInfo.m_bOutOfService = GetProfileInt		(_T("PLC"), dfcStr (_T("%s_ADDRESS_OUT_OF_SERVICE"), str), TRUE);
	m_sInfo.m_strIP			= GetProfileString	(_T("PLC"), dfcStr (_T("%s_ADDRESS_IP"),			 str), _T("192.168.0.1"));
	m_sInfo.m_nPortNum		= GetProfileInt		(_T("PLC"), dfcStr (_T("%s_ADDRESS_PORT"),			 str), 4001);
}

//////////////////////////////////////////////////////
BOOL	C_PlcAddressMComm::QueryBitData ()
{
	if (!IsConnected ()) return FALSE;

	CString strCmd;
	//5000(헤드) 00FF(번호) 03FF(모듈번호) 00(국번) 22 -> Hex(길이) 0090(타이머) 0401(읽기) / 0001(비트단위) or 0000(워드단위)
	strCmd.AppendFormat (_T("%s%04X%s%s%s"), PLC_CMD_HEAD, PLC_CMD_SIZE, PLC_CMD_TIMER, PLC_CMD_READ, PLC_CMD_BIT);

	//시작 위치
	strCmd.AppendFormat (_T("M*06d"), PLC_M_BEGIN_BIT);

	//갯수
	strCmd.AppendFormat (_T("%04X"), PLC_M_END_BIT - PLC_M_BEGIN_BIT);

	WriteComm (strCmd);

	return TRUE;
}

BOOL	C_PlcAddressMComm::SetBit (int nAddress, int nVal, int nBitCnt)
{
	if (!IsConnected ()) return FALSE;

	CString strCmd;
	//5000(헤드) 00FF(번호) 03FF(모듈번호) 00(국번) 22 + BitCnt -> Hex(길이) 0090(타이머) 0401(읽기) / 0001(비트단위) or 0000(워드단위)
	strCmd.AppendFormat (_T("%s%04X%s%s%s"), PLC_CMD_HEAD, PLC_CMD_SIZE + nBitCnt, PLC_CMD_TIMER, PLC_CMD_WRITE, PLC_CMD_BIT);

	//시작 위치
	strCmd.AppendFormat (_T("M*06d"), nAddress);

	//갯수
	strCmd.AppendFormat (_T("%04X"), nBitCnt);

	CString str;
	std::bitset<32> bit = nVal;
	for (int i = 0; i < nBitCnt; i++) {
		str.AppendFormat (_T("%d"), (int) bit[i]);
	}

	WriteComm (strCmd);

	return TRUE;
}

BOOL	C_PlcAddressMComm::GetBit (int nAddress)
{
	if (nAddress < PLC_M_BEGIN_BIT || nAddress > PLC_M_END_BIT) return FALSE;
	
	return m_bBitData[nAddress - PLC_M_BEGIN_BIT];
}

////////////////////////////////////////////////////RECEIVE
void	C_PlcAddressMComm::ReceiveCode (TCHAR* pBuf)
{
	CString strRcv = pBuf;

	CString strHeader, strData;
	strHeader.AppendFormat (_T("%s"),   PLC_RECEIVE_HEAD);
	strHeader.AppendFormat (_T("%06X"), PLC_M_END_BIT - PLC_M_BEGIN_BIT + 4);
	strHeader.AppendFormat (_T("%04X"), 0);

	int nReceiveSize = PLC_M_END_BIT - PLC_M_BEGIN_BIT + PLC_RECEIVE_HEAD_SIZE;
	int nPos = 0;

	if (strRcv.GetLength () != nReceiveSize) return;

	nPos = strRcv.Find (strHeader);
	if (nPos != 0) return;

	strData = strRcv.Mid (nPos + PLC_RECEIVE_HEAD_SIZE, PLC_M_END_BIT - PLC_M_BEGIN_BIT);

	for (int i = 0; i < PLC_M_END_BIT - PLC_M_BEGIN_BIT; i++) {
		m_bBitData[i] = _ttoi ((CString) strData.GetAt (i));
	}

	if (m_strOldData != strData) {
		m_strOldData = strData;
		m_pHandler (m_strOldData.GetBuffer (), EVENT_COMM_MSG, NULL);
	}
}

void	C_PlcAddressMComm::WriteComm (LPCTSTR lpszMsg)
{
	CString strCmd = lpszMsg;

	Send (lpszMsg);
}
