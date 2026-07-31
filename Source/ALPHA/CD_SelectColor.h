#pragma once


// CD_SelectColor 대화 상자

class CD_SelectColor : public CDialogEx
{
	DECLARE_DYNAMIC(CD_SelectColor)

public:
	CD_SelectColor(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CD_SelectColor();

	void	FillColor(int iMode);

	COLORREF m_colorWAD;
	COLORREF m_colorVision;
	COLORREF m_colorAngle;
	COLORREF m_colorColor;	

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_POPUP_SELECT_COLOR };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
//	virtual void PreInitDialog();
public:
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnSelectColorWad();
	afx_msg void OnBnClickedBtnSelectColorVision();
	afx_msg void OnBnClickedBtnSelectColorAngle();
	afx_msg void OnBnClickedBtnSelectColorColor();
};
