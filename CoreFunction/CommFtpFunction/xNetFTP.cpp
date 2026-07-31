#include "pch.h"
#include "xNetFTP.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CxNetFTP::CxNetFTP()
    : m_internetSession(_T("FTP Session"))
    , m_pFtpConnection(nullptr)
{
    m_ftpInfo.host		= _T("127.0.0.1");
    m_ftpInfo.username	= _T("kmac");
    m_ftpInfo.password	= _T("kmac");
    m_ftpInfo.port		= 21;
    m_ftpInfo.bPassive	= TRUE;
}

CxNetFTP::~CxNetFTP()
{
    if(m_pFtpConnection)
    {
        m_pFtpConnection->Close();

        delete m_pFtpConnection;
        m_pFtpConnection = nullptr;
    }
}

BOOL CxNetFTP::Connect()
{
    try
    {
        if(m_pFtpConnection)
        {
            Disconnect();
        }

        m_internetSession.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT,	1000); // 1s
        m_internetSession.SetOption(INTERNET_OPTION_SEND_TIMEOUT,		1000); // 1s
        m_internetSession.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT,	1000); // 1s

		if(m_ftpInfo.host.IsEmpty())
			return FALSE;

        m_pFtpConnection = m_internetSession.GetFtpConnection(m_ftpInfo.host, m_ftpInfo.username, m_ftpInfo.password, (INTERNET_PORT)m_ftpInfo.port, m_ftpInfo.bPassive);
    }
    catch (...)
    {
        //TCHAR msg[1024];
        //pEx->GetErrorMessage(msg, 1024);
        //pEx->Delete();

        return FALSE;
    }

    return TRUE;
}

void CxNetFTP::Disconnect()
{
    if(m_pFtpConnection)
    {
        m_pFtpConnection->Close();

        delete m_pFtpConnection;
        m_pFtpConnection = nullptr;
    }
}

BOOL CxNetFTP::IsConnected() const
{
	return m_pFtpConnection ? TRUE : FALSE;
}

long CxNetFTP::Upload(const CString &localFile, const CString &remoteFile)
{
    if(!m_pFtpConnection)
	{
		if(!Connect())
			return -1;
	}

    BOOL rv = m_pFtpConnection->PutFile(localFile, remoteFile);

    if (!rv)
    {
        DWORD dw = GetLastError();

	    Disconnect();

		return dw;
	}

	return 0;
}

BOOL CxNetFTP::Download(const CString &localFile, const CString &remoteFile)
{
    if(!m_pFtpConnection)
	{
		if(!Connect())
			return FALSE;
	}

    BOOL rv = m_pFtpConnection->GetFile(remoteFile, localFile);

    if (!rv)
    {
        DWORD dw = GetLastError();

	    Disconnect();
	}

    return rv;
}

BOOL CxNetFTP::Rename(const CString &oldName, const CString &newName)
{
    if(!m_pFtpConnection)
        return FALSE;

    return m_pFtpConnection->Rename((LPCTSTR)oldName, (LPCTSTR)newName);
}

BOOL CxNetFTP::Delete(const CString &filename)
{
    if(!m_pFtpConnection)
        return FALSE;

    return m_pFtpConnection->Remove((LPCTSTR)filename);
}

BOOL CxNetFTP::CreateDir(CString &dir)
{
    if(!m_pFtpConnection)
        return FALSE;

	if (dir == _T(""))
	{
		return FALSE;
	}

	dir.Replace(_T("\\"), _T("/"));

	CString sTem;
	int nFind = 2;
	BOOL bRet = TRUE;
	while ((nFind = dir.Find(_T("/"), nFind + 1)) >= 0)
	{
		sTem = dir.Left(nFind);
	
		bRet = m_pFtpConnection->CreateDirectory((LPCTSTR)sTem);
	}

	return bRet;
}

BOOL CxNetFTP::SetCurDir(const CString &dir)
{
    if(!m_pFtpConnection)
        return FALSE;

    return m_pFtpConnection->SetCurrentDirectory((LPCTSTR)dir);
}

BOOL CxNetFTP::GetCurDir(CString &dir)
{
	if (!m_pFtpConnection)
		return FALSE;

	return m_pFtpConnection->GetCurrentDirectory(dir);
}

void CxNetFTP::SetFtpInfo(const FtpInfo &info)
{
    m_ftpInfo = info;
}

void CxNetFTP::GetFtpInfo(FtpInfo &info)
{
    info = m_ftpInfo;
}

