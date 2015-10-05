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
#ifndef _ROADSIGN_DETECTION_INTERFACE_H_
#define _ROADSIGN_DETECTION_INTERFACE_H_

#if   defined _WIN32
#define LIB_PRE __declspec(dllexport)
#elif defined __unix__
#define LIB_PRE __attribute__ ((visibility ("default")))
#else
#define LIB_PRE 
#endif


namespace roadsignDetection
{
//! interface class for the road sign detection lib
/*!
This is the interface class for the library for the road sign detection lib
*/
class IRoadsignDetection
{
public:
		/*! initializes the library and sets the resolution
		@param imgWidthIn width of the input images
		@param imgHeightIn height of the input images
		@param imgBytesPerLineIn bytes per line of the input images
		*/
		virtual tBool init(tInt imgWidthIn, tInt imgHeightIn, tInt imgBytesPerLineIn) = 0;
		
		/*! sets the parameter with the given name to the new value
		@param paramName name of the parameter
		@param paramValue new value of the parameter
		*/
		virtual tVoid setParam(cString paramName, tFloat64 paramValue)=0;
		
		/*! sets the width of the input images
		@param inWidth value width of the images
		*/
		virtual tVoid setWidth(tInt inWidth) =0;
		
		/*! sets the height of the input images
		@param inHeight value for height of the images
		*/
		virtual tVoid setHeight(tInt inHeight) =0;
		
		/*! sets the bytes per line of the input images
		@param inBytesPerLine value for the bytes per line of the images
		*/
		virtual tVoid setBytesPerLine(tInt inBytesPerLine) =0;
		
		/*! gives a new image to the libary. The image is processed and the detected sign is given back by the result integer. The image data, of which the pointer is given must be according to the predefined width, height and bytes per line
		@param imgRawData pointer to the image data. 
		*/
		virtual tInt update(tUInt8 *imgRawData) = 0;
		
		/*! returns a pointer to the processd data. The processed data contains an image with the plotted contours
		*/
		virtual tVoid* getProcessedData()=0;
		
		/*! returns the bytes per line of the processed image data
		*/
		virtual tInt getProcImgBytesPerLine()=0;
		
		/*! returns the size of the processed image data
		*/
		virtual tInt getProcImgSize() =0;
		
		/*! returns the bits per pixel of the processed image data
		*/
		virtual tInt getProcImgBitsPerPixel() =0;
		
		/*! shuts the library down
		*/
		virtual tBool shutdown() = 0;
		
		/*! destructor
		*/
		virtual ~IRoadsignDetection(){};

};

/*! creates a new class
@param srcFolderPath the path where the images of the roadsigns are located
*/
LIB_PRE IRoadsignDetection* CreateMyClass(cString srcFolderPath);

/*! deletes the class
@param ptrToClass pointer to the class which has to be deleted
*/
LIB_PRE tVoid DestroyMyClass(IRoadsignDetection* ptrToClass);


}

#endif // _ROADSIGN_DETECTION_INTERFACE_H_