/**
Copyright (c) 
Audi Autonomous Driving Cup. All rights reserved.
 
Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3.  All advertising materials mentioning features or use of this software must display the following acknowledgement: “This product includes software developed by the Audi AG and its contributors for Audi Autonomous Driving Cup.”
4.  Neither the name of Audi nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY AUDI AG AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL AUDI AG OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


**********************************************************************
* $Author:: forchhe#$  $Date:: 2014-09-11 10:10:54#$ $Rev:: 25921   $
**********************************************************************/
#include "stdafx.h"
#include "roadsignDetectionHeaders.h"


using namespace std;

using namespace cv;

#if defined(WIN32)
	tBool APIENTRY DllMain( HMODULE hModule,
						   DWORD  ul_reason_for_call,
						   LPVOID lpReserved
						 )
	{
		switch (ul_reason_for_call)
		{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
		}
		return tTrue;

	}
#endif

#define CONCAT2(a,b)     cString(a)+cString(b)

// helper function:
// finds a cosine of angle between vectors
// from pt0->pt1 and from pt0->pt2
static tFloat64 angle( Point pt1, Point pt2, Point pt0 )
{
    tFloat64 dx1 = pt1.x - pt0.x;
    tFloat64 dy1 = pt1.y - pt0.y;
    tFloat64 dx2 = pt2.x - pt0.x;
    tFloat64 dy2 = pt2.y - pt0.y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

namespace roadsignDetection
{


CRoadsignDetection::CRoadsignDetection(cString srcFolderPath)
{
	m_srcFolderPath = srcFolderPath;
	//hessian matrix for SurfFeatureDetector, varies the number of keypoints which are genereted in image
	tInt minHessian = 200;		
		
	//loads templates of traffic signs  1: triangular signs; 2: all other
	LoadFileSign(CONCAT2(m_srcFolderPath,"/205_VorfahrtGewaehren.png"),VorfahrtGewaehren,minHessian,1);
	LoadFileSign(CONCAT2(m_srcFolderPath,"/206_Halt!VorfahrtGewaehren.png"),HaltVorfahrtGewaehren,minHessian,2);
	LoadFileSign(CONCAT2(m_srcFolderPath,"/209_VorgeschriebeneFahrtrichtungGeradeaus.png"),VorgeschriebeneFahrtrichtungGeradeaus,minHessian,2);
	LoadFileSign(CONCAT2(m_srcFolderPath,"/274_ZulaessigeHoechstGeschwindigkeit.png"),ZulaessigeHoechstGeschwindigkeit,minHessian,2);
	LoadFileSign(CONCAT2(m_srcFolderPath,"/301_Vorfahrt.png"),Vorfahrt,minHessian,1);
	//LoadFileSign(CONCAT2(m_srcFolderPath,"/315_ParkenAufGehwegen.png"),ParkenAufGehwegen,minHessian,2);
}

tVoid CRoadsignDetection::LoadFileSign(cString filename,RoadSign enumGiven, tInt minHessian, tInt shapeIdentifier )
{
	Mat image = imread(filename.GetPtr(), CV_LOAD_IMAGE_COLOR);
	if(! image.data )                              // Check for invalid input
		{
			return;			
		}
	else
		{
		if (shapeIdentifier == 2) m_redCircRoadSignsDB.push_back(cRoadSigns(filename,enumGiven,minHessian,shapeIdentifier));
		else if (shapeIdentifier == 1) m_triangleRoadSignsDB.push_back(cRoadSigns(filename,enumGiven,minHessian,shapeIdentifier));
		}
}

CRoadsignDetection::~CRoadsignDetection()
{
/*
	cv::destroyWindow("matching result");
	cv::destroyWindow("detected triangular sign");
	cv::destroyWindow("detected circular sign");
	cv::destroyWindow("detected traffic light");
*/
}

tBool CRoadsignDetection::init(tInt imgWidthIn, tInt imgHeightIn, tInt imgBytesPerLineIn)
{
		this->m_imgWidth = imgWidthIn;
		this->m_imgHeight = imgHeightIn;
		this->m_imgBytesPerLine = imgBytesPerLineIn;
/*
		cv::namedWindow("matching result",CV_WINDOW_AUTOSIZE);
		cv::namedWindow("detected triangular sign",CV_WINDOW_AUTOSIZE);
		cv::namedWindow("detected circular sign",CV_WINDOW_AUTOSIZE);
*/


		return tTrue;		
}

tInt CRoadsignDetection::update(tUInt8 *imgRawData)
{
	tInt bestResultItem=0;
	if (imgRawData!=NULL)
		{
		m_frame = cv::Mat(m_imgHeight,m_imgWidth,CV_8UC3,imgRawData,m_imgBytesPerLine);
		bestResultItem = process();
		}
	return bestResultItem;
}

tInt CRoadsignDetection::process()
{
	tInt bestResultItem = 0;


//-------------------------------------

// traffic sign detection algorithm
	 //cvtColor(m_frame, m_frame, CV_BGR2RGB);
	
	//convert iamge to ihls color space
	Mat ihls_image = convert_rgb_to_ihls(m_frame);	
	//convert iamge to nhs color space (for red color)
	Mat nhs_image = convert_ihls_to_nhs(ihls_image);	
	//convert iamge to nhs color space (for blue color)
	nhs_image += convert_ihls_to_nhs(ihls_image,1);

	Mat canny_output;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	
	/// Detect edges using canny
	//Canny( nhs_image, canny_output, 100, 100, 3 );
	
	/// Find contours
	findContours( nhs_image, contours, RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );	
	
	// drawing convexHull of the contours
	//for( tInt i = 0; i < contours.size(); i++ ) convexHull( Mat(contours[i]), contours[i], tFalse ); 
	
	
	for( tUInt i = 0; i< contours.size(); i++ )
		{
		
		tFloat64 actual_area = fabs(contourArea(contours[i]));	
		
		//filters unlikely areas 
		if ((actual_area > minArea) && (actual_area < maxArea)) 
			{			
			Rect boundRect = boundingRect( Mat(contours[i]) );
			tFloat32 contourAspectRatio = ((tFloat32)boundRect.width/(tFloat32)boundRect.height); //calculates aspect ratio between height and width
			//drawContours(m_frame, contours, i, Scalar(255, 0, 255),2, 8, hierarchy, 0, Point() ); //plot all contours
				
				//detects if contour is a likely sign due to the ratio of height and width					
				if ((contourAspectRatio<maxAspectRatio)  && (contourAspectRatio>minAspectRatio) )		
				{
				vector<Point> approxTriangle;
				approxPolyDP(contours[i], approxTriangle, arcLength(Mat(contours[i]), tTrue)*0.05, tTrue);
				//detects if contour is triangle
				if(approxTriangle.size() == 3)			
					{					
					tFloat64 cosine = 0;
					for( tInt j = 2; j < 4; j++ )
							{
								// find the average cosine of the angle between joint edges; a equilateral triangle as angles of 60 degree eacg
								cosine += fabs(angle(approxTriangle[j%4], approxTriangle[j-2], approxTriangle[j-1]));									
							}
					//detects if contour is equilateral triangle 
					if(( minCosineValueTriangle < cosine/3  ) && ( cosine/3 < maxCosineValueTriangle )) //cos(60) = 0.5 
						{
						//doing matching 
						bestResultItem = match(m_frame(boundingRect(contours[i])),1);						
						drawContours(m_frame, contours, i, Scalar(0, 0, 255),2, 8, hierarchy, 0, Point() );  //triangular shapes are red
						}
					}
				else //all other shapes than triangular
					{
					tInt A = boundRect.width / 2; 
					tInt B = boundRect.height / 2;
					tFloat64 estimated_area = M_PI * A * B;
					//detects whether shape could be an ellipse or an octagon or a rectangle
					if ((fabs(actual_area - estimated_area)<maxError) || (approxTriangle.size() == 8) || (approxTriangle.size() == 4))  			
						{
						//doing matching 
						bestResultItem = match(m_frame(boundingRect(contours[i])),2);						
						drawContours( m_frame, contours, i, Scalar(255, 0, 0 ), 2, 8, hierarchy, 0, Point() );	//other shapes are blue		
						}
					}
				}
			}
		
		}


	
//-------------------------------------
// traffic light detection
/*	
//paramters for traffic light detection:

	tInt minContourAreaTrafficLight = 1000;
	tFloat32 maxCosinusValueForCorners = 0.1;
	tFloat32 minContourAspectRatio = 0.1;
	tFloat32 maxContourAspectRatio = 0.6;

//algorithm for traffic light detection

	Mat mask;
	inRange(m_frame, Scalar(0,0, 0), Scalar(40, 40, 40), mask); //filter areas with dark/black color


	Mat canny_output;
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	/// Detect edges using canny
	Canny( mask, canny_output, 100, 100*2, 3 );
	/// Find contours
	findContours( canny_output, contours, hierarchy, RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
	
	for( tInt i = 0; i< contours.size(); i++ )
		{
			if (fabs(contourArea(contours[i]))>minContourAreaTrafficLight)
				{
				convexHull( Mat(contours[i]), contours[i], tFalse);
				vector<Point> approxRect;
				approxPolyDP(contours[i], approxRect, arcLength(Mat(contours[i]), tTrue)*0.02, tTrue);
				//checks if contour is rectangle 		
				if((approxRect.size() == 4) && isContourConvex(Mat(approxRect))) 
								{
								tFloat64 maxCosine = 0;
								for( tInt j = 2; j < 5; j++ )
												{
													// find the maximum cosine of the angle between joint edges
													tFloat64 cosine = fabs(angle(approxRect[j%4], approxRect[j-2], approxRect[j-1]));
													maxCosine = MAX(maxCosine, cosine);
												}
								//checks if contour is rectangle with orthoganal corners
								if( maxCosine < maxCosinusValueForCorners ) //cos(90) = 0
									{
									Rect boundRect = boundingRect( Mat(contours[i]) );
									tFloat32 contourAspectRatio = ((tFloat32)boundRect.width/(tFloat32)boundRect.height); //calculates aspect ratio between height and width
									//checks for correct AspectRatio for the detected traffic lights
									if ((contourAspectRatio>minContourAspectRatio) && (contourAspectRatio<maxContourAspectRatio))
										{
										Mat mask = Mat::zeros(m_frame.size(), CV_8UC1);
										drawContours(mask, contours, i, Scalar(255), CV_FILLED);
										Mat imageROI;
										m_frame.copyTo(imageROI, mask);	
										Scalar s = sum(imageROI);				
										if  (s[2]>s[1])
											 showTrafficLight(imageROI(boundingRect(contours[i])),"Rot");
										else
											showTrafficLight(imageROI(boundingRect(contours[i])),"Gruen");
										drawContours( m_frame, contours, i, Scalar(255, 0, 0 ), 2, 8, hierarchy, 0, Point() );
										}
									}
								}
				}
		}	
*/
//-------------------------------------
	//writing output image
	//m_frame = nhs_image;
	//setting parameters of processed image which are fetched by ADTF
	m_procBitsPerPixel = static_cast<tInt> (m_frame.elemSize() *8);
	m_procImgBytesPerLine = static_cast<tInt> (m_frame.step);
	m_procImgSize = static_cast<tInt> (m_frame.elemSize() * m_frame.total());

	return bestResultItem;
//-------------------------------------



}

tInt CRoadsignDetection::match(Mat inputImage, tInt shapeSelector)
{
	
	Mat descriptors;
	std::vector<KeyPoint> keypoints;	
		
	m_detector.detect( inputImage, keypoints );
	
	//remove keypoints which are near to border	 
			//remove keypoints which are near to border
	tInt borderSize = inputImage.cols/4;	 
	keypoints.erase( remove_if(keypoints.begin(), keypoints.end(),
								   RoiPredicatePic((tFloat32)borderSize, (tFloat32)borderSize,
												(tFloat32)(inputImage.cols - borderSize),
												(tFloat32)(inputImage.rows - borderSize))),
						 keypoints.end() );

	
	m_extractor.compute( inputImage, keypoints, descriptors );		
	tInt bestResultItem=0; //holds the index of the best match
	tInt result =0;
	if (shapeSelector == 1)
		{			
			for (tUInt j = 0; j<m_triangleRoadSignsDB.size(); j++)
			{									
				tInt resultTmp = m_triangleRoadSignsDB[j].matchDescriptors(descriptors);
				if ((resultTmp>resultTrian) && (resultTmp>matchesLimitTriangle))
					{
					bestResultItem = m_triangleRoadSignsDB[j].getEnumOfSign(); // results of triangular signs start with 11
					result = resultTmp;
					showTrianResult(m_triangleRoadSignsDB[j].getImage(),result);
					//m_triangleRoadSignsDB[j].drawMatchesOfSign();	//not supported if only matchDescriptors is called
					}
			}
			
		}
	else if (shapeSelector == 2)
		{
		for (tUInt j = 0; j<m_redCircRoadSignsDB.size(); j++)
			{
				tInt resultTmp = m_redCircRoadSignsDB[j].matchDescriptors(descriptors);
				if ((resultTmp>resultCirc) && (resultTmp>matchesLimit))
					{
					bestResultItem = m_redCircRoadSignsDB[j].getEnumOfSign();	// results of rectangular signs start with 11
					result=resultTmp;
					showCircResult(m_redCircRoadSignsDB[j].getImage(),result);
					//m_redCircRoadSignsDB[j].drawMatchesOfSign();  //not supported if only matchDescriptors is called
					}
			}
		
			
		}
	return bestResultItem;
}

tVoid * CRoadsignDetection::getProcessedData()
{	
	tVoid *data = (tVoid*)(m_frame.data);		
	return data;

}

tVoid CRoadsignDetection::showTrianResult(Mat image, tInt matches)
{
/*
	stringstream ss;
	ss << matches;
	Mat label(60,image.cols,image.type());
	label = cvScalar(255,255,255);
	putText(label, ss.str(), cvPoint(label.rows/2,label.cols/4),FONT_HERSHEY_COMPLEX_SMALL, 2, cvScalar(0,0,0), 2, CV_AA);
		
	Mat showMat;
	vconcat(image,label,showMat);
	imshow("detected circular sign", Mat(image.rows+60,image.cols,CV_8UC3, Scalar(255,255,255)));
	imshow("detected triangular sign", showMat);
*/
}

tVoid CRoadsignDetection::showCircResult(Mat image, tInt matches)
{
/*
	stringstream ss;
	ss << matches;
	Mat label(60,image.cols,image.type());
	label = cvScalar(255,255,255);
	putText(label, ss.str(), cvPoint(label.rows/2,label.cols/4),FONT_HERSHEY_COMPLEX_SMALL, 2, cvScalar(20,20,5), 2, CV_AA);

	Mat showMat;
	vconcat(image,label,showMat);
	imshow("detected triangular sign", Mat(image.rows+60,image.cols,CV_8UC3, Scalar(255,255,255)));
	imshow("detected circular sign", showMat);	
*/
}

tVoid CRoadsignDetection::showTrafficLight(Mat image, cString inText)
{
/*
	resize(image, image, Size(100,200));
	Mat label(60,image.cols,image.type());
	label = cvScalar(255,255,255);
	putText(label, inText, cvPoint(label.rows/2,label.cols/4),FONT_HERSHEY_COMPLEX_SMALL, 1, cvScalar(20,20,5), 2, CV_AA);
	
	Mat showMat;
	vconcat(image,label,showMat);
	imshow("detected traffic light", showMat);
*/
}

tBool CRoadsignDetection::shutdown()
{
	return tTrue;
}

tVoid CRoadsignDetection::setParam(cString paramName, tFloat64 paramValue)
{

}

IRoadsignDetection* CreateMyClass(cString srcFolderPath)
{

	IRoadsignDetection * ret = new CRoadsignDetection(srcFolderPath);
	return ret;
}
       
tVoid DestroyMyClass(IRoadsignDetection* ptrToClass)
{
	if(ptrToClass) delete ptrToClass;
}

}

