#ifndef _V4L2_IMAGESOURCE_H_
#define _V4L2_IMAGESOURCE_H_

#include "utils/ImageSourceUtils/imagesourceutil.h"
#include <linux/videodev2.h>
#include <malloc.h>
#include <sys/mman.h>
#include <unistd.h>

/** Gets image data from Video4Linux2-compatible cameras.
  * The device to be used is specified by the config parameter
  * \b device, with \e /dev/video0 being the default.
  *
  * There are several parameters available that may affect performance.
  * Increasing the number of \b buffers (default 2) may resolve image
  * fragmentation issues on some systems, but introduce lag in others.
  * The \b mode used for memory operations can be set tp \e mmap, 
  * \e read or \e userptr.
  *
  * @ingroup IMGSOURCE
  * @author Thomas Lampe, Stefan Welker */
class V4L2ImageSource : public ImageSource
{
public:
  V4L2ImageSource ();
  ~V4L2ImageSource ();
  bool init (int& width, int& height, const int imgs, unsigned int& add, const char* fname, const char* chapter);
  bool get_next_image (IplImage**, timeval& time);
  void deinit ();

 protected:
  bool open_device ();
  bool init_device ();
  bool init_mmap ();
  bool init_read (unsigned int buffer_size);
  bool init_usrptr (unsigned int buffer_size);
  bool start_capturing ();
  bool stop_capturing ();
  void process_image (const void* p);

  struct v4l2_queryctrl _queryctrl;
  struct v4l2_control _control;
  bool optimize ();
  double calculate_entropy ();
  bool set_gain_and_exposure (__s32 base);
  bool check_param (__u32 id);
  bool get_param (__u32 id, __s32 &value);
  bool set_param (__u32 id, __s32 value);

  struct buffer {
    void *start;
    size_t length;
  };

  enum io_method {
    IO_METHOD_READ,
    IO_METHOD_MMAP,
    IO_METHOD_USERPTR,
  };

  IplImage *_img;
  int _width, _height;
  long int _ftime, _remaining;
  bool _once;
  timeval _start, _stop;
  char *_dev;
  io_method _io;
  int _fd;
  buffer *_buffers;
  unsigned int _nbufs;

  __s32 _best, _factor, _current;
  int _cycle;
  double _entropy;
  bool _sweep, _silent;
};

#endif

