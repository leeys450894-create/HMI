// DlgSubOperate.cpp : 구현 파일입니다.
//

#include "pch.h"
#include "ALPHA.h"
#include "DlgSubOperate.h"
#include "afxdialogex.h"


#include "DoWork\SystemInitialize.h"
#include "SystemParamShare.h"
//#include "DoWorkGlassSequence.h"
//
#include "AuroraMgr.h"
//#include "Manager\AlarmMonitorMgr.h"
//#include "AlarmController.h"
#include "Manager\SystemConfigMgr.h"
#include "Manager\ShareMemoryMgr.h"	// LYS 2026-09-25 GetEqProbeID
#include "Manager\SequenceManager.h"
//#include "Manager\IOMapControlMgr.h"
//#include "Manager\MotorControlMgr.h"
#include "Manager\MaintControlMgr.h"
#include "Manager\IOMapAddressDefine.h"
#include "Recipe\RecipeEditMgr.h"
#include "Manager\SeqTactTimeMgr.h"
#include "Manager/MCCLogMgr.h"
#include "Manager/LightControlMgr.h"
#include "Alarm/AlarmMonitorMgr.h"
#include "Manager/CommTcpIpFactoryMgr.h"
#include "CommTcpIpObjectCIM.h"


// popup
#include "D_MeasureCycle.h"
#include "D_DisplayMessage.h"

#include "Manager/ResumeInfoMgr.h"
#include "Manager/ResumeJournal.h"		// LYS 2026-10-08 Hybrid Phase 5 IsTupleGlass, cell status reset counter
#include "ResumePrompt.h"				// LYS 2026-10-08 Hybrid Phase 5 journal resume prompt + start prompt guard (D5-8/9/10/16)
#include "D_MoveHomePosition.h"
#include "D_MainOperateInitializing.h"

#include "FunctionModule\FunctionModuleEQState.h"
#include "FunctionModule\FunctionModuleEQStateChange.h"
#include "FunctionModule\FunctionModuleProbeACC.h"	// LYS 2026-09-25 STOP 이 SACC 시퀀스도 세운다
#include "..\CoreFunction\common\IniHelper.h"
#include "AuroraFuncDefine.h"
#include "D_SystemHostMode.h"
#include "DoWork/DoWorkSequenceCIM.h"
#include "..\Include\DefinesCIM.h"
#include "DlgSubMeasure_Progress.h"
#include "DlgSMarkUI.h"
#include "Alarm/AlarmController.h"
#include "CD_SelectColor.h"

// CDlgSubOperate 대화 상자입니다.
#define STR_COMPANY_NAME			_T("K-MAC")
#define _df_MAX_IO_INPUT			48
#define _df_MAX_IO_OUTPUT			32

static CString ColumnName[] =
{
	_T("No."),
	_T("Name"),
	_T("Start"),
	_T("End"),
	_T("Time[sec]"),
};
static long ColumnSize[] =
{
	65,
	145,
	90,
	90,
	90,
};
enum enumColumnName
{
	enumColumnNameNo = 0,
	enumColumnName_Value1,
	enumColumnName_Value2,
	enumColumnName_Value3,
	enumColumnName_Value4,
	enumColumnName_End,
};

static CString ColumnName2[] =
{
	_T("No."),
	_T("RECIPE"),
	_T("Start"),
	_T("End"),
	_T("Time[sec]"),
};
static long ColumnSize2[] =
{
	40,
	200,
	70,
	70,
	70,
};
enum enumColumnName2
{
	enumColumnName2_No = 0,
	enumColumnName2_Value1,
	enumColumnName2_Value2,
	enumColumnName2_Value3,
	enumColumnName2_Value4,
	enumColumnName2_End,
};

static CString ColumnName2_1[] =
{
	_T("No."),
	_T("GLASSID"),
	_T("RECIPE"),
	_T("Start"),
	_T("End"),
	_T("Time"),
};
static long ColumnSize2_1[] =
{
	40,
	120,
	120,
	70,
	70,
	70,
};
enum enumColumnName2_1
{
	enumColumnName2_1_No = 0,
	enumColumnName2_1_Value1,
	enumColumnName2_1_Value2,
	enumColumnName2_1_Value3,
	enumColumnName2_1_Value4,
	enumColumnName2_1_Value5,
	enumColumnName2_1_End,
};


static CString ColumnName3[] =
{
	_T("이름"),
	_T("값"),
};
static long ColumnSize3[] =
{
	100,
	50,
};
enum enumColumnName3
{
	enumColumnName3_No = 0,
	enumColumnName3_Value1,
	enumColumnName3_End,
};


static CString ColumnName5[] =
{
	_T("No."),
	_T("Name"),
	_T("Use"),
};

static long ColumnSizeMeas[] =
{
	130,
	150,
};
enum enumColumnNameMeas
{
	enumColumnNameMeas_Name = 0,
	enumColumnNameMeas_Value1,
	enumColumnNameMeas_End,
};

static CString ColumnMeas[] =
{
	_T("Name"),
	_T("Value"),
};

// LYS 2026-09-19 온도 행 이름·순서를 PLCMAdress.csv 온도 비트 순서(M 주소 순서)로 맞췄다.
//                i 번째 행 = stFDCWData[FDC_WDATA_TEMP_1 + i] = 과열 표시기 M4210+i = 리셋 스위치 (i+1) 번.
//                이름은 IO List Map 위치명. FDC 가 안 보내는 PG-BOX R/L, PROFILE R1/R2/L1/L2 행은 뺐다.
//                순서가 행 인덱스이므로 ESD[V] / LIFT PIN 은 반드시 온도 뒤 마지막 두 칸에 둘 것.
//static CString szNameRow[] =
//{
//	_T("GPS EL.BOX"),
//	_T("UPS EL.BOX"),
//	_T("PC BOX"),
//	_T("PC E-BOX FRONT"),
//	_T("PC E-BOX REAR"),
//	_T("I/O PANEL"),
//	_T("P/N PANEL"),
//	_T("PG-BOX_RIGHT"),
//	_T("PG-BOX_LEFT"),
//	_T("PROFILE RIGHT1"),
//	_T("PROFILE RIGHT2"),
//	_T("PROFILE LEFT1"),
//	_T("PROFILE LEFT2"),
//	_T("FRONT S-BOX"),
//	_T("REAR S-BOX"),
//	_T("SACC"),
//	_T("ESD[V]"),
//	_T("LIFT PIN"),
//};
static CString szNameRow[] =
{
	_T("GPS EL.BOX"),
	_T("UPS EL.BOX"),
	_T("PC BOX"),
	_T("PC E-BOX FRONT"),
	_T("PC E-BOX REAR"),
	_T("I/O PANEL"),
	_T("P/N PANEL"),
	_T("FRONT S-BOX"),
	_T("REAR S-BOX"),
	_T("SACC"),
	_T("ESD[V]"),
	_T("LIFT PIN"),
};

enum _SIZE_t {
	_MAX_SIZE_X = 0,
	_MIN_SIZE_X,
	_MAX_SIZE_Y,
	_MIN_SIZE_Y,
};

// LYS 2026-09-25 NAME 열 추가, 폭은 컨트롤 190 DLU(약 281px)에 맞춤 - 구 130/145/50 = 325px 라 Count 열이 잘렸다
static long ColumnSizeProbe[] =
{
	55,
	100,
	90,
	36,
};
//static long ColumnSizeProbe[] = { 130, 145, 50 };

enum enumColumnNameProbe
{
	enumColumnNameProbe_Name = 0,
	enumColumnNameProbe_Value1,		// LYS 2026-09-25 was "= 0" (NAME column folded into ProbeID)
	enumColumnNameProbe_Value2,
	enumColumnNameProbe_Value3,
	enumColumnNameProbe_End,
};

static CString ColumnProbe[] =
{
	_T("NAME"),		// LYS 2026-09-25
	_T("PROBE ID"),
	_T("MOUNT TIME"),
	_T("CNT"),
};

// LYS 2026-09-25 램프 타이머 4 -> 2. 실제 사용 중인 Align 카메라 2대의 조명은 SPO(port 0) 채널 3(LEFT, CCD1) / 2(RIGHT, CCD2) 이고
//   (DoWorkGlassSWAlignStep / DoWorkGlassMeasureStep 의 iCh = {3, 2}), 채널 0/1 은 OUT_OF_SERVICE 인 프로브 교체용 카메라라 표시하지 않는다.
//   레지스트리 키(LampTime_Used%02d 등)는 종전처럼 SPO 채널 번호로 유지해 누적 시간을 잇고, 화면 라벨만 ALIGN CCD1/2 로 바꾼다.
//   LightControlMgr 가 보내는 WM_MSG_UPDATE_LAMP_TIME 의 iLamp 는 채널 번호이므로 LampChannelToSlot 으로 슬롯을 찾는다.
static const int     s_nLampTimerChannel[df_MAX_USE_LAMP] = { 3, 2 };
static const TCHAR*  s_szLampTimerLabel[df_MAX_USE_LAMP]  = { _T("ALIGN CCD1"), _T("ALIGN CCD2") };

int CDlgSubOperate::LampChannelToSlot(int nChannel)
{
	for (int i = 0; i < df_MAX_USE_LAMP; i++)
	{
		if (s_nLampTimerChannel[i] == nChannel) return i;
	}
	return -1;
}

IMPLEMENT_DYNAMIC(CDlgSubOperate, CMainSubDlgWin)

CDlgSubOperate::CDlgSubOperate(CAuroraMgr *pAuroraMgr, CWnd* pParent /*=NULL*/)
	: CMainSubDlgWin(IDD_DLG_SUB_OPERATE, pParent)
	, m_pIniHelper(new CIniHelper)
	, m_pAuroraMgr(pAuroraMgr)
	, m_pSeqTactTimeMgr(pAuroraMgr->GetSeqTactTimeMgrPtr())
	, m_nMeasCount(0)
	, m_pdGlassSize(nullptr)
	, m_pdPathSize(nullptr)
	, m_pdCardSize(nullptr)
	, m_pnGlassEdgeCut(nullptr)
	, m_bGraph_Reverse_X(FALSE)
	, m_bGraph_Reverse_Y(FALSE)
	, m_bGraph_InvertXY(FALSE)
	, m_bThreadRun(false)
{
	m_pSystemInitializingDlg = new CD_MainOperateInitializing;

	m_pAgingProgressDlg = new CDlgSubMeasure_Progress;

	m_lMeasDoneColorWAD = 0;
	m_lMeasDoneColorVision = 0;
	m_lMeasDoneColorAngle = 0;
	m_lMeasDoneColorColor = 0;
	m_nHybridMapSegCnt = 0;		// LYS 2026-09-30 Hybrid Phase 6
	m_nHybridMapFrameSeg = -2;
	m_nHybridPathSeg = -1;	// LYS 2026-10-12 Hybrid Phase 6
	m_nHybridPathRot = -1;
	m_nCellDrawLegacy = -1;
}

CDlgSubOperate::~CDlgSubOperate()
{
	// 2023.03.06 remove
	/*KillTimer(MAIN_STATE_TIMER_ID);
	KillTimer(MAIN_STATE_TIMER_ID + 1);
	KillTimer(MAIN_STATE_TIMER_ID + 2);
	KillTimer(MAIN_LAMP_OVER_TIMER_ID);*/

	if (m_pParentMeasPathDlg)
	{
		m_pParentMeasPathDlg->DestroyWindow();
		delete m_pParentMeasPathDlg;
		m_pParentMeasPathDlg = nullptr;
	}

	if (m_pParentCellMeasDlg)
	{
		m_pParentCellMeasDlg->DestroyWindow();
		delete m_pParentCellMeasDlg;
		m_pParentCellMeasDlg = nullptr;
	}

	if (m_pSystemInitializingDlg)
	{
		m_pSystemInitializingDlg->DestroyWindow();
		delete m_pSystemInitializingDlg;
		m_pSystemInitializingDlg = nullptr;
	}

	if (m_pAgingProgressDlg)
	{
		m_pAgingProgressDlg->DestroyWindow();
		delete m_pAgingProgressDlg;
		m_pAgingProgressDlg = nullptr;
	}

	if (m_pdGlassSize != nullptr)
	{
		delete[] m_pdGlassSize;
		m_pdGlassSize = nullptr;
	}

	if (m_pdPathSize != nullptr)
	{
		delete[] m_pdPathSize;
		m_pdPathSize = nullptr;
	}

	if (m_pdCardSize != nullptr)
	{
		delete[] m_pdCardSize;
		m_pdCardSize = nullptr;
	}

	if (m_pnGlassEdgeCut != nullptr)
	{
		delete[] m_pnGlassEdgeCut;
		m_pnGlassEdgeCut = nullptr;
	}
}

void CDlgSubOperate::DoDataExchange(CDataExchange* pDX)
{
	CMainSubDlgWin::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_CYCLE, m_btnImage[BUTTON_IMAGE_CYCLE]);
	DDX_Control(pDX, IDC_BUTTON_RESUME, m_btnImage[BUTTON_IMAGE_RESUME]);
	DDX_Control(pDX, IDC_BUTTON_PAUSE, m_btnImage[BUTTON_IMAGE_PAUSE]);
	DDX_Control(pDX, IDC_BUTTON_STOP, m_btnImage[BUTTON_IMAGE_STOP]);
	DDX_Control(pDX, IDC_STATIC_LAMP_TIMER1, m_LmapTimer[0]);
	DDX_Control(pDX, IDC_STATIC_LAMP_TIMER2, m_LmapTimer[1]);
	//DDX_Control(pDX, IDC_STATIC_LAMP_TIMER3, m_LmapTimer[2]);	// LYS 2026-09-25 removed (df_MAX_USE_LAMP 4 -> 2)
	//DDX_Control(pDX, IDC_STATIC_LAMP_TIMER4, m_LmapTimer[3]);

	DDX_Control(pDX, IDC_STATIC_OPERATION_INFO, m_LabelInfo[0]);
	DDX_Control(pDX, IDC_STATIC_LAMPTIME, m_LabelInfo[1]);

	DDX_Control(pDX, IDC_STATIC_TACT_TIME_INFO, m_LabelInfo[2]);
	DDX_Control(pDX, IDC_STATIC_TACT_TIME_INFO2, m_LabelInfo[3]);

	DDX_Control(pDX, IDC_STATIC_MEAS_INFO, m_LabelInfo[4]);
	DDX_Control(pDX, IDC_STATIC_CELL_INFO, m_LabelInfo[5]);
	DDX_Control(pDX, IDC_STATIC_MEAS_INFO2, m_LabelInfo[6]);
	

	DDX_Control(pDX, IDC_GRID_LIST_TACT_TIME_GLASS, m_gridListTactTimeGlass);
	DDX_Control(pDX, IDC_GRID_LIST_TACT_TIME_DETAIL, m_gridListTactTimeMeas);
	DDX_Control(pDX, IDC_GRID_LIST_MEAS_INFO, m_gridListMeasInfo);
	DDX_Control(pDX, IDC_GRID_LIST_CELL_DETAIL, m_gridListCellInfo);
	DDX_Control(pDX, IDC_GRID_LIST_MEAS_INFO3, m_gridListTempInfo);

	DDX_Control(pDX, IDC_CHK_FIXTURE_1, m_ledFixtureR[0]);
	DDX_Control(pDX, IDC_CHK_FIXTURE_2, m_ledFixtureR[1]);
	DDX_Control(pDX, IDC_CHK_FIXTURE_3, m_ledFixtureB[0]);
	DDX_Control(pDX, IDC_CHK_FIXTURE_4, m_ledFixtureB[1]);
	DDX_Control(pDX, IDC_CHK_FIXTURE_5, m_ledFixtureL[0]);
	DDX_Control(pDX, IDC_CHK_FIXTURE_6, m_ledFixtureL[1]);
	DDX_Control(pDX, IDC_CHK_FIXTURE_7, m_ledFixtureF[0]);
	DDX_Control(pDX, IDC_CHK_FIXTURE_8, m_ledFixtureF[1]);

	DDX_Control(pDX, IDC_CHK_FIXTURE_UPDOWN_1, m_ledFixtureRUp[0]);
	DDX_Control(pDX, IDC_CHK_FIXTURE_UPDOWN_2, m_ledFixtureRUp[1]);
	DDX_Control(pDX, IDC_CHK_FIXTURE_UPDOWN_3, m_ledFixtureBUp[0]);
	DDX_Control(pDX, IDC_CHK_FIXTURE_UPDOWN_4, m_ledFixtureBUp[1]);
	DDX_Control(pDX, IDC_CHK_FIXTURE_UPDOWN_5, m_ledFixtureLUp[0]);
	DDX_Control(pDX, IDC_CHK_FIXTURE_UPDOWN_6, m_ledFixtureLUp[1]);
	DDX_Control(pDX, IDC_CHK_FIXTURE_UPDOWN_7, m_ledFixtureFUp[0]);
	DDX_Control(pDX, IDC_CHK_FIXTURE_UPDOWN_8, m_ledFixtureFUp[1]);

	DDX_Control(pDX, IDC_CHK_SHUTTER_UPDOWN, m_ledShtterUp);
	DDX_Control(pDX, IDC_CHK_SHUTTER_UPDOWN2, m_ledShtterMoving);
	DDX_Control(pDX, IDC_CHECK_CIM_REPORT, m_btnCimAlarmNotReport);  // LYS 2026-07-20
}


BEGIN_MESSAGE_MAP(CDlgSubOperate, CMainSubDlgWin)
	ON_BN_CLICKED(IDC_BUTTON_INITIAL, &CDlgSubOperate::OnBnClickedButtonInitial)
	ON_BN_CLICKED(IDC_BUTTON_MOVE_SAFETY, &CDlgSubOperate::OnBnClickedButtonMoveSafety)
	ON_BN_CLICKED(IDC_BUTTON_CHANGE_LOAD, &CDlgSubOperate::OnBnClickedButtonChangeLoad)
	ON_BN_CLICKED(IDC_BUTTON_CHANGE_MEAS, &CDlgSubOperate::OnBnClickedButtonChangeMeas)
	ON_BN_CLICKED(IDC_BUTTON_CYCLE, &CDlgSubOperate::OnBnClickedButtonCycle)
	ON_BN_CLICKED(IDC_BUTTON_RESUME, &CDlgSubOperate::OnBnClickedButtonResume)
	ON_BN_CLICKED(IDC_BUTTON_PAUSE, &CDlgSubOperate::OnBnClickedButtonPause)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CDlgSubOperate::OnBnClickedButtonStop)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgSubOperate::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON_CHANGE_CONTROLMODE, &CDlgSubOperate::OnBnClickedButtonChangeControlmode)
	ON_BN_CLICKED(IDC_BUTTON_SAFEYT_PLC, &CDlgSubOperate::OnBnClickedButtonSafeytPlc)
	ON_BN_CLICKED(IDC_BUTTON_S_MARK_INFO, &CDlgSubOperate::OnBnClickedButtonSMarkInfo)
	ON_BN_CLICKED(IDC_CHECK_CIM_REPORT, &CDlgSubOperate::OnBnClickedCheckCimReport)  // LYS 2026-07-20 CIM Alarm not-report toggle
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CDlgSubOperate 메시지 처리기입니다.


BOOL CDlgSubOperate::OnInitDialog()
{
	CMainSubDlgWin::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	CString strAppName;
	strAppName.Format(_T("%s"), m_pAuroraMgr->GetSystemInfo().name);


	m_pdGlassSize = new double[4];
	m_pnGlassEdgeCut = new int[4];
	m_pdPathSize = new double[4];
	m_pdCardSize = new double[4];

	CString strConfigPath;
	strConfigPath.Format(_T("%s"), CSystemParamShare::GetInstance()->GetConfigFolderPath());
	m_pIniHelper->SetFilePath(strConfigPath + _T("\\Config.ini"));
	LinkIniFile(false);

	// Lamp Timer
	for (int iLamp = 0; iLamp< df_MAX_USE_LAMP; iLamp++)
	{
		m_LmapTimer[iLamp].SetLabel(s_szLampTimerLabel[iLamp]);	// LYS 2026-09-25 라벨은 InitTimer(DefaultTimer) 전에
		m_LmapTimer[iLamp].InitTimer(STR_COMPANY_NAME, strAppName, LT_LAMP_TIMER, s_nLampTimerChannel[iLamp]);	// LYS 2026-09-25 레지스트리 키 = SPO 채널
		//m_LmapTimer[iLamp].InitTimer(STR_COMPANY_NAME, strAppName, LT_LAMP_TIMER, iLamp);

		SetLampUse(iLamp, true);
	}

	// Clamp
	m_ledFixtureR[0].SetImage(IDB_UP_DOWN1, 22);
	m_ledFixtureR[1].SetImage(IDB_UP_DOWN1, 22);
	m_ledFixtureB[0].SetImage(IDB_LEFT_RIGHT1, 22);
	m_ledFixtureB[1].SetImage(IDB_LEFT_RIGHT1, 22);
	m_ledFixtureL[0].SetImage(IDB_UP_DOWN2, 22);
	m_ledFixtureL[1].SetImage(IDB_UP_DOWN2, 22);
	m_ledFixtureF[0].SetImage(IDB_LEFT_RIGHT2, 22);
	m_ledFixtureF[1].SetImage(IDB_LEFT_RIGHT2, 22);

	m_ledFixtureRUp[0].SetImage(IDB_BITMAP_RED_GRAY, 22);
	m_ledFixtureRUp[1].SetImage(IDB_BITMAP_RED_GRAY, 22);
	m_ledFixtureBUp[0].SetImage(IDB_BITMAP_RED_GRAY, 22);
	m_ledFixtureBUp[1].SetImage(IDB_BITMAP_RED_GRAY, 22);
	m_ledFixtureLUp[0].SetImage(IDB_BITMAP_RED_GRAY, 22);
	m_ledFixtureLUp[1].SetImage(IDB_BITMAP_RED_GRAY, 22);
	m_ledFixtureFUp[0].SetImage(IDB_BITMAP_RED_GRAY, 22);
	m_ledFixtureFUp[1].SetImage(IDB_BITMAP_RED_GRAY, 22);

	m_ledShtterUp.SetImage(IDB_UP_DOWN1, 22);

	m_ledShtterMoving.SetImage(IDB_UP_DOWN_MOVING, 22);
	

	//---------------------------------------------------------------------
	// 타이틀에 색상을 입혀준다.
	//---------------------------------------------------------------------
	for (int i = 0; i < 7; i++)
	{
		m_LabelInfo[i].SetBorder(TRUE, TRUE);
		m_LabelInfo[i].SetBkColor(RGB(0, 32, 0));
		m_LabelInfo[i].SetTextColor(RGB(255, 255, 192));
		m_LabelInfo[i].SetFontBold(TRUE);
	}

	//--------------------------------------------------------------------
	// 버튼 이미지 추가
	//--------------------------------------------------------------------
	m_btnImage[BUTTON_IMAGE_CYCLE].SetIcon(IDI_ICON_CYCLE, CXButtonXP::BOTTOM).EnableTheming(TRUE).SetDrawToolbar(FALSE);
	m_btnImage[BUTTON_IMAGE_RESUME].SetIcon(IDI_ICON_RESUME, CXButtonXP::BOTTOM).EnableTheming(TRUE).SetDrawToolbar(FALSE);
	m_btnImage[BUTTON_IMAGE_PAUSE].SetIcon(IDI_ICON_PAUSE, CXButtonXP::BOTTOM).EnableTheming(TRUE).SetDrawToolbar(FALSE);
	m_btnImage[BUTTON_IMAGE_STOP].SetIcon(IDI_ICON_STOP, CXButtonXP::BOTTOM).EnableTheming(TRUE).SetDrawToolbar(FALSE);

	//---------------------------------------------------------------------
	// List 초기화
	//---------------------------------------------------------------------
	__InitListControl();
	__InitialListData();
	__InitListControl2();
	__InitialListData2();
	__InitListControl3();
	__InitialListData3();
	__InitListControl4();
	__InitialListData4();
	__InitListControl5();
	__InitialListData5();

	//---------------------------------------------------------------------
	// SYSTEM INITIAL 을 표시한다.
	//---------------------------------------------------------------------
	m_pSystemInitializingDlg->Create(IDD_POPUP_OPERATE_SYSTEM_INITIALIZE, this);
	m_pSystemInitializingDlg->CenterWindow();
	m_pSystemInitializingDlg->ShowWindow(SW_HIDE);

	//---------------------------------------------------------------------
	// 
	//---------------------------------------------------------------------
	m_pAgingProgressDlg->Create(IDD_DLG_SUB_SACC_PROGRESS, this);
	m_pAgingProgressDlg->CenterWindow();
	m_pAgingProgressDlg->ShowWindow(SW_HIDE);

	//---------------------------------------------------------------------
	// 이니셜이 안되어 있으면 동작하지 못하도록 한다.
	//---------------------------------------------------------------------
	UpdateInitialState(FALSE);
	EnableControlMode(FALSE);
	m_btnImage[BUTTON_IMAGE_CYCLE].EnableWindow(FALSE);
	m_btnImage[BUTTON_IMAGE_RESUME].EnableWindow(FALSE);
	m_btnImage[BUTTON_IMAGE_PAUSE].EnableWindow(FALSE);
	m_btnImage[BUTTON_IMAGE_STOP].EnableWindow(TRUE); // stop 버튼은 사용할 수 있어야 한다.


	// chart
	CWnd* pCtrlRect = this->GetDlgItem(IDC_STATIC_PATH);
	m_pParentMeasPathDlg = new CUIMeasPathDlg();
	m_pParentMeasPathDlg->SetParentWnd(pCtrlRect);
	m_pParentMeasPathDlg->Create(IDD_DLG_UI_MEAS_PATH, this);
	m_pParentMeasPathDlg->DrawChart(pCtrlRect);

	m_pParentMeasPathDlg->SetNProbe(1);

	// LYS 2026-08-26 OCP는 P5만 장착. SetCardSize()가 그리기 전에 P1~P4 카드를 끈다.
	if (m_pAuroraMgr != nullptr && m_pAuroraMgr->GetSystemConfigMgrPtr() != nullptr
		&& m_pAuroraMgr->GetSystemConfigMgrPtr()->IsOCP())
	{
		for (int iCard = 0; iCard < 4; iCard++)	// P1 ~ P4
			m_pParentMeasPathDlg->SetProbeCardUse(iCard, FALSE);
	}
	m_pParentMeasPathDlg->SetAxisShow(TRUE);

	CWnd* pCtrlRect2 = this->GetDlgItem(IDC_STATIC_CELL_MEASURE);
	m_pParentCellMeasDlg = new CUIMeasPathDlg();
	m_pParentCellMeasDlg->SetParentWnd(pCtrlRect2);
	m_pParentCellMeasDlg->Create(IDD_DLG_UI_MEAS_CELL, this);
	m_pParentCellMeasDlg->DrawChart(pCtrlRect2);

	// LYS 2026-10-12 Hybrid Phase 6 cell chart capacity (D6-6 (a)): hybrid maps need more than CELL_MAX 150 (LS fixture 205 cells + frame).
	//   Only the series count grows; the chart's draw limit keeps the configured value for the 1-segment draw, so a 1-segment recipe draws
	//   exactly the cells it drew before. Must run before SetNProbe (InitialSetup allocates and the destructor frees by m_nMaxCell) and the
	//   capacity must never change afterwards (a later SetNProbe frees / reallocates by the same value).
	m_nCellDrawLegacy = m_pParentCellMeasDlg->m_chartOrbit.m_nMaxCell;
	if (m_pParentCellMeasDlg->m_chartOrbit.m_nMaxCell < 512)
		m_pParentCellMeasDlg->m_chartOrbit.m_nMaxCell = 512;
	m_pParentCellMeasDlg->m_chartOrbit.m_nDrawLimit = m_nCellDrawLegacy;
	m_pParentCellMeasDlg->m_chartOrbit.m_nClearBase = m_nCellDrawLegacy;	// every redraw clears max(configured, highest drawn): a 1-segment redraw clears the cells it cleared before
	m_pParentCellMeasDlg->SetNProbe(1, NULL, FALSE);
	m_pParentCellMeasDlg->SetAxisShow(FALSE);

	RotateMeasPath(m_nOldAngle);
	RotateGlassPath(0);	// LYS 2026-08-07 rotation-change: default view 0-deg (load posture, was 90)

	m_pdCardSize[0] = 210;		// LYS 2026-09-03 프로브 사이즈 250 -> 210
	m_pdCardSize[1] = 0;
	m_pdCardSize[2] = m_pdGlassSize[_MAX_SIZE_X];
	m_pdCardSize[3] = m_pdGlassSize[_MIN_SIZE_X];
	m_pParentMeasPathDlg->SetCardSize(m_pdCardSize[0], m_pdCardSize[1], m_pdCardSize[2], m_pdCardSize[3]);

	m_pParentMeasPathDlg->SetLoaderSize(m_pdGlassSize[_MAX_SIZE_X], m_pdGlassSize[_MIN_SIZE_X], m_pdGlassSize[_MAX_SIZE_Y], m_pdGlassSize[_MIN_SIZE_Y], m_pnGlassEdgeCut[0]);	// LYS 2026-08-07 rotation-change: loader at 0-deg load posture (was 90-deg swap)

//	SetTimer(MAIN_STATE_TIMER_ID, 5*1000, NULL);	// 45 sec

	// LYS 2026-07-01 Probe Contact blinking indicator on IDC_STATIC_PATH
	{
		CRect rcPath;
		pCtrlRect->GetClientRect(&rcPath);
		int nW = 150, nH = 30;
		int nX = (rcPath.Width() - nW) / 2;
		int nY = (rcPath.Height() - nH) / 2;
		m_labelProbeContact.Create(_T("Probe Contact"), WS_CHILD | SS_CENTER | SS_CENTERIMAGE, CRect(nX, nY, nX + nW, nY + nH), m_pParentMeasPathDlg, 0xFFFF);
		m_labelProbeContact.SetBkColor(RGB(255, 0, 0));
		m_labelProbeContact.SetTextColor(RGB(255, 255, 0));
		m_labelProbeContact.SetFontBold(TRUE);
		m_labelProbeContact.SetFontSize(14);
		m_labelProbeContact.SetBorder(TRUE, TRUE);
		m_labelProbeContact.ShowWindow(SW_HIDE);
		m_bProbeContactFlashing = false;
	}

	// LYS 2026-10-12 Hybrid Phase 6 segment / pass / resume indicator (D6-1 (a)): runtime CLabel in the cell chart's top band, like the
	//   Probe Contact label (no .rc / resource.h). Chart top +1..+19 = above the "Glass Size" text the chart draws at its y 20
	//   (CChartOrbit::OnOC2Repaint) and the plot area (top margin 30). Hidden until UpdateHybridIndicator has text - a 1-segment recipe never shows it.
	{
		CRect rcCell, rcChart;
		m_pParentCellMeasDlg->GetClientRect(&rcCell);
		m_pParentCellMeasDlg->m_chartOrbit.GetWindowRect(&rcChart);
		m_pParentCellMeasDlg->ScreenToClient(&rcChart);		// the host has WS_BORDER: the chart need not start at client (0, 0)
		int nTop = (rcChart.top > 0) ? rcChart.top : 0;
		int nLeft = ((rcChart.left > 0) ? rcChart.left : 0) + 2;
		int nRight = ((rcChart.right < rcCell.right) ? rcChart.right : rcCell.right) - 2;
		m_labelHybridInfo.Create(_T(""), WS_CHILD | SS_CENTER | SS_CENTERIMAGE, CRect(nLeft, nTop + 1, nRight, nTop + 19), m_pParentCellMeasDlg, 0xFFFE);
		m_labelHybridInfo.SetInvalidate(FALSE);	// no parent erase on SetText (the host has no WS_CLIPCHILDREN): no flicker over the chart
		CFont* pDlgFont = GetFont();				// operate dialog font (Gulim 9, HANGUL_CHARSET): a runtime CLabel starts with the raster SYSTEM_FONT
		if (pDlgFont != NULL)
		{
			LOGFONT lf;
			pDlgFont->GetLogFont(&lf);
			m_labelHybridInfo.SetFont(lf);
		}
		m_labelHybridInfo.SetBkColor(RGB(0, 32, 0));
		m_labelHybridInfo.SetTextColor(RGB(255, 255, 192));
		m_labelHybridInfo.SetFontBold(TRUE);		// size = the dialog's 9 pt (no SetFontSize: it would rebuild by point size)
		m_labelHybridInfo.ShowWindow(SW_HIDE);
		m_labelHybridInfo.SetWindowPos(&CWnd::wndTop, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);	// above the chart (created first)
		m_pParentCellMeasDlg->m_chartOrbit.ModifyStyle(0, WS_CLIPSIBLINGS, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);	// chart painting leaves a visible label alone (a hidden one clips nothing)
	}

	// LYS 2026-07-20 Init CIM Alarm not-report checkbox from config
	{
		m_btnCimAlarmNotReport.m_nFlatStyle = CMFCButton::BUTTONSTYLE_3D;
		m_btnCimAlarmNotReport.ModifyStyle(0, BS_CENTER);
		bool bNotReport = m_pAuroraMgr->GetSystemConfigMgrPtr()->IsUseCIMAlarmNotReport();
		m_btnCimAlarmNotReport.SetCheck(bNotReport ? BST_CHECKED : BST_UNCHECKED);
		UpdateCimReportButtonColor(bNotReport);
	}

	SetTimer(MAIN_STATE_TIMER_ID, 1000, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


void CDlgSubOperate::SetLampUse(int iLamp, bool bUse)
{
	if (bUse == true)	// USE
	{
		int iUseCnt[5];
		ZeroMemory(&iUseCnt, sizeof(iUseCnt));

	/*	for (int i = 0; i < 5; i++)
		{
			if (m_pSystem->stSystem[VTM_LED + i].bUse)
			{
				iUseCnt[i] = m_pSystem->stSystem[VTM_LED + i].stModuleParam.iUseCnt;
			}
		}

		CString str;
		int iLED = 0;
		if (iLamp < iUseCnt[0])
		{
			iLED = iLamp;
			str.Format(_T("%s"), m_pSystem->stSystem[VTM_LED].stModuleParam.szName[iLED]);
		}
		else if (iLamp < iUseCnt[0] + iUseCnt[1])
		{
			iLED = iLamp - iUseCnt[0];
			str.Format(_T("%s"), m_pSystem->stSystem[VTM_LED2].stModuleParam.szName[iLED]);
		}
		else if (iLamp < iUseCnt[0] + iUseCnt[1] + iUseCnt[2])
		{
			iLED = iLamp - (iUseCnt[0] + iUseCnt[1]);
			str.Format(_T("%s"), m_pSystem->stSystem[VTM_LED3].stModuleParam.szName[iLED]);
		}
		else if (iLamp < iUseCnt[0] + iUseCnt[1] + iUseCnt[2] + iUseCnt[3])
		{
			iLED = iLamp - (iUseCnt[0] + iUseCnt[1] + iUseCnt[2]);
			str.Format(_T("%s"), m_pSystem->stSystem[VTM_LED_TR].stModuleParam.szName[iLED]);
		}
		else if (iLamp < iUseCnt[0] + iUseCnt[1] + iUseCnt[2] + iUseCnt[3] + iUseCnt[4])
		{
			iLED = iLamp - (iUseCnt[0] + iUseCnt[1] + iUseCnt[2] + iUseCnt[3]);
			str.Format(_T("%s"), m_pSystem->stSystem[VTM_UV_LED].stModuleParam.szName[iLED]);
		}*/

	//	CRect r2, rect;
	//	int nMoveH = 0;
	//	GetDlgItem(IDC_STC_MAME_LAMP_TIMER1)->GetWindowRect(r2);
	//	ScreenToClient(&r2);
	//	nMoveH = r2.Height() * 2 + 5;

	//	rect = r2;
	//	rect.top += (nMoveH*iLamp);
	//	rect.bottom += (nMoveH*iLamp);
	//	GetDlgItem(IDC_STC_MAME_LAMP_TIMER1 + iLamp)->MoveWindow(rect);

	////	GetDlgItem(IDC_STATIC_LAMP_TIMER1 + iLamp)->SetWindowText(str);

	//	GetDlgItem(IDC_STC_MAME_LAMP_TIMER1 + iLamp)->ShowWindow(SW_SHOW);

	//	GetDlgItem(IDC_STC_MAME_LAMP_TIMER1)->GetWindowRect(r2);
	//	ScreenToClient(&r2);

	//	rect = r2;
	//	rect.top += (nMoveH*iLamp);
	//	rect.bottom += (nMoveH*iLamp);
	//	GetDlgItem(IDC_STATIC_LAMP_TIMER1 + iLamp)->MoveWindow(rect);

		m_LmapTimer[iLamp].ShowWindow(SW_SHOW);
	}
	else
	{
		m_LmapTimer[iLamp].ShowWindow(SW_HIDE);
	}
}


void CDlgSubOperate::__InitListControl()
{
	m_gridListTactTimeMeas.GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridListTactTimeMeas.GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridListTactTimeMeas.GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridListTactTimeMeas.SetDefCellMargin(0);
	m_gridListTactTimeMeas.SetFixedColumnSelection(FALSE);
	m_gridListTactTimeMeas.SetFixedRowSelection(FALSE);
	m_gridListTactTimeMeas.SetSingleColSelection(FALSE);
	m_gridListTactTimeMeas.SetSingleRowSelection(FALSE);
	m_gridListTactTimeMeas.SetTrackFocusCell(FALSE);
	m_gridListTactTimeMeas.SetRowResize(FALSE);
	m_gridListTactTimeMeas.SetColumnResize(FALSE);
}


void CDlgSubOperate::__InitialListData()
{
	// 결과값 리스트를 갱신한다.
	m_gridListTactTimeMeas.DeleteAllItems();
	m_gridListTactTimeMeas.SetColumnCount(enumColumnName_End);
	m_gridListTactTimeMeas.SetFixedRowCount(1);

	for (long n = 0; n < enumColumnName_End; ++n)
	{
		m_gridListTactTimeMeas.SetColumnWidth(n, ColumnSize[n]);
		m_gridListTactTimeMeas.SetItemText(0, n, ColumnName[n]);
		m_gridListTactTimeMeas.SetItemState(0, n, GVIS_READONLY);
		m_gridListTactTimeMeas.SetItemBkColour(0, n, RGB(172, 172, 172));
		m_gridListTactTimeMeas.SetItemFgColour(0, n, RGB(255, 255, 255));
	}

	// Items + Header
	m_gridListTactTimeMeas.SetEditable(TRUE);

	int nCount = 500;
	// jsyoon 2022.07.04 change
	m_gridListTactTimeMeas.SetRowCount(1);

//	nCount = m_pSeqTactTimeMgr->GetDataCount();
//	m_gridListTactTimeMeas.SetRowCount(nCount + 1);
//
//	m_gridListTactTimeMeas.SetRowHeight(0, 22);
//	for (long i = 0; i < nCount; i++)
//	{
////		m_gridListTactTimeMeas.SetRowHeight(i + 1, 0);
//		m_gridListTactTimeMeas.SetRowHeight(i + 1, 22);
//	}

	__RefreshListData();
}


void CDlgSubOperate::__RefreshListData()
{
	// jsyoon 2022.07.04 change
	int nCount = 0, nMaxRow = 0;
	nCount = m_pSeqTactTimeMgr->GetDataCount();
	nMaxRow = m_gridListTactTimeMeas.GetRowCount();
	if (nCount + 1 > nMaxRow)
	{
		m_gridListTactTimeMeas.SetRowCount(nCount + 1);
		m_gridListTactTimeMeas.SetRowHeight(0, 22);
		for (long i = 0; i < nCount; i++)
		{
			m_gridListTactTimeMeas.SetRowHeight(i + 1, 22);
		}
	}

	m_gridListTactTimeMeas.SetRedraw(FALSE);

	/*int nCount = 0;
	nCount = m_pSeqTactTimeMgr->GetDataCount();*/
	for (long i = 0; i < nCount; i++)
	{
		STactTimeInfo &data = m_pSeqTactTimeMgr->GetData(i);

//		m_gridListTactTimeMeas.SetRowHeight(i + 1, 22);

		// cycle
		CString str;
		str.Format(_T("%d"), i + 1);
		m_gridListTactTimeMeas.GetCell(i + 1, enumColumnNameNo)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridListTactTimeMeas.SetItemState(i + 1, enumColumnNameNo, GVIS_READONLY);
		m_gridListTactTimeMeas.SetItemText(i + 1, enumColumnNameNo, str);

		// Item Name
		m_gridListTactTimeMeas.SetItemText(i + 1, enumColumnName_Value1, data.strName);
		m_gridListTactTimeMeas.SetItemState(i + 1, enumColumnName_Value1, GVIS_READONLY);

		// start time
		m_gridListTactTimeMeas.SetItemText(i + 1, enumColumnName_Value2, data.fStart);
		m_gridListTactTimeMeas.SetItemState(i + 1, enumColumnName_Value2, GVIS_READONLY);

		// end time
		m_gridListTactTimeMeas.SetItemText(i + 1, enumColumnName_Value3, data.fEnd);
		m_gridListTactTimeMeas.SetItemState(i + 1, enumColumnName_Value3, GVIS_READONLY);

		// time elapsed
		str.Format(_T("%.3f"), data.fElapsed / 1000.f);
		m_gridListTactTimeMeas.SetItemText(i + 1, enumColumnName_Value4, str);
		m_gridListTactTimeMeas.SetItemState(i + 1, enumColumnName_Value4, GVIS_READONLY);

		for (long n = 0; n < enumColumnName_End; ++n)
		{
			m_gridListTactTimeMeas.SetItemFgColour(i + 1, n, RGB(0, 0, 0));
		}
	}

//	m_gridListTactTimeMeas.SetRedraw(TRUE, TRUE);
	m_gridListTactTimeMeas.SetRedraw(TRUE);
}


void CDlgSubOperate::__InitListControl2()
{
	m_gridListTactTimeGlass.GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridListTactTimeGlass.GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridListTactTimeGlass.GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridListTactTimeGlass.SetDefCellMargin(0);
	m_gridListTactTimeGlass.SetFixedColumnSelection(FALSE);
	m_gridListTactTimeGlass.SetFixedRowSelection(FALSE);
	m_gridListTactTimeGlass.SetSingleColSelection(FALSE);
	m_gridListTactTimeGlass.SetSingleRowSelection(FALSE);
	m_gridListTactTimeGlass.SetTrackFocusCell(FALSE);
	m_gridListTactTimeGlass.SetRowResize(FALSE);
	m_gridListTactTimeGlass.SetColumnResize(FALSE);
}


void CDlgSubOperate::__InitialListData2()
{
	// 결과값 리스트를 갱신한다.
	m_gridListTactTimeGlass.DeleteAllItems();
	m_gridListTactTimeGlass.SetColumnCount(enumColumnName2_1_End);
	m_gridListTactTimeGlass.SetFixedRowCount(1);

	for (long n = 0; n < enumColumnName2_1_End; ++n)
	{
		m_gridListTactTimeGlass.SetColumnWidth(n, ColumnSize2_1[n]);
		m_gridListTactTimeGlass.SetItemText(0, n, ColumnName2_1[n]);
		m_gridListTactTimeGlass.SetItemState(0, n, GVIS_READONLY);
		m_gridListTactTimeGlass.SetItemBkColour(0, n, RGB(172, 172, 172));
		m_gridListTactTimeGlass.SetItemFgColour(0, n, RGB(255, 255, 255));
	}

	// Items + Header
	m_gridListTactTimeGlass.SetEditable(TRUE);
	m_gridListTactTimeGlass.SetRowCount(m_pSeqTactTimeMgr->GetDataCount2() + 1);

	m_gridListTactTimeGlass.SetRowHeight(0, 22);
	for (long i = 0; i < m_pSeqTactTimeMgr->GetDataCount2(); i++)
	{
		m_gridListTactTimeGlass.SetRowHeight(i + 1, 22);
	}

	__RefreshListData2();
}


void CDlgSubOperate::__RefreshListData2()
{
	m_gridListTactTimeGlass.SetRedraw(FALSE);

	for (long i = 0; i < m_pSeqTactTimeMgr->GetDataCount2(); i++)
	{
		STactTimeMeasInfo &data = m_pSeqTactTimeMgr->GetData2(i);

		// No
		CString str;
		str.Format(_T("%d"), data.sitem_index + 1);
		m_gridListTactTimeGlass.GetCell(i + 1, enumColumnName2_1_No)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridListTactTimeGlass.SetItemState(i + 1, enumColumnName2_1_No, GVIS_READONLY);
		m_gridListTactTimeGlass.SetItemText(i + 1, enumColumnName2_1_No, str);

		// GlassID.
		str.Format(_T("%s"), data.strGlassID);
		m_gridListTactTimeGlass.SetItemText(i + 1, enumColumnName2_1_Value1, str);
		m_gridListTactTimeGlass.SetItemState(i + 1, enumColumnName2_1_Value1, GVIS_READONLY);

		// RecipeID.
		str.Format(_T("%s"), data.strRecipeID);
		//str.Format(_T("%s"), m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeID());
		m_gridListTactTimeGlass.SetItemText(i + 1, enumColumnName2_1_Value2, str);
		m_gridListTactTimeGlass.SetItemState(i + 1, enumColumnName2_1_Value2, GVIS_READONLY);

		// start time
		str.Format(_T("%s"), data.fStart);
		m_gridListTactTimeGlass.SetItemText(i + 1, enumColumnName2_1_Value3, str);
		m_gridListTactTimeGlass.SetItemState(i + 1, enumColumnName2_1_Value3, GVIS_READONLY);

		// end time
		str.Format(_T("%s"), data.fEnd);
		m_gridListTactTimeGlass.SetItemText(i + 1, enumColumnName2_1_Value4, str);
		m_gridListTactTimeGlass.SetItemState(i + 1, enumColumnName2_1_Value4, GVIS_READONLY);

		// time elapsed
		str.Format(_T("%.3f"), data.fElapsed / 1000.f);
		m_gridListTactTimeGlass.SetItemText(i + 1, enumColumnName2_1_Value5, str);
		m_gridListTactTimeGlass.SetItemState(i + 1, enumColumnName2_1_Value5, GVIS_READONLY);

		for (long n = 0; n < enumColumnName2_1_End; ++n)
		{
			m_gridListTactTimeGlass.SetItemFgColour(i + 1, n, RGB(0, 0, 0));
		}
	}

	m_gridListTactTimeGlass.SetRedraw(TRUE);
}

void CDlgSubOperate::__InitListControl3()
{
	m_gridListMeasInfo.GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridListMeasInfo.GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridListMeasInfo.GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridListMeasInfo.SetDefCellMargin(0);
	m_gridListMeasInfo.SetFixedColumnSelection(FALSE);
	m_gridListMeasInfo.SetFixedRowSelection(FALSE);
	m_gridListMeasInfo.SetSingleColSelection(FALSE);
	m_gridListMeasInfo.SetSingleRowSelection(FALSE);
	m_gridListMeasInfo.SetTrackFocusCell(FALSE);
	m_gridListMeasInfo.SetRowResize(FALSE);
	m_gridListMeasInfo.SetColumnResize(FALSE);
}


#if 0	// LYS 2026-09-25 구 PROBE INFO 그리드(P1~P5 5행 고정, NAME 열 없음) - 아래 새 구현으로 대체
void CDlgSubOperate::__InitialListData3()
{
	// 결과값 리스트를 갱신한다.
	m_gridListMeasInfo.DeleteAllItems();
	m_gridListMeasInfo.SetColumnCount(enumColumnNameProbe_End);
	m_gridListMeasInfo.SetFixedRowCount(1);

	for (long n = 0; n < enumColumnNameProbe_End; ++n)
	{
		m_gridListMeasInfo.SetColumnWidth(n, ColumnSizeProbe[n]);
		m_gridListMeasInfo.SetItemText(0, n, ColumnProbe[n]);
		m_gridListMeasInfo.SetItemState(0, n, GVIS_READONLY);
		m_gridListMeasInfo.SetItemBkColour(0, n, RGB(172, 172, 172));
		m_gridListMeasInfo.SetItemFgColour(0, n, RGB(255, 255, 255));
	}

	// Items + Header
	m_gridListMeasInfo.SetEditable(TRUE);
	m_gridListMeasInfo.SetRowCount(5 + 1);

	m_gridListMeasInfo.SetRowHeight(0, 18);
	for (long i = 0; i < 5; i++)
	{
		m_gridListMeasInfo.SetRowHeight(i + 1, 15);
	}

	__RefreshListData3();
}


void CDlgSubOperate::__RefreshListData3()
{
	m_gridListMeasInfo.SetRedraw(FALSE);

	int i = 0;
	CString strData;
	bool bCheck = false;
	double dValue = 0, dValueMax = 0, dValueMin = 0;
	for (int nRow = 0; nRow < 5; nRow++)
	{
		// LYS 2026-06-26 OCP: only P5 (index 4) has valid probe data
		bool bSkipProbe = (i < 4 && m_pAuroraMgr->GetSystemConfigMgrPtr() && m_pAuroraMgr->GetSystemConfigMgrPtr()->IsOCP());

		// Name 
		if (bSkipProbe)
			strData = _T("");
		else
			strData.Format(_T("%s"), m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->m_szPROBEID[i]); // CharToString(m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->stProbeInfo[i].m_szProbeCardID));
		m_gridListMeasInfo.GetCell(i + 1, enumColumnNameProbe_Value1)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridListMeasInfo.SetItemState(i + 1, enumColumnNameProbe_Value1, GVIS_READONLY);
		m_gridListMeasInfo.SetItemText(i + 1, enumColumnNameProbe_Value1, strData);

		// Mount 
		strData.Format(_T("%s"), m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->stProbeInfo[i].m_szProbeCardIDTime);
		m_gridListMeasInfo.GetCell(i + 1, enumColumnNameProbe_Value2)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridListMeasInfo.SetItemState(i + 1, enumColumnNameProbe_Value2, GVIS_READONLY);
		m_gridListMeasInfo.SetItemText(i + 1, enumColumnNameProbe_Value2, strData);

		// Count
		strData.Format(_T("%d"), m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->stProbeInfo[i].m_nUseCnt);
		m_gridListMeasInfo.GetCell(i + 1, enumColumnNameProbe_Value3)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridListMeasInfo.SetItemState(i + 1, enumColumnNameProbe_Value3, GVIS_READONLY);
		m_gridListMeasInfo.SetItemText(i + 1, enumColumnNameProbe_Value3, strData);

		for (long n = 0; n < enumColumnNameProbe_End; ++n)
		{
			m_gridListMeasInfo.SetItemFgColour(i + 1, n, RGB(0, 0, 0));
		}

		i++;
	}

	m_gridListMeasInfo.SetRedraw(TRUE);
}


#else
// LYS 2026-09-25 PROBE INFO 그리드를 현 컨셉에 맞춤.
//   OCP(1-Probe): P5 MEAS(설비 내 프로브, D2150) / BUFFER 1,2 / LOAD(RFID, D2400) 4행. 장착 시각·컨택 횟수는 P5 행에만.
//   HMI: P1~P4 AGING + P5 MEAS 5행. ID·시각·횟수는 CShareMemoryMgr 접근자(GetEqProbe*)로 읽는다.
int CDlgSubOperate::__MeasInfoRowCount()
{
	const bool bOcp = (m_pAuroraMgr->GetSystemConfigMgrPtr() && m_pAuroraMgr->GetSystemConfigMgrPtr()->IsOCP());
	return bOcp ? 4 : CShareMemoryMgr::EQ_PROBE_UNIT_MAX;
}

void CDlgSubOperate::__InitialListData3()
{
	m_gridListMeasInfo.DeleteAllItems();
	m_gridListMeasInfo.SetColumnCount(enumColumnNameProbe_End);
	m_gridListMeasInfo.SetFixedRowCount(1);

	for (long n = 0; n < enumColumnNameProbe_End; ++n)
	{
		m_gridListMeasInfo.SetColumnWidth(n, ColumnSizeProbe[n]);
		m_gridListMeasInfo.SetItemText(0, n, ColumnProbe[n]);
		m_gridListMeasInfo.SetItemState(0, n, GVIS_READONLY);
		m_gridListMeasInfo.SetItemBkColour(0, n, RGB(172, 172, 172));
		m_gridListMeasInfo.SetItemFgColour(0, n, RGB(255, 255, 255));
	}

	const int nRows = __MeasInfoRowCount();
	m_gridListMeasInfo.SetEditable(TRUE);
	m_gridListMeasInfo.SetRowCount(nRows + 1);
	m_gridListMeasInfo.SetRowHeight(0, 18);
	for (long i = 0; i < nRows; i++)
	{
		m_gridListMeasInfo.SetRowHeight(i + 1, 15);
	}
	m_gridListMeasInfo.ExpandColumnsToFit(TRUE);

	__RefreshListData3();
}

void CDlgSubOperate::__RefreshListData3()
{
	CShareMemoryMgr* pShm = m_pAuroraMgr->GetShareMemoryMgrPtr();
	if (pShm == nullptr) return;
	VTM_SYSTEM* pShare = pShm->GetSharedMemory();
	const bool bOcp = (m_pAuroraMgr->GetSystemConfigMgrPtr() && m_pAuroraMgr->GetSystemConfigMgrPtr()->IsOCP());
	const int nRows = __MeasInfoRowCount();
	if (m_gridListMeasInfo.GetRowCount() < nRows + 1) return;	// __InitialListData3 전

	m_gridListMeasInfo.SetRedraw(FALSE);

	for (int nRow = 0; nRow < nRows; nRow++)
	{
		CString strName, strID, strMount, strCnt;
		if (bOcp)
		{
			switch (nRow)
			{
			case 0:
				strName = _T("P5 MEAS");
				strID = pShm->GetEqProbeID(CShareMemoryMgr::EQ_PROBE_UNIT_MEAS);
				strMount = pShm->GetEqProbeMountTime(CShareMemoryMgr::EQ_PROBE_UNIT_MEAS);
				strCnt.Format(_T("%u"), pShm->GetEqProbeUseCnt(CShareMemoryMgr::EQ_PROBE_UNIT_MEAS));
				break;
			case 1:	strName = _T("BUFFER 1");	if (pShare != nullptr) strID = pShare->m_szBufferPROBEID[0];	break;
			case 2:	strName = _T("BUFFER 2");	if (pShare != nullptr) strID = pShare->m_szBufferPROBEID[1];	break;
			case 3:	strName = _T("LOAD");		if (pShare != nullptr) strID = pShare->m_szLoadPROBEID;		break;
			default: break;
			}
		}
		else
		{
			const int iUnit = nRow + 1;	// P1..P5
			if (iUnit == CShareMemoryMgr::EQ_PROBE_UNIT_MEAS)
				strName.Format(_T("P%d MEAS"), iUnit);
			else
				strName.Format(_T("P%d AGING"), iUnit);
			strID = pShm->GetEqProbeID(iUnit);
			strMount = pShm->GetEqProbeMountTime(iUnit);
			strCnt.Format(_T("%u"), pShm->GetEqProbeUseCnt(iUnit));
		}
		strID.Trim();

		const long nGridRow = nRow + 1;
		LPCTSTR pszCell[enumColumnNameProbe_End] = { strName, strID, strMount, strCnt };
		for (long n = 0; n < enumColumnNameProbe_End; ++n)
		{
			m_gridListMeasInfo.GetCell(nGridRow, n)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			m_gridListMeasInfo.SetItemState(nGridRow, n, GVIS_READONLY);
			m_gridListMeasInfo.SetItemText(nGridRow, n, pszCell[n]);
			m_gridListMeasInfo.SetItemFgColour(nGridRow, n, RGB(0, 0, 0));
		}
	}

	m_gridListMeasInfo.SetRedraw(TRUE);
}
#endif


void CDlgSubOperate::__InitListControl4()
{
	m_gridListCellInfo.GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridListCellInfo.GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridListCellInfo.GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridListCellInfo.SetDefCellMargin(0);
	m_gridListCellInfo.SetFixedColumnSelection(FALSE);
	m_gridListCellInfo.SetFixedRowSelection(FALSE);
	m_gridListCellInfo.SetSingleColSelection(FALSE);
	m_gridListCellInfo.SetSingleRowSelection(FALSE);
	m_gridListCellInfo.SetTrackFocusCell(FALSE);
	m_gridListCellInfo.SetRowResize(FALSE);
	m_gridListCellInfo.SetColumnResize(FALSE);
}


void CDlgSubOperate::__InitialListData4()
{
	// 결과값 리스트를 갱신한다.
	m_gridListCellInfo.DeleteAllItems();
	m_gridListCellInfo.SetColumnCount(enumColumnName3_End);
	m_gridListCellInfo.SetFixedRowCount(1);

	for (long n = 0; n < enumColumnName3_End; ++n)
	{
		m_gridListCellInfo.SetColumnWidth(n, ColumnSize3[n]);
		m_gridListCellInfo.SetItemText(0, n, ColumnName3[n]);
		m_gridListCellInfo.SetItemState(0, n, GVIS_READONLY);
		m_gridListCellInfo.SetItemBkColour(0, n, RGB(172, 172, 172));
		m_gridListCellInfo.SetItemFgColour(0, n, RGB(255, 255, 255));
	}

	// Items + Header
	m_gridListCellInfo.SetEditable(TRUE);
	m_gridListCellInfo.SetRowCount(1+ _df_MAX_IO_OUTPUT + 7);
	m_gridListCellInfo.SetRowHeight(0, 18);
	for (long i = 0; i < _df_MAX_IO_OUTPUT + 7 ; i++)
	{
		m_gridListCellInfo.SetRowHeight(i + 1, 13);
	}

	__RefreshListData4();
}


void CDlgSubOperate::__RefreshListData4()
{
	m_gridListCellInfo.SetRedraw(FALSE);
	
	//int nInputUnit[6] = { 0, 0, 0, 0, 0, 2 };
	//int nInputPort[6] = { 3, 4, 5, 6, 7, 1 };

	//long nCnt = 0, nCntMC4U = 0;
	//int nUnit = 0, nPort = 0;
	//for (long i = 0; i < _df_MAX_IO_OUTPUT + 7; i++)
	//{
	//	CString strNo;
	////	strNo.Format(_T("[%d]"), i + 1);
	//	//strNo.Format(_T("[%d]IDO%02d"), i + 1, nCnt);

	//	//m_gridListIOOutp.GetCell(i + 1, enumColumnName3_No)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	//	//m_gridListIOOutp.SetItemState(i + 1, enumColumnName3_No, GVIS_READONLY);
	//	//m_gridListIOOutp.SetItemText(i + 1, enumColumnName3_No, strNo);

	//	// description
	//	if (i < _df_MAX_IO_OUTPUT)
	//	{
	//		strNo.Format(_T("[%d]IDO%02d"), i + 1, nCnt);

	//		m_gridListIOOutp.GetCell(i + 1, enumColumnName3_No)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	//		m_gridListIOOutp.SetItemState(i + 1, enumColumnName3_No, GVIS_READONLY);
	//		m_gridListIOOutp.SetItemText(i + 1, enumColumnName3_No, strNo);

	//		CString strBitImportName;
	//		strBitImportName.Format(_T("%s"), ColumnIOOutput[i]);
	//		m_gridListIOOutp.SetItemText(i + 1, enumColumnName3_Value1, strBitImportName);
	//		m_gridListIOOutp.SetItemState(i + 1, enumColumnName3_Value1, GVIS_READONLY);

	//		// 활성화 되어 있는 상태를 표시해준다.
	//		if (m_pAuroraMgr->GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitExport(i) == true)
	//		{
	//			for (long n = enumColumnName3_Value1; n < enumColumnName3_End; ++n)
	//			{
	//				if (i == 0 || i == 5 || i == 11 || i == 12 || i == 16 || i == 17 )	// Interlock
	//				{
	//					m_gridListIOOutp.SetItemBkColour(i + 1, n, CLR_RED);
	//				}
	//				else
	//				{
	//					m_gridListIOOutp.SetItemBkColour(i + 1, n, CLR_LIGHT_YELLOW);
	//				}
	//					
	//			}
	//		}
	//		else
	//		{
	//			for (long n = enumColumnName3_Value1; n < enumColumnName3_End; ++n)
	//			{
	//				m_gridListIOOutp.SetItemBkColour(i + 1, n, CLR_GRAY);
	//			}
	//		}

	//		nCnt++;
	//	}
	//	else if(i == _df_MAX_IO_OUTPUT)
	//	{
	//		CString strBitImportName;
	//		strBitImportName.Format(_T("%s"), ColumnIOOutput[i]);
	//		m_gridListIOOutp.SetItemText(i + 1, enumColumnName3_Value1, strBitImportName);
	//		m_gridListIOOutp.SetItemState(i + 1, enumColumnName3_Value1, GVIS_READONLY);
	//		m_gridListIOOutp.SetRowHeight(i + 1, 18);

	//		for (long n = enumColumnName3_Value1; n < enumColumnName3_End; ++n)
	//		{
	//			m_gridListIOOutp.SetItemBkColour(i + 1, n, RGB(172, 172, 172));
	//			m_gridListIOOutp.SetItemFgColour(i + 1, n, RGB(255, 255, 255));
	//		}
	//	}
	//	else // MC4U
	//	{
	//		nUnit = nInputUnit[nCntMC4U];
	//		nPort = nInputPort[nCntMC4U];
	//		strNo.Format(_T("[%d]IN%d.%d"), nCntMC4U + 1, nUnit, nPort); 

	//		m_gridListIOOutp.GetCell(i + 1, enumColumnName3_No)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	//		m_gridListIOOutp.SetItemState(i + 1, enumColumnName3_No, GVIS_READONLY);
	//		m_gridListIOOutp.SetItemText(i + 1, enumColumnName3_No, strNo);

	//		CString strBitImportName;
	//		strBitImportName.Format(_T("%s"), ColumnIOOutput[i]);
	//		m_gridListIOOutp.SetItemText(i + 1, enumColumnName3_Value1, strBitImportName);
	//		m_gridListIOOutp.SetItemState(i + 1, enumColumnName3_Value1, GVIS_READONLY);

	//		// 활성화 되어 있는 상태를 표시해준다.
	//		if (m_pAuroraMgr->GetMotorControlMgrPtr()->GetInput(nUnit, nPort) == 1)
	//		{
	//			for (long n = enumColumnName3_Value1; n < enumColumnName3_End; ++n)
	//			{
	//				m_gridListIOOutp.SetItemBkColour(i + 1, n, CLR_LIGHT_YELLOW);
	//			}
	//		}
	//		else
	//		{
	//			for (long n = enumColumnName3_Value1; n < enumColumnName3_End; ++n)
	//			{
	//				m_gridListIOOutp.SetItemBkColour(i + 1, n, CLR_GRAY);
	//			}
	//		}

	//		nCntMC4U++;
	//	}
	//}

	m_gridListCellInfo.ExpandColumnsToFit(TRUE);
	m_gridListCellInfo.SetRedraw(TRUE);
}


void CDlgSubOperate::__InitListControl5()
{
	m_gridListTempInfo.GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridListTempInfo.GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridListTempInfo.GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
	m_gridListTempInfo.SetDefCellMargin(0);
	m_gridListTempInfo.SetFixedColumnSelection(FALSE);
	m_gridListTempInfo.SetFixedRowSelection(FALSE);
	m_gridListTempInfo.SetSingleColSelection(FALSE);
	m_gridListTempInfo.SetSingleRowSelection(FALSE);
	m_gridListTempInfo.SetTrackFocusCell(FALSE);
	m_gridListTempInfo.SetRowResize(FALSE);
	m_gridListTempInfo.SetColumnResize(FALSE);
}


void CDlgSubOperate::__InitialListData5()
{
	// 결과값 리스트를 갱신한다.
	m_gridListTempInfo.DeleteAllItems();
	m_gridListTempInfo.SetColumnCount(enumColumnName3_End);
	m_gridListTempInfo.SetFixedRowCount(1);

	for (long n = 0; n < enumColumnName3_End; ++n)
	{
		m_gridListTempInfo.SetColumnWidth(n, ColumnSize3[n]);
		m_gridListTempInfo.SetItemText(0, n, ColumnName3[n]);
		m_gridListTempInfo.SetItemState(0, n, GVIS_READONLY);
		m_gridListTempInfo.SetItemBkColour(0, n, RGB(172, 172, 172));
		m_gridListTempInfo.SetItemFgColour(0, n, RGB(255, 255, 255));
	}

	// Items + Header
	m_gridListTempInfo.SetEditable(TRUE);
	//m_gridListTempInfo.SetRowCount(1 + 18);
	m_gridListTempInfo.SetRowCount(1 + FDC_WDATA_TEMP_LIST_CNT + 2);	// LYS 2026-09-19 헤더 + 온도 10 + ESD + LIFT PIN
	m_gridListTempInfo.SetRowHeight(0, 19);
	//for (long i = 0; i < 18; i++)
	for (long i = 0; i < FDC_WDATA_TEMP_LIST_CNT + 2; i++)
	{
		m_gridListTempInfo.SetRowHeight(i + 1, 15);
	}

	__RefreshListData5();
}


void CDlgSubOperate::__RefreshListData5()
{
	m_gridListTempInfo.SetRedraw(FALSE);

	int i = 0;
	CString strData;
	bool bCheck = false;
	double dValue = 0, dValueMax = 0, dValueMin = 0;
	int nRow = 0;
	//for ( nRow = 0; nRow < 16; nRow++)
	for ( nRow = 0; nRow < FDC_WDATA_TEMP_LIST_CNT; nRow++)	// LYS 2026-09-19 FDC 가 보내는 온도 10 개
	{
		// Name 
		m_gridListTempInfo.GetCell(i + 1, enumColumnNameMeas_Name)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridListTempInfo.SetItemState(i + 1, enumColumnNameMeas_Name, GVIS_READONLY);
		m_gridListTempInfo.SetItemText(i + 1, enumColumnNameMeas_Name, szNameRow[i]);

		// Value 
		CMCCLogMgr::GetInstance()->GetFDCDValue(FDC_WDATA_TEMP_1+nRow, &dValue, &dValueMax, &dValueMin);
		strData.Format(_T("%.1f"), dValue);

		m_gridListTempInfo.GetCell(i + 1, enumColumnNameMeas_Value1)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridListTempInfo.SetItemState(i + 1, enumColumnNameMeas_Value1, GVIS_READONLY);
		m_gridListTempInfo.SetItemText(i + 1, enumColumnNameMeas_Value1, strData);

		for (long n = 0; n < enumColumnNameMeas_End; ++n)
		{
			m_gridListTempInfo.SetItemFgColour(i + 1, n, RGB(0, 0, 0));
		}

		i++;
	}

	//for (nRow = 16; nRow < 17; nRow++)
	for (nRow = FDC_WDATA_TEMP_LIST_CNT; nRow < FDC_WDATA_TEMP_LIST_CNT + 1; nRow++)	// LYS 2026-09-19 ESD 행
	{
		// Name 
		m_gridListTempInfo.GetCell(i + 1, enumColumnNameMeas_Name)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridListTempInfo.SetItemState(i + 1, enumColumnNameMeas_Name, GVIS_READONLY);
		m_gridListTempInfo.SetItemText(i + 1, enumColumnNameMeas_Name, szNameRow[i]);

		// Value 
		CMCCLogMgr::GetInstance()->GetFDCDValue(FDC_WDATA_ESD, &dValue, &dValueMax, &dValueMin);
		strData.Format(_T("%d"), (int)( dValue * 1000) );

		m_gridListTempInfo.GetCell(i + 1, enumColumnNameMeas_Value1)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridListTempInfo.SetItemState(i + 1, enumColumnNameMeas_Value1, GVIS_READONLY);
		m_gridListTempInfo.SetItemText(i + 1, enumColumnNameMeas_Value1, strData);

		for (long n = 0; n < enumColumnNameMeas_End; ++n)
		{
			m_gridListTempInfo.SetItemFgColour(i + 1, n, RGB(0, 0, 0));
		}

		i++;
	}

	// Lift Pin
	//for (nRow = 17; nRow < 18; nRow++)
	for (nRow = FDC_WDATA_TEMP_LIST_CNT + 1; nRow < FDC_WDATA_TEMP_LIST_CNT + 2; nRow++)	// LYS 2026-09-19 LIFT PIN 행
	{
		// Name 
		m_gridListTempInfo.GetCell(i + 1, enumColumnNameMeas_Name)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridListTempInfo.SetItemState(i + 1, enumColumnNameMeas_Name, GVIS_READONLY);
		m_gridListTempInfo.SetItemText(i + 1, enumColumnNameMeas_Name, szNameRow[i]);

		// Value 
		strData.Format(_T("%.2f"), m_pAuroraMgr->GetMaintControlMgr()->GetCurPos(AXIS_LOADER_Z));

		m_gridListTempInfo.GetCell(i + 1, enumColumnNameMeas_Value1)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		m_gridListTempInfo.SetItemState(i + 1, enumColumnNameMeas_Value1, GVIS_READONLY);
		m_gridListTempInfo.SetItemText(i + 1, enumColumnNameMeas_Value1, strData);

		for (long n = 0; n < enumColumnNameMeas_End; ++n)
		{
			m_gridListTempInfo.SetItemFgColour(i + 1, n, RGB(0, 0, 0));
		}

		i++;
	}

	m_gridListTempInfo.ExpandColumnsToFit(TRUE);
	m_gridListTempInfo.SetRedraw(TRUE);
}


void CDlgSubOperate::OnBnClickedButtonInitial()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_MainOperate::OnBnClickedButtonInitial"));

	// Alarm Check
	// jsyoon 2022.07.05 add 
	int nAlarm = CAlarmController::GetInstance()->GetAlarmCount();
	if (nAlarm > 0)
	{
		CString str;
		CAlarmController::GetInstance()->GetAlarm(39, str);
		CD_DisplayMessage dlg(str);
		dlg.DoModal();
		return;
	}

	// jsyoon 2022.07.19 add
	if (CSystemParamShare::GetInstance()->IsCycleState() == true)
	{
		CString str;
		CAlarmController::GetInstance()->GetAlarm(35, str);
		CD_DisplayMessage dlg(str);
		dlg.DoModal();
		return;
	}

	if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_ProcessStatus() != SPS_IDLE)
	{
		CString str;
		CAlarmController::GetInstance()->GetAlarm(35, str);
		CD_DisplayMessage dlg(str);
		dlg.DoModal();
		return;
	}

	if (CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Is_ThreadRun() == true)
	{
		CString str;
		CAlarmController::GetInstance()->GetAlarm(35, str);
		CD_DisplayMessage dlg(str);
		dlg.DoModal();
		return;
	}
	///--------------------------------------------------------------------


	//for (size_t i = 0; i < 4; i++) 
	//{
	//	m_pAuroraMgr->GetLightControlMgrPtr()->SetLedLampIntensity(i, 110);
	//}

	//return;

	//if (m_pAuroraMgr->GetRecipeEditMgrPtr()->IsRecipeOpened() == false)
	//{
	//	CD_DisplayMessage dlg(_T("There is no recipe information. Open the recipe."));
	//	dlg.DoModal();
	//	return;
	//}

	//----------------
	if (m_pAuroraMgr->GetSystemConfigMgrPtr()->IsUseHardwareSensor() == true)
	{
		if (m_pAuroraMgr->GetAlarmMonitorMgrPtr()->IsSafetyResetOn() == false)
		{
			CD_DisplayMessage dlg(_T("Please, check MC-ON Button, press the button."));
			dlg.DoModal();
			return;
		}
	}

	CString strMsg;

	const int iLanguageIndex = m_pAuroraMgr->GetSystemConfigMgrPtr()->GetLanguageIndex();

	if (iLanguageIndex == 0)
		strMsg.Format(_T("이니셜 진행하시겠습니까???"));
	else
		strMsg.Format(_T("Do you want Initialize???"));

	//----------------
	if (AfxMessageBox(strMsg, MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
		// 이니셜을 하는 경우에는 모든 사이클의 정보를 초기화 시켜줘야 한다.
		CSequenceManager::GetInstance()->EngineCycleInit();
//		CD_MoveHomePosition dlg(m_pAuroraMgr->GetSystemConfigMgrPtr(), m_pAuroraMgr->GetIOMapControlMgrPtr());
//		if (dlg.DoModal() == IDOK)
		{
			CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CHANGE_NOT_INITIALIZE, NULL, NULL);

			// 이니셜을 수행한다.
			m_btnImage[BUTTON_IMAGE_CYCLE].EnableWindow(FALSE);
			m_btnImage[BUTTON_IMAGE_RESUME].EnableWindow(FALSE);
			m_btnImage[BUTTON_IMAGE_PAUSE].EnableWindow(TRUE);
			m_btnImage[BUTTON_IMAGE_STOP].EnableWindow(TRUE);

	//		m_pSystemInitializingDlg->ShowWindow(SW_SHOW);
			m_pSystemInitializingDlg->InitStart();

			MoveCommandProcess(_T("SEQUENCE_ITEM_INITIALIZE"), false);
			CSequenceManager::GetInstance()->GetSystemInitialize()->Start();

			/*bool bRet = CSequenceManager::GetInstance()->GetSystemInitialize()->Is_Complete();
			if (!bRet)
			{
				return;
			}

			m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->bSystemInitOk = TRUE;*/
		}
	}
}


void CDlgSubOperate::OnBnClickedButtonMoveSafety()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_MainOperate::OnBnClickedButtonMoveSafety"));

	// Alarm Check
	// jsyoon 2022.07.05 add 
	int nAlarm = CAlarmController::GetInstance()->GetAlarmCount();
	if (nAlarm > 0)
	{
		CString str;
		CAlarmController::GetInstance()->GetAlarm(39, str);
		CD_DisplayMessage dlg(str);
		dlg.DoModal();
		return;
	}

	// jsyoon 2022.07.19 add
	if (CSystemParamShare::GetInstance()->IsCycleState() == true)
	{
		CString str;
		CAlarmController::GetInstance()->GetAlarm(35, str);
		CD_DisplayMessage dlg(str);
		dlg.DoModal();
		return;
	}

	if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_ProcessStatus() != SPS_IDLE)
	{
		CString str;
		CAlarmController::GetInstance()->GetAlarm(35, str);
		CD_DisplayMessage dlg(str);
		dlg.DoModal();
		return;
	}

	if (CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Is_ThreadRun() == true)
	{
		CString str;
		CAlarmController::GetInstance()->GetAlarm(35, str);
		CD_DisplayMessage dlg(str);
		dlg.DoModal();
		return;
	}
	///--------------------------------------------------------------------

	CString str;
	CAlarmController::GetInstance()->GetAlarm(40, str);
	if (AfxMessageBox(str, MB_YESNO | MB_ICONQUESTION) != IDYES) return;
	
	if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsSafetySensorOn() != true)
	{
		CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->SetEQStatusChangeStep(enumEQSTATUS_SAFETY);
		CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Start();

		/*bool bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Is_Complete();
		if (!bRet) 
		{
			return;
		}*/
	}
}


void CDlgSubOperate::OnBnClickedButtonChangeLoad()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_MainOperate::OnBnClickedButtonChangeLOAD"));

	// Alarm Check
	// jsyoon 2022.07.05 add 
	int nAlarm = CAlarmController::GetInstance()->GetAlarmCount();
	if (nAlarm > 0)
	{
		CString str;
		CAlarmController::GetInstance()->GetAlarm(39, str);
		CD_DisplayMessage dlg(str);
		dlg.DoModal();
		return;
	}

	// jsyoon 2022.07.19 add
	if (CSystemParamShare::GetInstance()->IsCycleState() == true)
	{
		CString str;
		CAlarmController::GetInstance()->GetAlarm(35, str);
		CD_DisplayMessage dlg(str);
		dlg.DoModal();
		return;
	}

	if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_ProcessStatus() != SPS_IDLE)
	{
		CString str;
		CAlarmController::GetInstance()->GetAlarm(35, str);
		CD_DisplayMessage dlg(str);
		dlg.DoModal();
		return;
	}

	if (CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Is_ThreadRun() == true)
	{
		CString str;
		CAlarmController::GetInstance()->GetAlarm(35, str);
		CD_DisplayMessage dlg(str);
		dlg.DoModal();
		return;
	}

	CString str;
	CAlarmController::GetInstance()->GetAlarm(41, str);
	if (AfxMessageBox(str, MB_YESNO | MB_ICONQUESTION) != IDYES) return;

	int nStatus = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->GetEQState();
	if (nStatus == enumEQStateReady)
	{
		CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->SetEQStatusChangeStep(enumEQSTATUS_R2L);
		CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Start();

		Sleep(500); // 2023.08.09 add
	}
	else if (nStatus == enumEQStateMeas)
	{
		CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->SetEQStatusChangeStep(enumEQSTATUS_M2L);
		CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Start();

		Sleep(500); // 2023.08.09 add
	}

	/*bool bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Is_Complete();
	if (!bRet)
	{
		return;
	}*/
}


void CDlgSubOperate::OnBnClickedButtonChangeMeas()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_MainOperate::OnBnClickedButtonChangeMeas"));

	// Alarm Check
	// jsyoon 2022.07.05 add 
	int nAlarm = CAlarmController::GetInstance()->GetAlarmCount();
	if (nAlarm > 0)
	{
		CString str;
		CAlarmController::GetInstance()->GetAlarm(39, str);
		CD_DisplayMessage dlg(str);
		dlg.DoModal();
		return;
	}

	// jsyoon 2022.07.19 add
	if (CSystemParamShare::GetInstance()->IsCycleState() == true)
	{
		CString str;
		CAlarmController::GetInstance()->GetAlarm(35, str);
		CD_DisplayMessage dlg(str);
		dlg.DoModal();
		return;
	}

	if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_ProcessStatus() != SPS_IDLE)
	{
		CString str;
		CAlarmController::GetInstance()->GetAlarm(35, str);
		CD_DisplayMessage dlg(str);
		dlg.DoModal();
		return;
	}

	if (CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Is_ThreadRun() == true)
	{
		CString str;
		CAlarmController::GetInstance()->GetAlarm(35, str);
		CD_DisplayMessage dlg(str);
		dlg.DoModal();
		return;
	}

	CString str;
	CAlarmController::GetInstance()->GetAlarm(42, str);
	if (AfxMessageBox(str, MB_YESNO | MB_ICONQUESTION) != IDYES) return;

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

	//bool bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Is_Complete();
	//if (!bRet)
	//{
	//	return;
	//}
}


void CDlgSubOperate::OnBnClickedButtonCycle()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_MainOperate::OnBnClickedButtonCycle"));

	//if (0 < CAlarmController::GetInstance()->GetAlarmCount())
	//{
	//	CD_DisplayMessage dlg(_T("First, clear the notification list item. After that, you can run the cycle."));
	//	dlg.DoModal();
	//	return;
	//}

	// Alarm Check
	// jsyoon 2022.07.05 add 
	int nAlarm = CAlarmController::GetInstance()->GetAlarmCount();
	if (nAlarm > 0)
	{
		CString str;
		CAlarmController::GetInstance()->GetAlarm(39, str);
		CD_DisplayMessage dlg(str);
		dlg.DoModal();
		return;
	}

	// jsyoon 2022.07.19 add
	if (CSystemParamShare::GetInstance()->IsCycleState() == true)
	{
		CString str;
		CAlarmController::GetInstance()->GetAlarm(35, str);
		CD_DisplayMessage dlg(str);
		dlg.DoModal();
		return;
	}

	if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_ProcessStatus() != SPS_IDLE)
	{
		CString str;
		CAlarmController::GetInstance()->GetAlarm(35, str);
		CD_DisplayMessage dlg(str);
		dlg.DoModal();
		return;
	}

	if (CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Is_ThreadRun() == true)
	{
		CString str;
		CAlarmController::GetInstance()->GetAlarm(35, str);
		CD_DisplayMessage dlg(str);
		dlg.DoModal();
		return;
	}

	// LYS 2026-10-08 Hybrid Phase 5 (review C9/K10) a resume start prompt is open (the CIM worker popup has no owner window): refuse a second start
	if (ResumePromptBusy())
	{
		CLogWrapper::GetInstance()->KMLOG(OPER, _T("Resume: another start prompt is open - CYCLE ignored"));
		CString str;
		CAlarmController::GetInstance()->GetAlarm(35, str);
		CD_DisplayMessage dlg(str);
		dlg.DoModal();
		return;
	}

//	m_pAuroraMgr->GetRecipeEditMgrPtr()->OpenRecipe(_T("KR1_DM0"));

	if (AfxMessageBox(_T("Do you want cycle Start sequence???"), MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
		CD_MeasureCycle dlg(m_pAuroraMgr->GetRecipeEditMgrPtr(), m_pAuroraMgr->GetIOMapControlMgrPtr());
		if (dlg.DoModal() == IDOK)
		{
			// LYS 2026-10-08 Hybrid Phase 5 (D5-10 a) resume check before the cycle state, buttons and START message change: CANCEL leaves everything
			//   as it was (no alarm 35 on the next CYCLE). The prompt guard is held meanwhile (dialog START is refused while the popup is open).
			bool bResumeOk = true;
			{
				CResumePromptBusy busy;
				bResumeOk = CheckAndApplyResumeMode();
			}
			if (!bResumeOk)
				return;

			// cycle mode 를 진행한다.
			CSystemParamShare::GetInstance()->SetCycleState(true);

			// 모든 버튼은 사용할 수 없다.
			CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CYCLE_INSPECTION_START, NULL, NULL);
			CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_TITLE, WM_MSG_UPDATE_CYCLE_INSPECTION_START, NULL, NULL);

			__RefreshListData3();

			m_btnImage[BUTTON_IMAGE_CYCLE].EnableWindow(FALSE);
			m_btnImage[BUTTON_IMAGE_RESUME].EnableWindow(FALSE);
			m_btnImage[BUTTON_IMAGE_PAUSE].EnableWindow(TRUE);
			m_btnImage[BUTTON_IMAGE_STOP].EnableWindow(TRUE);

			// 검사 시작
				// LYS 2026-07-07 Resume check (common function for manual and CIM paths)
				// LYS 2026-10-08 Hybrid Phase 5 (D5-10 a) the check moved before SetCycleState above

				CSequenceManager::GetInstance()->EngineStart();
		}
	}
}


void CDlgSubOperate::OnBnClickedButtonResume()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_btnImage[BUTTON_IMAGE_RESUME].EnableWindow(FALSE);
	m_btnImage[BUTTON_IMAGE_PAUSE].EnableWindow(TRUE);
	m_btnImage[BUTTON_IMAGE_STOP].EnableWindow(TRUE);

	CSequenceManager::GetInstance()->EngineResume();
}


void CDlgSubOperate::OnBnClickedButtonPause()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_btnImage[BUTTON_IMAGE_RESUME].EnableWindow(TRUE);
	m_btnImage[BUTTON_IMAGE_PAUSE].EnableWindow(FALSE);
	m_btnImage[BUTTON_IMAGE_STOP].EnableWindow(TRUE);
	CSequenceManager::GetInstance()->EnginePause();
}


void CDlgSubOperate::OnBnClickedButtonStop()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_MainOperate::OnBnClickedButtonStop"));

	//if (AfxMessageBox(_T("Do you want Stop sequence???"), MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
		BOOL ret = FALSE;
		//if (CSystemParamShare::GetInstance()->IsInitialized() == true)
		//{
		//	//if (m_pAuroraMgr->GetRecipeEditMgrPtr()->IsRecipeOpened() == true)
		//	//{
		//	//	ret = TRUE;
		//	//}
		//}

		AfxBeginThread(ThreadInspectEnd, this);

		/*m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectEnd(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_4);

		m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectEnd(enumTCPIpFactorytID_AGING);*/

		m_btnImage[BUTTON_IMAGE_CYCLE].EnableWindow(TRUE);
		m_btnImage[BUTTON_IMAGE_RESUME].EnableWindow(FALSE);
		m_btnImage[BUTTON_IMAGE_PAUSE].EnableWindow(FALSE);
		m_btnImage[BUTTON_IMAGE_STOP].EnableWindow(TRUE);

		// Initial 정지
		if (CSequenceManager::GetInstance()->GetSystemInitialize()->Is_Complete() == false)
		{
			m_pSystemInitializingDlg->InitStop();
		}

	//	CSequenceManager::GetInstance()->EngineStop();
	//	m_pSystemInitializingDlg->InitStop();

		// 검사 정지
		// LYS 2026-09-25 SACC(ProbeACC) 함수모듈도 세운다. 지금까지 STOP 은 초기화·Glass 엔진만 세워서, 클램프 락 대기에 갇힌
		//   ProbeACC 스레드(SPS_PROCESS 유지)는 STOP 으로 빠져나올 수 없었다. Stop() 은 StopUser 를 세우고 InitStep 으로 돌리며,
		//   진행 중인 PLC 대기(최대 30 초)가 끝나면 Idle 스텝이 SPS_IDLE 로 되돌린다.
		if (CSequenceManager::GetInstance()->GetFunctionModuleProbeACC() != nullptr &&
			CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->Is_ThreadRun() == true)
		{
			CLogWrapper::GetInstance()->KMLOG(WARN, _T("STOP - ProbeACC(SACC) sequence stop requested"));
			CSequenceManager::GetInstance()->GetFunctionModuleProbeACC()->Stop();
		}

		if (CSequenceManager::GetInstance()->IsEngineCompleteAll() == false)
		{
			CSequenceManager::GetInstance()->EngineStop();

			/*CD_DisplayMessage dlg(_T("It is forcibly stopped during inspection. Do your initials again."));
			dlg.DoModal();*/
		}
	}
}

void CDlgSubOperate::UpdateInitialState(BOOL bEnable)
{
	GetDlgItem(IDC_BUTTON_MOVE_SAFETY)->EnableWindow(bEnable);
	GetDlgItem(IDC_BUTTON_CHANGE_LOAD)->EnableWindow(bEnable);
	GetDlgItem(IDC_BUTTON_CHANGE_MEAS)->EnableWindow(bEnable);
	//	GetDlgItem(IDC_BUTTON_CHANGE_CONTROLMODE)->EnableWindow(bEnable);

	//m_pMainOperateInfo->EnableWindow(bEnable);
	//m_pMainOperateDeviceInfo->EnableWindow(bEnable);
	//m_pMainOperateRecipeInfo->EnableWindow(bEnable);
	//m_pMainOperateTactTimeInfo->EnableWindow(bEnable);
}


void CDlgSubOperate::UpdateCycleInspectionStart()
{
//	UpdateInitialState(FALSE);
	SetFunctionButtonStatus(FALSE);
}


void CDlgSubOperate::UpdateCycleInspectionComplete()
{
	SetFunctionButtonStatus(TRUE);

	//UpdateInitialState(TRUE);

	//m_btnImage[BUTTON_IMAGE_CYCLE].EnableWindow(TRUE);
	//m_btnImage[BUTTON_IMAGE_RESUME].EnableWindow(FALSE);
	//m_btnImage[BUTTON_IMAGE_PAUSE].EnableWindow(FALSE);
	//m_btnImage[BUTTON_IMAGE_STOP].EnableWindow(TRUE); // stop 버튼은 사용할 수 있어야 한다.
}


void CDlgSubOperate::UpdateChangeInitialize()
{
	SetFunctionButtonStatus(TRUE); 

	//UpdateInitialState(TRUE);


	//m_btnImage[BUTTON_IMAGE_CYCLE].EnableWindow(TRUE);
	//m_btnImage[BUTTON_IMAGE_RESUME].EnableWindow(FALSE);
	//m_btnImage[BUTTON_IMAGE_PAUSE].EnableWindow(FALSE);
	//m_btnImage[BUTTON_IMAGE_STOP].EnableWindow(TRUE); // stop 버튼은 사용할 수 있어야 한다.
}

bool CDlgSubOperate::MoveCommandProcess(CString strType, bool bCompleteCheck)
{
	return CSequenceManager::GetInstance()->EngineStart(strType, bCompleteCheck);
}

void CDlgSubOperate::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	// Lamp Time out
	//if (nIDEvent == MAIN_LAMP_OVER_TIMER_ID)
	//{
	//	for (int iLamp = 0; iLamp< df_MAX_USE_LAMP; iLamp++)
	//	{
	//		if (m_LmapTimer[iLamp].GetTimeOver() == TRUE)
	//		{
	//			m_pAuroraMgr->GetShareMemoryMgrPtr()->SetSys_LampTimeOver(iLamp, true);
	////			CAlarmController::GetInstance()->SetAlarm(JOB_LAMP_TIMEOUT);
	//		}
	//		else
	//		{
	//			m_pAuroraMgr->GetShareMemoryMgrPtr()->SetSys_LampTimeOver(iLamp, false);
	//		}
	//	}
	//}

	if (nIDEvent == MAIN_STATE_TIMER_ID)
	{
		UpdateCellStatus();
		UpdateHybridIndicator();	// LYS 2026-10-12 Hybrid Phase 6 (1-segment recipe: returns with nothing to do)
	}

	// LYS 2026-07-01 Probe Contact blink (Show 2s, Hide 1s)
	if (nIDEvent == PROBE_CONTACT_TIMER_ID)
	{
		KillTimer(PROBE_CONTACT_TIMER_ID);
		if (m_labelProbeContact.IsWindowVisible())
		{
			m_labelProbeContact.ShowWindow(SW_HIDE);
			SetTimer(PROBE_CONTACT_TIMER_ID, 500, NULL);
		}
		else
		{
			m_labelProbeContact.ShowWindow(SW_SHOW);
			SetTimer(PROBE_CONTACT_TIMER_ID, 2000, NULL);
		}
	}

	CMainSubDlgWin::OnTimer(nIDEvent);
}


void CDlgSubOperate::UpdateInformation()
{
//	__RefreshListData();
//	__RefreshListData2();
	__RefreshListData3();
	__RefreshListData5();

	UpdateFixtureInOut();

	UpdateCurCoord();

	int iLampTime = 0;
	// LYS 2026-09-25 공유메모리 LampTimeOver / MCC 램프시간은 SPO 채널 번호로 기록 (알람 모니터 JOB_LAMP_TIMEOUT_0n 이 채널 인덱스를 본다)
	for (int iLamp = 0; iLamp < df_MAX_USE_LAMP; iLamp++)
	{
		if (m_LmapTimer[iLamp].GetTimeOver() == TRUE) 
		{
			m_pAuroraMgr->GetShareMemoryMgrPtr()->SetSys_LampTimeOver(s_nLampTimerChannel[iLamp], true);
		}
		else
		{
			m_pAuroraMgr->GetShareMemoryMgrPtr()->SetSys_LampTimeOver(s_nLampTimerChannel[iLamp], false);
		}

		iLampTime = m_LmapTimer[iLamp].GetCurTime();
		CMCCLogMgr::GetInstance()->SetLampTime(s_nLampTimerChannel[iLamp], iLampTime);	// LYS 2026-09-25
	}

	CMCCLogMgr::GetInstance()->UpdateSensor();

//	UpdateCellStatus();
}

void CDlgSubOperate::UpdateDataLampTime(int iLamp, bool bOn)
{
	//// 각 Item 의 Object 에 Recipe 정보를 업데이트 한다.
	if (iLamp == -1)
	{
		for (size_t i = 0; i < df_MAX_USE_LAMP; i++)
		{
			if (bOn == true)
				m_LmapTimer[i].StartTimer();
			else
				m_LmapTimer[i].StopTimer();
		}
	}
	else
	{
		// LYS 2026-09-25 iLamp 는 SPO 채널 번호(LightControlMgr::SetLedLampIntensity). 슬롯으로 변환, 미사용 채널(0/1)은 무시
		const int nSlot = LampChannelToSlot(iLamp);
		if (nSlot >= 0)
		{
			if (bOn == true)
				m_LmapTimer[nSlot].StartTimer();
			else
				m_LmapTimer[nSlot].StopTimer();
		}
	}
}

void CDlgSubOperate::UpdateWriteLampTime(int iLamp, bool bOn)
{
	if (iLamp == -1)
	{
		for (size_t i = 0; i < df_MAX_USE_LAMP; i++)
		{
			if (bOn == true)
			{
				m_LmapTimer[i].ReloadRegistry(s_nLampTimerChannel[i]);	// LYS 2026-09-25
				m_LmapTimer[i].RefreshTimer();
				m_LmapTimer[i].SetRegistryWrite(TRUE);

			}
			else
			{
				m_LmapTimer[i].SetRegistryWrite(FALSE);
			}
				
		}
	}
	else
	{
		const int nSlot = LampChannelToSlot(iLamp);	// LYS 2026-09-25 채널 -> 슬롯
		if (nSlot >= 0)
		{
			if (bOn == true)
			{
				m_LmapTimer[nSlot].ReloadRegistry(iLamp);
				m_LmapTimer[nSlot].RefreshTimer();
				m_LmapTimer[nSlot].SetRegistryWrite(TRUE);
			}
			else
			{
				m_LmapTimer[nSlot].SetRegistryWrite(FALSE);
			}
		}
	}
}

void CDlgSubOperate::UpdateOperateStatus(CString str)
{
	m_LabelInfo[0].SetText(str);
}


void CDlgSubOperate::UpdateSeqItemElapsedTime()
{
	__InitialListData();

	//int nCount = 0;
	//nCount = m_pSeqTactTimeMgr->GetDataCount();

	//if (nCount == 1 || nCount > 500)
	//{
	//	// 결과값 리스트를 갱신한다.
	//	m_gridListTactTimeMeas.DeleteAllItems();
	//	m_gridListTactTimeMeas.SetColumnCount(enumColumnName_End);
	//	m_gridListTactTimeMeas.SetFixedRowCount(1);

	//	for (long n = 0; n < enumColumnName_End; ++n)
	//	{
	//		m_gridListTactTimeMeas.SetColumnWidth(n, ColumnSize[n]);
	//		m_gridListTactTimeMeas.SetItemText(0, n, ColumnName[n]);
	//		m_gridListTactTimeMeas.SetItemState(0, n, GVIS_READONLY);
	//		m_gridListTactTimeMeas.SetItemBkColour(0, n, RGB(172, 172, 172));
	//		m_gridListTactTimeMeas.SetItemFgColour(0, n, RGB(255, 255, 255));
	//	}

	//	// Items + Header
	//	m_gridListTactTimeMeas.SetEditable(TRUE);
	//	m_gridListTactTimeMeas.SetRowCount(nCount + 1);

	//	m_gridListTactTimeMeas.SetRowHeight(0, 22);
	//	for (long i = 0; i < nCount; i++)
	//	{
	//		m_gridListTactTimeMeas.SetRowHeight(i + 1, 0);
	//	}
	//}

	//__RefreshListData();

	// 마지막 행 보이도록 하기
	int icurpos = m_gridListTactTimeMeas.GetScrollLimit(SB_VERT);
	m_gridListTactTimeMeas.SetScrollPos(SB_VERT, icurpos);
}

void CDlgSubOperate::UpdateMeasureElapsedTime(bool bStart)
{
	if (bStart == true)
	{
		m_nMeasCount++;
	}

	__InitialListData2();

	// 마지막 행 보이도록 하기
	int icurpos = m_gridListTactTimeGlass.GetScrollLimit(SB_VERT);
	m_gridListTactTimeGlass.SetScrollPos(SB_VERT, icurpos);
}

void CDlgSubOperate::UpdateMeasCell(RECIPELAYOUT* pRecipe)
{
	// TODO: 여기에 구현 코드 추가.
	m_pParentCellMeasDlg->UpdateCellPosition3(pRecipe);
}


void CDlgSubOperate::RotateMeasPath(int nAngle)
{
	// TODO: 여기에 구현 코드 추가.

	double dMaxX = 0, dMinX = 0, dMaxY = 0, dMinY = 0;
	switch (nAngle)
	{
	case 0:	// 0
		dMaxX = m_pdPathSize[_MAX_SIZE_X];
		dMinX = m_pdPathSize[_MIN_SIZE_X];
		dMaxY = m_pdPathSize[_MAX_SIZE_Y];
		dMinY = m_pdPathSize[_MIN_SIZE_Y];
		break;
	case 1:	// 90
		dMaxX = m_pdPathSize[_MAX_SIZE_Y];
		dMinX = m_pdPathSize[_MIN_SIZE_Y];
		dMaxY = m_pdPathSize[_MAX_SIZE_X];
		dMinY = m_pdPathSize[_MIN_SIZE_X];
		break;
	case 2:	// 180
		dMaxX = m_pdPathSize[_MAX_SIZE_X];
		dMinX = m_pdPathSize[_MIN_SIZE_X];
		dMaxY = m_pdPathSize[_MAX_SIZE_Y];
		dMinY = m_pdPathSize[_MIN_SIZE_Y];
		break;
	case 3: // 270
		dMaxX = m_pdPathSize[_MAX_SIZE_Y];
		dMinX = m_pdPathSize[_MIN_SIZE_Y];
		dMaxY = m_pdPathSize[_MAX_SIZE_X];
		dMinY = m_pdPathSize[_MIN_SIZE_X];
		break;
	default:
		break;
	}

	if (m_pParentMeasPathDlg != nullptr)
	{
		m_pParentMeasPathDlg->SetMaxSize(dMaxX, dMinX, dMaxY, dMinY, m_bGraph_Reverse_X, m_bGraph_Reverse_Y, m_bGraph_InvertXY, nAngle);
	}

	//switch (nAngle)
	//{
	//case 0:	// 0
	//	dMaxX = m_pdGlassSize[_MAX_SIZE_X];
	//	dMinX = m_pdGlassSize[_MIN_SIZE_X];
	//	dMaxY = m_pdGlassSize[_MAX_SIZE_Y];
	//	dMinY = m_pdGlassSize[_MIN_SIZE_Y];
	//	break;
	//case 1:	// 90
	//	dMaxX = m_pdGlassSize[_MAX_SIZE_Y];
	//	dMinX = m_pdGlassSize[_MIN_SIZE_Y];
	//	dMaxY = m_pdGlassSize[_MAX_SIZE_X];
	//	dMinY = m_pdGlassSize[_MIN_SIZE_X];
	//	break;
	//case 2:	// 180
	//	dMaxX = m_pdGlassSize[_MAX_SIZE_X];
	//	dMinX = m_pdGlassSize[_MIN_SIZE_X];
	//	dMaxY = m_pdGlassSize[_MAX_SIZE_Y];
	//	dMinY = m_pdGlassSize[_MIN_SIZE_Y];
	//	break;
	//case 3: // 270
	//	dMaxX = m_pdGlassSize[_MAX_SIZE_Y];
	//	dMinX = m_pdGlassSize[_MIN_SIZE_Y];
	//	dMaxY = m_pdGlassSize[_MAX_SIZE_X];
	//	dMinY = m_pdGlassSize[_MIN_SIZE_X];
	//	break;
	//default:
	//	break;
	//}

	if (m_pParentCellMeasDlg != nullptr)
	{
		dMaxX = m_pdGlassSize[_MAX_SIZE_X];
		dMinX = m_pdGlassSize[_MIN_SIZE_X];
		dMaxY = m_pdGlassSize[_MAX_SIZE_Y];
		dMinY = m_pdGlassSize[_MIN_SIZE_Y];

	//	m_pParentCellMeasDlg->SetMaxSize(dMaxX, dMinX, dMaxY, dMinY, m_bGraph_Reverse_X, m_bGraph_Reverse_Y, m_bGraph_InvertXY, -1);
	//	m_pParentCellMeasDlg->SetMaxSize(dMaxX, dMinX, dMaxY, dMinY, 0, 0, 0, -1);
		m_pParentCellMeasDlg->SetMaxSize(dMaxX, dMinX, dMaxY, dMinY, 0, 1, 0, -1); // jsyoon 2022.07.18 change
	}

	/*if (m_iOldAngle != iAngle)
	{
		switch (m_iOldAngle)
		{
		case 0:
			m_btnPathView1.SetCheck(FALSE);
			break;
		case 1:
			m_btnPathView2.SetCheck(FALSE);
			break;
		case 2:
			m_btnPathView3.SetCheck(FALSE);
			break;
		case 3:
			m_btnPathView4.SetCheck(FALSE);
			break;
		default:
			break;
		}
	}
	else
	{
		switch (m_iOldAngle)
		{
		case 0:
			m_btnPathView1.SetCheck(TRUE);
			break;
		case 1:
			m_btnPathView2.SetCheck(TRUE);
			break;
		case 2:
			m_btnPathView3.SetCheck(TRUE);
			break;
		case 3:
			m_btnPathView4.SetCheck(TRUE);
			break;
		default:
			break;
		}
	}*/

//	m_iOldAngle = nAngle;

	return;
}


void CDlgSubOperate::RotateGlassPath(int nAngle)
{
	// TODO: 여기에 구현 코드 추가.

	int iGlsCCut = 0;
	iGlsCCut = m_pnGlassEdgeCut[nAngle];

	double dMaxX = 0, dMinX = 0, dMaxY = 0, dMinY = 0;
	switch (nAngle)
	{
	case 0:	// 0
		dMaxX = m_pdGlassSize[_MAX_SIZE_X];
		dMinX = m_pdGlassSize[_MIN_SIZE_X];
		dMaxY = m_pdGlassSize[_MAX_SIZE_Y];
		dMinY = m_pdGlassSize[_MIN_SIZE_Y];
		break;
	case 1:	// 90
		dMaxX = m_pdGlassSize[_MAX_SIZE_Y];
		dMinX = m_pdGlassSize[_MIN_SIZE_Y];
		dMaxY = m_pdGlassSize[_MAX_SIZE_X];
		dMinY = m_pdGlassSize[_MIN_SIZE_X];
		break;
	case 2:	// 180
		dMaxX = m_pdGlassSize[_MAX_SIZE_X];
		dMinX = m_pdGlassSize[_MIN_SIZE_X];
		dMaxY = m_pdGlassSize[_MAX_SIZE_Y];
		dMinY = m_pdGlassSize[_MIN_SIZE_Y];
		break;
	case 3: // 270
		dMaxX = m_pdGlassSize[_MAX_SIZE_Y];
		dMinX = m_pdGlassSize[_MIN_SIZE_Y];
		dMaxY = m_pdGlassSize[_MAX_SIZE_X];
		dMinY = m_pdGlassSize[_MIN_SIZE_X];
		break;
	default:
		break;
	}

	if (m_pParentMeasPathDlg != nullptr)
	{
		m_pParentMeasPathDlg->SetGlassSize(dMaxX, dMinX, dMaxY, dMinY, iGlsCCut);

		m_pParentMeasPathDlg->SetCardSize(m_pdCardSize[0], m_pdCardSize[1], dMaxY, dMinY); 
	}

	if (m_pParentCellMeasDlg != nullptr)
	{
//		iGlsCCut = m_pnGlassEdgeCut[0];

		dMaxX = m_pdGlassSize[_MAX_SIZE_X];
		dMinX = m_pdGlassSize[_MIN_SIZE_X];
		dMaxY = m_pdGlassSize[_MAX_SIZE_Y];
		dMinY = m_pdGlassSize[_MIN_SIZE_Y];

//		m_pParentCellMeasDlg->SetGlassSize(dMaxX, dMinX, dMaxY, dMinY, 1);
		m_pParentCellMeasDlg->SetGlassSize(dMaxX, dMinX, dMaxY, dMinY, 4); // jsyoon 2022.07.18 change
	}
	
//	m_iAngle = iAngle;

	return;
}


void CDlgSubOperate::GetConfig(bool bReload)
{
	// TODO: 여기에 구현 코드 추가.
	//if (bReload)
	//{
	//	//Added by Jay Hong : 2009.06.04
	//	gf_GetValue(_T("Glass Size"), _T("MAX_X"), _T("2500"), szValue, 10, stConfigPath);
	//	strVaule.Format(_T("%s"), szValue);
	//	m_pdGlassSize[_MAX_SIZE_X] = _tstof(strVaule);

	//	gf_GetValue(_T("Glass Size"), _T("MAX_Y"), _T("2500"), szValue, 10, stConfigPath);
	//	strVaule.Format(_T("%s"), szValue);
	//	m_pdGlassSize[_MAX_SIZE_Y] = _tstof(strVaule);

	//	gf_GetValue(_T("Glass Size"), _T("MIN_X"), _T("-200"), szValue, 10, stConfigPath);
	//	m_pdGlassSize[_MIN_SIZE_X] = _tstof(szValue);
	//	gf_GetValue(_T("Glass Size"), _T("MIN_Y"), _T("-400"), szValue, 10, stConfigPath);
	//	m_pdGlassSize[_MIN_SIZE_Y] = _tstof(szValue);

	//	gf_GetValue(_T("Path Size"), _T("MAX_X"), _T("2500"), szValue, 10, stConfigPath);
	//	m_pdPathSize[0] = _tstof(szValue);

	//	gf_GetValue(_T("Path Size"), _T("MIN_X"), _T("-200"), szValue, 10, stConfigPath);
	//	m_pdPathSize[1] = _tstof(szValue);

	//	gf_GetValue(_T("Path Size"), _T("MAX_Y"), _T("2500"), szValue, 10, stConfigPath);
	//	m_pdPathSize[2] = _tstof(szValue);

	//	gf_GetValue(_T("Path Size"), _T("MIN_Y"), _T("-400"), szValue, 10, stConfigPath);
	//	m_pdPathSize[3] = _tstof(szValue);

	//	gf_GetValue(_T("Graph Margin"), _T("ReverseX"), _T("1"), szValue, 2, stConfigPath);
	//	if (szValue[0] == '1')
	//		m_bGraph_Reverse_X = TRUE;
	//	else
	//		m_bGraph_Reverse_X = FALSE;

	//	gf_GetValue(_T("Graph Margin"), _T("ReverseY"), _T("1"), szValue, 2, stConfigPath);
	//	if (szValue[0] == '1')
	//		m_bGraph_Reverse_Y = TRUE;
	//	else
	//		m_bGraph_Reverse_Y = FALSE;

	//	gf_GetValue(_T("Graph Margin"), _T("InvertXY"), _T("1"), szValue, 2, stConfigPath);
	//	if (szValue[0] == '1')
	//		m_bGraph_InvertXY = TRUE;
	//	else
	//		m_bGraph_InvertXY = FALSE;

	///*	gf_GetValue(_T("Graph Margin"), _T("WTEDGECUT"), _T("0"), szValue, 2, stConfigPath);
	//	m_nWTEdgeCut = _tstoi(szValue);*/

	//	// jsyoon 20180831 add
	//	gf_GetValue(_T("GLASS EDGECUT"), _T("EDGECUT1"), _T("1"), szValue, 2, stConfigPath);
	//	m_pnGlassEdgeCut[0] = _tstoi(szValue);

	//	gf_GetValue(_T("GLASS EDGECUT"), _T("EDGECUT2"), _T("2"), szValue, 2, stConfigPath);
	//	m_pnGlassEdgeCut[1] = _tstoi(szValue);

	//	gf_GetValue(_T("GLASS EDGECUT"), _T("EDGECUT3"), _T("3"), szValue, 2, stConfigPath);
	//	m_pnGlassEdgeCut[2] = _tstoi(szValue);

	//	gf_GetValue(_T("GLASS EDGECUT"), _T("EDGECUT4"), _T("4"), szValue, 2, stConfigPath);
	//	m_pnGlassEdgeCut[3] = _tstoi(szValue);

	//	//gf_GetValue(_T("CELLUI"), _T("DIR"), _T("0"), szValue, 2, stConfigPath);
	//	//m_nCellUIDir = _tstoi(szValue);

	//	//gf_GetValue(_T("Glass Size"), _T("InvertXY"), _T("0"), szValue, 2, stConfigPath);
	//	//if (szValue[0] == '1')
	//	//	m_bInvertXY = true;
	//	//else
	//	//	m_bInvertXY = false;

	//	//if (szValue[0] == '1')
	//	//	m_pSystem->stSystem[VTM_MOTION].m_bInvertXY = TRUE;
	//	//else
	//	//	m_pSystem->stSystem[VTM_MOTION].m_bInvertXY = FALSE;

	//	return;
	//}


	////gf_GetValue(_T("Layout"), _T("Draw Panel"), _T("1"), szValue, 2, stConfigPath);
	////if (szValue[0] == '1')
	////	m_bPanelDraw = true;
	////else
	////	m_bPanelDraw = false;


	////gf_GetValue(_T("Layout"), _T("Ny"), _T("2"), szValue, 3, stConfigPath);
	////m_nPanelNy = _tstoi(szValue);

	////gf_GetValue(_T("Layout"), _T("Nx"), _T("3"), szValue, 3, stConfigPath);
	////m_nPanelNx = _tstoi(szValue);

	////gf_GetValue(_T("Layout"), _T("Dy"), _T("100"), szValue, 6, stConfigPath);
	////m_dPanelDy = _tstof(szValue);

	////gf_GetValue(_T("Layout"), _T("Dx"), _T("100"), szValue, 6, stConfigPath);
	////m_dPanelDx = _tstof(szValue);

	////gf_GetValue(_T("Layout"), _T("Ly"), _T("950"), szValue, 6, stConfigPath);
	////m_dPanelLy = _tstof(szValue);

	////gf_GetValue(_T("Layout"), _T("Lx"), _T("690"), szValue, 6, stConfigPath);
	////m_dPanelLx = _tstof(szValue);

	////gf_GetValue(_T("Layout"), _T("Y0"), _T("100"), szValue, 6, stConfigPath);
	////m_dPanelY0 = _tstof(szValue);

	////gf_GetValue(_T("Layout"), _T("X0"), _T("100"), szValue, 6, stConfigPath);
	////m_dPanelX0 = _tstof(szValue);

	////Added by Jay Hong : 2009.06.04
	//gf_GetValue(_T("Glass Size"), _T("MAX_X"), _T("2500"), szValue, 10, stConfigPath);
	//m_pdGlassSize[_MAX_SIZE_X] = _tstof(szValue);
	//gf_GetValue(_T("Glass Size"), _T("MAX_Y"), _T("2500"), szValue, 10, stConfigPath);
	//m_pdGlassSize[_MAX_SIZE_Y] = _tstof(szValue);

	//gf_GetValue(_T("Glass Size"), _T("MIN_X"), _T("-200"), szValue, 10, stConfigPath);
	//m_pdGlassSize[_MIN_SIZE_X] = _tstof(szValue);
	//gf_GetValue(_T("Glass Size"), _T("MIN_Y"), _T("-400"), szValue, 10, stConfigPath);
	//m_pdGlassSize[_MIN_SIZE_Y] = _tstof(szValue);

	//gf_GetValue(_T("Path Size"), _T("MAX_X"), _T("2500"), szValue, 10, stConfigPath);
	//m_pdPathSize[0] = _tstof(szValue);

	//gf_GetValue(_T("Path Size"), _T("MIN_X"), _T("-200"), szValue, 10, stConfigPath);
	//m_pdPathSize[1] = _tstof(szValue);

	//gf_GetValue(_T("Path Size"), _T("MAX_Y"), _T("2500"), szValue, 10, stConfigPath);
	//m_pdPathSize[2] = _tstof(szValue);

	//gf_GetValue(_T("Path Size"), _T("MIN_Y"), _T("-400"), szValue, 10, stConfigPath);
	//m_pdPathSize[3] = _tstof(szValue);


	//gf_GetValue(_T("Graph Margin"), _T("ReverseX"), _T("1"), szValue, 2, stConfigPath);
	//if (szValue[0] == '1')
	//	m_bGraph_Reverse_X = TRUE;
	//else
	//	m_bGraph_Reverse_X = FALSE;

	//gf_GetValue(_T("Graph Margin"), _T("ReverseY"), _T("1"), szValue, 2, stConfigPath);
	//if (szValue[0] == '1')
	//	m_bGraph_Reverse_Y = TRUE;
	//else
	//	m_bGraph_Reverse_Y = FALSE;

	//gf_GetValue(_T("Graph Margin"), _T("InvertXY"), _T("1"), szValue, 2, stConfigPath);
	//if (szValue[0] == '1')
	//	m_bGraph_InvertXY = TRUE;
	//else
	//	m_bGraph_InvertXY = FALSE;

	////	// jsyoon 20181004 add
	////gf_GetValue(_T("Graph Margin"), _T("WTEDGECUT"), _T("0"), szValue, 2, stConfigPath);
	////m_nWTEdgeCut = _tstoi(szValue);

	//gf_GetValue(_T("Graph Margin"), _T("ViewPos"), _T("0"), szValue, 2, stConfigPath);
	//m_nOldAngle = _tstoi(szValue);

	//// jsyoon 20180831 add
	//gf_GetValue(_T("GLASS EDGECUT"), _T("EDGECUT1"), _T("1"), szValue, 2, stConfigPath);
	//m_pnGlassEdgeCut[0] = _tstoi(szValue);

	//gf_GetValue(_T("GLASS EDGECUT"), _T("EDGECUT2"), _T("2"), szValue, 2, stConfigPath);
	//m_pnGlassEdgeCut[1] = _tstoi(szValue);

	//gf_GetValue(_T("GLASS EDGECUT"), _T("EDGECUT3"), _T("3"), szValue, 2, stConfigPath);
	//m_pnGlassEdgeCut[2] = _tstoi(szValue);

	//gf_GetValue(_T("GLASS EDGECUT"), _T("EDGECUT4"), _T("4"), szValue, 2, stConfigPath);
	//m_pnGlassEdgeCut[3] = _tstoi(szValue);

	///*gf_GetValue(_T("CELLUI"), _T("DIR"), _T("0"), szValue, 2, stConfigPath);
	//m_nCellUIDir = _tstoi(szValue);

	//gf_GetValue(_T("Glass Size"), _T("InvertXY"), _T("0"), szValue, 2, stConfigPath);
	//if (szValue[0] == '1')
	//	m_bInvertXY = true;
	//else
	//	m_bInvertXY = false;

	//if (szValue[0] == '1')
	//	m_pSystem->stSystem[VTM_MOTION].m_bInvertXY = TRUE;
	//else
	//	m_pSystem->stSystem[VTM_MOTION].m_bInvertXY = FALSE;*/
}


bool CDlgSubOperate::LinkIniFile(bool bSave)
{
	// TODO: 여기에 구현 코드 추가.
	CString strValue;
	// Glass
	m_pIniHelper->Link(bSave, _T("Glass Size"), _T("MAX_X"), &strValue, (CString)_T("2500"));
	m_pdGlassSize[_MAX_SIZE_X] = _tstof(strValue);

	m_pIniHelper->Link(bSave, _T("Glass Size"), _T("MIN_X"), &strValue, (CString)_T("-2500"));
	m_pdGlassSize[_MIN_SIZE_X] = _tstof(strValue);

	m_pIniHelper->Link(bSave, _T("Glass Size"), _T("MAX_Y"), &strValue, (CString)_T("2500"));
	m_pdGlassSize[_MAX_SIZE_Y] = _tstof(strValue);

	m_pIniHelper->Link(bSave, _T("Glass Size"), _T("MIN_Y"), &strValue, (CString)_T("-2500"));
	m_pdGlassSize[_MIN_SIZE_Y] = _tstof(strValue);

	// Path
	m_pIniHelper->Link(bSave, _T("Path Size"), _T("MAX_X"), &strValue, (CString)_T("2500"));
	m_pdPathSize[_MAX_SIZE_X] = _tstof(strValue);

	m_pIniHelper->Link(bSave, _T("Path Size"), _T("MIN_X"), &strValue, (CString)_T("-2500"));
	m_pdPathSize[_MIN_SIZE_X] = _tstof(strValue);

	m_pIniHelper->Link(bSave, _T("Path Size"), _T("MAX_Y"), &strValue, (CString)_T("2500"));
	m_pdPathSize[_MAX_SIZE_Y] = _tstof(strValue);

	m_pIniHelper->Link(bSave, _T("Path Size"), _T("MIN_Y"), &strValue, (CString)_T("-2500"));
	m_pdPathSize[_MIN_SIZE_Y] = _tstof(strValue);

	// GLASS EDGECUT
	m_pIniHelper->Link(bSave, _T("GLASS EDGECUT"), _T("EDGECUT1"), &strValue, (CString)_T("1"));
	m_pnGlassEdgeCut[0] = _tstoi(strValue);

	m_pIniHelper->Link(bSave, _T("GLASS EDGECUT"), _T("EDGECUT2"), &strValue, (CString)_T("1"));
	m_pnGlassEdgeCut[1] = _tstoi(strValue);

	m_pIniHelper->Link(bSave, _T("GLASS EDGECUT"), _T("EDGECUT3"), &strValue, (CString)_T("1"));
	m_pnGlassEdgeCut[2] = _tstoi(strValue);

	m_pIniHelper->Link(bSave, _T("GLASS EDGECUT"), _T("EDGECUT4"), &strValue, (CString)_T("1"));
	m_pnGlassEdgeCut[3] = _tstoi(strValue);

	m_pIniHelper->Link(bSave, _T("Graph Margin"), _T("ReverseX"), &strValue, (CString)_T("0"));
	if (strValue[0] == '1')
		m_bGraph_Reverse_X = TRUE;
	else
		m_bGraph_Reverse_X = FALSE;

	m_pIniHelper->Link(bSave, _T("Graph Margin"), _T("ReverseY"), &strValue, (CString)_T("0"));
	if (strValue[0] == '1')
		m_bGraph_Reverse_Y = TRUE;
	else
		m_bGraph_Reverse_Y = FALSE;

	m_pIniHelper->Link(bSave, _T("Graph Margin"), _T("InvertXY"), &strValue, (CString)_T("0"));
	if (strValue[0] == '1')
		m_bGraph_InvertXY = TRUE;
	else
		m_bGraph_InvertXY = FALSE;

	m_pIniHelper->Link(bSave, _T("Graph Margin"), _T("ViewPos"), &strValue, (CString)_T("0"));
	m_nOldAngle = _tstoi(strValue);

	// jsyoon 2022.07.20 add
	m_pIniHelper->Link(bSave, _T("COLOR"), _T("WAD"), &strValue, (CString)_T("1956630"));
	m_lMeasDoneColorWAD = _tstol(strValue);

	m_pIniHelper->Link(bSave, _T("COLOR"), _T("VISION"), &strValue, (CString)_T("1956630"));
	m_lMeasDoneColorVision = _tstol(strValue);

	m_pIniHelper->Link(bSave, _T("COLOR"), _T("ANGLE"), &strValue, (CString)_T("1956630"));
	m_lMeasDoneColorAngle = _tstol(strValue);

	m_pIniHelper->Link(bSave, _T("COLOR"), _T("COLOR"), &strValue, (CString)_T("1956630"));
	m_lMeasDoneColorColor = _tstol(strValue);

	return false;
}


void CDlgSubOperate::UpdateFixtureInOut()
{
	// TODO: 여기에 구현 코드 추가.
	bool bFixtureLIn[2];
	bool bFixtureFIn[2];
	bool bFixtureBIn[2];
	bool bFixtureRIn[2];
	ZeroMemory(&bFixtureLIn, sizeof(bFixtureLIn));
	ZeroMemory(&bFixtureFIn, sizeof(bFixtureFIn));
	ZeroMemory(&bFixtureBIn, sizeof(bFixtureBIn));
	ZeroMemory(&bFixtureRIn, sizeof(bFixtureRIn));

	bool bShtterUpDown[2];
	ZeroMemory(&bShtterUpDown, sizeof(bShtterUpDown));

	bool bShtterUpDownMoving[2];
	ZeroMemory(&bShtterUpDownMoving, sizeof(bShtterUpDownMoving));

	bool bFixtureLUp[2];
	bool bFixtureFUp[2];
	bool bFixtureBUp[2];
	bool bFixtureRUp[2];
	ZeroMemory(&bFixtureLUp, sizeof(bFixtureLUp));
	ZeroMemory(&bFixtureFUp, sizeof(bFixtureFUp));
	ZeroMemory(&bFixtureBUp, sizeof(bFixtureBUp));
	ZeroMemory(&bFixtureRUp, sizeof(bFixtureRUp));

	CIOMapControlMgr* pImpl = nullptr;
	pImpl = m_pAuroraMgr->GetIOMapControlMgrPtr();
	if (pImpl != nullptr)
	{
		bShtterUpDown[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_SUTTER_SENSOR_CLOSE);
		bShtterUpDown[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_SUTTER_SENSOR_OPEN);
		bShtterUpDownMoving[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_W_SUTTER_CLOSE_MOVE);
		bShtterUpDownMoving[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_W_SUTTER_OPEN_MOVE);

		switch (m_nOldAngle)
		{
		case 0:
			bFixtureLIn[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_LEFT_F01);
			bFixtureLIn[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_LEFT_F02);
			bFixtureFIn[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_FRONT_F01);
			bFixtureFIn[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_FRONT_F02);
			bFixtureBIn[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_BACK_F01);
			bFixtureBIn[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_BACK_F02);
			bFixtureRIn[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_RIGHT_F01);
			bFixtureRIn[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_RIGHT_F02);

			bFixtureLUp[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_LU01);
			bFixtureLUp[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_LU01);
			bFixtureFUp[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_FU01);
			bFixtureFUp[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_FU02);
			bFixtureBUp[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_BU01);
			bFixtureBUp[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_BU01);
			bFixtureRUp[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_RU01);
			bFixtureRUp[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_RU01);
			break;
		case 1:
			bFixtureBIn[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_LEFT_F01);
			bFixtureBIn[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_LEFT_F02);
			bFixtureLIn[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_FRONT_F01);
			bFixtureLIn[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_FRONT_F02);
			bFixtureRIn[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_BACK_F01);
			bFixtureRIn[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_BACK_F02);
			bFixtureFIn[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_RIGHT_F01);
			bFixtureFIn[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_RIGHT_F02);

			bFixtureLUp[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_LU01);
			bFixtureLUp[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_LU01);
			bFixtureFUp[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_FU01);
			bFixtureFUp[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_FU02);
			bFixtureBUp[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_BU01);
			bFixtureBUp[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_BU01);
			bFixtureRUp[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_RU01);
			bFixtureRUp[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_RU01);
			break;
		case 2:
			bFixtureRIn[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_LEFT_F01);
			bFixtureRIn[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_LEFT_F02);
			bFixtureBIn[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_FRONT_F01);
			bFixtureBIn[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_FRONT_F02);
			bFixtureFIn[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_BACK_F01);
			bFixtureFIn[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_BACK_F02);
			bFixtureLIn[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_RIGHT_F01);
			bFixtureLIn[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_RIGHT_F02);

			bFixtureLUp[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_LU01);
			bFixtureLUp[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_LU01);
			bFixtureFUp[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_FU01);
			bFixtureFUp[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_FU02);
			bFixtureBUp[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_BU01);
			bFixtureBUp[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_BU01);
			bFixtureRUp[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_RU01);
			bFixtureRUp[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_RU01);
			break;
		case 3:
			bFixtureFIn[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_LEFT_F01);
			bFixtureFIn[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_LEFT_F02);
			bFixtureRIn[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_FRONT_F01);
			bFixtureRIn[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_FRONT_F02);
			bFixtureLIn[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_BACK_F01);
			bFixtureLIn[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_BACK_F02);
			bFixtureBIn[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_RIGHT_F01);
			bFixtureBIn[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_RIGHT_F02);

			bFixtureLUp[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_LU01);
			bFixtureLUp[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_LU01);
			bFixtureFUp[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_FU01);
			bFixtureFUp[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_FU02);
			bFixtureBUp[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_BU01);
			bFixtureBUp[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_BU01);
			bFixtureRUp[0] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_RU01);
			bFixtureRUp[1] = pImpl->GetParam_DIOMapMemoryBitImport(PLC_BIT_FIXTURE_RU01);
			break;
		default:
			break;
		}
	}

	// In
	m_ledFixtureL[0].SetCheck(bFixtureLIn[0]);
	m_ledFixtureL[1].SetCheck(bFixtureLIn[1]);

	m_ledFixtureF[0].SetCheck(bFixtureFIn[0]);
	m_ledFixtureF[1].SetCheck(bFixtureFIn[1]);

	m_ledFixtureB[0].SetCheck(bFixtureBIn[0]);
	m_ledFixtureB[1].SetCheck(bFixtureBIn[1]);

	m_ledFixtureR[0].SetCheck(bFixtureRIn[0]);
	m_ledFixtureR[1].SetCheck(bFixtureRIn[1]);

	// Up
	m_ledFixtureLUp[0].SetCheck(bFixtureLUp[0]);
	m_ledFixtureLUp[1].SetCheck(bFixtureLUp[1]);

	m_ledFixtureFUp[0].SetCheck(bFixtureFUp[0]);
	m_ledFixtureFUp[1].SetCheck(bFixtureFUp[1]);

	m_ledFixtureBUp[0].SetCheck(bFixtureBUp[0]);
	m_ledFixtureBUp[1].SetCheck(bFixtureBUp[1]);

	m_ledFixtureRUp[0].SetCheck(bFixtureRUp[0]);
	m_ledFixtureRUp[1].SetCheck(bFixtureRUp[1]);

	if(bShtterUpDown[0] == true && bShtterUpDown[1] == false)	// Close
	{
		m_ledShtterUp.ShowWindow(SW_SHOW);
		m_ledShtterUp.SetCheck(false);
	}
	else if (bShtterUpDown[0] != true && bShtterUpDown[1] != false) // Open
	{
		m_ledShtterUp.ShowWindow(SW_SHOW);
		m_ledShtterUp.SetCheck(true);
	}
	else // Moving
	{
		m_ledShtterUp.ShowWindow(SW_HIDE);
		m_ledShtterUp.SetCheck(true);
	}
		
	if (bShtterUpDownMoving[0] == true && bShtterUpDownMoving[1] == false)
	{
		m_ledShtterMoving.ShowWindow(SW_SHOW);
		m_ledShtterMoving.SetCheck(true);
	}
	else if (bShtterUpDownMoving[0] == false && bShtterUpDownMoving[1] == true)
	{
		m_ledShtterMoving.ShowWindow(SW_SHOW);
		m_ledShtterMoving.SetCheck(false);
	}
	else
	{
		m_ledShtterMoving.ShowWindow(SW_HIDE);
		m_ledShtterMoving.SetCheck(false);
	}
}


void CDlgSubOperate::GenGlassPath()
{
	// TODO: 여기에 구현 코드 추가.
	int i = 0, nGroupCnt = 0, iNumBlock = 0;

	int iX = 0, iY = 0;
	for (iX = 0; iX < 100; iX++)
	{
		for (iY = 0; iY < 50; iY++)				// Stick 
		{
			m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[iX][iY] = df_CELLSTATE_ILDE;
			m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->nBFCellStatus[iX][iY] = -1;
		}
	}
	CResumeJournal::GetInstance()->NoteCellStatusReset();	// LYS 2026-10-08 Hybrid Phase 5 (5-1) a journal resume restores the cell status snapshot only after this reset (counter only, no log / I/O)

	BuildManualCellNameCache();	// LYS 2026-10-12 Hybrid Phase 6 1-segment manual Cell ID names (Auto/Auto or hybrid -> empty, no log)
	// LYS 2026-09-30 Hybrid Phase 6 operate cell map: >= 2 segments -> every segment on one layout, cached for the timer.
	//   1 segment: BuildHybridCellMap returns false without touching anything else and the code below runs unchanged.
	if (BuildHybridCellMap() == true)
		return;

	CELLINFO stGlassInfo;
	ZeroMemory(&stGlassInfo, sizeof(stGlassInfo));

	S_RecipeInfo* pstInfo = nullptr;
	pstInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();

	S_RecipeMap *pstMap = nullptr;
	pstMap = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeMap();

	// GlobalMark
	// 2023.02.03 change
//	stGlassInfo.GlobalMark.x = pstMap->m_fGlobalMark.x;
//	stGlassInfo.GlobalMark.y = pstMap->m_fGlobalMark.y;
	
	if (pstInfo->m_sOptionItem.m_nGlsRotate == 1)  // 90
	{
		stGlassInfo.GlobalMark.x = pstMap->m_fGlobalMark.x;
		stGlassInfo.GlobalMark.y = pstMap->m_fGlobalMark.y;

		stGlassInfo.CellEdig.x = pstMap->m_fCellCenter.x;
		stGlassInfo.CellEdig.y = pstMap->m_fCellCenter.y;

		stGlassInfo.CellSize.x = pstMap->m_fCellSize.x;
		stGlassInfo.CellSize.y = pstMap->m_fCellSize.y;
		stGlassInfo.CellDistance.x = pstMap->m_fCellDistance.x;
		stGlassInfo.CellDistance.y = pstMap->m_fCellDistance.y;

		stGlassInfo.CellCount.x = pstMap->m_fCellCount.x;	// CELL
		stGlassInfo.CellCount.y = pstMap->m_fCellCount.y;	// STICK
	}
	else  // 0
	{
		stGlassInfo.GlobalMark.y = pstMap->m_fGlobalMark.x;
		stGlassInfo.GlobalMark.x = pstMap->m_fGlobalMark.y;

		stGlassInfo.CellEdig.y = pstMap->m_fCellCenter.x;
		stGlassInfo.CellEdig.x = pstMap->m_fCellCenter.y;

		stGlassInfo.CellSize.y = pstMap->m_fCellSize.x;
		stGlassInfo.CellSize.x = pstMap->m_fCellSize.y;
		stGlassInfo.CellDistance.y = pstMap->m_fCellDistance.x;
		stGlassInfo.CellDistance.x = pstMap->m_fCellDistance.y;

		stGlassInfo.CellCount.y = pstMap->m_fCellCount.x;	// CELL
		stGlassInfo.CellCount.x = pstMap->m_fCellCount.y;	// STICK
	}

	//stGlassInfo.GlobalMark.x = -735.15;
	//stGlassInfo.GlobalMark.y = -429.5;
	//stGlassInfo.CellEdig.x = 164.6454; // 50.132;
	//stGlassInfo.CellEdig.y = 76.8775; // 74.931;
	//stGlassInfo.CellSize.x = 128.680000;
	//stGlassInfo.CellSize.y = 140.1430000;
	//stGlassInfo.CellDistance.x = 200.680000;
	//stGlassInfo.CellDistance.y = 145.143000;
	//stGlassInfo.CellCount.y = 6;

	/*stGlassInfo.GlobalMark.x = -735.15;
	stGlassInfo.GlobalMark.y = -429.5;
	stGlassInfo.CellEdig.x = 49.54;
	stGlassInfo.CellEdig.y = 109.3;
	stGlassInfo.CellSize.x = 70.580000;
	stGlassInfo.CellSize.y = 127.960000;
	stGlassInfo.CellDistance.x = 75.83;
	stGlassInfo.CellDistance.y = 133.48;
	stGlassInfo.CellCount.y = 6;*/


	//int nGroupCnt = 0;
	//int i = 0;
	int iStart = 0, iEnd = 0;
	BOOL bToggle = TRUE;
	CString str, strBlockdistance, strStic;
	CString strTemp;
	CStringArray strArrayBlock, strArrayStick;
	strArrayBlock.RemoveAll();		// Block Distance
	strArrayStick.RemoveAll();		// Stick Count

	// Block distance
//	strBlockdistance.Format(_T("77.75"));
//	strBlockdistance.Format(_T("95.416"));
	strBlockdistance.Format(_T("0.8"));
	strBlockdistance.Format(_T("%s"), pstMap->m_chCellDistance);
	strBlockdistance.TrimRight();
	strBlockdistance += ",";

	strArrayBlock.Add(_T("0.0"));

	for (i = 0; i < strBlockdistance.GetLength(); i++)
	{
		if (bToggle)
		{
			iStart = i;
			bToggle = FALSE;
		}

		if (strBlockdistance.GetAt(i) == ',')
		{
			bToggle = TRUE;
			iEnd = i;
			iEnd -= iStart;

			strTemp = strBlockdistance.Mid(iStart, iEnd);
			strArrayBlock.Add(strTemp);

			iNumBlock++;
		}
	}

	// Stick count
//	strStic.Format(_T("7,5,7"));
//	strStic.Format(_T("10,10"));
//	strStic.Format(_T("6"));
	strStic.Format(_T("%s"), pstMap->m_chCellCount);
	strStic.TrimRight();
	strStic += ",";

	for (i = 0; i < strStic.GetLength(); i++)
	{
		if (bToggle)
		{
			iStart = i;
			bToggle = FALSE;
		}

		if (strStic.GetAt(i) == ',')
		{
			bToggle = TRUE;
			iEnd = i;
			iEnd -= iStart;

			strTemp = strStic.Mid(iStart, iEnd);
			strArrayStick.Add(strTemp);

			nGroupCnt++;
		}
	}

	iX = 0;
	for (i = 0; i < nGroupCnt; i++)
	{
		if (i < strArrayStick.GetSize() && strArrayStick.GetSize() > 0)
			iX += _ttoi(strArrayStick.GetAt(i));
	}

	stGlassInfo.CellCount.x = (float)iX;
	stGlassInfo.CellNum = nGroupCnt;


	RECIPELAYOUT stRecipe;
	stRecipe.arrCellLayout.RemoveAll();

	CELLLAYOUT stCell;
	ZeroMemory(&stCell, sizeof(stCell));

//	CArray <CELLINFO, CELLINFO&>   aryCellGroup;
	m_aryCellGroup.RemoveAll();

	CString strCellName, strText;
	int nOldCllX = 0;
	double dBlockDistance = 0;

	double dShiftX = 0, dShiftY = 0;
	int iDX = 1, iDY = 1;
	if (stGlassInfo.GlobalMark.x > 0)
	{
		iDX = -1;
		dShiftX = stGlassInfo.CellSize.x;
	}

	if (stGlassInfo.GlobalMark.y > 0)
	{
		iDY = -1;
		dShiftY = stGlassInfo.CellSize.y;
	}

	CELLINFO stCellInfo;
	ZeroMemory(&stCellInfo, sizeof(stCellInfo));

	for (i = 0; i < nGroupCnt; i++)
	{
		ZeroMemory(&stCellInfo, sizeof(stCellInfo));

		stCellInfo.GlobalMark   = stGlassInfo.GlobalMark;
		stCellInfo.CellSize     = stGlassInfo.CellSize;
		stCellInfo.CellDistance = stGlassInfo.CellDistance;

		if (i < strArrayBlock.GetSize() && strArrayBlock.GetSize() > 0)
			dBlockDistance += _ttof(strArrayBlock.GetAt(i));
		else if (strArrayBlock.GetSize() > 0)
			dBlockDistance += _ttof(strArrayBlock.GetAt(1));


		stCellInfo.CellNum = 1;
		stCellInfo.CellCount.x = (float)_ttoi(strArrayStick.GetAt(i));
		stCellInfo.CellCount.y = stGlassInfo.CellCount.y;

		// CELL EDGE로 계산
//		stCellInfo.CellOffset.x = stGlassInfo.GlobalMark.x + (float)(iDX * (stGlassInfo.CellEdig.x + dBlockDistance + dShiftX + (stGlassInfo.CellDistance.x * nOldCllX) + (i * stGlassInfo.CellSize.x )));
// 		stCellInfo.CellOffset.y = stGlassInfo.GlobalMark.y + (float)(iDY * (stGlassInfo.CellEdig.y + dShiftY ));
		// CELL CENTER로 계산
		stCellInfo.CellOffset.x = stGlassInfo.GlobalMark.x + (float)(iDX * (stGlassInfo.CellEdig.x + dShiftX) + (stGlassInfo.CellDistance.x * nOldCllX) + (dBlockDistance));
		stCellInfo.CellOffset.y = stGlassInfo.GlobalMark.y + (float)(iDY * (stGlassInfo.CellEdig.y + dShiftY));
	
		stCellInfo.nResult = 0;

		m_aryCellGroup.Add(stCellInfo);

		nOldCllX += (int)(stCellInfo.CellCount.x - 1);

		TRACE("Distance %.4f\n", stCellInfo.CellOffset.x);
	}

	pstInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();
	// LYS 2026-08-07 rotation-change: follow full 0/90/180/270 glass rotate view (was binary 0/1)
	{
		int nRotView = pstInfo->m_sOptionItem.m_nGlsRotate;
		if (nRotView < 0 || nRotView > 3) nRotView = 0;
		RotateGlassPath(nRotView);
	}

	//int iBlock = 0, iNumX = 0, iNumY = 0, iSumX = 0, iStick = 0;
	//double dGlobalX = 0, dGlobalY = 0, dStartX = 0, dStartY = 0, dCellX = 0, dCellY = 0, dCellDX = 0, dCellDY = 0;
	//iY = 0;

	//for (iBlock = 0; iBlock < iNumBlock; iBlock++)		// Block Count
	//{
	//	for (i = 0; i < nGroupCnt; i++)
	//	{
	//		ZeroMemory(&stCellInfo, sizeof(stCellInfo));

	//		stCellInfo = m_aryCellGroup.GetAt(i);

	//		dStartX = stCellInfo.CellOffset.x;
	//		dStartY = stCellInfo.CellOffset.y;

	//		dCellX = stCellInfo.CellSize.x;
	//		dCellY = stCellInfo.CellSize.y;

	//		dCellDX = stCellInfo.CellDistance.x;
	//		dCellDY = stCellInfo.CellDistance.y;

	//		iNumX = (int)stCellInfo.CellCount.x;
	//		iNumY = (int)stCellInfo.CellCount.y;

	//		// 1St Gen 
	//		for (iX = 0; iX < iNumX; iX++)					// Stick Count 
	//		{
	//			stCell.dSDx = dStartX + (dCellDX * iX) - (dCellX/2);
	//			stCell.dEDx = stCell.dSDx + dCellX;

	//			for (iY = 0; iY < iNumY; iY++)
	//			{
	//				stCell.dSDy = dStartY + (dCellDY * iY) - (dCellY / 2) ;
	//				stCell.dEDy = stCell.dSDy + dCellY;

	//				// CELL ID						
	//			//	strCellName.Format(_T("%c%02d"), iSumX + 65, iNumY - iY );
	//				strCellName.Format(_T("%c%02d"), iSumX + 65, iY + 1);
	//				_tcscpy_s(stCell.szCellName, 32, strCellName);

	//				stCell.color = CLR_GRAY;
	//				strText.Format(_T("IDLE"));
	//				_tcscpy_s(stCell.szCellText, 32, strText);

	//				stRecipe.arrCellLayout.Add(stCell);
	//			}

	//			iSumX++;
	//		}
	//	}	
	//}

	//stRecipe.m_iVersion = 1;
	//stRecipe.m_iCellCnt = (int)stRecipe.arrCellLayout.GetSize();

//	UpdateMeasCell(&stRecipe);
}


void CDlgSubOperate::OnBnClickedButton1()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	// 

	// jblee 20260804 removed Not used new M-map // TODO: Verify this
	//m_pAuroraMgr->GetMaintControlMgr()->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_BIT_TEMP_SENSOR_RESET_SWITCH_14, true);

	/*for (size_t i = 0; i < 15; i++)
	{
		m_pAuroraMgr->GetMaintControlMgr()->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_BIT_SMOKE_DETECT_01+i, true);
	}*/
	
//	GenGlassPath();

//	m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[2][0] = 10; 
//	UpdateCellStatus();
}


void CDlgSubOperate::UpdateCurCoord()
{
	// TODO: 여기에 구현 코드 추가.
	if (m_pParentMeasPathDlg != nullptr)
	{
		CMotorControlMgr* pImpl = nullptr;
		pImpl = m_pAuroraMgr->GetMaintControlMgr()->GetMotorControlMgrPtr();

		double dMotionPos[100];
		ZeroMemory(&dMotionPos, sizeof(dMotionPos));

		if (pImpl != nullptr)
		{
			for (int iAxis = 0; iAxis < AXIS_TYPE_END; iAxis++)
			{
				if (pImpl->GetAxisProperty(iAxis)->bUse == FALSE)
					continue;

				dMotionPos[iAxis] = pImpl->GetCurPos(iAxis);
			}
		}

		double dChangePos = 0;
		m_pAuroraMgr->GetMaintControlMgr()->GetPositionConfigMgr()->GetACPosition(&dChangePos, nullptr, nullptr);

		if (m_pAuroraMgr->GetSystemConfigMgrPtr()->IsOCP())
		{
			m_pParentMeasPathDlg->UpdateCurCoord(-5, dMotionPos[AXIS_PROBE_X9], 0, 0, 0);		// PROBE
		}
		else
		{
			m_pParentMeasPathDlg->UpdateCurCoord(-1, dMotionPos[AXIS_PROBE_X1], 0, 0, 0);		// PROBE
			m_pParentMeasPathDlg->UpdateCurCoord(-2, dMotionPos[AXIS_PROBE_X3], 0, 0, 0);		// PROBE
			m_pParentMeasPathDlg->UpdateCurCoord(-3, dMotionPos[AXIS_PROBE_X5], 0, 0, 0);		// PROBE
			m_pParentMeasPathDlg->UpdateCurCoord(-4, dMotionPos[AXIS_PROBE_X7], 0, 0, 0);		// PROBE
			m_pParentMeasPathDlg->UpdateCurCoord(-5, dMotionPos[AXIS_PROBE_X9], 0, 0, 0);		// PROBE
		}


		m_pParentMeasPathDlg->UpdateCurCoord(-10, dMotionPos[AXIS_X], dMotionPos[AXIS_LOADER_R] * -1, 0, 0);		// STAGE   // jsyoon 2022.09.29 add * -1
		//m_pParentMeasPathDlg->UpdateCurCoord(-11, (dMotionPos[AXIS_X4] - dChangePos ) , 0, 0, 0);		// ACX
		m_pParentMeasPathDlg->UpdateCurCoord(-11, (dMotionPos[AXIS_X4]), 0, 0, 0);		// ACX		// LYS 2026-09-03 ACX UI 이동 수정

		m_pParentMeasPathDlg->UpdateCurCoord(1, dMotionPos[AXIS_X2], dMotionPos[AXIS_Y1], 0, 0);		// HEAD 1
		m_pParentMeasPathDlg->UpdateCurCoord(0, dMotionPos[AXIS_X2], dMotionPos[AXIS_Y2], 0, 0);		// HEAD 2

		m_pParentMeasPathDlg->UpdateCurCoord(3, dMotionPos[AXIS_X3], dMotionPos[AXIS_Y3], 0, 0);		// HEAD 3
		m_pParentMeasPathDlg->UpdateCurCoord(2, dMotionPos[AXIS_X3], dMotionPos[AXIS_Y4], 0, 0);		// HEAD 4

		// LYS 2026-07-01 Probe Contact blinking indicator (1s toggle)
		double dCurPosZ5 = pImpl->GetCurPos(AXIS_PROBE_Z5);
		if (dCurPosZ5 >= 1)
		{
			if (!m_bProbeContactFlashing)
			{
				m_labelProbeContact.ShowWindow(SW_SHOW);
				SetTimer(PROBE_CONTACT_TIMER_ID, 2000, NULL);
				m_bProbeContactFlashing = true;
			}
		}
		else
		{
			if (m_bProbeContactFlashing)
			{
				KillTimer(PROBE_CONTACT_TIMER_ID);
				m_labelProbeContact.ShowWindow(SW_HIDE);
				m_bProbeContactFlashing = false;
			}
		}
	}
}


void CDlgSubOperate::OnBnClickedButtonChangeControlmode()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CLogWrapper::GetInstance()->KMLOG(OPER, _T("CD_MainOperate::OnBnClickedButtonChangeControlmode"));

	// Alarm Check
	// jsyoon 2022.07.05 add 
	int nAlarm = CAlarmController::GetInstance()->GetAlarmCount();
	if (nAlarm > 0)
	{
		CString str;
		CAlarmController::GetInstance()->GetAlarm(39, str);
		CD_DisplayMessage dlg(str);
		dlg.DoModal();
		return;
	}

	if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_MachineMode() == df_HOST_OFFLINE )		// jsyoon 2022.08.10 add
	{
		// jsyoon 2022.07.19 add
		if (CSystemParamShare::GetInstance()->IsCycleState() == true)
		{
			CString str;
			CAlarmController::GetInstance()->GetAlarm(35, str);
			CD_DisplayMessage dlg(str);
			dlg.DoModal();
			return;
		}

		if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_ProcessStatus() != SPS_IDLE)
		{
			CString str;
			CAlarmController::GetInstance()->GetAlarm(35, str);
			CD_DisplayMessage dlg(str);
			dlg.DoModal();
			return;
		}
	}
	
	if (CSequenceManager::GetInstance()->GetFunctionModuleEQStateChange()->Is_ThreadRun() == true)
	{
		CString str;
		CAlarmController::GetInstance()->GetAlarm(35, str);
		CD_DisplayMessage dlg(str);
		dlg.DoModal();
		return;
	}

	if (CSequenceManager::GetInstance()->GetDoWorkSequenceCIM()->Is_ThreadRun() == true)
	{
		CString str;
		CAlarmController::GetInstance()->GetAlarm(35, str);
		CD_DisplayMessage dlg(str);
		dlg.DoModal();
		return;
	}

	if (m_pAuroraMgr->GetSystemConfigMgrPtr()->IsUseHardwareSensor() == true)
	{
		if (m_pAuroraMgr->GetAlarmMonitorMgrPtr()->IsSafetyResetOn() == false)
		{
			CD_DisplayMessage dlg(_T("Please, check MC-ON Button, press the button."));
			dlg.DoModal();
			return;
		}
	}

	const int iLanguageIndex = m_pAuroraMgr->GetSystemConfigMgrPtr()->GetLanguageIndex();
	CString strMsg;
	if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_MachineMode() == df_HOST_OFFLINE && m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_ProcessStatus() == SPS_IDLE)
	{
		if (m_pAuroraMgr->GetMaintControlMgr()->WorkReadySACC() != VTM_RV_SUCCESSED)
		{
			if (iLanguageIndex == 0)
				strMsg.Format(_T("SACC 준비가 안되어 있습니다. SACC 확인하고 진행하시겠습니까???"));
			else
				strMsg.Format(_T("Not Ready PACC, Not check PACC???"));

			if (AfxMessageBox(strMsg, MB_OKCANCEL, NULL) == IDOK)
			{
				return;
			}
		}

		// GLASS Exist update
		if (m_pAuroraMgr->GetMaintControlMgr()->IsGlassExist() ^ m_pAuroraMgr->GetSystemConfigMgrPtr()->ReadGalssExistFile())	// XOR	
		{
			if (iLanguageIndex == 0)
				strMsg.Format(_T("GLASS EXIST 파일정보와 센서정보가 다릅니다. 확인 후 진행하세요!!"));
			else
				strMsg.Format(_T("GLASS EXIST SENSOR !!"));

			AfxMessageBox(strMsg);
			return;
		}

		CD_SystemHostMode dlg(m_pAuroraMgr);
		if (dlg.DoModal() != IDOK)
		{
			return;
		}

		// AT1
		MoveCommandProcess(_T("SEQUENCE_ITEM_CIM"), false);
		CSequenceManager::GetInstance()->GetDoWorkSequenceCIM()->SetCIMStatusChangeStep(enumCIMStatus_INIT);
		CSequenceManager::GetInstance()->GetDoWorkSequenceCIM()->Start();
	}
	else if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_MachineMode() == df_HOST_ONLINE_REMOTE )
	{
		CD_SystemHostMode dlg(m_pAuroraMgr);
		if (dlg.DoModal() != IDOK)
		{
			return;
		}

		// ATO
		m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectEnd(enumTCPIpFactorytID_CIM);
	}
	else
	{
		;
	}
}

UINT CDlgSubOperate::ThreadRun(LPVOID pParam)
{
	CDlgSubOperate* pProc = (CDlgSubOperate*)pParam;

	pProc->RunStart();

	return 0;
}

void CDlgSubOperate::UpdateCIMState(int iState)
{
	m_iState = iState;

	AfxBeginThread(ThreadRun, this);	
}

void CDlgSubOperate::UpdateControlMode(int iState)
{
	switch (iState)
	{
	case 0:
		EnableControlMode(FALSE);
		break;
	case 1:
		EnableControlMode(TRUE);
		break;
	case 2:
		SetFunctionButtonStatus(FALSE);
		break;
	case 3:
		SetFunctionButtonStatus(TRUE);
		break;
	default:
		break;
	}
}

void CDlgSubOperate::RunStart()
{
	int nLoop = 0;
	do
	{
		Sleep(100);

		if (CSequenceManager::GetInstance()->GetDoWorkSequenceCIM()->GetThreadRun() != true)
			break;

		nLoop++;

	} while (nLoop < 50);

	switch (m_iState)
	{
	case TO_SERVER_LDREQ:
	{
		m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->GetCommTcpIpObjectCIM()->SetUserMode(0);

		MoveCommandProcess(_T("SEQUENCE_ITEM_CIM"), false);
		CSequenceManager::GetInstance()->GetDoWorkSequenceCIM()->SetCIMStatusChangeStep(enumCIMStatus_LOAD_READY);
		CSequenceManager::GetInstance()->GetDoWorkSequenceCIM()->Start();
	}
	break;
	case TO_SERVER_PPID:
	{
		m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->GetCommTcpIpObjectCIM()->SetUserMode(1);

		MoveCommandProcess(_T("SEQUENCE_ITEM_CIM"), false);
		CSequenceManager::GetInstance()->GetDoWorkSequenceCIM()->SetCIMStatusChangeStep(enumCIMStatus_LOAD);
		CSequenceManager::GetInstance()->GetDoWorkSequenceCIM()->Start();
	}
	break;
	case TO_SERVER_PRCS:
	{
		MoveCommandProcess(_T("SEQUENCE_ITEM_CIM"), false);
		CSequenceManager::GetInstance()->GetDoWorkSequenceCIM()->SetCIMStatusChangeStep(enumCIMStatus_MEAS);
		CSequenceManager::GetInstance()->GetDoWorkSequenceCIM()->Start();
	}
	break;
	case TO_SERVER_ULDREQ:
	{
		MoveCommandProcess(_T("SEQUENCE_ITEM_CIM"), false);
		CSequenceManager::GetInstance()->GetDoWorkSequenceCIM()->SetCIMStatusChangeStep(enumCIMStatus_UNLOAD_READY);
		CSequenceManager::GetInstance()->GetDoWorkSequenceCIM()->Start();
	}
	break;
	case TO_SERVER_ULDCPL:
	{
		m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->GetCommTcpIpObjectCIM()->SetUserMode(0);

		MoveCommandProcess(_T("SEQUENCE_ITEM_CIM"), false);
		CSequenceManager::GetInstance()->GetDoWorkSequenceCIM()->SetCIMStatusChangeStep(enumCIMStatus_UNLOAD);
		CSequenceManager::GetInstance()->GetDoWorkSequenceCIM()->Start();
	}
	break;
	case TO_SERVER_GDTREQ:
	{
		MoveCommandProcess(_T("SEQUENCE_ITEM_CIM"), false);
		CSequenceManager::GetInstance()->GetDoWorkSequenceCIM()->SetCIMStatusChangeStep(enumCIMStatus_DATA_UPLOAD);
		CSequenceManager::GetInstance()->GetDoWorkSequenceCIM()->Start();
	}
	break;
	// int Start
	case TO_SERVER_INIT_START:
	{
		m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->GetCommTcpIpObjectCIM()->SetUserMode(0);

		MoveCommandProcess(_T("SEQUENCE_ITEM_CIM"), false);
		CSequenceManager::GetInstance()->GetDoWorkSequenceCIM()->SetCIMStatusChangeStep(enumCIMStatus_READY);
		CSequenceManager::GetInstance()->GetDoWorkSequenceCIM()->Start();
	}
	break;
	// Meas End
	case TO_SERVER_MEAS_END:
	{
		m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->GetCommTcpIpObjectCIM()->SetUserMode(2);

		MoveCommandProcess(_T("SEQUENCE_ITEM_CIM"), false);
		CSequenceManager::GetInstance()->GetDoWorkSequenceCIM()->SetCIMStatusChangeStep(enumCIMStatus_MEAS_DONE);
		CSequenceManager::GetInstance()->GetDoWorkSequenceCIM()->Start();
	}
	break;

	default:

	// Measure Start
	case TO_SERVER_MEAS_START:
	{
		// LYS 2026-10-08 Hybrid Phase 5 (D5-10 a) resume check first, inside the prompt guard: this popup runs on the RunStart worker without an
		//   owner window, so CYCLE / dialog START are refused while it is open. CANCEL leaves the cycle state and buttons as they were.
		bool bResumeOk = true;
		{
			CResumePromptBusy busy;
			bResumeOk = CheckAndApplyResumeMode();
		}
		if (!bResumeOk)
			break;

		// cycle mode 를 진행한다.
		CSystemParamShare::GetInstance()->SetCycleState(true);

		// 모든 버튼은 사용할 수 없다.
		CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_CYCLE_INSPECTION_START, NULL, NULL);
		CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_TITLE, WM_MSG_UPDATE_CYCLE_INSPECTION_START, NULL, NULL);

		__RefreshListData3();

		m_btnImage[BUTTON_IMAGE_CYCLE].EnableWindow(FALSE);

		if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_MachineMode() != df_HOST_ONLINE_REMOTE)
		{	
			m_btnImage[BUTTON_IMAGE_RESUME].EnableWindow(FALSE);
			m_btnImage[BUTTON_IMAGE_PAUSE].EnableWindow(TRUE);
			m_btnImage[BUTTON_IMAGE_STOP].EnableWindow(TRUE);
		}

		// LYS 2026-07-07 Resume check for CIM Online path
		// LYS 2026-10-08 Hybrid Phase 5 (D5-10 a) the check moved to the top of this case

		CSequenceManager::GetInstance()->EngineStart();
	}
	break;

		break;
	}

	/*
	int nLoop = 0;
	bool bRet = false;
	do
	{
		Sleep(200);

		bRet = CSequenceManager::GetInstance()->GetDoWorkSequenceCIM()->Is_Complete();
		if (bRet == true)
		{
			break;
		}

		nLoop++;

	} while (nLoop < 50);

	if (bRet != true)
		return; */

	
}

// LYS 2026-07-07 수동 버튼과 CIM Online 경로가 공용하는 Resume 확인.
// 반환값 true = 검사 진행, false = 사용자 취소.
bool CDlgSubOperate::CheckAndApplyResumeMode()
{
	CResumeInfoMgr* pResumeInfo = m_pAuroraMgr->GetResumeInfoMgrPtr();
	// LYS 2026-10-08 Hybrid Phase 5 journal (Format=2) bookmark -> ResumePrompt.cpp; a format-1 bookmark of this same journal recipe is refused there (D5-16 a).
	//   1-segment format-1 bookmark (or a format-1 bookmark of another recipe): the code below runs unchanged
	if (pResumeInfo && m_pAuroraMgr->GetSystemConfigMgrPtr()->IsUseResumeInspection())
	{
		int nEntry = (GetCurrentThreadId() != AfxGetApp()->m_nThreadID) ? RESUME_ENTRY_CIM : RESUME_ENTRY_OPERATE;	// only RunStart calls this off the UI thread
		if (pResumeInfo->IsTupleBookmark())
			return (ResumeTuplePrompt(m_pAuroraMgr, nEntry) != RESUME_PROMPT_CANCEL);
		if (CResumeJournal::GetInstance()->IsTupleGlass(m_pAuroraMgr->GetRecipeEditMgrPtr()->GetSegmentLoopCount())
			&& pResumeInfo->GetRecipeID().CompareNoCase(m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeID()) == 0
			&& ResumeRefuseLegacyHybrid(m_pAuroraMgr))
			return true;
	}

	if (pResumeInfo && m_pAuroraMgr->GetSystemConfigMgrPtr()->IsUseResumeInspection()
		&& pResumeInfo->IsResumeAvailable())
	{
		CString strCurRecipe = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeID();
		S_RecipeData* pRD = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeData();
		int nTotalStick = pRD ? (int)pRD->m_vecStick.size() : 0;
		// LYS 2026-07-21 Get current glass rotation for resume validation
		S_RecipeInfo* pRI = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo();
		int nGlsRotate = pRI ? pRI->m_sOptionItem.m_nGlsRotate : -1;

		if (!strCurRecipe.IsEmpty() && nTotalStick > 0
			&& !pResumeInfo->ValidateResume(strCurRecipe, nTotalStick, nGlsRotate))
		{
			AfxMessageBox(_T("이전 검사 정보가 현재 레시피와 맞지 않습니다 (레시피/스틱 수/회전 불일치).\n처음부터 검사합니다."), MB_OK | MB_ICONWARNING);
			pResumeInfo->ClearResumeInfo();
			CSystemParamShare::GetInstance()->SetResumeMode(false);
		}
		else
		{
			CString strMsg;
			strMsg.Format(_T("이전 검사 이력이 있습니다.\n\nGlass ID : %s\n레시피 : %s\n마지막 완료 스틱 : %d\n\n스틱 %d부터 이어서 검사하시겠습니까?\n\n예 = 이어서 검사\n아니요 = 처음부터 검사\n취소 = 검사 시작 취소"),
				pResumeInfo->GetGlassID(), pResumeInfo->GetRecipeID(),
				pResumeInfo->GetLastStick() + 1, pResumeInfo->GetResumeStartStick() + 1);
			int nRet = AfxMessageBox(strMsg, MB_YESNOCANCEL | MB_ICONQUESTION);
			if (nRet == IDYES)
			{
				CSystemParamShare::GetInstance()->SetResumeMode(true, pResumeInfo->GetResumeStartStick());
				CLogWrapper::GetInstance()->KMLOG(OPER, _T("Resume Start: user selected resume inspection"));
			}
			else if (nRet == IDNO)
			{
				pResumeInfo->ClearResumeInfo();
				CSystemParamShare::GetInstance()->SetResumeMode(false);
				CLogWrapper::GetInstance()->KMLOG(OPER, _T("Resume Cancel: user selected fresh start"));
			}
			else
			{
				CLogWrapper::GetInstance()->KMLOG(OPER, _T("Resume Abort: user cancelled inspection start"));
				return false;
			}
		}
	}
	else
	{
		CSystemParamShare::GetInstance()->SetResumeMode(false);
	}

	return true;
}

void CDlgSubOperate::EnableControlMode(BOOL bEnable)
{
	// TODO: 여기에 구현 코드 추가.
	GetDlgItem(IDC_BUTTON_CHANGE_CONTROLMODE)->EnableWindow(bEnable);
}

void CDlgSubOperate::SetFunctionButtonStatus(BOOL bEnable)
{
	// TODO: 여기에 구현 코드 추가.
	if (bEnable)
	{
		m_btnImage[BUTTON_IMAGE_RESUME].EnableWindow(FALSE);
		m_btnImage[BUTTON_IMAGE_PAUSE].EnableWindow(FALSE);
		m_btnImage[BUTTON_IMAGE_STOP].EnableWindow(TRUE);

		if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_MachineMode() != df_HOST_ONLINE_REMOTE)
		{
			BOOL bSystemInitOk = FALSE;
			bSystemInitOk = m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->bSystemInitOk;

			GetDlgItem(IDC_BUTTON_INITIAL)-> EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_MOVE_SAFETY)->EnableWindow(bSystemInitOk);
			GetDlgItem(IDC_BUTTON_CHANGE_LOAD)->EnableWindow(bSystemInitOk);
			GetDlgItem(IDC_BUTTON_CHANGE_MEAS)->EnableWindow(bSystemInitOk);
			GetDlgItem(IDC_BUTTON_CHANGE_CONTROLMODE)->EnableWindow(bSystemInitOk);
			m_btnImage[BUTTON_IMAGE_CYCLE].EnableWindow(bSystemInitOk);
		}
		else
		{
			GetDlgItem(IDC_BUTTON_INITIAL)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_MOVE_SAFETY)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_CHANGE_LOAD)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_CHANGE_MEAS)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_CHANGE_CONTROLMODE)->EnableWindow(TRUE);
			m_btnImage[BUTTON_IMAGE_CYCLE].EnableWindow(FALSE);
		}
	}
	else
	{
		if (m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSys_MachineMode() != df_HOST_ONLINE_REMOTE)
		{
			m_btnImage[BUTTON_IMAGE_RESUME].EnableWindow(FALSE);
			m_btnImage[BUTTON_IMAGE_PAUSE].EnableWindow(FALSE);
			m_btnImage[BUTTON_IMAGE_STOP].EnableWindow(TRUE);
		}
		else
		{
			m_btnImage[BUTTON_IMAGE_RESUME].EnableWindow(FALSE);
			m_btnImage[BUTTON_IMAGE_PAUSE].EnableWindow(FALSE);
			m_btnImage[BUTTON_IMAGE_STOP].EnableWindow(FALSE);
		}

		GetDlgItem(IDC_BUTTON_INITIAL)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_MOVE_SAFETY)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_CHANGE_LOAD)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_CHANGE_MEAS)->EnableWindow(FALSE);
		m_btnImage[BUTTON_IMAGE_CYCLE].EnableWindow(FALSE);
	}
}
void CDlgSubOperate::UpdateRecipeInformation()
{
	GenGlassPath();
	// LYS 2026-07-21 싸이클 돌때 Cell 업데이트 하지 않기
	//if (!CSystemParamShare::GetInstance()->IsCycleState())
	//	UpdateCellStatus();
}

void CDlgSubOperate::UpdateCellStatus()
{
	// LYS 2026-09-30 Hybrid Phase 6: a hybrid recipe draws the all-segment cache built by GenGlassPath. 1 segment: code below unchanged.
	if (m_nHybridMapSegCnt > 1)
	{
		UpdateHybridCellStatus();
		return;
	}

	RECIPELAYOUT stRecipe;
	stRecipe.arrCellLayout.RemoveAll();

	int iBlock = 0, iNumBlock = 1, nGroupCnt = 0, i = 0, iX = 0, iY = 0, iNumX = 0, iNumY = 0, iSumX = 0, iStick = 0;
	double dStartX = 0, dStartY = 0, dCellX = 0, dCellY = 0, dCellDX = 0, dCellDY = 0;
	CString strCellName, strText;

	CELLINFO stCellInfo;
	ZeroMemory(&stCellInfo, sizeof(stCellInfo));

	CELLLAYOUT stCell;
	ZeroMemory(&stCell, sizeof(stCell));

	int iStatus = 0, iOldStatus = 0;

	// jsyoon 2022.07.01 add
	S_RecipeMgr* pslData = nullptr;
	pslData = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeMgrPtr();
	int nStickCnt = 0, nCellCnt = 0;
	nStickCnt = pslData->m_nStickCnt;
	nCellCnt = pslData->m_nCellCnt;
	// LYS 2026-09-29 Hybrid 3b A8 no stick base here: m_aryCellGroup below is a 1-segment layout (status base 0).
	//   LYS 2026-09-30 hybrid recipes never reach this code - UpdateHybridCellStatus draws every segment with its own base (Phase 6).

	bool bChange = false;
	for (iX = 0; iX < 100; iX++)
	{	
		if (iX < nCellCnt)		// jsyoon 2022.07.01 add
		{
			for (iY = 0; iY < 50; iY++)				// Stick 
			{
				if (iY < nStickCnt)		// jsyoon 2022.07.01 add
				{
					iStatus = m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[iX][iY];
					iOldStatus = m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->nBFCellStatus[iX][iY];

					if (iStatus != iOldStatus)
					{
						bChange = true;
						break;
					}
				}
			}
		}

		if (bChange == true)
		{
			break;
		}
	}

	if (bChange == false)
	{
		return;
	}

	int nRotate = 0;
	S_RecipeInfo* pstInfo = nullptr;
	pstInfo = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeInfo(); 
	if (pstInfo->m_sOptionItem.m_nGlsRotate == 1)  // jsyoon 2022.09.29 change 0 to 1
	{
		nRotate = 1;
	}

	int iEndCell = 0;
	for (iBlock = 0; iBlock < iNumBlock; iBlock++)		// Block Count
	{
		nGroupCnt = m_aryCellGroup.GetSize();

		for (i = 0; i < nGroupCnt; i++)		// Block 
		{
			ZeroMemory(&stCellInfo, sizeof(stCellInfo));

			stCellInfo = m_aryCellGroup.GetAt(i);

			dStartX = stCellInfo.CellOffset.x;
			dStartY = stCellInfo.CellOffset.y;

			dCellX = stCellInfo.CellSize.x;
			dCellY = stCellInfo.CellSize.y;

			dCellDX = stCellInfo.CellDistance.x;
			dCellDY = stCellInfo.CellDistance.y;

			iNumX = (int)stCellInfo.CellCount.x;
			iNumY = (int)stCellInfo.CellCount.y;

			// 1St Gen 
			for (iX = 0; iX < iNumX; iX++)					// Cell Count 
			{
				stCell.dSDx = dStartX + (dCellDX * iX) - (dCellX / 2);
				stCell.dEDx = stCell.dSDx + dCellX;

				for (iY = 0; iY < iNumY; iY++)				// Stick 
				{
					stCell.dSDy = dStartY + (dCellDY * iY) - (dCellY / 2);
					stCell.dEDy = stCell.dSDy + dCellY;

					// CELL ID						
				//	strCellName.Format(_T("%c%02d"), iSumX + 65, iY + 1);
				//	strCellName.Format(_T("%c%02d"), iSumX + 65, iNumY - iY);
					strCellName.Format(_T("%c%02d"), iSumX + 65, iY + 1);
					_tcscpy_s(stCell.szCellName, 32, strCellName);

					int nCell = 0;
					int nStick = 0;

					// jsyoon 2022.09.29 change
					if (nRotate == 1)	// YY, BY
					{
						nCell = iEndCell + iX;
						nStick = iY;
					}
					else  // DG
					{
						nCell = iY;
						nStick = iEndCell + iX;
					}

					/*if (nRotate == 1)
					{
						nCell = iNumY - iY - 1;
						nStick = iEndCell + iX;
					}
					else
					{
						nCell = iEndCell + iX;
						nStick = iY;
					}*/

					// LYS 2026-10-12 Hybrid Phase 6 manual Cell ID (panel A) cached by GenGlassPath - empty cache (Auto/Auto) keeps the name above
					if (m_aryManualCellName.GetSize() == 100 * 50 && nCell >= 0 && nCell < 100 && nStick >= 0 && nStick < 50 && m_aryManualCellName[nCell * 50 + nStick].IsEmpty() != true)
						_tcsncpy_s(stCell.szCellName, _countof(stCell.szCellName), m_aryManualCellName[nCell * 50 + nStick], _TRUNCATE);
					iStatus = m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[nCell][nStick];

					m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory()->nBFCellStatus[nCell][nStick] = iStatus;

					switch (iStatus)
					{
					// Idle
					case df_CELLSTATE_ILDE:
						stCell.color = CLR_GRAY;
						strText.Format(_T("IDLE"));
						_tcscpy_s(stCell.szCellText, 32, strText);
						break;
					//	PTP
					case df_CELLSTATE_PTP_NOR:	//	Normal
						stCell.color = CLR_YELLOW;
						strText.Format(_T("PTP Normal"));
						_tcscpy_s(stCell.szCellText, 32, strText);
						break;
					case df_CELLSTATE_PTP_SHORT:	// Short
						stCell.color = CLR_RED;
						strText.Format(_T("PTP SHORT"));
						_tcscpy_s(stCell.szCellText, 32, strText);
						break;
					case df_CELLSTATE_PTP_OPEN:	// Open
						stCell.color = CLR_RED;
						strText.Format(_T("PTP OPEN"));
						_tcscpy_s(stCell.szCellText, 32, strText);
						break;
					//	OST
					case df_CELLSTATE_OST_NOR:	//	Normal
						stCell.color = CLR_YELLOW;
						strText.Format(_T("OST Normal"));
						_tcscpy_s(stCell.szCellText, 32, strText);
						break;
					case df_CELLSTATE_OST_SHORT:	// Short
						stCell.color = CLR_RED;
						strText.Format(_T("OST SHORT"));
						_tcscpy_s(stCell.szCellText, 32, strText);
						break;
					case df_CELLSTATE_OST_OPEN:	// Open
						stCell.color = CLR_RED;
						strText.Format(_T("OST OPEN"));
						_tcscpy_s(stCell.szCellText, 32, strText);
						break;
					// R-TEST
					case df_CELLSTATE_PASS:		// PASS
						stCell.color = CLR_YELLOW;
						strText.Format(_T("PASS"));
						_tcscpy_s(stCell.szCellText, 32, strText);
						break;
					case df_CELLSTATE_OVER:		// Over
						stCell.color = CLR_RED;
						strText.Format(_T("Over"));
						_tcscpy_s(stCell.szCellText, 32, strText);
						break;
					case df_CELLSTATE_UNDER:		// UNDER
						stCell.color = CLR_RED;
						strText.Format(_T("Under"));
						_tcscpy_s(stCell.szCellText, 32, strText);
						break;
					case df_CELLSTATE_PG_NOTCONNACT:		// PG 미연결
						stCell.color = CLR_RED;
						strText.Format(_T("NotCON"));
						break;
					case df_CELLSTATE_RTEST_FAIL:		// R-TEST FAIL
						stCell.color = CLR_RED;
						strText.Format(_T("FAIL"));
						_tcscpy_s(stCell.szCellText, 32, strText);
						break;
					// AGING
					case df_CELLSTATE_AGING_START:
						stCell.color = CLR_LIGHT_YELLOW;
						strText.Format(_T("AGING S"));
						_tcscpy_s(stCell.szCellText, 32, strText);
						break;
					case df_CELLSTATE_AGING_NG:		// NG
						stCell.color = CLR_RED;
						strText.Format(_T("AGING NG"));
						_tcscpy_s(stCell.szCellText, 32, strText);
						break;
					case df_CELLSTATE_AGING_OK:		// OK
						stCell.color = CLR_LIGHT_GRAY;
						strText.Format(_T("AGING END"));
						_tcscpy_s(stCell.szCellText, 32, strText);
						break;

					// Meas
					case df_CELLSTATE_WAD_START:		// 특성측정시작
						stCell.color = CLR_YELLOW;
						strText.Format(_T("WAD S"));
						_tcscpy_s(stCell.szCellText, 32, strText);
						break;
					case df_CELLSTATE_WAD_END:		// 특성측정완료
						stCell.color = COLORREF(m_lMeasDoneColorWAD); // RGB_TEST_1;
						strText.Format(_T("WAD END"));
						_tcscpy_s(stCell.szCellText, 32, strText);
						break;
					case df_CELLSTATE_ANGLE_START:		// 측정시작
						stCell.color = CLR_YELLOW;
						strText.Format(_T("시야각 S"));
						_tcscpy_s(stCell.szCellText, 32, strText);
						break;
					case df_CELLSTATE_ANGLE_END:		// 측정완료
						stCell.color = COLORREF(m_lMeasDoneColorAngle); //RGB_TEST_2;
						strText.Format(_T("시야각 END"));
						_tcscpy_s(stCell.szCellText, 32, strText);
						break;
					case df_CELLSTATE_VISION_START:		// 화질측정시작
						stCell.color = CLR_YELLOW;
						strText.Format(_T("화질 S"));
						_tcscpy_s(stCell.szCellText, 32, strText);
						break;
					case df_CELLSTATE_VISION_END:		// 화질측정완료
						stCell.color = COLORREF(m_lMeasDoneColorVision); //RGB_TEST_3;
						strText.Format(_T("화질 END"));
						_tcscpy_s(stCell.szCellText, 32, strText);
						break;
					case df_CELLSTATE_COLOR_START:		// 측정시작
						stCell.color = CLR_YELLOW;
						strText.Format(_T("면계측 S"));
						_tcscpy_s(stCell.szCellText, 32, strText);
						break;
					case df_CELLSTATE_COLOR_END:		// 측정완료
						stCell.color = COLORREF(m_lMeasDoneColorColor); //RGB_TEST_4;
						strText.Format(_T("면계측 END"));
						_tcscpy_s(stCell.szCellText, 32, strText);
						break;
					default:
						stCell.color = CLR_BLUE;
						strText.Format(_T("NOTDEFINED"));
						_tcscpy_s(stCell.szCellText, 32, strText);
						break;
					}

					stRecipe.arrCellLayout.Add(stCell);
				}

				iSumX++;
			}

			iEndCell += iNumX;
		}
	}

	stRecipe.m_iVersion = 1;
	stRecipe.m_iCellCnt = (int)stRecipe.arrCellLayout.GetSize();

	UpdateMeasCell(&stRecipe);
}


// LYS 2026-09-30 Hybrid Phase 6 operate cell map (docs/HybridGlass_Porting_Plan.md Phase 6)
//   One fixed glass frame for every segment (this chart never rotates): x = 1500 side, y = 925 side drawn downward
//   (SetMaxSize(..., 0, 1, 0, -1)), origin = glass centre.
//     s = StageY + m_sMap.m_fContactOffset  (glass coordinate of the stick = inverse of MakeCell), c = CellX (head Y)
//     rotate 1 (R90, long side)   : centre (-c, -s), size (CellSize.x, CellSize.y)
//     other rotates (R180, short) : centre ( s, -c), size (CellSize.y, CellSize.x)
//   LYS 2026-09-30 geometry keys on footprint: rotate 3 (R270) lays cells along 1500 like rotate 1 (CheckRecipeLayout) and
//     takes the rotate 1 geometry; the status column and fallback names keep MakeCellID's rotate == 1 rule. Same as MakeCell GlassMap.
//   That is where the 1-segment view (GenGlassPath block math) puts the cells of each rotation - identical for rotate 2;
//   for rotate 1 the 1-segment view mirrors the stick rows about the segment's own centre, here they stay rigid.
//   Status slot = nCellStatus[cell][segment stick base + MakeCellID stick] (rotate 1 direct, else N-1-i), i.e. the
//   column the measure step / PG write through ToCellStatusStick while that segment is active.
//   Segment vectors are read only here: GenGlassPath runs at recipe open / glass start with segment 0 active.
bool CDlgSubOperate::BuildHybridCellMap()
{
	CRecipeEditMgr* pRcp = m_pAuroraMgr->GetRecipeEditMgrPtr();
	C_OutRecipeFunc* pFunc = (pRcp != nullptr) ? pRcp->GetRecipeFuncPtr() : nullptr;
	if (pFunc == nullptr || pRcp->GetSegmentLoopCount() <= 1)
	{
		m_nHybridMapSegCnt = 0;			// 1 segment: pre-hybrid GenGlassPath / UpdateCellStatus
		m_aryHybridMapCell.RemoveAll();
		m_aryHybridSegBox.RemoveAll();
		m_aryHybridSegRot.RemoveAll();	// LYS 2026-10-12 Hybrid Phase 6 PATH view / indicator caches
		m_aryHybridSegName.RemoveAll();
		m_nHybridPathSeg = -1;
		m_nHybridPathRot = -1;
		return false;
	}

	CString strLog;
	int nActive = pRcp->GetActiveSegment();
	if (nActive != 0 && m_nHybridMapSegCnt > 1)
	{
		// not expected (no UI rebuild is posted mid-glass): keep the cache rather than read vectors the measure thread may be copying
		strLog.Format(_T("[HYBRID] Cell map rebuild skipped - active segment %d, cache kept"), nActive + 1);
		CLogWrapper::GetInstance()->KMLOG(WARN, strLog);
		m_nHybridMapFrameSeg = -2;
		return true;
	}

	m_aryHybridSegRot.RemoveAll();	// LYS 2026-10-12 Hybrid Phase 6
	m_aryHybridSegName.RemoveAll();
	m_aryHybridMapCell.RemoveAll();
	m_aryHybridSegBox.RemoveAll();
	m_aryCellGroup.RemoveAll();			// the 1-segment layout is not drawn while a hybrid recipe is open
	m_nHybridMapFrameSeg = -2;

	int nSegCnt = pFunc->GetSegmentCount();
	int nBase = 0, nSkip = 0;
	int nLetterBase = 0;	// LYS 2026-10-01 Hybrid Phase 4 (4-5) cell ID letters of the segments before k (same carry as CRecipeEditMgr::SyncCellStatusStickBase)
	// LYS 2026-10-12 Hybrid Phase 6 manual Cell ID config (panel A): tokens collected per cell and applied after the loop only when no
	//   recipe load / segment change overlapped the build. Auto/Auto: nothing collected, names / logs as before
	int nCidLoadTry = pFunc->GetRcpLoadTry();	// try count first, then the busy count (OutRecipeFunc.h m_nRcpLoadTry)
	BOOL bManualCid = (pFunc->IsRcpLoadBusy() != TRUE && pFunc->IsCellIdDefault() != TRUE);
	CArray <int, int> aryCidIdx;
	CStringArray aryCidTok;
	CString strCidTok;
	for (int k = 0; k < nSegCnt; k++)
	{
		S_RecipeMgr* pSeg = pFunc->GetSegmentMgr(k);	// valid until the next SetActiveSegment - nothing of it is kept
		int nStick = (pSeg != nullptr) ? (int)pSeg->m_sData.m_vecStick.size() : 0;
		int nRot = (pSeg != nullptr) ? pSeg->m_sInfo.m_sOptionItem.m_nGlsRotate : 0;
		m_aryHybridSegRot.Add((nRot >= 0 && nRot <= 3) ? nRot : 0);	// LYS 2026-10-12 Hybrid Phase 6 PATH view of segment k (UI thread, segment 0 active)
		m_aryHybridSegName.Add(pFunc->GetSegmentName(k));
		bool bLong = (nRot == 1);			// MakeCellID rule: only rotate 1 is direct
		bool bLongFoot = (nRot % 2 == 1);	// LYS 2026-09-30 geometry: rotate 1 / 3 = cells along 1500
		double dContact = 0.0, dW = 0.0, dH = 0.0;
		if (pSeg != nullptr)
		{
			dContact = pSeg->m_sMap.m_fContactOffset;
			dW = bLongFoot ? pSeg->m_sMap.m_fCellSize.x : pSeg->m_sMap.m_fCellSize.y;
			dH = bLongFoot ? pSeg->m_sMap.m_fCellSize.y : pSeg->m_sMap.m_fCellSize.x;
		}
		double dBox[4] = { 1.0e9, 1.0e9, -1.0e9, -1.0e9 };
		int nCellSeg = 0, nCellPerStick = 0;

		for (int i = 0; i < nStick; i++)
		{
			const S_StickInfo& sStick = pSeg->m_sData.m_vecStick[i];
			double dS = sStick.m_fStageY + dContact;
			int nCol = nBase + (bLong ? i : (nStick - i - 1));
			int nCells = (int)sStick.m_vecCell.size();
			if (nCells > nCellPerStick) nCellPerStick = nCells;

			for (int j = 0; j < nCells; j++)
			{
				if (j >= 100 || nCol < 0 || nCol >= 50)	// nCellStatus[100][50]
				{
					nSkip++;
					continue;
				}

				const S_CellInfo& sCell = sStick.m_vecCell[j];
				double dCX = bLongFoot ? -sCell.m_fCellX : dS;
				double dCY = bLongFoot ? -dS : -sCell.m_fCellX;

				S_HybridMapCell stMap;
				ZeroMemory(&stMap, sizeof(stMap));
				// LYS 2026-10-01 Hybrid Phase 4 (4-5) the transmitted cell ID (MakeCellID token + letter carry), not the recipe's per-segment
				//   m_chCellID (every segment starts again at A there). A-panel token, no B-panel offset - as before.
				if (bLong)
					_stprintf_s(stMap.szName, _countof(stMap.szName), _T("%s%02d"), (LPCTSTR)CRecipeEditMgr::CellIdLetterPrefix(nLetterBase + j), i + 1);	// MakeCellID rotate 1
				else
					_stprintf_s(stMap.szName, _countof(stMap.szName), _T("%s%02d"), (LPCTSTR)CRecipeEditMgr::CellIdLetterPrefix(nLetterBase + nStick - i - 1), j + 1);
				if (bManualCid && pFunc->ResolveCellToken(k, i, j, 0, strCidTok, FALSE) == 1 && strCidTok.IsEmpty() != true)	// LYS 2026-10-12 Hybrid Phase 6 manual A token
				{
					aryCidIdx.Add((int)m_aryHybridMapCell.GetSize());	// index this cell gets at the Add below
					aryCidTok.Add(strCidTok);
				}
				stMap.dSX = dCX - dW / 2;
				stMap.dEX = dCX + dW / 2;
				stMap.dSY = dCY - dH / 2;
				stMap.dEY = dCY + dH / 2;
				stMap.nCell = j;
				stMap.nStick = nCol;
				stMap.nSeg = k;
				m_aryHybridMapCell.Add(stMap);
				nCellSeg++;

				if (stMap.dSX < dBox[0]) dBox[0] = stMap.dSX;
				if (stMap.dSY < dBox[1]) dBox[1] = stMap.dSY;
				if (stMap.dEX > dBox[2]) dBox[2] = stMap.dEX;
				if (stMap.dEY > dBox[3]) dBox[3] = stMap.dEY;
			}
		}

		for (int n = 0; n < 4; n++)
			m_aryHybridSegBox.Add(dBox[n]);

		strLog.Format(_T("[HYBRID] Cell map segment %d/%d rot=%d base=%d stick=%d cell=%d drawn=%d contact=%.4f x[%.1f,%.1f] y[%.1f,%.1f]"),
			k + 1, nSegCnt, nRot, nBase, nStick, nCellPerStick, nCellSeg, dContact, dBox[0], dBox[2], dBox[1], dBox[3]);
		CLogWrapper::GetInstance()->KMLOG(OPER, strLog);

		nBase += nStick;
		nLetterBase += CRecipeEditMgr::SegCellIdLetterExtent(pFunc, k);	// LYS 2026-10-01 Hybrid Phase 4 (4-5) rotate 1 -> cells per stick, else sticks
	}
	m_nHybridMapSegCnt = nSegCnt;

	int nMaxCell = (m_pParentCellMeasDlg != nullptr) ? m_pParentCellMeasDlg->m_chartOrbit.m_nMaxCell : 0;
	int nTotal = (int)m_aryHybridMapCell.GetSize();
	strLog.Format(_T("[HYBRID] Cell map %d segments, %d cells, %d sticks, skipped %d (chart CELL_MAX %d)"), nSegCnt, nTotal, nBase, nSkip, nMaxCell);
	CLogWrapper::GetInstance()->KMLOG((nSkip > 0 || nTotal + 1 > nMaxCell) ? WARN : OPER, strLog);	// LYS 2026-10-12 Hybrid Phase 6 + 1 = the active-segment frame
	if (bManualCid && aryCidIdx.GetSize() > 0)	// LYS 2026-10-12 Hybrid Phase 6 (A Auto + B Manual: nothing collected, no log)
	{
		if (pFunc->IsRcpLoadBusy() == TRUE || pFunc->GetRcpLoadTry() != nCidLoadTry || pRcp->GetActiveSegment() != 0)
			CLogWrapper::GetInstance()->KMLOG(WARN, _T("[CELLID] Cell map manual names dropped - recipe reload / segment change during the build (Auto names shown)"));
		else
		{
			for (int n = 0; n < aryCidIdx.GetSize(); n++)
			{
				int nIdx = aryCidIdx[n];
				if (nIdx < 0 || nIdx >= m_aryHybridMapCell.GetSize())
					continue;
				S_HybridMapCell& stFix = m_aryHybridMapCell[nIdx];
				_tcsncpy_s(stFix.szName, _countof(stFix.szName), aryCidTok[n], _TRUNCATE);	// hand-edited token > 31 chars is cut (_stprintf_s would abort)
			}
			strLog.Format(_T("[CELLID] Cell map manual names %d of %d (panel A, %d segments)"), (int)aryCidIdx.GetSize(), nTotal, nSegCnt);
			CLogWrapper::GetInstance()->KMLOG(OPER, strLog);
		}
	}

	// same view call as the 1-segment tail of GenGlassPath (active segment = 0 here)
	int nRotView = pRcp->GetRecipeInfo()->m_sOptionItem.m_nGlsRotate;
	if (nRotView < 0 || nRotView > 3) nRotView = 0;
	RotateGlassPath(nRotView);
	m_nHybridPathSeg = 0;	// LYS 2026-10-12 Hybrid Phase 6 the PATH outline shows segment 0 now
	m_nHybridPathRot = nRotView;
	return true;
}

// LYS 2026-09-30 Hybrid Phase 6 1 s timer for a hybrid recipe: redraw when a cached slot changed or the framed segment changed.
//   Reads only the cache, nCellStatus / nBFCellStatus and the int active segment (A8: no segment vectors during a run).
//   Statuses of finished segments stay on screen because every segment owns its own nCellStatus columns.
void CDlgSubOperate::UpdateHybridCellStatus()
{
	VTM_SYSTEM* pShm = m_pAuroraMgr->GetShareMemoryMgrPtr()->GetSharedMemory();
	int nCnt = (int)m_aryHybridMapCell.GetSize();

	int nFrameSeg = -1;		// white frame = segment being measured, only while a cycle runs
	if (CSystemParamShare::GetInstance()->IsCycleState() == true)
		nFrameSeg = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetActiveSegment();
	if (nFrameSeg < 0 || nFrameSeg >= m_nHybridMapSegCnt || (nFrameSeg * 4 + 3) >= (int)m_aryHybridSegBox.GetSize())
		nFrameSeg = -1;
	else if (m_aryHybridSegBox[nFrameSeg * 4 + 0] > m_aryHybridSegBox[nFrameSeg * 4 + 2])
		nFrameSeg = -1;	// segment without drawn cells

	bool bChange = (nFrameSeg != m_nHybridMapFrameSeg);
	for (int i = 0; i < nCnt && bChange == false; i++)
	{
		const S_HybridMapCell& stMap = m_aryHybridMapCell[i];
		if (pShm->nCellStatus[stMap.nCell][stMap.nStick] != pShm->nBFCellStatus[stMap.nCell][stMap.nStick])
			bChange = true;
	}
	if (bChange == false)
		return;

	RECIPELAYOUT stRecipe;
	stRecipe.arrCellLayout.RemoveAll();
	CELLLAYOUT stCell;

	for (int i = 0; i < nCnt; i++)
	{
		const S_HybridMapCell& stMap = m_aryHybridMapCell[i];
		ZeroMemory(&stCell, sizeof(stCell));
		_tcscpy_s(stCell.szCellName, 32, stMap.szName);
		stCell.dSDx = stMap.dSX;
		stCell.dSDy = stMap.dSY;
		stCell.dEDx = stMap.dEX;
		stCell.dEDy = stMap.dEY;

		int iStatus = pShm->nCellStatus[stMap.nCell][stMap.nStick];
		pShm->nBFCellStatus[stMap.nCell][stMap.nStick] = iStatus;
		SetHybridCellLook(iStatus, stCell);
		stRecipe.arrCellLayout.Add(stCell);
	}

	if (nFrameSeg >= 0)
	{
		ZeroMemory(&stCell, sizeof(stCell));	// no name / status text, drawn after the cells
		stCell.dSDx = m_aryHybridSegBox[nFrameSeg * 4 + 0] - 8.0;
		stCell.dSDy = m_aryHybridSegBox[nFrameSeg * 4 + 1] - 8.0;
		stCell.dEDx = m_aryHybridSegBox[nFrameSeg * 4 + 2] + 8.0;
		stCell.dEDy = m_aryHybridSegBox[nFrameSeg * 4 + 3] + 8.0;
		stCell.color = CLR_WHITE;
		if (nCnt + 1 > m_pParentCellMeasDlg->m_chartOrbit.m_nMaxCell)	// LYS 2026-10-12 Hybrid Phase 6 chart cannot hold every cell + frame: frame first, the last cells drop
			stRecipe.arrCellLayout.InsertAt(0, stCell);
		else
			stRecipe.arrCellLayout.Add(stCell);
	}

	stRecipe.m_iVersion = 1;
	stRecipe.m_iCellCnt = (int)stRecipe.arrCellLayout.GetSize();
	m_pParentCellMeasDlg->m_chartOrbit.m_nDrawLimit = -1;	// LYS 2026-10-12 Hybrid Phase 6 every cached cell (chart capacity, D6-6)
	UpdateMeasCell(&stRecipe);
	m_pParentCellMeasDlg->m_chartOrbit.m_nDrawLimit = m_nCellDrawLegacy;	// the 1-segment draw keeps its limit
	m_nHybridMapFrameSeg = nFrameSeg;
}

// LYS 2026-09-30 Hybrid Phase 6 status colour / text for one cell - byte copy of the UpdateCellStatus switch
//   (the 1-segment path keeps its own copy untouched), plus the NotCON text copy the original case misses.
void CDlgSubOperate::SetHybridCellLook(int iStatus, CELLLAYOUT& stCell)
{
	CString strText;
	switch (iStatus)
	{
	// Idle
	case df_CELLSTATE_ILDE:
		stCell.color = CLR_GRAY;
		strText.Format(_T("IDLE"));
		_tcscpy_s(stCell.szCellText, 32, strText);
		break;
	//	PTP
	case df_CELLSTATE_PTP_NOR:	//	Normal
		stCell.color = CLR_YELLOW;
		strText.Format(_T("PTP Normal"));
		_tcscpy_s(stCell.szCellText, 32, strText);
		break;
	case df_CELLSTATE_PTP_SHORT:	// Short
		stCell.color = CLR_RED;
		strText.Format(_T("PTP SHORT"));
		_tcscpy_s(stCell.szCellText, 32, strText);
		break;
	case df_CELLSTATE_PTP_OPEN:	// Open
		stCell.color = CLR_RED;
		strText.Format(_T("PTP OPEN"));
		_tcscpy_s(stCell.szCellText, 32, strText);
		break;
	//	OST
	case df_CELLSTATE_OST_NOR:	//	Normal
		stCell.color = CLR_YELLOW;
		strText.Format(_T("OST Normal"));
		_tcscpy_s(stCell.szCellText, 32, strText);
		break;
	case df_CELLSTATE_OST_SHORT:	// Short
		stCell.color = CLR_RED;
		strText.Format(_T("OST SHORT"));
		_tcscpy_s(stCell.szCellText, 32, strText);
		break;
	case df_CELLSTATE_OST_OPEN:	// Open
		stCell.color = CLR_RED;
		strText.Format(_T("OST OPEN"));
		_tcscpy_s(stCell.szCellText, 32, strText);
		break;
	// R-TEST
	case df_CELLSTATE_PASS:		// PASS
		stCell.color = CLR_YELLOW;
		strText.Format(_T("PASS"));
		_tcscpy_s(stCell.szCellText, 32, strText);
		break;
	case df_CELLSTATE_OVER:		// Over
		stCell.color = CLR_RED;
		strText.Format(_T("Over"));
		_tcscpy_s(stCell.szCellText, 32, strText);
		break;
	case df_CELLSTATE_UNDER:		// UNDER
		stCell.color = CLR_RED;
		strText.Format(_T("Under"));
		_tcscpy_s(stCell.szCellText, 32, strText);
		break;
	case df_CELLSTATE_PG_NOTCONNACT:		// PG 미연결
		stCell.color = CLR_RED;
		strText.Format(_T("NotCON"));
		_tcscpy_s(stCell.szCellText, 32, strText);	// LYS 2026-09-30 missing in the 1-segment copy
		break;
	case df_CELLSTATE_RTEST_FAIL:		// R-TEST FAIL
		stCell.color = CLR_RED;
		strText.Format(_T("FAIL"));
		_tcscpy_s(stCell.szCellText, 32, strText);
		break;
	// AGING
	case df_CELLSTATE_AGING_START:
		stCell.color = CLR_LIGHT_YELLOW;
		strText.Format(_T("AGING S"));
		_tcscpy_s(stCell.szCellText, 32, strText);
		break;
	case df_CELLSTATE_AGING_NG:		// NG
		stCell.color = CLR_RED;
		strText.Format(_T("AGING NG"));
		_tcscpy_s(stCell.szCellText, 32, strText);
		break;
	case df_CELLSTATE_AGING_OK:		// OK
		stCell.color = CLR_LIGHT_GRAY;
		strText.Format(_T("AGING END"));
		_tcscpy_s(stCell.szCellText, 32, strText);
		break;

	// Meas
	case df_CELLSTATE_WAD_START:		// 특성측정시작
		stCell.color = CLR_YELLOW;
		strText.Format(_T("WAD S"));
		_tcscpy_s(stCell.szCellText, 32, strText);
		break;
	case df_CELLSTATE_WAD_END:		// 특성측정완료
		stCell.color = COLORREF(m_lMeasDoneColorWAD); // RGB_TEST_1;
		strText.Format(_T("WAD END"));
		_tcscpy_s(stCell.szCellText, 32, strText);
		break;
	case df_CELLSTATE_ANGLE_START:		// 측정시작
		stCell.color = CLR_YELLOW;
		strText.Format(_T("시야각 S"));
		_tcscpy_s(stCell.szCellText, 32, strText);
		break;
	case df_CELLSTATE_ANGLE_END:		// 측정완료
		stCell.color = COLORREF(m_lMeasDoneColorAngle); //RGB_TEST_2;
		strText.Format(_T("시야각 END"));
		_tcscpy_s(stCell.szCellText, 32, strText);
		break;
	case df_CELLSTATE_VISION_START:		// 화질측정시작
		stCell.color = CLR_YELLOW;
		strText.Format(_T("화질 S"));
		_tcscpy_s(stCell.szCellText, 32, strText);
		break;
	case df_CELLSTATE_VISION_END:		// 화질측정완료
		stCell.color = COLORREF(m_lMeasDoneColorVision); //RGB_TEST_3;
		strText.Format(_T("화질 END"));
		_tcscpy_s(stCell.szCellText, 32, strText);
		break;
	case df_CELLSTATE_COLOR_START:		// 측정시작
		stCell.color = CLR_YELLOW;
		strText.Format(_T("면계측 S"));
		_tcscpy_s(stCell.szCellText, 32, strText);
		break;
	case df_CELLSTATE_COLOR_END:		// 측정완료
		stCell.color = COLORREF(m_lMeasDoneColorColor); //RGB_TEST_4;
		strText.Format(_T("면계측 END"));
		_tcscpy_s(stCell.szCellText, 32, strText);
		break;
	default:
		stCell.color = CLR_BLUE;
		strText.Format(_T("NOTDEFINED"));
		_tcscpy_s(stCell.szCellText, 32, strText);
		break;
	}
}

void CDlgSubOperate::UpdateAgingMode(bool bStart, int nTime)
{
	if (bStart == true)
	{
		m_pSystemInitializingDlg->InitStart(2);
		m_pSystemInitializingDlg->SetAlignTime(nTime);
	}
	else
	{
		m_pSystemInitializingDlg->InitStop();
	}
	// jsyoon 2022.06.10 나중에 표시
	//if (bStart == true)
	//{
	//	int nCell = 6;
	//	//nCell = m_aryCellGroup.GetSize();
	//	nCell = m_pAuroraMgr->GetRecipeEditMgrPtr()->GetRecipeMgrPtr()->m_nCellCnt;
	//	m_pAgingProgressDlg->InitStart(nCell);
	//}
	//else
	//{
	//	m_pAgingProgressDlg->InitStop();
	//}
}

void CDlgSubOperate::UpdateSACCMode(bool bStart)
{
	if (bStart == true)
	{
		m_pSystemInitializingDlg->InitStart(1);
	}
	else
	{
		m_pSystemInitializingDlg->InitStop();
	}
}

void CDlgSubOperate::OnBnClickedButtonSafeytPlc()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	HWND handle = NULL;
	CString m_sCommLogFileName = _T("");
	m_sCommLogFileName = _T("C:\\HBS\\SafetyPLC\\SafetyPLCHistory.exe");
	ShellExecute(handle, _T("open"), m_sCommLogFileName, NULL, NULL, SW_SHOWNORMAL);
}


void CDlgSubOperate::OnBnClickedButtonSMarkInfo()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CDlgSMarkUI dlg;
	dlg.DoModal();
}

// LYS 2026-07-20 CIM Alarm not-report toggle handler
void CDlgSubOperate::OnBnClickedCheckCimReport()
{
	bool bChecked = (m_btnCimAlarmNotReport.GetCheck() == BST_CHECKED);
	m_pAuroraMgr->GetSystemConfigMgrPtr()->SetUseCIMAlarmNotReport(bChecked);
	UpdateCimReportButtonColor(bChecked);
}

// LYS 2026-07-20 Update CIM report button face color and text
void CDlgSubOperate::UpdateCimReportButtonColor(bool bChecked)
{
	if (bChecked)
	{
		m_btnCimAlarmNotReport.SetFaceColor(RGB(255, 100, 100), TRUE);
		m_btnCimAlarmNotReport.SetTextColor(RGB(255, 100, 100));
		m_btnCimAlarmNotReport.SetWindowText(L"CIM Alarm\n\uBBF8\uBCF4\uACE0");
	}
	else
	{
		m_btnCimAlarmNotReport.SetFaceColor(GetSysColor(COLOR_BTNFACE), TRUE);
		m_btnCimAlarmNotReport.SetTextColor(RGB(0, 0, 0));
		m_btnCimAlarmNotReport.SetWindowText(L"CIM Alarm\n\uBCF4\uACE0");
	}
}

void CDlgSubOperate::WriteColorConfig()
{
	CString strValue;

	strValue.Format(_T("%d"), m_lMeasDoneColorWAD);
	m_pIniHelper->Link(true, _T("COLOR"), _T("WAD"), &strValue, (CString)_T("1956630"));

	strValue.Format(_T("%d"), m_lMeasDoneColorVision);
	m_pIniHelper->Link(true, _T("COLOR"), _T("VISION"), &strValue, (CString)_T("1956630"));

	strValue.Format(_T("%d"), m_lMeasDoneColorAngle);
	m_pIniHelper->Link(true, _T("COLOR"), _T("ANGLE"), &strValue, (CString)_T("1956630"));

	strValue.Format(_T("%d"), m_lMeasDoneColorColor);
	m_pIniHelper->Link(true, _T("COLOR"), _T("COLOR"), &strValue, (CString)_T("1956630"));
}

void CDlgSubOperate::UpdateMeasColor()
{
	CD_SelectColor dlg;
	dlg.m_colorWAD    = m_lMeasDoneColorWAD;
	dlg.m_colorVision = m_lMeasDoneColorVision;
	dlg.m_colorAngle  = m_lMeasDoneColorAngle;
	dlg.m_colorColor  = m_lMeasDoneColorColor;

	if (dlg.DoModal() == IDOK)
	{
		m_lMeasDoneColorWAD = dlg.m_colorWAD;
		m_lMeasDoneColorVision = dlg.m_colorVision;
		m_lMeasDoneColorAngle = dlg.m_colorAngle;
		m_lMeasDoneColorColor = dlg.m_colorColor;

		WriteColorConfig();
	}
}

UINT CDlgSubOperate::ThreadInspectEnd(LPVOID p1)
{
	CDlgSubOperate* pThis = (CDlgSubOperate*)p1;

	if (pThis->m_bThreadRun != true)
	{
		pThis->ThreadRun();
	}
	
	return 0;
}

UINT CDlgSubOperate::ThreadRun()
{
	int nRet = VTM_RV_SUCCESSED;

	m_bThreadRun = true;

	nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectEnd(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_4);

	nRet = m_pAuroraMgr->GetCommTcpIpFactoryMgrPtr()->DoWorkInspectEnd(enumTCPIpFactorytID_AGING);

	m_bThreadRun = false;

	return 0;
}

void CDlgSubOperate::UpdateCIMAlarm(int iAlarm)
{
	CAlarmController::GetInstance()->SetAlarm(iAlarm);
}

void CDlgSubOperate::OnDestroy()
{
	CMainSubDlgWin::OnDestroy();

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	// 2023.03.06 add
	KillTimer(MAIN_STATE_TIMER_ID);
	KillTimer(MAIN_STATE_TIMER_ID + 1);
	KillTimer(MAIN_STATE_TIMER_ID + 2);
	KillTimer(MAIN_LAMP_OVER_TIMER_ID);
}

// LYS 2026-10-12 Hybrid Phase 6 operate-screen indicator (D6-1/2/3/12): segment / EFA pass / journal replay text in the cell chart's top band,
//   plus the PATH outline of the measured segment. 1 s timer, ints only: the measure-loop progress CRecipeEditMgr publishes (GetUi*),
//   the journal replay counters and the UI caches BuildHybridCellMap filled (names, rotates). 1-segment recipe (m_nHybridMapSegCnt 0):
//   no text, label never shown, PATH view untouched (D6-3 (a)).
void CDlgSubOperate::UpdateHybridIndicator()
{
	if (m_labelHybridInfo.GetSafeHwnd() == NULL)
		return;
	CString strText;
	if (m_nHybridMapSegCnt > 1)
	{
		CRecipeEditMgr* pRcp = m_pAuroraMgr->GetRecipeEditMgrPtr();
		int nPass = pRcp->GetUiPass(), nPassCnt = pRcp->GetUiPassCount(), nSeg = pRcp->GetUiSeg(), nReady = pRcp->GetUiSegReady();	// pass first (SetUiProgress writes it last), seg before ready (the setter clears ready before it changes seg)
		int nDone = 0, nTotal = 0;
		CResumeJournal::GetInstance()->GetReplayProgress(nDone, nTotal);
		CString strPart;
		if (nPass >= 0 && nSeg >= 0 && nSeg < m_nHybridMapSegCnt && nSeg < m_aryHybridSegName.GetSize())
			strPart.Format(_T("세그먼트 %d/%d [%s]%s"), nSeg + 1, m_nHybridMapSegCnt, (LPCTSTR)m_aryHybridSegName[nSeg].Left(12), (nReady == 0) ? _T(" 전환 중") : _T(""));	// 12: centred single line, a longer text loses both ends
		else
			strPart.Format(_T("하이브리드 레시피 %d 세그먼트"), m_nHybridMapSegCnt);
		strText = strPart;
		if (nPassCnt > 0 && nPass >= 0)
		{
			strPart.Format(_T("   |   EFA T%d (%d/%d)"), nPass, nPass + 1, nPassCnt);
			strText += strPart;
		}
		if (nTotal > 0)
		{
			strPart.Format(_T("   |   재개 결과 복원 %d/%d"), nDone, nTotal);
			strText += strPart;
		}
		SyncHybridPathView(nPass, nSeg, nReady);	// (3-4)
	}
	if (strText == m_strHybridInfo)
		return;		// unchanged: no repaint
	m_strHybridInfo = strText;
	if (strText.IsEmpty())
		m_labelHybridInfo.ShowWindow(SW_HIDE);
	else
	{
		m_labelHybridInfo.SetText(strText);
		m_labelHybridInfo.ShowWindow(SW_SHOWNA);
	}
}

// LYS 2026-10-12 Hybrid Phase 6 PATH chart outline = the segment being measured once its boundary is done (D6-5 (a)): the outline turns after
//   the stage turned and re-aligned; outside the measure loop it shows segment 0 (= the GenGlassPath view, where the next glass starts).
//   The loader rectangle keeps showing the live R (UpdateCurCoord, 600 ms). Hybrid only, one OPER line per change.
void CDlgSubOperate::SyncHybridPathView(int nPass, int nSeg, int nReady)
{
	int nSegCnt = (int)m_aryHybridSegRot.GetSize();
	if (m_nHybridMapSegCnt <= 1 || nSegCnt != m_nHybridMapSegCnt || m_nHybridPathSeg < 0)
		return;
	int nTarget = m_nHybridPathSeg;
	if (nPass < 0)
		nTarget = 0;
	else if (nSeg >= 0 && nReady == 1)
		nTarget = nSeg;
	if (nTarget < 0 || nTarget >= nSegCnt || nTarget == m_nHybridPathSeg)
		return;
	int nRot = m_aryHybridSegRot[nTarget];
	if (nRot != m_nHybridPathRot)
	{
		RotateGlassPath(nRot);	// outline, edge cut, probe card height, "Glass Size" text (the cell chart outline is fixed)
		UpdateCurCoord();		// SetCardSize put ACX / P5 back at x 0..210 - redraw them at the live positions now
		m_nHybridPathRot = nRot;
	}
	m_nHybridPathSeg = nTarget;
	CString strLog;
	strLog.Format(_T("[HYBRID] Path view segment %d/%d [%s] rot=%d"), nTarget + 1, nSegCnt, (nTarget < m_aryHybridSegName.GetSize()) ? (LPCTSTR)m_aryHybridSegName[nTarget] : _T(""), nRot);
	CLogWrapper::GetInstance()->KMLOG(OPER, strLog);
}

// LYS 2026-10-12 Hybrid Phase 6 1-segment recipe with a manual Cell ID config: panel-A tokens keyed by the status slot the 1-segment chart
//   draws ([cell][MakeCellID stick]: rotate 1 direct, else NS-1-i). Vectors are read only here (GenGlassPath, UI thread, segment 0).
//   Auto/Auto (IsCellIdDefault), A Auto + B Manual (no panel-A token), >= 2 segments or a load in progress: cache empty, nothing logged
//   -> UpdateCellStatus unchanged.
void CDlgSubOperate::BuildManualCellNameCache()
{
	m_aryManualCellName.RemoveAll();
	CRecipeEditMgr* pRcp = m_pAuroraMgr->GetRecipeEditMgrPtr();
	C_OutRecipeFunc* pFunc = (pRcp != nullptr) ? pRcp->GetRecipeFuncPtr() : nullptr;
	if (pFunc == nullptr)
		return;
	int nLoadTry = pFunc->GetRcpLoadTry();	// try count first, then the busy count (OutRecipeFunc.h m_nRcpLoadTry, as GetManualCellToken)
	if (pFunc->IsRcpLoadBusy() == TRUE || pFunc->GetSegmentLoopCount() > 1 || pFunc->IsCellIdDefault() == TRUE)
		return;
	S_RecipeMgr* pSeg = pFunc->GetSegmentMgr(0);	// 1 segment: the working slot (m_recipeMgr)
	if (pSeg == nullptr)
		return;
	int nStick = (int)pSeg->m_sData.m_vecStick.size();
	int nRot = pSeg->m_sInfo.m_sOptionItem.m_nGlsRotate;
	CStringArray aryName;
	aryName.SetSize(100 * 50);		// nCellStatus[100][50]
	int nSet = 0, nCellTotal = 0;
	CString strTok;
	for (int i = 0; i < nStick; i++)
	{
		int nCol = (nRot == 1) ? i : (nStick - i - 1);	// UpdateCellStatus nStick (MakeCellID stick slot)
		int nCells = (int)pSeg->m_sData.m_vecStick[i].m_vecCell.size();
		for (int j = 0; j < nCells; j++)
		{
			nCellTotal++;
			if (j >= 100 || nCol < 0 || nCol >= 50)
				continue;
			if (pFunc->ResolveCellToken(0, i, j, 0, strTok, FALSE) == 1 && strTok.IsEmpty() != true)
			{
				aryName[j * 50 + nCol] = strTok;
				nSet++;
			}
		}
	}
	// LYS 2026-10-12 Hybrid Phase 6 fix (review correctness 1): nothing set is tested before the reload test - a 1-segment A Auto + B Manual recipe logs nothing, not even the WARN
	if (nSet <= 0)
		return;		// A Auto + B Manual: panel A is Auto -> nothing set, legacy names, no log
	if (pFunc->IsRcpLoadBusy() == TRUE || pFunc->GetRcpLoadTry() != nLoadTry)
	{
		CLogWrapper::GetInstance()->KMLOG(WARN, _T("[CELLID] Cell map manual names dropped - recipe reload during the build (Auto names shown)"));
		return;
	}
	m_aryManualCellName.Copy(aryName);
	CString strLog;
	strLog.Format(_T("[CELLID] Cell map manual names %d of %d (panel A, 1 segment, rot=%d)"), nSet, nCellTotal, nRot);
	CLogWrapper::GetInstance()->KMLOG(OPER, strLog);
}
