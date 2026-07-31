// DlgESD.cpp : implementation file
//

#include "stdafx.h"
#include "FFUMon.h"
#include "DlgESD.h"
#include "FFUMonDlg.h"
#include "crc16.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
// CDlgESD dialog


CDlgESD::CDlgESD(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgESD::IDD, pParent)
	, m_nType(0)
	, m_nSerialPort(1)
	, m_nNum(0)
	, m_nNGCount(0)
{
	//{{AFX_DATA_INIT(CDlgESD)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_bReceiveOut = FALSE;
	m_nSocketIndex = 1;

	m_strIP.Empty();
	m_strPort.Empty();

	m_pRcvThread = NULL;
	m_hEvtRcv     = CreateEvent(NULL, FALSE,  FALSE, NULL);  // auto reset,   initially set
	m_hEvtRcvStop = CreateEvent(NULL, FALSE, FALSE, NULL);
}


void CDlgESD::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgESD)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgESD, CDialog)
	//{{AFX_MSG_MAP(CDlgESD)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgESD message handlers


void CDlgESD::SetMainPoint(void* pvoid)
{
	m_pdlgMain = (CFFUMonDlg*)pvoid;
	// 	// sih 20150120 add 
	// 	for(int i=0; i<5; i++)
	// 		g_pdlgMainPLC->m_ffu.dTemper[i] = 0.0;
	// 	//ksk 20150123 add
	// 	for(i=0; i<16; i++)
	// 	//	g_pdlgMainPLC->m_ffu.dEFU[i] = 0.0;	//ksk 20151228 change
	// 	g_pdlgMainPLC->m_ffu.dEFU[i] = 800.0;
	// 
	// 	g_pdlgMainPLC->m_ffu.dH2O= 0.0;
	// 	g_pdlgMainPLC->m_ffu.dO2pct= 0.0;
	// 	g_pdlgMainPLC->m_ffu.dO2ppm= 0.0; 
	// 	g_pdlgMainPLC->m_ffu.dPressure= 0.0;
	// 	g_pdlgMainPLC->m_ffu.dPressure= 0.0;
	//
}

BOOL CDlgESD::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	for(int i = 0; i < 4; i++)
	{
		m_dStaticVolt[i] = -1.0;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

int CDlgESD::SocketConnectESD(CString strIP, CString strPort, int index/* = 1*/)
{
	int nRtn =0;

	SOCKET_OPEN stSockOpen;
	stSockOpen.nSockMode = SOCKET_MODE_CLIENT;
	stSockOpen.nPort = _ttoi(strPort);
//	strcpy(stSockOpen.szIPAddr, strIP);
//	strcpy_s(stSockOpen.szIPAddr, strIP);	// mn.bong 20180709 change strcpy -> strcpy_s
	_tcscpy_s(stSockOpen.szIPAddr, _countof(stSockOpen.szIPAddr), strIP);
	 	
 	CreateSocket(index, stSockOpen);

	//Set Refresh Time 
	nRtn = SendToDeviceWrite(0x00, 0x00, 0x00, 0x0A);		//0A -> 1Second

	if(!nRtn)
		return 0;

	//Set Channel Enable
	nRtn = SendToDeviceWrite(0x00, 0x01, 0x00, 0x0F);		//Enable (1, 2, ,3, 4 Channel)

	if(!nRtn)
		return 0;
	
	//Set Mode (Voltage or Current)					1 -> Current, 0 -> Voltage
	//SendToDeviceWrite(0x00, 0x01, 0x00, 0xFF);		//Set Voltage Mode  (1, 2, ,3, 4 Channel)

	nRtn = SendToESD();

	if(!nRtn)
		return 0;

	return nRtn;
}

int CDlgESD::SocketDisConnectESD(int index)
{
	int nRtn =0;

	SOCKET_OPEN stSockOpen;
	ZeroMemory(&stSockOpen, sizeof(stSockOpen));

	CreateSocket(index, stSockOpen, FALSE);

	return nRtn;
}

void CDlgESD::CreateSocket(int index, SOCKET_OPEN stSockOpen, BOOL bConnect)
{
	CString str;

	if( bConnect )
	{
		m_nSocketIndex = index;

		memcpy(&m_stSockOpen, &stSockOpen, sizeof(SOCKET_OPEN));

		BOOL bInitialize = FALSE;
		int nSockMode = stSockOpen.nSockMode;
		int nPort = stSockOpen.nPort;
		CString strIPAddr = stSockOpen.szIPAddr;

		if(api_SocketCtrl_IsConnected(index))
		{
			// 현재 연결되었으면, 연결 해제.
			api_SocketCtrl_Close(index);

			CString strLog;
			strLog.Format("[CLOSED] by user, %d", index);
			TRACE("%s\n", strLog);
			//			EventLog(strLog);

		}

		TCHAR szIp[32];
		_tcscpy_s(szIp, _countof(szIp), strIPAddr);
		bInitialize = api_SocketCtrl_Initialize(index, this, nSockMode, nPort, szIp);			// jsyoon 20180913 change
//			bInitialize = api_SocketCtrl_Initialize(m_nSocketIndex, this, nSockMode, nPort, strIPAddr);
		if(bInitialize)
		{
			CString strLog;
			strLog.Format("[CONNECTED] (%s)", strIPAddr);
			//g_pDlgMainView->m_stcWritePLC.SetWindowText(_T("Connected"));
			//					EventLog(strLog);
		}
		else
		{
			CString strLog = "Create Socket...Fail!!!";
			//g_pDlgMainView->m_stcWritePLC.SetWindowText(_T("Failed"));
			//AfxMessageBox(strLog);
			//					return;
		}
	}
	else
	{
		api_SocketCtrl_Close(index);
	}
}

int CDlgESD::SendToESD()
{
	int nRtn= 0;

	if( m_nType == 0)
	{
		if( !api_SocketCtrl_IsConnected(m_nSocketIndex) )
			return FALSE;

		//Request
		nRtn = SendToDeviceRequest(0x00, 0x00, 0x00, 16);		//16 -> (1,2,3,4 Channel Enable)
		if(!nRtn)
			return 0;

		nRtn = GetReceiveDataESD();
	}
	else
	{
		BOOL bRet = FALSE;
		int  i = 0, res = 0;
		CString strLog, strMsg;
		unsigned char data[8];// = {0x02, 0x04, 0x00, 0x00, 0x00, 0x02}; 
		unsigned short crc16;

		data[0] = 0x01;		// 유닛 아이디  
		data[1] = 0x04;     // 함수 코드  
		data[2] = 0x00;     // 함수 코드에 따른 데이터  
		data[3] = 0x00; 
		data[4] = 0x00; 
		data[5] = 0x10; 

		crc16 = CRC16_MODBUS(data, 6); 
		data[6] = (unsigned char)((crc16>>8) & 0x00ff);  //CRC 
		data[7] = (unsigned char)((crc16>>0) & 0x00ff); 

		for(i=0; i<8; i++)
		{
			strLog.Format(_T("%02X "), data[i] ); 
			strMsg += strLog;
		}

		if( api_SerialCom_IsOpened(m_nSerialPort))
		{
			// jsyoon 20181116 add
			api_SerialCom_ClearReceiveBuffer(m_nSerialPort);
			//////////////////////////////////////////////////////////////////////////

			bRet = api_SerialCom_WriteByte(m_nSerialPort, data, 8);

			strLog.Format("SEND(%d,%d) : %s", m_nSerialPort,bRet, strMsg);

			m_pdlgMain->WriteLogList(strLog, m_nNum);

			if(bRet)
			{
				res = WaitForSingleObject(m_hEvtRcv, 5000);	
				if( res == WAIT_TIMEOUT)
				{
					m_nNGCount++;
				}
				else
				{
					m_nNGCount = 0;

					// receive
				}
			}
			else
			{
				m_nNGCount++;
			}

			if( m_nNGCount > 5)
			{
				for( i = 0; i < 4; i++)
				{
					m_dStaticVolt[i] = -1.0;
				}

				m_nNGCount = 0;
			}
		}
		else
		{
			strLog.Format("SEND(%d,%d) : %s", m_nSerialPort, bRet, strMsg);

			m_pdlgMain->WriteLogList(strLog, m_nNum);

			for( i = 0; i < 4; i++)
			{
				m_dStaticVolt[i] = -1.0;
			}
		}	
	}

	return nRtn;
}

BOOL CDlgESD::SendToDeviceWrite(BYTE btAddress1, BYTE btAddress2, BYTE Data1, BYTE Data2)
{
	int nRtn =0;
	if( api_SocketCtrl_IsConnected(m_nSocketIndex))
	{
		char* pSendData = new char[J_PROTOCOL_WRITE_SIZE];
		
		//********** REQUEST HEAD PROTOCOL *******************
		pSendData[0] = 0x00;
		pSendData[1] = 0x00;
		pSendData[2] = 0x00;
		pSendData[3] = 0x00;
		
		pSendData[4] = 0x00;						//After DataLength (2Bytes)
		pSendData[5] = 8;
		
		pSendData[6] = 1;							//Unit ID
		
		pSendData[7] = J_PROTOCOL_WRITE;			//Function Code
		//********** REQUEST HEAD PROTOCOL END ***************
		
		//********** Data ************************************
		pSendData[8] = btAddress1;			//Address		   (2Bytes)
		pSendData[9] = btAddress2;
		
		//After DataLength (2Bytes)
		pSendData[10] = 0x00;			//Response Data Size (Word Count)
		pSendData[11] = 0x01;			//Response Data Size (Word Count)
		
		pSendData[12] = Data1;			//Data (1Bytes)
		pSendData[13] = Data2;			//Data (1Bytes)
		//********  After Data Length (pSendData[5]) Bytes END **********

		CString strLog, strMsg;
		for(int i = 0; i<J_PROTOCOL_WRITE_SIZE; i++)
		{
			strLog.Format(_T("%02X "), pSendData[i] ); 
			strMsg += strLog;
		}
		strLog.Format(_T("Send: %s (%d)"), strMsg, J_PROTOCOL_WRITE_SIZE);
		m_pdlgMain->WriteLogList(strLog, m_nNum);
		
		nRtn = api_SocketCtrl_SendData(m_nSocketIndex, (BYTE*)pSendData, J_PROTOCOL_WRITE_SIZE);

		SAFE_DELETE_AR(pSendData);		// jsyoon 20181112 add

		if(!nRtn)
			return 0;

		nRtn = GetReceiveDataDUMY();
	}

	return nRtn;
}


BOOL CDlgESD::SendToDeviceRequest(BYTE btAddress1, BYTE btAddress2, BYTE btLength1, BYTE btLength2)
{
	int nRtn = 0;
	if( api_SocketCtrl_IsConnected(m_nSocketIndex))
	{
		char* pSendData = new char[J_PROTOCOL_REQUEST_SIZE];
		
		//********** REQUEST HEAD PROTOCOL *******************
		pSendData[0] = 0x00;
		pSendData[1] = 0x00;
		pSendData[2] = 0x00;
		pSendData[3] = 0x00;
		
		pSendData[4] = 0x00;						//After DataLength (2Bytes)
		pSendData[5] = 6;
		
		pSendData[6] = 1;							//Unit ID
		
		pSendData[7] = J_PROTOCOL_REQUEST;			//Function Code
		//********** REQUEST HEAD PROTOCOL END ***************
		
		//********** Data ************************************
		pSendData[8] = btAddress1;				//Address		   (2Bytes)
		pSendData[9] = btAddress2;
		
		//After DataLength (2Bytes)
		pSendData[10] = btLength1;				//Response Data Size (Word Count)
		pSendData[11] = btLength2;				//Response Data Size (Word Count)
		//********  After Data Length (pSendData[5]) Bytes END **********

		CString strLog, strMsg;
		for(int i = 0; i<J_PROTOCOL_REQUEST_SIZE; i++)
		{
			strLog.Format(_T("%02X "), pSendData[i] ); 
			strMsg += strLog;
		}
		strLog.Format(_T("Send: %s (%d)"), strMsg, J_PROTOCOL_REQUEST_SIZE);
		m_pdlgMain->WriteLogList(strLog, m_nNum);
		
		nRtn =api_SocketCtrl_SendData(m_nSocketIndex, (BYTE*)pSendData, J_PROTOCOL_REQUEST_SIZE);

		SAFE_DELETE_AR(pSendData);		// jsyoon 20181112 add
	}

	return nRtn;
}

LRESULT CDlgESD::OnSocketMsgReceive(WPARAM wParam, LPARAM lParam)
{
	wParam;
	lParam;

//	UINT nSize = (UINT)lParam;
	
	return 0;
}

int CDlgESD::GetReceiveDataESD()
{
	DWORD  dwS, dwR, dwT=0;
	CString strMsg;
	CString strTmp;
	CString strLog;
	strTmp.Empty();
	
	for (dwS=GetTickCount(); dwT<3000;)  
	{
		if (m_bReceiveOut)
		{
			return TRUE;
		}
		
		int nLen = api_SocketCtrl_GetReceiveDataSize(m_nSocketIndex);
		if(nLen>0)
		{
		
			BYTE* pByRecv = new BYTE[nLen + 1];		
			//int nCmdStart= 0, nCmdEnd = 0;		
			ZeroMemory(pByRecv, nLen+1);	
			api_SocketCtrl_GetReceiveData(m_nSocketIndex, pByRecv, nLen);    //수신한 메세지 사이즈만큼 수신한 메세지를 가져온다. 
			
			for(int i = 0; i<nLen; i++)
			{
				strTmp.Format(_T("%02X "), pByRecv[i] ); 
				strMsg += strTmp;
			}
			strLog.Format(_T("RCV: %s (%d)"), strMsg, nLen);
			m_pdlgMain->WriteLogList(strLog, m_nNum);
					
			CString strRcv = pByRecv;
						
//			BYTE* pCmdByte;
//			pCmdByte = new BYTE[strRcv.GetLength() + 1];
//			memcpy(pCmdByte,(LPCTSTR)strRcv,strRcv.GetLength());
//			pCmdByte[strRcv.GetLength()] = 0x00;
			
			//AnalysisReceiveData(strRcv.GetLength(),pCmdByte);
			AnalysisReceiveData(nLen,pByRecv);
			
//			delete pCmdByte;
//			SAFE_DELETE(pByRecv);
			SAFE_DELETE_AR(pByRecv);
			
			return TRUE;
		}
		
		Sleep(10);
		dwR = GetTickCount();
		
        if (dwR >= dwS)  
			dwT = dwR - dwS;
        else
			dwT = ((DWORD)0xFFFFFFFF) - (dwS - dwR);
	}
	
	strLog.Format("RCV: Timeout...");
	m_pdlgMain->WriteLogList(strLog, m_nNum);
		
	return FALSE;
}

void CDlgESD::AnalysisReceiveData(UINT nSize, BYTE *szBuffer2)
{
	static char szBuffer[1024];
	::ZeroMemory(szBuffer, sizeof(szBuffer));
	memcpy(szBuffer,szBuffer2,nSize);

	char *pRaw = new char[nSize + 1];
	pRaw[nSize] = '\0';										//데이터의 끝을 알려줌
	
	static CString sHead;

	if( m_nType == 0)
	{
		static SDAS_HEAD_DATA Head;					//Head -> 8Bytes
		::ZeroMemory(&Head, sizeof(SDAS_HEAD_DATA));
// 		static short nDataLength;					//DataLength -> 2Bytes
// 		static CString sData;

		//Received Data
		//************		UnCapsulation Head		***********
		memcpy(&Head.btTransaction, szBuffer, sizeof(BYTE) * 2);				//Get Head Data - Transaction
		memcpy(&Head.btProtocol, szBuffer + 2, sizeof(BYTE) * 2);				//Get Head Data - Protocol
		memcpy(&Head.btLength, szBuffer + 4, sizeof(BYTE) * 2);					//Get Head Data - Frame Length
		memcpy(&Head.btUnitID, szBuffer + 6, sizeof(BYTE));						//Get Head Data - Unit ID
		memcpy(&Head.btFC, szBuffer + 7, sizeof(BYTE));							//Get Head Data - Function Code
	
		sHead.Format(_T("%02d %02d %02d %02d %02d %02d %02d %02d"), Head.btTransaction[0], Head.btTransaction[1], Head.btProtocol[0], Head.btProtocol[1], Head.btLength[0], Head.btLength[1], Head.btUnitID, Head.btFC);
		//******************************************************

		memcpy(pRaw, szBuffer + 8, nSize);
	}
	else
	{
		static RS232C_RESPONSE_DATA Head;
		::ZeroMemory(&Head, sizeof(SDAS_HEAD_DATA));

		//Received Data
		//************		UnCapsulation Head		***********
		memcpy(&Head.btUnitID, szBuffer,     sizeof(BYTE));						//Get Head Data - Unit ID
		memcpy(&Head.btFC,     szBuffer + 1, sizeof(BYTE));						//Get Head Data - Function Code
		memcpy(&Head.btLength, szBuffer + 3, sizeof(BYTE) * 2);					//Get Head Data - Frame Length
	
		sHead.Format(_T("%02d %02d %02d %02d"), Head.btUnitID, Head.btFC, Head.btLength[0], Head.btLength[1]);
		//******************************************************

		memcpy(pRaw, szBuffer + 4, nSize);
	}

	//************		Get Raw Data	*******************
	//Core Function!!!
	AddListItem(sHead, pRaw);
	//******************************************************
	
	::ZeroMemory(szBuffer, sizeof(szBuffer));
	
	delete [] pRaw;
}

void CDlgESD::AddListItem(CString sHead, char* pRawData)
{
	RawData	rawData[J_CHANNEL_MAX];
	
	CString sTemp = _T("0");
	int nTemp = 0;

	CString sCurrentTime = _T("");
	SYSTEMTIME systemTime;
	GetLocalTime(&systemTime);
	
	sCurrentTime.Format(_T("[%04d%02d%02d%02d%02d%02d]"), systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour, systemTime.wMinute, systemTime.wSecond);
	
	int nLength = pRawData[0];				//데이터의 실제 길이

	int nCurrent[J_CHANNEL_MAX];

	if(nLength <= 0)			//Data 없음
	{
		TRACE(_T("\n No recv Data"));
	}
	else if(nLength <= 8)		//Chaanel 1의 데이터만 날라옴
	{
		memcpy(&rawData[J_CHANNEL_1], pRawData + 1, sizeof(BYTE) * 8);

		ByteSwap(&rawData[J_CHANNEL_1].btCurrent[0], &rawData[J_CHANNEL_1].btCurrent[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_1].btCurrent, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
		nCurrent[J_CHANNEL_1] = _ttoi(sTemp);
		
		ByteSwap(&rawData[J_CHANNEL_1].btAverage[0], &rawData[J_CHANNEL_1].btAverage[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_1].btAverage, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
	

		ByteSwap(&rawData[J_CHANNEL_1].btMin[0], &rawData[J_CHANNEL_1].btMin[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_1].btMin, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
	

		ByteSwap(&rawData[J_CHANNEL_1].btMax[0], &rawData[J_CHANNEL_1].btMax[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_1].btMax, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
	
	}
	else if(nLength <= 16)		//Chaanel 1, 2의 데이터가 날라옴
	{
		memcpy(&rawData[J_CHANNEL_1], pRawData + 1, sizeof(BYTE) * 8);

		ByteSwap(&rawData[J_CHANNEL_1].btCurrent[0], &rawData[J_CHANNEL_1].btCurrent[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_1].btCurrent, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
		nCurrent[J_CHANNEL_1] = _ttoi(sTemp);
		
		ByteSwap(&rawData[J_CHANNEL_1].btAverage[0], &rawData[J_CHANNEL_1].btAverage[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_1].btAverage, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
		
		ByteSwap(&rawData[J_CHANNEL_1].btMin[0], &rawData[J_CHANNEL_1].btMin[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_1].btMin, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
	
		ByteSwap(&rawData[J_CHANNEL_1].btMax[0], &rawData[J_CHANNEL_1].btMax[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_1].btMax, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
	
		memcpy(&rawData[J_CHANNEL_2], pRawData + 9, sizeof(BYTE) * 8);

		ByteSwap(&rawData[J_CHANNEL_2].btCurrent[0], &rawData[J_CHANNEL_2].btCurrent[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_2].btCurrent, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
		nCurrent[J_CHANNEL_2] = _ttoi(sTemp);
	
		ByteSwap(&rawData[J_CHANNEL_2].btAverage[0], &rawData[J_CHANNEL_2].btAverage[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_2].btAverage, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
		
		ByteSwap(&rawData[J_CHANNEL_2].btMin[0], &rawData[J_CHANNEL_2].btMin[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_2].btMin, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
		

		ByteSwap(&rawData[J_CHANNEL_2].btMax[0], &rawData[J_CHANNEL_2].btMax[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_2].btMax, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
		
	}
	else if(nLength <= 24)		//Chaanel 1, 2, 3의 데이터가 날라옴
	{
		memcpy(&rawData[J_CHANNEL_1], pRawData + 1, sizeof(BYTE) * 8);

		ByteSwap(&rawData[J_CHANNEL_1].btCurrent[0], &rawData[J_CHANNEL_1].btCurrent[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_1].btCurrent, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
		nCurrent[J_CHANNEL_1] = _ttoi(sTemp);
	

		ByteSwap(&rawData[J_CHANNEL_1].btAverage[0], &rawData[J_CHANNEL_1].btAverage[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_1].btAverage, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
		
		ByteSwap(&rawData[J_CHANNEL_1].btMin[0], &rawData[J_CHANNEL_1].btMin[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_1].btMin, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
	

		ByteSwap(&rawData[J_CHANNEL_1].btMax[0], &rawData[J_CHANNEL_1].btMax[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_1].btMax, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);

		memcpy(&rawData[J_CHANNEL_2], pRawData + 9, sizeof(BYTE) * 8);

		ByteSwap(&rawData[J_CHANNEL_2].btCurrent[0], &rawData[J_CHANNEL_2].btCurrent[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_2].btCurrent, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
		nCurrent[J_CHANNEL_2] = _ttoi(sTemp);
		

		ByteSwap(&rawData[J_CHANNEL_2].btAverage[0], &rawData[J_CHANNEL_2].btAverage[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_2].btAverage, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
		

		ByteSwap(&rawData[J_CHANNEL_2].btMin[0], &rawData[J_CHANNEL_2].btMin[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_2].btMin, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
		

		ByteSwap(&rawData[J_CHANNEL_2].btMax[0], &rawData[J_CHANNEL_2].btMax[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_2].btMax, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
		
		memcpy(&rawData[J_CHANNEL_3], pRawData + 17, sizeof(BYTE) * 8);

		ByteSwap(&rawData[J_CHANNEL_3].btCurrent[0], &rawData[J_CHANNEL_3].btCurrent[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_3].btCurrent, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
		nCurrent[J_CHANNEL_3] = _ttoi(sTemp);
	
		ByteSwap(&rawData[J_CHANNEL_3].btAverage[0], &rawData[J_CHANNEL_3].btAverage[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_3].btAverage, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
	

		ByteSwap(&rawData[J_CHANNEL_3].btMin[0], &rawData[J_CHANNEL_3].btMin[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_3].btMin, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
	

		ByteSwap(&rawData[J_CHANNEL_3].btMax[0], &rawData[J_CHANNEL_3].btMax[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_3].btMax, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
	
	}
	else						//Channel 1, 2, 3, 4의 데이터가 날라옴
	{
		memcpy(&rawData[J_CHANNEL_1], pRawData + 1, sizeof(BYTE) * 8);

		ByteSwap(&rawData[J_CHANNEL_1].btCurrent[0], &rawData[J_CHANNEL_1].btCurrent[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_1].btCurrent, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
		nCurrent[J_CHANNEL_1] = _ttoi(sTemp);
		

		ByteSwap(&rawData[J_CHANNEL_1].btAverage[0], &rawData[J_CHANNEL_1].btAverage[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_1].btAverage, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
		

		ByteSwap(&rawData[J_CHANNEL_1].btMin[0], &rawData[J_CHANNEL_1].btMin[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_1].btMin, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
		
		ByteSwap(&rawData[J_CHANNEL_1].btMax[0], &rawData[J_CHANNEL_1].btMax[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_1].btMax, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
	
		memcpy(&rawData[J_CHANNEL_2], pRawData + 9, sizeof(BYTE) * 8);

		ByteSwap(&rawData[J_CHANNEL_2].btCurrent[0], &rawData[J_CHANNEL_2].btCurrent[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_2].btCurrent, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
		nCurrent[J_CHANNEL_2] = _ttoi(sTemp);
		
		ByteSwap(&rawData[J_CHANNEL_2].btAverage[0], &rawData[J_CHANNEL_2].btAverage[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_2].btAverage, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
		

		ByteSwap(&rawData[J_CHANNEL_2].btMin[0], &rawData[J_CHANNEL_2].btMin[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_2].btMin, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
		
		ByteSwap(&rawData[J_CHANNEL_2].btMax[0], &rawData[J_CHANNEL_2].btMax[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_2].btMax, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
		
		memcpy(&rawData[J_CHANNEL_3], pRawData + 17, sizeof(BYTE) * 8);

		ByteSwap(&rawData[J_CHANNEL_3].btCurrent[0], &rawData[J_CHANNEL_3].btCurrent[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_3].btCurrent, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
		nCurrent[J_CHANNEL_3] = _ttoi(sTemp);
		
		ByteSwap(&rawData[J_CHANNEL_3].btAverage[0], &rawData[J_CHANNEL_3].btAverage[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_3].btAverage, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
	
		ByteSwap(&rawData[J_CHANNEL_3].btMin[0], &rawData[J_CHANNEL_3].btMin[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_3].btMin, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
		
		ByteSwap(&rawData[J_CHANNEL_3].btMax[0], &rawData[J_CHANNEL_3].btMax[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_3].btMax, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
		

		memcpy(&rawData[J_CHANNEL_4], pRawData + 25, sizeof(BYTE) * 8);

		ByteSwap(&rawData[J_CHANNEL_4].btCurrent[0], &rawData[J_CHANNEL_4].btCurrent[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_4].btCurrent, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
		nCurrent[J_CHANNEL_4] = _ttoi(sTemp);
		

		ByteSwap(&rawData[J_CHANNEL_4].btAverage[0], &rawData[J_CHANNEL_4].btAverage[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_4].btAverage, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
		

		ByteSwap(&rawData[J_CHANNEL_4].btMin[0], &rawData[J_CHANNEL_4].btMin[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_4].btMin, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
	

		ByteSwap(&rawData[J_CHANNEL_4].btMax[0], &rawData[J_CHANNEL_4].btMax[1]);
		memcpy(&nTemp, rawData[J_CHANNEL_4].btMax, sizeof(BYTE) * 2);
		sTemp.Format(_T("%d"), nTemp);
		
	}

	//sCurrentTime
	m_strCH1Volt = CalVoltage(nCurrent[J_CHANNEL_1]);
	m_strCH2Volt = CalVoltage(nCurrent[J_CHANNEL_2]);
	m_strCH3Volt = CalVoltage(nCurrent[J_CHANNEL_3]);
	m_strCH4Volt = CalVoltage(nCurrent[J_CHANNEL_4]);

	m_dStaticVolt[0]	  = atof(m_strCH1Volt) / 1000;
	m_dStaticVolt[1]	  = atof(m_strCH2Volt);
	m_dStaticVolt[2]      = atof(m_strCH3Volt);
	m_dStaticVolt[3]      = atof(m_strCH4Volt);

	//SaveListDataLog();		//로그파일 저장 여부
}

void CDlgESD::ByteSwap(BYTE* pByte, BYTE* pSwapByte)
{
	//********* Swap Byte ******************
	BYTE btTemp;
	btTemp = *(pByte);
	*(pByte) = *(pSwapByte);
	*(pSwapByte) = btTemp;
}

CString CDlgESD::CalVoltage(int nCurrentData)			//Cal Voltage
{
	CString sRetval = _T("0.0");
	double dRetval			= 0;
	
	double dSpecMax			= 20000;
	double dSpecMin			= -20000;
	double dInputMax		= 4095;
	double dInputMin		= 0;
	
	dRetval = (((dSpecMax - dSpecMin) / (dInputMax - dInputMin)) * (nCurrentData - dInputMin)) + dSpecMin;
	
	sRetval.Format(_T("%.2f"),dRetval);
	
	return sRetval;
}

int CDlgESD::DoWorkStartGetESD()
{
	int nRtn = 0;
	nRtn = SendToESD();

	if(!nRtn)
	{
		for(int i = 0; i < 4; i++)
		{
			m_dStaticVolt[i] = -1.0;
		}

		return FALSE;
	}

	return TRUE;
}

void CDlgESD::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	CloseHandle(m_hEvtRcvStop);
	CloseHandle(m_hEvtRcv);

	SAFE_DELETE(m_pRcvThread);
}

int CDlgESD::GetReceiveDataDUMY()
{   
	DWORD  dwS, dwR, dwT=0;

	CString strMsg;
	CString strTmp;
	CString strLog;
	strTmp.Empty();
	
	for (dwS=GetTickCount(); dwT<3000;)  
	{
		if (m_bReceiveOut)
		{
			return TRUE;
		}
		
		int nLen = api_SocketCtrl_GetReceiveDataSize(m_nSocketIndex);
	
		if(nLen>0)
		{
			BYTE* pByte;
			pByte = new BYTE[nLen + 1];
			ZeroMemory(pByte, nLen + 1);	
			
			api_SocketCtrl_GetReceiveData(m_nSocketIndex, pByte, nLen);    
			pByte[nLen] = 0x00;
			
		
			for(int i = 0; i<nLen; i++)
			{
				strTmp.Format(_T("%02X "), pByte[i] ); 
				strMsg += strTmp;
			}
			strLog.Format(_T("RCV: %s (%d)"), strMsg, nLen);
			m_pdlgMain->WriteLogList(strLog, m_nNum);
			
		//	delete pByte;
			SAFE_DELETE_AR(pByte);		// jsyoon 20181112 add
			
			return TRUE;
		}
		
		Sleep(10);
		dwR = GetTickCount();
		
        if (dwR >= dwS)  
			dwT = dwR - dwS;
        else
			dwT = ((DWORD)0xFFFFFFFF) - (dwS - dwR);
	}
	
	strLog.Format("RCV: Timeout...");
	m_pdlgMain->WriteLogList(strLog, m_nNum);
	
	return TRUE;
}


int CDlgESD::SetType(int nType)
{
	m_nType = nType;
	return 0;
}


int CDlgESD::ConnectESD(int nNum, int index)
{
	m_nNum = nNum;

	if( m_nType == 0)
	{
		SocketConnectESD(m_strIP, m_strPort,index);
	}
	else
	{
		SerialConnectESD(m_nNum);
	}
	return 0;
}

int CDlgESD::DisConnectESD(int index)
{
	if( m_nType == 0)
		SocketDisConnectESD(index);
	else
		SerialDisConnect(m_nNum);

	return 0;
}


int CDlgESD::SetConnectInfo(CString strIp, CString strPort, ST_SERIAL stSerial)
{
	m_strIP = strIp;
	m_strPort = strPort; // jsyoon 2022.05.16 change

	m_stSerial = stSerial;
	return 0;
}


int CDlgESD::SerialConnectESD(int index)
{
	int nRet = 0;
	nRet = OpenPort(m_stSerial.m_uiPort, m_stSerial.m_uiBaudRate, m_stSerial.m_uiDataBits, m_stSerial.m_uiParity, m_stSerial.m_uiStopBit, index);

	return nRet;
}

int CDlgESD::SerialDisConnect(int nMode)
{
	m_pdlgMain->WriteLogList(_T("Close!"), nMode);
	
	BOOL bRes = FALSE;
	if(m_pRcvThread != NULL)
	{
		SetEvent(m_hEvtRcvStop);
	}

	api_SerialCom_Close(m_nSerialPort);

	return 0;
}

int CDlgESD::OpenPort(UINT nPort, DWORD dwRate,UINT bDataBits, UINT bParity, UINT bStopBits, UINT nMode)
{
	int nRet = 1;
	m_nSerialPort = nPort;

	if (api_SerialCom_Open(nPort,dwRate,(BYTE)bDataBits,(BYTE)bParity,(BYTE)bStopBits))
	{
		if(api_SerialCom_IsOpened(nPort))
		{
			ThreadBegin_RcvData();
			m_pdlgMain->WriteLogList(_T("Port open success!"), nMode);
			nRet = 1;
		}
		else
		{
			m_pdlgMain->WriteLogList(_T("Port open fail!"), nMode);
			nRet = 0;
		}
	}
	else
	{
		m_pdlgMain->WriteLogList(_T("Port open fail!"), nMode);
		nRet = -1;
	}

	return nRet;
}

void CDlgESD::ThreadBegin_RcvData(void)
{
	if(!m_pRcvThread)
	{
		m_pRcvThread = AfxBeginThread(ThreadRcvData, this);
		m_pRcvThread->m_bAutoDelete = FALSE;
	}
}

UINT CDlgESD::ThreadRcvData(LPVOID pParam)
{
	CDlgComm *pProc = (CDlgComm*) pParam;
	volatile BOOL bLoop  = TRUE;
	while(bLoop)
	{
		DWORD res = WaitForSingleObject(pProc->m_hEvtRcvStop, 100);
		switch(res)
		{
		case WAIT_OBJECT_0:
			bLoop = FALSE;
			break;
		case WAIT_TIMEOUT:
			pProc->RcvData();
			break;
		}
	}
	return TRUE;
}


int CDlgESD::RcvData(void)
{
	int nRcvCnt = 0;
	//	UINT nCurIdx = 0;
	BYTE byteReadChar;
	BYTE byteCheckSum = 0x00;
	BYTE byteBuffer[dfBUFFER_SIZE];
	CString strRcvData = _T("");
	CString strTotalRcvData = _T(""), strLog = _T("");
	int nCount=0;

	int i=0, j=0;
	//	int iMsgSize = 0;
	CString strTmp, strMsg;

	ZeroMemory(byteBuffer, sizeof(BYTE)*dfBUFFER_SIZE);

	int nPort = m_nSerialPort;

	while(TRUE)
	{
		if(!api_SerialCom_IsOpened(nPort))
			return FALSE; // return FALSE;

		nRcvCnt = api_SerialCom_GetDataInQueue(nPort);

		//20150604
		if(nRcvCnt>=1008)
		{
			api_SerialCom_ReadByte(nPort, byteBuffer, dfBUFFER_SIZE);
			return FALSE;
		}

		if(nRcvCnt > 0)
		{
			if( nRcvCnt < dfBUFFER_SIZE )
			{
				api_SerialCom_ReadByte(nPort, byteBuffer, nRcvCnt);	

				for(int i=0; j<nRcvCnt; j++)
				{
					strRcvData.Format(_T("%02X"), byteBuffer[i] ); 
					strTotalRcvData += strRcvData;
				}

				strLog.Format(_T("RCV: %s (%d)"), strTotalRcvData, nRcvCnt);
				m_pdlgMain->WriteLogList(strLog, m_nNum);

				unsigned short crc16;
				crc16 = CRC16_MODBUS(byteBuffer, nRcvCnt-16); 

				AnalysisReceiveData(nRcvCnt, byteBuffer);

				ZeroMemory(byteBuffer, sizeof(BYTE)*dfBUFFER_SIZE);
			}
		}
		else
		{
			return FALSE; //break;
		}
	}
	return TRUE;
}
