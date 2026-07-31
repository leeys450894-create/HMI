#pragma once 

class AFX_EXT_CLASS	C_OutRecipeFunc
{
	public:
		C_OutRecipeFunc ();
		virtual ~C_OutRecipeFunc ();

	public:
		S_RecipeMgr m_recipeMgr;

	public:
		//info
		int  GetRcpVer	  () {return m_recipeMgr.m_nVersion;};
		int  GetGlsRotate () {return m_recipeMgr.m_sInfo.m_sOptionItem.m_nGlsRotate;};

		void SetModelInfoName (CString strModel) {wcscpy_s (m_recipeMgr.m_sMap.m_chModelInfoName, strModel);};
		
		//Data
		int GetStickCount ();
		S_StickInfo GetStickItem (int nStick);

		int	GetCellCount (int nStick);
		S_CellInfo GetCellItem (int nStick, int nCell);
		void SetCellItem (int nStick, int nCell, S_CellInfo cellInfo);

		void AddStick (S_StickInfo stickInfo);
		void AddCell (int nStick, S_CellInfo cellInfo);

		CString GetCellID (int nStick, int nCell);
		int GetCellArea (int nStick, int nCell);

		void SetCellStatus (int nStick, int nCell, int nMode, int nStatus);
		int GetCellStatus (int nStick, int nCell, int nMode);

		void SetSelectCell (int nStick, int nCell, int nMode, int nSelect);
		int GetSelectCell (int nStick, int nCell, int nMode);
		////

		void ClearRcpData () {m_recipeMgr.m_sData.Clear ();};

		BOOL LoadRcpFile (CString strRcpName);
		BOOL LoadAlignRcpFile (CString strAlignRcpName);
		//void ResetCellStatus (); ป๓ลย, 
};
