#include "stdafx.h"
#include "signal_sender_filter.h"
#include <template_data.h>

// -------------------------------------------------------------------------------------------------
ADTF_FILTER_PLUGIN("FRUIT Sender Filter Signal", OID_ADTF_SIGNAL_SENDER_FILTER, SignalSenderFilter);
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
SignalSenderFilter::SignalSenderFilter(const tChar* __info) : adtf::cTimeTriggeredFilter(__info) {
// -------------------------------------------------------------------------------------------------
  SetPropertyInt("value", 1);
  SetPropertyInt("start_delay", 10);
  SetPropertyInt("interval_ms", 100);
  SetPropertyBool("debug", false);
}

// -------------------------------------------------------------------------------------------------
SignalSenderFilter::~SignalSenderFilter() {
// -------------------------------------------------------------------------------------------------

}

// -------------------------------------------------------------------------------------------------
tResult SignalSenderFilter::Init(tInitStage eStage, __exception) {
// -------------------------------------------------------------------------------------------------
  // never miss calling the parent implementation!!
  RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr))
  
  // Initialize the descriptors and media type stuff here
  cObjectPtr<IMediaDescriptionManager> description_manager;
  RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER,
    IID_ADTF_MEDIA_DESCRIPTION_MANAGER,(tVoid**)&description_manager,__exception_ptr));

  if (eStage == StageFirst) {
    // Create and register the output pin
    tChar const * stream_description = description_manager->GetMediaDescription("tSignalValue");
    RETURN_IF_POINTER_NULL(stream_description);        
    cObjectPtr<IMediaType> output_type = new cMediaType(0, 0, 0, "tSignalValue",
      stream_description,IMediaDescription::MDF_DDL_DEFAULT_VERSION);	
		RETURN_IF_FAILED(output_type->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION,
		  (tVoid**)&output_stream_description_));

    RETURN_IF_FAILED(data_.Create("signal", output_type, NULL));
		RETURN_IF_FAILED(RegisterPin(&data_));
  }
  
  else if (eStage == StageNormal) {
    // In this stage you would do further initialisation and/or create your dynamic pins.
    // Please take a look at the demo_dynamicpin example for further reference.
  }
  
  else if (eStage == StageGraphReady) {
    // All pin connections have been established in this stage so you can query your pins
    // about their media types and additional meta data.
    // Please take a look at the demo_imageproc example for further reference.
		this->SetInterval(GetPropertyInt("interval_ms") * 1000);
		
		counter_ = 0;
		debug_ = GetPropertyBool("debug");
  }
  
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult SignalSenderFilter::Shutdown(tInitStage eStage, __exception) {
// -------------------------------------------------------------------------------------------------
  // Call the base class implementation
  return cFilter::Shutdown(eStage, __exception_ptr);
}

// -------------------------------------------------------------------------------------------------
tResult SignalSenderFilter::Cycle(__exception) {
// -------------------------------------------------------------------------------------------------
    if (counter_ < GetPropertyInt("start_delay") * (1000.0f / GetPropertyInt("interval_ms"))) {
      counter_++;
      RETURN_NOERROR;
    }
    
    cObjectPtr<IMediaSample> pMediaSample;
    AllocMediaSample((tVoid**)&pMediaSample);

    //allocate memory with the size given by the descriptor
    cObjectPtr<IMediaSerializer> pSerializer;
    output_stream_description_->GetMediaSampleSerializer(&pSerializer);
    tInt nSize = pSerializer->GetDeserializedSize();
    pMediaSample->AllocBuffer(nSize);

    tFloat32 value = (tFloat32) GetPropertyInt("value");
    tUInt32 timeStamp = 0;
    
    {
      __adtf_sample_write_lock_mediadescription(output_stream_description_, pMediaSample, coder);
      coder->Set("f32Value", (tVoid*) &value);
      coder->Set("ui32ArduinoTimestamp", (tVoid*) &timeStamp);
    }

    //transmit media sample over output pin
    pMediaSample->SetTime(_clock->GetStreamTime());
    if (debug_) LOG_INFO(cString::Format("Sending value: %f", value));
    data_.Transmit(pMediaSample);
    RETURN_NOERROR;
}
