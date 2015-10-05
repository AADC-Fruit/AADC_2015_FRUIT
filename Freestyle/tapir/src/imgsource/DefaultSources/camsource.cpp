#include "camsource.h"
#include "global.h"
#include <fstream>

CamImageSource::CamImageSource ()
{
}

bool CamImageSource::init (int& width, int& height, const int imgs, unsigned int& add, const char* fname, const char* chapter)
{
  ValueParser vp(fname, CHAPTER "ImageSource");
  int device;
  vp.get("device", device, 0);

  _capture = cvCaptureFromCAM(device);
  if (!_capture) {
    EOUT("CamImageSource failed to initialize: capture is NULL");
    cvWaitKey();
    return false;
  }

  cvSetCaptureProperty(_capture, CV_CAP_PROP_FRAME_WIDTH, width);
  cvSetCaptureProperty(_capture, CV_CAP_PROP_FRAME_HEIGHT, height);

  add = 1;
  return true;
}

bool CamImageSource::get_next_image (IplImage** imgs, timeval& time)
{
  imgs[0] = cvQueryFrame(_capture);
  if (imgs[0] == 0) {
    EOUT("Frame capture failed");
    cvWaitKey();
    return false;
  }
  return true;
}

REGISTER(ImageSource, CamImageSource,"OpenCV cam.");

