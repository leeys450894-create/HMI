#pragma once

#include "AxisType.h"

class CMotorImpl;
class CMotorFunction
{
	CMotorImpl *m_pCtrlImpl;
	

	//------------------------------------------------------------------
	// 외부 접근 가능 함수
	//------------------------------------------------------------------
public:
    bool		InitDevice();
    CString		GetDeviceName();
    bool		IsConnected();

    long		ResetDevice(int iAxis);
	long		MoveHome(int iAxis);
	long		MoveCmdPosA(int iAxis, double dPos, bool bBlockMode, int iSpeed, int iLens, int iTube);
	long		MoveCmdPosAModify(int iAxis, double dPos);
	long		MoveCmdPosR(int iAxis, double dPos, bool bBlockMode, int iSpeed);
	long		MoveCmdPosMulti(stMotorPosition *pPos, int iAxis, int iSpeed);
	long		MoveJog(int iAxis, bool bDir);
	long		WaitMotionEnd(int iAxis, DWORD dwTimeout);
	bool		CheckMoveDone(int iAxis, DWORD dwTimeout);
	bool		CheckHomeDone(int iAxis, DWORD dwTimeout);
	long		CheckInterlock(CString strAxis, int iAcs_mapping = 0);	// LYS 2026-07-13
	void		StopHome();
	long		StopDevice(int iAxis);
	long		KillDevice(int iAxis);
	long		RunBuffer(int iAxis);
	long		StopBuffer(int iAxis);

	long		SetSpeed(int iAxis, int iSpeedType, int iSpeedMode);
	long		SetSpeedDirect(int iAxis, double dVel, double dAcc, double dDec, int iSpeedMode);
	void		SetHomeProperty(int iAxis, int iHomeMode, int iHomeClrMode, long lEzCount, int iOrgLogic, int iEzLogic, bool bErcOut, double dEscDist, double dOffsetDist);
	void		SetAxisMapping(int iAxis, int iAxis_real_hardware);
	void		SetAxisProperty(int iAxis, axis_data_type *type);
	long		SetAmpEnable(int iAxis, bool bOn);
	void		SetAddress(CString strAddress, int iPort);
	void		SetHomeDone(int iAxis, bool bDone);
	long		SetWriteBuffer(CString strEQPName, bool bDirectOut, int iValue);
	void		SetInterlockName(std::map<int, CString> &mapVal);
	
	double		GetCurPos(int iAxis, int iLens, int iTube);
	long		GetMotorStatus(int iAxis, int nStatus);
	axis_data_type *GetAxisProperty(int iAxis);
	CAxisType *GetAxisMapProperty(int iAxis);
	double		GetSpeedVal(int iAxis);
	double		GetSpeed(int iAxis, long iSpeed);
	long		GetAmpEnable(int iAxis);
	long		GetHWLimit(int iAxis, short bDir);
	double		GetMaxSWLimit(int iAxis);
	double		GetMinSWLimit(int iAxis);
	double		GetVelocityHigh(int iAxis);
	double		GetVelocityMiddle(int iAxis);
	double		GetVelocityLow(int iAxis);
	long		GetMoving(int iAxis);
	bool		GetHomeDone(int iAxis);
	double		GetMotorRMSValue(int iAxis);
	CString		GetLastErrorString();
	double		GetOffset(int iAxis);
	long		GetInput(int port, int bit, int iAcs_mapping = 0);	// LYS 2026-07-13
	double		GetFollowingErr(int iAxis);
	double		GetActualVel(int iAxis);
	void		LinkDatFile(bool bSave);
	void		SetMaxSWLimit(int iAxis, double dLimit);
	void		SetMinSWLimit(int iAxis, double dLimit);
	double		GetHomeVel(int iAxis);

public:
	bool Initialize();

public:
	CMotorFunction(CString strDeviceModel, CString strConfigPath, CString strFilePath, CString strCategory);
	~CMotorFunction();	
};

