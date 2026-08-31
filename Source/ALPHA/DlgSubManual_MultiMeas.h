// LYS 2026-07-23 WAD 다점 측정 설정 다이얼로그(OCP CDlgNaviManualMultiMeas 이식, Phase 3).
// 그리드 4종 - 점 리스트(CLASS/SUB_CLASS/X/Y/USE/RX/GX/BY/WX/WY 10열, 행 헤더=No),
// 검사모드(Single 0 / Multi 1), 셀 크기(범위 인터락 기준),
// 셀 맵(스틱x셀 체크박스, 헤더 더블클릭 = 일괄 토글).
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
