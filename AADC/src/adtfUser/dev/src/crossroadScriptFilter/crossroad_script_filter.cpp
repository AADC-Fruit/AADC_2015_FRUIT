#include "stdafx.h"
#include "crossroad_script_filter.h"
#include <iostream>

// -------------------------------------------------------------------------------------------------
ADTF_FILTER_PLUGIN("FRUIT Crossroad Script Filter", OID_ADTF_CROSSROAD_SCRIPT_FILTER, CrossroadScriptFilter);
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
CrossroadScriptFilter::CrossroadScriptFilter(const tChar* __info):adtf::cTimeTriggeredFilter(__info) {
// -------------------------------------------------------------------------------------------------
  SetPropertyInt("speed", 25);
  SetPropertyFloat("duration_forward", 1.25);
  SetPropertyInt("angle", 30);
  SetPropertyFloat("duration_turning", 5);
}

// -------------------------------------------------------------------------------------------------
CrossroadScriptFilter::~CrossroadScriptFilter() {
// -------------------------------------------------------------------------------------------------

}

// -------------------------------------------------------------------------------------------------
tResult CrossroadScriptFilter::Init(tInitStage eStage, __exception) {
// -------------------------------------------------------------------------------------------------
  // never miss calling the parent implementation!!
  RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr))
  
  // Initialize the descriptors and media type stuff here
  cObjectPtr<IMediaDescriptionManager> description_manager;
  RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER,
    IID_ADTF_MEDIA_DESCRIPTION_MANAGER,(tVoid**)&description_manager,__exception_ptr));

  if (eStage == StageFirst) {
    // Create and register the output pin
    tChar const * bool_stream_description = description_manager->GetMediaDescription("tBoolSignalValue");
    RETURN_IF_POINTER_NULL(bool_stream_description);        
    cObjectPtr<IMediaType> bool_type = new cMediaType(0, 0, 0, "tBoolSignalValue",
      bool_stream_description, IMediaDescription::MDF_DDL_DEFAULT_VERSION);	
		RETURN_IF_FAILED(bool_type->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION,
		  (tVoid**)&bool_stream_description_));

    RETURN_IF_FAILED(active_flag_input_pin_.Create("active", bool_type, static_cast<IPinEventSink*>(this)));
		RETURN_IF_FAILED(RegisterPin(&active_flag_input_pin_));
		RETURN_IF_FAILED(inactive_flag_output_pin_.Create("inactive", bool_type, NULL));
		RETURN_IF_FAILED(RegisterPin(&inactive_flag_output_pin_));
		
		tChar const * output_stream_description = description_manager->GetMediaDescription("tSignalValue");
    RETURN_IF_POINTER_NULL(output_stream_description);
    
    // Create the stream description used to receive/send data
    cObjectPtr<IMediaType> output_type = new cMediaType(0, 0, 0, "tSignalValue",
      output_stream_description, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(output_type->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION,
      (tVoid**) &output_stream_description_));

    RETURN_IF_FAILED(steering_angle_output_pin_.Create("steering_angle", output_type, NULL));
    RETURN_IF_FAILED(RegisterPin(&steering_angle_output_pin_));
    RETURN_IF_FAILED(speed_output_pin_.Create("speed", output_type, NULL));
    RETURN_IF_FAILED(RegisterPin(&speed_output_pin_));
  }  
  else if (eStage == StageGraphReady) {
    tUInt32 t = 250;
		this->SetInterval(t * 1000);
		counter_ = 0;
		active_ = false;
  }
  
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult CrossroadScriptFilter::Shutdown(tInitStage eStage, __exception) {
// -------------------------------------------------------------------------------------------------
  // Call the base class implementation
  return cFilter::Shutdown(eStage, __exception_ptr);
}

// -------------------------------------------------------------------------------------------------
tResult CrossroadScriptFilter::OnPinEvent(IPin* source, tInt event_code, tInt param1, tInt param2, IMediaSample* media_sample) {
// -------------------------------------------------------------------------------------------------
  RETURN_IF_POINTER_NULL(source);
	RETURN_IF_POINTER_NULL(media_sample);
	
	if (event_code == IPinEventSink::PE_MediaSampleReceived) {
		if (source == &active_flag_input_pin_) {
		  active_ = true;
		  std::cout << "Set active true" << std::endl;
		}
  }
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult CrossroadScriptFilter::Cycle(__exception) {
// -------------------------------------------------------------------------------------------------
  if (!active_) RETURN_NOERROR;
  if (counter_ < GetPropertyFloat("duration_forward") * 4) {
    int speed = GetPropertyInt("speed");
    std::cout << "Driving forward with " << speed << std::endl;
    TransmitSpeed(speed);
    TransmitAngle(0);
  } else if (counter_ < (GetPropertyFloat("duration_forward") + GetPropertyFloat("duration_turning")) * 4) {
    std::cout << "Turning right" << std::endl;
    TransmitSpeed(GetPropertyInt("speed"));
    TransmitAngle(GetPropertyFloat("angle"));
  } else {
    std::cout << "Set active false" << std::endl;
    TransmitInactiveFlag();
    counter_ = 0;
    active_ = false;
  }
  counter_++;
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
void CrossroadScriptFilter::TransmitInactiveFlag() {
// -------------------------------------------------------------------------------------------------
  cObjectPtr<IMediaSample> pMediaSample;
  AllocMediaSample((tVoid**)&pMediaSample);

  //allocate memory with the size given by the descriptor
  cObjectPtr<IMediaSerializer> pSerializer;
  bool_stream_description_->GetMediaSampleSerializer(&pSerializer);
  tInt nSize = pSerializer->GetDeserializedSize();
  pMediaSample->AllocBuffer(nSize);

  bool inactive = true;
  tUInt32 timeStamp = 0;
  
  {
    __adtf_sample_write_lock_mediadescription(bool_stream_description_, pMediaSample, coder);
    coder->Set("bValue", (tVoid*) &inactive);
    coder->Set("ui32ArduinoTimestamp", (tVoid*) &timeStamp);
  }

  //transmit media sample over output pin
  pMediaSample->SetTime(_clock->GetStreamTime());
  inactive_flag_output_pin_.Transmit(pMediaSample);
}

// -------------------------------------------------------------------------------------------------
void CrossroadScriptFilter::TransmitSpeed(float speed) {
// -------------------------------------------------------------------------------------------------
  cObjectPtr<IMediaSample> pMediaSample;
  AllocMediaSample((tVoid**)&pMediaSample);

  //allocate memory with the size given by the descriptor
  cObjectPtr<IMediaSerializer> pSerializer;
  output_stream_description_->GetMediaSampleSerializer(&pSerializer);
  tInt nSize = pSerializer->GetDeserializedSize();
  pMediaSample->AllocBuffer(nSize);

  tUInt32 timeStamp = 0;
  
  {
    __adtf_sample_write_lock_mediadescription(output_stream_description_, pMediaSample, coder);
    coder->Set("f32Value", (tVoid*) &speed);
    coder->Set("ui32ArduinoTimestamp", (tVoid*) &timeStamp);
  }

  //transmit media sample over output pin
  pMediaSample->SetTime(_clock->GetStreamTime());
  speed_output_pin_.Transmit(pMediaSample);
}

// -------------------------------------------------------------------------------------------------
void CrossroadScriptFilter::TransmitAngle(float angle) {
// -------------------------------------------------------------------------------------------------
    cObjectPtr<IMediaSample> pMediaSample;
  AllocMediaSample((tVoid**)&pMediaSample);

  //allocate memory with the size given by the descriptor
  cObjectPtr<IMediaSerializer> pSerializer;
  output_stream_description_->GetMediaSampleSerializer(&pSerializer);
  tInt nSize = pSerializer->GetDeserializedSize();
  pMediaSample->AllocBuffer(nSize);

  tUInt32 timeStamp = 0;
  
  {
    __adtf_sample_write_lock_mediadescription(output_stream_description_, pMediaSample, coder);
    coder->Set("f32Value", (tVoid*) &angle);
    coder->Set("ui32ArduinoTimestamp", (tVoid*) &timeStamp);
  }

  //transmit media sample over output pin
  pMediaSample->SetTime(_clock->GetStreamTime());
  steering_angle_output_pin_.Transmit(pMediaSample);
}
