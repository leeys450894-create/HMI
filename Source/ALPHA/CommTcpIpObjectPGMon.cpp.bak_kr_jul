#include "pch.h"
#include "CommTcpIpObjectPGMon.h"

#include "Manager\NetworkConfigMgr.h"
#include "Manager\SystemConfigMgr.h"
#include "Alarm\AlarmController.h"
#include "Manager\MCCLogMgr.h"		// LYS 2026-07-29 ELP data push for FDC upload (U1)
#include "AuroraFuncDefine.h"

#include "CommEthFunction\EthernetManagerTCP.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// LYS 2026-07-29 ELP ElecBox monitor (doc 2026-07-28).
// Passive receive-only channel: ELP(Server) pushes S10F01/S10F20, HMI(Client) replies ACK.
// Known GAI defects fixed here: offset accumulation in multi-alarm parse,
// unchecked alarm code range, delete instead of delete[], Len field validation.

CCommTcpIpObjectPGMon::CCommTcpIpObjectPGMon(int iSeqItemIndex, stAuroraMgrPool& MgrPool)
	: m_pSystemConfigMgr(MgrPool.pSystemConfigMgr)
	, m_pLogFileAccessor(new CLogFileAccessor)
	, m_bOutOfService(false)
	, m_dwLastRecvTick(0)
{
	ZeroMemory(&m_dElpData, sizeof(m_dElpData));
	ZeroMemory(&m_bDataValid, sizeof(m_bDataValid));

	CString strObjectName;
	strObjectName.Format(_T("%s"), _T("PGMON"));

	SetName(strObjectName);
	const bool& bRunMode = MgrPool.pNetworkConfigMgr->GetParam_Mode(iSeqItemIndex);
	const CString& strIP = MgrPool.pNetworkConfigMgr->GetParam_IPNumber(iSeqItemIndex);
	const long& nPort = MgrPool.pNetworkConfigMgr->GetParam_PortNumger(iSeqItemIndex);
	SetNetInfo(bRunMode, strIP, nPort);

	m_bOutOfService = MgrPool.pNetworkConfigMgr->GetParam_OutOfService(iSeqItemIndex);

	// log file
	const CString strLogFilePath = MgrPool.strLogPath;
	const CString strFolderPath = strLogFilePath + _T("Control\\") + strObjectName + _T("\\");
	const CString strLogFile = strFolderPath + strObjectName + _T(".log");
	MakeDir(strFolderPath);
	m_pLogFileAccessor->Initialize(strLogFile);
}

CCommTcpIpObjectPGMon::~CCommTcpIpObjectPGMon()
{
	Finalize();

	if (m_pLogFileAccessor)
	{
		delete m_pLogFileAccessor;
		m_pLogFileAccessor = nullptr;
	}
}

CLogFileAccessor* CCommTcpIpObjectPGMon::GetLogFileAccessorPtr()
{
	return m_pLogFileAccessor;
}

double CCommTcpIpObjectPGMon::GetElpData(int iBox, int iItem)
{
	CCriticalSectionSync single(&m_cs_Data, TRUE);

	if (iBox < 0 || iBox >= DF_ELPMON_BOX_MAX)
		return 0.;
	if (iItem < 0 || iItem >= DF_ELPMON_DATA_CNT)
		return 0.;

	return m_dElpData[iBox][iItem];
}

bool CCommTcpIpObjectPGMon::IsElpDataValid(int iBox)
{
	CCriticalSectionSync single(&m_cs_Data, TRUE);

	if (iBox < 0 || iBox >= DF_ELPMON_BOX_MAX)
		return false;

	return m_bDataValid[iBox];
}

// Alarm raiser thread: Heavy alarm SetAlarm() blocks until operator reset,
// so each received error code is raised on its own short-lived thread and
// never on the socket receive thread. Alarm DLL ignores already-listed IDs.
UINT CCommTcpIpObjectPGMon::ThreadRaiseAlarm(LPVOID pParam)
{
	int iAlarmID = (int)(INT_PTR)pParam;

	CAlarmController::GetInstance()->SetAlarm(iAlarmID);

	return 0;
}

// Doc 3. Error Code: box1 0~39, box2 100~139, socket 200~207, detail 300~303 / 400~403
bool CCommTcpIpObjectPGMon::IsValidAlarmCode(int iCode)
{
	if (iCode >= 0 && iCode <= 39)		return true;
	if (iCode >= 100 && iCode <= 139)	return true;
	if (iCode >= 200 && iCode <= 207)	return true;
	if (iCode >= 300 && iCode <= 303)	return true;
	if (iCode >= 400 && iCode <= 403)	return true;

	return false;
}

// S20F01 / S20F20 (doc 2.2.2 / 2.2.4)
// STX + "20" + Func(2) + Len(4, doc value 0) + Ack(2, 00:ACK 01:NAK) + Cause(4, 0000:Normal) + ETX
// LYS 2026-07-30 Confirmed with ELP: Len is fixed "0000" and Cause is fixed "0000".
// ELP validates neither field and never retransmits on NAK, so this ACK is purely
// informational - do NOT restore the old GAI value 0006.
void CCommTcpIpObjectPGMon::SendAck(int iFunc, int iAck, int iCause)
{
	CString strReturnCommand;
	strReturnCommand.Format(_T("%c20%02d%04d%02d%04d%c"), 0x02, iFunc, 0, iAck, iCause, 0x03);

	CString strLog;
	strLog.Format(_T("[%s] : %s"), GetName(), strReturnCommand);
	m_pLogFileAccessor->KMLOG(SEND, strLog);

	if (IsOutOfService() == true)
		return;

	SendPacket(strReturnCommand);
}

// S10F01 : ElecBox temp / smoke / power (doc 2.2.1, Len=80, one box per frame)
void CCommTcpIpObjectPGMon::ParseS10F01(const CString& strCmd)
{
	CString strLog;

	const int iLen = _ttoi(strCmd.Mid(4, 4));
	const int iBodyLen = strCmd.GetLength() - 8;

	// LYS 2026-07-30 ELP confirmed: ELP never retransmits on NAK. Reject only when
	// the body is genuinely too short to parse - a disagreeing Len field alone must
	// not discard an otherwise complete 80-char body (the data would be lost for good).
	if (iBodyLen < DF_ELPMON_BODY_LEN)
	{
		strLog.Format(_T("[%s] : S10F01 body too short. LEN(%d), BODY(%d)"), GetName(), iLen, iBodyLen);
		m_pLogFileAccessor->KMLOG(WARN, strLog);

		SendAck(1, 1, 0);	// NAK
		return;
	}

	if (iLen != DF_ELPMON_BODY_LEN)
	{
		strLog.Format(_T("[%s] : S10F01 LEN mismatch. LEN(%d), expected(%d) - parsing body anyway"), GetName(), iLen, DF_ELPMON_BODY_LEN);
		m_pLogFileAccessor->KMLOG(WARN, strLog);
	}

	ELPMONPROTOCOL_M stMon;
	ZeroMemory(&stMon, sizeof(stMon));
	memcpy(&stMon, (LPCTSTR)strCmd.Mid(8, DF_ELPMON_BODY_LEN), sizeof(stMon));

	TCHAR szField[11];
	CString strValue;

	// ElecBox Index: 01 ~ 02
	ZeroMemory(szField, sizeof(szField));
	memcpy(szField, stMon.szIndex, sizeof(TCHAR) * 2);
	const int iBox = _ttoi(szField) - 1;

	if (iBox < 0 || iBox >= DF_ELPMON_BOX_MAX)
	{
		strLog.Format(_T("[%s] : S10F01 invalid ElecBox index(%s)"), GetName(), szField);
		m_pLogFileAccessor->KMLOG(WARN, strLog);

		SendAck(1, 1, 0);	// NAK
		return;
	}

	double dData[DF_ELPMON_DATA_CNT];
	ZeroMemory(&dData, sizeof(dData));

	ZeroMemory(szField, sizeof(szField));
	memcpy(szField, stMon.szTemp, sizeof(TCHAR) * 4);
	dData[enumElpMonData_TEMP] = _ttof(szField) / 10.;			// 1234 -> 123.4 C

	ZeroMemory(szField, sizeof(szField));
	memcpy(szField, stMon.szSmoke, sizeof(TCHAR) * 4);
	dData[enumElpMonData_SMOKE] = _ttof(szField);				// %

	ZeroMemory(szField, sizeof(szField));
	memcpy(szField, stMon.szVolt, sizeof(TCHAR) * 10);
	dData[enumElpMonData_VOLT] = _ttof(szField) / 1000.;		// 12345 -> 12.345 V

	ZeroMemory(szField, sizeof(szField));
	memcpy(szField, stMon.szHarmVolt, sizeof(TCHAR) * 10);
	dData[enumElpMonData_HARM_VOLT] = _ttof(szField) / 1000.;	// V

	ZeroMemory(szField, sizeof(szField));
	memcpy(szField, stMon.szCurr, sizeof(TCHAR) * 10);
	dData[enumElpMonData_CURR] = _ttof(szField) / 1000.;		// A

	ZeroMemory(szField, sizeof(szField));
	memcpy(szField, stMon.szHarmCurr, sizeof(TCHAR) * 10);
	dData[enumElpMonData_HARM_CURR] = _ttof(szField) / 1000.;	// mA

	ZeroMemory(szField, sizeof(szField));
	memcpy(szField, stMon.szLeakCurr, sizeof(TCHAR) * 10);
	dData[enumElpMonData_LEAK_CURR] = _ttof(szField) / 1000.;	// mA

	ZeroMemory(szField, sizeof(szField));
	memcpy(szField, stMon.szInstPwr, sizeof(TCHAR) * 10);
	dData[enumElpMonData_INST_PWR] = _ttof(szField) / 1000.;	// kW

	ZeroMemory(szField, sizeof(szField));
	memcpy(szField, stMon.szAccumPwr, sizeof(TCHAR) * 10);
	dData[enumElpMonData_ACCUM_PWR] = (double)_ttoi(szField);	// 12345 -> 12345 kWh

	{
		CCriticalSectionSync single(&m_cs_Data, TRUE);
		memcpy(&m_dElpData[iBox], &dData, sizeof(dData));
		m_bDataValid[iBox] = true;
	}

	// LYS 2026-07-29 U1: push to MCCLogMgr -> VTM_FOR_FDC stMainWData (slots MAIN_WDATA_ELP_START+)
	// item order of enumElpMonData matches NUM_MAIN_WDATA_ELP item order.
	for (int i = 0; i < DF_ELPMON_DATA_CNT; i++)
	{
		CMCCLogMgr::GetInstance()->SetDataToELP(iBox, i, dData[i]);
	}

	CString strData;
	for (int i = 0; i < DF_ELPMON_DATA_CNT; i++)
	{
		strValue.Format(_T("%.3f,"), dData[i]);
		strData += strValue;
	}

	strLog.Format(_T("[%s] : S(10), F(01), LEN(%05d), BOX(%d), DATA(%s)"), GetName(), iLen, iBox + 1, strData);
	m_pLogFileAccessor->KMLOG(OPER, strLog);

	SendAck(1, 0, 0);	// S20F01 ACK
}

// S10F20 : alarm report, max 10ea x 5byte error code (doc 2.2.3)
void CCommTcpIpObjectPGMon::ParseS10F20(const CString& strCmd)
{
	CString strLog;

	const int iLen = _ttoi(strCmd.Mid(4, 4));
	const int iBodyLen = strCmd.GetLength() - 8;

	// LYS 2026-07-30 ELP confirmed: the alarm report is the critical path and ELP never
	// retransmits, so a suspect Len must not drop the codes the frame really carries.
	// Fall back to the actual body length, still capped at DF_ELPMON_ALARM_MAX so we
	// never read past what was received.
	int nAvail = iBodyLen / 5;
	if (nAvail > DF_ELPMON_ALARM_MAX)
		nAvail = DF_ELPMON_ALARM_MAX;

	int nCount = iLen / 5;

	if (iLen <= 0 || (iLen % 5) != 0 || nCount > nAvail)
	{
		strLog.Format(_T("[%s] : S10F20 LEN suspect. LEN(%d), BODY(%d) - using %d code(s) from body"), GetName(), iLen, iBodyLen, nAvail);
		m_pLogFileAccessor->KMLOG(WARN, strLog);

		nCount = nAvail;
	}

	if (nCount <= 0)
	{
		strLog.Format(_T("[%s] : S10F20 no parsable error code. LEN(%d), BODY(%d)"), GetName(), iLen, iBodyLen);
		m_pLogFileAccessor->KMLOG(WARN, strLog);

		SendAck(20, 1, 0);	// NAK
		return;
	}

	CString strErrorID;
	for (int i = 0; i < nCount; i++)
	{
		const int iStart = 8 + (i * 5);		// fixed offset (GAI defect: iStart += i*5 accumulated)
		const int iCode = _ttoi(strCmd.Mid(iStart, 5));

		if (IsValidAlarmCode(iCode) == true)
		{
			CString strTemp;
			strTemp.Format(_T("%05d, "), iCode);
			strErrorID += strTemp;

			// Raise on a dedicated thread: Heavy alarm blocks until reset.
			AfxBeginThread(ThreadRaiseAlarm, (LPVOID)(INT_PTR)(ELP_ELECBOX_ALARM_START + iCode));
		}
		else
		{
			strLog.Format(_T("[%s] : S10F20 undefined error code(%d) skipped"), GetName(), iCode);
			m_pLogFileAccessor->KMLOG(WARN, strLog);
		}
	}

	strLog.Format(_T("[%s] : S(10), F(20), LEN(%05d), Error Code(%s)"), GetName(), iLen, strErrorID);
	m_pLogFileAccessor->KMLOG(OPER, strLog);

	SendAck(20, 0, 0);	// S20F20 ACK
}

void CCommTcpIpObjectPGMon::xNetCallbackRecieveData(LPVOID ud)
{
	if ((IsConnected() == true) && (m_pSystemConfigMgr->IsCommunicationTypeString() == true))
	{
		LPRECEIVEDDATA pRcvData = (LPRECEIVEDDATA)ud;

		if (pRcvData->eCommState == (eXNET_COMM_STATE)XNET_NOTCONNECT)
		{
			CString strLog;
			strLog.Format(_T("[%s] : %s"), GetName(), _T("Close Connected"));
			m_pLogFileAccessor->KMLOG(SYST, strLog);
		}
		else if (pRcvData->eCommState == (eXNET_COMM_STATE)XNET_CONNECTED)
		{
			CString strIP(inet_ntoa(pRcvData->cliaddr.sin_addr));

			CString strLog;
			strLog.Format(_T("[%s, %d] : %s"), strIP, ntohs(pRcvData->cliaddr.sin_port), _T("Connected"));
			m_pLogFileAccessor->KMLOG(SYST, strLog);
		}
		else if (pRcvData->eCommState == (eXNET_COMM_STATE)XNET_RECEIVEDATA)
		{
			CString strMessage;
			char* pchar = pRcvData->szBody;
			int nLen = (int)pRcvData->sizeBody;

			wchar_t* pszWide = new wchar_t[nLen + 1];
			memset(pszWide, 0, sizeof(wchar_t) * (nLen + 1));

			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pchar, nLen, pszWide, nLen);

			for (int i = 0; i < nLen; i++)
			{
				if (pszWide[i] == 0x00)
				{
					pszWide[i] = ' ';
				}
			}

			pszWide[nLen] = 0x00;

			strMessage.Format(_T("%s"), pszWide);

			delete[] pszWide;
			pszWide = nullptr;

			CString strLog;
			strLog.Format(_T("[%s] : %s"), GetName(), strMessage);
			m_pLogFileAccessor->KMLOG(RECV, strLog);

			strMessage.Replace(_T(" "), _T(""));
			if (strMessage.IsEmpty() == true)
				return;

			m_dwLastRecvTick = ::GetTickCount();

			//---------------------------------------------------------------------------
			// Split by STX/ETX: multiple frames can arrive in one recv event.
			//---------------------------------------------------------------------------
			std::vector<CString> vecPacketCommandPool = GetExeHextoStr(strMessage);

			for (size_t i = 0; i < vecPacketCommandPool.size(); ++i)
			{
				const CString strMonCmd = vecPacketCommandPool[i];

				if (strMonCmd.GetLength() < 8)		// Stream(2)+Func(2)+Len(4)
				{
					strLog.Format(_T("[%s] : frame too short(%s)"), GetName(), strMonCmd);
					m_pLogFileAccessor->KMLOG(WARN, strLog);
					continue;
				}

				if (strMonCmd.Find(eMON_S10F1) == 0)
				{
					ParseS10F01(strMonCmd);
				}
				else if (strMonCmd.Find(eMON_S10F20) == 0)
				{
					ParseS10F20(strMonCmd);
				}
				else
				{
					strLog.Format(_T("[%s] : unknown command(%s)"), GetName(), strMonCmd.Left(8));
					m_pLogFileAccessor->KMLOG(WARN, strLog);
				}
			}
		}
	}
}
