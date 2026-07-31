#pragma once

#include "..\WindowEventHandler.h"
#include "..\Manager\ShareMemoryMgr.h"
#include "..\Include\API_RECIPE.h"

struct stMeasAreaItem
{
	CString strName;
	int		iWaitTime_sec;

	stMeasAreaItem()
	{
		strName = _T("");
		iWaitTime_sec = 20;
	}
};

struct stCoordinate
{
	bool	bUse;		//Pattern 사용 유무
	bool	bAF;		//AF 사용 유무
	bool	bSkip;		//Skip 사용 유무
	double	dx;			//위치 x
	double	dy;			//위치 y
	double	ds;			//위치 s
	double	dz;			//위치 z
	int		nAFCh;		// AF

	stCoordinate()
	{
		bUse = bAF = bSkip = false;
		dx = dy = ds = dz = 0.0;
		nAFCh = 0;
	}
};

struct stPtrnInfo
{
	CString		strName;			//Pattern Name
	CString		strPtrnImgName[10];		//Pattern Name
	CRect		rectPtrn;			//Pattern Size

	double		dThreshold;			//Threshold

	int			iFindNum;			//Search count
	int			iCntrNum;			//Centering count

	int			iCenMarginX;		//Margin
	int			iCenMarginY;

	double 		dCenOffsetX;		//Offset after Centering
	double 		dCenOffsetY;

	double		dWhiteBalanceRed;
	double		dWhiteBalanceGreen;
	double		dWhiteBalanceBlue;

	double		dOffsetAfterCenteringX;
	double		dOffsetAfterCenteringY;

	int			iCCDGain;
	int			iExposureTime;
	int			iLampIntensity;

	int			iLens;

	bool		m_bDetected; 
	bool		m_bPtrnLearn;
	Image*		pPtrnImage;
	Image*		m_pImgScaled;
	Image*		m_pImgInput;

	TCHAR		m_cMultiPtrn[10][256];

	~stPtrnInfo()
	{
		if(m_pImgScaled)imaqDispose(m_pImgScaled);
		if(pPtrnImage)	imaqDispose(pPtrnImage);
		if (m_pImgInput)imaqDispose(m_pImgInput);
			
	}

	stPtrnInfo()
	{
		strName = _T("");

		dThreshold = 500.0;

		iFindNum = iCntrNum = 5;

		iCenMarginX = iCenMarginY = 10;

		dCenOffsetX = dCenOffsetY = 0.0;

		iCCDGain = 0;
		iExposureTime = 500;
		iLampIntensity = 90;

		iLens = 0;

		dWhiteBalanceRed = 0.0;
		dWhiteBalanceGreen = 0.0;
		dWhiteBalanceBlue = 0.0;

		dOffsetAfterCenteringX = 0.0;
		dOffsetAfterCenteringY = 0.0;

		m_bDetected = m_bPtrnLearn = false;

		ZeroMemory(&m_cMultiPtrn, sizeof(m_cMultiPtrn));

		m_pImgScaled	= imaqCreateImage(IMAQ_IMAGE_RGB, 7);
		pPtrnImage		= imaqCreateImage(IMAQ_IMAGE_RGB, 7);
		m_pImgInput		= imaqCreateImage(IMAQ_IMAGE_RGB, 7);

		for (int i = 0; i < 10; ++i)
		{
			strPtrnImgName[i] = _T("");
		}
	}
};

struct stAlignPtrnInfo
{
	stCoordinate stCoordAlign;		// Align 설정 좌표
	stCoordinate stoffsetAlign;		// Align 공정 좌표

	stPtrnInfo stPtrnAlign;
};

struct stRecipeAlign
{
	bool	bUseAlign;						//Align 사용 유무
	bool	bUse4PointAlign;				//4점 Align 사용 유무

	stAlignPtrnInfo stAlignPtrn[4];			//2점은 2개만 사용

	stRecipeAlign()
	{
		bUseAlign = bUse4PointAlign = false;
	}
};

struct stRecipeMeas
{
	stPtrnInfo stPtrnMeas[8];			//Teg1 White, Teg2 White, Teg1 UV, Teg2 UV, AA1 White, AA2 White, AA1 UV, AA2 UV (8ea)
	stRecipeMeas()	
	{
		;
	}
};

struct stThetaCalc
{
	double Lx;
	double Ly;
	double L1;
	double L2;
	double L3;
	double L4;
	double L;

	stThetaCalc()
	{
		Lx = 0.0;
		Ly = 0.0;
		L1 = 0.0;
		L2 = 0.0;
		L3 = 0.0;
		L4 = 0.0;
		L = 0.0;
	}
};

struct stRecipeOption
{
	//측정모드
	int nMeasMode;				//PPA

	int nUseMeasHead;			//0 : 1 + 2, 1 : only 1 head, 2 : only 2 head

	int nMeasShotPPA;			// 2 Shot, 1 Shot(White), 1 Shot(UV)

	int nMeasShotAA;			// 2 Shot, 1 Shot(White), 1 Shot(UV)

	double dHeadGap;

	double dZOffsetAA;

	double		dCellSizeW;
	double		dCellSizeH;

	int nBeamSplitterH1;
	int nBeamSplitterH2;

	int nGlassModel;

	bool bUseAlign;
	bool bUsePatternMatch;
	bool bUsePPA;
	bool bUseN2MotorAging;

	stThetaCalc		stPPAThetaCalc;

	stRecipeOption()
	{
		nMeasMode = 0;
		nUseMeasHead = 0;
		nMeasShotPPA = 0;
		nMeasShotAA = 0;
		nBeamSplitterH1 = 0;
		nBeamSplitterH2 = 0;
		nGlassModel = 0;
		dHeadGap = 0.0;
		dZOffsetAA = 0.0;

		dCellSizeW = 0.0;
		dCellSizeH = 0.0;

		bUseAlign = false;
		bUsePatternMatch = false;
		bUsePPA = false;
		bUseN2MotorAging = false;
	}
};

struct stAlignSetParam
{
	int			iCCDGain;
	int			iExposureTime;
	int			iLampIntensity;

	double		dCamResolution;
	int			nGlassThreshold;
	int			nCanyEdgeThreshold1;
	int			nCanyEdgeThreshold2;

	stAlignSetParam()
	{
		iCCDGain = 300;
		iExposureTime = 2000;
		iLampIntensity = 10;
		dCamResolution = 0.001;
		nGlassThreshold = 70;
		nCanyEdgeThreshold1 = 250;
		nCanyEdgeThreshold2 = 150;
	}
};

struct stTriggerSetParam
{
	int			iDirection;
	double		dEncoderResulution;
	int			iTriggerPeriod;
	int			iTriggerCount;

	double		dMeasureReadyPos;
	double		dStartPos;
	double		dMeseRange;
	double		dTriggerPitch;
	
	double		dSpeedVel;
	double		dSpeedAcc;
	double		dTriggerOffset;

	stTriggerSetParam()
	{
		iDirection = 1;
		iTriggerCount = 0;
		dEncoderResulution = 0.1;
		iTriggerPeriod = 100;
		dStartPos = 0;
		dMeseRange = 100;
		dTriggerPitch = 10;
		
		dMeasureReadyPos = 0;
		dSpeedVel = 30;
		dSpeedAcc = 300;
		dTriggerOffset = 0;
	}
};

struct stRecipeUTGParam
{
	int nVersion;

	//측정모드
	int nMeasMode;				// 1: Glass 상부, Left(Top Meas 기준)
								// 2: Glass 상부, Right(Top Meas 기준)
								// 3 : Glass 하부, Left(Top Meas 기준)
								// 4 : Glass 하부, Right(Top Meas 기준)

	int nUseMeasTopBtm;			//0: Meas Top and Bottom 1 : Meas Top 2 : Meas bottom 
	int nUseMeasHead;			//0 : 1 + 2, 1 : only 1 head, 2 : only 2 head
	int nUseMeasMode;			//0 :  0 + 180, 1: only 0, 2: only 180

	double		dCellSizeW;
	double		dCellSizeH;
	double		dCellHWCenter;

	double		dStageSizeW;
	double		dStageSizeH;

	// AF
	int nAFChannel[2];

	// Align Pos
	stCoordinate stCoordAlign;		// Align 설정 좌표

	// 180일때 사용
	double dAlingRotateOffsetX;
	double dAlingRotateOffsetY;

	// Align Param
	double		dAlignCamDistance;
	double		dAlignMarginX;
	double		dAlignMarginY;
	double		dAlignMarginR;
	int			nAlignRetry;
	stAlignSetParam stAlignParam[3];

	// Meas Pos
	stCoordinate stCoordTopMeasPos[2];
	stCoordinate stCoordBottomMeasPos[2];

	stCoordinate stCoordMeasPos[2];		// 측정 설정 좌표

	// Trigger Set
	stTriggerSetParam stTriggerParam;

	stThetaCalc		stPPAThetaCalc;

	stRecipeUTGParam()
	{
		nVersion = 1;
		nMeasMode = 1;
		nUseMeasTopBtm = 1;
		nUseMeasMode = 0;
		nUseMeasHead = 0;
		dCellSizeW = 0.0;
		dCellSizeH = 0.0;
		dCellHWCenter = 0.0;
		dStageSizeW = 0.0;
		dStageSizeH = 0.0;
		dAlignCamDistance = 50;

		dAlignMarginX = 0.01;
		dAlignMarginY = 0.01;
		dAlignMarginR = 0.01;
		nAlignRetry = 3;

		dAlingRotateOffsetX = 0;
		dAlingRotateOffsetY = 0;

		stCoordAlign.dz = 12.5;

		nAFChannel[0] = 0;	// Top
		nAFChannel[1] = 1;	// Bottom
	}
};

struct stRecipeInfo
{
	//Align
	stRecipeAlign		stRecipeAlignPtrn;

	//Pattern
	stPtrnInfo			stRecipeMeasPtrn[8];

	//Measure
	stRecipeMeas		stRecipeMeas;
	
	//Option
	stRecipeOption		stRecipeOptionParam;

	// UTG
	stRecipeUTGParam    stRecipeUTG;

	// HMI
	S_RecipeMgr			stRecipeHMI;
};

struct stMeasAreaInfo
{
	CString			strCellID;
	int				iMeasHeadID_linked;		// 검사 위치별 할당된 Head 정보
	int				iAreaNumber;			// 검사 위치별 번호
	int				iMeasOption;			//PPA, AA, Snap
	stCoordinate	stMeasCoordinate;

	int				nMultiHeadMeasArea;		//실제 측정 시 사용되는 area

	stMeasAreaInfo()
	{
		strCellID = _T("");
		iMeasHeadID_linked = -1;
		iAreaNumber = -1;
		iMeasOption = 0;

		nMultiHeadMeasArea = -1;
	}
};

struct S_MeasPos {

	double dHeadFrontG;
	double dHeadBackG;
	double dHeadFrontZ;
	double dHeadBackZ;
	S_CellInfo m_sCellInfo[2];

	S_MeasPos() {
		Clear();
	}

	void Clear() {

		dHeadFrontG = 0;
		dHeadBackG = 0;
		dHeadFrontZ = 0;
		dHeadBackZ = 0;
		for (size_t i = 0; i < 2; i++)
		{
			m_sCellInfo[i].Clear();
		}
	}
};

#define df_Gantry_Compare	0.00001
#define df_Head_Gap			300

class CBinaryHelper;
class CIniHelper;					//sklee 20210601 add
class CRecipeData
{	
	stRecipeInfo					m_stRecipeInfo;
	int								m_iCurrentAreaNum;
	std::vector<stMeasAreaItem>		m_vecMeasAreaItem;	// 검사 항목
	std::vector<stMeasAreaInfo>		m_vecMeasAreaInfo;	// 검사 항목 별 검사 위치

	std::vector<stMeasAreaInfo>		m_vecMeasAreaInfo_clone;

public:
	//----------------------------------------------------
	// Teach
	//----------------------------------------------------
public:
	int		GetParam_RecipeMeasAreaCount();
	void	AddParam_RecipeMeasArea();
	void	DelParam_RecipeMeasArea(int index);
	stMeasAreaInfo *GetParam_RecipeMeasArea(int index);
	void	SetParam_RecipeMeasArea(int index, stMeasAreaInfo &data);

	stRecipeInfo *GetParam_RecipeInfo();

	S_RecipeMgr* GetRecipe();
	S_RecipeInfo* GetRecipeInfo();
	S_RecipeData* GetRecipeData();
	S_RecipeMap* GetRecipeMap();


public:
	int		GetParam_CurrentMeasArea();
	void	SetParam_CurrentMeasArea(int value);

public:
	int		GetParam_MeasAreaItemCount() const;
	void	AddParam_MeasAreaItem();
	void	DelParam_MeasAreaItem(int index);
	CString GetParam_MeasAreaItem(int index);
	void	SetParam_MeasAreaItem(int index, CString value);
	long	GetParam_MeasAreaItemWaitTime(int index);
	long	GetParam_MeasAreaItemWaitTime(CString strName);
	void	SetParam_MeasAreaItemWaitTime(int index, long value);


public:
	void Clone(CRecipeData *rhs);
	bool LinkDataBase(CBinaryHelper *pBinaryHelper, bool bSave);
	bool LinkIniFile(CIniHelper *pIniHelper, bool bSave);						//sklee 20210601 add
	bool LinkIniFileUTG(CIniHelper *pIniHelper, bool bSave);

	//void ChangeAreaInfoRecipeToMeas();		//RecipeEditMgr로 옮김 jhseo

public:
	CRecipeData();
	~CRecipeData();
};
