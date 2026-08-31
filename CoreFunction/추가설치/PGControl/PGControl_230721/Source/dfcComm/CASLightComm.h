#pragma once
#include <bitset>

class	AFX_EXT_CLASS	C_CASLightComm : public C_SerialComm
{
	public: 
		C_CASLightComm () {};
		C_CASLightComm (CString path, CString section, int nIndex);
		virtual ~C_CASLightComm ();

	protected:
		CCriticalSection	m_section;
		int					m_nIndex;
		S_SerialInfo		m_sInfo;
		int					*m_nLightVal;
		int					*m_nLightOn;

	protected:
		virtual void ReceiveCode (TCHAR *pBuf);

	public:
		void	ReadIniInfo ();
		//void	WriteIniInfo (S_SerialInfo *sInfo);
		int		CalcValue	  (int nVal);
		BOOL	SetLight	  (int nChannel, int nVal);
		BOOL	SetLightOn	  (int nChannel, BOOL bOn);
		int		GetLight	  (int nChannel);
		int		GetLightOn	  (int nChannel);
		BOOL	QueryLightVal (int nChannel);
		BOOL	QueryLightOn  ();
};