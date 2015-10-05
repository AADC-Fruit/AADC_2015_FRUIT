#include "stdafx.h"
#include "transformation_filter.h"
#include "../util/map/map_settings.h"
#include <iostream>
#include <vector>

// -------------------------------------------------------------------------------------------------
ADTF_FILTER_PLUGIN("FRUIT Transformation Filter", OID_ADTF_TRANSFORMATION_FILTER, TransformationFilter);
// -------------------------------------------------------------------------------------------------
 
// -------------------------------------------------------------------------------------------------
TransformationFilter::TransformationFilter(const tChar* __info) : adtf::cTimeTriggeredFilter(__info) {
// -------------------------------------------------------------------------------------------------
  SetPropertyInt("interval_ms", 250);
  SetPropertyBool("debug", false);
}

// -------------------------------------------------------------------------------------------------
TransformationFilter::~TransformationFilter() {
// -------------------------------------------------------------------------------------------------

}

// -------------------------------------------------------------------------------------------------
tResult TransformationFilter::Init(tInitStage eStage, __exception) {
// -------------------------------------------------------------------------------------------------
  // never miss calling the parent implementation!!
  RETURN_IF_FAILED(cFilter::Init(eStage, __exception_ptr))
  

  if (eStage == StageFirst) {
    cObjectPtr<IMediaDescriptionManager> description_manager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER,
      IID_ADTF_MEDIA_DESCRIPTION_MANAGER, (tVoid**) &description_manager, __exception_ptr));

    // Create the stream description used to receive/send data
    tChar const * gyro_input_stream_type = description_manager->GetMediaDescription("tSignalValue");
    RETURN_IF_POINTER_NULL(gyro_input_stream_type);
    cObjectPtr<IMediaType> gyro_input_type_signal_value = new cMediaType(0, 0, 0, "tSignalValue",
      gyro_input_stream_type, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(gyro_input_type_signal_value->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION,
      (tVoid**) &gyro_input_stream_description_));
      
    // Create the stream description used to receive/send data
    tChar const * speed_input_stream_type = description_manager->GetMediaDescription("tSignalValue");
    RETURN_IF_POINTER_NULL(speed_input_stream_type);
    cObjectPtr<IMediaType> speed_input_type_signal_value = new cMediaType(0, 0, 0, "tSignalValue",
      speed_input_stream_type, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(speed_input_type_signal_value->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION,
      (tVoid**) &speed_input_stream_description_));

    tChar const * output_stream_type = description_manager->GetMediaDescription("Vector2MatrixArray");
    RETURN_IF_POINTER_NULL(output_stream_type);
    cObjectPtr<IMediaType> output_type_signal_value = new cMediaType(0, 0, 0, "Vector2MatrixArray",
      output_stream_type, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(output_type_signal_value->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION,
      (tVoid**) &output_stream_description_));

    // Create the input pins
    RETURN_IF_FAILED(left_wheel_speed_pin_.Create("left_wheel_speed", speed_input_type_signal_value, static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&left_wheel_speed_pin_));
    RETURN_IF_FAILED(right_wheel_speed_pin_.Create("right_wheel_speed", speed_input_type_signal_value, static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&right_wheel_speed_pin_));
    RETURN_IF_FAILED(gyro_yaw_input_pin_.Create("gyro_yaw_angle", gyro_input_type_signal_value, static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&gyro_yaw_input_pin_));

    // Create the output pins
    RETURN_IF_FAILED(matrix_output_pin_.Create("matrix", output_type_signal_value, NULL));
    RETURN_IF_FAILED(RegisterPin(&matrix_output_pin_));
  }
  
  else if (eStage == StageNormal) {
    // In this stage you would do further initialisation and/or create your dynamic pins.
    // Please take a look at the demo_dynamicpin example for further reference.
  }
  
  else if (eStage == StageGraphReady) {
    // All pin connections have been established in this stage so you can query your pins
    // about their media types and additional meta data.
    // Please take a look at the demo_imageproc example for further reference.
    SetInterval(GetPropertyInt("interval_ms") * 1000);
    
    debug_ = GetPropertyBool("debug");
    
    last_transformation_time_stamp_ = 0;
    last_transformation_yaw_ = 0;
    last_measurement_yaw_ = 0;
    last_second_ = 0;
  }
  
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult TransformationFilter::Shutdown(tInitStage eStage, __exception) {
// -------------------------------------------------------------------------------------------------
  // Call the base class implementation
  return cFilter::Shutdown(eStage, __exception_ptr);
}

// -------------------------------------------------------------------------------------------------
tResult TransformationFilter::Cycle(__exception) {
// -------------------------------------------------------------------------------------------------
  // Check if the transformation is necessary due to yaw change
  if(check_transformation_gyro(last_measurement_yaw_)) {
    if (debug_) LOG_INFO("Calculate transformation matrix due to gyro");
    transform_map(last_timestamp_);
  }
  
  // Check if the transformation is necessary due to wheel sensors
  if(check_transformation_speeds(last_timestamp_)) {
    if (debug_) LOG_INFO("Calculate transformation matrix due to wheels");
    transform_map(last_timestamp_);
    right_wheel_speeds_.clear();
    left_wheel_speeds_.clear();
  }
  
  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult TransformationFilter::OnPinEvent(IPin* source, tInt event_code, tInt param1, tInt param2, IMediaSample* media_sample) {
// -------------------------------------------------------------------------------------------------
  // This pointer is not null if a media sample was received
  RETURN_IF_POINTER_NULL(media_sample);

  // Check for the type of the event (data received or transmitted, etc.)
  if (event_code == IPinEventSink::PE_MediaSampleReceived) {
    float sensor_value = 0;
	  tTimeStamp time_stamp = 0;
	  
    if (source == &gyro_yaw_input_pin_) {
      {
        __adtf_sample_read_lock_mediadescription(gyro_input_stream_description_, media_sample, coder);
        coder->Get("f32Value", (tVoid*) &sensor_value);
        coder->Get("ui32ArduinoTimestamp", (tVoid*) &time_stamp);
      }

      last_measurement_yaw_ = sensor_value;
      last_timestamp_ = time_stamp;
    }
    
    else {
      {
        __adtf_sample_read_lock_mediadescription(speed_input_stream_description_, media_sample, coder);
        coder->Get("f32Value", (tVoid*) &sensor_value);
        coder->Get("ui32ArduinoTimestamp", (tVoid*) &time_stamp);
      }
    
      // Update the timestamp
      last_timestamp_ = time_stamp;
    
      // Update the wheel data
      if (source == &left_wheel_speed_pin_) {
        left_wheel_speeds_.push_back(sensor_value);
      }

      else if (source == &right_wheel_speed_pin_) {
        right_wheel_speeds_.push_back(sensor_value);
      }
    }    
  }

  RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
bool TransformationFilter::check_transformation_speeds(tTimeStamp new_time_stamp) {
// -------------------------------------------------------------------------------------------------
  tFloat32 average_speed = 0;
  for(size_t i = 0; i < left_wheel_speeds_.size(); i++) {
    average_speed += left_wheel_speeds_.at(i);
  }
  
  for(size_t i = 0; i < right_wheel_speeds_.size(); i++) {
    average_speed += right_wheel_speeds_.at(i);
  }
  
  average_speed /= (left_wheel_speeds_.size() + right_wheel_speeds_.size());
  average_speed_ = average_speed;
  tFloat32 traveled_distance = average_speed * (new_time_stamp - last_transformation_time_stamp_);
  //no /10 because we need our distance in cm, not in m, since our cell size is also in cm
  //std::cout << "Average speed and distance: " << average_speed_ << ", " << traveled_distance << std::endl;
  // traveled_distance in cm, therefor only /10.0
  if((int)((new_time_stamp - last_transformation_time_stamp_) / (double)(1E3)) != last_second_){
    last_second_ = (int)((new_time_stamp - last_transformation_time_stamp_) / (double)(1E3));
    //std::cout << "Distance: " << traveled_distance << " and average measured speed: " << average_speed << " and time: " << (new_time_stamp - last_transformation_time_stamp_) / (double)(1E3) <<std::endl;
  }
  if(traveled_distance > CELL_SIZE) {
    //std::cout << "Distance: " << traveled_distance << " and average measured speed: " << average_speed << std::endl;
    return true;
  }
  return false;
}

// -------------------------------------------------------------------------------------------------
bool TransformationFilter::check_transformation_gyro(float new_gyro_value) {
// -------------------------------------------------------------------------------------------------
  if (((new_gyro_value > 0.0) && (last_transformation_yaw_ < 0.0)) || ((new_gyro_value < 0.0) && (last_transformation_yaw_ > 0.0))) {
    // passing 180 mark
    if ((fabs(new_gyro_value) > (90.0 * M_PI / 180.0 )) && (fabs(last_transformation_yaw_) > (90.0 * M_PI / 180.0))) {
      if ((360.0 * M_PI / 180.0)- fabs(new_gyro_value) - fabs(last_transformation_yaw_) > MIN_YAW_CHANGE) {
        return true;
      }
      else return false;
    } else {
      if ((fabs(new_gyro_value) + fabs(last_transformation_yaw_)) > MIN_YAW_CHANGE) {
        return true;
      }
      else return false;
    }
  } else if ((fabs(new_gyro_value - last_transformation_yaw_)) > MIN_YAW_CHANGE) {
    return true;
  }
  
  return false;
}

// -------------------------------------------------------------------------------------------------
void TransformationFilter::transform_map(tTimeStamp new_time_stamp) {
// -------------------------------------------------------------------------------------------------
  //std::cout << "[MF] Start transform_map()" << std::endl;
  if(average_speed_ > 0.0) forward_ = true;
  else forward_ = false;
  float traveled_distance = fabs(average_speed_) * (new_time_stamp - last_transformation_time_stamp_);
  //no /10 because we need cm, not m! (cellsize in cm)
  float yaw_change;
  last_transformation_time_stamp_ = new_time_stamp;
  
  yaw_change = last_measurement_yaw_ - last_transformation_yaw_;
  if(yaw_change > (180.0 * M_PI / 180.0)) {
    yaw_change = (360.0 * M_PI / 180.0) - yaw_change;
  } else if(yaw_change < (-180.0 * M_PI / 180.0)) {
    yaw_change = (-360.0 * M_PI / 180.0) - yaw_change;
  }
  if(yaw_change > 0.0) turn_left_ = false;
  else turn_left_ = true;
  float abs_yaw_change = fabs(yaw_change);
  
  //change rotation direction???
  float turn_x;
  float turn_y;
  if(abs_yaw_change < MIN_YAW_CHANGE) {
     turn_x = 0;
     turn_y = traveled_distance;
     yaw_change = 0;
  } else {
    last_transformation_yaw_ = last_measurement_yaw_;
    float radius = traveled_distance / abs_yaw_change;
    turn_x = radius;
    //turn_x = radius - radius * cos(abs_yaw_change);
    //turn_y = radius * sin(abs_yaw_change);
    turn_y = (CAR_ROW * CELL_SIZE) + 45;
  }
  
  // Modify results depending on direction and heading
  if(turn_left_) {
    turn_x = -turn_x;
  }
  
  turn_x = CAR_COL * CELL_SIZE + turn_x;
  // Create and transmit the matrix
  //std::cout << "[TF] Pre transform()" << std::endl;
  Vector2** transformation_matrix = new Vector2*[MAP_HEIGHT];
  for (int i = 0; i < MAP_HEIGHT; i++) {
    transformation_matrix[i] = new Vector2[MAP_WIDTH];
  }
  
  //turn x and y in cm, not grid cells anymore!
  transformator_.transform(transformation_matrix, turn_x, turn_y, yaw_change, MAP_HEIGHT, MAP_WIDTH, CELL_SIZE);

  
  //std::cout << "[TF] Post transform()" << std::endl;
  std::vector<Vector2> matrix;
  for (int i = 0; i < MAP_HEIGHT; i++) {
    for (int j = 0; j < MAP_WIDTH; j++) {
      matrix.push_back(transformation_matrix[i][j]);
    }
  }
  
  //std::cout << "[TF] Pre transmit()" << std::endl;
  transmitMatrix(matrix);
  
  for (int i = 0; i < MAP_HEIGHT; i++) {
    delete[] transformation_matrix[i];
  }
  delete[] transformation_matrix;
}

// -------------------------------------------------------------------------------------------------
tResult TransformationFilter::transmitMatrix(std::vector<Vector2> const & matrix) {
// -------------------------------------------------------------------------------------------------
  cObjectPtr<IMediaSample> lane_points_sample;
  RETURN_IF_FAILED(AllocMediaSample(&lane_points_sample));

  RETURN_IF_FAILED(lane_points_sample->AllocBuffer(sizeof(tUInt32) + sizeof(Vector2) * matrix.size()));

  tUInt32* dest_buffer = NULL;
  RETURN_IF_FAILED(lane_points_sample->WriteLock((tVoid**)&dest_buffer));
  
  (*dest_buffer) = (tUInt32)matrix.size();
  dest_buffer++;

  cMemoryBlock::MemCopy(dest_buffer, &(matrix[0]), sizeof(Vector2) * matrix.size());
  
  RETURN_IF_FAILED(lane_points_sample->Unlock((tVoid*)dest_buffer));
  
  matrix_output_pin_.Transmit(lane_points_sample);
	RETURN_NOERROR;
}
