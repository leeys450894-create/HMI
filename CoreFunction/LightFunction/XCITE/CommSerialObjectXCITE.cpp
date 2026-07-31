#include "pch.h"
#include "CommSerialObjectXCITE.h"

#include "..\..\Comm232Function\Comm232Config.h"
#include "..\..\common\TimeChecker.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCommSerialObjectXCITE::CCommSerialObjectXCITE(CString strFilePath, CString strCategory)
{	
	m_pConfig = new CComm232Config(strFilePath, strCategory);
}

CCommSerialObjectXCITE::~CCommSerialObjectXCITE()
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
BOOL CCommSerialObjectXCITE::LinkIniFile(bool bSave)
{
	return TRUE;
}

BOOL CCommSerialObjectXCITE::LinkDatFile(bool bSave)
{
	return TRUE;
}

bool CCommSerialObjectXCITE::InitDevice(int port)
{
	if (OpenPort(port) == true)
	{
		return ConnectDevice();
	}

	return FALSE;
}

bool CCommSerialObjectXCITE::CloseDevice()
{
	return Close();
}

CString CCommSerialObjectXCITE::GetDeviceName()
{
	return _T("XCITE");
}

bool CCommSerialObjectXCITE::IsConnected()
{
	return Is_Connected();
}
	

//************************************************************************************************
// 조명 실제 제어 관련 함수 - 시퀀스
//************************************************************************************************
bool CCommSerialObjectXCITE::LedLight_Channel_TurnOn(long nChannel, long nIntensity)
{
	bool bRet = true;

	//Lamp On
	bRet &= SetLedLampTurnOnOff(nChannel, nIntensity, enumLED_LAMP_CONTROL_TURN___ON);
	bRet &= GetUnitStatus();
	bRet &= WaitLedLampResponce(nChannel, nIntensity, enumRES_CHECK_TYPE_TURN___ON);

	//Set Intensity
	bRet &= SetLedLampIntensity(nChannel, nIntensity);

	//안정화 시간(필요 한지 모르겠음, 테스트 필요)
	m_pConfig->GetParam_SerialCommHoldTime();

	//Intensity 확인
	bRet &= GetLedLampIntensity(nChannel);
	bRet &= WaitLedLampResponce(nChannel, nIntensity, enumRES_CHECK_TYPE_INTENSITY);

	//Shutter Open
	bRet &= OpenShutter();
	bRet &= WaitLedLampResponce(nChannel, nIntensity, enumRES_CHECK_TYPE_SHUTTER__ON);

	return bRet;
}
bool CCommSerialObjectXCITE::LedLight_Channel_TurnOff(long nChannel, long nIntensity)
{
	bool bRet = true;

	//Lamp Off
	bRet &= SetLedLampTurnOnOff(nChannel, nIntensity, enumLED_LAMP_CONTROL_TURN__OFF);
	bRet &= GetUnitStatus();
	bRet &= WaitLedLampResponce(nChannel, nIntensity, enumRES_CHECK_TYPE_SHUTTER_OFF);

	return bRet;
}

//************************************************************************************************
// Led 조명 제어 전용 함수
//************************************************************************************************
bool CCommSerialObjectXCITE::SetLedLampIntensity(long nChannel, long nIntensity, CString &strCommand)
{
	strCommand = MakeCommand_SetIntensity(nChannel, nIntensity);	

	return SendCommand(strCommand);
}

bool CCommSerialObjectXCITE::GetLedLampIntensity(long nChannel, CString &strCommand)
{
	strCommand = MakeCommand_GetIntensity(nChannel);
	
	return SendCommand(strCommand);
}

bool CCommSerialObjectXCITE::SetLedLampTurnOnOff(long nChannel, long nIntensity, enumLED_LAMP_CONTROL nCONTROL, CString &strCommand)
{
	strCommand = MakeCommand_SetTurnOnOff(nChannel, nIntensity, nCONTROL);

	return SendCommand(strCommand);
}

bool CCommSerialObjectXCITE::GetLedLampTurnOnOff(long nChannel, CString &strCommand)
{
	//GetUnitStatus, MakeCommand_GetUnitStatus 사용
	return true;
}

bool CCommSerialObjectXCITE::WaitLedLampResponce(long nChannel, long nIntensity, enumRES_CHECK_TYPE nTYPE)
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

bool CCommSerialObjectXCITE::GetLedLampStateCompare(long nChannel, long nIntensity, enumRES_CHECK_TYPE nTYPE)
{
	CString strRecieveState;

	if(RecieveData(strRecieveState))
	{
		if(nTYPE == enumRES_CHECK_TYPE_INTENSITY)	return IsSameLedIntensity(nChannel, nIntensity, strRecieveState);
		if(nTYPE == enumRES_CHECK_TYPE_TURN___ON)	return IsLedTurnOnOff(nChannel,		true,		strRecieveState);
		if(nTYPE == enumRES_CHECK_TYPE_TURN__OFF)	return IsLedTurnOnOff(nChannel,		false,		strRecieveState);
		if(nTYPE == enumRES_CHECK_TYPE_SHUTTER__ON)	return IsShutterOpenClose(nChannel, true,		strRecieveState);
		if(nTYPE == enumRES_CHECK_TYPE_SHUTTER_OFF)	return IsShutterOpenClose(nChannel, false,		strRecieveState);
	}
	
	return false;
}



//************************************************************************************************
// 명령어 생성하는 부분 - INTENSITY
//************************************************************************************************
// 0 - intensity level of 0 %
// 1 - intensity level of 12 %
// 2 - intensity level of 25 %
// 3 - intensity level of 50 %
// 4 - intensity level of 100 %
CString CCommSerialObjectXCITE::MakeCommand_SetIntensity(long nChannel, long nIntensity)
{
	if (nIntensity < 0) nIntensity = 0;
	if (nIntensity > 4) nIntensity = 4;

	CString strCommand = _T("");
	strCommand.Format(_T("%s%d\r"), XCITE_IX, nIntensity);

	return strCommand;
}

CString CCommSerialObjectXCITE::MakeCommand_GetIntensity(long nChannel)
{
	CString strCommand = XCITE_II;

	return strCommand;
}

bool CCommSerialObjectXCITE::IsSameLedIntensity(long nChannel, long nIntensity, CString strVal)
{
	if(strVal.IsEmpty())
		return false;

	CString strRecieveIntensity;
	strRecieveIntensity.Format(_T("%d"), _wtol(strVal));
	
	const int nRecieveIntensity = _ttoi(strRecieveIntensity);

	return (nIntensity == nRecieveIntensity) ? true : false;
}



//************************************************************************************************
// 명령어 생성하는 부분 - TURN ON/OFF
//************************************************************************************************

CString CCommSerialObjectXCITE::MakeCommand_SetTurnOnOff(long nChannel, long nIntensity, enumLED_LAMP_CONTROL nCONTROL)
{
	CString strCommand = XCITE_BB;

	if (nCONTROL == enumLED_LAMP_CONTROL_TURN___ON)
	{
		strCommand = XCITE_BB;
	}
	else
	{
		strCommand = XCITE_SS;
	}

	return strCommand;
}

CString CCommSerialObjectXCITE::MakeCommand_GetTurnOnOff(long nChannel)
{
	// 현재 전체 조명 ON/OFF 상태
	//GetUnitStatus, MakeCommand_GetUnitStatus 사용
	return _T("");
}

bool CCommSerialObjectXCITE::IsLedTurnOnOff(long nChannel, bool bTurnOnOff, CString strVal)
{
	/*
	Unit Status
	Send : uu/r
	Receive : xxx\r” where xxx is the status of the unit.

	The number returned is bitwise and is decoded as follows:
	bit 5 - Lock Bit: 1 = front panel locked, 0 = front panel unlocked;
	bit 4 - Lamp Ready Bit: 1 = lamp is ready, 0 = lamp is not ready;
	bit 3 - Home Bit: 1 = fault, 0 = pass;
	bit 2 - Shutter Bit: 1 = shutter is opened, 0 = shutter is closed;
	bit 1 - Lamp Bit: 1 = lamp is ON, 0 = lamp is OFF;
	bit 0 - Alarm Bit: 1 = alarm is ON, 0 = alarm is OFF.
	*/

	if(strVal.IsEmpty())
		return false;

	bool bLampState	= false;

	if(bTurnOnOff)
	{
		bLampState = (_wtol(strVal) & 0x02) ? true : false;
	}
	else
	{
		bLampState = (_wtol(strVal) & 0x02) ? false : true;
	}
	
	return bLampState;
}

bool CCommSerialObjectXCITE::IsShutterOpenClose(long nChannel, bool bShutterOpenClose, CString strVal)
{
	/*
	Unit Status
	Send : uu/r
	Receive : xxx\r” where xxx is the status of the unit.

	The number returned is bitwise and is decoded as follows:
	bit 5 - Lock Bit: 1 = front panel locked, 0 = front panel unlocked;
	bit 4 - Lamp Ready Bit: 1 = lamp is ready, 0 = lamp is not ready;
	bit 3 - Home Bit: 1 = fault, 0 = pass;
	bit 2 - Shutter Bit: 1 = shutter is opened, 0 = shutter is closed;
	bit 1 - Lamp Bit: 1 = lamp is ON, 0 = lamp is OFF;
	bit 0 - Alarm Bit: 1 = alarm is ON, 0 = alarm is OFF.
	*/

	if (strVal.IsEmpty())
		return false;

	bool bShutterState = false;

	if (bShutterOpenClose)
	{
		bShutterState = (_wtol(strVal) & 0x04) ? true : false;
	}
	else
	{
		bShutterState = (_wtol(strVal) & 0x04) ? false : true;
	}

	return bShutterState;
}

bool CCommSerialObjectXCITE::ConnectDevice(CString &strCommand)
{
	strCommand = MakeCommand_ConnectDevice();

	return SendCommand(strCommand);
}
CString CCommSerialObjectXCITE::MakeCommand_ConnectDevice()
{
	CString strCommand = XCITE_TT;

	return strCommand;
}

bool CCommSerialObjectXCITE::OpenShutter(CString &strCommand)
{
	strCommand = MakeCommand_OpenShutter();

	return SendCommand(strCommand);
}
CString CCommSerialObjectXCITE::MakeCommand_OpenShutter()
{
	CString strCommand = XCITE_MM;

	return strCommand;
}

bool CCommSerialObjectXCITE::CloseShutter(CString &strCommand)
{
	strCommand = MakeCommand_CloseShutter();

	return SendCommand(strCommand);
}

CString CCommSerialObjectXCITE::MakeCommand_CloseShutter()
{
	CString strCommand = XCITE_ZZ;

	return strCommand;
}

bool CCommSerialObjectXCITE::GetUnitStatus(CString &strCommand)
{
	strCommand = MakeCommand_GetUnitStatus();

	return SendCommand(strCommand);
}

CString CCommSerialObjectXCITE::MakeCommand_GetUnitStatus()
{
	CString strCommand = XCITE_UU;

	return strCommand;
}






//----------------------------------------------------------------------------------
// Write 할때, BYTE 형태로 데이터를 작성해야 한다.
//----------------------------------------------------------------------------------
DWORD CCommSerialObjectXCITE::WriteComm(CString strData)
{
    // 포트가 연결되지 않은 상태이면..
    if( IsConnected() == false)
        return 0;

    BYTE WriteBuff[COMM_BUFF_SIZE];
    memset(WriteBuff, 0, COMM_BUFF_SIZE);

    const long nDataCount = strData.GetLength();
    
	for(long i = 0; i < nDataCount; ++i)
	{
		WriteBuff[i] = (BYTE)strData.GetAt(i);
	}

    DWORD	dwWritten, dwError, dwErrorFlags;
    COMSTAT	comstat;

    // 인자로 들어온 버퍼의 내용을 nToWrite 만큼 쓰고.. 쓴 갯수를.,dwWrite 에 넘김.
    if( !WriteFile( m_hComm, WriteBuff, nDataCount, &dwWritten, &m_osWrite))
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

void CCommSerialObjectXCITE::ThreadWorkingLoop()
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