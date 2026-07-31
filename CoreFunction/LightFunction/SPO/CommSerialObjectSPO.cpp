#include "pch.h"
#include "CommSerialObjectSPO.h"

#include "..\..\Comm232Function\Comm232Config.h"
#include "..\..\common\TimeChecker.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCommSerialObjectSPO::CCommSerialObjectSPO(CString strFilePath, CString strCategory, CLogFileAccessor* m_pLogFileAccessor)
{	
	m_pConfig = new CComm232Config(strFilePath, strCategory);

	SetConfig(m_pConfig);

	ZeroMemory(&m_nIntensity, sizeof(m_nIntensity));

	SetLogFileAccessor(m_pLogFileAccessor);
}

CCommSerialObjectSPO::~CCommSerialObjectSPO()
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
BOOL CCommSerialObjectSPO::LinkIniFile(bool bSave)
{
	return TRUE;
}

BOOL CCommSerialObjectSPO::LinkDatFile(bool bSave)
{
	return TRUE;
}

bool CCommSerialObjectSPO::InitDevice(int port)
{
	return OpenPort(port);
}

bool CCommSerialObjectSPO::CloseDevice()
{
	return Close();
}

CString CCommSerialObjectSPO::GetDeviceName()
{
	return _T("SPO");
}

bool CCommSerialObjectSPO::IsConnected()
{
	return Is_Connected();
}


	

//************************************************************************************************
// 조명 실제 제어 관련 함수 - 시퀀스
//************************************************************************************************
bool CCommSerialObjectSPO::LedLight_Channel_TurnOn(long nChannel, long nIntensity)
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
bool CCommSerialObjectSPO::LedLight_Channel_TurnOff(long nChannel, long nIntensity)
{
	bool bRet = true;

	// 조명 밝기를 설정하고 점등한다.
	bRet &= SetLedLampIntensity(nChannel, nIntensity);
	bRet &= SetLedLampTurnOnOff(nChannel, nIntensity, enumLED_LAMP_CONTROL_TURN__OFF);

	// 조명 Intensity 값으로 점등 상태를 확인한다.
	// SPO 는 조명을 완전히 끄지 않고 1값으로 켜놓는다 - 이게 바로 OFF 상태이다.
    bRet &= GetLedLampIntensity(nChannel);
	bRet &= WaitLedLampResponce(nChannel, 1, enumRES_CHECK_TYPE_INTENSITY);

	return bRet;
}





//************************************************************************************************
// Led 조명 제어 전용 함수
//************************************************************************************************
bool CCommSerialObjectSPO::SetLedLampIntensity(long nChannel, long nIntensity, CString &strCommand)
{
	strCommand = MakeCommand_SetIntensity(nChannel, nIntensity);

    return SendCommand(strCommand);
}

bool CCommSerialObjectSPO::GetLedLampIntensity(long nChannel, CString &strCommand)
{
	strCommand = MakeCommand_GetIntensity(nChannel);
	
	return SendCommand(strCommand);
}

bool CCommSerialObjectSPO::SetLedLampTurnOnOff(long nChannel, long nIntensity, enumLED_LAMP_CONTROL nCONTROL, CString &strCommand)
{
	strCommand = MakeCommand_SetTurnOnOff(nChannel, nIntensity, nCONTROL);

	return SendCommand(strCommand);
}

bool CCommSerialObjectSPO::GetLedLampTurnOnOff(long nChannel, CString &strCommand)
{
	strCommand = MakeCommand_GetTurnOnOff(nChannel);	

	return SendCommand(strCommand);
}

bool CCommSerialObjectSPO::WaitLedLampResponce(long nChannel, long nIntensity, enumRES_CHECK_TYPE nTYPE)
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


bool CCommSerialObjectSPO::GetCurLedLampIntensity(long nChannel, long &nIntensity, CString  &strCommand)
{
	nIntensity = m_nIntensity[nChannel];

//	bool bRet = false;
//	CString strRecieveState;
//	RecieveData(strRecieveState);
//
//	bRet = GetLedLampIntensity(nChannel);
//	 
//	if (bRet != true)
//		return false;
//
////	CString strRecieveState;
//
//	CTimeChecker timer;
//
//	while (1)
//	{
//		Sleep(100);
//
//		if (RecieveData(strRecieveState))
//		{
//			if (GetLedIntensity(nChannel, nIntensity, strRecieveState))
//				break;
//		}
//
//		// 통신에서 데이터가 들어올때까지 기다린다. - 제한 시간 30초
//		const float fElapsed_sec = timer.Elapsed_ms() / 1000.f;
//
//		if (1 < fElapsed_sec)
//		{
//			return false;
//		}
//	}
//
	return true;
}























bool CCommSerialObjectSPO::GetLedLampStateCompare(long nChannel, long nIntensity, enumRES_CHECK_TYPE nTYPE)
{
	CString strRecieveState;

	if(RecieveData(strRecieveState))
	{
		if(nTYPE == enumRES_CHECK_TYPE_INTENSITY)	return IsSameLedIntensity(nChannel, nIntensity, strRecieveState);
		if(nTYPE == enumRES_CHECK_TYPE_TURN___ON)	return IsSameLedTurnOnOff(nChannel, true,		strRecieveState);
		if(nTYPE == enumRES_CHECK_TYPE_TURN__OFF)	return IsSameLedTurnOnOff(nChannel, false,		strRecieveState);
		if (nTYPE == enumRES_CHECK_TYPE_CUR_INTENSITY)	return GetLedIntensity(nChannel, nIntensity, strRecieveState);
	}
	
	return false;
}



















//************************************************************************************************
// 명령어 생성하는 부분 - INTENSITY
//************************************************************************************************
// Ⅰ.2. 'D' 예) CHANNEL = 1, DATA = 100 으로 DATA만 변경시
// 0x43 0x31 0x64
// - x : 커맨드 : 'C'(=0x44, CHANGE OUTPUT DATA 변경(OUTPUT ON/OFF 변경없음, 유지됨))
// - n1 : OUTPUT CHANNEL (0x31='1')
// - n2 : OUTPUT CURRENT DATA 값(0x64=100)
CString CCommSerialObjectSPO::MakeCommand_SetIntensity(long nChannel, long nIntensity)
{
	std::vector<BYTE> vecbyteSendData(3, 0x00);

	vecbyteSendData[0] = 0x43;								// C
	if(nChannel == 1)		vecbyteSendData[1] = 0x31;		// 1
	else if(nChannel == 2)	vecbyteSendData[1] = 0x32;		// 2
	else if(nChannel == 3)	vecbyteSendData[1] = 0x33;		// 3
	else if(nChannel == 4)	vecbyteSendData[1] = 0x34;		// 4
	else if(nChannel == 5)	vecbyteSendData[1] = 0x35;		// 5
	else if(nChannel == 6)	vecbyteSendData[1] = 0x36;		// 6
	else if(nChannel == 7)	vecbyteSendData[1] = 0x37;		// 7
	else if(nChannel == 8)	vecbyteSendData[1] = 0x38;		// 8
	else					vecbyteSendData[1] = 0x31;		// 1

	vecbyteSendData[2] = (BYTE)nIntensity;					// Intensity

	CString strCommand;
	for(size_t i = 0; i < vecbyteSendData.size(); ++i)
	{
		CString str;
		str.Format(_T("%c"), vecbyteSendData[i]);

		strCommand += str;
	}

	m_nIntensity[nChannel] = nIntensity;
	 
//	CString str;
//	str.Format(_T("%c%c%d"), vecbyteSendData[0], vecbyteSendData[1], vecbyteSendData[2]);
//	CLogWrapper::GetInstance()->KMLOG(SEND, str);
	
	return strCommand;
}

// Ⅰ.4. 'R' REQUEST : CHANNEL DATA 요청 or ON/OFF(전 채널) 상태 요청
// x n1 n2
// - x : 커맨드 : 'R'(=0x52, REQUEST)
// - n1 : 제어기 DATA 요청시 : 요청 CHANNEL ‘1’ ~’8’
// 제어기 전 채널 ON/OFF 상태 요청시 : : 'O'(=0x4F, ON/OFF)
// - n2 : 제어기 DATA 요청시 : ‘D’(=0x44, DATA)
// 제어기 전 채널 ON/OFF 상태 요청시 : 'N'(=0x4E, ON/OFF)
CString CCommSerialObjectSPO::MakeCommand_GetIntensity(long nChannel)
{
	// 밝기 값 불어옴
	std::vector<BYTE> vecbyteSendData(3, 0x00);
	
	vecbyteSendData[0] = 0x52;				
	if (nChannel == 1)		vecbyteSendData[1] = 0x31;		// 1
	else if (nChannel == 2)	vecbyteSendData[1] = 0x32;		// 2
	else if (nChannel == 3)	vecbyteSendData[1] = 0x33;		// 3
	else if (nChannel == 4)	vecbyteSendData[1] = 0x34;		// 4
	else if (nChannel == 5)	vecbyteSendData[1] = 0x35;		// 5
	else if (nChannel == 6)	vecbyteSendData[1] = 0x36;		// 6
	else if (nChannel == 7)	vecbyteSendData[1] = 0x37;		// 7
	else if (nChannel == 8)	vecbyteSendData[1] = 0x38;		// 8
	else					vecbyteSendData[1] = 0x31;		// 1
															// R
	//if(nChannel == 0)		vecbyteSendData[1] = 0x31;		// 1
	//else if(nChannel == 1)	vecbyteSendData[1] = 0x32;		// 2
	//else if(nChannel == 2)	vecbyteSendData[1] = 0x33;		// 3
	//else if(nChannel == 3)	vecbyteSendData[1] = 0x34;		// 4
	//else if(nChannel == 4)	vecbyteSendData[1] = 0x35;		// 5
	//else if(nChannel == 5)	vecbyteSendData[1] = 0x36;		// 6
	//else if(nChannel == 6)	vecbyteSendData[1] = 0x37;		// 7	
	//else if(nChannel == 7)	vecbyteSendData[1] = 0x38;		// 8	
	vecbyteSendData[2] = 0x44;								// D
	
	CString strCommand;
	for(size_t i = 0; i < vecbyteSendData.size(); ++i)
	{
		CString str;
		str.Format(_T("%c"), vecbyteSendData[i]);

		strCommand += str;
	}

	return strCommand;
}

bool CCommSerialObjectSPO::IsSameLedIntensity(long nChannel, long nIntensity, CString strVal)
{
	CString strData;
	strData.Format(_T("R%d"), nChannel + 1);

	strVal.Replace(strData, _T(""));

	if(strVal.IsEmpty())
		return false;

	//-------------------------------------------------------------
	// convet hex to decimal [ 0 ~ 255 ]
	//-------------------------------------------------------------
	TCHAR Rx_Data = strVal.GetAt(0);		

	CString strRecieveIntensity; 
	strRecieveIntensity.Format(_T("%d"), Rx_Data);
	
	const int nRecieveIntensity = _ttoi(strRecieveIntensity);

	return (nIntensity == nRecieveIntensity) ? true : false;
}

bool CCommSerialObjectSPO::GetLedIntensity(long nChannel, long &nIntensity, CString strVal)
{
	CString strData;
	strData.Format(_T("R%d"), nChannel);

	strVal.Replace(strData, _T(""));

	if (strVal.IsEmpty())
		return false;

	//-------------------------------------------------------------
	// convet hex to decimal [ 0 ~ 255 ]
	//-------------------------------------------------------------
	TCHAR Rx_Data = strVal.GetAt(0);

	CString strRecieveIntensity;
	strRecieveIntensity.Format(_T("%d"), Rx_Data);

	const int nRecieveIntensity = _ttoi(strRecieveIntensity);

	nIntensity = nRecieveIntensity;

	return true;
}



//************************************************************************************************
// 명령어 생성하는 부분 - TURN ON/OFF
//************************************************************************************************
// Ⅰ.3. 커맨드 ‘O’ & 'N'
// => 각 채널 ON or OFF 한다.
// x n1 n2
// - x : 커맨드 : 'O'(=0x4F , ON/OFF)
// - n1 : 커맨드 : 'N'(=0x4E, ON/OFF)
// - n2 : b3(Channel 4 ON/OFF, 1=ON, 0=OFF), b2(Channel 3 ON/OFF, 1=ON, 0=OFF),
// b1(Channel 2 ON/OFF, 1=ON, 0=OFF), b0(Channel 1 ON/OFF, 1=ON, 0=OFF)
// => 예) 1번&3번&4번 CHANNEL ON, 나머지 채널 OFF 하고자 할 경우
// => 예) 1번&3번&4번 CHANNEL ON, 나머지 채널 OFF 하고자 할 경우
// 0x4E 0x46 0x0D
// - x : 커맨드 : 'O'(=0x4F , ON/OFF)
// - n1 : 커맨드 : 'N'(=0x4E, ON/OFF)
// - n2 : CHANNEL 8~5&2 OFF, 1&3&4 ON (0x0D=13=0000 1101)
CString CCommSerialObjectSPO::MakeCommand_SetTurnOnOff(long nChannel, long nIntensity, enumLED_LAMP_CONTROL nCONTROL)
{
	//----------------------------------------------------------------------------
	// 조명 을 끄지 않고 최소로 약한 조명 밝기 [ 1 ]로 변경한다.
	// 이유는?.. 조명을 껏다가 다시 켜는 안정화 시간도 필요하고
	// 조명을 끄고 다시켤때, 현재 컨트롤러에 설정된 조명 상태를 적용해야 함.
	//----------------------------------------------------------------------------
	// Ⅰ.1. 'C' 예) CHANNEL = 1, DATA = 250으로 변경시 및 OUTPUT ON
	// 0x43 0x31 0xFA
	// - x : 커맨드 : 'C'(=0x43, CHANGE OUTPUT DATA 변경 & OUTPUT ON)
	// - n1 : OUTPUT CHANNEL (0x31='1')
	// - n2 : OUTPUT CURRENT DATA 값(0xFA=250) - 조명의 밝기값 (0~255)
	std::vector<BYTE> vecbyteSendData(3, 0x00);

	vecbyteSendData[0] = 0x43;								// C
	//if(nChannel == 0)		vecbyteSendData[1] = 0x31;		// 1
	//else if(nChannel == 1)	vecbyteSendData[1] = 0x32;		// 2
	//else if(nChannel == 2)	vecbyteSendData[1] = 0x33;		// 3
	//else if(nChannel == 3)	vecbyteSendData[1] = 0x34;		// 4
	//else if(nChannel == 4)	vecbyteSendData[1] = 0x35;		// 5
	//else if(nChannel == 5)	vecbyteSendData[1] = 0x36;		// 6
	//else if(nChannel == 6)	vecbyteSendData[1] = 0x37;		// 7
	//else if(nChannel == 7)	vecbyteSendData[1] = 0x38;		// 8

	if (nChannel == 1)		vecbyteSendData[1] = 0x31;		// 1
	else if (nChannel == 2)	vecbyteSendData[1] = 0x32;		// 2
	else if (nChannel == 3)	vecbyteSendData[1] = 0x33;		// 3
	else if (nChannel == 4)	vecbyteSendData[1] = 0x34;		// 4
	else if (nChannel == 5)	vecbyteSendData[1] = 0x35;		// 5
	else if (nChannel == 6)	vecbyteSendData[1] = 0x36;		// 6
	else if (nChannel == 7)	vecbyteSendData[1] = 0x37;		// 7
	else if (nChannel == 8)	vecbyteSendData[1] = 0x38;		// 8
	else					vecbyteSendData[1] = 0x31;		// 1

	if (nIntensity == 0)
		nIntensity = 1;					// Intensity

	vecbyteSendData[2] = (nCONTROL == enumLED_LAMP_CONTROL_TURN___ON) ? (BYTE)nIntensity : 0x01;					// Intensity

	CString strCommand;
	for(size_t i = 0; i < vecbyteSendData.size(); ++i)
	{
		CString str;
		str.Format(_T("%c"), vecbyteSendData[i]);

		strCommand += str;
	}
	
	return strCommand;
}

CString CCommSerialObjectSPO::MakeCommand_GetTurnOnOff(long nChannel)
{
	// 현재 전체 조명 ON/OFF 상태
	CString strCommand = _T("RNF");

	return strCommand;
}

bool CCommSerialObjectSPO::IsSameLedTurnOnOff(long nChannel, bool bTurnOnOff, CString strVal)
{
	CString strData = _T("ON");
	strVal.Replace(strData, _T(""));
	
	if(strVal.IsEmpty())
		return false;

	//-------------------------------------------------------------
	// convet hex to decimal [ 0 ~ 255 ]
	//-------------------------------------------------------------
	TCHAR Rx_Data = strVal.GetAt(0);		

	bool bLampState	= false;

	if(bTurnOnOff)
	{
		//if(nChannel == 0)		bLampState = (Rx_Data & 0x01) ? true : false;		// 1
		//else if(nChannel == 1)	bLampState = (Rx_Data & 0x02) ? true : false;		// 2
		//else if(nChannel == 2)	bLampState = (Rx_Data & 0x04) ? true : false;		// 3
		//else if(nChannel == 3)	bLampState = (Rx_Data & 0x08) ? true : false;		// 4
		//else if(nChannel == 4)	bLampState = (Rx_Data & 0x10) ? true : false;		// 5
		//else if(nChannel == 5)	bLampState = (Rx_Data & 0x20) ? true : false;		// 6
		//else if(nChannel == 6)	bLampState = (Rx_Data & 0x40) ? true : false;		// 7	
		//else if(nChannel == 7)	bLampState = (Rx_Data & 0x80) ? true : false;		// 8	

		if (nChannel == 1)		bLampState = (Rx_Data & 0x01) ? true : false;		// 1
		else if (nChannel == 2)	bLampState = (Rx_Data & 0x02) ? true : false;		// 2
		else if (nChannel == 3)	bLampState = (Rx_Data & 0x04) ? true : false;		// 3
		else if (nChannel == 4)	bLampState = (Rx_Data & 0x08) ? true : false;		// 4
		else if (nChannel == 5)	bLampState = (Rx_Data & 0x10) ? true : false;		// 5
		else if (nChannel == 6)	bLampState = (Rx_Data & 0x20) ? true : false;		// 6
		else if (nChannel == 7)	bLampState = (Rx_Data & 0x40) ? true : false;		// 7	
		else if (nChannel == 8)	bLampState = (Rx_Data & 0x80) ? true : false;		// 8
		else                    bLampState = (Rx_Data & 0x01) ? true : false;		// 1
	}
	else
	{
		//if(nChannel == 0)		bLampState = (Rx_Data & 0x01) ? false: true;		// 1
		//else if(nChannel == 1)	bLampState = (Rx_Data & 0x02) ? false: true;		// 2
		//else if(nChannel == 2)	bLampState = (Rx_Data & 0x04) ? false: true;		// 3
		//else if(nChannel == 3)	bLampState = (Rx_Data & 0x08) ? false: true;		// 4
		//else if(nChannel == 4)	bLampState = (Rx_Data & 0x10) ? false: true;		// 5
		//else if(nChannel == 5)	bLampState = (Rx_Data & 0x20) ? false: true;		// 6
		//else if(nChannel == 6)	bLampState = (Rx_Data & 0x40) ? false: true;		// 7	
		//else if(nChannel == 7)	bLampState = (Rx_Data & 0x80) ? false: true;		// 8	

		if (nChannel == 1)		bLampState = (Rx_Data & 0x01) ? false : true;		// 1
		else if (nChannel == 2)	bLampState = (Rx_Data & 0x02) ? false : true;		// 2
		else if (nChannel == 3)	bLampState = (Rx_Data & 0x04) ? false : true;		// 3
		else if (nChannel == 4)	bLampState = (Rx_Data & 0x08) ? false : true;		// 4
		else if (nChannel == 5)	bLampState = (Rx_Data & 0x10) ? false : true;		// 5
		else if (nChannel == 6)	bLampState = (Rx_Data & 0x20) ? false : true;		// 6
		else if (nChannel == 7)	bLampState = (Rx_Data & 0x40) ? false : true;		// 7	
		else if (nChannel == 8)	bLampState = (Rx_Data & 0x80) ? false : true;		// 8
		else                    bLampState = (Rx_Data & 0x01) ? false : true;		// 1
	}

	return bLampState;
}











//----------------------------------------------------------------------------------
// Write 할때, BYTE 형태로 데이터를 작성해야 한다.
//----------------------------------------------------------------------------------
DWORD CCommSerialObjectSPO::WriteComm(CString strData)
{
    // 포트가 연결되지 않은 상태이면..
    if( IsConnected() == false)
        return 0;

    BYTE WriteBuff[COMM_BUFF_SIZE];
    memset(WriteBuff, 0, COMM_BUFF_SIZE);

    long nDataCount = strData.GetLength();
    
	for(long i = 0; i < nDataCount; ++i)
	{
		WriteBuff[i] = (BYTE)strData.GetAt(i);
	}

	if (nDataCount == 2)
	{
		nDataCount = 3;
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

void CCommSerialObjectSPO::ThreadWorkingLoop()
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