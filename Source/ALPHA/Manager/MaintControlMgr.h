#pragma once

#include "..\AuroraSubMgr.h"
#include "MotorControlMgr.h"
#include "IOMapControlMgr.h"
#include "PositionConfigMgr.h"

// LYS 2026-09-14 2Head 측정에서 짝 없는 셀의 "쉬는 헤드" 표시값.
//   측정 스텝이 Y1~Y4 목표에 이 값을 넣으면 MoveCmdPosWAD/Vision/Color/Angle 이 그 축을 Position.ini [SAFETY]
//   SafetyHeadY1(Y1/Y3) / SafetyHeadY2(Y2/Y4) 원시좌표로 바꿔 보낸다 (ApplyIdleHeadSafetyY). 실제 좌표로는 쓰이지 않는 값.
#define df_HEAD_Y_IDLE_SAFETY	(-99999.0)
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
class CMotionProbeToMcOffMove;	// LYS 2026-09-23 MC OFF Probe Z 긴급 후퇴
class CCommTcpIpFactoryMgr;
class CMaintControlMgr : public CAuroraSubMgr
{
	CIOMapControlMgr* m_pIOMapControlMgr;
	CMotorControlMgr* m_pMotorControlMgr;
	CShareMemoryMgr* m_pShareMemoryMgr;
	CPositionConfigMgr* m_pPositionConfigMgr;
	CLightControlMgr* m_pMgrLightControl;

	double m_dGlassAlignOffset[3];
	// LYS 2026-09-18 MoveHome 이 마지막으로 낸 알람 코드(0 = 없음).
	//                초기화는 축 루프에서 같은 인터락을 20번 넘게 만나므로 같은 코드는 한 번만 띄운다.
	int    m_nLastHomeAlarm;
	bool m_bGlassAlignDone = false;	// LYS 2026-09-15 Glass Align 성공 후 오프셋이 유효한 동안 true (타이틀바 표시용)
	bool m_bProbeAlignDone = false;	// LYS 2026-09-15 Probe Align 성공 후 오프셋이 유효한 동안 true
	double m_dGlassAlignRefR = 0.0;		// LYS 2026-10-13 Hybrid Phase 7 Glass Align reference R mark (Align coordinate R when AlignGlass succeeded)
	bool m_bGlassAlignRefValid = false;	// LYS 2026-10-13 Hybrid Phase 7 true from AlignGlass success until SetGlassAlignXYR / Do_StageVac_On / Do_StageVac_Off
	double m_dProbeAlignOffset[3];  // Probe Align 보정값

	double m_dAlignOffset[100];		// HW Align 보정값(Probe 장착시 보정값
	CMotionProbeToMcOffMove* m_pMcOffMove;	// LYS 2026-09-23 MC OFF 시 Probe Z 긴급 후퇴 감시 스레드(StartMotionProbeToMcOffMove 에서 생성)

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

	// LYS 2026-09-14 쉬는 헤드 -> [SAFETY]. pPos(호출부 입력)에서 df_HEAD_Y_IDLE_SAFETY 인 Y1~Y4 축을 찾아
	//   pPosY(오프셋 적용이 끝난 Y 이동 배열)의 같은 축 목표를 Safety 원시좌표로 바꾼다. 헤드 오프셋은 무시하고,
	//   뒤따르는 MoveCmdPosMulti 가 더할 m_dAlignOffset 은 미리 빼 MoveCmd_SafetyAll 과 같은 위치에 서게 한다. 반환 = 바꾼 축 수.
	static bool IsIdleHeadSafetyPos(double dPos) { return (dPos < df_HEAD_Y_IDLE_SAFETY + 1.0 && dPos > df_HEAD_Y_IDLE_SAFETY - 1.0); }
	// LYS 2026-09-14 (추가) FRONT 헤드 Safety 키 분리 - Y3/Y4 는 SafetyHeadY3/Y4 를 쓴다 (없으면 Y1/Y2 값).
	int ApplyIdleHeadSafetyY(const stMotorPosition* pPos, int nAxis, stMotorPosition* pPosY, int nPosY, LPCTSTR pszWhere);

	long SetAmpEnable(int iAxis, bool bOn);
	bool GetAmpEnable(int iAxis);
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
	bool IsConnectedMC();	// LYS 2026-08-31 both-controller ACS connect status for MOTION lamp
	long DoWork_LPPreMove();
	long DoWork_LPMove(bool bUp);
	long DoWork_RotateMove(double dPos, int nType = 0);
	long StopMove(int iAxis);
	int WaitForLiftPinPosComplete(bool bUp, UINT nTimeOut);
	void ResetAlign();
	// LYS 2026-09-15 Align 완료 상태 - Set*AlignXYR 가 호출될 때마다(시작 0 대입·리셋 포함) false 가 되고, 정렬 성공 경로에서만 true 로 세운다.
	void SetGlassAlignDone(bool bDone) { m_bGlassAlignDone = bDone; }
	void SetProbeAlignDone(bool bDone) { m_bProbeAlignDone = bDone; }
	bool IsGlassAlignDone() const { return m_bGlassAlignDone; }
	bool IsProbeAlignDone() const { return m_bProbeAlignDone; }
	// LYS 2026-10-13 Hybrid Phase 7 Glass Align reference R mark (p7x_plan (2-3), D7-3 (a)): GetCurPos(AXIS_LOADER_R) taken right after AlignGlass succeeded
	//   (Align coordinate R = that .alg StageRotate - the TZero rotation went into the offset). Cleared by SetGlassAlignXYR (align start, segment boundary,
	//   ResetAlign) and by Do_StageVac_On/Off (manual glass change). Only the hybrid manual gates read it - 1-segment recipes never do.
	void SetGlassAlignRefR(double dR) { m_dGlassAlignRefR = dR; m_bGlassAlignRefValid = true; }
	bool GetGlassAlignRefR(double* pdR) const { if (m_bGlassAlignRefValid == false || m_bGlassAlignDone == false) return false; if (pdR != nullptr) *pdR = m_dGlassAlignRefR; return true; }	// false = no mark (*pdR untouched)
	bool IsGlassAlignOrientationOk(double dTargetR, double dTol = 2.0) const { double dRef = 0.0; if (GetGlassAlignRefR(&dRef) == false) return true; double dDiff = dTargetR - dRef; if (dDiff < 0.0) dDiff = -dDiff; return dDiff <= dTol; }	// no mark -> true; D7-6 2.0 deg
	int SetGlassAlignXYR(double dX, double dY, double dR);
	int SetProbeAlignXYR(double dX, double dY, double dR, int nProbe = 0);
	int MoveProbeAlignR(double dR);
	bool IsGlassExist();
	int WorkReadySACC();
	long MoveCmdPosAging(stMotorPosition* pPos, int nAxis, int iSpeed = 0, bool bDoneCheck = true);
	// LYS 2026-09-18 bSkipSubZRetract : XY 앞 Sub Z 안전높이 후퇴를 건너뛴다.
	//                셀만 바뀌어 글래스가 제자리인 경우(TestMode Review)에 쓴다 - 기본값 false 라 기존 호출부는 그대로.
	long MoveCmdPosReview(int iStick, stMotorPosition* pPos, int nAxis, int iSpeed = 0, bool bDoneCheck = true, bool bSkipSubZRetract = false);
	long MoveCmdPosWAD(stMotorPosition* pPos, int nAxis, int iSpeed = 0, bool bDoneCheck = true);
	long MoveCmdPosVision(stMotorPosition* pPos, int nAxis, int iSpeed = 0, bool bDoneCheck = true, bool bHoldSafetyX2 = false);	// LYS 2026-10-14 bHoldSafetyX2: manual Meas keeps BACK GANTRY at safety (no head offset, no re-command when already there)
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

	// LYS 2026-09-23 MC OFF 시 Probe Z 긴급 후퇴 (IT_8.6G_HBS CMotionProbeToMcOffMove 이식). 알람 모니터 시작 시점에 DlgMainForm 이 부른다.
	void StartMotionProbeToMcOffMove(CString strLogPath, CString strConfigPath, CCommTcpIpFactoryMgr* pCommTcpIpFactoryMgr);
	void StopMotionProbeToMcOffMove();
};

