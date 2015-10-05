#include "lane_detector.h"
#include "../util/point_transformer.h"
#include <iostream>

// -------------------------------------------------------------------------------------------------
void LaneDetector2::detect_lanes(cv::Mat const & src, cv::Mat & dst, std::vector<Vector2> & mapped_points) {
// -------------------------------------------------------------------------------------------------
  mapped_points.clear();
  
  std::vector<Vector2> lane_points;
  collect_points(src, dst, lane_points, 5, 70);
  
  for (size_t i = 0; i < lane_points.size(); i++) {
    mapped_points.push_back(PointTransformer::map_to_aerial_view(lane_points[i]));
    cv::circle(dst, cv::Point((int)(mapped_points[i][0]) + (src.cols / 2), src.rows - (int)(mapped_points[i][1])), 3, cv::Scalar(255,0,0), -1);
  }
}

// -------------------------------------------------------------------------------------------------
void LaneDetector2::collect_points(cv::Mat const & src, cv::Mat & dst, std::vector<Vector2> & lane_points, int scale, int jump_size) {
// -------------------------------------------------------------------------------------------------
  for (int row = 0; row < src.rows; row++) {
    for (int col = 0; col < src.cols; col++) {
      if (src.at<uchar>(row, col) == 0) continue;
      lane_points.push_back(Vector2(col, row + 192));
    }
  }
  
  /*for (int row = 0; row < src.rows; row += scale) {
    for (int col = 0; col < src.cols; col++) {
      if (src.at<uchar>(row, col) == 0) continue;
      
      for (int k = jump_size; k > 1; k--) {
        if (k + col > src.cols) continue;
        if (src.at<uchar>(row, col + k) == 255) {
          lane_points.push_back(Vector2((col + k / 2), row + 192));
          col += k;
          break;
        }
      }
    }
  }*/
  
  /*for (int col = 0; col < src.cols; col += scale) {
    for (int row = 0; row < src.rows; row++) {
      if (src.at<uchar>(row, col) == 0) continue;
      
      for (int k = jump_size; k > 1; k--) {
        if (k + row > src.rows) continue;
        if (src.at<uchar>(row + k, col) == 255) {
          lane_points.push_back(Vector2(col, row + (k / 2) + 192));
          row += k;
          break;
        }
      }
    }
  }*/
}
