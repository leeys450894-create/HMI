#include "pch.h"
#include "TransporterResult.h"

#include "FTP_Link_CIM.h"
#include "..\CoreFunction\CommFtpFunction\CommFtpFunction.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CTransporterResult::CTransporterResult(CString strConfigPath)
	: m_pCommFtpFunction(new CCommFtpFunction)
	, m_pFTPLinkCIM(new CFTP_Link_CIM(strConfigPath))
	, m_bResult(false)
{
}

CTransporterResult::~CTransporterResult()
{
	delete m_pCommFtpFunction;
	m_pCommFtpFunction = nullptr;

	delete m_pFTPLinkCIM;
	m_pFTPLinkCIM = nullptr;
}

bool CTransporterResult::GetParam_UseFTPFileUpload() const
{
	return m_pFTPLinkCIM->GetParam_UseFTPFileUpload();
}

void CTransporterResult::SetParam_FTPFileUploadRemotePath(CString str)  const
{
	return m_pFTPLinkCIM->SetParam_FTPFileUploadRemotePath(str);
}

void CTransporterResult::SetParam(CString strUploadDataFolderPath)
{
	m_strUploadDataFolderPath = strUploadDataFolderPath;
}

bool CTransporterResult::Run()
{
	m_bResult = false;

	if (m_pFTPLinkCIM->GetParam_UseFTPFileUpload() == false)
	{
		CLogWrapper::GetInstance()->KMLOG(SYST, _T("Skip to FTP server"));
		return true;
	}

	//------------------------------------------------------------------------------------
	// 파일 전송하기
	//------------------------------------------------------------------------------------
	FtpInfo	Info;
	Info.host = m_pFTPLinkCIM->GetParam_FTPFileUploadHost();
	Info.username = m_pFTPLinkCIM->GetParam_FTPFileUploadUsername();
	Info.password = m_pFTPLinkCIM->GetParam_FTPFileUploadPassword();
	Info.port = m_pFTPLinkCIM->GetParam_FTPFileUploadPort();
	Info.bPassive = m_pFTPLinkCIM->GetParam_FTPFileUploadPassive();

	if (m_pCommFtpFunction->Connect(Info) == FALSE)
	{
		CLogWrapper::GetInstance()->KMLOG(SYST, _T("Failed to connect to FTP server - SpectrumImageTransporter"));
		return false;
	}

	//-----------------------------------------------------------
	// find update data List
	//-----------------------------------------------------------
	struct SUploadDataList
	{
		CString strFileFullPath;
		CString strFileName;
	};

	std::vector<SUploadDataList> vecData;

	CString sFind = m_strUploadDataFolderPath + _T("*.*");

	CFileFind find;
	BOOL bFound = find.FindFile(sFind);
	while (bFound)
	{
		bFound = find.FindNextFile();

		if (find.IsDots()) continue;
		if (find.IsDirectory()) continue;

		const CString strFileName = find.GetFileName();

		if (0 <= strFileName.Find(_T(".jpg")))		// .jgp
		{
			SUploadDataList temp;
			temp.strFileFullPath = find.GetFilePath();
			temp.strFileName = find.GetFileName();

			vecData.push_back(temp);
		}

		if (0 <= strFileName.Find(_T(".txt")))		// .txt		// APD, RcpList
		{
			SUploadDataList temp;
			temp.strFileFullPath = find.GetFilePath();
			temp.strFileName = find.GetFileName();

			vecData.push_back(temp);
		}

		if (0 <= strFileName.Find(_T(".ini")))		// .ini		// PPID
		{
			SUploadDataList temp;
			temp.strFileFullPath = find.GetFilePath();
			temp.strFileName = find.GetFileName();

			vecData.push_back(temp);
		}

		Sleep(1);
	}
	find.Close();
	//-----------------------------------------------------------


	if (vecData.empty())
	{
		CLogWrapper::GetInstance()->KMLOG(SYST, _T("FTP upload to CIM : result file Empty."));
		return false;
	}

	bool bSendResult = true;
	for (size_t i = 0; i < vecData.size(); ++i) 
	{
		CString strLocal_FilePath = vecData[i].strFileFullPath;
		CString strRemoteFilePath = m_pFTPLinkCIM->GetParam_FTPFileUploadRemotePath() + vecData[i].strFileName;
		strLocal_FilePath.Replace(_T("\r"), _T(""));
		strLocal_FilePath.Replace(_T("\n"), _T(""));
		strRemoteFilePath.Replace(_T("\r"), _T(""));
		strRemoteFilePath.Replace(_T("\n"), _T(""));

		m_pCommFtpFunction->CreateDir(strRemoteFilePath); 

		bool bUploadFileToFTP = false;
		for (long n = 0; n < 3; ++n)	
		{
			if (m_pCommFtpFunction->Upload(strLocal_FilePath, strRemoteFilePath) == 0)
			{
				bUploadFileToFTP = true;

				// 성공한 파일은 삭제한다. - backup 폴더에 남기고 있으니 지워도 되겠지?ㅋㅋ
				if (strRemoteFilePath.Find(_T("APD.")) > 0 )
				{
					DeleteFile(strLocal_FilePath);
					RemoveDirectory(m_strUploadDataFolderPath);
				}

				break;
			}
		}

		bSendResult &= bUploadFileToFTP;

		CString strLog;
		strLog.Format(_T("Send FTP Data [ %s ]"), bUploadFileToFTP ? _T("success") : _T("fail"));
		CLogWrapper::GetInstance()->KMLOG(SYST, strLog);
		strLog.Format(_T("send ftp local  path [ %s ]"), strLocal_FilePath.GetString());
		CLogWrapper::GetInstance()->KMLOG(SYST, strLog);
		strLog.Format(_T("send ftp remote path [ %s ]"), strRemoteFilePath.GetString());
		CLogWrapper::GetInstance()->KMLOG(SYST, strLog);
	}

	return m_bResult = bSendResult;
}