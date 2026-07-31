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


	DECLARE_MESSAGE_MAP()
};
