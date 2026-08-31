#pragma once

#include <map>
#include <vector>

enum axis_type_t 
{
	AXIS_CAMERA = -9,
	AXIS_NONE	= -8,
	AXIS_ALL_Z	= -7,
	AXIS_SACC	= -6,
	AXIS_LOADER = -5,
// 	AXIS_OCP	= -4,
// 	AXIS_ICM	= -3,
// 	AXIS_ICE	= -2,
	AXIS_ALL	= -1,	// ICP
	AXIS_X	 = 0,
	AXIS_X0  = AXIS_X,	// Top Gantry
	AXIS_X1	 = 1,
	AXIS_Y	 = 2,
	AXIS_Y0  = AXIS_Y,	// Top Cross

	AXIS_X2,			// Bottom Gantry
	AXIS_X3,

	// 
	// Auto Change
	AXIS_X4,			// Gantry
	AXIS_X5,
	AXIS_T = AXIS_X5,
	
	AXIS_Y1,			// Bottom Cross	Left
	AXIS_Y2,			// Bottom Cross Right
	AXIS_Y3,			// Top Align Left
	AXIS_Y4,			// Top Align Right
	AXIS_Y5,			// Bottom Align Left	
	AXIS_Y6,			// Bottom Align Right	// OCP: PPA Cross Right

	AXIS_Y7,			// Probe Cross Left		// ICE, OCP: PPA Cross Left
	AXIS_Y8,			// Probe Cross Right	// ICE, OCP: PPA Cross Right	

	AXIS_R,				// Rotate
	AXIS_R0 = AXIS_R,	// Top Chuck Rotate

	// 20190926 kks 
	AXIS_R1,			// Probe Rotate
	AXIS_PYAW = AXIS_R1,	
	AXIS_R2,			
	AXIS_UNIT_CROSS = AXIS_R2,

	AXIS_Z,
	AXIS_Z1 = AXIS_Z,	// Head 1 Vision		// 화질	2026-07-14 : 특성
	AXIS_Z2,			// Head 1 Review		// 특성 2026-07-14 : 화질
	AXIS_Z3,			// Head 1 Angle
	AXIS_Z4,			// Head 2 Vision
	AXIS_Z5,			// Head 2 Review		// AF
	AXIS_Z6,			// Head 2 Angle

	AXIS_Z7,			// Probe Z Left			// ICE, OCP: PPA1	
	AXIS_Z8,			// Probe Z Right		// ICE, OCP: PPA2

	AXIS_Z9,			// Auto Change Z
	AXIS_Z10,			// Auto Change Z
	AXIS_Z11,			// SE Z		// ICE

	AXIS_UNIT_Z,		// ICE: Z축
	AXIS_UNIT_Z2,

	AXIS_ZOOM1,			// 29M
	AXIS_THETA1,		// 29M
	AXIS_FILTER1,		// 29M
	AXIS_VLENS1,		// 29M
	AXIS_ZOOM2,
	AXIS_THETA2,
	AXIS_FILTER2,
	AXIS_VLENS2,	
	
	//	ColorMix
	AXIS_ROTATE1,		// 4M
	AXIS_FOCUS1,		// 4M
	AXIS_ROTATE2,
	AXIS_FOCUS2,

	AXIS_ROTATE3,		// 4M
	AXIS_FOCUS3,		// 4M
	AXIS_ROTATE4,
	AXIS_FOCUS4,

	// LOADER
	AXIS_LOADER_X,
	AXIS_LOADER_Y,
	AXIS_RJ,				// Rack jack	
	AXIS_LOADER_Z = AXIS_RJ,
	AXIS_LOADER_R,

	AXIS_LP,			// LOADER
	AXIS_LP2,	
	//////////////////////////////////////////////////////////////////////////

	// PPA
	AXIS_L,			// Lens
	AXIS_F,			// Filter
	AXIS_WL1,		// White Light
	AXIS_UL1,		// UV Light
	AXIS_XS,		// Sub X
	AXIS_YS,		// Sub Y

	AXIS_L2,
	AXIS_WL2,
	AXIS_UL2,
	AXIS_XS2,		// Sub
	AXIS_YS2,		// Sub Y

	AXIS_LENS,

	// TILTI
	// COLOR
	AXIS_COLOR_DEGREE1,
	AXIS_COLOR_DEGREE2,
		
	// PSI
	AXIS_SPECTO_DEGREE1,
	AXIS_SPECTO_DEGREE2,

	//AXIS_UNIT_CROSS,	
	//AXIS_PYAW,

	// PROBE
	AXIS_PROBE_X1,
	AXIS_PROBE_X2,
	AXIS_PROBE_X3,
	AXIS_PROBE_X4,
	AXIS_PROBE_X5,
	AXIS_PROBE_X6,
	AXIS_PROBE_X7,
	AXIS_PROBE_X8,
	AXIS_PROBE_X9,
	AXIS_PROBE_X10,

	AXIS_PROBE_Y1,
	AXIS_PROBE_Y2,
	AXIS_PROBE_Y3,
	AXIS_PROBE_Y4,
	AXIS_PROBE_Y5,

	AXIS_PROBE_Z1,
	AXIS_PROBE_Z2,
	AXIS_PROBE_Z3,
	AXIS_PROBE_Z4,
	AXIS_PROBE_Z5,

	// LYS 2026-08-19 ADD ANGLE AXIS
	AXIS_ANGLE_SF1,			// CAM Scheimpflug 35 ~ 75도
	AXIS_ANGLE_ROTATE1,		// CAM ROTATE -95 ~ 95도
	AXIS_ANGLE_ZOOM1,		// CAM ZOOM
	AXIS_ANGLE_TILT1,		// MODULE TILT 0 ~ 75도
	AXIS_ANGLE_AZM1,		// MODULE AZIMUTH(방위각) 0 ~ 180도

	AXIS_ANGLE_SF2,
	AXIS_ANGLE_ROTATE2,
	AXIS_ANGLE_ZOOM2,
	AXIS_ANGLE_TILT2,
	AXIS_ANGLE_AZM2,

	AXIS_TYPE_END = 99,

	//df_RUNBUFFER_1 = 91,
	//df_RUNBUFFER_2,
	
//	AXIS_TYPE_END2 = 60,
	df_USE_MAX_AXIS = 100,
};

enum axis_mei_type_t 
{
	AXIS_MEI_X = 0,
	AXIS_MEI_X0 = 0,
	AXIS_MEI_X1,
	AXIS_MEI_Y,
	AXIS_MEI_Z,
	AXIS_MEI_LENS,
	AXIS_MEI_RJ,
	AXIS_MEI_END
};

struct stMotorPosition
{
	int axis;
	double dPosition;
	int iLens;
	int iTube;
};

#define df_HOMINGDONE_TIME 1800		// 1800 sec
#define df_MOVINGDONE_TIME 60		// 60 sec

enum speed_type_t 
{
	SPEED_HIGH = 0,
	SPEED_MID,
	SPEED_LOW,
	SPEED_HOME,
	SPEED_LATCH,
	SPEED_MULTI,
	SPEED_TYPE_END
};

enum smode_type_t 
{
	SMODE_KEEP = -1,	// 이전 속도 모드 그대로 설정한다. 
	SMODE_C = 0,		// CONSTANT 속도 모드 : 가감속을 수행하지 않는다. 
	SMODE_T = 1,		// TRAPEZOIDAL 속도 모드 : 사다리꼴 가감속을 수행한다. 
	SMODE_S = 2,		// S-CURVE 속도 모드 : S-CURVE 가감속을 수행한다. 
	SMODE_TYPE_END,
};

enum mapping_error_t 
{
	ERROR_GANTRY = 0,
	ERROR_X_ERROR,
	ERROR_XY_ERROR,
	MAPPING_ERROR_END
};

enum motor_status_t
{
	STATE_MOT_ENABLE	=  0,
	STATE_MOT_DISABLE,
	STATE_MOT_STOPMOT,
	STATE_MOT_INPOSIT,
	STATE_MOT_LMTNEG,
	STATE_MOT_LMTPOS,
	STATE_MOT_STOPLMT,
	STATE_MOT_ERRFLOW,
	STATE_MOT_ERRPHAS,
	STATE_MOT_HOMCOMP,
	STATE_MOT_HOMDONE,
	STATE_MOT_END,
};

enum donecheck_method_t
{
	DONE_BOTH		= 0,	// default
	DONE_INPOS,
	DONE_ZEROVEL,	
	DONE_METHOD_END,
};

struct axis_data_type
{
	BOOL		bUse;			// config.read할때 True해준다.

	BOOL		bReverse;		// Reverse direction
	int			iStepper;		// Step motor 

	double		dVelH;			// Velocity High			in mm
	double		dVelM;			//          Middle
	double		dVelL;			//          Low
	double		dVelHome;		//          Home
	double		dVelLatch;		//          Latch
	
	double		dAccH;			// Acceleration time High	in msec
	double		dAccM;			//              Middle
	double		dAccL;			//              Low  
	double		dAccHome;		//              Home

	double		dScvH;			// Scurve time High	in msec
	double		dScvM;			//              Middle
	double		dScvL;			//              Low
	double		dScvHome;		//              Home

	double		dRevVelHome;	// Reverse Home Velocity

	double		dEncL;			// Encoder length			in um
	double		dStpL;			// Step length				in um

	double		dDimCen;		// Dimension Center			in mm
	double		dDimMax;		//           Max
	double		dDimMin;		//           Min
	
	double		dPulse;			// Pls



	//LX502 Home Property
	int			iHomeMode;
	int			iHomeClrMode;
	long		lEzCount;
	int			iOrgLogic;
	int			iEzLogic;
	BOOL		bErcOut;
	double		dEscDist;
	double		dOffsetDist;
	
	// MEI Home Property
	BOOL		bNeg;
	BOOL		bHome;				
	BOOL		bHomeEdge;
	BOOL		bIndex;		
	BOOL		lCaptureSrc;
	BOOL		lEncoder;		
	double		dGantryOffset;		
	BOOL		lGantryHomeMode;


	double		dOffset;		// Offset for adjusting homing speed or setting home
	double		dHomePos;		// Pos after homing
	int			iNumHomePLC;	// buffer number
	BOOL		bGantryAxis;
	int			iMasterAxis;
	int			iSlaveAxis;
	int			iClamp;
	double		dLensShift[10];
	double		dTubeShift[10];


	

	axis_data_type()
	{
		bUse = FALSE;
	}
};

class CAxisType  
{	
public:
	axis_data_type	m_setting;	
	int				m_iMode;		// Mode
	double			m_dVel;			// Current vel
	double			m_dAccT;		//         accel.
	double			m_dScvT;		//         decel.
	double			m_dRevVel;		// Reverse Vel
	double			m_dPos;			// Current pos						in mm
	double			m_dPosUM0;		// Zero position for shifting,		in um
	int				m_iDir;			// Swap direction, take 1 or -1, depending on m_bReverse
	int				m_iAxisMap;
	int				m_iACSMap;		// LYS 2026-07-13 ACS controller index (0 or 1)
	bool			m_bHomeDone;
	CString			m_strDivision;
	CString			m_strAxisName;

public:
	void	SetSpeed(int iSpeedType);														// Set speed, accel and decel setting
	void	SetSpeedDirect(double Speed, double AccT, double ScvT, double RevSpeed = 0);	// Set speed by value
	double	GetSpeed(int iSpeedType);														// Set speed, accel and decel setting

public:
	CAxisType&	operator=(CAxisType&);

public:
	CAxisType();
	CAxisType(axis_data_type setting, int iAxisMap);
	~CAxisType();
};
