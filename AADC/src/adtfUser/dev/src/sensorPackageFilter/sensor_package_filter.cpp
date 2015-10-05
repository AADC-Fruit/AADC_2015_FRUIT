#include "stdafx.h"
#include "sensor_package_filter.h"
#include <iostream>

ADTF_FILTER_PLUGIN("FRUIT Sensor Package Filter", OID_ADTF_SENSOR_PACKAGE_FILTER, SensorPackageFilter);

// -------------------------------------------------------------------------------------------------
SensorPackageFilter::SensorPackageFilter(const tChar* __info)
  : ir_l_fc_(BUFFER_SIZE), ir_l_fl_(BUFFER_SIZE), ir_l_fr_(BUFFER_SIZE), ir_s_fc_(BUFFER_SIZE),
    ir_s_fl_(BUFFER_SIZE), ir_s_fr_(BUFFER_SIZE), ir_s_l_(BUFFER_SIZE), ir_s_r_(BUFFER_SIZE),
    ir_s_rc_(BUFFER_SIZE), us_f_l_(BUFFER_SIZE), us_f_r_(BUFFER_SIZE), us_r_l_(BUFFER_SIZE),
    us_r_r_(BUFFER_SIZE) {
// -------------------------------------------------------------------------------------------------
  SetPropertyBool("debug", false);
  x_ = 0b1111111111111;
}

// -------------------------------------------------------------------------------------------------
tResult SensorPackageFilter::Init(tInitStage stage, __exception) {
// -------------------------------------------------------------------------------------------------
  RETURN_IF_FAILED(cFilter::Init(stage, __exception_ptr));
  
	if (stage == StageFirst) {
	  // Create and register the input pin
    cObjectPtr<IMediaDescriptionManager> description_manager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER,
      IID_ADTF_MEDIA_DESCRIPTION_MANAGER,(tVoid**)&description_manager,__exception_ptr));
    
    // Create the sensor value media description
    tChar const * sensor_description = description_manager->GetMediaDescription("tSignalValue");
    RETURN_IF_POINTER_NULL(sensor_description);        
    cObjectPtr<IMediaType> sensor_type = new cMediaType(0, 0, 0, "tSignalValue",
      sensor_description,IMediaDescription::MDF_DDL_DEFAULT_VERSION);	
    RETURN_IF_FAILED(sensor_type->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION,
      (tVoid**)&sensor_data_description_));
    
    // Create the sensor input pins
    RETURN_IF_FAILED(ir_short_front_left_input_pin_.Create("IR_front_left_shortrange", sensor_type, static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&ir_short_front_left_input_pin_));
    RETURN_IF_FAILED(ir_short_left_input_pin_.Create("IR_rear_left_shortrange", sensor_type, static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&ir_short_left_input_pin_));
    RETURN_IF_FAILED(ir_long_front_left_input_pin_.Create("IR_front_left_longrange", sensor_type, static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&ir_long_front_left_input_pin_));
    RETURN_IF_FAILED(ir_short_rear_center_input_pin_.Create("IR_rear_center_shortrange", sensor_type, static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&ir_short_rear_center_input_pin_));
    RETURN_IF_FAILED(us_front_right_input_pin_.Create("US_range_front_right", sensor_type, static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&us_front_right_input_pin_));
    RETURN_IF_FAILED(ir_short_front_center_input_pin_.Create("IR_front_center_shortrange", sensor_type, static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&ir_short_front_center_input_pin_));
    RETURN_IF_FAILED(ir_short_front_right_input_pin_.Create("IR_front_right_shortrange", sensor_type, static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&ir_short_front_right_input_pin_));
    RETURN_IF_FAILED(us_front_left_input_pin_.Create("US_range_front_left", sensor_type, static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&us_front_left_input_pin_));
    RETURN_IF_FAILED(ir_long_front_center_input_pin_.Create("IR_front_center_longrange", sensor_type, static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&ir_long_front_center_input_pin_));
    RETURN_IF_FAILED(ir_long_front_right_input_pin_.Create("IR_front_right_longrange", sensor_type, static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&ir_long_front_right_input_pin_));
    RETURN_IF_FAILED(us_rear_right_input_pin_.Create("US_range_rear_right", sensor_type, static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&us_rear_right_input_pin_));
    RETURN_IF_FAILED(ir_short_right_input_pin_.Create("IR_rear_right_shortrange", sensor_type, static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&ir_short_right_input_pin_));
    RETURN_IF_FAILED(us_rear_left_input_pin_.Create("US_range_rear_left", sensor_type, static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&us_rear_left_input_pin_));
    
    // Create the output pin description
    tChar const * package_description = description_manager->GetMediaDescription("SensorPackage");
    RETURN_IF_POINTER_NULL(package_description);        
    cObjectPtr<IMediaType> package_type = new cMediaType(0, 0, 0, "SensorPackage", package_description, IMediaDescription::MDF_DDL_DEFAULT_VERSION);	
    RETURN_IF_FAILED(package_type->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&sensor_package_description_));
    
    // Create the output pin
    RETURN_IF_FAILED(sensor_package_output_.Create("sensorPackage", package_type, NULL));
    RETURN_IF_FAILED(RegisterPin(&sensor_package_output_));
	}
	
	RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult SensorPackageFilter::Shutdown(tInitStage stage, __exception) {
// -------------------------------------------------------------------------------------------------
	return cFilter::Shutdown(stage,__exception_ptr);
}

// -------------------------------------------------------------------------------------------------
tResult SensorPackageFilter::OnPinEvent(IPin* source, tInt event_code, tInt param1, tInt param2,
    IMediaSample* media_sample) {
// -------------------------------------------------------------------------------------------------
	RETURN_IF_POINTER_NULL(source);
	RETURN_IF_POINTER_NULL(media_sample);
	
	// Create a synchronizer, that enters the critical section whenever possible. If a previous
	// thread is still inside the critical section, all proceeding threads have to wait for the
	// first to leave.
	__synchronized_obj(critical_section_);
	
	if (event_code == IPinEventSink::PE_MediaSampleReceived) {
	  tFloat32 sensor_value = 0;
	  tUInt32 time_stamp = 0;

    {
      __adtf_sample_read_lock_mediadescription(sensor_data_description_, media_sample, coder);

      coder->Get("f32Value", (tVoid*) &sensor_value);
      coder->Get("ui32ArduinoTimestamp", (tVoid*) &time_stamp);              
    }
    
	  if (source == &ir_long_front_center_input_pin_) {
      ir_l_fc_.add(sensor_value);
      x_ &= 0b1111111111110;
	  }
	
	  else if (source == &ir_long_front_left_input_pin_) {
	    ir_l_fl_.add(sensor_value);
	    x_ &= 0b1111111111101;
	  }
	
	  else if (source == &ir_long_front_right_input_pin_) {
	    ir_l_fr_.add(sensor_value);
	    x_ &= 0b1111111111011;
	  }
	  
	  // IR Short sensors
	  else if (source == &ir_short_front_center_input_pin_) {
	    ir_s_fc_.add(sensor_value);
	    x_ &= 0b1111111110111;
	  }
	  
	  else if (source == &ir_short_front_left_input_pin_) {
	    ir_s_fl_.add(sensor_value);
	    x_ &= 0b1111111101111;
	  }
	  
	  else if (source == &ir_short_front_right_input_pin_) {
	    ir_s_fr_.add(sensor_value);
	    x_ &= 0b1111111011111;
	  }
	  
	  else if (source == &ir_short_left_input_pin_) {
	    ir_s_l_.add(sensor_value);
	    x_ &= 0b1111110111111;
	  }
	  
	  else if (source == &ir_short_right_input_pin_) {
	    ir_s_r_.add(sensor_value);
	    x_ &= 0b1111101111111;
	  }
	  
	  else if (source == &ir_short_rear_center_input_pin_) {
	    ir_s_rc_.add(sensor_value);
	    x_ &= 0b1111011111111;
	  }
	  
	  else if (source == &us_front_left_input_pin_) {
	    us_f_l_.add(sensor_value);
	    x_ &= 0b1110111111111;
	  }
	  
	  else if (source == &us_front_right_input_pin_) {
	    us_f_r_.add(sensor_value);
	    x_ &= 0b1101111111111;
	  }
	  
	  else if (source == &us_rear_left_input_pin_) {
	    us_r_l_.add(sensor_value);
	    x_ &= 0b1011111111111;
	  }
	  
	  else if (source == &us_rear_right_input_pin_) {
	    us_r_r_.add(sensor_value);
	    x_ &= 0b0111111111111;
	  }
	  
	  if (!x_) {
	    if (GetPropertyBool("debug")) LOG_INFO("Transmit sensor package");
	    transmitSensorPackage(time_stamp);
	    x_ = 0b1111111111111;
	  } else {
	  
	  }
	}
	
	RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult SensorPackageFilter::transmitSensorPackage(tTimeStamp time) {
// -------------------------------------------------------------------------------------------------
  cObjectPtr<IMediaSample> media_sample;
  RETURN_IF_FAILED(AllocMediaSample((tVoid**)&media_sample));

  cObjectPtr<IMediaSerializer> serializer;
  sensor_package_description_->GetMediaSampleSerializer(&serializer);
  tInt size = serializer->GetDeserializedSize();

  RETURN_IF_FAILED(media_sample->AllocBuffer(size));
  
  float value_ir_l_fc = getMeanSensorValue(ir_l_fc_);
  float value_ir_l_fl = getMeanSensorValue(ir_l_fl_);
  float value_ir_l_fr = getMeanSensorValue(ir_l_fr_);
  float value_ir_s_fc = getMeanSensorValue(ir_s_fc_);
  float value_ir_s_fl = getMeanSensorValue(ir_s_fl_);
  float value_ir_s_fr = getMeanSensorValue(ir_s_fr_);
  float value_ir_s_l = getMeanSensorValue(ir_s_l_);
  float value_ir_s_r = getMeanSensorValue(ir_s_r_);
  float value_ir_s_rc = getMeanSensorValue(ir_s_rc_);
  float value_us_f_l = getMeanSensorValue(us_f_l_);
  float value_us_f_r = getMeanSensorValue(us_f_r_);
  float value_us_r_l = getMeanSensorValue(us_r_l_);
  float value_us_r_r = getMeanSensorValue(us_r_r_);
  {
    __adtf_sample_write_lock_mediadescription(sensor_package_description_, media_sample, coder);

    coder->Set("ir_l_fc", (tVoid*) &value_ir_l_fc);
    coder->Set("ir_l_fl", (tVoid*) &value_ir_l_fl);
    coder->Set("ir_l_fr", (tVoid*) &value_ir_l_fr);
    coder->Set("ir_s_fc", (tVoid*) &value_ir_s_fc);
    coder->Set("ir_s_fl", (tVoid*) &value_ir_s_fl);
    coder->Set("ir_s_fr", (tVoid*) &value_ir_s_fr);
    coder->Set("ir_s_l", (tVoid*) &value_ir_s_l);
    coder->Set("ir_s_r", (tVoid*) &value_ir_s_r);
    coder->Set("ir_s_rc", (tVoid*) &value_ir_s_rc);
    coder->Set("us_f_l", (tVoid*) &value_us_f_l);
    coder->Set("us_f_r", (tVoid*) &value_us_f_r);
    coder->Set("us_r_l", (tVoid*) &value_us_r_l);
    coder->Set("us_r_r", (tVoid*) &value_us_r_r);
  }      

  sensor_package_output_.Transmit(media_sample);
  
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
float SensorPackageFilter::getMeanSensorValue(CircularBuffer<float> & buffer) {
// -------------------------------------------------------------------------------------------------
  std::vector<float> values = buffer.getChronological();
  float sum = 0.0f;
  for (size_t i = 0; i < values.size(); i++) {
    sum += values[i];
  }
  
  return sum / (float)values.size();
}
