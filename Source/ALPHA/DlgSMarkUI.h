#pragma once


// CDlgSMarkUI dialog

class CDlgSMarkUI : public CDialog
{
	DECLARE_DYNAMIC(CDlgSMarkUI)

public:
	CDlgSMarkUI(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDlgSMarkUI();

	// Dialog Data
	enum { IDD = IDD_POPUP_S_MARK};

	CImage m_image;

protected:
	HICON m_hIcon;

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
};
