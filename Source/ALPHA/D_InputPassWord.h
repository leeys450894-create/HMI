#pragma once

// CD_InputPassWord 대화 상자입니다.

class CLogInControlMgr;
class CD_InputPassWord : public CDialogEx
{
	DECLARE_DYNAMIC(CD_InputPassWord)

private:
	CLogInControlMgr *m_pLogInControlMgr;
	CFont m_logFont;

public:
	CD_InputPassWord(CLogInControlMgr *pLogInControlMgr, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CD_InputPassWord();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_POPUP_INPUT_PASSWORD };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonInputtext();
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
