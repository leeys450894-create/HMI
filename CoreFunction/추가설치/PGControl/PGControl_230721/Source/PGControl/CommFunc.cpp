#include "pch.h"
#include "PGControl.h"
#include "CommFunc.h"

BOOL	CmStop (BOOL bByUser)
{
	if (bByUser) theLogMgr.AddEventLog (_T("사용자 정지."), 0); 

	//모든 작업 정지
	for (int i = 0; i < M_DELAY; i++) theApp.m_runState.m_nDelay[i] = RS_STOP;

	bByUser ? theApp.m_runState.m_nRun = RS_STOP_BYUSER : theApp.m_runState.m_nRun = RS_STOP;

	return TRUE;
}

void	CmPGControlBegin ()
{
	//모듈 재연결
	if (theApp.m_pMainPGClientComm) {
		if (theApp.m_pMainPGClientComm->GetCommUse () && !theApp.m_pMainPGClientComm->IsConnected ()) {
			theApp.m_pMainPGClientComm->ConnectCtrl (TRUE);
		}
	}

	theWaitDlg->SetWaitFunc (CmPGControlProc);
}

BOOL	CmPGControlProc ()
{
	//통신 관련 파라미터 초기화
	if (theApp.m_pQualityPG232Comm)  theApp.m_pQualityPG232Comm->SetWaitItem  (QPG_WAIT_CLEAR, FALSE);
	if (theApp.m_pColorPGServerComm) theApp.m_pColorPGServerComm->SetWaitItem (CPG_WAIT_CLEAR, FALSE);

	int nAutoMode = theApp.GetMainPtr()->m_nAutoMode;
	int nResetItem = -1;

	BOOL bQualitySet[QPG_WAIT_MAX], bColorSet[CPG_WAIT_MAX];
	memset (bQualitySet, FALSE, sizeof(BOOL) * QPG_WAIT_MAX);
	memset (bColorSet,	 FALSE, sizeof(BOOL) * CPG_WAIT_MAX);

	int nPattern = 1;

	theApp.GetMainPtr()->m_bQualityRun = false;
	theApp.GetMainPtr()->m_bColorPGOn = false;

	while (theApp.m_runState.m_nRun == RS_UNITFUNC) {
		nResetItem = -1;

		//화질 에서 오는 메시지 처리
		if (theApp.m_pQualityPG232Comm) {
			if (theApp.m_pQualityPG232Comm->IsConnected ()) {
				for (int i = 0; i < QPG_WAIT_MAX; i++) {
					bQualitySet[i] = theApp.m_pQualityPG232Comm->GetWaitItem(i);
				}
			}
		}

		//면휘도 에서 오는 메시지
		if (theApp.m_pColorPGServerComm) {
			if (theApp.m_pColorPGServerComm->IsConnected ()) {
				for (int i = 0; i < CPG_WAIT_MAX; i++) {
					bColorSet[i] = theApp.m_pColorPGServerComm->GetWaitItem(i);
				}
			}
		}

		//화질, 면휘도 PG 명령 취합
		if (nAutoMode == DF_MODE_SYNC ) {
			if (bQualitySet[QPG_WAIT_TURN_ON] /*&& bColorSet[CPG_WAIT_TURN_ON]*/) {
				if (theApp.m_pPG232Comm->ActPGOn()) { //PG On 성공시
					theApp.m_pQualityPG232Comm->SetPG_Ack_Msg(); //화질에 완료 메시지 전송
		//			theApp.m_pColorPGServerComm->SetPatternEnd(); //면휘도에 완료 메시지 전송
				}
				else {
					theApp.m_pColorPGServerComm->SetPatternEnd(false); //면휘도에 완료 메시지 전송
				}

				nResetItem = QPG_WAIT_TURN_ON;

				if (nResetItem != -1) {
					theApp.m_pQualityPG232Comm->SetWaitItem(nResetItem, FALSE);
					theApp.m_pColorPGServerComm->SetWaitItem(nResetItem, FALSE);
				}

				theApp.m_pColorPGServerComm->SetWaitItem(CPG_WAIT_TURN_OFF, FALSE);
			}

			if (bQualitySet[QPG_WAIT_TURN_OFF] && bColorSet[CPG_WAIT_TURN_OFF]) { 
				if (theApp.m_pPG232Comm->ActPGOff()) {
					theApp.m_pQualityPG232Comm->SetPG_Ack_Msg(); 
				}

				nResetItem = QPG_WAIT_TURN_OFF;

				theApp.GetMainPtr()->m_bQualityRun = false;

				theApp.m_pQualityPG232Comm->SetWaitItem(nResetItem, FALSE);
			}

			if (bQualitySet[QPG_WAIT_NEXT] && bColorSet[CPG_WAIT_NEXT]) {

				nPattern = theApp.m_pQualityPG232Comm->GetPtrnItem();

				if (theApp.m_pPG232Comm->ActPGSet(nPattern)) {
					theApp.m_pQualityPG232Comm->SetPG_Ack_Msg();
					theApp.m_pColorPGServerComm->SetPatternEnd(); //면휘도에 완료 메시지 전송

					// 2023.07.21 add 화질 측정 진행 확인
					if (theApp.GetMainPtr()->m_bQualityRun != true)
					{
						theApp.GetMainPtr()->m_bQualityRun = true;
					}
				}
				else {
					theApp.m_pColorPGServerComm->SetPatternEnd(false); //면휘도에 NG 메시지 전송
				}

				nResetItem = QPG_WAIT_NEXT;

				if (nResetItem != -1) {
					theApp.m_pQualityPG232Comm->SetWaitItem(nResetItem, FALSE);
					theApp.m_pColorPGServerComm->SetWaitItem(nResetItem, FALSE);
				}
			}

			if (bQualitySet[QPG_WAIT_TURN_OFF] && bColorSet[CPG_WAIT_NEXT]) {

				if (theApp.GetMainPtr()->m_bQualityRun == true)
				{
					theApp.GetMainPtr()->m_bQualityRun = false;

					theApp.m_pColorPGServerComm->SetPatternEnd(false); //면휘도에 NG 메시지 전송

					theApp.m_pQualityPG232Comm->SetWaitItem(QPG_WAIT_TURN_OFF, FALSE);
					theApp.m_pColorPGServerComm->SetWaitItem(CPG_WAIT_NEXT, FALSE);
				}
			}
		}

		else if (nAutoMode == DF_MODE_ONLY_QUALITY) {
			if (bQualitySet[QPG_WAIT_TURN_ON] /*&& bColorSet[CPG_WAIT_TURN_ON]*/) {
				if (theApp.m_pPG232Comm->ActPGOn ()) { //PG On 성공시
					theApp.m_pQualityPG232Comm->SetPG_Ack_Msg (); //화질에 완료 메시지 전송
				}
				nResetItem = QPG_WAIT_TURN_ON;
			}

			if (bQualitySet[QPG_WAIT_TURN_OFF] /*&& bColorSet[CPG_WAIT_TURN_OFF]*/) {
				if (theApp.m_pPG232Comm->ActPGOff ()) {
					theApp.m_pQualityPG232Comm->SetPG_Ack_Msg ();
				}
				nResetItem = QPG_WAIT_TURN_OFF;
			}
			
			if (bQualitySet[QPG_WAIT_NEXT]) {

				nPattern = theApp.m_pQualityPG232Comm->GetPtrnItem();

		//		if (theApp.m_pPG232Comm->ActPGNext ()) {
				if (theApp.m_pPG232Comm->ActPGSet(nPattern)) {
					theApp.m_pQualityPG232Comm->SetPG_Ack_Msg ();
				}
				nResetItem = QPG_WAIT_NEXT;
			}

			if (nResetItem != -1) {
				theApp.m_pQualityPG232Comm->SetWaitItem(nResetItem, FALSE);
				theApp.m_pColorPGServerComm->SetWaitItem(nResetItem, FALSE);
			}
		}
		else if (nAutoMode == DF_MODE_ONLY_COLOR) {
			if (bColorSet[CPG_WAIT_TURN_ON]) {
				//	if (theApp.m_pPG232Comm->ActPGOn ()) {
				if (theApp.m_pPG232Comm->ActPGOn()) {
					theApp.m_pColorPGServerComm->SetPatternEnd(); //면휘도에 완료 메시지 전송
				}
				else {
					theApp.m_pColorPGServerComm->SetPatternEnd(false); //면휘도에 완료 메시지 전송
				}
				//	}
				nResetItem = CPG_WAIT_TURN_ON;
			}

			if (bColorSet[CPG_WAIT_NEXT]) {

				// 2023.07.21 add Color PG On 
				if (theApp.GetMainPtr()->m_bColorPGOn != true)
				{
					theApp.GetMainPtr()->m_bColorPGOn = true;

					if (theApp.m_pPG232Comm->ActPGOn ()) {
						theApp.m_pColorPGServerComm->SetPatternEnd(); //면휘도에 완료 메시지 전송
					}
					else {
						theApp.m_pColorPGServerComm->SetPatternEnd(false); //면휘도에 완료 메시지 전송
					}
				}
				else
				{
					if (theApp.m_pPG232Comm->ActPGNext()) {
						theApp.m_pColorPGServerComm->SetPatternEnd(); //면휘도에 완료 메시지 전송
					}
					else {
						theApp.m_pColorPGServerComm->SetPatternEnd(false); //면휘도에 완료 메시지 전송
					}
				}
			
				nResetItem = CPG_WAIT_NEXT;
			}

			if (bColorSet[CPG_WAIT_TURN_OFF]) {

				theApp.GetMainPtr()->m_bColorPGOn = false; 
				theApp.m_pPG232Comm->ActPGOff();

				nResetItem = CPG_WAIT_TURN_OFF;
			}

			if (nResetItem != -1) {
				theApp.m_pQualityPG232Comm->SetWaitItem(nResetItem, FALSE);
				theApp.m_pColorPGServerComm->SetWaitItem(nResetItem, FALSE);
			}
		}
	}

	return FALSE;
}

BOOL	CmDelay (DWORD dwDelay)
{
	if (dwDelay <= 0) return TRUE;

	WaitForSingleObject (theApp.m_pEventDelay->m_hObject, INFINITE);
	theApp.m_pEventDelay->ResetEvent ();

	int nth = 0;
	for (int i = 0; i < M_DELAY; i++) {
		if (theApp.m_runState.m_nDelay[i] != RS_RUNPROC) {
			nth = i;
			break;
		}
	}
	theApp.m_runState.m_nDelay[nth] = RS_RUNPROC;
	theApp.m_pEventDelay->SetEvent ();
	Sleep (0);

	DWORD dwTimeout = dwDelay;
	DWORD dwBeginTick = GetTickCount ();
	DWORD dwCurrentTick = 0;
	DWORD dwAddTime = 0;
	while (theApp.m_runState.m_nDelay[nth] == RS_RUNPROC) {
		dwCurrentTick = GetTickCount ();

		if (dwCurrentTick < dwBeginTick) {
			dwBeginTick = dwCurrentTick;
			dwTimeout -= dwAddTime;
		}
		else dwAddTime = (dwCurrentTick - dwBeginTick);

		if ((dwCurrentTick - dwBeginTick) > dwTimeout) {
			theApp.m_runState.m_nDelay[nth] = RS_DONE;
		}

		Sleep(0);
	}
	return theApp.m_runState.m_nDelay[nth] == RS_DONE;

	return TRUE;
}

BOOL CmFileFind (CString strFile)
{
	//파일 이름만 가져오기
	int pos = 0;
	CString strID;
	CString resToken = strFile.Tokenize(_T("\\"), pos);
	while (resToken != _T("")) {
		strID = resToken.Trim();
		resToken = strFile.Tokenize(_T("\\"), pos);
	}

	//파일 찾기
	BOOL bExist = FALSE;
	CFileFind finder;
	BOOL bWorking = finder.FindFile (strFile);
	while (bWorking) {
		bWorking = finder.FindNextFile ();
		if (finder.IsDots () || finder.IsDirectory ()) continue;
		if (strID == finder.GetFileName ()) {
			bExist = TRUE;
			break;
		}

		if (theWaitDlg->m_bUserStop) break;
	}
	finder.Close ();

	return bExist;
}

BOOL	CmFileFindTimeOut (CString strFile, int nTimeOut)
{
	BOOL bTimeOut = FALSE;

	DWORD dwTimeout = nTimeOut;
	DWORD dwBeginTick = GetTickCount ();
	DWORD dwRecheck = 1000; //1Sec
	DWORD dwCurrentTick = 0;
	DWORD dwAddTime = 0;

	if (!CmFileFind (strFile)) {
		while (1) {
			dwCurrentTick = GetTickCount ();

			if (dwCurrentTick < dwBeginTick) {
				dwBeginTick = dwCurrentTick;
				dwTimeout -= dwAddTime;
			}
			else dwAddTime = (dwCurrentTick - dwBeginTick);

			if ((dwCurrentTick - dwBeginTick) > dwTimeout) {
				bTimeOut = TRUE;
				break;
			}
			else if ((dwCurrentTick - dwBeginTick) > dwRecheck) {
				if (CmFileFind (strFile)) break;
				dwRecheck += 1000;
			}

			if (theWaitDlg->m_bUserStop) break;
		}
	}

	return bTimeOut;
}
