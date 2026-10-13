#pragma once

#include "MainSubDlgWin.h"
#include "WindowEventHandler.h"
#include "common\MyLabelTimer.h"
#include "common\LedButton.h"
#include "UIMeasPathDlg.h"

// LYS 2026-07-20 CMFCButton with vertical-center multiline text
class CVCenterMFCButton : public CMFCButton
{
protected:
	virtual void OnDrawText(CDC* pDC, const CRect& rect, const CString& strText,
		UINT uiDTFlags, UINT uiState) override
	{
		CRect rcCalc = rect;
		pDC->DrawText(strText, &rcCalc, (uiDTFlags | DT_CALCRECT) & ~DT_END_ELLIPSIS);
		CRect rcDraw = rect;
		int nOff = (rect.Height() - rcCalc.Height()) / 2;
		if (nOff > 0) rcDraw.top += nOff;
		pDC->DrawText(strText, &rcDraw, uiDTFlags);
	}
};

#define df_MAX_USE_LAMP				2  	// MAX LAMP - LYS 2026-09-25 4 -> 2 : Align CCD1/CCD2 (SPO ch 3/2) 만 표시. ch 0/1 은 OUT_OF_SERVICE 프로브 교체 카메라용
//#define df_MAX_USE_LAMP				4  	// MAX LAMP
#define MAIN_STATE_TIMER_ID		1001
#define MAIN_LAMP_OVER_TIMER_ID		1011
#define PROBE_CONTACT_TIMER_ID		1021	// LYS 2026-07-01 Probe Contact blink timer

enum BUTTON_IMAGE
{
	BUTTON_IMAGE_CYCLE = 0,
	BUTTON_IMAGE_RESUME,
	BUTTON_IMAGE_PAUSE,
	BUTTON_IMAGE_STOP,
	BUTTON_IMAGE_COUNT,
};

// CDlgSubOperate 대화 상자입니다.
class CAuroraMgr;
class CSeqTactTimeMgr;
class CD_MainOperateInitializing;
class CDlgSubMeasure_Progress;
class CIniHelper;
class CDlgSubOperate : public CMainSubDlgWin
{
	DECLARE_DYNAMIC(CDlgSubOperate)

private:
	CIniHelper* m_pIniHelper;
	CAuroraMgr					*m_pAuroraMgr;
	CMyLabelTimer				m_LmapTimer[df_MAX_USE_LAMP];
	CXButtonXP					m_btnImage[BUTTON_IMAGE_COUNT];
	CD_MainOperateInitializing	*m_pSystemInitializingDlg;
	CDlgSubMeasure_Progress* m_pAgingProgressDlg;
	CSeqTactTimeMgr				*m_pSeqTactTimeMgr;

	CGridCtrl   m_gridListTactTimeMeas;
	CGridCtrl   m_gridListTactTimeGlass;
	CGridCtrl   m_gridListMeasInfo;
	CGridCtrl   m_gridListCellInfo;
	CGridCtrl   m_gridListTempInfo;

	CLabel      m_LabelInfo[7];

	CTime		m_timeStart;
	CTimeSpan	m_spanCur, m_spanStart, m_spanWarring;

	CUIMeasPathDlg* m_pParentMeasPathDlg;
	CUIMeasPathDlg* m_pParentCellMeasDlg;

	CLedButton m_ledFixtureL[2];	// Left
	CLedButton m_ledFixtureF[2];	// Front
	CLedButton m_ledFixtureB[2];	// Back
	CLedButton m_ledFixtureR[2];	// Right

	CLedButton m_ledFixtureLUp[2];	// Left
	CLedButton m_ledFixtureFUp[2];	// Front
	CLedButton m_ledFixtureBUp[2];	// Back
	CLedButton m_ledFixtureRUp[2];	// Right

	CLedButton m_ledShtterUp;	
	CLedButton m_ledShtterMoving;

	int		   m_iState;
	static UINT ThreadRun(LPVOID pParam);

	CArray <CELLINFO, CELLINFO&>   m_aryCellGroup;

	// LYS 2026-09-30 Hybrid Phase 6 operate cell map: every segment of a hybrid recipe on one glass.
	//   Built by GenGlassPath (recipe open / glass start, segment 0 active). The 1 s timer reads only this cache,
	//   nCellStatus and the int active segment - never the segment vectors (A8). m_nHybridMapSegCnt 0 = 1-segment path.
	struct S_HybridMapCell
	{
		TCHAR	szName[32];			// recipe cell ID
		double	dSX, dSY, dEX, dEY;	// chart frame: x = 1500 side, y = 925 side (drawn downward), origin = glass centre
		int		nCell, nStick;		// nCellStatus[nCell][nStick], nStick = segment stick base + MakeCellID stick
		int		nSeg;
	};
	CArray <S_HybridMapCell, S_HybridMapCell&>	m_aryHybridMapCell;
	CArray <double, double>	m_aryHybridSegBox;	// 4 per segment: min x, min y, max x, max y of its cells
	int		m_nHybridMapSegCnt;		// segments in the cache, 0 = 1-segment recipe
	int		m_nHybridMapFrameSeg;	// segment framed in the last drawing, -1 none, -2 force a redraw
	CArray <int, int>	m_aryHybridSegRot;	// LYS 2026-10-12 Hybrid Phase 6 GLASS ROTATE (0..3) of every segment, built with the cell map (UI thread)
	CStringArray		m_aryHybridSegName;	// LYS 2026-10-12 Hybrid Phase 6 segment names for the indicator (built with the cell map)
	int		m_nHybridPathSeg;		// LYS 2026-10-12 Hybrid Phase 6 segment the PATH outline shows, -1 = not hybrid
	int		m_nHybridPathRot;		// LYS 2026-10-12 Hybrid Phase 6 rotate the PATH outline was drawn with, -1 = not hybrid
	int		m_nCellDrawLegacy;		// LYS 2026-10-12 Hybrid Phase 6 configured CELL_MAX: the 1-segment cell draw limit (D6-6)
	CStringArray		m_aryManualCellName;	// LYS 2026-10-12 Hybrid Phase 6 1-segment manual Cell ID names [cell * 50 + status column], empty = Auto/Auto
	CString		m_strHybridInfo;		// LYS 2026-10-12 Hybrid Phase 6 indicator text on screen ("" = hidden)

	long m_lMeasDoneColorWAD;
	long m_lMeasDoneColorVision;
	long m_lMeasDoneColorAngle;	
	long m_lMeasDoneColorColor;

public:
	CDlgSubOperate(CAuroraMgr *pAuroraMgr, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgSubOperate();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_SUB_OPERATE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	void UpdateInitialState(BOOL bEnable);
	void SetLampUse(int iLamp, bool bUse);
	int  LampChannelToSlot(int nChannel);	// LYS 2026-09-25 SPO 채널 -> 타이머 슬롯(없으면 -1)
	void __InitListControl();	// Tact Time
	void __InitialListData();
	void __RefreshListData();
	void __InitListControl2();	// Glass Tact Time
	void __InitialListData2();
	void __RefreshListData2();
	void __InitListControl3();	// Probe Info
	void __InitialListData3();
	void __RefreshListData3();
	int  __MeasInfoRowCount();	// LYS 2026-09-25 PROBE INFO 그리드 행 수 (OCP 4 / HMI 5)
	void __InitListControl4();	// Cell Info
	void __InitialListData4();
	void __RefreshListData4();
	void __InitListControl5();	// Temp Info
	void __InitialListData5();
	void __RefreshListData5();

	afx_msg void OnBnClickedButtonInitial();
	afx_msg void OnBnClickedButtonMoveSafety();
	afx_msg void OnBnClickedButtonChangeLoad();
	afx_msg void OnBnClickedButtonChangeMeas();
	afx_msg void OnBnClickedButtonCycle();
	afx_msg void OnBnClickedButtonResume();
	afx_msg void OnBnClickedButtonPause();
	afx_msg void OnBnClickedButtonStop();
	void UpdateCycleInspectionStart();
	void UpdateCycleInspectionComplete();
	void UpdateChangeInitialize();
	bool MoveCommandProcess(CString strType, bool bCompleteCheck = false);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void UpdateInformation();
	void UpdateDataLampTime(int iLamp, bool bOn);
	void UpdateWriteLampTime(int iLamp, bool bOn);
	void UpdateOperateStatus(CString str);
	virtual void UpdateSeqItemElapsedTime() override;
	virtual void UpdateMeasureElapsedTime(bool bStart) override;
	long m_nMeasCount;
	void UpdateMeasCell(RECIPELAYOUT* pRecipe);
	void RotateMeasPath(int nAngle = 0);
	void RotateGlassPath(int nAngle = 0);
	double* m_pdGlassSize;
	double* m_pdPathSize;
	double* m_pdCardSize;
	int* m_pnGlassEdgeCut;
	BOOL m_bGraph_Reverse_X;
	BOOL m_bGraph_Reverse_Y;
	BOOL m_bGraph_InvertXY;
	void GetConfig(bool bReload = false);
	void WriteColorConfig();
	int m_nOldAngle;
	bool LinkIniFile(bool bSave = false);
	void UpdateFixtureInOut();
	void GenGlassPath();
	void UpdateCellStatus();
	bool BuildHybridCellMap();								// LYS 2026-09-30 Hybrid Phase 6
	void UpdateHybridCellStatus();							// LYS 2026-09-30 Hybrid Phase 6
	void SetHybridCellLook(int iStatus, CELLLAYOUT& stCell);	// LYS 2026-09-30 Hybrid Phase 6
	void UpdateHybridIndicator();							// LYS 2026-10-12 Hybrid Phase 6
	void SyncHybridPathView(int nPass, int nSeg, int nReady);	// LYS 2026-10-12 Hybrid Phase 6
	void BuildManualCellNameCache();						// LYS 2026-10-12 Hybrid Phase 6
	afx_msg void OnBnClickedButton1();
	void UpdateCurCoord();
	afx_msg void OnBnClickedButtonChangeControlmode();
	void UpdateCIMState(int iState);
	void UpdateControlMode(int iState);
	void RunStart();
	bool CheckAndApplyResumeMode();	// LYS 2026-07-07 Resume check for both manual and CIM paths

	void EnableControlMode(BOOL bEnable);
	void SetFunctionButtonStatus(BOOL bEnable);
	void UpdateRecipeInformation();
	void UpdateAgingMode(bool bStart, int nTime);
	void UpdateSACCMode(bool bStart);
	afx_msg void OnBnClickedButtonSafeytPlc();
	afx_msg void OnBnClickedButtonSMarkInfo();
	afx_msg void OnBnClickedCheckCimReport();  // LYS 2026-07-20 CIM Alarm not-report toggle
	CVCenterMFCButton m_btnCimAlarmNotReport;  // LYS 2026-07-20 CIM Alarm not-report toggle button
	void UpdateCimReportButtonColor(bool bChecked);  // LYS 2026-07-20
	void UpdateMeasColor();
	bool		m_bThreadRun;
	CLabel		m_labelProbeContact;	// LYS 2026-07-01 Probe Contact blinking indicator
	CLabel		m_labelHybridInfo;	// LYS 2026-10-12 Hybrid Phase 6 segment / pass / resume indicator (cell chart top band)
	bool		m_bProbeContactFlashing;	// LYS 2026-07-01 Flash state tracking
	static UINT ThreadInspectEnd(LPVOID p1);
	UINT ThreadRun();
	void UpdateCIMAlarm(int iAlarm);
	afx_msg void OnDestroy();
};
