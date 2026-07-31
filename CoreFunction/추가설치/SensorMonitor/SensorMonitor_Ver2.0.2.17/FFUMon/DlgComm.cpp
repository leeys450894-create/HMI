// DlgComm.cpp : implementation file
//

#include "stdafx.h"
#include "FFUMon.h"
#include "DlgComm.h"

#include "FFUMonDlg.h"
#include <MATH.H>

#include "../include/API_SerialCtrl.h"
#pragma comment(lib, "../Lib/dSerialctrl.lib")

#include "../include/API_SharedMemory.h"
#pragma comment(lib, "../Lib/dSharedMemory.lib")
#include "crc16.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define EBM_EFU_NUM  0x21		//20181116 edhan add
/////////////////////////////////////////////////////////////////////////////
// CDlgComm dialog

//union은 내부 멤버 중에 제일 큰 멤버 크기의 메모리를 할당합니다.
typedef union
{
    float value; //4바이트
    struct
    { 
        unsigned exponent:23; //23비트
        unsigned mantissa:8; //8비트
        unsigned sign:1; //1비트
    }sv; //4바이트
    unsigned iv;//4바이트    
}unFormat;

CDlgComm::CDlgComm(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgComm::IDD, pParent)
	, m_nMaxChannel(0)
	, m_nNGCount(0)
	, m_nRcvValue(0)
	, m_dRcvValue(0)
	, m_nSendType(0)
{
	//{{AFX_DATA_INIT(CDlgComm)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_pFFUMon = (CFFUMonDlg *)pParent;
	m_strReceive.Empty();

	m_nMode = 0;
	m_nPort = 0;
	m_nNum  = 0;

	//test
	m_pRcvThread = NULL;
	m_pGetStateThread = NULL;
	m_pMonitoringThread = NULL;
	//

	m_nSetAuccuraData = 0;

	m_Validity = 0;
	for( int j=0; j < 40; j++)
	{
		m_subunitValidity[j] = -1;
	}

	m_hEvtRcv = CreateEvent(NULL, FALSE,  FALSE, NULL);  // auto reset,   initially set
}


void CDlgComm::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgComm)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDlgComm, CDialog)
	//{{AFX_MSG_MAP(CDlgComm)
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
	ON_MESSAGE(UWM_RCVDATA, OnRcvSerialData)
	ON_BN_CLICKED(IDOK, &CDlgComm::OnBnClickedOk)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgComm message handlers

BOOL CDlgComm::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here

	if( m_nMode == SENSOR_MODE_EFU )
	{
		int i=0;
		for (i=0; i<dfMAX_EFU; i++)
		{
			API_SM_SetFFU_ALARM(i, 1, m_pFFUMon->m_nIdex);
		}
	}

	else if( m_nMode == SENSOR_MODE_ACCURA2 )
	{
		m_pfAuccuraData = new float[400];
		ZeroMemory(m_pfAuccuraData, sizeof(m_pfAuccuraData)*400);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDlgComm::OpenPort(UINT nPort, DWORD dwRate,UINT bDataBits, UINT bParity, UINT bStopBits, UINT nMode)
{
	m_nPort = nPort;

	if (api_SerialCom_Open(nPort,dwRate,(BYTE)bDataBits,(BYTE)bParity,(BYTE)bStopBits))
	{
		if(api_SerialCom_IsOpened(nPort))
		{
			ThreadBegin_RcvData();
			m_pFFUMon->WriteLogList(_T("Port open success!"), nMode);
		}
		else
		{
			m_pFFUMon->WriteLogList(_T("Port open fail!"), nMode);
		}
	}
	else
	{
		m_pFFUMon->WriteLogList(_T("Port open fail!"), nMode);
	}

//	if( api_SerialCom_IsOpened(nPort))
	{

		StartThreadGetState();	//test

		if (nMode == SENSOR_MODE_EFU || nMode == SENSOR_MODE_EFU2 || nMode == SENSOR_MODE_EFU3 || nMode == SENSOR_MODE_EFU4 || nMode == SENSOR_MODE_EFU5)	//20181207 edhan add EFU num(3 ~ 5)
			StartThreadMonitoring();
	}
}

void CDlgComm::StartThreadMonitoring()
{
	//AfxBeginThread(WorkThreadProcGetState, this);
	
	if(!m_pMonitoringThread)
	{
		m_hEvtMonitoringStop = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_pMonitoringThread = AfxBeginThread(WorkThreadProcMonitoring, this);
		m_pMonitoringThread->m_bAutoDelete = FALSE;
	}
}

void CDlgComm::ExitThreadAll()
{
	DWORD dwExitCode;
	BOOL bRes = FALSE;
	if(m_pRcvThread != NULL)
	{
		bRes = GetExitCodeThread(m_pRcvThread->m_hThread, &dwExitCode);
		if(bRes || dwExitCode == STILL_ACTIVE)
		{
			SetEvent(m_hEvtRcvStop);
			ExitThread(m_pRcvThread->m_hThread, 2000);
			
		}
	}

	if(m_pMonitoringThread != NULL)
	{
		SetEvent(m_hEvtMonitoringStop);
		ExitThread(m_pMonitoringThread->m_hThread, 2000);
	}

	if(m_pGetStateThread != NULL)
	{
		SetEvent(m_hEvtGetStateStop);
		ExitThread(m_pGetStateThread->m_hThread, 2000);
	}
	
}

BOOL CDlgComm::ExitThread(HANDLE hThread, DWORD dwDelay)
{
	DWORD dwTick = GetTickCount();
	while(TRUE)
	{
		if(GetTickCount() - dwTick >= dwDelay)
		{
			TerminateThread(hThread, 0);
			return FALSE;
		}
		if(WaitForSingleObject(hThread, 0) != WAIT_TIMEOUT)
			return TRUE;
		
		MSG msg;
		while(::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}
}


void CDlgComm::ThreadBegin_RcvData(void)
{
	if(!m_pRcvThread)
	{
		m_hEvtRcvStop = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_pRcvThread = AfxBeginThread(ThreadRcvData, this);
		m_pRcvThread->m_bAutoDelete = FALSE;
	}
}

void CDlgComm::ReceiveMsgProcess(vector<BYTE> vecRvcData)
{
	int i = 0;
	int iMsgSize = 0;
	CString strMsg = _T("");
	CString strTmp = _T("");
	CString strLog = _T("");
	CString strTemp = _T("");
	CString strPressure = _T("");
	CString strEFU = _T("");
	BOOL    bReciveDone = FALSE;
	byte bDelimeter = 0;
	
	iMsgSize = vecRvcData.size();	
	
	//	WriteLogList(strLog);

	if (m_nMode == SENSOR_MODE_H2O )
	{	
		// nk.lee 2020.06.15
		if(m_pFFUMon->m_iH2OSensorType == 1)
		{
			for(int i = 0; i<iMsgSize-1; i++)
			{
				strTemp += vecRvcData[i];

				strTmp.Format(_T("%02X "), vecRvcData[i] ); 
				strMsg += strTmp;

				bDelimeter	 = vecRvcData[i];
			}

			strLog.Format(_T("RCV: %s (%d)"), strMsg,iMsgSize);
			m_pFFUMon->WriteLogList(strLog, m_nMode);

			m_strReceive += strTemp;

			if(bDelimeter == 0x03 && m_strReceive.GetLength() == 12)			// Receive Data	Check
			{
				double dRet = 0;
				dRet = GetIntputValue(m_strReceive);

				if( dRet > -10 )		// Temp -10 
				{
					m_pFFUMon->m_dH2O[m_nNum] = dRet;	//yhjoo 20141120 add

					strLog.Format(_T(">>>>>>>Get VALUE = %.4f"), dRet);
					m_pFFUMon->WriteLogList(strLog, m_nMode);

					SetEvent(m_hEvtRcv);

				}


				m_strReceive.Empty();
			}
		}	// nk.lee end
		else
		{

			for( i = 0; i<iMsgSize; i++)
			{
				bDelimeter	 = vecRvcData[i];

				if( bDelimeter != 0x00 )
					strTmp.Format(_T("%c"), vecRvcData[i] ); 
				else
					strTmp.Format(_T("0"), vecRvcData[i] ); 
 
				strMsg += strTmp;
			}
		
			strLog.Format(_T("RCV: %s (%d)"), strMsg,iMsgSize);
			m_pFFUMon->WriteLogList(strLog, m_nMode+m_nNum);
		
			m_strReceive += strMsg;
			if(bDelimeter == 0x0a && m_strReceive.GetLength() >= 14)
			{
				double dRet = 0;
				dRet = GetIntputValue(m_strReceive);
				m_strReceive.Empty();

				SetEvent(m_hEvtRcv);

				m_pFFUMon->m_dH2O[m_nNum] = dRet;	//yhjoo 20141120 add
				strLog.Format(_T(">>>>>>>Get VALUE = %.4f"), dRet);
				m_pFFUMon->WriteLogList(strLog, m_nMode);	
			}
			else if(m_strReceive.GetLength() > 14)
			{
				m_strReceive.Empty();
			}
		}
	}
	else if (m_nMode == SENSOR_MODE_O2)
	{
		for( i = 0; i<iMsgSize; i++)
		{
			strTmp.Format(_T("%c"), vecRvcData[i] ); 
			strMsg += strTmp;
			
			bDelimeter	 = vecRvcData[i];
		}
		
		strLog.Format(_T("RCV: %s (%d)"), strMsg,iMsgSize);
		m_pFFUMon->WriteLogList(strLog, m_nMode);

		m_strReceive += strMsg;
		if(bDelimeter == 0x0d && m_strReceive.GetLength() >= 12)
		{
			double dRet = 0;		// jsyoon 20150106 change
			dRet = GetIntputValue(m_strReceive);
			m_strReceive.Empty();

			m_pFFUMon->m_dO2 = dRet;
			strLog.Format(_T(">>>>>>>Get VALUE = %.4f"), dRet);
			m_pFFUMon->WriteLogList(strLog, m_nMode);
		}
		else if(m_strReceive.GetLength() > 12)
		{
			m_strReceive.Empty();
		}
	}
	else if (m_nMode == SENSOR_MODE_TEMP1 )
	{
		for( i = 0; i<iMsgSize; i++)
		{
// 			strTmp.Format(_T("%s"), vecRvcData[i] ); 

			bDelimeter	 = vecRvcData[i];

			if( bDelimeter != 0x00 )
				strTemp += vecRvcData[i];
			else
				strTemp += "0";

			strTmp.Format(_T("%02X "), vecRvcData[i] ); 
			strMsg += strTmp;
		}
		
		strLog.Format(_T("RCV: %s (%d)"), strMsg, iMsgSize);
		m_pFFUMon->WriteLogList(strLog, m_nMode+m_nNum);

		m_strReceive += strTemp;
		
		CString strValue;
		int iAddr = 0;
		int nReciveLen = 0;
		
		if(bDelimeter == 0x00 )	
		{
			strTemp.Format("%s", m_strReceive);

			double dValue = 0;
			double dRet = 0;		// jsyoon 20150106 change
			int iFind = 0;
			int dot = 0;

			while(1) 
			{
				nReciveLen = strTemp.GetLength();
				if( nReciveLen >= 17)
				{
					strValue = strTemp.Mid(2, 2);
					iAddr = atoi(strValue) - 1;

					iFind = strTemp.Find("RDP");
					if( iFind > 1 )
					{
						strValue = strTemp.Mid(iFind+5, 5);
						dot = strTemp[13]-'0';		// 소수점
						dValue = atof(strValue)/10;

						for(int k=0;k<dot;k++)
							dValue /= 10;

						dRet = dValue;
					}
					//////////////////////////////////////////////////////////////////////////

					if( dRet > -10 )		// Temp -10 
					{
						if( iAddr >=0 && iAddr < 32)
							m_pFFUMon->m_dThermometer[m_nNum][iAddr] = dRet;

						SetEvent(m_hEvtRcv);

						strLog.Format(_T(">>>>>>>Get VALUE(%d) = %.3f"), iAddr+1, dRet);
						m_pFFUMon->WriteLogList(strLog, m_nMode+m_nNum);
					}

					strTemp.Delete(0, 17);
				}
				else
				{
					break;
				}
			} 

			m_strReceive.Empty();
		}
	}
	else if (m_nMode == SENSOR_MODE_PRESSURE)
	{
		for( i = 0; i<iMsgSize; i++)
		{
// 			strTmp.Format(_T("%s"), vecRvcData[i] ); 
			strPressure += vecRvcData[i];
			
			strTmp.Format(_T("%02X "), vecRvcData[i] ); 
			strMsg += strTmp;
			
			if (i != iMsgSize-1)
				bDelimeter	 = vecRvcData[i];
		}

		strLog.Format(_T("RCV: %s (%d)"), strMsg,iMsgSize);
		m_pFFUMon->WriteLogList(strLog, m_nMode);

		m_strReceive += strPressure;
		if(bDelimeter == 0x03 && m_strReceive.GetLength() >= 13)
		{
			double dRet = 0;								// jsyoon 20150106 change
			dRet = GetIntputValue(m_strReceive);	
			strLog.Format(_T(">>>>>>>Get VALUE = %.1f"), dRet);	//20141209 ing
			m_pFFUMon->m_dPressure = dRet;
			m_pFFUMon->WriteLogList(strLog, m_nMode);
			m_strReceive.Empty();
		}
		else if(m_strReceive.GetLength() > 13)
		{
			m_strReceive.Empty();
		}
	}
	else if (m_nMode == SENSOR_MODE_EFU || m_nMode == SENSOR_MODE_EFU2 || m_nMode == SENSOR_MODE_EFU3 || m_nMode == SENSOR_MODE_EFU4 || m_nMode == SENSOR_MODE_EFU5)
	{
		for( i = 0; i<iMsgSize; i++)
		{
			strTmp.Format(_T("%02X "), vecRvcData[i] ); 
			strMsg += strTmp;
		}

		strLog.Format(_T("RCV: %s (%d)"), strMsg,iMsgSize);	
		m_pFFUMon->WriteLogList(strLog, m_nMode);

//		if (m_pFFUMon->m_bEFU_Type)	//yhjoo 20160704 add (TRUE = SinSung, FALSE = ISU)
		if (m_pFFUMon->m_bEFU_Type == EFU_TYPE_SHINSUNG)	//yhjoo 20160704 add (TRUE = SinSung, FALSE = ISU)	//20181116 edhan change
		{
			bReciveDone = TRUE;
// 			strLog.Format(_T("RCV: %s (%d)"), strMsg,iMsgSize);
// 			
// 			m_pFFUMon->WriteLogList(strLog, m_nMode);
			
			SetEFUData(vecRvcData, SENSOR_MODE_EFU+m_nNum);
		}
		else if (m_pFFUMon->m_bEFU_Type == EFU_TYPE_ISU)
		{
			//ksk 20150608 add
			iMsgSize = vecRvcData.size();
			if(iMsgSize == 9 && vecRvcData[0]  == 0xFA)
			{
				bReciveDone = TRUE;

// 				strLog.Format(_T("RCV: %s (%d)"), strMsg,iMsgSize);
// 				
// 				m_pFFUMon->WriteLogList(strLog, m_nMode);
				
				SetEFUData(vecRvcData, SENSOR_MODE_EFU+m_nNum);
			}
		}
		//20181116 edhan add
		else if (m_pFFUMon->m_bEFU_Type == EFU_TYPE_EBM)
		{
			iMsgSize = vecRvcData.size();
			if(iMsgSize >= 5 && vecRvcData[0]  == 0x31)	//00110001
			{
				bReciveDone = TRUE;
				SetEFUData(vecRvcData, SENSOR_MODE_EFU+m_nNum);
			}
		}
		//

		else if(m_pFFUMon->m_bEFU_Type == EFU_TYPE_MAYAIR)			// 3 : MayAir	// yj.nam 20190520 add
		{
			iMsgSize = vecRvcData.size();

			if(iMsgSize >= 5)	
			{
				bReciveDone = TRUE;
				SetEFUData(vecRvcData);
			}
		}
		else if(m_pFFUMon->m_bEFU_Type == EFU_TYPE_HKC_H5)	// jsyoon 20201104 add
		{
			iMsgSize = vecRvcData.size();
			if(iMsgSize >= 5 && ( vecRvcData[1]  == 0x03 || vecRvcData[1]  == 0x83) )	//00110001
			{
				bReciveDone = TRUE;
				SetEFUData(vecRvcData, SENSOR_MODE_EFU+m_nNum);
			}
		}

		if( bReciveDone )
		{
			SetEvent(m_hEvtRcv);
		}
	}
	else if (m_nMode == ELECTRONIC_DISPLAY)
	{

	}
	else if (m_nMode == SENSOR_MODE_KMN2)		//jhseo 20170920 add for KM-N2
	{
		
		for(i = 0; i<iMsgSize; i++)
		{
			strTemp += vecRvcData[i];
			
			strTmp.Format(_T("%02X "), vecRvcData[i] ); 
			strMsg += strTmp;
			
			bDelimeter	 = vecRvcData[i];
		}
		
		strLog.Format(_T("RCV: %s (%d)"), strMsg,iMsgSize);
		m_pFFUMon->WriteLogList(strLog, m_nMode);
		
		m_strReceive += strTemp;
		if(m_strReceive.GetLength() > 25)
		{
			m_strReceive.Empty();
			return;
		}

		CString strResult;
		strResult.Empty();
		strResult	= m_strReceive.Mid(15, 8);
//		strResult	= m_strReceive.Mid(11, 12);
//		strResult	= m_strReceive;
		int nLength = strResult.GetLength();
		
		BYTE bin[8];
		
		unsigned short sData = 0;
		
		for (i=0; i<nLength; i++)
		{
			bin[i] = HEXtoBYTE(strResult[(nLength-1)-i]);
			if (bin[i]>=0 && bin[i]<=15)
				sData += bin[i]<<(4*i);
		}
		double dResult = sData/1000.0;

		CString strDelimeter;
		strDelimeter.Empty();
		strDelimeter = m_strReceive.Mid(1, 2);
		
		if(strDelimeter == "01")
		{
			TRACE("Address(1) REVData = %.3f\r\n", dResult);
			
			m_pFFUMon->m_dKMN2Result[0] = dResult;
		}
		else if(strDelimeter == "05")
		{
			TRACE("Address(5) REVData = %.3f\r\n", dResult);

			m_pFFUMon->m_dKMN2Result[1] = dResult;
		}
		else
		{
			TRACE("Address(Error) REVData = %.3f\r\n", dResult);
		}
		
		strLog.Format(_T("GetValue: %.4f"), dResult);
		m_pFFUMon->WriteLogList(strLog, m_nMode);

		m_strReceive.Empty();
	}
	else if (m_nMode == SENSOR_MODE_ACCURA2)		//jhseo 20170920 add for KM-N2
	{
		for(i = 0; i<iMsgSize; i++)
		{
			strTmp.Format(_T("%02X"), vecRvcData[i] ); 
//			strTmp.Format(_T("%d"), vecRvcData[i] ); 
			strMsg += strTmp;
		}

		strLog.Format(_T("RCV: %s (%d)"), strMsg,iMsgSize);

		m_pFFUMon->WriteLogList(strLog, m_nMode);

		ProceData(strMsg);

		SetEvent(m_hEvtRcv);
	}
	else if(m_nMode == SENSOR_MODE_AZBIL)
	{
		for(i = 0; i<iMsgSize; i++)
		{
			strTmp.Format(_T("%02X"), vecRvcData[i] ); 
			strMsg += strTmp;
		}

		//////////////////////////////////////////////////////////////////////////
		int  iBuf[4] = {0};
		for(i = 0; i < 4; i++)
		{
			iBuf[i] = HEXtoBYTE(vecRvcData[i+7]);

			if(i == 0)
				iBuf[i] *= 4096;
			else if(i == 1)
				iBuf[i] *= 256;
			else if(i == 2)
				iBuf[i] *= 16;
		}

		m_nRcvValue = iBuf[0] + iBuf[1] + iBuf[2] + iBuf[3];
		//////////////////////////////////////////////////////////////////////////

		strLog.Format(_T("RCV: %s (%d)"), strMsg,iMsgSize);

		m_pFFUMon->WriteLogList(strLog, m_nMode);

		SetEvent(m_hEvtRcv);
	}
	else if(m_nMode == SENSOR_MODE_PM5500)
	{
		BYTE btRecvBuf[dfBUFFER_SIZE];
		memcpy(btRecvBuf, btRecvBuf, dfBUFFER_SIZE);

		for(i = 0; i<iMsgSize; i++)
		{
			strTmp.Format(_T("%02X"), vecRvcData[i] ); 
			strMsg += strTmp;

			btRecvBuf[i] =  vecRvcData[i];
		}

		if( btRecvBuf[0] != 0x01 || btRecvBuf[1] != 0x03 )
			return;

		float	fData	=	0.f;
		ByteToFloat(btRecvBuf, fData, 3, true) ;		//Start Address(value) = 3
		m_dRcvValue = (double)fData;	

		strLog.Format(_T("RCV: %s (%d)"), strMsg,iMsgSize);

		m_pFFUMon->WriteLogList(strLog, m_nMode);

		SetEvent(m_hEvtRcv);
	}

	else if(m_nMode == SENSOR_MODE_IONIZER || m_nMode == SENSOR_MODE_IONIZER2)
	{
		SetIonizer(vecRvcData);
	}

// 	if(bDelimeter == 0x0a)	//H2O
// 	{
// 		m_pFFUMon->WriteLogList(strLog, m_nMode);
// 		strLog.Format(_T("Get VALUE = %.4f"), GetIntputValue(m_strReceive));
// 		m_pFFUMon->WriteLogList(strLog, m_nMode);
// 		m_strReceive.Empty();
// 	}
	
	
	//	SetEFUData(vecRvcData);
	
	//20131230 twkoo
	//vecRvcData.clear();
}

double CDlgComm::GetIntputValue(CString strRcv)
{
	CString strValue;
	int nLen = 0;
	int i=0;
	strValue.Empty();

	if (m_nMode == SENSOR_MODE_H2O )
	{
		// nk.lee 2020.06.15
		if(m_pFFUMon->m_iH2OSensorType == 1)
		{

			char cData[124] = {0,};
			strcpy(cData,strRcv.GetBuffer(0));

			if (cData[3] == 0x4F && cData[4] == 0x4B) // 0x4B = 'O', 0x4B = 'K'
			{
				nLen = strRcv.GetLength();
				for (i=6; i<nLen-1; i++)
				{
					strValue += cData[i];
				}

				if( strValue == _T("HHHHH"))
					return 9999;
			}
		}	// nk.lee end
		else{
			strValue = strRcv.Mid(0,strRcv.GetLength()-2);
			strValue.TrimLeft();
			strValue.TrimRight();
		}
		
	}
	else if (m_nMode == SENSOR_MODE_O2)
	{
		strValue = strRcv.Mid(2,strRcv.GetLength()-1);
		strValue.TrimLeft();
	}
	else if (m_nMode == SENSOR_MODE_TEMP1 )
	{
		int iFind = 0;
		iFind = strRcv.Find("RDP");
		if( iFind > 1 )
		{
			strValue = strRcv.Mid(iFind+5, 5);
		}

	//	TRACE("TEMP(%d)%s\n", m_nMode, strRcv);	//ksk 20150110 remove
	}
	else if (m_nMode == SENSOR_MODE_PRESSURE)
	{
		char cData[64] = {0,};
//		strcpy(cData,strRcv.GetBuffer(0));
//		strcpy_s(cData,strRcv.GetBuffer(0));	// mn.bong 20180709 change strcpy -> strcpy_s
		_tcscpy_s(cData, _countof(cData), strRcv.GetBuffer(0));
		if (cData[3] == 0x4F && cData[4] == 0x4B) // 0x4B = 'O', 0x4B = 'K'
		{
			nLen = strRcv.GetLength();
			for (i=6; i<nLen-1; i++)
			{
				strValue += cData[i];
			}
		}
		
		if (cData[5] == 0x2D)
			strValue = "-" + strValue;
	}
	else if (m_nMode == SENSOR_MODE_EFU)
	{

	}	

	if( strValue.IsEmpty())
		return -999999;

	return atof(strValue);
}

LRESULT CDlgComm::OnRcvSerialData(WPARAM wParam, LPARAM lParam)
{
	wParam;
	lParam;

	vector<BYTE> vecRvcData;
	vecRvcData = m_vecRvcData;
	ReceiveMsgProcess(vecRvcData);
	m_vecRvcData.clear();	// jsyoon 20150113 change
	return TRUE;
}

BOOL CDlgComm::RcvData(void)
{
	int nRcvCnt = 0;
//	UINT nCurIdx = 0;
	BYTE byteReadChar;
	BYTE byteCheckSum = 0x00;
	BYTE byteBuffer[dfBUFFER_SIZE];
	CString strRcvData = _T("");
	CString strTotalRcvData = _T("");
	BOOL bSTX= FALSE, bETX= FALSE;
	BOOL bDPUID= FALSE;
	int nCount=0;
	vector<BYTE> vecRvcData;

	int i=0, j=0;
//	int iMsgSize = 0;
	CString strTmp, strMsg;

	ZeroMemory(byteBuffer, sizeof(BYTE)*dfBUFFER_SIZE);
	
	vecRvcData.clear();

	while(TRUE)
	{
		if(!api_SerialCom_IsOpened(m_nPort))
			return FALSE; // return FALSE;

		nRcvCnt = api_SerialCom_GetDataInQueue(m_nPort);

		//20150604
		if(nRcvCnt>=1008)
		{
			api_SerialCom_ReadByte(m_nPort, byteBuffer, dfBUFFER_SIZE);
			return FALSE;
		}
		
		if(nRcvCnt >0)
		{
//			CString strLog, strMsg;
//			for(int n=0;n<nRcvCnt;n++)
//			{
//				strLog.Format(_T("%02X "), byteuf[i] ); 
//				strMsg += strLog;
//			}
			
			if(m_cEndDelimiter == NULL) // Receive delimeter : NULL
			{
				TRACE("REVData = %d\r\n", nRcvCnt);	//ksk 20150110 for test

				if( nRcvCnt < dfBUFFER_SIZE )
				{
					api_SerialCom_ReadByte(m_nPort, byteBuffer, nRcvCnt);	
					
					for( i = 0; i<nRcvCnt; i++)
					{
						m_vecRvcData.push_back(byteBuffer[i]);
					}
					
					//				::PostMessage(this->GetSafeHwnd(), UWM_RCVDATA, NULL, NULL);
					::SendMessage(this->GetSafeHwnd(), UWM_RCVDATA, NULL, NULL);	// jsyoon 20150109 change
					//				ZeroMemory(&byteBuffer, sizeof(byteBuffer));
					ZeroMemory(byteBuffer, sizeof(BYTE)*dfBUFFER_SIZE);
				}
			}
			else // Receive delimiter : CR, LF, CRLF, ETX, STX.
			{
				Sleep(100);
				for( i=0;i<nRcvCnt;i++)
				{
					api_SerialCom_ReadByte(m_nPort, &byteReadChar, 1);
					
					if(bSTX)
					{
						vecRvcData.push_back(byteReadChar);
					}
					
					//Check Start Bit
					if(byteReadChar == dfSTX && !bSTX && !bDPUID)
					{
						bSTX = TRUE;
						vecRvcData.push_back(byteReadChar);
					}
					if(bSTX && byteReadChar == dfDPU_ID)
					{
						bDPUID = TRUE;
					}
					
					//Check End Bit
					if(byteReadChar == m_cEndDelimiter)
					{
						byteCheckSum = 0x00;

						int Len = vecRvcData.size();
		//				if(Len>1)
						if(Len>2)		// jsyoon 20180725 change
						{
							Len = vecRvcData.size()-2;
							for( j=1; j<Len; j++)
							{
								byteCheckSum += vecRvcData[j];
								strRcvData.Format(_T("%02X"), vecRvcData[j] ); 
							}

							byteCheckSum = byteCheckSum & 0xff;
							strRcvData.Format(_T("%02X"), byteCheckSum ); // 계산한 CheckSum
							strRcvData.Format(_T("%02X"),  vecRvcData[Len]); //실제로 받은 CheckSum
					//		strRcvData.Format(_T("%02X"),  vecRvcData[vecRvcData.size()-2]); //실제로 받은 CheckSum
							
							//Check CheckCum
					//		if(byteCheckSum == vecRvcData[vecRvcData.size()-2])
							if(byteCheckSum == vecRvcData[Len])
							{
								bETX = TRUE;
							}
						}
// 						else
// 						{
// 							bETX = TRUE;
// 						}
					}
					
					if(bSTX && bDPUID && bETX)
					{
						//20131230 twkoo
						//m_vecRvcData = vecRvcData;
						//::PostMessage(this->GetSafeHwnd(), UWM_RCVDATA, NULL, NULL);
						
						bSTX = FALSE;
						bDPUID = FALSE;
						bETX = FALSE;
						nCount = 0;
						
						//20131230 twkoo
						ReceiveMsgProcess(vecRvcData);
						vecRvcData.clear();
					}
					nCount ++;
				}
			}
		}
		else
		{
			return FALSE; //break;
		}
	}
	return TRUE;
}

UINT CDlgComm::ThreadRcvData(LPVOID pParam)
{
	CDlgComm *pProc = (CDlgComm*) pParam;
	volatile BOOL bLoop  = TRUE;
	while(bLoop)
	{
		DWORD res = WaitForSingleObject(pProc->m_hEvtRcvStop, 200);
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

BOOL CDlgComm::SendMsg(BYTE *byteBuff, int iLength)
{
	BOOL bRet = FALSE;
	
	CString strLog, strMsg;
	
	for(int i = 0; i<iLength; i++)
	{
		strLog.Format(_T("%02X "), byteBuff[i] ); 
		strMsg += strLog;
	}
	
	m_pFFUMon->WriteLogList(strMsg, m_nMode);
	
	if(!api_SerialCom_IsOpened(m_nPort))
	{
		//		AfxMessageBox(_T("Please, open port first!"));
		m_pFFUMon->WriteLogList(_T("Port open fail!"), m_nMode);
		return FALSE;
	}
	
	bRet = api_SerialCom_WriteByte(m_nPort, byteBuff, iLength);
	
	return bRet;
}

void CDlgComm::SetEFUValue(UINT nEfu, UINT nValue)
{
	CString strValue = _T("");
	CString strHex = _T("");
	CString strMode1, strMode2, strLVID, strDPUID, strBLIDStart, strBLIDEnd, strSV;
	CString strSendMsg = _T("");
	
	CString strMsg;
	CString strout;
	BYTE byteuf[256];
	int nCnt = 0, i =0;
	
	int iValue = 1;	//고정
	int iLVID = iValue | 0x80;
	strHex.Format(_T("%02X"), iLVID);
	strLVID = strHex;
	
	iValue = nEfu;
	int iELIDStart = iValue;
	strHex.Format(_T("%02X"), iELIDStart);
	strBLIDStart = strHex;
	
	iValue = nEfu;
	int iBLIDEnd = iValue;
	strHex.Format(_T("%02X"), iBLIDEnd);
	strBLIDEnd = strHex;
	
	iValue = nValue;
	int iSV = iValue;
	strHex.Format(_T("%02X"), iSV);
	strSV = strHex;
	
	strMode1 = _T("89");
	strMode2 = _T("84");
	//	strLVID = "";
    strDPUID = _T("9F");
	//	strBLIDStart = "";
	//	strBLIDEnd = "";
	//	strSV = "";
	
	strSendMsg = strMode1 + strMode2 + strLVID + strDPUID + strBLIDStart + strBLIDEnd + strSV;
	
	
	int iMsgLength = strSendMsg.GetLength();
	int iLength = strSendMsg.GetLength()/2;
	
	for(i=0; i<iMsgLength; i++)
	{
		strMsg = strMsg + strSendMsg[i];
		if((i+1) %2 == 0 &&  i >0)
		{
			nCnt ++;
			strout.Format(_T("%d"), strtol(strMsg, NULL, 16));
//			byteuf[nCnt] = atoi(strout);
			byteuf[nCnt] = (BYTE)atof(strout);
			strMsg .Empty();
		}
	}
	
	byteuf[0] = dfSTX;
	byteuf[iLength+1] = 0x00;
	for(i =0; i<nCnt; i++)
	{
		byteuf[iLength+1] += byteuf[i+1];
		strMsg.Format(_T("%02X"), byteuf[i+1]); 
	}
	byteuf[iLength+1] = byteuf[iLength+1] & 0xff;
	
	strMsg.Format(_T("%02X"), byteuf[iLength+1]); 
	byteuf[iLength+2] = dfETX;
	
	CString strSumMsg = _T("Send : ");
	for(i =0; i<iLength+3; i++)
	{
		strMsg.Format(_T("%02X"), byteuf[i]); 
		
		strMsg = strMsg + " ";
		strSumMsg = strSumMsg + strMsg;
	}	
	
	byteuf[iLength+3] = 0x00;
	SendMsg(byteuf, iLength+3);
}

void CDlgComm::GetState()
{
	DWORD res = 0;
	UINT i = 0, j = 0;
	UINT uBufMax = 1;
	CString strLog, strMsg;
	BOOL bRet = FALSE;

	if (m_nMode == SENSOR_MODE_H2O )
	{
		// nk.lee 2020.06.15
		if(m_pFFUMon->m_iH2OSensorType == 1)
		{
			uBufMax = 7;

			BYTE byteuf[7];

			byteuf[0] = 0x02;	// STX
			byteuf[1] = 0x30;	//Address "0"
			byteuf[2] = 0x30;	//Address "0"
			byteuf[3] = 0x30;	//Command Head "0"
			byteuf[4] = 0x30;	//Command Head "0"
			byteuf[5] = 0x03;	//ETX
			byteuf[6] = 0x0F;   //BCC

			BYTE byteuf_Sum = 0x00;	// SUM

			for(i=0; i<6; i++)
			{
				byteuf_Sum = byteuf_Sum + byteuf[i];
			}


			byteuf[6] = (byteuf[6] & byteuf_Sum) + 0x30;		//change Dec to ASCII

			for(i=0; i<uBufMax; i++)
			{
				strLog.Format(_T("%02X "), byteuf[i] ); 
				strMsg += strLog;
			}

			strLog = "SEND : ";
			strLog += strMsg;

			m_pFFUMon->WriteLogList(strLog, m_nMode+m_nNum);

			if( api_SerialCom_IsOpened(m_nPort)){
				bRet = api_SerialCom_WriteByte(m_nPort, byteuf, uBufMax);

				if(bRet)
				{
					// jsyoon 20181005 change
					res = WaitForSingleObject(m_hEvtRcv, 2000);	
					if( res == WAIT_TIMEOUT)
					{
						m_nNGCount++;
					}
					else
					{
						m_nNGCount = 0;
					}
				}
				else
				{
					m_nNGCount++;
				}

				if( m_nNGCount > 5)
				{
					m_pFFUMon->m_dH2O[m_nNum] = -1;

					m_nNGCount = 0;
				}
			}
		}	// nk.lee end
		else
		{
			uBufMax = 5;
			BYTE byteuf[5];

			byteuf[0] = 'N';	//
			byteuf[1] = '0';	//
			byteuf[2] = 'T';	//Transmit Value(read)
			byteuf[3] = 'A';
			byteuf[4] = '*';

			for(i=0; i<uBufMax; i++)
			{
				strLog.Format(_T("%c"), byteuf[i] ); 
				strMsg += strLog;
			}

			if( api_SerialCom_IsOpened(m_nPort))
			{
				// jsyoon 20181116 add
				api_SerialCom_ClearReceiveBuffer(m_nPort);
				//////////////////////////////////////////////////////////////////////////

				bRet = api_SerialCom_WriteByte(m_nPort, byteuf, uBufMax);
				if(bRet)
				{
					// jsyoon 20181005 change
					res = WaitForSingleObject(m_hEvtRcv, 2000);	
					if( res == WAIT_TIMEOUT)
					{
						m_nNGCount++;
					}
					else
					{
						m_nNGCount = 0;
					}
				}
				else
				{
					m_nNGCount++;
				}

				if( m_nNGCount > 5)
				{
					m_pFFUMon->m_dH2O[m_nNum] = -1;

					m_nNGCount = 0;
				}
			}
			else
			{
				m_pFFUMon->m_dH2O[m_nNum] = -1;
			}	

			strLog.Format("SEND(%d,%d) : ", m_nPort, bRet);
			strLog += strMsg;

			m_pFFUMon->WriteLogList(strLog, m_nMode+m_nNum);
		}
	}
	else if (m_nMode == SENSOR_MODE_O2)
	{
		BYTE byteuf[3];
		
		byteuf[0] = 'M';	//
		byteuf[1] = '2';	//
		byteuf[2] = '\r';	//

		for(i=0; i<3; i++)
		{
			strLog.Format(_T("%c"), byteuf[i] ); 
			strMsg += strLog;
		}
		
		if( api_SerialCom_IsOpened(m_nPort))
		{
			bRet = api_SerialCom_WriteByte(m_nPort, byteuf, 3);
		}

		strLog.Format("SEND(%d,%d) : ", m_nPort, bRet);
		strLog += strMsg;

		m_pFFUMon->WriteLogList(strLog, m_nMode);
	}
	else if (m_nMode == SENSOR_MODE_TEMP1 )
	{
		uBufMax = 9;

		BYTE byteuf[9];

		byteuf[0] = 0x02;	// STX
		byteuf[1] = 0x30;	//Address "0"
		byteuf[2] = 0x31;	//Address "1"
		byteuf[3] = 0x52;	//Command Head "R"
		byteuf[4] = 0x58;	//Command Head "X"
		byteuf[5] = 0x50;	//Present Value "P"
		byteuf[6] = 0x30;	//Present Value "0"
		byteuf[7] = 0x03;	//ETX
		byteuf[8] = 0x00;   //BCC

		// (01~99)
		int iChannel=0, nMaxChannel = 1;
		nMaxChannel = m_nMaxChannel;
		for (iChannel=0; iChannel < nMaxChannel; iChannel++)
		{
			strMsg.Empty();

			byteuf[8] = 0x00;   //BCC

			if(iChannel == 0)
				byteuf[2] = 0x31;		// 1
			else if(iChannel == 1)
				byteuf[2] = 0x32;
			else if(iChannel == 2)
				byteuf[2] = 0x33;
			else if(iChannel == 3)
				byteuf[2] = 0x34;	
			else if(iChannel == 4)
				byteuf[2] = 0x35;
			else if(iChannel == 5)
				byteuf[2] = 0x36;
			else if(iChannel == 6)
				byteuf[2] = 0x37;	
			else if(iChannel == 7)
				byteuf[2] = 0x38;
			else if(iChannel == 8)
				byteuf[2] = 0x39;
			else if(iChannel == 9)
				byteuf[2] = 0x3A;

			for(i=0; i<=7; i++)
			{
				byteuf[8] = 	byteuf[8]^ byteuf[i];
			}

			if( api_SerialCom_IsOpened(m_nPort))
			{
				// jsyoon 20181116 add
				api_SerialCom_ClearReceiveBuffer(m_nPort);
				//////////////////////////////////////////////////////////////////////////

				bRet = api_SerialCom_WriteByte(m_nPort, byteuf, uBufMax);
				if( bRet)
				{
					res = WaitForSingleObject(m_hEvtRcv, 3000);	
					if( res == WAIT_TIMEOUT)
					{
						m_nNGCount++;
					}
					else
					{
						m_nNGCount = 0;
					}
				}
				else
				{
					m_nNGCount++;
				}
			}

	//		RcvThermTest();		// jsyoon 20180904 for Test

			if( m_nNGCount > 5)
			{
				m_pFFUMon->m_dThermometer[m_nNum][iChannel] = -1;

				m_nNGCount = 0;
			}

			for(i=0; i<uBufMax; i++)
			{
				strLog.Format(_T("%02X "), byteuf[i] ); 
				strMsg += strLog;
			}

			strLog.Format("SEND(%d,%d) : ", m_nPort, bRet);
			strLog += strMsg;

			m_pFFUMon->WriteLogList(strLog, m_nMode+m_nNum);
		}
	}
	else if (m_nMode == SENSOR_MODE_PRESSURE)
	{
		BYTE byteuf[7];
		BYTE byteCheckSum = 0x00;
		BYTE bin[4];
		unsigned short CMD = 0;
		UINT nLen = 0;
		CString str;
		CString strBCC;
		char* arr;

		//ksk 20150608 change
		memset(&byteuf,0,sizeof(BYTE) * 7);
		memset(&bin,0,sizeof(BYTE) * 4);
		memset(&CMD,0,sizeof(short) * 1);
		
		byteuf[0] = 0x02;	//STX
		byteuf[1] = 0x30;	//MNH'0'
		byteuf[2] = 0x30;	//MNL'0'
		m_pFFUMon->GetDlgItemText(IDC_EDIT_CMD,str);
		arr = LPSTR(LPCTSTR(str));
		str.Format("%02X%02X",arr[0],arr[1]);
		nLen = str.GetLength();
		for (i=0; i<nLen; i++)
		{
			bin[i] = HEXtoBYTE(str[(nLen-1)-i]);
			if (bin[i]>=0 && bin[i]<=15)
				CMD += bin[i]<<(4*i);
		}
		CMD = htons(CMD);
		memcpy(&byteuf[3],&CMD,sizeof(CMD));
		byteuf[5] = 0x03;	//ETX

		for(i=0; i<6; i++)
		{
			byteCheckSum += byteuf[i];
		}
		byteCheckSum = byteCheckSum & 0x0f;
		str.Format("%d",byteCheckSum);
		arr = LPSTR(LPCTSTR(str));
		strBCC.Format("%02X",arr[0]);
		nLen = strBCC.GetLength();
		for (i=0; i<nLen; i++)
		{
			bin[i] = HEXtoBYTE(strBCC[(nLen-1)-i]);
			if (bin[i]>=0 && bin[i]<=15)
				byteuf[6] += bin[i]<<(4*i);
		}
		
		for(i=0; i<7; i++)
		{
			strLog.Format(_T("%02X "), byteuf[i] ); 
			strMsg += strLog;
		}
		
		strLog = "SEND : ";
		strLog += strMsg;
		
		m_pFFUMon->WriteLogList(strLog, m_nMode);
		
		if( api_SerialCom_IsOpened(m_nPort))
			api_SerialCom_WriteByte(m_nPort, byteuf, 7);
	}
	else if (m_nMode == SENSOR_MODE_EFU || m_nMode == SENSOR_MODE_EFU2 || m_nMode == SENSOR_MODE_EFU3 || m_nMode == SENSOR_MODE_EFU4 || m_nMode == SENSOR_MODE_EFU5)
	{
		BYTE byteuf[9];
		BYTE bytefuEBM[4];		//20181116 edhan add
		ZeroMemory(bytefuEBM,sizeof(bytefuEBM));

		if (m_pFFUMon->m_bEFU_Type == EFU_TYPE_SHINSUNG)	//yhjoo 20160704 add (TRUE = SinSung, FALSE = ISU)
		{	
// 			WORD byteCheckSum = 0;
// 			unsigned short nSpeed = 0;
// 			unsigned short nCRC = 0;
			
			// yhjoo 20150128 add
			byteuf[0] = 0x02;
			byteuf[1] = 0x8A;
			byteuf[2] = 0x87;
			byteuf[3] = 0x81;
			byteuf[4] = 0x9F;
			byteuf[5] = 0x01;
			byteuf[6] = 0x20;
			byteuf[7] = 0x52;
			byteuf[8] = 0x03;
				
			if( api_SerialCom_IsOpened(m_nPort))
				api_SerialCom_WriteByte(m_nPort, byteuf, 9);

			for(i=0; i < 9; i++)
			{
				strLog.Format(_T("%02X "), byteuf[i] ); 
				strMsg += strLog;
			}
			
			strLog = "SEND : ";
			strLog += strMsg;

			m_pFFUMon->WriteLogList(strLog, m_nMode);
		}
		else if (m_pFFUMon->m_bEFU_Type == EFU_TYPE_ISU)
		{
//			BYTE byteCheckSum = 0x00;
//			WORD nSpeed = 0;
			WORD nCRC = 0;
			
			for ( j=0; j<m_pFFUMon->m_nEFUCnt[0]; j++)	//20150604 Jay Hong
			{
// 				nSpeed = m_pFFUMon->m_uEFUSpeed[j];
// 				nSpeed = htons(nSpeed);

				byteuf[0] = 0xFD;
				byteuf[1] = 0x01;	// j+1
//				byteuf[2] = j+1;
				byteuf[2] = (BYTE)j+1;
				byteuf[3] = 0xFE;	//상태조회 : 0xFE, 속도 변경 명령 : 0x96
	//			byteuf[3] = 0x96;	//상태조회 : 0xFE, 속도 변경 명령 : 0x96
	//			memcpy(&byteuf[4],&nSpeed,sizeof(nSpeed));
				byteuf[4] = 0x00;
				byteuf[5] = 0x00;
				byteuf[6] = 0xFE;
				byteuf[7] = 0x00;
				byteuf[8] = 0x00;
				
				// jsyoon 20150603 change
				#define CRC_SEED 0x0000
				nCRC = CRC_SEED;
				
				for(i=0; i<7; i++)
				{
					//byteCheckSum += byteuf[i];				//20150604 Jay Hong
					nCRC = CRC_COMPUTE(nCRC, byteuf[i]);	//20150604 Jay Hong
				}
				
				nCRC = htons(nCRC);
				memcpy(&byteuf[7],&nCRC,sizeof(nCRC));
				
				// jsyoon 20150602 change
				if( api_SerialCom_IsOpened(m_nPort))	
					api_SerialCom_WriteByte(m_nPort, byteuf, 9);
				
				strMsg.Empty();

				for(i=0; i<9; i++)
				{
					strLog.Format(_T("%02X "), byteuf[i] ); 
					strMsg += strLog;
				}
				
				strLog = "SEND : ";
				strLog += strMsg;
				
				m_pFFUMon->WriteLogList(strLog, m_nMode);
				
				//Sleep(3000);
				Sleep(1000);//20150618 changed
//				Sleep(200);//20150618 changed
			}
		}
		//20181116 edhan add
		else if (m_pFFUMon->m_bEFU_Type == EFU_TYPE_EBM)
		{
			BYTE bytChkSum;
			for ( j=0; j<m_pFFUMon->m_nEFUCnt[m_nNum]; j++)
			{
				//Get Speed 명령은 4개 Byte사용
				bytefuEBM[0] = 0x15;				// Preamble		//0x35
				bytefuEBM[1] = 0x21 + (BYTE)j;		// Command byte //0x21 = Read Speed 조건으로 1번쨰 Efu
				bytefuEBM[2] = 0x01;				// fan group address //1번째 그룹?

				bytChkSum  = bytefuEBM[0]^bytefuEBM[1];
				bytChkSum ^= bytefuEBM[2];
				bytChkSum ^= 0xFF;

				bytefuEBM[3] = bytChkSum;				// checksum		//0xB8
				
				if( api_SerialCom_IsOpened(m_nPort))
				{
					bRet = api_SerialCom_WriteByte(m_nPort, bytefuEBM, 4);
					if( bRet)
					{
						res = WaitForSingleObject(m_hEvtRcv, 3000);	
						if( res == WAIT_TIMEOUT)
						{
							m_nNGCount++;
						}
						else
						{
							m_nNGCount = 0;
						}
					}
					else
					{
						m_nNGCount++;
					}
				}

				strMsg.Empty();

				for(i=0; i < 4; i++)
				{
					strLog.Format(_T("%02X "), bytefuEBM[i] ); 
					strMsg += strLog;
				}

				strLog.Format("SEND(%d,%d) : ", m_nPort,bRet);
				strLog += strMsg;

				m_pFFUMon->WriteLogList(strLog, m_nMode);
			}
		}
		else if(m_pFFUMon->m_bEFU_Type ==  EFU_TYPE_MAYAIR)
		{
			BYTE byteEFU[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};	

			int nSize		= 8;
			int idxCRC		= 6;

			// Slave & Function code
			byteEFU[0] = 0x01;	// Slave Address 		
			byteEFU[1] = 0x03;	// Function code :03 (Read holding register)			

			// Start Address
			byteEFU[2] = 0x00;	// High-order address				
			byteEFU[3] = 0x00;	// Low-order address

			// The number of register content to read
			byteEFU[4] = 0x00;
			byteEFU[5] = 0x65;	// Read Address from start to End (65 is 101 in HEX)

			// CRC
			byteEFU[6] = 0x00;	// Low-order CRC 
			byteEFU[7] = 0x00;	// High-order CRC  

			GetCRC(idxCRC, byteEFU);	// 0xE185 : byteEFU[6] = 0x85, byteEFU[7] = 0xE1	

			if( api_SerialCom_IsOpened(m_nPort))
				api_SerialCom_WriteByte(m_nPort, byteEFU, nSize);

			for(int i=0; i<nSize; i++)
			{
				strLog.Format(_T("%02X "), byteEFU[i]); 
				strMsg += strLog;
			}

			strLog = "SEND : ";
			strLog += strMsg;

			m_pFFUMon->WriteLogList(strLog, m_nMode);
		}
	 	else if (m_pFFUMon->m_bEFU_Type == EFU_TYPE_HKC_H5)	// jsyoon 20201104 add
		{
			TestSendEFU(0, 1, -1, -1, 2);
			res = WaitForSingleObject(m_hEvtRcv, 2000);	
			if( res == WAIT_OBJECT_0)
			{
				TestSendEFU(0, 1, -1, -1, 0);
				res = WaitForSingleObject(m_hEvtRcv, 2000);	
			}

/*
			unsigned short crc16;
			BYTE byteEFU[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};	

			int nSize		= 8;

			// Get Speed
			byteEFU[0] = 0x01;	   // 유닛 아이디  
			byteEFU[1] = 0x03;     // Read Multiple Registers

			byteEFU[2] = 0x10;     // Status 1001~ // Real EFU SPEED 10FF ~  // Target EFU SPEED 12FB ~
			byteEFU[3] = 0x20; 
			byteEFU[4] = 0x00; 
			byteEFU[5] = 0x09; 

//			byteEFU[2] = 0x12;     // Status 1001~ // Real EFU SPEED 10FF ~  // Target EFU SPEED 12FB ~
//			byteEFU[3] = 0xFB; 
//			byteEFU[4] = 0x00; 
//			byteEFU[5] = 0x01; 

			crc16 = CRC16_MODBUS(byteEFU, 6); 
			byteEFU[6] = (unsigned char)((crc16>>8) & 0x00ff);  //CRC 
			byteEFU[7] = (unsigned char)((crc16>>0) & 0x00ff); 

			m_nSendType = 0;

			if( api_SerialCom_IsOpened(m_nPort))
				api_SerialCom_WriteByte(m_nPort, byteEFU, nSize);

			for(int i=0; i<nSize; i++)
			{
				strLog.Format(_T("%02X "), byteEFU[i]); 
				strMsg += strLog;
			}

			strLog = "SEND : ";
			strLog += strMsg;

			m_pFFUMon->WriteLogList(strLog, m_nMode);
*/			

// 			// Status
// 			byteEFU[0] = 0x01;	   // 유닛 아이디  
// 			byteEFU[1] = 0x03;     // Read Multiple Registers
// 			byteEFU[2] = 0x10;     // Status 1001~ // Real EFU SPEED 10FF ~  // Target EFU SPEED 12FB ~
// 			byteEFU[3] = 0x01; 
// 			byteEFU[4] = 0x00; 
// 			byteEFU[5] = 0x01; 
// 
// 			crc16 = CRC16_MODBUS(byteEFU, 6); 
// 			byteEFU[6] = (unsigned char)((crc16>>8) & 0x00ff);  //CRC 
// 			byteEFU[7] = (unsigned char)((crc16>>0) & 0x00ff); 
// 
// 			if( api_SerialCom_IsOpened(m_nPort))
// 				api_SerialCom_WriteByte(m_nPort, byteEFU, nSize);
// 
// 			for(int i=0; i<nSize; i++)
// 			{
// 				strLog.Format(_T("%02X "), byteEFU[i]); 
// 				strMsg += strLog;
// 			}
// 
// 			strLog = "SEND : ";
// 			strLog += strMsg;
// 
// 			m_pFFUMon->WriteLogList(strLog, m_nMode);
		}
		//
	}
//	else if (m_nMode == SENSOR_MODE_EFU2)
//	{
//		BYTE byteuf[9];
//		BYTE bytefuEBM[4];
//		ZeroMemory(bytefuEBM,sizeof(bytefuEBM));
//
//		if (m_pFFUMon->m_bEFU_Type == 1)	//yhjoo 20160704 add (TRUE = SinSung, FALSE = ISU)
//		{	
//			// 			WORD byteCheckSum = 0;
//			// 			unsigned short nSpeed = 0;
//			// 			unsigned short nCRC = 0;
//
//			// yhjoo 20150128 add
//			byteuf[0] = 0x02;
//			byteuf[1] = 0x8A;
//			byteuf[2] = 0x87;
//			byteuf[3] = 0x81;
//			byteuf[4] = 0x9F;
//			byteuf[5] = 0x01;
//			byteuf[6] = 0x20;
//			byteuf[7] = 0x52;
//			byteuf[8] = 0x03;
//
//			if( api_SerialCom_IsOpened(m_nPort))
//				api_SerialCom_WriteByte(m_nPort, byteuf, 9);
//
//			for(i=0; i < 9; i++)
//			{
//				strLog.Format(_T("%02X "), byteuf[i] ); 
//				strMsg += strLog;
//			}
//
//			strLog = "SEND : ";
//			strLog += strMsg;
//
//			m_pFFUMon->WriteLogList(strLog, m_nMode);
//		}
//		else if (m_pFFUMon->m_bEFU_Type == 0)
//		{
//			BYTE byteCheckSum = 0x00;
//			//			WORD nSpeed = 0;
//			WORD nCRC = 0;
//
//			//20161201
//			// 			int nMin = 32;
//			// 			if( nMin < m_pFFUMon->m_nEFUCnt)
//			// 				nMin = m_pFFUMon->m_nEFUCnt - 32;
//
//			for ( j=0; j<m_pFFUMon->m_nEFUCnt[1]; j++)	//20150604 Jay Hong
//			{
//				// 				nSpeed = m_pFFUMon->m_uEFUSpeed[j];
//				// 				nSpeed = htons(nSpeed);
//
//				byteuf[0] = 0xFD;
//				byteuf[1] = 0x01;	// j+1
//				byteuf[2] = j+1;
//				byteuf[3] = 0xFE;	//상태조회 : 0xFE, 속도 변경 명령 : 0x96
//				//			byteuf[3] = 0x96;	//상태조회 : 0xFE, 속도 변경 명령 : 0x96
//				//			memcpy(&byteuf[4],&nSpeed,sizeof(nSpeed));
//				byteuf[4] = 0x00;
//				byteuf[5] = 0x00;
//				byteuf[6] = 0xFE;
//				byteuf[7] = 0x00;
//				byteuf[8] = 0x00;
//
//				// jsyoon 20150603 change
//				#define CRC_SEED 0x0000
//				nCRC = CRC_SEED;
//
//				for(i=0; i<7; i++)
//				{
//					//byteCheckSum += byteuf[i];				//20150604 Jay Hong
//					nCRC = CRC_COMPUTE(nCRC, byteuf[i]);	//20150604 Jay Hong
//				}
//
//				nCRC = htons(nCRC);
//				memcpy(&byteuf[7],&nCRC,sizeof(nCRC));
//
//				// jsyoon 20150602 change
//				if( api_SerialCom_IsOpened(m_nPort))	
//					api_SerialCom_WriteByte(m_nPort, byteuf, 9);
//
//				strMsg.Empty();
//
//				for(i=0; i<9; i++)
//				{
//					strLog.Format(_T("%02X "), byteuf[i] ); 
//					strMsg += strLog;
//				}
//
//				strLog = "SEND : ";
//				strLog += strMsg;
//
//				m_pFFUMon->WriteLogList(strLog, m_nMode);
//
//				//Sleep(3000);
//				Sleep(1000);//20150618 changed
//				//				Sleep(200);//20150618 changed
//			}
//		}
//		//20181116 edhan add
//		else if (m_pFFUMon->m_bEFU_Type == 2)
//		{
//			BYTE bytChkSum;
//			for ( j=0; j<m_pFFUMon->m_nEFUCnt[1]; j++)
//			{
//				//Get Speed 명령은 4개 Byte사용
//				bytefuEBM[0] = 0x15;				// Preamble
//				bytefuEBM[1] = 0x21 + (BYTE)j;		// Command byte //0x21 = Read Speed 조건으로 1번쨰 Efu
//				bytefuEBM[2] = 0x01;				// fan group address //1번째 그룹?
//
//				bytChkSum  = bytefuEBM[0]^bytefuEBM[1];
//				bytChkSum ^= bytefuEBM[2];
//				bytChkSum ^= 0xFF;
//
//				bytefuEBM[3] = bytChkSum;				// checksum		//0xB8
//
//				if( api_SerialCom_IsOpened(m_nPort))
//				{
//					bRet = api_SerialCom_WriteByte(m_nPort, bytefuEBM, 4);
//					if( bRet)
//					{
//						res = WaitForSingleObject(m_hEvtRcv, 3000);	
//						if( res == WAIT_TIMEOUT)
//						{
//							m_nNGCount++;
//						}
//						else
//						{
//							m_nNGCount = 0;
//						}
//					}
//					else
//					{
//						m_nNGCount++;
//					}
//				}
//
//// 				if( api_SerialCom_IsOpened(m_nPort))
//// 				{
//// 					bRet = api_SerialCom_WriteByte(m_nPort, bytefuEBM, 4);
//// 				}
//
//				strMsg.Empty();
//
//				for(i=0; i < 4; i++)
//				{
//					strLog.Format(_T("%02X "), bytefuEBM[i] ); 
//					strMsg += strLog;
//				}
//
//				strLog.Format("SEND(%d) : ", bRet);
//				strLog += strMsg;
//
//				m_pFFUMon->WriteLogList(strLog, m_nMode);
//			}
//		}
//		//
//	}
	else if( m_nMode ==  ELECTRONIC_DISPLAY)
	{
//		if (m_pFFUMon->m_bUse_LED)
		{
	//		if( fabs(m_pFFUMon->m_dO2 - m_pFFUMon->m_dOldO2) > 0.1)
			if( fabs(m_pFFUMon->m_dO2 - m_pFFUMon->m_dOldO2) > 0.001)	// sih 20151022 change 
			{
				if( m_pFFUMon->m_dO2 > 10000 )
					SendToLed(m_nPort, 1, m_pFFUMon->m_dO2);
				else
					SendToLed(m_nPort, 0, m_pFFUMon->m_dO2);
				
				m_pFFUMon->m_dOldO2 = m_pFFUMon->m_dO2;
			}
		}
	}
	else if(m_nMode == SENSOR_MODE_KMN2)		//jhseo 20170920 add for KM-N2
	{
		uBufMax = 24;
		
		BYTE byteuf[24];		//Module 1
		BYTE byteuf2[24];		//Module 2
		
		byteuf[0] = 0x02;	//STX
		
		byteuf[1] = 0x30;	//Comm. Address 1
		byteuf[2] = 0x31;	//Comm. Address 2
		byteuf[3] = 0x30;	//Sub Address 1(Fix 0)
		byteuf[4] = 0x30;	//Sub Address 2(Fix 0)
		byteuf[5] = 0x30;	//SID(Fix 0)
		
		byteuf[6] = 0x30;	//MRC1
		byteuf[7] = 0x31;	//MRC2
		byteuf[8] = 0x30;	//SRC1
		byteuf[9] = 0x31;	//MRC1
		
		byteuf[10] = 0x43;	//OPCODE1	C
		byteuf[11] = 0x30;	//OPCODE2	0					C0 -> Get Measure Data Operation Code
		
		byteuf[12] = 0x30;	//Start Address(Reference Address Map)
		byteuf[13] = 0x31;
		byteuf[14] = 0x30;
		byteuf[15] = 0x30;
		
		byteuf[16] = 0x30;	//Bit Position1(Fix 0)
		byteuf[17] = 0x30;	//Bit Position2(Fix 0)
		
		byteuf[18] = 0x30;	//Data
		byteuf[19] = 0x30;
		byteuf[20] = 0x30;
		byteuf[21] = 0x31;
		
		byteuf[22] = 0x03;	//ETX
		
		byteuf[23] = 0x00;	//BCC

		for(i=0; i < 24; i++)
		{
			byteuf2[i] = byteuf[i];
		}
		byteuf2[2] = 0x35;
		
		//Calc BCC
		for(i=1; i < 23; i++)
		{
			byteuf[23] = byteuf[23] ^ (byteuf[i]);
		}

		for(i=1; i < 23; i++)
		{
			byteuf2[23] = byteuf2[23] ^ (byteuf2[i]);
		}
		//////////////////////////////////////////////////////////////////////////
	
		strMsg += "Module 1 ";
		for(i=0; i<uBufMax; i++)
		{
			strLog.Format(_T("%02X "), byteuf[i] ); 
			strMsg += strLog;
		}

 		strMsg += "Module 2 ";
		for(i=0; i<uBufMax; i++)
		{
			strLog.Format(_T("%02X "), byteuf2[i] ); 
			strMsg += strLog;
		}
		
		strLog = "SEND : ";
		strLog += strMsg;
		
		m_pFFUMon->WriteLogList(strLog, m_nMode);
		
		if( api_SerialCom_IsOpened(m_nPort))
		{
			api_SerialCom_WriteByte(m_nPort, byteuf, uBufMax);
			Sleep(100);
			api_SerialCom_WriteByte(m_nPort, byteuf2, uBufMax);
			Sleep(100);
		}
	}
	else if( m_nMode ==  SENSOR_MODE_ACCURA2)
	{
		BYTE byteEFU[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};	

		int nSize		= 8;
		int idxCRC		= 6;

		// Slave & Function code
		for (i=0; i<m_pFFUMon->m_bCnt_ACCURA; i++)	
		{
			ZeroMemory(&m_pFFUMon->m_sData, sizeof(m_pFFUMon->m_sData));

			m_Validity = 0;
			for( j=0; j < 40; j++)
			{
				m_subunitValidity[j] = -1;
			}

			GetDataFetch(i);		// Fetch Check

			if( m_Validity == 1)
			{
				GetDataVoltage(i);	
			}
			
			for (j=0; j<40; j++)
			{
				if( m_subunitValidity[j] == j )
					GetDataSub(i, j);
			}

			m_pFFUMon->m_pAccuraData[i] = m_pFFUMon->m_sData;
		}	
	}
	else if( m_nMode ==  SENSOR_MODE_AZBIL)
	{
		Azbil0150_WriteRead();
	}
	else if( m_nMode ==  SENSOR_MODE_PM5500)
	{
		PM5500_WriteRead();
	}
	else if( m_nMode ==  SENSOR_MODE_IONIZER || m_nMode ==  SENSOR_MODE_IONIZER2)
	{
		Ionizor_WriteRead();
	}
}

UINT CDlgComm::WorkThreadProcGetState(LPVOID pParam)	//test
{
	CDlgComm *pProc = (CDlgComm*) pParam;
	volatile BOOL bLoop  = TRUE;
	while(bLoop)
	{
		DWORD res = WaitForSingleObject(pProc->m_hEvtGetStateStop, 1000);
		switch(res)
		{
		case WAIT_OBJECT_0:
			bLoop = FALSE;
			break;
		case WAIT_TIMEOUT:
			pProc->GetState();
			break;
		}
	}
	
	pProc->m_pGetStateThread = NULL;
	return TRUE;
}

UINT CDlgComm::WorkThreadProcMonitoring(LPVOID pParam)
{
	CDlgComm *pProc = (CDlgComm*) pParam;
	volatile BOOL bLoop  = TRUE;
	BOOL bAllOff = FALSE;
	BOOL bRestore = FALSE;
	while(bLoop)
	{
		DWORD res = WaitForSingleObject(pProc->m_hEvtMonitoringStop, 1000);
		switch(res)
		{
		case WAIT_OBJECT_0:
			bLoop = FALSE;
			break;
		case WAIT_TIMEOUT:
			bAllOff = API_SM_GetFFU_ALLOFF(pProc->m_pFFUMon->m_nIdex);
			if(bAllOff)
			{
				pProc->m_pFFUMon->WriteLogList(_T("=== All Off TRUE ==="), pProc->m_nMode);
				
				SetEvent(pProc->m_hEvtGetStateStop);
				Sleep(1000);
				
				pProc->AllOff();
				
				Sleep(1000);
				pProc->StartThreadGetState();
			}
			
			bRestore = API_SM_GetFFU_RESTORE(pProc->m_pFFUMon->m_nIdex);
			if(bRestore)
			{
				pProc->m_pFFUMon->WriteLogList(_T("=== Restore Off TRUE ==="), pProc->m_nMode);
				
				SetEvent(pProc->m_hEvtGetStateStop);
				Sleep(1000);
				
				pProc->SetRestoreEFUValue();
				
				Sleep(1000);
				pProc->StartThreadGetState();
			}
			
			API_SM_SetFFU_ALLOFF(FALSE, pProc->m_pFFUMon->m_nIdex);		// jsyoon 20140708 add
			API_SM_SetFFU_RESTORE(FALSE, pProc->m_pFFUMon->m_nIdex);
		}
	}
	
	pProc->m_pMonitoringThread = NULL;
	return TRUE;
}

void CDlgComm::StartThreadGetState()
{
	if(!m_pGetStateThread)
	{
		m_hEvtGetStateStop = CreateEvent(NULL, FALSE, FALSE, NULL);
		m_pGetStateThread = AfxBeginThread(WorkThreadProcGetState, this);
		m_pGetStateThread->m_bAutoDelete = FALSE;
	}
}

UINT CDlgComm::WorkThreadProcRestoreValue(LPVOID pParam)
{
	CDlgComm* pProc = (CDlgComm*)pParam;
	
	SetEvent(pProc->m_hEvtGetStateStop);
	Sleep(1000);
	
	pProc->SetRestoreEFUValue();
	
	Sleep(1000);
	pProc->StartThreadGetState();
	
	return 1;
}

void CDlgComm::SetRestoreEFUValue()
{
	// 	for(int i=0;i<2;i++)	//혹시 몰라서 2번
	// 	{

	int nID = 0, nStart = 0;
	if (m_nMode == SENSOR_MODE_EFU)
	{
		nID = 0;
		nStart = 0;
	}
	else
	{
		nID = 1;
		nStart = 32;
	}

	UINT nEfu=0;

	if (m_pFFUMon->m_bEFU_Type == 2)
	{
		for( nEfu=0;nEfu<=m_pFFUMon->m_nEFUCnt[nID];nEfu++)
		{
			if( m_pFFUMon->m_uEFUSpeed[nStart+nEfu] <= m_pFFUMon->m_uEFUSpeedMax[nStart+nEfu] )	// jsyoon 20140811
			{
				SetEFUSpeed(m_pFFUMon->m_uEFUSpeed[nStart+nEfu], 1, nEfu+1, nEfu+1);
			}
		}
	}
	else
	{
		for( nEfu=0;nEfu<=m_pFFUMon->m_nEFUCnt[nID];nEfu++)
		{
			if( m_pFFUMon->m_uEFUSpeed[nStart+nEfu] <= m_pFFUMon->m_uEFUSpeedMax[nStart+nEfu] )	// jsyoon 20140811
			{
				SetEFUValue(nEfu+1, m_pFFUMon->m_uEFUSpeed[nStart+nEfu]);
				Sleep(500);
			}
		}
	}
	
	//	}
}

UINT CDlgComm::WorkThreadProcAllOff(LPVOID pParam)
{
	CDlgComm* pProc = (CDlgComm*)pParam;
	
	SetEvent(pProc->m_hEvtGetStateStop);
	Sleep(1000);
	
	pProc->AllOff();
	
	Sleep(1000);
	pProc->StartThreadGetState();
	
	return 1;
}

void CDlgComm::AllOff()
{
	// 	for(int i=0;i<2;i++)	//혹시 몰라서 2번
	// 	{
//  	for(UINT nEfu=0;nEfu<m_pFFUMon->m_nEFUCnt;nEfu++)
// 	{
// 		SetEFUValue(nEfu+1, 1);
// 		Sleep(500);
// 	}
	//	}

	int nID = 0, nStart = 0;
	if (m_nMode == SENSOR_MODE_EFU)
	{
		nID = 0;
	}
//	else
//	{
//		nID = 1;
//	}
	//20181217 edhan add
	else if (m_nMode == SENSOR_MODE_EFU2)
	{
		nID = 1;
	}
	else if (m_nMode == SENSOR_MODE_EFU3)
	{
		nID = 2;
	}
	else if (m_nMode == SENSOR_MODE_EFU4)
	{
		nID = 3;
	}
	else if (m_nMode == SENSOR_MODE_EFU5)
	{
		nID = 4;
	}
	//

	UINT nEfu=1;

	if (m_pFFUMon->m_bEFU_Type == 2)
	{
		SetEFUSpeed(1, 1, nEfu, m_pFFUMon->m_nEFUCnt[nID]);
	}
	else
	{
		for( nEfu=1;nEfu<=m_pFFUMon->m_nEFUCnt[nID];nEfu++)
		{
			SetEFUValue(nEfu, 1);

			Sleep(500);
		}
	}
}

void CDlgComm::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
// 	SetEvent(m_hEvtGetStateStop);
// 	SetEvent(m_hEvtMonitoringStop);
	m_pFFUMon->WriteLogList(_T("Close!"), m_nMode);
	api_SerialCom_Close(m_nPort);
	ExitThreadAll();

	SAFE_DELETE(m_pRcvThread);

	CloseHandle(m_hEvtRcvStop);
	CloseHandle(m_hEvtGetStateStop);
	CloseHandle(m_hEvtMonitoringStop);
	CloseHandle(m_hEvtRcv);
	
	CDialog::OnClose();
}

void CDlgComm::AllStop()
{
	SetEvent(m_hEvtGetStateStop);
}

void CDlgComm::RestoreValue()
{
	AfxBeginThread(WorkThreadProcRestoreValue, this);
}

void CDlgComm::SetAllOff()
{
	AfxBeginThread(WorkThreadProcAllOff, this);
}

void CDlgComm::SetEFUData(vector<BYTE> vecRvcData, int nMode)
{
// 	FFU_DATA stEFUData;

	int iMsgSize = 0, i =0;
	CString strHEX = _T("");
	BOOL bSTX = FALSE, bETX = FALSE ;
	BOOL bINFOMODE1 = FALSE, bINFOMODE2 = FALSE, bLV32_ID = FALSE, bDPU_ID = FALSE, bResponse = FALSE;
	BOOL bRPMMODE1 = FALSE, bRPMMODE2 = FALSE, bRPMFlagOK = FALSE;

	CString strLog = _T("");
	int iPV =0, iSV =0;
//	int iPV =0, iALARM = 0, iSV =0;
	int iEFUDataStart = 0;
	int iEDUCnt = 0;
	int iEFUNum = 0;
	UINT iEFUID = 0;
//	BOOL bAlarm = FALSE;

	BYTE byteCheckSum = 0x00;
//	unsigned short nCRC = 0;
	unsigned short nCRC_Check = 0;
	unsigned short nVal = 0;
	int nNum = 0;

	int iEFU = 0;
	int nId = 0;
	UINT nCnt = 0;
	double dValue = 0.;
	//20181207 edhan change
	if(nMode == SENSOR_MODE_EFU)	
	{
		nId = 0;
	}
	else if(nMode == SENSOR_MODE_EFU2)
	{
		nId = 1;
	}
	else if(nMode == SENSOR_MODE_EFU3)
	{
		nId = 2;
	}
	else if(nMode == SENSOR_MODE_EFU4)
	{
		nId = 3;
	}
	else if(nMode == SENSOR_MODE_EFU5)
	{
		nId = 4;
	}
	//

	if (m_pFFUMon->m_bEFU_Type == EFU_TYPE_SHINSUNG)	//yhjoo 20160704 add (TRUE = SinSung, FALSE = ISU)
	{
		iMsgSize = vecRvcData.size();

//		if(iMsgSize > 2)
		if(iMsgSize > 5) // mn.bong 20180710 change 2->5
		{
			if(vecRvcData[0]  == dfSTX)
			{
				bSTX = TRUE;
			}

			if(vecRvcData[1]  == dfINFOMODE1)
			{
				bINFOMODE1 = TRUE;
			}

			if(vecRvcData[2]  == dfINFOMODE2)
			{
				bINFOMODE2 = TRUE;
			}

			if(vecRvcData[3]  == dfLV32_ID)
			{
				bLV32_ID = TRUE;
			}

			if(vecRvcData[4]  == dfDPU_ID)
			{
				bDPU_ID = TRUE;
			}
			
			if(vecRvcData[iMsgSize-1]  == dfETX)
			{
				bETX = TRUE;
			}

			if(bSTX && bETX && bINFOMODE1 && bINFOMODE2 && bLV32_ID && bDPU_ID)
			{
				bResponse = TRUE;
			}
		}

		/*yhjoo 20150128 remove
		if(bResponse && iMsgSize == 9)
		{
	// 		iEFUDataStart = 5;
			for (int i=0; i<iMsgSize-2; i++)
			{
				byteCheckSum += vecRvcData[i];
			}

			unsigned char cCRC[7] = {0,};
			memcpy(&cCRC, &vecRvcData[0], 7);

			nCRC = m_pFFUMon->GetCRC16(cCRC,7);/*CRC_COMPUTE(cCRC, 7);	//yhjoo 20141209 ?????? m_dEFU
			nCRC = htons(nCRC);

			memcpy(&nCRC_Check, &vecRvcData[7], sizeof(nCRC_Check));
			nCRC_Check = htons(nCRC_Check);

			if (nCRC == nCRC_Check)
			{
				memcpy(&nVal, &vecRvcData[4], sizeof(nVal));
				nVal = htons(nVal);
				nNum = vecRvcData[1];
				m_pFFUMon->m_dEFU[nNum-1] = (double)nVal;
			}
		}
		*/

		if(bResponse)
		{
			iEFUDataStart = 5;
		}
 
 		iEDUCnt = (iMsgSize - 7) / 4 ;
 
 		if(vecRvcData[0]  == dfSTX)
 		{
 			bSTX = TRUE;
 		}
 		if(bSTX && vecRvcData[1]  == dfINFOMODE1)
 		{
 			bINFOMODE1 = TRUE;
 		}
 		if(bINFOMODE1 && vecRvcData[2]  == dfINFOMODE2)
 		{
 			bINFOMODE2 = TRUE;
 		}
 		if(bSTX && vecRvcData[1]  == dfRPMMODE1)
 		{
 			bRPMMODE1 = TRUE;
 		}
 		if(bRPMMODE1 && vecRvcData[2]  == dfRPMMODE2)
 		{
 			bRPMMODE2 = TRUE;
 		}
 		if(bRPMMODE2 && vecRvcData[iMsgSize - 2]  == dfRPMFlagOK)
 		{
 			bRPMFlagOK = TRUE;
 		}
 		if((bINFOMODE2 || bRPMMODE2) && vecRvcData[4]  == dfDPU_ID)
 		{
 			bDPU_ID = TRUE;
 			iEFUDataStart = 5;
 		}

 		for(i =0; i < iMsgSize; i++)
 		{
 			strHEX.Format(_T("%02X"), vecRvcData[i] );
 
 			//Success Start Bit 
 			if(bDPU_ID && bINFOMODE1)
 			{
 				if(i >= iEFUDataStart)
 				{
 					iEFUNum ++;
 
 					iEFUID = m_pFFUMon->GetByteToIDec(vecRvcData[i]);
 					iEFUID -= 128;
 
 					if(iEFUID == 31)
 					{
 						iEFUID = iEFUID;
 					}

					nCnt = m_pFFUMon->m_nEFUCnt[nId];
 
 					if( iEFUID > 0  && nCnt >= iEFUID)
 					{
 						iPV = m_pFFUMon->GetByteToIDec(vecRvcData[i+1]);
	// 					API_SM_SetFFU_PV(iEFUID, iPV);
 						
	// 					iALARM =m_pFFUMon->GetByteToIDec(vecRvcData[i+2]);
	// 					API_SM_SetFFU_ALARM(iEFUID, iALARM);
 						
 						iSV = m_pFFUMon->GetByteToIDec(vecRvcData[i+3]);
	// 					API_SM_SetFFU_SV(iEFUID, iSV);
 
 						strLog.Format(_T("=== ID(%d) PV(%d) SV(%d)==="), iEFUID, iPV, iSV);
//						m_pFFUMon->m_dEFU[32*nId+iEFUID-1] = (double)iPV * 10.0;	//iSV??
//						m_pFFUMon->SendEFUValue(iEFUID, m_pFFUMon->m_dEFU[iEFUID-1]);	//yhjoo 20160704 add
						dValue = (double)iPV * 10.0;	//iSV??


						iEFU = 32*nId+iEFUID-1;
						m_pFFUMon->m_dEFU[iEFU] = dValue;								//20181207 edhan change 32 to 50
			//			m_pFFUMon->SendEFUValue(32*nId+iEFUID, dValue, m_nMode);	//yhjoo 20160704 add		//20181207 edhan change 32 to 50
						m_pFFUMon->SendEFUValue(iEFU, dValue, m_nMode);	// jsyoon 20201111 change
 					}
 
 					iEFUDataStart = i + 4;
 
 					if(iEDUCnt == iEFUNum)
 					{
 						break;
 					}
 				}
 			}
 		}
 
 		if(bDPU_ID && bETX)
 		{
 			strLog = "== Message Success ==";
 			m_pFFUMon->WriteLogList(strLog, nMode);
 			if(bINFOMODE1 && bINFOMODE2)
 			{
 				strLog = "== INFO Message ==";
 				m_pFFUMon->WriteLogList(strLog, nMode);
 
 			}
 			else if(bRPMMODE1 && bRPMMODE2 && bRPMFlagOK)
 			{
 				strLog = "== RPM Message ==";
 				m_pFFUMon->WriteLogList(strLog, nMode);
 
 			}
 		}
 		else
 		{
 			strLog.Format(_T("== Message Fail : STX(%d) LV32_ID(%d) DPU_ID(%d) ETX(%d) =="), 
 				bSTX, bLV32_ID, bDPU_ID, bETX);
 			m_pFFUMon->WriteLogList(strLog, nMode);
 		}
	}
	else if (m_pFFUMon->m_bEFU_Type == EFU_TYPE_ISU)
	{
		for (int i=0; i<7; i++)
		{
			byteCheckSum += vecRvcData[i];
		}
		
		unsigned char cCRC[7] = {0,};
		memcpy(&cCRC, &vecRvcData[0], 7);
		
		memcpy(&nCRC_Check, &vecRvcData[7], sizeof(nCRC_Check));
		nCRC_Check = htons(nCRC_Check);
		
		nNum = vecRvcData[2];
		
		if(nNum > 6)
			return;
		
		memcpy(&nVal, &vecRvcData[4], sizeof(nVal));
		nVal = htons(nVal);
		dValue = (double)(nVal);
		iEFU = 32*nId + nNum-1;

		m_pFFUMon->m_dEFU[iEFU] = dValue;
		m_pFFUMon->SendEFUValue(iEFU, dValue, m_nMode);
	}
	//20181116 edhan add
	else if (m_pFFUMon->m_bEFU_Type == EFU_TYPE_EBM)
	{
		/*unsigned char cCRC[7] = {0,};
		memcpy(&cCRC, &vecRvcData[0], 7);*/

		//0x21 = Read Speed 조건으로 1번쨰 Efu

		nNum = (vecRvcData[1] - EBM_EFU_NUM);

		CString strHex;
		strHex.Format("%02X", vecRvcData[3]);
		nVal = strtol((LPCSTR)strHex, NULL, 16);
		dValue = (double)(nVal);
		dValue = dValue * (m_pFFUMon->m_uEFUSpeedMax[nNum]) / 250;

		// jsyoon 20190126 change
		int iEFUGroup = 0;
		switch (nId)
		{
		case 0:
			iEFUGroup = nNum;
			break;
		case 1:
			iEFUGroup = m_pFFUMon->m_nEFUCnt[0] +  nNum;
			break;
		case 2:
			iEFUGroup = (m_pFFUMon->m_nEFUCnt[0] + m_pFFUMon->m_nEFUCnt[1]) +  nNum;
			break;
		case 3:
			iEFUGroup = (m_pFFUMon->m_nEFUCnt[0] + m_pFFUMon->m_nEFUCnt[1]+ m_pFFUMon->m_nEFUCnt[2]) +  nNum;
			break;
		case 4:
			iEFUGroup = (m_pFFUMon->m_nEFUCnt[0] + m_pFFUMon->m_nEFUCnt[1]+ m_pFFUMon->m_nEFUCnt[2] + m_pFFUMon->m_nEFUCnt[3]) +  nNum;
			break;
		default:
			break;
		}

		m_pFFUMon->m_dEFU[iEFUGroup] = dValue;		//20181207 edhan change 32 to 50
		m_pFFUMon->SendEFUValue(iEFUGroup, dValue, m_nMode);		//20181207 edhan change 32 to 50
		API_SM_SetFFU_SV(iEFUGroup+1, (int)dValue, m_pFFUMon->m_nIdex);

// 		nNum = (vecRvcData[1] - EBM_EFU_NUM) + 1;
// 
// 		/*if(nNum > 6)
// 			return;*/
// 
// // 		memcpy(&nVal, &vecRvcData[3], sizeof(nVal));
// // 		nVal = htons(nVal);
// // 		dValue = (double)(nVal);
// // 		dValue = dValue * m_pFFUMon->m_uEFUSpeedMax[nNum-1] / 250;
// 		CString strHex;
// 		strHex.Format("%02X", vecRvcData[3]);
// 		nVal = strtol((LPCSTR)strHex, NULL, 16);
// 		dValue = (double)(nVal);
// 		dValue = dValue * (m_pFFUMon->m_uEFUSpeedMax[nNum-1]) / 250;
// 		//m_pFFUMon->m_dEFU[32*nId + nNum-1] = dValue;		//20181207 edhan change 32 to 50
// 		//m_pFFUMon->SendEFUValue(32*nId+nNum, dValue);		//20181207 edhan change 32 to 50
// 		//6 //3 //4
// 
// 		// jsyoon 20190126 change
// 		int iEFUGroup = 0;
// 		switch (nId)
// 		{
// 		case 0:
// 			iEFUGroup = nNum;
// 			break;
// 		case 1:
// 			iEFUGroup = m_pFFUMon->m_nEFUCnt[0] +  nNum;
// 			break;
// 		case 2:
// 			iEFUGroup = (m_pFFUMon->m_nEFUCnt[0] + m_pFFUMon->m_nEFUCnt[1]) +  nNum;
// 			break;
// 		case 3:
// 			iEFUGroup = (m_pFFUMon->m_nEFUCnt[0] + m_pFFUMon->m_nEFUCnt[1]+ m_pFFUMon->m_nEFUCnt[2]) +  nNum;
// 			break;
// 		case 4:
// 			iEFUGroup = (m_pFFUMon->m_nEFUCnt[0] + m_pFFUMon->m_nEFUCnt[1]+ m_pFFUMon->m_nEFUCnt[2] + m_pFFUMon->m_nEFUCnt[3]) +  nNum;
// 			break;
// 		default:
// 			break;
// 		}
// 
// 		m_pFFUMon->m_dEFU[iEFUGroup-1] = dValue;		//20181207 edhan change 32 to 50
// 		m_pFFUMon->SendEFUValue(iEFUGroup, dValue, m_nMode);		//20181207 edhan change 32 to 50
// 		API_SM_SetFFU_SV(iEFUGroup, (int)dValue, m_pFFUMon->m_nIdex);

// 		if(nId == 0)
// 		{
// 			m_pFFUMon->m_dEFU[nNum-1] = dValue;		//20181207 edhan change 32 to 50
// 			m_pFFUMon->SendEFUValue(nNum, dValue, m_nMode);		//20181207 edhan change 32 to 50
// //			m_pFFUMon->SendEFUValue(nNum, dValue);		//20181207 edhan change 32 to 50
// 			API_SM_SetFFU_SV(nNum, (int)dValue, m_pFFUMon->m_nIdex);
// 		}
// 		else if(nId == 1)
// 		{
// 			m_pFFUMon->m_dEFU[m_pFFUMon->m_nEFUCnt[0] +  nNum-1] = dValue;		//20181207 edhan change 32 to 50
// 			m_pFFUMon->SendEFUValue(m_pFFUMon->m_nEFUCnt[0] + nNum, dValue, m_nMode);		//20181207 edhan change 32 to 50
// //			m_pFFUMon->SendEFUValue(m_pFFUMon->m_nEFUCnt[0] + nNum, dValue);		//20181207 edhan change 32 to 50
// 			API_SM_SetFFU_SV(m_pFFUMon->m_nEFUCnt[0] +  nNum, (int)dValue, m_pFFUMon->m_nIdex);
// 		}
// 		else if(nId == 2)
// 		{
// 			m_pFFUMon->m_dEFU[(m_pFFUMon->m_nEFUCnt[0] + m_pFFUMon->m_nEFUCnt[1]) +  nNum-1] = dValue;		//20181207 edhan change 32 to 50
// 			m_pFFUMon->SendEFUValue((m_pFFUMon->m_nEFUCnt[0] + m_pFFUMon->m_nEFUCnt[1]) + nNum, dValue, m_nMode);		//20181207 edhan change 32 to 50			
// //			m_pFFUMon->SendEFUValue((m_pFFUMon->m_nEFUCnt[0] + m_pFFUMon->m_nEFUCnt[1]) + nNum, dValue);		//20181207 edhan change 32 to 50
// 			API_SM_SetFFU_SV((m_pFFUMon->m_nEFUCnt[0] + m_pFFUMon->m_nEFUCnt[1]) +  nNum, (int)dValue, m_pFFUMon->m_nIdex);
// 		}
// 		else if(nId == 3)
// 		{
// 			m_pFFUMon->m_dEFU[(m_pFFUMon->m_nEFUCnt[0] + m_pFFUMon->m_nEFUCnt[1]+ m_pFFUMon->m_nEFUCnt[2]) +  nNum-1] = dValue;		//20181207 edhan change 32 to 50
// 			m_pFFUMon->SendEFUValue((m_pFFUMon->m_nEFUCnt[0] + m_pFFUMon->m_nEFUCnt[1] + m_pFFUMon->m_nEFUCnt[2]) + nNum, dValue, m_nMode);		//20181207 edhan change 32 to 50			
// //			m_pFFUMon->SendEFUValue((m_pFFUMon->m_nEFUCnt[0] + m_pFFUMon->m_nEFUCnt[1] + m_pFFUMon->m_nEFUCnt[2]) + nNum, dValue);		//20181207 edhan change 32 to 50
// 			API_SM_SetFFU_SV((m_pFFUMon->m_nEFUCnt[0] + m_pFFUMon->m_nEFUCnt[1]+ m_pFFUMon->m_nEFUCnt[2]) +  nNum, (int)dValue, m_pFFUMon->m_nIdex);
// 		}
// 		else if(nId == 4)
// 		{
// 			m_pFFUMon->m_dEFU[(m_pFFUMon->m_nEFUCnt[0] + m_pFFUMon->m_nEFUCnt[1]+ m_pFFUMon->m_nEFUCnt[2] + m_pFFUMon->m_nEFUCnt[3]) +  nNum-1] = dValue;		//20181207 edhan change 32 to 50
// 			m_pFFUMon->SendEFUValue((m_pFFUMon->m_nEFUCnt[0] + m_pFFUMon->m_nEFUCnt[1] + m_pFFUMon->m_nEFUCnt[2] + m_pFFUMon->m_nEFUCnt[3]) + nNum, dValue, m_nMode);		//20181207 edhan change 32 to 50
// //			m_pFFUMon->SendEFUValue((m_pFFUMon->m_nEFUCnt[0] + m_pFFUMon->m_nEFUCnt[1] + m_pFFUMon->m_nEFUCnt[2] + m_pFFUMon->m_nEFUCnt[3]) + nNum, dValue);		//20181207 edhan change 32 to 50
// 			API_SM_SetFFU_SV((m_pFFUMon->m_nEFUCnt[0] + m_pFFUMon->m_nEFUCnt[1]+ m_pFFUMon->m_nEFUCnt[2] + m_pFFUMon->m_nEFUCnt[3]) +  nNum, (int)dValue, m_pFFUMon->m_nIdex);
// 		}
	}
	//
	else if(m_pFFUMon->m_bEFU_Type == EFU_TYPE_MAYAIR)
	{
//		*********************************************[Answer Frame Format]*********************************************
//		 => Format
//  	========================================================================================================
//		| [Slave Address] | [Function Code : 3] | [Byte Count]| [First word value]| [End word value] |  [CRC]  |
//		------------------|---------------------|-------------|-------------------|------------------|---------|
//      |     1 Byte      |       1 Byte        |   1 Byte    | 1 Word = 2 Bytes  | 1 Word = 2 Bytes |  1Byte  |
//		========================================================================================================
//		
//		=> Example 
// 		vecRvcData[0] = 0x05;	// Slave Address : 1 Byte	
// 		vecRvcData[1] = 0x03	// Function Code : 1 Byte
// 		vecRvcData[2] = 0x04;	// Byte Count : 1 Byte
//
//		// 1st Word Value
// 		vecRvcData[3] = 0x1D;	// High-order value
// 		vecRvcData[4] = 0xDD;	// Low-order value
//
//		// 2nd Word Value 
// 		vecRvcData[5] = 0x11;	// High-order value
// 		vecRvcData[6] = 0x6F;	// Low-order value
//
// 		vecRvcData[7] = 0xD6;	// CRC 
//		---------------------------------------------------------------------------------------------------------------

		CString strMsg				=	_T("");
		CString strTemp				=	_T("");
		CString strHex				=   _T("");

		int i					= 0; 
		int j					= 0;

		int iSlaveIdx			= 0;	
		int iFuncIdx			= 1;	
		int iByteCntIdx			= 2;
		int iDataStartIdx		= 3;

		int iFanCntIdx		    = iDataStartIdx + (MAYAIR_NUMBER_OF_FANS*2);
		int iFanSpeedStartIdx	= iDataStartIdx + (MAYAIR_FAN_CUR_SPEED_01*2);

		int nByteCnt			= 0;
		int iRcvData[200]		= {0, };

		int nFanCnt				= 0;
		int nFanMax				= 31;

		nByteCnt = (int)vecRvcData[iByteCntIdx];	

		int nDataCnt = nByteCnt/2;

		for(i=0; i<nDataCnt; i++)
		{
			int HighOrderIdx	= iDataStartIdx+j;
			int LowOrderIdx		= HighOrderIdx+1;

			int ByteToDec		= 0;

			ByteToDec = ByteToDec + (vecRvcData[HighOrderIdx] << 8);
			ByteToDec = ByteToDec + vecRvcData[LowOrderIdx];

			iRcvData[i] = ByteToDec;

			j += 2;
		}

		// The Number of Fans
		nFanCnt = iRcvData[MAYAIR_NUMBER_OF_FANS];	

		if(iRcvData[MAYAIR_NUMBER_OF_FANS] == 0)
			nFanCnt = m_pFFUMon->m_nEFUCnt[nId];

		//  Fan current speed 
		for(i=0; i<nFanCnt; i++)
		{
			int idx				 = MAYAIR_FAN_CUR_SPEED_01+i;
			dValue = iRcvData[idx];
			m_pFFUMon->m_dEFU[i] = dValue;
			m_pFFUMon->SendEFUValue(i, dValue, m_nMode);
		}
	}
	//
	else if(m_pFFUMon->m_bEFU_Type == EFU_TYPE_HKC_H5)	// jsyoon 20201104 add
	{
		//      MODBUS CRC-16
		//      Code: 0x03, 0x06, 0x10
		//		Function Code		  Meaning
		//		0x03                  Read  Multiple Registers
		//		0x10                  Write Multiple Registers
		//		0x06                  Write Single Register
		//		*********************************************[Answer Frame Format]*********************************************
		//		 => Format
		//  	========================================================================================================
		//		| [Register Address] | [Function Code : 3] | [Byte Count]  | [EFU SPEED value] | [CRC]   |
		//		------------------   |---------------------|-------------|-------------------|---------|
		//      |     1 Byte         |       1 Byte        |   1 Byte    | 1 Word = 2 Bytes  |  1 Word = 2 Bytes  |
		//		========================================================================================================
		//		
		//		=> Example 
		// 		vecRvcData[0] = 0x01;	// Register Address : 1 Byte	
		// 		vecRvcData[1] = 0x03	// Function Code : 1 Byte
		// 		vecRvcData[2] = 0x02;	// Count : 1 Byte
		//
		//		// EFU SPEED value    700 rpm
		// 		vecRvcData[3] = 0x02;	// High-order value
		// 		vecRvcData[4] = 0xBC;	// Low-order value
		//
		//		// CRC-16
		// 		vecRvcData[5] = 0xB8;	// High-order value
		// 		vecRvcData[6] = 0x95;	// Low-order value
		//
		//		---------------------------------------------------------------------------------------------------------------

		iMsgSize = vecRvcData.size();
		BYTE* pbyteEFU = NULL;
		pbyteEFU =new BYTE[iMsgSize];
		
		for(int i = 0; i <iMsgSize;  i++)
		{
			pbyteEFU[i] = vecRvcData[i];
		}

		if( m_nSendType == 0 )		// GET RPM
		{
			int j = 0, nMaxEFU = 0;
			j = iMsgSize-2;
			unsigned short crc16;
			crc16 = CRC16_MODBUS(pbyteEFU, j); 

			CString strHex;
			strHex.Format("%02X", pbyteEFU[2]);
			nMaxEFU = strtol((LPCSTR)strHex, NULL, 16);

			int iStart = 0;
			for(iEFU = 0; iEFU <nMaxEFU/2;  iEFU++)
			{
				iStart = 2*iEFU+3;
				strHex.Format("%02X%02X", pbyteEFU[iStart], pbyteEFU[iStart+1]);
				nVal = strtol((LPCSTR)strHex, NULL, 16);
				dValue = (double)(nVal);
	//			dValue = dValue * (m_pFFUMon->m_uEFUSpeedMax[iEFU]) / 250;

				// jsyoon 20190126 change
				int iEFUGroup = 0;
				switch (nId)
				{
				case 0:
					iEFUGroup = iEFU;
					break;
				case 1:
					iEFUGroup = m_pFFUMon->m_nEFUCnt[0] +  iEFU;
					break;
				case 2:
					iEFUGroup = (m_pFFUMon->m_nEFUCnt[0] + m_pFFUMon->m_nEFUCnt[1]) +  iEFU;
					break;
				case 3:
					iEFUGroup = (m_pFFUMon->m_nEFUCnt[0] + m_pFFUMon->m_nEFUCnt[1]+ m_pFFUMon->m_nEFUCnt[2]) +  iEFU;
					break;
				case 4:
					iEFUGroup = (m_pFFUMon->m_nEFUCnt[0] + m_pFFUMon->m_nEFUCnt[1]+ m_pFFUMon->m_nEFUCnt[2] + m_pFFUMon->m_nEFUCnt[3]) +  iEFU;
					break;
				default:
					break;
				}

				
				switch (m_pFFUMon->m_nEFUStatus[iEFUGroup])
				{
				case df_HKC_EFU_STS_HALL_FAULURE:
					m_pFFUMon->m_dEFU[iEFUGroup] = -10;
					break;
				case df_HKC_EFU_STS_OVERLOAD:
					m_pFFUMon->m_dEFU[iEFUGroup] = -11;
					break;
				case df_HKC_EFU_STS_LOCKED_ROTOR:
					m_pFFUMon->m_dEFU[iEFUGroup] = -12;
					break;
				case df_HKC_EFU_STS_UNKOWN_ERR:
					m_pFFUMon->m_dEFU[iEFUGroup] = -13;
					break;
				case df_HKC_EFU_STS_FFU_DISCONNECT:
					m_pFFUMon->m_dEFU[iEFUGroup] = -14;
					break;
				default:
					m_pFFUMon->m_dEFU[iEFUGroup] = dValue;		//20181207 edhan change 32 to 50
					break;
				}

				m_pFFUMon->SendEFUValue(iEFUGroup, dValue, m_nMode);		//20181207 edhan change 32 to 50
				API_SM_SetFFU_SV(iEFUGroup+1, (int)dValue, m_pFFUMon->m_nIdex);
			}
		}
		else if( m_nSendType == 2 )		// GET STATUS
		{
			int iStatus = 0;
			int j = 0, nMaxEFU = 0;
			j = iMsgSize-2;
			unsigned short crc16;
			crc16 = CRC16_MODBUS(pbyteEFU, j); 

			CString strHex;
			strHex.Format("%02X", pbyteEFU[2]);
			nMaxEFU = strtol((LPCSTR)strHex, NULL, 16);

			int iStart = 0;
			for(iEFU = 0; iEFU <nMaxEFU/2;  iEFU++)
			{
				iStart = 2*iEFU+3;
				strHex.Format("%02X%02X", pbyteEFU[iStart], pbyteEFU[iStart+1]);
				nVal = strtol((LPCSTR)strHex, NULL, 16);
				
//				0x0001：Hall Failure
//				0x0010: Overload
//				0x0080：locked-rotor
//				0x0400：Unkown error
//				0x0800：FFU Disconnect
//				0x8000：Run

				int nAlarm = 0;
				switch (nVal)
				{
				case 0:			// SPEED ZERO
					iStatus = df_HKC_EFU_STS_STOP;
					break;
				case 1:			// Hall Failure
					iStatus = df_HKC_EFU_STS_HALL_FAULURE;
					nAlarm = 7;
					break;
				case 16:		// Overload
					iStatus = df_HKC_EFU_STS_OVERLOAD;
					nAlarm = 6;
					break;
				case 128:		//locked-rotor
					iStatus = df_HKC_EFU_STS_LOCKED_ROTOR;
					nAlarm = 8;
					break;
				case 1028:		//Unkown error
					iStatus = df_HKC_EFU_STS_UNKOWN_ERR;
					nAlarm = 1028;
					break;
				case 2048:		//FFU Disconnect
					iStatus = df_HKC_EFU_STS_FFU_DISCONNECT;
					nAlarm = 2048;
					break;
				}

	//			dValue = dValue * (m_pFFUMon->m_uEFUSpeedMax[iEFU]) / 250;

				// jsyoon 20190126 change
				int iEFUGroup = 0;
				switch (nId)
				{
				case 0:
					iEFUGroup = iEFU;
					break;
				case 1:
					iEFUGroup = m_pFFUMon->m_nEFUCnt[0] +  iEFU;
					break;
				case 2:
					iEFUGroup = (m_pFFUMon->m_nEFUCnt[0] + m_pFFUMon->m_nEFUCnt[1]) +  iEFU;
					break;
				case 3:
					iEFUGroup = (m_pFFUMon->m_nEFUCnt[0] + m_pFFUMon->m_nEFUCnt[1]+ m_pFFUMon->m_nEFUCnt[2]) +  iEFU;
					break;
				case 4:
					iEFUGroup = (m_pFFUMon->m_nEFUCnt[0] + m_pFFUMon->m_nEFUCnt[1]+ m_pFFUMon->m_nEFUCnt[2] + m_pFFUMon->m_nEFUCnt[3]) +  iEFU;
					break;
				default:
					break;
				}

				m_pFFUMon->m_nEFUStatus[iEFUGroup] = iStatus;	// jsyoon 20201208 add
			//	API_SM_SetFFU_ALARM(iEFUGroup+1, iStatus, m_pFFUMon->m_nIdex);
				API_SM_SetFFU_ALARM(iEFUGroup+1, nAlarm, m_pFFUMon->m_nIdex);	// jsyoon 20210504 change

			}
		}

		delete pbyteEFU;
		pbyteEFU = NULL;

	}
}

BYTE CDlgComm::HEXtoBYTE(char hex)
{
	switch(hex)
	{
	case '0': return 0;
		break;
	case '1': return 1;
		break;
	case '2': return 2;
		break;
	case '3': return 3;
		break;
	case '4': return 4;
		break;
	case '5': return 5;
		break;
	case '6': return 6;
		break;
	case '7': return 7;
		break;
	case '8': return 8;
		break;
	case '9': return 9;
		break;
	case 'A': return 10;
		break;
	case 'B': return 11;
		break;
	case 'C': return 12;
		break;
	case 'D': return 13;
		break;
	case 'E': return 14;
		break;
	case 'F': return 15;
		break;
	case 'a': return 10;
		break;
	case 'b': return 11;
		break;
	case 'c': return 12;
		break;
	case 'd': return 13;
		break;
	case 'e': return 14;
		break;
	case 'f': return 15;
		break;
	default :				//20171102 edhan add
		TRACE("in default");
		break;
	}
	return 32;
}

void CDlgComm::ByteToFloat(BYTE* btSrc,  float &fDest, int nStart, bool bInverse)
{
	if(bInverse)
	{
		m_stConvertData.stData.bt1	=	btSrc[nStart+3];
		m_stConvertData.stData.bt2	=	btSrc[nStart+2];
		m_stConvertData.stData.bt3	=	btSrc[nStart+1];
		m_stConvertData.stData.bt4	=	btSrc[nStart+0];
	}
	else
	{
		m_stConvertData.stData.bt1	=	btSrc[nStart+0];
		m_stConvertData.stData.bt2	=	btSrc[nStart+1];
		m_stConvertData.stData.bt3	=	btSrc[nStart+2];
		m_stConvertData.stData.bt4	=	btSrc[nStart+3];
	}

	fDest	=	m_stConvertData.fData;
}

void CDlgComm::SendToLed(int iPort, int i, double dO2)
{
	if( api_SerialCom_IsOpened(iPort))
	{
		char szMsg[128];
		BYTE byteuf[128];
		byteuf[0] = 0x00;
		szMsg[0] = 0x00;
		CString strMsg;
		
// 		strMsg = _T("[TMODE]\r");
// 		strcpy(szMsg,strMsg);
// 		memcpy(&byteuf,&szMsg, strMsg.GetLength());
		byteuf[0] = '[';
		byteuf[1] = 'T';
		byteuf[2] = 'M';
		byteuf[3] = 'O';
		byteuf[4] = 'D';
		byteuf[5] = 'E';
		byteuf[6] = ']';
		byteuf[7] = 0x0d;
		api_SerialCom_WriteByte(m_nPort, byteuf, strMsg.GetLength());

		strMsg = _T("[COMON]");	//0x0d??
//		strcpy(szMsg,strMsg);
//		strcpy_s(szMsg,strMsg);	// mn.bong 20180709 change strcpy -> strcpy_s
		_tcscpy_s(szMsg, _countof(szMsg), strMsg);
		memcpy(&byteuf,&szMsg, strMsg.GetLength());
		api_SerialCom_WriteByte(m_nPort, byteuf, strMsg.GetLength());

		
		//ksk 20150708 change
	//	strMsg.Format("[PX0 PY0 CR3]산소농도 ");
//		strMsg.Format("[PX0 PY0 CR%d]산소농도 ", ( dO2/10000 > 20.0 && dO2/10000 < 23.0 )? 3:1);
		strMsg.Format("[PX0 PY0 CR%d]산소농도 ", ( dO2/10000 > 20.0 && dO2/10000 < 23.0 )? 2:1);				// sih 20151228 change 
//		strcpy(szMsg,strMsg);
//		strcpy_s(szMsg,strMsg);	// mn.bong 20180709 change strcpy -> strcpy_s
		_tcscpy_s(szMsg, _countof(szMsg), strMsg);
		memcpy(&byteuf,&szMsg, strMsg.GetLength());
		api_SerialCom_WriteByte(m_nPort, byteuf, strMsg.GetLength());
		
		//ksk 20150708 change
		if( i == 1)	// %
	//		strMsg.Format("[CR%d]%.1f%%[ACT0]",  ( dO2/10000 > 19.5 )? 3:1,  dO2/10000);		// CR3: YELLOW, CR1: Red	// jsyoon 20150320 change 18 to 19.5
//			strMsg.Format("[CR%d]%.1f%%[ACT0]",  ( dO2/10000 > 20.0 && dO2/10000 < 23.0)? 3:1,  dO2/10000);
			strMsg.Format("[CR%d]%.1f%%[ACT0]",  ( dO2/10000 > 20.0 && dO2/10000 < 23.0)? 2:1,  dO2/10000);		// sih 20151228 change 	
		else		// PPM
	//		strMsg.Format("[CR1]%.1fppm[ACT0]", dO2);	//ksk 20150908 change
			strMsg.Format("[CR1]%.3fppm[ACT0]", dO2);

//		strcpy(szMsg,strMsg);	
//		strcpy_s(szMsg,strMsg);	// mn.bong 20180709 change strcpy -> strcpy_s
		_tcscpy_s(szMsg, _countof(szMsg), strMsg);
		memcpy(&byteuf,&szMsg, strMsg.GetLength());
		api_SerialCom_WriteByte(m_nPort, byteuf, strMsg.GetLength());
		
		strMsg = _T("[COMOFF]");
//		strcpy(szMsg,strMsg);
//		strcpy_s(szMsg,strMsg);	// mn.bong 20180709 change strcpy -> strcpy_s
		_tcscpy_s(szMsg, _countof(szMsg), strMsg);
		memcpy(&byteuf,&szMsg, strMsg.GetLength());
		api_SerialCom_WriteByte(m_nPort, byteuf, strMsg.GetLength());
	}
}


int CDlgComm::RcvThermTest(void)
{
	CString strMsg, strLog, strTmp, strTemp;
	int iMsgSize = 0;

	BYTE bDelimeter = '0';
	BYTE vecRvcData[dfBUFFER_SIZE];
	ZeroMemory(vecRvcData, sizeof(BYTE)*dfBUFFER_SIZE);
	strMsg.Format("06 02 30 35 52 44 50 30 20 30 32 33 38 31 03 6C 00 06 02 30 36 52 44 50 30 20 30 33 31 37 31 03 63 00 06 02 30 37 52 44 50 30 20 30 32 36 36 31 03 65 00 06 02 30 33 52 44 50 30 20 30 32 34 31 31 03 64 00 06 02 30 34 52 44 50 30 20 30 32 39 30 31 03 6F 00");
	strMsg.Replace(" ", "");

	int i=0;
	iMsgSize = strMsg.GetLength();
	TCHAR szTmp;
	for( i = 0; i<iMsgSize/2; i++)
	{
		// Hex to Int
		strTmp.Format("%c", HEXtoBYTE(strMsg[2*i])*16 + HEXtoBYTE(strMsg[(2*i+1)]));
		memcpy(&szTmp, strTmp, sizeof(szTmp));
		vecRvcData[i] = (BYTE)(szTmp);
	}

	strMsg.Empty();
	for( i = 0; i<iMsgSize/2; i++)
	{
		bDelimeter	 = vecRvcData[i];

		if( bDelimeter != 0x00 )
			strTemp += vecRvcData[i];
		else
			strTemp += "0";

		strTmp.Format(_T("%02X "), vecRvcData[i] ); 
		strMsg += strTmp;
	}
	
	iMsgSize = strMsg.GetLength();
	strLog.Format(_T("RCV: %s (%d)"), strMsg, iMsgSize);
	m_pFFUMon->WriteLogList(strLog, m_nMode+m_nNum);

	m_strReceive.Format("%s", strTemp);

	CString strValue;
	int iAddr = 0;
	int nReciveLen = 0;

	if(bDelimeter == 0x00 )	
	{
		strTemp.Format("%s", m_strReceive);

		double dValue = 0;
		double dRet = 0;		// jsyoon 20150106 change
		int iFind = 0;
		int dot = 0;

		while(1) 
		{
			nReciveLen = strTemp.GetLength();
			if( nReciveLen >= 17)
			{
				strValue = strTemp.Mid(2, 2);
				iAddr = atoi(strValue) - 1;

				iFind = strTemp.Find("RDP");
				if( iFind > 1 )
				{
					strValue = strTemp.Mid(iFind+5, 5);
					dot = strTemp[13]-'0';		// 소수점
					dValue = atof(strValue)/10;

					for(int k=0;k<dot;k++)
						dValue /= 10;

					dRet = dValue;
				}
				//////////////////////////////////////////////////////////////////////////

				if( dRet > -10 )		// Temp -10 
				{
					m_pFFUMon->m_dThermometer[m_nNum][iAddr] = dRet;

					SetEvent(m_hEvtRcv);		// jsyoon 20181001 add

					strLog.Format(_T(">>>>>>>Get VALUE(%d) = %.1f"), iAddr+1, dRet);
					m_pFFUMon->WriteLogList(strLog, m_nMode+m_nNum);
				}

				strTemp.Delete(0, 17);
			}
			else
			{
				break;
			}
		} 
	}

	return 0;
}


void CDlgComm::OnBnClickedOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CDialog::OnOK();
}


int CDlgComm::SetEFUSpeed(int nSpeed, int iGroup, int nStart, int nEnd)
{
	int i = 0,j = 0;
	BYTE bytChkSum;

	CString strLog, strMsg, strHex, strout;

	long value= 0;
	UINT iSV = nSpeed;
	strHex.Format(_T("%02X"), iSV);
	strout.Format(_T("%d"), strtol(strHex, NULL, 16));
	//20181207 edhan add
	int index = 0;

	if(m_pFFUMon->m_chkModule[SENSOR_MODE_EFU].GetCheck())
		index = 0;
	else if(m_pFFUMon->m_chkModule[SENSOR_MODE_EFU2].GetCheck())
		index = 1;
	else if(m_pFFUMon->m_chkModule[SENSOR_MODE_EFU3].GetCheck())
		index = 2;
	else if(m_pFFUMon->m_chkModule[SENSOR_MODE_EFU4].GetCheck())
		index = 3;
	else if(m_pFFUMon->m_chkModule[SENSOR_MODE_EFU5].GetCheck())
		index = 4;
	for ( j=nStart; j<=nEnd; j++)
	{
		if(index == 0)
			API_SM_SetFFU_PV(j, nSpeed, m_pFFUMon->m_nIdex);
		else if(index == 1)
			API_SM_SetFFU_PV(m_pFFUMon->m_nEFUCnt[0] + j, nSpeed, m_pFFUMon->m_nIdex);
		else if(index == 2)
			API_SM_SetFFU_PV((m_pFFUMon->m_nEFUCnt[0] + m_pFFUMon->m_nEFUCnt[1]) + j, nSpeed, m_pFFUMon->m_nIdex);
		else if(index == 3)
			API_SM_SetFFU_PV((m_pFFUMon->m_nEFUCnt[0] + m_pFFUMon->m_nEFUCnt[1]+ m_pFFUMon->m_nEFUCnt[2]) + j, nSpeed, m_pFFUMon->m_nIdex);
		else if(index == 4)
			API_SM_SetFFU_PV((m_pFFUMon->m_nEFUCnt[0] + m_pFFUMon->m_nEFUCnt[1]+ m_pFFUMon->m_nEFUCnt[2] + m_pFFUMon->m_nEFUCnt[3]) + j, nSpeed, m_pFFUMon->m_nIdex);

		strMsg.Empty();

		if (m_pFFUMon->m_bEFU_Type == EFU_TYPE_EBM)
		{
			BYTE bytefuEBM[5];
			ZeroMemory(bytefuEBM,sizeof(bytefuEBM));
			//
			//Set Speed 명령은 5개 Byte사용
			bytefuEBM[0] = 0x35;				 // Preamble
			bytefuEBM[1] = (BYTE)(64+j);		 // Command byte //0x21 = Read Speed 조건으로 1번쨰 Efu
			bytefuEBM[2] = (BYTE)iGroup;		 // fan group address //1번째 그룹?

			value = _tstoi(strout);
			value *= 250;
			value /= m_pFFUMon->m_uEFUSpeedMax[j-1]; 
			bytefuEBM[3] = (BYTE)value; // Target Value

			bytChkSum = bytefuEBM[0]^bytefuEBM[1];
			bytChkSum ^= bytefuEBM[2];
			bytChkSum ^= bytefuEBM[3];
			bytChkSum ^= 0xFF;

			bytefuEBM[4] = bytChkSum;				// checksum		//0xB8

			if( api_SerialCom_IsOpened(m_nPort))
				api_SerialCom_WriteByte(m_nPort, bytefuEBM, 5);

			for(i=0; i < 5; i++)
			{
				strLog.Format(_T("%02X "), bytefuEBM[i] ); 
				strMsg += strLog;
			}
		}
		else if (m_pFFUMon->m_bEFU_Type == EFU_TYPE_HKC_H5)	// jsyoon 20201104 add
		{

			unsigned short crc16;
			BYTE byteEFU[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};	

			int nSize		= 8;

			byteEFU[0] = 0x01;	   // 유닛 아이디  
			byteEFU[1] = 0x10;     // Write Multiple Registers 
			byteEFU[2] = 0x11;     // Set 11FD ~
			byteEFU[3] = 0xFB; 
			byteEFU[4] = 0x00; 
			byteEFU[5] = 0x01; 

			crc16 = CRC16_MODBUS(byteEFU, 6); 
			byteEFU[6] = (unsigned char)((crc16>>8) & 0x00ff);  //CRC 
			byteEFU[7] = (unsigned char)((crc16>>0) & 0x00ff); 

			if( api_SerialCom_IsOpened(m_nPort))
				api_SerialCom_WriteByte(m_nPort, byteEFU, nSize);

			for(int i=0; i<nSize; i++)
			{
				strLog.Format(_T("%02X "), byteEFU[i]); 
				strMsg += strLog;
			}
		}

		strLog = "SEND : ";
		strLog += strMsg;

		m_pFFUMon->WriteLogList(strLog, m_nMode);

		Sleep(500);
	}

	return 0;
}


int CDlgComm::GetEFUStatus(int iGroup /*1*/, int nStart /*0*/, int nEnd /*1*/)	//hh.joo_ward 20181212
{
	int i = 0,j = 0;
	BYTE bytChkSum;
	BYTE bytefuEBM[5];
	ZeroMemory(bytefuEBM,sizeof(bytefuEBM));
	CString strLog, strMsg, strHex, strout;

	{
		for ( j=nStart; j<=nEnd; j++)
	{
		//Set Speed 명령은 5개 Byte사용
		bytefuEBM[0] = 0x35;				// Preamble			 //0011 0101
		bytefuEBM[1] = (BYTE)(j);			// Command byte		 //
		bytefuEBM[2] = (BYTE)iGroup;		// fan group address //1번째 그룹?
		bytefuEBM[3] = (BYTE)0;				// data byte

		bytChkSum = bytefuEBM[0]^bytefuEBM[1];
		bytChkSum ^= bytefuEBM[2];
		bytChkSum ^= 0xFF;

		bytefuEBM[4] = bytChkSum;			// checksum		//0xB8
		//bytefuEBM[3] = 0x81;				// data byte
		//bytefuEBM[4] = 0x81;				// data byte2
		//bytefuEBM[5] = 0xB8;				// checksum

		if( api_SerialCom_IsOpened(m_nPort))
			api_SerialCom_WriteByte(m_nPort, bytefuEBM, 4);

		strMsg.Empty();

		for(i=0; i < 5; i++)
		{
			strLog.Format(_T("%02X "), bytefuEBM[i] ); 
			strMsg += strLog;
		}

		strLog = "SEND : ";
		strLog += strMsg;

		m_pFFUMon->WriteLogList(strLog, m_nMode);

		Sleep(500);
	}
	}

	

	return 0;

// 	int i = 0,j = 0;
// 	BYTE bytChkSum;
// 	BYTE bytefuEBM[4];
// 	ZeroMemory(bytefuEBM,sizeof(bytefuEBM));
// 	CString strLog, strMsg, strHex, strout;
// 
// 	//Set Speed 명령은 5개 Byte사용
// 	bytefuEBM[0] = 0x35;					// Preamble
// 	bytefuEBM[1] = (BYTE)(64+iTarget);		// Command byte //0x21 = Read Speed 조건으로 1번쨰 Efu
// 	bytefuEBM[2] = (BYTE)iGroup;			// fan group address //1번째 그룹?
// 	bytefuEBM[3] = (BYTE)_tstoi(strout);	// Target Value
// 
// 	bytChkSum = bytefuEBM[0]^bytefuEBM[1];
// 	bytChkSum ^= bytefuEBM[2];
// 	bytChkSum ^= bytefuEBM[3];
// 	bytChkSum ^= 0xFF;
// 
// 	bytefuEBM[4] = bytChkSum;				// checksum		//0xB8
// 	//bytefuEBM[3] = 0x81;				// data byte
// 	//bytefuEBM[4] = 0x81;				// data byte2
// 	//bytefuEBM[5] = 0xB8;				// checksum
// 
// 	if( api_SerialCom_IsOpened(m_nPort))
// 		api_SerialCom_WriteByte(m_nPort, bytefuEBM, 5);
// 
// 	strMsg.Empty();
// 
// 	for(i=0; i < 5; i++)
// 	{
// 		strLog.Format(_T("%02X "), bytefuEBM[i] ); 
// 		strMsg += strLog;
// 	}
// 
// 	strLog = "SEND : ";
// 	strLog += strMsg;
// 
// 	m_pFFUMon->WriteLogList(strLog, m_nMode);
//
//	return 0;
}

int CDlgComm::GetEFUSpeed(int iGroup, int nStart, int nEnd)
{
	int i = 0,j = 0;
	BYTE bytChkSum;
	BYTE bytefuEBM[4];
	ZeroMemory(bytefuEBM,sizeof(bytefuEBM));
	CString strLog, strMsg, strHex, strout;

	for ( j=nStart; j<=nEnd; j++)
	{
		//Set Speed 명령은 5개 Byte사용
		bytefuEBM[0] = 0x15;				// Preamble
		bytefuEBM[1] = (BYTE)(32+j);		// Command byte //0x21 = Read Speed 조건으로 1번쨰 Efu
		bytefuEBM[2] = (BYTE)iGroup;		// fan group address //1번째 그룹?

		bytChkSum = bytefuEBM[0]^bytefuEBM[1];
		bytChkSum ^= bytefuEBM[2];
		bytChkSum ^= 0xFF;

		bytefuEBM[3] = bytChkSum;				// checksum		//0xB8
		//bytefuEBM[3] = 0x81;				// data byte
		//bytefuEBM[4] = 0x81;				// data byte2
		//bytefuEBM[5] = 0xB8;				// checksum

		if( api_SerialCom_IsOpened(m_nPort))
			api_SerialCom_WriteByte(m_nPort, bytefuEBM, 4);

		strMsg.Empty();

		for(i=0; i < 4; i++)
		{
			strLog.Format(_T("%02X "), bytefuEBM[i] ); 
			strMsg += strLog;
		}

		strLog = "SEND : ";
		strLog += strMsg;

		m_pFFUMon->WriteLogList(strLog, m_nMode);

		Sleep(500);
	}

	return 0;
}

// yj.nam 20190520 add
void CDlgComm::GetCRC(int idxCRC, BYTE *byteBuff)	
{
	WORD C = 65535;		// 0xFFFF
	BYTE D;
	WORD CY, DD;

	int nBufSize = idxCRC+2; 

	for(int i=0; i< idxCRC; i++)
	{

		D = byteBuff[i];
		C = C^D;	// Exclusive OR

		for(int j=0; j<nBufSize; j++)
		{
			CY = C&1;
			C = C/2;

			if(CY == 1)			// 적출한 하위 비트가 1이면 
				C = C^40961;	// 0xA001와 Exclusive OR  
		}

		DD = C /256;

		byteBuff[idxCRC]	= (BYTE) (C -DD * 256);	
		byteBuff[idxCRC+1] = (BYTE) DD;


	}

}

int CDlgComm::ProceData(CString szMsg)
{
	// Adress
	CString str, strMsg, strValue, strLog, strT;
	strMsg.Format("%s", szMsg);

//	strMsg.Format("01036C430203034303237A43023FF44302777C436263214362A6124360E8644361FB32436283EF4362A3EA4360A5764361EF1A3FC8EB983FD4DC0840030F98436283F000000000C2E60FB9C3433056C2DE9EA3434308993F03D4123EF335D03D0D55223F0CFE96426FFBAD41E739C00461");

	float fValue = 0;
	unFormat t;
//	BYTE bTmp[8];
	int n = 0, l = 0, i = 0, nCount = 0;
	int nLoop = 0, nTotal = 2;
	do
	{
		strValue.Format("%02d%02d", 1+nLoop, 3 );
		n = strMsg.Find(strValue);		// "0103"
		if( n == 0 )
		{
			l = strMsg.GetLength();
			strValue = strMsg.Mid(n+4, 2);
			nCount = strtol(strValue, NULL, 16);

			if( l > nCount*2 )
			{
				//
				strValue = strMsg.Mid(n+6);		// Count 2, CRC 4

				for (i=0; i<nCount/2; i++)	// 2byte
				{
					t.iv = strtol(strValue.Mid(0+(8*i), 8), NULL, 16);

					if( m_nSetAuccuraData == 11044)
					{
						fValue = t.iv;
					}
					else if( m_nSetAuccuraData == 11201)
					{
						if ( i >= 43 && i < 52 )
							fValue = t.iv;
						else
							fValue = t.value;
					}
					else if( m_nSetAuccuraData == 11351  )
					{
						fValue = t.iv;
					}
					else
					{
						fValue = t.value;
					}
					
					if( m_pfAuccuraData != NULL)
						m_pfAuccuraData[i] = fValue;

					str.Format("%.3f, ", fValue);

					strT +=str;
				}

				strLog.Format(_T("Get Value: %s"), strT);

				m_pFFUMon->WriteLogList(strLog, m_nMode);

				if( m_nSetAuccuraData == 11044)
				{
					m_Validity = (int)m_pfAuccuraData[1];							// Accura 2350 Voltage Data

					for (int iVal=0; iVal<40; iVal++)
					{
						m_subunitValidity[iVal] = (int)m_pfAuccuraData[iVal+2];		// Accura 2350 Module ID 0 ~ 39
					}
				}
				else if( m_nSetAuccuraData == 11001)
				{
					m_pFFUMon->m_sData.Vavg_ll_1 = m_pfAuccuraData[7];
				}
				else if( m_nSetAuccuraData == 11201)
				{
					m_pFFUMon->m_sData.lavg_1 = m_pfAuccuraData[3];			//삼상의 전류 평균(Acura#1)
					m_pFFUMon->m_sData.Ptot_1 = m_pfAuccuraData[33];		//삼상의 유효전력 종합(Acura#1)
					m_pFFUMon->m_sData.Net_1  = (int)m_pfAuccuraData[46];   //수전 유효전력량과 송전 유효전력량의 차(Acura#1)
				}
				else if( m_nSetAuccuraData == 11351)
				{
					for (int iVal=0; iVal<20; iVal++)
					{
						m_pFFUMon->m_sData.nValue[iVal]      = (int)m_pfAuccuraData[10+2*iVal];
						m_pFFUMon->m_sData.nValueTotal[iVal] = (int)m_pfAuccuraData[11+2*iVal];

						if( m_pFFUMon->m_sData.nValueTotal[iVal] > 0)
						{
							m_pFFUMon->m_sData.nValueCnt++;
						}
					}

					m_pFFUMon->m_sData.nSubDev++;
				}
			}

		//	m_pFFUMon->m_pAccuraData[nLoop] = m_pFFUMon->m_sData;
		}

		nLoop++;

	}while(nLoop<nTotal);

	return 1;
}


int CDlgComm::GetDataFetch(int iGroup)
{
	BYTE byteCmd[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};	

	DWORD res = 0;
	int nSize		= 8;
	int idxCRC		= 6;

	CString strLog, strMsg;

	// 11044
	m_nSetAuccuraData = 11044;

	byteCmd[0] = 0x01+iGroup;	// Slave Address 		
	byteCmd[1] = 0x03;	// Function code :03 (Read holding register)

	byteCmd[2] = 0x2B;	// High-order address				
	byteCmd[3] = 0x23;	// Low-order address	11044-1 = 11043


	byteCmd[4] = 0x00;
	byteCmd[5] = 0x2A;	// Read 3 11044, 11045, 11046

	// CRC
	byteCmd[6] = 0x00;	// Low-order CRC 
	byteCmd[7] = 0x00;	// High-order CRC  

	GetCRC(idxCRC, byteCmd);	// 0xE185 : byteEFU[6] = 0x85, byteEFU[7] = 0xE1	

	if( api_SerialCom_IsOpened(m_nPort))
		api_SerialCom_WriteByte(m_nPort, byteCmd, nSize);

	for(int i=0; i<nSize; i++)
	{
		strLog.Format(_T("%02X "), byteCmd[i]); 
		strMsg += strLog;
	}

	strLog = "SEND : ";
	strLog += strMsg;

	m_pFFUMon->WriteLogList(strLog, m_nMode);

	res = WaitForSingleObject(m_hEvtRcv, 10000);

	return 0;
}


int CDlgComm::GetDataVoltage(int iGroup)
{
	BYTE byteCmd[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};	

	DWORD res = 0;
	int nSize		= 8;
	int idxCRC		= 6;

	CString strLog, strMsg;

	// 11001
	m_nSetAuccuraData = 11001;

	byteCmd[0] = 0x01+iGroup;	// Slave Address 		
	byteCmd[1] = 0x03;	// Function code :03 (Read holding register)

	byteCmd[2] = 0x2B;	// High-order address				
	byteCmd[3] = 0x5C;	// Low-order address

	byteCmd[4] = 0x00;
	byteCmd[5] = 0x7D;	// Read 1 - 125

	// CRC
	byteCmd[6] = 0x00;	// Low-order CRC 
	byteCmd[7] = 0x00;	// High-order CRC  

	GetCRC(idxCRC, byteCmd);	// 0xE185 : byteEFU[6] = 0x85, byteEFU[7] = 0xE1	

	if( api_SerialCom_IsOpened(m_nPort))
		api_SerialCom_WriteByte(m_nPort, byteCmd, nSize);

	for(int i=0; i<nSize; i++)
	{
		strLog.Format(_T("%02X "), byteCmd[i]); 
		strMsg += strLog;
	}

	strLog = "SEND : ";
	strLog += strMsg;

	m_pFFUMon->WriteLogList(strLog, m_nMode);

	res = WaitForSingleObject(m_hEvtRcv, 10000);

	return 0;
}


int CDlgComm::GetDataSub(int iGroup, int nType)
{
	BYTE byteCmd[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};	

	DWORD res = 0;
	int nSize		= 8;
	int idxCRC		= 6;

	CString strLog, strMsg;

	// 11001
	m_nSetAuccuraData = 11001;

	byteCmd[0] = 0x01+iGroup;	// Slave Address 		
	byteCmd[1] = 0x03;	// Function code :03 (Read holding register)

	// Start Address
	if( nType == 0)			// 3 Current
	{
		byteCmd[2] = 0x2B;	// High-order address				
		byteCmd[3] = 0xC0;	// Low-order address

		m_nSetAuccuraData = 11201;
	}
	else if( nType == 1)	// 1 Current
	{
		byteCmd[2] = 0x2B;	// High-order address				
		byteCmd[3] = 0xC0;	// Low-order address

		m_nSetAuccuraData = 11201;
	}
	else if( nType == 3)	// GW Data
	{
		byteCmd[2] = 0x2C;	// High-order address				
		byteCmd[3] = 0x56;	// Low-order address

		m_nSetAuccuraData = 11351;
	}
	else
	{
		return 0;
	}

	byteCmd[4] = 0x00;
	byteCmd[5] = 0x7D;	// Read 1 - 125

	// CRC
	byteCmd[6] = 0x00;	// Low-order CRC 
	byteCmd[7] = 0x00;	// High-order CRC  

	GetCRC(idxCRC, byteCmd);	// 0xE185 : byteEFU[6] = 0x85, byteEFU[7] = 0xE1	

	if( api_SerialCom_IsOpened(m_nPort))
		api_SerialCom_WriteByte(m_nPort, byteCmd, nSize);

	for(int i=0; i<nSize; i++)
	{
		strLog.Format(_T("%02X "), byteCmd[i]); 
		strMsg += strLog;
	}

	strLog = "SEND : ";
	strLog += strMsg;

	m_pFFUMon->WriteLogList(strLog, m_nMode);

	res = WaitForSingleObject(m_hEvtRcv, 10000);

	return 0;
}


#define AZBIL_0150_REQUEST_SIZE 17			//Write 버퍼
#define AZBIL_0150_RESPONSE_SIZE 100			//Read 버퍼

int CDlgComm::Azbil0150_WriteRead(void)
{
	CString strLog, strMsg;
	DWORD res = 0;

	BYTE btSendBuf[AZBIL_0150_REQUEST_SIZE];

	memset(btSendBuf, 0, AZBIL_0150_REQUEST_SIZE);	

	//Command Protocalls		All Flow Data
	btSendBuf[0] = 0x3A;		//Start Byte
	btSendBuf[1] = 0x30;		//High Byte Address
	btSendBuf[2] = 0x31;		//Low Byte Address
	btSendBuf[3] = 0x30;		//Function Code
	btSendBuf[4] = 0x33;		//Function Code
	//Data
	btSendBuf[5] = 0x30;
	btSendBuf[6] = 0x34;
	btSendBuf[7] = 0x42;
	btSendBuf[8] = 0x35;
	btSendBuf[9] = 0x30;
	btSendBuf[10] = 0x30;
	btSendBuf[11] = 0x30;
	btSendBuf[12] = 0x32;

	btSendBuf[13] = 0x34;		//LRC, Error Code
	btSendBuf[14] = 0x31;
	btSendBuf[15] = 0x0D;		//CR, 의미없는 데이터	
	btSendBuf[16] = 0x0A;

	if( api_SerialCom_IsOpened(m_nPort))
		api_SerialCom_WriteByte(m_nPort, btSendBuf, AZBIL_0150_REQUEST_SIZE);

	for(int i=0; i<AZBIL_0150_REQUEST_SIZE; i++)
	{
		strLog.Format(_T("%02X "), btSendBuf[i]); 
		strMsg += strLog;
	}

	strLog = "SEND : ";
	strLog += strMsg;

	m_pFFUMon->WriteLogList(strLog, m_nMode);

	res = WaitForSingleObject(m_hEvtRcv, 10000);
	if( res == WAIT_TIMEOUT)
	{
		return 0;
	}
	
	m_pFFUMon->m_iAzbliValue[0] = m_nRcvValue;
	//////////////////////////////////////////////////////////////////////////


	memset(btSendBuf, 0, AZBIL_0150_REQUEST_SIZE);	

	//Command Protocalls		All Flow Data
	btSendBuf[0] = 0x3A;			//Start Byte
	btSendBuf[1] = 0x30;			//High Byte Address
	btSendBuf[2] = 0x32;			//Low Byte Address
	btSendBuf[3] = 0x30;			//Function Code
	btSendBuf[4] = 0x33;			//Function Code
	//Data
	btSendBuf[5] = 0x30;
	btSendBuf[6] = 0x34;
	btSendBuf[7] = 0x42;
	btSendBuf[8] = 0x35;
	btSendBuf[9] = 0x30;
	btSendBuf[10] = 0x30;
	btSendBuf[11] = 0x30;
	btSendBuf[12] = 0x32;

	btSendBuf[13] = 0x34;		//LRC, Error Code
	btSendBuf[14] = 0x30;
	btSendBuf[15] = 0x0D;		//CR, 의미없는 데이터	
	btSendBuf[16] = 0x0A;

	if( api_SerialCom_IsOpened(m_nPort))
		api_SerialCom_WriteByte(m_nPort, btSendBuf, AZBIL_0150_REQUEST_SIZE);

	for(int i=0; i<AZBIL_0150_REQUEST_SIZE; i++)
	{
		strLog.Format(_T("%02X "), btSendBuf[i]); 
		strMsg += strLog;
	}

	strLog = "SEND : ";
	strLog += strMsg;

	m_pFFUMon->WriteLogList(strLog, m_nMode);

	res = WaitForSingleObject(m_hEvtRcv, 10000);
	if( res == WAIT_TIMEOUT)
	{
		return 0;
	}

	m_pFFUMon->m_iAzbliValue[1] = m_nRcvValue;
	//////////////////////////////////////////////////////////////////////////

	memset(btSendBuf, 0, AZBIL_0150_REQUEST_SIZE);	

	//Command Protocalls		All Flow Data
	btSendBuf[0] = 0x3A;			//Start Byte
	btSendBuf[1] = 0x30;			//High Byte Address
	btSendBuf[2] = 0x33;			//Low Byte Address
	btSendBuf[3] = 0x30;			//Function Code
	btSendBuf[4] = 0x33;			//Function Code
	//Data
	btSendBuf[5] = 0x30;
	btSendBuf[6] = 0x34;
	btSendBuf[7] = 0x42;
	btSendBuf[8] = 0x35;
	btSendBuf[9] = 0x30;
	btSendBuf[10] = 0x30;
	btSendBuf[11] = 0x30;
	btSendBuf[12] = 0x32;

	btSendBuf[13] = 0x33;		//LRC, Error Code
	btSendBuf[14] = 0x46;
	btSendBuf[15] = 0x0D;		//CR, 의미없는 데이터	
	btSendBuf[16] = 0x0A;

	if( api_SerialCom_IsOpened(m_nPort))
		api_SerialCom_WriteByte(m_nPort, btSendBuf, AZBIL_0150_REQUEST_SIZE);

	for(int i=0; i<AZBIL_0150_REQUEST_SIZE; i++)
	{
		strLog.Format(_T("%02X "), btSendBuf[i]); 
		strMsg += strLog;
	}

	strLog = "SEND : ";
	strLog += strMsg;

	m_pFFUMon->WriteLogList(strLog, m_nMode);

	res = WaitForSingleObject(m_hEvtRcv, 10000);
	if( res == WAIT_TIMEOUT)
	{
		return 0;
	}

	m_pFFUMon->m_iAzbliValue[2] = m_nRcvValue;
	//////////////////////////////////////////////////////////////////////////

	memset(btSendBuf, 0, AZBIL_0150_REQUEST_SIZE);	

	//Command Protocalls		All Flow Data
	btSendBuf[0] = 0x3A;			//Start Byte
	btSendBuf[1] = 0x30;			//High Byte Address
	btSendBuf[2] = 0x34;			//Low Byte Address
	btSendBuf[3] = 0x30;			//Function Code
	btSendBuf[4] = 0x33;			//Function Code
	//Data
	btSendBuf[5] = 0x30;
	btSendBuf[6] = 0x34;
	btSendBuf[7] = 0x42;
	btSendBuf[8] = 0x35;
	btSendBuf[9] = 0x30;
	btSendBuf[10] = 0x30;
	btSendBuf[11] = 0x30;
	btSendBuf[12] = 0x32;

	btSendBuf[13] = 0x33;		//LRC, Error Code
	btSendBuf[14] = 0x45;
	btSendBuf[15] = 0x0D;		//CR, 의미없는 데이터	
	btSendBuf[16] = 0x0A;

	if( api_SerialCom_IsOpened(m_nPort))
		api_SerialCom_WriteByte(m_nPort, btSendBuf, AZBIL_0150_REQUEST_SIZE);

	for(int i=0; i<AZBIL_0150_REQUEST_SIZE; i++)
	{
		strLog.Format(_T("%02X "), btSendBuf[i]); 
		strMsg += strLog;
	}

	strLog = "SEND : ";
	strLog += strMsg;

	m_pFFUMon->WriteLogList(strLog, m_nMode);

	res = WaitForSingleObject(m_hEvtRcv, 10000);
	if( res == WAIT_TIMEOUT)
	{
		return 0;
	}

	m_pFFUMon->m_iAzbliValue[3] = m_nRcvValue;
	//////////////////////////////////////////////////////////////////////////

	memset(btSendBuf, 0, AZBIL_0150_REQUEST_SIZE);	

	//Command Protocalls		All Flow Data
	btSendBuf[0] = 0x3A;			//Start Byte
	btSendBuf[1] = 0x30;			//High Byte Address
	btSendBuf[2] = 0x35;			//Low Byte Address
	btSendBuf[3] = 0x30;			//Function Code
	btSendBuf[4] = 0x33;			//Function Code
	//Data
	btSendBuf[5] = 0x30;
	btSendBuf[6] = 0x34;
	btSendBuf[7] = 0x42;
	btSendBuf[8] = 0x35;
	btSendBuf[9] = 0x30;
	btSendBuf[10] = 0x30;
	btSendBuf[11] = 0x30;
	btSendBuf[12] = 0x32;

	btSendBuf[13] = 0x33;		//LRC, Error Code
	btSendBuf[14] = 0x44;
	btSendBuf[15] = 0x0D;		//CR, 의미없는 데이터	
	btSendBuf[16] = 0x0A;

	if( api_SerialCom_IsOpened(m_nPort))
		api_SerialCom_WriteByte(m_nPort, btSendBuf, AZBIL_0150_REQUEST_SIZE);

	for(int i=0; i<AZBIL_0150_REQUEST_SIZE; i++)
	{
		strLog.Format(_T("%02X "), btSendBuf[i]); 
		strMsg += strLog;
	}

	strLog = "SEND : ";
	strLog += strMsg;

	m_pFFUMon->WriteLogList(strLog, m_nMode);

	res = WaitForSingleObject(m_hEvtRcv, 10000);
	if( res == WAIT_TIMEOUT)
	{
		return 0;
	}

	m_pFFUMon->m_iAzbliValue[4] = m_nRcvValue;
	//////////////////////////////////////////////////////////////////////////

	return 1;

}

#define PM5500_RECV_BUFFER_SIZE 265			//hh.joo 20180710 add for PM5500
#define PM5500_REQUEST_CMD_SIZE_MODBUS	8	//hh.joo 20180710 add for PM5500

int CDlgComm::PM5500_WriteRead(void)
{
	PM5500_WriteRead_TotalPower();	//Total power
	
	PM5500_WriteRead_ActPowTotal();	//Instantaneous power
	
	PM5500_WriteRead_RS_line_volt();
	
	PM5500_WriteRead_ST_line_volt();
	
	PM5500_WriteRead_TR_line_volt();
	
	PM5500_WriteRead_R_phase_curr();
	
	PM5500_WriteRead_S_phase_curr();
	
	PM5500_WriteRead_T_phase_curr();

	PM5500_WriteRead_Electric_cur_I();	

	PM5500_WriteRead_Electric_volt_U();	

	PM5500_WriteRead_Active_Power_W();	

	PM5500_WriteRead_Power_factor_cso();

	PM5500_WriteRead_Frequency_f();		

	PM5500_WriteRead_THDi();			

	PM5500_WriteRead_THDu();	

	PM5500_WriteRead_THDi_A();

	PM5500_WriteRead_THDi_B();

	PM5500_WriteRead_THDi_C();

	return 1;
}


int CDlgComm::PM5500_WriteRead_TotalPower(void)
{
	CString strLog, strMsg;
	DWORD res = 0;

	UINT uBufMax = PM5500_REQUEST_CMD_SIZE_MODBUS;
	BYTE byteuf[PM5500_REQUEST_CMD_SIZE_MODBUS];	

	ZeroMemory(byteuf, sizeof(BYTE)*PM5500_REQUEST_CMD_SIZE_MODBUS);

	//0x0C87 //Register = 3207
	byteuf[0] = 0x01;	//Slave
	byteuf[1] = 0x03;	//function
	byteuf[2] = 0x0C;	//Starting addr Hi
	byteuf[3] = 0x87;	//Starting addr Lo
	byteuf[4] = 0x00;	//Registers Hi
	byteuf[5] = 0x02;	//Registers Lo
	byteuf[6] = 0x00;	//CRC
	byteuf[7] = 0x00;	//CRC

	
// 	CRC Maker
	MakerCRC(byteuf);

// 		//////////////////////////////////////////////////////////////////////////
// 		{
// 			WORD wCrc = 65535;	//0xFFFF
// 			BYTE btD;
// 			WORD wCY, wDD;
// 			int i;
// 	
// 			for (i = 0 ; i < 6 ; i++)
// 			{
// 				btD =byteuf[i];
// 				wCrc = wCrc ^ btD;		//Exclusive OR
// 	
// 				for(int j = 1 ; j <= 8 ; j++)
// 				{
// 					wCY = wCrc & 1 ; wCrc = wCrc / 2;
// 					if(wCY == 1)
// 						wCrc = wCrc ^ 40961;	//40961 : OxA001 //Exclusive OR 
// 				}
// 			}
// 	
// 			wDD = wCrc / 256;
// 			byteuf[7] = (byte)	wDD;				//CRC high
// 			byteuf[6] = (byte)	(wCrc - wDD * 256);	//CRC low
// 		}

	if( api_SerialCom_IsOpened(m_nPort))
		api_SerialCom_WriteByte(m_nPort, byteuf, PM5500_REQUEST_CMD_SIZE_MODBUS);

	for(int i=0; i<PM5500_REQUEST_CMD_SIZE_MODBUS; i++)
	{
		strLog.Format(_T("%02X "), byteuf[i]); 
		strMsg += strLog;
	}

	strLog = "SEND : ";
	strLog += strMsg;

	m_pFFUMon->WriteLogList(strLog, m_nMode);

	res = WaitForSingleObject(m_hEvtRcv, 10000);
	if( res == WAIT_TIMEOUT)
	{
		return 0;
	}

	m_pFFUMon->m_sData.nValueTotal[0] = m_dRcvValue;

	return 1;
}


int CDlgComm::PM5500_WriteRead_ActPowTotal(void)
{
	CString strLog, strMsg;
	DWORD res = 0;

	UINT uBufMax = PM5500_REQUEST_CMD_SIZE_MODBUS;
	BYTE byteuf[PM5500_REQUEST_CMD_SIZE_MODBUS];	

	ZeroMemory(byteuf, sizeof(BYTE)*PM5500_REQUEST_CMD_SIZE_MODBUS);

	//PM5500 Active Power Total		//0x0BF3 //Register = 3059	
	byteuf[0] = 0x01;	//Slave
	byteuf[1] = 0x03;	//function
	byteuf[2] = 0x0B;	//Starting addr Hi
	byteuf[3] = 0xF3;	//Starting addr Lo
	byteuf[4] = 0x00;	//Registers Hi
	byteuf[5] = 0x02;	//Registers Lo
	byteuf[6] = 0x00;	//CRC
	byteuf[7] = 0x00;	//CRC

	//CRC Maker
	MakerCRC(byteuf);

	if( api_SerialCom_IsOpened(m_nPort))
		api_SerialCom_WriteByte(m_nPort, byteuf, PM5500_REQUEST_CMD_SIZE_MODBUS);

	for(int i=0; i<PM5500_REQUEST_CMD_SIZE_MODBUS; i++)
	{
		strLog.Format(_T("%02X "), byteuf[i]); 
		strMsg += strLog;
	}

	strLog = "SEND : ";
	strLog += strMsg;

	m_pFFUMon->WriteLogList(strLog, m_nMode);

	res = WaitForSingleObject(m_hEvtRcv, 10000);
	if( res == WAIT_TIMEOUT)
	{
		return 0;
	}

	m_pFFUMon->m_sData.nValueTotal[0] = m_dRcvValue;

	return 1;
}


int CDlgComm::PM5500_WriteRead_RS_line_volt(void)
{
	CString strLog, strMsg;
	DWORD res = 0;

	UINT uBufMax = PM5500_REQUEST_CMD_SIZE_MODBUS;
	BYTE byteuf[PM5500_REQUEST_CMD_SIZE_MODBUS];	

	ZeroMemory(byteuf, sizeof(BYTE)*PM5500_REQUEST_CMD_SIZE_MODBUS);

	//0x0BCB //Register = 3019
	byteuf[0] = 0x01;	//Slave
	byteuf[1] = 0x03;	//function
	byteuf[2] = 0x0B;	//Starting addr Hi
	byteuf[3] = 0xCB;	//Starting addr Lo
	byteuf[4] = 0x00;	//Registers Hi
	byteuf[5] = 0x02;	//Registers Lo
	byteuf[6] = 0x00;	//CRC
	byteuf[7] = 0x00;	//CRC

	//CRC Maker
	MakerCRC(byteuf);

	if( api_SerialCom_IsOpened(m_nPort))
		api_SerialCom_WriteByte(m_nPort, byteuf, PM5500_REQUEST_CMD_SIZE_MODBUS);

	for(int i=0; i<PM5500_REQUEST_CMD_SIZE_MODBUS; i++)
	{
		strLog.Format(_T("%02X "), byteuf[i]); 
		strMsg += strLog;
	}

	strLog = "SEND : ";
	strLog += strMsg;

	m_pFFUMon->WriteLogList(strLog, m_nMode);

	res = WaitForSingleObject(m_hEvtRcv, 10000);
	if( res == WAIT_TIMEOUT)
	{
		return 0;
	}

	m_pFFUMon->m_sData.nValueTotal[0] = m_dRcvValue;

	return 1;
}


int CDlgComm::PM5500_WriteRead_ST_line_volt(void)
{
	CString strLog, strMsg;
	DWORD res = 0;

	UINT uBufMax = PM5500_REQUEST_CMD_SIZE_MODBUS;
	BYTE byteuf[PM5500_REQUEST_CMD_SIZE_MODBUS];	

	ZeroMemory(byteuf, sizeof(BYTE)*PM5500_REQUEST_CMD_SIZE_MODBUS);

	//0x0C87 //Register = 3021
	byteuf[0] = 0x01;	//Slave
	byteuf[1] = 0x03;	//function
	byteuf[2] = 0x0C;	//Starting addr Hi
	byteuf[3] = 0x87;	//Starting addr Lo
	byteuf[4] = 0x00;	//Registers Hi
	byteuf[5] = 0x02;	//Registers Lo
	byteuf[6] = 0x00;	//CRC
	byteuf[7] = 0x00;	//CRC

	//CRC Maker
	MakerCRC(byteuf);

	if( api_SerialCom_IsOpened(m_nPort))
		api_SerialCom_WriteByte(m_nPort, byteuf, PM5500_REQUEST_CMD_SIZE_MODBUS);

	for(int i=0; i<PM5500_REQUEST_CMD_SIZE_MODBUS; i++)
	{
		strLog.Format(_T("%02X "), byteuf[i]); 
		strMsg += strLog;
	}

	strLog = "SEND : ";
	strLog += strMsg;

	m_pFFUMon->WriteLogList(strLog, m_nMode);

	res = WaitForSingleObject(m_hEvtRcv, 10000);
	if( res == WAIT_TIMEOUT)
	{
		return 0;
	}

	m_pFFUMon->m_sData.nValueTotal[0] = m_dRcvValue;

	return 1;
}


int CDlgComm::PM5500_WriteRead_TR_line_volt(void)
{
	CString strLog, strMsg;
	DWORD res = 0;

	UINT uBufMax = PM5500_REQUEST_CMD_SIZE_MODBUS;
	BYTE byteuf[PM5500_REQUEST_CMD_SIZE_MODBUS];	

	ZeroMemory(byteuf, sizeof(BYTE)*PM5500_REQUEST_CMD_SIZE_MODBUS);

	//0x0BCF //Register = 3023
	byteuf[0] = 0x01;	//Slave
	byteuf[1] = 0x03;	//function
	byteuf[2] = 0x0B;	//Starting addr Hi
	byteuf[3] = 0xCF;	//Starting addr Lo
	byteuf[4] = 0x00;	//Registers Hi
	byteuf[5] = 0x02;	//Registers Lo
	byteuf[6] = 0x00;	//CRC
	byteuf[7] = 0x00;	//CRC

	//CRC Maker
	MakerCRC(byteuf);

	if( api_SerialCom_IsOpened(m_nPort))
		api_SerialCom_WriteByte(m_nPort, byteuf, PM5500_REQUEST_CMD_SIZE_MODBUS);

	for(int i=0; i<PM5500_REQUEST_CMD_SIZE_MODBUS; i++)
	{
		strLog.Format(_T("%02X "), byteuf[i]); 
		strMsg += strLog;
	}

	strLog = "SEND : ";
	strLog += strMsg;

	m_pFFUMon->WriteLogList(strLog, m_nMode);

	res = WaitForSingleObject(m_hEvtRcv, 10000);
	if( res == WAIT_TIMEOUT)
	{
		return 0;
	}

	m_pFFUMon->m_cDATA_PM55.m_dTR_line_volt = m_dRcvValue;

	return 1;
}


int CDlgComm::PM5500_WriteRead_R_phase_curr(void)
{
	CString strLog, strMsg;
	DWORD res = 0;

	UINT uBufMax = PM5500_REQUEST_CMD_SIZE_MODBUS;
	BYTE byteuf[PM5500_REQUEST_CMD_SIZE_MODBUS];	

	ZeroMemory(byteuf, sizeof(BYTE)*PM5500_REQUEST_CMD_SIZE_MODBUS);

	//0x0BB7 //Register = 2999
	byteuf[0] = 0x01;	//Slave
	byteuf[1] = 0x03;	//function
	byteuf[2] = 0x0B;	//Starting addr Hi
	byteuf[3] = 0xB7;	//Starting addr Lo
	byteuf[4] = 0x00;	//Registers Hi
	byteuf[5] = 0x02;	//Registers Lo
	byteuf[6] = 0x00;	//CRC
	byteuf[7] = 0x00;	//CRC

	//CRC Maker
	MakerCRC(byteuf);

	if( api_SerialCom_IsOpened(m_nPort))
		api_SerialCom_WriteByte(m_nPort, byteuf, PM5500_REQUEST_CMD_SIZE_MODBUS);

	for(int i=0; i<PM5500_REQUEST_CMD_SIZE_MODBUS; i++)
	{
		strLog.Format(_T("%02X "), byteuf[i]); 
		strMsg += strLog;
	}

	strLog = "SEND : ";
	strLog += strMsg;

	m_pFFUMon->WriteLogList(strLog, m_nMode);

	res = WaitForSingleObject(m_hEvtRcv, 10000);
	if( res == WAIT_TIMEOUT)
	{
		return 0;
	}

	m_pFFUMon->m_cDATA_PM55.m_dR_Phase_curr = m_dRcvValue;

	return 1;
}


int CDlgComm::PM5500_WriteRead_S_phase_curr(void)
{
	CString strLog, strMsg;
	DWORD res = 0;

	UINT uBufMax = PM5500_REQUEST_CMD_SIZE_MODBUS;
	BYTE byteuf[PM5500_REQUEST_CMD_SIZE_MODBUS];	

	ZeroMemory(byteuf, sizeof(BYTE)*PM5500_REQUEST_CMD_SIZE_MODBUS);

	//0x0BB9 //Register = 3001
	byteuf[0] = 0x01;	//Slave
	byteuf[1] = 0x03;	//function
	byteuf[2] = 0x0B;	//Starting addr Hi
	byteuf[3] = 0xB9;	//Starting addr Lo
	byteuf[4] = 0x00;	//Registers Hi
	byteuf[5] = 0x02;	//Registers Lo
	byteuf[6] = 0x00;	//CRC
	byteuf[7] = 0x00;	//CRC

	//CRC Maker
	MakerCRC(byteuf);

	if( api_SerialCom_IsOpened(m_nPort))
		api_SerialCom_WriteByte(m_nPort, byteuf, PM5500_REQUEST_CMD_SIZE_MODBUS);

	for(int i=0; i<PM5500_REQUEST_CMD_SIZE_MODBUS; i++)
	{
		strLog.Format(_T("%02X "), byteuf[i]); 
		strMsg += strLog;
	}

	strLog = "SEND : ";
	strLog += strMsg;

	m_pFFUMon->WriteLogList(strLog, m_nMode);

	res = WaitForSingleObject(m_hEvtRcv, 10000);
	if( res == WAIT_TIMEOUT)
	{
		return 0;
	}

	m_pFFUMon->m_cDATA_PM55.m_dS_Phase_curr = m_dRcvValue;

	return 1;
}


int CDlgComm::PM5500_WriteRead_T_phase_curr(void)
{
	CString strLog, strMsg;
	DWORD res = 0;

	UINT uBufMax = PM5500_REQUEST_CMD_SIZE_MODBUS;
	BYTE byteuf[PM5500_REQUEST_CMD_SIZE_MODBUS];	

	ZeroMemory(byteuf, sizeof(BYTE)*PM5500_REQUEST_CMD_SIZE_MODBUS);

	//0x0BBB //Register = 3003
	byteuf[0] = 0x01;	//Slave
	byteuf[1] = 0x03;	//function
	byteuf[2] = 0x0B;	//Starting addr Hi
	byteuf[3] = 0xBB;	//Starting addr Lo
	byteuf[4] = 0x00;	//Registers Hi
	byteuf[5] = 0x02;	//Registers Lo
	byteuf[6] = 0x00;	//CRC
	byteuf[7] = 0x00;	//CRC

	//CRC Maker
	MakerCRC(byteuf);

	if( api_SerialCom_IsOpened(m_nPort))
		api_SerialCom_WriteByte(m_nPort, byteuf, PM5500_REQUEST_CMD_SIZE_MODBUS);

	for(int i=0; i<PM5500_REQUEST_CMD_SIZE_MODBUS; i++)
	{
		strLog.Format(_T("%02X "), byteuf[i]); 
		strMsg += strLog;
	}

	strLog = "SEND : ";
	strLog += strMsg;

	m_pFFUMon->WriteLogList(strLog, m_nMode);

	res = WaitForSingleObject(m_hEvtRcv, 10000);
	if( res == WAIT_TIMEOUT)
	{
		return 0;
	}

	m_pFFUMon->m_cDATA_PM55.m_dT_Phase_curr = m_dRcvValue;

	return 1;
}


int CDlgComm::PM5500_WriteRead_Electric_cur_I(void)
{
	CString strLog, strMsg;
	DWORD res = 0;

	UINT uBufMax = PM5500_REQUEST_CMD_SIZE_MODBUS;
	BYTE byteuf[PM5500_REQUEST_CMD_SIZE_MODBUS];	

	ZeroMemory(byteuf, sizeof(BYTE)*PM5500_REQUEST_CMD_SIZE_MODBUS);

	//3009	//0x0BC1
	byteuf[0] = 0x01;	//Slave
	byteuf[1] = 0x03;	//function
	byteuf[2] = 0x0B;	//Starting addr Hi
	byteuf[3] = 0xC1;	//Starting addr Lo
	byteuf[4] = 0x00;	//Registers Hi
	byteuf[5] = 0x02;	//Registers Lo
	byteuf[6] = 0x00;	//CRC
	byteuf[7] = 0x00;	//CRC

	//CRC Maker
	MakerCRC(byteuf);

	if( api_SerialCom_IsOpened(m_nPort))
		api_SerialCom_WriteByte(m_nPort, byteuf, PM5500_REQUEST_CMD_SIZE_MODBUS);

	for(int i=0; i<PM5500_REQUEST_CMD_SIZE_MODBUS; i++)
	{
		strLog.Format(_T("%02X "), byteuf[i]); 
		strMsg += strLog;
	}

	strLog = "SEND : ";
	strLog += strMsg;

	m_pFFUMon->WriteLogList(strLog, m_nMode);

	res = WaitForSingleObject(m_hEvtRcv, 10000);
	if( res == WAIT_TIMEOUT)
	{
		return 0;
	}

	m_pFFUMon->m_cDATA_PM55.m_dElect_cur_I = m_dRcvValue;

	return 1;
}


int CDlgComm::PM5500_WriteRead_Electric_volt_U(void)
{
	CString strLog, strMsg;
	DWORD res = 0;

	UINT uBufMax = PM5500_REQUEST_CMD_SIZE_MODBUS;
	BYTE byteuf[PM5500_REQUEST_CMD_SIZE_MODBUS];	

	ZeroMemory(byteuf, sizeof(BYTE)*PM5500_REQUEST_CMD_SIZE_MODBUS);

	//3035	//0x0BDB	
	byteuf[0] = 0x01;	//Slave
	byteuf[1] = 0x03;	//function
	byteuf[2] = 0x0B;	//Starting addr Hi
	byteuf[3] = 0xDB;	//Starting addr Lo
	byteuf[4] = 0x00;	//Registers Hi
	byteuf[5] = 0x02;	//Registers Lo
	byteuf[6] = 0x00;	//CRC
	byteuf[7] = 0x00;	//CRC

	//CRC Maker
	MakerCRC(byteuf);

	if( api_SerialCom_IsOpened(m_nPort))
		api_SerialCom_WriteByte(m_nPort, byteuf, PM5500_REQUEST_CMD_SIZE_MODBUS);

	for(int i=0; i<PM5500_REQUEST_CMD_SIZE_MODBUS; i++)
	{
		strLog.Format(_T("%02X "), byteuf[i]); 
		strMsg += strLog;
	}

	strLog = "SEND : ";
	strLog += strMsg;

	m_pFFUMon->WriteLogList(strLog, m_nMode);

	res = WaitForSingleObject(m_hEvtRcv, 10000);
	if( res == WAIT_TIMEOUT)
	{
		return 0;
	}

	m_pFFUMon->m_cDATA_PM55.m_dElect_volt_U = m_dRcvValue;

	return 1;
}


int CDlgComm::PM5500_WriteRead_Active_Power_W(void)
{
	CString strLog, strMsg;
	DWORD res = 0;

	UINT uBufMax = PM5500_REQUEST_CMD_SIZE_MODBUS;
	BYTE byteuf[PM5500_REQUEST_CMD_SIZE_MODBUS];	

	ZeroMemory(byteuf, sizeof(BYTE)*PM5500_REQUEST_CMD_SIZE_MODBUS);

	//BED //BEF //BF1 //BF3
	byteuf[0] = 0x01;	//Slave
	byteuf[1] = 0x03;	//function
	byteuf[2] = 0x0B;	//Starting addr Hi
	byteuf[3] = 0xF3;	//Starting addr Lo
	byteuf[4] = 0x00;	//Registers Hi
	byteuf[5] = 0x02;	//Registers Lo
	byteuf[6] = 0x00;	//CRC
	byteuf[7] = 0x00;	//CRC

	//CRC Maker
	MakerCRC(byteuf);

	if( api_SerialCom_IsOpened(m_nPort))
		api_SerialCom_WriteByte(m_nPort, byteuf, PM5500_REQUEST_CMD_SIZE_MODBUS);

	for(int i=0; i<PM5500_REQUEST_CMD_SIZE_MODBUS; i++)
	{
		strLog.Format(_T("%02X "), byteuf[i]); 
		strMsg += strLog;
	}

	strLog = "SEND : ";
	strLog += strMsg;

	m_pFFUMon->WriteLogList(strLog, m_nMode);

	res = WaitForSingleObject(m_hEvtRcv, 10000);
	if( res == WAIT_TIMEOUT)
	{
		return 0;
	}

	m_pFFUMon->m_cDATA_PM55.m_dElect_act_pow = m_dRcvValue;

	return 1;
}


int CDlgComm::PM5500_WriteRead_Power_factor_cso(void)
{
	CString strLog, strMsg;
	DWORD res = 0;

	UINT uBufMax = PM5500_REQUEST_CMD_SIZE_MODBUS;
	BYTE byteuf[PM5500_REQUEST_CMD_SIZE_MODBUS];	

	ZeroMemory(byteuf, sizeof(BYTE)*PM5500_REQUEST_CMD_SIZE_MODBUS);

	//0x0C0B //Register = //3083
	byteuf[0] = 0x01;	//Slave
	byteuf[1] = 0x03;	//function
	byteuf[2] = 0x0C;	//Starting addr Hi
	byteuf[3] = 0x0B;	//Starting addr Lo
	byteuf[4] = 0x00;	//Registers Hi
	byteuf[5] = 0x02;	//Registers Lo
	byteuf[6] = 0x00;	//CRC
	byteuf[7] = 0x00;	//CRC

	//CRC Maker
	MakerCRC(byteuf);

	if( api_SerialCom_IsOpened(m_nPort))
		api_SerialCom_WriteByte(m_nPort, byteuf, PM5500_REQUEST_CMD_SIZE_MODBUS);

	for(int i=0; i<PM5500_REQUEST_CMD_SIZE_MODBUS; i++)
	{
		strLog.Format(_T("%02X "), byteuf[i]); 
		strMsg += strLog;
	}

	strLog = "SEND : ";
	strLog += strMsg;

	m_pFFUMon->WriteLogList(strLog, m_nMode);

	res = WaitForSingleObject(m_hEvtRcv, 10000);
	if( res == WAIT_TIMEOUT)
	{
		return 0;
	}

	m_pFFUMon->m_cDATA_PM55.m_dPower_factor_cso = m_dRcvValue;

	return 1;
}


int CDlgComm::PM5500_WriteRead_Frequency_f(void)
{
	CString strLog, strMsg;
	DWORD res = 0;

	UINT uBufMax = PM5500_REQUEST_CMD_SIZE_MODBUS;
	BYTE byteuf[PM5500_REQUEST_CMD_SIZE_MODBUS];	

	ZeroMemory(byteuf, sizeof(BYTE)*PM5500_REQUEST_CMD_SIZE_MODBUS);

	//0x0C25 //Register = //3109
	byteuf[0] = 0x01;	//Slave
	byteuf[1] = 0x03;	//function
	byteuf[2] = 0x0C;	//Starting addr Hi
	byteuf[3] = 0x25;	//Starting addr Lo
	byteuf[4] = 0x00;	//Registers Hi
	byteuf[5] = 0x02;	//Registers Lo
	byteuf[6] = 0x00;	//CRC
	byteuf[7] = 0x00;	//CRC

	//CRC Maker
	MakerCRC(byteuf);

	if( api_SerialCom_IsOpened(m_nPort))
		api_SerialCom_WriteByte(m_nPort, byteuf, PM5500_REQUEST_CMD_SIZE_MODBUS);

	for(int i=0; i<PM5500_REQUEST_CMD_SIZE_MODBUS; i++)
	{
		strLog.Format(_T("%02X "), byteuf[i]); 
		strMsg += strLog;
	}

	strLog = "SEND : ";
	strLog += strMsg;

	m_pFFUMon->WriteLogList(strLog, m_nMode);

	res = WaitForSingleObject(m_hEvtRcv, 10000);
	if( res == WAIT_TIMEOUT)
	{
		return 0;
	}

	m_pFFUMon->m_cDATA_PM55.m_dFrequency = m_dRcvValue;

	return 1;
}


int CDlgComm::PM5500_WriteRead_THDi(void)
{
	CString strLog, strMsg;
	DWORD res = 0;

	UINT uBufMax = PM5500_REQUEST_CMD_SIZE_MODBUS;
	BYTE byteuf[PM5500_REQUEST_CMD_SIZE_MODBUS];	

	ZeroMemory(byteuf, sizeof(BYTE)*PM5500_REQUEST_CMD_SIZE_MODBUS);

	byteuf[0] = 0x01;	//Slave
	byteuf[1] = 0x03;	//function
	byteuf[2] = 0x53;	//Starting addr Hi
	byteuf[3] = 0x3B;	//Starting addr Lo
	byteuf[4] = 0x00;	//Registers Hi
	byteuf[5] = 0x02;	//Registers Lo
	byteuf[6] = 0x00;	//CRC
	byteuf[7] = 0x00;	//CRC

	//CRC Maker
	MakerCRC(byteuf);

	if( api_SerialCom_IsOpened(m_nPort))
		api_SerialCom_WriteByte(m_nPort, byteuf, PM5500_REQUEST_CMD_SIZE_MODBUS);

	for(int i=0; i<PM5500_REQUEST_CMD_SIZE_MODBUS; i++)
	{
		strLog.Format(_T("%02X "), byteuf[i]); 
		strMsg += strLog;
	}

	strLog = "SEND : ";
	strLog += strMsg;

	m_pFFUMon->WriteLogList(strLog, m_nMode);

	res = WaitForSingleObject(m_hEvtRcv, 10000);
	if( res == WAIT_TIMEOUT)
	{
		return 0;
	}

	m_pFFUMon->m_cDATA_PM55.m_dTHDi = m_dRcvValue;

	return 1;
}


int CDlgComm::PM5500_WriteRead_THDu(void)
{
	CString strLog, strMsg;
	DWORD res = 0;

	UINT uBufMax = PM5500_REQUEST_CMD_SIZE_MODBUS;
	BYTE byteuf[PM5500_REQUEST_CMD_SIZE_MODBUS];	

	ZeroMemory(byteuf, sizeof(BYTE)*PM5500_REQUEST_CMD_SIZE_MODBUS);

	//21328-1	//0x534F
	byteuf[0] = 0x01;	//Slave
	byteuf[1] = 0x03;	//function
	byteuf[2] = 0x53;	//Starting addr Hi
	byteuf[3] = 0x4F;	//Starting addr Lo
	byteuf[4] = 0x00;	//Registers Hi
	byteuf[5] = 0x02;	//Registers Lo
	byteuf[6] = 0x00;	//CRC
	byteuf[7] = 0x00;	//CRC

	//CRC Maker
	MakerCRC(byteuf);

	if( api_SerialCom_IsOpened(m_nPort))
		api_SerialCom_WriteByte(m_nPort, byteuf, PM5500_REQUEST_CMD_SIZE_MODBUS);

	for(int i=0; i<PM5500_REQUEST_CMD_SIZE_MODBUS; i++)
	{
		strLog.Format(_T("%02X "), byteuf[i]); 
		strMsg += strLog;
	}

	strLog = "SEND : ";
	strLog += strMsg;

	m_pFFUMon->WriteLogList(strLog, m_nMode);

	res = WaitForSingleObject(m_hEvtRcv, 10000);
	if( res == WAIT_TIMEOUT)
	{
		return 0;
	}

	m_pFFUMon->m_cDATA_PM55.m_dTHDu = m_dRcvValue;

	return 1;
}

int CDlgComm::PM5500_WriteRead_THDi_A(void)
{
	CString strLog, strMsg;
	DWORD res = 0;

	UINT uBufMax = PM5500_REQUEST_CMD_SIZE_MODBUS;
	BYTE byteuf[PM5500_REQUEST_CMD_SIZE_MODBUS];	

	ZeroMemory(byteuf, sizeof(BYTE)*PM5500_REQUEST_CMD_SIZE_MODBUS);

	byteuf[0] = 0x01;	//Slave
	byteuf[1] = 0x03;	//function
	byteuf[2] = 0x53;	//Starting addr Hi
	byteuf[3] = 0x3B;	//Starting addr Lo
	byteuf[4] = 0x00;	//Registers Hi
	byteuf[5] = 0x02;	//Registers Lo
	byteuf[6] = 0x00;	//CRC
	byteuf[7] = 0x00;	//CRC

	//CRC Maker
	MakerCRC(byteuf);

	if( api_SerialCom_IsOpened(m_nPort))
		api_SerialCom_WriteByte(m_nPort, byteuf, PM5500_REQUEST_CMD_SIZE_MODBUS);

	for(int i=0; i<PM5500_REQUEST_CMD_SIZE_MODBUS; i++)
	{
		strLog.Format(_T("%02X "), byteuf[i]); 
		strMsg += strLog;
	}

	strLog = "SEND : ";
	strLog += strMsg;

	m_pFFUMon->WriteLogList(strLog, m_nMode);

	res = WaitForSingleObject(m_hEvtRcv, 10000);
	if( res == WAIT_TIMEOUT)
	{
		return 0;
	}

	m_pFFUMon->m_cDATA_PM55.m_dTHDi_A = m_dRcvValue;

	return 1;
}


int CDlgComm::PM5500_WriteRead_THDi_B(void)
{
	CString strLog, strMsg;
	DWORD res = 0;

	UINT uBufMax = PM5500_REQUEST_CMD_SIZE_MODBUS;
	BYTE byteuf[PM5500_REQUEST_CMD_SIZE_MODBUS];	

	ZeroMemory(byteuf, sizeof(BYTE)*PM5500_REQUEST_CMD_SIZE_MODBUS);

	byteuf[0] = 0x01;	//Slave
	byteuf[1] = 0x03;	//function
	byteuf[2] = 0x53;	//Starting addr Hi
	byteuf[3] = 0x35;	//Starting addr Lo
	byteuf[4] = 0x00;	//Registers Hi
	byteuf[5] = 0x02;	//Registers Lo
	byteuf[6] = 0x00;	//CRC
	byteuf[7] = 0x00;	//CRC

	//CRC Maker
	MakerCRC(byteuf);

	if( api_SerialCom_IsOpened(m_nPort))
		api_SerialCom_WriteByte(m_nPort, byteuf, PM5500_REQUEST_CMD_SIZE_MODBUS);

	for(int i=0; i<PM5500_REQUEST_CMD_SIZE_MODBUS; i++)
	{
		strLog.Format(_T("%02X "), byteuf[i]); 
		strMsg += strLog;
	}

	strLog = "SEND : ";
	strLog += strMsg;

	m_pFFUMon->WriteLogList(strLog, m_nMode);

	res = WaitForSingleObject(m_hEvtRcv, 10000);
	if( res == WAIT_TIMEOUT)
	{
		return 0;
	}

	m_pFFUMon->m_cDATA_PM55.m_dTHDi_B = m_dRcvValue;

	return 1;
}


int CDlgComm::PM5500_WriteRead_THDi_C(void)
{
	CString strLog, strMsg;
	DWORD res = 0;

	UINT uBufMax = PM5500_REQUEST_CMD_SIZE_MODBUS;
	BYTE byteuf[PM5500_REQUEST_CMD_SIZE_MODBUS];	

	ZeroMemory(byteuf, sizeof(BYTE)*PM5500_REQUEST_CMD_SIZE_MODBUS);

	byteuf[0] = 0x01;	//Slave
	byteuf[1] = 0x03;	//function
	byteuf[2] = 0x53;	//Starting addr Hi
	byteuf[3] = 0x37;	//Starting addr Lo
	byteuf[4] = 0x00;	//Registers Hi
	byteuf[5] = 0x02;	//Registers Lo
	byteuf[6] = 0x00;	//CRC
	byteuf[7] = 0x00;	//CRC

	//CRC Maker
	MakerCRC(byteuf);

	if( api_SerialCom_IsOpened(m_nPort))
		api_SerialCom_WriteByte(m_nPort, byteuf, PM5500_REQUEST_CMD_SIZE_MODBUS);

	for(int i=0; i<PM5500_REQUEST_CMD_SIZE_MODBUS; i++)
	{
		strLog.Format(_T("%02X "), byteuf[i]); 
		strMsg += strLog;
	}

	strLog = "SEND : ";
	strLog += strMsg;

	m_pFFUMon->WriteLogList(strLog, m_nMode);

	res = WaitForSingleObject(m_hEvtRcv, 10000);
	if( res == WAIT_TIMEOUT)
	{
		return 0;
	}

	m_pFFUMon->m_cDATA_PM55.m_dTHDi_C = m_dRcvValue;

	return 1;
}


int CDlgComm::MakerCRC(BYTE* pBytebuf)
{
	//CRC Maker
	//////////////////////////////////////////////////////////////////////////
	WORD wCrc = 65535;	//0xFFFF
	BYTE btD;
	WORD wCY, wDD;
	int i = 0;

	for (i = 0 ; i < 6 ; i++)
	{
		btD =pBytebuf[i];
		wCrc = wCrc ^ btD;		//Exclusive OR

		for(int j = 1 ; j <= 8 ; j++)
		{
			wCY = wCrc & 1 ; wCrc = wCrc / 2;
			if(wCY == 1)
				wCrc = wCrc ^ 40961;	//40961 : OxA001 //Exclusive OR 
		}
	}

	wDD = wCrc / 256;
	pBytebuf[7] = (byte)	wDD;				//CRC high
	pBytebuf[6] = (byte)	(wCrc - wDD * 256);	//CRC low

	return 0;
}





void CDlgComm::SetIonizer(vector<BYTE> vecRvcData)
{
	int iMsgSize = 0, i =0;
	CString strLog = _T("");
	CString strData = "";
	//CString strTemp[100];
	CString strTok;

	if(vecRvcData[4] == 'R' && vecRvcData[5] == 'U' &&  vecRvcData[6] == 'N')
	{
		strLog.Format(_T("RCV: %s (%d)"), strData,iMsgSize);
		m_pFFUMon->WriteLogList(strLog, SENSOR_MODE_IONIZER);
		return;
	}
	else if(vecRvcData[4] == 'S' && vecRvcData[5] == 'T' &&  vecRvcData[6] == 'P')
	{
		strLog.Format(_T("RCV: %s (%d)"), strData,iMsgSize);
		m_pFFUMon->WriteLogList(strLog, SENSOR_MODE_IONIZER);
		return;
	}

	iMsgSize = vecRvcData.size();

	for(i=0; i< iMsgSize; i++)
	{
		CString strTemp;
		strTemp.Format("%c",(char)vecRvcData[i]);
		strData +=strTemp;
	}

	strLog.Format(_T("RCV: %s (%d)"), strData,iMsgSize);
	//	m_pFFUMon->WriteLogList(strLog, m_nMode);		// sih 20180625 remove 

	if(m_nMode == SENSOR_MODE_IONIZER || m_nMode == SENSOR_MODE_IONIZER2 )
	{
		int cnt =0, nNum = 0, nStart = 0;

		if(m_nMode == SENSOR_MODE_IONIZER2 )
			nStart = 4;

		while(AfxExtractSubString(strTok,strData,cnt,','))
		{
			if( nStart+nNum > 7)
				break;

			if(cnt == 2 + (4*nNum))
			{
				m_pFFUMon->m_cDATA_IONIZER[nStart+nNum].bConnected =  _ttoi(strTok);
			}
			else if(cnt == 3 + (4*nNum))
			{
				m_pFFUMon->m_cDATA_IONIZER[nStart+nNum].dOverTime =  _ttof(strTok);
			}
			else if(cnt == 4 + (4*nNum))
			{
				m_pFFUMon->m_cDATA_IONIZER[nStart+nNum].bAlarm =  _ttoi(strTok);
			}
			else if(cnt == 5 + (4*nNum))
			{
				m_pFFUMon->m_cDATA_IONIZER[nStart+nNum].dRunTime =  _ttof(strTok);

				nNum++;
			}
			else
			{
				;
			}

			cnt++;	
		}
	}
}


void CDlgComm::Ionizor_WriteRead(void)
{
	BYTE packet[14];
	BYTE chsum = 0;

	packet[0] = '$';	// start code
	packet[1] = 'A';	// photo type ionizer 
	packet[2] = '9';	//model ionizer 7:T 8:2T 9:4T
	packet[3] = ',';	
	packet[4] = 'R';	//Command 
	packet[5] = 'E';	//Command 
	packet[6] = 'Q';	//Command 
	packet[7] = ',';	
	packet[8] = '1';   // Address 1,2,3 ~ G (16개)
	packet[9] = '*';	//end code 

	int i = 0;
	for(i=1; i<=8; i++)
	{
		if(i==1)
			chsum = packet[i];
		else
			chsum^=packet[i];
	}

	char ch[3];
	sprintf(ch,"%02X",chsum);
	packet[10] = ch[0]; // checksum
	packet[11] = ch[1]; // checksum
	packet[12] = '\r';
	packet[13] = '\n';

	BOOL bRet = FALSE;
	bRet = api_SerialCom_WriteByte(m_nPort, packet, 14);

	CString strLog, strMsg;
	for(i = 0; i<14; i++)
	{
		strLog.Format(_T("%02X "), packet[i] ); 
		strMsg += strLog;
	}

	strLog.Format("SEND(%d,%d) : %s", m_nPort, bRet, strMsg);

	m_pFFUMon->WriteLogList(strLog, SENSOR_MODE_IONIZER);
}


void CDlgComm::TestSendEFU(int nSpeed, int iGroup, int nStart, int nEnd, int nType)
{
	unsigned short crc16;
	BYTE byteEFU[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};	

	int nSize		= 8;

	// Get Speed
	byteEFU[0] = 0x01;	   // 유닛 아이디  
//	byteEFU[1] = 0x06;     // Write Single Registers
//	byteEFU[2] = 0x10;     // Status 1001~ // Real EFU SPEED 10FF ~  // Target EFU SPEED 12FB ~
//	byteEFU[3] = 0x5E; 
//	byteEFU[4] = 0x00; 
//	byteEFU[5] = 0x01;

	CString strLog, strMsg, strHex, strout;

	long value= 0;
	UINT iSV = nSpeed;
	strHex.Format(_T("%02X"), iSV);
	strout.Format(_T("%d"), strtol(strHex, NULL, 16));

	if( nStart == -1 && nEnd == -1)
	{
		if( nType == 0 ) // GET SPEED
		{
			byteEFU[1] = 0x03;
			byteEFU[2] = 0x10;
			byteEFU[3] = 0x20;
			byteEFU[4] = 0x00;
			byteEFU[5] = 0x00 + m_pFFUMon-> m_nEFUCnt[0];

			crc16 = CRC16_MODBUS(byteEFU, 6); 
			byteEFU[6] = (unsigned char)((crc16>>8) & 0x00ff);  //CRC 
			byteEFU[7] = (unsigned char)((crc16>>0) & 0x00ff); 
		}
		else if( nType == 2 ) // GET STATUS
		{
			byteEFU[1] = 0x03;
			byteEFU[2] = 0x10;
			byteEFU[3] = 0x01;
			byteEFU[4] = 0x00;
			byteEFU[5] = 0x00 + m_pFFUMon-> m_nEFUCnt[0];

			crc16 = CRC16_MODBUS(byteEFU, 6); 
			byteEFU[6] = (unsigned char)((crc16>>8) & 0x00ff);  //CRC 
			byteEFU[7] = (unsigned char)((crc16>>0) & 0x00ff); 
		}
		else
		{
			return;
		}

		m_nSendType = nType;

		if( api_SerialCom_IsOpened(m_nPort))
			api_SerialCom_WriteByte(m_nPort, byteEFU, nSize);

		for(int i=0; i<nSize; i++)
		{
			strLog.Format(_T("%02X "), byteEFU[i]); 
			strMsg += strLog;
		}

		strLog = "SEND : ";
		strLog += strMsg;

		m_pFFUMon->WriteLogList(strLog, m_nMode);

		return;
	}

	for(int j=0; j<32; j++)
	{
		strMsg.Empty();

		if( j+1 < nStart  || j+1 > nEnd ) 
			continue;

		if( nType == 1 )	//	RUN
		{
			byteEFU[1] = 0x06;
			byteEFU[2] = 0x10;
			byteEFU[3] = 0x3F + j;
			byteEFU[4] = 0x00;
			byteEFU[5] = 0x01;

			crc16 = CRC16_MODBUS(byteEFU, 6); 
			byteEFU[6] = (unsigned char)((crc16>>8) & 0x00ff);  //CRC 
			byteEFU[7] = (unsigned char)((crc16>>0) & 0x00ff); 
		}
		else if( nType == 2 ) // GET STATUS
		{
			byteEFU[1] = 0x03;
			byteEFU[2] = 0x10;
			byteEFU[3] = 0x01 + j;
			byteEFU[4] = 0x00;
			byteEFU[5] = 0x01;

			crc16 = CRC16_MODBUS(byteEFU, 6); 
			byteEFU[6] = (unsigned char)((crc16>>8) & 0x00ff);  //CRC 
			byteEFU[7] = (unsigned char)((crc16>>0) & 0x00ff); 
		}
		else if( nType == 3 ) // SET RPM
		{
			value = _tstoi(strout);
			byteEFU[1] = 0x06;
			byteEFU[2] = 0x10;
			byteEFU[3] = 0x5E + j;
			byteEFU[4] = (unsigned char)((value>>8) & 0x00ff);
			byteEFU[5] = (unsigned char)((value>>0) & 0x00ff);	// Target Value

			crc16 = CRC16_MODBUS(byteEFU, 6); 
			byteEFU[6] = (unsigned char)((crc16>>8) & 0x00ff);  //CRC 
			byteEFU[7] = (unsigned char)((crc16>>0) & 0x00ff); 
		}
		else if( nType == 0 ) // GET SPEED
		{
			byteEFU[1] = 0x03;
			byteEFU[2] = 0x10;
			byteEFU[3] = 0x20 + j;
			byteEFU[4] = 0x00;
			byteEFU[5] = 0x01;

			crc16 = CRC16_MODBUS(byteEFU, 6); 
			byteEFU[6] = (unsigned char)((crc16>>8) & 0x00ff);  //CRC 
			byteEFU[7] = (unsigned char)((crc16>>0) & 0x00ff); 
		}

		m_nSendType = nType;

		if( api_SerialCom_IsOpened(m_nPort))
			api_SerialCom_WriteByte(m_nPort, byteEFU, nSize);

		for(int i=0; i<nSize; i++)
		{
			strLog.Format(_T("%02X "), byteEFU[i]); 
			strMsg += strLog;
		}

		strLog = "SEND : ";
		strLog += strMsg;

		m_pFFUMon->WriteLogList(strLog, m_nMode);

		Sleep(500);
	}
}
