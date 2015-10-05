#ifndef _IMAGE_CROP_FILTER_H_
#define _IMAGE_CROP_FILTER_H_

#define OID_ADTF_IMAGE_CROP_FILTER  "fruit.dev.image_crop_filter"
 
using namespace cv;

class ImageCropFilter : public adtf::cFilter {
  ADTF_FILTER(OID_ADTF_IMAGE_CROP_FILTER, "FRUIT Image Crop Filter", adtf::OBJCAT_DataFilter);
 
 public:
 	ImageCropFilter(const tChar* __info);
 	virtual ~ImageCropFilter();
 	
 	tResult Init(tInitStage stage, __exception);
  tResult Shutdown(tInitStage stage, __exception);
	tResult OnPinEvent(IPin* source, tInt event_code, tInt param1, tInt param2,
    IMediaSample* media_sample);
 
 protected:
 	cVideoPin video_input_pin_;
 	tBitmapFormat input_format_;
 	
 	cVideoPin video_output_pin_;
 	tBitmapFormat output_format_;
 	Rect region_of_interest_;
 	int x_, y_, height_, width_;
};

#endif  // _IMAGE_CROP_FILTER_H_
