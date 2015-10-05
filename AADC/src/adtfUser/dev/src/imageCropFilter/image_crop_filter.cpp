#include "stdafx.h"
#include "image_crop_filter.h"
#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>

ADTF_FILTER_PLUGIN("FRUIT Image Crop Filter", OID_ADTF_IMAGE_CROP_FILTER, ImageCropFilter);
 
// -------------------------------------------------------------------------------------------------
ImageCropFilter::ImageCropFilter(const tChar* __info) {
// -------------------------------------------------------------------------------------------------
  cMemoryBlock::MemSet(&input_format_, 0, sizeof(input_format_));
  cMemoryBlock::MemSet(&output_format_, 0, sizeof(output_format_));
  
  SetPropertyInt("y", 165);
  SetPropertyInt("x", 0);
  SetPropertyInt("height", 100);
  SetPropertyInt("width", 400);
}

// -------------------------------------------------------------------------------------------------
ImageCropFilter::~ImageCropFilter() {
// -------------------------------------------------------------------------------------------------

}

// -------------------------------------------------------------------------------------------------
tResult ImageCropFilter::Init(tInitStage stage, __exception) {
// -------------------------------------------------------------------------------------------------
  RETURN_IF_FAILED(cFilter::Init(stage, __exception_ptr));
  
	if (stage == StageFirst) {
	  x_ = y_ = height_ = width_ = 0;
		// Create and register the video input pin
		RETURN_IF_FAILED(video_input_pin_.Create("Video_Input", IPin::PD_Input,
		  static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&video_input_pin_));
    
    // Create and register the video output pin
    RETURN_IF_FAILED(video_output_pin_.Create("Video_Output", IPin::PD_Output, NULL));
    RETURN_IF_FAILED(RegisterPin(&video_output_pin_));
	}
	
	else if (stage == StageGraphReady) {
	  y_ = GetPropertyInt("y");
	  x_ = GetPropertyInt("x");
	  height_ = GetPropertyInt("height");
	  width_ = GetPropertyInt("width");
	  region_of_interest_ = Rect(x_, y_, width_, height_);
	  
	  cObjectPtr<IMediaType> type;
	  RETURN_IF_FAILED(video_input_pin_.GetMediaType(&type));
	  
	  cObjectPtr<IMediaTypeVideo> type_video;
	  RETURN_IF_FAILED(type->GetInterface(IID_ADTF_MEDIA_TYPE_VIDEO, (tVoid**) &type_video));
	  
	  const tBitmapFormat* format = type_video->GetFormat();
	  tBitmapFormat output_format;
	  output_format.nWidth = width_;
	  output_format.nHeight = height_;
	  output_format.nBitsPerPixel = 24;
	  output_format.nPixelFormat = 45;
	  output_format.nBytesPerLine = width_ * 3;
	  output_format.nSize = height_ * width_ * 3;
	  output_format.nPaletteSize = 0;
	  if (format == NULL) RETURN_ERROR(ERR_NOT_SUPPORTED);
	  
    cMemoryBlock::MemCopy(&input_format_, format, sizeof(tBitmapFormat));
	  cMemoryBlock::MemCopy(&output_format_, &output_format, sizeof(tBitmapFormat));
	  
    video_output_pin_.SetFormat(&output_format_, NULL);
	}
	
	RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult ImageCropFilter::Shutdown(tInitStage stage, __exception) {
// -------------------------------------------------------------------------------------------------
	return cFilter::Shutdown(stage,__exception_ptr);
}

// -------------------------------------------------------------------------------------------------
tResult ImageCropFilter::OnPinEvent(IPin* source, tInt event_code, tInt param1, tInt param2,
    IMediaSample* media_sample) {
// -------------------------------------------------------------------------------------------------
	RETURN_IF_POINTER_NULL(source);
	RETURN_IF_POINTER_NULL(media_sample);
	
	if (event_code == IPinEventSink::PE_MediaSampleReceived) {
		if (source == &video_input_pin_) {
      cObjectPtr<IMediaSample> image_sample;
      RETURN_IF_FAILED(_runtime->CreateInstance(OID_ADTF_MEDIA_SAMPLE, IID_ADTF_MEDIA_SAMPLE, (tVoid**) &image_sample));
      RETURN_IF_FAILED(image_sample->AllocBuffer(output_format_.nSize));
      
      const tVoid* source_buffer;
      tVoid* dest_buffer;
      
      if (IS_OK(media_sample->Lock(&source_buffer))) {
        if (IS_OK(image_sample->WriteLock(&dest_buffer))) {
          int source_width = input_format_.nWidth;
          int source_height = input_format_.nHeight;
          
          
          // Create the source image matrix    
          Mat source_image(source_height, source_width, CV_8UC3, (Vec3b*)source_buffer);
          Mat destination_image(height_, width_, CV_8UC3, (Vec3b*)dest_buffer);
          Mat roi_image = source_image.clone();
          roi_image = roi_image(region_of_interest_);
          for (int i = 0; i < height_; i++) {
            for (int j = 0; j < width_; j++) {
              destination_image.at<Vec3b>(i,j) = roi_image.at<Vec3b>(i,j);
            }
          }
          image_sample->Unlock(dest_buffer);  
        }
        media_sample->Unlock(source_buffer);
        image_sample->SetTime(media_sample->GetTime());
        video_output_pin_.Transmit(image_sample);
      }
		}
	}
	RETURN_NOERROR;
}
