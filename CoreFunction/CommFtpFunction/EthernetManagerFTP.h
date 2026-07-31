#pragma once

#include "xNetFTP.h"

class CEthernetManagerFTP
{
    CxNetFTP netFTP;

public:
    BOOL Connect();
    void Disconnect();
	BOOL IsConnected() const;

    void SetFtpInfo(const FtpInfo &info);
    void GetFtpInfo(FtpInfo &info);

    long Upload(const CString &localFile, const CString &remoteFile);
    BOOL Download(const CString &localFile, const CString &remoteFile);

    BOOL Rename(const CString &oldName, const CString &newName);
    BOOL Delete(const CString &filename);
    BOOL CreateDir(CString &dir);
    BOOL SetCurDir(const CString &dir);
	BOOL GetCurDir(CString &dir);

public:
    CEthernetManagerFTP();
    ~CEthernetManagerFTP();
};
