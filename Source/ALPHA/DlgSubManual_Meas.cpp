// DlgNaviManualMeas.cpp : 구현 파일입니다.
//
#include "pch.h"
#include "ALPHA.h"
#include "DlgSubManual_Meas.h"
#include "afxdialogex.h"
#include "AuroraMgr.h"
#include "Manager/CommTcpIpFactoryMgr.h"
#include "Recipe/RecipeEditMgr.h"
#include "Manager/SequenceManager.h"
#include "Manager/PositionConfigMgr.h"
#include "FunctionModule/FunctionModuleEQState.h"
#include "Manager/MaintControlMgr.h"
#include "DoWork/DoWorkGlassSequence.h"
#include "DoWork/DoWorkGlassMeasureStep.h"	// LYS 2026-09-14 MakeHeadPairByOrder
#include "FunctionModule/FunctionModuleEQStateChange.h"
#include "SystemParamShare.h"
#include "Manager/ResultMgr.h"
#include "Alarm/AlarmController.h"
#include "FunctionModule/FunctionModuleProbeACC.h"
#include ".\CommTcpIpObjectPG.h"
#include "Manager\SystemConfigMgr.h"
#include "D_DisplayMessage.h"
//#include "Manager\ShareMemoryMgr.h"


// CDlgSubManual_Meas 대화 상자입니다.

IMPLEMENT_DYNAMIC(CDlgSubManual_Meas, CDialogEx)

CDlgSubManual_Meas::CDlgSubManual_Meas(CAuroraMgr* pAuroraMgr, CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgSubManual_Meas::IDD, pParent)
	, m_pAuroraMgr(pAuroraMgr)
	, m_iStep(0)
	, m_iMeasType(0)
	, m_bOpenRcp(FALSE)
	, m_iNumStick(0)
	, m_iNumCell(0)
	, m_nMeasureArea(0)
{
	//m_pSystem = NULL;
	//m_pSystem = SMEM_GetSharedMemory();

	//----------------------------------------------------------------------
	// Thread 를 미리 생성해 놓고 Event 로 제어 한다.
	//----------------------------------------------------------------------
	m_pThread = AfxBeginThread(ThreadRun, this);
	m_bThreadLive = false;
	m_hStartEvent = CreateEvent(NULL, FALSE, FALSE, NULL);		// auto reset, initial reset
	m_hCloseEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hMultiEvent[0] = m_hStartEvent;
	m_hMultiEvent[1] = m_hCloseEvent;

	m_pStInnerID = nullptr;

	m_vecMeasGroup.clear();

	SetMeasureArea(0);
}

CDlgSubManual_Meas::~CDlgSubManual_Meas()
{
	//----------------------------------------------------------------------
	// Thread 종료
	//----------------------------------------------------------------------
	ResetEvent(m_hStartEvent);
	SetEvent(m_hCloseEvent);
	if (m_pThread && m_bThreadLive)
	{
		WaitForSingleObject(m_pThread->m_hThread, INFINITE);
	}

	if (m_hStartEvent) CloseHandle(m_hStartEvent);
	if (m_hCloseEvent) CloseHandle(m_hCloseEvent);
}

void CDlgSubManual_Meas::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_MANUAL_TITLE, m_LabelInfo);
	DDX_Control(pDX, IDC_BTN_MANUAL_MEAS_RECIPE_OPEN, m_Btn_MM_Recipe_Open);
	DDX_Control(pDX, IDC_BTN_MANUAL_MEAS_LOADING_POSITION_MOVE, m_Btn_MM_Loading_Position_Move);
	DDX_Control(pDX, IDC_BTN_MANUAL_MEAS_LIFT_UP, m_Btn_MM_Lift_Up);
	DDX_Control(pDX, IDC_BTN_MANUAL_MEAS_VACUUM_MOTION, m_Btn_MM_Vacuum_Motion);
	DDX_Control(pDX, IDC_BTN_MANUAL_MEAS_LIFT_DOWN, m_Btn_MM_Lift_Down);
	DDX_Control(pDX, IDC_BTN_MANUAL_MEAS_CLAMP_IN, m_Btn_MM_Clamp_In);
	DDX_Control(pDX, IDC_BTN_MANUAL_MEAS_MOVE_ALIGN_POSITION_MOVE, m_Btn_MM_Move_Align_Position_Move);
	DDX_Control(pDX, IDC_BTN_MANUAL_MEAS_PROBE_ALIGN, m_Btn_MM_Probe_Align);
	DDX_Control(pDX, IDC_BTN_MANUAL_MEAS_GLASS_ALIGN, m_Btn_MM_Glass_Align);
	DDX_Control(pDX, IDC_BTN_MANUAL_MEAS_STICK_MOVE, m_Btn_MM_Stick_Move);
	DDX_Control(pDX, IDC_BTN_MANUAL_MEAS_AGING, m_Btn_MM_Aging);
	DDX_Control(pDX, IDC_BTN_MANUAL_MEAS_RTEST, m_Btn_MM_Rtest);
	DDX_Control(pDX, IDC_BTN_MANUAL_MEAS_OPST_TEST, m_Btn_MM_OpstTest);
	DDX_Control(pDX, IDC_BTN_MANUAL_MEAS_PTP_TEST, m_Btn_MM_PtpTest);
	DDX_Control(pDX, IDC_BTN_MANUAL_MEAS_NEXT_STICK_MOVE2, m_Btn_MM_Next_Stick_Move);
	DDX_Control(pDX, IDC_BTN_MANUAL_MEAS_PROPERTY_INSPECTION, m_Btn_MM_Property_Inspection);
	DDX_Control(pDX, IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE1, m_Btn_MM_Measurement_Cell_Move1);
	DDX_Control(pDX, IDC_BTN_MANUAL_MEAS_DEFINITION_INSPECTION, m_Btn_MM_Definition_Inspection);
	DDX_Control(pDX, IDC_BTN_MANUAL_MEAS_NEXT_CELL_MOVE1, m_Btn_MM_Next_Cell_Move1);
	DDX_Control(pDX, IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE2, m_Btn_MM_Measurement_Cell_Move2);
	DDX_Control(pDX, IDC_BTN_MANUAL_MEAS_ANGLE_OF_FIELD_INSPECTION, m_Btn_MM_Angle_Of_Field_Inspection);
	DDX_Control(pDX, IDC_BTN_MANUAL_MEAS_DATA_UPLOAD, m_Btn_MM_Data_Upload);
	DDX_Control(pDX, IDC_BTN_MANUAL_MEAS_LOADING_POSITION_MOVE2, m_Btn_MM_Loading_Position_Move2);
	DDX_Control(pDX, IDC_BTN_MANUAL_MEAS_CLAMP_OUT, m_Btn_MM_Clamp_Out);
	DDX_Control(pDX, IDC_BTN_MANUAL_MEAS_LIFT_UP2, m_Btn_MM_Lift_Up2);
	DDX_Control(pDX, IDC_BTN_MANUAL_MEAS_VACUUM_MOTION2, m_Btn_MM_Vacuum_Motion2);
	DDX_Control(pDX, IDC_BTN_MANUAL_MEAS_PREVIOUS_CELL_POSITION_MOVE, m_Btn_MM_Previous_Cell_Position_Move);
	DDX_Control(pDX, IDC_BTN_MANUAL_MEAS_NEXT_CELL_POSITION_MOVE3, m_Btn_MM_Next_Cell_Position_Move3);
	DDX_Control(pDX, IDC_BTN_MANUAL_MEAS_CONTACT_MOTION, m_Btn_MM_Contact_Motion);
	DDX_Control(pDX, IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE3, m_Btn_MM_Measurement_Cell_Move3);
}


BEGIN_MESSAGE_MAP(CDlgSubManual_Meas, CDialogEx)
	ON_WM_ERASEBKGND()

	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_RECIPE_OPEN, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasRecipeOpen)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_LOADING_POSITION_MOVE, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasLoadingPositionMove)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_LIFT_UP, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasLiftUp)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_VACUUM_MOTION, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasVacuumMotion)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_LIFT_DOWN, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasLiftDown)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_CLAMP_IN, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasClampIn)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_PROBE_ALIGN, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasProbeAlign)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_GLASS_ALIGN, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasGlassAlign)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_STICK_MOVE, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasStickMove)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_AGING, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasAging)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_RTEST, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasRtest)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_OPST_TEST, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasOpstTest)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_PTP_TEST, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasPtpTest)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_NEXT_STICK_MOVE2, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasNextStickMove)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_PROPERTY_INSPECTION, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasPropertyInspection)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE1, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasMeasurementCellMove1)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_DEFINITION_INSPECTION, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasDefinitionInspection)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_NEXT_CELL_MOVE1, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasNextCellMove1)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE2, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasMeasurementCellMove2)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_ANGLE_OF_FIELD_INSPECTION, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasAngleOfFieldInspection)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_DATA_UPLOAD, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasDataUpload)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_LOADING_POSITION_MOVE2, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasLoadingPositionMove2)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_CLAMP_OUT, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasClampOut)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_LIFT_UP2, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasLiftUp2)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_VACUUM_MOTION2, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasVacuumMotion2)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_PREVIOUS_CELL_POSITION_MOVE, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasPreviousCellPositionMove)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_NEXT_CELL_POSITION_MOVE3, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasNextCellPositionMove3)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_CONTACT_MOTION, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasContactMotion)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_CONTACT_MOTION2, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasContactMotion)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE3, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasMeasurementCellMove3)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_NEXT_STICK_MOVE3, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasNextStickMove3)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_PRE_STICK_MOVE2, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasPreStickMove)
	ON_BN_CLICKED(IDC_BTN_MANUAL_GLASS_ROTATE1, &CDlgSubManual_Meas::OnBnClickedBtnManualGlassRotate1)
	ON_BN_CLICKED(IDC_BTN_MANUAL_GLASS_ROTATE2, &CDlgSubManual_Meas::OnBnClickedBtnManualGlassRotate2)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_NEXT_CELL_MOVE3, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasNextCellMove3)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_NEXT_CELL_MOVE4, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasNextCellMove4)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_CHANGE, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasChange)
	ON_BN_CLICKED(IDC_BTN_MANUAL_READY_CHANGE, &CDlgSubManual_Meas::OnBnClickedBtnManualReadyChange)
	ON_BN_CLICKED(IDC_BTN_LP_DOWN, &CDlgSubManual_Meas::OnBnClickedBtnLpDown)
	ON_BN_CLICKED(IDC_BTN_LP_UP, &CDlgSubManual_Meas::OnBnClickedBtnLpUp)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_PG_CMD01, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasPgCmd01)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_PG_CMD2, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasPgCmd2)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_PG_CMD3, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasPgCmd3)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_PG_CMD4, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasPgCmd4)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_CLAMP_IN2, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasClampIn2)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_AIR_OFF, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasAirOff)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_AIR_ON, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasAirOn)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_CLAMP_OUT2, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasClampOut2)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_CLAMP_DOWN, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasClampDown)
	ON_BN_CLICKED(IDC_BTN_MANUAL_DEFINE_PTRN, &CDlgSubManual_Meas::OnBnClickedBtnManualDefinePtrn)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_CONTACT_MOTION3, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasContactMotion3)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_ANGLE_OF_FIELD_INSPECTION2, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasAngleOfFieldInspection2)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_ANGLE_OF_FIELD_INSPECTION4, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasAngleOfFieldInspection4)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_ANGLE_OF_FIELD_INSPECTION3, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasAngleOfFieldInspection3)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_CLAMP_IN5, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasClampIn5)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_VACUUM_MOTION5, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasVacuumMotion5)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_CLAMP_IN6, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasClampIn6)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_VACUUM_MOTION6, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasVacuumMotion6)
	ON_BN_CLICKED(IDC_BTN_MANUAL_GLASS_ALIGN, &CDlgSubManual_Meas::OnBnClickedBtnManualGlassAlign)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_NEXT_CELL_MOVE5, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasNextCellMove5)
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgSubManual_Meas::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CDlgSubManual_Meas::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CDlgSubManual_Meas::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CDlgSubManual_Meas::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CDlgSubManual_Meas::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, &CDlgSubManual_Meas::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BTN_MANUAL_SHUTTER, &CDlgSubManual_Meas::OnBnClickedBtnManualShutter)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_PG_CMD5, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasPgCmd5)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_AGING2, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasAging2)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_MEASUREMENT_SET_PG, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasMeasurementSetPg)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_MEASUREMENT_SET_PG2, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasMeasurementSetPg2)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_MEASUREMENT_SET_PG3, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasMeasurementSetPg3)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_MEASUREMENT_SET_PG4, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasMeasurementSetPg4)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_MEASUREMENT_SET_PG5, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasMeasurementSetPg5)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE7, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasMeasurementCellMove7)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE8, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasMeasurementCellMove8)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE9, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasMeasurementCellMove9)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE6, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasMeasurementCellMove6)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE4, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasMeasurementCellMove4)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE5, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasMeasurementCellMove5)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_CONTACT_MOTION4, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasContactMotion4)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_CONTACT_MOTION5, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasContactMotion5)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_PROBE_MOVE, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasProbeMove)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_RECIPE_OPEN2, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasRecipeOpen2)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_RECIPE_OPEN3, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasRecipeOpen3)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_RECIPE_OPEN4, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasRecipeOpen4)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_RECIPE_OPEN5, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasRecipeOpen5)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_RECIPE_OPEN6, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasRecipeOpen6)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_RECIPE_OPEN7, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasRecipeOpen7)
	ON_BN_CLICKED(IDC_BTN_MANUAL_MEAS_RECIPE_OPEN8, &CDlgSubManual_Meas::OnBnClickedBtnManualMeasRecipeOpen8)
	ON_BN_CLICKED(IDC_BTN_MANUAL_PLC_RESET, &CDlgSubManual_Meas::OnBnClickedBtnManualPlcReset)
	ON_MESSAGE(WM_MSG_UPDATE_CUR_STICK, &CDlgSubManual_Meas::OnUpdateCurStick)	// LYS 2026-08-24
	ON_BN_CLICKED(IDC_BTN_MANUAL_PLC_BIT_ON, &CDlgSubManual_Meas::OnBnClickedBtnManualPlcBitOn)
	ON_BN_CLICKED(IDC_BTN_MANUAL_PLC_BIT_OFF, &CDlgSubManual_Meas::OnBnClickedBtnManualPlcBItOff)
	ON_BN_CLICKED(IDC_BUTTON7, &CDlgSubManual_Meas::OnBnClickedBtnManualPlcWadReset)
END_MESSAGE_MAP()


// CDlgSubManual_Meas 메시지 처리기입니다.

BOOL CDlgSubManual_Meas::OnEraseBkgnd(CDC* pDC)
{
	// 	CRect rt;
	// 	GetClientRect(rt);
	// 	pDC->FillSolidRect(rt, RGB_MAIN3);
	// 	return TRUE;

	return CDialogEx::OnEraseBkgnd(pDC);
}


BOOL CDlgSubManual_Meas::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	// 
	//---------------------------------------------------------------------
	// 타이틀에 색상을 입혀준다.
	//---------------------------------------------------------------------
	m_LabelInfo.SetBorder(TRUE, TRUE);
	m_LabelInfo.SetBkColor(RGB(0, 32, 0));
	m_LabelInfo.SetTextColor(RGB(255, 255, 192));
	m_LabelInfo.SetFontBold(TRUE);

	UpdateCurStickUI();	// LYS 2026-08-24 seed the current stick readout


	//if (m_pSystem->stEquip.m_nEQType == EQU_TYPE_ICE || m_pSystem->stEquip.m_nEQType == EQU_TYPE_ICM)
	//{
	//	GetDlgItem(IDC_STATIC_ALIGN)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_STATIC_MEAS1)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_STATIC_MEAS2)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_STATIC_MEAS3)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_MOVE_ALIGN_POSITION_MOVE)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_STICK_MOVE)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_AGING)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_RTEST)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_OPST_TEST)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_PTP_TEST)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_NEXT_STICK_MOVE2)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE3)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_PROPERTY_INSPECTION)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE1)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_DEFINITION_INSPECTION)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE2)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_ANGLE_OF_FIELD_INSPECTION)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_NEXT_CELL_MOVE1)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_DATA_UPLOAD)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_PREVIOUS_CELL_POSITION_MOVE)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_NEXT_CELL_POSITION_MOVE3)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_CONTACT_MOTION)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_CONTACT_MOTION2)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_PRE_STICK_MOVE2)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_NEXT_STICK_MOVE3)->ShowWindow(SW_HIDE);
	//
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_CHANGE)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_READY_CHANGE)->ShowWindow(SW_HIDE);
	//}
	//else if (m_pSystem->stEquip.m_nEQType == EQU_TYPE_ICP)
	//{
	//
	//	GetDlgItem(IDC_STATIC_ALIGN)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_STATIC_MEAS1)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_STATIC_MEAS2)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_STATIC_MEAS3)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_MOVE_ALIGN_POSITION_MOVE)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_STICK_MOVE)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_AGING)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_RTEST)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_OPST_TEST)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_PTP_TEST)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_NEXT_STICK_MOVE2)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE3)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_PROPERTY_INSPECTION)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE1)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_DEFINITION_INSPECTION)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE2)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_ANGLE_OF_FIELD_INSPECTION)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_NEXT_CELL_MOVE1)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_DATA_UPLOAD)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_PREVIOUS_CELL_POSITION_MOVE)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_NEXT_CELL_POSITION_MOVE3)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_CONTACT_MOTION2)->ShowWindow(SW_HIDE);
	//
	//
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_CHANGE)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_READY_CHANGE)->ShowWindow(SW_HIDE);
	//
	//	// jsyoon 20171006 add
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_PREVIOUS_CELL_POSITION_MOVE)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_NEXT_CELL_POSITION_MOVE3)->ShowWindow(SW_HIDE);
	//
	//}
	//else if (m_pSystem->stEquip.m_nEQType == EQU_TYPE_OCP)
	//{
	//	GetDlgItem(IDC_STATIC_ALIGN)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_STATIC_MEAS1)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_STATIC_MEAS2)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_STATIC_MEAS3)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_MOVE_ALIGN_POSITION_MOVE)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_STICK_MOVE)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_AGING)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_RTEST)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_OPST_TEST)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_PTP_TEST)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_NEXT_STICK_MOVE2)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE3)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_PROPERTY_INSPECTION)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE1)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_DEFINITION_INSPECTION)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE2)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_ANGLE_OF_FIELD_INSPECTION)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_NEXT_CELL_MOVE1)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_DATA_UPLOAD)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_PREVIOUS_CELL_POSITION_MOVE)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_NEXT_CELL_POSITION_MOVE3)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_CONTACT_MOTION2)->ShowWindow(SW_HIDE);
	//
	//	CRect rect;
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_CLAMP_IN)->GetWindowRect(rect);
	//	ScreenToClient(&rect);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_CHANGE)->MoveWindow(rect);
	//
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_CLAMP_OUT)->GetWindowRect(rect);
	//	ScreenToClient(&rect);
	//	GetDlgItem(IDC_BTN_MANUAL_READY_CHANGE)->MoveWindow(rect);
	//
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_VACUUM_MOTION)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_CLAMP_IN)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_CLAMP_OUT)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_VACUUM_MOTION2)->ShowWindow(SW_HIDE);
	//
	//	// jsyoon 20171006 add
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_PREVIOUS_CELL_POSITION_MOVE)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_MANUAL_MEAS_NEXT_CELL_POSITION_MOVE3)->ShowWindow(SW_HIDE);
	//
	//	GetDlgItem(IDC_BTN_LP_DOWN)->ShowWindow(SW_HIDE);
	//	GetDlgItem(IDC_BTN_LP_UP)->ShowWindow(SW_HIDE);
	//
	//}
	//else
	//{
	//	;
	//}

	UPdateTestBtn(true);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}



///////////////////////	MANUAL MEAS ///////////////////////

//Recipe Open
void CDlgSubManual_Meas::OnBnClickedBtnManualMeasRecipeOpen()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_RECIPE_OPEN);
	m_iStep = enumStepID_RECIPE_OPEN;
	DoWorkManual();
}

///////////////////////	LOADING ///////////////////////

// Loding Position Move
void CDlgSubManual_Meas::OnBnClickedBtnManualMeasLoadingPositionMove()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_LOADING_POSITION_MOVE);
	m_iStep = enumStepID_GLASS_GET_01;
	DoWorkManual();
}

// LIFT Up
void CDlgSubManual_Meas::OnBnClickedBtnManualMeasLiftUp()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_LOADING_POSITION_MOVE);
	m_iStep = 12;
	DoWorkManual();
}

// Vacuum Motion
void CDlgSubManual_Meas::OnBnClickedBtnManualMeasVacuumMotion()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_VACUUM_MOTION);
	m_iStep = enumStepID_GLASS_GET_07;
	DoWorkManual();
}

// LIFT Down
void CDlgSubManual_Meas::OnBnClickedBtnManualMeasLiftDown()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_LIFT_DOWN);
	m_iStep = enumStepID_GLASS_GET_03;
	DoWorkManual();
}

// CLAMP IN
void CDlgSubManual_Meas::OnBnClickedBtnManualMeasClampIn()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_CLAMP_IN);
	m_iStep = enumStepID_GLASS_GET_05;
	DoWorkManual();
}

//////////////////////	UPLOADING ///////////////////////

// Loading Position Move
void CDlgSubManual_Meas::OnBnClickedBtnManualMeasLoadingPositionMove2()
{
	m_iStep = enumStepID_GLASS_PUT_01;
	DoWorkManual();
}

// CLAMP Out
void CDlgSubManual_Meas::OnBnClickedBtnManualMeasClampOut()
{
	m_iStep = enumStepID_GLASS_PUT_07;
	DoWorkManual();
}

// LIFT Up
void CDlgSubManual_Meas::OnBnClickedBtnManualMeasLiftUp2()
{
	m_iStep = enumStepID_GLASS_PUT_08;
	DoWorkManual();
}

// Vacuum Motion
void CDlgSubManual_Meas::OnBnClickedBtnManualMeasVacuumMotion2()
{
	m_iStep = enumStepID_GLASS_PUT_03;
	DoWorkManual();
}

///////////////////////	COMMAND ///////////////////////

// Previous Cell Position Move
void CDlgSubManual_Meas::OnBnClickedBtnManualMeasPreviousCellPositionMove()
{
	//OnClickBtnManualMeasPreviousCellPositionMove();

	//int iTmp = 0, iRet = VTM_RV_SUCCESSED;
	//iTmp = m_iStep;
	//m_iStep--;
	//iRet = ManualMoveCell();
	//if (iRet != VTM_RV_SUCCESSED)
	//	m_iStep = iTmp;
}

// Next Cell Position Move
void CDlgSubManual_Meas::OnBnClickedBtnManualMeasNextCellPositionMove3()
{
	/*OnClickBtnManualMeasNextCellPositionMove3();

	int iTmp = 0, iRet = VTM_RV_SUCCESSED;
	iTmp = m_iStep;
	m_iStep++;
	iRet = ManualMoveCell();
	if (iRet != VTM_RV_SUCCESSED)
		m_iStep = iTmp;*/
}

// Contact Motion
void CDlgSubManual_Meas::OnBnClickedBtnManualMeasContactMotion()
{
	OnClickBtnManualMeasContactMotion();

	m_iStep = enumStepID_PROBE_CONTACT_ALL;
	DoWorkManual();
}

///////////////////////	ALIGN ///////////////////////

// Move Align Position Move
void CDlgSubManual_Meas::OnBnClickedBtnManualMeasMoveAlignPositionMove()
{

}

// Probe Align
void CDlgSubManual_Meas::OnBnClickedBtnManualMeasProbeAlign()
{
	OnClickBtnManualMeasProbeAlign();
	m_iStep = enumStepID_PROBE_ALIGN;
	DoWorkManual();
	//m_pJobProc->DoStartSAlignDetect(0);
}

// Glass Align
void CDlgSubManual_Meas::OnBnClickedBtnManualMeasGlassAlign()
{
	OnClickBtnManualMeasGlassAlign();

	//m_pJobProc->DoStartSAlignDetect(1);
}

// Stick Move
void CDlgSubManual_Meas::OnBnClickedBtnManualMeasStickMove()
{
	OnClickBtnManualMeasStickMove();

	m_iStep = enumStepID_MOVE_STICK;
	DoWorkManual();

	//m_iNumStick = 0;
	//m_iStep = 0;

	//int iRet = VTM_RV_SUCCESSED;
	//iRet = ManualMoveStick();

}

///////////////////////	Electricity Inspection (전기검사) ///////////////////////

//AGING
void CDlgSubManual_Meas::OnBnClickedBtnManualMeasAging()
{
	OnClickBtnManualMeasAging();

	m_iStep = enumStepID_AGING_TEST;
	DoWorkManual();
}

//////////////////////	Preliminary Inspection (사전검사) ///////////////////////

// R-TEST
void CDlgSubManual_Meas::OnBnClickedBtnManualMeasRtest()
{
	OnClickBtnManualMeasRTest();

	m_iStep = enumStepID_PRE_TEST_03;
	DoWorkManual();
}

// OPEN/SHORT TEST
void CDlgSubManual_Meas::OnBnClickedBtnManualMeasOpstTest()
{
	OnClickBtnManualMeasOpStTest();

	m_iStep = enumStepID_PRE_TEST_02;
	DoWorkManual();

	//m_pJobProc->DoStartTestMeas(1);
}

// PIN TO PIN TEST
void CDlgSubManual_Meas::OnBnClickedBtnManualMeasPtpTest()
{
	OnClickBtnManualMeasPtpTest();

	m_iStep = enumStepID_PRE_TEST_01;
	DoWorkManual();
}

// Next Stick Move
void CDlgSubManual_Meas::OnBnClickedBtnManualMeasNextStickMove()
{
	OnClickBtnManualMeasNextStickMove();

	m_iStep = enumStepID_MOVE_STICK_NEXT;
	DoWorkManual();

	//m_iStep = 0;

	//int iNumStick = 0, iRet = VTM_RV_SUCCESSED;
	//iNumStick = m_iNumStick;

	//m_iNumStick++;
	//iRet = ManualMoveStick();
	//if (iRet != VTM_RV_SUCCESSED)
	//{
	//	m_iNumStick = iNumStick;
	//}
}

///////////////////////	Inspection (검사) ///////////////////////

// Property Inspection
void CDlgSubManual_Meas::OnBnClickedBtnManualMeasPropertyInspection()
{
	OnClickBtnManualMeasPropertyInspection();

	m_iStep = enumStepID_MEAS_TEST_WAD;
	DoWorkManual();

	//// Generate InnerID
	//int nProbe = 0;
	//CString strInnerID;
	//for (nProbe = 0; nProbe < df_MAX_PROBE; nProbe++)
	//{
	//	//strInnerID.Format(_T("%d%s"), nProbe + 1, gf_GetCurTimeString(3));
	//	//_tcscpy_s(m_pSystem->stLotInfo.m_szInnerID[nProbe], 32, strInnerID);
	//}

	//m_pJobProc->DoStartTestMeas(4);
}

// Measurement Cell Move1
void CDlgSubManual_Meas::OnBnClickedBtnManualMeasMeasurementCellMove1()
{
	OnClickBtnManualMeasMeasurementCellMove1();

	m_iStep = enumStepID_MOVE_CELL;
	m_iMeasType = M_TYPE_QUALITY;
	DoWorkManual();
//	ManualMoveCell(3);	// 화질
}

// Definition Inspection
void CDlgSubManual_Meas::OnBnClickedBtnManualMeasDefinitionInspection()
{
	OnClickBtnManualMeasDefinitionInspection();

	m_iStep = enumStepID_MEAS_TEST_VISION;
	DoWorkManual();

	//m_pJobProc->DoStartTestMeas(5);
}

// Nect Cell Move1
void CDlgSubManual_Meas::OnBnClickedBtnManualMeasNextCellMove1()
{
	OnClickBtnManualMeasNextCellMove1();

	m_iStep = enumStepID_MOVE_CELL_NEXT;
	DoWorkManual();

	//int iTmp = 0, iRet = VTM_RV_SUCCESSED;
	//iTmp = m_iStep;
	//m_iStep++;
	//iRet = ManualMoveCell();
	//if (iRet != VTM_RV_SUCCESSED)
	//	m_iStep = iTmp;
}

// Measurement Cell Move2
void CDlgSubManual_Meas::OnBnClickedBtnManualMeasMeasurementCellMove2()
{
	OnClickBtnManualMeasMeasurementCellMove2();

	m_iStep = enumStepID_MOVE_CELL;
	m_iMeasType = M_TYPE_COLOR;
	DoWorkManual();
//	ManualMoveCell(4);	// 색이상
}

// Angle of Field Inspection
void CDlgSubManual_Meas::OnBnClickedBtnManualMeasAngleOfFieldInspection()
{
	OnClickBtnManualMeasAngleOfFieldInspection();

	m_iStep = enumStepID_MEAS_TEST_COLOR;
	DoWorkManual();

	//m_pJobProc->DoStartTestMeas(6);
}

// Next Cell Move2
void CDlgSubManual_Meas::OnBnClickedBtnManualMeasNextCellMove2()
{
	OnClickBtnManualMeasNextCellMove2();
}

// DATA UPLOAD
void CDlgSubManual_Meas::OnBnClickedBtnManualMeasDataUpload()
{
	OnClickBtnManualMeasDataUpload();

	//m_pJobProc->DoWorkStartSave();
}

//////////////////// Log 생성 ///////////////////////

void CDlgSubManual_Meas::OnClickBtnManualMeasRecipeOpen()
{
	
}


void CDlgSubManual_Meas::OnClickBtnManualMeasLoadingPositionMove()
{
	
}


void CDlgSubManual_Meas::OnClickBtnManualMeasLiftUp()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_LIFT_UP);
}


void CDlgSubManual_Meas::OnClickBtnManualMeasVacuumMotion()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_VACUUM_MOTION);
}


void CDlgSubManual_Meas::OnClickBtnManualMeasLiftDown()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_LIFT_DOWN);
}


void CDlgSubManual_Meas::OnClickBtnManualMeasClampIn()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_CLAMP_IN);
}


void CDlgSubManual_Meas::OnClickBtnManualMeasMoveAlignPositionMove()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_MOVE_ALIGN_POSITION_MOVE);
}


void CDlgSubManual_Meas::OnClickBtnManualMeasProbeAlign()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_PROBE_ALIGN);
}


void CDlgSubManual_Meas::OnClickBtnManualMeasGlassAlign()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_GLASS_ALIGN);
}


void CDlgSubManual_Meas::OnClickBtnManualMeasStickMove()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_STICK_MOVE);
}


void CDlgSubManual_Meas::OnClickBtnManualMeasAging()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_AGING);
}


void CDlgSubManual_Meas::OnClickBtnManualMeasRTest()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_RTEST);
}


void CDlgSubManual_Meas::OnClickBtnManualMeasOpStTest()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_OPST_TEST);
}


void CDlgSubManual_Meas::OnClickBtnManualMeasPtpTest()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_PTP_TEST);
}


void CDlgSubManual_Meas::OnClickBtnManualMeasNextStickMove()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_NEXT_STICK_MOVE2);
}


void CDlgSubManual_Meas::OnClickBtnManualMeasPropertyInspection()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_PROPERTY_INSPECTION);
}


void CDlgSubManual_Meas::OnClickBtnManualMeasMeasurementCellMove1()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE1);
}


void CDlgSubManual_Meas::OnClickBtnManualMeasDefinitionInspection()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_DEFINITION_INSPECTION);
}


void CDlgSubManual_Meas::OnClickBtnManualMeasNextCellMove1()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_NEXT_CELL_MOVE1);
}


void CDlgSubManual_Meas::OnClickBtnManualMeasMeasurementCellMove2()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE2);
}

void CDlgSubManual_Meas::OnClickBtnManualMeasMeasurementCellMove3()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE3);
}

void CDlgSubManual_Meas::OnClickBtnManualMeasAngleOfFieldInspection()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_ANGLE_OF_FIELD_INSPECTION);
}


void CDlgSubManual_Meas::OnClickBtnManualMeasNextCellMove2()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_NEXT_CELL_MOVE2);
}


void CDlgSubManual_Meas::OnClickBtnManualMeasDataUpload()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_DATA_UPLOAD);
}


void CDlgSubManual_Meas::OnClickBtnManualMeasLoadingPositionMove2()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_LOADING_POSITION_MOVE2);
}


void CDlgSubManual_Meas::OnClickBtnManualMeasClampOut()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_CLAMP_OUT);
}


void CDlgSubManual_Meas::OnClickBtnManualMeasLiftUp2()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_LIFT_UP2);
}


void CDlgSubManual_Meas::OnClickBtnManualMeasVacuumMotion2()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_VACUUM_MOTION2);
}


void CDlgSubManual_Meas::OnClickBtnManualMeasLiftDown2()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_LIFT_DOWN2);
}


void CDlgSubManual_Meas::OnClickBtnManualMeasPreviousCellPositionMove()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_PREVIOUS_CELL_POSITION_MOVE);
}


void CDlgSubManual_Meas::OnClickBtnManualMeasNextCellPositionMove3()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_NEXT_CELL_POSITION_MOVE3);
}


void CDlgSubManual_Meas::OnClickBtnManualMeasContactMotion()
{
	BtnEventLog(IDC_BTN_MANUAL_MEAS_CONTACT_MOTION);
}


void CDlgSubManual_Meas::BtnEventLog(int iID)
{
	CString strText, strLog;
	switch (iID)
	{
	case IDC_BTN_MANUAL_MEAS_RECIPE_OPEN:
		m_Btn_MM_Recipe_Open.GetWindowText(strText);
		break;

	case IDC_BTN_MANUAL_MEAS_LOADING_POSITION_MOVE:
		m_Btn_MM_Loading_Position_Move.GetWindowText(strText);
		break;

	case IDC_BTN_MANUAL_MEAS_LIFT_UP:
		m_Btn_MM_Lift_Up.GetWindowText(strText);
		break;

	case IDC_BTN_MANUAL_MEAS_VACUUM_MOTION:
		m_Btn_MM_Vacuum_Motion.GetWindowText(strText);
		break;

	case IDC_BTN_MANUAL_MEAS_LIFT_DOWN:
		m_Btn_MM_Lift_Down.GetWindowText(strText);
		break;

	case IDC_BTN_MANUAL_MEAS_CLAMP_IN:
		m_Btn_MM_Clamp_In.GetWindowText(strText);
		break;

	case IDC_BTN_MANUAL_MEAS_MOVE_ALIGN_POSITION_MOVE:
		m_Btn_MM_Move_Align_Position_Move.GetWindowText(strText);
		break;

	case IDC_BTN_MANUAL_MEAS_PROBE_ALIGN:
		m_Btn_MM_Probe_Align.GetWindowText(strText);
		break;

	case IDC_BTN_MANUAL_MEAS_GLASS_ALIGN:
		m_Btn_MM_Glass_Align.GetWindowText(strText);
		break;

	case IDC_BTN_MANUAL_MEAS_STICK_MOVE:
		m_Btn_MM_Stick_Move.GetWindowText(strText);
		break;

	case IDC_BTN_MANUAL_MEAS_AGING:
		m_Btn_MM_Aging.GetWindowText(strText);
		break;

	case IDC_BTN_MANUAL_MEAS_RTEST:
		m_Btn_MM_Rtest.GetWindowText(strText);
		break;

	case IDC_BTN_MANUAL_MEAS_OPST_TEST:
		m_Btn_MM_OpstTest.GetWindowText(strText);
		break;

	case IDC_BTN_MANUAL_MEAS_PTP_TEST:
		m_Btn_MM_PtpTest.GetWindowText(strText);
		break;

	case IDC_BTN_MANUAL_MEAS_NEXT_STICK_MOVE2:
		m_Btn_MM_Next_Stick_Move.GetWindowText(strText);
		break;

	case IDC_BTN_MANUAL_MEAS_PROPERTY_INSPECTION:
		m_Btn_MM_Property_Inspection.GetWindowText(strText);
		break;

	case IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE1:
		m_Btn_MM_Measurement_Cell_Move1.GetWindowText(strText);
		break;

	case IDC_BTN_MANUAL_MEAS_DEFINITION_INSPECTION:
		m_Btn_MM_Definition_Inspection.GetWindowText(strText);
		break;

	case IDC_BTN_MANUAL_MEAS_NEXT_CELL_MOVE1:
		m_Btn_MM_Next_Cell_Move1.GetWindowText(strText);
		break;

	case IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE2:
		m_Btn_MM_Measurement_Cell_Move2.GetWindowText(strText);
		break;

	case IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE3:
		m_Btn_MM_Measurement_Cell_Move3.GetWindowText(strText);
		break;

	case IDC_BTN_MANUAL_MEAS_ANGLE_OF_FIELD_INSPECTION:
		m_Btn_MM_Angle_Of_Field_Inspection.GetWindowText(strText);
		break;

	case IDC_BTN_MANUAL_MEAS_NEXT_CELL_MOVE2:
		m_Btn_MM_Next_Cell_Move2.GetWindowText(strText);
		break;

	case IDC_BTN_MANUAL_MEAS_DATA_UPLOAD:
		m_Btn_MM_Data_Upload.GetWindowText(strText);
		break;

	case IDC_BTN_MANUAL_MEAS_LOADING_POSITION_MOVE2:
		m_Btn_MM_Loading_Position_Move2.GetWindowText(strText);
		break;

	case IDC_BTN_MANUAL_MEAS_CLAMP_OUT:
		m_Btn_MM_Clamp_Out.GetWindowText(strText);
		break;

	case IDC_BTN_MANUAL_MEAS_LIFT_UP2:
		m_Btn_MM_Lift_Up2.GetWindowText(strText);
		break;

	case IDC_BTN_MANUAL_MEAS_VACUUM_MOTION2:
		m_Btn_MM_Vacuum_Motion2.GetWindowText(strText);
		break;

	case IDC_BTN_MANUAL_MEAS_PREVIOUS_CELL_POSITION_MOVE:
		m_Btn_MM_Previous_Cell_Position_Move.GetWindowText(strText);
		break;

	case IDC_BTN_MANUAL_MEAS_NEXT_CELL_POSITION_MOVE3:
		m_Btn_MM_Next_Cell_Position_Move3.GetWindowText(strText);
		break;

	case IDC_BTN_MANUAL_MEAS_CONTACT_MOTION:
		m_Btn_MM_Contact_Motion.GetWindowText(strText);
		break;

	default:
		GetDlgItemText(iID, strText);
		break;

		// 	case IDC_BTN_MANUAL_GLASS_ROTATE2:
		// 		break;
		// 
		// 	case IDC_BTN_MANUAL_MEAS_NEXT_CELL_MOVE3:
		// 		break;
		// 
		// 	case IDC_BTN_MANUAL_MEAS_NEXT_CELL_MOVE4:
		// 		break;
	}


	strLog.Format(_T("OnClickBtn : %s"), strText);
	CLogWrapper::GetInstance()->KMLOG(OPER, strLog); // jsyoon 2022.08.04 add
	//gLog(m_pSystem->hLog, strLog, LOG_LEVEL_O);
}

BOOL CDlgSubManual_Meas::LoadRecipeDlg()
{
	CString filename = _T(""), strRcpFile;
	//	if(strFileName.IsEmpty())
	{
		//		CString filter = "Configuration Files (*.rcp)|*.rcp|All Files (*.*)|*.*||";
		CString filter = _T("Recipe Files (*.rcp)|*.rcp|");

		//		filename.Format("D:\\KMAC\\PARAM_ICP\\PARAM\\Recipe\\", _T("*.rcp"));
		//filename.Format("%s%s%s*.rcp", m_pSystem->m_strRootPath, STR_DIR_PARAM, dfRECIPE_DATA_FILE_PATH);

		CFileDialog dlg(TRUE, _T("rcp"), filename, OFN_FILEMUSTEXIST, filter); //OFN_FILEMUSTEXIST (존재하지 않는 파일명을 입력할 수 없음)
		//dlg.GetFileName().Format("%s", m_pSystem->stLotInfo.m_szRecipeID);

		if (dlg.DoModal() == IDOK)
		{
			filename = dlg.GetFileName();

			if (filename.IsEmpty())
			{
				MessageBox(_T("File is not an Recipe file or is locked and cannot be imported."));
			}
			else
			{
				int j = 0;
				j = filename.Find(_T(".rcp"));

				if (j > 0)
				{
					strRcpFile = filename.Left(j);		// remove ".rcp"
				}
				else
				{
					strRcpFile = filename;
				}

				//if (m_pDlgNaviRecipe->LoadPPID2(strRcpFile) == VTM_RV_SUCCESSED)
				//{
				//	MessageBox("Recipe file be imported complete.");
				//	return TRUE;
				//}
				//else
				//{
				//	AfxMessageBox("Load Recipe Error");
				//}
			}
		}
	}
	// 	else
	// 	{
	// 		filename = strFileName;
	// 	}

	return FALSE;
}

int CDlgSubManual_Meas::DoWorkManual()
{
	//if (SMEM_GetSys_ProcessStatus() == SPS_PROCESS)
	//{
	//	return 0;
	//}

	InterlockedExchange(&m_nWorkBusy, 1);	// LYS 2026-10-13 Hybrid Phase 7 (14)-C worker queued (2 is only taken and released inside UI-thread functions)
	SetEvent(m_hStartEvent);
	return 0;
}


int CDlgSubManual_Meas::UPdateTestBtn(BOOL bOn)
{
	if (m_bOpenRcp == false)
		bOn = FALSE;

	GetDlgItem(IDC_BTN_MANUAL_MEAS_RECIPE_OPEN2)->EnableWindow(bOn);
	GetDlgItem(IDC_BTN_MANUAL_MEAS_RECIPE_OPEN3)->EnableWindow(bOn);
	GetDlgItem(IDC_BTN_MANUAL_MEAS_RECIPE_OPEN4)->EnableWindow(bOn);
	GetDlgItem(IDC_BTN_MANUAL_MEAS_RECIPE_OPEN5)->EnableWindow(bOn);
	GetDlgItem(IDC_BTN_MANUAL_MEAS_RECIPE_OPEN6)->EnableWindow(bOn);
	GetDlgItem(IDC_BTN_MANUAL_MEAS_RECIPE_OPEN7)->EnableWindow(bOn);

	GetDlgItem(IDC_BTN_MANUAL_GLASS_ALIGN)->EnableWindow(bOn);
	GetDlgItem(IDC_BTN_MANUAL_MEAS_PROBE_ALIGN)->EnableWindow(bOn);
	GetDlgItem(IDC_BTN_MANUAL_MEAS_PROBE_MOVE)->EnableWindow(bOn);
	GetDlgItem(IDC_BTN_MANUAL_MEAS_PTP_TEST)->EnableWindow(bOn);
	GetDlgItem(IDC_BTN_MANUAL_MEAS_CONTACT_MOTION2)->EnableWindow(bOn);
	GetDlgItem(IDC_BTN_MANUAL_MEAS_OPST_TEST)->EnableWindow(bOn);
	GetDlgItem(IDC_BTN_MANUAL_MEAS_RTEST)->EnableWindow(bOn);
	GetDlgItem(IDC_BTN_MANUAL_MEAS_AGING2)->EnableWindow(bOn);
	GetDlgItem(IDC_BTN_MANUAL_MEAS_AGING)->EnableWindow(bOn);
	GetDlgItem(IDC_BTN_MANUAL_MEAS_CONTACT_MOTION3)->EnableWindow(bOn);

	GetDlgItem(IDC_BTN_MANUAL_MEAS_CONTACT_MOTION4)->EnableWindow(bOn);

	GetDlgItem(IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE7)->EnableWindow(bOn);
	GetDlgItem(IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE8)->EnableWindow(bOn);
	GetDlgItem(IDC_BTN_MANUAL_MEAS_MEASUREMENT_SET_PG2)->EnableWindow(bOn);
	GetDlgItem(IDC_BTN_MANUAL_MEAS_PROPERTY_INSPECTION)->EnableWindow(bOn);

	GetDlgItem(IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE1)->EnableWindow(bOn);
	GetDlgItem(IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE9)->EnableWindow(bOn);
	GetDlgItem(IDC_BTN_MANUAL_MEAS_MEASUREMENT_SET_PG)->EnableWindow(bOn);
	GetDlgItem(IDC_BTN_MANUAL_MEAS_DEFINITION_INSPECTION)->EnableWindow(bOn);

	GetDlgItem(IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE2)->EnableWindow(bOn);
	GetDlgItem(IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE6)->EnableWindow(bOn);
	GetDlgItem(IDC_BTN_MANUAL_MEAS_MEASUREMENT_SET_PG3)->EnableWindow(bOn);
	GetDlgItem(IDC_BTN_MANUAL_MEAS_ANGLE_OF_FIELD_INSPECTION)->EnableWindow(bOn);
	
	GetDlgItem(IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE4)->EnableWindow(bOn);
	GetDlgItem(IDC_BTN_MANUAL_MEAS_MEASUREMENT_CELL_MOVE5)->EnableWindow(bOn);
	GetDlgItem(IDC_BTN_MANUAL_MEAS_MEASUREMENT_SET_PG4)->EnableWindow(bOn);
	GetDlgItem(IDC_BTN_MANUAL_MEAS_ANGLE_OF_FIELD_INSPECTION2)->EnableWindow(bOn);

	GetDlgItem(IDC_BTN_MANUAL_MEAS_CONTACT_MOTION5)->EnableWindow(bOn);

	GetDlgItem(IDC_BTN_MANUAL_MEAS_MEASUREMENT_SET_PG5)->EnableWindow(bOn);
	GetDlgItem(IDC_BTN_MANUAL_MEAS_ANGLE_OF_FIELD_INSPECTION4)->EnableWindow(bOn);
	GetDlgItem(IDC_BTN_MANUAL_MEAS_ANGLE_OF_FIELD_INSPECTION3)->EnableWindow(bOn);

	GetDlgItem(IDC_BTN_MANUAL_MEAS_STICK_MOVE)->EnableWindow(bOn);
	GetDlgItem(IDC_BTN_MANUAL_MEAS_NEXT_STICK_MOVE2)->EnableWindow(bOn);

	GetDlgItem(IDC_BTN_MANUAL_MEAS_RECIPE_OPEN8)->EnableWindow(bOn);	// jsyoon 2022.10.20 add
	
	return 0;
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasMeasurementCellMove3()
{
	OnClickBtnManualMeasMeasurementCellMove3();

	m_iStep = enumStepID_MOVE_CELL;
	DoWorkManual();
}

/*!
* iMode로 선택한 Head 이동
*/
int CDlgSubManual_Meas::ManualMoveCell(int iMode, int iMove)
{
	// LYS 2026-10-13 manual meas = CycleTest positions (rca.md (5)(c) M6): OCP SPEC / QUALITY / ANGLE take the auto builders (ManualMoveCellShared).
	//   REVIEW / COLOR and HMI mode run the legacy code below unchanged
	if (UseSharedMeasPos(iMode)) return ManualMoveCellShared(iMode, iMove);

	int nRet = VTM_RV_SUCCESSED;
	stMotorPosition stPosition[20];
	ZeroMemory(&stPosition, sizeof(stPosition));
	int nAxis = 0;

	bool bMove = false;
	double dLeft = 0., dRight = 0.;
	double dLeft2 = 0., dRight2 = 0.;	// 2023.03.08 add for vision 2shot
	int i = 0;
	int nMeasArea = 0;
	nMeasArea = m_vecMeasGroup.size();
	bool bDirLtoR = true;
	for (i = 0; i < nMeasArea; i++)
	{
		if (i == iMove)
		{
			if (m_vecMeasGroup[i].m_sCellInfo[0].m_fCellX > m_vecMeasGroup[i].m_sCellInfo[1].m_fCellX)
			{
				dLeft = m_vecMeasGroup[i].m_sCellInfo[0].m_fCellX;
				dRight = m_vecMeasGroup[i].m_sCellInfo[1].m_fCellX;
			}
			else
			{
				dLeft = m_vecMeasGroup[i].m_sCellInfo[1].m_fCellX;
				dRight = m_vecMeasGroup[i].m_sCellInfo[0].m_fCellX;

				bDirLtoR = false;
			}

			bMove = true;

			break;
		}
	}

	if (bMove != true)
	{
		return VTM_RV_FAILED;
	}

	double dSafetyZ1 = 0., dSafetyZ2 = 0.;
	double fSafetyH1 = 0.0, fSafetyH2 = 0.0;
	m_pAuroraMgr->GetPositionConfigMgrPtr()->GetSafetyPosition(nullptr, nullptr, nullptr, &fSafetyH1, &fSafetyH2, &dSafetyZ1, &dSafetyZ2);
	m_pAuroraMgr->GetPositionConfigMgrPtr()->GetFrontHeadSafetyPosition(&fSafetyH1, &fSafetyH2);	// LYS 2026-09-14 fSafetyH1/H2 는 COLOR 의 dLeft2/dRight2(FRONT Y3/Y4) 에만 쓰여 FRONT Safety 키로 바꾼다
//	m_pAuroraMgr->GetPositionConfigMgrPtr()->GetSafetyPosition(nullptr, nullptr, nullptr, nullptr, nullptr, &dSafetyZ1, &dSafetyZ2);

	double dOffsetX = 0;
	S_RecipeInfo* pReciepInfo;
	pReciepInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();
	if (pReciepInfo != nullptr)
	{
		dOffsetX = pReciepInfo->m_sMotionOffsetPos.m_fStageY;	// 2023.07.05 add
	}

	if (nMeasArea == 1)
	{
		/*!
		* @note Area 가 하나일 경우 Vision Head 는 양쪽으로 이동하게 수정
		*/
		//S_RecipeInfo* pReciepInfo;
		//pReciepInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();

		double dHeadGapRcpF = 700.0, dHeadGapRcpB = 700.0, dMovePosF = 0.0, dMovePosB = 0.0;
		if (pReciepInfo != nullptr)
		{
			dHeadGapRcpF = pReciepInfo->m_sOptionItem.m_fFrontHeadGap;
			dHeadGapRcpB = pReciepInfo->m_sOptionItem.m_fBackHeadGap;
		}

		dMovePosF = dHeadGapRcpF / 2;
		dMovePosB = dHeadGapRcpB / 2;

		if (bDirLtoR == true)	//  - -> +
		{
			dLeft2 = dMovePosF * -1;
			dRight2 = dMovePosF;

			/*if (iMode == M_TYPE_QUALITY)
			{
				dLeft = dMovePosB * -1;
				dRight = dMovePosB;
			}*/
		}
		else  //  + -> -
		{
			dLeft2 = dMovePosF;
			dRight2 = dMovePosF * -1;

			//if (iMode == M_TYPE_QUALITY)
			//{
			//	dLeft = dMovePosB;
			//	dRight = dMovePosB * -1;
			//}
		}
	}
	else
	{
		dLeft2 = dLeft;
		dRight2 = dRight;
	}


	S_RecipeData* pRecipeData;
	pRecipeData = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeData();

	switch (iMode)
	{
	case M_TYPE_REVIEW: 
	{
		if (iMove == 0)
		{
			stPosition[0].axis = AXIS_Y1;   nAxis++;
			stPosition[0].dPosition = dLeft;
			stPosition[1].axis = AXIS_Y2;   nAxis++;
			stPosition[1].dPosition = dRight;
			stPosition[2].axis = AXIS_Y3;   nAxis++;
			stPosition[2].dPosition = dLeft2;
			stPosition[3].axis = AXIS_Y4;   nAxis++;
			stPosition[3].dPosition = dRight2;
			stPosition[4].axis = AXIS_X2;   nAxis++;
			stPosition[4].dPosition = m_vecMeasGroup[i].dHeadFrontG + dOffsetX;	// 2023.07.06 add offset
			stPosition[5].axis = AXIS_Z3;   nAxis++;	// LYS 2026-09-07 Review: Z1(OCP 미사용) -> Sub Z(Z3). 안전높이(0) 유지, 하강 없음
			stPosition[5].dPosition = 0.0;
			stPosition[6].axis = AXIS_Z2;   nAxis++;
			stPosition[6].dPosition = pRecipeData->m_sHeadZ.m_fReviewZ; // m_vecMeasGroup[i].dHeadBackZ;
		}
		else
		{
			stPosition[0].axis = AXIS_Y1;   nAxis++;
			stPosition[0].dPosition = dLeft;
			stPosition[1].axis = AXIS_Y2;   nAxis++;
			stPosition[1].dPosition = dRight;
			stPosition[2].axis = AXIS_X2;   nAxis++;
			stPosition[2].dPosition = m_vecMeasGroup[i].dHeadFrontG + dOffsetX; // 2023.07.06 add offset
			stPosition[3].axis = AXIS_Z3;   nAxis++;	// LYS 2026-09-07 Review: Z1(OCP 미사용) -> Sub Z(Z3). 안전높이(0) 유지, 하강 없음
			stPosition[3].dPosition = 0.0;
			stPosition[4].axis = AXIS_Z2;   nAxis++;
			stPosition[4].dPosition = pRecipeData->m_sHeadZ.m_fReviewZ; // m_vecMeasGroup[i].dHeadBackZ;
		}

		nRet = m_pAuroraMgr->GetMaintControlMgr()->MoveCmdPosReview(m_iNumStick, stPosition, nAxis, SPEED_HIGH);
	}
	break;
	case M_TYPE_SPEC:
	{
		if (iMove == 0)
		{
			stPosition[0].axis = AXIS_Y1;   nAxis++;
			stPosition[0].dPosition = dLeft;
			stPosition[1].axis = AXIS_Y2;   nAxis++;
			stPosition[1].dPosition = dRight;
			stPosition[2].axis = AXIS_Y3;   nAxis++;
			stPosition[2].dPosition = dLeft2;
			stPosition[3].axis = AXIS_Y4;   nAxis++;
			stPosition[3].dPosition = dRight2;
			// LYS 2026-09-14 OCP: 첫 이동의 FRONT 헤드 Y3/Y4 는 셀이 아니라 [SAFETY] 로 보낸다 (자동 측정 스텝과 동일, ApplyIdleHeadSafetyY 가 치환).
			if (m_pAuroraMgr->GetSystemConfigMgrPtr() && m_pAuroraMgr->GetSystemConfigMgrPtr()->IsOCP())
			{
				stPosition[2].dPosition = df_HEAD_Y_IDLE_SAFETY;
				stPosition[3].dPosition = df_HEAD_Y_IDLE_SAFETY;
			}
			stPosition[4].axis = AXIS_X2;   nAxis++;
			stPosition[4].dPosition = m_vecMeasGroup[i].dHeadFrontG + dOffsetX;	// 2023.07.06 add offset
			stPosition[5].axis = AXIS_Z3;   nAxis++;	// LYS 2026-09-07 WAD: Z1(OCP 미사용) -> Sub Z(Z3). 안전높이(0) 유지, 하강 없음
			stPosition[5].dPosition = 0.0;
			stPosition[6].axis = AXIS_Z2;   nAxis++;
			stPosition[6].dPosition = pRecipeData->m_sHeadZ.m_fSpecZ; // m_vecMeasGroup[i].dHeadBackZ;
		}
		else
		{
			stPosition[0].axis = AXIS_Y1;   nAxis++;
			stPosition[0].dPosition = dLeft;
			stPosition[1].axis = AXIS_Y2;   nAxis++;
			stPosition[1].dPosition = dRight;
			stPosition[2].axis = AXIS_X2;   nAxis++;
			stPosition[2].dPosition = m_vecMeasGroup[i].dHeadFrontG + dOffsetX;	// 2023.07.06 add offset
			stPosition[3].axis = AXIS_Z3;   nAxis++;	// LYS 2026-09-07 WAD: Z1(OCP 미사용) -> Sub Z(Z3). 안전높이(0) 유지, 하강 없음
			stPosition[3].dPosition = 0.0;
			stPosition[4].axis = AXIS_Z2;   nAxis++;
			stPosition[4].dPosition = pRecipeData->m_sHeadZ.m_fSpecZ; // m_vecMeasGroup[i].dHeadBackZ;
		}

		// jsyoon 2022.05.27 add
		S_RecipeInfo* pReciepInfo;
		pReciepInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();
		double dXPos = 0.;
		dXPos = m_vecMeasGroup[i].dHeadFrontG - m_pAuroraMgr->GetMaintControlMgr()->GetCurPos(AXIS_X) - pReciepInfo->m_sMotionOffsetPos.m_fHeadReviewY;

		m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->SetMeasPos(enumTCPIpFactorytID_WAD, dXPos, dLeft, dRight);

		MarkIdleHeadSafety(stPosition, nAxis, m_vecMeasGroup[i], 0.0);	// LYS 2026-09-14 짝 없는 셀의 쉬는 헤드 -> [SAFETY]
		nRet = m_pAuroraMgr->GetMaintControlMgr()->MoveCmdPosWAD(stPosition, nAxis, SPEED_HIGH);
	}
	break;
	case M_TYPE_QUALITY:
	{
		if (i == 0)
		{
			stPosition[0].axis = AXIS_Y1;   nAxis++;
			stPosition[0].dPosition = dLeft;
			stPosition[1].axis = AXIS_Y2;   nAxis++;
			stPosition[1].dPosition = dRight;
			stPosition[2].axis = AXIS_Y3;   nAxis++;
			stPosition[2].dPosition = dLeft2;
			stPosition[3].axis = AXIS_Y4;   nAxis++;
			stPosition[3].dPosition = dRight2;
			// LYS 2026-09-14 OCP: 첫 이동의 BACK 헤드 Y1/Y2 는 셀이 아니라 [SAFETY] 로 보낸다 (자동 측정 스텝과 동일, ApplyIdleHeadSafetyY 가 치환).
			if (m_pAuroraMgr->GetSystemConfigMgrPtr() && m_pAuroraMgr->GetSystemConfigMgrPtr()->IsOCP())
			{
				stPosition[0].dPosition = df_HEAD_Y_IDLE_SAFETY;
				stPosition[1].dPosition = df_HEAD_Y_IDLE_SAFETY;
			}
			stPosition[4].axis = AXIS_X3;   nAxis++;
			stPosition[4].dPosition = m_vecMeasGroup[i].dHeadFrontG + dOffsetX;	// 2023.07.06 add offset
			stPosition[5].axis = AXIS_Z1;   nAxis++;
			stPosition[5].dPosition = pRecipeData->m_sHeadZ.m_fQualityZ; //   m_vecMeasGroup[i].dHeadFrontZ;
			stPosition[6].axis = AXIS_Z2;   nAxis++;
			stPosition[6].dPosition = dSafetyZ1; //m_vecMeasGroup[i].dHeadBackZ;

			// jsyoon 2022.06.10 add
			stPosition[7].axis = AXIS_ZOOM1;   nAxis++;
			stPosition[7].dPosition = 0;
			stPosition[8].axis = AXIS_THETA1;   nAxis++;
			stPosition[8].dPosition = 0;
			stPosition[9].axis = AXIS_ZOOM2;   nAxis++;
			stPosition[9].dPosition = 0;
			stPosition[10].axis = AXIS_THETA2;   nAxis++;
			stPosition[10].dPosition = 0;
		}
		else
		{
			stPosition[0].axis = AXIS_Y3;   nAxis++;
			stPosition[0].dPosition = dLeft2;
			stPosition[1].axis = AXIS_Y4;   nAxis++;
			stPosition[1].dPosition = dRight2;
			stPosition[2].axis = AXIS_X3;   nAxis++;
			stPosition[2].dPosition = m_vecMeasGroup[i].dHeadFrontG + dOffsetX;	// 2023.07.06 add offset
			stPosition[3].axis = AXIS_Z1;   nAxis++;
			stPosition[3].dPosition = pRecipeData->m_sHeadZ.m_fQualityZ;  // m_vecMeasGroup[i].dHeadFrontZ;
			stPosition[4].axis = AXIS_Z2;   nAxis++;
			stPosition[4].dPosition = dSafetyZ1; //m_vecMeasGroup[i].dHeadBackZ;
		}

		MarkIdleHeadSafety(stPosition, nAxis, m_vecMeasGroup[i], 0.0);	// LYS 2026-09-14 짝 없는 셀의 쉬는 헤드 -> [SAFETY]
		nRet = m_pAuroraMgr->GetMaintControlMgr()->MoveCmdPosVision(stPosition, nAxis, SPEED_HIGH);

	}
	break;
	case M_TYPE_COLOR:
	{
		if (i == 0)
		{
			//2023.05.22 (면계측 HEAD1, 2 이동 후 수동 설정 진행 bLeftHead 값에 따라 left, right 진행) // UI LEFT, RIGHT 선택 추가 후 진행
			if (nMeasArea == 1) {
				BOOL bLeftHead = TRUE;

				if (bLeftHead)	dLeft2  = fSafetyH1;
				else			dRight2 = fSafetyH2;
			}

			stPosition[0].axis = AXIS_Y1;   nAxis++;
			stPosition[0].dPosition = dLeft;
			stPosition[1].axis = AXIS_Y2;   nAxis++;
			stPosition[1].dPosition = dRight;
			stPosition[2].axis = AXIS_Y3;   nAxis++;
			stPosition[2].dPosition = dLeft2;
			stPosition[3].axis = AXIS_Y4;   nAxis++;
			stPosition[3].dPosition = dRight2;
			// LYS 2026-09-14 OCP: 첫 이동의 BACK 헤드 Y1/Y2 는 셀이 아니라 [SAFETY] 로 보낸다 (자동 측정 스텝과 동일, ApplyIdleHeadSafetyY 가 치환).
			if (m_pAuroraMgr->GetSystemConfigMgrPtr() && m_pAuroraMgr->GetSystemConfigMgrPtr()->IsOCP())
			{
				stPosition[0].dPosition = df_HEAD_Y_IDLE_SAFETY;
				stPosition[1].dPosition = df_HEAD_Y_IDLE_SAFETY;
			}
			stPosition[4].axis = AXIS_X3;   nAxis++;
			stPosition[4].dPosition = m_vecMeasGroup[i].dHeadFrontG + dOffsetX;	// 2023.07.06 add offset
			stPosition[5].axis = AXIS_Z1;   nAxis++;
			stPosition[5].dPosition = pRecipeData->m_sHeadZ.m_fColorZ; //m_vecMeasGroup[i].dHeadFrontZ;
			stPosition[6].axis = AXIS_Z2;   nAxis++;
			stPosition[6].dPosition = dSafetyZ1; //m_vecMeasGroup[i].dHeadBackZ;
		}
		else
		{
			stPosition[0].axis = AXIS_Y3;   nAxis++;
			stPosition[0].dPosition = dLeft2;
			stPosition[1].axis = AXIS_Y4;   nAxis++;
			stPosition[1].dPosition = dRight2;
			stPosition[2].axis = AXIS_X3;   nAxis++;
			stPosition[2].dPosition = m_vecMeasGroup[i].dHeadFrontG + dOffsetX;	// 2023.07.06 add offset
			stPosition[3].axis = AXIS_Z1;   nAxis++;
			stPosition[3].dPosition = pRecipeData->m_sHeadZ.m_fColorZ; //m_vecMeasGroup[i].dHeadFrontZ;
			stPosition[4].axis = AXIS_Z2;   nAxis++;
			stPosition[4].dPosition = dSafetyZ1; //m_vecMeasGroup[i].dHeadBackZ;
		}

		MarkIdleHeadSafety(stPosition, nAxis, m_vecMeasGroup[i], 0.0);	// LYS 2026-09-14 짝 없는 셀의 쉬는 헤드 -> [SAFETY]
		nRet = m_pAuroraMgr->GetMaintControlMgr()->MoveCmdPosColor(stPosition, nAxis, SPEED_HIGH);
	}
	break;
	case M_TYPE_ANGLE:
	{
		if (i == 0)
		{
			stPosition[0].axis = AXIS_Y1;   nAxis++;
			stPosition[0].dPosition = dLeft;
			stPosition[1].axis = AXIS_Y2;   nAxis++;
			stPosition[1].dPosition = dRight;
			stPosition[2].axis = AXIS_Y3;   nAxis++;
			stPosition[2].dPosition = dLeft2;
			stPosition[3].axis = AXIS_Y4;   nAxis++;
			stPosition[3].dPosition = dRight2;
			// LYS 2026-09-14 OCP: 첫 이동의 BACK 헤드 Y1/Y2 는 셀이 아니라 [SAFETY] 로 보낸다 (자동 측정 스텝과 동일, ApplyIdleHeadSafetyY 가 치환).
			if (m_pAuroraMgr->GetSystemConfigMgrPtr() && m_pAuroraMgr->GetSystemConfigMgrPtr()->IsOCP())
			{
				stPosition[0].dPosition = df_HEAD_Y_IDLE_SAFETY;
				stPosition[1].dPosition = df_HEAD_Y_IDLE_SAFETY;
			}
			stPosition[4].axis = AXIS_X2;   nAxis++;
			stPosition[4].dPosition = m_vecMeasGroup[i].dHeadFrontG + dOffsetX;	// 2023.07.06 add offset
			stPosition[5].axis = AXIS_Z1;   nAxis++;
			stPosition[5].dPosition = dSafetyZ2; //m_vecMeasGroup[i].dHeadFrontZ;
			stPosition[6].axis = AXIS_Z2;   nAxis++;
			stPosition[6].dPosition = pRecipeData->m_sHeadZ.m_fAngleZ; //m_vecMeasGroup[i].dHeadBackZ;
		}
		else
		{
			stPosition[0].axis = AXIS_Y1;   nAxis++;
			stPosition[0].dPosition = dLeft;
			stPosition[1].axis = AXIS_Y2;   nAxis++;
			stPosition[1].dPosition = dRight;
			stPosition[2].axis = AXIS_X2;   nAxis++;
			stPosition[2].dPosition = m_vecMeasGroup[i].dHeadFrontG + dOffsetX;	// 2023.07.06 add offset
			stPosition[3].axis = AXIS_Z1;   nAxis++;
			stPosition[3].dPosition = dSafetyZ2; //m_vecMeasGroup[i].dHeadFrontZ;
			stPosition[4].axis = AXIS_Z2;   nAxis++;
			stPosition[4].dPosition = pRecipeData->m_sHeadZ.m_fAngleZ; //m_vecMeasGroup[i].dHeadBackZ;
		}

		MarkIdleHeadSafety(stPosition, nAxis, m_vecMeasGroup[i], 0.0);	// LYS 2026-09-14 짝 없는 셀의 쉬는 헤드 -> [SAFETY]
		nRet = m_pAuroraMgr->GetMaintControlMgr()->MoveCmdPosAngle(stPosition, nAxis, SPEED_HIGH);
	}
	break;
	default:
		break;
	}

	return nRet;

	//CMDMOVE stMove;
	//MEASGROUP	stMeasGroup;
	//ZeroMemory(&stMeasGroup, sizeof(stMeasGroup));
	//ZeroMemory(&stMove, sizeof(stMove));
	////stMeasGroup = m_pDlgNaviRecipe->GetMeasGroupStick(m_iNumStick, m_iStep);
	//
	//if (stMeasGroup.nGroup < 1)
	//	return VTM_RV_FAILED;
	//
	//BOOL bUseProbe[df_MaxProbe];
	//int  nUseCELLID[df_MaxProbe];
	//int nProbe = 0, iTop = 0, iBottom = 0, nMaxTop = 0, nMaxBottom = 0, iMoveTop = -1, iCountY = 0;
	//int iUseCount = 0, iUnuseCount = 0;
	//for (nProbe = 0; nProbe < df_MAX_PROBE; nProbe++)
	//{
	//	bUseProbe[nProbe] = FALSE;
	//	nUseCELLID[nProbe] = 0;
	//
	//	//if (m_pSystem->stSystem[VTM_PROBE1 + nProbe].bUse)
	//	//{
	//	//	iUseCount++;
	//	//}
	//}
	//
	//CString strMeasCell, strGlassID, strCellID, strMcrID, strInnerID;
	//
	//for (nProbe = 0; nProbe < iUseCount; nProbe++)	// PG CELL 번호 설정
	//{
	//	nUseCELLID[nProbe] = stMeasGroup.stProbe[nProbe].m_iAreaNum + 1 - (iTop * nMaxBottom);
	//	//		nUseCELLID[nProbe] = (nMaxBottom - stMeasGroup.stProbe[nProbe].m_iAreaNum) - (iTop * nMaxBottom);		// jsyoon 20161101 change
	//
	//	if (nUseCELLID[nProbe] < 1)
	//		nUseCELLID[nProbe] = 1;
	//
	//	bUseProbe[nProbe] = !stMeasGroup.stProbe[nProbe].m_bUsed;
	//
	//	if (bUseProbe[nProbe])
	//	{
	//		CString strTmp;
	//		strTmp.Format(_T("%s "), stMeasGroup.stProbe[nProbe].m_CellID);
	//		strMeasCell += strTmp;
	//
	//		// Cell ID
	//		//strGlassID.Format("%s", m_pSystem->stLotInfo.m_szGlassID);
	//		strCellID.Format(_T("%s%s", strGlassID.Left(strGlassID.GetLength() - 1), strTmp));
	//		strMcrID.Format(_T("%s", gf_MakeMcrID(strCellID)));
	//		//_tcscpy_s(m_pSystem->stLotInfo.m_szCellID[nProbe], 32, strMcrID);		// CELL ID 설정
	//
	//		double dCardW = 0.;
	//		//			dCardW = m_pDlgNaviRecipe->m_RecipeMeas.dTmp[1];			// Probe Pin 과 Bottom Align CCD 간격
	//		//dCardW = m_pDlgNaviRecipe->m_stRecipeMeas.dTmp[1];
	//
	//		// BOTTOM
	//		stMove.stAxis[AXIS_X2].bUse = TRUE;
	//		stMove.stAxis[AXIS_X2].dPos = stMeasGroup.stProbe[0].stAxis[AXIS_X2].dPos - dCardW;
	//		//stMove.dMoveUserOffset[AXIS_X2] = m_pDlgNaviRecipe->m_stRecipeMeas.dCentactZ[11];
	//
	//		if (nProbe == 1)
	//		{
	//			stMove.stAxis[AXIS_Y2].bUse = TRUE;
	//			stMove.stAxis[AXIS_Y2].dPos = stMeasGroup.stProbe[nProbe].stAxis[AXIS_Y1].dPos;
	//			//stMove.dMoveUserOffset[AXIS_Y2] = m_pDlgNaviRecipe->m_stRecipeMeas.dCentactZ[12];
	//
	//			stMove.stAxis[AXIS_Z3].bUse = TRUE;
	//			stMove.stAxis[AXIS_Z3].dPos = stMeasGroup.stProbe[nProbe].stAxis[AXIS_Z].dPos;
	//
	//			stMove.stAxis[AXIS_Z4].bUse = TRUE;
	//			stMove.stAxis[AXIS_Z4].dPos = stMeasGroup.stProbe[nProbe].stAxis[AXIS_Z1].dPos;
	//
	//			stMove.stAxis[AXIS_Z5].bUse = TRUE;
	//			stMove.stAxis[AXIS_Z5].dPos = stMeasGroup.stProbe[nProbe].stAxis[AXIS_Z2].dPos;
	//
	//			stMove.stAxis[AXIS_ZOOM2].bUse = TRUE;
	//			stMove.stAxis[AXIS_ZOOM2].dPos = stMeasGroup.stProbe[nProbe].stAxis[AXIS_ZOOM1].dPos;
	//		}
	//		else
	//		{
	//			stMove.stAxis[AXIS_Y1].bUse = TRUE;
	//			stMove.stAxis[AXIS_Y1].dPos = stMeasGroup.stProbe[nProbe].stAxis[AXIS_Y1].dPos;
	//			//stMove.dMoveUserOffset[AXIS_Y1] = m_pDlgNaviRecipe->m_stRecipeMeas.dCentactZ[12];
	//
	//			stMove.stAxis[AXIS_Z].bUse = TRUE;
	//			stMove.stAxis[AXIS_Z].dPos = stMeasGroup.stProbe[nProbe].stAxis[AXIS_Z].dPos;
	//
	//			stMove.stAxis[AXIS_Z1].bUse = TRUE;
	//			stMove.stAxis[AXIS_Z1].dPos = stMeasGroup.stProbe[nProbe].stAxis[AXIS_Z1].dPos;
	//
	//			stMove.stAxis[AXIS_Z2].bUse = TRUE;
	//			stMove.stAxis[AXIS_Z2].dPos = stMeasGroup.stProbe[nProbe].stAxis[AXIS_Z2].dPos;
	//
	//			stMove.stAxis[AXIS_ZOOM1].bUse = TRUE;
	//			stMove.stAxis[AXIS_ZOOM1].dPos = stMeasGroup.stProbe[nProbe].stAxis[AXIS_ZOOM1].dPos;
	//		}
	//	}
	//}
	//
	//if (!stMove.stAxis[AXIS_Y1].bUse && stMove.stAxis[AXIS_Y2].bUse)
	//{
	//	stMove.stAxis[AXIS_Y1].bUse = FALSE;
	//	//stMove.stAxis[AXIS_Y1].dPos = m_pSystem->stSystem[VTM_MOTION].m_dSafety[AXIS_Y1];
	//
	//	stMove.stAxis[AXIS_Z1].bUse = TRUE;
	//	stMove.stAxis[AXIS_Z1].dPos = stMove.stAxis[AXIS_Z4].dPos;
	//}
	//
	//if (stMove.stAxis[AXIS_Y1].bUse && !stMove.stAxis[AXIS_Y2].bUse)
	//{
	//	stMove.stAxis[AXIS_Y2].bUse = FALSE;
	//	//stMove.stAxis[AXIS_Y2].dPos = m_pSystem->stSystem[VTM_MOTION].m_dSafety[AXIS_Y2];
	//}
	//
	////// Rotate
	//////_TEST 90 or 0도 레시피에서 설정 가능 필요
	////if( m_pDlgNaviRecipe->m_stRecipePath.m_nRotate == 0 || m_pDlgNaviRecipe->m_stRecipePath.m_nRotate == 2 )	// 0 or 180
	////{
	////	stMove.stAxis[AXIS_THETA1].bUse = TRUE;
	////	stMove.stAxis[AXIS_THETA1].dPos = 90.0;
	////	stMove.stAxis[AXIS_THETA2].bUse = TRUE;
	////	stMove.stAxis[AXIS_THETA2].dPos = 90.0;
	////}
	////else
	////{
	////	stMove.stAxis[AXIS_THETA1].bUse = TRUE;
	////	stMove.stAxis[AXIS_THETA1].dPos = 90.0;
	////	stMove.stAxis[AXIS_THETA2].bUse = TRUE;
	////	stMove.stAxis[AXIS_THETA2].dPos = 90.0;
	////}
	//
	////stMove.dMoveUserOffset[AXIS_THETA1] = m_pDlgNaviRecipe->m_stRecipeMeas.dCentactZ[13]; 
	////stMove.dMoveUserOffset[AXIS_THETA2] = m_pDlgNaviRecipe->m_stRecipeMeas.dCentactZ[13];
	//
	////20181030 add by Hee
	//double fRotate = 0.0;
	////if (m_pDlgNaviRecipe->m_stRecipeMeas.dTmp[QUALITY_CCD_ROTATE]) fRotate = 0.0;
	////else														   fRotate = 90.0;
	//
	////20211020
	//stMove.stAxis[AXIS_THETA1].bUse = TRUE;
	////stMove.stAxis[AXIS_THETA1].dPos = fRotate + m_pDlgNaviRecipe->m_stRecipeMeas.dTmp[QUALITY_L_OFFSET];
	//stMove.stAxis[AXIS_THETA2].bUse = TRUE;
	////stMove.stAxis[AXIS_THETA2].dPos = fRotate + m_pDlgNaviRecipe->m_stRecipeMeas.dTmp[QUALITY_R_OFFSET];
	//
	////stMove.dMoveUserOffset[AXIS_THETA1] = m_pDlgNaviRecipe->m_stRecipeMeas.dCentactZ[13] + m_pDlgNaviRecipe->m_stRecipeMeas.dTmp[QUALITY_L_OFFSET]; 
	////stMove.dMoveUserOffset[AXIS_THETA2] = m_pDlgNaviRecipe->m_stRecipeMeas.dCentactZ[13] + m_pDlgNaviRecipe->m_stRecipeMeas.dTmp[QUALITY_R_OFFSET];
	////stMove.dMoveUserOffset[AXIS_THETA1] = m_pDlgNaviRecipe->m_stRecipeMeas.dTmp[QUALITY_L_OFFSET]; 
	////stMove.dMoveUserOffset[AXIS_THETA2] = m_pDlgNaviRecipe->m_stRecipeMeas.dTmp[QUALITY_R_OFFSET];
	////
	//
	//for (nProbe = 0; nProbe < iUseCount; nProbe++)
	//{
	//	if (!bUseProbe[nProbe])
	//	{
	//		iUnuseCount++;
	//	}
	//
	//	//m_pDlgNaviMaint->m_bMeasProbe[nProbe] = bUseProbe[nProbe];		// 레시피 Head 0 은 실제 Head 1과 같아 변경함
	//}
	//
	//if (iUnuseCount >= iUseCount)
	//{
	//	iUnuseCount = 0;
	//	return VTM_RV_FAILED;
	//}
	//
	////m_pDlgNaviMaint->m_dlgNaviMaintMotionACS.SetMeasIndex(iMove);	// Review
	////m_pDlgNaviMaint->m_dlgNaviMaintMotionACS.Cmd_MoveXYZ(stMove, TRUE);

	return VTM_RV_SUCCESSED;
}

void CDlgSubManual_Meas::OnBnClickedBtnManualMeasNextStickMove3()
{
	m_iStep = enumStepID_MOVE_STICK_NEXT;
	DoWorkManual();
	/*m_iStep = 0;

	int iNumStick = 0, iRet = VTM_RV_SUCCESSED;
	iNumStick = m_iNumStick;

	m_iNumStick++;
	iRet = ManualMoveStick();
	if (iRet != VTM_RV_SUCCESSED)
	{
		m_iNumStick = iNumStick;
	}*/
}


// LYS 2026-08-24 Current / Total stick Count
void	CDlgSubManual_Meas::UpdateCurStickUI()
{
	int nTotalStick = 0;
	S_RecipeData* pRecipeData = NULL;
	if (m_pAuroraMgr != NULL && m_pAuroraMgr->GetRecipeEditMgrPtr() != NULL)
	{
		pRecipeData = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeData();
	}
	if (pRecipeData != NULL)
	{
		nTotalStick = (int)pRecipeData->m_vecStick.size();
	}

	if (::IsWindow(GetSafeHwnd()))
	{
		::PostMessage(GetSafeHwnd(), WM_MSG_UPDATE_CUR_STICK,
			(WPARAM)m_iNumStick, (LPARAM)nTotalStick);
	}
}

LRESULT CDlgSubManual_Meas::OnUpdateCurStick(WPARAM wParam, LPARAM lParam)
{
	int nCur = (int)wParam;
	int nTotal = (int)lParam;

	CString strStick;

	if (nTotal > 0)
	{
		strStick.Format(_T("%d"), nCur + 1);
	}
	SetDlgItemText(IDC_EDIT_MANUAL_CUR_STICK, strStick);

	strStick.Format(_T("%d"), nTotal);
	SetDlgItemText(IDC_EDIT_MANUAL_TOTAL_STICK, strStick);

	return 0;
}

int CDlgSubManual_Meas::ManualMoveStick(int iStick)
{
	int nRet = VTM_RV_SUCCESSED;

	S_RecipeData* pRecipeData;
	pRecipeData = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeData();
	if (pRecipeData == nullptr || pRecipeData->m_vecStick.size() < 1)
	{
		return VTM_RV_FAILED;
	}

	S_RecipeMap* pRecipeMap;
	pRecipeMap = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeMap();
	if (pRecipeMap == nullptr)
	{
		return VTM_RV_FAILED;
	}

	S_RecipeInfo* pRecipeInfo;
	pRecipeInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();
	if (pRecipeInfo == nullptr)
	{
		return VTM_RV_FAILED;
	}

	// LYS 2026-08-24 Stick index guard. The Next/Pre buttons walk m_iNumStick on their
	if (iStick < 0 || iStick >= (int)pRecipeData->m_vecStick.size())
	{
		return VTM_RV_FAILED;
	}

	bool bOCP = (m_pAuroraMgr->GetSystemConfigMgrPtr() != nullptr
		&& m_pAuroraMgr->GetSystemConfigMgrPtr()->IsOCP());

	// LYS 2026-10-13 Hybrid Phase 7 CalcStickPose (hybrid gate) copies the pose formula below - change both together
	// LYS 2026-08-24 Target stage pose
	double dOffsetX = 0., dOffsetR = 0.;
	dOffsetX = pRecipeInfo->m_sMotionOffsetPos.m_fStageY;
	dOffsetR = pRecipeInfo->m_sMotionOffsetPos.m_fStageRotate;

	double dStageY = 0., dStageR = 0.;
	if (bOCP)
	{
		// LYS 2026-08-24 uses stick position
		dStageY = pRecipeData->m_vecStick[iStick].m_fStageY + dOffsetX;
	}
	else if (iStick > 0)
	{
		dStageY = pRecipeData->m_vecStick[0].m_fStageY - pRecipeData->m_vecStick[iStick].m_fStageY + dOffsetX;
	}
	else
	{
		dStageY = pRecipeMap->m_fCellDistance.y * iStick + dOffsetX;
	}

	dStageR = pRecipeData->m_vecStick[iStick].m_fStageR + dOffsetR;

	// LYS 2026-08-24 Already on target - do not un-contact or move anything.
	double dCurY = 0., dCurR = 0.;
	dCurY = m_pAuroraMgr->GetMaintControlMgr()->GetCurPos(AXIS_X);
	dCurR = m_pAuroraMgr->GetMaintControlMgr()->GetCurPos(AXIS_LOADER_R);
	if (fabs(dStageY - dCurY) < 0.0001 && fabs(dStageR - dCurR) < 0.0001)
	{
		return VTM_RV_SUCCESSED;
	}

	// Probe UnContact
	if (nRet == VTM_RV_SUCCESSED)
	{
		bool bMove = false;

		// LYS 2026-08-24 carries P5 only
		if (bOCP != true)
		{
			for (int i = AXIS_PROBE_Z1; i < AXIS_PROBE_Z5; i++)
			{
				if (m_pAuroraMgr->GetMaintControlMgr()->GetCurPos(i) > 0.1)
				{
					bMove = true;
					break;
				}
			}
		}

		double dfMove = 0.;
		dfMove = pRecipeInfo->m_sContactPos.m_fPreContactPos;

		if (bMove == true)
		{
			nRet = m_pAuroraMgr->GetMaintControlMgr()->MoveCmdPos_ProbeUnContact(AXIS_ALL_Z, dfMove);
		}
		else
		{
			if (m_pAuroraMgr->GetMaintControlMgr()->GetCurPos(AXIS_PROBE_Z5) > 0.1)
			nRet = m_pAuroraMgr->GetMaintControlMgr()->MoveCmdPos_ProbeUnContact(AXIS_PROBE_Z5, dfMove);
		}
	}

	// LYS 2026-10-13 manual meas = CycleTest positions ((8) safety, verify_fix #3): OCP - the shared moves leave Sub Z and FRONT Z at recipe height.
	//   Raise Sub Z Z3~Z6 / FRONT Z (only axes below safety, no log when none) before the stage turns or moves. HMI mode: unchanged
	if (nRet == VTM_RV_SUCCESSED && bOCP)
	{
		nRet = RetractHeadZBeforeStage(iStick);
	}

	// LYS 2026-08-24 Retract the back head before the stage turns.
	if (nRet == VTM_RV_SUCCESSED)
	{
		if (m_pAuroraMgr->GetMaintControlMgr()->GetCurPos(AXIS_Z2) > 0)
		{
			nRet = m_pAuroraMgr->GetMaintControlMgr()->MoveCmdPosA(AXIS_Z2, 0, false, SPEED_HIGH);
		}
	}

	// LYS 2026-08-24 Rotate first
	if (nRet == VTM_RV_SUCCESSED)
	{
		if (fabs(dStageR - dCurR) > 0.0001)
		{
			nRet = m_pAuroraMgr->GetMaintControlMgr()->MoveCmdPosA(AXIS_LOADER_R, dStageR, true, SPEED_HIGH);
		}
	}

	if (nRet == VTM_RV_SUCCESSED)
	{
		nRet = m_pAuroraMgr->GetMaintControlMgr()->MoveCmdPosA(AXIS_X, dStageY, true, SPEED_HIGH);
	}

	return nRet;

	//CMDMOVE stMove;
	//MEASGROUP	stMeasGroup;
	//ZeroMemory(&stMeasGroup, sizeof(stMeasGroup));
	//ZeroMemory(&stMove, sizeof(stMove));
	////stMeasGroup = m_pDlgNaviRecipe->GetMeasGroupStick(m_iNumStick, m_iStep);
	//
	//if (stMeasGroup.nGroup < 1)
	//	return VTM_RV_FAILED;
	//
	//double dCardW = 0.;
	////dCardW = m_pDlgNaviRecipe->m_stRecipeMeas.dTmp[1];	// Probe Pin 과 Bottom Align CCD 간격
	//
	//// TOP
	//stMove.stAxis[AXIS_X].bUse = TRUE;
	//stMove.stAxis[AXIS_X].dPos = stMeasGroup.stProbe[0].stAxis[AXIS_X].dPos - dCardW;
	////stMove.dMoveUserOffset[AXIS_X] = m_pDlgNaviRecipe->m_stRecipeMeas.dCentactZ[11];
	//
	//stMove.stAxis[AXIS_Y].bUse = TRUE;
	//stMove.stAxis[AXIS_Y].dPos = stMeasGroup.stProbe[0].stAxis[AXIS_Y].dPos;
	////stMove.dMoveUserOffset[AXIS_Y] = m_pDlgNaviRecipe->m_stRecipeMeas.dCentactZ[12];
	//
	//stMove.stAxis[AXIS_R].bUse = TRUE;
	//stMove.stAxis[AXIS_R].dPos = stMeasGroup.stProbe[0].stAxis[AXIS_R].dPos;
	////stMove.dMoveUserOffset[AXIS_R] = m_pDlgNaviRecipe->m_stRecipeMeas.dCentactZ[13];
	//
	////m_pDlgNaviMaint->m_dlgNaviMaintMotionACS.SetMeasIndex(1);	// Review
	////m_pDlgNaviMaint->m_dlgNaviMaintMotionACS.Cmd_MoveXYZ(stMove, TRUE);

	return VTM_RV_SUCCESSED;
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasPreStickMove()
{
	m_iStep = enumStepID_MOVE_STICK_PRE;
	DoWorkManual();

	/*m_iStep = 0;

	int iNumStick = 0, iRet = VTM_RV_SUCCESSED;
	iNumStick = m_iNumStick;

	m_iNumStick--;
	iRet = ManualMoveStick();
	if (iRet != VTM_RV_SUCCESSED)
	{
		m_iNumStick = iNumStick;
	}*/
}


void CDlgSubManual_Meas::OnBnClickedBtnManualGlassRotate1()
{
	m_iStep = 16;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualGlassRotate2()
{
	m_iStep = 26;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasNextCellMove3()
{
	m_iStep = enumStepID_MANUAL_CMD_01;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasNextCellMove4()
{
	m_iStep = enumStepID_MANUAL_CMD_02;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasChange()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_MANUAL_CMD_05;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualReadyChange()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_MANUAL_CMD_06;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnLpDown()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_MANUAL_CMD_03;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnLpUp()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_MANUAL_CMD_04;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasPgCmd01()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_PG_CMD_01;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasPgCmd2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_PG_CMD_02;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasPgCmd3()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_PG_CMD_03;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasPgCmd4()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_PG_CMD_04;
	DoWorkManual();
}

UINT CDlgSubManual_Meas::ThreadRun(LPVOID pParam)
{
	CDlgSubManual_Meas* pThis = (CDlgSubManual_Meas*)pParam;

	pThis->m_bThreadLive = true;

	while (1)
	{
		DWORD dw = WaitForMultipleObjects(2, pThis->m_hMultiEvent, FALSE, INFINITE);

		if (dw == WAIT_OBJECT_0) // Start Event 1가 set될 때까지 기다린다.
		{
			// LYS 2026-10-13 Hybrid Phase 7 (14)-C 3-state lock: take it here (0 -> 1; 1 = DoWorkManual marked it already), wait while a UI segment
			//   switch holds 2. The auto-reset start event queues a second run (double click) - it takes the lock again here. 1 segment: nobody takes 2
			while (InterlockedCompareExchange(&pThis->m_nWorkBusy, 1, 0) == 2) Sleep(10);
			pThis->ThreadFunc();
			InterlockedExchange(&pThis->m_nWorkBusy, 0);	// LYS 2026-10-13 Hybrid Phase 7 every return of ThreadFunc
		}
		else if (dw == (WAIT_OBJECT_0 + 1)) // Close Event 가 set될 때까지 기다린다.
		{
			break;
		}
		else // timeout, error state
		{
		}
	}

	pThis->m_bThreadLive = false;

	return 0;
}

/// <summary>
/// Threads the function.
/// </summary>
/// <returns></returns>
UINT CDlgSubManual_Meas::ThreadFunc()
{
	UINT nRet = VTM_RV_SUCCESSED;

	// 2023.08.09 add Online Interlock
	if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_MachineMode() != df_HOST_OFFLINE)
	{
		CD_DisplayMessage dlg(_T("Onlien 모드 입니다. OFFLine 변경 후 사용하세요."));
		dlg.DoModal();

		return 0;
	}

	// 2023.08.09 add Process Interlock
	if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_ProcessStatus() != SPS_IDLE || CSequenceManager::GetInstance()->IsEngineCompleteAll() != true)
	{
		CD_DisplayMessage dlg(_T("프로세스 진행 중 입니다. 정지 후 사용하세요."));
		dlg.DoModal();

		return 0;
	}

	// LYS 2026-10-13 Hybrid Phase 7 D7-14 (b) multi-cycle gap (IsCycleState with the engine done and SPS IDLE): refused on a hybrid runtime recipe only
	if (CSystemParamShare::GetInstance()->IsCycleState() && IsHybridRuntime())
	{
		CString strCycLog;
		strCycLog.Format(_T("[HYBRID] Manual Meas step %d refused - cycle test running (IsCycleState)"), m_iStep);
		CLogWrapper::GetInstance()->KMLOG(WARN, strCycLog);
		CD_DisplayMessage dlg(_T("프로세스 진행 중 입니다. 정지 후 사용하세요."));
		dlg.DoModal();

		return 0;
	}

	m_pAuroraMgr->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_CYCLETEST);

	this->EnableWindow(FALSE);
	// LYS 2026-10-13 Hybrid Phase 7 manual motion gate (runtime recipe with > 1 segment only; 1 segment returns true, no log) - before the step
	//   case changes m_iNumStick. Then the manual cell ID check before a measure set (D4b-9, D7-12 (b): hybrid only, 1 segment returns true)
	int nGateStep = m_iStep;	// LYS 2026-10-13 Hybrid Phase 7 fix (impl review 4): the gate judges this value - the step switch below must run the same step
	if (CheckHybridGate(nGateStep) == false || (IsMeasSetStep(nGateStep) && ValidateManualCellId() == false))
	{
		this->EnableWindow(TRUE);
		m_pAuroraMgr->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_IDLE);
		return 0;
	}
	int nNowStep = m_iStep;
	if (nNowStep != nGateStep && IsHybridRuntime())	// hybrid only: m_iStep changed while the gate ran -> not run (1 segment: the switch reads m_iStep as before)
	{
		CString strStepLog;
		strStepLog.Format(_T("[HYBRID] Manual Meas step %d refused - step changed to %d while the gate ran"), nGateStep, nNowStep);
		RefuseHybrid(strStepLog, _T("수동 동작 선택이 바뀌었습니다.\n이번 동작은 하지 않습니다. 다시 실행하십시오."));
		this->EnableWindow(TRUE);
		m_pAuroraMgr->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_IDLE);
		return 0;
	}
	
	switch (m_iStep)
	{
	case enumStepID_RECIPE_OPEN:
	{
		bool bLoadFail = false;	// LYS 2026-10-13 Hybrid Phase 7 D4b-15 (b) / (14)-K hybrid recipe open refused (1 segment: always false)
		OnClickBtnManualMeasRecipeOpen();

	CString sFilter = _T("Recipe Files (*.rcp) | *.rcp | All Files(*.*) | *.* |");

	CString sFileName;
	sFileName.Format(_T("%s%s%s"), m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeFolderPath(), STR_DIR_RECIPE, _RCP_ALL_FILE_NAME);

//	CString sFileName = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeFolderPath() + STR_DIR_RECIPE, _RCP_ALL_FILE_NAME    _T("Recipe\\*.rcp");

	CFileDialog dlg(TRUE, _T("rcp"), sFileName, OFN_HIDEREADONLY | OFN_NOCHANGEDIR, sFilter);
	dlg.m_ofn.lpstrInitialDir = _T("Recipe");

	if (dlg.DoModal() == IDOK)
	{
		CString strRcpFile = dlg.GetFileTitle();

		if (AfxMessageBox(_T("Load This Recipe : ") + strRcpFile, MB_OKCANCEL) == IDOK)
		{
			// LYS 2026-10-13 Hybrid Phase 7 D4b-15 (b) / D7-11 (b) on a hybrid runtime recipe: 0 < OpenRecipe() also accepted JOB_RECIPE_NOT_EXIST (1013).
			//   Refuse when the .rcp load itself failed (load serial unchanged) or a hybrid .rcp came without its .alg ((14)-K). 1 segment: the original 0 < test
			C_OutRecipeFunc* pLoadFunc = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeFuncPtr();
			int nLoadSeq = (pLoadFunc != NULL) ? pLoadFunc->GetRcpLoadSeq() : 0;
			int nOpenRtn = m_pAuroraMgr->GetRecipeEditMgrPtr()->OpenRecipe(strRcpFile);
			bool bHybOpen = IsHybridRuntime();	// after the load: a failed load leaves the previous recipe (and its segment count) in place
			if (bHybOpen && pLoadFunc != NULL && strRcpFile.IsEmpty() != true && pLoadFunc->GetRcpLoadSeq() == nLoadSeq)
			{
				bLoadFail = true;	// m_bOpenRcp and the groups stay as they were
				CString strOpenLog;
				strOpenLog.Format(_T("[HYBRID] Manual Meas recipe open refused - %s could not be read (returned %d, load serial %d unchanged)"), (LPCTSTR)strRcpFile, nOpenRtn, nLoadSeq);
				CLogWrapper::GetInstance()->KMLOG(WARN, strOpenLog);
				AfxMessageBox(_T("레시피 [") + strRcpFile + _T("] 파일을 읽을 수 없습니다.\n파일이 없거나 손상되었습니다. (Cell ID 트레일러 등)\n\n이전 레시피를 그대로 사용합니다."), MB_ICONWARNING);
			}
			else if (bHybOpen && nOpenRtn == JOB_RECIPE_NOT_EXIST && pLoadFunc != NULL && pLoadFunc->GetSegmentLoopCount() > 1)
			{
				bLoadFail = true;	// (14)-K the DLL holds the new hybrid recipe without its .alg - G0-b refuses every gated step until it is reopened
				CString strOpenLog;
				strOpenLog.Format(_T("[HYBRID] Manual Meas recipe open refused - %s returned %d (.alg load failed)"), (LPCTSTR)strRcpFile, nOpenRtn);
				CLogWrapper::GetInstance()->KMLOG(WARN, strOpenLog);
				AfxMessageBox(_T("레시피 [") + strRcpFile + _T("]의 Align 파일(.alg)을 읽지 못했습니다.\n하이브리드 레시피는 Align 파일 없이 열지 않습니다.\n\nAlign 파일을 확인한 뒤 다시 여십시오."), MB_ICONWARNING);
			}
			else if (0 < nOpenRtn)
			{
				m_bOpenRcp = true;
			}
			else
			{
				AfxMessageBox(_T("Fail : Load"));
			}
		}
	}

	if(m_bOpenRcp == true && bLoadFail == false)	// LYS 2026-10-13 Hybrid Phase 7 bLoadFail: hybrid open refused above (1 segment: always false)
	{
		UPdateTestBtn(true);

		// LYS 2026-10-13 Hybrid Phase 7 a newly loaded hybrid recipe (synced cache and live count > 1, view stamp stale) starts at stick 1 / cell 1 -
		//   segments differ in size (W7/W8); a cancelled dialog keeps the stick. 1 segment keeps the current stick as before.
		//   D7-20: the PG device model is unknown after a recipe open (1 segment: string only, never read)
		bool bHybView = (m_pAuroraMgr->GetRecipeEditMgrPtr()->GetSegCountCache() > 1 && m_pAuroraMgr->GetRecipeEditMgrPtr()->GetSegmentLoopCount() > 1);
		bool bHybReset = (bHybView && IsHybridViewStale());
		m_strPgModelSent.Empty();
		if (bHybReset)
		{
			m_iNumStick = 0;
			m_iNumCell = 0;
		}
		// LYS 2026-10-13 manual meas = CycleTest positions (review fix #9): nothing loaded (dialog cancelled or load failed - load serial unchanged) while
		//   the shared groups of this stick are held -> keep them and m_iNumCell: the heads still stand at m_vecMeasGroup[m_iNumCell], which the measure set /
		//   test steps use (legacy rebuilt the same legacy groups here and kept the index). The inner IDs are reset as that rebuild did. A loaded recipe,
		//   a hybrid view reset, another stick or legacy groups -> MakeMeasGroup as before
		if (bHybReset == false && m_nMeasGroupMode >= 0 && IsMeasGroupCurrent(m_nMeasGroupMode))
		{
			SetInnerIdArea(m_nMeasureArea, false);
			CString strKeepLog;
			strKeepLog.Format(_T("[MANUAL] Manual Meas recipe open: no recipe loaded - %s groups of stick %d kept (group %d of %d)"),
				(m_nMeasGroupMode == M_TYPE_SPEC) ? _T("WAD") : ((m_nMeasGroupMode == M_TYPE_ANGLE) ? _T("ANGLE") : _T("VISION")), m_iNumStick + 1, m_iNumCell + 1, (int)m_vecMeasGroup.size());
			CLogWrapper::GetInstance()->KMLOG(SYST, strKeepLog);
		}
		else
		MakeMeasGroup();
		if (bHybView)
		{
			StampHybridView();	// LYS 2026-10-13 Hybrid Phase 7 G0 view stamp (1 segment: none)
			UpdateCurStickUI();
		}
	}

	//	// File Open
	//	m_pAuroraMgr->GetRecipeEditMgrPtr()->OpenRecipe(_T("KR1_DM0"));

	////	if (LoadRecipeDlg() == TRUE)
	//		m_bOpenRcp = true;
	}
	break;

	// Glass Get 
	case enumStepID_GLASS_GET_01:
	{
		// LYS 2026-10-13 manual meas review #6: OCP raise Sub Z / FRONT Z before the stage / probe moves (none below safety = no log)
		nRet = RetractHeadZBeforeStep(_T("GLASS GET (loader position)"));
		if (nRet != VTM_RV_SUCCESSED) break;
		nRet = MoveLoaderXYR(0);
	}
	break;
	case enumStepID_GLASS_GET_02: // Clamp Up
	{
		nRet = m_pAuroraMgr->GetMaintControlMgr()->Do_Clamp_Up();
	}
	break;
	case enumStepID_GLASS_GET_03:
	{
		nRet = m_pAuroraMgr->GetMaintControlMgr()->DoWork_LPMove(false);
	}
	break;
	case enumStepID_GLASS_GET_04:
	{
		nRet = m_pAuroraMgr->GetMaintControlMgr()->Do_StageAir_On();
	}
	break;
	case enumStepID_GLASS_GET_05:
	{
		nRet = m_pAuroraMgr->GetMaintControlMgr()->Do_Clamp_In();
	}
	break;
	case enumStepID_GLASS_GET_06:
	{
		nRet = m_pAuroraMgr->GetMaintControlMgr()->Do_StageAir_Off();
	}
	break;
	case enumStepID_GLASS_GET_07:
	{
		nRet = m_pAuroraMgr->GetMaintControlMgr()->Do_StageVac_On();
	}
	break;
	case enumStepID_GLASS_GET_08:
	{
		nRet = m_pAuroraMgr->GetMaintControlMgr()->Do_Clamp_Out();
	}
	break;
	case enumStepID_GLASS_GET_09:
	{
		nRet = m_pAuroraMgr->GetMaintControlMgr()->Do_Clamp_Down();
	}
	break;

	// Glass Put
	case enumStepID_GLASS_PUT_01:
	{
		// LYS 2026-10-13 manual meas review #6: OCP raise Sub Z / FRONT Z before the stage / probe moves (none below safety = no log)
		nRet = RetractHeadZBeforeStep(_T("GLASS PUT (unloader position)"));
		if (nRet != VTM_RV_SUCCESSED) break;
		nRet = MoveLoaderXYR(1);
	}
	break;
	case enumStepID_GLASS_PUT_02:
	{
		nRet = m_pAuroraMgr->GetMaintControlMgr()->Do_Clamp_Up();
	}
	break;
	case enumStepID_GLASS_PUT_03:
	{
		nRet = m_pAuroraMgr->GetMaintControlMgr()->Do_StageVac_Off();
	}
	break;
	case enumStepID_GLASS_PUT_04:
	{
		nRet = m_pAuroraMgr->GetMaintControlMgr()->Do_StageAir_On();
	}
	break;
	case enumStepID_GLASS_PUT_05:
	{
		nRet = m_pAuroraMgr->GetMaintControlMgr()->Do_Clamp_In();
	}
	break;
	case enumStepID_GLASS_PUT_06:
	{
		nRet = m_pAuroraMgr->GetMaintControlMgr()->Do_StageAir_Off();
	}
	break;
	case enumStepID_GLASS_PUT_07:
	{
		nRet = m_pAuroraMgr->GetMaintControlMgr()->Do_Clamp_Out();
	}
	break;
	case enumStepID_GLASS_PUT_08:
	{
		nRet = m_pAuroraMgr->GetMaintControlMgr()->DoWork_LPMove(true);
	}
	break;

	// Align
	case enumStepID_GLASS_ALIGN:
	{
		// LYS 2026-10-13 manual meas review #6: OCP raise Sub Z / FRONT Z before the stage / probe moves (none below safety = no log)
		nRet = RetractHeadZBeforeStep(_T("GLASS ALIGN"));
		if (nRet != VTM_RV_SUCCESSED) break;
		bool bRet = false;
		CDoWorkGlassSequence* pDoWorkGlassSeq = CSequenceManager::GetInstance()->GeDoWorkGlassSequence();
		bRet = pDoWorkGlassSeq->ItemStart(_T("SEQUENCE_ITEM_DWGLASS_SEQ_GLASS_SW_ALIGN"), true);
		if (bRet != true)
		{
			nRet = JOB_PROC_ALIGN_ERROR;
		}
	}
	break;
	// jsyoon 2022.08.17 add
	case enumStepID_PROBE_ALIGN:
	{
		// LYS 2026-10-13 manual meas review #6: OCP raise Sub Z / FRONT Z before the stage / probe moves (none below safety = no log)
		nRet = RetractHeadZBeforeStep(_T("PROBE ALIGN"));
		if (nRet != VTM_RV_SUCCESSED) break;
		bool bRet = false;
		CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->SetProbe(df_PROBE_MEAS);

		CDoWorkGlassSequence* pDoWorkGlassSeq = CSequenceManager::GetInstance()->GeDoWorkGlassSequence();
		bRet = pDoWorkGlassSeq->ItemStart(_T("SEQUENCE_ITEM_DWGLASS_SEQ_PROBE_SW_ALIGN"), true);
		if (bRet != true)
		{
			nRet = JOB_PROC_ALIGN_ERROR;
		}
	}
	break;

	case enumStepID_PG_CMD_01:
	{
		m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PGEvent_5);
	}
	break;
	case enumStepID_PG_CMD_02:
	{
		m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PGEvent_6);
	}
	break;
	case enumStepID_PG_CMD_03:
	{
		m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PGEvent_7);
	}
	break;
	case enumStepID_PG_CMD_04:
	{
		m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PGEvent_8);
	}
	break;
	case enumStepID_PG_CMD_05:
	{
		// Turn On
		m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PGEvent_5);

		Sleep(1000);

		for (size_t i = 0; i < 3; i++)
		{
			m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PGEvent_6);

			Sleep(1000);
		}

		// Turn Off
		m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PGEvent_8);
	}
	break;


	case enumStepID_MANUAL_CMD_01:
	{
		nRet = MoveLoaderXYR(0);
	}
	break;
	case enumStepID_MANUAL_CMD_02:
	{
		nRet = MoveLoaderXYR(1);
	}
	break;
	case enumStepID_MANUAL_CMD_03:
	{
		nRet = m_pAuroraMgr->GetMaintControlMgr()->DoWork_LPMove(false);
	}
	break;
	case enumStepID_MANUAL_CMD_04:
	{
		nRet = m_pAuroraMgr->GetMaintControlMgr()->DoWork_LPMove(true);
	}
	break;
	case enumStepID_MANUAL_CMD_05:		// MEAS
	{
		bool bRet = false;
		int nStatus = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->GetEQState();
		if (nStatus == enumEQStateReady)
		{
			CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->SetEQStatusChangeStep(enumEQSTATUS_R2M);
			CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Start();
		}
		else if (nStatus == enumEQStateLoad)
		{
			CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->SetEQStatusChangeStep(enumEQSTATUS_L2M);
			CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Start();
		}

		if (nStatus == enumEQStateReady || nStatus == enumEQStateLoad)
		{
			Sleep(500); // jsyoon 2022.06.16 add

			int nLoop = 0;
			bool bRet = false;
			nRet = VTM_RV_FAILED;
			do
			{
				// jsyoon 2022.06.16 change 500 to 100
				Sleep(100);	

				bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Is_Complete();
				if (bRet == true)
				{
					nRet = VTM_RV_SUCCESSED;
					break;
				}

				nLoop++;

			} while (nLoop < 1000 /*200*/);
		}
	}
	break;
	case enumStepID_MANUAL_CMD_06:		// READY
	{
		bool bRet = false;
		int nStatus = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->GetEQState();
		if (nStatus == enumEQStateLoad)
		{
			CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->SetEQStatusChangeStep(enumEQSTATUS_L2R);
			CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Start();
		}
		else if (nStatus == enumEQStateMeas)
		{
			CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->SetEQStatusChangeStep(enumEQSTATUS_M2R);
			CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Start();
		}

		if (nStatus == enumEQStateLoad || nStatus == enumEQStateMeas)
		{
			Sleep(500);

			int nLoop = 0;
			bool bRet = false;
			nRet = VTM_RV_FAILED;
			do
			{
				// jsyoon 2022.06.16 change 500 to 100
				Sleep(100);

				bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Is_Complete();
				if (bRet == true)
				{
					nRet = VTM_RV_SUCCESSED;
					break;
				}

				nLoop++;

			} while (nLoop < 1000/*200*/);
		}
	}
	break;
	case enumStepID_MANUAL_CMD_07:
	{
		//if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsSafetySensorOn() != true)
		{
			CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->SetEQStatusChangeStep(enumEQSTATUS_SAFETY);
			CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Start();

			Sleep(500);

			int nLoop = 0;
			bool bRet = false;
			nRet = JOB_PROC_SAFETY_POS_ERROR;
			do
			{
				//	Sleep(500);
				// jsyoon 2022.06.16 change 500 to 100
				Sleep(100);

				bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Is_Complete();
				if (bRet == true)
				{
					nRet = VTM_RV_SUCCESSED;
					break;
				}
					

				if (CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Is_Fail() == true)
				{
					nRet = CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->GetFailNum();
					break;
				}

				nLoop++;

			} while (nLoop < 1000 /*200*/);
		}
	}
	break;
	// Test Code
	case enumStepID_MANUAL_CMD_08:
	{
		//nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TP_1);  
		//
		//if(nRet != VTM_RV_SUCCESSED)
		//{
		//	nRet = VTM_RV_SUCCESSED;
		//}

	//	if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_XIONIZER_REMOTE_STATE_01, 1, 1, true, PLC_IO_ACTION_DONE_TIME_03) == true)
	////	if( false )
	//	{
	//		m_pAuroraMgr->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_XIONIZER_REMOTE_ON_01, 0, 1);
	//	}
	//	else
	//	{
	//		m_pAuroraMgr->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_XIONIZER_REMOTE_ON_01, 1, 1);
	//	}

	//	CSequenceManager::GetInstance()->GetFunctionModuleEQState()->WaitActionDone(PLC_BIT_XIONIZER_REMOTE_STATE_01, 1, 1, false, PLC_IO_ACTION_DONE_TIME_60); 

		//RSRITEMDATA resultData;
		//ZeroMemory(&resultData, sizeof(resultData));

		//resultData.nMeasMode = PGM_MODE_ANGLE;
		//resultData.iMeasArea = 1;

		//CString Temp;
		//Temp = _T("DDDDD");
		//_tcscpy(resultData.szGlassID, Temp); 
		//Temp = _T("1144026.ini");
		//_tcscpy(resultData.szResultFile, Temp);
		//Temp = _T("D:\TEST\\");
		//_tcscpy(resultData.szResultPath, Temp);

		//CResultMgr::GetInstance()->Updata_Data(&resultData);


	//	nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkRecipeOpen(enumTCPIpFactorytID_PG);

		//m_pAuroraMgr->GetMaintControlMgr()->DoWork_Probe_Zero(df_PROBE_AGING4, false);
	//	m_pAuroraMgr->GetMaintControlMgr()->DoWork_Probe_Zero(df_PROBE_MEAS, false);

		//TCHAR sz[100];
		//ZeroMemory(&sz, sizeof(sz));

		//nRet = m_pAuroraMgr->GetMaintControlMgr()->GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2110"), _T("TEST1"), 10);
		//Sleep(200);

		//nRet = m_pAuroraMgr->GetMaintControlMgr()->GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2120"), _T("TEST2"), 10);
		//Sleep(200);

		//nRet = m_pAuroraMgr->GetMaintControlMgr()->GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2130"), _T("TEST3"), 10);
		//Sleep(200);

		//nRet = m_pAuroraMgr->GetMaintControlMgr()->GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2140"), _T("TEST4"), 10);
		//Sleep(200);

		//nRet = m_pAuroraMgr->GetMaintControlMgr()->GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2150"), _T("TEST5"), 10);
		//Sleep(200);

	//	m_pAuroraMgr->GetMaintControlMgr()->GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2110"), _T("TEST1"), 10); Sleep(200);
		/*m_pAuroraMgr->GetMaintControlMgr()->GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2110"), sz, 10);	Sleep(200);

		m_pAuroraMgr->GetMaintControlMgr()->GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2120"), sz, 10);	Sleep(200);

		m_pAuroraMgr->GetMaintControlMgr()->GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2130"), sz, 10);	Sleep(200);

		m_pAuroraMgr->GetMaintControlMgr()->GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2140"), sz, 10);	Sleep(200);

		m_pAuroraMgr->GetMaintControlMgr()->GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2150"), _T("MEASURE"), 10);	Sleep(200);*/


	//	Sleep(200);

	//	m_pAuroraMgr->GetMaintControlMgr()->GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2140"), sz, 5);

	//	Sleep(200);
	//
	//	m_pAuroraMgr->GetMaintControlMgr()->GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2100"), _T("21ABCDEDFH"), 10);

	////	Sleep(200);

	////	m_pAuroraMgr->GetMaintControlMgr()->GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2120"), _T(""), 5);
	////	Sleep(200);

	////	m_pAuroraMgr->GetMaintControlMgr()->GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2130"), _T("31ABCDED12"), 5);

	//	m_pAuroraMgr->GetMaintControlMgr()->GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2150"), _T("31ABCDED12"), 5);


	//	m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectEnd(enumTCPIpFactorytID_CIM);

	}
	break;
	case enumStepID_MANUAL_CMD_09:
	{
		//m_pAuroraMgr->GetMaintControlMgr()->Do_PROBE_CLAM_Lock(df_PROBE_MEAS);

		//m_pAuroraMgr->GetMaintControlMgr()->DoWork_Probe_Clam(df_PROBE_AGING4, true);
		nRet = m_pAuroraMgr->GetMaintControlMgr()->DoWork_Probe_Clam(df_PROBE_MEAS, true);
		
		
	}
	break;
	case enumStepID_MANUAL_CMD_10:
	{
		//m_pAuroraMgr->GetMaintControlMgr()->DoWork_Probe_PcbLock(df_PROBE_AGING4, true);
		nRet = m_pAuroraMgr->GetMaintControlMgr()->DoWork_Probe_PcbLock(df_PROBE_MEAS, true);
	}
	break;
	case enumStepID_MANUAL_CMD_11:
	{
		//m_pAuroraMgr->GetMaintControlMgr()->DoWork_Probe_PcbLock(df_PROBE_AGING4, false);
		nRet = m_pAuroraMgr->GetMaintControlMgr()->DoWork_Probe_PcbLock(df_PROBE_MEAS, false);
	}
	break;
	case enumStepID_MANUAL_CMD_12:
	{
		//m_pAuroraMgr->GetMaintControlMgr()->Do_PROBE_CLAM_Unlock(df_PROBE_MEAS);
		//m_pAuroraMgr->GetMaintControlMgr()->DoWork_Probe_Clam(df_PROBE_AGING4, false);
		nRet = m_pAuroraMgr->GetMaintControlMgr()->DoWork_Probe_Clam(df_PROBE_MEAS, false);
	}
	break;
	case enumStepID_MANUAL_CMD_13:
	{
	//	m_pAuroraMgr->GetMaintControlMgr()->DoWork_Probe_Zero(df_PROBE_MEAS, true);
		//m_pAuroraMgr->GetMaintControlMgr()->DoWork_Probe_Zero(df_PROBE_AGING4, true);
		nRet = m_pAuroraMgr->GetMaintControlMgr()->DoWork_Probe_Zero(df_PROBE_MEAS, true);
	}
	break;
	case enumStepID_MANUAL_SHUTTER:
	{
		nRet = m_pAuroraMgr->GetMaintControlMgr()->Do_ShutterOnOff();
	}
	break;
	case enumStepID_AGING_TEST:
	{
		S_RecipeMgr* pslData = nullptr;
		pslData = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeMgrPtr();
		int nCellCnt = 0;
		nCellCnt = pslData->m_nCellCnt;

		UINT nTimeOut = DEFINE_AGING_WAIT_TIME;
		S_RecipeInfo* pReciepInfo;
		pReciepInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();
		if (pReciepInfo != nullptr)
		{
			nTimeOut = pReciepInfo->m_sInspectItem.m_nTimeOut[2];
		}

		if (nCellCnt > 10)
		{
			// OOD
			if (m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_AGING, 0, 0) == VTM_RV_SUCCESSED)
			{
				nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectComplate(enumTCPIpFactorytID_AGING, 0, nTimeOut);
			}

			// EVEN
			if (nRet == VTM_RV_SUCCESSED)
			{
				if (m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_AGING, 1, 0) == VTM_RV_SUCCESSED)
				{
					nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectComplate(enumTCPIpFactorytID_AGING, 0, nTimeOut);
				}
			}
		}
		else
		{
			if (m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_AGING, 0, 0) == VTM_RV_SUCCESSED)
			{
				m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectComplate(enumTCPIpFactorytID_AGING, 0, nTimeOut);
			}
		}


//		CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_AGING_Start, NULL);

	//	if (m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_AGING, 0, 0) == VTM_RV_SUCCESSED)
		{
			;
		//	this->EnableWindow(TRUE);

		//	m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectComplate(enumTCPIpFactorytID_AGING, 0, DEFINE_AGING_WAIT_TIME);
		}

//		CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_AGING_End, NULL);
	}
	break;
	case enumStepID_MEAS_TEST_WAD:
	{
		CString strInnerID;

		for (size_t iMeasType = 0; iMeasType < 2; iMeasType++)
		{
			strInnerID.Format(_T("%d%s"), 1 + iMeasType, gf_GetCurTimeString(3));
			_tcscpy_s(m_pAuroraMgr->GetMaintControlMgr()->GetShareMemoryMgrPtr()->GetSharedMemory()->stLotInfo.m_szInnerID[iMeasType], 32, strInnerID);
		}

		int nTimeOut = DEFINE_RECIPE_WAIT_TIME;

		S_RecipeInfo* pReciepInfo;
		pReciepInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();
		if (pReciepInfo != nullptr)
		{
			nTimeOut = pReciepInfo->m_sInspectItem.m_nTimeOut[3];
		}

		// jsyoon 2022.09.27 add
		bool bMeasProbe[df_MAX_PROBE];
		ZeroMemory(bMeasProbe, sizeof(bMeasProbe));

		bMeasProbe[0] = true;
		bMeasProbe[1] = true;

		int nUseCELLID[df_MAX_PROBE];
		ZeroMemory(nUseCELLID, sizeof(nUseCELLID));

		int iMeasMode = 0;
		if (m_iNumCell >= 0)
		{
			nUseCELLID[0] = m_vecMeasGroup[m_iNumCell].m_sCellInfo[0].m_nArea;
			nUseCELLID[1] = m_vecMeasGroup[m_iNumCell].m_sCellInfo[1].m_nArea;

			if (nUseCELLID[0] < 0) bMeasProbe[0] = false;
			if (nUseCELLID[1] < 0) bMeasProbe[1] = false;
		}


		if (bMeasProbe[0] == true && bMeasProbe[1] != true)
		{
			iMeasMode = 1;
		}
		else if (bMeasProbe[0] != true && bMeasProbe[1] == true)
		{
			iMeasMode = 2;
		}
		else
		{
			iMeasMode = 0;
		}
		///

		//2023.06.12 (Right Test용)
		BOOL bRight = FALSE;
		bRight = ((CButton*)GetDlgItem(IDC_CHECK_1))->GetCheck();
		if (bRight) {
			iMeasMode = 2;
		}

		//if (m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_WAD, iMeasMode) == VTM_RV_SUCCESSED)
		if (m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_WAD, iMeasMode, 0, true, bRight) == VTM_RV_SUCCESSED)
		{
			m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectComplate(enumTCPIpFactorytID_WAD, iMeasMode, nTimeOut); //30ms
		}
	}
	break;
	case enumStepID_MEAS_TEST_VISION:
	{
		CString strInnerID;

		m_pAuroraMgr->GetMaintControlMgr()->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_BIT_W_GLASS_SENSOR_POWER_ON, true);	// jsyoon 2022.08.17 add Glass Sensor Off
		Sleep(1000);

		for (size_t iMeasType = 0; iMeasType < 2; iMeasType++)
		{
			strInnerID.Format(_T("%d%s"), 1 + iMeasType, gf_GetCurTimeString(3));
			_tcscpy_s(m_pAuroraMgr->GetMaintControlMgr()->GetShareMemoryMgrPtr()->GetSharedMemory()->stLotInfo.m_szInnerID[iMeasType], 32, strInnerID);
		}

		int nTimeOut = DEFINE_RECIPE_WAIT_TIME;

		S_RecipeInfo* pReciepInfo;
		pReciepInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();
		if (pReciepInfo != nullptr)
		{
			nTimeOut = pReciepInfo->m_sInspectItem.m_nTimeOut[4];
		}

		// jsyoon 2022.09.27 add
		bool bMeasProbe[df_MAX_PROBE];
		ZeroMemory(bMeasProbe, sizeof(bMeasProbe));

		bMeasProbe[0] = true;
		bMeasProbe[1] = true;

		int nUseCELLID[df_MAX_PROBE];
		ZeroMemory(nUseCELLID, sizeof(nUseCELLID));

		// 2023.05.09 add
		int nMeasArea = 0;
		nMeasArea = m_vecMeasGroup.size();

		int iMeasMode = 0;
		if (m_iNumCell >= 0) 
		{
			nUseCELLID[0] = m_vecMeasGroup[m_iNumCell].m_sCellInfo[0].m_nArea;
			nUseCELLID[1] = m_vecMeasGroup[m_iNumCell].m_sCellInfo[1].m_nArea;

			if (nMeasArea > 1)	// 2023.05.09 add
			{
				if (nUseCELLID[0] < 0) bMeasProbe[0] = false;
				if (nUseCELLID[1] < 0) bMeasProbe[1] = false;
			}
			else //Cell 1개 일때?? / Cell 1개를 화질 두개로 동시 측정 할때 innerid 동일 값으로 전달
			{
				strInnerID.Format(_T("%d%s"), 1, gf_GetCurTimeString(3));
				for (int i = 0; i < 2; i++) _tcscpy_s(m_pAuroraMgr->GetMaintControlMgr()->GetShareMemoryMgrPtr()->GetSharedMemory()->stLotInfo.m_szInnerID[i], 32, strInnerID);
			}
		}

		if (bMeasProbe[0] == true && bMeasProbe[1] != true)
		{
			iMeasMode = 1;
		}
		else if (bMeasProbe[0] != true && bMeasProbe[1] == true)
		{
			iMeasMode = 2;
		}
		else
		{
			iMeasMode = 0;
		}
		///

		m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkVisionCmd(EXE_QUALITY_RESET);			 //화질 자동검사 시작시 초기화
		m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkVisionCmd(EXE_QUALITY_WAIT_USE, FALSE); //화질 자동결과 설정

		// JCY 2023.06.23 화질 + 면계측 동시 측정 
		bool bCheck = m_pAuroraMgr->GetSystemConfigMgrPtr()->IsUseInspVisionColor();

//		if (FALSE)
		{
			if (bCheck)
			{
				// Color Start -> Quality Start -> Color End -> Quality End
				nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_COLOR, iMeasMode, 0);
				if (nRet == VTM_RV_SUCCESSED)
				{
					nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkVisionCmd(EXE_QUALITY_GRAB);
				}

				if (nRet == VTM_RV_SUCCESSED)
				{
					nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectEnd(enumTCPIpFactorytID_COLOR, iMeasMode, nTimeOut);
				}

				if (nRet == VTM_RV_SUCCESSED)
				{
					nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkVisionCmd(EXE_QUALITY_GRAB_WAIT);
				}
			}
			else
			{
				nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkVisionCmd(EXE_QUALITY_GRAB);

				if (nRet == VTM_RV_SUCCESSED)
				{
					nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkVisionCmd(EXE_QUALITY_GRAB_WAIT);
				}
			}
		}
	
		
		
		
		//nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkVisionCmd(EXE_QUALITY_GRAB);
//
//		if (nRet == VTM_RV_SUCCESSED)
//		{
//			nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkVisionCmd(EXE_QUALITY_GRAB_WAIT);
//		}

////		m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_VISION, 0);
//		// jsyoon 2022.08.04 change
//		if (m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_VISION, iMeasMode) == VTM_RV_SUCCESSED)
//		{
//			m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectEnd(enumTCPIpFactorytID_VISION, iMeasMode, nTimeOut);
//		}

		m_pAuroraMgr->GetMaintControlMgr()->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_BIT_W_GLASS_SENSOR_POWER_ON, false);	// jsyoon 2022.08.17 add Glass Sensor On
	}
	break;
	case enumStepID_MEAS_TEST_COLOR:
	{
		CString strInnerID;

		for (size_t iMeasType = 0; iMeasType < 2; iMeasType++)
		{
			strInnerID.Format(_T("%d%s"), 1 + iMeasType, gf_GetCurTimeString(3));
			_tcscpy_s(m_pAuroraMgr->GetMaintControlMgr()->GetShareMemoryMgrPtr()->GetSharedMemory()->stLotInfo.m_szInnerID[iMeasType], 32, strInnerID);
		}

		int nTimeOut = DEFINE_RECIPE_WAIT_TIME;

		S_RecipeInfo* pReciepInfo;
		pReciepInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();
		if (pReciepInfo != nullptr)
		{
			nTimeOut = pReciepInfo->m_sInspectItem.m_nTimeOut[5];
		}

		// jsyoon 2022.09.27 add
		bool bMeasProbe[df_MAX_PROBE];
		ZeroMemory(bMeasProbe, sizeof(bMeasProbe));

		bMeasProbe[0] = true;
		bMeasProbe[1] = true;

		int nUseCELLID[df_MAX_PROBE];
		ZeroMemory(nUseCELLID, sizeof(nUseCELLID));

		int iMeasMode = 0;

		int nMeasArea = 0;
		nMeasArea = m_vecMeasGroup.size();

		if (nMeasArea == 1) {
			//2023.05.22 (면계측 HEAD1, 2 이동 후 수동 설정 진행 bLeftHead 값에 따라 left, right 진행) // UI LEFT, RIGHT 선택 추가 후 진행
			BOOL bLeftHead = TRUE;// TRUE;
			int nRet = VTM_RV_SUCCESSED;

			if (bLeftHead) {
				nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TP_3); //수동 Zone
			}
			else {
				nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TP_4); //시야각 Zone
			}

			if (nRet != VTM_RV_SUCCESSED) return nRet;

			if (!bLeftHead) {
				bMeasProbe[0] = true;
				bMeasProbe[1] = true;

				nUseCELLID[0] = m_vecMeasGroup[m_iNumCell].m_sCellInfo[1].m_nArea;
				nUseCELLID[1] = m_vecMeasGroup[m_iNumCell].m_sCellInfo[0].m_nArea;

				if (nUseCELLID[0] < 0) bMeasProbe[0] = false;
				if (nUseCELLID[1] < 0) bMeasProbe[1] = false;
			}
			//
		}
		else {
			if (m_iNumCell >= 0)
			{
				nUseCELLID[0] = m_vecMeasGroup[m_iNumCell].m_sCellInfo[0].m_nArea;
				nUseCELLID[1] = m_vecMeasGroup[m_iNumCell].m_sCellInfo[1].m_nArea;

				if (nUseCELLID[0] < 0) bMeasProbe[0] = false;
				if (nUseCELLID[1] < 0) bMeasProbe[1] = false;
			}
		}

		if (bMeasProbe[0] == true && bMeasProbe[1] != true)
		{
			iMeasMode = 1;
		}
		else if (bMeasProbe[0] != true && bMeasProbe[1] == true)
		{
			iMeasMode = 2;
		}
		else
		{
			iMeasMode = 0;
		}
		///
	
//		m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_COLOR, 0);
		// jsyoon 2022.08.04 change
		if (m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_COLOR, iMeasMode) == VTM_RV_SUCCESSED)
		{
			m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectEnd(enumTCPIpFactorytID_COLOR, iMeasMode, nTimeOut);
		}
	}
	break;
	case enumStepID_MEAS_TEST_ANGLE:
	{
		CString strInnerID;

		for (size_t iMeasType = 0; iMeasType < 2; iMeasType++)
		{
			strInnerID.Format(_T("%d%s"), 1 + iMeasType, gf_GetCurTimeString(3));
			_tcscpy_s(m_pAuroraMgr->GetMaintControlMgr()->GetShareMemoryMgrPtr()->GetSharedMemory()->stLotInfo.m_szInnerID[iMeasType], 32, strInnerID);
		}

		int nTimeOut = DEFINE_RECIPE_WAIT_TIME;

		S_RecipeInfo* pReciepInfo;
		pReciepInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();
		if (pReciepInfo != nullptr)
		{
			nTimeOut = pReciepInfo->m_sInspectItem.m_nTimeOut[6];
		}

		// jsyoon 2022.09.27 add
		bool bMeasProbe[df_MAX_PROBE];
		ZeroMemory(bMeasProbe, sizeof(bMeasProbe));

		bMeasProbe[0] = true;
		bMeasProbe[1] = true;

		int nUseCELLID[df_MAX_PROBE];
		ZeroMemory(nUseCELLID, sizeof(nUseCELLID));

		int iMeasMode = 0;
		if (m_iNumCell >= 0)
		{
			nUseCELLID[0] = m_vecMeasGroup[m_iNumCell].m_sCellInfo[0].m_nArea;
			nUseCELLID[1] = m_vecMeasGroup[m_iNumCell].m_sCellInfo[1].m_nArea;

			if (nUseCELLID[0] < 0) bMeasProbe[0] = false;
			if (nUseCELLID[1] < 0) bMeasProbe[1] = false;
		}


		if (bMeasProbe[0] == true && bMeasProbe[1] != true)
		{
			iMeasMode = 1;
		}
		else if (bMeasProbe[0] != true && bMeasProbe[1] == true)
		{
			iMeasMode = 2;
		}
		else
		{
			iMeasMode = 0;
		}
		///

//		m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_ANGLE, 0);
		// jsyoon 2022.08.04 change
		if (m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_ANGLE, iMeasMode) == VTM_RV_SUCCESSED)
		{
			m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectEnd(enumTCPIpFactorytID_ANGLE, iMeasMode, nTimeOut);
		}
	}
	break;
	case enumStepID_MEAS_TEST_TSP:
	{
		// TP
		nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TP_5);

		if (nRet == VTM_RV_SUCCESSED)
		{
			// Init
			for (size_t i = 0; i < 16; i++)
			{
				_tcscpy_s(m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->stLotInfo.m_szCellID[i], 32, _T(""));	// CELL ID 설정

				_tcscpy_s(m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->stLotInfo.m_szInnerID[i], 32, _T(""));	// INNERID
			}

			UINT nTimeOut = DEFINE_AGING_WAIT_TIME;
			int nMeasArea = 0;
			nMeasArea = m_vecMeasGroup.size();
			if (nMeasArea > 0)
			{
				S_RecipeData* pRecipeData;
				pRecipeData = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeData();

				int nMaxStick = 0, nArea = 0;
				nMaxStick = pRecipeData->m_vecStick.size();	// jsyoon 2022.09.29 add
				nArea = pRecipeData->m_vecStick[0].m_vecCell.size();

				S_RecipeInfo* pReciepInfo;
				pReciepInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();

				// jsyoon 2022.09.29 change
				int iRotate = 0;
				if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
				{
					iRotate = 1;
				}

				nTimeOut = pReciepInfo->m_sInspectItem.m_nTimeOut[7];

				CString strCurTime;
				strCurTime.Format(_T("%s"), gf_GetCurTimeString(3));

				for (int i = 0; i < nArea; i++)
				{
					int nCellNum = i;

					// Generate CELL ID
					CString strTmp, strGlassID, strCellID, strMcrID, strInnerID;
					if (iRotate == 1) // YY, BT
					{
						strCellID.Format(_T("%c%02d"), 65 + nCellNum, m_iNumStick + 1);

					}
					else // DG
					{
						strCellID.Format(_T("%c%02d"), 65 + nMaxStick - m_iNumStick - 1, nCellNum + 1);
					}

					// MCRID 생성
					int nMaxTop = 0;
					if (iRotate == 1)
					{
						nMaxTop = pRecipeData->m_vecStick.size();
					}
					else
					{
						nMaxTop = pRecipeData->m_vecStick[0].m_vecCell.size();
					}

					// GlassID + CellID
					strTmp.Format(_T("%s "), strCellID);
					
					// Cell ID
					strGlassID.Format(_T("%s"), CSystemParamShare::GetInstance()->GetGlassID());

					// Glass Part A or B
					CString strPart;
					strPart.Format(_T("%s"), strGlassID.Mid(strGlassID.GetLength() - 2, 1));
					if (strPart.CompareNoCase(_T("B")) == 0)	// AB
					{
						CString strBuf;
						strBuf.Format(_T("%s%02d"), strTmp.Mid(0, 1), nMaxTop + _ttoi(strTmp.Mid(1, 2)));  // A01~A10
						strCellID.Format(_T("%s"), strBuf);
					}
					else										// AA
					{
						;
					}
					//////////////////////////////////////////////////////////////////////////

					strTmp.Format(_T("%s%s"), strGlassID.Left(strGlassID.GetLength() - 1), strCellID);
					strMcrID.Format(_T("%s"), gf_MakeMcrID(strTmp));

					// Inner ID 생성
					strInnerID.Format(_T("%d%s"), nCellNum + 1, strCurTime);
					
					if (i < 16)
					{
						_tcscpy_s(m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->stLotInfo.m_szCellID[i], 32, strMcrID);		// CELL ID 설정
						
						_tcscpy_s(m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->stLotInfo.m_szInnerID[i], 32, strInnerID);	// INNERID
					}
				}
			}

			// Start Measure
			m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_TSP, 0);

			// End Measure
			m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectComplate(enumTCPIpFactorytID_TSP, 0, nTimeOut);
		}
		else
		{
			nRet = VTM_RV_SUCCESSED;
		}
	}
	break;
	case enumStepID_MEAS_TEST_LCR:
	{
		m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_LCR, 0);
	}
	break;

	case enumStepID_PRE_TEST_01:
	{
		S_RecipeMgr* pslData = nullptr;
		pslData = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeMgrPtr();
		int nCellCnt = 0;
		nCellCnt = pslData->m_nCellCnt;

		UINT nTimeOut = DEFINE_AGING_WAIT_TIME;
		S_RecipeInfo* pReciepInfo;
		pReciepInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();
		if (pReciepInfo != nullptr)
		{
			nTimeOut = pReciepInfo->m_sInspectItem.m_nTimeOut[0];
		}

		m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4] = 0;

		if (nCellCnt > 10)
		{
			nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_1);
			if (nRet == VTM_RV_SUCCESSED)
			{
				Sleep(5000);
				nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_11); // EVEN
			}
		}
		else
		{
			nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_1);
		}

		if (nRet != VTM_RV_SUCCESSED)
		{
			nRet = VTM_RV_SUCCESSED;
		}

//		nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_1);
	}
	break;
	case enumStepID_PRE_TEST_02:
	{
		S_RecipeMgr* pslData = nullptr;
		pslData = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeMgrPtr();
		int nCellCnt = 0;
		nCellCnt = pslData->m_nCellCnt;

		UINT nTimeOut = DEFINE_AGING_WAIT_TIME;
		S_RecipeInfo* pReciepInfo;
		pReciepInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();
		if (pReciepInfo != nullptr)
		{
			nTimeOut = pReciepInfo->m_sInspectItem.m_nTimeOut[1];
		}

		m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4] = 0;

		if (nCellCnt > 10)
		{
			nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_2);
			if (nRet == VTM_RV_SUCCESSED)
			{
				nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_12); // EVEN
			}
		}
		else
		{
			nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_2);
		}

		if (nRet != VTM_RV_SUCCESSED)
		{
			nRet = VTM_RV_SUCCESSED;
		}

	//	nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_2);
	}
	break;
	case enumStepID_PRE_TEST_03:
	{
		S_RecipeMgr* pslData = nullptr;
		pslData = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeMgrPtr();
		int nCellCnt = 0;
		nCellCnt = pslData->m_nCellCnt;

		UINT nTimeOut = DEFINE_AGING_WAIT_TIME;
		S_RecipeInfo* pReciepInfo;
		pReciepInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();
		if (pReciepInfo != nullptr)
		{
			nTimeOut = pReciepInfo->m_sInspectItem.m_nTimeOut[1];
		}

		m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4] = 0;

		if (nCellCnt > 10)
		{
			nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_3);
			if (nRet == VTM_RV_SUCCESSED)
			{
				nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_13); // EVEN
			}
		}
		else
		{
			nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_3);
		}

		if (nRet != VTM_RV_SUCCESSED)
		{
			nRet = VTM_RV_SUCCESSED;
		}

	//	nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_3);
	}
	break;
	case enumStepID_PRE_TEST_04:
	{
		
		S_RecipeMgr* pslData = nullptr;
		pslData = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeMgrPtr();
		int nCellCnt = 0;
		nCellCnt = pslData->m_nCellCnt;


		UINT nTimeOut = DEFINE_AGING_WAIT_TIME;
		S_RecipeInfo* pReciepInfo;
		pReciepInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();
		if (pReciepInfo != nullptr)
		{
			nTimeOut = pReciepInfo->m_sInspectItem.m_nTimeOut[2];
		}
		
		int iAgingTime = 0;

		if (nCellCnt > 10)
		{
			nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_4);  // ODD
			if (nRet == VTM_RV_SUCCESSED)
			{
				iAgingTime = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->GetCommTcpIpObjectPG()->GetAgingTime();
				if (iAgingTime < 1)
					iAgingTime = 21;
				CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_AGING_Start, iAgingTime);

				nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectComplate(enumTCPIpFactorytID_PG, 8, nTimeOut); // EVEN
				
				CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_AGING_End, NULL);
			}

			if (nRet == VTM_RV_SUCCESSED)
			{
				nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_14); // EVEN
			}

			if (nRet == VTM_RV_SUCCESSED)
			{
					iAgingTime = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->GetCommTcpIpObjectPG()->GetAgingTime();
				if (iAgingTime < 1)
					iAgingTime = 21;
				CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_AGING_Start, iAgingTime);
				
				nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectComplate(enumTCPIpFactorytID_PG, 8, nTimeOut); // EVEN
				
				CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_AGING_End, NULL);
			}
		}
		else
		{
			nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_4);

			if (nRet == VTM_RV_SUCCESSED)
			{
				iAgingTime = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->GetCommTcpIpObjectPG()->GetAgingTime();
				if (iAgingTime < 1)
					iAgingTime = 21;
				CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_AGING_Start, iAgingTime);

				nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectComplate(enumTCPIpFactorytID_PG, 8, nTimeOut); // EVEN
				
				CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_AGING_End, NULL);
			}
		}

		if (nRet != VTM_RV_SUCCESSED)
		{
			nRet = VTM_RV_SUCCESSED;
		}
	}
	break;

	case enumStepID_MEAS_SET_WAD:
	{
		// TP
		nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TP_1);

		// CELL Num
		if (nRet == VTM_RV_SUCCESSED)
		{
			int nMeasArea = 0;
			nMeasArea = m_vecMeasGroup.size();

			int nUseCELLID[df_MAX_PROBE];
			ZeroMemory(nUseCELLID, sizeof(nUseCELLID));
			TCHAR cCellID[df_MAX_PROBE][32];
			ZeroMemory(cCellID, sizeof(cCellID));
			bool bMeasProbe[df_MAX_PROBE];
			ZeroMemory(bMeasProbe, sizeof(bMeasProbe));

			if (nMeasArea > 0)
			{
				S_RecipeData* pRecipeData;
				pRecipeData = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeData();
				int nMaxStick = 0, nArea = 0;
				nMaxStick = pRecipeData->m_vecStick.size();	// jsyoon 2022.09.29 add
				nArea = pRecipeData->m_vecStick[0].m_vecCell.size();
				for (size_t i = 0; i < nMeasArea; i++)
				{
					if (i != m_iNumCell)
					{
						continue;
					}

					nUseCELLID[0] = m_vecMeasGroup[i].m_sCellInfo[0].m_nArea;
					nUseCELLID[1] = m_vecMeasGroup[i].m_sCellInfo[1].m_nArea;

					int nCellNum = 0;
					CString strCellId;

					S_RecipeInfo* pReciepInfo;
					pReciepInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();

					// jsyoon 2022.09.29 change
					int iRotate = 0;
					if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
					{
						iRotate = 1;
					}

					for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
					{
						bMeasProbe[iProbe] = true;
					}

					int nMeasStick = 0;
					ZeroMemory(cCellID, sizeof(cCellID));
					nMeasStick = MakeCellID(nUseCELLID, bMeasProbe, *cCellID, iRotate, m_iNumStick, nMaxStick);
					///

					//for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
					//{
					//	bMeasProbe[iProbe] = true;
					//	nCellNum = nUseCELLID[iProbe];
					////	nCellNum = nMeasArea * iProbe + i;


					//	if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1) 
					//	{
					//		strCellId.Format(_T("%c%02d"), 65 + nCellNum, m_iNumStick + 1);
					//	}
					//	else
					//	{
					//		strCellId.Format(_T("%c%02d"), 65 + m_iNumStick, nArea - nCellNum);
					//	}
					////	strCellId.Format(_T("%c%02d"), 65 + nCellNum, m_iNumStick + 1);
					////	strCellId.Format(_T("%c%02d"), 65 + nCellNum, 5 - m_iNumStick);
					////	strCellId.Format(_T("%c%02d"), 65, 2 * iProbe + iProbe);
					////	strCellId.Format(_T("%c%02d"), 65, nMeasArea* iProbe + i + 1);
					//	_tcscpy_s(cCellID[iProbe], _countof(cCellID[iProbe]), strCellId);
					//}

					GenMeasID(nUseCELLID, bMeasProbe, *cCellID); 

					// Set PG Num
					// PG 번호 변경
					for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
					{
					//	if (nUseCELLID[iProbe] >= 0)
					//		m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, nUseCELLID[iProbe] + 1);

						if (nUseCELLID[iProbe] >= 0)
						{
							m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, nUseCELLID[iProbe] + 1);
						}
						else
						{
							if (iProbe == 0)
								m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, 1);
							else
								m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, nArea);
						}
					}

					int nAreaNum[DF_MAX_HEAD] = { 0, };
					// jsyoon 2022.09.29 change
					//nAreaNum[0] = nUseCELLID[0] + (nMeasArea * df_MAX_PROBE * m_iNumStick) + 1;
					//nAreaNum[1] = nUseCELLID[1] + (nMeasArea * df_MAX_PROBE * m_iNumStick) + 1;
					nAreaNum[0] = nUseCELLID[0] + (nArea * nMeasStick) + 1;
					nAreaNum[1] = nUseCELLID[1] + (nArea * nMeasStick) + 1;
					AddSegRowBase(nAreaNum);	// LYS 2026-10-13 Hybrid Phase 7 segBase of the active segment (1 segment: base 0, no-op)
					m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->SetMeasUseInfo(bMeasProbe, nAreaNum, 3000);
				}
			}
		}

		// Recipe
		if (nRet == VTM_RV_SUCCESSED)
		{
			nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkRecipeOpen(enumTCPIpFactorytID_WAD);
		}

		if (nRet != VTM_RV_SUCCESSED)
		{
			nRet = VTM_RV_SUCCESSED;
		}
	}
	break;
	case enumStepID_MEAS_SET_VISION:
	{
		// TP
		nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TP_2);

		// CELL Num
		if (nRet == VTM_RV_SUCCESSED)
		{
			int nMeasArea = 0;
			nMeasArea = m_vecMeasGroup.size();

			int nUseCELLID[df_MAX_PROBE];
			ZeroMemory(nUseCELLID, sizeof(nUseCELLID));
			TCHAR cCellID[df_MAX_PROBE][32];
			ZeroMemory(cCellID, sizeof(cCellID));
			bool bMeasProbe[df_MAX_PROBE];
			ZeroMemory(bMeasProbe, sizeof(bMeasProbe));


			

			if (nMeasArea > 0)
			{
				S_RecipeData* pRecipeData;
				pRecipeData = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeData();
				int nMaxStick = 0, nArea = 0;
				nMaxStick = pRecipeData->m_vecStick.size();	// jsyoon 2022.09.29 add
				nArea = pRecipeData->m_vecStick[0].m_vecCell.size();

				

				for (size_t i = 0; i < nMeasArea; i++)
				{
					if (i != m_iNumCell)
					{
						continue;
					}

					nUseCELLID[0] = m_vecMeasGroup[i].m_sCellInfo[0].m_nArea;
					nUseCELLID[1] = m_vecMeasGroup[i].m_sCellInfo[1].m_nArea;

					// Set PG Num
					// PG 번호 변경
					for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
					{
						if (nUseCELLID[iProbe] >= 0)
						{
							m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, nUseCELLID[iProbe] + 1);
						}
						else
						{
							if (iProbe == 0)
								m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, 1);
							else
								m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, nArea);
						}
					}

					if (nMeasArea == 1)	// 2023.05.09 add
					{
						nUseCELLID[0] = m_vecMeasGroup[i].m_sCellInfo[0].m_nArea;
						nUseCELLID[1] = nUseCELLID[0];
					}
				
					int nCellNum = 0;
					CString strCellId;

					S_RecipeInfo* pReciepInfo;
					pReciepInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();

					int nTimeOut = 30;
					nTimeOut = pReciepInfo->m_sInspectItem.m_nTimeOut[4];

					// jsyoon 2022.09.29 change
					int iRotate = 0;
					if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
					{
						iRotate = 1;
					}

					for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
					{
						bMeasProbe[iProbe] = true;
					}

					int nMeasStick = 0;
					ZeroMemory(cCellID, sizeof(cCellID));
					nMeasStick = MakeCellID(nUseCELLID, bMeasProbe, *cCellID, iRotate, m_iNumStick, nMaxStick);
					///

					GenMeasID(nUseCELLID, bMeasProbe, *cCellID);

					int nAreaNum[DF_MAX_HEAD] = { -1, -1 };

					if (nMeasArea == 1)	// 2023.05.09 add
					{
						if (nUseCELLID[0] >= 0) nAreaNum[1] = nUseCELLID[0] + (nArea * nMeasStick) + 1;	// HEAD 2 is Master

						bMeasProbe[0] = true;
						bMeasProbe[1] = true;
					}
					else
					{
						for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
						{
							bMeasProbe[iProbe] = false;

							if (nUseCELLID[iProbe] >= 0)
							{
								bMeasProbe[iProbe] = true;
								nAreaNum[iProbe] = nUseCELLID[iProbe] + (nArea * nMeasStick) + 1;
							}
						}

						//if (nUseCELLID[0] >= 0) nAreaNum[0] = nUseCELLID[0] + (nArea * nMeasStick) + 1;
						//if (nUseCELLID[1] >= 0) nAreaNum[1] = nUseCELLID[1] + (nArea * nMeasStick) + 1;
					}
					
					AddSegRowBase(nAreaNum);	// LYS 2026-10-13 Hybrid Phase 7 segBase of the active segment (1 segment: base 0, no-op)
					m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->SetMeasUseInfo(bMeasProbe, nAreaNum, nTimeOut);
				}
			}
		}

		if (nRet != VTM_RV_SUCCESSED)
		{
			nRet = VTM_RV_SUCCESSED;
		}
	}
	break;
	case enumStepID_MEAS_SET_COLOR:
	{
		// TP
		//nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TP_1); 
		nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TP_3); // jsyoon 2022.05.17 change

		// CELL Num
		if (nRet == VTM_RV_SUCCESSED)
		{
			int nMeasArea = 0;
			nMeasArea = m_vecMeasGroup.size();

			int nUseCELLID[df_MAX_PROBE];
			ZeroMemory(nUseCELLID, sizeof(nUseCELLID));
			TCHAR cCellID[df_MAX_PROBE][32];
			ZeroMemory(cCellID, sizeof(cCellID));
			bool bMeasProbe[df_MAX_PROBE];
			ZeroMemory(bMeasProbe, sizeof(bMeasProbe));

			if (nMeasArea > 0)
			{
				S_RecipeData* pRecipeData;
				pRecipeData = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeData();
				int nMaxStick = 0, nArea = 0;
				nMaxStick = pRecipeData->m_vecStick.size();	// jsyoon 2022.09.29 add
				nArea = pRecipeData->m_vecStick[0].m_vecCell.size();

				for (size_t i = 0; i < nMeasArea; i++)
				{
					if (i != m_iNumCell)
					{
						continue;
					}

					nUseCELLID[0] = m_vecMeasGroup[i].m_sCellInfo[0].m_nArea;
					nUseCELLID[1] = m_vecMeasGroup[i].m_sCellInfo[1].m_nArea;

					int nCellNum = 0;
					CString strCellId;

					S_RecipeInfo* pReciepInfo;
					pReciepInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();

					// jsyoon 2022.09.29 change
					int iRotate = 0;
					if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
					{
						iRotate = 1;
					}

					for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
					{
						bMeasProbe[iProbe] = true;
					}

					int nMeasStick = 0;
					ZeroMemory(cCellID, sizeof(cCellID));
					nMeasStick = MakeCellID(nUseCELLID, bMeasProbe, *cCellID, iRotate, m_iNumStick, nMaxStick);
					///

					//for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
					//{
					//	bMeasProbe[iProbe] = true;
					////	nCellNum = nMeasArea * iProbe + i;
					//	nCellNum = nUseCELLID[iProbe];

					//	if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
					//	{
					//		strCellId.Format(_T("%c%02d"), 65 + nCellNum, m_iNumStick + 1);
					//	}
					//	else
					//	{
					//		strCellId.Format(_T("%c%02d"), 65 + m_iNumStick, nArea - nCellNum);
					//	//	strCellId.Format(_T("%c%02d"), 65 + m_iNumStick, nMeasArea - nCellNum + 1);
					//	}

					////	strCellId.Format(_T("%c%02d"), 65 + nCellNum, m_iNumStick + 1);
					////	strCellId.Format(_T("%c%02d"), 65 + nCellNum, 5 - m_iNumStick);
					////	strCellId.Format(_T("%c%02d"), 65, nMeasArea* iProbe + i + 1);
					//	_tcscpy_s(cCellID[iProbe], _countof(cCellID[iProbe]), strCellId);
					//}

					GenMeasID(nUseCELLID, bMeasProbe, *cCellID);

					// Set PG Num
					// PG 번호 변경
					for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
					{
					/*	if (nUseCELLID[iProbe] >= 0)
							m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, nUseCELLID[iProbe] + 1);*/

						if (nUseCELLID[iProbe] >= 0)
						{
							m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, nUseCELLID[iProbe] + 1);
						}
						else
						{
							if (iProbe == 0)
								m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, 1);
							else
								m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, nArea);
						}
					}

					int nAreaNum[DF_MAX_HEAD] = { 0, };
					// jsyoon 2022.09.29 change
					//nAreaNum[0] = nUseCELLID[0] + (nMeasArea * df_MAX_PROBE * m_iNumStick) + 1;
					//nAreaNum[1] = nUseCELLID[1] + (nMeasArea * df_MAX_PROBE * m_iNumStick) + 1;
					nAreaNum[0] = nUseCELLID[0] + (nArea * nMeasStick) + 1;
					nAreaNum[1] = nUseCELLID[1] + (nArea * nMeasStick) + 1;
					AddSegRowBase(nAreaNum);	// LYS 2026-10-13 Hybrid Phase 7 segBase of the active segment (1 segment: base 0, no-op)
					m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->SetMeasUseInfo(bMeasProbe, nAreaNum, 3000);
				}
			}
		}

		if (nRet != VTM_RV_SUCCESSED)
		{
			nRet = VTM_RV_SUCCESSED;
		}
	}
	break;
	case enumStepID_MEAS_SET_ANGLE:
	{
		// TP
		nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TP_4);

		// CELL Num
		if (nRet == VTM_RV_SUCCESSED)
		{
			int nMeasArea = 0;
			nMeasArea = m_vecMeasGroup.size();

			int nUseCELLID[df_MAX_PROBE];
			ZeroMemory(nUseCELLID, sizeof(nUseCELLID));
			TCHAR cCellID[df_MAX_PROBE][32];
			ZeroMemory(cCellID, sizeof(cCellID));
			bool bMeasProbe[df_MAX_PROBE];
			ZeroMemory(bMeasProbe, sizeof(bMeasProbe));

			if (nMeasArea > 0)
			{
				S_RecipeData* pRecipeData;
				pRecipeData = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeData();
				int nMaxStick = 0, nArea = 0;
				nMaxStick = pRecipeData->m_vecStick.size();	// jsyoon 2022.09.29 add
				nArea = pRecipeData->m_vecStick[0].m_vecCell.size();

				for (size_t i = 0; i < nMeasArea; i++)
				{
					if (i != m_iNumCell)
					{
						continue;
					}

					nUseCELLID[0] = m_vecMeasGroup[i].m_sCellInfo[0].m_nArea;
					nUseCELLID[1] = m_vecMeasGroup[i].m_sCellInfo[1].m_nArea;

					int nCellNum = 0;
					CString strCellId;

					S_RecipeInfo* pReciepInfo;
					pReciepInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();

					// jsyoon 2022.09.29 change
					int iRotate = 0;
					if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
					{
						iRotate = 1;
					}

					for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
					{
						bMeasProbe[iProbe] = true;
					}

					int nMeasStick = 0;
					ZeroMemory(cCellID, sizeof(cCellID));
					nMeasStick = MakeCellID(nUseCELLID, bMeasProbe, *cCellID, iRotate, m_iNumStick, nMaxStick);
					///

					//for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
					//{
					//	bMeasProbe[iProbe] = true;
					////	nCellNum = nMeasArea * iProbe + i;
					//	nCellNum = nUseCELLID[iProbe];

					//	if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
					//	{
					//		strCellId.Format(_T("%c%02d"), 65 + nCellNum, m_iNumStick + 1);
					//	}
					//	else
					//	{
					//		strCellId.Format(_T("%c%02d"), 65 + m_iNumStick, nArea - nCellNum);
					//	//	strCellId.Format(_T("%c%02d"), 65 + m_iNumStick, nMeasArea - nCellNum + 1);
					//	}

					////	strCellId.Format(_T("%c%02d"), 65 + nCellNum, m_iNumStick + 1);
					////	strCellId.Format(_T("%c%02d"), 65 + nCellNum, 5 - m_iNumStick);
					////	strCellId.Format(_T("%c%02d"), 65, 2 * iProbe + iProbe);
					////	strCellId.Format(_T("%c%02d"), 65, nMeasArea* iProbe + i + 1);
					//	_tcscpy_s(cCellID[iProbe], _countof(cCellID[iProbe]), strCellId);
					//}

					GenMeasID(nUseCELLID, bMeasProbe, *cCellID);

					// Set PG Num
					// PG 번호 변경
					for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
					{
						/*if (nUseCELLID[iProbe] >= 0)
							m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, nUseCELLID[iProbe] + 1);*/

						if (nUseCELLID[iProbe] >= 0)
						{
							m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, nUseCELLID[iProbe] + 1);
						}
						else
						{
							if (iProbe == 0)
								m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, 1);
							else
								m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, nArea);
						}
					}

					int nAreaNum[DF_MAX_HEAD] = { 0, };
					// jsyoon 2022.09.29 change
					//nAreaNum[0] = nUseCELLID[0] + (nMeasArea * df_MAX_PROBE * m_iNumStick) + 1;
					//nAreaNum[1] = nUseCELLID[1] + (nMeasArea * df_MAX_PROBE * m_iNumStick) + 1;
					nAreaNum[0] = nUseCELLID[0] + (nArea * nMeasStick) + 1;
					nAreaNum[1] = nUseCELLID[1] + (nArea * nMeasStick) + 1;
					AddSegRowBase(nAreaNum);	// LYS 2026-10-13 Hybrid Phase 7 segBase of the active segment (1 segment: base 0, no-op)
					m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->SetMeasUseInfo(bMeasProbe, nAreaNum, 3000);
				}
			}
		}

		if (nRet != VTM_RV_SUCCESSED)
		{
			nRet = VTM_RV_SUCCESSED;
		}
	}
	break;
	case enumStepID_MEAS_SET_TSP:
	{
		//// TP
		//nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TP_5);

		//// CELL Num
		//if (nRet == VTM_RV_SUCCESSED)
		//{
		//	int nMeasArea = 0;
		//	nMeasArea = m_vecMeasGroup.size();

		//	int nUseCELLID[df_MAX_PROBE];
		//	ZeroMemory(nUseCELLID, sizeof(nUseCELLID));
		//	TCHAR cCellID[df_MAX_PROBE][32];
		//	ZeroMemory(cCellID, sizeof(cCellID));
		//	bool bMeasProbe[df_MAX_PROBE];
		//	ZeroMemory(bMeasProbe, sizeof(bMeasProbe));

		//	if (nMeasArea > 0)
		//	{
		//		S_RecipeData* pRecipeData;
		//		pRecipeData = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeData();
		//		int nMaxStick = 0, nArea = 0;
		//		nMaxStick = pRecipeData->m_vecStick.size();	// jsyoon 2022.09.29 add
		//		nArea = pRecipeData->m_vecStick[0].m_vecCell.size();

		//		for (size_t i = 0; i < nMaxStick; i++)
		//		{
		//			if (i != m_iNumCell)
		//			{
		//				continue;
		//			}

		//			nUseCELLID[0] = m_vecMeasGroup[i].m_sCellInfo[0].m_nArea;
		//			nUseCELLID[1] = m_vecMeasGroup[i].m_sCellInfo[1].m_nArea;

		//			int nCellNum = 0;
		//			CString strCellId;

		//			S_RecipeInfo* pReciepInfo;
		//			pReciepInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();

		//			// jsyoon 2022.09.29 change
		//			int iRotate = 0;
		//			if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
		//			{
		//				iRotate = 1;
		//			}

		//			for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
		//			{
		//				bMeasProbe[iProbe] = true;
		//			}

		//			int nMeasStick = 0;
		//			ZeroMemory(cCellID, sizeof(cCellID));
		//			nMeasStick = MakeCellID(nUseCELLID, bMeasProbe, *cCellID, iRotate, m_iNumStick, nMaxStick);

		//			GenMeasID(nUseCELLID, bMeasProbe, *cCellID);

		//			// Set PG Num
		//			// PG 번호 변경
		//			for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
		//			{
		//				if (nUseCELLID[iProbe] >= 0)
		//				{
		//					m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, nUseCELLID[iProbe] + 1);
		//				}
		//				else
		//				{
		//					if (iProbe == 0)
		//						m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, 1);
		//					else
		//						m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_SET_LEFT + iProbe, nArea);
		//				}
		//			}

		//			int nAreaNum[DF_MAX_HEAD] = { 0, };
		//			nAreaNum[0] = nUseCELLID[0] + (nArea * nMeasStick) + 1;
		//			nAreaNum[1] = nUseCELLID[1] + (nArea * nMeasStick) + 1;
		//			m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->SetMeasUseInfo(bMeasProbe, nAreaNum, 3000);
		//		}
		//	}
		//}
	}
	break;
	case enumStepID_MOVE_CELL:
	{
		// LYS 2026-10-13 manual meas = CycleTest positions: the move type is read once (verify_fix #10). OCP SPEC / QUALITY / ANGLE: the stage must be at
		//   the current stick pose first ((8), refused before anything changes), then the groups of that type are built for m_iNumStick (EnsureMeasGroup,
		//   stick range guard verify_fix #1). Other types: EnsureMeasGroup puts the legacy groups back if shared ones were in place
		int iMeasTypeNow = m_iMeasType;
		if (UseSharedMeasPos(iMeasTypeNow) && CheckManualMeasPose(iMeasTypeNow) == false)
		{
			nRet = VTM_RV_FAILED;	// WARN + popup done; VTM_RV_FAILED (0) raises no alarm below
			break;
		}
		m_iNumCell = 0;
		EnsureMeasGroup(iMeasTypeNow);

		// 이동
		nRet = ManualMoveCell(iMeasTypeNow, 0);	// LYS 2026-10-13 manual meas = CycleTest positions: one read of m_iMeasType (verify_fix #10)
	}
	break;
	case enumStepID_MOVE_CELL_NEXT:
	{
		int nSize = 0;
		//S_RecipeData* pRecipeData;
		//pRecipeData = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeData();
		// LYS 2026-10-13 manual meas = CycleTest positions: one read of the type (verify_fix #10). OCP SPEC / QUALITY / ANGLE: pose check first ((8)).
		//   When the groups held are not this type's current ones (another type's shared groups, the legacy groups, another stick or another recipe
		//   load), this button acts as the type's move button: cell 1 of freshly built groups (rca M5) - m_iNumCell = 0 whenever the group set is replaced.
		//   Legacy types after shared groups get the legacy groups back, also from cell 1. Same type, same groups: the next group as before
		int iMeasTypeNow = m_iMeasType;
		if (UseSharedMeasPos(iMeasTypeNow) && CheckManualMeasPose(iMeasTypeNow) == false)
		{
			nRet = VTM_RV_FAILED;	// WARN + popup done, m_iNumCell unchanged; VTM_RV_FAILED (0) raises no alarm below
			break;
		}
		if (IsMeasGroupCurrent(iMeasTypeNow) == false)
		{
			m_iNumCell = 0;
			EnsureMeasGroup(iMeasTypeNow);
			nRet = ManualMoveCell(iMeasTypeNow, 0);
			break;
		}
		nSize = m_vecMeasGroup.size();
		if (nSize > m_iNumCell + 1)
			m_iNumCell++;

		nRet = ManualMoveCell(iMeasTypeNow, m_iNumCell);
	}
	break;
	case enumStepID_MOVE_STICK:
	{
		m_iNumStick = 0;

		// 이동
		nRet = ManualMoveStick(0);
		UpdateCurStickUI();	// LYS 2026-08-24
	}
	break;
	case enumStepID_MOVE_STICK_NEXT:
	{
		int nSize = 0;
		S_RecipeData* pRecipeData;
		pRecipeData = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeData();
		nSize = pRecipeData->m_vecStick.size();

		if(nSize > m_iNumStick + 1)
			m_iNumStick++;

		// 이동
		nRet = ManualMoveStick(m_iNumStick);
		UpdateCurStickUI();	// LYS 2026-08-24
	}
	break;
	case enumStepID_MOVE_STICK_PRE:
	{
		// LYS 2026-08-24 Pre Stick Move 버튼이 m_iStep만 세팅하고 DoWorkManual()에
		// 대응 case가 없어 무동작이었다. MOVE_STICK_NEXT의 대칭 처리이며
		// ManualMoveStick()이 iStick 범위를 자체 검사한다.
		if (m_iNumStick > 0)
			m_iNumStick--;

		nRet = ManualMoveStick(m_iNumStick);
		UpdateCurStickUI();	// LYS 2026-08-24
	}
	break;
	case enumStepID_MOVE_PROBE:
	{
		// LYS 2026-10-13 manual meas review #6: OCP raise Sub Z / FRONT Z before the stage / probe moves (none below safety = no log)
		nRet = RetractHeadZBeforeStep(_T("PROBE MOVE"));
		if (nRet != VTM_RV_SUCCESSED) break;
		int nSize = 0;
		double dStageY = 0, dProbeGap = 0., dProbeShift = 0.;

		S_RecipeInfo* pRecipeInfo;
		pRecipeInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();

		S_RecipeMap* pRecipeMap;
		pRecipeMap = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeMap();

		S_RecipeData* pRecipeData;
		pRecipeData = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeData();
		if (pRecipeData != nullptr)
		{
			nSize = pRecipeData->m_vecStick.size();
			if (nSize > 0)
			{
				dProbeGap = pRecipeMap->m_fCellDistance.y;

				dProbeShift = pRecipeInfo->m_sMotionOffsetPos.m_fProbeShift;

				dStageY = pRecipeData->m_vecStick[0].m_fStageY + dProbeShift;

				// Probe X
				stMotorPosition stPositionProbeX[5];
		//		m_pAuroraMgr->GetMaintControlMgr()->GetCmdPos_ProbeX(stPositionProbeX, df_PROBE_MEAS, dStageY, dProbeGap);
				ZeroMemory(&stPositionProbeX, sizeof(stPositionProbeX));
				
				stPositionProbeX[0].axis = AXIS_PROBE_X1;
				stPositionProbeX[1].axis = AXIS_PROBE_X3;
				stPositionProbeX[2].axis = AXIS_PROBE_X5;
				stPositionProbeX[3].axis = AXIS_PROBE_X7;
				stPositionProbeX[4].axis = AXIS_PROBE_X9;

				int nStick = 0;
				nStick = pRecipeData->m_vecStick.size();

				// jsyoon 2022.09.27 change
				/*for (size_t i = 0; i < 5; i++)
				{
					if (i < nStick)
					{
						stPositionProbeX[4 - i].dPosition = pRecipeData->m_vecStick[i].m_fStageY + dProbeShift;
					}
					else
					{
						double fOffsetGap = 6.0;
						stPositionProbeX[4 - i].dPosition = pRecipeData->m_vecStick[0].m_fStageY - fOffsetGap - (dProbeGap * i);
					}
				}*/

				double dProbeInterlockPos = 0;
				m_pAuroraMgr->GetMaintControlMgr()->GetInterlockPos(nullptr, nullptr, &dProbeInterlockPos);

				double dOffsetX = 0.;
				dOffsetX = pRecipeInfo->m_sMotionOffsetPos.m_fStageY;

				bool bMoveSet[5];
				ZeroMemory(&bMoveSet, sizeof(bMoveSet));

				double fOffsetGap = 2.;	// 2023.05.12 change 3 to 2

				int k = 0;
				for (int i = 0; i < 5; i++)  // Probe 5개 
				{
					if (bMoveSet[4 - i] == true)	// A1, A2, A3, A4, M
						continue;

					if (i < nStick)
					{
						if (i == 0)
						{
							stPositionProbeX[4 - i].dPosition = pRecipeData->m_vecStick[i].m_fStageY + dProbeShift + dOffsetX;
							bMoveSet[4 - i] = true;  // Meas
							k = i;
						}
						else
						{
							if (k < nStick)
							{
								for (int j = k + 1; j < nStick; j++)
								{
									if (pRecipeData->m_vecStick[k].m_fStageY - pRecipeData->m_vecStick[j].m_fStageY > (dProbeInterlockPos + fOffsetGap) * 2)	// 2023.05.12 Stick 간 간격이 넓을 경우
									{
										int nCurProbe = 4 - i;
										int nNextProbe = 4 - i - 1;

										// Next Probe
										stPositionProbeX[nNextProbe].dPosition = pRecipeData->m_vecStick[j].m_fStageY + dProbeShift + dOffsetX;
										bMoveSet[nNextProbe] = true;

										// Cur Probe
										stPositionProbeX[nCurProbe].dPosition = stPositionProbeX[nNextProbe].dPosition + (dProbeInterlockPos + fOffsetGap);
										bMoveSet[nCurProbe] = true;

										k = j;

										break;
									}
									else if (pRecipeData->m_vecStick[k].m_fStageY - pRecipeData->m_vecStick[j].m_fStageY > (dProbeInterlockPos))
									{
										stPositionProbeX[4 - i].dPosition = pRecipeData->m_vecStick[j].m_fStageY + dProbeShift + dOffsetX;

										// jsyoon 2022.09.21 add
										double dMinAxisX3 = m_pAuroraMgr->GetMaintControlMgr()->GetMinSWLimit(AXIS_X3);
										if (stPositionProbeX[4 - i].dPosition > dMinAxisX3)
										{
											bMoveSet[4 - i] = true;	 // Aging
										}

										k = j;
										break;
									}
								}
							}
						}
					}
					else
					{
						bMoveSet[4 - i] = false;
					}
				}

				for (int i = 1; i < 5; i++)
				{
					if (bMoveSet[4 - i] != true)
					{
						stPositionProbeX[4 - i].dPosition = stPositionProbeX[5 - i].dPosition - ((dProbeInterlockPos + fOffsetGap));
						//	stPositionProbeX[4 - i].dPosition = pRecipeData->m_vecStick[0].m_fStageY - ((dProbeGap + fOffsetGap) * i) + dOffsetX;
					}
				}
				///////// 2022.09.27 change
		

				nRet = m_pAuroraMgr->GetMaintControlMgr()->MoveCmdPosMulti(stPositionProbeX, 5, SPEED_HIGH, true);
			}
		}
	}
	break;
	case enumStepID_PROBE_CONTACT:
	{
		
		S_RecipeInfo* pRecipeInfo;
		pRecipeInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();

		double dfMove = 0., dEMove = 0., dOD = 0.;
		dfMove = pRecipeInfo->m_sContactPos.m_fPreContactPos;
		dEMove = pRecipeInfo->m_sContactPos.m_fEndContactPos;
		dOD = pRecipeInfo->m_sContactPos.m_fAddOD;

		// LYS 2026-09-09 수동 CONTACT도 현재 선택 스틱(m_iNumStick)의 PROBE Z CONTACT OFFSET을 End에 가산
		double dZOffset = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetStickProbeZOffset(m_iNumStick);
		dEMove += dZOffset;
		{
			CString strLog;
			strLog.Format(_T("Manual Probe Contact stick=%d offset=%.4f -> Z5 target=%.4f"), m_iNumStick + 1, dZOffset, dEMove + dOD);
			CLogWrapper::GetInstance()->KMLOG(SYST, strLog);
		}

		nRet = m_pAuroraMgr->GetMaintControlMgr()->MoveCmdPos_ProbeContact(AXIS_PROBE_Z5, dfMove, dEMove, dOD);
	}
	break;
	case enumStepID_PROBE_UNCONTACT:
	{
		double dMove = 0;
		S_RecipeInfo* pRecipeInfo;
		pRecipeInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();
		dMove = pRecipeInfo->m_sContactPos.m_fPreContactPos;

		nRet = m_pAuroraMgr->GetMaintControlMgr()->MoveCmdPos_ProbeUnContact(AXIS_PROBE_Z5, dMove);
	}
	break;
	case enumStepID_PROBE_CONTACT_ALL:
	{
		
		S_RecipeInfo* pRecipeInfo;
		pRecipeInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();
		
		double dfMove = 0., dEMove = 0., dOD = 0.;
		dfMove = pRecipeInfo->m_sContactPos.m_fPreContactPos;
		dEMove = pRecipeInfo->m_sContactPos.m_fEndContactPos;
		dOD = pRecipeInfo->m_sContactPos.m_fAddOD;

		// jsyoon 2022.04.18 change
		nRet = m_pAuroraMgr->GetMaintControlMgr()->MoveCmdPos_ProbeContact(AXIS_ALL_Z, dfMove, dEMove, dOD);

		/*double dMove = 0;
		dMove = pRecipeInfo->m_sContactPos.m_fPreContactPos;

		stMotorPosition stPositionZ[5];
		ZeroMemory(&stPositionZ, sizeof(stPositionZ));
		stPositionZ[0].axis = AXIS_PROBE_Z1;
		stPositionZ[1].axis = AXIS_PROBE_Z2;
		stPositionZ[2].axis = AXIS_PROBE_Z3;
		stPositionZ[3].axis = AXIS_PROBE_Z4;
		stPositionZ[4].axis = AXIS_PROBE_Z5;

		stPositionZ[0].dPosition = dMove;
		stPositionZ[1].dPosition = dMove;
		stPositionZ[2].dPosition = dMove;
		stPositionZ[3].dPosition = dMove;
		stPositionZ[4].dPosition = dMove;

		nRet = m_pAuroraMgr->GetMaintControlMgr()->MoveCmdPosMulti(stPositionZ, 5, SPEED_HIGH, true);

		if (nRet == df_MOTION_RV_SUCCESSED)
		{
			dMove = pRecipeInfo->m_sContactPos.m_fEndContactPos + pRecipeInfo->m_sContactPos.m_fAddOD;

			stPositionZ[0].dPosition = dMove;
			stPositionZ[1].dPosition = dMove;
			stPositionZ[2].dPosition = dMove;
			stPositionZ[3].dPosition = dMove;
			stPositionZ[4].dPosition = dMove;

			nRet = m_pAuroraMgr->GetMaintControlMgr()->MoveCmdPosMulti(stPositionZ, 5, SPEED_LOW, true);
		}*/
	}
	break;
	case enumStepID_PROBE_UNCONTACT_ALL:
	{
		double dMove = 0;
		S_RecipeInfo* pRecipeInfo;
		pRecipeInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();
		dMove = pRecipeInfo->m_sContactPos.m_fPreContactPos;

		// jsyoon 2022.04.18 change
		nRet = m_pAuroraMgr->GetMaintControlMgr()->MoveCmdPos_ProbeUnContact(AXIS_ALL_Z, dMove);

		/*stMotorPosition stPositionZ[5];
		ZeroMemory(&stPositionZ, sizeof(stPositionZ));
		stPositionZ[0].axis = AXIS_PROBE_Z1;
		stPositionZ[1].axis = AXIS_PROBE_Z2;
		stPositionZ[2].axis = AXIS_PROBE_Z3;
		stPositionZ[3].axis = AXIS_PROBE_Z4;
		stPositionZ[4].axis = AXIS_PROBE_Z5;

		stPositionZ[0].dPosition = dMove;
		stPositionZ[1].dPosition = dMove;
		stPositionZ[2].dPosition = dMove;
		stPositionZ[3].dPosition = dMove;
		stPositionZ[4].dPosition = dMove;

		nRet = m_pAuroraMgr->GetMaintControlMgr()->MoveCmdPosMulti(stPositionZ, 5, SPEED_LOW, true);

		if (nRet == df_MOTION_RV_SUCCESSED)
		{
			dMove = 0;

			stPositionZ[0].dPosition = dMove;
			stPositionZ[1].dPosition = dMove;
			stPositionZ[2].dPosition = dMove;
			stPositionZ[3].dPosition = dMove;
			stPositionZ[4].dPosition = dMove;

			nRet = m_pAuroraMgr->GetMaintControlMgr()->MoveCmdPosMulti(stPositionZ, 5, SPEED_HIGH, true);
		}*/
	}
	break;
	case enumStepID_AGING_TEST_STOP:
	{
		m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectEnd(enumTCPIpFactorytID_AGING);

		m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectEnd(enumTCPIpFactorytID_PG);
	}
	break;
//	case enumStepID_AGING_TEST_STOP:
//	{
//	//	m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PGEvent_3);
//
//		m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectEnd(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_4);
//
//		m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectEnd(enumTCPIpFactorytID_AGING);
//
//	//	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_AGING_End, NULL);
//	}
	break;
	case enumStepID_RECIPE_OPEN_PG:
	{
//		m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkRecipeOpen(enumTCPIpFactorytID_PG);
		nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkRecipeOpen(enumTCPIpFactorytID_PG);

		if (nRet == VTM_RV_SUCCESSED)		// jsyoon 2022.08.16 add
		{
			nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PGEvent_1);
		}

		// LYS 2026-10-13 Hybrid Phase 7 D7-20 (14)-F model the PG device holds now - before the forced success below (1 segment: string only, never read)
		if (nRet == VTM_RV_SUCCESSED)
		{
			m_strPgModelSent = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetPGRecipeName();
			if (IsHybridRuntime())
			{
				CString strPgLog;
				strPgLog.Format(_T("[HYBRID] Manual Meas PG model sent [%s]"), (LPCTSTR)m_strPgModelSent);
				CLogWrapper::GetInstance()->KMLOG(OPER, strPgLog);
			}
		}
		else
		{
			m_strPgModelSent.Empty();	// LYS 2026-10-13 Hybrid Phase 7 a failed send leaves the device model unknown
		}
		if (nRet != VTM_RV_SUCCESSED)
		{
			nRet = VTM_RV_SUCCESSED;
		}
	}
	break;
	case enumStepID_RECIPE_OPEN_AGING:
	{
//		m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkRecipeOpen(enumTCPIpFactorytID_AGING);
		nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkRecipeOpen(enumTCPIpFactorytID_AGING);
	}
	break;
	case enumStepID_RECIPE_CHANGE_TSP:
	{
		nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkRecipeOpen(enumTCPIpFactorytID_TSP);
	}
	break;

	case enumStepID_RECIPE_CHANGE_WAD:
	{
		m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkRecipeOpen(enumTCPIpFactorytID_WAD);
	}
	break;
	case enumStepID_RECIPE_CHANGE_VISION:
	{
		if (m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkRecipeOpen(enumTCPIpFactorytID_VISION) == VTM_RV_SUCCESSED)
		{
			// 2022.12.27 add
			UINT nTimeOut = DEFINE_RECIPE_WAIT_TIME2;
			S_RecipeInfo* pReciepInfo;
			pReciepInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();
			if (pReciepInfo != nullptr)
			{
				nTimeOut = pReciepInfo->m_sInspectItem.m_nTimeOut[4];
			}

			if (nTimeOut < DEFINE_RECIPE_WAIT_TIME2)
			{
				nTimeOut = DEFINE_RECIPE_WAIT_TIME2;
			}

			m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkRecipeOpenComplate(enumTCPIpFactorytID_VISION, nTimeOut);
		}
	}
	break;
	case enumStepID_RECIPE_CHANGE_COLOR:
	{
		if (m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkRecipeOpen(enumTCPIpFactorytID_COLOR) == VTM_RV_SUCCESSED)
		{
			// 2022.12.27 add
			UINT nTimeOut = DEFINE_RECIPE_WAIT_TIME2;
			S_RecipeInfo* pReciepInfo;
			pReciepInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();
			if (pReciepInfo != nullptr)
			{
				nTimeOut = pReciepInfo->m_sInspectItem.m_nTimeOut[5];
			}

			if (nTimeOut < DEFINE_RECIPE_WAIT_TIME2)
			{
				nTimeOut = DEFINE_RECIPE_WAIT_TIME2;
			}

			m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkRecipeOpenComplate(enumTCPIpFactorytID_COLOR, nTimeOut);
		}
	}
	break;
	case enumStepID_RECIPE_CHANGE_ANGLE:
	{
		if (m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkRecipeOpen(enumTCPIpFactorytID_ANGLE) == VTM_RV_SUCCESSED)
		{
			// 2022.12.27 add
			UINT nTimeOut = DEFINE_RECIPE_WAIT_TIME2;
			S_RecipeInfo* pReciepInfo;
			pReciepInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();
			if (pReciepInfo != nullptr)
			{
				nTimeOut = pReciepInfo->m_sInspectItem.m_nTimeOut[6];
			}

			if (nTimeOut < DEFINE_RECIPE_WAIT_TIME2)
			{
				nTimeOut = DEFINE_RECIPE_WAIT_TIME2;
			}

			m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkRecipeOpenComplate(enumTCPIpFactorytID_ANGLE, nTimeOut);
		}
	}
	break;
	case enumStepID_MANUAL_PLC_RESET:
	{
		// LYS 2026-07-14 Add PLC Reset
		bool bRet = false;

		bRet = m_pAuroraMgr->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_ALRAM_RESET_STATUS, 1, 1);
		Sleep(200);
		if(bRet == 1) m_pAuroraMgr->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_ALRAM_RESET_STATUS, 0, 1);
		
	}
	break;

	case enumStepID_MANUAL_CMD_ON:
	{
		bool bRet = false;
		CString strAddr;
		GetDlgItemText(IDC_EDIT_MANUAL_TEST_ADDR, strAddr);
		int nAddr = _ttoi(strAddr);
		CString strM;	strM.Format(_T("M%d"), nAddr);	// LYS 2026-09-03 raw M-주소 쓰기(CString 오버로드). int 오버로드는 4000이상을 enum 인덱스로 오해석해 엉뚱한 비트를 write함
		bRet = m_pAuroraMgr->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(strM, 1, 1);
	}
	break;


	case enumStepID_MANUAL_CMD_OFF:
	{
		bool bRet = false;
		CString strAddr;
		GetDlgItemText(IDC_EDIT_MANUAL_TEST_ADDR, strAddr);
		int nAddr = _ttoi(strAddr);
		CString strM;	strM.Format(_T("M%d"), nAddr);	// LYS 2026-09-03 raw M-주소 쓰기(CString 오버로드). int 오버로드는 4000이상을 enum 인덱스로 오해석해 엉뚱한 비트를 write함
		bRet = m_pAuroraMgr->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(strM, 0, 1);
	}
	break;
	case enumStepID_MANUAL_WAD_RESET:
	{
		bool bRet = false;

		bRet = m_pAuroraMgr->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_ALRAM_RESET_STATUS, 1, 1);
		Sleep(200);
		if (bRet == 1) m_pAuroraMgr->GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_ALRAM_RESET_STATUS, 0, 1);
	}
	break;
	}



	if (nRet != VTM_RV_SUCCESSED)
	{
		CAlarmController::GetInstance()->SetAlarm(nRet);
	}

	this->EnableWindow(TRUE);

	m_pAuroraMgr->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_IDLE);

	return nRet;
}


UINT CDlgSubManual_Meas::ThreadFuncDone(LPARAM lp)
{
	if (IsWindowVisible() == TRUE)
	{
		CString strItemName = CString(LPCTSTR(lp));
		CString str = _T(" - SeqItem Done!!");
		CString strLog = strItemName + str;
		CLogWrapper::GetInstance()->KMLOG(OPER, strLog);

		AfxMessageBox(strLog);
	}

	return 0;
}

void CDlgSubManual_Meas::OnBnClickedBtnManualMeasClampIn2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	BtnEventLog(IDC_BTN_MANUAL_MEAS_CLAMP_IN2);
	m_iStep = enumStepID_GLASS_GET_02;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasAirOff()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	BtnEventLog(IDC_BTN_MANUAL_MEAS_AIR_OFF);
	m_iStep = enumStepID_GLASS_GET_06;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasAirOn()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	BtnEventLog(IDC_BTN_MANUAL_MEAS_AIR_ON);
	m_iStep = enumStepID_GLASS_GET_04;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasClampOut2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	BtnEventLog(IDC_BTN_MANUAL_MEAS_CLAMP_OUT2);
	m_iStep = enumStepID_GLASS_GET_08;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasClampDown()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	BtnEventLog(IDC_BTN_MANUAL_MEAS_CLAMP_DOWN);
	m_iStep = enumStepID_GLASS_GET_09;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualDefinePtrn()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
//	m_pAuroraMgr->GetRecipeEditMgrPtr()->SetROIAlignPtrn(0);
	m_pAuroraMgr->GetRecipeEditMgrPtr()->SetROIMeasPtrn(1);
}


int CDlgSubManual_Meas::MoveLoaderXYR(int iLoader)
{
	// TODO: 여기에 구현 코드 추가.
	double dPosX = 0.0;
	double dPosY = 0.0;
	double dPosR = 0.0;

	long lRet = VTM_RV_SUCCESSED;

	if (iLoader == 0)
	{
		m_pAuroraMgr->GetPositionConfigMgrPtr()->GetLoaderLoadPosition(&dPosX, &dPosY, &dPosR);
	}
	else if (iLoader == 1)
	{
		m_pAuroraMgr->GetPositionConfigMgrPtr()->GetLoaderUnloadPosition(&dPosX, &dPosY, &dPosR);
	}
	else if (iLoader == 2)
	{
		m_pAuroraMgr->GetPositionConfigMgrPtr()->GetLoaderMeasPosition(&dPosX, &dPosY, &dPosR);
	}
	else
	{
		return MOTION_CMDPOS_ERROR;
	}

	bool bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLiftpinSensorDown();
	if (bRet != true)
	{
		lRet = m_pAuroraMgr->GetMaintControlMgr()->DoWork_LPMove(false);

		if (lRet == VTM_RV_SUCCESSED)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLiftpinSensorDown();
		}
	}

	if (bRet == true)
	{	
		lRet = m_pAuroraMgr->GetMaintControlMgr()->MoveCmdPosA(AXIS_X, dPosX, true);

		if (lRet == VTM_RV_SUCCESSED)
		{
			lRet = m_pAuroraMgr->GetMaintControlMgr()->DoWork_RotateMove(dPosR, iLoader);
		}
	}
	else
	{
		lRet = PLC_LIFTPIN_DOWN_POS_ERROR;
	}

	return lRet;
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasContactMotion3()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	BtnEventLog(IDC_BTN_MANUAL_MEAS_CONTACT_MOTION3);

	m_iStep = enumStepID_PROBE_UNCONTACT_ALL;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasAngleOfFieldInspection2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	m_iStep = enumStepID_MEAS_TEST_ANGLE;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasAngleOfFieldInspection4()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	m_iStep = enumStepID_MEAS_TEST_LCR;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasAngleOfFieldInspection3()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	m_iStep = enumStepID_MEAS_TEST_TSP;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasClampIn5()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	m_iStep = enumStepID_GLASS_PUT_02;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasVacuumMotion5()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_GLASS_PUT_04;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasClampIn6()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_GLASS_PUT_05;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasVacuumMotion6()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_GLASS_PUT_06;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualGlassAlign()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	BtnEventLog(IDC_BTN_MANUAL_GLASS_ALIGN);
	m_iStep = enumStepID_GLASS_ALIGN;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasNextCellMove5()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	BtnEventLog(IDC_BTN_MANUAL_MEAS_NEXT_CELL_MOVE5);
	m_iStep = enumStepID_MANUAL_CMD_07;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedButton1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_MANUAL_CMD_08;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedButton2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_MANUAL_CMD_09;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedButton3()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_MANUAL_CMD_10;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedButton4()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_MANUAL_CMD_11;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedButton5()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_MANUAL_CMD_12;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedButton6()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_MANUAL_CMD_13;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualShutter()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_MANUAL_SHUTTER;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasPgCmd5()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_PG_CMD_05;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasAging2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_PRE_TEST_04;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasMeasurementSetPg()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_MEAS_SET_VISION;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasMeasurementSetPg2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_MEAS_SET_WAD;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasMeasurementSetPg3()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_MEAS_SET_COLOR;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasMeasurementSetPg4()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_MEAS_SET_ANGLE;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasMeasurementSetPg5()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_MEAS_SET_TSP;
	DoWorkManual();
}

// jsyoon 2022.09.29 add Cell ID
int CDlgSubManual_Meas::MakeCellID(int* pnCellNum, bool* pbMeasProbe, TCHAR* ppcCellID, int iRotate, int iStick, int nMaxStick)
{
	int nMeasStick = 0, nCellNum = 0;

	CString strCellId;
	TCHAR cCellID[df_MAX_PROBE][32];
	ZeroMemory(cCellID, sizeof(cCellID));
	int nLetterBase = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetCellIdLetterBase();	// LYS 2026-10-13 Hybrid Phase 7 letter carry (D4b-9, auto MakeCellID); -1 = 1-segment recipe -> the original "%c" lines below

	for (size_t iProbe = 0; iProbe < df_MAX_PROBE; iProbe++)
	{
		if (pbMeasProbe[iProbe] == true)
		{
			nCellNum = pnCellNum[iProbe];

			if (iRotate == 1) // YY, BT
			{
				if (nLetterBase >= 0 && nCellNum >= 0) strCellId.Format(_T("%s%02d"), (LPCTSTR)CRecipeEditMgr::CellIdLetterPrefix(nLetterBase + nCellNum), iStick + 1);	// LYS 2026-10-13 Hybrid Phase 7 (cell -1 -> the legacy "@" line)
				else
				strCellId.Format(_T("%c%02d"), 65 + nCellNum, iStick + 1);
				nMeasStick = iStick;
			}
			else // DG
			{
				if (nLetterBase >= 0) strCellId.Format(_T("%s%02d"), (LPCTSTR)CRecipeEditMgr::CellIdLetterPrefix(nLetterBase + nMaxStick - iStick - 1), nCellNum + 1);	// LYS 2026-10-13 Hybrid Phase 7 (nMeasStick below stays segment-local)
				else
				strCellId.Format(_T("%c%02d"), 65 + nMaxStick - iStick - 1, nCellNum + 1);
				nMeasStick = nMaxStick - iStick - 1;
			}

			_tcscpy_s(cCellID[iProbe], _countof(cCellID[iProbe]), strCellId);
		}
	}

	memcpy(ppcCellID, cCellID, sizeof(cCellID));

	return nMeasStick;
}

void CDlgSubManual_Meas::GenMeasID(int* pnCellNum, bool* pbMeasProbe, TCHAR* ppcCellID)
{
	// Generate CELL ID
	GenCellID(pnCellNum, pbMeasProbe, ppcCellID);

	// Generate InnerID
	GenInnerID(pnCellNum, pbMeasProbe);
}


void CDlgSubManual_Meas::GenCellID(int* pnCellNum, bool* pbMeasProbe, TCHAR* ppcCellID)
{
	CString strMeasCell;
	strMeasCell.Empty();
	int iTop = 0, nMaxTop = 5;
	bool bHybridCid = (m_pAuroraMgr->GetRecipeEditMgrPtr()->GetCellIdLetterBase() >= 0);	// LYS 2026-10-13 Hybrid Phase 7 letter carry (false = 1-segment recipe -> original B-panel parser, no length guard)
	bool bHybridTok = IsHybridRuntime();	// LYS 2026-10-13 Hybrid Phase 7 D7-12 (b) manual / derived cell token on a hybrid runtime recipe only

	//2022.08.26 JCY Modify
	S_RecipeData* pRecipeData;
	pRecipeData = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeData();
	//nMaxTop = pRecipeData->m_vecStick.size();

	// jsyoon 2022.09.21 add
	S_RecipeInfo* pReciepInfo;
	pReciepInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();
	if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
	{
		nMaxTop = pRecipeData->m_vecStick.size();
	}
	else
	{
		nMaxTop = pRecipeData->m_vecStick[0].m_vecCell.size();
	}

	//

	TCHAR cCellID[df_MAX_PROBE][32];
	memcpy(cCellID, ppcCellID, sizeof(cCellID));

	// Generate CELL ID
	CString strTmp, strGlassID, strCellID, strMcrID, strInnerID;

	for (size_t nProbe = 0; nProbe < df_MAX_PROBE; nProbe++)
	{
		strMcrID.Empty();
		bool bCidOverride = false;	// LYS 2026-10-13 Hybrid Phase 7 manual / derived token this slot

		if (pbMeasProbe[nProbe])
		{
			// GlassID + CellID
			strTmp.Format(_T("%s "), cCellID[nProbe]); // stMeasGroup.stProbe[nProbe].m_CellID);
			strMeasCell += strTmp;

			// Cell ID
			strGlassID.Format(_T("%s"), CSystemParamShare::GetInstance()->GetGlassID());

			// Glass Part A or B
			CString strPart;
			strPart.Format(_T("%s"), strGlassID.Mid(strGlassID.GetLength() - 2, 1));
			// LYS 2026-10-13 Hybrid Phase 7 D4b-9 manual / derived cell token (the DoWorkGlassMeasureStep GenCellID hook): final value, B offset already
			//   inside -> skip the B-panel branch. ValidateManualCellId bound latch 1 to this load + Glass ID right before this measure set
			CString strCidTok;
			if (bHybridTok) bCidOverride = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetManualCellToken(strPart.CompareNoCase(_T("B")) == 0, m_iNumStick, pnCellNum[nProbe], strGlassID, strCidTok);
			if (bCidOverride) strTmp = strCidTok;

			if (bCidOverride == false && strPart.CompareNoCase(_T("B")) == 0)	// AB (LYS 2026-10-13 Hybrid Phase 7: an override token is final)
			{
				CString strBuf;
				if (bHybridCid)	// LYS 2026-10-13 Hybrid Phase 7 the letter prefix may be 2 letters (AA01): split letters / digits instead of Mid(0,1) / Mid(1,2)
				{
					int nPfx = 0;
					while (nPfx < strTmp.GetLength() && strTmp[nPfx] >= _T('A') && strTmp[nPfx] <= _T('Z')) nPfx++;
					strBuf.Format(_T("%s%02d"), (LPCTSTR)strTmp.Left(nPfx), nMaxTop + _ttoi(strTmp.Mid(nPfx)));
				}
				else
				strBuf.Format(_T("%s%02d"), strTmp.Mid(0, 1), nMaxTop + _ttoi(strTmp.Mid(1, 2)));  // A01~A10
				strTmp.Format(_T("%s"), strBuf);

		//		strTmp.Format(_T("%s%02d"), strTmp.Mid(0, 1), nMaxTop + nMaxTop - iTop);
			}
			else										// AA
			{
				;
			}
			//////////////////////////////////////////////////////////////////////////

			strCellID.Format(_T("%s%s"), strGlassID.Left(strGlassID.GetLength() - 1), strTmp);
			strMcrID.Format(_T("%s"), gf_MakeMcrID(strCellID));
		}

		if ((bHybridCid || bCidOverride) && strMcrID.GetLength() > 31)	// LYS 2026-10-13 Hybrid Phase 7 m_szCellID[..][32]: _tcscpy_s aborts on 32+ chars (2-letter prefix / manual token + long GlassID)
		{
			CString strCidLog;
			strCidLog.Format(bHybridCid ? _T("[HYBRID] cell ID %s longer than 31 - cut to 31") : _T("[CELLID] cell ID %s longer than 31 - cut to 31"), (LPCTSTR)strMcrID);
			CLogWrapper::GetInstance()->KMLOG(WARN, strCidLog);
			strMcrID = strMcrID.Left(31);
		}
		TRACE(_T("GenCellID:%s %d,%d:%s\n"), strMeasCell, pnCellNum[nProbe] + 1, nProbe + 1, strMcrID);
		_tcscpy_s(m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->stLotInfo.m_szCellID[nProbe], 32, strMcrID);		// CELL ID 설정
	}
}

void CDlgSubManual_Meas::GenInnerID(int* pnCellNum, bool* pbMeasProbe)
{
	CString strInnerID;
	for (size_t nProbe = 0; nProbe < df_MAX_PROBE; nProbe++)
	{
		strInnerID.Empty();

		if (pbMeasProbe[nProbe])
		{
			if (m_pStInnerID != NULL && pnCellNum[nProbe] >= 0)
	//		if( false)
			{
				strInnerID.Format(_T("%s"), m_pStInnerID[pnCellNum[nProbe]]);
				if (strInnerID.GetLength() < 5)
				{
					strInnerID.Format(_T("%d%s"), nProbe + 1, gf_GetCurTimeString(3));

					_tcscpy_s(m_pStInnerID[pnCellNum[nProbe]], 32, strInnerID);
				}
			}
			else
			{
				strInnerID.Format(_T("%d%s"), nProbe + 1, gf_GetCurTimeString(3));
			}
		}

		TRACE(_T("GenInnerID:%d,%d:%s\n"), pnCellNum[nProbe] + 1, nProbe + 1, strInnerID);
		_tcscpy_s(m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->stLotInfo.m_szInnerID[nProbe], 32, strInnerID);
	}
}

void CDlgSubManual_Meas::SetMeasureArea(int nMeasureArea)
{
	if (m_pStInnerID != nullptr)
	{
		for (int k = 0; k < m_nMeasureArea; k++)
			delete m_pStInnerID[k];

		delete[] m_pStInnerID;
		m_pStInnerID = NULL;
	}

	if (nMeasureArea > 0)
	{
		m_pStInnerID = new TCHAR * [nMeasureArea];
		for (int i = 0; i < nMeasureArea; i++)
		{
			m_pStInnerID[i] = new TCHAR[32];
			memset(m_pStInnerID[i], 0x00, sizeof(m_pStInnerID[i]));			// INNER ID 저장 공간
		}
	}

	m_nMeasureArea = nMeasureArea;
}

int CDlgSubManual_Meas::GetMeasureArea()
{
	return m_nMeasureArea;
}

int CDlgSubManual_Meas::MakeMeasGroup(bool bKeepInnerId)
{
	// LYS 2026-10-13 manual meas = CycleTest positions (verify_fix #9): the groups built here are the legacy ones. When they replace shared groups
	//   (RECIPE_OPEN, Phase 7a resync, a legacy type after a shared move) the cell index restarts at 1 - the measure set / test steps read
	//   m_vecMeasGroup[m_iNumCell] unchecked. Legacy groups replacing legacy groups keep m_iNumCell as before
	if (m_nMeasGroupMode != -1) m_iNumCell = 0;
	m_nMeasGroupMode = -1;
	m_nMeasGroupStick = -1;
	m_nMeasGroupLoadSeq = -1;
	// TODO: 여기에 구현 코드 추가.
	m_vecMeasGroup.clear();

	int nMeasArea = 0;
	double dGantryGap = 0, dHeadGap = 0, dHeadGapRcp = 0;
//	m_pAuroraMgr->GetMaintControlMgr()->GetInterlockPos(&dGantryGap, &dHeadGap, nullptr);
	m_pAuroraMgr->GetMaintControlMgr()->GetInterlockPos(&dGantryGap, nullptr, nullptr, &dHeadGap);

	S_RecipeInfo* pReciepInfo;
	pReciepInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();

	dHeadGapRcp = pReciepInfo->m_sOptionItem.m_fBackHeadGap;
	if (dHeadGapRcp > dHeadGap)
	{
		dHeadGap = dHeadGapRcp;
	}

	double dGlassConactPos = 0;
	//		dGlassConactPos = m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_PROBE_X9) - 20;

	S_RecipeData* pRecipeData;
	pRecipeData = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeData();
	int iStick = 0, nArea;
	iStick = m_iNumStick;

	if (pRecipeData->m_vecStick.size() < iStick + 1)
	{
		return 0;
	}

	nArea = pRecipeData->m_vecStick[iStick].m_vecCell.size();
	dGlassConactPos = pRecipeData->m_vecStick[0].m_fStageY + pReciepInfo->m_sMotionOffsetPos.m_fHeadReviewY;

	// Generate 2Head Group
	bool* pbSet = new bool[nArea];
	for (int iGroup = 0; iGroup < nArea; iGroup++)
		pbSet[iGroup] = false;

//	vector <S_MeasPos>	vecMeasGroup; //MeasGroup 정보
//	vecMeasGroup.clear();

	S_CellInfo cellFirst, cellSecond;
	S_MeasPos measPos;
	cellFirst.Clear();

	// LYS 2026-09-14 2Head 짝을 순번 방식으로 먼저 정한다 (DoWorkGlassMeasureStep.cpp MakeHeadPairByOrder 주석 참조).
	//   -측 셀 k 번째 <-> +측 셀 k 번째 (A01-K01, B01-L01 ...). 수동 측정은 셀 필터가 없어 전 셀 대상.
	int* pnPair = new int[nArea];
	MakeHeadPairByOrder(pRecipeData->m_vecStick[iStick].m_vecCell, nullptr, nArea, dHeadGap, pnPair, _T("MANUAL"));

	for (int iGroup = 0; iGroup < nArea; iGroup++) 
	{
		if (pbSet[iGroup] != true)
		{
			cellFirst = pRecipeData->m_vecStick[iStick].m_vecCell[iGroup];

			cellSecond.Clear();
			for (int iSearch = 0; iSearch < nArea; iSearch++)
			{
				if (pbSet[iSearch] != true)
				{
					cellSecond = pRecipeData->m_vecStick[iStick].m_vecCell[iSearch];

					// LYS 2026-09-14 순번으로 정해진 짝(pnPair)만 묶는다. 구 조건(아래 주석)은 X<50 셀에 가장 먼저 걸린 먼 셀을 묶었다.
					//if (cellFirst.m_fCellX < 50)
					if (cellFirst.m_fCellX < 0 && pnPair[iGroup] == iSearch)
					{
						if (fabs(cellFirst.m_fCellX - cellSecond.m_fCellX) > dHeadGap)
						{
							pbSet[iGroup] = true;
							pbSet[iSearch] = true;
							measPos.Clear();
							measPos.dHeadFrontG = cellFirst.m_fCellY + dGlassConactPos;
							measPos.dHeadBackG = measPos.dHeadFrontG + dGantryGap;
							measPos.dHeadFrontZ = pRecipeData->m_sHeadZ.m_fSpecZ;
							measPos.dHeadBackG = pRecipeData->m_sHeadZ.m_fQualityZ;

							if (cellFirst.m_nArea >= nArea)
							{
								cellFirst.m_nArea -= (nArea * iStick);
							}

							if (cellSecond.m_nArea >= nArea)
							{
								cellSecond.m_nArea -= (nArea * iStick);
							}

							measPos.m_sCellInfo[0] = cellFirst;
							measPos.m_sCellInfo[1] = cellSecond;


							m_vecMeasGroup.push_back(measPos);

							break;
						}
					}
				}
			}
		}
	}

	// 선택하고 남은것
	for (int iGroup = 0; iGroup < nArea; iGroup++)
	{
		if (pbSet[iGroup] != true)
		{
			pbSet[iGroup] = true;

			cellFirst = pRecipeData->m_vecStick[iStick].m_vecCell[iGroup];

			// LYS 2026-09-14 단독 셀 헤드 판정은 기존 50 기준 유지 - 0 으로 두면 X=0 셀을 RIGHT 헤드가 맡아 화질/시야각 Y3=-50(HW 리밋 -66 근처)이 된다.
			// LYS 2026-09-14 (재변경) 단독 셀 헤드 판정 50 -> 0 (자동 측정 스텝과 동일 - ANGLE J01 을 Y3 로 보내기 위함).
			//if (cellFirst.m_fCellX > 50)
			if (cellFirst.m_fCellX > 0)
			{
				measPos.Clear();
				measPos.dHeadFrontG = cellFirst.m_fCellY + dGlassConactPos;
				measPos.dHeadBackG = measPos.dHeadFrontG + dGantryGap;
				measPos.dHeadFrontZ = pRecipeData->m_sHeadZ.m_fSpecZ;
				measPos.dHeadBackG = pRecipeData->m_sHeadZ.m_fQualityZ;


				if (cellFirst.m_nArea >= nArea)
				{
					cellFirst.m_nArea -= (nArea * iStick);
				}

				measPos.m_sCellInfo[1] = cellFirst;
				cellFirst.m_fCellX -= dHeadGap;
				cellFirst.m_nArea = -1;
				measPos.m_sCellInfo[0] = cellFirst;
				m_vecMeasGroup.push_back(measPos);
			}
			else
			{
				measPos.Clear();
				measPos.dHeadFrontG = cellFirst.m_fCellY + dGlassConactPos;
				measPos.dHeadBackG = measPos.dHeadFrontG + dGantryGap;
				measPos.dHeadFrontZ = pRecipeData->m_sHeadZ.m_fSpecZ;
				measPos.dHeadBackG = pRecipeData->m_sHeadZ.m_fQualityZ;

				if (cellFirst.m_nArea >= nArea)
				{
					cellFirst.m_nArea -= (nArea * iStick);
				}

				measPos.m_sCellInfo[0] = cellFirst;
				cellFirst.m_fCellX += dHeadGap;
				cellFirst.m_nArea = -1;
				measPos.m_sCellInfo[1] = cellFirst;
				m_vecMeasGroup.push_back(measPos);
			}
		}
	}

	delete[] pbSet;
	delete[] pnPair;	// LYS 2026-09-14

	nMeasArea = m_vecMeasGroup.size();
//	SetMeasureArea(nMeasArea *2);	// LYS 2026-10-13 manual meas = CycleTest positions (review fix #7): replaced by the line below (bKeepInnerId false = this reset + the inner ID stamp)
	SetInnerIdArea(nMeasArea * 2, bKeepInnerId);

	return 0;
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasMeasurementCellMove7()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_MOVE_CELL;
	m_iMeasType = M_TYPE_SPEC;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasMeasurementCellMove8()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_MOVE_CELL_NEXT;
	m_iMeasType = M_TYPE_SPEC;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasMeasurementCellMove9()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_MOVE_CELL_NEXT;
	m_iMeasType = M_TYPE_QUALITY;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasMeasurementCellMove6()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_MOVE_CELL_NEXT;
	m_iMeasType = M_TYPE_COLOR;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasMeasurementCellMove4()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_MOVE_CELL;
	m_iMeasType = M_TYPE_ANGLE;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasMeasurementCellMove5()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_MOVE_CELL_NEXT;
	m_iMeasType = M_TYPE_ANGLE;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasContactMotion4()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_PROBE_CONTACT;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasContactMotion5()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_PROBE_UNCONTACT;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasProbeMove()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_MOVE_PROBE;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasAging3()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_AGING_TEST_STOP;
	DoWorkManual();
}

void CDlgSubManual_Meas::OnBnClickedBtnManualMeasRecipeOpen2()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_RECIPE_CHANGE_WAD;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasRecipeOpen3()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_RECIPE_CHANGE_VISION;
	DoWorkManual();
}

void CDlgSubManual_Meas::OnBnClickedBtnManualMeasRecipeOpen4()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_RECIPE_CHANGE_COLOR;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasRecipeOpen5()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_RECIPE_CHANGE_ANGLE;
	DoWorkManual();
}

void CDlgSubManual_Meas::OnBnClickedBtnManualMeasRecipeOpen6()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_RECIPE_OPEN_PG;
	DoWorkManual();
}


void CDlgSubManual_Meas::OnBnClickedBtnManualMeasRecipeOpen7()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_RECIPE_OPEN_AGING;
	DoWorkManual();
}

void CDlgSubManual_Meas::OnBnClickedBtnManualMeasRecipeOpen8()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_RECIPE_CHANGE_TSP;
	DoWorkManual();
}

void CDlgSubManual_Meas::OnBnClickedBtnManualPlcReset()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_iStep = enumStepID_MANUAL_PLC_RESET;
	DoWorkManual();
}

void CDlgSubManual_Meas::OnBnClickedBtnManualPlcBitOn()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString str, strAddr;
	GetDlgItemText(IDC_EDIT_MANUAL_TEST_ADDR, strAddr);
	str.Format(_T("%s bit를 On 합니다.\n 계속 하시겠습니까?"), strAddr);
	if (AfxMessageBox(str, MB_YESNO | MB_ICONQUESTION) != IDYES) return;
	m_iStep = enumStepID_MANUAL_CMD_ON;
	DoWorkManual();
}

void CDlgSubManual_Meas::OnBnClickedBtnManualPlcBItOff()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString str, strAddr;
	GetDlgItemText(IDC_EDIT_MANUAL_TEST_ADDR, strAddr);
	str.Format(_T("%s bit를 Off 합니다.\n 계속 하시겠습니까?"), strAddr);
	if (AfxMessageBox(str, MB_YESNO | MB_ICONQUESTION) != IDYES) return;
	m_iStep = enumStepID_MANUAL_CMD_OFF;
	DoWorkManual();
}

void CDlgSubManual_Meas::OnBnClickedBtnManualPlcWadReset()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
}

// LYS 2026-10-13 Hybrid Phase 7 7a manual motion gate (p7x_plan (2-4), (3-3)..(3-7), (14)-A B C F K M, (15)). Every function below returns at once
//   on a 1-segment runtime recipe (segment-count cache <= 1 and live DLL count <= 1) - no log, no popup - so the 1-segment manual screen is unchanged.
//   The Glass Align reference R mark (stage 7-0) lives in CMaintControlMgr: GetGlassAlignRefR = Align coordinate R when AlignGlass succeeded.

// (14)-A hybrid runtime recipe - fix stage (impl review 1): one predicate = busy ? cache > 1 : live > 1. While a LoadRcpFile runs the segment-count
//   cache int decides, otherwise the live DLL count: a DLL TestMode reload over a 1-segment runtime leaves the cache at 1 (hybrid), a 1-segment reload
//   over a hybrid runtime leaves it at 2 (a 1-segment runtime - CheckHybridGate re-syncs it silently). Worker-thread callers only. Ints + one vector size, no log
bool CDlgSubManual_Meas::IsHybridRuntime()
{
	CRecipeEditMgr* pRcp = m_pAuroraMgr->GetRecipeEditMgrPtr();
	if (pRcp == NULL) return false;
	C_OutRecipeFunc* pFunc = pRcp->GetRecipeFuncPtr();
	if (pFunc == NULL || pFunc->IsRcpLoadBusy() == TRUE) return (pRcp->GetSegCountCache() > 1);	// load running: the cache int only
	return (pFunc->GetSegmentLoopCount() > 1);
}

// (14)-B step classes: S stick move / P pose dependent / C contact + measure set / T unsupported / G align (G0 + G0-b only) / PG (G0 + PG model only)
int CDlgSubManual_Meas::HybridStepClass(int nStep)
{
	switch (nStep)
	{
	case enumStepID_MOVE_STICK:
	case enumStepID_MOVE_STICK_PRE:		// also the literal m_iStep = 26 (OnBnClickedBtnManualGlassRotate2)
	case enumStepID_MOVE_STICK_NEXT:
		return HYB_STEP_S;
	case enumStepID_MOVE_CELL:
	case enumStepID_MOVE_CELL_NEXT:
	case enumStepID_MEAS_TEST_WAD:
	case enumStepID_MEAS_TEST_VISION:
	case enumStepID_MEAS_TEST_COLOR:
	case enumStepID_MEAS_TEST_ANGLE:
	case enumStepID_PRE_TEST_01:		// D7-23 (a): PG tests / aging after a contact need the stick pose (R-B)
	case enumStepID_PRE_TEST_02:
	case enumStepID_PRE_TEST_03:
	case enumStepID_PRE_TEST_04:
	case enumStepID_AGING_TEST:
		return HYB_STEP_P;
	case enumStepID_PROBE_CONTACT:
	case enumStepID_PROBE_CONTACT_ALL:
	case enumStepID_MEAS_SET_WAD:
	case enumStepID_MEAS_SET_VISION:
	case enumStepID_MEAS_SET_COLOR:
	case enumStepID_MEAS_SET_ANGLE:
		return HYB_STEP_C;
	case enumStepID_MEAS_SET_TSP:		// D7-13 (a): TSP / LCR / probe move (off-screen or hidden buttons) are not ported to hybrid
	case enumStepID_MEAS_SET_LCR:
	case enumStepID_MEAS_TEST_TSP:
	case enumStepID_MEAS_TEST_LCR:
	case enumStepID_MOVE_PROBE:
		return HYB_STEP_T;
	case enumStepID_GLASS_ALIGN:
	case enumStepID_PROBE_ALIGN:		// stage 7-4 adds the segment prepare step here
		return HYB_STEP_G;
	case enumStepID_PG_CMD_01:
	case enumStepID_PG_CMD_02:
	case enumStepID_PG_CMD_03:
	case enumStepID_PG_CMD_05:			// PG_CMD_04 Turn Off: no gate
		return HYB_STEP_PG;
	}
	return HYB_STEP_NONE;
}

bool CDlgSubManual_Meas::IsMeasSetStep(int nStep)
{
	return (nStep == enumStepID_MEAS_SET_WAD || nStep == enumStepID_MEAS_SET_VISION || nStep == enumStepID_MEAS_SET_COLOR || nStep == enumStepID_MEAS_SET_ANGLE);
}

// D7-20 steps that drive the PG with the active segment's model: PG tests / aging / PG commands always; the measure sets (PG TP) and the
//   COLOR test only when the active segment uses the PG (pre / electric / aging inspection)
bool CDlgSubManual_Meas::IsPgStep(int nStep)
{
	switch (nStep)
	{
	case enumStepID_PRE_TEST_01:
	case enumStepID_PRE_TEST_02:
	case enumStepID_PRE_TEST_03:
	case enumStepID_PRE_TEST_04:
	case enumStepID_AGING_TEST:
	case enumStepID_PG_CMD_01:
	case enumStepID_PG_CMD_02:
	case enumStepID_PG_CMD_03:
	case enumStepID_PG_CMD_05:
		return true;
	case enumStepID_MEAS_SET_WAD:
	case enumStepID_MEAS_SET_VISION:
	case enumStepID_MEAS_SET_COLOR:
	case enumStepID_MEAS_SET_ANGLE:
	case enumStepID_MEAS_TEST_COLOR:
	{
		S_RecipeInfo* pInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();
		if (pInfo == NULL) return true;
		return (pInfo->m_sInspectItem.m_bUsePre_0 == TRUE || pInfo->m_sInspectItem.m_bUseElect_1 == TRUE || pInfo->m_sInspectItem.m_bUseAgingTest_2 == TRUE);
	}
	}
	return false;
}

// target stick of a stick-move step - the same rule as the MOVE_STICK / _NEXT / _PRE cases, taken before they change m_iNumStick
int CDlgSubManual_Meas::PeekTargetStick(int nStep)
{
	if (nStep == enumStepID_MOVE_STICK) return 0;
	if (nStep == enumStepID_MOVE_STICK_NEXT)
	{
		S_RecipeData* pRecipeData = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeData();
		int nSize = (pRecipeData != NULL) ? (int)pRecipeData->m_vecStick.size() : 0;
		return (nSize > m_iNumStick + 1) ? m_iNumStick + 1 : m_iNumStick;
	}
	if (nStep == enumStepID_MOVE_STICK_PRE) return (m_iNumStick > 0) ? m_iNumStick - 1 : m_iNumStick;
	return m_iNumStick;
}

// target stage pose of a stick = ManualMoveStick's formula (OCP absolute / non-OCP relative + recipe StageY / StageRotate offsets).
//   Keep both in step ((12) risk). false = no pose (no recipe / stick outside the active segment)
bool CDlgSubManual_Meas::CalcStickPose(int iStick, double& dStageY, double& dStageR)
{
	CRecipeEditMgr* pRcp = m_pAuroraMgr->GetRecipeEditMgrPtr();
	if (pRcp == NULL) return false;
	S_RecipeData* pRecipeData = pRcp->GetRecipeData();
	S_RecipeMap* pRecipeMap = pRcp->GetRecipeMap();
	S_RecipeInfo* pRecipeInfo = pRcp->GetRecipeInfo();
	if (pRecipeData == nullptr || pRecipeMap == nullptr || pRecipeInfo == nullptr) return false;
	if (iStick < 0 || iStick >= (int)pRecipeData->m_vecStick.size()) return false;

	bool bOCP = (m_pAuroraMgr->GetSystemConfigMgrPtr() != nullptr && m_pAuroraMgr->GetSystemConfigMgrPtr()->IsOCP());
	double dOffsetX = pRecipeInfo->m_sMotionOffsetPos.m_fStageY;
	double dOffsetR = pRecipeInfo->m_sMotionOffsetPos.m_fStageRotate;
	if (bOCP)
		dStageY = pRecipeData->m_vecStick[iStick].m_fStageY + dOffsetX;
	else if (iStick > 0)
		dStageY = pRecipeData->m_vecStick[0].m_fStageY - pRecipeData->m_vecStick[iStick].m_fStageY + dOffsetX;
	else
		dStageY = pRecipeMap->m_fCellDistance.y * iStick + dOffsetX;
	dStageR = pRecipeData->m_vecStick[iStick].m_fStageR + dOffsetR;
	return true;
}

// (2-5) view stamp: what the stick / cell selection and the measure groups were built from
void CDlgSubManual_Meas::StampHybridView()
{
	CRecipeEditMgr* pRcp = m_pAuroraMgr->GetRecipeEditMgrPtr();
	C_OutRecipeFunc* pFunc = pRcp->GetRecipeFuncPtr();
	m_strViewRcp = pRcp->GetRecipeID();
	m_nViewLoadSeq = (pFunc != NULL) ? pFunc->GetRcpLoadSeq() : -1;
	m_nViewSegCount = pRcp->GetSegmentLoopCount();
	m_nViewActive = pRcp->GetActiveSegment();
	m_bViewStamped = true;
}

bool CDlgSubManual_Meas::IsHybridViewStale()
{
	if (m_bViewStamped == false) return true;
	CRecipeEditMgr* pRcp = m_pAuroraMgr->GetRecipeEditMgrPtr();
	C_OutRecipeFunc* pFunc = pRcp->GetRecipeFuncPtr();
	if (m_strViewRcp != pRcp->GetRecipeID()) return true;
	if (pFunc == NULL || m_nViewLoadSeq != pFunc->GetRcpLoadSeq()) return true;
	if (m_nViewSegCount != pRcp->GetSegmentLoopCount()) return true;
	if (m_nViewActive != pRcp->GetActiveSegment()) return true;
	return false;
}

// G0 resync (worker, after the idle guard): stick 1 / cell 1 of the active segment, groups rebuilt, PG model unknown, new stamp. No RecipeChange
void CDlgSubManual_Meas::ResyncHybridView()
{
	m_pAuroraMgr->GetRecipeEditMgrPtr()->SyncCellStatusStickBase();
	m_iNumStick = 0;
	m_iNumCell = 0;
	MakeMeasGroup();
	m_strPgModelSent.Empty();
	StampHybridView();
	UpdateCurStickUI();
}

// tail of the refusal WARN lines: segment (-1 = unknown while a load runs) / stick (1-based), live stage pose, target pose, Glass Align mark
CString CDlgSubManual_Meas::HybridGateDetail(int nSeg, int nSegCnt, int iStick, bool bTarget, double dTgtY, double dTgtR)
{
	CMaintControlMgr* pM = m_pAuroraMgr->GetMaintControlMgr();
	double dRef = 0.0;
	CString strSeg, strTgt, strRef, strOut;
	if (nSeg >= 0) strSeg.Format(_T("%d"), nSeg + 1); else strSeg = _T("?");
	if (bTarget) strTgt.Format(_T("target X %.3f R %.3f"), dTgtY, dTgtR); else strTgt = _T("target -");
	if (pM->GetGlassAlignRefR(&dRef)) strRef.Format(_T("%.3f"), dRef); else strRef = _T("none");
	strOut.Format(_T("(seg %s/%d stick %d, X %.3f R %.3f, %s, align R %s)"), (LPCTSTR)strSeg, nSegCnt, iStick + 1,
		pM->GetCurPos(AXIS_X), pM->GetCurPos(AXIS_LOADER_R), (LPCTSTR)strTgt, (LPCTSTR)strRef);
	return strOut;
}

// one WARN line + the Korean popup (worker thread, like the :2058 interlock). No alarm. The caller restores the window and SPS_IDLE
void CDlgSubManual_Meas::RefuseHybrid(const CString& strWarn, const CString& strPopup)
{
	CLogWrapper::GetInstance()->KMLOG(WARN, strWarn);
	CD_DisplayMessage dlg(strPopup);
	dlg.DoModal();
}

// (3-3) / (14)-B dispatcher gate, worker thread right after EnableWindow(FALSE) and before the step case (which may change m_iNumStick).
//   Order: hybrid predicate, class, load running, T (fix review 5), G0-b, G0, PG model, G, target pose, R-A, (S passes), R-B, R-C. false = refused before any
//   axis moves (WARN + popup done here)
bool CDlgSubManual_Meas::CheckHybridGate(int nStep)
{
	CRecipeEditMgr* pRcp = m_pAuroraMgr->GetRecipeEditMgrPtr();
	if (pRcp == NULL) return true;
	C_OutRecipeFunc* pFunc = pRcp->GetRecipeFuncPtr();
	// a LoadRcpFile in another thread (CIM PPID / DLL TestMode) rewrites the segment vectors: wait up to 2 s so the segment count can be read
	//   (a 1-segment recipe is then judged like any other time instead of being refused)
	for (int i = 0; i < 200 && pFunc != NULL && pFunc->IsRcpLoadBusy() == TRUE; i++) Sleep(10);
	bool bLoadBusy = (pFunc != NULL && pFunc->IsRcpLoadBusy() == TRUE);
	int nLive = (pFunc != NULL && bLoadBusy == false) ? pFunc->GetSegmentLoopCount() : 1;
	int nCache = pRcp->GetSegCountCache();
	if (bLoadBusy == false && nCache <= 1 && nLive <= 1) return true;						// D7-4 (a): 1 segment - no log, unchanged path
	if (bLoadBusy == false && nLive <= 1) { pRcp->SyncCellStatusStickBase(); return true; }	// stale cache over a 1-segment reload: silent, no log

	int nClass = HybridStepClass(nStep);
	if (nClass == HYB_STEP_NONE) return true;
	CString strWarn, strPop;
	int nSegCnt = (bLoadBusy == false) ? nLive : nCache;
	int nAct = (bLoadBusy == false) ? pFunc->GetActiveSegmentIndex() : -1;
	// LYS 2026-10-13 Hybrid Phase 7 fix (impl review 5): a load still running after the 2 s wait above is refused before R-T (R-T then always has the
	//   segment index). A 1-segment recipe whose load runs > 2 s is refused here too - safe direction, practically unreachable (EnableWindow(FALSE) waits
	//   for any UI-thread load); documented for 7-7
	if (bLoadBusy)
	{
		strWarn.Format(_T("[HYBRID] Manual Meas step %d refused - stale (recipe load running) %s"), nStep, (LPCTSTR)HybridGateDetail(nAct, nSegCnt, m_iNumStick, false, 0.0, 0.0));
		RefuseHybrid(strWarn, _T("레시피를 다시 읽는 중입니다(다른 화면 또는 CIM).\n수동 동작을 하지 않습니다. 잠시 후 다시 실행하십시오."));
		return false;
	}
	if (nClass == HYB_STEP_T)		// R-T (D7-13 (a))
	{
		strWarn.Format(_T("[HYBRID] Manual Meas step %d refused - unsupported %s"), nStep, (LPCTSTR)HybridGateDetail(nAct, nSegCnt, m_iNumStick, false, 0.0, 0.0));
		RefuseHybrid(strWarn, _T("하이브리드 레시피에서는 이 수동 동작(TSP / LCR / Probe 이동)을 지원하지 않습니다."));
		return false;
	}
	if (pFunc->IsSegmentAlignLoaded(nAct) != TRUE)	// G0-b: a reload bypassed RecipeChange (DLL TestMode) - RecipeID / segment .alg are stale, no resync
	{
		strWarn.Format(_T("[HYBRID] Manual Meas step %d refused - reload bypassed RecipeChange (segment %d .alg not loaded) - reopen the recipe"), nStep, nAct + 1);
		RefuseHybrid(strWarn, _T("레시피가 다른 경로(TestMode 등)로 다시 로드되어 세그먼트 Align 파일이 없습니다.\n수동 동작을 하지 않습니다.\n\n[레세피 선택]으로 레시피를 다시 여십시오."));
		return false;
	}
	if (nCache <= 1 || IsHybridViewStale())		// G0: reloaded elsewhere (auto run, CIM, another screen) - resync, refuse this one step
	{
		ResyncHybridView();
		strWarn.Format(_T("[HYBRID] Manual Meas view resynced (recipe %s load %d seg %d/%d) - step %d not run"), (LPCTSTR)pRcp->GetRecipeID(), pFunc->GetRcpLoadSeq(),
			pRcp->GetActiveSegment() + 1, pRcp->GetSegmentLoopCount(), nStep);
		RefuseHybrid(strWarn, _T("레시피가 다시 로드되었습니다(다른 화면 또는 자동 운전).\n스틱·셀 선택을 처음(스틱 1)으로 되돌렸습니다.\n위치를 확인한 뒤 다시 실행하십시오."));
		return false;
	}
	if (IsPgStep(nStep))	// D7-20 (a): the PG device must hold this segment's model ([PG recipe change] = RECIPE_OPEN_PG)
	{
		CString strPgModel = pRcp->GetPGRecipeName();
		if (strPgModel.CompareNoCase(m_strPgModelSent) != 0)
		{
			strWarn.Format(_T("[HYBRID] Manual Meas step %d refused - pg model (sent [%s], segment [%s])"), nStep, (LPCTSTR)m_strPgModelSent, (LPCTSTR)strPgModel);
			strPop.Format(_T("PG 장치에 이 세그먼트의 PG 모델(%s)을 보냈는지 확인되지 않았습니다.\n(세그먼트 %d/%d)\n\n[PG 레세피 변경]을 먼저 실행하십시오."), (LPCTSTR)strPgModel, nAct + 1, nLive);
			RefuseHybrid(strWarn, strPop);
			return false;
		}
	}
	if (nClass == HYB_STEP_PG) return true;
	if (nClass == HYB_STEP_G)
	{
		if (nStep == enumStepID_GLASS_ALIGN)	// (14) C-10: a Probe .alg may have replaced the active m_sAlign - read the segment .alg again first
		{
			CString strAlg = pFunc->GetSegmentAlignName(nAct);
			if (pRcp->OpenAlignRecipe(strAlg) != VTM_RV_SUCCESSED)
			{
				strWarn.Format(_T("[HYBRID] Manual Meas step %d refused - alg-open (segment %d/%d .alg [%s] not read)"), nStep, nAct + 1, nLive, (LPCTSTR)strAlg);
				strPop.Format(_T("세그먼트 %d/%d의 Align 파일(%s)을 읽지 못했습니다.\nALIGN을 하지 않습니다."), nAct + 1, nLive, (LPCTSTR)strAlg);
				RefuseHybrid(strWarn, strPop);
				return false;
			}
			CString strAlgLog;
			strAlgLog.Format(_T("[HYBRID] Manual Meas glass align: segment %d/%d .alg [%s] read again"), nAct + 1, nLive, (LPCTSTR)strAlg);
			CLogWrapper::GetInstance()->KMLOG(OPER, strAlgLog);
		}
		return true;
	}

	int iStick = (nClass == HYB_STEP_S) ? PeekTargetStick(nStep) : m_iNumStick;
	double dY = 0.0, dR = 0.0;
	if (CalcStickPose(iStick, dY, dR) == false)
	{
		if (nClass == HYB_STEP_S) return true;		// ManualMoveStick guards the index itself and moves nothing
		strWarn.Format(_T("[HYBRID] Manual Meas step %d refused - pose (stick %d not in this segment) %s"), nStep, iStick + 1, (LPCTSTR)HybridGateDetail(nAct, nLive, iStick, false, 0.0, 0.0));
		strPop.Format(_T("스테이지가 세그먼트 %d/%d 스틱 %d 위치에 있지 않습니다.\n(스틱 %d은 이 세그먼트에 없습니다)\n\n스틱 이동을 먼저 하십시오."), nAct + 1, nLive, iStick + 1, iStick + 1);
		RefuseHybrid(strWarn, strPop);	// (14) CS-11: P / C never run without a target pose
		return false;
	}
	CMaintControlMgr* pM = m_pAuroraMgr->GetMaintControlMgr();
	double dRef = 0.0;
	bool bRef = pM->GetGlassAlignRefR(&dRef);
	if (bRef && fabs(dR - dRef) > 2.0)	// R-A (D7-6: 2.0 deg = the motion layer's rotate threshold). Mark and target are the same axis coordinate, no modulo
	{
		strWarn.Format(_T("[HYBRID] Manual Meas step %d refused - orientation %s"), nStep, (LPCTSTR)HybridGateDetail(nAct, nLive, iStick, true, dY, dR));
		strPop.Format(_T("Glass Align은 R %.1f 방향에서 한 값입니다.\n이동할 스틱(세그먼트 %d/%d, 스틱 %d)은 R %.1f 방향이라 이동하지 않습니다.\n\nMOVE SAFETY 후 ALIGN 동작으로 이 방향을 다시 정렬하십시오."),
			dRef, nAct + 1, nLive, iStick + 1, dR);
		RefuseHybrid(strWarn, strPop);
		return false;
	}
	if (nClass == HYB_STEP_S) return true;
	double dCurY = pM->GetCurPos(AXIS_X), dCurR = pM->GetCurPos(AXIS_LOADER_R);
	if (fabs(dCurY - dY) > 0.1 || fabs(dCurR - dR) > 0.02)	// R-B (D7-6: X 0.1 mm, R 0.02 deg - (14) C-11)
	{
		strWarn.Format(_T("[HYBRID] Manual Meas step %d refused - pose %s"), nStep, (LPCTSTR)HybridGateDetail(nAct, nLive, iStick, true, dY, dR));
		// LYS 2026-10-14 popup: current / target position each on its own line (user request)
		strPop.Format(_T("스테이지가 세그먼트 %d/%d 스틱 %d 위치에 있지 않습니다.\n\n현재 위치: X %.3f, R %.3f\n목표 위치: X %.3f, R %.3f\n\n스틱 이동을 먼저 하십시오."),
			nAct + 1, nLive, iStick + 1, dCurY, dCurR, dY, dR);
		RefuseHybrid(strWarn, strPop);
		return false;
	}
	S_RecipeInfo* pInfo = pRcp->GetRecipeInfo();
	if (nClass == HYB_STEP_C && pInfo != NULL && pInfo->m_sOptionItem.m_bUseAlign == TRUE && bRef == false)	// R-C (D7-5 (a)): same-direction align required
	{
		strWarn.Format(_T("[HYBRID] Manual Meas step %d refused - align (ALIGN USE on, no glass align mark) %s"), nStep, (LPCTSTR)HybridGateDetail(nAct, nLive, iStick, true, dY, dR));
		strPop.Format(_T("세그먼트 %d/%d는 ALIGN USE가 켜져 있지만 이 방향(R %.1f)의 Glass Align이 없습니다.\n컨택·측정 셋팅을 하지 않습니다.\n\nMOVE SAFETY 후 ALIGN 동작을 먼저 하십시오."), nAct + 1, nLive, dR);
		RefuseHybrid(strWarn, strPop);
		return false;
	}
	return true;
}

// English text of one Cell ID error for the log - copy of DoWorkGlassRecipeStep.cpp CellIdErrText (that file stays unchanged)
static CString CellIdErrText(const S_CellIdError& e, int nGlassIdLen)
{
	CString strWhere, strWhy;
	strWhere.Format(_T("Segment %d Stick %d Cell %d (%c)"), e.m_nSeg + 1, e.m_nStick + 1, e.m_nCell + 1, e.m_nPanel ? _T('B') : _T('A'));
	switch (e.m_nCode)
	{
	case M_CELLID_ERR_EMPTY:  strWhy = _T("empty"); break;
	case M_CELLID_ERR_LENGTH: strWhy.Format(_T("'%s' longer than %d chars (cell ID max 31, GlassID length %d)"), e.m_strToken.GetString(), e.m_nLimit, nGlassIdLen); break;
	case M_CELLID_ERR_FORMAT: strWhy.Format(_T("'%s' is not A-Z letters + digits"), e.m_strToken.GetString()); break;
	case M_CELLID_ERR_DERIVE: strWhy.Format(_T("B cell ID cannot be derived from A manual ID '%s'"), e.m_strToken.GetString()); break;
	case M_CELLID_ERR_DUP:    strWhy.Format(_T("duplicate '%s' (Segment %d Stick %d Cell %d (%c))"), e.m_strToken.GetString(),
								  e.m_nSeg2 + 1, e.m_nStick2 + 1, e.m_nCell2 + 1, e.m_nPanel2 ? _T('B') : _T('A')); break;
	default:                  strWhy.Format(_T("error %d"), e.m_nCode); break;
	}
	return strWhere + _T(": ") + strWhy;
}

// D4b-9 / D7-12 (b) manual cell ID check right before a measure set (hybrid runtime recipe only). Same order as CheckCellIdConfig
//   (DoWorkGlassRecipeStep.cpp): load try / busy / serial, ValidateCellIdConfig, busy / try again. Pass: latch 1 bound to this load + Glass ID on
//   every pass ((14) CS-13: RecipeChange, a failed auto check and a mid-glass invalidation move it), one OPER line per load serial + Glass ID.
//   Fail: WARN + popup, the measure set is not run
bool CDlgSubManual_Meas::ValidateManualCellId()
{
	if (IsHybridRuntime() == false) return true;	// D7-12 (b): 1-segment recipe - nothing, no log (manual measure keeps Auto cell IDs)
	CRecipeEditMgr* pRcp = m_pAuroraMgr->GetRecipeEditMgrPtr();
	C_OutRecipeFunc* pFunc = pRcp->GetRecipeFuncPtr();
	if (pFunc == NULL) return true;
	int nLoadTry = pFunc->GetRcpLoadTry();	// try count first, then the busy count (LoadRcpFile raises them the other way round)
	BOOL bLoadBusy = pFunc->IsRcpLoadBusy();
	int nLoadSeq = pFunc->GetRcpLoadSeq();
	if (pFunc->IsCellIdDefault()) return true;		// Auto/Auto: nothing to check, no log
	CString strGlassID = CSystemParamShare::GetInstance()->GetGlassID();
	CString strGidLog = strGlassID.IsEmpty() ? CString(_T("(none)")) : strGlassID;
	int nGid = strGlassID.GetLength(), nLongest = 0;
	std::vector<S_CellIdError> vecErr;
	CString strLog, strPop;
	if (pFunc->ValidateCellIdConfig(nGid, vecErr, &nLongest) != TRUE)
	{
		for (size_t i = 0; i < vecErr.size() && i < 10; i++)
			CLogWrapper::GetInstance()->KMLOG(WARN, _T("[CELLID] ") + CellIdErrText(vecErr[i], nGid));
		if (vecErr.size() > 10)
		{
			strLog.Format(_T("[CELLID] ... %d more"), (int)vecErr.size() - 10);
			CLogWrapper::GetInstance()->KMLOG(WARN, strLog);
		}
		strLog.Format(_T("[CELLID] manual measure: cell ID check failed (%d errors, GlassID %s) - measure set skipped"), (int)vecErr.size(), (LPCTSTR)strGidLog);
		CLogWrapper::GetInstance()->KMLOG(WARN, strLog);
		pRcp->SetCellIdLatch(-1);
		m_nCidValSeq = -1;
		strPop.Format(_T("Cell ID 설정 검사에 실패했습니다 (%d건).\n측정 셋팅을 하지 않습니다. 로그 [CELLID]를 확인하십시오."), (int)vecErr.size());
		CD_DisplayMessage dlg(strPop);
		dlg.DoModal();
		return false;
	}
	if (bLoadBusy == TRUE || pFunc->IsRcpLoadBusy() == TRUE || pFunc->GetRcpLoadTry() != nLoadTry)	// a load ran while the config was read
	{
		strLog.Format(_T("[CELLID] manual measure: recipe reloaded during the cell ID check (recipe load %d/%d, GlassID %s) - measure set skipped"), nLoadSeq, pFunc->GetRcpLoadSeq(), (LPCTSTR)strGidLog);
		CLogWrapper::GetInstance()->KMLOG(WARN, strLog);
		pRcp->SetCellIdLatch(-1);
		m_nCidValSeq = -1;
		CD_DisplayMessage dlg(_T("Cell ID 검사 중 레시피가 다시 로드되었습니다.\n측정 셋팅을 하지 않습니다. 다시 실행하십시오."));
		dlg.DoModal();
		return false;
	}
	pRcp->SetCellIdLatch(1, nLoadSeq, strGlassID);	// GetManualCellToken compares the load serial and the Glass ID
	if (m_nCidValSeq != nLoadSeq || m_strCidValGlass != strGlassID)
	{
		bool bManA = (pFunc->GetCellIdMode(0) == M_CELLID_MODE_MANUAL), bManB = (pFunc->GetCellIdMode(1) == M_CELLID_MODE_MANUAL);
		bool bPanelB = (strGlassID.Mid(nGid - 2, 1).CompareNoCase(_T("B")) == 0);	// same rule as GenCellID
		strLog.Format(_T("[CELLID] manual cell ID: A %s, B %s, glass panel %c (GlassID %s), longest ID %d/31 chars (manual measure)"),
			bManA ? _T("MANUAL") : _T("AUTO"), bManB ? _T("MANUAL") : (bManA ? _T("AUTO (from A)") : _T("AUTO")),
			bPanelB ? _T('B') : _T('A'), (LPCTSTR)strGidLog, ((nGid > 0) ? nGid - 1 : 0) + nLongest + 1);
		CLogWrapper::GetInstance()->KMLOG(OPER, strLog);
		m_nCidValSeq = nLoadSeq;
		m_strCidValGlass = strGlassID;
	}
	return true;
}

// (3-6) / (14) CS-12 result rows of the active segment: segBase added to every head row > 0 (the auto rule adds it on both heads,
//   DoWorkGlassMeasureStep.cpp; the manual resting-head row nArea * stick stays in the active segment block). 1 segment: base 0 -> returns at once
void CDlgSubManual_Meas::AddSegRowBase(int* pnAreaNum)
{
	if (pnAreaNum == NULL) return;
	int nBase = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetResultRowBase();
	if (nBase == 0) return;
	for (int h = 0; h < DF_MAX_HEAD; h++)
	{
		if (pnAreaNum[h] > 0) pnAreaNum[h] += nBase;
	}
}

//=========================================================================
// LYS 2026-10-13 manual meas = CycleTest positions (manualpos/rca.md (5)(c) + (8) user decisions; verify_fix #1 #4 #5 #9 #10 #11).
//   On OCP the SPEC / QUALITY / ANGLE move buttons and their next cell call the builders of the auto Measure steps (MeasBuildGroups /
//   MeasBuildMovePos), so they command the CycleTest positions: D1 2-head pairs (WAD 444 / VISION 500 / ANGLE 1400 here), no cell filter (D3),
//   D6 (b) the full move array on every press, D10 WAD result Y = auto. REVIEW / COLOR (D4), VISION_COLOR on (verify_fix #5) and HMI mode (D5)
//   keep the legacy code. Safety (8): 1-segment pose check before these moves, Sub Z / FRONT Z raised before ManualMoveStick moves the stage
//=========================================================================

// HEAD_OFFSET mode of a manual move type (MeasBuildGroups / MeasBuildMovePos / GetHeadOffset): 0 WAD / 1 ANGLE / 2 VISION
static int ManualMeasHeadMode(int iMode)
{
	if (iMode == M_TYPE_SPEC) return 0;
	if (iMode == M_TYPE_ANGLE) return 1;
	return 2;
}

static LPCTSTR ManualMeasLogName(int iMode)
{
	if (iMode == M_TYPE_SPEC) return _T("WAD");
	if (iMode == M_TYPE_ANGLE) return _T("ANGLE");
	return _T("VISION");
}

static LPCTSTR ManualMeasPopupName(int iMode)
{
	if (iMode == M_TYPE_SPEC) return _T("특성");
	if (iMode == M_TYPE_ANGLE) return _T("시야각");
	return _T("화질");
}

static int ManualRcpLoadSeq(CRecipeEditMgr* pRcp)
{
	C_OutRecipeFunc* pFunc = (pRcp != NULL) ? pRcp->GetRecipeFuncPtr() : NULL;
	return (pFunc != NULL) ? pFunc->GetRcpLoadSeq() : -1;
}

// verify_fix #4: the axes MoveCmdPosWAD / MoveCmdPosVision / MoveCmdPosAngle really command. The manual move waits on these only: VISION FILTER1/2
//   (no branch in MoveCmdPosVision - a servo-off filter would time out after 30 s), Z1 (commanded by neither MoveCmdPosVision nor MoveCmdPosAngle)
//   and the zeroed ANGLE slots 0/1 (AXIS_X) are skipped. Disabled axes (angle optics not fitted) return at once in DoWork_MotionDone
static bool ManualMeasWaitAxis(int nMode, int iAxis)
{
	if (iAxis == AXIS_Y1 || iAxis == AXIS_Y2 || iAxis == AXIS_Y3 || iAxis == AXIS_Y4 || iAxis == AXIS_X2 || iAxis == AXIS_X3 || iAxis == AXIS_Z2) return true;
	if (nMode == 0) return (iAxis == AXIS_Z3 || iAxis == AXIS_Z4);
	if (iAxis == AXIS_Z5 || iAxis == AXIS_Z6) return true;
	if (nMode == 2) return (iAxis == AXIS_ZOOM1 || iAxis == AXIS_THETA1 || iAxis == AXIS_ZOOM2 || iAxis == AXIS_THETA2);
	return (iAxis >= AXIS_ANGLE_SF1 && iAxis <= AXIS_ANGLE_AZM2);
}

// verify_fix #11: MoveCmdPosWAD / MoveCmdPosAngle send X2 and X3 in one Multi command, and the Multi path has no gantry gap check (only
//   MoveCmdPosA_Sub / MoveCmdPosR_Sub have it). The gantry whose move opens the gap to the other one goes first with MoveCmdPosA (gap-checked,
//   waits); the Multi then re-commands it in place. Targets as those functions compute them on OCP: WAD X2 = array + HEAD1_WAD_X with
//   X3 = SafetyHeadX2, ANGLE X3 = array + HEAD1_ANGLE_X with X2 = SafetyHeadX1. VISION orders its gantries itself (MoveCmdPosA, X2 first when it
//   rises). Returns success when nothing has to go first. The target array is not changed
static long ManualGantryFirst(CMaintControlMgr* pMaint, CPositionConfigMgr* pPosCfg, int nMode, const stMotorPosition* pPos, int nAxis)
{
	if (pMaint == nullptr || pPosCfg == nullptr || pMaint->GetMotorControlMgrPtr() == nullptr || pPos == nullptr) return VTM_RV_SUCCESSED;
	if (nMode != 0 && nMode != 1) return VTM_RV_SUCCESSED;
	int iPrimary = -1;
	double dArrayX = 0.0;
	for (int k = 0; k < nAxis; k++)
	{
		if (pPos[k].axis == AXIS_X2 || pPos[k].axis == AXIS_X3)
		{
			iPrimary = pPos[k].axis;
			dArrayX = pPos[k].dPosition;
			break;
		}
	}
	double dOffX = 0.0, dOffY = 0.0, dOffZ = 0.0;
	pPosCfg->GetHeadOffset(0, nMode, &dOffX, &dOffY, &dOffZ);
	double dSafetyX2 = 0.0, dSafetyX3 = 0.0;	// Position.ini SafetyHeadX1 (BACK GANTRY X2) / SafetyHeadX2 (FRONT GANTRY X3)
	pPosCfg->GetSafetyPosition(nullptr, &dSafetyX2, &dSafetyX3, nullptr, nullptr, nullptr, nullptr);
	double dTgtX2 = 0.0, dTgtX3 = 0.0;
	if (nMode == 0 && iPrimary == AXIS_X2)
	{
		dTgtX2 = dArrayX + dOffX;
		dTgtX3 = dSafetyX3;
	}
	else if (nMode == 1 && iPrimary == AXIS_X3)
	{
		dTgtX3 = dArrayX + dOffX;
		dTgtX2 = dSafetyX2;
	}
	else
	{
		return VTM_RV_SUCCESSED;	// no gantry in the array / an HMI-mode layout (not on the OCP shared path)
	}

	CMotorControlMgr* pMotor = pMaint->GetMotorControlMgrPtr();
	double dCurX2 = pMotor->GetOrgPos(AXIS_X2), dCurX3 = pMotor->GetOrgPos(AXIS_X3);	// raw, as the gap interlock reads them
	bool bMoveX2 = (fabs(dTgtX2 - dCurX2) > 0.01), bMoveX3 = (fabs(dTgtX3 - dCurX3) > 0.01);
	if (bMoveX2 && fabs(dTgtX2 - dCurX3) > fabs(dCurX2 - dCurX3))
		return pMaint->MoveCmdPosA(AXIS_X2, dTgtX2, true, SPEED_HIGH);	// X2 moves away from X3
	if (bMoveX3 && fabs(dTgtX3 - dCurX2) > fabs(dCurX3 - dCurX2))
		return pMaint->MoveCmdPosA(AXIS_X3, dTgtX3, true, SPEED_HIGH);	// X3 moves away from X2
	return VTM_RV_SUCCESSED;
}

// D5 OCP only, D4 SPEC / QUALITY / ANGLE only. verify_fix #5: with USE_VISION_COLOR the auto QUALITY runs the VISION_COLOR step (not shared), so
//   manual QUALITY stays legacy then
bool CDlgSubManual_Meas::UseSharedMeasPos(int iMode)
{
	if (iMode != M_TYPE_SPEC && iMode != M_TYPE_QUALITY && iMode != M_TYPE_ANGLE) return false;
	CSystemConfigMgr* pSys = m_pAuroraMgr->GetSystemConfigMgrPtr();
	if (pSys == nullptr || pSys->IsOCP() == false) return false;
	if (iMode == M_TYPE_QUALITY && pSys->IsUseInspVisionColor()) return false;
	return true;
}

// the groups held are the ones a move of iMode uses now: shared types - this type, this stick, this recipe load; legacy types - the legacy groups
bool CDlgSubManual_Meas::IsMeasGroupCurrent(int iMode)
{
	if (UseSharedMeasPos(iMode) == false) return (m_nMeasGroupMode == -1);
	return (m_nMeasGroupMode == iMode && m_nMeasGroupStick == m_iNumStick && m_nMeasGroupLoadSeq == ManualRcpLoadSeq(m_pAuroraMgr->GetRecipeEditMgrPtr()));
}

// worker thread, only right after m_iNumCell = 0 (MOVE_CELL, MOVE_CELL_NEXT with a replaced group set) - the measure set / test steps index the
//   groups with m_iNumCell unchecked. Shared types: the auto builder for stick m_iNumStick of the active segment (GetRecipeData, as the auto
//   GetMeasureStick + active segment), all cells (D3), D1 2-head pairs; logs "[MANUAL_WAD|MANUAL_ANGLE|MANUAL_VISION] HeadPair by order ...".
//   verify_fix #1: no recipe or m_iNumStick outside m_vecStick (the stick index survives a recipe change) -> groups emptied, WARN + popup.
//   Legacy types: the legacy groups are put back when shared ones were in place
int CDlgSubManual_Meas::EnsureMeasGroup(int iMode)
{
	if (UseSharedMeasPos(iMode) == false)
	{
		if (m_nMeasGroupMode != -1) MakeMeasGroup(true);	// MakeMeasGroup resets the stamp to -1. LYS 2026-10-13 manual meas = CycleTest positions (review fix #7): true keeps the inner IDs of this stick / load
		return (int)m_vecMeasGroup.size();
	}
	CRecipeEditMgr* pRcp = m_pAuroraMgr->GetRecipeEditMgrPtr();
	S_RecipeInfo* pInfo = (pRcp != NULL) ? pRcp->GetRecipeInfo() : NULL;
	S_RecipeData* pData = (pRcp != NULL) ? pRcp->GetRecipeData() : NULL;
	int nStickCnt = (pData != NULL) ? (int)pData->m_vecStick.size() : 0;
	if (pInfo == NULL || pData == NULL || m_iNumStick < 0 || m_iNumStick >= nStickCnt)
	{
		m_vecMeasGroup.clear();
		SetMeasureArea(0);
		m_nMeasGroupMode = -2;	// refused: the next move of any type builds again (a legacy type restores the legacy groups)
		m_nMeasGroupStick = -1;
		m_nMeasGroupLoadSeq = -1;
		CString strWarn, strPop;
		strWarn.Format(_T("[MANUAL] Manual Meas %s groups not built - stick %d not in the recipe (%d sticks)%s"), ManualMeasLogName(iMode), m_iNumStick + 1, nStickCnt,
			(pInfo == NULL || pData == NULL) ? _T(", no recipe data") : _T(""));
		strPop.Format(_T("현재 스틱 %d이 레시피(스틱 %d개)에 없어 %s 위치로 이동하지 않습니다.\n\n[레세피 선택] 후 스틱 이동을 먼저 하십시오."), m_iNumStick + 1, nStickCnt, ManualMeasPopupName(iMode));
		RefuseHybrid(strWarn, strPop);	// one WARN line + the popup (worker thread), no alarm
		return 0;
	}
	int nMode = ManualMeasHeadMode(iMode);
	LPCTSTR pszTag = (nMode == 0) ? _T("MANUAL_WAD") : ((nMode == 1) ? _T("MANUAL_ANGLE") : _T("MANUAL_VISION"));
	vector<S_MeasPos> vecGroup;
	MeasBuildGroups(nMode, m_pAuroraMgr->GetPositionConfigMgrPtr(), m_pAuroraMgr->GetMaintControlMgr(), pInfo, pData, m_iNumStick, nullptr, pszTag, vecGroup);
	m_vecMeasGroup.swap(vecGroup);
	SetInnerIdArea((int)m_vecMeasGroup.size() * 2, true);	// LYS 2026-10-13 manual meas = CycleTest positions (review fix #7): was SetMeasureArea - a new inner ID per press and type
	m_nMeasGroupMode = iMode;
	m_nMeasGroupStick = m_iNumStick;
	m_nMeasGroupLoadSeq = ManualRcpLoadSeq(pRcp);
	if (m_vecMeasGroup.empty())
	{
		CString strWarn;
		strWarn.Format(_T("[MANUAL] Manual Meas %s: stick %d has no cells - nothing to move"), ManualMeasLogName(iMode), m_iNumStick + 1);
		CLogWrapper::GetInstance()->KMLOG(WARN, strWarn);
	}
	return (int)m_vecMeasGroup.size();
}

// LYS 2026-10-13 manual meas = CycleTest positions (review fix #7): the inner ID array (m_pStInnerID, indexed by the cell number in the stick - GenInnerID)
//   was reset only by a recipe open before (legacy MakeMeasGroup). bKeep (shared builds, legacy restore after a shared move): kept while the stick and
//   the recipe load are the ones it was made for, so a cell keeps its inner ID across presses and move types (auto keeps it for the whole stick); grown in
//   place when a type needs more slots. Another stick / load, no array, or bKeep false (RECIPE_OPEN, Phase 7a resync) -> SetMeasureArea (reset)
void CDlgSubManual_Meas::SetInnerIdArea(int nNeed, bool bKeep)
{
	int nSeq = ManualRcpLoadSeq(m_pAuroraMgr->GetRecipeEditMgrPtr());
	if (bKeep && m_pStInnerID != NULL && m_nInnerIdStick == m_iNumStick && m_nInnerIdLoadSeq == nSeq)
	{
		if (nNeed > m_nMeasureArea)
		{
			TCHAR** pGrow = new TCHAR * [nNeed];
			for (int i = 0; i < nNeed; i++)
			{
				if (i < m_nMeasureArea)
					pGrow[i] = m_pStInnerID[i];	// kept inner ID
				else
				{
					pGrow[i] = new TCHAR[32];
					memset(pGrow[i], 0x00, 32 * sizeof(TCHAR));
				}
			}
			delete[] m_pStInnerID;	// the pointer table only - its strings moved to pGrow
			m_pStInnerID = pGrow;
			m_nMeasureArea = nNeed;
		}
	}
	else
	{
		SetMeasureArea(nNeed);
	}
	m_nInnerIdStick = m_iNumStick;
	m_nInnerIdLoadSeq = nSeq;
}

// (8) safety: on a 1-segment runtime the shared moves run only when the stage is at the current stick's pose (Phase 7a CalcStickPose, X 0.1 mm /
//   R 0.02 deg - the hybrid R-B values, same live axes). Hybrid runtime: CheckHybridGate R-B already checked this step before the step switch -
//   7a behaviour unchanged, nothing here. false = refused before any axis moves (WARN + popup, no alarm)
bool CDlgSubManual_Meas::CheckManualMeasPose(int iMode)
{
	if (IsHybridRuntime()) return true;
	CString strWarn, strPop;
	double dY = 0.0, dR = 0.0;
	if (CalcStickPose(m_iNumStick, dY, dR) == false)
	{
		CRecipeEditMgr* pRcp = m_pAuroraMgr->GetRecipeEditMgrPtr();
		S_RecipeData* pData = (pRcp != NULL) ? pRcp->GetRecipeData() : NULL;
		int nStickCnt = (pData != NULL) ? (int)pData->m_vecStick.size() : 0;
		strWarn.Format(_T("[MANUAL] Manual Meas %s move refused - stick %d not in the recipe (%d sticks)"), ManualMeasLogName(iMode), m_iNumStick + 1, nStickCnt);
		strPop.Format(_T("현재 스틱 %d이 레시피(스틱 %d개)에 없어 %s 위치로 이동하지 않습니다.\n\n[레세피 선택] 후 스틱 이동을 먼저 하십시오."), m_iNumStick + 1, nStickCnt, ManualMeasPopupName(iMode));
		RefuseHybrid(strWarn, strPop);
		return false;
	}
	CMaintControlMgr* pM = m_pAuroraMgr->GetMaintControlMgr();
	double dCurY = pM->GetCurPos(AXIS_X), dCurR = pM->GetCurPos(AXIS_LOADER_R);
	if (fabs(dCurY - dY) > 0.1 || fabs(dCurR - dR) > 0.02)
	{
		strWarn.Format(_T("[MANUAL] Manual Meas %s move refused - stage not at stick %d pose (X %.3f R %.3f, target X %.3f R %.3f)"),
			ManualMeasLogName(iMode), m_iNumStick + 1, dCurY, dCurR, dY, dR);
		// LYS 2026-10-14 popup: current / target position each on its own line (user request)
		strPop.Format(_T("스테이지가 스틱 %d 위치에 있지 않아 %s 위치로 이동하지 않습니다.\n\n현재 위치: X %.3f, R %.3f\n목표 위치: X %.3f, R %.3f\n\n스틱 이동을 먼저 하십시오."),
			m_iNumStick + 1, ManualMeasPopupName(iMode), dCurY, dCurR, dY, dR);
		RefuseHybrid(strWarn, strPop);
		return false;
	}
	return true;
}

// one shared move - the move button and every next cell (D6 (b): the full array each time; axes already in place are re-commanded and do not
//   move). Same array, nAxis and calls as the auto step: MarkIdleHeadSafety, and for WAD SetMeasPos with the auto result Y (dHeadBackG, D10) first.
//   Then the departing gantry first (verify_fix #11), MoveCmdPos*(..., false) and a wait on the axes really commanded (verify_fix #4)
int CDlgSubManual_Meas::ManualMoveCellShared(int iMode, int iMove)
{
	CRecipeEditMgr* pRcp = m_pAuroraMgr->GetRecipeEditMgrPtr();
	S_RecipeInfo* pInfo = (pRcp != NULL) ? pRcp->GetRecipeInfo() : NULL;
	CMaintControlMgr* pMaint = m_pAuroraMgr->GetMaintControlMgr();
	int nMeasArea = (int)m_vecMeasGroup.size();
	if (pInfo == NULL || pMaint == NULL || m_nMeasGroupMode != iMode || iMove < 0 || iMove >= nMeasArea) return VTM_RV_FAILED;

	int nMode = ManualMeasHeadMode(iMode);
	const S_MeasPos& grp = m_vecMeasGroup[iMove];
	stMotorPosition stPosition[22];
	ZeroMemory(&stPosition, sizeof(stPosition));	// as the auto steps - ANGLE slots 0/1 stay zeroed (skipped by the wait). Never ClearMotorPos here
	double dLeft = 0., dRight = 0.;
	int nAxis = MeasBuildMovePos(nMode, grp, nMeasArea, true, true, pInfo, pInfo->m_sMotionOffsetPos.m_fStageY,
		stPosition, (int)_countof(stPosition), &dLeft, &dRight);	// bFirst = true (D6 b), bOCP = true (UseSharedMeasPos is OCP only)
	if (nAxis <= 0) return VTM_RV_FAILED;

	if (nMode == 0)
	{
		double dXPos = grp.dHeadBackG - pMaint->GetCurPos(AXIS_X) - pInfo->m_sMotionOffsetPos.m_fHeadReviewY;	// D10: the auto WAD result Y
		m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->SetMeasPos(enumTCPIpFactorytID_WAD, dXPos, dLeft, dRight);
	}
	MarkIdleHeadSafety(stPosition, nAxis, grp, 0.0);	// as auto: the idle head of a single-cell group -> [SAFETY]
	long nRet = ManualGantryFirst(pMaint, m_pAuroraMgr->GetPositionConfigMgrPtr(), nMode, stPosition, nAxis);
	if (nRet == VTM_RV_SUCCESSED)
	{
		if (nMode == 0)
			nRet = pMaint->MoveCmdPosWAD(stPosition, nAxis, SPEED_HIGH, false);
		else if (nMode == 1)
			nRet = pMaint->MoveCmdPosAngle(stPosition, nAxis, SPEED_HIGH, false);
		else	// LYS 2026-10-14 BACK GANTRY stays at safety (no head offset, no re-command when already there)
			nRet = pMaint->MoveCmdPosVision(stPosition, nAxis, SPEED_HIGH, false, true);
	}
	if (nRet == VTM_RV_SUCCESSED)
	{
		for (int k = (nMode == 1) ? 2 : 0; k < nAxis; k++)	// ANGLE: slots 0/1 are the zeroed dummies of the auto layout
		{
			if (ManualMeasWaitAxis(nMode, stPosition[k].axis) == false) continue;
			nRet = pMaint->DoWork_MotionDone(stPosition[k].axis);
			if (nRet != df_MOTION_RV_SUCCESSED) break;
		}
	}
	return (int)nRet;
}

// LYS 2026-10-13 manual meas review #6 (user decision): GLASS GET / PUT loader move, GLASS ALIGN, PROBE ALIGN and PROBE MOVE turn or move the
//   stage / probe like a stick move, so on OCP they raise Sub Z / FRONT Z first with the same helper (only axes below safety; none = no
//   command, no log). HMI mode: no-op. A failed raise returns its code and the step body is skipped (the worker raises the alarm)
long CDlgSubManual_Meas::RetractHeadZBeforeStep(LPCTSTR pszWhat)
{
	if (m_pAuroraMgr->GetSystemConfigMgrPtr() == nullptr || m_pAuroraMgr->GetSystemConfigMgrPtr()->IsOCP() == false) return VTM_RV_SUCCESSED;
	return RetractHeadZBeforeStage(-1, pszWhat);
}

// (8) safety, verify_fix #3 (OCP; ManualMoveStick calls it after its "already on target" return, before the stage turns or moves): the shared
//   moves leave Sub Z (Z3/Z4 WAD, Z5/Z6 VISION / ANGLE) and FRONT Z at recipe height. Each of them below its safety height is raised first:
//   FRONT Z (Z2) -> Position.ini SafetyZ2 (the key MOVE SAFETY uses for Z2), then Sub Z -> INTERLOCKSUBZSAFETY (CoreFunction.ini, 0 = top;
//   larger = lower on all five axes). Only those axes are commanded and one SYST line names them; none below safety = no command, no log
//   (a stick move from a raised state is unchanged). A failed raise returns its code and the stage does not move
long CDlgSubManual_Meas::RetractHeadZBeforeStage(int iStick, LPCTSTR pszWhat)
{
	CMaintControlMgr* pMaint = m_pAuroraMgr->GetMaintControlMgr();
	CPositionConfigMgr* pPosCfg = m_pAuroraMgr->GetPositionConfigMgrPtr();
	if (pMaint == nullptr || pPosCfg == nullptr || pMaint->GetMotorControlMgrPtr() == nullptr) return VTM_RV_SUCCESSED;
	CMotorControlMgr* pMotor = pMaint->GetMotorControlMgrPtr();
	double dSubZSafety = pMotor->GetInterlockSubZSafety();
	double dSafetyZ2 = 0.0;
	pPosCfg->GetSafetyPosition(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, &dSafetyZ2);

	CString strAxes, strOne;
	bool bZ2 = false;
	axis_data_type* pProp = pMotor->GetAxisProperty(AXIS_Z2);
	double dCurZ2 = pMaint->GetCurPos(AXIS_Z2);
	if (pProp != nullptr && pProp->bUse != FALSE && dCurZ2 > dSafetyZ2 + 0.1)
	{
		bZ2 = true;
		strOne.Format(_T(" FRONT Z %.3f -> %.3f"), dCurZ2, dSafetyZ2);
		strAxes += strOne;
	}
	const int nSubAxis[4] = { AXIS_Z3, AXIS_Z4, AXIS_Z5, AXIS_Z6 };
	const int nSubName[4] = { 3, 4, 5, 6 };
	stMotorPosition stSubZ[4];
	CMaintControlMgr::ClearMotorPos(stSubZ, 4);
	int nSubZ = 0;
	for (int k = 0; k < 4; k++)
	{
		pProp = pMotor->GetAxisProperty(nSubAxis[k]);
		double dCur = pMaint->GetCurPos(nSubAxis[k]);
		if (pProp == nullptr || pProp->bUse == FALSE || dCur <= dSubZSafety + 0.1) continue;
		stSubZ[nSubZ].axis = nSubAxis[k];
		stSubZ[nSubZ].dPosition = dSubZSafety;
		nSubZ++;
		strOne.Format(_T(" Z%d %.3f -> %.3f"), nSubName[k], dCur, dSubZSafety);
		strAxes += strOne;
	}
	if (bZ2 == false && nSubZ == 0) return VTM_RV_SUCCESSED;

	CString strLog;
	if (pszWhat == nullptr)
		strLog.Format(_T("[MANUAL] Manual Meas stick move (stick %d): head Z raised before the stage moves -%s"), iStick + 1, (LPCTSTR)strAxes);
	else	// LYS 2026-10-13 review #6
		strLog.Format(_T("[MANUAL] Manual Meas %s: head Z raised before the stage moves -%s"), pszWhat, (LPCTSTR)strAxes);
	CLogWrapper::GetInstance()->KMLOG(SYST, strLog);
	long nRet = VTM_RV_SUCCESSED;
	if (bZ2) nRet = pMaint->MoveCmdPosA(AXIS_Z2, dSafetyZ2, true, SPEED_HIGH);
	if (nRet == VTM_RV_SUCCESSED && nSubZ > 0) nRet = pMaint->MoveCmdPosMulti(stSubZ, nSubZ, SPEED_HIGH, true, false);
	if (nRet != VTM_RV_SUCCESSED)
	{
		if (pszWhat == nullptr)
			strLog.Format(_T("[MANUAL] Manual Meas stick move (stick %d): head Z raise failed (RET %d) - stage not moved"), iStick + 1, (int)nRet);
		else	// LYS 2026-10-13 review #6
			strLog.Format(_T("[MANUAL] Manual Meas %s: head Z raise failed (RET %d) - step not run"), pszWhat, (int)nRet);
		CLogWrapper::GetInstance()->KMLOG(WARN, strLog);
	}
	return nRet;
}
