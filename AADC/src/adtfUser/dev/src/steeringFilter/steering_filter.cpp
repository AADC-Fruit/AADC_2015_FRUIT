#include "stdafx.h"
#include "steering_filter.h"
#include <template_data.h>

// -------------------------------------------------------------------------------------------------
ADTF_FILTER_PLUGIN("Steering Filter", OID_ADTF_STEERING_FILTER, SteeringFilter);
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
SteeringFilter::SteeringFilter(const tChar* __info):cFilter(__info) {
// -------------------------------------------------------------------------------------------------
}

// -------------------------------------------------------------------------------------------------
SteeringFilter::~SteeringFilter() {
// -------------------------------------------------------------------------------------------------

}

// -------------------------------------------------------------------------------------------------
tResult SteeringFilter::Init(tInitStage eStage, __exception) {
// -------------------------------------------------------------------------------------------------
  // never miss calling the parent implementation!!
  RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr))
  

  if (eStage == StageFirst) {
    cObjectPtr<IMediaDescriptionManager> description_manager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER,
      IID_ADTF_MEDIA_DESCRIPTION_MANAGER, (tVoid**) &description_manager, __exception_ptr));
      
    tChar const * stream_type = description_manager->GetMediaDescription("tSignalValue");
    RETURN_IF_POINTER_NULL(stream_type);
    
    // Create the stream description used to receive/send data
    cObjectPtr<IMediaType> type_signal_value = new cMediaType(0, 0, 0, "tSignalValue",
      stream_type, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(type_signal_value->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION,
      (tVoid**) &stream_description_));
    
    // Create and register the input pin
    RETURN_IF_FAILED(steering_input_.Create("inp_angle", type_signal_value, static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&steering_input_));
    
    // Create and register the output pin
    RETURN_IF_FAILED(steering_output_.Create("res_angle", type_signal_value, NULL));
    RETURN_IF_FAILED(RegisterPin(&steering_output_));
  }
  
  else if (eStage == StageNormal) {
    // In this stage you would do further initialisation and/or create your dynamic pins.
    // Please take a look at the demo_dynamicpin example for further reference.
  }
  
  else if (eStage == StageGraphReady) {
  
    old_angle_ = 0.0;
    // All pin connections have been established in this stage so you can query your pins
    // about their media types and additional meta data.
    // Please take a look at the demo_imageproc example for further reference.
  }
  
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult SteeringFilter::Shutdown(tInitStage eStage, __exception) {
// -------------------------------------------------------------------------------------------------
  // Call the base class implementation
  return cFilter::Shutdown(eStage, __exception_ptr);
}

// -------------------------------------------------------------------------------------------------
tResult SteeringFilter::OnPinEvent(
  IPin* pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample* pMediaSample) {
// -------------------------------------------------------------------------------------------------
  // Check for the type of the event (data received or transmitted, etc.)
  if (nEventCode == IPinEventSink::PE_MediaSampleReceived) {
    // This pointer is not null if a media sample was received
    RETURN_IF_POINTER_NULL(pMediaSample);

    if (pMediaSample != NULL && stream_description_ != NULL) {
      // Get values from media sample
      tFloat32 signal_value = 0;
      
      {
        __adtf_sample_read_lock_mediadescription(stream_description_, pMediaSample, coder);
        coder->Get("f32Value", (tVoid*)&signal_value);              
      }
      
      // Send the steering value to processing
      processSteeringCommand(signal_value);
    }

    else {
      RETURN_ERROR(ERR_FAILED);
    }
  }

  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
void SteeringFilter::processSteeringCommand(tFloat32 value) {
// -------------------------------------------------------------------------------------------------
  // Todo: Implement processing stuff
  tFloat32 new_value = (value + old_angle_)/2.0;
  old_angle_ = new_value;
  // Transmit the new command
  transmitSteeringAngle(new_value);
}

// -------------------------------------------------------------------------------------------------
void SteeringFilter::transmitSteeringAngle(tFloat32 value) {
// -------------------------------------------------------------------------------------------------
  // Create the media sample
  cObjectPtr<IMediaSample> sample;
  AllocMediaSample((tVoid**) &sample);
  
  // Allocate the memory needed by the media sample by using info of the serializer
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
  
	steering_output_.Transmit(sample);
}
