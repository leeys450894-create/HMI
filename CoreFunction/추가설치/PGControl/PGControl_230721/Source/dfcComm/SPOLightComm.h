#pragma once
#include <bitset>

class	AFX_EXT_CLASS	C_SPOLightComm : public C_SyncSerialComm
{
	public: 
		C_SPOLightComm () {};
		C_SPOLightComm (CString path, CString section, int nIndex);
		virtual ~C_SPOLightComm ();

	protected:
		CCriticalSection	m_section;
		int					m_nIndex;
		S_SerialInfo		m_sInfo;
		int					*m_nLightVal;
		int					*m_nLightOn;

	public:
		void	ReadIniInfo ();
		//void	WriteIniInfo (S_SerialInfo *sInfo);
		BOOL	SetLight	  (int nChannel, int nVal);
		BOOL	SetLightOn	  (int nChannel, BOOL bOn);
		int		GetLight	  (int nChannel);
		int		GetLightOn	  (int nChannel);
		BOOL	QueryLightVal (int nChannel);
		BOOL	QueryLightOn  ();

		BOOL	GetCommOutOfService() { return m_sInfo.m_bOutOfService; };
};