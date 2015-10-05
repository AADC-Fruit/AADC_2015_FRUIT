#ifndef _TAPIR_CV_FILESOURCE_H_
#define _TAPIR_CV_FILESOURCE_H_

#include "imagesource.h"
#include <vector>
#include <string>
#include "opencv/highgui.h"

using namespace Tapir;

/** Gets image data from previously saved image files.
  * The parameter \b path points to the folder in which
  * the files are located.
  *
  * @ingroup IMGSOURCE
  * @author Thomas Lampe, Stefan Welker */
class FileImageSource : public ImageSource
{
 public:
  FileImageSource () {};
  ~FileImageSource () {};
  bool init (int& width, int& height, const int imgs, unsigned int& add, const char* fname, const char* chapter);
  bool get_next_image (IplImage** imgs, timeval& time);
  void deinit ();

 protected:
  /** Resets image sequence to beginning. */
  IplImage *_img;
  unsigned int _current;
  std::vector<std::string> _filelist;
};

#endif
