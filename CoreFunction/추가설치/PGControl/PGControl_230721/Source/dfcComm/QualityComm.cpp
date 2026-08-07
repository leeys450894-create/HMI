#include "pch.h"
#include "dfcComm.h"
#include "QualityComm.h"

C_QualityComm::C_QualityComm (QUALITY_EVENT_HANDLER pHandler, CString IniPath, int nIndex)
{
	m_nRunning	  = COMM_RS_FINISH;
	m_pHandler	  = pHandler;
	m_nIndex	  = nIndex;			//통신 개수 따라 증가 Ini파일 접근시만 사용
	m_nMoveFilter = 0;
	m_path		  = IniPath;

	memset (m_bWaitItem, FALSE, sizeof (BOOL) * QUALITY_WAIT_MAX);

	ReadIniInfo ();

	if (GetCommUse ()) ConnectCtrl (TRUE);
}

C_QualityComm::~C_QualityComm ()
{

}

void	C_QualityComm::OnClose (int nErrorCode)
{
	C_ClientSocket::OnClose (nErrorCode);

	CString strMsg;
	strMsg = dfcStr (_T("★ [%s%d] : Close"), m_sInfo.m_strName, m_sInfo.m_nHead);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_DISCONNECT, m_sInfo.m_nHead);
}

void	C_QualityComm::OnConnect (int nErrorCode)
{
	C_ClientSocket::OnConnect (nErrorCode);
}

void	C_QualityComm::ConnectCtrl (BOOL bConnect)
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

void	C_QualityComm::ReadIniInfo ()
{
	m_sInfo.m_bUse			= GetProfileInt		(_T("TCPIP"), dfcStr (_T("TCPIP_%d_USE"),				m_nIndex), FALSE);
	m_sInfo.m_bOutOfService = GetProfileInt		(_T("TCPIP"), dfcStr (_T("TCPIP_%d_OUT_OF_SERVICE"),	m_nIndex), TRUE);
	m_sInfo.m_strName		= GetProfileString	(_T("TCPIP"), dfcStr (_T("TCPIP_%d_NAME"),				m_nIndex), _T("화질"));
	m_sInfo.m_nHead			= GetProfileInt		(_T("TCPIP"), dfcStr (_T("TCPIP_%d_HEAD_NUM"),			m_nIndex), 1);
	m_sInfo.m_strIP			= GetProfileString	(_T("TCPIP"), dfcStr (_T("TCPIP_%d_IP"),				m_nIndex), _T("192.168.0.1"));
	m_sInfo.m_nPortNum		= GetProfileInt		(_T("TCPIP"), dfcStr (_T("TCPIP_%d_PORT"),				m_nIndex), 4001);
	m_sInfo.m_strResultDir  = GetProfileString	(_T("TCPIP"), dfcStr (_T("TCPIP_%d_RESULT_DIRECTORY"),	m_nIndex), _T("Z:\\"));
	m_sInfo.m_strRecipeFile = GetProfileString	(_T("TCPIP"), dfcStr (_T("TCPIP_%d_RECIPE_FILE"),		m_nIndex), _T("Z:\\Recipe.ini"));
}

void	C_QualityComm::WriteIniInfo (S_TcpIpInfo *sInfo)
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

void	C_QualityComm::SetWaitItem (int nItem, BOOL bSet)
{
	if (nItem == -1) {
		for (int i = 0; i < QUALITY_WAIT_MAX; i++) m_bWaitItem[i] = bSet;
	}
	else m_bWaitItem[nItem] = bSet;
}

BOOL	C_QualityComm::GetWaitItem (int nItem)
{
	return m_bWaitItem[nItem];
}

void	C_QualityComm::TimeOutAlarm (CString strCmd, int nTime)
{
	CString strMsg;
	strMsg = dfcStr (_T("★ [%s%d] : %s TimeOut...%d"), m_sInfo.m_strName, m_sInfo.m_nHead, strCmd, nTime);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_TIMEOUT, m_sInfo.m_nHead);
}

void	C_QualityComm::NakAlarm (CString strCmd)
{
	CString strMsg;
	strMsg = dfcStr (_T("★ [%s%d] : %s NAK"), m_sInfo.m_strName, m_sInfo.m_nHead, strCmd);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_NAK, m_sInfo.m_nHead);
}

void	C_QualityComm::SendAlarm (CString strCmd, int nTime)
{
	if		(m_nRunning == COMM_RS_TIMEOUT) TimeOutAlarm (strCmd, nTime);
	else if (m_nRunning == COMM_RS_ERROR)	NakAlarm (strCmd);
}

//////////////////////////////////////////////////////
BOOL	C_QualityComm::WaitSTOP ()
{
	m_nRunning = COMM_RS_STOP;

	SetWaitItem (QUALITY_WAIT_CLEAR, FALSE);

	CString strMsg = dfcStr (_T("★ [%s%d] : LoopStop"), m_sInfo.m_strName, m_sInfo.m_nHead);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_MSG, NULL);

	return TRUE;
}

BOOL	C_QualityComm::SetFilterMoveMsg (BOOL bMoveOK)
{
	if (!IsConnected()) return FALSE;

	CString strMsg;

	if (m_nMoveFilter == QUALITY_FILTER_IN)	strMsg.AppendFormat (_T("MOT.POL.MOVE.IN"));
	else									strMsg.AppendFormat (_T("MOT.POL.MOVE.OUT"));

	if (bMoveOK)	strMsg.AppendFormat(_T(".OK"));
	else			strMsg.AppendFormat(_T(".NG"));

	WriteComm (strMsg);

	return TRUE;
}

BOOL	C_QualityComm::SetInspectStart (CString strInnerID, int nSheetRow, CString strCellID)
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (QUALITY_WAIT_GRAB_START_OK, FALSE);
	SetWaitItem (QUALITY_WAIT_GRAB_END_OK,	FALSE);
	WriteComm (dfcStr (_T("MOT.GRAB.START.%s.%s"), strInnerID, strCellID));

	QualityWaitSet (strInnerID, nSheetRow, strCellID);

	return TRUE;
}

BOOL	C_QualityComm::WaitInspectEnd ()
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	DWORD dwTimeout = TIMEOUT_20;
	bRet = RunModalLoop (QUALITY_WAIT_GRAB_START_OK, dwTimeout);
	if (!bRet) {
		SendAlarm (_T("GRAB.START.OK"), dwTimeout);
		return FALSE;
	}

	bRet = RunModalLoop (QUALITY_WAIT_GRAB_END_OK, dwTimeout);
	if (!bRet) {
		SendAlarm (_T("GRAB.END.OK"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_QualityComm::ActChangeRecipe (CString strRcpName)
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	C_IniFile	fileRcp (m_sInfo.m_strRecipeFile);
	fileRcp.WriteProfileString (_T("Current Recipe"), _T("Recipe Name"), strRcpName);

	SetWaitItem (QUALITY_WAIT_RECIPE_CHANGE_OK, FALSE);
	WriteComm (_T("MOT.RECIPE.CHANGE"));

	DWORD dwTimeout = TIMEOUT_10;
	bRet = RunModalLoop (QUALITY_WAIT_RECIPE_CHANGE_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("RECIPE.CHANGE.OK"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_QualityComm::ActInspectStart (CString strInnerID, int nSheetRow, CString strCellID)
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (QUALITY_WAIT_GRAB_START_OK, FALSE);
	SetWaitItem (QUALITY_WAIT_GRAB_END_OK,	FALSE);
	WriteComm (dfcStr (_T("MOT.GRAB.START.%s.%s"), strInnerID, strCellID));

	QualityWaitSet (strInnerID, nSheetRow, strCellID);

	DWORD dwTimeout = TIMEOUT_20;
	bRet = RunModalLoop (QUALITY_WAIT_GRAB_START_OK, dwTimeout);
	if (!bRet) {
		SendAlarm (_T("GRAB.START.OK"), dwTimeout);
		return FALSE;
	}

	bRet = RunModalLoop (QUALITY_WAIT_GRAB_END_OK, dwTimeout);
	if (!bRet) {
		SendAlarm (_T("GRAB.END.OK"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

void	C_QualityComm::ReceiveEventOK (int nEvent, TCHAR* pBuf)
{
	SetWaitItem (nEvent, TRUE);
	RunModalFinish (FALSE);
}

void	C_QualityComm::ReceiveEventNG ()
{
	m_nRunning = COMM_RS_ERROR;
}

void	C_QualityComm::ReceiveClassEnd (CString strRcv)
{
	CString strTemp = strRcv;
	int nSizeMax = strTemp.GetLength ();

	CString strFindCode = _T("CLASS.END.OK.");
	int nLength = strFindCode.GetLength ();

	CString strInnerID;
	CStringArray strCodeArray;
	int nSerchCount = 0;

	//InnerID 여러개 들어올때 다 처리되게
	while ((nSerchCount = strTemp.Find (strFindCode)) >= 0) {
		strCodeArray.Add (strTemp.Mid (nLength + nSerchCount, 7)); //InnerID 코드가 항상 7자리일때만 가능 (ID + 시분초)
		strTemp = strTemp.Mid (nLength + nSerchCount, nSizeMax);
	}

	if (strCodeArray.GetCount () <= 0) return;

	for (int i = 0; i < strCodeArray.GetCount (); i++) {
		strInnerID = strCodeArray.GetAt (i).GetString ();
		QualityWaitRun (strInnerID);
	}
}

void	C_QualityComm::ReceiveFilter (int nItem)
{
	m_nMoveFilter = nItem;

	if (nItem == QUALITY_FILTER_IN) {
		if (m_pHandler) m_pHandler (NULL, EVENT_COMM_FILTER_IN,  m_sInfo.m_nHead);
	}
	else {
		if (m_pHandler) m_pHandler (NULL, EVENT_COMM_FILTER_OUT, m_sInfo.m_nHead);
	}
}

////////////////////////////////////////////////////RECEIVE
void	C_QualityComm::ReceiveCode (TCHAR* pBuf)
{
	CString strRcv = pBuf;

	if (strRcv.Find (_T(".NG")) >= 0) {
		ReceiveEventNG ();
	}
	else {
		if		(strRcv.Find (_T("FDB.RECIPE.CHANGE.OK")) >= 0) ReceiveEventOK (QUALITY_WAIT_RECIPE_CHANGE_OK, pBuf);
		else if (strRcv.Find (_T("FDB.GRAB.START.OK")) >= 0)	ReceiveEventOK (QUALITY_WAIT_GRAB_START_OK, pBuf);
		else if (strRcv.Find (_T("FDB.GRAB.END.OK")) >= 0)		ReceiveEventOK (QUALITY_WAIT_GRAB_END_OK, pBuf);
		else if (strRcv.Find (_T("FDB.CLASS.END")) >= 0)		ReceiveClassEnd (strRcv);
		else if (strRcv.Find (_T("FDB.POL.MOVE.IN")) >= 0)		ReceiveFilter (QUALITY_FILTER_IN);
		else if (strRcv.Find (_T("FDB.POL.MOVE.OUT")) >= 0)		ReceiveFilter (QUALITY_FILTER_OUT);
	}

	CString strMsg = dfcStr (_T("ME ◀ [%s%d] : %s"), m_sInfo.m_strName, m_sInfo.m_nHead, pBuf);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_MSG, NULL);

}

void	C_QualityComm::WriteComm (LPCTSTR lpszMsg)
{
	CString strCmd = lpszMsg;

	Send (lpszMsg);

	CString strMsg = dfcStr (_T("ME ▶ [%s%d] : %s"), m_sInfo.m_strName, m_sInfo.m_nHead, strCmd);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_MSG, NULL);
}

BOOL	C_QualityComm::Delay (DWORD dwTime)
{
	DWORD dwTimeout = dwTime;
	DWORD dwBeginTick = GetTickCount ();
	DWORD dwCurrentTick = 0;
	DWORD dwAddTime = 0;

	while (1) {
		dwCurrentTick = GetTickCount ();

		if (dwCurrentTick < dwBeginTick) {
			dwBeginTick = dwCurrentTick;
			dwTimeout -= dwAddTime;
		}
		else dwAddTime = (dwCurrentTick - dwBeginTick);

		if ((dwCurrentTick - dwBeginTick) > dwTimeout) break;
	}

	return TRUE;
}

BOOL	C_QualityComm::RunModalLoop (int nWaitItem, DWORD dwTime)
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


BOOL	C_QualityComm::RunModalFinish (BOOL bFinishSet)
{
	if (bFinishSet) m_nRunning = COMM_RS_FINISH;

	CString strMsg = dfcStr (_T("★ [%s%d] : RunModalFinish"), m_sInfo.m_strName, m_sInfo.m_nHead);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_MSG, NULL);

	return TRUE;
}

//두개의 버퍼중 하나라도 비어있으면 TRUE
BOOL	C_QualityComm::QualityBufferEmpty ()
{
	for (int i = 0; i < QUALITY_BUFFER_MAX; i++) {
		if (sQualityWait[i].m_bWaitStatus == FALSE) return TRUE;
	}

	return FALSE;
}

//두개의 버퍼가 모두 비어있으면 TRUE
BOOL	C_QualityComm::QualityBufferEmptyAll ()
{
	for (int i = 0; i < QUALITY_BUFFER_MAX; i++) {
		if (sQualityWait[i].m_bWaitStatus == TRUE) return FALSE;
	}

	return TRUE;
}

//화질검사 결과 처리 준비 및 내용(InnerID, Data출력 위치(SheetRow)) 저장
void	C_QualityComm::QualityWaitSet (CString strInnerID, int nSheetRow, CString strCellID)
{
	for (int i = 0; i < QUALITY_BUFFER_MAX; i++) {
		if (sQualityWait[i].m_bWaitStatus == FALSE) {
			sQualityWait[i].m_strInnerID  = strInnerID;
			sQualityWait[i].m_strCellID   = strCellID;
			sQualityWait[i].m_nSheetRow   = nSheetRow;
			sQualityWait[i].m_bWaitStatus = TRUE;
			return;
		}
	}
}

//화질검사 결과 처리 실행 (결과 데이터에서 받은 InnerID 를 토대로 설정 되었던 InnerID 비교 하여 처리)
void	C_QualityComm::QualityWaitRun (CString strInnerID)
{
	for (int i = 0; i < QUALITY_BUFFER_MAX; i++) {
		if (sQualityWait[i].m_bWaitStatus == TRUE && sQualityWait[i].m_strInnerID == strInnerID) {
			sQualityWait[i].m_bRunStatus = TRUE;
			return;
		}
	}
}

//화질검사 구조체 초기화
void	C_QualityComm::QualityWaitClear ()
{
	for (int i = 0; i < QUALITY_BUFFER_MAX; i++) sQualityWait[i].Clear ();
}