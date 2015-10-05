#ifndef _CROSSROAD_SCRIPT_FILTER_H_
#define _CROSSROAD_SCRIPT_FILTER_H_

#define OID_ADTF_CROSSROAD_SCRIPT_FILTER "fruit.dev.crossroad_script_filter"
 
class CrossroadScriptFilter : public adtf::cTimeTriggeredFilter {
  ADTF_FILTER(OID_ADTF_CROSSROAD_SCRIPT_FILTER, "FRUIT Crossroad Script Filter", adtf::OBJCAT_DataFilter);
  
  public:
    CrossroadScriptFilter(const tChar* __info);
    virtual ~CrossroadScriptFilter();
    tResult Cycle(__exception=NULL);
    tResult OnPinEvent(IPin* source, tInt event_code, tInt param1, tInt param2, IMediaSample* media_sample);
    
  protected:
  
    void TransmitInactiveFlag();
    void TransmitSpeed(float speed);
    void TransmitAngle(float angle);
  
    // Input pins
    cInputPin active_flag_input_pin_;
    cInputPin gyro_yaw_rad_input_pin_;
    
    // Output pins
    cOutputPin steering_angle_output_pin_;
    cOutputPin speed_output_pin_;
    cOutputPin inactive_flag_output_pin_;

    // Coder Descriptor for the pins
    cObjectPtr<IMediaTypeDescription> bool_stream_description_;
    cObjectPtr<IMediaTypeDescription> output_stream_description_;

    // Filter methods
    tResult Init(tInitStage eStage, __exception);
    tResult Shutdown(tInitStage eStage, __exception);
    int counter_;
    bool active_;
};

#endif // _CROSSROAD_SCRIPT_FILTER_H_
