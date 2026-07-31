// LYS 2026-07-23 WAD Multi Point measurement setting dialog (ported from OCP CDlgNaviManualMultiMeas)
// Phase 3: CMySpreadDll grids + CWadMultiMeasMgr binding.
//  - Point list grid  : CLASS/SUB_CLASS/X/Y/USE/RX/GX/BY/WX/WY (10 cols, row header = No)
//  - Insp mode grid   : Single(0)/Multi(1)
//  - Cell size grid   : CELL X / CELL Y (range interlock base)
//  - Cell map grid    : stick x cell checkbox map (header dbl-click = bulk toggle)
#pragma once

#include "afxwin.h"
#include <vector>

class CAuroraMgr;
class CMySpreadDll;
class CWadMultiMeasMgr;
struct S_WadMultiPoint;

class CDlgSubManual_MultiMeas : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSubManual_MultiMeas)

public:
	enum { IDD = IDD_DLG_SUB_MANUAL_MULTI_MEAS };

	CDlgSubManual_MultiMeas(CAuroraMgr* pAuroraMgr, CWnd* pParent = nullptr);
	virtual ~CDlgSubManual_MultiMeas();

	void UpdateStatus(BOOL bEnable);
	BOOL IsOfflineStatus();

private:
	// grid setup
	void	InitSpreadPointList();
	void	InitSpreadInspMode();
	void	InitSpreadCellSize();
	void	InitSpreadCellMap();
	void	SetPointRowType(int nRow);
	void	SetPointListHeaderText();		// LYS 2026-07-24 re-appliable header writer

	// grid <-> data
	void	GridToPoints(std::vector<S_WadMultiPoint>& outPoints);
	void	PointsToGrid(const std::vector<S_WadMultiPoint>& vecPoints);
	void	ResetPointGrid();
	void	LoadFromMgr();
	void	SaveToMgr();
	void	RebuildCellMap();

	CWadMultiMeasMgr* GetMgr();

	CAuroraMgr*		m_pAuroraMgr;

	CMySpreadDll*	m_spreadPointList;		// IDC_FPS_MM_LIST
	CMySpreadDll*	m_spreadInspMode;		// IDC_FPS_MM_MODE
	CMySpreadDll*	m_spreadCellSize;		// IDC_FPS_MM_CELLSIZE
	CMySpreadDll*	m_spreadCellMap;		// IDC_FPS_MM_CELLMAP

	HFONT			m_hFont;

	CString			m_strMultiRcpPath;		// WAD Multi recipe folder ([OPT_PARA] WAD_MULTI_RCP_PATH)

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

	afx_msg LRESULT OnSpreadDblClick(WPARAM wParam, LPARAM lParam);	// SSM_DBLCLK (cell map bulk toggle)

public:
	afx_msg void OnBnClickedButtonMmOpen();
	afx_msg void OnBnClickedButtonMmSave();
	afx_msg void OnBnClickedButtonMmListInit();
	afx_msg void OnBnClickedButtonMmAddRow();
	afx_msg void OnBnClickedButtonMmDelRow();
};
