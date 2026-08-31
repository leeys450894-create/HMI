#include "pch.h"
#include "LogMgr.h"

#define WM_EVENT_LOG_MESSAGE	WM_USER + 100

C_LogMgr *pLogMgr = NULL;

C_LogMgr::C_LogMgr ()
{
	pLogMgr = this;
}

C_LogMgr::~C_LogMgr ()
{
	pLogMgr = NULL;
}

void	C_LogMgr::AddEventLog (CString msg, int nItem, BOOL bReply, BOOL bAckNak)
{

	CSingleLock sLock(&m_csWaitLogData);
	sLock.Lock();

	if (sLock.IsLocked()) {
		if (m_path[nItem].IsEmpty ()) return;
		if (msg.IsEmpty ()) return;
		msg.Trim ();

		if (bAckNak) {
			bReply ? msg += _T(" ▶ ACK") : msg += _T(" ▷ NAK");	
		}

		SYSTEMTIME SystemTime;
		::GetSystemTime (&SystemTime);

		CString szTime;
		szTime.Format (_T("%02d:%02d:%02d.%03d"), CTime::GetCurrentTime ().GetHour (), SystemTime.wMinute, SystemTime.wSecond, SystemTime.wMilliseconds);
	
		WriteEventLog (0, msg, nItem);

		sLock.Unlock ();
	}
}

void	C_LogMgr::WriteEventLog (int nthWnd, CString msg, int nItem)
{
	if (m_path[nItem].IsEmpty ()) return;

	CTime	time = CTime::GetCurrentTime ();
	CString szFolder;
	
	if (nItem == 1) szFolder = m_path[nItem] + time.Format (_T("%Y\\%m\\%d\\")); //	2023.07.17 change 0 to 1
	else szFolder = m_path[nItem];// + time.Format (_T("%Y%m%d_session\\"));
	
	CString path;
	if (nItem == 1)		path = szFolder + time.Format (_T("%H_session.log"));	//	2023.07.17 change 0 to 1
	else				path = szFolder + time.Format (_T("%Y%m%d.log"));

	dfcCreateDirectory (szFolder);

	SYSTEMTIME SystemTime;
	::GetSystemTime (&SystemTime);
	CString szTime;
	szTime.Format (_T("%02d:%02d:%02d.%03d"), CTime::GetCurrentTime ().GetHour (), SystemTime.wMinute, SystemTime.wSecond, SystemTime.wMilliseconds);
	
	_TCHAR bom = (_TCHAR)0xFEFF;

	CFile File;
	if(!File.Open (path, CFile::modeNoTruncate | CFile::modeWrite)) {
		if(!File.Open(path, CFile::modeNoTruncate | CFile::modeCreate | CFile::modeWrite)) return; // file 생성 오류
		else {
			File.Write (&bom, sizeof(_TCHAR));
			CString string;
			//string.AppendFormat (_T("%-10s,"), _T("TYPE"));

			CString strItemList;
			CString strMerge;

			if (nItem == 0) {
				string.AppendFormat (_T("%-15s,"), _T("TIME"));
				string.AppendFormat (_T("%s"), _T("LOG"));
			}
			else if (nItem ==1) {
				strItemList = _T("EQP,TIME,GLASSID,CELLID,PATTERN,ALGORITHM,DIRECTION,CONTRAST,JUDGE,PARAMETER,SX,SY,EX,EY");

				for (int i = 0; i < 21; i++) {
					if (i < 10) strMerge.Append (dfcStr (_T("UF_FIG_SUB_V0%d,"), i)); 
					else strMerge.Append (dfcStr (_T("UF_FIG_SUB_V%d,"), i)); 
				}

				string = strItemList + strMerge;
			}
			else if (nItem == 2) {
				//string  = _T("EQP,TIME,GLASSID,CELLID,ACTIVE,NO,TIME,ID,MODEL,PATTERN,DEFECT_NAME,HIGH/OFF,NG/GD,CRP_X,CRP_Y,CCD_X,CCD_Y,"); //20171226 modify by Hee (컬럼 수정)
				string  = _T("EQP,TIME,GLASSID,CELLID,PATTERN,DEFECT_NAME,HIGH/OFF,NG/GD,CRP_X,CRP_Y,CCD_X,CCD_Y,");
				string += _T("FRT_BR_SIZE,FRT_DK_SIZE,SLIM_BR_SIZE,SLIM_BR_SIZE,FRT_BR_V,FRT_DK_V,SLIM_BR_V,SLIM_DK_V,");
				string += _T("WIDTH,HEIGHT,SIZE,W_BR_V,W_DK_V,R_BR_V,R_DK_V,G_BR_V,G_DK_V,B_BR_V,B_DK_V,BK_BR_V,BK_DK_V,GY_BR_V,GY_DK_V,");
				string += _T("W_DIFF,R_DIFF,G_DIFF,B_DIFF,GY_DIFF,W_DOT_RATE,R_DOT_RATE,G_DOT_RATE,B_DOT_RATE,GY_DOT_RATE,BK_DOT_RATE,");
				string += _T("L_START,L_END,L_PCF_LENG,L_BRIGHT,L_THICK,DIRECTION,D_MARGIN,D_TH");
			}

			string.AppendFormat (_T("\r\n"));	
			File.Write (LPCTSTR(string), string.GetLength()*sizeof(_TCHAR));
		}
	}
	ULONGLONG ullEnd = File.SeekToEnd ();
	CString string;
	//string.AppendFormat (_T("%-10s,"), szItem);
	if (nItem == 0) string.AppendFormat (_T("%-15s,"), szTime);
	string.AppendFormat (_T("%s"), msg);
	string.AppendFormat (_T("\r\n"));	
	File.Write (LPCTSTR(string), string.GetLength()*sizeof(_TCHAR));

	File.Close ();
}

//void	C_LogMgr::WriteEventLogError (int nthWnd, CString msg)
//{
//	//CString szItem = GetItem (nthWnd);
//	//if (szItem.IsEmpty ()) return;
//	if (m_path.IsEmpty ()) return;
//
//	CTime	time = CTime::GetCurrentTime ();
//	CString szFolder = m_path + time.Format (_T("%Y%m%d_session\\"));
//
//	CString path;
//	if (m_bTestLog)		path = szFolder + time.Format (_T("%d_session_2.log"));
//	else				path = szFolder + time.Format (_T("%H_session_2.log"));
//
//	CreateFolder (szFolder);
//
//	SYSTEMTIME SystemTime;
//	::GetSystemTime (&SystemTime);
//	CString szTime;
//	szTime.Format (_T("%02d:%02d:%02d.%03d"), CTime::GetCurrentTime ().GetHour (), SystemTime.wMinute, SystemTime.wSecond, SystemTime.wMilliseconds);
//	
//	_TCHAR bom = (_TCHAR)0xFEFF;
//
//	CFile File;
//	if(!File.Open (path, CFile::modeNoTruncate | CFile::modeWrite)) {
//		if(!File.Open(path, CFile::modeNoTruncate | CFile::modeCreate | CFile::modeWrite)) return; // file 생성 오류
//		else {
//			File.Write (&bom, sizeof(_TCHAR));
//			CString string;
//			//string.AppendFormat (_T("%-10s,"), _T("TYPE"));
//			if (m_bTestLog) {
//				CString strItemList;
//				CString strMerge;
//				strItemList = _T("EQP,TIME,GLASSID,CELLID,PATTERN,ALGORITHM,DIRECTION,CONTRAST,JUDGE,PARAMETER,SX,SY,EX,EY,");
//
//				for (int i = 0; i < 21; i++) {
//					if (i < 10) strMerge.Append (dfcStr (_T("UF_FIG_SUB_V0%d,"), i)); 
//					else strMerge.Append (dfcStr (_T("UF_FIG_SUB_V%d,"), i)); 
//				}
//
//				string = strItemList + strMerge;
//			}
//			else {
//				string.AppendFormat (_T("%-15s,"), _T("TIME"));
//				string.AppendFormat (_T("%s"), _T("LOG"));
//			}
//			string.AppendFormat (_T("\r\n"));	
//			File.Write (LPCTSTR(string), string.GetLength()*sizeof(_TCHAR));
//		}
//	}
//	ULONGLONG ullEnd = File.SeekToEnd ();
//	CString string;
//	//string.AppendFormat (_T("%-10s,"), szItem);
//	if (!m_bTestLog) string.AppendFormat (_T("%-15s,"), szTime);
//	string.AppendFormat (_T("%s"), msg);
//	string.AppendFormat (_T("\r\n"));	
//	File.Write (LPCTSTR(string), string.GetLength()*sizeof(_TCHAR));
//
//	File.Close ();
//}

CString C_LogMgr::GetItem (int nthWnd)
{
	CString szItem;
	//if (m_mapItem.end () != m_mapItem.find (nthWnd)) szItem = m_mapItem.find (nthWnd)->second;

	return szItem;
}