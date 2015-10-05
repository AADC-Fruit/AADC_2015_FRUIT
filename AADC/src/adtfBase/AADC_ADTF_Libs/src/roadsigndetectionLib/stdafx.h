/**********************************************************************
* Copyright (c) 2014 BFFT Gesellschaft fuer Fahrzeugtechnik mbH.
* All rights reserved.
**********************************************************************
* $Author:: exthanl#$  $Date:: 2014-09-04 20:28:25#$ $Rev:: 25787   $
**********************************************************************/
// stdafx.h : Includedatei für Standardsystem-Includedateien
// oder häufig verwendete projektspezifische Includedateien,
// die nur in unregelmäßigen Abständen geändert werden.
//

#ifndef _STANDARD_INCLUDES_H_
#define _STANDARD_INCLUDES_H_

// ADTF header
#include <adtf_platform_inc.h>
#include <adtf_plugin_sdk.h>

// windows header for HANDLE etc.
#ifdef WIN32
    #define WIN32_LEAN_AND_MEAN             // Selten verwendete Teile der Windows-Header nicht einbinden.
    #include <Windows.h>    
#endif

#include <fstream>

// opencv header
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <opencv/cxcore.h>
// nonfree header of opencv
#include <opencv2/nonfree/features2d.hpp>
using namespace cv;

#endif // _STANDARD_INCLUDES_H_
