#include "pch.h"
#include "McGaddressScanner.h"

//#include "McProtocolCtrl.h"
#include "..\..\common\XLS\XLEzAutomation.h"
#include "..\..\LogFunction\LogAccessor\LogFileAccessor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CMcGaddressScanner::CMcGaddressScanner(CString strDefaultFilePath, CMcProtocolCtrl *pMcProtocolCtrl)
//	: CMcAddressScannerImpl(strDefaultFilePath, pMcProtocolCtrl)
CMcGaddressScanner::CMcGaddressScanner(CString strLoggerFolderPath, CString strDefaultFilePath)
	: CMcAddressScannerImpl(strDefaultFilePath)
{
	m_strAddressName = _T("GlassSensor");

	//ReadFileCSV(m_strDefaultFilePath);

	// 로그 생성하기
	const CString strFolderPath = strLoggerFolderPath + _T("\\LogFile\\");
	const CString strLogFile = strFolderPath + m_strAddressName + _T(".log");
	MakeDir(strFolderPath);
	m_pLogFileAccessor->Initialize(strLogFile);
	//////

	m_strQueueMessage.Empty();

	memset(m_lMcProtocolReadData, 0, sizeof(long) * dfGSADDRESS_TOTAL_COUNT);
	memset(m_lMcProtocolReadData_clone, 0, sizeof(long) * dfGSADDRESS_TOTAL_COUNT);

	m_bThreadRunStop = false;
	m_strOldData.Empty();
}

CMcGaddressScanner::~CMcGaddressScanner()
{
}

bool CMcGaddressScanner::SendCommand()
{
	if (IsConnected() == false)
		return false;

	int		stadd = 1;
	int		stadd8 = 8;
	TCHAR	temp[20];
	TCHAR	str[100];
	str[0] = 0;
	//5000(헤드) 00FF(번호) 03FF(모듈번호) 00(국번) 0018(길이) 0090(타이머)  0401(읽기) 0001(비트단위) // 0000(워드단위)
	_tcscat_s(str, _countof(str), _T(PLC_CMD_ETHERNET_WORD_READ_D));		//Word

	short bit = 1;

	//4000번 부터 일괄적으로 읽을 것임
	// 	sprintf(temp, "M*%06d", PLC_READ_ALL_ADRESS);		strcat(str, temp); //M*004000, ex)address 4000
	// 	sprintf(temp, "%04X", dfMAX_PLC_IN_CNT);			strcat(str, temp); //200 ==> C8 (Hexa)
	_stprintf_s(temp, _countof(temp), _T("D*%06d"), dfGSADDRESS_WT_START);
	_tcscat_s(str, _countof(str), temp); //M*001001, ex)address 1001		// 시작번지

	int nReadCount = dfGSADDRESS_TOTAL_COUNT*2;

	_stprintf_s(temp, _countof(temp), _T("%04X"), nReadCount);
	_tcscat_s(str, _countof(str), temp); //stadd 1 ==> 0001					// 읽을 비트 수

	size_t n = _tcsclen(str);					//str[n]   = PLC_CHAR_EOT;
	str[n] = 0;									//n = 42

	CString strCommand;
	strCommand.Format(_T("%s"), str);
	SendPacket(strCommand, false, false);

	return true;
}

long CMcGaddressScanner::Initialize(bool bRunMode, CString strIP, long nPort)
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

long CMcGaddressScanner::Close()
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

bool CMcGaddressScanner::IsConnected()
{
	return CCommEthFunction::IsConnected();
}

void CMcGaddressScanner::DisConnect()
{
	CCommEthFunction::CloseConnect();
}

void CMcGaddressScanner::ThreadRun()
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
		memset(m_lMcProtocolReadData, 0, sizeof(long) * dfGSADDRESS_TOTAL_COUNT);
		memset(m_lMcProtocolReadData_clone, 0, sizeof(long) * dfGSADDRESS_TOTAL_COUNT);
	}
}

bool CMcGaddressScanner::ReadFileCSV(CString strFilename)
{
	m_vecRData.Clear();
	//m_vecWData.Clear();
	m_vecRData.vecData.resize(dfGSADDRESS_HALF_COUNT * 2);
	//m_vecWData.vecData.resize(dfGSADDRESS_HALF_COUNT);

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

			if (dfGSADDRESS_WT_START <= nAddress && nAddress < dfGSADDRESS_LIMIT_COUNT)
			{
				if (strLine.IsEmpty() == false)
				{
					////if (nAddress >= dfGSADDRESS_RD_START)
					//{
					//	nAddress -= dfGSADDRESS_WT_START;
					//	
					//	m_vecRData.bEnable = true;

					//	m_vecRData.vecData[nAddress].nAddress = nAddress;
					//	m_vecRData.vecData[nAddress].strMapNumber = strTemp;
					//	m_vecRData.vecData[nAddress].strDescription = strLine;
					//	m_vecRData.vecData[nAddress].bUsable = strLine.IsEmpty() ? false : true;
					//}
					////else
					//if(dfGSADDRESS_WT_START <= nAddress && nAddress < dfGSADDRESS_RD_START)
					//{
					//	nAddress -= dfGSADDRESS_WT_START;

					//	m_vecWData.bEnable = true;
					//	
					//	m_vecWData.vecData[nAddress].nAddress = nAddress;
					//	m_vecWData.vecData[nAddress].strMapNumber = strTemp;
					//	m_vecWData.vecData[nAddress].strDescription = strLine;
					//	m_vecWData.vecData[nAddress].bUsable = strLine.IsEmpty() ? false : true;					
					//}
					nAddress -= dfGSADDRESS_WT_START;

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
		strLog.Format(_T("There are '%d' GSADDRESS Numbers Out of range.\nPlease Refer to the Log."), vecErrorMemory.size());
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

bool CMcGaddressScanner::ReadFileXLS(CString strFileName)
{
	m_vecRData.Clear();
	//m_vecWData.Clear();
	m_vecRData.vecData.resize(dfGSADDRESS_HALF_COUNT * 2);
	//m_vecWData.vecData.resize(dfGSADDRESS_HALF_COUNT);

    std::vector<CString> vecErrorMemory;

	CFile file(strFileName, CFile::shareDenyNone );
	CArchive ar(&file, CArchive::load);
	CXLEzAutomation ListExcel(FALSE);
	if(ListExcel.OpenExcelFile(strFileName) == TRUE)
	{
		UINT iaccess_offset = 2;
		for (UINT i = iaccess_offset; i < dfGSADDRESS_TOTAL_COUNT + iaccess_offset; i++)
		{
			CString strAddress = ListExcel.GetCellValue(1, i);
			CString strDescription = ListExcel.GetCellValue(2, i);

			CString strTemp = strAddress;
			strTemp.Replace(_T("D"), _T(""));
			UINT nAddress = _ttoi(strTemp);

			if (dfGSADDRESS_RD_START <= nAddress && nAddress < dfGSADDRESS_LIMIT_COUNT)
			{
				if (strTemp.IsEmpty() == false)
				{
					//if (nAddress < dfGSADDRESS_WT_START)
					//{
					//	nAddress -= dfGSADDRESS_RD_START;
					//	
					//	m_vecRData.bEnable = true;

					//	m_vecRData.vecData[nAddress].nAddress = nAddress;
					//	m_vecRData.vecData[nAddress].strMapNumber = strTemp;
					//	m_vecRData.vecData[nAddress].strDescription = strDescription;						
					//	m_vecRData.vecData[nAddress].bUsable = strDescription.IsEmpty() ? false : true;
					//}
					//else
					//{
					//	nAddress -= dfGSADDRESS_WT_START;

					//	m_vecWData.bEnable = true;

					//	m_vecWData.vecData[nAddress].nAddress = nAddress;
					//	m_vecWData.vecData[nAddress].strMapNumber = strTemp;
					//	m_vecWData.vecData[nAddress].strDescription = strDescription;
					//	m_vecWData.vecData[nAddress].bUsable = strDescription.IsEmpty() ? false : true;
					//}
					nAddress -= dfGSADDRESS_RD_START;

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
		strLog.Format(_T("There are '%d' GSADDRESS Numbers Out of range.\nPlease Refer to the Log."), vecErrorMemory.size());
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

bool CMcGaddressScanner::ReadFileVerify(CString strFileName)
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

			if (dfGSADDRESS_RD_START <= nAddress && nAddress < dfGSADDRESS_LIMIT_COUNT)
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

bool CMcGaddressScanner::SetDefaultFile(CString strFileName)
{
	if(CopyFile(strFileName, m_strDefaultFilePath, FALSE) == TRUE)
	{
		return ReadFileCSV(m_strDefaultFilePath);
	}

	return false;
}

bool CMcGaddressScanner::SetMemoryBit(short addr, long val, long nBit/* = 1*/)
{
	return true;;
}

bool CMcGaddressScanner::SetMemoryBit(CString strAddr, long val, long nBit/* = 1*/)
{
	return true;;
}

bool CMcGaddressScanner::SetMemoryWord(long addr, float* val, long nWord/* = 1*/)
{
	if (IsConnected())
	{
		m_bThreadRunStop = true;

		int		stadd = 1;
		TCHAR	szTmp[100];
		TCHAR	szCmd[1000];

		szTmp[0] = 0x00;
		szCmd[0] = 0x00;

		_tcscat_s(szCmd, _countof(szCmd), _T("500000FF03FF00"));
		_stprintf_s(szTmp, _countof(szTmp), _T("%04X"), 24 + nWord * 4);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
		_tcscat_s(szCmd, _countof(szCmd), _T("0090"));				//PLC Time out
		_tcscat_s(szCmd, _countof(szCmd), _T("1401"));				//Command(Write)
		_tcscat_s(szCmd, _countof(szCmd), _T("0000"));				//Sub Command

		_stprintf_s(szTmp, _countof(szTmp), _T("D*%06d"), addr);	_tcscat_s(szCmd, _countof(szCmd), szTmp);

		//정의 필요
		_stprintf_s(szTmp, _countof(szTmp), _T("%04X"), nWord);		_tcscat_s(szCmd, _countof(szCmd), szTmp);

		for (int nCnt = 0; nCnt < nWord; nCnt++)
		{
			_stprintf_s(szTmp, _countof(szTmp), _T("%04X"), int(val[nCnt]));		_tcscat_s(szCmd, _countof(szCmd), szTmp);
		}

		size_t n = _tcsclen(szCmd);					//str[n]   = PLC_CHAR_EOT;
		szCmd[n] = 0;								//n = 42

		CString strCommand;
		strCommand.Format(_T("%s"), szCmd);
		SendPacket(strCommand, false, false);

		m_bThreadRunStop = false;

		return true;
	}
	else
	{
		m_pLogFileAccessor->KMLOG(OPER, _T("Fail : MC Protocol is Not Connect"));
	}
	return false;
}

long CMcGaddressScanner::GetAddressImportCount()
{
	return dfGSADDRESS_HALF_COUNT;
}

long CMcGaddressScanner::GetAddressExportCount()
{
	return dfGSADDRESS_HALF_COUNT;
}

bool CMcGaddressScanner::GetAddressImportDataValue(UINT Index)
{
	if ((dfGSADDRESS_HALF_COUNT <= Index) && (Index < dfGSADDRESS_TOTAL_COUNT))
	{
		return (m_lMcProtocolReadData_clone[Index] == 1) ? true : false;
	}

    return false;
}

CString CMcGaddressScanner::GetAddressImportDataValueString(UINT Index)
{
	Index -= dfGSADDRESS_WT_START;

	if ((0 <= Index) && (Index < dfGSADDRESS_TOTAL_COUNT))
	{
		CString strData;
		strData.Format(_T("%d"), m_lMcProtocolReadData_clone[Index]);
		return strData;
	}

	return _T("");
}

bool CMcGaddressScanner::GetAddressExportDataValue(UINT Index)
{
	if ((0 <= Index) && (Index < dfGSADDRESS_HALF_COUNT))
	{
		return (m_lMcProtocolReadData_clone[Index] == 1) ? true : false;
	}

    return false;
}

CString CMcGaddressScanner::GetAddressExportDataValueString(UINT Index)
{
	if ((dfGSADDRESS_HALF_COUNT <= Index) && (Index < dfGSADDRESS_TOTAL_COUNT))
	{
		CString strData;
		strData.Format(_T("%f"), m_lMcProtocolReadData_clone[Index]);
		return strData;
	}

	return _T("");
}

SADDRESSMAPDATA *CMcGaddressScanner::GetAddressImportDataPtr()
{
	return &m_vecRData;
}

SADDRESSMAPDATA *CMcGaddressScanner::GetAddressExportDataPtr()
{
	//return &m_vecWData;
	return &m_vecRData;
}

void CMcGaddressScanner::xNetCallbackRecieveData(LPVOID ud)
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

		CString strLog = _T("[RECV - GAddress] : ") + strMessage;
		m_pLogFileAccessor->KMLOG(RECV, strLog);

		/*CString strMessage(pRcvData->szBody);
		if (strMessage.IsEmpty() == true)
			return;

		CString strLog = _T("[RECV - MAddress] : ") + strMessage;*/
		//CLogWrapper::GetInstance()->KMLOG(SYST, strLog);

		//---------------------------------------------------------------------------
		// 패킷 단위로 응답에 대한 처리를 해야 한다.
		// socket 에 데이터가 진입하는데 2개 이상 붙어서 1개의 event 로 들어온다.
		// 모든 패킷에 대해서 command 단위로 분리해서 수행해야 한다.
		//---------------------------------------------------------------------------
		std::vector<CString> vecPacketCommandPool = GetCommand(strMessage);

		for (size_t iMsg = 0; iMsg < vecPacketCommandPool.size(); ++iMsg)
		{
			const CString strCommand = vecPacketCommandPool[iMsg];

			m_pLogFileAccessor->KMLOG(RECV, strMessage);

			if (strCommand == PLC_CMD_SUCCESS)
			{
				continue;
			}

			if (strCommand.GetLength() != dfGSADDRESS_TOTAL_COUNT * 8)
			{
				CString strLog = _T("[RECV - GAddress Small] : ") + strCommand;
				//CLogWrapper::GetInstance()->KMLOG(SYST, strLog);
				m_pLogFileAccessor->KMLOG(WARN, strLog);

				continue;
			}

			/*if (m_lMcProtocolReadData_clone == m_lMcProtocolReadData)
			{
			continue;
			}*/


			//Word 처리 방식
			//for (int iPos = 0; iPos < dfGSADDRESS_TOTAL_COUNT; iPos++)
			//{
			//	CString strWord = strCommand.Mid(iPos * 8, 8);
			//	
			//	FLOAT_FIELD u_FloatField;

			//	u_FloatField.m_16bit.m_Bottom1Byte = hextoi(strWord.Mid(0, 4));
			//	u_FloatField.m_16bit.m_Top1Byte = hextoi(strWord.Mid(4, 4));

			//	m_lMcProtocolReadData_clone[iPos] = u_FloatField.m_float;
			//}
			for (int iPos = 0; iPos < dfGSADDRESS_TOTAL_COUNT; iPos++)
			{
				CString strWord = strCommand.Mid(iPos * 4, 4);

				m_lMcProtocolReadData[iPos] = hextoi(strWord);
			}

			memcpy(m_lMcProtocolReadData_clone, m_lMcProtocolReadData, sizeof(long) * dfGSADDRESS_TOTAL_COUNT);
		}
	}
}

std::vector<CString> CMcGaddressScanner::GetCommand(CString &strData)
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
	if (false)
	{
		iCnt = dfGSADDRESS_TOTAL_COUNT;

		nReciveSize = iCnt + PLC_CMD_HEADER_LENGTH;
		_stprintf_s(temp, _countof(temp), _T("%06X"), iCnt + 4);		_tcscat_s(str, _countof(str), temp);
	}
	else
	{
		iCnt = (int)floor(dfGSADDRESS_TOTAL_COUNT / 16.0);

		iCnt = dfGSADDRESS_TOTAL_COUNT * 2;
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
					
					CString strParam = strMessage;
					strParam = strMessage.Mid(ipos + PLC_CMD_HEADER_LENGTH, dfGSADDRESS_TOTAL_COUNT * 2 * 4);

					int nnn = strParam.GetLength();

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