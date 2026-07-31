#include "pch.h"
#include "MotorControl_COMIZOA.h"

#include "..\..\common\IniHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CMotorControl_COMIZOA::CMotorControl_COMIZOA(CString strConfigPath, CString strFilePath, CString strCategory)
{
#ifdef WIN64
    m_strFilePath = strConfigPath + _T("COMIZOA_x64.cme2");
#else
    m_strFilePath = strConfigPath + _T("COMIZOA_x86.cme2");
#endif

    m_pIniHelper->SetFilePath(strConfigPath + _T(""));

    LinkIniFile(false);

    m_cAxis.clear();
	m_cAxis.resize(AXIS_TYPE_END);
}

CMotorControl_COMIZOA::~CMotorControl_COMIZOA()
{
    if(m_bBdInstalled == true)
    {
        cmmGnDeviceUnload();	// Unload Device
        cmmUnloadDll();			// Unload DLL
    }
}

BOOL CMotorControl_COMIZOA::LinkIniFile(bool bSave)
{
    //CString strcategory = _T("SYS_PARA_HERA_DEVICE");
    //m_pIniHelper->Link(bSave, strcategory, _T("SYS_PARA_USE"),			&m_SerialData.bUsed,								1);
    //m_pIniHelper->Link(bSave, strcategory, _T("SYS_PARA_PORTID"),		&m_SerialData.iPort,								3);
    //m_pIniHelper->Link(bSave, strcategory, _T("SYS_PARA_BAUDRATE"),		&m_SerialData.iBaudrate,							115200);
    //m_pIniHelper->Link(bSave, strcategory, _T("SYS_PARA_PARITYBIT"),	&m_SerialData.iParity,								0);
    //m_pIniHelper->Link(bSave, strcategory, _T("SYS_PARA_STOPBIT"),		&m_SerialData.iStopbit,								0);
    //m_pIniHelper->Link(bSave, strcategory, _T("SYS_PARA_DATABIT"),		&m_SerialData.iDatabit,								8);
    //m_pIniHelper->Link(bSave, strcategory, _T("SYS_PARA_FLOWCONTROL"),	&m_SerialData.iFlowContrl,							0);

    return m_pIniHelper->IsReadEnable();
}
BOOL CMotorControl_COMIZOA::LinkDatFile(bool bSave)
{
    //CString strcategory = _T("SYS_PARA_HERA_DEVICE");
    //m_pIniHelper->Link(bSave, strcategory, _T("SYS_PARA_USE"),			&m_SerialData.bUsed,								1);
    //m_pIniHelper->Link(bSave, strcategory, _T("SYS_PARA_PORTID"),		&m_SerialData.iPort,								3);
    //m_pIniHelper->Link(bSave, strcategory, _T("SYS_PARA_BAUDRATE"),		&m_SerialData.iBaudrate,							115200);
    //m_pIniHelper->Link(bSave, strcategory, _T("SYS_PARA_PARITYBIT"),	&m_SerialData.iParity,								0);
    //m_pIniHelper->Link(bSave, strcategory, _T("SYS_PARA_STOPBIT"),		&m_SerialData.iStopbit,								0);
    //m_pIniHelper->Link(bSave, strcategory, _T("SYS_PARA_DATABIT"),		&m_SerialData.iDatabit,								8);
    //m_pIniHelper->Link(bSave, strcategory, _T("SYS_PARA_FLOWCONTROL"),	&m_SerialData.iFlowContrl,							0);

    return m_pIniHelper->IsReadEnable();
}
UINT CMotorControl_COMIZOA::ThreadRun()
{
    //-------------------------------------------------------------------------------
    // 시뮬레이션 환경에서는 데이터를 만들어서 보내준다.
    //-------------------------------------------------------------------------------




    return 0;
}
bool CMotorControl_COMIZOA::InitDevice()
{
    m_bBdInstalled = false;

    // Load Motion & DIO devices //
    if(cmmLoadDll() == FALSE)
        return false;
    long nNumAxes = 0;
    if(cmmGnDeviceLoad(TRUE, &nNumAxes) != cmERR_NONE)
        return false;
    if(nNumAxes <= 0)
        return false;

	// Load cme 파일
    USES_CONVERSION;
    if(cmmGnInitFromFile(T2A(m_strFilePath)) != cmERR_NONE)
		return false;
        
	// CME 파일에서 정의한 축수와 실제 장착되어 있는 모션 축수를 비교확인
    long nNumAxes_installed = 0, nNumAxes_defined = 0;
    if(cmmAdvGetNumAvailAxes(&nNumAxes_installed) != cmERR_NONE) // 현재 실제로 PC에 장착되어 있는 모션축 수 (cmmGnDeviceLoad() 함수에서 반환하는 축수와 동일)
		return false;
    if(cmmAdvGetNumDefinedAxes(&nNumAxes_defined) != cmERR_NONE) // CME 파일에서 정의한 축 수(CME 파일이 지정되지 않는 경우에는 현재 장착된 축 수와 일치합니다.)
		return false;
	if(nNumAxes_installed != nNumAxes_defined)
    {
        AfxMessageBox(_T("현재 장착되어 있는 모션 축 수가 정의된 축 수와 일치하지 않습니다"));
        return false;
    }

    // SERVO-ON //
    for(int i = 0; i < nNumAxes_defined; i++)
    {
        cmmGnSetServoOn(i, ON);
    }

    m_bBdInstalled = true;

    return true;
}
CString CMotorControl_COMIZOA::GetDeviceName()
{
    return _T("COMIZOA");
}
bool CMotorControl_COMIZOA::IsConnected()
{
    return m_bBdInstalled;
}






long CMotorControl_COMIZOA::ResetDevice(int iAxis)
{
    /***********************************************************************
    * ResetMC() : Position "Reset" 버튼의 콜백함수. Command & Feedback
    * position을 0으로 리셋한다.
    ***********************************************************************/
    cmmStSetPosition(m_cAxis[iAxis].m_iAxisMap, cmCNT_COMM, 0.0);

    return cmmStSetPosition(m_cAxis[iAxis].m_iAxisMap, cmCNT_FEED, 0.0);
}

long CMotorControl_COMIZOA::MoveHome(int iAxis)
{
    if(m_bBdInstalled == false)
        return cmERR_INVALID_DEVICE_ID;

    //Parameter Setting
    //mm --> um
    const double dEscapeDist = (m_cAxis[iAxis].m_setting.dEscDist / m_cAxis[iAxis].m_setting.dStpL) * 1000.0;
    const double dOffsetDist = (m_cAxis[iAxis].m_setting.dOffsetDist / m_cAxis[iAxis].m_setting.dStpL) * 1000.0;

    cmmHomeSetConfig(m_cAxis[iAxis].m_iAxisMap, m_cAxis[iAxis].m_setting.iHomeMode, m_cAxis[iAxis].m_setting.lEzCount, dEscapeDist, dOffsetDist);
    cmmHomeSetPosClrMode(m_cAxis[iAxis].m_iAxisMap, m_cAxis[iAxis].m_setting.iHomeClrMode);
    cmmCfgSetMioProperty(m_cAxis[iAxis].m_iAxisMap, cmORG_LOGIC, m_cAxis[iAxis].m_setting.iOrgLogic);
    cmmCfgSetMioProperty(m_cAxis[iAxis].m_iAxisMap, cmEZ_LOGIC, m_cAxis[iAxis].m_setting.iEzLogic);
    cmmCfgSetMioProperty(m_cAxis[iAxis].m_iAxisMap, cmERC_OUT, m_cAxis[iAxis].m_setting.bErcOut);

    SetSpeed(iAxis, SPEED_HOME, 2);

	return cmmHomeMove(m_cAxis[iAxis].m_iAxisMap, m_cAxis[iAxis].m_setting.bReverse, FALSE);
}

long CMotorControl_COMIZOA::MoveCmdPosA(int iAxis, double dPos, bool bBlockMode, int iSpeed, int iLens, int iTube)
{
    if(m_bBdInstalled == false)
        return cmERR_INVALID_DEVICE_ID;

    //mm --> um
    const double dTarget = (dPos / m_cAxis[iAxis].m_setting.dStpL) * 1000.0;

    //모션 완료시까지 반환되지 않음.
    if(bBlockMode == true)
	{
        return cmmSxMoveTo(m_cAxis[iAxis].m_iAxisMap, dTarget, 1);
	}

    //모션 시작후 바로 반환.
    return cmmSxMoveToStart(m_cAxis[iAxis].m_iAxisMap, dTarget);
}

long CMotorControl_COMIZOA::MoveCmdPosR(int iAxis, double dPos, bool bBlockMode, int iSpeed)
{
    if(m_bBdInstalled == false)
        return cmERR_INVALID_DEVICE_ID;

    //mm --> um
    const double dTarget = (dPos / m_cAxis[iAxis].m_setting.dStpL) * 1000.0;

    //모션 완료시까지 반환되지 않음.
    if(bBlockMode == true)
	{
        return cmmSxMove(m_cAxis[iAxis].m_iAxisMap, dTarget, 1);
	}

    //모션 시작후 바로 반환.
    return cmmSxMoveStart(m_cAxis[iAxis].m_iAxisMap, dTarget);
}

bool CMotorControl_COMIZOA::CheckMoveDone(int iAxis, DWORD dwTimeout)
{
    if(m_bBdInstalled == false)
        return false;

    ULONGLONG dwStart = GetTickCount();

    do
    {
        WaitForSingleObject(m_hWaitTimeEvent, 10);

        if((GetTickCount() - dwStart) > dwTimeout)
        {
            return false;
        }

        long lIsDone = 0;
        if(cmmSxIsDone(m_cAxis[iAxis].m_iAxisMap, &lIsDone) != cmERR_NONE)
        {
            if (lIsDone == cmTRUE)
            {
                return true;
            }
        }
    }
    while(1);

    return true;
}

bool CMotorControl_COMIZOA::CheckHomeDone(int iAxis, DWORD dwTimeout)
{
	// 원점복귀가 성공되었는지 체크
    long dwIsSuccess = 0;
	if(cmmHomeGetSuccess(m_cAxis[iAxis].m_iAxisMap, &dwIsSuccess) != cmERR_NONE)
	{
		return (dwIsSuccess == 1) ? true : false;
	}

	return false;
}

long CMotorControl_COMIZOA::StopDevice(int iAxis)
{
    if(m_bBdInstalled == false)
        return cmERR_INVALID_DEVICE_ID;

    if( iAxis == AXIS_ALL)
    {
        for(int i = 0; i < AXIS_TYPE_END; i++)
        {
            if( i > AXIS_T)
                continue;

            return cmmSxStop(m_cAxis[i].m_iAxisMap, TRUE, FALSE);
        }
    }

    return cmmSxStop(m_cAxis[iAxis].m_iAxisMap, TRUE, FALSE);
}


//Set parameter
long CMotorControl_COMIZOA::SetSpeed(int iAxis, int iSpeedType, int iSpeedMode)
{
    if(m_bBdInstalled == false)
        return cmERR_INVALID_DEVICE_ID;

    m_cAxis[iAxis].SetSpeed(iSpeedType);

    if(iSpeedType == SPEED_HOME)
    {
        return SetSpeedDirectHome(iAxis, m_cAxis[iAxis].m_dVel, m_cAxis[iAxis].m_dAccT, m_cAxis[iAxis].m_dScvT, m_cAxis[iAxis].m_dRevVel, iSpeedMode);
    }

    return SetSpeedDirect(iAxis, m_cAxis[iAxis].m_dVel, m_cAxis[iAxis].m_dAccT, m_cAxis[iAxis].m_dScvT, iSpeedMode);
}

long CMotorControl_COMIZOA::SetSpeedDirect(int iAxis, double dVel, double dAcc, double dDec, int iSpeedMode)
{
    //mm --> um
    const double dV = (dVel / m_cAxis[iAxis].m_setting.dStpL) * 1000.0;
    const double dA = (dAcc / m_cAxis[iAxis].m_setting.dStpL) * 1000.0;
    const double dD = (dDec / m_cAxis[iAxis].m_setting.dStpL) * 1000.0;
	    
	return cmmCfgSetSpeedPattern(m_cAxis[iAxis].m_iAxisMap, iSpeedMode, dV, dA, dD);
}

void CMotorControl_COMIZOA::SetHomeProperty(int iAxis, int iHomeMode, int iHomeClrMode, long lEzCount, int iOrgLogic, int iEzLogic, bool bErcOut, double dEscDist, double dOffsetDist)
{
    m_cAxis[iAxis].m_setting.iHomeMode = iHomeMode;
    m_cAxis[iAxis].m_setting.iHomeClrMode = iHomeClrMode;
    m_cAxis[iAxis].m_setting.lEzCount = lEzCount;
    m_cAxis[iAxis].m_setting.iOrgLogic = iOrgLogic;
    m_cAxis[iAxis].m_setting.iEzLogic = iEzLogic;
    m_cAxis[iAxis].m_setting.bErcOut = bErcOut;
    m_cAxis[iAxis].m_setting.dEscDist = dEscDist;
    m_cAxis[iAxis].m_setting.dOffsetDist = dOffsetDist;

    cmmHomeSetConfig(m_cAxis[iAxis].m_iAxisMap, iHomeMode, lEzCount, dEscDist, dOffsetDist);
    cmmHomeSetPosClrMode(m_cAxis[iAxis].m_iAxisMap, iHomeClrMode);
    cmmCfgSetMioProperty(m_cAxis[iAxis].m_iAxisMap, cmORG_LOGIC, iOrgLogic);
    cmmCfgSetMioProperty(m_cAxis[iAxis].m_iAxisMap, cmEZ_LOGIC, iEzLogic);
    cmmCfgSetMioProperty(m_cAxis[iAxis].m_iAxisMap, cmERC_OUT, bErcOut);
}


//Get parameter
double CMotorControl_COMIZOA::GetCurPos(int iAxis, int iLens, int iTube)
{
    double dPos = 0.0;

    if(cmmStGetPosition(m_cAxis[iAxis].m_iAxisMap, cmCNT_COMM, &dPos) != cmERR_NONE)
    {
        dPos *= m_cAxis[iAxis].m_setting.dStpL;
        dPos /= 1000.0;	//um --> mm
    }

    return dPos;
}
long CMotorControl_COMIZOA::GetMotorStatus(int iAxis, int nStatus)
{
    // MIO Display // Read all MIO states
    long dwTemp = 0;
    if(cmmStReadMioStatuses(m_cAxis[iAxis].m_iAxisMap, &dwTemp) != cmERR_NONE)
    {
        long iRet = 0;
        switch(nStatus)
        {
            case cmIOST_DIR:	iRet = (dwTemp >> cmIOST_DIR) & 0x1;		break; // DIR
            case cmIOST_ALM:	iRet = (dwTemp >> cmIOST_ALM) & 0x1;		break; // ALM
            case cmIOST_EZ:		iRet = (dwTemp >> cmIOST_EZ)  & 0x1;		break; // EZ
            case cmIOST_INP:	iRet = (dwTemp >> cmIOST_INP) & 0x1;		break; // INP
            case cmIOST_ELN:	iRet = (dwTemp >> cmIOST_ELN) & 0x1;		break; // +EL
            case cmIOST_ELP:	iRet = (dwTemp >> cmIOST_ELP) & 0x1;		break; // -EL
            case cmIOST_RDY:	iRet = (dwTemp >> cmIOST_RDY) & 0x1;		break; // RDY
            case cmIOST_ORG:	iRet = (dwTemp >> cmIOST_ORG) & 0x1;		break; // ORG
            case cmIOST_SD:		iRet = (dwTemp >> cmIOST_SD)  & 0x1;		break; // SD
            case cmIOST_LTC:	iRet = (dwTemp >> cmIOST_LTC) & 0x1;		break; // LTC
            case cmIOST_DRN:	iRet = (dwTemp >> cmIOST_DRN) & 0x1;		break; // -DR
            case cmIOST_DRP:	iRet = (dwTemp >> cmIOST_DRP) & 0x1;		break; // +DR
            case cmIOST_STA:	iRet = (dwTemp >> cmIOST_STA) & 0x1;		break; // STA
            case cmIOST_STP:	iRet = (dwTemp >> cmIOST_STP) & 0x1;		break; // STP
            case cmIOST_ERC:	iRet = (dwTemp >> cmIOST_ERC) & 0x1;		break; // ERC
            case cmIOST_CLR:	iRet = (dwTemp >> cmIOST_CLR) & 0x1;		break; // CLR
            case cmIOST_PCS:	iRet = (dwTemp >> cmIOST_PCS) & 0x1;		break; // PCS
        }

        return iRet;
    }

    return cmERR_INVALID_DEVICE_ID;
}


CString CMotorControl_COMIZOA::GetLastErrorString()
{
	long error_code = 0;
	cmmErrGetLastCode(&error_code);

	char strError[256];
	cmmErrGetString(error_code, strError, 256);
	
	return CString(strError);
}









// 별도 함수
long CMotorControl_COMIZOA::SetSpeedDirectHome(int iAxis, double dVel, double dAcc, double dDec, double RevVel, int iSpeedMode)
{
    //mm --> um
    const double dV = (dVel / m_cAxis[iAxis].m_setting.dStpL) * 1000.0;
    const double dA = (dAcc / m_cAxis[iAxis].m_setting.dStpL) * 1000.0;
    const double dD = (dDec / m_cAxis[iAxis].m_setting.dStpL) * 1000.0;

    return cmmHomeSetSpeedPattern(m_cAxis[iAxis].m_iAxisMap, iSpeedMode, dV, dA, dD, RevVel);
}