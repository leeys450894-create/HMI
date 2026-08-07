#pragma once

#define PARAM_MAX	2

class AFX_EXT_CLASS C_SystemParamShare
{
	public:
		C_SystemParamShare ();
		~C_SystemParamShare ();

	private:
		static C_SystemParamShare* m_pInstance;

	public:
		static	C_SystemParamShare* GetInstance ();
		static  void				SetInstance ();
		static  void				DelInstance ();

	private:
		CString m_strLotID[PARAM_MAX];
		CString m_strGlassID[PARAM_MAX];
		CString m_strCellID[PARAM_MAX];
		CString m_strRecipeID[PARAM_MAX];

	public:
		CString GetLotID     (int nIndex = 0)					{ return m_strLotID[nIndex]; }
		void	SetLotID     (CString strVal, int nIndex = 0)	{ m_strLotID[nIndex] = strVal; }
						     
		CString GetGlassID   (int nIndex = 0)					{ return m_strGlassID[nIndex]; }
		void	SetGlassID   (CString strVal, int nIndex = 0)	{ m_strGlassID[nIndex] = strVal; }
						     
		CString GetCellID    (int nIndex = 0)					{ return m_strCellID[nIndex]; }
		void	SetCellID    (CString strVal, int nIndex = 0)	{ m_strCellID[nIndex] = strVal; }

		CString GetRecipeID  (int nIndex = 0)					{ return m_strRecipeID[nIndex]; }
		void	SetRecipeID  (CString strVal, int nIndex = 0)	{ m_strRecipeID[nIndex] = strVal; }
};

