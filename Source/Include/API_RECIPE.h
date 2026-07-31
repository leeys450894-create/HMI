#pragma once

//#include "preDefine.h"
#include "RecipeShareMem.h"
#include "OutRecipeFunc.h"

VTM_RECIPE* WINAPI Recipe_GetSharedMemory ();
void WINAPI Recipe_Initialize (HWND hParentWnd, CRect rc);
CWnd* WINAPI Recipe_GetRecipehWnd ();
void WINAPI Recipe_Finalize ();
void WINAPI Recipe_SetMsg (CString strMsg);
C_OutRecipeFunc* WINAPI GetRecipeMgr ();