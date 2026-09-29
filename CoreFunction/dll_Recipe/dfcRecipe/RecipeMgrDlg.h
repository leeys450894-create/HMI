#pragma once
#include "MakeCellDlg.h"
#include "SelectCellDlg.h"
#include "AlignRcpDlg.h"
#include "DlgTestMode.h"	// LYS 2026-07-02
#include "RecipeShareMem.h"

#define TIMER_RECIPE 0

class C_RecipeMgrDlg : public C_BaseDialog
{
	DECLARE_DYNAMIC(C_RecipeMgrDlg)

	public:
		C_RecipeMgrDlg (CWnd* pParent = NULL);   // 표준 생성자입니다.
		virtual ~C_RecipeMgrDlg();

	protected:
		CWnd		*m_hWnd;
		C_ButtonEx	m_btnStyle[8];
		C_ButtonEx	m_rdHeadSel[2];
		C_ButtonEx	m_rdHeadMode[5];
		int			m_nHeadSel;
		int			m_nHeadMode;
		int			m_nStage;
		int			m_nHead;

		int			m_nSelOld;

		C_GridCtrl	m_gridRecipeName;

		C_GridCtrl	m_gridCellInfoHead;
		C_GridCtrl	m_gridCellInfoData;

		C_GridCtrl	m_gridInspectOptionHead;
		C_GridCtrl	m_gridInspectOptionData;

		C_GridCtrl	m_gridInspectItemHead;
		C_GridCtrl	m_gridInspectItemData;

		C_GridCtrl	m_gridLampHead;
		C_GridCtrl	m_gridLampData;

		C_GridCtrl	m_gridContactHead;
		C_GridCtrl	m_gridContactData;

		C_GridCtrl	m_gridProbeIDHead;
		C_GridCtrl	m_gridProbeIDData;

		C_GridCtrl	m_gridUserOffsetHead;
		C_GridCtrl	m_gridUserOffsetData;

		C_GridCtrl	m_gridStageMoveHead;
		C_GridCtrl	m_gridStageMoveData;

		C_GridCtrl	m_gridHeadMoveHead;
		C_GridCtrl	m_gridHeadMoveData;

		C_GridCtrl	m_gridHeadZHead;
		C_GridCtrl	m_gridHeadZData;

		C_GridCtrl	m_gridSRMLampHead;
		C_GridCtrl	m_gridSRMLampData;

		C_GridCtrl m_gridColorHead;
		C_GridCtrl m_gridColorData;

		C_GridCtrl	m_gridSubZHead;
		C_GridCtrl	m_gridSubZData;

		C_GridCtrl	m_gridAxisAngleHead1;
		C_GridCtrl	m_gridAxisAngleData1;

		C_GridCtrl	m_gridAxisAngleHead2;
		C_GridCtrl	m_gridAxisAngleData2;

		C_MakeCellDlg	*m_pMakeCellDlg;
		C_SelectCellDlg	*m_pSelectCellDlg;
		C_AlignRcpDlg	*m_pAlignRcpDlg;
	CDlgTestMode	*m_pTestModeDlg;	// LYS 2026-07-02

	public:
		VTM_RECIPE	*m_pRecipeMem;
		C_IniFile	m_iniPPID;

	public:
		void InitRecipeName ();
		void SetRecipeName (CString str);
		CString GetRecipeName ();

		void SetAlignName (CString str);
		
		void InitCellInfo ();
		void SetCellInfoData ();
		BOOL GetCellInfoData ();

		void InitInspectOption ();
		void SetInspectOptionData ();
		BOOL GetInspectOptionData ();

		void InitInspectItem ();
		void SetInspectItemData ();
		BOOL GetInspectItemData ();

		void InitLamp ();
		void SetLampData ();
		BOOL GetLampData ();
		BOOL GetLampData (int *nData, int nItem);
		BOOL CheckLampData ();

		void InitSRMLamp();
		void SetSRMLampData();
		BOOL GetSRMLampData();
		BOOL GetSRMLampData(int* nData, int nItem);
		BOOL CheckSRMLampData();

		void InitContact ();
		void SetContactData ();
		BOOL GetContactData ();
		BOOL GetContactData (double *fData, int nItem);
		BOOL CheckContactData ();

		void InitProbeID ();
		void SetProbeIDData ();
		BOOL GetProbeIDData ();

		void InitUserOffset ();
		void SetUserOffsetData ();
		BOOL GetUserOffsetData ();

		void InitSubZ();
		void SetSubZ();
		BOOL GetSubZ();

		// LYS 2026-08-19 Angle axis recipe parameters
		void InitAngleAxis();
		void SetAngleAxis();
		BOOL GetAngleAxis();

		void InitStageMove (int nStick = 6);
		void SetStageMoveData ();
		BOOL GetStageMoveData ();
		void SetStageGetPos (double *fData);
		BOOL GetStageMoveData (double *fData);

		void InitHeadMove (int nCell = 22);
		void SetHeadMoveData (int nStick = 0);
		BOOL GetHeadMoveData (int nStick = 0);
		void SetHeadGetPos (double *fData);
		BOOL GetHeadGetPos (double *fData);

		void InitHeadZ ();
		void SetHeadZData ();
		BOOL GetHeadZData ();

		void InitColorHeadZ();

		void SetRecipeView ();
		void SendAlignKeyPos ();
		BOOL GetRecipeView ();
		BOOL SaveFile (CString strRcpName);

		void AlignRcpEventProc (int nItem, int nItem2);
		void UpdateUI ();

		void WritePPIDFile (CString strModel);
		void MakeRcpList ();

	protected:
		virtual BOOL OnInitDialog ();
		virtual void DoDataExchange (CDataExchange* pDX);    // DDX/DDV 지원입니다.
		afx_msg void OnDestroy ();
		afx_msg void OnTimer (UINT_PTR nIDEvent);

		afx_msg void OnRecipeLoad ();
		afx_msg void OnRecipeSave ();
		afx_msg void OnRecipeSaveAs ();
		afx_msg void OnRecipeMakeCell ();
		afx_msg void OnRecipeSelCell ();
		afx_msg void OnRecipeAlign ();
		afx_msg void OnSetLampOff();
		afx_msg void OnSetSRMLampOff();
		afx_msg void OnSetLampOn();
		afx_msg void OnSetLampOn2();
		afx_msg void OnSelHead (UINT nID);
		afx_msg void OnSelMode (UINT nID);
		afx_msg LRESULT	OnRecipeEvent (WPARAM wParam, LPARAM lParam);
		afx_msg void	OnSelAlignRcp (NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void	OnSetCcdLamp (NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void	OnSetContactPos (NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void	OnSetStage (NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void	OnInspectOptionEndEdit (NMHDR *pNMHDR, LRESULT *pResult);	// LYS 2026-09-29 GLASS ROTATE -> 스틱 R
		void			SyncStickRotateByGlsRotate (int nGlsRotate);
		afx_msg void	OnSetHead (NMHDR *pNMHDR, LRESULT *pResult);
		afx_msg void	OnSetSRMLamp(NMHDR* pNMHDR, LRESULT* pResult);
		afx_msg void	OnSetColorPos(NMHDR* pNMHDR, LRESULT* pResult);
		//afx_msg void	OnSetColorItem(NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg void OnBtnTestMode();	// LYS 2026-07-02 TestMode dialog

	// LYS 2026-09-29 Hybrid Phase 2 세그먼트 선택기 + 세그먼트 정보(이름/PG/WAD)
	protected:
		CComboBox	m_cmbSegment;
		C_ButtonEx	m_btnSeg[2];
		C_GridCtrl	m_gridSegHead;
		C_GridCtrl	m_gridSegData;
	public:
		void InitSegmentInfo ();
		void SetSegmentView ();
		BOOL GetSegmentView ();
		void SwitchSegment (int nSeg);
	protected:
		afx_msg void OnSegmentSelChange ();
		afx_msg void OnSegmentPrev ();
		afx_msg void OnSegmentNext ();

	DECLARE_MESSAGE_MAP()
};
