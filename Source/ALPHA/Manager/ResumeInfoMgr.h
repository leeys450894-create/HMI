#pragma once

// LYS 2026-07-03 Resume Inspection 관리자.
// 비정상 종료 후 검사를 재개하기 위한 상태 INI 파일을 관리한다.

class CResumeInfoMgr
{
public:
	CResumeInfoMgr(CString strConfigPath);
	~CResumeInfoMgr();

	// Load resume info from INI file
	bool LoadResumeInfo();

	// Save resume info (atomic write: tmp -> rename)
	bool SaveResumeInfo(CString strGlassID, CString strRecipeID, int nLastStick,
						bool bRunning, bool bCompleted,
						int nTotalStick = 0, CString strLotID = _T(""),
						CString strInnerID = _T(""), CString strResultPath = _T(""),
						int nGlsRotate = -1);  // LYS 2026-07-21 glass rotation

	// Update only the stick progress (lightweight save for per-stick updates)
	bool UpdateStickProgress(int nLastStick);

	// Clear resume info (inspection completed normally)
	void ClearResumeInfo();

	// Check if resume is available (Running=true, Completed=false, not expired)
	bool IsResumeAvailable() const;

	// Validate resume conditions
	bool ValidateResume(CString strCurrentRecipeID, int nTotalStick, int nGlsRotate = -1) const;  // LYS 2026-07-21 add rotation check

	// Set expire hours from SystemConfig
	void SetExpireHours(int nHours)	{ m_nExpireHours = nHours; }

	// Getters - identification
	CString GetGlassID() const		{ return m_strGlassID;		}
	CString GetRecipeID() const		{ return m_strRecipeID;		}
	CString GetLotID() const		{ return m_strLotID;		}
	CString GetInnerID() const		{ return m_strInnerID;		}

	// Getters - progress
	int		GetLastStick() const	{ return m_nLastStick;		}
	int		GetTotalStick() const	{ return m_nTotalStick;		}
	int		GetGlsRotate() const	{ return m_nGlsRotate;		}  // LYS 2026-07-21
	bool	IsRunning() const		{ return m_bRunning;		}
	bool	IsCompleted() const		{ return m_bCompleted;		}

	// Getters - time/path
	CString GetSaveTime() const		{ return m_strSaveTime;		}
	CString GetStartTime() const	{ return m_strStartTime;	}
	CString GetInspDate() const		{ return m_strInspDate;		}
	CString GetResultPath() const	{ return m_strResultPath;	}

	// Resume start stick (LastStick + 1)
	int		GetResumeStartStick() const { return m_nLastStick + 1; }

private:
	CString GetCurrentTimeString() const;
	CString GetCurrentDateString() const;
	bool IsExpired() const;
	bool WriteToFile(CString strPath);

	CString		m_strFilePath;		// ResumeInfo.ini full path
	CString		m_strTmpPath;		// ResumeInfo.ini.tmp for atomic save
	int			m_nExpireHours;		// resume validity period (hours, 0=no expiry)

	// Resume data - identification
	CString		m_strGlassID;
	CString		m_strRecipeID;
	CString		m_strLotID;
	CString		m_strInnerID;

	// Resume data - progress
	int			m_nLastStick;
	int			m_nTotalStick;
	int			m_nGlsRotate;		// LYS 2026-07-21 glass rotation angle (0/1/2/3)
	bool		m_bRunning;
	bool		m_bCompleted;

	// Resume data - time/path
	CString		m_strSaveTime;
	CString		m_strStartTime;
	CString		m_strInspDate;
	CString		m_strResultPath;
};
