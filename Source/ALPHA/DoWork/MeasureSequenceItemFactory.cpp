#include "pch.h"
#include "MeasureSequenceItemFactory.h"

#include "MeasureSequenceItemImpl.h"
//#include "MeasureSequenceItem_UTG.h"

#include "..\Manager\NetworkConfigMgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CMeasureSequenceItemFactory::CMeasureSequenceItemFactory(stAuroraMgrPool &MgrPool)
	: m_MgrPool_link(MgrPool)
{

}

CMeasureSequenceItemFactory::~CMeasureSequenceItemFactory()
{

}

std::vector<CString> CMeasureSequenceItemFactory::GetItemType()
{
    std::vector<CString> vecType;

    std::vector<CMeasureSequenceItemImpl *> vecData = GetItemList();

    for (size_t i = 0; i < vecData.size(); ++i)
    {
        vecType.push_back(vecData[i]->GetType());

		if(vecData[i])
		{
			delete vecData[i];
			vecData[i] = nullptr;
		}
    }

    return vecType;
}

std::vector<CString> CMeasureSequenceItemFactory::GetItemDesc()
{
    std::vector<CString> vecDesc;

    std::vector<CMeasureSequenceItemImpl *> vecData = GetItemList();

    for (size_t i = 0; i < vecData.size(); ++i)
    {
        vecDesc.push_back(vecData[i]->GetDesc());

		if(vecData[i])
		{
			delete vecData[i];
			vecData[i] = nullptr;
		}
    }

    return vecDesc;
}
CMeasureSequenceItemImpl *CMeasureSequenceItemFactory::GetItem(CString strType, CString strName)
{
    std::vector<CMeasureSequenceItemImpl *> vecData = GetItemList(strName);

    // Type 과 동일한 item 을 찾아야 한다.
    int iitem_index = 0;
    for (int i = 0; i < (int)vecData.size(); ++i)
    {
        if (strType == vecData[i]->GetType())
        {
            iitem_index = i;
        }
    }

    // 선택한 item 을 제외한 부분은 삭제 한다.
    // return 시킨 item 은 manager 에서 직접 삭제 할것이다.
    for (int i = 0; i < (int)vecData.size(); ++i)
    {
        if (iitem_index != i)
        {
			if(vecData[i])
			{
				delete vecData[i];
				vecData[i] = nullptr;
			}
        }
    }

    return vecData[iitem_index];
}

std::vector<CMeasureSequenceItemImpl *> CMeasureSequenceItemFactory::GetItemList(CString strName)
{
    std::vector<CMeasureSequenceItemImpl *> vecData;

//	vecData.push_back(dynamic_cast<CMeasureSequenceItemImpl *>(new CMeasureSequenceItem_UTG(m_MgrPool_link, 1)));
//s	vecData.push_back(dynamic_cast<CMeasureSequenceItemImpl *>(new CMeasureSequenceItem_UTG(m_MgrPool_link, 2)));

    // 커밋 TEST 2
    return vecData;
}