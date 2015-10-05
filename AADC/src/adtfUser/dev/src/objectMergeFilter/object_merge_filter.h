#ifndef _OBJECT_MERGE_FILTER_H_
#define _OBJECT_MERGE_FILTER_H_

#define OID_ADTF_OBJECT_MERGE_FILTER  "fruit.dev.object_merge_filter"

#include "../util/object.h"

class ObjectMergeFilter : public adtf::cFilter {
  ADTF_FILTER(OID_ADTF_OBJECT_MERGE_FILTER, "FRUIT Object Merge Filter", adtf::OBJCAT_DataFilter);
 
 public:
 	ObjectMergeFilter(const tChar* __info);
 	virtual ~ObjectMergeFilter();
 	
 	tResult Init(tInitStage stage, __exception);
  tResult Shutdown(tInitStage stage, __exception);
	tResult OnPinEvent(IPin* source, tInt event_code, tInt param1, tInt param2,
    IMediaSample* media_sample);
 
 protected:
  // Input pin
 	cVideoPin video_input_pin_;
 	tBitmapFormat input_format_;
 	
 	cInputPin object_input_pin_;
 	cObjectPtr<IMediaTypeDescription> object_stream_description_;
 	
 	// Output pin
 	cVideoPin video_output_pin_;
 	tBitmapFormat output_format_;
 	
 private:
  tResult setHeightAndWidth();
  tResult processImage(IMediaSample* sample);
  
  Object* current_objects_;
  int height_, width_;
};

#endif  // _OBJECT_MERGE_FILTER_H_
