#include "pch.h"
#include "McDaddressScanner.h"

//#include "McProtocolCtrl.h"
#include "..\..\common\XLS\XLEzAutomation.h"
#include "..\..\LogFunction\LogAccessor\LogFileAccessor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CMcDaddressScanner::CMcDaddressScanner(CString strDefaultFilePath, CMcProtocolCtrl *pMcProtocolCtrl)
//	: CMcAddressScannerImpl(strDefaultFilePath, pMcProtocolCtrl)
CMcDaddressScanner::CMcDaddressScanner(CString strLoggerFolderPath, CString strDefaultFilePath)
	: CMcAddressScannerImpl(strDefaultFilePath)
{
	m_strAddressName = _T("D_Address");

	//ReadFileCSV(m_strDefaultFilePath);

	// 로그 생성하기
	const CString strFolderPath = strLoggerFolderPath + _T("\\LogFile\\");
	const CString strLogFile = strFolderPath + m_strAddressName + _T(".log");
	MakeDir(strFolderPath);
	m_pLogFileAccessor->Initialize(strLogFile);
	//////

	m_strQueueMessage.Empty();
	m_strOldData.Empty();
}

CMcDaddressScanner::~CMcDaddressScanner()
{

}

bool CMcDaddressScanner::SendCommand()
{
	if (IsConnected() == false)
		return false;

	int		stadd = 1;
	int		stadd8 = 8;
	TCHAR	temp[20];
	TCHAR	str[100];
	str[0] = 0;

	_tcscat_s(str, _countof(str), _T(PLC_CMD_ETHERNET_WORD_READ_D));		//WORD

	short bit = 1;

	_stprintf_s(temp, _countof(temp), _T("D*%06d"), dfDADDRESS_RD_START);		_tcscat_s(str, _countof(str), temp); //M*001001, ex)address 1001		// 시작번지

	int nReadCount = dfDADDRESS_TOTAL_COUNT;
	nReadCount = (int)floor(dfDADDRESS_TOTAL_COUNT);

	_stprintf_s(temp, _countof(temp), _T("%04X"), nReadCount);		_tcscat_s(str, _countof(str), temp); //stadd 1 ==> 0001					// 읽을 비트 수

	size_t n = _tcsclen(str);					//str[n]   = PLC_CHAR_EOT;
	str[n] = 0;								//n = 42

	CString strCommand;
	strCommand.Format(_T("%s"), str);
	//SetMemoryBit(str);
	SendPacket(strCommand, false, false);

	return true;
}

long CMcDaddressScanner::Initialize(bool bRunMode, CString strIP, long nPort)
{
	if (IsConnected() == true)
	{
		//이미 연결
		return 0;
	}

	SetNetInfo(bRunMode, strIP, nPort);
	bool lRet = CCommEthFunction::Initialize();

	//   if(m_pDlgMcProtocolTcpIp == nullptr)
	//{
	//	m_pDlgMcProtocolTcpIp = new CDlgMcProtocolTcpIp;
	//}

	//m_pDlgMcProtocolTcpIp->Create(IDD_DLG_McProtocolTcpIp);
	//m_pDlgMcProtocolTcpIp->CenterWindow();
	//m_pDlgMcProtocolTcpIp->ShowWindow(SW_HIDE);
	//
	//long lRet = m_pDlgMcProtocolTcpIp->Initialize(iStationNum);

	if (lRet == true)
	{
		//	m_bMcProtocolIni = true;

		CString strLog;
		strLog.Format(_T("Initialize Start : %s"), m_strAddressName);
		m_pLogFileAccessor->KMLOG(SYST, strLog, false);
	}
	else
	{
		//	m_bMcProtocolIni = false;
	}

	return lRet;
}

long CMcDaddressScanner::Close()
{
	long lRet = MC_PROTOCOL_SUCCESS;

	CCommEthFunction::Finalize();

	//if(m_pDlgMcProtocolTcpIp)
	//{
	// lRet = m_pDlgMcProtocolTcpIp->Close();

	// m_pDlgMcProtocolTcpIp->DestroyWindow();
	//    delete m_pDlgMcProtocolTcpIp;
	//    m_pDlgMcProtocolTcpIp = nullptr;
	//}

	return lRet;
}

bool CMcDaddressScanner::IsConnected()
{
	return CCommEthFunction::IsConnected();
}

void CMcDaddressScanner::DisConnect()
{
	CCommEthFunction::CloseConnect();
}

void CMcDaddressScanner::ThreadRun()
{
    if (IsConnected() == true)
    {
		SendCommand();

		Sleep(200);
        //memset(m_lMcProtocolReadData, 0, sizeof(long) * dfDADDRESS_TOTAL_COUNT);

        ////m_pMcProtocolCtrl->GetMemoryBitBlock(_T("D11500"), dfDADDRESS_TOTAL_COUNT, m_lMcProtocolReadData);

        //memcpy(m_lMcProtocolReadData_clone, m_lMcProtocolReadData, sizeof(long) * dfDADDRESS_TOTAL_COUNT);
	}
	else
	{
		memset(m_lMcProtocolReadData, 0, sizeof(long) * dfDADDRESS_TOTAL_COUNT);
		memset(m_lMcProtocolReadData_clone, 0, sizeof(long) * dfDADDRESS_TOTAL_COUNT);
	}
}

bool CMcDaddressScanner::ReadFileCSV(CString strFilename)
{
	m_vecRData.Clear();
	//m_vecWData.Clear();
	m_vecRData.vecData.resize(dfDADDRESS_HALF_COUNT * 2);
	//m_vecWData.vecData.resize(dfDADDRESS_HALF_COUNT);

	std::vector<CString> vecErrorMemory;

	CStdioFile File(strFilename, CFile::modeRead);

	while (true)
	{
		CString strLine;

		if (File.ReadString(strLine))
		{
			if (strLine.Left(1) != _T("D"))
				continue;

			int iEnd = strLine.Find(_T(","));
			UINT nAddress = _ttoi(strLine.Mid(1, iEnd));

			CString strTemp;
			strTemp.Format(_T("D%d"), nAddress);
			strLine.Delete(0, iEnd + 1);

			if (dfDADDRESS_RD_START <= nAddress && nAddress < dfDADDRESS_LIMIT_COUNT)
			{
				if (strLine.IsEmpty() == false)
				{
					//if (nAddress >= dfDADDRESS_WT_START)		//Read
					//{
					//	nAddress -= dfDADDRESS_RD_START;
					//	
					//	m_vecRData.bEnable = true;

					//	m_vecRData.vecData[nAddress].nAddress = nAddress;
					//	m_vecRData.vecData[nAddress].strMapNumber = strTemp;
					//	m_vecRData.vecData[nAddress].strDescription = strLine;
					//	m_vecRData.vecData[nAddress].bUsable = strLine.IsEmpty() ? false : true;
					//}
					//else									//Write
					//{
					//	nAddress -= dfDADDRESS_WT_START;

					//	m_vecWData.bEnable = true;
					//	
					//	m_vecWData.vecData[nAddress].nAddress = nAddress;
					//	m_vecWData.vecData[nAddress].strMapNumber = strTemp;
					//	m_vecWData.vecData[nAddress].strDescription = strLine;
					//	m_vecWData.vecData[nAddress].bUsable = strLine.IsEmpty() ? false : true;					
					//}
					nAddress -= dfDADDRESS_RD_START;

					m_vecRData.bEnable = true;

					m_vecRData.vecData[nAddress].nAddress = nAddress;
					m_vecRData.vecData[nAddress].strMapNumber = strTemp;
					m_vecRData.vecData[nAddress].strDescription = strLine;
					m_vecRData.vecData[nAddress].bUsable = strLine.IsEmpty() ? false : true;
				}
			}
			else
			{
				vecErrorMemory.push_back(strTemp);
			}
		}
		else   // End file
		{
			break;
		}
	}

	File.Close();

	// Error Message & Log
	if (0 < vecErrorMemory.size())
	{
		CString strLog;
		strLog.Format(_T("There are '%d' DADDRESS Numbers Out of range.\nPlease Refer to the Log."), vecErrorMemory.size());
		AfxMessageBox(strLog);

		m_pLogFileAccessor->KMLOG(OPER, _T("D Address memory numbers out of range List."));
		for (size_t i = 0; i < vecErrorMemory.size(); i++)
		{
			m_pLogFileAccessor->KMLOG(OPER, _T("Error Address : ") + vecErrorMemory[i]);
		}
		
		return false;
	}

	return true;
}

bool CMcDaddressScanner::ReadFileXLS(CString strFileName)
{
	m_vecRData.Clear();
	//m_vecWData.Clear();
	m_vecRData.vecData.resize(dfDADDRESS_HALF_COUNT * 2);
	//m_vecWData.vecData.resize(dfDADDRESS_HALF_COUNT);

    std::vector<CString> vecErrorMemory;

	CFile file(strFileName, CFile::shareDenyNone );
	CArchive ar(&file, CArchive::load);
	CXLEzAutomation ListExcel(FALSE);
	if(ListExcel.OpenExcelFile(strFileName) == TRUE)
	{
		UINT iaccess_offset = 2;
		for (UINT i = iaccess_offset; i < dfDADDRESS_TOTAL_COUNT + iaccess_offset; i++)
		{
			CString strAddress = ListExcel.GetCellValue(1, i);
			CString strDescription = ListExcel.GetCellValue(2, i);

			CString strTemp = strAddress;
			strTemp.Replace(_T("D"), _T(""));
			UINT nAddress = _ttoi(strTemp);

			if (dfDADDRESS_RD_START <= nAddress && nAddress < dfDADDRESS_LIMIT_COUNT)
			{
				if (strTemp.IsEmpty() == false)
				{
					//if (nAddress < dfDADDRESS_WT_START)
					//{
					//	nAddress -= dfDADDRESS_RD_START;
					//	
					//	m_vecRData.bEnable = true;

					//	m_vecRData.vecData[nAddress].nAddress = nAddress;
					//	m_vecRData.vecData[nAddress].strMapNumber = strTemp;
					//	m_vecRData.vecData[nAddress].strDescription = strDescription;						
					//	m_vecRData.vecData[nAddress].bUsable = strDescription.IsEmpty() ? false : true;
					//}
					//else
					//{
					//	nAddress -= dfDADDRESS_WT_START;

					//	m_vecWData.bEnable = true;

					//	m_vecWData.vecData[nAddress].nAddress = nAddress;
					//	m_vecWData.vecData[nAddress].strMapNumber = strTemp;
					//	m_vecWData.vecData[nAddress].strDescription = strDescription;
					//	m_vecWData.vecData[nAddress].bUsable = strDescription.IsEmpty() ? false : true;
					//}
					nAddress -= dfDADDRESS_RD_START;

					m_vecRData.bEnable = true;

					m_vecRData.vecData[nAddress].nAddress = nAddress;
					m_vecRData.vecData[nAddress].strMapNumber = strTemp;
					m_vecRData.vecData[nAddress].strDescription = strDescription;
					m_vecRData.vecData[nAddress].bUsable = strDescription.IsEmpty() ? false : true;
				}
			}
			else
			{
				if (strTemp.IsEmpty() == false)
				{
					vecErrorMemory.push_back(strTemp);
				}
			}
		}
	}

	ListExcel.ReleaseExcel();
	file.Close();
	
	// Error Message & Log
	if (0 < vecErrorMemory.size())
	{
		CString strLog;
		strLog.Format(_T("There are '%d' DADDRESS Numbers Out of range.\nPlease Refer to the Log."), vecErrorMemory.size());
		AfxMessageBox(strLog);

		m_pLogFileAccessor->KMLOG(OPER, _T("M Address memory numbers out of range List."));
		for (size_t i = 0; i < vecErrorMemory.size(); i++)
		{
			m_pLogFileAccessor->KMLOG(OPER, _T("Error Address : ") + vecErrorMemory[i]);
		}
		
		return false;
	}

	return true;
}

bool CMcDaddressScanner::ReadFileVerify(CString strFileName)
{
	CStdioFile File(strFileName, CFile::modeRead);

	bool bRet = false;

	while (true)
	{
		CString strLine;

		if (File.ReadString(strLine))
		{
			if (strLine.Left(1) != _T("D"))
				continue;

			int iEnd = strLine.Find(_T(","));
			UINT nAddress = _ttoi(strLine.Mid(1, iEnd));

			if (dfDADDRESS_RD_START <= nAddress && nAddress < dfDADDRESS_LIMIT_COUNT)
			{
				bRet = true;
				continue;
			}
			else
			{
				File.Close();
				return bRet;
			}
		}
		else   // End file
		{
			break;
		}
	}

	File.Close();
	return bRet;
}

bool CMcDaddressScanner::SetDefaultFile(CString strFileName)
{
	if(CopyFile(strFileName, m_strDefaultFilePath, FALSE) == TRUE)
	{
		return ReadFileCSV(m_strDefaultFilePath);
	}
	
	return false;
}

bool CMcDaddressScanner::SetMemoryBit(short addr, long val, long nBit/* = 1*/)
{
	if (IsConnected())
	{
		CString _addr;
		_addr.Format(_T("D%d"), addr);

		long Rtn = 1;// m_pMcProtocolCtrl->SetMemoryBit(_addr, val);

		if (Rtn == 0)
		{
			CString strLog;
			strLog.Format(_T("Fail : SetMemoryBit() Address Bit : %d"), addr);
			m_pLogFileAccessor->KMLOG(OPER, strLog);
		}
		else
		{
			return true;
		}
	}
	else
	{
		m_pLogFileAccessor->KMLOG(OPER, _T("Fail : MXComponent(MX_PLC_M) is Not Connect"));
	}
	return false;
}

bool CMcDaddressScanner::SetMemoryBit(CString strAddr, long val, long nBit/* = 1*/)
{
	if (IsConnected())
	{
		long Rtn = 1;//m_pMcProtocolCtrl->SetMemoryBit(strAddr, bValue);

		if (Rtn == 0)
		{
			CString strLog;
			strLog.Format(_T("Fail : SetMemoryBit() Address : %s"), strAddr);
			m_pLogFileAccessor->KMLOG(OPER, strLog);
		}
		else
		{
			return true;
		}
	}
	else
	{
		m_pLogFileAccessor->KMLOG(OPER, _T("Fail : MXComponent(MX_PLC_M) is Not Connect"));
	}
	return false;
}

long CMcDaddressScanner::GetAddressImportCount()
{
	return dfDADDRESS_HALF_COUNT;
}

long CMcDaddressScanner::GetAddressExportCount()
{
	return dfDADDRESS_HALF_COUNT;
}

bool CMcDaddressScanner::GetAddressImportDataValue(UINT Index)
{
	if ((dfDADDRESS_HALF_COUNT <= Index) && (Index < dfDADDRESS_TOTAL_COUNT))
	{
		return (m_lMcProtocolReadData_clone[Index] == 1) ? true : false;
	}

    return false;
}

bool CMcDaddressScanner::GetAddressExportDataValue(UINT Index)
{
	if ((0 <= Index) && (Index < dfDADDRESS_HALF_COUNT))
	{
		return (m_lMcProtocolReadData_clone[Index] == 1) ? true : false;
	}

    return false;
}

SADDRESSMAPDATA *CMcDaddressScanner::GetAddressImportDataPtr()
{
	return &m_vecRData;
}

SADDRESSMAPDATA *CMcDaddressScanner::GetAddressExportDataPtr()
{
	//return &m_vecWData;
	return &m_vecRData;
}

void CMcDaddressScanner::xNetCallbackRecieveData(LPVOID ud)
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
		CString strMessage(pRcvData->szBody);
		strMessage.Replace(_T(" "), _T(""));
		if (strMessage.IsEmpty() == true)
			return;

		CString strLog = _T("[RECV - DAddress] : ") + strMessage;
		m_pLogFileAccessor->KMLOG(RECV, strLog);

		//CString strMessage(pRcvData->szBody);
		//CString strLog = _T("[RECV - DAddress] : ") + strMessage;
		//m_pLogFileAccessor->KMLOG(RECV, strLog);

		////	CLogWrapper::GetInstance()->KMLOG(SYST, strLog);
		//strMessage.Replace(_T(" "), _T(""));
		//if (strMessage.IsEmpty() == true)
		//	return;

		//---------------------------------------------------------------------------
		// 패킷 단위로 응답에 대한 처리를 해야 한다.
		// socket 에 데이터가 진입하는데 2개 이상 붙어서 1개의 event 로 들어온다.
		// 모든 패킷에 대해서 command 단위로 분리해서 수행해야 한다.
		//---------------------------------------------------------------------------
		std::vector<CString> vecPacketCommandPool = GetCommand(strMessage);

		for (size_t iMsg = 0; iMsg < vecPacketCommandPool.size(); ++iMsg)
		{
			//		const CString strCommand = vecPacketCommandPool[i];

			CString strCommand = vecPacketCommandPool[iMsg];

			m_pLogFileAccessor->KMLOG(RECV, strMessage);

			double dPLCData[dfDADDRESS_TOTAL_COUNT];
			ZeroMemory(&dPLCData, sizeof(dPLCData));

			char* StopStr;
			CString strTempH, strTempL, str;
			int toByte = 0, tNum = 0;
			int j = 0, k = 0;
			int nCnt = 0;
			int iCnt = (int)floor(dfDADDRESS_TOTAL_COUNT);
			if (iCnt % 2)
			{
				strCommand += "OOOO";
				nCnt = (iCnt + 1) / 2;
			}
			else
			{
				nCnt = iCnt / 2;
			}

			for (j = 0; j < nCnt; j++)
			{
				strTempH = strCommand.Mid((8 * j), 4);			// 4
				strTempL = strCommand.Mid((8 * j) + 4, 4);			// 4


				USES_CONVERSION;
				char	str1Word[4096];
				memset(str1Word, NULL, 4096);

				strcpy_s(str1Word, T2A(strTempL));
				strcat_s(str1Word, T2A(strTempH));

				unsigned int CurData = strtoul(str1Word, &StopStr, 16);

				float* RealData;
				RealData = (float*)&CurData;

				dPLCData[k] = *RealData;
				k++;
			}
		}
	}
}

std::vector<CString> CMcDaddressScanner::GetCommand(CString &strData)
{
	CString strMessage = strData;

	int iCnt = 0;
	int nReciveSize = 0;
	int nHeaderPos = 0;
	int nReplySize = 0;

	std::vector<CString> vecResult;

	if (m_strOldData == strMessage)
	{
		return vecResult;
	}
	else
	{
		m_strOldData = strMessage;
	}

	// Header
	TCHAR	temp[20];
	TCHAR	str[100];
	str[0] = 0;

	_tcscat_s(str, _countof(str), _T("D00000FF03FF"));

	iCnt = (int)floor(dfDADDRESS_TOTAL_COUNT);

	nReciveSize = iCnt * 4 + PLC_CMD_HEADER_LENGTH;
	_stprintf_s(temp, _countof(temp), _T("%06X"), (iCnt * 4) + 4);	_tcscat_s(str, _countof(str), temp);


	_stprintf_s(temp, _countof(temp), _T("%04X"), 0);	    _tcscat_s(str, _countof(str), temp);
	size_t n = _tcsclen(str);					//str[n]   = PLC_CHAR_EOT;
	str[n] = 0;

	CString strCommand;
	strCommand.Format(_T("%s"), str);

	while (1)
	{
		if (strMessage.IsEmpty() == true)
			break;

		nReplySize = strMessage.GetLength();

		if (nReplySize >= nReciveSize)
		{
			nHeaderPos = strMessage.Find(strCommand);

			if (nHeaderPos > -1)		// Header OK	
			{
				if (nReplySize >= nHeaderPos + nReciveSize)
				{
					const int ipos = strMessage.Find(strCommand);
					if (ipos < 0)
						break;

					//Word 사용 시 추가 필요함
					CString strParam = strMessage;
					strParam = strParam.Mid(ipos + PLC_CMD_HEADER_LENGTH, iCnt * 4);

					if (strParam.IsEmpty() == true)
						break;

					vecResult.push_back(strParam);

					strMessage = strMessage.Mid(nReciveSize);
				}
			}
		}
		else
		{
			//로그 기록 필요
			vecResult.push_back(strMessage);
			strMessage.Empty();
		}
	}

	return vecResult;
}