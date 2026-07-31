#include "pch.h"
#include "IOMapImpl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CIOMapImpl::CIOMapImpl()
{	
}

CIOMapImpl::~CIOMapImpl()
{
}



//------------------------------------------------------------------
// 공통 인터페이스 함수
//------------------------------------------------------------------
// address data value 관련 - teach
long CIOMapImpl::GetParam_DIOMapAddressTotalCount()
{
    return 0;
}

long CIOMapImpl::GetParam_DIOMapAddressBitCount(int nType)
{
    return 0;
}

CString CIOMapImpl::GetParam_DIOMapAddressBitImport(int index)
{
    return _T("");
}

CString CIOMapImpl::GetParam_DIOMapAddressBitImportName(int index)
{
    return _T("");
}

bool CIOMapImpl::GetParam_DIOMapMemoryBitImport(int index)
{
	return false;
}

CString CIOMapImpl::GetParam_DIOMapAddressBitExport(int index)
{
    return _T("");
}

CString CIOMapImpl::GetParam_DIOMapAddressBitExportName(int index)
{
    return _T("");
}

bool CIOMapImpl::GetParam_DIOMapMemoryBitExport(int index)
{
	return false;
}

long CIOMapImpl::GetParam_DIOMapAddressWordCount(int nType)
{
    return 0;
}
CString CIOMapImpl::GetParam_DIOMapAddressWordImport(int index)
{
    return _T("");
}

CString CIOMapImpl::GetParam_DIOMapAddressWordImportName(int index)
{
    return _T("");
}

long CIOMapImpl::GetParam_DIOMapMemoryWordImport(int index)
{
	return false;
}

CString CIOMapImpl::GetParam_DIOMapMemoryWordImportString(int index)
{
	return _T("");
}

CString CIOMapImpl::GetParam_DIOMapAddressWordExport(int index)
{
    return _T("");
}

CString CIOMapImpl::GetParam_DIOMapAddressWordExportName(int index)
{
    return _T("");
}

long CIOMapImpl::GetParam_DIOMapMemoryWordExport(int index)
{
	return false;
}

CString CIOMapImpl::GetParam_DIOMapMemoryWordExportString(int index)
{
	return _T("");
}

void CIOMapImpl::GetParam_DIOMapMemoryWordExportArray(int index, TCHAR** pValue, long* plSize)
{
	;
}

// address data value 관련 - inline
bool CIOMapImpl::SetMemoryBitToDevice(int index, bool value)
{
	return false;
}

bool CIOMapImpl::SetMemoryBitToDevice(CString address, bool value)
{
	return false;
}

bool CIOMapImpl::SetMemoryNBitToDevice(int index, long value, long lSize)
{
	return false;
}

bool CIOMapImpl::SetMemoryNBitToDevice(CString address, long value, long lSize)
{
	return false;
}

bool CIOMapImpl::SetMemoryWordToDevice(int index, float value)
{
	return false;
}

bool CIOMapImpl::SetMemoryWordToDevice(CString address, float value)
{
	return false;
}

bool CIOMapImpl::SetMemoryNWordToDevice(int index, float* value, long lSize)
{
	return false;
}

bool CIOMapImpl::SetMemoryNWordToDevice(CString address, float* value, long lSize)
{
	return false;
}

bool CIOMapImpl::SetMemoryNStrToDevice(int index, TCHAR* value, long lSize)
{
	return false;
}

bool CIOMapImpl::SetMemoryNStrToDevice(CString address, TCHAR* value, long lSize)
{
	return false;
}

bool CIOMapImpl::CheckActionDone(int index, DWORD dwTimeout, bool value)
{
	return false;
}

bool CIOMapImpl::ClearLinkSignalBit()
{
	return false;
}

bool CIOMapImpl::ClearLinkSignalWord()
{
	return false;
}

void CIOMapImpl::DisConnect()
{
	;
}