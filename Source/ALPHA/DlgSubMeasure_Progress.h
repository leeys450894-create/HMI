#pragma once
//#include "common\CommonFunc.h"
#include "common\Label.h"
#include "..\CoreFunction\common\MySpreadDll.h"

// CDlgSubSACC_Progress 대화 상자

class CDlgSubMeasure_Progress : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSubMeasure_Progress)

private:
	CMySpreadDll*	m_spreadProgress;
	CLabel			m_LabelInfo;
	CLabel			m_LabelSuccess;
	HFONT			m_hfont;

public:
	CDlgSubMeasure_Progress(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgSubMeasure_Progress();

	// Probe ID 가져오기
	int GetProbeID();

	// Glass ID 가져오기
	int GetGlassID();

	// CEll Aging 결과 가져오기
	int GetAgingResult();

	void InitStart(int iCell);
	void InitStop();

	// CELL 상태 체크
	void UpdateAgingCell();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_SUB_SACC_PROGRESS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
