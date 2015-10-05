#ifndef _TAPIR_PIPED_FILESOURCE_H_
#define _TAPIR_PIPED_FILESOURCE_H_

#include "imagesource.h"
#include <string>
#include "opencv/highgui.h"
#include "pipe.h"

using namespace Tapir;

/** Reads images from file, with the current index being communicated via pipe.
  *
  * @ingroup IMGSOURCE
  * @author Thomas Lampe */
class PipedFileImageSource : public ImageSource
{
 public:
  PipedFileImageSource () {};
  ~PipedFileImageSource () {};
  bool init (int& width, int& height, const int imgs, unsigned int& add, const char* fname, const char* chapter);
  bool get_next_image (IplImage** imgs, timeval& time);
  void deinit ();

 protected:
  IplImage *_img;
  char _buffer[MAX_STR_LEN], _mask[MAX_STR_LEN], _token[MAX_STR_LEN];
  Tapir::Pipe _pipe;
  int _bytes, _i, _rate;
};

#endif
