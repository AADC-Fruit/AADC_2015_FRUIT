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
#ifndef ROADSIGNHEADERS
#define ROADSIGNHEADERS

#ifndef M_PI
    #define M_PI 3.14159265358979323846f
#endif  // M_PI

namespace roadsignDetection
{


/*!
this struct holds the region of intereset of the predicted picture
*/
struct RoiPredicatePic
{
    /*! constructor which sets the region of interest in the image
	@param _minX the minimum limit for the x axis
	@param _minY the minimum limit for the y axis
	@param _maxX the maximum limit for the x axis
	@param _maxY the maximum limit for the y axis
	*/
	RoiPredicatePic(tFloat32 _minX, tFloat32 _minY, tFloat32 _maxX, tFloat32 _maxY)
    : minX(_minX), minY(_minY), maxX(_maxX), maxY(_maxY)
    {}
	/*! checks if the point is in the region of interested
	@param keyPt point which should be checked
	*/
    tBool operator()( const cv::KeyPoint& keyPt) const
    {
        cv::Point2f pt = keyPt.pt;
        return (pt.x < minX) || (pt.x >= maxX) || (pt.y < minY) || (pt.y >= maxY);
    }
	/*! the minimum limit for the x axis*/
	tFloat32 minX;
	/*! the minimum limit for the y axis*/
	tFloat32 minY;
	/*! the maximum limit for the x axis*/
	tFloat32 maxX;
	/*! the maximum limit for the y axis*/
	tFloat32 maxY;
};
/*!
enum for the different supported roadsign
*/
enum RoadSign{
		VorfahrtGewaehren=1,
		HaltVorfahrtGewaehren=2,
		VorgeschriebeneFahrtrichtungGeradeaus=3,
		ZulaessigeHoechstGeschwindigkeit=4,
		Vorfahrt=5,
		ParkenAufGehwegen=6,
	}; 

}

#include "IRoadsignDetection.h"
#include "cRoadSigns.h"

#include "algorithm/ihls_nhs/ihls.h"
#include "algorithm/ihls_nhs/math_utils.h"
#include "algorithm/ihls_nhs/nhs.h"

#include "CRoadsignDetectionDLL.h"



#define resultTrian 10 //counter of good matches of triangular signs to detect sign with maximum number of good matches
#define resultCirc 20	//counter of good matches of circular signs to detect sign with maximum number of good matches
#define matchesLimit  0				//minimum limit for good matches in all other shapes than triangle
#define matchesLimitTriangle  0		//minimum limit for good matches in triangle
// parameters for traffic sign detection	
#define minArea  1000.0 //minArea which is recognized as possible sign
#define maxArea  20000.0 //minArea which is recognized as possible sign
#define maxError 20000 //maxError which is used in detecting circular signs ( error between calculated area of an ellipse and the measured area of countour
#define minAspectRatio 0.6  // min aspectration between heigth and width to filter out contours with unlikely shapes
#define maxAspectRatio 1.4 	 // max aspectration between heigth and width to filter out contours with unlikely shapes
#define minCosineValueTriangle   0.4  //minimum value of cosinus of corner in triangle
#define maxCosineValueTriangle   0.6  //maximum value of cosinus of corner in triangle
#define factorForBorderSize 4 // the keypoints at the border of the image are deleted if they are nearer than inputImage.cols/factorForBorderSize or inputImage.rows/factorForBorderSize



#endif //ROADSIGNHEADERS
