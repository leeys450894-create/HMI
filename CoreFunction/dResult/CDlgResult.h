#pragma once
#include "pch.h"
#include "resource.h"
#include "..\common\MySpreadDll.h"
#include "..\common\CommonFunc.h"
#include "API_Result.h"
#include "..\CoreFunction\MotorFunction\AxisType.h"
#include <vector>

// CDlgResult 대화 상자
#define		WM_MSG_DISPLAY_RESULT		(WM_USER + 0x100 + 1501)
#define RITEM_BUFFER_MAX        20   // 4개의 버퍼를 사용.
#define df_RESULT_ORG 0

class CDlgResult : public CDialog
{
	DECLARE_DYNAMIC(CDlgResult)

private:
	HFONT m_hfont;
	CMySpreadDll* m_sprdResult;
	CMySpreadDll* m_sprdTempResult;
public:
	CDlgResult(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgResult();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DLG_RESULT };

protected:
	///////////////////////////////////////////////////////
	// 초기 변수
	/////////////////////////////////////////////////////// 
	CString m_strConfigPath;
	CString m_strLogPath;
	CString m_strResultPath;
	CString m_strRootPath;
	CString m_strCimData;
	BOOL	m_bLoadNoMeas;



	///////////////////////////////////////////////////////
	// 결과 데이터 Thread
	///////////////////////////////////////////////////////
	CWinThread*			m_pRMonitorThread;
	HANDLE				m_hEventStop;
	HANDLE				m_hEventResume;
	BOOL				m_bStop;

	bool				m_bInit;
	RSRITEMDATA			m_RData;
	int*				nBuffer;
	HANDLE				hMem_nBuffer;

	TCHAR				sErrMsg[1024];
	RSRITEMDATA*		pBuffer[RITEM_BUFFER_MAX];

	static UINT			CBResultMonitorProc(LPVOID pParam);
	void				CBResultMonitorBody();
	void				SetResultMonitoring(BOOL bSet);

	bool Upload(PRSRITEMDATA pArea);  // 모듈에서 Area결과를 버퍼로 업로드한다.
	bool Download(PRSRITEMDATA pArea, bool bJustMonitoring = false, int iIdx = 0); //Area 결과를 버퍼에서 다운로드한다.
	int UploadData(PRSRITEMDATA result); //다운로드한 결과를 Spread에 뿌림
	bool InitShareMemory();
	

	int	 GetNBuffer();	// 현재 몇개의 버퍼가 사용되었는지를 return 한다.
	TCHAR* GetErrMsg();	// 발생한 에러중 마지막 것을 받아온다.
	bool ClearAll();	// 버퍼를 비운다.


	///////////////////////////////////////////////////////
	// 결과 데이터 디스플레이 용
	///////////////////////////////////////////////////////
	bool UploadDataDone;

	// 입력값으로 받을 내용
	int m_nMeasureGlassCnt;		// 당일 측정한 레시피 개수
	int m_iTValue;				// Recipe Param t-VALUE 설정 시
	int m_nMeasMode;			// 측정모드
	// LYS 2026-07-24 WAD Multi Point (Phase 6)
	int m_nMultiPoint;
	double m_dMultiOffsetRX;
	double m_dMultiOffsetGX;
	double m_dMultiOffsetBY;
	int m_nMeasType;			// Recipe Param CS 측정 타입
	int m_nMeasAreaCnt;			// 측정 Area 총 개수
	int m_nMaxMeasureRcpFlag;	// 측정한 Recipe 개수, 멀티레시피 최대 10개
	TCHAR m_szGlassID[32];		// 현재 측정 GlassID
	TCHAR m_szLotID[32];		// 현재 측정 LotID
	TCHAR m_szRecipeID[32];		// 현재 측정 RecipeID
	bool m_bDisplayResulted;	
	CString m_stMeasDay;
	CString	m_strAPDBackUpPath;
	CString m_strStartTime;
	int m_nCellCountX;
	int m_nCellCountY;

	static UINT WorkThreadExportDataProc(LPVOID pParam);

	MEASRESULT		m_stReceiveMeasResult[df_MAX_PROBE];	// 측정결과 
	std::vector<RESULTSTRUCT> m_stResult;

	std::vector<std::vector<CString>>	m_vecVisDefecs;		//20171226 add by Hee (화질 데이터 저장 후 APD쓸때 같이 씀)
	std::vector<CString>				m_vecVisDefec;
	std::vector<std::vector<CString>>	m_vecMicroDefecs;	//20180507 add by Hee (화질 데이터 저장 후 APD쓸때 같이 씀)
	std::vector<CString>				m_vecMicroDefec;

	int ReadResultData(CString strPath, CString strFileName, CString strImageFile, int nProbe, PMEASRESULT strMeasResult);
	
public:
	CString strApdData3;
	CString strApdData2;
	CString strApdData1;
	double	m_dICPSTAT[10][100];	// jsyoon 2022.05.27 change 50 to 100

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();	
	int ResultCalculateICP(void);
	bool SaveResult(bool bSaveFile, bool bRemote, CString strStart);
	void ExportAsAPDHead(CArchive& ar);
	int ExportAsRawDateICP(void);
	int ExportAsTotalData(bool bAuto, bool bSaveFile, bool bRemote, CString strStart);
	int ExportAsAPDICP(CString strResultPath, CString strTime);
	void ExportToExcel(CString strFile);
	void ExportAsMarge(CString strPath);
	int TotalResultData(void);
	void SetSearchMode(int iMode);
	CString GetMeasMode();
	CString GetSpredtoStr2(int iRow);
	void SetConfigPath(CString str);
	void GetConfig();
	bool UploadDataToShare(PRSRITEMDATA result);
	void DataClear();
	bool m_bSaveResultDone;
	// LYS 2026-10-06 Hybrid Phase 5 resume replay: fast upload switch + finished-row counter (public like m_bSaveResultDone, used by CResultManager)
	volatile bool m_bFastUpload;					// true only during a resume journal replay: UploadData skips the 1 s display wait
	volatile LONG m_nUploadDoneCount;				// rows the monitor thread finished (UploadData returned), never reset
	void GetUploadState(int* pQueue, long* pDone);	// rows waiting in the shared ring / rows finished (counter above)
};
