#include "pch.h"
#include "McPaddressScanner.h"

//#include "McProtocolCtrl.h"
#include "..\..\common\XLS\XLEzAutomation.h"
#include "..\..\LogFunction\LogAccessor\LogFileAccessor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CMcPaddressScanner::CMcPaddressScanner(CString strDefaultFilePath, CMcProtocolCtrl *pMcProtocolCtrl)
//	: CMcAddressScannerImpl(strDefaultFilePath, pMcProtocolCtrl)
CMcPaddressScanner::CMcPaddressScanner(CString strLoggerFolderPath, CString strDefaultFilePath)
	: CMcAddressScannerImpl(strDefaultFilePath)
{
	m_strAddressName = _T("SACC");

//	ReadFileCSV(m_strDefaultFilePath);

	// 로그 생성하기
	const CString strFolderPath	= strLoggerFolderPath + _T("\\LogFile\\");
	const CString strLogFile	= strFolderPath + m_strAddressName + _T(".log");
	MakeDir(strFolderPath);
	m_pLogFileAccessor->Initialize(strLogFile);
	//////

	m_strQueueMessage.Empty();

	memset(m_lMcProtocolReadData, 0, sizeof(long) * dfPACCADDRESS_TOTAL_COUNT);
	memset(m_lMcProtocolReadData_clone, 0, sizeof(long) * dfPACCADDRESS_TOTAL_COUNT);
	m_bUseBit = false;
	m_bThreadRunStop = false;
	m_strOldData.Empty();
}

CMcPaddressScanner::~CMcPaddressScanner()
{
	;
}

bool CMcPaddressScanner::SendCommand(bool bUseBit/* = true*/)
{
	if (IsConnected() == false)
		return false;

	int		stadd = 1;
	int		stadd8 = 8;
	TCHAR	temp[20];
	TCHAR	str[100];
	str[0] = 0;

	//5000(헤드) 00FF(번호) 03FF(모듈번호) 00(국번) 0018(길이) 0090(타이머)  0401(읽기) 0001(비트단위) // 0000(워드단위)
	if (bUseBit)
	{
		_tcscat_s(str, _countof(str), _T(PLC_CMD_ETHERNET_SINGLE_READ));		//Bit
	}
	else
	{
		_tcscat_s(str, _countof(str), _T(PLC_CMD_ETHERNET_WORD_READ_D));		//Word
	}

	short bit = 1;

	//3900번 부터 일괄적으로 읽을 것임
	_stprintf_s(temp, _countof(temp), _T("M*%06d"), dfPACCADDRESS_RD_START);
	_tcscat_s(str, _countof(str), temp); //M*001001, ex)address 1001		// 시작번지

	int nReadCount = dfPACCADDRESS_TOTAL_COUNT;
	if (bUseBit)
	{
		nReadCount = dfPACCADDRESS_TOTAL_COUNT;
	}
	else
	{
		nReadCount = (int)floor(dfPACCADDRESS_TOTAL_COUNT / 16.0);
	}

	_stprintf_s(temp, _countof(temp), _T("%04X"), nReadCount);
	_tcscat_s(str, _countof(str), temp); //stadd 1 ==> 0001					// 읽을 비트 수

	size_t n = _tcsclen(str);					
	str[n] = 0;									

	bool bRet = false;
	CString strCommand;
	strCommand.Format(_T("%s"), str);
	bRet = SendPacket(strCommand, false, false);

	return bRet;
}

long CMcPaddressScanner::Initialize(bool bRunMode, CString strIP, long nPort)
{
	if (IsConnected() == true)
	{
		//이미 연결
		return 0;
	}

	SetNetInfo(bRunMode, strIP, nPort);
	bool lRet = CCommEthFunction::Initialize();
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

long CMcPaddressScanner::Close()
{
	long lRet = MC_PROTOCOL_SUCCESS;

	CCommEthFunction::Finalize();

	return lRet;
}

bool CMcPaddressScanner::IsConnected()
{
	return CCommEthFunction::IsConnected();
}

void CMcPaddressScanner::DisConnect()
{
	CCommEthFunction::CloseConnect();
}

void CMcPaddressScanner::ThreadRun()
{
    if (IsConnected() == true)
    {
		if (m_bThreadRunStop) return;

		SendCommand(m_bUseBit);

        /*memset(m_lMcProtocolReadData, 0, sizeof(long) * dfPACCADDRESS_TOTAL_COUNT);

        m_pMcProtocolCtrl->GetMemoryBitBlock(_T("M3900"), dfPACCADDRESS_TOTAL_COUNT, m_lMcProtocolReadData);

        memcpy(m_lMcProtocolReadData_clone, m_lMcProtocolReadData, sizeof(long) * dfPACCADDRESS_TOTAL_COUNT);*/
	}
	else
	{
		memset(m_lMcProtocolReadData, 0, sizeof(long) * dfPACCADDRESS_TOTAL_COUNT);
		memset(m_lMcProtocolReadData_clone, 0, sizeof(long) * dfPACCADDRESS_TOTAL_COUNT);
	}
}

bool CMcPaddressScanner::ReadFileCSV(CString strFilename)
{
	m_vecRData.Clear();
	//m_vecWData.Clear();
	m_vecRData.vecData.resize(dfPACCADDRESS_HALF_COUNT * 2);
	//m_vecWData.vecData.resize(dfPACCADDRESS_HALF_COUNT);

	setlocale(LC_ALL, "Korean");

	std::vector<CString> vecErrorMemory;

	CStdioFile File(strFilename, CFile::modeRead);

	while (true)
	{
		CString strLine;

		if (File.ReadString(strLine))
		{
			if (strLine.Left(1) != _T("M"))
				continue;

			int iEnd = strLine.Find(_T(","));
			UINT nAddress = _ttoi(strLine.Mid(1, iEnd));

			CString strTemp;
			strTemp.Format(_T("M%d"), nAddress);
			strLine.Delete(0, iEnd + 1);

			if (dfPACCADDRESS_RD_START <= nAddress && nAddress < dfPACCADDRESS_LIMIT_COUNT)
			{
				if (strLine.IsEmpty() == false)
				{
					nAddress -= dfPACCADDRESS_RD_START;

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
		strLog.Format(_T("There are '%d' PACC ADDRESS Numbers Out of range.\nPlease Refer to the Log."), vecErrorMemory.size());
		AfxMessageBox(strLog);

		m_pLogFileAccessor->KMLOG(OPER, _T("SACC Address memory numbers out of range List."));
		for (size_t i = 0; i < vecErrorMemory.size(); i++)
		{
			m_pLogFileAccessor->KMLOG(OPER, _T("Error Address : ") + vecErrorMemory[i]);
		}
		
		return false;
	}

	return true;
}

bool CMcPaddressScanner::ReadFileXLS(CString strFileName)
{
	m_vecRData.Clear();
	//m_vecWData.Clear();
	m_vecRData.vecData.resize(dfPACCADDRESS_HALF_COUNT * 2);
	//m_vecWData.vecData.resize(dfPACCADDRESS_HALF_COUNT);

    std::vector<CString> vecErrorMemory;

	CFile file(strFileName, CFile::shareDenyNone );
	CArchive ar(&file, CArchive::load);
	CXLEzAutomation ListExcel(FALSE);
	if(ListExcel.OpenExcelFile(strFileName) == TRUE)
	{
		UINT iaccess_offset = 2;
		for (UINT i = iaccess_offset; i < dfPACCADDRESS_TOTAL_COUNT + iaccess_offset; i++)
		{
			CString strAddress = ListExcel.GetCellValue(1, i);
			CString strDescription = ListExcel.GetCellValue(2, i);

			CString strTemp = strAddress;
			strTemp.Replace(_T("M"), _T(""));
			UINT nAddress = _ttoi(strTemp);

			if (dfPACCADDRESS_RD_START <= nAddress && nAddress < dfPACCADDRESS_LIMIT_COUNT)
			{
				if (strTemp.IsEmpty() == false)
				{
					//if (nAddress < dfPACCADDRESS_WT_START)
					//{
					//	nAddress -= dfPACCADDRESS_RD_START;
					//	
					//	m_vecRData.bEnable = true;

					//	m_vecRData.vecData[nAddress].nAddress = nAddress;
					//	m_vecRData.vecData[nAddress].strMapNumber = strTemp;
					//	m_vecRData.vecData[nAddress].strDescription = strDescription;						
					//	m_vecRData.vecData[nAddress].bUsable = strDescription.IsEmpty() ? false : true;
					//}
					//else
					//{
					//	nAddress -= dfPACCADDRESS_WT_START;

					//	m_vecWData.bEnable = true;

					//	m_vecWData.vecData[nAddress].nAddress = nAddress;
					//	m_vecWData.vecData[nAddress].strMapNumber = strTemp;
					//	m_vecWData.vecData[nAddress].strDescription = strDescription;
					//	m_vecWData.vecData[nAddress].bUsable = strDescription.IsEmpty() ? false : true;
					//}
					nAddress -= dfPACCADDRESS_RD_START;

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
		strLog.Format(_T("There are '%d' PACC ADDRESS Numbers Out of range.\nPlease Refer to the Log."), vecErrorMemory.size());
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

bool CMcPaddressScanner::ReadFileVerify(CString strFileName)
{
	CStdioFile File(strFileName, CFile::modeRead);

	bool bRet = false;

	while (true)
	{
		CString strLine;

		if (File.ReadString(strLine))
		{
			if (strLine.Left(1) != _T("M"))
				continue;

			int iEnd = strLine.Find(_T(","));
			UINT nAddress = _ttoi(strLine.Mid(1, iEnd));

			if (dfPACCADDRESS_RD_START <= nAddress && nAddress < dfPACCADDRESS_LIMIT_COUNT)
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

bool CMcPaddressScanner::SetDefaultFile(CString strFileName)
{
	if(CopyFile(strFileName, m_strDefaultFilePath, FALSE) == TRUE)
	{
		return ReadFileCSV(m_strDefaultFilePath);
	}

	return false;
}

bool CMcPaddressScanner::SetMemoryBit(short addr, long val, long nBit/* = 1*/)
{
	//if (IsConnected())
	//{
	//	CString _addr;
	//	_addr.Format(_T("M%d"), addr);

	//	long Rtn = 1;// m_pMcProtocolCtrl->SetMemoryBit(_addr, val);

	//	if (Rtn == 0)
	//	{
	//		CString strLog;
	//		strLog.Format(_T("Fail : SetMemoryBit() Address Bit : %d"), addr);
	//		m_pLogFileAccessor->KMLOG(OPER, strLog);
	//	}
	//	else
	//	{
	//		return true;
	//	}
	//}
	//else
	//{
	//	m_pLogFileAccessor->KMLOG(OPER, _T("Fail : MXComponent(MX_PLC_M) is Not Connect"));
	//}
	return false;
}

bool CMcPaddressScanner::SetMemoryBit(CString strAddr, long val, long nBit/* = 1*/)
{
	//if (IsConnected())
	//{
	//	long Rtn = 1;// m_pMcProtocolCtrl->SetMemoryBit(strAddr, bValue);

	//	if (Rtn == 0)
	//	{
	//		CString strLog;
	//		strLog.Format(_T("Fail : SetMemoryBit() Address : %s"), strAddr);
	//		m_pLogFileAccessor->KMLOG(OPER, strLog);
	//	}
	//	else
	//	{
	//		return true;
	//	}
	//}
	//else
	//{
	//	m_pLogFileAccessor->KMLOG(OPER, _T("Fail : MXComponent(MX_PLC_M) is Not Connect"));
	//}
	return false;
}

long CMcPaddressScanner::GetAddressImportCount()
{
	return dfPACCADDRESS_HALF_COUNT;
}

long CMcPaddressScanner::GetAddressExportCount()
{
	return dfPACCADDRESS_HALF_COUNT;
}

bool CMcPaddressScanner::GetAddressImportDataValue(UINT Index)
{
	if ((0 <= Index - dfPACCADDRESS_RD_START) && (Index - dfPACCADDRESS_RD_START < dfPACCADDRESS_TOTAL_COUNT))
	{
		return (m_lMcProtocolReadData_clone[Index - dfPACCADDRESS_RD_START] == 1) ? true : false;
	}

    return false;
}

bool CMcPaddressScanner::GetAddressExportDataValue(UINT Index)
{
	if ((0 <= Index) && (Index < dfPACCADDRESS_HALF_COUNT))
	{
		return (m_lMcProtocolReadData_clone[Index] == 1) ? true : false;
	}

    return false;
}

SADDRESSMAPDATA *CMcPaddressScanner::GetAddressImportDataPtr()
{
	return &m_vecRData;
}

SADDRESSMAPDATA *CMcPaddressScanner::GetAddressExportDataPtr()
{
	//return &m_vecWData;
	return &m_vecRData;
}

void CMcPaddressScanner::xNetCallbackRecieveData(LPVOID ud)
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

		CString strLog = _T("[RECV - PAddress] : ") + strMessage;
		m_pLogFileAccessor->KMLOG(RECV, strLog);

	//	CString strMessage(pRcvData->szBody);
	//	CString strLog = _T("[RECV - PAddress] : ") + strMessage;
	//	m_pLogFileAccessor->KMLOG(RECV, strLog);

	////	CLogWrapper::GetInstance()->KMLOG(SYST, strLog);
	//	strMessage.Replace(_T(" "), _T(""));
	//	if (strMessage.IsEmpty() == true)
	//		return;

		//---------------------------------------------------------------------------
		// 패킷 단위로 응답에 대한 처리를 해야 한다.
		// socket 에 데이터가 진입하는데 2개 이상 붙어서 1개의 event 로 들어온다.
		// 모든 패킷에 대해서 command 단위로 분리해서 수행해야 한다.
		//---------------------------------------------------------------------------
		std::vector<CString> vecPacketCommandPool = GetCommand(strMessage);

		for (size_t iMsg = 0; iMsg < vecPacketCommandPool.size(); ++iMsg)
		{
			CString strCommand = vecPacketCommandPool[iMsg];

			m_pLogFileAccessor->KMLOG(RECV, strCommand);

			if (strCommand == PLC_CMD_SUCCESS)
			{
				continue;
			}

			bool bUseBit = false;
			bUseBit = m_bUseBit;

			if (bUseBit == true)
			{
				if (strCommand.GetLength() != dfPACCADDRESS_TOTAL_COUNT)
				{
					CString strLog = _T("[RECV - MAddress Small] : ") + strCommand;
					//CLogWrapper::GetInstance()->KMLOG(SYST, strLog);
					m_pLogFileAccessor->KMLOG(WARN, strLog);

					continue;
				}

				//Bit 처리 방식
				for (int iPos = 0; iPos < dfPACCADDRESS_TOTAL_COUNT; iPos++)
				{
					TCHAR a = strCommand.GetAt(iPos);				// '1' or '0' ASCII code

					m_lMcProtocolReadData[iPos] = a - '0';			// 숫자로 변환
				}
			}
			else // Word
			{
				BOOL bPLCData[dfPACCADDRESS_TOTAL_COUNT];
				ZeroMemory(&bPLCData, sizeof(bPLCData));

				CString strTempH, strTempL, str;
				int toByte = 0, tNum = 0;
				int i = 0, j = 0, l = 0, k = 0, n = 0;
				int nCnt = 0;
				int iCnt = (int)floor(dfPACCADDRESS_TOTAL_COUNT / 16.0);
				if (iCnt % 2)
				{
					strCommand += _T("OOOO");
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
						if (k >= dfPACCADDRESS_TOTAL_COUNT)
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

			memcpy(m_lMcProtocolReadData_clone, m_lMcProtocolReadData, sizeof(long) * dfPACCADDRESS_TOTAL_COUNT);
		}
	}
}

std::vector<CString> CMcPaddressScanner::GetCommand(CString& strData)
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
	bool bUseBit = false;
	bUseBit = m_bUseBit;

	_tcscat_s(str, _countof(str), _T("D00000FF03FF"));
	if (bUseBit == true)
	{
		iCnt = dfPACCADDRESS_TOTAL_COUNT;

		nReciveSize = iCnt + PLC_CMD_HEADER_LENGTH;
		_stprintf_s(temp, _countof(temp), _T("%06X"), iCnt + 4);		_tcscat_s(str, _countof(str), temp);
	}
	else
	{
		iCnt = (int)floor(dfPACCADDRESS_TOTAL_COUNT / 16.0);

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
						strParam = strParam.Mid(ipos + PLC_CMD_HEADER_LENGTH, dfPACCADDRESS_TOTAL_COUNT);

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

bool CMcPaddressScanner::CheckActionDone(long addr, DWORD dwTimeout, bool val/* = true*/)
{
	DWORD dwStart = timeGetTime();
	do
	{
		WaitForSingleObject(m_hWaitTimeEvent, 100);

		if (GetAddressImportDataValue(addr) == val)
		{
			return true;
		}

		DWORD dwCurrent = timeGetTime();
		if ((dwCurrent - dwStart) > (dwTimeout * 1000))
		{
			break;
		}
	} while (1);

	return false;
}