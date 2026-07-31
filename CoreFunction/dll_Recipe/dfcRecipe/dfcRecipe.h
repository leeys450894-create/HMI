// dfcRecipe.h : dfcRecipe DLL의 기본 헤더 파일입니다.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH에 대해 이 파일을 포함하기 전에 'stdafx.h'를 포함합니다."
#endif

#include "resource.h"		// 주 기호입니다.
#include "RecipeShareMem.h"

// CdfcRecipeApp
// 이 클래스의 구현을 보려면 dfcRecipe.cpp를 참조하십시오.
//

extern HANDLE			g_hSysMapFile;
extern VTM_RECIPE*		g_pSysMapView;
//extern S_RecipeMgr		g_RecipeMgr;
extern C_InRecipeFunc	g_inRcpInfo;

class CdfcRecipeApp : public CWinApp
{
public:
	CdfcRecipeApp();

public:
	C_IniFile	m_iniFile;
	CString		m_strRcpFolder;
	CString		m_strRcpBackup;
	CString		m_strAlignFolder;
	CString		m_strRefImgFolder;
	double		m_fContactLimit;
	CString		m_strPPIDPath;
	CString		m_strRcpListPath;

	CStringArray m_arrColorItem;
	CStringArray m_arrColorPosZ;

// 재정의입니다.
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	DECLARE_MESSAGE_MAP()
};

extern CdfcRecipeApp theApp;