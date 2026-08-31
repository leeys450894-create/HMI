#include "pch.h"
#include "dfcComm.h"
#include "ElpPGComm.h"

C_ElpPGComm::C_ElpPGComm (PG_EVENT_HANDLER pHandler, CString IniPath, int nIndex)
{
	m_nRunning	  = COMM_RS_FINISH; //대기함수 진행 플래그
	m_pHandler	  = pHandler;		//EXE 쪽으로 메시지 전달
	m_nIndex	  = nIndex;			//통신 개수 따라 증가 Ini파일 접근시만 사용
	
	memset (m_bWaitItem, FALSE, sizeof (BOOL) * PG_WAIT_MAX); //대기 완료 아이템

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

C_ElpPGComm::~C_ElpPGComm ()
{

}

void	C_ElpPGComm::OnClose (int nErrorCode)
{
	C_ClientSocket::OnClose (nErrorCode);

	CString strMsg;
	strMsg = dfcStr (_T("★ [%s%d] : Close"), m_sInfo.m_strName, m_sInfo.m_nHead);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_DISCONNECT, m_sInfo.m_nHead);
}

void	C_ElpPGComm::OnConnect (int nErrorCode)
{
	C_ClientSocket::OnConnect (nErrorCode);
}

void	C_ElpPGComm::ConnectCtrl (BOOL bConnect)
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

void	C_ElpPGComm::ReadIniInfo ()
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

void	C_ElpPGComm::WriteIniInfo (S_TcpIpInfo *sInfo)
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

void	C_ElpPGComm::SetWaitItem (int nItem, BOOL bSet)
{
	if (nItem == -1) {
		for (int i = 0; i < PG_WAIT_MAX; i++) m_bWaitItem[i] = bSet;
	}
	else m_bWaitItem[nItem] = bSet;
}

BOOL	C_ElpPGComm::GetWaitItem (int nItem)
{
	return m_bWaitItem[nItem];
}

void	C_ElpPGComm::TimeOutAlarm (CString strCmd, int nTime)
{
	CString strMsg;
	strMsg = dfcStr (_T("★ [%s%d] : %s TimeOut...%d"), m_sInfo.m_strName, m_sInfo.m_nHead, strCmd, nTime);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_TIMEOUT, m_sInfo.m_nHead);
}

void	C_ElpPGComm::NakAlarm (CString strCmd)
{
	CString strMsg;
	strMsg = dfcStr (_T("★ [%s%d] : %s NAK, ErrorCode : %02d"), m_sInfo.m_strName, m_sInfo.m_nHead, strCmd, m_nErrorCode);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_NAK, m_sInfo.m_nHead);
}

void	C_ElpPGComm::SendAlarm (CString strCmd, int nTime)
{
	if		(m_nRunning == COMM_RS_TIMEOUT) TimeOutAlarm (strCmd, nTime);
	else if (m_nRunning == COMM_RS_ERROR)	NakAlarm (strCmd);
}

void	C_ElpPGComm::ParsingPGData (int nItem, CString strData)
{
	int nStat1 = -1, nStat2 = -1, nStat3 = -1;

	switch (nItem) {
		case PG_OST:	nStat1 = PG::CELL_PTP_NOR;		nStat2 = PG::CELL_PTP_SHORT;	nStat3 = PG::CELL_PTP_OPEN;		break;
		case PG_PTP:	nStat1 = PG::CELL_OST_NOR;		nStat2 = PG::CELL_OST_SHORT;	nStat3 = PG::CELL_OST_OPEN;		break;
		case PG_RTEST:	nStat1 = PG::CELL_RTEST_PASS;	nStat2 = PG::CELL_RTEST_OVER;	nStat3 = PG::CELL_RTEST_UNDER;	break;
		case PG_AGING:	nStat1 = PG::CELL_AGING_NG;		nStat2 = PG::CELL_AGING_OK;										break;
		default:		return;
	}

	if (nItem == PG_AGING) {
		for (int i = 0; i < strData.GetLength(); i++) {
			if (m_nPGStatus[m_nStick][i] == PG::CELL_IDLE	 || m_nPGStatus[m_nStick][i] == PG::CELL_PTP_NOR ||
				m_nPGStatus[m_nStick][i] == PG::CELL_OST_NOR || m_nPGStatus[m_nStick][i] == PG::CELL_RTEST_PASS) 
			{
				if		(_ttoi ((CString) strData.GetAt (i)) == 0) { //Aging NG
					m_nPGStatus[m_nStick][i] = nStat1;
				}
				else if (_ttoi ((CString) strData.GetAt (i)) == 1) { //Aging OK
					m_nPGStatus[m_nStick][i] = nStat2;
				}
			}
		}
	}
	else {
		for (int i = 0; i < strData.GetLength(); i++) {
			if		(_ttoi ((CString) strData.GetAt (i)) == 0) { //PTP : Normal, OST : Normal, RTest : Pass
				m_nPGStatus[m_nStick][i] = nStat1;
			}
			else if (_ttoi ((CString) strData.GetAt (i)) == 1) { //PTP : Short,  OST : Short,  RTest : Over
				m_nPGStatus[m_nStick][i] = nStat2;
			}
			else if (_ttoi ((CString) strData.GetAt (i)) == 2) { //PTP : Open,   OST : Open,   RTest : Under
				m_nPGStatus[m_nStick][i] = nStat3;
			}
		}
	}
}

void	C_ElpPGComm::SetEmptyStick (int nStick)
{
	if (nStick == -1) {
		memset (m_nPGStatus, FALSE, sizeof (int) * PG_MAX_STICK * PG_MAX_CELL);
	}
	else {
		for (int i = 0; i < PG_MAX_CELL; i++) m_nPGStatus[nStick][i] = PG::CELL_IDLE;
	}
}

int		C_ElpPGComm::GetCellState (int nStick, int nCell)
{
	int nCellState = -1;
	if (nStick == -1)	nCellState = m_nPGStatus[m_nStick][nCell];
	else				nCellState = m_nPGStatus[nStick][nCell];

	return nCellState;
}

//////////////////////////////////////////////////////
BOOL	C_ElpPGComm::WaitSTOP ()
{
	SetTestStop ();
	Sleep (200);
	SetTurnOff ();

	m_nRunning = COMM_RS_STOP;

	SetWaitItem (PG_WAIT_CLEAR, FALSE);

	CString strMsg = dfcStr (_T("★ [%s%d] : LoopStop"), m_sInfo.m_strName, m_sInfo.m_nHead);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_MSG, NULL);

	return TRUE;
}

BOOL	C_ElpPGComm::SetTestStop ()
{
	if (!IsConnected ()) return FALSE;

	CString strCmd;
	strCmd.AppendFormat (_T("%c"),   PGSTX);
	strCmd.AppendFormat (_T("%s"),	 S10F1);
	strCmd.AppendFormat (_T("%c"),   PGETX);

	WriteComm (strCmd);

	return TRUE;
}

BOOL	C_ElpPGComm::SetTurnOff ()
{
	if (!IsConnected ()) return FALSE;

	CString strCmd;
	strCmd.AppendFormat (_T("%c"),   PGSTX);
	strCmd.AppendFormat (_T("%s"),	 S10F3);
	strCmd.AppendFormat (_T("%c"),   PGETX);

	WriteComm (strCmd);

	return TRUE;
}

BOOL	C_ElpPGComm::ActChangeProgram (int nSelProgram)
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (PG_WAIT_PROGRAM_SET_OK, FALSE);

	CString strCmd;
	strCmd.AppendFormat (_T("%c"),   PGSTX);
	strCmd.AppendFormat (_T("%s"),	 S1F1);
	strCmd.AppendFormat (_T("%02d"), nSelProgram);
	strCmd.AppendFormat (_T("%c"),   PGETX);

	WriteComm (strCmd);

	DWORD dwTimeout = TIMEOUT_10;
	bRet = RunModalLoop (PG_WAIT_PROGRAM_SET_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("PROGRAM.CHANGE"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_ElpPGComm::ActSelectCell (int nArea, int nCell)
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (PG_WAIT_SELECT_CELL_OK, FALSE);

	CString strCmd;
	strCmd.AppendFormat (_T("%c"),   PGSTX);
	strCmd.AppendFormat (_T("%s"),   S1F5);
	strCmd.AppendFormat (_T("%d"),   nArea);
	strCmd.AppendFormat (_T("%02d"), nCell);
	strCmd.AppendFormat (_T("%c"),   PGETX);

	WriteComm (strCmd);

	DWORD dwTimeout = TIMEOUT_10;
	bRet = RunModalLoop (PG_WAIT_SELECT_CELL_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		if (nArea == PG_AREA_LEFT) SendAlarm (_T("CELL.SELECT.LEFT"), dwTimeout);
		else					   SendAlarm (_T("CELL.SELECT.RIGHT"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_ElpPGComm::ActChangeModel (CString strModelName, int nSelPG)
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (PG_WAIT_MODEL_CHANGE_CMD_OK, FALSE);
	SetWaitItem (PG_WAIT_MODEL_CHANGE_END_OK, FALSE);

	CString strCmd;
	strCmd.AppendFormat (_T("%c"), PGSTX);
	strCmd.AppendFormat (_T("%s"), S2F1);
	strCmd.AppendFormat (_T("%d"), nSelPG); // 1 : 전체 PG
	strCmd.AppendFormat (_T("%s"), (LPCWSTR) strModelName);
	strCmd.AppendFormat (_T("%c"), PGETX);

	WriteComm (strCmd);

	DWORD dwTimeout = TIMEOUT_10;
	bRet = RunModalLoop (PG_WAIT_MODEL_CHANGE_CMD_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("MODEL.CHANGE.CMD"), dwTimeout);
		return FALSE;
	}

	dwTimeout = TIMEOUT_30;
	bRet = RunModalLoop (PG_WAIT_MODEL_CHANGE_END_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("MODEL.CHANGE.END"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_ElpPGComm::ActChangeRecipe (CString strRcpName)
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (PG_WAIT_RECIPE_CHANGE_OK, FALSE);

	CString strCmd;
	strCmd.AppendFormat (_T("%c"), PGSTX);
	strCmd.AppendFormat (_T("%s"), S2F9);
	strCmd.AppendFormat (_T("%s"), (LPCWSTR) strRcpName);
	strCmd.AppendFormat (_T("%c"), PGETX);

	WriteComm (strCmd);

	DWORD dwTimeout = TIMEOUT_10;
	bRet = RunModalLoop (PG_WAIT_RECIPE_CHANGE_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("RECIPE.CHANGE"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_ElpPGComm::ActRunStatus ()
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (PG_WAIT_STATUS_RCV_OK, FALSE);

	CString strCmd;
	strCmd.AppendFormat (_T("%c"), PGSTX);
	strCmd.AppendFormat (_T("%s"), S5F1);
	strCmd.AppendFormat (_T("%c"), PGETX);

	WriteComm (strCmd);

	DWORD dwTimeout = TIMEOUT_10;
	bRet = RunModalLoop (PG_WAIT_STATUS_RCV_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("CHECK.STATUS"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_ElpPGComm::ActAging (int nStick, BOOL bRtestMaskUse)
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (PG_WAIT_AGING_TEST_CMD_OK, FALSE);
	SetWaitItem (PG_WAIT_AGING_TEST_END_OK, FALSE);

	CString strCmd;
	strCmd.AppendFormat (_T("%c"), PGSTX);
	strCmd.AppendFormat (_T("%s"), S6F1);
	strCmd.AppendFormat (_T("%d"), (int) bRtestMaskUse);
	strCmd.AppendFormat (_T("%c"), PGETX);

	SetPGStick (nStick);

	WriteComm (strCmd);

	DWORD dwTimeout = TIMEOUT_10;
	bRet = RunModalLoop (PG_WAIT_AGING_TEST_CMD_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("AGING.TEST.CMD"), dwTimeout);
		return FALSE;
	}

	dwTimeout = TIMEOUT_300/*m_nAgingTime*/; //Aging 시간 설정 확인
	bRet = RunModalLoop (PG_WAIT_AGING_TEST_END_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("AGING.TEST.END"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_ElpPGComm::ActTurnOn (int nSelCell)
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (PG_WAIT_CELL_CMD_OK, FALSE);
	SetWaitItem (PG_WAIT_TURN_ON_END_OK, FALSE);

	CString strCmd;
	strCmd.AppendFormat (_T("%c"),   PGSTX);
	strCmd.AppendFormat (_T("%s"),	 S8F1);
	strCmd.AppendFormat (_T("%s"),	 TURN_ON);
	strCmd.AppendFormat (_T("%02d"), nSelCell); // 0 : 전체 Cell
	strCmd.AppendFormat (_T("%c"),   PGETX);

	WriteComm (strCmd);

	DWORD dwTimeout = TIMEOUT_10;
	bRet = RunModalLoop (PG_WAIT_CELL_CMD_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("CELL.CMD"), dwTimeout);
		return FALSE;
	}

	bRet = RunModalLoop (PG_WAIT_TURN_ON_END_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("TURN.ON.END"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_ElpPGComm::ActTurnOff (int nSelCell)
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (PG_WAIT_CELL_CMD_OK, FALSE);
	SetWaitItem (PG_WAIT_TURN_OFF_END_OK, FALSE);

	CString strCmd;
	strCmd.AppendFormat (_T("%c"),   PGSTX);
	strCmd.AppendFormat (_T("%s"),	 S8F1);
	strCmd.AppendFormat (_T("%s"),	 TURN_OFF);
	strCmd.AppendFormat (_T("%02d"), nSelCell); // 0 : 전체 Cell
	strCmd.AppendFormat (_T("%c"),   PGETX);

	WriteComm (strCmd);

	DWORD dwTimeout = TIMEOUT_10;
	bRet = RunModalLoop (PG_WAIT_CELL_CMD_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("CELL.CMD"), dwTimeout);
		return FALSE;
	}

	bRet = RunModalLoop (PG_WAIT_TURN_OFF_END_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("TURN.OFF.END"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_ElpPGComm::ActPattrenNext (int nSelCell)
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (PG_WAIT_CELL_CMD_OK, FALSE);
	SetWaitItem (PG_WAIT_PTRN_NEXT_END_OK, FALSE);

	CString strCmd;
	strCmd.AppendFormat (_T("%c"),   PGSTX);
	strCmd.AppendFormat (_T("%s"),	 S8F1);
	strCmd.AppendFormat (_T("%s"),	 PTRN_NEXT);
	strCmd.AppendFormat (_T("%02d"), nSelCell); // 0 : 전체 Cell
	strCmd.AppendFormat (_T("%c"),   PGETX);

	WriteComm (strCmd);

	DWORD dwTimeout = TIMEOUT_10;
	bRet = RunModalLoop (PG_WAIT_CELL_CMD_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("CELL.CMD"), dwTimeout);
		return FALSE;
	}

	bRet = RunModalLoop (PG_WAIT_PTRN_NEXT_END_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("PATTERN.NEXT.END"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_ElpPGComm::ActPattrenPrev (int nSelCell)
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (PG_WAIT_CELL_CMD_OK, FALSE);
	SetWaitItem (PG_WAIT_PTRN_PREV_END_OK, FALSE);

	CString strCmd;
	strCmd.AppendFormat (_T("%c"),   PGSTX);
	strCmd.AppendFormat (_T("%s"),	 S8F1);
	strCmd.AppendFormat (_T("%s"),	 PTRN_PREV);
	strCmd.AppendFormat (_T("%02d"), nSelCell); // 0 : 전체 Cell
	strCmd.AppendFormat (_T("%c"),   PGETX);

	WriteComm (strCmd);

	DWORD dwTimeout = TIMEOUT_10;
	bRet = RunModalLoop (PG_WAIT_CELL_CMD_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("CELL.CMD"), dwTimeout);
		return FALSE;
	}

	bRet = RunModalLoop (PG_WAIT_PTRN_PREV_END_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("PATTERN.PREV.END"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_ElpPGComm::ActOST (int nStick, int nSelCell)
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (PG_WAIT_CELL_CMD_OK, FALSE);
	SetWaitItem (PG_WAIT_OST_END_OK, FALSE);

	CString strCmd;
	strCmd.AppendFormat (_T("%c"),   PGSTX);
	strCmd.AppendFormat (_T("%s"),	 S8F1);
	strCmd.AppendFormat (_T("%s"),	 OST);
	strCmd.AppendFormat (_T("%02d"), nSelCell); // 0 : 전체 Cell
	strCmd.AppendFormat (_T("%c"),   PGETX);

	SetPGStick (nStick);

	WriteComm (strCmd);

	DWORD dwTimeout = TIMEOUT_10;
	bRet = RunModalLoop (PG_WAIT_CELL_CMD_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("CELL.CMD"), dwTimeout);
		return FALSE;
	}

	bRet = RunModalLoop (PG_WAIT_OST_END_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("OST.END"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_ElpPGComm::ActPTP (int nStick, int nSelCell)
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (PG_WAIT_CELL_CMD_OK, FALSE);
	SetWaitItem (PG_WAIT_PTP_END_OK, FALSE);

	CString strCmd;
	strCmd.AppendFormat (_T("%c"),   PGSTX);
	strCmd.AppendFormat (_T("%s"),	 S8F1);
	strCmd.AppendFormat (_T("%s"),	 PTP);
	strCmd.AppendFormat (_T("%02d"), nSelCell); // 0 : 전체 Cell
	strCmd.AppendFormat (_T("%c"),   PGETX);

	SetPGStick (nStick);

	WriteComm (strCmd);

	DWORD dwTimeout = TIMEOUT_10;
	bRet = RunModalLoop (PG_WAIT_CELL_CMD_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("CELL.CMD"), dwTimeout);
		return FALSE;
	}

	bRet = RunModalLoop (PG_WAIT_PTP_END_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("PTP.END"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_ElpPGComm::ActRTest (int nStick, int nSelCell)
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (PG_WAIT_CELL_CMD_OK, FALSE);
	SetWaitItem (PG_WAIT_RTEST_END_OK, FALSE);

	CString strCmd;
	strCmd.AppendFormat (_T("%c"),   PGSTX);
	strCmd.AppendFormat (_T("%s"),	 S8F1);
	strCmd.AppendFormat (_T("%s"),	 RTEST);
	strCmd.AppendFormat (_T("%02d"), nSelCell); // 0 : 전체 Cell
	strCmd.AppendFormat (_T("%c"),   PGETX);

	SetPGStick (nStick);

	WriteComm (strCmd);

	DWORD dwTimeout = TIMEOUT_10;
	bRet = RunModalLoop (PG_WAIT_CELL_CMD_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("CELL.CMD"), dwTimeout);
		return FALSE;
	}

	dwTimeout = TIMEOUT_20;
	bRet = RunModalLoop (PG_WAIT_RTEST_END_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("RTEST.END"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

void	C_ElpPGComm::ReceiveEventOK (int nEvent, TCHAR* pBuf)
{
	SetWaitItem (nEvent, TRUE);
	RunModalFinish (FALSE);
}

void	C_ElpPGComm::ReceiveEventNG ()
{
	m_nRunning = COMM_RS_ERROR;
}

////////////////////////////////////////////////////RECEIVE
void	C_ElpPGComm::ReceiveCode (TCHAR* pBuf)
{
	CString strRcv = pBuf;

	BOOL bReceive = FALSE;
	CString strTemp;
	int	nBeginPos = -1;
	int nAck	  = -1;
	int nBeginMsg = -1, nEndMsg = -1;

	nBeginMsg = strRcv.Find (PGSTX);
	nEndMsg   = strRcv.Find (PGETX);

	if (nBeginMsg >= 0 && nEndMsg >= 0) {
		nBeginPos = nBeginMsg + 1;
		bReceive = TRUE;
	}

	if (!bReceive) return;

	nAck		 = _ttoi (strRcv.Mid (nBeginPos + 4, 2));
	m_nErrorCode = _ttoi (strRcv.Mid (nBeginPos + 6, 2));
	//nResult = _ttoi (strRcv.Mid (nBeginPos + 8, ?));

	if (nAck == PGACK) {
		if		(strRcv.Find (S1F2)  >= 0) { //프로그램 변경 완료
			ReceiveEventOK (PG_WAIT_PROGRAM_SET_OK, pBuf);
		}
		else if (strRcv.Find (S1F6) >= 0) { //CELL 선택 완료
			ReceiveEventOK (PG_WAIT_SELECT_CELL_OK, pBuf);
		}
		else if (strRcv.Find (S2F2) >= 0) { //모델 변경 명령 응답
			ReceiveEventOK (PG_WAIT_MODEL_CHANGE_CMD_OK, pBuf);
		}
		else if (strRcv.Find (S2F2) >= 0) { //모델 변경 완료
			ReceiveEventOK (PG_WAIT_MODEL_CHANGE_END_OK, pBuf);
		}
		else if (strRcv.Find (S2F10) >= 0) { //레시피 변경 완료
			ReceiveEventOK (PG_WAIT_RECIPE_CHANGE_OK, pBuf);
		}
		else if (strRcv.Find (S5F2) >= 0) { //PG Run 상태 확인 완료
			m_bRunning = _ttoi (strRcv.Mid (nBeginPos + 8, 1));
			ReceiveEventOK (PG_WAIT_STATUS_RCV_OK, pBuf);
		}
		else if (strRcv.Find (S6F2) >= 0) { //Aging Test 명령 응답
			m_nAgingTime = _ttoi (strRcv.Mid (nBeginPos + 8, 10));
			ReceiveEventOK (PG_WAIT_AGING_TEST_CMD_OK, pBuf);
		}
		else if (strRcv.Find (S6F3) >= 0) { //Aging Test 명령 완료 //결과 개수 변함
			ParsingPGData (PG_AGING, strRcv.Mid (nBeginPos + 8, nEndMsg - 8));
			ReceiveEventOK (PG_WAIT_AGING_TEST_END_OK, pBuf);
		}
		else if (strRcv.Find (S8F2) >= 0) { //Cell 명령 응답
			ReceiveEventOK (PG_WAIT_CELL_CMD_OK, pBuf);
		}
		else if (strRcv.Find (S8F3) >= 0) { //Cell 명령 완료
			strTemp = strRcv.Mid (nBeginPos + 8, 2);
			if		(strTemp.Find (TURN_ON) >= 0)   ReceiveEventOK (PG_WAIT_TURN_ON_END_OK,   pBuf);
			else if (strTemp.Find (TURN_OFF) >= 0)  ReceiveEventOK (PG_WAIT_TURN_OFF_END_OK,  pBuf);
			else if (strTemp.Find (PTRN_NEXT) >= 0) ReceiveEventOK (PG_WAIT_PTRN_NEXT_END_OK, pBuf);
			else if (strTemp.Find (PTRN_PREV) >= 0) ReceiveEventOK (PG_WAIT_PTRN_PREV_END_OK, pBuf);
			else if (strTemp.Find (OST) >= 0) {
				ParsingPGData (PG_OST, strRcv.Mid (nBeginPos + 12, nEndMsg - 12));
				ReceiveEventOK (PG_WAIT_OST_END_OK, pBuf);
			}
			else if (strTemp.Find (PTP) >= 0) {
				ParsingPGData (PG_PTP, strRcv.Mid (nBeginPos + 12, nEndMsg - 12));
				ReceiveEventOK (PG_WAIT_PTP_END_OK, pBuf);
			}
			else if (strTemp.Find (RTEST) >= 0) {
				ParsingPGData (PG_RTEST, strRcv.Mid (nBeginPos + 12, nEndMsg - 12));
				ReceiveEventOK (PG_WAIT_RTEST_END_OK, pBuf);
			}
		}
		else if (strRcv.Find (S10F2) >= 0) { //Test Stop

		}
		else if (strRcv.Find (S10F4) >= 0) { //Turn Off

		}
	}
	else ReceiveEventNG ();

	CString strMsg = dfcStr (_T("◀ [%s%d] : %s"), m_sInfo.m_strName, m_sInfo.m_nHead, pBuf);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_MSG, NULL);
}

void	C_ElpPGComm::WriteComm (LPCTSTR lpszMsg)
{
	CString strCmd = lpszMsg;

	Send (lpszMsg);

	CString strMsg = dfcStr (_T("▶ [%s%d] : %s"), m_sInfo.m_strName, m_sInfo.m_nHead, strCmd);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_MSG, NULL);
}

BOOL	C_ElpPGComm::RunModalLoop (int nWaitItem, DWORD dwTime)
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

BOOL	C_ElpPGComm::RunModalFinish (BOOL bFinishSet)
{
	if (bFinishSet) m_nRunning = COMM_RS_FINISH;

	CString strMsg = dfcStr (_T("★ [%s%d] : RunModalFinish"), m_sInfo.m_strName, m_sInfo.m_nHead);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_MSG, NULL);

	return TRUE;
}
