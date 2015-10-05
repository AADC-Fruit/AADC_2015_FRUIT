#ifndef _MOTOR_FILTER_H_
#define _MOTOR_FILTER_H_

#define OID_ADTF_MOTOR_FILTER "fruit.dev.motor_filter"

class MotorFilter : public adtf::cFilter {
  ADTF_FILTER(OID_ADTF_MOTOR_FILTER, "Motor Filter", adtf::OBJCAT_DataFilter);
  public:
    MotorFilter(const tChar* __info);
    virtual ~MotorFilter();

  protected:
    // Pins
    cInputPin speed_input_;    
    cOutputPin speed_output_;

    // Coder Descriptor for the input pins
    cObjectPtr<IMediaTypeDescription> stream_description_;

    // Filter methods
    tResult Init(tInitStage eStage, __exception);
    tResult Shutdown(tInitStage eStage, __exception);

    // Implements IPinEventSink
    tResult OnPinEvent(
      IPin* pSource, tInt nEventCode, tInt nParam1, tInt nParam2, IMediaSample* pMediaSample);
    
  private:
    void processSpeed(tFloat32 value);
    void transmit(tFloat32 value);
};

#endif // _MOTOR_FILTER_H_
