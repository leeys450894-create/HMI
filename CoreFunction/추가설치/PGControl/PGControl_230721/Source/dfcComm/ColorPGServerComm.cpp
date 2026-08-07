#include "pch.h"
#include "dfcComm.h"
#include "ColorPGServerComm.h"
#include "crc16.h"

unsigned char HexToByte(char hex)
{
	switch (hex)
	{
	case '0': return 0;
	case '1': return 1;
	case '2': return 2;
	case '3': return 3;
	case '4': return 4;
	case '5': return 5;
	case '6': return 6;
	case '7': return 7;
	case '8': return 8;
	case '9': return 9;
	case 'A': case 'a': return 10;
	case 'B': case 'b': return 11;
	case 'C': case 'c': return 12;
	case 'D': case 'd': return 13;
	case 'E': case 'e': return 14;
	case 'F': case 'f': return 15;
	}
	return 32;
}

C_ColorPGServerComm::C_ColorPGServerComm (COLOR_PG_EVENT_HANDLER pHandler, CString IniPath, int nIndex)
{
	m_nRunning	  = COMM_RS_FINISH;
	m_pHandler	  = pHandler;
	m_nIndex	  = nIndex;			//통신 개수 따라 증가 Ini파일 접근시만 사용
	m_path		  = IniPath;

	m_nPatternNum	 = 0;
	m_strPatternName = _T("A");

	memset (m_bWaitItem, FALSE, sizeof (BOOL) * CPG_WAIT_MAX);

	ReadIniInfo ();

	if (GetCommUse ()) ConnectCtrl (TRUE);
}

C_ColorPGServerComm::~C_ColorPGServerComm ()
{

}

void	C_ColorPGServerComm::ConnectCtrl (BOOL bConnect)
{
	C_ServerSocket::Close ();

	Create (m_sInfo.m_nPortNum);
}

void	C_ColorPGServerComm::ReadIniInfo ()
{
	m_sInfo.m_bUse			= GetProfileInt		(_T("TCPIP"), dfcStr (_T("TCPIP_%d_USE"),				m_nIndex), FALSE);
	m_sInfo.m_bOutOfService = GetProfileInt		(_T("TCPIP"), dfcStr (_T("TCPIP_%d_OUT_OF_SERVICE"),	m_nIndex), TRUE);
	m_sInfo.m_strName		= GetProfileString	(_T("TCPIP"), dfcStr (_T("TCPIP_%d_NAME"),				m_nIndex), _T("ColorPG(S)"));
	m_sInfo.m_nHead			= GetProfileInt		(_T("TCPIP"), dfcStr (_T("TCPIP_%d_HEAD_NUM"),			m_nIndex), 1);
	m_sInfo.m_strIP			= GetProfileString	(_T("TCPIP"), dfcStr (_T("TCPIP_%d_IP"),				m_nIndex), _T("192.168.0.1"));
	m_sInfo.m_nPortNum		= GetProfileInt		(_T("TCPIP"), dfcStr (_T("TCPIP_%d_PORT"),				m_nIndex), 4001);
	m_sInfo.m_strResultDir  = GetProfileString	(_T("TCPIP"), dfcStr (_T("TCPIP_%d_RESULT_DIRECTORY"),	m_nIndex), _T("Z:\\"));
	m_sInfo.m_strRecipeFile = GetProfileString	(_T("TCPIP"), dfcStr (_T("TCPIP_%d_RECIPE_FILE"),		m_nIndex), _T("Z:\\Recipe.ini"));
}

void	C_ColorPGServerComm::WriteIniInfo (S_TcpIpInfo *sInfo)
{
	WriteProfileInt		(_T("TCPIP"), dfcStr(_T("TCPIP_%d_USE"),				m_nIndex), sInfo->m_bUse);
	WriteProfileInt		(_T("TCPIP"), dfcStr(_T("TCPIP_%d_OUT_OF_SERVICE"),		m_nIndex), sInfo->m_bOutOfService);
	WriteProfileString	(_T("TCPIP"), dfcStr(_T("TCPIP_%d_NAME"),				m_nIndex), sInfo->m_strName);
	WriteProfileInt		(_T("TCPIP"), dfcStr(_T("TCPIP_%d_HEAD_NUM"),			m_nIndex), sInfo->m_nHead);
	WriteProfileString	(_T("TCPIP"), dfcStr(_T("TCPIP_%d_IP"),					m_nIndex), sInfo->m_strIP);
	WriteProfileInt		(_T("TCPIP"), dfcStr(_T("TCPIP_%d_PORT"),				m_nIndex), sInfo->m_nPortNum);
	WriteProfileString	(_T("TCPIP"), dfcStr(_T("TCPIP_%d_RESULT_DIRECTORY"),	m_nIndex), sInfo->m_strResultDir);
	WriteProfileString	(_T("TCPIP"), dfcStr(_T("TCPIP_%d_RECIPE_FILE"),		m_nIndex), sInfo->m_strRecipeFile);

	ReadIniInfo ();
}

void	C_ColorPGServerComm::SetWaitItem (int nItem, BOOL bSet)
{
	if (nItem == -1) {
		for (int i = 0; i < CPG_WAIT_MAX; i++) m_bWaitItem[i] = bSet;
	}
	else
	{
		m_bWaitItem[nItem] = bSet;
	}
}

BOOL	C_ColorPGServerComm::GetWaitItem (int nItem)
{
	return m_bWaitItem[nItem];
}

//////////////////////////////////////////////////////
BOOL	C_ColorPGServerComm::SetPatternAck ()
{
	return SendItem (PATTERN_ACK);//2006
}

BOOL	C_ColorPGServerComm::SetPatternEnd (bool bOk)
{
	if( bOk == true)
		return SendItem (PATTERN_END);//2007
	else
		return SendItem(PATTERN_NG);//2007
}

BOOL	C_ColorPGServerComm::SendItem (int nItem)
{
	if (!IsConnected ()) return FALSE;

	const int nBufferSize = 1000;
	TCHAR	szTmp[nBufferSize];
	TCHAR	szData[nBufferSize];
	memset (szTmp, 0x00, sizeof (TCHAR) * nBufferSize);
	memset (szData,0x00, sizeof (TCHAR) * nBufferSize);

	CString strCmd, strCode, strMsg;
	int nSize = 0;

	switch (nItem) {
		case PATTERN_ACK :
			strCode = _T("2006");
			strMsg.Format (_T("ACK,PTN,%d"), m_nPatternNum);
		break;
		
		case PATTERN_END:
			strCode = _T("2007");
			strMsg.Format (_T("PTN_ON,%d,%s,GOOD"), m_nPatternNum, m_strPatternName);
		break;

		case PATTERN_NG:
			strCode = _T("2007");
			strMsg.Format(_T("PTN_ON,%d,%s,NG"), m_nPatternNum, m_strPatternName);
		break;
	}

	nSize = strMsg.GetLength ();
	// Code
	_stprintf_s (szTmp, _countof(szTmp), _T("%s"), strCode);	_tcscat_s (szData, _countof(szData), szTmp);
	// Len
	_stprintf_s (szTmp, _countof(szTmp), _T("%04X"), nSize);	_tcscat_s (szData, _countof(szData), szTmp);
	// Data
	_stprintf_s (szTmp, _countof(szTmp), _T("%s"), strMsg);		_tcscat_s (szData, _countof(szData), szTmp);

	unsigned short crc16;
	nSize = _tcsclen(szData);
	crc16 = crc_xmodem((unsigned char*)szData, nSize);

	strCmd.AppendFormat (_T("%c"),   0x02); //STX
	strCmd.AppendFormat (_T("%c"),   0x01); //Begin
	strCmd.AppendFormat (_T("%s"),	 szData);
	strCmd.AppendFormat (_T("%04X"), crc16);
	strCmd.AppendFormat (_T("%c"),   0x03); //ETX

	WriteComm (strCmd);

	return TRUE;
}
////////////////////////////////////////////////////RECEIVE
void	C_ColorPGServerComm::ReceiveCode (TCHAR* pBuf)
{

	CString strData(pBuf);
	strData.Replace(_T(" "), _T(""));

	int nSize = 0;
	CStringArray strArray;
	strArray.RemoveAll();
	nSize = GetExeHextoStr(strData, strArray);

//	if( strRcv.Find(_T("CONNECT_ALL_CAM") > 0 ))
//		return;

//	CString strRcv = pBuf;
	CString strCommand;
	CString sCmd, sLen, sData;
	int nLen = 0;

	for (size_t i = 0; i < nSize; i++)
	{
		strCommand = strArray.GetAt(i);
		TCHAR c = (TCHAR)strCommand.GetAt(1);

		if (c != 0x08)
		{
			continue;
		}

		CString strMsg = dfcStr(_T("ME ◀ [%s%d] : %s"), m_sInfo.m_strName, m_sInfo.m_nHead, strCommand);
		if (m_pHandler) m_pHandler(strMsg.GetBuffer(), EVENT_COMM_MSG, NULL);

		sCmd = strCommand.Mid(2, 4);
		sLen = strCommand.Mid(6, 4);

		for (size_t i = 0; i < 4; i++)
		{
			if (i == 0)
				nLen += HexToByte(sLen.GetAt(i)) * (16 * 16 * 16);
			else if (i == 1)
				nLen += HexToByte(sLen.GetAt(i)) * (16 * 16);
			else if (i == 2)
				nLen += HexToByte(sLen.GetAt(i)) * (16 * 1);
			else
				nLen += HexToByte(sLen.GetAt(i));
		}

		sData = strCommand.Mid(10, nLen);

		if (sCmd.Find(_T("2005")) == 0) { //Pattern 변경
			CStringArray strArrTest;
			dfcTokenize(sData, _T(","), strArrTest);

			if (strArrTest.GetSize() == 3) {
				m_nPatternNum = _ttoi(strArrTest.GetAt(1).GetString());
				m_strPatternName = strArrTest.GetAt(2).GetString();
			}

			SetPatternAck();
			 
		/*	if(m_strPatternName.Find(_T("WHITE")) == 0) 
				SetWaitItem(CPG_WAIT_TURN_ON, TRUE);
			else*/
				SetWaitItem(CPG_WAIT_NEXT, TRUE);

		}
		else if (sCmd.Find(_T("2008")) == 0) { //Pattern End Ack

		}

		else if (sCmd.Find(_T("2004")) == 0) { //Start
			SetWaitItem(CPG_WAIT_TURN_OFF, TRUE);

			if (m_pHandler) m_pHandler(strCommand.GetBuffer(), EVENT_COOM_PG_COLOR_CMD, NULL);
		}

		else if (sCmd.Find(_T("2009")) == 0) { //Pattern OFF
			SetWaitItem(CPG_WAIT_TURN_OFF, TRUE);

			if (m_pHandler) m_pHandler(strCommand.GetBuffer(), EVENT_COOM_PG_COLOR_CMD, NULL);
		}

		else {
			if (m_pHandler) m_pHandler(strCommand.GetBuffer(), EVENT_COOM_PG_COLOR_CMD, NULL);
		}
	}
}

void	C_ColorPGServerComm::WriteComm (LPCTSTR lpszMsg)
{
	CString strCmd = lpszMsg;

	Send (lpszMsg);

	CString strMsg = dfcStr (_T("ME ▶ [%s%d] : %s"), m_sInfo.m_strName, m_sInfo.m_nHead, strCmd);
	if (m_pHandler) m_pHandler (strMsg.GetBuffer (), EVENT_COMM_MSG, NULL);
}


int		C_ColorPGServerComm::GetExeHextoStr(CString& strData, CStringArray& vecResult)
{
	CString strMessage = strData;

	int nRet = 0;
	int iStart = 0, iEnd = 0, nLen = 0;

	while (1)
	{
		if (strMessage.IsEmpty() == true)
			break;

		nLen = strMessage.GetLength();
		for (int i = 0; i < nLen; i++)
		{
			TCHAR c = (TCHAR)strMessage.GetAt(i);

			if (c == 0X02)	// STX
			{
				iStart = i;
			}

			if (c == 0X03)	// ETX
			{
				iEnd = i;

				if (iStart < iEnd)
				{
					CString strParam = strMessage.Mid(iStart, iEnd + 1 - iStart);
				//	CString strPush = strParam.Mid(0, strParam.GetLength() - 1);
					vecResult.Add(strParam);

					strMessage.Replace(strParam, _T(""));

					break;
				}
			}

			if (i + 1 == nLen)
			{
				strMessage.Empty();
				break;
			}
		}
	}

	nRet = vecResult.GetSize();

	return nRet;
}
