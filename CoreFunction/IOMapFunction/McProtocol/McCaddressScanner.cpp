#include "pch.h"
#include "McCaddressScanner.h"

//#include "McProtocolCtrl.h"
#include "..\..\common\XLS\XLEzAutomation.h"
#include "..\..\LogFunction\LogAccessor\LogFileAccessor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CMcCaddressScanner::CMcCaddressScanner(CString strDefaultFilePath, CMcProtocolCtrl *pMcProtocolCtrl)
//	: CMcAddressScannerImpl(strDefaultFilePath, pMcProtocolCtrl)
CMcCaddressScanner::CMcCaddressScanner(CString strLoggerFolderPath, CString strDefaultFilePath)
	: CMcAddressScannerImpl(strDefaultFilePath)
	, m_nReadType(0)
	, m_bDisconnectCleared(false)	// LYS 2026-09-16
{
	m_strAddressName = _T("ProbeCard");

	ReadFileCSV(m_strDefaultFilePath);

	// 로그 생성하기
	const CString strFolderPath = strLoggerFolderPath + _T("\\LogFile\\");
	const CString strLogFile = strFolderPath + m_strAddressName + _T(".log");
	MakeDir(strFolderPath);
	m_pLogFileAccessor->Initialize(strLogFile);
	//////

	m_iDataLast = dfPROBEADDRESS_TOTAL_COUNT;
	m_strQueueMessage.Empty();

	ZeroMemory(&m_szLoadProbeCardID, sizeof(m_szLoadProbeCardID));
	ZeroMemory(&m_szProbeCardBF, sizeof(m_szProbeCardBF));
	ZeroMemory(&m_szUnloadProbeCardID, sizeof(m_szUnloadProbeCardID));


	// LYS 2026-09-16 구 테스트용 더미 ID(TEST1~5) 제거. 첫 D2100 응답(약 5초) 전까지 P1~P5 모듈에 프로브가 있는 것처럼 보였다.
	//                현 설비(OCP)는 P5 1개뿐이고 [1..4]는 존재하지 않으므로 첫 읽기 전에는 전부 빈 문자열로 둔다.
	//_tcscpy_s(m_szUnloadProbeCardID[1], _countof(m_szUnloadProbeCardID[1]), _T("TEST1"));
	//_tcscpy_s(m_szUnloadProbeCardID[2], _countof(m_szUnloadProbeCardID[2]), _T("TEST2"));
	//_tcscpy_s(m_szUnloadProbeCardID[3], _countof(m_szUnloadProbeCardID[3]), _T("TEST3"));
	//_tcscpy_s(m_szUnloadProbeCardID[4], _countof(m_szUnloadProbeCardID[4]), _T("TEST4"));
	//_tcscpy_s(m_szUnloadProbeCardID[5], _countof(m_szUnloadProbeCardID[5]), _T("TEST5"));

	// LYS 2026-09-16 구 테스트용 더미 버퍼 ID(21ABCDEDFH) 제거: 첫 D2210 응답 전까지 버퍼 1 에 프로브가 있는 것처럼 보였다(m_szBufferPROBEID[0] 로 전달됨).
	//	_tcscpy_s(m_szProbeCardBF[0], _countof(m_szProbeCardBF[0]), _T("21ABCDEDFH"));


	memset(m_lMcProtocolReadData, 0, sizeof(long) * dfPROBEADDRESS_TOTAL_COUNT);
	memcpy(m_lMcProtocolReadData_clone, m_lMcProtocolReadData, sizeof(long) * dfPROBEADDRESS_TOTAL_COUNT);

	m_hRecvDEvent = CreateEvent(NULL, TRUE, FALSE, NULL);	// Manual reset, initial reset
}

CMcCaddressScanner::~CMcCaddressScanner()
{

	CloseHandle(m_hRecvDEvent);
}

#define PLC_D_READ_UNLOAD_BIT					2100

#define PLC_D_READ_STARY_BIT					2400
#define	dfMAX_PLC_IN_CNTD						10*6 // EQ Load / Unload Data, Slot 1~ 5
#define	dfMAX_PLC_IN_CNTD_LOAD					10*8 // LYS 2026-09-16 Load(D2400) 읽기: EQ Load Data + CST Slot 1~7 (D2400~D2479). Unload(D2100) 쪽은 6블록 유지
#define	dfPROBE_LOAD_BLOCK_CNT					8    // LYS 2026-09-16 위 80워드 = 10워드 블록 8개
#define	dfPROBE_UNLOAD_BLOCK_CNT				6    // LYS 2026-09-16 Unload(D2100) 60워드 = [0] EQ Unload Data + [1..5] Probe Module #1~#5. 현 설비(OCP)는 [5](D2150, P5)만 유효

#define PLC_D_READ_PROBE_BUFFER					2210
#define	dfMAX_READ_PROBE_BUFFER					10*2	// 2

bool CMcCaddressScanner::ReadProbeLoadData()
{
	bool bRet = false;
	m_nReadType = 0;
	//bRet = ReadCommand(PLC_D_READ_STARY_BIT, dfMAX_PLC_IN_CNTD);
	bRet = ReadCommand(PLC_D_READ_STARY_BIT, dfMAX_PLC_IN_CNTD_LOAD);	// LYS 2026-09-16 Cassette 5->7 슬롯

	CString strLog;
	//strLog.Format(_T("ReadProbeLoadData(%d) : %s, %s, %s, %s, %s, %s"), bRet, m_szLoadProbeCardID[0], m_szLoadProbeCardID[1], m_szLoadProbeCardID[2], m_szLoadProbeCardID[3], m_szLoadProbeCardID[4], m_szLoadProbeCardID[5]);
	//	strLog.Format(_T("ReadProbeLoadData(%d) : %s, %s, %s, %s, %s, %s, %s, %s"), bRet, m_szLoadProbeCardID[0], m_szLoadProbeCardID[1], m_szLoadProbeCardID[2], m_szLoadProbeCardID[3], m_szLoadProbeCardID[4], m_szLoadProbeCardID[5], m_szLoadProbeCardID[6], m_szLoadProbeCardID[7]);	// LYS 2026-09-16 slot 6,7
	strLog.Format(_T("ReadProbeLoadData send(%d) D%d x%d words"), bRet, PLC_D_READ_STARY_BIT, dfMAX_PLC_IN_CNTD_LOAD);	// LYS 2026-09-16 값은 파싱 후 LogProbeIDValues 가 기록
	m_pLogFileAccessor->KMLOG(OPER, strLog);

	return bRet;
}

bool CMcCaddressScanner::ReadProbeBufferData()
{
	bool bRet = false;
	m_nReadType = 1;
	bRet = ReadCommand(PLC_D_READ_PROBE_BUFFER, dfMAX_READ_PROBE_BUFFER);

	CString strLog;
	//	strLog.Format(_T("ReadProbeBufferData(%d) : %s, %s"), bRet, m_szProbeCardBF[0], m_szProbeCardBF[1]);
	strLog.Format(_T("ReadProbeBufferData send(%d) D%d x%d words"), bRet, PLC_D_READ_PROBE_BUFFER, dfMAX_READ_PROBE_BUFFER);	// LYS 2026-09-16 값은 파싱 후 LogProbeIDValues 가 기록
	m_pLogFileAccessor->KMLOG(OPER, strLog);

	return bRet;
}

bool CMcCaddressScanner::ReadProbeUnloadData()
{
	bool bRet = false;
	m_nReadType = 2;
	bRet = ReadCommand(PLC_D_READ_UNLOAD_BIT, dfMAX_PLC_IN_CNTD);

	CString strLog;
	//	strLog.Format(_T("ReadProbeUnloadData(%d) : %s, %s, %s, %s, %s, %s"), bRet, m_szUnloadProbeCardID[0], m_szUnloadProbeCardID[1], m_szUnloadProbeCardID[2], m_szUnloadProbeCardID[3], m_szUnloadProbeCardID[4], m_szUnloadProbeCardID[5]);
	strLog.Format(_T("ReadProbeUnloadData send(%d) D%d x%d words"), bRet, PLC_D_READ_UNLOAD_BIT, dfMAX_PLC_IN_CNTD);	// LYS 2026-09-16 값은 파싱 후 LogProbeIDValues 가 기록
	m_pLogFileAccessor->KMLOG(OPER, strLog);

	return bRet;
}

bool CMcCaddressScanner::ReadCommand(long addr, long nCount)
{
	int		stadd = 1;
	TCHAR	szTmp[100];
	TCHAR	szCmd[1000];

	szTmp[0] = 0x00;
	szCmd[0] = 0x00;

	_tcscat_s(szCmd, _countof(szCmd), _T("500000FF03FF00"));

	// 4+4+4+8+4+(4*nWord)
	int nTotalNLen = 0;
	nTotalNLen = 4 + 4 + 4 + 8 + 4;
	_stprintf_s(szTmp, _countof(szTmp), _T("%04X"), nTotalNLen);	// 길이 
	_tcscat_s(szCmd, _countof(szCmd), szTmp);
	//_stprintf_s(szTmp, _countof(szTmp), _T("%04X"), 24);	    _tcscat_s(szCmd, _countof(szCmd), szTmp);

	// PLC Time out 부터 데이터 길이
	_tcscat_s(szCmd, _countof(szCmd), _T("0090"));				//PLC Time out
	_tcscat_s(szCmd, _countof(szCmd), _T("0401"));				//Command(Read)
	_tcscat_s(szCmd, _countof(szCmd), _T("0000"));				//Sub Command

	_stprintf_s(szTmp, _countof(szTmp), _T("D*%06d"), addr);	_tcscat_s(szCmd, _countof(szCmd), szTmp);

	//정의 필요
	_stprintf_s(szTmp, _countof(szTmp), _T("%04X"), nCount);		_tcscat_s(szCmd, _countof(szCmd), szTmp);

	size_t n = _tcsclen(szCmd);
	szCmd[n] = 0;

	CString strCommand;
	strCommand.Format(_T("%s"), szCmd);
	CString strLog;

	ResetEvent(m_hRecvDEvent);

	bool bRet = false;
	bRet = IsConnected();
	if (bRet == true)
	{
		strLog.Format(_T("ReadCommand(%d) Address : %d, %s"), bRet, addr, strCommand);
		m_pLogFileAccessor->KMLOG(SEND, strLog);

		bRet = SendPacket(strCommand, false, false);
		if (bRet == true)
		{
			bRet = false;

			DWORD  dwS, dwR, dwT = 0;
			DWORD  dwSleep = 100;

			for (dwS = GetTickCount(); dwT < 1000;)
			{
				if (WaitForSingleObject(m_hRecvDEvent, 10)
					== WAIT_OBJECT_0)
				{
					bRet = true;
					break;
				}

				dwR = GetTickCount();

				if (dwR >= dwS)
					dwT = dwR - dwS;
				else
					dwT = ((DWORD)0xFFFFFFFF) - (dwS - dwR);
			}

			if (bRet != true)
			{
				strLog.Format(_T("ReadCommand, Rcv TimeOut=%d)...TimeOut"), 1000);
				m_pLogFileAccessor->KMLOG(WARN, strLog);
			}
		}
		else
		{
			strLog.Format(_T("ReadCommand(%d) Address : %d, %s, Send Fail"), bRet, addr, strCommand);
			m_pLogFileAccessor->KMLOG(WARN, strLog);
		}
	}
	else
	{
		strLog.Format(_T("ReadCommand(%d) Address : %d, %s, Not Connected"), bRet, addr, strCommand);
		m_pLogFileAccessor->KMLOG(WARN, strLog);
	}

	return bRet;
}

// D번지 읽기
bool CMcCaddressScanner::SendCommand()
{
	bool bRet = false;
	bRet = ReadProbeLoadData();

	if( bRet == true) bRet = ReadProbeBufferData();

	if (bRet == true) bRet = ReadProbeUnloadData();

	return bRet;

//	if (IsConnected() == false)
//		return false;
//
	//int		stadd = 1;
	//int		stadd8 = 8;
	//TCHAR	temp[20];
	//TCHAR	str[100];
	//str[0] = 0;
	////		strcat(str, PLC_CMD_ETHERNET_SINGLE_READ);		//5000(헤드) 00FF(번호) 03FF(모듈번호) 00(국번) 0018(길이) 0090(타이머)  0401(읽기) 0001(비트단위) // 0000(워드단위)
	//_tcscat_s(str, _countof(str), _T(PLC_CMD_ETHERNET_SINGLE_READ));		//Bit
	//																		//_tcscat_s(str, _countof(str), _T(PLC_CMD_ETHERNET_WORD_READ_D));		//Word
	//																			//500000FF03FF000018009004010001
	//short bit = 1;

	////4000번 부터 일괄적으로 읽을 것임
	////	sprintf(temp, "M*%06d", PLC_READ_ALL_ADRESS);		strcat(str, temp); //M*004000, ex)address 4000
	//// 	sprintf(temp, "%04X", dfMAX_PLC_IN_CNT);						strcat(str, temp); //200 ==> C8 (Hexa)
	//_stprintf_s(temp, _countof(temp), _T("M*%06d"), dfMADDRESS_RD_START);		_tcscat_s(str, _countof(str), temp); //M*001001, ex)address 1001		// 시작번지
	//_stprintf_s(temp, _countof(temp), _T("%04X"), dfMADDRESS_TOTAL_COUNT);		_tcscat_s(str, _countof(str), temp); //stadd 1 ==> 0001					// 읽을 비트 수

	//size_t n = _tcsclen(str);					//str[n]   = PLC_CHAR_EOT;
	//str[n] = 0;								//n = 42

	//CString strCommand;
	//strCommand.Format(_T("%s"), str);
	////SetMemoryBit(str);
	//SendPacket(str);


	return true;
}

bool CMcCaddressScanner::SetMemoryWord(long addr, float* val, long nWord/* = 1*/)
{
	int		stadd = 1;
	TCHAR	szTmp[100];
	TCHAR	szCmd[1000];

	szTmp[0] = 0x00;
	szCmd[0] = 0x00;

	_tcscat_s(szCmd, _countof(szCmd), _T("500000FF03FF00"));

	_stprintf_s(szTmp, _countof(szTmp), _T("%04X"), 24);	    _tcscat_s(szCmd, _countof(szCmd), szTmp);
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

	if (IsConnected())
	{
		CString strCommand;
		strCommand.Format(_T("%s"), szCmd);
		bool bRet = SendPacket(strCommand, false, false);

		CString strLog;
		strLog.Format(_T("SetMemoryWord(%d) Address : %d, Val : %d, %s"), bRet, addr, val, strCommand);
		m_pLogFileAccessor->KMLOG(SEND, strLog);

		return true;
	}
	else
	{
		m_pLogFileAccessor->KMLOG(OPER, _T("Fail : MC Protocol is Not Connect"));
	}

	return false;
}

long CMcCaddressScanner::Initialize(bool bRunMode, CString strIP, long nPort)
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
	}
	else
	{
		//	m_bMcProtocolIni = false;
	}

	return lRet;
}

long CMcCaddressScanner::Close()
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

bool CMcCaddressScanner::IsConnected()
{
	return CCommEthFunction::IsConnected();
}

void CMcCaddressScanner::DisConnect()
{
	CCommEthFunction::CloseConnect();
}

void CMcCaddressScanner::ThreadRun()
{
	if (IsConnected() == true)
	{
		SendCommand();

		Sleep(5000);

		//memset(m_lMcProtocolReadData, 0, sizeof(long) * dfPROBEADDRESS_TOTAL_COUNT);

		////m_pMcProtocolCtrl->GetMemoryBitBlock(_T("D2200"), m_iDataLast, m_lMcProtocolReadData);

		//memcpy(m_lMcProtocolReadData_clone, m_lMcProtocolReadData, sizeof(long) * dfPROBEADDRESS_TOTAL_COUNT);
		m_bDisconnectCleared = false;	// LYS 2026-09-16
	}
	//else
	//{
	//	ZeroMemory(&m_szLoadProbeCardID, sizeof(m_szLoadProbeCardID));
	//	ZeroMemory(&m_szProbeCardBF, sizeof(m_szProbeCardBF));
	//}
	else
	{
		// LYS 2026-09-16 연결이 끊기면 옛 ID 가 그대로 남아 SACC 화면이 끊긴 줄 모른 채 옛 값을 보여 줬다. 끊김 시 1회만 세 배열을 비우고 WARN 을 남긴다.
		//                (재연결되면 다음 주기 응답으로 다시 채워진다. 이 함수는 끊긴 동안 약 10ms 마다 호출되므로 플래그로 1회만 처리)
		if (m_bDisconnectCleared == false)
		{
			ZeroMemory(&m_szLoadProbeCardID, sizeof(m_szLoadProbeCardID));
			ZeroMemory(&m_szProbeCardBF, sizeof(m_szProbeCardBF));
			ZeroMemory(&m_szUnloadProbeCardID, sizeof(m_szUnloadProbeCardID));
			m_pLogFileAccessor->KMLOG(WARN, _T("ProbeCard scanner disconnected - Load/Buffer/Unload probe IDs cleared until reconnect"));
			m_bDisconnectCleared = true;
		}
	}
}

bool CMcCaddressScanner::ReadFileCSV(CString strFileName)
{
	m_vecRData.Clear();
	m_vecRData.vecData.resize(dfPROBEADDRESS_TOTAL_COUNT);

	std::vector<CString> vecErrorMemory;

	CStdioFile File(strFileName, CFile::modeRead);

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

			if ((0 <= nAddress) && (nAddress < dfPROBEADDRESS_LIMIT_COUNT))
			{
				if (strLine.IsEmpty() == false)
				{
					nAddress -= dfPROBEADDRESS_RD_START;

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
		strLog.Format(_T("There are '%d' PROBE Address Numbers Out of range.\nPlease Refer to the Log."), vecErrorMemory.size());
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

bool CMcCaddressScanner::ReadFileXLS(CString strFileName)
{
	m_vecRData.Clear();
	m_vecRData.vecData.resize(dfPROBEADDRESS_TOTAL_COUNT);

	std::vector<CString> vecErrorMemory;

	CFile file(strFileName, CFile::shareDenyNone);
	CArchive ar(&file, CArchive::load);
	CXLEzAutomation ListExcel(FALSE);
	if (ListExcel.OpenExcelFile(strFileName) == TRUE)
	{
		int iaccess_offset = 2;
		for (int i = iaccess_offset; i < (int)(dfPROBEADDRESS_TOTAL_COUNT + iaccess_offset); i++)
		{
			CString strAddress = ListExcel.GetCellValue(1, i);
			CString strDescription = ListExcel.GetCellValue(2, i);

			CString strTemp = strAddress;
			strTemp.Replace(_T("D"), _T(""));
			UINT nAddress = _ttoi(strTemp);

			if ((0 <= nAddress) && (nAddress < dfPROBEADDRESS_LIMIT_COUNT))
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
		strLog.Format(_T("There are '%d' PROBE Address Numbers Out of range.\nPlease Refer to the Log."), vecErrorMemory.size());
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

bool CMcCaddressScanner::ReadFileVerify(CString strFileName)
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

			if (0 <= nAddress && nAddress <= dfPROBEADDRESS_LIMIT_COUNT)
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

bool CMcCaddressScanner::SetDefaultFile(CString strFileName)
{
	if (CopyFile(strFileName, m_strDefaultFilePath, FALSE) == TRUE)
	{
		return ReadFileCSV(m_strDefaultFilePath);
	}

	return false;
}

long CMcCaddressScanner::GetAddressImportCount()
{
	return dfPROBEADDRESS_TOTAL_COUNT;
}

long CMcCaddressScanner::GetAddressExportCount()
{
	return 0;
}

bool CMcCaddressScanner::GetAddressImportDataValue(UINT Index)
{
	if ((0 <= Index) && (Index < dfPROBEADDRESS_TOTAL_COUNT))
	{
		return (m_lMcProtocolReadData_clone[Index] == 1) ? true : false;
	}

	return false;
}


void CMcCaddressScanner::GetAddressExportDataValueArray(TCHAR** pValue, long* plSize)
{
	for (size_t i = 0; i < 2; i++)
	{
		_tcscpy_s(pValue[i], sizeof(TCHAR)*32, m_szProbeCardBF[i]);			// Buffer
		plSize++;
	}

	for (size_t i = 0; i < 10; i++)
	{
		_tcscpy_s(pValue[2+i], sizeof(TCHAR) * 32, m_szLoadProbeCardID[i]);		// Cassette
		plSize++;
	}

	for (size_t i = 0; i < 10; i++)
	{
		_tcscpy_s(pValue[12+i], sizeof(TCHAR) * 32, m_szUnloadProbeCardID[i]);	// EQ ProbeID
		plSize++;
	}
}

SADDRESSMAPDATA *CMcCaddressScanner::GetAddressImportDataPtr()
{
	return &m_vecRData;
}





long CMcCaddressScanner::GetAlarmMapCount()
{
	std::vector<SAMapString> vecAlarm;

	for (UINT nCount = 0; nCount < dfPROBEADDRESS_TOTAL_COUNT; nCount++)
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

bool CMcCaddressScanner::IsOccurFAlarm()
{
	return (0 < GetAlarmMapCount());
}

void CMcCaddressScanner::xNetCallbackRecieveData(LPVOID ud)
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

		CString strLog = _T("[RECV - PROBEID] : ") + strMessage;
		m_pLogFileAccessor->KMLOG(RECV, strLog);

	/*	CString strMessage(pRcvData->szBody);
		if (strMessage.IsEmpty() == true)
			return;

		CString strLog = _T("[RECV - PROBEID] : ") + strMessage;
		m_pLogFileAccessor->KMLOG(RECV, strLog);*/
		//CLogWrapper::GetInstance()->KMLOG(SYST, strLog);

		//---------------------------------------------------------------------------
		// 패킷 단위로 응답에 대한 처리를 해야 한다.
		// socket 에 데이터가 진입하는데 2개 이상 붙어서 1개의 event 로 들어온다.
		// 모든 패킷에 대해서 command 단위로 분리해서 수행해야 한다.
		//---------------------------------------------------------------------------
		std::vector<CString> vecPacketCommandPool = GetCommand(strMessage);

		SetEvent(m_hRecvDEvent);

		for (size_t iMsg = 0; iMsg < vecPacketCommandPool.size(); ++iMsg)
		{
			CString strCommand = vecPacketCommandPool[iMsg];

			m_pLogFileAccessor->KMLOG(RECV, strCommand);

			if (strCommand == PLC_CMD_SUCCESS)
			{
				continue;
			}
			else
			{
				UpdateProbeID(strCommand, m_nReadType);
			}
		}
	}
}

std::vector<CString> CMcCaddressScanner::GetCommand(CString& strData)
{
	CString strMessage = strData;

	int iCnt = 0;
	int nReciveSize = 0;
	int nHeaderPos = 0;
	int nReplySize = 0;

	std::vector<CString> vecResult;

	// Header
	TCHAR	temp[20];
	TCHAR	str[100];
	str[0] = 0;
	

	// D00000FF03FF0001940000
	// D00000FF03FF0000540000
	// Word
	//{
	//	iCnt = (int)floor(dfMADDRESS_TOTAL_COUNT / 16.0);

	//	nReciveSize = iCnt * 4 + PLC_CMD_HEADER_LENGTH;
	//	_stprintf_s(temp, _countof(temp), _T("%06X"), (iCnt * 4) + 4);	_tcscat_s(str, _countof(str), temp);
	//}

	//_stprintf_s(temp, _countof(temp), _T("%04X"), 0);	    _tcscat_s(str, _countof(str), temp);
	//size_t n = _tcsclen(str);					//str[n]   = PLC_CHAR_EOT;
	//str[n] = 0;

	CString strCommand;
	strCommand.Format(_T("%s"), _T("D00000FF03FF"));
	CString strTmep;

	while (1)
	{
		if (strMessage.IsEmpty() == true)
			break;

		const int ipos = strMessage.Find(strCommand);

		if (ipos < 0)
		{
			break;
		}
		else if (ipos == 0)
		{
			CString strCmd = strMessage;
			strCmd.Delete(0, PLC_CMD_HEADER_LENGTH);
			const int ipos2 = strCmd.Find(strCommand);

			if (ipos2 < 0)
			{
				CString strParam = strMessage;

				vecResult.push_back(strParam);

				strMessage.Replace(strParam, _T(""));

				strMessage.TrimLeft();
			}
			else
			{
				CString strParam = strMessage.Mid(ipos, ipos2 + PLC_CMD_HEADER_LENGTH);
				vecResult.push_back(strParam);

				strMessage.Replace(strParam, _T(""));

				strMessage.TrimLeft();
			}
		}
		else
		{
			CString strParam = strMessage.Left(ipos);
			if (strParam.IsEmpty() == true)
				break;

			strMessage = strParam;
		}
	}

	return vecResult;
}

void CMcCaddressScanner::UpdateProbeID(CString strCommand, int iType)
{
	int i = 0, k = 0, nCnt = 0;
	long nCount = 0;

	switch (iType)
	{
	case 0:
		//nCount = dfMAX_PLC_IN_CNTD;
		//nCnt = 6;
		nCount = dfMAX_PLC_IN_CNTD_LOAD;	// LYS 2026-09-16 Load 응답 헤더 길이 = 블록 8개 기준
		nCnt = dfPROBE_LOAD_BLOCK_CNT;
			break;
	case 1:
		nCount = dfMAX_PLC_IN_CNTD;
		nCnt = 2;
		break;
	case 2:
		nCount = dfMAX_PLC_IN_CNTD;
		//nCnt = 6;
		nCnt = dfPROBE_UNLOAD_BLOCK_CNT;	// LYS 2026-09-16
		break;
	default:
		break;
	}

	// Header
	TCHAR	temp[20];
	TCHAR	str[100];
	str[0] = 0;
	_tcscat_s(str, _countof(str), _T("D00000FF03FF"));
	
	int nReciveSize = 0;
	nReciveSize = (nCnt * 40) + 4;
	_stprintf_s(temp, _countof(temp), _T("%06X"), nReciveSize);	_tcscat_s(str, _countof(str), temp);
	
	_stprintf_s(temp, _countof(temp), _T("%04X"), 0);	    _tcscat_s(str, _countof(str), temp);
	size_t n = _tcsclen(str);
	str[n] = 0;

	CString strHeader;
	strHeader.Format(_T("%s"), str);

	int nHeaderPos = 0;
	nHeaderPos = strCommand.Find(strHeader);

	if (nHeaderPos < 0)
	{
		// LYS 2026-09-16 헤더(기대 길이가 박힌 문자열)가 안 맞으면 조용히 옛 값을 유지하던 것을 WARN 으로 드러낸다.
		//                PLC 가 D 범위 오류 등으로 종료 코드 != 0000 을 돌려주거나 블록 수(기대 길이)가 PLC 설정과 다를 때 여기로 온다. 같은 내용은 1회만 기록.
		CString strRecvLen, strEndCode;
		if (strCommand.GetLength() >= 22 && strCommand.Left(12) == _T("D00000FF03FF"))
		{
			strRecvLen = strCommand.Mid(12, 6);
			strEndCode = strCommand.Mid(18, 4);
		}
		CString strWarn;
		strWarn.Format(_T("UpdateProbeID(type %d) header mismatch - expected len %06X (%d blocks), recv len [%s] end code [%s] : %s"),
			iType, nReciveSize, nCnt, strRecvLen, strEndCode, strCommand.Left(40));
		if (strWarn != m_strLastParseWarn)
		{
			m_pLogFileAccessor->KMLOG(WARN, strWarn);
			m_strLastParseWarn = strWarn;
		}
		return;
	}
	m_strLastParseWarn.Empty();	// LYS 2026-09-16 header OK again -> next mismatch is logged

	char* StopStr;
	CString strProbeID, strTempH, strTempL, strTemp;
	ULONG   lTempH = 0, lTempL = 0;

	CString strParam = strCommand;
	strParam = strParam.Mid(nHeaderPos + PLC_CMD_HEADER_LENGTH, nCnt * 40);


	TCHAR szProbeCardBF[10][32];	// Buffer
	ZeroMemory(&szProbeCardBF, sizeof(szProbeCardBF));

	// 총 읽은 개수
	for (i = 0; i < nCnt; i++)
	{
		strProbeID.Empty();

		for (k = 0; k < 10; k++)
		{
			strTempH = strParam.Mid((40 * i) + (k * 2) * 2, 2);
			strTempL = strParam.Mid((40 * i) + (k * 2) * 2 + 2, 2);
			

			USES_CONVERSION;
			char	str1Word[4096];
			memset(str1Word, NULL, 4096);

			strcpy_s(str1Word, T2A(strTempH));
			lTempH = strtoul(str1Word, &StopStr, 16);

			memset(str1Word, NULL, 4096);
			strcat_s(str1Word, T2A(strTempL));
			lTempL = strtoul(str1Word, &StopStr, 16);

			TCHAR cCh1 = (TCHAR)lTempH;
			TCHAR cCh2 = (TCHAR)lTempL;

			if (cCh1 == 0)
				cCh1 = '0';
			if (cCh2 == 0)
				cCh2 = '0';

			strTempH.Format(_T("%c"), cCh2);
			strTempL.Format(_T("%c"), cCh1);
			strTemp.Format(_T("%s%s"), strTempH, strTempL);

			strProbeID += strTemp;
		}

		strProbeID.TrimRight();

		// LYS 2026-09-16 빈 ID 정규화: PLC 는 빈 ID 를 공백 10자(0x2020 x5 워드)로 두는데 뒤 5워드가 0x0000 -> '0' 로 바뀌어 TrimRight 가 공백을 못 지웠다.
		//                결과 "          "(공백 10자) 가 m_szUnloadProbeCardID -> m_szPROBEID[4] 로 흘러 IsEmpty()/_tcslen() 판정이 항상 '프로브 있음' 이 되고,
		//                Probe Align 레시피명이 "PROBE_MEAS_  " 가 됐다. 10자 잘라낸 뒤 앞뒤 공백을 지워 빈 ID 는 "" 로 저장한다(Load 쪽 "0000000000" 은 그대로).
		//_tcscpy_s(szProbeCardBF[i], _countof(szProbeCardBF[i]), strProbeID.Mid(0, 10));
		CString strProbeID10 = strProbeID.Mid(0, 10);
		strProbeID10.Trim();
		_tcscpy_s(szProbeCardBF[i], _countof(szProbeCardBF[i]), strProbeID10);

		//strLog.Format("[RCVE] BUFFER(%d) : %s ", i + 1, strRFID.Mid(0, 9));
		//EventLog(strLog); //lhj 160902 add
	}


	switch (iType)
	{
	case 0:
	{
		//for (size_t i = 0; i < 6; i++)
		for (size_t i = 0; i < dfPROBE_LOAD_BLOCK_CNT; i++)	// LYS 2026-09-16 [0]=EQ Load, [1..7]=CST Slot 1~7
		{
			_tcscpy_s(m_szLoadProbeCardID[i], _countof(m_szLoadProbeCardID[i]), szProbeCardBF[i]);
		}
	}
		break;
	case 1:
		for (size_t i = 0; i < 2; i++)
		{
			_tcscpy_s(m_szProbeCardBF[i], _countof(m_szProbeCardBF[i]), szProbeCardBF[i]);
		}
		break;
	case 2:
		//for (size_t i = 0; i < 6; i++)
		for (size_t i = 0; i < dfPROBE_UNLOAD_BLOCK_CNT; i++)	// LYS 2026-09-16 [0]=EQ Unload, [1..5]=Probe Module 1~5 (OCP: [5]=P5 만 사용)
		{
			_tcscpy_s(m_szUnloadProbeCardID[i], _countof(m_szUnloadProbeCardID[i]), szProbeCardBF[i]);
		}
		break;
	default:
		break;
	}

	LogProbeIDValues(iType);	// LYS 2026-09-16 파싱 직후 값 기록(응답 전에 찍던 OPER 로그 대체)
}

// LYS 2026-09-16 읽기 명령 직후 찍던 OPER 로그는 항상 직전 주기 값이었다(응답은 비동기). 파싱이 끝난 뒤 실제 수신 값을 남긴다.
void CMcCaddressScanner::LogProbeIDValues(int iType)
{
	CString strLog, strTmp;
	switch (iType)
	{
	case 0:
		strLog = _T("RECV ProbeLoadData(D2400 EQ Load, CST1~7) :");
		for (int i = 0; i < dfPROBE_LOAD_BLOCK_CNT; i++) { strTmp.Format(_T("%s[%s]"), (i == 0) ? _T(" ") : _T(", "), m_szLoadProbeCardID[i]); strLog += strTmp; }
		break;
	case 1:
		strLog.Format(_T("RECV ProbeBufferData(D2210 BUF1, BUF2) : [%s], [%s]"), m_szProbeCardBF[0], m_szProbeCardBF[1]);
		break;
	case 2:
		strLog = _T("RECV ProbeUnloadData(D2100 EQ Unload, P1~P5) :");
		for (int i = 0; i < dfPROBE_UNLOAD_BLOCK_CNT; i++) { strTmp.Format(_T("%s[%s]"), (i == 0) ? _T(" ") : _T(", "), m_szUnloadProbeCardID[i]); strLog += strTmp; }
		break;
	default:
		return;
	}
	m_pLogFileAccessor->KMLOG(OPER, strLog);
}

