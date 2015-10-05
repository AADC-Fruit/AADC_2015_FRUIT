#include "stdafx.h"
#include "wheel_lock_filter.h"

ADTF_FILTER_PLUGIN("FRUIT Wheel Lock Filter", OID_ADTF_WHEEL_LOCK_FILTER, WheelLockFilter)

// -------------------------------------------------------------------------------------------------
WheelLockFilter::WheelLockFilter(const tChar* __info) : cTimeTriggeredFilter(__info) {
// -------------------------------------------------------------------------------------------------
  SetPropertyBool("debug", false);
  SetPropertyBool("start_active", false);
  SetPropertyInt("interval_ms", 100);
  SetPropertyInt("brake_duration_s", 1);
  SetPropertyInt("count_max", 2);
}

// -------------------------------------------------------------------------------------------------
WheelLockFilter::~WheelLockFilter() {
// -------------------------------------------------------------------------------------------------
}

// -------------------------------------------------------------------------------------------------
tResult WheelLockFilter::Init(tInitStage eStage, __exception) {
// -------------------------------------------------------------------------------------------------
    RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr))

    if (eStage == StageFirst) {
      cObjectPtr<IMediaDescriptionManager> pDescManager;
      RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER,IID_ADTF_MEDIA_DESCRIPTION_MANAGER,(tVoid**)&pDescManager,__exception_ptr));

      // Create the signal media description
      tChar const * signalValue = pDescManager->GetMediaDescription("tSignalValue");
      RETURN_IF_POINTER_NULL(signalValue);        
      cObjectPtr<IMediaType> signal_value_type = new cMediaType(0, 0, 0, "tSignalValue", signalValue, IMediaDescription::MDF_DDL_DEFAULT_VERSION);    
      RETURN_IF_FAILED(signal_value_type->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&signal_description_)); 
      
      // Create the signal media description
      tChar const * signalValueOutput = pDescManager->GetMediaDescription("tSignalValue");
      RETURN_IF_POINTER_NULL(signalValueOutput);        
      cObjectPtr<IMediaType> signal_value_type_output = new cMediaType(0, 0, 0, "tSignalValue", signalValueOutput, IMediaDescription::MDF_DDL_DEFAULT_VERSION);    
      RETURN_IF_FAILED(signal_value_type_output->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&signal_description_output_)); 

      // Create the signal input pins
      RETURN_IF_FAILED(steering_input_.Create("steering_in", signal_value_type, static_cast<IPinEventSink*>(this)));
      RETURN_IF_FAILED(RegisterPin(&steering_input_));
      RETURN_IF_FAILED(motor_input_.Create("accel_in", signal_value_type, static_cast<IPinEventSink*>(this)));
      RETURN_IF_FAILED(RegisterPin(&motor_input_));

      // Create the flag media description
      tChar const * juryFlagValue = pDescManager->GetMediaDescription("tBoolSignalValue");
      RETURN_IF_POINTER_NULL(juryFlagValue);
      cObjectPtr<IMediaType> jury_flag_value_type = new cMediaType(0, 0, 0, "tBoolSignalValue", juryFlagValue, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
      RETURN_IF_FAILED(jury_flag_value_type->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&jury_flag_description_));
      
      // Create the flag media description
      tChar const * emergencyFlagValue = pDescManager->GetMediaDescription("tBoolSignalValue");
      RETURN_IF_POINTER_NULL(emergencyFlagValue);
      cObjectPtr<IMediaType> emergency_flag_value_type = new cMediaType(0, 0, 0, "tBoolSignalValue", emergencyFlagValue, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
      RETURN_IF_FAILED(emergency_flag_value_type->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&emergency_flag_description_));
      
      // Create the flag media description
      tChar const * flagValueOutput = pDescManager->GetMediaDescription("tBoolSignalValue");
      RETURN_IF_POINTER_NULL(flagValueOutput);
      cObjectPtr<IMediaType> flag_value_type_output = new cMediaType(0, 0, 0, "tBoolSignalValue", flagValueOutput, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
      RETURN_IF_FAILED(flag_value_type_output->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&flag_description_output_));

      // Create the jury input pin
      RETURN_IF_FAILED(jury_flag_input_.Create("jury_flag_in", jury_flag_value_type, static_cast<IPinEventSink*>(this)));
      RETURN_IF_FAILED(RegisterPin(&jury_flag_input_));
      
      // Create the emergency input pin
      RETURN_IF_FAILED(emergency_flag_input_.Create("emergency_flag_in", emergency_flag_value_type, static_cast<IPinEventSink*>(this)));
      RETURN_IF_FAILED(RegisterPin(&emergency_flag_input_));
      
      // Create output pins
      RETURN_IF_FAILED(steering_output_.Create("steering_out", signal_value_type, NULL));
		  RETURN_IF_FAILED(RegisterPin(&steering_output_));
		  
		  RETURN_IF_FAILED(motor_output_.Create("accel_out", signal_value_type, NULL));
		  RETURN_IF_FAILED(RegisterPin(&motor_output_));
		  
		  RETURN_IF_FAILED(brake_light_output_.Create("brake_light_enabled", flag_value_type_output, NULL));
		  RETURN_IF_FAILED(RegisterPin(&brake_light_output_));

		  RETURN_IF_FAILED(reverse_light_output_.Create("rev_light_enabled", flag_value_type_output, NULL));
		  RETURN_IF_FAILED(RegisterPin(&reverse_light_output_));
		  
		  RETURN_IF_FAILED(driving_output_.Create("driving", flag_value_type_output, NULL));
		  RETURN_IF_FAILED(RegisterPin(&driving_output_));

    }

    else if(eStage == StageGraphReady) {
      SetInterval(GetPropertyInt("interval_ms") * 1000);
      jury_flag_ = GetPropertyBool("start_active");
      emergency_flag_ = true;
      was_running_ = GetPropertyBool("start_active");
      debug_ = GetPropertyBool("debug");
      
      current_accel_value_ = 0;
      current_steering_value_ = 0;
      accel_counter_ = 0;
      steering_counter_ = 0;
      brake_light_counter_ = 0;
      
      current_brake_light_value_ = false;
      current_reverse_light_value_ = false;
      
      driving_status_ = false;
      driving_status_changed_ = true;
    }
                
    RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult WheelLockFilter::Shutdown(tInitStage eStage, __exception) {
// -------------------------------------------------------------------------------------------------
  // Call the base class implementation
  return cFilter::Shutdown(eStage, __exception_ptr);
}

// -------------------------------------------------------------------------------------------------
tResult WheelLockFilter::Cycle(__exception) {
// -------------------------------------------------------------------------------------------------
  //bool pin logic!
  if(driving_status_changed_) {
    driving_status_changed_ = false;
    if(jury_flag_ && emergency_flag_) driving_status_ = true;
    else driving_status_ = false;
    // If the flag is again set to 0, stop the car.
    if (!driving_status_) {
      if (debug_) LOG_INFO("New flag value: Stop motor!");
      setCurrentAccel(0.0f);
      setCurrentSteering(0.0f);
      current_brake_light_value_ = true;
      //car is not driving!
    } else {
      if (debug_) LOG_INFO("New flag value: Start motor!");
      was_running_ = true;
      current_brake_light_value_ = false;
      //car is driving!
    }
  }
  
  if (jury_flag_ || was_running_) {
    if(current_brake_light_value_) {
      brake_light_counter_++;   
    }
    
    if(brake_light_counter_ > GetPropertyInt("brake_duration_s") * 1000 / GetPropertyInt("interval_ms")) {
      current_brake_light_value_ = false;
    }
    
    //if (debug_) LOG_INFO(cString::Format("Send brake signal  : %d", current_brake_light_value_));
    sendBrakeLightSignal(current_brake_light_value_);
    
    //if (debug_) LOG_INFO(cString::Format("Send reverse signal: %d", current_reverse_light_value_));
    sendReverseLightSignal(current_reverse_light_value_);
    
    if (debug_) LOG_INFO(cString::Format("Send running signal: %d", driving_status_));
    sendRunningSignal(driving_status_);
    
    if(accel_counter_ > 0) {
      //if (debug_) LOG_INFO(cString::Format("Send speed value   : %f", current_accel_value_));
      accel_counter_--;
      sendSignalValue(motor_output_, current_accel_value_);
    }
    
    if(steering_counter_ > 0) {
      //if (debug_) LOG_INFO(cString::Format("Send steering value   : %f", current_steering_value_));
      steering_counter_--;
      sendSignalValue(steering_output_, current_steering_value_);
    }
  }
  
  //LOG_INFO(cString::Format("Values: %d, %d", jury_flag_, emergency_flag_));
  
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult WheelLockFilter::OnPinEvent(IPin* pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample* pMediaSample) {
// -------------------------------------------------------------------------------------------------
    __synchronized_obj(critical_section_);
    
    if (nEventCode == IPinEventSink::PE_MediaSampleReceived) {        
      if (pSource == &jury_flag_input_) {
        bool flag = false;
        tTimeStamp timeStamp = 0;
        
        {
          __adtf_sample_read_lock_mediadescription(jury_flag_description_, pMediaSample, coder);
          coder->Get("bValue", (tVoid*)&flag);
          coder->Get("ui32ArduinoTimestamp", (tVoid*)&timeStamp);
        }
        //set new flag for later check
        if(jury_flag_ !=  flag){
          jury_flag_ = flag;
          driving_status_changed_ = true;
        }
        
        
      } else if(pSource == &emergency_flag_input_) {
        bool flag = false;
        tTimeStamp timeStamp = 0;
        
        {
          __adtf_sample_read_lock_mediadescription(emergency_flag_description_, pMediaSample, coder);
          coder->Get("bValue", (tVoid*)&flag);
          coder->Get("ui32ArduinoTimestamp", (tVoid*)&timeStamp);
        }
        //set new flag for later check
        if(emergency_flag_ != flag){
          emergency_flag_ = flag;
          driving_status_changed_ = true;
        }
        
      }
      
      else {
        tFloat32 value = 0;
        tTimeStamp timeStamp = 0;
        
        {
          __adtf_sample_read_lock_mediadescription(signal_description_, pMediaSample, coder);
          coder->Get("f32Value", (tVoid*)&value);
          coder->Get("ui32ArduinoTimestamp", (tVoid*)&timeStamp);
        }
        
        if (pSource == &steering_input_) {
          if (driving_status_) {
            if (fabs(current_steering_value_ - value) > 0.01) {
              if (debug_) LOG_INFO(cString::Format("New steering value: %f", value));
              setCurrentSteering(value);
            }
          }
        }
        
        else if (pSource == &motor_input_) {
          if (driving_status_) {
            if (fabs(current_accel_value_ - value) > 0.01) {
              if (debug_) LOG_INFO(cString::Format("New motor value: %f", value));
              
              if (value >= 0 && value < current_accel_value_) {
                current_brake_light_value_ = true;
                brake_light_counter_ = 0;
              } else if (value >= 0 && value >= current_accel_value_) {
                if (current_accel_value_ >= 0) current_brake_light_value_ = false;
                else {
                  current_brake_light_value_ = true;
                  brake_light_counter_ = 0;
                }
              }
              
              if (value < 0) {
                current_reverse_light_value_ = true;
                if (value > current_accel_value_) {
                  current_brake_light_value_ = true;
                  brake_light_counter_ = 0;
                }
                else current_brake_light_value_ = false;
              } else {
                current_reverse_light_value_ = false;
              }
              setCurrentAccel(value);
            }
            else {
              current_brake_light_value_ = false;
            }
          }
        }
      }
      
    }
    RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
void WheelLockFilter::setCurrentSteering(float value) {
// -------------------------------------------------------------------------------------------------
  current_steering_value_ = value;
  steering_counter_ = (size_t)GetPropertyInt("count_max");
}

// -------------------------------------------------------------------------------------------------
void WheelLockFilter::setCurrentAccel(float value) {
// -------------------------------------------------------------------------------------------------
  current_accel_value_ = value;
  accel_counter_ = (size_t)GetPropertyInt("count_max");
}

// -------------------------------------------------------------------------------------------------
tResult WheelLockFilter::sendRunningSignal(bool value) {
// -------------------------------------------------------------------------------------------------
  tUInt32 timeStamp = 0;
  
  cObjectPtr<IMediaSample> media_sample;
  RETURN_IF_FAILED(AllocMediaSample((tVoid**)&media_sample));

  cObjectPtr<IMediaSerializer> serializer;
  flag_description_output_->GetMediaSampleSerializer(&serializer);
  tInt size = serializer->GetDeserializedSize();

  RETURN_IF_FAILED(media_sample->AllocBuffer(size));
  
  {
    __adtf_sample_write_lock_mediadescription(flag_description_output_, media_sample, coder);
    coder->Set("bValue", (tVoid*)&value);
    coder->Set("ui32ArduinoTimestamp", (tVoid*)&timeStamp);
  }
  
  driving_output_.Transmit(media_sample);
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult WheelLockFilter::sendSignalValue(cOutputPin & pin, float value) {
// -------------------------------------------------------------------------------------------------
  tUInt32 timeStamp = 0;

  cObjectPtr<IMediaSample> media_sample;
  RETURN_IF_FAILED(AllocMediaSample((tVoid**)&media_sample));

  cObjectPtr<IMediaSerializer> serializer;
  signal_description_output_->GetMediaSampleSerializer(&serializer);
  tInt size = serializer->GetDeserializedSize();

  RETURN_IF_FAILED(media_sample->AllocBuffer(size));
  
  {
    __adtf_sample_write_lock_mediadescription(signal_description_output_, media_sample, coder);
    coder->Set("f32Value", (tVoid*)&value);
    coder->Set("ui32ArduinoTimestamp", (tVoid*)&timeStamp);
  }
  
  pin.Transmit(media_sample);
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult WheelLockFilter::sendBrakeLightSignal(bool value) {
// -------------------------------------------------------------------------------------------------
  tUInt32 timeStamp = 0;
  
  cObjectPtr<IMediaSample> media_sample;
  RETURN_IF_FAILED(AllocMediaSample((tVoid**)&media_sample));

  cObjectPtr<IMediaSerializer> serializer;
  flag_description_output_->GetMediaSampleSerializer(&serializer);
  tInt size = serializer->GetDeserializedSize();

  RETURN_IF_FAILED(media_sample->AllocBuffer(size));
  
  {
    __adtf_sample_write_lock_mediadescription(flag_description_output_, media_sample, coder);
    coder->Set("bValue", (tVoid*)&value);
    coder->Set("ui32ArduinoTimestamp", (tVoid*)&timeStamp);
  }
  
  current_brake_light_value_ = value;
  brake_light_output_.Transmit(media_sample);
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult WheelLockFilter::sendReverseLightSignal(bool value) {
// -------------------------------------------------------------------------------------------------
  tUInt32 timeStamp = 0;
  
  cObjectPtr<IMediaSample> media_sample;
  RETURN_IF_FAILED(AllocMediaSample((tVoid**)&media_sample));

  cObjectPtr<IMediaSerializer> serializer;
  flag_description_output_->GetMediaSampleSerializer(&serializer);
  tInt size = serializer->GetDeserializedSize();

  RETURN_IF_FAILED(media_sample->AllocBuffer(size));
  
  {
    __adtf_sample_write_lock_mediadescription(flag_description_output_, media_sample, coder);
    coder->Set("bValue", (tVoid*)&value);
    coder->Set("ui32ArduinoTimestamp", (tVoid*)&timeStamp);
  }
  
  current_reverse_light_value_ = value;
  reverse_light_output_.Transmit(media_sample);
  RETURN_NOERROR;
}
