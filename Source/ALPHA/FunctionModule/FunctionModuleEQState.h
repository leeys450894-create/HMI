#pragma once

#include "..\BaseSequenceImpl.h"

enum enumEQState
{
	enumEQStateReady = 0,
	enumEQStateLoad,
	enumEQStateMeas,
	enumEQStateAbnormal,
};

class CFunctionModuleEQState : public CBaseSequenceImpl
{
public:
	//현재 장비 상태 체크
	int GetEQState();
	bool IsEQStateReady();
	bool IsEQStateMeas();
	bool IsEQStateLoad();

	//OM - 장비 별로 다 설정해줘야 될려나
	bool IsSafetySensorOn();
	bool IsSafetySensorOnStage();
	bool IsSafetySensorOnHead();
	bool IsGlassSensorOn();
	
	bool IsLiftpinSensorUp();
	bool IsLiftpinSensorDown();

	bool IsVacuumSensorOn();
	bool IsVacuumSensorOff();

	bool IsAirSensorOn();
	bool IsAirSensorOff();

	bool IsStageClampFold();
	bool IsStageClampUnfold();

	bool IsLoaderXLoadSensorOn();
	bool IsLoaderXMeasSensorOn();

	bool IsLoaderZLoadSensorOn();
	bool IsLoaderZMeasSensorOn();

	bool IsPosion0_SensorOn();
	bool IsPosion180_SensorOn();
	bool IsPosion90_SensorOn();
	bool IsPosion270_SensorOn();

	bool IsPickerVacuumSensorOn();
	bool IsPickerVacuumSensorOff();

	bool IsLoaderPosLoad();
	bool IsLoaderPosUnLoad();
	bool IsLoaderPosMeas();

	// Interlock
	bool IsEQSensorClose();
	bool IsShutterSensorClose();
	bool IsDoorSensorClose();

	bool IsLoaderArmSensorOn();
	bool IsX_RaySensorRunOn();

	// Probe Base
	bool IsProbeContactSensorOn(int iProbeUnit);		// Contact
	bool IsProbeUnContactSensorOn(int iProbeUnit);		// UnContact


	bool IsProbeExistSensorOn(int iProbeUnit);		// Probe Exist
	bool IsProbeExistSensorOff(int iProbeUnit);		// Probe Exist
	bool IsProbeZerolockSensorOn(int iProbeUnit);	// Probe Zero lock
	bool IsProbeZeroUnlockSensorOn(int iProbeUnit);	// Probe Zero unlock
	bool IsProbeClampFold(int iProbeUnit);			// Probe Clamp Fold
	bool IsProbeClampUnFold(int iProbeUnit);		// Probe Clamp UnFold
	bool IsProbePCBFold(int iProbeUnit);			// Probe Clamp Fold
	bool IsProbePCBUnFold(int iProbeUnit);			// Probe Clamp UnFold
	bool IsSafetySensorOnProbe(int iProbeUnit);

	// Probe JIG
	bool IsPositionOnAC(int iPos);	// 0:Change, 1:SAFETY, 2: SACC

	// Auto Change Base
	bool IsACZxisUp();								// AC Z Up Check
	bool IsACZxisDown();							// AC Z Down check
	bool IsACShutterOpen();							// PACC Shutter Check
	bool IsACProbeExistSensorOn();					// AC Arm Probe Exist
	bool IsACProbeExistSensorOff();					// AC Arm Probe Exist

	bool SetEQLoad(int iCassetteClot);											// EQ Load(CST 선택) 명령 전달 
	bool SetEQUnLoad(int iCassetteClot);										// EQ Unload(CST 선택) 명령 전달
	bool SetEQExchange(int iLoadCassetteClot, int iUnLoadCassetteClot);			// EQ Exchange(CST) 명령 전달
	bool SetEQCompleteOn();														// EQ Complete On 명령 전달
	bool SetEQCompleteOff();													// EQ Complete On 명령 전달
	bool SetEQClear(bool bAuto);												// EQ 명령 초기화
	bool SetEQSetPrboeData(int iProbeUnit);										// EQ Probe Unloading Data 셋팅
	bool SetEQGetPrboeData(int iProbeUnit);										// EQ Probe Loading Data 셋팅
	bool SetEQClearPrboeData(int iProbeUnit);									// EQ Probe Data 초기화
	bool SetEQManualProbeID(int iProbeUnit, CString strID);				// LYS 2026-09-16 수동 장착 프로브 ID 등록: D2110~D2150 에 10자 기록("" = 공백 = 빈 ID)

	bool SetStatusbusyOn();														// EQ Arm 구동 중
	bool SetStatusbusyOff();													// EQ Arm 구동 완료
	bool SetEQCommandStart();													// EQ Command Start 명령
	//Interlock
	int GetInterlockMoveX();
	int GetInterlockMoveY();
	int GetInterlockMoveZ();

	int GetInterlockMoveLDX();
	int GetInterlockMoveLDY();
	int GetInterlockMoveLDR();
	int GetInterlockMoveLDZ();

	int GetinterlockIOLP();
	int GetinterlockIOClamp();
	int GetinterlockIOVacuum();
	int GetinterlockIOAir();

	// SACC
	bool IsBufferLoadPos();	// SACC Buffer Pos
	bool IsBufferUnloadPos();	
	bool IsBufferLoadPosDone();	// SACC Buffer Pos
	bool IsBufferUnloadPosDone();

	int ChkMotionInterlock(int nType);					//ACS Buffer 내의 인터락 체크용

	//동작 대기, 문제가 되는 마지막 M 번지 Adresss 리턴
	bool WaitActionDone(int nStartaddress, int nAddressCount = 1, int nOffset = 1, bool bOn = true, UINT nTimeOut = 10);
	void SetLastCheckAddress(int nAddress);
	int GetLastCheckAddress();
	int m_nLastCheckAddress;
	///

//=========================================================================
// INTERFACE 관련
//=========================================================================
public:
	// Create / Delete
	virtual bool	CreateDoWorkSeq()										override;
	virtual void	DeleteDoWorkSeq()										override;
	virtual bool	ChangeState()											override;
	virtual bool	UpdateState()											override;
	virtual bool	LinkDataBase(CBinaryHelper *pBinaryHelper, bool bSave)	override;
	virtual void	ThreadRun()												override;

public:
	CFunctionModuleEQState(stAuroraMgrPool &MgrPool);
	virtual ~CFunctionModuleEQState(void);
};

