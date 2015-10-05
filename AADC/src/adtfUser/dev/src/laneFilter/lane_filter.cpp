#include "stdafx.h"
#include "lane_filter.h"
#include "../util/l_point.h"
#include "../util/line_triple.h"

ADTF_FILTER_PLUGIN("FRUIT Lane Filter", OID_ADTF_LANE_FILTER, LaneFilter);

// -------------------------------------------------------------------------------------------------
LaneFilter::LaneFilter(const tChar* __info) {
// -------------------------------------------------------------------------------------------------
  cMemoryBlock::MemSet(&input_format_, 0, sizeof(input_format_));
  
  SetPropertyBool("debug", false);
  SetPropertyBool("headlights_enabled", true);
  SetPropertyInt("scan_rate", 50);
  SetPropertyFloat("lum_threshold", 100);
}

// -------------------------------------------------------------------------------------------------
LaneFilter::~LaneFilter() {
// -------------------------------------------------------------------------------------------------

}

// -------------------------------------------------------------------------------------------------
tResult LaneFilter::Init(tInitStage stage, __exception) {
// -------------------------------------------------------------------------------------------------
  RETURN_IF_FAILED(cFilter::Init(stage, __exception_ptr));
  
	if (stage == StageFirst) {
		// Video input.
		RETURN_IF_FAILED(video_input_pin_.Create("Video_Input", IPin::PD_Input,
		  static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&video_input_pin_));
    
    // Create the lane data media description
    
    // Set up the media description manager object for object output
    cObjectPtr<IMediaDescriptionManager> description_manager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER,
      IID_ADTF_MEDIA_DESCRIPTION_MANAGER, (tVoid**) &description_manager, __exception_ptr));
    
    tChar const * lane_type = description_manager->GetMediaDescription("LanePointArray");
    RETURN_IF_POINTER_NULL(lane_type);
    cObjectPtr<IMediaType> lane_signal_value = new cMediaType(0, 0, 0, "LanePointArray",
      lane_type,IMediaDescription::MDF_DDL_DEFAULT_VERSION);	
    RETURN_IF_FAILED(lane_signal_value->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION,
      (tVoid**)&lane_data_description_));
      
    RETURN_IF_FAILED(lane_output_pin_.Create("lanes", lane_signal_value, NULL));
    RETURN_IF_FAILED(RegisterPin(&lane_output_pin_));
    
    // Create the object input pin.      
    tChar const * object_stream_type = description_manager->GetMediaDescription("ObjectArray");
    RETURN_IF_POINTER_NULL(object_stream_type);
    
    // Create the stream description used to receive/send data
    cObjectPtr<IMediaType> object_type_signal_value = new cMediaType(0, 0, 0, "ObjectArray",
      object_stream_type, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(object_type_signal_value->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION,
      (tVoid**) &object_data_description_));
    
    // Create and register the input pin
    RETURN_IF_FAILED(object_input_pin_.Create("objects", object_type_signal_value, static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&object_input_pin_));
    
    // Create the headlight output stuff
    tChar const * bool_stream_type = description_manager->GetMediaDescription("tBoolSignalValue");
    RETURN_IF_POINTER_NULL(bool_stream_type);
    
    cObjectPtr<IMediaType> bool_signal_type = new cMediaType(0, 0, 0, "tBoolSignalValue", bool_stream_type, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(bool_signal_type->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**) &light_data_description_));
    
    RETURN_IF_FAILED(light_output_.Create("headlight_enabled", bool_signal_type, NULL));
    RETURN_IF_FAILED(RegisterPin(&light_output_));
	}
	
	else if (stage == StageGraphReady) {
	  cObjectPtr<IMediaType> type;
	  RETURN_IF_FAILED(video_input_pin_.GetMediaType(&type));
	  
	  cObjectPtr<IMediaTypeVideo> type_video;
	  RETURN_IF_FAILED(type->GetInterface(IID_ADTF_MEDIA_TYPE_VIDEO, (tVoid**) &type_video));
	  
	  const tBitmapFormat* format = type_video->GetFormat();
	  if (format == NULL) RETURN_ERROR(ERR_NOT_SUPPORTED);
	  
    cMemoryBlock::MemCopy(&input_format_, format, sizeof(tBitmapFormat));
	}
	
	RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult LaneFilter::Shutdown(tInitStage stage, __exception) {
// -------------------------------------------------------------------------------------------------
	return cFilter::Shutdown(stage,__exception_ptr);
}

// -------------------------------------------------------------------------------------------------
tResult LaneFilter::OnPinEvent(IPin* source, tInt event_code, tInt param1, tInt param2,
    IMediaSample* media_sample) {
// -------------------------------------------------------------------------------------------------
	RETURN_IF_POINTER_NULL(source);
	RETURN_IF_POINTER_NULL(media_sample);
	
	if (event_code == IPinEventSink::PE_MediaSampleReceived) {
		if (source == &video_input_pin_) {
      processImage(media_sample);
		} else if (source == &object_input_pin_) {
		  // read-out the incoming Media Sample
      cObjectPtr<IMediaCoder> coder_input;
      RETURN_IF_FAILED(object_data_description_->Lock(media_sample, &coder_input));
      
      //get values from media sample        
      tUInt32 size;
      coder_input->Get("size", (tVoid*)&size);
      Object object_array[size];
	    media_sample->CopyBufferTo((tVoid*)&object_array, sizeof(Object) * size, sizeof(tUInt32), 0);
	    lane_preprocessor_.set_object_vector(object_array, sizeof(object_array)/sizeof(Object));
      object_data_description_->Unlock(coder_input);
		}
	}
		
	RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult LaneFilter::processImage(IMediaSample* image_sample) {
// -------------------------------------------------------------------------------------------------
  const tVoid* source_buffer;
  
  bool enable_lights = false;
  
  if (IS_OK(image_sample->Lock(&source_buffer))) {
    int source_width = input_format_.nWidth;
    int source_height = input_format_.nHeight;
    
    Mat current_image(source_height, source_width, CV_8UC3, (uchar*)source_buffer);
    
    enable_lights = calculateImageLuminosity(current_image) < GetPropertyFloat("lum_threshold");
    
    // Detect the edges of the image by using a Canny Algo
    Mat preprocessed;
    lane_preprocessor_.set_gaussian_blur(Size(15,15), 2);
    lane_preprocessor_.preprocess_image(current_image, preprocessed);
    
    std::vector<Vector2> mapped_points;
    Mat mapping_image;
    lane_detector_.detect_lanes(preprocessed, mapping_image, mapped_points);
    image_sample->Unlock(source_buffer);
    
    transmitLanePoints(mapped_points);
  }
  
  if (GetPropertyBool("headlights_enabled")) {
    if(enable_lights != headlights_on_) {
      transmitHeadLight(enable_lights);
      headlights_on_ = enable_lights;
    }
  }
  
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
float LaneFilter::calculateImageLuminosity(Mat const & image) {
// -------------------------------------------------------------------------------------------------
  int scan_rate = GetPropertyInt("scan_rate");
  float sum = 0;
  int counter = 0;
  for (int row = 0; row < image.rows; row += scan_rate) {
    for (int col = 0; col < image.cols; col += scan_rate) {
      Vec3b cur = image.at<Vec3b>(row, col);
      sum += (1.0f / 3.0f) * (cur[0] + cur[1] + cur[2]);
      counter++;
    }
  }
  
  if (GetPropertyBool("debug")) LOG_INFO(cString::Format("Current luminosity: %f", sum / counter));
  return sum / counter;
}

// -------------------------------------------------------------------------------------------------
tResult LaneFilter::transmitLanePoints(std::vector<Vector2> const & mapped_points) {
// -------------------------------------------------------------------------------------------------
  cObjectPtr<IMediaSample> lane_points_sample;
  RETURN_IF_FAILED(AllocMediaSample(&lane_points_sample));

  RETURN_IF_FAILED(lane_points_sample->AllocBuffer(sizeof(tUInt32) + sizeof(Vector2) * mapped_points.size()));

  tUInt32* dest_buffer = NULL;
  RETURN_IF_FAILED(lane_points_sample->WriteLock((tVoid**)&dest_buffer));
  
  (*dest_buffer) = (tUInt32)mapped_points.size();
  dest_buffer++;

  cMemoryBlock::MemCopy(dest_buffer, &(mapped_points[0]), sizeof(Vector2) * mapped_points.size());
  
  RETURN_IF_FAILED(lane_points_sample->Unlock((tVoid*)dest_buffer));
  
  lane_output_pin_.Transmit(lane_points_sample);
	RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult LaneFilter::transmitHeadLight(bool value) {
// -------------------------------------------------------------------------------------------------
  cObjectPtr<IMediaSample> pMediaSample;
  AllocMediaSample((tVoid**)&pMediaSample);

  //allocate memory with the size given by the descriptor
  cObjectPtr<IMediaSerializer> pSerializer;
  light_data_description_->GetMediaSampleSerializer(&pSerializer);
  tInt nSize = pSerializer->GetDeserializedSize();
  pMediaSample->AllocBuffer(nSize);

  tUInt32 time_stamp = 0;
    
  {
    __adtf_sample_write_lock_mediadescription(light_data_description_, pMediaSample, coder);
    coder->Set("bValue", (tVoid*)&value);
    coder->Set("ui32ArduinoTimestamp", (tVoid*)&time_stamp);
  }

  //transmit media sample over output pin
  pMediaSample->SetTime(_clock->GetStreamTime());
  light_output_.Transmit(pMediaSample);
  RETURN_NOERROR;
}
