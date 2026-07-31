#pragma once
#include "common\Label.h"
#include "common\MFCGridControl\GridCtrl_src\GridCtrl.h"
#include "common\MFCGridControl\NewCellTypes\GridCellButton.h"
#include "common\MFCGridControl\NewCellTypes\GridCellCheck.h"
#include "IOMapFunction\DataStructure.h"

struct SPLCInfo
{
	bool	bUsable;
	UINT	nAddress;
	CString	strDescription;

	SPLCInfo()
	{
		bUsable = false;
		nAddress = 9999;
		strDescription = _T("   ");
	}
};

// CDlgSubMaint_IOFile 대화 상자
class CAuroraMgr;
class CDlgSubMaint_IOFile : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSubMaint_IOFile)

private:
	CAuroraMgr* m_pAuroraMgr;
	CLabel		m_LabelInfo;
	CLabel		m_LabelGrid[7];
	CGridCtrl   m_gridList[7];
	//std::vector<SPLCInfo>	m_vecnPLCInfo; //PLC LOAD, READY, MEAS

	SADDRESSMAPDATA		m_vecnPLCInfo[10];
	
public:
	CDlgSubMaint_IOFile(CAuroraMgr* pAuroraMgr, CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgSubMaint_IOFile();

	void __InitListControl(int iGridIndex);
	void __InitialListData(int iGridIndex);

	void __RefreshListData(int iGridIndex);


	bool ReadFileCSV(CString strFilename, int iGridIndex);
// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_SUB_MAINT_IO };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
