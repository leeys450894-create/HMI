#include "pch.h"
#include "MotorImpl.h"

#include "..\common\IniHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CMotorImpl::CMotorImpl()
	: m_pIniHelper(new CIniHelper)
	, m_bBdInstalled(false)
{	
	m_cAxis.clear();

	m_hWaitTimeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);	// auto reset, initial reset
}

CMotorImpl::~CMotorImpl()
{
	if(m_hWaitTimeEvent)
	{
		CloseHandle(m_hWaitTimeEvent);
	}

	//----------------------------------------------------------------------
	// 데이터 삭제
	//----------------------------------------------------------------------
	if (m_pIniHelper)
	{
		delete m_pIniHelper;
		m_pIniHelper = nullptr;
	}
}




//------------------------------------------------------------------
// 공통 인터페이스 함수
//------------------------------------------------------------------
long CMotorImpl::ResetDevice(int iAxis)
{
	return -1;
}

long CMotorImpl::MoveHome(int iAxis)
{
	return -1;
}

long CMotorImpl::MoveCmdPosA(int iAxis, double dPos, bool bBlockMode, int iSpeed, int iLens, int iTube)
{
	return -1;
}

long CMotorImpl::MoveCmdPosAModify(int iAxis, double dPos)
{
	return -1;
}

long CMotorImpl::MoveCmdPosR(int iAxis, double dPos, bool bBlockMode, int iSpeed)
{
	return -1;
}

long CMotorImpl::MoveCmdPosMulti(stMotorPosition *pPos, int iAxis, int iSpeed)
{
	return -1;
}

long CMotorImpl::MoveJog(int iAxis, bool bDir)
{
	return -1;
}

long CMotorImpl::WaitMotionEnd(int iAxis, DWORD dwTimeout)
{
	return -1;
}

bool CMotorImpl::CheckMoveDone(int iAxis, DWORD dwTimeout)
{
	return false;
}

bool CMotorImpl::CheckHomeDone(int iAxis, DWORD dwTimeout)
{
	return false;
}

long CMotorImpl::CheckInterlock(CString strAxis, int iAcs_mapping)	// LYS 2026-07-13
{
	return -1;
}

void CMotorImpl::StopHome()
{
}

long CMotorImpl::StopDevice(int iAxis)
{
	return -1;
}

long CMotorImpl::KillDevice(int iAxis)
{
	return -1;
}

long CMotorImpl::RunBuffer(int iAxis)
{
	return -1;
}

long CMotorImpl::StopBuffer(int iAxis)
{
	return -1;
}


//Set parameter
long CMotorImpl::SetSpeed(int iAxis, int iSpeedType, int iSpeedMode)
{
	return -1;
}

long CMotorImpl::SetSpeedDirect(int iAxis, double dVel, double dAcc, double dDec, int iSpeedMode)
{
	return -1;
}

void CMotorImpl::SetHomeProperty(int iAxis, int iHomeMode, int iHomeClrMode, long lEzCount, int iOrgLogic, int iEzLogic, bool bErcOut, double dEscDist, double dOffsetDist)
{
}

void CMotorImpl::SetAxisMapping(int iAxis, int iAxis_real_hardware)
{
	if( iAxis < AXIS_TYPE_END)
	{
	    m_cAxis[iAxis].m_iAxisMap = iAxis_real_hardware;
	}
}

void CMotorImpl::SetAxisProperty(int iAxis, axis_data_type *type)
{

}

long CMotorImpl::SetAmpEnable(int iAxis, bool bOn)
{
	return -1;
}

void CMotorImpl::SetAddress(CString strAddress, int iPort)
{
}


void CMotorImpl::SetHomeDone(int iAxis, bool bDone)
{
	if( iAxis < AXIS_TYPE_END)
	{
		if(m_cAxis[iAxis].m_setting.bUse == TRUE)
		{
			m_cAxis[iAxis].m_bHomeDone = bDone;
		}
	}
}

long CMotorImpl::SetWriteBuffer(CString strEQPName, bool bDirectOut, int iValue)
{
	return -1;
}

void CMotorImpl::SetInterlockName(std::map<int, CString> &mapVal)
{
}


//Get parameter
double CMotorImpl::GetCurPos(int iAxis, int iLens, int iTube)
{
	return 0.0;
}

long CMotorImpl::GetMotorStatus(int iAxis, int nStatus)
{
	return -1;
}

axis_data_type *CMotorImpl::GetAxisProperty(int iAxis)
{
	if( iAxis < AXIS_TYPE_END)
	{
		return &m_cAxis[iAxis].m_setting;
	}

	return nullptr;
}

CAxisType *CMotorImpl::GetAxisMapProperty(int iAxis)
{
	if (iAxis < AXIS_TYPE_END)
	{
		return &m_cAxis[iAxis];
	}

	return nullptr;

}
double CMotorImpl::GetSpeedVal(int iAxis)
{
	return 0.0;
}

double CMotorImpl::GetSpeed(int iAxis, long iSpeed)
{
	return 0.0;
}

long CMotorImpl::GetAmpEnable(int iAxis)
{
	return -1;
}
long CMotorImpl::GetHWLimit(int iAxis, short bDir)
{
	return -1;
}

double CMotorImpl::GetMaxSWLimit(int iAxis)
{
	if( iAxis < AXIS_TYPE_END)
	{
	    return m_cAxis[iAxis].m_setting.dDimMax;
	}

	return 0.0;
}

double	CMotorImpl::GetMinSWLimit(int iAxis)
{
	if( iAxis < AXIS_TYPE_END)
	{
	    return m_cAxis[iAxis].m_setting.dDimMin;
	}

	return 0.0;
}

double	CMotorImpl::GetVelocityHigh(int iAxis)
{
	if( iAxis < AXIS_TYPE_END)
	{
	    return m_cAxis[iAxis].m_setting.dVelH;
	}

	return 0.0;
}

double CMotorImpl::GetVelocityMiddle(int iAxis)
{
	if( iAxis < AXIS_TYPE_END)
	{
	    return m_cAxis[iAxis].m_setting.dVelM;
	}

	return 0.0;
}

double CMotorImpl::GetVelocityLow(int iAxis)
{
	if( iAxis < AXIS_TYPE_END)
	{
	    return m_cAxis[iAxis].m_setting.dVelL;
	}

	return 0.0;
}

long CMotorImpl::GetMoving(int iAxis)
{
	return -1;
}

bool CMotorImpl::GetHomeDone(int iAxis)
{
	if( iAxis < AXIS_TYPE_END)
	{
		if(m_cAxis[iAxis].m_setting.bUse == FALSE)
			return true;

		return m_cAxis[iAxis].m_bHomeDone;
	}
	
	return false;
}

double CMotorImpl::GetMotorRMSValue(int iAxis)
{
	return 0.0;
}

CString	CMotorImpl::GetLastErrorString()
{
	return _T("");
}

double	CMotorImpl::GetOffset(int iAxis)
{
	return 0.0;
}

long CMotorImpl::GetInput(int port, int bit, int iAcs_mapping)	// LYS 2026-07-13
{
	return -1;
}

double CMotorImpl::GetFollowingErr(int iAxis)
{
	return 0.0;
}
	
double CMotorImpl::GetActualVel(int iAxis)
{
	return 0.0;
}

void CMotorImpl::SetMaxSWLimit(int iAxis, double dLimit)
{

}

void CMotorImpl::SetMinSWLimit(int iAxis, double dLimit)
{

}

double CMotorImpl::GetHomeVel(int iAxis)
{
	return 0.0;
}
