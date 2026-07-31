#include "pch.h"
#include "CommSerialObjectCAS.h"

#include "..\..\Comm232Function\Comm232Config.h"
#include "..\..\common\TimeChecker.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCommSerialObjectCAS::CCommSerialObjectCAS(CString strFilePath, CString strCategory, CLogFileAccessor* m_pLogFileAccessor)
{	
	m_pConfig = new CComm232Config(strFilePath, strCategory);

	SetConfig(m_pConfig);

	SetLogFileAccessor(m_pLogFileAccessor);
}

CCommSerialObjectCAS::~CCommSerialObjectCAS()
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
BOOL CCommSerialObjectCAS::LinkIniFile(bool bSave)
{
	return TRUE;
}

BOOL CCommSerialObjectCAS::LinkDatFile(bool bSave)
{
	return TRUE;
}

bool CCommSerialObjectCAS::InitDevice(int port)
{
	return OpenPort(port);
}

bool CCommSerialObjectCAS::CloseDevice()
{
	return Close();
}

CString CCommSerialObjectCAS::GetDeviceName()
{
	return _T("CAS");
}

bool CCommSerialObjectCAS::IsConnected()
{
	return Is_Connected();
}

//************************************************************************************************
// 조명 실제 제어 관련 함수 - 시퀀스
//************************************************************************************************
bool CCommSerialObjectCAS::LedLight_Channel_TurnOn(long nChannel, long nIntensity)
{
	bool bRet = true;

	// 조명 밝기를 설정하고 점등한다.
	bRet &= SetLedLampIntensity(nChannel, nIntensity);
	bRet &= SetLedLampTurnOnOff(nChannel, nIntensity, enumLED_LAMP_CONTROL_TURN___ON);

	// 점등 후, 유지 시간을 적용해준다.
	m_pConfig->GetParam_SerialCommHoldTime();

	// 조명 Intensity 값으로 점등 상태를 확인한다.
	bRet &= GetLedLampIntensity(nChannel);
	bRet &= WaitLedLampResponce(nChannel, nIntensity, enumRES_CHECK_TYPE_INTENSITY);

	//-------------------------------------------------------------------------------------------
	// ----------------------- SDC 에서 검증한 내용 ------------------------
	//-------------------------------------------------------------------------------------------
	// UV는 딜레이 두개 쓰는것같은데요 1번헤드 조명제어후 2번헤드 조명제어전에는 200ms쓰는것 같고
	// 2번헤드 조명제어후 UV광원 켜지기까지 안정화 시간 100ms쓰는것같습니다.
	Sleep(200);
	//-------------------------------------------------------------------------------------------

	return bRet;
}
bool CCommSerialObjectCAS::LedLight_Channel_TurnOff(long nChannel, long nIntensity)
{
	bool bRet = true;

	// 조명 밝기를 설정하고 점등한다.
	bRet &= SetLedLampIntensity(nChannel, nIntensity);
	bRet &= SetLedLampTurnOnOff(nChannel, nIntensity, enumLED_LAMP_CONTROL_TURN__OFF);

	// 조명 ON/OFF 값으로 점등 상태를 확인한다.
	bRet &= GetLedLampTurnOnOff(nChannel);
	bRet &= WaitLedLampResponce(nChannel, nIntensity, enumRES_CHECK_TYPE_TURN__OFF);

	return bRet;
}






//************************************************************************************************
// Led 조명 제어 전용 함수
//************************************************************************************************
bool CCommSerialObjectCAS::SetLedLampIntensity(long nChannel, long nIntensity, CString &strCommand)
{
	strCommand = MakeCommand_SetIntensity(nChannel, nIntensity);	 

    return SendCommand(strCommand);
}

bool CCommSerialObjectCAS::GetLedLampIntensity(long nChannel, CString &strCommand)
{
	strCommand = MakeCommand_GetIntensity(nChannel);
	
    return SendCommand(strCommand);
}

bool CCommSerialObjectCAS::SetLedLampTurnOnOff(long nChannel, long nIntensity, enumLED_LAMP_CONTROL nCONTROL, CString &strCommand)
{
	strCommand = MakeCommand_SetTurnOnOff(nChannel, nIntensity, nCONTROL);

    return SendCommand(strCommand);
}

bool CCommSerialObjectCAS::GetLedLampTurnOnOff(long nChannel, CString &strCommand)
{
	strCommand = MakeCommand_GetTurnOnOff(nChannel);	

    return SendCommand(strCommand);
}

bool CCommSerialObjectCAS::WaitLedLampResponce(long nChannel, long nIntensity, enumRES_CHECK_TYPE nTYPE)
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







//----------------------------------------------------------------------------------
// Write 할때, char 형태로 데이터를 작성해야 한다.
//----------------------------------------------------------------------------------
DWORD CCommSerialObjectCAS::WriteComm(CString strData)
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














bool CCommSerialObjectCAS::GetLedLampStateCompare(long nChannel, long nIntensity, enumRES_CHECK_TYPE nTYPE)
{
	CString strRecieveState;

	if(RecieveData(strRecieveState))
	{
		if(nTYPE == enumRES_CHECK_TYPE_INTENSITY)	return IsSameLedIntensity(nChannel, nIntensity, strRecieveState);
		if(nTYPE == enumRES_CHECK_TYPE_TURN___ON)	return IsSameLedTurnOnOff(nChannel, true,		strRecieveState);
		if(nTYPE == enumRES_CHECK_TYPE_TURN__OFF)	return IsSameLedTurnOnOff(nChannel, false,		strRecieveState);
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
CString CCommSerialObjectCAS::MakeCommand_SetIntensity(long nChannel, long nIntensity)
{
	// 밝기 분포를 [ 0 ~ 256 : 0 ~ 1024 >>> 확대 시킴 ]
	//const double dScalingRate	= (double)nIntensity / 255.0;
	//const long nScaledIntensity = (long)(dScalingRate * 1024.0);

	long iCh = 0;
	switch (nChannel)
	{
	case 1:
		iCh = 0;
		break;
	case 2:
		iCh = 1;
		break;
	case 3:
		iCh = 2;
		break;
	case 4:
		iCh = 3;
		break;
	case 5:
		iCh = 4;
		break;
	case 6:
		iCh = 5;
		break;
	case 7:
		iCh = 6;
		break;
	case 8:
		iCh = 7;
		break;
	default:
		break;
	}

	// 	밝기 제어
	CString strCommand;
	//strCommand.Format(_T("setbrightness %d %d"), nChannel, nIntensity);
	strCommand.Format(_T("# %d %d"), iCh, nIntensity);

	return strCommand;
}

// "getbrightness [ch]\r"
//		- 선택 된 채널의 밝기 값을 불러 옴.
//		- [ch] : 밝기값을 읽을 채널 (4채널 : 0~3) (8채널 : 0~7)
// "$ [ch]\r"
//		- getbrightness와 동일한 기능을 함.
CString CCommSerialObjectCAS::MakeCommand_GetIntensity(long nChannel)
{
	// 밝기 값 불어옴
	CString strCommand;
	//strCommand.Format(_T("getbright %d"), nChannel);

	long iCh = 0;
	switch (nChannel)
	{
	case 1:
		iCh = 0;
		break;
	case 2:
		iCh = 1;
		break;
	case 3:
		iCh = 2;
		break;
	case 4:
		iCh = 3;
		break;
	case 5:
		iCh = 4;
		break;
	case 6:
		iCh = 5;
		break;
	case 7:
		iCh = 6;
		break;
	case 8:
		iCh = 7;
		break;
	default:
		break;
	}

	strCommand.Format(_T("$$ %d"), iCh);

	return strCommand;
}

bool CCommSerialObjectCAS::IsSameLedIntensity(long nChannel, long nIntensity, CString strVal)
{
	CString strData;

	long iCh = 0;
	switch (nChannel)
	{
	case 1:
		iCh = 0;
		break;
	case 2:
		iCh = 1;
		break;
	case 3:
		iCh = 2;
		break;
	case 4:
		iCh = 3;
		break;
	case 5:
		iCh = 4;
		break;
	case 6:
		iCh = 5;
		break;
	case 7:
		iCh = 6;
		break;
	case 8:
		iCh = 7;
		break;
	default:
		break;
	}


	strData.Format(_T("$$ %d "), iCh);
	
	strVal.Replace(strData, _T(""));

	const int nIntensityValue = _ttoi(strVal);

	return (nIntensity == nIntensityValue) ? true : false;
}

//************************************************************************************************
// 명령어 생성하는 부분 - TURN ON/OFF
//************************************************************************************************
CString CCommSerialObjectCAS::MakeCommand_SetTurnOnOff(long nChannel, long nIntensity, enumLED_LAMP_CONTROL nCONTROL)
{
	// 	조명 ON
	CString strCommand;

	long iCh = 0;
	switch (nChannel)
	{
	case 1:
		iCh = 0;
		break;
	case 2:
		iCh = 1;
		break;
	case 3:
		iCh = 2;
		break;
	case 4:
		iCh = 3;
		break;
	case 5:
		iCh = 4;
		break;
	case 6:
		iCh = 5;
		break;
	case 7:
		iCh = 6;
		break;
	case 8:
		iCh = 7;
		break;
	default:
		break;
	}

	if(nCONTROL == enumLED_LAMP_CONTROL_TURN___ON)	strCommand.Format(_T("setonoff %d 1"), iCh);
	else											strCommand.Format(_T("setonoff %d 0"), iCh);

	return strCommand;
}

CString CCommSerialObjectCAS::MakeCommand_GetTurnOnOff(long nChannel)
{
	long iCh = 0;
	switch (nChannel)
	{
	case 1:
		iCh = 0;
		break;
	case 2:
		iCh = 1;
		break;
	case 3:
		iCh = 2;
		break;
	case 4:
		iCh = 3;
		break;
	case 5:
		iCh = 4;
		break;
	case 6:
		iCh = 5;
		break;
	case 7:
		iCh = 6;
		break;
	case 8:
		iCh = 7;
		break;
	default:
		break;
	}

	// 	조명 상태 체크
	CString strCommand;
	strCommand.Format(_T("getonoff %d"), iCh);

	return strCommand;
}

bool CCommSerialObjectCAS::IsSameLedTurnOnOff(long nChannel, bool bTurnOnOff, CString strVal)
{
	long iCh = 0;
	switch (nChannel)
	{
	case 1:
		iCh = 0;
		break;
	case 2:
		iCh = 1;
		break;
	case 3:
		iCh = 2;
		break;
	case 4:
		iCh = 3;
		break;
	case 5:
		iCh = 4;
		break;
	case 6:
		iCh = 5;
		break;
	case 7:
		iCh = 6;
		break;
	case 8:
		iCh = 7;
		break;
	default:
		break;
	}

	CString strData;
	strData.Format(_T("getonoff %d "), iCh);

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
