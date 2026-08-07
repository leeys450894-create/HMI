#include "pch.h"
#include "dfcComm.h"
#include "QualityComm.h"

C_CIMComm::C_CIMComm (QUALITY_EVENT_HANDLER pHandler, CString IniPath, int nIndex)
{
	m_nRunning	= COMM_RS_FINISH;
	m_pHandler	= pHandler;
	m_nIndex	= nIndex;			//통신 개수 따라 증가 Ini파일 접근시만 사용
	m_path		= IniPath;

	m_nUDReq_or_MultiRcp  = -1;
	m_nExchange_or_Unload = -1;
	m_bLoadGlassExist     = FALSE;

	memset (m_bWaitItem, FALSE, sizeof (BOOL) * CIM_WAIT_MAX);

	ReadIniInfo ();

	if (GetCommUse ()) ConnectCtrl (TRUE);
}

C_CIMComm::~C_CIMComm ()
{

}

void	C_CIMComm::OnClose (int nErrorCode)
{
	C_ClientSocket::OnClose (nErrorCode);

	CString strMsg;
	strMsg = dfcStr (_T("★ [%s%d] : Close"), m_sInfo.m_strName, m_sInfo.m_nHead);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_DISCONNECT, m_sInfo.m_nHead);
}

void	C_CIMComm::OnConnect (int nErrorCode)
{
	C_ClientSocket::OnConnect (nErrorCode);
}

void	C_CIMComm::ConnectCtrl (BOOL bConnect)
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

void	C_CIMComm::ReadIniInfo ()
{
	m_sInfo.m_bUse			= GetProfileInt		(_T("TCPIP"), dfcStr (_T("TCPIP_%d_USE"),				m_nIndex), FALSE);
	m_sInfo.m_bOutOfService = GetProfileInt		(_T("TCPIP"), dfcStr (_T("TCPIP_%d_OUT_OF_SERVICE"),	m_nIndex), TRUE);
	m_sInfo.m_strName		= GetProfileString	(_T("TCPIP"), dfcStr (_T("TCPIP_%d_NAME"),				m_nIndex), _T("CIM"));
	m_sInfo.m_nHead			= GetProfileInt		(_T("TCPIP"), dfcStr (_T("TCPIP_%d_HEAD_NUM"),			m_nIndex), 1);
	m_sInfo.m_strIP			= GetProfileString	(_T("TCPIP"), dfcStr (_T("TCPIP_%d_IP"),				m_nIndex), _T("192.168.0.1"));
	m_sInfo.m_nPortNum		= GetProfileInt		(_T("TCPIP"), dfcStr (_T("TCPIP_%d_PORT"),				m_nIndex), 4001);
	m_sInfo.m_strResultDir  = GetProfileString	(_T("TCPIP"), dfcStr (_T("TCPIP_%d_RESULT_DIRECTORY"),	m_nIndex), _T("Z:\\"));
	m_sInfo.m_strRecipeFile = GetProfileString	(_T("TCPIP"), dfcStr (_T("TCPIP_%d_RECIPE_FILE"),		m_nIndex), _T("Z:\\Recipe.ini"));
}

void	C_CIMComm::WriteIniInfo (S_TcpIpInfo *sInfo)
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

void	C_CIMComm::SetWaitItem (int nItem, BOOL bSet)
{
	if (nItem == -1) {
		for (int i = 0; i < CIM_WAIT_MAX; i++) m_bWaitItem[i] = bSet;

	}
	else m_bWaitItem[nItem] = bSet;
}

BOOL	C_CIMComm::GetWaitItem (int nItem)
{
	return m_bWaitItem[nItem];
}

void	C_CIMComm::TimeOutAlarm (CString strCmd, int nTime)
{
	CString strMsg;
	strMsg = dfcStr (_T("★ [%s%d] : %s TimeOut...%d"), m_sInfo.m_strName, m_sInfo.m_nHead, strCmd, nTime);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_TIMEOUT, m_sInfo.m_nHead);
}

void	C_CIMComm::NakAlarm (CString strCmd)
{
	CString strMsg;
	strMsg = dfcStr (_T("★ [%s%d] : %s NAK"), m_sInfo.m_strName, m_sInfo.m_nHead, strCmd);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_NAK, m_sInfo.m_nHead);
}

void	C_CIMComm::SendAlarm (CString strCmd, int nTime)
{
	if		(m_nRunning == COMM_RS_TIMEOUT) TimeOutAlarm (strCmd, nTime);
	else if (m_nRunning == COMM_RS_ERROR)	NakAlarm (strCmd);
}

//////////////////////////////////////////////////////
BOOL	C_CIMComm::WaitSTOP ()
{
	m_nRunning = COMM_RS_STOP;

	SetWaitItem (CIM_WAIT_CLEAR, FALSE);

	CString strMsg = dfcStr (_T("★ [%s%d] : LoopStop"), m_sInfo.m_strName, m_sInfo.m_nHead);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_MSG, NULL);

	return TRUE;
}

BOOL	C_CIMComm::ActAT1 ()
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (CIM_WAIT_AT_1_OK,	   FALSE);
	SetWaitItem (CIM_WAIT_EQSTART_RCV, FALSE);

	WriteComm (_T("AT1$"));

	DWORD dwTimeout = TIMEOUT_5;
	bRet = RunModalLoop (CIM_WAIT_AT_1_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("AT1_OK$"), dwTimeout);
		return FALSE;
	}

	bRet = RunModalLoop (CIM_WAIT_EQSTART_RCV, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("EQPSTART$"), dwTimeout);
		return FALSE;
	}

	WriteComm (_T("EQPSTART_OK$"));

	return TRUE;
}

BOOL	C_CIMComm::SetRecipeList ()
{
	WriteComm (_T("RCPLIST$"));
	return TRUE;
}

BOOL	C_CIMComm::ActAlarmClear ()
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (CIM_WAIT_ALMACL_OK, FALSE);

	WriteComm (_T("ALMACL$"));

	DWORD dwTimeout = TIMEOUT_5;
	bRet = RunModalLoop (CIM_WAIT_ALMACL_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("ALMACL_OK$"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_CIMComm::ActCIMTime ()
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (CIM_WAIT_CIM_TIMESET, FALSE);

	WriteComm (_T("TIMEREQ$"));

	DWORD dwTimeout = TIMEOUT_5;
	bRet = RunModalLoop (CIM_WAIT_CIM_TIMESET, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("TIMESET"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_CIMComm::ActSTS0104 ()
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (CIM_WAIT_STS0104_OK, FALSE);

	WriteComm (_T("STS0104$"));

	DWORD dwTimeout = TIMEOUT_5;
	bRet = RunModalLoop (CIM_WAIT_STS0104_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("STS0104_OK$"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_CIMComm::ActSTS0204 ()
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (CIM_WAIT_STS0204_OK, FALSE);
	SetWaitItem (CIM_WAIT_ORG_RCV,	  FALSE);

	WriteComm (_T("STS0204$"));

	DWORD dwTimeout = TIMEOUT_5;
	bRet = RunModalLoop (CIM_WAIT_STS0204_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("STS0204_OK$"), dwTimeout);
		return FALSE;
	}

	bRet = RunModalLoop (CIM_WAIT_ORG_RCV, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("ORG$"), dwTimeout);
		return FALSE;
	}

	WriteComm(_T("ORG_OK$"));

	return TRUE;
}

BOOL	C_CIMComm::ActSTS0304 ()
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (CIM_WAIT_STS0304_OK, FALSE);

	WriteComm (_T("STS0304$"));

	DWORD dwTimeout = TIMEOUT_5;
	bRet = RunModalLoop (CIM_WAIT_STS0304_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("STS0304_OK$"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_CIMComm::ActSTS0004 ()
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (CIM_WAIT_STS0004_OK, FALSE);
	Reset_LoadGlassInfo ();//Glass Loading Check 변수 초기화

	WriteComm (_T("STS0004$"));

	DWORD dwTimeout = TIMEOUT_5;
	bRet = RunModalLoop (CIM_WAIT_STS0004_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("STS0004_OK$"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_CIMComm::ActSTS0506 ()
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (CIM_WAIT_STS0506_OK, FALSE);

	WriteComm (_T("STS0506$"));

	DWORD dwTimeout = TIMEOUT_5;
	bRet = RunModalLoop (CIM_WAIT_STS0506_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("STS0506_OK$"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_CIMComm::ActPPID_Req ()
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (CIM_WAIT_PPID,	FALSE);

	WriteComm (_T("PPIDREQ$"));

	DWORD dwTimeout = TIMEOUT_10;
	bRet = RunModalLoop (CIM_WAIT_PPID, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("PPID"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_CIMComm::SetPPID_OK ()
{
	WriteComm (_T("PPID_OK$"));
	return TRUE;
}

BOOL	C_CIMComm::SetPPID_NG ()
{
	WriteComm (_T("PPID_NG$"));
	return TRUE;
}

BOOL	C_CIMComm::ActSTS2004 ()
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (CIM_WAIT_STS2004_OK, FALSE);

	WriteComm (_T("STS2004$"));

	DWORD dwTimeout = TIMEOUT_5;
	bRet = RunModalLoop (CIM_WAIT_STS2004_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("STS2004_OK$"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_CIMComm::ActLotID_Req ()
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (CIM_WAIT_LOTID, FALSE);

	WriteComm (_T("LOTREQ$"));

	DWORD dwTimeout = TIMEOUT_5;
	bRet = RunModalLoop (CIM_WAIT_LOTID, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("LOTID"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_CIMComm::ActGlsID_Req ()
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (CIM_WAIT_GLSID, FALSE);

	WriteComm (_T("GLSREQ$"));

	DWORD dwTimeout = TIMEOUT_5;
	bRet = RunModalLoop (CIM_WAIT_GLSID, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("GLSID"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_CIMComm::ActSTS0006 ()
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (CIM_WAIT_STS0006_OK, FALSE);
	SetWaitItem (CIM_WAIT_PRCS_RCV,	  FALSE);

	WriteComm (_T("STS0006$"));

	DWORD dwTimeout = TIMEOUT_5;
	bRet = RunModalLoop (CIM_WAIT_STS0006_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("STS0006_OK$"), dwTimeout);
		return FALSE;
	}

	bRet = RunModalLoop (CIM_WAIT_PRCS_RCV, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("PRCS$"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_CIMComm::SetPCRS_OK ()
{
	WriteComm (_T("PRCS_OK$"));
	return TRUE;
}

BOOL	C_CIMComm::SetPCRS_NG ()
{
	WriteComm (_T("PRCS_NG$"));
	return TRUE;
}

BOOL	C_CIMComm::ActSTS1005 ()
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (CIM_WAIT_STS1005_OK, FALSE);

	WriteComm (_T("STS1005$"));

	DWORD dwTimeout = TIMEOUT_5;
	bRet = RunModalLoop (CIM_WAIT_STS1005_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("STS1005_OK$"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_CIMComm::ActSTS1006 ()
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (CIM_WAIT_STS1006_OK, FALSE);

	WriteComm (_T("STS1006$"));

	DWORD dwTimeout = TIMEOUT_5;
	bRet = RunModalLoop (CIM_WAIT_STS1006_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("STS1006_OK$"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_CIMComm::ActSTS1106 ()
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (CIM_WAIT_STS1106_OK, FALSE);

	WriteComm (_T("STS1106$"));

	DWORD dwTimeout = TIMEOUT_5;
	bRet = RunModalLoop (CIM_WAIT_STS1106_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("STS1106_OK$"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_CIMComm::ActDataJudge_OK ()
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (CIM_WAIT_GDTREQ_RCV, FALSE);

	WriteComm (_T("DATAJUDGE_OK$"));

	DWORD dwTimeout = TIMEOUT_5;
	bRet = RunModalLoop (CIM_WAIT_GDTREQ_RCV, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("GDTREQ$"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_CIMComm::ActDataJudge_NG ()
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (CIM_WAIT_GDTREQ_RCV, FALSE);

	WriteComm (_T("DATAJUDGE_SPECOUT$"));

	DWORD dwTimeout = TIMEOUT_5;
	bRet = RunModalLoop (CIM_WAIT_GDTREQ_RCV, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("GDTREQ$"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_CIMComm::SetGDTREQ_OK ()
{
	WriteComm (_T("GDTREQ_OK$"));
	return TRUE;
}

BOOL	C_CIMComm::SetGDTREQ_NG ()
{
	WriteComm (_T("GDTREQ_NG$"));
	return TRUE;
}

BOOL	C_CIMComm::SetGDTCPL_OK ()
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (CIM_WAIT_ULDREQ_OR_MULTIRCP_RCV, FALSE);
	Reset_UD_or_MultiRcp ();//ULDREQ or MULTIRECIPE 확인용

	WriteComm (_T("GDTCPL_OK$"));

	DWORD dwTimeout = TIMEOUT_5;
	bRet = RunModalLoop (CIM_WAIT_ULDREQ_OR_MULTIRCP_RCV, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("ULDREQ$ || MULTIRECIPE$"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_CIMComm::SetGDTCPL_NG ()
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (CIM_WAIT_ULDREQ_OR_MULTIRCP_RCV, FALSE);
	Reset_UD_or_MultiRcp ();//ULDREQ or MULTIRECIPE 확인용

	WriteComm (_T("GDTCPL_NG$"));

	DWORD dwTimeout = TIMEOUT_5;
	bRet = RunModalLoop (CIM_WAIT_ULDREQ_OR_MULTIRCP_RCV, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("ULDREQ$ || MULTIRECIPE$"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_CIMComm::SetULDREQ_OK ()
{
	WriteComm (_T("ULDREQ_OK$"));
	return TRUE;
}

BOOL	C_CIMComm::SetULDREQ_NG ()
{
	WriteComm (_T("ULDREQ_NG$"));
	return TRUE;
}

BOOL	C_CIMComm::ActSTS2106 ()
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (CIM_WAIT_STS2106_OK, FALSE);

	WriteComm (_T("STS2106$"));

	DWORD dwTimeout = TIMEOUT_5;
	bRet = RunModalLoop (CIM_WAIT_STS2106_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("STS2106_OK$"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_CIMComm::ActSTS2107 ()
{
	if (!IsConnected ()) return FALSE;

	BOOL bRet = FALSE;

	SetWaitItem (CIM_WAIT_STS2107_OK,			  FALSE);
	SetWaitItem (CIM_WAIT_HS_OFF,				  FALSE);
	SetWaitItem (CIM_WAIT_EXCHANGE_OR_UNLOAD_RCV, FALSE);
	Reset_Exchange_or_Unload (); //GLSREADY or ULDCPL 확인용

	WriteComm (_T("STS2107$"));

	DWORD dwTimeout = TIMEOUT_5;
	bRet = RunModalLoop (CIM_WAIT_STS2107_OK, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("STS2107_OK$"), dwTimeout);
		return FALSE;
	}

	dwTimeout = TIMEOUT_30;
	bRet = RunModalLoop (CIM_WAIT_HS_OFF, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("HS_OFF$"), dwTimeout);
		return FALSE;
	}

	dwTimeout = TIMEOUT_10;
	bRet = RunModalLoop (CIM_WAIT_EXCHANGE_OR_UNLOAD_RCV, dwTimeout);	//대기 함수
	if (!bRet) {
		SendAlarm (_T("GLSREADY$ || ULDCPL$"), dwTimeout);
		return FALSE;
	}

	return TRUE;
}

BOOL	C_CIMComm::SetMULTIRECIPE_OK ()
{
	WriteComm (_T("MULTIRECIPE_OK$"));
	return TRUE;
}

BOOL	C_CIMComm::SetMULTIRECIPE_NG ()
{
	WriteComm (_T("MULTIRECIPE_NG$"));
	return TRUE;
}

BOOL	C_CIMComm::SetGLSREADY_OK ()
{
	WriteComm (_T("GLSREADY_OK$"));
	return TRUE;
}

BOOL	C_CIMComm::SetGLSREADY_NG ()
{
	WriteComm (_T("GLSREADY_NG$"));
	return TRUE;
}

BOOL	C_CIMComm::SetULDCPL_OK ()
{
	WriteComm (_T("ULDCPL_OK$"));
	return TRUE;
}

BOOL	C_CIMComm::SetULDCPL_NG ()
{
	WriteComm (_T("ULDCPL_NG$"));
	return TRUE;
}

BOOL	C_CIMComm::SetLDREQ_OK ()
{
	WriteComm (_T("LDREQ_OK$"));
	return TRUE;
}

BOOL	C_CIMComm::SetLDREQ_NG ()
{
	WriteComm (_T("LDREQ_NG$"));
	return TRUE;
}

void	C_CIMComm::ReceiveEventOK (int nEvent, TCHAR* pBuf)
{
	SetWaitItem (nEvent, TRUE);
	RunModalFinish (FALSE);
}

void	C_CIMComm::ReceiveEventNG ()
{
	m_nRunning = COMM_RS_ERROR;
}

////////////////////////////////////////////////////RECEIVE
void	C_CIMComm::ReceiveCode (TCHAR* pBuf)
{
	CString strRcv = pBuf;

	if		(strRcv.Find (_T("AT1_OK$"))     >= 0)	ReceiveEventOK (CIM_WAIT_AT_1_OK,		pBuf);
	else if	(strRcv.Find (_T("EQPSTART$"))   >= 0)	ReceiveEventOK (CIM_WAIT_EQSTART_RCV,	pBuf);
	else if	(strRcv.Find (_T("ALMACL_OK$"))  >= 0)	ReceiveEventOK (CIM_WAIT_ALMACL_OK,		pBuf);
	else if	(strRcv.Find (_T("TIMESET^"))    >= 0)	GetCIMInfoData (strRcv, 1, CIM_WAIT_CIM_TIMESET);	//데이터 획득
	else if	(strRcv.Find (_T("STS0104_OK$")) >= 0)	ReceiveEventOK (CIM_WAIT_STS0104_OK,	pBuf);
	else if	(strRcv.Find (_T("STS0204_OK$")) >= 0)	ReceiveEventOK (CIM_WAIT_STS0204_OK,	pBuf);
	else if	(strRcv.Find (_T("ORG$"))		 >= 0)	ReceiveEventOK (CIM_WAIT_ORG_RCV,		pBuf);
	else if	(strRcv.Find (_T("STS0304_OK$")) >= 0)	ReceiveEventOK (CIM_WAIT_STS0304_OK,	pBuf);
	else if	(strRcv.Find (_T("STS0004_OK$")) >= 0)	ReceiveEventOK (CIM_WAIT_STS0004_OK,	pBuf);
	else if	(strRcv.Find (_T("STS0506_OK$")) >= 0)	ReceiveEventOK (CIM_WAIT_STS0506_OK,	pBuf);
	else if	(strRcv.Find (_T("PPID^"))		 >= 0)	GetCIMInfoData (strRcv, 2, CIM_WAIT_PPID);			//데이터 획득
	else if	(strRcv.Find (_T("STS2004_OK$")) >= 0)	ReceiveEventOK (CIM_WAIT_STS2004_OK,	pBuf);
	else if	(strRcv.Find (_T("PPID^"))		 >= 0)	GetCIMInfoData (strRcv, 3, CIM_WAIT_LOTID);			//데이터 획득
	else if	(strRcv.Find (_T("GLSID^"))		 >= 0)	GetCIMInfoData (strRcv, 4, CIM_WAIT_GLSID);			//데이터 획득
	else if	(strRcv.Find (_T("STS0006$"))	 >= 0)	ReceiveEventOK (CIM_WAIT_STS0006_OK,	pBuf);
	else if	(strRcv.Find (_T("PRCS$"))		 >= 0)	ReceiveEventOK (CIM_WAIT_PRCS_RCV,		pBuf);
	else if	(strRcv.Find (_T("STS1005$"))	 >= 0)	ReceiveEventOK (CIM_WAIT_STS1005_OK,	pBuf);
	else if	(strRcv.Find (_T("STS1006$"))	 >= 0)	ReceiveEventOK (CIM_WAIT_STS1006_OK,	pBuf);
	else if	(strRcv.Find (_T("STS1106$"))	 >= 0)	ReceiveEventOK (CIM_WAIT_STS1106_OK,	pBuf);
	else if	(strRcv.Find (_T("GDTREQ$"))	 >= 0)	ReceiveEventOK (CIM_WAIT_GDTREQ_RCV,	pBuf);
	else if	(strRcv.Find (_T("ULDREQ$"))	 >= 0 || strRcv.Find (_T("MULTIRECIPE$"))>= 0)	{
		if		(strRcv.Find(_T("ULDREQ$"))		 >= 0) Set_UD_or_MultiRcp (CIM_G1_UNLOAD_REQ_RCV);
		else if (strRcv.Find(_T("MULTIRECIPE$")) >= 0) Set_UD_or_MultiRcp (CIM_G1_MULTI_RCP_RCV);

		ReceiveEventOK (CIM_WAIT_ULDREQ_OR_MULTIRCP_RCV, pBuf);
	}
	else if	(strRcv.Find (_T("STS2106$"))	  >= 0)	ReceiveEventOK (CIM_WAIT_STS2106_OK,	pBuf);
	else if	(strRcv.Find (_T("STS2107$"))	  >= 0)	ReceiveEventOK (CIM_WAIT_STS2107_OK,	pBuf);
	else if	(strRcv.Find (_T("HSOFF$"))		  >= 0)	ReceiveEventOK (CIM_WAIT_HS_OFF,		pBuf);
	else if	(strRcv.Find (_T("GLSREADY$"))	  >= 0 || strRcv.Find (_T("ULDCPL$")) >= 0)	{
		if		(strRcv.Find(_T("GLSREADY$")) >= 0) Set_Exchange_or_Unload (CIM_G2_EXCHANGE_RCV);
		else if (strRcv.Find(_T("ULDCPL$"))	  >= 0) Set_Exchange_or_Unload (CIM_G2_UD_CPL_RCV);

		ReceiveEventOK (CIM_WAIT_EXCHANGE_OR_UNLOAD_RCV, pBuf);
	}
	else if	(strRcv.Find (_T("LDREQ$"))		  >= 0) Set_LoadGlassInfo (TRUE);

	CString strMsg = dfcStr (_T("◀ [%s%d] : %s"), m_sInfo.m_strName, m_sInfo.m_nHead, pBuf);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_MSG, NULL);

}

void	C_CIMComm::WriteComm (LPCTSTR lpszMsg)
{
	CString strCmd = lpszMsg;

	Send (lpszMsg);

	CString strMsg = dfcStr (_T("▶ [%s%d] : %s"), m_sInfo.m_strName, m_sInfo.m_nHead, strCmd);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_MSG, NULL);
}

BOOL	C_CIMComm::Delay (DWORD dwTime)
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

BOOL	C_CIMComm::RunModalLoop (int nWaitItem, DWORD dwTime)
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

BOOL	C_CIMComm::RunModalFinish (BOOL bFinishSet)
{
	if (bFinishSet) m_nRunning = COMM_RS_FINISH;

	CString strMsg = dfcStr (_T("★ [%s%d] : RunModalFinish"), m_sInfo.m_strName, m_sInfo.m_nHead);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_MSG, NULL);

	return TRUE;
}

void	C_CIMComm::GetParsingCIMData (CString strRcv, CStringArray &strArr)
{
	int nBeging = 0, nEnd = 0;

	for (int i =0; i < strRcv.GetLength (); i++) {
		if (strRcv.GetAt (i) == '^' || strRcv.GetAt (i) == '$') {

			if (nBeging == 0) {
				nBeging = i + 1;
			}
			else {
				nEnd = i - nBeging;
				strArr.Add (strRcv.Mid (nBeging, nEnd).Trim ());
				nBeging = i + 1;
			}
		}
	}
}

void	C_CIMComm::GetCIMInfoData (CString strRcv, int nDataCnt, int nWaitItem)
{
	CStringArray strArrData;
	strArrData.RemoveAll ();

	BOOL bRcvOK = FALSE;

	GetParsingCIMData(strRcv, strArrData);

	if (nWaitItem == CIM_WAIT_CIM_TIMESET) {
		if (strArrData.GetSize() == 1) {
			m_strCIMTime = strArrData.GetAt(0);
			bRcvOK = TRUE;
		}
	}
	else if (nWaitItem == CIM_WAIT_PPID) {
		if (strArrData.GetSize () == 2) {
			m_strPPID = strArrData.GetAt (0);
			m_strFlag = strArrData.GetAt (1);
			bRcvOK = TRUE;
		}
	}
	else if (nWaitItem == CIM_WAIT_LOTID) {
		if (strArrData.GetSize () == 3) {
			m_strLotID  = strArrData.GetAt (0);
			m_strCstID  = strArrData.GetAt (1);
			m_strOperID = strArrData.GetAt (2);
			bRcvOK = TRUE;
		}
	}
	else if (nWaitItem == CIM_WAIT_GLSID) {
		if (strArrData.GetSize () == 4) {
			m_strGlsID  = strArrData.GetAt (0);
			m_strSlotID = strArrData.GetAt (1);
			m_strStepID = strArrData.GetAt (2);
			m_strRotate = strArrData.GetAt (3);
			bRcvOK = TRUE;
		}
	}

	if (bRcvOK) ReceiveEventOK (nWaitItem, strRcv.GetBuffer ());
	else		ReceiveEventNG ();
}