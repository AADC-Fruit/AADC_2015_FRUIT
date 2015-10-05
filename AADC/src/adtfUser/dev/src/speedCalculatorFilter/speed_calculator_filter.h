#ifndef _SPEED_DISTANCE_CALCULATOR_H_
#define _SPEED_DISTANCE_CALCULATOR_H_

#define OID_ADTF_SPEED_CALCULATOR_FILTER "fruit.dev.speed_calculator_filter"

/*!
* This filter calculates the RPM of the wheels and how much they moved on the ground.
*/
class SpeedCalculatorFilter : public adtf::cFilter {
  ADTF_FILTER(OID_ADTF_SPEED_CALCULATOR_FILTER, "FRUIT Speed Calculator Filter", adtf::OBJCAT_DataFilter);

 public:
  SpeedCalculatorFilter(const tChar* __info);
  virtual ~SpeedCalculatorFilter();
    
 protected:
  tResult Init(tInitStage eStage, __exception = NULL);        
  tResult OnPinEvent(IPin* pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample* pMediaSample);
    
 private:
  tTimeStamp GetTime();
  tResult transmitSpeed(cOutputPin & pin, float value, tTimeStamp time_stamp);
  void calculateSpeed(tFloat32 counter_value, tFloat32 last_counter_value,tTimeStamp current_timestamp, tTimeStamp last_timestamp);
  
  tFloat32 left_speed_;
  tFloat32 right_speed_;
  bool left_;
  int motor_direction_; // Positive values indicate forward driving, negative backward. If 0, car stands still -> ignore odometry measurements
  static const tFloat32 THRESHOLD = 20.0;
  
  cInputPin input_counter_left_;
  cInputPin input_counter_right_;
  cInputPin input_motor_direction_;
  cOutputPin output_speed_left_;
  cOutputPin output_speed_right_;
  
  cSlidingWindow sliding_window_left_;
  cSlidingWindow sliding_window_right_;
  
  cObjectPtr<IMediaTypeDescription> signal_description_input_;
  cObjectPtr<IMediaTypeDescription> signal_description_output_;
  cObjectPtr<IMediaTypeDescription> signal_description_motor_;
};

//*************************************************************************************************

#endif // _SPEED_DISTANCE_CALCULATOR_H_

