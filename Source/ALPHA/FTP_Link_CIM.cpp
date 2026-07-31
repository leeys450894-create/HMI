#include "pch.h"
#include "FTP_Link_CIM.h"

#include "AuroraFuncDefine.h"
#include "..\CoreFunction\common\IniHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CFTP_Link_CIM::CFTP_Link_CIM(CString strConfigPath)
{
    m_pIniHelper->SetFilePath(strConfigPath + _T("\\FTP_CIM.ini"));

    LinkIniFile(false);
}

CFTP_Link_CIM::~CFTP_Link_CIM()
{
}

bool CFTP_Link_CIM::LinkIniFile(bool bSave)
{
    //-----------------------------------------------------------------------
    // CIM에서 ftp 서버로 접속하기 위한 파라미터
    //-----------------------------------------------------------------------
    m_pIniHelper->Link(bSave, _T("FTP_PARA"),	_T("FTP_FILE_UPLOAD_USE"),			&m_bUseFTPFileUpload,			false);
    m_pIniHelper->Link(bSave, _T("FTP_PARA"),	_T("FTP_FILE_UPLOAD_LOCAL_PATH"),	&m_strFTPFileUploadLocalPath,	(CString)_T("D:\\IMAGE_UPLOAD_TO_FTP\\"));
    m_pIniHelper->Link(bSave, _T("FTP_PARA"),	_T("FTP_FILE_UPLOAD_REMOTE_PATH"),	&m_strFTPFileUploadRemotePath,	(CString)_T("IMAGE//"));
    m_pIniHelper->Link(bSave, _T("FTP_PARA"),	_T("FTP_FILE_UPLOAD_HOST"),			&m_strFTPFileUploadHost,		(CString)_T(""));
    m_pIniHelper->Link(bSave, _T("FTP_PARA"),	_T("FTP_FILE_UPLOAD_USER"),			&m_strFTPFileUploadUsername,	(CString)_T(""));
    m_pIniHelper->Link(bSave, _T("FTP_PARA"),	_T("FTP_FILE_UPLOAD_PASSWD"),		&m_strFTPFileUploadPassword,	(CString)_T(""));
    m_pIniHelper->Link(bSave, _T("FTP_PARA"),	_T("FTP_FILE_UPLOAD_PORT"),			&m_nFTPFileUploadPort,			21);
    m_pIniHelper->Link(bSave, _T("FTP_PARA"),	_T("FTP_FILE_UPLOAD_PASSIVE"),		&m_bFTPFileUploadPassive,		TRUE);
   
	MakeDir(m_strFTPFileUploadLocalPath);// 일단 만들어 준다.

    return m_pIniHelper->IsReadEnable();
}