#include "pch.h"
#include "CommTcpIpObjectAngle.h"

#include "Manager\NetworkConfigMgr.h"
#include "Manager\SystemConfigMgr.h"
#include "Recipe\RecipeEditMgr.h"
#include "SystemParamShare.h"
#include "Alarm\AlarmController.h"
#include "AuroraFuncDefine.h"
#include "Manager\SequenceManager.h"
#include "Manager/ResultMgr.h"
#include "CommEthFunction\EthernetManagerTCP.h"
#include "common\CommonFunc.h"
#include "Manager/CommTcpIpFactoryMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCommTcpIpObjectAngle::CCommTcpIpObjectAngle(int iSeqItemIndex, stAuroraMgrPool& MgrPool)
	: m_pSystemConfigMgr(MgrPool.pSystemConfigMgr)
	, m_pRecipeEditMgr(MgrPool.pRecipeEditMgr)
	, m_pResultCollectMgr(MgrPool.pResultCollectMgr)
	, m_pShareMemory(MgrPool.pShareMemoryMgr)
	, m_pLogFileAccessor(new CLogFileAccessor)
	, m_bOutOfService(false)
	, m_iAlarmIndex(0)
	, m_iDeviceIndex(0)
	, m_pCommTcpIpFactoryMgr(MgrPool.pCommTcpIpFactoryMgr)
{
	m_strQueueMessage.Empty();
	m_pRecipeDataClone = nullptr;

	int iDeviceIndex = MgrPool.pNetworkConfigMgr->GetParam_DeviceIndex(iSeqItemIndex);
	m_iDeviceIndex = iDeviceIndex;

	m_iAlarmIndex = enumTCPIpFactorytID_ANGLE + iDeviceIndex;

	CString strObjectName;
	strObjectName.Format(_T("%s%d"), _T("시야각"), iDeviceIndex + 1);

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

	m_strNetDrive = MgrPool.pNetworkConfigMgr->GetParam_NetDrive(iSeqItemIndex);

	m_strNetRcpDrive = MgrPool.pNetworkConfigMgr->GetParam_NetRcpDrive(iSeqItemIndex);
}


CCommTcpIpObjectAngle::~CCommTcpIpObjectAngle()
{
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

void CCommTcpIpObjectAngle::UpdateRecipeInformation()
{
	m_pRecipeDataClone->Clone(m_pRecipeEditMgr->GetRecipeDataPtr());
}

bool CCommTcpIpObjectAngle::SetTime_Sync()
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

bool CCommTcpIpObjectAngle::RecipeChange()
{
	int iRet = VTM_RV_SUCCESSED;
	CString strLog;

	if (IsOutOfService() == true)
		return true;

	// File copy "Changerecipe.ini"
	iRet = WriteRecipeFile(); // CopyRecipeFile();
	if (iRet != VTM_RV_SUCCESSED)
	{
		strLog.Format(_T("[%s] : %s"), GetName(), _T("CHANGE RECIPE FILE FAIL"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		CAlarmController::GetInstance()->SetAlarm(QUALITY_MODULE_CHANGE_RCP_ERROR);
		return false;
	}

	CString strData, strReturnCommand;
	strData.Format(_T("MOT.A%d.RECIPE.CHANGE"), m_iDeviceIndex + 1);

	TBYTE nCmd = 0x30, nByte = 0x30, nLen[4], nStx = 0x04, nEtx = 0x08;
	TCHAR	szTmp[1000];
	TCHAR	szCmd[1000];
	szTmp[0] = 0x00;
	szCmd[0] = 0x00;
	_stprintf_s(szCmd, _countof(szCmd), _T("%c"), nStx);		// STX
	_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nCmd);	_tcscat_s(szCmd, _countof(szCmd), szTmp); // Header

	const int len = strData.GetLength();
	CString strDataLength;
	strDataLength.Format(_T("%04X"), len);
	_stprintf_s(szTmp, _countof(szTmp), _T("%s"), strDataLength);	_tcscat_s(szCmd, _countof(szCmd), szTmp);

	//int len = strData.GetLength();
	//nLen[0] = len / 1000 + nByte;
	//_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nLen[0]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
	//nLen[1] = (len % 1000) / 100 + nByte;
	//_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nLen[1]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
	//nLen[2] = (len % 1000) % 100 / 10 + nByte;
	//_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nLen[2]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
	//nLen[3] = (len % 1000) % 100 % 10 + nByte;
	//_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nLen[3]);	_tcscat_s(szCmd, _countof(szCmd), szTmp);

//	_stprintf_s(szTmp, _countof(szTmp), _T("%04d"), strData.GetLength());	_tcscat_s(szCmd, _countof(szCmd), szTmp); // Header

	_stprintf_s(szTmp, _countof(szTmp), _T("%s"), strData); _tcscat_s(szCmd, _countof(szCmd), szTmp);	// Data
	_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nEtx);    _tcscat_s(szCmd, _countof(szCmd), szTmp);	// EXT
	int iLen = _tcsclen(szCmd);
	szCmd[iLen] = 0;
	strReturnCommand.Format(_T("%s"), szCmd);

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
			return true;
		}
	}

	strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_NOT_CONNECTED"));
	m_pLogFileAccessor->KMLOG(WARN, strLog);
	CAlarmController::GetInstance()->SetAlarm(HEAD_NOT_CONNECTED + m_iAlarmIndex);
	return false;
}

bool CCommTcpIpObjectAngle::SetParameter()
{
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

bool CCommTcpIpObjectAngle::InspectStart(int iMeasType, bool bWait, int iStepNum)
{
	CString strData, strReturnCommand;

	CString str, strCellID, strGlassID, strInnerID, strMcrID;

	strInnerID.Format(_T("%s"), m_pShareMemory->GetSharedMemory()->stLotInfo.m_szInnerID[m_iDeviceIndex]);
	if (strInnerID.IsEmpty())
	{
		strInnerID.Format(_T("%d%s"), 1 + m_iDeviceIndex, gf_GetCurTimeString(3));
		_tcscpy_s(m_pShareMemory->GetSharedMemory()->stLotInfo.m_szInnerID[m_iDeviceIndex], 32, strInnerID);
	}

	strMcrID.Format(_T("%s"), m_pShareMemory->GetSharedMemory()->stLotInfo.m_szCellID[m_iDeviceIndex]);

	strData.Format(_T("MOT.A%d.GRAB.START.%s.%s"), m_iDeviceIndex + 1, strInnerID, strMcrID);

	TBYTE nCmd = 0x30, nByte = 0x30, nLen[4], nStx = 0x04, nEtx = 0x08;
	TCHAR	szTmp[1000];
	TCHAR	szCmd[1000];
	szTmp[0] = 0x00;
	szCmd[0] = 0x00;
	_stprintf_s(szCmd, _countof(szCmd), _T("%c"), nStx);		// STX
	_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nCmd);	_tcscat_s(szCmd, _countof(szCmd), szTmp); // Header

	const int len = strData.GetLength();
	CString strDataLength;
	strDataLength.Format(_T("%04X"), len);

	nLen[0] = len / 1000 + nByte;
	_stprintf_s(szTmp, _countof(szTmp), _T("%s"), strDataLength);	_tcscat_s(szCmd, _countof(szCmd), szTmp);

	_stprintf_s(szTmp, _countof(szTmp), _T("%s"), strData); _tcscat_s(szCmd, _countof(szCmd), szTmp);	// Data
	_stprintf_s(szTmp, _countof(szTmp), _T("%c"), nEtx);    _tcscat_s(szCmd, _countof(szCmd), szTmp);	// EXT
	int iLen = _tcsclen(szCmd);
	szCmd[iLen] = 0;
	strReturnCommand.Format(_T("%s"), szCmd);

//	strReturnCommand.Format(_T("%c%c%d%s%c"), 0x04, 0x00, strData.GetLength(), strData, 0x08);

	CString strLog;
	strLog.Format(_T("[%s] : %s"), GetName(), strReturnCommand);
	m_pLogFileAccessor->KMLOG(SEND, strLog);

	ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_START_OK]);
	ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_START_NG]);

	ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_END_OK]);
	ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_END_NG]);

	ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_OK]);
	ResetEvent(m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_NG]);

	if (IsOutOfService() == true)
		return true;

	/*SYSTEMTIME st;
	::GetLocalTime(&st);
	CString sTime;
	sTime.Format(_T("%04d%02d%02d%02d%02d%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);*/
	//	CString strCellID = CSystemParamShare::GetInstance()->GetCellID();
	//	CString strFileName = strCellID + _T("_") + sTime;
	//	CString strReturnCommand = GetStringCMDID(COMM_USER_DEFINE_REQ) + _T("^INSP_START^") + strCellID + _T("^") + strFileName;
	if (SendPacket(strReturnCommand) == true)
	{
		if (bWait)
		{
			HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_INSP_START_OK], m_hRecvDEvent[enumRecvEventID_INSP_START_NG] };

			DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, DEFINE_INSPECITON_WAIT_TIME);
			if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
			{
				strLog.Format(_T("[%s] : %s, %s"), GetName(), _T("InspectStart, HEAD_ACK_TIMEOUT"), strReturnCommand);
				m_pLogFileAccessor->KMLOG(WARN, strLog);
				CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iAlarmIndex);
				return false;
			}
			// jsyoon 2022.08.30 add
			else if (dw == (WAIT_OBJECT_0 + 1)) // NG
			{
				strLog.Format(_T("[%s] : %s, %s"), GetName(), _T("InspectStart, HEAD_ACK_FAIL"), strReturnCommand);
				m_pLogFileAccessor->KMLOG(WARN, strLog);
				CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_FAIL + m_iAlarmIndex);
				return false;
			}
			else
			{
				;
			}
		}

		return true;
	}

	strLog.Format(_T("[%s] : %s, %s"), GetName(), _T("HEAD_NOT_CONNECTED"), strReturnCommand);
	m_pLogFileAccessor->KMLOG(WARN, strLog);
	CAlarmController::GetInstance()->SetAlarm(HEAD_NOT_CONNECTED + m_iAlarmIndex);
	return false;
}

bool CCommTcpIpObjectAngle::InspectStartWait()
{
	if (IsOutOfService() == true)
	{
		Sleep(1000);
		return true;
	}

	HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_INSP_START_OK], m_hRecvDEvent[enumRecvEventID_INSP_START_NG] };


	int nWaitTimeOut = DEFINE_PARAMETERS_WAIT_TIME;

	CString strLog;
	DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, nWaitTimeOut);

	//CString strLog;
	//DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, DEFINE_INSPECITON_WAIT_TIME);
	if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
	{
		strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_TIMEOUT, InspectStartWait"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iAlarmIndex);
		return false;
	}
	// jsyoon 2022.01.17 add
	else if (dw == (WAIT_OBJECT_0 + 1)) // NG
	{
		strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_FAIL, InspectStartWait"));
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

bool CCommTcpIpObjectAngle::InspectEnd()
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

bool CCommTcpIpObjectAngle::WaitRecipeChangeEnd(DWORD dwTimeOut)
{
	if (IsOutOfService() == true)
		return true;

	HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_RECIPE_CHANGE_OK], m_hRecvDEvent[enumRecvEventID_RECIPE_CHANGE_NG] };

	CString strLog;
	DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, dwTimeOut * 1000);
	if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
	{
		strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_TIMEOUT, WaitRecipeChangeEnd"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iAlarmIndex);
		return false;
	}
	// jsyoon 2022.01.17 add
	else if (dw == (WAIT_OBJECT_0 + 1)) // NG
	{
		strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_FAIL, WaitRecipeChangeEnd"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_FAIL + m_iAlarmIndex);
		return false;
	}

	return true;
}

bool CCommTcpIpObjectAngle::WaitInspectionEnd(DWORD dwTimeOut)
{
	if (IsOutOfService() == true)
		return true;

	HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_INSP_END_OK], m_hRecvDEvent[enumRecvEventID_INSP_END_NG] };

	CString strLog;
	DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, dwTimeOut * 1000);
	if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
	{
		strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_TIMEOUT, WaitInspectionEnd"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iAlarmIndex);
		return false;
	}
	// jsyoon 2022.01.17 add
	else if (dw == (WAIT_OBJECT_0 + 1)) // NG
	{
		strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_FAIL, WaitInspectionEnd"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_FAIL + m_iAlarmIndex);
		return false;
	}

	return true;
}

bool CCommTcpIpObjectAngle::WaitInspectionComplete(DWORD dwTimeOut)
{
	if (IsOutOfService() == true)
		return true;

	HANDLE hRecvEvent[2] = { m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_OK], m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_NG] };

	CString strLog;
	DWORD dw = WaitForMultipleObjects(2, hRecvEvent, FALSE, dwTimeOut * 1000);
	if (dw == WAIT_TIMEOUT) // Start Event 가 set될 때까지 기다린다.
	{
		strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_TIMEOUT, WaitInspectionComplete"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_TIMEOUT + m_iAlarmIndex);
		return false;
	}
	// jsyoon 2022.01.17 add
	else if (dw == (WAIT_OBJECT_0 + 1)) // NG
	{
		strLog.Format(_T("[%s] : %s"), GetName(), _T("HEAD_ACK_FAIL, WaitInspectionComplete"));
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		CAlarmController::GetInstance()->SetAlarm(HEAD_ACK_FAIL + m_iAlarmIndex);
		return false;
	}

	return true;
}


//=========================================================================
// INTERFACE 관련
//=========================================================================
void CCommTcpIpObjectAngle::xNetCallbackRecieveData(LPVOID ud)
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
		}
		else if (pRcvData->eCommState == (eXNET_COMM_STATE)XNET_RECEIVEDATA)
		{
			CString strMessage, strTemp;

			char* pchar = pRcvData->szBody;
			int nLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pchar, -1, NULL, NULL);

			wchar_t szWide[100];
			memset(szWide, 0, sizeof(wchar_t) * 100);

			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pchar, pRcvData->sizeBody + 1, szWide, pRcvData->sizeBody);

			for (int i = 0; i < pRcvData->sizeBody; i++)
			{
				if (szWide[i] == 0X00)
				{
					szWide[i] = '0';
				}
			}

			strMessage.Format(_T("%s"), szWide);

			//for (int i = 0; i < pRcvData->sizeBody; i++)
			//{
			//	strTemp.Format(_T("%s"), szWide[i]);

			//	strMessage += strTemp;
			//}
			

			//for (int i = 0; i < pRcvData->sizeBody; i++)
			//{
			////	strTemp.Format(_T("%c"), pRcvData->szBody[i]);

			//	if(pRcvData->szBody[i] == 0X04 || pRcvData->szBody[i] == 0X08 )
			//		strTemp.Format(_T("%c"), pRcvData->szBody[i]);
			//	else if(pRcvData->szBody[i] == 0X30 || pRcvData->szBody[i] == 0X00)
			//		strTemp.Format(_T("0"));
			//	else
			//	{
			//		char* pchar = &pRcvData->szBody[i];
			//		int nLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pchar, -1, NULL, NULL);

			//		wchar_t szWide[100];
			//		szWide[0] = 0x00;

			//		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pchar, strlen(pchar) + 1, szWide, nLen);



			//		//strTemp.Format(_T("%s"), (TCHAR)pRcvData->szBody[i]);
			//	}
			//		

			//	strMessage += strTemp; 
			//}

		//	CString strMessage(pRcvData->szBody);
			//	CString strLog = _T("[RECV - UTG] : ") + strMessage;
			CString strLog;
			strLog.Format(_T("[%s] : %s"), GetName(), strMessage);

			//		CLogWrapper::GetInstance()->KMLOG(SYST, strLog);
			m_pLogFileAccessor->KMLOG(RECV, strLog);

			strMessage.TrimRight();
			strMessage.Replace(_T(" "), _T(""));
			if (strMessage.IsEmpty() == true)
				return;

			std::vector<CString> vecPacketCommandPool = GetExeHextoStr2(strMessage);

			for (size_t i = 0; i < vecPacketCommandPool.size(); ++i)
			{
				const CString strCommand = vecPacketCommandPool[i];

				strLog.Format(_T("[%s] : %s"), GetName(), strCommand);
				m_pLogFileAccessor->KMLOG(OPER, strLog);

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
					}

					else if (strCommand.Find(_T("CLASS.END")) >= 0)	// 검사 결과 확인
					{
						CString strInnerID, strParam;
						CStringArray strCodeArray;
						strCodeArray.RemoveAll();

						int cnt = 0;
						// FDB.A1.CLASS.END.InnerID.CELLID.CODENUM.Result.DetailResult
						while (AfxExtractSubString(strParam, strCommand, cnt, '.'))  // 문자를 잘라준다. (AfxExtractSubString = strTok)
						{
							strCodeArray.Add(strParam);
							cnt++;
						}

						// InnerID
						if (strCodeArray.GetCount() > 4)
							strInnerID.Format(_T("%s"), strCodeArray.GetAt(4).GetString());

						//			_tcscpy_s(m_pSystem->stLotInfo.m_szResultID[m_Type], 32, strCodeArray.GetAt(4).GetString());

								//if (m_bQualityWait) {
								//	//화질검사대기 03
								//	for (int i = 0; i < strCodeArray.GetCount(); i++) {
								//		strInnerID = strCodeArray.GetAt(i).GetString();
								//		QualityWaitRun(strInnerID);
								//	}
								//}
								//else {
								//	// 결과파일 복사 and update Data
								//	GetMeasData(m_iSeqItemIndex);

								//	SetEvent(m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_OK]);
								//}

						TossResultFile(strInnerID/*m_pShareMemory->GetSharedMemory()->stLotInfo.m_szInnerID[m_iDeviceIndex]*/, m_pCommTcpIpFactoryMgr->m_nAreaNum[m_iDeviceIndex]);

						SetEvent(m_hRecvDEvent[enumRecvEventID_INSP_COMPLETE_OK]);
					}

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

CLogFileAccessor* CCommTcpIpObjectAngle::GetLogFileAccessorPtr()
{
	return m_pLogFileAccessor;
}

int CCommTcpIpObjectAngle::WriteRecipeFile(void)
{
	TCHAR szValue[MAX_PATH];
	TCHAR	szGlassExistFile[MAX_PATH];
	ZeroMemory(szValue, MAX_PATH);

	CString str;
	
	str.Format(_T("%s%s"), m_strNetRcpDrive, dfDEFAULT_CHANGERCP_FILE_PATH);
//	str.Format(_T("%s%s"), m_pSystemConfigMgr->GetConfigFolderPath(), dfDEFAULT_CHANGERCP_FILE_PATH);

	_tcscpy_s(szGlassExistFile, _countof(szGlassExistFile), str);

	str.Format(_T("%s"), m_pRecipeEditMgr->GetRecipeID());

	BOOL bRet = FALSE;
	bRet = gf_WriteValue(_T("Current Recipe"), _T("Recipe Name"), str, szGlassExistFile);
	return bRet;
}

int CCommTcpIpObjectAngle::CopyRecipeFile(void)
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

BOOL CCommTcpIpObjectAngle::TossResultFile(CString strInnerID, int nArea)
{
	//RawData 복사 진행
	CString strNewPath, strNewFile, strRawFile;
	CString strIDFile;
	CString strTemp;

	strIDFile.Format(_T("%s.ini"), strInnerID);

	// 복사할 경로 위치 확인 필요
	strNewPath.Format(_T("%sRawData\\%s\\%s\\%s\\"), CSystemParamShare::GetInstance()->GetResultFolderPath(), gf_GetCurDayString(), CSystemParamShare::GetInstance()->GetGlassID(), _T("ANGLE"));
	strNewFile.Format(_T("%s%s"), strNewPath, strIDFile);
	//	strNewFile.Format(_T("%sRawData\\%s\\%s\\%s"), CSystemParamShare::GetInstance()->GetResultFolderPath(), gf_GetCurDayString(), CSystemParamShare::GetInstance()->GetGlassID(), strIDFile);
	strRawFile.Format(_T("%s\\%s"), m_strNetDrive, strIDFile);

	//화질 제외 결과파일 생성될때까지 기다리기
	strTemp.Format(_T("%s\\%s"), m_strNetDrive, strIDFile);
	//CmFileFindTimeOut(strTemp, 5000);

	m_pCommTcpIpFactoryMgr->CmFileFindTimeOut(strTemp, DEFINE_PARAMETERS_WAIT_TIME);

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

	resultData.nMeasMode = PGM_MODE_ANGLE; //확인 필요
	resultData.iMeasArea = nArea;

//	_tcscpy(resultData.szGlassID, CSystemParamShare::GetInstance()->GetGlassID());
	_tcsncpy_s(resultData.szLotID, _countof(resultData.szLotID), CSystemParamShare::GetInstance()->GetLotID(), _TRUNCATE);	// LYS 2026-10-14 recipe ID length: truncate into RSRITEMDATA szLotID[32] (unchecked _tcscpy overran into szGlassID)
	_tcsncpy_s(resultData.szGlassID, _countof(resultData.szGlassID), CSystemParamShare::GetInstance()->GetGlassID(), _TRUNCATE);	// LYS 2026-10-14 recipe ID length: truncate into RSRITEMDATA szGlassID[32] (unchecked _tcscpy overran into szRecipeID)
	_tcsncpy_s(resultData.szRecipeID, _countof(resultData.szRecipeID), CSystemParamShare::GetInstance()->GetRecipeID(), _TRUNCATE);	// LYS 2026-10-14 recipe ID length: truncate into RSRITEMDATA szRecipeID[32] (unchecked _tcscpy overran into szResultPath)

	_tcscpy(resultData.szResultFile, strIDFile /*strInnerID*/);

	_tcscpy(resultData.szResultPath, strNewPath);

	CResultMgr::GetInstance()->Updata_Data(&resultData);

	// LYS 2026-10-01 Hybrid Phase 4 (4-4) hybrid only: upload line with row / cell ID / segment. Receive thread, but the step waits for
	//   INSP_COMPLETE, so the segment is the measured one. 1 segment: tag "" -> no line
	CString strSegTag = (m_pRecipeEditMgr != nullptr) ? m_pRecipeEditMgr->GetSegLogTag(TRUE) : CString();
	if (!strSegTag.IsEmpty())
	{
		CString strSegLog;
		strSegLog.Format(_T("[%s] : TossResultFile Updata, %s row(%d) cell(%s)%s"), (LPCTSTR)GetName(), (LPCTSTR)strInnerID, nArea,
			m_pShareMemory->GetSharedMemory()->stLotInfo.m_szCellID[m_iDeviceIndex], (LPCTSTR)strSegTag);
		m_pLogFileAccessor->KMLOG(OPER, strSegLog);
	}

	return TRUE;
}