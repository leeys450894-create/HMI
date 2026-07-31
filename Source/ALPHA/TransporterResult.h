#pragma once

#include <vector>

class CCommFtpFunction;
class CFTP_Link_CIM;
class CTransporterResult
{
	CCommFtpFunction* m_pCommFtpFunction;
	CFTP_Link_CIM* m_pFTPLinkCIM;

	bool			m_bResult;
	CString			m_strUploadDataFolderPath;

public:
	bool GetParam_UseFTPFileUpload() const;
	void SetParam_FTPFileUploadRemotePath(CString str) const;
	void SetParam(CString strUploadDataFolderPath);
	bool Run();

	bool GetResult() const { return m_bResult; }

public:
	CTransporterResult(CString strConfigPath);
	~CTransporterResult();
};

