#ifndef __SHAREMEM_H__
#define __SHAREMEM_H__

#define	df_RECIPE_SHARE_MEM_STRING _T("HBS_RECIPE_SHARED_MEMORY")

#include "preDefine.h"

//typedef struct  {
//	int		nTest;
//	CString str;
//}VTM_RECIPE;

struct VTM_RECIPE {
	//int		nTest;
	//CString str;

	S_RecipeRunState m_sRunState;
	S_Align			 m_sAlign;

	VTM_RECIPE () {
		Clear ();
	}

	void Clear () {
		//nTest  	= 0;
		//str   	= _T("");

		m_sRunState.Clear ();
		m_sAlign.Clear ();
	}
};

#endif