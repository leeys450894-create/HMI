#pragma once

#include "../GridCtrl_src/GridCell.h"  
#include "../GridCtrl_src/GridCtrl.h"  

class CGridCellButton : public CGridCell
{
	friend class CGridCtrl;
	DECLARE_DYNCREATE(CGridCellButton)

public:
	CGridCellButton();
	~CGridCellButton();

public:
	virtual BOOL Draw(CDC* pDC, int nRow, int nCol, CRect rect, BOOL bEraseBkgnd = TRUE);

protected:
	//virtual void OnClick(CPoint PointCellRelative);
	virtual void OnClickDown(CPoint PointCellRelative);
	virtual void OnClickUp(CPoint PointCellRelative);

protected:
	CRect m_rect;
	BOOL m_bPushing;
};
