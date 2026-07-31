#include "pch.h"
#include "McMaddressScanner.h"

//#include "McProtocolCtrl.h"
#include "..\..\common\XLS\XLEzAutomation.h"
#include "..\..\LogFunction\LogAccessor\LogFileAccessor.h"



#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CMcMaddressScanner::CMcMaddressScanner(CString strDefaultFilePath, CMcProtocolCtrl *pMcProtocolCtrl)
//	: CMcAddressScannerImpl(strDefaultFilePath, pMcProtocolCtrl)
CMcMaddressScanner::CMcMaddressScanner(CString strLoggerFolderPath, CString strDefaultFilePath, CString strLogName, CLogFileAccessor* pLogFile, bool bWriteOnly)
	: CMcAddressScannerImpl(strDefaultFilePath)
{
	m_strAddressName = _T("M_Address");

	m_bUseWriteOnly = bWriteOnly;

//	ReadFileCSV(m_strDefaultFilePath);

	// 로그 생성하기
	const CString strFolderPath = strLoggerFolderPath + _T("\\LogFile\\");
	const CString strLogFile = strFolderPath + strLogName + _T(".log");
	MakeDir(strFolderPath);

	//if (pLogFile != nullptr)
	//	m_pLogFileAccessor = pLogFile;
	//else
		m_pLogFileAccessor->Initialize(strLogFile);
	//////

	m_strQueueMessage.Empty();
	
	memset(m_lMcProtocolReadData, 0, sizeof(long) * dfMADDRESS_TOTAL_COUNT);
	memset(m_lMcProtocolReadData_clone, 0, sizeof(long) * dfMADDRESS_TOTAL_COUNT);
	m_bUseBit = false;
	m_bThreadRunStop = false;
	m_strOldData.Empty();
}

CMcMaddressScanner::~CMcMaddressScanner()
{
	;
}

bool CMcMaddressScanner::SendCommand(bool bUseBit/* = true*/)
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

	//4000번 부터 일괄적으로 읽을 것임
	// 	sprintf(temp, "M*%06d", PLC_READ_ALL_ADRESS);		strcat(str, temp); //M*004000, ex)address 4000
	// 	sprintf(temp, "%04X", dfMAX_PLC_IN_CNT);			strcat(str, temp); //200 ==> C8 (Hexa)
	_stprintf_s(temp, _countof(temp), _T("M*%06d"), dfMADDRESS_RD_START);
	_tcscat_s(str, _countof(str), temp); //M*001001, ex)address 1001		// 시작번지

	int nReadCount = dfMADDRESS_TOTAL_COUNT;
	if (bUseBit)
	{
		nReadCount = dfMADDRESS_TOTAL_COUNT;
	}
	else
	{
		nReadCount = (int)floor(dfMADDRESS_TOTAL_COUNT / 16.0);
	}

	_stprintf_s(temp, _countof(temp), _T("%04X"), nReadCount);
	_tcscat_s(str, _countof(str), temp); //stadd 1 ==> 0001					// 읽을 비트 수

	size_t n = _tcsclen(str);					//str[n]   = PLC_CHAR_EOT;
	str[n] = 0;									//n = 42

	bool bRet = false;
	CString strCommand;
	strCommand.Format(_T("%s"), str);
	bRet = SendPacket(strCommand, false, false);

	return bRet;
}

long CMcMaddressScanner::Initialize(bool bRunMode, CString strIP, long nPort)
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

long CMcMaddressScanner::Close()
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

bool CMcMaddressScanner::IsConnected()
{
	return CCommEthFunction::IsConnected();
}

void CMcMaddressScanner::ThreadRun()
{
    if (IsConnected() == true)
    {
		if (m_bThreadRunStop) return;
		//memset(m_lMcProtocolReadData, 0, sizeof(long) * dfMADDRESS_TOTAL_COUNT);
		SendCommand(m_bUseBit);
	}
	else
	{
		memset(m_lMcProtocolReadData, 0, sizeof(long) * dfMADDRESS_TOTAL_COUNT);
		memset(m_lMcProtocolReadData_clone, 0, sizeof(long) * dfMADDRESS_TOTAL_COUNT);
	}
}

void CMcMaddressScanner::DisConnect()
{
	CCommEthFunction::CloseConnect();
}

bool CMcMaddressScanner::ReadFileCSV(CString strFilename)
{
	m_vecRData.Clear();
	//m_vecWData.Clear();
	m_vecRData.vecData.resize(dfMADDRESS_HALF_COUNT * 2);
	//m_vecWData.vecData.resize(dfMADDRESS_HALF_COUNT);

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

			if (dfMADDRESS_RD_START <= nAddress && nAddress < dfMADDRESS_LIMIT_COUNT)
			{
				if (strLine.IsEmpty() == false)
				{
					nAddress -= dfMADDRESS_RD_START;

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
		strLog.Format(_T("There are '%d' MAddress Numbers Out of range.\nPlease Refer to the Log."), vecErrorMemory.size());
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

bool CMcMaddressScanner::ReadFileXLS(CString strFileName)
{
	m_vecRData.Clear();
	//m_vecWData.Clear();
	m_vecRData.vecData.resize(dfMADDRESS_HALF_COUNT * 2);
	//m_vecWData.vecData.resize(dfMADDRESS_HALF_COUNT);

    std::vector<CString> vecErrorMemory;

	CFile file(strFileName, CFile::shareDenyNone );
	CArchive ar(&file, CArchive::load);
	CXLEzAutomation ListExcel(FALSE);
	if(ListExcel.OpenExcelFile(strFileName) == TRUE)
	{
		UINT iaccess_offset = 2;
		for (UINT i = iaccess_offset; i < dfMADDRESS_TOTAL_COUNT + iaccess_offset; i++)
		{
			CString strAddress = ListExcel.GetCellValue(1, i);
			CString strDescription = ListExcel.GetCellValue(2, i);

			CString strTemp = strAddress;
			strTemp.Replace(_T("M"), _T(""));
			UINT nAddress = _ttoi(strTemp);

			if (dfMADDRESS_RD_START <= nAddress && nAddress < dfMADDRESS_LIMIT_COUNT)
			{
				if (strTemp.IsEmpty() == false)
				{
					//if (nAddress < dfMADDRESS_WT_START)
					//{
					//	nAddress -= dfMADDRESS_RD_START;
					//	
					//	m_vecRData.bEnable = true;

					//	m_vecRData.vecData[nAddress].nAddress = nAddress;
					//	m_vecRData.vecData[nAddress].strMapNumber = strTemp;
					//	m_vecRData.vecData[nAddress].strDescription = strDescription;						
					//	m_vecRData.vecData[nAddress].bUsable = strDescription.IsEmpty() ? false : true;
					//}
					//else
					//{
					//	nAddress -= dfMADDRESS_WT_START;

					//	m_vecWData.bEnable = true;

					//	m_vecWData.vecData[nAddress].nAddress = nAddress;
					//	m_vecWData.vecData[nAddress].strMapNumber = strTemp;
					//	m_vecWData.vecData[nAddress].strDescription = strDescription;
					//	m_vecWData.vecData[nAddress].bUsable = strDescription.IsEmpty() ? false : true;
					//}
					nAddress -= dfMADDRESS_RD_START;

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
		strLog.Format(_T("There are '%d' MAddress Numbers Out of range.\nPlease Refer to the Log."), vecErrorMemory.size());
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

bool CMcMaddressScanner::ReadFileVerify(CString strFileName)
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

			if (dfMADDRESS_RD_START <= nAddress && nAddress < dfMADDRESS_LIMIT_COUNT)
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

bool CMcMaddressScanner::SetDefaultFile(CString strFileName)
{
	if(CopyFile(strFileName, m_strDefaultFilePath, FALSE) == TRUE)
	{
		return ReadFileCSV(m_strDefaultFilePath);
	}

	return false;
}

bool CMcMaddressScanner::SetMemoryBit(short addr, long val, long nBit/* = 1*/)
{
	if (IsConnected())
	{
		m_bThreadRunStop = true;

		int		stadd = 1;
		TCHAR	szTmp[100];
		TCHAR	szCmd[1000];
		stadd = nBit;

		szTmp[0] = 0x00;
		szCmd[0] = 0x00;

		_tcscat_s(szCmd, _countof(szCmd), _T("500000FF03FF00"));
		_stprintf_s(szTmp, _countof(szTmp), _T("%04X"), 24 + nBit);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
		_tcscat_s(szCmd, _countof(szCmd), _T("0090"));				//PLC Time out
		_tcscat_s(szCmd, _countof(szCmd), _T("1401"));				//Command(Write)
		_tcscat_s(szCmd, _countof(szCmd), _T("0001"));				//Sub Command

		_stprintf_s(szTmp, _countof(szTmp), _T("M*%06d"), addr);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
		_stprintf_s(szTmp, _countof(szTmp), _T("%04X"), stadd);		_tcscat_s(szCmd, _countof(szCmd), szTmp);

		BYTE nByte = 0x00;
		for (int i = 0; i<nBit; i++)
		{
			nByte = (val >> i) & 0x01;
			if (nByte == 0)
				_tcscat_s(szCmd, _countof(szCmd), _T("0"));
			else
				_tcscat_s(szCmd, _countof(szCmd), _T("1"));
		}

		size_t n = _tcsclen(szCmd);					//str[n]   = PLC_CHAR_EOT;
		szCmd[n] = 0;								//n = 42

		CString strCommand;
		strCommand.Format(_T("%s"), szCmd);
		bool bRet = SendPacket(strCommand, false, false);

		CString strLog;
		if (bRet)
		{
			//성공	
			strLog.Format(_T("Maddress, SetMemoryBit(%d) Address : %d, Val : %d, Bits : %d, %s"), bRet, addr, val, nBit, strCommand);
		}
		else
		{
			strLog.Format(_T("Maddress, SetMemoryBit(%d) Fail Address : %d, Val : %d, Bits : %d, %s"), bRet, addr, val, nBit, strCommand);
		}

		m_pLogFileAccessor->KMLOG(SEND, strLog);

		m_bThreadRunStop = false;
	
		return true;
	}
	else
	{
		m_pLogFileAccessor->KMLOG(SEND, _T("Maddress, Fail :  is Not Connect"));
	}
	
	m_bThreadRunStop = false;

	return false;
}

bool CMcMaddressScanner::SetMemoryBit(CString strAddr, long val, long nBit/* = 1*/)
{
	bool bRet = false;
	if (IsConnected())
	{
		int index = _wtoi(strAddr.Mid(1, strAddr.GetLength()));

		bRet = SetMemoryBit(index, val, nBit);
	}
	else
	{
		m_pLogFileAccessor->KMLOG(OPER, _T("Fail : MC Protocol is Not Connect"));
	}
	
	return bRet;
}

bool CMcMaddressScanner::SetMemoryWord(long addr, float* val, long nWord/* = 1*/)
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
		_stprintf_s(szTmp, _countof(szTmp), _T("%04X"), 24 + 1);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
		_tcscat_s(szCmd, _countof(szCmd), _T("0090"));				//PLC Time out
		_tcscat_s(szCmd, _countof(szCmd), _T("1401"));				//Command(Write)
		_tcscat_s(szCmd, _countof(szCmd), _T("0000"));				//Sub Command

		_stprintf_s(szTmp, _countof(szTmp), _T("D*%06d"), addr);	_tcscat_s(szCmd, _countof(szCmd), szTmp);

		//정의 필요
		_stprintf_s(szTmp, _countof(szTmp), _T("%04X"), 2);		_tcscat_s(szCmd, _countof(szCmd), szTmp);

		FLOAT_FIELD u_FloatField;
		u_FloatField.m_float = *val;

		_stprintf_s(szTmp, _countof(szTmp), _T("%04X%04X"), u_FloatField.m_16bit.m_Bottom1Byte, u_FloatField.m_16bit.m_Top1Byte);
		_tcscat_s(szCmd, _countof(szCmd), szTmp);

		size_t n = _tcsclen(szCmd);					//str[n]   = PLC_CHAR_EOT;
		szCmd[n] = 0;								//n = 42

		CString strCommand;
		strCommand.Format(_T("%s"), szCmd);
		bool bRet = SendPacket(strCommand, false, false);

		CString strLog;
		strLog.Format(_T("Maddress SetMemoryWord(%d) Address : %d, Val : %d, %s"), bRet, addr, val, strCommand);
		m_pLogFileAccessor->KMLOG(SEND, strLog);

		m_bThreadRunStop = false;

		return true;
	}
	else
	{
		m_pLogFileAccessor->KMLOG(OPER, _T("Fail : MC Protocol is Not Connect"));
	}

	return false;
}

bool CMcMaddressScanner::SetMemoryStr(long addr, TCHAR* val, long nWord/* = 1*/)
{
	if (nWord > 100)
	{
		CString strLog;
		strLog.Format(_T("Maddress SetMemoryWord(%d) Size Over!!, Address : D%d, Send : %s"), false, addr, val);
		m_pLogFileAccessor->KMLOG(WARN, strLog);
		return false;
	}

	int		stadd = 1;
	TCHAR	szTmp[1000];
	TCHAR	szCmd[10000];
	szTmp[0] = 0x00;
	szCmd[0] = 0x00;

	TCHAR   cVal[100];
	memset(cVal, NULL, sizeof(TCHAR)*100);

	// D번지
	// 500000FF03FF00 0038 000A 1401 0000

	_tcscat_s(szCmd, _countof(szCmd), _T("500000FF03FF00"));

	size_t nLen = _tcsclen(val);					//str[n]   = PLC_CHAR_EOT;

	long nWords = 0;
	for (int i = 0; i < nWord; i++)
	{
		if (i < nLen)
		{
			cVal[i] = val[i];
		}
	}
	
	if (nWord % 2)		// 홀수
	{
		nWords = (nWord + 1) / 2;
	}
	else                // 짝수
	{
		nWords = nWord / 2;
	}

	// 4+4+4+8+4+(4*nWord)
	int nTotalNLen = 0;
	nTotalNLen = 4 + 4 + 4 + 8 + 4 + (4 * nWords);
	_stprintf_s(szTmp, _countof(szTmp), _T("%04X"), nTotalNLen);	// 길이 
	_tcscat_s(szCmd, _countof(szCmd), szTmp);  

	// 000A 부터 데이터 길이
	_tcscat_s(szCmd, _countof(szCmd), _T("000A"));				//PLC Time out
	_tcscat_s(szCmd, _countof(szCmd), _T("1401"));				//Command(Write)
	_tcscat_s(szCmd, _countof(szCmd), _T("0000"));				//Sub Command

	_stprintf_s(szTmp, _countof(szTmp), _T("D*%06d"), addr);	_tcscat_s(szCmd, _countof(szCmd), szTmp);

	/*int nLen = 0;
	nLen = nWord / 4;*/

	//정의 필요
	_stprintf_s(szTmp, _countof(szTmp), _T("%04X"), nWords);		_tcscat_s(szCmd, _countof(szCmd), szTmp);

	int j = 0;
	for (int i = 0; i < nWord; i++)
	{
		if (cVal[i] == NULL)
		{
		//	val[i] = '0';
		//	cVal[i] = NULL;		// 2022.04.01 jsyoon change

			cVal[i] = ' ';
		}

		if (cVal[i] == '.') //kjhg 20140804 add 
		{
			cVal[i] = '0';
		} 
	}

	for (int i = 0; i < nWords; i++)
	{
		_stprintf_s(szTmp, _countof(szTmp), _T("%02X%02X"), cVal[2 * i+1], cVal[2 * i]);		_tcscat_s(szCmd, _countof(szCmd), szTmp);
	}

	size_t n = _tcsclen(szCmd);					//str[n]   = PLC_CHAR_EOT;
	szCmd[n] = 0;								//n = 42

	CString strCommand;
	strCommand.Format(_T("%s"), szCmd);

	bool bRet = false;
	if (IsConnected())
	{
		m_bThreadRunStop = true;

		bRet = SendPacket(strCommand, false, false);

		m_bThreadRunStop = false;
	}
	else
	{
		m_pLogFileAccessor->KMLOG(OPER, _T("Fail : MC Protocol is Not Connect"));

		bRet = false;
	}

	CString strLog;
	strLog.Format(_T("Maddress SetMemoryWord(%d) Address : D%d, Send : %s"), bRet, addr, strCommand);
	m_pLogFileAccessor->KMLOG(SEND, strLog);

	return bRet;
}

bool CMcMaddressScanner::CheckActionDone(long addr, DWORD dwTimeout, bool val/* = true*/)
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


long CMcMaddressScanner::GetAddressImportCount()
{
	return dfMADDRESS_HALF_COUNT;
}

long CMcMaddressScanner::GetAddressExportCount()
{
	return dfMADDRESS_HALF_COUNT;
}

bool CMcMaddressScanner::GetAddressImportDataValue(UINT Index)
{
	if( Index >= dfMADDRESS_RD_START)
		Index -= dfMADDRESS_RD_START;

    if( (0 <= Index ) && (Index < dfMADDRESS_HALF_COUNT) )
	{
		return (m_lMcProtocolReadData_clone[Index] == 1) ? true : false;
	}

    return false;
}

bool CMcMaddressScanner::GetAddressExportDataValue(UINT Index)
{
	Index -= dfMADDRESS_RD_START;
	if( (dfMADDRESS_HALF_COUNT <= Index) && (Index < dfMADDRESS_TOTAL_COUNT) )
	{
		return (m_lMcProtocolReadData_clone[Index] == 1) ? true : false;
	}

    return false;
}

SADDRESSMAPDATA *CMcMaddressScanner::GetAddressImportDataPtr()
{
	return &m_vecRData;
}

SADDRESSMAPDATA *CMcMaddressScanner::GetAddressExportDataPtr()
{
	//return &m_vecWData;
	return &m_vecRData;
}

void CMcMaddressScanner::xNetCallbackRecieveData(LPVOID ud)
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

		CString strLog = _T("[RECV - MAddress] : ") + strMessage;
		m_pLogFileAccessor->KMLOG(RECV, strLog);

		if (m_bUseWriteOnly == true)
			return;

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
			//		const CString strCommand = vecPacketCommandPool[i];
			CString strCommand = vecPacketCommandPool[iMsg];

			if (strCommand == PLC_CMD_SUCCESS)
			{
				continue;
			}

			m_pLogFileAccessor->KMLOG(RECV, strCommand);

			/*if (m_lMcProtocolReadData_clone == m_lMcProtocolReadData)
			{
				continue;
			}*/

			bool bUseBit = false;
			bUseBit = m_bUseBit;

			if (bUseBit == true)
			{
				if (strCommand.GetLength() != dfMADDRESS_TOTAL_COUNT)
				{
					CString strLog = _T("[RECV - MAddress Small] : ") + strCommand;
					//CLogWrapper::GetInstance()->KMLOG(SYST, strLog);
					m_pLogFileAccessor->KMLOG(WARN, strLog);

					continue;
				}

				//Bit 처리 방식
				for (int iPos = 0; iPos < dfMADDRESS_TOTAL_COUNT; iPos++)
				{
					TCHAR a = strCommand.GetAt(iPos);				// '1' or '0' ASCII code

					m_lMcProtocolReadData[iPos] = a - '0';			// 숫자로 변환
				}
			}
			else // Word
			{
				BOOL bPLCData[dfMADDRESS_TOTAL_COUNT];
				ZeroMemory(&bPLCData, sizeof(bPLCData));

				CString strTempH, strTempL, str;
				int toByte = 0, tNum = 0;
				int i = 0, j = 0, l = 0, k = 0, n = 0;
				int nCnt = 0;
				int iCnt = (int)floor(dfMADDRESS_TOTAL_COUNT / 16.0);
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
						if (k >= dfMADDRESS_TOTAL_COUNT)
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

							/*if (k < 1000)
							{
								if (tNum % 2)
									bPLCData[k] = TRUE;
								else
									bPLCData[k] = FALSE;

								m_pSystem->m_bPLCData[1000 + k] = bPLCData[k];
							}
							else if (k >= 1000 && k < dfMAX_PLC_IN_CNT)
							{
								if (tNum % 2)
									bPLCData[k] = TRUE;
								else
									bPLCData[k] = FALSE;

								m_pSystem->m_bPLCData[k - 1000] = bPLCData[k];
							}
							else
							{
								break;
							}*/


							tNum /= 2;

							k++;
						}
					}
				}
			}

			memcpy(m_lMcProtocolReadData_clone, m_lMcProtocolReadData, sizeof(long) * dfMADDRESS_TOTAL_COUNT);
		}
	}
}

std::vector<CString> CMcMaddressScanner::GetCommand(CString &strData)
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
		iCnt = dfMADDRESS_TOTAL_COUNT;

		nReciveSize = iCnt + PLC_CMD_HEADER_LENGTH;
		_stprintf_s(temp, _countof(temp), _T("%06X"), iCnt + 4);		_tcscat_s(str, _countof(str), temp);
	}
	else
	{
		iCnt = (int)floor(dfMADDRESS_TOTAL_COUNT / 16.0);
	
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
						strParam = strParam.Mid(ipos + PLC_CMD_HEADER_LENGTH, dfMADDRESS_TOTAL_COUNT);

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

/*
long CMcMaddressScanner::SetMemoryBit(long addr, long val, long nBit/* = 1)
{
	int		stadd = 1;
	TCHAR	szTmp[100];
	TCHAR	szCmd[1000];
	stadd = nBit;

	szTmp[0] = 0x00;
	szCmd[0] = 0x00;

	_tcscat_s(szCmd, _countof(szCmd), _T("500000FF03FF00"));
	_stprintf_s(szTmp, _countof(szTmp), _T("%04X"), 24 + nBit);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
	_tcscat_s(szCmd, _countof(szCmd), _T("0090"));				//PLC Time out
	_tcscat_s(szCmd, _countof(szCmd), _T("1401"));				//Command(Write)
	_tcscat_s(szCmd, _countof(szCmd), _T("0001"));				//Sub Command

	_stprintf_s(szTmp, _countof(szTmp), _T("M*%06d"), addr);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
	_stprintf_s(szTmp, _countof(szTmp), _T("%04X"), stadd);		_tcscat_s(szCmd, _countof(szCmd), szTmp);

	BYTE nByte = 0x00;
	for (int i = 0; i<nBit; i++)
	{
		nByte = (val >> i) & 0x01;
		if (nByte == 0)
			_tcscat_s(szCmd, _countof(szCmd), _T("0"));
		else
			_tcscat_s(szCmd, _countof(szCmd), _T("1"));
	}

	size_t n = _tcsclen(szCmd);					//str[n]   = PLC_CHAR_EOT;
	szCmd[n] = 0;								//n = 42

	CString strCommand;
	strCommand.Format(_T("%s"), szCmd);
	SendPacket(strCommand, false);

	//명령 전송 후 확인이 필요할까???

	/*
	int iLen = strlen(szCmd);
	szCmd[iLen] = 0;

	int iRet = MC_PROTOCOL_SUCCESS;
	iRet = Cmd_Send(m_nSocketIndex2, szCmd);
	if (iRet == MC_PROTOCOL_SUCCESS)
	{
	CString strReply;
	strReply.Empty();
	GetReceiveData(m_nSocketIndex2, &strReply, CMD_WRITE_BIT, PLC_RECIVE_TIMEOUT);
	}
	
	return 1;
}

long CMcMaddressScanner::SetMemoryWord(long addr, float val)
{
	int		stadd = 1;
	TCHAR	szTmp[100];
	TCHAR	szCmd[1000];

	szTmp[0] = 0x00;
	szCmd[0] = 0x00;

	_tcscat_s(szCmd, _countof(szCmd), _T("500000FF03FF00"));
	_stprintf_s(szTmp, _countof(szTmp), _T("%04X"), 24 + 1);	_tcscat_s(szCmd, _countof(szCmd), szTmp);
	_tcscat_s(szCmd, _countof(szCmd), _T("0090"));				//PLC Time out
	_tcscat_s(szCmd, _countof(szCmd), _T("1401"));				//Command(Write)
	_tcscat_s(szCmd, _countof(szCmd), _T("0000"));				//Sub Command

	_stprintf_s(szTmp, _countof(szTmp), _T("D*%06d"), addr);	_tcscat_s(szCmd, _countof(szCmd), szTmp);

	//정의 필요
	_stprintf_s(szTmp, _countof(szTmp), _T("%04X"), 2);		_tcscat_s(szCmd, _countof(szCmd), szTmp);

	FLOAT_FIELD u_FloatField;
	u_FloatField.m_float = val;

	_stprintf_s(szTmp, _countof(szTmp), _T("%04X%04X"), u_FloatField.m_16bit.m_Bottom1Byte, u_FloatField.m_16bit.m_Top1Byte);
	_tcscat_s(szCmd, _countof(szCmd), szTmp);

	size_t n = _tcsclen(szCmd);					//str[n]   = PLC_CHAR_EOT;
	szCmd[n] = 0;								//n = 42

	CString strCommand;
	strCommand.Format(_T("%s"), szCmd);
	SendPacket(strCommand, false);

	return 0;
}
*/