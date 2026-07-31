#include "pch.h"
#include "IOMapFunction.h"

//#include "CCLinkBoard\CCLinkBoardManager.h"
//#include "MxComponent\MxComponentManager.h"
#include "McProtocol\McProtocolManager.h"
//#include "IOBorad\IOBoardManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CIOFunction::CIOFunction(CLogFileAccessor* pLogFile, CString strDeviceModel, CString strConfigPath, CString strFilePath, CString strCategory)
{
	m_pCtrlImpl = nullptr;

	if(strDeviceModel == _T("MXCOMPONENT"))
	{
//		m_pCtrlImpl = dynamic_cast<CIOMapImpl*>(new CMxComponentManager(strConfigPath, strLogFilePath, strFilePath, strCategory));
	}
	else if (strDeviceModel == _T("MCPROTOCOL"))
	{
		m_pCtrlImpl = dynamic_cast<CIOMapImpl*>(new CMcProtocolManager(pLogFile, strConfigPath, strFilePath, strCategory));
	}
	else if(strDeviceModel == _T("CCLINK"))
	{
		//m_pCtrlImpl = dynamic_cast<CIOMapImpl*>(new CCCLinkBoardManager(strConfigPath, strFilePath, strCategory));
	}
	else if (strDeviceModel == _T("IOBOARD"))
	{
//		m_pCtrlImpl = dynamic_cast<CIOMapImpl*>(new CIOBoardManager(pLogFile, strConfigPath, strFilePath, strCategory));
	}
	else 
	{
//		m_pCtrlImpl = dynamic_cast<CIOMapImpl*>(new CMxComponentManager(strConfigPath, strLogFilePath, strFilePath, strCategory));
	}
}

CIOFunction::~CIOFunction()
{
    if (m_pCtrlImpl)
    {
        delete m_pCtrlImpl;
        m_pCtrlImpl = nullptr;
    }
}

void CIOFunction::DisConnect()
{
	if (m_pCtrlImpl != nullptr)
		m_pCtrlImpl->DisConnect();
}

//------------------------------------------------------------------
// 외부 접근 가능 함수
//------------------------------------------------------------------
bool CIOFunction::InitDevice(short index)
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->InitDevice(index);
	else
		return false;
}

CString	CIOFunction::GetDeviceName()
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->GetDeviceName();
	else
		return _T("");
}

bool CIOFunction::IsConnected()
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->IsConnected();
	else
		return false;
}

// address data value 관련 - teach
long CIOFunction::GetParam_DIOMapAddressTotalCount()
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->GetParam_DIOMapAddressTotalCount();
	else
		return 0;
}

long CIOFunction::GetParam_DIOMapAddressBitCount(int nType)
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->GetParam_DIOMapAddressBitCount(nType);
	else
		return 0;
}

CString CIOFunction::GetParam_DIOMapAddressBitImport(int index)
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->GetParam_DIOMapAddressBitImport(index);
	else
		return _T("");
}

CString CIOFunction::GetParam_DIOMapAddressBitImportName(int index)
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->GetParam_DIOMapAddressBitImportName(index);
	else
		return _T("");
}

bool CIOFunction::GetParam_DIOMapMemoryBitImport(int index)
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->GetParam_DIOMapMemoryBitImport(index);
	else
		return false;
}

CString CIOFunction::GetParam_DIOMapAddressBitExport(int index)
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->GetParam_DIOMapAddressBitExport(index);
	else
		return _T("");
}

CString CIOFunction::GetParam_DIOMapAddressBitExportName(int index)
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->GetParam_DIOMapAddressBitExportName(index);
	else
		return _T("");
}

bool CIOFunction::GetParam_DIOMapMemoryBitExport(int index)
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->GetParam_DIOMapMemoryBitExport(index);
	else
		return false;
}

long CIOFunction::GetParam_DIOMapAddressWordCount(int nType)
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->GetParam_DIOMapAddressWordCount(nType);
	else
		return 0;
}

CString CIOFunction::GetParam_DIOMapAddressWordImport(int index)
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->GetParam_DIOMapAddressWordImport(index);
	else
		return _T("");
}

CString CIOFunction::GetParam_DIOMapAddressWordImportName(int index)
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->GetParam_DIOMapAddressWordImportName(index);
	else
		return _T("");
}

long CIOFunction::GetParam_DIOMapMemoryWordImport(int index)
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->GetParam_DIOMapMemoryWordImport(index);
	else
		return 0;
}

CString CIOFunction::GetParam_DIOMapMemoryWordImportString(int index)
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->GetParam_DIOMapMemoryWordImportString(index);
	else
		return _T("");
}

CString CIOFunction::GetParam_DIOMapAddressWordExport(int index)
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->GetParam_DIOMapAddressWordExport(index);
	else
		return _T("");
}

CString CIOFunction::GetParam_DIOMapAddressWordExportName(int index)
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->GetParam_DIOMapAddressWordExportName(index);
	else
		return _T("");
}

long CIOFunction::GetParam_DIOMapMemoryWordExport(int index)
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->GetParam_DIOMapMemoryWordExport(index);
	else
		return 0;
}

CString CIOFunction::GetParam_DIOMapMemoryWordExportString(int index)
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->GetParam_DIOMapMemoryWordExportString(index);
	else
		return _T("");
}


void CIOFunction::GetParam_DIOMapMemoryWordExportArray(int index, TCHAR** pValue, long* plSize)
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->GetParam_DIOMapMemoryWordExportArray(index, pValue, plSize);
}





// address data value 관련 - inline
bool CIOFunction::SetMemoryBitToDevice(int index, bool value)
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->SetMemoryBitToDevice(index, value);
	else
		return false;
}

bool CIOFunction::SetMemoryBitToDevice(CString address, bool value)
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->SetMemoryBitToDevice(address, value);
	else
		return false;
}

bool CIOFunction::SetMemoryNBitToDevice(int index, long value, long lSize)
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->SetMemoryNBitToDevice(index, value, lSize);
	else
		return false;
}

bool CIOFunction::SetMemoryNBitToDevice(CString address, long value, long lSize)
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->SetMemoryNBitToDevice(address, value, lSize);
	else
		return false;
}

bool CIOFunction::SetMemoryWordToDevice(int index, float value)
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->SetMemoryWordToDevice(index, value);
	else
		return false;
}

bool CIOFunction::SetMemoryWordToDevice(CString address, float value)
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->SetMemoryWordToDevice(address, value);
	else
		return false;
}

bool CIOFunction::SetMemoryNWordToDevice(int index, float* value, long lSize)
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->SetMemoryNWordToDevice(index, value, lSize);
	else
		return false;
}

bool CIOFunction::SetMemoryNWordToDevice(CString address, float* value, long lSize)
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->SetMemoryNWordToDevice(address, value, lSize);
	else
		return false;
}

bool CIOFunction::SetMemoryNStrToDevice(int index, TCHAR* value, long lSize)
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->SetMemoryNStrToDevice(index, value, lSize);
	else
		return false;
}

bool CIOFunction::SetMemoryNStrToDevice(CString address, TCHAR* value, long lSize)
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->SetMemoryNStrToDevice(address, value, lSize);
	else
		return false;
}

bool CIOFunction::CheckActionDone(int index, DWORD dwTimeout, bool value)
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->CheckActionDone(index, dwTimeout, value);
	else
		return false;
}

bool CIOFunction::ClearLinkSignalBit()
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->ClearLinkSignalBit();
	else
		return false;
}

bool CIOFunction::ClearLinkSignalWord()
{
	if (m_pCtrlImpl != nullptr)
		return m_pCtrlImpl->ClearLinkSignalWord();
	else
		return false;
}


