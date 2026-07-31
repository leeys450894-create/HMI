#include "pch.h"
#include "Comm232Function.h"

#include "Comm232Config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CComm232Function::CComm232Function()
	: m_pConfig(nullptr)
	, m_nIndex(0)
{
    m_pThread			= nullptr;
    m_hCloseEvent		= CreateEvent(NULL, FALSE, FALSE, NULL);	// auto reset, initial reset
    m_bLive				= false;
	m_hComm				= nullptr;
    m_bConnected		= false;
    m_bDataRecieveOK	= false;

    m_CallbackPtr		= nullptr;
    m_CallbackFunc		= nullptr;
	m_pLogFileAccessor  = nullptr;
}

CComm232Function::~CComm232Function()
{
	Finalize();

 //   Close();

	if(m_hCloseEvent)
	{
		CloseHandle(m_hCloseEvent);
	}
}

/////////////////////////////////////////////////////////////////////////////
// Open & Close
bool CComm232Function::Open(long nIndex)
{
	if(m_pConfig == nullptr)
		return false;

	m_nIndex = nIndex;

	if(Is_Connected())
    {
		ClosePort();
    }

	//CLogWrapper::GetInstance()->KMLOG(SYST, _T("CComm232Function Open"));

	CString strPortID;
	strPortID.Format(_T("COM%d"), m_nIndex);
	m_pConfig->SetParam_PortID(strPortID);

	return OpenPort();
}

bool CComm232Function::Close()
{
    if(Is_Connected())
    {
        ClosePort();
	}

	//CLogWrapper::GetInstance()->KMLOG(SYST, _T("CComm232Function Close"));

	ClosePort();

    return !Is_Connected();
}

bool CComm232Function::OpenPort(DWORD wPortID)
{
	if(m_pConfig == nullptr)
		return false;

	CString strPortID;
	strPortID.Format(_T("COM%d"), wPortID);
	m_pConfig->SetParam_PortID(strPortID);

	return OpenPort();
}

bool CComm232Function::OpenPort()
{
	if(m_pConfig == nullptr)
		return false;
    // 이미 생성되어 있음.
    if(m_hComm)
        return true;

    m_bConnected = false;

    // overlapped structure 변수 초기화.
    m_osRead.Offset = 0;
    m_osRead.OffsetHigh = 0;

    if( !(m_osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) )
    {
        //AfxMessageBox(_T("fail m_osRead CreateEvent"));
		//CLogWrapper::GetInstance()->KMLOG(ERRO, _T("CComm232Function fail m_osRead CreateEvent"));
        return false;
    }

    m_osWrite.Offset = 0;
    m_osWrite.OffsetHigh = 0;

    if(! (m_osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)))
    {
        //AfxMessageBox(_T("fail m_osWrite CreateEvent"));
		//CLogWrapper::GetInstance()->KMLOG(ERRO, _T("CComm232Function fail m_osWrite CreateEvent"));
        return false;
    }

    // 실제적인...RS 232 포트 열기..
	CString strPort = m_pConfig->GetParam_PortID();
	if (strPort == _T("NONE"))
		return false;

	// Serial Port가 2자리 이상이면 Comport 앞에 "\\\\.\\"를 붙여준다.
	if (strPort.GetLength() > 4)
		strPort.Format(_T("\\\\.\\%s"), strPort.GetString());

	m_hComm = CreateFile(strPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);

    // 포트 열기에 실패하면..
    if(m_hComm == INVALID_HANDLE_VALUE)
    {
		CloseHandle(m_hComm);
		m_hComm = nullptr;
        //AfxMessageBox(_T("fail Port open"));
		//CLogWrapper::GetInstance()->KMLOG(ERRO, _T("CComm232Function fail Port open"));
        return false;
    }

    //----------------------------------------------------------------------------------
    // 포트 상태 설정
    //----------------------------------------------------------------------------------
    // EV_RXCHAR event 설정...데이터가 들어오면.. 수신 이벤트가 발생하게끔..
    SetCommMask( m_hComm, EV_RXCHAR);

    // InQueue, OutQueue 크기 설정.
    SetupComm( m_hComm, QUEUE_BUFF_SIZE, QUEUE_BUFF_SIZE);

    // 포트 비우기.
    PurgeComm( m_hComm,	PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR);

    // timeout 설정.
    COMMTIMEOUTS timeouts;
    timeouts.ReadIntervalTimeout			= 0xFFFFFFFF;
    timeouts.ReadTotalTimeoutMultiplier		= 0;
    timeouts.ReadTotalTimeoutConstant		= 0;
    timeouts.WriteTotalTimeoutMultiplier	= 2 * CBR_9600 / _ttoi(m_pConfig->GetParam_BaudRate());
    timeouts.WriteTotalTimeoutConstant		= 0;
    SetCommTimeouts( m_hComm, &timeouts);

    // dcb 설정....
    // 포트의 실제적인..제어를 담당하는 DCB 구조체값 셋팅..
    DCB dcb;
    dcb.DCBlength = sizeof(DCB);

    GetCommState( m_hComm, &dcb);		// 현재 설정된 값 중에서..

	dcb.BaudRate	= _ttoi(m_pConfig->GetParam_BaudRate());										// 보드레이트를 바꾸고.. CBR_19200;
    dcb.ByteSize	= _ttoi(m_pConfig->GetParam_DataBits());										// Data 8 Bit
    dcb.Parity		= m_pConfig->GetSerialStringToValue(m_pConfig->GetParam_Parity());				// Noparity
    dcb.StopBits	= m_pConfig->GetSerialStringToValue(m_pConfig->GetParam_StopBits());			// 1 Stop Bit

    // 포트를 재..설정값으로.. 설정해보고..
    if( !SetCommState( m_hComm, &dcb) )
    {
        CloseHandle(m_hComm);
		m_hComm = nullptr;
		CloseHandle(m_osRead.hEvent);
        CloseHandle(m_osWrite.hEvent);

        //AfxMessageBox(_T("fail SetCommState"));
		//CLogWrapper::GetInstance()->KMLOG(WARN, _T("CComm232Function fail SetCommState"));
        return false;
    }

    // 포트 감시 쓰레드 생성.
    Initialize();

    // 포트 감시 쓰레드 생성.
    m_bConnected = true;

	//CLogWrapper::GetInstance()->KMLOG(SYST, _T("CComm232Function Open Port Success"));
	Sleep(1000);

    return true;
}

// 포트를 닫는다.
void CComm232Function::ClosePort()
{
	if (m_hComm != nullptr)
	{
		// 모든 Event mask를 없앤다.
		SetCommMask(m_hComm, 0);

		// 통신 Queue들을 초기화 한다.
		PurgeComm(m_hComm, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR);

		// 파일 핸들을 닫는다.
		if (!CloseHandle(m_hComm))
		{
			//AfxMessageBox(_T("Port Close Fail"));
			//CLogWrapper::GetInstance()->KMLOG(WARN, _T("CComm232Function Close Port Fail"));
			return;
		}

		CloseHandle(m_osRead.hEvent);
		CloseHandle(m_osWrite.hEvent);
	}
	
	m_hComm			= nullptr;	
    m_bConnected	= false;

	//CLogWrapper::GetInstance()->KMLOG(SYST, _T("CComm232Function Close Port Success"));
	Sleep(1000);
}


/////////////////////////////////////////////////////////////////////////////
// Write & Read

// 포트에 strBuf의 내용을 BYTE-unsinged char 형태로 쓴다. 실제로 쓰여진 Byte수를 리턴한다.
DWORD CComm232Function::WriteComm(CString strData)
{
    // 포트가 연결되지 않은 상태이면..
    if( Is_Connected() == false)
        return 0;

	BYTE WriteBuff[COMM_BUFF_SIZE] = {0, };
    memset(WriteBuff, 0, COMM_BUFF_SIZE);

    DWORD dwToWrite = strData.GetLength();
	for (int i = 0; i < strData.GetLength(); ++i)
	{
		WriteBuff[i] = strData.GetAt(i);
	}

	if (dwToWrite == 2)
	{
		dwToWrite = 3;
	}

    // 인자로 들어온 버퍼의 내용을 nToWrite 만큼 쓰고.. 쓴 갯수를.,dwWrite 에 넘김.
    DWORD dwWritten = 0;
    if( !WriteFile( m_hComm, (BYTE *)WriteBuff, dwToWrite, &dwWritten, &m_osWrite))
    {
        DWORD dwError = GetLastError();
		DWORD dwErrorFlags = 0;
		COMSTAT	comstat;

        // 아직 전송할 문자가 남았을 경우..
        if(dwError == ERROR_IO_PENDING)
        {
            // I/O가 완료될 때 가지 기다린다.
            //::WaitForSingleObject(m_osWrite.hEvent, 1000);

            // 읽을 문자가 남아 있거나 전송할 문자가 남아 있을 경우 Overapped IO의
            // 특성에 따라 ERROR_IO_PENDING 에러 메시지가 전달된다.
            // timeouts에 정해준 시간만큼 기다려준다.
            while(! GetOverlappedResult( m_hComm, &m_osWrite, &dwWritten, TRUE))
            {
                dwError = GetLastError();

                if (dwError != ERROR_IO_INCOMPLETE)
                {
                    ClearCommError( m_hComm, &dwErrorFlags, &comstat);
                    break;
                }
            }
        }
        else
        {
            dwWritten = 0;
            ClearCommError( m_hComm, &dwErrorFlags, &comstat);
        }
    }

    // 실제 포트로 쓰여진 갯수를 리턴..
    return dwWritten;
}

// 포트로부터 pBuff에 nToWrite만큼 읽는다. 실제로 읽혀진 Byte수를 리턴한다.
DWORD CComm232Function::ReadComm(BYTE *pBuff, DWORD dwToRead)
{
    DWORD	dwRead, dwError, dwErrorFlags;
    COMSTAT comstat;

    // system queue에 도착한 byte수만 미리 읽는다.
    ClearCommError( m_hComm, &dwErrorFlags, &comstat);

    // 시스템 큐에서 읽을 거리가 있으면..
    dwRead = comstat.cbInQue;

    if(dwRead > 0)
    {
        // 버퍼에 일단 읽어들이는데.. 만일..읽어들인값이 없다면..
        if( !ReadFile( m_hComm, pBuff, dwToRead, &dwRead, &m_osRead) )
        {
            dwError = GetLastError();

            // 읽을 거리가 남았으면..
            if(dwError == ERROR_IO_PENDING)
            {
                // I/O가 완료될 때 가지 기다린다.
                //::WaitForSingleObject(m_osRead.hEvent, 1000);

                // timeouts에 정해준 시간만큼 기다려준다.
                while(! GetOverlappedResult( m_hComm, &m_osRead, &dwRead, TRUE))
                {
                    dwError = GetLastError();

                    if(dwError != ERROR_IO_INCOMPLETE)
                    {
                        ClearCommError( m_hComm, &dwErrorFlags, &comstat);
                        break;
                    }
                }
            }
            else
            {
                dwRead = 0;
                ClearCommError( m_hComm, &dwErrorFlags, &comstat);
            }
        }
    }
	
    // 실제 읽어들인 갯수를 리턴.
    return dwRead;

}

// UM_COMM_READ Message를 받은 MainWnd이 이 함수를 호출하여 Ring-Buffer의 내용을 읽어간다.
bool CComm232Function::RecieveData(CString &strMsg)
{
	if (!m_bDataRecieveOK)
		return false;

	m_bDataRecieveOK = false;

	BYTE QueueBuff[COMM_BUFF_SIZE] = { 0, };
	memset(QueueBuff, 0, COMM_BUFF_SIZE);

	const long nSize = m_QueueRead.GetSize();

	if (nSize < COMM_BUFF_SIZE)
	{
		for (long n = 0; n < nSize; ++n)
		{
			QueueBuff[n] = m_QueueRead.GetBuff(n);
		}

		m_QueueRead.Clear();

		strMsg = QueueBuff;

		//USES_CONVERSION;
		CString strLog;
		strLog.Format(_T("CComm232Function COM%d - RECIEVE %s"), m_nIndex, strMsg);

		if (m_pLogFileAccessor != nullptr)
		{
			m_pLogFileAccessor->KMLOG(SYST, strLog, true);
		}
		else
		{
			CLogWrapper::GetInstance()->KMLOG(INSP, strLog);
		}
	}
	else
	{
		m_QueueRead.Clear();

		return false;
	}
	
    return nSize != 0;
}

bool CComm232Function::SendCommand(CString strCommand)
{
    /*if(Is_Connected() == false)
    {
        return false;
    }*/

    bool bRet = (WriteComm(strCommand) != 0) ? true : false;

	CString strLog; 
    strLog.Format(_T("SendCommand COM%d - SEND(%d), %s"), m_nIndex, bRet, strCommand);
    if (m_pLogFileAccessor != nullptr)
    {
	    m_pLogFileAccessor->KMLOG(SEND, strLog, true);
    }

    Sleep(10);

    return bRet;
}

void CComm232Function::SetLogFileAccessor(CLogFileAccessor			*pLogFileAccessor)
{
	m_pLogFileAccessor = pLogFileAccessor;
}

UINT CComm232Function::ThreadWorkingLoop(LPVOID pParam)
{
    CComm232Function *pThis = (CComm232Function *)pParam;

    //----------------------------------------------------------------------------
    //----------------------------------------------------------------------------
    // Event, Overap Structure 설정.
    OVERLAPPED os;
    memset(&os, 0, sizeof(OVERLAPPED));

    if(	!(os.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL))	|| !SetCommMask(pThis->m_hComm, EV_RXCHAR)	)
    {
		//CLogWrapper::GetInstance()->KMLOG(ERRO, _T("CComm232Function Error while creating ThreadWatchComm"));
        return 0;
    }

    // 포트를 감시하는 루프. = Comm Port connection이 이루어지면, 무조건 루프를 돌기 시작한다.
    //----------------------------------------------------------------------------
	
    pThis->m_bLive = true;

	// 2.5초 마다 대기한다.
	const DWORD dwWaitTime = 2500;

	while (1)
    {
        pThis->ThreadWorkingLoop();

		// WaitForSingleObject의 기능 검사 지정된 객체의 현재 상태. 
		// 객체의 상태가 신호가 아닌 경우 호출 스레드는 효율적인 대기 상태가됩니다. 
		if(WaitForSingleObject(pThis->m_hCloseEvent, dwWaitTime) == WAIT_OBJECT_0)
		{
			// 객체를 삭제하고 thread 를 닫도록 하자.
			pThis->Close();
			break;
		}
    }

    // 포트가 ClosePort에 의해 닫히면 종료.
	if(pThis->Is_Connected())
    {
        CloseHandle(os.hEvent);
    }
	
    pThis->m_bLive = false;

    return 0;
}

void CComm232Function::ThreadWorkingLoop()
{
	if (Is_Connected())
	{
		// 포트에 읽을 거리가 올때까지 기다린다.
		DWORD dwEvent = 0;
		WaitCommEvent(m_hComm, &dwEvent, NULL);

		BYTE ReadBuff[COMM_BUFF_SIZE];	// InQueue의 데이터를 읽어오기 위한 Read-Buffer
		memset(ReadBuff, 0, COMM_BUFF_SIZE);

		// 데이터가 수신되었다는 메세지가 발생하면..
		if ((dwEvent & EV_RXCHAR) == EV_RXCHAR)
		{
			DWORD dwRead = 0;

			// 포트에서 읽을 수 있는 만큼 읽는다.
			do
			{
				dwRead = ReadComm(ReadBuff, COMM_BUFF_SIZE);	//들어온 데이터 읽어 오기

				if (QUEUE_BUFF_SIZE - m_QueueRead.GetSize() > (int)dwRead)
				{
					for (WORD i = 0; i < dwRead; i++)
					{
						m_QueueRead.PutByte(ReadBuff[i]);			//큐 버퍼에  들어온 데이터 넣기
					}
				}
				else
				{
					//큐버퍼의 크기를 초과하면 경고 메시지 보냄
					//CLogWrapper::GetInstance()->KMLOG(ERRO, _T("CComm232Function buff full"));
				}
			} while (dwRead);

			// 약속된 마지막 값 확인.
			m_bDataRecieveOK = false;

			//if ((m_QueueRead.GetBuff(m_QueueRead.GetSize() - 1) == ASCII_LF) ||
			//	(m_QueueRead.GetBuff(m_QueueRead.GetSize() - 1) == ASCII_CR) ||
			//	(m_QueueRead.GetBuff(m_QueueRead.GetSize() - 1) == ASCII_CRLF) ||
			//	(m_QueueRead.GetBuff(m_QueueRead.GetSize() - 1) == ASCII_ETX) ||
			//	(17 <= m_QueueRead.GetSize()) )
			//if(6 < m_QueueRead.GetSize())
			{
				m_bDataRecieveOK = true;

				if (RecieveData(m_strRecieveData) == true)
				{
					// 읽어가도록 메시지를 보낸다.
					if (m_CallbackFunc)
					{
						m_CallbackFunc(m_CallbackPtr, (LPVOID)(LPCTSTR)m_strRecieveData);
					}
				}
			}
        }
    }
}

bool CComm232Function::Initialize()
{
    if(m_pThread == nullptr)
    {
        m_pThread = AfxBeginThread(ThreadWorkingLoop, this);
    }
	
    return true;
}

void CComm232Function::Finalize()
{
	// 먼저 객체를 삭제하고 thread 를 닫도록 하자.
	SetEvent(m_hCloseEvent);

	if(m_pThread && m_bLive)
	{
//		WaitForSingleObject(m_pThread->m_hThread, INFINITE);
		WaitForSingleObject(m_pThread->m_hThread, 1000);
	}

	m_pThread = nullptr;
}

bool CComm232Function::IsLive()
{
   return m_bLive;
}

void CComm232Function::SetConfig(CComm232Config *pConfig)
{
	m_pConfig = pConfig;
};

