// DlgPLCDRead.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "FDCSense.h"
#include "DlgPLCDRead.h"
#include "afxdialogex.h"

#include "FDCSenseDlg.h"
//#include "FDCSharedDefine.h"
/*#include "FDCDefine.h"*/

#include <direct.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CFDCSenseDlg* g_pdlgMainPLCRead = NULL;

#define	PLC_READ_TIMEOUT 300
#define	dfMAX_PLC_IN_CNTD							110		/*95*/		// sih 20160711 change 20 -> 380 
#define PLC_READ_HEADER								"D00000FF03FF0006E40000"	/*"D00000FF03FF0005F40000"*/
#define PLC_CMD_ETHERNET_WORD_READ_D                "500000FF03FF000018009004010000"

// CDlgPLCDRead 대화 상자입니다.

IMPLEMENT_DYNAMIC(CDlgPLCDRead, CDialog)

CDlgPLCDRead::CDlgPLCDRead(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgPLCDRead::IDD, pParent)
{
	m_strReceiveLog.Empty();
//	memset(m_chReceiveLog, NULL, 2000);		// eh.kim 20170719 add.

	m_nMode = 0;
	m_nSocketIndex = 2;
	m_nStationNum  = 6;

	ZeroMemory(m_dPLCReadData,sizeof(m_dPLCReadData));	// jsyoon 20161023 change

	m_bReceiveOut = FALSE;
}

CDlgPLCDRead::~CDlgPLCDRead()
{
}

void CDlgPLCDRead::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PLCREAD_LOG, m_lbPlcLog);
}

void CDlgPLCDRead::SetMainPoint(void* pvoid)
{
	g_pdlgMainPLCRead = (CFDCSenseDlg*) pvoid;
}


BEGIN_MESSAGE_MAP(CDlgPLCDRead, CDialog)
	ON_BN_CLICKED(IDC_BTN_CONNET_PLC_D_READ, &CDlgPLCDRead::OnBnClickedBtnConnetPlcDRead)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

// CDlgPLCDRead 메시지 처리기입니다.

BOOL CDlgPLCDRead::OnInitDialog()
{
	CDialog::OnInitDialog();

	GetConfig();
	
	SOCKET_OPEN stSockOpen;
	stSockOpen.nSockMode = SOCKET_MODE_CLIENT;
	stSockOpen.nPort = m_nPlcPort;
	_tcscpy_s(stSockOpen.szIPAddr, _countof(stSockOpen.szIPAddr), m_szIPAddress);

	CreateSocket(stSockOpen);

	for(int i = 0; i < dfMAXFDCITEM; i++)
	{
		m_dPLCReadData[i] = 0.0;
	}

	return TRUE; 
}

void CDlgPLCDRead::OnBnClickedBtnConnetPlcDRead()
{
	GetConfig();

	SOCKET_OPEN stSockOpen;
	stSockOpen.nSockMode = SOCKET_MODE_CLIENT;
	stSockOpen.nPort = m_nPlcPort;
	_tcscpy_s(stSockOpen.szIPAddr, _countof(stSockOpen.szIPAddr), m_szIPAddress);
	CreateSocket(stSockOpen);

}

BOOL CDlgPLCDRead::DoWorkStartGetPLCMemoryBitAll()
{
	if (m_bOutOfService)
	{
		return TRUE;
	}
	
	int iHeader = 0;
	int iLen = 0;

	if( !api_SocketCtrl_IsConnected(m_nSocketIndex) )
		return FALSE;
	
	GetPLCMemoryBitAll();
	
	CString strReadDate;
	CString strReply;
	strReply.Empty();
	strReadDate.Empty();
	
	if(GetReceiveData(&strReply, dfMAX_PLC_IN_CNTD*16+22, PLC_READ_TIMEOUT))
	{
		iLen = strReply.GetLength();
		if(iLen < dfMAX_PLC_IN_CNTD*16+22)		
			return TRUE;
		
		strReadDate = strReply.Left(22);
		iHeader = strReply.Find(PLC_READ_HEADER);	
		if(iHeader<0)	
		{
			EventLog(strReply);
			strReply.ReleaseBuffer();
			return TRUE;
		}
		
		char	*StopStr;
		CString strLog, sTemp;
		CString strPLCData;
		strReadDate.Empty();
		strPLCData.Empty();
		int nDeviceNum = 0;
		
		if( iLen - (iHeader+22)  >= dfMAX_PLC_IN_CNTD*16 )
			strPLCData = strReply.Mid(iHeader+22, dfMAX_PLC_IN_CNTD*16);
		else
			return TRUE;
		
		int i = 0, j=0;
		for(i = 0; i < dfMAX_PLC_IN_CNTD*2; i++)		 
		{
			CString strSecond2Byte = strPLCData.Mid((i*8), 4);
			CString strFirst2Byte  = strPLCData.Mid((i*2+1)*4, 4);

//			CString strSecond2Byte = strPLCData.Mid((i-1)*8, 4);
//			CString strFirst2Byte  = strPLCData.Mid((i*2-1)*4, 4);
//			CString str1Word = strFirst2Byte + strSecond2Byte;			// eh.kim 20170206 remove.

			// eh.kim 20170206 add.
			char	str1Word[4096];
			memset(str1Word, NULL, 4096);
//			strcpy(str1Word, strFirst2Byte);
			_tcscpy_s(str1Word, _countof(str1Word), strFirst2Byte);
			strcat(str1Word, strSecond2Byte);
			//
			
			unsigned int CurData = strtoul(str1Word, &StopStr, 16);	
			
			float *RealData;
			RealData = (float*)&CurData;

			// jsyoon 20180607 add
			sTemp.Format("%s", g_pdlgMainPLCRead->m_spredFDC[j].cSourceAddress);
			nDeviceNum = atoi(sTemp.Mid(1));

			if( nDeviceNum == PLC_D_READ_STARY_BIT + 2*i)
			{
				m_dPLCReadData[j] = *RealData;		
				j++;
			}
			
//			m_dPLCReadData[i] = *RealData;		
		}
		strReply.ReleaseBuffer();
		
		return TRUE;
	}
	
	strReply.ReleaseBuffer();
	return FALSE;
}

BOOL CDlgPLCDRead::DoWorkStartGetPLCMemoryBitAll2()
{
	if (m_bOutOfService)
	{
		return TRUE;
	}

	int i=0, nDeviceNum = 0, j =0;
	long nRtn = 0; 
	long lTempData = 0;
	CString strDeviceText, sTemp;
	float fTempData[300];
	ZeroMemory(&fTempData, sizeof(lTempData));

	//nRtn = API_MXComponent_GetMemoryWordBlock(strDeviceText, nDeviceNum, 1, &lTempData);	//sypark 20180525 change
	nRtn = API_MXComponent_GetMemoryWordBlock(1, "D11500", 300, fTempData);

	for(i=0; i< 300/*g_pdlgMainPLCRead->nPLCCnt*/; i++)
	{	
		sTemp.Format("%s", g_pdlgMainPLCRead->m_spredFDC[j].cSourceAddress);
		nDeviceNum = atoi(sTemp.Mid(1));

		if( nDeviceNum == PLC_D_READ_STARY_BIT + 2*i)
		{
			m_dPLCReadData[j] = fTempData[i];		
			j++;
		}
	}

	CString strLog;
//	strLog.Format("RCV(%d), ", nRtn);
	strLog.Format("RCV(%d->%d), ", m_nStationNum, nRtn);
	for(i = 0; i<g_pdlgMainPLCRead->m_nPLCCnt;i ++)
	{
		sTemp.Format("%f,", m_dPLCReadData[i]);
		strLog +=sTemp;
	}

	g_pdlgMainPLCRead->m_dlgPLC.m_bStartSafetyPLCRead = TRUE;		// eh.kim 20171101 add.

	if(nRtn == 1)
	{
		EventLog(strLog);
		return TRUE;
	}


	return FALSE;
}
/*
BOOL CDlgPLCDRead::DoWorkStartGetPLCMemoryBitAll()
{
	if (m_bOutOfService)
	{
		return TRUE;
	}

	int i, nDeviceNum = 0;
	long nRtn = 0; 
	long lTempData = 0;
	CString strDeviceText, sTemp;
	char szValue[256];

	for(int i=0; i< g_pdlgMainPLCRead->nPLCCnt; i++)
	{
		sTemp.Format("%s", g_pdlgMainPLCRead->m_spredFDC[i].cSourceAddress);
		strDeviceText = sTemp.Left(1);

		int nLength = strDeviceText.GetLength() + 1;
		strcpy_s(szValue,nLength, strDeviceText);	//sypark 20180525 add
		nDeviceNum = atoi(sTemp.Mid(1));

		//nRtn = API_MXComponent_GetMemoryWordBlock(strDeviceText, nDeviceNum, 1, &lTempData);	//sypark 20180525 change
		nRtn = API_MXComponent_GetMemoryWordBlock((LPCTSTR)szValue, nDeviceNum, 1, &lTempData);
		m_dPLCReadData[i] = lTempData;
	} 

	int i=0;
	int nDeviceNum = 0;
	long nRtn = 0; 
	long lTempData[300];
	ZeroMemory(&lTempData, sizeof(lTempData));
//	long lTempData;

	CString strDeviceText, sTemp;
	TCHAR   szValue[256];			/// D, 
	szValue[0] = 0;
	int nLength =0;
	long lRet = 0;
// 	for(i=0; i< g_pdlgMainPLCRead->nPLCCnt; i++)
// 	{
// 		sTemp.Format("%s", g_pdlgMainPLCRead->m_spredFDC[i].cSourceAddress);
// //		strDeviceText = sTemp.Left(1);
// 		if( sTemp.IsEmpty() != TRUE)
// 		{
// 			sTemp.TrimLeft();
// 			sTemp.TrimRight();
// 			_tcscpy_s(szValue, _countof(szValue), sTemp);
// //			nLength = sTemp.GetLength();
// //			strDeviceText = sTemp.Left(1);
// //			_tcscpy_s(szValue, _countof(szValue), strDeviceText);
// //			nDeviceNum = atoi(sTemp.Mid(1, nLength-1));
// 
// //			nRtn = API_MXComponent_GetMemoryWordBlock((LPCTSTR)szValue, nDeviceNum, 1, &lTempData);
// 			nRtn = api_Melsoft_ReadDeviceWordBlock(1, szValue, 1, &lTempData, &lRet);
// //			Sleep(1);
// 			m_dPLCReadData[i] = lTempData;
// 		}
// 	}
	
	sTemp.Format("%s", g_pdlgMainPLCRead->m_spredFDC[0].cSourceAddress);
	sTemp.TrimLeft();
	sTemp.TrimRight();
	_tcscpy_s(szValue, _countof(szValue), sTemp);
	int nCnt;
	nCnt = g_pdlgMainPLCRead->nPLCCnt * 2;
	nRtn = api_Melsoft_ReadDeviceBitBlock(1, szValue, nCnt, lTempData, &lRet);
//	nRtn = api_Melsoft_ReadDeviceWordBlock(1, szValue, nCnt, lTempData, &lRet);

	CString strSecond2Byte;
	CString strFirst2Byte;
	CString Temp_Change_data;

//	Temp_Change_data = Receive_Data.Mid(index + 4, 4) + Receive_Data.Mid(index, 4);

	// 나중에 생각해 보자
	TCHAR	*StopStr;
	TCHAR	str1Word[4096];
	int mask = 1;
	int k = 0;
	TCHAR buf[8];
	TCHAR buf2[2];
	int ibuf = 0;
	long lTempData2[2];
	CString str, strTempH, strTempL;
	ULONG   lTempH = 0, lTempL = 0;
	float *RealData;
	for(i = 0; i<g_pdlgMainPLCRead->nPLCCnt;i ++)
	{
		strFirst2Byte.Empty();
		strSecond2Byte.Empty();

		mask = 0Xffff;
		for(k = 0; k < 2; k++)
		{
			ibuf = lTempData[2*i+1] & mask;
			ibuf >>= (16*k);
			memcpy(&buf2[k], &ibuf, sizeof(TCHAR)*2);
			mask <<= 0x10;
		}

		str.Empty();
		for(k = 0; k < 8; k++)
		{
			int cCh1 = (int)buf[k];
			strTempH .Format("%X",cCh1);
			str += strTempH;
		}
		

		mask = 0Xffff;
		for(k = 0; k < 2; k++)
		{
			ibuf = lTempData[2*i] & mask;
			ibuf >>= (16*k);

			memcpy(&buf2, &ibuf, sizeof(TCHAR)*2);

			buf[4+2*k]   = buf2[1];
			buf[4+2*k+1] = buf2[0];
			mask <<= 0x10;
		}
		


//		unsigned int CurData = 0;
//		memcpy(&CurData, &buf,	sizeof(TCHAR)*8);
//		RealData = (float*)&CurData;

		memset(str1Word, NULL, 4096);
		memcpy(&str1Word, &buf,	sizeof(TCHAR)*8);
//		_tcscpy_s(str1Word, _countof(str1Word), buf);

//		_tcscpy_s(str1Word, _countof(str1Word), strFirst2Byte);
//		strcat(str1Word, strSecond2Byte);
		unsigned int CurData = strtoul(str1Word, &StopStr, 16);	

//		float *RealData;
		RealData = (float*)&CurData;

		m_dPLCReadData[i] = *RealData;

//		memcpy(&buf[0], &alarm.id,			sizeof(USHORT) * 1);
//		m_dPLCReadData[i] = lTempData[2*i+1] & 0xffffffff + lTempData[2*i];
	}

//	CString str;
	CString strLog;
	strLog.Format("RCV(%d), ", nRtn);
	for(i = 0; i<g_pdlgMainPLCRead->nPLCCnt;i ++)
	{
		str.Format("%f,", m_dPLCReadData[i]);
		strLog +=str;
	}

	g_pdlgMainPLCRead->m_dlgPLC.m_bStartSafetyPLCRead = TRUE;		// eh.kim 20171101 add.

//	if(nRtn == 1)
		EventLog(strLog);

	return TRUE;
}
*/
int CDlgPLCDRead::Cmd_Send(CString strSend)
{
	int nRet = -1;
	CString strLog;

	if( api_SocketCtrl_IsConnected(m_nSocketIndex) )
	{
		nRet = api_SocketCtrl_SendData(m_nSocketIndex, (BYTE*)(LPCTSTR)strSend, strlen((LPCTSTR)strSend));		
		strLog.Format("[SEND](%d) %s", nRet, strSend);
		//	strLog.Format("[SEND] %s", strSend);
		//EventLog(strLog);
	}
	else
	{
		strLog.Format("[SEND](%d) Not Connected", nRet);
		//EventLog(strLog);
	}

	return nRet;
}

void CDlgPLCDRead::GetConfig() 
{
	char szValue[MAX_PATH];
	ZeroMemory(szValue,MAX_PATH);

	CString sPath;
	sPath.Format("%s%s", m_strAppPath, dfDEFAULT_CONFIG_FILE_PATH);

	TCHAR	szConfigFile[MAX_PATH];
	_tcscpy_s(szConfigFile,_countof(szConfigFile), sPath);

	CString strPLC;
	strPLC = _T("PLCREAD");
	GetPrivateProfileString (strPLC,"TYPE","1",szValue,MAX_PATH,szConfigFile);
	m_nMode = atoi(szValue);
	GetPrivateProfileString (strPLC,"STATIONNUM","6",szValue,MAX_PATH,szConfigFile);
	m_nStationNum = atoi(szValue);

	GetPrivateProfileString (strPLC,"IP","127.0.0.1",szValue,MAX_PATH,szConfigFile);
	m_szIPAddress = szValue;
	GetPrivateProfileString (strPLC,"Port","1000",szValue,MAX_PATH,szConfigFile);
	m_nPlcPort = atoi(szValue);
	GetPrivateProfileString (strPLC,"OutOfService","0",szValue,MAX_PATH,szConfigFile);
	m_bOutOfService = atoi(szValue);
}

void CDlgPLCDRead::EventLog(CString &s)
{
	// jsyoon 20181016 change
	CString strTemp, sLog;
	strTemp.Format("%s", s);
	if(strTemp.Find("[SEND]")>=0)
		return;

	sLog.Format("%s, %s", gf_MakeCurTimeString2(), s);

	if (m_strReceiveLog.CompareNoCase(strTemp) != 0) //kjhg 20141121 add
	{
//		int nR = WriteFile(&m_fileCommLog, GetLogFileName(), sLog);
		g_pdlgMainPLCRead->WriteELog(sLog, 2);

		m_strReceiveLog.Format("%s", strTemp);
	}

	m_lbPlcLog.InsertString(-1, sLog);

	if (m_lbPlcLog.GetCount() > 100)	// 100라인 정도까지만 출력
		m_lbPlcLog.DeleteString(0);

	m_lbPlcLog.SetCurSel(m_lbPlcLog.GetCount()-1);

// 	CString sLog;
// 	sLog.Format("%s, %s", gf_MakeCurTimeString2(), s);
// 
// 	m_lbPlcLog.InsertString(-1, sLog);
// 
// 	if (m_lbPlcLog.GetCount() > 100)	// 100라인 정도까지만 출력
// 		m_lbPlcLog.DeleteString(0);
// 
// 	m_lbPlcLog.SetCurSel(m_lbPlcLog.GetCount()-1);
// 
// 	CString strTemp;
// 	if( sLog.GetLength() < 2000)
// 		strTemp = sLog;
// 	else
// 		strTemp = sLog.Left(1900);
// 
// 
// 	return;
// 
// 	if (m_strReceiveLog.CompareNoCase(strTemp) != 0) //kjhg 20141121 add
// 	{
// 		int nR = WriteFile(&m_fileCommLog, GetLogFileName(), sLog);
// 	}
// 
// 	m_strReceiveLog = strTemp;

//	int nR = WriteFile(&m_fileCommLog, GetLogFileName(), sLog);

}

CString CDlgPLCDRead::gf_MakeCurTimeString()
{
	SYSTEMTIME st;
	::GetLocalTime(&st);
	CString sTime;
	sTime.Format("%04d.%02d.%02d-%02d:%02d:%02d:%03d",st.wYear ,st.wMonth ,st.wDay ,st.wHour ,st.wMinute ,st.wSecond, st.wMilliseconds  );
	return sTime;
}

CString CDlgPLCDRead::gf_MakeCurTimeString2()
{
	SYSTEMTIME st;
	::GetLocalTime(&st);

	char chTime[1040];
	memset(chTime, NULL, 1040);
	//sprintf(chTime, "%02d:%02d:%02d:%03d",st.wHour ,st.wMinute ,st.wSecond, st.wMilliseconds);	//sypark 20180514 change
	CString sTime;
	sTime.Format("%02d:%02d:%02d:%03d",st.wHour ,st.wMinute ,st.wSecond, st.wMilliseconds);
	int nLength = sTime.GetLength() + 1;
	strcpy_s(chTime, nLength, sTime);

	return chTime;

}

CString CDlgPLCDRead::GetLogFileName()
{
	SYSTEMTIME st;
	::GetLocalTime(&st);
	CTime ct(st);
	CString strLogFileName;

//	strLogFileName.Format("%s%s\\%02d\\PLC_READ.log", g_pdlgMainPLCRead->m_strRootPath, dfDEFAULT_CLOG_PATH, st.wDay);
	strLogFileName.Format("%s%02d\\PLC_READ.log", g_pdlgMainPLCRead->m_strRootPath, st.wDay);

	return strLogFileName;
}

int CDlgPLCDRead::WriteFile(CFile *pFile, CString &sPath, CString sLog)
{
	sLog += "\r\n";
	BOOL bNew = FALSE;
	CFileStatus fs;
	if (!CFile::GetStatus(sPath, fs))  //if not exist yet
	{
		_mkdir(sPath);// _mkdir;
		bNew = TRUE;
	}
	else if (pFile->m_hFile != CFile::hFileNull)
	{
		CString s1 = fs.m_szFullName ;
		if ((pFile->GetFilePath() != s1))
		{
			bNew = TRUE;
		}
	}

	if (pFile->m_hFile != CFile::hFileNull && bNew)
	{
		pFile->Close();
	}


	CFileException* pError = NULL;
	if (pFile->m_hFile == CFile::hFileNull)
	{
		UINT nOpenFlags = CFile::modeReadWrite | CFile::shareDenyWrite;
		if (bNew)
			nOpenFlags |= CFile::modeCreate;

		if (!pFile->Open(sPath , nOpenFlags, pError))
			return 0;
	}
	if (pFile->m_hFile != CFile::hFileNull)
	{
		pFile->SeekToEnd();
		pFile->Write(sLog, sLog.GetLength()) ;
		pFile->Flush();
	}

	return 1;
}

BOOL CDlgPLCDRead::GetReceiveData(CString *reply, UINT nRcvSize, UINT nTimeout)
{
 	DWORD  dwS, dwR, dwT=0;
 	DWORD  dwSleep = 100;
 
 	UINT nSize = nRcvSize;
 	CString Temp;
 
 	for (dwS=GetTickCount(); dwT<nTimeout;)  
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
 
 			Temp.Format("[RECEIVE, %d Bytes] %s",nLen,pByte);
			EventLog(Temp);
 
			*reply = pByte;
 			delete pByte;
 
 			return TRUE;
 		}
 
 		Sleep(10);
 		dwR = GetTickCount();
 
 		if (dwR >= dwS)  
 			dwT = dwR - dwS;
 		else
 			dwT = ((DWORD)0xFFFFFFFF) - (dwS - dwR);
	}
 
 	//	delete pByte;a
 	CString strLog;
 	strLog.Format("Timeout...");
 	EventLog(strLog);
	return TRUE;
}

BOOL CDlgPLCDRead::PreTranslateMessage(MSG* pMsg)
{
	switch( pMsg->wParam )
	{
	case VK_ESCAPE:
	case VK_RETURN:
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CDlgPLCDRead::CreateSocket(SOCKET_OPEN stSockOpen)
{
	if (m_bOutOfService)
	{
		return;
	}
	BOOL IsSet = FALSE;									// 설정이 되었는지 검사하는 변수 
	CString str;
	CString strLog;
	BOOL bConnect = FALSE;

	//	if(FALSE == IsSet)							// 설정이 되어 있지 않을 경우 
	{
		memcpy(&m_stSockOpen, &stSockOpen, sizeof(SOCKET_OPEN));

		BOOL bInitialize = FALSE;
		int nSockMode = stSockOpen.nSockMode;
		int nPort = stSockOpen.nPort;
		CString strIPAddr;

		if(nSockMode == SOCKET_MODE_CLIENT)
		{
			if( m_nMode == 0 )
			{
				if(api_SocketCtrl_IsConnected(m_nSocketIndex))
				{
					// 현재 연결되었으면, 연결 해제.
					api_SocketCtrl_Close(m_nSocketIndex);

					CString strLog;
					strLog.Format("[CLOSED] by user, %d", 1);
					TRACE("%s\n", strLog);
					//				EventLog(strLog);

				}

				strIPAddr = stSockOpen.szIPAddr;

				TCHAR szIp[32];
				_tcscpy_s(szIp, _countof(szIp), strIPAddr);
				bInitialize = api_SocketCtrl_Initialize(m_nSocketIndex, this, nSockMode, nPort, szIp);
				
			}
			else
			{	
				long lRet = 0;
//				if(API_MXComponent_Initialize(5, GetSafeHwnd()))
//				bInitialize = api_Melsoft_Initialize(1, 6, GetSafeHwnd(), &lRet);
				lRet = API_MXComponent_Initialize(1, m_nStationNum, GetSafeHwnd());
				if( lRet == COMPONENT_SUCCESS)
					bInitialize = TRUE;

				strIPAddr = "6";
			}
				
			if(bInitialize)
			{
				strLog.Format("[CONNECTED] (%s)", strIPAddr);
				EventLog(strLog);
			}
			else
			{
//				CString strLog = "Create Socket...Fail!!!";
//				AfxMessageBox(strLog);
				strLog.Format("[ERROR] Connect...Fail(%s)", strIPAddr);
				EventLog(strLog);
			}
		}
// 		else if(nSockMode == SOCKET_MODE_SERVER)
// 		{
// 			bInitialize = api_SocketCtrl_Initialize(m_nSocketIndex, this, nSockMode, nPort, strIPAddr);
// 
// 			if(!bInitialize)
// 			{
// 				CString strLog = "Create Socket...Fail!!!";
// 				//				EventLog(strLog);
// 				AfxMessageBox(strLog);
// 				return;
// 			}
// 
// //			if(API_MXComponent_Initialize(5, GetSafeHwnd()))
// 			long lRet = 0;
// 			if( api_Melsoft_Initialize(1, 6, GetSafeHwnd(), &lRet))
// 			{
// 				CString strLog;
// 				strLog = "[CONNECTED] PLC connection success!";
// 			}
// 			else
// 				AfxMessageBox("PLC connection fail.");
// 		}
	}
}

void CDlgPLCDRead::CloseSocket()
{
	CString strLog;
	BOOL bRet = TRUE;
	if( m_nMode == 0 )
	{
		if(api_SocketCtrl_IsConnected(m_nSocketIndex))
		{
			// 현재 연결되었으면, 연결 해제.
			bRet = api_SocketCtrl_Close(m_nSocketIndex);
		}
	}
	else
	{
		//				if(API_MXComponent_Initialize(5, GetSafeHwnd()))
		long lRet=0;
		//				bInitialize = api_Melsoft_Initialize(0, 5, GetSafeHwnd(), &lRet);
		lRet = API_MXComponent_Close(1);
		if( lRet == COMPONENT_SUCCESS )
		{
			bRet = TRUE;
		}
	}

	strLog.Format("[CLOSED] by user, %d", m_nSocketIndex);
	TRACE("%s\n", strLog);
	EventLog(strLog);
}

long CDlgPLCDRead::DoWorkStartGetPLCMemoryBit(CString strDeviceText,int iDeviceNum)
{

	if (m_bOutOfService)
	{
		return TRUE;
	}

 	long lData = 0;
// 	API_MXComponent_GetMemoryBit(strDeviceText, iDeviceNum, &lData);


	return lData;
}

void CDlgPLCDRead::OnDestroy()
{
	CDialog::OnDestroy();

//	api_SocketCtrl_Close(m_nSocketIndex);

//	long lret = 0;
//	api_Melsoft_Close(1, &lret);

//	API_MXComponent_Close(1);
}

BOOL CDlgPLCDRead::GetPLCMemoryBitAll()
{
	int		stadd = 1;
	int		stadd8 = 8;
	char	temp[20];
	char	str[100];

	str[0] = 0;
	//		strcat(str, PLC_CMD_ETHERNET_SINGLE_READ);		//5000(헤드) 00FF(번호) 03FF(모듈번호) 00(국번) 0018(길이) 0090(타이머)  0401(읽기) 0001(비트단위) // 0000(워드단위)
	_tcscat_s(str, _countof(str), PLC_CMD_ETHERNET_WORD_READ_D);

	short bit = 1;

	//4000번 부터 일괄적으로 읽을 것임
	// 	sprintf(temp, "M*%06d", PLC_READ_ALL_ADRESS);		strcat(str, temp); //M*004000, ex)address 4000
	// 	sprintf(temp, "%04X", dfMAX_PLC_IN_CNT);						strcat(str, temp); //200 ==> C8 (Hexa)

//	_stprintf_s(temp, _countof(temp), "D*%06d", PLC_M_READ_START_BIT);	_tcscat_s(str, _countof(str), temp); //M*001001, ex)address 1001		// 시작번지
//	_stprintf_s(temp, _countof(temp), "%04X",   1000);					_tcscat_s(str, _countof(str), temp); //stadd 1 ==> 0001					// 읽을 비트 수
	_stprintf_s(temp, _countof(temp), "D*%06d", PLC_D_READ_STARY_BIT);		strcat(str, temp); //M*004000, ex)address 4000
	_stprintf_s(temp, _countof(temp), "%04x",   dfMAX_PLC_IN_CNTD*4);		strcat(str, temp); //200 ==> C8 (Hexa)


	int n = strlen(str);					//str[n]   = PLC_CHAR_EOT;
	str[n] = 0;								//n = 42

	// 길이가 작으면 보내기 실패
	if( Cmd_Send(str) < n)
		return FALSE;
	else
		return TRUE;
}
