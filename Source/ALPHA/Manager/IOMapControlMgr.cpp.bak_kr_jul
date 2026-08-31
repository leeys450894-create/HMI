//#include "stdafx.h"
#include "pch.h"
#include "IOMapControlMgr.h"

#include "IOMapFunction\IOMapFunction.h"
#include "Common\IniHelper.h"
#include "IOMapFunction\DataStructure.h"
#include "..\AuroraFuncDefine.h"
#include "MCCLogMgr.h"
//#include "Alarm\AlarmIDList.h"
//#include "AlarmIDList.h"  // jsyoon 2022.01,.19 change


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CIOMapControlMgr::CIOMapControlMgr(CString strAppPath, CString strConfigPath, CString strLogPath)
    : m_pIniHelper(new CIniHelper)
	, m_pLogFileAccessor(new CLogFileAccessor)
	, m_bInitialized(false)
{
    SetName(_T("IO_MAP_CONTROL_MGR"));

	// 로그 생성하기
	const CString strFolderPath = strLogPath + _T("Control\\IOControl\\");
	const CString strLogFile = strFolderPath + _T("IOControl.log");
	MakeDir(strFolderPath);
	m_pLogFileAccessor->Initialize(strLogFile);
	//////

	const CString strFilePath = strConfigPath + _T("\\CoreFunction\\CoreFunction.ini");
    m_pIniHelper->SetFilePath(strFilePath);

    LinkIniFile(false);

	m_piPLCMAdr = new int[2000];
	for (int i = 0; i < 2000; i++)
	{
		m_piPLCMAdr[i] = 0;
	}

	ReadAdressFileCSV(strConfigPath + _T("\\CoreFunction\\IOMapConfig\\PLCMAdress.csv"));

//	const CString strFolderPath = strLogFilePath + _T("\\LogFile\\");
//	MakeDir(strFolderPath);

    //----------------------------------------------------------------------
    //----------------------------------------------------------------------	
	m_pFunction = new CIOFunction(m_pLogFileAccessor, m_strControlType, strConfigPath, strFilePath, GetName());

	InitDevice(0);

	if (IsOutOfService() == true)
	{
		int nAddress = 0;
		for (int i = 0; i < 8; i++)
		{
			nAddress = PLC_BIT_DOOR_SENSOR1 + i;
			SetMemoryBitToDevice(nAddress, false);
		}

		SetMemoryBitToDevice(PLC_BIT_RUN_TEACH_SWITCH, true);
		SetMemoryBitToDevice(PLC_LOADER_ARM_SENSOR_01, true);
		SetMemoryBitToDevice(PLC_BIT_SUTTER_SENSOR_CLOSE, true);
		SetMemoryBitToDevice(PLC_R_PACC_SHUTTER_CLOSE_01, true);
		SetMemoryBitToDevice(PLC_R_PACC_SHUTTER_CLOSE_02, true);
	}

	//// jsyoon 20210722 add Init
	//SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_011, false);	// interlock off
	//SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_007, true);
	//SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_008, true);
	//SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_009, true);
	//SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_010, true);
	//SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_014, true);


	//SetSinalTowerIdle();
}

CIOMapControlMgr::~CIOMapControlMgr()
{
    Finalize();

	if (m_piPLCMAdr != NULL)
	{
		delete[] m_piPLCMAdr;
	}

	//----------------------------------------------------------------------
    // 데이터 삭제
    //----------------------------------------------------------------------
	if (m_pLogFileAccessor)
	{
		delete m_pLogFileAccessor;
		m_pLogFileAccessor = nullptr;
	}
}

bool CIOMapControlMgr::LinkIniFile(bool bSave)
{
    m_pIniHelper->Link(bSave, GetName(), _T("USE"),							&m_bUse,				true);
    m_pIniHelper->Link(bSave, GetName(), _T("OUT_OF_SERVICE"),				&m_bOutOfService,		false);
    m_pIniHelper->Link(bSave, GetName(), _T("DEVICE_TYPE"),					&m_strControlType,		(CString)_T("MXCOMPONENT"));


	CString strLog;
	strLog.Format(_T("USE:%d, OUT_OF_SERVICE:%d, DEVICE_TYPE:%s"), m_bUse, m_bOutOfService, m_strControlType);
	m_pLogFileAccessor->KMLOG(SYST, strLog);

	return m_pIniHelper->IsReadEnable();
}

void CIOMapControlMgr::Finalize()
{
    //----------------------------------------------------------------------
    // 데이터 삭제
    //----------------------------------------------------------------------
	if(m_pFunction)
	{
		delete m_pFunction;
		m_pFunction = nullptr;
	}
	
	if(m_pIniHelper)
    {
        delete m_pIniHelper;
        m_pIniHelper = nullptr;
    }
}

void CIOMapControlMgr::DisConnect()
{
	if (m_pFunction)
	{
		m_pFunction->DisConnect();
	}
}

// RED
bool CIOMapControlMgr::SetSinalTowerAlarm()
{
	bool ret = true;

#ifdef _USE_IOBOARD
	ret &= SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_021, true);
	ret &= SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_022, false);
	ret &= SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_023, false);
#endif // _USE_IOBOARD

	return ret;
}

// YELLOW
bool CIOMapControlMgr::SetSinalTowerIdle()
{
	bool ret = true;

#ifdef _USE_IOBOARD
	ret &= SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_021, false);
	ret &= SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_022, true);
	ret &= SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_023, false);
#endif // _USE_IOBOARD

	return ret;
}

// GREEN
bool CIOMapControlMgr::SetSinalTowerRun()
{
	bool ret = true;

#ifdef _USE_IOBOARD
	ret &= SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_021, false);
	ret &= SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_022, false);
	ret &= SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_023, true);
#endif // _USE_IOBOARD

	return ret;
}

// YELLOW & GREEN
bool CIOMapControlMgr::SetSinalTowerWait()
{
	bool ret = true;

#ifdef _USE_IOBOARD
	ret &= SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_021, false);
	ret &= SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_022, true);
	ret &= SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_023, true);
#endif // _USE_IOBOARD

	return ret;
}


bool CIOMapControlMgr::SetPickertoStage()
{
	if (m_bOutOfService == true)
		return true;

	bool ret = true;

#ifdef _USE_IOBOARD

	ret &= SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_029, false);
	ret &= SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_031, true);
	Sleep(500);
	ret &= SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_028, true);
	ret &= SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_031, false);

	if (ret == true)
	{
		ret = false;

		for (size_t i = 0; i < 10; i++)
		{
			Sleep(100);

			if (GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_109) == true)
			{
				ret = true;
				break;
			}
		}
	}

#endif // _USE_IOBOARD

	return ret;
}

bool CIOMapControlMgr::SetStagetoPicker()
{
	if (m_bOutOfService == true)
		return true;

	bool ret = true;

#ifdef _USE_IOBOARD

	ret &= SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_028, false);
	ret &= SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_030, true);
	Sleep(500);
	ret &= SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_029, true);
	ret &= SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_030, false);

	if (ret == true)
	{
		ret = false;

		for (size_t i = 0; i < 10; i++)
		{
			Sleep(100);

			if (GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_110) == true)
			{
				ret = true;
				break;
			}
		}
	}
#endif // _USE_IOBOARD
	
	return ret;
}

bool CIOMapControlMgr::SetStageVaccumOn()
{
	if (m_bOutOfService == true)
		return true;

	bool ret = true;

#ifdef _USE_IOBOARD
	ret &= SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_029, false);
	ret &= SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_028, true);

	if (ret == true)
	{
		ret = false;

		for (size_t i = 0; i < 10; i++)
		{
			Sleep(100);

			if (GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_109) == true)
			{
				ret = true;
				break;
			}
		}
	}

#else

	ret = SetMemoryBitToDevice(PLC_BIT_VACCUM_ON01, true);

	if (ret == true)
	{
		ret = false;

		for (size_t i = 0; i < 10; i++)
		{
			Sleep(100);

			if (GetParam_DIOMapMemoryBitImport(PLC_BIT_VACCUM_PLATE01) == true && 
				GetParam_DIOMapMemoryBitImport(PLC_BIT_VACCUM_PLATE02) == true )
			{
				ret = true;
				break;
			}
		}
	}

#endif // _USE_IOBOARD
	

	return ret;
}

bool CIOMapControlMgr::SetStageVaccumOff()
{
	if (m_bOutOfService == true)
		return true;

	bool ret = true;

#ifdef _USE_IOBOARD

	ret &= SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_029, false);
	ret &= SetMemoryBitToDevice(IO_BOARD_READ_OUTPUT_028, false);

	if (ret == true)
	{
		ret = false;

		for (size_t i = 0; i < 10; i++)
		{
			Sleep(100);

			if (GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_109) != true)
			{
				ret = true;
				break;
			}
		}
	}
#else

	ret = SetMemoryBitToDevice(PLC_BIT_VACCUM_ON01, false);

	if (ret == true)
	{
		ret = false;

		for (size_t i = 0; i < 10; i++)
		{
			Sleep(100);

			if (GetParam_DIOMapMemoryBitImport(PLC_BIT_VACCUM_PLATE01) == false &&
				GetParam_DIOMapMemoryBitImport(PLC_BIT_VACCUM_PLATE02) == false)
			{
				ret = true;
				break;
			}
		}
	}

#endif // _USE_IOBOARD

	return ret;
}

int CIOMapControlMgr::GetMoveInterlock()
{
	if (m_bOutOfService == true)
		return VTM_RV_SUCCESSED;

	CString strLog;
	int nRet = VTM_RV_SUCCESSED;

#ifdef _USE_IOBOARD
	if (GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_011) != true)
	{
		CLogWrapper::GetInstance()->KMLOG(WARN, _T("Checking LIGHT CURTAIN DETECTED"));
		nRet = PLC_ARM_SENSOR_DETECT;
	}

	if (GetParam_DIOMapMemoryBitImport(IO_BOARD_READ_INPUT_102) != true)
	{
		CLogWrapper::GetInstance()->KMLOG(WARN, _T("Checking AUTO MODE KEY"));
		nRet = PLC_DOOR_LOCK_TEACHMODE_STATE;
	}
#else

	if (IsOutOfService() != true)
	{
		int nBit = 0;
		nBit = GetParam_DIOMapMemoryBitImport(PLC_LOADER_ARM_SENSOR_01);
		if (nBit != -1)
		{
			if (nBit != 1)
			{
				nRet = PLC_ARM_SENSOR_DETECT;
			}
		}

		nBit = GetParam_DIOMapMemoryBitImport(PLC_BIT_SUTTER_SENSOR_CLOSE);
		if (nBit != -1)
		{
			if (nBit != 1)
			{
				nRet = PLC_LOADER_SHUTTER_UP_ERROR;
			}
		}
	}

#endif // _USE_IOBOARD

	return nRet;
}



CString	CIOMapControlMgr::GetDeviceName()
{
	return m_pFunction->GetDeviceName();
}

bool CIOMapControlMgr::InitDevice(short index/* = 0*/)
{
	return m_pFunction->InitDevice(index);
}

void CIOMapControlMgr::SetInitialize(bool value)
{
	m_bInitialized = value;
}

bool CIOMapControlMgr::IsInitialized() const
{
	return m_bInitialized;
}

bool CIOMapControlMgr::IsConnected() const
{
	return m_pFunction->IsConnected();
}

bool CIOMapControlMgr::IsUseDevice() const
{
	return m_bUse;
}

bool CIOMapControlMgr::IsOutOfService() const
{
	return m_bOutOfService;
}



// address data value 관련 - teach, 알람에서 사용한다.
int CIOMapControlMgr::GetParam_DIOMapAddressTotalCount()
{
	return m_pFunction->GetParam_DIOMapAddressTotalCount();
}

int CIOMapControlMgr::GetParam_DIOMapAddressBitCount(int nType)
{
	return m_pFunction->GetParam_DIOMapAddressBitCount(nType);
}

CString CIOMapControlMgr::GetParam_DIOMapAddressBitImport(int index)
{
	return m_pFunction->GetParam_DIOMapAddressBitImport(index);
}

CString CIOMapControlMgr::GetParam_DIOMapAddressBitImportName(int index)
{
	return m_pFunction->GetParam_DIOMapAddressBitImportName(index);
}

int CIOMapControlMgr::GetParam_DIOMapMemoryBitImport(int index)
{
	int nBit = 0;
	if (index >= 4000)
	{
		nBit = m_piPLCMAdr[index - 4000];
		if (nBit < 3000)
		{
			return -1;
		}
	}
	else
	{
		nBit = index;
	}

	if (nBit > 5000)
	{
		return -1;
	}

	return m_pFunction->GetParam_DIOMapMemoryBitImport(nBit);
}

bool CIOMapControlMgr::GetParam_DIOMapMemoryBitImport2(int index)
{
	return m_pFunction->GetParam_DIOMapMemoryBitImport(index);
}

CString CIOMapControlMgr::GetParam_DIOMapAddressBitExport(int index)
{
	return m_pFunction->GetParam_DIOMapAddressBitExport(index);
}

CString CIOMapControlMgr::GetParam_DIOMapAddressBitExportName(int index)
{
	return m_pFunction->GetParam_DIOMapAddressBitExportName(index);
}

int CIOMapControlMgr::GetParam_DIOMapMemoryBitExport(int index)
{
	int nBit = 0;
	if (index >= 4000)
	{
		nBit = m_piPLCMAdr[index - 4000];
		if (nBit < 3000)
		{
			return -1;
		}
	}
	else
	{
		nBit = index;
	}

	if (nBit > 5000)
	{
		return -1;
	}

	return m_pFunction->GetParam_DIOMapMemoryBitExport(nBit);

//	return m_pFunction->GetParam_DIOMapMemoryBitExport(index);
}

bool CIOMapControlMgr::GetParam_DIOMapMemoryBitExport2(int index)
{
	return m_pFunction->GetParam_DIOMapMemoryBitExport(index);
}

int	CIOMapControlMgr::GetParam_DIOMapAddressWordCount(int nType)
{
	return m_pFunction->GetParam_DIOMapAddressWordCount(nType);
}

CString CIOMapControlMgr::GetParam_DIOMapAddressWordImport(int index)
{
	return m_pFunction->GetParam_DIOMapAddressWordImport(index);
}

CString CIOMapControlMgr::GetParam_DIOMapAddressWordImportName(int index)
{
	return m_pFunction->GetParam_DIOMapAddressWordImportName(index);
}

long CIOMapControlMgr::GetParam_DIOMapMemoryWordImport(int index)
{
	return m_pFunction->GetParam_DIOMapMemoryWordImport(index);
}

CString CIOMapControlMgr::GetParam_DIOMapMemoryWordImportString(int index)
{
	return m_pFunction->GetParam_DIOMapMemoryWordImportString(index);
}

CString CIOMapControlMgr::GetParam_DIOMapAddressWordExport(int index)
{
	return m_pFunction->GetParam_DIOMapAddressWordExport(index);
}

CString CIOMapControlMgr::GetParam_DIOMapAddressWordExportName(int index)
{
	return m_pFunction->GetParam_DIOMapAddressWordExportName(index);
}

long CIOMapControlMgr::GetParam_DIOMapMemoryWordExport(int index)
{
	return m_pFunction->GetParam_DIOMapMemoryWordExport(index);
}

CString CIOMapControlMgr::GetParam_DIOMapMemoryWordExportString(int index)
{
	return m_pFunction->GetParam_DIOMapMemoryWordExportString(index);
}




// address data value 관련 - inline
bool CIOMapControlMgr::SetMemoryBitToDevice_Sub(int index, bool value)
{
	CCriticalSectionSync single(&m_cs_set[0], TRUE);

	return m_pFunction->SetMemoryBitToDevice(index, value);
}

bool CIOMapControlMgr::SetMemoryBitToDevice(int index, bool value)
{
	bool bRet = false;

	bRet = SetMemoryBitToDevice_Sub(index, value);

	CString strLog;
	strLog.Format(_T("SetMemoryBitToDevice(Addr:%d, Val:%d, Ret:%d)"), index, value, bRet);
	m_pLogFileAccessor->KMLOG(SYST, strLog, true);

	return bRet; 
}

bool CIOMapControlMgr::SetMemoryBitToDevice(CString address, bool value)
{
    CCriticalSectionSync single(&m_cs_set[0], TRUE);

	return m_pFunction->SetMemoryBitToDevice(address, value);
}

bool CIOMapControlMgr::SetMemoryNBitToDevice(int index, long value, long lSize)
{
	CCriticalSectionSync single(&m_cs_set[1], TRUE);

	return m_pFunction->SetMemoryNBitToDevice(index, value, lSize);
}

bool CIOMapControlMgr::SetMemoryNBitToDevice(CString address, long value, long lSize)
{
	CCriticalSectionSync single(&m_cs_set[1], TRUE);

	return m_pFunction->SetMemoryNBitToDevice(address, value, lSize);
}

bool CIOMapControlMgr::SetMemoryWordToDevice(int index, float value)
{
    CCriticalSectionSync single(&m_cs_set[2], TRUE);

	return m_pFunction->SetMemoryWordToDevice(index, value);
}

bool CIOMapControlMgr::SetMemoryWordToDevice(CString address, float value)
{
    CCriticalSectionSync single(&m_cs_set[2], TRUE);

    return m_pFunction->SetMemoryWordToDevice(address, value);
}

bool CIOMapControlMgr::SetMemoryNWordToDevice(int index, float* value, long lSize)
{
	CCriticalSectionSync single(&m_cs_set[2], TRUE);

	return m_pFunction->SetMemoryNWordToDevice(index, value, lSize);
}

bool CIOMapControlMgr::SetMemoryNWordToDevice(CString address, float* value, long lSize)
{
	CCriticalSectionSync single(&m_cs_set[2], TRUE);

	return m_pFunction->SetMemoryNWordToDevice(address, value, lSize);
}

bool CIOMapControlMgr::SetMemoryNStrToDevice(int index, TCHAR* value, long lSize)
{
	CCriticalSectionSync single(&m_cs_set[2], TRUE);

	return m_pFunction->SetMemoryNStrToDevice(index, value, lSize);
}

bool CIOMapControlMgr::SetMemoryNStrToDevice(CString address, TCHAR* value, long lSize)
{
	CCriticalSectionSync single(&m_cs_set[2], TRUE);

	return m_pFunction->SetMemoryNStrToDevice(address, value, lSize);
}

bool CIOMapControlMgr::CheckActionDone(int index, DWORD dwTimeout, bool value/* = true*/)
{
	CCriticalSectionSync single(&m_cs_set[3], TRUE);

	return m_pFunction->CheckActionDone(index, dwTimeout, value);
}

bool CIOMapControlMgr::ClearLinkSignalBit()
{
    CCriticalSectionSync single(&m_cs, TRUE);

    return m_pFunction->ClearLinkSignalBit();
}

bool CIOMapControlMgr::ClearLinkSignalWord()
{
    CCriticalSectionSync single(&m_cs, TRUE);

	return m_pFunction->ClearLinkSignalWord();
}

bool CIOMapControlMgr::ClearLinkSignalAll()
{
    bool ret = true;
    ret &= ClearLinkSignalBit();
    ret &= ClearLinkSignalWord();
    return ret;
}

CLogFileAccessor* CIOMapControlMgr::GetLogFileAccessorPtr()
{
	return m_pLogFileAccessor;
}


bool CIOMapControlMgr::ReadAdressFileCSV(CString strFilename)
{
	CFileFind finder;
	if (finder.FindFile(strFilename) == FALSE)
	{
		CString str;	// = "Can not Find 'PLCMAdress.csv' file : ";
		str.Format(_T("Can not Find %s"), strFilename);
		AfxMessageBox(str);
		return FALSE;
	}

	CStdioFile fileConfig(strFilename, CStdioFile::shareDenyNone);

	CString strLine, strText, strAdress;
	int nAdrss = 0, nNext = 0;
	int nDataLine = 0, nPrevNext = 0;	// LYS 2026-07-28 startup verification counters
	CString strBadRows;

	fileConfig.ReadString(strLine);			// HEADER

	while (TRUE)
	{
		if (fileConfig.ReadString(strLine))
		{
			nDataLine++;				// LYS 2026-07-28 data line counter (header excluded)
			nPrevNext = nNext;
			bool bLineHasText = (strLine.GetLength() > 0);
			int iEnd = 0;
			iEnd = strLine.Find(_T(","));
			strLine.Delete(0, iEnd + 1);


			// ADRESS
			iEnd = strLine.Find(_T(","));
			if (iEnd < 0)
				iEnd = strLine.GetLength();

			if (iEnd > 0)
			{
				strAdress = strLine.Mid(0, iEnd);
				strLine.TrimRight(_T(","));

				if (strAdress.IsEmpty() != TRUE)
				{
					nAdrss = _tstoi(strAdress);

					if (nNext < 2000)
					{
						if (nAdrss == 9999)
						{
							m_piPLCMAdr[nNext] = 0;
							nNext++;
							// LYS 2026-07-28 detect a row that consumed no slot (it would SHIFT all following signals)
			if (bLineHasText && nNext == nPrevNext)
			{
				CString strTmp;
				strTmp.Format(_T("%d "), nDataLine + 1);	// +1 = header line
				strBadRows += strTmp;
			}
		}
						else if (nAdrss >= 3000)	// jsyoon 20191010 change
						{
							m_piPLCMAdr[nNext] = nAdrss;
							nNext++;
						}
					}
				}
			}
		}
		else				// End file
		{
			break;
		}
	}

	fileConfig.Close();

	// LYS 2026-07-28 startup verification: valid rows must equal enum count (index binding).
	// A mismatch means PLC signal mapping is (or will be) SHIFTED - warn immediately at startup.
	if (nNext + 4000 != (int)PLC_MBIT_COUNT || strBadRows.IsEmpty() == FALSE)
	{
		CString strMsg;
		strMsg.Format(_T("PLCMAdress.csv verification FAILED!\n\nValid rows = %d / enum count = %d\nSlot-skipped CSV line No. = %s\n\nPLC signal mapping may be SHIFTED.\nFile: %s"),
			nNext, (int)PLC_MBIT_COUNT,
			strBadRows.IsEmpty() ? _T("-") : (LPCTSTR)strBadRows, (LPCTSTR)strFilename);
		AfxMessageBox(strMsg, MB_ICONERROR);
	}

	return true;
}

void CIOMapControlMgr::GetParam_DIOMapMemoryWordImportArray(int index, TCHAR** pValue, long* plSize)
{
	m_pFunction->GetParam_DIOMapMemoryWordExportArray(index, pValue, plSize);
}