#pragma once

//#include <nimachinevision.h>
//#include "NI\nimachinevision.h"
#include "nimachinevision.h"

#include <vector>
using namespace std;

struct SEdgeDataBuffer
{
	vector<PointFloat> vecData1;
	vector<PointFloat> vecData2;
};

class CFineCentering
{
    // spec
    int			__Spec__threshold;        //Specifies the threshold for the contrast of an edge.
    int			__Spec__kernel;			  //Specifies the size of the edge detection kernel.
    int			__Spec__width;            //The number of pixels that the function averages to find the contrast at either side of the edge.
    int			__Spec__steepness;        //The span, in pixels, of the slope of the edge projected along the path specified by the input points.
    int			__Spec__subsamplingRatio; //The number of pixels that separates two consecutive search lines.

    // result
	PointFloat	__Result__ptPoint;
	PointFloat	__Result__ptPoint_center_based;
	float		__Result__fRotate_Angle_degree;
    Image		*__Result__pImage;
	Image		*__Egde__pImage;

public:
	// 검사 수행 전달 함수
    void		SetParam_threshold(long value)				{ __Spec__threshold			= value;								}
    void		SetParam_kernel(long value)					{ __Spec__kernel			= value;								}
    void		SetParam_width(long value)					{ __Spec__width				= value;								}
    void		SetParam_steepness(long value)				{ __Spec__steepness			= value;								}
    void		SetParam_subsamplingratio(long value)		{ __Spec__subsamplingRatio	= value;								}

	// 검사 결과 전달 함수
	PointFloat	GetResult_FoundPoint() const				{ return __Result__ptPoint;											}
	PointFloat	GetResult_FoundPoint_center_based() const	{ return __Result__ptPoint_center_based;							}
	float		GetResult_Rotate_Angle_degree() const		{ return __Result__fRotate_Angle_degree;							}
	Image		*GetResult_OverlayImage() const				{ return __Result__pImage;											}

public:
    bool Run(Image *pSrc, CRect rtROI, bool bEllipse = false);	// gray scale image, region of interest

private:
    bool __Find_Edge_SquareRect(Image *pImage, CRect rtROI, long nRakeNum, RakeDirection direction);
    bool __GetDistance_LineToLine(Image *pImage, BestLine *pLine1, BestLine *pLine2, RakeDirection direction);
	bool __GetAngle_LineANDLine(BestLine *pLine1, BestLine *pLine2, RakeDirection direction);
    void __EdgeDetection_Engine(Image *pImage, CRect rtROI, long nRakeNum, RakeDirection direction, SEdgeDataBuffer &data);

	bool __Find_Edge_Ellipse(Image *pImage, CRect rtROI, int nW, int nH);

public:
    CFineCentering(void);
    ~CFineCentering(void);
};

