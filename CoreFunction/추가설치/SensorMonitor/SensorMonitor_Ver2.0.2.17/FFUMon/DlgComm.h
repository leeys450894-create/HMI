#if !defined(AFX_DLGCOMM_H__11609579_ECA0_44FF_9D3E_3048D604F83F__INCLUDED_)
#define AFX_DLGCOMM_H__11609579_ECA0_44FF_9D3E_3048D604F83F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DlgComm.h : header file
#define UWM_RCVDATA WM_APP+100
//

//////////////////////////////////////////////////////////////////////////////////////////// yhjoo 20141209 EFU Table add.
const WORD crc16_table[256] = {
	0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
		0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
		0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
		0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
		0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
		0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
		0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
		0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
		0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
		0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
		0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
		0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
		0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
		0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
		0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
		0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
		0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
		0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
		0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
		0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
		0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
		0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
		0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
		0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
		0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
		0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
		0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
		0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
		0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
		0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
		0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
		0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

#define  CRC_COMPUTE(x_crc,x_byte) (((x_crc)>>8)^crc16_table[((x_crc)^(x_byte))&0x00ff])
////////////////////////////////////////////////////////////////////////////////////////////

// yj.nam 20190531 add
// EFU Manufacturer  
const enum EFU_TYPE{
	EFU_TYPE_ISU = 0,
	EFU_TYPE_SHINSUNG,
	EFU_TYPE_EBM,
	EFU_TYPE_MAYAIR,
	EFU_TYPE_HKC_H5,		// jsyoon 20201104 add
	EFU_TYPE_END,
};

// AMayAir Address Definition // yj.nam 20190521 add 
const enum EFU_ADDR_MAYAIR{
	MAYAIR_CONTROLLER_STATUS	= 0,
	MAYAIR_SOFTWARE_VERSION,
	MAYAIR_FAN_FAILURE_01,
	MAYAIR_FAN_FAILURE_02,
	MAYAIR_FAN_CUR_SPEED_01,
	MAYAIR_FAN_FAULT_CODE_01	= MAYAIR_FAN_CUR_SPEED_01+31,
	MAYAIR_CONTROLLER_ADDRESS   = MAYAIR_FAN_FAULT_CODE_01+ 31,
	MAYAIR_BAUD_RATE_CODING,
	MAYAIR_RAN_MODEL,
	MAYAIR_NUMBER_OF_FANS,
	MAYAIR_FAN_SET_SPEED_01,	
	MAYAIR_ADDR_END				= MAYAIR_FAN_SET_SPEED_01+31,
};

// 0x0001£ºHall Failure
//				0x0010: Overload
//				0x0080£ºlocked-rotor
//				0x0400£ºUnkown error
//				0x0800£ºFFU Disconnect
//				0x8000£ºRun
const enum EFU_STATUS_HKC{
	df_HKC_EFU_STS_RUN = 0,
	df_HKC_EFU_STS_STOP,
	df_HKC_EFU_STS_HALL_FAULURE,
	df_HKC_EFU_STS_OVERLOAD,
	df_HKC_EFU_STS_LOCKED_ROTOR,
	df_HKC_EFU_STS_UNKOWN_ERR,
	df_HKC_EFU_STS_FFU_DISCONNECT,
};

	

typedef struct stByte4
{
	BYTE	bt1;
	BYTE	bt2;
	BYTE	bt3;
	BYTE	bt4;
}ST_BYTE_4;

typedef union uni_
{
	ST_BYTE_4	stData;
	BYTE		btData[4];
	float		fData;
	int			iData;
	WORD		wData[2];
}UNI_TYPE;


// typedef struct Serial_
// {
// 	UINT m_uiPort;	//h20
// 	UINT m_uiBaudRate;
// 	UINT m_uiParity;
// 	UINT m_uiDataBits;
// 	UINT m_uiStopBit;
// }ST_SERIAL;

/////////////////////////////////////////////////////////////////////////////
// CDlgComm dialog
class CFFUMonDlg;
class CDlgComm : public CDialog
{
// Construction
public:
	int	   m_nSetAuccuraData;
	float* m_pfAuccuraData;
	void SendToLed(int iPort, int i, double dO2);
	CDlgComm(CWnd* pParent = NULL);   // standard constructor

	CFFUMonDlg *m_pFFUMon;

	void OpenPort(UINT nPort, DWORD dwRate,UINT bDataBits, UINT bParity, UINT bStopBits, UINT nMode);
	BOOL SendMsg(BYTE *byteBuff, int iLength);
	void AllStop();
	//test
	void SetEFUValue(UINT nEfu, UINT nValue);
	void GetState();
	void AllOff();
	static UINT WorkThreadProcGetState(LPVOID pParam);
	void StartThreadGetState();
	static UINT WorkThreadProcMonitoring(LPVOID pParam);
	static UINT WorkThreadProcAllOff(LPVOID pParam);
	static UINT WorkThreadProcRestoreValue(LPVOID pParam);
	void SetRestoreEFUValue();
	void StartThreadMonitoring();
	void ThreadBegin_RcvData(void);
	void ExitThreadAll();
	BOOL ExitThread(HANDLE hThread, DWORD dwDelay);
	static UINT ThreadRcvData(LPVOID pParam);
	BOOL RcvData(void);
	LRESULT OnRcvSerialData(WPARAM wParam, LPARAM lParam);
	void ReceiveMsgProcess(vector<BYTE> vecRvcData);
	double GetIntputValue(CString strRcv);
	void RestoreValue();
	void SetAllOff();
	void SetEFUData(vector<BYTE> vecRvcData, int nMode = 0);
	CWinThread*	 m_pGetStateThread;
	CWinThread* m_pMonitoringThread;
	CWinThread* m_pRcvThread;
	HANDLE m_hEvtGetStateStop;
	HANDLE m_hEvtMonitoringStop;
	HANDLE m_hEvtRcvStop;
	HANDLE m_hEvtRcv;
	char m_cEndDelimiter;
	vector<BYTE> m_vecRvcData;
	CString m_strReceive;
	BYTE HEXtoBYTE(char hex);
	UNI_TYPE	m_stConvertData;
	void ByteToFloat(BYTE* btSrc,  float &fDest, int nStart = 0, bool bInverse = false);
	//////////////////

	UINT m_nMode;
	UINT m_nPort;
	UINT m_nNum;

// Dialog Data
	//{{AFX_DATA(CDlgComm)
	enum { IDD = IDD_DIALOG_COMM };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgComm)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDlgComm)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnBnClickedOk();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	int RcvThermTest(void);
	int m_nMaxChannel;
	int m_nNGCount;

	int SetEFUSpeed(int nSpeed, int iGroup = 1, int nStart = 1, int nEnd = 1);
	//int GetEFUStatus(int iGroup, int iTarget);
	int GetEFUStatus(int iGroup = 1, int nStart = 0, int nEnd = 1);		//hh.joo 20181212 add
	int GetEFUSpeed(int iGroup = 1, int nStart = 0, int nEnd = 1);
	void GetCRC(int idxCRC, BYTE *byteBuff);		// yj.nam 20190520 add
	int ProceData(CString szMsg);
	int GetDataFetch(int iGroup = 0);
	int GetDataVoltage(int iGroup);
	int GetDataSub(int iGroup, int iType);
	int m_Validity;
	int m_subunitValidity[40];
	int m_nRcvValue;
	double m_dRcvValue;
	int Azbil0150_WriteRead(void);
	int PM5500_WriteRead(void);
	int PM5500_WriteRead_TotalPower(void);
	int PM5500_WriteRead_ActPowTotal(void);
	int PM5500_WriteRead_RS_line_volt(void);
	int PM5500_WriteRead_ST_line_volt(void);
	int PM5500_WriteRead_TR_line_volt(void);
	int PM5500_WriteRead_R_phase_curr(void);
	int PM5500_WriteRead_S_phase_curr(void);
	int PM5500_WriteRead_T_phase_curr(void);
	int PM5500_WriteRead_Electric_cur_I(void);
	int PM5500_WriteRead_Electric_volt_U(void);
	int PM5500_WriteRead_Active_Power_W(void);
	int PM5500_WriteRead_Power_factor_cso(void);
	int PM5500_WriteRead_Frequency_f(void);
	int PM5500_WriteRead_THDi(void);
	int PM5500_WriteRead_THDu(void);
	int MakerCRC(BYTE* pBytebuf);
	int PM5500_WriteRead_THDi_A(void);
	int PM5500_WriteRead_THDi_B(void);
	int PM5500_WriteRead_THDi_C(void);
	void SetIonizer(vector<BYTE> vecRvcData);
	void Ionizor_WriteRead(void);
	void TestSendEFU(int nSpeed, int iGroup = 1, int nStart = 1, int nEnd = 1, int nType = 0);
	int m_nSendType;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGCOMM_H__11609579_ECA0_44FF_9D3E_3048D604F83F__INCLUDED_)
