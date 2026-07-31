#pragma once

class CIOMapImpl;
class CLogFileAccessor;
class CIOFunction
{
	CIOMapImpl *m_pCtrlImpl;
	

	//------------------------------------------------------------------
	// 외부 접근 가능 함수
	//------------------------------------------------------------------
public:
    bool	InitDevice(short index);
    CString	GetDeviceName();
    bool	IsConnected();

	// address data value 관련 - teach, 알람에서 사용한다.
	long	GetParam_DIOMapAddressTotalCount();
	long	GetParam_DIOMapAddressBitCount(int nType = 0);
	CString GetParam_DIOMapAddressBitImport(int index);
	CString GetParam_DIOMapAddressBitImportName(int index);
	bool	GetParam_DIOMapMemoryBitImport(int index);
	CString GetParam_DIOMapAddressBitExport(int index);
	CString GetParam_DIOMapAddressBitExportName(int index);
	bool	GetParam_DIOMapMemoryBitExport(int index);
	long	GetParam_DIOMapAddressWordCount(int nType = 0);
	CString GetParam_DIOMapAddressWordImport(int index);
	CString GetParam_DIOMapAddressWordImportName(int index);
	long	GetParam_DIOMapMemoryWordImport(int index);
	CString	GetParam_DIOMapMemoryWordImportString(int index);
	CString GetParam_DIOMapAddressWordExport(int index);
	CString GetParam_DIOMapAddressWordExportName(int index);
	long	GetParam_DIOMapMemoryWordExport(int index);
	CString	GetParam_DIOMapMemoryWordExportString(int index);
	void    GetParam_DIOMapMemoryWordExportArray(int index, TCHAR** pValue, long* plSize);

	// address data value 관련 - inline
	bool		SetMemoryBitToDevice(int index, bool value);
	bool		SetMemoryBitToDevice(CString address, bool value);
	bool		SetMemoryNBitToDevice(int index, long value, long lSize);
	bool		SetMemoryNBitToDevice(CString address, long value, long lSize);
	bool		SetMemoryWordToDevice(int index, float value);
	bool		SetMemoryWordToDevice(CString address, float value);
	bool		SetMemoryNWordToDevice(int index, float* value, long lSize);
	bool		SetMemoryNWordToDevice(CString address, float* value, long lSize);
	bool		SetMemoryNStrToDevice(int index, TCHAR* value, long lSize);
	bool		SetMemoryNStrToDevice(CString address, TCHAR* value, long lSize);
	bool		CheckActionDone(int index, DWORD dwTimeout, bool value);
	bool		ClearLinkSignalBit();
	bool		ClearLinkSignalWord();

public:
	CIOFunction(CLogFileAccessor* pLogFile, CString strDeviceModel, CString strConfigPath, CString strFilePath, CString strCategory);
	~CIOFunction();	

	void DisConnect();
};

