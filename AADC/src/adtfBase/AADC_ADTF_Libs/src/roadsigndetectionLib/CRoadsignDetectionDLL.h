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
* $Author:: spiesra $  $Date:: 2014-09-16 11:29:51#$ $Rev:: 26093   $
**********************************************************************/

namespace roadsignDetection
{
//! class for road sign detection
	/*!
	this class was developed as prototyp for the road sign detection. It can be used as basic implemention for further work. It loads images in the defined source and is used by the update member function
	The processing in this class is based on the code published under https://sites.google.com/site/mcvibot2011sep/home. For further documentation refer to this site.
	*/
class CRoadsignDetection: public IRoadsignDetection
{
public:
	/*! constructor of the class
	@param srcFolderPath the path where the images of the roadsigns are located
	*/
	CRoadsignDetection(cString srcFolderPath);

	/*! loads image from file and appends this sign to list
	@param filename filename of image (absolute path)
	@param enumGiven enum of this roadsign
	@param minHessian matrix for SurfFeatureDetector
	@param shapeIdentifier 1=circular, 2=triangular;
	*/
	tVoid LoadFileSign(cString filename,RoadSign enumGiven, tInt minHessian, tInt shapeIdentifier);
		
	virtual ~CRoadsignDetection();
	/*! inits the dll with the width, height and BytesPerLine
	@param imgWidthIn width of input data
	@param imgHeightIn Height of input data
	@param imgBytesPerLineIn BytesPerLineIn of input data	
	*/
	virtual tBool init(tInt imgWidthIn, tInt imgHeightIn, tInt imgBytesPerLineIn);

	/*! over this function new rawdata can be put in the dll
	@param imgRawData pointer to rawdata
	*/
	virtual tInt update(tUInt8 *imgRawData);

	/*! over this function the processed data can be fetched*/
	virtual tVoid *getProcessedData();

	/*! shuts the filter down */
	virtual tBool shutdown();

	/*! sets the parameter with the given name to the new value
	@param paramName name of the parameter
	@param paramValue new value of the parameter
	*/
	virtual tVoid setParam(cString paramName, tFloat64 paramValue);
	
	/*! get the Bytesperline of the processed Image*/
	virtual tInt getProcImgBytesPerLine() {return this->m_procImgBytesPerLine;}
	/*! get the ImgSize of the processed Image*/
	virtual tInt getProcImgSize() {return this->m_procImgSize;}
	/*! get the BitsPerPixel of the processed Image*/
	virtual tInt getProcImgBitsPerPixel() {return this->m_procBitsPerPixel;}

	/*! sets the width of the input data
	@param inWidth new width of the input data
	*/
	virtual tVoid setWidth(tInt inWidth) {m_imgWidth=inWidth;}
	/*! sets the height of the input data
	@param inHeight new height of the input data
	*/
	virtual tVoid setHeight(tInt inHeight) {m_imgHeight=inHeight;}
	/*! sets the Bytesperline of the input data
	@param inBytesPerLine new Bytesperline of the input data
	*/
	virtual tVoid setBytesPerLine(tInt inBytesPerLine) {m_imgBytesPerLine=inBytesPerLine;}

private:
	/*! width of image*/
	tInt m_imgWidth;
	/*! height of image*/
	tInt m_imgHeight;
	/*! bytes per line of image*/
	tInt m_imgBytesPerLine;
	/*! bytes per line of image which is given back*/
	tInt m_procImgBytesPerLine;
	/*! width of image which is given back*/
	tInt m_procImgSize;
	/*! size of image which is given back*/
	tInt m_procBitsPerPixel;
	/*! bits per Pixel of image which is given back*/
	
	/*! contains input frame */
	cv::Mat m_frame;
	/*! source folder where the templates of the signs are stored*/ 
	cString m_srcFolderPath;

	/*! extractor for the extraction of keypoints from the images*/
	SurfDescriptorExtractor m_extractor;
	/*! surf detector for image detection*/
	SurfFeatureDetector m_detector;
	
	/*!vector which contains the road signs with triangle shape */
	vector<cRoadSigns> m_triangleRoadSignsDB;
	/*!vector which contains the road signs with circular shape and red color*/
	vector<cRoadSigns> m_redCircRoadSignsDB;

	/*! doing the process of the given rawdata */
	tInt process();
	
	/*! matches the input image to all loaded templates
	@param inputImage image to be compared
	@param shapeSelector selects which shape the image is 0=triangle, 1=circular
	*/
	tInt match(Mat inputImage, tInt shapeSelector);


	/*! showing the result of the circular algorithm 
	@param image image of detected sign
	@param number of good matches
	*/
	tVoid showCircResult(Mat image,tInt matches);
	
	/*! showing the result of the circular algorithm 
	@param image image of detected sign
	@param number of good matches
	*/
	tVoid showTrianResult(Mat image,tInt matches);
	
	/*! showing the result of the traffic light recognition
	@param image image of detected sign
	*/
	tVoid showTrafficLight(Mat image,cString inText);
	};



}
    
