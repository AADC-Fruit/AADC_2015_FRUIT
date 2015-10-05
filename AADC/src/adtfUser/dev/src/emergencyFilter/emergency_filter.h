#ifndef _EMERGENCY_FILTER_H_
#define _EMERGENCY_FILTER_H_

#define OID_ADTF_EMERGENCY_FILTER  "fruit.dev.emergency_filter"

class EmergencyFilter : public adtf::cFilter {
  ADTF_FILTER(OID_ADTF_EMERGENCY_FILTER, "FRUIT Emergency Filter", adtf::OBJCAT_DataFilter);
 
 public:
 	EmergencyFilter(const tChar* __info);
 	tResult Init(tInitStage stage, __exception);
  tResult Shutdown(tInitStage stage, __exception);
	tResult OnPinEvent(IPin* source, tInt event_code, tInt param1, tInt param2, IMediaSample* media_sample);
 
 private:
  struct SensorData {
    float value_ir_l_fc;
    float value_ir_l_fl;
    float value_ir_l_fr;
    float value_ir_s_fc;
    float value_ir_s_fl;
    float value_ir_s_fr;
    float value_ir_s_l;
    float value_ir_s_r;
    float value_ir_s_rc;
    float value_us_f_l;
    float value_us_f_r;
    float value_us_r_l;
    float value_us_r_r;
  };
 
  tResult transmitBool(cOutputPin & pin, bool value);
  bool stopCarNow(SensorData const & data);
  
 	// Infrared input pins longrange
  cInputPin sensor_package_input_;
  cInputPin suppress_input_;
  
  // Sensor package output pin
  cOutputPin wheel_active_output_;
  cOutputPin hazard_light_output_;

  // Coder Descriptors
  cObjectPtr<IMediaTypeDescription> bool_data_description_;
  cObjectPtr<IMediaTypeDescription> suppression_data_description_;
  cObjectPtr<IMediaTypeDescription> sensor_package_description_;
  
  bool suppressed_;
  
  bool debug_;
};

#endif  // _EMERGENCY_FILTER_H_
