#pragma once

#include "..\MotorImpl.h"
#include "include\Cmmsdk.h"

class CMotorControl_COMIZOA : public CMotorImpl
{

private:
	long SetSpeedDirectHome(int iAxis, double dVel, double dAcc, double dDec, double RevVel, int iSpeedMode);



	//------------------------------------------------------------------
	// 공통 인터페이스 함수
	//------------------------------------------------------------------
public:
	virtual BOOL		LinkIniFile(bool bSave)		override;
	virtual BOOL		LinkDatFile(bool bSave)		override;
	virtual UINT		ThreadRun()					override;
	virtual bool		InitDevice()				override;
	virtual CString		GetDeviceName()				override;
	virtual bool		IsConnected()				override;
	
	virtual long		ResetDevice(int iAxis) override;
	virtual long		MoveHome(int iAxis)	override;
	virtual long		MoveCmdPosA(int iAxis, double dPos, bool bBlockMode = false, int iSpeed = 0, int iLens = -1, int iTube = -1) override;
	virtual long		MoveCmdPosR(int iAxis, double dPos, bool bBlockMode = false, int iSpeed = 0) override;
	virtual bool		CheckMoveDone(int iAxis, DWORD dwTimeout) override;
	virtual bool		CheckHomeDone(int iAxis, DWORD dwTimeout) override;
	virtual	long		StopDevice(int iAxis) override;

	virtual long		SetSpeed(int iAxis, int iSpeedType, int iSpeedMode) override;
	virtual long		SetSpeedDirect(int iAxis, double dVel, double dAcc, double dDec, int iSpeedMode) override;
	virtual void		SetHomeProperty(int iAxis, int iHomeMode, int iHomeClrMode, long lEzCount, int iOrgLogic, int iEzLogic, bool bErcOut, double dEscDist, double dOffsetDist) override;

	virtual double		GetCurPos(int iAxis, int iLens = -1, int iTube = -1) override;
	virtual long		GetMotorStatus(int iAxis, int nStatus) override;
	virtual CString		GetLastErrorString() override;

public:
	CMotorControl_COMIZOA(CString strConfigPath, CString strFilePath, CString strCategory);
	virtual	~CMotorControl_COMIZOA();
};

