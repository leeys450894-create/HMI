#pragma once

#include "..\MotorImpl.h"

// LYS 2026-07-13 ACS 컨트롤러 수 - OCP는 2개(df_MAX_CTRL=2), HMI는 1개.
#define df_OCP_DUAL_CTRL		// comment out for HMI single controller
#ifdef df_OCP_DUAL_CTRL
#define df_MAX_CTRL 2
#else
#define df_MAX_CTRL 1
#endif

class CMotorControl_ACS : public CMotorImpl
{
	CString			m_strCategory;
	CString			m_strAddress[df_MAX_CTRL];	// LYS 2026-07-13 dual controller
	int				m_iPort;
	HANDLE			m_hAcs[df_MAX_CTRL];		// LYS 2026-07-13 dual controller
	int				m_iMaxLens;
	int				m_iMaxTubeLens;

private:
	CString GetDataFromString(CString inStr, CString startStr, CString endStr);
	bool	CheckHomeDoneLoop(int iAxis);
	int		MoveToPoint(int iAxis, double dPoint, int iFlag = 0);


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
	virtual long		MoveCmdPosMulti(stMotorPosition *pPos, int iAxis, int iSpeed) override;
	virtual long		MoveJog(int iAxis, bool bDir) override;
	virtual long		WaitMotionEnd(int iAxis, DWORD dwTimeout) override;
	virtual bool		CheckMoveDone(int iAxis, DWORD dwTimeout) override;
	virtual bool		CheckHomeDone(int iAxis, DWORD dwTimeout) override;
	virtual long		CheckInterlock(CString strAxis, int iAcs_mapping = 0) override;	// LYS 2026-07-13
	virtual	long		StopDevice(int iAxis) override;
	virtual	long		KillDevice(int iAxis) override;
	virtual long		RunBuffer(int iAxis) override;
	virtual long		StopBuffer(int iAxis) override;

	virtual long		SetSpeed(int iAxis, int iSpeedType, int iSpeedMode) override;
	virtual long		SetSpeedDirect(int iAxis, double dVel, double dAcc, double dDec, int iSpeedMode) override;
	virtual long		SetAmpEnable(int iAxis, bool bOn) override;
	virtual long		SetWriteBuffer(CString strEQPName, bool bDirectOut, int iValue) override;

	virtual double		GetCurPos(int iAxis, int iLens, int iTube) override;
	virtual long		GetMotorStatus(int iAxis, int nStatus) override;
	virtual double		GetSpeedVal(int iAxis) override;
	virtual double		GetSpeed(int iAxis, long iSpeed) override;
	virtual long		GetAmpEnable(int iAxis) override;
	virtual long		GetHWLimit(int iAxis, short bDir) override;
	virtual long		GetMoving(int iAxis) override;
	virtual bool		GetHomeDone(int iAxis) override;
	virtual double		GetMotorRMSValue(int iAxis) override;
	virtual CString		GetLastErrorString() override;
	virtual long		GetInput(int port, int bit, int iAcs_mapping = 0) override;	// LYS 2026-07-13
	virtual double		GetFollowingErr(int iAxis) override;
	virtual double		GetActualVel(int iAxis) override;
	virtual double		GetHomeVel(int iAxis) override;
	virtual void		SetMaxSWLimit(int iAxis, double dLimit) override;
	virtual void		SetMinSWLimit(int iAxis, double dLimit) override;

public:
	CMotorControl_ACS(CString strAppPath, CString strFilePath, CString strCategory);
	virtual	~CMotorControl_ACS();
};

