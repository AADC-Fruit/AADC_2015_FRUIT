#include "stdafx.h"
#include "emergency_filter.h"

ADTF_FILTER_PLUGIN("FRUIT Emergency Filter", OID_ADTF_EMERGENCY_FILTER, EmergencyFilter);

// -------------------------------------------------------------------------------------------------
EmergencyFilter::EmergencyFilter(const tChar* __info) : adtf::cFilter(__info) {
// -------------------------------------------------------------------------------------------------
  SetPropertyBool("debug", false);
  SetPropertyBool("Send hazard lights", true);
  SetPropertyInt("Front distance threshold", 50);
}

// -------------------------------------------------------------------------------------------------
tResult EmergencyFilter::Init(tInitStage stage, __exception) {
// -------------------------------------------------------------------------------------------------
  RETURN_IF_FAILED(cFilter::Init(stage, __exception_ptr));
  
	if (stage == StageFirst) {
	  // Create and register the input pin
    cObjectPtr<IMediaDescriptionManager> description_manager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER,
      IID_ADTF_MEDIA_DESCRIPTION_MANAGER,(tVoid**)&description_manager,__exception_ptr));
    
    // Create the output pin description
    tChar const * package_description = description_manager->GetMediaDescription("SensorPackage");
    RETURN_IF_POINTER_NULL(package_description);        
    cObjectPtr<IMediaType> package_type = new cMediaType(0, 0, 0, "SensorPackage", package_description, IMediaDescription::MDF_DDL_DEFAULT_VERSION);	
    RETURN_IF_FAILED(package_type->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&sensor_package_description_));
    
    // Create the input pin
    RETURN_IF_FAILED(sensor_package_input_.Create("sensorPackage", package_type, static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&sensor_package_input_));
    
    // Create the suppression input description
    tChar const * suppression_description = description_manager->GetMediaDescription("tBoolSignalValue");
    RETURN_IF_POINTER_NULL(suppression_description);        
    cObjectPtr<IMediaType> suppression_type = new cMediaType(0, 0, 0, "tBoolSignalValue", suppression_description, IMediaDescription::MDF_DDL_DEFAULT_VERSION);	
		RETURN_IF_FAILED(suppression_type->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&suppression_data_description_));
		
		// Create the input pin
    RETURN_IF_FAILED(suppress_input_.Create("suppress_warnings", suppression_type, static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&suppress_input_));
    
    // Create the bool output description
    tChar const * bool_description = description_manager->GetMediaDescription("tBoolSignalValue");
    RETURN_IF_POINTER_NULL(bool_description);        
    cObjectPtr<IMediaType> bool_type = new cMediaType(0, 0, 0, "tBoolSignalValue", bool_description, IMediaDescription::MDF_DDL_DEFAULT_VERSION);	
		RETURN_IF_FAILED(bool_type->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&bool_data_description_));
    
    // Create the output pins
    RETURN_IF_FAILED(wheel_active_output_.Create("wheelActive", bool_type, NULL));
    RETURN_IF_FAILED(RegisterPin(&wheel_active_output_));
    
    RETURN_IF_FAILED(hazard_light_output_.Create("hazardLightsEnabled", bool_type, NULL));
    RETURN_IF_FAILED(RegisterPin(&hazard_light_output_));
	}
	
	else if (stage == StageGraphReady) {
	  debug_ = GetPropertyBool("debug");
	  
	  suppressed_ = false;
	}
	
	RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult EmergencyFilter::Shutdown(tInitStage stage, __exception) {
// -------------------------------------------------------------------------------------------------
	return cFilter::Shutdown(stage,__exception_ptr);
}

// -------------------------------------------------------------------------------------------------
tResult EmergencyFilter::OnPinEvent(IPin* source, tInt event_code, tInt param1, tInt param2,
    IMediaSample* media_sample) {
// -------------------------------------------------------------------------------------------------
	RETURN_IF_POINTER_NULL(source);
	RETURN_IF_POINTER_NULL(media_sample);
	
	if (event_code == IPinEventSink::PE_MediaSampleReceived) {
	  // Retrieve the sensor values from the sensor package media sample
	  if (source == &sensor_package_input_) {
	    SensorData data;

      {
        __adtf_sample_read_lock_mediadescription(sensor_package_description_, media_sample, coder);
        coder->Get("ir_l_fc", (tVoid*) &(data.value_ir_l_fc));
        coder->Get("ir_l_fl", (tVoid*) &(data.value_ir_l_fl));
        coder->Get("ir_l_fr", (tVoid*) &(data.value_ir_l_fr));
        coder->Get("ir_s_fc", (tVoid*) &(data.value_ir_s_fc));
        coder->Get("ir_s_fl", (tVoid*) &(data.value_ir_s_fl));
        coder->Get("ir_s_fr", (tVoid*) &(data.value_ir_s_fr));
        coder->Get("ir_s_l", (tVoid*) &(data.value_ir_s_l));
        coder->Get("ir_s_r", (tVoid*) &(data.value_ir_s_r));
        coder->Get("ir_s_rc", (tVoid*) &(data.value_ir_s_rc));
        coder->Get("us_f_l", (tVoid*) &(data.value_us_f_l));
        coder->Get("us_f_r", (tVoid*) &(data.value_us_f_r));
        coder->Get("us_r_l", (tVoid*) &(data.value_us_r_l));
        coder->Get("us_r_r", (tVoid*) &(data.value_us_r_r));
      }
      
	    // Check if the car needs to stop immediately
	    if (stopCarNow(data) && !suppressed_) {
	      if (debug_) LOG_INFO("Enable emergency stopping");
	      transmitBool(wheel_active_output_, false);
	      transmitBool(hazard_light_output_, true);
	    } else if (!suppressed_) {
	      if (debug_) LOG_INFO("Disable emergency stopping");
	      transmitBool(wheel_active_output_, true);
	      transmitBool(hazard_light_output_, false);
	    }
	  } else if (source == &suppress_input_) {
      {
        __adtf_sample_read_lock_mediadescription(suppression_data_description_, media_sample, coder);
        coder->Get("bValue", (tVoid*)&suppressed_);
      }
      
      if(suppressed_) {
        transmitBool(wheel_active_output_, true);
      }
	  }
	}
	
	RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
bool EmergencyFilter::stopCarNow(SensorData const & data) {
// -------------------------------------------------------------------------------------------------
  float front_threshold = GetPropertyInt("Front distance threshold");
  if (data.value_ir_l_fc < front_threshold) return true;
  
  return false;
}

// -------------------------------------------------------------------------------------------------
tResult EmergencyFilter::transmitBool(cOutputPin & pin, bool value) {
// -------------------------------------------------------------------------------------------------
  cObjectPtr<IMediaSample> media_sample;
  RETURN_IF_FAILED(AllocMediaSample((tVoid**)&media_sample));

  cObjectPtr<IMediaSerializer> serializer;
  bool_data_description_->GetMediaSampleSerializer(&serializer);
  tInt size = serializer->GetDeserializedSize();

  RETURN_IF_FAILED(media_sample->AllocBuffer(size));
  tUInt32 timeStamp = 0;
  
  {
    __adtf_sample_write_lock_mediadescription(bool_data_description_, media_sample, coder);
    coder->Set("bValue", (tVoid*) &value);
    coder->Set("ui32ArduinoTimestamp", (tVoid*) &timeStamp);
  }

  pin.Transmit(media_sample);
  RETURN_NOERROR;
}
