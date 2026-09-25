#include "pch.h"
#include "DoWorkGlassSWAlignStep.h"

#include "DoWorkGlassSequenceImpl.h"
#include "..\SystemParamShare.h"
#include "..\Manager\SystemConfigMgr.h"
#include "..\Manager\AlignCameraMgr.h"
#include "..\Manager\LightControlMgr.h"
#include "..\Manager\IOMapControlMgr.h"
#include "..\Manager\PositionConfigMgr.h"
#include "..\Manager\SeqTactTimeMgr.h"
#include "..\Manager\CommTcpIpFactoryMgr.h"

#include "..\Recipe\RecipeEditMgr.h"
#include "..\Manager\SequenceManager.h"
#include "..\Manager\MaintControlMgr.h"
#include "..\FunctionModule\FunctionModuleCentering.h"
//#include "Alarm\AlarmIDList.h"
//#include "AlarmIDList.h"  // jsyoon 2022.01,.19 change
#include "..\AuroraFuncDefine.h"
#include "..\Manager\MCCLogMgr.h"
#include "..\Manager\SequenceManager.h"
#include "..\FunctionModule\FunctionModuleEQState.h"

//#include "AlignMatrixMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

bool CDoWorkGlassSWAlignStep::GetComplete()
{
	return m_pShareItem->GetComplete();
}

int CDoWorkGlassSWAlignStep::MoveAlignPosition(int nHead, int nIndex)
{
	int nRet = df_MOTION_RV_SUCCESSED;

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("[MoveAlignPos] ENTER"));
	int nMeasMode = 0;
	int iLampIntensity[2];
	int iExposureTime[2];
	int iCCDGain[2];

	ZeroMemory(&iLampIntensity, sizeof(iLampIntensity));
	ZeroMemory(&iExposureTime, sizeof(iExposureTime));
	ZeroMemory(&iCCDGain, sizeof(iCCDGain));

	S_Align* pAlignInfo = nullptr;
	pAlignInfo = &m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeMgrPtr()->m_sAlign;

	if (pAlignInfo != nullptr)
	{
		iLampIntensity[0] = pAlignInfo->m_sCcdSet.m_sFrontLeft.m_nLed;
		iLampIntensity[1] = pAlignInfo->m_sCcdSet.m_sFrontRight.m_nLed;

		iExposureTime[0] = pAlignInfo->m_sCcdSet.m_sFrontLeft.m_nExposureTime;
		iExposureTime[1] = pAlignInfo->m_sCcdSet.m_sFrontRight.m_nExposureTime;

		iCCDGain[0] = pAlignInfo->m_sCcdSet.m_sFrontLeft.m_nGain;
		iCCDGain[1] = pAlignInfo->m_sCcdSet.m_sFrontRight.m_nGain;
	}

	// Set CCD and Light
	int iCh[4] = { 3, 2, 1, 0 };

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("[MoveAlignPos] set CCD exposure/gain/LED"));
	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_CHNG_LAMP, true);

	for (size_t i = 0; i < 2; i++)
	{
		m_pShareItem->GetLightControlMgrPtr()->SetLedLampIntensity(0, iCh[i], iLampIntensity[i]); // SPO
		m_pShareItem->GetAlignCameraMgr(i)->SetParam_CameraExposureTime_ms(iExposureTime[i]);
		m_pShareItem->GetAlignCameraMgr(i)->SetParam_CameraGain(iCCDGain[i]);
	}

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_CHNG_LAMP, false);

	// Position
	double dPosX = 0., dPosX2 = 925 / 2 , dPosY1 = 750, dPosY2 = -750, dPosR = 0, dPosZ = 0;
	double dPosZR = 0., dPosZL = 0.;	// LYS 2026-09-05 Sub Z RIGHT/LEFT(Z3/Z4) 목표 - 구 BACK Z(Z1) 대체
	double dGlassSize = 0; // 925 / 2;

	//if (m_pShareItem->GetMaintControlMgrPtr()->GetIOMapControlMgrPtr()->IsOutOfService() != true)
	{
		dPosX = pAlignInfo->m_sGlsAlign.m_fStageY;
		dPosR = pAlignInfo->m_sGlsAlign.m_fStageRotate;
		dPosX2 = pAlignInfo->m_sGlsAlign.m_fFrontY;
		dPosY2 = pAlignInfo->m_sGlsAlign.m_fFrontLeft;
		dPosY1 = pAlignInfo->m_sGlsAlign.m_fFrontRight;
		dPosZ = pAlignInfo->m_sGlsAlign.m_fFrontZ;
		dPosZR = pAlignInfo->m_sGlsAlign.m_fSubZRight;	// LYS 2026-09-05 Align 레시피 Sub Z RIGHT(Z3)
		dPosZL = pAlignInfo->m_sGlsAlign.m_fSubZLeft;	// LYS 2026-09-05 Align 레시피 Sub Z LEFT(Z4)
	}


	bool bRet = false;
	CMaintControlMgr* pImpl = nullptr;
	pImpl = m_pShareItem->GetMaintControlMgrPtr();

	if (pImpl != nullptr)
	{
		//if (nRet == df_MOTION_RV_SUCCESSED)
		//{
		//	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("[MoveAlignPos] Z2 origin check/move"));
		//	if (pImpl->GetCurPos(AXIS_Z2) > 0)
		//	{
		//		nRet = pImpl->MoveOrgPosA(AXIS_Z2, 0, true, SPEED_HIGH, -1, -1);
		//	}
		//}

		//if (nRet == VTM_RV_SUCCESSED)
		//{
		//	for (size_t i = df_PROBE_AGING1; i <= df_PROBE_MEAS; i++)
		//	{
		//		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsProbeUnContactSensorOn(i);
		//		if (bRet != true)
		//		{
		//			nRet = MOTION_INTERLOCK_6;
		//			break;
		//		}
		//	}
		//
		//	if (nRet == MOTION_INTERLOCK_6)
		//	{
		//		nRet = pImpl->MoveCmdPos_ProbeMove(AXIS_ALL_Z, 0, SPEED_HIGH);
		//
		//		if (nRet == VTM_RV_SUCCESSED)
		//		{
		//			for (size_t i = df_PROBE_AGING1; i <= df_PROBE_MEAS; i++)
		//			{
		//				bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsProbeUnContactSensorOn(i);
		//				if (bRet != true)
		//				{
		//					nRet = MOTION_INTERLOCK_6;
		//					break;
		//				}
		//			}
		//		}
		//	}
		//}
		nRet = pImpl->MoveCmdPos_ProbeMove(AXIS_ALL_Z, 0, SPEED_HIGH);

		if (nRet == VTM_RV_SUCCESSED)
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("[MoveAlignPos] interlock: loader-arm sensor"));
			CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_ALIGN_CHK_STEP1, true);

			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLoaderArmSensorOn();
			if (bRet == true)
			{
				nRet = PLC_ARM_SENSOR_DETECT;
			}

			CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_ALIGN_CHK_STEP1, false);
		}

		if (nRet == VTM_RV_SUCCESSED)
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("[MoveAlignPos] interlock: shutter-close sensor"));
			CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_ALIGN_CHK_STEP2, true);

			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsShutterSensorClose();
			if (bRet != true)
			{
				nRet = PLC_LOADER_SHUTTER_UP_ERROR;
			}

			CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_ALIGN_CHK_STEP2, false);
		}

		if (nRet == VTM_RV_SUCCESSED)
		{
			CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_ALIGN_CHK_STEP3, true);
		
			//for (size_t i = df_PROBE_AGING1; i <= df_PROBE_MEAS; i++)
			//{
			//	bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsProbeUnContactSensorOn(i);
			//	if (bRet != true)
			//	{
			//		nRet = MOTION_INTERLOCK_6;
			//		break;
			//	}
			//}
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsProbeUnContactSensorOn(df_PROBE_MEAS);
			if (bRet != true)
			{
				nRet = MOTION_INTERLOCK_6;
			}
		
			CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_ALIGN_CHK_STEP3, false);
		}
		
		if (nRet == df_MOTION_RV_SUCCESSED)
		{
			// Stage R -> Head Y -> Probe -> Head X -> Stage X -> Head Z
			if (nRet == df_MOTION_RV_SUCCESSED)
			{
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_ALIGN_MOVE_STEP1, true);

				m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("[MoveAlignPos] move Stage R (LOADER_R)"));
				nRet = pImpl->MoveCmdPosA(AXIS_LOADER_R, dPosR, false, SPEED_HIGH, 0, 0);	
			}

			if (nRet == df_MOTION_RV_SUCCESSED)
			{
				m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("[MoveAlignPos] move Head Y (Y1/Y2)"));
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_ALIGN_MOVE_STEP3, true);
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_ALIGN_MOVE_STEP4, true);

				stMotorPosition stPosition[2];
				ZeroMemory(&stPosition, sizeof(stPosition));
				stPosition[0].axis = AXIS_Y1;
				stPosition[0].dPosition = dPosY2;
				stPosition[1].axis = AXIS_Y2;
				stPosition[1].dPosition = dPosY1;

				nRet = pImpl->MoveCmdPosMulti(stPosition, 2, SPEED_HIGH);
			}
			// Probe
			if (nRet == df_MOTION_RV_SUCCESSED)
			{
				double dCurProbe = 0;
			
			//	if (fabs(pImpl->GetCurPos(AXIS_PROBE_X9) - dPosX2) > 1)
				//if (pImpl->GetCurPos(AXIS_PROBE_X9) > dPosX2 - 50 )
				//{
				//	/*!
				//	* Probe 이동 개선
				//	* 이동 전 간격 여유가 있으면 이동 안함
				//	* stMotorPosition stPositionProbeX[5];
				//	* pImpl->GetCmdPos_ProbeX(stPositionProbeX, df_PROBE_MEAS, dPosX2 - 100, 200, TRUE);
				//	*
				//	* nRet = pImpl->MoveCmdPosMulti(stPositionProbeX, 5, SPEED_HIGH, true);
				//	*/
				//	double dProbeGap = 0;
				//	pImpl->GetInterlockPos(nullptr, nullptr, &dProbeGap);
				//
				//	double dPosProbe4 = 0;
				//	dPosProbe4 = pImpl->GetCurPos(AXIS_PROBE_X7);
				//	if ( (dPosX2 - 50) - dProbeGap < dPosProbe4)
				//	{
				//		// Probe X
				//		stMotorPosition stPositionProbeX[5];
				//		ZeroMemory(&stPositionProbeX, sizeof(stPositionProbeX));
				//		pImpl->GetCmdPos_ProbeX(stPositionProbeX, df_PROBE_MEAS, dPosX2 - 100, 200, TRUE);
				//
				//		nRet = pImpl->MoveCmdPosMulti(stPositionProbeX, 5, SPEED_HIGH, true);
				//	}
				//	else
				//	{
				//		nRet = pImpl->MoveCmdPosA(AXIS_PROBE_X9, dPosX2 - 50, true, SPEED_HIGH, 0, 0);
				//	}
				//}
			}

			if (nRet == df_MOTION_RV_SUCCESSED)
			{
				m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("[MoveAlignPos] move Head X (X2/X3)"));
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_ALIGN_MOVE_STEP5, true);

				double dGantryGap = 0;
				pImpl->GetInterlockPos(&dGantryGap, nullptr, nullptr);

				double dCurX3 = 0;
				dCurX3 = pImpl->GetCurPos(AXIS_X3);

				if ((dPosX2 - dGantryGap) < dCurX3)
				{
					stMotorPosition stPosition[2];
					ZeroMemory(&stPosition, sizeof(stPosition));

					stPosition[0].axis = AXIS_X2;
					stPosition[0].dPosition = dPosX2;
					stPosition[1].axis = AXIS_X3;
					stPosition[1].dPosition = pImpl->GetMinSWLimit(AXIS_X2) - (dGantryGap + 50);  //dPosX2 - dGantryGap; // jsyoon 2022.06.07 change
					nRet = pImpl->MoveCmdPosMulti(stPosition, 2, SPEED_HIGH);
				}
				else
				{
					nRet = pImpl->MoveCmdPosA(AXIS_X2, dPosX2, false, SPEED_HIGH, -1, -1);
				}
			}

			if (nRet == df_MOTION_RV_SUCCESSED)
			{
				m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("[MoveAlignPos] wait MotionDone (gantry X2/X3/Y1/Y2/R)"));
				for (int iAxis = AXIS_X; iAxis < AXIS_TYPE_END; iAxis++)
				{
					/*if (iAxis == AXIS_X)
					{
						nRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_MotionDone(iAxis);

						CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_ALIGN_MOVE_STEP2, false);
					}*/

					if (iAxis == AXIS_X2)
					{
						nRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_MotionDone(iAxis);
					
						CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_ALIGN_MOVE_STEP5, false);
					}
					else if (iAxis == AXIS_X3)
					{
						nRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_MotionDone(iAxis);
					}
					else if (iAxis == AXIS_Y1)
					{
						nRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_MotionDone(iAxis);

						CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_ALIGN_MOVE_STEP3, false);
					}
					else if (iAxis == AXIS_Y2)
					{
						nRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_MotionDone(iAxis);

						CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_ALIGN_MOVE_STEP4, false);
					}

					else if (iAxis == AXIS_LOADER_R)	// Stage R
					{
						nRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_MotionDone(iAxis);

						CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_ALIGN_MOVE_STEP1, false);
					}

					if (nRet != df_MOTION_RV_SUCCESSED)
					{
						break;
					}
				}
			}

			// Stage X
			if (nRet == df_MOTION_RV_SUCCESSED)
			{
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_ALIGN_MOVE_STEP2, true);

				m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("[MoveAlignPos] move Stage X"));
				nRet = pImpl->MoveCmdPosA(AXIS_X, dPosX, false, SPEED_HIGH);
			}

			// Head Z
			if (nRet == df_MOTION_RV_SUCCESSED)
			{
				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_ALIGN_MOVE_STEP6, true);

				m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("[MoveAlignPos] move Head Z (SUB Z Z3/Z4, blocking)"));
				// LYS 2026-09-16 Sub Z RIGHT/LEFT(Z3/Z4) 동시 하강 - Probe Align(09-16)과 같은 방식: 두 축을 대기 없이 연속 지령한 뒤 완료를 함께 기다린다(구 코드는 Z3 완료 후 Z4 시작)
				//nRet = pImpl->MoveCmdPosA(AXIS_Z3, dPosZR, true, SPEED_HIGH);	// LYS 2026-09-05 Head Z: Z1 대신 Sub Z RIGHT(Z3) 이동
				//if (nRet == df_MOTION_RV_SUCCESSED) nRet = pImpl->MoveCmdPosA(AXIS_Z4, dPosZL, true, SPEED_HIGH);	// LYS 2026-09-05 Z3 성공 시 Sub Z LEFT(Z4) 이동
				{
					long nRetZ3 = pImpl->MoveCmdPosA(AXIS_Z3, dPosZR, false, SPEED_HIGH);	// Sub Z RIGHT(Z3) 지령만
					long nRetZ4 = pImpl->MoveCmdPosA(AXIS_Z4, dPosZL, false, SPEED_HIGH);	// Sub Z LEFT(Z4) 지령만 - Z3 와 동시 하강
					if (nRetZ3 == df_MOTION_RV_SUCCESSED) nRetZ3 = pImpl->DoWork_MotionDone(AXIS_Z3);	// 지령이 나간 축은 실패와 무관하게 정지까지 기다린다
					if (nRetZ4 == df_MOTION_RV_SUCCESSED) nRetZ4 = pImpl->DoWork_MotionDone(AXIS_Z4);
					nRet = (nRetZ3 != df_MOTION_RV_SUCCESSED) ? nRetZ3 : nRetZ4;
				}

				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_ALIGN_MOVE_STEP6, false);
			}

			// Stage X
			if (nRet == df_MOTION_RV_SUCCESSED)
			{
				m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("[MoveAlignPos] wait Stage X MotionDone"));
				nRet = m_pShareItem->GetMaintControlMgrPtr()->DoWork_MotionDone(AXIS_X);

				CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_ALIGN_MOVE_STEP2, false);
			}
		}
	}
	
	CString strMapExit; strMapExit.Format(_T("[MoveAlignPos] EXIT nRet=%d"), nRet); m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strMapExit);
	return nRet;
}

int CDoWorkGlassSWAlignStep::AlignAF(int nHead)
{
//	return CSequenceManager::GetInstance()->GetFunctionModuleAFControl()->StartAF(nHead);
	return 1;
}

int CDoWorkGlassSWAlignStep::CenteringAlignPattern(int nHead)
{
	//CSequenceManager::GetInstance()->GetFunctionModuleCentering()->SetAlignType(0);
	//CSequenceManager::GetInstance()->GetFunctionModuleCentering()->SetHead(nHead);
	//CSequenceManager::GetInstance()->GetFunctionModuleCentering()->Start();

	//return CSequenceManager::GetInstance()->GetFunctionModuleCentering()->Is_Complete();

	return 1;
}

int CDoWorkGlassSWAlignStep::AlignGlassMoveShift(double* pdMovOrg, int nFailCnt, double dTheta)
{
	if (nFailCnt >= 25)
		return JOB_PROC_ALIGN_POS_ERROR;

	tagAlignPos arrAlignPos[25] =
	{
		//	X,		Y,		,Theta
		0.,		0.,		0.,
		0.5,	0.,		0.,		// X
		-0.5,	0.,		0.,
		0.,		0.5,	0.,		// Y
		0.,		-0.5,	0.,
		0.5,	0.5,	0.,		// XY
		0.5,	-0.5,	0.,
		-0.5,	0.5,	0.,
		-0.5,	-0.5,	0.,

		0.8,	0.,		0.,		// X2
		-0.8,	0.,		0.,
		0.,		0.8,	0.,		// X2
		0.,		-0.8,	0.,
		0.8,	0.8,	0.,		// XY2
		0.8,	-0.5,	0.,
		-0.8,	0.8,	0.,
		-0.8,	-0.8,	0.,

		1.2,	0.,		0.,		// X2
		-1.2,	0.,		0.,
		0.,		1.2,	0.,		// X2
		0.,	   -1.2,	0.,
		1.2,	1.2,	0.,		// XY2
		1.2,   -1.2,	0.,
		-1.2,	1.2,	0.,
		-1.2,  -1.2,	0.,
	};

	int nRtn = VTM_RV_SUCCESSED;
	CString strLog, strState;
	//CPerfTimer pTimer;
	//pTimer.Start(TRUE);

	CMaintControlMgr* pImpl = nullptr;
	pImpl = m_pShareItem->GetMaintControlMgrPtr();

	if (pImpl != nullptr)
	{
		double dX = 0, dY1 = 0, dY2 = 0, dR = 0;
		dX  = pdMovOrg[0] + arrAlignPos[nFailCnt].dX;
		dY1 = pdMovOrg[1] + arrAlignPos[nFailCnt].dY;
		dY2 = pdMovOrg[2] + arrAlignPos[nFailCnt].dY;
		dR  = pdMovOrg[3] + dTheta;

		pImpl->MoveCmdPosA(AXIS_LOADER_R, dR, false, SPEED_MID);
		pImpl->MoveCmdPosA(AXIS_X, dX, false, SPEED_MID);
		pImpl->MoveCmdPosA(AXIS_Y1, dY1, false, SPEED_MID);
		pImpl->MoveCmdPosA(AXIS_Y2, dY2, true, SPEED_MID);

		nRtn = pImpl->DoWork_MotionDone(AXIS_LOADER_R);
		if (nRtn == df_MOTION_RV_SUCCESSED)
		{
			nRtn = pImpl->DoWork_MotionDone(AXIS_X);
		}

		if (nRtn == df_MOTION_RV_SUCCESSED)
		{
			nRtn = pImpl->DoWork_MotionDone(AXIS_Y1);
		}

		if (nRtn == df_MOTION_RV_SUCCESSED)
		{
			nRtn = pImpl->DoWork_MotionDone(AXIS_Y2);
		}

		if (nRtn != df_MOTION_RV_SUCCESSED)
		{
			nRtn = JOB_PROC_ALIGN_POS_ERROR;
		}
	}

//	pTimer.Stop();
//	strLog.Format(_T("T:%6.2f Sec, %s"), pTimer.Elapsed(), strState);
//	gLog(m_pSystem->hLog, strLog, LOG_LEVEL_TT);

	return nRtn;
}

// Glass X, Y Align
int CDoWorkGlassSWAlignStep::AlignGlassXYZero(double* pdMovX, double* pdMovY)
{
	int nRet = df_MOTION_RV_SUCCESSED;
	float Score = 1.0;
	float MaxSorce = -1;
	int iPtrn[2] = { -1, -1 };
	CPtrnInfo ptrnInfo[df_MAX_PTRNINFO];
	for (size_t i = 0; i < df_MAX_PTRNINFO; i++)
	{
		ptrnInfo[i].ClearInfo();
	}

	S_Align* pAlignInfo = nullptr;
	pAlignInfo = &m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeMgrPtr()->m_sAlign;

//#ifdef _USE_TEST_ALIGN
//	ptrnInfo[0].m_strName.Format(_T("Test_Mark.png"));
//#endif // _USE_TEST_ALIGN

	CPtrnInfo* pPtrnInfo = NULL;
	double dPixelx[2] = { 0, }, dPixely[2] = { 0, };

	CMaintControlMgr* pImpl = nullptr;
	pImpl = m_pShareItem->GetMaintControlMgrPtr();

	// LYS 2026-09-05 센터링 시작 위치 캡처 - Align 오프셋은 절대 기계좌표가 아니라 잔차(최종-시작)여야 함
	double dStartX = 0., dStartY = 0.;
	if (pImpl != nullptr)
	{
		dStartX = pImpl->GetOrgPos(AXIS_X);
		dStartY = pImpl->GetOrgPos(AXIS_Y1);
	}

	int iReTry = 0, nRetrylimit = 3;
	double dAlignMarginX = 1, dAlignMarginY = 1;
	bool bAlignDone = false;
	bool bCentered = false;	// LYS 2026-09-05 마진 통과 여부 - 루프 내 DoWork_MotionDone이 nRet의 CENTERING_FAIL을 덮어쓰는 문제 보완

	dAlignMarginX = pAlignInfo->m_sMargin.m_sGlassMargin.m_fX;
	dAlignMarginY = pAlignInfo->m_sMargin.m_sGlassMargin.m_fY;
	nRetrylimit = pAlignInfo->m_sMargin.m_sGlassMargin.m_nCentering;

	// 반복
	do
	{
		// 이미지 패턴 매칭
		for (size_t j = 0; j < 2; j++)	// 2 Head
		{
			MaxSorce = -1;

			if (j == 0)
			{
				for (size_t k = 0; k < df_MAX_PTRNINFO; k++)
				{
					if (k < 5)
					{
						ptrnInfo[k].m_nThreshold = pAlignInfo->m_sCcdSet.m_sFrontLeft.m_nMatchingScore;
						ptrnInfo[k].m_strName.Format(_T("%s"), pAlignInfo->m_chPtnFrontLeft[k]);
					}

				}
			}
			else
			{
				for (size_t k = 0; k < df_MAX_PTRNINFO; k++)
				{
					if (k < 5)
					{
						ptrnInfo[k].m_nThreshold = pAlignInfo->m_sCcdSet.m_sFrontRight.m_nMatchingScore;
						ptrnInfo[k].m_strName.Format(_T("%s"), pAlignInfo->m_chPtnFrontRight[k]);
					}

				}
			}

			// 이미지 패턴 매칭
#if 0	// LYS 2026-09-25 구 코드: 첫 성공 패턴에서 break -> 최고 점수 선택이 아니었다. 아래 PtrnMatchBest 로 대체.
			for (size_t k = 0; k < df_MAX_PTRNINFO; k++)	// Pattern
			{
				pPtrnInfo = &ptrnInfo[k];
				nRet = m_pShareItem->GetAlignCameraMgr(j)->PtrnMatch(pPtrnInfo, &Score, LENS_USE_ALIGN);
				if (nRet == VTM_RV_SUCCESSED)
				{
					if (Score > MaxSorce)
					{
						MaxSorce = Score;
						iPtrn[j] = k;
						dPixelx[j] = pPtrnInfo->m_dRealX;	// 차이값(mm)
						dPixely[j] = pPtrnInfo->m_dRealY;	// 차이값(mm)
					}

					break;
				}
			}
#else
			// LYS 2026-09-25 등록 패턴 전부 매칭해 점수가 가장 높은 패턴 채택 (스냅 1회, 패턴별 점수는 [AlignCCDn] PtrnMatchBest 로그).
			{
				int nBestPtrn = -1;
				nRet = m_pShareItem->GetAlignCameraMgr(j)->PtrnMatchBest(ptrnInfo, df_MAX_PTRNINFO, &Score, &nBestPtrn, LENS_USE_ALIGN);
				if (nRet == VTM_RV_SUCCESSED && nBestPtrn >= 0)
				{
					MaxSorce = Score;
					iPtrn[j] = nBestPtrn;
					dPixelx[j] = ptrnInfo[nBestPtrn].m_dRealX;	// 차이값(mm)
					dPixely[j] = ptrnInfo[nBestPtrn].m_dRealY;	// 차이값(mm)
				}
			}
#endif
		}

		bAlignDone = true;

		for (size_t j = 0; j < 2; j++)
		{
		//	if (iPtrn[j] < -1)
			if (iPtrn[j] < 0)	// 2023.03.09 change
			{
				bAlignDone = false;
				break;
			}
		}

		if (bAlignDone == false)
		{
			nRet = VISION_ALIGN_DETECTED_FAIL;
			break;
		}


		// 센터링
		// LYS 2026-09-12 CCD 얼라인 카메라 방향이 바뀌어 XYR 보상 부호를 전부 반전. 2026-09-12 16:35 Glass XYZero 실측이 -0.0794→-0.1588→-0.3175 / 0.1436→0.2835→0.5708 로 정확히 2배씩 커지는 1:1 발산(=부호 반대)이었다. 아래 2026-09-05 주석은 방향 변경 전 근거라 참고용으로 남긴다.
		int iDirX = 1, iDirY = 1;	// LYS 2026-09-12 카메라 방향 변경으로 -1,-1 -> 1,1 반전 
		// LYS 2026-09-05 부호 반전 - 2026-09-05 10:35 실측에서 +방향 이동이 오프셋을 1:1로 키움(발산). Probe(iDirX=-1)와 동일 방식
		double dDistanceX = 0., dDistanceY = 0;
		double dX = 0, dY = 0;
		// 
		// Angle 계산
		COORDINATE stBase[2];		// Align Mark 좌표 위치
		COORDINATE stDetect[2];

		// Glass
	//	dX = ((dPixelx[0] + dPixelx[1]) / 2) * iDirX;		// Glass Y
	//	dY = ((dPixely[0] + dPixely[1]) / 2) * iDirY;		// Glass X
		
		//_TEST
		dX = dPixelx[0] * iDirX;		// Glass Y
		dY = dPixely[0] * iDirY;		// Glass X
		////


		/*
		// Save Image
		CString strPath, strSaveImage;
		CString strCellID;
		strCellID = CSystemParamShare::GetInstance()->GetCellID();
		strPath.Format(_T("%s%s%s\\%s\\"), m_pShareItem->GetSystemConfigMgrPtr()->GetLogFilePath(), _T("ALIGN\\"), m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeID(), strCellID);
		MakeDir(strPath);

		strSaveImage.Format(_T("%s%s_%s.png"), strPath, _T("AlignGlassThetaZeroSrc1"), GetCurTimeStr());
		imaqWritePNGFile2(pSnapImageCCD1, CStrToChar(strSaveImage), 750, NULL, FALSE);

		strSaveImage.Format(_T("%s%s_%s.png"), strPath, _T("AlignGlassThetaZeroSrc2"), GetCurTimeStr());
		imaqWritePNGFile2(pSnapImageCCD2, CStrToChar(strSaveImage), 750, NULL, FALSE);
		*/

		// Margin 
		if (nRet == VTM_RV_SUCCESSED)
		{
			//if (m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
			//{
			//	break;
			//}

			CString str;
			str.Format(_T("AlignGlassXYZero %d/%d, %.4f, %.4f < %.4f, %.4f"), iReTry+1, nRetrylimit,  dY, dX, dAlignMarginY, dAlignMarginX);
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, str);

			if (fabs(dX) < dAlignMarginX && fabs(dY) < dAlignMarginY)
			{
				bCentered = true;	// LYS 2026-09-05 마진 통과
				break;
			}
			else
			{
				nRet = VISION_ALIGN_CENTERING_FAIL;

		//		*pdMovX += dX;
		//		*pdMovY += dY;

				// Move
				if (pImpl != nullptr)
				{
					pImpl->MoveCmdPosR(AXIS_X, dY, false, SPEED_MID);
					pImpl->MoveCmdPosR(AXIS_Y1, dX, false, SPEED_MID);
					pImpl->MoveCmdPosR(AXIS_Y2, dX, false, SPEED_MID);

					nRet = pImpl->DoWork_MotionDone(AXIS_X);
					if (nRet != df_MOTION_RV_SUCCESSED)
					{
						break;
					}

					nRet = pImpl->DoWork_MotionDone(AXIS_Y1);
					if (nRet != df_MOTION_RV_SUCCESSED)
					{
						break;
					}

					nRet = pImpl->DoWork_MotionDone(AXIS_Y2);
					if (nRet != df_MOTION_RV_SUCCESSED)
					{
						break;
					}
				}
			}
		}

		iReTry++;


	} while (iReTry < nRetrylimit);

	// LYS 2026-09-05 재시도 한도 내에 마진을 못 맞추면 실패로 반환 (구 코드는 마지막 MotionDone 성공이 실패 상태를 지워 발산한 오프셋이 그대로 적용됨)
	if (nRet == VTM_RV_SUCCESSED && bCentered == false)
	{
		nRet = VISION_ALIGN_CENTERING_FAIL;
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(WARN, _T("AlignGlassXYZero centering NOT converged within retry limit - align FAIL"));
	}

	if (nRet == VTM_RV_SUCCESSED)
	{
		// LYS 2026-09-05 잔차(센터링 이동량)로 반환. 구 코드는 X/Y1 절대위치를 반환했음(m_fBase_FrontRight는 어디서도 set/load 안 되는 죽은 필드=0)
		//   -> Y1=556.4가 m_dAlignOffset[AXIS_Y1..Y4]에 실려 모든 셀 목표가 +556mm 밀림(SW 리밋 초과)
		*pdMovX = pImpl->GetOrgPos(AXIS_X) - dStartX;
		*pdMovY = pImpl->GetOrgPos(AXIS_Y1) - dStartY;
		CString strRes;
		strRes.Format(_T("AlignGlassXYZero residual X:%.4f (%.3f->%.3f)  Y1:%.4f (%.3f->%.3f)"),
			*pdMovX, dStartX, pImpl->GetOrgPos(AXIS_X), *pdMovY, dStartY, pImpl->GetOrgPos(AXIS_Y1));
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strRes);
	}

	return nRet;

	bool bRet = false;

	//CAlignCrossLine stAlingCrossLine;
	//// Image Grab
	//int nGrabSizeW = 0, nGrabSizeH = 0;
	//nGrabSizeW = m_pShareItem->GetAlignCameraMgr(2)->GetImageGrabSizeX();
	//nGrabSizeH = m_pShareItem->GetAlignCameraMgr(2)->GetImageGrabSizeY();
	//CString strLog;

	//size_t nPixel = nGrabSizeW * nGrabSizeH * 4;
	//BYTE *ucImg = new BYTE[nPixel];
	//memset(ucImg, 0, sizeof(BYTE)*nPixel);

	//Image* pSnapImageCCD1 = imaqCreateImage(IMAQ_IMAGE_RGB, 7);
	//Image* pSnapImageCCD1U8 = imaqCreateImage(IMAQ_IMAGE_U8, 7);
	//Image* pSnapImageCCD2 = imaqCreateImage(IMAQ_IMAGE_RGB, 7);
	//Image* pSnapImageCCD2U8 = imaqCreateImage(IMAQ_IMAGE_U8, 7);

	//CString strVal;
	//stThreshold stThreashol1, stThreashol2;
	//double dAngle = 0, dX = 0, dY = 0;
	//double dCamResoutionUm = 3.75, dCamResoution = dCamResoutionUm / 1000;
	//double dCamDistance = 0;

	//CMotorControlMgr *pImpl = nullptr;
	//pImpl = m_pShareItem->GetMotorControlMgrPtr();

	//int nExposureTime = 0;
	//int nRet = df_MOTION_RV_SUCCESSED;
	//int iReTry = 0, nRetrylimit = stRecipeUTG.nAlignRetry;

	//double dShiftX = 0, dCurX = 0, dNewX = 0, dShiftY = 0, dShiftR = 0;
	//dCurX = pImpl->GetCurPos(AXIS_X, 0, 0);

	//bool bAlignDone = false;

	//if (m_pShareItem->GetAlignCameraMgr(1)->IsOutOfService() != true && m_pShareItem->GetAlignCameraMgr(2)->IsOutOfService() != true)
	//{
	//	do
	//	{
	//		// Image Grab
	//		memset(ucImg, 0, sizeof(BYTE)*nPixel);
	//		if (m_pShareItem->GetAlignCameraMgr(1)->GetSnapImage(nExposureTime, ucImg) == true)
	//		{
	//			imaqArrayToImage(pSnapImageCCD1, (BYTE*)ucImg, nGrabSizeW, nGrabSizeH);
	//		}

	//		memset(ucImg, 0, sizeof(BYTE)*nPixel);
	//		if (m_pShareItem->GetAlignCameraMgr(2)->GetSnapImage(nExposureTime, ucImg) == true)
	//		{
	//			imaqArrayToImage(pSnapImageCCD2, (BYTE*)ucImg, nGrabSizeW, nGrabSizeH);
	//		}

	//		dCamDistance = stRecipeUTG.dAlignCamDistance;
	//		stThreashol1.nGlassThreshold_ = stRecipeUTG.stAlignParam[0].nGlassThreshold;
	//		stThreashol1.nCanyEdgeThreshold1_ = stRecipeUTG.stAlignParam[0].nCanyEdgeThreshold1;
	//		stThreashol1.nCanyEdgeThreshold2_ = stRecipeUTG.stAlignParam[0].nCanyEdgeThreshold2;

	//		stThreashol2.nGlassThreshold_ = stRecipeUTG.stAlignParam[2].nGlassThreshold;
	//		stThreashol2.nCanyEdgeThreshold1_ = stRecipeUTG.stAlignParam[2].nCanyEdgeThreshold1;
	//		stThreashol2.nCanyEdgeThreshold2_ = stRecipeUTG.stAlignParam[2].nCanyEdgeThreshold2;

	//		// Calc Shift
	//		stAlingCrossLine.SetParameter(dCamResoution, nGrabSizeW, nGrabSizeH);

	//		imaqCast(pSnapImageCCD1U8, pSnapImageCCD1, IMAQ_IMAGE_U8, NULL, 0);
	//		imaqCast(pSnapImageCCD2U8, pSnapImageCCD2, IMAQ_IMAGE_U8, NULL, 0);
	//		nRet = stAlingCrossLine.GetOffsetXYAlignValue(pSnapImageCCD1U8, pSnapImageCCD2U8, stThreashol1, stThreashol2, dX, dY);

	//		// Save Image
	//		CString strPath, strSaveImage;
	//		CString strCellID;
	//		strCellID = CSystemParamShare::GetInstance()->GetCellID();
	//		strPath.Format(_T("%s%s%s\\%s\\"), m_pShareItem->GetSystemConfigMgrPtr()->GetLogFilePath(), _T("ALIGN\\"), m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeID(), strCellID);
	//		MakeDir(strPath);

	//		strSaveImage.Format(_T("%s%s_%s.png"), strPath, _T("AlignGlassXYZeroSrc1"), GetCurTimeStr());
	//		imaqWritePNGFile2(pSnapImageCCD1, CStrToChar(strSaveImage), 750, NULL, FALSE);

	//		strSaveImage.Format(_T("%s%s_%s.png"), strPath, _T("AlignGlassXYZeroSrc2"), GetCurTimeStr());
	//		imaqWritePNGFile2(pSnapImageCCD2, CStrToChar(strSaveImage), 750, NULL, FALSE);

	//		if (nRet == 0)
	//		{
	//			if (fabs(dX) < stRecipeUTG.dAlignMarginX && fabs(dY) < stRecipeUTG.dAlignMarginY)
	//			{
	//				bAlignDone = true;
	//				break;
	//			}
	//		}

	//		// Move
	//		if (pImpl != nullptr)
	//		{
	//			pImpl->MoveCmdPosR(AXIS_Y, dX, false, SPEED_MID);
	//			pImpl->MoveCmdPosR(AXIS_X, dY, false, SPEED_MID);
	//			nRet = pImpl->DoWork_MotionDone(AXIS_ALL);
	//			if (nRet != df_MOTION_RV_SUCCESSED)
	//			{
	//				break;
	//			}
	//		}

	//		iReTry++;

	//	} while (iReTry < nRetrylimit);
	//}
	//else
	//{
	//	bAlignDone = true; 
	//}
	//delete[] ucImg; // jsyoon 20210928
	//imaqDispose(pSnapImageCCD1);
	//imaqDispose(pSnapImageCCD1U8);
	//imaqDispose(pSnapImageCCD2);
	//imaqDispose(pSnapImageCCD2U8);

	//if (bAlignDone == true)
	//{
	//	bRet = true;
	//}

	return bRet;
}

int CDoWorkGlassSWAlignStep::AlignGlassSearch()
{
	int nRet = df_MOTION_RV_SUCCESSED;
	float Score = 1.0;
	float MaxSorce = -1;
	int iPtrn[2] = { -1, -1 };
	CPtrnInfo ptrnInfo[df_MAX_PTRNINFO];
	for (size_t i = 0; i < df_MAX_PTRNINFO; i++)
	{
		ptrnInfo[i].ClearInfo();
	}

	S_Align* pAlignInfo = nullptr;
	pAlignInfo = &m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeMgrPtr()->m_sAlign;

//#ifdef _USE_TEST_ALIGN
//	ptrnInfo[0].m_strName.Format(_T("Test_Mark.png"));
//#endif // _USE_TEST_ALIGN

	CPtrnInfo* pPtrnInfo = NULL;
	double dPixelx[2] = { 0, }, dPixely[2] = { 0, };

	int iReTry = 0, nRetrylimit = 3;
	double dAlignMargin = 1;
	bool bAlignDone = false;

	Rect rectROI;
	ZeroMemory(&rectROI, sizeof(Rect));
	for (size_t i = 0; i < 4; i++) 
	{
		m_pShareItem->GetAlignCameraMgr(i)->SetShowROI(false, rectROI);  
	}

	// 이미지 패턴 매칭
	for (size_t j = 0; j < 2; j++)	// 2 Head
	{
		MaxSorce = -1;

		if (j == 0)
		{
			for (size_t k = 0; k < df_MAX_PTRNINFO; k++)
			{
				if (k < 5)
				{
					ptrnInfo[k].m_nThreshold = pAlignInfo->m_sCcdSet.m_sFrontLeft.m_nMatchingScore;
					ptrnInfo[k].m_strName.Format(_T("%s"), pAlignInfo->m_chPtnFrontLeft[k]);
				}
			}
		}
		else
		{
			for (size_t k = 0; k < df_MAX_PTRNINFO; k++)
			{
				if (k < 5)
				{
					ptrnInfo[k].m_nThreshold = pAlignInfo->m_sCcdSet.m_sFrontRight.m_nMatchingScore;
					ptrnInfo[k].m_strName.Format(_T("%s"), pAlignInfo->m_chPtnFrontRight[k]);
				}
			}
		}

		// 이미지 패턴 매칭
#if 0	// LYS 2026-09-25 구 코드: 첫 성공 패턴에서 break -> 최고 점수 선택이 아니었다. 아래 PtrnMatchBest 로 대체.
		for (size_t k = 0; k < df_MAX_PTRNINFO; k++)	// Pattern
		{
			pPtrnInfo = &ptrnInfo[k];
			nRet = m_pShareItem->GetAlignCameraMgr(j)->PtrnMatch(pPtrnInfo, &Score, LENS_USE_ALIGN);
			if (nRet == VTM_RV_SUCCESSED)
			{
				if (Score > MaxSorce)
				{
					MaxSorce = Score;
					iPtrn[j] = k;
					dPixelx[j] = pPtrnInfo->m_dRealX;	// 차이값(mm)
					dPixely[j] = pPtrnInfo->m_dRealY;	// 차이값(mm)
				}
				break;
			}
		}
#else
		// LYS 2026-09-25 등록 패턴 전부 매칭해 점수가 가장 높은 패턴 채택 (스냅 1회, 패턴별 점수는 [AlignCCDn] PtrnMatchBest 로그).
		{
			int nBestPtrn = -1;
			nRet = m_pShareItem->GetAlignCameraMgr(j)->PtrnMatchBest(ptrnInfo, df_MAX_PTRNINFO, &Score, &nBestPtrn, LENS_USE_ALIGN);
			if (nRet == VTM_RV_SUCCESSED && nBestPtrn >= 0)
			{
				MaxSorce = Score;
				iPtrn[j] = nBestPtrn;
				dPixelx[j] = ptrnInfo[nBestPtrn].m_dRealX;	// 차이값(mm)
				dPixely[j] = ptrnInfo[nBestPtrn].m_dRealY;	// 차이값(mm)
			}
		}
#endif
	}

	bAlignDone = true;

	

	for (size_t j = 0; j < 2; j++)
	{
	//	if (iPtrn[j] < -1)
		if (iPtrn[j] < 0)	// 2023.03.09 change
		{
			bAlignDone = false;
			break;
		}
	}

	// Shift Move
	if (bAlignDone == false)
	{
		CMaintControlMgr* pImpl = nullptr;
		pImpl = m_pShareItem->GetMaintControlMgrPtr();

		if (pImpl->IsOutOfServiceMC() != true)
		{
			double dCurPos[4] = { 0, };
			dCurPos[0] = pImpl->GetCurPos(AXIS_X2);
			dCurPos[1] = pImpl->GetCurPos(AXIS_Y1);
			dCurPos[2] = pImpl->GetCurPos(AXIS_Y2);
			dCurPos[3] = pImpl->GetCurPos(AXIS_LOADER_R);

			double dTheta[10] = { 0.01, -0.01, 0.02, -0.02, 0.03, -0.03, 0.04, -0.04, 0.05, -0.05 };

			bool bUserStop = false;

			// Retry Move Stage X,Y
			for (int iloop = 0; iloop < 3; iloop++)
			{
				// Retry Move Stage R
				for (int iTheta = 0; iTheta < 6; iTheta++)
				{
					if (m_pShareItem->IfPauseEvt_WaitResumeEvt() != true)	// 2023.04.06 add
					{
						bUserStop = true;
						break;
					}

					nRet = AlignGlassMoveShift(dCurPos, iloop, dTheta[iTheta]);
					if (nRet == VTM_RV_SUCCESSED)
					{
						iPtrn[0] = -1;
						iPtrn[1] = -1;

						// 이미지 패턴 매칭
						for (size_t j = 0; j < 2; j++)	// 2 Head
						{
							// 이미지 패턴 매칭
#if 0	// LYS 2026-09-25 구 코드: 첫 성공 패턴에서 break -> 최고 점수 선택이 아니었다. 아래 PtrnMatchBest 로 대체.
							for (size_t k = 0; k < df_MAX_PTRNINFO; k++)	// Pattern
							{
								pPtrnInfo = &ptrnInfo[k];
								nRet = m_pShareItem->GetAlignCameraMgr(j)->PtrnMatch(pPtrnInfo, &Score, LENS_USE_ALIGN);
								if (nRet == VTM_RV_SUCCESSED)
								{
									if (Score > MaxSorce)
									{
										MaxSorce = Score;
										iPtrn[j] = k;
										dPixelx[j] = pPtrnInfo->m_dRealX;	// 차이값(mm)
										dPixely[j] = pPtrnInfo->m_dRealY;	// 차이값(mm)
									}
									break;
								}
							}
#else
							// LYS 2026-09-25 등록 패턴 전부 매칭해 점수가 가장 높은 패턴 채택 (스냅 1회, 패턴별 점수는 [AlignCCDn] PtrnMatchBest 로그).
							{
								int nBestPtrn = -1;
								nRet = m_pShareItem->GetAlignCameraMgr(j)->PtrnMatchBest(ptrnInfo, df_MAX_PTRNINFO, &Score, &nBestPtrn, LENS_USE_ALIGN);
								if (nRet == VTM_RV_SUCCESSED && nBestPtrn >= 0)
								{
									MaxSorce = Score;
									iPtrn[j] = nBestPtrn;
									dPixelx[j] = ptrnInfo[nBestPtrn].m_dRealX;	// 차이값(mm)
									dPixely[j] = ptrnInfo[nBestPtrn].m_dRealY;	// 차이값(mm)
								}
							}
#endif
						}

						bAlignDone = true;

						for (size_t j = 0; j < 2; j++)
						{
							//	if (iPtrn[j] < -1)
							if (iPtrn[j] < 0)	// 2023.03.09 change
							{
								bAlignDone = false;
								break;
							}
						}
					}

					

					if (bAlignDone == true)
						break;
				}

				if (bUserStop == true)	// 2023.04.06 add
				{
					break;
				}

				if (bAlignDone == true)
					break;
			}
		}
		else
		{
			bAlignDone = true;
		}
	}

	if (bAlignDone == true)
	{
		nRet = VTM_RV_SUCCESSED;
	}
	else
	{
		nRet = VISION_ALIGN_DETECTED_FAIL;
	}

	return nRet;
}

int CDoWorkGlassSWAlignStep::AlignGlassTZero(double* pdMovR)
{
	int nRet = df_MOTION_RV_SUCCESSED;
	float Score = 1.0;
	float MaxSorce = -1;
	int iPtrn[2] = { -1, -1 };
	CPtrnInfo ptrnInfo[df_MAX_PTRNINFO];
	for (size_t i = 0; i < df_MAX_PTRNINFO; i++)
	{
		ptrnInfo[i].ClearInfo();
	}

	S_Align* pAlignInfo = nullptr;
	pAlignInfo = &m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeMgrPtr()->m_sAlign;

//#ifdef _USE_TEST_ALIGN
//	ptrnInfo[0].m_strName.Format(_T("Test_Mark.png"));
//#endif // _USE_TEST_ALIGN
	
	CPtrnInfo* pPtrnInfo = NULL;
	double dPixelx[2] = { 0, }, dPixely[2] = { 0, };

	CMaintControlMgr* pImpl = nullptr;
	pImpl = m_pShareItem->GetMaintControlMgrPtr();
	double dCurPos[4] = { 0, };

	dCurPos[0] = pImpl->GetCurPos(AXIS_X2);
	dCurPos[1] = pImpl->GetCurPos(AXIS_Y1);
	dCurPos[2] = pImpl->GetCurPos(AXIS_Y2);
	dCurPos[3] = pImpl->GetCurPos(AXIS_LOADER_R);

	int iReTry = 0, nRetrylimit = 3;
	double dAlignMargin = 1;
	bool bAlignDone = false;
	bool bCentered = false;	// LYS 2026-09-05 마진 통과 여부 - 루프 내 DoWork_MotionDone이 nRet의 CENTERING_FAIL을 덮어쓰는 문제 보완

	dAlignMargin = pAlignInfo->m_sMargin.m_sGlassMargin.m_fR; 

	nRetrylimit = pAlignInfo->m_sMargin.m_sGlassMargin.m_nCentering;

	// 반복
	do
	{
		// 이미지 패턴 매칭
		for (size_t j = 0; j < 2; j++)	// 2 Head
		{
			MaxSorce = -1;

			if (j == 0)
			{
				for (size_t k = 0; k < df_MAX_PTRNINFO; k++)
				{
					if (k < 5)
					{
						ptrnInfo[k].m_nThreshold = pAlignInfo->m_sCcdSet.m_sFrontLeft.m_nMatchingScore;
						ptrnInfo[k].m_strName.Format(_T("%s"), pAlignInfo->m_chPtnFrontLeft[k]);
					}
					
				}
			}
			else
			{
				for (size_t k = 0; k < df_MAX_PTRNINFO; k++)
				{
					if (k < 5)
					{
						ptrnInfo[k].m_nThreshold = pAlignInfo->m_sCcdSet.m_sFrontRight.m_nMatchingScore;
						ptrnInfo[k].m_strName.Format(_T("%s"), pAlignInfo->m_chPtnFrontRight[k]);
					}

				}
			}

			// 이미지 패턴 매칭
#if 0	// LYS 2026-09-25 구 코드: 첫 성공 패턴에서 break -> 최고 점수 선택이 아니었다. 아래 PtrnMatchBest 로 대체.
			for (size_t k = 0; k < df_MAX_PTRNINFO; k++)	// Pattern
			{
				pPtrnInfo = &ptrnInfo[k];
				nRet = m_pShareItem->GetAlignCameraMgr(j)->PtrnMatch(pPtrnInfo, &Score, LENS_USE_ALIGN);
				if (nRet == VTM_RV_SUCCESSED)
				{
					if (Score > MaxSorce)
					{
						MaxSorce = Score;
						iPtrn[j] = k;
						dPixelx[j] = pPtrnInfo->m_dRealX;	// 차이값(mm)
						dPixely[j] = pPtrnInfo->m_dRealY;	// 차이값(mm)
					}

					break;
				}
			}
#else
			// LYS 2026-09-25 등록 패턴 전부 매칭해 점수가 가장 높은 패턴 채택 (스냅 1회, 패턴별 점수는 [AlignCCDn] PtrnMatchBest 로그).
			{
				int nBestPtrn = -1;
				nRet = m_pShareItem->GetAlignCameraMgr(j)->PtrnMatchBest(ptrnInfo, df_MAX_PTRNINFO, &Score, &nBestPtrn, LENS_USE_ALIGN);
				if (nRet == VTM_RV_SUCCESSED && nBestPtrn >= 0)
				{
					MaxSorce = Score;
					iPtrn[j] = nBestPtrn;
					dPixelx[j] = ptrnInfo[nBestPtrn].m_dRealX;	// 차이값(mm)
					dPixely[j] = ptrnInfo[nBestPtrn].m_dRealY;	// 차이값(mm)
				}
			}
#endif

			
		}

		bAlignDone = true;

		for (size_t j = 0; j < 2; j++)
		{
		//	if (iPtrn[j] < -1)
			if (iPtrn[j] < 0)	// 2023.03.09 change
			{
				bAlignDone = false;
				break;
			}
		}

		if (bAlignDone == false)
		{
			nRet = VISION_ALIGN_DETECTED_FAIL;
			break;
		}

		// 센터링
		// LYS 2026-09-12 CCD 얼라인 카메라 방향이 바뀌어 XYR 보상 부호를 전부 반전. 2026-09-12 16:35 Glass XYZero 실측이 -0.0794→-0.1588→-0.3175 / 0.1436→0.2835→0.5708 로 정확히 2배씩 커지는 1:1 발산(=부호 반대)이었다. 아래 2026-09-05 주석은 방향 변경 전 근거라 참고용으로 남긴다.
		int iDir = 1;	// LYS 2026-09-12 카메라 방향 변경으로 -1 -> 1 반전 (R)
		// LYS 2026-09-05 부호 반전 - 11:46 실측에서 측정각 방향으로 R 이동 시 각도가 -0.008→-0.020→-0.045°로 발산(같은 부호). XYZero와 동일 현상
		double dDistanceX = 0., dDistanceY = 0., dRadian = 0.;

		// 
		// Angle 계산
		COORDINATE stBase[2];		// Align Mark 좌표 위치
		COORDINATE stDetect[2];

		// Probe
		stBase[0].m_dy = pImpl->GetCurPos(AXIS_X2);
		stBase[0].m_dx = pImpl->GetCurPos(AXIS_Y1);

		stBase[1].m_dy = pImpl->GetCurPos(AXIS_X2);
		stBase[1].m_dx = pImpl->GetCurPos(AXIS_Y2);

		stDetect[0].m_dx =  stBase[0].m_dx - dPixelx[0];
		stDetect[0].m_dy =  stBase[0].m_dy - dPixely[0];

		stDetect[1].m_dx = stBase[1].m_dx - dPixelx[1];
		stDetect[1].m_dy = stBase[1].m_dy - dPixely[1];

		// 0.00858 -> 0.2
		dRadian = m_pShareItem->GetAlignCameraMgr(0)->CalcAngle(stBase[0], stBase[1], stDetect[0], stDetect[1]);
		if (dRadian > 180)
			dRadian -= 360;

		/*
		// Save Image
		CString strPath, strSaveImage;
		CString strCellID;
		strCellID = CSystemParamShare::GetInstance()->GetCellID();
		strPath.Format(_T("%s%s%s\\%s\\"), m_pShareItem->GetSystemConfigMgrPtr()->GetLogFilePath(), _T("ALIGN\\"), m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeID(), strCellID);
		MakeDir(strPath);

		strSaveImage.Format(_T("%s%s_%s.png"), strPath, _T("AlignGlassThetaZeroSrc1"), GetCurTimeStr());
		imaqWritePNGFile2(pSnapImageCCD1, CStrToChar(strSaveImage), 750, NULL, FALSE);

		strSaveImage.Format(_T("%s%s_%s.png"), strPath, _T("AlignGlassThetaZeroSrc2"), GetCurTimeStr());
		imaqWritePNGFile2(pSnapImageCCD2, CStrToChar(strSaveImage), 750, NULL, FALSE);*/

		// Margin 
		if (nRet == VTM_RV_SUCCESSED)
		{
			/*if (m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
			{
				break;
			}*/

			CString str;
			str.Format(_T("AlignGlassTZero %d/%d, %.4f < %.4f, %.4f, %.4f"), iReTry + 1, nRetrylimit, dRadian, dAlignMargin, stDetect[0].m_dy, stDetect[1].m_dy);
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, str);

			
			if (fabs(dRadian) < dAlignMargin)
			{
				bCentered = true;	// LYS 2026-09-05 마진 통과
				break;
			}
			else
			{
				nRet = VISION_ALIGN_CENTERING_FAIL;

				
				// Move
				if (pImpl != nullptr)
				{
					// 이동
					pImpl->MoveCmdPosR(AXIS_LOADER_R, dRadian * iDir, true, SPEED_LOW);

					nRet = pImpl->DoWork_MotionDone(AXIS_LOADER_R);
					if (nRet != df_MOTION_RV_SUCCESSED)
					{
						break;
					}
				}
			}
		}

		iReTry++;

	} while (iReTry < nRetrylimit);

	// LYS 2026-09-05 재시도 한도 내에 마진을 못 맞추면 실패로 반환 (구 코드는 마지막 MotionDone 성공이 실패 상태를 지워 발산한 오프셋이 그대로 적용됨)
	if (nRet == VTM_RV_SUCCESSED && bCentered == false)
	{
		nRet = VISION_ALIGN_CENTERING_FAIL;
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(WARN, _T("AlignGlassTZero centering NOT converged within retry limit - align FAIL"));
	}

	if (nRet == VTM_RV_SUCCESSED)
	{
		*pdMovR = pImpl->GetCurPos(AXIS_LOADER_R) - dCurPos[3];
	//	*pdMovR = pImpl->GetCurPos(AXIS_LOADER_R) - 0;
	}

	return nRet;
}


// 
int CDoWorkGlassSWAlignStep::AlignGlass()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassSWAlignStep Start."));

	int nRet = VTM_RV_SUCCESSED;
	double dMovX = 0, dMovY = 0, dMovR = 0;

	CMaintControlMgr* pImpl = nullptr;
	pImpl = m_pShareItem->GetMaintControlMgrPtr();
	if (pImpl != nullptr)
		pImpl->SetGlassAlignXYR(dMovX, dMovY, dMovR);

	// 패턴 Search
	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_ALIGN_PATTERN_SEARCH, true);
//	Sleep(1000);
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("[AlignGlass] >>> Pattern SEARCH begin"));
	nRet = AlignGlassSearch();

	CString strSrch; strSrch.Format(_T("[AlignGlass] SEARCH done nRet=%d"), nRet); m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strSrch);
	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_ALIGN_PATTERN_SEARCH, false);

	// 센터링
	if (nRet == VTM_RV_SUCCESSED)
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_ALIGN_PATTERN_CENTERING, true);

		if (m_pShareItem->GetMaintControlMgrPtr()->IsOutOfServiceMC() != true)
		{
			if (nRet == VTM_RV_SUCCESSED)
			{
				m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("[AlignGlass] >>> TZero (theta) begin"));
				nRet = AlignGlassTZero(&dMovR);
			}

			if (nRet == VTM_RV_SUCCESSED)
			{
				m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("[AlignGlass] >>> XYZero (position) begin"));
				nRet = AlignGlassXYZero(&dMovX, &dMovY);
			}
		}
		else
		{
			Sleep(500);
		}
		
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_ALIGN_PATTERN_CENTERING, false);
	}

	if (nRet == VTM_RV_SUCCESSED)
	{
		// Align Offset
		/*CMaintControlMgr* pImpl = nullptr;
		pImpl = m_pShareItem->GetMaintControlMgrPtr();*/

		if (pImpl != nullptr)
		{

			// jsyoon 2022.08.16 add
			CString strLog;
			strLog.Format(_T("AlignGlass (XYR:%.4f,%.4f,%.4f)"), dMovX, dMovY, dMovR);
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);

			pImpl->SetGlassAlignXYR(dMovX, dMovY, dMovR);
			pImpl->SetGlassAlignDone(true);	// LYS 2026-09-15 Glass Align 완료 표시(타이틀바 파랑)
		}
			
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassSWAlignStep End."));

	return nRet;
}


bool CDoWorkGlassSWAlignStep_None::Run()
{
	// init
	m_pShareItem->SetComplete(false);
	m_pShareItem->SetAlarmOn(false);
	m_pShareItem->SetFail(false);
	m_pShareItem->SetStopUser(false);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassSWAlignStep_None Start."));

	m_pShareItem->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_PROCESS);	// 2023.08.09 add

	/*m_pShareItem->SetAlarmOn(false);
	m_pShareItem->SetComplete(false);
	m_pShareItem->SetFail(false);*/

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_ALIGN, true);


	/*if (m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
	{ 
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassSWAlignStep_None End. TEST"));
		return true;
	}*/
		
	//stRecipeUTGParam    stRecipeUTG = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeDataPtr()->GetParam_RecipeInfo()->stRecipeUTG;

	long nRet = df_MOTION_RV_SUCCESSED;
	//int nMeasHead = 0;
	//nMeasHead = stRecipeUTG.nUseMeasHead;

	//// Glass ID
	//m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassSWAlignStep_None SET_PARAMETER Set."));
	//nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkSetParameter(nMeasHead);

	//// Recipe Open
	//if (nRet == df_MOTION_RV_SUCCESSED)
	//{
	//	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassSWAlignStep_None RECIPE_CHANGE Set."));
	//	CString strRcp;
	//	strRcp.Format(_T(""), m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeID());
	//	nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkRecipeOpen(nMeasHead);
	//}

	//// Vaccum On
	//if (nRet == df_MOTION_RV_SUCCESSED)
	//{
	//	if (m_pShareItem->GetIOMapControlMgrPtr()->SetStageVaccumOn() != true)
	//	{
	//		nRet = PLC_VACUUM_SENSOR_ERROR;
	//	}
	//}
	//
	bool bRet = true;
	//if (nRet != df_MOTION_RV_SUCCESSED)
	//{
	//	m_pShareItem->SetFail(true);
	//	bRet = false;
	//}

//	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassSWAlignStep_None End."));

    return bRet;
}

bool CDoWorkGlassSWAlignStep_CheckSWAlignParam::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassSWAlignStep_CheckSWAlignParam Start."));
	
	if(m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
        return true;
	
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassSWAlignStep_CheckSWAlignParam End."));
	
	return true;
}

bool CDoWorkGlassSWAlignStep_MoveAlignPosistion_1st::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassSWAlignStep_MoveAlignPosistion_1st Start."));
	
	if(m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
        return true;

	if (m_pShareItem->GetComplete())
		return true;

	if (MoveAlignPosition(0, 0) != df_MOTION_RV_SUCCESSED)
	{
		//Error
		return false;
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassSWAlignStep_MoveAlignPosistion_1st End."));
	
	return true;
}

bool CDoWorkGlassSWAlignStep_AF_1st::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassSWAlignStep_AF_1st Start."));
	
	if(m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
        return true;

	if (m_pShareItem->GetComplete())
		return true;

	if (AlignAF(0) != 1)
	{
		//Error
		return false;
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassSWAlignStep_AF_1st End."));
	
	return true;
}

bool CDoWorkGlassSWAlignStep_CenteringAlignPattern_1st::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassSWAlignStep_CenteringAlignPattern_1st Start."));
	
	if(m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
        return true;

	if (m_pShareItem->GetComplete())
		return true;


	if (CenteringAlignPattern(0) == 1)
	{
		double dPosY = m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_Y, 1, 1);
		double dPosS = m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_XS, 1, 1);

//		m_pShareItem->GetAlignMatrixMgr()->SetParamDetPos(0, dPosS, dPosY, 0.0);
	}
	else
	{
//		m_pShareItem->GetAlignMatrixMgr()->SetParamDetPos(0, 0, 0, 0.0);

		return false;
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassSWAlignStep_CenteringAlignPattern_1st End."));
	
	return true;
}

bool CDoWorkGlassSWAlignStep_MoveAlignPosistion_2nd::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassSWAlignStep_MoveAlignPosistion_2nd Start."));
	
	if(m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
        return true;

	if (m_pShareItem->GetComplete())
		return true;

	if (MoveAlignPosition(1, 1) != df_MOTION_RV_SUCCESSED)
	{
		//Error
		return false;
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassSWAlignStep_MoveAlignPosistion_2nd End."));
	
	return true;
}

bool CDoWorkGlassSWAlignStep_AF_2nd::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassSWAlignStep_AF_2nd Start."));
	
	if(m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
        return true;

	if (m_pShareItem->GetComplete())
		return true;

	if (AlignAF(1) != 1)
	{
		//Error
		return false;
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassSWAlignStep_AF_2nd End."));
	
	return true;
}

bool CDoWorkGlassSWAlignStep_CenteringAlignPattern_2nd::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassSWAlignStep_CenteringAlignPattern_2nd Start."));
	
	if(m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
        return true;

	if (m_pShareItem->GetComplete())
		return true;

	if (CenteringAlignPattern(1) == 1)
	{
		double dPosY = m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_Y2, 1, 1);
		double dPosS = m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_XS2, 1, 1);

//		m_pShareItem->GetAlignMatrixMgr()->SetParamDetPos(1, dPosS, dPosY, 0.0);
	}
	else
	{
//		m_pShareItem->GetAlignMatrixMgr()->SetParamDetPos(1, 0, 0, 0.0);

		return false;
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassSWAlignStep_CenteringAlignPattern_2nd End."));
	
	return true;
}

bool CDoWorkGlassSWAlignStep_CalcAlignOffset::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassSWAlignStep_CalcAlignOffset Start."));
	
	if (m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
		return true;

	if (m_pShareItem->GetComplete())
		return true;

//	m_pShareItem->GetAlignMatrixMgr()->CalcAlignMatrix();

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassSWAlignStep_CalcAlignOffset End."));
	
	return true;
}

bool CDoWorkGlassSWAlignStep_Idle::Run()
{
	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_Idle, NULL);

	// Set CCD and Light
	//for (size_t i = 0; i < MAIN_CCD_MAX_NUM; i++)
	//{
	//	m_pShareItem->GetLightControlMgrPtr()->SetLedLampIntensity(i, 0);
	//}

	if (m_pShareItem->GetFail() != true)
	{
		m_pShareItem->SetComplete(true);   // 동작 완료
	}

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_ALIGN, false);

	// 완료
    ResetEvent(m_pShareItem->GetStartEventHandle());

    SetEvent(m_pShareItem->GetCompleteEventHandle()); // 시퀀스 완료 이벤트 SET

    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("DoWorkGlassSWAlignStep_Idle"));

	m_pShareItem->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_IDLE);	// 2023.08.09 add

    return false;
}

bool CDoWorkGlassSWAlignStep_MoveAlignPosistion::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassSWAlignStep_MoveAlignPosistion Start."));

	if (m_pShareItem->GetComplete())
		return true;

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MOVEING_Start, NULL);	// 2023.03.09 change

	// jsyoon 2022.08.23 add
	bool bGlassExist = false;
	if (m_pShareItem->GetMaintControlMgrPtr()->GetIOMapControlMgrPtr()->IsOutOfService() != true)
	{
		bGlassExist = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsGlassSensorOn();
	}
	else
	{
		bGlassExist = true;
	}
	
	bool bRet = true;
	//bGlassExist = true;		// LYS 2026-09-05 임시 - Glass 유무 센서 판정 강제 통과(테스트용, 양산 전 제거)
	if (bGlassExist != true)
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassSWAlignStep_MoveAlignPosistion Glass Exist NG."));

		m_pShareItem->SetFail(true, PLC_GLASS_OBLIVIOUS_WORKPLATE_SENSOR_ALARM);
		bRet = false;
		//return false;
	}
	else
	{
		int nRet = VTM_RV_SUCCESSED;
		nRet = MoveAlignPosition(0, 0);
		if (nRet != df_MOTION_RV_SUCCESSED)
		{
			CString strLog;
			strLog.Format(_T("CDoWorkGlassSWAlignStep_MoveAlignPosistion NG. %d"), nRet);
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(WARN, strLog);
			m_pShareItem->SetFail(true, nRet);
			bRet = false;
		}
	}

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MOVEING_End, NULL);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassSWAlignStep_MoveAlignPosistion End."));

	return bRet;
}

bool CDoWorkGlassSWAlignStep_AlignGlass::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassSWAlignStep_AlignGlass Start."));

	if (m_pShareItem->GetComplete())
		return true;

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_ALIGN_Start, NULL);	// 2023.03.09 change

	bool bRet = true;
	int nRet = VTM_RV_SUCCESSED;

	nRet = AlignGlass();

	if (nRet == VTM_RV_SUCCESSED)
	{
		m_pShareItem->SetComplete(true);   // 동작 완료
	}
	else
	{
		m_pShareItem->SetFail(true, nRet);
		bRet = false;
	}

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_ALIGN_End, NULL);	// 2023.03.09 change

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassSWAlignStep_AlignGlass End."));

	return bRet;
}