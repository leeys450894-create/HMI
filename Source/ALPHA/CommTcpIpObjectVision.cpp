#include "pch.h"
#include "CommTcpIpObjectVision.h"

#include "Manager\NetworkConfigMgr.h"
#include "Manager\SystemConfigMgr.h"
#include "Manager\MaintControlMgr.h"
#include "Recipe\RecipeEditMgr.h"
#include "SystemParamShare.h"
#include "Alarm\AlarmController.h"
#include "AuroraFuncDefine.h"
#include "Manager\SequenceManager.h"
#include "Manager/ResultMgr.h"
#include "CommEthFunction\EthernetManagerTCP.h"
#include "common\CommonFunc.h"
#include "Manager/CommTcpIpFactoryMgr.h"
#include "Manager/CCommTcpIpObjectMotion.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCommTcpIpObjectVision::CCommTcpIpObjectVision(int iSeqItemIndex, stAuroraMgrPool& MgrPool)
	: m_pSystemConfigMgr(MgrPool.pSystemConfigMgr)
	, m_pRecipeEditMgr(MgrPool.pRecipeEditMgr)
	, m_pResultCollectMgr(MgrPool.pResultCollectMgr)
	, m_pShareMemory(MgrPool.pShareMemoryMgr)
	, m_pMaintControlMgr(MgrPool.pMaintControlMgr)
	, m_pLogFileAccessor(new CLogFileAccessor)
	, m_bOutOfService(false)
	, m_iAlarmIndex(0)
	, m_iPattern(0)
	, m_pCommTcpIpFactoryMgr(MgrPool.pCommTcpIpFactoryMgr)
{
	m_strQueueMessage.Empty();
	m_pRecipeDataClone = nullptr;

	int iDeviceIndex = MgrPool.pNetworkConfigMgr->GetParam_DeviceIndex(iSeqItemIndex);
	m_iDeviceIndex = iDeviceIndex;

	m_iAlarmIndex = enumTCPIpFactorytID_VISION + iDeviceIndex;

	CString strObjectName;
	strObjectName.Format(_T("%s%d"), _T("화질"), iDeviceIndex + 1);

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

	//Initialize();

	//m_pRecipeDataClone = new CRecipeData;
	//m_pRecipeDataClone->Clone(m_pRecipeEditMgr->GetRecipeDataPtr());

	// 데이터 수신 확인용 event
	for (int i = 0; i < enumRecvEventIDCount; ++i)
	{
		m_hRecvDEvent[i] = CreateEvent(NULL, FALSE, FALSE, NULL);	// auto reset, initial reset
	}

	//
	m_nHead = iSeqItemIndex;
	m_bQualityWait = FALSE;
	m_pWaitThread = NULL;
	//m_bRunThread = FALSE;

	m_strNetDrive = MgrPool.pNetworkConfigMgr->GetParam_NetDrive(iSeqItemIndex);

	m_strNetRcpDrive = MgrPool.pNetworkConfigMgr->GetParam_NetRcpDrive(iSeqItemIndex);

	m_bRunThread = TRUE;
	m_pWaitThread = AfxBeginThread(QualityWaitThread, this);
	//
}


CCommTcpIpObjectVision::~CCommTcpIpObjectVision()
{
	if (m_bRunThread) {
		m_bRunThread = FALSE;
		while (m_pWaitThread) {
			Sleep(0);
		}
	}

	Finalize();

	if (m_pRecipeDataClone)
	{
		delete m_pRecipeDataClone;
		m_pRecipeDataClone = nullptr;
	}

	for (int i = 0; i < enumRecvEventIDCount; ++i)
	{
		if (m_hRecvDEvent[i])
		{
			CloseHandle(m_hRecvDEvent[i]);
		}
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

void CCommTcpIpObjectVision::UpdateRecipeInformation()
{
	m_pRecipeDataClone->Clone(m_pRecipeEditMgr->GetRecipeDataPtr());
}

bool CCommTcpIpObjectVision::SetTime_Sync()
{
	if (IsConnected() == false)
		return true;

	ResetEvent(m_hRecvDEvent[enumRecvEventID_TIME_SYNC_OK]);
	ResetEvent(m_hRecvDEvent[enumRecvEventID_TIME_SYNC_NG]);

	SYSTEMTIME st;
	::GetLocalTime(&st);
	CString sTime;
	sTime.Format(_T("%04d%02d%02d%02d%02d%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	CString m_strRecipeName;
	CString strReturnCommand = GetStringCMDID(COMM_USER_DEFINE_REQ) + _T("^TIME_SYNC^") + sTime;

	m_pLogFileAccessor->KMLOG(SEND, strReturnCommand);

	if (SendPacket(strReturnCommand) == true)
	{
		HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_TIME_SYNC_OK], m_hRecvDEvent[enumRecvEventID_TIME_SYNC_NG] };

		DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, DEFINE_PARAMETERS_WAIT_TIME);
		if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
		{
			//CAlarmController::GetInstance()->SetAlarm(SDC_DEF_MODULE_SOCKET_TIME_OUT);
		}

		return true;
	}

	return false;
}

bool CCommTcpIpObjectVision::RecipeChange()
{
	int iRet = VTM_RV_SUCCESSED;
	CString strLog;

	if (IsOutOfService() == true)
		return true;

	// File copy "Changerecipe.ini"
	iRet = WriteRecipeFile(); //CopyRecipeFile();
	if (iRet != VTM_RV_SUCCESSED)
	{
		strLog.Format(_T("[%s] : %s"), GetName(), _T("Change RECIPE FILE FAIL"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		CAlarmController::GetInstance()->SetAlarm(QUALITY_MODULE_CHANGE_RCP_ERROR);
		return false;
	}

	CString strReturnCommand;
	strReturnCommand.Format(_T("MOT.RECIPE.CHANGE"));

	strLog.Format(_T("[%s] : %s"), GetName(), strReturnCommand);
	m_pLogFileAccessor->KMLOG(SEND, strLog);

	ResetEvent(m_hRecvDEvent[enumRecvEventID_RECIPE_CHANGE_OK]);
	ResetEvent(m_hRecvDEvent[enumRecvEventID_RECIPE_CHANGE_NG]);

	if (IsOutOfService() == true)
		return true;

	if (SendPacket(strReturnCommand) == true)
	{
		//HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_RECIPE_CHANGE_OK], m_hRecvDEvent[enumRecvEventID_RECIPE_CHANGE_NG] };

		//DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, DEFINE_PARAMETERS_WAIT_TIME * 10);
		//if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
		//{
		//	strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_TIMEOUT"));
		//	m_pLogFileAccessor->KMLOG(WARN, strLog);
		//	CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iAlarmIndex);
		//	return false;
		//}
		//else if (dw == (WAIT_OBJECT_0 + 1)) // NG
		//{
		//	strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_FAIL"));
		//	m_pLogFileAccessor->KMLOG(WARN, strLog);
		//	CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_FAIL + m_iAlarmIndex);
		//	return false;
		//}
		//else
		{

			if (m_iAlarmIndex == 1)
			{
				CString str;
				str.Format(_T("%s"), m_pRecipeEditMgr->GetRecipeID());

				int nRet = VTM_RV_SUCCESSED;
				VisMotionPROTOCOL stPrl;
				ZeroMemory(&stPrl, sizeof(stPrl));

				//// Homing
				//stPrl.iCmd = enumTCPIpFactoryt_VisMotion_1;
				//nRet = m_pCommTcpIpFactoryMgr->GetCommTcpIpObjectVisMotion()->SetCommand(stPrl);

				//if (nRet == VTM_RV_SUCCESSED)
				{
				//	Sleep(1000);

					// Recipe Change
					stPrl.iCmd = enumTCPIpFactoryt_VisMotion_0;
					_tcscpy_s(stPrl.szModel, _countof(stPrl.szModel), str);
					nRet = m_pCommTcpIpFactoryMgr->GetCommTcpIpObjectVisMotion()->SetCommand(stPrl);
				}
			}

			return true;
		}
	}

	strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_NOT_CONNECTED"));
	m_pLogFileAccessor->KMLOG(WARN, strLog);
	CAlarmController::GetInstance()->SetAlarm(HEAD_NOT_CONNECTED + m_iAlarmIndex);
	return false;
}

bool CCommTcpIpObjectVision::SetParameter()
{
	//if (m_iAlarmIndex == 0)
	if( m_iDeviceIndex == 0 )	// jsyoon 2022.10.14 change
	{
		//CString str;
		//str.Format(_T("%s"), m_pRecipeEditMgr->GetRecipeID());

		int nRet = VTM_RV_SUCCESSED;
		VisMotionPROTOCOL stPrl;
		ZeroMemory(&stPrl, sizeof(stPrl));

		// Homing
		stPrl.iCmd = enumTCPIpFactoryt_VisMotion_1;
		nRet = m_pCommTcpIpFactoryMgr->GetCommTcpIpObjectVisMotion()->SetCommand(stPrl);

		//if (nRet == VTM_RV_SUCCESSED)
		//{
		//	// Recipe Change
		//	stPrl.iCmd = enumTCPIpFactoryt_VisMotion_0;
		//	_tcscpy_s(stPrl.szModel, _countof(stPrl.szModel), str);
		//	nRet = m_pCommTcpIpFactoryMgr->GetCommTcpIpObjectVisMotion()->SetCommand(stPrl);
		//}
	}

#ifdef _USE_NOT
	CString strCellID, strInnerID;
	strCellID = CSystemParamShare::GetInstance()->GetCellID();
	strInnerID = CSystemParamShare::GetInstance()->GetInnerID();

	CString strReturnCommand;
	strReturnCommand.Format(_T("%s^%s^%s^%s"), GetStringCMDID(COMM_USER_DEFINE_REQ), _T("INSP_PARAM"), strCellID, strInnerID);

	CString strLog;
	strLog.Format(_T("[%s] : %s"), GetName(), strReturnCommand);
	m_pLogFileAccessor->KMLOG(SEND, strLog);

	ResetEvent(m_hRecvDEvent[enumRecvEventID_SET_PARAMETER_OK]);
	ResetEvent(m_hRecvDEvent[enumRecvEventID_SET_PARAMETER_NG]);

	if (IsOutOfService() == true)
		return true;

	if (SendPacket(strReturnCommand) == true)
	{
		HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_SET_PARAMETER_OK], m_hRecvDEvent[enumRecvEventID_SET_PARAMETER_NG] };

		DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, DEFINE_RESPONSE_WAIT_TIME);
		if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
		{
			strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_TIMEOUT"));
			m_pLogFileAccessor->KMLOG(WARN, strLog);
			CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iSeqItemIndex);
			return false;
		}

		return true;
	}

	strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_NOT_CONNECTED"));
	m_pLogFileAccessor->KMLOG(WARN, strLog);
	CAlarmController::GetInstance()->SetAlarm(HEAD_NOT_CONNECTED + m_iSeqItemIndex);

	return false;
#endif // _USE_NOT

	return true;
}

bool CCommTcpIpObjectVision::InspectStart(int iMeasType, bool bWait, int iStepNum)
{
	CString strReturnCommand;

	CString str, strCellID, strGlassID, strInnerID, strMcrID;

	strInnerID.Format(_T("%s"), m_pShareMemory->GetSharedMemory()->stLotInfo.m_szInnerID[iMeasType]);
	if (strInnerID.IsEmpty())
	{
		strInnerID.Format(_T("%d%s"), 1 + iMeasType, gf_GetCurTimeString(3));
		_tcscpy_s(m_pShareMemory->GetSharedMemory()->stLotInfo.m_szInnerID[iMeasType], 32, strInnerID);
	}

	strMcrID.Format(_T("%s"), m_pShareMemory->GetSharedMemory()->stLotInfo.m_szCellID[iMeasType]);

	strReturnCommand.Format(_T("MOT.GRAB.START.%s.%s"), strInnerID, strMcrID);

	ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_START_OK]);
	ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_START_NG]);
	ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_END_OK]);
	ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_END_NG]);
	ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_OK]);
	ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_NG]);

	CString strLog;
	if (IsOutOfService() == true)
	{
		strLog.Format(_T("[%s] : %s, OutOfService"), GetName(), strReturnCommand);
		m_pLogFileAccessor->KMLOG(SEND, strLog);

		return true;
	}

	if (SendPacket(strReturnCommand) == true)
	{
		strLog.Format(_T("[%s] : %s"), GetName(), strReturnCommand);
		m_pLogFileAccessor->KMLOG(SEND, strLog);

		if (bWait)
		{
			HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_INSP_START_OK], m_hRecvDEvent[enumRecvEventID_INSP_START_NG] };

			DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, DEFINE_INSPECITON_WAIT_TIME);
			if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
			{
				strLog.Format(_T("[%s] : InspectStart, %s"), GetName(), _T("HEAD_ACK_TIMEOUT"));
				m_pLogFileAccessor->KMLOG(WARN, strLog);
				CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iAlarmIndex);
				return false;
			}
			// jsyoon 2022.08.30 add
			else if (dw == (WAIT_OBJECT_0 + 1)) // NG
			{
				strLog.Format(_T("[%s] : InspectStart, %s"), GetName(), _T("HEAD_ACK_FAIL"));
				m_pLogFileAccessor->KMLOG(WARN, strLog);
				CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_FAIL + m_iAlarmIndex);
				return false;
			}
		}

		return true;
	}

	strLog.Format(_T("[%s] : InspectStart, %s"), GetName(), _T("HEAD_NOT_CONNECTED"));
	m_pLogFileAccessor->KMLOG(WARN, strLog);
	CAlarmController::GetInstance()->SetAlarm(HEAD_NOT_CONNECTED + m_iAlarmIndex);
	return false;
}

bool CCommTcpIpObjectVision::InspectStartMulti(int iMeasType, int nArea)
{
	CString strReturnCommand;

	CString str, strCellID, strGlassID, strInnerID, strMcrID;

	strInnerID.Format(_T("%s"), m_pShareMemory->GetSharedMemory()->stLotInfo.m_szInnerID[iMeasType]);
	if (strInnerID.IsEmpty())
	{
		strInnerID.Format(_T("%d%s"), 1 + iMeasType, gf_GetCurTimeString(3));
		_tcscpy_s(m_pShareMemory->GetSharedMemory()->stLotInfo.m_szInnerID[iMeasType], 32, strInnerID);
	}

	strMcrID.Format(_T("%s"), m_pShareMemory->GetSharedMemory()->stLotInfo.m_szCellID[iMeasType]);

	strReturnCommand.Format(_T("MOT.GRAB.START.%s.%s"), strInnerID, strMcrID);

	CString strLog;
	

	ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_START_OK]);
	ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_START_NG]);
	ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_END_OK]);
	ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_END_NG]);
	//ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_OK]);
	//ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_NG]);

	QualityWaitSet(strInnerID, nArea, strMcrID);

	if (IsOutOfService() == true)
	{
		strLog.Format(_T("[%s] : %s, OutOfService"), GetName(), strReturnCommand);
		m_pLogFileAccessor->KMLOG(SEND, strLog);

		return true;
	}
	else
	{
		strLog.Format(_T("[%s] : %s"), GetName(), strReturnCommand);
		m_pLogFileAccessor->KMLOG(SEND, strLog);
	}

	if (SendPacket(strReturnCommand) == true)
	{
		return true;
	}

	strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_NOT_CONNECTED"));
	m_pLogFileAccessor->KMLOG(WARN, strLog);
	CAlarmController::GetInstance()->SetAlarm(HEAD_NOT_CONNECTED + m_iAlarmIndex);
	return false;
}

bool CCommTcpIpObjectVision::InspectStartWait()
{
	if (IsOutOfService() == true)
	{
		Sleep(100);
		return true;
	}

	HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_INSP_START_OK], m_hRecvDEvent[enumRecvEventID_INSP_START_NG] };

	CString strLog;
	DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, DEFINE_INSPECITON_WAIT_TIME);
	if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
	{
		strLog.Format(_T("[%s] : InspectStartWait, %s"), GetName(), _T("HEAD_ACK_TIMEOUT"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iAlarmIndex);
		return false;
	}
	// jsyoon 2022.01.17 add
	else if (dw == (WAIT_OBJECT_0 + 1)) // NG
	{
		strLog.Format(_T("[%s] : InspectStartWait, %s"), GetName(), _T("HEAD_ACK_FAIL"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_FAIL + m_iAlarmIndex);
		return false;
	}
	else
	{
		return true;
	}

	return true;
}

bool CCommTcpIpObjectVision::InspectEnd()
{
#ifdef _USE_NOT
	CString strReturnCommand;
	strReturnCommand.Format(_T("%s^%s"), GetStringCMDID(COMM_USER_DEFINE_REQ), _T("INSP_END"));

	CString strLog;
	strLog.Format(_T("[%s] : %s"), GetName(), strReturnCommand);
	m_pLogFileAccessor->KMLOG(SEND, strLog);

	ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_END_OK]);
	ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_END_NG]);

	if (IsOutOfService() == true)
		return true;

	if (SendPacket(strReturnCommand) == true)
	{
		HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_INSP_END_OK], m_hRecvDEvent[enumRecvEventID_INSP_END_NG] };

		DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, DEFINE_RESPONSE_WAIT_TIME);
		if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
		{
			strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_TIMEOUT"));
			m_pLogFileAccessor->KMLOG(WARN, strLog);
			CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iSeqItemIndex);
			return false;
		}
		// jsyoon 2022.01.17 add
		else if (dw == (WAIT_OBJECT_0 + 1)) // NG
		{
			strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_FAIL"));
			m_pLogFileAccessor->KMLOG(WARN, strLog);
			CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_FAIL + m_iSeqItemIndex);
			return false;
		}
		else
		{
			return true;
		}

		//	return true;
	}

	strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_NOT_CONNECTED"));
	m_pLogFileAccessor->KMLOG(WARN, strLog);
	CAlarmController::GetInstance()->SetAlarm(HEAD_NOT_CONNECTED + m_iSeqItemIndex);
	return false;
#endif // _USE_NOT

	return true;
}


bool CCommTcpIpObjectVision::WaitRecipeChangeEnd(DWORD dwTimeOut)
{
	if (IsOutOfService() == true)
		return true;

	HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_RECIPE_CHANGE_OK], m_hRecvDEvent[enumRecvEventID_RECIPE_CHANGE_NG] };

	CString strLog;
	DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, dwTimeOut * 1000);
	if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
	{
		strLog.Format(_T("[%s] : WaitRecipeChangeEnd, %s"), GetName(), _T("HEAD_ACK_TIMEOUT"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iAlarmIndex);
		return false;
	}
	// jsyoon 2022.01.17 add
	else if (dw == (WAIT_OBJECT_0 + 1)) // NG
	{
		strLog.Format(_T("[%s] : WaitRecipeChangeEnd, %s"), GetName(), _T("HEAD_ACK_FAIL"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_FAIL + m_iAlarmIndex);
		return false;
	}

	return true;
}

bool CCommTcpIpObjectVision::WaitInspectionEnd(DWORD dwTimeOut)
{
	if (IsOutOfService() == true)
		return true;

	HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_INSP_END_OK], m_hRecvDEvent[enumRecvEventID_INSP_END_NG] };

	CString strLog;
	DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, dwTimeOut * 1000);
	if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
	{
		strLog.Format(_T("[%s] : WaitInspectionEnd, %s"), GetName(), _T("HEAD_ACK_TIMEOUT"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iAlarmIndex);
		return false;
	}
	// jsyoon 2022.01.17 add
	else if (dw == (WAIT_OBJECT_0 + 1)) // NG
	{
		strLog.Format(_T("[%s] : WaitInspectionEnd, %s"), GetName(), _T("HEAD_ACK_FAIL"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_FAIL + m_iAlarmIndex);
		return false;
	}

	return true;
}

bool CCommTcpIpObjectVision::WaitInspectionComplete(DWORD dwTimeOut)
{
	if (IsOutOfService() == true)
		return true;

	HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_OK], m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_NG] };

	CString strLog;
	DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, dwTimeOut * 1000);
	if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
	{
		strLog.Format(_T("[%s] : WaitInspectionComplete, %s"), GetName(), _T("HEAD_ACK_TIMEOUT"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iAlarmIndex);
		return false;
	}
	// jsyoon 2022.01.17 add
	else if (dw == (WAIT_OBJECT_0 + 1)) // NG
	{
		strLog.Format(_T("[%s] : WaitInspectionComplete, %s"), GetName(), _T("HEAD_ACK_FAIL"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_FAIL + m_iAlarmIndex);
		return false;
	}

	return true;
}


//=========================================================================
// INTERFACE 관련
//=========================================================================
void CCommTcpIpObjectVision::xNetCallbackRecieveData(LPVOID ud)
{
	//if (CSystemParamShare::GetInstance()->IsAutoMode() == false)
	//	return;

	if ((IsConnected() == true) && (m_pSystemConfigMgr->IsCommunicationTypeString() == true))
	{
		LPRECEIVEDDATA pRcvData = (LPRECEIVEDDATA)ud;

		if (pRcvData->eCommState == (eXNET_COMM_STATE)XNET_NOTCONNECT)
		{
			CString strLog;
			strLog.Format(_T("[%s] : %s"), GetName(), _T("Close Connected"));
			m_pLogFileAccessor->KMLOG(SYST, strLog);

			SetEvent(m_hRecvDEvent[enumRecvEventID_RECIPE_CHANGE_NG]); 
			SetEvent(m_hRecvDEvent[enumRecvEventID_SET_PARAMETER_NG]);
			SetEvent(m_hRecvDEvent[enumRecvEventID_INSP_START_NG]);
			SetEvent(m_hRecvDEvent[enumRecvEventID_INSP_END_NG]);
			SetEvent(m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_NG]);
		}
		else if (pRcvData->eCommState == (eXNET_COMM_STATE)XNET_RECEIVEDATA)
		{
			CString strMessage(pRcvData->szBody);
			//	CString strLog = _T("[RECV - UTG] : ") + strMessage;
			CString strLog;
			strLog.Format(_T("[%s] : %s"), GetName(), strMessage);

			//		CLogWrapper::GetInstance()->KMLOG(SYST, strLog);
			m_pLogFileAccessor->KMLOG(RECV, strLog);

			strMessage.TrimRight();
			strMessage.Replace(_T(" "), _T(""));
			if (strMessage.IsEmpty() == true)
				return;

			//---------------------------------------------------------------------------
			// 패킷 단위로 응답에 대한 처리를 해야 한다.
			// socket 에 데이터가 진입하는데 2개 이상 붙어서 1개의 event 로 들어온다.
			// 모든 패킷에 대해서 command 단위로 분리해서 수행해야 한다.
			//---------------------------------------------------------------------------
			std::vector<CString> vecPacketCommandPool = GetCommand_SDC(strMessage);

			for (size_t i = 0; i < vecPacketCommandPool.size(); ++i)
			{
				const CString strCommand = vecPacketCommandPool[i];

				if (strCommand.Find(_T(".NG")) >= 0)	// NG
				{
					SetEvent(m_hRecvDEvent[enumRecvEventID_RECIPE_CHANGE_NG]);
					SetEvent(m_hRecvDEvent[enumRecvEventID_SET_PARAMETER_NG]);
					SetEvent(m_hRecvDEvent[enumRecvEventID_INSP_START_NG]);
					SetEvent(m_hRecvDEvent[enumRecvEventID_INSP_END_NG]);
					SetEvent(m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_NG]);
				}
				else
				{
					if (strCommand.Find(_T("GRAB.START.OK")) >= 0)	// 검사 시작
					{
						SetEvent(m_hRecvDEvent[enumRecvEventID_INSP_START_OK]);
					}

					else if (strCommand.Find(_T("GRAB.END.OK")) >= 0)	// 검사 완료
					{
						SetEvent(m_hRecvDEvent[enumRecvEventID_INSP_END_OK]);

					//	SetRunToThread(6);
					}

					else if (strCommand.Find(_T("CLASS.END")) >= 0)	// 검사 결과 확인
					{
						//CString strInnerID, strParam;
						//CStringArray strCodeArray;
						//strCodeArray.RemoveAll();

						//int cnt = 0;
						//// FDB.CLASS.END.OK.InnerID
						//while (AfxExtractSubString(strParam, strCommand, cnt, '.'))  // 문자를 잘라준다. (AfxExtractSubString = strTok)
						//{
						//	strCodeArray.Add(strParam);
						//	cnt++;
						//}

						//// InnerID 
						//if (strCodeArray.GetCount() > 4)
						//	strInnerID.Format(_T("%s"), strCodeArray.GetAt(4).GetString());
						////_tcscpy_s(m_pSystem->stLotInfo.m_szResultID[m_Type], 32, strCodeArray.GetAt(0).GetString());

						////if (m_bQualityWait) {
						////	//화질검사대기 03
						////	for (int i = 0; i < strCodeArray.GetCount(); i++) {
						////		strInnerID = strCodeArray.GetAt(i).GetString();
						////		QualityWaitRun(strInnerID);
						////	}
						////}
						////else {
						////	// 결과파일 복사 and update Data
						////	GetMeasData(m_iSeqItemIndex);

						////	SetEvent(m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_OK]);
						////}

						CString strTemp = strCommand;

						int nSizeMax = strTemp.GetLength();

						CString strFindCode = _T("CLASS.END.OK.");
						int nLength = strFindCode.GetLength();

						CString strInnerID;
						CStringArray strCodeArray;
						int nSerchCount = 0;

						SetEvent(m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_OK]);

						//CString strTemp;

						//InnerID 여러개 들어올때 다 처리되게
						while ((nSerchCount = strTemp.Find(strFindCode)) >= 0) {
							strCodeArray.Add(strTemp.Mid(nLength + nSerchCount, 7)); //InnerID 코드가 항상 7자리일때만 가능 (ID+시분초)
							strTemp  = strTemp.Mid(nLength + nSerchCount, nSizeMax);
						}

						if (strCodeArray.GetCount() <= 0) return;
						//_tcscpy_s(m_pSystem->stLotInfo.m_szResultID[m_Type], 32, strCodeArray.GetAt(0).GetString());
						////

						if (m_bQualityWait) {
							//화질검사대기 03
							for (int i = 0; i < strCodeArray.GetCount(); i++) {
								strInnerID = strCodeArray.GetAt(i).GetString();
								QualityWaitRun(strInnerID);
							}
						}
						else {

							CString strIDFile;
							strIDFile.Format(_T("%s//%s.txt"), m_strNetDrive, m_pShareMemory->GetSharedMemory()->stLotInfo.m_szInnerID[m_iDeviceIndex]);
							//CmFileFindTimeOut(strTemp, 5000);
							m_pCommTcpIpFactoryMgr->CmFileFindTimeOut(strIDFile, DEFINE_PARAMETERS_WAIT_TIME);


							int nArea = 0;
							if (m_iDeviceIndex == 1) nArea = 1;

							TossResultFile(m_pShareMemory->GetSharedMemory()->stLotInfo.m_szInnerID[m_iDeviceIndex], nArea/*m_pCommTcpIpFactoryMgr->m_nAreaNum[m_iDeviceIndex]*/);
						}
					}

					else if (strCommand.Find(_T("FILTER.")) >= 0)	// Filter 이동 1
					{
						if (strCommand.Find(_T(".IN.")) > 0)
						{
							// Move Filter 
							// 쓰레드 처리
							CString strTemp = strCommand;
							int nSizeMax = strTemp.GetLength();

							CString strFindCode = _T("FILTER.");
							int nLength = strFindCode.GetLength();
							int iFilter = 0, iFind = 0;;
							iFind = strTemp.Find(strFindCode);
							if (iFind > 0)
							{
								iFilter = _ttoi(strTemp.Mid(iFind + nLength, 2));
							}
							else
							{
								iFilter = 1;
							}

							SetRunToThread(iFilter + 10);	// IN
						}
						else if (strCommand.Find(_T(".OUT.")) > 0)
						{
							// Move Filter 
							// 쓰레드 처리
							CString strTemp = strCommand;
							int nSizeMax = strTemp.GetLength();

							CString strFindCode = _T("FILTER.");
							int nLength = strFindCode.GetLength();
							int iFilter = 0, iFind = 0;;
							iFind = strTemp.Find(strFindCode);
							if (iFind > 0)
							{
								iFilter = _ttoi(strTemp.Mid(iFind + nLength, 2));
							}
							else
							{
								iFilter = 1;
							}

							SetRunToThread(iFilter + 20);	// OUT
						}
						else
						{
							CString strReturnCommand;
							strReturnCommand.Format(_T("MOT.FILTER.NG"));
							SendPacket(strReturnCommand);
						}
					}
					else if (strCommand.Find(_T("POL.MOVE.IN")) >= 0)	// Filter 이동 1
					{
						// Move Filter 
						// 쓰레드 처리
						CString strTemp = strCommand;
						int nSizeMax = strTemp.GetLength();

						CString strFindCode = _T("POL.MOVE.IN.");
						int nLength = strFindCode.GetLength();
						int iFilter = 0, iFind = 0;;
						iFind = strTemp.Find(strFindCode);
						if (iFind > 0)
						{
							iFilter = _ttoi(strTemp.Mid(iFind + nLength, 1));
						}
						else
						{
							iFilter = 1;
						}

					//	SetRunToThread(iFilter);
						SetRunToThread(2);

						/*CString strReturnCommand;
						strReturnCommand.Format(_T("MOT.POL.MOVE.IN.OK"));
						SendPacket(strReturnCommand);

						CString strLog;
						strLog.Format(_T("[%s] : %s"), GetName(), strReturnCommand);
						m_pLogFileAccessor->KMLOG(SEND, strLog);*/
						//			::PostMessage(this->GetParent()->m_hWnd, WM_MSG_MAINT_HEAD, (WPARAM)eMEASURE_TYPE_1 + m_Type, (LPARAM)HEAD_CMD_TO_SERVER_FILER_MOVE_IN);
					}
					else if (strCommand.Find(_T("POL.MOVE.OUT")) >= 0)	// Filter 이동 2
					{
						// Move Filter 
						// 쓰레드 처리
						CString strTemp = strCommand;
						int nSizeMax = strTemp.GetLength();

						CString strFindCode = _T("POL.MOVE.IN.");
						int nLength = strFindCode.GetLength();
						int iFilter = 0, iFind = 0;;
						iFind = strTemp.Find(strFindCode);
						if (iFind > 0)
						{
							iFilter = _ttoi(strTemp.Mid(iFind + nLength, 1));
						}
						else
						{
							iFilter = 1;
						}

						SetRunToThread(5);

						// Move Filter
			//			::PostMessage(this->GetParent()->m_hWnd, WM_MSG_MAINT_HEAD, (WPARAM)eMEASURE_TYPE_1 + m_Type, (LPARAM)HEAD_CMD_TO_SERVER_FILER_MOVE_OUT);
					}
					//////////////////////////////////////////////////////////////////////////

					else if (strCommand.Find(_T("RECIPE.CHANGE.OK")) >= 0)	// 레시피 변경 확인
					{
						SetEvent(m_hRecvDEvent[enumRecvEventID_RECIPE_CHANGE_OK]);
					}

					else if (strCommand.Find(_T("CONNECT")) >= 0)
					{
						;
					}
					else if (strCommand.Find(_T("MANUAL")) >= 0)
					{
						;
					}
					else if (strCommand.Find(_T("ALARM")) >= 0)
					{
						;
					}
					else
					{
						CLogWrapper::GetInstance()->KMLOG(ERRO, _T("Fail : [Unknown Command]"));
						continue;
					}
				}
			}
		}
	}
}

CLogFileAccessor* CCommTcpIpObjectVision::GetLogFileAccessorPtr()
{
	return m_pLogFileAccessor;
}

int CCommTcpIpObjectVision::WriteRecipeFile(void)
{
	TCHAR szValue[MAX_PATH];
	TCHAR	szGlassExistFile[MAX_PATH];
	ZeroMemory(szValue, MAX_PATH);

	CString str;
	str.Format(_T("%s%s"), m_strNetRcpDrive, dfDEFAULT_CHANGERCP_FILE_PATH);

	_tcscpy_s(szGlassExistFile, _countof(szGlassExistFile), str);

	str.Format(_T("%s"), m_pRecipeEditMgr->GetRecipeID());

	BOOL bRet = FALSE;
	bRet = gf_WriteValue(_T("Current Recipe"), _T("Recipe Name"), str, szGlassExistFile);

	//if (m_iAlarmIndex == 1)
	//{
	//	int nRet = VTM_RV_SUCCESSED;
	//	VisMotionPROTOCOL stPrl;
	//	ZeroMemory(&stPrl, sizeof(stPrl));

	//	// Homing
	//	stPrl.iCmd = enumTCPIpFactoryt_VisMotion_1;
	//	nRet = m_pCommTcpIpFactoryMgr->GetCommTcpIpObjectVisMotion()->SetCommand(stPrl);

	//	if (nRet == VTM_RV_SUCCESSED)
	//	{
	//		Sleep(1000);

	//		// Recipe Change
	//		stPrl.iCmd = enumTCPIpFactoryt_VisMotion_0;
	//		_tcscpy_s(stPrl.szModel, _countof(stPrl.szModel), str);
	//		nRet = m_pCommTcpIpFactoryMgr->GetCommTcpIpObjectVisMotion()->SetCommand(stPrl);
	//	}
	//}
		
	return bRet;
}

int CCommTcpIpObjectVision::CopyRecipeFile(void)
{
	int nRet = VTM_RV_SUCCESSED;
	BOOL bRet = FALSE;
	WriteRecipeFile();

	CString strOrg, strNew;
	strOrg.Format(_T("%s%s"), m_pSystemConfigMgr->GetConfigFolderPath(), dfDEFAULT_CHANGERCP_FILE_PATH);

	//// 네트워크 드라이브 연결
	////nRet = NetUseDisConnection(m_Type, flase); //lhj temp 161203
	//nRet = NO_ERROR;
	//if (nRet == NO_ERROR || nRet == ERROR_NOT_CONNECTED)
	//{
	//	//nRet = NetUseConnection(m_Type, false); //lhj temp 161203
	//	if (nRet == NO_ERROR)		// 성공했을 시
	//	{
	//		strNew.Format(_T("%s\\%s"), , dfDEFAULT_CHANGERCP_FILE_PATH);	// jsyoon 20170619 change
	//		gf_MakeDir(strNew);
	//		bRet = CopyFile(strOrg, strNew, FALSE);
	//		if (bRet != TRUE)
	//			nRet = QUALITY_MODULE_CHANGE_RCP_ERROR;
	//		else
	//			nRet = VTM_RV_SUCCESSED;

	//		//NetUseDisConnection(m_Type, flase); //lhj temp 161203
	//	}
	//	else
	//	{
	//		return QUALITY_MODULE_CHANGE_RCP_ERROR;
	//	}
	//}
	//else
	//{
	//	return QUALITY_MODULE_DOWNLOAD_ERROR;
	//}

	return nRet;
}

UINT CCommTcpIpObjectVision::ThreadRun(LPVOID pParam)
{
	CCommTcpIpObjectVision* pProc = (CCommTcpIpObjectVision*)pParam;

	pProc->RunMoveFilter();

	return 0;
}

int CCommTcpIpObjectVision::SetRunToThread(int iPattern)
{
	// TODO: 여기에 구현 코드 추가.
	m_iPattern = iPattern;

	AfxBeginThread(ThreadRun, this);

	return 0;
}

void CCommTcpIpObjectVision::RunMoveFilter()
{
	long nRet = VTM_RV_SUCCESSED;

	CString strReturnCommand, strLog;
	strReturnCommand.Empty();

	switch (m_iPattern) 
	{
	case 0:
	{
		if (m_iAlarmIndex == 1)
			nRet = m_pMaintControlMgr->MoveCmdPosA(AXIS_FILTER1, 1, true);
		else
			nRet = m_pMaintControlMgr->MoveCmdPosA(AXIS_FILTER2, 1, true);

		if (nRet == VTM_RV_SUCCESSED)
		{
			strReturnCommand.Format(_T("MOT.POL.MOVE.IN.OK"));
		}
		else
		{
			strReturnCommand.Format(_T("MOT.POL.MOVE.IN.NG"));
		}

		/*if (nRet == VTM_RV_SUCCESSED)
		{
			strReturnCommand.Format(_T("MOT.POL.MOVE.OUT.OK"));
		}
		else
		{
			strReturnCommand.Format(_T("MOT.POL.MOVE.OUT.NG"));
		}*/
		
		SendPacket(strReturnCommand);
	}
		break;
	case 1:
	{
		VisMotionPROTOCOL stPrl;
		ZeroMemory(&stPrl, sizeof(stPrl));

		stPrl.iCmd = enumTCPIpFactoryt_VisMotion_2;

		if (m_iAlarmIndex == 1)
		{
			stPrl.iActiveID = 1;
			stPrl.iFilter = 2;
		}
		else
		{
			stPrl.iActiveID = 0;
			stPrl.iFilter = 2;
		}

		nRet = m_pCommTcpIpFactoryMgr->GetCommTcpIpObjectVisMotion()->SetCommand(stPrl);

		/*if(m_iAlarmIndex == 1)
			nRet = m_pMaintControlMgr->MoveCmdPosA(AXIS_FILTER1, 2, true);
		else
			nRet = m_pMaintControlMgr->MoveCmdPosA(AXIS_FILTER2, 2, true);*/

		if (nRet == VTM_RV_SUCCESSED)
		{
			strReturnCommand.Format(_T("MOT.POL.MOVE.IN.OK"));
		}
		else
		{
			strReturnCommand.Format(_T("MOT.POL.MOVE.IN.NG"));
		}

		SendPacket(strReturnCommand);
	}
		break;
	case 2:
	{
		VisMotionPROTOCOL stPrl;
		ZeroMemory(&stPrl, sizeof(stPrl));

		stPrl.iCmd = enumTCPIpFactoryt_VisMotion_2;

		if (m_iAlarmIndex == 1)
		{
			stPrl.iActiveID = 1;
			stPrl.iFilter = 2;
		}
		else
		{
			stPrl.iActiveID = 0;
			stPrl.iFilter = 2;
		}

		nRet = m_pCommTcpIpFactoryMgr->GetCommTcpIpObjectVisMotion()->SetCommand(stPrl);

		/*if (m_iAlarmIndex == 1)
			nRet = m_pMaintControlMgr->MoveCmdPosA(AXIS_FILTER1, 2, true);
		else
			nRet = m_pMaintControlMgr->MoveCmdPosA(AXIS_FILTER2, 2, true);*/

		if (nRet == VTM_RV_SUCCESSED)
		{
			strReturnCommand.Format(_T("MOT.POL.MOVE.IN.OK"));
		}
		else
		{
			strReturnCommand.Format(_T("MOT.POL.MOVE.IN.NG"));
		}

		SendPacket(strReturnCommand);
	}
		break;
	case 3:
	{
		if (m_iAlarmIndex == 1)
			nRet = m_pMaintControlMgr->MoveCmdPosA(AXIS_FILTER1, 4, true);
		else
			nRet = m_pMaintControlMgr->MoveCmdPosA(AXIS_FILTER2, 4, true);

		if (nRet == VTM_RV_SUCCESSED)
		{
			strReturnCommand.Format(_T("MOT.POL.MOVE.IN.OK"));
		}
		else
		{
			strReturnCommand.Format(_T("MOT.POL.MOVE.IN.NG"));
		}

		SendPacket(strReturnCommand);
	}
		break;
	case 4:
	{
		if (m_iAlarmIndex == 1)
			nRet = m_pMaintControlMgr->MoveCmdPosA(AXIS_FILTER1, 4, true);
		else
			nRet = m_pMaintControlMgr->MoveCmdPosA(AXIS_FILTER2, 4, true);

		if (nRet == VTM_RV_SUCCESSED)
		{
			strReturnCommand.Format(_T("MOT.POL.MOVE.IN.OK"));
		}
		else
		{
			strReturnCommand.Format(_T("MOT.POL.MOVE.IN.NG"));
		}

		SendPacket(strReturnCommand);
	}
	break;
	case 5:
	{
		VisMotionPROTOCOL stPrl;
		ZeroMemory(&stPrl, sizeof(stPrl));

		stPrl.iCmd = enumTCPIpFactoryt_VisMotion_2;

		if (m_iAlarmIndex == 1)
		{
			stPrl.iActiveID = 1;
			stPrl.iFilter = 0;
		}
		else
		{
			stPrl.iActiveID = 0;
			stPrl.iFilter = 0;
		}

		nRet = m_pCommTcpIpFactoryMgr->GetCommTcpIpObjectVisMotion()->SetCommand(stPrl);

		/*if (m_iAlarmIndex == 1)
			nRet = m_pMaintControlMgr->MoveCmdPosA(AXIS_FILTER1, 0, true);
		else
			nRet = m_pMaintControlMgr->MoveCmdPosA(AXIS_FILTER2, 0, true);*/

		if (nRet == VTM_RV_SUCCESSED)
		{
			strReturnCommand.Format(_T("MOT.POL.MOVE.OUT.OK"));
		}
		else
		{
			strReturnCommand.Format(_T("MOT.POL.MOVE.OUT.NG"));
		}

		SendPacket(strReturnCommand);
	}
		break;
	case 6:
	{
		if (m_iAlarmIndex == 1)
			nRet = m_pMaintControlMgr->MoveCmdPosA(AXIS_FILTER1, 0, true);
		else
			nRet = m_pMaintControlMgr->MoveCmdPosA(AXIS_FILTER2, 0, true);
	}
	break;	
	case 10:
	{
		VisMotionPROTOCOL stPrl;
		ZeroMemory(&stPrl, sizeof(stPrl));

		stPrl.iCmd = enumTCPIpFactoryt_VisMotion_2;

		if (m_iAlarmIndex == 1)
		{
			stPrl.iActiveID = 1;
			stPrl.iFilter = 0;  // ZERO
		}
		else
		{
			stPrl.iActiveID = 0;
			stPrl.iFilter = 0;
		}

		nRet = m_pCommTcpIpFactoryMgr->GetCommTcpIpObjectVisMotion()->SetCommand(stPrl);

		/*if (m_iAlarmIndex == 1)
			nRet = m_pMaintControlMgr->MoveCmdPosA(AXIS_FILTER1, 0, true);
		else
			nRet = m_pMaintControlMgr->MoveCmdPosA(AXIS_FILTER2, 0, true);*/

		if (nRet == VTM_RV_SUCCESSED)
		{
			strReturnCommand.Format(_T("MOT.FILTER.0.IN.OK"));
		}
		else
		{
			strReturnCommand.Format(_T("MOT.FILTER.0.IN.NG"));
		}

		SendPacket(strReturnCommand);
	}
	break;
	case 11:
	{
		VisMotionPROTOCOL stPrl;
		ZeroMemory(&stPrl, sizeof(stPrl));

		stPrl.iCmd = enumTCPIpFactoryt_VisMotion_2;

		if (m_iAlarmIndex == 1)
		{	
			stPrl.iActiveID = 1;
			stPrl.iFilter = 1;  // R
		}
		else
		{
			stPrl.iActiveID = 0;
			stPrl.iFilter = 1;
		}

		nRet = m_pCommTcpIpFactoryMgr->GetCommTcpIpObjectVisMotion()->SetCommand(stPrl);

		/*if (m_iAlarmIndex == 1)
			nRet = m_pMaintControlMgr->MoveCmdPosA(AXIS_FILTER1, 1, true);
		else
			nRet = m_pMaintControlMgr->MoveCmdPosA(AXIS_FILTER2, 1, true);*/

		if (nRet == VTM_RV_SUCCESSED)
		{
			strReturnCommand.Format(_T("MOT.FILTER.1.IN.OK"));
		}
		else
		{
			strReturnCommand.Format(_T("MOT.FILTER.1.IN.NG"));
		}

		SendPacket(strReturnCommand);
	}
	break;
	case 12:
	{
		VisMotionPROTOCOL stPrl;
		ZeroMemory(&stPrl, sizeof(stPrl));

		stPrl.iCmd = enumTCPIpFactoryt_VisMotion_2;

		if (m_iAlarmIndex == 1)
		{
			stPrl.iActiveID = 1;
			stPrl.iFilter = 2;  // G
		}
		else
		{
			stPrl.iActiveID = 0;
			stPrl.iFilter = 2;
		}

		nRet = m_pCommTcpIpFactoryMgr->GetCommTcpIpObjectVisMotion()->SetCommand(stPrl);

		/*if (m_iAlarmIndex == 1)
			nRet = m_pMaintControlMgr->MoveCmdPosA(AXIS_FILTER1, 2, true);
		else
			nRet = m_pMaintControlMgr->MoveCmdPosA(AXIS_FILTER2, 2, true);*/

		if (nRet == VTM_RV_SUCCESSED)
		{
			strReturnCommand.Format(_T("MOT.FILTER.2.IN.OK"));
		}
		else
		{
			strReturnCommand.Format(_T("MOT.FILTER.2.IN.NG"));
		}

		SendPacket(strReturnCommand);
	}
	break;
	case 13:
	{
		VisMotionPROTOCOL stPrl;
		ZeroMemory(&stPrl, sizeof(stPrl));

		stPrl.iCmd = enumTCPIpFactoryt_VisMotion_2;

		if (m_iAlarmIndex == 1)
		{
			stPrl.iActiveID = 1;
			stPrl.iFilter = 3;   // B
		}
		else
		{
			stPrl.iActiveID = 0;
			stPrl.iFilter = 3;
		}

		nRet = m_pCommTcpIpFactoryMgr->GetCommTcpIpObjectVisMotion()->SetCommand(stPrl);

		/*if (m_iAlarmIndex == 1)
			nRet = m_pMaintControlMgr->MoveCmdPosA(AXIS_FILTER1, 3, true);
		else
			nRet = m_pMaintControlMgr->MoveCmdPosA(AXIS_FILTER2, 3, true);*/

		if (nRet == VTM_RV_SUCCESSED)
		{
			strReturnCommand.Format(_T("MOT.FILTER.3.IN.OK"));
		}
		else
		{
			strReturnCommand.Format(_T("MOT.FILTER.3.IN.NG"));
		}

		SendPacket(strReturnCommand);
	}
	break;
	case 21:
	{
		VisMotionPROTOCOL stPrl;
		ZeroMemory(&stPrl, sizeof(stPrl));

		stPrl.iCmd = enumTCPIpFactoryt_VisMotion_2;

		if (m_iAlarmIndex == 1)
		{
			stPrl.iActiveID = 1;
			stPrl.iFilter = 0;  // ZERO
		}
		else
		{
			stPrl.iActiveID = 0;
			stPrl.iFilter = 0;
		}

		nRet = m_pCommTcpIpFactoryMgr->GetCommTcpIpObjectVisMotion()->SetCommand(stPrl);

		/*if (m_iAlarmIndex == 1)
			nRet = m_pMaintControlMgr->MoveCmdPosA(AXIS_FILTER1, 0, true);
		else
			nRet = m_pMaintControlMgr->MoveCmdPosA(AXIS_FILTER2, 0, true);*/

		if (nRet == VTM_RV_SUCCESSED)
		{
			strReturnCommand.Format(_T("MOT.FILTER.01.OUT.OK"));
		}
		else
		{
			strReturnCommand.Format(_T("MOT.FILTER.01.OUT.NG"));
		}

		SendPacket(strReturnCommand);
	}
	break;
	case 22:
	{
		VisMotionPROTOCOL stPrl;
		ZeroMemory(&stPrl, sizeof(stPrl));

		stPrl.iCmd = enumTCPIpFactoryt_VisMotion_2;

		if (m_iAlarmIndex == 1)
		{
			stPrl.iActiveID = 1;
			stPrl.iFilter = 0;
		}
		else
		{
			stPrl.iActiveID = 0;
			stPrl.iFilter = 0;
		}

		nRet = m_pCommTcpIpFactoryMgr->GetCommTcpIpObjectVisMotion()->SetCommand(stPrl);

		/*if (m_iAlarmIndex == 1)
			nRet = m_pMaintControlMgr->MoveCmdPosA(AXIS_FILTER1, 0, true);
		else
			nRet = m_pMaintControlMgr->MoveCmdPosA(AXIS_FILTER2, 0, true);*/

		if (nRet == VTM_RV_SUCCESSED)
		{
			strReturnCommand.Format(_T("MOT.FILTER.2.OUT.OK"));
		}
		else
		{
			strReturnCommand.Format(_T("MOT.FILTER.2.OUT.NG"));
		}

		SendPacket(strReturnCommand);
	}
	break;
	case 23:
	{
		VisMotionPROTOCOL stPrl;
		ZeroMemory(&stPrl, sizeof(stPrl));

		stPrl.iCmd = enumTCPIpFactoryt_VisMotion_2;

		if (m_iAlarmIndex == 1)
		{
			stPrl.iActiveID = 1;
			stPrl.iFilter = 0;
		}
		else
		{
			stPrl.iActiveID = 0;
			stPrl.iFilter = 0;
		}

		nRet = m_pCommTcpIpFactoryMgr->GetCommTcpIpObjectVisMotion()->SetCommand(stPrl);

		/*if (m_iAlarmIndex == 1)
			nRet = m_pMaintControlMgr->MoveCmdPosA(AXIS_FILTER1, 0, true);
		else
			nRet = m_pMaintControlMgr->MoveCmdPosA(AXIS_FILTER2, 0, true);*/

		if (nRet == VTM_RV_SUCCESSED)
		{
			strReturnCommand.Format(_T("MOT.FILTER.03.OUT.OK"));
		}
		else
		{
			strReturnCommand.Format(_T("MOT.FILTER.03.OUT.NG"));
		}

		SendPacket(strReturnCommand);
	}
	break;
	default:
		break;
	}

	if (strReturnCommand.IsEmpty() != true)
	{
		strLog.Format(_T("[%s] : %s"), GetName(), strReturnCommand);
		m_pLogFileAccessor->KMLOG(SEND, strLog);
	}
}

BOOL CCommTcpIpObjectVision::TossResultFile(CString strInnerID, int nArea)
{
	//RawData 복사 진행
	CString strNewPath, strNewFile, strRawFile;
	CString strIDFile;
	CString strTemp;

	strIDFile.Format(_T("%s.txt"), strInnerID);

	// 복사할 경로 위치 확인 필요
	strNewPath.Format(_T("%sRawData\\%s\\%s\\%s\\"), CSystemParamShare::GetInstance()->GetResultFolderPath(), gf_GetCurDayString(), CSystemParamShare::GetInstance()->GetGlassID(), _T("VISION"));
	strNewFile.Format(_T("%s%s"), strNewPath, strIDFile);
//	strNewFile.Format(_T("%sRawData\\%s\\%s\\%s"), CSystemParamShare::GetInstance()->GetResultFolderPath(), gf_GetCurDayString(), CSystemParamShare::GetInstance()->GetGlassID(), strIDFile);
	strRawFile.Format(_T("%s\\%s"), m_strNetDrive, strIDFile);

	//화질 제외 결과파일 생성될때까지 기다리기
	//strTemp.Format(_T("%s\\%s"), m_strNetDrive, strIDFile);
	//CmFileFindTimeOut(strTemp, 5000);
	
	//폴더 생성, 파일 복사
	gf_MakeDir(strNewFile);
	if (CopyFile(strRawFile, strNewFile, FALSE))
	{
		//if (m_pJobProc->m_iUseDelData == 1)
//		DeleteFile(strRawFile);
	}

	//결과 파일 정보 전달
	RSRITEMDATA resultData;
	ZeroMemory(&resultData, sizeof(resultData));

	resultData.nMeasMode = PGM_MODE_VISION; //확인 필요
	resultData.iMeasArea = nArea;

//	_tcscpy(resultData.szGlassID, CSystemParamShare::GetInstance()->GetGlassID());
	_tcsncpy_s(resultData.szLotID, _countof(resultData.szLotID), CSystemParamShare::GetInstance()->GetLotID(), _TRUNCATE);	// LYS 2026-10-14 recipe ID length: truncate into RSRITEMDATA szLotID[32] (unchecked _tcscpy overran into szGlassID)
	_tcsncpy_s(resultData.szGlassID, _countof(resultData.szGlassID), CSystemParamShare::GetInstance()->GetGlassID(), _TRUNCATE);	// LYS 2026-10-14 recipe ID length: truncate into RSRITEMDATA szGlassID[32] (unchecked _tcscpy overran into szRecipeID)
	_tcsncpy_s(resultData.szRecipeID, _countof(resultData.szRecipeID), CSystemParamShare::GetInstance()->GetRecipeID(), _TRUNCATE);	// LYS 2026-10-14 recipe ID length: truncate into RSRITEMDATA szRecipeID[32] (unchecked _tcscpy overran into szResultPath)

	_tcscpy(resultData.szResultFile, strIDFile /*strInnerID*/);

	_tcscpy(resultData.szResultPath, strNewPath);

	CResultMgr::GetInstance()->Updata_Data(&resultData);

	return TRUE;
}

//화질검사 결과 자동 처리 (2개 버퍼 사용), 화질검사 프로그램에서 두개의 쓰레드로 데이터 처리 한다고함.
UINT	CCommTcpIpObjectVision::QualityWaitThread(LPVOID pParam)
{
	CCommTcpIpObjectVision* pQuality = (CCommTcpIpObjectVision*)pParam;
	CString strRemotePath, strPath;
	//CString strLog;
	BOOL	bSearchFail = FALSE;

	//if		(pQuality->m_nHead == 0)	strRemotePath = _T("Z:");
	//else if (pQuality->m_nHead == 1)	strRemotePath = _T("Y:");
	strRemotePath = pQuality->m_strNetDrive;

	while (pQuality->m_bRunThread) {
		for (int i = 0; i < QUALITY_MAX; i++) {
			if (pQuality->sQualityWait[i].m_bRunStatus) {
				// LYS 2026-10-08 Hybrid Phase 5 (4-3) journal copies of this grab taken BEFORE the up-to-150 s file wait: a Recipe Change
				//   (EXE_QUALITY_RESET) can clear the slot and the next grab reuse it meanwhile. Live behaviour below is unchanged
				CString strInner0 = pQuality->sQualityWait[i].m_strInnerID;
				int nRow0 = pQuality->sQualityWait[i].m_nSheetRow;
				S_ResumeCtx ctx0 = pQuality->sQualityWait[i].m_sJournal;

				//파일 찾고 나서 진행
				strPath.Format(_T("%s\\%s.txt"), strRemotePath, pQuality->sQualityWait[i].m_strInnerID);

				//파일 찾으면 (TimeOut 아니면)
				if (!pQuality->m_pCommTcpIpFactoryMgr->CmFileFindTimeOut(strPath, 150000/*pQuality->m_nQualityDelay*/)) {
					//pQuality->GetMeasData(pQuality->m_nHead, pQuality->sQualityWait[i].m_strInnerID, pQuality->sQualityWait[i].m_nSheetRow, pQuality->sQualityWait[i].m_strCellID);
					//TossResultFile //여기 함수에서 복사하고 결과파일 정보 전달하기
					// LYS 2026-10-08 Hybrid Phase 5 (4-3) the journal row of this toss takes the grab-time context (thread-local, read by the
					//   CResultMgr::Updata_Data hook); a slot reused during the wait is left out of the journal (nGen 0). The toss is unchanged
					static const S_ResumeCtx s_ctxDropped = { 0, 0, 0, 0, 0, 0 };
					CResumeJournal::SetThreadCtx((pQuality->sQualityWait[i].m_strInnerID == strInner0) ? &ctx0 : &s_ctxDropped);
					pQuality->TossResultFile(pQuality->sQualityWait[i].m_strInnerID, pQuality->sQualityWait[i].m_nSheetRow);
					CResumeJournal::SetThreadCtx(NULL);
					// LYS 2026-10-01 Hybrid Phase 4 (4-4) hybrid only: grab-time row / cell ID / segment from the slot - the file may land up to
					//   150 s later, in the next segment. 1 segment: slot tag "" -> no line
					if (!pQuality->sQualityWait[i].m_strSegTag.IsEmpty())
					{
						CString strSegLog;
						strSegLog.Format(_T("[%s] : TossResultFile Updata, %s row(%d) cell(%s)%s"), (LPCTSTR)pQuality->GetName(), (LPCTSTR)pQuality->sQualityWait[i].m_strInnerID,
							pQuality->sQualityWait[i].m_nSheetRow, (LPCTSTR)pQuality->sQualityWait[i].m_strCellID, (LPCTSTR)pQuality->sQualityWait[i].m_strSegTag);
						pQuality->m_pLogFileAccessor->KMLOG(OPER, strSegLog);
					}
				}
				else
				{	// LYS 2026-10-08 Hybrid Phase 5 (4-3) no result file within 150 s: the slot is cleared below as before and the row stays empty -
					//   an X line closes the grab's P so a resume does not pull back to it (journal glass only; nGen 0 -> nothing written)
					CResumeJournal::GetInstance()->AppendClose(ctx0, pQuality->m_iDeviceIndex, nRow0, strInner0);
				}
				//

				pQuality->sQualityWait[i].Clear();
			}

			if (!pQuality->m_bRunThread) break;
		}

		Sleep(100);

	}
	pQuality->m_pWaitThread = NULL;

	return 0;
}

//두개의 버퍼중 하나라도 비어있으면 TRUE
BOOL	CCommTcpIpObjectVision::QualityBufferEmpty()
{
	if (m_bOutOfService) return TRUE;

	for (int i = 0; i < QUALITY_MAX; i++) {
		if (sQualityWait[i].m_bWaitStatus == FALSE) return TRUE;
	}

	return FALSE;
}

//두개의 버퍼가 모두 비어있으면 TRUE
BOOL	CCommTcpIpObjectVision::QualityBufferEmptyAll()
{
	if (m_bOutOfService) return TRUE;

	for (int i = 0; i < QUALITY_MAX; i++) {
		if (sQualityWait[i].m_bWaitStatus == TRUE) return FALSE;
	}

	return TRUE;
}

//화질검사 결과 처리 준비 및 내용(InnerID, Data출력 위치(SheetRow)) 저장
void	CCommTcpIpObjectVision::QualityWaitSet(CString strInnerID, int nSheetRow, CString strCellID)
{
	if (nSheetRow < 0) return;	// 2023.04.09 add

	for (int i = 0; i < QUALITY_MAX; i++) {
		if (sQualityWait[i].m_bWaitStatus == FALSE) {
			sQualityWait[i].m_strInnerID  = strInnerID;
			sQualityWait[i].m_strCellID	  = strCellID;
			sQualityWait[i].m_nSheetRow	  = nSheetRow;
			sQualityWait[i].m_strSegTag  = (m_pRecipeEditMgr != nullptr) ? m_pRecipeEditMgr->GetSegLogTag(TRUE) : CString();	// LYS 2026-10-01 Hybrid Phase 4 (4-4) grab-time segment tag (1 segment: "")
			// LYS 2026-10-08 Hybrid Phase 5 (4-3) grab-time journal context + P line (head = m_iDeviceIndex, row = nSheetRow = the iMeasArea
			//   TossResultFile uploads). Not armed (1-segment default, manual measure) -> nGen 0: nothing written. Offline the result file
			//   never comes, so the P line is kept only with RESUME_JOURNAL_OOS_SIM (closed there by the S lines)
			sQualityWait[i].m_sJournal = CResumeJournal::GetInstance()->Capture();
			if (IsOutOfService() == false || CResumeJournal::GetInstance()->GetSimMode() > 0)
				CResumeJournal::GetInstance()->AppendPending(sQualityWait[i].m_sJournal, m_iDeviceIndex, nSheetRow, strInnerID);
			sQualityWait[i].m_bWaitStatus = TRUE;
			return;
		}
	}
	// LYS 2026-10-08 Hybrid Phase 5 (4-3) OOS + RESUME_JOURNAL_OOS_SIM only: offline no INSP END comes (m_bRunStatus is never set), so both
	//   slots stay full after the first 2 grabs and every later grab lands here - keep its P line so the pull-back test sees the whole
	//   glass. Real runs (and OOS without the sim key): no line, the grab is dropped as before
	if (IsOutOfService() == true && CResumeJournal::GetInstance()->GetSimMode() > 0)
		CResumeJournal::GetInstance()->AppendPending(CResumeJournal::GetInstance()->Capture(), m_iDeviceIndex, nSheetRow, strInnerID);
}

//화질검사 결과 처리 실행 (결과 데이터에서 받은 InnerID 를 토대로 설정 되었던 InnerID 비교 하여 처리)
void	CCommTcpIpObjectVision::QualityWaitRun(CString strInnerID)
{
	for (int i = 0; i < QUALITY_MAX; i++) {
		if (sQualityWait[i].m_bWaitStatus == TRUE && sQualityWait[i].m_strInnerID == strInnerID) {
			sQualityWait[i].m_bRunStatus = TRUE;
			return;
		}
	}
}

//화질검사 구조체 초기화
void	CCommTcpIpObjectVision::QualityWaitClear()
{
	for (int i = 0; i < QUALITY_MAX; i++) sQualityWait[i].Clear();
}

//BOOL CCommTcpIpObjectVision::CmFileFind(CString strFile)
//{
//	//파일 이름만 가져오기
//	int pos = 0;
//	CString strID;
//	CString resToken = strFile.Tokenize(_T("\\"), pos);
//	while (resToken != _T("")) {
//		strID = resToken.Trim();
//		resToken = strFile.Tokenize(_T("\\"), pos);
//	}
//
//	//파일 찾기
//	BOOL bExist = FALSE;
//	CFileFind finder;
//	BOOL bWorking = finder.FindFile(strFile);
//	while (bWorking) {
//		bWorking = finder.FindNextFile();
//		if (finder.IsDots() || finder.IsDirectory()) continue;
//		if (strID == finder.GetFileName()) {
//			bExist = TRUE;
//			break;
//		}
//
//	}
//	finder.Close();
//
//	return bExist;
//}
//
//BOOL	CCommTcpIpObjectVision::CmFileFindTimeOut(CString strFile, int nTimeOut)
//{
//	BOOL bTimeOut = FALSE;
//
//	DWORD dwTimeout = nTimeOut;
//	DWORD dwBeginTick = GetTickCount();
//	DWORD dwRecheck = 1000; //1Sec
//	DWORD dwCurrentTick = 0;
//	DWORD dwTemp = 0;
//
//	if (!CmFileFind(strFile)) {
//		while (1) {
//			dwCurrentTick = GetTickCount();
//
//			if ((dwCurrentTick - dwBeginTick) < 0) dwBeginTick = dwTemp; //음수로 나오면 기존 지난 시간 사용
//			else dwTemp = (dwCurrentTick - dwBeginTick);
//
//			if ((dwCurrentTick - dwBeginTick) > dwTimeout) {
//				bTimeOut = TRUE;
//				break;
//			}
//			else if ((dwCurrentTick - dwBeginTick) > dwRecheck) {
//				if (CmFileFind(strFile)) break;
//				dwRecheck += 1000;
//			}
//		}
//	}
//
//	return bTimeOut;
//}