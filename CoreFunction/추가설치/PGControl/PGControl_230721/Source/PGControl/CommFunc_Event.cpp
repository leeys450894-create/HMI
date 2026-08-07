#include "pch.h"
#include "PGControl.h"
#include "CommFunc_Event.h"

void	CmEvent_CommEvt (TCHAR* pMsg, int nEvent1, int nEvent2)
{
	if		(nEvent1 == EVENT_COOM_PG_COLOR_CMD) {
		CmColorDirectMsg (CString(pMsg));
	}
	else if (nEvent1 == EVENT_COOM_PG_MAIN_CMD) {
		CmMainDirectMsg (CString(pMsg));
	}
	else {
		if (theApp.GetMainPtr ()) theApp.GetMainPtr()->WriteLog (CString (pMsg));
	}

	theLogMgr.AddEventLog (CString(pMsg), 0);
}

void	CmColorDirectMsg (CString strMsg)
{
	if (theApp.m_pMainPGClientComm) {
		if (theApp.m_pMainPGClientComm->IsConnected ()) theApp.m_pMainPGClientComm->WriteComm (strMsg);
	}
}

void	CmMainDirectMsg (CString strMsg)
{
	if (theApp.m_pColorPGServerComm) {
		if (theApp.m_pColorPGServerComm->IsConnected ()) theApp.m_pColorPGServerComm->WriteComm (strMsg);
	}
}