#include "stdafx.h"
#include "light_machine_filter.h"

ADTF_FILTER_PLUGIN("FRUIT Light Machine Filter", OID_ADTF_LIGHT_MACHINE_FILTER, LightMachineFilter)

// -------------------------------------------------------------------------------------------------
LightMachineFilter::LightMachineFilter(const tChar* __info) : cTimeTriggeredFilter(__info) {
// -------------------------------------------------------------------------------------------------
  SetPropertyBool("debug", false);
  SetPropertyInt("interval_ms", 200);
  SetPropertyInt("count_max", 2);
}

// -------------------------------------------------------------------------------------------------
LightMachineFilter::~LightMachineFilter() {
// -------------------------------------------------------------------------------------------------
}

// -------------------------------------------------------------------------------------------------
tResult LightMachineFilter::Init(tInitStage eStage, __exception) {
// -------------------------------------------------------------------------------------------------
    RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr))

    if (eStage == StageFirst) {
      cObjectPtr<IMediaDescriptionManager> pDescManager;
      RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER,IID_ADTF_MEDIA_DESCRIPTION_MANAGER,(tVoid**)&pDescManager,__exception_ptr));

      // Create the flag media description
      tChar const * boolValue = pDescManager->GetMediaDescription("tBoolSignalValue");
      RETURN_IF_POINTER_NULL(boolValue);
      cObjectPtr<IMediaType> bool_value_type = new cMediaType(0, 0, 0, "tBoolSignalValue", boolValue, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
      RETURN_IF_FAILED(bool_value_type->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&bool_description_));
      
      // Create the flag media description
      tChar const * boolValueOutput = pDescManager->GetMediaDescription("tBoolSignalValue");
      RETURN_IF_POINTER_NULL(boolValueOutput);
      cObjectPtr<IMediaType> bool_value_type_output = new cMediaType(0, 0, 0, "tBoolSignalValue", boolValueOutput, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
      RETURN_IF_FAILED(bool_value_type_output->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&bool_description_output_));

      // Create the input pins
      RETURN_IF_FAILED(headlights_input_.Create("headLightEnabledIn", bool_value_type, static_cast<IPinEventSink*>(this)));
      RETURN_IF_FAILED(RegisterPin(&headlights_input_));
      
      RETURN_IF_FAILED(brakelights_input_.Create("brakeLightEnabledIn", bool_value_type, static_cast<IPinEventSink*>(this)));
      RETURN_IF_FAILED(RegisterPin(&brakelights_input_));
      
      RETURN_IF_FAILED(reverselights_input_.Create("reverseLightsEnabledIn", bool_value_type, static_cast<IPinEventSink*>(this)));
      RETURN_IF_FAILED(RegisterPin(&reverselights_input_));
      
      RETURN_IF_FAILED(turn_right_input_.Create("turnSignalRightEnabledIn", bool_value_type, static_cast<IPinEventSink*>(this)));
      RETURN_IF_FAILED(RegisterPin(&turn_right_input_));
      
      RETURN_IF_FAILED(turn_left_input_.Create("turnSignalLeftEnabledIn", bool_value_type, static_cast<IPinEventSink*>(this)));
      RETURN_IF_FAILED(RegisterPin(&turn_left_input_));
      
      RETURN_IF_FAILED(hazardlights_input_.Create("hazardLightsEnabledIn", bool_value_type, static_cast<IPinEventSink*>(this)));
      RETURN_IF_FAILED(RegisterPin(&hazardlights_input_));
      
      // Create output pins
      RETURN_IF_FAILED(headlights_output_.Create("headLightEnabledOut", bool_value_type, NULL));
      RETURN_IF_FAILED(RegisterPin(&headlights_output_));
      
      RETURN_IF_FAILED(brakelights_output_.Create("brakeLightEnabledOut", bool_value_type, NULL));
      RETURN_IF_FAILED(RegisterPin(&brakelights_output_));
      
      RETURN_IF_FAILED(reverselights_output_.Create("reverseLightsEnabledOut", bool_value_type, NULL));
      RETURN_IF_FAILED(RegisterPin(&reverselights_output_));
      
      RETURN_IF_FAILED(turn_right_output_.Create("turnSignalRightEnabledOut", bool_value_type, NULL));
      RETURN_IF_FAILED(RegisterPin(&turn_right_output_));
      
      RETURN_IF_FAILED(turn_left_output_.Create("turnSignalLeftEnabledOut", bool_value_type, NULL));
      RETURN_IF_FAILED(RegisterPin(&turn_left_output_));
      
      RETURN_IF_FAILED(hazardlights_output_.Create("hazardLightsEnabledOut", bool_value_type, NULL));
      RETURN_IF_FAILED(RegisterPin(&hazardlights_output_));

    }

    else if(eStage == StageGraphReady) {
      SetInterval(GetPropertyInt("interval_ms") * 1000);
      
      headlights_on_ = false;
      brakelights_on_ = false;
      reverselights_on_ = false;
      turn_right_on_ = false;
      turn_left_on_ = false;
      hazardlights_on_ = false;
      
      headlights_counter_ = 0;
      brakelights_counter_ = 0;
      reverselights_counter_ = 0;
      turn_right_counter_ = 0;
      turn_left_counter_ = 0;
      hazardlights_counter_ = 0;
    }
                
    RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult LightMachineFilter::Shutdown(tInitStage eStage, __exception) {
// -------------------------------------------------------------------------------------------------
  // Call the base class implementation
  return cFilter::Shutdown(eStage, __exception_ptr);
}

// -------------------------------------------------------------------------------------------------
tResult LightMachineFilter::Cycle(__exception) {
// -------------------------------------------------------------------------------------------------

  if(headlights_counter_ > 0) {
    if(GetPropertyBool("debug")) LOG_INFO(cString::Format("headlights   : %d", headlights_on_));
    headlights_counter_--;
    sendLightSignal(headlights_output_, headlights_on_);
  }
  if(brakelights_counter_ > 0) {
    if(GetPropertyBool("debug")) LOG_INFO(cString::Format("brakelights  : %d", brakelights_on_));
    brakelights_counter_--;
    sendLightSignal(brakelights_output_, brakelights_on_);
  }
  if(reverselights_counter_ > 0) {
    if(GetPropertyBool("debug")) LOG_INFO(cString::Format("reverselights: %d", reverselights_on_));
    reverselights_counter_--;
    sendLightSignal(reverselights_output_, reverselights_on_);
  }
  if(turn_right_counter_ > 0) {
    if(GetPropertyBool("debug")) LOG_INFO(cString::Format("turn_right   : %d", turn_right_on_));
    turn_right_counter_--;
    sendLightSignal(turn_right_output_, turn_right_on_);
  }
  if(turn_left_counter_ > 0) {
    if(GetPropertyBool("debug")) LOG_INFO(cString::Format("turn_left    : %d", turn_left_on_));
    turn_left_counter_--;
    sendLightSignal(turn_left_output_, turn_left_on_);
  }
  if(hazardlights_counter_ > 0) {
    if(GetPropertyBool("debug")) LOG_INFO(cString::Format("hazardlights : %d", hazardlights_on_));
    hazardlights_counter_--;
    sendLightSignal(hazardlights_output_, hazardlights_on_);
  }
  
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult LightMachineFilter::OnPinEvent(IPin* pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample* media_sample) {
// -------------------------------------------------------------------------------------------------
  if (nEventCode == IPinEventSink::PE_MediaSampleReceived) {
    bool value = false;
    tTimeStamp timeStamp = 0;
    
    {
      __adtf_sample_read_lock_mediadescription(bool_description_, media_sample, coder);
      coder->Get("bValue", (tVoid*)&value);
      coder->Get("ui32ArduinoTimestamp", (tVoid*)&timeStamp);
    }
    if (pSource == &headlights_input_) {
      setHeadlightsValue(value);
    }
    else if (pSource == &brakelights_input_ && value != brakelights_on_) {
      setBrakelightsValue(value);
    }
    else if (pSource == &reverselights_input_ && value != reverselights_on_) {
      setReverselightsValue(value);
    }
    else if (pSource == &turn_right_input_ && value != turn_right_on_) {
      setTurnRightValue(value);
      setTurnLeftValue(false);
      setHazardlightsValue(false);
    }
    else if (pSource == &turn_left_input_ && value != turn_left_on_) {
      setTurnLeftValue(value);
      setTurnRightValue(false);
      setHazardlightsValue(false);
    }
    else if (pSource == &hazardlights_input_ && value != hazardlights_on_) {
      setHazardlightsValue(value);
      setTurnRightValue(false);
      setTurnLeftValue(false);
    }
  }
  
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
void LightMachineFilter::setHeadlightsValue(bool value) {
// -------------------------------------------------------------------------------------------------
  if(value != headlights_on_) {
    headlights_on_ = value;
    headlights_counter_ = (size_t)GetPropertyInt("count_max");
  }
}

// -------------------------------------------------------------------------------------------------
void LightMachineFilter::setBrakelightsValue(bool value) {
// -------------------------------------------------------------------------------------------------
  if(value != brakelights_on_) {
    brakelights_on_ = value;
    brakelights_counter_ = (size_t)GetPropertyInt("count_max");
  }
}

// -------------------------------------------------------------------------------------------------
void LightMachineFilter::setReverselightsValue(bool value) {
// -------------------------------------------------------------------------------------------------
  if(value != reverselights_on_) {
    reverselights_on_ = value;
    reverselights_counter_ = (size_t)GetPropertyInt("count_max");
  }
}

// -------------------------------------------------------------------------------------------------
void LightMachineFilter::setTurnRightValue(bool value) {
// -------------------------------------------------------------------------------------------------
  if(value != turn_right_on_) {
    turn_right_on_ = value;
    turn_right_counter_ = (size_t)GetPropertyInt("count_max");
  }
}

// -------------------------------------------------------------------------------------------------
void LightMachineFilter::setTurnLeftValue(bool value) {
// -------------------------------------------------------------------------------------------------
  if(value != turn_left_on_) {
    turn_left_on_ = value;
    turn_left_counter_ = (size_t)GetPropertyInt("count_max");
  }
}

// -------------------------------------------------------------------------------------------------
void LightMachineFilter::setHazardlightsValue(bool value) {
// -------------------------------------------------------------------------------------------------
  if(value != hazardlights_on_) {
    hazardlights_on_ = value;
    hazardlights_counter_ = (size_t)GetPropertyInt("count_max");
  }
}

// -------------------------------------------------------------------------------------------------
tResult LightMachineFilter::sendLightSignal(cOutputPin & pin, bool value) {
// -------------------------------------------------------------------------------------------------
  tUInt32 timeStamp = 0;
  
  cObjectPtr<IMediaSample> media_sample;
  RETURN_IF_FAILED(AllocMediaSample((tVoid**)&media_sample));

  cObjectPtr<IMediaSerializer> serializer;
  bool_description_output_->GetMediaSampleSerializer(&serializer);
  tInt size = serializer->GetDeserializedSize();

  RETURN_IF_FAILED(media_sample->AllocBuffer(size));
  
  {
    __adtf_sample_write_lock_mediadescription(bool_description_output_, media_sample, coder);
    coder->Set("bValue", (tVoid*)&value);
    coder->Set("ui32ArduinoTimestamp", (tVoid*)&timeStamp);
  }
  
  pin.Transmit(media_sample);
  RETURN_NOERROR;
}

