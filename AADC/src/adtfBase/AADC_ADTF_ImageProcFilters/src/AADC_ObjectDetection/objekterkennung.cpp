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
 * AADC_Objekterkennung
 * Author: BRICKL_CHRISTOPH 
 * Date: 2013-08-20 
 */

#include "stdafx.h"
#include "objekterkennung.h"


ADTF_FILTER_PLUGIN("Objekterkennung", OID_ADTF_OBJECTDETECTION, cObjectDetection)

cObjectDetection::cObjectDetection(const tChar* __info) : cFilter(__info)
{
	SetPropertyInt("Capture Mode", 0);
	SetPropertyStr("Capture Mode.ValueList", "0@640x480 30 FPS|1@320x240 60 FPS");
	SetPropertyBool("Capture Mode" NSSUBPROP_ISCHANGEABLE, tTrue);

	SetPropertyInt("Sensibility", 50);
	SetPropertyBool("Sensibility" NSSUBPROP_ISCHANGEABLE, tTrue);
	SetPropertyInt("Sensibility" NSSUBPROP_MIN, 1);
	SetPropertyInt("Sensibility" NSSUBPROP_MAX, 1000);

	SetPropertyInt("minBreiteObjekt", 3);
	SetPropertyBool("minBreiteObjekt" NSSUBPROP_ISCHANGEABLE, tTrue);
	SetPropertyInt("minBreiteObjekt" NSSUBPROP_MIN, 1);
	SetPropertyInt("minBreiteObjekt" NSSUBPROP_MAX, 1000);

	SetPropertyInt("GCL-Offset", 0);
	SetPropertyBool("GCL-Offset" NSSUBPROP_ISCHANGEABLE, tTrue);
	SetPropertyInt("GCL-Offset" NSSUBPROP_MIN, -50);
	SetPropertyInt("GCL-Offset" NSSUBPROP_MAX, 50);
}

cObjectDetection::~cObjectDetection()
{
#if defined(WIN32)
	destroyWindow("objectdetection");
#endif
}

tResult cObjectDetection::Init(tInitStage eStage, __exception )
{
    RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr));

	if (eStage == StageFirst)
	{
		cObjectPtr<IMediaDescriptionManager> pDescManager;
		RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER, IID_ADTF_MEDIA_DESCRIPTION_MANAGER, (tVoid**)&pDescManager, __exception_ptr));
		
		//Data from Xtion Video Input
		RETURN_IF_FAILED(m_oPinInputVideo.Create("Depth_Image_Input", IPin::PD_Input, static_cast<IPinEventSink*>(this)));
        RETURN_IF_FAILED(RegisterPin(&m_oPinInputVideo));
		/*
		////Data from Xtion
		cObjectPtr<IMediaType> pInputType = new cMediaType(0, 0, 0, "tXtionData", pDescManager->GetMediaDescription("tXtionData"));
		RETURN_IF_FAILED(m_oPinInput1.Create("Input_DepthData", pInputType, this));
		RETURN_IF_FAILED(RegisterPin(&m_oPinInput1));
		RETURN_IF_FAILED(pInputType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION_EXT, (tVoid**)&m_pCoderDesc_Input));
		*/
		//Object Output
		cObjectPtr<IMediaType> pOutputType = new cMediaType(0, 0, 0, "tObjData", pDescManager->GetMediaDescription("tObjData"), 0x0000004);
		RETURN_IF_FAILED(m_oOutputPin1.Create("ObjectData", pOutputType, NULL));
		RETURN_IF_FAILED(RegisterPin(&m_oOutputPin1));
		RETURN_IF_FAILED(pOutputType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION_EXT, (tVoid**)&m_pCoderDesc_Output));
		
		//GLC Output
		cObjectPtr<IMediaType> pCmdType = NULL;
        RETURN_IF_FAILED(AllocMediaType(&pCmdType, MEDIA_TYPE_COMMAND, MEDIA_SUBTYPE_COMMAND_GCL, __exception_ptr));
        RETURN_IF_FAILED(m_oGLCOutput.Create("GLC_output",pCmdType, static_cast<IPinEventSink*> (this)));
        RETURN_IF_FAILED(RegisterPin(&m_oGLCOutput));

	}
	else if (eStage == StageNormal)
	{
		m_nSensibility = GetPropertyInt("Sensibility");
		m_nminBreiteObjekt = GetPropertyInt("minBreiteObjekt");
		m_nGCLOffset=GetPropertyInt("GCL-Offset");
		m_nSensibilityneg = m_nSensibility*(-1);

		//Set Resolution an Frames
		if (GetPropertyInt("Capture Mode") == 0)
		{
			m_outputMode.nXRes = 640;
			m_outputMode.nYRes = 480;
			m_outputMode.nFPS = 30;
		}
		else
		{
			m_outputMode.nXRes = 320;
			m_outputMode.nYRes = 240;
			m_outputMode.nFPS = 60;
		}

		m_bAlreadyRead_Input = tFalse;
		m_bAlreadyRead_Output = tFalse;

		m_g_ObjStruct.nObjNr = 0;
		m_g_ObjStruct.nObjXLPos = 0;
		m_g_ObjStruct.nObjXLPixel = 0;
		m_g_ObjStruct.nObjXLDepth = 0;
		m_g_ObjStruct.nObjXMPos = 0;
		m_g_ObjStruct.nObjXMDepth = 0;
		m_g_ObjStruct.nObjXRPos = 0;
		m_g_ObjStruct.nObjXRPixel = 0;
		m_g_ObjStruct.nObjXRDepth = 0;
		m_g_ObjStruct.nGueltig = 0;

#if defined(WIN32)
		namedWindow("objectdetection", CV_WINDOW_NORMAL);
#endif
	}
	else if (eStage == StageGraphReady)
    {
		cObjectPtr<IMediaType> pType;
        RETURN_IF_FAILED(m_oPinInputVideo.GetMediaType(&pType));

        cObjectPtr<IMediaTypeVideo> pTypeVideo;
        RETURN_IF_FAILED(pType->GetInterface(IID_ADTF_MEDIA_TYPE_VIDEO, (tVoid**)&pTypeVideo));

        UpdateImageFormat(pTypeVideo->GetFormat());      
		
    }
	RETURN_NOERROR;
}

tResult cObjectDetection::PropertyChanged(const char* strProperty)
{
		m_nSensibility = GetPropertyInt("Sensibility");
		m_nminBreiteObjekt = GetPropertyInt("minBreiteObjekt");
		m_nSensibilityneg = m_nSensibility*(-1);
		m_nGCLOffset=GetPropertyInt("GCL-Offset");

RETURN_NOERROR;
}


tResult cObjectDetection::Shutdown(tInitStage eStage, ucom::IException** __exception_ptr)
{
	switch (eStage)
		{
		case cFilter::StageFirst:
		{
			m_pCoderDesc_Input=NULL;
			m_pCoderDesc_Output=NULL;
			break;
		}
		default:
			break;
		}
    return cFilter::Shutdown(eStage,__exception_ptr);
}

tResult cObjectDetection::OnPinEvent(IPin* pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample* pMediaSample)
{
    if (pSource == &m_oPinInput1)
	{
		switch (nEventCode)
		{
			case IPinEventSink::PE_MediaSampleReceived:
			{
				RETURN_IF_FAILED(ProcessInput(pMediaSample));
				break;
			}

			case IPinEventSink::PE_MediaTypeChanged:
			{
				cObjectPtr<IMediaType> pType;	
				RETURN_IF_FAILED(pSource->GetMediaType(&pType));
				if (pType != NULL)
				{
					RETURN_IF_FAILED(pType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION_EXT, (tVoid**)&m_pCoderDesc_Input));
				}
				break;
			}
		}
	}
	if (pSource == &m_oPinInputVideo)
	{
		switch (nEventCode)
		{
			case IPinEventSink::PE_MediaSampleReceived:
			{
				RETURN_IF_FAILED(ProcessInputImage(pMediaSample));
				break;
			}

			case IPinEventSink::PE_MediaTypeChanged:
			{
				if (pSource == &m_oPinInputVideo)
				{
						cObjectPtr<IMediaType> pType;
						RETURN_IF_FAILED(m_oPinInputVideo.GetMediaType(&pType));
						cObjectPtr<IMediaTypeVideo> pTypeVideo;
						RETURN_IF_FAILED(pType->GetInterface(IID_ADTF_MEDIA_TYPE_VIDEO, (tVoid**)&pTypeVideo));
						UpdateImageFormat(m_oPinInputVideo.GetFormat());
				}
				
				cObjectPtr<IMediaType> pType;	
				RETURN_IF_FAILED(pSource->GetMediaType(&pType));
				if (pType != NULL)
				{
					RETURN_IF_FAILED(pType->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION_EXT, (tVoid**)&m_pCoderDesc_Input));
				}
				break;
			}
		}
	}
	RETURN_NOERROR;
}


tResult cObjectDetection::ProcessInputImage(IMediaSample* pSample)
{
	//Depth_Information
	RETURN_IF_POINTER_NULL(pSample);

	const tVoid* l_pSrcBuffer;
	
	if (IS_OK(pSample->Lock(&l_pSrcBuffer)))
	{
			Mat depthImage;	
			depthImage.create(m_sInputFormatVideo.nHeight, m_sInputFormatVideo.nWidth, CV_16UC1);	
			depthImage.data = (uchar*)l_pSrcBuffer;
#if defined(WIN32)
			imshow("objectdetection",depthImage);
#endif
			for(tInt16 i=1;i <= m_sInputFormatVideo.nWidth;i++)
				{
					tFloatVector3D aProjective3D[1];
					tFloatVector3D aWorld3D[1];
					tInt PixelY = m_sInputFormatVideo.nHeight/2;

					aProjective3D[0].X= (tFloat32) i;
					aProjective3D[0].Y= (tFloat32) PixelY;
					aProjective3D[0].Z= (tFloat32) depthImage.at<tUInt8>(i,PixelY);  //slow but only one pixel is needed			

					ConvertRealWorldToProjectiveOffline(1, &aProjective3D[0], &aWorld3D[0]);
			
					tVector3D intVector3D;
			
					intVector3D.X = (tInt16) aWorld3D[0].X;
					intVector3D.Y = (tInt16) aWorld3D[0].Y;
					intVector3D.Z = (tInt16) aWorld3D[0].Z;	

					tUInt16 pos;
					pos = i;
					m_Depthraw [pos]= intVector3D.Z;
					m_PixelX [pos]=intVector3D.X;
	
					//Start ProcessFound
					if (pos == m_sInputFormatVideo.nWidth)
						{
							ProcessFoundOffline();
						}			
					}
			pSample->Unlock(l_pSrcBuffer);
	}
	
	
	RETURN_NOERROR;
}

tResult cObjectDetection::ProcessFoundOffline()
{
	//Average determination over 3 points
	m_Depth[1] = m_Depthraw[1];
	m_Depth[m_outputMode.nXRes] = m_Depthraw[m_outputMode.nXRes];
	for(tInt m=2;m<=(m_outputMode.nXRes-1);m++)	
	{
		m_Depth[m]=(m_Depthraw[m-1]+m_Depthraw[m]+m_Depthraw[m+1])/3;
	}

	//Detection-Algo
	for(tInt n=1; n<=(m_outputMode.nXRes-1);n++)
	{	
		//Check if Depth Information OK? (Information != 0)
		if( (m_Depthraw[n]==0) || (m_Depthraw[n+1]==0) )
		{
			m_SprungBewertung[n] = 0;
			m_DeltaDepth [n]= 0;
		}
		else
		{	
			//Calculate the difference between 2 points
			m_DeltaDepth [n] = m_Depth[n]-m_Depth[n+1];
			//Rating Flat
			if(abs(m_DeltaDepth [n]) <= m_nSensibility )
			{
				m_SprungBewertung[n] = 3;	
			}
			//Rating step out
			else if(m_DeltaDepth [n]<(m_nSensibilityneg))
			{
				m_SprungBewertung[n] = 2;	
			}
			//Rating step in
			else if(m_DeltaDepth [n]>m_nSensibility)
			{
				m_SprungBewertung[n] = 1;	
			}
		}
	}

	//Set points
	tInt nfirstL =0;//1=Yes, 0=No 
	tInt	nlast =2;//1=Right, 0=Left
	tInt nersatznr = 0;
	tInt nlaufnr =1;
	for(tInt i=2; i<=(m_outputMode.nXRes-2);i++)
	{

		//Left point
		if((m_SprungBewertung[i] == 1) && (m_SprungBewertung[i-1] == 3) )
		{					
			//Replacement for two following left points
			if(nfirstL == 0)
			{
				//Create left point
				nfirstL = 1;
				m_g_ObjStruct.nObjNr=nlaufnr;
				nlaufnr++;
				m_g_ObjStruct.nObjXLPos = m_PixelX [2];
				m_g_ObjStruct.nObjXLPixel = 2;
				m_g_ObjStruct.nObjXLDepth = m_Depthraw[2];
			}
			//Last point right -> Next point left
			if(nlast == 0)
			{
				//Create left point
				m_g_ObjStruct.nObjNr=nlaufnr;
				nlaufnr++;
				m_g_ObjStruct.nObjXLPos = m_PixelX [nersatznr+1];
				m_g_ObjStruct.nObjXLPixel = (nersatznr+1);
				m_g_ObjStruct.nObjXLDepth = m_Depthraw[nersatznr+1];
			}
			//Last point left -> Next point right
			m_g_ObjStruct.nObjXRPos = m_PixelX [i];
			m_g_ObjStruct.nObjXRPixel = i;
			m_g_ObjStruct.nObjXRDepth = m_Depthraw[i];

			ProcessOutput();			
			nlast =0;
			nersatznr = i;
		}

		//Right point
		if((m_SprungBewertung[i+1] == 3) && (m_SprungBewertung[i] == 2) ) 
		{	
			//Last point right -> Next point left
			if(nlast == 1)
			{
				//Create right point
				m_g_ObjStruct.nObjXRPos = m_PixelX [nersatznr+1];
				m_g_ObjStruct.nObjXRPixel =(nersatznr+1);
				m_g_ObjStruct.nObjXRDepth = m_Depthraw[nersatznr+1];
				
				ProcessOutput();
			}
			//Create left point
			m_g_ObjStruct.nObjNr = nlaufnr;
			nlaufnr++;						
			m_g_ObjStruct.nObjXLPos = m_PixelX [i];
			m_g_ObjStruct.nObjXLPixel =i;
			m_g_ObjStruct.nObjXLDepth = m_Depthraw[i];
			
			nfirstL = 1;
			nlast =1;
			nersatznr = i;
		}
	}

	//Replacement for last point in row
	if(nlast == 1)
	{
		//Create right point
		m_g_ObjStruct.nObjXRPos = m_PixelX [m_outputMode.nXRes-2];
		m_g_ObjStruct.nObjXRPixel = (m_outputMode.nXRes-2);
		m_g_ObjStruct.nObjXRDepth = m_Depthraw[m_outputMode.nXRes-2];
		ProcessOutput();
	}

	RETURN_NOERROR;
}

tResult cObjectDetection::ProcessInput(IMediaSample* pSample)
{
	/*if (pSample != NULL && m_pCoderDesc_Input != NULL)
	{ 

		cObjectPtr<IMediaCoderExt> pCoder_Input;
		RETURN_IF_FAILED(m_pCoderDesc_Input->Lock(pSample, (IMediaCoderExt**)&pCoder_Input));

		//Get ID
		if( tFalse == bAlreadyRead_Input)
		{
			RETURN_IF_FAILED(pCoder_Input->GetID("i16XtionDataDepth", nID_i16Depth));
			RETURN_IF_FAILED(pCoder_Input->GetID("i16XtionDataXPos", nID_i16XPos));
			RETURN_IF_FAILED(pCoder_Input->GetID("i16XtionDataYPos", nID_i16YPos));
			RETURN_IF_FAILED(pCoder_Input->GetID("i16XtionDataPixel", nID_i16Pixel));

			bAlreadyRead_Input = tTrue;
		}

		//Save data
		RETURN_IF_FAILED(pCoder_Input->Get(nID_i16Depth, (tVoid*)&i16BuffDepth));
		RETURN_IF_FAILED(pCoder_Input->Get(nID_i16XPos, (tVoid*)&i16BuffX));
		RETURN_IF_FAILED(pCoder_Input->Get(nID_i16YPos, (tVoid*)&i16BuffY));
		RETURN_IF_FAILED(pCoder_Input->Get(nID_i16Pixel, (tVoid*)&i16BuffPixel));

        m_pCoderDesc_Input->Unlock(pCoder_Input);

		//Order data in array
		tUInt16 pos;
		pos = i16BuffPixel;
		Depthraw [pos]= i16BuffDepth;
		PixelX [pos]=i16BuffX;
	
		//Start ProcessFound
		if (pos == outputMode.nXRes)
		{
			ProcessFound();
		}		
	}*/
	RETURN_NOERROR;			
}

tResult cObjectDetection::ProcessFound()
{
	/*//Average determination over 3 points
	Depth[1] = Depthraw[1];
	Depth[outputMode.nXRes] = Depthraw[outputMode.nXRes];
	for(tInt m=2;m<=(outputMode.nXRes-1);m++)	
	{
		Depth[m]=(Depthraw[m-1]+Depthraw[m]+Depthraw[m+1])/3;
	}

	//Detection-Algo
	for(tInt n=1; n<=(outputMode.nXRes-1);n++)
	{	
		//Check if Depth Information OK? (Information != 0)
		if( (Depthraw[n]==0) || (Depthraw[n+1]==0) )
		{
			SprungBewertung[n] = 0;
			DeltaDepth [n]= 0;
		}
		else
		{	
			//Calculate the difference between 2 points
			DeltaDepth [n] = Depth[n]-Depth[n+1];
			//Rating Flat
			if(abs(DeltaDepth [n]) <= m_nSensibility )
			{
				SprungBewertung[n] = 3;	
			}
			//Rating step out
			else if(DeltaDepth [n]<(m_nSensibilityneg))
			{
				SprungBewertung[n] = 2;	
			}
			//Rating step in
			else if(DeltaDepth [n]>m_nSensibility)
			{
				SprungBewertung[n] = 1;	
			}
		}
	}

	//Set points
	tInt nfirstL =0;//1=Yes, 0=No 
	tInt	nlast =2;//1=Right, 0=Left
	tInt nersatznr = 0;
	tInt nlaufnr =1;
	for(tInt i=2; i<=(outputMode.nXRes-2);i++)
	{

		//Left point
		if((SprungBewertung[i] == 1) && (SprungBewertung[i-1] == 3) )
		{					
			//Replacement for two following left points
			if(nfirstL == 0)
			{
				//Create left point
				nfirstL = 1;
				g_ObjStruct.nObjNr=nlaufnr;
				nlaufnr++;
				g_ObjStruct.nObjXLPos = PixelX [2];
				g_ObjStruct.nObjXLPixel = 2;
				g_ObjStruct.nObjXLDepth = Depthraw[2];
			}
			//Last point right -> Next point left
			if(nlast == 0)
			{
				//Create left point
				g_ObjStruct.nObjNr=nlaufnr;
				nlaufnr++;
				g_ObjStruct.nObjXLPos = PixelX [nersatznr+1];
				g_ObjStruct.nObjXLPixel = (nersatznr+1);
				g_ObjStruct.nObjXLDepth = Depthraw[nersatznr+1];
			}
			//Last point left -> Next point right
			g_ObjStruct.nObjXRPos = PixelX [i];
			g_ObjStruct.nObjXRPixel = i;
			g_ObjStruct.nObjXRDepth = Depthraw[i];

			ProcessOutput();			
			nlast =0;
			nersatznr = i;
		}

		//Right point
		if((SprungBewertung[i+1] == 3) && (SprungBewertung[i] == 2) ) 
		{	
			//Last point right -> Next point left
			if(nlast == 1)
			{
				//Create right point
				g_ObjStruct.nObjXRPos = PixelX [nersatznr+1];
				g_ObjStruct.nObjXRPixel =(nersatznr+1);
				g_ObjStruct.nObjXRDepth = Depthraw[nersatznr+1];
				
				ProcessOutput();
			}
			//Create left point
			g_ObjStruct.nObjNr = nlaufnr;
			nlaufnr++;						
			g_ObjStruct.nObjXLPos = PixelX [i];
			g_ObjStruct.nObjXLPixel =i;
			g_ObjStruct.nObjXLDepth = Depthraw[i];
			
			nfirstL = 1;
			nlast =1;
			nersatznr = i;
		}
	}

	//Replacement for last point in row
	if(nlast == 1)
	{
		//Create right point
		g_ObjStruct.nObjXRPos = PixelX [outputMode.nXRes-2];
		g_ObjStruct.nObjXRPixel = (outputMode.nXRes-2);
		g_ObjStruct.nObjXRDepth = Depthraw[outputMode.nXRes-2];
		ProcessOutput();
	}
	*/
	RETURN_NOERROR;
}

tResult cObjectDetection::ProcessOutput()
{
	//Create the middle point of the L-Shape
	tUInt16 nMinimum = 65535;
	tUInt16 nMinPos = 0;
	for(tInt16 i = m_g_ObjStruct.nObjXLPixel; i<=m_g_ObjStruct.nObjXRPixel; i++)
	{	
		if(m_Depthraw[i] == 0)
		{
			m_g_ObjStruct.nGueltig = 0;
		}

		if(m_Depthraw[i] < nMinimum)
		{
			nMinimum = m_Depthraw[i];
			nMinPos = i;
		}
		//Save the point
		m_g_ObjStruct.nObjXMPos = m_PixelX[nMinPos];
		m_g_ObjStruct.nObjXMDepth= nMinimum;
	}

	//Check if wight is OK?
	if(((m_g_ObjStruct.nObjXRPos) - (m_g_ObjStruct.nObjXLPos)) < m_nminBreiteObjekt)
	{
		m_g_ObjStruct.nGueltig = 0;
	}
	else
	{
		m_g_ObjStruct.nGueltig = 1;
		CreateAndTransmitGCL();
	}

	//LOG_INFO(cString::Format("ObjNr: %i, PixelRPos:%i, PixelRDepth:%i, ",g_ObjStruct.nObjNr,g_ObjStruct.nObjXRPos,g_ObjStruct.nObjXRDepth));
	//LOG_INFO(cString::Format("PixelMPos:%i, PixelMDepth:%i, ",g_ObjStruct.nObjXMPos,g_ObjStruct.nObjXMDepth));
	//LOG_INFO(cString::Format("PixelLPos:%i, PixelLDepth:%i, ",g_ObjStruct.nObjXLPos,g_ObjStruct.nObjXLDepth));
	//LOG_INFO(cString::Format("G\FCltig: %i",g_ObjStruct.nGueltig));

	cObjectPtr<IMediaSample> pMediaSample_Output;
	RETURN_IF_FAILED(AllocMediaSample((tVoid**)&pMediaSample_Output));
	cObjectPtr<IMediaSerializer> pSerializer_Output;
	m_pCoderDesc_Output->GetMediaSampleSerializer(&pSerializer_Output);
	tInt nSize = pSerializer_Output->GetDeserializedSize();
	RETURN_IF_FAILED(pMediaSample_Output->AllocBuffer(nSize));	
	cObjectPtr<IMediaCoderExt> pCoder_Output;

	RETURN_IF_FAILED(m_pCoderDesc_Output->WriteLock(pMediaSample_Output, &pCoder_Output));

	// Get ID
	if(tFalse == m_bAlreadyRead_Input)
	{
		RETURN_IF_FAILED(pCoder_Output->GetID("i16ObjNr", nID_i16ObjNr));
		RETURN_IF_FAILED(pCoder_Output->GetID("i16ObjXLPos", nID_i16ObjXLPos));
		RETURN_IF_FAILED(pCoder_Output->GetID("i16ObjXLDept", nID_i16ObjXLDept));
		RETURN_IF_FAILED(pCoder_Output->GetID("i16ObjXMPos", nID_i16ObjXMPos));
		RETURN_IF_FAILED(pCoder_Output->GetID("i16ObjXMDepth", nID_i16ObjXMDepth));
		RETURN_IF_FAILED(pCoder_Output->GetID("i16ObjXRPos", nID_i16ObjXRPos));
		RETURN_IF_FAILED(pCoder_Output->GetID("i16ObjXRDepth", nID_i16ObjXRDepth));
		RETURN_IF_FAILED(pCoder_Output->GetID("i8Valid", nID_i8Gueltig));
		m_bAlreadyRead_Input = tTrue;
	}

	// Set Data
	RETURN_IF_FAILED(pCoder_Output->Set(nID_i16ObjNr,		(tVoid*)&m_g_ObjStruct.nObjNr));
	RETURN_IF_FAILED(pCoder_Output->Set(nID_i16ObjXLPos,	(tVoid*)&m_g_ObjStruct.nObjXLPos));
	RETURN_IF_FAILED(pCoder_Output->Set(nID_i16ObjXLDept,	(tVoid*)&m_g_ObjStruct.nObjXLDepth));
	RETURN_IF_FAILED(pCoder_Output->Set(nID_i16ObjXMPos,	(tVoid*)&m_g_ObjStruct.nObjXMPos));
	RETURN_IF_FAILED(pCoder_Output->Set(nID_i16ObjXMDepth,	(tVoid*)&m_g_ObjStruct.nObjXMDepth));
	RETURN_IF_FAILED(pCoder_Output->Set(nID_i16ObjXRPos,	(tVoid*)&m_g_ObjStruct.nObjXRPos));
	RETURN_IF_FAILED(pCoder_Output->Set(nID_i16ObjXRDepth,	(tVoid*)&m_g_ObjStruct.nObjXRDepth));
	RETURN_IF_FAILED(pCoder_Output->Set(nID_i8Gueltig,		(tVoid*)&m_g_ObjStruct.nGueltig));
			
	m_pCoderDesc_Output->Unlock(pCoder_Output);
	//Transmit Data
	pMediaSample_Output->SetTime(_clock->GetStreamTime());
	m_oOutputPin1.Transmit(pMediaSample_Output);

	
	RETURN_NOERROR;
}


tResult cObjectDetection::CreateAndTransmitGCL()
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
	
	//Print the Line with GCL-Offset

	cGCLWriter::StoreCommand(pc, GCL_CMD_FGCOL, cColor(255, 50, 90).GetRGBA());

	cGCLWriter::StoreCommand(pc, GCL_CMD_DRAWLINE, m_g_ObjStruct.nObjXLPixel+m_nGCLOffset, 244, m_g_ObjStruct.nObjXRPixel+m_nGCLOffset, 244);

	cGCLWriter::StoreCommand(pc, GCL_CMD_END);

    pSample->Unlock(aGCLProc);

    RETURN_IF_FAILED(m_oGLCOutput.Transmit(pSample));
#endif
	RETURN_NOERROR;
}

tResult cObjectDetection::UpdateImageFormat(const tBitmapFormat* pFormat)
{
	if (pFormat != NULL)
	{
		m_sInputFormatVideo = (*pFormat);		
		LOG_INFO(adtf_util::cString::Format("Objectdetection Filter Input: Size %d x %d ; BPL %d ; Size %d , PixelFormat; %d",m_sInputFormatVideo.nWidth,m_sInputFormatVideo.nHeight, 
			m_sInputFormatVideo.nBytesPerLine, m_sInputFormatVideo.nSize, m_sInputFormatVideo.nPixelFormat));			
	}
	
	RETURN_NOERROR;
}


tResult cObjectDetection::ConvertRealWorldToProjectiveOffline(tInt32 nCount, const tFloatVector3D* aRealWorld, tFloatVector3D* aProjective)
{
	//refer to original function xnConvertRealWorldToProjective in XnOpenNI.cpp
	tFloat64 fXToZ = 1.122133;		//values found by debugging the OPENNI Source
	tFloat64 fYToZ = 0.84160;		//values found by debugging the OPENNI Source

	tFloat64 fCoeffX = m_sInputFormatVideo.nWidth / fXToZ;
	tFloat64 fCoeffY = m_outputMode.nYRes / fYToZ;

	// we can assume resolution is even (so integer div is sufficient)
	tInt32 nHalfXres =m_sInputFormatVideo.nWidth / 2;
	tInt32 nHalfYres =m_sInputFormatVideo.nHeight / 2;

	for (tInt32 i = 0; i < nCount; ++i)
	{
		aProjective[i].X = (tFloat32)fCoeffX * aRealWorld[i].X / aRealWorld[i].Z + nHalfXres;
		aProjective[i].Y = nHalfYres - (tFloat32)fCoeffY * aRealWorld[i].Y / aRealWorld[i].Z;
		aProjective[i].Z = aRealWorld[i].Z;
	}

	RETURN_NOERROR;
}
