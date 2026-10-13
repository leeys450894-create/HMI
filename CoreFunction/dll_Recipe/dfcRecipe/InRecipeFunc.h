#pragma once 

#include "SegmentDefine.h"	// LYS 2026-09-29 Hybrid Phase 1

class C_InRecipeFunc
{
	public:
		C_InRecipeFunc ();
		virtual ~C_InRecipeFunc ();

	public:
		S_RecipeMgr m_recipeMgr;

	public:
		//info
		void SetRcpVer (int nVer) {m_recipeMgr.m_nVersion = nVer;};
		int  GetRcpVer ()		 {return m_recipeMgr.m_nVersion;};
						  
		void SetGlsRotate (int nRotate) {m_recipeMgr.m_sInfo.m_sOptionItem.m_nGlsRotate = nRotate;};
		int  GetGlsRotate ()			{return m_recipeMgr.m_sInfo.m_sOptionItem.m_nGlsRotate;};

		void SetModelInfoName	 (CString strModel) {wcscpy_s (m_recipeMgr.m_sMap.m_chModelInfoName, strModel);};
		CString GetModelInfoName ()					{return m_recipeMgr.m_sMap.m_chModelInfoName;};

		CString GetAlignRecipeName () {return m_recipeMgr.m_sInfo.m_sOptionItem.m_chAlignName;};
		////
		
		//Data
		int GetStickCount ();
		S_StickInfo GetStickItem (int nStick);
		void SetStickItem (int nStick, S_StickInfo sStick);

		int	GetCellCount (int nStick);
		S_CellInfo GetCellItem (int nStick, int nCell);
		void SetCellItem (int nStick, int nCell, S_CellInfo cellInfo);

		void AddStick (S_StickInfo stickInfo);
		void AddCell (int nStick, S_CellInfo cellInfo);

		void SetCellID (int nStick, int nCell, CString strCellID);
		CString GetCellID (int nStick, int nCell);
		void SetCellArea (int nStick, int nCell, int nArea);
		int GetCellArea (int nStick, int nCell);

		void SetCellStatus (int nStick, int nCell, int nMode, int nStatus);
		int GetCellStatus (int nStick, int nCell, int nMode);

		void SetSelectCell (int nStick, int nCell, int nMode, int nSelect);
		int GetSelectCell (int nStick, int nCell, int nMode);
		////

		void SetRcpMap  (S_RecipeMap sMap)   {m_recipeMgr.m_sMap = sMap;};
		void SetRcpData (S_RecipeData sData) {m_recipeMgr.m_sData = sData;};
		void SetRcpInfo (S_RecipeInfo sInfo) {m_recipeMgr.m_sInfo = sInfo;};

		void ClearRcpMap  () {m_recipeMgr.m_sMap.Clear ();};
		void ClearRcpData () {m_recipeMgr.m_sData.Clear ();};
		void ClearRcpInfo () {m_recipeMgr.m_sInfo.Clear ();};

		BOOL LoadRcpFile (CString strRcpName);
		BOOL LoadAlignRcpFile (CString strAlignRcpName);
		//void ResetCellStatus (); 상태, 

	// LYS 2026-09-29 Hybrid Phase 1 세그먼트 모델 (docs/HybridGlass_Porting_Plan.md 5.2)
	//   m_recipeMgr = 활성 세그먼트 작업 슬롯 - 기존 소비처(GetRecipeInfo/Data 원시 포인터)는 계속 이것만 읽는다.
	//   m_vecSegments = 전체 세그먼트. 활성 세그먼트의 최신 값은 m_recipeMgr 쪽이고, 전환/저장 때 m_vecSegments[활성]에 되쓴다.
	//   멤버는 클래스 끝에만 추가 - 객체는 DLL 힙에만 생성되고 ALPHA는 포인터만 받으므로 기존 오프셋 불변.
	public:
		void EnsureSegments ();
		void ResetSegments ();
		int  GetSegmentCount ();
		int  GetSegmentLoopCount ();
		int  GetActiveSegmentIndex ();
		BOOL SetActiveSegment (int nSeg, BOOL bWriteBack = TRUE);
		void CommitActiveSegment ();
		S_RecipeMgr* GetSegmentMgr (int nSeg);
		S_SegmentHeader GetSegmentHeader (int nSeg);
		CString GetSegmentName (int nSeg);
		int  GetSegmentRotate (int nSeg);
		int  GetSegmentStickCount (int nSeg);
		int  GetSegmentCellCount (int nSeg);
		int  GetSegmentStickBase (int nSeg);
		CString GetSegmentAlignName (int nSeg);
		int  LoadSegmentAlignRcpFiles ();
		BOOL IsSegmentAlignLoaded (int nSeg);

	protected:
		BOOL LoadRcpSections (CStdioFile *pFile, int nVersion);
		BOOL LoadHybridTail (CStdioFile *pFile, int nVersion);

	public:
		std::vector<S_RecipeMgr>	 m_vecSegments;
		std::vector<S_SegmentHeader> m_vecSegHeader;
		std::vector<BOOL>			 m_vecSegAlignOK;
		int							 m_nActiveSegment;	// -1 = 아직 로드 전

	// LYS 2026-09-29 Hybrid Phase 1 저장 (RecipeMgrDlg::SaveFile 본문 이동)
	public:
		void SaveRcpSections (CStdioFile *pFile);
		void SaveRcpSegments (CStdioFile *pFile);
		CString ValidateSegments ();

	// LYS 2026-09-29 Hybrid Phase 2 편집기 세그먼트 편집 (MakeCell RECIPE APPLY, RecipeMgr 세그먼트 정보)
	public:
		BOOL SetSegmentHeader (int nSeg, S_SegmentHeader sHdr);
		BOOL ReplaceSegments (const std::vector<S_RecipeMgr> &vecSeg, const std::vector<S_SegmentHeader> &vecHdr);
		void CollapseToActiveSegment ();

	// LYS 2026-09-29 셀/스틱 좌표 글라스 범위 검사 (MakeCell 셀 정보 오류 재발 방지)
	public:
		CString CheckRecipeLayout (const S_RecipeMgr *pMgr, CString strWhat);

	// LYS 2026-10-01 Hybrid Phase 4b Cell ID A/B (docs/HybridGlass_Porting_Plan.md 5.7). Same rule core as C_OutRecipeFunc (CellIdRule.h);
	//   the editor / SaveFile / MakeCell APPLY use these. Members stay at the class end.
	public:
		BOOL			IsCellIdDefault ();								// both panels Auto -> legacy IDs everywhere
		int				GetCellIdMode (int nPanel);						// M_CELLID_MODE_*
		int				ResolveCellToken (int nSeg, int nStick, int nCell, int nPanel, CString &strToken, BOOL bAutoFallback = FALSE);
																	// 1 = manual / derived from A, 0 = Auto (strToken = Auto token only with bAutoFallback)
		BOOL			ValidateCellIdConfig (int nGlassIdLen, std::vector<S_CellIdError> &vecErr, int *pnLongest = NULL);
																	// SaveFile / editor pass -1 (token cap M_CELLID_TOKEN_MAX). TRUE = OK (Auto/Auto always)
		BOOL			ResetCellIdConfig ();							// MakeCell APPLY: TRUE = a non-default config was cleared (info popup). Every call bumps m_nCellIdCfgSeq (cells rebuilt)
		S_CellIdConfig	GetCellIdConfig ();								// editor working copy
		void			SetCellIdConfig (const S_CellIdConfig &cfg);		// editor APPLY (Auto panels emptied, Manual panels shaped), bumps m_nCellIdCfgSeq
		CString			GetDisplayCellId (int nSeg, int nStick, int nCell);	// DLL screens: 1-segment Auto/Auto -> m_chCellID as before, else resolved panel A (N > 1 Auto/Auto = carried letters, LYS 2026-10-12 Hybrid Phase 6 D4b-7 (b))
		void			SaveCellIdTail (CStdioFile *pFile);
		int				GetCellIdCfgSeq ();								// editor: refuse APPLY when the config / cells changed under it (review C1/K1)
	protected:
		BOOL			LoadCellIdTail (CStdioFile *pFile);
	public:
		S_CellIdConfig	m_sCellIdCfg;
		int				m_nCellIdCfgSeq;								// ++ on successful LoadRcpFile, every ResetCellIdConfig, SetCellIdConfig
};
