
#pragma once
#include "stdafx.h"
#include "afxwin.h"
#include "resource.h"

#include "..\common\MySpreadDll.h"
#include "..\MotorFunction\AxisType.h"
#include "common/AutoEdit.h"
#include "ShareManager.h"

#include "..\Include\IOMapAddressDefine.h"


#define WM_MSG_READ_RFID			7777 //lhj add 160823
// CDlgSACC 대화 상자입니다.


typedef struct {
	int iAxis;
	int iSpeed;
	double dPos;
} stTeached;

class CDlgSACC : public CDialog
{
public:
	CDlgSACC(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgSACC();

	// 대화 상자 데이터입니다.
	enum { IDD = IDD_DLG_SACCD };

	//
	HFONT m_hfont;

	double m_dCurAutoChangeX;
	double m_dCurAutoChangeZ;
	double m_dCurUnitZ;


	CShareManager* m_cManager;

	CMySpreadDll* m_sprdInsideProbe;
	CMySpreadDll* m_sprdProbeID;
	CMySpreadDll* m_sprdProbeBF;
	CMySpreadDll* m_sprdSACCIO;

	CString		m_sCommLogFileName;
	CFile		m_fileCommLog;

	int		m_nSelPos;		// User Manual 버튼 동작 시
	int		m_nGetProbePos;	// cassette 에서 가져올 Probe
	int		m_nPutProbePos; // cassette 로 보낼 Probe

	int		m_iSelectProbe; // 이동해야할 프로브 위치(1~5)
	int		m_iSelectLoadCassette; // Load Cassette 위치(1~5)
	int		m_iSelectUnLoadCassette; // UnLoad Cassette 위치(1~5)

	BOOL	m_bSACCStaus[4][10];
	BOOL	m_bOldSACCStaus[4][10];

	TCHAR	m_szProbeCardBF[2][32];			// Buffer
	BOOL	m_bProbeBufferStatus[2];
	BOOL	m_bProbeBufferPosUp[2];
	BOOL	m_bProbeBufferPosDown[2];

	TCHAR	m_szProbeCardID[10][32];		// Cassette
	BOOL	m_bBFProbeCardStatus[10];	
	BOOL	m_bBFProbeSlotStatus[10];

	TCHAR	m_szReadProbeCardID[10][32];	// Cassette

	CString m_strRobotProbeRFID;
	CString m_strSelRFID;
	CString m_strMountTime;
	CString m_strCurrInsideRFID;
	CString m_strUnmountTime;

	BOOL m_bGateValvePull;
	BOOL m_bGateValveOpen;
	BOOL m_bGateValveUnlock;
	BOOL m_bUnZeroPistion;
	BOOL m_bUnInsideProbe;
	BOOL m_bOldUnInsideProbe;
	BOOL m_bUnLockPCB;
	BOOL m_bRobotProbe;

	// 대기 상태

	// Probe 교체 
	double		m_dMeasUnitZPos;
	double		m_dMeasProbeXPos;


	stTeached m_stMoveACX[4];
	stTeached m_stMoveACZ[4];


	void UserButtonEnable(BOOL b);
	void UserButtonEnable2(BOOL b);
	void ManualButtonEnable(BOOL b);



	void ReadConfigRobotRFID();
	void WriteConfigFile(CString strAppName, CString strKeyName, CString strVal);
	void ReadConfigInsideRFID();
	void ReadTeachPosConfig();
	void UpdateTeachPosition();
	CString ChangeSpeedString(CString str);
	void WritePosConfigFile();
	CString ChangeSpeedNum(CString str);
	void EnableEdit(BOOL bBool);
	LRESULT OnMsgCompleteReadRFID(WPARAM wParam, LPARAM lParam);
	void SelectReadRFID();
	void UpdateUnitMovePosition(); 
	void UpdateState();

	int TeachingX1();
	int TeachingX2();
	int TeachingX3();
	int TeachingX4();
	int TeachingX1_UP();
	int TeachingX2_UP();
	int TeachingX3_UP();
	int TeachingX1_Down();
	int TeachingX2_Down();
	int TeachingX3_Down();
	void EventLog(CString strLog);
	CString GetLogFileName();
	CListCtrl m_listLog;
	int SetUpdateID(void);
	int ExeExchangeProbe(void);
	int ProcessExChanging(void);
	int DoWork_ExChanging(int iProbe);

	void GetProbeInfo(int iProbe, CString* pstrRFID, CString* pstrMountTime, int* pnContactCount);
	void SetProbeInfo(int iProbe, CString strRFID, CString strMountTime, int nContactCount);

protected:	
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.	
	virtual BOOL OnInitDialog();
	void ClickFpspreadPlcInputMemoryMap(long Col, long Row);
	afx_msg void OnDestroy();

	//SACC 동작 
	afx_msg void OnBnClickedBtnLoading();
	afx_msg void OnBnClickedBtnUnloading();
	afx_msg void OnBnClickedBtnExchange();
	afx_msg void OnBnClickedBtnGetInsideProbe();
	afx_msg void OnBnClickedBtnPutInsideProbe();
	afx_msg void OnBnClickedBtnGetInsideProbe_Get();
	afx_msg void OnBnClickedBtnPutInsideProbe_Put();
	afx_msg void OnBnClickedBtnACChangePos();
	afx_msg void OnBnClickedBtnACSafetyPos();
	afx_msg void OnBnClickedBtnACSACCPos();	
	afx_msg void OnBnClickedBtnChangeReadyMove();
	afx_msg void OnBnClickedBtnChangeMeasMove();
	afx_msg void OnBnClickedBtnLoad();
	afx_msg void OnBnClickedBtnSave();
	afx_msg void OnBnClickedChkManualEnable();
	afx_msg void OnBnClickedBtnReset();
	afx_msg void OnBnClickedBtnRifd();
	afx_msg void OnBnClickedChkEdit();
	afx_msg void OnBnClickedBtnZeroposition();
	afx_msg void OnBnClickedBtnPcb();
	afx_msg void OnBnClickedBtnGetInsideProbe_Move();
	afx_msg void OnBnClickedBtnGetInsideProbe_Close();
	afx_msg void OnBnClickedBtnPutInsideProbe_Move();
	afx_msg void OnBnClickedBtnPutInsideProbe_Close();
	afx_msg void OnBnClickedBtnReloadId();
	afx_msg void OnBnClickedBtnClam();
	afx_msg void OnBnClickedBtnSelectProbe1();
	afx_msg void OnBnClickedBtnSelectProbe2();
	afx_msg void OnBnClickedBtnSelectProbe3();
	afx_msg void OnBnClickedBtnSelectProbe4();
	afx_msg void OnBnClickedBtnSelectProbe5();
	afx_msg void OnBnClickedBtnAcgantryZup();
	afx_msg void OnBnClickedBtnAcgantryZdown();
	afx_msg void OnBnClickedBtnSetProbeID();		// LYS 2026-09-16 manual probe ID -> D2150
	afx_msg void OnBnClickedBtnClearProbeID();	// LYS 2026-09-16
	void SendManualProbeID(const CString& strID);	// LYS 2026-09-16 common sender ("" = clear)

	afx_msg LRESULT OnFPSpreadClick_Cassette(WPARAM wParam, LPARAM lParam); //

	DECLARE_MESSAGE_MAP()
public:
	/////////////////////////////////////SACC 동작 관련////////////////////////////////////////////////////
	// ///////////////////////////////////////////////////////////////////////////////////////////////////
	// Auto 동작 함수
	int ProcessUnloading();
	int ProcessLoading(int iProbe = -1);
	int DoWork_Unloading(int iProbe = -1);				// EQ >> SACC, EQ에서 Probe 탈착하여 SACC 까지 넣는 동작
	int DoWork_Loading(int iProbe = -1);				// SACC >> EQ, SACC에서 Probe 가져와서 EQ까지 장착하는 동작

	// Manual 버튼 동작
	int ManualGetOutsideProbe(int iprobe);				// EQ Probe 탈착
	int ManualPutInsideProbe(int iprobe);				// EQ Probe 장착
	int ManualGetSACCProbe(int iProbe);					// SACC Get
	int ManualPutSACCProbe(int iProbe);					// SACC Put

	// EQ 동작
	int DoWork_GetInsideProbe(void);					// EQ	Get
	int DoWork_PutInsideProbe(int iProbe);				// EQ	Put

	// SACC만 동작
	int DoWork_OpenSACC(BOOL bLoad, BOOL bDone = TRUE);	// SACC Open Loading or UnLoading 준비
	int DoWork_ChangeSACC(void);						// SACC Open Loading+UnLoaidng 동시 준비
	int DoWork_CloseSACC(void);							// SACC Close

	// SACC Open 시 Auto Change 동작
	int DoWork_GetStoreProbe(void);						// SACC Get
	int DoWork_PutStoreProbe(void);						// SACC	Put
	int DoWorkCheckReady(void);							// Auto Change 움직이기 전 모션위치 확인
	int ProcessReady_Z_Axis();
	int ProcessMeas_Z_Axis();
	int DoSearchSlot(CString strID);
	int ReadProbeIDtoFile(void);
	int UpdateSpreadProbe(void);
	int UpdateTestBtn(BOOL bEnable);
	int UpdateBtn(void);

	//PLC 요청 함수
	int PLC_Do_PROBE_CLAMP_Unlock();
	int PLC_Do_PROBE_CLAMP_Lock();
	int PLC_Do_Zero_Position_Lock();
	int PLC_Do_Zero_Position_UnLock();
	int PLC_Do_PCBUNIT_Lock();
	int PLC_Do_PCBUNIT_UnLock();
	int PLC_Do_WaitForSACCSafePosComplete(UINT nTimeOut);

	int PLC_Do_Loding(int iProbe);
	int PLC_Do_Loding_Done(int iProbe);
	int PLC_Do_UnLoding(int iProbe);
	int PLC_Do_Arm_Busy();
	int PLC_Do_Arm_Done();
	int PLC_Do_SACC_HandFoldComplete();
	int PLC_Do_WorkReadySACC();
	int PLC_Do_Command_Reset();


	//Motion 요청 함수
	int MOTION_Do_Work_ProbeTeaching(int iAxis, double dPos, int iSpeed);
	int MOTION_Do_Work_Move(int iAxis, double dPos, int iSpeed);
	int MOTION_DO_GetAxisMinLimit();
	int MOTION_Do_MoveSafeProbe();

	// AC Z UP
	CButton m_chkZ0;
	// AC Z DN
	CButton m_chkZ1;
	// ACX0
	CButton m_chkACX0;
	CButton m_chkACX1;
	CButton m_chkACX2;
	CButton m_chkACX3;
	CAutoEditDbl m_editPosZ0;
	CAutoEditDbl m_editPosZ1;
	CAutoEditDbl m_editPosZ2;
	CAutoEditDbl m_editPosZ3;
	CAutoEditDbl m_editPosACX0;
	CAutoEditDbl m_editPosACX1;
	CAutoEditDbl m_editPosACX2;
	CAutoEditDbl m_editPosACX3;
	CAutoEditDbl m_editPosUnitZ0;
	CComboBox m_cbBoxACX0;
	CComboBox m_cbBoxACX1;
	CComboBox m_cbBoxACX2;
	CComboBox m_cbBoxACX3;
	CComboBox m_cbBoxZ0;
	CComboBox m_cbBoxZ1;
	CComboBox m_cbBoxZ2;
	CComboBox m_cbBoxZ3;
	CAutoEditDbl m_editPosProbeX;
	afx_msg void OnBnClickedBtnZeroposition2();
	afx_msg void OnBnClickedBtnClam2();
	afx_msg void OnBnClickedBtnPcb2();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedBtnProbeAlign();
};

