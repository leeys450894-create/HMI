#pragma once

class CIniHelper;
class CFTP_Link_Interface
{
protected:
	CIniHelper	*m_pIniHelper;

	bool	m_bUseFTPFileUpload;							// 사용 유무
	CString	m_strFTPFileUploadLocalPath;					// 서버로 전송하기 위해 LOCAL 에 저장한 경로
	CString	m_strFTPFileUploadRemotePath;					// 서버로 전송하는 REMOTE 경로
	CString	m_strFTPFileUploadHost;							// 서버 IP
	CString	m_strFTPFileUploadUsername;						// 서버 접속 ID
	CString	m_strFTPFileUploadPassword;						// 서버 접속 PASSWORD
	int		m_nFTPFileUploadPort;							// 서버 PORT
	BOOL	m_bFTPFileUploadPassive;						// 서버 접속 방식

public:
	bool	GetParam_UseFTPFileUpload() const				{ return m_bUseFTPFileUpload;			}
	CString GetParam_FTPFileUploadLocalPath() const			{ return m_strFTPFileUploadLocalPath;	}
	CString GetParam_FTPFileUploadRemotePath() const		{ return m_strFTPFileUploadRemotePath;	}
	CString GetParam_FTPFileUploadHost() const				{ return m_strFTPFileUploadHost;		} 
	CString GetParam_FTPFileUploadUsername() const			{ return m_strFTPFileUploadUsername;	}
	CString GetParam_FTPFileUploadPassword() const			{ return m_strFTPFileUploadPassword;	}
	int		GetParam_FTPFileUploadPort() const				{ return m_nFTPFileUploadPort;			}
	BOOL	GetParam_FTPFileUploadPassive() const			{ return m_bFTPFileUploadPassive;		}
    void	SetParam_FTPFileUploadLocalPath(CString path)	{ m_strFTPFileUploadLocalPath = path;	}
	void	SetParam_FTPFileUploadRemotePath(CString path)	{ m_strFTPFileUploadRemotePath = path;	}
    void	SetParam_FTPFileUploadHost(CString host)		{ m_strFTPFileUploadHost = host;		}
    void	SetParam_FTPFileUploadUsername(CString name)	{ m_strFTPFileUploadUsername = name;	}
    void	SetParam_FTPFileUploadPassword(CString path)	{ m_strFTPFileUploadPassword = path;	}
    void	SetParam_FTPFileUploadPort(long nPort)			{ m_nFTPFileUploadPort = nPort;			}
    void	SetParam_FTPFileUploadPassive(bool bflag)		{ m_bFTPFileUploadPassive = bflag;		}

public:
    virtual bool LinkIniFile(bool bSave);

public:
    CFTP_Link_Interface();
    virtual ~CFTP_Link_Interface();
};