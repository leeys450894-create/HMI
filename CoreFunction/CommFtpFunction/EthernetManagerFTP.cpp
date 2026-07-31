#include "pch.h"
#include "EthernetManagerFTP.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CEthernetManagerFTP::CEthernetManagerFTP()
{
}

CEthernetManagerFTP::~CEthernetManagerFTP()
{
    netFTP.Disconnect();
}

BOOL CEthernetManagerFTP::Connect()
{
    return netFTP.Connect();
}

void CEthernetManagerFTP::Disconnect()
{
	netFTP.Disconnect();
}

BOOL CEthernetManagerFTP::IsConnected() const
{
	return netFTP.IsConnected();
}

long CEthernetManagerFTP::Upload(const CString &localFile, const CString &remoteFile)
{
	return netFTP.Upload(localFile, remoteFile);
}

BOOL CEthernetManagerFTP::Download(const CString &localFile, const CString &remoteFile)
{
    return netFTP.Download(localFile, remoteFile);
}

BOOL CEthernetManagerFTP::Rename(const CString &oldName, const CString &newName)
{
    return netFTP.Rename(oldName, newName);
}

BOOL CEthernetManagerFTP::Delete(const CString &filename)
{
    return netFTP.Delete(filename);
}

BOOL CEthernetManagerFTP::CreateDir(CString &dir)
{
    return netFTP.CreateDir(dir);
}

BOOL CEthernetManagerFTP::SetCurDir(const CString &dir)
{
    return netFTP.SetCurDir(dir);
}

BOOL CEthernetManagerFTP::GetCurDir(CString &dir)
{
    return netFTP.GetCurDir(dir);
}

void CEthernetManagerFTP::SetFtpInfo(const FtpInfo &info)
{
    netFTP.SetFtpInfo(info);
}

void CEthernetManagerFTP::GetFtpInfo(FtpInfo &info)
{
    netFTP.GetFtpInfo(info);
}
