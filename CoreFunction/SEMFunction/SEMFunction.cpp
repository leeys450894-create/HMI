#include "pch.h"
#include "SEMFunction.h"

#include "..\common\IniHelper.h"
#include "..\common\TimeChecker.h"
#include "..\Comm232Function\Comm232Config.h"
#include "..\CommEthFunction\CommEthFunction.h"


// Delimeter
#define		dfCR						0x0D
#define     dfLF						0x0A
#define     dfCRLF						_T("\r\n")

inline BOOL MakeDir(CString sPath)
{
	if (sPath == _T(""))
	{
		return FALSE;
	}

	if (sPath[1] != ':' || sPath[2] != '\\')
	{
		return FALSE;
	}

	int nFind = 2;
	while ((nFind = sPath.Find(_T("\\"), nFind + 1)) >= 0)
	{
		CString sTem = sPath.Left(nFind);
		CreateDirectory(sTem, NULL);
	}

	CreateDirectory(sPath, NULL);

	return TRUE;
}

CSEMFunction::CSEMFunction(CString strAppPath, CString strConfigPath, CLogFileAccessor *pLogFileAccessor)
	: m_pIniHelper(new CIniHelper)
	, m_pLogFileAccessor(pLogFileAccessor)
	, m_pComm232(nullptr)
	, m_pCommEth(nullptr)
	, m_nInterface(1)
	, m_bIsConnet(false)
	, m_bSetAlarm(false)
{ 
	//// 로그 생성하기
	//const CString strFolderPath = strLogFilePath + _T("SEM\\");
	//const CString strLogFile = strFolderPath + _T("SEM.log");
	//MakeDir(strFolderPath);
	//m_pLogFileAccessor->Initialize(strLogFile);
	////////

	m_strConfigPath.Format(_T("%s"), strConfigPath);
	m_pIniHelper->SetFilePath(m_strConfigPath);
	LinkIniFile(false);

	CString strLog;
	strLog.Format(_T("SEM Function Initialize Start"));
	m_pLogFileAccessor->KMLOG(SYST, strLog, true);

	m_vecData.clear();

	for (size_t i = 0; i < 20; i++)
	{
		m_stSEM_Data[i].nID = 0;
		m_stSEM_Data[i].fVal = 0;
		_tcscpy_s(m_stSEM_Data[i].szSVName, _countof(m_stSEM_Data[i].szSVName), _T(""));
	}

	m_stSEM_Test.nID = 0;
	m_stSEM_Test.fVal = 0;
	_tcscpy_s(m_stSEM_Test.szSVName, _countof(m_stSEM_Test.szSVName), _T(""));
}


CSEMFunction::~CSEMFunction()
{
	CloseDevice();
}

//------------------------------------------------------------------
// 외부 접근 가능 함수
//------------------------------------------------------------------
bool CSEMFunction::LinkIniFile(bool bSave)
{
	CString strCategory;
	strCategory.Format(_T("%s"), _T("SEM_CONTROL_MGR"));

	m_pIniHelper->Link(bSave, strCategory, _T("USE"), &m_bUse, false);
	m_pIniHelper->Link(bSave, strCategory, _T("OUT_OF_SERVICE"), &m_bOutOfService, false);
	m_pIniHelper->Link(bSave, strCategory, _T("DEVICE_TYPE"), &m_strDeviceType, (CString)_T("RS232")); // Ethernet
	m_pIniHelper->Link(bSave, strCategory, _T("ALARMTIME"), &m_nAlarmTime, 8);

	if (m_strDeviceType.CompareNoCase(_T("RS232")) == 0)
		m_nInterface = 1;
	else
		m_nInterface = 0;

	//
	m_pIniHelper->Link(bSave, strCategory, _T("RS232_PORTID"), &m_strComPort, (CString)_T("COM1"));
	m_pIniHelper->Link(bSave, strCategory, _T("RS232_BAUD_RATE"), &m_strBaudRate, (CString)_T("115200"));

	m_pIniHelper->Link(bSave, strCategory, _T("TCPIP_IP"), &m_strIPNumber, (CString)_T("192.168.100.200"));
	m_pIniHelper->Link(bSave, strCategory, _T("TCPIP_PORT"), &m_strPortNumber, (CString)_T("9001"));
	//

	return m_pIniHelper->IsReadEnable();
}

bool CSEMFunction::Initialize()
{
	CString strCategory;
	strCategory.Format(_T("%s"), _T("SEM_CONTROL_MGR"));

	bool bRet = false;

	CString strCommand;
	strCommand.Format(_T("$START"));

	if (m_nInterface == 1)	// RS-232
	{
		if (m_pComm232 == nullptr)
		{
			m_pComm232 = new CComm232Config(m_strConfigPath, strCategory);
		}

		SetLogFileAccessor(m_pLogFileAccessor);
		SetConfig(m_pComm232);
		SetCallbackFunction(this, CallbackRecieveData);

	 	if (OpenPort() == true)
		{
			m_bSetAlarm = true;

			bRet = SendCommand(strCommand + dfCRLF);
		}
	}
	else
	{
		if (m_pCommEth == nullptr)
		{
			m_pCommEth = new CCommEthFunction();
		}

		const CString &strIP = m_strIPNumber;
		const long &nPort = _ttoi(m_strPortNumber);
		m_pCommEth->SetNetInfo(false, strIP, nPort);

		m_pCommEth->SetCallbackFunction(this, CallbackRecieveData);

		if (m_pCommEth->Initialize(false) == true)
		{
			m_bSetAlarm = true;

			Sleep(500);

			bRet = m_pCommEth->SendCommand(strCommand + dfCRLF);
		}
	}

	CString strLog;
	strLog.Format(_T("%d, %s"), bRet, strCommand);
	m_pLogFileAccessor->KMLOG(SEND, strLog, true);

	m_bIsConnet = bRet;

	return bRet;
}

bool CSEMFunction::StartDevice()
{
	bool bRet = false;

	CString strCommand;
	strCommand.Format(_T("$START"));

	m_bSetAlarm = true;

	if (m_nInterface == 1)	// RS-232
	{
		if (OpenPort() == true)
		{
			bRet = SendCommand(strCommand + dfCRLF);
		}
	}
	else
	{
		bRet = m_pCommEth->SendCommand(strCommand + dfCRLF);
	}

	CString strLog;
	strLog.Format(_T("%d, %s"), bRet, strCommand);
	m_pLogFileAccessor->KMLOG(SEND, strLog, true);

	return bRet;
}

bool CSEMFunction::CloseDevice()
{
	bool bRet = false;

	CString strCommand;
	strCommand.Format(_T("$END"));

	if (m_nInterface == 1)
	{
		bRet = SendCommand(strCommand + dfCRLF);

		if (m_pComm232 != nullptr)
		{
			delete m_pComm232;
			m_pComm232 = nullptr;
		}
	}
	else
	{	
		bRet = m_pCommEth->SendCommand(strCommand + dfCRLF);

		if (m_pCommEth != nullptr)
		{
			m_pCommEth->Finalize();
			delete m_pCommEth;
			m_pCommEth = nullptr;
		}
	}  

	CString strLog;
	strLog.Format(_T("%d, %s"), bRet, strCommand);
	m_pLogFileAccessor->KMLOG(SEND, strLog, true);

	m_bIsConnet = false;

	return true;
}

bool CSEMFunction::IsConnected()
{
	return 	m_bIsConnet;
}

bool CSEMFunction::IsReceive()
{
	return 	!m_bSetAlarm;
}

int CSEMFunction::GetAlarmTime()
{
	return m_nAlarmTime;
}

void CSEMFunction::CallbackRecieveData(LPVOID ud1, LPVOID ud2)
{
	CSEMFunction *pThis = (CSEMFunction *)ud1;
	if (pThis)
	{
		pThis->CallbackRecieveData(ud2);
	}
}
void CSEMFunction::CallbackRecieveData(LPVOID ud)
{
	if (m_nInterface == 1)
	{
		CString strMessage((LPCTSTR)(ud));

		strMessage.Replace(_T(" "), _T(""));
		if (strMessage.IsEmpty() == true)
			return;

		AnalysisReceive(strMessage);
	}
	else
	{
		LPRECEIVEDDATA pRcvData = (LPRECEIVEDDATA)ud;

		CString strMessage(pRcvData->szBody);

		strMessage.Replace(_T(" "), _T(""));
		if (strMessage.IsEmpty() == true)
			return;

		AnalysisReceive(strMessage); 
	}
}

void CSEMFunction::AnalysisReceive(CString strRcv)
{
	CString strLog = _T("[RECV - SEM] : ") + strRcv;
	m_pLogFileAccessor->KMLOG(RECV, strLog, true);

	CString strData; strData.Format(_T("%s"), strRcv);

	int nCmdStart = 0;
	int nCmdEnd = 0;

	m_bSetAlarm = false;

	while (strRcv.Find(dfCRLF) >= 0)
	{
		nCmdStart = 0;
		nCmdEnd = strRcv.Find(_T("\r\n"));
		CString strCmd;
		strCmd = strRcv.Mid(nCmdStart, nCmdEnd);

		AnalysisReceiveData(strCmd);
		strRcv.Delete(nCmdStart, nCmdEnd + 2);
	}
}


void CSEMFunction::AnalysisReceiveData(CString strData)
{
	if (strData.Find(_T(",\r\n")) < 0) strData += _T(",");

	if (strData.Find(_T("$ACK")) >= 0) return;
	else if (strData.Find(_T("$ERROR")) >= 0) return;

	if (strData.Find(_T("$SEM_TEST")) >= 0)
	{
		strData.Replace(_T("$SEM_TEST"), _T("SEM_TEST"));
		CString strSV, strID, strSVName, strSVVal;
		float fSVVal;
		int nFindSVVal, nFindnSVName, nFindSV = strData.Find(',');
		int nID;
		int nRetVal = 0;

		int nStart = strData.Find(_T("SEM_TEST"));

		int nEqSEMCnt = 0;	//20200417

		strSV = strData.Mid(nStart, nFindSV);
		strData.Delete(nStart, nFindSV + 1);
		nFindSV = strData.Find(',');

		nFindnSVName = strSV.Find('(');
		strSVName = strSV.Mid(0, nFindnSVName);

		nFindSVVal = strSV.Find('=');
		strSVVal = strSV.Mid(nFindSVVal + 1, strSV.GetLength() - (nFindSVVal + 1));
		fSVVal = (float)_ttof(strSVVal);

		strID = strSV.Mid(nFindnSVName + 1, 5);
		nID = _ttoi(strID);

		m_stSEM_Test.nID = nID;
		m_stSEM_Test.fVal = fSVVal;

		_tcscpy_s(m_stSEM_Test.szSVName, _countof(m_stSEM_Test.szSVName), strSVName);
	}

	if (strData.Find(_T("$N01_CDA")) >= 0)
	{
		CString strFlag;
		strFlag.Format(_T("%s"), _T("N01_CDA"));
		CString strData2;

		int nStart = strData.Find(_T("$N01_CDA"));
		int nEnd = strData.ReverseFind(',');
		strData2 = strData.Mid(nStart, nEnd);
		strData.Delete(nStart, nEnd + 1);

		strData2.Remove('$');
		CString strSV, strID, strSVName, strSVVal;
		float fSVVal;
		int nFindSVVal, nFindnSVName, nFindSV = strData2.Find(',');
		int nID;
		int nRetVal = 0;

		int nEqSEMCnt = 0;	//20200417

		stSEMDATA	stResult;

		while (nFindSV >= 0)
		{
			strSV = strData2.Mid(0, nFindSV);
			strData2.Delete(0, nFindSV + 1);
			nFindSV = strData2.Find(',');

			nFindnSVName = strSV.Find('(');
			strSVName = strSV.Mid(0, nFindnSVName);

			nFindSVVal = strSV.Find('=');
			strSVVal = strSV.Mid(nFindSVVal + 1, strSV.GetLength() - (nFindSVVal + 1));
			fSVVal = (float)_ttof(strSVVal);

			strID = strSV.Mid(nFindnSVName + 1, 5);
			nID = _ttoi(strID);

			memset(&stResult, NULL, sizeof(stSEMDATA));

			switch (nID)
			{
			case 61401:
				stResult.nID = nID;
				stResult.fVal = fSVVal;
				_tcscpy_s(stResult.szSVName, _countof(stResult.szSVName), strSVName);

				m_stSEM_Data[6] = stResult;
				break;
			case 61402:
				stResult.nID = nID;
				stResult.fVal = fSVVal;
				_tcscpy_s(stResult.szSVName, _countof(stResult.szSVName), strSVName);

				m_stSEM_Data[7] = stResult;
				break;
			default:
				break;
			}
		}
	}

	////
	{
		strData.Remove('$');
		CString strSV, strID, strSVName, strSVVal;
		float fSVVal;
		int nFindSVVal, nFindnSVName, nFindSV = strData.Find(',');
		int nID;
		int nRetVal = 0;

		int nEqSEMCnt = 0;	//20200417

		stSEMDATA	stResult;
		//		m_vecData.clear();

		while (nFindSV >= 0)
		{
			strSV = strData.Mid(0, nFindSV);
			strData.Delete(0, nFindSV + 1);
			nFindSV = strData.Find(',');

			nFindnSVName = strSV.Find('(');
			strSVName = strSV.Mid(0, nFindnSVName);

			nFindSVVal = strSV.Find('=');
			strSVVal = strSV.Mid(nFindSVVal + 1, strSV.GetLength() - (nFindSVVal + 1));
			fSVVal = (float)_ttof(strSVVal);

			strID = strSV.Mid(nFindnSVName + 1, 5);
			nID = _ttoi(strID);

			memset(&stResult, NULL, sizeof(stSEMDATA));

			stResult.nID = nID;
			stResult.fVal = fSVVal;
			_tcscpy_s(stResult.szSVName, _countof(stResult.szSVName), strSVName);

			switch (nID)
			{
			case 60000:
				m_stSEM_Data[0] = stResult;
				break;
			case 60001:
				m_stSEM_Data[1] = stResult;
				break;
			case 60002:
				m_stSEM_Data[2] = stResult;
				break;
			case 60003:
				m_stSEM_Data[3] = stResult;
				break;
			case 60004:
				m_stSEM_Data[4] = stResult;
				break;
			case 60005:
				m_stSEM_Data[5] = stResult;
				break;
			default:
				break;
			}
		}

		int i = 0;
		//		for ( i = 0; i < m_vecData.size(); i++)
		//		{
		//			if( i < 19)
		//				m_stSEM_Data[i] = m_vecData[i];
		//		}

		m_stSEM_Data[8] = m_stSEM_Test;

	}
}


bool CSEMFunction::GetData(int ID, UINT& uAddr, CString& sName, float& fVal)
{
	/*if (ID == 0)
	{
		CString str(_T("$SEM_TEST(65000)=10002.6\r\n$N01_T_PW(60000) = 399.000, N01_I_PW(60001) = 0.892, N01_R_VOL(60002) = 226.406, N01_T_VOL(60003) = 225.810, N01_R_CUR(60004) = 4.986, N01_T_CUR(60005) = 4.382\r\n$SEM_TEST(65000) = 10002.7\r\n$N01_T_PW(60000) = 399.000, N01_I_PW(60001) = 0.885, N01_R_VOL(60002) = 226.456, N01_T_VOL(60003) = 225.828, N01_R_CUR(60004) = 4.991, N01_T_CUR(60005) = 4.370\r\n$N01_CDA01_T(61401) = 33.000, N01_CDA02_T(61402) = 1241250.000,\r\n$SEM_TEST(65000) = 10002.8\r\n$N01_T_PW(60000) = 399.000, N01_I_PW(60001) = 0.890, N01_R_VOL(60002) = 226.504, N01_T_VOL(60003) = 225.928, N01_R_CUR(60004) = 4.991, N01_T_CUR(60005) = 4.406\r\n"));


		AnalysisReceive(str);
	}*/
	
	bool bRet = false;
	if (ID < 20)
	{
		uAddr = m_stSEM_Data[ID].nID;
		fVal = m_stSEM_Data[ID].fVal;
		sName.Format(_T("%s"), m_stSEM_Data[ID].szSVName);

		if (uAddr > 0)
			bRet = true;
		else
			bRet = false;

	}

	return bRet;
}

void CSEMFunction::SetConfigFile(int nInterface, int nPort, int nBaudrate, int nAlarmTime)
{
	m_nInterface = nInterface;

	if (m_nInterface == 1)	// RS-232
	{
		m_strDeviceType = _T("RS232");
	}
	else
	{
		m_strDeviceType = _T("ETHERNET");
	}

	m_nAlarmTime = nAlarmTime + 1;

	m_strComPort.Format(_T("COM%d"), nPort + 1);
	int nBaudrateDB[9] = { 1200, 2400, 4800, 9600, 19200, 28800, 38400, 57600, 115200 };
	m_strBaudRate.Format(_T("%d"), nBaudrateDB[nBaudrate]);

	LinkIniFile(true);
}