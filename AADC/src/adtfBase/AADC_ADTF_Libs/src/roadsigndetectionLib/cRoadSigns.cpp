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

namespace roadsignDetection
{

cRoadSigns::cRoadSigns(cString filename,RoadSign enumGiven , tInt minHessian, tInt shapeIdentifier) : m_detector(minHessian,20) //setting parameters of detector minHessian and nOctaves
{
	//m_image = imread(filename, CV_LOAD_IMAGE_GRAYSCALE );	
	m_enumSign = enumGiven;
	m_shapeIdentifier = shapeIdentifier;
	if (std::ifstream(filename.GetPtr()))
	{
		m_image = imread(filename.GetPtr(), CV_LOAD_IMAGE_COLOR);
		//blur(m_image,m_image,cvSize(9,9));
		m_minHessian = minHessian;
#if defined(WIN32)
		//setting parameters of detector
		m_detector.set("hessianThreshold",minHessian);
		m_detector.set("nOctaves",20);
#endif

		m_detector.detect( m_image, m_keypoints );
		
		if (m_keypoints.size() == 0)
			{
			m_state=tFalse;
			return;
			}
		//remove keypoints which are near to border
		tInt borderSize = m_image.cols/4;	 	 
		m_keypoints.erase( remove_if(m_keypoints.begin(), m_keypoints.end(),
								   RoiPredicatePic((tFloat32)borderSize, (tFloat32)borderSize,
												(tFloat32)(m_image.cols - borderSize),
												(tFloat32)(m_image.rows - borderSize))),
						 m_keypoints.end() );
	
		m_extractor.compute( m_image, m_keypoints, m_descriptors );

		if (m_descriptors.total() == 0)
			{
			m_state=tFalse;
			return;
			}

		//drawKeypoints( m_image, m_keypoints, m_image, Scalar::all(-1), DrawMatchesFlags::DEFAULT );

		m_state = tTrue;
	}
	else
		m_state=tFalse;
}

cRoadSigns::~cRoadSigns()
{
}

tInt cRoadSigns::match(Mat inputImage)
{
	if (m_state)
		{
		Mat descriptors;
		std::vector<KeyPoint> keypoints;	
		
		m_detector.detect( inputImage, keypoints );
	
		//remove keypoints which are near to border
		tInt borderSize = inputImage.cols/4;	 
		keypoints.erase( remove_if(keypoints.begin(), keypoints.end(),
								   RoiPredicatePic((tFloat32)borderSize, (tFloat32)borderSize,
												(tFloat32)(inputImage.cols - borderSize),
												(tFloat32)(inputImage.rows - borderSize))),
						 keypoints.end() );
	
		m_extractor.compute( inputImage, keypoints, descriptors );	

		std::vector< DMatch > matches;

		//doing the matching
		m_matcher.match(descriptors,m_descriptors, matches );
		
		//limits for maximum and minimum distance
		tFloat64 min_dist = 0;
		if (m_shapeIdentifier == 1)
			min_dist = 0.1;
		else if (m_shapeIdentifier == 2)
			min_dist = 0.2;
		else
			min_dist = 5;
		//-- Quick calculation of max and min distances between keypoints
			
		//for( tInt i = 0; i < descriptors.rows; i++ )
		//	{ tFloat64 dist = matches[i].distance;
		//		if( dist < min_dist ) min_dist = dist;
		//		if( dist > max_dist ) max_dist = dist;
		//	}
		
		//creating vector with good matches; lesser distance is better then higher distance
		std::vector< DMatch > good_matches;
		for( tInt i = 0; i < descriptors.rows; i++ )
			{ if( matches[i].distance <= max(2*min_dist, 0.02) )
			{ good_matches.push_back( matches[i]); }
			}
		
	
		//drawKeypoints( inputImage, keypoints, inputImage, Scalar::all(-1), DrawMatchesFlags::DEFAULT );

		drawMatches( inputImage, keypoints,m_image, m_keypoints, 
						good_matches, m_img_matches, Scalar::all(-1), Scalar::all(-1),
						vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );

		return static_cast<tInt> (good_matches.size());
		}
	else 
		return 0;
}

tInt cRoadSigns::matchDescriptors(Mat descriptors)
{
	if (m_state)
		{
		std::vector< DMatch > matches;

		//doing the matching
		m_matcher.match(descriptors,m_descriptors, matches );
		
		//limits for maximum and minimum distance
		tFloat64 min_dist=0;
		if (m_shapeIdentifier == 1)
			min_dist = 0.2;
		else if (m_shapeIdentifier == 2)
			min_dist = 0.2;
		else
			min_dist = 0.1;
		//-- Quick calculation of max and min distances between keypoints
			
		//for( tInt i = 0; i < descriptors.rows; i++ )
		//	{ tFloat64 dist = matches[i].distance;
		//		if( dist < min_dist ) min_dist = dist;
		//		if( dist > max_dist ) max_dist = dist;
		//	}
		
		//creating vector with good matches; lesser distance is better then higher distance
		std::vector< DMatch > good_matches;
		for( tInt i = 0; i < descriptors.rows; i++ )
			{ if( matches[i].distance <= max(2*min_dist, 0.02) )
			{ good_matches.push_back( matches[i]); }
			}
		
	
		//drawKeypoints( inputImage, keypoints, inputImage, Scalar::all(-1), DrawMatchesFlags::DEFAULT );

		/*drawMatches( inputImage, keypoints,m_image, m_keypoints, 
						good_matches, m_img_matches, Scalar::all(-1), Scalar::all(-1),
						vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );*/

		return static_cast<tInt> (good_matches.size());
		}
	else 
		return 0;
}

tVoid cRoadSigns::drawMatchesOfSign()
{
#if defined(WIN32)
	if (m_state)
		{
		cv::imshow("matching result",m_img_matches);
		HWND hWnd = (HWND)cvGetWindowHandle("matching result"); 
		::SendMessage(hWnd, WM_PAINT, 0, 0);
		}
#endif
}

Mat cRoadSigns::getImage()
{
	if (m_state)
		{
		return m_image;
		}
	else return Mat(180, 180, CV_8UC3, Scalar(0));
}



}

