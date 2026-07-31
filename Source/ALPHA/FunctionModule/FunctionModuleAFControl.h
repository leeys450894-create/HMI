#pragma once

#include "..\BaseSequenceImpl.h"

//AF Module
struct stKMACAFModuleParameter
{
	//KMAC Image
	double dMulti;
	bool bFitPeak;
	int nStep;
	double dStepSize;
	double dStroke;
	double dSpeed;

	//Other
	int nChannel;

	stKMACAFModuleParameter()
	{
		dMulti = 1.0;
		bFitPeak = true;

		nStep = 200;			//LensDB "NSTP" * dMulti
		dStepSize = 0.002;		//LendDB "STEP" / 100000 mm
		dStroke = 4.0;			//nStep * nStepSize
		dSpeed = 4.0;			//nstepSize * CCD "NSTP" - CCD Frame Rate

		nChannel = 0;
	}
};

//KMAC AF Module
enum af_result_t {
	AF_RES_SUCCESS = 0,		//
	AF_RES_MAX_LOW_END,		// Too small difference in energy function, and max located near lower end		
	AF_RES_MAX_HIGH_END,	// Too small difference in energy function, and max located near upper end
	AF_RES_SMALL_E_DIFF,	// More AF-trying seems to be effortless, usu. because too small energy diff and max located around center.
	AF_RES_PEAK_LOW_END,	// Found peak is near the upper end of scan range
	AF_RES_PEAK_HIGH_END,	//                        lower
	AF_RES_SMALL_RANGE,		// Too large width value after peak fitting, probably due to monotonic energy increase/decrease
	AF_RES_FIT_ERROR,		/// <LifeSoul> 20090707 : AF Fit Error(Retry to once again FitPeakFocus)
	AF_RES_END
};


class CAutoFocusController;
class CFunctionModuleAFControl : public CBaseSequenceImpl
{
	CAutoFocusController *m_pAutoFocusController;


public:
	//AF 동작
	// 1. 연결 확인
	bool InitAFModule(int nIndex);
	int HomingAFModule(int nIndex);
	bool CheckAFModule(int nIndex);
	bool CloseAFModule(int nIndex);
	// 2. AF Parameter 설정
	void SetAFParameter(int nIndex, int nChannel);
	void SetAFParameterKMACImage(int nIndex);
	void SetAFParameterOther(int nIndex, int nChannel);
	// 3. AF
	int StartAF(int nIndex);
	int StartAFKMACImage(int nIndex);
	int StartAFOther(int nIndex, bool bDoneCheck = true);
	// 4. AF 완료 확인
	int CheckAFDone(int nIndex);
	// 5. AF Image 저장
	int SaveAFImage(int nIndex);
	// 6. AF Stop
	int AFStop(int nIndex);

	//기타
	stKMACAFModuleParameter GetAFParameter(int nIndex);

	int GetAFObjInfo(int nIndex, int nLensIndex);

	// AF 성공 시 위치 설정
	void SetAFPosition(double dAFPosition);
	double GetAFPosition();

	int MovePosition(int nIndex, double dPostion);
	int CheckMoveDone(int nIndex);
	double GetPos(int nIndex);
	void GetStatus(int nIndex, bool* pbHome, bool* pbPos, bool* pbNeg);

private:
	stKMACAFModuleParameter m_stParam[2];
	double m_dAFPosition;

	//=========================================================================
	// INTERFACE 관련
	//=========================================================================
public:
	// Create / Delete
	virtual bool	CreateDoWorkSeq()										override;
	virtual void	DeleteDoWorkSeq()										override;
	virtual bool	ChangeState()											override;
	virtual bool	UpdateState()											override;
	virtual bool	LinkDataBase(CBinaryHelper *pBinaryHelper, bool bSave)	override;
	virtual void	ThreadRun()												override;

public:
	CFunctionModuleAFControl(stAuroraMgrPool &MgrPool, CAutoFocusController *pAutoFocusController);
	virtual ~CFunctionModuleAFControl(void);
};

