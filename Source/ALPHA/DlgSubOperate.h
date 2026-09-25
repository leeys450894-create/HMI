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
	bool		m_bProbeContactFlashing;	// LYS 2026-07-01 Flash state tracking
	static UINT ThreadInspectEnd(LPVOID p1);
	UINT ThreadRun();
	void UpdateCIMAlarm(int iAlarm);
	afx_msg void OnDestroy();
};
