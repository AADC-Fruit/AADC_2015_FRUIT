#ifndef _SENSOR_PACKAGE_FILTER_H_
#define _SENSOR_PACKAGE_FILTER_H_

#define OID_ADTF_SENSOR_PACKAGE_FILTER  "fruit.dev.sensor_package_filter"

#include "../util/circularBuffer/circular_buffer.h"

class SensorPackageFilter : public adtf::cFilter {
  ADTF_FILTER(OID_ADTF_SENSOR_PACKAGE_FILTER, "FRUIT Sensor Package Filter", adtf::OBJCAT_DataFilter);
 
 public:
 	SensorPackageFilter(const tChar* __info);
 	
 	tResult Init(tInitStage stage, __exception);
  tResult Shutdown(tInitStage stage, __exception);
	tResult OnPinEvent(IPin* source, tInt event_code, tInt param1, tInt param2, IMediaSample* media_sample);
 
 private:
  tResult transmitSensorPackage(tTimeStamp time);
  float getMeanSensorValue(CircularBuffer<float> & buffer);
  
 	// Infrared input pins longrange
  cInputPin ir_long_front_center_input_pin_;
  cInputPin ir_long_front_left_input_pin_;
  cInputPin ir_long_front_right_input_pin_;
  
  // Infrared input pins shortrange
  cInputPin ir_short_front_center_input_pin_;
  cInputPin ir_short_front_left_input_pin_;
  cInputPin ir_short_front_right_input_pin_;
  cInputPin ir_short_left_input_pin_;
  cInputPin ir_short_right_input_pin_;
  cInputPin ir_short_rear_center_input_pin_;
  
  // Ultrasonic input pins
  cInputPin us_front_left_input_pin_;
  cInputPin us_front_right_input_pin_;
  cInputPin us_rear_left_input_pin_;
  cInputPin us_rear_right_input_pin_;
  
  // Sensor package output pin
  cOutputPin sensor_package_output_;

  // Coder Descriptors
  cObjectPtr<IMediaTypeDescription> sensor_data_description_;
  cObjectPtr<IMediaTypeDescription> sensor_package_description_;
  
  CircularBuffer<float> ir_l_fc_;
  CircularBuffer<float> ir_l_fl_;
  CircularBuffer<float> ir_l_fr_;
  CircularBuffer<float> ir_s_fc_;
  CircularBuffer<float> ir_s_fl_;
  CircularBuffer<float> ir_s_fr_;
  CircularBuffer<float> ir_s_l_;
  CircularBuffer<float> ir_s_r_;
  CircularBuffer<float> ir_s_rc_;
  CircularBuffer<float> us_f_l_;
  CircularBuffer<float> us_f_r_;
  CircularBuffer<float> us_r_l_;
  CircularBuffer<float> us_r_r_;
  
  int x_;
  
  static const size_t BUFFER_SIZE = 5;
  
  cCriticalSection critical_section_;
};

#endif  // _SENSOR_PACKAGE_FILTER_H_
