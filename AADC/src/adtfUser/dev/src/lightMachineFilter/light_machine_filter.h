#ifndef _LIGHT_MACHINE_FILTER_H_
#define _LIGHT_MACHINE_FILTER_H_

#define OID_ADTF_LIGHT_MACHINE_FILTER "fruit.dev.light_machine_filter"

class LightMachineFilter : public adtf::cTimeTriggeredFilter {
ADTF_FILTER(OID_ADTF_LIGHT_MACHINE_FILTER, "FRUIT Light Machine Filter", adtf::OBJCAT_DataFilter);

 public:
  LightMachineFilter(const tChar* __info);
  virtual ~LightMachineFilter();

 protected:
  tResult Init(tInitStage eStage, __exception = NULL);        
  tResult OnPinEvent(IPin* pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample* media_sample);
  tResult Cycle(__exception=NULL);
  tResult Shutdown(tInitStage eStage, __exception);

 private:
  tResult sendLightSignal(cOutputPin & pin, bool value);
 
  cInputPin headlights_input_;
  cInputPin brakelights_input_;
  cInputPin reverselights_input_;
  cInputPin turn_right_input_;
  cInputPin turn_left_input_;
  cInputPin hazardlights_input_;
  
  cOutputPin headlights_output_;
  cOutputPin brakelights_output_;
  cOutputPin reverselights_output_;
  cOutputPin turn_right_output_;
  cOutputPin turn_left_output_;
  cOutputPin hazardlights_output_;
  
  cObjectPtr<IMediaTypeDescription> bool_description_;
  cObjectPtr<IMediaTypeDescription> bool_description_output_;
 
  bool headlights_on_;
  bool brakelights_on_;
  bool reverselights_on_;
  bool turn_right_on_;
  bool turn_left_on_;
  bool hazardlights_on_;
  
  size_t headlights_counter_;
  size_t brakelights_counter_;
  size_t reverselights_counter_;
  size_t turn_right_counter_;
  size_t turn_left_counter_;
  size_t hazardlights_counter_;
  
  void setHeadlightsValue(bool value);
  void setBrakelightsValue(bool value);
  void setReverselightsValue(bool value);
  void setTurnRightValue(bool value);
  void setTurnLeftValue(bool value);
  void setHazardlightsValue(bool value);
    
};

#endif // _LIGHT_MACHINE_CALCULATOR_H_

