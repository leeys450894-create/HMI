#include "pch.h"
#include "DoWorkGlassPreMeasStep.h"

#include "DoWorkGlassSequenceImpl.h"
#include "..\SystemParamShare.h"
#include "..\Manager\SystemConfigMgr.h"
#include "..\Manager\CommTcpIpFactoryMgr.h"
#include "..\Recipe\RecipeEditMgr.h"
#include "..\Manager\MaintControlMgr.h"
#include "..\Manager\LightControlMgr.h"
#include "..\Manager\AlignCameraMgr.h"
#include "..\Manager\MCCLogMgr.h"
#include "..\Manager\SequenceManager.h"
#include "..\FunctionModule\FunctionModuleEQState.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

bool CDoWorkGlassPreMeasStep_None::Run()
{
    //	if(m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
    //		return true;

	// init
	m_pShareItem->SetComplete(false);
	m_pShareItem->SetAlarmOn(false);
	m_pShareItem->SetFail(false);
	m_pShareItem->SetStopUser(false);

	/*m_pShareItem->SetAlarmOn(false);
	m_pShareItem->SetComplete(false);
	m_pShareItem->SetFail(false);*/

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS, true);

	m_pShareItem->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_PROCESS);	// 2023.08.09 add

	m_pShareItem->GetMaintControlMgrPtr()->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_BIT_W_GLASS_SENSOR_POWER_ON, true); // jsyoon 2022.05.26 add
	
    return true;
}

bool CDoWorkGlassPreMeasStep_Idle::Run()
{
    CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_Idle, NULL);

    // 완료
    ResetEvent(m_pShareItem->GetStartEventHandle());
 
	if (m_pShareItem->GetFail() != true)
	{
		m_pShareItem->SetComplete(true);   // 동작 완료

		m_pShareItem->GetMaintControlMgrPtr()->GetIOMapControlMgrPtr()->SetMemoryBitToDevice(PLC_BIT_W_GLASS_SENSOR_POWER_ON, false); // jsyoon 2022.05.26 add
	}
	else
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS, true);
	}
    
    SetEvent(m_pShareItem->GetCompleteEventHandle()); // 시퀀스 완료 이벤트 SET

    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassPreMeasStep_Idle"));

	m_pShareItem->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_IDLE);	// 2023.08.09 add

    return false;
}

bool CDoWorkGlassPreMeasStep_Move::Run()
{
	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS_CONTACT_PROC, true);

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MOVEING_Start, NULL);	// 2023.03.09 change
//	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_P2P_Start, NULL);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassPreMeasStep_Move Start."));

	for (size_t i = 0; i < 5; i++)
	{
		m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[i] = -1;
	}

	bool bRet = false;
	long lRet = VTM_RV_SUCCESSED;
	int nMeasMode = 0;

	// Position
	double dPosX = 0.;
	double dProbeX = 0, dProbeY = 0;


	S_RecipeData* pRecipeData;
	pRecipeData = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeData();
	int iStick = 0, nArea;
	iStick = 0;


	double dOffsetX = 0., dOffsetR = 0.;
	S_RecipeInfo* pRecipeInfo;
	pRecipeInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
	dOffsetX = pRecipeInfo->m_sMotionOffsetPos.m_fStageY;
	dOffsetR = pRecipeInfo->m_sMotionOffsetPos.m_fStageRotate;	// jsyoon 2022.08.26 add

	double dStageY = 0., dStageR = 0;
//	double dStageY = 0, dStageR = 0;
	dStageR = pRecipeData->m_vecStick[0].m_fStageR + dOffsetR;
	// LYS 2026-06-26 OCP: Stage moves to stick[0] position (Probe X fixed at 0)
	if (m_pShareItem->GetSystemConfigMgrPtr() && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP())
		dStageY = pRecipeData->m_vecStick[0].m_fStageY + dOffsetX;
	else
		dStageY = 0. + dOffsetX;
//	dStageY = pRecipeData->m_vecStick[0].m_fStageY + dOffsetX;;


	int nCell = 0;
	nCell = pRecipeData->m_vecStick[iStick].m_vecCell.size();

	int iLampIntensity[2];
	ZeroMemory(&iLampIntensity, sizeof(iLampIntensity));

	// jsyoon 2022.05.12 add
	double dLeftReview = 0., dRightReview = 0.;
	//S_RecipeInfo* pRecipeInfo;
	//pRecipeInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
	dRightReview = pRecipeInfo->m_sMotionOffsetPos.m_fHeadReviewLeft;
	dLeftReview = pRecipeInfo->m_sMotionOffsetPos.m_fHeadReviewRight;

	if (pRecipeInfo != nullptr)
	{
		iLampIntensity[0] = pRecipeInfo->m_sLampItem.m_nFrontLeft;
		iLampIntensity[1] = pRecipeInfo->m_sLampItem.m_nFrontRight;
	/*	iLampIntensity[0] = pRecipeInfo->m_sLampItem.m_nFrontRight;
		iLampIntensity[1] = pRecipeInfo->m_sLampItem.m_nFrontLeft;*/
	}

	/*S_Align* pAlignInfo = nullptr;
	pAlignInfo = &m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeMgrPtr()->m_sAlign;
	if (pAlignInfo != nullptr)
	{
		iLampIntensity[0] = pAlignInfo->m_sCcdSet.m_sFrontLeft.m_nLed;
		iLampIntensity[1] = pAlignInfo->m_sCcdSet.m_sFrontRight.m_nLed;
	}*/

	// Set CCD and Light
	int iCh[4] = { 3, 2, 1, 0 };

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_CHNG_LAMP, true);

	for (size_t i = 0; i < 2; i++)
	{
		m_pShareItem->GetLightControlMgrPtr()->SetLedLampIntensity(0, iCh[i], iLampIntensity[i]);
	}

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_CHNG_LAMP, false);

	double dLeft = 0., dRight = 0., dX = 0;
	if (pRecipeData->m_vecStick[iStick].m_vecCell[0].m_fCellX > pRecipeData->m_vecStick[iStick].m_vecCell[nCell - 1].m_fCellX)
	{
		dLeft = pRecipeData->m_vecStick[iStick].m_vecCell[0].m_fCellX;
		dRight = pRecipeData->m_vecStick[iStick].m_vecCell[nCell - 1].m_fCellX;
	}
	else
	{
		dLeft = pRecipeData->m_vecStick[iStick].m_vecCell[nCell - 1].m_fCellX;
		dRight = pRecipeData->m_vecStick[iStick].m_vecCell[0].m_fCellX;
	}

	// jsyoon 2022.05.12 add
	dLeft  += dLeftReview;
	dRight += dRightReview;

	CMaintControlMgr* pImpl = nullptr;
	pImpl = m_pShareItem->GetMaintControlMgrPtr();

	if (lRet == VTM_RV_SUCCESSED)
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS_CONTACT_PROC_STEP1, true);

		lRet = pImpl->MoveCmdPos_ProbeMove(AXIS_ALL_Z, 0, SPEED_HIGH);

		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS_CONTACT_PROC_STEP1, false);
	}

	if (lRet == VTM_RV_SUCCESSED)
	{
		for (size_t i = df_PROBE_AGING1; i <= df_PROBE_MEAS; i++)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsProbeUnContactSensorOn(i);
			if (bRet != true)
			{
				lRet = MOTION_INTERLOCK_6;
				break;
			}
		}
	}

	if (lRet == VTM_RV_SUCCESSED)
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS_CONTACT_PROC_STEP2, true);

		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsLoaderArmSensorOn();
		if (bRet == true)
		{
			lRet = PLC_ARM_SENSOR_DETECT;
		}

		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS_CONTACT_PROC_STEP2, false);
	}

	if (lRet == VTM_RV_SUCCESSED)
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS_CONTACT_PROC_STEP3, true);

		bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsShutterSensorClose();
		if (bRet != true)
		{
			lRet = PLC_LOADER_SHUTTER_UP_ERROR;
		}

		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS_CONTACT_PROC_STEP3, false);
	}

	if (lRet == VTM_RV_SUCCESSED)
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS_CONTACT_PROC_STEP4, true);

		for (size_t i = df_PROBE_AGING1; i <= df_PROBE_MEAS; i++)
		{
			bRet = CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsProbeUnContactSensorOn(i);
			if (bRet != true)
			{
				lRet = MOTION_INTERLOCK_6;
				break;
			}
		}
		
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS_CONTACT_PROC_STEP4, false);
	}

	if (lRet == VTM_RV_SUCCESSED)
	{
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS_CONTACT_MOVE, true);

		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS_CONTACT_MOVE_STAGE_STEP1, true);

		if (lRet == df_MOTION_RV_SUCCESSED)
		{
			lRet = pImpl->MoveCmdPosA(AXIS_LOADER_R, dStageR, true, SPEED_HIGH, 0, 0);
		}

		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS_CONTACT_MOVE_STAGE_STEP1, false);
		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS_CONTACT_MOVE_STAGE_STEP2, true);
		
		if (lRet == df_MOTION_RV_SUCCESSED)
		{
			lRet = pImpl->MoveCmdPosA(AXIS_X, dStageY, true, SPEED_HIGH, 0, 0);
		}

		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS_CONTACT_MOVE_STAGE_STEP2, false);

		//// Probe X
		if (lRet == df_MOTION_RV_SUCCESSED)
		{
			double dProbeGap = 0., dProbeShift = 0.;
			S_RecipeMap* pRecipeMap;
			pRecipeMap = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeMap();

			dProbeShift = pRecipeInfo->m_sMotionOffsetPos.m_fProbeShift;

			dProbeGap = pRecipeMap->m_fCellDistance.y;
			stMotorPosition stPositionProbeX[5];
			ZeroMemory(&stPositionProbeX, sizeof(stPositionProbeX));

			stPositionProbeX[0].axis = AXIS_PROBE_X1;
			stPositionProbeX[1].axis = AXIS_PROBE_X3;
			stPositionProbeX[2].axis = AXIS_PROBE_X5;
			stPositionProbeX[3].axis = AXIS_PROBE_X7;
			stPositionProbeX[4].axis = AXIS_PROBE_X9;

			int nStick = 0;
			nStick = pRecipeData->m_vecStick.size();

			// LYS 2026-06-26 OCP mode: only P5 (MEAS) probe, skip P1-P4 positioning
			if (m_pShareItem->GetSystemConfigMgrPtr() && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP())
			{
				// LYS 2026-06-29
				// P5 position = first stick position
				//stPositionProbeX[4].dPosition = pRecipeData->m_vecStick[0].m_fStageY + dProbeShift + dOffsetX;
				//m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4] = 0;  // Meas
				//for (int i = 0; i < 4; i++)
				//	m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[i] = -1;  // No Aging
				////lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_PROBE_X9, stPositionProbeX[4].dPosition, true, SPEED_HIGH);
				//lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_X, stPositionProbeX[4].dPosition, true, SPEED_HIGH);		// LYS 2026-06-26 Contact Position : Move Stage, Probe Gantry Not Move(Only Move Z)

			}
			else if (nStick > 2)
			{
				double dProbeInterlockPos = 0;
				m_pShareItem->GetMaintControlMgrPtr()->GetInterlockPos(nullptr, nullptr, &dProbeInterlockPos);


				// jsyoon 2022.09.29 add
				int iRotate = 0;
				if (pRecipeInfo->m_sOptionItem.m_nGlsRotate == 1)
				{
					iRotate = 1;
				}
				///

				double fOffsetGap = 2.;	// 2023.05.12 change 3 to 2

				int k = 0;
				bool bSetPos[5];
				ZeroMemory(&bSetPos, sizeof(bSetPos));

				for (int i = 0; i < 5; i++)  // Probe 5개 
				{
					if (bSetPos[4 - i] == true)
						continue;

					if (i < nStick)
					{
						if (i == 0)
						{
							stPositionProbeX[4 - i].dPosition = pRecipeData->m_vecStick[i].m_fStageY + dProbeShift + dOffsetX;

							// jsyoon 2022.09.29 change
							if (iRotate == 1)
								m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - i] = i;  // Meas
							else
								m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - i] = nStick - i - 1;  // Meas

							bSetPos[4 - i] = true;

							//m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - i] = i;  // Meas
							k = i;
						}
						else
						{
							if (k < nStick)
							{
								for (int j = k + 1; j < nStick; j++)
								{
									if (pRecipeData->m_vecStick[k].m_fStageY - pRecipeData->m_vecStick[j].m_fStageY > (dProbeInterlockPos + fOffsetGap) * 2)	// 2023.05.12 Stick 간 간격이 넓을 경우
									{
										int nCurProbe = 4 - i;
										int nNextProbe = 4 - i - 1;
										// Next Probe
										stPositionProbeX[nNextProbe].dPosition = pRecipeData->m_vecStick[j].m_fStageY + dProbeShift + dOffsetX;
										double dMinAxisX3 = m_pShareItem->GetMaintControlMgrPtr()->GetMinSWLimit(AXIS_X3);
										if (stPositionProbeX[nNextProbe].dPosition > dMinAxisX3)
										{
											if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsProbeExistSensorOn(nNextProbe + df_PROBE_AGING1) == true) 	// 2022.12.21 add Probe Exist
											{
												// jsyoon 2022.09.29 change
												if (iRotate == 1)
													m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[nNextProbe] = j;	 // Aging
												else
													m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[nNextProbe] = nStick - j - 1;	 // Aging
											}
										}

										bSetPos[nNextProbe] = true;

										// Cur Probe
										stPositionProbeX[nCurProbe].dPosition = stPositionProbeX[nNextProbe].dPosition + (dProbeInterlockPos + fOffsetGap);
										bSetPos[nCurProbe] = true;

										k = j;

										break;
									}
									else if (pRecipeData->m_vecStick[k].m_fStageY - pRecipeData->m_vecStick[j].m_fStageY > (dProbeInterlockPos))
									{
										stPositionProbeX[4 - i].dPosition = pRecipeData->m_vecStick[j].m_fStageY + dProbeShift + dOffsetX;

										// jsyoon 2022.09.21 add
										double dMinAxisX3 = m_pShareItem->GetMaintControlMgrPtr()->GetMinSWLimit(AXIS_X3);
										if (stPositionProbeX[4 - i].dPosition > dMinAxisX3)
										{
											if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsProbeExistSensorOn(4 - i + df_PROBE_AGING1) == true) 	// 2022.12.21 add Probe Exist
											{
												// jsyoon 2022.09.29 change
												if (iRotate == 1)
													m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - i] = j;	 // Aging
												else
													m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - i] = nStick - j - 1;	 // Aging

												// m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - i] = j;	 // Aging
											}

											bSetPos[4 - i] = true;
										}

										k = j;
										break;
									}
								}
							}
						}
					}
					else
					{
						//	double fOffsetGap = 6.0;
						//	stPositionProbeX[4 - i].dPosition = stPositionProbeX[5 - i].dPosition - ((dProbeGap + fOffsetGap)) + dOffsetX;
						//	stPositionProbeX[4 - i].dPosition = pRecipeData->m_vecStick[0].m_fStageY - ((dProbeGap + fOffsetGap) * i) + dOffsetX;

						m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - i] = -1;
					}
				}

				for (int i = 1; i < 5; i++)
				{
					if (m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - i] == -1)
					{
						//	if (bSetPos[i] != true)
						if (bSetPos[4 - i] != true)	 // 2023.02.14 change
						{

							stPositionProbeX[4 - i].dPosition = stPositionProbeX[5 - i].dPosition - ((dProbeInterlockPos + fOffsetGap));
							//	stPositionProbeX[4 - i].dPosition = pRecipeData->m_vecStick[0].m_fStageY - ((dProbeGap + fOffsetGap) * i) + dOffsetX;
						}
					}
				}

				//lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosMulti(stPositionProbeX, 5, SPEED_HIGH, true);		// LYS 2026-06-29
			}
		}

		//// Probe Y
		if (lRet == df_MOTION_RV_SUCCESSED)
		{
			//	dProbeY = pImpl->GetCurPos(AXIS_PROBE_Y5, 0, 0);
			stMotorPosition stPositionProbeY[5];
			ZeroMemory(&stPositionProbeY, sizeof(stPositionProbeY));

			stPositionProbeY[0].axis = AXIS_PROBE_Y1;
			stPositionProbeY[0].dPosition = dProbeY;
			stPositionProbeY[1].axis = AXIS_PROBE_Y2;
			stPositionProbeY[1].dPosition = dProbeY;
			stPositionProbeY[2].axis = AXIS_PROBE_Y3;
			stPositionProbeY[2].dPosition = dProbeY;
			stPositionProbeY[3].axis = AXIS_PROBE_Y4;
			stPositionProbeY[3].dPosition = dProbeY;
			stPositionProbeY[4].axis = AXIS_PROBE_Y5;
			stPositionProbeY[4].dPosition = dProbeY;

			// LYS 2026-06-26 OCP mode: skip all Probe Y (not used)
			if (!(m_pShareItem->GetSystemConfigMgrPtr() && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP()))
			{
				lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosMulti(stPositionProbeY, 5, SPEED_HIGH, true);
			}
		}

		if (lRet == df_MOTION_RV_SUCCESSED)
		{
			int nAxis = 0;
			stMotorPosition stPosition[10];
			ZeroMemory(&stPosition, sizeof(stPosition));
			stPosition[0].axis = AXIS_Y1;   nAxis++;
			stPosition[0].dPosition = dLeft;
			stPosition[1].axis = AXIS_Y2;   nAxis++;
			stPosition[1].dPosition = dRight;
			stPosition[2].axis = AXIS_Y3;   nAxis++;
			stPosition[2].dPosition = dLeft;
			stPosition[3].axis = AXIS_Y4;   nAxis++;
			stPosition[3].dPosition = dRight;
			stPosition[4].axis = AXIS_X2;   nAxis++;
			//stPosition[4].dPosition = pRecipeData->m_vecStick[0].m_fStageY + dOffsetX;
			stPosition[4].dPosition = dOffsetX;		// LYS 2026-06-29
			stPosition[5].axis = AXIS_Z1;   nAxis++;
			stPosition[5].dPosition = pRecipeData->m_sHeadZ.m_fQualityZ;
			stPosition[6].axis = AXIS_Z2;   nAxis++;
			stPosition[6].dPosition = pRecipeData->m_sHeadZ.m_fReviewZ;

			lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosAging(stPosition, nAxis, SPEED_HIGH);
		}

		CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS_CONTACT_MOVE, false);
	}

	if (lRet == df_MOTION_RV_SUCCESSED)
	{
		bRet = true;
	}
	else
	{
		bRet = false;

		m_pShareItem->SetFail(true, lRet);
	}
	
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassPreMeasStep_Move End. End"));

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MOVEING_End, NULL);
//	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_End, NULL);

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS_CONTACT_PROC, false);

	return bRet;
}

bool CDoWorkGlassPreMeasStep_PtrnSearch::Run()
{
    CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_PATTERN_SEARCH_Start, NULL);

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS_CONTACT_SEARCH_PTRN, true);

    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassPreMeasStep_PtrnSearch Start."));

    bool bRet = false;
    long lRet = VTM_RV_SUCCESSED;

    bool bUseMeas = false;
    S_RecipeInfo* pReciepInfo;
    pReciepInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
    if (pReciepInfo != nullptr)
    {
        bUseMeas = pReciepInfo->m_sOptionItem.m_bUseReviewSnap;
    }

	int iLampIntensity[2];
	int iExposureTime[2];
	int iCCDGain[2];

	ZeroMemory(&iLampIntensity, sizeof(iLampIntensity));
	ZeroMemory(&iExposureTime, sizeof(iExposureTime));
	ZeroMemory(&iCCDGain, sizeof(iCCDGain));

	S_RecipeInfo* pRecipeInfo;
	pRecipeInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
	if (pRecipeInfo != nullptr)
	{
		iLampIntensity[0] = pRecipeInfo->m_sLampItem.m_nFrontLeft;
		iLampIntensity[1] = pRecipeInfo->m_sLampItem.m_nFrontRight;
		/*iLampIntensity[0] = pRecipeInfo->m_sLampItem.m_nFrontRight;
		iLampIntensity[1] = pRecipeInfo->m_sLampItem.m_nFrontLeft;*/
	}

	S_Align* pAlignInfo = nullptr;
	pAlignInfo = &m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeMgrPtr()->m_sAlign;

	if (pAlignInfo != nullptr)
	{
		/*iLampIntensity[0] = pAlignInfo->m_sCcdSet.m_sFrontLeft.m_nLed;
		iLampIntensity[1] = pAlignInfo->m_sCcdSet.m_sFrontRight.m_nLed;*/

		iExposureTime[0] = pAlignInfo->m_sCcdSet.m_sFrontLeft.m_nExposureTime;
		iExposureTime[1] = pAlignInfo->m_sCcdSet.m_sFrontRight.m_nExposureTime;

		iCCDGain[0] = pAlignInfo->m_sCcdSet.m_sFrontLeft.m_nGain;
		iCCDGain[1] = pAlignInfo->m_sCcdSet.m_sFrontRight.m_nGain;
	}

	CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_CCD_LIVE, enumSeqItemID_CCD_LIVE_START, NULL);

	// Set CCD and Light
	int iCh[4] = { 3, 2, 1, 0 };

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_CHNG_LAMP, true);

	for (size_t i = 0; i < 2; i++)
	{
		m_pShareItem->GetLightControlMgrPtr()->SetLedLampIntensity(0, iCh[i], iLampIntensity[i]);
		m_pShareItem->GetAlignCameraMgr(i)->SetParam_CameraExposureTime_ms(iExposureTime[i]);
		m_pShareItem->GetAlignCameraMgr(i)->SetParam_CameraGain(iCCDGain[i]);
	}

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_CHNG_LAMP, false);

	Sleep(500);

    if (bUseMeas == true)
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

//#ifdef _USE_TEST_ALIGN
//		ptrnInfo[4].m_strName.Format(_T("Test_Mark.png"));
//#endif // _USE_TEST_ALIGN

		CPtrnInfo* pPtrnInfo = NULL;
		double dPixelx[2] = { 0, }, dPixely[2] = { 0, };

		CMaintControlMgr* pImpl = nullptr;
		pImpl = m_pShareItem->GetMaintControlMgrPtr();

		bool bAlignDone = false;

		// 이미지 패턴 매칭
        int k = 4;
		for (size_t j = 0; j < 2; j++)	// 2 Head
		{
			MaxSorce = -1;

			if (j == 0)
			{
				ptrnInfo[0].m_nThreshold = pAlignInfo->m_sCcdSet.m_sFrontLeft.m_nMatchingScore;
				ptrnInfo[0].m_strName.Format(_T("%s"), pAlignInfo->m_chPtnFrontLeft[k]);
			}
			else 
			{
				ptrnInfo[0].m_nThreshold = pAlignInfo->m_sCcdSet.m_sFrontRight.m_nMatchingScore;
				ptrnInfo[0].m_strName.Format(_T("%s"), pAlignInfo->m_chPtnFrontRight[k]);
			}

			// 이미지 패턴 매칭
			pPtrnInfo = &ptrnInfo[0];
			nRet = m_pShareItem->GetAlignCameraMgr(j)->PtrnMatch(pPtrnInfo, &Score, LENS_USE_MEAS);
			if (nRet == VTM_RV_SUCCESSED)
			{
				if (Score > MaxSorce)
				{
					iPtrn[j] = 0;
					dPixelx[j] = pPtrnInfo->m_dRealX;	// 차이값(mm)
					dPixely[j] = pPtrnInfo->m_dRealY;	// 차이값(mm)
				}
			}
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

		if (bAlignDone == true)
		{
            bRet = true;
		}
        else
        {
            bRet = false;

			m_pShareItem->SetFail(true, nRet);	// jsyoon 2022.06.27 add
        }
    }
    else
    {
	//	Sleep(500);

		for (size_t j = 0; j < 2; j++)	// 2 Head
		{
			m_pShareItem->GetAlignCameraMgr(j)->SaveCurImage(2);
		}

        bRet = true; 
    }

    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassPreMeasStep_PtrnSearch End. End"));

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS_CONTACT_SEARCH_PTRN, false);

    CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_End, NULL);

    return bRet;
}

bool CDoWorkGlassPreMeasStep_RcpChangeDone::Run()
{
	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassPreMeasStep_RcpChangeDone Start."));


	bool bMeasMode[enumMEASURE_MODE_END];
	ZeroMemory(&bMeasMode, sizeof(bMeasMode));

	S_RecipeInfo* pReciepInfo;
	pReciepInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();

	S_RecipeData* pRecipeData;
	pRecipeData = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeData();

	if (pReciepInfo->m_sInspectItem.m_bUseAgingTest_2 == TRUE) bMeasMode[enumMEASURE_MODE_AGING] = true;
	if (pReciepInfo->m_sInspectItem.m_bUseSpec_3 == TRUE)      bMeasMode[enumMEASURE_MODE_WAD] = true;
	if (pReciepInfo->m_sInspectItem.m_bUseQuality_4 == TRUE)   bMeasMode[enumMEASURE_MODE_VISION] = true;
	if (pReciepInfo->m_sInspectItem.m_bUseColor_5 == TRUE)     bMeasMode[enumMEASURE_MODE_COLOR] = true;
	if (pReciepInfo->m_sInspectItem.m_bUseAngle_6 == TRUE)     bMeasMode[enumMEASURE_MODE_ANGLE] = true;
	if (pReciepInfo->m_sInspectItem.m_bUseTSP_7 == TRUE)       bMeasMode[enumMEASURE_MODE_TSP] = true;

	if (pReciepInfo->m_sInspectItem.m_bUsePre_0 == TRUE | pReciepInfo->m_sInspectItem.m_bUseElect_1 == TRUE | pReciepInfo->m_sInspectItem.m_bUseAgingTest_2 == TRUE)
		bMeasMode[enumMEASURE_MODE_PG] = true;
	//	bMeasMode[enumMEASURE_MODE_LCR] = true;

	// Recipe Open
	int nRet = VTM_RV_SUCCESSED;
	bool bRet = true;
	DWORD dwTimeOut = 60;

	//// Change Recipe Done
	//if (nRet == VTM_RV_SUCCESSED)
	//{
	//	dwTimeOut = 60;
	//	if (bMeasMode[enumMEASURE_MODE_VISION]) nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkRecipeOpenComplate(enumTCPIpFactorytID_VISION, dwTimeOut);
	//}

	/*
	if (nRet == VTM_RV_SUCCESSED)
	{
		dwTimeOut = pReciepInfo->m_sInspectItem.m_nTimeOut[5];
		if (bMeasMode[enumMEASURE_MODE_COLOR]) nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkRecipeOpenComplate(enumTCPIpFactorytID_COLOR, dwTimeOut);
	}*/

	/*if (nRet == VTM_RV_SUCCESSED)
	{
		dwTimeOut = pReciepInfo->m_sInspectItem.m_nTimeOut[6];
		if (bMeasMode[enumMEASURE_MODE_ANGLE]) nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkRecipeOpenComplate(enumTCPIpFactorytID_ANGLE, dwTimeOut);
	}*/

	/*if (nRet == VTM_RV_SUCCESSED)
	{
		if (bMeasMode[enumMEASURE_MODE_VISION]) nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkSetParameter(enumTCPIpFactorytID_VISION);
	}*/

	if (nRet == VTM_RV_SUCCESSED)
	{
		if (bMeasMode[enumMEASURE_MODE_PG]) nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PGEvent_1);
	}

	if (nRet == VTM_RV_SUCCESSED)
	{
		if (bMeasMode[enumMEASURE_MODE_AGING]) nRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkRecipeOpenComplate(enumTCPIpFactorytID_AGING, dwTimeOut);
	}

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassPreMeasStep_RcpChangeDone End."));

	if (nRet != VTM_RV_SUCCESSED)
	{
		m_pShareItem->SetFail(true, nRet);
		bRet = false;
	}

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_CHNG_RECIPE, false);

	return bRet;
}


bool CDoWorkGlassPreMeasStep_P2P::Run()
{
	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS_PTOP, true);

    CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_P2P_Start, NULL);

    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassPreMeasStep_P2P Start."));


    bool bRet = false;
    long lRet = VTM_RV_SUCCESSED;

    bool bUseMeas = false;
    S_RecipeInfo* pReciepInfo;
    pReciepInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
    if (pReciepInfo != nullptr)
    {
        bUseMeas = pReciepInfo->m_sInspectItem.m_bUsePre_0;
    }

    if (bUseMeas == true)
    {
	//	m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4] = 0;

		S_RecipeMgr* pslData = nullptr;
		pslData = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeMgrPtr();
		int nCellCnt = 0, nStickCnt = 0;
		nCellCnt = pslData->m_nCellCnt;
		nStickCnt = pslData->m_nStickCnt;

		// jsyoon 2022.09.29 add
		int iRotate = 0;
		if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
		{
			iRotate = 1;
		}
		///

		if( iRotate == 1)
			m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4] = 0;
		else
			m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4] = nStickCnt - 1;
		///

		if (nCellCnt > 10)
		{
			// P2P
			lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_1);
			if (lRet == VTM_RV_SUCCESSED)
			{
				lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_11);
			}
		}
		else
		{
			// P2P
			lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_1);
		}

        if (lRet == VTM_RV_SUCCESSED)
        {
            bRet = true;
            if (m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
            {
                Sleep(1000);
            }
        }
        else
        {
            m_pShareItem->SetFail(true, lRet);
        }
    }
    else
    {
        bRet = true;
    }

    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassPreMeasStep_P2P End. End"));

    CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_End, NULL);

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS_PTOP, false);

    return bRet;
}

//bool CDoWorkGlassPreMeasStep_OST::Run()
//{
//	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS_OST, true);
//
//    CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_OST_Start, NULL);
//
//    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassPreMeasStep_OST Start."));
//
//    bool bRet = false;
//    long lRet = VTM_RV_SUCCESSED;
//
//    bool bUseMeas = false;
//    S_RecipeInfo* pReciepInfo;
//    pReciepInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
//    if (pReciepInfo != nullptr)
//    {
//        bUseMeas = pReciepInfo->m_sInspectItem.m_bUseElect_1;
//    }
//
//    if (bUseMeas == true)
//    {
//	//	m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4] = 0;
//
//		S_RecipeMgr* pslData = nullptr;
//		pslData = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeMgrPtr();
//	//	int nCellCnt = 0;
//	//	nCellCnt = pslData->m_nCellCnt;
//
//		int nCellCnt = 0, nStickCnt = 0;
//		nCellCnt = pslData->m_nCellCnt;
//		nStickCnt = pslData->m_nStickCnt;
//
//		// jsyoon 2022.09.29 add
//		int iRotate = 0;
//		if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
//		{
//			iRotate = 1;
//		}
//		///
//
//		if (iRotate == 1)
//			m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4] = 0;
//		else
//			m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4] = nStickCnt - 1;
//		///
//
//
//		if (nCellCnt > 10)
//		{
//			lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_2);
//			if (lRet == VTM_RV_SUCCESSED)
//			{
//				lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_12);
//			}
//		}
//		else
//		{
//			lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_2);
//		}
//        
//        if (lRet == VTM_RV_SUCCESSED)
//        {
//            bRet = true;
//            if (m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
//            {
//                Sleep(1000);
//            }
//        }
//        else
//        {
//            m_pShareItem->SetFail(true, lRet);
//        }
//    }
//    else
//    {
//        bRet = true;
//    }
//
//    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassPreMeasStep_OST End."));
//
//    CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_End, NULL);
//
//	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS_OST, false);
//
//    return bRet;
//}
//
//bool CDoWorkGlassPreMeasStep_RTEST::Run()
//{
//	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS_R_TEST, true);
//
//    CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_RTEST_Start, NULL);
//
//    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassPreMeasStep_RTEST Start."));
//
//    bool bRet = false;
//    long lRet = VTM_RV_SUCCESSED;
//
//    bool bUseMeas = false;
//    S_RecipeInfo* pReciepInfo;
//    pReciepInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
//    if (pReciepInfo != nullptr)
//    {
//       // bUseMeas = pReciepInfo->m_sInspectItem.m_bUseElect_1;
//		bUseMeas = pReciepInfo->m_sInspectItem.m_bUseRTest;		// jsyoon 2022.08.10 change
//    }
//
//    if (bUseMeas == true)
//    {
//	//	m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4] = 0;
//
//		S_RecipeMgr* pslData = nullptr;
//		pslData = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeMgrPtr();
//	//	int nCellCnt = 0;
//	//	nCellCnt = pslData->m_nCellCnt;
//
//		int nCellCnt = 0, nStickCnt = 0;
//		nCellCnt = pslData->m_nCellCnt;
//		nStickCnt = pslData->m_nStickCnt;
//
//		// jsyoon 2022.09.29 add
//		int iRotate = 0;
//		if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
//		{
//			iRotate = 1;
//		}
//		///
//
//		if (iRotate == 1)
//			m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4] = 0;
//		else
//			m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4] = nStickCnt - 1;
//		///
//
//
//		if (nCellCnt > 10)
//		{
//			lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_3);
//			if (lRet == VTM_RV_SUCCESSED)
//			{
//				lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_13);
//			}
//		}
//		else
//		{
//			lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_3);
//		}
//
//    //    lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_3);
//        if (lRet == VTM_RV_SUCCESSED)
//        {
//            bRet = true;
//            if (m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
//            {
//                Sleep(1000);
//            }
//        }
//        else
//        {
//            m_pShareItem->SetFail(true, lRet);
//        }
//    }
//    else
//    {
//        bRet = true;
//    }
//
//    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassPreMeasStep_RTEST End."));
//
//    CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_End, NULL);
//
//	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS_R_TEST, false);
//
//    return bRet;
//}

bool CDoWorkGlassPreMeasStep_Contact::Run()
{
	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS_CONTACT, true);

    CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_PROBE_CONTACT_Start, NULL);

    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassPreMeasStep_Contact Start."));

    bool bRet = false;
    long lRet = VTM_RV_SUCCESSED;

	// jsyoon 2022.05.12 add
	bool bUseMeas = false;
	S_RecipeInfo* pRecipeInfo;
	pRecipeInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
	if (pRecipeInfo != nullptr)
	{
		if (pRecipeInfo->m_sInspectItem.m_bUseElect_1 == true)	// OST
			bUseMeas = true;

		// 2023.08.31 change
	/*	if (pRecipeInfo->m_sInspectItem.m_bUsePre_0 == true)
			bUseMeas = true;

		if (pRecipeInfo->m_sInspectItem.m_bUseElect_1 == true)
			bUseMeas = true;

		if (pRecipeInfo->m_sInspectItem.m_bUseAgingTest_2 == true)
			bUseMeas = true;*/
	}

    // Probe Contact
	if (bUseMeas == true)
	{
		if (lRet == VTM_RV_SUCCESSED)
		{
			double dfMove = 0., dEMove = 0., dOD = 0.;
			dfMove = pRecipeInfo->m_sContactPos.m_fPreContactPos;
			dEMove = pRecipeInfo->m_sContactPos.m_fEndContactPos;
			dOD = pRecipeInfo->m_sContactPos.m_fAddOD;

			if (m_pShareItem->GetSystemConfigMgrPtr() && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP())  // LYS 2026-06-26 OCP: P5 only
			{
				lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeContact(AXIS_PROBE_Z5, dfMove, dEMove, dOD);
			}
			else if (pRecipeInfo->m_sInspectItem.m_bUseAgingTest_2 == true)  //  Use Aging
			{
				// 2023.05.12 change
				lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeContact(21, dfMove, dEMove, dOD);

				//// jsyoon 2022.08.01 change
				//int nStick = 0;
				//S_RecipeData* pRecipeData;
				//pRecipeData = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeData();
				//if (pRecipeData != nullptr)
				//{
				//	nStick = pRecipeData->m_vecStick.size();
				//}

				//if (nStick > 5)
				//{
				//	/*!
				//	* Probe Contact 조건 수정
				//	*/
				//	int nAgingStick = 0;
				//	for (int i = 0; i < 5; i++)
				//	{
				//		if (m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - i] != -1)
				//		{
				//			nAgingStick++;
				//		}
				//	}

				//	if (nAgingStick == 5)
				//	{
				//		lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeContact(AXIS_ALL_Z, dfMove, dEMove, dOD);  // 5개만 Contact
				//	}
				//	else
				//	{
				//		lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeContact(4, dfMove, dEMove, dOD);  // 4개만 Contact
				//	}
				//}
				//else
				//{
				//	lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeContact(21, dfMove, dEMove, dOD);

				//	//if (m_pShareItem->GetMaintControlMgrPtr()->Get_ProbeExist() == nStick)	// Stick 개수와 Probe 개수가 같은 경우
				//	//{
				//	//	switch (nStick)
				//	//	{
				//	//	case 1:
				//	//		lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeContact(AXIS_PROBE_Z5, dfMove, dEMove, dOD);
				//	//		break;
				//	//	case 2:
				//	//		lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeContact(2, dfMove, dEMove, dOD);
				//	//		break;
				//	//	case 3:
				//	//		lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeContact(3, dfMove, dEMove, dOD);
				//	//		break;
				//	//	case 4:
				//	//		lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeContact(4, dfMove, dEMove, dOD);
				//	//		break;
				//	//	case 5:
				//	//		lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeContact(AXIS_ALL_Z, dfMove, dEMove, dOD);
				//	//		break;
				//	//	default:
				//	//		lRet = df_MOTION_RV_ERROR;
				//	//		break;
				//	//	}
				//	//}
				//	//else
				//	//{
				//	//	lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeContact(21, dfMove, dEMove, dOD);	
				//	//}
				//}

			//	lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeContact(AXIS_ALL_Z, dfMove, dEMove, dOD);
			}
			else
			{
				lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeContact(AXIS_PROBE_Z5, dfMove, dEMove, dOD);
			}
		}
		// 　
		for (size_t j = 0; j < 2; j++)	// 2 Head
		{
			m_pShareItem->GetAlignCameraMgr(j)->SaveCurImage(3);
		}

		if (lRet == VTM_RV_SUCCESSED)
		{
			CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_CCD_LIVE, enumSeqItemID_CCD_LIVE_STOP, NULL);

			bRet = true;
			if (m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
			{
				Sleep(1000);
			}
		}
		else
		{
			m_pShareItem->SetFail(true, lRet);
		}
	}
	else
	{
		Sleep(500);
		bRet = true;
	}

	int iCh[4] = { 3, 2, 1, 0 };
	for (size_t i = 0; i < MAIN_CCD_MAX_NUM; i++)
	{
		m_pShareItem->GetLightControlMgrPtr()->SetLedLampIntensity(0, iCh[i], 0);
	}
   
    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassPreMeasStep_Contact End."));

	CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS_CONTACT, false);

    CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_End, NULL);

    return bRet;
}