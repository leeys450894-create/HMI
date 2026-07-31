#pragma once

#include "AxisType.h"

class CIniHelper;
class CMotorImpl
{
protected:
	CIniHelper			*m_pIniHelper;
	bool				m_bBdInstalled;
	CString				m_strFilePath;
	std::vector<CAxisType>	m_cAxis;
	HANDLE				m_hWaitTimeEvent;



	//------------------------------------------------------------------
	// 공통 인터페이스 함수
	//------------------------------------------------------------------
public:
	virtual BOOL		LinkIniFile(bool bSave) = 0;	// pure virtual
	virtual BOOL		LinkDatFile(bool bSave) = 0;	// pure virtual
	virtual UINT		ThreadRun() = 0;				// pure virtual
    virtual bool		InitDevice() = 0;				// pure virtual
    virtual CString		GetDeviceName() = 0;			// pure virtual
    virtual bool		IsConnected() = 0;				// pure virtual


    virtual long		ResetDevice(int iAxis);
	virtual long		MoveHome(int iAxis);
	virtual long		MoveCmdPosA(int iAxis, double dPos, bool bBlockMode, int iSpeed, int iLens, int iTube);
	virtual long		MoveCmdPosAModify(int iAxis, double dPos);
	virtual long		MoveCmdPosR(int iAxis, double dPos, bool bBlockMode, int iSpeed);
	virtual long		MoveCmdPosMulti(stMotorPosition *pPos, int iAxis, int iSpeed);
	virtual long		MoveJog(int iAxis, bool bDir);
	virtual long		WaitMotionEnd(int iAxis, DWORD dwTimeout);
	virtual bool		CheckMoveDone(int iAxis, DWORD dwTimeout);
	virtual bool		CheckHomeDone(int iAxis, DWORD dwTimeout);
	virtual long		CheckInterlock(CString strAxis, int iAcs_mapping = 0);	// LYS 2026-07-13
	virtual void		StopHome();
	virtual	long		StopDevice(int iAxis);
	virtual	long		KillDevice(int iAxis);
	virtual long		RunBuffer(int iAxis);
	virtual long		StopBuffer(int iAxis);

	virtual long		SetSpeed(int iAxis, int iSpeedType, int iSpeedMode);
	virtual long		SetSpeedDirect(int iAxis, double dVel, double dAcc, double dDec, int iSpeedMode);
	virtual void		SetHomeProperty(int iAxis, int iHomeMode, int iHomeClrMode, long lEzCount, int iOrgLogic, int iEzLogic, bool bErcOut, double dEscDist, double dOffsetDist);
	virtual void		SetAxisMapping(int iAxis, int iAxis_real_hardware);
	virtual void		SetAxisProperty(int iAxis, axis_data_type *type);
	virtual long		SetAmpEnable(int iAxis, bool bOn);
	virtual void		SetAddress(CString strAddress, int iPort);
	virtual void		SetHomeDone(int iAxis, bool bDone);
	virtual long		SetWriteBuffer(CString strEQPName, bool bDirectOut, int iValue);
	virtual void		SetInterlockName(std::map<int, CString> &mapVal);
	
	virtual double		GetCurPos(int iAxis, int iLens, int iTube);
	virtual long		GetMotorStatus(int iAxis, int nStatus);
	virtual axis_data_type *GetAxisProperty(int iAxis);
	virtual CAxisType *GetAxisMapProperty(int iAxis);
	virtual double		GetSpeedVal(int iAxis);
	virtual double		GetSpeed(int iAxis, long iSpeed);
	virtual long		GetAmpEnable(int iAxis);
	virtual long		GetHWLimit(int iAxis, short bDir);
	virtual double		GetMaxSWLimit(int iAxis);
	virtual double		GetMinSWLimit(int iAxis);
	virtual double		GetVelocityHigh(int iAxis);
	virtual double		GetVelocityMiddle(int iAxis);
	virtual double		GetVelocityLow(int iAxis);
	virtual long		GetMoving(int iAxis);
	virtual bool		GetHomeDone(int iAxis);
	virtual double		GetMotorRMSValue(int iAxis);
	virtual CString		GetLastErrorString();
	virtual double		GetOffset(int iAxis);
	virtual long		GetInput(int port, int bit, int iAcs_mapping = 0);	// LYS 2026-07-13
	virtual double		GetFollowingErr(int iAxis);
	virtual double		GetActualVel(int iAxis);
	virtual void		SetMaxSWLimit(int iAxis, double dLimit);
	virtual void		SetMinSWLimit(int iAxis, double dLimit);
	virtual double		GetHomeVel(int iAxis);

public:
    CMotorImpl();
    virtual ~CMotorImpl();
};

