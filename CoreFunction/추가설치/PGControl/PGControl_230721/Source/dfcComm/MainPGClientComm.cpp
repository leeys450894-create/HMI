#include "pch.h"
#include "dfcComm.h"
#include "MainPGClientComm.h"

C_MainPGClientComm::C_MainPGClientComm (MAIN_PG_EVENT_HANDLER pHandler, CString IniPath, int nIndex)
{
	m_nRunning	  = COMM_RS_FINISH;
	m_pHandler	  = pHandler;
	m_nIndex	  = nIndex;			//통신 개수 따라 증가 Ini파일 접근시만 사용
	m_path		  = IniPath;

	ReadIniInfo ();

//	if (GetCommUse ()) ConnectCtrl (TRUE);
}

C_MainPGClientComm::~C_MainPGClientComm ()
{

}

void	C_MainPGClientComm::OnClose (int nErrorCode)
{
	C_ClientSocket::OnClose (nErrorCode);

	CString strMsg;
	strMsg = dfcStr (_T("★ [%s%d] : Close"), m_sInfo.m_strName, m_sInfo.m_nHead);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_DISCONNECT, m_sInfo.m_nHead);
}

void	C_MainPGClientComm::OnConnect (int nErrorCode)
{
	C_ClientSocket::OnConnect (nErrorCode);
}

void	C_MainPGClientComm::ConnectCtrl (BOOL bConnect)
{
	if (GetCommUse() != TRUE) return;

	C_ClientSocket::Close ();

	CString strMsg;
	BOOL bOpen = FALSE;

	if (bConnect) {
		bOpen = OpenPort (m_sInfo.m_strIP, m_sInfo.m_nPortNum);

		if (bOpen)	strMsg = dfcStr (_T("★ [%s%d] : Connect Sucess"),	m_sInfo.m_strName, m_sInfo.m_nHead);
		else		strMsg = dfcStr (_T("★ [%s%d] : Connect Fail"),	m_sInfo.m_strName, m_sInfo.m_nHead);
	}
	else			strMsg = dfcStr (_T("★ [%s%d] : Close"),			m_sInfo.m_strName, m_sInfo.m_nHead);

	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_MSG, NULL);
}

void	C_MainPGClientComm::ReadIniInfo ()
{
	m_sInfo.m_bUse			= GetProfileInt		(_T("TCPIP"), dfcStr (_T("TCPIP_%d_USE"),				m_nIndex), FALSE);
	m_sInfo.m_bOutOfService = GetProfileInt		(_T("TCPIP"), dfcStr (_T("TCPIP_%d_OUT_OF_SERVICE"),	m_nIndex), TRUE);
	m_sInfo.m_strName		= GetProfileString	(_T("TCPIP"), dfcStr (_T("TCPIP_%d_NAME"),				m_nIndex), _T("ColorPG(S)"));
	m_sInfo.m_nHead			= GetProfileInt		(_T("TCPIP"), dfcStr (_T("TCPIP_%d_HEAD_NUM"),			m_nIndex), 1);
	m_sInfo.m_strIP			= GetProfileString	(_T("TCPIP"), dfcStr (_T("TCPIP_%d_IP"),				m_nIndex), _T("192.168.0.1"));
	m_sInfo.m_nPortNum		= GetProfileInt		(_T("TCPIP"), dfcStr (_T("TCPIP_%d_PORT"),				m_nIndex), 4001);
	m_sInfo.m_strResultDir  = GetProfileString	(_T("TCPIP"), dfcStr (_T("TCPIP_%d_RESULT_DIRECTORY"),	m_nIndex), _T("Z:\\"));
	m_sInfo.m_strRecipeFile = GetProfileString	(_T("TCPIP"), dfcStr (_T("TCPIP_%d_RECIPE_FILE"),		m_nIndex), _T("Z:\\Recipe.ini"));
}

void	C_MainPGClientComm::WriteIniInfo (S_TcpIpInfo *sInfo)
{
	WriteProfileInt		(_T("TCPIP"), dfcStr(_T("TCPIP_%d_USE"),				m_nIndex), sInfo->m_bUse);
	WriteProfileInt		(_T("TCPIP"), dfcStr(_T("TCPIP_%d_OUT_OF_SERVICE"),		m_nIndex), sInfo->m_bOutOfService);
	WriteProfileString	(_T("TCPIP"), dfcStr(_T("TCPIP_%d_NAME"),				m_nIndex), sInfo->m_strName);
	WriteProfileInt		(_T("TCPIP"), dfcStr(_T("TCPIP_%d_HEAD_NUM"),			m_nIndex), sInfo->m_nHead);
	WriteProfileString	(_T("TCPIP"), dfcStr(_T("TCPIP_%d_IP"),					m_nIndex), sInfo->m_strIP);
	WriteProfileInt		(_T("TCPIP"), dfcStr(_T("TCPIP_%d_PORT"),				m_nIndex), sInfo->m_nPortNum);
	WriteProfileString	(_T("TCPIP"), dfcStr(_T("TCPIP_%d_RESULT_DIRECTORY"),	m_nIndex), sInfo->m_strResultDir);
	WriteProfileString	(_T("TCPIP"), dfcStr(_T("TCPIP_%d_RECIPE_FILE"),		m_nIndex), sInfo->m_strRecipeFile);

	ReadIniInfo ();
}

void	C_MainPGClientComm::OnReceive (int nErrorCode)
{
	static	char	buf[COMM_QUEUE_SIZE];
	int		nRead = Receive (buf, COMM_QUEUE_SIZE);
	if (!nRead) return; 

	for (int i = 0; i < nRead; i++) {

#ifdef _UNICODE
		mbtowc (&m_queue[m_nReadLen++], &buf[i], 1);
#else
		m_queue[m_nReadLen++] = buf[i];
#endif
		if (buf[i] == 0x03 || buf[i] == '\r') {
			m_queue[m_nReadLen] = 0;
			m_nReadLen = 0;
			ReceiveCode (m_queue);
		}
	}

	CSocket::OnReceive (nErrorCode);
}
//////////////////////////////////////////////////////


////////////////////////////////////////////////////RECEIVE
void	C_MainPGClientComm::ReceiveCode (TCHAR* pBuf)
{
	CString strRcv = pBuf;

	CString strMsg = dfcStr (_T("ME ◀ [%s%d] : %s"), m_sInfo.m_strName, m_sInfo.m_nHead, pBuf);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_MSG, NULL);

	if (m_pHandler) m_pHandler (strRcv.GetBuffer (), EVENT_COOM_PG_MAIN_CMD, NULL);

}

void	C_MainPGClientComm::WriteComm (LPCTSTR lpszMsg)
{
	CString strCmd = lpszMsg;

	Send (lpszMsg);

	CString strMsg = dfcStr (_T("ME ▶ [%s%d] : %s"), m_sInfo.m_strName, m_sInfo.m_nHead, strCmd);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_MSG, NULL);
}
