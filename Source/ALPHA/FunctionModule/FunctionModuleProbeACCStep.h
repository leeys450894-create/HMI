#pragma once

class CBaseSequenceImpl;
class CFunctionModuleProbeACCStep
{
protected:
	CString m_strName;
	CBaseSequenceImpl *m_pShareItem;
public:
	virtual bool Run() = 0;

public:
	CFunctionModuleProbeACCStep(CBaseSequenceImpl *pShareItem){ m_pShareItem = pShareItem; }
	virtual ~CFunctionModuleProbeACCStep(){}
};

class CFunctionModuleProbeACCStep_CheckIO : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_CheckIO(CBaseSequenceImpl *pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_CheckIO() {}
};

class CFunctionModuleProbeACCStep_RecipeCheck : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_RecipeCheck(CBaseSequenceImpl *pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_RecipeCheck() {}
};

class CFunctionModuleProbeACCStep_MoveOrigin : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_MoveOrigin(CBaseSequenceImpl *pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_MoveOrigin() {}
};

class CFunctionModuleProbeACCStep_MoveOriginCheck : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_MoveOriginCheck(CBaseSequenceImpl *pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_MoveOriginCheck() {}
};

class CFunctionModuleProbeACCStep_Complete : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_Complete(CBaseSequenceImpl *pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_Complete() {}
};

class CFunctionModuleProbeACCStep_CompleteCheck : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_CompleteCheck(CBaseSequenceImpl *pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_CompleteCheck() {}
};


class CFunctionModuleProbeACCStep_None : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_None(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_None() {}
};

class CFunctionModuleProbeACCStep_Idle : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_Idle(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_Idle() {}
};

class CFunctionModuleProbeACCStep_ACReadyPosition : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_ACReadyPosition(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_ACReadyPosition() {}
};

class CFunctionModuleProbeACCStep_SACCToACReadyPosition : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_SACCToACReadyPosition(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_SACCToACReadyPosition() {}
};


class CFunctionModuleProbeACCStep_ShutterOpen : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_ShutterOpen(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_ShutterOpen() {}
};

class CFunctionModuleProbeACCStep_SACCChangePosition : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_SACCChangePosition(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_SACCChangePosition() {}
};

class CFunctionModuleProbeACCStep_SACCGetReq : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_SACCGetReq(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_SACCGetReq() {}
};

class CFunctionModuleProbeACCStep_SACCPutReq : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_SACCPutReq(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_SACCPutReq() {}
};

class CFunctionModuleProbeACCStep_SACCExChangeReq : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_SACCExChangeReq(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_SACCExChangeReq() {}
};

class CFunctionModuleProbeACCStep_ACProbeExistSensorOn : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_ACProbeExistSensorOn(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_ACProbeExistSensorOn() {}
};

class CFunctionModuleProbeACCStep_ACProbeExistSensorOff : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_ACProbeExistSensorOff(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_ACProbeExistSensorOff() {}
};

class CFunctionModuleProbeACCStep_SACCCompleteReq : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_SACCCompleteReq(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_SACCCompleteReq() {}
};

class CFunctionModuleProbeACCStep_ProbeChangePosition : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_ProbeChangePosition(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_ProbeChangePosition() {}
};

class CFunctionModuleProbeACCStep_ACZxisUp : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_ACZxisUp(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_ACZxisUp() {}
};

class CFunctionModuleProbeACCStep_ZEROLOCK : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_ZEROLOCK(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_ZEROLOCK() {}
};

class CFunctionModuleProbeACCStep_CardProbeExistSensorOn : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_CardProbeExistSensorOn(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_CardProbeExistSensorOn() {}
};

class CFunctionModuleProbeACCStep_CardProbeExistSensorOff : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_CardProbeExistSensorOff(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_CardProbeExistSensorOff() {}
};

class CFunctionModuleProbeACCStep_CLAMPLOCK : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_CLAMPLOCK(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_CLAMPLOCK() {}
};

class CFunctionModuleProbeACCStep_PCBLOCK : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_PCBLOCK(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_PCBLOCK() {}
};

class CFunctionModuleProbeACCStep_CardExistProbe_ZMOVE : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_CardExistProbe_ZMOVE(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_CardExistProbe_ZMOVE() {}
};

class CFunctionModuleProbeACCStep_ACZxisDown : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_ACZxisDown(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_ACZxisDown() {}
};

class CFunctionModuleProbeACCStep_Align : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_Align(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_Align() {}
};

class CFunctionModuleProbeACCStep_PCBUNLOCK : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_PCBUNLOCK(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_PCBUNLOCK() {}
};

class CFunctionModuleProbeACCStep_CLAMPUNLOCK : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_CLAMPUNLOCK(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_CLAMPUNLOCK() {}
};

class CFunctionModuleProbeACCStep_ZEROUNLOCK : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_ZEROUNLOCK(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_ZEROUNLOCK() {}
};

class CFunctionModuleProbeACCStep_ReadyPos : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_ReadyPos(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_ReadyPos() {}
};

class CFunctionModuleProbeACCStep_MeasurePos : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_MeasurePos(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_MeasurePos() {}
};

// 
class CFunctionModuleProbeACCStep_SetProbeUnloadClear : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_SetProbeUnloadClear(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_SetProbeUnloadClear() {}
};

class CFunctionModuleProbeACCStep_SetProbeloadData : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_SetProbeloadData(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_SetProbeloadData() {}
};

class CFunctionModuleProbeACCStep_Mount : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_Mount(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_Mount() {}
};

class CFunctionModuleProbeACCStep_UnMount : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_UnMount(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_UnMount() {}
};

class CFunctionModuleProbeACCStep_Safety : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_Safety(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_Safety() {}
};

class CFunctionModuleProbeACCStep_WaitForMoveACZDown : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_WaitForMoveACZDown(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_WaitForMoveACZDown() {}
};

class CFunctionModuleProbeACCStep_ProbeChangeMove : public CFunctionModuleProbeACCStep
{
public:
	virtual bool Run() override;
public:
	CFunctionModuleProbeACCStep_ProbeChangeMove(CBaseSequenceImpl* pShareItem) : CFunctionModuleProbeACCStep(pShareItem) {}
	virtual ~CFunctionModuleProbeACCStep_ProbeChangeMove() {}
};