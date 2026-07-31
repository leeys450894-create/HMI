#pragma once

#include "..\MotorImpl.h"

//class CXmpQ;
class CIniHelper;
class CMotorControl_MEI : public CMotorImpl
{
	CString			m_strCategory;
	//CXmpQ			*m_pXmpQ;
	int				m_iXHomeDir;
	int				m_iYHomeDir;
	double			m_dXUnloadPos;
	double			m_dYUnloadPos;
	double			m_dZUnloadPos;

private:
	CString GetDataFromString(CString inStr, CString startStr, CString endStr);

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
	virtual	long		StopDevice(int iAxis) override;

	virtual long		SetAmpEnable(int iAxis, bool bOn);
	virtual long		SetSpeed(int iAxis, int iSpeedType, int iSpeedMode) override;
	virtual long		SetSpeedDirect(int iAxis, double dVel, double dAcc, double dDec, int iSpeedMode) override;

	virtual double		GetCurPos(int iAxis, int iLens = -1, int iTube = -1) override;
	virtual long		GetMotorStatus(int iAxis, int nStatus) override;
	
public:
	CMotorControl_MEI(CString strConfigPath, CString strFilePath, CString strCategory);
	virtual	~CMotorControl_MEI();
};

