// AxisType.cpp: implementation of the CAxisType class.
//
//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "AxisType.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CAxisType::CAxisType()
{
	m_setting.bReverse			= TRUE;		// Reverse direction
	m_setting.iStepper			= 1;		// Step motor 
	
	m_setting.dVelH				= 500;	// Velocity High
	m_setting.dVelM				= 200;		//          Middle
	m_setting.dVelL				= 100;		//          Low
	m_setting.dVelHome			= 50;		//          Home
	m_setting.dVelLatch			= 50;		//          Latch
	
	m_setting.dAccH				= 100;	// Acceleration High
	m_setting.dAccM				= 80;		//              Middle
	m_setting.dAccL				= 50;		//              Low  
	m_setting.dAccHome			= 50;		//              Home
	
	m_setting.dScvH				= 50;	// Deceleration High
	m_setting.dScvM				= 40;		//              Middle
	m_setting.dScvL				= 25;		//              Low
	m_setting.dScvHome			= 25;		//              Home
	
	m_setting.dRevVelHome		= 25;	//Reverse Home Velocity

	m_setting.dEncL				= 1;		// Encoder length
	m_setting.dStpL				= 0.04;		// Step length
	
	m_setting.dDimCen			= 0;		// Dimension Center
	m_setting.dDimMax			= 100;		//           Max
	m_setting.dDimMin			= -100;		//           Min
	
	m_setting.iHomeMode			= 0;
	m_setting.iHomeClrMode		= 0;
	m_setting.lEzCount			= 0;
	m_setting.iOrgLogic			= 0;
	m_setting.iEzLogic			= 0;
	m_setting.bErcOut			= FALSE;
	m_setting.dEscDist			= 0.0;
	m_setting.dOffsetDist		= 0.0;

	m_setting.dOffset			= 0.0;	// Home offset
	m_setting.dHomePos			= 0.0;
	
	m_dPos						= 0;
	m_dPosUM0					= 0;
	m_iDir						= 1;
	m_bHomeDone					= FALSE;

	m_setting.dPulse = 1;			// jsyoon 2022.02.22 add

	SetSpeed(SPEED_HIGH);

	m_iAxisMap = -1;
	m_iACSMap = 0;							// LYS 2026-07-13
	m_setting.iNumHomePLC = -1;
	m_setting.bGantryAxis = FALSE;
	m_setting.iMasterAxis = -1;
	for (int i=0; i<10; i++)
	{
		m_setting.dLensShift[i] = 0.0;
		m_setting.dTubeShift[i] = 0.0;
	}

	m_strDivision = _T("");
	m_strAxisName = _T("");
}

CAxisType::CAxisType(axis_data_type setting, int iAxisMap)
{
	m_setting	= setting;
	m_iAxisMap	= iAxisMap;
	m_dPos		= 0;
	m_dPosUM0	= 0;
	m_iDir		= (m_setting.bReverse) ? -1 : 1;

	SetSpeed(SPEED_HIGH);
}

CAxisType::~CAxisType()
{

}

void CAxisType::SetSpeed(int iSpeedType)
{
	switch (iSpeedType)
	{
		case SPEED_HIGH:
		{
			m_dVel = m_setting.dVelH;
			m_dAccT = m_setting.dAccH;
			m_dScvT = m_setting.dScvH;
			m_dRevVel = m_setting.dRevVelHome;
		}
		break;

		case SPEED_MID:	
		{
			m_dVel = m_setting.dVelM;
			m_dAccT = m_setting.dAccM;
			m_dScvT = m_setting.dScvM;
			m_dRevVel = m_setting.dRevVelHome;
		}
		break;

		case SPEED_LOW:
		{
			m_dVel = m_setting.dVelL;
			m_dAccT = m_setting.dAccL;
			m_dScvT = m_setting.dScvL;
			m_dRevVel = m_setting.dRevVelHome;
		}
		break;

		case SPEED_HOME:
		{
			m_dVel = m_setting.dVelHome;
			m_dAccT = m_setting.dAccHome;
			m_dScvT = m_setting.dScvHome;
			m_dRevVel = m_setting.dRevVelHome;
		}
		break;

		case SPEED_LATCH:
		{
			m_dVel = m_setting.dVelLatch;
			m_dAccT = m_setting.dAccH;
			m_dScvT = m_setting.dScvH;
			m_dRevVel = m_setting.dRevVelHome;
		}
		break;

		case SPEED_MULTI:
		{
			m_dVel = m_setting.dVelH * 1.5;
			m_dAccT = m_setting.dAccH * 1.5;
			m_dScvT = m_setting.dScvH;
			m_dRevVel = m_setting.dRevVelHome;
		}
		break;

		default:	// LOW
		{
			m_dVel = m_setting.dVelL;
			m_dAccT = m_setting.dAccL;
			m_dScvT = m_setting.dScvL;
			m_dRevVel = m_setting.dRevVelHome;
		}
		break;
	}
}

void CAxisType::SetSpeedDirect(double Speed, double AccT, double DecT, double RevSpeed)
{
	m_dVel		= Speed;
	m_dAccT		= AccT;
	m_dScvT		= DecT;
	m_dRevVel	= RevSpeed;
}

double CAxisType::GetSpeed(int iSpeedType)
{
	double dRet = 0.0;

	switch (iSpeedType)
	{
		case SPEED_HIGH:	dRet = m_setting.dVelH;		break;
		case SPEED_MID:		dRet = m_setting.dVelM;		break;
		case SPEED_LOW:		dRet = m_setting.dVelL;		break;
		case SPEED_HOME:	dRet = m_setting.dVelHome;	break;
		case SPEED_LATCH:	dRet = m_setting.dVelLatch;	break;
		default:			dRet = m_setting.dVelL;		break;
	}

	return dRet;
}



CAxisType& CAxisType::operator=(CAxisType& rValue)
{
	this->m_setting		= rValue.m_setting;	
	this->m_iMode		= rValue.m_iMode;
	this->m_dVel		= rValue.m_dVel;
	this->m_dAccT		= rValue.m_dAccT;
	this->m_dScvT		= rValue.m_dScvT;
	this->m_dRevVel		= rValue.m_dRevVel;	
	this->m_dPos		= rValue.m_dPos;
	this->m_dPosUM0		= rValue.m_dPosUM0;
	this->m_iDir		= rValue.m_iDir;
	this->m_iAxisMap	= rValue.m_iAxisMap;
	this->m_iACSMap	= rValue.m_iACSMap;	// LYS 2026-07-13
	this->m_bHomeDone	= rValue.m_bHomeDone;
	
	return *this;
}