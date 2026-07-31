#pragma once
#include "nivision.h"
#include "common/AutoEdit.h"
#include "common\Label.h"
#include "ShareMemFunction\ShareMem.h"

// CD_RoiSet 대화 상자

class CIMAQImageCtrl;
class CD_RoiSet : public CDialogEx
{
	DECLARE_DYNAMIC(CD_RoiSet)

public:
	CD_RoiSet(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CD_RoiSet();

	Image* m_pBaseImage_u8;
	CIMAQImageCtrl* m_pimaqctrlROI;
	CPtrnInfo* m_ptrnROI;
	CAutoEditInt m_editY;
	CAutoEditInt m_editX;
	CAutoEditInt m_editW;
	CAutoEditInt m_editH;
	CLabel m_LabelInfo;
	int			m_iWIDTH;
	int			m_iHEIGHT; 
	CString m_strSavePath;

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PUPUP_ROI_SET };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnApplyRoi();
	afx_msg void OnBnClickedBtnCancelRoi();
	afx_msg void OnBnClickedBtnSaveImage();
	afx_msg LRESULT OnROIEvent_Click(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnROIEvent_Draw(WPARAM wp, LPARAM lp);
	afx_msg LRESULT OnROIEvent_Move(WPARAM wp, LPARAM lp);
	void DefineROI();
	int m_iCallingView;
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedBtnChangeRoi();
	void		SetROIViewInfo(int iCallingView, CPtrnInfo* pInfo);
	void		SetSavePath(CString sPath);
};
