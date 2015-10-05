/**
Copyright (c) 
Audi Autonomous Driving Cup. All rights reserved.
 
Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3.  All advertising materials mentioning features or use of this software must display the following acknowledgement: “This product includes software developed by the Audi AG and its contributors for Audi Autonomous Driving Cup.”
4.  Neither the name of Audi nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY AUDI AG AND CONTRIBUTORS “AS IS” AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL AUDI AG OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/


/**
 *
 *AADC_Spurerkennung
 *
 *BRICKL_CHRISTOPH, AEV
 */


#include "stdafx.h"
#include "spurerkennung.h"

using namespace std;
using namespace cv;


ADTF_FILTER_PLUGIN("Spurerkennung", OID_ADTF_Spurerkennung, cSpurerkennung)

cSpurerkennung::cSpurerkennung(const tChar* __info) : cFilter(__info)
{
	SetPropertyInt("FadenkreuzH", 240);
	SetPropertyBool("FadenkreuzH" NSSUBPROP_ISCHANGEABLE, tTrue); 

	SetPropertyInt("FadenkreuzV", 360);
	SetPropertyBool("FadenkreuzV" NSSUBPROP_ISCHANGEABLE, tTrue);

	SetPropertyInt("Grenze V Links", 0);
	SetPropertyBool("Grenze V Links" NSSUBPROP_ISCHANGEABLE, tTrue);
	SetPropertyInt("Grenze V Links" NSSUBPROP_MINIMUM  , 0);
	SetPropertyInt("Grenze V Links" NSSUBPROP_MAXIMUM  , 640);

	SetPropertyInt("Grenze V Rechts", 640);
	SetPropertyBool("Grenze V Rechts" NSSUBPROP_ISCHANGEABLE, tTrue);
	SetPropertyInt("Grenze V Rechts" NSSUBPROP_MINIMUM  , 0);
	SetPropertyInt("Grenze V Rechts" NSSUBPROP_MAXIMUM  , 640);


	SetPropertyInt("Grenze H Oben", 0);
	SetPropertyBool("Grenze H Oben" NSSUBPROP_ISCHANGEABLE, tTrue);
	SetPropertyInt("Grenze H Oben" NSSUBPROP_MINIMUM  , 0);
	SetPropertyInt("Grenze H Oben" NSSUBPROP_MAXIMUM  , 480);

	SetPropertyInt("Grenze H Unten", 480);
	SetPropertyBool("Grenze H Unten" NSSUBPROP_ISCHANGEABLE, tTrue);
	SetPropertyInt("Grenze H Unten" NSSUBPROP_MINIMUM  , 0);
	SetPropertyInt("Grenze H Unten" NSSUBPROP_MAXIMUM  , 480);

	SetPropertyInt("ThresholdValue", 0);
	SetPropertyBool("ThresholdValue" NSSUBPROP_ISCHANGEABLE, tTrue);

	SetPropertyInt("Diff", 1);
	SetPropertyBool("Diff" NSSUBPROP_ISCHANGEABLE, tTrue);

	SetPropertyInt("50cm-Marke", 277);
	SetPropertyBool("50cm-Marke" NSSUBPROP_ISCHANGEABLE, tTrue);
	SetPropertyInt("70cm-Marke", 244);
	SetPropertyBool("70cm-Marke" NSSUBPROP_ISCHANGEABLE, tTrue);
	SetPropertyInt("90cm-Marke", 225);
	SetPropertyBool("90cm-Marke" NSSUBPROP_ISCHANGEABLE, tTrue);
	SetPropertyInt("110cm-Marke", 215);
	SetPropertyBool("110cm-Marke" NSSUBPROP_ISCHANGEABLE, tTrue);
	SetPropertyInt("130cm-Marke", 207);
	SetPropertyBool("130cm-Marke" NSSUBPROP_ISCHANGEABLE, tTrue);
	SetPropertyInt("150cm-Marke", 199);
	SetPropertyBool("150cm-Marke" NSSUBPROP_ISCHANGEABLE, tTrue);
	SetPropertyInt("170cm-Marke", 195);
	SetPropertyBool("170cm-Marke" NSSUBPROP_ISCHANGEABLE, tTrue);
	SetPropertyInt("190cm-Marke", 191);
	SetPropertyBool("190cm-Marke" NSSUBPROP_ISCHANGEABLE, tTrue);
}

cSpurerkennung::~cSpurerkennung()
{
#if defined(WIN32)
	cv::destroyWindow("laneAssistance");
#endif
}

tResult cSpurerkennung::Init(tInitStage eStage, __exception )
{
    RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr));

	if (eStage == StageFirst)
	{
		// Video Input
		RETURN_IF_FAILED(m_oVideoInputPin.Create("Video_Input", IPin::PD_Input, static_cast<IPinEventSink*>(this)));
        RETURN_IF_FAILED(RegisterPin(&m_oVideoInputPin));
		//GLC Output
		cObjectPtr<IMediaType> pCmdType = NULL;
        RETURN_IF_FAILED(AllocMediaType(&pCmdType, MEDIA_TYPE_COMMAND, MEDIA_SUBTYPE_COMMAND_GCL, __exception_ptr));
        RETURN_IF_FAILED(m_oGLCOutput.Create("GLC_Output",pCmdType, static_cast<IPinEventSink*> (this)));
        RETURN_IF_FAILED(RegisterPin(&m_oGLCOutput));

	}
	else if (eStage == StageNormal)
	{
	firstFrame = tTrue;
	imagecount = 0;

	printingpointsnr=0;

	m_nFadenkreuzH = GetPropertyInt("FadenkreuzH");
	m_nFadenkreuzV = GetPropertyInt("FadenkreuzV");
	m_nGrenzeVLinks = GetPropertyInt("Grenze V Links");
	m_nGrenzeVRechts = GetPropertyInt("Grenze V Rechts");
	m_nGrenzeHOben = GetPropertyInt("Grenze H Oben");
	m_nGrenzeHUnten = GetPropertyInt("Grenze H Unten");
	m_nthresholdvalue = GetPropertyInt("ThresholdValue");
	m_nDiff=GetPropertyInt("Diff");
	m_50cmMarke=GetPropertyInt("50cm-Marke");
	m_70cmMarke=GetPropertyInt("70cm-Marke");
	m_90cmMarke=GetPropertyInt("90cm-Marke");
	m_110cmMarke=GetPropertyInt("110cm-Marke");
	m_130cmMarke=GetPropertyInt("130cm-Marke");
	m_150cmMarke=GetPropertyInt("150cm-Marke");
	m_170cmMarke=GetPropertyInt("170cm-Marke");
	m_190cmMarke=GetPropertyInt("190cm-Marke");
#if defined(WIN32)
	namedWindow("laneAssistance", WINDOW_NORMAL);
#endif
	}
	RETURN_NOERROR;
}

tResult cSpurerkennung::PropertyChanged(const char* strProperty)
{
	m_nFadenkreuzH = GetPropertyInt("FadenkreuzH");
	m_nFadenkreuzV = GetPropertyInt("FadenkreuzV");
	m_nGrenzeVLinks = GetPropertyInt("Grenze V Links");
	m_nGrenzeVRechts = GetPropertyInt("Grenze V Rechts");
	m_nGrenzeHOben = GetPropertyInt("Grenze H Oben");
	m_nGrenzeHUnten = GetPropertyInt("Grenze H Unten");
	m_nthresholdvalue = GetPropertyInt("ThresholdValue");
	m_nDiff=GetPropertyInt("Diff");
	m_50cmMarke=GetPropertyInt("50cm-Marke");
	m_70cmMarke=GetPropertyInt("70cm-Marke");
	m_90cmMarke=GetPropertyInt("90cm-Marke");
	m_110cmMarke=GetPropertyInt("110cm-Marke");
	m_130cmMarke=GetPropertyInt("130cm-Marke");
	m_150cmMarke=GetPropertyInt("150cm-Marke");
	m_170cmMarke=GetPropertyInt("170cm-Marke");
	m_190cmMarke=GetPropertyInt("190cm-Marke");


RETURN_NOERROR;
}

tResult cSpurerkennung::Shutdown(tInitStage eStage, ucom::IException** __exception_ptr)
{
	switch (eStage)
		{
		case cFilter::StageFirst:
		{
			//destroyWindow("image2");
			break;
		}
		default:
			break;
		}
    return cFilter::Shutdown(eStage,__exception_ptr);
}

tResult cSpurerkennung::OnPinEvent(IPin* pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample* pMediaSample)
{
 	RETURN_IF_POINTER_NULL(pMediaSample);
	RETURN_IF_POINTER_NULL(pSource);
	if(nEventCode == IPinEventSink::PE_MediaSampleReceived)
	{
		if(pSource == &m_oVideoInputPin)
		{
			//Videoformat
			if (firstFrame)
			{		
				cObjectPtr<IMediaType> pType;
				RETURN_IF_FAILED(m_oVideoInputPin.GetMediaType(&pType));
				cObjectPtr<IMediaTypeVideo> pTypeVideo;
				RETURN_IF_FAILED(pType->GetInterface(IID_ADTF_MEDIA_TYPE_VIDEO, (tVoid**)&pTypeVideo));
				const tBitmapFormat* pFormat = pTypeVideo->GetFormat();								
				if (pFormat == NULL)
				{
					LOG_ERROR("Spurerkennung: No Bitmap information found on pin \"input\"");
					RETURN_ERROR(ERR_NOT_SUPPORTED);
				}
				m_sInputFormat.nPixelFormat = pFormat->nPixelFormat;
				m_sInputFormat.nWidth = pFormat->nWidth;
				m_sInputFormat.nHeight =  pFormat->nHeight;
				m_sInputFormat.nBitsPerPixel = pFormat->nBitsPerPixel;
				m_sInputFormat.nBytesPerLine = pFormat->nBytesPerLine;
				m_sInputFormat.nSize = pFormat->nSize;
				m_sInputFormat.nPaletteSize = pFormat->nPaletteSize;
				firstFrame = tFalse;
			}
			
			ProcessInput(pMediaSample);
		} 
	
	}
	if (nEventCode== IPinEventSink::PE_MediaTypeChanged)
			{
				if (pSource == &m_oVideoInputPin)
				{
						cObjectPtr<IMediaType> pType;
						RETURN_IF_FAILED(m_oVideoInputPin.GetMediaType(&pType));
						cObjectPtr<IMediaTypeVideo> pTypeVideo;
						RETURN_IF_FAILED(pType->GetInterface(IID_ADTF_MEDIA_TYPE_VIDEO, (tVoid**)&pTypeVideo));
						UpdateImageFormat(m_oVideoInputPin.GetFormat());
				}
		}

RETURN_NOERROR;
}

tResult cSpurerkennung::ProcessInput(IMediaSample* pSample)
{
		// VideoInput
		RETURN_IF_POINTER_NULL(pSample);

		cObjectPtr<IMediaSample> pNewRGBSample;
	
		const tVoid* l_pSrcBuffer;

		if (IS_OK(pSample->Lock(&l_pSrcBuffer)))
		{
			
			IplImage* img = cvCreateImageHeader(cvSize(m_sInputFormat.nWidth, m_sInputFormat.nHeight), IPL_DEPTH_8U, 3);
			img->imageData = (char*)l_pSrcBuffer;
			//Übergang von OpenCV1 auf OpenCV2
			Mat image(cvarrToMat(img));
			cvReleaseImage(&img);
			pSample->Unlock(l_pSrcBuffer);
			//Zuschneiden des Bildes
			Mat imagecut;
			if ((m_nGrenzeHUnten<m_sInputFormat.nHeight) && (m_nGrenzeVRechts >m_sInputFormat.nWidth))
				imagecut= image(cv::Range(m_nGrenzeHOben, m_nGrenzeHUnten), cv::Range(m_nGrenzeVLinks, m_nGrenzeVRechts)).clone();	
			else
				imagecut=image;
			//Erzeugen eines Graustufenbildes
			cvtColor(imagecut, grey,CV_RGB2GRAY);
			//Treshold um Binärbild zu erhalten
			threshold(grey, greythresh, m_nthresholdvalue, 500,THRESH_BINARY);
			//Kantendedektion
			Canny(greythresh, linecanny, 0, 2, 3, tFalse);
			cannysize=linecanny.size();
#if defined(WIN32)			
			// Möglichkeit das Bild in einem extra Fenster für Debug Zwecke anzeigen zu lassen
			////Ausgabe des Bildes
			//
			//if(imagecount>14)
			//{
			//	imagecount=0;
			imshow("laneAssistance",linecanny);
			//	waitKey(1);
			//}
			//imagecount++;
#endif
			//LOG_INFO(cString::Format("Breite des Bildes: %i",cannysize.width));
			//LOG_INFO(cString::Format("Höhe des Bildes: %i",cannysize.height));	
			
			//LOG_INFO("Search");
			Search( allpoint1, &lfdnr1, 0 ,50);	//50cm
			Search( allpoint2, &lfdnr2, 33,50);	//70cm
			Search( allpoint3, &lfdnr3, 52,50);	//90cm
			Search( allpoint4, &lfdnr4, 62,50);	//110cm
			Search( allpoint5, &lfdnr5, 70,50);	//130cm
			Search( allpoint6, &lfdnr6, 78,50);	//150cm
			Search( allpoint7, &lfdnr7, 82,50);	//170cm
			Search( allpoint8, &lfdnr8, 86,50);	//190cm

			//LOG_INFO(cString::Format("lfdnr1: %i",lfdnr1));
			//LOG_INFO(cString::Format("lfdnr2: %i",lfdnr2));
			//LOG_INFO(cString::Format("lfdnr3: %i",lfdnr3));
			//LOG_INFO(cString::Format("lfdnr4: %i",lfdnr4));
			//LOG_INFO(cString::Format("lfdnr5: %i",lfdnr5));
			//LOG_INFO(cString::Format("lfdnr6: %i",lfdnr6));
			//LOG_INFO(cString::Format("lfdnr7: %i",lfdnr7));
			//LOG_INFO(cString::Format("lfdnr8: %i",lfdnr8));

			//LOG_INFO("Planing");
			Planing(allpoint1, allpoint2, &lfdnr1, &lfdnr2);	//50cm auf 70cm
			Planing(allpoint2, allpoint3, &lfdnr2, &lfdnr3);	//70cm auf 90cm
			Planing(allpoint3, allpoint4, &lfdnr3, &lfdnr4);	//90cm auf 110cm
			Planing(allpoint4, allpoint5, &lfdnr4, &lfdnr5);	//110cm auf 130cm
			Planing(allpoint5, allpoint6, &lfdnr5, &lfdnr6);	//130cm auf 150cm
			Planing(allpoint6, allpoint7, &lfdnr6, &lfdnr7);	//150cm auf 170cm
			Planing(allpoint7, allpoint8, &lfdnr7, &lfdnr8);	//170cm auf 190cm

			CreateAndTransmitGCL();
		}
	RETURN_NOERROR;			
}

tResult cSpurerkennung::ProcessFound()
{		

	RETURN_NOERROR;
}

tResult cSpurerkennung::ProcessOutput()
{
	
	RETURN_NOERROR;
}

tResult cSpurerkennung::CreateAndTransmitGCL()
{
#if defined(WIN32)
	if (!m_oGLCOutput.IsConnected())
    {
        RETURN_NOERROR;
    }

    cObjectPtr<IMediaSample> pSample;
    RETURN_IF_FAILED(AllocMediaSample((tVoid**)&pSample));
    
    RETURN_IF_FAILED(pSample->AllocBuffer(8192));

    pSample->SetTime(_clock->GetStreamTime());

    tUInt32* aGCLProc;
    RETURN_IF_FAILED(pSample->WriteLock((tVoid**)&aGCLProc));
	
    tUInt32* pc = aGCLProc;
	//Zeichnen
	//Rechteck für anpassung an Videosignal, Autoscale im Videodisplay aktivieren
    cGCLWriter::StoreCommand(pc, GCL_CMD_FGCOL, cColor(0, 0, 0).GetRGBA());	
	cGCLWriter::StoreCommand(pc, GCL_CMD_DRAWRECT, 0, 0, m_sInputFormat.nWidth, m_sInputFormat.nHeight);
	//Zugeschnittenes Window
	cGCLWriter::StoreCommand(pc, GCL_CMD_FGCOL, cColor(255, 0, 255).GetRGBA());
	cGCLWriter::StoreCommand(pc, GCL_CMD_DRAWLINE, m_nGrenzeVLinks, m_nGrenzeHOben, m_nGrenzeVRechts, m_nGrenzeHOben);
	cGCLWriter::StoreCommand(pc, GCL_CMD_DRAWLINE, m_nGrenzeVLinks, m_nGrenzeHUnten, m_nGrenzeVRechts, m_nGrenzeHUnten);	
	cGCLWriter::StoreCommand(pc, GCL_CMD_DRAWLINE, m_nGrenzeVLinks, m_nGrenzeHOben, m_nGrenzeVLinks, m_nGrenzeHUnten);
    cGCLWriter::StoreCommand(pc, GCL_CMD_DRAWLINE, m_nGrenzeVRechts, m_nGrenzeHOben, m_nGrenzeVRechts, m_nGrenzeHUnten);
	//Fadenkreuz
	cGCLWriter::StoreCommand(pc, GCL_CMD_FGCOL, cColor(255, 0, 0).GetRGBA());
	cGCLWriter::StoreCommand(pc, GCL_CMD_DRAWLINE, 0, m_nFadenkreuzH, m_sInputFormat.nWidth, m_nFadenkreuzH);
	cGCLWriter::StoreCommand(pc, GCL_CMD_DRAWLINE, m_nFadenkreuzV, 0, m_nFadenkreuzV, m_sInputFormat.nHeight);

	cGCLWriter::StoreCommand(pc, GCL_CMD_FGCOL, cColor(255,70,0).GetRGBA());
	//gefundene Linien zeichnen
	for(tInt i=0;i<=printingpointsnr-2;i=i+2)
	{
		cGCLWriter::StoreCommand(pc, GCL_CMD_DRAWLINE, printingpoints[i].x, printingpoints[i].y, printingpoints[i+1].x, printingpoints[i+1].y);
	}
	//LOG_INFO(cString::Format("Anzahl der gezeichneten Linien: %i",printingpointsnr/2));
	printingpointsnr=0;
	
	cGCLWriter::StoreCommand(pc, GCL_CMD_END);

    pSample->Unlock(aGCLProc);

    RETURN_IF_FAILED(m_oGLCOutput.Transmit(pSample));
	RETURN_NOERROR;
#else
	RETURN_NOERROR;
#endif
}

tResult cSpurerkennung::Search(sPoint *points,tUInt8 *lfdnr,tUInt8 offset,tUInt8 limit)
{
	//Hier wird ausgehend vom binären Bild der Kantendedektion Punkte auf Höhe der mit Offset beschriebenen Linien gesucht
	//Kriterien sind u.a. Abstand zweier Kanten größer 5 bzw. kleiner limit
	 ASSERT_IF_POINTER_NULL(points)
	 ASSERT_IF_POINTER_NULL(lfdnr)

	*lfdnr=0;
	tInt colomlast=0;

	for(tInt colom=1;colom<=cannysize.width;colom++)
	{					
		if(linecanny.at<uchar>(m_nGrenzeHUnten-1-m_nGrenzeHOben-offset,colom)!=0) 
		{	
			if(abs(colomlast-colom)>5 && abs(colomlast-colom)<limit && colomlast!=0)
			{
				*lfdnr=*lfdnr+1;
				points[*lfdnr].x=(tInt) colom-(abs(colomlast-colom)/2)+m_nGrenzeVLinks;
				points[*lfdnr].y=m_nGrenzeHUnten-1-offset;
				colomlast=0;
			}
			else
			{
				colomlast=colom;
			}
		}
	}
	RETURN_NOERROR;
}

tResult cSpurerkennung::Planing(sPoint *points1, sPoint *points2,tUInt8 *lfdnrp1,tUInt8 *lfdnrp2)
{
	//Hier werden die Linien ermittelt
	//Es werden Punkte der Horizontalen Linien miteinander verglichen
	//Falls die Differenz der beiden X-Koordinaten kleiner als das Property "Diff" ist, Entsteht eine Linie

	for(tInt i=1;i<=*lfdnrp1;i++)
	{
		for(tInt u=1;u<=*lfdnrp2;u++)
		{
			tInt Diff=abs(points1[i].x-points2[u].x);
			//LOG_INFO(cString::Format("Diff: %i",Diff));
			if(Diff<= m_nDiff) 
			{
				printingpoints[printingpointsnr].x=points1[i].x;
				printingpoints[printingpointsnr].y=points1[i].y;

				printingpoints[printingpointsnr+1].x=points2[u].x;
				printingpoints[printingpointsnr+1].y=points2[u].y;


				printingpointsnr=printingpointsnr+2;

				//LOG_INFO(cString::Format("Save Points   -> X1: %i, Y1: %i, X2: %i, Y2: %i",allpoint1[i].x,allpoint1[i].y,allpoint2[u].x,allpoint2[u].y));
				
			}
		}
	}

	RETURN_NOERROR;
}

tResult cSpurerkennung::UpdateImageFormat(const tBitmapFormat* pFormat)
{
	if (pFormat != NULL)
	{
		m_sInputFormat = (*pFormat);		
		LOG_INFO(adtf_util::cString::Format("Spurerkennung Filter Input: Size %d x %d ; BPL %d ; Size %d , PixelFormat; %d",m_sInputFormat.nWidth,m_sInputFormat.nHeight, 
			m_sInputFormat.nBytesPerLine, m_sInputFormat.nSize, m_sInputFormat.nPixelFormat));			
	}
	
	RETURN_NOERROR;
}
