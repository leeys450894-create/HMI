#pragma once

#include "..\Include\API_SocketControl.h"
// CDlgPLC 대화 상자입니다.
#define dfMAXFDCITEM 300
#define dfTIMEOUTCNT 200

#define CDrive	195
#define DDrive	736
#define EDrive	931
#define ICEDrive 465
#define ICPDrive 238

typedef struct 
{
	int nNum;
	double dData[dfMAXFDCITEM];
}PLCDDATA, *PPLCDDATA;

class CDlgPLC : public CDialog
{
	DECLARE_DYNAMIC(CDlgPLC)

public:
	CDlgPLC(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgPLC();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DIG_PLC };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:

	int				m_iStartSafetyTime;			// eh.kik 20170206 add.
	BOOL			m_bStartSafety;				// eh.kik 20170206 add.	

	BOOL			m_bStartSafetyPLCRead;		// eh.kik 20171101 add.	
	char			m_chReceiveLog[2000];		// eh.kim 20170719 add.
	// eh.kik 20180104 add.
	BOOL			m_bStartSafetyDefault;
	int				m_iStartSafetyDefaultCount;
	int				m_iStartSafetyDefaultCountMax;
	//	
	int				m_nMode;
	int				m_nStationNum;
	int				m_nPlcPort;
	UINT			m_nSocketIndex;
	BOOL			m_bOutOfService;
	CFile			m_fileCommLog;
	CString			m_szIPAddress;
	CString			m_strReceiveLog;

	SOCKET_OPEN		m_stSockOpen;	

	int				m_nUseEFUNum;
	double			m_bFilter_Standard[dfMAXFDCITEM];
	double			m_bFilter_Min[dfMAXFDCITEM];	
	double			m_bFilter_Max[dfMAXFDCITEM];
	BOOL			m_bFilterUse[dfMAXFDCITEM];
	BOOL			m_bFilterValue[dfMAXFDCITEM];
	int				m_nFDCID[dfMAXFDCITEM];
//	CStringArray	m_strArray;					

	double			m_dSensorData[dfMAXFDCITEM];			//sypark 20180430 add
	double			m_valArray[dfMAXFDCITEM];
	BOOL			m_bShowFLog[dfMAXFDCITEM];			// gdh 20180321	add
	double			*pPLCReciveValue;
	 
	CString			m_strFilterFLogList;			// gdh 20180321 add
	CStringArray	m_strArrayForFLogList;			// gdh 20180321 add

	int	m_nUseMonitorNum;		//sypark 20180608 add

	int				m_iRefAddressToWrite[4][dfMAXFDCITEM];	// yj.nam 20160719 add   

	void			SplitString(CString strLine, char split);	//gdh 20180321 add
	void			SetFilterFLogList();	//gdh 20180321 add
	void			WriteFDC_Filtering(BOOL bUse);
	BOOL			DoWorkStartGetPLCMemoryBitAll();
	BOOL			DoWorkStartGetPLCMemoryBit(CString strDeviceText,int iDeviceNum);
	BOOL			DoWorkStartGetPLCSendRetrun();
	int				WriteFile(CFile *pFile, CString &sPath, CString sLog);
	void			GetConfig();
	void			CreateSocket(SOCKET_OPEN stSockOpen);
	void			CloseSocket();
	void			EventLog(CString &s);
	void			EventLog2(char *c);		// eh.kim 20170719 add.
	void			EventDLog(CString &s);		
	CString			gf_MakeCurTimeString();
	CString			gf_MakeCurTimeString2();
	CString			GetLogFileName();
	BOOL			WriteDData(CString strName, int DDevice, double Val, BOOL bWrite = TRUE);
	BOOL			GetReceiveData(CString *reply, UINT nRcvSize, UINT nTimeout); 
	void			Cmd_Send(CString strSend);
	int				getPOW(int nNum);		//sypark 20180529 add


	int				SetMStateCodeToSpread();				
	void			SetMainPoint(void* pvoid);
	afx_msg void	OnBnClickedBtnConnetPlc();
	virtual BOOL	OnInitDialog();
	CString			m_strAppPath;
	void			SetAppPath(CString str) { m_strAppPath = str; };
//	afx_msg			void OnTimer(UINT_PTR nIDEvent);
	CListBox		m_lbPlcLog;

	void			WriteFDC_Data(int uSVType = 0);
	void			SleepX(DWORD dwDelay);		

	void			WriteFDC();
	BOOL			ReadPLC_DData(BOOL bUse = FALSE);

	CListBox		m_lbDPlcLog;	
	 
	afx_msg void OnDestroy();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	BOOL			m_bSendOK;
	UINT			m_nUseSize;
	UINT			m_nUseGap;
	int WriteFDC_OffsetData(void);
};
