#include "V4L2ImageSource.h"
#include "valueparser.h"
#include <iostream>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/stat.h>

#define CLEAR(xxx) memset (&(xxx), 0, sizeof(xxx))
#define ERRNO_PRINT(xxx) { EOUT(xxx << "; errno " << errno << " (" << strerror(errno) << ")"); }
#define ERRNO_EXIT(xxx) { ERRNO_PRINT(xxx); return 0; }

extern "C" {
  static int xioctl (int _fd, int request, void * arg) {
    int r;
    do
      r = ioctl(_fd, request, arg);
    while (-1 == r && EINTR == errno);
    return r;
  }
}

V4L2ImageSource::V4L2ImageSource ()
{
  _io = IO_METHOD_MMAP;
  _fd = -1;
  _buffers = NULL;
  _nbufs = 0;
  _dev = new char[255];
  _once = true;

  _best = 0;
  _current = 0;
  _cycle = 0;
  _entropy = 0.;
  _sweep = true;
  _silent = false;
}

V4L2ImageSource::~V4L2ImageSource ()
{
  unsigned int i;

  switch (_io) {
  case IO_METHOD_READ:
    free(_buffers[0].start);
    break;

  case IO_METHOD_MMAP:
    for (i=0; i<_nbufs; ++i)
      if (-1 == munmap(_buffers[i].start, _buffers[i].length))
        ERRNO_PRINT("munmap");
    break;

  case IO_METHOD_USERPTR:
    for (i=0; i<_nbufs; ++i)
      free(_buffers[i].start);
    break;
  }

  free(_buffers);

  if (-1 == close(_fd))
    ERRNO_PRINT("close");

  _fd = -1;
  delete [] _dev;
}

bool V4L2ImageSource::init (int& width, int& height, const int imgs, unsigned int& add, const char* fname, const char* chapter)
{
  CLS::ValueParser vp(fname, chapter!=0 ? chapter : "ImageSource");

  if (vp.get("device", _dev, 255) < 1) {
    _dev = (char*)"/dev/video0\0";
    WOUT(0, "No device name specified. Using " << _dev << " instead.");
  }

  char config[512];
  if (vp.get("guvcview_config", config, 512) > 0) {
    IOUT("Loading guvcview config from file \"" << config << "\"...");
    stringstream ss;
    ss << "guvcview -o -d " << _dev << " -l " << config << " --exit_on_close &";
    if (system(ss.str().c_str()) != 0)
      WOUT(0, "Failed to load guvcview profile.");
    sleep(1);
    if (system("pkill -9 guvcview") != 0);
  }

  int tmpi;
  vp.get("_buffers", tmpi, 2);
  if (tmpi < 0) {
    WOUT(0, "Number of _buffers must be positive.");
    tmpi = 0;
  }
  _nbufs = tmpi;

  char tmp[64];
  if (vp.get("mode", tmp, 64) < 0) {
    WOUT(0, "No capture mode specified, using \"mmap\".");
  } else if (strcmp(tmp, "userptr") == 0) {
    _io = IO_METHOD_USERPTR;
  } else if (strcmp(tmp, "read") == 0) {
    _io = IO_METHOD_READ;
  } else if (strcmp(tmp, "mmap") != 0) {
    EOUT("Invalid capture mode \"" << tmp << "\".");
    return false;
  }

  _width = width;
  _height = height;

  if (!open_device()) {
    EOUT("Failed to open device.");
    return false;
  }
  if (!init_device()) {
    EOUT("Failed to initialize device.");
    return false;
  }
  if (!start_capturing()) {
    EOUT("Failed to start capture.");
    return false;
  }

  ///< \todo properly determine and set framerate; not with V4L?
  int fps;
  vp.get("fps", fps, 30);
  _ftime = 1000000 / fps;

  _img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
  add = 1;

  bool autogain;
  vp.get("autogain", autogain, false);
  if (autogain) {
    vp.get("exposure_gain_ratio", _factor, 3);
    IplImage *tmpimg[1];
    timeval tmptime;

    IOUT("Determining optimal camera parameters...");
    set_gain_and_exposure(0);
    _silent = true;
    usleep(500000);
    while (optimize()) {
      gettimeofday(&_start, NULL);
      get_next_image(tmpimg,tmptime);
      usleep(50000);
    }
    _silent = false;
    IOUT("Optimal gain " << _best << ", exposure " << _best*_factor << " with entropy " << _entropy);
  }

  return true;
}

void V4L2ImageSource::deinit ()
{
  cvReleaseImage(&_img);
}

bool V4L2ImageSource::get_next_image (IplImage** imgs, timeval& time)
{
  // wait if we're too fast, color errors produced otherwise
  if (_once) {
    _remaining = 0;
    _once = false;
  } else {
    gettimeofday(&_stop, NULL);
    _remaining = _ftime - (1000000 * (_stop.tv_sec - _start.tv_sec) + (_stop.tv_usec - _start.tv_usec));
  }
  if (_remaining > 500)
    usleep(_remaining);
  if (_remaining < -1000)
    WOUT(0, "Desyncing: " << (-_remaining/1000) << "ms");
  gettimeofday(&_start, NULL);

  struct v4l2_buffer buf;
  unsigned int i;

  switch (_io) {
  case IO_METHOD_READ:
    if (-1 == read(_fd, _buffers[0].start, _buffers[0].length)) {
      switch (errno) {
      case EAGAIN:
        return 0;
      case EIO:
      default:
        ERRNO_EXIT("read");
      }
    }

    process_image(_buffers[0].start);
    break;

  case IO_METHOD_MMAP:
    CLEAR(buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl(_fd, VIDIOC_DQBUF, &buf)) {
      switch (errno) {
      case EAGAIN:
        return 0;
      case EIO:
      default:
        ERRNO_EXIT("VIDIOC_DQBUF");
      }
    }

    assert(buf.index < _nbufs);
    process_image(_buffers[buf.index].start);
    if (-1 == xioctl(_fd, VIDIOC_QBUF, &buf))
      ERRNO_EXIT("VIDIOC_QBUF");
    break;

  case IO_METHOD_USERPTR:
    CLEAR(buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_USERPTR;

    if (-1 == xioctl(_fd, VIDIOC_DQBUF, &buf)) {
      switch (errno) {
      case EAGAIN:
        return 0;
      case EIO:
      default:
        ERRNO_EXIT("VIDIOC_DQBUF");
      }
    }

    for (i = 0; i < _nbufs; ++i)
      if (buf.m.userptr == (unsigned long) _buffers[i].start && buf.length == _buffers[i].length)
        break;

    assert(i < _nbufs);
    process_image((void *) buf.m.userptr);
    if (-1 == xioctl(_fd, VIDIOC_QBUF, &buf))
      ERRNO_EXIT("VIDIOC_QBUF");
    break;
  }

  imgs[0] = _img;
  return true;
}

bool V4L2ImageSource::init_mmap ()
{
  struct v4l2_requestbuffers req;
  CLEAR(req);
  req.count = 4;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;

  if (-1 == xioctl(_fd, VIDIOC_REQBUFS, &req)) {
    if (EINVAL == errno) {
      EOUT(_dev << " does not support memory mapping");
      return false;
    } else {
      ERRNO_EXIT("VIDIOC_REQBUFS");
    }
  }

  if (req.count < 2) {
    EOUT("Insufficient buffer memory on " << _dev);
    return false;
  }

  _buffers = (V4L2ImageSource::buffer*)calloc(req.count, sizeof(*_buffers));
  if (!_buffers) {
    EOUT("Out of memory");
    return false;
  }

  for (_nbufs = 0; _nbufs < req.count; ++_nbufs) {
    struct v4l2_buffer buf;
    CLEAR(buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = _nbufs;

    if (-1 == xioctl(_fd, VIDIOC_QUERYBUF, &buf))
      ERRNO_EXIT("VIDIOC_QUERYBUF");

    _buffers[_nbufs].length = buf.length;
    _buffers[_nbufs].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, buf.m.offset);
    if (MAP_FAILED == _buffers[_nbufs].start)
      ERRNO_EXIT("mmap");
  }
  return true;
}

bool V4L2ImageSource::init_usrptr (unsigned int buffer_size)
{
  struct v4l2_requestbuffers req;
  unsigned int page_size;
  page_size = getpagesize();
  buffer_size = (buffer_size + page_size - 1) & ~(page_size - 1);
  CLEAR (req);
  req.count = 4;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_USERPTR;

  if (-1 == xioctl(_fd, VIDIOC_REQBUFS, &req)) {
    if (EINVAL == errno) {
      EOUT(_dev << " does not support user pointer I/O");
      return false;
    } else {
      ERRNO_EXIT("VIDIOC_REQBUFS");
    }
    unsigned int i;

    switch (_io) {
    case IO_METHOD_READ:
      free(_buffers[0].start);
      break;

    case IO_METHOD_MMAP:
      for (i = 0; i < _nbufs; ++i)
        if (-1 == munmap(_buffers[i].start, _buffers[i].length))
          ERRNO_EXIT("munmap");
      break;

    case IO_METHOD_USERPTR:
      for (i = 0; i < _nbufs; ++i)
        free(_buffers[i].start);
      break;
    }

    free(_buffers);
  }

  _buffers = (V4L2ImageSource::buffer*) calloc(4, sizeof(*_buffers));
  if (!_buffers) {
    EOUT("Out of memory");
    return false;
  }

  for (_nbufs = 0; _nbufs < 4; ++_nbufs) {
    _buffers[_nbufs].length = buffer_size;
    _buffers[_nbufs].start = memalign(page_size, buffer_size);
    if (!_buffers[_nbufs].start) {
      EOUT("Out of memory");
      return false;
    }
  }
  return true;
}

bool V4L2ImageSource::init_read (unsigned int buffer_size)
{
  _buffers =(V4L2ImageSource::buffer*) calloc(1, sizeof(*_buffers));
  if (!_buffers) {
    EOUT("Out of memory");
    return false;
  }

  _buffers[0].length = buffer_size;
  _buffers[0].start = malloc(buffer_size);
  if (!_buffers[0].start) {
    EOUT("Out of memory");
    return false;
  }
  return true;
}

bool V4L2ImageSource::start_capturing ()
{
  unsigned int i;
  enum v4l2_buf_type type;

  switch (_io) {
  case IO_METHOD_READ:
    break;

  case IO_METHOD_MMAP:
    for (i = 0; i < _nbufs; ++i) {
      struct v4l2_buffer buf;
      CLEAR (buf);
      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_MMAP;
      buf.index = i;
      if (-1 == xioctl(_fd, VIDIOC_QBUF, &buf))
        ERRNO_EXIT("VIDIOC_QBUF");
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(_fd, VIDIOC_STREAMON, &type))
      ERRNO_EXIT("VIDIOC_STREAMON");
    break;

  case IO_METHOD_USERPTR:
    for (i = 0; i < _nbufs; ++i) {
      struct v4l2_buffer buf;
      CLEAR (buf);
      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_USERPTR;
      buf.index = i;
      buf.m.userptr = (unsigned long) _buffers[i].start;
      buf.length = _buffers[i].length;
      if (-1 == xioctl(_fd, VIDIOC_QBUF, &buf))
        ERRNO_EXIT("VIDIOC_QBUF");
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(_fd, VIDIOC_STREAMON, &type))
      ERRNO_EXIT("VIDIOC_STREAMON");
    break;
  }
  return true;
}

bool V4L2ImageSource::stop_capturing ()
{
  enum v4l2_buf_type type;

  switch (_io) {
  case IO_METHOD_READ:
    break;

  case IO_METHOD_MMAP:
  case IO_METHOD_USERPTR:
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (-1 == xioctl(_fd, VIDIOC_STREAMOFF, &type))
      ERRNO_EXIT("VIDIOC_STREAMOFF");
    break;
  }
  return true;
}

void V4L2ImageSource::process_image (const void* p)
{
  int u, y0, v, y1;
  char* rgbbuffer = reinterpret_cast<char*> (_img->imageData);
  char* yuvbuffer = (char*)p;

  for (int i=_width*_height; i>0; i-=2, ++yuvbuffer, ++rgbbuffer) {
    y0 = *yuvbuffer;
    u = *(++yuvbuffer);
    y1= *(++yuvbuffer);
    v = *(++yuvbuffer);

    *(rgbbuffer) = y0;
    *(++rgbbuffer) = u;
    *(++rgbbuffer) = v;

    *(++rgbbuffer) = y1;
    *(++rgbbuffer) = u;
    *(++rgbbuffer) = v;
  }

  cvCvtColor(_img, _img, CV_YCrCb2RGB);
}

bool V4L2ImageSource::open_device ()
{
  struct stat st;

  if (-1 == stat (_dev, &st)) {
    fprintf (stderr, "Cannot identify '%s': %d, %s\n", _dev, errno, strerror (errno));
    return false;
  }

  if (!S_ISCHR (st.st_mode)) {
    fprintf (stderr, "%s is no device\n", _dev);
    return false;
  }

  _fd = open (_dev, O_RDWR | O_NONBLOCK, 0);
  if (-1 == _fd) {
    fprintf (stderr, "Cannot open '%s': %d, %s\n", _dev, errno, strerror (errno));
    return false;
  }

  return true;
}

bool V4L2ImageSource::init_device ()
{
  struct v4l2_capability cap;
  struct v4l2_cropcap cropcap;
  struct v4l2_crop crop;
  struct v4l2_format fmt;
  unsigned int min;

  if (-1 == xioctl (_fd, VIDIOC_QUERYCAP, &cap)) {
    if (EINVAL == errno) {
      fprintf (stderr, "%s is no V4L2 device\n", _dev);
      return false;
    } else {
      ERRNO_EXIT ("VIDIOC_QUERYCAP");
    }
  }

  if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
    fprintf (stderr, "%s is no video capture device\n", _dev);
    return false;
  }

  switch (_io) {
  case IO_METHOD_READ:
    if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
      fprintf (stderr, "%s does not support read i/o\n", _dev);
      return false;
    }
    break;

  case IO_METHOD_MMAP:
  case IO_METHOD_USERPTR:
    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
      fprintf (stderr, "%s does not support streaming i/o\n", _dev);
      return false;
    }
    break;
  }

  // select video input, video standard and tune here

  CLEAR (cropcap);
  cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (0 == xioctl (_fd, VIDIOC_CROPCAP, &cropcap)) {
    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    crop.c = cropcap.defrect;

    if (-1 == xioctl (_fd, VIDIOC_S_CROP, &crop)) {
      switch (errno) {
      case EINVAL:
        break;
      default:
        break;
      }
    }
  } else {
  }

  CLEAR (fmt);
  fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width       = _width;
  fmt.fmt.pix.height      = _height;
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_UYVY;
  fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

  // note VIDIOC_S_FMT may change width and height
  if (-1 == xioctl (_fd, VIDIOC_S_FMT, &fmt))
    ERRNO_EXIT ("VIDIOC_S_FMT");

  min = fmt.fmt.pix.width * 2;
  if (fmt.fmt.pix.bytesperline < min)
    fmt.fmt.pix.bytesperline = min;
  min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
  if (fmt.fmt.pix.sizeimage < min)
    fmt.fmt.pix.sizeimage = min;

  switch (_io) {
  case IO_METHOD_READ:
    init_read (fmt.fmt.pix.sizeimage);
    break;

  case IO_METHOD_MMAP:
    init_mmap ();
    break;

  case IO_METHOD_USERPTR:
    init_usrptr (fmt.fmt.pix.sizeimage);
    break;
  }

  return true;
}

bool V4L2ImageSource::check_param (__u32 id)
{
  memset (&_queryctrl, 0, sizeof(_queryctrl));
  _queryctrl.id = id;
  if (ioctl(_fd, VIDIOC_QUERYCTRL, &_queryctrl) == -1 || _queryctrl.flags & V4L2_CTRL_FLAG_DISABLED) {
    EOUT("Parameter " << id << " not available!");
    return false;
  }
  return true;
}

bool V4L2ImageSource::get_param (__u32 id, __s32 &value)
{
  if (!V4L2ImageSource::check_param(id))
    return false;

  memset(&_control, 0, sizeof(_control));
  _control.id = id;
  if (ioctl(_fd, VIDIOC_G_CTRL, &_control) != 0) {
    EOUT("Failed to get parameter (errno " << errno << ")");
    return false;
  }

  value = _control.value;
  return true;
}

bool V4L2ImageSource::set_param (__u32 id, __s32 value)
{
  if (!V4L2ImageSource::check_param(id))
    return false;

  memset(&_control, 0, sizeof(_control));
  _control.id = id;
  _control.value = value;

  if (ioctl(_fd, VIDIOC_S_CTRL, &_control) == -1) {
    if (!_silent) EOUT("Failed to set parameter (errno " << errno << ")");
    return false;
  }

  return true;
}

bool V4L2ImageSource::optimize ()
{
  if (!_img) return false;
  check_param(V4L2_CID_GAIN);
  check_param(V4L2_CID_EXPOSURE);
  double entropy;

  if (_sweep) {
    entropy = calculate_entropy();
    if (entropy > _entropy) {
      _entropy = entropy;
      _best = _current;
    }
    //IOUT("Test entropy (" << _current << "): " << entropy);
    return set_gain_and_exposure(_current+1);
  }

#if 0
  int phase = _cycle % 20;
  __s32 dir = 1;
  switch (phase) {
    case 10:
      dir = -1;
    case 0:
      _entropy = calculate_entropy();
      set_gain_and_exposure(_current+dir);
      break;
    case 5:
    case 15:
      entropy = calculate_entropy();
      if (entropy > _entropy) {
        _entropy = entropy;
        _best = _current;
      } else
        set_gain_and_exposure(_best);
      IOUT("New entropy (" << _current << "): " << entropy);
      break;
    default:
      break;
  }
#endif

  _cycle++;
  return false;
}

bool V4L2ImageSource::set_gain_and_exposure (__s32 base)
{
  if (set_param(V4L2_CID_EXPOSURE, base * _factor) && set_param(V4L2_CID_GAIN, base)) {
    _current = base;
    return true;
  }
  set_gain_and_exposure(_best);
  _cycle = 0;
  _sweep = false;
  return false;
}

double V4L2ImageSource::calculate_entropy ()
{
  int hist_size = 256;
  float range[] = {0,256};
  float *ranges[] = {range};
  double entropy = 0., tmp;
  IplImage *channels[3];

  for (int c=0; c<3; c++) 
    channels[c] = cvCreateImage(cvGetSize(_img), IPL_DEPTH_8U, 1);
  cvSplit(_img, channels[0], channels[1], channels[2], 0);

  for (int c=0; c<3; c++) {
    CvHistogram* hist = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges, 1);
    cvCalcHist(&channels[c], hist, 0, NULL);
    for (int b=0; b<hist_size; b++) {
      tmp = (float)cvGetReal1D(hist->bins, b) / 921600.;
      if (tmp != 0.) entropy -= tmp * log(tmp);
    }
  }

  for (int c=0; c<3; c++)
    cvReleaseImage(&channels[c]);
  return entropy;
}

//REGISTER(ImageSource, V4L2ImageSource, "Acquires images using Video4Linux2.");

