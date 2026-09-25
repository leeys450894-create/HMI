//#include "stdafx.h"
#include "pch.h"
#include "FunctionModuleEQState.h"

#include "LogFunction\LogAccessor\LogFileAccessor.h"

#include "..\SystemParamShare.h"
#include "..\AuroraFuncDefine.h"
#include "..\Manager\SequenceManager.h"
#include "..\Manager\ShareMemoryMgr.h"
#include "..\Manager\MaintControlMgr.h"
//#include "..\Manager\IOMapControlMgr.h"
#include "..\Manager\MCCLogMgr.h"
#include "..\Manager\SystemConfigMgr.h"	// LYS 2026-06-26 OCP option

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CFunctionModuleEQState::CFunctionModuleEQState(stAuroraMgrPool &MgrPool)
	: CBaseSequenceImpl(MgrPool, enumSeqItemID_REQ_EQ_STATE)
{
	m_strSeqType = _T("SEQUENCE_ITEM_FUNC_MODULE_CHECK_EQ_STATE");
	m_strSeqComm = _T("TCPIP");
	m_strSeqCorp = _T("HBS");
	m_strSeqName = _T("CHECK_EQ_STATE");
	m_strSeqGUID = _T("5A41AA54-A352-4D76-85E2-FF82B31D04A4");
	m_strSeqDesc = _T("Check EQ State");

	m_nLastCheckAddress = 0;
}


CFunctionModuleEQState::~CFunctionModuleEQState()
{
}




//모든 설비들을 어떻게 볼 것인가?(Config에서 읽어온 장비에 대해 조건 처리가 좋을 거 같긴 함)
int CFunctionModuleEQState::GetEQState()
{
	int nStatus = -1;

	if (IsEQStateReady())
	{
		nStatus = PLC_MBIT_READY;
	}
	else if(IsEQStateMeas())
	{
		nStatus = PLC_MBIT_MEAS;
	}
	else if (IsEQStateLoad())
	{
		nStatus = PLC_MBIT_LOAD;
	}
	else
	{
		nStatus = -1;
	}

	return nStatus;
}

bool CFunctionModuleEQState::IsEQStateReady()
{
	bool bRet = true;

	int nAddress = PLC_BIT_STATE_READY;
	bRet = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
	SetLastCheckAddress(nAddress);
	return bRet;

//	//OM 기준
//	//모든 축 Safety
//	bool bRet = false;
////	if (IsSafetySensorOn() && IsVacuumSensorOff() && (IsPosion0_SensorOn() || IsPosion180_SensorOn()) )
//	{
//		bRet = true;
//	}
//
//	return bRet;
}

bool CFunctionModuleEQState::IsEQStateMeas()
{
	bool bRet = true;

	int nAddress = PLC_BIT_STATE_MEASURE;
	bRet = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
	SetLastCheckAddress(nAddress);
	return bRet;

	////OM 기준
	////모든 축 Safety + Sample Stage Vacuum On
	//bool bRet = false;
	//
	//if (IsSafetySensorOn() && IsVacuumSensorOn() && ( IsPosion0_SensorOn() || IsPosion180_SensorOn()) )
	//{
	//	bRet = true;
	//}

	//return bRet;
}

bool CFunctionModuleEQState::IsEQStateLoad()
{
	bool bRet = true;

	int nAddress = PLC_BIT_STATE_LOAD;
	bRet = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
	SetLastCheckAddress(nAddress);
	return bRet;

	////OM 기준
	////모든 축 Safety
	//bool bRet = false;
	//if (IsSafetySensorOn() && IsVacuumSensorOff() && (IsPosion0_SensorOn() || IsPosion180_SensorOn()) )
	//{
	//	bRet = true;
	//}
	//return bRet;
}

bool CFunctionModuleEQState::IsSafetySensorOn()
{
	bool bRet = true;

	//Stage, Head
	bRet &= IsSafetySensorOnStage();
	if(bRet == false) return false;
	bRet &= IsSafetySensorOnHead();
	if (bRet == false) return false;

	return bRet;
}

bool CFunctionModuleEQState::IsSafetySensorOnStage()
{
	bool bRet = true;

//	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_CHK_MOTION_SAFETY, true);

//	int nAddress = IO_BOARD_READ_INPUT_013;
	int nAddress = PLC_BIT_UNLOAD_SENSOR03;
	bRet = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
	SetLastCheckAddress(nAddress);

//	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_CHK_MOTION_SAFETY, false);

	return bRet;
}

bool CFunctionModuleEQState::IsSafetySensorOnHead()
{
	bool bRet = true;

	// Gantry
	for (int i = 0; i < 2; i++)
	{
//		int nAddress = IO_BOARD_READ_INPUT_014 + i;
		int nAddress = PLC_BIT_UNLOAD_SENSOR01 + i;
		bRet &= GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
		if (bRet == false)
		{
			SetLastCheckAddress(nAddress);
			return false;
		}
	}

	//// HEAD
	//for (int i = 0; i < 4; i++)
	//{
	//	int nAddress = PLC_BIT_UNLOAD_HEAD_SENSOR01 + i;
	//	bRet &= GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
	//	if (bRet == false)
	//	{
	//		SetLastCheckAddress(nAddress);
	//		return false;
	//	}
	//}

	//// PROBE
	//for (int i = 0; i < 5; i++)
	//{
	//	int nAddress = PLC_BIT_UNLOAD_PROBE_SENSOR01 + i;
	//	bRet &= GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
	//	if (bRet == false)
	//	{
	//		SetLastCheckAddress(nAddress);
	//		return false;
	//	}
	//}
	
	return bRet;
}

bool CFunctionModuleEQState::IsGlassSensorOn()
{
	bool bRet = false;
	int nRet = 0;
	//for (int i = 0; i < 4; i++)
	//{
	//	int nAddress = PLC_BIT_GLASS_EXIST_01 + i;
	//
	//	nRet = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
	//	if (nRet != -1)
	//	{
	//		if (nRet == 1)
	//		{
	//			bRet |= true;
	//		}
	//		else
	//		{
	//			bRet |= false;
	//		}
	//	}
	//}

	// LYS 2026-09-05 Sample On Glass Check
	int nAddress = PLC_BIT_GLASS_EXIST_05;

	nRet = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
	if (nRet != -1)
	{
		if (nRet == 1)
		{
			bRet |= true;
		}
		else
		{
			bRet |= false;
		}
	}

	return bRet;
}

bool CFunctionModuleEQState::IsLiftpinSensorUp()
{
	bool bRet = true;
	int nRet = 0;
	int nAddress = PLC_BIT_LIFT_PIN_SENSOR_UP01;
	nRet = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
	if (nRet != -1)
	{
		if (nRet == 1)
		{
			bRet = true;
		}
		else
		{
			bRet = false;
		}
	}

	SetLastCheckAddress(nAddress);

	//for (int i = 0; i < 4; i++)
	//{
	//	int nAddress = PLC_IMP_B_LIFT_PIN_01_UP + i * 2;
	//	bRet &= GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
	//	if(bRet == false)
	//	{
	//		SetLastCheckAddress(nAddress);
	//		return false;
	//	}
	//}

	return bRet;
}

bool CFunctionModuleEQState::IsLiftpinSensorDown()
{
	bool bRet = true;
	int nRet = 0;
	int nAddress = PLC_BIT_LIFT_PIN_SENSOR_DOWN01;
	nRet = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
	if (nRet != -1)
	{
		if (nRet == 1)
		{
			bRet = true;
		}
		else
		{
			bRet = false;
		}
	}

	SetLastCheckAddress(nAddress);

	/*for (int i = 0; i < 4; i++)
	{
		int nAddress = PLC_IMP_B_LIFT_PIN_01_DOWN + i * 2;
		bRet &= GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
		if (bRet == false)
		{
			SetLastCheckAddress(nAddress);
			return false;
		}
	}*/

	return bRet;
}

bool CFunctionModuleEQState::IsVacuumSensorOn()
{
	bool bRet = false;
	int nRet = 0;
	for (int i = 0; i < 2; i++)
	{
		int nAddress = PLC_BIT_VACCUM_PLATE01 + i;
		nRet = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
		if (nRet != -1)
		{
			if (nRet == 1)
			{
				bRet |= true;
			}
			else
			{
				bRet |= false;
			}
		}

		SetLastCheckAddress(nAddress);
	}

	return bRet;
}

bool CFunctionModuleEQState::IsVacuumSensorOff()
{
	bool bRet = false;
	int nRet = 0;
	for (int i = 0; i < 2; i++)
	{
		int nAddress = PLC_BIT_VACCUM_PLATE01 + i;
		nRet = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
		if (nRet != -1)
		{
			if (nRet == 1)
			{
				bRet |= true;
			}
			else
			{
				bRet |= false;
			}
		}

		SetLastCheckAddress(nAddress);
	}

	if (bRet == true)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool CFunctionModuleEQState::IsAirSensorOn()
{
	bool bRet = false;

	return bRet;
}

bool CFunctionModuleEQState::IsAirSensorOff()
{
	bool bRet = false;

	return bRet;
}

bool CFunctionModuleEQState::IsStageClampFold()
{
	bool bRet = true;
#ifdef _USE_NOT
	for (int i = 0; i < 2; i++)
	{
		int nAddress = PLC_IMP_B_FRONT_CLAMP_ON_01 + i * 2;
		bRet &= GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
		if (bRet == false)
		{
			SetLastCheckAddress(nAddress);
			return false;
		}
	}

	for (int i = 0; i < 4; i++)
	{
		int nAddress = PLC_IMP_B_RIGHT_CLAMP_ON_01 + i * 2;
		bRet &= GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
		if (bRet == false)
		{
			SetLastCheckAddress(nAddress);
			return false;
		}
	}

	for (int i = 0; i < 2; i++)
	{
		int nAddress = PLC_IMP_B_BACK_CLAMP_ON_01 + i * 2;
		bRet &= GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
		if (bRet == false)
		{
			SetLastCheckAddress(nAddress);
			return false;
		}
	}

	for (int i = 0; i < 4; i++)
	{
		int nAddress = PLC_IMP_B_LEFT_CLAMP_ON_01 + i * 2;
		bRet &= GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
		if (bRet == false)
		{
			SetLastCheckAddress(nAddress);
			return false;
		}
	}
#endif // _USE_NOT
	return bRet;
}

bool CFunctionModuleEQState::IsStageClampUnfold()
{
	bool bRet = true;

#ifdef _USE_NOT
	for (int i = 0; i < 2; i++)
	{
		int nAddress = PLC_IMP_B_FRONT_CLAMP_OFF_01 + i * 2;
		bRet &= GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
		if (bRet == false)
		{
			SetLastCheckAddress(nAddress);
			return false;
		}
	}

	for (int i = 0; i < 4; i++)
	{
		int nAddress = PLC_IMP_B_RIGHT_CLAMP_OFF_01 + i * 2;
		bRet &= GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
		if (bRet == false)
		{
			SetLastCheckAddress(nAddress);
			return false;
		}
	}

	for (int i = 0; i < 2; i++)
	{
		int nAddress = PLC_IMP_B_BACK_CLAMP_OFF_01 + i * 2;
		bRet &= GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
		if (bRet == false)
		{
			SetLastCheckAddress(nAddress);
			return false;
		}
	}

	for (int i = 0; i < 4; i++)
	{
		int nAddress = PLC_IMP_B_LEFT_CLAMP_OFF_01 + i * 2;
		bRet &= GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
		if (bRet == false)
		{
			SetLastCheckAddress(nAddress);
			return false;
		}
	}
#endif // _USE_NOT
	return bRet;
}

bool CFunctionModuleEQState::IsLoaderXLoadSensorOn()
{
	bool bRet = false;
#ifdef _USE_NOT
	int nAddress = PLC_IMP_B_LDX_LOADING_POSITION;
	bRet = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
	SetLastCheckAddress(nAddress);
#endif // _USE_NOT
	return bRet;
}

bool CFunctionModuleEQState::IsLoaderXMeasSensorOn()
{
	bool bRet = false;
#ifdef _USE_NOT
	int nAddress = PLC_IMP_B_LDX_CONTACT_POSITION;
	bRet = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
	SetLastCheckAddress(nAddress);
#endif // _USE_NOT
	return bRet;
}

bool CFunctionModuleEQState::IsLoaderZLoadSensorOn()
{
	bool bRet = false;
#ifdef _USE_NOT
	int nAddress = PLC_IMP_B_LDZ_LOADING_POSITION;
	bRet = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
	SetLastCheckAddress(nAddress);
#endif // _USE_NOT
	return bRet;
}

bool CFunctionModuleEQState::IsLoaderZMeasSensorOn()
{
	bool bRet = false;
#ifdef _USE_NOT
	int nAddress = PLC_IMP_B_LDZ_CONTACT_POSITION;
	bRet = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
	SetLastCheckAddress(nAddress);
#endif // _USE_NOT

	return bRet;
}

bool CFunctionModuleEQState::IsPosion0_SensorOn()
{
	bool bRet = false;

	int nAddress = PLC_R_MOTION_LOADER_DEGREE_0; // IO_BOARD_READ_INPUT_028;
	bRet = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
	SetLastCheckAddress(nAddress);

	return bRet;
}

bool CFunctionModuleEQState::IsPosion180_SensorOn()
{
	bool bRet = false;

	int nAddress = PLC_R_MOTION_LOADER_DEGREE_2; // IO_BOARD_READ_INPUT_029;
	bRet = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
	SetLastCheckAddress(nAddress);

	return bRet;
}

bool CFunctionModuleEQState::IsPosion90_SensorOn()
{
	bool bRet = false;

	int nAddress = PLC_R_MOTION_LOADER_DEGREE_1;
	bRet = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
	SetLastCheckAddress(nAddress);

	return bRet;
}

bool CFunctionModuleEQState::IsPosion270_SensorOn()
{
	bool bRet = false;

	int nAddress = PLC_R_MOTION_LOADER_DEGREE_3;
	bRet = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
	SetLastCheckAddress(nAddress);

	return bRet;
}

bool CFunctionModuleEQState::IsPickerVacuumSensorOn()
{
	bool bRet = false;

#ifdef _USE_NOT
	int nAddress = IO_BOARD_READ_INPUT_110;
	bRet = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
	SetLastCheckAddress(nAddress);
#endif // _USE_NOT

	return bRet;
}

bool CFunctionModuleEQState::IsPickerVacuumSensorOff()
{
	bool bRet = false;

	int nAddress = PLC_BIT_VACCUM_PLATE01; // IO_BOARD_READ_INPUT_110;
	bRet = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
	SetLastCheckAddress(nAddress);

	return !bRet;
}

bool CFunctionModuleEQState::IsProbeContactSensorOn(int iProbeUnit)
{
	bool bRet = false;

//	iProbeUnit -= 1;

	// LYS 2026-06-26 OCP mode: P1-P4 always return false
	if (GetSystemConfigMgrPtr() && GetSystemConfigMgrPtr()->IsOCP())
	{
		if (iProbeUnit != df_PROBE_MEAS)
			return false;
	}


	if (GetIOMapControlMgrPtr()->IsOutOfService() == true)
	{
		return false;
	}

	int nProbeUnit = PLC_R_MOTION_PROBE_UNIT01_POS_04;

	switch (iProbeUnit)
	{
	case df_PROBE_AGING1:
		nProbeUnit = PLC_R_MOTION_PROBE_UNIT01_POS_04;
		break;
	case df_PROBE_AGING2:
		nProbeUnit = PLC_R_MOTION_PROBE_UNIT02_POS_04;
		break;
	case df_PROBE_AGING3:
		nProbeUnit = PLC_R_MOTION_PROBE_UNIT03_POS_04;
		break;
	case df_PROBE_AGING4:
		nProbeUnit = PLC_R_MOTION_PROBE_UNIT04_POS_04;
		break;
	case df_PROBE_MEAS:
		nProbeUnit = PLC_R_MOTION_PROBE_UNIT05_POS_04;
		break;
	default:
		break;
	}

	int nBit = 0;
	int nAddress = nProbeUnit;
	nBit = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
	if (nBit != -1)
	{
		if (nBit == 1)
			bRet = true;
		else
			bRet = false;
	}

	SetLastCheckAddress(nAddress);

	return bRet;
}

bool CFunctionModuleEQState::IsProbeUnContactSensorOn(int iProbeUnit)
{
	bool bRet = false;

//	iProbeUnit -= 1;

	if (GetIOMapControlMgrPtr()->IsOutOfService() == true)
	{
		return true;
	}

	int nProbeUnit = PLC_R_MOTION_PROBE_UNIT01_POS_04;

	switch (iProbeUnit)
	{
	case df_PROBE_AGING1:
		nProbeUnit = PLC_R_MOTION_PROBE_UNIT01_POS_04;
		break;
	case df_PROBE_AGING2:
		nProbeUnit = PLC_R_MOTION_PROBE_UNIT02_POS_04;
		break;
	case df_PROBE_AGING3:
		nProbeUnit = PLC_R_MOTION_PROBE_UNIT03_POS_04;
		break;
	case df_PROBE_AGING4:
		nProbeUnit = PLC_R_MOTION_PROBE_UNIT04_POS_04;
		break;
	case df_PROBE_MEAS:
		//nProbeUnit = PLC_R_MOTION_PROBE_UNIT05_POS_04;
		nProbeUnit = PLC_R_MOTION_PROBE_UNIT01_POS_01;		// LYS 2026-09-05 Contact sensor 프로브 변경
		break;
	default:
		break;
	}

	int nBit = 0;
	int nAddress = nProbeUnit;
	nBit = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
	if (nBit != -1)
	{
		if (nBit == 1)
			bRet = true;
		else
			bRet = false;
	}

	SetLastCheckAddress(nAddress);

	// LYS 2026-09-09 언컨택 임시 해제
	bRet = true;

	return bRet;
}

bool CFunctionModuleEQState::IsProbeExistSensorOn(int iProbeUnit)
{
	bool bRet = false;

//	iProbeUnit -= 1;

	if (GetIOMapControlMgrPtr()->IsOutOfService() == true)
	{
		// 2023.12.22 change
		if (iProbeUnit == df_PROBE_MEAS)
			return true;
		else
			return false;

	//	if( iProbeUnit == df_PROBE_AGING3 || iProbeUnit == df_PROBE_AGING4 || iProbeUnit == df_PROBE_MEAS)	// 2022.12.21 for Test BT
	//	if (iProbeUnit == df_PROBE_AGING2 || iProbeUnit == df_PROBE_AGING3 || iProbeUnit == df_PROBE_AGING4 || iProbeUnit == df_PROBE_MEAS)	// 2022.12.21 for Test DG
	/*		return true;
		else
			return false;*/
	}

	int nProbeUnit = PLC_R_PROBE_UNIT01_CARD_CHK_01;

	switch (iProbeUnit)
	{
	case df_PROBE_AGING1:
		nProbeUnit = PLC_R_PROBE_UNIT01_CARD_CHK_01;
		break;
	case df_PROBE_AGING2:
		nProbeUnit = PLC_R_PROBE_UNIT02_CARD_CHK_01;
		break;
	case df_PROBE_AGING3:
		nProbeUnit = PLC_R_PROBE_UNIT03_CARD_CHK_01;
		break;
	case df_PROBE_AGING4:
		nProbeUnit = PLC_R_PROBE_UNIT04_CARD_CHK_01;
		break;
	case df_PROBE_MEAS:
		nProbeUnit = PLC_R_PROBE_UNIT05_CARD_CHK_01;
		break;
	default:
		break;
	}

	for (int i = 0; i < 4; i++)
	{
		int nAddress = nProbeUnit +  i;
		bRet |= GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);	
		SetLastCheckAddress(nAddress);
	}

	bRet = true;

	// LYS 2026-09-09 언컨택 신호 임시 해제
	//if (bRet == false)
	//{
	//	return false;
	//}
	//else
	//{
	//	return true;
	//}
}

bool CFunctionModuleEQState::IsProbeExistSensorOff(int iProbeUnit)
{
	bool bRet = false;

//	iProbeUnit -= 1;

	if (GetIOMapControlMgrPtr()->IsOutOfService() == true)
	{
		return true;
	}

	int nProbeUnit = PLC_R_PROBE_UNIT01_CARD_CHK_01;

	switch (iProbeUnit)
	{
	case df_PROBE_AGING1:
		nProbeUnit = PLC_R_PROBE_UNIT01_CARD_CHK_01;
		break;
	case df_PROBE_AGING2:
		nProbeUnit = PLC_R_PROBE_UNIT02_CARD_CHK_01;
		break;
	case df_PROBE_AGING3:
		nProbeUnit = PLC_R_PROBE_UNIT03_CARD_CHK_01;
		break;
	case df_PROBE_AGING4:
		nProbeUnit = PLC_R_PROBE_UNIT04_CARD_CHK_01;
		break;
	case df_PROBE_MEAS:
		nProbeUnit = PLC_R_PROBE_UNIT05_CARD_CHK_01;
		break;
	default:
		break;
	}

	for (int i = 0; i < 4; i++)
	{
		int nAddress = nProbeUnit + i;
		bRet |= GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
		SetLastCheckAddress(nAddress);
	}

	if (bRet == true)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool CFunctionModuleEQState::IsProbeZerolockSensorOn(int iProbeUnit)
{
	bool bRet = false;

	int nProbeUnit = PLC_R_PROBE_UNIT01_CARD_ZERO_01;

	switch (iProbeUnit)
	{
	case df_PROBE_AGING1:
		nProbeUnit = PLC_R_PROBE_UNIT01_CARD_ZERO_01;
		break;
	case df_PROBE_AGING2:
		nProbeUnit = PLC_R_PROBE_UNIT02_CARD_ZERO_01;
		break;
	case df_PROBE_AGING3:
		nProbeUnit = PLC_R_PROBE_UNIT03_CARD_ZERO_01;
		break;
	case df_PROBE_AGING4:
		nProbeUnit = PLC_R_PROBE_UNIT04_CARD_ZERO_01;
		break;
	case df_PROBE_MEAS:
		nProbeUnit = PLC_R_PROBE_UNIT05_CARD_ZERO_01;
		break;
	default:
		break;
	}

	for (int i = 0; i < 4; i++)
	{
		int nAddress = nProbeUnit + i;
		bRet |= GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
		SetLastCheckAddress(nAddress);
	}

	if (bRet == false)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool CFunctionModuleEQState::IsProbeZeroUnlockSensorOn(int iProbeUnit)
{
	bool bRet = false;

	int nProbeUnit = PLC_R_PROBE_UNIT01_CARD_ZERO_01;

	switch (iProbeUnit)
	{
	case df_PROBE_AGING1:
		nProbeUnit = PLC_R_PROBE_UNIT01_CARD_ZERO_01;
		break;
	case df_PROBE_AGING2:
		nProbeUnit = PLC_R_PROBE_UNIT02_CARD_ZERO_01;
		break;
	case df_PROBE_AGING3:
		nProbeUnit = PLC_R_PROBE_UNIT03_CARD_ZERO_01;
		break;
	case df_PROBE_AGING4:
		nProbeUnit = PLC_R_PROBE_UNIT04_CARD_ZERO_01;
		break;
	case df_PROBE_MEAS:
		nProbeUnit = PLC_R_PROBE_UNIT05_CARD_ZERO_01;
		break;
	default:
		break;
	}

	for (int i = 0; i < 4; i++)
	{
		int nAddress = nProbeUnit + i;
		bRet |= GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
		SetLastCheckAddress(nAddress);
	}

	if (bRet == true)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool CFunctionModuleEQState::IsProbeClampFold(int iProbeUnit)
{
	bool bRetLock = true;
	bool bRetUnLock = false;

	int nProbeUnitLock = PLC_R_PROBE_UNIT01_CLAM_LOCK_01;
	int nProbeUnitUnLock = PLC_R_PROBE_UNIT01_CLAM_UNLOCK_01;

	switch (iProbeUnit)
	{
	case df_PROBE_AGING1:
		nProbeUnitLock = PLC_R_PROBE_UNIT01_CLAM_LOCK_01;
		nProbeUnitUnLock = PLC_R_PROBE_UNIT01_CLAM_UNLOCK_01;
		break;
	case df_PROBE_AGING2:
		nProbeUnitLock = PLC_R_PROBE_UNIT02_CLAM_LOCK_01;
		nProbeUnitUnLock = PLC_R_PROBE_UNIT02_CLAM_UNLOCK_01;
		break;
	case df_PROBE_AGING3:
		nProbeUnitLock = PLC_R_PROBE_UNIT03_CLAM_LOCK_01;
		nProbeUnitUnLock = PLC_R_PROBE_UNIT03_CLAM_UNLOCK_01;
		break;
	case df_PROBE_AGING4:
		nProbeUnitLock = PLC_R_PROBE_UNIT04_CLAM_LOCK_01;
		nProbeUnitUnLock = PLC_R_PROBE_UNIT04_CLAM_UNLOCK_01;
		break;
	case df_PROBE_MEAS:
		nProbeUnitLock = PLC_R_PROBE_UNIT05_CLAM_LOCK_01;
		nProbeUnitUnLock = PLC_R_PROBE_UNIT05_CLAM_UNLOCK_01;
		break;
	default:
		break;
	}

	// Lock
	for (int i = 0; i < 2; i++)
	{
		int nAddress = nProbeUnitLock + i;
		bRetLock &= GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
		SetLastCheckAddress(nAddress);
	}

	// UnLock
	for (int i = 0; i < 2; i++)
	{
		int nAddress = nProbeUnitUnLock + i;
		bRetUnLock |= GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
		SetLastCheckAddress(nAddress);
	}

	if (bRetLock == true && bRetUnLock == false)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool CFunctionModuleEQState::IsProbeClampUnFold(int iProbeUnit)
{
	bool bRetLock = true;
	bool bRetUnLock = false;

	int nProbeUnitLock = PLC_R_PROBE_UNIT01_CLAM_LOCK_01;
	int nProbeUnitUnLock = PLC_R_PROBE_UNIT01_CLAM_UNLOCK_01;

	switch (iProbeUnit)
	{
	case df_PROBE_AGING1:
		nProbeUnitLock = PLC_R_PROBE_UNIT01_CLAM_LOCK_01;
		nProbeUnitUnLock = PLC_R_PROBE_UNIT01_CLAM_UNLOCK_01;
		break;
	case df_PROBE_AGING2:
		nProbeUnitLock = PLC_R_PROBE_UNIT02_CLAM_LOCK_01;
		nProbeUnitUnLock = PLC_R_PROBE_UNIT02_CLAM_UNLOCK_01;
		break;
	case df_PROBE_AGING3:
		nProbeUnitLock = PLC_R_PROBE_UNIT03_CLAM_LOCK_01;
		nProbeUnitUnLock = PLC_R_PROBE_UNIT03_CLAM_UNLOCK_01;
		break;
	case df_PROBE_AGING4:
		nProbeUnitLock = PLC_R_PROBE_UNIT04_CLAM_LOCK_01;
		nProbeUnitUnLock = PLC_R_PROBE_UNIT04_CLAM_UNLOCK_01;
		break;
	case df_PROBE_MEAS:
		nProbeUnitLock = PLC_R_PROBE_UNIT05_CLAM_LOCK_01;
		nProbeUnitUnLock = PLC_R_PROBE_UNIT05_CLAM_UNLOCK_01;
		break;
	default:
		break;
	}

	// Lock
	for (int i = 0; i < 2; i++)
	{
		int nAddress = nProbeUnitLock + i;
		bRetLock |= GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
		SetLastCheckAddress(nAddress);
	}

	// UnLock
	for (int i = 0; i < 2; i++)
	{
		int nAddress = nProbeUnitUnLock + i;
		bRetUnLock &= GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
		SetLastCheckAddress(nAddress);
	}

	if (bRetLock == false && bRetUnLock == true)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool CFunctionModuleEQState::IsProbePCBFold(int iProbeUnit)
{
	bool bRetLock = true;
	bool bRetUnLock = false;

	int nProbeUnitLock = PLC_R_PROBE_UNIT01_PCB_LOCK_01;
	int nProbeUnitUnLock = PLC_R_PROBE_UNIT01_PCB_UNLOCK_01;

	switch (iProbeUnit)
	{
	case df_PROBE_AGING1:
		nProbeUnitLock = PLC_R_PROBE_UNIT01_PCB_LOCK_01;
		nProbeUnitUnLock = PLC_R_PROBE_UNIT01_PCB_UNLOCK_01;
		break;
	case df_PROBE_AGING2:
		nProbeUnitLock = PLC_R_PROBE_UNIT02_PCB_LOCK_01;
		nProbeUnitUnLock = PLC_R_PROBE_UNIT02_PCB_UNLOCK_01;
		break;
	case df_PROBE_AGING3:
		nProbeUnitLock = PLC_R_PROBE_UNIT03_PCB_LOCK_01;
		nProbeUnitUnLock = PLC_R_PROBE_UNIT03_PCB_UNLOCK_01;
		break;
	case df_PROBE_AGING4:
		nProbeUnitLock = PLC_R_PROBE_UNIT04_PCB_LOCK_01;
		nProbeUnitUnLock = PLC_R_PROBE_UNIT04_PCB_UNLOCK_01;
		break;
	case df_PROBE_MEAS:
		nProbeUnitLock = PLC_R_PROBE_UNIT05_PCB_LOCK_01;
		nProbeUnitUnLock = PLC_R_PROBE_UNIT05_PCB_UNLOCK_01;
		break;
	default:
		break;
	}

	// Lock
	for (int i = 0; i < 2; i++)
	{
		int nAddress = nProbeUnitLock + i;
		bRetLock &= GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
		SetLastCheckAddress(nAddress);
	}

	// UnLock
	for (int i = 0; i < 2; i++)
	{
		int nAddress = nProbeUnitUnLock + i;
		bRetUnLock |= GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
		SetLastCheckAddress(nAddress);
	}

	if (bRetLock == true && bRetUnLock == false)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool CFunctionModuleEQState::IsProbePCBUnFold(int iProbeUnit)
{
	bool bRetLock = true;
	bool bRetUnLock = false;

	int nProbeUnitLock = PLC_R_PROBE_UNIT01_PCB_LOCK_01;
	int nProbeUnitUnLock = PLC_R_PROBE_UNIT01_PCB_UNLOCK_01;

	switch (iProbeUnit)
	{
	case df_PROBE_AGING1:
		nProbeUnitLock = PLC_R_PROBE_UNIT01_PCB_LOCK_01;
		nProbeUnitUnLock = PLC_R_PROBE_UNIT01_PCB_UNLOCK_01;
		break;
	case df_PROBE_AGING2:
		nProbeUnitLock = PLC_R_PROBE_UNIT02_PCB_LOCK_01;
		nProbeUnitUnLock = PLC_R_PROBE_UNIT02_PCB_UNLOCK_01;
		break;
	case df_PROBE_AGING3:
		nProbeUnitLock = PLC_R_PROBE_UNIT03_PCB_LOCK_01;
		nProbeUnitUnLock = PLC_R_PROBE_UNIT03_PCB_UNLOCK_01;
		break;
	case df_PROBE_AGING4:
		nProbeUnitLock = PLC_R_PROBE_UNIT04_PCB_LOCK_01;
		nProbeUnitUnLock = PLC_R_PROBE_UNIT04_PCB_UNLOCK_01;
		break;
	case df_PROBE_MEAS:
		nProbeUnitLock = PLC_R_PROBE_UNIT05_PCB_LOCK_01;
		nProbeUnitUnLock = PLC_R_PROBE_UNIT05_PCB_UNLOCK_01;
		break;
	default:
		break;
	}

	// Lock
	for (int i = 0; i < 2; i++)
	{
		int nAddress = nProbeUnitLock + i;
		bRetLock |= GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
		SetLastCheckAddress(nAddress);
	}

	// UnLock
	for (int i = 0; i < 2; i++)
	{
		int nAddress = nProbeUnitUnLock + i;
		bRetUnLock &= GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
		SetLastCheckAddress(nAddress);
	}

	if (bRetLock == false && bRetUnLock == true)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool CFunctionModuleEQState::IsSafetySensorOnProbe(int iProbeUnit)
{
	bool bRet = false;

	if (iProbeUnit == -1)
	{
		bRet = true;

		for (size_t i = df_PROBE_AGING1; i <= df_PROBE_MEAS; i++)
		{
			bRet &= IsSafetySensorOnProbe(i);
		}

		// LYS 2026-08-04 OCP mode: P1-P4 always return true
		if (GetSystemConfigMgrPtr() && GetSystemConfigMgrPtr()->IsOCP())
		{
			bRet &= IsSafetySensorOnProbe(df_PROBE_MEAS);
		}
		
		return bRet;
	}

	int nProbeUnit = PLC_R_PROBE_UNIT01_SAFETY_POS;

	switch (iProbeUnit)
	{
	case df_PROBE_AGING1:
		nProbeUnit = PLC_R_PROBE_UNIT01_SAFETY_POS;
		break;
	case df_PROBE_AGING2:
		nProbeUnit = PLC_R_PROBE_UNIT02_SAFETY_POS;
		break;
	case df_PROBE_AGING3:
		nProbeUnit = PLC_R_PROBE_UNIT03_SAFETY_POS;
		break;
	case df_PROBE_AGING4:
		nProbeUnit = PLC_R_PROBE_UNIT04_SAFETY_POS;
		break;
	case df_PROBE_MEAS:
		nProbeUnit = PLC_R_PROBE_UNIT05_SAFETY_POS;
		break;
	default:
		break;
	}

	int nAddress = nProbeUnit;
	bRet = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
	SetLastCheckAddress(nAddress);
	return bRet;
}


bool CFunctionModuleEQState::IsEQSensorClose()
{
	bool bRet = true;

	//Shuuter, Door
	bRet &= IsShutterSensorClose();

	if (bRet == true)
	{
		bRet &= IsDoorSensorClose();
	}

	return bRet;
}

bool CFunctionModuleEQState::IsShutterSensorClose()
{
	bool bRet = false;

	if (GetIOMapControlMgrPtr()->IsOutOfService() != true)
	{
		int nAddress = PLC_BIT_SUTTER_SENSOR_CLOSE;
		bRet = ((GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(PLC_BIT_SUTTER_SENSOR_CLOSE) == true) &&
			    (GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(PLC_BIT_SUTTER_SENSOR_OPEN) != true) );
	}
	else
	{
		bRet = true;
	}

	return bRet;
}

bool CFunctionModuleEQState::IsDoorSensorClose()
{
	bool bRet = false;

	int nRet = 0;
	for (int i = 0; i < 8; i++)
	{
		int nAddress = PLC_BIT_DOOR_SENSOR1 + i;

		nRet = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
		if (nRet != 1)
		{
			if (nRet == 1)
				bRet |= true;
		}
		
		SetLastCheckAddress(nAddress);
	}

	return !bRet;
}

bool CFunctionModuleEQState::IsLoaderArmSensorOn()
{
	bool bRet = false;

	if (GetIOMapControlMgrPtr()->IsOutOfService() != true)
	{
		int nAddress = PLC_LOADER_ARM_SENSOR_01;
		int nRet = 0;
		nRet = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
		SetLastCheckAddress(nAddress);

		if (nRet != -1)
		{
			if (nRet == 1)
			{
				bRet = false;
			}
			else
			{
				bRet = true;
			}
		}
	}
	else
	{
		bRet = false;
	}

	return bRet;
}

bool CFunctionModuleEQState::IsX_RaySensorRunOn()
{
	bool bRet = false;

	if (GetIOMapControlMgrPtr()->IsOutOfService() != true)
	{
		long nRet = 0;
		for (int i = 0; i < 2; i++)
		{
			int nAddress = PLC_BIT_XIONIZER_REMOTE_STATE_01 + (i * 5);

			nRet = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
			if (nRet != -1)
			{
				if (nRet == 1)
				{
					bRet |= true;
				}
			}

			SetLastCheckAddress(nAddress);
		}
	}
	else
	{
		bRet = false;
	}

	return bRet;
}

bool CFunctionModuleEQState::IsPositionOnAC(int iPos)
{
	bool bRet = false;

	int nProbeUnit = PLC_AUTO_CHANGE_X0_POSITION_04;

	switch (iPos)
	{
	case 0:
		nProbeUnit = PLC_AUTO_CHANGE_X0_POSITION_02;
		break;
	case 1:
		nProbeUnit = PLC_AUTO_CHANGE_X0_POSITION_03;
		break;
	case 2:
		nProbeUnit = PLC_AUTO_CHANGE_X0_POSITION_04;
		break;
	default:
		break;
	}

	if (GetIOMapControlMgrPtr()->IsOutOfService() != true)
	{
		int nAddress = nProbeUnit;
		bRet = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
		SetLastCheckAddress(nAddress);
	}
	else
	{
		bRet = true;
	}
	

	return bRet;
}

bool CFunctionModuleEQState::IsACZxisUp()
{
	bool bRet = false;

	int nAddress = PLC_AUTOCHANGE_UP_01;
	bRet = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
	SetLastCheckAddress(nAddress);

	return bRet;
}

bool CFunctionModuleEQState::IsACZxisDown()
{
	bool bRet = false;

	int nAddress = PLC_AUTOCHANGE_DOWN_01;
	bRet = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
	SetLastCheckAddress(nAddress);

	return bRet;
}

bool CFunctionModuleEQState::IsACShutterOpen()
{
	bool bRet = false;

	if (GetIOMapControlMgrPtr()->IsOutOfService() != true)
	{
		int nAddress = PLC_R_PACC_SHUTTER_OPEN_01;
		bRet = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
		SetLastCheckAddress(nAddress);
	}
	else
	{
		bRet = true;
	}

	return bRet;
}

bool CFunctionModuleEQState::IsACProbeExistSensorOn()
{
	bool bRet = false;

	if (GetIOMapControlMgrPtr()->IsOutOfService() != true)
	{
		int nAddress = PLC_AUTOCHANGE_PROBE_CHECK1;
		bRet = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
		SetLastCheckAddress(nAddress);
	}
	else
	{
		bRet = true;
	}


	//int nAddress = PLC_AUTOCHANGE_PROBE_CHECK1;
	//bRet = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
	//SetLastCheckAddress(nAddress);

	return bRet;	
}

bool CFunctionModuleEQState::IsACProbeExistSensorOff()
{
	bool bRet = false;

	if (GetIOMapControlMgrPtr()->IsOutOfService() != true)
	{
		int nAddress = PLC_AUTOCHANGE_PROBE_CHECK1;
		if (GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress) != true)
		{
			bRet = true;
		}
		SetLastCheckAddress(nAddress);
	}
	else
	{
		bRet = true;
	}


	//int nAddress = PLC_AUTOCHANGE_PROBE_CHECK1;
	//bRet = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
	//SetLastCheckAddress(nAddress);

	return bRet;
}

bool CFunctionModuleEQState::SetEQLoad(int iCassetteClot)
{
	bool bRet = false;

	// Command 변환 
	// 1****110000****01
	TCHAR	szCmd[1000];
	szCmd[0] = 0x00;

	BYTE nByte = 0x00;
	int val = 0, i = 0;

	//// Start
	//_tcscat_s(szCmd, _countof(szCmd), _T("1"));

	// Reserve
	_tcscat_s(szCmd, _countof(szCmd), _T("000"));

	// Loading / Unloading / Exchange 
	_tcscat_s(szCmd, _countof(szCmd), _T("001"));

	// Unloading
	val = 0;
	for (i = 0; i < 4; i++)
	{
		nByte = (val >> (3 - i)) & 0x01;
		if (nByte == 0)
			_tcscat_s(szCmd, _countof(szCmd), _T("0"));
		else
			_tcscat_s(szCmd, _countof(szCmd), _T("1"));
	}
	
	// Loading
	val = iCassetteClot;
	for (i = 0; i < 4; i++)
	{
		nByte = (val >> (3 - i)) & 0x01;
		if (nByte == 0)
			_tcscat_s(szCmd, _countof(szCmd), _T("0"));
		else
			_tcscat_s(szCmd, _countof(szCmd), _T("1"));
	}

	// Auto Mode
	_tcscat_s(szCmd, _countof(szCmd), _T("01"));
	

//	int iLen = strlen(szCmd);
	int iLen = _tcsclen(szCmd);
	szCmd[iLen] = 0;

	int iCmd = 0, iTemp = 0;
	for (i = 0; i < iLen; i++)
	{
		if (szCmd[iLen - i - 1] & 0x01)
		{
			iTemp = 1;
		}
		else
		{
			iTemp = 0;
		}

		iCmd += (int)(pow((double)2, (double)(i)) * iTemp);
	}

	bRet = GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_EQ_AUTO_MODE, iCmd, iLen);

	if( bRet == true)
		bRet = SetEQCommandStart();

	if (bRet != true)
		SetEQClear(false);

	return bRet;
}

bool CFunctionModuleEQState::SetEQUnLoad(int iCassetteClot)
{
	bool bRet = false;

	// Command 변환 
// 1****110000****01
	TCHAR	szCmd[1000];
	szCmd[0] = 0x00;

	BYTE nByte = 0x00;
	int val = 0, i = 0;

	//// Start
	//_tcscat_s(szCmd, _countof(szCmd), _T("1"));

	// Reserve
	_tcscat_s(szCmd, _countof(szCmd), _T("000"));

	// Loading / Unloading / Exchange 
	_tcscat_s(szCmd, _countof(szCmd), _T("010"));

	// Unloading
	val = iCassetteClot;
	for (i = 0; i < 4; i++)
	{
		nByte = (val >> (3 - i)) & 0x01;
		if (nByte == 0)
			_tcscat_s(szCmd, _countof(szCmd), _T("0"));
		else
			_tcscat_s(szCmd, _countof(szCmd), _T("1"));
	}

	// Loading
	val = 0;
	for (i = 0; i < 4; i++)
	{
		nByte = (val >> (3 - i)) & 0x01;
		if (nByte == 0)
			_tcscat_s(szCmd, _countof(szCmd), _T("0"));
		else
			_tcscat_s(szCmd, _countof(szCmd), _T("1"));
	}

	// Auto Mode
	_tcscat_s(szCmd, _countof(szCmd), _T("01"));


	//	int iLen = strlen(szCmd);
	int iLen = _tcsclen(szCmd);
	szCmd[iLen] = 0;

	int iCmd = 0, iTemp = 0;
	for (i = 0; i < iLen; i++)
	{
		if (szCmd[iLen - i - 1] & 0x01)
		{
			iTemp = 1;
		}
		else
		{
			iTemp = 0;
		}

		iCmd += (int)(pow((double)2, (double)(i)) * iTemp);
	}

	bRet = GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_EQ_AUTO_MODE, iCmd, iLen);

	if (bRet == true)
		bRet = SetEQCommandStart();

	if (bRet != true)
		SetEQClear(false);

	return bRet;
}

bool CFunctionModuleEQState::SetEQCommandStart()
{
	bool bRet = false;

	// Command Start On
	if (GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_START, true) == true) 
	{
		bRet = WaitActionDone(PLC_PACC_COMMAND_CHECK_ACK, 1, 1, true, PLC_IO_ACTION_DONE_TIME);
	}
	else
	{
		bRet = false;
	}
		
	// Command Start Off
	if (bRet == true)
		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_START, false);

	return bRet;
}

bool CFunctionModuleEQState::SetEQExchange(int iLoadCassetteClot, int iUnLoadCassetteClot)
{
	bool bRet = false;

	// Command 변환 
// 1****110000****01
	TCHAR	szCmd[1000];
	szCmd[0] = 0x00;

	BYTE nByte = 0x00;
	int val = 0, i = 0;

	//// Start
	//_tcscat_s(szCmd, _countof(szCmd), _T("1"));

	// Reserve
	_tcscat_s(szCmd, _countof(szCmd), _T("000"));

	// Loading / Unloading / Exchange 
	_tcscat_s(szCmd, _countof(szCmd), _T("100"));

	// Unloading
	val = iUnLoadCassetteClot;
	for (i = 0; i < 4; i++)
	{
		nByte = (val >> (3 - i)) & 0x01;
		if (nByte == 0)
			_tcscat_s(szCmd, _countof(szCmd), _T("0"));
		else
			_tcscat_s(szCmd, _countof(szCmd), _T("1"));
	}

	// Loading
	val = iLoadCassetteClot;
	for (i = 0; i < 4; i++)
	{
		nByte = (val >> (3 - i)) & 0x01;
		if (nByte == 0)
			_tcscat_s(szCmd, _countof(szCmd), _T("0"));
		else
			_tcscat_s(szCmd, _countof(szCmd), _T("1"));
	}

	// Auto Mode
	_tcscat_s(szCmd, _countof(szCmd), _T("01"));


	//	int iLen = strlen(szCmd);
	int iLen = _tcsclen(szCmd);
	szCmd[iLen] = 0;

	int iCmd = 0, iTemp = 0;
	for (i = 0; i < iLen; i++)
	{
		if (szCmd[iLen - i - 1] & 0x01)
		{
			iTemp = 1;
		}
		else
		{
			iTemp = 0;
		}

		iCmd += (int)(pow((double)2, (double)(i)) * iTemp);
	}

	bRet = GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_EQ_AUTO_MODE, iCmd, iLen);

	//// Load CST Slot Select
	//switch (iLoadCassetteClot)
	//{
	//	case 0:
	//	{
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_LOAD_SLOT01, false);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_LOAD_SLOT02, false);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_LOAD_SLOT03, false);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_LOAD_SLOT04, false);
	//		break;
	//	}
	//	case 1:
	//	{
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_LOAD_SLOT01, false);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_LOAD_SLOT02, false);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_LOAD_SLOT03, false);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_LOAD_SLOT04, true);
	//		break;
	//	}
	//	case 2:
	//	{
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_LOAD_SLOT01, false);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_LOAD_SLOT02, false);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_LOAD_SLOT03, true);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_LOAD_SLOT04, false);
	//		break;
	//	}
	//	case 3:
	//	{
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_LOAD_SLOT01, false);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_LOAD_SLOT02, true);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_LOAD_SLOT03, false);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_LOAD_SLOT04, false);
	//		break;
	//	}
	//	case 4:
	//	{
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_LOAD_SLOT01, true);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_LOAD_SLOT02, false);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_LOAD_SLOT03, false);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_LOAD_SLOT04, false);
	//		break;
	//	}
	//	case 5:
	//	{
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_UNLOAD_SLOT01, true);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_UNLOAD_SLOT02, false);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_UNLOAD_SLOT03, false);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_UNLOAD_SLOT04, true);
	//		break;
	//	}
	//}

	//// UnLoad CST Slot Select
	//switch (iUnLoadCassetteClot)
	//{
	//	case 0:
	//	{
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_UNLOAD_SLOT01, false);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_UNLOAD_SLOT02, false);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_UNLOAD_SLOT03, false);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_UNLOAD_SLOT04, false);
	//		break;
	//	}
	//	case 1:
	//	{
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_UNLOAD_SLOT01, false);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_UNLOAD_SLOT02, false);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_UNLOAD_SLOT03, false);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_UNLOAD_SLOT04, true);
	//		break;
	//	}
	//	case 2:
	//	{
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_UNLOAD_SLOT01, false);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_UNLOAD_SLOT02, false);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_UNLOAD_SLOT03, true);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_UNLOAD_SLOT04, false);
	//		break;
	//	}
	//	case 3:
	//	{
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_UNLOAD_SLOT01, false);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_UNLOAD_SLOT02, true);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_UNLOAD_SLOT03, false);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_UNLOAD_SLOT04, false);
	//		break;
	//	}
	//	case 4:
	//	{
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_UNLOAD_SLOT01, true);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_UNLOAD_SLOT02, false);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_UNLOAD_SLOT03, false);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_UNLOAD_SLOT04, false);
	//		break;
	//	}
	//	case 5:
	//	{
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_UNLOAD_SLOT01, true);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_UNLOAD_SLOT02, false);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_UNLOAD_SLOT03, false);
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_UNLOAD_SLOT04, true);
	//		break;
	//	}
	//}

	//// ExChange Command
	//GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_EXCHSANGE, true);

	// Command Start On
	if (bRet == true)
		bRet = SetEQCommandStart();

	if (bRet != true)
		SetEQClear(false);

	//if (GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_START, true) == true)
	//{
	//	bRet = WaitActionDone(PLC_PACC_COMMAND_CHECK_ACK);

	//	if (bRet)
	//		// Command Start Off
	//		GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_COMMAND_START, false);
	//	else
	//		bRet = false;
	//}
	//else
	//	bRet = false;

	return bRet;
}

bool CFunctionModuleEQState::SetEQCompleteOn()
{
	bool bRet = false;

	bRet = GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_STATUS_COMPLETE, true);

	return bRet;
}

bool CFunctionModuleEQState::SetEQCompleteOff()
{
	bool bRet = false;

	bRet = GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_STATUS_COMPLETE, false);

	return bRet;
}

bool CFunctionModuleEQState::SetStatusbusyOn()
{
	bool bRet = false;

	bRet = GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_STATUS_BUSY, true);

	return bRet;
}

bool CFunctionModuleEQState::SetStatusbusyOff()
{
	bool bRet = false;

	bRet = GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_EQ_STATUS_BUSY, false);

	return bRet;
}

bool CFunctionModuleEQState::IsLoaderPosLoad()
{
	bool bRet = false;

	return bRet;
}

bool CFunctionModuleEQState::IsLoaderPosUnLoad()
{
	bool bRet = false;

	return bRet;
}

bool CFunctionModuleEQState::IsLoaderPosMeas()
{
	bool bRet = false;

	return bRet;
}

int CFunctionModuleEQState::GetInterlockMoveX()
{
	int nRet = VTM_RV_SUCCESSED;

	if (IsLoaderXLoadSensorOn() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLoaderZLoadSensorOn() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsStageClampFold() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsStageClampUnfold() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	return nRet;
}

int CFunctionModuleEQState::GetInterlockMoveY()
{
	int nRet = VTM_RV_SUCCESSED;

	if (IsLoaderXLoadSensorOn() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLoaderZLoadSensorOn() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsStageClampFold() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsStageClampUnfold() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	return nRet;
}

int CFunctionModuleEQState::GetInterlockMoveZ()
{
	int nRet = VTM_RV_SUCCESSED;

	if (IsLoaderXLoadSensorOn() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLoaderZLoadSensorOn() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsStageClampFold() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsStageClampUnfold() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	return nRet;
}

int CFunctionModuleEQState::GetInterlockMoveLDX()
{
	int nRet = VTM_RV_SUCCESSED;

	if (IsSafetySensorOnStage() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsSafetySensorOnHead() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLoaderZLoadSensorOn() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsGlassSensorOn() == VTM_RV_SUCCESSED)
	{
		if (IsLiftpinSensorUp() != VTM_RV_SUCCESSED)
		{
			return nRet;
		}
	}

	return nRet;
}

int CFunctionModuleEQState::GetInterlockMoveLDY()
{
	int nRet = VTM_RV_SUCCESSED;

	if (IsSafetySensorOnStage() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsSafetySensorOnHead() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLoaderZLoadSensorOn() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsGlassSensorOn() == VTM_RV_SUCCESSED)
	{
		if (IsLiftpinSensorUp() != VTM_RV_SUCCESSED)
		{
			return nRet;
		}
	}

	return nRet;
}

int CFunctionModuleEQState::GetInterlockMoveLDR()
{
	int nRet = VTM_RV_SUCCESSED;

	if (IsSafetySensorOnStage() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsSafetySensorOnHead() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLoaderZLoadSensorOn() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsGlassSensorOn() == VTM_RV_SUCCESSED)
	{
		if (IsLiftpinSensorUp() != VTM_RV_SUCCESSED)
		{
			return nRet;
		}
	}

	return nRet;
}

int CFunctionModuleEQState::GetInterlockMoveLDZ()
{
	int nRet = VTM_RV_SUCCESSED;

	if (IsSafetySensorOnStage() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsSafetySensorOnHead() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLoaderZMeasSensorOn() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLiftpinSensorUp() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	return nRet;
}

int CFunctionModuleEQState::GetinterlockIOLP()
{
	int nRet = VTM_RV_SUCCESSED;

	if (IsSafetySensorOnStage() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsSafetySensorOnHead() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLoaderXLoadSensorOn() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLoaderZLoadSensorOn() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	return nRet;
}

int CFunctionModuleEQState::GetinterlockIOClamp()
{
	int nRet = VTM_RV_SUCCESSED;

	if (IsSafetySensorOnStage() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsSafetySensorOnHead() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLoaderXMeasSensorOn() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLoaderZLoadSensorOn() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	return nRet;
}

int CFunctionModuleEQState::GetinterlockIOVacuum()
{
	int nRet = VTM_RV_SUCCESSED;

	if (IsSafetySensorOnStage() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsSafetySensorOnHead() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLoaderXMeasSensorOn() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLoaderZMeasSensorOn() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLiftpinSensorUp() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsStageClampUnfold() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	return nRet;
}

int CFunctionModuleEQState::GetinterlockIOAir()
{
	int nRet = VTM_RV_SUCCESSED;

	if (IsSafetySensorOnStage() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsSafetySensorOnHead() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLoaderXMeasSensorOn() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLoaderZMeasSensorOn() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLiftpinSensorUp() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsStageClampUnfold() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	return nRet;
}

//전장팀으로 부터 버퍼를 받아 와서 확인
int CFunctionModuleEQState::ChkMotionInterlock(int nType)
{
	//ACS 버퍼로 부터 해당 인터락을 가져 온 후 확인

	int nRet = VTM_RV_SUCCESSED;

	return nRet;
}



//nStartaddress : 시작주소, nAddressCount : 확인할 Bit 수, nOffset : 확인할 Bit의 간격, bOn = 확인할 Bit의 상태
//확인할 주소들이 Up, Down 순으로 되어 있어 현재 방식으로 추가
bool CFunctionModuleEQState::WaitActionDone(int nStartaddress, int nAddressCount/* = 1*/, int nOffset/* = 1*/, bool bOn/* = true*/, UINT nTimeOut/* = 10*/)
{
	m_nLastCheckAddress = 0;

	bool bRet = true;

	for (int i = 0; i < nAddressCount; i++)
	{
		int nAddress = nStartaddress + i * nOffset;


		if (nStartaddress == PLC_BIT_STATE_READY || nStartaddress == PLC_BIT_STATE_MEASURE)
		{
			if (GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(PLC_BIT_STATE_ALARM) == 1)
			{
				bRet = GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_ALRAM_RESET_STATUS, 1, 1);
				Sleep(200);
				GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_BIT_ALRAM_RESET_STATUS, 0, 1);
			}
		}

		bRet &= GetIOMapControlMgrPtr()->CheckActionDone(nAddress, nTimeOut, bOn);

		if (!bRet)
		{
			m_nLastCheckAddress = nAddress;

			return false;
		}

	}

	return bRet;
}

void CFunctionModuleEQState::SetLastCheckAddress(int nAddress)
{
	m_nLastCheckAddress = nAddress;
}

int CFunctionModuleEQState::GetLastCheckAddress()
{
	return m_nLastCheckAddress;
}



//=========================================================================
// INTERFACE 관련
//=========================================================================
bool CFunctionModuleEQState::CreateDoWorkSeq()
{
	return true;
}

void CFunctionModuleEQState::DeleteDoWorkSeq()
{
}

bool CFunctionModuleEQState::ChangeState()
{
	DeleteDoWorkSeq();

	return CreateDoWorkSeq();
}

bool CFunctionModuleEQState::UpdateState()
{
	return true;
}

bool CFunctionModuleEQState::LinkDataBase(CBinaryHelper *pBinaryHelper, bool bSave)
{
	return true;
}

void CFunctionModuleEQState::ThreadRun()
{

}

bool CFunctionModuleEQState::SetEQClear(bool bAuto)
{
	bool bRet = false;

	// Command 변환 
// 1****110000****01
	TCHAR	szCmd[1000];
	szCmd[0] = 0x00;

	BYTE nByte = 0x00;
	int val = 0, i = 0;

	_tcscat_s(szCmd, _countof(szCmd), _T("0000"));

	// Reserve
	_tcscat_s(szCmd, _countof(szCmd), _T("000"));

	// Loading / Unloading / Exchange 
	_tcscat_s(szCmd, _countof(szCmd), _T("000"));

	// Unloading
	val = 0;
	for (i = 0; i < 4; i++)
	{
		nByte = (val >> (3 - i)) & 0x01;
		if (nByte == 0)
			_tcscat_s(szCmd, _countof(szCmd), _T("0"));
		else
			_tcscat_s(szCmd, _countof(szCmd), _T("1"));
	}

	// Loading
	val = 0;
	for (i = 0; i < 4; i++)
	{
		nByte = (val >> (3 - i)) & 0x01;
		if (nByte == 0)
			_tcscat_s(szCmd, _countof(szCmd), _T("0"));
		else
			_tcscat_s(szCmd, _countof(szCmd), _T("1"));
	}

	// Auto Mode
	if(bAuto == true)
		_tcscat_s(szCmd, _countof(szCmd), _T("01"));
	else
		_tcscat_s(szCmd, _countof(szCmd), _T("10"));


	//	int iLen = strlen(szCmd);
	int iLen = _tcsclen(szCmd);
	szCmd[iLen] = 0;

	int iCmd = 0, iTemp = 0;
	for (i = 0; i < iLen; i++)
	{
		if (szCmd[iLen - i - 1] & 0x01)
		{
			iTemp = 1;
		}
		else
		{
			iTemp = 0;
		}

		iCmd += (int)(pow((double)2, (double)(i)) * iTemp);
	}


	bRet = GetIOMapControlMgrPtr()->SetMemoryNBitToDevice(PLC_EQ_AUTO_MODE, iCmd, 20);
	return bRet;
}

// LYS 2026-09-16 수동 장착 프로브 ID 등록: SACC 화면 SET ID / CLEAR ID -> 해당 프로브 모듈 워드 블록(D2110~D2150, P5 = D2150)에 10자 기록.
//                빈 문자열이면 공백 10자(= PLC 의 빈 ID 표현, SetMemoryStr 이 NUL 을 공백으로 보냄). ProbeCard 스캐너가 약 5초 주기로 다시 읽어 m_szPROBEID 에 반영된다.
//                로딩 시퀀스의 SetEQGetPrboeData 와 같은 쓰기 경로이며, 언로드 시 SACCPutReq 가 이 값을 D2100 에 복사해 PLC 가 로봇 동작에 쓴다.
bool CFunctionModuleEQState::SetEQManualProbeID(int iProbeUnit, CString strID)
{
	CString strLog;
	if (iProbeUnit < df_PROBE_AGING1 || iProbeUnit > df_PROBE_MEAS)
	{
		strLog.Format(_T("SetEQManualProbeID NG - probe %d out of range"), iProbeUnit);
		// (이 모듈의 로그 accessor 는 Initialize 되지 않으므로 여기서 KMLOG 하지 않는다 - 호출측 CSACCMgr 가 SEQ SACC 로그에 남김)
		return false;
	}

	strID.Trim();
	if (strID.GetLength() > 10)
		strID = strID.Left(10);

	TCHAR sz[100];
	ZeroMemory(&sz, sizeof(sz));
	_tcscpy_s(sz, _countof(sz), strID);

	CString strAddr;
	strAddr.Format(_T("D%d"), 2100 + 10 * iProbeUnit);	// P1 = D2110 ... P5 = D2150
	bool bRet = GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(strAddr, sz, 10);

	strLog.Format(_T("SetEQManualProbeID probe=%d %s id=[%s] -> %s"), iProbeUnit, strAddr, strID, bRet ? _T("OK") : _T("NG"));
	// 로그는 호출측 CSACCMgr 가 SEQ SACC 로그에 남긴다(이 모듈의 accessor 는 미초기화)
	return bRet;
}

// loading 시 장비내 Probe ID 기록
bool CFunctionModuleEQState::SetEQGetPrboeData(int iProbeUnit)
{
	bool bRet = true;

	TCHAR sz[100];
	ZeroMemory(&sz, sizeof(sz));

	CString strloadProbeID, strProbeID01, strProbeID02, strProbeID03, strProbeID04, strProbeID05;
	strloadProbeID.Empty();
	strProbeID01.Empty();
	strProbeID02.Empty();
	strProbeID03.Empty();
	strProbeID04.Empty();
	strProbeID05.Empty();

	VTM_SYSTEM* pShare = nullptr;
	pShare = GetShareMemoryMgrPtr()->GetSharedMemory();
	if (pShare != nullptr)
	{
//		strUnloadProbeID.Format(_T("%s"), pShare->m_szBufferPROBEID[0]);
		strloadProbeID.Format(_T("%s"), pShare->m_szLoadPROBEID);
	}

	switch (iProbeUnit)
	{
	case 0:
	{
		strProbeID01 = strloadProbeID;

		_tcscpy_s(sz, _countof(sz), strProbeID01);		bRet &= GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2110"), sz, 10);		// Probe Module #1
	}
	break;
	case 1:
	{
		strProbeID02 = strloadProbeID;

		_tcscpy_s(sz, _countof(sz), strProbeID02);		bRet &= GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2120"), sz, 10);		// Probe Module #2
	}
	break;
	case 2:
	{
		strProbeID03 = strloadProbeID;

		_tcscpy_s(sz, _countof(sz), strProbeID03);		bRet &= GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2130"), sz, 10);		// Probe Module #3
	}
	break;
	case 3:
	{
		strProbeID04 = strloadProbeID;

		_tcscpy_s(sz, _countof(sz), strProbeID04);		bRet &= GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2140"), sz, 10);		// Probe Module #4
	}
	break;
	case 4:
	{
		strProbeID05 = strloadProbeID;

		_tcscpy_s(sz, _countof(sz), strProbeID05);		bRet &= GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2150"), sz, 10);		// Probe Module #5
	}
	break;
	}

	return bRet;
}

// Unloading 시 장비내 Probe ID 기록
bool CFunctionModuleEQState::SetEQSetPrboeData(int iProbeUnit)
{
	bool bRet = true;

	TCHAR sz[100];
	ZeroMemory(&sz, sizeof(sz));

#if 0	// LYS 2026-09-25 5벌 복사 + switch -> 접근자 (iProbeUnit 0 기반 -> 유닛 1..5, 범위 밖이면 빈 문자열로 종전과 동일)
	CString strUnloadProbeID, strProbeID01, strProbeID02, strProbeID03, strProbeID04, strProbeID05;
	strProbeID01.Empty();
	strProbeID02.Empty();
	strProbeID03.Empty();
	strProbeID04.Empty();
	strProbeID05.Empty();

	VTM_SYSTEM* pShare = nullptr;
	pShare = GetShareMemoryMgrPtr()->GetSharedMemory();
	if (pShare != nullptr)
	{
		strProbeID01.Format(_T("%s"), pShare->m_szPROBEID[0]);
		strProbeID02.Format(_T("%s"), pShare->m_szPROBEID[1]);
		strProbeID03.Format(_T("%s"), pShare->m_szPROBEID[2]);
		strProbeID04.Format(_T("%s"), pShare->m_szPROBEID[3]);
		strProbeID05.Format(_T("%s"), pShare->m_szPROBEID[4]);
	}

	switch (iProbeUnit)
	{
	case 0:
	{
		strUnloadProbeID = strProbeID01;
	}
	break;
	case 1:
	{
		strUnloadProbeID = strProbeID02;
	}
	break;
	case 2:
	{
		strUnloadProbeID = strProbeID03;
	}
	break;
	case 3:
	{
		strUnloadProbeID = strProbeID04;
	}
	break;
	case 4:
	{
		strUnloadProbeID = strProbeID05;
	}
	break;
	}
#else
	CString strUnloadProbeID = GetShareMemoryMgrPtr()->GetEqProbeID(iProbeUnit + 1);	// LYS 2026-09-25
#endif

	_tcscpy_s(sz, _countof(sz), strUnloadProbeID);	bRet &= GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2100"), sz, 10);			// EQ Unload Data
	//_stprintf_s(sz, _countof(sz), _T("%s"), strProbeID01);		bRet &= GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2110"), sz, 10);		// Probe Module #1
	//_stprintf_s(sz, _countof(sz), _T("%s"), strProbeID02);		bRet &= GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2120"), sz, 10);		// Probe Module #2
	//_stprintf_s(sz, _countof(sz), _T("%s"), strProbeID03);		bRet &= GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2130"), sz, 10);		// Probe Module #3
	//_stprintf_s(sz, _countof(sz), _T("%s"), strProbeID04);		bRet &= GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2140"), sz, 10);		// Probe Module #4
	//_stprintf_s(sz, _countof(sz), _T("%s"), strProbeID04);		bRet &= GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2150"), sz, 10);		// Probe Module #5

	return bRet;
}

// Unloading 후 내용 삭제
bool CFunctionModuleEQState::SetEQClearPrboeData(int iProbeUnit)
{
	bool bRet = true;

	TCHAR sz[100];
	ZeroMemory(&sz, sizeof(sz));
	
	switch (iProbeUnit)
	{
	case df_PROBE_AGING1:
	{
		bRet &= GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2110"), sz, 10);		// Probe Module #1
	}
	break;
	case df_PROBE_AGING2:
	{
		bRet &= GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2120"), sz, 10);		// Probe Module #2
	}
	break;
	case df_PROBE_AGING3:
	{
		bRet &= GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2130"), sz, 10);		// Probe Module #3
	}
	break;
	case df_PROBE_AGING4:
	{
		bRet &= GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2140"), sz, 10);		// Probe Module #4
	}
	break;
	case df_PROBE_MEAS:
	{
		bRet &= GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2150"), sz, 10);		// Probe Module #5
	}
	break;
	}

	bRet &= GetIOMapControlMgrPtr()->SetMemoryNStrToDevice(_T("D2100"), sz, 10);		// EQ Unload Data

	return bRet;
}

bool CFunctionModuleEQState::IsBufferLoadPos()
{
	bool bRet = false;

	if (GetIOMapControlMgrPtr()->IsOutOfService() != true)
	{
		int nAddress = PLC_R_PACC_BUFFER_01_UP_01;

		int nBit = 0;
		for (size_t i = 0; i < 2; i++)
		{
			nAddress = PLC_R_PACC_BUFFER_01_UP_01 + i;
			nBit = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
			if (nBit != -1)
			{
				if (nBit == 1)
				{
					bRet |= true;
				}
			}

			SetLastCheckAddress(nAddress);
		}
	}
	else
	{
		bRet = true;
	}

	return bRet;
}

bool CFunctionModuleEQState::IsBufferUnloadPos()
{
	bool bRet = false;

	int nAddress = PLC_R_PACC_BUFFER_02_DOWN_01;

	int nBit = 0;
	for (size_t i = 0; i < 2; i++)
	{
		nAddress = PLC_R_PACC_BUFFER_02_DOWN_01 + i;
		nBit = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
		if (nBit != -1)
		{
			if (nBit == 1)
			{
				bRet |= true;
			}
		}

		SetLastCheckAddress(nAddress);
	}

	return bRet;
}

bool CFunctionModuleEQState::IsBufferLoadPosDone()
{
	bool bRet = false;

	if (GetIOMapControlMgrPtr()->IsOutOfService() != true)
	{
		int nAddress = PLC_R_PACC_BUFFER_01_DOWN_01;

		int nBit = 0;
		for (size_t i = 0; i < 2; i++)
		{
			nAddress = PLC_R_PACC_BUFFER_01_DOWN_01 + i;
			nBit = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
			if (nBit != -1)
			{
				if (nBit == 1)
				{
					bRet |= true;
				}
			}

			SetLastCheckAddress(nAddress);
		}
	}
	else
	{
		bRet = true;
	}

	return bRet;
}

bool CFunctionModuleEQState::IsBufferUnloadPosDone()
{
	bool bRet = false;

	int nAddress = PLC_R_PACC_BUFFER_02_UP_01;

	int nBit = 0;
	for (size_t i = 0; i < 2; i++)
	{
		nAddress = PLC_R_PACC_BUFFER_02_UP_01 + i;
		nBit = GetIOMapControlMgrPtr()->GetParam_DIOMapMemoryBitImport(nAddress);
		if (nBit != -1)
		{
			if (nBit == 1)
			{
				bRet |= true;
			}
		}

		SetLastCheckAddress(nAddress);
	}

	return bRet;
}