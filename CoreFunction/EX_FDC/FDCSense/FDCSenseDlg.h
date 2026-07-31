
// FDCSenseDlg.h : 헤더 파일
//

#pragma once

#include "FPSpreadsheet/spreadsheet.h"
#include "..\\Include\API_MXComponent.h"
//#include "..\\Include\API_MelsoftControl.h"


#include "DlgPLC.h"
#include "DlgFDCMainView.h"
#include "DlgPLCDRead.h"
#include "DlgMonitoring.h"	
#include "afxwin.h"
#include "..\\Include\FDCSheardDefine.h"
#include "..\\Include\FDCDefine.h"

#include "ExcelAPI/XLEzAutomation.h"

#define dfDATASTARTROW						2

#define WM_TRAYICON_MSG WM_USER + 1 

#define dfMAXMONITER						10
#define df_MAX_ITEM							300

// CFDCAppDlg 대화 상자입니다.
const enum EQUIP_TYPE_VALUE{
	EQU_TYPE_ICE		= 0,
	EQU_TYPE_ICM,
	EQU_TYPE_ICP,
	EQU_TYPE_OCP,
	EQU_TYPE_EVEN,
	EQU_TYPE_BP,
	EQU_TYPE_8K,
	EQU_TYPE_TFE,
	EQU_TYPE_SRM,
	EQU_TYPE_TM,
	EQU_TYPE_END
};

typedef struct // DLL Input데이터
{
	short	sDevice;				// 	Destination
	char	cPARARMName[1024];
	double	dFilterMin;					
	double	dFilterMax;
	double	dDefaultVal;
	int		nSoureType;
	char    cSourceAddress[64];
	char	cDestination[10];
	int		ShareMem_num;
	double  dVal;	
	BOOL	bUse; 
	int		uiMagnificaion;			//sypark 20180529 add	// jsyoon 20190125 change
	UINT	uSVType;
}FDCSPRED, *PFDCSPRED;

enum source_type_t {
	SOURCE_TYPE_DEFAULT = -1,
	SOURCE_TYPE_PLC = 1,
	SOURCE_TYPE_SENSOR,
	SOURCE_TYPE_SHAHREMEMORY,
	SOURCE_TYPE_CAPA,
	SOURCE_TYPE_SELF,
	SOURCE_TYPE_REG,	// jsyoon 20210129 add
	SOURCE_TYPE_END
};

//FDC excel list col 
enum FDCLIst_col_type_t{
	COL_SVID = 1,
	COL_PARARM_NAME,
	COL_FILTER_USE,
	COL_FILTER_MIN,
	COL_FILTER_MAX,
	COL_FILTER_DEFAULT,
	COL_USE,
	COL_SOURCE_TYPE,
	COL_SOURCE_ADDRESS,
	COL_DESTINATIO,
	COL_SHAREMEM_NUM,
	COL_VALUE_MAGNIFICATION,	//sypark 20180529 add
	COL_SV_TYPE,				// jsyoon 20181220 add
	COL_END
};

// CFDCSenseDlg 대화 상자
class CFDCSenseDlg : public CDialogEx
{
// 생성입니다.
public:
	CFDCSenseDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
	enum { IDD = IDD_FDCSENSE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.
	virtual BOOL PreTranslateMessage(MSG* pMsg);

// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	CSpreadSheet m_sprdFDCResult;
	CSpreadSheet m_sprdFDCList;

public:

	BOOL			m_bUseTemp[dfMAXNUM_PCTEMP];	//sypark 20180515 add

	BOOL			m_bFilteringUse;
	double			dVal[dfMAXFDCITEM];

 	int				nIndexNum_CAPA[10];
	int				nIndexNum_Share[dfMAXFDCITEM];
	int				nIndexNum_Reg[10];

	// sih 20160726 add 
	BOOL m_bLedUse;
	UINT m_nPort;
	UINT m_uiLedPort;
	UINT m_uiLedBaudRate;
	UINT m_uiLedParity;
	UINT m_uiLedDataBits;
	UINT m_uiLedStopBit;
	CWinThread*	 m_pGetStateThread;
	HANDLE m_hEvtGetStateStop;
	double m_dO2ppm;
	double m_dO2ppm2;			//sypark 20180502 add
	double m_dOldO2ppm;
	//

	//jhseo 20170627 add for LED Display 2
	BOOL m_bLedUse2;
	UINT m_nPort2;
	UINT m_uiLedPort2;
	UINT m_uiLedBaudRate2;
	UINT m_uiLedParity2;
	UINT m_uiLedDataBits2;
	UINT m_uiLedStopBit2;
	//////////////////////////////////////////////////////////////////////////

	// eh.kim 20170419 add.
	double m_dO2Min;
	double m_dO2Max;
	//

	//FDC_list excel 파일 내각 Type 갯수
	int m_nPLCCnt;
	int m_nSENSORCnt;
	int m_nSHAREDCnt;
	int m_nSELFCnt;
	int m_nCAPACnt;
	int m_nRegCnt;
	int m_nSharedMemoryCnt;
	int m_nFDCListCnt;
	//

	int m_pCAPA[10];
	int m_pShared[100];
	int m_pSelf[10];
	int m_pLampTime[10];

	double m_dSENSOR[serial_End];

	// 설비 종류 
	int				m_nEQTYPE;
	double			m_dMoniter[100];
	BOOL			m_bUseDisplayPPM;
	BOOL			m_bTrayIcon;
	FFU				m_ffu;	
	HDD				m_hdd;
//	CString			strFDCListPath;
	CString			m_strRootPath;
	CString			m_strEQName;
	char			m_chEQName[30];		
	FDCSPRED		m_spredFDC[dfMAXFDCITEM];	
	FDCSPRED		m_spredFDCOld[dfMAXFDCITEM];	
	double			m_dBfFDCValue[dfMAXFDCITEM];

	CDlgPLCDRead	m_dlgPLCRead;
	CDlgFDCMainView m_dlgFDCView;	
	CDlgPLC		    m_dlgPLC;
	BOOL			m_bShowLog;
	CRect			m_rectMaindlg;

	int	m_nUseMonitorNum;
	//jhseo 20171107 add for monitoring
	CDlgMonitoring*	m_dlgMonitoring[dfMAXMONITER];
	//CDlgMonitoring		m_dlgMonitoring[10];

// 	//sypark 20080502 add
// 	CDlgMonitoring*	m_dlgMonitoring2;
// 	CDlgMonitoring*	m_dlgMonitoring3;
// 	CDlgMonitoring*	m_dlgMonitoring4;
// 	//

	BOOL			m_bUseMonitoring;
	//int				m_nUiData[2];		//sypark 20180608 remove


	int				m_nUiData1[2];	
	int				m_nUiData2[2];	
	int				m_nUiData3[2];	
	int				m_nUiData4[2];	
	int				m_nUiData5[2];	
	int				m_nUiData6[2];	

	int				m_nUiMonitorData[dfMAXMONITER][3];	//sypark 20180608 add


	afx_msg void	OnClickedBtnShowLog();
	afx_msg void	OnBnClickedBtnShowMonitoring();
	afx_msg void	OnBnClickedButtonVersion();


	int				CreateMemoryMap();
	int				gf_CloseMemoryMap();
	void			SendToLed(int iPort, int i, double dO2);
	void			GetState();
	static UINT		WorkThreadProcGetState(LPVOID pParam);
	void			StartThreadGetState();
	void			OpenPort(UINT nPort, DWORD dwRate, BYTE bDataBits, BYTE bParity, BYTE bStopBits);	
	BOOL			SHRegDeleteValue(HKEY hKey, LPCTSTR lpKey, LPCTSTR lpValue);
	BOOL			SHRegWriteString(HKEY hKey, LPCTSTR lpKey, LPCTSTR lpValue, LPCTSTR lpData);
	BOOL			SHRegReadString(HKEY hKey, LPCTSTR lpKey, LPCTSTR lpValue, LPCTSTR lpDefault, LPTSTR lpRet, DWORD nSize);
	LRESULT			TrayIconMSG(WPARAM wParam, LPARAM lParam);
	LRESULT			OnTrayReset(WPARAM wParam, LPARAM lParam);	//	yj.k 20180201 add explorer shotdown -> restart -> TrayIcon Retry show
	void			ResTrayIcon();
	int				SetMStateCodeToSpread();	
	void			UpdateSpreadHeader();
	void			UpdateSpreadData();
	void			SetShowLog(BOOL bShow);
	void			SetSpredData(int iRow,FDCSPRED SpredData);
	CString			GetVersionInfo ();					
	void			UpdateVersionInfo ();
	CButton			m_btnVersion;
	int				UploadFDCListToExcelFile(void);
	CString			m_strAppPath;
	void			SetAppPath(CString str) { m_strAppPath = str; };
	afx_msg void	OnBnClickedReload2();
	afx_msg void	OnDestroy();
	void			GetDiskFree();
	void			GetConfig();
	void			UpdateSpredDate();
	void			CloseDialog();
	void			WriteInfoLog(CString strMsg);
	void			WriteELog(CString strMsg, int iType = 0);
	int				WriteFile(CFile *pFile, CString &sPath, CString sLog);
	int				WriteFile2(CString sPath, CString sLog);	// LOG


	HANDLE			m_hEvtThreadReadKilled;     //set by self	쓰레드가 죽었다는 핸들
	HANDLE			m_hEvtThreadReadTerminated;
	HANDLE			m_hEvtThreadStart;	
	HANDLE			m_hEvtThreadKill2;	
	HANDLE			m_hEvtThreadKilled;			 //set by self	쓰레드가 죽었다는 핸들
	HANDLE			m_hEvtThreadTerminated;

	CWinThread*		m_pThreadEfu;
	CWinThread*		m_pThreadFDC;
	CWinThread*		m_pThreadFDCRead;

	static UINT ThreadFDCProc(LPVOID pParam);
	static UINT ThreadFDCReadProc(LPVOID pParam);
	static UINT ThreadEFUProc(LPVOID pParam);

	BOOL m_bUpdateLed;
	BOOL m_bPLCReadDone;

	afx_msg void OnTimer(UINT_PTR nIDEvent);

	//VTM_FDC*		m_pFDCMap;
	VTM_FOR_FDC*	m_pFDCMap;

	afx_msg void	OnMenuitemOpen();
	afx_msg void	OnMenuitemWindowstartautorun();
	afx_msg void	OnMenuitemExit();
	void CloseTerminal();

	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnBnClickedBtnExit();
	bool m_bWriteStart;
	bool m_bInitUpdate;
	ADDR_DATA m_stAccuraData[dfMAXNUM_ACCURA];
	CString m_strAutoRunName;
	int StartRecive(void);
	int StopRecive(void);
	int CreateFDCListFromSheet(void);
	CString GetFDCListValue(int iRow, int iCol);
	int SetFDCHeader(void);
	bool m_bSetUpdatedata;
	afx_msg void OnBnClickedBtnMini();
	int GetReadRegistry(void);
	CString m_strAPPName;
};
