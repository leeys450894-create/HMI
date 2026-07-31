#pragma once

#include "..\IOMapImpl.h"

struct SMcNetInfo
{
	bool bUse;
	bool bOutOfService;

	CString	strIPNumber;		// default : 192.168.0.100
	CString	strPortNumber;		// default : 6000
	bool	bConnetMode;		// default : false [ Server : 1, Client : 0 ]
	int		iDeviceIndex;		// default : -1
	int	iNumber;

	SMcNetInfo()
	{
		bUse = false;
		bOutOfService = false;

		strIPNumber = _T("");
		strPortNumber = _T("");
		bConnetMode = false;
		iNumber = -1;
	}
};

#define PLC_MBIT_MAX 2000

class CIniHelper;
class CLogFileAccessor;
class CMcAddressScannerImpl;
class CMcProtocolManager : public CIOMapImpl
{
	CIniHelper	*m_pIniHelper;
	CString		m_strCategory;
	std::vector<SMcNetInfo>				m_vecnMcnetInfo;
	std::vector<CString>				m_vecstrDataFilePath;
	std::vector<CMcAddressScannerImpl*>	m_vecpAddressScanner;
	int*		m_piPLCMAdr;
	bool		m_bVirtualBit[PLC_MBIT_MAX];

public:
	bool ReadAdressFileCSV(CString strFilename);
	bool ReadFileCSV(enumIO_DEVICE dev_index, CString strFileName);
	bool ReadFileXLS(enumIO_DEVICE dev_index, CString strFileName);
	bool ReadFileVerify(enumIO_DEVICE dev_index, CString strFileName);
	bool SetDefaultFile(enumIO_DEVICE dev_index, CString strFileName);
	bool SetMemoryBit(enumIO_DEVICE dev_index, short addr, long val, long lBit = 1);
	bool SetMemoryBit(enumIO_DEVICE dev_index, CString strAddr, long val, long lBit = 1);
	long GetAddressImportCount(enumIO_DEVICE dev_index) const;
	long GetAddressExportCount(enumIO_DEVICE dev_index) const;
	bool GetAddressImportDataValue(enumIO_DEVICE dev_index, UINT data_index) const;
	CString GetAddressImportDataValueString(enumIO_DEVICE dev_index, UINT data_index) const;
	bool GetAddressExportDataValue(enumIO_DEVICE dev_index, UINT data_index) const;
	CString GetAddressExportDataValueString(enumIO_DEVICE dev_index, UINT data_index) const;
	void GetAddressExportDataValueArray(enumIO_DEVICE dev_index, TCHAR** pValue, long* plSize) const;
	SADDRESSMAPDATA *GetAddressImportDataPtr(enumIO_DEVICE dev_index);
	SADDRESSMAPDATA *GetAddressExportDataPtr(enumIO_DEVICE dev_index);
	long GetFAlarmCount() const;
	bool IsOccur_FAlarm() const;
	bool IsConnectPLC(UINT Index) const;
	long GetAddressBitType(UINT index) const;
	long GetAddressWordType(UINT index) const;
	long GetAddressBitTypeToIndex(enumIO_DEVICE dev_index) const;
	long GetAddressWordTypeToIndex(enumIO_DEVICE dev_index) const;
	long GetAddressToString(CString address) const;





	//------------------------------------------------------------------
	// 공통 인터페이스 함수
	//------------------------------------------------------------------
public:
	virtual BOOL		LinkIniFile(bool bSave) override;
	virtual BOOL		LinkDatFile(bool bSave) override;
    virtual bool		InitDevice(int index) override;	
    virtual CString		GetDeviceName() override;		
    virtual bool		IsConnected() override;		
	virtual void		DisConnect() override;
	
	// address data value 관련 - teach
	virtual long		GetParam_DIOMapAddressTotalCount() override;
	virtual long		GetParam_DIOMapAddressBitCount(int nType = 0) override;
	virtual CString		GetParam_DIOMapAddressBitImport(int index) override;
	virtual CString		GetParam_DIOMapAddressBitImportName(int index) override;
	virtual bool		GetParam_DIOMapMemoryBitImport(int index) override;
	virtual CString		GetParam_DIOMapAddressBitExport(int index) override;
	virtual CString		GetParam_DIOMapAddressBitExportName(int index) override;
	virtual bool		GetParam_DIOMapMemoryBitExport(int index) override;
	virtual long		GetParam_DIOMapAddressWordCount(int nType = 0) override;
	virtual CString		GetParam_DIOMapAddressWordImport(int index) override;
	virtual CString		GetParam_DIOMapAddressWordImportName(int index) override;
	virtual long		GetParam_DIOMapMemoryWordImport(int index) override;
	virtual CString		GetParam_DIOMapMemoryWordImportString(int index) override;
	virtual CString		GetParam_DIOMapAddressWordExport(int index) override;
	virtual CString		GetParam_DIOMapAddressWordExportName(int index) override;
	virtual long		GetParam_DIOMapMemoryWordExport(int index) override;
	virtual CString		GetParam_DIOMapMemoryWordExportString(int index) override;
	virtual void		GetParam_DIOMapMemoryWordExportArray(int index, TCHAR** pValue, long* plSize) override;

	// address data value 관련 - inline
	virtual bool		SetMemoryBitToDevice(int index, bool value) override;
	virtual bool		SetMemoryBitToDevice(CString address, bool value) override;
	virtual bool		SetMemoryNBitToDevice(int index, long value, long lSize) override;
	virtual bool		SetMemoryNBitToDevice(CString address, long value, long lSize) override;
	virtual bool		SetMemoryWordToDevice(int index, float value) override;
	virtual bool		SetMemoryWordToDevice(CString address, float value) override;
	virtual bool		SetMemoryNWordToDevice(int index, float* value, long lSize)override;
	virtual bool		SetMemoryNWordToDevice(CString address, float* value, long lSize) override;
	virtual bool		SetMemoryNStrToDevice(int index, TCHAR* value, long lSize)override;
	virtual bool		SetMemoryNStrToDevice(CString address, TCHAR* value, long lSize) override;
	virtual bool		CheckActionDone(int index, DWORD dwTimeout, bool value) override;
	//virtual bool		ClearLinkSignalBit() override;
	//virtual bool		ClearLinkSignalWord() override;

public:
	CMcProtocolManager(CLogFileAccessor* pLogFile, CString strConfigPath, CString strFilePath, CString strCategory);
	virtual ~CMcProtocolManager();
};
