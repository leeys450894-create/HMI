#pragma once


enum enumNavigateID
{
	enumNavigateID_LOGIN = 0,
	enumNavigateID_OPERATE,
	enumNavigateID_RESULT_DATA,	// RESULT DATA
	enumNavigateID_MAINT,
	enumNavigateID_MANUAL,
	enumNavigateID_RECIPE,
	enumNavigateID_SYSTEM_LOG,
	enumNavigateID_CONFIG,
	enumNavigateID_ALARM,
	enumNavigateID_BUZZER_OFF,
	enumNavigateID_PM_MODE,
	enumNavigateIDCount,
};

enum enumDoWorkStateID
{
	enumDoWorkStateID_Idle = 0,
	enumDoWorkStateID_LOADING_STATUS_ready,	// move condition
	enumDoWorkStateID_LOADING_STATUS_start,	// permit
	enumDoWorkStateID_MEASURE_STATUS_ready,
	enumDoWorkStateID_MEASURE_STATUS_start,
	enumDoWorkStateID_Unloading_STATUS_ready,
	enumDoWorkStateID_Unloading_STATUS_start,

	enumDoWorkStateID_MOVEING_Start,
	enumDoWorkStateID_MOVEING_End,
	enumDoWorkStateID_PATTERN_SEARCH_Start,
	enumDoWorkStateID_PATTERN_SEARCH_End,
	enumDoWorkStateID_ALIGN_Start,
	enumDoWorkStateID_ALIGN_End,
	enumDoWorkStateID_AF_Start,
	enumDoWorkStateID_AF_End,
	enumDoWorkStateID_MEASURE_Start,
	enumDoWorkStateID_MEASURE_End,
	enumDoWorkStateID_SAFETY_Start,
	enumDoWorkStateID_SAFETY_End,

	enumDoWorkStateID_MEASURE_P2P_Start,
	enumDoWorkStateID_MEASURE_OST_Start,
	enumDoWorkStateID_MEASURE_RTEST_Start,
	enumDoWorkStateID_MEASURE_AGING_Start,
	enumDoWorkStateID_MEASURE_AGING_End,
	enumDoWorkStateID_PROBE_CONTACT_Start,
	enumDoWorkStateID_PROBE_UNCONTACT_Start,

	enumDoWorkStateID_WORK_SACC_Start,
	enumDoWorkStateID_WORK_SACC_End,

	enumDoWorkStateID_PROBE_ALIGN_Start,
	enumDoWorkStateID_PROBE_ALIGN_End,

	enumDoWorkStateIDCount,

	enumEMOPositionID_OP = 30,
//	enumEMOPositionID_LEFT,
//	enumEMOPositionID_RIGHT,
//	enumEMOPositionID_BACK,
//	enumEMOPositionID_FRONT,
//	enumEMOPositionID_EBOX,
	enumEMOPositionID_LEFT_FRONT,
	enumEMOPositionID_RIGHT_FRONT,
	enumEMOPositionID_RIGHT_BACK,
	enumEMOPositionID_LEFT_BACK,
	enumEMOPositionID_INNER_LEFT_FRONT,
	enumEMOPositionID_INNER_RIGHT_FRONT,
	enumEMOPositionID_INNER_RIGHT_BACK,
	enumEMOPositionID_INNER_LEFT_BACK,
	enumEMOPositionID_SACC,
	enumEMOPositionID_INNER_SACC,
	enumEMOPositionID_EBOX_FRONT,
	enumEMOPositionID_EBOX_BACK,
	enumEMOPositionID_ENABLE_SWITCH,
	enumEMOPositionID_SBOX_FRONT,
	enumEMOPositionID_SBOX_BACK,
	enumEMOPositionID_Count,

	enumDoorPositionID_LEFT_FRONT,
	enumDoorPositionID_RIGHT_FRONT,
	enumDoorPositionID_RIGHT_BACK,
	enumDoorPositionID_LEFT_BACK,
	enumDoorPositionID_PACC_LOWER,
	enumDoorPositionID_PACC_UPPER,

};

class CSystemParamShare
{
	CString	m_strAppFolderPath;
	CString	m_strConfigFolderPath;
	CString	m_strLoggerFolderPath;
	CString m_strDefaultRecipeFolderPath;
	CString m_strRecipeFolderPath;
	CString	m_strResultFolderPath;
	CString m_strAlarmFolderPath;
public:
	CString	GetAppFolderPath() const						{ return m_strAppFolderPath;				}
	CString	GetConfigFolderPath() const						{ return m_strConfigFolderPath;				}
	CString	GetLoggerFolderPath() const						{ return m_strLoggerFolderPath;				}
	CString	GetDefaultRecipeFolderPath() const				{ return m_strDefaultRecipeFolderPath;		}
	CString	GetRecipeFolderPath() const						{ return m_strRecipeFolderPath;				}
	CString	GetResultFolderPath() const						{ return m_strResultFolderPath;				}
	CString	GetAlarmFolderPath() const						{ return m_strAlarmFolderPath;				}
	void	SetAppFolderPath(CString strValue)				{ m_strAppFolderPath = strValue;			}
	void	SetConfigFolderPath(CString strValue)			{ m_strConfigFolderPath = strValue;			}
	void	SetLoggerFolderPath(CString strValue)			{ m_strLoggerFolderPath = strValue;			}
	void	SetDefaultRecipeFolderPath(CString strValue)	{ m_strDefaultRecipeFolderPath = strValue;	}
	void	SetRecipeFolderPath(CString strValue)			{ m_strRecipeFolderPath = strValue;			}
	void	SetResultFolderPath(CString strValue)			{ m_strResultFolderPath = strValue;			}
	void	SetAlarmFolderPath(CString strValue)			{ m_strAlarmFolderPath = strValue;			}


private:
	bool	m_bInitialized;
	bool	m_bAutoMode;				// System Engine Mode state
	bool	m_bForceStopByUser;			// 사용자가 강제로 종료한 상황
	bool	m_bCycleState;				// Cycle Inspection Mode state
	bool	m_bTestMode;				// 테스트로 검사 확인하는 상태

	bool	m_bResumeMode;				// LYS 2026-07-03 Resume inspection mode
	int		m_nResumeStartStick;		// LYS 2026-07-03 Resume start stick index
public:
	void	SetInitialized(bool value);
	bool	IsInitialized() const;
    void	SetAutoMode(bool value);
    bool	IsAutoMode() const;
	void	SetForceStopByUser(bool value);
	bool	IsForceStopByUser() const;
	void	SetCycleState(bool value);
	bool	IsCycleState() const;
	void	SetTestMode(bool value);
	bool	IsTestMode() const;

	void	SetResumeMode(bool value, int nStartStick = 0)	{ m_bResumeMode = value; m_nResumeStartStick = nStartStick; }
	bool	IsResumeMode() const							{ return m_bResumeMode; }
	int		GetResumeStartStick() const						{ return m_nResumeStartStick; }

private:
	CString m_strInnerID;
	CString m_strGlassID;
	CString m_strCellID;
	CString m_strLotID;
	CString m_strRecipeID;
	CString m_strStepID;
	CString m_strSlotID;
	CString m_strCassetteID;
	CString m_strOperID;

	CString m_strGlassRot;		// Glass 반입 회전 방향


	long	m_nCycleCount;
public:
	CString GetInnerID() const					{ return m_strInnerID;				}
	void	SetInnerID(CString value)			{ m_strInnerID = value;				}
	CString GetGlassID() const					{ return m_strGlassID;				}
	void	SetGlassID(CString value)			{ m_strGlassID = value;				}
	CString GetLotID() const					{ return m_strLotID; }
	void	SetLotID(CString value)				{ m_strLotID = value; }
	long	GetCycleCount() const				{ return m_nCycleCount;				}
	void	SetCycleCount(long value)			{ m_nCycleCount = value;			}
	CString GetCellID() const					{ return m_strCellID; }
	void	SetCellID(CString value)			{ m_strCellID = value; }
	CString GetRecipeID() const { return m_strRecipeID; }
	void	SetRecipeID(CString value) { m_strRecipeID = value; }
	CString GetStepID() const { return m_strStepID; }
	void	SetStepID(CString value) { m_strStepID = value; }
	CString GetSlotID() const { return m_strSlotID; }
	void	SetSlotID(CString value) { m_strSlotID = value; }
	CString GetCassetteID() const { return m_strCassetteID; }
	void	SetCassetteID(CString value) { m_strCassetteID = value; }
	CString GetOperID() const { return m_strOperID; }
	void	SetOperID(CString value) { m_strOperID = value; }
	CString GetGlassRot() const { return m_strGlassRot; }
	void	SetGlassRot(CString value) { m_strGlassRot = value; }


    //----------------------------------------------------------------------------------------------------
    // Singleton
    //----------------------------------------------------------------------------------------------------
private:
    static CSystemParamShare	*m_pInstance;
public:
    static CSystemParamShare	*GetInstance();
    static void					DelInstance();
private:
    CSystemParamShare(void);
    ~CSystemParamShare(void);
};

