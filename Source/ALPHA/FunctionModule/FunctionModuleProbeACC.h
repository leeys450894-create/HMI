#pragma once

#include "BaseSequenceImpl.h"

#include "..\Include\API_SACC.h"

enum enumPACCStatus
{
	enumPACCSTATUS_NONE = 0,

	// EQ 동작
	enumPACCSTATUS_GETEQPROBE,
	enumPACCSTATUS_PUTEQPROBE,

	// SACC 연동
	enumPACCSTATUS_LODING,
	enumPACCSTATUS_UNLODING,
	enumPACCSTATUS_EXCHANGE,

	enumPACCSTATUS_ALIGNPROBE,

	// 단위동작
	enumPACCSTATUS_READY_POS,
	enumPACCSTATUS_MEASURE_POS,

	enumPACCSTATUS_ZERO_LOCK_ON,
	enumPACCSTATUS_ZERO_LOCK_OFF,

	enumPACCSTATUS_PROBE_CLAMP_ON,
	enumPACCSTATUS_PROBE_CLAMP_OFF,

	enumPACCSTATUS_PROBE_PCB_ON,
	enumPACCSTATUS_PROBE_PCB_OFF,

	enumPACCSTATUS_PROBE_ZUP,


	// Auto change
	enumPACCSTATUS_AC_Y_CHANGEPOS,
	enumPACCSTATUS_AC_Y_SAFETYPOS,
	enumPACCSTATUS_AC_Y_SACCPOS,
	enumPACCSTATUS_AC_Z_UP,
	enumPACCSTATUS_AC_Z_DOWN,

	// SACC
	enumPACCSTATUS_SLOT_GET,
	enumPACCSTATUS_PACC_GET,
	enumPACCSTATUS_SLOT_PUT,
	enumPACCSTATUS_PACC_PUT,
	enumPACCSTATUS_PACC_DONE,


	enumPACCSTATUS_CHANGE_COUNT
};

class CFunctionModuleProbeACCStep;
class CFunctionModuleProbeACC : public CBaseSequenceImpl
{
	std::vector<CFunctionModuleProbeACCStep*> m_vecSeqStep;
//	std::vector<CFunctionModuleProbeACCStep*> m_vecSeqStep_GetPACCPROBE;  // jsyoon 2022.09.13 미사용 삭제
	std::vector<CFunctionModuleProbeACCStep*> m_vecSeqStep_PutEQPROBE;
	std::vector<CFunctionModuleProbeACCStep*> m_vecSeqStep_AlignPROBE;
	std::vector<CFunctionModuleProbeACCStep*> m_vecSeqStep_GetEQPROBE;
//	std::vector<CFunctionModuleProbeACCStep*> m_vecSeqStep_PutPACCPROBE; // jsyoon 2022.09.13 미사용 삭제

	std::vector<CFunctionModuleProbeACCStep*> m_vecSeqStep_Loding;
	std::vector<CFunctionModuleProbeACCStep*> m_vecSeqStep_UnLoding;
	std::vector<CFunctionModuleProbeACCStep*> m_vecSeqStep_Exchange;

	std::vector<CFunctionModuleProbeACCStep*> m_vecSeqStep_ReadyPos;
	std::vector<CFunctionModuleProbeACCStep*> m_vecSeqStep_MeasurePos;

	std::vector<CFunctionModuleProbeACCStep*> m_vecSeqStep_ZeroLockOn;
	std::vector<CFunctionModuleProbeACCStep*> m_vecSeqStep_ZeroLockOff;

	std::vector<CFunctionModuleProbeACCStep*> m_vecSeqStep_ClampOn;
	std::vector<CFunctionModuleProbeACCStep*> m_vecSeqStep_ClampOff;

	std::vector<CFunctionModuleProbeACCStep*> m_vecSeqStep_PCBOn;
	std::vector<CFunctionModuleProbeACCStep*> m_vecSeqStep_PCBOff;

	std::vector<CFunctionModuleProbeACCStep*> m_vecSeqStep_Probe_ZUp;

	std::vector<CFunctionModuleProbeACCStep*> m_vecSeqStep_AC_Y_CHANGEPOS;
	std::vector<CFunctionModuleProbeACCStep*> m_vecSeqStep_AC_Y_SAFETYPOS;
	std::vector<CFunctionModuleProbeACCStep*> m_vecSeqStep_AC_Y_SACCPOS;

	std::vector<CFunctionModuleProbeACCStep*> m_vecSeqStep_AC_ZUp;
	std::vector<CFunctionModuleProbeACCStep*> m_vecSeqStep_AC_ZDown;	

	std::vector<CFunctionModuleProbeACCStep*> m_vecSeqStep_SLOT_Get;
	std::vector<CFunctionModuleProbeACCStep*> m_vecSeqStep_PACC_Get;
	std::vector<CFunctionModuleProbeACCStep*> m_vecSeqStep_SLOT_Put;
	std::vector<CFunctionModuleProbeACCStep*> m_vecSeqStep_PACC_Put;
	std::vector<CFunctionModuleProbeACCStep*> m_vecSeqStep_PACC_DONE;

	std::vector<CFunctionModuleProbeACCStep*> m_vecSeqStepALL; // 메모리 삭제용
//=========================================================================
// INTERFACE 관련
//=========================================================================
public:
	virtual bool	CreateDoWorkSeq()										override;
	virtual void	DeleteDoWorkSeq()										override;
	virtual bool	ChangeState()											override;
	virtual bool	UpdateState()											override;
	virtual bool	LinkDataBase(CBinaryHelper *pBinaryHelper, bool bSave)	override;
	virtual void	ThreadRun()												override;

public:
	CFunctionModuleProbeACC(stAuroraMgrPool &MgrPool);
	virtual ~CFunctionModuleProbeACC();

	stAuroraMgrPool* m_pAuroraMgrPool;

	// 상태 전환 명령 시 필요할 듯?
	long m_lCurrentStep;
	
	void SetPACCStatusChangeStep(long lStep);
	long GetPACCStatusChangeStep() { return m_lCurrentStep; }
	void SetPACCStatusExitStepIndex(long lStep);

	// Probe 1 ~ 5 
	int  m_iSelectProbe;
	void SetProbe(int iProbe) { m_iSelectProbe = iProbe; }		// Defalut 0 ~
	int  GetProbe() { return m_iSelectProbe; }
	bool m_bProbeAligned[5];
	void SetProbeAlign(int iProbe, bool bSet = true) { m_bProbeAligned[iProbe] = bSet; }
	bool GetProbeAlign(int iProbe) { return m_bProbeAligned[iProbe]; }

	// Cassette 1 ~ 5
	int  m_iSelectLoadCassette;
	int  m_iSelectUnLoadCassette;
	void SetLoadCassette(int iLoadCassette) { m_iSelectLoadCassette = iLoadCassette;}	
	void SetUnLoadCassette(int iUnloadCassette) {m_iSelectUnLoadCassette = iUnloadCassette; }
	int GetLoadCassette() { return m_iSelectLoadCassette; }
	int  GetUnLoadCassette() { return m_iSelectUnLoadCassette; }  
;
	long m_lEQStatusStepStart[enumPACCSTATUS_CHANGE_COUNT];
	long m_lEQStatusStepEnd[enumPACCSTATUS_CHANGE_COUNT];

	void GetIOModuleProbeACC(PLCRDATA* pbIO);

	void GetMotionModuleProbeACC(MSDATA* pData);

	CLogFileAccessor* GetLogFileAccessorPtr() { return m_pLogFileAccessor; }
};