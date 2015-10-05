#ifndef _BOOL_SENDER_FILTER_H_
#define _BOOL_SENDER_FILTER_H_

#define OID_ADTF_BOOL_SENDER_FILTER "fruit.dev.bool_sender_filter"

class BoolSenderFilter : public adtf::cTimeTriggeredFilter {
  ADTF_FILTER(OID_ADTF_BOOL_SENDER_FILTER, "FRUIT Sender Filter Bool", adtf::OBJCAT_DataFilter);
  public:
    BoolSenderFilter(const tChar* __info);
    virtual ~BoolSenderFilter();
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

#endif // _BOOL_SENDER_FILTER_H_
