#pragma once

class CAuroraSubMgr
{
	CString m_strName;

public:
	CString GetName() const					{ return m_strName; }
	void SetName(const CString &val)		{ m_strName = val;	}


	//------------------------------------------------------------------
	// 공통 인터페이스 함수
	//------------------------------------------------------------------
public:
	virtual bool Initialize()				{ return false;		}
	virtual bool LinkIniFile(bool bSave)	{ return false;		}
	virtual bool SetDefault()				{ return false;		}
	virtual void Finalize()					{					}

public:
	CAuroraSubMgr() {};
	virtual ~CAuroraSubMgr() {};
};
