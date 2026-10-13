#pragma once

// LYS 2026-07-03 Resume Inspection 관리자.
// 비정상 종료 후 검사를 재개하기 위한 상태 INI 파일을 관리한다.

// LYS 2026-10-08 Hybrid Phase 5 tuple bookmark types - defined in Manager/ResumeJournal.h (include it before calling the tuple API)
struct S_ResumeShape;
struct S_ResumeTuple;
struct S_ResumeBookmark;

// LYS 2026-10-08 Hybrid Phase 5 ValidateTupleShape *pnFail: the first failing check, in the popup's order (RecipeID -> shape -> files)
enum enumResumeShapeCheck
{
	RESUME_SHAPE_OK = 0,
	RESUME_SHAPE_RECIPE,		// RecipeID differs - reversible refusal (plan (2-2) #2)
	RESUME_SHAPE_SEGMENTS,		// SegmentCount / SegShape / GlassRows differ, or not a Format 2 bookmark - structural refusal (#3)
	RESUME_SHAPE_FILES,			// Fingerprint differs (rcp / aging / efa / wadmp file content) - reversible refusal (#4)
};

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

	// LYS 2026-10-08 Hybrid Phase 5 tuple bookmark (Format=2) of a journal glass (CResumeJournal::IsTupleGlass) - scratchpad p5_plan.md (1) (6-2).
	//   A legacy (Format 1) bookmark keeps the functions above and its file bytes. The functions below hold m_cs (lock order m_cs -> journal lock).
	bool	IsTupleBookmark() const			{ return m_nFormat >= 2;		}
	bool	BeginTupleGlass(CString strGlassID, CString strRecipeID, CString strLotID, CString strInnerID, CString strResultPath,
							int nTotalStick0, int nGlsRotate0, const S_ResumeShape& shape, int nPassCount, int nEfaMode, int nEfaFlag, bool bLog);	// measure start: Format 2, (0,0,-1), new StartTime; bLog = N > 1
	bool	UpdateTupleProgress(int nPass, int nSeg, int nStick, bool bJournalOK, int nJournalLines);	// stick done; JournalOK is sticky (once 0 stays 0)
	bool	SetPassBoundaryDone(int nPass);					// EfaPassBoundary side effects done (never lowers)
	bool	MarkResultSaved(int nLevel);					// 1 Result item done, 2 result handled (never lowers)
	bool	ApplyCompaction(bool bRewind, const S_ResumeTuple& last, int nKeptLines);	// Attempt+1, JournalLines; bRewind: Last* = last, ResultSaved = 0 (BEFORE the journal swap; write failure = no change)
	int		GetResumePoint(int& rPass, int& rSeg, int& rStick) const;	// 0 point, 1 all done (outputs PassCount,0,0), -1 invalid or legacy bookmark
	bool	ValidateTupleShape(const CString& strRecipeID, const S_ResumeShape& cur, CString& strWhyEn, int* pnFail = NULL) const;	// *pnFail = enumResumeShapeCheck
	void	GetBookmark(S_ResumeBookmark& out) const;		// one consistent copy (nPoint / T filled) for Analyze / WriteCompacted / popup
	int		GetLastPass() const				{ return m_nLastPass;			}
	int		GetLastSegment() const			{ return m_nLastSegment;		}
	int		GetSegmentCount() const			{ return m_nSegmentCount;		}
	int		GetPassCount() const			{ return m_nPassCount;			}
	int		GetEfaMode() const				{ return m_nEfaMode;			}
	int		GetEfaFlag() const				{ return m_nEfaFlag;			}
	int		GetPassBoundaryDone() const		{ return m_nPassBoundaryDone;	}
	int		GetResultSaved() const			{ return m_nResultSaved;		}
	int		GetAttempt() const				{ return m_nAttempt;			}
	int		GetJournalOK() const			{ return m_nJournalOK;			}
	int		GetJournalLines() const			{ return m_nJournalLines;		}
	int		GetGlassRows() const			{ return m_nGlassRows;			}
	CString	GetSegShape() const;							// m_cs
	CString	GetFingerprint() const;							// m_cs

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

	// LYS 2026-10-08 Hybrid Phase 5 tuple bookmark (Format=2) - keys appended after the legacy ones, written only when m_nFormat >= 2
	void		ResetTupleMembers();	// Format 1 defaults - shared by the missing-file return of LoadResumeInfo, ClearResumeInfo, SaveResumeInfo (review C14)
	void		ReplaceIniFromTmp();	// D5-18 (a) MoveFileEx(REPLACE_EXISTING | WRITE_THROUGH); the legacy delete + move / copy is the fallback
	bool		SaveTupleLocked();		// SaveTime + tmp + replace (m_cs held)

	mutable CRITICAL_SECTION m_cs;		// Save / Update / tuple writes / Clear / Load / GetBookmark
	int			m_nFormat;				// 1 legacy (no key), 2 tuple
	int			m_nLastPass;			// pass of the last finished stick (0 = T0)
	int			m_nLastSegment;			// its segment (0 based); m_nLastStick = its segment-local stick
	int			m_nSegmentCount;		// N
	CString		m_strSegShape;			// "{sticks}x{cells}r{rotate}" per segment joined by '|'
	int			m_nGlassRows;			// G
	int			m_nPassCount;			// passes at measure start (EFA rcp items, 1 = no EFA pass loop)
	int			m_nEfaMode;				// bEfaMode at measure start
	int			m_nEfaFlag;				// shared NewAgingMode at measure start - compared, never restored (D5-9 b)
	int			m_nPassBoundaryDone;	// highest pass whose entry boundary finished
	int			m_nResultSaved;			// 0 / 1 Result item done / 2 result handled
	int			m_nAttempt;				// resume attempts (one per compaction)
	int			m_nJournalOK;			// sticky journal write state of this glass
	int			m_nJournalLines;		// journal data lines at the last Commit / compaction
	CString		m_strFingerprint;		// "{rcp}:{aging}:{efa}:{wadmp}"
};
