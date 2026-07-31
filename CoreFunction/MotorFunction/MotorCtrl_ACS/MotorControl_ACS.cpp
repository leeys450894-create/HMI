#include "pch.h"
#include "MotorControl_ACS.h"

#include "..\..\common\IniHelper.h"
#include "include\ACSC.h"

#include <MMSYSTEM.H>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CMotorControl_ACS::CMotorControl_ACS(CString strAppPath, CString strFilePath, CString strCategory)
    : m_strCategory(strCategory)
{
    m_pIniHelper->SetFilePath(strFilePath);

    LinkIniFile(false);

    m_cAxis.clear();
    m_cAxis.resize(df_USE_MAX_AXIS);

    // LYS 2026-07-13 dual controller init
    for (int i = 0; i < df_MAX_CTRL; i++)
        m_hAcs[i] = ACSC_INVALID;

    m_strFilePath = strAppPath + _T("CoreFunction\\MotorConfig\\Config_Motion_ACS.dat");
	LinkDatFile(false);
}

CMotorControl_ACS::~CMotorControl_ACS()
{
    // LYS 2026-07-13 dual controller cleanup
    for (int i = 0; i < df_MAX_CTRL; i++)
    {
        if (m_hAcs[i] != ACSC_INVALID)
        {
            acsc_CloseComm(m_hAcs[i]);
            m_hAcs[i] = ACSC_INVALID;
        }
    }
}

BOOL CMotorControl_ACS::LinkIniFile(bool bSave)
{
    // LYS 2026-07-13 dual controller addresses
    m_pIniHelper->Link(bSave, m_strCategory, _T("ADDRESS"),		&m_strAddress[0],			(CString)_T("10.0.0.100"));
    m_pIniHelper->Link(bSave, m_strCategory, _T("ADDRESS2"),	&m_strAddress[1],			(CString)_T("10.0.0.101"));
    m_pIniHelper->Link(bSave, m_strCategory, _T("PORT"),		&m_iPort,					701);

    return m_pIniHelper->IsReadEnable();
}

BOOL CMotorControl_ACS::LinkDatFile(bool bSave)
{
	// for motion setting
	CMap<CString, LPCTSTR, int, int> g_iMapAxis;
	g_iMapAxis.SetAt(_T("X0-AXIS"), AXIS_X0);
	g_iMapAxis.SetAt(_T("X2-AXIS"), AXIS_X2);
    g_iMapAxis.SetAt(_T("X3-AXIS"), AXIS_X3);
	g_iMapAxis.SetAt(_T("X4-AXIS"), AXIS_X4);
	g_iMapAxis.SetAt(_T("Y0-AXIS"), AXIS_Y0);
	g_iMapAxis.SetAt(_T("Y3-AXIS"), AXIS_Y3);
	g_iMapAxis.SetAt(_T("Y4-AXIS"), AXIS_Y4);
	g_iMapAxis.SetAt(_T("Y1-AXIS"), AXIS_Y1);
	g_iMapAxis.SetAt(_T("Y2-AXIS"), AXIS_Y2);
	g_iMapAxis.SetAt(_T("Y5-AXIS"), AXIS_Y5);
	g_iMapAxis.SetAt(_T("Y6-AXIS"), AXIS_Y6);
	g_iMapAxis.SetAt(_T("Y7-AXIS")/*"PROBE CROSS LEFT"*/, AXIS_Y7);
	g_iMapAxis.SetAt(_T("Y8-AXIS")/*"PROBE CROSS RIGHT"*/, AXIS_Y8);
	g_iMapAxis.SetAt(_T("R0-AXIS")/*"TOP CHUCK ROTATE"*/, AXIS_R0);
	g_iMapAxis.SetAt(_T("R1-AXIS")/*"PROBE ROTATE"*/, AXIS_R1);
	g_iMapAxis.SetAt(_T("R2-AXIS")/*"PROBE ROTATE RIGHT"*/, AXIS_R2);

	g_iMapAxis.SetAt(_T("Z1-AXIS")/*"Z0(29M HEAD1)"*/, AXIS_Z1);
	g_iMapAxis.SetAt(_T("Z2-AXIS")/*"Z2(REVIEW HEAD1"*/, AXIS_Z2);
	g_iMapAxis.SetAt(_T("Z3-AXIS")/*"Z1(4M HEAD1)"*/, AXIS_Z3);

	g_iMapAxis.SetAt(_T("Z4-AXIS")/*"Z5(29M HEAD2)"*/, AXIS_Z4);
	g_iMapAxis.SetAt(_T("Z5-AXIS")/*"Z3(REVIEW HEAD2)"*/, AXIS_Z5);
	g_iMapAxis.SetAt(_T("Z6-AXIS")/*"Z4(4M HEAD2)"*/, AXIS_Z6);

	g_iMapAxis.SetAt(_T("Z7-AXIS"), AXIS_Z7);
	g_iMapAxis.SetAt(_T("Z8-AXIS"), AXIS_Z8);

	g_iMapAxis.SetAt(_T("VZ1-AXIS")/*"ZOOM1(29M)"*/, AXIS_ZOOM1);
	g_iMapAxis.SetAt(_T("VR1-AXIS")/*"ROTATE1(29M)"*/, AXIS_THETA1);
	g_iMapAxis.SetAt(_T("VF1-AXIS")/*"FILTER1(29M)"*/, AXIS_FILTER1);
    g_iMapAxis.SetAt(_T("VL1-AXIS")/*"LENS CORRECTION1(29M)"*/, AXIS_VLENS1);

	g_iMapAxis.SetAt(_T("VZ2-AXIS")/*"ZOOM2(29M)"*/, AXIS_ZOOM2);
	g_iMapAxis.SetAt(_T("VR2-AXIS")/*"ROTATE2(29M)"*/, AXIS_THETA2);
	g_iMapAxis.SetAt(_T("VF2-AXIS")/*"FILTER2(29M)"*/, AXIS_FILTER2);
    g_iMapAxis.SetAt(_T("VL2-AXIS")/*"LENS CORRECTION2(29M)"*/, AXIS_VLENS2);

	g_iMapAxis.SetAt(_T("VR3-AXIS")/*"ROTATE1(4M)"*/, AXIS_ROTATE1);
	g_iMapAxis.SetAt(_T("VF3-AXIS")/*"FOCUS1(4M)"*/, AXIS_FOCUS1);

	g_iMapAxis.SetAt(_T("VR4-AXIS")/*"ROTATE2(4M)"*/, AXIS_ROTATE2);
	g_iMapAxis.SetAt(_T("VF4-AXIS")/*"FOCUS2(4M)"*/, AXIS_FOCUS2);

	g_iMapAxis.SetAt(_T("VR5-AXIS")/*"ROTATE3(4M)"*/, AXIS_ROTATE3);
	g_iMapAxis.SetAt(_T("VF5-AXIS")/*"FOCUS3(4M)"*/, AXIS_FOCUS3);

	g_iMapAxis.SetAt(_T("VR6-AXIS")/*"ROTATE4(4M)"*/, AXIS_ROTATE4);
	g_iMapAxis.SetAt(_T("VF6-AXIS")/*"FOCUS4(4M)"*/, AXIS_FOCUS4);

	g_iMapAxis.SetAt(_T("VT1-AXIS"), AXIS_COLOR_DEGREE1);
	g_iMapAxis.SetAt(_T("VT2-AXIS"), AXIS_COLOR_DEGREE2);

	g_iMapAxis.SetAt(_T("VT3-AXIS"), AXIS_SPECTO_DEGREE1);
	g_iMapAxis.SetAt(_T("VT4-AXIS"), AXIS_SPECTO_DEGREE2);

	g_iMapAxis.SetAt(_T("UZ1-AXIS")/*"UNIT_Z"*/, AXIS_UNIT_Z);
	g_iMapAxis.SetAt(_T("UZ2-AXIS")/*"UNIT_Z2"*/, AXIS_UNIT_Z2);
	g_iMapAxis.SetAt(_T("Z9-AXIS")/*"AUTO CHANGE Z"*/, AXIS_Z9);

	g_iMapAxis.SetAt(_T("LX-AXIS")/*"LOADER X"*/, AXIS_LOADER_X);
	g_iMapAxis.SetAt(_T("LY-AXIS")/*"LOADER Y"*/, AXIS_LOADER_Y);
	g_iMapAxis.SetAt(_T("LZ-AXIS")/*"LOADER Z"*/, AXIS_LOADER_Z);
	g_iMapAxis.SetAt(_T("LR-AXIS")/*"LOADER ROTATE"*/, AXIS_LOADER_R);

	g_iMapAxis.SetAt(_T("LP1-AXIS")/*"LOADER LP1"*/, AXIS_LP);
	g_iMapAxis.SetAt(_T("LP2-AXIS")/*"LOADER LP2"*/, AXIS_LP2);

	g_iMapAxis.SetAt(_T("L1-AXIS")/*"LENS LEFT"*/, AXIS_L);
	g_iMapAxis.SetAt(_T("L2-AXIS")/*"LENS RIGHT"*/, AXIS_L2);
	g_iMapAxis.SetAt(_T("XS1-AXIS")/*"SUBG LEFT"*/, AXIS_XS);
	g_iMapAxis.SetAt(_T("XS2-AXIS")/*"SUBG RIGHT"*/, AXIS_XS2);

	g_iMapAxis.SetAt(_T("WL1-AXIS")/*"WHITE LEFT"*/, AXIS_WL1);
	g_iMapAxis.SetAt(_T("WL2-AXIS")/*"WHITE RIGHT"*/, AXIS_WL2);
	g_iMapAxis.SetAt(_T("UL1-AXIS")/*"UV LEFT"*/, AXIS_UL1);
	g_iMapAxis.SetAt(_T("UL2-AXIS")/*"UV RIGHT"*/, AXIS_UL2);

	g_iMapAxis.SetAt(_T("UZY_AXIS"), AXIS_UNIT_CROSS);
	g_iMapAxis.SetAt(_T("PROBE ROTATE"), AXIS_PYAW);
	g_iMapAxis.SetAt(_T("YS1-AXIS"), AXIS_YS);
	g_iMapAxis.SetAt(_T("YS2-AXIS"), AXIS_YS2);

    g_iMapAxis.SetAt(_T("Z11-AXIS")/*"SE Z1"*/, AXIS_Z11);

    // jsyoon 2022.01.21 add PROBE AXIS
    g_iMapAxis.SetAt(_T("PBX1-AXIS")/*"SE Z1"*/, AXIS_PROBE_X1);
    g_iMapAxis.SetAt(_T("PBX2-AXIS")/*"SE Z1"*/, AXIS_PROBE_X2);
    g_iMapAxis.SetAt(_T("PBX3-AXIS")/*"SE Z1"*/, AXIS_PROBE_X3);
    g_iMapAxis.SetAt(_T("PBX4-AXIS")/*"SE Z1"*/, AXIS_PROBE_X4);
    g_iMapAxis.SetAt(_T("PBX5-AXIS")/*"SE Z1"*/, AXIS_PROBE_X5);
    g_iMapAxis.SetAt(_T("PBX6-AXIS")/*"SE Z1"*/, AXIS_PROBE_X6);
    g_iMapAxis.SetAt(_T("PBX7-AXIS")/*"SE Z1"*/, AXIS_PROBE_X7);
    g_iMapAxis.SetAt(_T("PBX8-AXIS")/*"SE Z1"*/, AXIS_PROBE_X8);
    g_iMapAxis.SetAt(_T("PBX9-AXIS")/*"SE Z1"*/, AXIS_PROBE_X9);
    g_iMapAxis.SetAt(_T("PBX10-AXIS")/*"SE Z1"*/, AXIS_PROBE_X10);
    g_iMapAxis.SetAt(_T("PBY1-AXIS")/*"SE Z1"*/, AXIS_PROBE_Y1);
    g_iMapAxis.SetAt(_T("PBY2-AXIS")/*"SE Z1"*/, AXIS_PROBE_Y2);
    g_iMapAxis.SetAt(_T("PBY3-AXIS")/*"SE Z1"*/, AXIS_PROBE_Y3);
    g_iMapAxis.SetAt(_T("PBY4-AXIS")/*"SE Z1"*/, AXIS_PROBE_Y4);
    g_iMapAxis.SetAt(_T("PBY5-AXIS")/*"SE Z1"*/, AXIS_PROBE_Y5);
    g_iMapAxis.SetAt(_T("PBZ1-AXIS")/*"SE Z1"*/, AXIS_PROBE_Z1);
    g_iMapAxis.SetAt(_T("PBZ2-AXIS")/*"SE Z1"*/, AXIS_PROBE_Z2);
    g_iMapAxis.SetAt(_T("PBZ3-AXIS")/*"SE Z1"*/, AXIS_PROBE_Z3);
    g_iMapAxis.SetAt(_T("PBZ4-AXIS")/*"SE Z1"*/, AXIS_PROBE_Z4);
    g_iMapAxis.SetAt(_T("PBZ5-AXIS")/*"SE Z1"*/, AXIS_PROBE_Z5);
	//////////////////////////////////////////////////////////////////////////

	if (bSave == true)
	{
		//backup Orgfile
		CString szConfigFile, sNewPath, strBackFile;
		SYSTEMTIME st;
		CString sTime;
		::GetLocalTime(&st);
		sTime.Format(_T("%04d%02d%02d-%02d%02d%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

		strBackFile.Format(_T("Config_Motion_ACS_%s.dat"), sTime);
		sNewPath.Format(_T("%s"), m_strFilePath);
		sNewPath.Replace(_T("Config_Motion_ACS.dat"), strBackFile);
		CopyFile(m_strFilePath, sNewPath, FALSE);

		int i = 0, j = 0, n = 0, nAxis = 0;
		for (int i = 0; i < AXIS_TYPE_END; i++)
		{
			if (m_cAxis[i].m_setting.bUse == TRUE)
				nAxis++;
		}

		// Save
		CStdioFile  cfile;
		cfile.Open(m_strFilePath, CStdioFile::modeCreate | CStdioFile::modeWrite);
		CString strLine;
		strLine.Format(_T("[SERVO]\n"));
		cfile.WriteString(strLine);

		strLine.Format(_T("  NUM[%d]\n"), nAxis);
		cfile.WriteString(strLine);

		i = 0;
		for (j = 0; j<df_USE_MAX_AXIS; j++)
		{
			if (m_cAxis[j].m_setting.bUse == TRUE)
			{
				strLine.Format(_T("  N[%2d] DIVISION[%-10s] NAME[%-20s] USE[%d] HW[%2d] HPLC[%2d] DIR[%d], CLAMP[%d] TYPE[%d] VH[%10.1f] VM[%10.1f] VL[%10.1f] VO[%10.1f] AH[%10.1f] AM[%10.1f] AL[%10.1f] STH[%10.1f] STM[%10.1f] STL[%10.1f] PLS[%10.5f] EL[%10.5f] CN[%10.4f] MX[%10.4f] MN[%10.4f] OF[%10.4f] HOME[%10.4f]\n"),
					i+1,						// N
					m_cAxis[j].m_strDivision,	// DIVISION
					m_cAxis[j].m_strAxisName,		// NAME
					m_cAxis[j].m_setting.bUse,  // USE
					m_cAxis[j].m_iAxisMap,	  // HW
					m_cAxis[j].m_setting.iNumHomePLC, // HPLC 
					m_cAxis[j].m_iDir,				// DIR
					m_cAxis[j].m_setting.iClamp,		// CLAMP
					m_cAxis[j].m_setting.iStepper,	// TYPE
					m_cAxis[j].m_setting.dVelH,						// VH
					m_cAxis[j].m_setting.dVelM,						// VM
					m_cAxis[j].m_setting.dVelL,						// VL
					m_cAxis[j].m_setting.dVelHome,
					m_cAxis[j].m_setting.dAccH,
					m_cAxis[j].m_setting.dAccM,
					m_cAxis[j].m_setting.dAccL,
					m_cAxis[j].m_setting.dScvH,
					m_cAxis[j].m_setting.dScvM,
					m_cAxis[j].m_setting.dScvL,
                    m_cAxis[j].m_setting.dPulse,        // jsyoon 2022.02.22 add Pulse
					m_cAxis[j].m_setting.dEncL,
					m_cAxis[j].m_setting.dDimCen,
					m_cAxis[j].m_setting.dDimMax,
					m_cAxis[j].m_setting.dDimMin,
					m_cAxis[j].m_setting.dOffset,
					m_cAxis[j].m_setting.dHomePos
				);
				cfile.WriteString(strLine);

				i++;
			}
		}

        strLine.Format(_T("[OFFSET]\n"));
        cfile.WriteString(strLine);

        strLine.Format(_T("[BASEPOS]\n"));
        cfile.WriteString(strLine);

        strLine.Format(_T("[GAP]\n"));
        cfile.WriteString(strLine);

        strLine.Format(_T("[FILTER]\n"));
        cfile.WriteString(strLine);

        

		strLine.Format(_T("[BUFFERS]\n"));
		cfile.WriteString(strLine);

		strLine.Format(_T("  NUM[%d]\n"), 0);
		cfile.WriteString(strLine);

		strLine.Format(_T("[END]\n"));
		cfile.WriteString(strLine);

		//	ar.Close();
		cfile.Close();
	}
	else
	{
		CStdioFile File(m_strFilePath, CFile::modeRead);

		while (TRUE)
		{
			CString strLine;
			if (File.ReadString(strLine))
			{
				if (strLine == _T("[SERVO]"))
				{
					if (File.ReadString(strLine))
					{
						int n = _ttoi(GetDataFromString(strLine, _T("NUM["), _T("]")));
						for (int i = 0; i < n; i++)
						{
							if (File.ReadString(strLine))
							{
								int k = _ttoi(GetDataFromString(strLine, _T("N["), _T("]")));
								ASSERT(k == i + 1);

								CString str = GetDataFromString(strLine, _T("DIVISION["), _T("]"));

								str.TrimRight();

								int j = 0;
								if (!g_iMapAxis.Lookup(str, j))	ASSERT(FALSE);

								m_cAxis[j].m_strDivision = str;

								str = GetDataFromString(strLine, _T("NAME["), _T("]"));
								str.TrimRight();
								m_cAxis[j].m_strAxisName = str;

								m_cAxis[j].m_setting.bUse = _ttoi(GetDataFromString(strLine, _T("USE["), _T("]")));

								m_cAxis[j].m_iAxisMap = _ttoi(GetDataFromString(strLine, _T("HW["), _T("]")));
								SetAxisMapping(j, m_cAxis[j].m_iAxisMap);
								// LYS 2026-07-13 ACS controller mapping
								CString strACS = GetDataFromString(strLine, _T("ACS["), _T("]"));
								m_cAxis[j].m_iACSMap = strACS.IsEmpty() ? 0 : _ttoi(strACS);
								m_cAxis[j].m_iDir = (_ttoi(GetDataFromString(strLine, _T("DIR["), _T("]"))) > 0 ? 1 : -1);
								m_cAxis[j].m_setting.iClamp = (_ttoi(GetDataFromString(strLine, _T("CLAMP["), _T("]"))));
								m_cAxis[j].m_setting.iStepper = (_ttoi(GetDataFromString(strLine, _T("TYPE["), _T("]"))));
								m_cAxis[j].m_setting.iNumHomePLC = _ttoi(GetDataFromString(strLine, _T("HPLC["), _T("]")));

								m_cAxis[j].m_setting.dVelH = _ttof(GetDataFromString(strLine, _T("VH["), _T("]")));
								m_cAxis[j].m_setting.dVelM = _ttof(GetDataFromString(strLine, _T("VM["), _T("]")));
								m_cAxis[j].m_setting.dVelL = _ttof(GetDataFromString(strLine, _T("VL["), _T("]")));
								m_cAxis[j].m_setting.dVelHome = _ttof(GetDataFromString(strLine, _T("VO["), _T("]")));


								m_cAxis[j].m_setting.dAccH = _ttof(GetDataFromString(strLine, _T("AH["), _T("]")));
								m_cAxis[j].m_setting.dAccM = _ttof(GetDataFromString(strLine, _T("AM["), _T("]")));
								m_cAxis[j].m_setting.dAccL = _ttof(GetDataFromString(strLine, _T("AL["), _T("]")));


								m_cAxis[j].m_setting.dScvH = _ttof(GetDataFromString(strLine, _T("STH["), _T("]")));
								m_cAxis[j].m_setting.dScvM = _ttof(GetDataFromString(strLine, _T("STM["), _T("]")));
								m_cAxis[j].m_setting.dScvL = _ttof(GetDataFromString(strLine, _T("STL["), _T("]")));

                                m_cAxis[j].m_setting.dPulse = _ttof(GetDataFromString(strLine, _T("PLS["), _T("]")));     // jsyoon 2022.02.22 add

								m_cAxis[j].m_setting.dEncL = _ttof(GetDataFromString(strLine, _T("EL["), _T("]")));

								m_cAxis[j].m_setting.dDimCen = _ttof(GetDataFromString(strLine, _T("CN["), _T("]")));
								m_cAxis[j].m_setting.dDimMax = _ttof(GetDataFromString(strLine, _T("MX["), _T("]")));
								m_cAxis[j].m_setting.dDimMin = _ttof(GetDataFromString(strLine, _T("MN["), _T("]")));

								m_cAxis[j].m_setting.dOffset = _ttof(GetDataFromString(strLine, _T("OF["), _T("]")));
								m_cAxis[j].m_setting.dHomePos = _ttof(GetDataFromString(strLine, _T("HOME["), _T("]")));
							}
						}
					}
					continue;
				} // [SERVO]
				else if (strLine == _T("[BUFFERS]"))
				{
					if (File.ReadString(strLine))
					{
						int n = _ttoi(GetDataFromString(strLine, _T("NUM["), _T("]")));
						for (int i = 0; i < n; i++)
						{
							if (File.ReadString(strLine))
							{
								int j = _ttoi(GetDataFromString(strLine, _T("RUNBUFFER["), _T("]")));
								m_cAxis[j].m_setting.iNumHomePLC = _ttoi(GetDataFromString(strLine, _T("HOMEINDEX["), _T("]")));
							}
						}
					}

				}
				else if (strLine == _T("[OFFSET]"))
				{
					;
				}
				else if (strLine == _T("[END]"))
				{
					break;
				}
			}
			else
			{
				break;
			}
		}

		File.Close();
	}

    return TRUE;
}

UINT CMotorControl_ACS::ThreadRun()
{
    //-------------------------------------------------------------------------------
    // 시뮬레이션 환경에서는 데이터를 만들어서 보내준다.
    //-------------------------------------------------------------------------------

    return 0;
}

bool CMotorControl_ACS::InitDevice()
{
	m_bBdInstalled = false;

    int NConnections = 0;
    ACSC_CONNECTION_DESC Connections[10];
    acsc_GetConnectionsList(Connections, 10, &NConnections);
    for(int i = 0; i < NConnections; i++)
    {
   
		CString strAppName(Connections[i].Application);
//		CString strAppName;
   //     strAppName.Format(_T("%s"), Connections[i].Application);
        if( strAppName.Find(_T("ACS.")) >= 0)
        {
            continue;
        }
        else
        {
            acsc_TerminateConnection(&(Connections[i]));
        }
    }

    for (int iAxis = 0; iAxis < AXIS_TYPE_END; iAxis++ )
    {
        m_cAxis[iAxis].m_bHomeDone = false;
    }

    // LYS 2026-07-13 dual controller connection
    USES_CONVERSION;
    CString strAddressOld;
    strAddressOld.Empty();

    bool bRet = false;
    int nAcs = 0;
    do
    {
        CString strAddressCur = m_strAddress[nAcs];

        if (strAddressOld.CompareNoCase(strAddressCur) == 0)
            continue;

        m_hAcs[nAcs] = acsc_OpenCommEthernetTCP(T2A(strAddressCur), m_iPort);

        if (m_hAcs[nAcs] == ACSC_INVALID)
        {
            bRet = false;
        }
        else
        {
            bRet = true;
        }

        if (bRet != true)
            break;

        nAcs++;

        if (nAcs >= df_MAX_CTRL)
            break;

        strAddressOld = strAddressCur;

    } while (true);

	m_bBdInstalled = bRet;

    return bRet;
}

CString CMotorControl_ACS::GetDeviceName()
{
    return _T("ACS");
}

bool CMotorControl_ACS::IsConnected()
{
    return m_bBdInstalled;
}



long CMotorControl_ACS::ResetDevice(int iAxis)
{
    // LYS 2026-07-13 dual controller dispatch
    if(iAxis < 0)
    {
        // All-axis reset: dispatch per controller
        for (int ic = 0; ic < df_MAX_CTRL; ic++)
        {
            HANDLE hAcs = m_hAcs[ic];
            if (hAcs == ACSC_INVALID) continue;

            if (acsc_KillAll(hAcs, NULL) == 0) return -1;
            if (acsc_DisableAll(hAcs, NULL) == 0) return -1;
        }

        // Per-controller fault clear and enable
        for (int ic = 0; ic < df_MAX_CTRL; ic++)
        {
            HANDLE hAcs = m_hAcs[ic];
            if (hAcs == ACSC_INVALID) continue;

            int nAxes[200];
            int nCount = 0;
            for (int i = 0; i < df_USE_MAX_AXIS; i++)
            {
                if (m_cAxis[i].m_setting.bUse == FALSE) continue;
                if (m_cAxis[i].m_iACSMap != ic) continue;
                nAxes[nCount++] = m_cAxis[i].m_iAxisMap;
            }
            nAxes[nCount] = -1;
            if (nCount > 0)
            {
                acsc_FaultClearM(hAcs, nAxes, NULL);
                acsc_EnableM(hAcs, nAxes, NULL);
            }
        }
    }
    else
    {
        int iAcs_mapping = m_cAxis[iAxis].m_iACSMap;
        HANDLE hAcs = (iAcs_mapping < df_MAX_CTRL) ? m_hAcs[iAcs_mapping] : ACSC_INVALID;
        if (hAcs == ACSC_INVALID) return -1;

        int iAxis_mapping = m_cAxis[iAxis].m_iAxisMap;

        if (acsc_Kill(hAcs, iAxis_mapping, NULL) == 0) return -1;
        if (acsc_Disable(hAcs, iAxis_mapping, NULL) == 0) return -1;
        if (acsc_FaultClear(hAcs, iAxis_mapping, NULL) == 0) return -1;
        if (acsc_Enable(hAcs, iAxis_mapping, NULL) == 0) return -1;
        if (acsc_SetFPosition(hAcs, iAxis_mapping, 0, NULL) == 0) return -1;
    }

    return 0;
}

long CMotorControl_ACS::MoveHome(int iAxis)
{
    // LYS 2026-07-13 dual controller dispatch
    int iAcs_mapping = m_cAxis[iAxis].m_iACSMap;
    HANDLE hAcs = (iAcs_mapping < df_MAX_CTRL) ? m_hAcs[iAcs_mapping] : ACSC_INVALID;
    if (hAcs == ACSC_INVALID) return -1;

    m_cAxis[iAxis].m_bHomeDone = false;

    if(m_cAxis[iAxis].m_setting.bUse == FALSE)
        return 1;

    if(m_cAxis[iAxis].m_setting.iNumHomePLC < 0 )
        return -1;
	
    //Return Value
    //	If the function succeeds, the return value is non - zero.
    //	If the function fails, the return value is zero.
    return acsc_RunBuffer(hAcs, m_cAxis[iAxis].m_setting.iNumHomePLC, NULL, NULL);
}

long CMotorControl_ACS::MoveCmdPosA(int iAxis, double dPos, bool bBlockMode, int iSpeed, int iLens, int iTube)
{
    // LYS 2026-07-13 dual controller dispatch
    int iAcs_mapping = m_cAxis[iAxis].m_iACSMap;
    HANDLE hAcs = (iAcs_mapping < df_MAX_CTRL) ? m_hAcs[iAcs_mapping] : ACSC_INVALID;
    if (hAcs == ACSC_INVALID) return -1;
    if(m_cAxis[iAxis].m_setting.bUse == FALSE)
        return 0;
    if( GetMotorStatus(iAxis, STATE_MOT_DISABLE) != 0)
        return -1;

	if( iSpeed != -1)	SetSpeed(iAxis, iSpeed, 2);

    // Check if out of range
    if (dPos > m_cAxis[iAxis].m_setting.dDimMax || dPos < m_cAxis[iAxis].m_setting.dDimMin )
        return -1;

    // 	//get distance
    double dMovePos = dPos - m_cAxis[iAxis].m_setting.dHomePos;
    double dMovePos2 = dMovePos * m_cAxis[iAxis].m_iDir;


    // jsyoon 2022.02.22 change Move
    return MoveToPoint(iAxis, dMovePos2);

    //move method
	//Return Value
	//	If the function succeeds, the return value is non - zero.
	//	If the function fails, the return value is zero.
    //return acsc_ToPoint(hAcs, 0, m_cAxis[iAxis].m_iAxisMap, dMovePos2, NULL);
}

long CMotorControl_ACS::MoveCmdPosR(int iAxis, double dPos, bool bBlockMode, int iSpeed)
{
    // LYS 2026-07-13 dual controller dispatch
    int iAcs_mapping = m_cAxis[iAxis].m_iACSMap;
    HANDLE hAcs = (iAcs_mapping < df_MAX_CTRL) ? m_hAcs[iAcs_mapping] : ACSC_INVALID;
    if (hAcs == ACSC_INVALID) return -1;
    if( GetMotorStatus(iAxis, STATE_MOT_DISABLE) != 0)
        return -1;

	if (iSpeed != -1)	SetSpeed(iAxis, iSpeed, 2);

    double dCur = GetCurPos(iAxis, -1, -1) + dPos;

    // Check if out of range
    if (dCur > m_cAxis[iAxis].m_setting.dDimMax || dCur < m_cAxis[iAxis].m_setting.dDimMin)
        return -1;

    double dPos2 = dPos * m_cAxis[iAxis].m_iDir;


    // jsyoon 2022.02.22 change Move
    return MoveToPoint(iAxis, dPos2, ACSC_AMF_RELATIVE);

	//move method
	//Return Value
	//	If the function succeeds, the return value is non - zero.
	//	If the function fails, the return value is zero.
    //return acsc_ToPoint(hAcs, ACSC_AMF_RELATIVE, m_cAxis[iAxis].m_iAxisMap, dPos2, NULL);
}

long CMotorControl_ACS::MoveCmdPosMulti(stMotorPosition *pPos, int iAxis, int iSpeed)
{
    // LYS 2026-07-13 dual controller dispatch (use first axis controller)
    int iAcs_mapping = (iAxis > 0) ? m_cAxis[pPos[0].axis].m_iACSMap : 0;
    HANDLE hAcs = (iAcs_mapping < df_MAX_CTRL) ? m_hAcs[iAcs_mapping] : ACSC_INVALID;
    if (hAcs == ACSC_INVALID) return -1;

    // check amp enable
    for(int i = 0; i < iAxis; i++)
    {
        if( GetMotorStatus(pPos[i].axis, STATE_MOT_DISABLE) != 0)
            return pPos[i].axis;
    }

    int Axes[AXIS_TYPE_END];
    double Points[AXIS_TYPE_END];
	ZeroMemory(Axes, sizeof(Axes));
	ZeroMemory(Points, sizeof(Points));

    for (int i = 0; i < iAxis; i++)
    {
        // Check if out of range
        int nAxis   = pPos[i].axis;
        double dPos = pPos[i].dPosition;
        int iLens   = pPos[i].iLens;
        int iTube   = pPos[i].iTube;

        if( SetSpeed(nAxis, iSpeed, 2) == 0 )
        {
            if (dPos > m_cAxis[nAxis].m_setting.dDimMax || dPos < m_cAxis[nAxis].m_setting.dDimMin )
                return nAxis;

            double dMovePos = dPos - m_cAxis[nAxis].m_setting.dHomePos;
            double dMovePos2 = dMovePos * m_cAxis[nAxis].m_iDir;

            Axes[i]   = m_cAxis[nAxis].m_iAxisMap;


           // Points[i] = dMovePos2;
           // 
            // jsyoon 2022.02.22 add  Change Pulse
            double dPoint = 0;
            if (m_cAxis[nAxis].m_setting.iStepper == 2)	// LYS 2026-07-13 fix: iAxis -> nAxis
            {
                // Change Pulse
                dPoint = (m_cAxis[nAxis].m_setting.dPulse * dMovePos2) / m_cAxis[nAxis].m_setting.dEncL;
            }
            else
            {
                dPoint = dMovePos2;
            }

            Points[i] = dPoint;
        }
        else
        {
            return nAxis;
        }
    }

    Axes[iAxis] = -1;

    //move method
	//Return Value
	//	If the function succeeds, the return value is non - zero.
	//	If the function fails, the return value is zero.

	return acsc_ToPointM(hAcs, 0 | ACSC_AMF_MAXIMUM, Axes, Points, NULL);	// jsyoon 2020.04.15 change
 //   return acsc_ToPointM(hAcs, 0, Axes, Points, NULL);
}

long CMotorControl_ACS::MoveJog(int iAxis, bool bDir)
{
    // LYS 2026-07-13 dual controller dispatch
    int iAcs_mapping = m_cAxis[iAxis].m_iACSMap;
    HANDLE hAcs = (iAcs_mapping < df_MAX_CTRL) ? m_hAcs[iAcs_mapping] : ACSC_INVALID;
    if (hAcs == ACSC_INVALID) return -1;
    if( GetMotorStatus(iAxis, STATE_MOT_DISABLE) != 0)
        return -1;

    int bDirection = ACSC_POSITIVE_DIRECTION;

    if( m_cAxis[iAxis].m_iDir != 1 )
    {
        bDirection = ACSC_NEGATIVE_DIRECTION;
    }
    else
    {
        bDirection = ACSC_POSITIVE_DIRECTION;
    }

    //Return Value
    //	If the function succeeds, the return value is non - zero.
    //	If the function fails, the return value is zero.
    return acsc_Jog(hAcs, 0, m_cAxis[iAxis].m_iAxisMap, bDirection, NULL);
}

long CMotorControl_ACS::WaitMotionEnd(int iAxis, DWORD dwTimeout)
{
    // LYS 2026-07-13 dual controller dispatch
    int iAcs_mapping = m_cAxis[iAxis].m_iACSMap;
    HANDLE hAcs = (iAcs_mapping < df_MAX_CTRL) ? m_hAcs[iAcs_mapping] : ACSC_INVALID;
    if (hAcs == ACSC_INVALID) return -1;

    return acsc_WaitMotionEnd(hAcs, m_cAxis[iAxis].m_iAxisMap, dwTimeout);
}

bool CMotorControl_ACS::CheckMoveDone(int iAxis, DWORD dwTimeout)
{
    // LYS 2026-07-13 dual controller dispatch
    int iAcs_mapping = m_cAxis[iAxis].m_iACSMap;
    HANDLE hAcs = (iAcs_mapping < df_MAX_CTRL) ? m_hAcs[iAcs_mapping] : ACSC_INVALID;
    if (hAcs == ACSC_INVALID) return false;
    if(m_cAxis[iAxis].m_setting.bUse == FALSE)
        return true;
    if(GetMotorStatus(iAxis, STATE_MOT_DISABLE) != 0)
        return false;

    ULONGLONG dwStart = GetTickCount();

    do
    {
		if (dwTimeout > 100)
		{
			WaitForSingleObject(m_hWaitTimeEvent, 50);	// change 100 to 50
		}

		// jsyoon 2022.06.17 add
		if (GetMotorStatus(iAxis, STATE_MOT_DISABLE) != 0)
		{
			return false;
		}

		if (GetMotorStatus(iAxis, STATE_MOT_LMTNEG) != 0)
		{
			return false;
		}

		if (GetMotorStatus(iAxis, STATE_MOT_LMTPOS) != 0)
		{
			return false;
		}
			
			
        int iState = 0;
        if (acsc_GetMotorState(hAcs, m_cAxis[iAxis].m_iAxisMap, &iState, NULL) == 1)
        {
			int bMotorState_INPOS = iState & ACSC_MST_INPOS;	// 포지션 안에 들어왔는가?
			int bMotorState_MOVE = iState & ACSC_MST_MOVE;		// 움직이고 있는가?

            if((bMotorState_MOVE != ACSC_MST_MOVE) && (bMotorState_INPOS == ACSC_MST_INPOS))
            {
                return true;
            }
        }

		if ((GetTickCount() - dwStart) > dwTimeout * 1000)
		{
			return false;
		}
    }
    while(1);

    return false;
}

bool CMotorControl_ACS::CheckHomeDone(int iAxis, DWORD dwTimeout)
{
    if(m_cAxis[iAxis].m_setting.bUse == FALSE)
        return true;

	int iError = 0;

	DWORD dwStart = timeGetTime();
    do
    {
        WaitForSingleObject(m_hWaitTimeEvent, 500);

        if (iError == 20)		// 10sec
            break;

        if (GetMotorStatus(iAxis, STATE_MOT_DISABLE))
        {
            iError++;
        }
        else
        {
            iError = 0;
        }

        if(CheckHomeDoneLoop(iAxis) == true)
		{
			return true;
		}

		iError++;

        DWORD dwCurrent = timeGetTime();
        if ((dwCurrent - dwStart) > (dwTimeout * 1000))
		{
			break;
		}
    }
    while (1);

    return false;
}

long CMotorControl_ACS::CheckInterlock(CString strAxis, int iAcs_mapping)	// LYS 2026-07-13
{
	// LYS 2026-07-13 dual controller dispatch
	HANDLE hAcs = (iAcs_mapping < df_MAX_CTRL) ? m_hAcs[iAcs_mapping] : ACSC_INVALID;
	if (hAcs == ACSC_INVALID)
		return 0;

    int res = 0;

    USES_CONVERSION;
    acsc_ReadInteger(hAcs, ACSC_NONE, T2A(strAxis), 0, 0, ACSC_NONE, ACSC_NONE, &res, NULL);

    return res;
}

long CMotorControl_ACS::StopDevice(int iAxis)
{
    // LYS 2026-07-13 dual controller dispatch
    int iAcs_mapping = m_cAxis[iAxis].m_iACSMap;
    HANDLE hAcs = (iAcs_mapping < df_MAX_CTRL) ? m_hAcs[iAcs_mapping] : ACSC_INVALID;
    if (hAcs == ACSC_INVALID) return -1;

    if(m_cAxis[iAxis].m_setting.bUse == FALSE)
        return 0;

    // check amp enable
    if( GetMotorStatus(iAxis, STATE_MOT_DISABLE) != 0)
        return -1;

    return acsc_Halt(hAcs, m_cAxis[iAxis].m_iAxisMap, NULL);
}

long CMotorControl_ACS::KillDevice(int iAxis)
{
    // LYS 2026-07-13 dual controller dispatch
    if(iAxis < 0)
    {
        for (int ic = 0; ic < df_MAX_CTRL; ic++)
        {
            if (m_hAcs[ic] == ACSC_INVALID) continue;
            acsc_KillAll(m_hAcs[ic], NULL);
            acsc_DisableAll(m_hAcs[ic], NULL);
        }
        return 0;
    }

    int iAcs_mapping = m_cAxis[iAxis].m_iACSMap;
    HANDLE hAcs = (iAcs_mapping < df_MAX_CTRL) ? m_hAcs[iAcs_mapping] : ACSC_INVALID;
    if (hAcs == ACSC_INVALID) return -1;
    return acsc_Disable(hAcs, m_cAxis[iAxis].m_iAxisMap, NULL);
}

long CMotorControl_ACS::RunBuffer(int iAxis)
{
    // LYS 2026-07-13 dual controller dispatch
    int iAcs_mapping = m_cAxis[iAxis].m_iACSMap;
    HANDLE hAcs = (iAcs_mapping < df_MAX_CTRL) ? m_hAcs[iAcs_mapping] : ACSC_INVALID;
    if (hAcs == ACSC_INVALID) return -1;

    if(m_cAxis[iAxis].m_setting.bUse == FALSE)
        return 0;
    if(m_cAxis[iAxis].m_setting.iNumHomePLC < 0 )
        return -1;

    return acsc_RunBuffer(hAcs, m_cAxis[iAxis].m_setting.iNumHomePLC, NULL, NULL);
}

long CMotorControl_ACS::StopBuffer(int iAxis)
{
    // LYS 2026-07-13 dual controller dispatch
    int iAcs_mapping = m_cAxis[iAxis].m_iACSMap;
    HANDLE hAcs = (iAcs_mapping < df_MAX_CTRL) ? m_hAcs[iAcs_mapping] : ACSC_INVALID;
    if (hAcs == ACSC_INVALID) return -1;

    if(m_cAxis[iAxis].m_setting.bUse == FALSE)
        return 0;
    if(m_cAxis[iAxis].m_setting.iNumHomePLC < 0 )
        return -1;

    return acsc_StopBuffer(hAcs, m_cAxis[iAxis].m_setting.iNumHomePLC, NULL);
}


//Set parameter
long CMotorControl_ACS::SetSpeed(int iAxis, int iSpeedType, int iSpeedMode)
{
	// LYS 2026-07-13 dual controller dispatch
	int iAcs_mapping = m_cAxis[iAxis].m_iACSMap;
	HANDLE hAcs = (iAcs_mapping < df_MAX_CTRL) ? m_hAcs[iAcs_mapping] : ACSC_INVALID;
	if (hAcs == ACSC_INVALID) return -1;

    if(m_cAxis[iAxis].m_setting.bUse == FALSE)
        return 0;

    m_cAxis[iAxis].SetSpeed(iSpeedType);

    return SetSpeedDirect(iAxis, m_cAxis[iAxis].m_dVel, m_cAxis[iAxis].m_dAccT, m_cAxis[iAxis].m_dScvT, iSpeedMode);
}

long CMotorControl_ACS::SetSpeedDirect(int iAxis, double dVel, double dAcc, double dDec, int iSpeedMode)
{
    // LYS 2026-07-13 dual controller dispatch
    int iAcs_mapping_sd = m_cAxis[iAxis].m_iACSMap;
    HANDLE hAcs = (iAcs_mapping_sd < df_MAX_CTRL) ? m_hAcs[iAcs_mapping_sd] : ACSC_INVALID;
    if (hAcs == ACSC_INVALID) return -1;

	if( dVel < 0.01 || dAcc < 0.01 || dDec < 0.01 )
		return -1;

    int bRet = 0;
    int iAxis_mapping = m_cAxis[iAxis].m_iAxisMap;


	double dSetVel = 0,  dSetAcc = 0,  dSetJerk = 0;

	// jsyoon 2022.02.22 add
	if (m_cAxis[iAxis].m_setting.iStepper == 2)
	{
		// Change Pulse
		dSetVel = (m_cAxis[iAxis].m_setting.dPulse * dVel) / m_cAxis[iAxis].m_setting.dEncL;
		dSetAcc = (m_cAxis[iAxis].m_setting.dPulse * dAcc) / m_cAxis[iAxis].m_setting.dEncL;
		dSetJerk = (m_cAxis[iAxis].m_setting.dPulse * dDec) / m_cAxis[iAxis].m_setting.dEncL;
	}
	else
	{
		dSetVel = dVel;
		dSetAcc = dAcc;
		dSetJerk = dDec;
	}

    if( iAxis == AXIS_UNIT_Z || iAxis == AXIS_Z9 )
    {
        for (int i = iAxis_mapping; i <= iAxis_mapping + 1; i++)
        {
            if(!acsc_SetVelocity(hAcs, i, dSetVel,	NULL))	bRet &= acsc_GetLastError();
            if(!acsc_SetAcceleration(hAcs, i, dSetAcc,	NULL))	bRet &= acsc_GetLastError();
            if(!acsc_SetDeceleration(hAcs, i, dSetAcc,	NULL))	bRet &= acsc_GetLastError();
            if(!acsc_SetKillDeceleration(hAcs, i, dSetJerk,	NULL))	bRet &= acsc_GetLastError();
            if(!acsc_SetJerk(hAcs, i, dSetJerk,	NULL))	bRet &= acsc_GetLastError();
        }
    }
    else
    {
        if(!acsc_SetVelocity(hAcs, iAxis_mapping, dSetVel,	NULL))	bRet &= acsc_GetLastError();
        if(!acsc_SetAcceleration(hAcs, iAxis_mapping, dSetAcc,	NULL))	bRet &= acsc_GetLastError();
        if(!acsc_SetDeceleration(hAcs, iAxis_mapping, dSetAcc,	NULL))	bRet &= acsc_GetLastError();
        if(!acsc_SetKillDeceleration(hAcs, iAxis_mapping, dSetJerk,	NULL))	bRet &= acsc_GetLastError();
        if(!acsc_SetJerk(hAcs, iAxis_mapping, dSetJerk,	NULL))	bRet &= acsc_GetLastError();
    }

    return bRet;
}

long CMotorControl_ACS::SetAmpEnable(int iAxis, bool bOn)
{
    if(bOn == true)
    {
        ResetDevice(iAxis);
    }
    else
    {
        KillDevice(iAxis);

        m_cAxis[iAxis].m_bHomeDone = false;
    }

    return GetMotorStatus(iAxis, STATE_MOT_ENABLE);
}

long CMotorControl_ACS::SetWriteBuffer(CString strEQPName, bool bDirectOut, int iValue)
{
    // LYS 2026-07-13 dual controller: use controller 0
    HANDLE hAcs = m_hAcs[0];
    if (hAcs == ACSC_INVALID) return -1;

	USES_CONVERSION;
    return acsc_WriteInteger(hAcs, ACSC_NONE, T2A(strEQPName), 0, 0, ACSC_NONE, ACSC_NONE, &iValue, NULL);
}



//Get parameter
double CMotorControl_ACS::GetCurPos(int iAxis, int iLens, int iTube)
{
    double dFPosition = 0, dPos = 0.0;
    // LYS 2026-07-13 dual controller dispatch
    int iAcs_mapping = m_cAxis[iAxis].m_iACSMap;
    HANDLE hAcs = (iAcs_mapping < df_MAX_CTRL) ? m_hAcs[iAcs_mapping] : ACSC_INVALID;
    if (hAcs == ACSC_INVALID) return dPos;

    if (!acsc_GetFPosition(hAcs, m_cAxis[iAxis].m_iAxisMap, &dFPosition, NULL))
    {
        int res = acsc_GetLastError();
        return dFPosition;
    }

    // jsyoon 2022.02.22 add 
    if (m_cAxis[iAxis].m_setting.iStepper == 2)
    {
        // Change Pulse
        dPos = (m_cAxis[iAxis].m_setting.dEncL * dFPosition) / m_cAxis[iAxis].m_setting.dPulse;
    }
    else
    {
        dPos = dFPosition;
    }

    double dResult = dPos * m_cAxis[iAxis].m_iDir;
    double dResult2 = m_cAxis[iAxis].m_setting.dHomePos + dResult;

	//if (dResult2 < 0.00001 && dResult2 > -0.001)
	//	dResult2 = 0.0;

    m_cAxis[iAxis].m_dPos = dResult2;

    return dResult2;
}

long CMotorControl_ACS::GetMotorStatus(int iAxis, int nStatus)
{
    // LYS 2026-07-13 dual controller dispatch
    int iAcs_mapping = m_cAxis[iAxis].m_iACSMap;
    HANDLE hAcs = (iAcs_mapping < df_MAX_CTRL) ? m_hAcs[iAcs_mapping] : ACSC_INVALID;
    if (hAcs == ACSC_INVALID) return -1;

    if(m_cAxis[iAxis].m_setting.bUse == FALSE)
        return 1;

    int iMotState = 0, iMotLimit = 0;
    int iAxis_mapping = m_cAxis[iAxis].m_iAxisMap;

    if( iAxis < AXIS_TYPE_END )
    {
        if(nStatus < STATE_MOT_LMTNEG )
        {
            iMotState = 0;
            if (acsc_GetMotorState(hAcs, iAxis_mapping, &iMotState, NULL) == 0 )
            {
                return -1;
            }
        }
        else
        {
            iMotLimit = 0;
            if (acsc_GetFault(hAcs, iAxis_mapping, &iMotLimit, NULL) == 0 )
            {
                TRACE("acsc_GetFault error: %d\n", acsc_GetLastError());
                return -1;
            }

            int iMotErr = 0;
            acsc_GetMotorError(hAcs, iAxis_mapping, &iMotErr, NULL);
            if (iMotErr > 0)
            {
                //char ErrorStr[256];
                //ErrorStr[0] = '\0';
                //int Received = 0;
                //if (acsc_GetErrorString(hAcs, iMotErr, ErrorStr, 255, &Received))
                //{
                //	ErrorStr[Received] = '\0';
                //}

                return iMotErr;
            }
        }
    }
    else
    {
        int res = 0;

        if(nStatus == STATE_MOT_HOMDONE)
        {
			int iHomeFlag[64];
			CString strHome;
			strHome.Format(_T("HomeFlag"));

			USES_CONVERSION;
			acsc_ReadInteger(hAcs, ACSC_NONE, T2A(strHome), 0, 64, ACSC_NONE, ACSC_NONE, iHomeFlag, NULL);

			int iNumHome = 0;
			iNumHome = m_cAxis[iAxis].m_setting.iNumHomePLC;
			res = iHomeFlag[iNumHome];

            /*
			double AnalogInPut = 0.0;
            CString strHome;
            strHome.Format(_T("Home_%02d"), m_cAxis[iAxis].m_setting.iNumHomePLC);
            USES_CONVERSION;
            acsc_ReadReal(hAcs, ACSC_NONE, T2A(strHome), 0, 0, ACSC_NONE, ACSC_NONE, &AnalogInPut, NULL);
            res = (int)AnalogInPut;
			*/
        }

        return res;
    }

    int res = 0;
    switch(nStatus)
    {
        case STATE_MOT_ENABLE:
        {
            if(iMotState & ACSC_MST_ENABLE)
                res = TRUE;
            else
                res = FALSE;
        }
        break;	// Amp Enable

        case STATE_MOT_DISABLE:
        {
            if(iMotState & ACSC_MST_ENABLE)
                res = FALSE;
            else
                res = TRUE;
        }
        break;

        case STATE_MOT_STOPMOT:
        {
            if(iMotState & ACSC_MST_MOVE)
                res = FALSE;
            else
                res = TRUE;
        }
        break;

        case STATE_MOT_LMTNEG:
        {
            if(iMotLimit & ACSC_SAFETY_LL)
                res = TRUE;
            else
                res = FALSE;
        }
        break;

        case STATE_MOT_LMTPOS:
        {
            if(iMotLimit & ACSC_SAFETY_RL)
                res = TRUE;
            else
                res = FALSE;
        }
        break;

        case STATE_MOT_ERRFLOW:
        {
            if(iMotLimit & ACSC_SAFETY_CPE)
                res = TRUE;
            else
                res = FALSE;
        }
        break;

        case STATE_MOT_INPOSIT:
        {
            if(iMotState & ACSC_MST_INPOS)
                res = TRUE;
            else
                res = FALSE;
        }
        break;

        case STATE_MOT_HOMDONE:
        {

        }
        break;
    }

    return res;
}

double CMotorControl_ACS::GetSpeedVal(int iAxis)
{
    double dSpeed = 0.0;
    // LYS 2026-07-13 dual controller dispatch
    int iAcs_mapping = m_cAxis[iAxis].m_iACSMap;
    HANDLE hAcs = (iAcs_mapping < df_MAX_CTRL) ? m_hAcs[iAcs_mapping] : ACSC_INVALID;
    if (hAcs == ACSC_INVALID) return dSpeed;

    if(m_cAxis[iAxis].m_setting.bUse == FALSE)
        return dSpeed;

    if(!acsc_GetVelocity(hAcs, m_cAxis[iAxis].m_iAxisMap, &dSpeed, NULL))
    {
        int res = acsc_GetLastError();
    }

    return dSpeed;
}

double CMotorControl_ACS::GetSpeed(int iAxis, long iSpeed)
{
    double dRet = 0.0;

    if(m_cAxis[iAxis].m_setting.bUse == FALSE)
        return dRet;

    return m_cAxis[iAxis].GetSpeed(iSpeed);
}

long CMotorControl_ACS::GetAmpEnable(int iAxis)
{
    // LYS 2026-07-13 dual controller dispatch
    int iAcs_mapping = m_cAxis[iAxis].m_iACSMap;
    HANDLE hAcs = (iAcs_mapping < df_MAX_CTRL) ? m_hAcs[iAcs_mapping] : ACSC_INVALID;
    if (hAcs == ACSC_INVALID) return -1;

    if(m_cAxis[iAxis].m_setting.bUse == FALSE)
        return 0;

	int iState = 0;
	iState = GetMotorStatus(iAxis, STATE_MOT_ENABLE);

  /*  if (!acsc_GetMotorState(hAcs, m_cAxis[iAxis].m_iAxisMap, &iState, NULL))
    {
        int res = acsc_GetLastError();
    }*/

    return iState;
}

long CMotorControl_ACS::GetHWLimit(int iAxis, short bDir)
{
    // LYS 2026-07-13 dual controller dispatch
    int iAcs_mapping = m_cAxis[iAxis].m_iACSMap;
    HANDLE hAcs = (iAcs_mapping < df_MAX_CTRL) ? m_hAcs[iAcs_mapping] : ACSC_INVALID;
    if (hAcs == ACSC_INVALID) return -1;

	bool bRet = false;

    int	iState	= 0;
    if ( !acsc_GetFault(hAcs, m_cAxis[iAxis].m_iAxisMap, &iState, NULL ) )
    {
        int res	= acsc_GetLastError();
		bRet &= false;
	}
    else
    {
        if( bDir )		// +
        {
        	if(iState & ACSC_SAFETY_RL)
        	{
				bRet &= false;
        	}
        	else
        	{
				bRet = true;
        	}
        }
        else
        {
        	if(iState & ACSC_SAFETY_LL)
        	{
				bRet &= false;
        	}
        	else
        	{
				bRet = true;
        	}
        }
    }

    return (long)bRet;
}

long CMotorControl_ACS::GetMoving(int iAxis)
{
    // LYS 2026-07-13 dual controller dispatch
    int iAcs_mapping = m_cAxis[iAxis].m_iACSMap;
    HANDLE hAcs = (iAcs_mapping < df_MAX_CTRL) ? m_hAcs[iAcs_mapping] : ACSC_INVALID;
    if (hAcs == ACSC_INVALID) return -1;

    int iState = 0;
    if (!acsc_GetMotorState(hAcs, m_cAxis[iAxis].m_iAxisMap, &iState, NULL))
    {
        int res = acsc_GetLastError();
    }

    return iState;
}

bool CMotorControl_ACS::GetHomeDone(int iAxis)
{
    if(m_cAxis[iAxis].m_setting.bUse == FALSE)
        return true;

    return m_cAxis[iAxis].m_bHomeDone;
}

double CMotorControl_ACS::GetMotorRMSValue(int iAxis)
{
    double dRMSValue = 0.0;
    // LYS 2026-07-13 dual controller dispatch
    int iAcs_mapping = m_cAxis[iAxis].m_iACSMap;
    HANDLE hAcs = (iAcs_mapping < df_MAX_CTRL) ? m_hAcs[iAcs_mapping] : ACSC_INVALID;
    if (hAcs == ACSC_INVALID) return dRMSValue;

    if(m_cAxis[iAxis].m_setting.bUse == FALSE)
        return dRMSValue;

    acsc_ReadReal(hAcs, ACSC_NONE, "RMS", m_cAxis[iAxis].m_iAxisMap, m_cAxis[iAxis].m_iAxisMap, ACSC_NONE, ACSC_NONE, &dRMSValue, NULL);

    return dRMSValue;
}

CString	CMotorControl_ACS::GetLastErrorString()
{
	// LYS 2026-07-13 dual controller: use controller 0
	HANDLE hAcs = m_hAcs[0];
	if (hAcs == ACSC_INVALID)
		return _T("");

	CString strLastErrorString;

	char ErrorStr[256];	
	int Received = 0, ErrorCode = acsc_GetLastError();
	if (acsc_GetErrorString(hAcs,				// communication handle
							ErrorCode, 			// error code
							ErrorStr,			// buffer for the error explanation
							256,				// available buffer length
							&Received) == 1)	// number of actually received bytes
	{
		ErrorStr[Received] = '\0';

		strLastErrorString.Format(_T("function returned error: %d (%s)\n"), ErrorCode, ErrorStr);
	}

	return strLastErrorString;
}

long CMotorControl_ACS::GetInput(int port, int bit, int iAcs_mapping)	// LYS 2026-07-13
{
    // LYS 2026-07-13 dual controller dispatch
    HANDLE hAcs = (iAcs_mapping < df_MAX_CTRL) ? m_hAcs[iAcs_mapping] : ACSC_INVALID;
    if (hAcs == ACSC_INVALID) return -1;

    int iValue = 0;
    if( !acsc_GetInput( hAcs, port, bit, &iValue, NULL) )
    {
        int res = acsc_GetLastError();
    }

    return iValue;
}

double CMotorControl_ACS::GetFollowingErr(int iAxis)
{
    double dRes = 0.0;
    // LYS 2026-07-13 dual controller dispatch
    int iAcs_mapping = m_cAxis[iAxis].m_iACSMap;
    HANDLE hAcs = (iAcs_mapping < df_MAX_CTRL) ? m_hAcs[iAcs_mapping] : ACSC_INVALID;
    if (hAcs == ACSC_INVALID) return dRes;
    if(m_cAxis[iAxis].m_setting.bUse == FALSE)
        return dRes;

    acsc_ReadReal( hAcs, ACSC_NONE, "FPOS", m_cAxis[iAxis].m_iAxisMap, m_cAxis[iAxis].m_iAxisMap, ACSC_NONE, ACSC_NONE, &dRes, NULL );//jw.kim 2010.11.04

    return dRes;
}



double CMotorControl_ACS::GetActualVel(int iAxis)
{
    // LYS 2026-07-13 dual controller dispatch
    int iAcs_mapping = m_cAxis[iAxis].m_iACSMap;
    HANDLE hAcs = (iAcs_mapping < df_MAX_CTRL) ? m_hAcs[iAcs_mapping] : ACSC_INVALID;
    if (hAcs == ACSC_INVALID) return -1;
    if(m_cAxis[iAxis].m_iAxisMap < 0)
        return 0;

    double FVelocity = 0.0;

    if ( acsc_GetRVelocity(hAcs, m_cAxis[iAxis].m_iAxisMap, &FVelocity, NULL) == 0 )
    {
        int res = acsc_GetLastError();
    }

    return FVelocity;
}

CString CMotorControl_ACS::GetDataFromString(CString inStr, CString startStr, CString endStr)
{
    int iStart = inStr.Find(startStr);
    if( iStart == -1)
        return _T("");

    int iEnd = inStr.Find(endStr, iStart);
    if( iEnd == -1)
        return _T("");

    if (iStart < iEnd)
    {
        return inStr.Mid(iStart + startStr.GetLength(), iEnd - iStart - startStr.GetLength());
    }

    return _T("");
}

bool CMotorControl_ACS::CheckHomeDoneLoop(int iAxis)
{
	// LYS 2026-07-13 dual controller dispatch
	int iAcs_mapping = m_cAxis[iAxis].m_iACSMap;
	HANDLE hAcs = (iAcs_mapping < df_MAX_CTRL) ? m_hAcs[iAcs_mapping] : ACSC_INVALID;
	if (hAcs == ACSC_INVALID) return false;

 //   CString strHome;
 //   strHome.Format(_T("Home_%02d"), m_cAxis[iAxis].m_setting.iNumHomePLC);

   int iAxis_mapping = m_cAxis[iAxis].m_iAxisMap;

	//// Homming Buffer 가 동작을 완료 하였는가?
    int AnalogInPut = 0;
 //   USES_CONVERSION;
 //   acsc_ReadInteger(hAcs, ACSC_NONE, T2A(strHome), 0, 0, ACSC_NONE, ACSC_NONE, &AnalogInPut, NULL);

	int iHomeFlag[64];
	ZeroMemory(iHomeFlag, sizeof(iHomeFlag));
	CString strHome;
	strHome.Format(_T("HomeFlag"));

	USES_CONVERSION;
	acsc_ReadInteger(hAcs, ACSC_NONE, T2A(strHome), 0, 63, ACSC_NONE, ACSC_NONE, iHomeFlag, NULL);

	int iNumHome = 0;
	iNumHome = m_cAxis[iAxis].m_setting.iNumHomePLC;
	AnalogInPut = iHomeFlag[iNumHome];

	// 모터 상태가 동작을 완료 하였는가?
    int iState = 0;
    acsc_GetMotorState(hAcs, iAxis_mapping, &iState, NULL);
    Sleep(10);

	// 모터가 멈춰 있는가?
    int iActualVel = (int)GetActualVel(iAxis);

	//homing complete
    if( (AnalogInPut == 1) && (iState & ACSC_MST_INPOS) && (iActualVel == 0) )
    {
        Sleep(500);

		if (m_cAxis[iAxis].m_setting.iClamp == 0)	// 호밍후 위치값 0로 셋팅 안함
		{
			m_cAxis[iAxis].m_bHomeDone = true;

			return true;
		}
		else
		{
			double dOffset = m_cAxis[iAxis].m_setting.dOffset; 
			int iOffset = (int)dOffset;

			if (0 < fabs(dOffset))
			{
				if (fabs(dOffset) < 1)	SetSpeed(iAxis, SPEED_LOW, 2);
				else					SetSpeed(iAxis, SPEED_MID, 2);

				// jsyoon 2022.02.22 change 
				//    if(acsc_ToPoint(hAcs, 0, iAxis_mapping, dOffset, NULL) == 1)
				if (MoveToPoint(iAxis, dOffset) == 1)
				{
					Sleep(1000);

					if (CheckMoveDone(iAxis, 120) == true)
					{
						Sleep(500);

						if (acsc_SetFPosition(hAcs, iAxis_mapping, 0.0, NULL) == 1)
						{
							m_cAxis[iAxis].m_bHomeDone = true;

							if (iAxis == AXIS_LP)
							{
								m_cAxis[AXIS_LP2].m_bHomeDone = true;
							}

							return true;
						}
					}
				}
			}
			else
			{
				if (acsc_SetFPosition(hAcs, iAxis_mapping, 0.0, NULL) == 1)
				{
					m_cAxis[iAxis].m_bHomeDone = true;

					if (iAxis == AXIS_LP)
					{
						m_cAxis[AXIS_LP2].m_bHomeDone = true;
					}

					return true;
				}
			}
		}

		
    }

	return false;
}

void CMotorControl_ACS::SetMaxSWLimit(int iAxis, double dLimit)
{
	m_cAxis[iAxis].m_setting.dDimMax = dLimit;
}

void CMotorControl_ACS::SetMinSWLimit(int iAxis, double dLimit)
{
	m_cAxis[iAxis].m_setting.dDimMin = dLimit;
}

double CMotorControl_ACS::GetHomeVel(int iAxis)
{
	return m_cAxis[iAxis].m_setting.dHomePos;
}

int CMotorControl_ACS::MoveToPoint(int iAxis, double dPoint, int iFlag)
{
    int nRet = 1;

    // LYS 2026-07-13 dual controller dispatch
    int iAcs_mapping = m_cAxis[iAxis].m_iACSMap;
    HANDLE hAcs = (iAcs_mapping < df_MAX_CTRL) ? m_hAcs[iAcs_mapping] : ACSC_INVALID;
    if (hAcs == ACSC_INVALID)
    {
        nRet = 0;
    }
    else
    {
        // jsyoon 2022.02.22 add
        if (m_cAxis[iAxis].m_setting.iStepper == 2)
        {
            // Change Pulse
            dPoint = (m_cAxis[iAxis].m_setting.dPulse * dPoint) / m_cAxis[iAxis].m_setting.dEncL;
        }
        else
        {
            dPoint = dPoint;
        }

        int nAxis = m_cAxis[iAxis].m_iAxisMap;

        //move method
        //Return Value
        //	If the function succeeds, the return value is non - zero.
        //	If the function fails, the return value is zero.
        nRet = acsc_ToPoint(hAcs, iFlag, nAxis, dPoint, NULL);
    }
        
    return nRet;

}