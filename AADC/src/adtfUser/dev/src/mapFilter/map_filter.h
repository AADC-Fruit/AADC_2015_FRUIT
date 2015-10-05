#ifndef _MAP_FILTER_H_
#define _MAP_FILTER_H_

#define OID_ADTF_MAP_FILTER  "fruit.dev.map_filter"

#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "../util/map/sensor_map.h"
#include "../util/map/lane_map.h"
#include "../util/map/object_map.h"
#include "../util/gridSearch/GridSearch.h"
#include "../util/sensor/sensor_ir_long.h"
#include "../util/sensor/sensor_ir_short.h"
#include "../util/sensor/sensor_us.h"
#include "../util/control/steering_control.h"
#include "../util/maneuver_actions.h"
#include "../util/circularBuffer/circular_buffer.h"
#include "../util/sensor_data.h"

#include <time.h>

#include <vector>

using namespace cv;
 
class MapFilter : public adtf::cFilter {
  ADTF_FILTER(OID_ADTF_MAP_FILTER, "FRUIT Map Filter", adtf::OBJCAT_DataFilter);
 
 public:
 	MapFilter(const tChar* __info);
 	virtual ~MapFilter();
 	
 	tResult Init(tInitStage stage, __exception);
  tResult Shutdown(tInitStage stage, __exception);
	tResult OnPinEvent(IPin* source, tInt event_code, tInt param1, tInt param2, IMediaSample* media_sample);
 
 protected:
 	// Video output pin
 	cVideoPin map_video_output_pin_;  
  tBitmapFormat output_format_;
  
  cOutputPin steering_output_pin_;
  cOutputPin speed_output_pin_;
  cOutputPin active_flag_output_pin_;
 	cOutputPin script_output_pin_;
  cOutputPin command_output_pin_;
  cOutputPin confidence_output_pin_;
  
  cInputPin lane_input_pin_;
 	cInputPin object_input_pin_;
 	cInputPin transformation_input_pin_;
 	cInputPin road_sign_input_pin_;
 	cInputPin inactive_flag_input_pin_;
 	cInputPin command_input_pin_;
 	cInputPin sensor_package_input_;

  // Coder Descriptor for the input pins
  cObjectPtr<IMediaTypeDescription> signal_data_description_;
  cObjectPtr<IMediaTypeDescription> lane_data_description_;
 	cObjectPtr<IMediaTypeDescription> object_data_description_;
 	cObjectPtr<IMediaTypeDescription> transformation_data_description_;
 	cObjectPtr<IMediaTypeDescription> road_sign_data_description_;
 	cObjectPtr<IMediaTypeDescription> bool_data_description_;
 	cObjectPtr<IMediaTypeDescription> confidence_data_description_;
 	cObjectPtr<IMediaTypeDescription> string_stream_description_;
 	cObjectPtr<IMediaTypeDescription> sensor_package_description_;
 	
 private:
  struct SignTriple {
    tInt8 ID;
    tTimeStamp time_stamp;
    int counter;
    bool is_active;
  };
  
  struct SignTriple fill(tInt8 ID) {
    struct SignTriple sign;
    sign.ID = ID;
    sign.time_stamp = 0;
    sign.counter = 0;
    sign.is_active = false;
    return sign;
  }
  
  SignTriple signs_[6];
  static const int SIGN_TIMESTAMP_THRESHOLD = 150000;
  static const int SIGN_COUNTER_THRESHOLD = 2;
  
  struct CrossroadTriple {
    tTimeStamp time_stamp;
    int positive_counter;
    int negative_counter;
  };
  
  struct CrossroadTriple fill() {
    struct CrossroadTriple crossroad_triple;
    crossroad_triple.time_stamp = 0;
    crossroad_triple.positive_counter = 0;
    crossroad_triple.negative_counter = 0;
    return crossroad_triple;
  }
  
  struct ObstacleTriple {
    tTimeStamp time_stamp;
    int counter;
    bool exists; 
  };
  
  struct ObstacleTriple create() {
    struct ObstacleTriple obstacle_triple;
    obstacle_triple.time_stamp = 0;
    obstacle_triple.counter = 0;
    obstacle_triple.exists = false;
    return obstacle_triple;
  }
  
  // left 0, top 1, right 2
  CrossroadTriple crossroad_lanes_[3];
  static const int CROSSROAD_LANES_TIMESTAMP_THRESHOLD = 600000;
  
  ObstacleTriple obstacles_[2];
  static const int OBSTACLES_TIMESTAMP_THRESHOLD = 300000;
  static const int OBSTACLES_COUNTER_THRESHOLD = 2;
  
  tResult transmitImage(tTimeStamp time);
  tResult transmitSteeringAngle(tTimeStamp time, float angle);
  tResult transmitSpeed(tTimeStamp time, float speed);
  tResult transmitActiveFlag(tTimeStamp time);
  tResult transmitConfidence(bool value, tTimeStamp time);
  tResult transmitScript(const char* path, int length);
  tResult transmitCommandDone(tTimeStamp time);
   
  //void classifyCrossroad(int left_cost, int top_cost, int right_cost);
  //void updateCrossroadCounter(tTimeStamp time_stamp);
  bool isValid(tInt8 id);
  bool isActive(tInt8 id, tTimeStamp time);
  bool checkForStopLine(int & row, int & col, int offset);
  void eraseStopLine(int row, int col);
  bool checkCrossParking(int & first_line_row, int & second_line_row);
  bool checkIfLine(int row, int col);
  int updateAngleHistory(int angle, int max_angle);
  int getSign(int angle);
  void pullOut(int type, int direction, tTimeStamp time_stamp);
  void park(int type, tTimeStamp time_stamp);
  double sensorDistance(int sensor_type);
  bool checkForObstacle(int row, int col, int width, int height);
  void updateObstacle(int id, tTimeStamp time_stamp);
  
  SensorMap sensor_map_;
  LaneMap lane_map_;
  ObjectMap object_map_;

  GridSearch grid_search_;
  SteeringControl steering_control_;

  static const int PLAN_LENGTH = 5;
  static const int PLAN_COST_THRESHOLD = 30000;
  std::vector<Vector2> plan_;

  // Stuff for crossroads
  //Vector2 left_goal_;
  //Vector2 top_goal_;
  //Vector2 right_goal_;
  
  //std::vector<Vector2> left_path_, top_path_, right_path_;
  bool crossroad_detected_;
  bool crossroad_ahead_;
  //bool left_free_, top_free_, right_free_;
  
  int stop_line_row_, stop_line_col_;
  bool stop_line_ahead_;
  
  cCriticalSection critical_section_;
  bool script_active_;
  tTimeStamp last_script_time_;
  int current_maneuver_;
  bool maneuver_done_;
  
  CircularBuffer<double> angle_history_;
  static const int ANGLE_HISTORY_SIZE = 5;
  double error_angle_;
  
  SensorData sensor_data_;
  bool already_parking_;
  std::vector<Vector2> parking_spots_;
};

#endif  // _MAP_FILTER_H_
