#pragma once

#include "..\AuroraSubMgr.h"

enum enumAXIS
{
	enumAXIS_X = 0,
	enumAXIS_Y,
	enumAXIS_R,
	enumAXIS_COUNT,
};

struct stPositionAxis
{
	double dX;
	double dY;
	double dZ;

	stPositionAxis()
	{
		dX = 0;
		dY = 0;
		dZ = 0;
	}
};

struct stPositionParam
{
	double dSafetyStageX;
	double dSafetyHeadY1;
	double dSafetyHeadY2;
	double dSafetyHeadX1;
	double dSafetyHeadX2;
	double dSafetyZ;
	double dSafetyZ2;

	double dLoaderZUp;
	double dLoaderZMiddle;
	double dLoaderZPre;
	double dLoaderZDown;

	double dLoadPosition[enumAXIS_COUNT];
	double dUnloadPosition[enumAXIS_COUNT];
	double dMeasPosition[enumAXIS_COUNT];
		
	double dProbeSafety[5];
	double dACPosition[3];	   // 0:change, 1:Safety, 2:SACC
	double dACZPosition[3];    // 0:change, 1:Safety, 2:SACC
	double dProbeChangePos;    // change
	double dProbeZChangePos[3];   // 0: change, 1:Safety, 3:Zero
	double dProbeXOffsetPos[5];
	double dProbeYOffsetPos[5];
	double dProbeTOffsetPos[5];

	double dPickerStageX;
	double dAlignToMeasGap;

	double dAlignPos;

	double dGantryGap;
	double dHeadGap;

	// Head Offset
	stPositionAxis dHeadOffset[2][4];  // 0:WAD, 1:Vision, 2:Color, 3;Angle

	stPositionParam()
	{
		dSafetyStageX = 0.0;
		dSafetyHeadY1 = 0.0;
		dSafetyHeadY2 = 0.0;
		dSafetyHeadX1 = 0.0;
		dSafetyHeadX2 = 0.0;
		dSafetyZ = 0.0;
		dSafetyZ2 = 0.0;

		dLoaderZUp = 0.0;
		dLoaderZMiddle = 0.0;
		dLoaderZPre = 0.0;
		dLoaderZDown = 0.0;

		dPickerStageX = 0.0;
		dAlignToMeasGap = 300;

		for (int i = 0; i < enumAXIS_COUNT; i++)
		{
			dLoadPosition[i] = 0.0;
			dUnloadPosition[i] = 0.0;
			dMeasPosition[i] = 0.0;
		}

		dProbeChangePos = 0;

		for (int i = 0; i < 3; i++)
		{
			dProbeZChangePos[i] = 0;		
		}		

		for (int i = 0; i < 3; i++)
		{
			dACPosition[i] = 0;
			dACZPosition[i] = 0;
		}

		for (int i = 0; i < 5; i++)
		{
			dProbeSafety[i] = 0;
		}

		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				dHeadOffset[i][j].dX = 0;
				dHeadOffset[i][j].dY = 0;
				dHeadOffset[i][j].dZ = 0;
			}
		}

		for (int i = 0; i < 5; i++)
		{
			dProbeXOffsetPos[i] = 0.;
			dProbeYOffsetPos[i] = 0.;
		}

		dGantryGap = 500;
		dHeadGap = 400;

		dAlignPos = 0;
	}
};

class CIniHelper;
class CPositionConfigMgr : public CAuroraSubMgr
{
    CIniHelper			*m_pIniHelper;	
	CIniHelper			*m_pIniHelper2;

	stPositionParam	m_stPositionParam;


public:

	stPositionParam GetPositionParam() const;

	void GetSafetyPosition(double *dSafetyStageX, double* dSafetyHeadX1, double* dSafetyHeadX2, double *dSafetyHeadY1, double *dSafetyHeadY2, double *dSafetyZ, double* dSafetyZ2) const;
	void SetSafetyPosition(double dSafetyStageX, double dSafetyHeadX1, double dSafetyHeadX2, double dSafetyHeadY1, double dSafetyHeadY2, double dSafetyZ, double dSafetyZ2);

	void GetLoaderZPossition(double *dLoaderZUp, double *dLoaderZMiddle, double* dLoaderZPre, double *dLoaderZDown) const;
	void SetLoaderZPossition(double dLoaderZUp, double dLoaderZMiddle, double dLoaderZPre, double dLoaderZDown = 0);

	void GetLoaderLoadPosition(double *dLoadPositionX, double *dLoadPositionY, double *dLoadPositionR) const;
	void SetLoaderLoadPosition(double dLoadPositionX, double dLoadPositionY, double dLoadPositionR);

	void GetLoaderUnloadPosition(double *dUnLoadPositionX, double *dUnLoadPositionY, double *dUnLoadPositionR) const;
	void SetLoaderUnloadPosition(double dUnLoadPositionX, double dUnLoadPositionY, double dUnLoadPositionR);

	void GetLoaderMeasPosition(double *dMeasPositionX, double *dMeasPositionY, double *dMeasPositionR) const;
	void SetLoaderMeasPosition(double dMeasPositionX, double dMeasPositionY, double dMeasPositionR);

	void GetPickerPosition(double *dSafetyStageX) const;
	void GetAlignToMeasGap(double *dGap) const;

	void GetProbeSafetyPosition(double* dSafetyProbe1, double* dSafetyProbe2, double* dSafetyProbe3, double* dSafetyProbe4, double* dSafetyProbe5) const;
	void SetProbeSafetyPosition(double dSafetyProbe1, double dSafetyProbe2, double dSafetyProbe3, double dSafetyProbe4, double dSafetyProbe5);

	void GetACPosition(double* dChangePos, double* dSafetyPos, double* dSaccPos) const;
	void SetACPosition(double dChangePos, double dSafetyPos, double dSaccPos);
	void GetACZPosition(double* dChangePos, double* dSafetyPos, double* dSaccPos) const;
	void SetACZPosition(double dChangePos, double dSafetyPos, double dSaccPos);

	void GetProbeChangePos(double* dChangePos) const;
	void SetProbeChangePos(double dChangePos);
	void GetProbeZChangePos(double* dChangePos, double* dSafetyPos, double* dZeroPos) const;
	void SetProbeZChangePos(double dChangePos);

	void GetHeadOffset(int iHead, int iMode, double* dPosX, double* dPosY, double* dPosZ) const;
	void GetHeadGap(double* dPos) const;
	void GetGantryGap(double* dPos) const;

	

	void GetProbeOffsetPos(int iProbe, double* dProbeX, double* dProbeY, double* dProbeT ) const;

	void GetAlignPosition(double* dAlignPos) const;

	bool LinkIniFile2(bool bSave = false);

//------------------------------------------------------------------
// 공통 인터페이스 함수
//------------------------------------------------------------------
public:
	virtual bool LinkIniFile(bool bSave) override;

private:
	virtual void Finalize() override;
public:
    CPositionConfigMgr(CString strConfigPath);
    virtual ~CPositionConfigMgr();
};