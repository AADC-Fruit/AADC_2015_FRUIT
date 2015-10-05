#include "stdafx.h"
#include "lychee_script_filter.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>

// -------------------------------------------------------------------------------------------------
ADTF_FILTER_PLUGIN("FRUIT Lychee Script Filter", OID_ADTF_LYCHEE_SCRIPT_FILTER, LycheeScriptFilter);
// -------------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------------
LycheeScriptFilter::LycheeScriptFilter(const tChar* __info):adtf::cTimeTriggeredFilter(__info) {
// -------------------------------------------------------------------------------------------------
  SetPropertyInt("interval_ms", 100);
  SetPropertyBool("debug", false);
  SetPropertyStr("start_path", "");
  SetPropertyBool("start_path" NSSUBPROP_FILENAME, tTrue);
  SetPropertyStr("start_path" NSSUBPROP_FILENAME NSSUBSUBPROP_EXTENSIONFILTER, "Lychee script files (*.lychee)");
  SetPropertyBool("direct_start", false);
}

// -------------------------------------------------------------------------------------------------
LycheeScriptFilter::~LycheeScriptFilter() {
// -------------------------------------------------------------------------------------------------

}

// -------------------------------------------------------------------------------------------------
tResult LycheeScriptFilter::Init(tInitStage eStage, __exception) {
// -------------------------------------------------------------------------------------------------
  // never miss calling the parent implementation!!
  RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr))

  if (eStage == StageFirst) {
    // Initialize the descriptors and media type stuff here
    cObjectPtr<IMediaDescriptionManager> description_manager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER,
    IID_ADTF_MEDIA_DESCRIPTION_MANAGER,(tVoid**)&description_manager,__exception_ptr));
  
    // Create and register the output pin
    tChar const * bool_stream_description = description_manager->GetMediaDescription("tBoolSignalValue");
    RETURN_IF_POINTER_NULL(bool_stream_description);        
    cObjectPtr<IMediaType> bool_type = new cMediaType(0, 0, 0, "tBoolSignalValue",
      bool_stream_description, IMediaDescription::MDF_DDL_DEFAULT_VERSION);	
		RETURN_IF_FAILED(bool_type->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION,
		  (tVoid**)&bool_stream_description_));
		  
	  tChar const * wheel_lock_bool_stream_description = description_manager->GetMediaDescription("tBoolSignalValue");
    RETURN_IF_POINTER_NULL(wheel_lock_bool_stream_description);        
    cObjectPtr<IMediaType> wheel_lock_bool_type = new cMediaType(0, 0, 0, "tBoolSignalValue",
      wheel_lock_bool_stream_description, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
      	
		RETURN_IF_FAILED(wheel_lock_bool_type->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION,
		  (tVoid**)&wheel_lock_bool_stream_description_));

    RETURN_IF_FAILED(active_flag_input_pin_.Create("active", bool_type, static_cast<IPinEventSink*>(this)));
		RETURN_IF_FAILED(RegisterPin(&active_flag_input_pin_));
		RETURN_IF_FAILED(wheel_lock_active_flag_input_pin_.Create("wheel_lock_active", wheel_lock_bool_type, static_cast<IPinEventSink*>(this)));
		RETURN_IF_FAILED(RegisterPin(&wheel_lock_active_flag_input_pin_));
		RETURN_IF_FAILED(inactive_flag_output_pin_.Create("inactive", bool_type, NULL));
		RETURN_IF_FAILED(RegisterPin(&inactive_flag_output_pin_));
		
		tChar const * output_stream_description = description_manager->GetMediaDescription("tSignalValue");
    RETURN_IF_POINTER_NULL(output_stream_description);
    
    // Create the turn and hazard signal pins
    RETURN_IF_FAILED(turn_left_output_.Create("left_turning_lights", bool_type, NULL));
		RETURN_IF_FAILED(RegisterPin(&turn_left_output_));

    RETURN_IF_FAILED(turn_right_output_.Create("right_turning_lights", bool_type, NULL));
		RETURN_IF_FAILED(RegisterPin(&turn_right_output_));
		
		RETURN_IF_FAILED(hazardlights_output_.Create("hazardLightsEnabled", bool_type, NULL));
		RETURN_IF_FAILED(RegisterPin(&hazardlights_output_));
    
    // Create the stream description used to receive/send data
    cObjectPtr<IMediaType> output_type = new cMediaType(0, 0, 0, "tSignalValue",
      output_stream_description, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(output_type->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION,
      (tVoid**) &signal_stream_description_));

    RETURN_IF_FAILED(steering_angle_output_pin_.Create("steering_angle", output_type, NULL));
    RETURN_IF_FAILED(RegisterPin(&steering_angle_output_pin_));
    RETURN_IF_FAILED(speed_output_pin_.Create("speed", output_type, NULL));
    RETURN_IF_FAILED(RegisterPin(&speed_output_pin_));
    
    // Create the string media description
    tChar const * string_stream_description = description_manager->GetMediaDescription("StringData");
    RETURN_IF_POINTER_NULL(string_stream_description);  
    cObjectPtr<IMediaType> string_type = new cMediaType(0, 0, 0, "StringData", string_stream_description, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(string_type->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**) &string_stream_description_));
    
    RETURN_IF_FAILED(script_input_pin_.Create("script_path", string_type, static_cast<IPinEventSink*>(this)));
		RETURN_IF_FAILED(RegisterPin(&script_input_pin_));
  }  
  
  else if (eStage == StageGraphReady) {
		this->SetInterval(GetPropertyInt("interval_ms") * 1000);
		timer_current_ = 0;
		timer_goal_ = 0;
		
		if (GetPropertyBool("direct_start")) {
      active_ = true;
    } else active_ = false;
    
    wheel_lock_active_ = false;
    
    if (strlen(GetPropertyStr("start_path")) != 0) {
      LOG_INFO(cString::Format("Starting with %s", GetPropertyStr("start_path")));
      std::string path(GetPropertyStr("start_path"));
      parseScript(path);
    }
  }
  
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult LycheeScriptFilter::Shutdown(tInitStage eStage, __exception) {
// -------------------------------------------------------------------------------------------------
  // Call the base class implementation
  return cFilter::Shutdown(eStage, __exception_ptr);
}

// -------------------------------------------------------------------------------------------------
tResult LycheeScriptFilter::OnPinEvent(IPin* source, tInt event_code, tInt param1, tInt param2, IMediaSample* media_sample) {
// -------------------------------------------------------------------------------------------------
  RETURN_IF_POINTER_NULL(source);
	RETURN_IF_POINTER_NULL(media_sample);
	
	if (event_code == IPinEventSink::PE_MediaSampleReceived) {
		if (source == &active_flag_input_pin_) {
		  {
        __adtf_sample_read_lock_mediadescription(bool_stream_description_, media_sample, coder);
        coder->Get("bValue", (tVoid*)&active_);
      }
		}
		
		else if (source == &wheel_lock_active_flag_input_pin_) {
		  {
        __adtf_sample_read_lock_mediadescription(wheel_lock_bool_stream_description_, media_sample, coder);
        coder->Get("bValue", (tVoid*)&wheel_lock_active_);
      }
		}
		
		else if (source == &script_input_pin_) {
		  size_t length = 0;
		  std::string filepath;
		  {
		    __adtf_sample_read_lock_mediadescription(string_stream_description_, media_sample, coder);
		    coder->Get("length", (tVoid*) &length);
		    
		    char path_array[length];
	      media_sample->CopyBufferTo((tVoid*)&path_array, sizeof(char) * length, sizeof(tUInt32), 0);
	      filepath = std::string(path_array);
		  }
		  LOG_INFO(cString::Format("Parsing Script %s", filepath.c_str()));
		  parseScript(filepath);
		}
  }
	
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult LycheeScriptFilter::Cycle(__exception) {
// -------------------------------------------------------------------------------------------------
  if (!active_ || !wheel_lock_active_) {
    if (GetPropertyBool("debug")) if (!active_) LOG_INFO("Active is false.");
    if (GetPropertyBool("debug")) if (!wheel_lock_active_) LOG_INFO("Wheel lock active is false.");
    RETURN_NOERROR;
  }
  
  // Check for waiting
  if (timer_current_ < timer_goal_) {
    //if (GetPropertyBool("debug")) LOG_INFO("Increment counter");
    timer_current_++;
    RETURN_NOERROR;
  }
  
  if (script_list_.empty()) {
    LOG_INFO("Script done.");
    active_ = false;
    TransmitInactiveFlag(0);
    RETURN_NOERROR;
  }
  
  // Retrieve the next element in the list
  std::pair<ScriptType, float> current = script_list_.front();
  
  if (current.first == DELAY) {
    if (GetPropertyBool("debug")) LOG_INFO(cString::Format("Process DELAY %f", current.second));
    timer_goal_ = (current.second * 1000) / GetPropertyInt("interval_ms");
    timer_current_ = 0;
    //if (GetPropertyBool("debug")) LOG_INFO(cString::Format("Set timer to %d, current %d", timer_goal_, timer_current_));
  } else if (current.first == SPEED) {
    if (GetPropertyBool("debug")) LOG_INFO(cString::Format("Process SPEED %f", current.second));
    TransmitSpeed(0, current.second);
  } else if (current.first == ANGLE) {
    if (GetPropertyBool("debug")) LOG_INFO(cString::Format("Process ANGLE %f", current.second));
    TransmitAngle(0, current.second);
  } else if (current.first == RSIGN) {
    if (GetPropertyBool("debug")) LOG_INFO(cString::Format("Process RSIGN %f", current.second));
    TransmitLight(turn_right_output_, current.second == 1);
  } else if (current.first == LSIGN) {
    if (GetPropertyBool("debug")) LOG_INFO(cString::Format("Process LSIGN %f", current.second));
    TransmitLight(turn_left_output_, current.second == 1);
  } else if (current.first == HAZZA) {
    if (GetPropertyBool("debug")) LOG_INFO(cString::Format("Process HAZZA %f", current.second));
    TransmitLight(hazardlights_output_, current.second == 1);
  }
  script_list_.pop_front();
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
void LycheeScriptFilter::parseScript(std::string const & filepath) {
// -------------------------------------------------------------------------------------------------
  /*if (!script_list_.empty()) LOG_INFO("Overwriting not-empty script list");
  script_list_.clear();*/
  
  std::ifstream file(filepath.c_str());
  while (file.good()) {
    std::string line;
    std::getline(file, line);

    // Intercept comments
    if (line.length() == 0) continue;
    if (line.find("#") == 0) continue;
    
    // Retrieve the float value
    float value = -1;
    size_t space_index = line.find(" ");
    if (space_index == std::string::npos) {
      LOG_INFO(cString::Format("ERROR: Failed to parse line: %s", line.c_str()));
      continue;
    }
    
    std::stringstream conv;
    conv << line.substr(space_index + 1);
    conv >> value;
    
    if (line.find("DELAY") != std::string::npos) {
      script_list_.push_back(std::pair<ScriptType, float>(DELAY, value));
      if (GetPropertyBool("debug")) LOG_INFO(cString::Format("Add DELAY %f", value));
    } else if (line.find("SPEED") != std::string::npos) {
      script_list_.push_back(std::pair<ScriptType, float>(SPEED, value));
      if (GetPropertyBool("debug")) LOG_INFO(cString::Format("Add SPEED %f", value));
    } else if (line.find("ANGLE") != std::string::npos) {
      script_list_.push_back(std::pair<ScriptType, float>(ANGLE, value));
      if (GetPropertyBool("debug")) LOG_INFO(cString::Format("Add ANGLE %f", value));
    } else if (line.find("LSIGN") != std::string::npos) {
      script_list_.push_back(std::pair<ScriptType, float>(LSIGN, value));
      if (GetPropertyBool("debug")) LOG_INFO(cString::Format("Add LSIGN %f", value));
    } else if (line.find("RSIGN") != std::string::npos) {
      script_list_.push_back(std::pair<ScriptType, float>(RSIGN, value));
      if (GetPropertyBool("debug")) LOG_INFO(cString::Format("Add RSIGN %f", value));
    } else if (line.find("HAZZA") != std::string::npos) {
      script_list_.push_back(std::pair<ScriptType, float>(HAZZA, value));
      if (GetPropertyBool("debug")) LOG_INFO(cString::Format("Add HAZZA %f", value));
    }
  }
  
  file.close();
}

// -------------------------------------------------------------------------------------------------
tResult LycheeScriptFilter::TransmitLight(cOutputPin & pin, bool value) {
// -------------------------------------------------------------------------------------------------
  cObjectPtr<IMediaSample> pMediaSample;
  AllocMediaSample((tVoid**)&pMediaSample);

  //allocate memory with the size given by the descriptor
  cObjectPtr<IMediaSerializer> pSerializer;
  bool_stream_description_->GetMediaSampleSerializer(&pSerializer);
  tInt nSize = pSerializer->GetDeserializedSize();
  pMediaSample->AllocBuffer(nSize);
  
  tUInt32 time_stamp = 0;

  {
    __adtf_sample_write_lock_mediadescription(bool_stream_description_, pMediaSample, coder);
    coder->Set("bValue", (tVoid*)&value);
    coder->Set("ui32ArduinoTimestamp", (tVoid*)&time_stamp);
  }

  //transmit media sample over output pin
  pMediaSample->SetTime(_clock->GetStreamTime());
  pin.Transmit(pMediaSample);
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
void LycheeScriptFilter::TransmitInactiveFlag(tTimeStamp time_stamp) {
// -------------------------------------------------------------------------------------------------
  cObjectPtr<IMediaSample> pMediaSample;
  AllocMediaSample((tVoid**)&pMediaSample);

  //allocate memory with the size given by the descriptor
  cObjectPtr<IMediaSerializer> pSerializer;
  bool_stream_description_->GetMediaSampleSerializer(&pSerializer);
  tInt nSize = pSerializer->GetDeserializedSize();
  pMediaSample->AllocBuffer(nSize);
     
  //write date to the media sample with the coder of the descriptor
  bool inactive = true;
    
  {
    __adtf_sample_write_lock_mediadescription(bool_stream_description_, pMediaSample, coder);
    coder->Set("bValue", (tVoid*)&inactive);
    coder->Set("ui32ArduinoTimestamp", (tVoid*)&time_stamp);
  }

  //transmit media sample over output pin
  pMediaSample->SetTime(_clock->GetStreamTime());
  inactive_flag_output_pin_.Transmit(pMediaSample);
}

// -------------------------------------------------------------------------------------------------
void LycheeScriptFilter::TransmitSpeed(tTimeStamp time_stamp, float speed) {
// -------------------------------------------------------------------------------------------------
  cObjectPtr<IMediaSample> pMediaSample;
  AllocMediaSample((tVoid**)&pMediaSample);

  //allocate memory with the size given by the descriptor
  cObjectPtr<IMediaSerializer> pSerializer;
  signal_stream_description_->GetMediaSampleSerializer(&pSerializer);
  tInt nSize = pSerializer->GetDeserializedSize();
  pMediaSample->AllocBuffer(nSize);
     
  //write date to the media sample with the coder of the descriptor	
  {
    __adtf_sample_write_lock_mediadescription(signal_stream_description_, pMediaSample, coder);
    coder->Set("f32Value", (tVoid*)&speed);
    coder->Set("ui32ArduinoTimestamp", (tVoid*)&time_stamp);
  }
  //transmit media sample over output pin
  pMediaSample->SetTime(_clock->GetStreamTime());
  speed_output_pin_.Transmit(pMediaSample);
}

// -------------------------------------------------------------------------------------------------
void LycheeScriptFilter::TransmitAngle(tTimeStamp time_stamp, float angle) {
// -------------------------------------------------------------------------------------------------
  cObjectPtr<IMediaSample> pMediaSample;
  AllocMediaSample((tVoid**)&pMediaSample);

  //allocate memory with the size given by the descriptor
  cObjectPtr<IMediaSerializer> pSerializer;
  signal_stream_description_->GetMediaSampleSerializer(&pSerializer);
  tInt nSize = pSerializer->GetDeserializedSize();
  pMediaSample->AllocBuffer(nSize);
     
  //write date to the media sample with the coder of the descriptor     
  {
    __adtf_sample_write_lock_mediadescription(signal_stream_description_, pMediaSample, coder);
    coder->Set("f32Value", (tVoid*)&angle);
    coder->Set("ui32ArduinoTimestamp", (tVoid*)&time_stamp);
  }

  //transmit media sample over output pin
  pMediaSample->SetTime(_clock->GetStreamTime());
  steering_angle_output_pin_.Transmit(pMediaSample);
}
