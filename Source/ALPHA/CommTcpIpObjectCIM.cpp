#include "pch.h"
#include "CommTcpIpObjectCIM.h"

#include "Manager\NetworkConfigMgr.h"
#include "Manager\SystemConfigMgr.h"
#include "Manager\ShareMemoryMgr.h"
#include "Alarm\AlarmController.h"
#include "AuroraFuncDefine.h"
#include "Manager\SequenceManager.h"
#include "CommEthFunction\EthernetManagerTCP.h"
#include "common\CommonFunc.h"
#include "TransporterResult.h"
#include "SystemParamShare.h"
#include "Manager/MaintControlMgr.h"
#include "Manager/MCCLogMgr.h"
#include "Manager/SeqTactTimeMgr.h"
#include "Manager/CommTcpIpFactoryMgr.h"
#include "Manager/CCommTcpIpObjectSysTime.h"
#include "Recipe/RecipeEditMgr.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCommTcpIpObjectCIM::CCommTcpIpObjectCIM(int iSeqItemIndex, stAuroraMgrPool& MgrPool)
	: m_pSystemConfigMgr(MgrPool.pSystemConfigMgr)
	, m_pShareMemory(MgrPool.pShareMemoryMgr)
	, m_pMaintControlMgr(MgrPool.pMaintControlMgr)
	, m_pCommTcpIpFactoryMgr(MgrPool.pCommTcpIpFactoryMgr)
	, m_pLogFileAccessor(new CLogFileAccessor)
	, m_bOutOfService(false)
	, m_bUserBypassGlass(false)
	, m_nUserSelectMode(0)
	, m_bProcessHandOn(false)
	, m_MgrPool_link(&MgrPool)
{
	m_strQueueMessage.Empty();
	m_strOldGlsID.Empty();
	m_strGlassModelName.Empty();

	CString strObjectName;
	strObjectName.Format(_T("%s"), _T("CIM"));

	SetName(strObjectName); // _T("UTG"));
	const bool& bRunMode = MgrPool.pNetworkConfigMgr->GetParam_Mode(iSeqItemIndex);
	const CString& strIP = MgrPool.pNetworkConfigMgr->GetParam_IPNumber(iSeqItemIndex);
	const long& nPort = MgrPool.pNetworkConfigMgr->GetParam_PortNumger(iSeqItemIndex);
	SetNetInfo(bRunMode, strIP, nPort);

	m_bOutOfService = MgrPool.pNetworkConfigMgr->GetParam_OutOfService(iSeqItemIndex);

	// 로그 생성하기
	const CString strLogFilePath = MgrPool.strLogPath;
	const CString strFolderPath = strLogFilePath + _T("Control\\") + strObjectName + _T("\\");
	const CString strLogFile = strFolderPath + strObjectName + _T(".log");
	MakeDir(strFolderPath);
	m_pLogFileAccessor->Initialize(strLogFile);
	//////

	// 데이터 수신 확인용 event
	int i = 0;
	for (i = 0; i < CIM_CMD_SERVER_TO_CIM_END; ++i)
	{
		m_hRecvDEvent[i] = CreateEvent(NULL, TRUE, FALSE, NULL);	// auto reset, initial reset
	}

	m_pTransporterResult = new CTransporterResult(m_pSystemConfigMgr->GetConfigFolderPath());
	m_pTransporterResult->SetParam(m_pSystemConfigMgr->GetUploadCIMFolderPath());
	

	//----------------------------------------------------------------------
	// Thread 생성
	//----------------------------------------------------------------------
	m_pThread = AfxBeginThread(ThreadRun, this);
	m_bThreadLive = false;
	m_hStartEvent = CreateEvent(NULL, FALSE, FALSE, NULL);		// auto reset, initial reset
	m_hCloseEvent = CreateEvent(NULL, FALSE, FALSE, NULL);		// auto reset, initial reset

	for (i = 0; i < 6; i++)
	{
		m_nHandShakeXBit[i] = 0;
		m_nHandShakeYBit[i] = 0;
	}
}


CCommTcpIpObjectCIM::~CCommTcpIpObjectCIM()
{
	SetEvent(m_hCloseEvent);

	if (m_pThread )
	{
		WaitForSingleObject(m_pThread->m_hThread, 1000);
	}

	m_pThread = nullptr;

	Finalize();

	for (int i = 0; i < CIM_CMD_SERVER_TO_CIM_END; ++i)
	{
		if (m_hRecvDEvent[i])
		{
			CloseHandle(m_hRecvDEvent[i]);
		}
	}

	if (m_pTransporterResult != nullptr)
	{
		delete m_pTransporterResult;
		m_pTransporterResult = nullptr;
	}
	
	//----------------------------------------------------------------------
	// Data 삭제
	//----------------------------------------------------------------------
	if (m_pLogFileAccessor)
	{
		delete m_pLogFileAccessor;
		m_pLogFileAccessor = nullptr;
	}
}

//=========================================================================
// INTERFACE 관련
//=========================================================================
void CCommTcpIpObjectCIM::xNetCallbackRecieveData(LPVOID ud)
{
	//if (CSystemParamShare::GetInstance()->IsAutoMode() == false)
	//	return;

//	if ((IsConnected() == true) && (m_pSystemConfigMgr->IsCommunicationTypeString() == true)) 
	{
		LPRECEIVEDDATA pRcvData = (LPRECEIVEDDATA)ud;


		if (pRcvData->eCommState == (eXNET_COMM_STATE)XNET_NOTCONNECT)
		{
			CString strIP(inet_ntoa(pRcvData->cliaddr.sin_addr));

			CString strLog;
			strLog.Format(_T("[%s, %d] : %s"), strIP, ntohs(pRcvData->cliaddr.sin_port), _T("Close Connected"));
			m_pLogFileAccessor->KMLOG(SYST, strLog);

			m_pShareMemory->SetSys_MachineMode(df_HOST_OFFLINE);

			CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CHANGE_CONTROLMODE, 3, NULL); // Offline

			m_bProcessHandOn = false;	// jsyoon 2022.07.22 add
		}
		else if (pRcvData->eCommState == (eXNET_COMM_STATE)XNET_CONNECTED)
		{
			CString strIP(inet_ntoa(pRcvData->cliaddr.sin_addr));

			CString strLog;
			strLog.Format(_T("[%s, %d] : %s"), strIP, ntohs(pRcvData->cliaddr.sin_port), _T("Connected"));
			m_pLogFileAccessor->KMLOG(SYST, strLog);
		}
		else if (pRcvData->eCommState == (eXNET_COMM_STATE)XNET_SERVER_ACCEPT)
		{
			CString strIP(inet_ntoa(pRcvData->cliaddr.sin_addr));
			CString strLog;
			strLog.Format(_T("[%s, %d] : %s"), strIP, ntohs(pRcvData->cliaddr.sin_port), _T("ACCEPT"));
			m_pLogFileAccessor->KMLOG(SYST, strLog);
		}
		else if (pRcvData->eCommState == (eXNET_COMM_STATE)XNET_RECEIVEDATA)
		{
			CString strIP(inet_ntoa(pRcvData->cliaddr.sin_addr));
			CString strMessage(pRcvData->szBody);
			CString strLog;
			strLog.Format(_T("[%s, %d] : %s"), strIP, ntohs(pRcvData->cliaddr.sin_port), strMessage);
			m_pLogFileAccessor->KMLOG(RECV, strLog);

			strMessage.Replace(_T(" "), _T(""));
			if (strMessage.IsEmpty() == true)
				return;

		//if (pRcvData->eCommState == (eXNET_COMM_STATE)XNET_NOTCONNECT)
		//{
		//	CString strLog;
		//	strLog.Format(_T("[%s] : %s"), GetName(), _T("Close Connected"));
		//	m_pLogFileAccessor->KMLOG(SYST, strLog);

		//	m_pShareMemory->SetSys_MachineMode(df_HOST_OFFLINE);

		//	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CHANGE_CONTROLMODE, 3, NULL); // Offline

		//	m_bProcessHandOn = false;	// jsyoon 2022.07.22 add
		//}
		//else if (pRcvData->eCommState == (eXNET_COMM_STATE)XNET_RECEIVEDATA)
		//{
		//	CString strMessage(pRcvData->szBody);
		//	//	CString strLog = _T("[RECV - UTG] : ") + strMessage;
		//	CString strLog;
		//	strLog.Format(_T("[%s](%d) : %s"), GetName(), pRcvData->sizeBody, strMessage);

		//	//		CLogWrapper::GetInstance()->KMLOG(SYST, strLog);
		//	m_pLogFileAccessor->KMLOG(RECV, strLog);

		//	strMessage.Replace(_T(" "), _T(""));
		//	if (strMessage.IsEmpty() == true)
		//		return;

			//	m_strQueueMessage += strMessage;

				//---------------------------------------------------------------------------
				// 패킷 단위로 응답에 대한 처리를 해야 한다.
				// socket 에 데이터가 진입하는데 2개 이상 붙어서 1개의 event 로 들어온다.
				// 모든 패킷에 대해서 command 단위로 분리해서 수행해야 한다.
				//---------------------------------------------------------------------------
			std::vector<CString> vecPacketCommandPool = GetCommand(strMessage); 

			for (size_t i = 0; i < vecPacketCommandPool.size(); ++i)
			{
				const CString strRcv = vecPacketCommandPool[i];

				if (strRcv.Find(dfSTS_01_04_OK) >= 0)
				{
					SetEvent(m_hRecvDEvent[TO_CIM_STS0104]);
				}

				else if (strRcv.Find(dfSTS_02_04_OK) >= 0)
				{
					SetEvent(m_hRecvDEvent[TO_CIM_STS0204]);
				}

				else if (strRcv.Find(dfSTS_03_04_OK) >= 0)
				{
					SetEvent(m_hRecvDEvent[TO_CIM_STS0304]);
					//		m_pDlgNaviOperate->EnableControlMode(TRUE);
				}

				else if (strRcv.Find(dfSTS_ORG) >= 0)
				{
					SetEvent(m_hRecvDEvent[TO_CIM_ORG]);
					Sleep(100);
					Cmd_Send(dfSTS_ORG_OK);
				}

				else if (strRcv.Find(dfSTS_05_06_OK) >= 0)
				{
					SetEvent(m_hRecvDEvent[TO_CIM_STS0506]);

					// User Select Measure Mode
					ProcessTo_PPIDREQ();
				}

				else if (strRcv.Find(dfSTS_00_04_OK) >= 0)
				{
					SetEvent(m_hRecvDEvent[TO_CIM_STS0004]);
				}

				else if (strRcv.Find(dfSTS_11_06_OK) >= 0)
				{
					SetEvent(m_hRecvDEvent[TO_CIM_STS1106]);
				}

				//ksk 20150728 add for Alarm From CIM
				else if (strRcv.Find(dfSTS_ALARM) >= 0)
				{
					int nCmdStart = 6;
					int nCmdEnd = 0;
					nCmdEnd = strRcv.GetLength();
					CString strAlarm = _T("");
					strAlarm = strRcv.Mid(nCmdStart, nCmdEnd - nCmdStart);
					int nAlarmID = _ttoi(strAlarm);

					//	m_pSystem->m_bCIMAlarmReported = TRUE;
				//	CWindowEventHandler::GetInstance()->PostMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CHANGE_CIM_STATE, TO_SERVER_ALARM, nAlarmID);

					CAlarmController::GetInstance()->SetAlarm(nAlarmID);

					//	m_pSystem->m_bCIMAlarmReported = FALSE;
				}

				//PPID or Recipe ID
				else if (strRcv.Find(dfPPID) >= 0 && strRcv.Left(4) == dfPPID)
				{
					m_strRcvPPID = strRcv;
					SetEvent(m_hRecvDEvent[TO_CIM_PPID]);
				}

				else if (strRcv.Find(dfSTS_20_04_OK) >= 0)
				{
					SetEvent(m_hRecvDEvent[TO_CIM_STS2004]);
				}

				else if (strRcv.Find(dfMULTIRCP) >= 0)				// pwk 20130904
				{
					Sleep(200);
					Cmd_Send(dfMULTIRCP_OK);

					// Multi Measure Mode
					ProcessTo_PPIDREQ();
				}

				//GLSREADY - Glass Exchange
				else if (strRcv.Find(dfGLSREADY) >= 0)
				{
					Sleep(200);
					Cmd_Send(dfGLSREADY_OK);

					// Next Glass Measure Mode
					ProcessTo_PPIDREQ();
					//	ProcessTo_GLSREADY();
				}



				//LOTID^LOTID                         ^CSTID                         ^OPERID                        $
				else if (strRcv.Find(dfLOTID) >= 0 && strRcv.Left(5) == dfLOTID)
			//	else if (strRcv.Find(dfLOTID) >= 0 ) 
				{
					m_strRcvLotID = strRcv;
					SetEvent(m_hRecvDEvent[TO_CIM_LOTID]);
				}

				//GLSID^GLASSID                       ^1                             ^1                             $
				else if (strRcv.Find(dfGLSID) >= 0 || strRcv.Left(5) == dfGLSID)
				{
					m_strRcvGlsID = strRcv;
					SetEvent(m_hRecvDEvent[TO_CIM_GLSID]);
				}

				//Glass In Complete
				else if (strRcv.Find(dfSTS_00_06_OK) >= 0)
				{
					SetEvent(m_hRecvDEvent[TO_CIM_STS0006]);
				}

				//PRCS (Process Start)
				else if (strRcv.Find(dfPRCS) >= 0)
				{
					Process_Start();
				}

				//Process Start Status. Step#1
				else if (strRcv.Find(dfSTS_10_05_OK) >= 0)
				{
					SetEvent(m_hRecvDEvent[TO_CIM_STS1005]);
					//Sleep(200);										
					//Cmd_Send(dfSTS_10_06);
				}

				//Now Eq Process Status. Step#2
				else if (strRcv.Find(dfSTS_10_06_OK) >= 0)
				{
					SetEvent(m_hRecvDEvent[TO_CIM_STS1006]);

					//  //		m_pDlgNaviOperate->DoStartMeasure();
					   //if (CanDoStartPRCS(FALSE, TRUE))
					   //{
					   //	// jsyoon 20150302 add
					   //	CString StrTmp, strLog;
					   //	StrTmp.Format("%s", m_pSystem->stGlassInfo.m_szGlassID);

					   //	ReadGalssExistFile();	// Read Stop GlassID
					   //	if (strcmp(m_strStopGlassID, StrTmp) == 0)
					   //	{
					   //		strLog.Format("StopGlassID(%s), Question is Measure or Off-line(PM)", StrTmp);
					   //		gLog(m_pSystem->hLog, strLog);
					   //		if (AfxMessageBox(_T("레시피 수정을 위해 설비를 정지할까요?"), MB_YESNO | MB_ICONQUESTION) != IDYES)
					   //		{
					   //			m_pDlgNaviOperate->DoStartMeasure();
					   //		}
					   //		else	// Off-line
					   //		{
					   //			Cmd_Send(dfAT_0);
					   //		}
					   //	}
					   //	else
					   //	{
					   //		if (m_strStopGlassID.Find("HOLD_USER") >= 0)
					   //		{
					   //			strLog.Format("StopGlassID(%s), Question is Measure or Off-line(PM)", StrTmp);
					   //			gLog(m_pSystem->hLog, strLog);
					   //			if (AfxMessageBox(_T("레시피 수정을 위해 설비를 정지할까요?"), MB_YESNO | MB_ICONQUESTION) != IDYES)
					   //			{
					   //				m_pDlgNaviOperate->DoStartMeasure();
					   //			}
					   //			else	// Off-line
					   //			{
					   //				Cmd_Send(dfAT_0);
					   //			} 
					   //		}
					   //		else
					   //		{
					   //			m_pDlgNaviOperate->DoStartMeasure();
					   //		}

					   //	}
					   //}
					   //else
					   //{
					   //	gLog(m_pSystem->hLog, "dfSTS_10_06_OK But HandShakeComplete FALSE");
					   //	CAlarmController::GetInstance()->SetAlarm(CIM_RV_HANDSHAKE_FAIL);
					   //}
				}

				//Glass Data Request
				else if (strRcv.Find(dfGDTREQ) >= 0)
				{
					ProcessTo_GDTCPL();
				}

				//Reply for Prepare Glass Out Wait.
				else if (strRcv.Find(dfSTS_21_06_OK) >= 0)
				{
					SetEvent(m_hRecvDEvent[TO_CIM_STS2106]);
				}

				//Reply for Prepare Glass Out Wait.
				else if (strRcv.Find(dfSTS_21_07_OK) >= 0)
				{
					m_bUserBypassGlass = false;
					SetEvent(m_hRecvDEvent[TO_CIM_STS2107]);
				}

				//Unload Complete
				else if (strRcv.Find(dfULDCPL) >= 0)
				{
					ProcessTo_ULDCPL();
				}

				else if (strRcv.Find(dfLDCPL) >= 0)
				{
					Sleep(200);
					Cmd_Send_LDCPL(true);

					// First Measure Mode
					ProcessTo_PPIDREQ();
				}

				//Unload Request
				else if (strRcv.Find(dfULDREQ) >= 0)
				{
					ProcessTo_ULDREQ();
				}
				else if (strRcv.Find(dfLDREQ) >= 0)	 	//Load Request	// jsyoon 20141224 change
				{
					ProcessTo_LDREQ();
				}

				else if (strRcv.Find(dfSTS_21_04_OK) >= 0)
				{
					Cmd_Send(dfSTS_21_05);
				}

				else if (strRcv.Find(dfSTS_21_05_OK) >= 0)
				{
					;
				}





				//EQP Abort Current Glass
				else if (strRcv.Find(dfEQPABORT) >= 0)
				{
					//	::PostMessage(this->GetParent(), WM_MSG_MAINT, (WPARAM)VTM_CIM, (LPARAM)TO_SERVER_EQPABORT);
				}

				//EQP Pause Current Glass
				else if (strRcv.Find(dfEQPPAUSE) >= 0)
				{
					//	::PostMessage(this->GetParent()->m_hWnd, WM_MSG_MAINT, (WPARAM)VTM_CIM, (LPARAM)TO_SERVER_EQPPAUSE);
				}

				//EQP Abort Current Glass
				else if (strRcv.Find(dfEQPRESTART) >= 0)
				{
					//	::PostMessage(this->GetParent()->m_hWnd, WM_MSG_MAINT, (WPARAM)VTM_CIM, (LPARAM)TO_SERVER_EQPRESTART);
				}

				//밑에 코드는 아직 검증이 안된 Sequence임
				else if (strRcv.Find(dfMD_1) >= 0)
				{
					Cmd_Send(dfMD_1_OK);
				}

				else if (strRcv.Find(dfMD_0) >= 0)
				{
					Cmd_Send(dfMD_0_OK);
				}

				else if (strRcv.Find(dfATREQ) >= 0)
				{
					Process_SetHostMode();
				}

				else if (strRcv.Find(dfAT_1_OK) >= 0)
				{
					SetEvent(m_hRecvDEvent[TO_CIM_AT1]);
					//		Process_SetHostMode(df_HOST_ONLINE_REMOTE);
				}

				else if (strRcv.Find(dfAT_0_OK) >= 0)
				{
					SetEvent(m_hRecvDEvent[TO_CIM_AT0]);
					//		Process_SetHostMode(df_HOST_OFFLINE);
				}

				else if (strRcv.Find(dfEQPSTART) >= 0)
				{
					SetEvent(m_hRecvDEvent[TO_CIM_EQPSTART]);
				}

				else if (strRcv.Find(dfGLSSTART) >= 0)
				{
					//		
					Cmd_Send(dfGLSSTART_OK);
				}

				else if (strRcv.Find(dfMCCON) >= 0)
				{
					Process_SetMCCTransfer(true);
				}

				else if (strRcv.Find(dfMCCOFF) >= 0)
				{
					Process_SetMCCTransfer(false);
				}

				else if (strRcv.Find(dfMCCTIME) >= 0)		// pwk 20120925 add
				{
					Process_SetMCCTime(strRcv);
				}

				else if (strRcv.Find(dfALMACL_OK) >= 0)
				{
					SetEvent(m_hRecvDEvent[TO_CIM_ALMACL]);
				}

				else if (strRcv.Find(dfTIMESET) >= 0)
				{
					SetEvent(m_hRecvDEvent[TO_CIM_TIMEREQ]);
					ProcessTIME(strRcv);
				}

				else if (strRcv.Find(dfHSON) >= 0)
				{
					ProcessHandOn();
				}

				else if (strRcv.Find(dfHSOFF) >= 0)
				{
					ProcessHandOff();
				}
				else if (strRcv.Find(dfSTS_HANDSHAKE) >= 0)
				{
					ProcessHandShake(strRcv);
				}
				else if (strRcv.Find(dfSTS_COMPONENT) >= 0)
				{
					ProcessComponent();
				}
				else
				{
					;
				}

				strLog.Format(_T("[%s] : %s, End"), GetName(), strRcv);
				m_pLogFileAccessor->KMLOG(OPER, strLog);
			}
		}
	}
}

CLogFileAccessor* CCommTcpIpObjectCIM::GetLogFileAccessorPtr()
{
	return m_pLogFileAccessor;
}

int CCommTcpIpObjectCIM::Cmd_Send(CString strSend)
{
	// TODO: 여기에 구현 코드 추가.
	int nRtn = VTM_RV_SUCCESSED;

	if (strSend.CompareNoCase(dfAT_0) == 0)
	{
		int i = 0;
		for (i = 0; i < 6; i++)
		{
			// SIGNAL
			if (m_nHandShakeXBit[i] == 1)
			{
				if (i < 3)
					CMCCLogMgr::GetInstance()->SetMCC_Signal(MCC_S_X_SEND_ABLE + i, 0, 1);
				else
					CMCCLogMgr::GetInstance()->SetMCC_Signal(MCC_S_X_SEND_ABLE + i, 0, 2);

				m_nHandShakeXBit[i] = 0;
			}

			if (m_nHandShakeYBit[i] == 1)
			{
				if (i < 3)
					CMCCLogMgr::GetInstance()->SetMCC_Signal(MCC_S_Y_RCV_ABLE + i, 0, 1);
				else
					CMCCLogMgr::GetInstance()->SetMCC_Signal(MCC_S_Y_RCV_ABLE + i, 0, 2);

				m_nHandShakeYBit[i] = 0;
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////

	if (IsOutOfService() == TRUE)
		return VTM_RV_SUCCESSED;

	int iFind = 0, iFind2 = 0;
	iFind = strSend.CompareNoCase(dfAT_0);
	iFind2 = strSend.CompareNoCase(dfAT_1);
	if (iFind != 0 && iFind2 != 0)		// jsyoon 20150422 add
	{
		if (m_pShareMemory->GetSys_MachineMode() == df_HOST_OFFLINE)
		{
			return CIM_COMMAND_ERROR;
		}
	}

	bool bRet = false;
	if (strSend.Find(_T("$")) > 0)
	{
		bRet = SendPacket(strSend, false, false);
	}
	else
	{
		bRet = SendPacket(strSend, true, false);
	}

	CString strLog;
	strLog.Format(_T("[%s](%d): %s"), GetName(), bRet, strSend);
	m_pLogFileAccessor->KMLOG(SEND, strLog);

	if (bRet == true)
	{
		nRtn = VTM_RV_SUCCESSED;
	}
	else
	{
		strLog.Format(_T("[%s] : %s"), GetName(), _T("CIM_NOT_CONNECTED"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		CAlarmController::GetInstance()->SetAlarm(CIM_RV_SOCKET_CREATE_FAIL);
		nRtn = CIM_RV_SOCKET_CREATE_FAIL;
	}

	return nRtn;
}


int CCommTcpIpObjectCIM::ProcessComponent()
{
	// TODO: 여기에 구현 코드 추가.
	int nBit = 0, i = 0;
	if (m_nHandShakeYBit[1] == 1)			// Y_RCV_START -> X_SEND_START
	{
		i = 1;
		CMCCLogMgr::GetInstance()->SetMCC_Signal(MCC_S_X_SEND_ABLE + i, 1, 1);	// SIGNAL START
		m_nHandShakeXBit[i] = 1;
	}
	else if (m_nHandShakeYBit[4] == 1)		// Y_SEND_START -> X_RCV_START
	{
		i = 4;
		CMCCLogMgr::GetInstance()->SetMCC_Signal(MCC_S_X_SEND_ABLE + i, 1, 2);	// SIGNAL START
		m_nHandShakeXBit[i] = 1;
	}
	else									// COMPLETE
	{
		for (i = 0; i < 6; i++)
		{
			if (m_nHandShakeXBit[i] == 1)	// 
			{
				nBit = 0;
				if (i < 3)
					CMCCLogMgr::GetInstance()->SetMCC_Signal(MCC_S_X_SEND_ABLE + i, nBit, 1);	// SIGNAL END
				else
					CMCCLogMgr::GetInstance()->SetMCC_Signal(MCC_S_X_SEND_ABLE + i, nBit, 2);	// SIGNAL END

				m_nHandShakeXBit[i] = nBit;
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////

	// Glass 반입							CIM					Loader
	//HANDSHAKE ^ 1 ^ 0 ^ 0 ^ 0 ^ 0 ^ 0$	Y_RCV_ABLE
	//														   X_SEND_ABLE
	//HANDSHAKE ^ 1 ^ 1 ^ 0 ^ 0 ^ 0 ^ 0$    Y_RCV_START
	//COMPONENT ^ 1 ^ 0 ^ 0 ^ 0$							   X_SEND_START

	//HS_ON$
	//HANDSHAKE ^ 1 ^ 1 ^ 1 ^ 0 ^ 0 ^ 0$	Y_RCV_COMPLETE
	//HANDSHAKE ^ 0 ^ 0 ^ 0 ^ 0 ^ 0 ^ 0$
	//HS_OFF$												  

	//COMPONENT ^ 0 ^ 1 ^ 0 ^ 0$							   X_SEND_COMPLETE

	// Glass 배출
	//HANDSHAKE ^ 0 ^ 0 ^ 0 ^ 1 ^ 0 ^ 0$    Y_SEND_ABLE
	//															X_RCV_ABLE
	//HANDSHAKE ^ 0 ^ 0 ^ 0 ^ 1 ^ 1 ^ 0$	Y_SEND_START
	//COMPONENT ^ 0 ^ 0 ^ 1 ^ 0$								X_RCV_START

	//HS_ON$	
	//HANDSHAKE ^ 0 ^ 0 ^ 0 ^ 1 ^ 1 ^ 1$    Y_SEND_COMPLETE
	//HANDSHAKE ^ 0 ^ 0 ^ 0 ^ 0 ^ 0 ^ 0$						
	//HS_OFF$	

	//COMPONENT ^ 0 ^ 0 ^ 0 ^ 1$								X_RCV_COMPLETE

	// Glass Exchange
	//HANDSHAKE ^ 0 ^ 0 ^ 0 ^ 1 ^ 0 ^ 0$
	//COMPONENT ^ 0 ^ 0 ^ 1 ^ 0$
	//HANDSHAKE ^ 0 ^ 0 ^ 0 ^ 1 ^ 1 ^ 0$
	//COMPONENT ^ 0 ^ 0 ^ 0 ^ 1$
	//COMPONENT ^ 1 ^ 0 ^ 0 ^ 0$
	//HANDSHAKE ^ 0 ^ 0 ^ 0 ^ 1 ^ 1 ^ 1$
	//HANDSHAKE ^ 0 ^ 0 ^ 0 ^ 0 ^ 0 ^ 0$
	//COMPONENT ^ 0 ^ 1 ^ 0 ^ 0$
	//HS_OFF$

	return 0;
}


int CCommTcpIpObjectCIM::ProcessHandShake(CString strMsg)
{
	// TODO: 여기에 구현 코드 추가.
	int iStart = 0;
	int iEnd = 0;
	bool bToggle = true;
	CString strTemp;
	CStringArray strArray;
	strArray.RemoveAll();
	CString strLog;
	int iIndex = 0;
	iIndex = strMsg.Find(dfSTS_HANDSHAKE);

	for (int i = 9; i < strMsg.GetLength(); i++)
	{
		if (bToggle == true)
		{
			iStart = i;
			bToggle = false;
		}

		if (strMsg.GetAt(i) == '^')
		{
			bToggle = true;
			iEnd = i;
			iEnd -= iStart;

			strTemp = strMsg.Mid(iStart, iEnd);
			strArray.Add(strTemp);
		}
		else if (strMsg.GetAt(i) == '$')
		{
			iEnd = i;
			iEnd -= iStart;

			strTemp = strMsg.Mid(iStart, iEnd);
			strArray.Add(strTemp);
		}
	}

	if (strArray.GetSize() < 0 || strArray.GetSize() < 6)
	{
		strLog.Format(_T("[%s] : %s"), GetName(), _T("HandShake Data Format is Incorrect!!"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		
		return 0;
	}

	int nBit = 0, i = 0;
	for ( i = 0; i < 6; i++) 
	{
		strTemp = strArray.GetAt(i);  
		strTemp.TrimRight();

		nBit = _ttoi(strTemp);
		if (m_nHandShakeYBit[i] != nBit)
		{
			if (i < 3)
			{
				CMCCLogMgr::GetInstance()->SetMCC_Signal(MCC_S_Y_RCV_ABLE + i, nBit, 1);	// SIGNAL START
			}
			else
			{
				CMCCLogMgr::GetInstance()->SetMCC_Signal(MCC_S_Y_RCV_ABLE + i, nBit, 2);	// SIGNAL END
			}

			m_nHandShakeYBit[i] = nBit;
		}
	}

	// Glass 반입							CIM					Loader
	//HANDSHAKE ^ 1 ^ 0 ^ 0 ^ 0 ^ 0 ^ 0$	Y_RCV_ABLE
	//														   X_SEND_ABLE
	//HANDSHAKE ^ 1 ^ 1 ^ 0 ^ 0 ^ 0 ^ 0$    Y_RCV_START
	//COMPONENT ^ 1 ^ 0 ^ 0 ^ 0$							   X_SEND_START

	//HS_ON$
	//HANDSHAKE ^ 1 ^ 1 ^ 1 ^ 0 ^ 0 ^ 0$	Y_RCV_COMPLETE
	//HANDSHAKE ^ 0 ^ 0 ^ 0 ^ 0 ^ 0 ^ 0$
	//HS_OFF$												  

	//COMPONENT ^ 0 ^ 1 ^ 0 ^ 0$							   X_SEND_COMPLETE
	
	// Glass 배출
	//HANDSHAKE ^ 0 ^ 0 ^ 0 ^ 1 ^ 0 ^ 0$    Y_SEND_ABLE
	//															X_RCV_ABLE
	//HANDSHAKE ^ 0 ^ 0 ^ 0 ^ 1 ^ 1 ^ 0$	Y_SEND_START
	//COMPONENT ^ 0 ^ 0 ^ 1 ^ 0$								X_RCV_START

	//HS_ON$	
	//HANDSHAKE ^ 0 ^ 0 ^ 0 ^ 1 ^ 1 ^ 1$    Y_SEND_COMPLETE
	//HANDSHAKE ^ 0 ^ 0 ^ 0 ^ 0 ^ 0 ^ 0$						
	//HS_OFF$	

	//COMPONENT ^ 0 ^ 0 ^ 0 ^ 1$								X_RCV_COMPLETE

	// Glass Exchange
	//HANDSHAKE ^ 0 ^ 0 ^ 0 ^ 1 ^ 0 ^ 0$
	//COMPONENT ^ 0 ^ 0 ^ 1 ^ 0$
	//HANDSHAKE ^ 0 ^ 0 ^ 0 ^ 1 ^ 1 ^ 0$
	//COMPONENT ^ 0 ^ 0 ^ 0 ^ 1$
	//COMPONENT ^ 1 ^ 0 ^ 0 ^ 0$
	//HANDSHAKE ^ 0 ^ 0 ^ 0 ^ 1 ^ 1 ^ 1$
	//HANDSHAKE ^ 0 ^ 0 ^ 0 ^ 0 ^ 0 ^ 0$
	//COMPONENT ^ 0 ^ 1 ^ 0 ^ 0$
	//HS_OFF$

	return 1;
}


int CCommTcpIpObjectCIM::ProcessHandOff()
{
	// TODO: 여기에 구현 코드 추가.
	int nBit = 0, i = 0;
	if (m_nHandShakeYBit[2] == 0)			// Y_RCV_COMPLETE -> X_SEND_COMPLETE
	{
		i = 2;
		CMCCLogMgr::GetInstance()->SetMCC_Signal(MCC_S_X_SEND_ABLE + i, 1, 1);	// SIGNAL START
		m_nHandShakeXBit[i] = 1;
	}
	else if (m_nHandShakeYBit[5] == 0)		// Y_SEND_COMPLETE -> X_RCV_COMPLETE
	{
		i = 5;
		CMCCLogMgr::GetInstance()->SetMCC_Signal(MCC_S_X_SEND_ABLE + i, 1, 2);	// SIGNAL START
		m_nHandShakeXBit[i] = 1;
	}
	//////////////////////////////////////////////////////////////////////////

	if (m_pMaintControlMgr->IsGlassExist() == true)	// COMPONENT_IN
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_COMPONENT_IN, FALSE, 0, 1);
	}
	else										// COMPONENT_OUT
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_COMPONENT_OUT, FALSE, 0, 2);
	}
	//////////////////////////////////////////////////////////////////////////

	m_bProcessHandOn = false;			//jhseo 20140508

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CHANGE_CONTROLMODE, 1, NULL); // Hands Off

	//m_pJobProc->UpdateProcess(WORK_GLASS_HANDSHAKE, FALSE);

	return 0;
}


int CCommTcpIpObjectCIM::ProcessHandOn()
{
	// TODO: 여기에 구현 코드 추가.
	int nBit = 0, i = 0;
	if (m_nHandShakeYBit[0] == 1)			// Y_RCV_ABLE -> X_SEND_ABLE
	{
		i = 0;
		CMCCLogMgr::GetInstance()->SetMCC_Signal(MCC_S_X_SEND_ABLE + i, 1, 1);	// SIGNAL START
		m_nHandShakeXBit[i] = 1;
	}
	else if (m_nHandShakeYBit[3] == 1)		// Y_SEND_ABLE -> X_RCV_ABLE
	{
		i = 3;
		CMCCLogMgr::GetInstance()->SetMCC_Signal(MCC_S_X_SEND_ABLE + i, 1, 2);	// SIGNAL START
		m_nHandShakeXBit[i] = 1;
	}
	//////////////////////////////////////////////////////////////////////////

	if (m_pMaintControlMgr->IsGlassExist() == true)	// COMPONENT_OUT
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_COMPONENT_OUT, TRUE, 0, 2);
	}
	else										// COMPONENT_IN
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_COMPONENT_IN, TRUE, 0, 1);
	}
	//////////////////////////////////////////////////////////////////////////

	m_bProcessHandOn = true;			//jhseo 20140508

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CHANGE_CONTROLMODE, 0, NULL); // Hands On

	//m_pJobProc->UpdateProcess(WORK_GLASS_HANDSHAKE, TRUE);

	return 0;
}


int CCommTcpIpObjectCIM::ProcessTIME(CString strMsg)
{
	// TODO: 여기에 구현 코드 추가.
	m_pCommTcpIpFactoryMgr->GetCommTcpIpObjectSystime()->Cmd_Send_SysTime(strMsg);

	return 0;


//	m_pDlgNaviOperate->DoJobSystemChangeMode();
	// jsyoon 2022.08.11 change
	CString strYear = strMsg.Mid(8, 4);
	CString strMonth = strMsg.Mid(12, 2);
	CString strDay = strMsg.Mid(14, 2);
	CString strHour = strMsg.Mid(16, 2);
	CString strMin = strMsg.Mid(18, 2);
	CString strSec = strMsg.Mid(20, 2);

	SYSTEMTIME stcurTime, stNewTime;	//--> 시간을 가져오기 위한 struct문이다.
	GetLocalTime(&stcurTime);			//--> 현 시스템에 대한 시간을 가져온다.
	stNewTime = stcurTime;

	stNewTime.wYear = (unsigned short)_tstoi(strYear);
	stNewTime.wMonth = (unsigned short)_tstoi(strMonth);
	stNewTime.wDay = (unsigned short)_tstoi(strDay);
	stNewTime.wHour = (unsigned short)_tstoi(strHour);
	stNewTime.wMinute = (unsigned short)_tstoi(strMin);
	stNewTime.wSecond = (unsigned short)_tstoi(strSec);

//	BOOL bRet = SetLocalTime(&stNewTime);
//	if (bRet == 0)
//	{
//		GetLastError();
//	}

	return 0;
}


int CCommTcpIpObjectCIM::Process_EQPStart()
{
	// TODO: 여기에 구현 코드 추가.
	int nRet = VTM_RV_SUCCESSED;
	nRet = Cmd_Send(dfEQPSTART_OK);

	if (nRet == VTM_RV_SUCCESSED)
	{
		CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CHANGE_CONTROLMODE, 2, NULL);  // Online

		CString str;
		str.Format(_T("%s"), m_pSystemConfigMgr->GetUploadCIMFolderPath());

		m_pTransporterResult->SetParam_FTPFileUploadRemotePath(_T(""));
		m_pTransporterResult->SetParam(str);
		m_iTransporterType = 0;
		SetEvent(m_hStartEvent);
	}

	

	//m_iTransporterType = 0;
	//CString str;
	//str.Format(_T("%s"), m_pSystemConfigMgr->GetUploadCIMFolderPath());
	//m_pTransporterResult->SetParam(str);

	//SetEvent(m_hStartEvent);

	//		Sleep(500);
	//BOOL bRet = m_pDlgNaviData->SendRcpListFileToFTP();
	//if (!bRet)
	//	gLog(m_pSystem->hLog, "SendRcpListFileToFTP Fail", LOG_LEVEL_E);

	//bRet = m_pDlgNaviData->SendPPIDFileToFTP();			// pwk 20120917
	//if (!bRet)
	//	gLog(m_pSystem->hLog, "SendPPIDFileToFTP Fail", LOG_LEVEL_E);

	/*Cmd_Send(dfRCPLIST);
	Sleep(200);

	Cmd_Send(dfALMACL);*/

	return nRet; 
}


int CCommTcpIpObjectCIM::Process_SetMCCTime(CString strMsg)
{
	// TODO: 여기에 구현 코드 추가.
	int nCmdStart = 11;		// MCCTIMESET^HHMM$
	int iHour, iMinute;
	CString strHour, strMinute;
	strHour = strMsg.Mid(nCmdStart, 2);
	strMinute = strMsg.Mid(nCmdStart + 2, 2);
	iHour = _ttoi(strHour);
	iMinute = _ttoi(strMinute);
//	m_pSystem->m_iMCCUploadTime = iHour * 60 + iMinute;

	return 0;
}


int CCommTcpIpObjectCIM::Process_SetMCCTransfer(bool bOn)
{
	// TODO: 여기에 구현 코드 추가.
	// if( bOn)
	//	m_pSystem->m_bMccTransferOn = TRUE;
	//else
	//m_pSystem->m_bMccTransferOn = FALSE;
	return 0;
}


int CCommTcpIpObjectCIM::Process_SetHostMode(int iMode)
{
	// TODO: 여기에 구현 코드 추가.
	switch (iMode)
	{
	case df_HOST_OFFLINE:
	{
		//m_pSystem->stTitle.m_nHostMode = df_HOST_OFFLINE;
	//m_pSystem->stTitle.m_nLPMode = df_HOST_OFFLINE;

	//// jsyoon 20150422 add
	//m_pDlgNaviOperate->SetFunctionButtonStatus(FALSE);
	}
	break;
	case df_HOST_ONLINE_REMOTE:
	{
		//m_pSystem->stTitle.m_nHostMode = df_HOST_ONLINE_REMOTE;
		//m_pSystem->stTitle.m_nLPMode = df_HOST_ONLINE_REMOTE;

		//// jsyoon 20150422 add
		//m_pDlgNaviOperate->SetFunctionButtonStatus(TRUE);

		// jsyoon 20170210 add
		int i = 0;
		for (i = 0; i < 6; i++)
		{
			m_nHandShakeXBit[i] = 0;
			m_nHandShakeYBit[i] = 0;
		}
		////////////////////////////////////////////////////////////////////////
	}
	break;
	case -1:
	{
		if (m_pShareMemory->GetSys_MachineMode() == df_HOST_OFFLINE)
		{
			Cmd_Send(dfAT_0);
		}

		if (m_pShareMemory->GetSys_MachineMode() == df_HOST_ONLINE_REMOTE)
		{
			Cmd_Send(dfAT_1);
		}
	}
	break;
	default:
		break;
	}
	
	return 0;
}


int CCommTcpIpObjectCIM::Process_LOTID(CString strMsg)
{
	// TODO: 여기에 구현 코드 추가.
	bool bRet = true;
	int iStart = 0;
	int iEnd = 0;
	bool bToggle = TRUE;
	CString strTemp;
	CStringArray strArray;
	strArray.RemoveAll();
	CString strLog;
	int iIndex = 0;
	iIndex = strMsg.Find(dfLOTID);

	for (int i = 6; i < strMsg.GetLength(); i++)
	{
		if (bToggle)
		{
			iStart = i;
			bToggle = FALSE;
		}

		if (strMsg.GetAt(i) == '^')
		{
			bToggle = TRUE;
			iEnd = i;
			iEnd -= iStart;

			strTemp = strMsg.Mid(iStart, iEnd);
			strArray.Add(strTemp);
		}
		else if (strMsg.GetAt(i) == '$')
		{
			iEnd = i;
			iEnd -= iStart;

			strTemp = strMsg.Mid(iStart, iEnd);
			strArray.Add(strTemp);
		}
	}

	if (strArray.GetSize() <= 0 || strArray.GetSize() < 3)
	{
		strLog.Format(_T("LOT Data Format is Incorrect!!"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		bRet = false;
	}

	//LOTID
	CString strLotId = _T("");

	if (bRet == true)
	{
		strLotId = strArray.GetAt(0);
		strLotId.TrimRight();

		if (strLotId.IsEmpty())
		{
			strLog.Format(_T("LOT ID is Blank!!"));
			m_pLogFileAccessor->KMLOG(WARN, strLog);
			bRet = false;
		}
		else
		{
			CSystemParamShare::GetInstance()->SetLotID(strLotId);
		}
	}
	
	//CSTID
	CString strCstID = _T("");

	if (bRet == true)
	{
		strCstID = strArray.GetAt(1);
		strCstID.TrimRight();

		CSystemParamShare::GetInstance()->SetCassetteID(strCstID);
	}
	

	//OPERID
	CString strOperID = _T("");

	if (bRet == true)
	{
		strOperID = strArray.GetAt(2);
		strOperID.TrimRight();

		CSystemParamShare::GetInstance()->SetOperID(strOperID);
	}
	
	int nRet = VTM_RV_SUCCESSED;

	if (bRet != true)
	{
		nRet = CIM_RV_LOTID_NG;
	}
	
	return nRet;
}


int CCommTcpIpObjectCIM::Process_GLSID(CString strMsg)
{
	// TODO: 여기에 구현 코드 추가.
	bool bRet = true;
	int iStart = 0;
	int iEnd = 0;
	bool bToggle = TRUE;
	CString strTemp;
	CStringArray strArray;
	strArray.RemoveAll();
	CString strLog;
	int iIndex = 0;
	iIndex = strMsg.Find(dfGLSID);

	for (int i = 6; i < strMsg.GetLength(); i++)
	{
		if (bToggle)
		{
			iStart = i;
			bToggle = FALSE;
		}

		if (strMsg.GetAt(i) == '^')
		{
			bToggle = TRUE;
			iEnd = i;
			iEnd -= iStart;

			strTemp = strMsg.Mid(iStart, iEnd);
			strArray.Add(strTemp);
		}
		else if (strMsg.GetAt(i) == '$')
		{
			iEnd = i;
			iEnd -= iStart;

			strTemp = strMsg.Mid(iStart, iEnd);
			strArray.Add(strTemp);
		}
	}

	if (strArray.GetSize() < 0 || strArray.GetSize() < 3)
	{
		strLog.Format(_T("GLASS Data Format is Incorrect!!"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		bRet = false;
	}

	//GLSID
	CString strGlsId = _T("");

	if (bRet == true)
	{
		strGlsId = strArray.GetAt(0);
		strGlsId.TrimRight();
		if (strGlsId.IsEmpty())
		{
			strLog.Format(_T("GLASS ID is Blank!!"));
			m_pLogFileAccessor->KMLOG(WARN, strLog);
			bRet = false;
		}
		else
		{
			CSystemParamShare::GetInstance()->SetGlassID(strGlsId);
		}
	}
	

	//SLOTID
	CString strSlotID = _T("");
	if (bRet == true)
	{
		strSlotID = strArray.GetAt(1);
		strSlotID.TrimRight();

		CSystemParamShare::GetInstance()->SetSlotID(strSlotID);
	}

	

	//StepID
	CString strStepID = _T("");
	if (bRet == true)
	{
		strStepID = strArray.GetAt(2);
		strStepID.TrimRight();

		CSystemParamShare::GetInstance()->SetStepID(strSlotID);
	}
	

	//Glass Rotate
	CString strRotate = _T("0");

	if (bRet == true)
	{
		if (strArray.GetSize() > 3)
		{
			strRotate = strArray.GetAt(3);
		}
	
		strRotate.TrimRight();

		CSystemParamShare::GetInstance()->SetGlassRot(strRotate);
	}
	
	int nRet = VTM_RV_SUCCESSED;

	if (m_pMaintControlMgr->IsGlassExist() == true)
	{
		//// jsyoon 20150327 add
		//m_pJobProc->m_dGlassTime[0] = gf_GetCurTSec();
		//m_pSystem->bGlassLoadState = TRUE;

		//m_pJobProc->ClearGlassData();

		m_pSystemConfigMgr->WriteGalssExistFile(true);
		//WriteGalssExistFile();
		////////////////////////////////////////////////////////////////////////////

		tickGlassMeas.Reset();
		strLog.Format(_T("Glass Load Complete :LotID : %s,  GlassID : %s"), CSystemParamShare::GetInstance()->GetLotID(), CSystemParamShare::GetInstance()->GetGlassID());
		if (GetSeqTactTimeMgrPtr() != nullptr) GetSeqTactTimeMgrPtr()->SetLogFile(strLog);
	}
	else
	{
		nRet = PLC_GLASS_OBLIVIOUS_WORKPLATE_SENSOR_ALARM;

		strLog.Format(_T("Glass Load Complete Fail :LotID : %s,  GlassID : %s"), CSystemParamShare::GetInstance()->GetLotID(), CSystemParamShare::GetInstance()->GetGlassID());
		if (GetSeqTactTimeMgrPtr() != nullptr) GetSeqTactTimeMgrPtr()->SetLogFile(strLog);
	}

	if (nRet == VTM_RV_SUCCESSED)
	{
		if (bRet != true)
		{
			nRet = CIM_RV_GLASSID_NG;
			Cmd_Send(dfGLSID_NG);
		}
		else
		{
			//알람발생된 글라스를 재측정 할때는 CIM 알람 보고 안한다
			CString strCurGlsID = strGlsId;
			if (m_pSystemConfigMgr->IsUseCurGlsOneAlarm() == true) {

				if (strCurGlsID == m_strOldGlsID)
				{
					m_pShareMemory->SetSys_Validation(true);
				}

				m_strOldGlsID = strCurGlsID;
			}

			//개발런 문자 있을시 CIM 알람 보고 X)
			if (strCurGlsID.GetLength() > 6) {
				CString strCompare;
				strCompare = strCurGlsID.GetAt(5);
				CString str;
				for (int i = 0; i < EXCEPT_STR_MAX; i++) {
					str = (CString)m_pSystemConfigMgr->GetChrExcept(i);

					if (str == _T("")) continue;
					if (str == strCompare) {
						m_pShareMemory->SetSys_Validation(true);
						break;
					}
				}
			}
			//
			//GlassCount();				// pwk 20130814 



		}
	}

	return nRet;
}


int CCommTcpIpObjectCIM::Process_PPID(CString strMsg)
{
	// TODO: 여기에 구현 코드 추가.
	int nRet = VTM_RV_SUCCESSED;

	//20171207 add by Hee (Aging 자동 검사 대응)
	//PPID^NC3_DM0                       ^A                       $
	int nBeging = 0, nEnd = 0;

	CStringArray strArrPPID;
	strArrPPID.RemoveAll();

	for (int i = 0; i < strMsg.GetLength(); i++) {
		if (strMsg.GetAt(i) == '^' || strMsg.GetAt(i) == '$') {

			if (nBeging == 0) {
				nBeging = i + 1;
			}
			else {
				nEnd = i - nBeging;
				strArrPPID.Add(strMsg.Mid(nBeging, nEnd).Trim());
				nBeging = i + 1;
			}
		}
	}

	CString strPPID, strAging;
	if (strArrPPID.GetSize() == 2) {
		strPPID = strArrPPID.GetAt(0);
		strAging = strArrPPID.GetAt(1);
	}
	else {
		strPPID = strArrPPID.GetAt(0);
	}

	int nRtn = VTM_RV_SUCCESSED;
	nRtn = GetRecipeEditMgrPtr()->OpenRecipe(strPPID);

	m_pShareMemory->SetSys_AutoAgingMode(false);

	if (nRtn == VTM_RV_SUCCESSED && strAging == 'J') { //20210518 modify by Hee (경시변환 측정 모드 J 플래그로 변경 요청)
		//Aging 측정 리스트 없으면 알람									//PPID 와 Aging 플래그 없을시 *알람명은 바꿔야 할수도
	//	if (!m_pDlgNaviManual->m_dlgNaviManualAgingTest.LoadAgingInfo() || strArrPPID.GetSize() != 2) 
		if (strArrPPID.GetSize() != 2)
		{
			nRtn = JOB_RECIPE_NOT_EXIST;
		}
		else {
			m_pShareMemory->SetSys_AutoAgingMode(true);
		//	m_pSystem->nJobCount = 1;
		//	m_pSystem->m_nAgingListRow = 0;
		//	m_pSystem->m_nAgingListCol = 0;
		}
	}

	if (nRet == VTM_RV_SUCCESSED)
	{
		if (m_pSystemConfigMgr->IsUseModelChangeAlarm() == true) {
			//글라스 모델이 바뀌면 알람 CIM 보고 X
			if (m_strGlassModelName == _T("") || m_strGlassModelName == strPPID) {
				m_pShareMemory->SetSys_Validation(false);
				m_strGlassModelName = strPPID;
			}
			else if (m_strGlassModelName != strPPID) {
				m_pShareMemory->SetSys_Validation(true);
				m_strGlassModelName = strPPID;
			}
		}
	}

	if (nRet == VTM_RV_SUCCESSED)
	{
		CSystemParamShare::GetInstance()->SetRecipeID(strPPID);
	}

	return nRet;
}


int CCommTcpIpObjectCIM::Process_Start()
{
	// TODO: 여기에 구현 코드 추가.
	m_bUserBypassGlass = false;

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CHANGE_CIM_STATE, TO_SERVER_PRCS, NULL);

	return 0;
}


int CCommTcpIpObjectCIM::ProcessTo_GDTCPL()
{
	// TODO: 여기에 구현 코드 추가.
	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CHANGE_CIM_STATE, TO_SERVER_GDTREQ, NULL);

	return 0;
}


int CCommTcpIpObjectCIM::ProcessTo_ULDCPL()
{
	// TODO: 여기에 구현 코드 추가.
	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CHANGE_CIM_STATE, TO_SERVER_ULDCPL, NULL);

	//m_pSystem->iMachineStatus = df_STATUS_NONE;

	//m_bUserBypassGlass = false;
	//SetEvent(m_hEvt_CIM_Reply[TO_CIM_ULDCPL]);

	//Cmd_Send(dfULDCPL_OK);

	//m_pSystem->bGlassLoadState = FALSE;
	//WriteGalssExistFile();

	//m_pJobProc->ClearGlassData();

	//// 글라스 받을 준비 상태로 만든다
	//m_pJobProc->DoSetEventStateUnLoad();	// jsyoon 2015427 add Loading 

	//tickGlassMeas.Stop();
	//double dTick = 0.0;
	//dTick = tickGlassMeas.Elapsed();
	//CString strLog;
	//strLog.Format("Glass UnLoad Complete : [ID : %s, LotID : %s", m_pSystem->stLotInfo.m_szGlassID, m_pSystem->stLotInfo.m_szLotID);
	//gLog(m_pSystem->hLog, strLog, LOG_LEVEL_T3);
	//strLog.Format("[%s] 전체 Tact : %02d분 %02d초", m_pSystem->stLotInfo.m_szGlassID, (int)(dTick / 60), (int)((int)dTick % 60));
	//gLog(m_pSystem->hLog, strLog, LOG_LEVEL_T3);

	//Sleep(500);
	//Cmd_Send(dfSTS_00_04);
	//m_pDlgNaviOperate->EnableControlMode(TRUE);

	return 0;
}


int CCommTcpIpObjectCIM::ProcessTo_ULDREQ()
{
	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CHANGE_CIM_STATE, TO_SERVER_ULDREQ, NULL);

	// TODO: 여기에 구현 코드 추가.
	//m_pSystem->m_bMeasMultiRcp = FALSE;		// jsyoon 20150410 add

	//if (CanDoStartPRCS(TRUE, FALSE))	// Only check O2
	//{
	//	Cmd_Send(dfULDREQ_OK);
	//	Sleep(1000);
	//	m_pDlgNaviOperate->DoStartGlassUnLoadReq();
	//}
	//else
	//{
	//	Cmd_Send(dfULDREQ_NG);
	//	gLog(m_pSystem->hLog, "ULDREQ But HandShakeComplete FALSE");
	//	CAlarmController::GetInstance()->SetAlarm(CIM_RV_HANDSHAKE_FAIL);
	//}

	return 0;
}


int CCommTcpIpObjectCIM::ProcessTo_LDREQ()
{
	// TODO: 여기에 구현 코드 추가.
	m_bUserBypassGlass = false;

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CHANGE_CIM_STATE, TO_SERVER_LDREQ, NULL);

//	::PostMessage(this->GetParent()->m_hWnd, WM_MSG_MAINT, (WPARAM)VTM_CIM, (LPARAM)TO_SERVER_LDREQ);

	return 0;
}




int CCommTcpIpObjectCIM::ProcessTo_LDCPL()
{
	// TODO: 여기에 구현 코드 추가.
	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CHANGE_CIM_STATE, TO_SERVER_LDCPL, NULL);
	// 
	//m_pSystem->iMachineStatus = df_STATUS_NONE;
	//Sleep(200);	// jsyoon 20150327 change GLSID
	//Cmd_Send(dfLDCPL_OK);

	//m_pSystem->m_bMeasMultiRcp = FALSE;		// jsyoon 20150410 add

	//Sleep(200);

	//Cmd_Send(dfPPIDREQ);

	//m_bUserBypassGlass = false;
	//m_pDlgNaviOperate->EnableControlMode(TRUE);

	return 0;
}


int CCommTcpIpObjectCIM::ProcessTo_Init()
{
	// TODO: 여기에 구현 코드 추가.
	m_bUserBypassGlass = false;

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CHANGE_CIM_STATE, TO_SERVER_INIT_START, NULL);

	return 0;
}

int CCommTcpIpObjectCIM::ProcessTo_PPIDREQ()
{
	// TODO: 여기에 구현 코드 추가.
	m_bUserBypassGlass = false;

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CHANGE_CIM_STATE, TO_SERVER_PPID, NULL);

	return 0;
}


int CCommTcpIpObjectCIM::ProcessTo_GLSREADY()
{
	// TODO: 여기에 구현 코드 추가.
	//m_bUserBypassGlass = false;
	//Cmd_Send(dfGLSREADY_OK);
	//m_pSystem->m_bMeasMultiRcp = FALSE;			// jsyoon 20150410 add
	//Sleep(500);
	//Cmd_Send(dfPPIDREQ);

	//m_pDlgNaviOperate->EnableControlMode(TRUE);

	return 0;
}

int CCommTcpIpObjectCIM::ProcessTo_MeasStart()
{
	// TODO: 여기에 구현 코드 추가.
	int nRet = VTM_RV_SUCCESSED;
	nRet = CanDoStartPRCS(false, true);
	if (nRet == VTM_RV_SUCCESSED)
	{
		CString strGlsID, strStopGlassID, strLog;
		strGlsID.Format(_T("%s"), CSystemParamShare::GetInstance()->GetGlassID());

		m_pSystemConfigMgr->ReadGalssExistFile();	// Read Stop GlassID

		strStopGlassID.Format(_T("%s"), m_pSystemConfigMgr->GetStopGlassID());
		
		if (strStopGlassID.Compare(strGlsID) == 0)
		{
			strLog.Format(_T("StopGlassID(%s), Question is Measure or Off-line(PM)"), strGlsID);
			m_pLogFileAccessor->KMLOG(SYST, strLog);

			if (AfxMessageBox(_T("레시피 수정을 위해 설비를 정지할까요?"), MB_YESNO | MB_ICONQUESTION) != IDYES)
			{
				CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CHANGE_CIM_STATE, TO_SERVER_MEAS_START, NULL);
			}
			else	// Off-line
			{
				Cmd_Send(dfAT_0);
				nRet = CIM_RV_HANDSHAKE_FAIL;
			}
		}
		else
		{
			if (strStopGlassID.Find(_T("HOLD_USER")) >= 0)
			{
				strLog.Format(_T("StopGlassID(%s), Question is Measure or Off-line(PM)"), strGlsID);
				m_pLogFileAccessor->KMLOG(SYST, strLog);
				if (AfxMessageBox(_T("레시피 수정을 위해 설비를 정지할까요?"), MB_YESNO | MB_ICONQUESTION) != IDYES)
				{
					CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CHANGE_CIM_STATE, TO_SERVER_MEAS_START, NULL);
				}
				else	// Off-line
				{
					Cmd_Send(dfAT_0);
					nRet = CIM_RV_HANDSHAKE_FAIL;
				}
			}
			else
			{
				CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CHANGE_CIM_STATE, TO_SERVER_MEAS_START, NULL);
			}
		}
	}

	return nRet;
}

int CCommTcpIpObjectCIM::ProcessTo_MeasEnd()
{
	int nRet = VTM_RV_SUCCESSED;

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CHANGE_CIM_STATE, TO_SERVER_MEAS_END, NULL);

	return nRet;
}

bool CCommTcpIpObjectCIM::SendRecipeListToFTP()
{
	// TODO: 여기에 구현 코드 추가.
	CString strLog;
	strLog.Format(_T("SendRecipeListToFTP() Start"));
	m_pLogFileAccessor->KMLOG(SYST, strLog);

	CString str;
	str.Format(_T("%s"), m_pSystemConfigMgr->GetUploadCIMFolderPath());

	m_pTransporterResult->SetParam_FTPFileUploadRemotePath(_T(""));
	m_pTransporterResult->SetParam(str);
	m_iTransporterType = 0;
	SetEvent(m_hStartEvent);

	return true;
}

bool CCommTcpIpObjectCIM::SendAPDFileToFTP()
{
	// TODO: 여기에 구현 코드 추가.
	CString strLog;
	strLog.Format(_T("SendAPDFileToFTP() Start"));
	m_pLogFileAccessor->KMLOG(SYST, strLog);

	CString str, strGlassID;
	strGlassID.Format(_T("%s\\"), CSystemParamShare::GetInstance()->GetGlassID());
	str.Format(_T("%s%s"), m_pSystemConfigMgr->GetUploadCIMFolderPath(), strGlassID);

	CString strApdData = _T(""), strApdCimPath = _T("");
	strApdData.Format(_T("%s%s"), m_pSystemConfigMgr->GetResultFilePath(), _T("\\APD.txt"));
	strApdCimPath.Format(_T("%s%s"), str, _T("\\APD.txt"));

	strApdData.Replace(_T("\\\\"), _T("\\"));
	strApdCimPath.Replace(_T("\\\\"), _T("\\"));
	gf_MakeDir(strApdCimPath);

	if (CopyFile(strApdData, strApdCimPath, FALSE) == TRUE)
	{
		m_pTransporterResult->SetParam_FTPFileUploadRemotePath(strGlassID);
		m_pTransporterResult->SetParam(str);
		m_iTransporterType = 1;
		SetEvent(m_hStartEvent);
		return true;
	}
	else
	{
		strLog.Format(_T("SendAPDFileToFTP() Error, CopyFile NG %s to %s"), strApdData, strApdCimPath);
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		return false;
	}
}

UINT CCommTcpIpObjectCIM::ThreadRun(LPVOID p1)
{
	CCommTcpIpObjectCIM* pThis = (CCommTcpIpObjectCIM*)p1;

	while (1)
	{
		// Event 가 set될 때까지 기다린다.
		if (WaitForSingleObject(pThis->m_hStartEvent, 10) == WAIT_OBJECT_0)
		{
			pThis->m_bThreadLive = true;

			pThis->ThreadRun();

			pThis->m_bThreadLive = false; 
		}

		// Close Event 가 set될 때까지 기다린다.
		if (WaitForSingleObject(pThis->m_hCloseEvent, 10) == WAIT_OBJECT_0)
		{
			break;
		}

		Sleep(1);
	}

	pThis->m_bThreadLive = false;

	return 0;
}

UINT CCommTcpIpObjectCIM::ThreadRun()
{
	bool bSendFile = false;
	bSendFile = m_pTransporterResult->Run();

	switch (m_iTransporterType)
	{
	case 0:
		Cmd_Send(dfRCPLIST);
		Sleep(200);

		Cmd_Send(dfALMACL);
		break;
	case 1:
	{
		if (bSendFile)
		{
			Cmd_Send(dfGDTCPL_OK);
		}
		else
		{
			Cmd_Send(dfGDTCPL_NG);
		}
	}
		break;
	default:
		break;
	}

	return 0;
}

void CCommTcpIpObjectCIM::GlassCount()
{
	//// TODO: 여기에 구현 코드 추가.
	//CString strLotId, strGlassId;
	//ReadGalssDataFile();

	//strLotId.Format("%s", m_pSystem->stLotInfo.m_szLotID);
	//strGlassId.Format("%s", m_pSystem->stLotInfo.m_szGlassID);

	//if (m_strGlassID != strGlassId || m_strLotID != strLotId)
	//{
	//	if (m_pSystem->nMeasMode == PGM_MODE_PPA)
	//		m_iCount[PGM_MODE_PPA]++;
	//	else if (m_pSystem->nMeasMode == PGM_MODE_SE)
	//		m_iCount[PGM_MODE_SE]++;
	//	else if (m_pSystem->nMeasMode == PGM_MODE_REVIEW)
	//		m_iCount[PGM_MODE_REVIEW]++;
	//	else if (m_pSystem->nMeasMode == PGM_MODE_RS)
	//		m_iCount[PGM_MODE_RS]++;
	//	else if (m_pSystem->nMeasMode == PGM_MODE_ICP)
	//		m_iCount[PGM_MODE_ICP]++;
	//}

	//CString strLog;
	//int i = 0;

	//SYSTEMTIME stcurTime;								//--> 시간을 가져오기 위한 struct문이다.
	//GetLocalTime(&stcurTime); 							//--> 현 시스템에 대한 시간을 가져온다.
	//if (stcurTime.wDay != m_iResetDate && stcurTime.wHour >= 6 && m_bResetCount == FALSE)
	//{
	//	int iTotalCnt = 0;

	//	for (i = 0; i < PGM_MODE_END; i++)
	//	{
	//		iTotalCnt += m_iCount[i];
	//	}

	//	//		strLog.Format("Total = %d, PPA : %d, SE : %d, REVIEW : %d, RS : %d, ICP : %d", m_iCountPPA+m_iCountSE+m_iCountREVIEW+m_iCountRS+m_iCountICP, m_iCountPPA, m_iCountSE, m_iCountREVIEW, m_iCountRS, m_iCountICP);
	//	strLog.Format("Total = %d, PPA : %d, SE : %d, REVIEW : %d, RS : %d, ICP : %d", iTotalCnt, m_iCount[PGM_MODE_PPA], m_iCount[PGM_MODE_SE], m_iCount[PGM_MODE_REVIEW], m_iCount[PGM_MODE_RS], m_iCount[PGM_MODE_ICP]);

	//	gLog(m_pSystem->hLog, strLog);

	//	m_iResetDate = stcurTime.wDay;
	//	m_bResetCount = TRUE;

	//	for (i = 0; i < PGM_MODE_END; i++)
	//	{
	//		m_iCount[i] = 0;
	//	}

	//	m_iCount[m_pSystem->nMeasMode] = 1;

	//}
	//else if (stcurTime.wDay != m_iResetDate && m_bResetCount == TRUE)
	//	m_bResetCount = FALSE;

	//WriteGalssDataFile();
}

int CCommTcpIpObjectCIM::SendMainStatus(bool bRemote)
{
//	m_pSystem->m_bMeasMultiRcp = FALSE;		// jsyoon 20150410 add
//	m_pSystem->m_bUseGlassAbrot = FALSE;	//ksk 2010612 add

	if(bRemote == true)
	{
		ResetEvent(m_hRecvDEvent[TO_CIM_AT1]);
		ResetEvent(m_hRecvDEvent[TO_CIM_EQPSTART]);
		Cmd_Send(dfAT_1);
	}
	else
	{
		ResetEvent(m_hRecvDEvent[TO_CIM_AT0]);
		Cmd_Send(dfAT_0);
	}

	return 1;
}

bool CCommTcpIpObjectCIM::IsValidReply(int nCmd)
{
	if (nCmd<TO_CIM_STS0104 || nCmd > CIM_CMD_SERVER_TO_CIM_END)
		return false;

	return true;
}

int CCommTcpIpObjectCIM::WaitForCIMResponse_Action(CString strCmd, UINT nAction, UINT nTimeOut)
{
	CString strLog;
	strLog.Format(_T("WaitForCIMResponse_Action, Cmd=%s, nAction=%d, nTimeOut=%d)..."), strCmd, nAction, nTimeOut);
	m_pLogFileAccessor->KMLOG(OPER, strLog);

	if (IsOutOfService() == true)
	{
		return VTM_RV_SUCCESSED;
	}

	if (IsConnected() != true)
	{
		return CIM_RV_SOCKET_CREATE_FAIL;
	}

	if (IsValidReply(nAction) != true)
		return VTM_RV_FAILED;

	DWORD  dwS, dwR, dwT = 0;
	DWORD  dwSleep = 100;

	for (dwS = GetTickCount(); dwT < nTimeOut * 1000;)
	{
		if (WaitForSingleObject(m_hRecvDEvent[nAction], 0)
			== WAIT_OBJECT_0)
		{
			strLog.Format(_T("WaitForCIMResponse_Action, Cmd=%s,nAction=%d, nTimeOut=%d)...Ok"), strCmd, nAction, nTimeOut);
			m_pLogFileAccessor->KMLOG(OPER, strLog);

			ResetEvent(m_hRecvDEvent[nAction]);

			return VTM_RV_SUCCESSED;
		}


		Sleep(10);
		dwR = GetTickCount();

		if (dwR >= dwS)
			dwT = dwR - dwS;
		else
			dwT = ((DWORD)0xFFFFFFFF) - (dwS - dwR);
	}

	strLog.Format(_T("WaitForCIMResponse_Action, Cmd=%s, nAction=%d, nTimeOut=%d)...Timeout"), strCmd, nAction, nTimeOut);
	m_pLogFileAccessor->KMLOG(WARN, strLog);

	return CIM_RV_TIMEOUT;
}



int CCommTcpIpObjectCIM::Cmd_Send_AT0()
{
	ResetEvent(m_hRecvDEvent[TO_CIM_AT0]);

	int nRet = VTM_RV_SUCCESSED;
	nRet = Cmd_Send(dfAT_0);

	m_bProcessHandOn = false;	// jsyoon 2022.07.22 add

	return nRet;
}

int CCommTcpIpObjectCIM::Cmd_Send_STS0104()
{
	ResetEvent(m_hRecvDEvent[TO_CIM_STS0104]);

	int nRet = VTM_RV_SUCCESSED;
	nRet = Cmd_Send(dfSTS_01_04);

	if (nRet == VTM_RV_SUCCESSED)
	{
		nRet = WaitForCIMResponse_Action(dfSTS_01_04, TO_CIM_STS0104, df_WAIT_TIMEOUT);
	}

	return nRet;
}


int CCommTcpIpObjectCIM::Cmd_Send_STS0204()
{
	// TODO: 여기에 구현 코드 추가.
	ResetEvent(m_hRecvDEvent[TO_CIM_STS0204]);
	ResetEvent(m_hRecvDEvent[TO_CIM_ORG]);

	int nRet = VTM_RV_SUCCESSED;
	nRet = Cmd_Send(dfSTS_02_04);

	if (nRet == VTM_RV_SUCCESSED)
	{
		nRet = WaitForCIMResponse_Action(dfSTS_02_04, TO_CIM_STS0204, df_WAIT_TIMEOUT);
	}

	return nRet;
}


int CCommTcpIpObjectCIM::Cmd_Send_STS0304()
{
	// TODO: 여기에 구현 코드 추가.
	ResetEvent(m_hRecvDEvent[TO_CIM_STS0304]);

	int nRet = VTM_RV_SUCCESSED;
	nRet = Cmd_Send(dfSTS_03_04);

	if (nRet == VTM_RV_SUCCESSED)
	{
		nRet = WaitForCIMResponse_Action(dfSTS_03_04, TO_CIM_STS0304, df_WAIT_TIMEOUT);
	}

	return nRet;
}


int CCommTcpIpObjectCIM::Cmd_Send_STS0004()
{
	// TODO: 여기에 구현 코드 추가.
	if (CanDoStartPRCS(true, false) != VTM_RV_SUCCESSED)	// Only check O2
	{
		return CIM_RV_HANDSHAKE_FAIL;
	}

	ResetEvent(m_hRecvDEvent[TO_CIM_STS0004]);

	int nRet = VTM_RV_SUCCESSED;
	nRet = Cmd_Send(dfSTS_00_04);

	if (nRet == VTM_RV_SUCCESSED)
	{
		nRet = WaitForCIMResponse_Action(dfSTS_00_04, TO_CIM_STS0004, df_WAIT_TIMEOUT);
	}

	return nRet;
}


int CCommTcpIpObjectCIM::Cmd_Send_STS0506()
{
	// TODO: 여기에 구현 코드 추가.
	ResetEvent(m_hRecvDEvent[TO_CIM_STS0506]);

	int nRet = VTM_RV_SUCCESSED;
	nRet = Cmd_Send(dfSTS_05_06);

	/*if (nRet == VTM_RV_SUCCESSED)
	{
		nRet = WaitForCIMResponse_Action(dfSTS_05_06, TO_CIM_STS0506, df_WAIT_TIMEOUT);
	}*/

	return nRet;
}

// Measure State
int CCommTcpIpObjectCIM::Cmd_Send_STS1005()
{
	// TODO: 여기에 구현 코드 추가.
	ResetEvent(m_hRecvDEvent[TO_CIM_STS1005]);

	int nRet = VTM_RV_SUCCESSED;
	nRet = Cmd_Send(dfSTS_10_05);

	if (nRet == VTM_RV_SUCCESSED)
	{
		nRet = WaitForCIMResponse_Action(dfSTS_10_05, TO_CIM_STS1005, df_WAIT_TIMEOUT);
	}

	return nRet;
}

// Measure Start
int CCommTcpIpObjectCIM::Cmd_Send_STS1006()
{
	// TODO: 여기에 구현 코드 추가.
	ResetEvent(m_hRecvDEvent[TO_CIM_STS1006]);

	int nRet = VTM_RV_SUCCESSED;
	nRet = Cmd_Send(dfSTS_10_06);

	if (nRet == VTM_RV_SUCCESSED)
	{
		nRet = WaitForCIMResponse_Action(dfSTS_10_06, TO_CIM_STS1006, df_WAIT_TIMEOUT);
	}

	return nRet;
}

int CCommTcpIpObjectCIM::Cmd_Send_STS1106()
{
	// TODO: 여기에 구현 코드 추가.
	ResetEvent(m_hRecvDEvent[TO_CIM_STS1106]);

	int nRet = VTM_RV_SUCCESSED;
	nRet = Cmd_Send(dfSTS_11_06);

	if (nRet == VTM_RV_SUCCESSED)
	{
		nRet = WaitForCIMResponse_Action(dfSTS_11_06, TO_CIM_STS1106, df_WAIT_TIMEOUT);
	}

	return nRet;
}

int CCommTcpIpObjectCIM::Cmd_Send_STS2105()
{
	// TODO: 여기에 구현 코드 추가.
	ResetEvent(m_hRecvDEvent[TO_CIM_STS2105]);

	int nRet = VTM_RV_SUCCESSED;
	nRet = Cmd_Send(dfSTS_21_05);

	return nRet;
}

int CCommTcpIpObjectCIM::Cmd_Send_STS2106()
{
	// TODO: 여기에 구현 코드 추가.
	ResetEvent(m_hRecvDEvent[TO_CIM_STS2106]);

	int nRet = VTM_RV_SUCCESSED;
	nRet = Cmd_Send(dfSTS_21_06);

	if (nRet == VTM_RV_SUCCESSED)
	{
		nRet = WaitForCIMResponse_Action(dfSTS_21_06, TO_CIM_STS2106, df_WAIT_TIMEOUT);
	}

	return nRet;
}

int CCommTcpIpObjectCIM::Cmd_Send_STS2107()
{
	// TODO: 여기에 구현 코드 추가.
	if (CanDoStartPRCS(true, false) != VTM_RV_SUCCESSED)	// Only check O2
	{
		return CIM_RV_HANDSHAKE_FAIL;
	}

	ResetEvent(m_hRecvDEvent[TO_CIM_STS2107]);

	int nRet = VTM_RV_SUCCESSED;
	nRet = Cmd_Send(dfSTS_21_07);

	if (nRet == VTM_RV_SUCCESSED)
	{
		nRet = WaitForCIMResponse_Action(dfSTS_21_07, TO_CIM_STS2107, df_WAIT_TIMEOUT);
	}

	return nRet;
}

int CCommTcpIpObjectCIM::Cmd_Send_GDTREQ()
{
	// TODO: 여기에 구현 코드 추가.
	int nRet = VTM_RV_SUCCESSED;
	nRet = Cmd_Send(dfGDTREQ_OK);

	return nRet;
}

int CCommTcpIpObjectCIM::Cmd_Send_GDTCPL(bool bOk)
{
	// TODO: 여기에 구현 코드 추가.
	int nRet = VTM_RV_SUCCESSED;

	if (bOk)
	{
		nRet = Cmd_Send(dfGDTCPL_OK);
	}
	else
	{
		nRet = Cmd_Send(dfGDTCPL_NG);
	}

	return nRet;
}


int CCommTcpIpObjectCIM::Cmd_Send_LDREQ(bool bOk)
{
	// TODO: 여기에 구현 코드 추가.
	int nRet = VTM_RV_SUCCESSED;

	if (bOk)
	{
		nRet = Cmd_Send(dfLDREQ_OK);
	}
	else
	{
		nRet = Cmd_Send(dfLDREQ_NG);
	}

	return nRet;
}

int CCommTcpIpObjectCIM::Cmd_Send_LDCPL(bool bOk)
{
	// TODO: 여기에 구현 코드 추가.
	int nRet = VTM_RV_SUCCESSED;

	if (bOk)
	{
		nRet = Cmd_Send(dfLDCPL_OK);


	}
	else
	{
		nRet = Cmd_Send(dfLDCPL_NG);
	}

	return nRet;
}

int CCommTcpIpObjectCIM::Cmd_Send_ULDREQ(bool bOk)
{
	// TODO: 여기에 구현 코드 추가.
	int nRet = VTM_RV_SUCCESSED;

	if (bOk)
	{
		nRet = Cmd_Send(dfULDREQ_OK);
	}
	else
	{
		nRet = Cmd_Send(dfULDREQ_NG);
	}

	return nRet;
}

int CCommTcpIpObjectCIM::Cmd_Send_ULDCPL(bool bOk)
{
	// TODO: 여기에 구현 코드 추가.
	int nRet = VTM_RV_SUCCESSED;

	if (bOk)
	{
		nRet = Cmd_Send(dfULDCPL_OK);

		m_pSystemConfigMgr->WriteGalssExistFile(false);

		//m_pJobProc->ClearGlassData();

		CString strLog;
		strLog.Format(_T("Glass UnLoad Complete :LotID : %s,  GlassID : %s"), CSystemParamShare::GetInstance()->GetLotID(), CSystemParamShare::GetInstance()->GetGlassID());
		if (GetSeqTactTimeMgrPtr() != nullptr ) GetSeqTactTimeMgrPtr()->SetLogFile(strLog);

		double dTick = 0.0;
		dTick = tickGlassMeas.Elapsed();

		strLog.Format(_T("[%s] 전체 Tact : %02d분 %02d초"), CSystemParamShare::GetInstance()->GetGlassID(), (int)(dTick / 60), (int)((int)dTick % 60));
		if (GetSeqTactTimeMgrPtr() != nullptr) GetSeqTactTimeMgrPtr()->SetLogFile(strLog);
	}
	else
	{
		nRet = Cmd_Send(dfULDCPL_NG);
	}

	return nRet;
}

int CCommTcpIpObjectCIM::Cmd_Send_PRCS(bool bOk)
{
	// TODO: 여기에 구현 코드 추가.
	int nRet = VTM_RV_SUCCESSED;

	if (bOk)
	{
		nRet = Cmd_Send(dfPRCS_OK);
	}
	else
	{
		nRet = Cmd_Send(dfPRCS_NG);
	}

	return nRet;
}

int CCommTcpIpObjectCIM::Cmd_Send_PPID(bool bOk)
{
	// TODO: 여기에 구현 코드 추가.
	int nRet = VTM_RV_SUCCESSED;

	if (bOk)
	{
		nRet = Cmd_Send(dfPPIDREQ_OK);
	}
	else
	{
		nRet = Cmd_Send(dfPPIDREQ_NG);
	}

	return nRet;
}

int CCommTcpIpObjectCIM::Cmd_Send_DataJudge()
{
	// TODO: 여기에 구현 코드 추가.
	int nRet = VTM_RV_SUCCESSED;

	int iJudge = 0;

	if (m_bUserBypassGlass != true)
	{
		iJudge = 1;
		/*if (m_pShareMemory->GetSys_GlassJudge() == true)
		{
			iJudge = 1;
		}
		else
		{
			iJudge = 2;
		}*/
	}
	else
	{
		iJudge = 0;
	}

	
	switch (iJudge)
	{
	case 0:
		nRet = Cmd_Send(dfSTS_DATAJUDGE_NOTMEAS);
		break;
	case 1:
		nRet = Cmd_Send(dfSTS_DATAJUDGE_OK);
		break;
	case 2:
		nRet = Cmd_Send(dfSTS_DATAJUDGE_SPCOUT);
		break;
	default:
		break;
	}

	return nRet;
}


int CCommTcpIpObjectCIM::Cmd_Send_PPIDREQ()
{
	int nRet = VTM_RV_SUCCESSED;
	ResetEvent(m_hRecvDEvent[TO_CIM_PPID]);
	nRet = Cmd_Send(dfPPIDREQ);

	if (nRet == VTM_RV_SUCCESSED)
	{
		nRet = WaitForCIMResponse_Action(dfPPIDREQ, TO_CIM_PPID, df_WAIT_TIMEOUT);
	}

	return nRet;
}


int CCommTcpIpObjectCIM::Cmd_Send_STS2004()
{
	int nRet = VTM_RV_SUCCESSED;
	ResetEvent(m_hRecvDEvent[TO_CIM_STS2004]);
	nRet = Cmd_Send(dfSTS_20_04);

	if (nRet == VTM_RV_SUCCESSED)
	{
		nRet = WaitForCIMResponse_Action(dfSTS_20_04, TO_CIM_STS2004, df_WAIT_TIMEOUT);
	}

	return nRet;
}

int CCommTcpIpObjectCIM::Cmd_Send_STS0006()
{
	int nRet = VTM_RV_SUCCESSED;
	nRet = Cmd_Send(dfSTS_00_06);

	//if (nRet == VTM_RV_SUCCESSED)
	//{
	//	nRet = WaitForCIMResponse_Action(dfSTS_00_06, TO_CIM_STS0006, df_WAIT_TIMEOUT);
	//}

	return nRet;
}

int CCommTcpIpObjectCIM::Cmd_Send_LOTIDREQ()
{
	int nRet = VTM_RV_SUCCESSED;
	ResetEvent(m_hRecvDEvent[TO_CIM_LOTID]);
	nRet = Cmd_Send(dfLOTREQ);

	if (nRet == VTM_RV_SUCCESSED)
	{
		nRet = WaitForCIMResponse_Action(dfLOTREQ, TO_CIM_LOTID, df_WAIT_TIMEOUT);
	}

	return nRet;
}

int CCommTcpIpObjectCIM::Cmd_Send_GLSIDREQ()
{
	ResetEvent(m_hRecvDEvent[TO_CIM_GLSID]);
	int nRet = VTM_RV_SUCCESSED;
	nRet = Cmd_Send(dfGLSREQ);

	if (nRet == VTM_RV_SUCCESSED)
	{
		nRet = WaitForCIMResponse_Action(dfGLSREQ, TO_CIM_GLSID, df_WAIT_TIMEOUT);
	}

	return nRet;
}


void CCommTcpIpObjectCIM::SetUserBypassGlass(bool bPass)
{
	// TODO: 여기에 구현 코드 추가.
	m_bUserBypassGlass = bPass;
}


int CCommTcpIpObjectCIM::WaitForORG()
{
	// TODO: 여기에 구현 코드 추가.
	return WaitForCIMResponse_Action(dfSTS_ORG, TO_CIM_ORG, df_WAIT_TIMEOUT);
}


int CCommTcpIpObjectCIM::CanDoStartPRCS(bool bCheckLoad, bool bCheckMeas)
{
	// TODO: 여기에 구현 코드 추가.
	int nRet = VTM_RV_SUCCESSED;

	CString strLog;

	if (m_pShareMemory->GetSys_MachineMode() == df_HOST_OFFLINE)
	{
		strLog.Format(_T("CanDoStartPRCS, Offline!!"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		return VTM_RV_FAILED;
	}

	//jhseo 20140508
	if (m_bProcessHandOn == true)
	{
		strLog.Format(_T("CanDoStartPRCS, HandOn is true!!"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		return CIM_RV_HANDSHAKE_FAIL;
	}
	//////////////////////////////////////////////////////////////////////////

	// jsyoon 20170307 add
	if (m_pShareMemory->GetSys_AlarmStatus() != 0)
	{
		strLog.Format(_T("CanDoStartPRCS, Alarm Status return FALSE!!"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		return VTM_RV_FAILED;
	}
	//////////////////////////////////////////////////////////////////////////

	if (bCheckMeas == true)
	{
		int nBit = 0;
		int iLoop = 0;
		nRet = PLC_LOADER_SHUTTER_UP_ERROR;
		do
		{
			nBit = m_pMaintControlMgr->GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(PLC_BIT_SUTTER_SENSOR_CLOSE);
			if (nBit != -1)
			{
				if (nBit == 1)
				{
					nRet = VTM_RV_SUCCESSED;
					break;
				}
			}

			Sleep(1000);
			iLoop++;

		} while (iLoop < 50);

		if (nRet != VTM_RV_SUCCESSED)
		{
			strLog.Format(_T("CanDoStartPRCS, Shutter Open FALSE!!"));
			m_pLogFileAccessor->KMLOG(WARN, strLog);
		}
	}
	


//	if (bCheckLoad)
//	{
//		//ksk 20150506 remove
//		// jsyoon 20150409 add
//		// O2 check
//// 		if( !m_pSystem->stInterLock.bSensorO2Low )
//// 		{
//// 			strLog.Format("CDlgNaviMaintCIM::CanDoStartPRCS, O2(%.2f) is Over!!", m_pSystem->stInterLock.nSensorO2PPM);
//// 			gLog(m_pSystem->hLog, strLog, LOG_LEVEL_E);
//// 			//		EventLog(strLog);
//// 			if( !m_checkInterlockO2.GetCheck())
//// 				return FALSE;
//// 		}
//// 		
//// 		// H2O check
//// 		if( !m_pSystem->stInterLock.bSensorH2OLow )
//// 		{
//// 			strLog.Format("CDlgNaviMaintCIM::CanDoStartPRCS, H2O(%.2f) is Over!!", m_pSystem->stInterLock.nSensorH2O);
//// 			gLog(m_pSystem->hLog, strLog, LOG_LEVEL_E);
//// 			//		EventLog(strLog);	
//// 			if( !m_checkInterlockH2O.GetCheck())
//// 				return FALSE;
//// 		}
//	}

	return nRet;
}

int CCommTcpIpObjectCIM::ProcessPPID()
{
	int nRet = VTM_RV_SUCCESSED;

	nRet = Process_PPID(m_strRcvPPID);
	return nRet;
}

int CCommTcpIpObjectCIM::ProcessLOTID()
{
	int nRet = VTM_RV_SUCCESSED;

	nRet = Process_LOTID(m_strRcvLotID);
	return nRet;
}

int CCommTcpIpObjectCIM::ProcessGLSID()
{
	int nRet = VTM_RV_SUCCESSED;

	nRet = Process_GLSID(m_strRcvGlsID);
	return nRet;
}

int CCommTcpIpObjectCIM::SendAlarmCode(int nCode)
{
	int nRet = VTM_RV_SUCCESSED;
	CString strAlm;
	strAlm.Format(_T("ALM%d$"), nCode);
	nRet = Cmd_Send(strAlm);

	return nRet;
}

int CCommTcpIpObjectCIM::SendAlarmClear(int nCode)
{
	int nRet = VTM_RV_SUCCESSED;
	CString strAlm;
	strAlm.Format(_T("ALMCLR%d$"), nCode);
	nRet = Cmd_Send(strAlm);

	return nRet;
}

int CCommTcpIpObjectCIM::SendWarningCode(int nCode)
{
	int nRet = VTM_RV_SUCCESSED;
	CString strAlm;
	strAlm.Format(_T("WRN%d$"), nCode);
	nRet = Cmd_Send(strAlm);

	return nRet;
}

int CCommTcpIpObjectCIM::SendWarningClear(int nCode)
{
	int nRet = VTM_RV_SUCCESSED;
	CString strAlm;
	strAlm.Format(_T("WRNCLR%d$"), nCode);
	nRet = Cmd_Send(strAlm);

	return nRet;
}

bool CCommTcpIpObjectCIM::Send_Data()
{
	bool bRet = false;

	if(m_pTransporterResult != nullptr)
		bRet = m_pTransporterResult->GetParam_UseFTPFileUpload();

	return bRet;
}