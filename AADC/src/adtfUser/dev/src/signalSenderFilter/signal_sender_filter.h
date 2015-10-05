#ifndef _SIGNAL_SENDER_FILTER_H_
#define _SIGNAL_SENDER_FILTER_H_

#define OID_ADTF_SIGNAL_SENDER_FILTER "fruit.dev.signal_sender_filter"

class SignalSenderFilter : public adtf::cTimeTriggeredFilter {
  ADTF_FILTER(OID_ADTF_SIGNAL_SENDER_FILTER, "FRUIT Sender Filter Signal", adtf::OBJCAT_DataFilter);
  public:
    SignalSenderFilter(const tChar* __info);
    virtual ~SignalSenderFilter();
    tResult Cycle(__exception=NULL);

  private:
    // Filter methods
    tResult Init(tInitStage eStage, __exception);
    tResult Shutdown(tInitStage eStage, __exception);
    
    // Input pin
    cOutputPin data_;

    // Coder Descriptor for the input pins
    cObjectPtr<IMediaTypeDescription> output_stream_description_;

    int counter_;
    bool debug_;
};

#endif // _SIGNAL_SENDER_FILTER_H_
