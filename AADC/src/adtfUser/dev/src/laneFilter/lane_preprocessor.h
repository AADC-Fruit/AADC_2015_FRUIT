#ifndef _LANE_PREPROCESSOR_H_
#define _LANE_PREPROCESSOR_H_

#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "../util/object.h"

class LanePreprocessor {
 public:
  LanePreprocessor();
  
  void preprocess_image(cv::Mat const & src, cv::Mat & dst);
  void set_brightness(int brightness);
  void set_contrast(double contrast);
  void set_gaussian_blur(cv::Size blur_size, double intensity);
  void set_canny(int canny_min, int canny_max, int canny_kernel_size);
  void set_region_of_interest(int x, int y, int width, int height);
  void set_object_vector(Object const * new_objects, int new_size);
  
 private:
  cv::Rect region_of_interest_;
  int brightness_;
  double contrast_;
  cv::Size gaussian_size_;
  double gaussian_intensity_;
  int canny_min_;
  int canny_max_;
  int canny_kernel_size_;
  std::vector<Object> objects_;
};

#endif  // _LANE_PREPROCESSOR_H_
