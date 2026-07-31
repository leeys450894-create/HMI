#pragma once 

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
		//void ResetCellStatus (); ป๓ลย, 
};
