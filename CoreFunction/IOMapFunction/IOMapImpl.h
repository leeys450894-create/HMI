#pragma once

#include "DataStructure.h"

class CIOMapImpl
{

	//------------------------------------------------------------------
	// 공통 인터페이스 함수
	//------------------------------------------------------------------
public:
	virtual BOOL		LinkIniFile(bool bSave) = 0;	// pure virtual
	virtual BOOL		LinkDatFile(bool bSave) = 0;	// pure virtual
    virtual bool		InitDevice(int index) = 0;		// pure virtual
    virtual CString		GetDeviceName() = 0;			// pure virtual
    virtual bool		IsConnected() = 0;				// pure virtual
	virtual void		DisConnect();

	// address data value 관련 - teach, 알람에서 사용한다.
	virtual long		GetParam_DIOMapAddressTotalCount();

	virtual long		GetParam_DIOMapAddressBitCount(int nType = 0);
	virtual CString		GetParam_DIOMapAddressBitImport(int index);
	virtual CString		GetParam_DIOMapAddressBitImportName(int index);
	virtual bool		GetParam_DIOMapMemoryBitImport(int index);
	virtual CString		GetParam_DIOMapAddressBitExport(int index);
	virtual CString		GetParam_DIOMapAddressBitExportName(int index);
	virtual bool		GetParam_DIOMapMemoryBitExport(int index);
	virtual long		GetParam_DIOMapAddressWordCount(int nType = 0);
	virtual CString		GetParam_DIOMapAddressWordImport(int index);
	virtual CString		GetParam_DIOMapAddressWordImportName(int index);
	virtual long		GetParam_DIOMapMemoryWordImport(int index);
	virtual CString		GetParam_DIOMapMemoryWordImportString(int index);
	virtual CString		GetParam_DIOMapAddressWordExport(int index);
	virtual CString		GetParam_DIOMapAddressWordExportName(int index);
	virtual long		GetParam_DIOMapMemoryWordExport(int index);
	virtual CString		GetParam_DIOMapMemoryWordExportString(int index);
	virtual void		GetParam_DIOMapMemoryWordExportArray(int index, TCHAR** pValue, long* plSize);

	// address data value 관련 - inline
	virtual bool		SetMemoryBitToDevice(int index, bool value);
	virtual bool		SetMemoryBitToDevice(CString address, bool value);
	virtual bool		SetMemoryNBitToDevice(int index, long value, long lSize);
	virtual bool		SetMemoryNBitToDevice(CString address, long value, long lSize);
	virtual bool		SetMemoryWordToDevice(int index, float value);
	virtual bool		SetMemoryWordToDevice(CString address, float value);
	virtual bool		SetMemoryNWordToDevice(int index, float* value, long lSize);
	virtual bool		SetMemoryNWordToDevice(CString address, float* value, long lSize);
	virtual bool		SetMemoryNStrToDevice(int index, TCHAR* value, long lSize);
	virtual bool		SetMemoryNStrToDevice(CString address, TCHAR* value, long lSize) ;

	virtual bool		CheckActionDone(int index, DWORD dwTimeout, bool value);
	virtual bool		ClearLinkSignalBit();
	virtual bool		ClearLinkSignalWord();

public:
    CIOMapImpl();
    virtual ~CIOMapImpl();
};

