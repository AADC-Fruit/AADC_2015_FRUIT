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
#include "RoadSignDetectHeaders.h"

ADTF_FILTER_PLUGIN("Roadsign Detection Filter Plugin", OID_ADTF_ROADSIGNDETECTIONFILTER, cRoadsignDetectionFilter)



cRoadsignDetectionFilter::cRoadsignDetectionFilter(const tChar* __info):cFilter(__info)
{
	SetPropertyStr("Source for road sign templates","");
	SetPropertyBool("Source for road sign templates" ".IsDirectory", tTrue);
	UCOM_REGISTER_TIMING_SPOT(cString(OIGetInstanceName()) + "::Process::Start", m_oProcessStart);

	m_bDebugModeEnabled = tFalse;
	SetPropertyBool("DebugOutput",              m_bDebugModeEnabled);
    UCOM_REGISTER_TIMING_SPOT(cString(OIGetInstanceName()) + "::Process::End", m_oProcessEnd);	
}

cRoadsignDetectionFilter::~cRoadsignDetectionFilter()
{
	interfaceClass->shutdown();
	roadsignDetection::DestroyMyClass(interfaceClass);
}

tResult cRoadsignDetectionFilter::Init(tInitStage eStage, __exception)
{
    RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr));
    if (eStage == StageFirst)
	{
		//this is a VideoPin
        RETURN_IF_FAILED(m_oPinInputVideo.Create("Video_RGB_input",IPin::PD_Input, static_cast<IPinEventSink*>(this))); 
		RETURN_IF_FAILED(RegisterPin(&m_oPinInputVideo));

        //this is a VideoPin
        RETURN_IF_FAILED(m_oPinOutputVideo.Create("Video_RGB_output", IPin::PD_Output, static_cast<IPinEventSink*>(this)));
		RETURN_IF_FAILED(RegisterPin(&m_oPinOutputVideo));

		//this is a OutputPin
		RETURN_IF_FAILED(m_oPinSignNumber.Create("RoadSign_detected", new cMediaType(MEDIA_TYPE_COMMAND, MEDIA_TYPE_COMMAND), static_cast<IPinEventSink*> (this)));
		RETURN_IF_FAILED(RegisterPin(&m_oPinSignNumber));

		
		cFilename filename = GetPropertyStr("Source for road sign templates");
		ADTF_GET_CONFIG_FILENAME(filename);
		if (filename == "")
			LOG_INFO("Roadsign Detection Filter: Please give correct path for sign templates");
		else if(filename.IsRelative())
			filename = filename.CreateAbsolutePath(".");
			
		LOG_INFO(adtf_util::cString::Format("Roadsign Detection Filter: sign templates loaded from %s",filename.GetPtr()));

		interfaceClass = roadsignDetection::CreateMyClass(filename.GetPtr());
	
	}
    else if (eStage == StageNormal)
    {
       	m_bDebugModeEnabled = GetPropertyBool("DebugOutput");
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

tResult cRoadsignDetectionFilter::Shutdown(tInitStage eStage, __exception)
{
	
	if (eStage == StageFirst)
    {
 
    }
	
	return cFilter::Shutdown(eStage, __exception_ptr);
}

tResult cRoadsignDetectionFilter::Connect(IPin* pSource, const tChar* strDestname, ucom::IException** __exception_ptr)
{
	if (pSource != NULL && strDestname != NULL)
	{
		
	}
	RETURN_IF_FAILED(cFilter::Connect(pSource, strDestname, __exception_ptr));
	RETURN_NOERROR;
}

tResult cRoadsignDetectionFilter::OnPinEvent(IPin* pSource,
                                            tInt nEventCode,
                                            tInt nParam1,
                                            tInt nParam2,
                                            IMediaSample* pMediaSample)
{
	switch (nEventCode)
    {
        case IPinEventSink::PE_MediaSampleReceived:
        {
			if (pSource == &m_oPinInputVideo)
			{				
				UCOM_TIMING_SPOT(m_oProcessStart);
				ProcessVideo(pMediaSample);		
				UCOM_TIMING_SPOT(m_oProcessEnd);
			}					
			 break;
        }
        case IPinEventSink::PE_MediaTypeChanged:
        {
            if (pSource == &m_oPinInputVideo)
            {                
                {
					cObjectPtr<IMediaType> pType;
					RETURN_IF_FAILED(m_oPinInputVideo.GetMediaType(&pType));

					cObjectPtr<IMediaTypeVideo> pTypeVideo;
					RETURN_IF_FAILED(pType->GetInterface(IID_ADTF_MEDIA_TYPE_VIDEO, (tVoid**)&pTypeVideo));

					UpdateImageFormat(m_oPinInputVideo.GetFormat());
                }
            }
			break;
        }
		default:
		{
			break;
		}
    }
	
	RETURN_NOERROR;
}



tResult cRoadsignDetectionFilter::ProcessVideo(adtf::IMediaSample* pISample)
{
	
	RETURN_IF_POINTER_NULL(pISample);
//creating new temporary variables
	adtf_util::cImage oImage;	
	const tVoid* l_pSrcBuffer;

//receiving data from inputsample, and saving to oImage
	if (IS_OK(pISample->Lock(&l_pSrcBuffer)))
	{
			oImage.SetBits((tUInt8*) l_pSrcBuffer, &m_sInputFormat);			
			pISample->Unlock(l_pSrcBuffer);
	}

//sending data to dll and processing data in DLL
	tInt Result = 0;
	Result = interfaceClass->update((tUInt8*)oImage.GetBitmap()); 
	
	
	if (Result !=0)	
		{
		cString newSign = convertIntToEnum(Result);
		if (m_bDebugModeEnabled) LOG_INFO(adtf_util::cString::Format("Roadsign Detection Filter: Erkanntes Verkehrszeichen: %s",newSign.GetPtr()));
		
		if (m_RoadSignResult != Result)
		{
			m_RoadSignResult = Result;	
	
			//transmit number of detected sign
			cObjectPtr<IMediaSample> pNewSample;
			RETURN_IF_FAILED(AllocMediaSample((tVoid**)&pNewSample));
			RETURN_IF_FAILED(pNewSample->Update(cHighResTimer::GetTime(), &m_RoadSignResult, sizeof(tInt), IMediaSample::MSF_None));
			RETURN_IF_FAILED(m_oPinSignNumber.Transmit(pNewSample));
		}
		}
	
//checking if output image of dll has changed in size

	if (m_sOutputFormat.nSize != (tInt)interfaceClass->getProcImgSize())
		{	
		m_sOutputFormat.nBytesPerLine =	(tInt)interfaceClass->getProcImgBytesPerLine();
		m_sOutputFormat.nSize =(tInt)interfaceClass->getProcImgSize();
		m_sOutputFormat.nBitsPerPixel=(tInt)interfaceClass->getProcImgBitsPerPixel();
		m_sOutputFormat.nPaletteSize = 0;
		m_sOutputFormat.nPixelFormat = adtf_util::cImage::PF_8BIT ;  
		m_oPinOutputVideo.SetFormat(&m_sOutputFormat, NULL);
		}
		
//creating new media sample with processed data from dll and sending frame via pin
	cObjectPtr<IMediaSample> pNewSample;
	RETURN_IF_FAILED(AllocMediaSample((tVoid**)&pNewSample));
	RETURN_IF_FAILED(pNewSample->AllocBuffer(m_sOutputFormat.nSize));
	//setting timestamp of media sample
	tTimeStamp tmTime = (_clock != NULL ? _clock->GetStreamTime() : cHighResTimer::GetTime());
	
	RETURN_IF_FAILED(pNewSample->SetTime(tmTime));		
	
	if (IS_OK(pNewSample->CopyBufferFrom((tVoid*)interfaceClass->getProcessedData(),m_sOutputFormat.nSize)))
	{									
			RETURN_IF_FAILED(m_oPinOutputVideo.Transmit(pNewSample));
	}
	
	RETURN_NOERROR;
}

tResult cRoadsignDetectionFilter::Start(__exception)
{
	RETURN_IF_FAILED(cFilter::Start(__exception_ptr));
	RETURN_NOERROR;
}

tResult cRoadsignDetectionFilter::Stop(__exception)
{
	RETURN_IF_FAILED(cFilter::Stop(__exception_ptr));
	RETURN_NOERROR;
}

tResult cRoadsignDetectionFilter::UpdateImageFormat(const tBitmapFormat* pFormat)
{
	if (pFormat != NULL)
	{
		//memcpy(&m_sInputFormat, pFormat, sizeof(tBitmapFormat));
		//memcpy(&m_sOutputFormat, pFormat, sizeof(tBitmapFormat));
		m_sInputFormat = (*pFormat);
		m_sOutputFormat = (*pFormat);
		
		LOG_INFO(adtf_util::cString::Format("Roadsign Detection Filter: Input: Size %d x %d ; BPL %d ; Size %d , PixelFormat; %d",m_sInputFormat.nWidth,m_sInputFormat.nHeight,m_sInputFormat.nBytesPerLine, m_sInputFormat.nSize, m_sInputFormat.nPixelFormat));		
		m_oPinOutputVideo.SetFormat(&m_sOutputFormat, NULL);
		
		LOG_INFO(adtf_util::cString::Format("Roadsign Detection Filter: Output: Size %d x %d ; BPL %d ; Size %d ",m_sOutputFormat.nWidth,m_sOutputFormat.nHeight,m_sOutputFormat.nBytesPerLine, m_sOutputFormat.nSize));
		interfaceClass->init((tInt)m_sOutputFormat.nWidth,(tInt)m_sOutputFormat.nHeight,(tInt)m_sOutputFormat.nBytesPerLine);
		
		interfaceClass->setWidth((tInt)m_sOutputFormat.nWidth);
		interfaceClass->setHeight((tInt)m_sOutputFormat.nHeight);
		interfaceClass->setBytesPerLine((tInt)m_sOutputFormat.nBytesPerLine);
	}
	
	RETURN_NOERROR;
}
