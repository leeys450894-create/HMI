#include "pch.h"
#include "FileAccessor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CFileAccessor::CFileAccessor(void)
{
	m_pStream = NULL;
}

CFileAccessor::~CFileAccessor(void)
{
	CloseStream();
}

bool CFileAccessor::CloseStream()
{
	if(m_pStream != NULL)
	{
		fflush(m_pStream);
		fclose(m_pStream);
	}

	return true;
}

bool CFileAccessor::Open_Stream(CString strPath)
{
	m_pStream = _tfsopen(strPath, _T("a+"), _SH_DENYNO);   

	if (m_pStream == NULL)
	{
		TRACE(_T("Error : %ld\n"), errno);

		return false;
	}

	return true;
}

bool CFileAccessor::WriteStream(CString strMsg)
{
	if (m_pStream == NULL)
	{
		TRACE(_T("Error : %ld\n"), errno);

		return false;
	}

	_ftprintf(m_pStream, _T("%s\n"), strMsg.GetString());

	fflush(m_pStream);	

	return true;
}