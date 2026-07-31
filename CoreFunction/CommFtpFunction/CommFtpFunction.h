#pragma once

#include "EthernetManagerFTPDefine.h"

class CEthernetManagerFTP;
class CCommFtpFunction
{
    CEthernetManagerFTP *m_pEthernetFTP;

public:
    BOOL Connect(const FtpInfo &info);
    void Disconnect();
	BOOL IsConnected() const;

    void SetInfo(const FtpInfo &info);
    void GetInfo(FtpInfo &info);

    long Upload(const CString &localFile, const CString &remoteFile);
    BOOL Download(const CString &localFile, const CString &remoteFile);

    BOOL Rename(const CString &oldName, const CString &newName);
    BOOL Delete(const CString &filename);
    BOOL CreateDir(CString &dir);
    BOOL SetCurDir(const CString &dir);
	BOOL GetCurDir(CString &dir);

public:
    CCommFtpFunction(void);
    virtual ~CCommFtpFunction(void);
};
