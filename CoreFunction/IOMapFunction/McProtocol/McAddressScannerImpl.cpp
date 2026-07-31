#include "pch.h"
#include "McAddressScannerImpl.h"

//#include "McProtocolCtrl.h"
#include "..\..\LogFunction\LogAccessor\LogFileAccessor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CMcAddressScannerImpl::CMcAddressScannerImpl(CString strDefaultFilePath)
	: m_strDefaultFilePath(strDefaultFilePath)
	, m_pLogFileAccessor(new CLogFileAccessor)
{
	m_pThread		= AfxBeginThread(ThreadRun, this);
    m_bThreadLive	= false;
	m_hStartEvent	= CreateEvent(NULL, TRUE, FALSE, NULL);
    m_hCloseEvent	= CreateEvent(NULL, FALSE, FALSE, NULL);
}

CMcAddressScannerImpl::~CMcAddressScannerImpl()
{
    //----------------------------------------------------------------------
    // Thread 종료
    //----------------------------------------------------------------------
    ResetEvent(m_hStartEvent);
    SetEvent(m_hCloseEvent);
	if(m_pThread && m_bThreadLive)
	{
		WaitForSingleObject(m_pThread->m_hThread, INFINITE);
	}
    CloseHandle(m_hStartEvent);
    CloseHandle(m_hCloseEvent);

	//----------------------------------------------------------------------
    // Data 삭제
    //----------------------------------------------------------------------
    if(m_pLogFileAccessor)
    {
        delete m_pLogFileAccessor;
        m_pLogFileAccessor = nullptr;
    }
}

void CMcAddressScannerImpl::Start()
{
    SetEvent(m_hStartEvent);
}

int CMcAddressScannerImpl::hextoi(CString strHex)
{
	unsigned char bin[8] = { 0, };
	unsigned int m_nDEC = 0;
	int nLen = strHex.GetLength();
	if (nLen > 8)
		return -1;
	for (int i = 0; i<nLen; i++) {
		bin[i] = HexToByte(strHex[(nLen - 1) - i]);
		if (bin[i] >= 0 && bin[i] <= 15) {    // 0 ~ A ~ F
			m_nDEC += bin[i] << (4 * i);     // 10진수 출력
		}
		else if (bin[i] == 32) {
			return -1;
		}
	}
	return m_nDEC;
}

unsigned char CMcAddressScannerImpl::HexToByte(char hex)
{
	switch (hex)
	{
	case '0': return 0;
	case '1': return 1;
	case '2': return 2;
	case '3': return 3;
	case '4': return 4;
	case '5': return 5;
	case '6': return 6;
	case '7': return 7;
	case '8': return 8;
	case '9': return 9;
	case 'A': case 'a': return 10;
	case 'B': case 'b': return 11;
	case 'C': case 'c': return 12;
	case 'D': case 'd': return 13;
	case 'E': case 'e': return 14;
	case 'F': case 'f': return 15;
	}
	return 32;
}





long CMcAddressScannerImpl::Initialize(bool bRunMode, CString strIP, long nPort)
{
	return 0;
}

long CMcAddressScannerImpl::Close()
{
	return 0;
}

bool CMcAddressScannerImpl::IsConnected()
{
	return false;
}

void CMcAddressScannerImpl::DisConnect()
{

}

UINT CMcAddressScannerImpl::ThreadRun(LPVOID p1)
{
    CMcAddressScannerImpl *pThis = (CMcAddressScannerImpl *)p1;

    pThis->m_bThreadLive = true;

    while (1)
    {
        // Event 가 set될 때까지 기다린다.
        if (WaitForSingleObject(pThis->m_hStartEvent, 10) == WAIT_OBJECT_0)
        {
            pThis->ThreadRun();

			//--------------------------------------------------------------------------------------
			// 너무 빨리 읽지 말고 100ms 정도로 읽자. Gxworks2 가 멈추는 현상이 있다.
			// 홍재희B 가 50ms 정도로 읽어야 한다고 했지만ㅋㅋㅋ 혹시 몰라서, main S/W 에서 사용하는
			// 시간이 100ms 인것을 확인 하였음. 이미 정상적으로 잘 사용하고 있는 상태이므로 동일하게 한다.
			// 확인결과 정상적으로 data 가 update 되려면 100~200ms 이상은 읽어줘야 한다. 
			//--------------------------------------------------------------------------------------
			Sleep(200);
        }

        // Close Event 가 set될 때까지 기다린다.
        if (WaitForSingleObject(pThis->m_hCloseEvent, 10) == WAIT_OBJECT_0)
        {
            break;
        }

		Sleep(1);
    }

    pThis->m_bThreadLive = false;

    return 0;
}

void CMcAddressScannerImpl::ThreadRun()
{
}

bool CMcAddressScannerImpl::ReadFileCSV(CString strFilename)
{
    return false;
}

bool CMcAddressScannerImpl::ReadFileXLS(CString strFileName)
{
    return false;
}

bool CMcAddressScannerImpl::ReadFileVerify(CString strFileName)
{
    return false;
}

bool CMcAddressScannerImpl::SetDefaultFile(CString strFileName)
{
    return false;
}

bool CMcAddressScannerImpl::SetMemoryBit(short addr, long val, long lBit/* = 1*/)
{
	return false;
}

bool CMcAddressScannerImpl::SetMemoryBit(CString strAddr, long val, long lBit/* = 1*/)
{
	return false;
}

bool CMcAddressScannerImpl::SetMemoryWord(long addr, float* val, long nWord/* = 1*/)
{
	return false;
}

bool CMcAddressScannerImpl::SetMemoryStr(long addr, TCHAR* val, long nWord/* = 1*/)
{
	return false;
}

bool CMcAddressScannerImpl::CheckActionDone(long addr, DWORD dwTimeout, bool val/* = true*/)
{
	return false;
}

long CMcAddressScannerImpl::GetAddressImportCount()
{
	return 0;
}

long CMcAddressScannerImpl::GetAddressExportCount()
{
	return 0;
}
	
bool CMcAddressScannerImpl::GetAddressImportDataValue(UINT Index)
{
	return false;
}

void CMcAddressScannerImpl::GetParam_DIOMapMemoryWordExportArray(TCHAR** pValue, long* plSize)
{
	;
}

CString CMcAddressScannerImpl::GetAddressImportDataValueString(UINT Index)
{
	return _T("");
}

bool CMcAddressScannerImpl::GetAddressExportDataValue(UINT Index)
{
	return false;
}

CString CMcAddressScannerImpl::GetAddressExportDataValueString(UINT Index)
{
	return _T("");
}

void CMcAddressScannerImpl::GetAddressExportDataValueArray(TCHAR** pValue, long* plSize)
{
	;
}

SADDRESSMAPDATA *CMcAddressScannerImpl::GetAddressImportDataPtr()
{
	return nullptr;
}

SADDRESSMAPDATA *CMcAddressScannerImpl::GetAddressExportDataPtr()
{
	return nullptr;
}


long CMcAddressScannerImpl::GetAddressAlarmCount()
{
	return 0;
}

bool CMcAddressScannerImpl::IsOccur_addressAlarm()
{
	return false;
}

SADDRESSMAPDATA	*CMcAddressScannerImpl::GetAddressAlarmDataPtr()
{
	return nullptr;
}

std::vector<CString> CMcAddressScannerImpl::GetCommand(CString &strData)
{
	std::vector<CString> vecResult;

	vecResult.clear();

	return vecResult;
}


