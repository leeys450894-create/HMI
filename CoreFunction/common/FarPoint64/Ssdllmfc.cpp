//----------------------------------------------------------
//
// File: SSDLLMFC.CPP
//
// Copyright (C) 2004 FarPoint Technologies.
// All rights reserved.
//
//----------------------------------------------------------

#include "pch.h"
#include "ssdllmfc.h"

///////////////////////////////////////////////////////////////////////////////////
// TSpread

IMPLEMENT_DYNAMIC(TSpread, CWnd)

BOOL TSpread::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
  return CWnd::Create(SS_CLASSNAME, NULL, dwStyle, rect, pParentWnd, nID);
}

WNDPROC* TSpread::GetSuperWndProcAddr()
{
  static WNDPROC NEAR pfnSuper;
  return &pfnSuper;
};


///////////////////////////////////////////////////////////////////////////////////
// TPreview

IMPLEMENT_DYNAMIC(TPreview, CWnd)

BOOL TPreview::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
  return CWnd::Create(SPV_CLASSNAME, NULL, dwStyle, rect, pParentWnd, nID);
}

WNDPROC* TPreview::GetSuperWndProcAddr()
{
  static WNDPROC NEAR pfnSuper;
  return &pfnSuper;
};

