#if !defined(AFX_DLGNMCCREPORT_H__087B2A06_90CD_4554_A027_BDF3D248B089__INCLUDED_)
#define AFX_DLGNMCCREPORT_H__087B2A06_90CD_4554_A027_BDF3D248B089__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#include "SheardDefineMCC.h"
#include "../common/MySpreadDll.h"
#include "afxwin.h"
#include "common\StaticSpread.h"

#endif // _MSC_VER > 1000
// DlgnMCCReport.h : header file
//
#define FPS_RESOURCE_ID	10000	// FPSpreadDll : resource 생성할 때 쓸 default id


//const enum EQUIP_TYPE_VALUE {
//	EQU_TYPE_ICE = 0,
//	EQU_TYPE_ICM,
//	EQU_TYPE_ICP,
//	EQU_TYPE_OCP,
//	EQU_TYPE_END
//};

/////////////////////////////////////////////////////////////////////////////
// CDlgnMCCReport dialog

const enum MCC_JOB_NO{
		WORK_MCC_SEND_LOGFILE	= 100,
		WORK_MCC_KILL_JOB		,
		WORK_MCC_NONE,
};

#define MCC_NAME_FILE					_T("MCCNAME.csv")	// ILOG
#define MCC_ERROR_FILE					_T("MCCERROR.csv")	// ERROR
#define MCC_ACTION_FILE					_T("MCCACTION.csv")	// ACTION
#define MCC_SIGNAL_FILE					_T("MCCESIGNAL.csv")	// SIGNAL

#define WM_MCC_ADDQUEUE		(WM_USER + 5000)

enum MCC_ILOG_COL {						 
	MCC_ILOG_COL_NO = 0,
	MCC_ILOG_COL_TIME,
	MCC_ILOG_COL_TYPE,
	MCC_ILOG_COL_ITEM,
	MCC_ILOG_COL_ITEM_E  = MCC_ILOG_COL_ITEM + 150,
	MCC_ILOG_COL_ITEM2   = MCC_ILOG_COL_ITEM_E,
	MCC_ILOG_COL_ITEM2_E = MCC_ILOG_COL_ITEM2 + 17,

// 	WT_X_SET_SPD
// 	WT_X_SET_POS
// 	WT_X_CUR_SPD
// 	WT_X_CUR_POS
// 	WT_X_CUR_TOR
// 	WT_Y_SET_SPD
// 	WT_Y_SET_POS
// 	WT_Y_CUR_SPD
// 	WT_Y_CUR_POS
// 	WT_Y_CUR_TOR
// 	WT_TURN_SET_SPD
// 	WT_TURN_SET_POS
// 	WT_TURN_CUR_SPD
// 	WT_TURN_CUR_POS
// 	WT_TURN_CUR_TOR
// 	LD_X_SET_SPD
// 	LD_X_SET_POS
// 	LD_X_CUR_SPD
// 	LD_X_CUR_POS
// 	LD_X_CUR_TOR
// 	LD_Y_SET_SPD
// 	LD_Y_SET_POS
// 	LD_Y_CUR_SPD
// 	LD_Y_CUR_POS
// 	LD_Y_CUR_TOR
// 	LD_Z_SET_SPD
// 	LD_Z_SET_POS
// 	LD_Z_CUR_SPD
// 	LD_Z_CUR_POS
// 	LD_Z_CUR_TOR
// 	LD_LP_SET_SPD
// 	LD_LP_SET_POS
// 	LD_LP_CUR_SPD
// 	LD_LP_CUR_POS
// 	LD_LP_CUR_TOR
// 	LD_TURN_SET_SPD
// 	LD_TURN_SET_POS
// 	LD_TURN_CUR_SPD
// 	LD_TURN_CUR_POS
// 	LD_TURN_CUR_TOR
// 	CAM_X_SET_SPD
// 	CAM_X_SET_POS
// 	CAM_X_CUR_SPD
// 	CAM_X_CUR_POS
// 	CAM_X_CUR_TOR
// 	CAM_LEFT_Y_SET_SPD
// 	CAM_LEFT_Y_SET_POS
// 	CAM_LEFT_Y_CUR_SPD
// 	CAM_LEFT_Y_CUR_POS
// 	CAM_LEFT_Y_CUR_TOR
// 	CAM_RIGHT_SET_SPD
// 	CAM_RIGHT_SET_POS
// 	CAM_RIGHT_Y_CUR_SPD
// 	CAM_RIGHT_Y_CUR_POS
// 	CAM_RIGHT_Y_CUR_TOR
// 	CAM_LEFT_Z_SET_SPD
// 	CAM_LEFT_Z_SET_POS
// 	CAM_LEFT_Z_CUR_SPD
// 	CAM_LEFT_Z_CUR_POS
// 	CAM_LEFT_Z_CUR_TOR
// 	CAM_RIGHT_Z_SET_SPD
// 	CAM_RIGHT_Z_SET_POS
// 	CAM_RIGHT_Z_CUR_SPD
// 	CAM_RIGHT_Z_CUR_POS
// 	CAM_RIGHT_Z_CUR_TOR
// 	CAM_LEFT_ZOOM_SET_SPD
// 	CAM_LEFT_ZOOM_SET_POS
// 	CAM_LEFT_ZOOM_CUR_SPD
// 	CAM_LEFT_ZOOM_CUR_POS
// 	CAM_LEFT_ZOOM_CUR_TOR
// 	CAM_RIGHT_ZOOM_SET_SPD
// 	CAM_RIGHT_ZOOM_SET_POS
// 	CAM_RIGHT_ZOOM_CUR_SPD
// 	CAM_RIGHT_ZOOM_CUR_POS
// 	CAM_RIGHT_ZOOM_CUR_TOR
// 	CAM_LEFT_THETA_SET_SPD
// 	CAM_LEFT_THETA_SET_POS
// 	CAM_LEFT_THETA_CUR_SPD
// 	CAM_LEFT_THETA_CUR_POS
// 	CAM_LEFT_THETA_CUR_TOR
// 	CAM_RIGHT_THETA_SET_SPD
// 	CAM_RIGHT_THETA_SET_POS
// 	CAM_RIGHT_THETA_CUR_SPD
// 	CAM_RIGHT_THETA_CUR_POS
// 	CAM_RIGHT_THETA_CUR_TOR
// 	ANGLE_LEFT_Z_SET_SPD
// 	ANGLE_LEFT_Z_SET_POS
// 	ANGLE_LEFT_Z_CUR_SPD
// 	ANGLE_LEFT_Z_CUR_POS
// 	ANGLE_LEFT_Z_CUR_TOR
// 	ANGLE_RIGHT_Z_SET_SPD
// 	ANGLE_RIGHT_Z_SET_POS
// 	ANGLE_RIGHT_Z_CUR_SPD
// 	ANGLE_RIGHT_Z_CUR_POS
// 	ANGLE_RIGHT_Z_CUR_TOR
// 	ANGLE_LEFT_FCS_SET_SPD
// 	ANGLE_LEFT_FCS_SET_POS
// 	ANGLE_LEFT_FCS_CUR_SPD
// 	ANGLE_LEFT_FCS_CUR_POS
// 	ANGLE_LEFT_FCS_CUR_TOR
// 	ANGLE_RIGHT_FCS_SET_SPD
// 	ANGLE_RIGHT_FCS_SET_POS
// 	ANGLE_RIGHT_FCS_CUR_SPD
// 	ANGLE_RIGHT_FCS_CUR_POS
// 	ANGLE_RIGHT_FCS_CUR_TOR
// 	ANGLE_LEFT_TURN_SET_SPD
// 	ANGLE_LEFT_TURN_SET_POS
// 	ANGLE_LEFT_TURN_CUR_SPD
// 	ANGLE_LEFT_TURN_CUR_POS
// 	ANGLE_LEFT_TURN_CUR_TOR
// 	ANGLE_RIGHT_TURN_SET_SPD
// 	ANGLE_RIGHT_TURN_SET_POS
// 	ANGLE_RIGHT_TURN_CUR_SPD
// 	ANGLE_RIGHT_TURN_CUR_POS
// 	ANGLE_RIGHT_TURN_CUR_TOR
// 	BG_LEFT_ZSET_SPD
// 	BG_LEFT_Z_SET_POS
// 	BG_LEFT_Z_CUR_SPD
// 	BG_LEFT_Z_CUR_POS
// 	BG_LEFT_Z_CUR_TOR
// 	BG_RIGHT_Z_SET_SPD
// 	BG_RIGHT_Z_SET_POS
// 	BG_RIGHT_Z_CUR_SPD
// 	BG_RIGHT_Z_CUR_POS
// 	BG_RIGHT_Z_CUR_TOR
// 	ALGN_TOP_LEFT_Y_SET_SPD
// 	ALGN_TOP_LEFT_Y_SET_POS
// 	ALGN_TOP_LEFT_Y_CUR_SPD
// 	ALGN_TOP_LEFT_Y_CUR_POS
// 	ALGN_TOP_LEFT_Y_CUR_TOR
// 	ALGN_TOP_RIGHT_Y_SET_SPD
// 	ALGN_TOP_RIGHT_Y_SET_POS
// 	ALGN_TOP_RIGHT_Y_CUR_SPD
// 	ALGN_TOP_RIGHT_Y_CUR_POS
// 	ALGN_TOP_RIGHT_Y_CUR_TOR
// 	ALGN_BTM_LEFT_Y_SET_SPD
// 	ALGN_BTM_LEFT_Y_SET_POS
// 	ALGN_BTM_LEFT_Y_CUR_SPD
// 	ALGN_BTM_LEFT_Y_CUR_POS
// 	ALGN_BTM_LEFT_Y_CUR_TOR
// 	ALGN_BTM_RIGHT_Y_SET_SPD
// 	ALGN_BTM_RIGHT_Y_SET_POS
// 	ALGN_BTM_RIGHT_Y_CUR_SPD
// 	ALGN_BTM_RIGHT_Y_CUR_POS
// 	ALGN_BTM_RIGHT_Y_CUR_TOR
// 	PB_Z_SET_SPD
// 	PB_Z_SET_POS
// 	PB_Z_CUR_SPD
// 	PB_Z_CUR_POS
// 	PB_Z_CUR_TOR
// 	PC_CUR_TEMP
// 	CB_CUR_TEMP
// 	CB_CUR_02
// 	CB_CUR_H20
// 	CB_CUR_PRE
// 	N01_T_PW
// 	N01_I_PW
// 	N01_R_VOL
// 	N01_T_VOL
// 	N01_R_CUR
// 	N01_T_CUR
// 	N02_T_PW
// 	N02_I_PW
// 	N02_R_VOL
// 	N02_T_VOL
// 	N02_R_CUR
// 	N02_T_CUR

	MCC_ILOG_COL_END,
};

#define MAX_QUEUE_COUNT 10000
#define STR_MCC_NAME	_T("MCCLog")

//class MyQueue
//{
//private:
//	int m_front;
//	int m_rear;
//	MCC_EQDATA m_queue_data[MAX_QUEUE_COUNT];
//public:
//	MyQueue();
//	int AddQueue(MCC_EQDATA stData);
//	int DeleteQueue(PMCC_EQDATA pstData);
//	bool IsFull();
//	bool IsEmpty();
//};

class CDlgnMCCReport : public CDialog
{
// Construction
public:
	BOOL WaitTerminateWorkerThread();
	BOOL m_bInitSendToFtp;
	int m_iOldTime;
	void KillThred();
	void SetConfigPath(CString str);
	void SetLogPath(CString str);
	void SetFilePath(CString str);
	void SetEQName(CString str);
	void SetSendToFtp();
	BOOL SendToFtpServer();
	int m_nProcessJobNo;
	BOOL GoToWork(LPVOID pParam);
	void BeginThread_WorkProc();

	CString GetErrortoStr(int iError);
	CString GetSignaltoStr(int iSignal);
	CString GetActiontoStr(int iAction);
	CString GetFromto(int iAction);
	void WriteMCCWarring(CString strTime,int iError, BOOL bSet);
	void WriteMCCError(CString strTime,int iError, BOOL bSet);
	void WriteMCCSignal(CString strTime, int iSignal, BOOL bStart, int nFromto = 0);
	void WriteMCCAction(CString strTime, int iAction, BOOL bStart, int nStep = 0, int nFromto = 0);
	void WriteMCCInfo(int iRow);
	void WriteMCCEvent(int iEvent, BOOL bStart);
	static UINT WorkThreadProbeProc(LPVOID pParam);

	HBRUSH 			  m_hbrushBK;
	CWinThread*       m_pWorkThread;
//	MyQueue			  m_queue;

	// MCC 공유 메모리
	HANDLE		    m_hMCCMapFile;
	VTM_MCC*	    m_pMCCMap;
	bool CreateMemoryMap();
	void Clear();
	bool IsFull();
	bool IsEmpty();
	int AddQueue(MCC_EQDATA_T stData);
	int DeleteQueue(PMCC_EQDATA_T pstData);

	// 별도의 MCCLog.exe의 핸들값 저장
	HWND m_hWndMccLog;
	HWND	m_hTarget;

	//Theread 관련 이벤트 핸들
	HANDLE		m_hEvtWorkStart;        //set by UI		쓰레드 시작 핸들
    HANDLE		m_hEvtWorkDone;         //set by self	작업완료 핸들(쓰레드 종료 아님)
    HANDLE		m_hEvtKillThread;       //set by UI		쓰레드를 강제로 죽이는 핸들
    HANDLE		m_hEvtThreadKilled;     //set by self	쓰레드가 죽었다는 핸들

	CString m_strModlueID;
	CString m_strEQPID;
	CString m_strNewLogFile;
	CString m_strSendLogFile;
	CString m_strLogFile;
	CString m_strTLogFile;
	CString m_strILogFile;
	CString m_strindexFile;

	CString m_strLogPath;
	CString m_strConfigPath;
	CString m_strFilePath;
	CString m_strEQName;
	CString GetCnfPath(int nEQType, CString sPath);
	void AddString(CString &sLog, int iList = 0);
	int indexLOG();
	int ILOG(CString s);
	int TLOG(CString s);
	int WriteFile(CFile *pFile, CString &sPath, CString sLog);
	CDlgnMCCReport(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDlgnMCCReport)
	enum { IDD = IDD_DLG_MMC_LOG };
	CListCtrl		m_list_TLOG;
	CListCtrl		m_list_ILOG;
	//CSpreadSheet	m_sprdMccILog;
	CMySpreadDll*	 m_sprdMccILog;
//	CStaticSpread	 m_stSpreadILog;
//	CListCtrl	m_list_ILOG;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDlgnMCCReport)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CFile			m_fileTLog;
	CFile			m_fileILog;
	CFile			m_fileindexLog;
//	VTM_SYSTEM*		m_pSystem;


	// Generated message map functions
	//{{AFX_MSG(CDlgnMCCReport)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	int SetMccILogHeader(void);
	CString m_strAppPath;
	int UpdateMCCInfo(int iRow);
	int m_nDeleteRowNum;
	int m_oldEvtFlag;
	int UpdateMCCTLog(void);
	int AddMccQueue(int iFlag);
	int UpdateMCCILOG(void);
	CButton m_chkUseSend;
	afx_msg void OnBnClickedCheckT();
	BOOL FindTargetWindow(CString sWndName);
	bool m_bStartMCCExe;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGNMCCREPORT_H__087B2A06_90CD_4554_A027_BDF3D248B089__INCLUDED_)
