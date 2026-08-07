#pragma once

BOOL	CmStop (BOOL bByUser = FALSE);
void	CmPGControlBegin ();
BOOL	CmPGControlProc ();
BOOL	CmDelay (DWORD dwDelay);
BOOL	CmFileFind (CString strFile);
BOOL	CmFileFindTimeOut (CString strFile, int nTimeOut);
