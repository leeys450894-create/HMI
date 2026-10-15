#include "pch.h"
#include "MCCLogMgr.h"

#include "common\IniHelper.h"

#include "AuroraFuncDefine.h"
#include "common/CommonFunc.h"
#include "..\SystemParamShare.h"
#include "SystemParamShare.h"
#include "MotorFunction/AxisType.h"	// LYS 2026-07-31 removed stray trailing quote


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


//----------------------------------------------------------------------------------------------------
// Singleton
//----------------------------------------------------------------------------------------------------
CMCCLogMgr* CMCCLogMgr::m_pInstance = nullptr;
CMCCLogMgr* CMCCLogMgr::GetInstance()
{
	return m_pInstance ? m_pInstance : (m_pInstance = new CMCCLogMgr);
}

void CMCCLogMgr::DelInstance()
{
	if (m_pInstance)
	{
		delete m_pInstance;
		m_pInstance = nullptr;
	}
}

CMCCLogMgr::CMCCLogMgr()
	: m_pIniHelper(new CIniHelper)
	, m_bUse(false)
	, m_bOutOfService(false)
	, m_pDFDCData(nullptr)
	, m_pDFDCDataMax(nullptr)
	, m_pDFDCDataMin(nullptr)
{
	m_hFDCMapFile = nullptr;
	m_pSharedMCC = nullptr;
	m_pSharedFDC = nullptr;

	m_strVersion = _T("20220602");

	m_bIonizerRun = false;

	// LYS 2026-07-31 MCC config defaults (overridden by Config_MCC.ini in LinkIniFile)
	m_bMCCTransfer = false;
	m_bMCCWriteFile = true;
	m_lMCCUploadTime = 60;
	m_lMCCSamplingTime = 1000;

	ZeroMemory(&m_dDataToELP, sizeof(m_dDataToELP));	// LYS 2026-07-29
}

CMCCLogMgr::~CMCCLogMgr()
{
	Finalize();
}

void CMCCLogMgr::Initialize(HWND hParentWnd, CRect rtROI, CString strAppName, CString strAppPath, CString strConfigPath, CString strLogPath, CString strVersion)
{
	// LYS 2026-07-31 종전에는 존재하지 않는 Bin\CoreFunction\CoreFunction.ini를 읽어 키가 무효였다.
	// GMI와 동일하게 Config\Config_MCC.ini를 읽는다(MCCLog.exe도 같은 파일을 본다).
	const CString strFilePath = strConfigPath + _T("\\Config\\Config_MCC.ini");
	m_pIniHelper->SetFilePath(strFilePath);

	LinkIniFile(false);

	if (m_bUse == true)
	{
		::API_MCC_Initialize(hParentWnd, rtROI);

		::API_MCC_SetEQName(strAppName);

		CString strMCCFilePath;
		strMCCFilePath.Format(_T("%s%s"), strAppPath, _T("MCCLog.exe"));
		::API_MCC_SetFilePath(strMCCFilePath);

		CString strRecipePath, strMCCLogPath;

		strRecipePath.Format(_T("%s"), strLogPath);
		strMCCLogPath.Format(_T("%s%s"), strRecipePath, _T("MCC"));
		::API_MCC_SetLogPath(strMCCLogPath);
		::API_MCC_SetConfigPath(strConfigPath);

		// For MCC
		m_pSharedMCC = API_MCCR_GetSharedMemory();
		if (m_pSharedMCC != nullptr)
		{
			ZeroMemory(&m_pSharedMCC->stMCCDataEQ, sizeof(m_pSharedMCC->stMCCDataEQ));
			m_pSharedMCC->nEQType = CUR_EQU_TYPE;
			m_pSharedMCC->stMCCDataEQ.iEvtFlag = -1;

			// LYS 2026-07-31 reflect Config_MCC.ini into shared memory (consumed by MCCLog.exe)
			m_pSharedMCC->uMCCUploadTime = (UINT)m_lMCCUploadTime;
			m_pSharedMCC->bMCCTransfer = m_bMCCTransfer ? TRUE : FALSE;
		}
	}

	m_pDFDCData = new double[FDC_WDATA_END];
	m_pDFDCDataMax = new double[FDC_WDATA_END];
	m_pDFDCDataMin = new double[FDC_WDATA_END];

	for (size_t i = 0; i < FDC_WDATA_END; i++)
	{
		m_pDFDCData[i] = 0;
		m_pDFDCDataMax[i] = 0;
		m_pDFDCDataMin[i] = 0;
	}


	for (size_t i = 0; i < 10; i++)
	{
		m_uLampTime[i] = 0;
		m_uLiftPinMoveTime[i] = 0;
	}

	m_strVersion = strVersion;

	CreateMemoryMapFDC();

	SetMccConvert();
}


bool CMCCLogMgr::LinkIniFile(bool bSave)
{
	m_pIniHelper->Link(bSave, _T("MCC"), _T("USE"), &m_bUse, true);
	m_pIniHelper->Link(bSave, _T("MCC"), _T("OUT_OF_SERVICE"), &m_bOutOfService, true);
	// LYS 2026-07-31 GMI-parity keys. NOTE: GMI code has a trailing-space key bug ("USABLE ") - not replicated.
	m_pIniHelper->Link(bSave, _T("MCC"), _T("USABLE"), &m_bMCCTransfer, false);
	m_pIniHelper->Link(bSave, _T("MCC"), _T("WRITE_DATA"), &m_bMCCWriteFile, true);
	m_pIniHelper->Link(bSave, _T("MCC"), _T("UPLOAD_TIME"), &m_lMCCUploadTime, 60);
	m_pIniHelper->Link(bSave, _T("MCC"), _T("SAMPLING_TIME"), &m_lMCCSamplingTime, 1000);

	return m_pIniHelper->IsReadEnable();
}


void CMCCLogMgr::Finalize()
{
	//----------------------------------------------------------------------
	// 데이터 삭제
	//----------------------------------------------------------------------
	if (m_pIniHelper)
	{
		delete m_pIniHelper;
		m_pIniHelper = nullptr;
	}

	if (m_pDFDCData) delete[] m_pDFDCData;
	if (m_pDFDCDataMax) delete[] m_pDFDCDataMax;	// 2023.03.07 add
	if (m_pDFDCDataMin) delete[] m_pDFDCDataMin;	// 2023.03.07 add

	if (m_bUse == true)	// LYS 2026-07-31 DLL side was never initialized when USE=0
	{
		::API_MCC_KillThred();

		::API_MCC_FinalizeReport();
	}
}

bool CMCCLogMgr::IsOutOfService()
{
	return m_bOutOfService;
}

CWnd* CMCCLogMgr::GetReportmWnd()
{
	return API_MCC_GetReportmWnd();
}

void CMCCLogMgr::SetMccMoterInfo(int iAxis, double dPos, double dSpeed)
{
	if (m_pSharedMCC == nullptr)	// LYS 2026-07-31 MCC disabled (Config_MCC.ini [MCC] USE=0)
		return;
	// TODO: 여기에 구현 코드 추가.
	m_pSharedMCC->stMCCDataEQ_ILog.stMoterInfo[iAxis].dSetAxisPos = dPos;
	m_pSharedMCC->stMCCDataEQ_ILog.stMoterInfo[iAxis].dSetAxisSpeed = dSpeed;
}


void CMCCLogMgr::SetMccMoterInfoCur(int iAxis, double dPos, double dSpeed, double dRMS)
{
	// LYS 2026-09-19 FDC 전송용 캐시는 MCC 사용 여부와 무관하게 채운다(UpdateSensor 가 읽는다).
	static_assert(AXIS_TYPE_END <= CMCCLogMgr::FDC_AXIS_CACHE_MAX, "FDC axis cache too small");
	if (iAxis >= 0 && iAxis < FDC_AXIS_CACHE_MAX)
	{
		m_dFdcAxisPos[iAxis] = dPos;
		m_dFdcAxisRMS[iAxis] = dRMS;
	}
	if (m_pSharedMCC == nullptr)	// LYS 2026-07-31 MCC disabled (Config_MCC.ini [MCC] USE=0)
		return;
	// TODO: 여기에 구현 코드 추가.
	m_pSharedMCC->stMCCDataEQ_ILog.stMoterInfo[iAxis].bUse = TRUE;
	m_pSharedMCC->stMCCDataEQ_ILog.stMoterInfo[iAxis].dAxisRMS = dRMS;
	m_pSharedMCC->stMCCDataEQ_ILog.stMoterInfo[iAxis].dAxisPos = dPos;
	m_pSharedMCC->stMCCDataEQ_ILog.stMoterInfo[iAxis].dAxisSpeed = dSpeed;
}


void CMCCLogMgr::UpdateSensor()
{
	// TODO: 여기에 구현 코드 추가.
	short* pFDDCMap = NULL;
	// LYS 2026-09-29 스택 오버플로 방지 - VTM_FOR_FDC 전체(160,016 바이트)를 스택에 복사하지 않는다. 쓰는 것은 stFDCWData 뿐이라
	//                아래에서 공유메모리의 그 배열만 stData로 직접 복사한다 (하이브리드 MakeCell 확인창 중 UI 타이머에서 1MB 스택 초과)
	//VTM_FOR_FDC	FDCMap;
	pFDDCMap = (short*)m_pSharedFDC;

	if (pFDDCMap == nullptr)
		return;



	//memcpy(&FDCMap, pFDDCMap, sizeof(VTM_FOR_FDC));	// LYS 2026-09-29
	int i = 0;
//	double dFDCData[100];
//	ZeroMemory(&dFDCData, sizeof(dFDCData));

	double dChuckVacuumValue[5];	 ZeroMemory(&dChuckVacuumValue, sizeof(dChuckVacuumValue)); 	// Chuck Vacuum
	double dTemperature[50];		 ZeroMemory(&dTemperature, sizeof(dTemperature));	// Temperature under

	RDATA	stData[FDC_WDATA_END];	// y.yun 20190716 add
	//memset(stData, 0, sizeof(RDATA) * 1000);
	memset(stData, 0, sizeof(stData));	// LYS 2026-09-19 배열 크기를 따라가게 했다 - FDC_WDATA_END 가 1000 보다 작아지면 1000 고정은 스택을 넘친다

	for ( i = 0; i < FDC_WDATA_END; i++)
	{
		stData[i].nMax = 9999;
		stData[i].nMin = -9999;
	}
	

//	m_pSharedFDC->FDCValue.nMainDataCount = 1;
//
//	USES_CONVERSION;
//	strcpy_s(m_pSharedFDC->FDCValue.stMainWData[20].szName, sizeof(m_pSharedFDC->FDCValue.stMainWData[20].szName), T2A(_T("VERSION")));
//	strcpy_s(m_pSharedFDC->FDCValue.stMainWData[20].szData, sizeof(m_pSharedFDC->FDCValue.stMainWData[20].szData), T2A(m_strVersion));
////	strcpy_s(m_pSharedFDC->FDCValue.stMainWData[20].szData, sizeof(m_pSharedFDC->FDCValue.stMainWData[20].szData), T2A(_T("20220602")));

	//memcpy(stData, FDCMap.FDCValue.stFDCWData, sizeof(RDATA) * 1000);
	memcpy(stData, ((VTM_FOR_FDC*)pFDDCMap)->FDCValue.stFDCWData, sizeof(stData));	// LYS 2026-09-29
	//memcpy(stData, FDCMap.FDCValue.stFDCWData, sizeof(stData));	// LYS 2026-09-19 위와 같은 이유 - 두 배열 모두 RDATA[FDC_WDATA_END]

//	if (m_lLiveCheck != FDCMap.lLiveCheck)
	{
//		m_lLiveCheck = FDCMap.lLiveCheck;
//		bUpdate = TRUE;

		//////////////////////////////////////////////////////////////////////////
		// MAIN << FDC
		CString strValue;
		for (i = 0; i < FDC_WDATA_END; i++)
		{
			strValue.Format(_T("%s"), CharToString(stData[i].szData));
			m_pDFDCData[i] = _ttof(strValue);

			m_pDFDCDataMax[i] = stData[i].nMax;
			m_pDFDCDataMin[i] = stData[i].nMin;
		}

		for (i = 0; i < 5; i++)
		{
			dChuckVacuumValue[i] = m_pDFDCData[FDC_WDATA_WT_VACUUM_1 + i];
		}

		for (i = 0; i <  FDC_WDATA_TEMP_END; i++)
		{
			dTemperature[i] = m_pDFDCData[FDC_WDATA_TEMP_1+i];
		}


		//////////////////////////////////////////////////////////////////////////
		// MCC
		// Update FDC Data
		// LYS 2026-07-31 I-Log dData 고정 슬롯 - Config\MCC\MCCNAME.csv 아날로그 행과 1:1 대응
		// (GMI 동일: 온도 3 + 전력계 12 = 15슬롯, nCntILog=15).
		// 온도 출처(FDC_LIST.xls): stFDCWData[1]=Temp_EBOX_F, [2]=Temp_EBOX_B, [3]=Temp_PCBOX.
		// LYS 2026-09-19 ※ 위 출처는 옛 목록이다. 운용 FDC_LIST 명칭 기준으로 다시 묶었다(아래 pILog[0..2]).
		//                EB_RACK_1/2 <- EL_BOX_GPS/UPS(1/2 번), PC_RACK <- PC_BOX(3 번). 슬롯 번호는 FDCSheardDefine.h
		//                FDC_WDATA_TEMP_* 가 정한다(PLCMAdress.csv 온도 비트 순서).
		// TODO: MCC_CONFIRM - PC_RACK/EB_RACK_1/EB_RACK_2 <-> PCBOX/EBOX_F/EBOX_B 명칭 확인 필요.
		// TEST
		if (m_pSharedMCC != nullptr) 
		{
			double dElpILog[MAIN_WDATA_ELP_BOX_MAX][MAIN_WDATA_ELP_ITEM_CNT];
			{
				CCriticalSectionSync single(&m_cs_ELP, TRUE);
				memcpy(&dElpILog, &m_dDataToELP, sizeof(dElpILog));
			}

			double* pILog = m_pSharedMCC->stMCCDataEQ_ILog.dData;
			//pILog[0] = m_pDFDCData[FDC_WDATA_TEMP_1 + 2];	// TEMP_PC_RACK   <- Temp_PCBOX
			//pILog[1] = m_pDFDCData[FDC_WDATA_TEMP_1 + 0];	// TEMP_EB_RACK_1 <- Temp_EBOX_F
			//pILog[2] = m_pDFDCData[FDC_WDATA_TEMP_1 + 1];	// TEMP_EB_RACK_2 <- Temp_EBOX_B
			pILog[0] = m_pDFDCData[FDC_WDATA_TEMP_PC_BOX];		// TEMP_PC_RACK   <- TEMP_SENS_PC_BOX      (LYS 2026-09-19)
			pILog[1] = m_pDFDCData[FDC_WDATA_TEMP_EL_BOX_GPS];	// TEMP_EB_RACK_1 <- TEMP_SENS_EL_BOX_GPS
			pILog[2] = m_pDFDCData[FDC_WDATA_TEMP_EL_BOX_UPS];	// TEMP_EB_RACK_2 <- TEMP_SENS_EL_BOX_UPS

			// ELP ElecBox EB1/EB2 -> N01_*/N02_* (ELP item order: TEMP,SMOKE,VOLT,HARM_VOLT,CURR,HARM_CURR,LEAK_CURR,INST_PW,ACCUM_PW)
			for (int iBox = 0; iBox < MAIN_WDATA_ELP_BOX_MAX; iBox++)
			{
				const int nBase = 3 + (iBox * 6);
				pILog[nBase + 0] = dElpILog[iBox][8];	// N0x_T_PW(kWh) <- ACCUM_PW
				pILog[nBase + 1] = dElpILog[iBox][7];	// N0x_I_PW(kW)  <- INST_PW
				pILog[nBase + 2] = dElpILog[iBox][2];	// N0x_R_VOL(V)  <- VOLT   TODO: MCC_CONFIRM phase mapping
				pILog[nBase + 3] = 0.0;					// N0x_T_VOL(V)  ELP has no T-phase. TODO: MCC_CONFIRM
				pILog[nBase + 4] = dElpILog[iBox][4];	// N0x_R_CUR(A)  <- CURR   TODO: MCC_CONFIRM phase mapping
				pILog[nBase + 5] = 0.0;					// N0x_T_CUR(A)  ELP has no T-phase. TODO: MCC_CONFIRM
			}
		}


		//// SEM Controller
		//m_pSharedMCC->stMCCDataEQ_ILog.dData[5] = m_pDFDCData[FDC_WDATA_UPS_1]; // FDCMap.FDCValue.dN01TPW1;			// UPS 적산 전력량
		//m_pSharedMCC->stMCCDataEQ_ILog.dData[6] = m_pDFDCData[FDC_WDATA_UPS_2]; // FDCMap.FDCValue.dN01TPW2;			// UPS 순시 전력량
		//m_pSharedMCC->stMCCDataEQ_ILog.dData[7] = m_pDFDCData[FDC_WDATA_UPS_3]; // FDCMap.FDCValue.dN01TPW3;			// UPS R상 전압
		//m_pSharedMCC->stMCCDataEQ_ILog.dData[8] = m_pDFDCData[FDC_WDATA_UPS_4]; // FDCMap.FDCValue.dN01TPW4;			// UPS T상 전압
		//m_pSharedMCC->stMCCDataEQ_ILog.dData[9] = m_pDFDCData[FDC_WDATA_UPS_5]; // FDCMap.FDCValue.dN01TPW5;			// UPS R상 전류
		//m_pSharedMCC->stMCCDataEQ_ILog.dData[10] = m_pDFDCData[FDC_WDATA_UPS_6]; // FDCMap.FDCValue.dN01TPW6;			// UPS T상 전류
		//m_pSharedMCC->stMCCDataEQ_ILog.dData[11] = m_pDFDCData[FDC_WDATA_GPS_1]; // FDCMap.FDCValue.dN02TPW7;			// GPS 적산 전력량
		//m_pSharedMCC->stMCCDataEQ_ILog.dData[12] = m_pDFDCData[FDC_WDATA_GPS_2]; // FDCMap.FDCValue.dN02TPW8;			// GPS 순시 전력량
		//m_pSharedMCC->stMCCDataEQ_ILog.dData[13] = m_pDFDCData[FDC_WDATA_GPS_3]; // FDCMap.FDCValue.dN02TPW9;			// GPS R상 전압
		//m_pSharedMCC->stMCCDataEQ_ILog.dData[14] = m_pDFDCData[FDC_WDATA_GPS_4]; // FDCMap.FDCValue.dN02TPW10;			// GPS T상 전압
		//m_pSharedMCC->stMCCDataEQ_ILog.dData[15] = m_pDFDCData[FDC_WDATA_GPS_5]; // FDCMap.FDCValue.dN02TPW11;			// GPS R상 전류
		//m_pSharedMCC->stMCCDataEQ_ILog.dData[16] = m_pDFDCData[FDC_WDATA_GPS_6]; // FDCMap.FDCValue.dN02TPW12;			// GPS T상 전류
		//////////////////////////////////////////////////////////////////////////
		if (m_pSharedMCC != nullptr)	// LYS 2026-07-31 fixed count = MCCNAME.csv analog rows
			m_pSharedMCC->stMCCDataEQ_ILog.nCntILog = 15;

		//////////////////////////////////////////////////////////////////////////
		// MAIN >> FDC
	//	m_pFDCMapForOCP->FDCValue.bionizerstatus = m_pSystem->stInterLock.bionizerstatus;	//ksk 20150529 add

		// AXIS
		//int nAxis[23] = { AXIS_LOADER_Z, AXIS_X, AXIS_Y, AXIS_R, AXIS_X4, AXIS_UNIT_Z, AXIS_R1,
		//	AXIS_Y7, AXIS_Y8,
		//	AXIS_Y1, AXIS_Z1, AXIS_X2, AXIS_THETA1, AXIS_ZOOM1, AXIS_FILTER1, AXIS_ROTATE1,
		//	AXIS_Y2, AXIS_Z4, AXIS_X2, AXIS_THETA2, AXIS_ZOOM2, AXIS_FILTER2, AXIS_ROTATE2 };

		//for (int iAxis = 0; iAxis < 23; iAxis++)
		//{
		//	if (m_pSystem->stSystem[VTM_MOTION].bConnect)		// jsyoon 20171102 add
		//	{
		//		m_pSharedFDC->FDCValue.dFDCData[FDC_OCP_NUM_AXIS_LOAD_R_TOG + iAxis] = m_pSystem->stMotionStatus[nAxis[iAxis]].dAxisRMS;
		//		m_pSharedFDC->FDCValue.dFDCData[FDC_OCP_NUM_AXIS_LOAD_R_POS + iAxis] = m_pSystem->stMotionStatus[nAxis[iAxis]].dAxisPos;		// mm
		//	}
		//}

		//// LIGHT TIME
		//for (int iLamp = 0; iLamp < df_MAX_LAMP; iLamp++)
		//{
		//	m_pSharedFDC->FDCValue.dLampTime[iLamp] = (double)m_LmapTimer[iLamp].m_spanCur.GetTotalHours();
		//}

		// jsyoon 20170830 add
		//m_pSharedFDC->FDCValue.dFDCData[FDC_OCP_ILLUMINATION_1] = m_pSystem->stResult.dILLMIPower[0];		// H1M1
		//m_pSharedFDC->FDCValue.dFDCData[FDC_OCP_ILLUMINATION_2] = m_pSystem->stResult.dILLMIPower[1];		// H2M1
		//m_pSharedFDC->FDCValue.dFDCData[FDC_OCP_ILLUMINATION_3] = m_pSystem->stResult.dILLMIPower[2];		// H1M2
		//m_pSharedFDC->FDCValue.dFDCData[FDC_OCP_ILLUMINATION_4] = m_pSystem->stResult.dILLMIPower[3];		// H2M2

		//m_pSharedFDC->FDCValue.dFDCData[FDC_ICE_MAIN_VER] = m_dMainVer;		// jsyoon 20171031 add


//		int nNext = 20;
//		CString strName, strData;
//		strName.Empty();
//		strData.Empty();

//	//	m_pSharedFDC->FDCValue.nMainDataCount = 1;

//		USES_CONVERSION;
//		strcpy_s(m_pSharedFDC->FDCValue.stMainWData[nNext].szName, sizeof(m_pSharedFDC->FDCValue.stMainWData[nNext].szName), T2A(_T("VERSION")));
//		strcpy_s(m_pSharedFDC->FDCValue.stMainWData[nNext].szData, sizeof(m_pSharedFDC->FDCValue.stMainWData[nNext].szData), T2A(m_strVersion));		nNext++;
//	//	strcpy_s(m_pSharedFDC->FDCValue.stMainWData[20].szData, sizeof(m_pSharedFDC->FDCValue.stMainWData[20].szData), T2A(_T("20220602")));

//		strcpy_s(m_pSharedFDC->FDCValue.stMainWData[nNext].szName, sizeof(m_pSharedFDC->FDCValue.stMainWData[nNext].szName), T2A(_T("CIM_VERSION")));
//		strcpy_s(m_pSharedFDC->FDCValue.stMainWData[nNext].szData, sizeof(m_pSharedFDC->FDCValue.stMainWData[nNext].szData), T2A(m_strVersion));		nNext++;

//		strData.Format(_T("%d"), 10);
//		strcpy_s(m_pSharedFDC->FDCValue.stMainWData[nNext].szName, sizeof(m_pSharedFDC->FDCValue.stMainWData[nNext].szName), T2A(_T("CIM_DISK_C")));
//		strcpy_s(m_pSharedFDC->FDCValue.stMainWData[nNext].szData, sizeof(m_pSharedFDC->FDCValue.stMainWData[nNext].szData), T2A(strData));				nNext++;

//		strData.Format(_T("%d"), 10);
//		strcpy_s(m_pSharedFDC->FDCValue.stMainWData[nNext].szName, sizeof(m_pSharedFDC->FDCValue.stMainWData[nNext].szName), T2A(_T("CIM_DISK_D")));
//		strcpy_s(m_pSharedFDC->FDCValue.stMainWData[nNext].szData, sizeof(m_pSharedFDC->FDCValue.stMainWData[nNext].szData), T2A(strData));				nNext++;


//		strName.Format(_T("Ionizer Run"));
//		strData.Format(_T("%d"), m_bIonizerRun);
//		strcpy_s(m_pSharedFDC->FDCValue.stMainWData[nNext].szName, sizeof(m_pSharedFDC->FDCValue.stMainWData[nNext].szName), T2A(strName));
//		strcpy_s(m_pSharedFDC->FDCValue.stMainWData[nNext].szData, sizeof(m_pSharedFDC->FDCValue.stMainWData[nNext].szData), T2A(strData));			nNext++;


//		// LIGHT TIME
//		for (size_t i = 0; i < 4; i++)
//		{
//			if (m_uLampTime[i] > 0)
//			{
//				strName.Format(_T("LAMP TIME%d"), i + 1);
//				strData.Format(_T("%d"), m_uLampTime[i]);

//				strcpy_s(m_pSharedFDC->FDCValue.stMainWData[nNext].szName, sizeof(m_pSharedFDC->FDCValue.stMainWData[nNext].szName), T2A(strName));
//				strcpy_s(m_pSharedFDC->FDCValue.stMainWData[nNext].szData, sizeof(m_pSharedFDC->FDCValue.stMainWData[nNext].szData), T2A(strData));			
//			}

//			nNext++;
//		}

//		// Lift Pin
//		for (size_t i = 0; i < 3; i++)
//		{
//			if (m_uLiftPinMoveTime[i] > 0.01)
//			{
//				strName.Format(_T("LIFT STEP%d"), i + 1);
//				strData.Format(_T("%d"), m_uLiftPinMoveTime[i]);

//				strcpy_s(m_pSharedFDC->FDCValue.stMainWData[nNext].szName, sizeof(m_pSharedFDC->FDCValue.stMainWData[nNext].szName), T2A(strName));
//				strcpy_s(m_pSharedFDC->FDCValue.stMainWData[nNext].szData, sizeof(m_pSharedFDC->FDCValue.stMainWData[nNext].szData), T2A(strData));			
//			}	

//			nNext++;
//		}

//		int nAxisNum[24] = { AXIS_LOADER_Z, AXIS_X, AXIS_LOADER_R, AXIS_X4, AXIS_Z9,
//				AXIS_Y1, AXIS_Y2, AXIS_X2, AXIS_Z2,
//				AXIS_Y3, AXIS_Y4, AXIS_X3, AXIS_Z1,
//				AXIS_THETA2, AXIS_ZOOM2, AXIS_FILTER2,
//				AXIS_THETA1, AXIS_ZOOM1, AXIS_FILTER1,
//				AXIS_PROBE_Z1 , AXIS_PROBE_Z2 , AXIS_PROBE_Z3 , AXIS_PROBE_Z4 , AXIS_PROBE_Z5 };

//		// AXIS
//		int nAxis = 0;
//		for (size_t iAxis = 0; iAxis < 24; iAxis++)
//		{
//			nAxis = nAxisNum[iAxis];
//			if (m_pSharedMCC != nullptr && m_pSharedMCC->stMCCDataEQ_ILog.stMoterInfo[nAxis].bUse)	// LYS 2026-07-31 null guard
//			{
//				strName.Format(_T("AXIS POS_%d"), nAxis);
//				strData.Format(_T("%d"), (int)(m_pSharedMCC->stMCCDataEQ_ILog.stMoterInfo[nAxis].dAxisPos * 1000));

//				// POS
//				strcpy_s(m_pSharedFDC->FDCValue.stMainWData[nNext].szName, sizeof(m_pSharedFDC->FDCValue.stMainWData[nNext].szName), T2A(strName));
//				strcpy_s(m_pSharedFDC->FDCValue.stMainWData[nNext].szData, sizeof(m_pSharedFDC->FDCValue.stMainWData[nNext].szData), T2A(strData));

//				nNext++;

//				strName.Format(_T("AXIS RMS_%d"), nAxis);
//				strData.Format(_T("%.4f"),(m_pSharedMCC->stMCCDataEQ_ILog.stMoterInfo[nAxis].dAxisRMS));

//				// RMS
//				strcpy_s(m_pSharedFDC->FDCValue.stMainWData[nNext].szName, sizeof(m_pSharedFDC->FDCValue.stMainWData[nNext].szName), T2A(strName));
//				strcpy_s(m_pSharedFDC->FDCValue.stMainWData[nNext].szData, sizeof(m_pSharedFDC->FDCValue.stMainWData[nNext].szData), T2A(strData));

//				nNext++;
//			}
//		}

//	
		// LYS 2026-09-19 제어 -> FDC 를 운용 FDC 의 읽기 방식에 맞춰 고정 슬롯으로 다시 썼다.
		//                운용 FDC 는 FDC_LIST SHARE 행을 stMainWData[0] 부터 순서대로 읽는다(FDCSheardDefine.h NUM_MAIN_WDATA_SHARE).
		//                위 옛 코드는 20 번부터, 그것도 사용 축만 골라 가변 위치로 써서 어느 행과도 맞지 않았다 - 주석 처리.
		//                축 대응: FDC_LIST 이름 <-> Config_Motion_ACS.dat NAME (VISION 1 = ZOOM1_Left 등 = AXIS_*2).
		//                값: POSI = 모니터 루프의 GetCurPos (Maint 화면과 같은 값, mm/deg), LOAD = GetMotorRMSValue.
		//                사용 안 하는 축(.dat USE[0])은 모니터 루프가 건너뛰어 0 으로 나간다.
		int nNext = MAIN_WDATA_SHARE_END;
		CString strName, strData;
		USES_CONVERSION;
		{
			// LYS 2026-09-24 FDC_LIST 개정(FDC_LIST_after.csv): LOAD 는 35 축 중 16 축만 남았다(bLoad = 1).
			//   POSI 는 35 축 그대로 슬롯 MTR_POSI_START(12)+i, LOAD 는 bLoad 축만 표 순서대로 MTR_LOAD_START(47)+n.
			//   FDC_LIST 의 LOAD 행 순서가 POSI 순서의 부분열이라 표 하나에 플래그로 둔다. 구 코드는 아래 #if 0.
			static const struct { int nAxis; const char* pszName; int bLoad; } s_FdcMtr[MAIN_WDATA_SHARE_MTR_CNT] = {
			{ AXIS_X0,             "SMPL_PLATE_X",                1 },
			{ AXIS_X2,             "BACK_HEAD_X",                 1 },
			{ AXIS_X3,             "FRONT_HEAD_X",                1 },
			{ AXIS_X4,             "AC_X",                        1 },
			{ AXIS_Y1,             "BACK_HEAD_1_Y",               1 },
			{ AXIS_Y2,             "BACK_HEAD_2_Y",               1 },
			{ AXIS_Y3,             "FRONT_HEAD_1_Y",              1 },
			{ AXIS_Y4,             "FRONT_HEAD_2_Y",              1 },
			{ AXIS_Z2,             "FRONT_HEAD_Z",                1 },
			{ AXIS_Z3,             "BACK_HEAD_1_SUB_Z",           1 },
			{ AXIS_Z4,             "BACK_HEAD_2_SUB_Z",           1 },
			{ AXIS_Z5,             "FRONT_HEAD_1_SUB_Z",          1 },
			{ AXIS_Z6,             "FRONT_HEAD_2_SUB_Z",          1 },
			{ AXIS_Z9,             "AC_Z",                        0 },
			{ AXIS_ZOOM1,          "VISION_2_ZOOM",               0 },
			{ AXIS_THETA1,         "VISION_2_ANG",                0 },
			{ AXIS_FILTER1,        "VISION_2_FILTER",             0 },
			{ AXIS_ZOOM2,          "VISION_1_ZOOM",               0 },
			{ AXIS_THETA2,         "VISION_1_ANG",                0 },
			{ AXIS_FILTER2,        "VISION_1_FILTER",             0 },
			{ AXIS_LOADER_Z,       "LIFT_PIN",                    0 },
			{ AXIS_LOADER_R,       "SMPL_PLATE_R",                1 },
			{ AXIS_PROBE_X9,       "PROBE_X",                     1 },
			{ AXIS_PROBE_Y5,       "PROBE_Y",                     1 },
			{ AXIS_PROBE_Z5,       "PROBE_Z",                     0 },
			{ AXIS_ANGLE_SF1,      "VIEWING_ANG_1_SCHEIMPFLUG",   0 },
			{ AXIS_ANGLE_ROTATE1,  "VIEWING_ANG_1_ROTATE",        0 },
			{ AXIS_ANGLE_ZOOM1,    "VIEWING_ANG_1_ZOOM",          0 },
			{ AXIS_ANGLE_TILT1,    "VIEWING_ANG_1_TILT",          0 },
			{ AXIS_ANGLE_AZM1,     "VIEWING_ANG_1_AZIMUTH",       0 },
			{ AXIS_ANGLE_SF2,      "VIEWING_ANG_2_SCHEIMPFLUG",   0 },
			{ AXIS_ANGLE_ROTATE2,  "VIEWING_ANG_2_ROTATE",        0 },
			{ AXIS_ANGLE_ZOOM2,    "VIEWING_ANG_2_ZOOM",          0 },
			{ AXIS_ANGLE_TILT2,    "VIEWING_ANG_2_TILT",          0 },
			{ AXIS_ANGLE_AZM2,     "VIEWING_ANG_2_AZIMUTH",       0 },
			};
			int nLoad = 0;
			for (int i = 0; i < MAIN_WDATA_SHARE_MTR_CNT; i++)
			{
				const int nAxis = s_FdcMtr[i].nAxis;
				double dPos = 0.0, dRMS = 0.0;
				if (nAxis >= 0 && nAxis < FDC_AXIS_CACHE_MAX)
				{
					dPos = m_dFdcAxisPos[nAxis];
					dRMS = m_dFdcAxisRMS[nAxis];
				}
				RDATA& rPos = m_pSharedFDC->FDCValue.stMainWData[MAIN_WDATA_SHARE_MTR_POSI_START + i];
				_snprintf_s(rPos.szName, sizeof(rPos.szName), _TRUNCATE, "MTR_%s_POSI", s_FdcMtr[i].pszName);
				_snprintf_s(rPos.szData, sizeof(rPos.szData), _TRUNCATE, "%.3f", dPos);
				if (s_FdcMtr[i].bLoad != 0 && nLoad < MAIN_WDATA_SHARE_MTR_LOAD_CNT)
				{
					RDATA& rLoad = m_pSharedFDC->FDCValue.stMainWData[MAIN_WDATA_SHARE_MTR_LOAD_START + nLoad];
					_snprintf_s(rLoad.szName, sizeof(rLoad.szName), _TRUNCATE, "MTR_%s_LOAD", s_FdcMtr[i].pszName);
					_snprintf_s(rLoad.szData, sizeof(rLoad.szData), _TRUNCATE, "%.2f", dRMS);
					nLoad++;
				}
			}
			{
				static bool s_bLoadCntWarned = false;
				if (nLoad != MAIN_WDATA_SHARE_MTR_LOAD_CNT && s_bLoadCntWarned == false)
				{
					s_bLoadCntWarned = true;
					CString strLogL;
					strLogL.Format(_T("FDC SHARE MTR LOAD count mismatch - table %d, FDC_LIST %d"), nLoad, (int)MAIN_WDATA_SHARE_MTR_LOAD_CNT);
					CLogWrapper::GetInstance()->KMLOG(WARN, strLogL);
				}
			}

			// LYS 2026-09-24 SHARE 2~11 PG 전력량 (FDC_LIST PG1_I_LEAK, PG2_I_LEAK, PG1_N03_T_PW/I_PW/VOL/CUR, PG2_N04_T_PW/I_PW/VOL/CUR).
			//   출처는 ELP ElecBox 모니터(S10F01, m_dDataToELP) - PG1 = 전장 박스 1(box 0), PG2 = 전장 박스 2(box 1).
			//   T_PW = ACCUM_PW(kWh), I_PW = INST_PW(kW), VOL = VOLT(V), CUR = CURR(A), I_LEAK = LEAK_CURR(mA). 형식은 400 번대 ELP 칸과 같은 %.4f.
			{
				double dElpPg[MAIN_WDATA_ELP_BOX_MAX][MAIN_WDATA_ELP_ITEM_CNT];
				{
					CCriticalSectionSync single(&m_cs_ELP, TRUE);
					memcpy(&dElpPg, &m_dDataToELP, sizeof(dElpPg));
				}
				static const struct { const char* pszName; int iBox; int iItem; } s_FdcPg[MAIN_WDATA_SHARE_PG_CNT] = {
					{ "PG1_I_LEAK",   0, MAIN_WDATA_ELP_ITEM_LEAK_CURR },
					{ "PG2_I_LEAK",   1, MAIN_WDATA_ELP_ITEM_LEAK_CURR },
					{ "PG1_N03_T_PW", 0, MAIN_WDATA_ELP_ITEM_ACCUM_PW  },
					{ "PG1_N03_I_PW", 0, MAIN_WDATA_ELP_ITEM_INST_PW   },
					{ "PG1_N03_VOL",  0, MAIN_WDATA_ELP_ITEM_VOLT      },
					{ "PG1_N03_CUR",  0, MAIN_WDATA_ELP_ITEM_CURR      },
					{ "PG2_N04_T_PW", 1, MAIN_WDATA_ELP_ITEM_ACCUM_PW  },
					{ "PG2_N04_I_PW", 1, MAIN_WDATA_ELP_ITEM_INST_PW   },
					{ "PG2_N04_VOL",  1, MAIN_WDATA_ELP_ITEM_VOLT      },
					{ "PG2_N04_CUR",  1, MAIN_WDATA_ELP_ITEM_CURR      },
				};
				for (int i = 0; i < MAIN_WDATA_SHARE_PG_CNT; i++)
				{
					RDATA& r = m_pSharedFDC->FDCValue.stMainWData[MAIN_WDATA_SHARE_PG_START + i];
					_snprintf_s(r.szName, sizeof(r.szName), _TRUNCATE, "%s", s_FdcPg[i].pszName);
					_snprintf_s(r.szData, sizeof(r.szData), _TRUNCATE, "%.4f", dElpPg[s_FdcPg[i].iBox][s_FdcPg[i].iItem]);
				}
			}

#if 0	// LYS 2026-09-24 구: LOAD 35 축 (FDC_LIST 2026-09-19 판)
			static const struct { int nAxis; const char* pszName; } s_FdcMtr[MAIN_WDATA_SHARE_MTR_CNT] = {
			{ AXIS_X0,             "SMPL_PLATE_X" },
			{ AXIS_X2,             "BACK_HEAD_X" },
			{ AXIS_X3,             "FRONT_HEAD_X" },
			{ AXIS_X4,             "AC_X" },
			{ AXIS_Y1,             "BACK_HEAD_1_Y" },
			{ AXIS_Y2,             "BACK_HEAD_2_Y" },
			{ AXIS_Y3,             "FRONT_HEAD_1_Y" },
			{ AXIS_Y4,             "FRONT_HEAD_2_Y" },
			{ AXIS_Z2,             "FRONT_HEAD_Z" },
			{ AXIS_Z3,             "BACK_HEAD_1_SUB_Z" },
			{ AXIS_Z4,             "BACK_HEAD_2_SUB_Z" },
			{ AXIS_Z5,             "FRONT_HEAD_1_SUB_Z" },
			{ AXIS_Z6,             "FRONT_HEAD_2_SUB_Z" },
			{ AXIS_Z9,             "AC_Z" },
			{ AXIS_ZOOM1,          "VISION_2_ZOOM" },
			{ AXIS_THETA1,         "VISION_2_ANG" },
			{ AXIS_FILTER1,        "VISION_2_FILTER" },
			{ AXIS_ZOOM2,          "VISION_1_ZOOM" },
			{ AXIS_THETA2,         "VISION_1_ANG" },
			{ AXIS_FILTER2,        "VISION_1_FILTER" },
			{ AXIS_LOADER_Z,       "LIFT_PIN" },
			{ AXIS_LOADER_R,       "SMPL_PLATE_R" },
			{ AXIS_PROBE_X9,       "PROBE_X" },
			{ AXIS_PROBE_Y5,       "PROBE_Y" },
			{ AXIS_PROBE_Z5,       "PROBE_Z" },
			{ AXIS_ANGLE_SF1,      "VIEWING_ANG_1_SCHEIMPFLUG" },
			{ AXIS_ANGLE_ROTATE1,  "VIEWING_ANG_1_ROTATE" },
			{ AXIS_ANGLE_ZOOM1,    "VIEWING_ANG_1_ZOOM" },
			{ AXIS_ANGLE_TILT1,    "VIEWING_ANG_1_TILT" },
			{ AXIS_ANGLE_AZM1,     "VIEWING_ANG_1_AZIMUTH" },
			{ AXIS_ANGLE_SF2,      "VIEWING_ANG_2_SCHEIMPFLUG" },
			{ AXIS_ANGLE_ROTATE2,  "VIEWING_ANG_2_ROTATE" },
			{ AXIS_ANGLE_ZOOM2,    "VIEWING_ANG_2_ZOOM" },
			{ AXIS_ANGLE_TILT2,    "VIEWING_ANG_2_TILT" },
			{ AXIS_ANGLE_AZM2,     "VIEWING_ANG_2_AZIMUTH" },
			};
			for (int i = 0; i < MAIN_WDATA_SHARE_MTR_CNT; i++)
			{
				const int nAxis = s_FdcMtr[i].nAxis;
				double dPos = 0.0, dRMS = 0.0;
				if (nAxis >= 0 && nAxis < FDC_AXIS_CACHE_MAX)
				{
					dPos = m_dFdcAxisPos[nAxis];
					dRMS = m_dFdcAxisRMS[nAxis];
				}
				RDATA& rPos  = m_pSharedFDC->FDCValue.stMainWData[MAIN_WDATA_SHARE_MTR_POSI_START + i];
				RDATA& rLoad = m_pSharedFDC->FDCValue.stMainWData[MAIN_WDATA_SHARE_MTR_LOAD_START + i];
				// 이름은 참고용(FDC 는 szData 만 읽는다). 긴 이름은 szName[32] 를 넘으므로 잘라 쓴다.
				_snprintf_s(rPos.szName,  sizeof(rPos.szName),  _TRUNCATE, "MTR_%s_POSI", s_FdcMtr[i].pszName);
				_snprintf_s(rPos.szData,  sizeof(rPos.szData),  _TRUNCATE, "%.3f", dPos);
				_snprintf_s(rLoad.szName, sizeof(rLoad.szName), _TRUNCATE, "MTR_%s_LOAD", s_FdcMtr[i].pszName);
				_snprintf_s(rLoad.szData, sizeof(rLoad.szData), _TRUNCATE, "%.2f", dRMS);
			}
#endif

			// LYS 2026-09-24 아래 슬롯 번호는 옛 목록 기준이다. PG 10 행 추가·LOAD 19 행 삭제로 LIFT PIN 63~66, PROBE OD 67, SW VER 68 이 됐다(enum 참조).
			// LYS 2026-09-19 모션 외 SHARE 칸.
			//   0/1  MCCB TRIP : M4072 / M4073 (1 = TRIP). DlgMainForm 모니터 루프가 SetFdcMccbTrip 으로 넘긴다.
			//   72~74 LIFT PIN FIRST/SECOND/THIRD : 다단 상승 구간별 소요 시간(초, 소수 3자리).
			//         DoWork_LPPreMove / DoWork_LPMove 가 그 단계가 실제로 움직였을 때만 SetLiftPinMoveSec(0~2) 로 넘긴다.
			//   75   LIFT PIN DOWN : 다단 하강(유리 있음) 3 단계 전체 소요 시간(초). DoWork_LPMove 가 SetLiftPinMoveSec(3).
			//   76   INSPEC PROBE OD : 레시피 m_sContactPos.m_fAddOD (측정·AGING 이 End Contact 에 더하는 OD).
			//   77   CONTR SW VER : ALPHA 빌드 날짜 yyyymmdd (CAuroraMgr::GetCompiledDate(1), MCC VERSION 과 같은 값).
			//        FDC 는 szData 를 atof 로 읽으므로 숫자만 들어가는 형식을 쓴다(파일 버전 "1.0.x" 는 1.0 으로 잘린다).
			{
				struct { int nSlot; const char* pszName; CStringA strData; } s_Fdc[] = {
					{ MAIN_WDATA_SHARE_MCCB_TRIP_GPS,   "MAIN_GPS_MCCB_TRIP", "" },
					{ MAIN_WDATA_SHARE_MCCB_TRIP_UPS,   "MAIN_UPS_MCCB_TRIP", "" },
					{ MAIN_WDATA_SHARE_LIFT_PIN_FIRST,  "LIFT_PIN_FIRST",     "" },
					{ MAIN_WDATA_SHARE_LIFT_PIN_SECOND, "LIFT_PIN_SECOND",    "" },
					{ MAIN_WDATA_SHARE_LIFT_PIN_THIRD,  "LIFT_PIN_THIRD",     "" },
					{ MAIN_WDATA_SHARE_LIFT_PIN_DOWN,   "LIFT_PIN_DOWN",      "" },
					{ MAIN_WDATA_SHARE_INSPEC_PROBE_OD, "INSPEC_PROBE_OD",    "" },
					{ MAIN_WDATA_SHARE_CONTR_SW_VER,    "CONTR_SW_VER",       "" },
				};
				s_Fdc[0].strData.Format("%d", m_nFdcMccbTripGps);
				s_Fdc[1].strData.Format("%d", m_nFdcMccbTripUps);
				//s_Fdc[2].strData.Format("%u", m_uLiftPinMoveTime[0]);
				//s_Fdc[3].strData.Format("%u", m_uLiftPinMoveTime[1]);
				//s_Fdc[4].strData.Format("%u", m_uLiftPinMoveTime[2]);
				s_Fdc[2].strData.Format("%.3f", m_dLiftPinMoveSec[0]);	// LYS 2026-09-19 초
				s_Fdc[3].strData.Format("%.3f", m_dLiftPinMoveSec[1]);
				s_Fdc[4].strData.Format("%.3f", m_dLiftPinMoveSec[2]);
				s_Fdc[5].strData.Format("%.3f", m_dLiftPinMoveSec[3]);	// DOWN
				s_Fdc[6].strData.Format("%.3f", m_dFdcProbeOD);
				s_Fdc[7].strData = CStringA(m_strVersion);
				for (int i = 0; i < _countof(s_Fdc); i++)
				{
					RDATA& r = m_pSharedFDC->FDCValue.stMainWData[s_Fdc[i].nSlot];
					_snprintf_s(r.szName, sizeof(r.szName), _TRUNCATE, "%s", s_Fdc[i].pszName);
					_snprintf_s(r.szData, sizeof(r.szData), _TRUNCATE, "%s", (LPCSTR)s_Fdc[i].strData);
				}
			}
		}

		// LYS 2026-07-29 ELP ElecBox 모니터 데이터(사양서 2026-07-28) -> MAIN_WDATA_ELP_START+ 고정 슬롯.
		// FDC_LIST가 SOURCE ADDRESS로 직접 인덱싱하므로 nMainDataCount는 그대로 둔다.
		// LYS 2026-09-19 ※ 운용 FDC 는 SOURCE ADDRESS 를 보지 않고 SHARE 행을 순서대로 읽으므로 이 400 번대는 읽히지 않는다.
		{
			static const char* szElpItem[MAIN_WDATA_ELP_ITEM_CNT] =
				{ "TEMP", "SMOKE", "VOLT", "HARM_VOLT", "CURR", "HARM_CURR", "LEAK_CURR", "INST_PW", "ACCUM_PW" };

			double dElpData[MAIN_WDATA_ELP_BOX_MAX][MAIN_WDATA_ELP_ITEM_CNT];
			{
				CCriticalSectionSync single(&m_cs_ELP, TRUE);
				memcpy(&dElpData, &m_dDataToELP, sizeof(dElpData));
			}

			int nSlot = 0;
			for (int iBox = 0; iBox < MAIN_WDATA_ELP_BOX_MAX; iBox++)
			{
				for (int iItem = 0; iItem < MAIN_WDATA_ELP_ITEM_CNT; iItem++)
				{
					nSlot = MAIN_WDATA_ELP_START + (iBox * MAIN_WDATA_ELP_BOX_STRIDE) + iItem;

					strName.Format(_T("EB%d_%hs"), iBox + 1, szElpItem[iItem]);
					strData.Format(_T("%.4f"), dElpData[iBox][iItem]);

					strcpy_s(m_pSharedFDC->FDCValue.stMainWData[nSlot].szName, sizeof(m_pSharedFDC->FDCValue.stMainWData[nSlot].szName), T2A(strName));
					strcpy_s(m_pSharedFDC->FDCValue.stMainWData[nSlot].szData, sizeof(m_pSharedFDC->FDCValue.stMainWData[nSlot].szData), T2A(strData));
				}
			}
		}

		//m_pSharedFDC->FDCValue.nMainDataCount = nNext - 20;
		m_pSharedFDC->FDCValue.nMainDataCount = nNext;	// LYS 2026-09-19 SHARE 슬롯 수(운용 FDC 는 이 값을 쓰지 않는다)
		
	}
}


int CMCCLogMgr::CreateMemoryMapFDC()
{
	// TODO: 여기에 구현 코드 추가.
	if (!m_hFDCMapFile)
	{
		m_hFDCMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0,
			sizeof(VTM_FOR_FDC), VTM_SHARE_MEM_STRING_FDC);

		if (!m_hFDCMapFile)
		{
			AfxMessageBox(_T("CreateFileMapping Failed (VTM_SHARE_MEM_STRING_FDC)"));
			return FALSE;
		}


		m_pSharedFDC = (VTM_FOR_FDC*)MapViewOfFile(m_hFDCMapFile, FILE_MAP_ALL_ACCESS, 0, 0,
			sizeof(VTM_FOR_FDC));

		if (!m_pSharedFDC)
		{
			AfxMessageBox(_T("MapViewOfFile Failed (VTM_SHARE_MEM_STRING_FDC)"));
			return FALSE;
		}
	}

	return 1;
}


bool CMCCLogMgr::GetFDCDValue(int iIndex, double* pdValue, double* pdMax, double* pdMin)
{
	// TODO: 여기에 구현 코드 추가.
	bool bRet = true;
	// LYS 2026-09-19 범위 밖 인덱스를 막는다. 호출부는 반환값을 안 보므로 0 을 채워 준다.
	if (iIndex < 0 || iIndex >= FDC_WDATA_END)
	{
		if (pdValue != nullptr) *pdValue = 0.0;
		if (pdMax != nullptr) *pdMax = 0.0;
		if (pdMin != nullptr) *pdMin = 0.0;
		return false;
	}
	if (m_pDFDCData != nullptr)
	{
		if(pdValue !=nullptr) * pdValue = m_pDFDCData[iIndex];
		if (pdMax != nullptr) * pdMax = m_pDFDCDataMax[iIndex];
		if (pdMin != nullptr) * pdMin = m_pDFDCDataMin[iIndex];
	}

	return bRet;
}

// Action
void CMCCLogMgr::SetMCC_Action(int iSqNum, BOOL bStart, int nStep/* = 0*/, int nFromTo/* = 0*/)
{
	if (m_pSharedMCC == nullptr)	// LYS 2026-07-31 MCC disabled (Config_MCC.ini [MCC] USE=0)
		return;
	CCriticalSectionSync syncEvt(&m_cs_MCCEvent, TRUE);	// LYS 2026-07-31 slot write + AddQueue must be atomic

	int iFlag = 0;
	m_pSharedMCC->stMCCDataEQ.iEvtFlag = iFlag;

	if (/*iSqNum == MCC_A_MOVE_ORGIN || SqNum == MCC_A_MOVE_UNLOAD || iSqNum == MCC_A_MOVE_MEAS || */ iSqNum == MCC_A_CHNG_LAMP || iSqNum == MCC_A_CHNG_PROBE)
	{
		m_pSharedMCC->stMCCDataEQ.iSpqNum = m_uMCCAMath[iSqNum] + nStep;
		nStep = 0;
	}
	else
	{
		m_pSharedMCC->stMCCDataEQ.iSpqNum = m_uMCCAMath[iSqNum];
	}

	m_pSharedMCC->stMCCDataEQ.bFlagStoE = bStart;
	m_pSharedMCC->stMCCDataEQ.iStepNum = nStep;		
	m_pSharedMCC->stMCCDataEQ.iFromto  = nFromTo;		

	// LYS 2026-10-14 recipe ID length: truncate into the 32-TCHAR MCC shared fields (MCCLog.exe layout unchanged). _tcscpy_s took the CRT invalid-parameter
	//   path on a 32+ char ID (Debug assert -> abort on the engine thread). <= 31 chars: same bytes and return value (0) as before
	_tcsncpy_s(m_pSharedMCC->stMCCInfoEQ.szGlassID, _countof(m_pSharedMCC->stMCCInfoEQ.szGlassID), CSystemParamShare::GetInstance()->GetGlassID(), _TRUNCATE);
	_tcsncpy_s(m_pSharedMCC->stMCCInfoEQ.szLotID, _countof(m_pSharedMCC->stMCCInfoEQ.szLotID), CSystemParamShare::GetInstance()->GetLotID(), _TRUNCATE);
	_tcsncpy_s(m_pSharedMCC->stMCCInfoEQ.szRecipeID, _countof(m_pSharedMCC->stMCCInfoEQ.szRecipeID), CSystemParamShare::GetInstance()->GetRecipeID(), _TRUNCATE);
	_tcsncpy_s(m_pSharedMCC->stMCCInfoEQ.szStepID, _countof(m_pSharedMCC->stMCCInfoEQ.szStepID), CSystemParamShare::GetInstance()->GetStepID(), _TRUNCATE);

	::API_MCC_AddQueue(iFlag);
}

// Signal
void CMCCLogMgr::SetMCC_Signal(int iSqNum, BOOL bStart, int nStep/* = 0*/, int nFromTo/* = 0*/)
{
	if (m_pSharedMCC == nullptr)	// LYS 2026-07-31 MCC disabled (Config_MCC.ini [MCC] USE=0)
		return;
	CCriticalSectionSync syncEvt(&m_cs_MCCEvent, TRUE);	// LYS 2026-07-31 slot write + AddQueue must be atomic

	int iFlag = 1;
	m_pSharedMCC->stMCCDataEQ.iEvtFlag = iFlag;

	m_pSharedMCC->stMCCDataEQ.iSpqNum = m_uMCCSMath[iSqNum];

	m_pSharedMCC->stMCCDataEQ.bFlagStoE = bStart;
	m_pSharedMCC->stMCCDataEQ.iStepNum = nStep;		
	m_pSharedMCC->stMCCDataEQ.iFromto= nFromTo;	

	// LYS 2026-10-14 recipe ID length: truncate into the 32-TCHAR MCC shared fields (MCCLog.exe layout unchanged). _tcscpy_s took the CRT invalid-parameter
	//   path on a 32+ char ID (Debug assert -> abort on the engine thread). <= 31 chars: same bytes and return value (0) as before
	_tcsncpy_s(m_pSharedMCC->stMCCInfoEQ.szGlassID, _countof(m_pSharedMCC->stMCCInfoEQ.szGlassID), CSystemParamShare::GetInstance()->GetGlassID(), _TRUNCATE);
	_tcsncpy_s(m_pSharedMCC->stMCCInfoEQ.szLotID, _countof(m_pSharedMCC->stMCCInfoEQ.szLotID), CSystemParamShare::GetInstance()->GetLotID(), _TRUNCATE);
	_tcsncpy_s(m_pSharedMCC->stMCCInfoEQ.szRecipeID, _countof(m_pSharedMCC->stMCCInfoEQ.szRecipeID), CSystemParamShare::GetInstance()->GetRecipeID(), _TRUNCATE);
	_tcsncpy_s(m_pSharedMCC->stMCCInfoEQ.szStepID, _countof(m_pSharedMCC->stMCCInfoEQ.szStepID), CSystemParamShare::GetInstance()->GetStepID(), _TRUNCATE);

	::API_MCC_AddQueue(iFlag);
}

// Alarm
void CMCCLogMgr::SetMCC_Alarm(int iSqNum, BOOL bStart, int nStep/* = 0*/, int nFromTo/* = 0*/)
{
	if (m_pSharedMCC == nullptr)	// LYS 2026-07-31 MCC disabled (Config_MCC.ini [MCC] USE=0)
		return;
	CCriticalSectionSync syncEvt(&m_cs_MCCEvent, TRUE);	// LYS 2026-07-31 slot write + AddQueue must be atomic

	int iFlag = 2;
	m_pSharedMCC->stMCCDataEQ.iEvtFlag = iFlag;

	m_pSharedMCC->stMCCDataEQ.iSpqNum = iSqNum;

	m_pSharedMCC->stMCCDataEQ.bFlagStoE = bStart;
	m_pSharedMCC->stMCCDataEQ.iStepNum = nStep;
	m_pSharedMCC->stMCCDataEQ.iFromto = nFromTo;

	// LYS 2026-10-14 recipe ID length: truncate into the 32-TCHAR MCC shared fields (MCCLog.exe layout unchanged). _tcscpy_s took the CRT invalid-parameter
	//   path on a 32+ char ID (Debug assert -> abort on the engine thread). <= 31 chars: same bytes and return value (0) as before
	_tcsncpy_s(m_pSharedMCC->stMCCInfoEQ.szGlassID, _countof(m_pSharedMCC->stMCCInfoEQ.szGlassID), CSystemParamShare::GetInstance()->GetGlassID(), _TRUNCATE);
	_tcsncpy_s(m_pSharedMCC->stMCCInfoEQ.szLotID, _countof(m_pSharedMCC->stMCCInfoEQ.szLotID), CSystemParamShare::GetInstance()->GetLotID(), _TRUNCATE);
	_tcsncpy_s(m_pSharedMCC->stMCCInfoEQ.szRecipeID, _countof(m_pSharedMCC->stMCCInfoEQ.szRecipeID), CSystemParamShare::GetInstance()->GetRecipeID(), _TRUNCATE);
	_tcsncpy_s(m_pSharedMCC->stMCCInfoEQ.szStepID, _countof(m_pSharedMCC->stMCCInfoEQ.szStepID), CSystemParamShare::GetInstance()->GetStepID(), _TRUNCATE);

	::API_MCC_AddQueue(iFlag);
}

void CMCCLogMgr::SetMccConvert()
{
	for (size_t i = 0; i < 500; i++)
	{
		m_uMCCAMath[i] = i;
		m_uMCCSMath[i] = i;
	}

//	m_uMCCAMath[MCC_A_MOVE_ORGIN] = 500;
//	m_uMCCAMath[MCC_A_MOVE_UNLOAD] = 600;
//	m_uMCCAMath[MCC_A_MOVE_MEAS] = 700;
//
//	m_uMCCAMath[MCC_A_INIT_MOT] = 1;
//	m_uMCCAMath[MCC_A_INIT_IO_CTRL] = 2;
//	m_uMCCAMath[MCC_A_INIT_CAM] = 3;
////	m_uMCCAMath[MCC_A_INIT_AFC] = 4;
//	m_uMCCAMath[MCC_A_MOT_HOME] = 5;
//
//	m_uMCCAMath[MCC_A_CHNG_PROBE] = 11;
//	m_uMCCAMath[MCC_A_CHNG_LAMP] = 201;
//	m_uMCCAMath[MCC_A_CHNG_LENS] = 202;
//
//	m_uMCCAMath[MCC_A_PROBE_PCB_LOCK] = 22;
//	m_uMCCAMath[MCC_A_PROBE_PCB_UNLOCK] = 31;
//	m_uMCCAMath[MCC_A_PROBE_CLAM_LOCK] = 21;
//	m_uMCCAMath[MCC_A_PROBE_CLAM_UNLOCK] = 32;
//	m_uMCCAMath[MCC_A_PROBE_ZERO_LOCK] = 20;
//	m_uMCCAMath[MCC_A_PROBE_ZERO_UNLOCK] = 33;
//
//	m_uMCCAMath[MCC_A_VACUUM_ON] = 53;
//	m_uMCCAMath[MCC_A_VACUUM_OFF] = 95;
//
//	m_uMCCAMath[MCC_A_COMPONENT_IN] = 41;
//	m_uMCCAMath[MCC_A_AUTO_CYCLE] = 42;
//	m_uMCCAMath[MCC_A_COMPONENT_OUT] = 43;
//
//	m_uMCCAMath[MCC_A_LOAD_READY] = 44;
//	m_uMCCAMath[MCC_A_MEAS_READY] = 45;
//	m_uMCCAMath[MCC_A_UNLOAD_READY] = 46;
//	m_uMCCAMath[MCC_A_STEP_MOVE_UNLOAD] = 47;
//
//	m_uMCCAMath[MCC_A_STEP_GLASS_GET] = 51;
//	m_uMCCAMath[MCC_A_STEP_GLASS_PUT] = 71;
//
//	m_uMCCAMath[MCC_A_STEP_LOADER_UNLOAD] = 91;
//	m_uMCCAMath[MCC_A_STEP_LOADER_LOAD] = 101;
//
//	m_uMCCAMath[MCC_A_PROC_ALIGN] = 111;
//
//	m_uMCCAMath[MCC_A_PROC_MEAS] = 131;
//	m_uMCCAMath[MCC_A_STEP_MEAS] = 132;
//
//	m_uMCCAMath[MCC_A_PROC_MEAS_BP] = 133;
//	m_uMCCAMath[MCC_A_MEAS_STEP_BP] = 134;
//	m_uMCCAMath[MCC_A_MEAS_SAMP_BP] = 135;
//
//	m_uMCCAMath[MCC_A_PROC_MEAS_CAM] = 136;
//	m_uMCCAMath[MCC_A_MEAS_STEP_CAM] = 137;
//	m_uMCCAMath[MCC_A_MEAS_SAMP_CAM] = 138;
//
//	m_uMCCAMath[MCC_A_PROC_MEAS_ANGLE] = 139;
//	m_uMCCAMath[MCC_A_MEAS_STEP_ANGLE] = 140;
//	m_uMCCAMath[MCC_A_MEAS_SAMP_ANGLE] = 141;
//
//	m_uMCCAMath[MCC_A_PROC_MEAS_PPA] = 142;
//	m_uMCCAMath[MCC_A_MEAS_STEP_PPA] = 143;
//	m_uMCCAMath[MCC_A_MEAS_SAMP_PPA] = 144;
//
//	// ACTION
//	m_uMCCSMath[MCC_S_X_ENTRY_ARM] = 1;
//	m_uMCCSMath[MCC_S_X_GLASS_EXIST] = 2;
//	m_uMCCSMath[MCC_S_X_PIN_UP] = 3;
//	m_uMCCSMath[MCC_S_X_PIN_DOWN] = 4;
//	m_uMCCSMath[MCC_S_X_SAFETY_ON] = 5;
//	m_uMCCSMath[MCC_S_X_LOADER_ROT_POS_0] = 6;
//	m_uMCCSMath[MCC_S_X_LOADER_ROT_POS_1] = 7;
//	m_uMCCSMath[MCC_S_X_LOADER_ROT_POS_2] = 8;
//	m_uMCCSMath[MCC_S_X_LOADER_ROT_POS_3] = 9;
//	m_uMCCSMath[MCC_S_X_RUN_AUTO_SWITCH] = 10;
//	m_uMCCSMath[MCC_S_X_PLC_INIT] = 11;
//	m_uMCCSMath[MCC_S_X_PLC_READY] = 12;
//	m_uMCCSMath[MCC_S_X_PLC_LOAD] = 13;
//	m_uMCCSMath[MCC_S_X_PLC_MEAS] = 14;
//
//	m_uMCCSMath[MCC_S_X_PROBE_EXIST] = 15;
//	m_uMCCSMath[MCC_S_X_PCB_LOCK] = 16;
//	m_uMCCSMath[MCC_S_X_CLAM_LOCK] = 17;
//	m_uMCCSMath[MCC_S_X_ZERO_LOCK] = 18;
//
//	m_uMCCSMath[MCC_S_Y_POWOR_ON_1] = 51;
//	m_uMCCSMath[MCC_S_Y_POWOR_ON_4] = 52;
//
//	m_uMCCSMath[MCC_S_Y_PLC_INIT] = 53;
//	m_uMCCSMath[MCC_S_Y_PLC_READY] = 54;
//	m_uMCCSMath[MCC_S_Y_PLC_LOAD] = 55;
//	m_uMCCSMath[MCC_S_Y_PLC_MEAS] = 56;
}

void CMCCLogMgr::SetLampTime(int iLamp, int Time)
{
	if(iLamp >=0 && iLamp < 10)
		m_uLampTime[iLamp] = Time; 
}

void CMCCLogMgr::SetLiftPinMoveTime(int iStep, int Time)
{
	if (iStep >= 0 && iStep < 10)
		m_uLiftPinMoveTime[iStep] = Time;
}

void CMCCLogMgr::SetIonizerRun(bool bRun)
{
	m_bIonizerRun = bRun;
}

// LYS 2026-09-19 FDC SHARE 0/1 - MCCB TRIP (DlgMainForm 모니터 루프에서 호출)
void CMCCLogMgr::SetFdcMccbTrip(int nGps, int nUps)
{
	m_nFdcMccbTripGps = nGps;
	m_nFdcMccbTripUps = nUps;
}

// LYS 2026-09-19 FDC SHARE 76 - 레시피 OD (DlgMainForm 모니터 루프에서 호출)
void CMCCLogMgr::SetFdcProbeOD(double dOD)
{
	m_dFdcProbeOD = dOD;
}

// LYS 2026-09-19 FDC SHARE 72~75 - 리프트 핀 다단 동작 소요 시간(초). 0~2 = 상승 1/2/3 단계, 3 = 하강 전체.
void CMCCLogMgr::SetLiftPinMoveSec(int iStep, double dSec)
{
	if (iStep >= 0 && iStep < _countof(m_dLiftPinMoveSec))
		m_dLiftPinMoveSec[iStep] = dSec;
}

// LYS 2026-07-29 ELP ElecBox monitor data push (called from PGMon socket receive thread)
void CMCCLogMgr::SetDataToELP(int iBox, int iItem, double dData)
{
	if (iBox < 0 || iBox >= MAIN_WDATA_ELP_BOX_MAX)
		return;
	if (iItem < 0 || iItem >= MAIN_WDATA_ELP_ITEM_CNT)
		return;

	CCriticalSectionSync single(&m_cs_ELP, TRUE);
	m_dDataToELP[iBox][iItem] = dData;
}
