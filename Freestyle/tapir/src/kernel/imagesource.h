#ifndef _TAPIR_IMAGESOURCE_H_
#define _TAPIR_IMAGESOURCE_H_

#include "global.h"
#include "modules.h"

/** @defgroup IMGSOURCE Image Source Modules
  * Interface for modules that provide camera images.
  * Unlike other modules, an ImageSource must always provide
  * at least one additional image, and most other modules will
  * assume that it is \i exactly one. */

namespace Tapir
{

/** Interface for modules that provide camera images.
  * Unlike other modules, an ImageSource must always provide
  * at least one additional image, and most other modules will
  * assume that it is \i exactly one. */
class ImageSource : public TapirModule
{
 public:
  ImageSource () {};
  ~ImageSource () {};

  /** Initializes the module. Overrides the default init() of the base Module class to allow modifying dimensions.
    * \param width width of the source image
    * \param height height of the source image
    * \param fname path to the program configuration
    * \param imgs number of images the camera provides
    * \return false if initialization failed */
  virtual bool init (int& width, int& height, const int imgs, unsigned int& add, const char* fname, const char* chapter)
    { _img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3); add = 1; return true; };

  /** Fetches the next frame from the camera.
    * \param img list of raw and processed images
    * \param time return variable to hold the image acquisition time
    * \return false if no frame could be acquired */
  virtual bool get_next_image (IplImage** img, timeval& time) { img[0] = _img; return true; };

  virtual void deinit () { if (_img!=0) cvReleaseImage(&_img); };
  virtual void notify (const long int cmd, const char* params) {};

  protected:
    IplImage *_img;
};

};

#endif

