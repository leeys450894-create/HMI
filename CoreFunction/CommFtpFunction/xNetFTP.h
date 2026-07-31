#pragma once

#include <afxinet.h>
#include "EthernetManagerFTPDefine.h"

class CxNetFTP
{
    CInternetSession	m_internetSession;
    CFtpConnection		*m_pFtpConnection;
    FtpInfo				m_ftpInfo;

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
    CxNetFTP();
    virtual ~CxNetFTP();
};
