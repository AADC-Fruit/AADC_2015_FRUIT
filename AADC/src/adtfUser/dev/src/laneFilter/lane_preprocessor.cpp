#include "lane_preprocessor.h"

// -------------------------------------------------------------------------------------------------
LanePreprocessor::LanePreprocessor() : region_of_interest_(0, 192, 640, 225), gaussian_size_(15,15){ // 0, 192, 640, 288
// -------------------------------------------------------------------------------------------------
  brightness_ = 80;
  contrast_ = 3.2;
  gaussian_intensity_ = 3;
  canny_min_ = 35;//50;
  canny_max_ = 105;//150;
  canny_kernel_size_ = 3;  
}

// -------------------------------------------------------------------------------------------------
void LanePreprocessor::preprocess_image(cv::Mat const & src, cv::Mat & dst) {
// -------------------------------------------------------------------------------------------------
  cv::Mat roi_image = src.clone();
  roi_image = roi_image(region_of_interest_);
      
  if (roi_image.channels() == 3) cv::cvtColor(roi_image, roi_image, CV_BGR2GRAY);
  
  cv::GaussianBlur(roi_image, roi_image, gaussian_size_, gaussian_intensity_);
  
  cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
  clahe->setClipLimit(1);
  clahe->apply(roi_image, roi_image);  
  cv::Canny(roi_image, dst, canny_min_, canny_max_, canny_kernel_size_);
  
  int offset = region_of_interest_.y;
  for (size_t i = 0; i < objects_.size(); i++) {
    Object cur = objects_[i];
    if (cur.get_absolute_x() == Object::ERROR_VALUE) continue;

    for (int j = 0; j < cur.get_absolute_width(); j++) {
      for (int k = 0; k < cur.get_absolute_height(); k++) {
        int y = cur.get_absolute_y() + k - offset;
        int x = cur.get_absolute_x() + j;
        if (y < 0 || x < 0 || y >= dst.rows || x >= dst.cols) continue; 
        dst.at<uchar>(y, x) = 0;
      }
    }
  }
}

// -------------------------------------------------------------------------------------------------
void LanePreprocessor::set_brightness(int brightness) {
// -------------------------------------------------------------------------------------------------
  brightness_ = brightness;
}

// -------------------------------------------------------------------------------------------------
void LanePreprocessor::set_contrast(double contrast) {
// -------------------------------------------------------------------------------------------------
  contrast_ = contrast;
}

// -------------------------------------------------------------------------------------------------
void LanePreprocessor::set_gaussian_blur(cv::Size blur_size, double intensity) {
// -------------------------------------------------------------------------------------------------
  gaussian_size_ = blur_size;
  gaussian_intensity_ = intensity;
}

// -------------------------------------------------------------------------------------------------
void LanePreprocessor::set_canny(int canny_min, int canny_max, int canny_kernel_size) {
// -------------------------------------------------------------------------------------------------
  canny_min_ = canny_min;
  canny_max_ = canny_max;
  canny_kernel_size_ = canny_kernel_size;
}

// -------------------------------------------------------------------------------------------------
void LanePreprocessor::set_region_of_interest(int x, int y, int width, int height) {
// -------------------------------------------------------------------------------------------------
  region_of_interest_ = cv::Rect(x, y, width, height);
}

// -------------------------------------------------------------------------------------------------
void LanePreprocessor::set_object_vector(Object const * new_objects, int new_size) {
// -------------------------------------------------------------------------------------------------
  objects_.clear();
  objects_ = std::vector<Object>(new_objects, new_objects + new_size);
}
