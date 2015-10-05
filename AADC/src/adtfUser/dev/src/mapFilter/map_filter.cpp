#include "stdafx.h"
#include "../util/vector2.h"
#include "map_filter.h"
#include "../util/map/map_settings.h"
#include <iostream>
#include <fstream>
//#include "../util/gridSearch/aStarSearch/a_star.h"
#include "../util/sign.h"
#include "../util/parking_type.h"
#include "../util/obstacle_type.h"

ADTF_FILTER_PLUGIN("FRUIT Map Filter", OID_ADTF_MAP_FILTER, MapFilter);

// -------------------------------------------------------------------------------------------------
MapFilter::MapFilter(const tChar* __info) 
  : sensor_map_(MAP_WIDTH, MAP_HEIGHT, CELL_SIZE, 0),
    lane_map_(MAP_WIDTH, MAP_HEIGHT, CELL_SIZE, 1000),
    object_map_(MAP_WIDTH, MAP_HEIGHT, CELL_SIZE, 0),
    angle_history_(ANGLE_HISTORY_SIZE) {
// -------------------------------------------------------------------------------------------------
  cMemoryBlock::MemSet(&output_format_, 0, sizeof(output_format_));
  output_format_.nWidth = MAP_WIDTH;
  output_format_.nHeight = MAP_HEIGHT;
  output_format_.nBitsPerPixel = 24;
  output_format_.nPixelFormat = 45;
  output_format_.nBytesPerLine = MAP_WIDTH * 3;
  output_format_.nSize = MAP_WIDTH * MAP_HEIGHT * 3;
  output_format_.nPaletteSize = 0;
  //grid_search_.addMap(sensor_map_);
  grid_search_.addMap(lane_map_);
  //grid_search_.addMap(object_map_);
  SetPropertyInt("speed", 25);
}

// -------------------------------------------------------------------------------------------------
MapFilter::~MapFilter() {
// -------------------------------------------------------------------------------------------------

}

// -------------------------------------------------------------------------------------------------
tResult MapFilter::Init(tInitStage stage, __exception) {
// -------------------------------------------------------------------------------------------------
  RETURN_IF_FAILED(cFilter::Init(stage, __exception_ptr));
  
	if (stage == StageFirst) {
	  current_maneuver_ = -1;
	  //left_goal_ = Vector2(0 ,0);
	  //top_goal_ = Vector2(0, 0);
	  //right_goal_ = Vector2(0,0);
	  crossroad_detected_ = false;
	  //left_free_ = top_free_ = right_free_ = false;
	  script_active_ = false;
	  maneuver_done_ = false;
	  already_parking_ = false;
	  
	  for (int i = 0; i < 6; i++) signs_[i] = fill((tInt8)(i + 1));
	    
    stop_line_row_ = stop_line_col_ = 0;
    stop_line_ahead_ = false;
	  
	  //for (int i = 0; i < 3; i++) crossroad_lanes_[i] = fill();
	  
	  for (int i = 0; i < 2; i++) obstacles_[i] = create();
	  error_angle_ = 0;
	  last_script_time_ = 0;
	  
	  // Create and register the input pin
    cObjectPtr<IMediaDescriptionManager> description_manager;
    RETURN_IF_FAILED(_runtime->GetObject(OID_ADTF_MEDIA_DESCRIPTION_MANAGER,
      IID_ADTF_MEDIA_DESCRIPTION_MANAGER,(tVoid**)&description_manager,__exception_ptr));
    
    // Create the sensor value media description
    tChar const * stream_type = description_manager->GetMediaDescription("tSignalValue");
    RETURN_IF_POINTER_NULL(stream_type);        
    cObjectPtr<IMediaType> pTypeSignalValue = new cMediaType(0, 0, 0, "tSignalValue",
      stream_type,IMediaDescription::MDF_DDL_DEFAULT_VERSION);	
    RETURN_IF_FAILED(pTypeSignalValue->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION,
      (tVoid**)&signal_data_description_));
      
    // Create the lane data media description
    tChar const * lane_type = description_manager->GetMediaDescription("LanePointArray");
    RETURN_IF_POINTER_NULL(lane_type);
    cObjectPtr<IMediaType> lane_signal_value = new cMediaType(0, 0, 0, "LanePointArray",
      lane_type,IMediaDescription::MDF_DDL_DEFAULT_VERSION);	
    RETURN_IF_FAILED(lane_signal_value->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION,
      (tVoid**)&lane_data_description_));
      
    // Create the object data media description      
    tChar const * object_stream_type = description_manager->GetMediaDescription("ObjectArray");
    RETURN_IF_POINTER_NULL(object_stream_type);    
    cObjectPtr<IMediaType> object_type_signal_value = new cMediaType(0, 0, 0, "ObjectArray",
      object_stream_type, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(object_type_signal_value->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION,
      (tVoid**) &object_data_description_));
      
    // Create the transformation data media description
    tChar const * transformation_stream_type = description_manager->GetMediaDescription("Vector2MatrixArray");
    RETURN_IF_POINTER_NULL(transformation_stream_type);    
    cObjectPtr<IMediaType> transformation_type_signal_value = new cMediaType(0, 0, 0, "Vector2MatrixArray",
      transformation_stream_type, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(transformation_type_signal_value->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION,
      (tVoid**) &transformation_data_description_));
      
    // Create the transformation data media description
    tChar const * road_sign_stream_type = description_manager->GetMediaDescription("tRoadSign");
    RETURN_IF_POINTER_NULL(road_sign_stream_type);    
    cObjectPtr<IMediaType> road_sign_signal_value = new cMediaType(0, 0, 0, "tRoadSign",
      road_sign_stream_type, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(road_sign_signal_value->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION,
      (tVoid**) &road_sign_data_description_));
      
    // Create the transformation data media description
    tChar const * bool_stream_type = description_manager->GetMediaDescription("tBoolSignalValue");
    RETURN_IF_POINTER_NULL(bool_stream_type);    
    cObjectPtr<IMediaType> bool_signal_value = new cMediaType(0, 0, 0, "tBoolSignalValue",
      bool_stream_type, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(bool_signal_value->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION,
      (tVoid**) &bool_data_description_));
      
    // Create the transformation data media description
    tChar const * confidence_stream_type = description_manager->GetMediaDescription("tBoolSignalValue");
    RETURN_IF_POINTER_NULL(confidence_stream_type);    
    cObjectPtr<IMediaType> confidence_signal_value = new cMediaType(0, 0, 0, "tBoolSignalValue",
      confidence_stream_type, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(confidence_signal_value->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION,
      (tVoid**) &confidence_data_description_));
      
    // Create the string media description
    tChar const * string_stream_description = description_manager->GetMediaDescription("StringData");
    RETURN_IF_POINTER_NULL(string_stream_description);  
    cObjectPtr<IMediaType> string_type = new cMediaType(0, 0, 0, "StringData", string_stream_description, IMediaDescription::MDF_DDL_DEFAULT_VERSION);
    RETURN_IF_FAILED(string_type->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**) &string_stream_description_));
    
    // Create the output pin description
    tChar const * package_description = description_manager->GetMediaDescription("SensorPackage");
    RETURN_IF_POINTER_NULL(package_description);        
    cObjectPtr<IMediaType> package_type = new cMediaType(0, 0, 0, "SensorPackage", package_description, IMediaDescription::MDF_DDL_DEFAULT_VERSION);	
    RETURN_IF_FAILED(package_type->GetInterface(IID_ADTF_MEDIA_TYPE_DESCRIPTION, (tVoid**)&sensor_package_description_));
    
    // Create the complex input pins
    RETURN_IF_FAILED(lane_input_pin_.Create("lanes", lane_signal_value, static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&lane_input_pin_));
   
    RETURN_IF_FAILED(object_input_pin_.Create("objects", object_type_signal_value, static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&object_input_pin_));
	
	  RETURN_IF_FAILED(transformation_input_pin_.Create("matrix", transformation_type_signal_value, static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&transformation_input_pin_));
    
    RETURN_IF_FAILED(road_sign_input_pin_.Create("road_sign", road_sign_signal_value, static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&road_sign_input_pin_));
    
    RETURN_IF_FAILED(inactive_flag_input_pin_.Create("script_inactive", bool_signal_value, static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&inactive_flag_input_pin_));
	
	  RETURN_IF_FAILED(command_input_pin_.Create("command", pTypeSignalValue, static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&command_input_pin_));
    
    RETURN_IF_FAILED(sensor_package_input_.Create("sensorPackage", package_type, static_cast<IPinEventSink*>(this)));
    RETURN_IF_FAILED(RegisterPin(&sensor_package_input_));
    
	  // Create and register the output pin
    RETURN_IF_FAILED(steering_output_pin_.Create("angle", pTypeSignalValue, NULL));
    RETURN_IF_FAILED(RegisterPin(&steering_output_pin_));
    
	  RETURN_IF_FAILED(speed_output_pin_.Create("speed", pTypeSignalValue, NULL));
    RETURN_IF_FAILED(RegisterPin(&speed_output_pin_));
    
    RETURN_IF_FAILED(active_flag_output_pin_.Create("script_active", bool_signal_value, NULL));
    RETURN_IF_FAILED(RegisterPin(&active_flag_output_pin_));
    
    RETURN_IF_FAILED(script_output_pin_.Create("script_path", string_type, NULL));
		RETURN_IF_FAILED(RegisterPin(&script_output_pin_));
		
		RETURN_IF_FAILED(command_output_pin_.Create("command_inactive", bool_signal_value, NULL));
    RETURN_IF_FAILED(RegisterPin(&command_output_pin_));
    
    RETURN_IF_FAILED(confidence_output_pin_.Create("confidence", confidence_signal_value, NULL));
    RETURN_IF_FAILED(RegisterPin(&confidence_output_pin_));
    
    // Video output
    RETURN_IF_FAILED(map_video_output_pin_.Create("Map_Video", IPin::PD_Output,
      NULL));
    RETURN_IF_FAILED(RegisterPin(&map_video_output_pin_));
    map_video_output_pin_.SetFormat(&output_format_, NULL);
	}
	
	RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
tResult MapFilter::Shutdown(tInitStage stage, __exception) {
// -------------------------------------------------------------------------------------------------
	return cFilter::Shutdown(stage,__exception_ptr);
}

// -------------------------------------------------------------------------------------------------
tResult MapFilter::OnPinEvent(IPin* source, tInt event_code, tInt param1, tInt param2,
    IMediaSample* media_sample) {
// -------------------------------------------------------------------------------------------------
	RETURN_IF_POINTER_NULL(source);
	RETURN_IF_POINTER_NULL(media_sample);
	
	
	// Create a synchronizer, that enters the critical section whenever possible. If a previous
	// thread is still inside the critical section, all proceeding threads have to wait for the
	// first to leave.
	__synchronized_obj(critical_section_);
	
	if (event_code == IPinEventSink::PE_MediaSampleReceived) {
		if (source == &lane_input_pin_ && current_maneuver_ != NOOP) {
		  //std::cout << "[M] Start OnPinEvent with source = lane_input_pin" << std::endl;
		  // read-out the incoming Media Sample
      cObjectPtr<IMediaCoder> coder_input;
      RETURN_IF_FAILED(lane_data_description_->Lock(media_sample, &coder_input));
      
      //get values from media sample        
      int size;
      coder_input->Get("size", (tVoid*)&size);
      
      Vector2 lane_points_array[size];
	    media_sample->CopyBufferTo((tVoid*)&lane_points_array, sizeof(Vector2) * size, sizeof(tUInt32), 0);      
      std::vector<Vector2> lane_points(lane_points_array, lane_points_array + sizeof(lane_points_array)/sizeof(Vector2));
      
      lane_data_description_->Unlock(coder_input);
		  
		  lane_map_.update(lane_points);
		  if (!script_active_) {
		    Vector2 start(CAR_COL, CAR_ROW);// - ceil(35/CELL_SIZE));
		    int cost = 0;
		    if (current_maneuver_ != PULL_OUT_RIGHT && current_maneuver_ != -1) {
		      for (int h = - 15/CELL_SIZE; h <= 15/CELL_SIZE; h+= 15/CELL_SIZE) {
            stop_line_ahead_ = checkForStopLine(stop_line_row_, stop_line_col_, h);
		        if (stop_line_ahead_) eraseStopLine(stop_line_row_, stop_line_col_);
          }
		    }
		    plan_ = grid_search_.getPlan(start, PLAN_LENGTH, cost);
		    /*while (cost > PLAN_COST_THRESHOLD * ((double)dynamic_plan_length / PLAN_LENGTH) && dynamic_plan_length > 2) {
		      dynamic_plan_length--;
		      plan_ = grid_search_.getPlan(start, dynamic_plan_length, cost);
		    }*/
		    
	      double angle = 0;
	      int angle_devider = 0;
	      int normal_angle = 0;
	      
	      double speed = 0;
	      int speed_devider = 0;
	      int normal_speed = 0;
	      
	      int max_angle = 0;
	      
	      for (int i = plan_.size() - 1; i > 0; i--) {
	        int x_diff = plan_[i - 1].get_x() - plan_[i].get_x();
	        int y_diff = plan_[i - 1].get_y() - plan_[i].get_y();
	        
	        ActionSet action_set;
	        for (int j = 0; j < action_set.size(); j++) {
	          if (action_set[j].x_diff() == x_diff && action_set[j].y_diff() == y_diff) {
	            if(abs(action_set[j].angle()) > max_angle) max_angle = abs(action_set[j].angle());
	            if(i == plan_.size() - 1) {
	              normal_angle = action_set[j].angle();
	              normal_speed = action_set[j].speed();
	            }
	            if (fabs(action_set[j].angle()) >= fabs(normal_angle) && getSign(action_set[j].angle()) * getSign(normal_angle) != -1) {
	              angle += (action_set[j].angle() * (i + 1));
	              //std::cout << "Added angle " << action_set[j].angle() << std::endl;
	              angle_devider += (i + 1);
	            }
	            if (action_set[j].speed() <= normal_speed) {
	              speed += action_set[j].speed();
	              speed_devider++;
	            }
            }
	        }		      
		    }
		    
		    if (angle_devider > 0) angle /= angle_devider;
	      if (speed_devider > 0) speed /= speed_devider;
	      
	      for (int i = 0; i < 6; i++) {
	        if (signs_[i].is_active && media_sample->GetTime() - signs_[i].time_stamp < SIGN_TIMESTAMP_THRESHOLD * 2) {
	          speed = 24;
	        }
	        if (signs_[SIGN_PARKING].is_active) speed = 23;
	      }
	      angle = updateAngleHistory(angle, max_angle);
	      //std::cout << "Angle: " << angle << ", normal would be " << normal_angle << std::endl;
        transmitSteeringAngle(media_sample->GetTime(), angle);
        transmitSpeed(media_sample->GetTime(), speed);
		    /*
		    int x_diff = plan_[plan_.size() - 2].get_x() - plan_[plan_.size() - 1].get_x();
	      int y_diff = plan_[plan_.size() - 2].get_y() - plan_[plan_.size() - 1].get_y();
		    
		    ActionSet action_set;
	        for (int j = 0; j < action_set.size(); j++) {
	          if (action_set[j].x_diff() == x_diff && action_set[j].y_diff() == y_diff) {
              std::cout << "Angle: " << action_set[j].angle() << std::endl;
	            transmitSteeringAngle(media_sample->GetTime(), action_set[j].angle());
              int speed = 26;//crossroad_ahead_ ? 26 : action_set[j].speed();
              transmitSpeed(media_sample->GetTime(), speed); 
            }
	        }
		    */
	      /*std::ofstream myfile;
        myfile.open ("/home/odroid/Desktop/img/path_costs.txt", std::ios::app);
        myfile << "Costs: " << cost << ", length: " << dynamic_plan_length << "\n";
        myfile.close();*/
		  }
		}
    
    else if (source == &object_input_pin_ && current_maneuver_ != NOOP) {
      // read-out the incoming Media Sample
      cObjectPtr<IMediaCoder> coder_input;
      RETURN_IF_FAILED(object_data_description_->Lock(media_sample, &coder_input));
      
      //get values from media sample        
      tUInt32 size;
      coder_input->Get("size", (tVoid*)&size);
      Object object_array[size];
	    media_sample->CopyBufferTo((tVoid*)&object_array, sizeof(Object) * size, sizeof(tUInt32), 0);
	    
	    std::vector<Object> objects(object_array, object_array + sizeof(object_array)/sizeof(Object));
      for (int i = 0; i < objects.size(); i++) {
        //std::cout << "Object" << i << ": " << objects[i].get_absolute_x()/CELL_SIZE + CAR_COL << ", " << CAR_ROW - objects[i].get_absolute_y()/CELL_SIZE << std::endl;
      }
      object_data_description_->Unlock(coder_input);
      object_map_.update(objects);
    }
    
    else if (source == &transformation_input_pin_ && current_maneuver_ != NOOP) {
      // read-out the incoming Media Sample
      cObjectPtr<IMediaCoder> coder_input;
      RETURN_IF_FAILED(transformation_data_description_->Lock(media_sample, &coder_input));
      
      //get values from media sample        
      int size;
      coder_input->Get("size", (tVoid*)&size);
      
      Vector2 matrix_array[size];
	    media_sample->CopyBufferTo((tVoid*)&matrix_array, sizeof(Vector2) * size, sizeof(tUInt32), 0);
	    transformation_data_description_->Unlock(coder_input);
	    
      std::vector<Vector2> matrix(matrix_array, matrix_array + sizeof(matrix_array)/sizeof(Vector2));
      
      Vector2** transformation_array = new Vector2*[MAP_HEIGHT];
      for (int i = 0; i < MAP_HEIGHT; i++) {
        transformation_array[i] = new Vector2[MAP_WIDTH];
        for (int j = 0; j < MAP_WIDTH; j++) {
          transformation_array[i][j] = matrix.at(i * MAP_WIDTH + j);
        }
      } 
		  
		  lane_map_.transform(transformation_array);
		  
		  std::vector<Vector2> transformed_parking_spots;
	    for (int i = 0; i < parking_spots_.size(); i++) {
	      int col = parking_spots_[i].get_x();
	      int row = parking_spots_[i].get_y();
	      if (row < 0 || col < 0) continue;
	      transformed_parking_spots.push_back(transformation_array[row][col]);
	    }
	    parking_spots_.clear();
	    for (int i = 0; i < transformed_parking_spots.size(); i++) {
	      if (transformed_parking_spots[i].get_x() > 0  && transformed_parking_spots[i].get_y() > 0) parking_spots_.push_back(transformed_parking_spots[i]);
	    }
		  
		  if (!already_parking_ && (current_maneuver_ == PARALLEL_PARKING || current_maneuver_ == CROSS_PARKING)) {
        for (int i = 0; i < parking_spots_.size(); i++) {
          std::cout << parking_spots_.size() << " spots remaining" << std::endl;
          if (abs(parking_spots_[i].get_y() - CAR_ROW) < 10/CELL_SIZE) {
            //std::cout << "near parking spot" << std::endl;
            if (sensorDistance(IR_L_FR) > 50) {
              //std::cout << "parking spot free" << std::endl;
              if (current_maneuver_ == PARALLEL_PARKING) {
                park(PARALLEL, media_sample->GetTime());
                //std::cout << "done with parallel parking" << std::endl;
                maneuver_done_ = true;
                parking_spots_.clear();
                break;
              }
              else if (current_maneuver_ == CROSS_PARKING) {
                park(CROSS, media_sample->GetTime());
                //std::cout << "done with cross parking" << std::endl;
                maneuver_done_ = true;
                parking_spots_.clear();
                break;
              }
            }
          }
        }
	 	  }
		  for (int i = 0; i < MAP_HEIGHT; i++) {
        delete[] transformation_array[i];
      }
      delete[] transformation_array;
    }
    
    else if (source == &road_sign_input_pin_ && !script_active_ && current_maneuver_ != NOOP) {
      //get values from media sample        
      tFloat32 image_size;
      tInt8 id;
      
      {
        __adtf_sample_read_lock_mediadescription(road_sign_data_description_, media_sample, coder);
        coder->Get("fl32Imagesize", (tVoid*) &image_size);
        coder->Get("i8Identifier", (tVoid*) &id);
      }
      
      if (isValid(id) && isActive(id, media_sample->GetTime())) {
        //std::cout << "Traffic sign with id " << (int)id << " is active!" << std::endl;
        //tFloat32 sign_distance = (0.0000652513 * (image_size * image_size)) - (0.2332 * image_size) + 334.89;
        tFloat32 sign_distance_squared = 464.092 - 0.49042 * image_size + 0.000188781 * (image_size * image_size);
        tFloat32 sign_distance_cubic = 512.408 - 0.75894 * image_size + 0.000578916 * (image_size * image_size)
          - 1.59942 * 1E-7 * (image_size * image_size * image_size);
        tFloat32 sign_distance = (sign_distance_squared + sign_distance_cubic) / 2.0;
        sign_distance = sign_distance > 220 ? sign_distance * 0.95 : sign_distance;
        //std::cout << "Distance: " << sign_distance << " with squared: " << sign_distance_squared << " and cubic " << sign_distance_cubic 
        //  << ", id: " << current_maneuver_ << std::endl;
        if (sign_distance > 120 && sign_distance < 210 && id != SIGN_PARKING) {
          crossroad_detected_ = true;
          /* old
          left_goal_ = Vector2(CAR_ROW - sign_distance / CELL_SIZE + 19, CAR_COL - 33);
          top_goal_ = Vector2(CAR_ROW - sign_distance / CELL_SIZE, CAR_COL);
          right_goal_ = Vector2(CAR_ROW - sign_distance / CELL_SIZE + 37, CAR_COL + 16);*/
          /*
          left_goal_ = Vector2(CAR_ROW - sign_distance / CELL_SIZE + 26, CAR_COL - 28); //20
          top_goal_ = Vector2(CAR_ROW - sign_distance / CELL_SIZE + 3, CAR_COL);
          right_goal_ = Vector2(CAR_ROW - sign_distance / CELL_SIZE + 26, CAR_COL + 16); //20
          
          //std::cout << "before planning" << std::endl;
          AStar a_star(lane_map_);
          std::vector<Vector2> left_goal_vector, top_goal_vector, right_goal_vector;
          for (int i = -11; i <= 11; i++) { // -9 -> 9
            left_goal_vector.push_back(Vector2(left_goal_.get_x() + i, left_goal_.get_y()));
            top_goal_vector.push_back(Vector2(top_goal_.get_x(), top_goal_.get_y() + i));
            right_goal_vector.push_back(Vector2(right_goal_.get_x() + i, right_goal_.get_y()));
          }
          //std::cout << "planning" << std::endl;
          int costs_left = a_star.getPlanCost(Vector2(CAR_ROW - ceil(35/CELL_SIZE), CAR_COL), left_goal_vector, AStarActionSet(0), 0, left_path_);
          int costs_top = a_star.getPlanCost(Vector2(CAR_ROW - ceil(35/CELL_SIZE), CAR_COL), top_goal_vector, AStarActionSet(1), 1, top_path_);
          int costs_right = a_star.getPlanCost(Vector2(CAR_ROW - ceil(35/CELL_SIZE), CAR_COL), right_goal_vector, AStarActionSet(2), 2, right_path_);
           
          //std::cout << "after planning" << std::endl;
          classifyCrossroad(costs_left, costs_top, costs_right);
          if (costs_left < 1000) left_free_ = true;
          if (costs_top < 1000) top_free_ = true;
          if (costs_right < 1000) right_free_ = true;
          std::cout << "Path costs: Left = " << costs_left << ", Top = " << costs_top << ", Right = " << costs_right 
            << " -> Classification: (" << left_free_ << ", " << top_free_ << ", " << right_free_ << ")" << std::endl; 
          updateCrossroadCounter(media_sample->GetTime());
          */
          if (current_maneuver_ == LEFT) {// && crossroad_lanes_[0].positive_counter > crossroad_lanes_[0].negative_counter) {
            if(checkForObstacle(CAR_ROW - 200/CELL_SIZE, CAR_COL -90/CELL_SIZE, 75/CELL_SIZE, 120/CELL_SIZE)) updateObstacle(OBSTACLE_FRONT, media_sample->GetTime());
          }
          
          if (!(current_maneuver_ == RIGHT && id != SIGN_STRAIGHT)/* && crossroad_lanes_[2].positive_counter > crossroad_lanes_[2].negative_counter*/ && (id == SIGN_YIELD || id == SIGN_CROSSROAD)) {
            if(checkForObstacle(CAR_ROW - 150/CELL_SIZE, CAR_COL, 90/CELL_SIZE, 90/CELL_SIZE)) updateObstacle(OBSTACLE_RIGHT, media_sample->GetTime());
          }
          
          if (sign_distance < 160 && media_sample->GetTime() - last_script_time_ > 2000000) {

            //std::cout << "Script active at " << sign_distance << " with time: " << media_sample->GetTime() - last_script_time_ << std::endl;      
            last_script_time_ = media_sample->GetTime();
            stringstream ss;
            // Stop sign
            if (id == SIGN_STOP) {
              //std::cout << "Stop Sign" << std::endl;
              ss << "/home/odroid/AADC/scripts/stop.lychee";
              last_script_time_ += 3500000;
              transmitScript(ss.str().c_str(), ss.str().length() + 1);
              ss.str("");
              ss.clear();
            }
            
            if (((current_maneuver_ == LEFT  /*&&crossroad_lanes_[0].positive_counter > crossroad_lanes_[0].negative_counter */&& obstacles_[OBSTACLE_FRONT].exists)) ||
              (!(current_maneuver_ == RIGHT && id != SIGN_STRAIGHT) && /*crossroad_lanes_[2].positive_counter > crossroad_lanes_[2].negative_counter &&*/ 
              (id == SIGN_YIELD || id == SIGN_CROSSROAD) && obstacles_[OBSTACLE_RIGHT].exists)) {
              ss << "/home/odroid/AADC/scripts/wait.lychee";
              last_script_time_ += 6500000;
              transmitScript(ss.str().c_str(), ss.str().length() + 1);
              ss.str("");
              ss.clear();
              //std::cout << "stopped with " << obstacles_[OBSTACLE_FRONT].exists << " and " << obstacles_[OBSTACLE_RIGHT].exists << std::endl;
            }
            
            switch (current_maneuver_) {
              case LEFT:
                //std::cout << "Command Left" << std::endl;
                if (/*crossroad_lanes_[0].positive_counter > crossroad_lanes_[0].negative_counter &&*/ id != SIGN_STRAIGHT) { 
                  maneuver_done_ = true;
                  ss << "/home/odroid/AADC/scripts/curve_left.lychee";
                }
                else {
                  ss << "/home/odroid/AADC/scripts/straight.lychee";
                  //std::cout << "Denied left with " << crossroad_lanes_[0].positive_counter << " to " << crossroad_lanes_[0].negative_counter << std::endl;
                }
                break;
              case STRAIGHT:
                //std::cout << "Command Straight" << std::endl;
                //if (/*crossroad_lanes_[1].positive_counter > crossroad_lanes_[1].negative_counter*/) { 
                  maneuver_done_ = true;
                  ss << "/home/odroid/AADC/scripts/straight.lychee";
                //}
                break;
              case RIGHT:
                //std::cout << "Command Right" << std::endl;
                if (/*crossroad_lanes_[2].positive_counter > crossroad_lanes_[2].negative_counter  && */id != SIGN_STRAIGHT) { 
                  maneuver_done_ = true;
                  ss << "/home/odroid/AADC/scripts/curve_right.lychee";
                }
                else {
                  ss << "/home/odroid/AADC/scripts/straight.lychee";
                  //std::cout << "Denied right with " << crossroad_lanes_[2].positive_counter << " to " << crossroad_lanes_[2].negative_counter << std::endl;
                }
                break;
              default:
                //std::cout << "No Command (" << current_maneuver_ << ")" << std::endl;
                ss << "/home/odroid/AADC/scripts/straight.lychee";
                break;                  
            }
            script_active_ = true;
            plan_.clear();
            transmitScript(ss.str().c_str(), ss.str().length() + 1);
            transmitActiveFlag(media_sample->GetTime());
          }
        } else if (sign_distance < 230 && id == SIGN_PARKING) { // Parking
          if (current_maneuver_ == CROSS_PARKING) {
            parking_spots_.clear();
            for (int i = 0; i < 4; i++) parking_spots_.push_back(Vector2(CAR_COL, CAR_ROW - (sign_distance - 105 + 50 * i)/CELL_SIZE));
          } else if (current_maneuver_ == PARALLEL_PARKING) {
            parking_spots_.clear();
            for (int i = 0; i < 5; i++) parking_spots_.push_back(Vector2(CAR_COL, CAR_ROW - (sign_distance - 70 + 80 * i)/CELL_SIZE));
          }
        }
      }
    }
    else if (source == &inactive_flag_input_pin_ && current_maneuver_ != NOOP) {
      crossroad_detected_ = false;
      script_active_ = false;
      already_parking_ = false;
      
      for (int i = 0; i < 6; i++) {
        signs_[i].counter = 0;
        signs_[i].is_active = false;
      }
      
      for (int i = 0; i < ANGLE_HISTORY_SIZE - 1; i++) angle_history_.add(-5);
      //std::cout << "Script inactive" << std::endl;
      if (current_maneuver_ == PULL_OUT_LEFT || current_maneuver_ == PULL_OUT_RIGHT || current_maneuver_ == PARALLEL_PARKING || current_maneuver_ == CROSS_PARKING) {
        transmitConfidence(false, media_sample->GetTime());
      }
      if (maneuver_done_ || current_maneuver_ == PULL_OUT_LEFT || current_maneuver_ == PULL_OUT_RIGHT) { 
        //std::cout << "Resetting maneuver with " << current_maneuver_ << std::endl;
        current_maneuver_ = -1;

        transmitCommandDone(media_sample->GetTime());
      }
      maneuver_done_ = false;
    }
    else if (source == &command_input_pin_) {
      //std::cout << std::endl << std::endl << std::endl << std::endl;
      //std::cout << "Current Maneuver: " << current_maneuver_ << std::endl;
            
      {
        __adtf_sample_read_lock_mediadescription(signal_data_description_, media_sample, coder);
        coder->Get("f32Value", (tVoid*) &current_maneuver_);
      }
      //std::cout << "Next Current Maneuver: " << current_maneuver_ << std::endl;
      maneuver_done_ = false;
      
      if (current_maneuver_ == NOOP) {
        transmitSteeringAngle(media_sample->GetTime(), 0);
        transmitSpeed(media_sample->GetTime(), 0);
      }
      
      if(current_maneuver_ == PULL_OUT_LEFT || current_maneuver_ == PULL_OUT_RIGHT) {
        // Suppress emergency stops
        transmitConfidence(true, media_sample->GetTime());
      }
      if (current_maneuver_ == PULL_OUT_LEFT) {
        pullOut(CROSS, PULL_OUT_LEFT, media_sample->GetTime());  
      } else if (current_maneuver_ == PULL_OUT_RIGHT) {
        if (sensorDistance(IR_L_FC) < 50) {
          //std::cout << "Sensor front measures object at " << sensorDistance(IR_L_FC) << std::endl;
          pullOut(PARALLEL, PULL_OUT_RIGHT, media_sample->GetTime());    
        } else {
          //std::cout << "Sensor front measures object at " << sensorDistance(IR_L_FC) << std::endl;
          
          int first_line_row, second_line_row;
          if (checkCrossParking(first_line_row, second_line_row)) {
            //std::cout << "Cross Parking check true." << std::endl;
            pullOut(CROSS, PULL_OUT_RIGHT, media_sample->GetTime());
          } else {
            //std::cout << "Cross Parking check false." << std::endl;
            pullOut(PARALLEL, PULL_OUT_RIGHT, media_sample->GetTime());
          }
        }
      }
    }
		else if (source == &sensor_package_input_ && current_maneuver_ != NOOP) {
      {
        __adtf_sample_read_lock_mediadescription(sensor_package_description_, media_sample, coder);
        coder->Get("ir_l_fc", (tVoid*) &(sensor_data_.value_ir_l_fc));
        coder->Get("ir_l_fl", (tVoid*) &(sensor_data_.value_ir_l_fl));
        coder->Get("ir_l_fr", (tVoid*) &(sensor_data_.value_ir_l_fr));
        coder->Get("ir_s_fc", (tVoid*) &(sensor_data_.value_ir_s_fc));
        coder->Get("ir_s_fl", (tVoid*) &(sensor_data_.value_ir_s_fl));
        coder->Get("ir_s_fr", (tVoid*) &(sensor_data_.value_ir_s_fr));
        coder->Get("ir_s_l", (tVoid*) &(sensor_data_.value_ir_s_l));
        coder->Get("ir_s_r", (tVoid*) &(sensor_data_.value_ir_s_r));
        coder->Get("ir_s_rc", (tVoid*) &(sensor_data_.value_ir_s_rc));
        coder->Get("us_f_l", (tVoid*) &(sensor_data_.value_us_f_l));
        coder->Get("us_f_r", (tVoid*) &(sensor_data_.value_us_f_r));
        coder->Get("us_r_l", (tVoid*) &(sensor_data_.value_us_r_l));
        coder->Get("us_r_r", (tVoid*) &(sensor_data_.value_us_r_r));
      }
    }
		
		transmitImage(media_sample->GetTime());
	}
	
	
	RETURN_NOERROR;
}

/*
// -------------------------------------------------------------------------------------------------
void MapFilter::classifyCrossroad(int left_cost, int top_cost, int right_cost) {
// -------------------------------------------------------------------------------------------------
  int first, second, third;
  left_free_ = top_free_ = right_free_ = false;
  if (left_cost < top_cost) {
    if (left_cost < right_cost) {
      first = left_cost;
      left_free_ = true; 
      if (top_cost < right_cost) {
        second = top_cost;
        top_free_ = true;
        third = right_cost;
        if (third < second + (second - first) / 2) right_free_ = true;
      } else {
        second = right_cost;
        right_free_ = true;
        third = top_cost;
        if (third < second + (second - first) / 2) top_free_ = true;
      }
    } else {
      first = right_cost;
      right_free_ = true;
      second = left_cost;
      left_free_ = true;
      third = top_cost;
      if (third < second + (second - first) / 2) top_free_ = true;
    }
  } else {
    if (top_cost < right_cost) {
      first = top_cost;
      top_free_ = true;
      if (left_cost < right_cost) {
        second = left_cost;
        left_free_ = true;
        third = right_cost;
        if (third < second + (second - first) / 2) right_free_ = true;
      } else {
        second = right_cost;
        right_free_ = true;
        third = left_cost;
        if (third < second + (second - first) / 2) left_free_ = true;
      }
    } else {
      first = right_cost;
      right_free_ = true;
      second = top_cost;
      top_free_ = true;
      third = left_cost;
      if (third < second + (second - first) / 2) left_free_ = true;
    }
  }
}

// -------------------------------------------------------------------------------------------------
void MapFilter::updateCrossroadCounter(tTimeStamp time_stamp) {
// -------------------------------------------------------------------------------------------------
  if (time_stamp - crossroad_lanes_[0].time_stamp < CROSSROAD_LANES_TIMESTAMP_THRESHOLD) {
    if (left_free_) crossroad_lanes_[0].positive_counter++;
    else crossroad_lanes_[0].negative_counter++;
    crossroad_lanes_[0].time_stamp = time_stamp;
  } else {
    if (left_free_) {
      crossroad_lanes_[0].positive_counter = 1;
      crossroad_lanes_[0].negative_counter = 0;
    } else {
      crossroad_lanes_[0].positive_counter = 0;
      crossroad_lanes_[0].negative_counter = 1;
    }
    crossroad_lanes_[0].time_stamp = time_stamp;
  }
  if (time_stamp - crossroad_lanes_[1].time_stamp < CROSSROAD_LANES_TIMESTAMP_THRESHOLD) {
    if (top_free_) crossroad_lanes_[1].positive_counter++;
    else crossroad_lanes_[1].negative_counter++;
    crossroad_lanes_[1].time_stamp = time_stamp;
  } else {
    if (top_free_) {
      crossroad_lanes_[1].positive_counter = 1;
      crossroad_lanes_[1].negative_counter = 0;
    } else {
      crossroad_lanes_[1].positive_counter = 0;
      crossroad_lanes_[1].negative_counter = 1;
    }
    crossroad_lanes_[1].time_stamp = time_stamp;
  }
  if (time_stamp - crossroad_lanes_[2].time_stamp < CROSSROAD_LANES_TIMESTAMP_THRESHOLD) {
    if (right_free_) crossroad_lanes_[2].positive_counter++;
    else crossroad_lanes_[2].negative_counter++;
    crossroad_lanes_[2].time_stamp = time_stamp;
  } else {
    if (right_free_) {
      crossroad_lanes_[2].positive_counter = 1;
      crossroad_lanes_[2].negative_counter = 0;
    } else {
      crossroad_lanes_[2].positive_counter = 0;
      crossroad_lanes_[2].negative_counter = 1;
    }
    crossroad_lanes_[2].time_stamp = time_stamp;
  }
}*/

//int ctr = 0;
// -------------------------------------------------------------------------------------------------
tResult MapFilter::transmitImage(tTimeStamp time) {
// -------------------------------------------------------------------------------------------------
  // Create the new image sample to transmit at the end	
  cObjectPtr<IMediaSample> image_sample;
  RETURN_IF_FAILED(_runtime->CreateInstance(OID_ADTF_MEDIA_SAMPLE, IID_ADTF_MEDIA_SAMPLE, (tVoid**) &image_sample));
  RETURN_IF_FAILED(image_sample->AllocBuffer(output_format_.nSize));
  
  tVoid* dest_buffer;
  if (IS_OK(image_sample->WriteLock(&dest_buffer))) {
    Mat output_image(sensor_map_.get_map_height(), sensor_map_.get_map_width(), CV_8UC3, (Vec3b*)dest_buffer);
    lane_map_.draw(output_image);
    output_image.at<cv::Vec3b>(CAR_ROW, CAR_COL-1) = cv::Vec3b(255, 255, 0);
    output_image.at<cv::Vec3b>(CAR_ROW, CAR_COL+1) = cv::Vec3b(255, 255, 0);
    for (std::vector<Vector2>::iterator it = plan_.begin(); it != plan_.end(); it++) {
      output_image.at<cv::Vec3b>(it->get_y(), it->get_x()) = cv::Vec3b(0, 255, 255);
      for (int i = 0; i < (CAR_WIDTH / CELL_SIZE); i++) {
        for (int j = 0; j < (CAR_LENGTH / CELL_SIZE); j++) {
          int cur_x = it->get_x() - ((CAR_WIDTH / CELL_SIZE) / 2) + i;
          int cur_y = it->get_y() + j;

          // Retrieve the occupancy probability value and multiply it with the map cost
          if (cur_x < 0 || cur_y < 0 || cur_x >= (int)MAP_WIDTH || cur_y >= (int)MAP_HEIGHT) {
            continue;
          }

          output_image.at<cv::Vec3b>(cur_y, cur_x) += cv::Vec3b(100, 0, 0);
        }
      }      
    }
    if (crossroad_detected_) {
      //std::cout << "Before printing goal lines with goal coordinates (" << left_goal_.get_x() << ", " << left_goal_.get_y() << ")" << std::endl;
      /*for(int i = -11; i <= 11; i++) {
        output_image.at<cv::Vec3b>(left_goal_.get_x() + i, left_goal_.get_y()) = cv::Vec3b(0, 255 * left_free_, 255 * !left_free_);
        output_image.at<cv::Vec3b>(top_goal_.get_x(), top_goal_.get_y() + i) = cv::Vec3b(0, 255 * top_free_, 255 * !top_free_);
        output_image.at<cv::Vec3b>(right_goal_.get_x() + i, right_goal_.get_y()) = cv::Vec3b(0, 255 * right_free_, 255 * !right_free_);
      }
      //std::cout << "Before printing paths with goal coordinates (" << left_goal_.get_x() << ", " << left_goal_.get_y() << ")" << std::endl;
      for (size_t i = 0; i < left_path_.size(); i++) {
        if(lane_map_.occupancy(Vector2(left_path_[i].get_y(), left_path_[i].get_x())) != 0) output_image.at<cv::Vec3b>(left_path_[i].get_x(), left_path_[i].get_y()) = cv::Vec3b(128, 255, 255);
        else output_image.at<cv::Vec3b>(left_path_[i].get_x(), left_path_[i].get_y()) = cv::Vec3b(255, 0, 255);
      }
      for (size_t i = 0; i < top_path_.size(); i++) {        
        if(lane_map_.occupancy(Vector2(top_path_[i].get_y(), top_path_[i].get_x())) != 0) output_image.at<cv::Vec3b>(top_path_[i].get_x(), top_path_[i].get_y()) = cv::Vec3b(128, 255, 255);
        else output_image.at<cv::Vec3b>(top_path_[i].get_x(), top_path_[i].get_y()) = cv::Vec3b(255, 0, 255);
      }
      for (size_t i = 0; i < right_path_.size(); i++) {
        if(lane_map_.occupancy(Vector2(right_path_[i].get_y(), right_path_[i].get_x())) != 0) output_image.at<cv::Vec3b>(right_path_[i].get_x(), right_path_[i].get_y()) = cv::Vec3b(128, 255, 255);
        else output_image.at<cv::Vec3b>(right_path_[i].get_x(), right_path_[i].get_y()) = cv::Vec3b(255, 0, 255);
      }
      //std::cout << "After printing paths" << std::endl;
      left_path_.clear();
      top_path_.clear();
      right_path_.clear();*/
      //std::cout << "After clearing lists" << std::endl;
      crossroad_detected_ = false;
    }
    
    if(stop_line_ahead_) {
      output_image.at<cv::Vec3b>(stop_line_row_, stop_line_col_) = cv::Vec3b(255, 0, 0);
    } else {
      output_image.at<cv::Vec3b>(stop_line_row_, stop_line_col_) = cv::Vec3b(0, 0, 255);
    }
    
    std::stringstream ss;
   
    //ss << "/home/odroid/Desktop/img/debug" << ctr << ".png"; 
    //ctr++;
    //if(crossroad_detected_) imwrite(ss.str().c_str(), output_image);
    
    int first_line_row = 0;
    int second_line_row = 0;
    //if (checkCrossParking(first_line_row, second_line_row)) std::cout << "Cross parking with " << first_line_row - second_line_row << std::endl;
    //else std::cout << "Normal parking with " << first_line_row - second_line_row << std::endl;
		output_image.at<cv::Vec3b>(first_line_row, CAR_COL) = cv::Vec3b(0, 255, 100);
		output_image.at<cv::Vec3b>(second_line_row, CAR_COL) = cv::Vec3b(255, 0, 100);
		
		for (int i = 0; i < parking_spots_.size(); i++) output_image.at<cv::Vec3b>(parking_spots_[i].get_y(), CAR_COL) = cv::Vec3b(0, 100, 200);
		
		object_map_.draw(output_image);
    
    image_sample->Unlock(dest_buffer);
    image_sample->SetTime(time);
    RETURN_IF_FAILED(map_video_output_pin_.Transmit(image_sample));
  }	
  RETURN_NOERROR;
}
  
// -------------------------------------------------------------------------------------------------
tResult MapFilter::transmitSteeringAngle(tTimeStamp time, float angle) {
// -------------------------------------------------------------------------------------------------
  // Create the media sample
  cObjectPtr<IMediaSample> sample;
  AllocMediaSample((tVoid**) &sample);
  
  // Allocate the memory needed by the media sample by using info of the serializer
  cObjectPtr<IMediaSerializer> serializer;
  signal_data_description_->GetMediaSampleSerializer(&serializer);
  tInt size = serializer->GetDeserializedSize();
  sample->AllocBuffer(size);
  
  {
    __adtf_sample_write_lock_mediadescription(signal_data_description_, sample, coder);
    coder->Set("f32Value", (tVoid*) &angle);
    coder->Set("ui32ArduinoTimestamp", (tVoid*) &time);
  } 
	
	RETURN_IF_FAILED(steering_output_pin_.Transmit(sample));
	RETURN_NOERROR;  
}

// -------------------------------------------------------------------------------------------------
tResult MapFilter::transmitSpeed(tTimeStamp time, float speed) {
// -------------------------------------------------------------------------------------------------
  // Create the media sample
  cObjectPtr<IMediaSample> sample;
  AllocMediaSample((tVoid**) &sample);
  
  // Allocate the memory needed by the media sample by using info of the serializer
  cObjectPtr<IMediaSerializer> serializer;
  signal_data_description_->GetMediaSampleSerializer(&serializer);
  tInt size = serializer->GetDeserializedSize();
  sample->AllocBuffer(size);
	
	{
    __adtf_sample_write_lock_mediadescription(signal_data_description_, sample, coder);
    coder->Set("f32Value", (tVoid*) &speed);
    coder->Set("ui32ArduinoTimestamp", (tVoid*) &time);
  } 
	
	RETURN_IF_FAILED(speed_output_pin_.Transmit(sample));
	RETURN_NOERROR;  
}

// -------------------------------------------------------------------------------------------------
tResult MapFilter::transmitActiveFlag(tTimeStamp time) {
// -------------------------------------------------------------------------------------------------
  cObjectPtr<IMediaSample> sample;
  AllocMediaSample((tVoid**)&sample);

  //allocate memory with the size given by the descriptor
  cObjectPtr<IMediaSerializer> serializer;
  bool_data_description_->GetMediaSampleSerializer(&serializer);
  tInt size = serializer->GetDeserializedSize();
  sample->AllocBuffer(size);

  bool active = true;
  
  {
    __adtf_sample_write_lock_mediadescription(bool_data_description_, sample, coder);
    coder->Set("bValue", (tVoid*) &active);
    coder->Set("ui32ArduinoTimestamp", (tVoid*) &time);
  }

	RETURN_IF_FAILED(active_flag_output_pin_.Transmit(sample));
	RETURN_NOERROR; 
}

// -------------------------------------------------------------------------------------------------
tResult MapFilter::transmitConfidence(bool value, tTimeStamp time) {
// -------------------------------------------------------------------------------------------------
  cObjectPtr<IMediaSample> sample;
  AllocMediaSample((tVoid**)&sample);

  //allocate memory with the size given by the descriptor
  cObjectPtr<IMediaSerializer> serializer;
  confidence_data_description_->GetMediaSampleSerializer(&serializer);
  tInt size = serializer->GetDeserializedSize();
  sample->AllocBuffer(size);
  
  {
    __adtf_sample_write_lock_mediadescription(confidence_data_description_, sample, coder);
    coder->Set("bValue", (tVoid*) &value);
    coder->Set("ui32ArduinoTimestamp", (tVoid*) &time);
  }

	RETURN_IF_FAILED(confidence_output_pin_.Transmit(sample));
	RETURN_NOERROR; 
}

// -------------------------------------------------------------------------------------------------
tResult MapFilter::transmitCommandDone(tTimeStamp time) {
// -------------------------------------------------------------------------------------------------
  cObjectPtr<IMediaSample> sample;
  AllocMediaSample((tVoid**)&sample);

  //allocate memory with the size given by the descriptor
  cObjectPtr<IMediaSerializer> serializer;
  bool_data_description_->GetMediaSampleSerializer(&serializer);
  tInt size = serializer->GetDeserializedSize();
  sample->AllocBuffer(size);

  bool command_done = true;
  
  {
    __adtf_sample_write_lock_mediadescription(bool_data_description_, sample, coder);
    coder->Set("bValue", (tVoid*) &command_done);
    coder->Set("ui32ArduinoTimestamp", (tVoid*) &time);
  }

	RETURN_IF_FAILED(command_output_pin_.Transmit(sample));
	RETURN_NOERROR; 
}

// -------------------------------------------------------------------------------------------------
tResult MapFilter::transmitScript(const char* path, int length) {
// -------------------------------------------------------------------------------------------------
  cObjectPtr<IMediaSample> script_sample;
  RETURN_IF_FAILED(AllocMediaSample(&script_sample));

  RETURN_IF_FAILED(script_sample->AllocBuffer(sizeof(tUInt32) + sizeof(tChar) * length));

  tUInt32* dest_buffer = NULL;
  RETURN_IF_FAILED(script_sample->WriteLock((tVoid**)&dest_buffer));
  
  (*dest_buffer) = (tUInt32)length;
  dest_buffer++;

  cMemoryBlock::MemCopy(dest_buffer, path, sizeof(tChar) * length);
  
  RETURN_IF_FAILED(script_sample->Unlock((tVoid*)dest_buffer));
	script_output_pin_.Transmit(script_sample);
	RETURN_NOERROR;
}

// -------------------------------------------------------------------------------------------------
bool MapFilter::isValid(tInt8 id) {
// -------------------------------------------------------------------------------------------------
  bool is_valid = id == 1 || id == 2 || id == 3 || id == 4 || id == 5 || id == 6;
  //std::cout << "ID: " << (int)id << " is " << is_valid << std::endl;
  return is_valid;
}

// -------------------------------------------------------------------------------------------------
bool MapFilter::isActive(tInt8 id, tTimeStamp time_stamp) {
// -------------------------------------------------------------------------------------------------
  //std::cout << "ID: "  << (int)id << " TimeStamp: " << time_stamp << std::endl;
  for (int i = 0; i < 6; i++) {
    if (!signs_[i].ID == id) continue;
    if (time_stamp - signs_[i].time_stamp > (signs_[i].is_active ? 5 : 1) * SIGN_TIMESTAMP_THRESHOLD) {
      signs_[i].counter = 1;
      signs_[i].time_stamp = time_stamp;
      signs_[i].is_active = false;
      break;
    }
    signs_[i].counter++;
    signs_[i].time_stamp = time_stamp;
    bool is_active = signs_[i].counter > SIGN_COUNTER_THRESHOLD;
    if (is_active) signs_[i].is_active = true;
    return is_active;
  }
  return false;
}

// -------------------------------------------------------------------------------------------------
bool MapFilter::checkForStopLine(int & row, int & col, int offset) {
// -------------------------------------------------------------------------------------------------
  bool found_stop_line = true;
  for (int i = CAR_ROW/* - 36/CELL_SIZE*/; i >= CAR_ROW - 126/CELL_SIZE; i--) {
    if (lane_map_.occupancy(Vector2(CAR_COL + offset, i)) == 1)
    {
      for (int j = -10/CELL_SIZE; j < 10/CELL_SIZE; j++) {
        if(!found_stop_line) {
          row = i;
          col = CAR_COL + offset;
          return false;
        }
        found_stop_line = false;
        for (int k = -3/CELL_SIZE; k < 9/CELL_SIZE; k++) { 
          if(lane_map_.occupancy(Vector2(CAR_COL + offset + j, i + k)) == 1) {
            found_stop_line = true;
            break;
          }
        }
      } 
      row = i;
      col = CAR_COL + offset;
      return true;
    }
  }
  row = CAR_ROW;
  col = CAR_COL;
  return false;
}

// -------------------------------------------------------------------------------------------------
bool MapFilter::checkCrossParking(int & first_line_row, int & second_line_row) {
// -------------------------------------------------------------------------------------------------
  int y_offset = CAR_ROW - ceil(35/CELL_SIZE);
  bool found_first_line = false;
  //int first_line_row = -1;
  bool found_second_line = false;
  //int second_line_row = -1;
  for (int row = y_offset; row > y_offset - 150/CELL_SIZE; row--) {
    for (int col = CAR_COL - 9/CELL_SIZE; col <= CAR_COL + 9/CELL_SIZE; col++) {
      if (lane_map_.occupancy(Vector2(col, row)) == 1) {
        if (checkIfLine(row, col)) {
          if (!found_first_line) {
            found_first_line = true;
            first_line_row = row;
            row -= 6;
            break;
          } else if (!found_second_line) {
            found_second_line = true;
            second_line_row = row;
            break;
          }
        }
      }
    }
    if (found_first_line && found_second_line) break;
  }
  
  return (first_line_row - second_line_row) < 17 && first_line_row > 0 && second_line_row > 0;
}

// -------------------------------------------------------------------------------------------------
bool MapFilter::checkIfLine(int row, int col) {
// -------------------------------------------------------------------------------------------------
  int filled_cells = 1;
  for (int i = -2; i <= 2; i++) {
    for (int j = -9/CELL_SIZE; j < 9/CELL_SIZE; j++) {
      if (lane_map_.occupancy(Vector2(col + j, row + i)) == 1) filled_cells++;
    }
  }
  return filled_cells > 4;
}

// -------------------------------------------------------------------------------------------------
void MapFilter::eraseStopLine(int row, int col) {
// -------------------------------------------------------------------------------------------------
  for (int j = - 6/CELL_SIZE; j <= 12/CELL_SIZE; j++) {
    for (int i = -15/CELL_SIZE; i <= 21/CELL_SIZE; i++) {
      lane_map_.setOccupancy(Vector2(col + i, row - j), 0);
    }
  }
}

// -------------------------------------------------------------------------------------------------
int MapFilter::updateAngleHistory(int angle, int max_angle) {
// -------------------------------------------------------------------------------------------------
  std::vector<double> current_angle_history = angle_history_.getChronological();
  double avg_angle = 0;
  int history_min_angle = -5;
  int history_max_angle = -5;
  
  for (size_t i = 0; i < ANGLE_HISTORY_SIZE; i++) {
    avg_angle += current_angle_history[i];
    if(current_angle_history[i] < history_min_angle) history_min_angle = current_angle_history[i];
    if(current_angle_history[i] > history_max_angle) history_max_angle = current_angle_history[i];
  }
  avg_angle /= ANGLE_HISTORY_SIZE;
  int angle_history_sign = getSign(avg_angle);
  int angle_sign = getSign(angle);
  if ((avg_angle - angle > 15 || avg_angle - angle < -25) && angle_history_sign != angle_sign && max_angle >= 20 && (history_min_angle < -13 || history_max_angle > 10)
    && !(current_angle_history[ANGLE_HISTORY_SIZE - 1] > -2 && current_angle_history[ANGLE_HISTORY_SIZE - 1] < -8) && angle_sign != 0) {
    //std::cout << "Ignored angle " << angle << " with history " << avg_angle << " and drove " << 1.5 * current_angle_history[ANGLE_HISTORY_SIZE - 1] << "instead" << std::endl;
    //angle_history_.add(angle);//(angle + avg_angle)/2.0);
    /*if (fabs(angle) >= fabs(error_angle_) && getSign(angle) * getSign(error_angle_) != -1) {
      angle = (angle + current_angle_history[ANGLE_HISTORY_SIZE - 1]) / 2.0;
      angle_history_.add(angle);
      std::cout << "JK lol, took it anyway ;)" << std::endl;
    }*/
    error_angle_ = angle;
    angle = 1.5 * current_angle_history[ANGLE_HISTORY_SIZE - 1];
    if (fabs(angle) > 30) angle = angle < 0 ? -30 : 30;
    angle_history_.add(angle);
    return angle;
  } else {
    angle_history_.add(angle);
    //std::cout << "Took angle " << angle << std::endl;
    return angle;
  }
}

// -------------------------------------------------------------------------------------------------
int MapFilter::getSign(int angle) {
// -------------------------------------------------------------------------------------------------
  if (angle == -5) return 0;
  if (angle > -5) return 1;
  if (angle < -5) return -1;
  return 0;
}

// -------------------------------------------------------------------------------------------------
void MapFilter::pullOut(int type, int direction, tTimeStamp time_stamp) {
// -------------------------------------------------------------------------------------------------
  stringstream ss;
  if (direction == PULL_OUT_LEFT) ss << "/home/odroid/AADC/scripts/depark_cross_front_left.lychee";
  else if (direction == PULL_OUT_RIGHT) {
    if (type == CROSS) ss << "/home/odroid/AADC/scripts/depark_cross_front_right.lychee";
    else if (type == PARALLEL) ss << "/home/odroid/AADC/scripts/depark_parallel.lychee";
  }
  else return;
  transmitScript(ss.str().c_str(), ss.str().length() + 1);
  script_active_ = true;
  transmitActiveFlag(time_stamp);
}

// -------------------------------------------------------------------------------------------------
void MapFilter::park(int type, tTimeStamp time_stamp) {
// -------------------------------------------------------------------------------------------------
  already_parking_ = true;
  stringstream ss;
  if (type == CROSS) ss << "/home/odroid/AADC/scripts/park_cross_rev_right.lychee";
  else if (type == PARALLEL) ss << "/home/odroid/AADC/scripts/park_parallel.lychee";
  else return;
  script_active_ = true;
  transmitConfidence(true, time_stamp);
  transmitScript(ss.str().c_str(), ss.str().length() + 1);
  transmitActiveFlag(time_stamp);  
}

// -------------------------------------------------------------------------------------------------
double MapFilter::sensorDistance(int sensor_type) {
// -------------------------------------------------------------------------------------------------
  switch(sensor_type) {
    case IR_L_FC: return sensor_data_.value_ir_l_fc;
    case IR_L_FL: return sensor_data_.value_ir_l_fl;
    case IR_L_FR: return sensor_data_.value_ir_l_fr;
    case IR_S_FC: return sensor_data_.value_ir_s_fc;
    case IR_S_FL: return sensor_data_.value_ir_s_fl;
    case IR_S_FR: return sensor_data_.value_ir_s_fr;
    case IR_S_L: return sensor_data_.value_ir_s_l;
    case IR_S_R: return sensor_data_.value_ir_s_r;
    case IR_S_RC: return sensor_data_.value_ir_s_rc;
    case US_F_L: return sensor_data_.value_us_f_l;
    case US_F_R: return sensor_data_.value_us_f_r;
    case US_R_L: return sensor_data_.value_us_r_l;
    case US_R_R: return sensor_data_.value_us_r_r;
    default: return -1;
  }
}

// -------------------------------------------------------------------------------------------------
bool MapFilter::checkForObstacle(int row, int col, int width, int height) {
// -------------------------------------------------------------------------------------------------
  for (int i = row; i < row + height; i++) {
    for (int j = col; j < col + width; j++) {
      if (object_map_.occupancy(Vector2(j, i)) > 0) return true;
    }
  }
  return false;
}

// -------------------------------------------------------------------------------------------------
void MapFilter::updateObstacle(int id, tTimeStamp time_stamp) {
// -------------------------------------------------------------------------------------------------
  if (time_stamp - obstacles_[id].time_stamp > OBSTACLES_TIMESTAMP_THRESHOLD) {
    obstacles_[id].counter = 1;
    obstacles_[id].time_stamp = time_stamp;
    obstacles_[id].exists = false;
  }
  obstacles_[id].counter++;
  obstacles_[id].time_stamp = time_stamp;
  bool exists = obstacles_[id].counter > OBSTACLES_COUNTER_THRESHOLD;
  if (exists) obstacles_[id].exists = true;
}
