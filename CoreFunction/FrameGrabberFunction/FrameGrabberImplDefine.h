#pragma once

#define		FG_UWM_UPDATEGRABDISPLY		(WM_USER + 101)
#define		FG_IMAGE_BORDER_SIZE		3
#define		FG_NUM_AF_SCAN_DEF			120

#define		INT_CCD_SERIAL_TIMEOUT		500
#define		INT_FOCUS_VAL_NOISE			-2
#define		INT_MONITOR_WND_HEIGHT		480
#define		INT_MONITOR_WND_WIDTH		640


inline BOOL MakeDir(CString sPath)
{
	if (sPath == _T(""))
	{
		return FALSE;
	}

	if (sPath[1] != ':' || sPath[2] != '\\')
	{
		return FALSE;
	}

	int nFind = 2;
	while ((nFind = sPath.Find(_T("\\"), nFind + 1)) >= 0)
	{
		CString sTem = sPath.Left(nFind);
		CreateDirectory(sTem, NULL);
	}

	CreateDirectory(sPath, NULL);

	return TRUE;
}