//#include "stdafx.h"
#include "pch.h"
//#include "WindowEventHandler.h"
#include "common\IniHelper.h"

#include "LogInControlMgr.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CLogInControlMgr::CLogInControlMgr(CString strConfigPath)
	: m_pIniHelper(new CIniHelper) 
	, m_nSelectedIndex(-1)
	, m_bLoginStatus(false)
	, m_nUserLevel(0)
{
	SetName(_T("LOGIN_CONTROL_MGR"));

	m_pIniHelper->SetFilePath(strConfigPath + _T("\\LoginInfo.ini"));

	LinkIniFile(false);	
}

CLogInControlMgr::~CLogInControlMgr()
{
	Finalize();
}

bool CLogInControlMgr::LinkIniFile(bool bSave)
{
	int iCount = (int)m_vecLogin.size();

	if(bSave)
	{
		m_pIniHelper->Link(bSave, _T("LOGIN_INFO"),		_T("LOGIN_COUNT"),	&iCount,	0);
	}
	else
	{
		m_pIniHelper->Link(bSave, _T("LOGIN_INFO"),		_T("LOGIN_COUNT"),	&iCount,	0);

		m_vecLogin.clear();
		m_vecLogin.resize(iCount);
	}

	for (int i = 0; i < iCount; i++)
	{
		CString strText;
		strText.Format(_T("USER_ID_%02d"), i);
		m_pIniHelper->Link(bSave,	_T("LOGIN_INFO"),		strText,	&m_vecLogin[i].s_strUserID,		m_vecLogin[i].s_strUserID);

		strText.Format(_T("USER_PW_%02d"), i);
		m_pIniHelper->Link(bSave,	_T("LOGIN_INFO"),		strText,	&m_vecLogin[i].s_strPassword,	m_vecLogin[i].s_strPassword);

		strText.Format(_T("USER_LV_%02d"), i);
		m_pIniHelper->Link(bSave,	_T("LOGIN_INFO"),		strText,	&m_vecLogin[i].s_nUserLv,		m_vecLogin[i].s_nUserLv);
	}

	//--------------------------------------------------------------
	// PM MODE 에서 lock 해제를 위해 사용하는 PW
	//-----------------------------------------------------------------------
	m_pIniHelper->Link(bSave, _T("PM_MODE_INFO"),	_T("PW"),			&	m_strPM_mode_PW,				(CString)_T("kmac1234"));
	
	return m_pIniHelper->IsReadEnable();
}
	
void CLogInControlMgr::Finalize()
{
	if(m_pIniHelper)
	{
		delete m_pIniHelper;
		m_pIniHelper = nullptr;
	}
}



int CLogInControlMgr::SetLoninCheck(int nIndex, CString strId, CString strPassword, int nLevel)
{
	// -1	: 등록되지 않은 사용자입니다.
	// 0	: 패스워드 틀림
	// 1	: 로그인 성공

	if(0 <= nIndex && nIndex < (int)m_vecLogin.size())
	{
		bool bRet = false;
		for (size_t i = 0; i < m_vecLogin.size(); i++)
		{
			if(strId.CompareNoCase(m_vecLogin[i].s_strUserID) == 0)
			{ 
				bRet = true;
				break;
			}
				
		}
		if(!bRet)	return -1;

		if(strPassword.CompareNoCase(m_vecLogin[nIndex].s_strPassword) == 0)
		{
			m_nSelectedIndex = nIndex;
			SetLoginStatus(nLevel, true);

			m_strLoginUser	= strId;
			m_strLoginLevel = nLevel == 0 ? _T("Operator") : _T("Engineer");

			CString strLog;
			strLog.Format(_T("[LogIn - User : %s, Level : %s]"), m_strLoginUser.GetString(), m_strLoginLevel.GetString());
			CLogWrapper::GetInstance()->KMLOG(SYST, strLog);

			CWindowEventHandler::GetInstance()->PostMessageToUI(enumWndID_TITLE, WM_MSG_UPDATE_LOGIN_INFO, NULL, NULL);

			return 1;
		}
		else
		{
			SetLoginStatus(nLevel,false);
			return 0;
		}
	}
	return 0;
}

STLOGININFO *CLogInControlMgr::GetLoginInfo(int nIndex)
{
	if(0 <= nIndex && nIndex < (int)m_vecLogin.size())
	{
		return &m_vecLogin[nIndex];
	}

	return nullptr;
}

STLOGININFO *CLogInControlMgr::GetSelectedLoginInfo()
{
	if (0 <= m_nSelectedIndex && m_nSelectedIndex < (int)m_vecLogin.size())
	{
		return &m_vecLogin[m_nSelectedIndex];
	}

	return nullptr;
}

int CLogInControlMgr::GetLoginInfoCount()
{
	return (int)m_vecLogin.size();
}

bool CLogInControlMgr::SetParam_Add(CString strId, CString strPassword, int nLevel)
{
	for (size_t i = 0; i < m_vecLogin.size(); i++)
	{
		if(strId.CompareNoCase(m_vecLogin[i].s_strUserID) == 0)
			return false;
	}

	STLOGININFO stLogin;

	stLogin.s_strUserID		= strId;
	stLogin.s_strPassword	= strPassword;
	stLogin.s_nUserLv		= nLevel;

	m_vecLogin.push_back(stLogin);

	LinkIniFile(true);

	m_nSelectedIndex = (int)m_vecLogin.size() - 1;

//	SetLoginStatus(nLevel, true);

	CString strLog;
	strLog.Format(_T("[LogIn ADD - User : %s, Level : %s]"), strId.GetString(), (nLevel == 0) ? _T("Operator") : _T("Engineer"));
	CLogWrapper::GetInstance()->KMLOG(OPER, strLog);

	return true;
}

bool CLogInControlMgr::SetParam_del(int nIndex)
{
	if (nIndex != m_nSelectedIndex)	return false;
	
	if(0 <= nIndex && nIndex < (int)m_vecLogin.size())
	{
		m_vecLogin.erase(m_vecLogin.begin() + nIndex);
	}

//	SetLoginStatus(0, false);

	m_nSelectedIndex = -1;

	LinkIniFile(true);

	return true;
}

void CLogInControlMgr::SetLoginStatus(int nLevel, bool bLogin)
{
	m_bLoginStatus	= bLogin;
	m_nUserLevel	= nLevel;

	CString strData;
	strData = m_bLoginStatus ? m_vecLogin[m_nSelectedIndex].s_strUserID : _T("---");

	CWindowEventHandler::GetInstance()->PostMessageToUI(enumWndID_TITLE, WM_MSG_UPDATE_LOGIN_STATE, NULL, NULL);
}

