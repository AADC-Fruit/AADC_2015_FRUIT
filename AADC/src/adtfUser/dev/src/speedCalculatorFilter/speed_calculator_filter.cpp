#include "stdafx.h"
#include "cSlidingWindow.h"
#include "speed_calculator_filter.h"
#include <iostream>

ADTF_FILTER_PLUGIN("FRUIT Speed Calculator Filter", OID_ADTF_SPEED_CALCULATOR_FILTER, SpeedCalculatorFilter)

// -------------------------------------------------------------------------------------------------
SpeedCalculatorFilter::SpeedCalculatorFilter(const tChar* __info) : cFilter(__info), sliding_window_left_(10), sliding_window_right_(10) {
// -------------------------------------------------------------------------------------------------
}

// -------------------------------------------------------------------------------------------------
SpeedCalculatorFilter::~SpeedCalculatorFilter() {
// -------------------------------------------------------------------------------------------------
}

// -------------------------------------------------------------------------------------------------
tResult SpeedCalculatorFilter::Init(tInitStage eStage, __exception) {
// -------------------------------------------------------------------------------------------------
  RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr))

  if (eStage == StageFirst) {
    cObjectPtr<IMediaDescriptionManager> pDescManager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER,IID_ADTF_MEDIA_DESCRIPTION_MANAGER,(tVoid**)&pDescManager,__exception_ptr));

    tChar const * strDescSignalValue = pDescManager->GetMediaDescription("tSignalValue");
    RETURN_IF_POINTER_NULL(strDescSignalValue);        
    cObjectPtr<IMediaType> pTypeSignalValue = new cMediaType(0, 0, 0, "tSignalValue", strDescSignalValue,IMediaDescription::MDF_DDL_DEFAULT_VERSION);    
    RETURN_IF_FAILED(pTypeSignalValue->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&signal_description_input_)); 
    
    tChar const * strDescSignalValueOutput = pDescManager->GetMediaDescription("tSignalValue");
    RETURN_IF_POINTER_NULL(strDescSignalValueOutput);        
    cObjectPtr<IMediaType> pTypeSignalValueOutput = new cMediaType(0, 0, 0, "tSignalValue", strDescSignalValueOutput,IMediaDescription::MDF_DDL_DEFAULT_VERSION);    
    RETURN_IF_FAILED(pTypeSignalValueOutput->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&signal_description_output_)); 
    
    tChar const * strDescSignalValueMotor = pDescManager->GetMediaDescription("tSignalValue");
    RETURN_IF_POINTER_NULL(strDescSignalValueMotor);        
    cObjectPtr<IMediaType> pTypeSignalValueMotor = new cMediaType(0, 0, 0, "tSignalValue", strDescSignalValueMotor,IMediaDescription::MDF_DDL_DEFAULT_VERSION);    
    RETURN_IF_FAILED(pTypeSignalValueMotor->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&signal_description_motor_)); 
    
    RETURN_IF_FAILED(input_counter_left_.Create("wheel_speed_sensor_left", pTypeSignalValue, static_cast<IPinEventSink*> (this)));
    RETURN_IF_FAILED(RegisterPin(&input_counter_left_));
    RETURN_IF_FAILED(input_counter_right_.Create("wheel_speed_sensor_right", pTypeSignalValue, static_cast<IPinEventSink*> (this)));
    RETURN_IF_FAILED(RegisterPin(&input_counter_right_));

    RETURN_IF_FAILED(input_motor_direction_.Create("motor_direction", pTypeSignalValue, static_cast<IPinEventSink*> (this)));
    RETURN_IF_FAILED(RegisterPin(&input_motor_direction_));
    
    RETURN_IF_FAILED(output_speed_left_.Create("speed_left_wheel", pTypeSignalValue, NULL));
    RETURN_IF_FAILED(RegisterPin(&output_speed_left_));
    RETURN_IF_FAILED(output_speed_right_.Create("speed_right_wheel", pTypeSignalValue, NULL));
    RETURN_IF_FAILED(RegisterPin(&output_speed_right_));
    
    left_speed_ = 0;
    right_speed_ = 0;
    left_ = false;

    RETURN_NOERROR;
  }
  else if (eStage == StageNormal) {
    
  }
  else if(eStage == StageGraphReady) {
    
  }
                
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult SpeedCalculatorFilter::OnPinEvent(IPin* pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample* pMediaSample) {
// -------------------------------------------------------------------------------------------------
  if (nEventCode == IPinEventSink::PE_MediaSampleReceived && pMediaSample != NULL && signal_description_input_ != NULL) {
    if (pSource == &input_motor_direction_) {
      //write values with zero
      tFloat32 value = 0;
      tUInt32 timeStamp = 0;
      
      {
        __adtf_sample_read_lock_mediadescription(signal_description_motor_, pMediaSample, coder);
        coder->Get("f32Value", (tVoid*) &value);
        coder->Get("ui32ArduinoTimestamp", (tVoid*) &timeStamp);
      }
      
      if(value > 0.00001) motor_direction_ = 1;
      else if(fabs(value) < 0.00001) motor_direction_ = 0;
      else if(value < -0.00001) motor_direction_ = -1;
    }
    
    else {
      //write values with zero
      tFloat32 value = 0;
      tUInt32 timeStamp = 0;
      
      {
        __adtf_sample_read_lock_mediadescription(signal_description_input_, pMediaSample, coder);
        coder->Get("f32Value", (tVoid*) &value);
        coder->Get("ui32ArduinoTimestamp", (tVoid*) &timeStamp);
      }
      
      if (pSource == &input_counter_left_) {                    
        //processing the data
        sliding_window_left_.addNewValue(value,GetTime());
        if (sliding_window_left_.getEndValue()!=0) {                      
          left_ = true;
          calculateSpeed(sliding_window_left_.getEndValue(),sliding_window_left_.getBeginValue(), sliding_window_left_.getEndTime(),sliding_window_left_.getBeginTime());
        }
        
        if(left_speed_ < THRESHOLD){
          // Transmit speed
          transmitSpeed(output_speed_left_, left_speed_, timeStamp);
        }
      }
      
      else if (pSource == &input_counter_right_) {
        //processing the data
        sliding_window_right_.addNewValue(value,GetTime());
        if (sliding_window_right_.getEndValue()!=0) {                       
            left_ = false;
            calculateSpeed(sliding_window_right_.getEndValue(),sliding_window_right_.getBeginValue(), 
                sliding_window_right_.getEndTime(),sliding_window_right_.getBeginTime());
        }
        if(right_speed_ < THRESHOLD){
          // Transmit speed
          transmitSpeed(output_speed_right_, right_speed_, timeStamp);
        }
      }
    }
  }
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tTimeStamp SpeedCalculatorFilter::GetTime() {
// -------------------------------------------------------------------------------------------------
  return (_clock != NULL) ? _clock->GetTime () : cSystem::GetTime();
}

// -------------------------------------------------------------------------------------------------
void SpeedCalculatorFilter::calculateSpeed( tFloat32 counter_value, tFloat32 last_counter_value,tTimeStamp current_timestamp, tTimeStamp last_timestamp ) {
// -------------------------------------------------------------------------------------------------
  long delta_time = long(current_timestamp - last_timestamp);  //microseconds
  unsigned long delta_counter = long(counter_value) - long(last_counter_value);  
  tFloat32 delta_rounds = tFloat32(delta_counter)/8.0f;  //one round = 8 counts
  tFloat32 rpms;
  if (delta_time!=0) {
      rpms = tFloat32(delta_rounds/delta_time);
  } else {
      rpms = 0;
  }
  rpms *= motor_direction_;
  if(left_) left_speed_ = rpms * 1E5 * 0.1 * M_PI;
  else right_speed_ = rpms * 1E5 * 0.1 * M_PI;
}

// -------------------------------------------------------------------------------------------------
tResult SpeedCalculatorFilter::transmitSpeed(cOutputPin & pin, float value, tTimeStamp time_stamp) {
// -------------------------------------------------------------------------------------------------
  cObjectPtr<IMediaSample> media_sample;
  AllocMediaSample((tVoid**)&media_sample);

  //allocate memory with the size given by the descriptor
  cObjectPtr<IMediaSerializer> serializer;
  signal_description_output_->GetMediaSampleSerializer(&serializer);
  tInt size = serializer->GetDeserializedSize();
  media_sample->AllocBuffer(size);

  {
    __adtf_sample_write_lock_mediadescription(signal_description_output_, media_sample, coder);
    coder->Set("f32Value", (tVoid*) &left_speed_);
    coder->Set("ui32ArduinoTimestamp", (tVoid*) &time_stamp);
  }

  media_sample->SetTime(GetTime());
  pin.Transmit(media_sample);
  RETURN_NOERROR;
}
