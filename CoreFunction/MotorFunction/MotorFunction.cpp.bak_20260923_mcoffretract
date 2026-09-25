#include "pch.h"
#include "MotorFunction.h"

#include "MotorCtrl_ACS\MotorControl_ACS.h"
#include "MotorCtrl_COMIZOA\MotorControl_COMIZOA.h"
#include "MotorCtrl_MEI\MotorControl_MEI.h"
//#include "MotorCtrl_PMAC\MotorControl_PMAC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CMotorFunction::CMotorFunction(CString strDeviceModel, CString strConfigPath, CString strFilePath, CString strCategory)
{
	m_pCtrlImpl = nullptr;

	//if(strDeviceModel == _T("ACS"))
	//{
	//	m_pCtrlImpl = dynamic_cast<CMotorImpl*>(new CMotorControl_ACS(strConfigPath, strFilePath, strCategory));
	//}
	//else if(strDeviceModel == _T("COMIZOA"))
	//{
	//	m_pCtrlImpl = dynamic_cast<CMotorImpl*>(new CMotorControl_COMIZOA(strConfigPath, strFilePath, strCategory));
	//}
	//else if(strDeviceModel == _T("MEI"))
	//{
	//	m_pCtrlImpl = dynamic_cast<CMotorImpl*>(new CMotorControl_MEI(strConfigPath, strFilePath, strCategory));
	//}
	//else if(strDeviceModel == _T("PMAC"))
	//{
	//	m_pCtrlImpl = dynamic_cast<CMotorImpl*>(new CMotorControl_PMAC(strConfigPath, strFilePath, strCategory));
	//}
	//else 
	{
		m_pCtrlImpl = dynamic_cast<CMotorImpl*>(new CMotorControl_ACS(strConfigPath, strFilePath, strCategory));
	}
}

CMotorFunction::~CMotorFunction()
{
    if (m_pCtrlImpl)
    {
        delete m_pCtrlImpl;
        m_pCtrlImpl = nullptr;
    }
}

bool CMotorFunction::Initialize()
{
	return m_pCtrlImpl->InitDevice();
}


//------------------------------------------------------------------
// 외부 접근 가능 함수
//------------------------------------------------------------------

bool CMotorFunction::InitDevice()
{
	return m_pCtrlImpl->InitDevice();
}

CString	CMotorFunction::GetDeviceName()
{
	return m_pCtrlImpl->GetDeviceName();
}

bool CMotorFunction::IsConnected()
{
	return m_pCtrlImpl->IsConnected();
}

long CMotorFunction::ResetDevice(int iAxis)
{
	return m_pCtrlImpl->ResetDevice(iAxis);
}

long CMotorFunction::MoveHome(int iAxis)
{
	return m_pCtrlImpl->MoveHome(iAxis);
}

long CMotorFunction::MoveCmdPosA(int iAxis, double dPos, bool bBlockMode, int iSpeed, int iLens, int iTube)
{
	return m_pCtrlImpl->MoveCmdPosA(iAxis, dPos, bBlockMode, iSpeed, iLens, iTube);
}
	
long CMotorFunction::MoveCmdPosAModify(int iAxis, double dPos)
{
	return m_pCtrlImpl->MoveCmdPosAModify(iAxis, dPos);
}

long CMotorFunction::MoveCmdPosR(int iAxis, double dPos, bool bBlockMode, int iSpeed)
{
	return m_pCtrlImpl->MoveCmdPosR(iAxis, dPos, bBlockMode, iSpeed);
}

long CMotorFunction::MoveCmdPosMulti(stMotorPosition *pPos, int iAxis, int iSpeed)
{
	return m_pCtrlImpl->MoveCmdPosMulti(pPos, iAxis, iSpeed);
}
	
long CMotorFunction::MoveJog(int iAxis, bool bDir)
{
	return m_pCtrlImpl->MoveJog(iAxis, bDir);
}

long CMotorFunction::WaitMotionEnd(int iAxis, DWORD dwTimeout)
{
	return m_pCtrlImpl->WaitMotionEnd(iAxis, dwTimeout);
}

bool CMotorFunction::CheckMoveDone(int iAxis, DWORD dwTimeout)
{
	return m_pCtrlImpl->CheckMoveDone(iAxis, dwTimeout);
}

bool CMotorFunction::CheckHomeDone(int iAxis, DWORD dwTimeout)
{
	return m_pCtrlImpl->CheckHomeDone(iAxis, dwTimeout);
}

long CMotorFunction::CheckInterlock(CString strAxis, int iAcs_mapping)	// LYS 2026-07-13
{
	return m_pCtrlImpl->CheckInterlock(strAxis, iAcs_mapping);
}

void CMotorFunction::StopHome()
{
	return m_pCtrlImpl->StopHome();
}

long CMotorFunction::StopDevice(int iAxis)
{
	return m_pCtrlImpl->StopDevice(iAxis);
}

long CMotorFunction::KillDevice(int iAxis)
{
	return m_pCtrlImpl->KillDevice(iAxis);
}

long CMotorFunction::RunBuffer(int iAxis)
{
	return m_pCtrlImpl->RunBuffer(iAxis);
}

long CMotorFunction::StopBuffer(int iAxis)
{
	return m_pCtrlImpl->StopBuffer(iAxis);
}


//Set parameter
long CMotorFunction::SetSpeed(int iAxis, int iSpeedType, int iSpeedMode)
{
	return m_pCtrlImpl->SetSpeed(iAxis, iSpeedType, iSpeedMode);
}

long CMotorFunction::SetSpeedDirect(int iAxis, double dVel, double dAcc, double dDec, int iSpeedMode)
{
	return m_pCtrlImpl->SetSpeedDirect(iAxis, dVel, dAcc, dDec, iSpeedMode);
}

void CMotorFunction::SetHomeProperty(int iAxis, int iHomeMode, int iHomeClrMode, long lEzCount, int iOrgLogic, int iEzLogic, bool bErcOut, double dEscDist, double dOffsetDist)
{
	return m_pCtrlImpl->SetHomeProperty(iAxis, iHomeMode, iHomeClrMode, lEzCount, iOrgLogic, iEzLogic, bErcOut, dEscDist, dOffsetDist);
}

void CMotorFunction::SetAxisMapping(int iAxis, int iAxis_real_hardware)
{
	return m_pCtrlImpl->SetAxisMapping(iAxis, iAxis_real_hardware);
}

void CMotorFunction::SetAxisProperty(int iAxis, axis_data_type *type)
{
	return m_pCtrlImpl->SetAxisProperty(iAxis, type);
}

long CMotorFunction::SetAmpEnable(int iAxis, bool bOn)
{
	return m_pCtrlImpl->SetAmpEnable(iAxis, bOn);
}

void CMotorFunction::SetAddress(CString strAddress, int iPort)
{
	return m_pCtrlImpl->SetAddress(strAddress, iPort);
}

void CMotorFunction::SetHomeDone(int iAxis, bool bDone)
{
	return m_pCtrlImpl->SetHomeDone(iAxis, bDone);
}

long CMotorFunction::SetWriteBuffer(CString strEQPName, bool bDirectOut, int iValue)
{
	return m_pCtrlImpl->SetWriteBuffer(strEQPName, bDirectOut, iValue);
}

void CMotorFunction::SetInterlockName(std::map<int, CString> &mapVal)
{
	return m_pCtrlImpl->SetInterlockName(mapVal);
}


//Get parameter
double CMotorFunction::GetCurPos(int iAxis, int iLens, int iTube)
{
	return m_pCtrlImpl->GetCurPos(iAxis, iLens, iTube);
}

long CMotorFunction::GetMotorStatus(int iAxis, int nStatus)
{
	return m_pCtrlImpl->GetMotorStatus(iAxis, nStatus);
}

axis_data_type *CMotorFunction::GetAxisProperty(int iAxis)
{
	return m_pCtrlImpl->GetAxisProperty(iAxis);
}

CAxisType  *CMotorFunction::GetAxisMapProperty(int iAxis)
{
	return m_pCtrlImpl->GetAxisMapProperty(iAxis);
}


double CMotorFunction::GetSpeedVal(int iAxis)
{
	return m_pCtrlImpl->GetSpeedVal(iAxis);
}

double CMotorFunction::GetSpeed(int iAxis, long iSpeed)
{
	return m_pCtrlImpl->GetSpeed(iAxis, iSpeed);
}

long CMotorFunction::GetAmpEnable(int iAxis)
{
	return m_pCtrlImpl->GetAmpEnable(iAxis);
}

long CMotorFunction::GetHWLimit(int iAxis, short bDir)
{
	return m_pCtrlImpl->GetHWLimit(iAxis, bDir);
}

double CMotorFunction::GetMaxSWLimit(int iAxis)
{
	return m_pCtrlImpl->GetMaxSWLimit(iAxis);
}

double CMotorFunction::GetMinSWLimit(int iAxis)
{
	return m_pCtrlImpl->GetMinSWLimit(iAxis);
}

double CMotorFunction::GetVelocityHigh(int iAxis)
{
	return m_pCtrlImpl->GetVelocityHigh(iAxis);
}

double CMotorFunction::GetVelocityMiddle(int iAxis)
{
	return m_pCtrlImpl->GetVelocityMiddle(iAxis);
}

double CMotorFunction::GetVelocityLow(int iAxis)
{
	return m_pCtrlImpl->GetVelocityLow(iAxis);
}

long CMotorFunction::GetMoving(int iAxis)
{
	return m_pCtrlImpl->GetMoving(iAxis);
}

bool CMotorFunction::GetHomeDone(int iAxis)
{
	return m_pCtrlImpl->GetHomeDone(iAxis);
}

double CMotorFunction::GetMotorRMSValue(int iAxis)
{
	return m_pCtrlImpl->GetMotorRMSValue(iAxis);
}

CString	CMotorFunction::GetLastErrorString()
{
	return m_pCtrlImpl->GetLastErrorString();
}

double CMotorFunction::GetOffset(int iAxis)
{
	return m_pCtrlImpl->GetOffset(iAxis);
}

long CMotorFunction::GetInput(int port, int bit, int iAcs_mapping)	// LYS 2026-07-13
{
	return m_pCtrlImpl->GetInput(port, bit, iAcs_mapping);
}
	
double CMotorFunction::GetFollowingErr(int iAxis)
{
	return m_pCtrlImpl->GetFollowingErr(iAxis);
}
	
double CMotorFunction::GetActualVel(int iAxis)
{
	return m_pCtrlImpl->GetActualVel(iAxis);
}

void CMotorFunction::LinkDatFile(bool bSave)
{
	m_pCtrlImpl->LinkDatFile(bSave);
}

void CMotorFunction::SetMaxSWLimit(int iAxis, double dLimit)
{
	m_pCtrlImpl->SetMaxSWLimit(iAxis, dLimit);
}

void CMotorFunction::SetMinSWLimit(int iAxis, double dLimit)
{
	m_pCtrlImpl->SetMinSWLimit(iAxis, dLimit);
}

double CMotorFunction::GetHomeVel(int iAxis)
{
	return m_pCtrlImpl->GetHomeVel(iAxis);
}