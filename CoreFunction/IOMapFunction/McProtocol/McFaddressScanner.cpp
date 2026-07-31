#include "pch.h"
#include "McFaddressScanner.h"

//#include "McProtocolCtrl.h"
#include "..\..\common\XLS\XLEzAutomation.h"
#include "..\..\LogFunction\LogAccessor\LogFileAccessor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CMcFaddressScanner::CMcFaddressScanner(CString strDefaultFilePath, CMcProtocolCtrl *pMcProtocolCtrl)
//	: CMcAddressScannerImpl(strDefaultFilePath, pMcProtocolCtrl)
CMcFaddressScanner::CMcFaddressScanner(CString strLoggerFolderPath, CString strDefaultFilePath)
	: CMcAddressScannerImpl(strDefaultFilePath) 
{
	m_strAddressName = _T("F_Address");

	//ReadFileCSV(m_strDefaultFilePath);

	// 로그 생성하기
	const CString strFolderPath = strLoggerFolderPath + _T("\\LogFile\\");
	const CString strLogFile = strFolderPath + m_strAddressName + _T(".log");
	MakeDir(strFolderPath);
	m_pLogFileAccessor->Initialize(strLogFile);
	//////

	m_iDataLast	= dfFADDRESS_TOTAL_COUNT;
	m_strQueueMessage.Empty();

	memset(m_lMcProtocolReadData, 0, sizeof(long) * dfGSADDRESS_TOTAL_COUNT);
	memset(m_lMcProtocolReadData_clone, 0, sizeof(long) * dfGSADDRESS_TOTAL_COUNT);
	m_bThreadRunStop = false;
	m_strOldData.Empty();
	m_bUseBit = false;
}

CMcFaddressScanner::~CMcFaddressScanner()
{
	;
}

bool CMcFaddressScanner::SendCommand()
{
	if (IsConnected() == false)
		return false;

	int		stadd = 1;
	int		stadd8 = 8;
	TCHAR	temp[20];
	TCHAR	str[100];
	str[0] = 0;
//	_tcscat_s(str, _countof(str), _T(PLC_CMD_ETHERNET_SINGLE_READ));		//Bit

	bool bUseBit = false;
	bUseBit = m_bUseBit;
	if (bUseBit)
	{
		_tcscat_s(str, _countof(str), _T(PLC_CMD_ETHERNET_SINGLE_READ));		//Bit
	}
	else
	{
		_tcscat_s(str, _countof(str), _T(PLC_CMD_ETHERNET_WORD_READ_D));		//Word
	}

	short bit = 1;

	//4000번 부터 일괄적으로 읽을 것임
	// 	sprintf(temp, "M*%06d", PLC_READ_ALL_ADRESS);		strcat(str, temp); //M*004000, ex)address 4000
	// 	sprintf(temp, "%04X", dfMAX_PLC_IN_CNT);			strcat(str, temp); //200 ==> C8 (Hexa)
	_stprintf_s(temp, _countof(temp), _T("F*%06d"), dfFADDRESS_RD_START);
	_tcscat_s(str, _countof(str), temp); //M*001001, ex)address 1001		// 시작번지

	int nReadCount = dfFADDRESS_TOTAL_COUNT;
	if (bUseBit)
	{
		nReadCount = dfFADDRESS_TOTAL_COUNT;
	}
	else
	{
		nReadCount = (int)floor(dfFADDRESS_TOTAL_COUNT / 16.0);
	}

	_stprintf_s(temp, _countof(temp), _T("%04X"), nReadCount);
	_tcscat_s(str, _countof(str), temp); //stadd 1 ==> 0001					// 읽을 비트 수

	size_t n = _tcsclen(str);					//str[n]   = PLC_CHAR_EOT;
	str[n] = 0;									//n = 42

	CString strCommand;
	strCommand.Format(_T("%s"), str);
	SendPacket(strCommand, false, false);

	return true;
}

long CMcFaddressScanner::Initialize(bool bRunMode, CString strIP, long nPort)
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

long CMcFaddressScanner::Close()
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

bool CMcFaddressScanner::IsConnected()
{
	return CCommEthFunction::IsConnected();
}

void CMcFaddressScanner::DisConnect()
{
	CCommEthFunction::CloseConnect();
}

void CMcFaddressScanner::ThreadRun()
{
	if (IsConnected() == true)
	{
		if (m_bThreadRunStop) return;
		//memset(m_lMcProtocolReadData, 0, sizeof(long) * dfGSADDRESS_TOTAL_COUNT);
		SendCommand();

		Sleep(200);
	}
	else
	{
		memset(m_lMcProtocolReadData, 0, sizeof(long) * dfFADDRESS_TOTAL_COUNT);
		memset(m_lMcProtocolReadData_clone, 0, sizeof(long) * dfFADDRESS_TOTAL_COUNT);
	}
}

bool CMcFaddressScanner::ReadFileCSV(CString strFileName)
{
    m_vecRData.Clear();
    m_vecRData.vecData.resize(dfFADDRESS_TOTAL_COUNT);
	
	std::vector<CString> vecErrorMemory;

    CStdioFile File(strFileName, CFile::modeRead);

    while (true)
    {
        CString strLine;

        if (File.ReadString(strLine))
        {
            if (strLine.Left(1) != _T("F"))
                continue;

            int iEnd = strLine.Find(_T(","));
            UINT nAddress = _ttoi(strLine.Mid(1, iEnd));

            CString strTemp;
            strTemp.Format(_T("F%d"), nAddress);
            strLine.Delete(0, iEnd + 1);

			if( (0 <= nAddress) && (nAddress < dfFADDRESS_LIMIT_COUNT) )
			{
				if (strLine.IsEmpty() == false)
				{
					m_vecRData.bEnable = true;

					m_vecRData.vecData[nAddress].nAddress = nAddress;
					m_vecRData.vecData[nAddress].strMapNumber = strTemp;
					m_vecRData.vecData[nAddress].strDescription = strLine;					
					m_vecRData.vecData[nAddress].bUsable = strLine.IsEmpty() ? false : true;
					
					m_iDataLast = nAddress;
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
		strLog.Format(_T("There are '%d' FAddress Numbers Out of range.\nPlease Refer to the Log."), vecErrorMemory.size());
		AfxMessageBox(strLog);

		m_pLogFileAccessor->KMLOG(OPER, _T("F Address memory numbers out of range List."));
		
		for (size_t i = 0; i < vecErrorMemory.size(); i++)
		{
			m_pLogFileAccessor->KMLOG(OPER, _T("Error Address : ") + vecErrorMemory[i]);
		}

		return false;
	}

	return true;
}

bool CMcFaddressScanner::ReadFileXLS(CString strFileName)
{
    m_vecRData.Clear();
    m_vecRData.vecData.resize(dfFADDRESS_TOTAL_COUNT);

    std::vector<CString> vecErrorMemory;
		
	CFile file(strFileName, CFile::shareDenyNone );
	CArchive ar(&file, CArchive::load);
	CXLEzAutomation ListExcel(FALSE);
	if(ListExcel.OpenExcelFile(strFileName) == TRUE)
	{
		int iaccess_offset = 2;
		for (int i = iaccess_offset; i < (int)(dfFADDRESS_TOTAL_COUNT + iaccess_offset); i++)
		{
			CString strAddress = ListExcel.GetCellValue(1, i);
			CString strDescription = ListExcel.GetCellValue(2, i);

			CString strTemp = strAddress;
			strTemp.Replace(_T("F"), _T(""));
            UINT nAddress = _ttoi(strTemp);

			if( (0 <= nAddress) && (nAddress < dfFADDRESS_LIMIT_COUNT) )
			{
				if (strTemp.IsEmpty() == false)
				{
					m_vecRData.bEnable = true;

					m_vecRData.vecData[nAddress].nAddress = nAddress;
					m_vecRData.vecData[nAddress].strMapNumber = strTemp;
					m_vecRData.vecData[nAddress].strDescription = strDescription;
					m_vecRData.vecData[nAddress].bUsable = strDescription.IsEmpty() ? false : true;

					m_iDataLast = nAddress;
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
		strLog.Format(_T("There are '%d' FAddress Numbers Out of range.\nPlease Refer to the Log."), vecErrorMemory.size());
		AfxMessageBox(strLog);

		m_pLogFileAccessor->KMLOG(OPER, _T("F Address memory numbers out of range List."));
		
		for (size_t i = 0; i < vecErrorMemory.size(); i++)
		{
			m_pLogFileAccessor->KMLOG(OPER, _T("Error Address : ") + vecErrorMemory[i]);
		}

		return false;
	}

	return true;
}

bool CMcFaddressScanner::ReadFileVerify(CString strFileName)
{
	CStdioFile File(strFileName, CFile::modeRead);

	bool bRet = false;

	while (true)
	{
		CString strLine;

		if (File.ReadString(strLine))
		{
			if (strLine.Left(1) != _T("F"))
				continue;

			int iEnd = strLine.Find(_T(","));
			UINT nAddress = _ttoi(strLine.Mid(1, iEnd));

			if (0 <= nAddress && nAddress <= dfFADDRESS_LIMIT_COUNT)
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

bool CMcFaddressScanner::SetDefaultFile(CString strFileName)
{
	if(CopyFile(strFileName, m_strDefaultFilePath, FALSE) == TRUE)
	{
		return ReadFileCSV(m_strDefaultFilePath);
	}

	return false;
}

long CMcFaddressScanner::GetAddressImportCount()
{
	return dfFADDRESS_TOTAL_COUNT;
}

long CMcFaddressScanner::GetAddressExportCount()
{
	return 0;
}

bool CMcFaddressScanner::GetAddressImportDataValue(UINT Index)
{
    if( (0 <= Index) && (Index < dfFADDRESS_TOTAL_COUNT) )
	{
		return (m_lMcProtocolReadData_clone[Index] == 1) ? true : false;
	}

    return false;
}

SADDRESSMAPDATA *CMcFaddressScanner::GetAddressImportDataPtr()
{
	return &m_vecRData;
}




long CMcFaddressScanner::GetAddressAlarmCount()
{
	std::vector<SAMapString> vecAlarm;

	for (UINT nCount = 0; nCount < dfFADDRESS_TOTAL_COUNT; nCount++)
	{
		if (m_lMcProtocolReadData_clone[nCount] == 1)
		{
			vecAlarm.push_back(m_vecRData.vecData[nCount]);
		}
	}

	if (vecAlarm.empty() == true)
	{
		m_vecAData.bEnable = false;
		m_vecAData.vecData = vecAlarm;

		m_pLogFileAccessor->KMLOG(OPER, _T("GetFAddressReadData not enable"));
		CString strText; strText.Format(_T("GetFAddressReadData : %d"), vecAlarm.size());
		m_pLogFileAccessor->KMLOG(OPER, strText);
	}
	else
	{
		m_vecAData.bEnable = true;
		m_vecAData.vecData = vecAlarm;

		m_pLogFileAccessor->KMLOG(OPER, _T("GetFAddressReadData enable"));
		CString strText; strText.Format(_T("GetFAddressReadData : %d"), vecAlarm.size());
		m_pLogFileAccessor->KMLOG(OPER, strText);
	}

	return (long)(m_vecAData.vecData.size());
}

bool CMcFaddressScanner::IsOccur_addressAlarm()
{
	return (0 < GetAddressAlarmCount());	
}

SADDRESSMAPDATA	*CMcFaddressScanner::GetAddressAlarmDataPtr()
{
	return &m_vecAData;
}

void CMcFaddressScanner::xNetCallbackRecieveData(LPVOID ud)
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

		if (m_strOldData.CompareNoCase(strMessage) == 0)
		{
			return;
		}
		else
		{
			m_strOldData.Format(_T("%s"), strMessage);
		}

		CString strLog = _T("[RECV - FAddress] : ") + strMessage;
		m_pLogFileAccessor->KMLOG(RECV, strLog);

		/*CString strMessage(pRcvData->szBody);
		if (strMessage.IsEmpty() == true)
			return;

		CString strLog = _T("[RECV - MAddress] : ") + strMessage;
		m_pLogFileAccessor->KMLOG(RECV, strLog);*/
		//CLogWrapper::GetInstance()->KMLOG(SYST, strLog);

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

			if (strCommand == PLC_CMD_SUCCESS)
			{
				continue;
			}

			bool bUseBit = false;
			bUseBit = m_bUseBit;

			if (bUseBit == true)
			{
				if (strCommand.GetLength() != dfFADDRESS_TOTAL_COUNT)
				{
					CString strLog = _T("[RECV - FAddress Small] : ") + strCommand;
					//CLogWrapper::GetInstance()->KMLOG(SYST, strLog);
					m_pLogFileAccessor->KMLOG(WARN, strLog);

					continue;
				}

				/*if (m_lMcProtocolReadData_clone == m_lMcProtocolReadData)
				{
				continue;
				}*/

				//Bit 처리 방식
				for (int iPos = 0; iPos < dfFADDRESS_TOTAL_COUNT; iPos++)
				{
					TCHAR a = strCommand.GetAt(iPos);				// '1' or '0' ASCII code

					m_lMcProtocolReadData[iPos] = a - '0';			// 숫자로 변환
				}
			}
			else // Word
			{
				BOOL bPLCData[dfFADDRESS_TOTAL_COUNT];
				ZeroMemory(&bPLCData, sizeof(bPLCData));

				CString strTempH, strTempL, str;
				int toByte = 0, tNum = 0;
				int i = 0, j = 0, l = 0, k = 0, n = 0;
				int nCnt = 0;
				int iCnt = (int)floor(dfFADDRESS_TOTAL_COUNT / 16.0);
				if (iCnt % 2)
				{
					strCommand += "OOOO";
					nCnt = (iCnt + 1) / 2;
				}
				else
				{
					nCnt = iCnt / 2;
				}

				for (i = 0; i < nCnt; i++)
				{
					strTempH = strCommand.Mid((8 * i), 4);			// 4
					strTempL = strCommand.Mid((8 * i) + 4, 4);			// 4

					str.Format(_T("%s%s"), strTempL, strTempH);	// H/L to L/H 

					l = str.GetLength() - 1;
					for (j = l; j >= 0; j--)
					{
						if (k >= dfFADDRESS_TOTAL_COUNT)
						{
							break;
						}

						// Hex to int
						toByte = HexToByte(str.GetAt(j));

						// Hex to Bin
						tNum = toByte;
						for (n = 0; n < 4; n++)		// 16 보다 작을거니까 4번만 나누워주면 된다
						{
							if (tNum % 2)
								bPLCData[k] = TRUE;
							else
								bPLCData[k] = FALSE;

							m_lMcProtocolReadData[k] = bPLCData[k];

							tNum /= 2;

							k++;
						}
					}
				}
			}

			memcpy(m_lMcProtocolReadData_clone, m_lMcProtocolReadData, sizeof(long) * dfFADDRESS_TOTAL_COUNT);
		}
	}
}

std::vector<CString> CMcFaddressScanner::GetCommand(CString &strData)
{
	CString strMessage = strData;

	//Test
	//strMessage.Format(_T("D00000FF03FF0003EC00000000000000000010001000010000001100111000000000000001010000000000000000000000000000000000000000000000000000000000000000001100000000111110000011000000001110100000010000000011111000001001001000110011100011100000001000001000100000000000000000000011000110000001010010111100000000000000000000000000000000001011110000000000000011111111000000000000010000100000000000001000000010111111000001010000001111111100000000101000001100001000010100010101000000000011000001000000111101111101010100001110001000000000000000000000000000000000000000000011111110000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000111110000001010000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001010000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"));
	//strMessage.Format(_T("D00000FF03FF0003EC00000000000000100010001000010000001100111000000000000001010000000000000000000000000000000000000000000000000000000000000000001100000000111110000011000000001110100000010000000011111000001001001000110011100011100000001000001000100000000000000000000011000110000001010010111100000000000000000000000000000000001011110000000000000011111111000000000000010000100000000000001000000010111111000001010000001111111100000000101000001100001000010100010101000000000011000001000000111101111101010100001110001000000000000000000000000000000000000000000011111110000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000111110000001010000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001010000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000D00000FF03FF0003EC00000000000000000010001000010000001100111000000000000001010000000000000000000000000000000000000000000000000000000000000000001100000000111110000011000000001110100000010000000011111000001001001000110011100011100000001000001000100000000000000000000011000110000001010010111100000000000000000000000000000000001011110000000000000011111111000000000000010000100000000000001000000010111111000001010000001111111100000000101000001100001000010100010101000000000011000001000000111101111101010100001110001000000000000000000000000000000000000000000011111110000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000111110000001010000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001010000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"));
	//strMessage.Format(_T("D00000FF03FF0003EC00000010000000100010001000010000001100111000000000000001010000000000000000000000000000000000000000000000000000000000000000001100000000111110000011000000001110100000010000000011111000001001001000110011100011100000001000001000100000000000000000000011000110000001010010111100000000000000000000000000000000001011110000000000000011111111000000000000010000100000000000001000000010111111000001010000001111111100000000101000001100001000010100010101000000000011000001000000111101111101010100001110001000000000000000000000000000000000000000000011111110000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000111110000001010000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001010000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000D00000FF03FF0003EC00000000000000000010001000010000001100111000000000000001010000000000000000000000000000000000000000000000000000000000000000001100000000111110000011000000001110100000010000000011111000001001001000110011100011100000001000001000100000000000000000000011000110000001010010111100000000000000000000000000000000001011110000000000000011111111000000000000010000100000000000001000000010111111000001010000001111111100000000101000001100001000010100010101000000000011000001000000111101111101010100001110001000000000000000000000000000000000000000000011111110000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000111110000001010000000000000000000000000000000000000000000000000000000000000000000000000000000000000000001010000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"));
	//////

	int iCnt = 0;
	int nReciveSize = 0;
	int nHeaderPos = 0;
	int nReplySize = 0;

	std::vector<CString> vecResult;

	//if (m_strOldData == strMessage)
	//{
	//	return vecResult;
	//}
	//else
	//{
	//	m_strOldData = strMessage;
	//}

	// Header
	TCHAR	temp[20];
	TCHAR	str[100];
	str[0] = 0;

	bool bUseBit = false;
	bUseBit = m_bUseBit;

	_tcscat_s(str, _countof(str), _T("D00000FF03FF"));

	if (bUseBit == true)
	{
		iCnt = dfFADDRESS_TOTAL_COUNT;

		nReciveSize = iCnt + PLC_CMD_HEADER_LENGTH;
		_stprintf_s(temp, _countof(temp), _T("%06X"), iCnt + 4);		_tcscat_s(str, _countof(str), temp);
	}
	else
	{
		iCnt = (int)floor(dfFADDRESS_TOTAL_COUNT / 16.0);

		nReciveSize = iCnt * 4 + PLC_CMD_HEADER_LENGTH;
		_stprintf_s(temp, _countof(temp), _T("%06X"), (iCnt * 4) + 4);	_tcscat_s(str, _countof(str), temp);
	}

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

					if (bUseBit)
					{
						CString strParam = strMessage;
						strParam = strParam.Mid(ipos + PLC_CMD_HEADER_LENGTH, dfFADDRESS_TOTAL_COUNT);

						if (strParam.IsEmpty() == true)
							break;

						vecResult.push_back(strParam);

						strMessage = strMessage.Mid(nReciveSize);
					}
					else
					{
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