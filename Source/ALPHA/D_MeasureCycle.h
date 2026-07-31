#pragma once

#include "common\Label.h"
#include "common\MFCGridControl\GridCtrl_src\GridCtrl.h"
#include "common\MFCGridControl\NewCellTypes\GridCellCheck.h"
#include "afxwin.h"

// CD_MeasureCycle 대화 상자입니다.

class CRecipeEditMgr;
class CIOMapControlMgr;
class CD_MeasureCycle : public CDialogEx
{
	DECLARE_DYNAMIC(CD_MeasureCycle)

private:
	CRecipeEditMgr *m_pRecipeEditMgr;
	CIOMapControlMgr	*m_pIOMapControlMgr;
    CLabel			m_LabelInfo;
	CComboBox		m_cmbRecipeList;

public:
	CD_MeasureCycle(CRecipeEditMgr *pRecipeEditMgr, CIOMapControlMgr *pIOMapControlMgr, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CD_MeasureCycle();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_POPUP_CYCLE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
};
