#pragma once

#include <nivision.h>
#include "OtherPPADefine.h"	// S사
#include "CPtrnInfo.h"

#define VTM_RV_SUCCESSED			1
#define VTM_RV_FAILED				0
#define df_HOST_OFFLINE				0
#define	df_HOST_ONLINE_REMOTE		1
#define	df_HOST_ONLINE_LOCAL		2
#define	df_HOST_MAINT_MODE			3
#define	df_HOST_PM_MODE				4
#define	df_ROTATE0					0	
#define	df_ROTATE90					1	
#define	df_ROTATE180				2	


#define	WBS_SHARE_MEM_STRING		_T("KMAC_ICM_SHARED_MEMORY")
#define	VTM_SHARE_MEM_STRING		_T("CCLINK")

#define df_MAX_PPA_COUNT			1080
#define df_RESULT_NUM_PPA			6
#define	dfSystem_ITEM_MAX			147	
#define	dfSLOT_ITME_MAX				45	
#define	dfSTAGE_ITME_MAX			50
#define	df_MAX_IO					128
#define df_MAX_RESULT_DATA_SE		20		// Result 결과를 화면에 보여주는 갯수	
#define df_MAX_RESULT_DATA_CD		10
#define df_MAX_SPC_DATA_CD			5			
#define df_MAX_CALIBRATION_DATA_CD	5			
#define df_MAX_CELL_NUM				250			
#define df_MAX_PPA_NUM				20			
#define df_MAX_PTRN_NUM				10			
#define df_MAX_DEFECT_CNT			30			
#define df_MAX_PB_NUM				1000	// Stick 당 측정 횟수


#define	MAX_STAGE					8
#define MAX_SLOT					25
#define MAX_WID_SIZE				20
#define MAX_CID_SIZE				20
#define MAX_RECIPEID_SIZE			64
#define MAX_LOADTIME_SIZE			30
#define MAX_JOBID_SIZE				64
#define MAX_STAGENAME_SIZE			64
#define	MAX_GLASS_ID				64		//Glass ID 최대 길이
#define	MAX_CASSETTE_ID				64		//Cassette ID 최대 길이
#define	MAX_RESERVE_RECIPE			64		//나중에 사용할지 몰라서
#define	MAX_OPER_ID					30	
#define	MAX_RECIPE_ID				256		//Recipe ID 최대 길이
#define	MAX_PROBE					1		//현재는 Head가 2개만 사용, 나중에 추가 가능
#define	MAX_ALIGN_MARK				2		//Align Mark 사용할 Image 갯수 
#define MAX_OVLI_PG_PATTERN			4	
#define MAX_OVLI_PG_DEGREE			3


#define	INT_SUB_PATTERN_NO_MAX 		10	
#define	df_MAX_PROBE				2
#define	df_MAX_GANTRY				1
#define	df_MAX_RESERVE_RECIPE		64		//Reserved
#define	MAX_STR_SIZE_NORMAL			1024
#define	MAX_STR_SIZE_MAX			4096
#define	df_MAX_ALIGN_MARK			2		//Align Mark 사용할 Image 갯수
#define	df_MAX_VISION_DLG			3
#define	df_MAX_PTRNINFO				10
#define	INT_LAYER_NO_MAX 			5	//jw.lee 20191011 change.
#define	df_MAX_DATA_COUNT			100
#define	df_MAX_CELL					16  // jsyoon 2022.10.19 add
enum lens_use_t 
{
	LENS_USE_ALIGN = 0,
	LENS_USE_MEAS,
	LENS_USE_AUX1,
	LENS_USE_AUX2,
	LENS_USE_AUX3,
	USE_LENS_END
};

enum bsc_use_t {
	BSC_USE_UVAA = 0,
	BSC_USE_MAIN,
	BSC_USE_UVTEG,
	USE_BSC_END
};

#define INT_OBJ_LENS_USE_MAX		USE_LENS_END
#define INT_OBJ_LENS_DB_MAX			20

const enum USE_ALARM
{
	df_USE_PLC_TEACH,
	df_USE_PLC_IONIZER_ALARM,
	df_USE_PLC_IONIZER_LOW,			// W
	df_USE_PLC_IONIZER_CONDITION,	// W
	df_USE_PLC_TEMPERSS,			// Temperature
	df_USE_PLC_AREASS,				// ARM
	df_USE_PLC_VACUUM,
	df_USE_PLC_AIRSS,
	df_USE_PLC_DOOR,
	df_USE_PLC_EMO,
	df_USE_PLC_BOX_FAN,
	df_USE_PLC_GLOVE, 
	df_USE_PLC_PCW_LEAK,
	df_USE_PLC_ISOLATOR,			// POSITION
	df_USE_PLC_WINDOWS,
	df_USE_SMOKE_DETECT,			
	df_USE_PLC_DOOR_USER_UNLOCK,  
	df_USE_PLC_ISOLATOR_PRESS,    
	df_USE_PLC_PROBE_EXIST_SENSOR_1,	
	df_USE_PLC_PROBE_EXIST_SENSOR_2,	
	df_USE_PLC_GPS_BREAKER_ON,
	df_USE_PLC_UPS_BREAKER_ON,
	df_USE_PLC_SAFETY_PLC_CP_ON,
	df_USE_PLC_HEATINGJACK_POWER,
	df_USE_PLC_PRESSURE_CDA,
	df_USE_PLC_PRESSURE_N2,		
	df_USE_PLC_PRESSURE_CYLINDER,
	df_USE_PLC_PRESSURE_RS_CLAMP_LIFTPIN_SWITCH,
	df_USE_PLC_PRESSURE_CHUCK_BLOWING_OVLI,
	df_USE_PLC_PRESSURE_CHUCK_BLOWING_STEC,
	//end

	df_USE_MAX_ALARM,	
};
#define IONIZER_USE_MAX					2

const enum VTM_SUBMODULE
{
	VTM_MOTION		= 0,
	VTM_PLC,
	VTM_CIM,	
	VTM_VISION,	
	VTM_VISION2,
	VTM_VISION_ALIGN,
//	VTM_SE,		
	VTM_CCLINK,
	VTM_GANTRY,
	VTM_PROBE1,		//1			// 화질
	VTM_PROBE2,		//2			// 화질
	VTM_PROBE3,		//3			// 혼색
	VTM_PROBE4,		//4			// 혼색
	VTM_PROBE5,		//5			// 특성
	VTM_PROBE6,		//6			// ALIGNER
	VTM_PROBE7,		//7			// PG
	VTM_PROBE8,		//8			// LINE SCAN
	VTM_PROBE9,		//9			// SE
	VTM_PROBE10,	//10		// PPA
	VTM_PROBE11,	//11		// PPA
	VTM_PROBE12,	//12		// PG
	VTM_PROBE13,	//13		// DFS
	VTM_PROBE14,	//14		// PIXEL BITE
	VTM_PROBE15,	//15		// PIXEL BITE
	VTM_PROBE16,	//16
	VTM_PLC_WRITE,
	// RS-232
	VTM_LED,	
	VTM_LED2,
	VTM_LED3,
	VTM_LED_TR,		// TR
	VTM_UV_LED,		
	VTM_AFSENSOR,	
	VTM_AFSENSOR2,
	VTM_FIXTURE,	
	VTM_CLAMP,
	VTM_PCAN = VTM_CLAMP,
	VTM_ZOOMSCOPE,
	VTM_ZOOMSCOPE2,
	VTM_RSMETER,	// RS8
	VTM_VCR,		//
	VTM_COMM1,
	VTM_COMM2,
	VTM_COMM3,
	// USB
	VTM_ILLUMINOMETER,	// 조도계
	//
	VTM_SPECTRO,		// ThickMeasure	
	VTM_RESERVE1,
	VTM_RESERVE2,
	VTM_RESERVE3,
	VTM_CA,	
	MAX_SUBMODULE,
//	VTM_END
};

// 통합으로 관리하는 BIT 
const enum PLC_MBIT{
 	PLC_MBIT_READY = 0,
 	PLC_MBIT_LOAD,
 	PLC_MBIT_MEAS,
	PLC_MBIT_ALARM,
	PLC_MBIT_WARRING,

	PLC_MBIT_LOADER_DETECT,

	PLC_MBIT_HEAD_SAFETY,
	PLC_MBIT_WT_SAFETY,

	PLC_MBIT_WT_BLOWING_ON,
	PLC_MBIT_WT_VACCUM_ON,
	PLC_MBIT_WT_VACCUM_OFF,
	PLC_MBIT_MAIN_VACCUM_ON,

	PLC_MBIT_GLASS_EXIST,									//GLASS EXIST CHECK(SAMPLE STAGE and LOADER)
	PLC_MBIT_GLASS_EXIST_LOADER,		// 챔버설비			//GLASS EXIST CHECK(LOADER)
	PLC_MBIT_GLASS_EXIST_LOADER_XOR,	// 챔버설비			//GLASS EXIST CHECK(LOADER)
	PLC_MBIT_GLASS_EXIST_WT,								//GLASS EXIST CHECK(SAMPLE STAGE)
	PLC_MBIT_GLASS_EDGE,				// 글라스 파손확인	

	PLC_MBIT_GATE_OPEN_LOADER,								//LOADER GATE OPEN
	PLC_MBIT_GATE_CLOSE_LOADER,								//LOADER GATE CLOSE
	PLC_MBIT_GATE_OPEN_PROBE,								//PACC GATE OPEN
	PLC_MBIT_GATE_CLOSE_PROBE,								//PACC GATE CLOSE

	PLC_MBIT_FIXTURE_UP,
	PLC_MBIT_FIXTURE_DOWN,

	// 점등검사기
	PLC_MBIT_PROBE_HOME,			
	PLC_MBIT_PROBE_NONCONTACT,		
	PLC_MBIT_PROBE_CONTACT,			

	PLC_MBIT_PROBE_ZERO_LOCK,
	PLC_MBIT_PROBE_PCB_LOCK,
	PLC_MBIT_PROBE_CLAM_LOCK,
	PLC_MBIT_PROBE_ZERO_LOCK_ABNORMAL,

	PLC_MBIT_PROBE_EXIST_UNIT,
	PLC_MBIT_PROBE_EXIST_AC,

	PLC_MBIT_AC_POS_HOME,
	PLC_MBIT_AC_POS_UNIT,
	PLC_MBIT_AC_POS_SAFETY,
	PLC_MBIT_AC_POS_PROBE,

	PLC_MBIT_AC_POS_UP,
	PLC_MBIT_AC_POS_DOWN,
	PLC_MBIT_AC_POS_XOR,

	PLC_MBIT_SE_LEMP_A,
	PLC_MBIT_SE_LEMP_B,

	PLC_MBIT_FIXTURE_IN,
	PLC_MBIT_FIXTURE_OUT,

	PLC_MBIT_PROBE_EXIST_UNIT_OR,		

	//clamp status check
	PLC_MBIT_CLAMP_ON,					
	PLC_MBIT_CLAMP_OFF,					

	PLC_MBIT_END,
};


/**************************************************************************
구조체를  변경하면 SMEM.dll도 함께 변경해야함.
1. ShareMem : dfSLOT_ITME_MAX,dfSTAGE_ITME_MAX갯수 변경
2. SMEM : StringTable추가삭제
3. SMEM : int StageItem[]추가삭제
4. SMEM : Api함수추가삭제 (cpp, h, def)
5. SMEM : SMEM_GetStageItemValue , SMEM_SetStageItemValue 추가삭제.
6. SMEM : SMEM_TransferCarrier,SMEM_ReadStageFromFile,SMEM_ResetStage_AllData,등 추가.
**************************************************************************/
const enum PROCESS_STATUS
{
	PROCESS_NONE = -1,			//No Process
	PROCESS_SKIP,				//Ready To Process But will not run process
	PROCESS_READY,				//Ready To Process
	PROCESS_RUNNING,			//Processing
	PROCESS_COMPLETE,			//Finish Process
	PROCESS_ALIGN_COMPLETE,		//Align Finished but Process not finished
	PROCESS_ERROR,				//Error state		
	PROCESS_END,
};

const enum PROCESS_TYPE_VALUE
{
	PTV_NONE		 = 0,
	PTV_CARRIER_IN	 ,
	PTV_READ_CID	 ,
	PTV_LOAD_READY	 ,			//Ready
	PTV_LOAD_QUE	 ,			//open + processwithcarrier(slotmap)
	PTV_LOAD		 ,			//work
	PTV_UNLOAD_READY ,			//Ready
	PTV_UNLOAD_QUE	 ,			//Queue
	PTV_UNLOAD		 ,			//open + work
	PTV_CARROUT_QUE	 ,
	PTV_CARROUT		 ,
	PTV_CANCELCARRIER_QUE ,
	PTV_CANCELCARRIER ,
	PTV_CARRIERCOMPLETE ,
};

const enum EQUIP_STATE_VALUE
{
	ESV_IDLE		 = 0,
	ESV_READY		 ,
	ESV_RUN			 ,
	ESV_ERROR		 ,
	ESV_SKIPMODE	 ,			// SKIP MODE
	ESV_OUTOFSERVICE ,			// COLD RUN MODE
	ESV_NOT_INITIALIZED,
};

const enum SYSTEM_PROCESS_STATUS
{
	SPS_IDLE		=0,
	SPS_PROCESS		,
	SPS_INITIAL		,
	SPS_MAINT		,
	SPS_WAFERTRANSFER,
	SPS_CYCLETEST,
	SPS_PARTICLETEST,
	SPS_MAINTING,
	SPS_EXITING,
};

//순서 바뀌면 안됨.
const enum SYSTEM_HOSTMODE_STATUS
{
	SHS_OFFLINE		   =0,
	SHS_ONLINE_REMOTE  ,
	SHS_ONLINE_LOCAL   ,
	SHS_MAINT		   ,
};

const enum SYSTEM_HOSTCONNECTSTATUS
{
	SCS_HOST_DISCONNECT		= 0,
	SCS_HOST_CONNECT  ,
};

const enum SYSTEM_ACCESSMODE_STATUS
{
	SAS_ACCESS_MANUAL		= 0,
	SAS_ACCESS_AUTO,
};

const enum SYSTEM_INITIAL_STATUS
{
	SIS_INITIAL_NONE		 = 0,
	SIS_INITIAL_COMPLETE		,
	SHS_INITIAL_CHANGE_MAINT	,
};

const enum IDS_STAGE_OPTION
{
	IDS_STAGE_BOPTION1		 = 26,
	IDS_STAGE_BOPTION2,
	IDS_STAGE_BOPTION3,
	IDS_STAGE_BOPTION4,
	IDS_STAGE_BOPTION5,
	IDS_STAGE_BOPTION6,
	IDS_STAGE_BOPTION7,
	IDS_STAGE_BOPTION8,
	IDS_STAGE_BOPTION9,
	IDS_STAGE_BOPTION10,
	IDS_STAGE_DOPTION1,
	IDS_STAGE_DOPTION2,
	IDS_STAGE_DOPTION3,
	IDS_STAGE_DOPTION4,
	IDS_STAGE_DOPTION5,
	IDS_STAGE_DOPTION6,
	IDS_STAGE_DOPTION7,
	IDS_STAGE_DOPTION8,
	IDS_STAGE_DOPTION9,
	IDS_STAGE_DOPTION10,
	IDS_STAGE_OUTOFSERVICE,
	IDS_STAGE_VIRTUALMODE,
		
};

const enum IDS_SLOT_OPTION
{
	IDS_SLOT_BOPTION1		 = 26,
	IDS_SLOT_BOPTION2,
	IDS_SLOT_BOPTION3,
	IDS_SLOT_BOPTION4,
	IDS_SLOT_BOPTION5,
	IDS_SLOT_BOPTION6,
	IDS_SLOT_BOPTION7,
	IDS_SLOT_BOPTION8,
	IDS_SLOT_BOPTION9,
	IDS_SLOT_BOPTION10,
	IDS_SLOT_DOPTION1,
	IDS_SLOT_DOPTION2,
	IDS_SLOT_DOPTION3,
	IDS_SLOT_DOPTION4,
	IDS_SLOT_DOPTION5,
	IDS_SLOT_DOPTION6,
	IDS_SLOT_DOPTION7,
	IDS_SLOT_DOPTION8,
	IDS_SLOT_DOPTION9,
	IDS_SLOT_DOPTION10,
};
	
const enum Cell_Num
{
	CELL_01		=0,
	CELL_02,
	CELL_03,
	CELL_04,
	CELL_05,
	CELL_06,
	CELL_07,
	CELL_08,
	CELL_09,
	CELL_10,
	CELL_11,
	CELL_12,
	CELL_13,
	CELL_14,
	CELL_15,
	CELL_16,
	CELL_17,
	CELL_18,
	CELL_19,
	CELL_20,
	CELL_21,
	CELL_22,
	CELL_23,
	CELL_24,
	CELL_25,
	CELL_26,
	CELL_27,
	CELL_28,
	CELL_29,
	CELL_30,
	CELL_31,
	CELL_32,
	CELL_33,
	CELL_34,
	CELL_35,
	CELL_36,
	CELL_37,
	CELL_38,
	CELL_39,
	CELL_40,
	CELL_41,
	CELL_42,
	CELL_43,
	CELL_44,
	CELL_45,
	CELL_46,
	CELL_47,
	CELL_48,
	CELL_49,
	CELL_50,
	CELL_51,
	CELL_52,
	CELL_53,		
	CELL_54,		
	CELL_55,
};


typedef struct  
{
	double			m_dx;			//위치 x
	double			m_dy;			//위치 y
	double			m_ds;			//위치 s
	double			m_dz;			//위치 z
	BOOL			bAF;
	BOOL			bUse;	
}COORDINATE, *pCOORDINATE;

typedef struct  
{
	double	dPosX;
	double	dPosY;
	double	dPosY2;
	double	dPosS;
	double	dPosS2;
	double	dPosZ;
	double	dPosZ2;
	double	dPosL;
	double	dPosL2;
}DBLMOVE, *PDBLMOVE;

typedef struct  
{
	COORDINATE	coord;
	BOOL		bUse;	
	int			iDir;
}PPA_COORD, *pPPA_COORD;


typedef struct  
{
	COORDINATE	coord;
	int			nRefDarkSelect;		//-1:NONE, 0:REF, 1:DARK, 2:SAFETY
	int			nProbe;
}REF_DARK_COORD, *pREF_DARK_COORD;

typedef struct  
{
	//for S2F87
	char			szGlassStatus[256];		//	"O" or "1" 
	char			szLotID[32];			//	LOT ID
	char			szCassetteID[32];		//	Cassette ID
	char			szSlotID[32];			//	Slot ID
	char			szRecipeID[32];			//	Recipe ID
	char			szProcessingCode[256];	//	Processing Code
	char			szGlassOrder[2561];		//	Glass Order
	char			szGlassID[32];			//	Glass ID
	char			szGlassGradeCode[256];	//	Glass Grade Code
	char			szPanelJudgeInfo[256];	//	Glass Panel Judgement Information
	char			szSubPanelInfo[256];	//	Sub Pane Information
	short			nPortID;
	long			lGlassCode;
	short			nReIput;				//	Glass Re-Input Flag
	char			szGlassThickness[256];	//	Glass's Thickness
	char			szPartNumber[256];
	char			szTypeOfProducing[256];	//	Type Of Producing
	char			szLotAttribute[256];	//	Lot Attribute
	char			szGlassType[256];		//	"N" Normal Glass, "D" Dummy Glass
	char			szGlassKeyID[256];		//	Glass Key ID
	
	//for S6F63
	char			szGLSProc[256];			//	PROCESSING CODE - GLSPROC_UNPROCESSED("0"),GLSPROC_PROCESSING("1"),GLSPROC_PROCESSED("2"),GLSPROC_REJECTED("3")
	
	char			szMODECODE[1];			//	Host Online / Offline code
	char			szMULTIFLAG[1];			//	"T" True (Multi Recipe ID), "F" False (Single Recipe ID)
	
	short			nSourceECS;				//	ECS number, Glass Data from which ECS
}GLASSDATA;

typedef struct  
{
	char			m_szRecipeDescription[MAX_RECIPE_ID];
	char			m_szRecipeID[MAX_RECIPE_ID];
}RECIPE_ID, *PRECIPE_ID;

//장비에 투입된 Glass 1장에 대한 정보
typedef struct  
{
	char			m_szGlassID[MAX_GLASS_ID];
	char			m_szCassetteID[MAX_CASSETTE_ID];
	char			m_szOperID[MAX_OPER_ID];

	char			m_szGlassReportScrapType[64];				
	char			m_szGlassReportScrapCode[64];
	char			m_szGlassRequestGlassID[64];			

	char			m_szCurrent_GLSMVSTATUS[64];	//현재 GLSMVSTAUTS

	int				m_nControlMode;
	int				m_nConnected;
	int				m_nPortID;
	int				m_nSlotID;
	int				m_nStepID;
	
	BOOL			m_bExist;
	int				m_nJobState;					//const enum GLASS_Job_Status 참조할 것
	double			m_dGlassSizeX;
	double			m_dGlassSizeY;
	RECIPE_ID		stRecipe;
	long			m_nGlassCode;						//DataViewer에 추가할 것 by Jay Hong : 2009.10.26

	BOOL			m_bNormalStatus;
	BOOL			m_bUnloadEnable;
	BOOL			m_bLoadEnable;
	BOOL			m_bExchangeEnable;
	BOOL			m_bPinActionBusy;
	BOOL			m_bStageDescription1;
	BOOL			m_bStageDescription2;
	BOOL			m_bUnloadReady;
	BOOL			m_bLoadReady;
	BOOL			m_bExchangeReady;
	BOOL			m_bRobotBusy;
	BOOL			m_bRobotEnd;
	BOOL			m_bNextGlassExist;
	BOOL			m_bPinActionRequest;
	BOOL			m_bUnloadComplete;
	BOOL			m_bLoadComplete;

	GLASSDATA		stGlassData;

	int				m_nCellCount;
	int				m_nStickCount;
	
}ECS_INFO, *PECS_INFO;

//typedef struct
//{
//	CString	   strRecipe;
//	CString	   strAlignParam;
//	CString	   strPosParam;
//	CString	   strMeasParam1;
//	CString	   strMeasParam2;
//	BOOL	   bUseAlign;
//	int		   iPosCnt;
//}RECIPELIST, *PRECIPELIST;

typedef struct 
{						
	int			m_nSubPtrnNum;
	double		m_dThreshold;
	int 		m_nWhitePPAXNum[INT_SUB_PATTERN_NO_MAX+1];	
	int 		m_nWhitePPAYNum[INT_SUB_PATTERN_NO_MAX+1];
	int 		m_nUVPPAXNum[INT_SUB_PATTERN_NO_MAX+1];	
	int 		m_nUVPPAYNum[INT_SUB_PATTERN_NO_MAX+1];
	char		WhitePtrn[INT_SUB_PATTERN_NO_MAX+1][256];
	char		UVPtrn[INT_SUB_PATTERN_NO_MAX+1][256];
	CRect		WhitePtrnRect[INT_SUB_PATTERN_NO_MAX+1];			// defined Rectangle
	CRect		UVPtrnRect[INT_SUB_PATTERN_NO_MAX+1];				// defined Rectangle
}SUBPATTERN, *PSUBPATTERN;


//VISION 관련 recipe 정보 
typedef struct	
{
	int		m_iCCDShutter;
	int		m_iCCDGain;
	int		m_iExposureTime;
	int		m_iLampIntensity;
	int		iLensIdx;
	int		iAFCch;
	int		iAfIntensity;		// AF Sensor Intensity
	double	dAfOffset;
	
	double	m_dAAZOffset;
	double	m_dRGain;
	double	m_dGGain;
	double	m_dBGain;

	int		m_iUVTypeIdx;
}VISION, *PVISION;

typedef struct 
{
	BOOL   bUse;
	int    iAxis;
	double dPos;
	int	   iSpeed;
}CMDAXISMOVE, *PCMDAXISMOVE;

typedef struct 
{
	int    m_iAreaNum;
	int    m_iProbeIndex;
	int    m_iLens;
	BOOL   m_bAF;
	BOOL   m_bSkip;
	BOOL   m_bGetSig;		// SE
	int	   m_iSrmTaget;		// SE
	TCHAR  m_szModel[256];	// SE
	double m_dPos[100];
	TCHAR  m_szCellID[256];
	TCHAR  m_szPattern[256];
}AREAINFO, *PAREAINFO;

typedef struct 
{
	double			m_dAF;								//	Auto Focus
	double			m_ds;								//	Probe S좌표
	double			m_dz;								//	Probe Z좌표
	double			m_dy;								//	Probe Y좌표
	double			m_dx;
	double			m_dl;								//	Probe L좌표
	int				m_nProbe;							//	Probe Number
	BOOL			m_bAF;								//	Auto Focus
	BOOL			m_bSelected;						//	Probe 선택여부
	BOOL			m_bUsed;							//	Probe Use	// Meas Skip TRUE is Skip
	BOOL			m_bOptMTP;							//	Use MTP Option
	int				m_iUsedMeas;						//	Use Meas Option		// ICP 4:시야각, 2:화질, 1:특성		// ICE // 1: SE, 2: RS, 	// PPA, SNAP, HOLE, OPEN-MASK		// TR 101: SAMPLE, 102:DUMMY
	int 			m_iProbeIndex;						//	0: Review, 1:화질, 2:시야각
	int 			m_iAreaNum;							
	TCHAR			m_CellID[32];						//	Cell ID
	TCHAR			m_szModel[32];						//	Model 정보
	int				m_iReason[df_MAX_PPA_NUM];			//	Measure Result
	int				m_nTmp[20];
	double			m_dTmp[20];
	CMDAXISMOVE		stAxis[100];	// Multi
	double			dRsProbeCotactPos;					//jw.lee 20191025 add.
}PROBE, *PPROBE;

typedef struct  
{
	PROBE			stProbe[df_MAX_PROBE];
	double			dGantry[df_MAX_GANTRY];	//Gantry X좌표
	double			dMoveUserOffset[100];	// User Offset
	int				nGroup;					//Group Number
	int				iGroup;					//Selected Group
}MEASGROUP, *PMEASGROUP;

typedef struct  
{
	int				CellNum;		// BLOCK Count
	PointFloat		GlobalMark;
	PointFloat		CellEdig;
	PointFloat		CellDistance;
	PointFloat		CellSize;
	PointFloat		CellCount;		// CELL 
	PointFloat		CellOffset;		// BLOCK Distance
	int				nResult;		// 측정진행 상태
}CELLINFO, *PCELLINFO;

typedef struct  
{
	int					m_iVersion;		
	BOOL				m_bOption1;		// SE장비에서는 Pattern Match 실행 여부
	BOOL				m_bOption2;		// Reserved
	BOOL				m_bOption3;		// Reserved
	BOOL				m_bOption4;		// Reserved
	BOOL				m_bOption5;		// Reserved
	BOOL				m_bOption6;		// Reserved
	BOOL				m_bOption7;		// Bend Mode
	BOOL				m_bOption8;		//
	BOOL				m_bOption9;		//
	
	double				m_dReserve[df_MAX_RESERVE_RECIPE];//Reserved	
	UINT				m_nGroupCnt;
	UINT				m_nProbeCnt;

	UINT				m_nRotate;
	UINT				m_nCellCnt;
	double				m_dProbeW;
	double				m_dProbeH;

	CELLINFO			stCellInfo;

	CArray <CELLINFO, CELLINFO&>   m_aryCellGroup;	
	CArray <MEASGROUP, MEASGROUP&> m_aryMeasGroup;

}RECIPEPATH, *PRECIPEPATH;

typedef struct taqResultPPA
{
	BOOL   bAlgorithm;
	int	nEdgeStrength;
	char szPPAName[MAX_STR_SIZE_NORMAL];
	PointFloat nresPPA[df_MAX_PPA_NUM];
	int	iMeasResult;	// 0: NotMeas, 1: OK, 2: Spc out, 3: AF, 4:Pattern
}RESULTPPADATA , *pRESULTPPADATA;

typedef struct taqResult
{
	int nMeasArea;	// Meas Group Count
	int iMeasArea;	// Current Meas Group
	int iEvenCnt;	// Meas Even No
	int nEvenCnt;	// Meas Even Max
	int nPPAArea;	// PPA Count
	int	nStickNo;	// CELL Col		// Point H
	int nCellNo;	// CELL Row		// Point R
	int sPPAArea;	// Spread Row
	int sPPAArea2;	// Spread Row
	int iProbe;
	BOOL bResultOK;
	double dFocus;
	double dMean;
	int    iMethod;
	char szCellName[MAX_STR_SIZE_NORMAL];
	char szPPAImage[MAX_STR_SIZE_MAX];
	double dAngle;
	BOOL bMeasOK;
	int nLampIntensity;				
	int nExposureTime;				
	int nCCDGain;					
	RESULTPPADATA stDataPPA[10];
}RESULTDATA , *pRESULTDATA;

// clamp state
typedef struct
{
	BOOL				bOn;
	BOOL				bOff;
	long				lPos;
}CLAMPSTS, *PCLAMPSTS;

// clamp state
typedef struct
{
	double				dS;
	double				dY;
	double				dZ;
	double				dL;
}HEADPOSINFO, *PHEADPOSINFO;

typedef struct  
{
	int					m_iVersion;					
	BOOL				m_bOptionUseAlign;					//SE장비에서는 Pattern Match 실행 여부
	BOOL				m_bOptionUseStaticAlign;		
	BOOL				m_bOptionUsePattern;				//Reserved

	COORDINATE			stCoordAlign[df_MAX_ALIGN_MARK];	//Align Mark용. 2개 사용
	COORDINATE			stcoordRef;							//Ref Position save
	COORDINATE			stcoordDark;						//Dark Position save
	COORDINATE			stcoordSafety;						//safety Position save	
	RECIPE_ID			stRecipeID;
	UINT				m_nGroupCnt;
	CArray <MEASGROUP, MEASGROUP&> m_aryMeasGroup;			
	VISION				stVision[df_MAX_VISION_DLG];		
 	CPtrnInfo			m_ptrnMeas;							
 	CPtrnInfo			m_ptrnAlign;	
 	CPtrnInfo			m_pSubPtnWhite[df_MAX_RESULT_DATA_CD];
 	CPtrnInfo			m_pSubPtnUV[df_MAX_RESULT_DATA_CD];
	double				dRSStanbyZ;
	int					m_iLensIdx[INT_OBJ_LENS_USE_MAX];			
	CArray <SUBPATTERN, SUBPATTERN&> m_arySubPattern;					
	BOOL				m_bUseClamp;	
	
	// PPA 
	BOOL				m_bOneShotMode;								
	BOOL				m_bOneShotUseUvLed;								
	double				m_dPPASpcMax;								
	double				m_dPPASpcMin;
	char				m_szPPAID[5][MAX_STR_SIZE_NORMAL];	
	
	double				m_dResultPos;			
	double				m_dTopPos;								
	double				m_dBottomPos;								
	double				m_dLineDistance;								
	int					m_iPointH;			
	int					m_iPointR;								
	int					m_iPointN;				
				
	double				m_dDefoSizeX[df_MAX_RESULT_DATA_CD];					
	double				m_dDefoSizeX_Offset[df_MAX_RESULT_DATA_CD];
	double				m_dDefoSizeY[df_MAX_RESULT_DATA_CD];								
	double				m_dDefoSizeY_Offset[df_MAX_RESULT_DATA_CD];	
	double				m_dGlassSizeX[df_MAX_RESULT_DATA_CD];								
	double				m_dGlassSizeX_Offset[df_MAX_RESULT_DATA_CD];			
	double				m_dGlassSizeY[df_MAX_RESULT_DATA_CD];								
	double				m_dGlassSizeY_Offset[df_MAX_RESULT_DATA_CD];
	double				m_dPPArangeX[df_MAX_RESULT_DATA_CD];								
	double				m_dPPArangeY[df_MAX_RESULT_DATA_CD];	

	double				m_dCalibration_X[df_MAX_CALIBRATION_DATA_CD];			
	double				m_dCalibration_Y[df_MAX_CALIBRATION_DATA_CD];
	double				m_dOffset_X[df_MAX_CALIBRATION_DATA_CD];								
	double				m_dOffset_Y[df_MAX_CALIBRATION_DATA_CD];	

	double				dReviewMeasZ;								
	double				dFirstCellPosX;								
	double				dFirstCellPosY;								
	double				dCellDistanceX;								
	double				dCellDistanceY;								
	int					iDefectCode[df_MAX_DEFECT_CNT];				
	double				dFirstProbePosX;							
	double				dFirstProbePosY;							
	double				dSecondProbePosX;							
	double				dSecondProbePosY;							
	BOOL				m_bSaveProbeImage;													
	BOOL				m_bCellNum[df_MAX_CELL_NUM];								

}RECIPE, *PRECIPE;

typedef struct 
{
	PointFloat nCenPoint;
	PointFloat ndistance;	
	PointFloat strengthLR;
	PointFloat strengthTB;

	Rect	   rectROI;
	char	   szPPAName[MAX_STR_SIZE_NORMAL];
	int		   nMeasEVEN;
	int		   nEdgeDir;		// In-Out, Out->in
	int		   nEdgeStrength;
	int		   nSmoothing;
	int		   nPreprocess;
}SUBMEASDATAPPA, *PSUBMEASDATAPPA;

typedef struct  
{
	// PPA POS
	PointFloat nPPAPos;	
	PointFloat fDepo[df_MAX_PTRN_NUM];
	PointFloat fGlass[df_MAX_PTRN_NUM];
	PointFloat fPPA[df_MAX_PTRN_NUM];

	PointFloat fStenLR[df_MAX_PTRN_NUM];
	PointFloat fStenTB[df_MAX_PTRN_NUM];

	int		   nRotate;
	int		   nCntEVEN;	// pattern cnt
	int		   nCntLTPS;	// pattern cnt
	double	   dMeas;
	double	   dFocus;

	// PPA 
	SUBMEASDATAPPA LTPSDefort;
	SUBMEASDATAPPA EVENPoint[df_MAX_PTRN_NUM];
	SUBMEASDATAPPA LTPSPoint[df_MAX_PTRN_NUM];

}MEASDATAPPA, *PMEASDATAPPA;

typedef struct  
{
	double x;
	double y;
}DOUBLEDATA, *PDOUBLEDATA;

typedef struct	
{
	char szIP[MAX_STR_SIZE_NORMAL];
	int  iPort;
	int  iBaudrate;
	int	 iDataBit;
	int  iParityBit;
	int	 iStopBit;
	int  iUseCnt;
	int  iMaxCnt;	
	int  iCtrlType;				// 0: SPO  1: CAS	2:아날로그(ACS) 3: MEI	4:IntensiLight
	TCHAR szName[8][32];
}MODULEPARAM, *PMODULEPARAM;

typedef struct 
{
	int					iVersion;							// Version
	int					m_nCoordPPa;						// Probe num
	int					m_nPPACnt;							// PPA Count
	int					m_iLensIdx[INT_OBJ_LENS_USE_MAX];	// Lens	

	// PPA 
	int					m_nShotMode;
	int					m_iResultPos;	
	
	// SPC
	int					m_nMarginNG;						// NG Margin count
	BOOL				m_bUseSpc;							// SPC Option
 	DOUBLEDATA			m_dCalibration[df_MAX_PTRN_NUM];	// PPA Calibration						
	DOUBLEDATA			m_dSPCDefoSize[df_MAX_PTRN_NUM];
	DOUBLEDATA			m_dSPCGlassSize[df_MAX_PTRN_NUM];
	DOUBLEDATA			m_dSPCPPASize[df_MAX_PTRN_NUM];
	DOUBLEDATA			m_dSPCDefoLimit[df_MAX_PTRN_NUM];
	DOUBLEDATA			m_dSPCGlassLimit[df_MAX_PTRN_NUM];

	VISION				stSetCCD;							// CCD Settiong
	CPtrnInfo			ptrnMeas;							// PPA Align Makr
}RECIPEPPA, *PRECIPEPPA;


typedef struct  
{
	BOOL	bMeasSkip;
	CString	strCellID;
	double	dCross;	
	double	dGantry;
	double	dReviewZ;
	double	dVisionZ;
	double	dVisionFcs;
	double	dVisionZoom;
	double	dColorZ;
	double	dColorFcs;
	double	dColorR;
	double	dColor_sZ;
	double	dColor_sFcs;
	double	dColor_sR;
	short	sModel;
}ALLMEASPARAM, *PALLMEASPARAM;

typedef struct  
{
	BOOL		m_bUsed;

	// PPA POS
	PointDouble nPPAPos;		// PPA 위치
	int			nRotate;		// PPA 방향
	int			nLens;			// 측정 Lens
	double		dMeasZ;			// Z축 위치
	int			nPtrnPosNum;	// PPA 패턴 위치 번호

	int			m_iFilterUVType;
	int			m_iXCiteUVCh;

	// CELL ID
	TCHAR		m_CellID[32];	
	TCHAR		szModel[10][128];	// 0: MT, 1: REAL, 2:PPA

}MEASPARAMETER, *PMEASPARAMETER;

typedef struct  
{
 	UINT	m_nSubParamCnt;
 	MEASPARAMETER m_nSubParm[df_MAX_PPA_NUM];			// 20개까지 설정가능하게 하자

}MEASSUBGROUP, *PMEASSUBGROUP;

typedef struct 
{
	UINT		  nSubGroup;	// Group 번호
	MEASPARAMETER stParam;
	CMDAXISMOVE	stAxis[100];
	short		sModel;

}MEASHEADGROUP, *PMEASHEADGROUP;

typedef struct 
{
	int		m_iVersion;	
	int		iLight[20];
	BOOL	bUse[20];			// 0: 사전검사, 1:전기검사,2:특성,3:화질,4:색이상,5:MTP,6:PPA	13:R-TEST PASS,14:샘플링검사,15:ProbeAF,16:Align,17:LineScan,18:ReviwSnap,19: MTP
	int		nTimeout[20];		// 0: 사전검사, 1:전기검사,2:특성,3:화질,4:색이상,5:MTP,6:PPA
	double	dCentactZ[20];		// User Offset X:11, Y:12, R:13

	int		iTmp[100];			// 0:Filter, 
	double	dTmp[100];			// 0:User Gap 

	PointFloat	CellCount;		// CELL 
	
	int		m_nUseProbe;
	char	m_szProbeID[5][MAX_RECIPE_ID];

}RECIPEICP, *PRECIPEICP;

// ICE RECIPE DLL 에서 MAIN으로 전달하기위한 구조체
typedef struct  
{
	UINT	iLight[20];
	UINT	nTimeout[20];
	double	dCentactZ[20];		// RS
	UINT	iTmp[100];			// 0:RS High, 1:RS Low,
								// 2:Measure Head, 3:Gap, 4:Probe Model	
								// 5:Current, 6:Use Coeff., 7:WITDH, 8:HIGHT, 9:PIN DSIT. S, 10:X, 11:Y
	double	dTmp[100];			// 0:User Gap 
	UINT	m_iSelHead;	
	UINT	m_iMeasType;		// 1:SE, 2:RS, 3:SR, 4:TR, 5:PPA, 6:CA	
	MEASPARAMETER m_stMeasParam;
}RECIPEMEASICE, *PRECIPEMEASICE;

// 패턴 매칭을 위한 구조체
typedef struct 
{
	VISION		stSetCCD;		// CCD 정보
	CPtrnInfo	m_ptrnInfo;		// 패턴 정보
}RECIPEPATTEN, *PRECIPEPATTEN;

struct RECIPEMEAS_POS
{
	double	dTmp[100];			// 0:User Offset	91:PPA SPC

	int		m_nColMax;
	int		m_nRowMax;
	int		m_nRotate;
	int		m_nSubGrup;				// Sub Group 개수
		
	CArray <MEASGROUP, MEASGROUP&> m_aryMeasGroup;					// WT 이동 
	CArray <MEASSUBGROUP, MEASSUBGROUP&> m_aryMeasSubGroup;			// Sub 이동
	CArray <MEASHEADGROUP, MEASHEADGROUP&> m_aryMeasHeadGroup;		// HEAD 이동

	RECIPEMEAS_POS()	
	{
		ClearInfo();
	}

	void operator=(RECIPEMEAS_POS& rPos)
	{
		memcpy(dTmp, rPos.dTmp, sizeof(double)*100);
		m_nColMax = rPos.m_nColMax;
		m_nRowMax = rPos.m_nRowMax;
		m_nRotate = rPos.m_nRotate;
		m_nSubGrup = rPos.m_nSubGrup;
		m_aryMeasGroup.Copy(rPos.m_aryMeasGroup);
		m_aryMeasSubGroup.Copy(rPos.m_aryMeasSubGroup);
		m_aryMeasHeadGroup.Copy(rPos.m_aryMeasHeadGroup);
	}
	void ClearInfo ()
	{
		for(int i=0; i<100; i++)
		{
			dTmp[i] = 0.0;
		}
		
		m_nColMax = 0;
		m_nRowMax = 0;
		m_nRotate = 0;
		m_nSubGrup= 0;

		m_aryMeasGroup.RemoveAll();
		m_aryMeasSubGroup.RemoveAll();
		m_aryMeasHeadGroup.RemoveAll();
	}
};

struct  THICK_MEAS
{
	int		m_nITime;
	int		m_nAvg;
	int		m_nBoxcar;
	int		m_nRefExposureTime;	
};

struct  RECIPEMEAS
{
	UINT	m_iVersion;	

	//	ThickMeasure
	THICK_MEAS stThick[10];
		
	// ICE							// 2:특성,3:화질,4:색이상,5:MTP,6:PPA,7:SE,8:RS,9:SR,10:TR,11:8K			16:Align,17:Pattern
	UINT	iLight[21];
	BOOL	bUse[21];				// 0: SE, 1:RS,2:PPA,		16:Align,17:Pattern
	UINT	nTimeout[21];			// OCP 2:특성,3:화질,4:색이상,5:MTP,6:PPA,  ICE 0:SE, 1:RS, 2:PPA
	double	dCentactZ[21];			// RS

	UINT	nPercentDefect[2];		// 0: Mixed Color, 1:Characteristic
	BOOL	bUsePercentDefect[2];	// 0: Mixed Color, 1:Characteristic

	BOOL	bTmp[100];				// 
	UINT	iTmp[100];				// 0:RS High, 1:RS Low

	double	dTmp[100];				// 0:User Gap 

	PointFloat	CellCount;			// CELL		// Operate DLL 사용

	// ALIGN
	TCHAR	szRcpAlign[MAX_STR_SIZE_NORMAL];


	// PROBE ID -> USE ICP, OCP  
	int		m_nUseProbe;
	TCHAR	m_szProbeID[5][MAX_RECIPE_ID];
	//////////////////////////////////////////////////////////////////////////

	UINT	m_nSubGroupCnt;
	CArray <MEASSUBGROUP, MEASSUBGROUP&> m_aryMeasSubGroup;

	RECIPEMEAS()	
	{
		ClearInfo();
	}

	void ClearInfo ()
	{
		m_iVersion	= 1;
		m_nSubGroupCnt = 0;

		for(int i = 0; i < 2; i++)
		{
			nPercentDefect[i] = 0;
			bUsePercentDefect[i] = 0;
		}

		for(int i=0; i<10; i++)
		{
			stThick[i].m_nITime = 10;
			stThick[i].m_nAvg = 5;
			stThick[i].m_nBoxcar = 3;
			stThick[i].m_nRefExposureTime = 1000;
		}
			
		for(int i=0; i<21; i++)
		{
			iLight[i]= 0;
			bUse[i] = FALSE;
			nTimeout[i] = 60;
			dCentactZ[i]= 0;		// RS, PROBE
		}

		for(int i=0; i<100; i++)
		{
			bTmp[i] = FALSE;
			iTmp[i] = 0;
			dTmp[i] = 0.0;
		}

		_tcscpy_s(szRcpAlign, _countof(szRcpAlign), _T(""));

		m_nUseProbe = 0;

		for(int i=0; i<5; i++)
		{
			_tcscpy_s(m_szProbeID[i], _countof(m_szProbeID[i]), _T("PROBEID"));
		}

		m_aryMeasSubGroup.RemoveAll();
	}
};

// ICP 사용 구조체, 패턴이미지 필요없음
typedef struct 
{
	int					m_iVersion;	
	BOOL				bUse4PAlign;					// 4Point Align
	char				szName[MAX_STR_SIZE_NORMAL];
	int					iLight[20];						// ICP CCD 
	VISION				stSetCCD;
	COORDINATE			stCoordAlign[df_MAX_ALIGN_MARK][df_MAX_PROBE];	// BASE 좌표, Align Mark용. 2개 사용
	COORDINATE			stoffsetAlign[4];								// Aligner Recipe
	double				dPosCard[10];	// Probe Card AF Position
	double				dPosAlign[50];	// Glass Align Position
	CMDAXISMOVE			stAxis[2][100];	// 0:Probe, 1:Glass Align Position	// 리뷰로 얼라인하니까 동일축이 필요해서 새로 추가함
	int					nBuf[100];
	double				dBuf[100];

}RECIPEALIGN, *PRECIPEALIGN;

// ICE 사용 구조체, 패턴이미지 필요함
struct RECIPEALIGN2  
{
 	int					m_iVersion;	
	RECIPEALIGN			stRecipeAlign;
	CPtrnInfo			ptrnAlign[df_MAX_PTRNINFO];

	RECIPEALIGN2 ()
	{
		void ClearInfo();
		void ClearAlignInfo();
	}

	void ClearInfo()
	{
 		m_iVersion = 1001;

		ZeroMemory(&stRecipeAlign, sizeof(stRecipeAlign));

		stRecipeAlign.m_iVersion = 1000;
		stRecipeAlign.bUse4PAlign = FALSE;

		strcpy_s(stRecipeAlign.szName, _countof(stRecipeAlign.szName), "Default");

		int i=0;
		for (i=0; i<df_MAX_PTRNINFO; i++)
		{
			ptrnAlign[i].ClearPtrn();
			ptrnAlign[i].ClearInfo();
		}
	}

	void ClearAlignInfo()
	{
		ZeroMemory(&stRecipeAlign, sizeof(stRecipeAlign));
	}
};




//==============================================================================
//Main View의 Current Position display에 표시될 내용 - 공유메모리에서 사용될 예정=====
typedef struct  
{
	TCHAR			szName[MAX_STR_SIZE_NORMAL];		// UI 표시
	BOOL			bUse;								//장치 사용여부, UI상에서도 보이지 않게 처리함. OutOfService와는 다름
	BOOL			bConnect;							//연결 상태
	BOOL			bOutOfService;						//모듈 사용 여부 0: USE
	BOOL			bInitOk;
	BOOL			bRunInit;							//Initialize를 할 것인지 여부. False인 경우 Init 하지 않음
	int				nTypeNum;							// 연결 Type 설정
	MODULEPARAM     stModuleParam;						// TCP / RS-232

	double			dMoveOffset;		// Line CCD 

	double			m_dx;				//Prober의 위치 x
	double			m_dy;				//Prober의 위치 y
	double			m_ds;				//Prober의 위치 s
	double			m_dz;				//Prober의 위치 z
 	double			m_dLP;				//Prober의 위치 Lift Pin
	double			m_dLP2;				//Prober의 위치 Lift Pin
	double			m_dRJ;				//Prober의 위치 
	double			m_dR;				//Prober의 위치 Rotate

	BOOL			bWaitORG;			//Only for xCIM
	
	//Safety Position
	double			m_dSafety_X;		// TOP
	double			m_dSafety_Y;		// TOP
	double			m_dSafety_Y1;		// BOTTOM Align left
	double			m_dSafety_Y2;		// BOTTOM Align Right
	double			m_dSafety_Z;
	double			m_dSafety_Z2;

	double			m_dSafety[50];

	double			m_dUnitOffset;

	//Lift Pin Position					
	double			m_dPinUpPosition;
	double			m_dFixtureOffPosition;
	double			m_dPinDownPosition;
	double			m_dLPDownPosition;

	double			m_dLoadPos[3];			// X, Y, R
	double			m_dUnLoadPos[3];		// X, Y, R
	double			m_dMeasPos[3];			// X, Y, R
	double			m_dAutoChagnePos[3];	// Y. R, ACS FeedBack Value
	
	double			m_dRotateLDX;
	double			m_dRotateLDY;
	double			m_dRotateLDR;

	
	//Reference Position
	double			m_dRef_x;
	double			m_dRef_s;
	double			m_dRef_y;
	double			m_dRef_z;
	BOOL			m_bRef_MoveDone;
	BOOL			m_bRef_MeasDone;

	//Dark Position
	double			m_dDark_x;
	double			m_dDark_s;
	double			m_dDark_y;
	double			m_dDark_z;
	BOOL			m_bDark_MoveDone;
	BOOL			m_bDark_MeasDone;

	//AF
	BOOL			m_bAFRes;			// AF 실행 유무
	BOOL			m_bAFMode;			// IO AF 모드 확인용
	BOOL			m_bAFDone;			// IO AF 완료 확인용

	//PTRN
	BOOL			m_bDetect_Spot;	
	BOOL			m_bDetected_Meas;
	BOOL			m_bDetected_Align;
	float			m_fScore;	

	// Error
	int				m_nError;			// Measure skip
	int				m_nReason;			// Not Use
	 
	BOOL			m_bDisPlacementSensor;
	char			m_szEqState[1];

	BOOL			m_bUseLifePin;
	BOOL			m_bInvertXY;
	CLAMPSTS		bChampState[15];				
	CLAMPSTS		bOldChampState[15];				
	BOOL			m_bUseClampInterlock;			
	HEADPOSINFO		strHeadPosInfo[df_MAX_PROBE];	// 2Head Pos

	double			m_dVSSpotDiaUM;
	BOOL			m_bMeasureDone;		// 측정 완료
	BOOL			m_bComputeDone;		// 계산 결과완료
	BOOL			m_bUseMode[2];		// SR / TR

	int				m_iSpotCx;
	int				m_iSpotCy;

	double			m_dSpeed;

}SYSTEM, *PSYSTEM;
//====================================================================================


typedef struct 
{
	TCHAR	m_szProbeCardID[32];
	TCHAR	m_szProbeCardIDTime[32];
	UINT	m_nUseCnt;
}PROBEINFO, *PPROBEINFO;

typedef struct  
{
	char	szMonitor[MAX_STR_SIZE_NORMAL];
	UINT	m_nProcess;
}MONITOR, *PMONITOR;

typedef struct  
{
	char	szName[MAX_PATH];
	int		iAxisMap;	
	int		iDir;		
	int		iStepper;
	double	dVH;
	double	dVM;
	double	dVL;
	double	dVO;
	double	dAH;
	double	dAM;
	double	dAL;
	double	dSTH;
	double	dSTM;
	double	dSTL;
	double	dEL;
	double	dCN;
	double	dDimMax;
	double	dDimMin;
	double	dOffset;
	double	dHomePos;

}CONFIGINFO_ACS, *PCONFIGINFO_ACS;

typedef struct  
{
	BOOL				bUse;
	BOOL				bEnable;
	BOOL				bLimitP;			// +
	BOOL				bLimitN;			// -
	BOOL				bHomeDone;
	double				dSetAxisSpeed;		// Set Speed
	double				dAxisSpeed;			// Current Speed
	double				dSetAxisPos;		// Set position
	double				dAxisPos;			// Current position
	double				dAxisRMS;			// 부하률
	CONFIGINFO_ACS		stConfigInfo_ACS;

}MOTIONSTATUS, *PMOTIONSTATUS;



//Main View의 Panel display에 표시될 내용 - 공유메모리에서 사용될 예정===============
typedef struct  
{
	TCHAR			m_szLotID[32];
	TCHAR			m_szGlassID[32];
	TCHAR			m_szSlotID[32];
	TCHAR			m_szRecipeID[32];
	TCHAR			m_szStepID[32];
	TCHAR			m_szGlsRotate[32];
	TCHAR			m_szCassetteID[32];
	TCHAR			m_szOperID[32];
	TCHAR			m_szSEModelID[32];					// SE Run Recipe // Sub Recipe
	TCHAR			m_szGlassCoordiType[32];	
	TCHAR			m_szEQPID[32];				
	TCHAR			m_szInnerID_SI[df_MAX_CELL][32];	// jsyoon 2022.10.19 change df_MAX_PROBE to df_MAX_CELL
	TCHAR			m_szInnerID[df_MAX_CELL][32];		// jsyoon 2022.10.19 change df_MAX_PROBE to df_MAX_CELL
	TCHAR			m_szCellID[df_MAX_CELL][32];		// jsyoon 2022.10.19 change df_MAX_PROBE to df_MAX_CELL
	TCHAR			m_szSubRecipe[df_MAX_PROBE][32];	// 별로레시피 사용하는 모듈
	TCHAR			m_szResultID[df_MAX_PROBE][32];
	TCHAR			m_szGlassID_Half[32];

	TCHAR			m_szWorkNo[32];
	TCHAR			m_szCIMRecipe[32];
	TCHAR			m_szLotType[32];

}LOTINFO, *PLOTINFO;
//====================================================================================



typedef struct 
{
	double	dAVGSignal;
	double	dDUVSignal;	
	double	dUVSignal;
	double	dIRSignal;
}RESULT_SE_SIGNAL, *PRESULT_SE_SIGNAL;

typedef struct  
{
	RESULT_SE_SIGNAL stSignal[5];	// 1:2, 2:10, 3:25, 4:60

	double	dAlignX;
	double	dAlignY;
	double	dIntensity;
	double	dAngle;
	double	dEMA;                
	char	szSEMeasModel[256];
	double	dBT[INT_LAYER_NO_MAX];	 // Bottom Layer
}RESULT_SE_INFO, *PRESULT_SE_INFO;

typedef struct  
{
	BOOL			m_bResult;
	int				m_nMaxLayer;
	int				m_nMaxGroup;
	double			m_dEbg;			// ??
	double			m_dRS;			// RS
	double          m_dMSE;
	double			m_dT[INT_LAYER_NO_MAX];
	double			m_dN[INT_LAYER_NO_MAX];
	double			m_dK[INT_LAYER_NO_MAX];
	TCHAR			szMatName[INT_LAYER_NO_MAX][256];
	TCHAR			szImageSE[MAX_STR_SIZE_MAX];
	RESULT_SE_INFO	stResultSE;		// SE 추가 항목

	// SRM 
	double			dSRM_SEThick[5];		// SE	100A, 250A, 600A
	double			dSRM_SRThick[5];		// 6K	500A, 2000A, 5000A, 10000A

	// 
	double			dILLMIPower[4];			// 조도계 값


	double			dPos[100];				// 측정좌표

	int				nData;								// 결과값 개수
	TCHAR			TcHead[df_MAX_DATA_COUNT][32];		// Data view 에 표시하기 위한 이름
	TCHAR			TcName[df_MAX_DATA_COUNT][32];		// 결과파일에 저장하는 이름
	TCHAR			TcData[df_MAX_DATA_COUNT][32];		// 결과값
}RESULT_INFO, *PRESULT_INFO;
//====================================================================================


typedef struct 
{
	// A/S/E/W Log Type
	int iEvtFlag;			// 0:A, 1:S, 2:E, 3:W
	char m_szEvtTime[64];	// Event Time
	int iSpqNum[10];		// NO
	int iStepNum[10];		// Meas Step
	int iFromto[10];		// FROM=TO
	BOOL bFlagStoE[10];		// Start to End	

	// I Log Type
	MOTIONSTATUS stMoterInfo[100];

	int nCntILog;			// ILOG Count		// ICP: 17, ICE: 31 ICM: OCP: 
	double dData[100];

}MCCICPDATA, *PMCCICPDATA;

//System InterLock Check
typedef struct  
{
	BOOL			m_bLampRedOn;
	BOOL			m_bLampYellowOn;
	BOOL			m_bLampGreenOn;
	BOOL			m_bLampBlueOn;
 	BOOL			m_bDetectedVacuumSensor;	// Chuck Glass 흡착 확인
	BOOL			m_bIsVacuumOnSensor;		// Vacuum On/Off 확인

	BOOL			m_bDetectedTTSensor[5];		// Temperature
	BOOL			m_bMCDown;
	BOOL			m_bCPDown;

	BOOL			m_bUseAlarm[df_USE_MAX_ALARM];		// Config Read
	BOOL			m_bSetAlarm[df_USE_MAX_ALARM];		// PLC Read
	int				m_nCntAlarm[df_USE_MAX_ALARM];		// Count

	BOOL			m_bUseCheckLoadState;
	BOOL			m_bUseCheckMeasureState;
	BOOL			m_bUseCheckReadyState;

	BOOL			m_bOldLockKeyAutoMode;		
	BOOL			m_bOldDoorLockMode;			
	BOOL			m_bLockKeyAutoMode;			

	// Sensor
	BOOL			bSensorH2OLow;				// H2O
	BOOL			bSensorO2Low;				// O2 18% under			
	BOOL			bSensorO2Under;				// O2 18% under	
	double			nSensorO2;					
	double			nSensorH2O;
	double			nSensorChemT;				// Temperature under
	double			nSensorO2PPM;
	double			nSensorSACCO2PPM;			// SACC O2 
	BOOL			bSensorSACCO2Low;			// O2 18% under			
	BOOL			bionizerstatus;				// Ionizer
	BOOL			bionizerPower;				// Ionizer Power
	double			nTemperature[5];			// Temperature under
	double			dChuckVacuumValue[5];		// Chuck Vacuum

	double			dInterlockO2PPM;			// O2 PPM 인터락 값
	double			dInterlockH2OPPM;			// H2O PPM 인터락 값
	bool			bO2InterlockUse;			// O2 ppm Interlock use
	bool			bH2OInterlockUse;			// H2O ppm Interlock use

	// LP
	BOOL			bAxisLPUpPos;
	BOOL			bAxisLPDownPos;

	// Glass
	BOOL			m_bGlassExist;		

	// R
	BOOL			bGlassRPos0;	
	BOOL			bGlassRPos90;	
	BOOL			bGlassRPos180;
	BOOL			bGlassRPos270;

	// RJ
	BOOL			bLoaderRPos0;			// 글라스 반입
	BOOL			bLoaderRPos90;			// 글라스 흡착 위치
	BOOL			bLoaderRPos180;			// 글라스 반출

	BOOL			m_bLiftPinDown;	
	BOOL			m_bLiftPinUp;
	BOOL			m_bLiftPinFixtureOffPos;
	BOOL			m_bLiftPinCylinderDown;		

	// RS 
	BOOL			m_bRSProbeContact;
	BOOL			m_bRSProbeNonContact;
	BOOL			m_bRSSafetyPos;		

	BOOL			m_bTRShutter;

	// CA
	BOOL			m_bCACylinderUp;
	BOOL			m_bCACylinderDown;

	BOOL			m_bCAPistonForward;
	BOOL			m_bCAPistonReverse;

	// UV
	BOOL			bUVSignalOn;	
	
	BOOL			m_bDoorUnLockMode;

	// PROBE
	BOOL			m_bProbeBufferExist[2];		// Buffer#1, Buffer#2
	BOOL			m_bProbeBufferUp[2];
	BOOL			m_bProbeBufferDown[2];
	BOOL			m_bProbeCardExist[10];

	// Grip
	BOOL			m_bUseGripSW;			// USE Grip switch

	BOOL			m_bArmDetected;
	BOOL			m_bArmDetectedAlarm;

	BOOL			m_bUserManualOn;

	BOOL			m_bMBit[1000];		// 인터락사용

	//Ionizer
	BOOL			m_bIonizerUse;
	BOOL			m_bIonizerAutoOff;			   //설정 시간 이후 자동 Off 기능
	BOOL			m_bIonizerOn;				   //PLC Ionizer IN IO
	BOOL			m_bIonizerSet[IONIZER_USE_MAX];//Ionizer On 시점
	int				m_nIonizerDelay;			   //설정 시간
	int				m_nIonizerCycleCount;		   //TEST 반복 횟수
	int				m_nIonizerProcCount;		   //완료 횟수
	

	BOOL			m_bIonizerBar_HighVoltALM;		
	BOOL			m_bIonizerBar_TipCleanALM;
	
	//mc Segmentation(mc 세분화, m_bMC_EFU, m_bMC_PACC)
	BOOL			m_bMC_EFU;						//EFU MC	
	BOOL			m_bMC_PACC;						//PACC MC

	BOOL			m_bXRayIonizerState;			
}SYSINTERLOCK, *PSYSINTERLOCK;
//====================================================================================

//Title Window에 표시될 내용
typedef struct 
{
	char		m_szUserPermit[64];
	char		m_szUser[64];
	int			m_nHostMode;
	BOOL		m_bHostConnect;
	int			m_nLPMode;
	int			m_nUPMode;
	char		m_szProcessStatus[64];
	char		m_strEquipNum[64];
	char		m_strEquipName[260];
}TITLEPANEL, *PTITLEPANEL;

typedef struct  
{
	UINT		m_nEQType;						//0: ICE, 1: ICM, 2: ICP, 3: OCP
	int			m_nProcessStatus;				//SYSTEM_PROCESS_STATUS
	int			m_nEQStatus;					//EQUIP_STATE_VALUE
	int			m_nAlarmStatus;					//0: NO ALARM, 1: ALARM
	
	int			m_nAccessModeLod1;				//SYSTEM_ACCESSMODE_STATUS
	int			m_nAccessModeLod2;				//SYSTEM_ACCESSMODE_STATUS
	int			m_nHostConnect;					//SYSTEM_HOSTCONNECTSTATUS
	int			m_nHostMode;					//SYSTEM_HOSTMODE_STATUS
	int			m_nInitialStatus;				//SYSTEM_INITIAL_STATUS
	int			m_nJobStage;
	char		m_szUserPermit[64];
	char		m_szUser[64];
	int			m_NoUser;
	UINT		m_nEQSubType;
	BOOL		m_bOption[10];						//Reserved 
	double		m_dOption[10];						//Reserved
	char		m_szSysMemory[256];		
	char		m_szMetrosysVersion[256]; 

} EQUIP, *PEQUIP;

typedef struct		// BOE B7 OLED OVLI DFS Format. Defect Information
{
	char szDefectNo[20];	
	int		iDefectNo;	
	char szDefecCode[20];
	char szDefectName[20];
	char szDefectType[10];
	char szCellID[20];

	double dCoorX1;
	double dCoorY1;
	double dCoorX2;
	double dCoorY2;

	int		iWidth;				
	int		iHeight;			

	double dCoorGX;
	double dCoorGY;
	double dCoorGX2;
	double dCoorGY2;
	double dCoorGY3;
	double dCoorGX3;
	double dCoorSX;
	double dCoorSY;
	double dCoorSX2;
	double dCoorSY2;
	double dCoorSX3;
	double dCoorSY3;
	double dCoorPX;
	double dCoorPY;
	double dCoorPX2;
	double dCoorPY2;
	double dCoorPX3;
	double dCoorPY3;
	double dMaskCoorSX;
	double dMaskCoorSY;
	double dMaskCoorSX2;
	double dMaskCoorSY2;
	double dMaskCoorSX3;
	double dMaskCoorSY3;
	double dDefectLineAddress;
	int	   iData;
	int    iGate;
	int	   iData2;
	int	   iGate2;

	double  dRealSize;
	double  dRealXSize;
	double  dRealYSize;
	double dRealArea;			

	char szVisionImage_1[200];
	char szVisionImage_2[200];
	char szDefectShape[30];
	//////////////////////////////////////////////////////////////////////////

	char szDefectPatternName[20];
	char szDefectPixel[10];
	char szImageFileName[200];
	char szDefectSizeType[10];
	double dDefectLength;
	double dDefectSizeX;
	double dDefectSizeY;
	int iDefectArea;
	char   szCaptre[10];

	char szRepeatDefect[10];
	char szBypass[10];
}VISION_DEFECT_INFORMATION;


typedef struct
{
	char	szTYPE[20];
	char	szNAME[20];
	char	szCOLOR[20];

	char	szSIZETYPE[20];
	int 	iWidth;
	int 	iHeight;
	double	dRealWidth;
	double	dRealHeight;

	int		iData;
	int		iGate;
	double	dSize;
	char	szIMAGEPath[64];
	char	szVisionMappingPath;
	char	szColorMappingPath;

}VISION_CELL_RESULT;

typedef struct _stOVLI_CELL_INFO
{
	// BOE B7 OLED OVLI DFS Format.
	char szCellGrade[10];
	char szCellJudge[10];
	char szABJudge[10];		

	int iTotalDefectedDefCount;
	int iTotalDefectedCodeCount;

	// ToTal NG count
	int iTotalDotDefectCount;
	int iTotalLineDefectCount;
	int iTotalCMDefectCount;
	int iTotalMuraDefectCount;

	int iDarkLineDefect_H;
	int iDarkLineDefect_V;
	int iBrightLineDefect_H;
	int iBrightLineDefect_V;


	int iTotalDotDark;
	int iTotalDotBright;

	int iTotalLineDark;
	int iTotalLineBright;

	int iSizeODefectNum;
	int iSizeLDefectNum;
	int iSizeMDefectNum;
	int iSizeSDefectNum;

	int	iTotalMIXDefectCount;
	int	iTotalCMODefectCount;
	int	iTotalCMIDefectCount;

	char szCIEFlag[10];

	int iDataLine;
	int iGateLine;
	//////////////////////////////////////////////////////////////////////////

	// CIE NG Count
	int iCIENGCount[MAX_OVLI_PG_PATTERN];

	// WAD NG Count
	int iWADNGCount[MAX_OVLI_PG_PATTERN];

	// Luminance NG count
	int iLumNGCount[MAX_OVLI_PG_PATTERN];

	// C NG Count
	int iCNGCount[MAX_OVLI_PG_PATTERN];
	
	// E NG Count
	int iENGCount[MAX_OVLI_PG_PATTERN];

	VISION_DEFECT_INFORMATION VisionDefectInfo[500];
	
	// PG Data.
	int iPointNum;
	double dCoorOpticX1;
	double dCoorOpticY1;
	double dCoorOpticX2;
	double dCoorOpticY2;

	double dJNCD;

	char szOpticPatternName[10];
	char szRepeatAbnomal[10];
	char szBypass[10];

	double dPeakWave[MAX_OVLI_PG_PATTERN][MAX_OVLI_PG_DEGREE];
	double dHalfPeakWave[MAX_OVLI_PG_PATTERN][MAX_OVLI_PG_DEGREE];
	double dVoltage;

	char szCellID[32];

	// PG/Luminance
	double dCurrent[MAX_OVLI_PG_PATTERN];
	double dEfficiency[MAX_OVLI_PG_PATTERN][MAX_OVLI_PG_DEGREE];
	double dLuminance[MAX_OVLI_PG_PATTERN][MAX_OVLI_PG_DEGREE];
	double dx[MAX_OVLI_PG_PATTERN][MAX_OVLI_PG_DEGREE];
	double dy[MAX_OVLI_PG_PATTERN][MAX_OVLI_PG_DEGREE];
	double dUdot[MAX_OVLI_PG_PATTERN][MAX_OVLI_PG_DEGREE];
	double dVdot[MAX_OVLI_PG_PATTERN][MAX_OVLI_PG_DEGREE];
	BOOL bDefectCode[27];// Luminance(5), CIER(3), CIEG(3), CIEB(3), CIEW(3), Current(5), efficient(5)

	char iCIERedJudge[500];
	char iEfficiencyRedJudge[500];
	char iCIEGreenJudge[500];
	char iEfficiencyGreenJudge[500];
	char iCIEBlueJudge[500];
	char iEfficiencyBlueJudge[500];
	char iCIEWhiteJudge[500];
	char iEfficiencyWhiteJudge[500];
	int iPanelRejectType;

	int	iLuminanceRedSpcH;
	int	iLuminanceRedSpcL;
	int	iLuminanceGreenSpcH;
	int	iLuminanceGreenSpcL;
	int	iLuminanceBlueSpcH;
	int	iLuminanceBlueSpcL;

	int iPanelRejectCode;
	
	// Vision
	int	iCelltotalDefect;
	int iCelldotDark;
	int iCelldotBright;
	int iCelllineDarkV;
	int iCelllineDarkH;
	int iCelllineBrightV;
	int iCelllineBrightH;
	int iCelldarkMura;
	int iCellcmMura;
	int iCellcmoMura;
	int iCellcmiMura;
	VISION_CELL_RESULT stVision[500];

	// Color
	Rect stColorRect[100];

	inline void Clear()
	{ 
		memset(this, 0x0, sizeof(OVLI_CELL_INFO));
	}
}OVLI_CELL_INFO, *POVLI_CELL_INFO;


typedef struct _stRESULT_OVLI
{
	// BOE B7 OLED OV-LI DFS format.
	char	szLotID[20];
	char	szGLSID[20];
	char	szGLSJudge[10];
	char	szGLSGrade[10];
	char	szProgramName[50];

	int	iNGCellCount;
	int	iDotNGCellCount;
	int	iLineNGCellCount;
	int	iCMNGCellCount;
	int	iMuraNGCellCount;

	int	iCieNGCellCount;
	int	iCieX1NGCellCountR;
	int	iCieX1NGCellCountG;
	int	iCieX1NGCellCountB;
	int	iCieX1NGCellCountW;

	int	iWadCieNGCellCount;
	int	iWadCieNGCellCountR;
	int	iWadCieNGCellCountG;
	int	iWadCieNGCellCountB;
	int	iWadCieNGCellCountW;

	int	iLumNGCellCountR;
	int	iLumNGCellCountG;
	int	iLumNGCellCountB;
	int	iLumNGCellCountW;

	int	iCNGCellCountR;
	int	iCNGCellCountG;
	int	iCNGCellCountB;
	int	iCNGCellCountW;

	int	iENGCellCountR;
	int	iENGCellCountG;
	int	iENGCellCountB;
	int	iENGCellCountW;

	int iENGPCellCountB;
	int iCIENGPCellCountB;
	int iENGPCellCountG;
	int iCIENGPCellCountG;
	int iENGPCellCountR;
	int iCIENGPCellCountR;


	SYSTEMTIME stStartTime;
	SYSTEMTIME stEndTime;
	int	iGlasstotalDefect;
	int	iGlasstotaldotDark;
	int	iGlasstotaldotBright;
	int	iGlasstotallineDarkV;
	int	iGlasstotallineDarkH;
	int	iGlasstotallineBrightV;
	int	iGlasstotallineBrightH;
	int	iGlasstotaldarkMura;
	int	iGlasstotalcmMura;
	int iGlasstotalcmoMura;
	int iGlasstotalcmiMura;

	//Wuhan Tianma DFS Format
	int		m_iTotalPanel;
	int		m_iTotalInspectPanel;
	int		m_iTotalSkipPanel;
	int		m_iGoodPanel;
	int		m_iSize1DefectPanel;
	int		m_iSize2DefectPanel;
	int		m_iSize3DefectPanel;
	int		m_iSize4DefectPanel;
	int		m_iGPYield;

	int		iNGDefectNum;
	int		iMURADefectNum;
	int		iMIXDefectNum;
	int		iCMODefectNum;
	int		iCMIDefectNum;

	int		iLineCellCount;
	int		iDotCount;
	int		iCOMDefectCount;
	int		iENGCellCount;
	int		iCOM_DefectNGCount;
	int		iCMI_DefectNGCount;
	int		iCMO_DefectNGCount;
	int		iDM_DefectNGCount;
	int		iBD_DefectNGCount;
	int		iBL_DefectNGCount;
	int		iDD_DefectNGCount;
	int		iDL_DefectNGCount;
	int		iABC_DefectNGCount;
	int		iABP_DefectNGCount;

	int		iENGPCount;
	int		iENGCount;
	int		iCIENGCount;
	int		iCIENGPCount;


	int		iLineDefectDark;
	int		iLineDefectBright;

	OVLI_CELL_INFO m_stOVLICell[10][44];		//44 for PMOLED(index 40 ~ 43)
}RESULT_OVLI, *PRESULT_OVLI;


#define df_MAX_READ		64
#define df_MAX_WRITE	64

//(Purifier, CDA, N2)
#define PURGE_PURIFIER	0
#define PURGE_CDA		1
#define PURGE_N2		2
#define PURGE_END		3


enum cellStatus_use_t {
	df_CELLSTATE_ILDE = 0,
	df_CELLSTATE_PTP_NOR,		// Normal
	df_CELLSTATE_PTP_SHORT,			// Short
	df_CELLSTATE_PTP_OPEN,			// Open

	df_CELLSTATE_OST_NOR,		// Normal
	df_CELLSTATE_OST_SHORT,			// Short
	df_CELLSTATE_OST_OPEN,			// Open

	df_CELLSTATE_PASS,			// R_TEST PASS
	df_CELLSTATE_OVER,			// Over
	df_CELLSTATE_UNDER,			// UNDER


	df_CELLSTATE_PG_NOTCONNACT,
	df_CELLSTATE_RTEST_FAIL,

	df_CELLSTATE_AGING_START,
	df_CELLSTATE_AGING_NG,
	df_CELLSTATE_AGING_OK,

	df_CELLSTATE_WAD_START,
	df_CELLSTATE_WAD_END,
	df_CELLSTATE_VISION_START,
	df_CELLSTATE_VISION_END,
	df_CELLSTATE_COLOR_START,
	df_CELLSTATE_COLOR_END,
	df_CELLSTATE_ANGLE_START,
	df_CELLSTATE_ANGLE_END,

	
	df_CELLSTATE_END
};



typedef struct _stVTM_SYSTEM
{
	TCHAR					szEquipName[256];
	int						nMeasMode;

	BOOL					m_bChkBitIntensity_H1;
	BOOL					m_bChkBitIntensity_H2;

	PROBEINFO				stProbeInfo[5];
	SYSTEM					stSystem[MAX_SUBMODULE];	//SubModule 개수. 현재는 Gantry, Probe1, Probe2, SocketTest
	MONITOR					stMonitor;
	LOTINFO					stLotInfo;
	TITLEPANEL				stTitle;
	BOOL					bSystemInitOk;
	SYSINTERLOCK			stInterLock;				//System InterLock Checking
	ECS_INFO				stGlassInfo;				// 사용안함
	RESULT_INFO				stResult;	
	MEASGROUP			    stMeasGrp;
	EQUIP					stEquip;
	MOTIONSTATUS			stMotionStatus[100];				// Motion Info
	int						nCellStatus[100][50];			
	int						nBFCellStatus[100][50];			

	TCHAR					szProductID[32];
	
	TCHAR					szLastGlassID[256];
	TCHAR					szLastRcpName[256];
	TCHAR					szLastMeasTime[256];
	int						nLastStepT;
	int						nLastStepB;
	int						nLastMeasMode;
	int						nJobCount;
	int						dCurrentCycleCount;

	int						m_iMeasPPANum[df_MAX_PROBE];	// 측정 중인 PPA 순서
	
	double					dDisplacement_X;		//틀어진 만큼의 좌표값
	double					dDisplacement_Y;		//틀어진 만큼의 좌표값
	double					dDisplacement_Z;		//틀어진 만큼의 좌표값
	double					dDisplacement_S;		//틀어진 만큼의 좌표값
	
	double					dDisplacement_Angle;
	double					dNewTarpos_X;			//새롭게 보정된 좌표값
	double					dNewTarpos_Y;			//새롭게 보정된 좌표값
	
	int						m_dSystemErrCode;
	double					m_dJobGrpCnt;
	BOOL					m_bDarkMeas;
	
	COORDINATE				stRecipeAlignMarkPos[MAX_ALIGN_MARK][df_MAX_PROBE];	//Align Mark용. 2개 사용
	COORDINATE				stDetectAlignMarkPos[MAX_ALIGN_MARK][df_MAX_PROBE];	//Align Mark용. 2개 사용
	COORDINATE				stStaticAlignMarkPos;
	BOOL					m_bAlignMarkDetected;
	BOOL					m_bAlignMarkDetectedPos[df_MAX_ALIGN_MARK];
	BOOL					m_bAlignMarkDetected2[MAX_ALIGN_MARK][df_MAX_PROBE];
	int						m_nAlignMarkNum;				
	BOOL					m_bSkipTarget;
	double					m_dAlignOffset[5];				// Glass가 틀어짐을 보상
	double					m_dAlignResult[3];				// Probe가 틀어짐을 보상

	BOOL					m_bReWork;
	TCHAR					sz_MeasStartTime[16];
	TCHAR					sz_MeasEndTime[16];
	BOOL					m_bProcessComplete;		//각가의 서브 모듈들(Probe1, Probe2... 측정모듈들) 전부 끝났을 경우
 	BOOL					m_bMccTransferOn;
	TCHAR					m_strRootPath[260];
	TCHAR					m_strResultPath[260];
	TCHAR					m_strLogPath[260];
	TCHAR					m_strMCCPath[260];		
	TCHAR					m_strEquipName[260];	
	TCHAR					m_strEquipName2[260];	
	TCHAR					m_strModuleID[260];		
	BOOL					m_bVirtualMode;
	BOOL					m_bGlassJudge;
	HWND					hLog;
	HWND					hMetrosysView;			
	HWND					hCDlgNaviOperate;		
	HWND					hCDlgNaviRecipe;	
	TCHAR					m_szConfigPath[MAX_PATH];
	double					m_iMCCUploadTime;		
	BOOL					bCCLinkIn[df_MAX_READ];
	BOOL					bCCLinkOut[df_MAX_WRITE];
	double					dAxisZMax[df_MAX_PROBE];
	double					dAxisZMin[df_MAX_PROBE];
	double					m_dRSOffset_X;			
	double					m_dRSOffset_Y;			
	TCHAR					m_szPPAImage[256];
	TCHAR					m_szReviewImage[256];
	BOOL					bGlassLoadState;
	int						m_iRsContactNum;		 
	BOOL					m_bAxisMoving;			 
	int						iMachineStatus;	
	TCHAR					m_strRowImagePath[100];

	double					m_dMeasx;				//Prober의 위치 x
	double					m_dMeasy[df_MAX_PROBE];	//Prober의 위치 y

	int						iCurGroup;
	int						iShotMode;

	double					m_dGlobalX;
	double					m_dGlobalY;

	BOOL					m_bUsableOtherPPA;		
	int						m_iTEG_W;
	int						m_iTEG_H;
	VTM_PPA*				m_pOtherPPA;			

	BOOL					m_bMeasMultiRcp;
	BOOL					m_bUseSnapCellEdge;		

	BOOL					m_bUseSnapCellEdgeSecond;	

	BOOL					m_bUseDataUpLoad;	
	BOOL					m_bUseSecondDataUpLoad;	
	BOOL					m_bUseImageSizeChange;	
	BOOL					m_bUseGlassAbrot;		
	BOOL					m_bValidation;			
	BOOL					m_bCIMAlarmReported;		

	BOOL					m_bUseClamp[6];			
	UINT					m_nCurLIgnt[5][8];

	double					dRotatePos[3];				// Loader Align 보정값 챔버장비만 사용
	BOOL					m_bUseLdAlign;

	BOOL					m_bCompleteLoadResult;		

	int						m_nApdOpt[6];
	int						m_nDefectCnt;

	BOOL					m_bUseIntensityLight;		
	BOOL					m_bUseXCite;				

	BOOL					m_bTEGorAA;	//TRUE : TEG, FALSE : AA

	int m_dx;
	int m_dy;
	int m_cx;
	int m_cy;
	int m_firstx;
	int m_firsty;
	int m_offsety;
	double m_pitchx;
	double m_pitchy;


	int						m_iRJSleepTime[4];
	int						m_iRJSpeed[4];			// 1: First, 2:Second, 3:Third
	double					m_dRJPosition[4];	
	double					m_dRJMoveTact[4];		// 1: First, 2:Second, 3:Third, 0:Down
	
	double					m_fGlassSkipErr;
	double					m_fGlassRTestErr;
	double					m_fGlassDisplayErr;

	BOOL					m_bTargetAlarmUse;
	int						m_nPercentEfy;		//타겟값 알람 설정값
	int						m_nPercenElect;		
	int						m_nPercenSkip;		
	int						m_nPercentRTest;		
	int						m_nPercenDisplay;		

	BOOL					m_bMotionDisable;
	BOOL					m_bPurifierAlarmUse;
	BOOL					m_bPurifierStatus[PURGE_END]; //Purifier Circulation, CDA Purge, N2 Purge

	int						m_nO2ppmLimit;

	BOOL					m_bAutoAgingMode;
	int						m_nAgingListRow;
	int						m_nAgingListCol;
	BOOL					m_bUseN2PurgeRecipe;			

	int						m_nLanguage; 
	BOOL					m_bExchangeComplete;
	BOOL					m_bProbeExistError;		

	int						iDir_StageG;		
	int						iDir_HEADG;		

	TCHAR					szPPAName[df_RESULT_NUM_PPA][MAX_STR_SIZE_NORMAL/4];
	int						m_iPPANum;		

	double					m_dCAOffsetX;
	double					m_dCAOffsetY;
	double					m_dCAStandbyZ;			
	double					m_dCAMeasZ; 

	// Dummy position	// CA Meas Init. Position 
	double					m_dCADummyPosX;
	double					m_dCADummyPosY;
	// Dump Position
	double					m_dCADumpPosX;
	double					m_dCADumpPosY;

	TCHAR					m_strCommonDFRoot[MAX_PATH];
	TCHAR					m_strCommonDFPath[MAX_PATH];
	
	BOOL					m_bPreAlignResult;						// Pre Align 
	double					m_dPreAlignResultX;						// Pre Align
	double					m_dPreAlignResultY;						// Pre Align
	double					m_dPreAlignResultR;						// Pre Align

	//end
	
	unsigned short			m_nLPType;							
	unsigned short			m_nWhiteUVLEDType;					
	unsigned short			m_nMeasStateDegree;					
	unsigned short			m_nMotionInterlockType;				

	int						m_iCoefficientUse;					
	int						m_iCoefficientWidth;				
	int						m_iCoefficientHeight;				
	int						m_iCoefficientPindis;				
	int						m_iCoefficientX;					
	int						m_iCoefficientY;					
	int						m_nPPASrmLens;				
	RECIPEPATTEN			m_RecipePaternInfo[df_MAX_PTRNINFO];		
	unsigned short			m_nSEmodelCnt;							

	//4Point Align Use Value 
	BOOL					m_bUseFunc4PAlign;
	
	BOOL					m_bCheckRcpOpenForRS;	

	int						m_iEQFaceUpDown;			
	BOOL					m_iEQChamber;				

	int			m_nSEMultiRcpNum;
	int			m_nSRMultiRcpNum;

	BOOL		m_bAleadySendRoadRsRcp;					

	int GateValveStatus;	//Open 1, Close 2, BM 3
	int PurifierStatus;		//Run 1, IDLE 2, BM 3

	int nFlagJobTactTime;
	int nGlassInTime;
	int nGlassAlignTime;
	int nGlassMess;
	int nGlassOutReadyTime;
	int nGlassOutDelayTime;
	int nGlassOutTime;

	int nFlagPMTactTime;
	SYSTEMTIME stPMStartTime;		//마지막 Glass Out 시간
	int nPMReadyTime;				//PM준비: 마지막 Glass Out후 ATM 변경 Start까지
	int nPMVentTime;				//Vent Time: ATM 변경 Start ~ ATM 완료까지 시간
	int nPMWorkingTime;             //PM Working Time: ATM완료 ~ N2 치환 Start까지의 시간
	int nPMN2CirculationTime;       //N2 Ciculation : N2치환 ~ N2 치환 완료까지 시간
	int nPMGlassInWaitTime;			//Glass Wait Time After PM: N2 치환 완료 ~ GlassIn까지의 대기 시간

	SYSTEMTIME stJobTactStartTime; 
	SYSTEMTIME stGlassInCompleteTime;
	SYSTEMTIME stGlassAlignCompleteTime;
	SYSTEMTIME stGlassMeasCompleteTime;
	SYSTEMTIME stGlassOutReadyCompleteTime;
	SYSTEMTIME stGlassOutDelayTime;
	SYSTEMTIME stGlassOutTime;

	SYSTEMTIME stPMTactStartTime;				//마지막 Glass Out 시간
	SYSTEMTIME stPMReadyCompleteTime;			//PM준비: 마지막 Glass Out후 ATM 변경 Start까지
	SYSTEMTIME stPMVentCompleteTime;			//Vent Time: ATM 변경 Start ~ ATM 완료까지 시간		
	SYSTEMTIME stPMWorkingCompleteTime;			//PM Working Time: ATM완료 ~ N2 치환 Start까지의 시간
	SYSTEMTIME stPMN2CirculationCompleteTime;	//N2 Ciculation : N2치환 ~ N2 치환 완료까지 시간
	SYSTEMTIME stPMGlassInWaitCompleteTime;		//N2 Ciculation : N2치환 ~ N2 치환 완료까지 시간
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	int						m_nStatusGateValveOpen;
	int						m_nStatusGateValveClose;
	int						m_nStatusGateValveBM;
	int						m_nStatusPurifier1Run;
	int						m_nStatusPurifier1Idle;
	int						m_nStatusPurifier1BM;
	int						m_nStatusPurifier2Run;
	int						m_nStatusPurifier2Idle;
	int						m_nStatusPurifier2BM;
	int						m_nStatusPMCDAStart;
	int						m_nStatusPMCDAComplete;
	int						m_nStatusPMN2Start;
	int						m_nStatusPMN2Complete;
	int						nPPMO2Gap;

	TCHAR					m_szBufferPROBEID[2][32];	// Probe ID 
	TCHAR					m_szPROBEID[5][32];			// Probe ID 
	TCHAR					m_szLoadPROBEID[32];		// Probe ID 
	TCHAR					m_szACPROBEID[32];		    // AC Probe ID 
	TCHAR					m_szCassettePROBEID[10][32];	// Probe ID 

	bool					m_bAlarm_LampTimeOver[10];
	bool					m_bAlarm_TemperOver[20];

	int						m_nProbeChange;		// jsyoon 2022.05.19 add
	int						m_nAgingStartStick[5];  // 0~3 : Aging, 4 Meas

}VTM_SYSTEM, *PVTM_SYSTEM;


#define LANGUAGE_NONE					-1
#define LANGUAGE_KOREA					0
#define LANGUAGE_ENGLISH				1
#define LANGUAGE_CHINA					2

// RY
#define RY_SIG_TOWER_RED				0
#define RY_SIG_TOWER_YELLOW				1
#define RY_SIG_TOWER_GREEN				2
#define RY_SIG_TOWER_BLUE				3
#define RY_SIG_TOWER_BUZZER				4
#define RY_RSPICO_UPDOWN				5
#define RY_CCD_POWER					6
#define RY_UV_LAMP_SHUTTER_ONOFF		7
#define RY_WHITE_LAMP_SHUTTER_ONOFF		8
#define RY_EMO_SW_OUTPUT				9

// RX	
#define RX_AREA_SENSOR_01				0
#define RX_AREA_SENSOR_02				1
#define RX_UNLOADING_SENSOR_X			2
#define RX_UNLOADING_SENSOR_Y			3
#define RX_LIFTPIN_DOWN_SENSOR			6
#define RX_LIFTPIN_LOAD_UP_SENSOR		7
#define RX_LIFTPIN_VACUUM_UP_SENSOR		8
#define RX_POSITIVE_LIMIT_AXIS_X		9
#define RX_POSITIVE_LIMIT_AXIS_Y		10
#define RX_POSITIVE_LIMIT_AXIS_Z		11
#define RX_POSITIVE_LIMIT_AXIS_LIFT		12
#define RX_NEGATIVE_LIMIT_AXIS_X		13
#define RX_NEGATIVE_LIMIT_AXIS_Y		14
#define RX_NEGATIVE_LIMIT_AXIS_Z		15
#define RX_NEGATIVE_LIMIT_AXIS_LIFT		16
#define RX_EMO_SW						17
#define RX_AIR_ALARM_SENSOR				18
#define RX_VACUUM_ALARM_SENSOR			19
#define RX_TEMPERATURE_ALARM_SENSOR		20
#define RX_DOOR_SENSOR					21
#define RX_ARM_SENSOR					22
#define RX_RS_CYLINDER_DOWN				23
#define RX_RS_CYLINDER_UP				24
#define RX_UV_SHUTTER_ON_SENSOR			25
#define RX_UV_SHUTTER_OFF_SENSOR		26
#define RX_WHITE_SHUTTER_ON_SENSOR		27
#define RX_WHITE_SHUTTER_OFF_SENSOR		28	
#define RX_TEACH_MODE					29
#define RX_GLASS_EXIST					30
#define RX_EMO_STATE					31

#define dfEQ_FACE_DOWN					0							
#define dfEQ_FACE_UP					1

		