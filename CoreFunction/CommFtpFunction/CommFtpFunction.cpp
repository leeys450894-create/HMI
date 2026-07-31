#include "pch.h"
#include "CommFtpFunction.h"

#include "EthernetManagerFTP.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCommFtpFunction::CCommFtpFunction(void)
{
    m_pEthernetFTP = new CEthernetManagerFTP;
}

CCommFtpFunction::~CCommFtpFunction(void)
{
    if(m_pEthernetFTP)
    {
        delete m_pEthernetFTP;
        m_pEthernetFTP = nullptr;
    }
}

BOOL CCommFtpFunction::Connect(const FtpInfo &info)
{
    m_pEthernetFTP->SetFtpInfo(info);

    return m_pEthernetFTP->Connect();
}

void CCommFtpFunction::Disconnect()
{
    m_pEthernetFTP->Disconnect();
}

BOOL CCommFtpFunction::IsConnected() const
{
    return m_pEthernetFTP->IsConnected();
}

void CCommFtpFunction::SetInfo(const FtpInfo &info)
{
    m_pEthernetFTP->SetFtpInfo(info);
}

void CCommFtpFunction::GetInfo(FtpInfo &info)
{
    m_pEthernetFTP->GetFtpInfo(info);
}

long CCommFtpFunction::Upload(const CString &localFile, const CString &remoteFile)
{
    return m_pEthernetFTP->Upload(localFile, remoteFile);
}

BOOL CCommFtpFunction::Download(const CString &localFile, const CString &remoteFile)
{
    return m_pEthernetFTP->Download(localFile, remoteFile);
}

BOOL CCommFtpFunction::Rename(const CString &oldName, const CString &newName)
{
    return m_pEthernetFTP->Rename(oldName, newName);
}

BOOL CCommFtpFunction::Delete(const CString &filename)
{
    return m_pEthernetFTP->Delete(filename);
}

BOOL CCommFtpFunction::CreateDir(CString &dir)
{
    return m_pEthernetFTP->CreateDir(dir);
}

BOOL CCommFtpFunction::SetCurDir(const CString &dir)
{
    return m_pEthernetFTP->SetCurDir(dir);
}

BOOL CCommFtpFunction::GetCurDir(CString &dir)
{
    return m_pEthernetFTP->GetCurDir(dir);
}

