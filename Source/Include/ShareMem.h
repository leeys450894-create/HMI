//ShareMem.h
#ifndef __SHAREMEM_H__
#define __SHAREMEM_H__
#include <afxtempl.h>    // MFC templates

#if _MSC_VER > 1000
#pragma once
#include "DefinesSys.h"
//#include "CPtrnInfo.h"
#include "SharedDefin.h"
#endif // _MSC_VER > 1000

#define	WBS_SHARE_MEM_STRING	_T("KMAC_KKKOOO1234_SHARED_MEMORY")
#define	VTM_SHARE_MEM_STRING	_T("CCLINK")

#define	dfSystem_ITEM_MAX			147	
#define	dfSLOT_ITME_MAX				45	
#define	dfSTAGE_ITME_MAX			50
#define	MAX_STAGE					8
#define MAX_SLOT					25
#define	df_MAX_IO					128
#define MAX_WID_SIZE				20
#define MAX_CID_SIZE				20
#define MAX_RECIPEID_SIZE			64
#define MAX_LOADTIME_SIZE			30
#define MAX_JOBID_SIZE				64
#define MAX_STAGENAME_SIZE			64
#define	MAX_GLASS_ID				64		//Glass ID 최대 길이
#define	MAX_CASSETTE_ID				64		//Cassette ID 최대 길이
#define	MAX_OPER_ID					30	
#define	MAX_RECIPE_ID				256		//Recipe ID 최대 길이
#define	MAX_RESERVE_RECIPE			64		//나중에 사용할지 몰라서
#define	MAX_PROBE					1		//현재는 Head가 2개만 사용, 나중에 추가 가능
#define	MAX_ALIGN_MARK				2		//Align Mark 사용할 Image 갯수 
//#define	MAX_PLC_IN_CNT				200		//PLC Input 최대 접점 갯 수//Added by Jay Hong : 2009.05.20
//#define	MAX_SUBMODULE				32		//Sub Module 32개까지 지원 가능하도록 설정

#define df_MAX_RESULT_DATA_SE		20	// Result 결과를 화면에 보여주는 갯수	
#define df_MAX_RESULT_DATA_CD		10
#define df_MAX_SPC_DATA_CD			5				// pwk 20130704 
#define df_MAX_CALIBRATION_DATA_CD	5				// pwk 20310715 
#define df_MAX_CELL_NUM				250				// pwk 20130819 
#define df_MAX_PPA_NUM				20				// pwk 20130819
#define df_MAX_PTRN_NUM				10				// pwk 20130819
#define df_MAX_DEFECT_CNT			30				// pwk 20130819 
#define df_MAX_PB_NUM				1000			// jsyoon 20180430 Stick 당 측정 횟수

/**************************************************************************
구조체를  변경하면 SMEM.dll도 함께 변경해야함.
1. ShareMem : dfSLOT_ITME_MAX,dfSTAGE_ITME_MAX갯수 변경
2. SMEM : StringTable추가삭제
3. SMEM : int StageItem[]추가삭제
4. SMEM : Api함수추가삭제 (cpp, h, def)
5. SMEM : SMEM_GetStageItemValue , SMEM_SetStageItemValue 추가삭제.
6. SMEM : SMEM_TransferCarrier,SMEM_ReadStageFromFile,SMEM_ResetStage_AllData,등 추가.
**************************************************************************/
const enum PROCESS_STATUS{
	PROCESS_NONE = -1,			//No Process
	PROCESS_SKIP,				//Ready To Process But will not run process
	PROCESS_READY,				//Ready To Process
	PROCESS_RUNNING,			//Processing
	PROCESS_COMPLETE,			//Finish Process
	PROCESS_ALIGN_COMPLETE,		//Align Finished but Process not finished	//20120227 added by Jay Hong
	PROCESS_ERROR,				//Error state
		
	PROCESS_END
};

const enum PROCESS_TYPE_VALUE{
	PTV_NONE		 = 0,
	PTV_CARRIER_IN	 ,
	PTV_READ_CID	 ,
	PTV_LOAD_READY	 , //Ready
	PTV_LOAD_QUE	 , //open + processwithcarrier(slotmap)
	PTV_LOAD		 , //work
	PTV_UNLOAD_READY , //Ready
	PTV_UNLOAD_QUE	 , //Queue
	PTV_UNLOAD		 , //open + work
	PTV_CARROUT_QUE	 ,
	PTV_CARROUT		 ,
	PTV_CANCELCARRIER_QUE ,
	PTV_CANCELCARRIER ,
	PTV_CARRIERCOMPLETE ,
};

const enum EQUIP_STATE_VALUE{
	ESV_IDLE		 = 0,		// kjoh 20120216 add.
	ESV_READY		 ,
	ESV_RUN			 ,
	ESV_ERROR		 ,
	ESV_SKIPMODE	 ,			// SKIP MODE
	ESV_OUTOFSERVICE ,			// COLD RUN MODE
	ESV_NOT_INITIALIZED,		//20120222 added by Jay Hong
};

const enum SYSTEM_PROCESS_STATUS{
	SPS_IDLE		=0,
	SPS_PROCESS		,
	SPS_INITIAL		,
	SPS_MAINT		,
	SPS_WAFERTRANSFER,
	SPS_CYCLETEST,
	SPS_PARTICLETEST,
	SPS_MAINTING,
	SPS_REMEASURE,
	SPS_EXITING,
};

const enum SYSTEM_HOSTMODE_STATUS{//순서 바뀌면 안됨.
	SHS_OFFLINE		   =0,
	SHS_ONLINE_REMOTE  ,
	SHS_ONLINE_LOCAL   ,
	SHS_MAINT		   ,
};

const enum SYSTEM_HOSTCONNECTSTATUS{
	SCS_HOST_DISCONNECT		= 0,
	SCS_HOST_CONNECT  ,
};

const enum SYSTEM_ACCESSMODE_STATUS{
	SAS_ACCESS_MANUAL		= 0,
	SAS_ACCESS_AUTO,
};

const enum SYSTEM_INITIAL_STATUS{
	SIS_INITIAL_NONE		 = 0,
	SIS_INITIAL_COMPLETE		,
	SHS_INITIAL_CHANGE_MAINT	,
};


const enum IDS_STAGE_OPTION{
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

const enum IDS_SLOT_OPTION{
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
	
// pwk 20130819
const enum Cell_Num{
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

//Added by Jay Hong : 2009.06.02
typedef struct  {
	double			m_dx;			//위치 x
	double			m_dy;			//위치 y
	double			m_ds;			//위치 s
	double			m_dz;			//위치 z
	double			m_dRef;			//위치 Ref
	BOOL			bAF;
	BOOL			bUse;	
}COORDINATE, *pCOORDINATE;

typedef struct  {
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

typedef struct  {
	COORDINATE	coord;
	BOOL		bUse;	
	int			iDir;
}PPA_COORD, *pPPA_COORD;

//Added by Jay Hong : 2009.06.12
typedef struct  {
	COORDINATE	coord;
	int			nRefDarkSelect;		//-1:NONE, 0:REF, 1:DARK, 2:SAFETY
	int			nProbe;
}REF_DARK_COORD, *pREF_DARK_COORD;

typedef struct  {
	//for S2F87
	char			szGlassStatus[256];		//"O" or "1" 
	char			szLotID[32];			//LOT ID
	char			szCassetteID[32];		//Cassette ID
	char			szSlotID[32];			//Slot ID
	char			szRecipeID[32];			//Recipe ID
	char			szProcessingCode[256];	//Processing Code
	char			szGlassOrder[2561];		//Glass Order
	char			szGlassID[32];			//Glass ID
	char			szGlassGradeCode[256];	//Glass Grade Code
	char			szPanelJudgeInfo[256];	//Glass Panel Judgement Information
	char			szSubPanelInfo[256];	//Sub Pane Information
	short			nPortID;
	long			lGlassCode;
	short			nReIput;				//Glass Re-Input Flag
	char			szGlassThickness[256];	//Glass's Thickness
	char			szPartNumber[256];
	char			szTypeOfProducing[256];	//Type Of Producing
	char			szLotAttribute[256];	//Lot Attribute
	char			szGlassType[256];		//"N" Normal Glass, "D" Dummy Glass
	char			szGlassKeyID[256];		//Glass Key ID
	
	//for S6F63
	char			szGLSProc[256];			//PROCESSING CODE - GLSPROC_UNPROCESSED("0"),GLSPROC_PROCESSING("1"),GLSPROC_PROCESSED("2"),GLSPROC_REJECTED("3")
	
	char			szMODECODE[1];			//Host Online / Offline code
	char			szMULTIFLAG[1];			//"T" True (Multi Recipe ID), "F" False (Single Recipe ID)
	
	short			nSourceECS;				//ECS number, Glass Data from which ECS
}GLASSDATA;

typedef struct  {
	TCHAR			m_szRecipeDescription[df_MAX_RECIPE_ID];
	TCHAR			m_szRecipeID[df_MAX_RECIPE_ID];
	TCHAR			m_szPrbRecipeID[df_MAX_RECIPE_ID];
	TCHAR			m_szPPID[df_MAX_RECIPE_ID];
	TCHAR			m_szAlignRec[df_MAX_RECIPE_ID];		// jsyoon 20140701 add
}RECIPE_ID, *PRECIPE_ID;

//장비에 투입된 Glass 1장에 대한 정보
typedef struct  {
	TCHAR			m_szGlassID[MAX_GLASS_ID];
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
	int				m_nPRCID;		// jsyoon 20190927 add
	
	BOOL			m_bExist;
	int				m_nJobState;	//const enum GLASS_Job_Status 참조할 것
	double			m_dGlassSizeX;
	double			m_dGlassSizeY;
	RECIPE_ID		stRecipe;
	long			m_nGlassCode;	//DataViewer에 추가할 것 by Jay Hong : 2009.10.26

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
	
}ECS_INFO, *PECS_INFO;

typedef struct  {
	int				CellNum;		// BLOCK Count
	PointFloat		GlobalMark;
	PointFloat		CellEdig;
	PointFloat		CellDistance;
	PointFloat		CellSize;
	PointFloat		CellCount;		// CELL 
	PointFloat		CellOffset;		// BLOCK Distance
}CELLINFO, *PCELLINFO;

typedef struct  {						
	int			m_nSubPtrnNum;
	double		m_dThreshold;
	int 		m_nWhitePPAXNum[INT_SUB_PATTERN_NO_MAX+1];	
	int 		m_nWhitePPAYNum[INT_SUB_PATTERN_NO_MAX+1];
	int 		m_nUVPPAXNum[INT_SUB_PATTERN_NO_MAX+1];	
	int 		m_nUVPPAYNum[INT_SUB_PATTERN_NO_MAX+1];
	char		WhitePtrn[INT_SUB_PATTERN_NO_MAX+1][256];
	char		UVPtrn[INT_SUB_PATTERN_NO_MAX+1][256];
	CRect		WhitePtrnRect[INT_SUB_PATTERN_NO_MAX+1];				// defined Rectangle
	CRect		UVPtrnRect[INT_SUB_PATTERN_NO_MAX+1];				// defined Rectangle
}SUBPATTERN, *PSUBPATTERN;


// jsyoon 20120719 VISION 관련 recipe 정보 
typedef struct	{
	int					m_iCCDShutter;
	int					m_iCCDGain;
	int					m_iExposureTime;
	int					m_iLampIntensity;
	int					iLensIdx;
	int					iAFCch;
	int					iAfIntensity;		// jsyoon 20170405 add AF Sensor Intensity
	double				dAfOffset;
}VISION, *PVISION;

typedef struct {
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

typedef struct {
	char	m_strMCCPath[260];		
	char	m_strEquipName[260];	
	char	m_strEquipName2[260];	
}MCCINFO, *PMCCINFO;

const enum PROBE_nTEMP_ITEM{
	nTMP_WAITTIME = 0,
	nTMP_RCP_NUM,
	nTMP_ITEM_NUM,
	
	nTMP_LENS,
	nTMP_LENS_TUBE,

	nTMP_LIGHT,
	nTMP_GAIN,
	nTMP_SHUTTER,

	nTMP_GAIN_3D,
	nTMP_SHUTTER_3D,

	nTMP_LIGHT_MICRO,
	nTMP_GAIN_MICRO,
	nTMP_EXPOSURE_MICRO,

	nTMP_USE_AF,

	nTMP_OD_2STEP,
	nTMP_CM_RGB,

	nTMP_PANEL_NO,	// jsyoon 20210102 add
	nTMP_AREA_NO,	// jsyoon 20210102 add
	nTMP_ONLY_SNAP, // jsyoon 20210224 add

	nTMP_END = 100,
};

const enum PROBE_dTEMP_ITEM{
	dTMP_SHIFT_X = 0,
	dTMP_SHIFT_Y,
	dTMP_DELTA_X,		// DEMO
	dTMP_TRIGGER,		// DEMO

	dTMP_POS_SX,
	dTMP_POS_YL,
	dTMP_POS_ZL,
	dTMP_POS_SXL,
	dTMP_POS_REF,

	dTMP_END = 20,
};

typedef struct  {
	double			m_dAF;								//Auto Focus
	double			m_ds;								//Probe S좌표
	double			m_dz;								//Probe Z좌표
	double			m_dy;								//Probe Y좌표
	double			m_dx;
	double			m_dl;								//Probe L좌표
	double			m_dT;								//Probe T좌표
	int				m_nProbe;							//Probe Number
	BOOL			m_bAF;								//Auto Focus
	BOOL			m_bSelected;						//Probe 선택여부
	BOOL			m_bUsed;							//Probe Use	// Meas Skip TRUE is Skip
	BOOL			m_bOptMTP;							// Use MTP Option
	int				m_iUsedMeas;						// Use Meas Option		// ICP 4:시야각, 2:화질, 1:특성		// ICE // 1: SE, 2: RS, 	// PPA, SNAP, HOLE, OPEN-MASK		// TR 101: SAMPLE, 102:DUMMY
	int 			m_iProbeIndex;						// pwk 20101205 add.	// jsyoon 20160424 0: Review, 1:화질, 2:시야각
	int 			m_iAreaNum;							//tg.you 20130212
	TCHAR			m_CellID[32];						// Cell ID
	TCHAR			m_szModel[32];						// Model 정보 jsyoon 20180503 add
	int				m_iReason[df_MAX_PPA_NUM];			// Measure Result
	int				m_nTmp[100];
	double			m_dTmp[20];
	CMDAXISMOVE		stAxis[100];	// Multi
}PROBE, *PPROBE;

typedef struct  {
	PROBE			stProbe[df_MAX_PROBE];
	double			dGantry[df_MAX_GANTRY];	//Gantry X좌표
	double			dMoveUserOffset[100];	// User Offset
	int				nGroup;					//Group Number
	int				iGroup;					//Selected Group
}MEASGROUP, *PMEASGROUP;

typedef struct  {
	PROBE			m_stProbe[df_MAX_PROBE];
	double			m_dGantry[df_MAX_GANTRY];	//Gantry X좌표
	int				m_nGroup;					//Group Number
	int				m_iGroup;					//Selected Group
	BOOL            m_bUse;                     // TigerMoth 20151224 Add for Step Use
	int				m_iMeasMode;				// jsyoon 20201007 add Mode Offset
}MEASGROUPPOS2, *PMEASGROUPPOS2;

typedef struct  {
	int					m_iVersion;
	BOOL				m_bOption1;						//SE장비에서는 Pattern Match 실행 여부, by Jay Hong : 2009.09.24
	BOOL				m_bOption2;						//Reserved
	BOOL				m_bOption3;						//Reserved
	BOOL				m_bOption4;						//Reserved
	BOOL				m_bOption5;						//Reserved
	BOOL				m_bOption6;						//Reserved
	BOOL				m_bOption7;						//tg.you 20130115 add Bend Mode
	BOOL				m_bOption8;						//tg.you 20130227 add
	BOOL				m_bOption9;						//tg.you 20130227 add
	
	double				m_dReserve[df_MAX_RESERVE_RECIPE];//Reserved

	UINT				m_nGroupCnt;
	UINT				m_nProbeCnt;

	UINT				m_nRotate;
	UINT				m_nCellCnt;
	double				m_dProbeW;
	double				m_dProbeH;

// 	// jsyoon 20161214 add
// 	int					m_nUseProbe;
// 	char				m_szProbeID[5][MAX_RECIPE_ID];
// 	//////////////////////////////////////////////////////////////////////////

	int					m_iUseProbe;	// blue 2021.03.25 for CDlgRecipeMatrixGen

	int					m_iGenX;
	int					m_iGenY;
	
	CELLINFO			stCellInfo;

	CArray <CELLINFO, CELLINFO&>   m_aryCellGroup;		// jsyoon 20160330 add
	CArray <MEASGROUP, MEASGROUP&> m_aryMeasGroup;		//Added by Jay Hong : 2009.06.02

	CArray <MEASGROUPPOS, MEASGROUPPOS&>   m_aryMeasPosM;
	CArray <MEASGROUPPOS, MEASGROUPPOS&>   m_aryMeasPos;	// jsyoon 20190919 add	Head 1Point 
	CArray <MEASGROUPPOS2, MEASGROUPPOS2&> m_aryMeasGroup2;	// 
}RECIPEPATH, *PRECIPEPATH;

typedef struct taqResultPPA{
	BOOL   bAlgorithm;
	int	nEdgeStrength;				// pwk 20141213 add
	char szPPAName[MAX_STR_SIZE_NORMAL];
	PointFloat nresPPA[df_MAX_PPA_NUM];
	int	iMeasResult;	// 0: NotMeas, 1: OK, 2: Spc out, 3: AF, 4:Pattern
}RESULTPPADATA , *pRESULTPPADATA;

typedef struct taqResult{
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
	int nLampIntensity;				// pwk 20141213 add
	int nExposureTime;				// pwk 20141213 add
	int nCCDGain;					// pwk 20141213 add
	RESULTPPADATA stDataPPA[10];
}RESULTDATA , *pRESULTDATA;

// jsyoon 20140411 add champ state
typedef struct{
	BOOL				bOn;
	BOOL				bOff;
	long				lPos;
}CLAMPSTS, *PCLAMPSTS;

// jsyoon 20140411 add champ state
typedef struct{
	double				dS;
	double				dY;
	double				dZ;
	double				dL;
}HEADPOSINFO, *PHEADPOSINFO;

#if 0
typedef struct  {
	int					m_iVersion;					
	BOOL				m_bOptionUseAlign;				//SE장비에서는 Pattern Match 실행 여부, by Jay Hong : 2009.09.24
	BOOL				m_bOptionUseStaticAlign;		
	BOOL				m_bOptionUsePattern;			//Reserved

	//	char				m_szReserve[10][256];				//Reserved
	//	double				m_dReserve[df_MAX_RESERVE_RECIPE];	//Reserved	//1:X //2:y //3:z //:4 Intensity
	COORDINATE			stCoordAlign[df_MAX_ALIGN_MARK];	//Align Mark용. 2개 사용
	COORDINATE			stcoordRef;							//Ref Position save
	COORDINATE			stcoordDark;						//Dark Position save
	COORDINATE			stcoordSafety;						//safety Position save	
	RECIPE_ID			stRecipeID;
	UINT				m_nGroupCnt;
	CArray <MEASGROUP, MEASGROUP&> m_aryMeasGroup;			//Added by Jay Hong : 2009.06.02
	VISION				stVision[df_MAX_VISION_DLG];							// jsyoon 20120719 add
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
	double				m_dResultPos;				// pwk 20130520							
	double				m_dTopPos;								
	double				m_dBottomPos;								
	double				m_dLineDistance;								
	int					m_iPointH;					// pwk 20130529						
	int					m_iPointR;								
	int					m_iPointN;				

	double				m_dDefoSizeX[df_MAX_RESULT_DATA_CD];				// pwk 20130704							
	double				m_dDefoSizeX_Offset[df_MAX_RESULT_DATA_CD];
	double				m_dDefoSizeY[df_MAX_RESULT_DATA_CD];								
	double				m_dDefoSizeY_Offset[df_MAX_RESULT_DATA_CD];	
	double				m_dGlassSizeX[df_MAX_RESULT_DATA_CD];								
	double				m_dGlassSizeX_Offset[df_MAX_RESULT_DATA_CD];			
	double				m_dGlassSizeY[df_MAX_RESULT_DATA_CD];								
	double				m_dGlassSizeY_Offset[df_MAX_RESULT_DATA_CD];
	double				m_dPPArangeX[df_MAX_RESULT_DATA_CD];								
	double				m_dPPArangeY[df_MAX_RESULT_DATA_CD];	

	double				m_dCalibration_X[df_MAX_CALIBRATION_DATA_CD];				// pwk 20130715							
	double				m_dCalibration_Y[df_MAX_CALIBRATION_DATA_CD];
	double				m_dOffset_X[df_MAX_CALIBRATION_DATA_CD];								
	double				m_dOffset_Y[df_MAX_CALIBRATION_DATA_CD];	

	double				dReviewMeasZ;								// pwk 20130722
	double				dFirstCellPosX;								// pwk 20130722
	double				dFirstCellPosY;								// pwk 20130722
	double				dCellDistanceX;								// pwk 20130722
	double				dCellDistanceY;								// pwk 20130722
	int					iDefectCode[df_MAX_DEFECT_CNT];				// pwk 20130722
	double				dFirstProbePosX;							// pwk 20130819
	double				dFirstProbePosY;							// pwk 20130819
	double				dSecondProbePosX;							// pwk 20130819
	double				dSecondProbePosY;							// pwk 20130819
	BOOL				m_bSaveProbeImage;							// pwk 20130819							
	BOOL				m_bCellNum[df_MAX_CELL_NUM];								// pwk 20130819							

}RECIPE, *PRECIPE;
#else
typedef struct  {
	BOOL				m_bOption1;						//SE장비에서는 Pattern Match 실행 여부, by Jay Hong : 2009.09.24
	BOOL				m_bOption2;						//Reserved
	BOOL				m_bOption3;						//Reserved
	BOOL				m_bOption4;						//Reserved
	BOOL				m_bOption5;						//Reserved
	BOOL				m_bOption6;						//Reserved

	COORDINATE			stCoordAlign[df_MAX_ALIGN_MARK];//Align Mark용. 2개 사용

	RECIPE_ID			stRecipeID;
	UINT				m_nGroupCnt;
	UINT				m_nProbeCnt;

	TCHAR				m_szOffsetFileMachine[df_MAX_RECIPE_ID];
	TCHAR				m_szOffsetFileCRM[df_MAX_RECIPE_ID];
	TCHAR				m_szOffsetFileRcp[df_MAX_RECIPE_ID];
	TCHAR				m_szCellLayoutFile[df_MAX_RECIPE_ID];
	TCHAR				szAlignFile[df_MAX_RECIPE_ID];

	BOOL				m_bSendLocalFile;
	FilterRange			m_filterRangePoint[MEAS_DATA_ITEM_END];
	FilterRange			m_filterRangeCell[100];		//Cell Layout 갯수와 맞춰야 함.
	FilterRange			m_filterRangeGlass;
	RFO					m_rfoRetryFilterOffset[MEAS_DATA_ITEM_END];
	RetryParam			m_rdtryParm;						// jsyoon 20140213 change
	CArray <MEASGROUP, MEASGROUP&> m_aryMeasGroup;			//Added by Jay Hong : 2009.06.02
}RECIPE, *PRECIPE;
#endif

//================

typedef struct  {
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

typedef struct  {
	// PPA POS
	PointFloat nPPAPos;	
	PointFloat fDepo[df_MAX_PTRN_NUM];
	PointFloat fGlass[df_MAX_PTRN_NUM];
	PointFloat fPPA[df_MAX_PTRN_NUM];

	PointFloat fStenLR[df_MAX_PTRN_NUM];	// jsyoon 20141216 add
	PointFloat fStenTB[df_MAX_PTRN_NUM];

	int		   nRotate;
	int		   nCntEVEN;	// pattern cnt
	int		   nCntLTPS;	// pattern cnt
	double	   dMeas;
	double	   dFocus;
//	float	   fdistance;

	// PPA 
	SUBMEASDATAPPA LTPSDefort;
	SUBMEASDATAPPA EVENPoint[df_MAX_PTRN_NUM];
	SUBMEASDATAPPA LTPSPoint[df_MAX_PTRN_NUM];

}MEASDATAPPA, *PMEASDATAPPA;

typedef struct  {
	double x;
	double y;
}DOUBLEDATA, *PDOUBLEDATA;

typedef struct	{
	int  iPort;
	int  iBaudrate;
	int	 iDataBit;
	int  iParityBit;
	int	 iStopBit;
	int  iUseCnt;
	int  iMaxCnt;	
	int  iCtrlType;				// 0: SPO  1: CAS	2:아날로그(ACS) 3: MEI
	TCHAR szIP[MAX_STR_SIZE_NORMAL];
	TCHAR szName[8][32];		// jsyoon 20180219 add
	int  iCh[8];			// jsyoon 20210216 add
}MODULEPARAM, *PMODULEPARAM;

typedef struct  {
	int					iVersion;						// Version
	int					m_nCoordPPa;					// Probe num
	int					m_nPPACnt;						// PPA Count
	int					m_iLensIdx[INT_OBJ_LENS_USE_MAX]; // Lens	

	// PPA 
	int					m_nShotMode;
	int					m_iResultPos;	

	// SPC

	int					m_nMarginNG;					// NG Margin count
	BOOL				m_bUseSpc;						// SPC Option
 	DOUBLEDATA			m_dCalibration[df_MAX_PTRN_NUM];// PPA Calibration						
	DOUBLEDATA			m_dSPCDefoSize[df_MAX_PTRN_NUM];
	DOUBLEDATA			m_dSPCGlassSize[df_MAX_PTRN_NUM];
	DOUBLEDATA			m_dSPCPPASize[df_MAX_PTRN_NUM];
	DOUBLEDATA			m_dSPCDefoLimit[df_MAX_PTRN_NUM];
	DOUBLEDATA			m_dSPCGlassLimit[df_MAX_PTRN_NUM];

	VISION				stSetCCD;						// CCD Settiong
	CPtrnInfo			ptrnMeas;						// PPA Align Makr
		
//	CArray <MEASDATAPPA, MEASDATAPPA&> m_aryPPAPattern;

}RECIPEPPA, *PRECIPEPPA;


//jw.lee 20190306 add
typedef struct  {
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
//end
typedef struct  {

	BOOL		m_bUsed;

	// PPA POS
	PointDouble nPPAPos;	// PPA 위치
	int			nRotate;	// PPA 방향
	int			nLens;		// 측정 Lens
	double		dMeasZ;		// Z축 위치
	int			nPtrnPosNum;	//jw.lee 20190207 add, PPA 패턴 위치 번호

	// CELL ID
	TCHAR   m_CellID[32];	

	// Model
//	TCHAR szModel[10][MAX_STR_SIZE_NORMAL];	// 0: MT, 1: REAL, 2:PPA
	TCHAR szModel[10][128];	// 0: MT, 1: REAL, 2:PPA		// jsyoon 20190716 change

}MEASPARAMETER, *PMEASPARAMETER;

typedef struct  {

 	UINT	m_nSubParamCnt;
 	MEASPARAMETER m_nSubParm[df_MAX_PPA_NUM];			// 20개까지 설정가능하게 하자		// jsyoon 20180430 50까지 우선 늘려놓자 

}MEASSUBGROUP, *PMEASSUBGROUP;

typedef struct  {

	UINT		  nSubGroup;		// Group 번호
	MEASPARAMETER stParam;
	CMDAXISMOVE	stAxis[100];
	short		sModel;			//jw.lee 20190306 add

}MEASHEADGROUP, *PMEASHEADGROUP;

typedef struct  {
	int		m_iVersion;	
	int		iLight[20];
	BOOL	bUse[20];			// 0: 사전검사, 1:전기검사,2:특성,3:화질,4:색이상,5:MTP,6:PPA	13:R-TEST PASS,14:샘플링검사,15:ProbeAF,16:Align,17:LineScan,18:ReviwSnap,19: MTP
	int		nTimeout[20];		// 0: 사전검사, 1:전기검사,2:특성,3:화질,4:색이상,5:MTP,6:PPA
	double	dCentactZ[20];		// User Offset X:11, Y:12, R:13

	int		iTmp[100];			// 0:Filter, 
	double	dTmp[100];			// 0:User Gap 

	PointFloat	CellCount;		// CELL 

	// jsyoon 20161214 add
	int		m_nUseProbe;
	char	m_szProbeID[5][MAX_RECIPE_ID];
	//////////////////////////////////////////////////////////////////////////

}RECIPEICP, *PRECIPEICP;

// ICE RECIPE DLL 에서 MAIN으로 전달하기위한 구조체
typedef struct  {
	UINT	iLight[20];
	UINT	nTimeout[20];
	double	dCentactZ[20];		// RS
	UINT	iTmp[100];			// 0:RS High, 1:RS Low,
	double	dTmp[100];			// 0:User Gap 
	UINT	m_iSelHead;			
	UINT	m_iMeasType;		// 0:SE, 1:RS, 2:PPA
	MEASPARAMETER m_stMeasParam;
}RECIPEMEASICE, *PRECIPEMEASICE;

// 패턴 매칭을 위한 구조체
typedef struct  {
	VISION		stSetCCD;		// CCD 정보
	CPtrnInfo	m_ptrnInfo;		// 패턴 정보
}RECIPEPATTEN, *PRECIPEPATTEN;

struct RECIPEMEAS_POS{

	double	dTmp[100];			// 0:User Offset	91:PPA SPC

	int		m_nColMax;
	int		m_nRowMax;
	int		m_nRotate;
	int		m_nSubGrup;				// Sub Group 개수
		
	CArray <MEASGROUP, MEASGROUP&> m_aryMeasGroup;					// WT 이동 
	CArray <MEASSUBGROUP, MEASSUBGROUP&> m_aryMeasSubGroup;			// Sub 이동
	CArray <MEASHEADGROUP, MEASHEADGROUP&> m_aryMeasHeadGroup;		// HEAD 이동

	RECIPEMEAS_POS()	{
		void		ClearInfo();
	}

	// y.yun 20190303 add
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
	//////////////////////

	void ClearInfo () {
		int i=0;
		for(i=0; i<100; i++)
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

struct  THICK_MEAS{
//	ThickMeasure
int		m_nITime;
int		m_nAvg;
int		m_nBoxcar;
int		m_nRefExposureTime;	
};

struct  RECIPEMEAS{
	UINT	m_iVersion;	
	UINT	iLight[21];			// seoh.choi 20181218 change [20] -> [21]

	//	ThickMeasure
	THICK_MEAS stThick[10];

	// ICE						//	  2:특성,3:화질,4:색이상,5:MTP,6:PPA,7:SE,8:RS,9:SR,10:TR,11:8K			16:Align,17:Pattern
	//BOOL	bUse[20];			// 0: SE, 1:RS,2:PPA,		16:Align,17:Pattern
	BOOL	bUse[21];			//jw.lee 20181214 change
	//UINT	nTimeout[20];		// OCP 2:특성,3:화질,4:색이상,5:MTP,6:PPA,  ICE 0:SE, 1:RS, 2:PPA
	UINT	nTimeout[21];		//jw.lee 20181214 change
	// jw.lee 20190317 add.
	UINT	nPercentDefect[2];	// 0: Mixed Color, 1:Characteristic
	BOOL	bUsePercentDefect[2]; // 0: Mixed Color, 1:Characteristic
	//
	double	dCentactZ[21];		// RS		// seoh.choi 20181217 change 20 -> 21

	BOOL	bTmp[100];			// 
	UINT	iTmp[100];			// 0:RS High, 1:RS Low,
	double	dTmp[100];			// 0:User Gap 

	PointFloat	CellCount;		// CELL		// Operate DLL 사용

	// ALIGN
	TCHAR	szRcpAlign[MAX_STR_SIZE_NORMAL];


	// PROBE ID -> USE ICP, OCP  
	int		m_nUseProbe;
	TCHAR	m_szProbeID[5][MAX_RECIPE_ID];
	//////////////////////////////////////////////////////////////////////////

	UINT	m_nSubGroupCnt;
	CArray <MEASSUBGROUP, MEASSUBGROUP&> m_aryMeasSubGroup;

	RECIPEMEAS()	{
		void		ClearInfo();
	}

	void ClearInfo () {
		m_iVersion	= 1;
		m_nSubGroupCnt = 0;

		//jw.lee 20190412 add
		for(int i = 0; i < 2; i++)
		{
			nPercentDefect[i] = 0;
			bUsePercentDefect[i] = 0;
		}
		//

		int i=0;

		for(i=0; i<10; i++)
		{
			stThick[i].m_nITime = 10;
			stThick[i].m_nAvg = 5;
			stThick[i].m_nBoxcar = 3;
			stThick[i].m_nRefExposureTime = 1000;
		}
			
		//for(i=0; i<20; i++)
		for(i=0; i<21; i++) //jw.lee 20181214 change
		{
			iLight[i]= 0;
			bUse[i] = FALSE;
			nTimeout[i] = 60;
			dCentactZ[i]= 0;		// RS, PROBE
		}

		for(i=0; i<100; i++)
		{
			bTmp[i] = FALSE;
			iTmp[i] = 0;
			dTmp[i] = 0.0;
		}

		_tcscpy_s(szRcpAlign, _countof(szRcpAlign), _T(""));

		m_nUseProbe = 0;

		for(i=0; i<5; i++)
		{
			_tcscpy_s(m_szProbeID[i], _countof(m_szProbeID[i]), _T("PROBEID"));
		}

		m_aryMeasSubGroup.RemoveAll();
	}
};

struct RECIPERTRN {
	int					m_iVersion;

	int					m_iIntensityTop;
	int					m_iIntensityBottom;

	int					m_iLens;
	int					m_iCCDGain;
	int					m_iExposureTime;
	int					m_iAfch;

	CPtrnInfo			ptrnInfo[df_MAX_PTRNINFO];

	RECIPERTRN() {
		void ClearInfo();
	}

	void ClearInfo()
	{
		m_iVersion = 1002;	// jsyoon 20180307 1000 to 1001

		m_iIntensityTop = 0;
		m_iIntensityBottom = 0;

		m_iLens = 0;
		m_iCCDGain = 0;
		m_iExposureTime = 100;
		m_iAfch = 0;

		int i = 0;
		for (i = 0; i<df_MAX_PTRNINFO; i++)
		{
			ptrnInfo[i].ClearPtrn();
			ptrnInfo[i].ClearInfo();
		}
	}
};

// ICP 사용 구조체, 패턴이미지 필요없음
typedef struct  {
	int					m_iVersion;	
	BOOL				bUse4PAlign;		// 4Point Align
	TCHAR				szName[MAX_STR_SIZE_NORMAL];
//	int					m_iLensIdx[INT_OBJ_LENS_USE_MAX]; // Lens	
	int					iLight[20];		// ICP CCD 
	VISION				stSetCCD;
	COORDINATE			stCoordAlign[df_MAX_ALIGN_MARK][df_MAX_PROBE];	// Align Mark용. 2개 사용
	COORDINATE			stoffsetAlign[4];								// Aligner Recipe
	COORDINATE			stCoordBase[df_MAX_ALIGN_MARK][df_MAX_PROBE];	// Base
	double				dPosCard[10];	// Probe Card AF Position
	double				dPosAlign[50];	// Glass Align Position
	CMDAXISMOVE			stAxis[2][100];	// 0:Probe, 1:Glass Align Position	// jsyoon 20180307 add 리뷰로 얼라인하니까 동일축이 필요해서 새로 추가함
	int					nBuf[100];
	double				dBuf[100];
	int					iMeasMode;		//sklee 20201227 add

}RECIPEALIGN, *PRECIPEALIGN;

// ICE 사용 구조체, 패턴이미지 필요함
struct RECIPEALIGN2  {
 	int					m_iVersion;	
	int					m_iDetect;

	RECIPEALIGN			stRecipeAlign;

	RECIPERTRN			m_RecipePtrn;
//	CPtrnInfo			ptrnAlign[df_MAX_PTRNINFO];

	RECIPEALIGN2 (){
		void ClearInfo();
		void ClearAlignInfo();
	}

	void ClearInfo()
	{
 		m_iVersion = 1001;	// jsyoon 20180307 1000 to 1001

		m_iDetect = 0;

		ZeroMemory(&stRecipeAlign, sizeof(stRecipeAlign));

		stRecipeAlign.m_iVersion = 1000;
		stRecipeAlign.bUse4PAlign = FALSE;

//		_tcscpy_s(stRecipeAlign.szName, _T("Default"));

		m_RecipePtrn.ClearInfo();

//		int i=0;
//		for (i=0; i<df_MAX_PTRNINFO; i++)
//		{
//			ptrnAlign[i].ClearPtrn();
//			ptrnAlign[i].ClearInfo();
//		}
	}

	void ClearAlignInfo()
	{
		ZeroMemory(&stRecipeAlign, sizeof(stRecipeAlign));
	}
};

typedef struct struct_RECIPE_CD
{
	BOOL			bUseMeasLowLED;
	int				iSetLowLED;
	int				iFilterNo;
	BOOL			bUseAutoSnap;					//sklee 20210125 add

} RECIPE_CD;

typedef struct struct_RECIPE_ALC
{
	BOOL			bUseALC;
	int				nALCRetry;
	int				nALCAddValue;
	int				nALCDelayTime;
	double			dALCRange;
	double			dALCBaseLight;

} RECIPE_ALC;

typedef struct struct_RECIPE_D
{
	int				nExposeInmSec;

	int				iCCDGain;
	int				iCCDGain2;

	int				iExposureTimeRef;
	int				iExposureTimeRefLow;
	int				iExposureTimeRef2;		// OD 2Step
	int				iExposureTimeRefLow2;	// OD 2Step

	int				iExposureTimeMeas;
	int				iExposureTimeMeasLow[4];

	int				iIntensityMain;
	int				iIntensityTR;
	int				iIntensityOD;
	int				iIntensityRS;
	int				iIntensityCD;
	int				iIntensityCDLower[4];

	int				iAFLamp;
	double			dRefAFPosUpper;
	double			dRefAFPosLower;

	// SPC REF
	BOOL			bUseSPCRef;
	double			dMinSPCRef_Light;
	double			dMaxSPCRef_Light;
	double			dMinSPCRef_High;
	double			dMaxSPCRef_High;
	double			dMaxSPCRef_nTime;

	// ALC
	RECIPE_ALC		stALC;

} RECIPE_D;


typedef struct struct_RECIPE_CM
{
	BOOL			bUseBMShift;
	double			dBMShifhX;
	double			dBMShifhY;

	double			dStepOffsetX[4];
	double			dStepOffsetY[4];

	int				iMeasPromary;
	BOOL			bMeasCell[4];
	BOOL			bMeasCellAF[4];
	BOOL			bMeasCellCentering[4];

} RECIPE_CM;

typedef struct struct_RECIPE_TK
{
	double			dStepOffsetX[4];
	double			dStepOffsetY[4];

	int				iMeasPromary;
	BOOL			bMeasCell[4];
	BOOL			bMeasCellAF[4];
	BOOL			bMeasCellCentering[4];

	BOOL			bUseMeasRGB;

} RECIPE_TK;

typedef struct struct_RECIPE_OD
{
	BOOL			bUseTwoStep;
	

} RECIPE_OD;

typedef struct struct_RECIPE_RS
{
	double			dCentactZ[2];		// 0: Ready, 1: Meas
	int				CurrentSrc[2];		// 0: High,  1: Low
	int				iSetMode;			// 0: single    1: mono   2: dual    3: bulk		

	int				nSfiftCount;
	double			dShiftOffsetX;
	double			dShiftOffsetY;

} RECIPE_RS;

struct RECIPEROI {
	int					m_iVersion;

	BOOL				m_bUsePtrnMatch;
	int					m_iRoiStartX;
	int					m_iRoiStartY;
	int					m_iRoiWidth;
	int					m_iRoiHeight;

	int					m_iMeasMode;
	int					m_iMeasBS;

	RECIPERTRN			m_PtrnInfo;

	RECIPE_D			m_RCP;
	RECIPE_CD			m_CD;
	RECIPE_CM			m_CM;
	RECIPE_TK			m_TK;
	RECIPE_OD			m_OD;
	RECIPE_RS			m_RS;

	RECIPEROI() {
		void ClearInfo();
	}

	void ClearInfo()
	{
		m_iVersion = 1001;	// jsyoon 20180307 1000 to 1001

		m_bUsePtrnMatch = FALSE;
	
		m_iRoiStartX = 0;
		m_iRoiStartY = 0;
		m_iRoiWidth  = 640;
		m_iRoiHeight = 480;

		m_iMeasMode  = 0;
		m_iMeasBS = 0;

		m_PtrnInfo.ClearInfo();

		ZeroMemory(&m_RCP, sizeof(m_RCP));
		ZeroMemory(&m_CD, sizeof(m_CD));
		ZeroMemory(&m_CM, sizeof(m_CM));
		ZeroMemory(&m_TK, sizeof(m_TK));
		ZeroMemory(&m_OD, sizeof(m_OD));
		ZeroMemory(&m_RS, sizeof(m_RS));
	}
};


//==============================================================================


//Main View의 Current Position display에 표시될 내용 - 공유메모리에서 사용될 예정=====
typedef struct  {
	
	BOOL			bUse;			//장치 사용여부, UI상에서도 보이지 않게 처리함. OutOfService와는 다름	//Added by Jay Hong : 2009.08.24
	BOOL			bConnect;		//연결 상태
	BOOL			bOutOfService;	//모듈 사용 여부 0: USE
	BOOL			bInitOk;
	BOOL			bRunInit;		//Initialize를 할 것인지 여부. False인 경우 Init 하지 않음
	int				nTypeNum;		// 연결 Type 설정
	

	double			dMoveOffset;		// Line CCD 

	double			m_dx;			//Prober의 위치 x
	double			m_dy;			//Prober의 위치 y
	double			m_ds;			//Prober의 위치 s
	double			m_dz;			//Prober의 위치 z
 	double			m_dLP;			//Prober의 위치 Lift Pin
	double			m_dLP2;			//Prober의 위치 Lift Pin
	double			m_dRJ;			//Prober의 위치 
	double			m_dR;			//Prober의 위치 Rotate

	BOOL			bWaitORG;		//Only for xCIM
	
	//Safety Position
	/*
	double			m_dSafety_X;	// TOP
	double			m_dSafety_Y;	// TOP
	double			m_dSafety_Y1;	// BOTTOM Align left
	double			m_dSafety_Y2;	// BOTTOM Align Right
	double			m_dSafety_Z;
	double			m_dSafety_Z2;
	*/

	double			m_dSafety[100];
	double			m_dUnitOffset;
	double			m_dLowLedOffset;

	/*
	//Lift Pin Position					
	double			m_dPinUpPosition;
	double			m_dFixtureOffPosition;
	double			m_dPinDownPosition;
	double			m_dLPDownPosition;

// 	double			m_dRINPosition;
// 	double			m_dROUTPosition;
	double			m_dLoadPos[3];		// X, Y, R
	double			m_dUnLoadPos[3];	// X, Y, R
	double			m_dMeasPos[3];		// X, Y, R		// jsyoon 20171006
	*/
	
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
	BOOL			m_bDetect_Spot;		//Added by Jay Hong : 2009.06.19
	BOOL			m_bDetected_Meas;
	BOOL			m_bDetected_Align;
	float			m_fScore;	

	// Error
	int				m_nError;			// Measure skip
	int				m_nReason;			// Not Use

	BOOL			m_bDisPlacementSensor;
	TCHAR			m_szEqState[1];

	BOOL			m_bUseLifePin;
	BOOL			m_bInvertXY;
	CLAMPSTS		bChampState[15];				// jsyoon 20140411 add
	CLAMPSTS		bOldChampState[15];				// jsyoon 20140411 add
	BOOL			m_bUseClampInterlock;			// jhseo 20140603

	// jsyoon 20180126 add
	//SpecTro
	//	double			m_dVSWaveCoeff[4];	//ksk 20150902 add
	double			m_dVSSpotDiaUM;
	BOOL			m_bMeasureDone;		// 측정 완료
	BOOL			m_bComputeDone;		// 계산 결과완료
	BOOL			m_bUseMode[2];		// SR / TR

	int				m_iSpotCx;
	int				m_iSpotCy;
	int				m_nGroup;			//sklee 20200106 add

	TCHAR			szName[MAX_STR_SIZE_NORMAL];		// UI 표시
	MODULEPARAM     stModuleParam;	// TCP / RS-232
//	HEADPOSINFO		strHeadPosInfo[df_MAX_PROBE];	// 2Head Pos

	int				m_nWDITrkMode;	//WDI 트래킹 모드			// hykim 20210201 add WDI Tacking Mode
	int				m_nWDISucess;	//WDI 모드 변환 완료

}SYSTEM, *PSYSTEM;
//====================================================================================


typedef struct  {
	char	m_szProbeCardID[32];
	char	m_szProbeCardIDTime[32];
	UINT	m_nUseCnt;
}PROBEINFO, *PPROBEINFO;

typedef struct  {
	TCHAR	szMonitor[MAX_STR_SIZE_NORMAL];
	UINT	m_nProcess;
}MONITOR, *PMONITOR;

typedef struct  {
	BOOL	bUse;
	BOOL	bEnable;
	BOOL	bLimitP;	// +
	BOOL	bLimitN;	// -
	BOOL	bHomeDone;
	double	dSetAxisSpeed;	// Set Speed
	double	dAxisSpeed;		// Current Speed
	double  dSetAxisPos;	// Set position
	double	dAxisPos;		// Current position
	double	dAxisRMS;		// 부하률
}MOTIONSTATUS, *PMOTIONSTATUS;

//Main View의 Panel display에 표시될 내용 - 공유메모리에서 사용될 예정===============
typedef struct  {
	TCHAR			m_szLotID[64];
	TCHAR			m_szGlassID[64];
	TCHAR			m_szHostPPID[64];		// jsyoon 20190927 add
	TCHAR			m_szSlotID[64];
	TCHAR			m_szRecipeID[64];
	TCHAR			m_szStepID[64];
	TCHAR			m_szGlsRotate[64];
	TCHAR			m_szCassetteID[64];
	TCHAR			m_szOperID[32];
	TCHAR			m_szSEModelID[32];			// SE Run Recipe // Sub Recipe
	TCHAR			m_szGlassCoordiType[32];	//ksk 20150518 add
	TCHAR			m_szEQPID[32];				//20170725 kks	// jsyoon 20180131 add
	TCHAR			m_szInnerID[df_MAX_PROBE][32];
	TCHAR			m_szCellID[df_MAX_PROBE][32];
	TCHAR			m_szSubRecipe[df_MAX_PROBE][df_MAX_RECIPE_ID];	// 별로레시피 사용하는 모듈
	TCHAR			m_szAlignRec[df_MAX_RECIPE_ID];	// Align recipe
	TCHAR			m_szGlassID_Half[32];		//lhj 20190324 add
	BOOL			m_bBufferMode;				// jsyoon 21050916 add LGD AP3 Buffer Glass Use
	int				m_nBufferCycle;				// LGD AP3 Buffer Glass Cycle Count

	TCHAR			m_szMeasStartTime[32];		// 측정 시작 시간
	TCHAR			m_szMeasEndTime[32];		// 측정 완료 시간
	CTimeSpan		m_timeMeasTactTime;

	TCHAR			m_szResultID[df_MAX_PROBE][32];
	TCHAR			m_szResultIMG[df_MAX_PROBE][260];

	TCHAR			m_szProcessID[64];			//20210214 jychoi add
	TCHAR			m_szProductID[64];          //wb 20211008 add
}LOTINFO, *PLOTINFO;
//====================================================================================

typedef struct  {
	double			dAVGSignal;
	double			dDUVSignal;	
	double			dUVSignal;
	double			dIRSignal;
}RESULT_SE_SIGNAL, *PRESULT_SE_SIGNAL;

typedef struct  {
	RESULT_SE_SIGNAL stSignal[5];	// 1:2, 2:10, 3:25, 4:60

	double			dAlignX;
	double			dAlignY;
	double			dIntensity;
	double			dAngle;
	double			dEMA;                
	char			szSEMeasModel[256];
	double			dBT[INT_LAYER_NO_MAX];	 // Bottom Layer
}RESULT_SE_INFO, *PRESULT_SE_INFO;

typedef struct  {
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


typedef struct  {
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

//System InterLock Check Added by Jay Hong : 2009.07.03
typedef struct  {
	BOOL			m_bLampRedOn;
	BOOL			m_bLampYellowOn;
	BOOL			m_bLampGreenOn;
	BOOL			m_bLampBlueOn;
 	BOOL			m_bDetectedVacuumSensor;		// Chuck Glass 흡착 확인
	BOOL			m_bIsVacuumOnSensor;			// Vacuum On/Off 확인

	BOOL			m_bDetectedTTSensor[5];		// pwk 20121128	Temperature	// jsyoon 20150302 add 5
	BOOL			m_bMCDown;
	BOOL			m_bCPDown;

	BOOL			m_bUseAlarm[df_USE_MAX_ALARM];		// Config Read
	BOOL			m_bSetAlarm[df_USE_MAX_ALARM];		// PLC Read
	int				m_nCntAlarm[df_USE_MAX_ALARM];		// Count

	BOOL			m_bUseCheckLoadState;
	BOOL			m_bUseCheckMeasureState;
	BOOL			m_bUseCheckReadyState;

	BOOL			m_bOldLockKeyAutoMode;		// pwk 20131007
	BOOL			m_bOldDoorLockMode;			// jsyoon 20171113 add
	BOOL			m_bLockKeyAutoMode;			// jsyoon 20171113 add

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
	double			dChuckVacuumValue[10];		// Chuck Vacuum
	double			dFixtureTime[16];			// FixtureTime

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
	BOOL			m_bRSCylinderUp;		// READY
	BOOL			m_bRSCylinderDown;		// MEAS

	// UV
	BOOL			bUVSignalOn;				// jsyoon 20150409 add


	BOOL			m_bDoorUnLockMode;

	// PROBE
	BOOL			m_bProbeBufferExist[2];		// Buffer#1, Buffer#2
	BOOL			m_bProbeBufferUp[2];
	BOOL			m_bProbeBufferDown[2];
	BOOL			m_bProbeCardExist[10];

	// Grip
	BOOL			m_bUseGripSW;	// USE Grip switch

	BOOL			m_bArmDetected;
	BOOL			m_bArmDetectedAlarm;

	BOOL			m_bUserManualOn;	// jsyoon 20170105 add

	BOOL			m_bMBit[1000];		// 인터락사용

	// jsyoon 20171115 add
	//Ionizer //20170516 add by Hee
	BOOL			m_bIonizerUse;
	BOOL			m_bIonizerAutoOff;			   //설정 시간 이후 자동 Off 기능
	BOOL			m_bIonizerOn;				   //PLC Ionizer IN IO
	BOOL			m_bIonizerSet[IONIZER_USE_MAX];//Ionizer On 시점
	int				m_nIonizerDelay;			   //설정 시간
	int				m_nIonizerCycleCount;		   //TEST 반복 횟수
	int				m_nIonizerProcCount;		   //완료 횟수
	
}SYSINTERLOCK, *PSYSINTERLOCK;
//====================================================================================

//Title Window에 표시될 내용
typedef struct {
	TCHAR		m_szUserPermit[64];
	TCHAR		m_szUser[64];
	int			m_nHostMode;	// 현재 상태
	int			m_nOldHostMode;	// 이전 상태
	BOOL		m_bHostConnect;
	int			m_nLPMode;
	int			m_nUPMode;
	TCHAR		m_szProcessStatus[64];
	TCHAR		m_strEquipNum[64];
	TCHAR		m_strEquipName[260];
	TCHAR		m_szCurTime[32];		// %04d.%02d.%02d-%02d:%02d:%02d:%03d
	TCHAR		m_szCurTime2[32];		// %04d%02d%02d%02d%02d%02d
	TCHAR		m_szCurTime3[32];		// HHMMSS %02d%02d%02d
	TCHAR		m_szCurDay[32];			// MMDD   %02d%02d
}TITLEPANEL, *PTITLEPANEL;

typedef struct  {
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
	TCHAR		m_szUserPermit[64];
	TCHAR		m_szUser[64];
	int			m_NoUser;
	UINT		m_nEQSubType;
	BOOL		m_bOption[10];						//Reserved 
	double		m_dOption[10];						//Reserved
	TCHAR		m_szSysMemory[256];			//20170524 add by Hee		// jsyoon 20171115 add
	TCHAR		m_szMetrosysVersion[256]; //lhj 20190324 add
	BOOL		m_bSystemInitOk;
	int			m_nMachineStatus;
} EQUIP, *PEQUIP;

typedef struct  {
	// Current Position
	COORDINATE		stCurPos;

	//Safety Position
	COORDINATE		stSafetyPos;

	//Reference Position
	COORDINATE		stReferencePos;

	//Dark Position
	COORDINATE		stDarkPos;

	// Measure Position
	COORDINATE		stMeasPos;

	// CRM
	COORDINATE		stCRMPos;

	// SPOT
	COORDINATE		stSpotPos;

	// Head
	COORDINATE		stGetPos;	// Head Get Position

	// Axis
	double			dCurPos[100];	// For STCM

}POSINFO, *PPOSINFO;


typedef struct  {
	TCHAR		m_szRootPath[MAX_PATH];
	TCHAR		m_szResultPath[MAX_PATH];
	TCHAR		m_szLogPath[MAX_PATH];
	TCHAR		m_szConfigPath[MAX_PATH];
}PATHINFO, *PPATHINFO;


#define df_MAX_READ		64
#define df_MAX_WRITE	64

//20170802 add by Hee (Purifier, CDA, N2)
#define PURGE_PURIFIER	0
#define PURGE_CDA		1
#define PURGE_N2		2
#define PURGE_END		3


typedef struct  {
//	char					szEquipName[256];			//Added by Jay Hong : 2009.08.24
	int						nMeasMode;					// 

//	char					m_szModuleID[MAX_PATH];		
	BOOL					m_bVirtualMode;

	HWND					hLog;
	HWND					hMetrosysView;			
	HWND					hCDlgNaviOperate;	

	SYSTEM					stSystem[MAX_SUBMODULE];	//SubModule 개수. 현재는 Gantry, Probe1, Probe2, SocketTest
	MONITOR					stMonitor;
	LOTINFO					stLotInfo;
	TITLEPANEL				stTitle;
	SYSINTERLOCK			stInterLock;				//System InterLock Checking, Added by Jay Hong : 2009.07.03
	ECS_INFO				stGlassInfo;				// 사용안함
	RESULT_INFO				stResult;	
	MEASGROUP			    stMeasGrp;
	EQUIP					stEquip;
	MOTIONSTATUS			stMotionStatus[100];			// jsyoon 20160216 add Motion Info
	PATHINFO				stSysPath;
	

	// Process Measure
	char					szProductID[32];					// 20190112 kks
	char					szLastGlassID[256];
	char					szLastRcpName[256];
	char					szLastMeasTime[256];

	int						iCurGroup;					// 현재 측정 Group
	int						nLastStepT;					// 마지막 측정 횟수
	int						nLastStepB;					// 측정 STEP		// nRealMeasAreaCnt
	int						nLastMeasMode;
	int						nJobCount;					// 총 Cycle 횟수
	int						nCurrentCycleCount;			// 현재 Cycle 횟수
	
	BOOL					m_bMeasMultiRcp;			// Multi recipe 
	BOOL					m_bBufferMode;				// Buffer Glass Measure for LGD

	int						m_dSystemErrCode;
	double					m_dJobGrpCnt;				// 측정 Group 개수
	BOOL					m_bDarkMeas;				// CS Measure Dark Measure use
	BOOL					m_bReWork;					// Re-Measure
	int						nReMeasureTotalCount;		// Re-Measure 
	BOOL					m_bGlassJudge;
	BOOL					m_bGlassLoadState;			
	int						iMachineStatus;	
	BOOL					m_bUseGlassAbrot;			//ksk 20150612 add
	BOOL					m_bExchangeComplete;		// jdm 20190205 add.
	BOOL					m_bForceChangeMode;			// Online 으로 변경

	// Cycle Test
	int						m_nShiftSkip;				// 0: 좌표보정 모드 
	//////////////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////////////
	double					dDisplacement_X;		//틀어진 만큼의 좌표값
	double					dDisplacement_Y;		//틀어진 만큼의 좌표값
	double					dDisplacement_Z;		//틀어진 만큼의 좌표값
	double					dDisplacement_S;		//틀어진 만큼의 좌표값
	
	double					dDisplacement_Angle;
	double					dNewTarpos_X;			//새롭게 보정된 좌표값
	double					dNewTarpos_Y;			//새롭게 보정된 좌표값
	
	// Align
	COORDINATE				stRecipeAlignMarkPos[MAX_ALIGN_MARK][df_MAX_PROBE];	//Align Mark용. 2개 사용
	COORDINATE				stDetectAlignMarkPos[MAX_ALIGN_MARK][df_MAX_PROBE];	//Align Mark용. 2개 사용
	COORDINATE				stStaticAlignMarkPos;
	BOOL					m_bAlignMarkDetected;
	BOOL					m_bAlignMarkDetectedPos[df_MAX_ALIGN_MARK];
	BOOL					m_bAlignMarkDetected2[MAX_ALIGN_MARK][df_MAX_PROBE];
	BOOL					m_bAlignContinue;
	int						m_nAlignMarkNum;				// pwk 20130627
	BOOL					m_bSkipTarget;
	double					m_dAlignOffset[5];				// Glass가 틀어짐을 보상
	double					m_dAlignResult[3];				// Probe가 틀어짐을 보상
	//////////////////////////////////////////////////////////////////////////

	
	// MCC
	MCCINFO					stMCCInfo;
	BOOL					m_bProcessComplete;		//각가의 서브 모듈들(Probe1, Probe2... 측정모듈들) 전부 끝났을 경우
 	BOOL					m_bMccTransferOn;
	double					m_iMCCUploadTime;
	//////////////////////////////////////////////////////////////////////////

	// Motion
	POSINFO					stPosInfo[df_MAX_PROBE];
	double					m_dMoveGap[100];

	double					m_dRSOffset_X;			// pwk 20130521
	double					m_dRSOffset_Y;			// pwk 20130521

	double					dAxisZMax[df_MAX_PROBE];
	double					dAxisZMin[df_MAX_PROBE];

	BOOL					m_bAxisMoving;			// pwk 20130730 
	//////////////////////////////////////////////////////////////////////////

	// PLC and CCLINK
	BOOL					bCCLinkIn[df_MAX_READ];
	BOOL					bCCLinkOut[df_MAX_WRITE];

	BOOL					m_bPLCData[dfMAX_PLC_IN_CNT];
	BOOL					m_bBFPLCData[dfMAX_PLC_IN_CNT];
	//////////////////////////////////////////////////////////////////////////

	// RS
	int						m_iRsContactNum;		// RS Contact 누적횟수

	double					m_dMeasx;				//Prober의 위치 x
	double					m_dMeasy[df_MAX_PROBE];	//Prober의 위치 y

	double					m_dGlobalX;
	double					m_dGlobalY;

	// PPA
	int						iShotMode;
	int						m_iMeasPPANum[df_MAX_PROBE];	// 측정 중인 PPA 순서
	char					m_szPPAImage[256];		// pwk 20130529
	char					m_szReviewImage[256];	// pwk 20130722

	BOOL					m_bUsableOtherPPA;		// jsyoon 20150224 add
	int						m_iTEG_W;
	int						m_iTEG_H;
	VTM_PPA*				m_pOtherPPA;			// jsyoon 20150223 add 
	//


	// LO
	PROBEINFO				stProbeInfo;				// LO PROBE CARD Info
	int						nCellStatus[100][50];			// jsyoon 20160513 add
	int						nBFCellStatus[100][50];			// jsyoon 20160513 add
	BOOL					m_bProbeExistError;		// eh.kim 20190225 add.
	//////////////////////////////////////////////////////////////////////////
	
	BOOL					m_bUseSnapCellEdge;		// jsyoon 20150411 add
	BOOL					m_bUseSnapCellEdgeSecond;	//ksk 20150506 add

	BOOL					m_bUseDataUpLoad;	//ksk 20150518 add
	BOOL					m_bUseSecondDataUpLoad;	//ksk 20150518 add
	BOOL					m_bUseImageSizeChange;	//ksk 20150518 add
	
	BOOL					m_bValidation;			//ksk 20150727 add
	BOOL					m_bCIMAlarmReported;	//ksk 20150727 add	

	BOOL					m_bUseClamp[6];			//ksk 20150917 add
	UINT					m_nCurLIgnt[5][8];

	// 
	double					dRotatePos[3];	//ksk 20160813 add	// Loader Align 보정값 챔버장비만 사용
	BOOL					m_bUseLdAlign;
	//////////////////////////////////////////////////////////////////////////

	//lhj add 161115 //
	int						m_nApdOpt[6];
	int						m_nDefectCnt;
	//

	// RACK Jack
	int						m_iRJSleepTime[4];
	int						m_iRJSpeed[4];			// 1: First, 2:Second, 3:Third
	double					m_dRJPosition[4];		// 1: First, 2:Second, 3:Third, 0:Down
	double					m_dRJMoveTact[4];		// 1: First, 2:Second, 3:Third, 0:Down
	//////////////////////////////////////////////////////////////////////////
	
	// jsyoon 20171113 add
	// 20170823 add by Hee 
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
	BOOL					m_bUseN2PurgeRecipe;			// seoh.choi 20181217 add

	// jsyoon 20171115 add
	//20170929 add by Hee
	BOOL					m_bAutoAgingMode;
	int						m_nAgingListRow;
	int						m_nAgingListCol;
	int						m_nGroup;					//sklee 20210106 add
	int						m_nLanguage; //lhj 20181225 lang

	BOOL					m_bReWorkUseLogin;		//20210612 jychoi

}VTM_SYSTEM, *PVTM_SYSTEM;

//lhj 20181225 lang
#define LANGUAGE_NONE					-1
#define LANGUAGE_KOREA					0
#define LANGUAGE_ENGLISH				1
#define LANGUAGE_CHINA					2
//

#define RY_SIG_TOWER_RED				0		   // RY
#define RY_SIG_TOWER_YELLOW				1
#define RY_SIG_TOWER_GREEN				2
#define RY_SIG_TOWER_BLUE				3
#define RY_SIG_TOWER_BUZZER				4
#define RY_RSPICO_UPDOWN				5
#define RY_CCD_POWER					6
#define RY_UV_LAMP_SHUTTER_ONOFF		7
#define RY_WHITE_LAMP_SHUTTER_ONOFF		8
#define RY_EMO_SW_OUTPUT				9


#define RX_AREA_SENSOR_01				0			// RX	
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
		

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// System Status
//#define	WBS_SYSTEM_SHARE_MEM_STRING	"WBS_M_System_Shared_Memory_TopsoL"


#endif
