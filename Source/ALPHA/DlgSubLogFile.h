#pragma once

#include "common\Label.h"
// CDlgSubLogFile 대화 상자입니다.
class CLogFileAccessor;
class CFileAccessor;	// LYS 2026-09-11 화면에 뜬 로그를 파일로도 남기기 위한 raw 파일 핸들
class CDlgSubLogFile : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSubLogFile)

private:
	CFont		m_logFont;
	CLabel		m_LabelInfo;
	CListBox	m_listLog;
	CString		m_strTitle;
	CLogFileAccessor *m_pLogFile;

	// LYS 2026-09-11 이 창에 표시된 줄을 그대로 한 파일에 누적 저장(EnableFileSave 를 부른 창만 동작).
	//                여러 accessor 가 한 창으로 모이는 DLOG 는 개별 파일로 흩어져 있어 시간순 한 벌이 없었다.
	CFileAccessor	*m_pSaveAccessor;
	CString			m_strSaveBase;	// "...\\DLOG.log" 형태의 원본 경로
	CString			m_strSaveDate;	// 현재 열려 있는 파일의 yyyymmdd (날짜 바뀌면 재오픈)
	void __SaveToFile(const CString &strLine);

public:
	void LinkParam(CLogFileAccessor *log);
	void EnableFileSave(CString strFilePath);	// LYS 2026-09-11 strFilePath 의 .log 를 _yyyymmdd.log 로 바꿔 저장

public:
	CDlgSubLogFile(CString strTitle, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgSubLogFile();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_SUB_LOG_FILE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnGuiLog(WPARAM wp, LPARAM lp);
public:
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	CLogFileAccessor *GetLogFileAccessorPtr() { return m_pLogFile; }
};
