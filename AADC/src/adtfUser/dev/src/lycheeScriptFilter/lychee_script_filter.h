#ifndef _LYCHEE_SCRIPT_FILTER_H_
#define _LYCHEE_SCRIPT_FILTER_H_

#define OID_ADTF_LYCHEE_SCRIPT_FILTER "fruit.dev.lychee_script_filter"

#include <list>
#include <utility>
#include <string>
 
class LycheeScriptFilter : public adtf::cTimeTriggeredFilter {
  ADTF_FILTER(OID_ADTF_LYCHEE_SCRIPT_FILTER, "FRUIT Lychee Script Filter", adtf::OBJCAT_DataFilter);
  
  public:
    LycheeScriptFilter(const tChar* __info);
    virtual ~LycheeScriptFilter();
    tResult Cycle(__exception=NULL);
    tResult OnPinEvent(IPin* source, tInt event_code, tInt param1, tInt param2, IMediaSample* media_sample);
    
  protected:
    enum ScriptType {
      DELAY,
      SPEED,
      ANGLE,
      RSIGN,
      LSIGN,
      HAZZA
    };
    
    // Filter methods
    tResult Init(tInitStage eStage, __exception);
    tResult Shutdown(tInitStage eStage, __exception);
    void TransmitInactiveFlag(tTimeStamp time_stamp);
    void TransmitSpeed(tTimeStamp time_stamp, float speed);
    void TransmitAngle(tTimeStamp time_stamp, float angle);
    tResult TransmitLight(cOutputPin & pin, bool value);

    void parseScript(std::string const & filepath);
    
    // Input pins
    cInputPin active_flag_input_pin_;
    cInputPin wheel_lock_active_flag_input_pin_;
    cInputPin script_input_pin_;
    
    // Output pins
    cOutputPin steering_angle_output_pin_;
    cOutputPin speed_output_pin_;
    cOutputPin inactive_flag_output_pin_;
    cOutputPin turn_left_output_;
    cOutputPin turn_right_output_;
    cOutputPin hazardlights_output_;

    // Coder Descriptor for the pins
    cObjectPtr<IMediaTypeDescription> bool_stream_description_;
    cObjectPtr<IMediaTypeDescription> wheel_lock_bool_stream_description_;
    cObjectPtr<IMediaTypeDescription> signal_stream_description_;
    cObjectPtr<IMediaTypeDescription> string_stream_description_;
    
    size_t timer_current_;
    size_t timer_goal_;
    bool active_;
    bool wheel_lock_active_;
    std::list<std::pair<ScriptType, float> > script_list_;
    
    static const int INTERVAL = 125;
};

#endif // _LYCHEE_SCRIPT_FILTER_H_
