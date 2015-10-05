#ifndef _LANE_FILTER_H_
#define _LANE_FILTER_H_

#define OID_ADTF_LANE_FILTER  "fruit.dev.lane_filter"

#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "lane_preprocessor.h"
#include "lane_detector.h"

using namespace cv;

class LaneFilter : public adtf::cFilter {
  ADTF_FILTER(OID_ADTF_LANE_FILTER, "FRUIT Lane Filter", adtf::OBJCAT_DataFilter);
 
 public:
 	LaneFilter(const tChar* __info);
 	virtual ~LaneFilter();
 	
 	tResult Init(tInitStage stage, __exception);
  tResult Shutdown(tInitStage stage, __exception);
	tResult OnPinEvent(IPin* source, tInt event_code, tInt param1, tInt param2, IMediaSample* media_sample);
 
 protected:
  // RGB video input pin
 	cVideoPin video_input_pin_;
 	tBitmapFormat input_format_;
 	
 	// Object input pin
 	cInputPin object_input_pin_;
 	cObjectPtr<IMediaTypeDescription> object_data_description_;
 	
 	// lane points output pin
 	cOutputPin lane_output_pin_;
 	cOutputPin light_output_;
  cObjectPtr<IMediaTypeDescription> lane_data_description_;
  cObjectPtr<IMediaTypeDescription> light_data_description_;
 	
 private:
  tResult processImage(IMediaSample* sample);
  tResult transmitLanePoints(std::vector<Vector2> const & mapped_points);
  tResult transmitHeadLight(bool value);
  float calculateImageLuminosity(Mat const & image);
  void annotateImage(Mat & image);
  
 	LaneDetector2 lane_detector_;
 	LanePreprocessor lane_preprocessor_;
 	
 	bool headlights_on_;
};

#endif  // _LANE_FILTER_H_
