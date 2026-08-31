#include "pch.h"
#include "dfcComm.h"
#include "EnsisPGComm.h"

C_EnsisPGComm::C_EnsisPGComm (PG_EVENT_HANDLER pHandler, CString IniPath, int nIndex)
{
	m_nRunning	  = COMM_RS_FINISH;
	m_pHandler	  = pHandler;
	m_nIndex	  = nIndex;			//통신 개수 따라 증가 Ini파일 접근시만 사용
	
	memset (m_bWaitItem, FALSE, sizeof (BOOL) * PG_WAIT_MAX);

	m_path = IniPath;
	ReadIniInfo ();

	if (GetCommUse ()) ConnectCtrl (TRUE);

	//
	m_nErrorCode = -1;
	m_nAgingTime = 0;
	m_nStick	 = 0;
	m_bRunning   = FALSE;
	memset (m_nPGStatus, FALSE, sizeof (int) * PG_MAX_STICK * PG_MAX_CELL);
	//
}

C_EnsisPGComm::~C_EnsisPGComm ()
{

}

void	C_EnsisPGComm::OnClose (int nErrorCode)
{
	C_ClientSocket::OnClose (nErrorCode);

	CString strMsg;
	strMsg = dfcStr (_T("★ [%s%d] : Close"), m_sInfo.m_strName, m_sInfo.m_nHead);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_DISCONNECT, m_sInfo.m_nHead);
}

void	C_EnsisPGComm::OnConnect (int nErrorCode)
{
	C_ClientSocket::OnConnect (nErrorCode);
}

void	C_EnsisPGComm::ConnectCtrl (BOOL bConnect)
{
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

void	C_EnsisPGComm::ReadIniInfo ()
{
	m_sInfo.m_bUse			= GetProfileInt		(_T("TCPIP"), dfcStr (_T("TCPIP_%d_USE"),				m_nIndex), FALSE);
	m_sInfo.m_bOutOfService = GetProfileInt		(_T("TCPIP"), dfcStr (_T("TCPIP_%d_OUT_OF_SERVICE"),	m_nIndex), TRUE);
	m_sInfo.m_strName		= GetProfileString	(_T("TCPIP"), dfcStr (_T("TCPIP_%d_NAME"),				m_nIndex), _T("PG"));
	m_sInfo.m_nHead			= GetProfileInt		(_T("TCPIP"), dfcStr (_T("TCPIP_%d_HEAD_NUM"),			m_nIndex), 1);
	m_sInfo.m_strIP			= GetProfileString	(_T("TCPIP"), dfcStr (_T("TCPIP_%d_IP"),				m_nIndex), _T("192.168.0.1"));
	m_sInfo.m_nPortNum		= GetProfileInt		(_T("TCPIP"), dfcStr (_T("TCPIP_%d_PORT"),				m_nIndex), 4001);
	m_sInfo.m_strResultDir  = GetProfileString	(_T("TCPIP"), dfcStr (_T("TCPIP_%d_RESULT_DIRECTORY"),	m_nIndex), _T("Z:\\"));
	m_sInfo.m_strRecipeFile = GetProfileString	(_T("TCPIP"), dfcStr (_T("TCPIP_%d_RECIPE_FILE"),		m_nIndex), _T("Z:\\Recipe.ini"));
}

void	C_EnsisPGComm::WriteIniInfo (S_TcpIpInfo *sInfo)
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

void	C_EnsisPGComm::SetWaitItem (int nItem, BOOL bSet)
{
	if (nItem == -1) {
		for (int i = 0; i < PG_WAIT_MAX; i++) m_bWaitItem[i] = bSet;
	}
	else m_bWaitItem[nItem] = bSet;
}

BOOL	C_EnsisPGComm::GetWaitItem (int nItem)
{
	return m_bWaitItem[nItem];
}

void	C_EnsisPGComm::TimeOutAlarm (CString strCmd, int nTime)
{
	CString strMsg;
	strMsg = dfcStr (_T("★ [%s%d] : %s TimeOut...%d"), m_sInfo.m_strName, m_sInfo.m_nHead, strCmd, nTime);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_TIMEOUT, m_sInfo.m_nHead);
}

void	C_EnsisPGComm::NakAlarm (CString strCmd)
{
	CString strMsg;
	strMsg = dfcStr (_T("★ [%s%d] : %s NAK, ErrorCode : %02d"), m_sInfo.m_strName, m_sInfo.m_nHead, strCmd, m_nErrorCode);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_NAK, m_sInfo.m_nHead);
}

void	C_EnsisPGComm::SendAlarm (CString strCmd, int nTime)
{
	if		(m_nRunning == COMM_RS_TIMEOUT) TimeOutAlarm (strCmd, nTime);
	else if (m_nRunning == COMM_RS_ERROR)	NakAlarm (strCmd);
}

void	C_EnsisPGComm::ParsingPGData (int nItem, CString strData)
{
	return;
}

void	C_EnsisPGComm::SetEmptyStick (int nStick)
{
	if (nStick == -1) {
		memset (m_nPGStatus, FALSE, sizeof (int) * PG_MAX_STICK * PG_MAX_CELL);
	}
	else {
		for (int i = 0; i < PG_MAX_CELL; i++) m_nPGStatus[nStick][i] = PG::CELL_IDLE;
	}
}

int		C_EnsisPGComm::GetCellState (int nStick, int nCell)
{
	int nCellState = -1;
	if (nStick == -1)	nCellState = m_nPGStatus[m_nStick][nCell];
	else				nCellState = m_nPGStatus[nStick][nCell];

	return nCellState;
}
//////////////////////////////////////////////////////
BOOL	C_EnsisPGComm::WaitSTOP ()
{
	return FALSE;
}

BOOL	C_EnsisPGComm::SetTestStop ()
{
	return FALSE;
}

BOOL	C_EnsisPGComm::SetTurnOff ()
{
	return FALSE;
}

BOOL	C_EnsisPGComm::ActChangeProgram (int nSelProgram)
{
	return FALSE;
}

BOOL	C_EnsisPGComm::ActSelectCell (int nArea, int nCell)
{
	return FALSE;
}

BOOL	C_EnsisPGComm::ActChangeModel (CString strModelName, int nSelPG)
{
	return FALSE;
}

BOOL	C_EnsisPGComm::ActChangeRecipe (CString strRcpName)
{
	return FALSE;
}

BOOL	C_EnsisPGComm::ActRunStatus ()
{
	return FALSE;
}

BOOL	C_EnsisPGComm::ActAging (int nStick, BOOL bRtestMaskUse)
{
	return FALSE;
}

BOOL	C_EnsisPGComm::ActTurnOn (int nSelCell)
{
	return FALSE;
}

BOOL	C_EnsisPGComm::ActTurnOff (int nSelCell)
{
	return FALSE;
}

BOOL	C_EnsisPGComm::ActPattrenNext (int nSelCell)
{
	return FALSE;
}

BOOL	C_EnsisPGComm::ActPattrenPrev (int nSelCell)
{
	return FALSE;
}

BOOL	C_EnsisPGComm::ActOST (int nStick, int nSelCell)
{
	return FALSE;
}

BOOL	C_EnsisPGComm::ActPTP (int nStick, int nSelCell)
{
	return FALSE;
}

BOOL	C_EnsisPGComm::ActRTest (int nStick, int nSelCell)
{
	return FALSE;
}

void	C_EnsisPGComm::ReceiveEventOK (int nEvent, TCHAR* pBuf)
{
	SetWaitItem (nEvent, TRUE);
	RunModalFinish (FALSE);
}

void	C_EnsisPGComm::ReceiveEventNG ()
{
	m_nRunning = COMM_RS_ERROR;
}

////////////////////////////////////////////////////RECEIVE
void	C_EnsisPGComm::ReceiveCode (TCHAR* pBuf)
{
	CString strMsg = dfcStr (_T("◀ [%s%d] : %s"), m_sInfo.m_strName, m_sInfo.m_nHead, pBuf);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_MSG, NULL);
}

void	C_EnsisPGComm::WriteComm (LPCTSTR lpszMsg)
{
	CString strCmd = lpszMsg;

	Send (lpszMsg);

	CString strMsg = dfcStr (_T("ME ▶ [%s%d] : %s"), m_sInfo.m_strName, m_sInfo.m_nHead, strCmd);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_MSG, NULL);
}

BOOL	C_EnsisPGComm::RunModalLoop (int nWaitItem, DWORD dwTime)
{
	m_nRunning = COMM_RS_RUN;

	DWORD dwTimeout = dwTime;
	DWORD dwBeginTick = GetTickCount ();
	DWORD dwCurrentTick = 0;
	DWORD dwAddTime = 0;

	while (m_nRunning == COMM_RS_RUN) {
		dwCurrentTick = GetTickCount ();

		if (dwCurrentTick < dwBeginTick) {
			dwBeginTick = dwCurrentTick;
			dwTimeout -= dwAddTime;
		}
		else dwAddTime = (dwCurrentTick - dwBeginTick);

		if ((dwCurrentTick - dwBeginTick) > dwTimeout) m_nRunning = COMM_RS_TIMEOUT;

		if (GetWaitItem (nWaitItem)) m_nRunning = COMM_RS_FINISH;
	}

	return m_nRunning == COMM_RS_FINISH;
}

BOOL	C_EnsisPGComm::RunModalFinish (BOOL bFinishSet)
{
	if (bFinishSet) m_nRunning = COMM_RS_FINISH;

	CString strMsg = dfcStr (_T("★ [%s%d] : RunModalFinish"), m_sInfo.m_strName, m_sInfo.m_nHead);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_MSG, NULL);

	return TRUE;
}
