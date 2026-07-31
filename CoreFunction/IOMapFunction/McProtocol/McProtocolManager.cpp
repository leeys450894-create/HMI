#include "pch.h"
#include "McProtocolManager.h"

#include "McMaddressScanner.h"
#include "McFaddressScanner.h"
#include "McDaddressScanner.h"
#include "McGaddressScanner.h"
#include "McPaddressScanner.h"
#include "McCaddressScanner.h"

#include "..\..\common\IniHelper.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CMcProtocolManager::CMcProtocolManager(CLogFileAccessor* pLogFile, CString strConfigPath, CString strFilePath, CString strCategory)
	: m_pIniHelper(new CIniHelper)
	, m_strCategory(strCategory)
	, m_piPLCMAdr(nullptr)
{
	// system config 를 load 한다.
    m_pIniHelper->SetFilePath(strFilePath);

    LinkIniFile(false);

	// default file 정보를 load 한다.
	m_vecstrDataFilePath.clear();	
	
	m_vecstrDataFilePath.push_back(strConfigPath + _T("\\CoreFunction\\IOMapConfig\\M.csv"));			// M-	// M4000~
	m_vecstrDataFilePath.push_back(strConfigPath + _T("\\CoreFunction\\IOMapConfig\\M.csv"));			// M-	// Write Only
	m_vecstrDataFilePath.push_back(strConfigPath + _T("\\CoreFunction\\IOMapConfig\\F.csv"));			// F-   // F0~

	m_vecstrDataFilePath.push_back(strConfigPath + _T("\\CoreFunction\\IOMapConfig\\ProbeCard.csv"));	// probe card  // M3900~
	m_vecstrDataFilePath.push_back(strConfigPath + _T("\\CoreFunction\\IOMapConfig\\SACC.csv"));		// pacc
	
//	m_vecstrDataFilePath.push_back(strConfigPath + _T("\\CoreFunction\\IOMapConfig\\D.csv"));			// D-  // FDC
//	m_vecstrDataFilePath.push_back(strConfigPath + _T("\\CoreFunction\\IOMapConfig\\GlassSensor.csv"));	// glass sensor


	CString strLogFilePath;
	strLogFilePath.Format(_T("%s"), pLogFile->GetFilePath());

	m_piPLCMAdr = new int[PLC_MBIT_MAX];
	for (int i = 0; i < PLC_MBIT_MAX; i++)
	{
		m_piPLCMAdr[i] = 0;

		m_bVirtualBit[i] = false;
	}

	ReadAdressFileCSV(strConfigPath + _T("\\CoreFunction\\IOMapConfig\\PLCMAdress.csv"));

	//IOControl.log
	strLogFilePath.Replace(_T("\\IOControl.log"), _T("\\"));

	m_vecpAddressScanner.clear();
	
	// M번지
	m_vecpAddressScanner.push_back(dynamic_cast<CMcAddressScannerImpl*>(new CMcMaddressScanner(strLogFilePath, m_vecstrDataFilePath[enumIO_DEVICE_READ_M], _T("PLC_READ"))));
	// M번지
	m_vecpAddressScanner.push_back(dynamic_cast<CMcAddressScannerImpl*>(new CMcMaddressScanner(strLogFilePath, m_vecstrDataFilePath[enumIO_DEVICE_WRITE_M], _T("PLC_WRITE"), nullptr, true)));
//	m_vecpAddressScanner.push_back(dynamic_cast<CMcAddressScannerImpl*>(new CMcMaddressScanner(strLogFilePath, m_vecstrDataFilePath[enumIO_DEVICE_WRITE_M], _T("PLC_WRITE"), pLogFile, true)));

	// F번지
	m_vecpAddressScanner.push_back(dynamic_cast<CMcAddressScannerImpl*>(new CMcFaddressScanner(strLogFilePath, m_vecstrDataFilePath[enumIO_DEVICE_F])));

	// 점등기 장비만 사용
	if (m_vecnMcnetInfo[enumIO_DEVICE_PROBE_CARD].bUse) m_vecpAddressScanner.push_back(dynamic_cast<CMcAddressScannerImpl*>(new CMcCaddressScanner(strLogFilePath, m_vecstrDataFilePath[enumIO_DEVICE_PROBE_CARD])));		// PROBE CARD NAME
	if (m_vecnMcnetInfo[enumIO_DEVICE_PACC].bUse)	    m_vecpAddressScanner.push_back(dynamic_cast<CMcAddressScannerImpl*>(new CMcPaddressScanner(strLogFilePath, m_vecstrDataFilePath[enumIO_DEVICE_PACC])));

	// FDC 
	//m_vecpAddressScanner.push_back(dynamic_cast<CMcAddressScannerImpl*>(new CMcDaddressScanner(strLogFilePath, m_vecstrDataFilePath[enumIO_DEVICE_D])));

	// 어디에 사용하는지 모르겠음
	//if( m_vecnMcnetInfo[enumIO_DEVICE_GLASS_SENSOR].bUse) m_vecpAddressScanner.push_back(dynamic_cast<CMcAddressScannerImpl*>(new CMcGaddressScanner(strLogFilePath, m_vecstrDataFilePath[enumIO_DEVICE_GLASS_SENSOR])));		// ??
	

	//--------------------------------------------------------------
	// 통신 module 에 접속한다.
	//--------------------------------------------------------------
	// 프로그램이 켜지면 연결 시도
}

CMcProtocolManager::~CMcProtocolManager()
{
	for(size_t i = 0; i < m_vecpAddressScanner.size(); ++i)
	{
		if(m_vecpAddressScanner[i])
		{
			delete m_vecpAddressScanner[i];
			m_vecpAddressScanner[i] = nullptr;
		}
	}

	if(m_pIniHelper)
	{
		delete m_pIniHelper;
		m_pIniHelper = nullptr;
	}

	if (m_piPLCMAdr != NULL)
	{
		delete[] m_piPLCMAdr;
	}
}



bool CMcProtocolManager::ReadFileCSV(enumIO_DEVICE dev_index, CString strFileName)
{
	if (dev_index != enumIO_DEVICE_WRITE_M)
	{
		if ((0 <= dev_index) && (dev_index < (int)m_vecpAddressScanner.size()))
		{
			return m_vecpAddressScanner[dev_index]->ReadFileCSV(strFileName);
		}
	}

	return false;
}

bool CMcProtocolManager::ReadFileXLS(enumIO_DEVICE dev_index, CString strFileName)	
{
	if (dev_index != enumIO_DEVICE_WRITE_M)
	{
		if ((0 <= dev_index) && (dev_index < (int)m_vecpAddressScanner.size()))
		{
			return m_vecpAddressScanner[dev_index]->ReadFileXLS(strFileName);
		}
	}

	return false;
}

bool CMcProtocolManager::ReadFileVerify(enumIO_DEVICE dev_index, CString strFileName)
{
	if (dev_index != enumIO_DEVICE_WRITE_M)
	{
		if ((0 <= dev_index) && (dev_index < (int)m_vecpAddressScanner.size()))
		{
			return m_vecpAddressScanner[dev_index]->ReadFileVerify(strFileName);
		}
	}

	return false;
}

bool CMcProtocolManager::SetDefaultFile(enumIO_DEVICE dev_index, CString strFileName)
{
	if (dev_index != enumIO_DEVICE_WRITE_M)
	{
		if ((0 <= dev_index) && (dev_index < (int)m_vecpAddressScanner.size()))
		{
			return m_vecpAddressScanner[dev_index]->SetDefaultFile(strFileName);
		}
	}

	return false;
}

bool CMcProtocolManager::SetMemoryBit(enumIO_DEVICE dev_index, short addr, long val, long lBit/* = 1*/)
{
	// M, D쓰기 가능
	if ((dev_index == enumIO_DEVICE_WRITE_M) || (dev_index == enumIO_DEVICE_D))
	{
		if (m_vecpAddressScanner.empty() == false)
		{
			return m_vecpAddressScanner[dev_index]->SetMemoryBit(addr, val, lBit);
		}
	}

	return false;
}

bool CMcProtocolManager::SetMemoryBit(enumIO_DEVICE dev_index, CString strAddr, long val, long lBit/* = 1*/)
{
	// M, D 쓰기 가능
	if ((dev_index == enumIO_DEVICE_WRITE_M) || (dev_index == enumIO_DEVICE_D))
	{
		if (m_vecpAddressScanner.empty() == false)
		{
			return m_vecpAddressScanner[dev_index]->SetMemoryBit(strAddr, val, lBit);
		}
	}

	return false;
}

long CMcProtocolManager::GetAddressImportCount(enumIO_DEVICE dev_index) const
{
	// Write Only
	if (dev_index != enumIO_DEVICE_WRITE_M)
	{
		if ((0 <= dev_index) && (dev_index < (int)m_vecpAddressScanner.size()))
		{
			return m_vecpAddressScanner[dev_index]->GetAddressImportCount();
		}
	}
	
	return false;
}

long CMcProtocolManager::GetAddressExportCount(enumIO_DEVICE dev_index) const
{
	// Write Only
	if (dev_index != enumIO_DEVICE_WRITE_M)
	{
		if ((0 <= dev_index) && (dev_index < (int)m_vecpAddressScanner.size()))
		{
			return m_vecpAddressScanner[dev_index]->GetAddressExportCount();
		}
	}

	return false;
}

bool CMcProtocolManager::GetAddressImportDataValue(enumIO_DEVICE dev_index, UINT data_index) const
{
	// Write Only
	if (dev_index != enumIO_DEVICE_WRITE_M)
	{
		if ((0 <= dev_index) && (dev_index < (int)m_vecpAddressScanner.size()))
		{
			return m_vecpAddressScanner[dev_index]->GetAddressImportDataValue(data_index);
		}
	}

	return false;
}

CString CMcProtocolManager::GetAddressImportDataValueString(enumIO_DEVICE dev_index, UINT data_index) const
{
	// Write Only
	if (dev_index != enumIO_DEVICE_WRITE_M)
	{
		if ((0 <= dev_index) && (dev_index < (int)m_vecpAddressScanner.size()))
		{
			return m_vecpAddressScanner[dev_index]->GetAddressImportDataValueString(data_index);
		}
	}

	return _T("");
}

bool CMcProtocolManager::GetAddressExportDataValue(enumIO_DEVICE dev_index, UINT data_index) const
{
	// Write Only
	if (dev_index != enumIO_DEVICE_WRITE_M)
	{
		if ((0 <= dev_index) && (dev_index < (int)m_vecpAddressScanner.size()))
		{
			return m_vecpAddressScanner[dev_index]->GetAddressExportDataValue(data_index);
		}
	}

	return false;
}

CString CMcProtocolManager::GetAddressExportDataValueString(enumIO_DEVICE dev_index, UINT data_index) const
{
	// Write Only
	if (dev_index != enumIO_DEVICE_WRITE_M)
	{
		if ((0 <= dev_index) && (dev_index < (int)m_vecpAddressScanner.size()))
		{
			return m_vecpAddressScanner[dev_index]->GetAddressExportDataValueString(data_index);
		}
	}

	return _T("");
}

void CMcProtocolManager::GetAddressExportDataValueArray(enumIO_DEVICE dev_index, TCHAR** pValue, long* plSize) const
{
	// Write Only
	if (dev_index != enumIO_DEVICE_WRITE_M)
	{
		if ((0 <= dev_index) && (dev_index < (int)m_vecpAddressScanner.size()))
		{
			m_vecpAddressScanner[dev_index]->GetAddressExportDataValueArray(pValue, plSize);
		}
	}
}

SADDRESSMAPDATA *CMcProtocolManager::GetAddressImportDataPtr(enumIO_DEVICE dev_index)
{
	if ((0 <= dev_index) && (dev_index < (int)m_vecpAddressScanner.size()))
	{
		return m_vecpAddressScanner[dev_index]->GetAddressImportDataPtr();
	}
	
	return nullptr;
}

SADDRESSMAPDATA *CMcProtocolManager::GetAddressExportDataPtr(enumIO_DEVICE dev_index)
{
	if( (0 <= dev_index) && (dev_index < (int)m_vecpAddressScanner.size()) )
	{
		return m_vecpAddressScanner[dev_index]->GetAddressExportDataPtr();				
	}

	return nullptr;
}



long CMcProtocolManager::GetFAlarmCount() const						
{ 
	// F- address 에만 데이터를 쓸수 있다.
	if(enumIO_DEVICE_F < (int)m_vecpAddressScanner.size())
	{
		return m_vecpAddressScanner[enumIO_DEVICE_F]->GetAddressAlarmCount();			
	}				
	
	return 0;
}

bool CMcProtocolManager::IsOccur_FAlarm() const						
{ 
	// F- address 에만 데이터를 쓸수 있다.
	if(enumIO_DEVICE_F < (int)m_vecpAddressScanner.size())
	{
		return m_vecpAddressScanner[enumIO_DEVICE_F]->IsOccur_addressAlarm();
	}				
	
	return false;
}

bool CMcProtocolManager::IsConnectPLC(UINT Index) const				
{ 
	return (m_vecpAddressScanner[Index]->IsConnected() == TRUE) ? true : false;
}

long CMcProtocolManager::GetAddressBitType(UINT Index) const
{
	long nType = enumIO_DEVICE_READ_M;

	if (Index >= dfFADDRESS_RD_START && Index < dfFADDRESS_LIMIT_COUNT)
	{
		nType = enumIO_DEVICE_F;
	}
	else if (Index >= dfPACCADDRESS_RD_START && Index < dfPACCADDRESS_LIMIT_COUNT)
	{
		nType = enumIO_DEVICE_PACC;
	}
	else if (Index >= dfMADDRESS_RD_START && Index < dfMADDRESS_LIMIT_COUNT)
	{
		nType = enumIO_DEVICE_READ_M;
	}
	else
	{
		nType = enumIO_DEVICE_READ_M;
	}

	return nType;
}

long CMcProtocolManager::GetAddressWordType(UINT Index) const
{
	long nType = enumIO_DEVICE_D;

	if (Index >= dfGSADDRESS_WT_START && Index < dfGSADDRESS_LIMIT_COUNT)
	{
		nType = enumIO_DEVICE_GLASS_SENSOR;
	}
	else if (Index >= dfPROBEADDRESS_RD_START && Index < dfPROBEADDRESS_LIMIT_COUNT)
	{
		nType = enumIO_DEVICE_PROBE_CARD;
	}
	else if (Index >= dfDADDRESS_WT_START && Index < dfDADDRESS_LIMIT_COUNT)
	{
		nType = enumIO_DEVICE_D;
	}
	else
	{
		nType = enumIO_DEVICE_D;
	}

	return nType;
}

long CMcProtocolManager::GetAddressBitTypeToIndex(enumIO_DEVICE dev_index) const
{
	long nIndex = dfMADDRESS_RD_START;

	if (dev_index == enumIO_DEVICE_F)
	{
		nIndex = dfFADDRESS_RD_START;
	}
	else if (dev_index == enumIO_DEVICE_PACC)
	{
		nIndex = dfPACCADDRESS_RD_START;
	}
	else if (dev_index == enumIO_DEVICE_READ_M)
	{
		nIndex = dfMADDRESS_RD_START;
	}
	else
	{
		nIndex = dfMADDRESS_RD_START;
	}

	return nIndex;
}

long CMcProtocolManager::GetAddressWordTypeToIndex(enumIO_DEVICE dev_index) const
{
	long nIndex = enumIO_DEVICE_D;

	if (dev_index == enumIO_DEVICE_D)
	{
		nIndex = dfDADDRESS_WT_START;
	}
	else if (dev_index == enumIO_DEVICE_GLASS_SENSOR)
	{
		nIndex = dfGSADDRESS_WT_START;
	}
	else if (dev_index == enumIO_DEVICE_PROBE_CARD)
	{
		nIndex = dfPROBEADDRESS_RD_START;
	}
	else
	{
		nIndex = dfDADDRESS_WT_START;
	}

	return nIndex;
}

long CMcProtocolManager::GetAddressToString(CString address) const
{
	//M4000 -> 4000

	int nLength = address.GetLength();

	CString strAddress = address.Mid(1, nLength -1);

	return _wtol(strAddress);
}







//------------------------------------------------------------------
// 공통 인터페이스 함수
//------------------------------------------------------------------
BOOL CMcProtocolManager::LinkIniFile(bool bSave)
{
	m_vecnMcnetInfo.clear();
	m_vecnMcnetInfo.resize(enumADDRESS_DEVICE_Count);

	// address M
	m_pIniHelper->Link(bSave, m_strCategory, _T("M_ADDRESS_USE"), &m_vecnMcnetInfo[enumIO_DEVICE_READ_M].bUse, true);
	m_pIniHelper->Link(bSave, m_strCategory, _T("M_ADDRESS_OUT_OF_SERVICE"), &m_vecnMcnetInfo[enumIO_DEVICE_READ_M].bOutOfService, false);
	m_pIniHelper->Link(bSave, m_strCategory, _T("M_ADDRESS_STATION_NUM"), &m_vecnMcnetInfo[enumIO_DEVICE_READ_M].iNumber, 2);
	m_pIniHelper->Link(bSave, m_strCategory, _T("M_TCPIP_IP"), &m_vecnMcnetInfo[enumIO_DEVICE_READ_M].strIPNumber, (CString)_T("127.0.0.1"));
	m_pIniHelper->Link(bSave, m_strCategory, _T("M_TCPIP_PORT"), &m_vecnMcnetInfo[enumIO_DEVICE_READ_M].strPortNumber, (CString)_T("2001"));

	// address M
	m_pIniHelper->Link(bSave, m_strCategory, _T("M_W_ADDRESS_USE"), &m_vecnMcnetInfo[enumIO_DEVICE_WRITE_M].bUse, true);
	m_pIniHelper->Link(bSave, m_strCategory, _T("M_W_ADDRESS_OUT_OF_SERVICE"), &m_vecnMcnetInfo[enumIO_DEVICE_WRITE_M].bOutOfService, false);
	m_pIniHelper->Link(bSave, m_strCategory, _T("M_W_ADDRESS_STATION_NUM"), &m_vecnMcnetInfo[enumIO_DEVICE_WRITE_M].iNumber, 2);
	m_pIniHelper->Link(bSave, m_strCategory, _T("M_W_TCPIP_IP"), &m_vecnMcnetInfo[enumIO_DEVICE_WRITE_M].strIPNumber, (CString)_T("127.0.0.1"));
	m_pIniHelper->Link(bSave, m_strCategory, _T("M_W_TCPIP_PORT"), &m_vecnMcnetInfo[enumIO_DEVICE_WRITE_M].strPortNumber, (CString)_T("2001"));

	// address F
	m_pIniHelper->Link(bSave, m_strCategory, _T("F_ADDRESS_USE"), &m_vecnMcnetInfo[enumIO_DEVICE_F].bUse, true);
	m_pIniHelper->Link(bSave, m_strCategory, _T("F_ADDRESS_OUT_OF_SERVICE"), &m_vecnMcnetInfo[enumIO_DEVICE_F].bOutOfService, false);
	m_pIniHelper->Link(bSave, m_strCategory, _T("F_ADDRESS_STATION_NUM"), &m_vecnMcnetInfo[enumIO_DEVICE_F].iNumber, 3);
	m_pIniHelper->Link(bSave, m_strCategory, _T("F_TCPIP_IP"), &m_vecnMcnetInfo[enumIO_DEVICE_F].strIPNumber, (CString)_T("127.0.0.1"));
	m_pIniHelper->Link(bSave, m_strCategory, _T("F_TCPIP_PORT"), &m_vecnMcnetInfo[enumIO_DEVICE_F].strPortNumber, (CString)_T("2001"));

	// address D
	m_pIniHelper->Link(bSave, m_strCategory, _T("D_ADDRESS_USE"), &m_vecnMcnetInfo[enumIO_DEVICE_D].bUse, true);
	m_pIniHelper->Link(bSave, m_strCategory, _T("D_ADDRESS_OUT_OF_SERVICE"), &m_vecnMcnetInfo[enumIO_DEVICE_D].bOutOfService, false);
	m_pIniHelper->Link(bSave, m_strCategory, _T("D_ADDRESS_STATION_NUM"), &m_vecnMcnetInfo[enumIO_DEVICE_D].iNumber, 6);
	m_pIniHelper->Link(bSave, m_strCategory, _T("D_TCPIP_IP"), &m_vecnMcnetInfo[enumIO_DEVICE_D].strIPNumber, (CString)_T("127.0.0.1"));
	m_pIniHelper->Link(bSave, m_strCategory, _T("D_TCPIP_PORT"), &m_vecnMcnetInfo[enumIO_DEVICE_D].strPortNumber, (CString)_T("2001"));

	// address M
	m_pIniHelper->Link(bSave, m_strCategory, _T("P_ADDRESS_USE"), &m_vecnMcnetInfo[enumIO_DEVICE_PACC].bUse, true);
	m_pIniHelper->Link(bSave, m_strCategory, _T("P_ADDRESS_OUT_OF_SERVICE"), &m_vecnMcnetInfo[enumIO_DEVICE_PACC].bOutOfService, false);
	m_pIniHelper->Link(bSave, m_strCategory, _T("P_ADDRESS_STATION_NUM"), &m_vecnMcnetInfo[enumIO_DEVICE_PACC].iNumber, 1);
	m_pIniHelper->Link(bSave, m_strCategory, _T("P_TCPIP_IP"), &m_vecnMcnetInfo[enumIO_DEVICE_PACC].strIPNumber, (CString)_T("127.0.0.1"));
	m_pIniHelper->Link(bSave, m_strCategory, _T("P_TCPIP_PORT"), &m_vecnMcnetInfo[enumIO_DEVICE_PACC].strPortNumber, (CString)_T("2001"));

    m_pIniHelper->Link(bSave, m_strCategory, _T("C_ADDRESS_USE"),			&m_vecnMcnetInfo[enumIO_DEVICE_PROBE_CARD		].bUse,				true);
    m_pIniHelper->Link(bSave, m_strCategory, _T("C_ADDRESS_OUT_OF_SERVICE"),	&m_vecnMcnetInfo[enumIO_DEVICE_PROBE_CARD		].bOutOfService,	false);
	m_pIniHelper->Link(bSave, m_strCategory, _T("C_ADDRESS_STATION_NUM"),	&m_vecnMcnetInfo[enumIO_DEVICE_PROBE_CARD		].iNumber,			5);
	m_pIniHelper->Link(bSave, m_strCategory, _T("C_TCPIP_IP"),				&m_vecnMcnetInfo[enumIO_DEVICE_PROBE_CARD		].strIPNumber,		(CString)_T("127.0.0.1"));
	m_pIniHelper->Link(bSave, m_strCategory, _T("C_TCPIP_PORT"),			&m_vecnMcnetInfo[enumIO_DEVICE_PROBE_CARD		].strPortNumber,	(CString)_T("2001"));

	// address D
	m_pIniHelper->Link(bSave, m_strCategory, _T("G_ADDRESS_USE"), &m_vecnMcnetInfo[enumIO_DEVICE_GLASS_SENSOR].bUse, true);
	m_pIniHelper->Link(bSave, m_strCategory, _T("G_ADDRESS_OUT_OF_SERVICE"), &m_vecnMcnetInfo[enumIO_DEVICE_GLASS_SENSOR].bOutOfService, false);
	m_pIniHelper->Link(bSave, m_strCategory, _T("G_ADDRESS_STATION_NUM"), &m_vecnMcnetInfo[enumIO_DEVICE_GLASS_SENSOR].iNumber, 4);
	m_pIniHelper->Link(bSave, m_strCategory, _T("G_TCPIP_IP"), &m_vecnMcnetInfo[enumIO_DEVICE_GLASS_SENSOR].strIPNumber, (CString)_T("127.0.0.1"));
	m_pIniHelper->Link(bSave, m_strCategory, _T("G_TCPIP_PORT"), &m_vecnMcnetInfo[enumIO_DEVICE_GLASS_SENSOR].strPortNumber, (CString)_T("2001"));

	return TRUE;
}

BOOL CMcProtocolManager::LinkDatFile(bool bSave)
{
	return TRUE;
}

bool CMcProtocolManager::InitDevice(int index)
{
	for(size_t i = 0; i < m_vecpAddressScanner.size(); ++i)
	{
		if(m_vecpAddressScanner[i])
		{
			if (m_vecnMcnetInfo[i].bUse)
			{
				if (i != enumIO_DEVICE_WRITE_M)
				{
					m_vecpAddressScanner[i]->ReadFileCSV(m_vecstrDataFilePath[i]);
				}

				if (m_vecnMcnetInfo[i].bOutOfService != true)
				{
					if (i != enumIO_DEVICE_WRITE_M)
					{
						m_vecpAddressScanner[i]->Start();
					}

					m_vecpAddressScanner[i]->Initialize(false, m_vecnMcnetInfo[i].strIPNumber, _wtol(m_vecnMcnetInfo[i].strPortNumber));
				}
			}
		}
	}

	return true;
}

CString	CMcProtocolManager::GetDeviceName()
{
	return _T("McProtocol");
}

bool CMcProtocolManager::IsConnected()
{
	bool ret = true;

 	for(size_t i = 0; i < m_vecpAddressScanner.size(); ++i)
	{
		if (m_vecnMcnetInfo[i].bUse)
		{
			ret &= m_vecpAddressScanner[i]->IsConnected();
		}
	}
	
	return ret;
}

void CMcProtocolManager::DisConnect()
{
	for (size_t i = 0; i < m_vecpAddressScanner.size(); ++i)
	{
		if (m_vecnMcnetInfo[i].bUse)
		{
			m_vecpAddressScanner[i]->DisConnect();
		}
	}
}

// address data value 관련 - teach, 알람에서 사용한다.
long CMcProtocolManager::GetParam_DIOMapAddressTotalCount()
{
    return GetAddressImportCount(enumIO_DEVICE_READ_M) + GetAddressImportCount(enumIO_DEVICE_READ_M);
}

long CMcProtocolManager::GetParam_DIOMapAddressBitCount(int nType)
{
    return GetAddressImportCount((enumIO_DEVICE)nType) + GetAddressExportCount((enumIO_DEVICE)nType);
}

CString CMcProtocolManager::GetParam_DIOMapAddressBitImport(int index)
{
	long nType = GetAddressBitType(index);
	int Address = (int)GetAddressBitTypeToIndex((enumIO_DEVICE)nType);
	int nAddress = index - Address;

	// 2023.12.22 change
	if (GetAddressImportDataPtr((enumIO_DEVICE)nType) != nullptr)
		return GetAddressImportDataPtr((enumIO_DEVICE)nType)->vecData[nAddress].strMapNumber;
	else
		return _T("");

//	return GetAddressImportDataPtr((enumIO_DEVICE)nType)->vecData[nAddress].strMapNumber;
}

CString CMcProtocolManager::GetParam_DIOMapAddressBitImportName(int index)
{
	long nType = GetAddressBitType(index);
	int Address = (int)GetAddressBitTypeToIndex((enumIO_DEVICE)nType);
	int nAddress = index - Address;

	// 2023.12.22 change
	if (GetAddressImportDataPtr((enumIO_DEVICE)nType) != nullptr)
		return GetAddressImportDataPtr((enumIO_DEVICE)nType)->vecData[nAddress].strDescription;
	else
		return _T("");

//	return GetAddressImportDataPtr((enumIO_DEVICE)nType)->vecData[nAddress].strDescription;
}

bool CMcProtocolManager::GetParam_DIOMapMemoryBitImport(int index)
{
	long lType = GetAddressBitType(index);
	if (lType == enumIO_DEVICE_READ_M )
	{
		if (m_vecnMcnetInfo[enumIO_DEVICE_READ_M].bOutOfService == true)
		{
			return m_bVirtualBit[index - 4000];
		}
		else
		{
			return GetAddressImportDataValue((enumIO_DEVICE)lType, index);
		}
	}
	else if (lType == enumIO_DEVICE_PACC)
	{
		if (m_vecnMcnetInfo[enumIO_DEVICE_PACC].bOutOfService == true)
		{
			return m_bVirtualBit[index - 2000];
		}
		else
		{
			return GetAddressImportDataValue((enumIO_DEVICE)lType, index);
		}
	}
	else
	{
		
		return GetAddressImportDataValue((enumIO_DEVICE)lType, index);
	}

//	return GetAddressImportDataValue((enumIO_DEVICE)GetAddressBitType(nBit), nBit);
}

CString CMcProtocolManager::GetParam_DIOMapAddressBitExport(int index)
{
	long nType = GetAddressBitType(index);
	int Address = GetAddressBitTypeToIndex((enumIO_DEVICE)nType);

	// 2023.12.22 change
	int nAddress = index - Address;
	if (GetAddressExportDataPtr((enumIO_DEVICE)nType) != nullptr)
		return GetAddressExportDataPtr((enumIO_DEVICE)nType)->vecData[nAddress].strMapNumber;
	else
		return _T("");
//	return GetAddressExportDataPtr((enumIO_DEVICE)nType)->vecData[index - Address].strMapNumber;
}

CString CMcProtocolManager::GetParam_DIOMapAddressBitExportName(int index)
{
	long nType = GetAddressBitType(index);
	int Address = GetAddressBitTypeToIndex((enumIO_DEVICE)nType);

	// 2023.12.22 change
	int nAddress = index - Address;
	if (GetAddressExportDataPtr((enumIO_DEVICE)nType) != nullptr)
		return GetAddressExportDataPtr((enumIO_DEVICE)nType)->vecData[nAddress].strDescription;
	else
		return _T("");
//	return GetAddressExportDataPtr((enumIO_DEVICE)nType)->vecData[index - Address].strDescription;
}

bool CMcProtocolManager::GetParam_DIOMapMemoryBitExport(int index)
{
    return GetAddressExportDataValue((enumIO_DEVICE)GetAddressBitType(index), index);
}

long CMcProtocolManager::GetParam_DIOMapAddressWordCount(int nType)
{
	return GetAddressImportCount((enumIO_DEVICE)nType) + GetAddressExportCount((enumIO_DEVICE)nType);
}

CString CMcProtocolManager::GetParam_DIOMapAddressWordImport(int index)
{
	long nType = GetAddressWordType(index);
	int Address = GetAddressWordTypeToIndex((enumIO_DEVICE)nType);

	// 2023.12.22 change
	int nAddress = index - Address;
	if (GetAddressImportDataPtr((enumIO_DEVICE)nType) != nullptr)
		return GetAddressImportDataPtr((enumIO_DEVICE)nType)->vecData[nAddress].strMapNumber;
	else
		return _T("");

//	return GetAddressImportDataPtr((enumIO_DEVICE)nType)->vecData[index - Address].strMapNumber;
}

CString CMcProtocolManager::GetParam_DIOMapAddressWordImportName(int index)
{
	long nType = GetAddressWordType(index);
	int Address = GetAddressWordTypeToIndex((enumIO_DEVICE)nType);

	// 2023.12.22 change
	int nAddress = index - Address;
	if (GetAddressImportDataPtr((enumIO_DEVICE)nType) != nullptr)
		return GetAddressImportDataPtr((enumIO_DEVICE)nType)->vecData[nAddress].strDescription;
	else
		return _T("");

//	return GetAddressImportDataPtr((enumIO_DEVICE)nType)->vecData[index - Address].strDescription;
}

long CMcProtocolManager::GetParam_DIOMapMemoryWordImport(int index)
{
	return GetAddressImportDataValue((enumIO_DEVICE)GetAddressWordType(index), index);
}

CString CMcProtocolManager::GetParam_DIOMapMemoryWordImportString(int index)
{
	long nType = GetAddressWordType(index);
	int Address = GetAddressWordTypeToIndex((enumIO_DEVICE)nType);

	return GetAddressImportDataValueString((enumIO_DEVICE)GetAddressWordType(index), index);
}

CString CMcProtocolManager::GetParam_DIOMapAddressWordExport(int index)
{
	long nType = GetAddressBitType(index);
	int Address = GetAddressWordTypeToIndex((enumIO_DEVICE)nType);

	// 2023.12.22 change
	int nAddress = index - Address;
	if (GetAddressExportDataPtr((enumIO_DEVICE)nType) != nullptr)
		return GetAddressExportDataPtr((enumIO_DEVICE)nType)->vecData[nAddress].strMapNumber;
	else
		return _T("");

//	return GetAddressExportDataPtr((enumIO_DEVICE)nType)->vecData[index - Address].strMapNumber;
}

CString CMcProtocolManager::GetParam_DIOMapAddressWordExportName(int index)
{
	long nType = GetAddressBitType(index);
	int Address = GetAddressWordTypeToIndex((enumIO_DEVICE)nType);

	// 2023.12.22 change
	int nAddress = index - Address;
	if (GetAddressExportDataPtr((enumIO_DEVICE)nType) != nullptr)
		return GetAddressExportDataPtr((enumIO_DEVICE)nType)->vecData[nAddress].strDescription;
	else
		return _T("");

//	return GetAddressExportDataPtr((enumIO_DEVICE)nType)->vecData[index - Address].strDescription;
}

long CMcProtocolManager::GetParam_DIOMapMemoryWordExport(int index)
{
	return GetAddressExportDataValue((enumIO_DEVICE)GetAddressWordType(index), index);
}

CString CMcProtocolManager::GetParam_DIOMapMemoryWordExportString(int index)
{
	return GetAddressExportDataValueString((enumIO_DEVICE)GetAddressWordType(index), index);
}


void CMcProtocolManager::GetParam_DIOMapMemoryWordExportArray(int index, TCHAR** pValue, long* plSize)
{
	GetAddressExportDataValueArray((enumIO_DEVICE)GetAddressWordType(index), pValue, plSize);
}



// address data value 관련 - inline
bool CMcProtocolManager::SetMemoryBitToDevice(int index, bool value)
{
	int nBit = 0;
	if (index >= 4000)
	{
		nBit = m_piPLCMAdr[index - 4000];
		if (nBit < 3000)
		{
			return false;
		}
	}
	else
	{
		nBit = index;
	}

	if (nBit > 5000)
	{
		return false;
	}

	long lType = GetAddressBitType(nBit);
	if (lType == enumIO_DEVICE_READ_M)
	{
		if (m_vecnMcnetInfo[enumIO_DEVICE_READ_M].bOutOfService == true)
		{
			m_bVirtualBit[nBit - 4000] = value;

			return true;
		}
		else
		{
			const CString strDeviceText = GetParam_DIOMapAddressBitExport(nBit);

			return SetMemoryNBitToDevice(strDeviceText, value, 1);
		}
	}
	else if (lType == enumIO_DEVICE_PACC)
	{
		if (m_vecnMcnetInfo[enumIO_DEVICE_PACC].bOutOfService == true)
		{
			m_bVirtualBit[nBit - 2000] = value;

			return true;
		}
		else
		{
			const CString strDeviceText = GetParam_DIOMapAddressBitExport(nBit);

			return SetMemoryNBitToDevice(strDeviceText, value, 1);
		}
	}
	else
	{
		const CString strDeviceText = GetParam_DIOMapAddressBitExport(nBit);

		return SetMemoryNBitToDevice(strDeviceText, value, 1);
	}



	/*if (index >= 0 && index < PLC_MBIT_MAX)
	{
		
		

		if (m_vecnMcnetInfo[enumIO_DEVICE_READ_M].bOutOfService == true)
		{
			m_bVirtualBit[nBit - 4000] = value;

			return true;
		}
		else
		{
			const CString strDeviceText = GetParam_DIOMapAddressBitExport(nBit);

			return SetMemoryNBitToDevice(strDeviceText, value, 1);
		}
	}
	else
	{
		return false;
	}*/
	

	//const CString strDeviceText = GetParam_DIOMapAddressBitExport(nBit);

	//return SetMemoryNBitToDevice(strDeviceText, value, 1);
}

bool CMcProtocolManager::SetMemoryBitToDevice(CString address, bool value)
{
	const long ltype = GetAddressBitType(GetAddressToString(address));

	return SetMemoryNBitToDevice(address, value, 1);
}

bool CMcProtocolManager::SetMemoryNBitToDevice(int index, long value, long lSize)
{
	int nBit = 0;
	if (index >= 4000)
	{
		nBit = m_piPLCMAdr[index - 4000];
		if (nBit < 3000)
		{
			return false;
		}
	}
	else
	{
		nBit = index;
	}

	if (nBit > 5000)
	{
		return false;
	}

	long lType = GetAddressBitType(nBit);
	if (lType == enumIO_DEVICE_READ_M )
	{
		if (m_vecnMcnetInfo[enumIO_DEVICE_READ_M].bOutOfService == true)
		{
			if (nBit >= 4000)
			{
				BYTE nByte = 0x00;
				for (int i = 0; i < lSize; i++)
				{
					nByte = (value >> i) & 0x01;
					if (nByte == 0)
						m_bVirtualBit[nBit + i - 4000] = false;
					else
						m_bVirtualBit[nBit + i - 4000] = true;
				}
				return true;

			}
			else
			{
				return false;
			}
		}
		else
		{
			/*const CString strDeviceText = GetParam_DIOMapAddressBitExport(nBit);
			const long ltype = GetAddressBitType(nBit);

			return m_vecpAddressScanner[ltype]->SetMemoryBit((short)nBit, value, lSize);*/

			return m_vecpAddressScanner[enumIO_DEVICE_WRITE_M]->SetMemoryBit((short)nBit, value, lSize);
		}
	}
	else if (lType == enumIO_DEVICE_PACC)
	{
		if (m_vecnMcnetInfo[enumIO_DEVICE_PACC].bOutOfService == true)
		{
			if (nBit >= 3000)
			{
				BYTE nByte = 0x00;
				for (int i = 0; i < lSize; i++)
				{
					nByte = (value >> i) & 0x01;
					if (nByte == 0)
						m_bVirtualBit[nBit + i - 2000] = false;
					else
						m_bVirtualBit[nBit + i - 2000] = true;
				}

				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
	//		const CString strDeviceText = GetParam_DIOMapAddressBitExport(nBit);
	//		const long ltype = GetAddressBitType(nBit);

			return m_vecpAddressScanner[enumIO_DEVICE_WRITE_M]->SetMemoryBit((short)nBit, value, lSize);
		}
	}
	else
	{
		const CString strDeviceText = GetParam_DIOMapAddressBitExport(nBit);
		const long ltype = GetAddressBitType(nBit);

		return m_vecpAddressScanner[ltype]->SetMemoryBit((short)nBit, value, lSize);
	}


	//const CString strDeviceText = GetParam_DIOMapAddressBitExport(index);
	//const long ltype = GetAddressBitType(index);

	//return m_vecpAddressScanner[ltype]->SetMemoryBit((short)index, value, lSize);
}

bool CMcProtocolManager::SetMemoryNBitToDevice(CString address, long value, long lSize)
{
//	const long ltype = GetAddressBitType(GetAddressToString(address));

//	return (m_vecpAddressScanner[ltype]->SetMemoryBit(address, value, lSize) == 0) ? true : false;

	// jsyoon 2022.04.04 change
	//return (m_vecpAddressScanner[enumIO_DEVICE_WRITE_M]->SetMemoryBit(address, value, lSize) == 0) ? true : false;
	return m_vecpAddressScanner[enumIO_DEVICE_WRITE_M]->SetMemoryBit(address, value, lSize);	// jsyoon 2022.07.12 change
}

bool CMcProtocolManager::SetMemoryWordToDevice(int index, float value)
{
	const CString strDeviceText = GetParam_DIOMapAddressBitExport(index);
	const long ltype = GetAddressWordType(index);

	return m_vecpAddressScanner[ltype]->SetMemoryWord(index, &value, 1);
}

bool CMcProtocolManager::SetMemoryWordToDevice(CString address, float value)
{
	const long ltype = GetAddressWordType(GetAddressToString(address));

	float fvalue = (float)value;
	int index = _wtoi(address.Mid(1, address.GetLength()));

	return m_vecpAddressScanner[ltype]->SetMemoryWord(index, &value, 1);
}

bool CMcProtocolManager::SetMemoryNWordToDevice(int index, float* value, long lSize)
{
	const CString strDeviceText = GetParam_DIOMapAddressBitExport(index);
	const long ltype = GetAddressWordType(index);

	return m_vecpAddressScanner[ltype]->SetMemoryWord(index, value, lSize);
}

bool CMcProtocolManager::SetMemoryNWordToDevice(CString address, float* value, long lSize)
{
	const long ltype = GetAddressWordType(GetAddressToString(address));
	int index = _wtoi(address.Mid(1, address.GetLength()));

	return m_vecpAddressScanner[ltype]->SetMemoryWord(index, value, lSize);
}

bool CMcProtocolManager::SetMemoryNStrToDevice(int index, TCHAR* value, long lSize)
{
	const CString strDeviceText = GetParam_DIOMapAddressBitExport(index);
	const long ltype = GetAddressWordType(index);

	return m_vecpAddressScanner[ltype]->SetMemoryStr(index, value, lSize);
}

bool CMcProtocolManager::SetMemoryNStrToDevice(CString address, TCHAR* value, long lSize)
{
	/*const long ltype = GetAddressWordType(GetAddressToString(address));
	int index = _wtoi(address.Mid(1, address.GetLength()));

	return m_vecpAddressScanner[ltype]->SetMemoryStr(index, value, lSize);*/

	int index = _wtoi(address.Mid(1, address.GetLength()));
	return m_vecpAddressScanner[enumIO_DEVICE_WRITE_M]->SetMemoryStr(index, value, lSize);
}

bool CMcProtocolManager::CheckActionDone(int index, DWORD dwTimeout, bool value)
{
	int nBit = 0;
	if (index >= 4000)
	{
		nBit = m_piPLCMAdr[index - 4000];
		if (nBit < 3000)
		{
			return false;
		}
	}
	else
	{
		nBit = index;
	}

	if (nBit > 5000)
	{
		return false;
	}

	long lType = GetAddressBitType(nBit);
	if (lType == enumIO_DEVICE_READ_M)
	{
		if (m_vecnMcnetInfo[enumIO_DEVICE_READ_M].bOutOfService == true) 
		{
			if (dwTimeout > 10)   
				Sleep(2000);
			else
				Sleep(500 * dwTimeout);

			return m_bVirtualBit[nBit - 4000];
		}
		else
		{
			return m_vecpAddressScanner[enumIO_DEVICE_READ_M]->CheckActionDone(nBit, dwTimeout, value);
			//		return m_vecpAddressScanner[enumIO_DEVICE_READ_M]->CheckActionDone(index, dwTimeout, value);
		}
	}
	else if (lType == enumIO_DEVICE_PACC)
	{
		if (m_vecnMcnetInfo[enumIO_DEVICE_PACC].bOutOfService == true)
		{

			Sleep(500 * dwTimeout);

			return m_bVirtualBit[nBit - 2000];
		}
		else
		{
			return m_vecpAddressScanner[enumIO_DEVICE_PACC]->CheckActionDone(nBit, dwTimeout, value);
			//		return m_vecpAddressScanner[enumIO_DEVICE_READ_M]->CheckActionDone(index, dwTimeout, value);
		}
	}
	else
	{
		return false;
	}

	//if (index >= 0 && index < PLC_MBIT_MAX)
	//{
	//	int nBit = m_piPLCMAdr[index];
	//	if (nBit < 3000)
	//	{
	//		return false;
	//	}

	//	if (m_vecnMcnetInfo[enumIO_DEVICE_READ_M].bOutOfService == true)
	//	{
	//		return m_bVirtualBit[nBit - 4000]; 
	//	}
	//	else
	//	{
	//		return m_vecpAddressScanner[enumIO_DEVICE_READ_M]->CheckActionDone(nBit, dwTimeout, value);
	////		return m_vecpAddressScanner[enumIO_DEVICE_READ_M]->CheckActionDone(index, dwTimeout, value);
	//	}
	//}
	//else
	//{
	//	return false;
	//}

	
}

//bool CMcProtocolManager::ClearLinkSignalBit()
//{
//	return m_pDlgMXComponent->ClearLinkSignalBit();
//}
//
//bool CMcProtocolManager::ClearLinkSignalWord()
//{
//	return m_pDlgMXComponent->ClearLinkSignalWord();
//}

bool CMcProtocolManager::ReadAdressFileCSV(CString strFilename)
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

	fileConfig.ReadString(strLine);			// HEADER

	while (TRUE)
	{
		if (fileConfig.ReadString(strLine))
		{
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

					if (nNext < PLC_MBIT_MAX)
					{
						if (nAdrss == 9999)
						{
							m_piPLCMAdr[nNext] = 0;
							nNext++;
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

	return true;
}