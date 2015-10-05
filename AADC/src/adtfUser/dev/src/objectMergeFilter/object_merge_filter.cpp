#include "stdafx.h"
#include "object_merge_filter.h"
#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>

ADTF_FILTER_PLUGIN("FRUIT Object Merge Filter", OID_ADTF_OBJECT_MERGE_FILTER, ObjectMergeFilter);

using namespace cv;

// -------------------------------------------------------------------------------------------------
ObjectMergeFilter::ObjectMergeFilter(const tChar* __info) {
// -------------------------------------------------------------------------------------------------
  cMemoryBlock::MemSet(&output_format_, 0, sizeof(output_format_));
  cMemoryBlock::MemSet(&input_format_, 0, sizeof(input_format_));
  
  current_objects_ = new Object[10];
}

// -------------------------------------------------------------------------------------------------
ObjectMergeFilter::~ObjectMergeFilter() {
// -------------------------------------------------------------------------------------------------

}

// -------------------------------------------------------------------------------------------------
tResult ObjectMergeFilter::Init(tInitStage stage, __exception) {
// -------------------------------------------------------------------------------------------------
  RETURN_IF_FAILED(cFilter::Init(stage, __exception_ptr));
  
	if (stage == StageFirst) {
		// Video input.
		RETURN_IF_FAILED(video_input_pin_.Create("Video_Input", IPin::PD_Input,
		  static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&video_input_pin_));
    
    // Video output
    RETURN_IF_FAILED(video_output_pin_.Create("Video_Output", IPin::PD_Output, NULL));
    RETURN_IF_FAILED(RegisterPin(&video_output_pin_));
    
    // Create the object input pin.
    cObjectPtr<IMediaDescriptionManager> description_manager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER,
      IID_ADTF_MEDIA_DESCRIPTION_MANAGER, (tVoid**) &description_manager, __exception_ptr));
      
    tChar const * stream_type = description_manager->GetMediaDescription("ObjectList");
    RETURN_IF_POINTER_NULL(stream_type);
    
    // Create the stream description used to receive/send data
    cObjectPtr<IMediaType> type_signal_value = new cMediaType(0, 0, 0, "ObjectList",
      stream_type, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(type_signal_value->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION,
      (tVoid**) &object_stream_description_));
    
    // Create and register the input pin
    RETURN_IF_FAILED(object_input_pin_.Create("objects", type_signal_value, static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&object_input_pin_));

    height_ = -1;
    width_ = -1;
	} else if (stage == StageGraphReady) {
	  cObjectPtr<IMediaType> type;
	  RETURN_IF_FAILED(video_input_pin_.GetMediaType(&type));
	  
	  cObjectPtr<IMediaTypeVideo> type_video;
	  RETURN_IF_FAILED(type->GetInterface(IID_ADTF_MEDIA_TYPE_VIDEO, (tVoid**) &type_video));
	  
	  const tBitmapFormat* format = type_video->GetFormat();
	  if (format == NULL) RETURN_ERROR(ERR_NOT_SUPPORTED);
	  
    cMemoryBlock::MemCopy(&input_format_, format, sizeof(tBitmapFormat));
	  cMemoryBlock::MemCopy(&output_format_, format, sizeof(tBitmapFormat));
	  
    video_output_pin_.SetFormat(&output_format_, NULL);
	}
	
	RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult ObjectMergeFilter::Shutdown(tInitStage stage, __exception) {
// -------------------------------------------------------------------------------------------------
	return cFilter::Shutdown(stage,__exception_ptr);
}

// -------------------------------------------------------------------------------------------------
tResult ObjectMergeFilter::OnPinEvent(IPin* source, tInt event_code, tInt param1, tInt param2,
    IMediaSample* media_sample) {
// -------------------------------------------------------------------------------------------------
	RETURN_IF_POINTER_NULL(source);
	RETURN_IF_POINTER_NULL(media_sample);
	
	if (event_code == IPinEventSink::PE_MediaSampleReceived) {
		if (source == &video_input_pin_) {
			if (height_ == -1 || width_ == -1) setHeightAndWidth();
			std::cout << "[OMF] Video input received!" << std::endl;
      processImage(media_sample);
		} else if (source == &object_input_pin_) {
		  std::cout << "[OMF] Object input received!" << std::endl;
		  // Read-out the incoming Media Sample
      cObjectPtr<IMediaCoder> pCoderInput;
      RETURN_IF_FAILED(object_stream_description_->Lock(media_sample, &pCoderInput));
                
      // Get values from media sample
      Object* signal_value = new Object[10];
      pCoderInput->Get("Objects", (tVoid*) signal_value);
      
      std::cout << "[OMF] First blob: " << signal_value[0].get_x() << ", ";
      std::cout << signal_value[0].get_y() << std::endl;
      
      std::cout << "[OMF] Preparing to memcpy..." << std::endl;
      memcpy(current_objects_, signal_value, sizeof(Object) * 10);
      std::cout << "[OMF] Memcpy done!" << std::endl;
      
      // Unlock the sample again
      object_stream_description_->Unlock(pCoderInput);
		}
	}
		
	RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult ObjectMergeFilter::processImage(IMediaSample* sample) {
// -------------------------------------------------------------------------------------------------
  RETURN_IF_POINTER_NULL(sample);
  
  cObjectPtr<IMediaSample> image_sample, object_sample;
  RETURN_IF_FAILED(_runtime->CreateInstance(OID_ADTF_MEDIA_SAMPLE, IID_ADTF_MEDIA_SAMPLE, (tVoid**) &image_sample));
  
  RETURN_IF_FAILED(image_sample->AllocBuffer(output_format_.nSize));
  AllocMediaSample((tVoid**)&object_sample);
  cObjectPtr<IMediaSerializer> pSerializer;
  object_stream_description_->GetMediaSampleSerializer(&pSerializer);
  tInt nSize = pSerializer->GetDeserializedSize();
  object_sample->AllocBuffer(nSize);
  
  const tVoid* source_buffer;
  tVoid* dest_buffer;
  
  if (IS_OK(sample->Lock(&source_buffer))) {
    if (IS_OK(image_sample->WriteLock(&dest_buffer))) {
      std::cout << "[OMF] Processing video input!" << std::endl;
      // Do image processing!
      // Mem-copy von source_buffer auf dest_buffer
      memcpy((uchar*)dest_buffer, (uchar*)source_buffer, 3 * height_ * width_);
      Mat current_image(height_, width_, CV_8UC3, (uchar*)source_buffer);
      Mat object_image(height_, width_, CV_8UC3, (uchar*)dest_buffer);
      
      // Draw the object boundaries into the image
      for (int i = 0; i < 10; i++) {
        Object cur = current_objects_[i];
        if (cur.get_x() != -999999) {
          int blue = (int)((double)rand()/RAND_MAX * 255.0);
          int green = (int)((double)rand()/RAND_MAX * 255.0);
          int red = (int)((double)rand()/RAND_MAX * 255.0);
          rectangle(object_image, Rect(cur.get_x(), cur.get_y(), cur.get_width(), cur.get_height()), Scalar(blue, green, red), 2);
        }
      }
      
      image_sample->Unlock(dest_buffer);
    }
    image_sample->SetTime(sample->GetTime());
    sample->Unlock(source_buffer);
  }
  
  std::cout << "[OMF] Transmitting video input!" << std::endl;
  RETURN_IF_FAILED(video_output_pin_.Transmit(image_sample));
  
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult ObjectMergeFilter::setHeightAndWidth() {
// -------------------------------------------------------------------------------------------------
	// Retrieve the input type
	cObjectPtr<IMediaType> type;
	RETURN_IF_FAILED(video_input_pin_.GetMediaType(&type));
	
	// Retrieve the video type based on the input type
	cObjectPtr<IMediaTypeVideo> video_type;
	RETURN_IF_FAILED(type->GetInterface(IID_ADTF_MEDIA_TYPE_VIDEO, (tVoid**)&video_type));
	
	// Retrieve height and width values from the video type
	height_ = video_type->GetFormat()->nHeight;
	width_ = video_type->GetFormat()->nWidth;
	
	RETURN_NOERROR;
}
