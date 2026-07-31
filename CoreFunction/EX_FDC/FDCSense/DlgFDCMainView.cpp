// DlgFDCMainView.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "FDCSEnse.h"
#include "DlgFDCMainView.h"
#include "afxdialogex.h"

#include "FDCSenseDlg.h"

#include "DlgPLC.h"
#include "DlgFDCMainView.h"
#include "DlgPLCDRead.h"
#include <direct.h>
/*#include "FDCDefine.h"*/

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CFDCSenseDlg* g_pdlgMain = NULL;

#define SAFE_DELETE(ptr)		if (ptr)	{ delete ptr;	ptr = NULL;	}
#define SAFE_DELETE_AR(ptr)		if (ptr)	{ delete [] ptr;	ptr = NULL;	}

// CDlgFDCMainView 대화 상자입니다.

IMPLEMENT_DYNAMIC(CDlgFDCMainView, CDialog)

CDlgFDCMainView::CDlgFDCMainView(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgFDCMainView::IDD, pParent)
{
	m_nSocketIndex = 0;
}

CDlgFDCMainView::~CDlgFDCMainView()
{
}

void CDlgFDCMainView::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_FFU_LOG, m_lbCimLog);
}


BEGIN_MESSAGE_MAP(CDlgFDCMainView, CDialog)
//	ON_BN_CLICKED(IDC_BTN_CONNET_CLIENT, &CDlgFDCMainView::OnBnClickedBtnConnetClient)
	ON_WM_DESTROY()
	ON_MESSAGE(NET_MSG_ACCEPT,  OnMsgAccept)            // 소켓 연결
	ON_MESSAGE(NET_MSG_CLOSE,   OnMsgClose)             // 소켓 해제
	ON_MESSAGE(NET_MSG_RECEIVE, OnMsgReceive)           // 소켓으로 부터 수신되는 데이터
END_MESSAGE_MAP()


// CDlgFDCMainView 메시지 처리기입니다.

void CDlgFDCMainView::SetMainPoint(void* pvoid)
{
	g_pdlgMain = (CFDCSenseDlg*) pvoid;
}


void CDlgFDCMainView::OnBnClickedBtnConnetClient()
{
	GetConfig();
	SOCKET_OPEN stSockOpen;
	stSockOpen.nSockMode = SOCKET_MODE_SERVER;
	stSockOpen.nPort = m_nPlcPort;

	_tcscpy_s(stSockOpen.szIPAddr, _countof(stSockOpen.szIPAddr), m_szIPAddress);	
	CreateSocket(stSockOpen);
}

void CDlgFDCMainView::CreateSocket(SOCKET_OPEN stSockOpen)
{	
	if (m_bOutOfService)
	{
		return;
	}
	BOOL IsSet = FALSE;							// 설정이 되었는지 검사하는 변수 
	CString str;
	CString strLog;
	BOOL bConnect = FALSE;

	//	if(FALSE == IsSet)							// 설정이 되어 있지 않을 경우 
	{
		memcpy(&m_stSockOpen, &stSockOpen, sizeof(SOCKET_OPEN));

		BOOL bInitialize = FALSE;
		int nSockMode = stSockOpen.nSockMode;
		int nPort = stSockOpen.nPort;
		CString strIPAddr = stSockOpen.szIPAddr;

// 		if(nSockMode == SOCKET_MODE_CLIENT)
// 		{
// 			if(api_SocketCtrl_IsConnected(m_nSocketIndex))
// 			{
// 				// 현재 연결되었으면, 연결 해제.
// 				api_SocketCtrl_Close(m_nSocketIndex);
// 
// 				CString strLog;
// 				strLog.Format("[CLOSED] by user, %d", m_nSocketIndex);
// 				TRACE("%s\n", strLog);
// 				//				EventLog(strLog);
// 			}
// 			else
// 			{
// 				bInitialize = api_SocketCtrl_Initialize(m_nSocketIndex, this, nSockMode, nPort, strIPAddr);
// 
// 				if(bInitialize)
// 				{
// 					CString strLog;
// 					strLog.Format("[CONNECTED] (%s)", strIPAddr);
// 					//EventLog(strLog);
// 				}
// 				else
// 				{
// 					CString strLog = "Create Socket...Fail!!!";
// 					AfxMessageBox(strLog);
// 					//					return;
// 				}
// 			}
// 		}
//		else if(nSockMode == SOCKET_MODE_SERVER)
		{
			TCHAR szIp[32];
			_tcscpy_s(szIp, _countof(szIp), strIPAddr);
			bInitialize = api_SocketCtrl_Initialize(m_nSocketIndex, this, nSockMode, nPort, szIp);
			if(!bInitialize)
			{
				strLog = "Create Socket...Fail!!!";
				EventLog(strLog);
//				AfxMessageBox(strLog);
			}
			else
			{
				strLog.Format("[CREATED] (%s)", strIPAddr);
				EventLog(strLog);
			}
		}
	}
}

void CDlgFDCMainView::GetConfig() 
{
	char szValue[MAX_PATH];
	ZeroMemory(szValue,MAX_PATH);

	CString sPath;
	sPath.Format("%s%s", m_strAppPath, dfDEFAULT_CONFIG_FILE_PATH);

	TCHAR	szConfigFile[MAX_PATH];
	_tcscpy_s(szConfigFile,_countof(szConfigFile), sPath);

	CString strPLC;
	strPLC = _T("FFU");

	GetPrivateProfileString (strPLC,"IP","127.0.0.1",szValue,MAX_PATH,szConfigFile);
	m_szIPAddress = szValue;
	GetPrivateProfileString (strPLC,"Port","1000",szValue,MAX_PATH,szConfigFile);
	m_nPlcPort = atoi(szValue);
	GetPrivateProfileString (strPLC,"OutOfService","0",szValue,MAX_PATH,szConfigFile);
	m_bOutOfService = atoi(szValue);
}

// void CDlgFDCMainView::OnStnClickedStaticLivecheck()
// {
// 
// }


// void CDlgFDCMainView::OnTimer(UINT_PTR nIDEvent)
// {
// 	if( nIDEvent == 1)
// 	{
// 		CString str;
// 		str.Format("%d", g_pdlgMain->m_ffu.lLiveCheck);
// 		GetDlgItem(IDC_STATIC_LIVECHECK)->SetWindowText(str);
// 	}
// 
// 	CDialog::OnTimer(nIDEvent);
// }

void CDlgFDCMainView::GetReceiveData()
{
	if( api_SocketCtrl_IsConnected(m_nSocketIndex) != TRUE)
		return;

	int nLen = 0;
	nLen = api_SocketCtrl_GetReceiveDataSize(m_nSocketIndex);
	if(nLen>0 && nLen < 1024*8)
	{
		BYTE* pByRecv = new BYTE[nLen + 1];		
		ZeroMemory(pByRecv, nLen+1);	
		api_SocketCtrl_GetReceiveData(m_nSocketIndex, pByRecv, nLen);    //수신한 메세지 사이즈만큼 수신한 메세지를 가져온다. 

		if(nLen < 1024*8)
		{
			int nCmdStart= 0, nCmdEnd = 0;		
			CString Temp;		
			Temp.Empty();
			Temp.Format("[RECEIVE](%4d), %s",nLen, pByRecv);
			EventLog(Temp);

			CString strRcv = LPCTSTR(pByRecv);
			while(strRcv.Find('*')>=0)
			{
				nCmdStart = 0;
				nCmdEnd = strRcv.Find('*');
				CString strCmd;
				strCmd = strRcv.Mid(nCmdStart, nCmdEnd+1);

				BYTE* pCmdByte;
				pCmdByte = new BYTE[strCmd.GetLength() + 1];

				memcpy(pCmdByte,(LPCTSTR)strCmd,strCmd.GetLength());

				pCmdByte[strCmd.GetLength()] = 0x00;

				AnalysisReceiveData(strCmd.GetLength(),pCmdByte);

				strRcv.Delete(nCmdStart,nCmdEnd+1);

				delete pCmdByte;
			}
		}

		SAFE_DELETE(pByRecv);
	}
}

BOOL CDlgFDCMainView::OnInitDialog()
{
	CDialog::OnInitDialog();

	GetConfig();

//	SetTimer(1, 1000, NULL);

	return TRUE;
}

// LRESULT CDlgFDCMainView::OnSocketMsgReceive(WPARAM wParam, LPARAM lParam)
// {
// 	UINT nSize = (UINT)lParam;
// 
// 	CString strLog;
// 	strLog.Format("Receive size %d, \n", nSize);
// 	OutputDebugString(strLog);						// jsyoon 20170706 add
// 
// 	return 0;
// 
// 	//	UINT nSize = (UINT)lParam;
// 	CString Temp;
// 
// 	BYTE* pByRecv = new BYTE[nSize + 1];
// 	char* pSzRecv = new char[nSize + 1];
// 
// 	int nCmdStart= 0;
// 	int nCmdEnd = 0;
// 
// 	ZeroMemory(pByRecv, nSize+1);
// 	ZeroMemory(pSzRecv, nSize+1);
// 
// 	api_SocketCtrl_GetReceiveData(m_nSocketIndex, pByRecv, nSize);    //수신한 메세지 사이즈만큼 수신한 메세지를 가져온다. 
// 	memcpy(pSzRecv, pByRecv, nSize);
// 
// 	Temp.Empty();
// 	Temp.Format("[RECEIVE] %s",pByRecv);
// 	EventLog(Temp);
// 
// 	CString strRcv = LPCTSTR(pByRecv);
// 	while(strRcv.Find('*')>=0)
// 	{
// 		nCmdStart = 0;
// 		nCmdEnd = strRcv.Find('*');
// 		CString strCmd;
// 		strCmd = strRcv.Mid(nCmdStart, nCmdEnd+1);
// 
// 		BYTE* pCmdByte;
// 		pCmdByte = new BYTE[strCmd.GetLength() + 1];
// 
// 		memcpy(pCmdByte,(LPCTSTR)strCmd,strCmd.GetLength());
// 
// 		pCmdByte[strCmd.GetLength()] = 0x00;
// 
// 		AnalysisReceiveData(strCmd.GetLength(),pCmdByte);
// 
// 		strRcv.Delete(nCmdStart,nCmdEnd+1);
// 
// 		delete pCmdByte;
// 	}
// 
// 	SAFE_DELETE(pByRecv);
// 	SAFE_DELETE(pSzRecv);
// 
// 	return 0;
// }

void CDlgFDCMainView::AnalysisReceiveData(UINT nSize, BYTE *pByte)
{
	CString Temp;
	BYTE* pRcvData;
	int i = 0;
	pRcvData = pByte;

	pByte[nSize] = 0x00;

	CString strRcv = LPCTSTR(pByte);
	CString strParam;

	int nCmdEnd = 0;
	nCmdEnd = strRcv.Find('*');
	int nCmdStart = 0;
	CString str;
	int iLoop = 0;

	// 	Temp.Empty();
	// 	Temp.Format("[RECEIVE] %s",strRcv);
	// 	EventLog(Temp);

	g_pdlgMain->m_ffu.lLiveCheck = GetTickCount();

	int iNum = 0, iStart, iEnd = 0, iMsg = 0, k = 0;
	double dTEMP = 0.;
	CString strTmp;

	if(strRcv.Find("H2O")>=0)		// 온도센서
	{
		iNum = 0, iStart = 0, iEnd = 0, iMsg = 0;
		dTEMP = 0.;
		strTmp.Format("H2O*^");
		iMsg  = strTmp.GetLength();
		iLoop = 0;

		do 
		{
			iStart = strRcv.Find("^");
			iEnd   = strRcv.Find(",");
		
			if( iStart+1 >= iMsg && iEnd > iStart+1 )
			{
				// jsyoon 20161117 change
				str = strRcv.Mid(iStart-1, 1);		// NUM 1
				iNum = atoi(str)-1;
				if( iNum >= 0 && iNum < 2 ) // 20161226
				{
					str = strRcv.Mid(iStart+1, iEnd - iMsg );			// jsyoon 20161117 change
					dTEMP = atof(str);
					g_pdlgMain->m_ffu.dTmp[dfStart_H20+iNum] = dTEMP; 
				}
			}

			iLoop++;

			if( iLoop > 100)
			{
				iLoop = 0;
				iEnd   = strRcv.GetLength();
			}
			else
			{
				if( iEnd < 0 )
					iEnd   = strRcv.Find("*");
			}

			strRcv.Delete(0, iEnd+1);

		} while (strRcv.Find("H2O")>=0);
	}

	if(strRcv.Find("TEMP")>=0)		// 온도센서
	{
		iNum = 0, iStart = 0, iEnd = 0, iMsg = 0;
		dTEMP = 0.;
		strTmp.Format("TEMP**^");
		iMsg  = strTmp.GetLength();
		iLoop = 0;

		do 
		{
			iStart = strRcv.Find("^");
			iEnd   = strRcv.Find(",");

			if( iStart+1 >= iMsg && iEnd > iStart+1 )
			{
				// jsyoon 20161117 change	
				str = strRcv.Mid(iStart-2, 2);		// NUM 2
				iNum = atoi(str)-1;
				if( iNum >= 0 && iNum < dfMAXNUM_PCTEMP ) // 20161226
				{
					str = strRcv.Mid(iStart+1, iEnd-iMsg);			// jsyoon 20161117 change
					dTEMP = atof(str);
					g_pdlgMain->m_ffu.dTemper[iNum] = dTEMP; 
				}
			}

			iLoop++;

			if( iLoop > dfMAXNUM_PCTEMP)
			{
				iLoop = 0;
				iEnd   = strRcv.GetLength();
			}
			else
			{
				if( iEnd < 0 )
					iEnd   = strRcv.Find("*");
			}

			strRcv.Delete(0, iEnd+1);

		} while (strRcv.Find("TEMP")>=0);
	}

	//jhseo 20170920 add for KM-N2
	if(strRcv.Find("KMN2_1")>=0)
	{
		nCmdStart = 7;
		str = strRcv.Mid(nCmdStart, nCmdEnd-nCmdStart);
		double dKMN2 = atof(str);
		g_pdlgMain->m_ffu.dKMN2Result[0] = dKMN2; 
	}

	if(strRcv.Find("KMN2_2")>=0)
	{
		nCmdStart = 7;
		str = strRcv.Mid(nCmdStart, nCmdEnd-nCmdStart);
		double dKMN2 = atof(str);
		g_pdlgMain->m_ffu.dKMN2Result[1] = dKMN2; 
	}

	if(strRcv.Find("EFU")>=0)		// EFU
	{
		iNum = 0, iStart = 0, iEnd = 0, iMsg = 0;
		dTEMP = 0.;
		strTmp.Format("EFU**^");
		iMsg  = strTmp.GetLength();
		iLoop = 0;

		do 
		{
			iStart = strRcv.Find("^");
			iEnd   = strRcv.Find(",");

			if( iStart+1 >= iMsg && iEnd > iStart+1 )
			{
				// jsyoon 20161117 change	
				str = strRcv.Mid(iStart-2, 2);		// NUM 2
				iNum = atoi(str)-1;
				if( iNum >= 0 && iNum < dfMAXNUM_EFU )
				{
					str = strRcv.Mid(iStart+1, iEnd-iMsg);			// jsyoon 20161117 change
					dTEMP = atof(str);
					g_pdlgMain->m_ffu.dEFU[iNum] = dTEMP; 
				}
			}

			iLoop++;

			if( iLoop > dfMAXNUM_EFU)
			{
				iLoop = 0;
				iEnd   = strRcv.GetLength();
			}
			else
			{
				if( iEnd < 0 )
					iEnd   = strRcv.Find("*");
			}

			strRcv.Delete(0, iEnd+1);

		} while (strRcv.Find("EFU")>=0);
	}

	//jh.kang 20170518 add
	if(strRcv.Find("ELST")>=0)
	{
		nCmdStart = 5;
		str = strRcv.Mid(nCmdStart, nCmdEnd-nCmdStart);
		double dStElectric= atof(str);
		g_pdlgMain->m_ffu.dStaticElectric = dStElectric; 
	}
	//

	// jsyoon 20171121 add
	if(strRcv.Find("ION1_Head")>=0)
	{
		iNum = 0, iStart= 0, iEnd= 0;

		iStart = strRcv.Find("^");								
		iEnd = strRcv.Find("*");	
		str = strRcv.Mid(iStart+1, (iEnd - iStart));	

		if(strRcv.Find("HeadConnect")>=0)
		{	
			BOOL bConnect = FALSE;
			bConnect = atoi(str);
			g_pdlgMain->m_ffu.bIonizerConnect[0] = bConnect; 
		}
		else if(strRcv.Find("HeadAlarm")>=0)
		{
			BOOL bAlarm = FALSE;
			bAlarm = atoi(str);
			g_pdlgMain->m_ffu.bIonizerAlarm[0] = bAlarm; 
		}
		else if(strRcv.Find("RunTime")>=0)
		{
			double dTime =0.;
			dTime = atof(str);
			g_pdlgMain->m_ffu.dIonizerTime[0] = dTime; 
		}
	}

	if(strRcv.Find("ION2_Head")>=0)
	{
		iNum = 0, iStart= 0, iEnd= 0;

		iStart = strRcv.Find("^");								
		iEnd = strRcv.Find("*");	
		str = strRcv.Mid(iStart+1, (iEnd - iStart));	

		if(strRcv.Find("HeadConnect")>=0)
		{	
			BOOL bConnect = FALSE;
			bConnect = atoi(str);
			g_pdlgMain->m_ffu.bIonizerConnect[1] = bConnect; 
		}
		else if(strRcv.Find("HeadAlarm")>=0)
		{
			BOOL bAlarm = FALSE;
			bAlarm = atoi(str);
			g_pdlgMain->m_ffu.bIonizerAlarm[1] = bAlarm; 
		}
		else if(strRcv.Find("RunTime")>=0)
		{
			double dTime =0.;
			dTime = atof(str);
			g_pdlgMain->m_ffu.dIonizerTime[1] = dTime; 
		}
	}
	if(strRcv.Find("ION3_Head")>=0)
	{
		iNum = 0, iStart= 0, iEnd= 0;

		iStart = strRcv.Find("^");								
		iEnd = strRcv.Find("*");	
		str = strRcv.Mid(iStart+1, (iEnd - iStart));	

		if(strRcv.Find("HeadConnect")>=0)
		{	
			BOOL bConnect = FALSE;
			bConnect = atoi(str);
			g_pdlgMain->m_ffu.bIonizerConnect[2] = bConnect; 
		}
		else if(strRcv.Find("HeadAlarm")>=0)
		{
			BOOL bAlarm = FALSE;
			bAlarm = atoi(str);
			g_pdlgMain->m_ffu.bIonizerAlarm[2] = bAlarm; 
		}
		else if(strRcv.Find("RunTime")>=0)
		{
			double dTime =0.;
			dTime = atof(str);
			g_pdlgMain->m_ffu.dIonizerTime[2] = dTime; 
		}
	}
	if(strRcv.Find("ION4_Head")>=0)
	{
		iNum = 0, iStart= 0, iEnd= 0;

		iStart = strRcv.Find("^");								
		iEnd = strRcv.Find("*");	
		str = strRcv.Mid(iStart+1, (iEnd - iStart));	

		if(strRcv.Find("HeadConnect")>=0)
		{	
			BOOL bConnect = FALSE;
			bConnect = atoi(str);
			g_pdlgMain->m_ffu.bIonizerConnect[3] = bConnect; 
		}
		else if(strRcv.Find("HeadAlarm")>=0)
		{
			BOOL bAlarm = FALSE;
			bAlarm = atoi(str);
			g_pdlgMain->m_ffu.bIonizerAlarm[3] = bAlarm; 
		}
		else if(strRcv.Find("RunTime")>=0)
		{
			double dTime =0.;
			dTime = atof(str);
			g_pdlgMain->m_ffu.dIonizerTime[3] = dTime; 
		}
	}

	if(strRcv.Find("ACCURA")>=0)		// ACCURA
	{
		iNum = 0, iStart= 0, iEnd= 0, iMsg;
		dTEMP = 0.;
		strTmp.Format("ACCURA*^");
		iMsg  = strTmp.GetLength();
		iLoop = 0;

		do 
		{
			iStart = strRcv.Find("^");
			iEnd   = strRcv.Find("*"); 
			
			if( iStart+1 >= iMsg && iEnd > iStart+1 )
			{
				// jsyoon 20161117 change	
				str = strRcv.Mid(iStart-1, 1);		// NUM 2
				iNum = atoi(str)-1;
				if( iNum >= 0 && iNum < dfMAXNUM_ACCURA ) // 20161226
				{
					strRcv.Replace(_T("*"), _T(""));
					AfxExtractSubString(strParam, strRcv, 1, '^');
					g_pdlgMain->m_stAccuraData[iNum].Vavg_ll_1 = (float)_tstof(strParam); 

					AfxExtractSubString(strParam, strRcv, 2, '^');
					g_pdlgMain->m_stAccuraData[iNum].lavg_1 = (float)_tstof(strParam); 

					AfxExtractSubString(strParam, strRcv, 3, '^');
					g_pdlgMain->m_stAccuraData[iNum].Ptot_1 = (float)_tstof(strParam); 

					AfxExtractSubString(strParam, strRcv, 4, '^');
					g_pdlgMain->m_stAccuraData[iNum].Net_1 = _tstoi(strParam); 

					// jsyoon 20190318 change
					for (k=0; k<20; k++)
					{
						AfxExtractSubString(strParam, strRcv, 5+(2*k), '^');
						g_pdlgMain->m_stAccuraData[iNum].nValue[k]		= _tstoi(strParam); 

						AfxExtractSubString(strParam, strRcv, 6+(2*k), '^');
						g_pdlgMain->m_stAccuraData[iNum].nValueTotal[k] = _tstoi(strParam); 
					}


// 					AfxExtractSubString(strParam, strRcv, 5, '^');
// 					g_pdlgMain->m_stAccuraData[iNum].N2_1 = _tstoi(strParam); 
// 
// 					AfxExtractSubString(strParam, strRcv, 6, '^');
// 					g_pdlgMain->m_stAccuraData[iNum].N2_1_Total = _tstoi(strParam); 
// 
// 					AfxExtractSubString(strParam, strRcv, 7, '^');
// 					g_pdlgMain->m_stAccuraData[iNum].N2_2 = _tstoi(strParam); 
// 
// 					AfxExtractSubString(strParam, strRcv, 8, '^');
// 					g_pdlgMain->m_stAccuraData[iNum].N2_2_Total = _tstoi(strParam); 
// 
// 					AfxExtractSubString(strParam, strRcv, 7, '^');
// 					g_pdlgMain->m_stAccuraData[iNum].CDA_1 = _tstoi(strParam); 
// 
// 					AfxExtractSubString(strParam, strRcv, 8, '^');
// 					g_pdlgMain->m_stAccuraData[iNum].CDA_1_Total = _tstoi(strParam); 
				}
			}

			iLoop++;

			if( iLoop > dfMAXNUM_ACCURA)
			{
				iLoop = 0;
				iEnd   = strRcv.GetLength();
			}
			else
			{
				if( iEnd < 0 )
					iEnd   = strRcv.Find("*");
			}

			strRcv.Delete(0, iEnd+1);

		} while (strRcv.Find("ACCURA")>=0);

	}

	if(strRcv.Find("ESD")>=0)		// 온도센서
	{
		iNum = 0, iStart = 0, iEnd = 0, iMsg = 0;
		dTEMP = 0.;
		strTmp.Format("ESD*^");
		iMsg  = strTmp.GetLength();
		iLoop = 0;

		do 
		{
			iStart = strRcv.Find("^");
			iEnd   = strRcv.Find(",");

			if( iStart+1 >= iMsg && iEnd > iStart+1 )
			{
				// jsyoon 20161117 change
				str = strRcv.Mid(iStart-1, 1);		// NUM 1
				iNum = atoi(str)-1;
				if( iNum >= 0 && iNum < dfMAXNUM_ESD )
				{
					str = strRcv.Mid(iStart+1, iEnd - iMsg );			// jsyoon 20161117 change
					dTEMP = atof(str);
					g_pdlgMain->m_ffu.dTmp[dfStart_ESD+iNum] = dTEMP; 
				}
			}

			iLoop++;

			if( iLoop > dfMAXNUM_ESD)
			{
				iLoop = 0;
				iEnd   = strRcv.GetLength();
			}
			else
			{
				if( iEnd < 0 )
					iEnd   = strRcv.Find("*");
			}

			strRcv.Delete(0, iEnd+1);

		} while (strRcv.Find("ESD")>=0);
	}

	if(strRcv.Find("AZBIL^")>=0)		// 온도센서
	{
		iNum = 0, iStart = 0, iEnd = 0;
		dTEMP = 0.;
		iLoop = 0;

		do 
		{
			iStart = strRcv.Find("^");
			iEnd   = strRcv.Find(",");

			if( iNum < dfMAXNUM_AZBIL  && iEnd > iStart+1 )
			{
				str = strRcv.Mid(iStart+1, iEnd );			// jsyoon 20161117 change
				dTEMP = atof(str);
				g_pdlgMain->m_ffu.dTmp[dfStart_AZBIL+iNum] = dTEMP; 
				iNum++;
			}

			iLoop++;

			if( iLoop > dfMAXNUM_AZBIL)
			{
				iLoop = 0;
				iEnd   = strRcv.GetLength();
			}
			else
			{
				if( iEnd < 0 )
					iEnd   = strRcv.Find("*");
			}

			strRcv.Delete(0, iEnd+1);

		} while (strRcv.Find("*")>=0);
	}

	if(strRcv.Find("PM5500^")>=0)		// 온도센서
	{
		iNum = 0, iStart = 0, iEnd = 0;
		dTEMP = 0.;
		iLoop = 0;

		do 
		{
			iStart = strRcv.Find("^");
			iEnd   = strRcv.Find(",");

			if( iNum < dfMAXNUM_PM5500  && iEnd > iStart+1 )
			{
				str = strRcv.Mid(iStart+1, iEnd );			// jsyoon 20161117 change
				dTEMP = atof(str);
				g_pdlgMain->m_ffu.dTmp[dfStart_PM5500+iNum] = dTEMP; 
				iNum++;
			}

			iLoop++;

			if( iLoop > dfMAXNUM_PM5500)
			{
				iLoop = 0;
				iEnd   = strRcv.GetLength();
			}
			else
			{
				if( iEnd < 0 )
					iEnd   = strRcv.Find("*");
			}

			strRcv.Delete(0, iEnd+1);

		} while (strRcv.Find("*")>=0);
	}

	if(strRcv.Find("ION_")>=0)		// 온도센서
	{
		int cnt = 0, nNum = 0;
		CString strTok;

		while(AfxExtractSubString(strTok,strRcv,cnt,','))
		{
			iStart = strTok.Find("^");
			iEnd   = strTok.GetLength();

			if( iNum < dfMAXNUM_IONIZER  && iEnd > iStart+1 )
			{
				str = strTok.Mid(iStart+1, iEnd );			// jsyoon 20161117 change

				if(cnt == 0 + (3*nNum))
				{
					g_pdlgMain->m_ffu.bIonizerConnect[nNum] = _ttoi(str);
				}
				else if(cnt == 1 + (3*nNum))
				{
					g_pdlgMain->m_ffu.bIonizerAlarm[nNum] = _ttoi(str);
				}
				else if(cnt == 2 + (3*nNum))
				{
					g_pdlgMain->m_ffu.dIonizerTime[nNum] = _ttof(str);

					nNum++;
				}
				else
				{
					;
				}
			}

			cnt++;
		}
	}
}

void CDlgFDCMainView::EventLog(CString &s)
{
	CString sLog;
	sLog.Format("%s, %s",gf_MakeCurTimeString2(), s);

	m_lbCimLog.InsertString(-1, sLog);

	if (m_lbCimLog.GetCount() > 100)	// 100라인 정도까지만 출력
		m_lbCimLog.DeleteString(0);

	m_lbCimLog.SetCurSel(m_lbCimLog.GetCount()-1);

	g_pdlgMain->WriteELog(sLog, 0);

//	int nR = WriteFile(&m_fileCommLog, GetLogFileName(), sLog);

}

CString CDlgFDCMainView::GetLogFileName()
{
	SYSTEMTIME st;
	::GetLocalTime(&st);
	CTime ct(st);
	CString strLogFileName;
//	strLogFileName.Format("%s%s\\%02d\\Sensor.log", g_pdlgMain->m_strRootPath, dfDEFAULT_CLOG_PATH, st.wDay);
	strLogFileName.Format("%s%02d\\TCP_RECEIVE.log", g_pdlgMain->m_strRootPath, st.wDay);

	return strLogFileName;
}

int CDlgFDCMainView::WriteFile(CFile *pFile, CString &sPath, CString sLog)
{
	sLog += "\r\n";
	BOOL bNew = FALSE;
	CFileStatus fs;
	if (!CFile::GetStatus(sPath, fs))  //if not exist yet
	{
		gf_MakeDir(sPath);// _mkdir;
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

CString CDlgFDCMainView::gf_MakeCurTimeString()
{
	SYSTEMTIME st;
	::GetLocalTime(&st);
	CString sTime;
	sTime.Format("%04d.%02d.%02d-%02d:%02d:%02d:%03d",st.wYear ,st.wMonth ,st.wDay ,st.wHour ,st.wMinute ,st.wSecond, st.wMilliseconds  );
	return sTime;
}

CString CDlgFDCMainView::gf_MakeCurTimeString2()
{
	SYSTEMTIME st;
	::GetLocalTime(&st);
	CString sTime;
	sTime.Format("%02d:%02d:%02d:%03d",st.wHour ,st.wMinute ,st.wSecond, st.wMilliseconds );
	return sTime;
}

BOOL CDlgFDCMainView::gf_MakeDir(CString sPath)
{
	if (sPath[1] != ':' || sPath[2] != '\\')
		return FALSE;

	CString sTem;
	int nFind = 2;
	while ((nFind = sPath.Find("\\",nFind+1)) >= 0)
	{
		sTem = sPath.Left(nFind);
		_mkdir(sTem);
	}
	return TRUE;
}

void CDlgFDCMainView::DisConnect()
{
	api_SocketCtrl_Close(m_nSocketIndex);
}


void CDlgFDCMainView::OnDestroy()
{
	CDialog::OnDestroy();

	api_SocketCtrl_Close(m_nSocketIndex);

	//API_MXComponent_Close();

	KillTimer(1);
}


BOOL CDlgFDCMainView::PreTranslateMessage(MSG* pMsg)
{
	switch( pMsg->wParam )
	{
	case VK_ESCAPE:
	case VK_RETURN:
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CDlgFDCMainView::ConnectSock()
{
	SOCKET_OPEN stSockOpen;
	stSockOpen.nSockMode = SOCKET_MODE_SERVER;
	stSockOpen.nPort = m_nPlcPort;
	_tcscpy_s(stSockOpen.szIPAddr, _countof(stSockOpen.szIPAddr), m_szIPAddress);
	CreateSocket(stSockOpen);
}

// CNetWallTalk으로 부터 NET_MSG_ACCEPT를 수신했을 경우 실행되는 함수 
LRESULT CDlgFDCMainView::OnMsgAccept(WPARAM wParam, LPARAM lParam)
{
	CString strLog;
	TCHAR szIp[32];
	ZeroMemory(&szIp, sizeof(szIp));
	api_SocketCtrl_GetClientIPAddress(m_nSocketIndex, szIp);
	strLog.Format("[CONNECTED] (%s)", szIp);
	EventLog(strLog);

	return 0;
}

// CNetWallTalk으로 부터 NET_MSG_CLOSE를 수신했을 경우 실행되는 함수 
LRESULT CDlgFDCMainView::OnMsgClose(WPARAM wParam, LPARAM lParam)
{
	g_pdlgMain->StopRecive();

	CString strLog;
	TCHAR szIp[32];
	ZeroMemory(&szIp, sizeof(szIp));
	api_SocketCtrl_GetClientIPAddress(m_nSocketIndex, szIp);
	strLog.Format("[DISCONNECTED] (%s)", szIp);
	EventLog(strLog);

	return 0;
}

// CNetWallTalk으로 부터 NET_MSG_RECEIVE를 수신했을 경우 실행되는 함수 
LRESULT CDlgFDCMainView::OnMsgReceive(WPARAM wParam, LPARAM lParam)
{
	g_pdlgMain->StartRecive();

	return 0;
}