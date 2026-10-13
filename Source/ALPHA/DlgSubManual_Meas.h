#pragma once

// LYS 2026-08-24 Current stick readout (worker thread -> UI thread).
#define WM_MSG_UPDATE_CUR_STICK		(WM_USER + 3521)
#include "afxwin.h"
#include "DlgSubManual_STEP.h"
#include "Recipe/RecipeData.h"

// CDlgNaviManualMeas 대화 상자입니다.

enum enumStepID
{
	enumStepID_RECIPE_OPEN = 0,

	// GLASS GET
	enumStepID_GLASS_GET_01,
	enumStepID_GLASS_GET_02,
	enumStepID_GLASS_GET_03,
	enumStepID_GLASS_GET_04,
	enumStepID_GLASS_GET_05,
	enumStepID_GLASS_GET_06,
	enumStepID_GLASS_GET_07,
	enumStepID_GLASS_GET_08,
	enumStepID_GLASS_GET_09,

	// GLASS PUT
	enumStepID_GLASS_PUT_01,
	enumStepID_GLASS_PUT_02,
	enumStepID_GLASS_PUT_03,
	enumStepID_GLASS_PUT_04,
	enumStepID_GLASS_PUT_05,
	enumStepID_GLASS_PUT_06,
	enumStepID_GLASS_PUT_07,
	enumStepID_GLASS_PUT_08,

	// PG COMMAND
	enumStepID_PG_CMD_01,  // PG Turn On
	enumStepID_PG_CMD_02,  // PG Pattern Next
	enumStepID_PG_CMD_03,  // PG Pattern Prev
	enumStepID_PG_CMD_04,  // PG Turn Off
	enumStepID_PG_CMD_05,  // PG Turn On -> Next R, G, B -> Trun Off



	// ALIGN
	enumStepID_GLASS_ALIGN,
	enumStepID_PROBE_ALIGN,

	// MOVE STICK
	enumStepID_MOVE_STICK,
	enumStepID_MOVE_STICK_PRE,
	enumStepID_MOVE_STICK_NEXT,

	// MOVE CELL
	enumStepID_MOVE_CELL,
	enumStepID_MOVE_CELL_NEXT,

	// MOVE PROBE
	enumStepID_MOVE_PROBE,

	// Probe
	enumStepID_PROBE_CONTACT,
	enumStepID_PROBE_UNCONTACT,
	enumStepID_PROBE_CONTACT_ALL,
	enumStepID_PROBE_UNCONTACT_ALL,

	// RRE TEST
	enumStepID_PRE_TEST_01,		// Pin to Pin
	enumStepID_PRE_TEST_02,		// OST
	enumStepID_PRE_TEST_03,		// R-TEST
	enumStepID_PRE_TEST_04,

	// AGING TEST
	enumStepID_AGING_TEST,
	enumStepID_AGING_TEST_STOP,


	// MEAS SET
	enumStepID_MEAS_SET_WAD,
	enumStepID_MEAS_SET_VISION,
	enumStepID_MEAS_SET_COLOR,
	enumStepID_MEAS_SET_ANGLE,
	enumStepID_MEAS_SET_TSP,
	enumStepID_MEAS_SET_LCR,



	// MEAS TEST
	enumStepID_RECIPE_OPEN_PG,
	enumStepID_RECIPE_OPEN_AGING,
	enumStepID_RECIPE_CHANGE_WAD,
	enumStepID_RECIPE_CHANGE_VISION,
	enumStepID_RECIPE_CHANGE_COLOR,
	enumStepID_RECIPE_CHANGE_ANGLE,
	enumStepID_RECIPE_CHANGE_TSP,
	enumStepID_RECIPE_CHANGE_LCR,
	enumStepID_MEAS_TEST_WAD,
	enumStepID_MEAS_TEST_VISION,
	enumStepID_MEAS_TEST_COLOR,
	enumStepID_MEAS_TEST_ANGLE,
	enumStepID_MEAS_TEST_TSP,
	enumStepID_MEAS_TEST_LCR,


	// MANUAL COMMAND
	enumStepID_MANUAL_SHUTTER,

	enumStepID_MANUAL_CMD_01,
	enumStepID_MANUAL_CMD_02,
	enumStepID_MANUAL_CMD_03,
	enumStepID_MANUAL_CMD_04,
	enumStepID_MANUAL_CMD_05,
	enumStepID_MANUAL_CMD_06,
	enumStepID_MANUAL_CMD_07,

	enumStepID_MANUAL_CMD_08,
	enumStepID_MANUAL_CMD_09,
	enumStepID_MANUAL_CMD_10,
	enumStepID_MANUAL_CMD_11,
	enumStepID_MANUAL_CMD_12,
	enumStepID_MANUAL_CMD_13,
	enumStepID_MANUAL_PLC_RESET,
	enumStepID_MANUAL_CMD_ON,
	enumStepID_MANUAL_CMD_OFF,
	enumStepID_MANUAL_WAD_RESET,
};

class CAuroraMgr;
class CDlgSubManual_Meas : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSubManual_Meas)

private:
	CAuroraMgr* m_pAuroraMgr;
	CLabel		m_LabelInfo;

public:
	CDlgSubManual_Meas(CAuroraMgr* pAuroraMgr, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgSubManual_Meas();

	// 대화 상자 데이터입니다.
	enum { IDD = IDD_DLG_SUB_MANUAL_MEAS };

	//VTM_SYSTEM* m_pSystem;	//To Use Shared Memory Pointer
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnManualMeasRecipeOpen();
	afx_msg void OnBnClickedBtnManualMeasLoadingPositionMove();
	afx_msg void OnBnClickedBtnManualMeasLiftUp();
	afx_msg void OnBnClickedBtnManualMeasVacuumMotion();
	afx_msg void OnBnClickedBtnManualMeasLiftDown();
	afx_msg void OnBnClickedBtnManualMeasClampIn();
	afx_msg void OnBnClickedBtnManualMeasMoveAlignPositionMove();
	afx_msg void OnBnClickedBtnManualMeasProbeAlign();
	afx_msg void OnBnClickedBtnManualMeasGlassAlign();
	afx_msg void OnBnClickedBtnManualMeasStickMove();
	afx_msg void OnBnClickedBtnManualMeasAging();
	afx_msg void OnBnClickedBtnManualMeasRtest();
	afx_msg void OnBnClickedBtnManualMeasOpstTest();
	afx_msg void OnBnClickedBtnManualMeasPtpTest();
	afx_msg void OnBnClickedBtnManualMeasNextStickMove();
	afx_msg void OnBnClickedBtnManualMeasPropertyInspection();
	afx_msg void OnBnClickedBtnManualMeasMeasurementCellMove1();
	afx_msg void OnBnClickedBtnManualMeasDefinitionInspection();
	afx_msg void OnBnClickedBtnManualMeasNextCellMove1();
	afx_msg void OnBnClickedBtnManualMeasMeasurementCellMove2();
	afx_msg void OnBnClickedBtnManualMeasAngleOfFieldInspection();
	afx_msg void OnBnClickedBtnManualMeasNextCellMove2();
	afx_msg void OnBnClickedBtnManualMeasDataUpload();
	afx_msg void OnBnClickedBtnManualMeasLoadingPositionMove2();
	afx_msg void OnBnClickedBtnManualMeasClampOut();
	afx_msg void OnBnClickedBtnManualMeasLiftUp2();
	afx_msg void OnBnClickedBtnManualMeasVacuumMotion2();
	afx_msg void OnBnClickedBtnManualMeasPreviousCellPositionMove();
	afx_msg void OnBnClickedBtnManualMeasNextCellPositionMove3();
	afx_msg void OnBnClickedBtnManualMeasContactMotion();

	void OnClickBtnManualMeasRecipeOpen();
	void OnClickBtnManualMeasLoadingPositionMove();
	void OnClickBtnManualMeasLiftUp();
	void OnClickBtnManualMeasVacuumMotion();
	void OnClickBtnManualMeasLiftDown();
	void OnClickBtnManualMeasClampIn();
	void OnClickBtnManualMeasMoveAlignPositionMove();
	void OnClickBtnManualMeasProbeAlign();
	void OnClickBtnManualMeasGlassAlign();
	void OnClickBtnManualMeasStickMove();
	void OnClickBtnManualMeasAging();
	void OnClickBtnManualMeasRTest();
	void OnClickBtnManualMeasOpStTest();
	void OnClickBtnManualMeasPtpTest();
	void OnClickBtnManualMeasNextStickMove();
	void OnClickBtnManualMeasPropertyInspection();
	void OnClickBtnManualMeasMeasurementCellMove1();
	void OnClickBtnManualMeasDefinitionInspection();
	void OnClickBtnManualMeasNextCellMove1();
	void OnClickBtnManualMeasMeasurementCellMove2();
	void OnClickBtnManualMeasMeasurementCellMove3();
	void OnClickBtnManualMeasAngleOfFieldInspection();
	void OnClickBtnManualMeasNextCellMove2();
	void OnClickBtnManualMeasDataUpload();
	void OnClickBtnManualMeasLoadingPositionMove2();
	void OnClickBtnManualMeasClampOut();
	void OnClickBtnManualMeasLiftUp2();
	void OnClickBtnManualMeasVacuumMotion2();
	void OnClickBtnManualMeasLiftDown2();
	void OnClickBtnManualMeasPreviousCellPositionMove();
	void OnClickBtnManualMeasNextCellPositionMove3();
	void OnClickBtnManualMeasContactMotion();
	void BtnEventLog(int iID);

	/////////////////////////////////////////// jh.kang 20160601 add //This member variable to used in Button Log of DlgNaviManualMeas
	CButton m_Btn_MM_Recipe_Open;
	CButton m_Btn_MM_Loading_Position_Move;
	CButton m_Btn_MM_Lift_Up;
	CButton m_Btn_MM_Vacuum_Motion;
	CButton m_Btn_MM_Lift_Down;
	CButton m_Btn_MM_Clamp_In;
	CButton m_Btn_MM_Move_Align_Position_Move;
	CButton m_Btn_MM_Probe_Align;
	CButton m_Btn_MM_Glass_Align;
	CButton m_Btn_MM_Stick_Move;
	CButton m_Btn_MM_Aging;
	CButton m_Btn_MM_Rtest;
	CButton m_Btn_MM_OpstTest;
	CButton m_Btn_MM_PtpTest;
	CButton m_Btn_MM_Next_Stick_Move;
	CButton m_Btn_MM_Property_Inspection;
	CButton m_Btn_MM_Measurement_Cell_Move1;
	CButton m_Btn_MM_Definition_Inspection;
	CButton m_Btn_MM_Next_Cell_Move1;
	CButton m_Btn_MM_Measurement_Cell_Move2;
	CButton m_Btn_MM_Angle_Of_Field_Inspection;
	CButton m_Btn_MM_Next_Cell_Move2;
	CButton m_Btn_MM_Data_Upload;
	CButton m_Btn_MM_Loading_Position_Move2;
	CButton m_Btn_MM_Clamp_Out;
	CButton m_Btn_MM_Lift_Up2;
	CButton m_Btn_MM_Vacuum_Motion2;
	CButton m_Btn_MM_Previous_Cell_Position_Move;
	CButton m_Btn_MM_Next_Cell_Position_Move3;
	CButton m_Btn_MM_Contact_Motion;
	CButton m_Btn_MM_Measurement_Cell_Move3;
	/////////////////////////////////////////// 
	afx_msg void OnBnClickedBtnManualMeasNextStickMove3();
	afx_msg void OnBnClickedBtnManualMeasPreStickMove();
	afx_msg void OnBnClickedBtnManualGlassRotate1();
	afx_msg void OnBnClickedBtnManualGlassRotate2();
	afx_msg void OnBnClickedBtnManualMeasNextCellMove3();
	afx_msg void OnBnClickedBtnManualMeasNextCellMove4();
	afx_msg void OnBnClickedBtnManualMeasChange();
	afx_msg void OnBnClickedBtnManualReadyChange();
	afx_msg void OnBnClickedBtnLpDown();
	afx_msg void OnBnClickedBtnLpUp();
	afx_msg void OnBnClickedBtnManualMeasVacuumMotion4();
	afx_msg void OnBnClickedBtnManualMeasClampIn3();
	afx_msg void OnBnClickedBtnManualMeasPgCmd01();
	afx_msg void OnBnClickedBtnManualMeasPgCmd2();
	afx_msg void OnBnClickedBtnManualMeasPgCmd3();
	afx_msg void OnBnClickedBtnManualMeasPgCmd4();
	afx_msg void OnBnClickedBtnManualMeasClampIn2();
	afx_msg void OnBnClickedBtnManualMeasAirOff();
	afx_msg void OnBnClickedBtnManualMeasAirOn();
	afx_msg void OnBnClickedBtnManualMeasClampOut2();
	afx_msg void OnBnClickedBtnManualMeasClampIn4();
	afx_msg void OnBnClickedBtnManualMeasClampDown();
	afx_msg void OnBnClickedBtnManualDefinePtrn();
	afx_msg void OnBnClickedBtnManualMeasContactMotion3();
	afx_msg void OnBnClickedBtnManualMeasAngleOfFieldInspection2();
	afx_msg void OnBnClickedBtnManualMeasAngleOfFieldInspection4();
	afx_msg void OnBnClickedBtnManualMeasAngleOfFieldInspection3();
	afx_msg void OnBnClickedBtnManualMeasClampIn5();
	afx_msg void OnBnClickedBtnManualMeasVacuumMotion5();
	afx_msg void OnBnClickedBtnManualMeasClampIn6();
	afx_msg void OnBnClickedBtnManualMeasVacuumMotion6();
	afx_msg void OnBnClickedBtnManualGlassAlign();
	afx_msg void OnBnClickedBtnManualMeasNextCellMove5();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedBtnManualShutter();
	afx_msg void OnBnClickedBtnManualMeasPgCmd5();
	afx_msg void OnBnClickedBtnManualMeasAging2();
	afx_msg void OnBnClickedBtnManualMeasMeasurementSetPg();
	afx_msg void OnBnClickedBtnManualMeasMeasurementSetPg2();
	afx_msg void OnBnClickedBtnManualMeasMeasurementSetPg3();
	afx_msg void OnBnClickedBtnManualMeasMeasurementSetPg4();
	afx_msg void OnBnClickedBtnManualMeasMeasurementSetPg5();
	afx_msg void OnBnClickedBtnManualMeasMeasurementCellMove7();
	afx_msg void OnBnClickedBtnManualMeasMeasurementCellMove8();
	afx_msg void OnBnClickedBtnManualMeasMeasurementCellMove9();
	afx_msg void OnBnClickedBtnManualMeasMeasurementCellMove6();
	afx_msg void OnBnClickedBtnManualMeasMeasurementCellMove4();
	afx_msg void OnBnClickedBtnManualMeasMeasurementCellMove5();
	///////////////////////////////////////////

	CWinThread* m_pThread;
	bool		m_bThreadLive;
	HANDLE		m_hStartEvent;
	HANDLE		m_hCloseEvent;
	HANDLE		m_hMultiEvent[2];
	UINT static ThreadRun(LPVOID pParam);
	UINT		ThreadFunc();
	UINT		ThreadFuncDone(LPARAM lp);

	vector <S_MeasPos>	m_vecMeasGroup; //MeasGroup 정보

	int  MakeCellID(int* pnCellNum, bool* pbMeasProbe, TCHAR* ppcCellID, int iRotate, int iStick, int nMaxStick);	// jsyoon 2022.09.29 add
	void GenMeasID(int* pnCellNum, bool* pbMeasProbe, TCHAR* ppcCellID);
	void GenCellID(int* pnCellNum, bool* pbMeasProbe, TCHAR* ppcCellID);
	void GenInnerID(int* pnCellNum, bool* pbMeasProbe);
	
	//Area
	void SetMeasureArea(int nMeasureArea);
	int GetMeasureArea();
	int m_nMeasureArea;
	TCHAR** m_pStInnerID;

//	CString strFileName;
	BOOL LoadRecipeDlg();
	int DoWorkManual();
	int m_iStep;
	int m_iMeasType;
	bool m_bOpenRcp;
	int UPdateTestBtn(BOOL bOn);
	int m_iNumStick;
	afx_msg void OnBnClickedBtnManualMeasMeasurementCellMove3();
	int m_iNumCell;
	int ManualMoveCell(int iMode, int iMove = 0);
	int ManualMoveStick(int iStick);

	// LYS 2026-10-13 Hybrid Phase 7 7a manual motion gate (p7x_plan (2-4) / (3-3) / (14)-B C F M, (15)). Each helper returns at once on a
	//   1-segment runtime recipe (no log, no popup). In-class initializers: the constructor starts the worker thread first (AfxBeginThread)
	enum { HYB_STEP_NONE = 0, HYB_STEP_S, HYB_STEP_P, HYB_STEP_C, HYB_STEP_T, HYB_STEP_G, HYB_STEP_PG };	// step classes of (14)-B
	volatile LONG	m_nWorkBusy = 0;		// (14)-C worker lock: 0 free / 1 worker (DoWorkManual, ThreadRun CAS) / 2 UI segment switch (7b)
	bool		m_bViewStamped = false;		// (2-5) view stamp: recipe ID / load serial / segment count / active segment the selection was built from
	CString		m_strViewRcp;
	int			m_nViewLoadSeq = -1;
	int			m_nViewSegCount = 0;
	int			m_nViewActive = -1;
	int			m_nCidValSeq = -1;			// ValidateManualCellId: load serial + Glass ID of the last OPER line (the latch itself is set on every pass)
	CString		m_strCidValGlass;
	CString		m_strPgModelSent;			// D7-20 PG model this screen sent with RECIPE_OPEN_PG (emptied on recipe open / resync)
	bool	IsHybridRuntime();
	int		HybridStepClass(int nStep);
	bool	IsMeasSetStep(int nStep);
	bool	IsPgStep(int nStep);
	int		PeekTargetStick(int nStep);
	bool	CalcStickPose(int iStick, double& dStageY, double& dStageR);	// same pose formula as ManualMoveStick - change both together
	bool	IsHybridViewStale();
	void	StampHybridView();
	void	ResyncHybridView();
	CString	HybridGateDetail(int nSeg, int nSegCnt, int iStick, bool bTarget, double dTgtY, double dTgtR);
	void	RefuseHybrid(const CString& strWarn, const CString& strPopup);
	bool	CheckHybridGate(int nStep);
	bool	ValidateManualCellId();
	void	AddSegRowBase(int* pnAreaNum);

	// LYS 2026-08-24 Current stick 표기. step switch가 워커 스레드에서 돌아
	// UI 스레드로 값을 post 한다.
	void	UpdateCurStickUI();
	afx_msg LRESULT OnUpdateCurStick(WPARAM wParam, LPARAM lParam);
	int MoveLoaderXYR(int iLoader);
	int MakeMeasGroup(bool bKeepInnerId = false);	// LYS 2026-10-13 manual meas = CycleTest positions (review fix #7): true = legacy restore after a shared move - keeps the inner IDs of this stick / load
	// LYS 2026-10-13 manual meas = CycleTest positions (manualpos/rca.md (5)(c) + (8)): on OCP the SPEC / QUALITY / ANGLE moves and their next cell build
	//   the groups and the move array with the auto Measure step builders (MeasBuildGroups / MeasBuildMovePos, DoWorkGlassMeasureStep.h). REVIEW / COLOR
	//   and HMI mode keep the legacy MakeMeasGroup / ManualMoveCell code. The stamp below says what m_vecMeasGroup holds (in-class initializers as 7a)
	int		m_nMeasGroupMode = -1;		// M_TYPE_* of the shared groups held / -1 legacy MakeMeasGroup groups / -2 shared build refused (groups empty)
	int		m_nMeasGroupStick = -1;	// stick the shared groups were built for
	int		m_nMeasGroupLoadSeq = -1;	// recipe load serial they were built from
	bool	UseSharedMeasPos(int iMode);
	bool	IsMeasGroupCurrent(int iMode);
	int		EnsureMeasGroup(int iMode);
	bool	CheckManualMeasPose(int iMode);
	int		ManualMoveCellShared(int iMode, int iMove);
	long	RetractHeadZBeforeStage(int iStick, LPCTSTR pszWhat = nullptr);	// LYS 2026-10-13 review #6: pszWhat names a non-stick step in the log
	long	RetractHeadZBeforeStep(LPCTSTR pszWhat);	// LYS 2026-10-13 review #6: OCP raise before a stage / probe moving step (HMI mode: no-op)
	// LYS 2026-10-13 manual meas = CycleTest positions (review fix #7): stick / recipe load serial the inner ID array (m_pStInnerID, one slot per cell) was made for
	int		m_nInnerIdStick = -1;
	int		m_nInnerIdLoadSeq = -1;
	void	SetInnerIdArea(int nNeed, bool bKeep);
	afx_msg void OnBnClickedBtnManualMeasContactMotion4();
	afx_msg void OnBnClickedBtnManualMeasContactMotion5();
	afx_msg void OnBnClickedBtnManualMeasProbeMove();
	afx_msg void OnBnClickedBtnManualMeasAging3();		// Stop
	afx_msg void OnBnClickedBtnManualMeasRecipeOpen2();
	afx_msg void OnBnClickedBtnManualMeasRecipeOpen3();
	afx_msg void OnBnClickedBtnManualMeasRecipeOpen4();
	afx_msg void OnBnClickedBtnManualMeasRecipeOpen5();
	afx_msg void OnBnClickedBtnManualMeasRecipeOpen6();
	afx_msg void OnBnClickedBtnManualMeasRecipeOpen7();
	afx_msg void OnBnClickedBtnManualMeasRecipeOpen8();
	afx_msg void OnBnClickedBtnManualPlcReset();
	afx_msg void OnBnClickedBtnManualPlcBitOn();
	afx_msg void OnBnClickedBtnManualPlcBItOff();
	afx_msg void OnBnClickedBtnManualPlcWadReset();
};
