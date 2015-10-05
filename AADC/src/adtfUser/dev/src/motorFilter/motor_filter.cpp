#include "stdafx.h"
#include "motor_filter.h"
#include <template_data.h>

// -------------------------------------------------------------------------------------------------
ADTF_FILTER_PLUGIN("Motor Filter", OID_ADTF_MOTOR_FILTER, MotorFilter);
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
MotorFilter::MotorFilter(const tChar* __info):cFilter(__info) {
// -------------------------------------------------------------------------------------------------
}

// -------------------------------------------------------------------------------------------------
MotorFilter::~MotorFilter() {
// -------------------------------------------------------------------------------------------------

}

// -------------------------------------------------------------------------------------------------
tResult MotorFilter::Init(tInitStage eStage, __exception) {
// -------------------------------------------------------------------------------------------------
  // never miss calling the parent implementation!!
  RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr))
  

  if (eStage == StageFirst) {
    // Retrieve the description manager
    cObjectPtr<IMediaDescriptionManager> description_manager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER,
      IID_ADTF_MEDIA_DESCRIPTION_MANAGER,(tVoid**)&description_manager,__exception_ptr));
    
    // Create the signal value description object
    tChar const * stream_type = description_manager->GetMediaDescription("tSignalValue");
    RETURN_IF_POINTER_NULL(stream_type);
    
    // Get the type value
    cObjectPtr<IMediaType> pTypeSignalValue = new cMediaType(0, 0, 0, "tSignalValue",
      stream_type,IMediaDescription::MDF_DDL_DEFAULT_VERSION);	
    RETURN_IF_FAILED(pTypeSignalValue->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION,
      (tVoid**) &stream_description_));
  
    // Create and register the input pin
    RETURN_IF_FAILED(speed_input_.Create("speed_in", new cMediaType(0, 0, 0, "tSignalValue"), static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&speed_input_));
    
    // Create and register the output pin
    RETURN_IF_FAILED(speed_output_.Create("speed_out", new cMediaType(0, 0, 0, "tSignalValue"), NULL));
    RETURN_IF_FAILED(RegisterPin(&speed_output_));
  }
  
  else if (eStage == StageNormal) {
    // In this stage you would do further initialisation and/or create your dynamic pins.
    // Please take a look at the demo_dynamicpin example for further reference.
  }
  
  else if (eStage == StageGraphReady) {
    // All pin connections have been established in this stage so you can query your pins
    // about their media types and additional meta data.
    // Please take a look at the demo_imageproc example for further reference.
  }
  
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult MotorFilter::Shutdown(tInitStage eStage, __exception) {
// -------------------------------------------------------------------------------------------------
  // Call the base class implementation
  return cFilter::Shutdown(eStage, __exception_ptr);
}

// -------------------------------------------------------------------------------------------------
tResult MotorFilter::OnPinEvent(
  IPin* pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample* pMediaSample) {
// -------------------------------------------------------------------------------------------------
  // Check for the type of the event (data received or transmitted, etc.)
  if (nEventCode == IPinEventSink::PE_MediaSampleReceived) {
    // This pointer is not null if a media sample was received
    RETURN_IF_POINTER_NULL(pMediaSample);

    if (pMediaSample != NULL && stream_description_ != NULL) {           
      //get values from media sample
      tFloat32 signal_value = 0;
      
      {
        __adtf_sample_read_lock_mediadescription(stream_description_, pMediaSample, pCoder);
        pCoder->Get("f32Value", (tVoid*)&signal_value);            
      }
      
      // Process the read sample
      processSpeed(signal_value);
    }

    else {
      RETURN_ERROR(ERR_FAILED);
    }
  }

  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
void MotorFilter::processSpeed(tFloat32 value) {
// -------------------------------------------------------------------------------------------------
  tFloat32 new_value = value;

  // Transmit the new speed value
  transmit(new_value);
}

// -------------------------------------------------------------------------------------------------
void MotorFilter::transmit(tFloat32 value) {
// -------------------------------------------------------------------------------------------------
  //create new media sample
  cObjectPtr<IMediaSample> sample;
  AllocMediaSample((tVoid**) &sample);

  //allocate memory with the size given by the descriptor
  cObjectPtr<IMediaSerializer> serializer;
  stream_description_->GetMediaSampleSerializer(&serializer);
  tInt size = serializer->GetDeserializedSize();
  sample->AllocBuffer(size);
  
  tUInt32 timeStamp = 0;
  
  {
    __adtf_sample_write_lock_mediadescription(stream_description_, sample, coder);
    coder->Set("f32Value", (tVoid*) &value);
    coder->Set("ui32ArduinoTimestamp", (tVoid*) &timeStamp);
  }
  
  //transmit media sample over output pin
  sample->SetTime(_clock->GetStreamTime());
  speed_output_.Transmit(sample);
}
