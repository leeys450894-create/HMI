#include "pch.h"
#include "CommSerialObjectECS.h"

#include "..\..\Comm232Function\Comm232Config.h"
#include "..\..\common\TimeChecker.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCommSerialObjectECS::CCommSerialObjectECS(CString strFilePath, CString strCategory)
{	
	m_pConfig = new CComm232Config(strFilePath, strCategory);
}

CCommSerialObjectECS::~CCommSerialObjectECS()
{
	if(m_pConfig)
	{
		delete m_pConfig;
		m_pConfig = nullptr;
	}
}



//------------------------------------------------------------------
// 공통 인터페이스 함수
//------------------------------------------------------------------
BOOL CCommSerialObjectECS::LinkIniFile(bool bSave)
{
	return TRUE;
}

BOOL CCommSerialObjectECS::LinkDatFile(bool bSave)
{
	return TRUE;
}

bool CCommSerialObjectECS::InitDevice(int port)
{
	return OpenPort(port);
}

bool CCommSerialObjectECS::CloseDevice()
{
	return Close();
}

CString CCommSerialObjectECS::GetDeviceName()
{
	return _T("ECS");
}

bool CCommSerialObjectECS::IsConnected()
{
	return Is_Connected();
}


//************************************************************************************************
// 조명 실제 제어 관련 함수 - 시퀀스
//************************************************************************************************
bool CCommSerialObjectECS::LedLight_Channel_TurnOn(long nChannel, long nIntensity)
{
	CString strCommand;
	return SetLedLampIntensity(nChannel, nIntensity, strCommand);
}
bool CCommSerialObjectECS::LedLight_Channel_TurnOff(long nChannel, long nIntensity)
{
	CString strCommand;
	return SetLedLampTurnOnOff(nChannel, nIntensity, enumLED_LAMP_CONTROL_TURN__OFF, strCommand);
}






//************************************************************************************************
// Led 조명 제어 전용 함수
//************************************************************************************************
bool CCommSerialObjectECS::SetLedLampIntensity(long nChannel, long nIntensity, CString &strCommand)
{
	// 컴마(,) 이거 붙이면 안댐
	// :1W11xxx - 1ch,  :1W12xxx - 2ch, 
	strCommand.Format(_T(":1W11%03d"), nIntensity);

	return SendCommand(strCommand);
}

bool CCommSerialObjectECS::GetLedLampIntensity(long nChannel, CString &strCommand)
{
	return false;
}

bool CCommSerialObjectECS::SetLedLampTurnOnOff(long nChannel, long nIntensity, enumLED_LAMP_CONTROL nCONTROL, CString &strCommand)
{
	// 컴마(,) 이거 붙이면 안댐
	// :1W11xxx - 1ch,  :1W12xxx - 2ch, 
	strCommand.Format(_T(":1W11000"));

	return SendCommand(strCommand);
}

bool CCommSerialObjectECS::GetLedLampTurnOnOff(long nChannel, CString &strCommand)
{
	return false;
}

bool CCommSerialObjectECS::WaitLedLampResponce(long nChannel, long nIntensity, enumRES_CHECK_TYPE nTYPE)
{
	CTimeChecker timer;

    while(1)
    {
		// 통신에서 데이터가 들어올때까지 기다리지 않는다. 그냥 바로 리턴한다. [ 무조건 들어왔다고 가정한다. ]
		if(!m_pConfig->GetParam_ResponceCheck())
		{
			Sleep(m_pConfig->GetParam_ResponceWaitTime());
			break;
		}
		else
		{
			if(GetLedLampStateCompare(nChannel, nIntensity, nTYPE))
				break;

			Sleep(1);

			// 통신에서 데이터가 들어올때까지 기다린다. - 제한 시간 30초
			const float fElapsed_sec = timer.Elapsed_ms() / 1000.f;

			if(m_pConfig->GetParam_ResponceTimeLimit() < fElapsed_sec)
			{
				return false;			
			}
		}
    }

    return true;
}










bool CCommSerialObjectECS::GetLedLampStateCompare(long nChannel, long nIntensity, enumRES_CHECK_TYPE nTYPE)
{
	CString strRecieveState;

	if(RecieveData(strRecieveState))
	{
		return true;
	}
	
	return false;
}

//************************************************************************************************
// 명령어 생성하는 부분 - INTENSITY
//************************************************************************************************
//	"setbrightness [ch] [bright]\r"
//		- 선택 된 채널의 밝기 값을 조정.
//		- [ch] : 조명의 채널을 선택한다. (4채널 : 0~3) (8채널 : 0~7)
//		- [bright] : 조명의 밝기값 (0~1024)
//	"# [ch] [bright]\r"
//		-."setbrightness"와 동일한 기능을 함.
CString CCommSerialObjectECS::MakeCommand_SetIntensity(long nChannel, long nIntensity)
{
	// 밝기 분포를 [ 0 ~ 256 : 0 ~ 1024 >>> 확대 시킴 ]
	//const double dScalingRate	= (double)nIntensity / 255.0;
	//const long nScaledIntensity = (long)(dScalingRate * 1024.0);

	// 	밝기 제어
	CString strCommand;
	//strCommand.Format(_T("setbrightness %d %d"), nChannel, nIntensity);
	strCommand.Format(_T("# %d %d"), nChannel, nIntensity);

	return strCommand;
}

// "getbrightness [ch]\r"
//		- 선택 된 채널의 밝기 값을 불러 옴.
//		- [ch] : 밝기값을 읽을 채널 (4채널 : 0~3) (8채널 : 0~7)
// "$ [ch]\r"
//		- getbrightness와 동일한 기능을 함.
CString CCommSerialObjectECS::MakeCommand_GetIntensity(long nChannel)
{
	// 밝기 값 불어옴
	CString strCommand;
	//strCommand.Format(_T("getbright %d"), nChannel);
	strCommand.Format(_T("$$ %d"), nChannel);

	return strCommand;
}

bool CCommSerialObjectECS::IsSameLedIntensity(long nChannel, long nIntensity, CString strVal)
{
	CString strData;
	strData.Format(_T("$$ %d "), nChannel);
	
	strVal.Replace(strData, _T(""));

	const int nIntensityValue = _ttoi(strVal);

	return (nIntensity == nIntensityValue) ? true : false;
}

//************************************************************************************************
// 명령어 생성하는 부분 - TURN ON/OFF
//************************************************************************************************
CString CCommSerialObjectECS::MakeCommand_SetTurnOnOff(long nChannel, long nIntensity, enumLED_LAMP_CONTROL nCONTROL)
{
	// 	조명 ON
	CString strCommand;

	if(nCONTROL == enumLED_LAMP_CONTROL_TURN___ON)	strCommand.Format(_T("setonoff %d 1"), nChannel);
	else											strCommand.Format(_T("setonoff %d 0"), nChannel);

	return strCommand;
}

CString CCommSerialObjectECS::MakeCommand_GetTurnOnOff(long nChannel)
{
	// 	조명 상태 체크
	CString strCommand;
	strCommand.Format(_T("getonoff %d"), nChannel);

	return strCommand;
}

bool CCommSerialObjectECS::IsSameLedTurnOnOff(long nChannel, bool bTurnOnOff, CString strVal)
{
	CString strData;
	strData.Format(_T("getonoff %d "), nChannel);

	strVal.Replace(strData, _T(""));

	const long nLampState = _ttoi(strVal);

	bool bLampState = false;

	if(bTurnOnOff)
	{
		bLampState = (nLampState == 1) ? true : false;
	}
	else
	{
		bLampState = (nLampState == 0) ? true : false;
	}

	return bLampState;
}










//----------------------------------------------------------------------------------
// Write 할때, char 형태로 데이터를 작성해야 한다.
//----------------------------------------------------------------------------------
DWORD CCommSerialObjectECS::WriteComm(CString strData)
{
    // 포트가 연결되지 않은 상태이면..
    if( IsConnected() == false)
        return 0;

    char WriteBuff[COMM_BUFF_SIZE];
    memset(WriteBuff, 0, COMM_BUFF_SIZE);

    const DWORD dwToWrite = strData.GetLength();
    USES_CONVERSION;
    memcpy(WriteBuff, T2A(strData.GetBuffer()), dwToWrite);

    DWORD	dwWritten, dwError, dwErrorFlags;
    COMSTAT	comstat;

    // 인자로 들어온 버퍼의 내용을 nToWrite 만큼 쓰고.. 쓴 갯수를.,dwWrite 에 넘김.
    if( !WriteFile( m_hComm, (char *)WriteBuff, dwToWrite, &dwWritten, &m_osWrite))
    {
        dwError = GetLastError();

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

void CCommSerialObjectECS::ThreadWorkingLoop()
{
    if(IsConnected())
    {
        // 포트에 읽을 거리가 올때까지 기다린다.
        DWORD dwEvent = 0;
        WaitCommEvent( m_hComm, &dwEvent, NULL);

        BYTE ReadBuff[COMM_BUFF_SIZE];	// InQueue의 데이터를 읽어오기 위한 Read-Buffer
        memset(ReadBuff, 0, COMM_BUFF_SIZE);

        // 데이터가 수신되었다는 메세지가 발생하면..
        if ((dwEvent & EV_RXCHAR) == EV_RXCHAR)
        {
            DWORD dwRead = 0;

            // 포트에서 읽을 수 있는 만큼 읽는다.
            do
            {
                dwRead = ReadComm( ReadBuff, COMM_BUFF_SIZE);	//들어온 데이터 읽어 오기

                if(QUEUE_BUFF_SIZE - m_QueueRead.GetSize() > (int)dwRead)
                {
                    for( WORD i = 0; i < dwRead; i++ )
                    {
                        m_QueueRead.PutByte(ReadBuff[i]);			//큐 버퍼에  들어온 데이터 넣기
                    }
                }
				else
				{
					//큐버퍼의 크기를 초과하면 경고 메시지 보냄
					//KMLOG(ERRO, _T("CCommSerialImpl buff full")); 
				}
            }
            while(dwRead);

            // 약속된 마지막 값 확인.
			m_bDataRecieveOK = false;

			if(m_QueueRead.GetBuff(m_QueueRead.GetSize()) == 0x00)
			{
				m_bDataRecieveOK = true;

				// 읽어가도록 메시지를 보낸다.
				if(m_CallbackFunc)
				{
					m_CallbackFunc(m_CallbackPtr, NULL);
				}
			}
		}
    }
}

