#ifndef _IMGSOURCE_CV_CAMSOURCE_H_
#define _IMGSOURCE_CV_CAMSOURCE_H_

#include "utils/ImageSourceUtils/imagesourceutil.h"
#include <string>
#include "opencv2/highgui/highgui.hpp"

/** Gets image data from OpenCV-compatible cameras.
  * The device number can be specified through the config parameter
  * \device, such as device=1 to use /dev/video1.
  *
  * @ingroup IMGSOURCE
  * @author Thomas Lampe, Stefan Welker */
class CamImageSource : public ImageSource
{
 public:
  CamImageSource();
  ~CamImageSource () {};
  bool get_next_image (IplImage** imgs, timeval& time);
  bool init (int& width, int& height, const int imgs, unsigned int& add, const char* fname, const char* chapter);
  void deinit () {};

 protected:
  bool _cam;
  CvCapture *_capture;
};

#endif
