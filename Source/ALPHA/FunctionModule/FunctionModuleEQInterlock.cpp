//#include "stdafx.h"
#include "pch.h"
#include "FunctionModuleEQInterlock.h"

#include "LogFunction\LogAccessor\LogFileAccessor.h"
#include "..\SystemParamShare.h"
#include "..\AuroraFuncDefine.h"
#include "..\Manager\ShareMemoryMgr.h"
#include "..\Manager\MotorControlMgr.h"
#include "..\Manager\IOMapControlMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CFunctionModuleEQInterlock::CFunctionModuleEQInterlock(stAuroraMgrPool &MgrPool)
	: CBaseSequenceImpl(MgrPool, enumSeqItemID_REQ_EQ_INTERLOCK)
{
	m_strSeqType = _T("SEQUENCE_ITEM_FUNC_MODULE_CHECK_INTERLOCK");
	m_strSeqComm = _T("TCPIP");
	m_strSeqCorp = _T("HBS");
	m_strSeqName = _T("CHECK_INTERLOCK");
	m_strSeqGUID = _T("2F30C4D2-1328-4CF6-B649-7C176BD80F3A");
	m_strSeqDesc = _T("Check Interlock State");

}

CFunctionModuleEQInterlock::~CFunctionModuleEQInterlock()
{
}







int CFunctionModuleEQInterlock::GetInterlockMoveX()
{
	int nRet = VTM_RV_SUCCESSED;

	if (IsLDXLoading() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLDZLoading() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsClampFold() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsClampUnfold() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	return nRet;
}

int CFunctionModuleEQInterlock::GetInterlockMoveY()
{
	int nRet = VTM_RV_SUCCESSED;

	if (IsLDXLoading() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLDZLoading() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsClampFold() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsClampUnfold() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	return nRet;
}

int CFunctionModuleEQInterlock::GetInterlockMoveZ()
{
	int nRet = VTM_RV_SUCCESSED;

	if (IsLDXLoading() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLDZLoading() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsClampFold() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsClampUnfold() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	return nRet;
}

int CFunctionModuleEQInterlock::GetInterlockMoveLDX()
{
	int nRet = VTM_RV_SUCCESSED;

	if (IsStageSafety() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsHeadSafety() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLDZLoading() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsGlassExist() == VTM_RV_SUCCESSED)
	{
		if (IsLPUp() != VTM_RV_SUCCESSED)
		{
			return nRet;
		}
	}

	return nRet;
}

int CFunctionModuleEQInterlock::GetInterlockMoveLDY()
{
	int nRet = VTM_RV_SUCCESSED;

	if (IsStageSafety() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsHeadSafety() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLDZLoading() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsGlassExist() == VTM_RV_SUCCESSED)
	{
		if (IsLPUp() != VTM_RV_SUCCESSED)
		{
			return nRet;
		}
	}

	return nRet;
}

int CFunctionModuleEQInterlock::GetInterlockMoveLDR()
{
	int nRet = VTM_RV_SUCCESSED;

	if (IsStageSafety() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsHeadSafety() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLDZLoading() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsGlassExist() == VTM_RV_SUCCESSED)
	{
		if (IsLPUp() != VTM_RV_SUCCESSED)
		{
			return nRet;
		}
	}

	return nRet;
}

int CFunctionModuleEQInterlock::GetInterlockMoveLDZ()
{
	int nRet = VTM_RV_SUCCESSED;

	if (IsStageSafety() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsHeadSafety() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLDZContact() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLPUp() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	return nRet;
}

int CFunctionModuleEQInterlock::GetinterlockIOLP()
{
	int nRet = VTM_RV_SUCCESSED;

	if (IsStageSafety() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsHeadSafety() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLDXLoading() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLDZLoading() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	return nRet;
}

int CFunctionModuleEQInterlock::GetinterlockIOClamp()
{
	int nRet = VTM_RV_SUCCESSED;

	if (IsStageSafety() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsHeadSafety() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLDXContact() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLDZLoading() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	return nRet;
}

int CFunctionModuleEQInterlock::GetinterlockIOVacuum()
{
	int nRet = VTM_RV_SUCCESSED;

	if (IsStageSafety() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsHeadSafety() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLDXContact() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLDZContact() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLPUp() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsClampUnfold() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	return nRet;
}

int CFunctionModuleEQInterlock::GetinterlockIOAir()
{
	int nRet = VTM_RV_SUCCESSED;

	if (IsStageSafety() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsHeadSafety() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLDXContact() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLDZContact() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsLPUp() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	if (IsClampUnfold() != VTM_RV_SUCCESSED)
	{
		return nRet;
	}

	return nRet;
}

//전장팀으로 부터 버퍼를 받아 와서 확인
int CFunctionModuleEQInterlock::ChkMotionInterlock(int nType)
{
	//ACS 버퍼로 부터 해당 인터락을 가져 온 후 확인

	int nRet = VTM_RV_SUCCESSED;

	return nRet;
}










int CFunctionModuleEQInterlock::IsStageSafety()
{
	int nRet = VTM_RV_SUCCESSED;
	
	//Unladoing Sensor
	//Gantry X

	return nRet;
}

int CFunctionModuleEQInterlock::IsHeadSafety(int nAxis/* = -1*/)
{
	int nRet = VTM_RV_SUCCESSED;

	//Unladoing Sensor
	//Head
	//-1일 경우 모든 Cross


	return nRet;
}

int CFunctionModuleEQInterlock::IsLDXLoading()
{
	int nRet = VTM_RV_SUCCESSED;

	//Loader X Loading

	return nRet;
}

int CFunctionModuleEQInterlock::IsLDXContact()
{
	int nRet = VTM_RV_SUCCESSED;

	//Loader X Contact


	return nRet;
}

int CFunctionModuleEQInterlock::IsLDZLoading()
{
	int nRet = VTM_RV_SUCCESSED;

	//Loader Z Loading


	return nRet;
}

int CFunctionModuleEQInterlock::IsLDZContact()
{
	int nRet = VTM_RV_SUCCESSED;

	//Loader Z Contact


	return nRet;
}

int CFunctionModuleEQInterlock::IsLPUp()
{
	int nRet = VTM_RV_SUCCESSED;

	//All LP Up

	return nRet;
}

int CFunctionModuleEQInterlock::IsLPDown()
{
	int nRet = VTM_RV_SUCCESSED;

	//All LP Down


	return nRet;
}

int CFunctionModuleEQInterlock::IsClampFold()
{
	int nRet = VTM_RV_SUCCESSED;

	//All Clamp Fold


	return nRet;
}

int CFunctionModuleEQInterlock::IsClampUnfold()
{
	int nRet = VTM_RV_SUCCESSED;

	//All Clamp Unfold


	return nRet;
}

int CFunctionModuleEQInterlock::IsGlassExist(int nState/* = -1*/)
{
	int nRet = VTM_RV_SUCCESSED;

	//Glass Check


	return nRet;
}







//=========================================================================
// INTERFACE 관련
//=========================================================================
bool CFunctionModuleEQInterlock::CreateDoWorkSeq()
{
	return true;
}

void CFunctionModuleEQInterlock::DeleteDoWorkSeq()
{
}

bool CFunctionModuleEQInterlock::ChangeState()
{
	DeleteDoWorkSeq();

	return CreateDoWorkSeq();
}

bool CFunctionModuleEQInterlock::UpdateState()
{
	return true;
}

bool CFunctionModuleEQInterlock::LinkDataBase(CBinaryHelper *pBinaryHelper, bool bSave)
{
	return true;
}

void CFunctionModuleEQInterlock::ThreadRun()
{

}