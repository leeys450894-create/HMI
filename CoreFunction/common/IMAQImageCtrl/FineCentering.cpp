#include "pch.h"
#include "FineCentering.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CFineCentering::CFineCentering(void)
{
    __Spec__threshold			= 20;	//DEFAULT_THRESHOLD;
    __Spec__kernel				= 5;	//
    __Spec__width				= 5;	//DEFAULT_WIDTH;
    __Spec__steepness			= 2;	//DEFAULT_STEEPNESS;
    __Spec__subsamplingRatio	= 3;	//DEFAULT_SUBSAMPLING_RATIO;

	__Result__ptPoint				= imaqMakePointFloat(-9999.f, -9999.f);
	__Result__ptPoint_center_based	= imaqMakePointFloat(-9999.f, -9999.f);

	__Result__pImage = NULL;
	__Egde__pImage = NULL;
}

CFineCentering::~CFineCentering(void)
{
  //  imaqDispose(__Result__pImage);
  //  imaqDispose(__Egde__pImage);
}

bool CFineCentering::Run(Image *pImage, CRect rtROI, bool bEllipse)
{
    //-------------------------------------------------------
    // verification parameters [ image, roi ]
    //-------------------------------------------------------
    int w = 0, h = 0;
    imaqGetImageSize(pImage, &w, &h);
    if((w <= 0) || (h <= 0))
		return false;

    rtROI &= CRect(0, 0, w, h);
    if(rtROI.IsRectNull())
        return false;
		
	//-------------------------------------------------------
    // initialize result buffer
    //-------------------------------------------------------
	__Result__ptPoint				= imaqMakePointFloat(-9999.f, -9999.f);
	__Result__ptPoint_center_based	= imaqMakePointFloat(-9999.f, -9999.f);


	//in case Color image
	ImageType type;
	imaqGetImageType((const Image*)pImage, &type);
	if(type == IMAQ_IMAGE_RGB || type == IMAQ_IMAGE_HSL ||type == IMAQ_IMAGE_RGB_U64)
	{
		__Result__pImage = imaqCreateImage(IMAQ_IMAGE_RGB, 1);
		imaqDuplicate(__Result__pImage, pImage);

		__Egde__pImage = imaqCreateImage(IMAQ_IMAGE_U8, 1);
		imaqDuplicate(__Egde__pImage, pImage);

		imaqExtractColorPlanes(pImage, IMAQ_HSI, NULL, NULL, __Egde__pImage);
		
	}
	else
	{
		__Result__pImage = imaqCreateImage(IMAQ_IMAGE_U8, 1);
		imaqDuplicate(__Result__pImage, pImage);

		__Egde__pImage = imaqCreateImage(IMAQ_IMAGE_U8, 1);
		imaqDuplicate(__Egde__pImage, pImage);	
	}
	
	imaqEdgeFilter(__Egde__pImage, __Egde__pImage, IMAQ_EDGE_SOBEL, NULL);

#if _DEBUG
	imaqWriteJPEGFile(__Egde__pImage, "D:\\Temp\\test1.jpg", 750, NULL);
#endif

	//-------------------------------------------------------
    // Find Center
    //-------------------------------------------------------
	const long nW = rtROI.Width();	// vertical length
	const long nH = rtROI.Height();	// horizontal length

	if(bEllipse)
	{
		//-------------------------------------------------------
		// rect edge detect + Ellipse fitting
		//-------------------------------------------------------
		if (!__Find_Edge_Ellipse(__Egde__pImage, rtROI, nW, nH))
		{
			imaqDispose(__Result__pImage);
			imaqDispose(__Egde__pImage);
			return false;
		}
			
	}
	else
	{
		//-------------------------------------------------------
		// rect edge detect + line fitting
		//-------------------------------------------------------
		if(!__Find_Edge_SquareRect(__Egde__pImage, rtROI, nH, IMAQ_LEFT_TO_RIGHT))
		{
			imaqDispose(__Result__pImage);
			imaqDispose(__Egde__pImage);
			return false;
		}

		//	return false;

		if(!__Find_Edge_SquareRect(__Egde__pImage, rtROI, nW, IMAQ_TOP_TO_BOTTOM))
		{
			imaqDispose(__Result__pImage);
			imaqDispose(__Egde__pImage);
			return false;
		}

		//	return false;
	}
	__Result__ptPoint_center_based.x = __Result__ptPoint.x - (w / 2.f);
	__Result__ptPoint_center_based.y = __Result__ptPoint.y - (h / 2.f);


    //-------------------------------------------------------
    // draw result
    //-------------------------------------------------------
	imaqOverlayRect(__Result__pImage, imaqMakeRect(rtROI.top, rtROI.left, rtROI.Height(), rtROI.Width()), &IMAQ_RGB_WHITE, IMAQ_DRAW_VALUE , NULL);

	const Point ptCenterPoint = imaqMakePoint((int)__Result__ptPoint.x, (int)__Result__ptPoint.y);
	imaqOverlayPoints(__Result__pImage, &ptCenterPoint, 1, &IMAQ_RGB_RED, 1, IMAQ_POINT_AS_CROSS, NULL, NULL);

	// overlay 를 이미지에 적용시키기
    imaqMergeOverlay(__Result__pImage, __Result__pImage, NULL, NULL, NULL);

#if _DEBUG
	imaqWriteJPEGFile(__Result__pImage, "D:\\Temp\\MergeOverlay2.jpg", 750, NULL);
//	imaqWriteJPEGFile(__Result__pImage, "D:\\Temp\\test2.jpg", 750, NULL);
#endif

	imaqDispose(__Result__pImage);
	imaqDispose(__Egde__pImage);

	return true;
}

bool CFineCentering::__Find_Edge_SquareRect(Image *pImage, CRect rtROI, long nRakeNum, RakeDirection direction)
{
    if(nRakeNum <= 0)
	{
        return false;
	}

	//-------------------------------------------------------
    // find edge
    //-------------------------------------------------------
    SEdgeDataBuffer edge;
	edge.vecData1.clear();
	edge.vecData2.clear();
    edge.vecData1.resize(nRakeNum, imaqMakePointFloat(0.f, 0.f));
	edge.vecData2.resize(nRakeNum, imaqMakePointFloat(0.f, 0.f));

    __EdgeDetection_Engine(pImage, rtROI, nRakeNum, direction, edge);
	
	if((edge.vecData1.size() < 3) || (edge.vecData2.size() < 3))
	{
        return false;
	}

    //-------------------------------------------------------
    // 찾은 edge 들의 그룹들로 피팅을 수행한다.
    //-------------------------------------------------------
    FitLineOptions fitLineOptions;
    fitLineOptions.minScore			= 80;//DEFAULT_MIN_LINE_SCORE;
    fitLineOptions.numRefinements	= 10; //30;//DEFAULT_REFINEMENTS;
    fitLineOptions.pixelRadius		= 1;//DEFAULT_PIXEL_RADIUS;

    BestLine *pLine1 = imaqFitLine(&edge.vecData1[0], (int)edge.vecData1.size(), &fitLineOptions);
    BestLine *pLine2 = imaqFitLine(&edge.vecData2[0], (int)edge.vecData2.size(), &fitLineOptions);

    if((pLine1 == NULL) || (pLine2 == NULL))
    {
        imaqDispose(pLine1);
        imaqDispose(pLine2);
        return false;
    }

    //-------------------------------------------------------
    // 두 직선 사이의 거리를 구한다. (중심 찾기)
    //-------------------------------------------------------
    if(!__GetDistance_LineToLine(pImage, pLine1, pLine2, direction))
	{
		imaqDispose(pLine1);
		imaqDispose(pLine2);
		return false;
	}

    //-------------------------------------------------------
	// 두 방향에 대한 각도를 계산한다. ( 평균 각도 )
    //-------------------------------------------------------
	__GetAngle_LineANDLine(pLine1, pLine2, direction); 
	
	imaqDispose(pLine1);
	imaqDispose(pLine2);
	
	return true;
}

void CFineCentering::__EdgeDetection_Engine(Image *pImage, CRect rtROI, long nRakeNum, RakeDirection direction, SEdgeDataBuffer &data)
{
    EdgeOptions2 edgeOptions;
	edgeOptions.polarity			 = IMAQ_SEARCH_FOR_ALL_EDGES;
	edgeOptions.kernelSize			 = __Spec__kernel;
	edgeOptions.width				 = 3;__Spec__width;	// 1, 3, 5 형태를 갖어야만 한다.
	edgeOptions.minThreshold		 = (float)__Spec__threshold;
	edgeOptions.interpolationType	 = IMAQ_BILINEAR_FIXED;
	edgeOptions.columnProcessingMode = IMAQ_AVERAGE_COLUMNS;
//     edgeOptions.polarity			 = IMAQ_SEARCH_FOR_RISING_EDGES;
//     edgeOptions.kernelSize			 = __Spec__kernel;
//     edgeOptions.width				 = __Spec__width;	// 1, 3, 5 형태를 갖어야만 한다.
//     edgeOptions.minThreshold		 = (float)__Spec__threshold;
//     edgeOptions.interpolationType	 = IMAQ_BILINEAR_FIXED;
//     edgeOptions.columnProcessingMode = IMAQ_AVERAGE_COLUMNS;

	long dir;
    // Edge 는 항상 두 방향에 대해서 한번씩 한다. [ 0 : LEFT, RIGHT ] [ 1 : TOP, BOTTOM ]
    for(dir = 0; dir < 2; dir++)
    {
        PointFloat *pEdge = NULL;

		if(dir == 0) pEdge = &data.vecData1[0];
		if(dir == 1) pEdge = &data.vecData2[0];

        for(long nline = 0; nline < nRakeNum; nline += __Spec__subsamplingRatio)
        {
            Point start, end;
            if(dir == 0)
            {
                if(direction == IMAQ_LEFT_TO_RIGHT)
                {
                    start = imaqMakePoint(rtROI.left,  rtROI.top + nline);
                    end   = imaqMakePoint(rtROI.right, rtROI.top + nline);
                }
                else if(direction == IMAQ_TOP_TO_BOTTOM)
                {
                    start = imaqMakePoint(rtROI.left + nline, rtROI.top);
                    end   = imaqMakePoint(rtROI.left + nline, rtROI.bottom);
                }
            }
            else if(dir == 1)
            {
                if(direction == IMAQ_LEFT_TO_RIGHT)
                {
                    start = imaqMakePoint(rtROI.right, rtROI.top + nline);
                    end   = imaqMakePoint(rtROI.left,	 rtROI.top + nline);
                }
                else if(direction == IMAQ_TOP_TO_BOTTOM)
                {
                    start = imaqMakePoint(rtROI.left + nline, rtROI.bottom);
                    end   = imaqMakePoint(rtROI.left + nline,	rtROI.top);
                }
            }

            // 실제 Edge 를 찾는다.
            ROI *pROI = imaqCreateROI();
            imaqAddLineContour(pROI, start, end);

			EdgeReport2 *pEdgereport = imaqEdgeTool4(pImage, pROI, IMAQ_FIRST, &edgeOptions, FALSE);

            if(pEdgereport)
            {
                if(0 < pEdgereport->numEdges)
                {
                    pEdge[nline] = pEdgereport->edges[0].position;
                }
            }

            imaqDispose(pEdgereport);

            imaqRemoveContour(pROI, imaqGetContour(pROI, 0));
            imaqDispose(pROI);
        }
    }

    for( dir = 0; dir < 2; dir++)
    {
		if(dir == 0)
		{
			if(!data.vecData1.empty())
			{
				const PointFloat *pEdge = &data.vecData1[0];

				for(int i = (int)data.vecData1.size() - 1; i >= 0; --i)
				{
					if((pEdge[i].x == 0.f) && (pEdge[i].y == 0.f))
					{
						data.vecData1.erase(data.vecData1.begin() + i);
					}
				}
			}
		}

		if(dir == 1)
		{
			if(!data.vecData2.empty())
			{
				const PointFloat *pEdge = &data.vecData2[0];

				for(int i = (int)data.vecData2.size() - 1; i >= 0; --i)
				{
					if((pEdge[i].x == 0.f) && (pEdge[i].y == 0.f))
					{
						data.vecData2.erase(data.vecData2.begin() + i);
					}
				}
			}
		}
    }

    //-------------------------------------------------------
    // 찾은 Edge 위치를 화면에 표시해주기 위해서..
    //-------------------------------------------------------
    for( dir = 0; dir < 2; dir++)
    {
		vector<PointFloat> vecEdge;

		if(dir == 0) vecEdge = data.vecData1;
		if(dir == 1) vecEdge = data.vecData2;		

        if(!vecEdge.empty())
        {
            const PointFloat *pEdge = &vecEdge[0];

            for(size_t i = 0; i < vecEdge.size(); ++i)
            {
				const Point ptEdgePoint = imaqMakePoint((int)pEdge[i].x, (int)pEdge[i].y);

				//20200327 jychoi
				imaqOverlayPoints(__Result__pImage, &ptEdgePoint,  2, &IMAQ_RGB_GREEN, 1, IMAQ_POINT_AS_CROSS, NULL, NULL);
				//imaqOverlayPoints(__Result__pImage, &ptEdgePoint,  1, &IMAQ_RGB_GREEN, 1, IMAQ_POINT_AS_PIXEL, NULL, NULL);
            }
        }
    }    
}

bool CFineCentering::__GetDistance_LineToLine(Image *pImage, BestLine *pLine1, BestLine *pLine2, RakeDirection direction)
{
    //------------------------------------------------------------------------------
    // 한개의 직선의 수직인 직선을 만들어 낸다.
    //------------------------------------------------------------------------------
    const double Line1_cx = (pLine1->start.x + pLine1->end.x ) / 2.0;
    const double Line1_cy = (pLine1->start.y + pLine1->end.y ) / 2.0;

    BestLine lineCross;
    lineCross.equation.a = pLine1->equation.b;
    lineCross.equation.b = -pLine1->equation.a;
    lineCross.equation.c = pLine1->equation.a * Line1_cy - pLine1->equation.b * Line1_cx;

    const double det = pLine2->equation.b * lineCross.equation.a - lineCross.equation.b * pLine2->equation.a;

    if( fabs(det) >= 1.0e-10 )
    {
        //------------------------------------------------------------------------------
        // 두 직선의 교차점을 찾고 직선간의 거리를 구한다.
        //------------------------------------------------------------------------------
        const double dcrossPoint_x	= (pLine2->equation.c * lineCross.equation.b - lineCross.equation.c * pLine2->equation.b) / det;
        const double dcrossPoint_y	= (pLine2->equation.a * lineCross.equation.c - lineCross.equation.a * pLine2->equation.c) / det;
        const double magnitude		= sqrt(pLine1->equation.a * pLine1->equation.a + pLine1->equation.b * pLine1->equation.b);
        const double distance		= (fabs(pLine1->equation.a * dcrossPoint_x + pLine1->equation.b * dcrossPoint_y + pLine1->equation.c)) / magnitude;

        //------------------------------------------------------------------------------
        // 두 직선간의 중심을 반환한다.
        //------------------------------------------------------------------------------
        if(direction == IMAQ_LEFT_TO_RIGHT)
		{
			__Result__ptPoint.x = (float)(Line1_cx + (distance / 2.0));

			return true;
		}

        if(direction == IMAQ_TOP_TO_BOTTOM)
		{
			__Result__ptPoint.y = (float)(Line1_cy + (distance / 2.0));
			return true;
		}
    }

    return false;
}

bool CFineCentering::__GetAngle_LineANDLine(BestLine *pLine1, BestLine *pLine2, RakeDirection direction)
{
	const float fAngle1 = (float)(atan2( -pLine1->equation.a, pLine1->equation.b ) * 180.0 / IMAQ_PI);

	const float fAngle2 = (float)(atan2( -pLine2->equation.a, pLine2->equation.b ) * 180.0 / IMAQ_PI);

	const float fAngle  = (fAngle1 + fAngle2) / 2.f;
	
	if(direction == IMAQ_LEFT_TO_RIGHT)
	{
		__Result__fRotate_Angle_degree = 90.f + fAngle;
	}

	if(direction == IMAQ_TOP_TO_BOTTOM)
	{
		__Result__fRotate_Angle_degree += fAngle;
		__Result__fRotate_Angle_degree /= 2.f;
	}

	return true;
}





//------------------------------------------------------------------------
// Edge 검사 알고리즘 4 - User Ellipse
//------------------------------------------------------------------------
bool CFineCentering::__Find_Edge_Ellipse(Image *pImage, CRect rtROI, int nW, int nH)
{
    //-------------------------------------------------------
    // find edge
    //-------------------------------------------------------
    SEdgeDataBuffer edge[2];
	edge[0].vecData1.clear();
	edge[0].vecData2.clear();
    edge[0].vecData1.resize(nH);
	edge[0].vecData2.resize(nH);
	edge[1].vecData1.clear();
	edge[1].vecData2.clear();
    edge[1].vecData1.resize(nW);
	edge[1].vecData2.resize(nW);

    __EdgeDetection_Engine(pImage, rtROI, nH, IMAQ_LEFT_TO_RIGHT, edge[0]);
    __EdgeDetection_Engine(pImage, rtROI, nW, IMAQ_TOP_TO_BOTTOM, edge[1]);

	if( (edge[0].vecData1.size() < 3) || (edge[0].vecData2.size() < 3) ||
		(edge[1].vecData1.size() < 3) || (edge[1].vecData2.size() < 3) )
        return false;

    vector<PointFloat> vecEdgeAll;
    vecEdgeAll.insert( vecEdgeAll.end(), edge[0].vecData1.begin(), edge[0].vecData1.end());
    vecEdgeAll.insert( vecEdgeAll.end(), edge[0].vecData2.begin(), edge[0].vecData2.end());
    vecEdgeAll.insert( vecEdgeAll.end(), edge[1].vecData1.begin(), edge[1].vecData1.end());
    vecEdgeAll.insert( vecEdgeAll.end(), edge[1].vecData2.begin(), edge[1].vecData2.end());

    //-------------------------------------------------------
    // 찾은 edge 들의 그룹들로 피팅을 수행한다.
    //-------------------------------------------------------
    FitEllipseOptions fitEllipseOptions;
    fitEllipseOptions.rejectOutliers	= TRUE;
    fitEllipseOptions.minScore			= DEFAULT_MIN_LINE_SCORE;
    fitEllipseOptions.pixelRadius		= 1;
    fitEllipseOptions.maxIterations		= (int)vecEdgeAll.size();

    BestEllipse2 *pEllipse = imaqFitEllipse2(&vecEdgeAll[0], (int)vecEdgeAll.size(), &fitEllipseOptions);

    if(pEllipse)
    {
		//-------------------------------------------------------
		// 중심 찾기
		//-------------------------------------------------------
        if(pEllipse->numPointsUsed)
        {
			__Result__ptPoint				= pEllipse->center;
			__Result__fRotate_Angle_degree	= 0.f;

			imaqDispose(pEllipse);
			return true;
        }
    }

    imaqDispose(pEllipse);
    return false;
}