#pragma once

#include "..\AuroraSubMgr.h"
#include "MotorControlMgr.h"
#include "IOMapControlMgr.h"
#include "PositionConfigMgr.h"
const enum MOT_POS {
	df_POS_DOWN = 0,
	df_POS_PRE,
	df_POS_MIDDLE,
	df_POS_UP,
	df_POS_MAX,
};

const enum _PROBE_TYPE_SET {
	df_PROBE_AGING1 = 1,
	df_PROBE_AGING2,
	df_PROBE_AGING3,
	df_PROBE_AGING4,
	df_PROBE_MEAS,
};

class CIOMapControlMgr;
class CMotorControlMgr;
class CPositionConfigMgr;
class CLightControlMgr;
class CShareMemoryMgr;
class CMaintControlMgr : public CAuroraSubMgr
{
	CIOMapControlMgr* m_pIOMapControlMgr;
	CMotorControlMgr* m_pMotorControlMgr;
	CShareMemoryMgr* m_pShareMemoryMgr;
	CPositionConfigMgr* m_pPositionConfigMgr;
	CLightControlMgr* m_pMgrLightControl;

	double m_dGlassAlignOffset[3];
	double m_dProbeAlignOffset[3];  // Probe Align 보정값

	double m_dAlignOffset[100];		// HW Align 보정값(Probe 장착시 보정값

	//------------------------------------------------------------------
	// 공통 인터페이스 함수
	//------------------------------------------------------------------
public:
	virtual bool LinkIniFile(bool bSave) override;
private:
	virtual void Finalize() override;
public:
	CMaintControlMgr(CMotorControlMgr* pMotorControlMgr, CIOMapControlMgr* pIOMapControlMgr, CPositionConfigMgr* pPositionConfigMgr, CLightControlMgr* pMgrLightControl, CShareMemoryMgr* pShareMemoryMgr );
	~CMaintControlMgr(void);

	CIOMapControlMgr* GetIOMapControlMgrPtr() const { return m_pIOMapControlMgr; }
	CMotorControlMgr* GetMotorControlMgrPtr() const { return m_pMotorControlMgr; }
	CShareMemoryMgr* GetShareMemoryMgrPtr() const { return m_pShareMemoryMgr; }
	CPositionConfigMgr* GetPositionConfigMgr() const { return m_pPositionConfigMgr; }
	CLightControlMgr* GetLightControlMgrPtr() const { return m_pMgrLightControl; }

	// LYS 2026-08-25 stMotorPosition 배열을 채우기 전에 비운다(정의는 MaintControlMgr.cpp).
	// ZeroMemory 대신 이것을 쓸 것 - AXIS_X가 0이라 0으로 채운 슬롯은 스테이지 갠트리 이동 지령이 된다.
	static void ClearMotorPos(stMotorPosition* pPos, int nCount);

	long SetAmpEnable(int iAxis, bool bOn);
	long MoveHome(int iAxis);
	long MoveCmdPosA(int iAxis, double dPos, bool bBlockMode, int iSpeed = 0, bool bUseOffset = false, int iLens = -1, int iTube = -1);
	long MoveCmdPosR(int iAxis, double dPos, bool bBlockMode, int iSpeed = 0);
	long MoveCmdPosMulti(stMotorPosition* pPos, int iAxis, int iSpeed = 0, bool bDoneCheck = false, bool bUseOffset = false);
	double GetCurPos(int iAxis, int iLens = -1, int iTube = -1);

	long MoveOrgPosA(int iAxis, double dPos, bool bBlockMode, int iSpeed = 0, int iLens = -1, int iTube = -1);
	long MoveOrgPosR(int iAxis, double dPos, bool bBlockMode, int iSpeed = 0);
	long MoveOrgPosMulti(stMotorPosition* pPos, int iAxis, int iSpeed = 0, bool bDoneCheck = false);
	double GetOrgPos(int iAxis, int iLens = -1, int iTube = -1);


	long CheckHomeDone(int iAxis, DWORD dwTimeout);
	long DoWork_MotionDone(int iAxis);
	bool DoWork_MotionEnable(int iAxis);
	double GetHomeVel(int iAxis);
	long SetSpeedDirect(int iAxis, double dVel, double dAcc, double dDec, int iSpeedMode = 2);
	double GetMaxSWLimit(int iAxis);
	double GetMinSWLimit(int iAxis);
	bool IsOutOfServiceMC();
	void SetInitializeMC(bool value);
	bool InitDeviceMC();
	bool IsInitializedMC();
	long DoWork_LPPreMove();
	long DoWork_LPMove(bool bUp);
	long DoWork_RotateMove(double dPos, int nType = 0);
	long StopMove(int iAxis);
	int WaitForLiftPinPosComplete(bool bUp, UINT nTimeOut);
	void ResetAlign();
	int SetGlassAlignXYR(double dX, double dY, double dR);
	int SetProbeAlignXYR(double dX, double dY, double dR, int nProbe = 0);
	int MoveProbeAlignR(double dR);
	bool IsGlassExist();
	int WorkReadySACC();
	long MoveCmdPosAging(stMotorPosition* pPos, int nAxis, int iSpeed = 0, bool bDoneCheck = true);
	long MoveCmdPosReview(int iStick, stMotorPosition* pPos, int nAxis, int iSpeed = 0, bool bDoneCheck = true);
	long MoveCmdPosWAD(stMotorPosition* pPos, int nAxis, int iSpeed = 0, bool bDoneCheck = true);
	long MoveCmdPosVision(stMotorPosition* pPos, int nAxis, int iSpeed = 0, bool bDoneCheck = true);
	long MoveCmdPosColor(stMotorPosition* pPos, int nAxis, int iSpeed = 0, bool bDoneCheck = true);
	long MoveCmdPosAngle(stMotorPosition* pPos, int nAxis, int iSpeed = 0, bool bDoneCheck = true);
	long MoveCmdAlignPos_Glass(double *pPos);
	long MoveCmdAlignPos_Probe(double* pPos);
	long MoveCmdPos_Stage(double* pPos);
	long MoveCmdPos_Head(double* pPos, int nType = 0, int nSelect = 0);
	long MoveCmdPos_Probe(double* pPos);
	long Move_SelectProbe(int iProbe, double dPos, bool bDoneCheck = true);		// 선택한 Probe를 이동한다
	long MoveCmd_SafetyAll(bool bInit = false);


	long Do_ShutterOnOff();


	// PROBE
	int MoveCmdPos_CompareMS(int iProbe);
	int MoveCmdPos_ProbeOffset(int iProbe);
	int GetCmdPos_ProbeX(stMotorPosition* pPos, int iProbe, double dPos, double dGap, BOOL bOffset = FALSE);
	int Do_PCB_UNIT_Lock(int iProbe);
	int Do_PCB_UNIT_Unlock(int iProbe);
	int Do_PROBE_CLAM_Unlock(int iProbe);
	int Do_PROBE_CLAM_Lock(int iProbe);
	int Do_Zero_Position_UnLock(int Probe);
	int Do_Zero_Position_Lock(int iProbe);

	int DoWork_Probe_Clam(int iProbe, bool bOn);
	int DoWork_Probe_PcbLock(int iProbe, bool bOn);
	int DoWork_Probe_Zero(int iProbe, bool bOn);
	int WaitForPCB_UNIT_UnlockComplete(int iProbe, UINT nTimeOut);
	int WaitForPCB_UNIT_LockComplete(int iProbe, UINT nTimeOut);
	int WaitForPROBE_CLAM_UnlockComplete(int iProbe, UINT nTimeOut);
	int WaitForPROBE_CLAM_LockComplete(int iProbe, UINT nTimeOut);
	int WaitForPROBE_Zero_Position_UnlockComplete(int iProbe, UINT nTimeOut);
	int WaitForPROBE_Zero_Position_LockComplete(int iProbe, UINT nTimeOut);
	int Is_PCB_UNIT_Unlock(int iProbe);
	int Is_PCB_UNIT_Lock(int iProbe);
	int Is_PROBE_CLAM_Unlock(int iProbe);
	int Is_PROBE_CLAM_Lock(int iProbe);
	int Is_PROBE_Zero_Unlock(int iProbe);
	int Is_PROBE_Zero_Lock(int iProbe);

	int Do_Clamp_Up();
	int Do_Clamp_Down();
	int Do_Clamp_In();
	int Do_Clamp_Out();
	int DoWork_Clamp_UpNDown(bool bUp);
	int DoWork_Clamp_InNOut(bool bIn);
	int Is_Clamp_UpNDown(bool bUp);
	int Is_Clamp_InNOut(bool bIn);
	int WaitForClamp_UpNDownComplete(bool bUp, UINT nTimeOut);
	int WaitForClamp_InNOutComplete(bool bIn, UINT nTimeOut);

	int Do_StageAir_On();
	int Do_StageAir_Off();
	int DoWork_StageAir_OnNOff(bool bOn);
	int Is_StageAir_OnNOff(bool bOn);
	int WaitForStageAir_OnNOffComplete(bool bOn, UINT nTimeOut);

	int Do_StageVac_On();
	int Do_StageVac_Off();
	int DoWork_StageVac_OnNOff(bool bOn);
	int Is_StageVac_OnNOff(bool bOn);
	int WaitForStageVac_OnNOffComplete(bool bOn, UINT nTimeOut);

//	void GetInterlockPos(double* dPosGantry, double* dPosHead, double* dPosProbe);
	void GetInterlockPos(double* dPosGantry, double* dPosHead, double* dPosProbe, double* dPosHeadB = nullptr, double* dPosLiftPin = nullptr);	// 2023.03.08 change
	long MoveCmdPos_ProbeContact(int nAxis, double fContatZ, double EContatZ, double dOD);
	long MoveCmdPos_ProbeContactAging(int nCount, double fContatZ, double EContatZ, double dOD);
	long MoveCmdPos_ProbeUnContact(int nAxis, double fContatZ);
	long MoveCmdPos_ProbeMove(int nAxis, double dMoveZ, int iSpeed);
	int  Get_ProbeExist();
};

