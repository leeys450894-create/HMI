#pragma once
#include <vector>
using namespace std;

#define WM_RECIPE_EVENT		WM_USER + 101

////
//Make Cell
#define M_MAKE_CELL_SET			1

//Align
#define M_ALIGN_SHOW_REFIMG		2
#define M_ALIGN_PATTERN_SEARCH  3
#define M_ALIGN_CCD_SET		    4
#define M_ALIGN_GLASS_GET_POS	5
#define M_ALIGN_GLASS_MOVE		6
#define M_ALIGN_PROBE_GET_POS	7
#define M_ALIGN_PROBE_MOVE		8
#define M_ALIGN_GLASS_PROC		9
#define M_ALIGN_PROBE_PROC		10
#define M_ALIGN_RESET			11

#define M_RCP_LAMP_SET			12
#define M_RCP_CONTACT_SET		13

#define M_RCP_STAGE_GET_POS		14
#define M_RCP_STAGE_MOVE		15
#define M_RCP_PROBE_MOVE		16
#define M_RCP_HEAD_GET_POS		17
#define M_RCP_HEAD_MOVE			18

#define M_RCP_SRM_LAMP_SET		19
#define M_RCP_TEST_MODE			20	// LYS 2026-07-02 TestMode dialog
////

#define RCP_CELL_ANG_0		0
#define RCP_CELL_ANG_90	    1
// LYS 2026-08-18 Glass rotate 180 (단변 측정, m_nGlsRotate=2).
// 2026-08-07 회전 체계와 함께 추가. 기존 0/90 분기에 2가 없어 대입이 통째로 스킵되고
// 이전 값이 그대로 남아 있었다.
#define RCP_CELL_ANG_180	2

#define PIX_UNIFORMITY		1
#define PIX_NON_UNIFORMITY  2
#define PIX_COORD_ITEM		3

#define RCP_HEAD_LEFT	0
#define RCP_HEAD_RIGHT	1

#define RCP_MODE_REVIEW		0
#define RCP_MODE_SPEC		1
#define RCP_MODE_QUALITY	2
#define RCP_MODE_COLOR		3
#define RCP_MODE_ANGLE		4

#define RCP_GLASS_SIZE_X	1500.0
#define RCP_GLASS_SIZE_Y	925.0

#define CH_LEN				1024

#define ALG_FRONT_LEFT	0
#define ALG_FRONT_RIGHT	1
#define ALG_BACK_LEFT	2
#define ALG_BACK_RIGHT	3

#define ALG_FRONT	5
#define ALG_BACK	6

#define STATIC_CLASS					_T("Static")
#define BUTTON_CLASS					_T("Button")
#define LIST_CLASS						_T("SysListView32")
#define EDIT_CLASS						_T("Edit")
#define GRID_CLASS						_T("MFCGridCtrl")
#define COMBOBOX_CLASS					_T("ComboBox")
#define SCROLLBAR_CLASS					_T("ScrollBar")
#define MFCEDITBROWSE_CLASS				_T("MFCEditBrowse")

const enum CAD_ROW {
	CAD_ROW_A,		CAD_ROW_B,		CAD_ROW_C,		CAD_ROW_D,		CAD_ROW_E,
	CAD_ROW_F,		CAD_ROW_G,		CAD_ROW_H,		CAD_ROW_I,		CAD_ROW_J,
	CAD_ROW_K,		CAD_ROW_L,		CAD_ROW_M,		CAD_ROW_N,		CAD_ROW_O,
	CAD_ROW_P,		CAD_ROW_Q,		CAD_ROW_R,		CAD_ROW_S,		CAD_ROW_T,
	CAD_ROW_U,		CAD_ROW_V,		CAD_ROW_W,		CAD_ROW_X,		CAD_ROW_Y,
	CAD_ROW_X1,		CAD_ROW_X2,		CAD_ROW_X3,		CAD_ROW_X4,		CAD_ROW_X5,
	CAD_ROW_X6,		CAD_ROW_X7,		CAD_ROW_P1_1,	CAD_ROW_P1_2,	CAD_ROW_P2_1,
	CAD_ROW_P2_2,	CAD_ROW_P3_1,	CAD_ROW_P3_2,	CAD_ROW_P4_1,	CAD_ROW_P4_2,
	CAD_ROW_P5_1,	CAD_ROW_P5_2,	CAD_ROW_END,
};

const enum RCP_VERSION{
	RECIPE_FILE_V1000	= 1000,
	RECIPE_FILE_V1001,
	RECIPE_FILE_V1002,
	RECIPE_FILE_V1003,
	RECIPE_FILE_V1004,
	RECIPE_FILE_V1005,		// LYS 2026-07-08 Add
	RECIPE_FILE_V1006,		// LYS 2026-08-06 Add
	RECIPE_FILE_V1007,		// LYS 2026-08-19 Add
	RECIPE_FILE_END,
};

///////////////////////레시피 구조체
#define M_RCP_PROBE_MAX 5	//메인 프로브 ID 개수
#define M_RCP_INS_ITEM	10	//레시피 검사 항목 수

//검사 항목 구조체
struct S_InspectItem {
	BOOL m_bUsePre_0;	    //사전검사
	BOOL m_bUseElect_1;		//전기검사
	BOOL m_bUseRTest;	    //R-TEST
	BOOL m_bUseAgingTest_2;	//Aging Test
	BOOL m_bUseSpec_3;		//특성검사
	BOOL m_bUseQuality_4; 	//화질검사
	BOOL m_bUseColor_5;		//색이상검사
	BOOL m_bUseAngle_6;		//시야각검사
	BOOL m_bUseTSP_7;		//TSP	//전수
	BOOL m_bUseLCR_8;		//LCR	//Cell 고정

	// LYS 2026-07-14 추가 기능
	BOOL m_bUseEFA;
	BOOL m_bUseSpec_Multi;

	// LYS 2026-08-06 강화 검사
	BOOL m_bUseAging_Test;

	int  m_nTimeOut[M_RCP_INS_ITEM];

	S_InspectItem () {
		Clear ();
	}

	void Clear () {
		m_bUsePre_0		  = FALSE;
		m_bUseElect_1	  = FALSE;
		m_bUseRTest		  = FALSE;
		m_bUseAgingTest_2 = FALSE;
		m_bUseSpec_3      = FALSE;
		m_bUseQuality_4   = FALSE;
		m_bUseColor_5     = FALSE;
		m_bUseAngle_6     = FALSE;
		m_bUseTSP_7	      = FALSE;
		m_bUseLCR_8		  = FALSE;
		m_bUseEFA		  = FALSE;
		m_bUseSpec_Multi  = FALSE;
		m_bUseAging_Test  = FALSE;

		ZeroMemory (&m_nTimeOut, sizeof (m_nTimeOut));
	}
};

//검사 옵션 구조체
struct S_OptionItem {
	BOOL 	m_bUseRtestNgPass;	//R-Test NG 무시
	BOOL 	m_bUseSampling;		//Sampling 검사
	BOOL 	m_bUseAlign;		//Glass Align 사용 유무
	BOOL 	m_bUseReviewSnap;	//Contact 위치 리뷰 이미지 저장
	int 	m_nGlsRotate;		//Glass Rotate 0(0), 1(90), 2(180), 3(270)
	//int 	m_nFilter;			//0,1 (사용 하는지 확인)
	double  m_fFrontHeadGap;	//Front Head 간격
	double  m_fBackHeadGap;		//Back Head 간격
	wchar_t	m_chAlignName[CH_LEN];//Align Recipe Name


	S_OptionItem () {
		Clear ();
	}

	void Clear () {
		m_bUseRtestNgPass	= FALSE;
		m_bUseSampling		= FALSE;
		m_bUseAlign			= FALSE;
		m_bUseReviewSnap	= FALSE;
		m_nGlsRotate 		= 1;
		//m_nFilter 			= -1;
		m_fFrontHeadGap 	= 0.0;
		m_fBackHeadGap  	= 0.0;
		ZeroMemory (&m_chAlignName, sizeof (m_chAlignName));
	}
};

//조명 구조체
struct S_LampItem {
	int	m_nFrontLeft;
	int	m_nFrontRight;
	int	m_nBackLeft;
	int	m_nBackRight;

	int m_nSRMLamp[4];

	S_LampItem () {
		Clear ();
	}

	void Clear () {
		m_nFrontLeft  = 0;
		m_nFrontRight = 0;
		m_nBackLeft   = 0;
		m_nBackRight  = 0;

		for (size_t i = 0; i < 4; i++)
		{
			m_nSRMLamp[i] = 0;
		}
	}
};
 
//프로브 컨택 구조체
struct S_ContactPos {
	double m_fReadyPos;
	double m_fPreContactPos;
	double m_fEndContactPos;
	double m_fAddOD;
	
	S_ContactPos () {
		Clear ();
	}

	void Clear () {
		m_fReadyPos  	 = 0.0;
		m_fPreContactPos = 0.0;
		m_fEndContactPos = 0.0;
		m_fAddOD 		 = 0.0;
	}
}; 

//모션 Offset 구조체	//Probe 고정이 아니라서 Offset 어찌될지 확인 필요
struct S_MotionOffsetPos {
	double m_fStageY;			//Stage Offset
	double m_fStageRotate;
	
	double m_fHeadReviewY;		//Contact Review Offset
	double m_fHeadReviewLeft;
	double m_fHeadReviewRight;
	
	int	   m_nQualityRotate;	//Quality Rotate Offset //0(0도) 1(90도)
	double m_fQualityLeftRot;
	double m_fQualityRightRot;
	double m_fProbeShift;
	
	S_MotionOffsetPos () {
		Clear ();
	}

	void Clear () {
		m_fStageY  	 		= 0.0;
		m_fStageRotate 		= 0.0;
		
		m_fHeadReviewY 		= 0.0;
		m_fHeadReviewLeft 	= 0.0;
		m_fHeadReviewLeft   = 0.0;
		
		m_nQualityRotate 	= 1;
		m_fQualityLeftRot 	= 0.0;
		m_fQualityRightRot 	= 0.0;
		m_fProbeShift		= 0.0;
	}
}; 

// LYS 2026-07-07 Sub Z축 구조체
struct S_MotionSubZ {
	double m_fWadSubzRight;
	double m_fWadSubzLeft;

	double m_fVisionSubzRight;
	double m_fVisionSubzLeft;

	//double m_fAngleSubzRight;
	//double m_fAngleSubzLeft;

	S_MotionSubZ() {
		Clear();
	}

	void Clear() {
		m_fWadSubzRight = 0.0;
		m_fWadSubzLeft = 0.0;

		m_fVisionSubzRight = 0.0;
		m_fVisionSubzLeft = 0.0;

		//m_fAngleSubzRight = 0.0;
		//m_fAngleSubzLeft = 0.0;
	}
};

// LYS 2026-08-19 Angle axis recipe parameters
struct S_MotionAngle {
	double m_fAngleScheimpflug1;
	double m_fAngleRotate1;
	double m_fAngleZoom1;
	double m_fAngleTilt1;
	double m_fAngleAzimuth1;

	double m_fAngleScheimpflug2;
	double m_fAngleRotate2;
	double m_fAngleZoom2;
	double m_fAngleTilt2;
	double m_fAngleAzimuth2;

	S_MotionAngle() {
		Clear();
	}
	void Clear() {
		m_fAngleScheimpflug1 = 0.0;
		m_fAngleRotate1 = 0.0;
		m_fAngleZoom1 = 0.0;
		m_fAngleTilt1 = 0.0;
		m_fAngleAzimuth1 = 0.0;

		m_fAngleScheimpflug2 = 0.0;
		m_fAngleRotate2 = 0.0;
		m_fAngleZoom2 = 0.0;
		m_fAngleTilt2 = 0.0;
		m_fAngleAzimuth2 = 0.0;
	}
};

//프로브 정보 구조체
struct S_ProbeInfo {
	double	m_fProbeW;
	double	m_fProbeH;
	int		m_nProbePriority;  	//프로브 우선순위
	wchar_t	m_chProbeFirst[M_RCP_PROBE_MAX][CH_LEN];
	wchar_t	m_chProbeSecond[M_RCP_PROBE_MAX][CH_LEN];
	
	S_ProbeInfo () {
		Clear ();
	}

	void Clear () {
		m_fProbeW = 0.0;
		m_fProbeH = 0.0;
		m_nProbePriority = 0;
		ZeroMemory (&m_chProbeFirst,  sizeof (m_chProbeFirst));
		ZeroMemory (&m_chProbeSecond, sizeof (m_chProbeFirst));
	}
};

/////////////////////////////////////////////////
#define M_CELL_INIT  0	//초기값
#define M_CELL_PASS  1	//검사 안함
#define M_CELL_READY 2  //검사Cell 은 2로 만들어줌
#define M_CELL_OK 	 3
#define M_CELL_NG 	 4

#define M_CELL_NON_SEL	0
#define M_CELL_SEL		1

#define M_SPEC		 0
#define M_QUALITY	 1
#define M_COLOR		 2
#define M_ANGLE		 3
#define M_MODE_MAX   4

//Cell 정보 구조체
struct S_CellInfo {
	int		m_nArea;
	wchar_t	m_chCellID[CH_LEN];
	double	m_fCellY;
	double	m_fCellX;
	int		m_nCurStatus[M_MODE_MAX];	// M_SPEC / M_QUALITY / M_COLOR / M_ANGLE
	int		m_nSelectCell[M_MODE_MAX]; //

	S_CellInfo () {
		Clear ();
	}

	void Clear () {
		m_nArea		= 0;
		m_fCellY	= 0.0;
		m_fCellX	= 0.0;
		ZeroMemory (&m_nCurStatus,  sizeof (m_nCurStatus));
		ZeroMemory (&m_chCellID, sizeof (m_chCellID));
		ZeroMemory (&m_nSelectCell, sizeof (m_nSelectCell));
	}
};

//Stick 정보 구조체
struct S_StickInfo {
	int		m_nStickNum;
	double	m_fStageY;
	double	m_fStageR;
	
	vector <S_CellInfo>	m_vecCell; //Cell 정보

	S_StickInfo () {
		Clear ();
	}

	void Clear () {
		m_nStickNum = 0;
		m_fStageY	= 0.0;
		m_fStageR	= 0.0;
		
		if (!m_vecCell.empty())  m_vecCell.clear ();
	}
};

//Head Z축 정보
struct S_HeadZ {
	double m_fReviewZ;
	double m_fSpecZ;
	double m_fQualityZ;
	double m_fColorZ;
	double m_fAngleZ;
	double m_fZoom;
	double m_fFocus;

	S_HeadZ () {
		Clear ();
	}

	void Clear () {
		m_fReviewZ	= 0.0;
		m_fSpecZ	= 0.0;
		m_fQualityZ	= 0.0;
		m_fColorZ	= 0.0;
		m_fAngleZ	= 0.0;
		m_fZoom		= 0.0;
		m_fFocus	= 0.0;
	}
};

struct S_Point {
	double	x;
	double	y;

	S_Point () {
		Clear ();
	}

	void Clear () {
		x	= 0.0;
		y	= 0.0;
	}
};

//Cell Map 정보
struct S_RecipeMap {
	wchar_t	m_chModelInfoName[CH_LEN];
	S_Point	m_fGlobalMark;		//Glass Key 위치
	S_Point	m_fCellCenter;		//Cell 중심 좌표
	S_Point	m_fCellCount;		//Cell 가로 세로 개수
	S_Point	m_fCellDistance;	//Cell 간격
	S_Point	m_fCellSize;		//Cell 크기
	//S_Point	m_fCellOffset;		//만들어진 Cell
	S_Point m_fAlignKey;		//Align Recipe로 보내는 정보

	double	m_fContactOffset;
	//double	m_fReviewOffset;

	double m_fCadData[CAD_ROW_P1_1];
	wchar_t m_chCadData[CAD_ROW_END - CAD_ROW_P1_1][CH_LEN];

	wchar_t m_chCellCount[CH_LEN];		//(,) 포함한 문자열
	wchar_t m_chCellDistance[CH_LEN];   //(,) 포함한 문자열

	S_RecipeMap () {
		Clear ();
	}

	void Clear () {
		m_fGlobalMark.Clear ();
		m_fCellCenter.Clear ();
		m_fCellCount.Clear ();
		m_fCellDistance.Clear ();
		m_fCellSize.Clear ();
		//m_fCellOffset.Clear ();
		m_fAlignKey.Clear ();
		
		m_fContactOffset = 0.0;
		//m_fReviewOffset = 0.0;
		ZeroMemory (&m_chModelInfoName, sizeof (m_chModelInfoName));

		ZeroMemory (&m_fCadData, sizeof (m_fCadData));
		ZeroMemory (&m_chCadData, sizeof (m_chCadData));

		ZeroMemory (&m_chCellCount, sizeof(m_chCellCount));
		ZeroMemory (&m_chCellDistance, sizeof(m_chCellDistance));
	}
};

//Glass Align Info
struct S_GlsAlign {
	double	m_fBase_StageY;
	double	m_fBase_FrontLeft;
	double	m_fBase_FrontRight;

	double	m_fStageY;
	double	m_fStageRotate;
	double	m_fFrontY;
	double	m_fFrontLeft;
	double	m_fFrontRight;
	double	m_fFrontZ;

	S_GlsAlign () {
		Clear ();
	}

	void Clear () {
		m_fBase_StageY		= 0.0;
		m_fBase_FrontLeft	= 0.0;
		m_fBase_FrontRight	= 0.0;

		m_fStageY			= 0.0;
		m_fStageRotate		= 0.0;
		m_fFrontY			= 0.0;
		m_fFrontLeft		= 0.0;
		m_fFrontRight		= 0.0;
		m_fFrontZ			= 0.0;
	}
};

//Probe Align Info
struct S_ProbeAlign {
	double	m_fBase_ProbeY;
	double	m_fBase_BackLeft;
	double	m_fBase_BackRight;

	double	m_fProbeY;
	double	m_fProbeX;
	double	m_fBackY;
	double	m_fBackLeft;
	double	m_fBackRight;
	double	m_fBackZ;
	double	m_fStageY;

	S_ProbeAlign () {
		Clear ();
	}

	void Clear () {
		m_fBase_ProbeY		= 0.0;
		m_fBase_BackLeft	= 0.0;
		m_fBase_BackRight	= 0.0;

		m_fProbeY			= 0.0;
		m_fProbeX			= 0.0;
		m_fBackY			= 0.0;
		m_fBackLeft			= 0.0;
		m_fBackRight		= 0.0;
		m_fBackZ			= 0.0;
		m_fStageY			= 0.0;
	}
};

//Align CCD Item
struct S_AlignCcdItem {
	int	m_nExposureTime;
	int	m_nGain;
	int	m_nLed;
	int	m_nMatchingScore;

	S_AlignCcdItem () {
		Clear ();
	}

	void Clear () {
		m_nExposureTime  = 0;
		m_nGain			 = 0;
		m_nLed			 = 0;
		m_nMatchingScore = 0;
	}
};

//Align CCD Info
struct S_AlignCcdSet {
	S_AlignCcdItem m_sFrontLeft;
	S_AlignCcdItem m_sFrontRight;
	S_AlignCcdItem m_sBackLeft;
	S_AlignCcdItem m_sBackRight;

	S_AlignCcdSet () {
		Clear ();
	}

	void Clear () {
		m_sFrontLeft.Clear ();
		m_sFrontRight.Clear ();
		m_sBackLeft.Clear ();
		m_sBackRight.Clear ();
	}
};

//Align Margin Item
struct S_AlignMarginItem {
	double	m_fX;
	double	m_fY;
	double	m_fR;
	int		m_nCentering;
	int		m_nFindPtn;

	S_AlignMarginItem () {
		Clear ();
	}

	void Clear () {
		m_fX		 = 0.0;
		m_fY		 = 0.0;
		m_fR		 = 0.0;
		m_nCentering = 1;
		m_nFindPtn	 = 0;
	}
};

//Align Margin Info
struct S_AlignMargin {
	S_AlignMarginItem m_sGlassMargin;
	S_AlignMarginItem m_sProbeMargin;

	S_AlignMargin () {
		Clear ();
	}

	void Clear () {
		m_sGlassMargin.Clear ();
		m_sProbeMargin.Clear ();
	}
};

//Align Info
struct S_Align {
	wchar_t	m_chPtnFrontLeft[M_RCP_PROBE_MAX][CH_LEN];
	wchar_t	m_chPtnFrontRight[M_RCP_PROBE_MAX][CH_LEN];
	wchar_t	m_chPtnBackLeft[M_RCP_PROBE_MAX][CH_LEN];
	wchar_t	m_chPtnBackRight[M_RCP_PROBE_MAX][CH_LEN];

	S_GlsAlign		m_sGlsAlign;
	S_ProbeAlign	m_sProbeAlign;
	S_AlignCcdSet	m_sCcdSet;
	S_AlignMargin	m_sMargin;

	S_Align () {
		Clear ();
	}

	void Clear () {
		m_sGlsAlign.Clear ();
		m_sProbeAlign.Clear ();
		m_sCcdSet.Clear ();
		m_sMargin.Clear ();

		ZeroMemory (&m_chPtnFrontLeft, sizeof (m_chPtnFrontLeft));
		ZeroMemory (&m_chPtnFrontRight, sizeof (m_chPtnFrontRight));
		ZeroMemory (&m_chPtnBackLeft, sizeof (m_chPtnBackLeft));
		ZeroMemory (&m_chPtnBackRight, sizeof (m_chPtnBackRight));
	}
};

//_________________________________________________________//
//레시피 데이터 정보
struct S_RecipeData {
	vector <S_StickInfo> m_vecStick;
	S_HeadZ	m_sHeadZ;
	
	S_RecipeData () {
		Clear ();
	}

	void Clear () {
		if (!m_vecStick.empty()) m_vecStick.clear ();
		m_sHeadZ.Clear ();
	}
};

//레시피 옵션 정보
struct S_RecipeInfo {
	S_InspectItem		m_sInspectItem;
	S_OptionItem		m_sOptionItem;
	S_LampItem			m_sLampItem;
	S_ContactPos		m_sContactPos;
	S_MotionOffsetPos	m_sMotionOffsetPos;
	S_ProbeInfo			m_sProbeInfo;
	S_MotionSubZ		m_sMotionSubZ;
	S_MotionAngle		m_sMotionAngle;

	S_RecipeInfo () {
		Clear ();
	}

	void Clear () {
		m_sInspectItem.Clear ();
		m_sOptionItem.Clear ();
		m_sLampItem.Clear ();
		m_sContactPos.Clear ();
		m_sMotionOffsetPos.Clear ();
		m_sProbeInfo.Clear ();
		m_sMotionSubZ.Clear();
		m_sMotionAngle.Clear();
	}
};

//실사용 레시피
struct S_RecipeMgr {
	int				m_nVersion;
	int				m_nStickCnt;
	int				m_nCellCnt;
	S_RecipeInfo	m_sInfo;
	S_RecipeData	m_sData;
	S_RecipeMap		m_sMap;
	S_Align			m_sAlign;

	S_RecipeMgr () {
		Clear ();
	}

	void Clear () {
		m_nVersion  = 0;
		m_nStickCnt = 0;
		m_nCellCnt  = 0;
		m_sInfo.Clear ();
		m_sData.Clear ();
		m_sMap.Clear ();
		m_sAlign.Clear ();
	}
};
//_________________________________________________________//


//////////////////////////////////////////////
//Main 으로 명령 전달

//dw1
#define M_CMD_SHOW_REF_SET		0x01
#define M_CMD_PATTERN_SEARCH	0x02
#define M_CMD_CCD_SET			0x03
#define M_CMD_GLASS_GET_POS		0x04
#define M_CMD_GLASS_MOVE		0x05
#define M_CMD_PROBE_GET_POS		0x06
#define M_CMD_PROBE_MOVE		0x07
#define M_CMD_GLASS_PROC		0x08
#define M_CMD_PROBE_PROC		0x09
#define M_CMD_RESET				0x10
#define M_CMD_LAMP_SET			0x11
#define M_CMD_CONTACT_SET		0x12
#define M_CMD_STAGE_GET_POS		0x13
#define M_CMD_STAGE_MOVE		0x14
#define M_CMD_STAGE_PROBE_MOVE	0x15
#define M_CMD_HEAD_GET_POS		0x16
#define M_CMD_HEAD_MOVE			0x17
#define M_CMD_SRM_LAMP_SET		0x18
#define M_CMD_TEST_MODE			0x19	// LYS 2026-07-02 TestMode dialog


//#define	M_CMD_LAMP_ALL		0x01
//#define M_CMD_LAMP_SEL		0x02
//#define M_CMD_MOVE_STAGE	0x03
//#define M_CMD_MOVE_HEAD		0x04
//#define M_CMD_MOVE_CONTACT	0x05

//dw2
#define M_HEAD_FRONT_LEFT	0x01
#define M_HEAD_FRONT_RIGHT	0x02
#define M_HEAD_BACK_LEFT	0x03
#define M_HEAD_BACK_RIGHT	0x04
#define M_PROBE_MOVE_POS	0x05		
#define M_PROBE_MOVE_OD		0x06
#define M_HEAD_LEFT			0x07
#define M_HEAD_RIGHT		0x08
#define M_HEAD_FRONT		0x09
#define M_HEAD_BACK			0x10
//#define M_HEAD_LEFT			0x01
//#define M_HEAD_RIGHT		0x02

//dw3
#define M_TYPE_REVIEW		0x01
#define M_TYPE_SPEC			0x02
#define M_TYPE_QUALITY		0x03
#define M_TYPE_COLOR		0x04
#define M_TYPE_ANGLE		0x05

//ProcRun
#define M_RUN_NONE			0
#define M_RUN_REQ			1
#define M_RUN_PROC			2
#define M_RUN_END			3
#define M_RUN_NG			4

struct	S_RecipeRunState {
	LPVOID	m_pData;
	DWORD	m_dwData1;
	DWORD	m_dwData2;
	DWORD	m_dwData3;

	double	m_fData[100];
	int		m_nData[10];

	//double	m_fData1;
	//double	m_fData2;
	//double	m_fData3;
	//double	m_fData4;
	//double	m_fData5;
	
	int		m_nRunStatus;

	wchar_t	m_chData1[CH_LEN];

	S_RecipeRunState () {
		Clear ();
	}
	
	void Clear () {
		m_pData			= 0;
		m_dwData1		= 0x00;
		m_dwData2		= 0x00;
		m_dwData3		= 0x00;

		ZeroMemory (&m_fData, sizeof (m_fData));
		ZeroMemory (&m_nData, sizeof (m_nData));
		//m_fData1		= 0.0;
		//m_fData2		= 0.0;
		//m_fData3		= 0.0;
		//m_fData4		= 0.0;
		//m_fData5		= 0.0;
		m_nRunStatus	= 0;

		ZeroMemory (&m_chData1, sizeof(m_chData1));
	}
};