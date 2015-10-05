/**
 *
 * ADTF Template Project
 *
 * @file
 * Copyright &copy; Audi Electronics Venture GmbH. All rights reserved
 *
 * $Author: belkera $
 * $Date: 2011-06-30 16:51:21 +0200 (Thu, 30 Jun 2011) $
 * $Revision: 26514 $
 *
 * @remarks
 *
 */
#include "stdafx.h"
#include "printFilter.h"
#include <template_data.h>

#define THRESHOLD 30

/// Create filter shell
ADTF_FILTER_PLUGIN("Print Filter", OID_ADTF_PRINT_FILTER, PrintFilter);


PrintFilter::PrintFilter(const tChar* __info):cFilter(__info), file_("/home/odroid/Desktop/sensor.txt") {
    // Initialize the counter/accumulator
    datapoint_counter_ = 0;
    acc_value_ = 0.0f;
    last_value_ = 0.0f;

    // Initialize the property values
    SetPropertyInt("acc_values", 20);
}

PrintFilter::~PrintFilter() {

}

tResult PrintFilter::Init(tInitStage eStage, __exception) {
    // never miss calling the parent implementation!!
    RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr))
    
    // Copy-pasted code from Carvisualization.cpp that processes sensor data. --------------------------------------------------------------------------
    cObjectPtr<IMediaDescriptionManager> pDescManager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER,IID_ADTF_MEDIA_DESCRIPTION_MANAGER,(tVoid**)&pDescManager,__exception_ptr));
        
    //input descriptor
    tChar const * strDescSignalValue = pDescManager->GetMediaDescription("tSignalValue");
    RETURN_IF_POINTER_NULL(strDescSignalValue);        
    cObjectPtr<IMediaType> pTypeSignalValue = new cMediaType(0, 0, 0, "tSignalValue", strDescSignalValue,IMediaDescription::MDF_DDL_DEFAULT_VERSION);	
    RETURN_IF_FAILED(pTypeSignalValue->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&m_pCoderDescSignalInput));

    // End of copypasta ---------------------------------------------------------------------------------------------------------------------------------

    // in StageFirst you can create and register your static pins.
    if (eStage == StageFirst)
    {        
        // ----- Input pins

        // get a media type for the input pin
        cObjectPtr<IMediaType> pInputType;
        RETURN_IF_FAILED(AllocMediaType(&pInputType, MEDIA_TYPE_TEMPLATE, MEDIA_SUBTYPE_TEMPLATE, __exception_ptr));
        
        // create and register the input pin
        // front-left US-sensor
        RETURN_IF_FAILED(data_.Create("data", new cMediaType(0, 0, 0, "tSignalValue"), static_cast<IPinEventSink*>(this)));
        RETURN_IF_FAILED(RegisterPin(&data_));
    }
    else if (eStage == StageNormal)
    {
        // In this stage you would do further initialisation and/or create your dynamic pins.
        // Please take a look at the demo_dynamicpin example for further reference.
    }
    else if (eStage == StageGraphReady)
    {
        // All pin connections have been established in this stage so you can query your pins
        // about their media types and additional meta data.
        // Please take a look at the demo_imageproc example for further reference.
    }
    
    RETURN_NOERROR;
}

tResult PrintFilter::Shutdown(tInitStage eStage, __exception) {
    if (eStage == StageGraphReady) {
    }

    // call the base class implementation
    return cFilter::Shutdown(eStage, __exception_ptr);
}

tResult PrintFilter::OnPinEvent(IPin* pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample* pMediaSample) {
    // Check for the type of the event (data received or transmitted, etc.)
    if (nEventCode == IPinEventSink::PE_MediaSampleReceived) {
        tFloat32 signalValue = 0;
        tUInt32 timeStamp = 0;

        // so we received a media sample, so this pointer better be valid.
        RETURN_IF_POINTER_NULL(pMediaSample);
        if (pMediaSample != NULL && m_pCoderDescSignalInput != NULL) {
                // read-out the incoming Media Sample
                cObjectPtr<IMediaCoder> pCoderInput;
                RETURN_IF_FAILED(m_pCoderDescSignalInput->Lock(pMediaSample, &pCoderInput));
                
                //get values from media sample        
                pCoderInput->Get("f32Value", (tVoid*)&signalValue);
                pCoderInput->Get("ui32ArduinoTimestamp", (tVoid*)&timeStamp);
                m_pCoderDescSignalInput->Unlock(pCoderInput);       
        } else {
            RETURN_ERROR(ERR_FAILED);
        }

        if (pSource == &data_) {
            datapoint_counter_++;
            acc_value_ += signalValue;
            int amount = GetPropertyInt("acc_values");

            if (datapoint_counter_ == amount) {
                last_value_ = acc_value_ / (float)amount;
                if (file_.is_open()) {
                  file_ << timeStamp << ":\t" << last_value_ << std::endl;
                  LOG_INFO(cString::Format("%d: %f", timeStamp, last_value_));
                }
                datapoint_counter_ = 0;
                acc_value_ = 0;
            }
        }
    }

    RETURN_NOERROR;
}
