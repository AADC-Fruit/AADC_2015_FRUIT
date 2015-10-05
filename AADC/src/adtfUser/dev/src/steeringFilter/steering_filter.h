#ifndef _STEERING_FILTER_H_
#define _STEERING_FILTER_H_

#define OID_ADTF_STEERING_FILTER "fruit.dev.steering_filter"

class SteeringFilter : public adtf::cFilter {
 ADTF_FILTER(OID_ADTF_STEERING_FILTER, "Steering Filter", adtf::OBJCAT_DataFilter);

 public:
  SteeringFilter(const tChar* __info);
  virtual ~SteeringFilter();

 protected:
  // Input pin
  cInputPin steering_input_;
  cOutputPin steering_output_;

  // Coder Descriptor for the input pins
  cObjectPtr<IMediaTypeDescription> stream_description_;

  // Filter methods
  tResult Init(tInitStage eStage, __exception);
  tResult Shutdown(tInitStage eStage, __exception);

  // Implements IPinEventSink
  tResult OnPinEvent(
    IPin* pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample* pMediaSample);
  
 private:
  void processSteeringCommand(tFloat32 value);
  void transmitSteeringAngle(tFloat32 value);
  
  float old_angle_;
};

#endif // _STEERING_FILTER_H_
