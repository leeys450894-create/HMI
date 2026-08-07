#include "pch.h"
#include "dfcComm.h"
#include "PlcAddressFComm.h"

C_PlcAddressFComm::C_PlcAddressFComm (PLC_EVENT_HANDLER pHandler, CString IniPath)
{
	m_nRunning	= COMM_RS_FINISH;
	m_path		= IniPath;
	m_pHandler  = pHandler;

	memset (m_bBitData,  FALSE, sizeof (UINT) * (PLC_F_END_BIT - PLC_F_BEGIN_BIT));

	ReadIniInfo ();

	if (GetCommUse ()) ConnectCtrl (TRUE);
}

C_PlcAddressFComm::~C_PlcAddressFComm ()
{

}

void	C_PlcAddressFComm::OnClose (int nErrorCode)
{
	C_ClientSocket::OnClose (nErrorCode);

	CString strMsg;
	strMsg = dfcStr (_T("★ [%s] : Close"), m_sInfo.m_strName);
	m_pHandler (strMsg.GetBuffer (), EVENT_COMM_DISCONNECT, NULL);
}

void	C_PlcAddressFComm::OnConnect (int nErrorCode)
{
	C_ClientSocket::OnConnect (nErrorCode);
}

void	C_PlcAddressFComm::ConnectCtrl (BOOL bConnect)
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

void	C_PlcAddressFComm::ReadIniInfo ()
{
	CString str = _T("F_READ");
	m_sInfo.m_strName = _T("PLC(F)READ");

	m_sInfo.m_bUse			= GetProfileInt		(_T("PLC"), dfcStr (_T("%s_ADDRESS_USE"),			 str), FALSE);
	m_sInfo.m_bOutOfService = GetProfileInt		(_T("PLC"), dfcStr (_T("%s_ADDRESS_OUT_OF_SERVICE"), str), TRUE);
	m_sInfo.m_strIP			= GetProfileString	(_T("PLC"), dfcStr (_T("%s_ADDRESS_IP"),			 str), _T("192.168.0.1"));
	m_sInfo.m_nPortNum		= GetProfileInt		(_T("PLC"), dfcStr (_T("%s_ADDRESS_PORT"),			 str), 4001);
}

//////////////////////////////////////////////////////
BOOL	C_PlcAddressFComm::QueryBitData ()
{
	if (!IsConnected ()) return FALSE;

	CString strCmd;
	//5000(헤드) 00FF(번호) 03FF(모듈번호) 00(국번) 22 -> Hex(길이) 0090(타이머) 0401(읽기) / 0001(비트단위) or 0000(워드단위)
	strCmd.AppendFormat (_T("%s%04X%s%s%s"), PLC_CMD_HEAD, PLC_CMD_SIZE, PLC_CMD_TIMER, PLC_CMD_READ, PLC_CMD_BIT);

	//시작 위치
	strCmd.AppendFormat (_T("F*06d"), PLC_F_BEGIN_BIT);

	//갯수
	strCmd.AppendFormat (_T("%04X"), PLC_F_END_BIT - PLC_F_BEGIN_BIT);

	WriteComm (strCmd);

	return TRUE;
}

BOOL	C_PlcAddressFComm::GetBit (int nAddress)
{
	if (nAddress < PLC_F_BEGIN_BIT || nAddress > PLC_F_END_BIT) return FALSE;
	
	return m_bBitData[nAddress - PLC_F_BEGIN_BIT];
}

////////////////////////////////////////////////////RECEIVE
void	C_PlcAddressFComm::ReceiveCode (TCHAR* pBuf)
{
	CString strRcv = pBuf;

	CString strHeader, strData;
	strHeader.AppendFormat (_T("%s"), PLC_RECEIVE_HEAD);
	strHeader.AppendFormat (_T("%06X"), PLC_F_END_BIT - PLC_F_BEGIN_BIT + 4);
	strHeader.AppendFormat (_T("%04X"), 0);

	int nReceiveSize = PLC_F_END_BIT - PLC_F_BEGIN_BIT + PLC_RECEIVE_HEAD_SIZE;
	int nPos = 0;

	if (strRcv.GetLength () != nReceiveSize) return;

	nPos = strRcv.Find (strHeader);
	if (nPos != 0) return;

	strData = strRcv.Mid (nPos + PLC_RECEIVE_HEAD_SIZE, PLC_F_END_BIT - PLC_F_BEGIN_BIT);

	for (int i = 0; i < PLC_F_END_BIT - PLC_F_BEGIN_BIT; i++) {
		m_bBitData[i] = _ttoi ((CString) strData.GetAt (i));
	}
}

void	C_PlcAddressFComm::WriteComm (LPCTSTR lpszMsg)
{
	CString strCmd = lpszMsg;

	Send (lpszMsg);
}
