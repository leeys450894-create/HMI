#pragma once
#include "common\Label.h"

// CD_SelectMeasOrUnload 대화 상자

class CD_SelectMeasOrUnload : public CDialogEx
{
	DECLARE_DYNAMIC(CD_SelectMeasOrUnload)

public:
	CD_SelectMeasOrUnload(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CD_SelectMeasOrUnload();
	CLabel			m_LabelInfo;

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_POPUP_SELECT_MEAS_OR_UNLOAD };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnGlassExistSelectMeas();
	afx_msg void OnBnClickedBtnGlassExistSelectUnload();
	afx_msg void OnBnClickedBtnGlassExistSelectAbort();
	int m_nMode;
};
