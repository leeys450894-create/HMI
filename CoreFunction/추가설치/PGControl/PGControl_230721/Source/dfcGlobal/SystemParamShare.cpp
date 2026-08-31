#include "pch.h"
#include "dfcGlobal.h"
#include "SystemParamShare.h"

C_SystemParamShare* C_SystemParamShare::m_pInstance = nullptr;
C_SystemParamShare* C_SystemParamShare::GetInstance ()
{
	return m_pInstance ? m_pInstance : NULL;
}

void C_SystemParamShare::SetInstance ()
{
	if (!m_pInstance) 
		m_pInstance = new C_SystemParamShare;
}

void C_SystemParamShare::DelInstance  ()
{
	SAFE_DELETE (m_pInstance);
}

C_SystemParamShare::C_SystemParamShare ()
{
	for (int i = 0; i < PARAM_MAX; i++) {
		m_strLotID[i] = m_strGlassID[i] = m_strCellID[i] = m_strRecipeID[i] = _T("");
	}
}

C_SystemParamShare::~C_SystemParamShare ()
{

}