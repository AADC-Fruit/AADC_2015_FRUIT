#ifndef _TRANSFORMATION_FILTER_H_
#define _TRANSFORMATION_FILTER_H_

#define OID_ADTF_TRANSFORMATION_FILTER "fruit.dev.transformation_filter"

#include <math.h>
#include "map_transformation.h"

class TransformationFilter : public adtf::cTimeTriggeredFilter {
 ADTF_FILTER(OID_ADTF_TRANSFORMATION_FILTER, "FRUIT Transformation Filter", adtf::OBJCAT_DataFilter);

 public:
  TransformationFilter(const tChar* __info);
  virtual ~TransformationFilter();

 protected:
  tResult Init(tInitStage eStage, __exception);
  tResult Shutdown(tInitStage eStage, __exception);
  tResult OnPinEvent(IPin* source, tInt event_code, tInt param1, tInt param2, IMediaSample* media_sample);
  tResult Cycle(__exception);
  
 private:
  bool check_transformation_speeds(tTimeStamp new_time_stamp);
  bool check_transformation_gyro(float new_gyro_value);
  void transform_map(tTimeStamp new_time_stamp);
  tResult transmitMatrix(std::vector<Vector2> const & matrix);
  
  cInputPin left_wheel_speed_pin_;
 	cInputPin right_wheel_speed_pin_;
 	cInputPin gyro_yaw_input_pin_;
 	
 	cOutputPin matrix_output_pin_;
 	
 	cObjectPtr<IMediaTypeDescription> gyro_input_stream_description_;
 	cObjectPtr<IMediaTypeDescription> speed_input_stream_description_;
 	cObjectPtr<IMediaTypeDescription> output_stream_description_;
 	
 	tTimeStamp last_transformation_time_stamp_;
  float last_transformation_yaw_;
  std::vector<float> left_wheel_speeds_;
  std::vector<float> right_wheel_speeds_;
  float last_measurement_yaw_;
  float average_speed_;
  
  bool forward_;
  bool turn_left_;
  int last_second_;
  
  bool debug_;
  tTimeStamp last_timestamp_;
  
  MapTransformation transformator_;
   
  static const tFloat32 MIN_YAW_CHANGE = 2.0 * M_PI / 180.0;
};

#endif // _TRANSFORMATION_FILTER_H_
