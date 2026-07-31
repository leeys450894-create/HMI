#if !defined(AFX_DLGESD_H__57539DD6_DC46_4DDF_9070_A37EBB655D82__INCLUDED_)
#define AFX_DLGESD_H__57539DD6_DC46_4DDF_9070_A37EBB655D82__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgESD.h : header file
//

#include "../include/API_SocketControl.h"
#include "..\Include\API_SerialCtrl.h"

#define J_PROTOCOL_REQUEST_SIZE			12				//REQUEST PROCOTOL SIZE
#define J_PROTOCOL_WRITE_SIZE			14				//WRITE PROCOTOL SIZE
#define J_PROTOCOL_REQUEST				0x04			//Read Protocol
#define J_PROTOCOL_WRITE				0x06			//Write Protocol
#define J_PROTOCOL_READ					0x03			//Read Protocol

typedef struct tagSDAS_4AD_Head
{
	BYTE btTransaction[2];
	BYTE btProtocol[2];
	BYTE btLength[2];
	BYTE btUnitID;
	BYTE btFC;
}SDAS_HEAD_DATA;

typedef struct tagRS232C_RESPONSE_DATA
{
	BYTE btUnitID;
	BYTE btFC;
	BYTE btLength[2];
}RS232C_RESPONSE_DATA;


typedef struct tagRawRealData
{
	BYTE btCurrent[2];
	BYTE btAverage[2];
	BYTE btMin[2];
	BYTE btMax[2];
}RawData;

enum J_SDAS_Channel
{
	J_CHANNEL_1 = 0,
	J_CHANNEL_2,
	J_CHANNEL_3,
	J_CHANNEL_4,
	J_CHANNEL_MAX
};

typedef struct Serial_
{
	UINT m_uiPort;	//h20
	UINT m_uiBaudRate;
	UINT m_uiParity;
	UINT m_uiDataBits;
	UINT m_uiStopBit;
}ST_SERIAL;

/////////////////////////////////////////////////////////////////////////////
// CDlgESD dialog
class CFFUMonDlg;

class CDlgESD : public CDialog
{
// Construction
public:
	CDlgESD(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgESD)
	enum { IDD = IDD_DLG_ESD2 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	CFFUMonDlg*		m_pdlgMain;

	SOCKET_OPEN		m_stSockOpen;
	CString			m_strCH1Volt;
	CString			m_strCH2Volt; 
	CString			m_strCH3Volt;
	CString			m_strCH4Volt;
	double			m_dStaticVolt[4];
	BOOL m_bReceiveOut;
	int m_nSocketIndex;

//	int SocketDisConnectESD(CString strIP, CString strPort);
	int SocketDisConnectESD(int index = 1);
	int SocketConnectESD(CString strIP, CString strPort, int index = 1);
	void CreateSocket(int index, SOCKET_OPEN stSockOpen, BOOL bConnect=TRUE);
	void SetMainPoint(void* pvoid); 
	int SendToESD();
	BOOL	SendToDeviceWrite(BYTE btAddress1, BYTE btAddress2, BYTE Data1, BYTE Data2);
	BOOL	SendToDeviceRequest(BYTE btAddress1, BYTE btAddress2, BYTE btLength1, BYTE btLength2);
	LRESULT	OnSocketMsgReceive(WPARAM wParam, LPARAM lParam);
	int GetReceiveDataESD();
	void AnalysisReceiveData(UINT nSize, BYTE *szBuffer2);
	void AddListItem(CString sHead, char* pRawData);
	void ByteSwap(BYTE* pByte, BYTE* pSwapByte);
	CString CalVoltage(int nCurrentData);
	int DoWorkStartGetESD();
	int GetReceiveDataDUMY();


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgESD)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgESD)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	int SetType(int nType);
	int m_nType;
	CString			m_strIP;
	CString			m_strPort;	
	int ConnectESD(int nNum, int index = 1);
	int SetConnectInfo(CString strIp, CString strPort, ST_SERIAL stSerial);
	int SerialConnectESD(int index);
	int OpenPort(UINT nPort, DWORD dwRate,UINT bDataBits, UINT bParity, UINT bStopBits, UINT nMode);
	int m_nSerialPort;
	int m_nNum;
	int m_nNGCount;
	ST_SERIAL m_stSerial;
	CWinThread* m_pRcvThread;
	HANDLE	  m_hEvtRcv;
	HANDLE	  m_hEvtRcvStop;
	void ThreadBegin_RcvData(void);
	static UINT ThreadRcvData(LPVOID pParam);
	int RcvData(void);
	int DisConnectESD(int index);
	int SerialDisConnect(int nMode);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGESD_H__57539DD6_DC46_4DDF_9070_A37EBB655D82__INCLUDED_)
