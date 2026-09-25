#include "pch.h"
#include "DoWorkGlassAGINGStep.h"

#include "DoWorkGlassSequenceImpl.h"
#include "..\SystemParamShare.h"
#include "..\Manager\SystemConfigMgr.h"
#include "..\Manager\CommTcpIpFactoryMgr.h"
#include "..\Manager\MaintControlMgr.h"
#include "..\Manager\LightControlMgr.h"
#include "..\Manager\AlignCameraMgr.h"
#include "..\Recipe\RecipeData.h"
#include "..\Recipe\RecipeEditMgr.h"
#include "..\Manager\MCCLogMgr.h"
#include "CommTcpIpObjectPG.h"
#include "..\Manager\SequenceManager.h"
#include "..\FunctionModule\FunctionModuleEQState.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

bool CDoWorkGlassAGINGStep_None::Run()
{
    //	if(m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
    //		return true;

	// init
	m_pShareItem->SetComplete(false);
	m_pShareItem->SetAlarmOn(false);
	m_pShareItem->SetFail(false);
	m_pShareItem->SetStopUser(false);

	m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAGINGStep_None Start"));

	m_pShareItem->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_PROCESS);	// 2023.08.09 add

    CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS_AGING, true);

    return true;
}

bool CDoWorkGlassAGINGStep_Idle::Run()
{
    CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_Idle, NULL);

    // 완료
    ResetEvent(m_pShareItem->GetStartEventHandle());

    CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS_AGING, false);

    if (m_pShareItem->GetFail() != true)
    {
        m_pShareItem->SetComplete(true);   // 동작 완료
    }
    else
    {
        CMCCLogMgr::GetInstance()->SetMCC_Action(MCC_A_PROC_MEAS, true);
    }

    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAGINGStep_Idle"));

	m_pShareItem->GetShareMemoryMgrPtr()->SetSys_ProcessStatus(SPS_IDLE);	// 2023.08.09 add

	SetEvent(m_pShareItem->GetCompleteEventHandle()); // 시퀀스 완료 이벤트 SET

    return false;
}

bool CDoWorkGlassAGINGStep_Aging::Run()
{
    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAGINGStep_Aging Start."));

    bool bRet = false;
    long lRet = VTM_RV_SUCCESSED;

    bool bUseMeas = false;
    S_RecipeInfo* pReciepInfo;
    pReciepInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
    if (pReciepInfo != nullptr)
    {
        bUseMeas = pReciepInfo->m_sInspectItem.m_bUseAgingTest_2;
    }

    if (bUseMeas == true)
    {
		double dCurPos = 0.;
		dCurPos = m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_X2);	// 2023.08.31 add

        // Start
		bool bMeasAging = false;
		if (m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4] > -1)
		{
			lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_4);
			bMeasAging = true;
		}
		
		if (lRet == VTM_RV_SUCCESSED)
		{
			lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_AGING);
		}

		DWORD  dwS, dwR, dwT = 0;
		DWORD  dwSleep = 100;

		int iAgingTime = 0;
		if (bMeasAging == true)
		{
			iAgingTime = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectPG()->GetAgingTime();
			if (iAgingTime < 1)
				iAgingTime = 21;
		}
		else
		{
			iAgingTime = pReciepInfo->m_sInspectItem.m_nTimeOut[2];
		}
		
		CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_AGING_Start, iAgingTime);
		dwS = GetTickCount();
            
		// Move Snap Image
		if (lRet == VTM_RV_SUCCESSED)
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAGINGStep_Aging Snap Start."));

			S_RecipeData* pRecipeData;
			pRecipeData = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeData();

			S_RecipeInfo* pRecipeInfo;
			pRecipeInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();

			/*int iLampIntensity[2];
			ZeroMemory(&iLampIntensity, sizeof(iLampIntensity));
			if (pRecipeInfo != nullptr)
			{
				iLampIntensity[0] = pRecipeInfo->m_sLampItem.m_nBackLeft; 
				iLampIntensity[1] = pRecipeInfo->m_sLampItem.m_nBackRight;
			}*/

			int iLampIntensity[4];
			ZeroMemory(&iLampIntensity, sizeof(iLampIntensity));
			if (pRecipeInfo != nullptr)
			{
				iLampIntensity[0] = pRecipeInfo->m_sLampItem.m_nBackLeft;
				iLampIntensity[1] = pRecipeInfo->m_sLampItem.m_nBackRight;

				iLampIntensity[2] = pRecipeInfo->m_sLampItem.m_nFrontLeft;
				iLampIntensity[3] = pRecipeInfo->m_sLampItem.m_nFrontRight;
			}

			// Set Light
		//	int iCh[2] = {0, 1 };

			// Set Light
			int iCh[4] = { 0, 1, 3, 2 };

			for (size_t i = 2; i < 4; i++)
			{
				m_pShareItem->GetLightControlMgrPtr()->SetLedLampIntensity(0, iCh[i], iLampIntensity[i]); // SPO
			}

			Sleep(1000);

			for (size_t j = 0; j < 2; j++)	// 2 Head
			{
				m_pShareItem->GetAlignCameraMgr(j)->SaveCurImage(8);
			}

			for (size_t i = 0; i < 2; i++)
			{
				m_pShareItem->GetLightControlMgrPtr()->SetLedLampIntensity(0, iCh[i], iLampIntensity[i]); // SPO
			}

			int iStick = 0, nCell = 0, nCellL = 0, nCellR = 0;
			// Review Move
			double dLeft = 0., dRight = 0.;

			double dOffsetX = 0.;
			double dLeftReview = 0., dRightReview = 0.;
		
			if (pRecipeInfo != nullptr)
			{
				dOffsetX = pReciepInfo->m_sMotionOffsetPos.m_fStageY;

				dRightReview = pRecipeInfo->m_sMotionOffsetPos.m_fHeadReviewLeft;
				dLeftReview = pRecipeInfo->m_sMotionOffsetPos.m_fHeadReviewRight;
			}

			nCell = pRecipeData->m_vecStick[iStick].m_vecCell.size(); 
			
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
			dLeft += dLeftReview;
			dRight += dRightReview;

			int iProbe = 0, nProbe = 0;
			int nStick = pRecipeData->m_vecStick.size();

			// jsyoon 2022.09.29 add
			int iRotate = 0;
			if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
			{
				iRotate = 1;
			}

			//for (iStick = 0; iStick < nStick; iStick++)
			//{
			//for (iStick = 1; iStick < nStick; iStick++)

			bool iFirst = true;
			for (nProbe = 1; nProbe < 5; nProbe++)
			{
				if (m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] >= 0)
				{
					// jsyoon 2022.09.29 change
					if(iRotate == 1)
						iStick = m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe];
					else
						iStick = nStick - m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] - 1;

				//	iStick = m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe];
				}
				else
				{
					continue;
				}

				if (iStick >= pRecipeData->m_vecStick.size())
				{
					continue;
				}

				double dStageY = 0.;
				dStageY = pRecipeData->m_vecStick[iStick].m_fStageY + dOffsetX;
			//	if (dStageY > 100)
			//		continue;

				double dMinAxisX3 = m_pShareItem->GetMaintControlMgrPtr()->GetMinSWLimit(AXIS_X3);
				if (dStageY < dMinAxisX3)
				{
					continue;
				}

				int nAxis = 0;
				stMotorPosition stPosition[10];
				ZeroMemory(&stPosition, sizeof(stPosition));

				if (iFirst == true)
				{
					stPosition[0].axis = AXIS_Y3;   nAxis++;
					stPosition[0].dPosition = dLeft;
					stPosition[1].axis = AXIS_Y4;   nAxis++;
					stPosition[1].dPosition = dRight;
					stPosition[2].axis = AXIS_X3;   nAxis++;
					stPosition[2].dPosition = dStageY;
					stPosition[3].axis = AXIS_Z1;   nAxis++;
					stPosition[3].dPosition = pRecipeData->m_sHeadZ.m_fReviewZ;
					stPosition[4].axis = AXIS_Z2;   nAxis++;
					stPosition[4].dPosition = pRecipeData->m_sHeadZ.m_fReviewZ;

					lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosAging(stPosition, nAxis, SPEED_HIGH);
				}
				else
				{
					stPosition[nAxis].axis = AXIS_X3;  
					stPosition[nAxis].dPosition = dStageY;	nAxis++;
					lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosAging(stPosition, nAxis, SPEED_HIGH);
				}

				if (lRet == VTM_RV_SUCCESSED)
				{
					// Review Snap
					Sleep(1000);
					for (size_t j = 2; j < 4; j++)	// 2 Head
					{
						m_pShareItem->GetAlignCameraMgr(j)->SaveCurImage(4 + iProbe);
					}
				}
				else
				{
					break;
				}

				iProbe++;
			}

			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAGINGStep_Aging Snap End."));

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
				stPosition[4].dPosition = dCurPos; // pRecipeData->m_vecStick[0].m_fStageY + dOffsetX;
				// LYS 2026-09-07 Z1(BACK Z, OCP 미사용) -> Sub Z RIGHT/LEFT(Z3/Z4), 레시피 화상 Sub Z
				{
					S_RecipeInfo* pRcpSubZ = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
					stPosition[5].axis = AXIS_Z3;   nAxis++;
					stPosition[5].dPosition = (pRcpSubZ != nullptr) ? pRcpSubZ->m_sMotionSubZ.m_fVisionSubzRight : 0.0;
					stPosition[6].axis = AXIS_Z2;   nAxis++;
					stPosition[6].dPosition = pRecipeData->m_sHeadZ.m_fReviewZ;
					stPosition[7].axis = AXIS_Z4;   nAxis++;
					stPosition[7].dPosition = (pRcpSubZ != nullptr) ? pRcpSubZ->m_sMotionSubZ.m_fVisionSubzLeft : 0.0;
				}

				lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosReview(iStick, stPosition, nAxis, SPEED_HIGH);
			}

			for (size_t i = 0; i < 4; i++)
			{
				m_pShareItem->GetLightControlMgrPtr()->SetLedLampIntensity(0, iCh[i], 0); // SPO
			}

			// Complate
			if (lRet == VTM_RV_SUCCESSED)
			{
				// 2023.05.15 add For Virtual Test
				if (m_pShareItem->GetCommTcpIpFactoryMgr()->IsOutOfService(enumTCPIpFactorytID_PG) == true)
				{
					do
					{
						Sleep(dwSleep);

						dwR = GetTickCount();

						if (dwR >= dwS)
							dwT = dwR - dwS;
						else
							dwT = ((DWORD)0xFFFFFFFF) - (dwS - dwR);

						if (m_pShareItem->IfPauseEvt_WaitResumeEvt() != true)
						{
							break;
						}

					} while (dwT < iAgingTime * 1000);
				}
				
				UINT nTimeOut = DEFINE_AGING_WAIT_TIME;
				nTimeOut = pReciepInfo->m_sInspectItem.m_nTimeOut[2];
				if (bMeasAging == true)
				{
					if (lRet == VTM_RV_SUCCESSED)
					{
						lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectComplate(enumTCPIpFactorytID_PG, 8, nTimeOut);
					}
				}
				
				if (lRet == VTM_RV_SUCCESSED)
				{
					lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectComplate(enumTCPIpFactorytID_AGING, 0, nTimeOut);
				}
			}
			else
			{
				// Aging Stop
				m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectEnd(enumTCPIpFactorytID_AGING);

				m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectEnd(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_4);
			}

			if (lRet == VTM_RV_SUCCESSED)
			{
				// Aging EVEN
				if (nCell > 10)
				{
					CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_AGING_End, NULL);

					Sleep(500);

					CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_AGING_Start, iAgingTime);
					dwS = GetTickCount();

					m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAGINGStep_Aging Even Start."));

					lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_AGING, 1);

					if (lRet == VTM_RV_SUCCESSED)
					{
						lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_14);
					}

					if (lRet == VTM_RV_SUCCESSED)
					{
						// 2023.05.15 add For Virtual Test
						if (m_pShareItem->GetCommTcpIpFactoryMgr()->IsOutOfService(enumTCPIpFactorytID_PG) == true)
						{
							do
							{
								Sleep(dwSleep);

								dwR = GetTickCount();

								if (dwR >= dwS)
									dwT = dwR - dwS;
								else
									dwT = ((DWORD)0xFFFFFFFF) - (dwS - dwR);

							} while (dwT < iAgingTime * 1000);
						}

						UINT nTimeOut = DEFINE_AGING_WAIT_TIME;
						nTimeOut = pReciepInfo->m_sInspectItem.m_nTimeOut[2];
						if (lRet == VTM_RV_SUCCESSED)
						{
							lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectComplate(enumTCPIpFactorytID_PG, 8, nTimeOut);
						}

						if (lRet == VTM_RV_SUCCESSED)
						{
							lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectComplate(enumTCPIpFactorytID_AGING, 0, nTimeOut);
						}
					}

					m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAGINGStep_Aging Even End."));
				}
			}
		}
		else
		{
			// Aging Stop
			m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectEnd(enumTCPIpFactorytID_AGING);

			m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectEnd(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_4);
		}

		CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_AGING_End, NULL);
    }

    if (lRet == VTM_RV_SUCCESSED) 
    {
        bRet = true;
    }
	else
	{
		m_pShareItem->SetFail(true, lRet);
	}

	if (m_pShareItem->IfPauseEvt_WaitResumeEvt() != true)
	{
		bRet = false;
	}

    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAGINGStep_Aging End."));

    return bRet;
}

bool CDoWorkGlassAGINGStep_AgingDone::Run()
{
    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAGINGStep_AgingDone Start."));

    bool bRet = false;
    long lRet = VTM_RV_SUCCESSED;


    bool bUseMeas = false;
    S_RecipeInfo* pReciepInfo;
    pReciepInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
    if (pReciepInfo != nullptr)
    {
        bUseMeas = pReciepInfo->m_sInspectItem.m_bUseAgingTest_2;
    }

    if (bUseMeas == true)
    {
        if (m_pShareItem->GetSystemConfigMgrPtr()->IsUseHardwareMotor() == false)
        {
            Sleep(1000);
        }

        lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectComplate(enumTCPIpFactorytID_PG);
        if (lRet == VTM_RV_SUCCESSED)
        {
            lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectComplate(enumTCPIpFactorytID_AGING, 0, DEFINE_AGING_WAIT_TIME);
        }
    }

    if (lRet == VTM_RV_SUCCESSED)
    {
        bRet = true;
    }

    m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAGINGStep_AgingDone End."));

    CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_End, NULL);

    return bRet;
}


bool CDoWorkGlassAGINGStep_Move_Second::Run()
{
	// Stick 개수가 5개가 넘는 경우 한번더 Aging 진행을 위해 이동
	bool bRet = false;
	long lRet = VTM_RV_SUCCESSED;

	bool bUseMeas = false;
	S_RecipeInfo* pReciepInfo;
	pReciepInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
	if (pReciepInfo != nullptr)
	{
		bUseMeas = pReciepInfo->m_sInspectItem.m_bUseAgingTest_2;
	}

	S_RecipeData* pRecipeData;
	pRecipeData = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeData();
	int nMaxStick = pRecipeData->m_vecStick.size();

	if (bUseMeas == true)
	{	
		if (nMaxStick <= 5)
		{
			if ( nMaxStick == m_pShareItem->GetMaintControlMgrPtr()->Get_ProbeExist() )	// 2022.12.21 add Probe Exist Check
				bUseMeas = false;
		}
	}

	/*!
	* 2023.03.08
	* Aging 완료 되었는지 확인 후 진행
	*/
	if (bUseMeas == true)
	{
		bUseMeas = false;

		for (int iStick = 0; iStick < nMaxStick; iStick++)
		{
			if (m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[0][iStick] != df_CELLSTATE_AGING_NG &&
				m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[0][iStick] != df_CELLSTATE_AGING_OK)
			{
				bUseMeas = true;
				break;
			}
		}
	}
	
	if (bUseMeas == true)
	{
		CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_CCD_LIVE, enumSeqItemID_CCD_LIVE_START, NULL);

		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAGINGStep_Move_Second Start."));

		for (size_t i = 0; i < 5; i++)
		{
			m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[i] = -1;
		}

		// Probe Unit Unconnected
		double dfMove = 0.;
		dfMove = pReciepInfo->m_sContactPos.m_fPreContactPos;
		lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeUnContact(AXIS_ALL_Z, dfMove);

		double dProbeGap = 0., dProbeShift = 0.;
		S_RecipeMap* pRecipeMap;
		pRecipeMap = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeMap();
		dProbeGap = pRecipeMap->m_fCellDistance.y;

		double dProbeInterlockPos = 0;
		m_pShareItem->GetMaintControlMgrPtr()->GetInterlockPos(nullptr, nullptr, &dProbeInterlockPos);

		int nProbeMeasPos = 4;
		int nStick = nMaxStick - 5;

		if (dProbeGap > dProbeInterlockPos)
		{
			if (nStick > 5)	// 총 Stick 개수가 10개 보다 큰 경우 Probe 위치를 5번 Stick 부터 시작
			{
				nProbeMeasPos = 5;
			}
			else
			{
				nProbeMeasPos = 1;	// 2022.12.21 add Next
			}
		}
		else
		{
			nProbeMeasPos = 1;
		}

		// Glass 이동
		double dStageY = 0., dOffsetX = 0.;

		if (lRet == VTM_RV_SUCCESSED)
		{	
			dOffsetX = pReciepInfo->m_sMotionOffsetPos.m_fStageY;
			dStageY = pRecipeData->m_vecStick[0].m_fStageY - pRecipeData->m_vecStick[nProbeMeasPos].m_fStageY + dOffsetX;

			lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_X, dStageY, true, SPEED_HIGH);
		}

		// Probe 이동
		if (lRet == df_MOTION_RV_SUCCESSED)
		{
			dProbeShift = pReciepInfo->m_sMotionOffsetPos.m_fProbeShift;

			dProbeGap = pRecipeMap->m_fCellDistance.y;
			stMotorPosition stPositionProbeX[5];
			ZeroMemory(&stPositionProbeX, sizeof(stPositionProbeX));

			stPositionProbeX[0].axis = AXIS_PROBE_X1;
			stPositionProbeX[1].axis = AXIS_PROBE_X3;
			stPositionProbeX[2].axis = AXIS_PROBE_X5;
			stPositionProbeX[3].axis = AXIS_PROBE_X7;
			stPositionProbeX[4].axis = AXIS_PROBE_X9;


			// jsyoon 2022.09.29 add
			int iRotate = 0;
			if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
			{
				iRotate = 1;
			}

			int k = 0;

			bool bSetPos[5];
			ZeroMemory(&bSetPos, sizeof(bSetPos));

			if (nMaxStick == 5) // 2022.12.21 add 
			{
				for (int nProbe = 0; nProbe < 5; nProbe++)
				{
					if (nProbe == 0)
					{
						stPositionProbeX[4 - nProbe].dPosition = dStageY + pRecipeData->m_vecStick[nProbe + nProbeMeasPos].m_fStageY + dProbeShift + dOffsetX;

						// jsyoon 2022.09.29 change
						if (iRotate == 1)
							m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] = nProbe + nProbeMeasPos; // Meas
						else
							m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] = nMaxStick - (nProbe + nProbeMeasPos) - 1; // Meas

						// m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] = nProbe + nProbeMeasPos; // Meas
						k = nProbe + nProbeMeasPos;
					}
					else
					{
						if (k <= nMaxStick)
						{
							for (int j = k + 1; j < nMaxStick; j++)
							{
								if (pRecipeData->m_vecStick[k].m_fStageY - pRecipeData->m_vecStick[j].m_fStageY > (dProbeInterlockPos))
								{
									stPositionProbeX[4 - nProbe].dPosition = dStageY + pRecipeData->m_vecStick[j].m_fStageY + dProbeShift + dOffsetX;

									// jsyoon 2022.09.21 change
									double dMinAxisX3 = m_pShareItem->GetMaintControlMgrPtr()->GetMinSWLimit(AXIS_X3);
									if (stPositionProbeX[4 - nProbe].dPosition > dMinAxisX3)
									{
										if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsProbeExistSensorOn(4 - nProbe + df_PROBE_AGING1) == true) 	// 2022.12.21 add Probe Exist
										{
											// jsyoon 2022.09.29 change
											if (iRotate == 1)
												m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] = j;	 // Aging
											else
												m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] = nMaxStick - j - 1;	 // Aging

											//m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] = j;	 // Aging
										}

										bSetPos[4 - nProbe] = true;
									}

									k = j;
									break;

									//m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] = j; // Aging
									//k = j;
									//break;
								}
							}
						}
					}
				}
			}
			else
			{
				for (int nProbe = 0; nProbe < 5; nProbe++)
				{
					// 2023.12.22 change
					int nSetStick = 0;
					if (nMaxStick > 5)
						nSetStick = nMaxStick - 5;
					else
						nSetStick = nMaxStick;

					if (nProbe < nSetStick)
				//	if (nProbe < nMaxStick - 5)
					{
						if (nProbe == 0)
						{
							stPositionProbeX[4 - nProbe].dPosition = dStageY + pRecipeData->m_vecStick[nProbe + nProbeMeasPos].m_fStageY + dProbeShift + dOffsetX;

							// jsyoon 2022.09.29 change
							if (iRotate == 1)
								m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] = nProbe + nProbeMeasPos; // Meas
							else
								m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] = nMaxStick - (nProbe + nProbeMeasPos) - 1; // Meas

							// m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] = nProbe + nProbeMeasPos; // Meas
							k = nProbe + nProbeMeasPos;
						}
						else
						{
							if (k <= nMaxStick)
							{
								for (int j = k + 1; j < nMaxStick; j++)
								{
									if (pRecipeData->m_vecStick[k].m_fStageY - pRecipeData->m_vecStick[j].m_fStageY > (dProbeInterlockPos))
									{
										stPositionProbeX[4 - nProbe].dPosition = dStageY + pRecipeData->m_vecStick[j].m_fStageY + dProbeShift + dOffsetX;

										// jsyoon 2022.09.21 change
										double dMinAxisX3 = m_pShareItem->GetMaintControlMgrPtr()->GetMinSWLimit(AXIS_X3);
										if (stPositionProbeX[4 - nProbe].dPosition > dMinAxisX3)
										{
											if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsProbeExistSensorOn(4 - nProbe + df_PROBE_AGING1) == true) 	// 2022.12.21 add Probe Exist
											{
												// jsyoon 2022.09.29 change
												if (iRotate == 1)
													m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] = j;	 // Aging
												else
													m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] = nMaxStick - j - 1;	 // Aging

												//m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] = j;	 // Aging
											}


										}

										k = j;
										break;

										//m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] = j; // Aging
										//k = j;
										//break;
									}
								}
							}
						}
					}
					else
					{
						double fOffsetGap = 2.0;
						if (dProbeGap > dProbeInterlockPos)
						{
							stPositionProbeX[4 - nProbe].dPosition = dStageY - ((dProbeGap + fOffsetGap) * (nProbe + 1)) + dOffsetX;

							m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] = -1; // Skip
						}
						else
						{
							stPositionProbeX[4 - nProbe].dPosition = dStageY - ((dProbeInterlockPos + fOffsetGap) * (nProbe + 1)) + dOffsetX;

							m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] = -1; // Skip
						}
					}
				}
			}
			
			for (int i = 1; i < 5; i++)
			{
				if (m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - i] == -1)
				{
			//		if (bSetPos[i] != true)
					if (bSetPos[4 -i] != true)	// 2023.02.14 change
					{
						double fOffsetGap = 2.;
						stPositionProbeX[4 - i].dPosition = stPositionProbeX[5 - i].dPosition - ((dProbeInterlockPos + fOffsetGap));

						/*!
						* 2023.03.08
						* Probe1의 현재위치에서 움직일 경우 위치 확인
						*/
						if (i == 4)	// Probe 1
						{
							double dPosProbe1 = 0;
							dPosProbe1 = m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_PROBE_X1);

							if (stPositionProbeX[4 - i].dPosition > dPosProbe1)
							{
								stPositionProbeX[4 - i].dPosition = dPosProbe1;
							}
						}
					}
				}
			}

			/*for (int nProbe = 0; nProbe < 5; nProbe++) 
			{
				if (nProbe <= nMaxStick - 5)
				{
					stPositionProbeX[4 - nProbe].dPosition = dStageY + pRecipeData->m_vecStick[nProbe + nProbeMeasPos].m_fStageY + dProbeShift;
				}
				else
				{
					double fOffsetGap = 6.0;
					stPositionProbeX[4 - nProbe].dPosition = dStageY - ( (dProbeGap + fOffsetGap) * (nProbe + 1) );
				}
			}*/

			lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosMulti(stPositionProbeX, 5, SPEED_HIGH, true);
		}


		// Probe Unit Connected
		if (lRet == df_MOTION_RV_SUCCESSED)
		{
			double dfMove = 0., dEMove = 0., dOD = 0.;
			dfMove = pReciepInfo->m_sContactPos.m_fPreContactPos;
			dEMove = pReciepInfo->m_sContactPos.m_fEndContactPos;
			dOD = pReciepInfo->m_sContactPos.m_fAddOD;

			// LYS 2026-09-09 OCP(P5 강제)에서 P5 슬롯의 스틱(m_nAgingStartStick[4])에 설정된 PROBE Z CONTACT OFFSET을 End에 가산. 슬롯 미지정(-1)이면 0
			if (m_pShareItem->GetSystemConfigMgrPtr() && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP())
			{
				int iZStick = m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4];
				double dZOffset = (iZStick >= 0) ? m_pShareItem->GetRecipeEditMgrPtr()->GetStickProbeZOffset(iZStick) : 0.0;
				dEMove += dZOffset;
				CString strLog;
				strLog.Format(_T("AGING Probe Contact stick=%d offset=%.4f -> Z5 target=%.4f"), iZStick + 1, dZOffset, dEMove + dOD);
				m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);
			}
			// 2023.05.12 change
			lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeContact(21, dfMove, dEMove, dOD);	 // Probe가 있는곳만 Contact
			//if (nMaxStick == 5) // 2022.12.21 add 
			//	lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeContact(21, dfMove, dEMove, dOD);	 // Probe가 있는곳만 Contact
			//else
			//	lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeContact(4, dfMove, dEMove, dOD);	 // 4개만 Contact

			/*if(nStick < 5)
				lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeContactAging(nStick, dfMove, dEMove, dOD);
			else
				lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeContact(AXIS_ALL_Z, dfMove, dEMove, dOD);*/
		}

		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAGINGStep_Move_Second End."));
	}

	if (lRet == VTM_RV_SUCCESSED)
	{
		bRet = true;
	}
	else
	{
		m_pShareItem->SetFail(true, lRet);
	}

	if (m_pShareItem->IfPauseEvt_WaitResumeEvt() != true)
	{
		bRet = false;
	}

	return bRet;
}

bool CDoWorkGlassAGINGStep_Aging_Second::Run()
{
	bool bRet = false;
	long lRet = VTM_RV_SUCCESSED;

	bool bUseMeas = false;
	S_RecipeInfo* pReciepInfo;
	pReciepInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
	if (pReciepInfo != nullptr)
	{
		bUseMeas = pReciepInfo->m_sInspectItem.m_bUseAgingTest_2;
	}

	S_RecipeData* pRecipeData;
	pRecipeData = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeData();
	int nStick = pRecipeData->m_vecStick.size();

	if (bUseMeas == true)
	{
		if (nStick <= 5)
		{
			if (nStick == m_pShareItem->GetMaintControlMgrPtr()->Get_ProbeExist())	// 2022.12.21 add Probe Exist Check
				bUseMeas = false;
		}
	}

	/*!
	* 2023.03.08
	* Aging 완료 되었는지 확인 후 진행
	*/
	if (bUseMeas == true)
	{
		bUseMeas = false;

		for (int iStick = 0; iStick < nStick; iStick++)
		{
			if (m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[0][iStick] != df_CELLSTATE_AGING_NG &&
				m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[0][iStick] != df_CELLSTATE_AGING_OK)
			{
				bUseMeas = true;
				break;
			}
		}
	}

	if (bUseMeas == true)
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAGINGStep_Aging_Second Start."));

		bool bUseMeasProbe = true;
		int nMaxStick = 0, nProbeMeasPos = 4;
		nMaxStick = pRecipeData->m_vecStick.size();

		double dProbeGap = 0.;
		S_RecipeMap* pRecipeMap;
		pRecipeMap = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeMap();
		dProbeGap = pRecipeMap->m_fCellDistance.y;

		double dOffsetX = 0.;
		dOffsetX = pReciepInfo->m_sMotionOffsetPos.m_fStageY;

		double dProbeInterlockPos = 0;
		m_pShareItem->GetMaintControlMgrPtr()->GetInterlockPos(nullptr, nullptr, &dProbeInterlockPos);

	//	int nProbeMeasPos = 4;
		int nStick = nMaxStick - 5;

		if (dProbeGap > dProbeInterlockPos)
		{
			if (nStick > 5)	// 총 Stick 개수가 10개 보다 큰 경우 Probe 위치를 5번 Stick 부터 시작
			{
				nProbeMeasPos = 5;
			}
			else
			{
				nProbeMeasPos = 1;
			}
		}
		else
		{
			nProbeMeasPos = 1;
		}

		//if (nMaxStick > 5)	// 총 Stick 개수가 10개 보다 큰 경우 Probe 위치를 5번 Stick 부터 시작
		//{
		//	bUseMeasProbe = true;
		//	nProbeMeasPos = nMaxStick - 5;
		//}

	//	m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick = nProbeMeasPos;

		// Start
		lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_AGING, 0);

		if (lRet == VTM_RV_SUCCESSED)
		{
			if (bUseMeasProbe == true)
			{
				lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_4);
			}
		}

		DWORD  dwS, dwR, dwT = 0;
		DWORD  dwSleep = 100;

		int iAgingTime = 0;
		iAgingTime = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectPG()->GetAgingTime();
		if (iAgingTime < 1)
			iAgingTime = 21;

		CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_AGING_Start, iAgingTime);
		dwS = GetTickCount();

		// Move Snap Image
		if (lRet == VTM_RV_SUCCESSED)
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAGINGStep_Aging_Second Snap Start."));

			S_RecipeData* pRecipeData;
			pRecipeData = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeData();

			S_RecipeInfo* pRecipeInfo;
			pRecipeInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();

			int iLampIntensity[4];
			ZeroMemory(&iLampIntensity, sizeof(iLampIntensity));
			if (pRecipeInfo != nullptr)
			{
				iLampIntensity[0] = pRecipeInfo->m_sLampItem.m_nBackLeft;
				iLampIntensity[1] = pRecipeInfo->m_sLampItem.m_nBackRight;

				iLampIntensity[2] = pRecipeInfo->m_sLampItem.m_nFrontLeft;
				iLampIntensity[3] = pRecipeInfo->m_sLampItem.m_nFrontRight;
			}

			// Set Light
			int iCh[4] = { 0, 1, 3, 2 };

			for (size_t i = 2; i < 4; i++)
			{
				m_pShareItem->GetLightControlMgrPtr()->SetLedLampIntensity(0, iCh[i], iLampIntensity[i]); // SPO
			}

			Sleep(1000);

			for (size_t j = 0; j < 2; j++)	// 2 Head
			{
				m_pShareItem->GetAlignCameraMgr(j)->SaveCurImage(8);
			}

			for (size_t i = 0; i < 2; i++)
			{
				m_pShareItem->GetLightControlMgrPtr()->SetLedLampIntensity(0, iCh[i], iLampIntensity[i]); // SPO
			}

			int iStick = 0, nCell = 0, nCellL = 0, nCellR = 0;
			// Review Move
			double dLeft = 0., dRight = 0.;

			double dLeftReview = 0., dRightReview = 0.;
			if (pRecipeInfo != nullptr)
			{
				dRightReview = pRecipeInfo->m_sMotionOffsetPos.m_fHeadReviewLeft; 
				dLeftReview = pRecipeInfo->m_sMotionOffsetPos.m_fHeadReviewRight;
			}

			nCell = pRecipeData->m_vecStick[iStick].m_vecCell.size();
			if (nCell > 1)
			{
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
			}

			// jsyoon 2022.05.12 add
			dLeft += dLeftReview;
			dRight += dRightReview;

			int iProbe = 0, nProbe = 0;
			int nStick = pRecipeData->m_vecStick.size();
		//	for (iStick = 0; iStick < nStick; iStick++)

			// jsyoon 2022.09.29 add
			int iRotate = 0;
			if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
			{
				iRotate = 1;
			}
		
		//	for (iStick = 5; iStick < nStick; iStick++)
			for (nProbe = 1; nProbe < 5; nProbe++)
			{
				if (m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] >= 0)
				{
					// jsyoon 2022.09.29 change
					if (iRotate == 1)
						iStick = m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe];
					else
						iStick = nStick - m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] - 1;

					//	iStick = m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe];
				}
				else
				{
					continue;
				}

				if (iStick >= pRecipeData->m_vecStick.size())
				{
					continue;
				}

				double dStageY = 0.;
				dStageY = pRecipeData->m_vecStick[0].m_fStageY - pRecipeData->m_vecStick[nProbeMeasPos].m_fStageY + pRecipeData->m_vecStick[iStick].m_fStageY + dOffsetX;
			//	dStageY = pRecipeData->m_vecStick[iStick].m_fStageY;
			//	if (dStageY > 100)
			//		continue;

				double dMinAxisX3 = m_pShareItem->GetMaintControlMgrPtr()->GetMinSWLimit(AXIS_X3);
				if (dStageY < dMinAxisX3)
				{
					continue;
				}

				int nAxis = 0;
				stMotorPosition stPosition[10];
				ZeroMemory(&stPosition, sizeof(stPosition));
				stPosition[0].axis = AXIS_Y3;   nAxis++;
				stPosition[0].dPosition = dLeft;
				stPosition[1].axis = AXIS_Y4;   nAxis++;
				stPosition[1].dPosition = dRight;
				stPosition[2].axis = AXIS_X3;   nAxis++;
				stPosition[2].dPosition = dStageY;
				stPosition[3].axis = AXIS_Z1;   nAxis++;
				stPosition[3].dPosition = pRecipeData->m_sHeadZ.m_fReviewZ;
				stPosition[4].axis = AXIS_Z2;   nAxis++;
				stPosition[4].dPosition = pRecipeData->m_sHeadZ.m_fReviewZ;

				lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosAging(stPosition, nAxis, SPEED_HIGH);
				if (lRet == VTM_RV_SUCCESSED)
				{
					// Review Snap
					Sleep(1000);
					for (size_t j = 2; j < 4; j++)	// 2 Head
					{
						m_pShareItem->GetAlignCameraMgr(j)->SaveCurImage(4 + iProbe);
					}
				}
				else
				{
					break;
				}

				iProbe++;
			}

			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAGINGStep_Aging_Second Snap End."));

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
				stPosition[4].dPosition = pRecipeData->m_vecStick[0].m_fStageY + dOffsetX;
				// LYS 2026-09-07 Z1(BACK Z, OCP 미사용) -> Sub Z RIGHT/LEFT(Z3/Z4), 레시피 화상 Sub Z
				{
					S_RecipeInfo* pRcpSubZ = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
					stPosition[5].axis = AXIS_Z3;   nAxis++;
					stPosition[5].dPosition = (pRcpSubZ != nullptr) ? pRcpSubZ->m_sMotionSubZ.m_fVisionSubzRight : 0.0;
					stPosition[6].axis = AXIS_Z2;   nAxis++;
					stPosition[6].dPosition = pRecipeData->m_sHeadZ.m_fReviewZ;
					stPosition[7].axis = AXIS_Z4;   nAxis++;
					stPosition[7].dPosition = (pRcpSubZ != nullptr) ? pRcpSubZ->m_sMotionSubZ.m_fVisionSubzLeft : 0.0;
				}

				lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosReview(iStick, stPosition, nAxis, SPEED_HIGH);
			}

			for (size_t i = 0; i < 4; i++)
			{
				m_pShareItem->GetLightControlMgrPtr()->SetLedLampIntensity(0, iCh[i], 0); // SPO
			}

			// Complate
			if (lRet == VTM_RV_SUCCESSED)
			{
				// 2023.05.15 add For Virtual Test
				if (m_pShareItem->GetCommTcpIpFactoryMgr()->IsOutOfService(enumTCPIpFactorytID_PG) == true)
				{
					do
					{
						Sleep(dwSleep);

						dwR = GetTickCount();

						if (dwR >= dwS)
							dwT = dwR - dwS;
						else
							dwT = ((DWORD)0xFFFFFFFF) - (dwS - dwR);

					} while (dwT < iAgingTime * 1000);
				}

				UINT nTimeOut = DEFINE_AGING_WAIT_TIME;
				nTimeOut = pReciepInfo->m_sInspectItem.m_nTimeOut[2];
				if (lRet == VTM_RV_SUCCESSED)
				{
					if (bUseMeasProbe == true)
					{
						lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectComplate(enumTCPIpFactorytID_PG, 8, nTimeOut);
					}
				}

				if (lRet == VTM_RV_SUCCESSED)
				{
					lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectComplate(enumTCPIpFactorytID_AGING, 0, nTimeOut);
				}
			}
			else
			{
				// Aging Stop
				m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectEnd(enumTCPIpFactorytID_AGING);

				if (bUseMeasProbe == true)
				{
					m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectEnd(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_4);
				}	
			}

			if (lRet == VTM_RV_SUCCESSED)
			{
				// Aging EVEN
				if (nCell > 10)
				{
					CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_AGING_End, NULL);

					Sleep(500);

					CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_AGING_Start, iAgingTime);
					dwS = GetTickCount();

					m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAGINGStep_Aging_Second Even Start."));

					lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_AGING, 1);

					if (lRet == VTM_RV_SUCCESSED)
					{
						lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_14);
					}

					if (lRet == VTM_RV_SUCCESSED)
					{
						// 2023.05.15 add For Virtual Test
						if (m_pShareItem->GetCommTcpIpFactoryMgr()->IsOutOfService(enumTCPIpFactorytID_PG) == true)
						{
							do
							{
								Sleep(dwSleep);

								dwR = GetTickCount();

								if (dwR >= dwS)
									dwT = dwR - dwS;
								else
									dwT = ((DWORD)0xFFFFFFFF) - (dwS - dwR);

							} while (dwT < iAgingTime * 1000);
						}

						UINT nTimeOut = DEFINE_AGING_WAIT_TIME;
						nTimeOut = pReciepInfo->m_sInspectItem.m_nTimeOut[2];
						if (lRet == VTM_RV_SUCCESSED)
						{
							lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectComplate(enumTCPIpFactorytID_PG, 8, nTimeOut);
						}

						if (lRet == VTM_RV_SUCCESSED)
						{
							lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectComplate(enumTCPIpFactorytID_AGING, 0, nTimeOut);
						}
					}

					m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAGINGStep_Aging_Second Even End."));
				}
			}
		}
		else
		{
			// Aging Stop
			m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectEnd(enumTCPIpFactorytID_AGING);

			if (bUseMeasProbe == true)
			{
				m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectEnd(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_4);
			}
		}

		CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_AGING_End, NULL);

		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAGINGStep_Aging_Second End"));
	}

	if (lRet == VTM_RV_SUCCESSED)
	{
		bRet = true;
	}

	if (m_pShareItem->IfPauseEvt_WaitResumeEvt() != true)
	{
		bRet = false;
	}

	return bRet;
}

bool CDoWorkGlassAGINGStep_Move_3::Run()
{
	// Stick 개수가 5개가 넘는 경우 한번더 Aging 진행을 위해 이동
	bool bRet = false;
	long lRet = VTM_RV_SUCCESSED;

	bool bUseMeas = false;
	S_RecipeInfo* pReciepInfo;
	pReciepInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
	if (pReciepInfo != nullptr)
	{
		bUseMeas = pReciepInfo->m_sInspectItem.m_bUseAgingTest_2;
	}

	S_RecipeData* pRecipeData;
	pRecipeData = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeData();
	int nMaxStick = pRecipeData->m_vecStick.size();

	if (bUseMeas == true)
	{
		if (nMaxStick <= 5)
		{
			if (nMaxStick == m_pShareItem->GetMaintControlMgrPtr()->Get_ProbeExist())	// 2022.12.21 add Probe Exist Check
				bUseMeas = false;
		}
	}

	/*!
	* 2023.03.08 
	* Aging 완료 되었는지 확인 후 진행
	*/
	if (bUseMeas == true)
	{
		bUseMeas = false;

		for (int iStick = 0; iStick < nMaxStick; iStick++)
		{
			if (m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[0][iStick] != df_CELLSTATE_AGING_NG &&
				m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[0][iStick] != df_CELLSTATE_AGING_OK)
			{
				bUseMeas = true;
				break;
			}
		}
	}

	if (bUseMeas == true)
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAGINGStep_Move_3 Start."));

		for (size_t i = 0; i < 5; i++)
		{
			m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[i] = -1;
		}

		// Probe Unit Unconnected
		double dfMove = 0.;
		dfMove = pReciepInfo->m_sContactPos.m_fPreContactPos;
		lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeUnContact(AXIS_ALL_Z, dfMove);

		double dProbeGap = 0., dProbeShift = 0.;
		S_RecipeMap* pRecipeMap;
		pRecipeMap = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeMap();
		dProbeGap = pRecipeMap->m_fCellDistance.y;

		double dProbeInterlockPos = 0;
		m_pShareItem->GetMaintControlMgrPtr()->GetInterlockPos(nullptr, nullptr, &dProbeInterlockPos);

		int nProbeMeasPos = 4;
		int nStick = nMaxStick - 5;

		if (dProbeGap > dProbeInterlockPos)
		{
			if (nStick > 5)	// 총 Stick 개수가 10개 보다 큰 경우 Probe 위치를 5번 Stick 부터 시작
			{
				nProbeMeasPos = 5;
			}
			else
			{
				nProbeMeasPos = 2;	// 2022.12.21 add Next
			}
		}
		else
		{
			nProbeMeasPos = 2;
		}

		// Glass 이동
		double dStageY = 0., dOffsetX = 0.;

		if (lRet == VTM_RV_SUCCESSED)
		{
			dOffsetX = pReciepInfo->m_sMotionOffsetPos.m_fStageY;
			dStageY = pRecipeData->m_vecStick[0].m_fStageY - pRecipeData->m_vecStick[nProbeMeasPos].m_fStageY + dOffsetX;

			lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosA(AXIS_X, dStageY, true, SPEED_HIGH);
		}

		// Probe 이동
		if (lRet == df_MOTION_RV_SUCCESSED)
		{
			dProbeShift = pReciepInfo->m_sMotionOffsetPos.m_fProbeShift;

			dProbeGap = pRecipeMap->m_fCellDistance.y;
			stMotorPosition stPositionProbeX[5];
			ZeroMemory(&stPositionProbeX, sizeof(stPositionProbeX));

			stPositionProbeX[0].axis = AXIS_PROBE_X1;
			stPositionProbeX[1].axis = AXIS_PROBE_X3;
			stPositionProbeX[2].axis = AXIS_PROBE_X5;
			stPositionProbeX[3].axis = AXIS_PROBE_X7;
			stPositionProbeX[4].axis = AXIS_PROBE_X9;

			// jsyoon 2022.09.29 add
			int iRotate = 0;
			if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
			{
				iRotate = 1;
			}

			int k = 0;

			bool bSetPos[5];
			ZeroMemory(&bSetPos, sizeof(bSetPos));

			if (nMaxStick == 5) // 2022.12.21 add 
			{
				// Get Current Position
				for (int i = 0; i < 5; i++)	
				{
					stPositionProbeX[i].dPosition = m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(stPositionProbeX[i].axis);
				}

				for (int nProbe = 0; nProbe < 5; nProbe++)
				{
					if (nProbe == 0)
					{
						stPositionProbeX[4 - nProbe].dPosition = dStageY + pRecipeData->m_vecStick[nProbe + nProbeMeasPos].m_fStageY + dProbeShift + dOffsetX;

						int iS = 0;
						if (iRotate == 1)
						{
							iS = nProbe + nProbeMeasPos;
						}
						else
						{
							iS = nMaxStick - (nProbe + nProbeMeasPos) - 1;
						}

						if (m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[0][iS] != df_CELLSTATE_AGING_NG &&
							m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[0][iS] != df_CELLSTATE_AGING_OK)
						{

							m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] = iS;	 // Meas
						}

						k = nProbe + nProbeMeasPos;
					}
					else
					{
						if (k <= nMaxStick)
						{
							for (int j = k + 1; j < nMaxStick; j++)
							{
								if (pRecipeData->m_vecStick[k].m_fStageY - pRecipeData->m_vecStick[j].m_fStageY > (dProbeInterlockPos))
								{
									stPositionProbeX[4 - nProbe].dPosition = dStageY + pRecipeData->m_vecStick[j].m_fStageY + dProbeShift + dOffsetX;

									// jsyoon 2022.09.21 change
									double dMinAxisX3 = m_pShareItem->GetMaintControlMgrPtr()->GetMinSWLimit(AXIS_X3);
									if (stPositionProbeX[4 - nProbe].dPosition > dMinAxisX3)
									{
										if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsProbeExistSensorOn(4 - nProbe + df_PROBE_AGING1) == true) 	// 2022.12.21 add Probe Exist
										{
											// jsyoon 2022.09.29 change
											int iS = 0;
											if (iRotate == 1)
											{
												iS = j;
											}
											else
											{
												iS = nMaxStick - j - 1;
											}

											if (m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[0][iS] != df_CELLSTATE_AGING_NG &&
												m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[0][iS] != df_CELLSTATE_AGING_OK)
											{

												m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] = iS;	 // Aging
											}
										}

										bSetPos[4 - nProbe] = true;
									}

									k = j;
									break;
								}
							}
						}
					}
				}
			}
			else
			{
				for (int nProbe = 0; nProbe < 5; nProbe++)
				{
					// 2023.12.22 change
					int nSetStick = 0;
					if (nMaxStick > 5)
						nSetStick = nMaxStick - 5;
					else
						nSetStick = nMaxStick;

					if (nProbe < nSetStick)
				//	if (nProbe < nMaxStick - 5)
					{
						if (nProbe == 0)
						{
							stPositionProbeX[4 - nProbe].dPosition = dStageY + pRecipeData->m_vecStick[nProbe + nProbeMeasPos].m_fStageY + dProbeShift + dOffsetX;

							int iS = 0;
							if (iRotate == 1)
							{
								iS = nProbe + nProbeMeasPos;
							}
							else
							{
								iS = nMaxStick - (nProbe + nProbeMeasPos) - 1;
							}

							if (m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[0][iS] != df_CELLSTATE_AGING_NG &&
								m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[0][iS] != df_CELLSTATE_AGING_OK)
							{

								m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] = iS;	 // Meas
							}

							//if (m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[0][nProbe + nProbeMeasPos] != df_CELLSTATE_AGING_NG &&
							//	m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[0][nProbe + nProbeMeasPos] != df_CELLSTATE_AGING_OK )
							//{
							//	// jsyoon 2022.09.29 change
							//	if(iRotate == 1)
							//		m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] = nProbe + nProbeMeasPos; // Meas
							//	else
							//		m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] = nMaxStick - (nProbe + nProbeMeasPos) - 1 ; // Meas

							//	// m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] = nProbe + nProbeMeasPos; // Meas
							//}

							k = nProbe + nProbeMeasPos;
						}
						else
						{
							if (k <= nMaxStick)
							{
								for (int j = k + 1; j < nMaxStick; j++)
								{
									if (pRecipeData->m_vecStick[k].m_fStageY - pRecipeData->m_vecStick[j].m_fStageY > (dProbeInterlockPos))
									{

										//stPositionProbeX[4 - nProbe].dPosition = dStageY + pRecipeData->m_vecStick[j].m_fStageY + dProbeShift + dOffsetX;
										//m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] = j; // Aging
										//k = j;
										//break; 

										stPositionProbeX[4 - nProbe].dPosition = dStageY + pRecipeData->m_vecStick[j].m_fStageY + dProbeShift + dOffsetX;

										// jsyoon 2022.09.21 change
										double dMinAxisX3 = m_pShareItem->GetMaintControlMgrPtr()->GetMinSWLimit(AXIS_X3);
										if (stPositionProbeX[4 - nProbe].dPosition > dMinAxisX3)
										{
											if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsProbeExistSensorOn(4 - nProbe + df_PROBE_AGING1) == true) 	// 2022.12.21 add Probe Exist
											{
												// jsyoon 2022.09.29 change
												int iS = 0;
												if (iRotate == 1)
												{
													iS = j;
												}
												else
												{
													iS = nMaxStick - j - 1;
												}

												if (m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[0][iS] != df_CELLSTATE_AGING_NG &&
													m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[0][iS] != df_CELLSTATE_AGING_OK)
												{

													m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] = iS;	 // Aging
												}
											}


											//if (m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[0][j] != df_CELLSTATE_AGING_NG &&
											//	m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[0][j] != df_CELLSTATE_AGING_OK)
											//{

											//	m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] = j;	 // Aging
											//}
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
						double fOffsetGap = 2.0;
						if (dProbeGap > dProbeInterlockPos)
						{
							stPositionProbeX[4 - nProbe].dPosition = dStageY - ((dProbeGap + fOffsetGap) * (nProbe + 1)) + dOffsetX;

							m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] = -1; // Skip
						}
						else
						{
							stPositionProbeX[4 - nProbe].dPosition = dStageY - ((dProbeInterlockPos + fOffsetGap) * (nProbe + 1)) + dOffsetX;

							m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] = -1; // Skip
						}
					}
				}
			}

			/*for (int nProbe = 0; nProbe < 5; nProbe++)
			{
				if (nProbe <= nMaxStick - 5)
				{
					stPositionProbeX[4 - nProbe].dPosition = dStageY + pRecipeData->m_vecStick[nProbe + nProbeMeasPos].m_fStageY + dProbeShift;
				}
				else
				{
					double fOffsetGap = 6.0;
					stPositionProbeX[4 - nProbe].dPosition = dStageY - ( (dProbeGap + fOffsetGap) * (nProbe + 1) );
				}
			}*/

			/*!
			* 2023.03.08
			* Aging 하지안는 Probe 중
			* Probe1의 현재위치에서 움직일 경우 위치 확인
			*/
			//for (int i = 1; i < 5; i++)
			//{
			//	if (m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - i] == -1)
			//	{
			//		if (i == 4)	// Probe 1
			//		{
			//			double dPosProbe1 = 0;
			//			dPosProbe1 = m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_PROBE_X1);

			//			if (stPositionProbeX[4 - i].dPosition > dPosProbe1)
			//			{
			//				stPositionProbeX[4 - i].dPosition = dPosProbe1;
			//			}
			//		}
			//	}
			//}

			// 2023.12.22 change
			for (int i = 1; i < 5; i++)
			{
				if (m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - i] == -1)
				{
					if (bSetPos[4 - i] != true)	
					{
						double fOffsetGap = 2.;
						stPositionProbeX[4 - i].dPosition = stPositionProbeX[5 - i].dPosition - ((dProbeInterlockPos + fOffsetGap));

						if (i == 4)	// Probe 1
						{
							double dPosProbe1 = 0;
							dPosProbe1 = m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_PROBE_X1);

							if (stPositionProbeX[4 - i].dPosition > dPosProbe1)
							{
								stPositionProbeX[4 - i].dPosition = dPosProbe1;
							}
						}
					}
				}
			}

			lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosMulti(stPositionProbeX, 5, SPEED_HIGH, true);
		}


		// Probe Unit Connected
		if (lRet == df_MOTION_RV_SUCCESSED)
		{
			double dfMove = 0., dEMove = 0., dOD = 0.;
			dfMove = pReciepInfo->m_sContactPos.m_fPreContactPos;
			dEMove = pReciepInfo->m_sContactPos.m_fEndContactPos;
			dOD = pReciepInfo->m_sContactPos.m_fAddOD;

			// LYS 2026-09-09 OCP(P5 강제)에서 P5 슬롯의 스틱(m_nAgingStartStick[4])에 설정된 PROBE Z CONTACT OFFSET을 End에 가산. 슬롯 미지정(-1)이면 0
			if (m_pShareItem->GetSystemConfigMgrPtr() && m_pShareItem->GetSystemConfigMgrPtr()->IsOCP())
			{
				int iZStick = m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4];
				double dZOffset = (iZStick >= 0) ? m_pShareItem->GetRecipeEditMgrPtr()->GetStickProbeZOffset(iZStick) : 0.0;
				dEMove += dZOffset;
				CString strLog;
				strLog.Format(_T("AGING Probe Contact stick=%d offset=%.4f -> Z5 target=%.4f"), iZStick + 1, dZOffset, dEMove + dOD);
				m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, strLog);
			}
			// 2023.05.12 change
			lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeContact(21, dfMove, dEMove, dOD);	 // Probe가 있는곳만 Contact
			//if (nMaxStick == 5) // 2022.12.21 add 
			//	lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeContact(AXIS_PROBE_Z3, dfMove, dEMove, dOD);  // 1개만 Contact
			//else
			//	lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeContact(AXIS_PROBE_Z2, dfMove, dEMove, dOD);  // 1개만 Contact

		//	if (nStick < 5)
		//		lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeContactAging(nStick, dfMove, dEMove, dOD);
		//	else
		//		lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeContact(AXIS_ALL_Z, dfMove, dEMove, dOD);
		}

		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAGINGStep_Move_3 End."));
	}

	if (lRet == VTM_RV_SUCCESSED)
	{
		bRet = true;
	}
	else
	{
		m_pShareItem->SetFail(true, lRet);
	}

	if (m_pShareItem->IfPauseEvt_WaitResumeEvt() != true)
	{
		bRet = false;
	}

	return bRet;
}

bool CDoWorkGlassAGINGStep_Aging_3::Run()
{
	bool bRet = false;
	long lRet = VTM_RV_SUCCESSED;

	bool bUseMeas = false;
	S_RecipeInfo* pReciepInfo;
	pReciepInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
	if (pReciepInfo != nullptr)
	{
		bUseMeas = pReciepInfo->m_sInspectItem.m_bUseAgingTest_2;
	}

	S_RecipeData* pRecipeData;
	pRecipeData = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeData();
	int nStick = pRecipeData->m_vecStick.size();

	if (bUseMeas == true)
	{
		if (nStick <= 5)
		{
			if (nStick == m_pShareItem->GetMaintControlMgrPtr()->Get_ProbeExist())	// 2022.12.21 add Probe Exist Check
				bUseMeas = false;
		}
	}

	/*!
	* 2023.03.08 
	* Aging 완료 되었는지 확인 후 진행
	*/
	if (bUseMeas == true)
	{
		bUseMeas = false;

		for (int iStick = 0; iStick < nStick; iStick++)
		{
			if (m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[0][iStick] != df_CELLSTATE_AGING_NG &&
				m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->nCellStatus[0][iStick] != df_CELLSTATE_AGING_OK)
			{
				bUseMeas = true;
				break;
			}
		}
	}

	if (bUseMeas == true)
	{
		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAGINGStep_Aging_3 Start."));

		bool bUseMeasProbe = false;
		int nMaxStick = 0, nProbeMeasPos = 4;
		nMaxStick = pRecipeData->m_vecStick.size();

		double dProbeGap = 0.;
		S_RecipeMap* pRecipeMap;
		pRecipeMap = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeMap();
		dProbeGap = pRecipeMap->m_fCellDistance.y;

		double dOffsetX = 0.;
		dOffsetX = pReciepInfo->m_sMotionOffsetPos.m_fStageY;

		double dProbeInterlockPos = 0;
		m_pShareItem->GetMaintControlMgrPtr()->GetInterlockPos(nullptr, nullptr, &dProbeInterlockPos);

		//	int nProbeMeasPos = 4;
		int nStick = nMaxStick - 5;

		if (dProbeGap > dProbeInterlockPos)
		{
			if (nStick > 5)	// 총 Stick 개수가 10개 보다 큰 경우 Probe 위치를 5번 Stick 부터 시작
			{
				nProbeMeasPos = 5;
			}
			else
			{
				nProbeMeasPos = 2;
			}
		}
		else
		{
			nProbeMeasPos = 2;
		}

		// 2023.12.22 add
		if (m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4] > -1)
		{
			bUseMeasProbe = true;
		}
			

		//if (nMaxStick > 5)	// 총 Stick 개수가 10개 보다 큰 경우 Probe 위치를 5번 Stick 부터 시작
		//{
		//	bUseMeasProbe = true;
		//	nProbeMeasPos = nMaxStick - 5;
		//}

	//	m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick = nProbeMeasPos;

		// Start
		lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_AGING, 0);

		if (lRet == VTM_RV_SUCCESSED)
		{
			if (bUseMeasProbe == true)
			{
				lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_4);
			}
		}

		DWORD  dwS, dwR, dwT = 0;
		DWORD  dwSleep = 100;

		int iAgingTime = 0;
		iAgingTime = m_pShareItem->GetCommTcpIpFactoryMgr()->GetCommTcpIpObjectPG()->GetAgingTime();
		if (iAgingTime < 1)
			iAgingTime = 21;

		CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_AGING_Start, iAgingTime);
		dwS = GetTickCount();

		// Move Snap Image
		if (lRet == VTM_RV_SUCCESSED)
		{
			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAGINGStep_Aging_3 Snap Start."));

			S_RecipeData* pRecipeData;
			pRecipeData = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeData();

			S_RecipeInfo* pRecipeInfo;
			pRecipeInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();

			int iLampIntensity[2];
			ZeroMemory(&iLampIntensity, sizeof(iLampIntensity));
			if (pRecipeInfo != nullptr)
			{
				iLampIntensity[0] = pRecipeInfo->m_sLampItem.m_nBackLeft;
				iLampIntensity[1] = pRecipeInfo->m_sLampItem.m_nBackRight;
			}

			// Set Light
			int iCh[2] = { 0, 1 };

			for (size_t i = 0; i < 2; i++)
			{
				m_pShareItem->GetLightControlMgrPtr()->SetLedLampIntensity(0, iCh[i], iLampIntensity[i]); // SPO
			}

			int iStick = 0, nCell = 0, nCellL = 0, nCellR = 0;
			// Review Move
			double dLeft = 0., dRight = 0.;

			double dLeftReview = 0., dRightReview = 0.;
			if (pRecipeInfo != nullptr)
			{
				dRightReview = pRecipeInfo->m_sMotionOffsetPos.m_fHeadReviewLeft;
				dLeftReview = pRecipeInfo->m_sMotionOffsetPos.m_fHeadReviewRight;
			}

			nCell = pRecipeData->m_vecStick[iStick].m_vecCell.size();
			if (nCell > 1)
			{
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
			}

			// jsyoon 2022.05.12 add
			dLeft += dLeftReview;
			dRight += dRightReview;

			int iProbe = 0, nProbe = 0;
			int nStick = pRecipeData->m_vecStick.size();

			// jsyoon 2022.09.29 add
			int iRotate = 0;
			if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
			{
				iRotate = 1;
			}

			//	for (iStick = 0; iStick < nStick; iStick++)

			//	for (iStick = 5; iStick < nStick; iStick++)
			for (nProbe = 1; nProbe < 5; nProbe++)
			{
				if (m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] >= 0)
				{
					// jsyoon 2022.09.29 change
					if (iRotate == 1)
						iStick = m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe];
					else
						iStick = nStick - m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] - 1;

					//iStick = m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe];
				}
				else
				{
					continue;
				}

				if (iStick >= pRecipeData->m_vecStick.size())
				{
					continue;
				}

				double dStageY = 0.;
				dStageY = pRecipeData->m_vecStick[0].m_fStageY - pRecipeData->m_vecStick[nProbeMeasPos].m_fStageY + pRecipeData->m_vecStick[iStick].m_fStageY + dOffsetX;
				//	dStageY = pRecipeData->m_vecStick[iStick].m_fStageY;
				//	if (dStageY > 100)
				//		continue;

				double dMinAxisX3 = m_pShareItem->GetMaintControlMgrPtr()->GetMinSWLimit(AXIS_X3);
				if (dStageY < dMinAxisX3)
				{
					continue;
				}

				int nAxis = 0;
				stMotorPosition stPosition[10];
				ZeroMemory(&stPosition, sizeof(stPosition));
				stPosition[0].axis = AXIS_Y3;   nAxis++;
				stPosition[0].dPosition = dLeft;
				stPosition[1].axis = AXIS_Y4;   nAxis++;
				stPosition[1].dPosition = dRight;
				stPosition[2].axis = AXIS_X3;   nAxis++;
				stPosition[2].dPosition = dStageY;
				stPosition[3].axis = AXIS_Z1;   nAxis++;
				stPosition[3].dPosition = pRecipeData->m_sHeadZ.m_fReviewZ;
				stPosition[4].axis = AXIS_Z2;   nAxis++;
				stPosition[4].dPosition = pRecipeData->m_sHeadZ.m_fReviewZ;

				lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosAging(stPosition, nAxis, SPEED_HIGH);
				if (lRet == VTM_RV_SUCCESSED)
				{
					// Review Snap
					Sleep(1000);
					for (size_t j = 2; j < 4; j++)	// 2 Head
					{
						m_pShareItem->GetAlignCameraMgr(j)->SaveCurImage(4 + iProbe);
					}
				}
				else
				{
					break;
				}

				iProbe++;
			}

			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAGINGStep_Aging_3 Snap End."));

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
				stPosition[4].dPosition = pRecipeData->m_vecStick[0].m_fStageY + dOffsetX;
				// LYS 2026-09-07 Z1(BACK Z, OCP 미사용) -> Sub Z RIGHT/LEFT(Z3/Z4), 레시피 화상 Sub Z
				{
					S_RecipeInfo* pRcpSubZ = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
					stPosition[5].axis = AXIS_Z3;   nAxis++;
					stPosition[5].dPosition = (pRcpSubZ != nullptr) ? pRcpSubZ->m_sMotionSubZ.m_fVisionSubzRight : 0.0;
					stPosition[6].axis = AXIS_Z2;   nAxis++;
					stPosition[6].dPosition = pRecipeData->m_sHeadZ.m_fReviewZ;
					stPosition[7].axis = AXIS_Z4;   nAxis++;
					stPosition[7].dPosition = (pRcpSubZ != nullptr) ? pRcpSubZ->m_sMotionSubZ.m_fVisionSubzLeft : 0.0;
				}

				lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosReview(iStick, stPosition, nAxis, SPEED_HIGH);
			}

			for (size_t i = 0; i < 2; i++)
			{
				m_pShareItem->GetLightControlMgrPtr()->SetLedLampIntensity(0, iCh[i], 0); // SPO
			}

			// Complate
			if (lRet == VTM_RV_SUCCESSED)
			{
				// 2023.05.15 add For Virtual Test
				if (m_pShareItem->GetCommTcpIpFactoryMgr()->IsOutOfService(enumTCPIpFactorytID_PG) == true)
				{
					do
					{
						Sleep(dwSleep);

						dwR = GetTickCount();

						if (dwR >= dwS)
							dwT = dwR - dwS;
						else
							dwT = ((DWORD)0xFFFFFFFF) - (dwS - dwR);

					} while (dwT < iAgingTime * 1000);
				}

				UINT nTimeOut = DEFINE_AGING_WAIT_TIME;
				nTimeOut = pReciepInfo->m_sInspectItem.m_nTimeOut[2];
				if (lRet == VTM_RV_SUCCESSED)
				{
					if (bUseMeasProbe == true)
					{
						lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectComplate(enumTCPIpFactorytID_PG, 8, nTimeOut);
					}
				}

				if (lRet == VTM_RV_SUCCESSED)
				{
					lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectComplate(enumTCPIpFactorytID_AGING, 0, nTimeOut);
				}
			}
			else
			{
				// Aging Stop
				m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectEnd(enumTCPIpFactorytID_AGING);

				if (bUseMeasProbe == true)
				{
					m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectEnd(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_4);
				}
			}

			if (lRet == VTM_RV_SUCCESSED)
			{
				// Aging EVEN
				if (nCell > 10)
				{
					CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_AGING_End, NULL);

					Sleep(500);

					CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_AGING_Start, iAgingTime);
					dwS = GetTickCount();

					m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAGINGStep_Aging_3 Even Start."));

					lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_AGING, 1);

					if (lRet == VTM_RV_SUCCESSED)
					{
						if (bUseMeasProbe == true)
						{
							lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectStart(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_14);
						}
						
					}

					if (lRet == VTM_RV_SUCCESSED)
					{
						// 2023.05.15 add For Virtual Test
						if (m_pShareItem->GetCommTcpIpFactoryMgr()->IsOutOfService(enumTCPIpFactorytID_PG) == true)
						{
							do
							{
								Sleep(dwSleep);

								dwR = GetTickCount();

								if (dwR >= dwS)
									dwT = dwR - dwS;
								else
									dwT = ((DWORD)0xFFFFFFFF) - (dwS - dwR);

							} while (dwT < iAgingTime * 1000);
						}

						UINT nTimeOut = DEFINE_AGING_WAIT_TIME;
						nTimeOut = pReciepInfo->m_sInspectItem.m_nTimeOut[2];
						if (lRet == VTM_RV_SUCCESSED)
						{
							if (bUseMeasProbe == true)
							{
								lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectComplate(enumTCPIpFactorytID_PG, 8, nTimeOut);
							}
						}

						if (lRet == VTM_RV_SUCCESSED)
						{
							lRet = m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectComplate(enumTCPIpFactorytID_AGING, 0, nTimeOut);
						}
					}

					m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAGINGStep_Aging_3 Even End."));
				}
			}
		}
		else
		{
			// Aging Stop
			m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectEnd(enumTCPIpFactorytID_AGING);

			if (bUseMeasProbe == true)
			{
				m_pShareItem->GetCommTcpIpFactoryMgr()->DoWorkInspectEnd(enumTCPIpFactorytID_PG, enumTCPIpFactoryt_PG_TEST_4);
			}
		}


		// Probe Unit Connected
		double dfMove = 0., dEMove = 0., dOD = 0.;
		dfMove = pReciepInfo->m_sContactPos.m_fPreContactPos;
		lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeUnContact(AXIS_ALL_Z, dfMove);  

		CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_UPDATE_POSITION_STATE, enumDoWorkStateID_MEASURE_AGING_End, NULL);

		m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAGINGStep_Aging_3 End"));

		CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_CCD_LIVE, enumSeqItemID_CCD_LIVE_STOP, NULL);
	}

	if (lRet == VTM_RV_SUCCESSED)
	{
		bRet = true;
	}

	if (m_pShareItem->IfPauseEvt_WaitResumeEvt() != true)
	{
		bRet = false;
	}

	return bRet;
}

bool CDoWorkGlassAGINGStep_Move_First::Run()
{
	/*!
	*	Stick 개수가 2개 이하일 경우 Aging probe 로 Aging 진행
	*/
	bool bRet = false;
	long lRet = VTM_RV_SUCCESSED;

	bool bUseMeas = false;
	S_RecipeInfo* pReciepInfo;
	pReciepInfo = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeInfo();
	if (pReciepInfo != nullptr)
	{
		bUseMeas = pReciepInfo->m_sInspectItem.m_bUseAgingTest_2;
	}

	S_RecipeData* pRecipeData;
	pRecipeData = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeData();
	int nMaxStick = pRecipeData->m_vecStick.size();
	if (nMaxStick < 3)
	{
		if (bUseMeas == true)
		{
			for (size_t i = 0; i < 5; i++)
			{
				m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[i] = -1;
			}
		}

		if (bUseMeas == true)
		{
			CWindowEventHandler::GetInstance()->SendMessageToUI(enumWndID_FRAME, WM_MSG_CCD_LIVE, enumSeqItemID_CCD_LIVE_START, NULL);

			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAGINGStep_Move_First Start."));

			for (size_t i = 0; i < 5; i++)
			{
				m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[i] = -1;
			}

			// Probe Unit Unconnected
			double dfMove = 0.;
			dfMove = pReciepInfo->m_sContactPos.m_fPreContactPos;
			lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeUnContact(AXIS_ALL_Z, dfMove);

			double dProbeGap = 0., dProbeShift = 0.;
			S_RecipeMap* pRecipeMap;
			pRecipeMap = m_pShareItem->GetRecipeEditMgrPtr()->GetRecipeMap();
			dProbeGap = pRecipeMap->m_fCellDistance.y;

			double dProbeInterlockPos = 0;
			m_pShareItem->GetMaintControlMgrPtr()->GetInterlockPos(nullptr, nullptr, &dProbeInterlockPos);

			// Glass 이동
			double dStageY = 0., dOffsetX = 0.;

			dOffsetX = pReciepInfo->m_sMotionOffsetPos.m_fStageY;
			dStageY = 0. + dOffsetX;

			// Head 이동
			stMotorPosition stPosition[2];
			ZeroMemory(&stPosition, sizeof(stPosition));
			stPosition[0].axis = AXIS_X2;
			stPosition[0].dPosition = m_pShareItem->GetMaintControlMgrPtr()->GetMaxSWLimit(AXIS_X2) - 1;
			stPosition[1].axis = AXIS_X3;
			stPosition[1].dPosition = pRecipeData->m_vecStick[0].m_fStageY;
			lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosMulti(stPosition, 2, SPEED_HIGH);


			// Probe 이동
			if (lRet == df_MOTION_RV_SUCCESSED)
			{
				dProbeShift = pReciepInfo->m_sMotionOffsetPos.m_fProbeShift;

				dProbeGap = pRecipeMap->m_fCellDistance.y;
				stMotorPosition stPositionProbeX[5];
				ZeroMemory(&stPositionProbeX, sizeof(stPositionProbeX));

				stPositionProbeX[0].axis = AXIS_PROBE_X1;
				stPositionProbeX[1].axis = AXIS_PROBE_X3;
				stPositionProbeX[2].axis = AXIS_PROBE_X5;
				stPositionProbeX[3].axis = AXIS_PROBE_X7;
				stPositionProbeX[4].axis = AXIS_PROBE_X9;

				// jsyoon 2022.09.29 add
				int iRotate = 0;
				if (pReciepInfo->m_sOptionItem.m_nGlsRotate == 1)
				{
					iRotate = 1;
				}

				int k = 0;
				double fOffsetGap = 2.0;

				bool bSetPos[5];
				ZeroMemory(&bSetPos, sizeof(bSetPos));
				int nCount = 0;

				for (int nProbe = 1; nProbe < 5; nProbe++)
				{
					if (bSetPos[4 - nProbe] == true)
						continue;

					if (nCount < nMaxStick)
					{
						if (nProbe == 1)	// Aging
						{
							stPositionProbeX[4 - nProbe].dPosition = pRecipeData->m_vecStick[0].m_fStageY + dProbeShift + dOffsetX;

							// jsyoon 2022.09.29 change
							if (iRotate == 1)
								m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] = 0;
							else
								m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] = nMaxStick - 1;

							k = 0;

							bSetPos[4 - nProbe] = true;

							nCount++;
						}
						else
						{
							if (k < nMaxStick)
							{
								for (int j = k + 1; j < nMaxStick; j++)
								{
									//if (pRecipeData->m_vecStick[k].m_fStageY - pRecipeData->m_vecStick[j].m_fStageY > (dProbeInterlockPos + fOffsetGap) * 2)	// 2023.05.12 Stick 간 간격이 넓을 경우
									//{
									//	int nCurProbe = 4 - nProbe;
									//	int nNextProbe = 4 - nProbe - 1;
									//	// Next Probe
									//	stPositionProbeX[nNextProbe].dPosition = pRecipeData->m_vecStick[j].m_fStageY + dProbeShift + dOffsetX;
									//	double dMinAxisX3 = m_pShareItem->GetMaintControlMgrPtr()->GetMinSWLimit(AXIS_X3);
									//	if (stPositionProbeX[nNextProbe].dPosition > dMinAxisX3)
									//	{
									//		if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsProbeExistSensorOn(nNextProbe + df_PROBE_AGING1) == true) 	// 2022.12.21 add Probe Exist
									//		{
									//			// jsyoon 2022.09.29 change
									//			if (iRotate == 1)
									//				m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[nNextProbe] = j;	 // Aging
									//			else
									//				m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[nNextProbe] = nMaxStick - j - 1;	 // Aging
									//		}
									//	}

									//	bSetPos[nNextProbe] = true;

									//	// Cur Probe
									//	stPositionProbeX[nCurProbe].dPosition = stPositionProbeX[nNextProbe].dPosition + (dProbeInterlockPos + fOffsetGap);
									//	bSetPos[nCurProbe] = true;

									//	k = j;

									//	nCount++;

									//	break;
									//}
									//else if (pRecipeData->m_vecStick[k].m_fStageY - pRecipeData->m_vecStick[j].m_fStageY > (dProbeInterlockPos))
									if (pRecipeData->m_vecStick[k].m_fStageY - pRecipeData->m_vecStick[j].m_fStageY > (dProbeInterlockPos))
									{
										stPositionProbeX[4 - nProbe].dPosition = pRecipeData->m_vecStick[j].m_fStageY + dProbeShift + dOffsetX;

										// jsyoon 2022.09.21 add
										double dMinAxisX3 = m_pShareItem->GetMaintControlMgrPtr()->GetMinSWLimit(AXIS_X3);
										if (stPositionProbeX[4 - nProbe].dPosition > dMinAxisX3)
										{
											if (CSequenceManager::GetInstance()->GetFunctionModuleEQState()->IsProbeExistSensorOn(4 - nProbe + df_PROBE_AGING1) == true) 	// 2022.12.21 add Probe Exist
											{
												// jsyoon 2022.09.29 change
												if (iRotate == 1)
													m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] = j;	 // Aging
												else
													m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] = nMaxStick - j - 1;	 // Aging
											}

											bSetPos[4 - nProbe] = true;

											nCount++;
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
						m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - nProbe] = -1;
					}
				}

				if (m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4] == -1)
				{
					stPositionProbeX[4].dPosition = stPositionProbeX[3].dPosition + ((dProbeInterlockPos + fOffsetGap));
				}

				for (int i = 1; i < 5; i++)
				{
					if (m_pShareItem->GetShareMemoryMgrPtr()->GetSharedMemory()->m_nAgingStartStick[4 - i] == -1)
					{
						if (bSetPos[4 - i] != true)	// 2023.02.14 change
						{
							stPositionProbeX[4 - i].dPosition = stPositionProbeX[5 - i].dPosition - ((dProbeInterlockPos + fOffsetGap));

							/*!
							* 2023.03.08
							* Probe1의 현재위치에서 움직일 경우 위치 확인
							*/
							if (i == 4)	// Probe 1
							{
								double dPosProbe1 = 0;
								dPosProbe1 = m_pShareItem->GetMaintControlMgrPtr()->GetCurPos(AXIS_PROBE_X1);

								if (stPositionProbeX[4 - i].dPosition > dPosProbe1)
								{
									stPositionProbeX[4 - i].dPosition = dPosProbe1;
								}
							}
						}
					}
				}

				lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPosMulti(stPositionProbeX, 5, SPEED_HIGH, true);
			}

			// Probe Unit Connected
			if (lRet == df_MOTION_RV_SUCCESSED)
			{
				double dfMove = 0., dEMove = 0., dOD = 0.;
				dfMove = pReciepInfo->m_sContactPos.m_fPreContactPos;
				dEMove = pReciepInfo->m_sContactPos.m_fEndContactPos;
				dOD = pReciepInfo->m_sContactPos.m_fAddOD;

				// 2023.05.12 change
				lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeContact(21, dfMove, dEMove, dOD);	 // Probe가 있는곳만 Contact
			}

			m_pShareItem->GetLogFileAccessorPtr()->KMLOG(SYST, _T("CDoWorkGlassAGINGStep_Move_First End."));
		}
	}
	else
	{
		if (bUseMeas == true)
		{
			// Probe Unit Connected
			if (lRet == df_MOTION_RV_SUCCESSED)
			{
				double dfMove = 0., dEMove = 0., dOD = 0.;
				dfMove = pReciepInfo->m_sContactPos.m_fPreContactPos;
				dEMove = pReciepInfo->m_sContactPos.m_fEndContactPos;
				dOD = pReciepInfo->m_sContactPos.m_fAddOD;

				// 2023.05.12 change
				lRet = m_pShareItem->GetMaintControlMgrPtr()->MoveCmdPos_ProbeContact(21, dfMove, dEMove, dOD);	 // Probe가 있는곳만 Contact
			}
		}
	}
	
	if (lRet == VTM_RV_SUCCESSED)
	{
		bRet = true;
	}
	else
	{
		m_pShareItem->SetFail(true, lRet);
	}

	if (m_pShareItem->IfPauseEvt_WaitResumeEvt() != true)
	{
		bRet = false;
	}

	return bRet;
}
