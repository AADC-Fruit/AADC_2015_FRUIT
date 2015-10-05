#ifndef _WHEEL_LOCK_FILTER_H_
#define _WHEEL_LOCK_FILTER_H_

#define OID_ADTF_WHEEL_LOCK_FILTER "fruit.dev.wheel_lock_filter"

class WheelLockFilter : public adtf::cTimeTriggeredFilter {
ADTF_FILTER(OID_ADTF_WHEEL_LOCK_FILTER, "FRUIT Wheel Lock Filter", adtf::OBJCAT_DataFilter);

 public:
  WheelLockFilter(const tChar* __info);
  virtual ~WheelLockFilter();

 protected:
  tResult Init(tInitStage eStage, __exception = NULL);        
  tResult OnPinEvent(IPin* pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample* pMediaSample);
  tResult Cycle(__exception=NULL);
  tResult Shutdown(tInitStage eStage, __exception);

 private:
  tResult sendSignalValue(cOutputPin & pin, float value);
  tResult sendBrakeLightSignal(bool value);
  tResult sendReverseLightSignal(bool value);
  tResult sendRunningSignal(bool value);
  
  void setCurrentAccel(float value);
  void setCurrentSteering(float value);
 
  cInputPin steering_input_;
  cInputPin motor_input_;
  cInputPin jury_flag_input_;
  cInputPin emergency_flag_input_;
  cOutputPin steering_output_;
  cOutputPin motor_output_;
  cOutputPin brake_light_output_;
  cOutputPin reverse_light_output_;
  cOutputPin driving_output_;
  
  cObjectPtr<IMediaTypeDescription> signal_description_;
  cObjectPtr<IMediaTypeDescription> jury_flag_description_;
  cObjectPtr<IMediaTypeDescription> emergency_flag_description_;
  cObjectPtr<IMediaTypeDescription> flag_description_output_;
  cObjectPtr<IMediaTypeDescription> signal_description_output_;
  
  bool jury_flag_;
  bool emergency_flag_;
  bool was_running_;
  int brake_light_counter_;
  
  bool debug_;
  
  float current_accel_value_;
  float current_steering_value_;
  size_t accel_counter_;
  size_t steering_counter_;
  bool current_brake_light_value_;
  bool current_reverse_light_value_;
  bool driving_status_;
  bool driving_status_changed_;
  
  cCriticalSection critical_section_;
  tTimeStamp last_time_stamp_;
};

#endif // _LIGHT_MACHINE_CALCULATOR_H_

