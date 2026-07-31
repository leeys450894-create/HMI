#pragma once

#include "..\AuroraSubMgr.h"

struct STLOGININFO
{
	CString s_strUserID;
	CString s_strPassword;
	int		s_nUserLv;

	STLOGININFO()
	{
		s_strUserID		= _T("");
		s_strPassword	= _T("0000");
		s_nUserLv		= 0;
	}
};

class CIniHelper;
class CLogInControlMgr : public CAuroraSubMgr
{
	CIniHelper	*m_pIniHelper;
	std::vector<STLOGININFO>	m_vecLogin;
	long			m_nSelectedIndex;
	bool			m_bLoginStatus;
	long			m_nUserLevel;
	CString			m_strLoginUser;
	CString			m_strLoginLevel;

public:
	void SetLoginStatus(int nLevel, bool bLogin);
	bool GetLoginStatus() const { return m_bLoginStatus;	}
	int	 GetLoginLevel() const	{ return m_nUserLevel;		}

	CString GetLoginUserInfo() const	{ return m_strLoginUser;	}
	CString GetLoginLevelInfo() const	{ return m_strLoginLevel;	}

	int SetLoninCheck(int nIdex, CString strId, CString strPassword, int nLevel = 0);
	STLOGININFO *GetLoginInfo(int nIndex);
	STLOGININFO *GetSelectedLoginInfo();
	int  GetLoginInfoCount();
	bool SetParam_Add(CString strId, CString strPassword, int nLevel);
	bool SetParam_del(int nIndex);


private:
	CString m_strPM_mode_PW;
public:
	CString GetPMmodePW() const	{ return m_strPM_mode_PW;	}



//------------------------------------------------------------------
// 공통 인터페이스 함수
//------------------------------------------------------------------
public:
	virtual bool LinkIniFile(bool bSave) override;
private:
	virtual void Finalize() override;
public:
	CLogInControlMgr(CString strConfigPath);
	virtual ~CLogInControlMgr();
};