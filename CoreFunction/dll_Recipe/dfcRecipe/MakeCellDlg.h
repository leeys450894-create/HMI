#pragma once

const enum CELL_ROW {
	CELL_ROW_00,	CELL_ROW_01,	CELL_ROW_02,	CELL_ROW_03,	CELL_ROW_04,
	CELL_ROW_05,	CELL_ROW_06,	CELL_ROW_07,	CELL_ROW_08,	CELL_ROW_09,	
	CELL_ROW_10,	CELL_ROW_11,	CELL_ROW_12,	CELL_ROW_13,	CELL_ROW_14,
	CELL_ROW_15,	CELL_ROW_END,
};

//const enum CAD_ROW {
//	CAD_ROW_A,		CAD_ROW_B,		CAD_ROW_C,		CAD_ROW_D,		CAD_ROW_E,
//	CAD_ROW_F,		CAD_ROW_G,		CAD_ROW_H,		CAD_ROW_I,		CAD_ROW_J,
//	CAD_ROW_K,		CAD_ROW_L,		CAD_ROW_M,		CAD_ROW_N,		CAD_ROW_O,
//	CAD_ROW_P,		CAD_ROW_Q,		CAD_ROW_R,		CAD_ROW_S,		CAD_ROW_T,
//	CAD_ROW_U,		CAD_ROW_V,		CAD_ROW_W,		CAD_ROW_X,		CAD_ROW_Y,
//	CAD_ROW_X1,		CAD_ROW_X2,		CAD_ROW_X3,		CAD_ROW_X4,		CAD_ROW_X5,
//	CAD_ROW_X6,		CAD_ROW_X7,		CAD_ROW_P1_1,	CAD_ROW_P1_2,	CAD_ROW_P2_1,
//	CAD_ROW_P2_2,	CAD_ROW_P3_1,	CAD_ROW_P3_2,	CAD_ROW_P4_1,	CAD_ROW_P4_2,
//	CAD_ROW_P5_1,	CAD_ROW_P5_2,	CAD_ROW_END,
//};

#include "BaseDialog.h"
#include "IniFile.h"
#include <vector>	// LYS 2026-09-29 Hybrid Phase 2 - RecipeManager.cpp includes this header without pch


class C_MakeCellDlg : public C_BaseDialog
{
	DECLARE_DYNAMIC(C_MakeCellDlg)

	public:
		C_MakeCellDlg (CWnd* pParent = NULL);   // 표준 생성자입니다.
		virtual ~C_MakeCellDlg();

	protected:
		CWnd		*m_hWnd;
		C_IniFile	m_iniCellInfo;
		C_GridCtrl	m_gridCoordHead;
		C_GridCtrl	m_gridCoordData;
		C_GridCtrl	m_gridCADData;

		CComboBox	m_cmbCellInfoRcp;
		CComboBox	m_cmbAngle;

		CStringArray m_strArrCoordData;
	public:
		//VTM_RECIPE	*m_pRecipeMem;

	public:
		void InitCoordInfo ();
		void InitCADInfo ();
		void GetCadInfo ();

	protected:
		virtual BOOL OnInitDialog ();
		virtual void DoDataExchange (CDataExchange* pDX);    // DDX/DDV 지원입니다.
		afx_msg void OnDestroy ();
		afx_msg void OnCmbChangeRcp ();
		afx_msg void OnCellInfoOpen ();
		afx_msg void OnCellInfoSave ();
		afx_msg void OnCellMapUpdate ();
		afx_msg void OnRecipeUpdate ();
		afx_msg void OnCalcUpdate ();


	// LYS 2026-09-29 Hybrid Phase 2 하이브리드 UI - 세그먼트마다 MakeCell 입력 슬롯을 두고 RECIPE APPLY에서 한 번에 커밋
	protected:
		struct S_MakeSegInput {
			BOOL				 bOpened;		// OPEN 한 슬롯만 RECIPE APPLY에서 재생성, 나머지 세그먼트는 그대로
			CString				 strCellInfo;	// 셀 정보(GlassInfo.ini 섹션) 이름
			int					 nAngle;		// m_cmbAngle
			int					 nRotateSel;		// 좌표 그리드 GLASS ROTATE
			std::vector<CString> vecOpenData;	// m_strArrCoordData (OPEN 원본, CAD APPLY가 쓴다)
			std::vector<CString> vecCoord;		// 좌표 그리드 DATA 열
			std::vector<CString> vecCoordDesc;	// 좌표 그리드 DESCRIPTION 열
			std::vector<CString> vecCad;		// CAD 그리드 값 열
			S_MakeSegInput () : bOpened (FALSE), nAngle (0), nRotateSel (0) {}
		};
		CButton		m_chkHybrid;
		CComboBox	m_cmbSegCount;
		std::vector<S_MakeSegInput> m_vecSegInput;
		int			m_nSegView;			// 지금 그리드에 보이는 슬롯
		int			m_nSingleSrcSeg;	// 하이브리드 해제 후 RECIPE APPLY의 기준 세그먼트

		void InitHybridUI ();
		void SyncHybridFromRecipe ();
		void UpdateHybridUI ();
		BOOL IsHybridEdit ();
		void SaveSegInput (int nSeg);
		void LoadSegInput (int nSeg);
		void MoveSegView (int nSeg);
		void BuildRecipeFromGrid (CString strRcpName);
		void ApplyHybridRecipe ();
		afx_msg void OnHybridCheck ();
		afx_msg void OnSegCountApply ();
		afx_msg void OnSegPrev ();
		afx_msg void OnSegNext ();
	public:
		BOOL ShowWindow (int nCmdShow);	// SW_SHOW 때 레시피 세그먼트 수로 동기화 (SelectCellDlg와 같은 방식)

	DECLARE_MESSAGE_MAP()
};
