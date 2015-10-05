#ifndef _DC1394_CAM_SOURCE_H_
#define _DC1394_CAM_SOURCE_H_

#include "utils/ImageSourceUtils/imagesourceutil.h"
#include <dc1394/dc1394.h>
#include <string>

/** Gets image data from FireWire cameras using libdc1394.
  * It allows the following parameters to be set for the device,
  * though not every camera may support each mode:
  * \li \b color: the color format used by the camera, which will be internally
  *     converted to RGB; may be either \e RGB, \e BGR, \e YUV422 and \e MONO;
  *     only YUV422 supports a resolution of 320x240;
  * \li \b framerate: may be any value in \f$1.875*2^x\f$, with x being any
  *     number from 0 to 7; the default value is 15;
  * \li \b guid: the globally unique identifier of the camera; if not set,
  *     the first camera on the bus will be used; note that one 32-bit systems,
  *     setting the GUID is not possible;
  * \li \b memory_channel: if supported by the camera, loads parameters
  *     from the matching memory channel; if not set, the previous settings
  *     will be retained.
  *
  * @ingroup IMGSOURCE
  * @author Thomas Lampe, Sascha Lange */
class Dc1394Cam : public ImageSource
{
 public:
  Dc1394Cam();
  ~Dc1394Cam ();
  bool get_next_image (IplImage** imgs, timeval& time);
  bool init (int& width, int& height, const int imgs, unsigned int& add, const char* fname, const char* chapter);
  void deinit ();

 protected:
  IplImage *_img;
  dc1394_t *_context;
  dc1394camera_t *_camera;
  dc1394camera_list_t *_cameras;
  dc1394video_frame_t *_frame;
  dc1394featureset_t _features;
  int _width, _height;
  bool _bgr;
};

#endif
