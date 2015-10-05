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
* $Author:: spiesra $  $Date:: 2014-09-16 09:23:21#$ $Rev:: 26076   $
**********************************************************************/
#ifndef ROADSIGN_H
#define ROADSIGN_H

namespace roadsignDetection
{
//! class for one road sign
	/*!
	this class holds one traffic sign and is used in the roadsign detection
	*/
class cRoadSigns
{
public:
	/*! loads image from file and creates keypoints in it
	@param filename filename of image (absolute path)
	@param enumGiven Roadsign enum of this roadsign
	@param minHessian matrix for SurfFeatureDetector
	@param shapeIdentifier 1=circular, 2=triangular;
	*/
	cRoadSigns(cString filename, RoadSign enumGiven, tInt minHessian, tInt shapeIdentifier);
	
	~cRoadSigns();

	/*! matches the input image with the image of the object and returns number of good matches
	@param inputImage input image to be matched
	*/
	tInt match(cv::Mat inputImage);
	
	/*! matches the input descriptors with the image of the object and returns number of good matches
	@param descriptors descriptors to be matched
	*/
	tInt matchDescriptors(cv::Mat descriptors);

	/*!draw the matches of the comparing algorithm*/
	tVoid drawMatchesOfSign();

	/*! returns the image of traffic sign*/
	cv::Mat getImage();

	/*! returns to tInt converted enum of this sign*/
	tInt getEnumOfSign() {return tInt(m_enumSign);}
private:
	/*! shapeIdentifier: 1=circular, 2=triangular */
	tInt m_shapeIdentifier;
	/*! enum of this roadsign*/
	RoadSign m_enumSign;
	/*! template image */
	cv::Mat m_image;
	/*! image with the included matches*/
	cv::Mat m_img_matches;
	/*! hessian matrix forS urfFeatureDetector */
	tInt m_minHessian;
	/*! the descriptors of the traffic sign*/
	cv::Mat m_descriptors;
	/*! the keypoints of the traffic sign*/
	std::vector<cv::KeyPoint> m_keypoints;
	/*! the state of the object; tFalse if file could not be opened*/
	tBool m_state;
	cv::SurfDescriptorExtractor m_extractor;
	cv::SurfFeatureDetector m_detector;			

	cv::FlannBasedMatcher m_matcher;
};

}

#endif