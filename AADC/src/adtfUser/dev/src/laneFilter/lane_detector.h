#ifndef _LANE_DETECTOR_2_H_
#define _LANE_DETECTOR_2_H_

#include <opencv2/core/core.hpp>
#include "../util/vector2.h"
#include <vector>

class LaneDetector2 {
 public:
  LaneDetector2() {};
  
  void detect_lanes(cv::Mat const & src, cv::Mat & dst, std::vector<Vector2> & mapped_points);
  
 private:
  void collect_points(cv::Mat const & src, cv::Mat & dst, std::vector<Vector2> & lane_points, int scale, int jump_size);
};

#endif  // _LANE_DETECTOR_2_H_
