// FFUMonDlg.h : header file
//

#if !defined(AFX_FFUMONDLG_H__DF5343EA_C0D8_4947_BB96_0E7C46CB8F94__INCLUDED_)
#define AFX_FFUMONDLG_H__DF5343EA_C0D8_4947_BB96_0E7C46CB8F94__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#include "DlgWarnMsg.h"		//20140729added by Jay Hong
#include "BtnST.h"
#include "Label.h"
#include "LedButton.h"
#include "..\Include\AccuraClient.h"
#endif // _MSC_VER > 1000


/////////////////////////////////////////////////////////////////////////////
// CFFUMonDlg dialog
//#include "afxwin.h"
// #include "vector"	//yhjoo 20141118 remove

#define dfBUFFER_SIZE		2048 
#define dfCR				0x0D
#define dfLF				0x0A
#define dfCRLF				"\r\n"
#define dfBRACE				0x3E
#define dfSTX				0x02
#define dfETX				0x03
#define dfINFOMODE1			0x8A
#define dfINFOMODE2			0x87
#define dfRPMMODE1			0x89
#define dfRPMMODE2			0x84
#define dfRPMFlagOK			0xB9
#define dfLV32_ID			0x81
#define dfDPU_ID			0x9F
#define SAFE_DELETE(p)			if(p!=NULL)		{ delete p; p = NULL;}
#define SAFE_DELETE_AR(ptr)		if (ptr!=NULL)	{ delete [] ptr;	ptr = NULL;	}
#define	CLR_GREEN						RGB(0x00, 0xFF, 0x00)
#define	CLR_GRAY						RGB(0x80, 0x80, 0x80)

#define NORMAL_ALARM 0x80
#define EC_ALARM 0xA0
#define MOTOR_ALARM 0xC0
#define COMM_ALARM 0x00

#define UWM_RCVDATA			WM_APP+100

#define WM_TRAYICON_MSG		WM_USER + 1 

#define dfMAX_EFU			160	//20181207 edhan change 32 to 160

#define dfMAX_THERMETER		10
#define dfMAX_H2O			5

#define TIMER_GET_DATA		10

const enum SENSOR_MODE{
	DEFAULT_LOG     = -1,
	SENSOR_MODE_O2	= 0,
	SENSOR_MODE_PRESSURE,
	ELECTRONIC_DISPLAY,
	SENSOR_MODE_KMN2,		//jhseo 20170920 add for KM-N
	SENSOR_MODE_ACCURA2,

	SENSOR_MODE_H2O	  = 11,
	SENSOR_MODE_H2O_END = SENSOR_MODE_H2O+dfMAX_H2O,

	SENSOR_MODE_TEMP1 = 21,
	SENSOR_MODE_TEMP_END = SENSOR_MODE_TEMP1+dfMAX_THERMETER,

	SENSOR_MODE_EFU,		//32
	SENSOR_MODE_EFU2,
	//20181207 edhan add
	SENSOR_MODE_EFU3,
	SENSOR_MODE_EFU4,
	SENSOR_MODE_EFU5,
	//

	SENSOR_MODE_ESD,		//lhj 20171223 add
	SENSOR_MODE_ESD2,
	SENSOR_MODE_ESD3,
	SENSOR_MODE_ACCURA,		//jh.lee 20160722 add	// jsyoon 20180622 add
	SENSOR_MODE_ELECTRICITY_METER,	// yj.nam 20190327 add for Electricity Meter
	SENSOR_MODE_PM5500,
	SENSOR_MODE_AZBIL,
	SENSOR_MODE_IONIZER,
	SENSOR_MODE_IONIZER2,
	SENSOR_MODE_END,
};


#define dfMAX_USE_CONNECT	46	//20181207 edhan change 40 to 44



#include "DlgESD.h" //lhj 20171223 add
#include "afxwin.h"
#include "DlgComm.h"

// using namespace std;	//yhjoo 20141118 remove

class CDlgComm;
class CFFUMonDlg : public CDialog
{
// Construction
public:
	//lhj 20171223 add ///////////////////////////////////////////////////////
	CDlgESD* m_pDlgESD[10];		// jsyoon 20181017 change
//	CDlgESD m_dlgESD;
	CWinThread*		m_pThreadESDRead;		// ESD
	BOOL			m_bESDReadDone;
	HANDLE			m_hEvtThreadESDReadKilled;     //set by self	쓰레드가 죽었다는 핸들
	HANDLE			m_hEvtThreadESDReadTerminated;
	static UINT ThreadESDReadProc(LPVOID pParam);
	///////////////////////////////////////////////////////////////////////////


	int m_iOldModule;
	void UpdateBtn(int iModule);
	void SendEFUValue(int nEfu, double dVal, int nMode);		//20181217 edhan add parameter
	CString m_strLogPath;
	void WriteInfoLog(CString strMsg);
	double GetIntputValue(CString strRcv);
	BOOL SetWarning(BOOL bSet, int nAlarmID);
// 	static UINT WorkThreadProcAllOff(LPVOID pParam);
// 	static UINT WorkThreadProcRestoreValue(LPVOID pParam);

//	void StartThreadGetState();
// 	static UINT WorkThreadProcGetState(LPVOID pParam);
// 	void GetState();

//	CWinThread*			m_pGetStateThread;
	HANDLE				m_hEvtGetStateStop;

// 	void StartThreadMonitoring();
// 	static UINT WorkThreadProcMonitoring(LPVOID pParam);
// 	
// 	CWinThread*			m_pMonitoringThread;
// 	HANDLE				m_hEvtMonitoringStop;

	int DLOG(CString s, UINT nMode);	//yhjoo 20141118 add short nMode
	int WriteFile(CFile *pFile, CString sPath, CString sLog);
//	void AllOff();
// 	void SetEFUValue(UINT nEfu, UINT nValue);
// 	void SetRestoreEFUValue();
	CFFUMonDlg(CWnd* pParent = NULL);	// standard constructor

	struct FFU_DATA
	{
		CString strID;
		CString strPV;
		CString strALARM;
		CString strSV;
	};
	
//	CWinThread*			m_pRcvThread;
//	HANDLE				m_hEvtRcvStop;
	
	//	CStringArray		m_saMsgQueue;
	
	CString m_strConfigFile;
	UINT m_uiPort;	//h20
	UINT m_uiBaudRate;
	UINT m_uiBaudRate8;
	UINT m_uiParity;
	UINT m_uiDataBits;
	UINT m_uiStopBit;

	ST_SERIAL m_stSerial[SENSOR_MODE_END];
	
	UINT m_uEFUSpeedMax[dfMAX_EFU];
	UINT m_uEFUSpeed[dfMAX_EFU];
	UINT m_nEFUCnt[5];		//20181207 edhan change 2 to 5
	UINT m_nEFUAllCnt;
	
	CString m_strPassWord;
	UINT m_nLocalAlarm;	//20140729 added by Jay Hong 1-->Show Warm msg for EFU alarm, 0 --> Hide Warm Alarm Msg
	CDlgWarnMsg*	m_pWarnMsgDlg;	//20140729 added by Jay Hong
	
	char				m_cStartDelimiter;
	char				m_cEndDelimiter;
	vector<BYTE>		m_vecRvcData;
	vector<FFU_DATA>	m_vecEFUData;
	
	void				SetCboData();
	void				GetSettingFile();
	int					SetcboCurSel(CComboBox *pcbo, CString strValue);
	CString				GetFileValue(CString strAPPName, CString strKeyName , CString strConfigFile, CString strDefault = _T("0"));
	char				GetDelimiter(CString strDelimiter);
	
// 	void				ThreadBegin_RcvData(void);
// 	static UINT			ThreadRcvData(LPVOID pParam);
	BOOL				RcvData(void);
	LRESULT				OnRcvSerialData(WPARAM wParam, LPARAM lParam);
	void				ReceiveMsgProcess(vector<BYTE> vecRvcData);
//	void				SetEFUData(vector<BYTE> vecRvcData);
	void				ExitThreadAll();
	BOOL				ExitThread(HANDLE hThread, DWORD dwDelay);
	int					GetByteToIDec(BYTE byteValue);
	BOOL				CheckAlarm(BYTE byteAlarm);
	
	void				WriteLogList(CString strMsg, int nMode);	//yhjoo 20141118 add short nMode
	CString				GetCboValue(CComboBox *pcbo);
	
	BOOL				WriteValue(LPCTSTR section, LPCTSTR key, LPCTSTR value);
	BOOL				SendMsg(BYTE *byteBuff, int iLength);
	WORD				GetCRC16(const WORD *nData, WORD wLength);	//yhjoo 20141215 add

	CString				m_strReceive;
	float				m_fValue;

	//yhjoo 20141118 add
	CString m_strPressureCMD;

	CDlgComm		*m_pDlgComm[dfMAX_USE_CONNECT];		
	CFont			font;	
	int				m_nMode;
	void			SetControlShow(BOOL bShow);
	//void			SetSendData();
	BOOL			ConnectTCP(BOOL bConnect = TRUE);
	
	int				m_bEFU_Type;	//yhjoo 20160704 add		//20181116 edhan change BOOL to int
	BOOL			m_bUse_FDC;		//yhjoo 20160704 add

	UINT			m_uiPortSensor[10];
	BOOL			m_bUseSensor[10];

	UINT			m_uiPortEFU[5];	// 20181207 edhan change 2 to 5
	BOOL			m_bUseEfuNum[5];	//20181207 edhan add

	// 온도계
	UINT			m_uiPort485[dfMAX_THERMETER];
	TCHAR			m_szThermometer[dfMAX_THERMETER][32];
	double			m_dThermometer[dfMAX_THERMETER][32];	// 최대 32개 까지 연결가능
//	double			m_dTherm_old[dfMAX_THERMETER][32];
	BOOL			m_bUseTherm[dfMAX_THERMETER];
	UINT			m_uThermometerSub[dfMAX_THERMETER]; 
	TCHAR			m_szThermometerSub[dfMAX_THERMETER][32][32];
	//////////////////////////////////////////////////////////////////////////

	double			m_dKMN2Result[2];		//jhseo 20170920 add for KM-N2

	TCHAR			m_szH2O[dfMAX_H2O][32];
	UINT			m_uiPortH2O[dfMAX_H2O];
	double			m_dH2O[dfMAX_H2O];
	BOOL			m_bUse_H2O[dfMAX_H2O];

	double			m_dOldO2;
	double			m_dO2;
	
// 	double			m_dTemp1;
// 	double			m_dTemp2;
// 	double			m_dTemp3;
	double			m_dPressure;
	int				m_nEFUStatus[dfMAX_EFU];
	double			m_dEFU[dfMAX_EFU];
	double			m_dEFU_old[dfMAX_EFU];	//ksk 20150708 add
	//ksk 20150628 add
//	double			m_dH2O_old[2];
	double			m_dO2_old;
// 	double			m_dTemp1_old;
// 	double			m_dTemp2_old;
// 	double			m_dTemp3_old;
	double			m_dPressure_old;
	//end
	
//	BOOL			m_bUse_O2;
// 	BOOL			m_bUse_Temp1;
// 	BOOL			m_bUse_Temp2;
// 	BOOL			m_bUse_Temp3;
//	BOOL			m_bUse_Pressure;
	BOOL			m_bUse_EFU;
	int				m_nIdex;
//	BOOL			m_bUse_LED;
//	BOOL			m_bUse_KMN2;	//jhseo 20170920 add for KM-N2

	BOOL			m_bUse_ESD;		//lhj 20171223 add
	CStringArray	m_strIPESD;		//lhj 20171223 add
	CString			m_strPortESD;	//lhj 20171223 add
	UINT			m_nCnt_ESD;
	UINT			m_nType_ESD;	// TCP : 0, RS232 : 1

	BOOL			m_bSetSendData;

	BOOL			m_bRecvThreadRun;
	HANDLE			m_hRecvThread;
	WSADATA			m_wsaData;
	SOCKET			m_socetClient;

	CString			m_strIP;
	int				m_nPort;

	BOOL			m_bSocketConnect;	//20150604 added
	/////////////////////////////////////
	// Dialog Data
	//{{AFX_DATA(CFFUMonDlg)
	enum { IDD = IDD_FFUMON_DIALOG };
	CButtonST	m_chkModule[dfMAX_USE_CONNECT];
// 	CButtonST   m_chkModule16;
// 	CButtonST	m_chkModuleAccura;
// 	CButtonST	m_chkModuleESD; //lhj 20171223 add
// 	CButtonST	m_chkModuleKMN2;
// 	CButtonST	m_chkModule12;
// 	CButtonST	m_chkModule11;
// 	CButtonST	m_chkModule10;
// 	CButtonST	m_chkModule9;
// 	CButtonST	m_chkModule8;
// 	CButtonST	m_chkModule7;
// 	CButtonST	m_chkModule6;
// 	CButtonST	m_chkModule5;
// 	CButtonST	m_chkModule4;
// 	CButtonST	m_chkModule3;
// 	CButtonST	m_chkModule2;
// 	CButtonST	m_chkModule1;
	CLabel		m_ledConnected;
	CEdit		m_edtCMD;
	CComboBox	m_cboLVID;
	CComboBox	m_cboBLIDEnd;
	CComboBox	m_cboBLIDStart;
	CSpinButtonCtrl	m_spRPM;
	CComboBox	m_cboPort;
	CComboBox	m_cboBaudRate;
	CComboBox	m_cboParity;
	CComboBox	m_cboDataBit;
	CComboBox	m_cboStopBit;
	CComboBox	m_cboStartDelimiter;
	CComboBox	m_cboEndDelimiter;
	CEdit m_editSendMsg;
	CListBox m_ListBox;
	int		m_nRPM;
	CString	m_strCMD;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFFUMonDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CFFUMonDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnButtonOpen();
	afx_msg void OnButtonClose();
	afx_msg void OnButtonSend();
	afx_msg void OnButtonSave();
	afx_msg void OnDestroy();
	afx_msg void OnButtonRpm();
	afx_msg void OnButtonStatus();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnButtonStatus2();
	afx_msg void OnSelchangeComboPort();
	afx_msg void OnButtonRestoreValue();
	afx_msg void OnButtonAllOff();
	afx_msg void OnButtonAllShow();
	afx_msg void OnButtonGetvalue();
	afx_msg void OnButtonClear();
	afx_msg void OnButtonH2o();
	afx_msg void OnButtonO2();
	afx_msg void OnButtonTemp1();
	afx_msg void OnButtonTemp2();
	afx_msg void OnButtonTemp3();
	afx_msg void OnButtonTemp4();
	afx_msg void OnButtonTemp5();
	afx_msg void OnButtonTemp6();
	afx_msg void OnButtonPressure();
	afx_msg void OnButtonEfu();
	afx_msg void OnButtonTcp();
	afx_msg void OnBnClickedChk10();
	afx_msg void OnBnClickedChk9();
	afx_msg void OnBnClickedChk16();
	afx_msg void OnChangeEditCmd();
	virtual void OnOK();
	afx_msg void OnMini();
	afx_msg void OnChkKmn2();
	afx_msg void OnChkEsd();
	afx_msg void OnChkAccura();
	afx_msg void OnMenuitemExit();
	afx_msg void OnMenuitemOpen();
	afx_msg void OnMenuitemWindowstartautorun();
	afx_msg void OnButtonChk1();
	afx_msg void OnButtonChk2();
	afx_msg void OnButtonChk3();
	afx_msg void OnButtonChk4();
	afx_msg void OnButtonChk5();
	afx_msg void OnButtonChk6();
	afx_msg void OnButtonChk7();
	afx_msg void OnButtonChk8();
	afx_msg void OnButtonChk9();
	afx_msg void OnButtonChk10();
	afx_msg void OnButtonChk11();
	afx_msg void OnButtonChk12();
	afx_msg void OnButtonChk13();
	afx_msg void OnButtonChk14();
	afx_msg void OnButtonChk15();
	afx_msg void OnButtonChk16();
	afx_msg void OnButtonChk17();
	afx_msg void OnButtonChk18();
	afx_msg void OnButtonChk19();
	afx_msg void OnButtonChk20();
	afx_msg void OnButtonChk21();
	afx_msg void OnButtonChk22();
	afx_msg void OnButtonChk23();
	afx_msg void OnButtonChk24();
	afx_msg void OnButtonChk25();
	afx_msg void OnButtonChk26();
	afx_msg void OnButtonChk27();
	afx_msg void OnButtonChk28();
	afx_msg void OnButtonChk29();
	afx_msg void OnButtonChk30();
	afx_msg void OnButtonChk31();
	afx_msg void OnButtonChk32();
	afx_msg void OnButtonChk33();
	afx_msg void OnButtonChk34();
	afx_msg void OnButtonChk35();
	afx_msg void OnButtonChk36();
	afx_msg void OnButtonChk37();
	afx_msg void OnButtonChk38();
	afx_msg void OnButtonChk39();
	afx_msg void OnButtonChk40();
	afx_msg void OnButtonChk41();
	afx_msg void OnButtonChk42();
	afx_msg void OnButtonChk43();
	afx_msg void OnButtonChk44();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


public:

	typedef struct
	{
		//Acura#1 Data
		float Vavg_ll_1;    //삼상의 선간전압 평균
		float lavg_1;	    //삼상의 전류 평균
		float Ptot_1;	    //삼상의 유효전력 종합
		int	  Net_1;		//수전 유효전력량과 송전 유효전력량의 차

		int nSubDev;

		int N2_1;		    //N2_1 수치
		int N2_1_Total;		//N2_1 총합수치
		int N2_2;		    //N2_2 수치
		int N2_2_Total;		//N2_2 총합수치
		
		int CDA_1;			//CDA_1 수치
		int CDA_1_Total;	//CDA_1 총합수치

		// Accura Data		// jsyoon 20190318 add
		int nValueCnt;
		int nValue[20];
		int nValueTotal[20];
		
// 		//Acura#2 Data
// 		float Vavg_ll_2;    //≫i≫oAC ¼±°￡Au¾Ð Æo±O
// 		float lavg_2;		//≫i≫oAC Au·u Æo±O
// 		float Ptot_2;		//≫i≫oAC A?E¿Au·A A¾CO
// 		int   Net_2;		//¼oAu A?E¿Au·A·®°u ¼UAu A?E¿Au·A·®AC A÷
		
	}ADDR_DATA;
	ADDR_DATA m_sData;

	typedef struct
	{
		CString			m_Addr_str;  
		CString			m_Addr_hex;  
		int				m_Addr_int;  
		CString			m_Addr;		 
		DWORD			m_Address;
		char*			m_IpAddr;
		BOOL			m_nStop;	
	}ADDR_CONVERT;
	ADDR_CONVERT m_cDATA;

	typedef struct
	{
		double		m_dVoltAvg;
		double		m_dTotalPow;		//3207
		double		m_dInstantPow;		//3059
		double		m_dRS_line_volt;	//3019
		double		m_dST_line_volt;	//3021	
		double		m_dTR_line_volt;	//3023	
		double		m_dR_Phase_curr;	//2999
		double		m_dS_Phase_curr;	//3001
		double		m_dT_Phase_curr;	//3003

		//
		double		m_dElect_cur_I;		//	
		double		m_dElect_volt_U;	//	
		double		m_dElect_act_pow;	//	
		//
		double		m_dPower_factor_cso;//3083					//0x0C0B
		double		m_dFrequency;		//3109					//0x0C25
		double		m_dTHDi;			//21308-1 || 21320-1	//0x533B	//or //0x5347
		double		m_dTHDu;			//21328-1				//0x534F

		double		m_dTHDi_A;			//21300->21299	//0x5333
		double		m_dTHDi_B;			//21302->21301	//0x5335
		double		m_dTHDi_C;			//21304->21303	//0x5337	
	}PM55_DATA;
	PM55_DATA m_cDATA_PM55;

	typedef struct
	{
		BOOL		bConnected;
		BOOL		bAlarm;		
		double		dRunTime;	
		double		dOverTime;
	}IONIZER_DATA;

	IONIZER_DATA m_cDATA_IONIZER[8];

	AccuraClients&  m_rAcClient;  
	AccuraModbusTcpClient** clientsArray;
	void		    ReadMeasurement(AccuraModbusTcpClient* client, int index);  
	void		    AcuraReadMeasure(); 
	BOOL			AddDevice(CString ipAddr);  
	void			ReadDevinfo();
	float			ToFloat(unsigned short* buffer, int index);  
	int				ToInt32(unsigned short* buffer, int index);  
	int				ToInt32new(unsigned short* buffer, int index);  
	CString			IntToHex_String(int Addr_int);  
	DWORD			HexStrToInt(CString aStr);
	void			Extract_Convert(CString m_strIP_Acura); 
	void			CStringIPtoDWORD(CString m_strIP_Acura); 
	BOOL			m_bUse_ACCURA;
	UINT 			m_bCnt_ACCURA;

	//jh.lee 20160722 add - Get Data Thread
	HANDLE			m_hEvtGetsDataStop;
	CWinThread*		m_pGetsDataThread;				// ACCURA
	void			StartThreadGetsData();
	static UINT		WorkThreadProcGetsData(LPVOID pParam);
	ADDR_DATA*		m_pAccuraData;

	// TrayIcon
	BOOL			SHRegDeleteValue(HKEY hKey, LPCTSTR lpKey, LPCTSTR lpValue);
	BOOL			SHRegWriteString(HKEY hKey, LPCTSTR lpKey, LPCTSTR lpValue, LPCTSTR lpData);
	BOOL			SHRegReadString(HKEY hKey, LPCTSTR lpKey, LPCTSTR lpValue, LPCTSTR lpDefault, LPTSTR lpRet, DWORD nSize);

	BOOL			m_bTrayIcon;
	LRESULT			TrayIconMSG(WPARAM wParam, LPARAM lParam);
	LRESULT			OnTrayReset(WPARAM wParam, LPARAM lParam);	//	yj.k 20180201 add explorer shotdown -> restart -> TrayIcon Retry show
	void			ResTrayIcon();
	bool			FFUMonDlgExit();


	afx_msg void OnBnClickedChk19();
	CButtonST m_chkModule19;
	int m_nNumChk;
	CString m_strEQName;
	CListBox m_ListBox2;
	int ShowBtnEFUCtrol(bool bShow);
	bool m_bShowEFU;
	afx_msg void OnBnClickedButtonGetSpeed();
	afx_msg void OnBnClickedButtonGetState();
	CString			GetVersionInfo ();	
	afx_msg void OnBnClickedChkEfu();
	BOOL m_bUse_PM5500;
	int  m_iAzbliValue[5];
	BOOL m_bUse_AZBIL;
	BOOL m_bUse_Ionizer;	// jsyoon 20200526 add
	afx_msg void OnBnClickedChk45();
	afx_msg void OnBnClickedChk46();

	int			m_iH2OSensorType;		// nk.lee 2020.06.15 add

	public:
		BOOL	m_bTimer;
		CString m_strData;

	public:
		CString GetTempData ();
		CString GetEFUData ();
		CString GetEtcData ();
		void	SetMonitorSensorData ();
		void	SendSensorData (CString strData);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FFUMONDLG_H__DF5343EA_C0D8_4947_BB96_0E7C46CB8F94__INCLUDED_)
