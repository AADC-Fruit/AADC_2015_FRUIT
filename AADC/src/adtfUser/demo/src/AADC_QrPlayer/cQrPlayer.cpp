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
#include "cQrPlayer.h"

ADTF_FILTER_PLUGIN("AADC QR Player Filter", OID_ADTF_QRPLAYERFILTER, cQrPlayer)



cQrPlayer::cQrPlayer(const tChar* __info):cFilter(__info)
{
	m_bConsoleOutputEnabled = false;
	SetPropertyBool("Output to Console",              m_bConsoleOutputEnabled);
    // configure the reader
    m_qrScanner.set_config(ZBAR_QRCODE, ZBAR_CFG_ENABLE, 1);    
}

cQrPlayer::~cQrPlayer()
{
}

tResult cQrPlayer::Init(tInitStage eStage, __exception)
{
    RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr));
    if (eStage == StageFirst)
	{
		//this is a VideoPin
        RETURN_IF_FAILED(m_oPinInputVideo.Create("Video_RGB_input",IPin::PD_Input, static_cast<IPinEventSink*>(this))); 
		RETURN_IF_FAILED(RegisterPin(&m_oPinInputVideo));
		
		
		cObjectPtr<IMediaDescriptionManager> pDescManager;
        RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER,IID_ADTF_MEDIA_DESCRIPTION_MANAGER,(tVoid**)&pDescManager,__exception_ptr));

        tChar const * strDescSignalValueOutput = pDescManager->GetMediaDescription("tSignalValue");
        RETURN_IF_POINTER_NULL(strDescSignalValueOutput);        
        cObjectPtr<IMediaType> pTypeSignalValueOutput = new cMediaType(0, 0, 0, "tSignalValue", strDescSignalValueOutput,IMediaDescription::MDF_DDL_DEFAULT_VERSION);	
        RETURN_IF_FAILED(pTypeSignalValueOutput->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&m_pCoderDescSignalOutput)); 
		
        //accelerate output
        RETURN_IF_FAILED(m_oPinAccelerateSignal.Create("accelerate", pTypeSignalValueOutput, static_cast<IPinEventSink*> (this)));
        RETURN_IF_FAILED(RegisterPin(&m_oPinAccelerateSignal));

        // Steer Angle Output
        RETURN_IF_FAILED(m_oPinSteeringSignal.Create("steerAngle", pTypeSignalValueOutput, static_cast<IPinEventSink*> (this)));
        RETURN_IF_FAILED(RegisterPin(&m_oPinSteeringSignal));
     
	}
    else if (eStage == StageNormal)
    {
       	m_bConsoleOutputEnabled = GetPropertyBool("Output to Console");
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

tResult cQrPlayer::Shutdown(tInitStage eStage, __exception)
{		
	return cFilter::Shutdown(eStage, __exception_ptr);
}

tResult cQrPlayer::OnPinEvent(IPin* pSource,
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
				ProcessVideo(pMediaSample);		
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



tResult cQrPlayer::ProcessVideo(adtf::IMediaSample* pISample)
{
	RETURN_IF_POINTER_NULL(pISample);
//  creating new temporary variables
	adtf_util::cImage oImage;	
	const tVoid* l_pSrcBuffer;

//  receiving data from inputsample, and saving to oImage
	if (IS_OK(pISample->Lock(&l_pSrcBuffer)))
	{
			oImage.SetBits((tUInt8*) l_pSrcBuffer, &m_sInputFormat);
			
			pISample->Unlock(l_pSrcBuffer);
	}

//  convert adtf-image to greyscaled image which is used by zbar
	RETURN_IF_FAILED(oImage.Convert(&m_sProcessFormat));	

//  creates image which is used by the QR Scanner   
	Image image(m_sInputFormat.nWidth, m_sInputFormat.nHeight, "Y800", (unsigned char*)oImage.GetBitmap(), m_sInputFormat.nWidth * m_sInputFormat.nHeight);

//  scan image for codes and check for errors		
	int n =  m_qrScanner.scan(image);
	if (n== -1) LOG_ERROR(cString::Format("QR Player: Error in QRScanner"));	
	
//  extract results to console
    for(Image::SymbolIterator symbol = image.symbol_begin();
        symbol != image.symbol_end();
        ++symbol) {
        //print only new results
        if (m_lastQRCode!=symbol->get_data())
            {
            m_lastQRCode=symbol->get_data();            
            }
        ProcessCommand(cString(symbol->get_data().c_str()));
    }
    
    // clean up
    image.set_data(NULL, 0);		

	RETURN_NOERROR;
}

tResult cQrPlayer::Start(__exception)
{
	return cFilter::Start(__exception_ptr);
}

tResult cQrPlayer::Stop(__exception)
{
	return cFilter::Stop(__exception_ptr);
}

tResult cQrPlayer::UpdateImageFormat(const tBitmapFormat* pFormat)
{
	if (pFormat != NULL)
	{
        //detects input format
        m_sInputFormat = (*pFormat);
        //set image format for the images to be processed; defined by zbar
        m_sProcessFormat.nWidth = m_sInputFormat.nWidth;
		m_sProcessFormat.nHeight = m_sInputFormat.nHeight;
		m_sProcessFormat.nBitsPerPixel = 8;
		m_sProcessFormat.nPixelFormat = cImage::PF_GREYSCALE_8;
		m_sProcessFormat.nBytesPerLine = m_sProcessFormat.nWidth * 1;
		m_sProcessFormat.nSize = m_sProcessFormat.nBytesPerLine * m_sProcessFormat.nHeight;
		m_sProcessFormat.nPaletteSize = 0;	
		
		LOG_INFO(adtf_util::cString::Format("QR Player: Input Image Format: Size %d x %d ; BPL %d ; Size %d , PixelFormat; %d",m_sInputFormat.nWidth,m_sInputFormat.nHeight,m_sInputFormat.nBytesPerLine, m_sInputFormat.nSize, m_sInputFormat.nPixelFormat));		
		
		LOG_INFO(adtf_util::cString::Format("QR Player: Process Image Format: Size %d x %d ; BPL %d ; Size %d , PixelFormat; %d",m_sProcessFormat.nWidth,m_sProcessFormat.nHeight,m_sProcessFormat.nBytesPerLine, m_sProcessFormat.nSize, m_sProcessFormat.nPixelFormat));	
	}
	
	RETURN_NOERROR;
}

tResult cQrPlayer::ProcessCommand(const cString inputString)
{
    if (inputString.Compare("Rechts") == 0)
    {
        OutputSteering(30);
    }
    else if (inputString.Compare("Links") == 0)
    {
        OutputSteering(-30);
    }
    else if (inputString.Compare("Geradeaus") == 0)
    {
        OutputSteering(0);
    }
    else if (inputString.Compare("Stop") == 0)
    {
        OutputSpeed(0);
    }
    else if (inputString.Compare("Fahren") == 0)
    {
        OutputSpeed(25);
    }
     else if (inputString.Compare("Rueckwaerts") == 0)
    {
        OutputSpeed(-25);
    }
    RETURN_NOERROR; 
}

void cQrPlayer::OutputSteering(tFloat32 val)
{	
	tUInt32 timeStamp = 0;
							
    //create new media sample
    cObjectPtr<IMediaSample> pMediaSample;
    AllocMediaSample((tVoid**)&pMediaSample);

    //allocate memory with the size given by the descriptor
    cObjectPtr<IMediaSerializer> pSerializer;
    m_pCoderDescSignalOutput->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();
    pMediaSample->AllocBuffer(nSize);
       
    //write date to the media sample with the coder of the descriptor
    cObjectPtr<IMediaCoder> pCoder;
    m_pCoderDescSignalOutput->WriteLock(pMediaSample, &pCoder);	
		
	pCoder->Set("f32Value", (tVoid*)&val);	
    pCoder->Set("ui32ArduinoTimestamp", (tVoid*)&timeStamp);    
    m_pCoderDescSignalOutput->Unlock(pCoder);
    
    //transmit media sample over output pin
    pMediaSample->SetTime(_clock->GetStreamTime());
    m_oPinSteeringSignal.Transmit(pMediaSample);
	if (m_bConsoleOutputEnabled) LOG_INFO(adtf_util::cString::Format("QrPlayer: OutputSteering %f", val));
}

void cQrPlayer::OutputSpeed(tFloat32 val)
{
	tUInt32 timeStamp = 0;
							
    //create new media sample
    cObjectPtr<IMediaSample> pMediaSample;
    AllocMediaSample((tVoid**)&pMediaSample);

    //allocate memory with the size given by the descriptor
    cObjectPtr<IMediaSerializer> pSerializer;
    m_pCoderDescSignalOutput->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();
    pMediaSample->AllocBuffer(nSize);
       
    //write date to the media sample with the coder of the descriptor
    cObjectPtr<IMediaCoder> pCoder;
    m_pCoderDescSignalOutput->WriteLock(pMediaSample, &pCoder);	
		
	pCoder->Set("f32Value", (tVoid*)&val);	
    pCoder->Set("ui32ArduinoTimestamp", (tVoid*)&timeStamp);    
    m_pCoderDescSignalOutput->Unlock(pCoder);
    
    //transmit media sample over output pin
    pMediaSample->SetTime(_clock->GetStreamTime());
    m_oPinAccelerateSignal.Transmit(pMediaSample);
    if (m_bConsoleOutputEnabled) LOG_INFO(adtf_util::cString::Format("QrPlayer: OutputSpeed %f", val));
}
