#pragma once

#include "..\AuroraMgrPool.h"

class CMeasureSequenceItemImpl;
class CMeasureSequenceItemFactory
{
	stAuroraMgrPool m_MgrPool_link;

public:
	std::vector<CString> GetItemType();
	std::vector<CString> GetItemDesc();

	CMeasureSequenceItemImpl *GetItem(CString strType, CString strName);

private:
	std::vector<CMeasureSequenceItemImpl *>	GetItemList(CString strName = _T(""));

public:
	CMeasureSequenceItemFactory(stAuroraMgrPool &MgrPool);
	~CMeasureSequenceItemFactory();
};